# DASAM STOCKWIN ARS 일반결제 시나리오 분석

## 1. 개요

**ALLAT_Stockwin_Quick_New_Scenario**는 한국경제TV(WowTV) 신용카드 결제 연동 ARS 시나리오 DLL입니다.
ISDN PRI E1 회선 기반 ARS 시스템에서 고객의 음성 입력을 통한 신용카드 결제를 처리합니다.

### 핵심 컴포넌트 구조

```
IScenario (인터페이스)
    └── CALLAT_WOWTV_Billkey_Easy_Scenario (메인 시나리오 클래스)
            ├── CADODB (MS SQL Server ADO 데이터베이스 연결)
            ├── PayLetterAPI (REST API - 주문정보 조회)
            │       └── PL_GetPaymentInfo() - JSON 기반 HTTP/SSL 통신
            └── AllatUtil (ALLAT PG사 SSL 통신)
```

> **Note**: 기존 `CWowTvSocket` (TCP 소켓 통신)은 `PayLetterAPI` REST API로 대체되었습니다.

---

## 2. 시나리오 Flow (상태 머신)

### 2.1 전체 시나리오 흐름도

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         ARS 결제 시나리오 흐름                           │
│                    (ANI 자동 인식 - 전화번호 입력 불필요)                  │
└─────────────────────────────────────────────────────────────────────────┘

[전화 착신] ─→ DNIS/ANI 자동 캡처
    │           (예: DNIS=6690, ANI=01024020684)
    ▼
┌──────────────────────────────────────────┐
│ ALLAT_WOWTV_Quick_CIAScenarioStart [0]   │ → 인사말 재생
│ 인사말: wownet_intro.wav                  │   "안녕하세요, 와우넷 결제 안내입니다"
└────────┬─────────────────────────────────┘
         │ POST_PLAY
         ▼
┌──────────────────────────────────────────┐
│ PayLetter REST API 호출                   │ ← [주문정보 조회]
│ PL_InfoOrderReq_Process()                 │
│ URL: devswbillapi.wowtv.co.kr             │
│      /v1/payment/simple/getpaymentinfo_V2 │
└────────┬─────────────────────────────────┘
         │ 주문정보 수신 완료
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_WOWTV_Quick_getOrderInfo [0~10]    │ → TTS 주문정보 안내
│ (고객명, 가맹점명, 상품명, 금액)           │   "고객님 SW2637262148, 상품 xxx, 금액 20,000원"
└────────┬─────────────────────────────────┘
         │ POST_DTMF
         ▼
┌──────────────────────────────────────────┐
│ 결제진행 확인 (1=예, 2=취소)              │ ← DTMF 입력 대기
└────────┬─────────────────────────────────┘
         │ '1' 입력
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_WOWTV_Quick_CardInput [0~3]        │ → "카드번호 16자리를 눌러주세요"
│ 카드번호 입력 (13~16자리)                 │
└────────┬─────────────────────────────────┘
         │ POST_DTMF
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_WOWTV_Quick_EffecDate [0~3]        │ → "유효기간 월2자리, 년도2자리"
│ 유효기간 입력 (MMYY 4자리)                │
└────────┬─────────────────────────────────┘
         │ POST_DTMF
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_WOWTV_Quick_InstallmentCConfrim    │ → "일시불은 1번, 할부는 2번"
│ 할부 선택 (1=일시불, 2=할부개월 입력)      │
└────────┬─────────────────────────────────┘
         │ '1' 또는 할부개월 입력
         ▼
┌──────────────────────────────────────────┐
│ AllatArsPayProcess()                      │ ← [ALLAT 결제 승인 API]
│ (멀티스레드 비동기 처리)                   │
└────────┬─────────────────────────────────┘
         │ 승인결과 수신
         ▼
┌──────────────────────────────────────────┐
│ setPayLogProc()                           │ ← [DB 결제결과 저장]
│ 저장프로시저: setPayLog                    │
└────────┬─────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_WOWTV_Quick_payARS [0]             │ → 결과 안내
│ 성공: "결제가 완료되었습니다. 승인번호 xxx" │
│ 실패: pay_fail_msg.wav 에러 안내           │
└────────┬─────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_ExitSvc [0]                        │ → service_end.wav
│ gc_hookon() → 전화 종료                   │
└──────────────────────────────────────────┘
```

### 2.2 인증타입별 카드정보 입력 흐름

| 인증타입 | 코드 | 입력 순서 |
|----------|------|-----------|
| 비인증 | 01 | 카드번호 → 유효기간 → 할부개월 → 결제 |
| 구인증 | 02 | 카드번호 → 유효기간 → 생년월일 → 할부개월 → 비밀번호 → 결제 |
| 부분인증 | 03 | 카드번호 → 유효기간 → 생년월일 → 할부개월 → 결제 |
| 빌키 비인증 | 41 | 빌키 동의 → (동의시 빌키발급) → 결제 |
| 빌키 구인증 | 42 | 빌키 동의 → 비밀번호 → (동의시 빌키발급) → 결제 |
| 빌키 부분인증 | 43 | 빌키 동의 → (동의시 빌키발급) → 결제 |

### 2.3 카드정보 입력 상태 머신

```
┌─ ALLAT_CardInput (카드번호 입력)
│   ├─ State 0: "카드번호 16자리를 눌러주세요" 음성안내
│   ├─ State 1: DTMF 16자리 수집, 유효성 검사
│   │           - Luhn 알고리즘 체크
│   │           - 13~16자리 허용
│   └─ State 2: ALLAT_EffecDate(0) → 유효기간 입력으로 전이
│
├─ ALLAT_EffecDate (유효기간 입력)
│   ├─ State 0: "유효기간 월 2자리, 년도 2자리를 눌러주세요" 음성안내
│   ├─ State 1: DTMF 4자리 수집 (MMYY 형식)
│   │           - 월 01~12 범위 검증
│   │           - 만료 여부 검사
│   └─ State 2: 인증타입 확인
│               ├─ 01(비인증) → ALLAT_InstallmentCConfrim (할부선택)
│               ├─ 02(구인증) → ALLAT_JuminNo (생년월일 입력)
│               └─ 03(부분인증) → ALLAT_JuminNo (생년월일 입력)
│
├─ ALLAT_JuminNo (생년월일/법인번호 입력)
│   ├─ State 0: "생년월일 6자리를 눌러주세요" 음성안내
│   ├─ State 1: DTMF 6자리 수집 (YYMMDD 형식)
│   │           - 날짜 유효성 검증
│   └─ State 2: 인증타입 확인
│               ├─ 02(구인증) → ALLAT_CardPw (비밀번호 입력)
│               └─ 03(부분인증) → ALLAT_InstallmentCConfrim (할부선택)
│
├─ ALLAT_CardPw (비밀번호 앞2자리 입력)
│   ├─ State 0: "카드 비밀번호 앞 2자리를 눌러주세요" 음성안내
│   ├─ State 1: DTMF 2자리 수집
│   └─ State 2: ALLAT_InstallmentCConfrim (할부선택)
│
├─ ALLAT_InstallmentCConfrim (할부개월 선택)
│   ├─ State 0: "일시불은 1번, 할부는 2번을 눌러주세요" 음성안내
│   ├─ State 1-1: '1' → 일시불(00) 설정
│   └─ State 1-2: '2' → ALLAT_Installment (할부개월 입력)
│
├─ ALLAT_Installment (할부개월 입력)
│   ├─ State 0: "할부개월 2자리를 눌러주세요" 음성안내
│   ├─ State 1: DTMF 2자리 수집 (02~12)
│   └─ State 2: 최종 결제 확인으로 전이
│
└─ ALLAT_consent (빌키 동의 확인) - 41~43 인증타입 전용
    ├─ State 0: "간편결제 등록에 동의하시면 1번, 미동의시 2번" 음성안내
    └─ State 1:
        ├─ '1'(동의) → Allat_Get_FixKey_host(90) [빌키 발급]
        └─ '2'(미동의) → AllatPaymemt_host(90) [일반결제]
```

---

## 3. API 정리

### 3.1 주문정보 조회 API (PayLetter REST API)

#### 함수: `PL_InfoOrderReq_Process()` (PayLetterAPI.cpp)

**호출 방식**: HTTP/SSL POST (JSON)

```
Base URL (개발): https://devswbillapi.wowtv.co.kr
Base URL (운영): https://swbillapi.wowtv.co.kr
Endpoint: /v1/payment/simple/getpaymentinfo_V2
Method: POST
Content-Type: application/json
```

**인증 헤더**:
```
Authorization: BASIC {signature}
```
- Signature: SHA256 HMAC (AppId + Method + Timestamp + Random)

**JSON 요청**:
```json
{
  "reqType": 1,
  "reqTypeVal": "6690",       // DNIS (착신번호)
  "phoneNo": "01024020684",   // ANI (발신자 전화번호)
  "ARSType": "ARS"
}
```

**JSON 응답**:
```json
{
  "resultCode": "0",
  "data": {
    "memberId": "eda9c2e5-7ee5-405e-ac77-713fe148e32e",
    "orderNo": 202601230105280,
    "nickName": "SW2637262148",
    "packageId": 101164,
    "itemName": "주식비타민 정규 1개월 2만원",
    "categoryId_2nd": "AAAAAA",
    "pgCode": "A",
    "merchantId": "T_arsstockwin1",
    "mallIdSimple": "T_arsstockwin1",
    "mallIdGeneral": "T_arsstockwin",
    "payAmt": 20000,
    "CouponUseFlag": "N",
    "CouponName": "",
    "BonusCashUseFlag": "Y",
    "BonusCashUseAmt": 0,
    "purchaseAmt": 20000,
    "notiUrlSimple": "http://devswbilluser.wowtv.co.kr/Fillup/Allat/AllatCallBack",
    "notiUrlGeneral": "http://devswbilluser.wowtv.co.kr/Fillup/Allat/AllatCallBack",
    "billKey": "",
    "billPassword": "",
    "cardCompany": "",
    "payAgreeFlag": "N",
    "purchaseLimitFlag": "1",
    "memberState": 1,
    "serviceCheckFlag": "N",
    "checkCompleteTime": ""
  }
}
```

**응답 필드 매핑**:

| API 필드 | 시나리오 변수 | 설명 |
|----------|---------------|------|
| merchantId | m_szMx_id | 가맹점 ID (mallIdGeneral 사용) |
| - | m_szMx_name | 가맹점명 (DB 조회로 보충) |
| nickName | m_szCC_name (buyer_nm) | 고객명/닉네임 |
| itemName | m_szCC_prod_desc | 상품명 |
| orderNo | m_szMx_issue_no | 주문번호 |
| purchaseAmt | m_nAmount | 결제금액 |
| pgCode | - | PG사 코드 (A=ALLAT) |
| packageId | product_cd | 상품 코드 |
| notiUrlGeneral | m_szSHOP_RET_URL | 결과 콜백 URL |
| billKey | - | 기존 빌키 (간편결제용) |
| payAgreeFlag | - | 결제 동의 여부 |

**요청 흐름** (실제 로그 기반):
```cpp
// 1. PayLetter API 초기화
PL_Initialize(isLive, appId, baseUrl);

// 2. 주문정보 조회 API 호출
PL_GetPaymentInfo(reqType, reqTypeVal, phoneNo, arsType);

// 3. JSON 응답 파싱 및 시나리오 변수 설정
pScenario->m_szMx_issue_no = orderNo;
pScenario->m_nAmount = purchaseAmt;
pScenario->m_szCC_name = nickName;
// ...

// 4. DB 보충 조회 (가맹점명 등)
RegOrderInfo();
sp_getAllatOrderInfoByOrderNo();
```

> **Note**: 기존 `Wow_InfoRodocReq_Process()` (TCP 소켓 + XML)는 `PL_InfoOrderReq_Process()` (REST API + JSON)로 대체되었습니다.

---

### 3.2 결제 승인 API (ALLAT PG)

#### 함수: `AllatArsPayProcess()` (ALLAT_Access.cpp)

**호출 방식**: ALLAT PG SDK (SSL 통신)

**요청 파라미터**:
```cpp
// ALLAT_ENCDATA 구조체에 설정
setValue(atEnc, "allat_shop_id", szShopId);      // 가맹점 ID
setValue(atEnc, "allat_order_no", szOrderNo);    // 주문번호
setValue(atEnc, "allat_amt", szAmt);             // 결제금액
setValue(atEnc, "allat_pmember_id", szMemberId); // 회원 ID
setValue(atEnc, "allat_product_nm", szProductNm);// 상품명
setValue(atEnc, "allat_buyer_nm", szBuyerNm);    // 구매자명
setValue(atEnc, "allat_card_no", szCardNo);      // 카드번호
setValue(atEnc, "allat_card_expiry", szCardValidYm); // 유효기간
setValue(atEnc, "allat_card_pwd", szPasswordNo); // 비밀번호 앞2자리
setValue(atEnc, "allat_auth_no", szJuminNo);     // 생년월일/법인번호
setValue(atEnc, "allat_sellmm", szSellMm);       // 할부개월
setValue(atEnc, "allat_enc_data", encryptedData);// 암호화 데이터
```

**API 호출**:
```cpp
ApprovalReq(at_data, "SSL", sMsg);  // 승인 요청 전송
```

**응답 파싱**:
```cpp
getValue("reply_cd=", sMsg, sReplyCd, ...);       // 결과코드 (0000=성공)
getValue("reply_msg=", sMsg, sReplyMsg, ...);     // 결과메시지
getValue("approval_no=", sMsg, sApprovalNo, ...); // 승인번호
getValue("auth_date=", sMsg, sApprovalYMDHMS, ...);// 승인일시
getValue("tid=", sMsg, sTid, ...);                // 거래번호
getValue("card_no=", sMsg, sCardNo, ...);         // 마스킹된 카드번호
```

**응답 저장 구조체** (Card_ResInfo):
```cpp
pScenario->m_CardResInfo.m_szRESULTCODE  // 결과코드
pScenario->m_CardResInfo.m_szRESULTMSG   // 결과메시지
pScenario->m_CardResInfo.m_AUTHCODE      // 승인번호
pScenario->m_CardResInfo.m_AUTHDATE      // 승인일시
pScenario->m_CardResInfo.m_Tid           // 거래번호
pScenario->m_CardResInfo.m_szCardNo      // 카드번호(마스킹)
```

---

### 3.3 빌키 발급 API (ALLAT PG)

#### 함수: `Allat_Get_FixKey_Process()` (ALLAT_Access.cpp)

**목적**: 간편결제용 빌키(Bill Key) 발급

**요청 파라미터**:
```cpp
setValue(atEnc, "allat_fix_key", "");              // 빈값 (신규발급)
setValue(atEnc, "allat_card_no", szCardNo);        // 카드번호
setValue(atEnc, "allat_card_expiry", szCardValidYm);// 유효기간
// 인증타입에 따라 추가 필드...
```

**응답 필드**:
- `fix_key`: 발급된 빌키 (24자리)
- `reply_cd`: 결과코드
- `card_no`: 마스킹된 카드번호

---

### 3.4 결제 취소 API (ALLAT PG)

#### 함수: `AllatArsCancleProcess()` (ALLAT_Access.cpp)

**요청 파라미터**:
```cpp
setValue(atEnc, "allat_shop_id", szShopId);
setValue(atEnc, "allat_amt", szCancelAmt);         // 취소금액
setValue(atEnc, "allat_tid", szTid);               // 원거래 TID
setValue(atEnc, "allat_cancel_pwd", szCancelPwd);  // 취소비밀번호
```

---

## 4. DB 저장 (저장프로시저)

### 4.1 ADODB 클래스 (ADODB.cpp)

**데이터베이스 연결**:
```cpp
// 연결 문자열
"Provider=SQLOLEDB;Data Source=서버주소;Initial Catalog=DB명;User Id=계정;Password=비번"
```

### 4.2 주요 저장프로시저

#### 4.2.1 `upOrderPayState` - 결제 상태 업데이트

**용도**: 주문의 결제상태를 '결제완료'로 변경

```sql
EXEC upOrderPayState
    @orderId = '주문번호',
    @paymentStatus = '결제완료',
    @authCode = '승인번호',
    @approvalDate = '승인일시',
    @amount = 결제금액,
    @tid = '거래번호'
```

**호출 위치**: 결제 승인 성공 후

---

#### 4.2.2 `setPayLog` - 결제 로그 기록

**용도**: 결제 결과를 로그 테이블에 기록 (감사/분석용)

```sql
EXEC setPayLog
    @resultCode = '결과코드',
    @resultMsg = '결과메시지',
    @tid = '거래번호',
    @orderId = '주문번호',
    @amount = 결제금액,
    @timestamp = '처리시간',
    @cardNo = '카드번호(마스킹)',
    @approvalNo = '승인번호'
```

**호출 위치**: 결제 승인 성공/실패 모두

---

#### 4.2.3 `sp_getAllatOrderInfoByTel2` - 전화번호 기반 주문조회

**용도**: 고객 입력 전화번호로 해당 주문정보 조회

```sql
EXEC sp_getAllatOrderInfoByTel2
    @phoneNumber = '01012345678'

-- 출력: orderId, customerName, productName, amount, authType, ...
```

**호출 위치**: 주문정보 조회 시 (TCP 조회 실패 시 대체)

---

#### 4.2.4 `RegOrderInfo` - 주문정보 등록

**용도**: 신규 주문정보 등록

**호출 위치**: 특정 시나리오에서 주문 생성 시

---

## 5. 결과 URL 전송

### 함수: `CreateAg()` / `Http_SSL_RetPageSend()`

**용도**: 결제 완료 후 상점 서버로 결과 콜백

```cpp
// URL 플레이스홀더 치환
szURL.Replace("{replyCd}", pScenario->m_CardResInfo.m_szRESULTCODE);
szURL.Replace("{approvalNo}", pScenario->m_CardResInfo.m_AUTHCODE);
szURL.Replace("{approvalAmt}", pScenario->m_CardResInfo.m_AMT);
szURL.Replace("{approvalDate}", pScenario->m_CardResInfo.m_AUTHDATE);
szURL.Replace("{tid}", pScenario->m_CardResInfo.m_Tid);
szURL.Replace("{cardNo}", pScenario->m_CardResInfo.m_szCardNo);
szURL.Replace("{orderId}", pScenario->m_szMx_issue_no);

// 상점 서버로 전송
Http_SSL_RetPageSend(data, szURL, "POST");
```

---

## 6. 핵심 데이터 구조체

### 6.1 CARDINFO (카드 입력 정보)

```cpp
typedef struct CarsInfo {
    char Card_Num[16+1];     // 카드번호 (16자리)
    char ExpireDt[4+1];      // 유효기간 (MMYY)
    char SecretNo[10+1];     // 생년월일/법인번호
    char Password[2+1];      // 비밀번호 앞2자리
    char InstPeriod[2+1];    // 할부개월 (00=일시불)
    char Fix_Key[24+1];      // 빌키 (간편결제용)
} CARDINFO;
```

### 6.2 Card_ResInfo (결제 승인 응답)

```cpp
typedef struct Card_ResInfo {
    char m_Tid[50+1];           // 거래번호 (ALLAT TID)
    char m_szRESULTCODE[10+1];  // 결과코드 (0000=성공)
    char m_szRESULTMSG[200+1];  // 결과메시지
    char m_AUTHCODE[10+1];      // 승인번호
    char m_AUTHDATE[12+1];      // 승인일시 (YYMMDDHHmmss)
    char m_szMoid[80+1];        // 주문번호
    char m_szCardNo[18+1];      // 카드번호 (마스킹됨)
    char m_AMT[12+1];           // 승인금액
    char szBill_Key[24+1];      // 빌키 (발급된 경우)
    char m_CARD_NM[30+1];       // 카드사명
    char m_INSTALLMENT[2+1];    // 할부개월
} Card_ResInfo;
```

### 6.3 INFOPRODOCREQ/RES (TCP 전문 구조체)

```cpp
#pragma pack(push, 1)  // 1바이트 정렬

typedef struct {
    char szDnis[20];       // 착신번호
    char szPhoneNo[20];    // 고객전화번호
    char szReserved[60];   // 예비
} INFOPRODOCREQ;

typedef struct {
    char szResultCode[4];  // 결과코드
    char szOrderNo[30];    // 주문번호
    char szCustomerName[50];// 고객명
    char szProductName[100];// 상품명
    char szAmount[12];     // 금액
    char szAuthType[2];    // 인증타입
    // ... 추가 필드
} INFOPRODOCRES;

#pragma pack(pop)
```

---

## 7. 에러 처리

### 7.1 상태 코드 기반 에러 처리

```cpp
// 결제 API 호출 결과 확인
if (pScenario->m_PaySysCd < 0) {
    // 연동 실패
    set_guide(399);  // "시스템 오류가 발생했습니다" 음성 안내
    setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
    return;
}

// ALLAT 응답 코드 확인
if (strcmp(pScenario->m_CardResInfo.m_szRESULTCODE, "0000") != 0) {
    // 승인 거부
    // 에러코드별 안내 메시지 분기
    ALLAT_ErrorProcess(pScenario->m_CardResInfo.m_szRESULTCODE);
}
```

### 7.2 DTMF 입력 검증

```cpp
// 전화번호 검증
if ((check_validform("*#:7:12", refinfo)) < 0) {
    // 자릿수 오류
    return send_error();  // 재입력 안내
}

// 010~019 시작 여부 확인
if (strncmp(refinfo, "01", 2) != 0) {
    return send_error();
}
```

### 7.3 주요 에러 코드

| 코드 | 설명 | 처리 |
|------|------|------|
| 0000 | 정상승인 | 성공 처리 |
| 0001 | 카드번호 오류 | 재입력 안내 |
| 0002 | 유효기간 오류 | 재입력 안내 |
| 0003 | 한도초과 | 결제 불가 안내 |
| 0004 | 분실/도난카드 | 결제 거부 안내 |
| 9999 | 시스템 오류 | 재시도 또는 종료 |

---

## 8. 멀티스레드 처리 패턴

### 8.1 비동기 호출 구조

```cpp
// 1. 호스트 앱에서 스레드 시작 요청
getOrderInfo_host(90);  // 90 = 타임아웃(초)

// 2. 별도 스레드 생성
_beginthreadex(NULL, 0, Wow_InfoRodocReq_Process, data, 0, &threadId);

// 3. 스레드 내부에서 네트워크 작업 수행
Http_SSL_RetPageSend(...);  // HTTP/SSL 요청

// 4. 완료 후 호스트 앱에 이벤트 전송
Wow_REQ_Quithostio("success", ch);  // POST_NET 이벤트 발생

// 5. 메인 시나리오에서 다음 상태로 자동 전이
```

### 8.2 타임아웃 처리

```cpp
#define DEFAULT_RECV_TIME  5000   // 수신 타임아웃 5초
#define DEFAULT_SEND_TIME  5000   // 송신 타임아웃 5초
#define MAX_RETRY_COUNT    3      // 최대 재시도 횟수
```

---

## 9. 설정 파일

### 파일: `ALLAT_WOWTV_Billkey_Easy_Scenario_para.ini`

```ini
[COMMON]
LogLevel=3
MaxChannel=240

[ALLAT]
ShopId=가맹점ID
LicenseKey=라이센스키
CancelPwd=취소비밀번호
TestMode=0

[DATABASE]
Server=DB서버주소
Database=DB명
UserId=계정
Password=비밀번호

[TIMEOUT]
RecvTimeout=5000
SendTimeout=5000
```

---

## 10. 참고사항

### 10.1 인코딩
- 모든 소스 파일: UTF-8 with BOM
- 네트워크 전문: EUC-KR (레거시 호환)

### 10.2 채널 지원
- 최대 240채널 동시 처리 (MAXCHAN=240)
- 채널별 독립적인 시나리오 인스턴스

### 10.3 빌드 환경
- Windows Server 2016
- Visual Studio C++
- 빌드 디렉토리: `c:\dasam\windows_pri_20220113\windows_pri\allat_stockwin_quick_new_scenario\`

---

## 11. 실제 실행 로그 예시

### 11.1 정상 시나리오 타임라인 (로그 기반)

아래는 실제 ARS 통화 로그에서 추출한 시나리오 실행 흐름입니다.

```
시간         | 단계                    | 함수/이벤트                              | 설명
-------------|-------------------------|------------------------------------------|---------------------------
23:53:12.638 | 전화 착신               | GCEV_OFFERED                             | DNIS=6690, ANI=01024020684
23:53:12.888 | 통화 연결               | GCEV_ANSWERED                            | 서비스 시작
23:53:14.278 | DLL 로드                | ALLAT_Stockwin_Quick_New_Scenario.dll    | 시나리오 로드 완료
23:53:14.450 | 인사말 재생             | ALLAT_WOWTV_Quick_CIAScenarioStart [0]   | wownet_intro.wav
23:53:19.592 | PayLetter 초기화        | PL_Initialize                            | isLive=0 (개발모드)
23:53:19.701 | 주문정보 조회 시작      | PL_InfoOrderReq_Process START            | REST API 호출 시작
23:53:19.732 | API 요청                | PL_HttpPost                              | URL: /v1/payment/simple/getpaymentinfo_V2
23:53:19.889 | API 응답                | StatusCode=200                           | 주문정보 수신 완료
23:53:19.936 | 주문정보 파싱           | orderNo=202601230105280                  | 금액=20,000원
23:53:20.076 | DB 보충 조회            | RegOrderInfo, sp_getAllatOrderInfoByOrderNo | 가맹점명 조회
23:53:20.357 | 주문정보 안내           | ALLAT_WOWTV_Quick_getOrderInfo [0]       | TTS 안내 시작
23:53:32.593 | 결제 확인               | DTMF '1' 입력                            | 결제 진행 선택
23:53:42.234 | 카드번호 입력           | ALLAT_WOWTV_Quick_CardInput [0]          | 16자리 입력 안내
23:53:49.265 | 카드번호 수집           | ALLAT_WOWTV_Quick_CardInput [1]          | DTMF 16자리 수집 완료
23:53:51.593 | 유효기간 입력           | ALLAT_WOWTV_Quick_EffecDate [0]          | 4자리 입력 안내
23:53:54.859 | 유효기간 수집           | ALLAT_WOWTV_Quick_EffecDate [1]          | DTMF 4자리 수집 완료
23:53:56.750 | 할부 선택               | ALLAT_WOWTV_Quick_InstallmentCConfrim    | 일시불 선택 (DTMF '1')
23:53:56.922 | ALLAT 결제 요청         | AllatArsPayProcess                       | 승인 요청 전송
23:53:57.094 | 결제 응답               | 응답코드: MB14                           | 카드번호 오류
23:53:57.547 | 결제 로그 저장          | setPayLogProc                            | DB 기록
23:53:57.297 | 에러 안내               | ALLAT_WOWTV_Quick_payARS [0]             | pay_fail_msg.wav
23:54:02.735 | 서비스 종료             | ALLAT_ExitSvc [0]                        | service_end.wav
23:54:04.750 | 통화 종료               | gc_hookon()                              | 전화 끊기
```

### 11.2 주요 로그 패턴

#### PayLetter API 호출 로그
```
[PayLetterAPI] PL_Initialize: isLive=0, appId=8c3cdc588ff746599a0beb714b4dce3a, baseUrl=https://devswbillapi.wowtv.co.kr
[PayLetterAPI] PL_GetPaymentInfo: reqType=1, reqTypeVal=6690, phoneNo=01024020684, arsType=ARS
[PayLetterAPI] PL_HttpPost: URL=https://devswbillapi.wowtv.co.kr/v1/payment/simple/getpaymentinfo_V2
[PayLetterAPI] PL_HttpPost: StatusCode=200, ResponseLength=757
```

#### 주문정보 파싱 로그
```
PL_InfoOrderReq_Process: 주문정보 파싱 완료
  주문번호: 202601230105280
  가맹점ID: T_arsstockwin
  회원ID: SW2637262148
  상품명: 주식비타민 정규 1개월 2만원
  별명: 스트림
  결제금액: 20000
  금액: 20000
```

#### ALLAT 결제 결과 로그
```
==============================================
응답코드        : MB14
응답메세지      : 카드번호 오류
==============================================
```

### 11.3 함수 호출 순서

```
1. CreateEngine()                          // DLL 엔트리 포인트
2. ALLAT_WOWTV_Quick_CIAScenarioStart()    // 시나리오 시작
3. PL_Initialize()                         // PayLetter API 초기화
4. PL_InfoOrderReq_Process()               // 주문정보 조회
   └── PL_GetPaymentInfo()                 // REST API 호출
   └── RegOrderInfo()                      // DB 등록
   └── sp_getAllatOrderInfoByOrderNo()     // DB 보충 조회
5. ALLAT_WOWTV_Quick_getOrderInfo()        // 주문정보 TTS 안내
6. ALLAT_WOWTV_Quick_CardInput()           // 카드번호 입력
7. ALLAT_WOWTV_Quick_EffecDate()           // 유효기간 입력
8. ALLAT_WOWTV_Quick_InstallmentCConfrim() // 할부 선택
9. AllatArsPayProcess()                    // ALLAT 결제 승인
10. setPayLogProc()                        // 결제 로그 저장
11. ALLAT_WOWTV_Quick_payARS()             // 결과 안내
12. ALLAT_ExitSvc()                        // 서비스 종료
13. DestroyEngine()                        // DLL 종료
```

### 11.4 에러 코드 참조

| 에러 코드 | 설명 | 처리 방법 |
|-----------|------|-----------|
| MB14 | 카드번호 오류 | 카드번호 재입력 요청 |
| 0000 | 정상 승인 | 승인 완료 안내 |
| MA01 | 유효기간 오류 | 유효기간 재입력 요청 |
| MA05 | 한도 초과 | 결제 불가 안내 |

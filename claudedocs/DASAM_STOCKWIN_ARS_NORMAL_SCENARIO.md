# DASAM STOCKWIN ARS 일반결제 시나리오 분석

**문서 버전**: 2.0
**최종 수정일**: 2026-01-26
**목적**: ALLAT_Stockwin_Quick_New_Scenario 기반 일반결제 ARS 시나리오 분석

---

## 1. 개요

**ALLAT_Stockwin_Quick_New_Scenario**는 한국경제TV(WowTV) 신용카드 결제 연동 ARS 시나리오 DLL입니다.
ISDN PRI E1 회선 기반 ARS 시스템에서 고객의 음성 입력을 통한 신용카드 일반결제를 처리합니다.

### 핵심 컴포넌트 구조

```
IScenario (인터페이스)
    └── CALLAT_Hangung_Quick_Scenario (메인 시나리오 클래스)
            ├── CADODB (MS SQL Server ADO 데이터베이스 연결)
            ├── PayLetterAPI (REST API - 주문정보 조회)
            │       └── PL_GetPaymentInfo() - JSON 기반 HTTP/SSL 통신
            └── AllatUtil (ALLAT PG사 SSL 통신)
```

> **Note**: 기존 `CWowTvSocket` (TCP 소켓 통신)은 `PayLetterAPI` REST API로 대체되었습니다.

### 문서 범위

| 포함 | 제외 |
|------|------|
| ✅ 일반결제 시나리오 | ❌ 정기결제 시나리오 |
| ✅ 쿠폰/보너스캐시 처리 | ❌ 간편결제(빌키) 시나리오 |
| ✅ 상품유형별 안내 (SERVICE/TABLET/EDUCATION) | ❌ 빌키 발급/등록 프로세스 |
| ✅ 투자 유의사항 안내 | |

---

## 2. 시나리오 Flow (상태 머신)

### 2.1 전체 시나리오 흐름도

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         ARS 일반결제 시나리오 흐름                         │
│                    (ANI 자동 인식 - 전화번호 입력 불필요)                  │
└─────────────────────────────────────────────────────────────────────────┘

[전화 착신] ─→ DNIS/ANI 자동 캡처
    │           (예: DNIS=6690, ANI=01024020684)
    ▼
┌──────────────────────────────────────────┐
│ ALLAT_CIP_ScenarioStart [0]              │ → 인사말 재생
│ 인사말: wownet_intro.wav                  │   "안녕하세요, 와우넷 결제 안내입니다"
│ (코드: ALLAT_Stockwin_Quick_New_Scenario.cpp:2070-2084)
└────────┬─────────────────────────────────┘
         │ POST_PLAY
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_CIP_ScenarioStart [1]              │ ← ANI 휴대폰 검증
│ ANI가 휴대폰인 경우 → API 호출           │
│ ANI가 휴대폰 아닌 경우 → 전화번호 입력 요청 │
│ (코드: 2087-2103)
└────────┬─────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│ PayLetter REST API 호출                   │ ← [주문정보 조회]
│ PL_InfoOrderReq_Process()                 │
│ URL: devswbillapi.wowtv.co.kr             │
│      /v1/payment/simple/getpaymentinfo_V2 │
│ (코드: WowTvSocket.cpp:1030-1149)
└────────┬─────────────────────────────────┘
         │ 주문정보 수신 완료
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_getOrderInfo [0]                    │ → 주문정보 검증
│ 시스템 장애/주문정보 없음/와우캐시 결제 체크 │
│ (코드: 1373-1420)
└────────┬─────────────────────────────────┘
         │ 정상
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_getOrderInfo [0] → TTS             │ → 정보제공 동의 안내
│ "한국경제TV를 통해 제공되는...정보제공에    │
│  동의하시겠습니까? 1번=동의, 2번=미동의"   │
│ (코드: 1425-1433)
└────────┬─────────────────────────────────┘
         │ POST_NET → POST_DTMF
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_getOrderInfo [2]                    │ ← DTMF 입력 대기
│ 1=동의 → state 9로 이동                   │
│ 2=미동의 → 시나리오 재시작                │
│ (코드: 1465-1491)
└────────┬─────────────────────────────────┘
         │ '1' 입력 (동의)
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_getOrderInfo [9]                    │ → TTS 결제정보 안내
│ ★ 결제금액 안내 (4분기)                   │
│ ★ 상품유형별 해지조건 안내                │
│ ★ 투자 유의사항 안내                      │
│ ★ 동의 확인 멘트                          │
│ (코드: 1492-1627)
└────────┬─────────────────────────────────┘
         │ POST_NET → POST_DTMF
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_getOrderInfo [12]                   │ ← DTMF 입력 대기
│ 1=결제진행 → 카드입력                     │
│ 2=취소 → 시나리오 재시작                  │
│ (코드: 1664-1690)
└────────┬─────────────────────────────────┘
         │ '1' 입력 (결제진행)
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_CardInput [0~3]                     │ → "카드번호 16자리를 눌러주세요"
│ 카드번호 입력 (13~16자리)                 │
└────────┬─────────────────────────────────┘
         │ POST_DTMF
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_EffecDate [0~3]                     │ → "유효기간 월2자리, 년도2자리"
│ 유효기간 입력 (MMYY 4자리)                │
└────────┬─────────────────────────────────┘
         │ POST_DTMF
         ▼
┌──────────────────────────────────────────┐
│ ALLAT_InstallmentCConfrim                 │ → "일시불은 1번, 할부는 2번"
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
│ ALLAT_payARS [0]                          │ → 결과 안내
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

### 2.2 ALLAT_getOrderInfo 상태 머신 상세

`ALLAT_getOrderInfo()` 함수는 주문정보 조회 후 결제 동의를 받는 핵심 함수입니다.

**코드 위치**: `ALLAT_Stockwin_Quick_New_Scenario.cpp:1356-1720`

```
┌─────────────────────────────────────────────────────────────────┐
│                   ALLAT_getOrderInfo 상태 머신                    │
└─────────────────────────────────────────────────────────────────┘

[State 0] 주문정보 검증
    │
    ├─ m_DBAccess == -1 또는 m_bDnisInfo == -1 → 시스템 장애 안내 → 종료
    ├─ m_bDnisInfo < 0 → 송수신 에러 안내 → 종료
    ├─ m_bDnisInfo == 0 → 주문정보 없음 안내 → 종료
    ├─ m_szretval == "6020" → 와우캐시 결제완료 → State 20
    └─ 정상 → 정보제공 동의 TTS 재생 → State 1

[State 1] TTS 재생 완료 대기
    └─ TTS 파일 재생 → State 2 (DTMF 입력 대기)

[State 2] 정보제공 동의 DTMF 입력
    ├─ '1' (동의) → State 9 (결제정보 안내)
    └─ '2' (미동의) → 시나리오 재시작

[State 9] ★ 결제정보 TTS 안내 (핵심)
    │
    ├─ 결제금액 안내 (4분기)
    ├─ 상품유형별 해지조건 안내
    ├─ 투자 유의사항 안내
    └─ 동의 확인 멘트 → State 10

[State 10] TTS 재생 완료 대기
    └─ TTS 파일 재생 → State 12 (DTMF 입력 대기)

[State 12] 결제진행 동의 DTMF 입력
    ├─ '1' (동의) → ALLAT_CardInput(0) 카드입력 진행
    └─ '2' (취소) → 시나리오 재시작

[State 20] 와우캐시 결제완료 안내
    └─ TTS 재생 → 종료
```

---

## 3. 결제정보 TTS 안내 (State 9)

### 3.1 TTS 멘트 구성 (4부 구조)

**코드 위치**: `ALLAT_Stockwin_Quick_New_Scenario.cpp:1492-1620`

```
┌─────────────────────────────────────────────────────────────────┐
│                      TTS 멘트 4부 구성                           │
├─────────────────────────────────────────────────────────────────┤
│ 1부: 결제금액 안내 (szPaymentMent) - 4분기                       │
│ 2부: 상품유형별 해지조건 안내 (szTermsMent) - 3가지 타입          │
│ 3부: 투자 유의사항 안내 (szInvestMent) - 고정 멘트               │
│ 4부: 동의 확인 멘트 (szConfirmMent) - 고정 멘트                  │
└─────────────────────────────────────────────────────────────────┘

최종 멘트 = szPaymentMent + szTermsMent + szInvestMent + szConfirmMent
```

### 3.2 결제금액 안내 (4분기)

**코드 위치**: `ALLAT_Stockwin_Quick_New_Scenario.cpp:1503-1556`

| 분기 조건 | 변수 조합 | TTS 멘트 |
|-----------|-----------|----------|
| **할인 없음** | bCoupon=false, bBonusCash=false | `[고객명] 고객님, [가맹점명]에서, 주문하신 [상품명]의 결제하실 금액은 [금액]원입니다.` |
| **쿠폰만 적용** | bCoupon=true, bBonusCash=false | `... [고객명]님께서 보유하신 [쿠폰명] 쿠폰이 적용되어 최종 결제 금액은 [금액]원입니다.` |
| **보너스캐시만 적용** | bCoupon=false, bBonusCash=true | `... [고객명]님께서 보유하신 보너스 캐시 [금액]원이 적용되어 최종 결제 금액은 [금액]원입니다.` |
| **둘 다 적용** | bCoupon=true, bBonusCash=true | `... [고객명]님께서 보유하신 [쿠폰명] 쿠폰과 보너스 캐시 [금액]원이 적용되어 최종 결제 금액은 [금액]원입니다.` |

**분기 조건 코드**:
```cpp
bool bCoupon = (strcmp(pScenario->m_szCouponUseFlag, "Y") == 0 &&
                strlen(pScenario->m_szCouponName) > 0);
bool bBonusCash = (pScenario->m_nBonusCashUseAmt > 0);
```

### 3.3 상품유형별 해지조건 안내

**코드 위치**: `ALLAT_Stockwin_Quick_New_Scenario.cpp:1567-1593`

| 상품유형 (m_szCategoryId) | TTS 멘트 |
|---------------------------|----------|
| **SERVICE** (기본값) | 서비스 중도해지 시 해지일까지 이용요금과 해지수수료 10퍼센트와 제공받으신 사은품 정가가 함께 차감됩니다. |
| **TABLET** | 박스 개봉 후 제품 불량을 제외하고는 교환 및 반품이 불가합니다. |
| **EDUCATION** | 본 상품은 교육 상품으로 결제 후 해지가 불가능합니다. 또한, 무료로 제공되는 서비스는 중도 해지 및 일시 정지 파일 양도가 불가합니다. |

**상품유형 정규화 로직** (WowTvSocket.cpp:1107-1117):
```cpp
// TABLET, EDUCATION 외에는 모두 SERVICE로 정규화
if (strcmp(plInfo.categoryId_2nd, "TABLET") == 0 ||
    strcmp(plInfo.categoryId_2nd, "EDUCATION") == 0) {
    strncpy_s(pScenario->m_szCategoryId, plInfo.categoryId_2nd);
} else {
    strncpy_s(pScenario->m_szCategoryId, "SERVICE");
}
```

### 3.4 쿠폰/보너스캐시 소멸 안내 (3분기)

**조건**: 일반상품(SERVICE)이면서 쿠폰 또는 보너스캐시 사용 시에만 추가

**코드 위치**: `ALLAT_Stockwin_Quick_New_Scenario.cpp:1587-1615`

| 분기 조건 | TTS 멘트 |
|-----------|----------|
| **쿠폰 + 보너스캐시 둘 다** | `또한, 자동 결제 시 적용되는 [쿠폰명] 쿠폰과 보너스캐시 [금액]원은 소멸됨을 알려드립니다.` |
| **쿠폰만 적용** | `또한, 자동 결제 시 적용되는 [쿠폰명] 쿠폰이 소멸됨을 알려드립니다.` |
| **보너스캐시만 적용** | `또한, 자동 결제 시 적용되는 [금액]원 보너스캐시는 소멸됨을 알려드립니다.` |

```cpp
// 쿠폰/보너스캐시 사용 시 소멸 안내 추가 (3분기)
if (bCoupon && bBonusCash) {
    // 쿠폰 + 보너스캐시 둘 다 있는 경우
    sprintf_s(szExpireMent, " 또한, 자동 결제 시 적용되는 %s 쿠폰과 보너스캐시 %d원은 소멸됨을 알려드립니다.",
        pScenario->m_szCouponName, pScenario->m_nBonusCashUseAmt);
}
else if (bCoupon) {
    // 쿠폰만 있는 경우
    sprintf_s(szExpireMent, " 또한, 자동 결제 시 적용되는 %s 쿠폰이 소멸됨을 알려드립니다.",
        pScenario->m_szCouponName);
}
else if (bBonusCash) {
    // 보너스캐시만 있는 경우
    sprintf_s(szExpireMent, " 또한, 자동 결제 시 적용되는 %d원 보너스캐시는 소멸됨을 알려드립니다.",
        pScenario->m_nBonusCashUseAmt);
}
```

| 상품유형 | 할인 적용 | 소멸 안내 포함 |
|----------|-----------|----------------|
| SERVICE | 쿠폰+보너스캐시 | ✅ 포함 (쿠폰명+금액 언급) |
| SERVICE | 쿠폰만 사용 | ✅ 포함 (쿠폰명 언급) |
| SERVICE | 보너스캐시만 사용 | ✅ 포함 (금액 언급) |
| SERVICE | 할인 없음 | ❌ 미포함 |
| TABLET | 쿠폰/보너스캐시 사용 | ❌ 미포함 |
| EDUCATION | 쿠폰/보너스캐시 사용 | ❌ 미포함 |

### 3.5 투자 유의사항 안내 (고정 멘트)

**코드 위치**: `ALLAT_Stockwin_Quick_New_Scenario.cpp:1598-1601`

> 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.

### 3.6 동의 확인 멘트 (고정 멘트)

**코드 위치**: `ALLAT_Stockwin_Quick_New_Scenario.cpp:1606-1607`

> 동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.

---

## 4. 데이터 구조

### 4.1 주요 멤버 변수 (ALLAT_Stockwin_Quick_New_Scenario.h)

**코드 위치**: `ALLAT_Stockwin_Quick_New_Scenario.h:17-122`

#### 기본 정보
| 변수명 | 타입 | 설명 |
|--------|------|------|
| `m_szMx_issue_no` | char[81] | 주문번호 |
| `m_szMx_id` | char[33] | 가맹점 ID |
| `m_szMx_name` | char[51] | 가맹점명 |
| `m_szCC_name` | char[65] | 고객명/닉네임 |
| `m_szCC_Prod_Desc` | char[256] | 상품명 |
| `m_nAmount` | int | 결제금액 (할인 후) |
| `m_szpsrtner_nm` | char[257] | 파트너명 |

#### 쿠폰/보너스캐시 (신규 추가)
| 변수명 | 타입 | 설명 |
|--------|------|------|
| `m_szCouponUseFlag` | char[2] | 쿠폰 사용 여부 (Y/N) |
| `m_szCouponName` | char[51] | 쿠폰명 |
| `m_szBonusCashUseFlag` | char[2] | 보너스캐시 사용 여부 (Y/N) |
| `m_nBonusCashUseAmt` | int | 보너스캐시 금액 |
| `m_nPurchaseAmt` | int | 할인 전 원가 |

#### 상품유형
| 변수명 | 타입 | 설명 |
|--------|------|------|
| `m_szCategoryId` | char[17] | 상품유형 (SERVICE/TABLET/EDUCATION) |

#### 구매 제한/상태
| 변수명 | 타입 | 설명 |
|--------|------|------|
| `m_szPurchaseLimitFlag` | char[2] | 구매 제한 상태 (1=정상, 2=불량, 3=횟수초과, 4=시작전, 5=종료, 6=중지) |
| `m_szPgCode` | char[2] | PG 코드 (A=올앳, P=페이레터) |
| `m_nMemberState` | int | 고객상태 (1=비회원, 2=유료회원, 3=기구매자) |
| `m_szServiceCheckFlag` | char[2] | 서비스 점검 여부 (Y/N) |

### 4.2 카드 정보 구조체

```cpp
typedef struct CarsInfo {
    char Card_Num[16+1];     // 카드번호 (16자리)
    char ExpireDt[4+1];      // 유효기간 (MMYY)
    char SecretNo[10+1];     // 생년월일/법인번호
    char Password[2+1];      // 비밀번호 앞2자리
    char InstPeriod[2+1];    // 할부개월 (00=일시불)
} CARDINFO;
```

### 4.3 결제 응답 구조체

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
    char m_CARD_NM[30+1];       // 카드사명
    char m_INSTALLMENT[2+1];    // 할부개월
} Card_ResInfo;
```

---

## 5. API 정리

### 5.1 주문정보 조회 API (PayLetter REST API)

#### 함수: `PL_InfoOrderReq_Process()` (WowTvSocket.cpp:1030-1149)

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
    "CouponUseFlag": "Y",
    "CouponName": "정액 50%할인 쿠폰",
    "BonusCashUseFlag": "Y",
    "BonusCashUseAmt": 5000,
    "purchaseAmt": 30000,
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
| nickName | m_szCC_name | 고객명/닉네임 |
| itemName | m_szCC_prod_desc | 상품명 |
| orderNo | m_szMx_issue_no | 주문번호 |
| payAmt | m_nAmount | 결제금액 (할인 후) |
| purchaseAmt | m_nPurchaseAmt | 할인 전 원가 |
| CouponUseFlag | m_szCouponUseFlag | 쿠폰 사용 여부 |
| CouponName | m_szCouponName | 쿠폰명 |
| BonusCashUseFlag | m_szBonusCashUseFlag | 보너스캐시 사용 여부 |
| BonusCashUseAmt | m_nBonusCashUseAmt | 보너스캐시 금액 |
| categoryId_2nd | m_szCategoryId | 상품유형 (SERVICE/TABLET/EDUCATION으로 정규화) |
| purchaseLimitFlag | m_szPurchaseLimitFlag | 구매 제한 상태 |
| pgCode | m_szPgCode | PG 코드 |
| memberState | m_nMemberState | 고객 상태 |

---

### 5.2 결제 승인 API (ALLAT PG)

#### 함수: `AllatArsPayProcess()` (ALLAT_Access.cpp)

**호출 방식**: ALLAT PG SDK (SSL 통신)

**요청 파라미터**:
```cpp
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
```

---

## 6. 인증타입별 카드정보 입력 흐름

| 인증타입 | 코드 | 입력 순서 |
|----------|------|-----------
| 비인증 | 01 | 카드번호 → 유효기간 → 할부개월 → 결제 |
| 구인증 | 02 | 카드번호 → 유효기간 → 생년월일 → 할부개월 → 비밀번호 → 결제 |
| 부분인증 | 03 | 카드번호 → 유효기간 → 생년월일 → 할부개월 → 결제 |

---

## 7. 에러 처리

### 7.1 주요 에러 코드

| 코드 | 설명 | 처리 |
|------|------|------|
| 0000 | 정상승인 | 성공 처리 |
| MB14 | 카드번호 오류 | 카드번호 재입력 요청 |
| MA01 | 유효기간 오류 | 유효기간 재입력 요청 |
| MA05 | 한도 초과 | 결제 불가 안내 |
| 9999 | 시스템 오류 | 재시도 또는 종료 |

### 7.2 주문정보 조회 에러

| 조건 | 처리 |
|------|------|
| m_DBAccess == -1 | 시스템 장애 안내 → 종료 |
| m_bDnisInfo < 0 | 송수신 에러 안내 → 종료 |
| m_bDnisInfo == 0 | 주문정보 없음 안내 → 종료 |
| m_szretval == "6020" | 와우캐시 결제완료 안내 → 종료 |

---

## 8. 테스트 시나리오

### 8.1 결제금액 안내 테스트

| TC ID | couponUseFlag | couponName | bonusCashUseAmt | 예상 멘트 |
|-------|---------------|------------|-----------------|-----------|
| TC-01 | N | - | 0 | 기본 멘트 (할인 없음) |
| TC-02 | Y | 10%할인쿠폰 | 0 | 쿠폰 적용 멘트 |
| TC-03 | N | - | 1000 | 보너스캐시 적용 멘트 |
| TC-04 | Y | 10%할인쿠폰 | 1000 | 쿠폰+보너스캐시 멘트 |

### 8.2 상품유형별 테스트

| TC ID | categoryId_2nd (원본) | m_szCategoryId (정규화) | 예상 해지조건 멘트 |
|-------|----------------------|------------------------|-------------------|
| TC-05 | (빈값) | SERVICE | 일반상품 해지조건 |
| TC-06 | AAAAAA | SERVICE | 일반상품 해지조건 |
| TC-07 | TABLET | TABLET | 태블릿 교환/반품 불가 안내 |
| TC-08 | EDUCATION | EDUCATION | 교육상품 해지불가 안내 |
| TC-09 | UNKNOWN | SERVICE | 일반상품 해지조건 |

### 8.3 쿠폰/보너스캐시 소멸 안내 테스트

| TC ID | categoryId | couponName | bonusCashUseAmt | 예상 소멸 안내 멘트 |
|-------|------------|------------|-----------------|---------------------|
| TC-10 | SERVICE | 10%할인쿠폰 | 1000 | `...10%할인쿠폰 쿠폰과 보너스캐시 1000원은 소멸됨을 알려드립니다.` |
| TC-11 | SERVICE | 10%할인쿠폰 | 0 | `...10%할인쿠폰 쿠폰이 소멸됨을 알려드립니다.` |
| TC-12 | SERVICE | (없음) | 1000 | `...1000원 보너스캐시는 소멸됨을 알려드립니다.` |
| TC-13 | SERVICE | (없음) | 0 | ❌ 소멸 안내 없음 |
| TC-14 | TABLET | 10%할인쿠폰 | 1000 | ❌ 소멸 안내 없음 (상품유형 불일치) |
| TC-15 | EDUCATION | 10%할인쿠폰 | 1000 | ❌ 소멸 안내 없음 (상품유형 불일치) |

---

## 9. 참고사항

### 9.1 인코딩
- 모든 소스 파일: UTF-8 with BOM
- 네트워크 전문: EUC-KR (레거시 호환)

### 9.2 채널 지원
- 최대 240채널 동시 처리 (MAXCHAN=240)
- 채널별 독립적인 시나리오 인스턴스

### 9.3 빌드 환경
- Windows Server 2016
- Visual Studio C++
- 빌드 디렉토리: `c:\dasam\windows_pri_20220113\windows_pri\allat_stockwin_quick_new_scenario\`

---

## 10. 코드 위치 참조

| 기능 | 파일 | 라인 번호 |
|------|------|-----------|
| ALLAT_getOrderInfo 함수 | ALLAT_Stockwin_Quick_New_Scenario.cpp | 1356-1720 |
| 결제금액 안내 4분기 | ALLAT_Stockwin_Quick_New_Scenario.cpp | 1503-1556 |
| 상품유형별 해지조건 안내 | ALLAT_Stockwin_Quick_New_Scenario.cpp | 1567-1593 |
| 쿠폰/보너스캐시 소멸 안내 (3분기) | ALLAT_Stockwin_Quick_New_Scenario.cpp | 1587-1615 |
| 투자 유의사항 안내 | ALLAT_Stockwin_Quick_New_Scenario.cpp | 1618-1624 |
| 동의 확인 멘트 | ALLAT_Stockwin_Quick_New_Scenario.cpp | 1629-1630 |
| TTS 멘트 조합 | ALLAT_Stockwin_Quick_New_Scenario.cpp | 1635-1643 |
| 상품유형 정규화 로직 | WowTvSocket.cpp | 1107-1117 |
| 쿠폰/보너스캐시 저장 | WowTvSocket.cpp | 1097-1105 |
| PayLetter API 호출 | WowTvSocket.cpp | 1030-1149 |
| ALLAT_CIP_ScenarioStart | ALLAT_Stockwin_Quick_New_Scenario.cpp | 2051-2217 |
| 시나리오 클래스 정의 | ALLAT_Stockwin_Quick_New_Scenario.h | 17-122 |

---

**문서 종료**

| 버전 | 일자 | 작성자 | 변경내용 |
|------|------|--------|----------|
| 1.0 | 2026-01-23 | Claude | 최초 작성 |
| 2.0 | 2026-01-26 | Claude | ALLAT_Stockwin_Quick_New_Scenario 기준 전면 개정 - 쿠폰/보너스캐시 4분기, 상품유형별 안내 (SERVICE/TABLET/EDUCATION), 투자 유의사항 추가, 코드 라인 참조 추가 |
| 2.1 | 2026-01-26 | Claude | 쿠폰/보너스캐시 소멸 안내 3분기로 세분화 - 쿠폰명/금액을 동적으로 TTS 멘트에 포함 |

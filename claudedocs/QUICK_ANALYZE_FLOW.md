# ALLAT Stockwin Quick New Scenario 시나리오 흐름 분석

## 목차
1. [개요](#개요)
2. [시나리오 진입점](#시나리오-진입점)
3. [전체 시나리오 흐름도](#전체-시나리오-흐름도)
4. [세부 함수별 흐름](#세부-함수별-흐름)
5. [분기 조건 상세](#분기-조건-상세)
6. [결제 API 함수](#결제-api-함수)
7. [외부 통신 함수](#외부-통신-함수)
8. [Billkey 버전과의 주요 차이점](#billkey-버전과의-주요-차이점)

---

## 개요

이 DLL은 한국경제TV(WowTV) ALLAT 결제 연동 **일반결제 전용** ARS 시나리오를 처리합니다. ISDN PRI E1 회선 기반 ARS 시스템에서 신용카드 일반결제(비인증 방식)만 지원합니다.

### 핵심 컴포넌트
- **ALLAT_Stockwin_Quick_New_Scenario.cpp**: ARS 시나리오 메인 로직 (일반결제 전용)
- **ALLAT_Access.cpp**: ALLAT PG사 결제 API 호출 (승인/취소)
- **WowTvSocket.cpp**: 외부 서버와 TCP 소켓 통신 (주문 정보 조회)
- **ADODB.cpp**: MS SQL Server ADO 연결 및 저장 프로시저 호출

### 클래스명
`CALLAT_Hangung_Quick_Scenario` - IScenario 인터페이스 구현

### 주요 특징
- **빌키(간편결제) 미지원**: 일반결제만 처리
- **비인증 방식**: 카드번호 + 유효기간만으로 결제 (비밀번호/생년월일 선택적)
- **TCP 소켓 주문 조회**: CIA 시나리오에서 getTcpOrderInfo_host 사용

---

## 시나리오 진입점

### `jobArs()` - 모든 시나리오의 시작점 (Line 2158)

```
jobArs(state)
    │
    ├─ szArsType == "ARS" ───────→ ALLAT_ArsScenarioStart(0)
    │                              (전화번호 직접 입력 방식)
    │
    ├─ szArsType == "SMS" ───────→ ALLAT_SMSScenarioStart(0)
    │                              (SMS 주문번호 입력 방식)
    │
    ├─ szArsType == "CID" ───────→ ALLAT_CID_ScenarioStart(0)
    │                              (Caller ID 자동 인식 방식)
    │
    ├─ szArsType == "CIA" ───────→ ALLAT_CIA_ScenarioStart(0)
    │                              (Caller ID + 휴대폰 인증 방식)
    │
    └─ 기타 ────────────────────→ ALLAT_jobArs(0)
                                   (기본 서비스 준비 중)
```

---

## 전체 시나리오 흐름도

### 1. ARS 시나리오 (전화번호 직접 입력)

```
ALLAT_ArsScenarioStart (Line 1604)
    │
    ├─[state=0] 인사말 재생
    │           "audio\shop_intro\wownet_intro"
    │
    ├─[state=1] 전화번호 입력 안내
    │           "전화번호 입력" (최대 13자리)
    │
    ├─[state=2] 전화번호 입력 처리
    │           유효성 검사: 010/011/012/016/017/018/019 시작
    │
    ├─[state=3] TTS 전화번호 확인
    │           "고객님께서 누르신 전화번호는 XXX번 입니다"
    │
    ├─[state=4] 확인 입력 (1:예, 2:아니오)
    │    │
    │    ├─ '1' (예) ──→ getOrderInfo_host(90) → ALLAT_getOrderInfo(0)
    │    │
    │    └─ '2' (아니오) → ALLAT_ArsScenarioStart(1) (재입력)
    │
    └─ 다음: ALLAT_getOrderInfo
```

### 2. SMS 시나리오 (주문번호 입력)

```
ALLAT_SMSScenarioStart (Line 1747)
    │
    ├─[state=0] 인사말 재생
    │
    ├─[state=1] SMS 주문번호 입력 안내
    │           "SMS로 받은 주문번호 입력" (6자리)
    │
    ├─[state=2] 주문번호 입력 처리
    │
    ├─[state=3] TTS 주문번호 확인
    │
    ├─[state=4] 확인 입력 (1:예, 2:아니오)
    │    │
    │    ├─ '1' (예) ──→ getSMSOrderInfo_host(90) → ALLAT_getOrderInfo(0)
    │    │
    │    └─ '2' (아니오) → ALLAT_SMSScenarioStart(1) (재입력)
    │
    └─ 다음: ALLAT_getOrderInfo
```

### 3. CID 시나리오 (Caller ID 자동 인식)

```
ALLAT_CID_ScenarioStart (Line 1877)
    │
    ├─[state=0] 인사말 재생
    │
    ├─[state=1] ANI(발신자번호)로 자동 주문 조회
    │           m_szInputTel = ani
    │           getOrderInfo_host(90)
    │
    └─ 다음: ALLAT_getOrderInfo(0)
```

### 4. CIA 시나리오 (Caller ID + 휴대폰 확인)

```
ALLAT_CIA_ScenarioStart (Line 1933)
    │
    ├─[state=0] 인사말 재생
    │
    ├─[state=1] ANI 휴대폰 여부 확인
    │    │
    │    ├─ 휴대폰(010/011 등) → getTcpOrderInfo_host(90)
    │    │                       ★ TCP 소켓으로 주문정보 조회
    │    │
    │    └─ 휴대폰 아님 → ALLAT_CIA_ScenarioStart(11) (번호 입력)
    │
    ├─[state=11] 전화번호 입력 안내
    │
    ├─[state=12] 전화번호 입력 처리
    │            휴대폰 번호 형식 검증
    │
    ├─[state=13] TTS 전화번호 확인
    │
    ├─[state=14] 확인 입력 (1:예, 2:아니오)
    │    │
    │    ├─ '1' (예) ──→ getTcpOrderInfo_host(90) → ALLAT_getOrderInfo(0)
    │    │
    │    └─ '2' (아니오) → ALLAT_CIA_ScenarioStart(11) (재입력)
    │
    └─ 다음: ALLAT_getOrderInfo(0)
```

---

## 세부 함수별 흐름

### ALLAT_getOrderInfo - 주문정보 확인 및 동의 처리 (Line 1354)

```
ALLAT_getOrderInfo
    │
    ├─[state=0] 주문정보 조회 결과 확인
    │    │
    │    ├─ m_DBAccess == -1 또는 m_bDnisInfo == -1
    │    │   → 시스템 장애 → ALLAT_ExitSvc(0)
    │    │
    │    ├─ m_bDnisInfo < 0
    │    │   → TCP 송수신 에러 → "Tcp_Error" 안내 → ALLAT_ExitSvc(0)
    │    │
    │    ├─ m_bDnisInfo == 0
    │    │   → 주문정보 없음 → "주문이 접수되지 않았습니다" → ALLAT_ExitSvc(0)
    │    │
    │    ├─ m_szretval == "6020"
    │    │   → 와우캐시 결제 완료 → TTS 안내 → ALLAT_ExitSvc(0)
    │    │
    │    └─ 정보제공 동의 TTS 안내
    │        "한국경제TV를 통해 제공되는 서비스를 위하여 [파트너명]에게..."
    │
    ├─[state=1] 동의 TTS 재생 후
    │
    ├─[state=2] 동의 확인 입력 (1:동의, 2:미동의)
    │    │
    │    ├─ '1' (동의) → ALLAT_getOrderInfo(9)
    │    │
    │    └─ '2' (미동의) → 시나리오 시작점으로 복귀
    │
    ├─[state=9] 상품정보 및 해지 안내 TTS
    │           "[고객명]님, [가맹점명]에서 주문하신 [상품명]의 결제하실 금액은 [금액]원입니다.
    │            서비스 중도해지시 해지일까지 일수 만큼의 이용요금과 가입비의 10%에 해당되는
    │            해지수수료, 사은품 비용이 함께 차감됩니다..."
    │
    ├─[state=10] TTS 재생 후
    │
    ├─[state=12] 동의/취소 확인 (1:동의 및 결제, 2:취소)
    │    │
    │    ├─ '1' (동의) → ALLAT_CardInput(0) (카드입력)
    │    │
    │    └─ '2' (취소) → 시나리오 시작점 복귀
    │
    ├─[state=20] 와우캐시 결제 완료 안내 후 종료
    │
    └─ 다음: ALLAT_CardInput
```

### ALLAT_CardInput - 카드번호 입력 (Line 1238)

```
ALLAT_CardInput
    │
    ├─[state=0] 카드번호 입력 안내
    │           "카드번호 입력 후 우물정자" (13~16자리)
    │           ★ 농협카드 결제 불가 안내 포함
    │
    ├─[state=1] 카드번호 입력 처리
    │           유효성 검사 (13~16자리)
    │
    ├─[state=2] TTS 카드번호 확인
    │           "고객님께서 누르신 카드번호는 X,X,X,X... 번입니다"
    │
    ├─[state=3] 확인 입력 (1:예, 2:아니오)
    │    │
    │    ├─ '1' (예) → ALLAT_EffecDate(0) (유효기간 입력)
    │    │
    │    └─ '2' (아니오) → ALLAT_CardInput(0) (재입력)
    │
    └─ 다음: ALLAT_EffecDate
```

### ALLAT_EffecDate - 유효기간 입력 (Line 1091)

```
ALLAT_EffecDate
    │
    ├─[state=0] 유효기간 입력 안내
    │           "유효기간 4자리를 월,연 순서로 입력" (MMYY)
    │
    ├─[state=1] 유효기간 입력 및 유효성 검사
    │           - 월: 1~12
    │           - 년: 현재년도 이상
    │           - 현재월 이후
    │           - 내부 저장: YYMM 형식으로 변환
    │
    ├─[state=2] TTS 유효기간 확인
    │           "고객님께서 누르신 유효기간은 20XX년 XX월입니다"
    │
    ├─[state=3] 확인 입력 (1:예, 2:아니오)
    │    │
    │    ├─ '1' (예) → ALLAT_InstallmentCConfrim(0) (할부 입력)
    │    │             ★ Quick 버전은 주민번호 입력 생략
    │    │
    │    └─ '2' (아니오) → ALLAT_EffecDate(0) (재입력)
    │
    └─ 다음: ALLAT_InstallmentCConfrim
```

### ALLAT_JuminNo - 생년월일/법인번호 입력 (Line 936)

```
ALLAT_JuminNo
    │
    ├─[state=0] 생년월일 입력 안내
    │           "생년월일 6자리 또는 법인번호 10자리 입력"
    │
    ├─[state=1] 입력 처리 (6자리 또는 10자리)
    │           - 6자리: 생년월일 (YYMMDD)
    │           - 10자리: 법인번호
    │           - 일자 유효성 검사 (1~31일)
    │
    ├─[state=2] TTS 확인
    │           "고객님께서 누르신 생년월일 또는 법인번호는 X,X,X... 번입니다"
    │
    ├─[state=3] 확인 입력 (1:예, 2:아니오)
    │    │
    │    ├─ '1' (예)
    │    │    │
    │    │    ├─ 할부개월수 이미 입력된 경우
    │    │    │   → ALLAT_InstallmentCConfrim(3) (TTS 확인)
    │    │    │
    │    │    └─ 그 외
    │    │        → ALLAT_InstallmentCConfrim(0) (할부 입력)
    │    │
    │    └─ '2' (아니오) → ALLAT_JuminNo(0) (재입력)
    │
    └─ 다음: ALLAT_InstallmentCConfrim
```

### ALLAT_InstallmentCConfrim - 할부개월수 입력 (Line 722)

```
ALLAT_InstallmentCConfrim
    │
    ├─[state=0] 할부개월수 입력 또는 자동 설정
    │    │
    │    ├─ 금액 < 50,000원 (PARAINI의 ALLAT_MIN_AMT)
    │    │   → 할부 0개월(일시불) 자동 설정
    │    │   → AllatPaymemt_host(90) 직접 호출 (결제 진행)
    │    │
    │    └─ 금액 >= 50,000원
    │        → 할부개월수 입력 안내
    │        "요청하실 할부개월수를... 일시불은 0번..."
    │        ★ 무이자 할부 카드사별 안내 멘트
    │
    ├─[state=1] 할부개월수 입력 처리
    │    │
    │    ├─ 0 또는 1 → 일시불 처리
    │    │
    │    ├─ 2~12 → TTS 확인 (state=3)
    │    │
    │    └─ 12 초과 또는 2 미만 → 오류 안내 후 재입력
    │
    ├─[state=3] TTS 할부개월수 확인
    │           "고객님께서 요청하신 할부개월수는 X개월입니다"
    │
    ├─[state=4] 확인 입력 (1:예, 2:아니오)
    │    │
    │    ├─ '1' (예)
    │    │   → AllatPaymemt_host(90) 호출 → ALLAT_payARS(0)
    │    │
    │    └─ '2' (아니오) → ALLAT_InstallmentCConfrim(0) (재입력)
    │
    ├─[state=9] 일시불 자동 처리 후 결제
    │           → AllatPaymemt_host(90)
    │
    └─ 다음: ALLAT_payARS
```

### ALLAT_CardPw - 카드 비밀번호 입력 (Line 671)

```
ALLAT_CardPw
    │
    ├─[state=0] 비밀번호 앞 2자리 입력 안내
    │           "카드 비밀번호 4자리 중 앞 2자리를 입력"
    │
    ├─[state=1] 입력 처리
    │           유효성 검사: 숫자 0-9만 허용
    │           → AllatPaymemt_host(90) 직접 호출
    │
    └─ 다음: ALLAT_payARS

    ★ 참고: Quick 버전에서는 올앳 요청으로 비밀번호 입력을 받지 않음
            코드는 존재하나 실제 흐름에서 호출되지 않음
```

### ALLAT_payARS - 결제 후처리 (Line 431)

```
ALLAT_payARS
    │
    ├─[state=0] 결제 연동 결과 확인
    │    │
    │    ├─ m_PaySysCd < 0 → 시스템 장애 → 오류 멘트 → 종료
    │    │
    │    └─ URL_YN == "Y" → 외부 URL 콜백 처리
    │        → CreateAg() (응답 URL 생성)
    │        → AllatPayRetPrc_host(92) (결과 전송)
    │
    ├─[state=70] 로그 저장
    │           → setPayLog_host(92)
    │
    ├─[state=1] 로그 저장 결과 확인
    │    │
    │    ├─ m_PayResult < 0 (DB 시스템 장애)
    │    │   → 결제 성공(RESULTCODE==0000)시 자동 취소
    │    │   → AllatPayCancle_host(92) → state=90
    │    │
    │    ├─ m_PayResult == 0 (로그 저장 실패)
    │    │   → 결제 성공시 자동 취소 → state=91
    │    │
    │    └─ m_PayResult > 0 (성공)
    │         │
    │         ├─ 결제 실패 (RESULTCODE != "0000")
    │         │   → TTS 실패 사유 안내 → state=91
    │         │
    │         └─ 결제 성공
    │             → upOrderPayState_host(92) (주문상태 변경) → state=80
    │
    ├─[state=80] 주문상태 변경 결과 확인
    │    │
    │    ├─ m_PayResult < 0 (시스템 장애)
    │    │   → 자동 취소 → state=90
    │    │
    │    ├─ m_PayResult == 0 (변경 실패)
    │    │   → 자동 취소 → state=91
    │    │
    │    └─ m_PayResult > 0 (성공)
    │        → "결제가 완료되었습니다" → ALLAT_ExitSvc(0)
    │
    ├─[state=90] 시스템 장애 후 종료
    │           → 오류 멘트 → ALLAT_ExitSvc(0)
    │
    └─[state=91] 결제 실패/취소 후 종료
               → TTS 실패 사유 재생 → "결제 실패" → ALLAT_ExitSvc(0)
```

### ALLAT_ExitSvc - 종료 서비스 (Line 321)

```
ALLAT_ExitSvc
    │
    ├─[state=0] 정상 종료
    │           → "마지막 인사말" (service_end) → goto_hookon()
    │
    ├─[state=10] 오류 종료
    │           → "이용방법 확인" (Error_end) → goto_hookon()
    │
    └─[state=0xffff] 즉시 종료
               → goto_hookon()
```

---

## 분기 조건 상세

### 금액에 따른 할부 분기

```
m_nAmount 체크 (PARAINI의 ALLAT_MIN_AMT, 기본값 50,000원)
    │
    ├─ 금액 < 50,000원
    │   → 할부 입력 스킵
    │   → InstPeriod = "00" (일시불) 자동 설정
    │   → 바로 AllatPaymemt_host 호출
    │
    └─ 금액 >= 50,000원
        → 할부개월수 입력 진행
        → 0~12개월 선택 가능
```

### 발신자번호(ANI) 휴대폰 여부 분기 (CIA 시나리오)

```
ANI 검사
    │
    ├─ 휴대폰 (010/011/012/016/017/018/019)
    │   → getTcpOrderInfo_host 호출 (TCP 소켓 조회)
    │   → ALLAT_getOrderInfo(0)
    │
    └─ 휴대폰 아님
        → ALLAT_CIA_ScenarioStart(11) (전화번호 수동 입력)
```

### 주문정보 조회 결과 분기

```
주문정보 조회 결과
    │
    ├─ m_DBAccess == -1 (DB 접속 실패)
    │   → 시스템 장애 → 종료
    │
    ├─ m_bDnisInfo == -1 (조회 오류)
    │   → 시스템 장애 → 종료
    │
    ├─ m_bDnisInfo < 0 (TCP 통신 오류)
    │   → "Tcp_Error" 안내 → 종료
    │
    ├─ m_bDnisInfo == 0 (주문정보 없음)
    │   → "주문이 접수되지 않았습니다" → 종료
    │
    ├─ m_szretval == "6020" (와우캐시 결제 완료)
    │   → "와우캐시로 정상 결제되었습니다" → 종료
    │
    └─ 주문정보 있음
        → 정보제공 동의 안내 진행
```

### 자동 취소 조건

```
자동 취소 실행 조건
    │
    ├─ 결제 성공(RESULTCODE=="0000") 후 로그 저장 실패
    │   → AllatPayCancle_host 호출
    │
    ├─ 결제 성공 후 로그 DB 시스템 장애
    │   → AllatPayCancle_host 호출
    │
    └─ 결제 성공 후 주문상태 변경 실패
        → AllatPayCancle_host 호출
```

---

## 결제 API 함수

### ALLAT_Access.cpp 주요 함수

| 함수명 | 용도 | 설명 |
|--------|------|------|
| `AllatPaymemt_host` | 일반 신용카드 결제 | 카드번호+유효기간+할부로 결제 승인 요청 |
| `AllatPayCancle_host` | 결제 취소 | 로그 저장 실패, 주문상태 변경 실패시 자동 취소 |
| `AllatPayRetPrc_host` | 결제 결과 URL 콜백 | URL_YN=="Y"인 경우 외부 URL로 결과 전송 |

### 결제 요청 파라미터 (AllatArsPayProcess)

```
필수 항목:
- allat_card_no: 카드번호 (최대 16자)
- allat_cardvalid_ym: 유효기간 (YYMM, 4자)
- allat_sell_mm: 할부개월 (2자)
- allat_amt: 결제금액 (최대 10자)
- allat_shop_id: 상점ID (최대 20자)
- allat_order_no: 주문번호 (최대 80자)
- allat_product_nm: 상품명 (최대 1000자)

선택 항목:
- allat_passwd_no: 비밀번호 앞 2자리
- allat_registry_no: 주민번호 또는 생년월일
- allat_biz_no: 사업자번호 (법인인 경우)
- allat_business_type: 개인(0)/법인(1)
```

---

## 외부 통신 함수

### WowTvSocket.cpp / ADODB.cpp 주요 함수

| 함수명 | 용도 | 사용 시나리오 |
|--------|------|--------------|
| `getOrderInfo_host` | 전화번호로 주문정보 조회 | ARS, CID |
| `getSMSOrderInfo_host` | SMS 주문번호로 주문정보 조회 | SMS |
| `getTcpOrderInfo_host` | TCP 소켓으로 주문정보 조회 | CIA |
| `setPayLog_host` | 결제 로그 저장 | 모든 시나리오 |
| `upOrderPayState_host` | 주문 결제상태 변경 | 결제 성공시 |

---

## Billkey 버전과의 주요 차이점

### 기능 비교표

| 기능 | Quick 버전 | Billkey 버전 |
|------|-----------|-------------|
| 일반 신용카드 결제 | O | O |
| 빌키(간편결제) 결제 | X | O |
| 빌키 채번(발급) | X | O |
| 빌키 해지 | X | O |
| 정기결제 상태 분기 | X | O |
| 간편결제 동의 플로우 | X | O |
| TCP 소켓 주문 조회 | O (CIA) | O (CIA) |
| 비밀번호 입력 | 코드 존재하나 미사용 | 구인증시 사용 |
| 생년월일 입력 | 선택적 | 필수 (빌키 결제시) |

### 미사용 함수 (Billkey 버전에만 존재)

- `AllatFixPaymemt_host` - 빌키(간편결제) 결제
- `Allat_Get_FixKey_host` - 빌키 채번(발급)
- `AllatPayFixKeyCancle_host` - 빌키 해지
- `bill_delTcp_host` - 빌키 삭제 (외부 서버 동기화)
- `ALLAT_consent` - 간편결제 동의 처리

### 플로우 차이점

```
Quick 버전 (일반결제 전용):
주문조회 → 동의 → 카드번호 → 유효기간 → 할부 → 결제 → 완료

Billkey 버전 (간편결제 포함):
주문조회 → 정기결제 상태 확인 → 동의
    ├─ 빌키 있음 → 생년월일 검증 → 빌키결제 → 완료
    └─ 빌키 없음 → 카드입력 → 유효기간 → 생년월일 → 할부
                   → 비밀번호(구인증) → 동의 → 빌키채번 → 빌키결제 → 완료
```

### INI 설정 파일

- Quick 버전: `AllatWowTvQuickPay_para.ini`
- Billkey 버전: `AllatWowTvBillkeyEasyPay_para.ini`

---

## 핵심 흐름 요약

```
전화 착신
    │
    ▼
시나리오 진입 (ARS/SMS/CID/CIA)
    │
    ├─ ARS: 전화번호 입력 → getOrderInfo_host
    ├─ SMS: 주문번호 입력 → getSMSOrderInfo_host
    ├─ CID: ANI 자동 사용 → getOrderInfo_host
    └─ CIA: ANI 확인 → getTcpOrderInfo_host
    │
    ▼
주문정보 조회 결과 확인
    │
    ├─ 주문 없음/시스템 오류 → 종료
    ├─ 와우캐시 결제 완료 → 안내 후 종료
    └─ 주문 있음 → 정보제공 동의 진행
    │
    ▼
정보제공 동의 (파트너에게 정보 제공)
    │
    ├─ 미동의 → 시나리오 시작점 복귀
    └─ 동의 → 상품정보 및 해지 안내
    │
    ▼
상품정보/해지 안내 TTS
    │
    ├─ 취소 → 시나리오 시작점 복귀
    └─ 동의 → 카드 입력 진행
    │
    ▼
카드정보 입력 (일반결제)
    │
    ├─ 카드번호 (13~16자리)
    ├─ 유효기간 (MMYY)
    └─ 할부개월 (5만원 이상시)
    │
    ▼
결제 연동
    │
    └─ AllatPaymemt_host (일반 신용카드 결제)
    │
    ▼
결제 후처리
    │
    ├─ 외부 콜백 (URL_YN=Y)
    ├─ 로그 저장 (실패시 자동 취소)
    └─ 주문상태 변경 (실패시 자동 취소)
    │
    ▼
종료 안내
    │
    ├─ 성공: "결제가 완료되었습니다"
    └─ 실패: TTS 실패 사유 안내
```

---

## 참고사항

### 설정 파일 (PARAINI)
- 경로: `.\AllatWowTvQuickPay_para.ini`
- 주요 설정:
  - `[ALLAT_PAYMEMT] ALLAT_MIN_AMT`: 할부 최소 금액 (기본값: 50,000원)
  - `[RETRY] MAXCOUNT`: 재시도 최대 횟수

### 음성 파일 경로
- 인사말: `audio\shop_intro\wownet_intro`
- 공통 안내: `ment\_common\common_audio\*`
- 할부 안내: `ment\ALLAT_Hangung\input_halbu_start_ilsibul`
- 카드번호 입력: `ment\_common\common_audio\input_card_num_var_nonghyup` (농협 불가 안내 포함)

### 무이자 할부 카드사 안내 (2025년 12월 기준)
- 신한, 삼성, 국민, 현대카드: 최대 3개월
- 하나카드: 최대 4개월
- NH농협카드: 결제 불가
- 우리카드: BC카드와 별도 정책

# ALLAT StockWin Billkey Easy New Scenario 시나리오 흐름 분석

## 목차
1. [개요](#개요)
2. [시나리오 진입점](#시나리오-진입점)
3. [인증 타입 분류](#인증-타입-분류)
4. [정기결제 상태 분류](#정기결제-상태-분류)
5. [전체 시나리오 흐름도](#전체-시나리오-흐름도)
6. [세부 함수별 흐름](#세부-함수별-흐름)
7. [분기 조건 상세](#분기-조건-상세)

---

## 개요

이 DLL은 한국경제TV(WowTV) ALLAT 결제 연동 ARS 시나리오를 처리합니다. ISDN PRI E1 회선 기반 ARS 시스템에서 신용카드 결제(일반결제/간편결제/빌키 결제/정기결제)를 처리합니다.

### 핵심 컴포넌트
- **ALLAT_StockWin_Billkey_Easy_New_Scenario.cpp**: ARS 시나리오 메인 로직
- **ALLAT_Access.cpp**: ALLAT PG사 결제 API 호출 (승인/취소/빌키 발급)
- **WowTvSocket.cpp**: 외부 서버와 TCP 소켓 통신 (주문 정보 조회)
- **ADODB.cpp**: MS SQL Server ADO 연결 및 저장 프로시저 호출

---

## 시나리오 진입점

### `jobArs()` - 모든 시나리오의 시작점 (Line 3460)

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

## 인증 타입 분류

### ALLATCommom.h에 정의된 인증 타입 코드

| 코드 | 상수명 | 설명 | 입력 순서 |
|------|--------|------|-----------|
| 01 | CDAUTH_EXP | 비인증 | 카드번호 → 유효기간 → 할부 → 결제 |
| 02 | CDAUTH_ALL | 구인증 | 카드번호 → 유효기간 → 생년월일 → 할부 → 비밀번호 → 결제 |
| 03 | CDAUTH_SNN | 부분인증 | 카드번호 → 유효기간 → 생년월일 → 할부 → 결제 |
| 41 | CDAUTH_EXP_BILL | 빌키 간편결제 (비인증) | 체번: 카드→유효기간→할부→동의→체번 / 결제: 생년월일→결제 |
| 42 | CDAUTH_ALL_BILL | 빌키 간편결제 (구인증) | 체번: 카드→유효기간→생년월일→할부→비밀번호→동의→체번 / 결제: 생년월일→결제 |
| 43 | CDAUTH_SNN_BILL | 빌키 간편결제 (부분인증) | 체번: 카드→유효기간→생년월일→할부→동의→체번 / 결제: 생년월일→결제 |

---

## 정기결제 상태 분류

### `m_szsub_status` 값에 따른 분기

| 값 | 상태 | 처리 방식 |
|----|------|-----------|
| "0" | 첫 결제 | 정기결제 동의 멘트 후 카드 입력 진행 |
| "1" | 이용 중 | "이미 등록되어 있습니다" 안내 후 종료 |
| "2" | 해지 후 재결제 | 무료체험 최초 1회 안내 후 카드 입력 진행 |

### `m_szsub_has_trial` - 체험 상품 유무

| 값 | 의미 | 멘트 |
|----|------|------|
| "Y" | 체험상품 있음 | "무료체험 기간동안 언제든지 해지하셔도 이용요금은 부과되지 않습니다" |
| "N" | 체험상품 없음 | "결제 후에는 서비스에 가입되며..." |

---

## 전체 시나리오 흐름도

### 1. ARS 시나리오 (전화번호 직접 입력)

```
ALLAT_ArsScenarioStart
    │
    ├─[state=0] 인사말 재생
    │
    ├─[state=1] 전화번호 입력 안내
    │           "전화번호 입력"
    │
    ├─[state=2] 전화번호 입력 처리
    │           유효성 검사: 010/011/012/016/017/018/019 시작
    │
    ├─[state=3] TTS 전화번호 확인
    │           "고객님께서 누르신 전화번호는 XXX번입니다"
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
ALLAT_SMSScenarioStart
    │
    ├─[state=0] 인사말 재생
    │
    ├─[state=1] SMS 주문번호 입력 안내
    │           "SMS로 받은 주문번호 입력"
    │
    ├─[state=2] 주문번호 입력 처리 (6자리)
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
ALLAT_CID_ScenarioStart
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
ALLAT_CIA_ScenarioStart
    │
    ├─[state=0] 인사말 재생
    │
    ├─[state=1] ANI 휴대폰 여부 확인
    │    │
    │    ├─ 휴대폰(010/011 등) → getTcpOrderInfo_host(90)
    │    │
    │    └─ 휴대폰 아님 → ALLAT_CIA_ScenarioStart(11) (번호 입력)
    │
    ├─[state=11~14] 휴대폰 번호 수동 입력 및 확인
    │
    └─ 다음: ALLAT_getOrderInfo(0)
```

---

## 세부 함수별 흐름

### ALLAT_getOrderInfo - 주문정보 확인 및 동의 처리 (Line 2224)

```
ALLAT_getOrderInfo
    │
    ├─[state=0] 주문정보 조회 결과 확인
    │    │
    │    ├─ m_DBAccess == -1 또는 m_bDnisInfo == -1
    │    │   → 시스템 장애 → ALLAT_ExitSvc(0)
    │    │
    │    ├─ m_bDnisInfo == 0
    │    │   → 주문정보 없음 → "주문이 접수되지 않았습니다" → ALLAT_ExitSvc(0)
    │    │
    │    ├─ m_szretval == "6020"
    │    │   → 와우캐시 결제 완료 → TTS 안내 → ALLAT_ExitSvc(0)
    │    │
    │    ├─ m_szrenew_flag == "Y" (기존 동의 고객)
    │    │   → 동의 멘트 스킵 → state=9
    │    │
    │    └─ 동의 안내 TTS 재생
    │        (파트너 정보 제공 동의)
    │
    ├─[state=1~2] 동의 확인
    │    │
    │    ├─ '1' (동의) → state=9
    │    │
    │    └─ '2' (미동의) → 시나리오 시작점으로 복귀
    │
    ├─[state=9] 정기결제 상태 분기 ★ 중요 분기점
    │    │
    │    ├─ m_szsub_status == "0" (첫 결제)
    │    │    │
    │    │    ├─ m_szsub_has_trial == "N" (무료체험 없음)
    │    │    │   → 정기결제 동의 멘트 (결제금액 안내)
    │    │    │
    │    │    └─ m_szsub_has_trial == "Y" (무료체험 있음)
    │    │        → 무료체험 안내 멘트
    │    │        → "무료 체험 만료일에 정기 결제됩니다"
    │    │
    │    ├─ m_szsub_status == "1" (이용 중)
    │    │   → "이미 등록되어 있습니다" → ALLAT_ExitSvc(0)
    │    │
    │    └─ m_szsub_status == "2" (해지 후 재결제)
    │         │
    │         ├─ expire_date 없음 → 재가입 가능 → 동의 멘트
    │         │
    │         └─ expire_date 있음 → 잔여일 안내 (현재는 재가입 가능으로 변경됨)
    │
    ├─[state=10] 상품정보 TTS 안내
    │
    ├─[state=12] 동의/취소 확인 (1:동의, 2:취소)
    │    │
    │    ├─ '1' (동의) → ALLAT_CardInput(0) (카드입력)
    │    │              ★ 정기결제는 무조건 카드번호 입력
    │    │
    │    └─ '2' (취소) → 시나리오 시작점 복귀
    │
    ├─[state=3~4] 간편결제 메뉴 (빌키 있는 경우)
    │    │
    │    ├─ '1' 등록카드로 결제 → ALLAT_InstallmentCConfrim(0)
    │    │
    │    ├─ '2' 다른 카드로 결제 → 빌키 해제 후 ALLAT_CardInput(0)
    │    │
    │    └─ '3' 간편결제 해지 → AllatPayFixKeyCancle → ALLAT_ExitSvc(0)
    │
    └─[state=5~7] 간편결제 해지 후 처리
```

### ALLAT_CardInput - 카드번호 입력 (Line 2102)

```
ALLAT_CardInput
    │
    ├─[state=0] 카드번호 입력 안내
    │           "카드번호 입력 후 우물정자"
    │
    ├─[state=1] 카드번호 입력 (13~16자리)
    │           유효성 검사
    │
    ├─[state=2] TTS 카드번호 확인
    │
    ├─[state=3] 확인 입력
    │    │
    │    ├─ '1' (예) → ALLAT_EffecDate(0) (유효기간 입력)
    │    │
    │    └─ '2' (아니오) → ALLAT_CardInput(0) (재입력)
    │
    └─ 다음: ALLAT_EffecDate
```

### ALLAT_EffecDate - 유효기간 입력 (Line 1942)

```
ALLAT_EffecDate
    │
    ├─[state=0] 유효기간 입력 안내
    │           "월/연 순서로 4자리 입력"
    │
    ├─[state=1] 유효기간 입력 및 유효성 검사
    │           - 월: 1~12
    │           - 년: 현재년도 이상
    │           - 현재월 이후
    │
    ├─[state=2] TTS 유효기간 확인
    │
    ├─[state=3] 확인 입력
    │    │
    │    ├─ '1' (예)
    │    │    │
    │    │    ├─ AUTH_TYPE = CDAUTH_EXP 또는 CDAUTH_EXP_BILL
    │    │    │   → ALLAT_InstallmentCConfrim(0) (할부 입력)
    │    │    │
    │    │    └─ 기타 (생년월일 필요)
    │    │        → ALLAT_JuminNo(0) (생년월일 입력)
    │    │
    │    └─ '2' (아니오) → ALLAT_EffecDate(0) (재입력)
    │
    └─ 다음: AUTH_TYPE에 따라 분기
```

### ALLAT_JuminNo - 생년월일/법인번호 입력 (Line 762)

```
ALLAT_JuminNo
    │
    ├─[state=0] 생년월일 입력 안내
    │    │
    │    ├─ CDAUTH_EXP_BILL (빌키 비인증)
    │    │    ├─ 빌키 없음 → 빌키 생성 전 생년월일 입력
    │    │    └─ 빌키 있음 → 결제용 생년월일 확인
    │    │
    │    ├─ CDAUTH_ALL_BILL (빌키 구인증)
    │    │    ├─ 빌키 없음 → 빌키 생성 전 생년월일 입력
    │    │    └─ 빌키 있음 → 결제용 생년월일 확인
    │    │
    │    └─ CDAUTH_SNN_BILL (빌키 부분인증)
    │         ├─ 빌키 없음 → 빌키 생성 전 생년월일 입력
    │         └─ 빌키 있음 → 결제용 생년월일 확인
    │
    ├─[state=1] 입력 처리 (6자리 또는 10자리)
    │           SHA256 해시 암호화 저장
    │
    ├─[state=2] TTS 확인
    │
    ├─[state=3] 확인 입력
    │    │
    │    ├─ '1' (예)
    │    │    │
    │    │    ├─ CDAUTH_ALL 또는 CDAUTH_SNN
    │    │    │   → ALLAT_InstallmentCConfrim(0) (할부)
    │    │    │
    │    │    ├─ CDAUTH_ALL_BILL 또는 CDAUTH_SNN_BILL (빌키 없음)
    │    │    │   → ALLAT_InstallmentCConfrim(0) (할부)
    │    │    │
    │    │    ├─ CDAUTH_ALL_BILL 또는 CDAUTH_SNN_BILL (빌키 있음)
    │    │    │   │
    │    │    │   ├─ 재결제 → 생년월일 검증
    │    │    │   │    ├─ 일치 → AllatFixPaymemt_host (빌키 결제)
    │    │    │   │    └─ 불일치 → 재입력 (최대 횟수 초과시 종료)
    │    │    │   │
    │    │    │   └─ 최초결제 → 생년월일 검증 후 결제
    │    │    │
    │    │    └─ CDAUTH_EXP_BILL (빌키 비인증)
    │    │         ├─ 빌키 없음 → 빌키 체번 후 동의
    │    │         └─ 빌키 있음 → 생년월일 검증 후 빌키 결제
    │    │
    │    └─ '2' (아니오) → ALLAT_JuminNo(0) (재입력)
    │
    ├─[state=4] 빌키 결제 처리 ★ 0원 결제 분기
    │    │
    │    ├─ m_nAmount > 0 → AllatFixPaymemt_host (빌키 결제)
    │    │
    │    └─ m_nAmount == 0 → 직접 ALLAT_payARS(0) (0원 결제)
    │        ★ PG사 승인요청 없이 결제완료 처리
    │
    └─ 다음: 분기에 따라
```

### ALLAT_InstallmentCConfrim - 할부개월수 입력 (Line 1466)

```
ALLAT_InstallmentCConfrim
    │
    ├─[state=0] 할부개월수 입력 또는 자동 설정
    │    │
    │    ├─ 금액 < 50,000원
    │    │   → 할부 0개월(일시불) 자동 설정
    │    │   → AUTH_TYPE에 따라 다음 단계로
    │    │
    │    └─ 금액 >= 50,000원
    │        → 할부개월수 입력 안내
    │        "0:일시불, 2~12:할부"
    │
    ├─[state=1] 할부개월수 입력 처리
    │    │
    │    ├─ 0 또는 1 → 일시불
    │    │
    │    ├─ 2~12 → TTS 확인
    │    │
    │    └─ 12 초과 또는 2 미만 → 재입력
    │
    ├─[state=3] TTS 확인
    │
    ├─[state=4] 확인 입력
    │    │
    │    ├─ '1' (예) → AUTH_TYPE에 따라 분기
    │    │    │
    │    │    ├─ CDAUTH_EXP_BILL (빌키 없음)
    │    │    │   → ALLAT_consent(0) (동의)
    │    │    │
    │    │    ├─ CDAUTH_EXP_BILL (빌키 있음)
    │    │    │   → ALLAT_JuminNo(0) (생년월일)
    │    │    │
    │    │    ├─ CDAUTH_EXP
    │    │    │   → AllatPaymemt_host (일반 결제)
    │    │    │
    │    │    ├─ CDAUTH_SNN
    │    │    │   ├─ 생년월일 있음 → 결제
    │    │    │   └─ 생년월일 없음 → ALLAT_JuminNo(0)
    │    │    │
    │    │    ├─ CDAUTH_ALL_BILL (빌키 없음)
    │    │    │   → ALLAT_CardPw(0) (비밀번호)
    │    │    │
    │    │    ├─ CDAUTH_SNN_BILL (빌키 없음)
    │    │    │   → ALLAT_consent(0) (동의)
    │    │    │
    │    │    └─ CDAUTH_ALL
    │    │        → ALLAT_CardPw(0) (비밀번호)
    │    │
    │    └─ '2' (아니오) → ALLAT_InstallmentCConfrim(0) (재입력)
    │
    └─ 다음: AUTH_TYPE에 따라
```

### ALLAT_CardPw - 카드 비밀번호 입력 (Line 702)

```
ALLAT_CardPw
    │
    ├─[state=0] 비밀번호 앞 2자리 입력 안내
    │
    ├─[state=1] 입력 처리
    │    │
    │    ├─ CDAUTH_ALL_BILL (빌키 없음)
    │    │   → ALLAT_consent(0) (동의)
    │    │
    │    └─ 기타
    │        → AllatPaymemt_host (결제)
    │
    └─ 다음: ALLAT_payARS
```

### ALLAT_consent - 간편결제 동의 (Line 1282)

```
ALLAT_consent
    │
    ├─[state=0] 간편결제 동의 안내
    │           "동의하시면 1번, 일반결제 2번"
    │
    ├─[state=1] 동의 확인
    │    │
    │    ├─ '1' (동의)
    │    │    │
    │    │    ├─ CDAUTH_EXP_BILL (빌키 없음)
    │    │    │   → ALLAT_JuminNo(0) (생년월일 입력 후 빌키 채번)
    │    │    │
    │    │    ├─ CDAUTH_ALL_BILL (빌키 없음)
    │    │    │   → Allat_Get_FixKey_host (빌키 채번)
    │    │    │
    │    │    └─ CDAUTH_SNN_BILL (빌키 없음)
    │    │        → Allat_Get_FixKey_host (빌키 채번)
    │    │
    │    └─ '2' (미동의 = 일반결제)
    │         │
    │         ├─ AUTH_TYPE → AUTH_TYPE2로 변경 (일반결제로 전환)
    │         │
    │         ├─ CDAUTH_EXP → AllatPaymemt_host (일반 결제)
    │         │
    │         ├─ CDAUTH_ALL/SNN → ALLAT_InstallmentCConfrim(0)
    │         │
    │         └─ 기타 → 종료 (정기결제 미동의시)
    │
    ├─[state=2] 빌키 채번 결과 확인
    │    │
    │    ├─ m_PaySysCd != 1 → 채번 실패 → 종료
    │    │
    │    ├─ 빌키 없음 → TTS 채번 실패 안내
    │    │
    │    └─ 빌키 있음 → ALLAT_JuminNo(4) (생년월일 검증 후 결제)
    │
    └─[state=3~4] 채번 실패 후 처리 → 종료
```

### ALLAT_payARS - 결제 후처리 (Line 462)

```
ALLAT_payARS
    │
    ├─[state=0] 결제 연동 결과 확인
    │    │
    │    ├─ m_PaySysCd < 0 → 시스템 장애 → 종료
    │    │
    │    └─ URL_YN == "Y" → 외부 URL 콜백 처리
    │        → CreateAg() (응답 URL 생성)
    │        → AllatPayRetPrc_host (결과 전송)
    │
    ├─[state=70] 로그 저장
    │
    ├─[state=1] 로그 저장 결과 확인
    │    │
    │    ├─ m_PayResult < 0 (DB 장애)
    │    │   → 결제 성공시 자동 취소 → AllatPayCancle_host
    │    │
    │    ├─ m_PayResult == 0 (로그 저장 실패)
    │    │   → 결제 성공시 자동 취소
    │    │
    │    └─ m_PayResult > 0 (성공)
    │         │
    │         ├─ 결제 실패 (RESULTCODE != "0000")
    │         │   → TTS 실패 사유 안내 → 종료
    │         │
    │         └─ 결제 성공
    │             → upOrderPayState_host (주문상태 변경)
    │
    ├─[state=80] 주문상태 변경 결과
    │    │
    │    ├─ 실패 → 자동 취소
    │    │
    │    └─ 성공 → "결제가 완료되었습니다" → 종료
    │
    └─[state=90~91] 오류 처리 후 종료
```

---

## 분기 조건 상세

### 빌키(간편결제) 유무에 따른 분기

```
m_szbill_key 체크
    │
    ├─ strlen(m_szbill_key) < 1 (빌키 없음)
    │    │
    │    ├─ 최초 등록 → 카드정보 입력 → 동의 → 빌키 채번 → 결제
    │    │
    │    └─ 일반 결제 선택 → 카드정보 입력 → 결제
    │
    └─ strlen(m_szbill_key) > 0 (빌키 있음)
         │
         ├─ 재결제 (m_bPayFlag == TRUE)
         │   → 생년월일 검증 → AllatFixPaymemt_host (빌키 결제)
         │
         └─ 최초 결제 (m_bPayFlag == FALSE)
             → 생년월일 검증 → AllatFixPaymemt_host (빌키 결제)
```

### 무료상품(0원 결제)에 따른 분기

```
m_nAmount 체크
    │
    ├─ m_nAmount > 0 (유료)
    │   → PG사 결제 승인 요청 필수
    │   → AllatPaymemt_host 또는 AllatFixPaymemt_host
    │
    └─ m_nAmount == 0 (무료/체험)
        │
        ├─ 빌키 채번만 수행
        │
        └─ PG사 승인요청 없이 ALLAT_payARS(0)로 직접 이동
            ★ CreateAg()에서 주문번호, 상점아이디 강제 설정
```

### 금액에 따른 할부 분기

```
m_nAmount 체크 (PARAINI의 ALLAT_MIN_AMT, 기본값 50,000원)
    │
    ├─ 금액 < 50,000원
    │   → 할부 입력 스킵
    │   → InstPeriod = "00" (일시불) 자동 설정
    │
    └─ 금액 >= 50,000원
        → 할부개월수 입력 진행
        → 0~12개월 선택 가능
```

### 생년월일 검증 분기 (재결제시)

```
생년월일 검증 (SHA256 해시 비교)
    │
    ├─ 입력값 해시 == m_szext_data (일치)
    │   → 결제 진행
    │
    └─ 불일치
        │
        ├─ m_nRetryCnt < m_nRetryMaxCnt
        │   → "생년월일이 일치하지 않습니다" → 재입력
        │
        └─ m_nRetryCnt >= m_nRetryMaxCnt
            → 종료
```

### DNIS(착신번호)별 특수 멘트 분기

| DNIS | 서비스 | 특이사항 |
|------|--------|----------|
| 4542, 6617 | 와우 글로벌 파트너스 | 정보제공 동의 멘트 커스텀 |
| 6625 | 와우 아카데미 | 교육상품 환불 규정 안내 |
| 5037 | 정기결제 | 5만5천원 고정 안내 |
| 5013 | 교육특강 | 44만원 고정, 환불 불가 안내 |
| 5039, 4618, 5018, 4577, 4639 | 예약결제 | 해지수수료 안내 |
| 4479, 4649, 4542, 5094, 4527, 4609 | 예약결제 | 환불 불가 안내 |

---

## 결제 API 함수 (ALLAT_Access.cpp)

| 함수명 | 용도 | 호출 위치 |
|--------|------|-----------|
| `AllatPaymemt_host` | 일반 신용카드 결제 | CDAUTH_EXP, CDAUTH_ALL, CDAUTH_SNN |
| `AllatFixPaymemt_host` | 빌키(간편결제) 결제 | CDAUTH_EXP_BILL, CDAUTH_ALL_BILL, CDAUTH_SNN_BILL |
| `AllatPayCancle_host` | 결제 취소 | 로그 저장 실패, 주문상태 변경 실패시 |
| `Allat_Get_FixKey_host` | 빌키 채번(발급) | 간편결제 동의 후 |
| `AllatPayFixKeyCancle_host` | 빌키 해지 | 간편결제 갱신/해지 선택시 |

---

## 외부 통신 함수 (WowTvSocket.cpp)

| 함수명 | 용도 |
|--------|------|
| `getOrderInfo_host` | 전화번호로 주문정보 조회 (ARS/CID) |
| `getSMSOrderInfo_host` | SMS 주문번호로 주문정보 조회 |
| `getTcpOrderInfo_host` | TCP 소켓으로 주문정보 조회 (CIA) |
| `setPayLog_host` | 결제 로그 저장 |
| `upOrderPayState_host` | 주문 결제상태 변경 |
| `bill_delTcp_host` | 빌키 삭제 (외부 서버 동기화) |
| `AllatPayRetPrc_host` | 결제 결과 외부 URL 콜백 |

---

## 오류 처리 및 종료

### 종료 함수 `ALLAT_ExitSvc` (Line 328)

| state | 동작 |
|-------|------|
| 0 | 정상 종료 멘트 → "마지막 인사말" |
| 10 | 오류 종료 → "이용방법 확인" |
| 0xffff | 즉시 종료 (goto_hookon) |

### 재시도 카운터

- `m_nRetryMaxCnt`: INI 파일에서 설정 (RETRY/MAXCOUNT)
- `m_nRetryCnt`: 현재 재시도 횟수
- 초과시 `ALLAT_ExitSvc(10)` 호출

---

## 핵심 흐름 요약

```
전화 착신
    │
    ▼
시나리오 진입 (ARS/SMS/CID/CIA)
    │
    ▼
주문정보 조회 (TCP/DB)
    │
    ├─ 주문 없음 → 종료
    │
    ├─ 와우캐시 결제 완료 → 안내 후 종료
    │
    ├─ 정보제공 동의 (신규 고객)
    │
    ▼
정기결제 상태 확인
    │
    ├─ 첫결제/해지후재결제 → 동의 멘트 → 카드 입력 진행
    │
    └─ 이용중 → "이미 등록됨" → 종료
    │
    ▼
카드정보 입력
    │
    ├─ 카드번호 (13~16자리)
    ├─ 유효기간 (MMYY)
    ├─ 생년월일 (필요시)
    ├─ 할부개월 (5만원 이상)
    └─ 비밀번호 (구인증)
    │
    ▼
결제 타입 분기
    │
    ├─ 일반결제 → AllatPaymemt_host
    │
    ├─ 빌키 없음 → 동의 → 빌키채번 → 빌키결제
    │
    └─ 빌키 있음 → 생년월일 검증 → AllatFixPaymemt_host
    │
    ▼
결제 후처리
    │
    ├─ 로그 저장
    ├─ 주문상태 변경
    ├─ 외부 콜백 (URL_YN=Y)
    │
    ▼
종료 안내
```

# 일반결제 시나리오 Migration 설계 계획서

## 참고 문서와 코드 파일
- 기존 시나리오 코드 : ALLAT_Stockwin_Quick_New_Scenario
- 기존 시나리오 문서 : QUICK_ANALYZE_FLOW.md
- 새로운 시나리오 문서 : NEW_SPEC_SCENARIO.md
- 새로운 시나리오에 적용되어야 할 API 문서 : NEW_SPEC_PL_API.md

## 요구사항
- 기존 시나리오 코드에 새로운 시나리오가 적용되도록 코드를 수정해야 한다.
- 기존의 코드에서 사용하고 있는 상품정보,주문정보를 가져와서 저장, 사용하는 부분(api포함)을 분석해야 한다.
- 기존 API를 새로운 API 로 대체해야 한다.
- 새로운 API로 대체하기 위한 설계를 해야 하며 문제가 없는 지 검토해야 한다.
- 각 API의 파라미터가 어떻게 매칭이 되어서 사용되어야 하는지 검토해서 알려주어야 한다.
- 구현하기 위한 설계를 해서 지금 현 문서에 업데이트 해라.
- 설계와 구현 단계를 구분해서 문서를 작성하고 구현 부분은 각 Phase별로 기술하며 각 단계별 체크박스를 두어서 나중에 니게 구현을 할 때 구현된 부분은 체크해서 상태 관리를 한다.

---

## 1. 개요

### 1.1 목적
ALLAT_Stockwin_Quick_New_Scenario의 기존 ASP/XML 기반 API를 새로운 RESTful JSON API로 마이그레이션

### 1.2 범위
- 일반결제 시나리오 (Quick Scenario)만 대상
- 정기결제/빌키 결제는 별도 프로젝트로 분리

### 1.3 핵심 변경사항
| 항목 | 기존 | 신규 |
|------|------|------|
| 도메인 | `billadmin.wownet.co.kr` | `swbillapi.wowtv.co.kr` |
| 프로토콜 | ASP 페이지 호출 | RESTful API |
| 데이터 형식 | XML | JSON |
| 인증 | Query Parameter | HMAC-SHA256 Authorization |

---

## 2. API 매핑 설계

### 2.1 기능별 API 매핑

| 기능 | 기존 API | 신규 API | 비고 |
|------|----------|----------|------|
| 주문정보 조회 | `getOrderInfo_host()` | `POST /v1/payment/simple/getpaymentinfo_V2` | 핵심 |
| 주문정보 조회 (SMS) | `getSMSOrderInfo_host()` | `POST /v1/payment/simple/getpaymentinfo_V2` | 동일 엔드포인트 |
| 주문정보 조회 (TCP) | `getTcpOrderInfo_host()` | `POST /v1/payment/simple/getpaymentinfo_V2` | 동일 엔드포인트 |
| 결제 동의 등록 | (없음) | `POST /v1/payment/simple/agree` | 신규 추가 |
| 결제 처리 | `AllatPaymemt_host()` | 기존 ALLAT 유지 | 변경 없음 |

#### 2.1.1 시나리오별 API 호출 매핑

| 시나리오 | 기존 함수 | 신규 API | reqType | 비고 |
|----------|-----------|----------|---------|------|
| ARS | `getOrderInfo_host()` | `/v1/payment/simple/getpaymentinfo_V2` | 1 | 회선번호(DNIS)로 조회 |
| SMS | `getSMSOrderInfo_host()` | `/v1/payment/simple/getpaymentinfo_V2` | 1 | 회선번호(DNIS)로 조회 |
| CID | `getOrderInfo_host()` | `/v1/payment/simple/getpaymentinfo_V2` | 1 | 회선번호(DNIS)로 조회 |
| CIA | `getTcpOrderInfo_host()` | `/v1/payment/simple/getpaymentinfo_V2` | 1 | 회선번호(DNIS)로 조회 |

### 2.2 필드 매핑표 (상세)

#### 2.2.1 요청 파라미터 매핑

| 기존 요청 | 신규 요청 | 타입 | 설명 |
|-----------|-----------|------|------|
| DNIS (Query) | `reqTypeVal` | String(12) | 회선번호 (reqType=1일 때) |
| HP_NO (Query) | `phoneNo` | String(16) | 고객 휴대폰번호 |
| (없음) | `reqType` | Byte | 요청타입 (1=회선번호) |
| (없음) | `ARSType` | String | ARS구분 (VARS/ARS) |

#### 2.2.2 응답 필드 매핑 (XML → JSON)

| 기존 XML 필드 | 신규 JSON 필드 | 타입 변환 | 시나리오 변수 | 용도 |
|---------------|----------------|-----------|---------------|------|
| `retval` | `resultCode` | string→string | - | 결과코드 (0=성공) |
| `errmsg` | `error.message` | string→string | - | 오류메시지 |
| `order_no` | `orderNo` | string(70)→Int64 | `m_szMx_issue_no` | 주문번호 |
| `shop_id` | `mallIdGeneral` | string(32)→string(40) | `m_szMx_id` | 가맹점ID |
| `cc_name` | `memberId` | string(64)→string(36) | `m_szCC_name` | 회원ID |
| `cc_pord_desc` | `itemName` | string(255)→string(100) | `m_szCC_Prod_Desc` | 상품명 |
| `cc_pord_code` | `categoryId_2nd` | string(50)→string(6) | `m_szCC_Prod_Code` | 상품코드 |
| `amount` | `payAmt` | string(10)→Int64 | `m_nAmount` | 결제금액 |
| `partner_nm` | `nickName` | string(256)→string(20) | `m_szpartner_nm` | 필명 |

#### 2.2.3 신규 추가 필드

| 신규 JSON 필드 | 타입 | 신규 변수 | 용도 | TTS 사용 |
|----------------|------|-----------|------|----------|
| `purchaseAmt` | Int64 | `m_purchaseAmt` | 할인 전 원가 | O |
| `CouponUseFlag` | String(1) | `m_couponUseFlag` | 쿠폰 사용 여부 | O |
| `CouponName` | String(50) | `m_couponName` | 쿠폰명 | O |
| `BonusCashUseFlag` | String(1) | `m_bonusCashUseFlag` | 보너스캐시 사용 여부 | O |
| `BonusCashUseAmt` | Int64 | `m_bonusCashUseAmt` | 보너스캐시 금액 | O |
| `purchaseLimitFlag` | String(1) | `m_purchaseLimitFlag` | 구매 제한 상태 | - |
| `pgCode` | String(1) | `m_pgCode` | PG코드 (A/P) | - |
| `memberState` | Byte | `m_memberState` | 고객상태 | - |
| `serviceCheckFlag` | String(1) | `m_serviceCheckFlag` | 점검 여부 | - |

#### 2.2.4 DB 저장 필드 매핑

| DB 컬럼 (sp_getAllatOrderInfoByOrderNo) | 기존 소스 | 신규 소스 | 비고 |
|------------------------------------------|-----------|-----------|------|
| `MX_ISSUE_NO` | XML order_no | JSON orderNo | 주문번호 |
| `MX_ID` | XML shop_id | JSON mallIdGeneral | 가맹점ID |
| `MX_OPT` | DB 조회 | DB 조회 (기존 유지) | CrossKey |
| `MX_NAME` | DB 조회 | DB 조회 (기존 유지) | 가맹점명 |
| `CC_NAME` | XML cc_name | JSON memberId | 회원ID |
| `CC_PORD_DESC` | XML cc_pord_desc | JSON itemName | 상품명 |
| `AMOUNT` | XML amount | JSON payAmt | 결제금액 |
| `CC_EMAIL` | DB 조회 | DB 조회 (기존 유지) | 이메일 |
| `URL_YN` | DB 조회 | DB 조회 (기존 유지) | 콜백여부 |
| `SHOP_RET_URL` | DB 조회 | JSON notiUrlGeneral | 콜백URL |

#### 2.2.5 결제 API 파라미터 (변경 없음)

ALLAT 결제 API 호출 시 사용되는 파라미터는 변경 없이 기존 로직 유지:

| ALLAT 파라미터 | 소스 변수 | 설명 |
|----------------|-----------|------|
| `allat_shop_id` | `m_szMx_id` | 가맹점ID (신규 API에서 mallIdGeneral) |
| `allat_order_no` | `m_szMx_issue_no` | 주문번호 (신규 API에서 orderNo) |
| `allat_amt` | `m_nAmount` | 결제금액 (신규 API에서 payAmt) |
| `allat_product_nm` | `m_szCC_Prod_Desc` | 상품명 (신규 API에서 itemName) |
| `allat_buyer_nm` | `m_szpsrtner_nm` | 고객명/필명 (신규 API에서 nickName) |
| `allat_cross_key` | `m_szMx_opt` | CrossKey (DB 조회 - 변경 없음) |

#### 2.2.6 purchaseLimitFlag 처리

| 값 | 의미 | 처리 |
|----|------|------|
| 1 | 정상 (구매가능) | 결제 진행 |
| 2 | 불량사용자 등록 | 오류 안내 후 종료 |
| 3 | 구매 가능 횟수 초과 | 오류 안내 후 종료 |
| 4 | 판매 시작전 상품 | 오류 안내 후 종료 |
| 5 | 판매 종료 상품 | 오류 안내 후 종료 |
| 6 | 판매 중지 상품 | 오류 안내 후 종료 |

### 2.3 인증 헤더 설계

```
Authorization: PLTOKEN {APP_ID}:{Signature}:{Nonce}:{Timestamp}

# Signature 생성 로직 (NEW_SPEC_PL_API.md 기준)
RequestString = APP_ID + UpperCase(HTTP_METHOD) + Timestamp + Nonce
Signature = Base64(HMAC-SHA256(Base64Decode(APP_KEY), UTF8(RequestString)))

# 예시 (테스트 서버 기준)
APP_ID = "8c3cdc588ff746599a0beb714b4dce3a"
APP_KEY = "ljyay9tYbSSVNIQIgZpZYdzgdr7Nlg7O4hdZ+pfrElw="
METHOD = "POST"
Timestamp = "1583391560"
Nonce = "20200305571158"

RequestString = "8c3cdc588ff746599a0beb714b4dce3aPOST158339156020200305571158"
Signature = Base64(HMAC-SHA256(Base64Decode(APP_KEY), RequestString))

# 주의사항
- APP_KEY는 반드시 Base64 디코딩 후 바이트 배열로 사용
- RequestBody는 서명에 포함되지 않음
- 시간 동기화: 클라이언트-서버 시간 차이 5분 이상이면 요청 거부
- Nonce 재사용 금지: 동일 Nonce 5분간 거부
```

---

## 3. 아키텍처 설계

### 3.1 컴포넌트 구조

```
[기존 구조]
ALLAT_Stockwin_Quick_New_Scenario.cpp
    ├── WowTvSocket.cpp (ASP/XML 통신)
    ├── ADODB.cpp (DB 처리)
    └── ALLAT_Access.cpp (결제 처리)

[신규 구조]
ALLAT_Stockwin_Quick_New_Scenario.cpp
    ├── PayLetterAPI.cpp [신규] (REST/JSON 통신)
    │       ├── PL_GenerateAuthHeader()
    │       ├── PL_GetPaymentInfo()
    │       ├── PL_ParseResponse()
    │       └── PL_HandleError()
    ├── WowTvSocket.cpp [유지] (폴백용)
    ├── ADODB.cpp [유지]
    └── ALLAT_Access.cpp [유지]
```

### 3.2 데이터 흐름

```
[신규 흐름]
전화 착신 → 시나리오 시작
    ↓
PayLetterAPI::PL_GetPaymentInfo()
    ├── Authorization 헤더 생성
    ├── POST /v1/payment/simple/getpaymentinfo_V2
    └── JSON 응답 파싱
    ↓
응답 검증
    ├── resultCode == "0" → 성공
    ├── purchaseLimitFlag == "1" → 구매 가능
    └── 기타 → 에러 처리
    ↓
쿠폰/보너스캐시 정보 확인
    ├── CouponUseFlag == "Y" → 쿠폰 안내 포함
    └── BonusCashUseFlag == "Y" → 보너스캐시 안내 포함
    ↓
동적 TTS 생성 → 결제 금액 안내
    ↓
카드 정보 입력 (기존 로직 유지)
    ↓
결제 처리 (ALLAT API - 기존 유지)
    ↓
결제 완료 후처리
```

---

## 4. 구현 설계

### 4.1 신규 파일: PayLetterAPI.h

```cpp
// PayLetterAPI.h - REST API 통신 모듈

#pragma once
#include <string>

// API 응답 구조체
struct PL_PaymentInfo {
    std::string memberId;
    long long orderNo;
    std::string nickName;
    std::string itemName;
    std::string pgCode;
    std::string mallIdSimple;
    std::string mallIdGeneral;
    int payAmt;           // 실제 결제 금액
    int purchaseAmt;      // 원가
    std::string couponUseFlag;
    std::string couponName;
    std::string bonusCashUseFlag;
    int bonusCashUseAmt;
    std::string cardCompany;
    std::string purchaseLimitFlag;
    std::string resultCode;
    std::string resultMessage;
};

// API 함수 선언
bool PL_Initialize(const char* appId, const char* appKey, const char* baseUrl);
void PL_Cleanup();

std::string PL_GenerateAuthHeader();  // requestBody는 서명에 포함되지 않음
bool PL_GetPaymentInfo(int reqType, const std::string& reqTypeVal,
                       const std::string& phoneNo, const std::string& arsType,
                       PL_PaymentInfo& outInfo);
std::string PL_GetLastError();
```

### 4.2 인증 헤더 생성 로직

```cpp
std::string PL_GenerateAuthHeader() {
    // 1. Nonce 생성 (UUID 또는 타임스탬프 기반)
    std::string nonce = GenerateNonce();  // 예: "20200305571158"

    // 2. Timestamp 생성 (Unix timestamp, UTC)
    time_t timestamp = time(nullptr);
    std::string timestampStr = std::to_string(timestamp);

    // 3. 서명 데이터 구성 (NEW_SPEC_PL_API.md 규격)
    // RequestString = APP_ID + METHOD + Timestamp + Nonce
    // ※ RequestBody는 서명에 포함되지 않음!
    std::string signData = g_appId + "POST" + timestampStr + nonce;

    // 4. APP_KEY Base64 디코딩
    std::vector<unsigned char> appKeyBytes = Base64Decode(g_appKey);

    // 5. HMAC-SHA256 서명 생성
    unsigned char hash[32];
    HMAC_SHA256(appKeyBytes.data(), appKeyBytes.size(),
                (unsigned char*)signData.c_str(), signData.length(), hash);

    // 6. Base64 인코딩
    std::string signature = Base64Encode(hash, 32);

    // 7. Authorization 헤더 조립
    return "PLTOKEN " + g_appId + ":" + signature + ":" +
           nonce + ":" + timestampStr;
}
```

### 4.3 기존 함수 수정: ALLAT_getOrderInfo()

```cpp
// 기존 코드 (WowTvSocket 사용)
int CALLAT_Stockwin_Quick_Scenario::ALLAT_getOrderInfo() {
    // 기존: XML 기반 주문 조회
    m_WowTvSocket->getOrderInfo_host(...);
    // XML 파싱...
}

// 신규 코드 (PayLetterAPI 사용)
int CALLAT_Stockwin_Quick_Scenario::ALLAT_getOrderInfo() {
    PL_PaymentInfo info;

    // REST API 호출
    if (!PL_GetPaymentInfo(1, m_dnis, m_phoneNo, "ARS", info)) {
        info_printf("API 호출 실패: %s", PL_GetLastError().c_str());
        return -1;
    }

    // 결과 검증
    if (info.resultCode != "0") {
        info_printf("API 응답 오류: %s", info.resultMessage.c_str());
        return -2;
    }

    // 구매 제한 확인
    if (info.purchaseLimitFlag != "1") {
        // 구매 불가 안내
        return -3;
    }

    // 멤버 변수에 저장 (기존 변수명 유지)
    // ※ 필드 매핑: cc_name→memberId, partner_nm→nickName
    m_szMx_issue_no = std::to_string(info.orderNo);  // 주문번호
    m_szMx_id = info.mallIdGeneral;                   // 가맹점ID (일반결제)
    m_szCC_name = info.memberId;                      // 회원ID
    m_szpsrtner_nm = info.nickName;                   // 필명 (TTS 안내용)
    m_szCC_Prod_Desc = info.itemName;                 // 상품명
    m_nAmount = info.payAmt;                          // 결제금액

    // 신규 필드 저장
    m_couponUseFlag = info.couponUseFlag;
    m_couponName = info.couponName;
    m_bonusCashUseFlag = info.bonusCashUseFlag;
    m_bonusCashUseAmt = info.bonusCashUseAmt;
    m_purchaseAmt = info.purchaseAmt;

    return 0;
}
```

### 4.4 TTS 생성 로직 추가

```cpp
std::string GeneratePaymentTTS() {
    std::string tts;
    // TTS 안내 시 고객명은 nickName(필명) 사용 - m_szpsrtner_nm
    tts += m_szpsrtner_nm + " 고객님, 주문하신 " + m_szCC_Prod_Desc + "의 ";

    // 할인 정보 안내
    if (m_couponUseFlag == "Y" || m_bonusCashUseFlag == "Y") {
        tts += m_szpsrtner_nm + "께서 보유하신 ";

        if (m_couponUseFlag == "Y") {
            tts += m_couponName + " 쿠폰";
            if (m_bonusCashUseFlag == "Y") tts += "과 ";
        }

        if (m_bonusCashUseFlag == "Y") {
            tts += "보너스 캐시 " + std::to_string(m_bonusCashUseAmt) + "원";
        }
        tts += "이 적용되어 ";
    }

    tts += "최종 결제 금액은 " + std::to_string(m_nAmount) + "원입니다.";
    return tts;
}
```

---

## 5. 구현 Phase

### Phase 1: 기초 인프라 구축
- [x] OpenSSL HMAC-SHA256 함수 래퍼 구현 ✅ (2026-01-19)
- [x] UUID/Nonce 생성 함수 구현 ✅ (2026-01-19)
- [x] JSON 파싱 라이브러리 도입 (간단한 자체 구현) ✅ (2026-01-19)
- [x] HTTP/HTTPS 클라이언트 구현 (WinHTTP) ✅ (2026-01-19)

### Phase 2: API 통신 계층 구현
- [x] PayLetterAPI.h/cpp 파일 생성 ✅ (2026-01-19)
- [x] PL_Initialize() 함수 구현 ✅ (2026-01-19)
- [x] PL_GenerateAuthHeader() 함수 구현 ✅ (2026-01-19)
- [x] PL_GetPaymentInfo() 함수 구현 ✅ (2026-01-19)
- [x] JSON 요청/응답 변환 함수 구현 ✅ (2026-01-19)
- [x] 에러 처리 및 로깅 구현 ✅ (2026-01-19)

### Phase 3: 비즈니스 로직 수정
- [x] ALLAT_getOrderInfo() 함수 수정 (getOrderInfo_NewAPI_host 추가) ✅ (2026-01-19)
- [x] 쿠폰/보너스캐시 멤버 변수 추가 ✅ (2026-01-19)
- [x] GeneratePaymentTTS() 함수 구현 ✅ (2026-01-19)
- [ ] 음성 안내 시나리오 수정 (추후 TTS 연동 테스트 필요)

### Phase 4: 테스트 및 검증
- [ ] QA 서버 연동 테스트
- [ ] 단위 테스트 케이스 작성
- [ ] 통합 테스트 수행
- [ ] 에러 케이스 검증

### Phase 5: 배포 및 전환
- [ ] LIVE 서버 테스트
- [ ] 병행 운영 설정
- [ ] 모니터링 및 로깅 강화
- [ ] 롤백 절차 문서화

---

## 6. 설정 파일 변경

### 6.1 INI 파일 추가 항목

```ini
[PAYLETTER_API]
# 테스트 서버 (QA)
QA_APP_ID=8c3cdc588ff746599a0beb714b4dce3a
QA_APP_KEY=ljyay9tYbSSVNIQIgZpZYdzgdr7Nlg7O4hdZ+pfrElw=
QA_URL=https://devswbillapi.wowtv.co.kr

# 운영 서버 (LIVE)
LIVE_APP_ID=3f70be5fa9bd40f4b5726116ebd05c61
LIVE_APP_KEY=WWBDhJAQJ/AL0r/qtr5gT3RyWmIpSICt5M64iEz6acw=
LIVE_URL=https://swbillapi.wowtv.co.kr

# 공통 설정
TIMEOUT=30
USE_NEW_API=true
USE_LEGACY_FALLBACK=true
```

---

## 7. 위험 관리

### 7.1 주요 위험 및 완화

| 위험 | 영향도 | 완화 전략 |
|------|--------|----------|
| 인증 실패 (시간 동기화) | 🔴 Critical | NTP 동기화, 5분 여유 확인 |
| JSON 파싱 오류 | 🟡 High | 타입 안전 라이브러리, 스키마 검증 |
| 응답 지연 | 🟡 High | 30초 타임아웃, 재시도 로직 |
| 기존 API 호환성 | 🟡 High | 병행 운영, 폴백 로직 |

### 7.2 롤백 계획

```
문제 발생 시:
1. INI 설정에서 USE_NEW_API=false로 변경
2. 기존 WowTvSocket 로직으로 자동 폴백
3. 로그 분석 및 원인 파악
4. 수정 후 재테스트
```

---

## 8. 검증 계획

### 8.1 테스트 케이스

| 케이스 | 입력 | 예상 결과 |
|--------|------|----------|
| 정상 주문 조회 | 유효한 회선번호 | 주문 정보 반환 |
| 쿠폰 적용 | 쿠폰 보유 고객 | 쿠폰명 포함 TTS |
| 보너스캐시 적용 | 캐시 보유 고객 | 캐시 금액 포함 TTS |
| 구매 제한 | purchaseLimitFlag != "1" | 구매 불가 안내 |
| API 오류 | 잘못된 인증 | 에러 로그, 폴백 |
| 타임아웃 | 지연 응답 | 재시도 또는 폴백 |

### 8.2 End-to-End 검증

1. QA 서버에서 전체 시나리오 테스트
2. 실제 전화 착신 → 주문 조회 → 카드 입력 → 결제 완료
3. 로그 확인 및 DB 상태 검증

---

## 9. 수정 대상 파일 목록

| 파일 | 작업 내용 | 상태 |
|------|----------|------|
| `PayLetterAPI.h` | 신규 생성 - API 헤더 | ✅ 완료 |
| `PayLetterAPI.cpp` | 신규 생성 - API 구현 | ✅ 완료 |
| `WowTvSocket.cpp` | 수정 - 신규 API 호출 함수 추가 | ✅ 완료 |
| `ALLAT_Stockwin_Quick_New_Scenario.h` | 수정 - 멤버 변수 추가 | ✅ 완료 |
| `AllatWowTvQuickPay_para.ini` | 신규 생성 - API 설정 추가 | ✅ 완료 |
| `ALLAT_Stockwin_Quick_New_Scenario.cpp` | 수정 - 시나리오에서 신규 API 호출 | ⏳ 대기 |
| `stdafx.h` | 수정 - 필요시 헤더 포함 | ⏳ 대기 |

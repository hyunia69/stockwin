# ALLAT_Stockwin_Billkey_New_Scenario 수정 요구사항

> **버전**: 2.0  
> **최종수정일**: 2026-01-31  
> **대상 프로젝트**: `ALLAT_Stockwin_Billkey_New_Scenario`  
> **Base Code**: `ALLAT_Stockwin_Quick_New_Scenario` (Quick 시나리오 패턴 기반)

---

## 목차

1. [개요](#1-개요)
2. [Quick 시나리오 기반 구현 원칙](#2-quick-시나리오-기반-구현-원칙)
3. [구현 상세](#3-구현-상세)
4. [구현 상태 테이블](#4-구현-상태-테이블)
5. [신규 API 연동](#5-신규-api-연동)
6. [신규 시나리오 요구사항](#6-신규-시나리오-요구사항)
7. [개발 가이드라인](#7-개발-가이드라인)
8. [구현 체크리스트](#8-구현-체크리스트)

---

## 1. 개요

### 1.1 프로젝트 목적

Billkey(빌키)를 사용한 정기결제 ARS 시나리오 DLL을 신규 API 및 시나리오 스펙에 맞게 수정합니다.
**모든 구현은 ALLAT_Stockwin_Quick_New_Scenario의 검증된 패턴을 기반으로 합니다.**

### 1.2 수정 범위

| 구분 | 기존 | 변경 후 |
|------|------|---------|
| **상품정보 API** | 레거시 ASP API | REST API (`/v1/payment/batch/getpayment/arsno_V2`) |
| **응답 형식** | XML | JSON |
| **인증 방식** | Query Parameter | HMAC-SHA256 Authorization 헤더 |
| **시나리오 흐름** | 기존 정기결제 | 신규 정기결제 시나리오 (무료체험 포함) |
| **Base Code** | N/A | Quick 시나리오 패턴 적용 |

### 1.3 v2.0 주요 변경사항

- Quick 시나리오 기반 구현 원칙 추가
- 검증된 코드 참조 (함수명 + 라인번호)
- 구현 상태 테이블 추가
- Free trial attr 파싱 가이드
- 동의 스킵 (SKIP_PARTNER_CONSENT) 구현 가이드
- 롤백 메커니즘 상세 설명
- 소멸자 안전성 구현 가이드

---

## 2. Quick 시나리오 기반 구현 원칙

### 2.1 Base Code 프로젝트

```
../ALLAT_Stockwin_Quick_New_Scenario/
├── WowTvSocket.cpp                           # REST API 호출 패턴
├── PayLetterAPI.cpp                          # HMAC-SHA256, JSON 파싱, 롤백 API
├── PayLetterAPI.h                            # 데이터 구조체
├── ALLAT_Stockwin_Quick_New_Scenario.cpp     # 소멸자, DisConnectProcess, 동의 스킵
├── ALLAT_Stockwin_Quick_New_Scenario.h       # 멤버 변수 선언
└── ADODB.cpp                                 # DNIS 조회 패턴
```

### 2.2 핵심 구현 패턴 (검증된 코드 참조)

#### 2.2.1 REST API 호출 패턴

**Quick 시나리오 참조**: `PL_InfoOrderReq_Process()` (WowTvSocket.cpp:884-903)

```cpp
// REST API 호출을 위한 스레드 함수
unsigned int __stdcall PL_InfoOrderReq_Process(void *data)
{
    // 멤버 변수 초기화
    // HTTP 요청 생성
    // JSON 응답 파싱
    // 결과 저장
}
```

**Billkey 구현 가이드**:
1. `PL_BatchInfoOrderReq_Process()` 함수 생성
2. Quick의 `PL_InfoOrderReq_Process()` 로직 복사
3. API 엔드포인트 변경: `/v1/payment/batch/getpayment/arsno_V2`
4. 응답 구조체 변경: `PL_PaymentInfo` → `PL_BatchPaymentInfo`

#### 2.2.2 롤백 메커니즘

**Quick 시나리오 참조**:
- **롤백 플래그 설정**: (WowTvSocket.cpp:1112)
  ```cpp
  // 쿠폰 또는 보너스 캐시 사용 시
  pScenario->m_bNeedRollback = TRUE;
  ```

- **PL_ReserveRollback()**: (PayLetterAPI.cpp:797-816)
  ```cpp
  int PL_ReserveRollback(const char* orderNo, const char* memberId)
  {
      // POST /v1/payment/simple/reserverollback
      // HMAC-SHA256 인증
      // JSON 응답 처리
  }
  ```

- **DisConnectProcess()**: (ALLAT_Stockwin_Quick_New_Scenario.cpp:2341-2355)
  ```cpp
  int CALLAT_Hangung_Quick_Scenario::DisConnectProcess()
  {
      // m_bDisconnectProcessed 플래그로 중복 호출 방지
      if (InterlockedCompareExchange(&m_bDisconnectProcessed, 1, 0) == 0)
      {
          if (m_bNeedRollback && m_bPaymentApproved)
          {
              PL_ReserveRollback(orderNo, memberId);
          }
      }
  }
  ```

- **소멸자**: (ALLAT_Stockwin_Quick_New_Scenario.cpp:2260-2269)
  ```cpp
  CALLAT_Hangung_Quick_Scenario::~CALLAT_Hangung_Quick_Scenario()
  {
      DisConnectProcess();  // 안전한 정리
  }
  ```

**Billkey 구현 가이드**:
1. **Header 파일** (`ALLAT_Stockwin_Billkey_New_Scenario.h`)에 멤버 변수 추가:
   - Quick 참조: (ALLAT_Stockwin_Quick_New_Scenario.h:121-123)
   ```cpp
   volatile LONG m_bNeedRollback;        // 롤백 필요 플래그
   LONG m_bPaymentApproved;              // 결제 승인 성공 플래그
   BOOL m_bDisconnectProcessed;          // DisConnectProcess 호출 플래그
   ```

2. **WowTvSocket.cpp**에서 쿠폰/캐시 사용 시 플래그 설정:
   ```cpp
   if (CouponUseFlag == 'Y' || BonusCashUseFlag == 'Y')
   {
       pScenario->m_bNeedRollback = TRUE;
   }
   ```

3. **DisConnectProcess() 함수** 추가 (Quick cpp:2341-2355 패턴 복사)

4. **소멸자**에서 DisConnectProcess() 호출 (Quick cpp:2260-2269 패턴 복사)

5. **PL_ReserveRollback() 함수** 추가:
   - Quick PayLetterAPI.cpp:797-816 복사
   - 엔드포인트 확인 필요 (Quick: `/v1/payment/simple/reserverollback`, Billkey: 동일 또는 `/v1/payment/batch/reserverollback`)

#### 2.2.3 동의 스킵 (SKIP_PARTNER_CONSENT)

**Quick 시나리오 참조**: (ALLAT_Stockwin_Quick_New_Scenario.cpp:1427-1438)

```cpp
// INI 설정 읽기
GetPrivateProfileString("SCENARIO_OPTIONS", "SKIP_PARTNER_CONSENT", "0", 
                        szSkipConsent, sizeof(szSkipConsent), INI_FILE);

if (atoi(szSkipConsent) == 1)
{
    // 동의 절차 스킵, 바로 카드 입력으로
    return ALLAT_CardInput(0);
}
else
{
    // 기존 동의 절차 진행
    return ALLAT_Consent(0);
}
```

**Billkey 구현 가이드**:
1. INI 파일에 설정 추가:
   ```ini
   [SCENARIO_OPTIONS]
   SKIP_PARTNER_CONSENT=1  ; 1=스킵, 0=진행
   ```

2. 시나리오 분기 로직에 Quick cpp:1427-1438 패턴 적용

#### 2.2.4 DNIS 조회 패턴

**Quick 시나리오 참조**: `GetCategoryByDnis()` (ADODB.cpp:1647-1666)

```cpp
int CADODB::GetCategoryByDnis(CString szDnis, char* szCategoryOut, int nCategorySize)
{
    // COMMON_DNIS_INFO 테이블 조회
    // SERVOCE_NAME 필드 반환
}
```

**중요**: Quick은 `SERVOCE_NAME` 필드를 직접 조회합니다. Billkey는 `DNIS_DESCRIPTION` JSON 파싱이 필요합니다.

---

## 3. 구현 상세

### 3.1 Free Trial Attr 파싱 (DNIS_DESCRIPTION JSON)

#### 3.1.1 요구사항

`DNIS_COMMON_INFO.DNIS_DESCRIPTION` 컬럼의 JSON에서 `attr` 필드 추출:

```json
{
  "amount": "99000",
  "attr": "1MONTH",
  "startdtm": "202601200900",
  "product_code": "PROD001",
  "service_name": "프리미엄종목"
}
```

**attr 값**:
- `NOFREE`: 무료체험 없음 (A-1)
- `1WEEK`: 1주일 무료체험 (A-2)
- `2WEEK`: 2주일 무료체험 (A-3)
- `1MONTH`: 1개월 무료체험 (A-4)

#### 3.1.2 구현 가이드

**⚠️ 현재 상태**: Quick 시나리오는 `GetCategoryByDnis()` (ADODB.cpp:1647-1666)로 `SERVOCE_NAME` 필드를 직접 조회합니다. JSON 파싱 함수는 **존재하지 않습니다**.

**새 함수 작성 필요**:

```cpp
// ADODB.cpp에 추가
int CADODB::GetFreeTrialAttrByDnis(CString szDnis, char* szAttrOut, int nAttrSize)
{
    // 1. DNIS_COMMON_INFO 테이블에서 DNIS_DESCRIPTION 조회
    // 2. JSON 파싱 (MSXML6 또는 수동 파싱)
    // 3. "attr" 필드 추출
    // 4. szAttrOut에 복사 (NOFREE, 1WEEK, 2WEEK, 1MONTH)
    
    // GetCategoryByDnis() (ADODB.cpp:1647-1666) 패턴 참조
}
```

**JSON 파싱 옵션**:
1. MSXML6 (Quick 시나리오에서 사용 중)
2. 수동 문자열 파싱 (`strstr`, `sscanf`)

#### 3.1.3 m_szTrialType 변수

**⚠️ 중요**: `m_szTrialType` 변수는 코드에서 **참조되지만 선언되지 않았습니다**.

**구현 필요**:
1. Header 파일에 선언 추가:
   ```cpp
   char m_szTrialType[16];  // NOFREE, 1WEEK, 2WEEK, 1MONTH
   ```

2. 초기화 (생성자):
   ```cpp
   memset(m_szTrialType, 0x00, sizeof(m_szTrialType));
   ```

3. 값 설정 (DNIS 조회 후):
   ```cpp
   GetFreeTrialAttrByDnis(szDnis, pScenario->m_szTrialType, sizeof(pScenario->m_szTrialType));
   ```

### 3.2 동의 스킵 (SKIP_PARTNER_CONSENT)

**Quick 시나리오 참조**: (ALLAT_Stockwin_Quick_New_Scenario.cpp:1427-1438)

**구현 완료 가이드**:
- INI 파일 설정 추가
- 시나리오 분기 로직에 Quick 패턴 적용
- 상세 내용은 섹션 2.2.3 참조

### 3.3 롤백 메커니즘

**Quick 시나리오 참조**:
- 롤백 플래그: (WowTvSocket.cpp:1112)
- PL_ReserveRollback(): (PayLetterAPI.cpp:797-816)
- DisConnectProcess(): (ALLAT_Stockwin_Quick_New_Scenario.cpp:2341-2355)
- 소멸자: (ALLAT_Stockwin_Quick_New_Scenario.cpp:2260-2269)
- 멤버 변수: (ALLAT_Stockwin_Quick_New_Scenario.h:121-123)

**구현 완료 가이드**:
- 상세 내용은 섹션 2.2.2 참조

### 3.4 소멸자 안전성 (DisConnectProcess, InterlockedExchange)

**Quick 시나리오 참조**: (ALLAT_Stockwin_Quick_New_Scenario.cpp:2341-2355)

**핵심 패턴**:
```cpp
// InterlockedCompareExchange로 중복 호출 방지
if (InterlockedCompareExchange(&m_bDisconnectProcessed, 1, 0) == 0)
{
    // 첫 호출만 실행
    if (m_bNeedRollback && m_bPaymentApproved)
    {
        PL_ReserveRollback(m_szOrderNo, m_szMemberId);
    }
    // DB 정리, 리소스 해제
}
```

**구현 완료 가이드**:
1. `m_bDisconnectProcessed` 변수 선언 (BOOL 타입)
2. DisConnectProcess() 함수에 InterlockedCompareExchange 패턴 적용
3. 소멸자에서 DisConnectProcess() 호출

### 3.5 REST API 통합 (PL_GetBatchPaymentInfo)

**Billkey 시나리오 참조**:
- **PL_BatchPaymentInfo 구조체**: (PayLetterAPI.h:83-111)
- **PL_GetBatchPaymentInfo()**: (PayLetterAPI.cpp:772-877)

**현재 상태**: ✅ **이미 구현됨**

```cpp
int PL_GetBatchPaymentInfo(int reqType, const char* reqTypeVal,
                           const char* phoneNo, const char* arsType,
                           PL_BatchPaymentInfo* outInfo)
{
    // POST /v1/payment/batch/getpayment/arsno_V2
    // HMAC-SHA256 인증
    // JSON 응답 파싱
    // outInfo 구조체에 결과 저장
}
```

**사용 예시**:
```cpp
PL_BatchPaymentInfo batchInfo;
PL_InitBatchPaymentInfo(&batchInfo);

int result = PL_GetBatchPaymentInfo(1, szLineNo, szPhoneNo, "ARS", &batchInfo);
if (result == 0)
{
    // batchInfo.batchPayType 기반 분기
    // batchInfo.CouponUseFlag, BonusCashUseFlag 확인
}
```

---

## 4. 구현 상태 테이블

| Feature | Quick Scenario | Billkey Scenario | Status |
|---------|----------------|------------------|--------|
| **멤버 변수 (m_bNeedRollback 등)** | ✅ Implemented (h:121-123) | ❌ Not Yet | **TODO** |
| **PL_ReserveRollback API** | ✅ Implemented (cpp:797-816) | ❌ Not Yet | **TODO** |
| **DisConnectProcess** | ✅ Implemented (cpp:2341-2355) | ❌ Not Yet | **TODO** |
| **소멸자 안전성** | ✅ Implemented (cpp:2260-2269) | ❌ Not Yet | **TODO** |
| **m_szTrialType 변수** | ❌ **NOT DECLARED** | ❌ **NOT DECLARED** | **NEEDS HEADER DECL** |
| **DNIS JSON Parser** | ❌ Uses SERVOCE_NAME (cpp:1647-1666) | ❌ Not Yet | **NEW FUNCTION NEEDED** |
| **SKIP_PARTNER_CONSENT** | ✅ Implemented (cpp:1427-1438) | ❌ Not Yet | **TODO** |
| **PL_GetBatchPaymentInfo** | N/A | ✅ Implemented (cpp:772-877) | **DONE** |
| **PL_BatchPaymentInfo 구조체** | N/A | ✅ Implemented (h:83-111) | **DONE** |
| **REST API 호출 패턴** | ✅ Implemented (cpp:884-903) | ❌ Not Yet | **TODO** |

**범례**:
- ✅ **Implemented**: 구현 완료, 코드 참조 가능
- ❌ **Not Yet**: 미구현, Quick 패턴 복사 필요
- ❌ **NOT DECLARED**: 변수 선언 자체가 없음 (구현 갭)
- **NEW FUNCTION NEEDED**: 새 함수 작성 필요 (Quick에 없음)

---

## 5. 신규 API 연동

### 5.1 참조 문서

- **API 스펙**: [`NEW_SPEC_PL_API.md`](./NEW_SPEC_PL_API.md)

### 5.2 사용할 API

#### 정기결제 정보 조회

```
POST /v1/payment/batch/getpayment/arsno_V2
```

| 환경 | URL |
|------|-----|
| **QA** | `https://devswbillapi.wowtv.co.kr/v1/payment/batch/getpayment/arsno_V2` |
| **LIVE** | `https://swbillapi.wowtv.co.kr/v1/payment/batch/getpayment/arsno_V2` |

### 5.3 요청 파라미터

```json
{
  "reqType": 1,
  "reqTypeVal": "1234",
  "phoneNo": "01011112222",
  "ARSType": "ARS"
}
```

| 필드 | 타입 | 필수 | 설명 |
|------|------|------|------|
| `reqType` | Byte | O | 1=회선번호, 2=상품코드, 3=종목알파고용 |
| `reqTypeVal` | String(12) | O | 회선번호 또는 상품코드 |
| `phoneNo` | String(16) | O | 휴대폰번호 (하이픈 제외) |
| `ARSType` | String(4) | O | 고정값: `ARS` |

### 5.4 응답 파라미터 (핵심 필드)

```json
{
  "resultCode": "0",
  "data": {
    "memberId": "604ab16a-e525-49d8-984d-b06662ff8c46",
    "orderNo": 202001011222154,
    "nickName": "테스터",
    "itemName": "주식비타민 3일 패키지(정기)",
    "payAmt": 34000,
    "CouponUseFlag": "Y",
    "CouponName": "정액 50%할인 쿠폰",
    "BonusCashUseFlag": "Y",
    "BonusCashUseAmt": 8500,
    "purchaseAmt": 27000,
    "batchPayType": 0,
    "serviceEndMonth": "03",
    "serviceEndDay": "31",
    "serviceDivDay": "10"
  }
}
```

| 필드 | 타입 | 설명 |
|------|------|------|
| `memberId` | String(36) | 회원 ID (UUID) |
| `orderNo` | Int64 | 주문번호 |
| `nickName` | String(20) | 필명 (고객명) |
| `itemName` | String(100) | 상품명 |
| `payAmt` | Int64 | 결제 금액 (할인 적용 후) |
| `purchaseAmt` | Int64 | 상품 원가 (할인 전) |
| `CouponUseFlag` | String(1) | 쿠폰 적용 여부 (`Y`/`N`) |
| `CouponName` | String(50) | 쿠폰명 |
| `BonusCashUseFlag` | String(1) | 보너스캐시 적용 여부 (`Y`/`N`) |
| `BonusCashUseAmt` | Int64 | 보너스캐시 사용 금액 |
| `batchPayType` | Byte | **핵심** - 0=최초, 1=해지, 2=사용중 |
| `serviceEndMonth` | String(2) | 서비스 종료 월 |
| `serviceEndDay` | String(2) | 서비스 종료 일 |
| `serviceDivDay` | String(2) | 남은 기간 (일) |
| `purchaseLimitFlag` | String(1) | 구매 제한 (1=정상, 2~6=제한) |

### 5.5 인증 방식

```
Authorization: PLTOKEN {APP_ID}:{Signature}:{Nonce}:{Timestamp}
```

**Signature 생성:**
```
RequestString = APP_ID + "POST" + Timestamp + Nonce
Signature = Base64( HMAC-SHA256( Base64Decode(APP_KEY), UTF8(RequestString) ) )
```

> 상세 구현은 [`NEW_SPEC_PL_API.md`](./NEW_SPEC_PL_API.md) 섹션 2 참조

**Quick 시나리오 참조**: PayLetterAPI.cpp에 HMAC-SHA256 구현 있음

---

## 6. 신규 시나리오 요구사항

### 6.1 참조 문서

- **시나리오 스펙**: [`NEW_SPEC_SCENARIO.md`](./NEW_SPEC_SCENARIO.md) - 정기결제 시나리오 섹션
- **원본 파일**: `STOCKWIN_SCENARIO.xlsb` (필요 시 참조)

### 6.2 정기결제 시나리오 흐름

```
┌─────────────────────────────────────────────────────────────────┐
│                      ARS 정기결제 시나리오                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │     인사 멘트          │
                    │ "안녕하세요? 한국경제   │
                    │  TV 정기결제 등록       │
                    │  서비스입니다"          │
                    └───────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │   고객 상태 확인        │
                    │ (batchPayType 기반)    │
                    └───────────────────────┘
                                │
        ┌───────────┬───────────┼───────────┬───────────┐
        ▼           ▼           ▼           ▼           ▼
   ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐
   │ A: 최초 │ │ B: 이미 │ │ C: 해지 │ │ C-2: 키 │ │ D: 재등록│
   │   등록  │ │ 이용중  │ │후 재결제│ │   존재  │ │   완료  │
   └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘
        │           │           │           │           │
        ▼           │           ▼           ▼           │
   ┌─────────┐      │      ┌─────────┐ ┌─────────┐      │
   │ 무료체험 │      │      │ 재결제  │ │ 고객센터│      │
   │ 유형선택│      │      │ 동의요청│ │ 안내    │      │
   └─────────┘      │      └─────────┘ └─────────┘      │
        │           │           │                       │
   ┌────┼────┬──────┘      ┌────┘                       │
   ▼    ▼    ▼             ▼                            ▼
┌────┐┌────┐┌────┐    ┌─────────┐               ┌─────────────┐
│없음││1주││1개월│    │1: 동의  │               │ 남은기간 안내│
└────┘└────┘└────┘    │2: 취소  │               └─────────────┘
   │    │    │        └─────────┘
   └────┼────┘             │
        ▼                  ▼
   ┌─────────────────────────────┐
   │    동의 확인 (1: 예, 2: 아니오)  │
   └─────────────────────────────┘
              │           │
        1번   │           │ 2번
              ▼           ▼
   ┌─────────────────┐   ┌─────────────────┐
   │ [공통] 카드 입력 │   │    종료         │
   └─────────────────┘   └─────────────────┘
              │
              ▼
   ┌─────────────────┐
   │  정기결제 등록    │
   │     완료        │
   └─────────────────┘
```

### 6.3 고객 상태별 분기 (batchPayType 기반)

| batchPayType | 상태 | 케이스 | 처리 |
|--------------|------|--------|------|
| 0 | 최초가입 | A | 무료체험 유형 선택 후 등록 |
| 2 | 사용중 | B | 이미 등록 완료 안내 |
| 1 | 해지 | C | 재결제 동의 요청 |
| - | 키 존재 (미등록) | C-2 | 고객센터 안내 |
| - | 재등록 완료 | D | 남은 기간 안내 |

### 6.4 무료체험 유형 (케이스 A)

| 코드 | 유형 | 결제 시점 | attr 값 |
|------|------|----------|---------|
| A-1 | 무료체험 없음 | 즉시 결제 | `NOFREE` |
| A-2 | 1주일 무료체험 | 1주일 후 첫 결제 | `1WEEK` |
| A-3 | 2주일 무료체험 | 2주일 후 첫 결제 | `2WEEK` |
| A-4 | 1개월 무료체험 | 1개월 후 첫 결제 | `1MONTH` |

### 6.5 쿠폰/보너스캐시 처리

| 항목 | 정책 |
|------|------|
| **쿠폰** | 첫 결제 시에만 적용 가능 |
| **보너스캐시** | 정기결제 사용 불가 |
| **적용 우선순위** | 쿠폰 우선, 동일 쿠폰 시 만료 임박 순 |
| **롤백** | 쿠폰/캐시 사용 시 `m_bNeedRollback = TRUE` 설정 필요 |

---

## 7. 개발 가이드라인

### 7.1 파일 인코딩 (필수)

```
⚠️ 모든 소스 파일은 반드시 UTF-8 with BOM 인코딩 사용
```

**확인 방법:**
```bash
# BOM 확인 (efbbbf가 있어야 함)
head -c 3 <파일명> | xxd -p

# BOM 추가 (없는 경우)
sed -i '1s/^/\xEF\xBB\xBF/' <파일명>
```

### 7.2 코드 스타일

| 규칙 | 예시 |
|------|------|
| 변수 선언 | 함수 시작부에 선언 (C89 스타일) |
| 멤버 변수 | `m_` 접두사 (`m_szCardNo`, `m_nAmount`) |
| 문자열 안전성 | `strncpy()` + null 종료 필수 |
| 버퍼 초기화 | `memset(buf, 0x00, sizeof(buf))` |

### 7.3 레거시 필드 매핑

```
기존 XML 필드        →  신규 JSON 필드
─────────────────────────────────────
cc_name             →  nickName
cc_pord_desc        →  itemName
amount              →  payAmt
sub_status          →  batchPayType
sub_amount          →  payAmt
expire_date         →  serviceEndMonth + serviceEndDay
```

---

## 8. 구현 체크리스트

### 8.1 Quick 시나리오 패턴 적용

- [ ] **멤버 변수 추가** (h:121-123 패턴)
  - [ ] `volatile LONG m_bNeedRollback`
  - [ ] `LONG m_bPaymentApproved`
  - [ ] `BOOL m_bDisconnectProcessed`
  - [ ] `char m_szTrialType[16]` (**NEW - 선언 필요**)

- [ ] **DisConnectProcess() 함수** (cpp:2341-2355 패턴)
  - [ ] InterlockedCompareExchange로 중복 호출 방지
  - [ ] m_bNeedRollback 확인 후 PL_ReserveRollback 호출
  - [ ] DB 정리, 리소스 해제

- [ ] **소멸자 수정** (cpp:2260-2269 패턴)
  - [ ] DisConnectProcess() 호출 추가

- [ ] **PL_ReserveRollback() 함수** (PayLetterAPI.cpp:797-816 패턴)
  - [ ] HMAC-SHA256 인증
  - [ ] POST /v1/payment/simple/reserverollback (또는 /batch/reserverollback)
  - [ ] JSON 응답 처리

- [ ] **롤백 플래그 설정** (WowTvSocket.cpp:1112 패턴)
  - [ ] CouponUseFlag == 'Y' 시 m_bNeedRollback = TRUE
  - [ ] BonusCashUseFlag == 'Y' 시 m_bNeedRollback = TRUE

- [ ] **SKIP_PARTNER_CONSENT** (cpp:1427-1438 패턴)
  - [ ] INI 파일 설정 추가
  - [ ] 시나리오 분기 로직 적용

### 8.2 Billkey 전용 구현

- [ ] **GetFreeTrialAttrByDnis() 함수** (**NEW - 작성 필요**)
  - [ ] DNIS_DESCRIPTION JSON 파싱
  - [ ] attr 필드 추출 (NOFREE, 1WEEK, 2WEEK, 1MONTH)
  - [ ] GetCategoryByDnis() (ADODB.cpp:1647-1666) 패턴 참조

- [ ] **PL_BatchInfoOrderReq_Process() 함수**
  - [ ] PL_InfoOrderReq_Process() (WowTvSocket.cpp:884-903) 패턴 복사
  - [ ] API 엔드포인트: /v1/payment/batch/getpayment/arsno_V2
  - [ ] PL_BatchPaymentInfo 구조체 사용

### 8.3 API 연동

- [ ] HMAC-SHA256 서명 생성 로직 (Quick PayLetterAPI.cpp 참조)
- [ ] Authorization 헤더 조립 함수
- [ ] `/v1/payment/batch/getpayment/arsno_V2` 호출
- [ ] JSON 응답 파싱 (Quick PayLetterAPI.cpp 참조)
- [ ] 오류 처리 (HTTP Status, error 객체)

### 8.4 시나리오 수정

- [ ] 인사 멘트 변경 ("정기결제 등록 서비스")
- [ ] `batchPayType` 기반 고객 상태 분기
- [ ] 케이스 A: 무료체험 유형 선택 처리 (m_szTrialType 사용)
- [ ] 케이스 B: 이미 이용중 안내
- [ ] 케이스 C: 해지 후 재결제 동의 요청
- [ ] 케이스 D: 재등록 완료 안내
- [ ] 쿠폰/보너스캐시 멘트 적용

### 8.5 검증

- [ ] QA 환경 테스트
- [ ] 각 케이스별 시나리오 검증
- [ ] 빌키 발급/결제 정상 동작 확인
- [ ] 롤백 메커니즘 테스트 (쿠폰/캐시 사용 시)
- [ ] UTF-8 BOM 인코딩 확인
- [ ] 메모리 누수 확인 (DisConnectProcess 호출 확인)

---

## 변경 이력

| 일자 | 버전 | 내용 |
|------|------|------|
| 2026-01-31 | 2.0 | Quick 시나리오 기반 구현 가이드 추가, 검증된 코드 참조, 구현 상태 테이블 추가 |
| 2026-01-31 | 1.0 | 문서 재구성 및 상세화 |

---

## 관련 문서

- [NEW_SPEC_SCENARIO.md](./NEW_SPEC_SCENARIO.md) - 시나리오 상세 스펙
- [NEW_SPEC_PL_API.md](./NEW_SPEC_PL_API.md) - API 연동 스펙
- [DB_SCHEMA_ALLAT.md](./DB_SCHEMA_ALLAT.md) - DB 스키마
- [AGENTS.md](../AGENTS.md) - AI 코딩 에이전트 가이드라인

---

## 검증된 코드 참조 인덱스

### Quick Scenario (Base Code)
- PL_InfoOrderReq_Process() - WowTvSocket.cpp:884-903
- Rollback flag setting - WowTvSocket.cpp:1112
- PL_ReserveRollback() - PayLetterAPI.cpp:797-816
- Destructor - ALLAT_Stockwin_Quick_New_Scenario.cpp:2260-2269
- DisConnectProcess() - ALLAT_Stockwin_Quick_New_Scenario.cpp:2341-2355
- Consent skip - ALLAT_Stockwin_Quick_New_Scenario.cpp:1427-1438
- GetCategoryByDnis() - ADODB.cpp:1647-1666
- Member variables - ALLAT_Stockwin_Quick_New_Scenario.h:121-123

### Billkey Scenario (Target)
- PL_BatchPaymentInfo struct - PayLetterAPI.h:83-111
- PL_GetBatchPaymentInfo() - PayLetterAPI.cpp:772-877

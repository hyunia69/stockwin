# ALLAT_Stockwin_Billkey_New_Scenario 수정 요구사항

> **버전**: 1.0  
> **최종수정일**: 2026-01-31  
> **대상 프로젝트**: `ALLAT_Stockwin_Billkey_New_Scenario`

---

## 목차

1. [개요](#1-개요)
2. [기존 코드베이스 분석](#2-기존-코드베이스-분석)
3. [신규 시나리오 요구사항](#3-신규-시나리오-요구사항)
4. [신규 API 연동](#4-신규-api-연동)
5. [참조 프로젝트](#5-참조-프로젝트)
6. [개발 가이드라인](#6-개발-가이드라인)
7. [구현 체크리스트](#7-구현-체크리스트)

---

## 1. 개요

### 1.1 프로젝트 목적

Billkey(빌키)를 사용한 정기결제 ARS 시나리오 DLL을 신규 API 및 시나리오 스펙에 맞게 수정합니다.
신규 API 를 구현하는 부분은 ALLAT_Stockwin_Quick_New_Scenario 의 구조와 유사하게 간다. 

### 1.2 수정 범위

| 구분 | 기존 | 변경 후 |
|------|------|---------|
| **상품정보 API** | 레거시 ASP API | REST API (`/v1/payment/batch/getpayment/arsno_V2`) |
| **응답 형식** | XML | JSON |
| **인증 방식** | Query Parameter | HMAC-SHA256 Authorization 헤더 |
| **시나리오 흐름** | 기존 정기결제 | 신규 정기결제 시나리오 (무료체험 포함) |

---

## 2. 기존 코드베이스 분석

### 2.1 프로젝트 구조

```
ALLAT_Stockwin_Billkey_New_Scenario/
├── ALLAT_Stockwin_Billkey_New_Scenario.cpp  # ARS 시나리오 메인
├── ALLAT_Access.cpp                          # ALLAT PG API (승인/취소/빌키)
├── WowTvSocket.cpp                           # TCP 소켓 통신 (레거시)
├── ADODB.cpp                                 # MS SQL Server ADO 연결
├── AllatUtil.cpp                             # SSL 통신 유틸리티
└── SHA256/                                   # KISA SHA256 라이브러리
```

### 2.2 레거시 특성

- **Legacy Code**: 기존 빌키 발급 및 정기결제 시나리오
- **상품정보 API**: 기존 ASP 기반 API 사용 중 - **수정 필요**
- **활용 가능한 부분**: 빌키 발급/결제 로직, DB 연동, 카드 입력 처리

### 2.3 유지할 기능

| 기능 | 파일 | 비고 |
|------|------|------|
| 빌키 발급 | `ALLAT_Access.cpp` | 그대로 유지 |
| 빌키 결제 | `ALLAT_Access.cpp` | 그대로 유지 |
| DB 연동 | `ADODB.cpp` | 그대로 유지 |
| 카드 정보 입력 | 시나리오 메인 | 공통 모듈로 분리 권장 |

### 2.4 수정할 기능

| 기능 | 현재 | 변경 |
|------|------|------|
| 상품정보 조회 | `WowTvSocket.cpp` (ASP API) | 신규 REST API |
| 고객 상태 확인 | 기존 로직 | `batchPayType` 기반 분기 |
| 쿠폰/보너스캐시 | 미지원 | 신규 API 응답 필드 활용 |

---

## 3. 신규 시나리오 요구사항

### 3.1 참조 문서

- **시나리오 스펙**: [`NEW_SPEC_SCENARIO.md`](./NEW_SPEC_SCENARIO.md) - 정기결제 시나리오 섹션
- **원본 파일**: `STOCKWIN_SCENARIO.xlsb` (필요 시 참조)

### 3.2 정기결제 시나리오 흐름

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

### 3.3 고객 상태별 분기 (batchPayType 기반)

| batchPayType | 상태 | 케이스 | 처리 |
|--------------|------|--------|------|
| 0 | 최초가입 | A | 무료체험 유형 선택 후 등록 |
| 2 | 사용중 | B | 이미 등록 완료 안내 |
| 1 | 해지 | C | 재결제 동의 요청 |
| - | 키 존재 (미등록) | C-2 | 고객센터 안내 |
| - | 재등록 완료 | D | 남은 기간 안내 |

### 3.4 무료체험 유형 (케이스 A)

| 코드 | 유형 | 결제 시점 |
|------|------|----------|
| A-1 | 무료체험 없음 | 즉시 결제 |
| A-2 | 1주일 무료체험 | 1주일 후 첫 결제 |
| A-3 | 2주일 무료체험 | 2주일 후 첫 결제 |
| A-4 | 1개월 무료체험 | 1개월 후 첫 결제 |

### 3.5 쿠폰/보너스캐시 처리

| 항목 | 정책 |
|------|------|
| **쿠폰** | 첫 결제 시에만 적용 가능 |
| **보너스캐시** | 정기결제 사용 불가 |
| **적용 우선순위** | 쿠폰 우선, 동일 쿠폰 시 만료 임박 순 |

---

## 4. 신규 API 연동

### 4.1 참조 문서

- **API 스펙**: [`NEW_SPEC_PL_API.md`](./NEW_SPEC_PL_API.md)

### 4.2 사용할 API

#### 정기결제 정보 조회

```
POST /v1/payment/batch/getpayment/arsno_V2
```

| 환경 | URL |
|------|-----|
| **QA** | `https://devswbillapi.wowtv.co.kr/v1/payment/batch/getpayment/arsno_V2` |
| **LIVE** | `https://swbillapi.wowtv.co.kr/v1/payment/batch/getpayment/arsno_V2` |

### 4.3 요청 파라미터

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

### 4.4 응답 파라미터 (핵심 필드)

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

### 4.5 인증 방식

```
Authorization: PLTOKEN {APP_ID}:{Signature}:{Nonce}:{Timestamp}
```

**Signature 생성:**
```
RequestString = APP_ID + "POST" + Timestamp + Nonce
Signature = Base64( HMAC-SHA256( Base64Decode(APP_KEY), UTF8(RequestString) ) )
```

> 상세 구현은 [`NEW_SPEC_PL_API.md`](./NEW_SPEC_PL_API.md) 섹션 2 참조

---

## 5. 참조 프로젝트

### 5.1 ALLAT_Stockwin_Quick_New_Scenario

신규 REST API를 이미 사용 중인 프로젝트입니다.

#### 참조할 구현 내용

| 항목 | 파일 | 설명 |
|------|------|------|
| API 호출 | `PayLetterAPI.cpp` | REST API 호출 로직 |
| HMAC-SHA256 인증 | `PayLetterAPI.cpp` | Signature 생성 |
| JSON 파싱 | `PayLetterAPI.cpp` | 응답 처리 |
| 키/암호화 관리 | INI 또는 코드 | APP_ID, APP_KEY 관리 |

#### 프로젝트 위치

```
../ALLAT_Stockwin_Quick_New_Scenario/
├── PayLetterAPI.cpp       # ← 핵심 참조 파일
├── PayLetterAPI.h
└── ...
```

### 5.2 참조 시 주의사항

- **파라미터 차이**: 빌키 결제 시 추가 파라미터 필요 가능
- **시나리오 차이**: Quick(일반결제) vs Billkey(정기결제) 흐름 상이
- **API 엔드포인트 차이**: 
  - Quick: `/v1/payment/simple/getpaymentinfo_V2`
  - Billkey: `/v1/payment/batch/getpayment/arsno_V2`

---

## 6. 개발 가이드라인

### 6.1 파일 인코딩 (필수)

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

### 6.2 코드 스타일

| 규칙 | 예시 |
|------|------|
| 변수 선언 | 함수 시작부에 선언 (C89 스타일) |
| 멤버 변수 | `m_` 접두사 (`m_szCardNo`, `m_nAmount`) |
| 문자열 안전성 | `strncpy()` + null 종료 필수 |
| 버퍼 초기화 | `memset(buf, 0x00, sizeof(buf))` |

### 6.3 레거시 필드 매핑

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

## 7. 구현 체크리스트

### 7.1 API 연동

- [ ] HMAC-SHA256 서명 생성 로직 구현
- [ ] Authorization 헤더 조립 함수 구현
- [ ] `/v1/payment/batch/getpayment/arsno_V2` 호출 구현
- [ ] JSON 응답 파싱 구현
- [ ] 오류 처리 (HTTP Status, error 객체)

### 7.2 시나리오 수정

- [ ] 인사 멘트 변경 ("정기결제 등록 서비스")
- [ ] `batchPayType` 기반 고객 상태 분기
- [ ] 케이스 A: 무료체험 유형 선택 처리
- [ ] 케이스 B: 이미 이용중 안내
- [ ] 케이스 C: 해지 후 재결제 동의 요청
- [ ] 케이스 D: 재등록 완료 안내
- [ ] 쿠폰/보너스캐시 멘트 적용

### 7.3 검증

- [ ] QA 환경 테스트
- [ ] 각 케이스별 시나리오 검증
- [ ] 빌키 발급/결제 정상 동작 확인
- [ ] UTF-8 BOM 인코딩 확인

---

## 변경 이력

| 일자 | 버전 | 내용 |
|------|------|------|
| 2026-01-31 | 1.0 | 문서 재구성 및 상세화 |

---

## 관련 문서

- [NEW_SPEC_SCENARIO.md](./NEW_SPEC_SCENARIO.md) - 시나리오 상세 스펙
- [NEW_SPEC_PL_API.md](./NEW_SPEC_PL_API.md) - API 연동 스펙
- [DB_SCHEMA_ALLAT.md](./DB_SCHEMA_ALLAT.md) - DB 스키마

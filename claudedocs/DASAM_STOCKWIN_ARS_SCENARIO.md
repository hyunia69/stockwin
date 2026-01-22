# StockWin ARS 결제 시나리오 마이그레이션 분석 문서

**문서 버전**: 1.0
**작성일**: 2026-01-13
**목적**: 기존 ARS 시나리오에서 신규 일반결제 시나리오로 마이그레이션 및 신규 페이레터 REST API 연동을 위한 종합 분석

---

## 참조 문서

본 분석 문서는 아래 원본 문서들을 기반으로 작성되었습니다.

| 문서 | 경로 | 설명 |
|------|------|------|
| **현행 시나리오 분석** | `claudedocs/OLD_ANALYZE_FLOW.md` | 현재 운영 중인 ARS 시나리오 흐름 분석 (정기결제 중심) |
| **신규 시나리오 명세** | `claudedocs/NEW_SPEC_SCENARIO.md` | 신규 일반결제/정기결제/사전예약 시나리오 TTS 멘트 정의 |
| **신규 API 스펙** | `claudedocs/NEW_SPEC_PL_API.md` | 페이레터 BOQV9 REST API 연동 규격 (v1.03) |

### 참조 소스 코드

| 파일 | 경로 | 용도 |
|------|------|------|
| **메인 시나리오** | `ALLAT_StockWin_Billkey_Easy_New_Scenario/ALLAT_StockWin_Billkey_Easy_New_Scenario.cpp` | ARS 시나리오 상태 머신 |
| **API 통신** | `ALLAT_StockWin_Billkey_Easy_New_Scenario/WowTvSocket.cpp` | 외부 API 호출 및 응답 파싱 |
| **DB 연동** | `ALLAT_StockWin_Billkey_Easy_New_Scenario/ADODB.cpp` | MS SQL Server 저장 프로시저 |
| **PG 결제** | `ALLAT_StockWin_Billkey_Easy_New_Scenario/ALLAT_Access.cpp` | ALLAT PG사 승인/취소 |
| **공통 정의** | `ALLAT_StockWin_Billkey_Easy_New_Scenario/ALLATCommom.h` | 인증 타입, 구조체 정의 |

---

## 목차

1. [개요](#1-개요)
2. [현행 시스템 분석](#2-현행-시스템-분석)
3. [신규 시나리오 요구사항](#3-신규-시나리오-요구사항)
4. [API 마이그레이션 분석](#4-api-마이그레이션-분석)
5. [코드 변경 영향도 분석](#5-코드-변경-영향도-분석)
6. [구현 계획](#6-구현-계획)
7. [리스크 및 고려사항](#7-리스크-및-고려사항)

---

## 1. 개요

### 1.1 프로젝트 배경

본 문서는 한국경제TV(WowTV) ARS 결제 시스템의 시나리오 마이그레이션을 위한 분석 문서입니다. 현행 시스템은 정기결제 중심의 복잡한 시나리오를 운영 중이며, 신규 요구사항은 일반결제 시나리오로의 단순화와 신규 REST API 연동입니다.

### 1.2 마이그레이션 목표

| 항목 | 현행 | 목표 |
|------|------|------|
| **시나리오** | 정기결제 중심 복합 시나리오 | 일반결제 단순화 시나리오 (1회성 카드결제) |
| **API 방식** | ASP 기반 XML 응답 | REST API JSON 응답 |
| **API 도메인** | `billadmin.wownet.co.kr` | `swbillapi.wowtv.co.kr` |
| **인증** | Query Parameter | HMAC-SHA256 Authorization |
| **신규 기능** | - | 쿠폰/보너스캐시 적용 |
| **제외 항목** | - | 빌키(간편결제키) 등록 프로세스 (추후 개발) |

### 1.3 핵심 소스 파일

```
ALLAT_StockWin_Billkey_Easy_New_Scenario/
├── ALLAT_StockWin_Billkey_Easy_New_Scenario.cpp  (3,567 lines) - 메인 시나리오
├── WowTvSocket.cpp                                (1,243 lines) - API 통신
├── ADODB.cpp                                      (2,272 lines) - DB 연동
├── ALLAT_Access.cpp                               (1,337 lines) - PG사 결제
├── AllatUtil.cpp                                  (1,286 lines) - SSL 유틸리티
└── ALLATCommom.h                                  (헤더)        - 공통 정의
```

---

## 2. 현행 시스템 분석

### 2.1 현행 시나리오 흐름 (OLD_ANALYZE_FLOW.md 기준)

```
┌─────────────────────────────────────────────────────────────────┐
│                    현행 정기결제 시나리오 흐름                     │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │   시나리오 진입점       │
                    │   jobArs(state)        │
                    └───────────────────────┘
                                │
        ┌───────────┬───────────┼───────────┬───────────┐
        ▼           ▼           ▼           ▼           ▼
   ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐
   │ ARS     │ │ SMS     │ │ CID     │ │ CIA     │ │ 기타    │
   │ 전화번호│ │ 주문번호│ │ Caller  │ │ CID+    │ │         │
   │ 직접입력│ │ 입력    │ │ ID자동  │ │ 휴대폰  │ │         │
   └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘
        │           │           │           │
        └───────────┴───────────┼───────────┘
                                ▼
                    ┌───────────────────────┐
                    │   ALLAT_getOrderInfo  │
                    │   주문정보 조회 및     │
                    │   정기결제 상태 분기   │
                    │   (Line 2223-2903)    │
                    └───────────────────────┘
                                │
            ┌───────────────────┼───────────────────┐
            ▼                   ▼                   ▼
       ┌─────────┐        ┌─────────┐        ┌─────────┐
       │ sub_    │        │ sub_    │        │ sub_    │
       │status=0 │        │status=1 │        │status=2 │
       │ 첫결제  │        │ 이용중  │        │해지후재 │
       └─────────┘        └─────────┘        └─────────┘
            │                   │                   │
            ▼                   ▼                   ▼
       ┌─────────┐        ┌─────────┐        ┌─────────┐
       │ 무료체험│        │ 이미등록│        │ 재결제  │
       │ 유무확인│        │ 안내후  │        │ 동의요청│
       │ 동의안내│        │ 종료    │        │         │
       └─────────┘        └─────────┘        └─────────┘
            │                                       │
            └───────────────────┬───────────────────┘
                                ▼
                    ┌───────────────────────┐
                    │   ALLAT_CardInput     │
                    │   카드정보 입력        │
                    │   (Line 2102-2222)    │
                    └───────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │   결제 승인 처리       │
                    │   AllatPaymemt_host   │
                    └───────────────────────┘
```

### 2.2 현행 API 통신 구조

#### 2.2.1 주문정보 조회 API (WowTvSocket.cpp:484)

```cpp
// 현행 API 호출 코드
sendURL.Format("https://billadmin.wownet.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s",
    (LPCTSTR)pScenario->szDnis,
    (LPCTSTR)pScenario->m_szInputTel);
Http_SSL_RetPageSend(data, sendURL.GetBuffer(), "POST");
```

#### 2.2.2 현행 XML 응답 파싱 구조 (WowTvSocket.cpp:530-571)

```xml
<Response>
    <retval>0</retval>
    <errmsg></errmsg>
    <order_no>주문번호</order_no>
    <shop_id>가맹점ID</shop_id>
    <cc_name>고객명</cc_name>
    <cc_pord_desc>상품명</cc_pord_desc>
    <cc_pord_code>상품코드</cc_pord_code>
    <amount>결제금액</amount>
    <partner_nm>파트너명</partner_nm>
    <bill_key>빌키</bill_key>
    <ext_data>확장데이터</ext_data>
    <renew_flag>Y/N</renew_flag>
    <card_name>카드사명</card_name>
    <sub_status>0/1/2</sub_status>
    <sub_amount>정기결제금액</sub_amount>
    <sub_has_trial>Y/N</sub_has_trial>
    <expire_date>만료일자</expire_date>
</Response>
```

### 2.3 현행 데이터 구조체

#### 2.3.1 주요 멤버 변수 (ALLAT_StockWin_Billkey_Easy_New_Scenario.h)

| 변수명 | 타입 | 설명 | 신규 매핑 |
|--------|------|------|----------|
| `m_szMx_issue_no` | char[81] | 주문번호 | `orderNo` |
| `m_szMx_id` | char[33] | 가맹점ID (최종선택) | `mallIdSimple` |
| `m_szMx_id1` | char[33] | 간편결제용 가맹점ID | `mallIdSimple` |
| `m_szMx_id2` | char[33] | 일반결제용 가맹점ID | `mallIdGeneral` |
| `m_szCC_name` | char[65] | 고객명 | `nickName` |
| `m_szCC_Prod_Desc` | char[256] | 상품명 | `itemName` |
| `m_nAmount` | int | 결제금액 | `payAmt` |
| `m_szbill_key` | char[257] | 빌키 | `billKey` |
| `m_szext_data` | char[257] | 확장데이터(생년월일) | `billPassword` |
| `m_szrenew_flag` | char[2] | 동의여부 | `payAgreeFlag` |
| `m_szcardName` | char[22] | 카드사명 | `cardCompany` |
| `m_szsub_status` | char[9] | 정기결제상태 | `batchPayType` |
| `m_nsub_amount` | int | 정기결제금액 | `payAmt` |
| `m_szsub_has_trial` | char[9] | 체험상품유무 | (비즈니스로직) |
| `m_szexpire_date` | char[33] | 만료일자 | `serviceEndMonth`+`serviceEndDay` |

#### 2.3.2 신규 추가 필요 변수

| 변수명 | 타입 | 설명 | 용도 |
|--------|------|------|------|
| `m_szMemberId` | char[40] | 회원 UUID | REST API 요청 |
| `m_szCouponUseFlag` | char[2] | 쿠폰존재여부 | 멘트 분기 |
| `m_szCouponName` | char[52] | 쿠폰명 | TTS 안내 |
| `m_szBonusCashUseFlag` | char[2] | 보너스캐시여부 | 멘트 분기 |
| `m_nBonusCashUseAmt` | int | 보너스캐시금액 | TTS 안내 |
| `m_nPurchaseAmt` | int | 상품원가(할인전) | TTS 안내 |
| `m_szPurchaseLimitFlag` | char[2] | 구매제한여부 | 분기 처리 |
| `m_szCategoryId2nd` | char[8] | 중분류카테고리 | 상품유형 판단 |
| `m_szPgCode` | char[2] | PG코드(A/P) | PG사 분기 |

---

## 3. 신규 시나리오 요구사항

### 3.1 일반결제 시나리오 흐름 (NEW_SPEC_SCENARIO.md 기준)

```
┌─────────────────────────────────────────────────────────────────┐
│                    신규 일반결제 시나리오 흐름                     │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │     인사 멘트          │
                    │ "안녕하세요? 한국경제   │
                    │  TV ARS 간편결제입니다" │
                    └───────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │   고객 정보 조회        │
                    │   쿠폰/보너스캐시 확인   │
                    │  [REST API 호출]       │
                    └───────────────────────┘
                                │
            ┌───────────────────┼───────────────────┐
            ▼                   ▼                   ▼
    ┌───────────────┐  ┌───────────────┐  ┌───────────────┐
    │ 할인 없음      │  │ 쿠폰만 있음   │  │ 둘 다 있음    │
    │ [기본 멘트]   │  │ [쿠폰 멘트]   │  │ [복합 멘트]   │
    └───────────────┘  └───────────────┘  └───────────────┘
            │                   │                   │
            └───────────────────┼───────────────────┘
                                ▼
                    ┌───────────────────────┐
                    │   결제 금액 안내        │
                    │   (할인 적용 후 금액)   │
                    └───────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │   투자 유의사항 안내    │
                    └───────────────────────┘
                                │
            ┌───────────────────┼───────────────────┐
            ▼                   ▼                   ▼
    ┌───────────────┐  ┌───────────────┐  ┌───────────────┐
    │ 일반상품       │  │ 태블릿 제공   │  │ 교육상품      │
    │ 해지조건 안내  │  │ 반품불가 안내 │  │ 해지불가 안내 │
    └───────────────┘  └───────────────┘  └───────────────┘
            │                   │                   │
            └───────────────────┼───────────────────┘
                                ▼
                    ┌───────────────────────┐
                    │ 결제 동의 확인         │
                    │ (1: 동의, 2: 취소)    │
                    └───────────────────────┘
                          │           │
                    1번   │           │ 2번
                          ▼           ▼
            ┌─────────────────┐   ┌─────────────────┐
            │ 카드 정보 입력   │   │    종료         │
            │ (1회성 결제)    │   └─────────────────┘
            └─────────────────┘
                      │
                      ▼
            ┌─────────────────┐
            │  결제 완료       │
            └─────────────────┘
```

### 3.2 신규 TTS 멘트 구조

#### 3.2.1 결제 금액 안내 멘트

| 조건 | 멘트 |
|------|------|
| **기본** (할인없음) | `[고객명] 고객님, [한국경제TV]에서, 주문하신 [상품명]의 결제하실 금액은 [X]원입니다.` |
| **쿠폰만 있음** | `[고객명] 고객님, [한국경제TV]에서, 주문하신 [상품명]의 결제 금액은 [고객명]께서 보유하신 [쿠폰명]쿠폰이 적용되어 최종 결제 금액은 [X]원입니다.` |
| **보너스캐시만** | `[고객명] 고객님, [한국경제TV]에서, 주문하신 [상품명]의 결제 금액은 [고객명]께서 보유하신 보너스 캐시[*]원이 적용되어 최종 결제 금액은 [X]원입니다.` |
| **둘 다 있음** | `[고객명] 고객님, [한국경제TV]에서, 주문하신 [상품명]의 결제 금액은 [고객명]께서 보유하신 [쿠폰명] 쿠폰과 보너스 캐시[*]원이 적용되어 최종 결제 금액은 [X]원입니다.` |

#### 3.2.2 상품유형별 안내 멘트

| 상품유형 | 멘트 |
|----------|------|
| **일반상품** | 서비스 중도해지 시 해지일까지 이용요금과 해지수수료 10퍼센트와 제공받으신 사은품 정가가 함께 차감됩니다. |
| **태블릿 제공** | 박스 개봉 후 제품 불량을 제외하고는 교환 및 반품이 불가합니다. |
| **교육상품** | 본 상품은 교육 상품으로 결제 후 해지가 불가능합니다. |

---

## 4. API 마이그레이션 분석

### 4.1 API 비교표

| 항목 | 현행 API | 신규 REST API |
|------|----------|---------------|
| **엔드포인트** | `/pgmodule/DasomARS/UserCall/orderRequestApi.asp` | `/v1/payment/simple/getpaymentinfo_V2` |
| **도메인** | `billadmin.wownet.co.kr` | `swbillapi.wowtv.co.kr` |
| **메소드** | GET/POST (Query Param) | POST |
| **인코딩** | EUC-KR/UTF-8 | UTF-8 |
| **응답형식** | XML | JSON |
| **인증** | 없음 | HMAC-SHA256 |

### 4.2 신규 API 인증 구현 요구사항

#### 4.2.1 인증 헤더 형식

```
Authorization: PLTOKEN {APP_ID}:{Signature}:{Nonce}:{Timestamp}
```

#### 4.2.2 Signature 생성 로직

```cpp
// 의사코드
RequestString = APP_ID + UpperCase(HTTP_METHOD) + Timestamp + Nonce
Signature = Base64( HMAC-SHA256( Base64Decode(APP_KEY), UTF8(RequestString) ) )
```

#### 4.2.3 구현 시 필요한 라이브러리

| 기능 | 현행 라이브러리 | 추가 필요 |
|------|----------------|----------|
| **HTTPS** | WinINet | (기존 활용) |
| **HMAC-SHA256** | KISA_SHA256 | OpenSSL HMAC 확장 필요 |
| **Base64** | (없음) | 신규 구현 필요 |
| **JSON 파싱** | (없음) | 신규 구현 or cJSON 라이브러리 |
| **UUID 생성** | (없음) | 신규 구현 필요 |

### 4.3 응답 필드 매핑

| 현행 XML 필드 | 신규 JSON 필드 | 데이터 타입 | 비고 |
|---------------|----------------|-------------|------|
| `retval` | `resultCode` | String | "0" = 성공 |
| `errmsg` | (error.message) | String | 에러시 |
| `order_no` | `orderNo` | Int64 | 타입 변경! |
| `shop_id` | `mallIdSimple` / `mallIdGeneral` | String(40) | 분리됨 |
| `cc_name` | `nickName` | String(20) | - |
| `cc_pord_desc` | `itemName` | String(100) | - |
| `amount` | `payAmt` | Int64 | 할인 적용 후 금액 |
| `partner_nm` | (별도 조회) | - | 필드 없음 |
| `bill_key` | `billKey` | String(128) | - |
| `ext_data` | `billPassword` | String(128) | - |
| `renew_flag` | `payAgreeFlag` | String(1) | - |
| `card_name` | `cardCompany` | String(30) | - |
| `sub_status` | `batchPayType` | Byte | 0/1/2 동일 |
| `sub_amount` | `payAmt` | Int64 | 통합 |
| `sub_has_trial` | (없음) | - | 비즈니스 로직 처리 |
| `expire_date` | `serviceEndMonth` + `serviceEndDay` | String | 분리됨 |
| (신규) | `CouponUseFlag` | String(1) | Y/N |
| (신규) | `CouponName` | String(50) | 쿠폰명 |
| (신규) | `BonusCashUseFlag` | String(1) | Y/N |
| (신규) | `BonusCashUseAmt` | Int64 | 보너스캐시 금액 |
| (신규) | `purchaseAmt` | Int64 | 할인 전 원가 |
| (신규) | `purchaseLimitFlag` | String(1) | 1-6 |
| (신규) | `memberId` | String(36) | UUID |
| (신규) | `pgCode` | String(1) | A/P |

### 4.4 신규 API 요청/응답 예시

#### 4.4.1 요청

```http
POST /v1/payment/simple/getpaymentinfo_V2 HTTP/1.1
Host: swbillapi.wowtv.co.kr
Authorization: PLTOKEN 3f2e11523b634600bc432fe38ed73e3d:Lu7AmhstnNLghIafRMNIXrswDRblv/D75Kee1BheIGY=:035ae3de-b68b-4d0f-b5fc-401b98a2da7b:1583391560
Content-Type: application/json; charset=utf-8

{
  "reqType": 1,
  "reqTypeVal": "4400",
  "phoneNo": "01012345678",
  "ARSType": "ARS"
}
```

#### 4.4.2 응답

```json
{
  "resultCode": "0",
  "data": {
    "memberId": "604ab16a-e525-49d8-984d-b06662ff8c46",
    "orderNo": 202001011222154,
    "nickName": "테스터",
    "packageId": 4,
    "itemName": "주식비타민 3일 패키지",
    "categoryId_2nd": "AAAAAA",
    "pgCode": "A",
    "merchantId": "testmerchantid",
    "mallIdSimple": "testmallidsimple",
    "mallIdGeneral": "testmallidgeneral",
    "payAmt": 5000,
    "CouponUseFlag": "Y",
    "CouponName": "정액 50%할인 쿠폰",
    "BonusCashUseFlag": "Y",
    "BonusCashUseAmt": 8500,
    "purchaseAmt": 27000,
    "billKey": "",
    "billPassword": "",
    "cardCompany": "",
    "payAgreeFlag": "N",
    "purchaseLimitFlag": "1"
  }
}
```

---

## 5. 코드 변경 영향도 분석

### 5.1 파일별 변경 범위

| 파일 | 변경 범위 | 주요 변경 내용 |
|------|----------|---------------|
| **ALLAT_StockWin_Billkey_Easy_New_Scenario.h** | 중 | 신규 멤버변수 추가 |
| **ALLAT_StockWin_Billkey_Easy_New_Scenario.cpp** | 대 | 시나리오 흐름 전면 수정, TTS 멘트 변경 |
| **WowTvSocket.cpp** | 대 | REST API 호출 로직 전면 수정, JSON 파싱 |
| **WowTvSocket.h** | 중 | 신규 구조체 정의 |
| **ADODB.cpp** | 소 | 신규 필드 저장 SP 파라미터 추가 |
| **ALLATCommom.h** | 소 | 신규 상수 정의 |
| **(신규) RestApiUtil.cpp** | 신규 | HMAC-SHA256, Base64, UUID 구현 |
| **(신규) JsonParser.cpp** | 신규 | JSON 파싱 유틸리티 |

### 5.2 함수별 변경 분석

#### 5.2.1 ALLAT_StockWin_Billkey_Easy_New_Scenario.cpp

| 함수명 | Line | 변경내용 | 영향도 |
|--------|------|----------|--------|
| `ALLAT_getOrderInfo` | 2223-2903 | 전면 재작성 - 일반결제 흐름으로 단순화 | 상 |
| `ALLAT_ArsScenarioStart` | 2905-3046 | 인사멘트 변경 | 하 |
| `ALLAT_CardInput` | 2102-2222 | NH농협카드 안내 추가 | 중 |
| `ALLAT_payARS` | 462-700 | 결제완료 멘트 변경 | 하 |

#### 5.2.2 WowTvSocket.cpp

| 함수명 | Line | 변경내용 | 영향도 |
|--------|------|----------|--------|
| `Wow_InfoRodocReq_Process` | 396-1122 | **전면 재작성** - REST API 호출, JSON 파싱 | 상 |
| `Http_SSL_RetPageSend` | (ADODB.cpp) | HMAC-SHA256 인증 헤더 추가 | 상 |

### 5.3 제거 대상 코드

#### 5.3.0 승인 후 Noti 전송 비활성화 (ADODB.cpp)

> **⚠️ 승인 후 Noti 전송 기능이 비활성화됨. Noti를 보낼 필요 없음.**

| 함수명 | Line | 변경내용 | 상태 |
|--------|------|----------|------|
| `AllatPayRetProcess` | 667-708 | `Http_SSL_RetPageSend` 호출 주석 처리 | 완료 |
| `AllatPayRetPrc_host` | 710-732 | 호출되어도 실제 Noti 전송하지 않음 | 완료 |

**변경 내용:**
```cpp
// 비활성화 전
Http_SSL_RetPageSend(data, pScenario->m_szSHOP_RET_URL_AG, "POST");

// 비활성화 후
// Noti 전송 비활성화 - 승인 후 Noti를 보낼 필요 없음
// Http_SSL_RetPageSend(data, pScenario->m_szSHOP_RET_URL_AG, "POST");
xprintf("[CH:%03d] AllatPayRetProcess Noti 전송 SKIP (비활성화됨)", ch);
```

**참고:** WowTvSocket.cpp의 `Http_SSL_RetPageSend` 호출(주문 정보 조회용)은 유지됨.

#### 5.3.1 정기결제 관련 코드 (일반결제에서 불필요)

| 코드 영역 | Line | 설명 |
|----------|------|------|
| `sub_status` 분기 | 2573-2870 | 정기결제 상태별 분기 로직 |
| `sub_has_trial` 분기 | 2591-2867 | 무료체험 유무 분기 |
| `expire_date` 처리 | 2721-2788 | 만료일 계산 로직 |
| DNIS별 특수 분기 | 다수 | 정기결제용 DNIS 분기 (4447, 5037, 5013 등) |

#### 5.3.2 빌키(간편결제키) 등록 관련 코드 (추후 개발 예정)

> **⚠️ 빌키 등록 프로세스는 현재 개발 범위에서 제외됨. 별도 세션에서 구현 예정.**

| 코드 영역 | 파일 | Line | 설명 |
|----------|------|------|------|
| `ALLAT_consent` | .cpp | 1282-1459 | 빌키 동의 입력부 (1번: 빌키 등록, 2번: 일반결제) |
| `Allat_Get_FixKey_host` | ALLAT_Access.cpp | 1253-1274 | 빌키 발급 호스트 함수 |
| `AllatArsGetFixKeyProcess` | ALLAT_Access.cpp | 950-1198 | 빌키 발급 스레드 함수 |
| AUTH_TYPE 41/42/43 | ALLATCommom.h | - | 빌키용 인증타입 (비인증/구인증/부분인증) |

**현재 구현**: AUTH_TYPE 01/02/03 (일반결제용)만 사용, 빌키 발급 로직 미사용

---

## 6. 구현 계획

### 6.1 단계별 구현 계획

#### Phase 1: 인프라 구축
1. HMAC-SHA256 인증 모듈 구현 (`RestApiUtil.cpp`)
2. Base64 인코딩/디코딩 구현
3. UUID(Nonce) 생성 구현
4. JSON 파싱 라이브러리 통합 또는 구현

#### Phase 2: API 통신 계층 수정
1. `WowTvSocket.cpp` REST API 호출 함수 구현
2. 신규 응답 구조체 정의 (`INFOPRODOCRES_V2`)
3. JSON 응답 파싱 로직 구현
4. 에러 처리 로직 구현 (HTTP Status Code 기반)

#### Phase 3: 시나리오 로직 수정
1. `ALLAT_getOrderInfo` 일반결제 흐름으로 단순화
2. 쿠폰/보너스캐시 분기 로직 추가
3. 상품유형별 분기 로직 구현
4. TTS 멘트 전면 교체

#### Phase 4: 카드입력 모듈 수정
1. NH농협카드 안내 멘트 추가
2. 유효기간 입력 순서 변경 (년월 → 년월 순서 유지)
3. 암호화 동의 멘트 추가

#### Phase 5: 테스트 및 검증
1. QA 서버 연동 테스트 (`devswbillapi.wowtv.co.kr`)
2. 각 시나리오 분기별 단위 테스트
3. 통합 테스트 및 운영 전환

#### ⚠️ 추후 개발 예정 (별도 세션)

> 빌키(간편결제키) 등록 프로세스는 현재 개발 범위에서 제외됨

| 항목 | 설명 |
|------|------|
| **빌키 동의 단계** | "동의하시면 1번, 일반결제 하시려면 2번" 멘트 |
| **빌키 발급 API** | `Allat_Get_FixKey_host()` → `AllatArsGetFixKeyProcess()` |
| **AUTH_TYPE** | 41(비인증), 42(구인증), 43(부분인증) |
| **빌키 저장** | `m_szbill_key`, DB(setPayLog) |

### 6.2 신규 파일 구조

```cpp
// RestApiUtil.h
#pragma once

// HMAC-SHA256 서명 생성
std::string GenerateSignature(
    const std::string& appId,
    const std::string& appKeyBase64,
    const std::string& method,
    const std::string& timestamp,
    const std::string& nonce);

// Authorization 헤더 생성
std::string BuildAuthHeader(
    const std::string& appId,
    const std::string& signature,
    const std::string& nonce,
    const std::string& timestamp);

// Base64 인코딩/디코딩
std::string Base64Encode(const unsigned char* data, size_t len);
std::vector<unsigned char> Base64Decode(const std::string& encoded);

// UUID 생성 (Nonce용)
std::string GenerateUUID();

// UNIX Timestamp 생성 (UTC)
std::string GetUnixTimestamp();
```

```cpp
// PaymentInfoResponse.h (신규 응답 구조체)
#pragma pack(push, 1)
typedef struct Payment_Info_Response_V2
{
    char m_szMemberId[40];           // 회원 UUID
    char m_szOrderNo[20];            // 주문번호 (Int64 → String 변환)
    char m_szNickName[24];           // 필명
    int  m_nPackageId;               // 패키지ID
    char m_szItemName[104];          // 상품명
    char m_szCategoryId2nd[8];       // 중분류 카테고리
    char m_szPgCode[4];              // PG코드 (A/P)
    char m_szMerchantId[44];         // 가맹점ID
    char m_szMallIdSimple[44];       // 간편결제용 상점ID
    char m_szMallIdGeneral[44];      // 일반결제용 상점ID
    int  m_nPayAmt;                  // 실결제금액
    char m_szCouponUseFlag[4];       // 쿠폰존재여부 (Y/N)
    char m_szCouponName[54];         // 쿠폰명
    char m_szBonusCashUseFlag[4];    // 보너스캐시 여부 (Y/N)
    int  m_nBonusCashUseAmt;         // 보너스캐시 금액
    int  m_nPurchaseAmt;             // 상품원가 (할인전)
    char m_szNotiUrlSimple[204];     // Notification URL (간편) - ⚠️ 미사용 (Noti 비활성화)
    char m_szNotiUrlGeneral[204];    // Notification URL (일반) - ⚠️ 미사용 (Noti 비활성화)
    char m_szBillKey[132];           // 빌키
    char m_szBillPassword[132];      // 빌키 비밀번호
    char m_szCardCompany[34];        // 카드사명
    char m_szPayAgreeFlag[4];        // 결제동의여부
    char m_szPurchaseLimitFlag[4];   // 구매제한여부 (1-6)
    int  m_nMemberState;             // 고객상태 (1:비회원, 2:유료, 3:기구매)
} PAYMENTINFORES_V2;
#pragma pack(pop)
```

### 6.3 신규 시나리오 함수 설계

```cpp
// 일반결제 시나리오 메인 함수
int ALLAT_GeneralPaymentScenario(int state)
{
    switch (state)
    {
    case 0:  // 인사멘트
        // "안녕하세요? 한국경제TV ARS 간편결제입니다"
        break;

    case 10: // 고객정보 조회 후 처리
        // REST API 응답 결과 확인
        // purchaseLimitFlag 검사
        break;

    case 20: // 결제금액 안내 (쿠폰/보너스캐시 분기)
        // CouponUseFlag, BonusCashUseFlag 조합으로 4가지 분기
        break;

    case 30: // 투자 유의사항 안내
        break;

    case 40: // 상품유형별 안내 (categoryId_2nd 기반)
        break;

    case 50: // 동의 확인 (1: 예, 2: 아니오)
        break;

    case 60: // 카드입력 진행
        return ALLAT_CardInput(0);

    case 70: // 종료 처리
        return ALLAT_ExitSvc(0);
    }
}
```

---

## 7. 리스크 및 고려사항

### 7.1 기술적 리스크

| 리스크 | 영향도 | 대응방안 |
|--------|--------|----------|
| **HMAC-SHA256 구현 오류** | 상 | OpenSSL 검증된 라이브러리 활용 |
| **JSON 파싱 실패** | 상 | cJSON 또는 RapidJSON 라이브러리 채택 |
| **시간 동기화 문제** | 중 | NTP 서버 동기화, 서버 시간차 5분 이내 보장 |
| **인코딩 불일치** | 중 | UTF-8 일관 적용, EUC-KR 변환 모듈 제거 |
| **네트워크 타임아웃** | 중 | 30초 타임아웃, 재시도 로직 구현 |

### 7.2 비즈니스 리스크

| 리스크 | 영향도 | 대응방안 |
|--------|--------|----------|
| **기존 정기결제 고객 영향** | 상 | 별도 시나리오 유지 또는 점진적 전환 |
| **DNIS 라우팅 변경** | 중 | 신규 DNIS 할당 또는 기존 DNIS 재활용 검토 |
| **TTS 멘트 검수** | 하 | 운영팀 검수 프로세스 진행 |

### 7.3 운영 고려사항

1. **병행 운영**: 기존 정기결제 시나리오와 신규 일반결제 시나리오 병행 운영 검토
2. **롤백 전략**: QA 테스트 완료 전 운영 배포 금지, 롤백 스크립트 준비
3. **모니터링**: API 호출 성공률, 응답시간 모니터링 대시보드 구성
4. **로그 강화**: 신규 API 호출 로그 상세화 (요청/응답 전문 기록)

### 7.4 테스트 체크리스트

#### 7.4.1 API 연동 테스트
- [ ] HMAC-SHA256 서명 생성 검증
- [ ] Authorization 헤더 형식 검증
- [ ] Nonce 재사용 방지 검증
- [ ] 타임스탬프 5분 이내 검증
- [ ] JSON 응답 파싱 검증 (모든 필드)
- [ ] HTTP 에러 코드별 처리 검증 (400, 401, 404, 500)

#### 7.4.2 시나리오 테스트
- [ ] 할인 없음 케이스
- [ ] 쿠폰만 있는 케이스
- [ ] 보너스캐시만 있는 케이스
- [ ] 쿠폰 + 보너스캐시 케이스
- [ ] 일반상품 안내 멘트
- [ ] 태블릿 제공 상품 안내 멘트
- [ ] 교육상품 안내 멘트
- [ ] purchaseLimitFlag 2-6 케이스 (구매 불가 처리)

#### 7.4.3 카드입력 테스트
- [ ] NH농협카드 안내 멘트 재생
- [ ] 카드번호 입력 (13-16자리)
- [ ] 유효기간 입력 (YYMM)
- [ ] 암호화 동의 멘트 재생
- [ ] 생년월일/법인번호 입력

---

## 부록 A: 현행 코드 주요 분기점 참조

### A.1 ALLAT_getOrderInfo 분기 구조 (Line 2223-2903)

```cpp
// state=0: 주문정보 조회 결과 확인
if (pScenario->m_DBAccess == -1 || pScenario->m_bDnisInfo == -1) → 시스템 장애
if (pScenario->m_bDnisInfo == 0) → 주문정보 없음
if (strcmp(pScenario->m_szretval,"6020")==0) → 와우캐시 결제완료
if (strcmp(pScenario->m_szrenew_flag, "Y") == 0) → 기존 동의고객 (state=9)

// state=9: 정기결제 상태 분기 (삭제 대상)
if (strcmp(pScenario->m_szsub_status, "0") == 0) → 첫결제
if (strcmp(pScenario->m_szsub_status, "1") == 0) → 이용중
if (strcmp(pScenario->m_szsub_status, "2") == 0) → 해지후 재결제
```

### A.2 DNIS별 특수 처리 목록

| DNIS | 서비스 | 특수처리 |
|------|--------|----------|
| 4542, 6617 | 와우 글로벌 파트너스 | 정보제공 동의 멘트 커스텀 |
| 6625 | 와우 아카데미 | 교육상품 환불 규정 |
| 5037 | 정기결제 | 5만5천원 고정 안내 |
| 5013 | 교육특강 | 44만원 고정, 환불불가 |
| 5039, 4618, 5018, 4577, 4639 | 예약결제 | 해지수수료 안내 |
| 4479, 4649, 4542, 5094, 4527, 4609 | 예약결제 | 환불불가 안내 |

---

## 부록 B: 신규 API 에러 코드 처리

| HTTP Status | Error Code | 설명 | 처리 |
|-------------|------------|------|------|
| 200 | 0 | 성공 | 정상 처리 |
| 400 | 997 | 요청 파라미터 오류 | 시스템 장애 안내 |
| 401 | 998 | 인증 실패 | 시스템 장애 안내 |
| 403 | 993 | 권한 없음/IP 미허용 | 시스템 장애 안내 |
| 404 | 911 | 사용자 없음 | 주문정보 없음 안내 |
| 404 | 996 | 리소스 없음 | 시스템 장애 안내 |
| 500 | 999 | 시스템 내부 오류 | 시스템 장애 안내 |

---

**문서 종료**

---

| 버전 | 일자 | 작성자 | 변경내용 |
|------|------|--------|----------|
| 1.0 | 2026-01-13 | Claude | 최초 작성 |
| 1.1 | 2026-01-14 | Claude | 빌키(간편결제키) 등록 프로세스 제외 - 일반결제만 구현, 추후 개발 예정으로 분리 |

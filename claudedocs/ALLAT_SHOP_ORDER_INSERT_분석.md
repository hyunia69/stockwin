# ALLAT_SHOP_ORDER 테이블 INSERT 데이터 매핑 분석

## 개요

`RegOrderInfo` 함수 (`ADODB.cpp:1642-1726`)에서 `ALLAT_SHOP_ORDER` 테이블에 상품정보를 INSERT합니다.

---

## 컬럼별 데이터 매핑

| 컬럼명 | 데이터 소스 | 값 예시 | 비고 |
|--------|------------|---------|------|
| **MX_ISSUE_NO** | `infoOrder.m_szorder_no` | `202601210105272` | 주문번호 |
| **MX_NAME** | 서브쿼리: `ALLAT_SHOP_ADMIN.SHOP_NAME` | `한국경제TV` | shop_id로 조회 |
| **MX_ID** | `infoOrder.m_szshop_id` | `T_arsstockwin` | 가맹점ID |
| **MX_OPT** | 서브쿼리: `ALLAT_SHOP_ADMIN.SHOP_KEY` | - | shop_id로 조회 |
| **ADMIN_ID** | 서브쿼리: `ALLAT_SHOP_ADMIN.ADMIN_ID` | - | shop_id로 조회 |
| **CC_NAME** | `infoOrder.m_szcc_name` | `eda9c2e5-7ee5-...` | 회원ID(UUID) |
| **CC_PORD_DESC** | `m_szcc_pord_desc ^ m_szcc_pord_code` | `주식비타민 유료 1개월^PROD001` | 상품명^상품코드 |
| **CC_EMAIL** | 고정값 | `''` | 빈값 |
| **AMOUNT** | `infoOrder.m_szamount` | `20000` | 결제금액 |
| **PHONE_NO** | `infoReq.m_szHP_NO` | `01012345678` | 고객 휴대전화 |
| **AUTH_NO** | 고정값 | `''` | 빈값 (결제 후 업데이트) |
| **REQUEST_TYPE** | 고정값 | `'CIA'` | ARS 결제 타입 |
| **REPLY_CODE** | 고정값 | `''` | 빈값 (결제 후 업데이트) |
| **REPLY_MESSAGE** | 고정값 | `''` | 빈값 (결제 후 업데이트) |
| **REG_DATE** | SQL 함수 | `getdate()` | 등록일시 |
| **AUTO_INPUT** | 고정값 | `'N'` | 자동입력 여부 |
| **PAYMENT_CODE** | 고정값 | `'0'` | 결제코드 |
| **ADMIN_NAME** | 서브쿼리: `ALLAT_SHOP_ADMIN.ADMIN_NAME` | - | shop_id로 조회 |
| **PAY_DATE** | 고정값 | `''` | 빈값 (결제 후 업데이트) |
| **POINT_YN** | 고정값 | `'N'` | 포인트 사용여부 |

---

## 데이터 흐름도

```
PayLetter API 응답 (WowTvSocket.cpp)
    │
    ├─ plInfo.orderNo ────────────► m_szorder_no ──► MX_ISSUE_NO
    ├─ plInfo.mallIdGeneral ──────► m_szshop_id ───► MX_ID
    │                                    │
    │                                    └──► 서브쿼리 ──► MX_NAME, MX_OPT, ADMIN_ID, ADMIN_NAME
    │
    ├─ plInfo.memberKey ──────────► m_szcc_name ───► CC_NAME
    ├─ plInfo.itemName ───────────► m_szcc_pord_desc ► CC_PORD_DESC
    ├─ plInfo.itemCode ───────────► m_szcc_pord_code ► CC_PORD_DESC (결합)
    ├─ plInfo.payAmount ──────────► m_szamount ────► AMOUNT
    │
    └─ 요청 전문 (INFOPRODOCREQ)
         └─ m_szHP_NO ────────────────────────────► PHONE_NO
```

---

## 서브쿼리 의존성 (ALLAT_SHOP_ADMIN 테이블)

INSERT 시 4개 컬럼이 `ALLAT_SHOP_ADMIN` 테이블에서 서브쿼리로 조회됩니다:

| INSERT 컬럼 | 조회 컬럼 | 조회 조건 |
|-------------|----------|-----------|
| MX_NAME | SHOP_NAME | `WHERE SHOP_ID = '{shop_id}'` |
| MX_OPT | SHOP_KEY | `WHERE SHOP_ID = '{shop_id}'` |
| ADMIN_ID | ADMIN_ID | `WHERE SHOP_ID = '{shop_id}'` |
| ADMIN_NAME | ADMIN_NAME | `WHERE SHOP_ID = '{shop_id}'` |

**주의**: `ALLAT_SHOP_ADMIN`에 해당 `SHOP_ID` 레코드가 없거나 컬럼이 NULL이면 INSERT 실패

---

## 관련 구조체

### INFOPRODOCRES (WowTvSocket.h:31-42)

```cpp
typedef struct Info_PRODOC_RES {
    char m_szorder_no[70 + 1];      // 주문번호 → MX_ISSUE_NO
    char m_szshop_id[32 + 1];       // 가맹점ID → MX_ID, 서브쿼리 조건
    char m_szcc_name[64 + 1];       // 회원ID → CC_NAME
    char m_szcc_pord_desc[255 + 1]; // 상품명 → CC_PORD_DESC
    char m_szpartner_nm[256 + 1];   // 필명 (INSERT에 미사용)
    char m_szcc_pord_code[50 + 1];  // 상품코드 → CC_PORD_DESC 결합
    char m_szamount[10 + 1];        // 결제금액 → AMOUNT
} INFOPRODOCRES;
```

### INFOPRODOCREQ (WowTvSocket.h:23-29)

```cpp
typedef struct Info_PRODOC_REQ {
    char m_szDNIS[4 + 1];   // 회선번호
    char m_szHP_NO[12 + 1]; // 고객 휴대전화 → PHONE_NO
} INFOPRODOCREQ;
```

---

## 참고사항

### 필명(m_szpartner_nm) 미사용
로그에서 `필명: SW2637262148`이 출력되지만, `RegOrderInfo`의 INSERT 쿼리에서 이 값은 사용되지 않습니다.
필명을 저장하려면 별도 컬럼 추가 또는 기존 컬럼 활용이 필요합니다.

### MX_NAME NULL 오류 해결
`ALLAT_SHOP_ADMIN` 테이블에 해당 `SHOP_ID`(예: `T_arsstockwin`)의 `SHOP_NAME` 값이 없으면 INSERT 실패합니다.
반드시 `ALLAT_SHOP_ADMIN`에 가맹점 정보를 먼저 등록해야 합니다.

---

## 관련 파일

- `ADODB.cpp:1642-1726` - RegOrderInfo 함수
- `WowTvSocket.h:23-42` - 구조체 정의
- `WowTvSocket.cpp:889-1244` - PL_InfoOrderReq_Process 함수

# ALLAT ARS 결제 시스템 DB 스키마

> **Database**: arspg_web
> **Server**: 211.196.157.121:1433
> **조회일**: 2026-01-22

---

## 1. ALLAT 서버 설정

### 운영 서버 (기본값)

| 항목 | 값 |
|------|-----|
| **서버 주소** | `tx.mobilians.co.kr` |
| **호스트** | `tx.mobilians.co.kr` |
| **포트** | SSL: 443 / 일반: 80 |

### API URI 경로

| 기능 | URI |
|------|-----|
| **결제 승인** | `/servlet/AllatPay/pay/approval.jsp` |
| **제재(검증)** | `/servlet/AllatPay/pay/sanction.jsp` |
| **결제 취소** | `/servlet/AllatPay/pay/cancel.jsp` |
| **현금영수증 등록** | `/servlet/AllatPay/pay/cash_registry.jsp` |
| **현금영수증 승인** | `/servlet/AllatPay/pay/cash_approval.jsp` |
| **현금영수증 취소** | `/servlet/AllatPay/pay/cash_cancel.jsp` |
| **에스크로 확인** | `/servlet/AllatPay/pay/escrow_check.jsp` |

### 전체 승인 요청 URL
```
https://tx.mobilians.co.kr:443/servlet/AllatPay/pay/approval.jsp
```

### 테스트 서버 설정

> **참고**: ALLAT 테스트 서버 설정은 INI 파일에서 오버라이드 가능
> 일반적인 ALLAT 테스트 서버: `testpay.allatpay.com` (확인 필요)

```ini
[ALLAT_PAYMEMT]
ALLAT_ADDR=testpay.allatpay.com
ALLAT_HOST=testpay.allatpay.com
```

---

## 2. 테이블 목록

### ALLAT 관련 테이블
- `ALLAT_ASSIGN_DNIS` - DNIS 할당
- `ALLAT_PAY_LOG` - 결제 로그
- `ALLAT_SHOP_ADMIN` - 가맹점 관리
- `ALLAT_SHOP_ORDER` - 주문 정보
- `ALLAT_SHOP_USER` - 사용자 관리

### 공통 테이블
- `COMMON_DNIS_INFO` - DNIS 정보
- `COMMON_DNIS_MID` - DNIS-가맹점 매핑

---

## 3. 테이블 스키마

### 3.1 ALLAT_SHOP_ORDER (주문 정보)

| 컬럼 | 타입 | NULL | 설명 |
|------|------|------|------|
| MX_ISSUE_NO | varchar(70) | NOT NULL | 주문번호 (PK) |
| MX_NAME | varchar(50) | NOT NULL | 가맹점명 |
| MX_ID | varchar(32) | NOT NULL | 가맹점 ID |
| MX_OPT | varchar(50) | NOT NULL | CrossKey |
| ADMIN_ID | varchar(20) | NULL | 관리자 ID |
| CC_NAME | varchar(64) | NOT NULL | 고객명 |
| CC_PORD_DESC | varchar(255) | NULL | 상품설명 |
| CC_EMAIL | varchar(200) | NULL | 이메일 |
| AMOUNT | int | NOT NULL | 결제금액 |
| PHONE_NO | varchar(32) | NULL | 전화번호 |
| AUTH_NO | varchar(10) | NULL | 인증번호 |
| REQUEST_TYPE | char(3) | NOT NULL | 요청타입 |
| REPLY_CODE | varchar(20) | NULL | 응답코드 |
| REPLY_MESSAGE | varchar(255) | NULL | 응답메시지 |
| REG_DATE | datetime | NULL | 등록일시 |
| AUTO_INPUT | char(1) | NOT NULL | 자동입력 여부 |
| PAYMENT_CODE | char(1) | NULL | 결제상태 (0:미결제, 1:결제완료) |
| ADMIN_NAME | varchar(20) | NULL | 관리자명 |
| PAY_DATE | datetime | NULL | 결제일시 |
| POINT_YN | char(8) | NULL | 포인트 여부 |
| MX_ID2 | varchar(32) | NULL | 2차 가맹점 ID |
| ITEM_CODE | varchar(255) | NULL | 상품코드 |

### 3.2 ALLAT_PAY_LOG (결제 로그)

| 컬럼 | 타입 | NULL | 설명 |
|------|------|------|------|
| MX_SEQ | int | NOT NULL | 시퀀스 (PK) |
| MX_ISSUE_NO | varchar(70) | NOT NULL | 주문번호 |
| MX_ISSUE_DATE | varchar(14) | NOT NULL | 주문일시 (YYYYMMDDHHMMSS) |
| AMOUNT | int | NOT NULL | 결제금액 |
| INSTALLMENT | int | NULL | 할부개월 |
| REPLY_CODE | varchar(20) | NULL | 응답코드 |
| REPLY_MESSAGE | varchar(255) | NULL | 응답메시지 |
| REPLY_DATE | datetime | NULL | 응답일시 |
| PAYMENT_TYPE | char(1) | NULL | 결제타입 |
| MX_ID | varchar(32) | NULL | 가맹점 ID |
| SEQ_NO | varchar(32) | NULL | ALLAT 거래번호 |

### 3.3 ALLAT_SHOP_ADMIN (가맹점 관리)

| 컬럼 | 타입 | NULL | 설명 |
|------|------|------|------|
| ADMIN_ID | varchar(20) | NOT NULL | 관리자 ID (PK) |
| ADMIN_PASS | varchar(15) | NOT NULL | 비밀번호 |
| ADMIN_NAME | varchar(20) | NOT NULL | 관리자명 |
| SHOP_ID | varchar(32) | NULL | 가맹점 ID |
| SHOP_NAME | varchar(50) | NULL | 가맹점명 |
| SHOP_KEY | varchar(50) | NULL | 가맹점 키 (CrossKey) |
| ARS_INTRO | varchar(200) | NULL | ARS 인트로 멘트 |
| ARS_DNIS | char(4) | NULL | ARS 전화번호 |
| ARS_TYPE | varchar(3) | NULL | ARS 타입 |
| REGDATE | datetime | NULL | 등록일시 |
| ADMIN_LVL | char(1) | NULL | 관리자 레벨 |
| AUTH_TYPE | varchar(2) | NULL | 인증타입 (01/02/03) |

### 3.4 ALLAT_SHOP_USER (사용자 관리)

| 컬럼 | 타입 | NULL | 설명 |
|------|------|------|------|
| ADMIN_ID | varchar(20) | NOT NULL | 관리자 ID (PK) |
| ADMIN_PASS | varchar(15) | NOT NULL | 비밀번호 |
| ADMIN_NAME | varchar(20) | NOT NULL | 관리자명 |
| SHOP_ID | varchar(32) | NULL | 가맹점 ID |
| ADMIN_LVL | char(1) | NULL | 관리자 레벨 |
| REGDATE | datetime | NULL | 등록일시 |

### 3.5 ALLAT_ASSIGN_DNIS (DNIS 할당)

| 컬럼 | 타입 | NULL | 설명 |
|------|------|------|------|
| DNIS | char(4) | NOT NULL | DNIS (PK) |

### 3.6 COMMON_DNIS_MID (DNIS-가맹점 매핑)

| 컬럼 | 타입 | NULL | 설명 |
|------|------|------|------|
| ARS_DNIS | varchar(32) | NOT NULL | ARS 전화번호 (PK) |
| SHOP_ID | varchar(20) | NOT NULL | 가맹점 ID (PK) |
| ARS_ADMIN | varchar(20) | NULL | ARS 관리자 |
| CREATE_DT | datetime | NULL | 생성일시 |
| WRITE_ID | varchar(20) | NULL | 작성자 ID |
| WRITE_DT | datetime | NULL | 작성일시 |
| SHOP_PW | varchar(50) | NULL | 가맹점 비밀번호 |
| **SHOP_RET_URL** | varchar(2048) | NULL | Noti 콜백 URL (⚠️ 비활성화됨) |
| **URL_YN** | varchar(1) | NULL | 콜백 사용여부 |
| SHOP_DESCRIPT | varchar(1024) | NULL | 가맹점 설명 |
| USE_YN | varchar(1) | NULL | 사용여부 |
| AUTH_TYPE | char(2) | NULL | 인증타입 |
| EXT_NO_YN | char(1) | NULL | 확장번호 여부 |
| INVALID_NO_YN | char(1) | NOT NULL | 무효번호 여부 |
| SHOP_REQ_URL | varchar(2048) | NULL | 요청 URL |
| SHOP_AUTH_NO | varchar(6) | NULL | 인증번호 |
| REQ_URL_YN | char(1) | NOT NULL | 요청 URL 여부 |

### 3.7 COMMON_DNIS_INFO (DNIS 정보)

| 컬럼 | 타입 | NULL | 설명 |
|------|------|------|------|
| ARS_DNIS | varchar(32) | NOT NULL | ARS 전화번호 (PK) |
| DLL_NAME | varchar(1024) | NULL | DLL 파일명 |
| CALL_CNT | int | NULL | 호출 횟수 |
| PG_CODE | varchar(12) | NULL | PG 코드 |
| ADMIN_ID | varchar(20) | NULL | 관리자 ID |
| WRITE_DATE | datetime | NOT NULL | 작성일시 |
| WRITE_ID | varchar(20) | NULL | 작성자 ID |
| WRITE_DT | datetime | NOT NULL | 수정일시 |
| ARS_TYPE | char(3) | NULL | ARS 타입 |
| DNIS_DESCRIPT | varchar(1024) | NULL | DNIS 설명 |
| SERVOCE_NAME | varchar(100) | NULL | 서비스명 |
| USE_YN | char(1) | NULL | 사용여부 |
| ERROR_YN | char(1) | NULL | 오류여부 |
| EXT_NO_YN | char(1) | NULL | 확장번호 여부 |
| INVALID_NO_YN | char(1) | NOT NULL | 무효번호 여부 |

---

## 4. 저장 프로시저

### 4.1 sp_getAllatOrderInfoByTel4 (주요)

**용도**: 전화번호 + DNIS로 주문 정보 조회 (가장 상세)

```sql
CREATE PROCEDURE sp_getAllatOrderInfoByTel4
   @PHONE_NO varchar(32),
   @DNIS     varchar(12),
   @SHOP_ID1  varchar(32),
   @SHOP_ID2  varchar(50)
AS
BEGIN
  SELECT TOP 1 mx_issue_no,
     CONVERT(VARCHAR, A.REG_DATE, 112) + REPLACE(CONVERT(VARCHAR(8), A.REG_DATE, 114), ':','') As REG_DATE,
     A.mx_id,
     A.mx_name,
     A.cc_name,
     A.cc_pord_desc,
     A.amount,
     A.phone_no,
     A.CC_EMAIL,
     A.MX_ID as SHOP_ID1,
     A.MX_ID2 as SHOP_ID2,
     B.SHOP_RET_URL as NOTI_URL,
     B2.SHOP_RET_URL as NOTI_URL2,
     B.URL_YN,
     B.SHOP_RET_URL,
     C.SHOP_ID as CLIENT_ID,
     C2.SHOP_ID as CLIENT_ID2,
     ISNULL(C.AUTH_TYPE,'') as AUTH_TYPE,
     ISNULL(C2.AUTH_TYPE,'') as AUTH_TYPE2,
     ISNULL(C.SHOP_KEY,'') as MX_OPT,
     ISNULL(C2.SHOP_KEY,'') as MX_OPT2,
     A.ITEM_CODE
 FROM dbo.ALLAT_SHOP_ORDER A
 INNER JOIN dbo.COMMON_DNIS_MID B ON A.MX_ID = B.SHOP_ID AND B.ARS_DNIS = @DNIS
 INNER JOIN dbo.COMMON_DNIS_MID B2 ON A.MX_ID2 = B2.SHOP_ID AND B2.ARS_DNIS = @DNIS
 INNER JOIN dbo.ALLAT_SHOP_ADMIN C ON A.MX_ID = C.SHOP_ID
 LEFT OUTER JOIN dbo.ALLAT_SHOP_ADMIN C2 ON A.MX_ID2 = C2.SHOP_ID
 WHERE DATEDIFF(dd, A.REG_DATE, GETDATE()) <= 7 AND
      A.MX_ID = @SHOP_ID1 AND
      A.MX_ID2 = @SHOP_ID2 AND
      A.payment_code = '0' AND
      A.phone_no = @PHONE_NO
 ORDER BY REG_DATE DESC
END
```

### 4.2 sp_getAllatOrderInfoByTel2

**용도**: 전화번호 + DNIS로 주문 정보 조회

```sql
CREATE PROCEDURE sp_getAllatOrderInfoByTel2
   @PHONE_NO varchar(32),
   @DNIS     varchar(12)
AS
BEGIN
  SELECT TOP 1 mx_issue_no,
     A.mx_id,
     A.mx_opt,
     A.mx_name,
     A.cc_name,
     A.cc_pord_desc,
     A.amount,
     A.phone_no,
     A.CC_EMAIL,
     B.SHOP_ID,
     B.URL_YN,
     B.SHOP_RET_URL
 FROM dbo.ALLAT_SHOP_ORDER A
 INNER JOIN dbo.COMMON_DNIS_MID B ON A.mx_id = B.SHOP_ID
 WHERE DATEDIFF(dd, A.reg_date, GETDATE()) <= 7 AND
      B.ARS_DNIS = @DNIS AND
      A.payment_code = '0' AND
      A.phone_no = @PHONE_NO
 ORDER BY reg_date DESC
END
```

### 4.3 sp_getAllatOrderInfoByTel

**용도**: 전화번호로 주문 정보 조회 (기본)

```sql
CREATE PROC dbo.sp_getAllatOrderInfoByTel
@PHONE_NO VARCHAR(20)
AS
BEGIN
 SELECT TOP 1 mx_issue_no,
     mx_id,
     mx_opt,
     mx_name,
     cc_name,
     cc_pord_desc,
     amount,
     phone_no
 FROM dbo.ALLAT_SHOP_ORDER
 WHERE DATEDIFF(dd, reg_date, GETDATE()) <= 7 AND
      payment_code = '0' AND
      phone_no = @PHONE_NO
 ORDER BY reg_date DESC
END
```

### 4.4 sp_getAllatOrderInfoBySMS2

**용도**: 인증번호 + DNIS로 주문 정보 조회

```sql
CREATE PROCEDURE sp_getAllatOrderInfoBySMS2
   @AUTH_NO varchar(12),
   @DNIS    varchar(12)
AS
BEGIN
 SELECT TOP 1 mx_issue_no,
     mx_id,
     mx_opt,
     mx_name,
     cc_name,
     cc_pord_desc,
     amount,
     phone_no,
     A.CC_EMAIL,
     B.SHOP_ID,
     B.URL_YN,
     B.SHOP_RET_URL
 FROM dbo.ALLAT_SHOP_ORDER A
 INNER JOIN dbo.COMMON_DNIS_MID B ON A.mx_id = B.SHOP_ID
 WHERE DATEDIFF(dd, reg_date, GETDATE()) <= 7 AND
      B.ARS_DNIS = @DNIS AND
      payment_code = '0' AND
      auth_no = @AUTH_NO
 ORDER BY reg_date DESC
END
```

### 4.5 sp_getAllatOrderCancelInfo

**용도**: 결제 취소 대상 주문 조회

```sql
CREATE PROC dbo.sp_getAllatOrderCancelInfo
@MX_ISSUE_NO VARCHAR(32),
@MX_ID VARCHAR(32)
AS
BEGIN
  SELECT a.MX_ISSUE_NO,
       a.MX_ID,
       a.CC_NAME,
       a.CC_PORD_DESC,
       a.AMOUNT,
       a.MX_OPT,
       b.SEQ_NO,
       b.REPLY_DATE,
       b.MX_ISSUE_DATE
  FROM ALLAT_SHOP_ORDER a, ALLAT_PAY_LOG b
  WHERE a.mx_issue_no = @MX_ISSUE_NO AND
       a.mx_id = @MX_ID AND
       a.payment_code = '1' AND
       a.reply_code = '0000' AND
       a.mx_issue_no = b.mx_issue_no AND
       b.mx_id IN (a.mx_id, a.mx_id2) AND
       b.reply_code = '0000'
END
```

### 4.6 sp_getAllatShopDnisInfo

**용도**: DNIS별 ARS 설정 조회

```sql
CREATE PROC dbo.sp_getAllatShopDnisInfo
@DNIS CHAR(4)
AS
BEGIN
  SELECT ARS_INTRO, ARS_TYPE
  FROM dbo.ALLAT_SHOP_ADMIN
  WHERE ARS_DNIS = @DNIS
END
```

### 4.7 sp_getAllatShopInfo

**용도**: 가맹점 정보 조회

```sql
CREATE PROC dbo.sp_getAllatShopInfo
@SHOP_ID VARCHAR(20)
AS
SELECT TOP 1 ADMIN_ID,
  ADMIN_NAME,
  SHOP_ID,
  SHOP_NAME,
  SHOP_KEY,
  ARS_INTRO,
  ARS_DNIS,
  ARS_TYPE
FROM dbo.ALLAT_SHOP_ADMIN
WHERE SHOP_ID = @SHOP_ID
```

### 4.8 기타 프로시저

| 프로시저 | 용도 |
|----------|------|
| `sp_getAllatAdminCntById` | 관리자 ID 중복 체크 |
| `sp_getAllatAssignedDnis` | 할당된 DNIS 목록 조회 |
| `sp_getAllatAuthNo` | 인증번호 생성 (MAX+1) |
| `sp_getAllatOrderNo` | 주문번호 생성 |
| `sp_getAllatOrderCancelInfo2` | 취소 정보 조회 (2차 가맹점 포함) |

---

## 5. 테이블 관계도

```
ALLAT_ASSIGN_DNIS (DNIS)
        │
        ▼
COMMON_DNIS_INFO (ARS_DNIS) ◄──── DLL/PG 설정
        │
        ▼
COMMON_DNIS_MID (ARS_DNIS, SHOP_ID) ◄──── URL_YN, SHOP_RET_URL (Noti)
        │
        ├──► ALLAT_SHOP_ADMIN (SHOP_ID) ◄──── AUTH_TYPE, SHOP_KEY
        │
        └──► ALLAT_SHOP_ORDER (MX_ID, MX_ID2)
                    │
                    ▼
             ALLAT_PAY_LOG (MX_ISSUE_NO, MX_ID)
```

---

## 6. 주요 필드 설명

### 인증타입 (AUTH_TYPE)
| 코드 | 설명 |
|------|------|
| 01 | 비인증 (카드번호 + 유효기간) |
| 02 | 구인증 (카드번호 + 유효기간 + 생년월일 + 비밀번호) |
| 03 | 부분인증 (카드번호 + 유효기간 + 생년월일) |

### 결제상태 (PAYMENT_CODE)
| 코드 | 설명 |
|------|------|
| 0 | 미결제 |
| 1 | 결제완료 |

### 응답코드 (REPLY_CODE)
| 코드 | 설명 |
|------|------|
| 0000 | 성공 |
| 기타 | 오류 (REPLY_MESSAGE 참조) |

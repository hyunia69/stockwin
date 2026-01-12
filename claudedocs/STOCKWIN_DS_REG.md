# StockWin 결제 시나리오 등록 API 설계서

**문서번호**: STOCKWIN_DS_REG
**버전**: 1.0
**작성일**: 2026-01-12
**대상 파일**: `db/stockwin_item_update.asp`

---

## 1. 개요

### 1.1 목적

한국경제TV 상품 관리 시스템에서 페이엠솔루션 ARS 결제 서비스 시스템에 결제 시나리오를 자동 등록하기 위한 ASP 구현 설계서입니다.

### 1.2 참조 문서

- `claudedocs/NEW_SPEC_DS_REG_SCENARIO.md`: API 명세서
- `db/allat_ars_order.asp`: 기존 구현 참조

---

## 2. 시스템 구성

### 2.1 DB 연결 정보

| 항목 | 값 |
|------|-----|
| Data Source | 211.196.157.121 |
| Database | arspg_web |
| Provider | SQLOLEDB.1 |

### 2.2 아키텍처

```
[한국경제TV 상품관리]
        │
        ▼ HTTP POST
[stockwin_item_update.asp]
        │
        ├─ (1) 파라미터 검증
        ├─ (2) 중복 체크 (ARS_DNIS)
        ├─ (3) COMMON_DNIS_INFO 테이블 INSERT/UPDATE
        │
        ▼
[응답: ars_tel_no|Item_cd|Error_cd|message]
```

### 2.2 대상 테이블

**COMMON_DNIS_INFO**

| 컬럼명 | 타입 | NULL | 설명 |
|--------|------|------|------|
| ADMIN_ID | varchar(20) | YES | shop_id 저장 |
| ARS_DNIS | varchar(32) | NO | ARS 전화번호 (PK 역할) |
| ARS_TYPE | char(3) | YES | 시나리오 유형 (CQS/CIP) |
| CALL_CNT | int | YES | 호출 횟수 |
| DLL_NAME | varchar(1024) | YES | 시나리오 DLL 경로 |
| DNIS_DESCRIPT | varchar(1024) | YES | JSON 형태 추가정보 |
| ERROR_YN | char(1) | YES | 오류 여부 |
| EXT_NO_YN | char(1) | YES | 내선번호 여부 |
| INVALID_NO_YN | char(1) | NO | 무효번호 여부 |
| PG_CODE | varchar(12) | YES | PG사 코드 (allat 고정) |
| SERVOCE_NAME | varchar(250) | YES | 서비스/상품명 |
| USE_YN | char(1) | YES | 사용 여부 |
| WRITE_DATE | datetime | NO | 등록일시 |
| WRITE_DT | datetime | NO | 수정일시 |
| WRITE_ID | varchar(20) | YES | 등록자 ID |

---

## 3. 파라미터 매핑

### 3.1 요청 파라미터 → DB 필드 매핑

| 요청 파라미터 | DB 필드 | 변환 규칙 |
|--------------|---------|----------|
| `mode` | - | 검증용 (`hangung2^alphago_hankyung` 고정) |
| `shop_id` | ADMIN_ID | 직접 저장 |
| `ars_tel_no` | ARS_DNIS | 직접 저장 |
| `scenario_type` | ARS_TYPE | CQS 또는 CIP |
| `arrribute_type` | DNIS_DESCRIPT (JSON.attr) | JSON 내 attr 키로 저장 |
| `amount` | DNIS_DESCRIPT (JSON.amount) | JSON 내 amount 키로 저장 |
| `cc_pord_desc` | SERVOCE_NAME | 상품명 부분 추출 |
| `startdtm` | DNIS_DESCRIPT (JSON.startdtm) | JSON 내 startdtm 키로 저장 (선택) |
| - | PG_CODE | `allat` 고정값 |
| - | DLL_NAME | shop_id 기반 자동 설정 |
| - | USE_YN | `Y` 기본값 |
| - | INVALID_NO_YN | `N` 기본값 |
| - | WRITE_DATE | 현재 시간 |
| - | WRITE_DT | 현재 시간 |
| - | WRITE_ID | `SYSTEM` |

### 3.2 DLL_NAME 결정 로직

```
DLL_NAME = "ALLAT_StockWin_Billkey_Easy_New_Scenario.dll"  ' 모든 shop_id에 동일 적용 (추후 변경 예정)
```

### 3.3 DNIS_DESCRIPT JSON 형식

```json
{
    "amount": "99000",
    "attr": "1MONTH",
    "startdtm": "202209300850",
    "product_code": "PROD001^PROD002"
}
```

**필드 설명:**
- `amount`: 결제/안내 금액
- `attr`: 시나리오 속성 코드 (1MONTH, 1WEEK, NOFREE 등)
- `startdtm`: 사전예약 시작일시 (ADVRESERVD 속성일 때만)
- `product_code`: cc_pord_desc에서 추출한 상품코드

---

## 4. 처리 흐름

### 4.1 요청 처리 순서

```
1. 요청 파라미터 수신 (POST)
2. 필수 파라미터 검증
   ├─ mode 검증 → 실패 시 0001 반환
   ├─ shop_id 검증 → 실패 시 0002 반환
   ├─ ars_tel_no 검증 → 실패 시 0003 반환
   ├─ scenario_type 검증 → 실패 시 0004 반환
   ├─ arrribute_type 검증 → 실패 시 0005 반환
   ├─ amount 검증 → 실패 시 0006 반환
   └─ cc_pord_desc 검증 → 실패 시 0007 반환
3. 조건부 파라미터 검증
   └─ arrribute_type = "ADVRESERVD" → startdtm 필수 → 실패 시 0008 반환
4. ARS_DNIS 중복 체크
   └─ 중복 시 UPDATE / 미존재 시 INSERT
5. DB 저장
   └─ 실패 시 9001 반환
6. 성공 응답 반환 (0000)
```

### 4.2 파라미터 유효성 검증

| 파라미터 | 검증 규칙 |
|---------|----------|
| `mode` | `hangung2^alphago_hankyung` 정확히 일치 |
| `shop_id` | `arsstockwin`, `arsstockwin1`, `arsstockwin2` 중 하나 |
| `ars_tel_no` | 숫자만, 8~12자리 |
| `scenario_type` | `CQS` 또는 `CIP` |
| `arrribute_type` | 정의된 코드 목록 중 하나 |
| `amount` | 숫자만, 양수 |
| `startdtm` | `YYYYMMDDHHMM` 형식, 미래 일시 |

### 4.3 arrribute_type 유효값

**정기결제 속성:**
- `1MINTHSEAL`: 1개월 할인금액 후 정기결제
- `1MONTH`: 1개월 무료체험 후 정기결제
- `1WEEK`: 1주일 무료체험 후 정기결제
- `2WEEK`: 2주일 무료체험 후 정기결제
- `NOFREE`: 무료체험 없는 정기결제
- `ADVRESERVD`: 사전예약형

**일반/간편결제 속성:**
- `EDUCATION`: 교육형 시나리오
- `SERVICE`: 일반 서비스형
- `SERVICE_VARS`: 보이는 ARS 연동형
- `SELECT_MENU`: 파트너 선택형
- `CONSENT_MENT`: 개인정보 수집 동의형

---

## 5. 응답 규격

### 5.1 응답 형식

```
ars_tel_no|Item_cd|Error_cd|message
```

### 5.2 에러 코드 정의

| 코드 | 설명 | 상황 |
|------|------|------|
| `0000` | 성공 | 정상 등록/수정 완료 |
| `0001` | mode 오류 | mode 값 불일치 |
| `0002` | shop_id 누락 | shop_id 미전송 |
| `0003` | ars_tel_no 누락 | 전화번호 미전송 |
| `0004` | scenario_type 오류 | 유효하지 않은 시나리오 유형 |
| `0005` | arrribute_type 오류 | 유효하지 않은 속성 코드 |
| `0006` | amount 오류 | 금액 누락 또는 형식 오류 |
| `0007` | cc_pord_desc 누락 | 상품정보 미전송 |
| `0008` | startdtm 오류 | ADVRESERVD인데 시작일시 누락/형식오류 |
| `9001` | DB 오류 | 데이터베이스 저장 실패 |
| `9999` | 시스템 오류 | 예상치 못한 오류 |

### 5.3 응답 예시

**성공:**
```
0212345678|PROD001|0000|등록이 완료되었습니다.
```

**실패:**
```
0212345678||0005|유효하지 않은 속성 코드입니다.
```

---

## 6. 구현 상세

### 6.1 파일 구조

```
db/
└── stockwin_item_update.asp    # 신규 생성
```

### 6.2 주요 함수/섹션

```asp
<%
' ============================================
' 1. 설정 및 초기화
' ============================================
' - DB 연결 문자열
' - 로깅 설정

' ============================================
' 2. 요청 파라미터 수신
' ============================================
' - Request.Form() 또는 Request.QueryString()

' ============================================
' 3. 파라미터 검증 함수
' ============================================
Function ValidateMode(mode)
Function ValidateShopId(shop_id)
Function ValidateArsTelNo(ars_tel_no)
Function ValidateScenarioType(scenario_type)
Function ValidateAttributeType(attr_type)
Function ValidateAmount(amount)
Function ValidateStartDtm(startdtm, attr_type)

' ============================================
' 4. DLL 이름 결정 함수
' ============================================
Function GetDllName(shop_id)

' ============================================
' 5. JSON 생성 함수
' ============================================
Function BuildDescriptJson(amount, attr_type, startdtm, product_code)

' ============================================
' 6. 상품명 파싱 함수
' ============================================
Function ParseProductName(cc_pord_desc)
Function ParseProductCode(cc_pord_desc)

' ============================================
' 7. DB 처리
' ============================================
' - 중복 체크: SELECT COUNT(*) FROM COMMON_DNIS_INFO WHERE ARS_DNIS = ?
' - INSERT 또는 UPDATE 실행

' ============================================
' 8. 응답 출력
' ============================================
Sub SendResponse(ars_tel_no, item_cd, error_cd, message)
%>
```

### 6.3 DB 쿼리

**중복 체크:**
```sql
SELECT COUNT(*) AS cnt FROM COMMON_DNIS_INFO WHERE ARS_DNIS = @ars_dnis
```

**INSERT:**
```sql
INSERT INTO COMMON_DNIS_INFO (
    ADMIN_ID, ARS_DNIS, ARS_TYPE, DLL_NAME, DNIS_DESCRIPT,
    PG_CODE, SERVOCE_NAME, USE_YN, INVALID_NO_YN,
    WRITE_DATE, WRITE_DT, WRITE_ID
) VALUES (
    @admin_id, @ars_dnis, @ars_type, @dll_name, @dnis_descript,
    'allat', @service_name, 'Y', 'N',
    GETDATE(), GETDATE(), 'SYSTEM'
)
```

**UPDATE:**
```sql
UPDATE COMMON_DNIS_INFO SET
    ADMIN_ID = @admin_id,
    ARS_TYPE = @ars_type,
    DLL_NAME = @dll_name,
    DNIS_DESCRIPT = @dnis_descript,
    SERVOCE_NAME = @service_name,
    WRITE_DT = GETDATE()
WHERE ARS_DNIS = @ars_dnis
```

---

## 7. 보안 고려사항

### 7.1 입력값 검증

- SQL Injection 방지: 파라미터화된 쿼리 사용
- XSS 방지: 입력값 이스케이프 처리
- 숫자 필드 타입 검증

### 7.2 로깅

- 모든 요청/응답 로깅
- 민감정보(카드정보 등) 마스킹
- 로그 파일 경로: `.\logs\YYYY-MM-DD.log`

---

## 8. 테스트 시나리오

### 8.1 정상 케이스

| 테스트 | 입력 | 예상 결과 |
|--------|------|----------|
| 정기결제 등록 | shop_id=arsstockwin2, scenario_type=CQS, attr=1MONTH | 0000 |
| 일반결제 등록 | shop_id=arsstockwin, scenario_type=CIP, attr=SERVICE | 0000 |
| 사전예약 등록 | attr=ADVRESERVD, startdtm=202601150900 | 0000 |
| 기존 DNIS 수정 | 동일 ars_tel_no로 재요청 | 0000 (UPDATE) |

### 8.2 오류 케이스

| 테스트 | 입력 | 예상 결과 |
|--------|------|----------|
| mode 불일치 | mode=invalid | 0001 |
| 필수값 누락 | shop_id 미전송 | 0002 |
| 잘못된 scenario_type | scenario_type=XXX | 0004 |
| ADVRESERVD인데 startdtm 누락 | attr=ADVRESERVD, startdtm 없음 | 0008 |

---

## 9. 테스트 환경

### 9.1 API 엔드포인트

| 환경 | URL |
|------|-----|
| 개발 | `https://www.arspg.co.kr/ars/allat/dev/db/stockwin_item_update.asp` |

### 9.2 테스트용 ARS_DNIS

| DNIS | 용도 |
|------|------|
| `6690` | 테스트용 1 |
| `6691` | 테스트용 2 |

### 9.3 테스트 예시

```bash
# 정상 등록 테스트
curl -X POST "https://www.arspg.co.kr/ars/allat/dev/db/stockwin_item_update.asp" \
  -d "mode=hangung2^alphago_hankyung" \
  -d "shop_id=arsstockwin2" \
  -d "ars_tel_no=6690" \
  -d "scenario_type=CQS" \
  -d "arrribute_type=1MONTH" \
  -d "amount=99000" \
  -d "cc_pord_desc=테스트상품^PROD001"
```

---

## 10. 변경 이력

| 버전 | 일자 | 내용 | 작성자 |
|------|------|------|--------|
| 1.0 | 2026-01-12 | 최초 작성 | Claude |
| 1.1 | 2026-01-12 | 테스트 환경 정보 추가 (DNIS: 6690, 6691) | Claude |

# stockwin_order_request.asp 구현 계획

## 요구사항 분석

### API 명세 (DASAM_STOCKWIN_ARS_API.md 섹션 5)
- **Endpoint**: `/stockwin_order_request.asp`
- **Method**: `POST`
- **Content-Type**: `application/json`
- **인코딩**: UTF-8

### 요청 파라미터 (10개)
| Parameter | 필수 | 설명 | 고정값 |
|-----------|------|------|--------|
| mode | O | 인증모드 | `hangung^alphago_hankyung` |
| shop_id | O | 상점 ID | `arsstockwin` |
| amount | O | 결제금액 | - |
| phone_no | O | 핸드폰번호 | - |
| verify_num | O | 인증번호 | `123456` |
| request_type | O | 요청타입 | `CIA` |
| alert_show | O | 알림 표시 | `N` |
| order_no | O | 주문번호 | - |
| cc_name | O | 필명/주문자명 | - |
| cc_pord_desc | O | 상품명^패키지번호 | - |

### DB 테이블: ALLAT_SHOP_ORDER
| 컬럼 | 타입 | 매핑 |
|------|------|------|
| MX_ISSUE_NO | VARCHAR(70) | 자동생성 (order_no 기반) |
| MX_NAME | VARCHAR(50) | cc_name |
| MX_ID | VARCHAR(32) | phone_no |
| MX_OPT | VARCHAR(50) | request_type |
| ADMIN_ID | VARCHAR(20) | shop_id |
| CC_NAME | VARCHAR(64) | cc_name |
| CC_PORD_DESC | VARCHAR(255) | cc_pord_desc |
| CC_EMAIL | VARCHAR(200) | - |
| AMOUNT | INT | amount |
| PHONE_NO | VARCHAR(32) | phone_no |
| AUTH_NO | VARCHAR(10) | verify_num |
| REQUEST_TYPE | CHAR(3) | request_type |
| REPLY_CODE | VARCHAR(20) | 응답코드 |
| REPLY_MESSAGE | VARCHAR(255) | 응답메시지 |
| REG_DATE | DATETIME | GETDATE() |
| AUTO_INPUT | CHAR(1) | 'N' |
| PAYMENT_CODE | CHAR(1) | - |
| ADMIN_NAME | VARCHAR(20) | - |
| PAY_DATE | DATETIME | - |
| POINT_YN | CHAR(8) | - |
| MX_ID2 | VARCHAR(32) | - |
| ITEM_CODE | VARCHAR(255) | 상품코드 (cc_pord_desc에서 추출) |

## 구현 방식

### 참조 코드: stockwin_item_update.asp
- UTF-8 인코딩 처리 함수 재사용
- DB 연결 방식 동일하게 적용
- 에러 처리 패턴 동일

### JSON 처리 차이점
- `BinaryRead` → JSON 문자열로 읽기
- VBScript JSON 파서 구현 필요 (간단한 파싱 함수)
- 응답도 JSON 형식으로 반환

## 구현 파일
- `db/stockwin_order_request.asp`

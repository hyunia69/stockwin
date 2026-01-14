# 다삼 주문 요청 API 명세서

## 1. 연동 API 목록

| URL | 설명 |
|-----|------|
| `http://api.mspay.co.kr/pg/allat/hangung_ars_order_letter.php` | 페이레터 → 페이엠 측으로 결제정보 전달 |

---

## 2. 결제정보 전달 API

페이레터에서 페이엠 측으로 결제정보를 전달하는 API입니다.

### 2.1 기본 정보

| 항목 | 값 |
|------|-----|
| **URL** | `http://api.mspay.co.kr/pg/allat/hangung_ars_order_letter.php` |
| **HTTP Method** | `POST` |
| **Content-Type** | `application/json` |

### 2.2 Request 파라미터

| No | Parameter | Type | 필수 | 설명 | 예시 |
|----|-----------|------|------|------|------|
| 1 | `mode` | String | M | 인증모드 (고정값) | `hangung^alphago_hankyung` |
| 2 | `shop_id` | String | M | 상점 ID (고정값) | `arsstockwin` |
| 3 | `amount` | String | M | 결제금액 | `100000` |
| 4 | `phone_no` | String | M | 핸드폰번호 | `010****1111` |
| 5 | `verify_num` | String | M | 인증번호 (고정값) | `123456` |
| 6 | `request_type` | String | M | 요청타입 (고정값) | `CIA` |
| 7 | `alert_show` | String | M | 알림 표시 여부 (고정값) | `N` |
| 8 | `order_no` | String | M | 주문번호 | `202601134267168` |
| 9 | `cc_name` | String | M | 필명 (주문자명) | `Sy` |
| 10 | `cc_pord_desc` | String | M | 상품명^패키지번호 | `감은숙 머니차트 풀패키지 1개월 19만원^102958` |

### 2.3 파라미터 상세 설명

#### 고정값 파라미터
- **mode**: 인증모드 식별자. 항상 `hangung^alphago_hankyung` 사용
- **shop_id**: 상점 식별자. 항상 `arsstockwin` 사용
- **verify_num**: 인증번호. 항상 `123456` 사용
- **request_type**: 요청 타입. 항상 `CIA` 사용
- **alert_show**: 알림 표시 여부. 항상 `N` 사용

#### 가변 파라미터
- **amount**: 결제 금액 (원 단위, 숫자 문자열)
- **phone_no**: 고객 핸드폰 번호 (마스킹 처리 가능: `010****1111`)
- **order_no**: 주문번호 (시스템에서 생성된 고유 번호)
- **cc_name**: 주문자 필명/이름
- **cc_pord_desc**: 상품 설명과 패키지 번호를 `^`로 구분하여 결합
  - 형식: `{상품명}^{패키지번호}`
  - 예: `감은숙 머니차트 풀패키지 1개월 19만원^102958`

### 2.4 Request 예시

```http
POST http://api.mspay.co.kr/pg/allat/hangung_ars_order_letter.php
Content-Type: application/json

{
  "mode": "hangung^alphago_hankyung",
  "shop_id": "arsstockwin",
  "amount": "190000",
  "phone_no": "010****4925",
  "verify_num": "123456",
  "request_type": "CIA",
  "alert_show": "N",
  "order_no": "202601134267168",
  "cc_name": "sy",
  "cc_pord_desc": "감은숙 머니차트 풀패키지 1개월 19만원^102958"
}
```

---

## 3. 비고

- 모든 파라미터는 문자열(String) 타입으로 전달
- 필수(M) 파라미터는 반드시 포함되어야 함
- 고정값 파라미터는 명시된 값을 그대로 사용할 것
- 핸드폰 번호는 개인정보 보호를 위해 마스킹 처리 가능

---

*문서 생성일: 2026-01-14*
*원본: DASAM_ORDER_REQ.docx*

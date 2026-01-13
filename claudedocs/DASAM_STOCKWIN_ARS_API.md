# 다솜 StockWin ARS 결제 API 명세서

**문서번호**: DASAM_STOCKWIN_ARS_API_v1.0
**버전**: 1.0
**최종수정일**: 2026-01-13
**작성자**: 다삼솔루션

---

## 1. 개요

### 1.1 목적

본 문서는 StockWin 서비스에서 다삼솔루션 ARS 결제 시스템과 연동하기 위한 API 명세입니다. 결제 유형 조회 및 시나리오 등록 기능을 제공합니다.

### 1.2 통신 규약

| 항목 | 값 |
|------|-----|
| 프로토콜 | HTTP 1.1 |
| 방식 | RESTful API |
| 인코딩 | UTF-8 |
| 응답 형식 | Plain Text (구분자: `\|`) |

### 1.3 서비스 흐름

```
[StockWin 상품관리]
        │
        ▼ (1) 결제 유형 조회 (GET)
[다솜 ARS 시스템]
        │
        ▼ (2) 결제유형 반환 (CQS/CIP/NDY)
[StockWin 상품관리]
        │
        ▼ (3) 시나리오 등록 요청 (POST)
[다솜 ARS 시스템]
        │
        ▼ (4) 등록 결과 반환
[StockWin 상품관리]
```

---

## 2. API 엔드포인트

### 2.1 서버 정보

| 환경 | 기본 URL |
|------|----------|
| 개발 | `https://www.arspg.co.kr/ars/allat/dev/db/` |
| 운영 | `https://www.arspg.co.kr/ars/allat/db/` |

### 2.2 API 목록

| API | Method | Endpoint | 설명 |
|-----|--------|----------|------|
| 결제유형 조회 | GET | `/stockwin_get_arstype.asp` | ARS 전화번호별 결제유형 조회 |
| 시나리오 등록 | POST | `/stockwin_item_update.asp` | 결제 시나리오 등록/수정 |

---

## 3. 결제유형 조회 API

### 3.1 기본 정보

| 항목 | 값 |
|------|-----|
| Endpoint | `/stockwin_get_arstype.asp` |
| Method | `GET` |
| Content-Type | `text/html; charset=UTF-8` |

### 3.2 요청 파라미터

| No | Parameter | Type | 필수 | 설명 |
|----|-----------|------|:----:|------|
| 1 | `mode` | String | O | 인증 모드<br>• 고정값: `hangung2^alphago_hankyung` |
| 2 | `ars_tel_no` | String | O | ARS 전화번호<br>• 숫자 4자리 (예: `2972`) |

### 3.3 요청 예시

```http
GET /ars/allat/db/stockwin_get_arstype.asp?mode=hangung2^alphago_hankyung&ars_tel_no=2972 HTTP/1.1
Host: www.arspg.co.kr
```

### 3.4 응답 형식

```
ars_tel_no|type_cd_list|Error_cd|message
```

### 3.5 응답 필드

| No | Field | Type | 설명 |
|----|-------|------|------|
| 1 | `ars_tel_no` | String | 요청한 ARS 전화번호 |
| 2 | `type_cd_list` | String | 결제유형 코드 (쉼표 구분)<br>• 실패 시 빈 값 |
| 3 | `Error_cd` | String | 결과 코드 |
| 4 | `message` | String | 결과 메시지 |

### 3.6 결제유형 코드

| 코드 | 설명 | 용도 |
|------|------|------|
| `CQS` | 정기결제용 | 자동 CID 인식형 |
| `CIP` | 일반/간편결제용 | CID + 휴대폰 번호 입력형 |
| `NDY` | 결제유형 미지정 | 전화번호만 배정된 상태 |
| `COD` | 레거시 시나리오 | 종목알파고 전용 (미사용) |

### 3.7 응답 예시

**성공 (단일 유형)**
```
2972|CQS|0000|결제유형획득에 성공하셨습니다.
```

**성공 (복수 유형)**
```
2972|CQS,CIP|0000|결제유형획득에 성공하셨습니다.
```

**실패 (전화번호 없음)**
```
2972||9002|해당 전화번호를 찾을 수 없습니다.
```

---

## 4. 시나리오 등록 API

### 4.1 기본 정보

| 항목 | 값 |
|------|-----|
| Endpoint | `/stockwin_item_update.asp` |
| Method | `POST` |
| Content-Type | `application/x-www-form-urlencoded` |
| Request Encoding | UTF-8 |

### 4.2 요청 파라미터

| No | Parameter | Type | 필수 | 설명 |
|----|-----------|------|:----:|------|
| 1 | `mode` | String | O | 인증 모드<br>• 고정값: `hangung2^alphago_hankyung` |
| 2 | `shop_id` | String | O | PG사 결제 ID (4.3 참조) |
| 3 | `ars_tel_no` | String | O | ARS 전화번호<br>• 숫자 4자리 |
| 4 | `scenario_type` | String | O | 시나리오 유형 (4.4 참조) |
| 5 | `arrribute_type` | String | O | 시나리오 속성 (4.5 참조) |
| 6 | `amount` | String | O | 결제/안내 금액 |
| 7 | `cc_pord_desc` | String | O | 상품 정보 (4.6 참조) |
| 8 | `startdtm` | String | 조건부 | 사전예약 시작일시<br>• `ADVRESERVD` 속성일 때 필수<br>• 형식: `YYYYMMDDHHMM` |

### 4.3 shop_id 유형

| shop_id | 결제 유형 | 설명 |
|---------|----------|------|
| `arsstockwin2` | 정기결제 | 상품별 빌키 발급 |
| `arsstockwin1` | 간편결제 | 빌키 기반 결제 |
| `arsstockwin` | 일반결제 | 매번 카드정보 입력 |

### 4.4 scenario_type 유형

| 코드 | 설명 |
|------|------|
| `CQS` | 정기결제용 / 자동 전화번호 인식 |
| `CIP` | 일반/간편결제 / 수동 전화번호 입력 |

### 4.5 arrribute_type 유형

#### 정기결제 속성

| 코드 | 설명 |
|------|------|
| `1MINTHSEAL` | 1개월 할인금액 후 정기결제 |
| `1MONTH` | 1개월 무료체험 후 정기결제 |
| `1WEEK` | 1주일 무료체험 후 정기결제 |
| `2WEEK` | 2주일 무료체험 후 정기결제 |
| `NOFREE` | 무료체험 없는 정기결제 |
| `ADVRESERVD` | 사전예약형 (`startdtm` 필수) |

#### 일반/간편결제 속성

| 코드 | 설명 |
|------|------|
| `EDUCATION` | 교육형 시나리오 |
| `SERVICE` | 일반 서비스형 |

### 4.6 cc_pord_desc 형식

```
상품명^상품코드
```

**예시**:
- 단일 상품: `프리미엄종목^PROD001`
- 정기결제: `골드멤버십^PROD001_REG`

### 4.7 요청 예시

```http
POST /ars/allat/db/stockwin_item_update.asp HTTP/1.1
Host: www.arspg.co.kr
Content-Type: application/x-www-form-urlencoded; charset=UTF-8

mode=hangung2%5Ealphago_hankyung&shop_id=arsstockwin2&ars_tel_no=2972&scenario_type=CQS&arrribute_type=1MONTH&amount=99000&cc_pord_desc=%ED%94%84%EB%A6%AC%EB%AF%B8%EC%97%84%5EPROD001
```

### 4.8 응답 형식

```
ars_tel_no|Item_cd|Error_cd|message
```

### 4.9 응답 필드

| No | Field | Type | 설명 |
|----|-------|------|------|
| 1 | `ars_tel_no` | String | 요청한 ARS 전화번호 |
| 2 | `Item_cd` | String | 상품코드 (등록된 경우) |
| 3 | `Error_cd` | String | 결과 코드 |
| 4 | `message` | String | 결과 메시지 |

### 4.10 응답 예시

**성공 (신규 등록)**
```
2972|PROD001|0000|등록이 완료되었습니다.
```

**성공 (기존 수정)**
```
2972|PROD001|0000|등록이 완료되었습니다.
```

**실패**
```
2972||0004|유효하지 않은 scenario_type입니다.
```

---

## 5. 에러 코드

### 5.1 공통 에러 코드

| 코드 | 설명 |
|------|------|
| `0000` | 성공 |
| `0001` | mode 값 오류 |
| `9001` | 데이터베이스 오류 |

### 5.2 결제유형 조회 에러 코드

| 코드 | 설명 |
|------|------|
| `0002` | ars_tel_no 누락 |
| `9002` | 전화번호 미등록 |

### 5.3 시나리오 등록 에러 코드

| 코드 | 설명 |
|------|------|
| `0002` | shop_id 누락 또는 유효하지 않음 |
| `0003` | ars_tel_no 누락 |
| `0004` | scenario_type 누락 또는 유효하지 않음 |
| `0005` | arrribute_type 누락 |
| `0006` | amount 누락 |
| `0007` | cc_pord_desc 누락 |
| `0008` | startdtm 누락 (ADVRESERVD 속성 시) |

---

## 6. 연동 가이드

### 6.1 연동 순서

```
1. 결제유형 조회 API 호출
   └─ ars_tel_no로 사용 가능한 결제유형 확인

2. 결제유형에 따른 파라미터 설정
   ├─ CQS → 정기결제용 파라미터
   └─ CIP → 일반/간편결제용 파라미터

3. 시나리오 등록 API 호출
   └─ 상품 정보 등록

4. 응답 결과 처리
   ├─ 0000 → 등록 성공
   └─ 그 외 → 에러 메시지 확인
```

### 6.2 권장 처리 로직

```javascript
// 결제유형 조회 결과 처리
function handleArsType(response) {
    const [ars_tel_no, type_cd_list, error_cd, message] = response.split('|');

    if (error_cd !== '0000') {
        throw new Error(message);
    }

    const types = type_cd_list.split(',');

    if (types.includes('CQS')) {
        return 'regular';  // 정기결제
    } else if (types.includes('CIP')) {
        return 'simple';   // 일반/간편결제
    } else if (types.includes('NDY')) {
        return 'unassigned';  // 미지정
    } else {
        console.warn('복수 유형 감지:', types);
        return types[0];  // 첫 번째 유형 사용
    }
}
```

### 6.3 주의사항

1. **인코딩**: 모든 요청/응답은 UTF-8 인코딩 사용
2. **URL 인코딩**: POST 데이터의 한글은 반드시 URL 인코딩 필요
3. **복수 결제유형**: 하나의 전화번호에 여러 유형이 있을 수 있음
4. **중복 등록**: 동일 ars_tel_no로 재등록 시 UPDATE 처리됨

---

## 7. 보안 고려사항

### 7.1 인증

- `mode` 파라미터를 통한 기본 인증
- 추후 API Key 방식 도입 검토 권장

### 7.2 전송 보안

- HTTPS 사용 권장
- 민감 정보 로깅 금지

### 7.3 입력 검증

- SQL Injection 방지 (작은따옴표 이스케이프 처리)
- 파라미터 화이트리스트 검증

---

## 8. 변경 이력

| 버전 | 일자 | 내용 | 작성자 |
|------|------|------|--------|
| 1.0 | 2026-01-13 | 최초 작성 (GET/POST API 통합) | 다삼솔루션 |

---

## 부록 A. 테스트 데이터

### A.1 테스트용 전화번호

| 전화번호 | 용도 |
|----------|------|
| `2972` | 정기결제 테스트 |
| `2973` | 일반결제 테스트 |

### A.2 테스트 요청 예시

**cURL - 결제유형 조회**
```bash
curl -X GET "https://www.arspg.co.kr/ars/allat/db/stockwin_get_arstype.asp?mode=hangung2%5Ealphago_hankyung&ars_tel_no=2972"
```

**cURL - 시나리오 등록**
```bash
curl -X POST "https://www.arspg.co.kr/ars/allat/db/stockwin_item_update.asp" \
  -H "Content-Type: application/x-www-form-urlencoded; charset=UTF-8" \
  -d "mode=hangung2%5Ealphago_hankyung" \
  -d "shop_id=arsstockwin2" \
  -d "ars_tel_no=2972" \
  -d "scenario_type=CQS" \
  -d "arrribute_type=1MONTH" \
  -d "amount=99000" \
  -d "cc_pord_desc=%ED%94%84%EB%A6%AC%EB%AF%B8%EC%97%84%5EPROD001"
```

# 다솜 결제 시나리오 등록 API 명세서

**문서번호**: WOWTV_DM_D0020102_결제시나리오등록프로토콜정의서
**버전**: 1.0
**최종수정일**: 2025-01-13
**작성자**: Claude Code

---

## 1. 개요

### 1.1 목적

ARS 전화번호에 결제 시나리오(상품 정보, 금액, 결제유형 등)를 등록하기 위한 API 명세입니다.

### 1.2 통신 규약

| 항목 | 값 |
|------|-----|
| 프로토콜 | HTTPS |
| 방식 | RESTful API (POST) |
| 인코딩 | UTF-8 |
| 응답 형식 | Plain Text (구분자: `\|`) |

### 1.3 서비스 흐름

```
[한국경제TV 상품관리]
        │
        ▼ (1) 결제 유형 조회 (GET_ARSTYPE API)
[페이엠솔루션 ARS 시스템]
        │
        ▼ (2) 결제유형 확인 후 시나리오 등록 요청
[한국경제TV 상품관리]
        │
        ▼ (3) 시나리오 등록 처리
[페이엠솔루션 ARS 시스템]
        │
        ▼ (4) 등록 결과 반환
[한국경제TV 상품관리]
```

---

## 2. API 엔드포인트

### 2.1 서버 환경

| 환경 | URL |
|------|-----|
| DEV | `https://www.arspg.co.kr/ars/allat/dev/db/stockwin_item_update.asp` |
| LIVE | `http://api.mspay.co.kr/pg/allat/hangung_item_update.php` |

### 2.2 요청 형식

```
POST {endpoint}
Content-Type: application/x-www-form-urlencoded
```

---

## 3. 요청 파라미터

### 3.1 파라미터 목록

| No | Parameter | Type | 필수 | 설명 |
|----|-----------|------|------|------|
| 1 | `mode` | Char | O | 결제 모드 식별자<br>• `hangung2^alphago_hankyung` (고정값) |
| 2 | `shop_id` | Char | O | 상점 ID<br>• `arsstockwin`, `arsstockwin1`, `arsstockwin2` 중 하나 |
| 3 | `ars_tel_no` | Char | O | ARS 전화번호 (DNIS)<br>• 숫자만 사용 |
| 4 | `scenario_type` | Char | O | 결제 시나리오 유형<br>• `CQS`: 정기결제용<br>• `CIP`: 일반/간편결제용 |
| 5 | `arrribute_type` | Char | O | 결제 속성<br>• `NORMAL`: 일반 즉시결제<br>• `ADVRESERVD`: 예약결제 |
| 6 | `amount` | Char | O | 결제 금액 (원 단위) |
| 7 | `cc_pord_desc` | Char | O | 상품 설명<br>• 형식: `서비스명^상품코드`<br>• 예: `주식창프리미엄^STOCK001` |
| 8 | `startdtm` | Char | △ | 예약결제 시작일시<br>• `ADVRESERVD` 속성일 때 필수<br>• 형식: `YYYYMMDDHHmmss` |

### 3.2 요청 예시

**일반 결제 등록:**
```
POST https://www.arspg.co.kr/ars/allat/dev/db/stockwin_item_update.asp
Content-Type: application/x-www-form-urlencoded

mode=hangung2^alphago_hankyung&shop_id=arsstockwin&ars_tel_no=6690&scenario_type=CQS&arrribute_type=NORMAL&amount=50000&cc_pord_desc=주식창프리미엄^STOCK001
```

**예약 결제 등록:**
```
POST https://www.arspg.co.kr/ars/allat/dev/db/stockwin_item_update.asp
Content-Type: application/x-www-form-urlencoded

mode=hangung2^alphago_hankyung&shop_id=arsstockwin&ars_tel_no=6690&scenario_type=CQS&arrribute_type=ADVRESERVD&amount=50000&cc_pord_desc=주식창프리미엄^STOCK001&startdtm=20250120100000
```

---

## 4. 응답

### 4.1 응답 형식

Plain Text, 파이프(`|`) 구분자 사용

```
ars_tel_no|item_cd|Error_cd|message
```

### 4.2 응답 필드

| No | Parameter | Type | 설명 |
|----|-----------|------|------|
| 1 | `ars_tel_no` | Char | 요청 시 사용한 ARS 전화번호 |
| 2 | `item_cd` | Char | 상품 코드<br>• 성공 시: `cc_pord_desc`에서 추출한 상품코드<br>• 실패 시: 빈 값 |
| 3 | `Error_cd` | Char | 결과코드<br>• `0000`: 성공 |
| 4 | `message` | Char | 결과 메시지 |

### 4.3 응답 예시

**성공:**
```
6690|STOCK001|0000|등록이 완료되었습니다.
```

**실패 (mode 오류):**
```
6690||0001|mode 값이 올바르지 않습니다.
```

**실패 (shop_id 오류):**
```
6690||0002|유효하지 않은 shop_id입니다.
```

---

## 5. 에러 코드

| 코드 | 설명 |
|------|------|
| `0000` | 성공 |
| `0001` | mode 값이 올바르지 않음 |
| `0002` | shop_id가 누락되었거나 유효하지 않음 |
| `0003` | ars_tel_no가 누락됨 |
| `0004` | scenario_type이 누락되었거나 유효하지 않음 |
| `0005` | arrribute_type이 누락됨 |
| `0006` | amount가 누락됨 |
| `0007` | cc_pord_desc가 누락됨 |
| `0008` | ADVRESERVD 속성에 startdtm이 누락됨 |
| `9001` | 데이터베이스 오류 (연결/조회/등록/수정 실패) |

---

## 6. 비즈니스 로직

### 6.1 등록/수정 판단

```
IF ARS_DNIS가 DB에 존재하지 않음 THEN
    → INSERT (신규 등록)
ELSE
    → UPDATE (기존 정보 수정)
END IF
```

### 6.2 DNIS_DESCRIPT JSON 생성

요청 파라미터를 기반으로 JSON 형식의 `DNIS_DESCRIPT` 컬럼 값 생성:

```json
{
    "amount": "50000",
    "attr": "NORMAL",
    "startdtm": "20250120100000",  // ADVRESERVD일 때만
    "product_code": "STOCK001"      // cc_pord_desc에 ^가 있을 때만
}
```

### 6.3 DB 저장 컬럼 매핑

| 요청 파라미터 | DB 컬럼 | 비고 |
|--------------|---------|------|
| `shop_id` | `ADMIN_ID` | |
| `ars_tel_no` | `ARS_DNIS` | |
| `scenario_type` | `ARS_TYPE` | CQS, CIP |
| (고정값) | `DLL_NAME` | `ALLAT_StockWin_Billkey_Easy_New_Scenario.dll` |
| (생성값) | `DNIS_DESCRIPT` | JSON 형식 |
| (고정값) | `PG_CODE` | `allat` |
| `cc_pord_desc`의 서비스명 | `SERVOCE_NAME` | |
| (고정값) | `USE_YN` | `Y` |

---

## 7. 관련 API

| API | 용도 | 문서 |
|-----|------|------|
| 결제유형 조회 API | 시나리오 등록 전 결제유형 확인 | [NEW_SPEC_DS_GET_SCENARIO.md](./NEW_SPEC_DS_GET_SCENARIO.md) |

---

## 8. 연동 시 주의사항

### 8.1 인코딩

- 클라이언트 요청은 **UTF-8** 인코딩 필수
- 한글 파라미터(cc_pord_desc 등)는 URL 인코딩하여 전송
- DB 저장 시 EUC-KR로 자동 변환 (SQL Server varchar)

### 8.2 권장 호출 순서

```
1. GET_ARSTYPE API로 결제유형 조회
2. 결제유형 확인 후 적절한 scenario_type 결정
3. REG_SCENARIO API로 시나리오 등록
4. 응답 코드 확인 및 오류 처리
```

### 8.3 예약결제 (ADVRESERVD)

- `arrribute_type=ADVRESERVD` 지정 시 `startdtm` 필수
- `startdtm` 형식: `YYYYMMDDHHmmss` (14자리)
- 예약 시간이 지나면 자동 결제 진행

---

## 9. 변경 이력

| 일자 | 내용 | 수정인 |
|------|------|--------|
| 2025-01-13 | 최초 작성 (stockwin_item_update.asp 기반) | Claude Code |

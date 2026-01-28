# API 문서에 service_name 파라미터 추가

## Context

### Original Request
`docs/` 폴더의 Swagger 및 HTML 파일에 `service_name` 파라미터 추가

### Files to Modify
1. `docs/openapi.yaml` - OpenAPI 스펙 파일
2. `docs/api-docs.html` - API 문서 HTML
3. `docs/swagger-ui.html` - Swagger UI HTML (내장 스펙)

---

## Work Objectives

### Core Objective
API 문서에 `service_name` 파라미터 정보 추가

### Concrete Deliverables
- 3개 파일 수정 완료

### Definition of Done
- [x] openapi.yaml에 service_name 파라미터 추가
- [x] api-docs.html에 service_name 파라미터 추가
- [x] swagger-ui.html에 service_name 파라미터 추가

---

## TODOs

- [x] 1. openapi.yaml 수정

  **What to do**:
  
  `/stockwin_item_update.asp` 경로의 properties에 `service_name` 추가 (Line 112-113 사이)
  
  ```yaml
  # cc_pord_desc 다음, startdtm 전에 추가
                service_name:
                  type: string
                  description: 서비스명 (DB SERVOCE_NAME 컬럼에 저장)
                  example: "스탁윈프리미엄"
  ```

  **References**:
  - `docs/openapi.yaml` Line 109-116

  **Acceptance Criteria**:
  - [ ] service_name 파라미터가 cc_pord_desc와 startdtm 사이에 추가됨

  **Commit**: NO (다른 파일과 함께 커밋)

---

- [x] 2. api-docs.html 수정

  **What to do**:
  
  시나리오 등록 API 파라미터 테이블에 `service_name` 행 추가 (Line 645-652 사이)
  
  ```html
                            <tr>
                                <td><span class="param-name">service_name</span></td>
                                <td>string</td>
                                <td>선택</td>
                                <td>서비스명 (DB SERVOCE_NAME 컬럼에 저장)</td>
                            </tr>
  ```

  **References**:
  - `docs/api-docs.html` Line 641-651 (cc_pord_desc와 startdtm 행 사이)

  **Acceptance Criteria**:
  - [ ] service_name 행이 파라미터 테이블에 추가됨

  **Commit**: NO (다른 파일과 함께 커밋)

---

- [x] 3. swagger-ui.html 수정

  **What to do**:
  
  내장된 spec JSON의 `/stockwin_item_update.asp` properties에 `service_name` 추가 (Line 665-670 사이)
  
  ```javascript
                                            "service_name": {
                                                "type": "string",
                                                "description": "서비스명 (DB SERVOCE_NAME 컬럼에 저장)",
                                                "example": "스탁윈프리미엄"
                                            },
  ```

  **References**:
  - `docs/swagger-ui.html` Line 661-670 (cc_pord_desc와 startdtm 사이)

  **Acceptance Criteria**:
  - [ ] service_name이 내장 스펙에 추가됨

  **Commit**: YES
  - Message: `docs: add service_name parameter to API documentation`
  - Files: `docs/openapi.yaml`, `docs/api-docs.html`, `docs/swagger-ui.html`

---

## Summary of Changes

| File | Location | Change |
|------|----------|--------|
| openapi.yaml | Line ~113 | `service_name` property 추가 |
| api-docs.html | Line ~648 | `<tr>` 행 추가 (service_name) |
| swagger-ui.html | Line ~667 | `service_name` JSON property 추가 |

---

## Parameter Definition

| Field | Value |
|-------|-------|
| Name | `service_name` |
| Type | string |
| Required | 선택 (optional) |
| Description | 서비스명 (DB SERVOCE_NAME 컬럼에 저장) |
| Example | "스탁윈프리미엄" |

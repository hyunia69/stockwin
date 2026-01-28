# service_name 파라미터 필수값 변경 및 설명 수정

## Context

### Original Request
- service_name을 필수값으로 변경
- description을 "SERVICE | EDUCATION | TABLET"로 변경

### Files to Modify
1. `docs/openapi.yaml` - OpenAPI 스펙 파일
2. `docs/api-docs.html` - API 문서 HTML
3. `docs/swagger-ui.html` - Swagger UI HTML (내장 스펙)

---

## Work Objectives

### Core Objective
service_name 파라미터를 필수값으로 변경하고 허용 값 명시

### Concrete Deliverables
- 3개 파일 수정 완료

### Definition of Done
- [x] openapi.yaml에서 service_name 필수 및 enum 추가
- [x] api-docs.html에서 service_name 필수 표시 및 설명 변경
- [x] swagger-ui.html에서 service_name 필수 및 설명 변경

---

## TODOs

- [x] 1. openapi.yaml 수정

  **What to do**:
  
  1) required 목록에 `service_name` 추가 (Line 69, cc_pord_desc 다음)
  2) service_name property의 description과 enum 수정 (Line 113-116)
  
  **변경 1 - required 목록 (Line 62-69):**
  ```yaml
              required:
                - mode
                - shop_id
                - ars_tel_no
                - scenario_type
                - arrribute_type
                - amount
                - cc_pord_desc
                - service_name
  ```
  
  **변경 2 - service_name property (Line 113-116):**
  ```yaml
                service_name:
                  type: string
                  description: SERVICE | EDUCATION | TABLET
                  enum:
                    - SERVICE
                    - EDUCATION
                    - TABLET
  ```

  **References**:
  - `docs/openapi.yaml` Line 62-69 (required 목록)
  - `docs/openapi.yaml` Line 113-116 (service_name property)

  **Acceptance Criteria**:
  - [ ] service_name이 required 목록에 추가됨
  - [ ] description이 "SERVICE | EDUCATION | TABLET"로 변경됨
  - [ ] enum 값이 추가됨
  - [ ] example 제거 (enum으로 대체)

  **Commit**: NO (다른 파일과 함께 커밋)

---

- [x] 2. api-docs.html 수정

  **What to do**:
  
  service_name 행의 "선택"을 "필수"로, 설명을 변경 (Line 646-651)
  
  **변경 전:**
  ```html
                            <tr>
                                <td><span class="param-name">service_name</span></td>
                                <td>string</td>
                                <td>선택</td>
                                <td>서비스명 (DB SERVOCE_NAME 컬럼에 저장)</td>
                            </tr>
  ```
  
  **변경 후:**
  ```html
                            <tr>
                                <td><span class="param-name">service_name</span></td>
                                <td>string</td>
                                <td><span class="required">필수</span></td>
                                <td>SERVICE | EDUCATION | TABLET</td>
                            </tr>
  ```

  **References**:
  - `docs/api-docs.html` Line 646-651

  **Acceptance Criteria**:
  - [ ] "선택"이 `<span class="required">필수</span>`로 변경됨
  - [ ] 설명이 "SERVICE | EDUCATION | TABLET"로 변경됨

  **Commit**: NO (다른 파일과 함께 커밋)

---

- [x] 3. swagger-ui.html 수정

  **What to do**:
  
  1) required 배열에 "service_name" 추가 (Line 626)
  2) service_name property 수정 (Line 666-670)
  
  **변경 1 - required 배열 찾아서 service_name 추가:**
  ```javascript
  "required": ["mode", "shop_id", "ars_tel_no", "scenario_type", "arrribute_type", "amount", "cc_pord_desc", "service_name"],
  ```
  
  **변경 2 - service_name property:**
  ```javascript
                                            "service_name": {
                                                "type": "string",
                                                "description": "SERVICE | EDUCATION | TABLET",
                                                "enum": ["SERVICE", "EDUCATION", "TABLET"]
                                            },
  ```

  **References**:
  - `docs/swagger-ui.html` Line 626 (required 배열)
  - `docs/swagger-ui.html` Line 666-670 (service_name property)

  **Acceptance Criteria**:
  - [ ] required 배열에 "service_name" 추가됨
  - [ ] description 변경됨
  - [ ] enum 추가됨
  - [ ] example 제거됨

  **Commit**: YES
  - Message: `docs: make service_name required with enum values`
  - Files: `docs/openapi.yaml`, `docs/api-docs.html`, `docs/swagger-ui.html`

---

## Summary of Changes

| File | Location | Change |
|------|----------|--------|
| openapi.yaml | Line 69 | required 목록에 service_name 추가 |
| openapi.yaml | Line 113-116 | description 변경, enum 추가 |
| api-docs.html | Line 649 | "선택" → "필수" |
| api-docs.html | Line 650 | description 변경 |
| swagger-ui.html | Line 626 | required에 service_name 추가 |
| swagger-ui.html | Line 666-670 | description, enum 변경 |

---

## Parameter Definition (After Change)

| Field | Value |
|-------|-------|
| Name | `service_name` |
| Type | string |
| Required | **필수** |
| Description | SERVICE \| EDUCATION \| TABLET |
| Enum | SERVICE, EDUCATION, TABLET |

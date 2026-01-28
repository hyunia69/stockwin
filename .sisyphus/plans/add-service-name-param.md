# service_name 파라미터 추가 및 cc_pord_desc 파싱 변경

## Context

### Original Request
1. `cc_pord_desc`의 `A^B` 파싱을 `product_name^product_cd`로 변경
2. 새 파라미터 `service_name` 추가
3. `SERVOCE_NAME` 컬럼에 `service_name` 파라미터 값 저장 (기존 `arrribute_type` 저장 제거)

### Current State
```
cc_pord_desc = "상품명^상품코드"
         ↓ 파싱
service_name = "상품명"      ← 변수명 부적절
product_code = "상품코드"    ← 변수명 부적절

SERVOCE_NAME 컬럼 ← arrribute_type 저장 (잘못된 설계)
```

### Target State
```
cc_pord_desc = "상품명^상품코드"
         ↓ 파싱
product_name = "상품명"      ← 변수명 수정
product_cd = "상품코드"      ← 변수명 수정

service_name 파라미터 ← 새로 추가
SERVOCE_NAME 컬럼 ← service_name 파라미터 저장
```

---

## Work Objectives

### Core Objective
- `cc_pord_desc` 파싱 변수명을 `product_name`, `product_cd`로 변경
- 새 파라미터 `service_name` 추가하여 `SERVOCE_NAME` 컬럼에 저장

### Concrete Deliverables
- `db/stockwin_item_update.asp` 파일 수정

### Definition of Done
- [x] `cc_pord_desc` 파싱 결과가 `product_name`, `product_cd` 변수에 저장됨
- [x] `service_name` 파라미터가 정상적으로 수신됨
- [x] INSERT/UPDATE 쿼리에서 `SERVOCE_NAME`에 `service_name` 파라미터 값 저장

---

## TODOs

- [x] 1. stockwin_item_update.asp 수정

  **What to do**:

  ### 1.1 파라미터 선언에 `service_name` 추가 (Line 117-118)
  ```asp
  ' 변경 전:
  Dim strMode, shop_id, ars_tel_no, scenario_type, arrribute_type
  Dim amount, cc_pord_desc, startdtm

  ' 변경 후:
  Dim strMode, shop_id, ars_tel_no, scenario_type, arrribute_type
  Dim amount, cc_pord_desc, startdtm, service_name
  ```

  ### 1.2 `service_name` 파라미터 파싱 추가 (Line 127 다음)
  ```asp
  startdtm = Trim(GetPostParam(rawPostData, "startdtm"))
  service_name = Trim(GetPostParam(rawPostData, "service_name"))
  ```

  ### 1.3 비즈니스 로직 변수명 변경 (Line 190)
  ```asp
  ' 변경 전:
  Dim service_name, product_code, dnis_descript, pos

  ' 변경 후:
  Dim product_name, product_cd, dnis_descript, pos
  ```

  ### 1.4 cc_pord_desc 파싱 로직 변수명 변경 (Line 192-199)
  ```asp
  ' 변경 전:
  pos = InStr(cc_pord_desc, "^")
  If pos > 0 Then
      service_name = Left(cc_pord_desc, pos - 1)
      product_code = Mid(cc_pord_desc, pos + 1)
  Else
      service_name = cc_pord_desc
      product_code = ""
  End If

  ' 변경 후:
  pos = InStr(cc_pord_desc, "^")
  If pos > 0 Then
      product_name = Left(cc_pord_desc, pos - 1)
      product_cd = Mid(cc_pord_desc, pos + 1)
  Else
      product_name = cc_pord_desc
      product_cd = ""
  End If
  ```

  ### 1.5 JSON 생성 로직 변경 (Line 206-211)
  ```asp
  ' 변경 전:
  If product_code <> "" Then
      dnis_descript = dnis_descript & ",""product_code"":""" & product_code & """"
  End If
  If service_name <> "" Then
      dnis_descript = dnis_descript & ",""service_name"":""" & service_name & """"
  End If

  ' 변경 후:
  If product_cd <> "" Then
      dnis_descript = dnis_descript & ",""product_cd"":""" & product_cd & """"
  End If
  If product_name <> "" Then
      dnis_descript = dnis_descript & ",""product_name"":""" & product_name & """"
  End If
  ```

  ### 1.6 INSERT 쿼리 - SERVOCE_NAME 값 변경 (Line 255)
  ```asp
  ' 변경 전:
  "'" & Replace(arrribute_type, "'", "''") & "', " & _

  ' 변경 후:
  "'" & Replace(service_name, "'", "''") & "', " & _
  ```

  ### 1.7 UPDATE 쿼리 - SERVOCE_NAME 값 변경 (Line 275)
  ```asp
  ' 변경 전:
  "SERVOCE_NAME = '" & Replace(arrribute_type, "'", "''") & "', " & _

  ' 변경 후:
  "SERVOCE_NAME = '" & Replace(service_name, "'", "''") & "', " & _
  ```

  ### 1.8 응답 생성 로직 변경 (Line 296-299)
  ```asp
  ' 변경 전:
  item_cd = product_code
  If item_cd = "" Then
      item_cd = service_name
  End If

  ' 변경 후:
  item_cd = product_cd
  If item_cd = "" Then
      item_cd = product_name
  End If
  ```

  **Must NOT do**:
  - 다른 파라미터 처리 로직 변경
  - DB 연결 정보 변경
  - 검증 로직 변경

  **References**:
  - `db/stockwin_item_update.asp` - 전체 파일

  **Acceptance Criteria**:
  - [x] `service_name` 파라미터 수신 확인
  - [x] `cc_pord_desc` 파싱이 `product_name`, `product_cd`로 동작
  - [x] `SERVOCE_NAME` 컬럼에 `service_name` 파라미터 값 저장

  **Commit**: YES
  - Message: `feat(api): add service_name param and rename cc_pord_desc parsing vars`
  - Files: `db/stockwin_item_update.asp`

---

## Summary of Changes

| Line | 변경 전 | 변경 후 |
|------|---------|---------|
| 118 | `Dim amount, cc_pord_desc, startdtm` | `Dim amount, cc_pord_desc, startdtm, service_name` |
| 128 | (없음) | `service_name = Trim(GetPostParam(...))` |
| 190 | `Dim service_name, product_code, ...` | `Dim product_name, product_cd, ...` |
| 194 | `service_name = Left(...)` | `product_name = Left(...)` |
| 195 | `product_code = Mid(...)` | `product_cd = Mid(...)` |
| 197 | `service_name = cc_pord_desc` | `product_name = cc_pord_desc` |
| 198 | `product_code = ""` | `product_cd = ""` |
| 206 | `product_code` | `product_cd` |
| 209 | `service_name` (JSON key도 변경) | `product_name` |
| 255 | `arrribute_type` | `service_name` |
| 275 | `arrribute_type` | `service_name` |
| 296 | `product_code` | `product_cd` |
| 298 | `service_name` | `product_name` |

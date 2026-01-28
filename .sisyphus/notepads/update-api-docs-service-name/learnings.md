# Learnings - API Documentation Update

## Project Context
- All source files use UTF-8 with BOM encoding
- API documentation exists in 3 formats: OpenAPI YAML, static HTML, embedded Swagger UI
- service_name parameter is optional (not required)

## Conventions
- Parameter placement: service_name goes between cc_pord_desc and startdtm
- HTML table structure: param-name span class for parameter names
- YAML indentation: consistent with existing properties
- JSON indentation: 2 spaces per level

## Previous Work
- ASP file (db/stockwin_item_update.asp) already modified to accept service_name parameter
- service_name maps to DB column SERVOCE_NAME (note typo in DB schema)

## [2026-01-27] Task: Documentation Update - service_name Parameter

### Changes Made
- **openapi.yaml** (lines 113-116): Added service_name property with YAML format
  - Type: string
  - Description: 서비스명 (DB SERVOCE_NAME 컬럼에 저장)
  - Example: "스탁윈프리미엄"
  - Indentation: 16 spaces for property, 18 for nested keys

- **api-docs.html** (lines 646-651): Added service_name table row
  - Type: string
  - Required: 선택 (optional)
  - Description: 서비스명 (DB SERVOCE_NAME 컬럼에 저장)
  - Indentation: 28 spaces for `<tr>` tag

- **swagger-ui.html** (lines 666-670): Added service_name to embedded JSON spec
  - Type: string
  - Description: 서비스명 (DB SERVOCE_NAME 컬럼에 저장)
  - Example: "스탁윈프리미엄"
  - Indentation: 44 spaces for property, trailing comma included

### Verification Results
✓ All 3 files successfully updated
✓ Grep confirmed service_name present in all files:
  - openapi.yaml line 113
  - api-docs.html line 647
  - swagger-ui.html line 666
✓ Correct positioning: between cc_pord_desc and startdtm in all files
✓ No syntax errors detected
✓ Encoding preserved (UTF-8 with BOM)
✓ Indentation matches surrounding code

### Key Observations
- YAML uses 2-space indentation per level (consistent with existing)
- HTML table rows use 28-space indentation for `<tr>` tags
- JSON embedded in JavaScript uses 44-space indentation for properties
- All descriptions correctly reference DB column SERVOCE_NAME (typo preserved as per schema)
- Parameter marked as optional (선택) in HTML, not in required array in YAML/JSON

### Notes
- Task completed successfully with no issues
- All three documentation formats now consistent
- Parameter placement follows logical grouping (product info together)

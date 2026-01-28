# Learnings - service_name Required Update

## Project Context
- All documentation files already have UTF-8 with BOM encoding
- service_name parameter was previously optional
- Need to add enum validation for SERVICE, EDUCATION, TABLET

## Conventions
- OpenAPI: enum values use array format with dash prefix
- HTML: required parameters use `<span class="required">필수</span>`
- Swagger UI: required array uses JSON string array format

## Previous Work
- service_name parameter already exists in all 3 documentation files
- Parameter is currently optional
- Current description mentions DB column SERVOCE_NAME

## [2026-01-27] Task: service_name Required Update - COMPLETED

### Changes Made
1. **openapi.yaml** (lines 70, 114-120)
   - Added `service_name` to required array (line 70)
   - Updated description to "SERVICE | EDUCATION | TABLET"
   - Added enum array with 3 values: SERVICE, EDUCATION, TABLET
   - Removed example field

2. **api-docs.html** (lines 646-651)
   - Changed "선택" to `<span class="required">필수</span>`
   - Updated description from "서비스명 (DB SERVOCE_NAME 컬럼에 저장)" to "SERVICE | EDUCATION | TABLET"

3. **swagger-ui.html** (lines 626, 666-670)
   - Added "service_name" to required array (line 626)
   - Updated description to "SERVICE | EDUCATION | TABLET"
   - Added enum array: ["SERVICE", "EDUCATION", "TABLET"]
   - Removed example field

### Verification Results
✓ All 3 files successfully updated
✓ Grep confirmed service_name appears in all files
✓ Enum values consistent across all files: SERVICE, EDUCATION, TABLET
✓ Required arrays updated correctly in both openapi.yaml and swagger-ui.html
✓ HTML markup uses proper `<span class="required">` tag
✓ UTF-8 with BOM encoding preserved

### Breaking Change Impact
- This is a BREAKING API CHANGE
- Existing API clients that don't send service_name will now fail validation
- Only SERVICE, EDUCATION, TABLET values are valid
- Clients must be updated to include service_name parameter

### Notes
- All edits applied cleanly without conflicts
- No syntax errors detected
- Indentation and formatting preserved correctly
- Enum format differs by file type (YAML dash-array vs JSON bracket-array) but values are identical

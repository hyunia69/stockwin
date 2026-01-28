# Issues - service_name Required Update

## Known Issues

### Breaking Change
- Making service_name required is a BREAKING CHANGE
- Existing API clients that don't send service_name will fail
- Status: Expected - business requirement
- Impact: HIGH - requires client updates

## Potential Gotchas

### OpenAPI Required Syntax
- required is an array at schema level, not in property
- Must add to existing required array, not create new one
- Location: Line 62-69

### Swagger UI Required Array
- Uses JSON string array format
- Must find correct required array (there may be multiple endpoints)
- Ensure trailing comma syntax is correct

### HTML Styling
- "필수" must use `<span class="required">` for red color styling
- Without span tag, text will be plain black

### Enum Format Differences
- OpenAPI YAML: dash-prefixed array
- Swagger UI JSON: bracket array with quotes
- Must maintain correct format for each file type

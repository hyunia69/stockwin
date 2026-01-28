# Issues - API Documentation Update

## Known Issues

### DB Schema Typo
- Column name: `SERVOCE_NAME` (typo: "SERVOCE" instead of "SERVICE")
- Status: Existing schema, cannot change without migration
- Workaround: Documentation explicitly mentions DB column name
- Impact: Low - internal only

## Potential Gotchas

### Encoding
- All files must maintain UTF-8 with BOM
- Watch for encoding corruption during edit
- Verify after changes

### YAML Indentation
- OpenAPI spec uses spaces (not tabs)
- Must match existing indentation level exactly
- Misalignment breaks YAML parsing

### HTML Table Structure
- Each <tr> must be properly closed
- Maintain existing CSS class names (param-name, required)
- Optional params do NOT use required class

### JSON Syntax
- Swagger UI has embedded JavaScript spec object
- Trailing commas are valid in modern JS but watch for consistency
- Example values should be strings with quotes

# Learnings from add-service-name-param

## Technical Insights

### ASP Classic Variable Scoping
- ASP Classic uses `Dim` for variable declaration
- Variables declared in one section can conflict with variables in another section
- Renaming variables requires careful tracking across the entire file

### Database Column Naming
- `SERVOCE_NAME` column was incorrectly storing `arrribute_type` (attribute type)
- Column name suggests it should store service name, not attribute type
- This was a design flaw that has now been corrected

### JSON Generation in ASP
- JSON is built manually using string concatenation
- Conditional fields are added only when values are non-empty
- Field names in JSON should match variable names for clarity

## Code Patterns

### Parameter Parsing Pattern
```asp
' Standard pattern for POST parameter extraction
param_name = Trim(GetPostParam(rawPostData, "param_name"))
```

### String Splitting Pattern
```asp
' Split "A^B" format
pos = InStr(string, "^")
If pos > 0 Then
    part1 = Left(string, pos - 1)
    part2 = Mid(string, pos + 1)
Else
    part1 = string
    part2 = ""
End If
```

### SQL Injection Prevention
```asp
' Always use Replace for SQL string escaping
"'" & Replace(value, "'", "''") & "'"
```

## Conventions Discovered

### Variable Naming
- `product_cd` for product code (not `product_code`)
- `product_name` for product name
- `service_name` for service name parameter
- Shorter names preferred in this codebase

### JSON Field Naming
- Use snake_case for JSON fields: `product_cd`, `product_name`
- Match internal variable names when possible

## Gotchas

### Variable Name Conflicts
- Original code had `service_name` variable for parsed value
- New parameter also named `service_name`
- Required renaming parsed variable to `product_name`

### Line Number Changes
- After adding new lines, all subsequent line numbers shift
- Plan referenced specific line numbers that became outdated
- Always verify actual content, not just line numbers

## Best Practices Applied

1. **Consistent Naming**: All variables now have clear, consistent names
2. **SQL Safety**: All user inputs properly escaped with `Replace()`
3. **JSON Clarity**: JSON field names match variable names
4. **Code Readability**: Variable names now reflect their actual purpose

## Future Recommendations

1. Consider adding validation for `service_name` parameter
2. Update API documentation to reflect new parameter
3. Add database migration notes for `SERVOCE_NAME` column usage change
4. Consider adding unit tests for parameter parsing logic

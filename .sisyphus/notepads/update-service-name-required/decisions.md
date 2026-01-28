# Decisions - service_name Required Update

## [2026-01-27T06:10:00Z] Make service_name Required

**Decision**: Change service_name from optional to required parameter  
**Rationale**: Business requirement - all scenarios must specify service type  
**Impact**: API clients must now provide service_name in all requests

## [2026-01-27T06:10:00Z] Add Enum Validation

**Decision**: Restrict service_name to SERVICE, EDUCATION, TABLET  
**Rationale**: These are the three valid service types in the system  
**Impact**: 
- API will reject invalid service_name values
- Documentation clearly shows allowed values
- Swagger UI provides dropdown for valid options

## [2026-01-27T06:10:00Z] Update Description

**Decision**: Change description from "서비스명 (DB SERVOCE_NAME 컬럼에 저장)" to "SERVICE | EDUCATION | TABLET"  
**Rationale**: Users need to know valid values, not internal DB details  
**Impact**: Clearer API documentation for consumers

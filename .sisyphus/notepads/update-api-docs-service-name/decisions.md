# Decisions - API Documentation Update

## [2026-01-27T05:00:00Z] Parameter Specification

**Decision**: service_name is optional parameter  
**Rationale**: Not all scenarios require service name; allows backward compatibility  
**Impact**: Documentation marks as "선택" (optional) not "필수" (required)

## [2026-01-27T05:00:00Z] Parameter Placement

**Decision**: Place service_name between cc_pord_desc and startdtm  
**Rationale**: Logical grouping - product info together, then temporal info (startdtm)  
**Impact**: All 3 documentation files follow same order

## [2026-01-27T05:00:00Z] Example Value

**Decision**: Use "스탁윈프리미엄" as example  
**Rationale**: Matches actual StockWin premium service naming convention  
**Impact**: Consistent example across all documentation

# 롤백 안정성 수정 (2026-01-29)

## 개요

결제 성공 후 롤백이 잘못 트리거되는 치명적 버그와 소멸자 데드락 위험을 수정했습니다.

## 수정된 버그

### 1. P0-1: m_bPaymentApproved 플래그 미설정 (Critical)

**문제**: 결제가 성공(reply_cd=0000)해도 `m_bPaymentApproved`가 TRUE로 설정되지 않아, 
DisConnectProcess에서 항상 롤백이 트리거됨.

**수정**: `ALLAT_Access.cpp` line 615-617
```cpp
// 결제 승인 성공 플래그 설정 (롤백 방지)
xprintf("[CH:%03d] AllatArsPayProcess > Payment approved, setting m_bPaymentApproved=TRUE", ch);
InterlockedExchange((LONG*)&pScenario->m_bPaymentApproved, TRUE);
```

### 2. P0-2: 스레드 안전성 (High)

**문제**: `m_bPaymentApproved` 플래그가 여러 스레드에서 동기화 없이 접근됨.

**수정**:
- `ALLAT_Stockwin_Quick_New_Scenario.h` line 122: `BOOL` → `LONG` 타입 변경
- `ALLAT_Stockwin_Quick_New_Scenario.cpp` line 2312: `InterlockedExchange` 사용
- `ALLAT_Stockwin_Quick_New_Scenario.cpp` line 2381: `InterlockedCompareExchange` 사용

### 3. P0-3: 소멸자 INFINITE 대기 (Critical)

**문제**: 소멸자에서 스레드 대기 시 INFINITE 타임아웃 사용으로 데드락 위험.

**수정**: `ALLAT_Stockwin_Quick_New_Scenario.cpp` lines 2274, 2285
- `INFINITE` → `10000` (10초) 타임아웃 변경
- 타임아웃 시 로그 출력 추가

## 변경된 파일

| 파일 | 변경 내용 |
|------|-----------|
| `ALLAT_Access.cpp` | line 615-617: 플래그 설정 추가 |
| `ALLAT_Stockwin_Quick_New_Scenario.h` | line 122: BOOL → LONG |
| `ALLAT_Stockwin_Quick_New_Scenario.cpp` | line 2312, 2381: 원자적 연산 |
| `ALLAT_Stockwin_Quick_New_Scenario.cpp` | line 2274, 2285: 타임아웃 변경 |

## 검증 방법

### 로그 확인

결제 성공 시:
```
[CH:XXX] AllatArsPayProcess > Payment approved, setting m_bPaymentApproved=TRUE
```

롤백 스킵 시 (결제 성공 후 끊김):
```
[CH:XXX] DisConnectProcess > No rollback needed (payment approved)
```

### 코드 검증
```bash
# 플래그 설정 확인
grep -n "m_bPaymentApproved.*TRUE" ALLAT_Stockwin_Quick_New_Scenario/ALLAT_Access.cpp

# 원자적 연산 확인
grep -n "InterlockedExchange\|InterlockedCompareExchange" ALLAT_Stockwin_Quick_New_Scenario/*.cpp

# INFINITE 대기 제거 확인 (소멸자 범위)
grep -n "WaitForSingleObject.*10000" ALLAT_Stockwin_Quick_New_Scenario/ALLAT_Stockwin_Quick_New_Scenario.cpp
```

## 알려진 제한사항

1. `ALLAT_Access.cpp` line 659, 683의 INFINITE 대기는 스레드 생성 전 대기로, 이번 범위 외
2. `m_bNeedRollback`, `m_bDisconnectProcessed` 플래그는 원자적 연산 미적용 (향후 작업)
3. 롤백 API 재시도 로직 미구현 (P2로 연기)

## 빌드 확인

Windows Server 2016에서 빌드 후 배포 필요:
```
msbuild ALLAT_Stockwin_Quick_New_Scenario.vcxproj /p:Configuration=Release
```

# 소멸자 롤백 기능 안정성 종합 검토

**작성일**: 2026-01-30  
**검토 대상**: `CALLAT_Hangung_Quick_Scenario` 클래스 소멸자 및 `DisConnectProcess` 함수  
**위험 등급**: 🔴 **높음 (High Risk)**

---

## 1. 개요

### 1.1 검토 목적

결제 시스템에서 소멸자를 통한 롤백 기능의 안정성을 종합적으로 검토합니다.
캐시/쿠폰 사용 후 결제가 완료되지 않은 상태에서 통화가 끊기면, 자동으로 롤백 API를 호출하여 데이터 무결성을 보장하는 것이 목표입니다.

### 1.2 현재 구현 구조

```
┌─────────────────────────────────────────────────────────────────┐
│  통화 종료 시 호출 순서                                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. IScenario::DisConnectProcess() - 가상 메서드 (호스트 호출)    │
│           ↓                                                      │
│  2. 롤백 조건 확인 (m_bNeedRollback && !m_bPaymentApproved)      │
│           ↓                                                      │
│  3. PL_ReserveRollback() 호출 (HTTP API)                         │
│           ↓                                                      │
│  4. DB UPDATE (CALL_CNT 감소)                                    │
│           ↓                                                      │
│  5. ~CALLAT_Hangung_Quick_Scenario() 소멸자                      │
│           ↓                                                      │
│  6. 스레드 대기 (10초 타임아웃)                                    │
│           ↓                                                      │
│  7. 핸들 정리                                                     │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. 발견된 문제점

### 2.1 🔴 위험도 높음 (Critical)

#### 2.1.1 HTTP 타임아웃 미설정

**현재 코드** (PayLetterAPI.cpp):
```cpp
pSession = new CInternetSession(_T("PayLetterAPI/1.0"));
// 타임아웃 설정 없음 → 기본값 30초
```

**문제점**:
- 결제 서버 장애 시 채널당 30초 이상 블로킹
- 30채널 시스템에서 모든 채널 교착 시 최대 20분 서비스 중단 가능
- ARS 시스템 전체 장애로 확대될 수 있음

**권장 수정**:
```cpp
pSession = new CInternetSession(_T("PayLetterAPI/1.0"));
pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 3000);   // 3초
pSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 3000);      // 3초
pSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000);   // 5초
```

---

#### 2.1.2 소멸자에서 예외 전파 위험

**현재 코드** (소멸자):
```cpp
~CALLAT_Hangung_Quick_Scenario()
{
    if (!m_bDisconnectProcessed)
    {
        this->DisConnectProcess();  // 예외 발생 가능!
    }
    // ...
}
```

**문제점**:
- C++11 이후 소멸자는 기본적으로 `noexcept`
- 소멸자에서 예외 전파 시 `std::terminate()` 호출 → 프로세스 크래시
- SEH (`__try/__except`)는 C++ 객체가 있는 함수에서 사용 불가 (C2712 오류)

**발생 가능한 예외**:
| 예외 유형 | 발생 상황 | 현재 처리 |
|-----------|----------|----------|
| `CInternetException` | 네트워크 오류 | ✅ 처리됨 |
| `CMemoryException` | 메모리 할당 실패 | ❌ 미처리 |
| `std::bad_alloc` | STL 메모리 부족 | ❌ 미처리 |
| Access Violation | 잘못된 포인터 | ❌ SEH 불가 |

---

### 2.2 🟡 위험도 중간 (Medium)

#### 2.2.1 m_bNeedRollback 동기화 미흡

**현재 코드**:
```cpp
// m_bPaymentApproved: Interlocked 사용 ✅
LONG paymentApproved = InterlockedCompareExchange(&m_bPaymentApproved, 0, 0);

// m_bNeedRollback: 일반 변수 접근 ❌
if (m_bNeedRollback && !paymentApproved)
{
    // ...
    m_bNeedRollback = FALSE;  // 데이터 레이스 가능!
}
```

**문제 시나리오**:
```
Thread A (결제 스레드)              Thread B (DisConnectProcess)
─────────────────────────          ───────────────────────────
m_bNeedRollback = TRUE          
                                   if (m_bNeedRollback)  // TRUE 읽음
m_bNeedRollback = FALSE         
                                       PL_ReserveRollback()  // 중복 롤백!
```

**권장 수정**:
```cpp
// 헤더 파일
volatile LONG m_bNeedRollback;  // BOOL → volatile LONG

// 사용 시
InterlockedExchange(&m_bNeedRollback, TRUE);   // 설정
InterlockedExchange(&m_bNeedRollback, FALSE);  // 리셋
LONG needRollback = InterlockedCompareExchange(&m_bNeedRollback, 0, 0);  // 읽기
```

---

#### 2.2.2 스레드 타임아웃 후 처리 미흡

**현재 코드**:
```cpp
DWORD waitResult1 = ::WaitForSingleObject(m_hThread, 10000);
if (waitResult1 == WAIT_TIMEOUT) {
    xprintf("[CH:%03d] ~Destructor > DB thread wait timeout (10s)", nChan);
}
CloseHandle(m_hThread);  // ⚠️ 스레드가 아직 실행 중인데 핸들 닫음!
m_hThread = NULL;
```

**문제점**:
- `CloseHandle()`은 스레드를 종료시키지 않음 (핸들만 해제)
- 스레드가 계속 실행 중이면 소멸된 객체의 멤버에 접근 → Use-After-Free

```
┌────────────────────────────────────────────────────────┐
│  타임라인                                              │
├────────────────────────────────────────────────────────┤
│  T+0s:   WaitForSingleObject 시작                      │
│  T+10s:  WAIT_TIMEOUT 반환                             │
│  T+10s:  CloseHandle(m_hThread) - 핸들만 해제          │
│  T+10s:  소멸자 완료 - 객체 메모리 해제                │
│  T+12s:  스레드가 pScenario->m_AdoDb 접근 시도         │
│          → CRASH: Access Violation                    │
└────────────────────────────────────────────────────────┘
```

---

## 3. 외부 리소스 참조 Best Practices

### 3.1 C++ 소멸자 안전 지침 (출처: cppreference, Microsoft Docs, C++ Core Guidelines)

| 규칙 | 우선순위 | 출처 |
|------|----------|------|
| 소멸자는 `noexcept`여야 함 | **필수** | C++ Standard |
| 예외가 소멸자에서 빠져나가면 안 됨 | **필수** | SEI CERT, Scott Meyers |
| 네트워크/HTTP 호출 금지 | **권장** | Best Practice |
| SEH (`__try`)는 C++ 객체와 함께 사용 불가 | **제약** | Microsoft MSVC |
| RAII로 리소스 관리 | **권장** | C++ Core Guidelines |
| 명시적 `close()`/`flush()` 메서드 제공 | **권장** | Scott Meyers |

### 3.2 RAII 트랜잭션 가드 패턴 (출처: ClickHouse, Facebook Folly, Audacity)

**핵심 원칙**: "Commit or Rollback on Destruction"

```cpp
// 권장 패턴 (Audacity Transaction.cpp 참조)
Transaction::~Transaction()
{
    Abort();  // 소멸자에서 항상 롤백 시도
}

Error Transaction::Commit() noexcept
{
    if (mCommitted) return {};  // 이미 커밋됨
    mCommitted = true;          // 먼저 플래그 설정
    return doCommit();          // 실제 커밋
}

Error Transaction::Abort() noexcept
{
    if (mCommitted) return {};  // 이미 커밋됨, 롤백 불필요
    mCommitted = true;          // 중복 롤백 방지
    return doRollback();        // 실제 롤백
}
```

### 3.3 C++17 std::uncaught_exceptions() 활용 (출처: ClickHouse)

```cpp
// 예외 스택 언와인딩 중인지 확인
~TransactionHolder() noexcept
{
    if (std::uncaught_exceptions() == 0 && autocommit)
    {
        // 정상 종료: 커밋 시도
        try { commit(); }
        catch (...) { rollback(); }
    }
    else
    {
        // 예외 발생 중: 항상 롤백
        rollback();
    }
}
```

---

## 4. 권장 개선 사항

### 4.1 즉시 수정 필요 (Quick Fix: <1시간)

#### Fix 1: HTTP 타임아웃 설정 추가

**파일**: `PayLetterAPI.cpp` - `PL_HttpPost()` 함수

```cpp
// 변경 전
pSession = new CInternetSession(_T("PayLetterAPI/1.0"));

// 변경 후
pSession = new CInternetSession(_T("PayLetterAPI/1.0"));
pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 3000);   // 3초
pSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 3000);      // 3초
pSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000);   // 5초
```

#### Fix 2: m_bNeedRollback 원자적 연산 적용

**파일**: `ALLAT_Stockwin_Quick_New_Scenario.h`
```cpp
// 변경 전
BOOL  m_bNeedRollback;

// 변경 후
volatile LONG m_bNeedRollback;
```

**파일**: 관련 .cpp 파일들
```cpp
// 설정 시
InterlockedExchange(&m_bNeedRollback, TRUE);

// 읽기 및 리셋
LONG needRollback = InterlockedCompareExchange(&m_bNeedRollback, 0, 0);
if (needRollback && !paymentApproved)
{
    // 롤백 처리
    InterlockedExchange(&m_bNeedRollback, FALSE);
}
```

---

### 4.2 단기 수정 권장 (Short: 1-4시간)

#### Fix 3: SEH 래퍼 함수 구현

**파일**: 새 함수 또는 기존 유틸리티 파일

```cpp
// SEH를 사용하는 안전한 롤백 래퍼 (소멸자 외부 함수)
int SafeRollback(const char* orderNo, const char* memberId)
{
    __try
    {
        return PL_ReserveRollback(orderNo, memberId);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        if (eprintf) eprintf("[CRITICAL] SafeRollback exception: %08X", 
                             GetExceptionCode());
        return -99;
    }
}
```

#### Fix 4: 스레드 타임아웃 로깅 강화

```cpp
if (m_hPayThread)
{
    DWORD waitResult = ::WaitForSingleObject(m_hPayThread, 10000);
    if (waitResult == WAIT_TIMEOUT) 
    {
        xprintf("[CH:%03d] CRITICAL: Payment thread timeout - possible resource leak", nChan);
        // 경고: TerminateThread 사용 금지 (데이터 손상 위험)
    }
    CloseHandle(m_hPayThread);
    m_hPayThread = NULL;
}
```

---

### 4.3 중기 아키텍처 개선 권장 (Medium: 1-2일)

#### Fix 5: 비동기 롤백 큐 구현 (근본적 해결)

```
┌─────────────────────────────────────────────────────────────────┐
│  현재 아키텍처 (동기 블로킹)                                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  통화 종료 → 소멸자 → DisConnectProcess → HTTP 롤백 (블로킹)    │
│                                     ↓                            │
│                              DB UPDATE (블로킹)                  │
│                                     ↓                            │
│                           (최대 40초 블로킹)                      │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  권장 아키텍처 (비동기 롤백 큐)                                   │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  통화 종료 → DisConnectProcess → 롤백 큐에 등록 (즉시 반환)     │
│                    ↓                                             │
│               소멸자 (빠른 리소스 해제만)                         │
│                                                                  │
│  [별도 워커 스레드]                                              │
│        ↓                                                         │
│  롤백 큐 처리 → HTTP 롤백 → DB UPDATE → 재시도 로직              │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

**비동기 롤백 큐 클래스 개요**:
```cpp
// 전역 싱글톤 롤백 큐
class CRollbackQueue {
private:
    CRITICAL_SECTION m_cs;
    std::queue<RollbackItem> m_queue;
    HANDLE m_hWorkerThread;
    HANDLE m_hStopEvent;
    
public:
    // 비동기 롤백 요청 (소멸자에서 호출 - 즉시 반환)
    void EnqueueRollback(const char* orderNo, const char* memberId) {
        EnterCriticalSection(&m_cs);
        m_queue.push({orderNo, memberId, GetTickCount64()});
        LeaveCriticalSection(&m_cs);
        SetEvent(m_hQueueEvent);  // 워커 깨우기
    }
};
```

---

## 5. 위험도 요약 매트릭스

| 문제 | 위험도 | 발생 확률 | 영향도 | 권장 조치 |
|------|--------|----------|--------|----------|
| HTTP 타임아웃 미설정 | 🔴 High | 서버 장애 시 100% | 시스템 교착 | **즉시 수정** |
| 소멸자 예외 전파 | 🔴 High | 네트워크 오류 시 | 프로세스 크래시 | **즉시 수정** |
| m_bNeedRollback 동기화 | 🟡 Medium | 드물지만 존재 | 중복 롤백/누락 | 권장 수정 |
| 스레드 타임아웃 후 처리 | 🟡 Medium | DB 장애 시 | Use-After-Free | 권장 수정 |
| 동기 블로킹 설계 | 🟡 Medium | 항상 | 성능 저하 | 장기 개선 |

---

## 6. 검증 체크리스트

### 6.1 코드 검증
```bash
# HTTP 타임아웃 설정 확인
grep -n "INTERNET_OPTION_.*TIMEOUT" ALLAT_Stockwin_Quick_New_Scenario/PayLetterAPI.cpp

# m_bNeedRollback 원자적 연산 확인
grep -n "InterlockedExchange.*m_bNeedRollback" ALLAT_Stockwin_Quick_New_Scenario/*.cpp

# SEH 래퍼 함수 확인
grep -n "SafeRollback\|__try.*__except" ALLAT_Stockwin_Quick_New_Scenario/*.cpp
```

### 6.2 로그 확인
```
# 타임아웃 발생 시
[CH:XXX] CRITICAL: Payment thread timeout - possible resource leak

# 롤백 성공 시
[PayLetterAPI] PL_ReserveRollback: 롤백 성공

# SEH 예외 발생 시
[CRITICAL] SafeRollback exception: XXXXXXXX
```

---

## 7. 참고 문헌

### 7.1 C++ 표준 및 가이드라인
- cppreference.com - Destructors
- C++ Core Guidelines C.36: "A destructor must not fail"
- SEI CERT DCL57-CPP: "Do not let exceptions escape from destructors"
- Scott Meyers, Effective C++ Item 8

### 7.2 Microsoft 문서
- MSVC C2712: Cannot use __try in functions that require object unwinding
- CInternetSession Class - Timeout Options

### 7.3 오픈소스 프로젝트 참조
- ClickHouse MergeTreeTransactionHolder - std::uncaught_exceptions() 활용
- Facebook Folly ScopeGuard - SCOPE_EXIT/FAIL/SUCCESS 패턴
- LLVM scope_exit - 경량 스코프 가드
- Audacity Transaction - SQLite 트랜잭션 가드

---

## 8. 결론

현재 구현은 **결제 데이터 무결성**을 보장하려는 의도는 적절하나, **프로덕션 환경에서 시스템 장애를 유발할 수 있는 구조적 결함**이 존재합니다.

### 즉시 조치 필요:
1. ✅ HTTP 타임아웃 설정 (3/3/5초)
2. ✅ m_bNeedRollback 원자적 연산 적용

### 단기 권장:
3. SEH 래퍼 함수 구현
4. 스레드 타임아웃 로깅 강화

### 중기 권장:
5. 비동기 롤백 큐 아키텍처 도입

---

**문서 작성**: Prometheus (AI Planning Agent)  
**검토 소스**: Oracle (Architecture Consultant), Librarian (External Resources)

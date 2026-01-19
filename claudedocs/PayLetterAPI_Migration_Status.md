# PayLetterAPI 마이그레이션 상태 문서

**작성일**: 2026-01-19
**상태**: ✅ 완료 (헤더 파일 + CPP 파일 모두 완료)

---

## 1. 문제 상황

### 컴파일 오류
Visual Studio 2012/2013 빌드 서버에서 `PayLetterAPI.h`/`PayLetterAPI.cpp` 컴파일 시 100개 이상의 오류 발생.

### 오류 원인
- `PayLetterAPI.h`에서 `std::string`, `std::vector<unsigned char>` 사용
- MFC 프로젝트에서 STL 헤더 순서 문제 (`<afx.h>` vs `<string>`)
- 구버전 Visual Studio와의 호환성 문제

### 오류 예시
```
error C2059: syntax error : '}'    PayLetterAPI.h  44
error C2065: 'PL_PaymentInfo' : undeclared identifier
error C2146: syntax error : missing ';' before identifier 'g_plConfig'
```

---

## 2. 해결 방안

### 기존 프로젝트 패턴 참조
`ALLAT_Stockwin_Quick_New_Scenario.h` 분석 결과:
- **C 스타일 문자 배열** 사용: `char m_szXXX[N + 1]`
- `std::string` 전혀 사용하지 않음
- STL 컨테이너 미사용

### 적용 방침
`PayLetterAPI.h`와 `PayLetterAPI.cpp`를 기존 코드 스타일에 맞게 완전 재작성:
1. `std::string` → `char[]` 배열
2. `std::vector<unsigned char>` → `unsigned char*` + 크기 파라미터
3. `bool` 반환 → `int` 반환 (1=성공, 0=실패)
4. `<string>`, `<vector>`, `<sstream>` include 제거

---

## 3. 완료된 작업

### ✅ PayLetterAPI.h 재작성 완료

**파일 위치**: `D:\dasam\stockwin\code\stockwin\ALLAT_Stockwin_Quick_New_Scenario\PayLetterAPI.h`

**주요 변경 내용**:

#### 상수 정의 추가
```c
#define PL_MAX_MEMBER_ID        40
#define PL_MAX_NICK_NAME        64
#define PL_MAX_ITEM_NAME        256
#define PL_MAX_ERROR_MSG        512
#define PL_MAX_AUTH_HEADER      512
#define PL_MAX_JSON_BUFFER      4096
#define PL_MAX_RESPONSE_BUFFER  8192
// ... 등
```

#### 구조체 변경 (Before → After)
```c
// Before (std::string 사용)
typedef struct _PL_PaymentInfo {
    std::string memberId;
    std::string nickName;
    // ...
} PL_PaymentInfo;

// After (C 스타일)
typedef struct _PL_PaymentInfo {
    char memberId[PL_MAX_MEMBER_ID + 1];
    char nickName[PL_MAX_NICK_NAME + 1];
    // ...
} PL_PaymentInfo;
```

#### 함수 시그니처 변경
```c
// Before
std::string PL_GenerateAuthHeader();
bool PL_GetPaymentInfo(int reqType, const std::string& reqTypeVal, ...);
std::string PL_GetLastError();

// After
int PL_GenerateAuthHeader(char* outHeader, int outHeaderSize);
int PL_GetPaymentInfo(int reqType, const char* reqTypeVal, ..., PL_PaymentInfo* outInfo);
void PL_GetLastError(char* outError, int outErrorSize);
```

---

### ✅ PayLetterAPI.cpp 재작성 완료

**파일 위치**: `D:\dasam\stockwin\code\stockwin\ALLAT_Stockwin_Quick_New_Scenario\PayLetterAPI.cpp`

**주요 변경 내용**:

#### STL 제거
- `#include <sstream>`, `#include <iomanip>`, `#include <algorithm>` 제거
- `std::string` → `char[]` 배열
- `std::vector<unsigned char>` → `unsigned char*` + 크기 파라미터
- `std::ostringstream` → `sprintf_s()`

#### 전역 변수 변경
```c
// Before
static std::string g_lastError;

// After
static char g_lastError[PL_MAX_ERROR_MSG + 1];
```

#### 함수 구현 변경
- `PL_InitPaymentInfo()` - `memset()` 사용
- `PL_InitConfig()` - `memset()` 사용
- `PL_Initialize()` - `strncpy_s()` 사용
- `PL_GenerateAuthHeader()` - 출력 버퍼 방식
- `PL_GenerateHMACSHA256()` - 버퍼 방식
- `PL_Base64Encode()` / `PL_Base64Decode()` - 버퍼 방식
- `PL_GetPaymentInfo()` - `strncpy_s()`로 결과 복사
- `PL_HttpPost()` - 버퍼 방식
- `PL_JsonGetString()` - 버퍼 방식
- `PL_GetLastError()` / `PL_SetLastError()` - 전역 `char[]` 사용

---

### ✅ WowTvSocket.cpp 수정 완료

**파일 위치**: `D:\dasam\stockwin\code\stockwin\ALLAT_Stockwin_Quick_New_Scenario\WowTvSocket.cpp`

**수정 부분 (PL_InfoOrderReq_Process 함수)**:

#### std::string 제거
```c
// Before
std::string dnis(pScenario->szDnis);
std::string phoneNo(pScenario->m_szInputTel);
PL_GetPaymentInfo(1, dnis, phoneNo, "VARS", plInfo);
std::string errMsg = PL_GetLastError();

// After
PL_GetPaymentInfo(1, pScenario->szDnis, pScenario->m_szInputTel, "VARS", &plInfo);
char errMsg[PL_MAX_ERROR_MSG + 1] = { 0 };
PL_GetLastError(errMsg, sizeof(errMsg));
```

#### 문자열 비교 변경
```c
// Before
if (plInfo.resultCode != "0") { ... }
if (plInfo.purchaseLimitFlag != "1") { ... }
if (plInfo.serviceCheckFlag == "Y") { ... }

// After
if (strcmp(plInfo.resultCode, "0") != 0) { ... }
if (strcmp(plInfo.purchaseLimitFlag, "1") != 0) { ... }
if (strcmp(plInfo.serviceCheckFlag, "Y") == 0) { ... }
```

#### .c_str() 제거
```c
// Before
strncpy_s(pScenario->m_szMx_id, ..., plInfo.mallIdGeneral.c_str(), ...);
strncpy_s(pScenario->m_szCC_name, ..., plInfo.memberId.c_str(), ...);

// After
strncpy_s(pScenario->m_szMx_id, ..., plInfo.mallIdGeneral, ...);
strncpy_s(pScenario->m_szCC_name, ..., plInfo.memberId, ...);
```

#### PL_GetPurchaseLimitMessage 호출 변경
```c
// Before
std::string limitMsg = PL_GetPurchaseLimitMessage(plInfo.purchaseLimitFlag);
xprintf("... %s ...", limitMsg.c_str());

// After
char limitMsg[256] = { 0 };
PL_GetPurchaseLimitMessage(plInfo.purchaseLimitFlag, limitMsg, sizeof(limitMsg));
xprintf("... %s ...", limitMsg);
```

---

## 4. 빌드 서버 적용

수정 완료된 파일 복사:
- **소스**: `D:\dasam\stockwin\code\stockwin\ALLAT_Stockwin_Quick_New_Scenario\`
- **대상**: `C:\dasam\windows_PRI_20220113\windows_PRI\ALLAT_Stockwin_Quick_New_Scenario\`

복사할 파일:
1. ✅ `PayLetterAPI.h`
2. ✅ `PayLetterAPI.cpp`
3. ✅ `WowTvSocket.cpp`

---

## 5. 참고 사항

### 기존 코드 스타일 예시 (ALLAT_Stockwin_Quick_New_Scenario.h)
```c
class CALLAT_Hangung_Quick_Scenario : public IScenario
{
public:
    char  m_szInputTel[127 + 1];      // 고객이 입력한 전화 번호
    char  m_szCC_name[64 + 1];        // 고객명
    char  m_szCC_Prod_Desc[255 + 1];  // 상품명
    char  m_szMx_issue_no[80 + 1];    // 주문번호
    int   m_nAmount;                   // 결제금액
    // ...
};
```

### 문자열 비교 방법
```c
// std::string 방식
if (plInfo.resultCode == "0") { ... }

// C 스타일 방식
if (strcmp(plInfo.resultCode, "0") == 0) { ... }
```

### 문자열 복사 방법
```c
// std::string 방식
plInfo.memberId = jsonValue;

// C 스타일 방식
strncpy_s(plInfo.memberId, sizeof(plInfo.memberId), jsonValue, _TRUNCATE);
```

---

## 6. 변경 이력

| 날짜 | 작업 내용 | 상태 |
|------|----------|------|
| 2026-01-19 | PayLetterAPI.h C 스타일 재작성 | ✅ 완료 |
| 2026-01-19 | PayLetterAPI.cpp C 스타일 재작성 | ✅ 완료 |
| 2026-01-19 | WowTvSocket.cpp std::string 제거 | ✅ 완료 |

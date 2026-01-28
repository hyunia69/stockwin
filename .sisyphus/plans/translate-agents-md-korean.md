# AGENTS.md 한글화

## TL;DR

> **요약**: AGENTS.md 파일을 한글로 변환
> 
> **산출물**:
> - `AGENTS.md` 한글 버전
> 
> **예상 작업량**: Quick
> **병렬 실행**: NO - 단일 작업

---

## TODOs

- [x] 1. AGENTS.md 파일을 한글로 변환

  **할 일**:
  - 기존 AGENTS.md 파일의 영문 내용을 한글로 변환
  - 코드 예시는 그대로 유지 (주석만 한글화)
  - UTF-8 with BOM 인코딩 유지

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Acceptance Criteria**:
  - [x] AGENTS.md 파일이 한글로 변환됨
  - [x] 코드 예시는 영문 유지
  - [x] UTF-8 with BOM 인코딩

  **Commit**: YES
  - Message: `docs: translate AGENTS.md to Korean`
  - Files: `AGENTS.md`

---

## 한글 AGENTS.md 내용

다음 내용을 `AGENTS.md`에 작성:

```markdown
# AGENTS.md - AI 코딩 에이전트 가이드라인

이 문서는 이 저장소에서 작업하는 AI 코딩 에이전트를 위한 필수 지침을 제공합니다.

## 프로젝트 개요

한국경제TV(WowTV)용 ALLAT 결제 연동 ARS 시나리오 DLL입니다.
- **대상**: ISDN PRI E1 ARS 시스템 신용카드 결제 (일반결제/간편결제/빌키 결제)
- **언어**: C++ (Visual Studio)
- **빌드 환경**: Windows Server 2016

## 빌드 명령어

### 빌드 환경
- **빌드 서버**: Windows Server 2016
- **빌드 디렉토리**: `c:\dasam\windows_pri_20220113\windows_pri\allat_stockwin_quick_new_scenario\`
- **로컬 디렉토리**: `C:\Work\dasam\code\claude\stockwin\ars\`
- **작업 흐름**: 로컬에서 편집 -> 빌드 서버로 복사 -> 빌드 서버에서 빌드

### 빌드 과정
Windows Server 2016에서 Visual Studio로 빌드합니다.
.vcxproj 파일을 열고 Debug/Release 구성으로 빌드하세요.
참고: 오류 메시지는 로컬 경로가 아닌 빌드 서버 경로를 참조합니다.

### 자동화된 테스트 없음
이 프로젝트에는 자동화된 단위 테스트가 없습니다. 수동 검증이 필요합니다.

## 파일 인코딩

**중요**: 모든 소스 파일은 반드시 **UTF-8 with BOM** 인코딩이어야 합니다.
- 기존 파일들은 EUC-KR/CP949에서 변환되었습니다
- 필요시 `convert_encoding.py` 스크립트로 변환하세요
- 커밋 전 인코딩을 확인하세요

## 코드 스타일 가이드라인

### Include 순서
1. `stdafx.h` - 미리 컴파일된 헤더 (항상 첫 번째)
2. `CommonDef.H` - 공통 정의
3. `ALLATCommom.h` - 프로젝트 전용 헤더
4. 모듈 헤더 (WowTvSocket.h, ADODB.h 등)
5. 시스템 헤더 (<windows.h>)
6. 외부 라이브러리 헤더 (<openssl/ssl.h>)

### 명명 규칙

| 타입 | 규칙 | 예시 |
|------|------|------|
| 클래스 | `C` 접두사 + PascalCase | `CADODB`, `CWowTvSocket` |
| 구조체 | PascalCase 또는 UPPER_SNAKE | `CARDINFO`, `Card_ResInfo` |
| 멤버 변수 | `m_` 접두사 | `m_szInputTel`, `m_nAmount` |
| 전역 변수 | `g_` 또는 `g` 접두사 | `g_strMid`, `gNiceDebug` |
| 상수/매크로 | UPPER_SNAKE_CASE | `MAXCHAN`, `VOC_MAIL_ID` |
| 함수 | PascalCase 또는 snake_case | `ScenarioInit`, `getOrderInfo_host` |
| 함수 포인터 | camelCase | `info_printf`, `set_guide` |

### 멤버 변수 헝가리안 표기법
- `sz` = 문자열 (null 종료): `m_szInputTel`
- `n` = 숫자 (int): `m_nAmount`
- `b` = 불리언: `m_bDnisInfo`
- `p` = 포인터: `m_pBuffer`

### 변수 선언 (VS 2012/2013 호환)
**중요**: 모든 변수는 함수 시작 부분에 선언하세요 (C89 스타일):
```cpp
int MyFunction(void)
{
    // 모든 선언은 상단에
    char szBuffer[256];
    int result;
    const char* section = "CONFIG";
    
    // 그 다음 코드
    memset(szBuffer, 0, sizeof(szBuffer));
    result = DoSomething();
    return result;
}
```

### 네트워크 프로토콜용 구조체 패킹
```cpp
#pragma pack(push, 1)    // 네트워크 구조체는 1바이트 정렬
typedef struct {
    char field1[16];
    int  field2;
} NetworkPacket;
#pragma pack(pop)        // 기본 정렬로 복원
```

### 함수 포인터 패턴
```cpp
// 선언
extern void(*eprintf)(const char *str, ...);
extern void(*info_printf)(int chan, const char *str, ...);

// Setter 함수 (DLL 내보내기)
SCENARIO_API void Set_peprintf(void(*peprintf)(const char *str, ...))
{
    eprintf = peprintf;
}
```

### 오류 처리 패턴
```cpp
__try
{
    // 위험한 작업
}
__except (GetExceptionCode() == EXCEPTION_BREAKPOINT ?
    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
{
    xprintf("[CH:%03d] Exception in Function", ch);
    return FALSE;
}
```

### 메모리 초기화
```cpp
// 버퍼는 항상 0으로 초기화
char szBuffer[256];
memset(szBuffer, 0x00, sizeof(szBuffer));
```

## 아키텍처 패턴

### DLL 진입점
```cpp
extern "C" {
    SCENARIO_API IScenario* CreateEngine();
    SCENARIO_API void DestroyEngine(IScenario* pComponent);
}
```

### 상태 머신 (ARS 흐름)
```cpp
int STDMETHODCALLTYPE jobArs(int state)
{
    switch (state)
    {
    case STATE_INIT:    break;
    case STATE_INPUT:   break;
    case STATE_PROCESS: break;
    }
    return nextState;
}
```

## 주요 파일

| 파일 | 용도 |
|------|------|
| `ALLAT_Stockwin_Quick_New_Scenario.cpp` | ARS 시나리오 메인, DLL 진입점 |
| `ALLAT_Access.cpp` | ALLAT PG API (승인/취소/빌키) |
| `WowTvSocket.cpp` | TCP 소켓 통신 |
| `ADODB.cpp` | MS SQL Server ADO 연결 |
| `PayLetterAPI.cpp` | PayLetter REST API 연동 |

## 설정

- **INI 파일**: `Allat_Stockwin_Quick_New_para.ini`
- **환경 변수**: `.env` (로컬 개발용, 절대 커밋하지 마세요)

## 외부 의존성

- OpenSSL (SSL 통신)
- MS ADO (`msado15.dll`)
- MSXML6 (XML 파싱)
- KISA_SHA256 (한국인터넷진흥원 SHA256 라이브러리)
- Dialogic IVR SDK (`srllib.h`, `dxxxlib.h`, `vmsdef.h`)

## 중요 사항

1. **범위**: `ALLAT_Stockwin_Quick_New_Scenario`만 현재 개발 대상입니다
2. **문서**: `.md` 파일은 `claudedocs/` 폴더에 저장하세요
3. **유틸리티**: Python 스크립트는 `utils/` 폴더에 저장하세요
4. **DB 스키마**: `claudedocs/DB_SCHEMA_ALLAT.md` 참조하세요
5. **절대 커밋 금지**: `.env` 파일 (자격 증명 포함)

## 자주 사용하는 패턴

### 로깅
```cpp
xprintf("[CH:%03d] Module > Function > Message", channel);
eprintf("Error: %s", errorMessage);
```

### 문자열 안전성
```cpp
szBuffer[bytesRead] = 0x00;  // 항상 null 종료
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';
```

### 채널 기반 작업
```cpp
CALLAT_Hangung_Quick_Scenario *pScenario = 
    (CALLAT_Hangung_Quick_Scenario *)((*port)[ch].pScenario);
```
```

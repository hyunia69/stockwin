# Create AGENTS.md File

## TL;DR

> **Quick Summary**: Create AGENTS.md file with build commands and code style guidelines for AI coding agents
> 
> **Deliverables**:
> - `AGENTS.md` at project root (~150 lines)
> 
> **Estimated Effort**: Quick
> **Parallel Execution**: NO - single task
> **Critical Path**: Task 1 only

---

## Context

### Original Request
Create an AGENTS.md file containing build/lint/test commands and code style guidelines for agentic coding agents.

### Research Findings
- No existing AGENTS.md, Cursor rules, or Copilot rules
- Build environment: Visual Studio on Windows Server 2016
- Encoding: UTF-8 with BOM (mandatory)
- Code patterns: Hungarian notation, C89 variable declarations, function pointer callbacks
- No automated tests exist

---

## Work Objectives

### Core Objective
Create comprehensive AGENTS.md documentation for AI coding agents.

### Concrete Deliverables
- `AGENTS.md` file at project root

### Definition of Done
- [x] File exists at `C:\Work\dasam\code\claude\stockwin\ars\AGENTS.md`
- [x] Contains ~150 lines of documentation
- [x] Includes build environment info
- [x] Includes code style guidelines
- [x] File is UTF-8 with BOM encoded

### Must Have
- Build/environment documentation
- Code style guidelines (naming, formatting)
- File organization notes
- Key file descriptions

### Must NOT Have (Guardrails)
- Incorrect encoding (must be UTF-8 with BOM)
- References to non-existent test commands

---

## Verification Strategy

### Test Decision
- **Infrastructure exists**: NO
- **User wants tests**: Manual-only
- **Framework**: none

### Manual QA
Verify file creation and content via:
```bash
# Check file exists and has correct line count
wc -l AGENTS.md

# Verify encoding is UTF-8 with BOM
file AGENTS.md
```

---

## TODOs

- [x] 1. Create AGENTS.md file

  **What to do**:
  - Create `AGENTS.md` at project root with content from draft
  - Ensure UTF-8 with BOM encoding
  - Content should be ~150 lines covering:
    - Project overview
    - Build commands and environment
    - File encoding requirements
    - Code style guidelines (includes, naming, structs)
    - Architecture patterns
    - Key files and dependencies
    - Common patterns

  **Must NOT do**:
  - Don't claim automated tests exist
  - Don't reference non-existent configuration

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: Single file creation, no complex logic
  - **Skills**: []
    - No special skills needed

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: N/A (single task)
  - **Blocks**: None
  - **Blocked By**: None

  **References**:
  - Draft: `.sisyphus/drafts/agents-md-creation.md` - Contains all research findings
  - `CLAUDE.md` - Existing project guidelines to incorporate
  - `README.md` - Project overview

  **Acceptance Criteria**:
  - [x] `AGENTS.md` file exists at project root
  - [x] File is ~150 lines
  - [x] File is UTF-8 with BOM encoded
  - [x] Contains all required sections

  **Commit**: YES
  - Message: `docs: add AGENTS.md for AI coding agents`
  - Files: `AGENTS.md`

---

## AGENTS.md Content

Write this exact content to `AGENTS.md`:

```markdown
# AGENTS.md - Coding Agent Guidelines

This document provides essential guidance for AI coding agents working in this repository.

## Project Overview

ALLAT Payment Integration ARS Scenario DLL for WowTV (한국경제TV).
- **Target**: ISDN PRI E1 ARS system for credit card payments (general/simple/billkey)
- **Language**: C++ (Visual Studio)
- **Build Target**: Windows Server 2016

## Build Commands

### Build Environment
- **Build Server**: Windows Server 2016
- **Build Directory**: `c:\dasam\windows_pri_20220113\windows_pri\allat_stockwin_quick_new_scenario\`
- **Local Directory**: `C:\Work\dasam\code\claude\stockwin\ars\`
- **Workflow**: Edit locally -> Copy to build server -> Build on build server

### Build Process
Build is done via Visual Studio on Windows Server 2016.
Open .vcxproj and build Debug/Release configuration.
Note: Error messages reference build server paths, not local paths.

### No Automated Tests
This project does not have automated unit tests. Manual verification is required.

## File Encoding

**CRITICAL**: All source files must be **UTF-8 with BOM**.
- Existing files were converted from EUC-KR/CP949
- Use `convert_encoding.py` if needed for conversion
- Verify encoding before committing

## Code Style Guidelines

### Include Order
1. `stdafx.h` - Precompiled header (always first)
2. `CommonDef.H` - Common definitions
3. `ALLATCommom.h` - Project-specific headers
4. Module headers (WowTvSocket.h, ADODB.h, etc.)
5. System headers (<windows.h>)
6. External library headers (<openssl/ssl.h>)

### Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Class | `C` prefix + PascalCase | `CADODB`, `CWowTvSocket` |
| Struct | PascalCase or UPPER_SNAKE | `CARDINFO`, `Card_ResInfo` |
| Member variables | `m_` prefix | `m_szInputTel`, `m_nAmount` |
| Global variables | `g_` or `g` prefix | `g_strMid`, `gNiceDebug` |
| Constants/Macros | UPPER_SNAKE_CASE | `MAXCHAN`, `VOC_MAIL_ID` |
| Functions | PascalCase or snake_case | `ScenarioInit`, `getOrderInfo_host` |
| Function pointers | camelCase | `info_printf`, `set_guide` |

### Hungarian Notation for Members
- `sz` = string (zero-terminated): `m_szInputTel`
- `n` = number (int): `m_nAmount`
- `b` = boolean: `m_bDnisInfo`
- `p` = pointer: `m_pBuffer`

### Variable Declaration (VS 2012/2013 Compatibility)
**IMPORTANT**: Declare all variables at function start (C89 style):
```cpp
int MyFunction(void)
{
    // All declarations at the top
    char szBuffer[256];
    int result;
    const char* section = "CONFIG";
    
    // Then code
    memset(szBuffer, 0, sizeof(szBuffer));
    result = DoSomething();
    return result;
}
```

### Struct Packing for Network Protocols
```cpp
#pragma pack(push, 1)    // 1-byte alignment for network structures
typedef struct {
    char field1[16];
    int  field2;
} NetworkPacket;
#pragma pack(pop)        // Restore default alignment
```

### Function Pointer Pattern
```cpp
// Declaration
extern void(*eprintf)(const char *str, ...);
extern void(*info_printf)(int chan, const char *str, ...);

// Setter functions (DLL export)
SCENARIO_API void Set_peprintf(void(*peprintf)(const char *str, ...))
{
    eprintf = peprintf;
}
```

### Error Handling Pattern
```cpp
__try
{
    // Risky operation
}
__except (GetExceptionCode() == EXCEPTION_BREAKPOINT ?
    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
{
    xprintf("[CH:%03d] Exception in Function", ch);
    return FALSE;
}
```

### Memory Initialization
```cpp
// Always zero-initialize buffers
char szBuffer[256];
memset(szBuffer, 0x00, sizeof(szBuffer));
```

## Architecture Patterns

### DLL Entry Points
```cpp
extern "C" {
    SCENARIO_API IScenario* CreateEngine();
    SCENARIO_API void DestroyEngine(IScenario* pComponent);
}
```

### State Machine (ARS Flow)
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

## Key Files

| File | Purpose |
|------|---------|
| `ALLAT_Stockwin_Quick_New_Scenario.cpp` | Main ARS scenario, DLL entry |
| `ALLAT_Access.cpp` | ALLAT PG API (approval/cancel/billkey) |
| `WowTvSocket.cpp` | TCP socket communication |
| `ADODB.cpp` | MS SQL Server ADO connection |
| `PayLetterAPI.cpp` | PayLetter REST API integration |

## Configuration

- **INI File**: `Allat_Stockwin_Quick_New_para.ini`
- **Environment**: `.env` (local dev only, never commit)

## External Dependencies

- OpenSSL (SSL communication)
- MS ADO (`msado15.dll`)
- MSXML6 (XML parsing)
- KISA_SHA256 (Korean SHA256 library)
- Dialogic IVR SDK (`srllib.h`, `dxxxlib.h`, `vmsdef.h`)

## Important Notes

1. **Scope**: Only `ALLAT_Stockwin_Quick_New_Scenario` is in active development
2. **Documentation**: Store `.md` files in `claudedocs/` folder
3. **Utilities**: Store Python scripts in `utils/` folder
4. **DB Schema**: Reference `claudedocs/DB_SCHEMA_ALLAT.md`
5. **Never commit**: `.env` files (contains credentials)

## Common Patterns

### Logging
```cpp
xprintf("[CH:%03d] Module > Function > Message", channel);
eprintf("Error: %s", errorMessage);
```

### String Safety
```cpp
szBuffer[bytesRead] = 0x00;  // Always null-terminate
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';
```

### Channel-based Operations
```cpp
CALLAT_Hangung_Quick_Scenario *pScenario = 
    (CALLAT_Hangung_Quick_Scenario *)((*port)[ch].pScenario);
```
```

---

## Success Criteria

### Final Checklist
- [x] AGENTS.md exists at project root
- [x] File is UTF-8 with BOM encoded
- [x] Contains ~150 lines
- [x] All sections present (Build, Style, Architecture, Files, Dependencies)

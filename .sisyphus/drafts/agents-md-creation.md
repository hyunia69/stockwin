# Draft: AGENTS.md Creation

## Requirements (confirmed)
- Create AGENTS.md file at project root
- Include build/lint/test commands
- Include code style guidelines
- Target ~150 lines
- No Cursor/Copilot rules to incorporate (none exist)

## Research Findings

### Build Environment
- Visual Studio C++ on Windows Server 2016
- Build server: `c:\dasam\windows_pri_20220113\windows_pri\allat_stockwin_quick_new_scenario\`
- Local dev: `C:\Work\dasam\code\claude\stockwin\ars\`
- No automated tests exist

### Encoding
- UTF-8 with BOM (mandatory)
- Converted from EUC-KR/CP949

### Code Style Patterns Identified
1. Include order: stdafx.h first, then CommonDef.H, project headers, system headers
2. Class naming: `C` prefix + PascalCase (CADODB, CWowTvSocket)
3. Member variables: `m_` prefix with Hungarian (m_szBuffer, m_nCount, m_bFlag)
4. Global variables: `g_` or `g` prefix
5. Constants: UPPER_SNAKE_CASE
6. C89 variable declarations (VS 2012/2013 compatibility)
7. `#pragma pack(push, 1)` for network structures
8. Function pointer pattern for DLL callbacks
9. __try/__except for exception handling
10. Channel-based logging: `xprintf("[CH:%03d] Module > Function > Msg", ch)`

### Key Files
- ALLAT_Stockwin_Quick_New_Scenario.cpp - Main scenario logic
- ALLAT_Access.cpp - ALLAT PG API
- WowTvSocket.cpp - TCP socket
- ADODB.cpp - MS SQL ADO
- PayLetterAPI.cpp - REST API

### External Dependencies
- OpenSSL
- MS ADO (msado15.dll)
- MSXML6
- KISA_SHA256
- Dialogic IVR SDK

## Content for AGENTS.md

Full content prepared - see plan for exact text to write.

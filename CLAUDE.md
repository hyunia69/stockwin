# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 프로젝트 개요

한국경제TV(WowTV)용 ALLAT 결제 연동 ARS 시나리오 DLL입니다. ISDN PRI E1 회선 기반 ARS 시스템에서 신용카드 결제(일반결제/간편결제/빌키 결제)를 처리합니다.

## 아키텍처

### 핵심 컴포넌트

```
IScenario (인터페이스)
    └── CALLAT_WOWTV_Billkey_Easy_Scenario (DLL 내보내기 클래스)
            ├── CADODB (ADO 데이터베이스 연결)
            ├── CWowTvSocket (TCP 소켓 통신)
            └── AllatUtil (ALLAT PG사 SSL 통신)
```

- **ALLAT_StockWin_Billkey_Easy_New_Scenario.cpp**: ARS 시나리오 메인 로직, DLL 진입점(`CreateEngine`/`DestroyEngine`), 상태 기반 음성 안내 흐름
- **ALLAT_Access.cpp**: ALLAT PG사 결제 API 호출 로직 (승인/취소/빌키 발급)
- **WowTvSocket.cpp**: 외부 서버와 TCP 소켓 통신 (주문 정보 조회)
- **ADODB.cpp**: MS SQL Server ADO 연결 및 저장 프로시저 호출
- **AllatUtil.cpp**: ALLAT PG 서버 SSL 통신 유틸리티

### 결제 인증 타입 (ALLATCommom.h)

| 코드 | 설명 |
|------|------|
| 01 | 비인증 (카드번호 + 유효기간) |
| 02 | 구인증 (카드번호 + 유효기간 + 생년월일 + 비번) |
| 03 | 부분인증 (카드번호 + 유효기간 + 생년월일) |
| 41-43 | 빌키 간편결제 (각각 비인증/구인증/부분인증) |

### 데이터 구조체

- `CARDINFO`: 카드 입력 정보 (카드번호, 유효기간, 비밀번호, 할부)
- `Card_ResInfo`: 결제 승인 응답 (거래번호, 승인번호, 결과코드)
- `Card_CancleInfo`: 결제 취소 요청 정보
- `INFOPRODOCREQ`/`INFOPRODOCRES`: TCP 소켓 전문 구조체

### 외부 의존성

- **OpenSSL**: SSL 통신
- **MS ADO**: 데이터베이스 연결 (`msado15.dll`)
- **MSXML6**: XML 파싱
- **KISA_SHA256**: 한국인터넷진흥원 SHA256 라이브러리
- **Dialogic IVR SDK**: `srllib.h`, `dxxxlib.h`, `vmsdef.h` 등

### 설정 파일

- `ALLAT_WOWTV_Billkey_Easy_Scenario_para.ini`: 런타임 설정

## 빌드

Visual Studio C++ 프로젝트입니다. 빌드 전 필요 사항:
- OpenSSL 개발 라이브러리
- Dialogic IVR SDK 헤더 및 라이브러리
- MS ADO 런타임 (`C:\Program Files\Common Files\System\ADO\msado15.dll`)

## 인코딩

모든 소스 파일은 UTF-8 with BOM 인코딩입니다. `convert_encoding.py` 스크립트로 EUC-KR/CP949에서 변환되었습니다.

## 코드 스타일

- 함수 포인터를 통한 호스트 애플리케이션 콜백 연결 (`info_printf`, `set_guide` 등)
- `#pragma pack(push, 1)` 사용하여 네트워크 전문 구조체 1바이트 정렬
- 상태 기반 ARS 흐름 제어 (switch-case 패턴)

## 기타 사항

- md 파일은 claudedocs 폴더 아래 생성한다. 
- 파이선 코드나 기타 유틸리티는 utils 폴더 아래 생성한다.
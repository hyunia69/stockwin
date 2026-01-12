# ALLAT StockWin Billkey Easy New Scenario

한국경제TV(WowTV) 주식창 서비스용 ALLAT 결제 연동 ARS 시나리오 DLL

## 개요

ISDN PRI E1 회선 기반 ARS 시스템에서 신용카드 결제를 처리하는 DLL입니다.

**지원 결제 방식:**
- 일반 카드결제
- 간편결제
- 빌키(Billkey) 결제

## 아키텍처

```
IScenario (인터페이스)
    └── CALLAT_WOWTV_Billkey_Easy_Scenario (DLL 내보내기 클래스)
            ├── CADODB (ADO 데이터베이스 연결)
            ├── CWowTvSocket (TCP 소켓 통신)
            └── AllatUtil (ALLAT PG사 SSL 통신)
```

### 주요 파일

| 파일 | 설명 |
|------|------|
| `ALLAT_StockWin_Billkey_Easy_New_Scenario.cpp` | ARS 시나리오 메인 로직, DLL 진입점 |
| `ALLAT_Access.cpp` | ALLAT PG사 결제 API 호출 (승인/취소/빌키 발급) |
| `WowTvSocket.cpp` | 외부 서버 TCP 소켓 통신 (주문 정보 조회) |
| `ADODB.cpp` | MS SQL Server ADO 연결 및 저장 프로시저 |
| `AllatUtil.cpp` | ALLAT PG 서버 SSL 통신 유틸리티 |

### 결제 인증 타입

| 코드 | 설명 |
|------|------|
| 01 | 비인증 (카드번호 + 유효기간) |
| 02 | 구인증 (카드번호 + 유효기간 + 생년월일 + 비번) |
| 03 | 부분인증 (카드번호 + 유효기간 + 생년월일) |
| 41-43 | 빌키 간편결제 (각각 비인증/구인증/부분인증) |

## 빌드 요구사항

- **Visual Studio** (C++ 프로젝트)
- **OpenSSL** 개발 라이브러리
- **Dialogic IVR SDK** 헤더 및 라이브러리
- **MS ADO 런타임** (`C:\Program Files\Common Files\System\ADO\msado15.dll`)

## 외부 의존성

- **OpenSSL**: SSL 통신
- **MS ADO**: 데이터베이스 연결
- **MSXML6**: XML 파싱
- **KISA_SHA256**: 한국인터넷진흥원 SHA256 라이브러리
- **Dialogic IVR SDK**: ARS 음성 처리

## 프로젝트 구조

```
├── ALLAT_StockWin_Billkey_Easy_New_Scenario/   # 소스 코드
│   ├── *.cpp, *.h
│   └── SHA256/                                  # KISA SHA256 라이브러리
├── claudedocs/                                  # 분석 문서
├── utils/                                       # 유틸리티 스크립트
└── docs/                                        # 문서
```

## 설정

런타임 설정 파일: `ALLAT_WOWTV_Billkey_Easy_Scenario_para.ini`

## 인코딩

모든 소스 파일은 UTF-8 with BOM 인코딩입니다.

## 라이선스

Private - 내부 사용 전용

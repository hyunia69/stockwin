# SQL Server 드라이버 업그레이드 가이드

## 문제

ARS 서버 → 121번 DB 서버 접속 시 **SSL 보안 오류** 발생

```
[DBNETLIB][ConnectionOpen (SECDoClientHandshake()].SSL 보안 오류입니다.
```

## 원인

| 항목 | 상태 |
|------|------|
| SQLOLEDB.1 | TLS 1.0만 지원 |
| ARS 서버 | TLS 1.0 Client 비활성화 |
| **결과** | SSL 핸드셰이크 실패 |

## 해결

### 1. MSOLEDBSQL 18.0.2 설치 (ARS 서버)

**다운로드:**
```
https://www.sqlserverversions.com/2021/06/sql-server-oledb-driver-versions.html
```

- **버전**: 18.0.2 (TLS 1.2 지원, KB2999226 불필요)
- **설치**: 64비트 MSI 실행

### 2. 코드 수정

**파일**: `ADODB.cpp` - DBConnect 함수

```cpp
// 변경 전
"Provider=SQLOLEDB.1;..."

// 변경 후
"Provider=MSOLEDBSQL;..."
```

### 3. 빌드 & 배포

1. ADODB.cpp → 빌드 서버 복사
2. 빌드
3. DLL → ARS 서버 배포

## 하위 호환성

SQLOLEDB.1과 MSOLEDBSQL은 **독립적으로 공존** - 기존 서비스 영향 없음

---

**작성일**: 2026-01-21

# Billkey WowTvSocket REST API 구현 설계서

> **버전**: 1.0  
> **작성일**: 2026-02-01  
> **기반 문서**: BILL_KEY_REQ.md, billkey-wowtvsocket-restapi.md  
> **대상 프로젝트**: `ALLAT_Stockwin_Billkey_New_Scenario`

---

## 1. 현재 상태 요약

### 1.1 PayLetterAPI 모듈 상태

| 항목 | 상태 | 위치 |
|------|------|------|
| PL_BatchPaymentInfo 구조체 | ✅ 구현됨 | PayLetterAPI.h:83-111 |
| PL_GetBatchPaymentInfo() | ✅ 구현됨 | PayLetterAPI.cpp:772-877 |
| HMAC-SHA256 인증 | ✅ 구현됨 | PayLetterAPI.cpp |
| JSON 파싱 | ✅ 구현됨 | PayLetterAPI.cpp |

### 1.2 미구현 항목

| 항목 | 상태 | 설명 |
|------|------|------|
| WowTvSocket.cpp Include | ❌ 누락 | `#include "PayLetterAPI.h"` 없음 |
| Header 멤버 변수 | ❌ 누락 | REST API용 변수 15개 미선언 |
| REST API 스레드 함수 | ❌ 미구현 | PL_BatchInfoOrderReq_Process |
| Wrapper 함수 | ❌ 미구현 | getOrderInfo_host_wrapper |

---

## 2. 구현 순서 (의존성 기반)

```
Phase 1: Header 멤버 변수 추가
    ↓
Phase 2: WowTvSocket.cpp Include 추가
    ↓
Phase 3: PL_BatchInfoOrderReq_Process 스레드 함수 추가
    ↓
Phase 4: Wrapper 함수 추가
    ↓
Phase 5: UTF-8 BOM 확인
    ↓
Phase 6: 빌드 검증
```

---

## 3. Phase 1: Header 멤버 변수 추가

### 3.1 대상 파일
`ALLAT_Stockwin_Billkey_New_Scenario/ALLAT_Stockwin_Billkey_New_Scenario.h`

### 3.2 추가할 코드

기존 멤버 변수 섹션 근처에 다음 코드 추가:

```cpp
// ========================================
// REST API 지원 멤버 변수 (2026.02.01 추가)
// ========================================
volatile LONG m_bNeedRollback;         // 롤백 필요 플래그 (InterlockedExchange용)
LONG m_bPaymentApproved;               // 결제 승인 상태
BOOL m_bDisconnectProcessed;           // DisConnectProcess 중복 호출 방지

int m_nPurchaseAmt;                    // 원가 (할인 전)
char m_szCouponUseFlag[4];             // 쿠폰 사용 여부 (Y/N)
char m_szCouponName[64];               // 쿠폰명
char m_szBonusCashUseFlag[4];          // 보너스 캐시 사용 여부 (Y/N)
int m_nBonusCashUseAmt;                // 보너스 캐시 사용 금액
char m_szMemberId[48];                 // 회원 UUID
char m_szCategoryId[20];               // 상품 카테고리 ID
char m_szPurchaseLimitFlag[4];         // 구매 제한 플래그
char m_szServiceCheckFlag[4];          // 서비스 점검 플래그
char m_szPgCode[8];                    // PG 코드 (A=올앳, P=페이레터)
int m_nMemberState;                    // 고객 상태 (1=비회원, 2=유료회원, 3=기구매자)
char m_szTrialType[16];                // 무료체험 유형 (NOFREE, 1WEEK, 2WEEK, 1MONTH)
```

### 3.3 검증 명령어

```bash
grep -c "m_bNeedRollback" ALLAT_Stockwin_Billkey_New_Scenario/ALLAT_Stockwin_Billkey_New_Scenario.h
# Expected: 1
```

---

## 4. Phase 2: WowTvSocket.cpp Include 추가

### 4.1 대상 파일
`ALLAT_Stockwin_Billkey_New_Scenario/WowTvSocket.cpp`

### 4.2 추가할 코드

파일 상단 (line ~14)에 추가:

```cpp
#include "PayLetterAPI.h"  // REST API 모듈 (정기결제 정보 조회)
```

---

## 5. Phase 3: PL_BatchInfoOrderReq_Process 스레드 함수

### 5.1 추가 위치
`Wow_InfoRodocReq_Process` 함수 이후 (line ~1122)

### 5.2 전체 코드

```cpp
unsigned int __stdcall PL_BatchInfoOrderReq_Process(void *data)
{
    int ch = 0;
    int threadID;
    LPMTP *lineTablePtr = (LPMTP *)data;
    CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = 
        (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);
    
    ch = lineTablePtr->chanID;
    threadID = lineTablePtr->threadID;
    
    xprintf("[CH:%03d] PL_BatchInfoOrderReq_Process START (REST API)", ch);
    
    // 멤버 변수 초기화
    memset(pScenario->m_szMx_issue_no, 0x00, sizeof(pScenario->m_szMx_issue_no));
    pScenario->m_nPurchaseAmt = 0;
    memset(pScenario->m_szCouponUseFlag, 0x00, sizeof(pScenario->m_szCouponUseFlag));
    memset(pScenario->m_szMemberId, 0x00, sizeof(pScenario->m_szMemberId));
    
    // 스레드 유효성 검사
    if (threadID != lineTablePtr->threadID) {
        pScenario->m_bDnisInfo = -1;
        (*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
        Wow_REQ_Quithostio("PL_BatchInfoOrderReq_Process thread invalid", ch);
        _endthreadex((unsigned int)(*port)[ch].m_hThread);
        return -1;
    }
    
    CoInitialize(NULL);
    
    // PayLetter Batch API 호출
    PL_BatchPaymentInfo plInfo;
    char szReqTypeVal[32] = { 0 };
    PL_InitBatchPaymentInfo(&plInfo);
    
    strncpy_s(szReqTypeVal, sizeof(szReqTypeVal), pScenario->szDnis, _TRUNCATE);
    
    if (!PL_GetBatchPaymentInfo(1, szReqTypeVal, pScenario->m_szInputTel, "ARS", &plInfo)) {
        char errMsg[PL_MAX_ERROR_MSG + 1] = { 0 };
        PL_GetLastError(errMsg, sizeof(errMsg));
        xprintf("[CH:%03d] API 호출 실패 - %s", ch, errMsg);
        pScenario->m_bDnisInfo = -1;
        CoUninitialize();
        _endthreadex((unsigned int)(*port)[ch].m_hThread);
        return -1;
    }
    
    // 응답 데이터 저장
    char szOrderNo[80] = { 0 };
    sprintf_s(szOrderNo, sizeof(szOrderNo), "%lld", plInfo.orderNo);
    strncpy_s(pScenario->m_szMx_issue_no, sizeof(pScenario->m_szMx_issue_no), szOrderNo, _TRUNCATE);
    strncpy_s(pScenario->m_szMemberId, sizeof(pScenario->m_szMemberId), plInfo.memberId, _TRUNCATE);
    pScenario->m_nAmount = plInfo.payAmt;
    sprintf_s(pScenario->m_szsub_status, sizeof(pScenario->m_szsub_status), "%d", plInfo.batchPayType);
    
    // 롤백 플래그 설정
    if (strcmp(plInfo.couponUseFlag, "Y") == 0 || plInfo.bonusCashUseAmt > 0) {
        InterlockedExchange(&pScenario->m_bNeedRollback, TRUE);
    }
    
    pScenario->m_bDnisInfo = 1;
    CoUninitialize();
    _endthreadex((unsigned int)(*port)[ch].m_hThread);
    return 0;
}
```

---

## 6. Phase 4: Wrapper 함수

```cpp
int getOrderInfo_BatchAPI_host(int holdm)
{
    ((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_DBAccess = 0;
    if (holdm != 0) {
        if (new_guide) new_guide();
        (*lpmt)->trials = 0;
        (*lpmt)->Hmusic = HM_LOOP;
        if (set_guide) set_guide(holdm);
        if (send_guide) send_guide(NODTMF);
    }
    
    (*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;
    
    ((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hThread =
        (HANDLE)_beginthreadex(NULL, 0, PL_BatchInfoOrderReq_Process, (LPVOID)(*lpmt), 0,
            &(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->threadID));
    
    return 0;
}

int getOrderInfo_host_wrapper(int holdm)
{
    char szUseNewAPI[8] = { 0 };
    GetPrivateProfileStringA("PAYLETTER_API", "USE_NEW_API", "false",
                             szUseNewAPI, sizeof(szUseNewAPI), PARAINI);
    
    if (_stricmp(szUseNewAPI, "true") == 0) {
        return getOrderInfo_BatchAPI_host(holdm);
    }
    return getTcpOrderInfo_host(holdm);
}
```

---

## 7. 필드 매핑 테이블

| API 응답 필드 | 멤버 변수 | 변환 |
|--------------|----------|------|
| orderNo | m_szMx_issue_no | Int64→String |
| memberId | m_szMemberId | String |
| nickName | m_szCC_name | String |
| payAmt | m_nAmount | Int64 |
| batchPayType | m_szsub_status | Byte→String |
| couponUseFlag | m_szCouponUseFlag | String |

---

## 8. INI 설정

```ini
[PAYLETTER_API]
USE_NEW_API=false
IS_LIVE=false
QA_URL=https://devswbillapi.wowtv.co.kr
LIVE_URL=https://swbillapi.wowtv.co.kr
TIMEOUT=30
```

---

## 9. 검증 명령어

```bash
# Header 확인
grep -c "m_bNeedRollback" ALLAT_Stockwin_Billkey_New_Scenario/ALLAT_Stockwin_Billkey_New_Scenario.h

# Include 확인
grep "#include \"PayLetterAPI.h\"" ALLAT_Stockwin_Billkey_New_Scenario/WowTvSocket.cpp

# 함수 확인
grep "PL_BatchInfoOrderReq_Process" ALLAT_Stockwin_Billkey_New_Scenario/WowTvSocket.cpp
grep "getOrderInfo_host_wrapper" ALLAT_Stockwin_Billkey_New_Scenario/WowTvSocket.cpp

# UTF-8 BOM 확인
head -c 3 ALLAT_Stockwin_Billkey_New_Scenario/WowTvSocket.cpp | xxd -p
# Expected: efbbbf
```

---

## 10. 향후 작업

1. 호출부 변경: `getTcpOrderInfo_host(90)` → `getOrderInfo_host_wrapper(90)`
2. DisConnectProcess() 구현 (Quick cpp:2341-2355 참조)
3. GetFreeTrialAttrByDnis() 신규 함수 작성
4. SKIP_PARTNER_CONSENT 로직 추가

---

## 참조 문서

- claudedocs/BILL_KEY_REQ.md
- .sisyphus/plans/billkey-wowtvsocket-restapi.md
- claudedocs/NEW_SPEC_PL_API.md

// PayLetterAPI.cpp - REST API 통신 모듈 구현
// 페이레터 BOQV9 REST API 연동
// 작성일: 2026-01-19
// 수정: 구버전 Visual Studio (2012/2013) 호환 - C89 스타일 변수 선언
// HTTP 통신: MFC CInternetSession 사용 (AfxInet.h 호환)

#include "stdafx.h"
#include "PayLetterAPI.h"

#include <windows.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// OpenSSL 헤더
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

// ============================================================================
// 전역 변수
// ============================================================================

// 전역 설정
PL_Config g_plConfig;

// 마지막 에러 메시지
static char g_lastError[PL_MAX_ERROR_MSG + 1] = { 0 };

// 외부 로깅 함수 포인터
extern void(*xprintf)(const char *str, ...);
extern void(*eprintf)(const char *str, ...);

// ============================================================================
// 구조체 초기화 함수
// ============================================================================

void PL_InitPaymentInfo(PL_PaymentInfo* pInfo)
{
    if (pInfo == NULL) return;
    memset(pInfo, 0x00, sizeof(PL_PaymentInfo));
}

void PL_InitConfig(PL_Config* pConfig)
{
    if (pConfig == NULL) return;
    memset(pConfig, 0x00, sizeof(PL_Config));
    pConfig->timeout = 30;
    pConfig->useLegacyFallback = 1;
}

// ============================================================================
// 초기화 및 정리
// ============================================================================

int PL_Initialize(const char* iniPath, int isLive)
{
    // 모든 변수를 함수 시작 부분에 선언 (C89 호환)
    char szAppId[64];
    char szAppKey[128];
    char szBaseUrl[256];
    char szTimeout[16];
    char szUseFallback[8];
    const char* section = "PAYLETTER_API";

    if (iniPath == NULL) {
        PL_SetLastError("INI 경로가 NULL입니다.");
        return 0;
    }

    memset(szAppId, 0, sizeof(szAppId));
    memset(szAppKey, 0, sizeof(szAppKey));
    memset(szBaseUrl, 0, sizeof(szBaseUrl));
    memset(szTimeout, 0, sizeof(szTimeout));
    memset(szUseFallback, 0, sizeof(szUseFallback));

    if (isLive) {
        // 운영 서버 설정
        GetPrivateProfileStringA(section, "LIVE_APP_ID", "", szAppId, sizeof(szAppId), iniPath);
        GetPrivateProfileStringA(section, "LIVE_APP_KEY", "", szAppKey, sizeof(szAppKey), iniPath);
        GetPrivateProfileStringA(section, "LIVE_URL", "https://swbillapi.wowtv.co.kr", szBaseUrl, sizeof(szBaseUrl), iniPath);
    }
    else {
        // QA 서버 설정
        GetPrivateProfileStringA(section, "QA_APP_ID", "", szAppId, sizeof(szAppId), iniPath);
        GetPrivateProfileStringA(section, "QA_APP_KEY", "", szAppKey, sizeof(szAppKey), iniPath);
        GetPrivateProfileStringA(section, "QA_URL", "https://devswbillapi.wowtv.co.kr", szBaseUrl, sizeof(szBaseUrl), iniPath);
    }

    GetPrivateProfileStringA(section, "TIMEOUT", "30", szTimeout, sizeof(szTimeout), iniPath);
    GetPrivateProfileStringA(section, "USE_LEGACY_FALLBACK", "true", szUseFallback, sizeof(szUseFallback), iniPath);

    // 설정 검증
    if (strlen(szAppId) == 0 || strlen(szAppKey) == 0) {
        PL_SetLastError("APP_ID 또는 APP_KEY가 설정되지 않았습니다.");
        return 0;
    }

    // 전역 설정에 저장
    strncpy_s(g_plConfig.appId, sizeof(g_plConfig.appId), szAppId, _TRUNCATE);
    strncpy_s(g_plConfig.appKey, sizeof(g_plConfig.appKey), szAppKey, _TRUNCATE);
    strncpy_s(g_plConfig.baseUrl, sizeof(g_plConfig.baseUrl), szBaseUrl, _TRUNCATE);
    g_plConfig.timeout = atoi(szTimeout);
    g_plConfig.useLegacyFallback = (_stricmp(szUseFallback, "true") == 0) ? 1 : 0;

    PL_Log("PL_Initialize: isLive=%d, appId=%s, baseUrl=%s",
           isLive, g_plConfig.appId, g_plConfig.baseUrl);

    return 1;
}

void PL_Cleanup()
{
    memset(&g_plConfig, 0x00, sizeof(g_plConfig));
    memset(g_lastError, 0x00, sizeof(g_lastError));
}

// ============================================================================
// Base64 인코딩/디코딩
// ============================================================================

int PL_Base64Encode(const unsigned char* data, int len, char* outEncoded, int outEncodedSize)
{
    BIO* bmem = NULL;
    BIO* b64 = NULL;
    BUF_MEM* bptr = NULL;
    int resultLen = 0;

    if (data == NULL || outEncoded == NULL || outEncodedSize <= 0) {
        return 0;
    }

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);

    BIO_write(b64, data, len);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    resultLen = (int)bptr->length;
    if (resultLen >= outEncodedSize) {
        resultLen = outEncodedSize - 1;
    }

    memcpy(outEncoded, bptr->data, resultLen);
    outEncoded[resultLen] = '\0';

    BIO_free_all(b64);

    return resultLen;
}

int PL_Base64Decode(const char* encoded, unsigned char* outDecoded, int outDecodedSize)
{
    BIO* bio = NULL;
    BIO* b64 = NULL;
    int encodedLen = 0;
    int actualLen = 0;

    if (encoded == NULL || outDecoded == NULL || outDecodedSize <= 0) {
        return 0;
    }

    encodedLen = (int)strlen(encoded);

    // const_cast로 구버전 OpenSSL 호환
    bio = BIO_new_mem_buf((void*)encoded, encodedLen);
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);

    actualLen = BIO_read(bio, outDecoded, outDecodedSize);

    BIO_free_all(bio);

    if (actualLen <= 0) {
        return 0;
    }

    return actualLen;
}

// ============================================================================
// 인증 관련 함수
// ============================================================================

void PL_GenerateNonce(char* outNonce, int outNonceSize)
{
    SYSTEMTIME st;

    if (outNonce == NULL || outNonceSize <= 0) return;

    // UUID 형식의 Nonce 생성
    // 형식: yyyyMMddHHmmssSSS + random
    GetSystemTime(&st);

    sprintf_s(outNonce, outNonceSize, "%04d%02d%02d%02d%02d%02d%03d%04d",
              st.wYear, st.wMonth, st.wDay,
              st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
              rand() % 10000);
}

void PL_GetTimestamp(char* outTimestamp, int outTimestampSize)
{
    time_t now;

    if (outTimestamp == NULL || outTimestampSize <= 0) return;

    now = time(NULL);
    sprintf_s(outTimestamp, outTimestampSize, "%lld", (__int64)now);
}

void PL_ToUpper(const char* str, char* outUpper, int outUpperSize)
{
    int i = 0;
    int len = 0;

    if (str == NULL || outUpper == NULL || outUpperSize <= 0) return;

    len = (int)strlen(str);
    if (len >= outUpperSize) {
        len = outUpperSize - 1;
    }

    for (i = 0; i < len; i++) {
        outUpper[i] = (char)toupper((unsigned char)str[i]);
    }
    outUpper[len] = '\0';
}

int PL_GenerateHMACSHA256(const unsigned char* key, int keyLen,
                          const char* data, char* outSignature, int outSignatureSize)
{
    unsigned char hmacResult[EVP_MAX_MD_SIZE];
    unsigned int hmacLen = 0;
    int encLen = 0;

    if (key == NULL || data == NULL || outSignature == NULL) {
        return 0;
    }

    HMAC(EVP_sha256(),
         key, keyLen,
         (const unsigned char*)data, strlen(data),
         hmacResult, &hmacLen);

    if (hmacLen == 0) {
        PL_SetLastError("HMAC-SHA256 생성 실패");
        return 0;
    }

    // Base64 인코딩
    encLen = PL_Base64Encode(hmacResult, (int)hmacLen, outSignature, outSignatureSize);
    return (encLen > 0) ? 1 : 0;
}

int PL_GenerateAuthHeader(char* outHeader, int outHeaderSize)
{
    char nonce[64];
    char timestamp[32];
    char requestString[512];
    unsigned char appKeyBytes[128];
    int appKeyLen = 0;
    char signature[256];

    if (outHeader == NULL || outHeaderSize <= 0) {
        return 0;
    }

    memset(nonce, 0, sizeof(nonce));
    memset(timestamp, 0, sizeof(timestamp));
    memset(requestString, 0, sizeof(requestString));
    memset(appKeyBytes, 0, sizeof(appKeyBytes));
    memset(signature, 0, sizeof(signature));

    // 1. Nonce 생성
    PL_GenerateNonce(nonce, sizeof(nonce));

    // 2. Timestamp 생성 (UTC)
    PL_GetTimestamp(timestamp, sizeof(timestamp));

    // 3. RequestString 생성
    // RequestString = APP_ID + UpperCase(METHOD) + Timestamp + Nonce
    sprintf_s(requestString, sizeof(requestString), "%s%s%s%s",
              g_plConfig.appId, "POST", timestamp, nonce);

    // 4. APP_KEY Base64 디코딩
    appKeyLen = PL_Base64Decode(g_plConfig.appKey, appKeyBytes, sizeof(appKeyBytes));
    if (appKeyLen <= 0) {
        PL_SetLastError("APP_KEY Base64 디코딩 실패");
        return 0;
    }

    // 5. HMAC-SHA256 서명 생성
    if (!PL_GenerateHMACSHA256(appKeyBytes, appKeyLen, requestString, signature, sizeof(signature))) {
        return 0;
    }

    // 6. Authorization 헤더 조립
    // PLTOKEN {APP_ID}:{Signature}:{Nonce}:{Timestamp}
    sprintf_s(outHeader, outHeaderSize, "PLTOKEN %s:%s:%s:%s",
              g_plConfig.appId, signature, nonce, timestamp);

    PL_Log("PL_GenerateAuthHeader: RequestString=%s", requestString);
    PL_Log("PL_GenerateAuthHeader: Signature=%s", signature);

    return 1;
}

// ============================================================================
// JSON 처리 함수 (간단한 구현 - 외부 라이브러리 없이)
// ============================================================================

// 간단한 JSON 파서 - key에 해당하는 string 값 추출
int PL_JsonGetString(const char* json, const char* key, char* outValue, int outValueSize)
{
    char searchKey[256];
    const char* keyPos = NULL;
    const char* colonPos = NULL;
    const char* valueStart = NULL;
    const char* stringStart = NULL;
    const char* stringEnd = NULL;
    int copyLen = 0;
    int i = 0;

    if (json == NULL || key == NULL || outValue == NULL || outValueSize <= 0) {
        return 0;
    }

    outValue[0] = '\0';
    memset(searchKey, 0, sizeof(searchKey));

    // "key" 형태로 검색
    sprintf_s(searchKey, sizeof(searchKey), "\"%s\"", key);

    keyPos = strstr(json, searchKey);
    if (keyPos == NULL) {
        return 0;
    }

    // "key" 다음에 : 찾기
    colonPos = strchr(keyPos + strlen(searchKey), ':');
    if (colonPos == NULL) {
        return 0;
    }

    // 콜론 다음 공백 스킵
    valueStart = colonPos + 1;
    while (*valueStart && (*valueStart == ' ' || *valueStart == '\t' || *valueStart == '\n' || *valueStart == '\r')) {
        valueStart++;
    }

    if (*valueStart == '\0') {
        return 0;
    }

    // null 체크
    if (strncmp(valueStart, "null", 4) == 0) {
        return 1; // null은 빈 문자열로 반환 (성공)
    }

    // 값이 문자열인 경우 (따옴표로 시작)
    if (*valueStart == '"') {
        stringStart = valueStart + 1;
        stringEnd = strchr(stringStart, '"');
        if (stringEnd != NULL) {
            copyLen = (int)(stringEnd - stringStart);
            if (copyLen >= outValueSize) {
                copyLen = outValueSize - 1;
            }
            strncpy_s(outValue, outValueSize, stringStart, copyLen);
            return 1;
        }
    }
    else {
        // 숫자나 다른 값 (쉼표, 중괄호, 대괄호까지)
        i = 0;
        while (valueStart[i] && valueStart[i] != ',' && valueStart[i] != '}' &&
               valueStart[i] != ']' && valueStart[i] != '\n' && valueStart[i] != '\r') {
            if (i >= outValueSize - 1) break;
            outValue[i] = valueStart[i];
            i++;
        }
        outValue[i] = '\0';

        // 앞뒤 공백 제거
        while (i > 0 && (outValue[i-1] == ' ' || outValue[i-1] == '\t')) {
            outValue[--i] = '\0';
        }
        return 1;
    }

    return 0;
}

__int64 PL_JsonGetInt64(const char* json, const char* key)
{
    char value[64];

    memset(value, 0, sizeof(value));
    if (!PL_JsonGetString(json, key, value, sizeof(value))) {
        return 0;
    }
    if (value[0] == '\0') {
        return 0;
    }
    return _atoi64(value);
}

int PL_JsonGetInt(const char* json, const char* key)
{
    return (int)PL_JsonGetInt64(json, key);
}

void PL_JsonBuildRequest(int reqType, const char* reqTypeVal,
                         const char* phoneNo, const char* arsType,
                         char* outJson, int outJsonSize)
{
    if (outJson == NULL || outJsonSize <= 0) return;

    sprintf_s(outJson, outJsonSize,
              "{\"reqType\":%d,\"reqTypeVal\":\"%s\",\"phoneNo\":\"%s\",\"ARSType\":\"%s\"}",
              reqType,
              reqTypeVal ? reqTypeVal : "",
              phoneNo ? phoneNo : "",
              arsType ? arsType : "");
}

// ============================================================================
// HTTP 통신 함수 (MFC CInternetSession 사용)
// ============================================================================

int PL_HttpPost(const char* endpoint, const char* jsonBody,
                char* outResponse, int outResponseSize, int* outStatusCode)
{
    CInternetSession* pSession = NULL;
    CHttpConnection* pConnection = NULL;
    CHttpFile* pFile = NULL;
    char fullUrl[1024];
    CString strUrl;
    DWORD dwServiceType = 0;
    CString strServer, strObject;
    INTERNET_PORT nPort = 0;
    DWORD dwFlags = 0;
    DWORD dwSecFlags = 0;
    char authHeader[PL_MAX_AUTH_HEADER + 1];
    CString strHeaders;
    int bodyLen = 0;
    DWORD dwStatus = 0;
    char szBuf[4096];
    UINT nRead = 0;
    int totalRead = 0;
    int copyLen = 0;

    if (outResponse == NULL || outResponseSize <= 0) {
        return 0;
    }

    outResponse[0] = '\0';
    if (outStatusCode) *outStatusCode = 0;

    memset(fullUrl, 0, sizeof(fullUrl));
    memset(authHeader, 0, sizeof(authHeader));

    pSession = new CInternetSession(_T("PayLetterAPI/1.0"));

    try {
        // URL 조립
        sprintf_s(fullUrl, sizeof(fullUrl), "%s%s", g_plConfig.baseUrl, endpoint);
        strUrl = fullUrl;

        if (!AfxParseURL(strUrl, dwServiceType, strServer, strObject, nPort)) {
            PL_SetLastError("URL 파싱 실패");
            delete pSession;
            return 0;
        }

        // HTTPS 연결
        dwFlags = INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
                 INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_RELOAD |
                 INTERNET_FLAG_NO_CACHE_WRITE;

        pConnection = pSession->GetHttpConnection(strServer, nPort);
        if (pConnection == NULL) {
            PL_SetLastError("HTTP 연결 실패");
            delete pSession;
            return 0;
        }

        pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,
                                         strObject, NULL, 1, NULL, NULL, dwFlags);
        if (pFile == NULL) {
            PL_SetLastError("HTTP 요청 생성 실패");
            delete pConnection;
            delete pSession;
            return 0;
        }

        // SSL 인증서 검증 무시 옵션
        pFile->QueryOption(INTERNET_OPTION_SECURITY_FLAGS, dwSecFlags);
        dwSecFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                      SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                      SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
        pFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, dwSecFlags);

        // Authorization 헤더 생성 및 추가
        if (!PL_GenerateAuthHeader(authHeader, sizeof(authHeader))) {
            delete pFile;
            delete pConnection;
            delete pSession;
            return 0;
        }

        strHeaders.Format(_T("Content-Type: application/json; charset=utf-8\r\nAuthorization: %s\r\n"),
                         CString(authHeader));

        // eprintf 로그 출력
        if (eprintf) {
            eprintf("[PayLetterAPI] PL_HttpPost: ========== API 요청 시작 ==========");
            eprintf("[PayLetterAPI] PL_HttpPost: URL=%s", fullUrl);
            eprintf("[PayLetterAPI] PL_HttpPost: Server=%s, Port=%d", (LPCTSTR)strServer, nPort);
        }

        PL_Log("PL_HttpPost: Endpoint=%s", endpoint);
        PL_Log("PL_HttpPost: Authorization=%s...%s (masked)",
               strlen(authHeader) > 20 ? "BASIC " : "",
               strlen(authHeader) > 20 ? "(hidden)" : authHeader);
        PL_Log("PL_HttpPost: Body=%s", jsonBody ? jsonBody : "(null)");
        PL_Log("PL_HttpPost: BodyLength=%d", jsonBody ? (int)strlen(jsonBody) : 0);

        // 요청 전송
        bodyLen = jsonBody ? (int)strlen(jsonBody) : 0;
        if (!pFile->SendRequest(strHeaders, (LPVOID)jsonBody, (DWORD)bodyLen)) {
            PL_SetLastError("HTTP 요청 전송 실패");
            delete pFile;
            delete pConnection;
            delete pSession;
            return 0;
        }

        // 상태 코드 확인
        pFile->QueryInfoStatusCode(dwStatus);
        if (outStatusCode) *outStatusCode = (int)dwStatus;

        // 응답 읽기
        totalRead = 0;
        while ((nRead = pFile->Read(szBuf, sizeof(szBuf) - 1)) > 0) {
            szBuf[nRead] = '\0';
            copyLen = nRead;
            if (totalRead + copyLen >= outResponseSize) {
                copyLen = outResponseSize - totalRead - 1;
            }
            if (copyLen > 0) {
                memcpy(outResponse + totalRead, szBuf, copyLen);
                totalRead += copyLen;
            }
            if (totalRead >= outResponseSize - 1) break;
        }
        outResponse[totalRead] = '\0';

        // eprintf 로그 출력
        if (eprintf) {
            eprintf("[PayLetterAPI] PL_HttpPost: ========== API 응답 수신 ==========");
            eprintf("[PayLetterAPI] PL_HttpPost: StatusCode=%d, ResponseLength=%d", (int)dwStatus, totalRead);
            eprintf("[PayLetterAPI] PL_HttpPost: Response=%s", outResponse);
        }

        PL_Log("PL_HttpPost: ========== API 요청 완료 ==========");

        delete pFile;
        delete pConnection;
        pSession->Close();
        delete pSession;

        return 1;
    }
    catch (CInternetException* pEx) {
        TCHAR szError[256];
        char errMsg[512];
        DWORD dwError = pEx->m_dwError;

        memset(szError, 0, sizeof(szError));
        memset(errMsg, 0, sizeof(errMsg));

        pEx->GetErrorMessage(szError, 256);
        CStringA strErrorA(szError);

        sprintf_s(errMsg, sizeof(errMsg), "HTTP 예외: %s (ErrorCode=%lu)", (const char*)strErrorA, dwError);
        PL_SetLastError(errMsg);
        PL_Log("PL_HttpPost: ========== API 예외 발생 ==========");
        PL_Log("PL_HttpPost: ErrorCode=%lu", dwError);
        PL_Log("PL_HttpPost: ErrorMessage=%s", (const char*)strErrorA);
        pEx->Delete();

        if (pFile) delete pFile;
        if (pConnection) delete pConnection;
        if (pSession) {
            pSession->Close();
            delete pSession;
        }

        return 0;
    }
}

// ============================================================================
// API 호출 함수
// ============================================================================

int PL_GetPaymentInfo(int reqType, const char* reqTypeVal,
                      const char* phoneNo, const char* arsType,
                      PL_PaymentInfo* outInfo)
{
    char normalizedPhone[32];
    char jsonBody[PL_MAX_JSON_BUFFER];
    char response[PL_MAX_RESPONSE_BUFFER];
    int statusCode = 0;

    if (outInfo == NULL) {
        return 0;
    }

    PL_InitPaymentInfo(outInfo);

    memset(normalizedPhone, 0, sizeof(normalizedPhone));
    memset(jsonBody, 0, sizeof(jsonBody));
    memset(response, 0, sizeof(response));

    // 휴대폰번호 정규화
    PL_NormalizePhoneNo(phoneNo, normalizedPhone, sizeof(normalizedPhone));

    // eprintf 로그 출력
    if (eprintf) {
        eprintf("[PayLetterAPI] PL_GetPaymentInfo: ========== 결제정보 조회 시작 ==========");
        eprintf("[PayLetterAPI] PL_GetPaymentInfo: reqType=%d, reqTypeVal=%s, phoneNo=%s, arsType=%s",
                reqType, reqTypeVal ? reqTypeVal : "(null)", normalizedPhone, arsType ? arsType : "(null)");
    }

    PL_Log("PL_GetPaymentInfo: ========== 결제정보 조회 시작 ==========");
    PL_Log("PL_GetPaymentInfo: reqType=%d, reqTypeVal=%s", reqType, reqTypeVal ? reqTypeVal : "(null)");
    PL_Log("PL_GetPaymentInfo: phoneNo=%s -> normalized=%s", phoneNo ? phoneNo : "(null)", normalizedPhone);
    PL_Log("PL_GetPaymentInfo: arsType=%s", arsType ? arsType : "(null)");

    // JSON 요청 생성
    PL_JsonBuildRequest(reqType, reqTypeVal, normalizedPhone, arsType, jsonBody, sizeof(jsonBody));
    PL_Log("PL_GetPaymentInfo: JSON Request=%s", jsonBody);

    // API 호출
    if (!PL_HttpPost("/v1/payment/simple/getpaymentinfo_V2", jsonBody, response, sizeof(response), &statusCode)) {
        PL_Log("PL_GetPaymentInfo: HTTP 요청 실패");
        return 0;
    }

    // 응답 파싱
    if (statusCode == 200) {
        // 성공 응답
        PL_JsonGetString(response, "resultCode", outInfo->resultCode, sizeof(outInfo->resultCode));

        if (strcmp(outInfo->resultCode, "0") == 0) {
            // data 객체 내의 필드들 파싱
            // 한글 필드용 임시 버퍼 (UTF-8 → EUC-KR 변환용)
            char tempUtf8[512];

            PL_JsonGetString(response, "memberId", outInfo->memberId, sizeof(outInfo->memberId));
            outInfo->orderNo = PL_JsonGetInt64(response, "orderNo");

            // nickName (한글 가능) - UTF-8 → EUC-KR 변환
            memset(tempUtf8, 0, sizeof(tempUtf8));
            PL_JsonGetString(response, "nickName", tempUtf8, sizeof(tempUtf8));
            PL_ConvertUtf8ToEucKr(tempUtf8, outInfo->nickName, sizeof(outInfo->nickName));

            // itemName (한글) - UTF-8 → EUC-KR 변환
            memset(tempUtf8, 0, sizeof(tempUtf8));
            PL_JsonGetString(response, "itemName", tempUtf8, sizeof(tempUtf8));
            PL_ConvertUtf8ToEucKr(tempUtf8, outInfo->itemName, sizeof(outInfo->itemName));

            PL_JsonGetString(response, "pgCode", outInfo->pgCode, sizeof(outInfo->pgCode));
            PL_JsonGetString(response, "categoryId_2nd", outInfo->categoryId_2nd, sizeof(outInfo->categoryId_2nd));
            PL_JsonGetString(response, "merchantId", outInfo->merchantId, sizeof(outInfo->merchantId));
            PL_JsonGetString(response, "mallIdSimple", outInfo->mallIdSimple, sizeof(outInfo->mallIdSimple));
            PL_JsonGetString(response, "mallIdGeneral", outInfo->mallIdGeneral, sizeof(outInfo->mallIdGeneral));
            outInfo->payAmt = PL_JsonGetInt(response, "payAmt");
            outInfo->purchaseAmt = PL_JsonGetInt(response, "purchaseAmt");
            PL_JsonGetString(response, "CouponUseFlag", outInfo->couponUseFlag, sizeof(outInfo->couponUseFlag));

            // CouponName (한글 가능) - UTF-8 → EUC-KR 변환
            memset(tempUtf8, 0, sizeof(tempUtf8));
            PL_JsonGetString(response, "CouponName", tempUtf8, sizeof(tempUtf8));
            PL_ConvertUtf8ToEucKr(tempUtf8, outInfo->couponName, sizeof(outInfo->couponName));

            PL_JsonGetString(response, "BonusCashUseFlag", outInfo->bonusCashUseFlag, sizeof(outInfo->bonusCashUseFlag));
            outInfo->bonusCashUseAmt = PL_JsonGetInt(response, "BonusCashUseAmt");

            // cardCompany (한글 가능) - UTF-8 → EUC-KR 변환
            memset(tempUtf8, 0, sizeof(tempUtf8));
            PL_JsonGetString(response, "cardCompany", tempUtf8, sizeof(tempUtf8));
            PL_ConvertUtf8ToEucKr(tempUtf8, outInfo->cardCompany, sizeof(outInfo->cardCompany));

            PL_JsonGetString(response, "purchaseLimitFlag", outInfo->purchaseLimitFlag, sizeof(outInfo->purchaseLimitFlag));
            PL_JsonGetString(response, "payAgreeFlag", outInfo->payAgreeFlag, sizeof(outInfo->payAgreeFlag));
            outInfo->memberState = PL_JsonGetInt(response, "memberState");
            PL_JsonGetString(response, "serviceCheckFlag", outInfo->serviceCheckFlag, sizeof(outInfo->serviceCheckFlag));
            PL_JsonGetString(response, "checkCompleteTime", outInfo->checkCompleteTime, sizeof(outInfo->checkCompleteTime));
            PL_JsonGetString(response, "notiUrlSimple", outInfo->notiUrlSimple, sizeof(outInfo->notiUrlSimple));
            PL_JsonGetString(response, "notiUrlGeneral", outInfo->notiUrlGeneral, sizeof(outInfo->notiUrlGeneral));
            PL_JsonGetString(response, "billKey", outInfo->billKey, sizeof(outInfo->billKey));
            PL_JsonGetString(response, "billPassword", outInfo->billPassword, sizeof(outInfo->billPassword));

            PL_Log("PL_GetPaymentInfo: ========== 결제정보 조회 성공 ==========");
            PL_Log("PL_GetPaymentInfo: resultCode=%s", outInfo->resultCode);
            PL_Log("PL_GetPaymentInfo: memberId=%s", outInfo->memberId);
            PL_Log("PL_GetPaymentInfo: orderNo=%lld", outInfo->orderNo);
            PL_Log("PL_GetPaymentInfo: nickName=%s", outInfo->nickName);
            PL_Log("PL_GetPaymentInfo: itemName=%s", outInfo->itemName);
            PL_Log("PL_GetPaymentInfo: pgCode=%s", outInfo->pgCode);
            PL_Log("PL_GetPaymentInfo: payAmt=%d, purchaseAmt=%d", outInfo->payAmt, outInfo->purchaseAmt);
            PL_Log("PL_GetPaymentInfo: purchaseLimitFlag=%s", outInfo->purchaseLimitFlag);
            PL_Log("PL_GetPaymentInfo: payAgreeFlag=%s", outInfo->payAgreeFlag);
            PL_Log("PL_GetPaymentInfo: memberState=%d", outInfo->memberState);
            PL_Log("PL_GetPaymentInfo: billKey=%s", strlen(outInfo->billKey) > 0 ? "(exists)" : "(empty)");

            return 1;
        }
        else {
            // resultCode가 0이 아닌 경우
            sprintf_s(outInfo->resultMessage, sizeof(outInfo->resultMessage),
                      "API 응답 resultCode: %s", outInfo->resultCode);
            PL_SetLastError(outInfo->resultMessage);
            PL_Log("PL_GetPaymentInfo: ========== 결제정보 조회 실패 (resultCode!=0) ==========");
            PL_Log("PL_GetPaymentInfo: resultCode=%s", outInfo->resultCode);
            return 0;
        }
    }
    else {
        // 오류 응답
        outInfo->errorCode = PL_JsonGetInt(response, "code");
        PL_JsonGetString(response, "message", outInfo->resultMessage, sizeof(outInfo->resultMessage));

        if (strlen(outInfo->resultMessage) == 0) {
            sprintf_s(outInfo->resultMessage, sizeof(outInfo->resultMessage), "HTTP 오류: %d", statusCode);
        }

        PL_SetLastError(outInfo->resultMessage);
        PL_Log("PL_GetPaymentInfo: ========== 결제정보 조회 실패 (HTTP 오류) ==========");
        PL_Log("PL_GetPaymentInfo: statusCode=%d", statusCode);
        PL_Log("PL_GetPaymentInfo: errorCode=%d", outInfo->errorCode);
        PL_Log("PL_GetPaymentInfo: errorMessage=%s", outInfo->resultMessage);

        return 0;
    }
}

int PL_RegisterAgree(const char* phoneNo, const char* pgCode, const char* agreeFlag)
{
    char normalizedPhone[32];
    char jsonBody[256];
    char response[PL_MAX_RESPONSE_BUFFER];
    int statusCode = 0;
    char resultCode[16];

    memset(normalizedPhone, 0, sizeof(normalizedPhone));
    memset(jsonBody, 0, sizeof(jsonBody));
    memset(response, 0, sizeof(response));
    memset(resultCode, 0, sizeof(resultCode));

    // 휴대폰번호 정규화
    PL_NormalizePhoneNo(phoneNo, normalizedPhone, sizeof(normalizedPhone));

    // JSON 요청 생성
    sprintf_s(jsonBody, sizeof(jsonBody),
              "{\"phoneNo\":\"%s\",\"pgCode\":\"%s\",\"agreeFlag\":\"%s\"}",
              normalizedPhone,
              pgCode ? pgCode : "",
              agreeFlag ? agreeFlag : "");

    if (!PL_HttpPost("/v1/payment/simple/agree", jsonBody, response, sizeof(response), &statusCode)) {
        return 0;
    }

    if (statusCode == 200) {
        PL_JsonGetString(response, "resultCode", resultCode, sizeof(resultCode));
        return (strcmp(resultCode, "0") == 0) ? 1 : 0;
    }

    return 0;
}

// ============================================================================
// 에러 처리
// ============================================================================

void PL_GetLastError(char* outError, int outErrorSize)
{
    if (outError == NULL || outErrorSize <= 0) return;
    strncpy_s(outError, outErrorSize, g_lastError, _TRUNCATE);
}

void PL_SetLastError(const char* error)
{
    if (error == NULL) {
        g_lastError[0] = '\0';
    }
    else {
        strncpy_s(g_lastError, sizeof(g_lastError), error, _TRUNCATE);
    }
    PL_Log("PL_Error: %s", error ? error : "(null)");
}

void PL_GetPurchaseLimitMessage(const char* flag, char* outMessage, int outMessageSize)
{
    if (outMessage == NULL || outMessageSize <= 0) return;

    if (flag == NULL || flag[0] == '\0') {
        strncpy_s(outMessage, outMessageSize, "알 수 없는 상태", _TRUNCATE);
        return;
    }

    if (strcmp(flag, "1") == 0) {
        strncpy_s(outMessage, outMessageSize, "정상 (구매가능)", _TRUNCATE);
    }
    else if (strcmp(flag, "2") == 0) {
        strncpy_s(outMessage, outMessageSize, "불량사용자 등록", _TRUNCATE);
    }
    else if (strcmp(flag, "3") == 0) {
        strncpy_s(outMessage, outMessageSize, "구매 가능 횟수 초과", _TRUNCATE);
    }
    else if (strcmp(flag, "4") == 0) {
        strncpy_s(outMessage, outMessageSize, "판매 시작전 상품", _TRUNCATE);
    }
    else if (strcmp(flag, "5") == 0) {
        strncpy_s(outMessage, outMessageSize, "판매 종료 상품", _TRUNCATE);
    }
    else if (strcmp(flag, "6") == 0) {
        strncpy_s(outMessage, outMessageSize, "판매 중지 상품", _TRUNCATE);
    }
    else {
        sprintf_s(outMessage, outMessageSize, "알 수 없는 상태: %s", flag);
    }
}

// ============================================================================
// 유틸리티 함수
// ============================================================================

void PL_NormalizePhoneNo(const char* phoneNo, char* outNormalized, int outNormalizedSize)
{
    int j = 0;
    int len = 0;
    int i = 0;

    if (outNormalized == NULL || outNormalizedSize <= 0) return;
    outNormalized[0] = '\0';

    if (phoneNo == NULL) return;

    len = (int)strlen(phoneNo);
    for (i = 0; i < len && j < outNormalizedSize - 1; i++) {
        if (isdigit((unsigned char)phoneNo[i])) {
            outNormalized[j++] = phoneNo[i];
        }
    }
    outNormalized[j] = '\0';
}

void PL_Log(const char* format, ...)
{
    char buffer[2048];
    va_list args;

    if (eprintf == NULL) {
        return;
    }

    memset(buffer, 0, sizeof(buffer));
    va_start(args, format);
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
    va_end(args);

    eprintf("[PayLetterAPI] %s", buffer);
}

// ============================================================================
// UTF-8 → EUC-KR(CP949) 변환
// ============================================================================

int PL_ConvertUtf8ToEucKr(const char* utf8Str, char* outEucKr, int outEucKrSize)
{
    wchar_t* wideStr = NULL;
    int wideLen = 0;
    int eucKrLen = 0;

    if (utf8Str == NULL || outEucKr == NULL || outEucKrSize <= 0) {
        return 0;
    }

    outEucKr[0] = '\0';

    // 빈 문자열 처리
    if (utf8Str[0] == '\0') {
        return 1;
    }

    // 1단계: UTF-8 → UTF-16 (Wide Character)
    wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
    if (wideLen <= 0) {
        // 변환 실패 시 원본 복사
        strncpy_s(outEucKr, outEucKrSize, utf8Str, _TRUNCATE);
        return 0;
    }

    wideStr = (wchar_t*)malloc(wideLen * sizeof(wchar_t));
    if (wideStr == NULL) {
        strncpy_s(outEucKr, outEucKrSize, utf8Str, _TRUNCATE);
        return 0;
    }

    if (MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, wideStr, wideLen) <= 0) {
        free(wideStr);
        strncpy_s(outEucKr, outEucKrSize, utf8Str, _TRUNCATE);
        return 0;
    }

    // 2단계: UTF-16 → EUC-KR (CP949)
    eucKrLen = WideCharToMultiByte(949, 0, wideStr, -1, NULL, 0, NULL, NULL);
    if (eucKrLen <= 0 || eucKrLen > outEucKrSize) {
        free(wideStr);
        strncpy_s(outEucKr, outEucKrSize, utf8Str, _TRUNCATE);
        return 0;
    }

    if (WideCharToMultiByte(949, 0, wideStr, -1, outEucKr, outEucKrSize, NULL, NULL) <= 0) {
        free(wideStr);
        strncpy_s(outEucKr, outEucKrSize, utf8Str, _TRUNCATE);
        return 0;
    }

    free(wideStr);
    return 1;
}

// PayLetterAPI.cpp - REST API 통신 모듈 구현
// 페이레터 BOQV9 REST API 연동
// 작성일: 2026-01-19

#include "stdafx.h"
#include "PayLetterAPI.h"

#include <windows.h>
#include <winhttp.h>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdarg.h>

// OpenSSL 헤더
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

// WinHTTP 라이브러리 링크
#pragma comment(lib, "winhttp.lib")

// ============================================================================
// 전역 변수
// ============================================================================

// 전역 설정
PL_Config g_plConfig;

// 마지막 에러 메시지
static std::string g_lastError;

// 외부 로깅 함수 포인터
extern void(*xprintf)(const char *str, ...);

// ============================================================================
// 초기화 및 정리
// ============================================================================

bool PL_Initialize(const char* iniPath, bool isLive)
{
    if (iniPath == NULL) {
        PL_SetLastError("INI 경로가 NULL입니다.");
        return false;
    }

    char szAppId[64] = { 0 };
    char szAppKey[128] = { 0 };
    char szBaseUrl[256] = { 0 };
    char szTimeout[16] = { 0 };
    char szUseFallback[8] = { 0 };

    const char* section = "PAYLETTER_API";

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
        return false;
    }

    // 전역 설정에 저장
    g_plConfig.appId = szAppId;
    g_plConfig.appKey = szAppKey;
    g_plConfig.baseUrl = szBaseUrl;
    g_plConfig.timeout = atoi(szTimeout);
    g_plConfig.useLegacyFallback = (_stricmp(szUseFallback, "true") == 0);

    PL_Log("PL_Initialize: isLive=%d, appId=%s, baseUrl=%s",
           isLive ? 1 : 0, g_plConfig.appId.c_str(), g_plConfig.baseUrl.c_str());

    return true;
}

void PL_Cleanup()
{
    g_plConfig.appId.clear();
    g_plConfig.appKey.clear();
    g_plConfig.baseUrl.clear();
    g_lastError.clear();
}

// ============================================================================
// Base64 인코딩/디코딩
// ============================================================================

std::string PL_Base64Encode(const unsigned char* data, size_t len)
{
    BIO* bmem = NULL;
    BIO* b64 = NULL;
    BUF_MEM* bptr = NULL;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);

    BIO_write(b64, data, (int)len);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    std::string result(bptr->data, bptr->length);
    BIO_free_all(b64);

    return result;
}

std::vector<unsigned char> PL_Base64Decode(const std::string& encoded)
{
    BIO* bio = NULL;
    BIO* b64 = NULL;
    std::vector<unsigned char> result;

    size_t decodeLen = encoded.length();
    result.resize(decodeLen);

    bio = BIO_new_mem_buf(encoded.c_str(), (int)encoded.length());
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);

    int actualLen = BIO_read(bio, result.data(), (int)decodeLen);
    if (actualLen > 0) {
        result.resize(actualLen);
    }
    else {
        result.clear();
    }

    BIO_free_all(bio);

    return result;
}

// ============================================================================
// 인증 관련 함수
// ============================================================================

std::string PL_GenerateNonce()
{
    // UUID 형식의 Nonce 생성
    // 형식: yyyyMMddHHmmssSSS + random
    SYSTEMTIME st;
    GetSystemTime(&st);

    char buffer[64];
    sprintf_s(buffer, sizeof(buffer), "%04d%02d%02d%02d%02d%02d%03d%04d",
              st.wYear, st.wMonth, st.wDay,
              st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
              rand() % 10000);

    return std::string(buffer);
}

std::string PL_GetTimestamp()
{
    time_t now = time(NULL);
    char buffer[32];
    sprintf_s(buffer, sizeof(buffer), "%lld", (long long)now);
    return std::string(buffer);
}

std::string PL_ToUpper(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool PL_GenerateHMACSHA256(const unsigned char* key, size_t keyLen,
                           const std::string& data, std::string& outSignature)
{
    unsigned char hmacResult[EVP_MAX_MD_SIZE];
    unsigned int hmacLen = 0;

    HMAC(EVP_sha256(),
         key, (int)keyLen,
         (const unsigned char*)data.c_str(), data.length(),
         hmacResult, &hmacLen);

    if (hmacLen == 0) {
        PL_SetLastError("HMAC-SHA256 생성 실패");
        return false;
    }

    outSignature = PL_Base64Encode(hmacResult, hmacLen);
    return true;
}

std::string PL_GenerateAuthHeader()
{
    // 1. Nonce 생성
    std::string nonce = PL_GenerateNonce();

    // 2. Timestamp 생성 (UTC)
    std::string timestamp = PL_GetTimestamp();

    // 3. RequestString 생성
    // RequestString = APP_ID + UpperCase(METHOD) + Timestamp + Nonce
    std::string requestString = g_plConfig.appId + "POST" + timestamp + nonce;

    // 4. APP_KEY Base64 디코딩
    std::vector<unsigned char> appKeyBytes = PL_Base64Decode(g_plConfig.appKey);
    if (appKeyBytes.empty()) {
        PL_SetLastError("APP_KEY Base64 디코딩 실패");
        return "";
    }

    // 5. HMAC-SHA256 서명 생성
    std::string signature;
    if (!PL_GenerateHMACSHA256(appKeyBytes.data(), appKeyBytes.size(),
                               requestString, signature)) {
        return "";
    }

    // 6. Authorization 헤더 조립
    // PLTOKEN {APP_ID}:{Signature}:{Nonce}:{Timestamp}
    std::string authHeader = "PLTOKEN " + g_plConfig.appId + ":" +
                             signature + ":" + nonce + ":" + timestamp;

    PL_Log("PL_GenerateAuthHeader: RequestString=%s", requestString.c_str());
    PL_Log("PL_GenerateAuthHeader: Signature=%s", signature.c_str());

    return authHeader;
}

// ============================================================================
// JSON 처리 함수 (간단한 구현 - 외부 라이브러리 없이)
// ============================================================================

// 간단한 JSON 파서 - key에 해당하는 string 값 추출
std::string PL_JsonGetString(const std::string& json, const std::string& key)
{
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) {
        return "";
    }

    // "key" 다음에 : 찾기
    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) {
        return "";
    }

    // 콜론 다음 공백 스킵
    size_t valueStart = colonPos + 1;
    while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\t' || json[valueStart] == '\n' || json[valueStart] == '\r')) {
        valueStart++;
    }

    if (valueStart >= json.length()) {
        return "";
    }

    // null 체크
    if (json.compare(valueStart, 4, "null") == 0) {
        return "";
    }

    // 값이 문자열인 경우 (따옴표로 시작)
    if (json[valueStart] == '"') {
        size_t stringStart = valueStart + 1;
        size_t stringEnd = json.find('"', stringStart);
        if (stringEnd != std::string::npos) {
            return json.substr(stringStart, stringEnd - stringStart);
        }
    }
    else {
        // 숫자나 다른 값 (쉼표, 중괄호, 대괄호까지)
        size_t valueEnd = json.find_first_of(",}]\n\r", valueStart);
        if (valueEnd != std::string::npos) {
            std::string value = json.substr(valueStart, valueEnd - valueStart);
            // 앞뒤 공백 제거
            size_t start = value.find_first_not_of(" \t\n\r");
            size_t end = value.find_last_not_of(" \t\n\r");
            if (start != std::string::npos && end != std::string::npos) {
                return value.substr(start, end - start + 1);
            }
        }
    }

    return "";
}

long long PL_JsonGetInt64(const std::string& json, const std::string& key)
{
    std::string value = PL_JsonGetString(json, key);
    if (value.empty()) {
        return 0;
    }
    return _atoi64(value.c_str());
}

int PL_JsonGetInt(const std::string& json, const std::string& key)
{
    return (int)PL_JsonGetInt64(json, key);
}

std::string PL_JsonBuildRequest(int reqType, const std::string& reqTypeVal,
                                const std::string& phoneNo, const std::string& arsType)
{
    std::ostringstream oss;
    oss << "{";
    oss << "\"reqType\":" << reqType << ",";
    oss << "\"reqTypeVal\":\"" << reqTypeVal << "\",";
    oss << "\"phoneNo\":\"" << phoneNo << "\",";
    oss << "\"ARSType\":\"" << arsType << "\"";
    oss << "}";
    return oss.str();
}

// ============================================================================
// HTTP 통신 함수
// ============================================================================

bool PL_HttpPost(const std::string& endpoint, const std::string& jsonBody,
                 std::string& outResponse, int& outStatusCode)
{
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool result = false;

    outResponse.clear();
    outStatusCode = 0;

    // URL 파싱
    std::string baseUrl = g_plConfig.baseUrl;
    std::wstring wideUrl(baseUrl.begin(), baseUrl.end());

    URL_COMPONENTS urlComp = { 0 };
    urlComp.dwStructSize = sizeof(urlComp);

    wchar_t hostName[256] = { 0 };
    wchar_t urlPath[1024] = { 0 };

    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);

    if (!WinHttpCrackUrl(wideUrl.c_str(), (DWORD)wideUrl.length(), 0, &urlComp)) {
        PL_SetLastError("URL 파싱 실패");
        return false;
    }

    // 세션 생성
    hSession = WinHttpOpen(L"PayLetterAPI/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        PL_SetLastError("WinHttpOpen 실패");
        return false;
    }

    // 타임아웃 설정
    DWORD timeout = g_plConfig.timeout * 1000;
    WinHttpSetTimeouts(hSession, timeout, timeout, timeout, timeout);

    // 연결
    hConnect = WinHttpConnect(hSession, hostName, urlComp.nPort, 0);
    if (!hConnect) {
        PL_SetLastError("WinHttpConnect 실패");
        WinHttpCloseHandle(hSession);
        return false;
    }

    // 요청 URL 생성
    std::wstring wideEndpoint(endpoint.begin(), endpoint.end());
    std::wstring fullPath = urlPath;
    fullPath += wideEndpoint;

    // 요청 생성
    DWORD dwFlags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    hRequest = WinHttpOpenRequest(hConnect, L"POST", fullPath.c_str(),
                                  NULL, WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES, dwFlags);
    if (!hRequest) {
        PL_SetLastError("WinHttpOpenRequest 실패");
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Authorization 헤더 생성 및 추가
    std::string authHeader = PL_GenerateAuthHeader();
    if (authHeader.empty()) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    std::string headers = "Content-Type: application/json; charset=utf-8\r\n";
    headers += "Authorization: " + authHeader + "\r\n";

    std::wstring wideHeaders(headers.begin(), headers.end());

    if (!WinHttpAddRequestHeaders(hRequest, wideHeaders.c_str(),
                                  (DWORD)wideHeaders.length(),
                                  WINHTTP_ADDREQ_FLAG_ADD)) {
        PL_SetLastError("헤더 추가 실패");
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // HTTPS 인증서 검증 옵션 (필요시 무시)
    if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
        DWORD dwSecFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                          SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                          SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
        WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS,
                        &dwSecFlags, sizeof(dwSecFlags));
    }

    // 요청 전송
    PL_Log("PL_HttpPost: URL=%s%s", baseUrl.c_str(), endpoint.c_str());
    PL_Log("PL_HttpPost: Body=%s", jsonBody.c_str());

    if (!WinHttpSendRequest(hRequest,
                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            (LPVOID)jsonBody.c_str(), (DWORD)jsonBody.length(),
                            (DWORD)jsonBody.length(), 0)) {
        DWORD err = GetLastError();
        char errMsg[128];
        sprintf_s(errMsg, sizeof(errMsg), "WinHttpSendRequest 실패: %d", err);
        PL_SetLastError(errMsg);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // 응답 수신
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        PL_SetLastError("WinHttpReceiveResponse 실패");
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // 상태 코드 확인
    DWORD dwStatusCode = 0;
    DWORD dwSize = sizeof(dwStatusCode);
    if (WinHttpQueryHeaders(hRequest,
                            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX,
                            &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
        outStatusCode = (int)dwStatusCode;
    }

    // 응답 본문 읽기
    std::string responseBody;
    DWORD dwDownloaded = 0;
    DWORD dwBytesToRead = 0;
    char* pszOutBuffer = NULL;

    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            break;
        }

        if (dwSize == 0) {
            break;
        }

        pszOutBuffer = new char[dwSize + 1];
        if (!pszOutBuffer) {
            break;
        }

        ZeroMemory(pszOutBuffer, dwSize + 1);

        if (!WinHttpReadData(hRequest, pszOutBuffer, dwSize, &dwDownloaded)) {
            delete[] pszOutBuffer;
            break;
        }

        responseBody.append(pszOutBuffer, dwDownloaded);
        delete[] pszOutBuffer;

    } while (dwSize > 0);

    outResponse = responseBody;
    result = true;

    PL_Log("PL_HttpPost: StatusCode=%d", outStatusCode);
    PL_Log("PL_HttpPost: Response=%s", outResponse.c_str());

    // 리소스 정리
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return result;
}

// ============================================================================
// API 호출 함수
// ============================================================================

bool PL_GetPaymentInfo(int reqType, const std::string& reqTypeVal,
                       const std::string& phoneNo, const std::string& arsType,
                       PL_PaymentInfo& outInfo)
{
    // 휴대폰번호 정규화
    std::string normalizedPhone = PL_NormalizePhoneNo(phoneNo);

    // JSON 요청 생성
    std::string jsonBody = PL_JsonBuildRequest(reqType, reqTypeVal, normalizedPhone, arsType);

    // API 호출
    std::string response;
    int statusCode = 0;

    if (!PL_HttpPost("/v1/payment/simple/getpaymentinfo_V2", jsonBody, response, statusCode)) {
        return false;
    }

    // 응답 파싱
    if (statusCode == 200) {
        // 성공 응답
        outInfo.resultCode = PL_JsonGetString(response, "resultCode");

        if (outInfo.resultCode == "0") {
            // data 객체 내의 필드들 파싱
            outInfo.memberId = PL_JsonGetString(response, "memberId");
            outInfo.orderNo = PL_JsonGetInt64(response, "orderNo");
            outInfo.nickName = PL_JsonGetString(response, "nickName");
            outInfo.itemName = PL_JsonGetString(response, "itemName");
            outInfo.pgCode = PL_JsonGetString(response, "pgCode");
            outInfo.categoryId_2nd = PL_JsonGetString(response, "categoryId_2nd");
            outInfo.merchantId = PL_JsonGetString(response, "merchantId");
            outInfo.mallIdSimple = PL_JsonGetString(response, "mallIdSimple");
            outInfo.mallIdGeneral = PL_JsonGetString(response, "mallIdGeneral");
            outInfo.payAmt = PL_JsonGetInt(response, "payAmt");
            outInfo.purchaseAmt = PL_JsonGetInt(response, "purchaseAmt");
            outInfo.couponUseFlag = PL_JsonGetString(response, "CouponUseFlag");
            outInfo.couponName = PL_JsonGetString(response, "CouponName");
            outInfo.bonusCashUseFlag = PL_JsonGetString(response, "BonusCashUseFlag");
            outInfo.bonusCashUseAmt = PL_JsonGetInt(response, "BonusCashUseAmt");
            outInfo.cardCompany = PL_JsonGetString(response, "cardCompany");
            outInfo.purchaseLimitFlag = PL_JsonGetString(response, "purchaseLimitFlag");
            outInfo.payAgreeFlag = PL_JsonGetString(response, "payAgreeFlag");
            outInfo.memberState = PL_JsonGetInt(response, "memberState");
            outInfo.serviceCheckFlag = PL_JsonGetString(response, "serviceCheckFlag");
            outInfo.checkCompleteTime = PL_JsonGetString(response, "checkCompleteTime");
            outInfo.notiUrlSimple = PL_JsonGetString(response, "notiUrlSimple");
            outInfo.notiUrlGeneral = PL_JsonGetString(response, "notiUrlGeneral");
            outInfo.billKey = PL_JsonGetString(response, "billKey");
            outInfo.billPassword = PL_JsonGetString(response, "billPassword");

            PL_Log("PL_GetPaymentInfo: 성공 - orderNo=%lld, payAmt=%d, nickName=%s",
                   outInfo.orderNo, outInfo.payAmt, outInfo.nickName.c_str());

            return true;
        }
        else {
            // resultCode가 0이 아닌 경우
            outInfo.resultMessage = "API 응답 resultCode: " + outInfo.resultCode;
            PL_SetLastError(outInfo.resultMessage);
            return false;
        }
    }
    else {
        // 오류 응답
        outInfo.errorCode = PL_JsonGetInt(response, "code");
        outInfo.resultMessage = PL_JsonGetString(response, "message");

        if (outInfo.resultMessage.empty()) {
            char errMsg[64];
            sprintf_s(errMsg, sizeof(errMsg), "HTTP 오류: %d", statusCode);
            outInfo.resultMessage = errMsg;
        }

        PL_SetLastError(outInfo.resultMessage);
        PL_Log("PL_GetPaymentInfo: 실패 - statusCode=%d, errorCode=%d, message=%s",
               statusCode, outInfo.errorCode, outInfo.resultMessage.c_str());

        return false;
    }
}

bool PL_RegisterAgree(const std::string& phoneNo, const std::string& pgCode,
                      const std::string& agreeFlag)
{
    std::string normalizedPhone = PL_NormalizePhoneNo(phoneNo);

    std::ostringstream oss;
    oss << "{";
    oss << "\"phoneNo\":\"" << normalizedPhone << "\",";
    oss << "\"pgCode\":\"" << pgCode << "\",";
    oss << "\"agreeFlag\":\"" << agreeFlag << "\"";
    oss << "}";

    std::string response;
    int statusCode = 0;

    if (!PL_HttpPost("/v1/payment/simple/agree", oss.str(), response, statusCode)) {
        return false;
    }

    if (statusCode == 200) {
        std::string resultCode = PL_JsonGetString(response, "resultCode");
        return (resultCode == "0");
    }

    return false;
}

// ============================================================================
// 에러 처리
// ============================================================================

std::string PL_GetLastError()
{
    return g_lastError;
}

void PL_SetLastError(const std::string& error)
{
    g_lastError = error;
    PL_Log("PL_Error: %s", error.c_str());
}

std::string PL_GetPurchaseLimitMessage(const std::string& flag)
{
    if (flag == "1") return "정상 (구매가능)";
    if (flag == "2") return "불량사용자 등록";
    if (flag == "3") return "구매 가능 횟수 초과";
    if (flag == "4") return "판매 시작전 상품";
    if (flag == "5") return "판매 종료 상품";
    if (flag == "6") return "판매 중지 상품";
    return "알 수 없는 상태: " + flag;
}

// ============================================================================
// 유틸리티 함수
// ============================================================================

std::string PL_NormalizePhoneNo(const std::string& phoneNo)
{
    std::string result;
    for (size_t i = 0; i < phoneNo.length(); i++) {
        if (isdigit((unsigned char)phoneNo[i])) {
            result += phoneNo[i];
        }
    }
    return result;
}

void PL_Log(const char* format, ...)
{
    if (xprintf == NULL) {
        return;
    }

    char buffer[2048];
    va_list args;
    va_start(args, format);
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
    va_end(args);

    xprintf("[PayLetterAPI] %s", buffer);
}

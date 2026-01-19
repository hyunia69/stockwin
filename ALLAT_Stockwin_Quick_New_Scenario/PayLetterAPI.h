// PayLetterAPI.h - REST API 통신 모듈
// 페이레터 BOQV9 REST API 연동
// 작성일: 2026-01-19
// 수정: 구버전 Visual Studio (2012/2013) 호환 - C 스타일 문자 배열 사용

#pragma once

#ifndef _PAYLETTER_API_H_
#define _PAYLETTER_API_H_

// ============================================================================
// 상수 정의
// ============================================================================

#define PL_MAX_MEMBER_ID        40      // 회원 ID (UUID)
#define PL_MAX_NICK_NAME        64      // 필명
#define PL_MAX_ITEM_NAME        256     // 상품명
#define PL_MAX_PG_CODE          4       // PG 코드
#define PL_MAX_MALL_ID          64      // 상점 ID
#define PL_MAX_CATEGORY_ID      16      // 카테고리 ID
#define PL_MAX_MERCHANT_ID      64      // 가맹점 ID
#define PL_MAX_FLAG             4       // 플래그 필드
#define PL_MAX_CARD_COMPANY     32      // 카드사명
#define PL_MAX_DATETIME         32      // 일시 문자열
#define PL_MAX_URL              512     // URL
#define PL_MAX_BILL_KEY         128     // 빌키
#define PL_MAX_BILL_PASSWORD    32      // 빌키 비밀번호
#define PL_MAX_RESULT_CODE      16      // 결과코드
#define PL_MAX_RESULT_MESSAGE   256     // 결과메시지
#define PL_MAX_COUPON_NAME      64      // 쿠폰명

#define PL_MAX_APP_ID           64      // APP_ID
#define PL_MAX_APP_KEY          128     // APP_KEY (Base64)
#define PL_MAX_BASE_URL         256     // API 기본 URL

#define PL_MAX_ERROR_MSG        512     // 에러 메시지
#define PL_MAX_AUTH_HEADER      512     // Authorization 헤더
#define PL_MAX_JSON_BUFFER      4096    // JSON 버퍼
#define PL_MAX_RESPONSE_BUFFER  8192    // HTTP 응답 버퍼

// ============================================================================
// API 응답 구조체
// ============================================================================

// 구조체 정렬을 명시적으로 8바이트로 지정 (컴파일 단위 간 일관성 보장)
#pragma pack(push, 8)

typedef struct _PL_PaymentInfo {
    char memberId[PL_MAX_MEMBER_ID + 1];           // 회원 ID (UUID)
    __int64 orderNo;                                // 주문번호
    char nickName[PL_MAX_NICK_NAME + 1];           // 필명
    char itemName[PL_MAX_ITEM_NAME + 1];           // 상품명
    char pgCode[PL_MAX_PG_CODE + 1];               // PG 코드 (A=올앳, P=페이레터)
    char mallIdSimple[PL_MAX_MALL_ID + 1];         // 상점 ID (간편)
    char mallIdGeneral[PL_MAX_MALL_ID + 1];        // 상점 ID (일반)
    char categoryId_2nd[PL_MAX_CATEGORY_ID + 1];   // 중분류 카테고리ID (상품코드 대체)
    char merchantId[PL_MAX_MERCHANT_ID + 1];       // 가맹점 ID
    int payAmt;                                     // 실제 결제 금액 (할인 적용 후)
    int purchaseAmt;                                // 상품 원가 (할인 전)
    char couponUseFlag[PL_MAX_FLAG + 1];           // 쿠폰 존재 여부 (Y/N)
    char couponName[PL_MAX_COUPON_NAME + 1];       // 쿠폰 이름
    char bonusCashUseFlag[PL_MAX_FLAG + 1];        // 보너스 캐시 존재 여부 (Y/N)
    int bonusCashUseAmt;                            // 보너스 캐시 사용 금액
    char cardCompany[PL_MAX_CARD_COMPANY + 1];     // 카드사명
    char purchaseLimitFlag[PL_MAX_FLAG + 1];       // 구매 제한 여부 (1=정상, 2=불량, 3=횟수초과, 4=시작전, 5=종료, 6=중지)
    char payAgreeFlag[PL_MAX_FLAG + 1];            // 결제 동의 여부
    int memberState;                                // 고객상태 (1=비회원, 2=유료회원, 3=기구매자)
    char serviceCheckFlag[PL_MAX_FLAG + 1];        // 서비스 점검여부 (Y/N)
    char checkCompleteTime[PL_MAX_DATETIME + 1];   // 점검완료일시
    char notiUrlSimple[PL_MAX_URL + 1];            // Notification URL (간편)
    char notiUrlGeneral[PL_MAX_URL + 1];           // Notification URL (일반)
    char billKey[PL_MAX_BILL_KEY + 1];             // 정기결제 키
    char billPassword[PL_MAX_BILL_PASSWORD + 1];   // 정기결제 비밀번호

    // API 응답 상태
    char resultCode[PL_MAX_RESULT_CODE + 1];       // 결과코드 (0=성공)
    char resultMessage[PL_MAX_RESULT_MESSAGE + 1]; // 오류 메시지
    int errorCode;                                  // 오류 코드
} PL_PaymentInfo;

// ============================================================================
// API 설정 구조체
// ============================================================================

typedef struct _PL_Config {
    char appId[PL_MAX_APP_ID + 1];                 // APP_ID (32자)
    char appKey[PL_MAX_APP_KEY + 1];               // APP_KEY (Base64)
    char baseUrl[PL_MAX_BASE_URL + 1];             // API 기본 URL
    int timeout;                                    // 타임아웃 (초)
    int useLegacyFallback;                          // 레거시 API 폴백 사용 여부 (0/1)
} PL_Config;

#pragma pack(pop)  // 구조체 정렬 복원

// 전역 설정
extern PL_Config g_plConfig;

// ============================================================================
// 구조체 초기화 함수
// ============================================================================

void PL_InitPaymentInfo(PL_PaymentInfo* pInfo);
void PL_InitConfig(PL_Config* pConfig);

// ============================================================================
// 초기화 및 정리
// ============================================================================

// API 초기화 (INI 파일에서 설정 로드)
// @param iniPath INI 파일 경로
// @param isLive 1=운영, 0=QA
// @return 성공 여부 (1=성공, 0=실패)
int PL_Initialize(const char* iniPath, int isLive);

// API 정리 (리소스 해제)
void PL_Cleanup();

// ============================================================================
// 인증 관련 함수
// ============================================================================

// Authorization 헤더 생성
// RequestString = APP_ID + UpperCase(METHOD) + Timestamp + Nonce
// Signature = Base64(HMAC-SHA256(Base64Decode(APP_KEY), RequestString))
// @param outHeader 출력 버퍼 (최소 PL_MAX_AUTH_HEADER 크기)
// @return 성공 여부 (1=성공, 0=실패)
int PL_GenerateAuthHeader(char* outHeader, int outHeaderSize);

// HMAC-SHA256 서명 생성
// @param key Base64 디코딩된 키 바이트 배열
// @param keyLen 키 길이
// @param data 서명 대상 문자열
// @param outSignature 출력 서명 버퍼 (Base64 인코딩)
// @param outSignatureSize 출력 버퍼 크기
// @return 성공 여부 (1=성공, 0=실패)
int PL_GenerateHMACSHA256(const unsigned char* key, int keyLen,
                          const char* data, char* outSignature, int outSignatureSize);

// Base64 인코딩
// @param data 입력 데이터
// @param len 입력 길이
// @param outEncoded 출력 버퍼
// @param outEncodedSize 출력 버퍼 크기
// @return 인코딩된 문자열 길이 (실패 시 0)
int PL_Base64Encode(const unsigned char* data, int len, char* outEncoded, int outEncodedSize);

// Base64 디코딩
// @param encoded 인코딩된 문자열
// @param outDecoded 출력 버퍼
// @param outDecodedSize 출력 버퍼 크기
// @return 디코딩된 바이트 수 (실패 시 0)
int PL_Base64Decode(const char* encoded, unsigned char* outDecoded, int outDecodedSize);

// Nonce (UUID) 생성
// @param outNonce 출력 버퍼
// @param outNonceSize 출력 버퍼 크기
void PL_GenerateNonce(char* outNonce, int outNonceSize);

// UNIX Timestamp 생성 (UTC 기준)
// @param outTimestamp 출력 버퍼
// @param outTimestampSize 출력 버퍼 크기
void PL_GetTimestamp(char* outTimestamp, int outTimestampSize);

// ============================================================================
// API 호출 함수
// ============================================================================

// 간편결제 정보 조회
// POST /v1/payment/simple/getpaymentinfo_V2
// @param reqType 요청타입 (1=회선번호, 2=상품코드, 3=종목알파고용)
// @param reqTypeVal 요청데이터 (회선번호 또는 상품코드)
// @param phoneNo 휴대폰번호 (하이픈 제외)
// @param arsType ARS구분 (VARS=보는ARS, ARS=듣는ARS)
// @param outInfo 결과 저장 구조체
// @return 성공 여부 (1=성공, 0=실패)
int PL_GetPaymentInfo(int reqType, const char* reqTypeVal,
                      const char* phoneNo, const char* arsType,
                      PL_PaymentInfo* outInfo);

// 결제 동의 등록
// POST /v1/payment/simple/agree
// @param phoneNo 휴대폰번호 (하이픈 제외)
// @param pgCode PG 코드 (A=올앳, P=페이레터)
// @param agreeFlag 동의 여부 (Y=동의)
// @return 성공 여부 (1=성공, 0=실패)
int PL_RegisterAgree(const char* phoneNo, const char* pgCode, const char* agreeFlag);

// ============================================================================
// HTTP 통신 함수
// ============================================================================

// HTTPS POST 요청
// @param endpoint API 엔드포인트 (예: "/v1/payment/simple/getpaymentinfo_V2")
// @param jsonBody JSON 요청 본문
// @param outResponse HTTP 응답 본문 버퍼
// @param outResponseSize 응답 버퍼 크기
// @param outStatusCode HTTP 상태 코드 출력
// @return 성공 여부 (1=성공, 0=실패)
int PL_HttpPost(const char* endpoint, const char* jsonBody,
                char* outResponse, int outResponseSize, int* outStatusCode);

// ============================================================================
// JSON 파싱 함수
// ============================================================================

// JSON 응답에서 문자열 값 추출
// @param json JSON 문자열
// @param key 키 이름
// @param outValue 출력 버퍼
// @param outValueSize 출력 버퍼 크기
// @return 성공 여부 (1=성공, 0=실패)
int PL_JsonGetString(const char* json, const char* key, char* outValue, int outValueSize);

// JSON 응답에서 정수 값 추출
__int64 PL_JsonGetInt64(const char* json, const char* key);

// JSON 응답에서 int 값 추출
int PL_JsonGetInt(const char* json, const char* key);

// JSON 객체 생성
// @param reqType 요청타입
// @param reqTypeVal 요청데이터
// @param phoneNo 휴대폰번호
// @param arsType ARS구분
// @param outJson 출력 버퍼
// @param outJsonSize 출력 버퍼 크기
void PL_JsonBuildRequest(int reqType, const char* reqTypeVal,
                         const char* phoneNo, const char* arsType,
                         char* outJson, int outJsonSize);

// ============================================================================
// 에러 처리
// ============================================================================

// 마지막 에러 메시지 가져오기
// @param outError 출력 버퍼
// @param outErrorSize 출력 버퍼 크기
void PL_GetLastError(char* outError, int outErrorSize);

// 에러 설정
void PL_SetLastError(const char* error);

// purchaseLimitFlag 값에 대한 에러 메시지 반환
// @param flag 플래그 값 ("1", "2", ...)
// @param outMessage 출력 버퍼
// @param outMessageSize 출력 버퍼 크기
void PL_GetPurchaseLimitMessage(const char* flag, char* outMessage, int outMessageSize);

// ============================================================================
// 유틸리티 함수
// ============================================================================

// 문자열 대문자 변환
// @param str 입력 문자열
// @param outUpper 출력 버퍼
// @param outUpperSize 출력 버퍼 크기
void PL_ToUpper(const char* str, char* outUpper, int outUpperSize);

// 휴대폰번호 정규화 (하이픈 제거)
// @param phoneNo 입력 전화번호
// @param outNormalized 출력 버퍼
// @param outNormalizedSize 출력 버퍼 크기
void PL_NormalizePhoneNo(const char* phoneNo, char* outNormalized, int outNormalizedSize);

// 로깅 함수 (외부 printf 연결)
void PL_Log(const char* format, ...);

// UTF-8 → EUC-KR(CP949) 변환
// API 응답은 UTF-8이지만, Windows 콘솔/로그는 EUC-KR 사용
// @param utf8Str UTF-8 인코딩 문자열
// @param outEucKr 출력 버퍼 (EUC-KR)
// @param outEucKrSize 출력 버퍼 크기
// @return 성공 여부 (1=성공, 0=실패)
int PL_ConvertUtf8ToEucKr(const char* utf8Str, char* outEucKr, int outEucKrSize);

#endif // _PAYLETTER_API_H_

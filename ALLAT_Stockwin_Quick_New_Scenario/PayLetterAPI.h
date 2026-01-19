// PayLetterAPI.h - REST API 통신 모듈
// 페이레터 BOQV9 REST API 연동
// 작성일: 2026-01-19

#pragma once

#ifndef _PAYLETTER_API_H_
#define _PAYLETTER_API_H_

#include <string>
#include <vector>

// API 응답 구조체
struct PL_PaymentInfo {
    std::string memberId;           // 회원 ID (UUID)
    long long orderNo;              // 주문번호
    std::string nickName;           // 필명
    std::string itemName;           // 상품명
    std::string pgCode;             // PG 코드 (A=올앳, P=페이레터)
    std::string mallIdSimple;       // 상점 ID (간편)
    std::string mallIdGeneral;      // 상점 ID (일반)
    std::string categoryId_2nd;     // 중분류 카테고리ID (상품코드 대체)
    std::string merchantId;         // 가맹점 ID
    int payAmt;                     // 실제 결제 금액 (할인 적용 후)
    int purchaseAmt;                // 상품 원가 (할인 전)
    std::string couponUseFlag;      // 쿠폰 존재 여부 (Y/N)
    std::string couponName;         // 쿠폰 이름
    std::string bonusCashUseFlag;   // 보너스 캐시 존재 여부 (Y/N)
    int bonusCashUseAmt;            // 보너스 캐시 사용 금액
    std::string cardCompany;        // 카드사명
    std::string purchaseLimitFlag;  // 구매 제한 여부 (1=정상, 2=불량, 3=횟수초과, 4=시작전, 5=종료, 6=중지)
    std::string payAgreeFlag;       // 결제 동의 여부
    int memberState;                // 고객상태 (1=비회원, 2=유료회원, 3=기구매자)
    std::string serviceCheckFlag;   // 서비스 점검여부 (Y/N)
    std::string checkCompleteTime;  // 점검완료일시
    std::string notiUrlSimple;      // Notification URL (간편)
    std::string notiUrlGeneral;     // Notification URL (일반)
    std::string billKey;            // 정기결제 키
    std::string billPassword;       // 정기결제 비밀번호

    // API 응답 상태
    std::string resultCode;         // 결과코드 (0=성공)
    std::string resultMessage;      // 오류 메시지
    int errorCode;                  // 오류 코드

    // 생성자
    PL_PaymentInfo() : orderNo(0), payAmt(0), purchaseAmt(0),
        bonusCashUseAmt(0), memberState(0), errorCode(0) {}
};

// API 설정 구조체
struct PL_Config {
    std::string appId;              // APP_ID (32자)
    std::string appKey;             // APP_KEY (Base64)
    std::string baseUrl;            // API 기본 URL
    int timeout;                    // 타임아웃 (초)
    bool useLegacyFallback;         // 레거시 API 폴백 사용 여부

    PL_Config() : timeout(30), useLegacyFallback(true) {}
};

// 전역 설정
extern PL_Config g_plConfig;

// ============================================================================
// 초기화 및 정리
// ============================================================================

// API 초기화 (INI 파일에서 설정 로드)
// @param iniPath INI 파일 경로
// @param isLive true=운영, false=QA
// @return 성공 여부
bool PL_Initialize(const char* iniPath, bool isLive);

// API 정리 (리소스 해제)
void PL_Cleanup();

// ============================================================================
// 인증 관련 함수
// ============================================================================

// Authorization 헤더 생성
// RequestString = APP_ID + UpperCase(METHOD) + Timestamp + Nonce
// Signature = Base64(HMAC-SHA256(Base64Decode(APP_KEY), RequestString))
// @return "PLTOKEN {APP_ID}:{Signature}:{Nonce}:{Timestamp}"
std::string PL_GenerateAuthHeader();

// HMAC-SHA256 서명 생성
// @param key Base64 디코딩된 키 바이트 배열
// @param keyLen 키 길이
// @param data 서명 대상 문자열
// @param outSignature 출력 서명 (Base64 인코딩)
// @return 성공 여부
bool PL_GenerateHMACSHA256(const unsigned char* key, size_t keyLen,
                           const std::string& data, std::string& outSignature);

// Base64 인코딩/디코딩
std::string PL_Base64Encode(const unsigned char* data, size_t len);
std::vector<unsigned char> PL_Base64Decode(const std::string& encoded);

// Nonce (UUID) 생성
std::string PL_GenerateNonce();

// UNIX Timestamp 생성 (UTC 기준)
std::string PL_GetTimestamp();

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
// @return 성공 여부
bool PL_GetPaymentInfo(int reqType, const std::string& reqTypeVal,
                       const std::string& phoneNo, const std::string& arsType,
                       PL_PaymentInfo& outInfo);

// 결제 동의 등록
// POST /v1/payment/simple/agree
// @param phoneNo 휴대폰번호 (하이픈 제외)
// @param pgCode PG 코드 (A=올앳, P=페이레터)
// @param agreeFlag 동의 여부 (Y=동의)
// @return 성공 여부
bool PL_RegisterAgree(const std::string& phoneNo, const std::string& pgCode,
                      const std::string& agreeFlag);

// ============================================================================
// HTTP 통신 함수
// ============================================================================

// HTTPS POST 요청
// @param endpoint API 엔드포인트 (예: "/v1/payment/simple/getpaymentinfo_V2")
// @param jsonBody JSON 요청 본문
// @param outResponse HTTP 응답 본문
// @param outStatusCode HTTP 상태 코드
// @return 성공 여부
bool PL_HttpPost(const std::string& endpoint, const std::string& jsonBody,
                 std::string& outResponse, int& outStatusCode);

// ============================================================================
// JSON 파싱 함수
// ============================================================================

// JSON 응답에서 문자열 값 추출
std::string PL_JsonGetString(const std::string& json, const std::string& key);

// JSON 응답에서 정수 값 추출
long long PL_JsonGetInt64(const std::string& json, const std::string& key);

// JSON 응답에서 int 값 추출
int PL_JsonGetInt(const std::string& json, const std::string& key);

// JSON 객체 생성
std::string PL_JsonBuildRequest(int reqType, const std::string& reqTypeVal,
                                const std::string& phoneNo, const std::string& arsType);

// ============================================================================
// 에러 처리
// ============================================================================

// 마지막 에러 메시지 가져오기
std::string PL_GetLastError();

// 에러 설정
void PL_SetLastError(const std::string& error);

// purchaseLimitFlag 값에 대한 에러 메시지 반환
std::string PL_GetPurchaseLimitMessage(const std::string& flag);

// ============================================================================
// 유틸리티 함수
// ============================================================================

// 문자열 대문자 변환
std::string PL_ToUpper(const std::string& str);

// 휴대폰번호 정규화 (하이픈 제거)
std::string PL_NormalizePhoneNo(const std::string& phoneNo);

// 로깅 함수 (외부 printf 연결)
void PL_Log(const char* format, ...);

#endif // _PAYLETTER_API_H_

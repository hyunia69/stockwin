#ifndef _ALLATUTIL_H_
#define _ALLATUTIL_H_

#define UTIL_LANG      "C"
#define UTIL_VER       "1.0.0.6"
#define APPROVAL_URI   "/servlet/AllatPay/pay/approval.jsp"
#define SANCTION_URI   "/servlet/AllatPay/pay/sanction.jsp"
#define CANCEL_URI     "/servlet/AllatPay/pay/cancel.jsp"
#define CASHREG_URI    "/servlet/AllatPay/pay/cash_registry.jsp"
#define CASHAPP_URI    "/servlet/AllatPay/pay/cash_approval.jsp"
#define CASHCAN_URI    "/servlet/AllatPay/pay/cash_cancel.jsp"
#define ESCROWCHK_URI  "/servlet/AllatPay/pay/escrow_check.jsp"
// 2018.06.20
// 추가자:최영락
// 간편결제를 위해 정기 결제 추가
#define CERTREQ_URI    "/servlet/AllatPay/pay/fix.jsp"
#define CERTCANCEL_URI "/servlet/AllatPay/pay/fix_cancel.jsp"
// 추가 끝
#define ALLAT_ADDR     "tx.mobilians.co.kr"
#define ALLAT_HOST     "tx.mobilians.co.kr"
#define ALLAT_ENCDATA_LEN 16383
#define ALLAT_DATA_LEN    8194
typedef char ALLAT_ENCDATA[ALLAT_ENCDATA_LEN + 1];
typedef char ALLAT_DATA[ALLAT_DATA_LEN + 1];

int _check_enc(const char *sMsg);
int ApprovalReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
int SanctionReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
// 2018.06.20
// 추가자:최영락
// 간편결제를 위해 정기 결제 추가
int CertRegReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
int CertCancelReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
// 추가 끝
int CancelReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
int CashRegReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
int CashAppReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
int CashCanReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
int EscrowChkReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt);
int SendRepo(ALLAT_DATA sAt_data, const char *sAt_addr, const char *sAt_uri, const char *sAt_host, int iAt_Port, ALLAT_DATA sRetTmp);
int SendReq(ALLAT_DATA sAt_data, const char *aAt_addr, const char *sAt_uri, const char *sAt_host, int iAt_Port, ALLAT_DATA sRetTmp);
int SendReqSSL(ALLAT_DATA sAt_data, const char *aAt_addr, const char *sAt_uri, const char *sAt_host, int iAt_Port, ALLAT_DATA sRetTmp);
void escapeString(char *dst, char *src, int srclen);
char* _ltrim(char *szInput);
char* _rtrim(char *szInput);
char* Allat_trim(char *szInput);
void initEnc(ALLAT_ENCDATA atEnc);
int validEnc(ALLAT_ENCDATA atEnc);
int setValue(ALLAT_ENCDATA atEnc, const char *sKey, const char *sValue);
void getValue(const char* sKey, const char* sMsg, char *sRet, int nLen);
// 2015.12.16
// 올앳 페이먼트의 연계 모듈의 구조적 문제로 메모리 누수 현상 발견
// 해결하기 위한 코드 개발
// 최영락
void SSL_init();
void SSL_free_comp_methods();

#endif

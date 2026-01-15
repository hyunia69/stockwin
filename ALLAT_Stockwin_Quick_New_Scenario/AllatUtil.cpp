
#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>


// 2015.12.16
// 올앳 페이먼트의 연계 모듈의 구저적 문제로 메모리 누수 현상 발견
// 해결하기 위한 코드 개발
// 최영락
#pragma comment(lib, "crypt32.lib")
#include "Wincrypt.h"


#define ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

#include <openssl/ssl.h>

#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/conf.h>

#include <signal.h>
#include <sys/types.h>
#include "AllatUtil.h"

#ifndef WIN32
//LINUX 및 UNIX의 소켓을 위한 헤더들
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#else 
#include <WinSock.h>
#include <time.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#define	PARAINI		".\\AllatPay_work_para.ini"

#include "AllatUtil.h"


#define UTIL_LANG      "C"
#define UTIL_VER       "1.0.0.6"
#define ALLAT_ADDR     "tx.mobilians.co.kr"
#define ALLAT_HOST     "tx.mobilians.co.kr"
#define APPROVAL_URI   "/servlet/AllatPay/pay/approval.jsp"
#define SANCTION_URI   "/servlet/AllatPay/pay/sanction.jsp"
#define CANCEL_URI     "/servlet/AllatPay/pay/cancel.jsp"
#define CASHREG_URI    "/servlet/AllatPay/pay/cash_registry.jsp"
#define CASHAPP_URI    "/servlet/AllatPay/pay/cash_approval.jsp"
#define CASHCAN_URI    "/servlet/AllatPay/pay/cash_cancel.jsp"
#define ESCROWCHK_URI  "/servlet/AllatPay/pay/escrow_check.jsp"

extern void(*xprintf)(const char *str, ...);


SSL_CTX *g_ctx = NULL;

// isalnum: 영어 대문자와 소문자, 숫자를 판별하는 함수입니다.
int myisalnum(int c)
{
	return ((0x60 < c && 0x7b > c) || (0x40 < c && 0x5b > c) || (0x2f < c && 0x3a > c)) ? 1 : 0;
}

// isalpha: 영어 대문자와 소문자를 판별하는 함수입니다.
int myisalpha(int c)
{
	return ((0x60 < c && 0x7b > c) || (0x40 < c && 0x5b > c)) ? 1 : 0;
}

// isascii: ASCII 영역 내의 문자인지 판별하는 함수입니다.
int myisascii(int c)
{
	return (-0x01 < c && 0x80 > c) ? 1 : 0;
}

// isbinary: 숫자 0 또는 1에 해당하는 문자인지 판별하는 함수입니다. (제가 추가한 함수입니다.)
int myisbinary(int c)
{
	return (c == 0x30 || c == 0x31) ? 1 : 0;
}

// iscntrl: 제어 문자를 판별하는 함수입니다.
int myiscntrl(int c)
{
	return ((-0x01 < c && 0x20 > c) || 0x7f == c) ? 1 : 0;
}

// isdigit: 10진수 숫자 (0 ~ 9)에 해당하는 문자인지 판별하는 함수입니다.
int myisdigit(int c)
{
	return (0x2f < c && 0x3a > c) ? 1 : 0;
}

// isgraph: 인쇄 문자 (공백 문자 포함) 인지 판별합니다.
int myisgraph(int c)
{
	return ((-0x01 < c && 0x20 > c) || 0x7f == c) ? 0 : 1;
}

// islower: 영어 소문자인지 판별합니다.
int myislower(int c)
{
	return (0x60 < c && 0x7b > c) ? 1 : 0;
}

// ismbcs: 2바이트 이상의 문자인지 판별합니다. (제가 추가한 함수입니다.)
int myismbcs(int c)
{
	return ((0x80 < c && 0xa0 > c) || (0xdf < c && 0xfd > c)) ? 0 : 1;
}

// isodigit: 8진수 숫자 (0 ~ 8)에 해당하는 문자인지 판별합니다. (제가 추가한 함수입니다.)
int myisodigit(int c)
{
	return (0x2f < c && 0x38 > c) ? 1 : 0;
}

// isprint: 인쇄 가능 문자 (실제로 보이며, 읽을 수 있는 문자만 해당) 인지 판별합니다.
int myisprint(int c)
{
	return ((-0x01 < c && 0x20 >= c) || 0x7f == c || (0x08 < c && 0x0e > c)) ? 0 : 1;
}

// ispunct: 영숫자, 공백을 제외한 기호 문자인지 판별합니다.
int myispunct(int c)
{
	return ((0x1f < c && 0x30 > c) || (0x39 < c && 0x41 > c) || (0x5a < c && 0x61 > c) || (0x7a < c && 0x7f > c)) ? 1 : 0;
}

// issbcs: 1바이트 이내의 문자인지 판별합니다. (제가 추가한 함수입니다.)
int myissbcs(int c)
{
	return ((0x80 < c && 0xa0 > c) || (0xdf < c && 0xfd > c)) ? 1 : 0;
}

// isspace: 공백 문자인지 판별합니다.
int myisspace(int c)
{
	return ((0x08 < c && 0x0e > c) || c == 0x20) ? 1 : 0;
}

// isupper: 영어 대문자인지 판별합니다.
int myisupper(int c)
{
	return (0x40 < c && 0x5b > c) ? 1 : 0;
}
// isxdigit: 16진수 숫자 (0 ~ 9, A ~ E)에 해당하는 문자인지 판별합니다.
int myisxdigit(int c)
{
	return ((0x2f < c && 0x3a > c) || (0x40 < c && 0x47 > c) || (0x60 < c && 0x67 > c)) ? 1 : 0;
}
// 위 함수들은 기본적으로 제공 되는 함수 이나..
// 디버그 모드에서 쓸모 없는 코드영역 검증하는 _ASSERT로 인해 한글
// 데이터에 대해서 디버그 모드에서 오류 창이 띄워진다.
// 따라서 평이성을 위해 위릐 함수로 대치 한다.


void getValue(const char* sKey, const char* sMsg, char *sRet, int nLen) {
	char *p;
	int i, j = 0, f = 0;

	p = strstr((char *)sMsg, sKey);
	if (!p) {
		sRet[0] = 0;
		return;
	}
	p += strlen(sKey);

	for (i = 0; i<nLen - 1; i++) {
		if (p[i] == '\r' || p[i] == '\n' || p[i] == 0) break;
		if (!myisspace(p[i])) f = 1;
		if (f) sRet[j++] = p[i];
	}
	for (i = i - 1; i >= 0; i--) if (!myisspace(sRet[i])) break;
	sRet[i + 1] = 0;
}

int _check_enc(const char *sMsg){
	char sSrc[4096 + 1];
	char *sBegin;

	memset(sSrc, 0, sizeof(sSrc));

	if (sMsg == NULL){
		return -1;
	}
	strcpy_s(sSrc, sizeof(sSrc), sMsg);

	sBegin = strstr(sSrc, "allat_enc_data=");

	if (sBegin == NULL){
		return -1;
	}

	if (strncmp(sBegin + 15 + 5, "1", 1) == 0){
		return 0;
	}
	else{
		return 1;
	}
}


int ApprovalReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt){

	int  nChkEnc = 0;
	int  nRet = 0;

	/*memset(sRetTxt, 0, sizeof(sRetTxt));*/

	char szALLAT_ADDR[512 + 1] = { 0x00, };
	char szALLAT_HOST[512 + 1] = { 0x00, };
	char szAPPROVAL_URI[512 + 1] = { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_ADDR", ALLAT_ADDR, (char *)szALLAT_ADDR, sizeof(szALLAT_ADDR), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_HOST", ALLAT_HOST, (char *)szALLAT_HOST, sizeof(szALLAT_HOST), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "APPROVAL_URI", APPROVAL_URI, (char *)szAPPROVAL_URI, sizeof(szAPPROVAL_URI), PARAINI);

	if (strcmp(sSsl_flag, "SSL") == 0){
		nRet = SendRepo(sAt_data, szALLAT_ADDR, szAPPROVAL_URI, szALLAT_HOST, 443, sRetTxt);
		if (nRet != 0){
			return nRet;
		}
	}
	else{
		nChkEnc = _check_enc(sAt_data);
		if (nChkEnc == 0){      /** 암호화 됨 **/
			nRet = SendRepo(sAt_data, szALLAT_ADDR, szAPPROVAL_URI, szALLAT_HOST, 80, sRetTxt);
			if (nRet != 0){
				return nRet;
			}
		}
		else if (nChkEnc > 0){ /** 암호화 되지 않음  **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0230\nreply_msg=암호화오류\n");
		}
		else{                   /** 에러 **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0231\nreply_msg=암호화 체크 오류\n");
		}
	}
	return 0;
}

int SanctionReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt){

	int  nChkEnc = 0;
	int  nRet = 0;

	/*memset(sRetTxt, 0, sizeof(sRetTxt));*/

	char szALLAT_ADDR[512 + 1] = { 0x00, };
	char szALLAT_HOST[512 + 1] = { 0x00, };
	char szSANCTION_URI[512 + 1] = { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_ADDR", ALLAT_ADDR, (char *)szALLAT_ADDR, sizeof(szALLAT_ADDR), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_HOST", ALLAT_HOST, (char *)szALLAT_HOST, sizeof(szALLAT_HOST), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "SANCTION_URI", SANCTION_URI, (char *)szSANCTION_URI, sizeof(szSANCTION_URI), PARAINI);

	if (strcmp(sSsl_flag, "SSL") == 0){
		nRet = SendRepo(sAt_data, szALLAT_ADDR, szSANCTION_URI, szALLAT_HOST, 443, sRetTxt);
		if (nRet != 0){
			return nRet;
		}
	}
	else{
		nChkEnc = _check_enc(sAt_data);
		if (nChkEnc == 0){      /** 암호화 됨 **/
			nRet = SendRepo(sAt_data, szALLAT_ADDR, szSANCTION_URI, szALLAT_HOST, 80, sRetTxt);
			if (nRet != 0){
				return nRet;
			}
		}
		else if (nChkEnc > 0){ /** 암호화 되지 않음  **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0230\nreply_msg=암호화오류\n");
		}
		else{                   /** 에러 **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0231\nreply_msg=암호화 체크 오류\n");
		}
	}
	return 0;

}

int CancelReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt){

	int  nChkEnc = 0;
	int  nRet = 0;

	/*memset(sRetTxt, 0, sizeof(sRetTxt));*/

	char szALLAT_ADDR[512 + 1] = { 0x00, };
	char szALLAT_HOST[512 + 1] = { 0x00, };
	char szCANCEL_URI[512 + 1] = { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_ADDR", ALLAT_ADDR, (char *)szALLAT_ADDR, sizeof(szALLAT_ADDR), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_HOST", ALLAT_HOST, (char *)szALLAT_HOST, sizeof(szALLAT_HOST), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "CANCEL_URI", CANCEL_URI, (char *)szCANCEL_URI, sizeof(szCANCEL_URI), PARAINI);


	if (strcmp(sSsl_flag, "SSL") == 0){
		nRet = SendRepo(sAt_data, szALLAT_ADDR, szCANCEL_URI, szALLAT_HOST, 443, sRetTxt);
		if (nRet != 0){
			return nRet;
		}
	}
	else{
		nChkEnc = _check_enc(sAt_data);
		if (nChkEnc == 0){      /** 암호화 됨 **/
			nRet = SendRepo(sAt_data, szALLAT_ADDR, szCANCEL_URI, szALLAT_HOST, 80, sRetTxt);
			if (nRet != 0){
				return nRet;
			}
		}
		else if (nChkEnc > 0){ /** 암호화 되지 않음  **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0230\nreply_msg=암호화오류\n");
		}
		else{                   /** 에러 **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0231\nreply_msg=암호화 체크 오류\n");
		}
	}
	return 0;

}

int CashRegReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt){

	int  nChkEnc = 0;
	int  nRet = 0;

	/*memset(sRetTxt, 0, sizeof(sRetTxt));*/

	char szALLAT_ADDR[512 + 1] = { 0x00, };
	char szALLAT_HOST[512 + 1] = { 0x00, };
	char szCASHREG_URI[512 + 1] = { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_ADDR", ALLAT_ADDR, (char *)szALLAT_ADDR, sizeof(szALLAT_ADDR), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_HOST", ALLAT_HOST, (char *)szALLAT_HOST, sizeof(szALLAT_HOST), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "CASHREG_URI", szCASHREG_URI, (char *)szCASHREG_URI, sizeof(szCASHREG_URI), PARAINI);


	if (strcmp(sSsl_flag, "SSL") == 0){
		nRet = SendRepo(sAt_data, szALLAT_ADDR, szCASHREG_URI, szALLAT_HOST, 443, sRetTxt);
		if (nRet != 0){
			return nRet;
		}
	}
	else{
		nChkEnc = _check_enc(sAt_data);
		if (nChkEnc == 0){      /** 암호화 됨 **/
			nRet = SendRepo(sAt_data, szALLAT_ADDR, szCASHREG_URI, szALLAT_HOST, 80, sRetTxt);
			if (nRet != 0){
				return nRet;
			}
		}
		else if (nChkEnc > 0){ /** 암호화 되지 않음  **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0230\nreply_msg=암호화오류\n");
		}
		else{                   /** 에러 **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0231\nreply_msg=암호화 체크 오류\n");
		}
	}
	return 0;

}

int CashAppReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt){

	int  nChkEnc = 0;
	int  nRet = 0;

	/*memset(sRetTxt, 0, sizeof(sRetTxt));*/

	char szALLAT_ADDR[512 + 1] = { 0x00, };
	char szALLAT_HOST[512 + 1] = { 0x00, };
	char szCASHAPP_URI[512 + 1] = { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_ADDR", ALLAT_ADDR, (char *)szALLAT_ADDR, sizeof(szALLAT_ADDR), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_HOST", ALLAT_HOST, (char *)szALLAT_HOST, sizeof(szALLAT_HOST), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "CASHAPP_URI", CASHAPP_URI, (char *)szCASHAPP_URI, sizeof(szCASHAPP_URI), PARAINI);

	if (strcmp(sSsl_flag, "SSL") == 0){
		nRet = SendRepo(sAt_data, szALLAT_ADDR, szCASHAPP_URI, szALLAT_HOST, 443, sRetTxt);
		if (nRet != 0){
			return nRet;
		}
	}
	else{
		nChkEnc = _check_enc(sAt_data);
		if (nChkEnc == 0){      /** 암호화 됨 **/
			nRet = SendRepo(sAt_data, szALLAT_ADDR, szCASHAPP_URI, szALLAT_HOST, 80, sRetTxt);
			if (nRet != 0){
				return nRet;
			}
		}
		else if (nChkEnc > 0){ /** 암호화 되지 않음  **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0230\nreply_msg=암호화오류\n");
		}
		else{                   /** 에러 **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0231\nreply_msg=암호화 체크 오류\n");
		}
	}
	return 0;

}

int CashCanReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt){

	int  nChkEnc = 0;
	int  nRet = 0;

	/*memset(sRetTxt, 0, sizeof(sRetTxt));*/

	char szALLAT_ADDR[512 + 1] = { 0x00, };
	char szALLAT_HOST[512 + 1] = { 0x00, };
	char szCASHCAN_URI[512 + 1] = { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_ADDR", ALLAT_ADDR, (char *)szALLAT_ADDR, sizeof(szALLAT_ADDR), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_HOST", ALLAT_HOST, (char *)szALLAT_HOST, sizeof(szALLAT_HOST), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "CASHCAN_URI", CASHCAN_URI, (char *)szCASHCAN_URI, sizeof(szCASHCAN_URI), PARAINI);


	if (strcmp(sSsl_flag, "SSL") == 0){
		nRet = SendRepo(sAt_data, szALLAT_ADDR, szCASHCAN_URI, szALLAT_HOST, 443, sRetTxt);
		if (nRet != 0){
			return nRet;
		}
	}
	else{
		nChkEnc = _check_enc(sAt_data);
		if (nChkEnc == 0){      /** 암호화 됨 **/
			nRet = SendRepo(sAt_data, szALLAT_ADDR, szCASHCAN_URI, szALLAT_HOST, 80, sRetTxt);
			if (nRet != 0){
				return nRet;
			}
		}
		else if (nChkEnc > 0){ /** 암호화 되지 않음  **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0230\nreply_msg=암호화오류\n");
		}
		else{                   /** 에러 **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0231\nreply_msg=암호화 체크 오류\n");
		}
	}
	return 0;

}

int EscrowChkReq(ALLAT_DATA sAt_data, const char *sSsl_flag, ALLAT_DATA sRetTxt){

	int  nChkEnc = 0;
	int  nRet = 0;

	/*memset(sRetTxt, 0, sizeof(sRetTxt));*/

	char szALLAT_ADDR[512 + 1] = { 0x00, };
	char szALLAT_HOST[512 + 1] = { 0x00, };
	char szESCROWCHK_URI[512 + 1] = { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_ADDR", ALLAT_ADDR, (char *)szALLAT_ADDR, sizeof(szALLAT_ADDR), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_HOST", ALLAT_HOST, (char *)szALLAT_HOST, sizeof(szALLAT_HOST), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "ESCROWCHK_URI", ESCROWCHK_URI, (char *)szESCROWCHK_URI, sizeof(szESCROWCHK_URI), PARAINI);


	if (strcmp(sSsl_flag, "SSL") == 0){
		nRet = SendRepo(sAt_data, szALLAT_ADDR, szESCROWCHK_URI, szALLAT_HOST, 443, sRetTxt);
		if (nRet != 0){
			return nRet;
		}
	}
	else{
		nChkEnc = _check_enc(sAt_data);
		if (nChkEnc == 0){      /** 암호화 됨 **/
			nRet = SendRepo(sAt_data, szALLAT_ADDR, szESCROWCHK_URI, szALLAT_HOST, 80, sRetTxt);
			if (nRet != 0){
				return nRet;
			}
		}
		else if (nChkEnc > 0){ /** 암호화 되지 않음  **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0230\nreply_msg=암호화오류\n");
		}
		else{                   /** 에러 **/
			strcpy_s(sRetTxt, sizeof(ALLAT_DATA), "reply_cd=0231\nreply_msg=암호화 체크 오류\n");
		}
	}
	return 0;

}

int SendRepo(ALLAT_DATA sAt_data, const char *sAt_addr, const char *sAt_uri, const char *sAt_host, int iAt_Port, ALLAT_DATA sRetTmp){

	int  nTmp = 0;
	int  nRePort = 443;
	char sReIp[15 + 1];
	char sRePort[5 + 1];
	char sRetCd[4 + 1];

	memset(sRetCd, 0, sizeof(sRetCd));
	if (iAt_Port == 80){
		nTmp = SendReq(sAt_data, sAt_addr, sAt_uri, sAt_host, iAt_Port, sRetTmp);
		printf("\n<br>80연결\n<br>\n");
	}
	else{
		nTmp = SendReqSSL(sAt_data, sAt_addr, sAt_uri, sAt_host, iAt_Port, sRetTmp);
		printf("\n<br>443연결\n<br>\n");
	}
	if (nTmp == 0){
		getValue("reply_cd=", sRetTmp, sRetCd, sizeof(sRetCd));
		if (strcmp(sRetCd, "0290") == 0){
			getValue("redirect_ip=", sRetTmp, sReIp, sizeof(sReIp));
			getValue("redirect_port=", sRetTmp, sRePort, sizeof(sRePort));
			nRePort = atoi(sRePort);
			if (iAt_Port == 80){
				nTmp = SendReq(sAt_data, sReIp, sAt_uri, sAt_host, nRePort, sRetTmp);
			}
			else{
				nTmp = SendReqSSL(sAt_data, sReIp, sAt_uri, sAt_host, nRePort, sRetTmp);
			}
			if (nTmp != 0){
				/*** 에러 처리 ***/
				/*** sRetTmp  ***/
				return -1;
			}
		}
	}
	else{
		/*** 에러 처리 ***/
		/*** sRetTmp  ***/
		return -1;
	}
	return 0;
}

void SSL_init()
{
	// 2015.12.16
	// 올앳 페이먼트의 연계 모듈의 구저적 문제로 메모리 누수 현상 발견
	// 해결하기 위한 코드 개발

	//libcrypto과의 libssl을 모두 초기화하려면 최소의 방법
	//SSL_load_error_strings();//libcrypto 에러 및 crypto만 초기화
	SSL_library_init();//libssl  초기화

	//libcrypto과의 libssl을 모두 초기화하려면 :
	//SSL_load_error_strings();//libcrypto 에러 및 crypto만 초기화
	//SSL_library_init();//libssl  초기화
	//OpenSSL_add_all_algorithms();
	//OPENSSL_config(NULL);//쓰레드에 대한 것
	//OPENSSL_no_config();

	////libcrypto만 초기화하려면 :
	//ERR_load_crypto_strings();
	//OpenSSL_add_all_algorithms();
	//OPENSSL_config(NULL); //쓰레드에 대한 것

	//libssl만 모두 초기화하려면 :
	//SSL_library_init();//libssl  초기화
	//OpenSSL_add_all_algorithms();
	//OPENSSL_config(NULL);ctx

	// AHN 20250605
	//if (!(g_ctx = SSL_CTX_new(SSLv3_method()))) goto _bad;
	if (!(g_ctx = SSL_CTX_new(SSLv23_client_method()))) goto _bad;
	SSL_CTX_set_options(g_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

	return;

_bad:
	if (g_ctx)
	{
		SSL_CTX_free(g_ctx);
		g_ctx = NULL;
	}
	return;
}

void SSL_free_comp_methods()
{
	if (g_ctx)
	{
		SSL_CTX_free(g_ctx);
		g_ctx = NULL;
	}

	/*CONF_modules_free();
	ENGINE_cleanup();
	CONF_modules_unload(1);
	ERR_remove_state(0);
	ERR_free_strings();
	ERR_remove_thread_state(NULL);*/

	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();

	SSL_COMP_free_compression_methods();
	COMP_zlib_cleanup();
}


int SendReqSSL(ALLAT_DATA sAt_data, const char *aAt_addr, const char *sAt_uri, const char *sAt_host, int iAt_Port, ALLAT_DATA sRetTmp)
{
	char sRequest[9095 + 1];
	char sReqData[8191 + 1]; /** Request Data  **/
	char sResData[8191 + 1]; /** Response Data **/

	/** Apply Time Check **/
	char sApplyYMDHMS[14 + 1];
	time_t ti;
	struct tm tp;

	/** Socket Connect **/
	int e;
	int fd = -1;
	int len;

	// 2015.12.16
	// 올앳 페이먼트의 연계 모듈의 구저적 문제로 메모리 누수 현상 발견
	// 해결하기 위한 코드 개발
	// 삭제 처리
	SSL_CTX *ctx = NULL;

	SSL *ssl = NULL;

	fd_set fds;
	struct timeval tv;

	struct sockaddr_in server;
	struct hostent *pstHe;

	char *ptr;
	int n, ncl, header, total;

	header = 0;
	memset(sApplyYMDHMS, 0, sizeof(sApplyYMDHMS));
	memset(sRequest, 0, sizeof(sRequest));
	memset(sReqData, 0, sizeof(sReqData));
	memset(sResData, 0, sizeof(sResData));


	ti = time(NULL);

	if (localtime_s(&tp, (const time_t*)&ti) != 0)
	{
		printf("SendReqSSL>Invalid argument to localtime_s.");
		return NULL;
	}

	sprintf_s(sApplyYMDHMS, sizeof(sApplyYMDHMS), "%04d%02d%02d%02d%02d%02d",
		tp.tm_year + 1900, tp.tm_mon + 1, tp.tm_mday, tp.tm_hour, tp.tm_min, tp.tm_sec);

	char szUTIL_LANG[1 + 1] = { 0x00, };
	char szUTIL_VER[20 + 1]  { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "UTIL_LANG", UTIL_LANG, (char *)szUTIL_LANG, sizeof(szUTIL_LANG), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "UTIL_VER", UTIL_VER, (char *)szUTIL_VER, sizeof(szUTIL_VER), PARAINI);

	sprintf_s(sReqData, sizeof(sReqData), "%s&allat_opt_lang=%s&allat_opt_ver=%s&allat_apply_ymdhms=%s",
		sAt_data, szUTIL_LANG, szUTIL_VER, sApplyYMDHMS);
	/************* Thread에 안전 하지 않습니다. **************************/
	/* 해당 위치를 옮기도록 한다.... 소켓 초기화 이후로 해야 한다.*/
	//pstHe = gethostbyname(sAt_host);		// AllatServer IP

	//if (!pstHe) goto _bad;

	//memset(&server, 0, sizeof(server));
	//server.sin_family = pstHe->h_addrtype;
	//server.sin_port = htons(iAt_Port);
	//server.sin_addr.s_addr = ((unsigned long *)(pstHe->h_addr_list[0]))[0];
	///********************************************************************/

	//len = sizeof(server);

#ifdef WIN32
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2)/*0x0101*/, &WsaData); //윈속 2.2

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd<0) goto _bad;

#else
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd<0) goto _bad;

	signal(SIGPIPE, SIG_IGN);
#endif

	/************* Thread에 안전 하지 않습니다. **************************/

	pstHe = gethostbyname(sAt_host);		// AllatServer IP

	if (!pstHe) goto _bad;

	struct in_addr addr;
	addr.s_addr = *(unsigned long *)pstHe->h_addr_list[0];

	memset(&server, 0, sizeof(server));
	server.sin_family = pstHe->h_addrtype;
	server.sin_port = htons(iAt_Port);
	server.sin_addr.s_addr = ((unsigned long *)(pstHe->h_addr_list[0]))[0];
	/********************************************************************/
	len = sizeof(server);

	if (connect(fd, (struct sockaddr *)&server, sizeof server) < 0) goto _bad;


	//SSL_library_init();
	//if (!(ctx = SSL_CTX_new(SSLv3_method()))) goto _bad;
	if (!(ssl = SSL_new(/*ctx*/g_ctx))) goto _bad;
	SSL_set_connect_state(ssl);
	if (SSL_set_fd(ssl, fd) == -1) goto _bad;
	if (SSL_connect(ssl) == -1) goto _bad;

	len = 1;

#ifdef WIN32
	ioctlsocket(fd, FIONBIO, (u_long *)&len);
#else
	ioctl(fd, FIONBIO, &len);
#endif



	len = sprintf_s(sRequest, sizeof(sRequest),				// Check Send Config
		"POST %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"Content-type: application/x-www-form-urlencoded\r\n"
		"Host: %s:%d\r\n"
		"Content-length: %d\r\n"
		"\r\n"
		"%s", sAt_uri, sAt_host, iAt_Port, strlen(sReqData), sReqData);
	printf("진짜로 보내는 전문은(%d) : %s", strlen(sRequest), sRequest);

	tv.tv_sec = 5;
	tv.tv_usec = 0;
	total = 0;

	while (total<len) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		if (select(fd + 1, NULL, &fds, NULL, &tv) <= 0) goto _bad; // interrupt 무시
		
		n = SSL_write(ssl, sRequest + total, len - total);
		
		if (n<0) {
			e = SSL_get_error(ssl, n);
			if (e != SSL_ERROR_WANT_READ && e != SSL_ERROR_WANT_WRITE) n = 0;
			else continue;
		}
		if (n == 0) goto _bad;
		total += n;
	}

	tv.tv_sec = 22;
	tv.tv_usec = 0;
	total = 0;
	ncl = 0;
	len = sizeof(sResData) - 1;

	while (total<len) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		if (select(fd + 1, &fds, NULL, NULL, &tv) <= 0) goto _bad; // interrupt 무시

		n = SSL_read(ssl, sResData + total, len - total);		///Server RESPONSE
		if (n<0) {
			if (SSL_get_error(ssl, n) == SSL_ERROR_WANT_READ) continue;
			else goto _bad;
		}
		if (n == 0) {
			if (!ncl) goto _bad;
			else break;
		}
		total += n;
		sResData[total] = 0;

		if (!header) {
			char *p;
			ptr = strstr(sResData, "\r\n\r\n");
			if (!ptr) {
				if (total >= len) goto _bad; // bad header length
				else continue;
			}
			header = 1;
			total = total - (ptr - sResData) - 4;

			p = strstr(sResData, "Content-Length:");
			if (!p) p = strstr(sResData, "Content-length:");
			if (p) {
				len = atoi(p + 16);
				if (len <= 0 || len>sizeof(sResData) - 1) goto _bad; // bad body length
			}
			else {
				ncl = 1;
				len = sizeof(sResData) - 1;
			}

			memmove(sResData, ptr + 4, total + 1);
		}
	}
	strcpy_s(sRetTmp, sizeof(ALLAT_DATA), sResData);
	SSL_shutdown(ssl);
	SSL_free(ssl);
	//SSL_CTX_free(ctx);

	ERR_remove_state(0);
	ERR_free_strings();
	ERR_remove_thread_state(NULL);

	// 2015.12.16
	// 올앳 페이먼트의 연계 모듈의 구저적 문제로 메모리 누수 현상 발견
	// 해결하기 위한 코드 개발
	// 삭제 처리
	//while (SSL_shutdown(ssl) == 0); 
	//if (ssl) SSL_free(ssl);
	// SSL_CTX_free(ctx);

#ifdef WIN32
	closesocket(fd);
#else
	close(fd);
#endif

#ifdef WIN32
	WSACleanup();
#else

#endif

	return 0;
_bad:
	if (ssl) SSL_free(ssl);
	// 2015.12.16
	// 올앳 페이먼트의 연계 모듈의 구저적 문제로 메모리 누수 현상 발견
	// 해결하기 위한 코드 개발
	// 삭제 처리
	//if (ctx) SSL_CTX_free(ctx);

	ERR_remove_state(0);
	ERR_free_strings();
	ERR_remove_thread_state(NULL);

#ifdef WIN32
	if (fd != -1) closesocket(fd);
#else
	if (fd != -1) close(fd);
#endif
	strcpy_s(sRetTmp, sizeof(ALLAT_DATA), "reply_cd=0261\nreply_msg=SSL Socket 통신 오류");

#ifdef WIN32
	WSACleanup();
#else

#endif



	return -1;
}

int SendReq(ALLAT_DATA sAt_data, const char *aAt_addr, const char *sAt_uri, const char *sAt_host, int iAt_Port, ALLAT_DATA sRetTmp){
	char sRequest[9095 + 1];
	char sReqData[8191 + 1]; /** Request Data  **/
	char sResData[8191 + 1]; /** Response Data **/

	/** Apply Time Check **/
	char sApplyYMDHMS[14 + 1];
	time_t ti;
	struct tm tp;

	/** Socket Connect **/
	int fd = -1;
	int len;

	fd_set fds;
	struct timeval tv;

	struct sockaddr_in server;
	struct hostent *pstHe;

	char *ptr;
	int n, ncl, header, total;

	header = 0;
	memset(sApplyYMDHMS, 0, sizeof(sApplyYMDHMS));
	memset(sRequest, 0, sizeof(sRequest));
	memset(sReqData, 0, sizeof(sReqData));
	memset(sResData, 0, sizeof(sResData));

	ti = time(NULL);
	if (localtime_s(&tp, (const time_t*)&ti) != 0)
	{
		printf("SendReq>Invalid argument to localtime_s.");
		return NULL;
	}

	sprintf_s(sApplyYMDHMS, sizeof(sApplyYMDHMS), "%04d%02d%02d%02d%02d%02d",
		tp.tm_year + 1900, tp.tm_mon + 1, tp.tm_mday, tp.tm_hour, tp.tm_min, tp.tm_sec);

	char szUTIL_LANG[1 + 1] = { 0x00, };
	char szUTIL_VER[20 + 1]  { 0x00, };

	GetPrivateProfileString("ALLAT_PAYMEMT", "UTIL_LANG", UTIL_LANG, (char *)szUTIL_LANG, sizeof(szUTIL_LANG), PARAINI);
	GetPrivateProfileString("ALLAT_PAYMEMT", "UTIL_VER", UTIL_VER, (char *)szUTIL_VER, sizeof(szUTIL_VER), PARAINI);

	sprintf_s(sReqData, sizeof(sReqData), "%s&allat_opt_lang=%s&allat_opt_ver=%s&allat_apply_ymdhms=%s",
		sAt_data, szUTIL_LANG, szUTIL_VER, sApplyYMDHMS);
	/************* Thread에 안전 하지 않습니다. **************************/
	/* 소켓 초기화 이후에 해야 한다.*/
	//pstHe = gethostbyname(sAt_host);		// AllatServer IP

	//if (!pstHe) goto _bad;

	//memset(&server, 0, sizeof(server));
	//server.sin_family = pstHe->h_addrtype;
	//server.sin_port = htons(iAt_Port);
	//server.sin_addr.s_addr = ((unsigned long *)(pstHe->h_addr_list[0]))[0];
	/********************************************************************/
	len = sizeof(server);

#ifdef WIN32
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2)/*0x0101*/, &WsaData); //윈속 2.2

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd<0) goto _bad;

#else
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd<0) goto _bad;

	signal(SIGPIPE, SIG_IGN);
#endif

	/************* Thread에 안전 하지 않습니다. **************************/
	pstHe = gethostbyname(sAt_host);		// AllatServer IP

	if (!pstHe) goto _bad;

	memset(&server, 0, sizeof(server));
	server.sin_family = pstHe->h_addrtype;
	server.sin_port = htons(iAt_Port);
	server.sin_addr.s_addr = ((unsigned long *)(pstHe->h_addr_list[0]))[0];
	/********************************************************************/

	if (connect(fd, (struct sockaddr *)&server, sizeof server) < 0) goto _bad;

	len = 1;

#ifdef WIN32
	ioctlsocket(fd, FIONBIO, (u_long *)&len);
#else
	ioctl(fd, FIONBIO, &len);
#endif

	len = sprintf_s(sRequest, sizeof(sRequest),				// Check Send Config
		"POST %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"Content-type: application/x-www-form-urlencoded\r\n"
		"Host: %s:%d\r\n"
		"Content-length: %d\r\n"
		"\r\n"
		"%ss", sAt_uri, sAt_host, iAt_Port, strlen(sReqData), sReqData);

	tv.tv_sec = 5;
	tv.tv_usec = 0;
	total = 0;
	while (total<len) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		if (select(fd + 1, NULL, &fds, NULL, &tv) <= 0) goto _bad; // interrupt 무시

#ifdef WIN32
		n = send(fd, sRequest + total, len - total, 0);		///SEND REQUEST
#else
		n = write(fd, sRequest + total, len - total);		///SEND REQUEST
#endif
		if (n<0) {
			if (errno != EINTR && errno != EAGAIN) goto _bad;
			else continue;
		}
		if (n == 0) goto _bad;
		total += n;
	}

	tv.tv_sec = 30;
	tv.tv_usec = 0;
	total = 0;
	ncl = 0;
	len = sizeof(sResData) - 1;

	while (total<len) {
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		if (select(fd + 1, &fds, NULL, NULL, &tv) <= 0) goto _bad; // interrupt 무시

#ifdef WIN32
		n = recv(fd, sResData + total, len - total, 0);		///Server RESPONSE
#else
		n = read(fd, sResData + total, len - total);		///Server RESPONSE
#endif
		if (n<0) {
			if (errno != EINTR && errno != EAGAIN) goto _bad;
			else continue;
		}
		if (n == 0) {
			if (!ncl) goto _bad;
			else break;
		}
		total += n;
		sResData[total] = 0;

		if (!header) {
			char *p;
			ptr = strstr(sResData, "\r\n\r\n");
			if (!ptr) {
				if (total >= len) goto _bad; // bad header length
				else continue;
			}
			header = 1;
			total = total - (ptr - sResData) - 4;

			p = strstr(sResData, "Content-Length:");
			if (!p) p = strstr(sResData, "Content-length:");
			if (p) {
				len = atoi(p + 16);
				if (len <= 0 || len>sizeof(sResData) - 1) goto _bad; // bad body length
			}
			else {
				ncl = 1;
				len = sizeof(sResData) - 1;
			}

			memmove(sResData, ptr + 4, total + 1);
		}
	}
	printf("sResData---->: %s\n", sResData);
	strcpy_s(sRetTmp, sizeof(ALLAT_DATA), sResData);
#ifdef WIN32
	closesocket(fd);
#else
	close(fd);
#endif

#ifdef WIN32
	WSACleanup();
#else

#endif

	return 0;
_bad:

#ifdef WIN32
	if (fd != -1) closesocket(fd);
#else
	if (fd != -1) close(fd);
#endif
	strcpy_s(sRetTmp, sizeof(ALLAT_DATA), "reply_cd=0271\nreply_msg=Socket 통신 오류");

#ifdef WIN32
	WSACleanup();
#else

#endif

	return -1;
}


void escapeString(char *dst, char *src, int srclen) {
	int i, p;
	//int srclen = 0;

	if (src == NULL){
		dst[0] = 0;
		return;
	}
	//srclen = strlen(src);

	p = 0;
	for (i = 0; i<srclen; i++) {


		if (!myisalnum(src[i]) && src[i] != '_' && src[i] != '-') {
			sprintf(dst + p, "%%%02X", (unsigned char)src[i]);
			p += 3;
		}
		else {
			dst[p++] = src[i];
		}
	}
	dst[p] = 0;
}

char* _ltrim(char *szInput) {
	int nSize;
	if (!szInput) return szInput;

	nSize = strspn(szInput, " \f\n\r\t\v");
	if (nSize) memmove(szInput, szInput + nSize, strlen(szInput) - nSize + 1);

	return szInput;
}

char* _rtrim(char *szInput) {
	int i;
	if (!szInput) return szInput;

	for (i = strlen(szInput) - 1; i >= 0; i--) {
		if (!myisspace(szInput[i])) break;
	}
	szInput[i + 1] = 0;
	return szInput;
}

char* Allat_trim(char *szInput) {
	return _rtrim(_ltrim(szInput));
}

void initEnc(ALLAT_ENCDATA atEnc){
	memset(atEnc, 0, ALLAT_ENCDATA_LEN + 1);
	strcpy_s(atEnc, sizeof(ALLAT_ENCDATA), "00000010");
}

int validEnc(ALLAT_ENCDATA atEnc){
	int i, nvl, cx = 0;

	if (!atEnc) return 0;
	nvl = strlen(atEnc);

	if (nvl == 0) return 1;
	if (nvl == 8 && (strcmp(atEnc, "00000010") == 0)) return 1;
	if (nvl>ALLAT_ENCDATA_LEN || atEnc[nvl - 1] != 24) return 0;

	for (i = 0; i<nvl; i++) {
		if (atEnc[i] == 3) {
			if (cx) return 0;
			cx = !cx;
		}
		else if (atEnc[i] == 24) {
			if (!cx) return 0;
			cx = !cx;
		}
	}
	return 1;
}

/******************************
return
-3: 잘못된 VL (InitVL 필요)
-2: 파라미터 오류
-1: 실패
0: 성공
1: 중복(over write)
*******************************/
int setValue(ALLAT_ENCDATA atEnc, const char *sKey, const char *sValue){
	int nFind = 0;
	int nPos = 0;
	int novalue = 0;

	int nvl, nKey, nValue;

	char *p, *pp;

	if (!validEnc(atEnc)) return -3;
	if (!sKey || !sValue) return -2;
	if (strchr(sKey, 3) || strchr(sKey, 24) || strchr(sValue, 3) || strchr(sValue, 24)) return -2;

	nvl = strlen(atEnc);
	nKey = strlen(sKey);
	nValue = strlen(sValue);

	pp = atEnc;
	for (;;) {
		p = strstr(pp, sKey);
		if (!p) break;

		if (p[nKey] == 3) {
			if (p == atEnc || p[-1] == 24) {
				nFind = 1;
				nPos = p - atEnc;
				pp = p + nKey + 1;
				p = strchr(pp, 24);
				novalue = p - pp;
				break;
			}
		}
		pp = p + nKey;
	}

	if (nFind) {
		if (ALLAT_ENCDATA_LEN - nvl < nValue - novalue) return -1;
		memmove(atEnc + nPos, p + 1, strlen(p + 1) + 1);
	}
	else {
		if (ALLAT_ENCDATA_LEN - nvl < nKey + nValue + 2) return -1;
	}

	strcat_s(atEnc, sizeof(ALLAT_ENCDATA), sKey);
	strcat_s(atEnc, sizeof(ALLAT_ENCDATA), "\003");
	strcat_s(atEnc, sizeof(ALLAT_ENCDATA), sValue);
	strcat_s(atEnc, sizeof(ALLAT_ENCDATA), "\030");

	return nFind;
}

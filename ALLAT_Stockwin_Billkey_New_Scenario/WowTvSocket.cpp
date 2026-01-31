
// WowTvSocket.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "CommonDef.H"
#include "ALLATCommom.h"

#include    "WowTvSocket.h"
#include    "ADODB.h"
#include    "Scenaio.h"
#include    "AllatUtil.h"
#include    "ALLAT_Stockwin_Billkey_New_Scenario.h"



#pragma once
#include <afx.h>
#include <ATLComTime.h>
#include <malloc.h>
#include <vector>
#include<new>
#include <iostream>
#include <string>
using namespace std;

#include <ws2tcpip.h>
#include <stdlib.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#ifndef UNICODE
#include <atlconv.h> // USES_CONVERSION과 W2A를 사용하기 위해 include
std::string ConverseBstrToString(const _bstr_t& str) {
	USES_CONVERSION;
	return std::string(W2A(str));
}
#endif

#import <msxml6.dll>
using namespace MSXML2;

extern void(*eprintf)(const char *str, ...);
extern void(*xprintf)(const char *str, ...);
extern LPMTP **lpmt, **port;

//LPMTP	*curyport=NULL;
extern void(*info_printf)(int chan, const char *str, ...);
extern void(*new_guide)(void);
extern int(*set_guide)(int vid, ...);
extern void(*setPostfunc)(int type, int(*func)(int), int poststate, int wtime);
extern int(*send_guide)(int mode);
extern int(*goto_hookon)(void);
extern int(*check_validform)(char *form, char *data);
extern int(*send_error)(void);
extern int(*check_validdtmf)(int c, char *vkeys);
extern int(*in_multifunc)(int chan);
extern int(*quitchan)(int chan);

extern int(*atoi2)(char *p);
extern int(*atoi3)(char *p);
extern int(*atoi4)(char *p);
extern int(*atoiN)(char *p, int n);
extern long(*atolN)(char *p, int n);
extern int(*atox)(char *s);

extern int Http_SSL_RetPageSend(void *data, char * szURL, char * szPostData);


CString GetErrorDescription(CException* ex)
{
	CString strExceptionDescription;
	TCHAR szException[1024] = { 0, };

	if (ex->GetErrorMessage(szException, 1024))
		strExceptionDescription = szException;

	return strExceptionDescription;
}

CString GetDetailErrorMessage(DWORD dwErrorCode, char *strErrorDescription, char *strFile, char * strFunc, DWORD dwLineNo)
{
	CString strErrorMessage;
	COleDateTime datetime = COleDateTime::GetCurrentTime();

	strErrorMessage.Format(
		_T("KSNET_Kungnam_Call_Scenario > GetDetailErrorMessage\n")
		_T("Date Time: %s\n")
		_T("ErrorCode: %d\n\n")
		_T("%s\n\n")
		_T("File: %s\n")
		_T("Function: %s"),
		_T("Line: %d"),
		datetime.Format(),
		dwErrorCode,
		strErrorDescription,
		strFile,
		strFunc,
		dwLineNo);

	return strErrorMessage;
}

void DetailErrorMessage(int ch, CException* ex, char *strFile, char *strFunc, DWORD dwLineNo)
{
	CString strErrorDescription = GetErrorDescription(ex);
	CString strDetailErrorMessage = GetDetailErrorMessage(GetLastError(), strErrorDescription.GetBuffer(), strFile, strFunc, dwLineNo);

	//::AfxMessageBox(strDetailErrorMessage, uMB_IconButton);
	if (ch>0) xprintf("[CH:%03d] %s", ch, strDetailErrorMessage);
	else xprintf("%s", strDetailErrorMessage);
}

#ifndef u_char
#define u_char	unsigned char
#define u_int	unsigned int
#define u_long	unsigned long
#endif

#define	TITLE_NAME	"ISDN PRI E1 - ARS"
#define	MAXSTRING	200
#define	MSG_SET_VIEW		WM_USER + 00
#define	MSG_INIT_LINE		WM_USER + 01
#define	MSG_SET_LINE		WM_USER + 02
#define	MSG_INBOUND_LINE	WM_USER + 03
#define	MSG_ASR_LINE	    WM_USER + 04

#define	PARAINI		".\\ALLAT_Stockwin_Billkey_New_Scenario.ini"
#define MAXCHAN 	240		// 최대 회선 수
//#define MAXCHAN 	120		// 최대 회선 수


#define TRUE		1		// 참
#define FALSE		0		// 거짓

#define HI_OK		0
#define HI_COMM 	98	// 통신 장애
#define HI_BADPKT	97	// BAD Packet

//////////////////////////////////////////////////////////////////////
#define VOC_MAIL_ID	500
#define VOC_MESG_ID	501
#define VOC_TEMP_ID	502
#define VOC_TTS_ID  503
#define VOC_WAVE_ID 504
#define VOC_MAIL	20		// 안내문
#define	VOC_MESG	21		// 사서함 메세지
#define VOC_TEMP	22		// Temp
#define VOC_TTS  	23		// TTS
#define VOC_WAVE  	24		// WAVE
///////////////////////////////////////////////////////////////////////

// Port 구분
#define	SERVER_PORT		(API_PORT) + 0


#define DATABASE_SESSION "DATABASE"
#define DATABASE_IP_URL "arsdb.mediaford.co.kr,1433"
#define DATABASE  "arspg_web"
#define SID       "sa"
#define PASSWORD  "medi@ford"

#define NICE_MAII_NM "옴니텔"
#define NICE_MAII_ID "T5102001"

#define CALLBACK_NO "0234904411"


#define  DEFAULT_ADDRESS "121.189.59.98,8096"

#define		DEFAULT_RECV_TIME	5
#define		DEFAULT_SEND_TIME	5

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

using namespace std;

std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

//////////////////////////////////////////////////////////////////////////////////////////
//																						//
//																						//
//																						//
//////////////////////////////////////////////////////////////////////////////////////////

int Wow_split(char *szSrc, const char *strDelimit, char **Dest, int Cnt)
{
	char* token;   // Token 된 문자열 포인터
	char* nexttok; // Token 이후 포인터
	char* pSrc;

	pSrc = (char*)malloc(strlen(szSrc) + 1);
	memset(pSrc, 0x00, strlen(szSrc) + 1);
	memcpy(pSrc, szSrc, strlen(szSrc));
	token = strtok_s(pSrc, strDelimit, &nexttok);

	if (token == NULL)
	{
		free(pSrc);
		return 0;
	}

	memcpy(Dest[0], token, strlen(token));

	int iRep = 0;
	for (iRep = 1; iRep < Cnt; iRep++)
	{
		token = strtok_s(NULL, strDelimit, &nexttok);
		if (token == NULL) break;
		memcpy(Dest[iRep], token, strlen(token));
	}
	free(pSrc);

	return iRep;

}


int Wow_ReadPacket(int ch, SOCKET insock, char *request, int size) {
	int rc, st;

	fd_set  readfds, exceptfds;

	static struct timeval tv;
	tv.tv_sec = DEFAULT_RECV_TIME;
	tv.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);
	FD_SET(insock, &readfds);
	FD_SET(insock, &exceptfds);

	st = select(0, &readfds, NULL, &exceptfds, &tv);
	if (st == SOCKET_ERROR){
		xprintf("[CH:%03d] Wow_ReadPacket: select() with error %d", ch, WSAGetLastError());
		return -2;
	}
	else if (!FD_ISSET(insock, &readfds)){
		xprintf("[CH:%03d] (%d) Wow_ReadPacket: Select() returned no readfds ready", ch, insock);
		return 0;
	}

	rc = recv(insock, request, size, 0);
	if (rc == SOCKET_ERROR){
		xprintf("[CH:%03d] Wow_ReadPacket: recv() failed with error %d", ch, WSAGetLastError());
		return -2;
	}
	else if (rc == 0){
		xprintf("[CH:%03d] Wow_ReadPacket: Connection closed by client", ch);
		return -3;
	}

	xprintf("[CH:%03d] Received [%d] -> [%-16.16s] from client", ch, rc, request);
	return rc;
}


int  Wow_ReceiveSize(int ch, SOCKET sock, char *rbuf, int recvsize)
{
	int			rn;
	int			totrcvsize = 0;

	while (1)
	{
		rn = Wow_ReadPacket(ch, sock, ((char *)rbuf) + (totrcvsize), recvsize - totrcvsize);
		if (rn == 0)
			continue;
		else if (rn < 0)
			return 0;

		totrcvsize += rn;
		if (recvsize - totrcvsize == 0)
			break;
	}

	return	totrcvsize;
}

//////////////////////////////////////////////////////////////////////////////////////////
//																						//
//																						//
//																						//
//////////////////////////////////////////////////////////////////////////////////////////
int Wow_WritePacket(int ch, SOCKET outsock, char *answer, int size) {
	int rc, st;

	fd_set  writefds, exceptfds;

	static struct timeval tv;
	tv.tv_sec = DEFAULT_SEND_TIME;
	tv.tv_usec = 0;

	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	FD_SET(outsock, &writefds);
	FD_SET(outsock, &exceptfds);

	st = select(0, NULL, &writefds, &exceptfds, &tv);
	if (st == SOCKET_ERROR){
		xprintf("[CH:%03d] Wow_WritePacket: select() with error %d", ch, WSAGetLastError());
		return -2;
	}
	else if (!FD_ISSET(outsock, &writefds)){
		xprintf("[CH:%03d] Wow_WritePacket: Select() returned no writefds ready", ch);
		return 0;
	}

	rc = send(outsock, answer, size, 0);
	if (rc == SOCKET_ERROR){
		xprintf("[CH:%03d] Wow_WritePacket: send() failed with error %d", ch, WSAGetLastError());
		return -2;
	}
	else if (rc == 0){
		xprintf("[CH:%03d] Wow_WritePacket: Connection closed by client", ch);
		return -3;
	}
	xprintf("[CH:%03d] Sending [%d] -> to client", ch, rc);
	return rc;
}


int  Wow_WriteSize(int ch, SOCKET sock, char *rbuf, int sendsize)
{
	int			rn;
	int			totrcvsize = 0;

	while (1)
	{
		rn = Wow_WritePacket(ch, sock, ((char *)rbuf) + (totrcvsize), sendsize - totrcvsize);
		if (rn == 0)
			continue;
		else if (rn < 0)
			return 0;

		totrcvsize += rn;
		if (sendsize - totrcvsize == 0)
			break;
	}

	return	totrcvsize;
}
void Wow_REQ_Quithostio(char *p, int ch)
{
	xprintf("[CH:%03d] Wow_REQ_Quithostio===START", ch);
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*port)[ch].pScenario);


	if (!in_multifunc(ch))
	{
		Sleep(500);
	}

	if (pScenario->m_AdoDb != NULL)
	{
		delete pScenario->m_AdoDb;
		pScenario->m_AdoDb = NULL;
	}
	CoUninitialize();


	(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;

	if ((*port)[ch].used != L_IDLE && in_multifunc(ch))
		quitchan(ch);

	string ErrMsg = GetLastErrorAsString();
	xprintf("[CH:%03d] %s > %s\n", ch, p, ErrMsg.c_str());

	xprintf("[CH:%03d] Wow_REQ_Quithostio _endthread", ch);
}

unsigned int __stdcall Wow_InfoRodocReq_Process(void *data)
{
	int			ch = 0;
	int			threadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = lineTablePtr->threadID;

	xprintf("[CH:%03d] Wow_InfoRodocReq_Process START", ch);

	memset(pScenario->m_szMx_issue_no, 0x00, sizeof(pScenario->m_szMx_issue_no));
	memset(pScenario->m_szMx_name, 0x00, sizeof(pScenario->m_szMx_name));
	memset(pScenario->m_szMx_id, 0x00, sizeof(pScenario->m_szMx_id));
	memset(pScenario->m_szCC_name, 0x00, sizeof(pScenario->m_szCC_name));
	memset(pScenario->m_szCC_Prod_Desc, 0x00, sizeof(pScenario->m_szCC_Prod_Desc));
	memset(pScenario->m_szpsrtner_nm, 0x00, sizeof(pScenario->m_szpsrtner_nm));
	memset(pScenario->m_szCC_Prod_Code, 0x00, sizeof(pScenario->m_szCC_Prod_Code));
	memset(pScenario->m_szPhone_no, 0x00, sizeof(pScenario->m_szPhone_no));
	memset(pScenario->m_szMx_opt, 0x00, sizeof(pScenario->m_szMx_opt));
	memset(pScenario->m_szCC_email, 0X00, sizeof(pScenario->m_szCC_email));
	memset(pScenario->m_sz_Shop_Pw, 0X00, sizeof(pScenario->m_sz_Shop_Pw));
	pScenario->m_nAmount = 0;
	memset(pScenario->m_szInstallment, 0x00, sizeof(pScenario->m_szInstallment));
	// AHN 2022.0808 정기결제 수정 
	memset(pScenario->m_szsub_status, 0X00, sizeof(pScenario->m_szsub_status));
	memset(pScenario->m_szsub_has_trial, 0X00, sizeof(pScenario->m_szsub_has_trial));
	memset(pScenario->m_szexpire_date, 0X00, sizeof(pScenario->m_szexpire_date));
	pScenario->m_nsub_amount = 0;

	// 결제 후 리턴 페이지 주소 정보 초기화
	memset(pScenario->m_szURL_YN, 0X00, sizeof(pScenario->m_szURL_YN));
	memset(pScenario->m_szSHOP_RET_URL, 0X00, sizeof(pScenario->m_szSHOP_RET_URL));


	if (threadID != lineTablePtr->threadID) {
		pScenario->m_bDnisInfo = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;

		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process the line service is not valid any more.", ch);
		xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);

		_endthreadex((unsigned int)(*port)[ch].m_hThread);
		return -1;
	}

	BOOL bRet = 0;
	if (pScenario->m_hObject != NULL)
	{
		bRet = InternetCloseHandle(pScenario->m_hObject);
		pScenario->m_hObject = NULL;
	}
	if (pScenario->m_hConnect != NULL)
	{
		bRet = InternetCloseHandle(pScenario->m_hConnect);
		pScenario->m_hConnect = NULL;
	}
	if (pScenario->m_hSession != NULL)
	{
		bRet = InternetCloseHandle(pScenario->m_hSession);
		pScenario->m_hSession = NULL;
	}


#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
	CoInitialize(NULL);
#endif

	INFOPRODOCREQ stuReq_Body;
	memset(&stuReq_Body, 0x00, sizeof(INFOPRODOCREQ));
	sprintf_s(stuReq_Body.m_szDNIS, sizeof(stuReq_Body), "%s", pScenario->szDnis);
	sprintf_s(stuReq_Body.m_szHP_NO, sizeof(stuReq_Body.m_szHP_NO), "%s", pScenario->m_szInputTel);

	CString sendURL;
// AHN 2022.07.13 wownet 정기결제 개발 
#if 1 //정기결제  페이레터에서 일반결제와 동일한 api 사용하기를 원해서 수정함. 
	//sendURL.Format("https://dev-adminwownet.payletter.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"0102"*/, "01011111001");		//A-1 정기결제 최초 등록하는 경우 (무료체험 없는 경우)
	//sendURL.Format("https://dev-adminwownet.payletter.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"0103"*/, "01011111002");	//A-2 정기결제 최초 등록하는 경우 (무료체험 있는 경우)
	//sendURL.Format("https://dev-adminwownet.payletter.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"0102"*/, "01011111003");	//B   정기결제 등록되어 이용 중인 경우
	//sendURL.Format("https://dev-adminwownet.payletter.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"0103"*/, "01011111004");	//C 정기결제 해지 후 동일 상품 재 결제 시 (사용기간 남은 경우)
	//sendURL.Format("https://dev-adminwownet.payletter.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"0102"*/, "01011111005");	//D-1 정기결제 해지 후 동일 상품 재결제 시 (사용기간 종료된 경우) (무료 체험 없는 경우)
	//sendURL.Format("https://dev-adminwownet.payletter.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"0103"*/, "01011111006");	//D-2 정기결제 해지 후 동일 상품 재결제 시 (사용기간 종료된 경우) (무료 체험 있는 경우)

	//sendURL.Format("https://dev-adminwownet.payletter.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"4400"*/, (LPCTSTR)pScenario->m_szInputTel/*"01085676505"*/);
	sendURL.Format("https://billadmin.wownet.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"4400"*/, (LPCTSTR)pScenario->m_szInputTel/*"01085676505"*/);
#else
	sendURL.Format("https://billadmin.wownet.co.kr/pgmodule/DasomARS/UserCall/orderRequest_DirectPayApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"4400"*/, (LPCTSTR)pScenario->m_szInputTel/*"01085676505"*/);
#endif
	Http_SSL_RetPageSend(data, sendURL.GetBuffer(), "POST"); //221208

	MSXML2::IXMLDOMDocument2Ptr pDoc2 = NULL;
	pDoc2.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if (!pDoc2) {
		pScenario->m_bDnisInfo = -1;

		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process 응답 XML 초기화 실패", ch);
		return 0; // DOMDocument인스턴스 생성 실패
	}

	if (!pDoc2->loadXML(pScenario->m_szHttpBuffer)) {

		MSXML2::IXMLDOMParseErrorPtr pError = pDoc2->parseError;
		pScenario->m_bDnisInfo = -1;
		_bstr_t strResult = "";
		bool isValidate = true;
		if (pError->errorCode != S_OK)
		{
			strResult = _bstr_t("Validation Ret failed on ") +
				_bstr_t("\n=====================") +
				_bstr_t("\nReason: ") + _bstr_t(pError->Getreason()) +
				_bstr_t("\nSource: ") + _bstr_t(pError->GetsrcText()) +
				_bstr_t("\nLine: ") + _bstr_t(pError->Getline()) +
				_bstr_t("\n");
			isValidate = false;
		}
		else {
			strResult = _bstr_t("Validation Ret succeeded for ") +
				_bstr_t("\n======================\n") +
				_bstr_t(pDoc2->xml) + _bstr_t("\n");
			isValidate = true;
		}

		pError.Release(); // RefCount를 사용하는 포인터이므로 Release과정이 꼭 필요
		pDoc2.Release();

		xprintf("[CH:%03d] %s", ch, (LPCSTR)strResult);
		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process RET XML ERROR", ch);

		return 0; // XML파싱 실패
	}
	MSXML2::IXMLDOMNodePtr retval;// 1: 리턴 코드
	MSXML2::IXMLDOMNodePtr errmsg;// 2: 오류 메시지
	MSXML2::IXMLDOMNodePtr order_no;// 3:	char	71	가맹점 거래번호	O	가맹점에서 생성 - 거래고유번호;
	MSXML2::IXMLDOMNodePtr shop_id1;// 4: char	33	가맹점 아이디	O	가맹점에게 부여된 아이디:간편결제용
	MSXML2::IXMLDOMNodePtr shop_id2;// 5: char	33	가맹점 아이디	O	가맹점에게 부여된 아이디:일반결제용
	MSXML2::IXMLDOMNodePtr cc_name;	//6: char	65	고객명	x
	MSXML2::IXMLDOMNodePtr cc_pord_desc;// 7: char	256	상품명	O
	MSXML2::IXMLDOMNodePtr cc_pord_code;// 8: char	51	상품코드		상품 코드입니다.
	MSXML2::IXMLDOMNodePtr amount;	   // 9: char	11	결제금액	O
	MSXML2::IXMLDOMNodePtr partner_nm;	// 10: char	256	파트너명	O
	MSXML2::IXMLDOMNodePtr bill_key;// 11:	varchar	256	O	간편결제용 빌키
	MSXML2::IXMLDOMNodePtr ext_data;// 12:	char	64	O	간편결제 인증용 생년월일
	MSXML2::IXMLDOMNodePtr renew_flag;// 13: char	1	M	동의ARS 여부를 위해 상품보유 여부 값	동일상품 보유 중 : Y 보유하지 않으면 : N
	MSXML2::IXMLDOMNodePtr card_name;//카드사명
// AHN 2022.0808 정기결제 수정 
	MSXML2::IXMLDOMNodePtr sub_status;// 정기결제 상태 (0: 첫 결제, 1: 이용 중, 2: 해지)
	MSXML2::IXMLDOMNodePtr sub_amount;// 정기결제 금액
	MSXML2::IXMLDOMNodePtr sub_has_trial;// 체험 상품 유무 (Y: 체험상품 있는 상품, N: 체험상품 없는 상품)
	MSXML2::IXMLDOMNodePtr expire_date;// 상품 만료 일자 (yyyyMMdd 형식)

	try{
// AHN 2022.07.13 wownet 정기결제 개발
#if 1   //shopid1, shopid2 가 오지 않아서 shopid 를 받는 것으로 처리, renew_flag, bill_key가 null 이면 안되기에 임시값으로 처리 
		retval = pDoc2->selectSingleNode(_bstr_t("/Response/retval"));
		errmsg = pDoc2->selectSingleNode(_bstr_t("/Response/errmsg"));
		order_no = pDoc2->selectSingleNode(_bstr_t("/Response/order_no"));
		shop_id1 = pDoc2->selectSingleNode(_bstr_t("/Response/shop_id"));
		shop_id2 = pDoc2->selectSingleNode(_bstr_t("/Response/shop_id"));
		cc_name = pDoc2->selectSingleNode(_bstr_t("/Response/cc_name"));
		cc_pord_desc = pDoc2->selectSingleNode(_bstr_t("/Response/cc_pord_desc"));
		cc_pord_code = pDoc2->selectSingleNode(_bstr_t("/Response/cc_pord_code"));
		amount = pDoc2->selectSingleNode(_bstr_t("/Response/amount"));
		partner_nm = pDoc2->selectSingleNode(_bstr_t("/Response/partner_nm"));
		bill_key = pDoc2->selectSingleNode(_bstr_t("/Response/shop_id"));
		ext_data = pDoc2->selectSingleNode(_bstr_t("/Response/ext_data"));
		renew_flag = pDoc2->selectSingleNode(_bstr_t("/Response/shop_id"));
		card_name = pDoc2->selectSingleNode(_bstr_t("/Response/card_name"));
// AHN 2022.0808 정기결제 수정 
		sub_status = pDoc2->selectSingleNode(_bstr_t("/Response/sub_status"));
		sub_amount = pDoc2->selectSingleNode(_bstr_t("/Response/sub_amount"));
		sub_has_trial = pDoc2->selectSingleNode(_bstr_t("/Response/sub_has_trial"));
		expire_date = pDoc2->selectSingleNode(_bstr_t("/Response/expire_date"));

#else		
		retval = pDoc2->selectSingleNode(_bstr_t("/Response/retval"));
		errmsg = pDoc2->selectSingleNode(_bstr_t("/Response/errmsg"));
		order_no = pDoc2->selectSingleNode(_bstr_t("/Response/order_no"));
		shop_id1 = pDoc2->selectSingleNode(_bstr_t("/Response/shop_id1"));
		shop_id2 = pDoc2->selectSingleNode(_bstr_t("/Response/shop_id2"));
		cc_name = pDoc2->selectSingleNode(_bstr_t("/Response/cc_name"));
		cc_pord_desc = pDoc2->selectSingleNode(_bstr_t("/Response/cc_pord_desc"));
		cc_pord_code = pDoc2->selectSingleNode(_bstr_t("/Response/cc_pord_code"));
		amount = pDoc2->selectSingleNode(_bstr_t("/Response/amount"));
		partner_nm = pDoc2->selectSingleNode(_bstr_t("/Response/partner_nm"));
		bill_key = pDoc2->selectSingleNode(_bstr_t("/Response/bill_key"));
		ext_data = pDoc2->selectSingleNode(_bstr_t("/Response/ext_data"));
		renew_flag = pDoc2->selectSingleNode(_bstr_t("/Response/renew_flag"));
		card_name = pDoc2->selectSingleNode(_bstr_t("/Response/card_name"));
#endif
	}
	catch (CMemoryException * e)
	{
		DetailErrorMessage(ch, e, __FILE__, __FUNCTION__, __LINE__);
		e->Delete();

		retval.Release();
		errmsg.Release();
		order_no.Release();
		shop_id1.Release();
		shop_id2.Release();
		cc_name.Release();
		cc_pord_desc.Release();
		partner_nm.Release();
		cc_pord_code.Release();
		amount.Release();

		bill_key.Release();
		ext_data.Release();
		renew_flag.Release();
		card_name.Release();

// AHN 2022.0808 정기결제 수정 
		sub_status.Release();
		sub_amount.Release();
		sub_has_trial.Release();
		expire_date.Release();

		pDoc2.Release();
		pScenario->m_bDnisInfo = -1;
		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process  개체 미 발견", ch);
		return 0; // XML파싱 실패
	}
	catch (CException * e)
	{
		DetailErrorMessage(ch, e, __FILE__, __FUNCTION__, __LINE__);
		e->Delete();

		if (retval != NULL)       retval.Release();
		if (errmsg != NULL)       errmsg.Release();
		if (order_no != NULL)     order_no.Release();
		if (shop_id1 != NULL)      shop_id1.Release();
		if (shop_id2 != NULL)      shop_id2.Release();
		if (cc_name != NULL)      cc_name.Release();
		if (cc_pord_desc != NULL) cc_pord_desc.Release();
		if (partner_nm != NULL)   partner_nm.Release();
		if (cc_pord_code != NULL) cc_pord_code.Release();
		if (amount != NULL)       amount.Release();
		if (bill_key != NULL)       bill_key.Release();
		if (ext_data != NULL)       ext_data.Release();
		if (renew_flag != NULL)       renew_flag.Release();
		if (card_name != NULL)  card_name.Release();

// AHN 2022.0808 정기결제 수정 
		if (sub_status != NULL)  sub_status.Release();
		if (sub_amount != NULL)  sub_amount.Release();
		if (sub_has_trial != NULL)  sub_has_trial.Release();
		if (expire_date != NULL)  expire_date.Release();

		pDoc2.Release();
		pScenario->m_bDnisInfo = -1;
		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process 개체 미 발견", ch);
		return 0; // XML파싱 실패
	}

	if (retval != NULL)
	{
		xprintf("[CH:%03d]  INFOPRODOCRES::리턴코드 %s", ch, (LPCTSTR)retval->Gettext());
		memset(pScenario->m_szretval, 0x00, sizeof(pScenario->m_szretval) - 1);
		
		if (strcmp((LPCTSTR)retval->Gettext(), "0") != 0)
		{
			pScenario->m_bDnisInfo = 0;
			if (strcmp((LPCTSTR)retval->Gettext(), "6020") == 0)
			{
				pScenario->m_bDnisInfo = 1;
				strncpy_s(pScenario->m_szretval, sizeof(pScenario->m_szretval), "6020", sizeof(pScenario->m_szretval) - 1);
				if (errmsg != NULL)
				{
					xprintf("[CH:%03d]  INFOPRODOCRES::리턴메시지 %s", ch, (LPCTSTR)errmsg->Gettext());
					memset(pScenario->m_szretMsg, 0x00, sizeof(pScenario->m_szretMsg));
					strncpy_s(pScenario->m_szretMsg, sizeof(pScenario->m_szretMsg), (LPCTSTR)errmsg->Gettext(), sizeof(pScenario->m_szretMsg) - 1);
				}
				if (partner_nm != NULL)
				{
					xprintf("[CH:%03d] INFOPRODOCRES::파트너명 (%s) ", ch, (LPCTSTR)partner_nm->Gettext());
					memset(pScenario->m_szpsrtner_nm, 0x00, sizeof(pScenario->m_szpsrtner_nm));
					strncpy_s(pScenario->m_szpsrtner_nm, sizeof(pScenario->m_szpsrtner_nm), (LPCTSTR)partner_nm->Gettext(), sizeof(pScenario->m_szpsrtner_nm) - 1);
				}
			}

			if (retval != NULL)       retval.Release();
			if (errmsg != NULL)       errmsg.Release();
			if (order_no != NULL)     order_no.Release();
			if (shop_id1 != NULL)      shop_id1.Release();
			if (shop_id2 != NULL)      shop_id2.Release();
			if (cc_name != NULL)      cc_name.Release();
			if (cc_pord_desc != NULL) cc_pord_desc.Release();
			if (partner_nm != NULL)   partner_nm.Release();
			if (cc_pord_code != NULL) cc_pord_code.Release();
			if (amount != NULL)       amount.Release();
			if (bill_key != NULL)       bill_key.Release();
			if (ext_data != NULL)       ext_data.Release();
			if (renew_flag != NULL)       renew_flag.Release();
			if (card_name != NULL)  card_name.Release();
// AHN 2022.0808 정기결제 수정 
			if (sub_status != NULL)  sub_status.Release();
			if (sub_amount != NULL)  sub_amount.Release();
			if (sub_has_trial != NULL)  sub_has_trial.Release();
			if (expire_date != NULL)  expire_date.Release();

			pDoc2.Release();

			if (strcmp(pScenario->m_szretval, "6020") == 0)
			{
				Wow_REQ_Quithostio("Wow_InfoRodocReq_Process the line service is SUCCESS.", ch);
				xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);

				return 0; // DOMDocument인스턴스 생성 실패
			}

			Wow_REQ_Quithostio("Wow_InfoRodocReq_Process 존재하지 않은 고객", ch);
			return 0; // DOMDocument인스턴스 생성 실패
		}
	}
	
	if (errmsg != NULL)
	{
		xprintf("[CH:%03d]  INFOPRODOCRES::리턴메시지 %s", ch, (LPCTSTR)errmsg->Gettext());
		memset(pScenario->m_szretMsg, 0x00, sizeof(pScenario->m_szretMsg) - 1);
		strncpy_s(pScenario->m_szretMsg, sizeof(pScenario->m_szretMsg), (LPCTSTR)errmsg->Gettext(), sizeof(pScenario->m_szretMsg) - 1);

// AHN 2022.07.13 wownet 정기결제 개발
#if 0 //정기결제 0원 결제 
		if(atoi((LPCTSTR)amount->Gettext()) < 1)
		{
			pScenario->m_bDnisInfo = 0;

			if (retval != NULL)       retval.Release();
			if (errmsg != NULL)       errmsg.Release();
			if (order_no != NULL)     order_no.Release();
			if (shop_id1 != NULL)      shop_id1.Release();
			if (shop_id2 != NULL)      shop_id2.Release();
			if (cc_name != NULL)      cc_name.Release();
			if (cc_pord_desc != NULL) cc_pord_desc.Release();
			if (partner_nm != NULL)   partner_nm.Release();
			if (cc_pord_code != NULL) cc_pord_code.Release();
			if (amount != NULL)       amount.Release();
			if (bill_key != NULL)       bill_key.Release();
			if (ext_data != NULL)       ext_data.Release();
			if (renew_flag != NULL)       renew_flag.Release();
			if (card_name != NULL)  card_name.Release();
			// AHN 2022.0808 정기결제 수정 
			if (sub_status != NULL)  sub_status.Release();
			if (sub_amount != NULL)  sub_amount.Release();
			if (sub_has_trial != NULL)  sub_has_trial.Release();
			if (expire_date != NULL)  expire_date.Release();

			pDoc2.Release();

			Wow_REQ_Quithostio("Wow_InfoRodocReq_Process 결제 금액 0원이하로 결제 금액 없음", ch);
			return 0;
		}
#endif
	}

	INFOPRODOCRES stu_InfoProdocRes;
	memset(&stu_InfoProdocRes, 0x00, sizeof(INFOPRODOCRES));

	if (order_no != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::주문번호 (%s) ", ch, (LPCTSTR)order_no->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szorder_no, sizeof(stu_InfoProdocRes.m_szorder_no), (LPCTSTR)order_no->Gettext(), sizeof(stu_InfoProdocRes.m_szorder_no) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::주문번호 (%s) ", ch, stu_InfoProdocRes.m_szorder_no);
	}

	if (shop_id1 != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::간편결제용 가맹점 아이디 (%s) ", ch, (LPCTSTR)shop_id1->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szshop_id1, sizeof(stu_InfoProdocRes.m_szshop_id1), (LPCTSTR)shop_id1->Gettext(), sizeof(stu_InfoProdocRes.m_szshop_id1) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::간편결제용가맹점 아이디 (%s) ", ch, stu_InfoProdocRes.m_szshop_id1);
	}
	if (shop_id2 != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::일반결제용 가맹점 아이디 (%s) ", ch, (LPCTSTR)shop_id2->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szshop_id2, sizeof(stu_InfoProdocRes.m_szshop_id2), (LPCTSTR)shop_id2->Gettext(), sizeof(stu_InfoProdocRes.m_szshop_id2) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::일반결제용가맹점 아이디 (%s) ", ch, stu_InfoProdocRes.m_szshop_id2);
	}

	if (cc_name != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::고객명 (%s) ", ch, (LPCTSTR)cc_name->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szcc_name, sizeof(stu_InfoProdocRes.m_szcc_name), (LPCTSTR)cc_name->Gettext(), sizeof(stu_InfoProdocRes.m_szcc_name) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::고객명 (%s) ", ch, stu_InfoProdocRes.m_szcc_name);
	}

	if (cc_pord_desc != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::상품명 (%s) ", ch, (LPCTSTR)cc_pord_desc->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szcc_pord_desc, sizeof(stu_InfoProdocRes.m_szcc_pord_desc), (LPCTSTR)cc_pord_desc->Gettext(), sizeof(stu_InfoProdocRes.m_szcc_pord_desc) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::상품명 (%s) ", ch, stu_InfoProdocRes.m_szcc_pord_desc);
	}

	if (partner_nm != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::파트너명 (%s) ", ch, (LPCTSTR)partner_nm->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szpartner_nm, sizeof(stu_InfoProdocRes.m_szpartner_nm), (LPCTSTR)partner_nm->Gettext(), sizeof(stu_InfoProdocRes.m_szpartner_nm) - 1);
	}
	else strncpy_s((char *)stu_InfoProdocRes.m_szpartner_nm, sizeof(stu_InfoProdocRes.m_szpartner_nm), "최승옥", strlen("최승옥"));
	xprintf("[CH:%03d] stu_InfoProdocRes::파츠너명 (%s) ", ch, stu_InfoProdocRes.m_szpartner_nm);

	if (cc_pord_code != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::상품코드 (%s) ", ch, (LPCTSTR)cc_pord_code->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szcc_pord_code, sizeof(stu_InfoProdocRes.m_szcc_pord_code), (LPCTSTR)cc_pord_code->Gettext(), sizeof(stu_InfoProdocRes.m_szcc_pord_code) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::상품코드 (%s) ", ch, stu_InfoProdocRes.m_szcc_pord_code);
	}

	if (amount != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::결제금액 (%s) ", ch, (LPCTSTR)amount->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szamount, sizeof(stu_InfoProdocRes.m_szamount), (LPCTSTR)amount->Gettext(), sizeof(stu_InfoProdocRes.m_szamount) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::결제금액 (%s) ", ch, stu_InfoProdocRes.m_szamount);
	}

	if (bill_key != NULL)
	{
// AHN 2022.07.13 wownet 정기결제 개발
#if 1 //정기결제 빌키가 null 이면 후처리 과정에서 빌키가 있는 것으로 처리 됨. 
		xprintf("[CH:%03d] INFOPRODOCRES::빌키 (%s) ", ch, (LPCTSTR)bill_key->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szbill_key, sizeof(stu_InfoProdocRes.m_szbill_key), "", sizeof(stu_InfoProdocRes.m_szbill_key) - 1);
		strncpy_s(pScenario->m_szbill_key, sizeof(pScenario->m_szbill_key), (const char*)&stu_InfoProdocRes.m_szbill_key, sizeof(pScenario->m_szbill_key) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::빌키 (%s) ", ch, stu_InfoProdocRes.m_szbill_key);
#else
		xprintf("[CH:%03d] INFOPRODOCRES::빌키 (%s) ", ch, (LPCTSTR)bill_key->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szbill_key, sizeof(stu_InfoProdocRes.m_szbill_key), (LPCTSTR)bill_key->Gettext(), sizeof(stu_InfoProdocRes.m_szbill_key) - 1);
		strncpy_s(pScenario->m_szbill_key, sizeof(pScenario->m_szbill_key), (const char*)&stu_InfoProdocRes.m_szbill_key, sizeof(pScenario->m_szbill_key) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::빌키 (%s) ", ch, stu_InfoProdocRes.m_szbill_key);
#endif
	}	
	if (ext_data != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::확장 데이터 (%s) ", ch, (LPCTSTR)ext_data->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szext_data, sizeof(stu_InfoProdocRes.m_szext_data), (LPCTSTR)ext_data->Gettext(), sizeof(stu_InfoProdocRes.m_szext_data) - 1);
		strncpy_s(pScenario->m_szext_data, sizeof(pScenario->m_szext_data), (const char*)&stu_InfoProdocRes.m_szext_data, sizeof(pScenario->m_szext_data) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::확장 데이터 (%s) ", ch, stu_InfoProdocRes.m_szext_data);
	}
	if (renew_flag != NULL)
	{
// AHN 2022.07.13 wownet 정기결제 개발
#if 1 //정기결제 약관동의가 N 인 것으로 강제 처리 
		xprintf("[CH:%03d] INFOPRODOCRES::약간동의 구분 (%s) ", ch, (LPCTSTR)renew_flag->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szrenew_flag, sizeof(stu_InfoProdocRes.m_szrenew_flag), "N", sizeof(stu_InfoProdocRes.m_szrenew_flag) - 1);
		strncpy_s(pScenario->m_szrenew_flag, sizeof(pScenario->m_szrenew_flag), (const char*)&stu_InfoProdocRes.m_szrenew_flag, sizeof(pScenario->m_szrenew_flag) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::약간동의 구분 (%s) ", ch, stu_InfoProdocRes.m_szrenew_flag);
#else
		xprintf("[CH:%03d] INFOPRODOCRES::약간동의 구분 (%s) ", ch, (LPCTSTR)renew_flag->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szrenew_flag, sizeof(stu_InfoProdocRes.m_szrenew_flag), (LPCTSTR)renew_flag->Gettext(), sizeof(stu_InfoProdocRes.m_szrenew_flag) - 1);
		strncpy_s(pScenario->m_szrenew_flag, sizeof(pScenario->m_szrenew_flag), (const char*)&stu_InfoProdocRes.m_szrenew_flag, sizeof(pScenario->m_szrenew_flag) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::약간동의 구분 (%s) ", ch, stu_InfoProdocRes.m_szrenew_flag);
#endif
	}
	if (card_name != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::카드사명 (%s) ", ch, (LPCTSTR)card_name->Gettext());
		strncpy_s((char *)&stu_InfoProdocRes.m_szcardName, sizeof(stu_InfoProdocRes.m_szcardName), (LPCTSTR)card_name->Gettext(), sizeof(stu_InfoProdocRes.m_szcardName) - 1);
		strncpy_s(pScenario->m_szcardName, sizeof(pScenario->m_szcardName), (const char*)&stu_InfoProdocRes.m_szcardName, sizeof(pScenario->m_szcardName) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::카드사명 (%s) ", ch, stu_InfoProdocRes.m_szrenew_flag);
	}

// AHN 2022.0808 정기결제 수정 

	if (sub_status != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::정기결제상태 (%s) ", ch, (LPCTSTR)sub_status->Gettext());
		strncpy_s(pScenario->m_szsub_status, sizeof(pScenario->m_szsub_status), (LPCTSTR)sub_status->Gettext(), sizeof(pScenario->m_szsub_status) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::정기결제상태 (%s) ", ch, pScenario->m_szsub_status);
	}

	if (sub_has_trial != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::체험상태유무 (%s) ", ch, (LPCTSTR)sub_has_trial->Gettext());
		strncpy_s(pScenario->m_szsub_has_trial, sizeof(pScenario->m_szsub_has_trial), (LPCTSTR)sub_has_trial->Gettext(), sizeof(pScenario->m_szsub_has_trial) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::체험상태유무 (%s) ", ch, pScenario->m_szsub_has_trial);
	}

	if (expire_date != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::상품만료일자 (%s) ", ch, (LPCTSTR)expire_date->Gettext());
		strncpy_s(pScenario->m_szexpire_date, sizeof(pScenario->m_szexpire_date), (LPCTSTR)expire_date->Gettext(), sizeof(pScenario->m_szexpire_date) - 1);
		//strncpy_s(pScenario->m_szexpire_date, sizeof(pScenario->m_szexpire_date), "20220901", sizeof(pScenario->m_szexpire_date) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::상품만료일자 (%s) ", ch, pScenario->m_szexpire_date);
	}

	if (sub_amount != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::정기결제금액 (%s) ", ch, (LPCTSTR)sub_amount->Gettext());
		pScenario->m_nsub_amount = atoi((LPCTSTR)sub_amount->Gettext());
//		strncpy_s(pScenario->m_szexpire_date, sizeof(pScenario->m_szexpire_date), (LPCTSTR)sub_amount->Gettext(), sizeof(pScenario->m_szexpire_date) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::정기결제금액 (%d) ", ch, pScenario->m_nsub_amount);
	}

	
	char szDATABASE_IP_URL[50 + 1];
	char szDATABASE[50 + 1];
	char szSID[50 + 1];
	char szPASSWORD[50 + 1];

	GetPrivateProfileString(DATABASE_SESSION, "DATABASE_IP_URL", DATABASE_IP_URL, szDATABASE_IP_URL, sizeof(szDATABASE_IP_URL), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "DATABASE", DATABASE, szDATABASE, sizeof(szDATABASE), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "SID", SID, szSID, sizeof(szSID), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "PASSWORD", PASSWORD, szPASSWORD, sizeof(szPASSWORD), PARAINI);

	CoInitialize(0);
	pScenario->m_AdoDb = new CADODB(pScenario);
	if (pScenario->m_AdoDb == NULL){
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;

		if (retval != NULL)       retval.Release();
		if (errmsg != NULL)       errmsg.Release();
		if (order_no != NULL)     order_no.Release();
		if (shop_id1 != NULL)      shop_id1.Release();
		if (shop_id2 != NULL)      shop_id2.Release();
		if (cc_name != NULL)      cc_name.Release();
		if (cc_pord_desc != NULL) cc_pord_desc.Release();
		if (partner_nm != NULL)   partner_nm.Release();
		if (cc_pord_code != NULL) cc_pord_code.Release();
		if (amount != NULL)       amount.Release();
		if (bill_key != NULL)       bill_key.Release();
		if (ext_data != NULL)       ext_data.Release();
		if (renew_flag != NULL)       renew_flag.Release();
		if (card_name != NULL)  card_name.Release();
		// AHN 2022.0808 정기결제 수정 
		if (sub_status != NULL)  sub_status.Release();
		if (sub_amount != NULL)  sub_amount.Release();
		if (sub_has_trial != NULL)  sub_has_trial.Release();
		if (expire_date != NULL)  expire_date.Release();

		pDoc2.Release();

		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process > the line service Object is Null.", ch);
		xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}
	if (pScenario->m_AdoDb->DBConnect(szPASSWORD, szSID, szDATABASE, szDATABASE_IP_URL) == NULL)
	{
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;

		if (retval != NULL)       retval.Release();
		if (errmsg != NULL)       errmsg.Release();
		if (order_no != NULL)     order_no.Release();
		if (shop_id1 != NULL)      shop_id1.Release();
		if (shop_id2 != NULL)      shop_id2.Release();
		if (cc_name != NULL)      cc_name.Release();
		if (cc_pord_desc != NULL) cc_pord_desc.Release();
		if (partner_nm != NULL)   partner_nm.Release();
		if (cc_pord_code != NULL) cc_pord_code.Release();
		if (amount != NULL)       amount.Release();
		if (bill_key != NULL)       bill_key.Release();
		if (ext_data != NULL)       ext_data.Release();
		if (renew_flag != NULL)       renew_flag.Release();
		if (card_name != NULL)  card_name.Release();
		// AHN 2022.0808 정기결제 수정 
		if (sub_status != NULL)  sub_status.Release();
		if (sub_amount != NULL)  sub_amount.Release();
		if (sub_has_trial != NULL)  sub_has_trial.Release();
		if (expire_date != NULL)  expire_date.Release();

		pDoc2.Release();

		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process > (DBConnect iResult Error)", ch);
		xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	bRet = pScenario->m_AdoDb->RegOrderInfo(stuReq_Body, stu_InfoProdocRes);

	if (bRet == TRUE)
	{
		if (pScenario->m_AdoDb->sp_getAllatOrderInfoByTel4(stuReq_Body.m_szDNIS, stuReq_Body.m_szHP_NO, stu_InfoProdocRes.m_szshop_id1, stu_InfoProdocRes.m_szshop_id2))
		{
			// 2015.12.10  형 변환 호환성 문제 발생
			/*_bstr_t*/ _variant_t bt = "";
			char szAmount[12 + 1];
			int iROWCOUNT;
			int iField;

			iROWCOUNT = pScenario->m_AdoDb->GetRecCount();
			if (iROWCOUNT <= 0)
			{
				pScenario->m_bDnisInfo = 0;
				(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;

				if (retval != NULL)       retval.Release();
				if (errmsg != NULL)       errmsg.Release();
				if (order_no != NULL)     order_no.Release();
				if (shop_id1 != NULL)      shop_id1.Release();
				if (shop_id2 != NULL)      shop_id2.Release();
				if (cc_name != NULL)      cc_name.Release();
				if (cc_pord_desc != NULL) cc_pord_desc.Release();
				if (partner_nm != NULL)   partner_nm.Release();
				if (cc_pord_code != NULL) cc_pord_code.Release();
				if (amount != NULL)       amount.Release();
				if (bill_key != NULL)       bill_key.Release();
				if (ext_data != NULL)       ext_data.Release();
				if (renew_flag != NULL)       renew_flag.Release();
				if (card_name != NULL)  card_name.Release();
				// AHN 2022.0808 정기결제 수정 
				if (sub_status != NULL)  sub_status.Release();
				if (sub_amount != NULL)  sub_amount.Release();
				if (sub_has_trial != NULL)  sub_has_trial.Release();
				if (expire_date != NULL)  expire_date.Release();

				pDoc2.Release();

				Wow_REQ_Quithostio("Wow_InfoRodocReq_Process > (DBConnect iResult Error)", ch);
				xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);
				_endthreadex((unsigned int)pScenario->m_hThread);
				return 0;
			}

			iField = pScenario->m_AdoDb->GetFieldCount();
			for (int iROW = 0; iROW < iROWCOUNT; iROW++)
			{//무조건 하나다
				// 필요한 변수 초기화 반드시 한다.
				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_ISSUE_NO"), bt);
				strncpy_s(pScenario->m_szMx_issue_no, sizeof(pScenario->m_szMx_issue_no), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_issue_no) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"SHOP_ID1"), bt);
				strncpy_s(pScenario->m_szMx_id1, sizeof(pScenario->m_szMx_id1), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_id1) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_OPT"), bt);
				strncpy_s(pScenario->m_szMx_opt1, sizeof(pScenario->m_szMx_opt1), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_opt1) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_NAME"), bt);
				strncpy_s(pScenario->m_szMx_name, sizeof(pScenario->m_szMx_name), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_name) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"CC_NAME"), bt);
				strncpy_s(pScenario->m_szCC_name, sizeof(pScenario->m_szCC_name), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_name) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"CC_PORD_DESC"), bt);
				strncpy_s(pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_szCC_Prod_Desc), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_Prod_Desc) - 1);

				pScenario->m_AdoDb->GetRs(_variant_t(L"AUTH_TYPE"), bt);
				strncpy_s(pScenario->m_szAUTH_TYPE1, sizeof(pScenario->m_szAUTH_TYPE1), (char*)(_bstr_t)bt, sizeof(pScenario->m_szAUTH_TYPE1) - 1);
				
				pScenario->m_AdoDb->GetRs(_variant_t(L"SHOP_ID2"), bt);
				strncpy_s(pScenario->m_szMx_id2, sizeof(pScenario->m_szMx_id2), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_id2) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_OPT2"), bt);
				strncpy_s(pScenario->m_szMx_opt2, sizeof(pScenario->m_szMx_opt2), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_opt2) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"AUTH_TYPE2"), bt);
				strncpy_s(pScenario->m_szAUTH_TYPE2, sizeof(pScenario->m_szAUTH_TYPE2), (char*)(_bstr_t)bt, sizeof(pScenario->m_szAUTH_TYPE2) - 1);
				
				strncpy_s(pScenario->m_szpsrtner_nm, sizeof(pScenario->m_szpsrtner_nm), stu_InfoProdocRes.m_szpartner_nm, sizeof(pScenario->m_szpsrtner_nm) - 1);

				char *pTmpChr = NULL;
				pTmpChr = strchr(pScenario->m_szCC_Prod_Desc, '^');
				if (pTmpChr)
				{
					strncpy_s(pScenario->m_szCC_Prod_Code, sizeof(pScenario->m_szCC_Prod_Code), pTmpChr + 1, sizeof(pScenario->m_szCC_Prod_Code) - 1);
					pTmpChr[0] = 0x00;
					//2017.08.17
					//수정
					//memset(pTmpChr, 0x00, strlen(pScenario->m_szCC_Prod_Code) + 1);
				}
				pScenario->m_AdoDb->GetRs(_variant_t(L"PHONE_NO"), bt);
				strncpy_s(pScenario->m_szPhone_no, sizeof(pScenario->m_szPhone_no), (char*)(_bstr_t)bt, sizeof(pScenario->m_szPhone_no) - 1);
				/*pScenario->m_AdoDb->GetRs(_variant_t(L"INSTALLMENT"), bt);
				strncpy_s(pScenario->m_szInstallment,sizeof(pScenario->m_szInstallment), (char*)(_bstr_t)bt, sizeof(pScenario->m_szInstallment) - 1);*/
				/*pScenario->m_AdoDb->GetRs(_variant_t(L"SHOP_PW"), bt);
				strncpy_s(pScenario->m_sz_Shop_Pw,sizeof(pScenario->m_sz_Shop_Pw), (char*)(_bstr_t)bt, sizeof(pScenario->m_sz_Shop_Pw) - 1); */

				memset(szAmount, 0x00, sizeof(szAmount));
				pScenario->m_AdoDb->GetRs(_variant_t(L"AMOUNT"), bt);
				strncpy_s(szAmount, sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
				pScenario->m_nAmount = atoi(szAmount);

				pScenario->m_AdoDb->GetRs(_variant_t(L"CC_EMAIL"), bt);
				strncpy_s(pScenario->m_szCC_email, sizeof(pScenario->m_szCC_email), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_email) - 1);

				// 결제 후 리턴 페이지 정보
				pScenario->m_AdoDb->GetRs(_variant_t(L"URL_YN"), bt);
				strncpy_s(pScenario->m_szURL_YN, sizeof(pScenario->m_szURL_YN), (char*)(_bstr_t)bt, sizeof(pScenario->m_szURL_YN) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"NOTI_URL"), bt);
				strncpy_s(pScenario->m_szSHOP_RET_URL1, sizeof(pScenario->m_szSHOP_RET_URL1), (char*)(_bstr_t)bt, sizeof(pScenario->m_szSHOP_RET_URL1) - 1);


				pScenario->m_AdoDb->GetRs(_variant_t(L"NOTI_URL2"), bt);
				strncpy_s(pScenario->m_szSHOP_RET_URL2, sizeof(pScenario->m_szSHOP_RET_URL2), (char*)(_bstr_t)bt, sizeof(pScenario->m_szSHOP_RET_URL2) - 1);

				/// 기본 선택은 간편결제로 한다.
				strncpy_s(pScenario->m_szMx_id, sizeof(pScenario->m_szMx_id), pScenario->m_szMx_id1, sizeof(pScenario->m_szMx_id) - 1);
				strncpy_s(pScenario->m_szAUTH_TYPE, sizeof(pScenario->m_szAUTH_TYPE), pScenario->m_szAUTH_TYPE1, sizeof(pScenario->m_szAUTH_TYPE) - 1);
				strncpy_s(pScenario->m_szMx_opt, sizeof(pScenario->m_szMx_opt), pScenario->m_szMx_opt1, sizeof(pScenario->m_szMx_opt) - 1);
				strncpy_s(pScenario->m_szSHOP_RET_URL, sizeof(pScenario->m_szSHOP_RET_URL), pScenario->m_szSHOP_RET_URL1, sizeof(pScenario->m_szSHOP_RET_URL) - 1);

				pScenario->m_AdoDb->Next();
			}
			pScenario->m_AdoDb->RSClose();
			pScenario->m_bDnisInfo = 1;
		}
		else pScenario->m_bDnisInfo = 0;
	}
	else pScenario->m_bDnisInfo = 0;


	if (retval != NULL)       retval.Release();
	if (errmsg != NULL)       errmsg.Release();
	if (order_no != NULL)     order_no.Release();
	if (shop_id1 != NULL)      shop_id1.Release();
	if (shop_id2 != NULL)      shop_id2.Release();
	if (cc_name != NULL)      cc_name.Release();
	if (cc_pord_desc != NULL) cc_pord_desc.Release();
	if (partner_nm != NULL)   partner_nm.Release();
	if (cc_pord_code != NULL) cc_pord_code.Release();
	if (amount != NULL)       amount.Release();

	if (bill_key != NULL)       bill_key.Release();
	if (ext_data != NULL)       ext_data.Release();
	if (renew_flag != NULL)       renew_flag.Release();
	if (card_name != NULL)  card_name.Release();
	// AHN 2022.0808 정기결제 수정 
	if (sub_status != NULL)  sub_status.Release();
	if (sub_amount != NULL)  sub_amount.Release();
	if (sub_has_trial != NULL)  sub_has_trial.Release();
	if (expire_date != NULL)  expire_date.Release();

	pDoc2.Release();
	Wow_REQ_Quithostio("Wow_InfoRodocReq_Process the line service is SUCCESS.", ch);

	xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);

	_endthreadex((unsigned int)(*port)[ch].m_hThread);
	return 0;
}

int getTcpOrderInfo_host(int holdm)
{
	//초기화	
	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_DBAccess = 0;
	if (holdm != 0) {
		if (new_guide) new_guide();
		(*lpmt)->trials = 0;
		(*lpmt)->Hmusic = HM_LOOP;
		if (set_guide) set_guide(holdm);
		if (send_guide)  send_guide(NODTMF);
	}

	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_AdoDb != NULL)
	{
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_DBAccess = -1;
		(*lpmt)->ppftbl[POST_NET].postcode = HI_OK;
		Wow_REQ_Quithostio("getOrderInfo_host  the line service is Load Error", (*lpmt)->chanID);
		return 0;
	}

	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hThread = (HANDLE)_beginthreadex(NULL, 0, Wow_InfoRodocReq_Process, (LPVOID)(*lpmt), 0, &(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->threadID));

	return(0);
}

// 간편 결제 결제키 해지 통보 쓰레드 함수
unsigned int __stdcall Wow_Billdelete_Process(void *data)
{
	int			ch = 0;
	int			threadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = lineTablePtr->threadID;

	xprintf("[CH:%03d] Wow_InfoRodocReq_Process START", ch);
	
	if (threadID != lineTablePtr->threadID) {
		pScenario->m_bDnisInfo = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;

		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process the line service is not valid any more.", ch);
		xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);

		_endthreadex((unsigned int)(*port)[ch].m_hThread);
		return -1;
	}

	BOOL bRet = 0;
	if (pScenario->m_hObject != NULL)
	{
		bRet = InternetCloseHandle(pScenario->m_hObject);
		pScenario->m_hObject = NULL;
	}
	if (pScenario->m_hConnect != NULL)
	{
		bRet = InternetCloseHandle(pScenario->m_hConnect);
		pScenario->m_hConnect = NULL;
	}
	if (pScenario->m_hSession != NULL)
	{
		bRet = InternetCloseHandle(pScenario->m_hSession);
		pScenario->m_hSession = NULL;
	}
	
	INFOPRODOCREQ stuReq_Body;
	memset(&stuReq_Body, 0x00, sizeof(INFOPRODOCREQ));
	sprintf_s(stuReq_Body.m_szDNIS, sizeof(stuReq_Body), "%s", pScenario->szDnis);
	sprintf_s(stuReq_Body.m_szHP_NO, sizeof(stuReq_Body.m_szHP_NO), "%s", pScenario->m_szInputTel);

	CString sendURL;
	sendURL.Format("https://billadmin.wownet.co.kr/pgmodule/DasomARS/UserCall/DirectPayBillTerminate.asp?shop_id=%s&bill_key=%s", (LPCTSTR)pScenario->m_szMx_id, (LPCTSTR)pScenario->m_szbill_key);
	Http_SSL_RetPageSend(data, sendURL.GetBuffer(), "POST");

    if (pScenario->m_RetCode < 0)
	{
		xprintf("[CH:%03d] Wow_Billdelete_Process 응답 XML  수신 실패", ch);
	}
	xprintf("[CH:%03d] Wow_Billdelete_Process SUECCESSFULLY END", ch);

	
	Wow_REQ_Quithostio("Wow_InfoRodocReq_Process the line service is SUCCESS.", ch);

	xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);

	_endthreadex((unsigned int)(*port)[ch].m_hThread);
	return 0;
}

// 간편 결제 결제키 해지 통보
int bill_delTcp_host(int holdm)
{
	//초기화	
	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_DBAccess = 0;
	if (holdm != 0) {
		if (new_guide) new_guide();
		(*lpmt)->trials = 0;
		(*lpmt)->Hmusic = HM_LOOP;
		if (set_guide) set_guide(holdm);
		if (send_guide)  send_guide(NODTMF);
	}

	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_AdoDb != NULL)
	{
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_DBAccess = -1;
		(*lpmt)->ppftbl[POST_NET].postcode = HI_OK;
		Wow_REQ_Quithostio("getOrderInfo_host  the line service is Load Error", (*lpmt)->chanID);
		return 0;
	}

	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hThread = (HANDLE)_beginthreadex(NULL, 0, Wow_Billdelete_Process, (LPVOID)(*lpmt), 0, &(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->threadID));

	return(0);
}
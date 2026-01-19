
// WowTvSocket.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "CommonDef.H"
#include "ALLATCommom.h"

#include    "WowTvSocket.h"
#include    "ADODB.h"
#include    "Scenaio.h"
#include    "AllatUtil.h"
#include    "ALLAT_Stockwin_Quick_New_Scenario.h"



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

#define	PARAINI		".\\AllatWowTvQuickPay_para.ini"
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
//#define DATABASE_IP_URL "arsdb.mediaford.co.kr,1433"
#define DATABASE_IP_URL "211.196.157.119,1433"
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
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*port)[ch].pScenario);


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
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)(lineTablePtr->pScenario);

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

#if 1 //AHN 20201110 도메인변경 요청으로 변경함. 신규서버 
	CString sendURL;//https://billadmin.wownet.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi
	sendURL.Format("https://billadmin.wownet.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"4400"*/, (LPCTSTR)pScenario->m_szInputTel/*"01085676505"*/);
#else
	CString sendURL;//https://billadmin.wowtv.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi
	sendURL.Format("https://billadmin.wowtv.co.kr/pgmodule/DasomARS/UserCall/orderRequestApi.asp?DNIS=%s&HP_NO=%s", (LPCTSTR)pScenario->szDnis/*"4400"*/, (LPCTSTR)pScenario->m_szInputTel/*"01085676505"*/);
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
	MSXML2::IXMLDOMNodePtr retval;// 리턴 코드
	MSXML2::IXMLDOMNodePtr errmsg;// 오류 메시지
	MSXML2::IXMLDOMNodePtr order_no;//	char	71	가맹점 거래번호	O	가맹점에서 생성 - 거래고유번호;
	MSXML2::IXMLDOMNodePtr shop_id;	//char	33	가맹점 아이디	O	가맹점에게 부여된 아이디
	MSXML2::IXMLDOMNodePtr cc_name;	//char	65	고객명	x
	MSXML2::IXMLDOMNodePtr cc_pord_desc;	//char	256	상품명	O
	MSXML2::IXMLDOMNodePtr partner_nm;	//char	256	파트너명	O
	MSXML2::IXMLDOMNodePtr cc_pord_code;	//char	51	상품코드		상품 코드입니다.
	MSXML2::IXMLDOMNodePtr amount;	//char	11	결제금액	O

	try{
		retval = pDoc2->selectSingleNode(_bstr_t("/Response/retval"));
		errmsg = pDoc2->selectSingleNode(_bstr_t("/Response/errmsg"));
		order_no = pDoc2->selectSingleNode(_bstr_t("/Response/order_no"));
		shop_id = pDoc2->selectSingleNode(_bstr_t("/Response/shop_id"));
		cc_name = pDoc2->selectSingleNode(_bstr_t("/Response/cc_name"));
		cc_pord_desc = pDoc2->selectSingleNode(_bstr_t("/Response/cc_pord_desc"));
		partner_nm = pDoc2->selectSingleNode(_bstr_t("/Response/partner_nm"));
		cc_pord_code = pDoc2->selectSingleNode(_bstr_t("/Response/cc_pord_code"));
		amount = pDoc2->selectSingleNode(_bstr_t("/Response/amount"));
	}
	catch (CMemoryException * e)
	{
		DetailErrorMessage(ch, e, __FILE__, __FUNCTION__, __LINE__);
		e->Delete();

		retval.Release();
		errmsg.Release();
		order_no.Release();
		shop_id.Release();
		cc_name.Release();
		cc_pord_desc.Release();
		partner_nm.Release();
		cc_pord_code.Release();
		amount.Release();

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
		if (shop_id != NULL)      shop_id.Release();
		if (cc_name != NULL)      cc_name.Release();
		if (cc_pord_desc != NULL) cc_pord_desc.Release();
		if (partner_nm != NULL)   partner_nm.Release();
		if (cc_pord_code != NULL) cc_pord_code.Release();
		if (amount != NULL)       amount.Release();

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
			if (shop_id != NULL)      shop_id.Release();
			if (cc_name != NULL)      cc_name.Release();
			if (cc_pord_desc != NULL) cc_pord_desc.Release();
			if (partner_nm != NULL)   partner_nm.Release();
			if (cc_pord_code != NULL) cc_pord_code.Release();
			if (amount != NULL)       amount.Release();

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

		if(atoi((LPCTSTR)amount->Gettext()) < 1)
		{
			pScenario->m_bDnisInfo = 0;

			if (retval != NULL)       retval.Release();
			if (errmsg != NULL)       errmsg.Release();
			if (order_no != NULL)     order_no.Release();
			if (shop_id != NULL)      shop_id.Release();
			if (cc_name != NULL)      cc_name.Release();
			if (cc_pord_desc != NULL) cc_pord_desc.Release();
			if (partner_nm != NULL)   partner_nm.Release();
			if (cc_pord_code != NULL) cc_pord_code.Release();
			if (amount != NULL)       amount.Release();

			pDoc2.Release();

			Wow_REQ_Quithostio("Wow_InfoRodocReq_Process 결제 금액 0원이하로 결제 금액 없음", ch);
			return 0;
		}
	}

	INFOPRODOCRES stu_InfoProdocRes;
	memset(&stu_InfoProdocRes, 0x00, sizeof(INFOPRODOCRES));

	if (order_no != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::주문번호 (%s) ", ch, (LPCTSTR)order_no->Gettext());
		memcpy(&stu_InfoProdocRes.m_szorder_no, (LPCTSTR)order_no->Gettext(), sizeof(stu_InfoProdocRes.m_szorder_no) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::주문번호 (%s) ", ch, stu_InfoProdocRes.m_szorder_no);
	}

	if (shop_id != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::가맹점 아이디 (%s) ", ch, (LPCTSTR)shop_id->Gettext());
		memcpy(&stu_InfoProdocRes.m_szshop_id, (LPCTSTR)shop_id->Gettext(), sizeof(stu_InfoProdocRes.m_szshop_id) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::가맹점 아이디 (%s) ", ch, stu_InfoProdocRes.m_szshop_id);
	}

	if (cc_name != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::고객명 (%s) ", ch, (LPCTSTR)cc_name->Gettext());
		memcpy(&stu_InfoProdocRes.m_szcc_name, (LPCTSTR)cc_name->Gettext(), sizeof(stu_InfoProdocRes.m_szcc_name) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::고객명 (%s) ", ch, stu_InfoProdocRes.m_szcc_name);
	}

	if (cc_pord_desc != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::상품명 (%s) ", ch, (LPCTSTR)cc_pord_desc->Gettext());
		memcpy(&stu_InfoProdocRes.m_szcc_pord_desc, (LPCTSTR)cc_pord_desc->Gettext(), sizeof(stu_InfoProdocRes.m_szcc_pord_desc) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::상품명 (%s) ", ch, stu_InfoProdocRes.m_szcc_pord_desc);
	}

	if (partner_nm != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::파트너명 (%s) ", ch, (LPCTSTR)partner_nm->Gettext());
		memcpy(&stu_InfoProdocRes.m_szpartner_nm, (LPCTSTR)partner_nm->Gettext(), sizeof(stu_InfoProdocRes.m_szpartner_nm) - 1);
	}
	else memcpy(stu_InfoProdocRes.m_szpartner_nm, "최승옥", strlen("최승옥"));
	xprintf("[CH:%03d] stu_InfoProdocRes::파츠너명 (%s) ", ch, stu_InfoProdocRes.m_szpartner_nm);

	if (cc_pord_code != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::상품코드 (%s) ", ch, (LPCTSTR)cc_pord_code->Gettext());
		memcpy(&stu_InfoProdocRes.m_szcc_pord_code, (LPCTSTR)cc_pord_code->Gettext(), sizeof(stu_InfoProdocRes.m_szcc_pord_code) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::상품코드 (%s) ", ch, stu_InfoProdocRes.m_szcc_pord_code);
	}

	if (amount != NULL)
	{
		xprintf("[CH:%03d] INFOPRODOCRES::결제금액 (%s) ", ch, (LPCTSTR)amount->Gettext());
		memcpy(&stu_InfoProdocRes.m_szamount, (LPCTSTR)amount->Gettext(), sizeof(stu_InfoProdocRes.m_szamount) - 1);
		xprintf("[CH:%03d] stu_InfoProdocRes::결제금액 (%s) ", ch, stu_InfoProdocRes.m_szamount);
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

		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process > the line service Object is Null.", ch);
		xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}
	if (pScenario->m_AdoDb->DBConnect(szPASSWORD, szSID, szDATABASE, szDATABASE_IP_URL) == NULL)
	{
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		Wow_REQ_Quithostio("Wow_InfoRodocReq_Process > (DBConnect iResult Error)", ch);
		xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	//AHN 20240630 개인정보보호
	xprintf("stuReq_Body.m_szHP_NO before : ", stuReq_Body.m_szHP_NO);
	memset(stuReq_Body.m_szHP_NO, 0x00, sizeof(stuReq_Body.m_szHP_NO));
	xprintf("stuReq_Body.m_szHP_NO after : ", stuReq_Body.m_szHP_NO);

	bRet = pScenario->m_AdoDb->RegOrderInfo(stuReq_Body, stu_InfoProdocRes);

	if (bRet == TRUE)
	{
		//AHN 20240630 개인정보보호 휴대폰대신 주문번호로 쿼리
		if (pScenario->m_AdoDb->sp_getAllatOrderInfoByOrderNo(stuReq_Body.m_szDNIS, stu_InfoProdocRes.m_szorder_no))
//		if (pScenario->m_AdoDb->sp_getAllatOrderInfoByTel2(stuReq_Body.m_szDNIS, stuReq_Body.m_szHP_NO))
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
				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_ID"), bt);
				strncpy_s(pScenario->m_szMx_id, sizeof(pScenario->m_szMx_id), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_id) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_OPT"), bt);
				strncpy_s(pScenario->m_szMx_opt, sizeof(pScenario->m_szMx_opt), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_opt) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_NAME"), bt);
				strncpy_s(pScenario->m_szMx_name, sizeof(pScenario->m_szMx_name), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_name) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"CC_NAME"), bt);
				strncpy_s(pScenario->m_szCC_name, sizeof(pScenario->m_szCC_name), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_name) - 1);
				pScenario->m_AdoDb->GetRs(_variant_t(L"CC_PORD_DESC"), bt);
				strncpy_s(pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_szCC_Prod_Desc), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_Prod_Desc) - 1);

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
				pScenario->m_AdoDb->GetRs(_variant_t(L"SHOP_RET_URL"), bt);
				strncpy_s(pScenario->m_szSHOP_RET_URL, sizeof(pScenario->m_szSHOP_RET_URL), (char*)(_bstr_t)bt, sizeof(pScenario->m_szSHOP_RET_URL) - 1);

				pScenario->m_AdoDb->Next();
			}
			pScenario->m_AdoDb->RSClose();
		}

		pScenario->m_bDnisInfo = 1;
	}
	else pScenario->m_bDnisInfo = 0;

	if (retval != NULL)       retval.Release();
	if (errmsg != NULL)       errmsg.Release();
	if (order_no != NULL)     order_no.Release();
	if (shop_id != NULL)      shop_id.Release();
	if (cc_name != NULL)      cc_name.Release();
	if (cc_pord_desc != NULL) cc_pord_desc.Release();
	if (partner_nm != NULL)   partner_nm.Release();
	if (cc_pord_code != NULL) cc_pord_code.Release();
	if (amount != NULL)       amount.Release();

	pDoc2.Release();
	Wow_REQ_Quithostio("Wow_InfoRodocReq_Process the line service is SUCCESS.", ch);

	xprintf("[CH:%03d] Wow_InfoRodocReq_Process END", ch);

	_endthreadex((unsigned int)(*port)[ch].m_hThread);
	return 0;
}

int getTcpOrderInfo_host(int holdm)
{
	//초기화
	((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->m_DBAccess = 0;
	if (holdm != 0) {
		if (new_guide) new_guide();
		(*lpmt)->trials = 0;
		(*lpmt)->Hmusic = HM_LOOP;
		if (set_guide) set_guide(holdm);
		if (send_guide)  send_guide(NODTMF);
	}

	if (((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->m_AdoDb != NULL)
	{
		((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->m_DBAccess = -1;
		(*lpmt)->ppftbl[POST_NET].postcode = HI_OK;
		Wow_REQ_Quithostio("getOrderInfo_host  the line service is Load Error", (*lpmt)->chanID);
		return 0;
	}

	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->m_hThread = (HANDLE)_beginthreadex(NULL, 0, Wow_InfoRodocReq_Process, (LPVOID)(*lpmt), 0, &(((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->threadID));

	return(0);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  신규 PayLetter REST API 연동 함수
//  작성일: 2026-01-19
//  기존 XML 기반 orderRequestApi.asp를 JSON 기반 REST API로 대체
//////////////////////////////////////////////////////////////////////////////////////////

#include "PayLetterAPI.h"

// 신규 API 호출 스레드 함수
unsigned int __stdcall PL_InfoOrderReq_Process(void *data)
{
	int ch = 0;
	int threadID;
	LPMTP *lineTablePtr = (LPMTP *)data;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = lineTablePtr->threadID;

	xprintf("[CH:%03d] PL_InfoOrderReq_Process START (New REST API)", ch);

	// 멤버 변수 초기화
	memset(pScenario->m_szMx_issue_no, 0x00, sizeof(pScenario->m_szMx_issue_no));
	memset(pScenario->m_szMx_name, 0x00, sizeof(pScenario->m_szMx_name));
	memset(pScenario->m_szMx_id, 0x00, sizeof(pScenario->m_szMx_id));
	memset(pScenario->m_szCC_name, 0x00, sizeof(pScenario->m_szCC_name));
	memset(pScenario->m_szCC_Prod_Desc, 0x00, sizeof(pScenario->m_szCC_Prod_Desc));
	memset(pScenario->m_szpsrtner_nm, 0x00, sizeof(pScenario->m_szpsrtner_nm));
	memset(pScenario->m_szCC_Prod_Code, 0x00, sizeof(pScenario->m_szCC_Prod_Code));
	memset(pScenario->m_szPhone_no, 0x00, sizeof(pScenario->m_szPhone_no));
	memset(pScenario->m_szMx_opt, 0x00, sizeof(pScenario->m_szMx_opt));
	memset(pScenario->m_szCC_email, 0x00, sizeof(pScenario->m_szCC_email));
	memset(pScenario->m_sz_Shop_Pw, 0x00, sizeof(pScenario->m_sz_Shop_Pw));
	pScenario->m_nAmount = 0;
	memset(pScenario->m_szInstallment, 0x00, sizeof(pScenario->m_szInstallment));
	memset(pScenario->m_szURL_YN, 0x00, sizeof(pScenario->m_szURL_YN));
	memset(pScenario->m_szSHOP_RET_URL, 0x00, sizeof(pScenario->m_szSHOP_RET_URL));

	// 신규 멤버 변수 초기화
	pScenario->m_nPurchaseAmt = 0;
	memset(pScenario->m_szCouponUseFlag, 0x00, sizeof(pScenario->m_szCouponUseFlag));
	memset(pScenario->m_szCouponName, 0x00, sizeof(pScenario->m_szCouponName));
	memset(pScenario->m_szBonusCashUseFlag, 0x00, sizeof(pScenario->m_szBonusCashUseFlag));
	pScenario->m_nBonusCashUseAmt = 0;
	memset(pScenario->m_szPurchaseLimitFlag, 0x00, sizeof(pScenario->m_szPurchaseLimitFlag));
	memset(pScenario->m_szPgCode, 0x00, sizeof(pScenario->m_szPgCode));
	pScenario->m_nMemberState = 0;
	memset(pScenario->m_szServiceCheckFlag, 0x00, sizeof(pScenario->m_szServiceCheckFlag));
	memset(pScenario->m_szMemberId, 0x00, sizeof(pScenario->m_szMemberId));

	// 스레드 유효성 검사
	if (threadID != lineTablePtr->threadID) {
		pScenario->m_bDnisInfo = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		Wow_REQ_Quithostio("PL_InfoOrderReq_Process the line service is not valid any more.", ch);
		xprintf("[CH:%03d] PL_InfoOrderReq_Process END", ch);
		_endthreadex((unsigned int)(*port)[ch].m_hThread);
		return -1;
	}

	// 기존 인터넷 핸들 정리
	BOOL bRet = 0;
	if (pScenario->m_hObject != NULL) {
		bRet = InternetCloseHandle(pScenario->m_hObject);
		pScenario->m_hObject = NULL;
	}
	if (pScenario->m_hConnect != NULL) {
		bRet = InternetCloseHandle(pScenario->m_hConnect);
		pScenario->m_hConnect = NULL;
	}
	if (pScenario->m_hSession != NULL) {
		bRet = InternetCloseHandle(pScenario->m_hSession);
		pScenario->m_hSession = NULL;
	}

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
	CoInitialize(NULL);
#endif

	// PayLetter API 호출
	PL_PaymentInfo plInfo;
	std::string dnis(pScenario->szDnis);
	std::string phoneNo(pScenario->m_szInputTel);

	xprintf("[CH:%03d] PL_InfoOrderReq_Process: DNIS=%s, PhoneNo=%s", ch, dnis.c_str(), phoneNo.c_str());

	// reqType=1 (회선번호), arsType="VARS" (보는ARS)
	if (!PL_GetPaymentInfo(1, dnis, phoneNo, "VARS", plInfo)) {
		// API 호출 실패
		std::string errMsg = PL_GetLastError();
		xprintf("[CH:%03d] PL_InfoOrderReq_Process: API 호출 실패 - %s", ch, errMsg.c_str());
		pScenario->m_bDnisInfo = -1;

		Wow_REQ_Quithostio("PL_InfoOrderReq_Process API call failed", ch);
		_endthreadex((unsigned int)(*port)[ch].m_hThread);
		return -1;
	}

	// 결과 코드 확인
	if (plInfo.resultCode != "0") {
		xprintf("[CH:%03d] PL_InfoOrderReq_Process: API 응답 오류 - resultCode=%s",
				ch, plInfo.resultCode.c_str());
		pScenario->m_bDnisInfo = 0;

		Wow_REQ_Quithostio("PL_InfoOrderReq_Process API response error", ch);
		_endthreadex((unsigned int)(*port)[ch].m_hThread);
		return 0;
	}

	// 구매 제한 확인
	if (plInfo.purchaseLimitFlag != "1") {
		std::string limitMsg = PL_GetPurchaseLimitMessage(plInfo.purchaseLimitFlag);
		xprintf("[CH:%03d] PL_InfoOrderReq_Process: 구매 제한 - %s (flag=%s)",
				ch, limitMsg.c_str(), plInfo.purchaseLimitFlag.c_str());
		pScenario->m_bDnisInfo = 0;

		// 구매 제한 플래그 저장
		strncpy_s(pScenario->m_szPurchaseLimitFlag, sizeof(pScenario->m_szPurchaseLimitFlag),
				  plInfo.purchaseLimitFlag.c_str(), sizeof(pScenario->m_szPurchaseLimitFlag) - 1);

		Wow_REQ_Quithostio("PL_InfoOrderReq_Process purchase limit", ch);
		_endthreadex((unsigned int)(*port)[ch].m_hThread);
		return 0;
	}

	// 서비스 점검 확인
	if (plInfo.serviceCheckFlag == "Y") {
		xprintf("[CH:%03d] PL_InfoOrderReq_Process: 서비스 점검 중 - 완료 예정: %s",
				ch, plInfo.checkCompleteTime.c_str());
		pScenario->m_bDnisInfo = 0;

		Wow_REQ_Quithostio("PL_InfoOrderReq_Process service maintenance", ch);
		_endthreadex((unsigned int)(*port)[ch].m_hThread);
		return 0;
	}

	// 결제 금액 검증
	if (plInfo.payAmt < 1) {
		xprintf("[CH:%03d] PL_InfoOrderReq_Process: 결제 금액 없음 - payAmt=%d", ch, plInfo.payAmt);
		pScenario->m_bDnisInfo = 0;

		Wow_REQ_Quithostio("PL_InfoOrderReq_Process invalid payment amount", ch);
		_endthreadex((unsigned int)(*port)[ch].m_hThread);
		return 0;
	}

	// ========================================================================
	// 응답 데이터를 시나리오 멤버 변수에 저장
	// 필드 매핑 (XML → JSON):
	//   order_no → orderNo (m_szMx_issue_no)
	//   shop_id → mallIdGeneral (m_szMx_id) - 일반결제용
	//   cc_name → memberId (m_szCC_name)
	//   cc_pord_desc → itemName (m_szCC_Prod_Desc)
	//   partner_nm → nickName (m_szpsrtner_nm)
	//   cc_pord_code → categoryId_2nd (m_szCC_Prod_Code)
	//   amount → payAmt (m_nAmount)
	// ========================================================================

	// 주문번호 (Int64 → String)
	char szOrderNo[80] = { 0 };
	sprintf_s(szOrderNo, sizeof(szOrderNo), "%lld", plInfo.orderNo);
	strncpy_s(pScenario->m_szMx_issue_no, sizeof(pScenario->m_szMx_issue_no),
			  szOrderNo, sizeof(pScenario->m_szMx_issue_no) - 1);

	// 가맹점 아이디 (일반결제: mallIdGeneral)
	strncpy_s(pScenario->m_szMx_id, sizeof(pScenario->m_szMx_id),
			  plInfo.mallIdGeneral.c_str(), sizeof(pScenario->m_szMx_id) - 1);

	// 회원ID (memberId → cc_name 대체)
	strncpy_s(pScenario->m_szCC_name, sizeof(pScenario->m_szCC_name),
			  plInfo.memberId.c_str(), sizeof(pScenario->m_szCC_name) - 1);

	// 상품명
	strncpy_s(pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_szCC_Prod_Desc),
			  plInfo.itemName.c_str(), sizeof(pScenario->m_szCC_Prod_Desc) - 1);

	// 필명 (nickName → partner_nm 대체, TTS 안내용)
	strncpy_s(pScenario->m_szpsrtner_nm, sizeof(pScenario->m_szpsrtner_nm),
			  plInfo.nickName.c_str(), sizeof(pScenario->m_szpsrtner_nm) - 1);

	// 상품코드 (categoryId_2nd 사용)
	strncpy_s(pScenario->m_szCC_Prod_Code, sizeof(pScenario->m_szCC_Prod_Code),
			  plInfo.categoryId_2nd.c_str(), sizeof(pScenario->m_szCC_Prod_Code) - 1);

	// 결제금액
	pScenario->m_nAmount = plInfo.payAmt;

	// 회원ID (UUID) 별도 저장
	strncpy_s(pScenario->m_szMemberId, sizeof(pScenario->m_szMemberId),
			  plInfo.memberId.c_str(), sizeof(pScenario->m_szMemberId) - 1);

	// ========================================================================
	// 신규 필드 저장
	// ========================================================================

	// 원가 (할인 전)
	pScenario->m_nPurchaseAmt = plInfo.purchaseAmt;

	// 쿠폰 정보
	strncpy_s(pScenario->m_szCouponUseFlag, sizeof(pScenario->m_szCouponUseFlag),
			  plInfo.couponUseFlag.c_str(), sizeof(pScenario->m_szCouponUseFlag) - 1);
	strncpy_s(pScenario->m_szCouponName, sizeof(pScenario->m_szCouponName),
			  plInfo.couponName.c_str(), sizeof(pScenario->m_szCouponName) - 1);

	// 보너스 캐시 정보
	strncpy_s(pScenario->m_szBonusCashUseFlag, sizeof(pScenario->m_szBonusCashUseFlag),
			  plInfo.bonusCashUseFlag.c_str(), sizeof(pScenario->m_szBonusCashUseFlag) - 1);
	pScenario->m_nBonusCashUseAmt = plInfo.bonusCashUseAmt;

	// 구매 제한/상태 정보
	strncpy_s(pScenario->m_szPurchaseLimitFlag, sizeof(pScenario->m_szPurchaseLimitFlag),
			  plInfo.purchaseLimitFlag.c_str(), sizeof(pScenario->m_szPurchaseLimitFlag) - 1);
	strncpy_s(pScenario->m_szPgCode, sizeof(pScenario->m_szPgCode),
			  plInfo.pgCode.c_str(), sizeof(pScenario->m_szPgCode) - 1);
	pScenario->m_nMemberState = plInfo.memberState;
	strncpy_s(pScenario->m_szServiceCheckFlag, sizeof(pScenario->m_szServiceCheckFlag),
			  plInfo.serviceCheckFlag.c_str(), sizeof(pScenario->m_szServiceCheckFlag) - 1);

	// Notification URL (일반결제)
	strncpy_s(pScenario->m_szSHOP_RET_URL, sizeof(pScenario->m_szSHOP_RET_URL),
			  plInfo.notiUrlGeneral.c_str(), sizeof(pScenario->m_szSHOP_RET_URL) - 1);
	if (strlen(pScenario->m_szSHOP_RET_URL) > 0) {
		strncpy_s(pScenario->m_szURL_YN, sizeof(pScenario->m_szURL_YN), "Y", 1);
	}

	xprintf("[CH:%03d] PL_InfoOrderReq_Process: 데이터 저장 완료", ch);
	xprintf("[CH:%03d]   주문번호: %s", ch, pScenario->m_szMx_issue_no);
	xprintf("[CH:%03d]   가맹점ID: %s", ch, pScenario->m_szMx_id);
	xprintf("[CH:%03d]   회원ID: %s", ch, pScenario->m_szCC_name);
	xprintf("[CH:%03d]   상품명: %s", ch, pScenario->m_szCC_Prod_Desc);
	xprintf("[CH:%03d]   필명: %s", ch, pScenario->m_szpsrtner_nm);
	xprintf("[CH:%03d]   결제금액: %d", ch, pScenario->m_nAmount);
	xprintf("[CH:%03d]   원가: %d", ch, pScenario->m_nPurchaseAmt);
	xprintf("[CH:%03d]   쿠폰사용: %s (%s)", ch, pScenario->m_szCouponUseFlag, pScenario->m_szCouponName);
	xprintf("[CH:%03d]   보너스캐시사용: %s (%d원)", ch, pScenario->m_szBonusCashUseFlag, pScenario->m_nBonusCashUseAmt);

	// ========================================================================
	// DB 저장 (기존 로직 유지 - sp_getAllatOrderInfoByOrderNo 호출)
	// ========================================================================

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
	if (pScenario->m_AdoDb == NULL) {
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		Wow_REQ_Quithostio("PL_InfoOrderReq_Process > DB Object is Null.", ch);
		xprintf("[CH:%03d] PL_InfoOrderReq_Process END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	if (pScenario->m_AdoDb->DBConnect(szPASSWORD, szSID, szDATABASE, szDATABASE_IP_URL) == NULL) {
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		Wow_REQ_Quithostio("PL_InfoOrderReq_Process > DBConnect Error", ch);
		xprintf("[CH:%03d] PL_InfoOrderReq_Process END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	// 주문 정보 DB에 저장 (RegOrderInfo_NewAPI 함수 필요)
	INFOPRODOCREQ stuReq_Body;
	memset(&stuReq_Body, 0x00, sizeof(INFOPRODOCREQ));
	sprintf_s(stuReq_Body.m_szDNIS, sizeof(stuReq_Body.m_szDNIS), "%s", pScenario->szDnis);
	// 휴대폰번호는 개인정보 보호를 위해 저장하지 않음
	memset(stuReq_Body.m_szHP_NO, 0x00, sizeof(stuReq_Body.m_szHP_NO));

	INFOPRODOCRES stu_InfoProdocRes;
	memset(&stu_InfoProdocRes, 0x00, sizeof(INFOPRODOCRES));
	strncpy_s(stu_InfoProdocRes.m_szorder_no, sizeof(stu_InfoProdocRes.m_szorder_no),
			  pScenario->m_szMx_issue_no, sizeof(stu_InfoProdocRes.m_szorder_no) - 1);
	strncpy_s(stu_InfoProdocRes.m_szshop_id, sizeof(stu_InfoProdocRes.m_szshop_id),
			  pScenario->m_szMx_id, sizeof(stu_InfoProdocRes.m_szshop_id) - 1);
	strncpy_s(stu_InfoProdocRes.m_szcc_name, sizeof(stu_InfoProdocRes.m_szcc_name),
			  pScenario->m_szCC_name, sizeof(stu_InfoProdocRes.m_szcc_name) - 1);
	strncpy_s(stu_InfoProdocRes.m_szcc_pord_desc, sizeof(stu_InfoProdocRes.m_szcc_pord_desc),
			  pScenario->m_szCC_Prod_Desc, sizeof(stu_InfoProdocRes.m_szcc_pord_desc) - 1);
	strncpy_s(stu_InfoProdocRes.m_szpartner_nm, sizeof(stu_InfoProdocRes.m_szpartner_nm),
			  pScenario->m_szpsrtner_nm, sizeof(stu_InfoProdocRes.m_szpartner_nm) - 1);
	strncpy_s(stu_InfoProdocRes.m_szcc_pord_code, sizeof(stu_InfoProdocRes.m_szcc_pord_code),
			  pScenario->m_szCC_Prod_Code, sizeof(stu_InfoProdocRes.m_szcc_pord_code) - 1);
	sprintf_s(stu_InfoProdocRes.m_szamount, sizeof(stu_InfoProdocRes.m_szamount), "%d", pScenario->m_nAmount);

	bRet = pScenario->m_AdoDb->RegOrderInfo(stuReq_Body, stu_InfoProdocRes);

	if (bRet == TRUE) {
		// DB 조회로 추가 정보 획득 (MX_OPT 등)
		if (pScenario->m_AdoDb->sp_getAllatOrderInfoByOrderNo(stuReq_Body.m_szDNIS, stu_InfoProdocRes.m_szorder_no)) {
			_variant_t bt = "";
			char szAmount[12 + 1];
			int iROWCOUNT = pScenario->m_AdoDb->GetRecCount();

			if (iROWCOUNT > 0) {
				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_OPT"), bt);
				strncpy_s(pScenario->m_szMx_opt, sizeof(pScenario->m_szMx_opt),
						  (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_opt) - 1);

				pScenario->m_AdoDb->GetRs(_variant_t(L"MX_NAME"), bt);
				strncpy_s(pScenario->m_szMx_name, sizeof(pScenario->m_szMx_name),
						  (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_name) - 1);

				pScenario->m_AdoDb->GetRs(_variant_t(L"CC_EMAIL"), bt);
				strncpy_s(pScenario->m_szCC_email, sizeof(pScenario->m_szCC_email),
						  (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_email) - 1);

				xprintf("[CH:%03d] PL_InfoOrderReq_Process: DB 추가 정보 획득 완료", ch);
				xprintf("[CH:%03d]   MX_OPT: %s", ch, pScenario->m_szMx_opt);
				xprintf("[CH:%03d]   MX_NAME: %s", ch, pScenario->m_szMx_name);
			}
			pScenario->m_AdoDb->RSClose();
		}

		pScenario->m_bDnisInfo = 1; // 성공
	}
	else {
		pScenario->m_bDnisInfo = 0;
		xprintf("[CH:%03d] PL_InfoOrderReq_Process: DB 저장 실패", ch);
	}

	Wow_REQ_Quithostio("PL_InfoOrderReq_Process SUCCESS", ch);
	xprintf("[CH:%03d] PL_InfoOrderReq_Process END", ch);

	_endthreadex((unsigned int)(*port)[ch].m_hThread);
	return 0;
}

// 신규 API 호출 Host 함수
int getOrderInfo_NewAPI_host(int holdm)
{
	// 초기화
	((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->m_DBAccess = 0;
	if (holdm != 0) {
		if (new_guide) new_guide();
		(*lpmt)->trials = 0;
		(*lpmt)->Hmusic = HM_LOOP;
		if (set_guide) set_guide(holdm);
		if (send_guide) send_guide(NODTMF);
	}

	if (((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->m_AdoDb != NULL) {
		((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->m_DBAccess = -1;
		(*port)[(*lpmt)->chanID].ppftbl[POST_NET].postcode = HI_OK;
		Wow_REQ_Quithostio("getOrderInfo_NewAPI_host line service is Load Error", (*lpmt)->chanID);
		return 0;
	}

	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;

	((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->m_hThread =
		(HANDLE)_beginthreadex(NULL, 0, PL_InfoOrderReq_Process, (LPVOID)(*lpmt), 0,
			&(((CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario))->threadID));

	return 0;
}

// TTS 안내 문구 생성 함수
// 쿠폰/보너스캐시 적용 정보를 포함한 동적 TTS 생성
CString GeneratePaymentTTS(CALLAT_Hangung_Quick_Scenario *pScenario)
{
	CString tts;

	// 고객명(필명) + 상품명
	tts.Format("%s 고객님, 주문하신 %s의 ",
			   pScenario->m_szpsrtner_nm, pScenario->m_szCC_Prod_Desc);

	// 할인 정보가 있는 경우
	BOOL bHasCoupon = (strcmp(pScenario->m_szCouponUseFlag, "Y") == 0);
	BOOL bHasBonus = (strcmp(pScenario->m_szBonusCashUseFlag, "Y") == 0);

	if (bHasCoupon || bHasBonus) {
		CString discount;
		discount.Format("%s께서 보유하신 ", pScenario->m_szpsrtner_nm);
		tts += discount;

		if (bHasCoupon) {
			CString coupon;
			coupon.Format("%s 쿠폰", pScenario->m_szCouponName);
			tts += coupon;
			if (bHasBonus) {
				tts += "과 ";
			}
		}

		if (bHasBonus) {
			CString bonus;
			bonus.Format("보너스 캐시 %d원", pScenario->m_nBonusCashUseAmt);
			tts += bonus;
		}
		tts += "이 적용되어 ";
	}

	// 최종 결제 금액
	CString amount;
	amount.Format("최종 결제 금액은 %d원입니다.", pScenario->m_nAmount);
	tts += amount;

	return tts;
}
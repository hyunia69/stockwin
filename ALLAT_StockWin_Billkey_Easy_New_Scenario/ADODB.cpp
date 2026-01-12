// ADODB.cpp: implementation of the CADODB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommonDef.H"
#include "ALLATCommom.h"

#include    "WowTvSocket.h"
#include    "ADODB.h"
#include    "Scenaio.h"
#include    "AllatUtil.h"
#include    "ALLAT_StockWin_Billkey_Easy_New_Scenario.h"

extern void(*eprintf)(const char *str, ...);
extern void(*xprintf)(const char *str, ...);
extern LPMTP **lpmt , **port;

//LPMTP	*curyport=NULL;
extern void(*info_printf)(int chan, const char *str, ...) ;
extern void(*new_guide)(void) ;
extern int(*set_guide)(int vid, ...);
extern void(*setPostfunc)(int type, int(*func)(int), int poststate, int wtime);
extern int(*send_guide)(int mode);
extern int(*goto_hookon)(void);
extern int(*check_validform)(char *form, char *data);
extern int(*send_error)(void);
extern int(*check_validdtmf)(int c, char *vkeys);
extern int(*in_multifunc)(int chan);
extern int  (*quitchan)(int chan);

extern int(*atoi2)(char *p);
extern int(*atoi3)(char *p);
extern int(*atoi4)(char *p);
extern int(*atoiN)(char *p, int n);
extern long(*atolN)(char *p, int n);
extern int(*atox)(char *s);

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

#define	PARAINI		".\\ALLAT_WOWTV_Billkey_Easy_Scenario_para.ini"
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BOOL ReadFileInternet(int ch, char *szBuffer, DWORD dSize, DWORD *pdwRead)
{
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*port)[ch].pScenario);
	DWORD dReadSize = 0;
	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > ReadFileInternet Calling", ch);
	if (szBuffer == NULL)
	{
		xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > ReadFileInternet > szBuffer=NULL", ch);

		*pdwRead = 0;
		return FALSE;
	}
	memset(szBuffer, 0x00, dSize);
	__try
	{
		xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > ReadFileInternet > InternetQueryDataAvailable ", ch);
		if (!InternetQueryDataAvailable(pScenario->m_hObject, &dReadSize, 0, 0))
		{
			*pdwRead = 0;
			return FALSE;
		}
		xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > ReadFileInternet > InternetQueryDataAvailable[%d] ", ch, dReadSize);

		//INTERNET_BUFFERS inetBuffer;
		//memset(&inetBuffer, 0, sizeof(inetBuffer));
		//inetBuffer.dwStructSize = sizeof(INTERNET_BUFFERS);
		//inetBuffer.Next = NULL;
		//inetBuffer.lpvBuffer = szBuffer;
		//inetBuffer.dwBufferLength = dSize;

		//DWORD dwContent = 1; // cannot be zero for async

		if (!::InternetReadFile(pScenario->m_hObject, szBuffer, dSize>dReadSize ? dReadSize:(dSize - 1), pdwRead))
		{
			*pdwRead = 0;
			xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > ReadFileInternet > InternetReadFile:FALSE[%d] ", ch, *pdwRead);

			return FALSE;
		}
		szBuffer[*pdwRead] = 0x00;
		xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > ReadFileInternet > m_hObject[%s,%d] ", ch, szBuffer, *pdwRead);
	}
	__except (GetExceptionCode() == EXCEPTION_BREAKPOINT ?
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > ReadFileInternet Excepted of Memory ", ch);
		return FALSE;
	}
	if (*pdwRead>0) xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > ReadFileInternet [%s]", ch, szBuffer);
	return TRUE;
}

void HttpQuithostio(char *p, int ch)
{
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*port)[ch].pScenario);

	xprintf("[CH:%03d] HttpQuithostio===START", ch);

	BOOL bRet;


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

	(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
	if ((*port)[ch].used != L_IDLE && in_multifunc(ch))
		quitchan(ch);
	if (pScenario->m_hThread)
	{
		::CloseHandle(pScenario->m_hThread);
		pScenario->m_hThread = NULL;
	}
	

	xprintf("[CH:%03d] %s", ch, p);
	xprintf("[CH:%03d] HttpQuithostio _endthread", ch);
}


/////////////////////////////////////////////////////////////////////
// Http_SSL_RetPageSend : 지정한 URL로 
// char * szURL : URL : GET 방식 용으로 기본 주소와 인자를 합치 한다.
// char * szPostData :  POST 인 경우 인자1=값1 & 인자 2= 값2....
//                      
/////////////////////////////////////////////////////////////////////

int Http_SSL_RetPageSend(void *data , char * szURL, char * szPostData)
{
	int			ch;
	int			threadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = pScenario->threadID;

	CString aString;
	aString = "";
	DWORD dwServiceType;
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort;

	if (szURL == NULL || szPostData == NULL)
	{
		pScenario->m_RetCode = -1;
		xprintf("[CH:%03d] Http_SSL_RetPageSend URL 주소가 없음 NULL", ch);
		return NULL;
	}

	if (!AfxParseURL(szURL, dwServiceType, strServerName, strObject, nPort) ||
		(dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS))
	{
		pScenario->m_RetCode = -1;
		xprintf("[CH:%03d] Http_SSL_RetPageSend초기화 실패", ch);
		return -1;
	}

	/*
	#define INTERNET_OPEN_TYPE_PRECONFIG                    0   // use registry configuration(기본)
	#define INTERNET_OPEN_TYPE_DIRECT                       1   // direct to net
	#define INTERNET_OPEN_TYPE_PROXY                        3   // via named proxy
	#define INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY  4   // prevent using java/script/INS
	*/

	xprintf("[CH:%03d] Http_SSL_RetPageSendSTART", ch);
	xprintf("[CH:%03d] Http_SSL_RetPageSend(szURL:%s), %d", ch, szURL, strlen(szURL));

	char szError[1025] = { 0x00, };
	try
	{
		xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetOpen ", ch);
		pScenario->m_hSession = ::InternetOpen("AllatWowTvWEbReq", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	}
	catch (CInternetException *e)
	{
		memset(szError, 0x00, sizeof(szError));
		xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetOpen[ Error Code : 0x%x, %s ]", ch,
			e->m_dwError, e->GetErrorMessage(szError,1024));
		e->Delete();
		pScenario->m_RetCode = -1;
		return 0;
	}
	
	// 첫 인자에 Test는 넣지 말라!..
	//해당 사이트에서 이를 검증 하는 듯 하다..
	// 전문 전송 후 리턴 시 값을 가지고 오지 못한다.
	if (pScenario->m_hSession)
	{
		// crack url..
		char szExtraInfo[MAX_PATH * 4] = { 0x00, };
		char szHostName[MAX_PATH];
		char szPassword[MAX_PATH];
		char szScheme[MAX_PATH];
		char szUrlPath[MAX_PATH * 4];
		char szUserName[MAX_PATH];

		URL_COMPONENTS      urlcomponent;
		urlcomponent.dwExtraInfoLength = MAX_PATH;
		urlcomponent.dwHostNameLength = MAX_PATH;
		urlcomponent.dwPasswordLength = MAX_PATH;
		urlcomponent.dwSchemeLength = MAX_PATH;
		urlcomponent.dwStructSize = sizeof(URL_COMPONENTS);
		urlcomponent.dwUrlPathLength = MAX_PATH * 4;
		urlcomponent.dwUserNameLength = MAX_PATH;
		urlcomponent.lpszExtraInfo = szExtraInfo;
		urlcomponent.lpszHostName = szHostName;
		urlcomponent.lpszPassword = szPassword;
		urlcomponent.lpszScheme = szScheme;
		urlcomponent.lpszUrlPath = szUrlPath;
		urlcomponent.lpszUserName = szUserName;
		urlcomponent.nPort = 0;
		//urlcomponent.nScheme = 0;
		try
		{
			xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetCrackUrl ", ch);
			::InternetCrackUrl(szURL,  strlen(szURL) /*sizeof(URL_COMPONENTS)*/, NULL, &urlcomponent);
		}
		catch (CInternetException *e)
		{
			memset(szError, 0x00, sizeof(szError));
			xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetCrackUrl[ Error Code : 0x%x, %s ]", ch,
				e->m_dwError, e->GetErrorMessage(szError,1024));
			e->Delete();
			pScenario->m_RetCode = -1;
			return 0;
		}
		// download user
		try
		{
			xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetConnect ", ch);
			pScenario->m_hConnect = ::InternetConnect(pScenario->m_hSession,
				urlcomponent.lpszHostName,
				urlcomponent.nPort,
				urlcomponent.lpszUserName,
				urlcomponent.lpszPassword,
				INTERNET_SERVICE_HTTP,
				0,/*INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE,*/
				0);
		}
		catch (CInternetException *e)
		{
			xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetConnect[ Error Code : 0x%x, %s ]",ch,
				e->m_dwError, e->GetErrorMessage(szError, 1024));
			e->Delete();
			pScenario->m_RetCode = -1;
			return 0;
		}
		if (pScenario->m_hConnect)
		{
			// connected server..
			if (strlen(urlcomponent.lpszExtraInfo) < 1)
			{
				memcpy(urlcomponent.lpszExtraInfo, strstr(szURL, "?"), strlen(strstr(szURL, "?")));
			}

			char PostData[5] = { 0x00, };
			memcpy(PostData, szPostData, strlen(szPostData));
			_strupr_s(PostData, sizeof(PostData));
			if (strcmp(PostData, "GET") != 0 && strcmp(PostData, "POST") != 0)
			{
				pScenario->m_RetCode = -1;
				xprintf("[CH:%03d] Http_SSL_RetPageSendPostData POST 또는  GET 이어야 합니다.", ch);
				return -1;
			}
			char GetPath[MAX_PATH * 4] = { 0x00, };

			DWORD dwFlags = 0;
			//만약 https ssl 통신이 필요하다면 INTERNET_FLAG_SECURE 플래그 추가
			if (INTERNET_SCHEME_HTTP == urlcomponent.nScheme)
			{	// HTTP
				dwFlags = INTERNET_FLAG_RELOAD
					| INTERNET_FLAG_DONT_CACHE
					| INTERNET_FLAG_NO_AUTO_REDIRECT;
			}
			else if (INTERNET_SCHEME_HTTPS == urlcomponent.nScheme)
			{	// HTTPS
				dwFlags = INTERNET_FLAG_RELOAD
					| INTERNET_FLAG_DONT_CACHE
					| INTERNET_FLAG_NO_AUTO_REDIRECT
					| INTERNET_FLAG_SECURE
					| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID
					| INTERNET_FLAG_IGNORE_CERT_CN_INVALID;
			}

			/*if (dwServiceType == AFX_INET_SERVICE_HTTPS)
			{
			dwFlags = INTERNET_FLAG_RELOAD
			| INTERNET_FLAG_DONT_CACHE
			| INTERNET_FLAG_NO_AUTO_REDIRECT
			| INTERNET_FLAG_SECURE
			| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID
			| INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

			}
			else
			{
			dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_AUTO_REDIRECT;
			}*/

			if (strcmp(PostData, "GET") == 0)
			{
				sprintf_s(GetPath, sizeof(GetPath), "%s%s", urlcomponent.lpszUrlPath, urlcomponent.lpszExtraInfo);
			}
			else
				sprintf_s(GetPath, sizeof(GetPath), "%s", urlcomponent.lpszUrlPath);

			LPCTSTR ppszAcceptTypes[2];
			ppszAcceptTypes[0] = _T("*/*");
			ppszAcceptTypes[1] = NULL;

			try
			{
				xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetAttemptConnect ", ch);
				if (::InternetAttemptConnect(NULL) != ERROR_SUCCESS)
				{
					pScenario->m_RetCode = -1;
					xprintf("[CH:%03d] Http_SSL_RetPageSend InternetAttemptConnect 오류. ", ch);
					return -1;
				}
			}
			catch (CInternetException *e)
			{
				memset(szError, 0x00, sizeof(szError));
				xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetAttemptConnect[ Error Code : 0x%x, %s ]", ch,
					e->m_dwError, e->GetErrorMessage(szError,1024));
				e->Delete();
				pScenario->m_RetCode = -1;
				return 0;
			}
			try
			{
				xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > HttpOpenRequest ", ch);
				pScenario->m_hObject = ::HttpOpenRequest(pScenario->m_hConnect,
					PostData,
					GetPath,
					NULL,
					NULL,
					ppszAcceptTypes,
					dwFlags,
					NULL);
			}
			catch (CInternetException *e)
			{
				memset(szError, 0x00, sizeof(szError));
				xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > HttpOpenRequest[ Error Code : 0x%x, %s ]", ch,
					e->m_dwError, e->GetErrorMessage(szError,1024));
				e->Delete();
				pScenario->m_RetCode = -1;
				return 0;
			}
			if (pScenario->m_hObject)
			{
				// post header
				char szLen[MAX_PATH];
				CString aHeader;
				aHeader = "";

				DWORD dwFlags = 0;
				DWORD dwBuffLen = sizeof(dwFlags);
				xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetQueryOption ", ch);
				if (!InternetQueryOption(pScenario->m_hObject, INTERNET_OPTION_SECURITY_FLAGS,
					(LPVOID)&dwFlags, &dwBuffLen))
				{
					xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetQueryOption  오류: 0x%x", ch, ::GetLastError());
				}
				if (dwServiceType == AFX_INET_SERVICE_HTTPS)
				{

					dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
						SECURITY_FLAG_IGNORE_REVOCATION |

						SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS |
						SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
						SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
				}
				else
				{
					dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
						SECURITY_FLAG_IGNORE_REVOCATION |
						SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP |
						SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
						SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
				}

				DWORD dwVal = 30;
				try
				{
					//전체 지정이라 핸들을 지정하지 않음
					xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetSetOption 접속 대상 서버의 개수 지정 ", ch);
					if (!InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &dwVal, sizeof(dwVal)))
					{
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetSetOption(HTTP1.0) 접속 대상 서버의 개수 지정 오류 (0x%X,%d)", ch, ::GetLastError(), dwVal);

					}
					if (!InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &dwVal, sizeof(dwVal)))
					{
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetSetOption(HTTP1.1) 접속 대상 서버의 개수 지정 오류 (0x%X,%d)", ch, ::GetLastError(), dwVal);
					}
					xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetSetOption SECURITY_FLAGS 지정 ", ch);
					BOOL ret = InternetSetOption(pScenario->m_hObject, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
					if (ret)
					{
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetSetOption  오류: 0x%x [dwBuffLen:%d, sizeof(dwFlags):%d]", ch, ::GetLastError(), dwBuffLen, sizeof(dwFlags));
					}
				}
				catch (CInternetException *e)
				{
					memset(szError, 0x00, sizeof(szError));
					xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetSetOption[ Error Code : 0x%x, %s ]", ch,
						e->m_dwError, e->GetErrorMessage(szError,1024));
					e->Delete();
					pScenario->m_RetCode = -1;
					return 0;
				}

				wsprintf(szLen, "%d", (PostData == NULL || strcmp(PostData, "GET") == 0) ? 0 : strlen(&urlcomponent.lpszExtraInfo[1]));
				aHeader += _T("User-Agent: Mozilla/4.0 (compatible; MSIE 5.0;* Windows NT)\r\n");
				aHeader += _T("Content-Type: application/x-www-form-urlencoded\r\n");
				aHeader += _T("Content-length: ");
				aHeader += szLen;
				aHeader += _T("\r\n\n");

				CString cstLen;


				cstLen.Format("%s, %d", szLen, strlen(szURL));
				// post data..
				if (strcmp(PostData, "POST") == 0)
				{
					try
					{
						xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario{POST] > Http_SSL_RetPageSend > HttpAddRequestHeaders ", ch);
						BOOL ret = ::HttpAddRequestHeaders(pScenario->m_hObject, (LPCTSTR)aHeader.GetBuffer(), -1L, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
					}
					catch (CInternetException *e)
					{
						memset(szError, 0x00, sizeof(szError));
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario[POST] > Http_SSL_RetPageSend > HttpAddRequestHeaders[ Error Code : 0x%x, %s ]", ch,
							e->m_dwError, e->GetErrorMessage(szError,1024));
						e->Delete();
						pScenario->m_RetCode = -1;
						return 0;
					}
					try
					{
						xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario{POST] > Http_SSL_RetPageSend > HttpSendRequest ", ch);
						::HttpSendRequest(pScenario->m_hObject, aHeader, aHeader.GetLength(), (LPVOID)&urlcomponent.lpszExtraInfo[1], strlen(&urlcomponent.lpszExtraInfo[1]));
					}
					catch (CInternetException *e)
					{
						memset(szError, 0x00, sizeof(szError));
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario[POST] > Http_SSL_RetPageSend > HttpSendRequest[ Error Code : 0x%x, %s ]", ch,
							e->m_dwError, e->GetErrorMessage(szError,1024));
						e->Delete();
						pScenario->m_RetCode = -1;
						return 0;
					}					
				}
				else
				{
					try
					{
						xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario{GET} > Http_SSL_RetPageSend > HttpSendRequest ", ch);
						::HttpSendRequest(pScenario->m_hObject, NULL, 0L, NULL, 0);
					}
					catch (CInternetException *e)
					{
						memset(szError, 0x00, sizeof(szError));
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario[GET] > Http_SSL_RetPageSend > HttpSendRequest[ Error Code : 0x%x, %s ]", ch,
							e->m_dwError, e->GetErrorMessage(szError,1024));
						e->Delete();
						pScenario->m_RetCode = -1;
						return 0;
					}
				}

				TCHAR szContentLength[32];
				DWORD dwInfoSize = 32;
				DWORD dwFileSize = 0;
				try
				{
					xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > HttpQueryInfo ", ch);
					if (::HttpQueryInfo(pScenario->m_hObject, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwInfoSize, NULL)){
						dwFileSize = (DWORD)_ttol(szContentLength);
					}
				}
				catch (CInternetException *e)
				{
					memset(szError, 0x00, sizeof(szError));
					xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > HttpQueryInfo[ Error Code : 0x%x, %s ]", ch,
						e->m_dwError, e->GetErrorMessage(szError,1024));
					e->Delete();
					pScenario->m_RetCode = -1;
					return 0;
				}

				// open object
				//char szBuffer[1024];
				char szBuffer[4096 + 1] = { 0x00, };//UTF-8 를 위해서
				DWORD dwRead = 0;
				//DWORD dwWrite = 0;

				pScenario->m_RetCode = 0; 
				memset(pScenario->m_szHttpBuffer, 0x00, sizeof(pScenario->m_szHttpBuffer));
				try
				{
					xprintf("[CH:%03d]ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > ReadFileInternet ", ch);
					while (ReadFileInternet(ch,szBuffer,1023,&dwRead) &&
						dwRead > 0)
					{
						szBuffer[dwRead] = 0;
						aString += szBuffer;
						Sleep(1);
					}
				}
				catch (CInternetException *e)
				{
					memset(szError, 0x00, sizeof(szError));
					xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > InternetReadFile[ Error Code : 0x%x, %s ]", ch,
						e->m_dwError, e->GetErrorMessage(szError,1024));
					e->Delete();
					pScenario->m_RetCode = -1;
					return 0;
				}
			
				if (aString.GetLength() > sizeof(pScenario->m_szHttpBuffer))
				{
					xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > 큰버퍼 : aUtf8Buf.GetBuffer(%s,%d) ", ch, aString.GetBuffer(), aString.GetLength());
					try
					{
						memcpy(pScenario->m_szHttpBuffer, aString.GetBuffer(), sizeof(pScenario->m_szHttpBuffer) - 1);
					}
					catch (CMemoryException * e)
					{
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > 큰버퍼 : 메모리 예외 발생");
						e->Delete();
					}
					catch (CException *e)
					{
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > 큰버퍼 : 예외 발생");
						e->Delete();
					}

				}
				else
				{
					xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > 적절 버퍼 :  aUtf8Buf.GetBuffer(%s,%d) ", ch, aString.GetBuffer(), aString.GetLength());
					try
					{
						memcpy(pScenario->m_szHttpBuffer, aString.GetBuffer(), aString.GetLength());
					}
					catch (CMemoryException * e)
					{
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > 큰버퍼 : 메모리 예외 발생");
						e->Delete();
					}
					catch (CException *e)
					{
						xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend > 큰버퍼 : 예외 발생");
						e->Delete();
					}
				}
				xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend :%s", ch, aString.GetBuffer());

				xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario > Http_SSL_RetPageSend END", ch);
				return 0;
			}
			else
			{
				pScenario->m_RetCode = -1;
				xprintf("[CH:%03d] Http_SSL_RetPageSend URL에 해당 하는 페이지가 없습니다. ", ch);
				return -1;
			}
		}
		else
		{
			pScenario->m_RetCode = -1;
			xprintf("[CH:%03d] Http_SSL_RetPageSend 해당하는 URL에 접속이 안 됩니다.", ch);
			return -1;
		}
	}
	else
	{
		pScenario->m_RetCode = -1;
		xprintf("[CH:%03d] Http_SSL_RetPageSend HTTP&HTTPS 클라이언트 초기화 실패.", ch);
		return -1;
	}
	return 0; 
}

// 결제 응답 전송
unsigned int __stdcall AllatPayRetProcess(void *data)
{

	int			ch;
	int			threadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = pScenario->threadID;

	xprintf("[CH:%03d] AllatPayRetProcess START", ch);

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

	Http_SSL_RetPageSend(data, pScenario->m_szSHOP_RET_URL_AG, "POST"); //221208
	if (pScenario->m_RetCode < 0)
	{
		HttpQuithostio("AllatPayRetProcess 응답 XML 수신 실패", ch);
		return 0;
	}
	HttpQuithostio("AllatPayRetProcess END", ch);
	return 0;
}

int  AllatPayRetPrc_host(int holdm)
{
	//초기화	
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

	if (holdm != 0) {
		new_guide();
		(*lpmt)->trials = 0;
		(*lpmt)->Hmusic = HM_LOOP;

		set_guide(holdm);
		send_guide(NODTMF);
	}
	if (pScenario->m_hThread)
	{
		::WaitForSingleObject(pScenario->m_hThread, 300);
		::CloseHandle(pScenario->m_hThread);
		pScenario->m_hThread = NULL;
	}
	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	pScenario->m_hThread = (HANDLE)_beginthreadex(NULL, 0, AllatPayRetProcess, (LPVOID)(*lpmt), 0, &(pScenario->threadID));

	return(0);
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CADODB::CADODB()
{
	m_RS = NULL;
	m_CMD = NULL;
	m_CONN = NULL;
	SetScenarion(NULL);
}

void CADODB::SetScenarion(CALLAT_WOWTV_Billkey_Easy_Scenario  *pScenario)
{
	m_pScenario = pScenario;
}

CADODB::CADODB(CALLAT_WOWTV_Billkey_Easy_Scenario  *pScenario)
{
    m_RS = NULL;
    m_CMD = NULL;
    m_CONN = NULL;

	SetScenarion(pScenario);
}

CADODB::~CADODB()
{
    
    if(m_RS != NULL)
    {
        if(ISRSCon())
        {
            m_RS->Close();
        }
    }

    if(m_CONN != NULL)
    {
        if(ISOpen())
        {
            m_CONN->Close();
        }
    }
}

BOOL CADODB::DBConnect(char* pWD, char* pID, char* pDataBase, char* pConnectIP)
{
	char strConnectionString[1024]={0x00,};
	strcpy_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), "Provider=SQLOLEDB.1;Password=");
	strcat_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), pWD);
	strcat_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), ";Persist Security Info=True;User ID=");
	strcat_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), pID);
	strcat_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), ";Initial Catalog=");
	strcat_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), pDataBase);
	strcat_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), ";Data Source=");
	strcat_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), pConnectIP);
	strcat_s(strConnectionString,sizeof(strConnectionString)-strlen(strConnectionString), "; Network Library = dbmssocn");
    
    m_CONN.CreateInstance("ADODB.Connection");
    m_CONN->ConnectionString = strConnectionString;
  //HRESULT hr = m_CONN->Open(strConnectionString, "", "", -1);
    HRESULT hr = m_CONN->Open("", "", "", -1);

    if(SUCCEEDED(hr))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CADODB::ISRSCon()
{
    return ((m_RS->GetState() == adStateOpen ) ? TRUE : FALSE);
}

BOOL CADODB::ISOpen()
{
    return ((m_CONN->GetState() == adStateOpen ) ? TRUE : FALSE);
}

BOOL CADODB::GetDBCon()
{
    return ISOpen();
}

void CADODB::ConClose()
{
    if(ISOpen())
    {
        m_CONN->Close();
    }
}

long CADODB::ConBeginTrans()
{
    return m_CONN->BeginTrans();
}

void CADODB::ConCommitTrans()
{
    m_CONN->CommitTrans();
}

void CADODB::ConRollbackTrans()
{
    m_CONN->RollbackTrans();
}

void CADODB::ConCancel()
{
    m_CONN->Cancel();
}

BOOL CADODB::IsEOF()
{
    return m_RS->adoEOF;
}


BOOL CADODB::Next()
{
    return (FAILED(m_RS->MoveNext()) ? FALSE : TRUE);
}

BOOL CADODB::Prev()
{
    return (FAILED(m_RS->MovePrevious()) ? FALSE : TRUE);
}

BOOL CADODB::First()
{
    return (FAILED(m_RS->MoveFirst()) ? FALSE : TRUE);
}

BOOL CADODB::Last()
{
    return (FAILED(m_RS->MoveLast()) ? FALSE : TRUE);
}

int CADODB::GetRecCount()
{
	HRESULT hr;
	ASSERT(m_RS != NULL);
	try
	{
		int count = (int)m_RS->GetRecordCount();

		if (count > 0)
		{
			hr = m_RS->MoveFirst();
			if (!SUCCEEDED(hr))
			{
				return -1;
			}
			count = 0;
			while (!m_RS->adoEOF)
			{
				count++;
				hr = m_RS->MoveNext();
				if (!SUCCEEDED(hr))
				{
					return count;
				}

			}

			if (m_RS->adoEOF)
			{
				hr = m_RS->MoveFirst();
				if (!SUCCEEDED(hr))
				{
					return -1;
				}
			}

			xprintf("SUCCESS: GetRecordCount  %d:\n", count);
		}
		else
			xprintf("Warning: GetRecordCount  %d; File: %s; Line: %d\n", count, __FILE__, __LINE__);
		return count;
	}
	catch (_com_error e)
	{
		xprintf("Warning: GetRecordCount ErrorMessage: %s; File: %s; Line: %d\n", e.ErrorMessage(), __FILE__, __LINE__);
		PrintProviderError();
		PrintComError(e);
		return -1;
	}
}

int CADODB::GetFieldCount()
{
    return (int)m_RS->Fields->GetCount();
}

void CADODB::RSClose()
{
    if(ISRSCon())
    {
        m_RS->Close();
    }
}

int CADODB::GetRs(_variant_t x, _bstr_t& ret)
{
	
	try {
		_variant_t vtNull;
		vtNull.ChangeType(VT_NULL);
		ret = (m_RS->Fields->Item[x]->Value != vtNull) ? m_RS->Fields->Item[x]->Value : "";
	}
	catch (_com_error e)
	{
		_bstr_t bstrColName = x;
		LPCSTR strColName = bstrColName;
		xprintf("[CH:%03d] WARNING:(%s)", m_pScenario->nChan, strColName);

		PrintProviderError();
		PrintComWARNING(e);
		
		ret = "";
		return -1;
	}
	return 0;
}


int CADODB::GetRs(_variant_t x, _variant_t& ret)
{
	
	try {
		_variant_t vtNull;
		vtNull.ChangeType(VT_NULL);
		ret = (m_RS->Fields->Item[x]->Value != vtNull) ? m_RS->Fields->Item[x]->Value : "";
	}
	catch (_com_error e)
	{
		_bstr_t bstrColName = x;
		LPCSTR strColName = bstrColName;
		xprintf("[CH:%03d] WARNING:(%s)", m_pScenario->nChan, strColName);

		PrintProviderError();
		PrintComWARNING(e);
		ret = "";
		return -1;
	}
	return 0;
}


int CADODB::GetRs(_variant_t x, float& ret)
{
	try {
		_variant_t vtNull;
		vtNull.ChangeType(VT_NULL);
		ret = (m_RS->Fields->Item[x]->Value != vtNull) ? m_RS->Fields->Item[x]->Value : 0.0;
	}
	catch (_com_error e)
	{
		_bstr_t bstrColName = x;
		LPCSTR strColName = bstrColName;
		xprintf("[CH:%03d] WARNING:(%s)", m_pScenario->nChan, strColName);

		PrintProviderError();
		PrintComWARNING(e);
		ret = 0;
		return -1;
	}
	return 0;
}


int CADODB::GetRs(_variant_t x, long& ret)
{
	try {
		_variant_t vtNull;
		vtNull.ChangeType(VT_NULL);
		ret = (m_RS->Fields->Item[x]->Value != vtNull) ? m_RS->Fields->Item[x]->Value : 0;
	}
	catch (_com_error e)
	{
		_bstr_t bstrColName = x;
		LPCSTR strColName = bstrColName;
		xprintf("[CH:%03d] WARNING:(%s)", m_pScenario->nChan, strColName);

		PrintProviderError();
		PrintComWARNING(e);
		ret = 0;
		return -1;
	}
	return 0;
}


int CADODB::GetRs(_variant_t x, double& ret)
{
	try {
		_variant_t vtNull;
		vtNull.ChangeType(VT_NULL);
		ret = (m_RS->Fields->Item[x]->Value != vtNull) ? m_RS->Fields->Item[x]->Value : 0;
	}
	catch (_com_error e)
	{
		_bstr_t bstrColName = x;
		LPCSTR strColName = bstrColName;
		xprintf("[CH:%03d] WARNING:(%s)", m_pScenario->nChan, strColName);

		PrintProviderError();
		PrintComWARNING(e);
		ret = 0;
		return -1;
	}
	return 0;
}

BOOL CADODB::Open(char* pSourceBuf, long option)
{
    if(ISOpen())
    {
        m_RS.CreateInstance(__uuidof(Recordset));
        m_RS->PutRefActiveConnection(m_CONN);
        
        HRESULT hr;
        m_RS->CursorType = adOpenStatic;
        hr = m_RS->Open(pSourceBuf, (IDispatch *)m_CONN, adOpenStatic, adLockOptimistic, option);
        
        if(SUCCEEDED(hr))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

BOOL CADODB::Excute(char* pSourceBuf, long option)
{
    if(ISOpen())
    {
        m_CMD.CreateInstance(__uuidof(Command));
        m_CMD->ActiveConnection = m_CONN;
        m_CMD->CommandText = pSourceBuf;
        m_CMD->Execute(NULL, NULL, adCmdText);
        
        m_RS.CreateInstance(__uuidof(Recordset));
        m_RS->PutRefSource(m_CMD);
        
        _variant_t vNull;
        vNull.vt = VT_ERROR;
        vNull.scode = DISP_E_PARAMNOTFOUND;
        m_RS->CursorLocation = adUseClient;
        m_RS->Open(vNull, vNull, adOpenDynamic, adLockOptimistic, adCmdUnknown);
        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


///////////////////////////////////////////////////////////
//                                                       //
//      PrintProviderError Function                      //
//                                                       //
///////////////////////////////////////////////////////////
void CADODB::PrintProviderError()
{
	// Print Provider Errors from Connection object.
	// pErr is a record object in the Connection's Error collection.
	ErrorPtr  pErr = NULL;

	if ((m_CONN->Errors->Count) > 0)
	{
		long nCount = m_CONN->Errors->Count;

		// Collection ranges from 0 to nCount -1.
		for (long i = 0; i < nCount; i++)
		{
			pErr = m_CONN->Errors->GetItem(i);
			m_pScenario != NULL ? xprintf("[CH:%03d] Error number: %x\t%s\n", m_pScenario->nChan, pErr->Number, (LPCSTR)pErr->Description) :
				xprintf("Error number: %x\t%s\n", pErr->Number, (LPCSTR)pErr->Description);
		}
	}
}

///////////////////////////////////////////////////////////
//                                                       //
//      PrintComError Function                           //
//                                                       //
///////////////////////////////////////////////////////////
void CADODB::PrintComError(_com_error &e)
{
	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());

	// Print Com errors.
	m_pScenario != NULL ? xprintf("[CH:%03d] Error", m_pScenario->nChan) : xprintf("Error");
	m_pScenario != NULL ? xprintf("[CH:%03d] \tCode = %08lx", m_pScenario->nChan, e.Error()) : xprintf("\tCode = %08lx",  e.Error());
	m_pScenario != NULL ? xprintf("[CH:%03d] \tCode meaning = %s", m_pScenario->nChan, e.ErrorMessage()) : xprintf("\tCode meaning = %s", e.ErrorMessage());
	m_pScenario != NULL ? xprintf("[CH:%03d] \tSource = %s", m_pScenario->nChan,  (LPCSTR)bstrSource) : xprintf("\tSource = %s", (LPCSTR)bstrSource);
	m_pScenario != NULL ? xprintf("[CH:%03d] \tDescription = %s", m_pScenario->nChan, (LPCSTR)bstrDescription) : xprintf("\tDescription = %s", (LPCSTR)bstrDescription);
}

///////////////////////////////////////////////////////////
//                                                       //
//      PrintComError Function                           //
//                                                       //
///////////////////////////////////////////////////////////
void CADODB::PrintComWARNING(_com_error &e)
{
	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());

	// Print Com errors.
	m_pScenario != NULL ? xprintf("[CH:%03d] WARNING", m_pScenario->nChan) : xprintf("Error");
	m_pScenario != NULL ? xprintf("[CH:%03d] \tWARNING_Code = %08lx", m_pScenario->nChan, e.Error()) : xprintf("\tCode = %08lx", e.Error());
	m_pScenario != NULL ? xprintf("[CH:%03d] \tWARNING_Code meaning = %s", m_pScenario->nChan, e.ErrorMessage()) : xprintf("\tCode meaning = %s", e.ErrorMessage());
	m_pScenario != NULL ? xprintf("[CH:%03d] \tWARNING_Source = %s", m_pScenario->nChan, (LPCSTR)bstrSource) : xprintf("\tSource = %s", (LPCSTR)bstrSource);
	m_pScenario != NULL ? xprintf("[CH:%03d] \tDWARNING_escription = %s", m_pScenario->nChan, (LPCSTR)bstrDescription) : xprintf("\tDescription = %s", (LPCSTR)bstrDescription);
}

BOOL CADODB::upOrderPayState(char *sxResultCode, char *szResultMsg, char *szMoid , char *szMid)
{
	if (ISOpen())
	{
		_CommandPtr& cmd = m_CMD;
		cmd.CreateInstance("ADODB.Command");
		CString strSql;

		char szCALLBACK_NO[15 + 1] = { 0x00, };
		CString strCALLBACK_NO;

		GetPrivateProfileString("SMS_SEND", "CALLBACK_NO", CALLBACK_NO, szCALLBACK_NO, sizeof(szCALLBACK_NO), PARAINI);
		strCALLBACK_NO = szCALLBACK_NO;

		//"MX_SEQ" INT NOT NULL DEFAULT NULL,

		if (strlen(m_pScenario->m_CardResInfo.szBill_Key))
		{
			strSql.Format("UPDATE ALLAT_SHOP_ORDER \n"
				"SET reply_code = \'%s\'\n"
				"  , reply_message = \'%s\' \n"
				"  , payment_code = \'1\' \n"
				"WHERE mx_issue_no = \'%s\'\n"
				"AND mx_id = \'%s\'\n"
				"AND mx_id2 = \'%s\';"
				, sxResultCode, szResultMsg, szMoid, szMid, m_pScenario->m_szMx_id2);
		}
		else
		{
			strSql.Format("UPDATE ALLAT_SHOP_ORDER \n"
				"SET reply_code = \'%s\'\n"
				"  , reply_message = \'%s\' \n"
				"  , payment_code = \'1\' \n"
				"WHERE mx_issue_no = \'%s\'\n"
				"AND mx_id2 = \'%s\'\n"
				"AND mx_id = \'%s\';"
				, sxResultCode, szResultMsg, szMoid, szMid, m_pScenario->m_szMx_id1);
		}

		cmd->CommandText = strSql.AllocSysString();
		cmd->CommandType = adCmdText;

		cmd->ActiveConnection = m_CONN;
		try{
			cmd->Execute(NULL, NULL, adCmdText);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			return FALSE;
		}
		//*Return_INPUT_Error_NUM = long(cmd->Parameters->Item[L"@DB_ERR"]->Value);
		//*Return_iMageNUM = long(cmd->Parameters->Item[L"@iMageNUM_1"]->Value);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


BOOL CADODB::setPayLog(Card_ResInfo ag_Card_ResInfo)
{
	if (ISOpen())
	{
		_CommandPtr& cmd = m_CMD;
		cmd.CreateInstance("ADODB.Command");
		CString strSql;
		
		//"MX_SEQ" INT NOT NULL DEFAULT NULL,
		strSql.Format("insert into ALLAT_PAY_LOG\n"
			"(MX_ISSUE_NO,MX_ISSUE_DATE , AMOUNT, INSTALLMENT, REPLY_CODE, REPLY_MESSAGE, REPLY_DATE,PAYMENT_TYPE,MX_ID)\n"
			"values(\'%s\', \'%s\', \'%s\' , \'%s\', \'%s\' , \'%s\', getdate(),\'%s\',\'%s\');"
			, TEXT(ag_Card_ResInfo.m_szMoid), TEXT(ag_Card_ResInfo.MX_ISSUE_DATE), 
			TEXT(ag_Card_ResInfo.m_AMT), TEXT(ag_Card_ResInfo.m_szCardQuota)
			, TEXT(ag_Card_ResInfo.m_szRESULTCODE), TEXT(ag_Card_ResInfo.m_szRESULTMSG), "1", TEXT(ag_Card_ResInfo.m_szMid));


		cmd->CommandText = strSql.AllocSysString();
		cmd->CommandType = adCmdText;

		cmd->ActiveConnection = m_CONN;
		try{
			cmd->Execute(NULL, NULL, adCmdText);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			return FALSE;
		}
		//*Return_INPUT_Error_NUM = long(cmd->Parameters->Item[L"@DB_ERR"]->Value);
		//*Return_iMageNUM = long(cmd->Parameters->Item[L"@iMageNUM_1"]->Value);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int CADODB::sp_getAllatOrderInfoBySMS2(CString szDnis, CString szAuthNo)
{
	if (ISOpen())
	{
		_ParameterPtr pParam;
		_CommandPtr& cmd = m_CMD;

		//2016.05.23
		//try 위치 및 범위 넓게 하자!!!
		try{
			cmd.CreateInstance("ADODB.Command");
			cmd->CommandText = "dbo.sp_getAllatOrderInfoBySMS2";
			cmd->CommandType = adCmdStoredProc;

			pParam.CreateInstance("ADODB.Parameter");
			pParam->Name = L"@AUTH_NO";
			pParam->Type = adVarChar;
			pParam->Size = 12;
			pParam->Direction = adParamInput;
			cmd->Parameters->Append(pParam);
			cmd->Parameters->Item[L"@AUTH_NO"]->Value = _variant_t(szAuthNo.GetBuffer(pParam->Size));// 주문 번호


			pParam.CreateInstance("ADODB.Parameter");
			pParam->Name = L"@DNIS";
			pParam->Type = adVarChar;
			pParam->Size = 12;
			pParam->Direction = adParamInput;
			cmd->Parameters->Append(pParam);
			cmd->Parameters->Item[L"@DNIS"]->Value = _variant_t(szDnis.GetBuffer(pParam->Size));

			cmd->ActiveConnection = m_CONN;
		//try{
			cmd->Execute(NULL, NULL, adCmdStoredProc);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			return FALSE;
		}
		m_RS.CreateInstance(__uuidof(Recordset));
		m_RS->PutRefSource(cmd);

		_variant_t vNull;
		vNull.vt = VT_ERROR;
		vNull.scode = DISP_E_PARAMNOTFOUND;
		m_RS->CursorLocation = adUseClient;

		try{
			m_RS->Open(vNull, vNull, adOpenStatic, adLockOptimistic, adCmdStoredProc);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			return FALSE;
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int CADODB::sp_getAllatOrderInfoByTel2(CString szDnis, CString szInputTelNum)
{
	if (ISOpen())
	{
		_ParameterPtr pParam;
		_CommandPtr& cmd = m_CMD;

		//2016.05.23
		//try 위치 및 범위 넓게 하자!!!
		try{
			cmd.CreateInstance("ADODB.Command");
			cmd->CommandText = "dbo.sp_getAllatOrderInfoByTel2";
			cmd->CommandType = adCmdStoredProc;

			pParam.CreateInstance("ADODB.Parameter");
			pParam->Name = L"@PHONE_NO";
			pParam->Type = adVarChar;
			pParam->Size = 32;//12자리에서 32 자리로 수정 2016.11.29 인입호가 해외의 번호의 것을 수용하기 위해 수정
			pParam->Direction = adParamInput;
			cmd->Parameters->Append(pParam);
			cmd->Parameters->Item[L"@PHONE_NO"]->Value = _variant_t(szInputTelNum.GetBuffer(pParam->Size));// 전화 번호

			pParam.CreateInstance("ADODB.Parameter");
			pParam->Name = L"@DNIS";
			pParam->Type = adVarChar;
			pParam->Size = 12;
			pParam->Direction = adParamInput;
			cmd->Parameters->Append(pParam);
			cmd->Parameters->Item[L"@DNIS"]->Value = _variant_t(szDnis.GetBuffer(pParam->Size));

			cmd->ActiveConnection = m_CONN;
		//try{
			cmd->Execute(NULL, NULL, adCmdStoredProc);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			return FALSE;
		}
		m_RS.CreateInstance(__uuidof(Recordset));
		m_RS->PutRefSource(cmd);

		_variant_t vNull;
		vNull.vt = VT_ERROR;
		vNull.scode = DISP_E_PARAMNOTFOUND;
		m_RS->CursorLocation = adUseClient;

		try{
			m_RS->Open(vNull, vNull, adOpenStatic, adLockOptimistic, adCmdStoredProc);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			return FALSE;
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


int CADODB::sp_getAllatOrderInfoByTel4(CString szDnis, CString szInputTelNum, CString szShopId1, CString szShopId2)
{
	if (ISOpen())
	{
		_ParameterPtr pParam;
		_CommandPtr& cmd = m_CMD;
		xprintf("[CH:%03d] sp_getAllatOrderInfoByTel4 START", m_pScenario->nChan);
		//2016.05.23
		//try 위치 및 범위 넓게 하자!!!
		try{
			cmd.CreateInstance("ADODB.Command");
			cmd->CommandText = "dbo.sp_getAllatOrderInfoByTel4";
			cmd->CommandType = adCmdStoredProc;

			pParam.CreateInstance("ADODB.Parameter");
			pParam->Name = L"@PHONE_NO";
			pParam->Type = adVarChar;
			pParam->Size = 32;//12자리에서 32 자리로 수정 2016.11.29 인입호가 해외의 번호의 것을 수용하기 위해 수정
			pParam->Direction = adParamInput;
			cmd->Parameters->Append(pParam);
			cmd->Parameters->Item[L"@PHONE_NO"]->Value = _variant_t(szInputTelNum.GetBuffer(pParam->Size));// 전화 번호

			pParam.CreateInstance("ADODB.Parameter");
			pParam->Name = L"@DNIS";
			pParam->Type = adVarChar;
			pParam->Size = 12;
			pParam->Direction = adParamInput;
			cmd->Parameters->Append(pParam);
			cmd->Parameters->Item[L"@DNIS"]->Value = _variant_t(szDnis.GetBuffer(pParam->Size));

			pParam.CreateInstance("ADODB.Parameter");
			pParam->Name = L"@SHOP_ID1";
			pParam->Type = adVarChar;
			pParam->Size = 32;
			pParam->Direction = adParamInput;
			cmd->Parameters->Append(pParam);
			cmd->Parameters->Item[L"@SHOP_ID1"]->Value = _variant_t(szShopId1.GetBuffer(pParam->Size));

			pParam.CreateInstance("ADODB.Parameter");
			pParam->Name = L"@SHOP_ID2";
			pParam->Type = adVarChar;
			pParam->Size = 32;
			pParam->Direction = adParamInput;
			cmd->Parameters->Append(pParam);
			cmd->Parameters->Item[L"@SHOP_ID2"]->Value = _variant_t(szShopId2.GetBuffer(pParam->Size));

			cmd->ActiveConnection = m_CONN;
			//try{
			cmd->Execute(NULL, NULL, adCmdStoredProc);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			xprintf("[CH:%03d] sp_getAllatOrderInfoByTel4 END", m_pScenario->nChan);
			return FALSE;
		}
		m_RS.CreateInstance(__uuidof(Recordset));
		m_RS->PutRefSource(cmd);

		_variant_t vNull;
		vNull.vt = VT_ERROR;
		vNull.scode = DISP_E_PARAMNOTFOUND;
		m_RS->CursorLocation = adUseClient;

		try{
			m_RS->Open(vNull, vNull, adOpenStatic, adLockOptimistic, adCmdStoredProc);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			xprintf("[CH:%03d] sp_getAllatOrderInfoByTel4 END", m_pScenario->nChan);

			return FALSE;
		}
		xprintf("[CH:%03d] sp_getAllatOrderInfoByTel4 END", m_pScenario->nChan);

		return TRUE;
	}
	else
	{
		xprintf("[CH:%03d] sp_getAllatOrderInfoByTel4 END", m_pScenario->nChan);

		return FALSE;
	}
}

int CADODB::RegOrderInfo(INFOPRODOCREQ infoReq, INFOPRODOCRES infoOrder)
{
	if (ISOpen())
	{
		_CommandPtr& cmd = m_CMD;
		cmd.CreateInstance("ADODB.Command");
		CString strSql;

		//"MX_SEQ" INT NOT NULL DEFAULT NULL,
		strSql.Format("insert into dbo.ALLAT_SHOP_ORDER\n"
			"( \n"
			"  MX_ISSUE_NO, \n"
			"  MX_NAME, \n"
			"  MX_ID, \n"
			"  MX_OPT, \n"
			"  ADMIN_ID, \n"
			"  CC_NAME, \n"
			"  CC_PORD_DESC, \n"
			"  CC_EMAIL, \n"
			"  AMOUNT, \n"
			"  PHONE_NO, \n"
			"  AUTH_NO, \n"
			"  REQUEST_TYPE, \n"
			"  REPLY_CODE, \n"
			"  REPLY_MESSAGE, \n"
			"  REG_DATE, \n"
			"  AUTO_INPUT, \n"
			"  PAYMENT_CODE, \n"
			"  ADMIN_NAME, \n"
			"  PAY_DATE, \n"
			"  POINT_YN, \n"
			"  MX_ID2, \n"
			"  ITEM_CODE \n"
			") \n"
			"select top 1  \'%s\', \n"                                                                             // MX_ISSUE_NO                   
			" (select SHOP_NAME from dbo.ALLAT_SHOP_ADMIN where SHOP_ID =  \'%s\') AS  MX_NAME , \n"               // MX_NAME   SHOP_ID				
			" \'%s\' , \n"                                                                                         // MX_ID	    SHOP_ID							
			" (select top 1 SHOP_KEY from dbo.ALLAT_SHOP_ADMIN where SHOP_ID =  \'%s\') AS  MX_OPT  , \n"          // MX_OPT	SHOP_ID					
			" (select top 1 ADMIN_ID from dbo.ALLAT_SHOP_ADMIN where SHOP_ID =  \'%s\') AS  ADMIN_ID  , \n"        // ADMIN_ID	SHOP_ID					
			" \'%s\' , \n"                                                                                         // CC_NAME						
			" \'%s^%s\' , \n"                                                                                      // CC_PORD_DESC ^ PORD_CODE		
			" \'\'  , \n"                                                                                          // CC_EMAIL						
			" \'%s\', \n"                                                                                          // AMOUNT 						
			" \'%s\', \n"                                                                                          // PHONE_NO						
			" \'\', \n"                                                                                            // AUTH_NO						
			" \'CIA\', \n"                                                                                         // REQUEST_TYPE					
			" \'\', \n"                                                                                            // REPLY_CODE					
			" \'\', \n"                                                                                            // REPLY_MESSAGE					
			" getdate(), \n"                                                                                       // REG_DATE						
			" \'N\', \n"                                                                                           // AUTO_INPUT 					
			" \'0\', \n"                                                                                           // PAYMENT_CODE 					
			" (select top 1 ADMIN_NAME   from dbo.ALLAT_SHOP_ADMIN where SHOP_ID =  \'%s\') AS  ADMIN_NAME  , \n"  // ADMIN_NAME	SHOP_ID						
			" \'\', \n"                                                                                            // PAY_DATE						
			" \'N\' , \n"                                                                                          // POINT_YN							
			" \'%s\', "                                                                                              // 일반결제용 SHOP_ID(SHOP_ID2)			
			" \'%s\' "                                                                                              //  상품코드
			, TEXT(infoOrder.m_szorder_no),         // MX_ISSUE_NO               
			TEXT(infoOrder.m_szshop_id1),		    // MX_NAME					
			TEXT(infoOrder.m_szshop_id1),		    // MX_ID						
			TEXT(infoOrder.m_szshop_id1),    	    // MX_OPT					
			TEXT(infoOrder.m_szshop_id1),            // ADMIN_ID					 
			TEXT(infoOrder.m_szcc_name),            // CC_NAME					
			TEXT(infoOrder.m_szcc_pord_desc), TEXT(infoOrder.m_szcc_pord_code), // CC_PORD_DESC ^ PORD_CODE	
			TEXT(infoOrder.m_szamount),   		    // AMOUNT				
			TEXT(infoReq.m_szHP_NO),   		        // PHONE_NO	
			TEXT(infoOrder.m_szshop_id1),             // ADMIN_NAME
			TEXT(infoOrder.m_szshop_id2),            // 일반결제용
			TEXT(infoOrder.m_szcc_pord_code)         // 상품코드
		);
			
		xprintf("strSql: %s", strSql);
		cmd->CommandText = strSql.AllocSysString();
		cmd->CommandType = adCmdText;

		cmd->ActiveConnection = m_CONN;
		try{
			cmd->Execute(NULL, NULL, adCmdText);
		}
		catch (_com_error e)
		{
			PrintProviderError();
			PrintComError(e);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/* ------------------------------------------------------------------------ */
void ADO_Quithostio(char *p, int ch)
{
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*port)[ch].pScenario);


	xprintf("[CH:%03d] ADO_Quithostio===START", ch);

	if (pScenario->m_AdoDb != NULL)
	{
		delete pScenario->m_AdoDb;
		pScenario->m_AdoDb  = NULL;
	}
	CoUninitialize();

	(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
	if ((*port)[ch].used != L_IDLE && in_multifunc(ch))
		quitchan(ch);

	xprintf("[CH:%03d] %s", ch, p);
	xprintf("[CH:%03d] ADO_Quithostio _endthread", ch);
}


unsigned int __stdcall upOrderPayStateProc(void *data)
{
	int			ch;
	int			threadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = pScenario->threadID;

	xprintf("[CH:%03d] upOrderPaySatetProc START", ch);

	CoInitialize(0);
	pScenario->m_AdoDb = new CADODB(pScenario);

	//리턴값
	//=====================
	//쿼리인자변수선언
	//=====================
	pScenario->m_PayResult = 0;
	if (threadID != pScenario->threadID) {
		pScenario->m_PayResult = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("upOrderPaySatetProc the line service is not valid any more.", ch);
		xprintf("[CH:%03d] upOrderPaySatetProc END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}
	if (pScenario->m_AdoDb == NULL){
		pScenario->m_PayResult = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("upOrderPaySatetProc the line service Object is Null.", ch);
		xprintf("[CH:%03d] upOrderPaySatetProc END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	char szDATABASE_IP_URL[50 + 1];
	char szDATABASE[50 + 1];
	char szSID[50 + 1];
	char szPASSWORD[50 + 1];

	GetPrivateProfileString(DATABASE_SESSION, "DATABASE_IP_URL", DATABASE_IP_URL, szDATABASE_IP_URL, sizeof(szDATABASE_IP_URL), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "DATABASE", DATABASE, szDATABASE, sizeof(szDATABASE), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "SID", SID, szSID, sizeof(szSID), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "PASSWORD", PASSWORD, szPASSWORD, sizeof(szPASSWORD), PARAINI);

	if (pScenario->m_AdoDb->DBConnect(szPASSWORD, szSID, szDATABASE, szDATABASE_IP_URL) == NULL)
	{
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("upOrderPaySatetProc the line service is Conneted Error", ch);
		xprintf("[CH:%03d] upOrderPaySatetProc END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	BOOL bRet = pScenario->m_AdoDb->upOrderPayState(pScenario->m_CardResInfo.m_szRESULTCODE
		                               , pScenario->m_CardResInfo.m_szRESULTMSG 
									   , pScenario->m_CardResInfo.m_szMoid, pScenario->m_CardResInfo.m_szMid);

	if (bRet == TRUE) pScenario->m_PayResult = 1;
	else pScenario->m_PayResult = 0;

	ADO_Quithostio("upOrderPaySatetProc the line service is Success...........", ch);
	xprintf("[CH:%03d] upOrderPaySatetProc END", ch);
	_endthreadex((unsigned int)pScenario->m_hThread);
	return 0;
}

unsigned int __stdcall setPayLogPorc(void *data)
{
	int			ch;
	int			threadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = pScenario->threadID;

	xprintf("[CH:%03d] setPayLogPorc START", ch);

	CoInitialize(0);
	pScenario->m_AdoDb = new CADODB(pScenario);

	//리턴값
	//=====================
	//쿼리인자변수선언
	//=====================
	pScenario->m_PayResult = 0;
	if (threadID != pScenario->threadID) {
		pScenario->m_PayResult = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("setPayLogPorc the line service is not valid any more.", ch);
		xprintf("[CH:%03d] setPayLogPorc END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}
	if (pScenario->m_AdoDb == NULL){
		pScenario->m_PayResult = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("setPayLogPorc the line service Object is Null.", ch);
		xprintf("[CH:%03d] setPayLogPorc END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	char szDATABASE_IP_URL[50 + 1];
	char szDATABASE[50 + 1];
	char szSID[50 + 1];
	char szPASSWORD[50 + 1];

	GetPrivateProfileString(DATABASE_SESSION, "DATABASE_IP_URL", DATABASE_IP_URL, szDATABASE_IP_URL, sizeof(szDATABASE_IP_URL), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "DATABASE", DATABASE, szDATABASE, sizeof(szDATABASE), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "SID", SID, szSID, sizeof(szSID), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "PASSWORD", PASSWORD, szPASSWORD, sizeof(szPASSWORD), PARAINI);

	if (pScenario->m_AdoDb->DBConnect(szPASSWORD, szSID, szDATABASE, szDATABASE_IP_URL) == NULL)
	{
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("setPayLogPorc the line service is Conneted Error", ch);
		xprintf("[CH:%03d] setPayLogPorc END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	xprintf("[CH:%03d] ag_Card_ResInfo.m_szMoid: %s", ch, pScenario->m_CardResInfo.m_szMoid);
	xprintf("[CH:%03d] ag_Card_ResInfo.m_szCardQuota: %s", ch, pScenario->m_CardResInfo.m_szCardQuota);
	xprintf("[CH:%03d] ag_Card_ResInfo.m_szRESULTCODE: %s", ch, pScenario->m_CardResInfo.m_szRESULTCODE);
	xprintf("[CH:%03d] ag_Card_ResInfo.m_szMid: %s", ch, pScenario->m_CardResInfo.m_szMid);

	BOOL bRet = pScenario->m_AdoDb->setPayLog(pScenario->m_CardResInfo);

	if (bRet == TRUE) pScenario->m_PayResult = 1;
	else pScenario->m_PayResult = 0;

	ADO_Quithostio("setPayLogPorc the line service is Success...........", ch);
	xprintf("[CH:%03d] setPayLogPorc END", ch);
	_endthreadex((unsigned int)pScenario->m_hThread);
	return 0;
}

// DNIS에 맵핑 된 해당 DLL 정보 및 시나리오 유형을 획득 한다.
// 지정된 주문번호를 키로 한다.
unsigned int __stdcall sp_getAllatOrderInfoBySMS2(void *data)
{
	int			ch;
	int			threadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = pScenario->threadID;

	xprintf("[CH:%03d] sp_getAllatOrderInfoBySMS2 START", ch);
	
	//2016.12.26
	//교환기로부터 정의 되지 않은 길이 및 유형의 데이터 수신 
	//대응을 위해 초기화 위치 변경
	memset(pScenario->m_szMx_issue_no, 0x00, sizeof(pScenario->m_szMx_issue_no));
	memset(pScenario->m_szMx_name, 0x00, sizeof(pScenario->m_szMx_name));
	memset(pScenario->m_szMx_id, 0x00, sizeof(pScenario->m_szMx_id));
	memset(pScenario->m_szCC_name, 0x00, sizeof(pScenario->m_szCC_name));
	memset(pScenario->m_szCC_Prod_Desc, 0x00, sizeof(pScenario->m_szCC_Prod_Desc));
	memset(pScenario->m_szCC_Prod_Code, 0x00, sizeof(pScenario->m_szCC_Prod_Code));
	memset(pScenario->m_szPhone_no, 0x00, sizeof(pScenario->m_szPhone_no));
	memset(pScenario->m_szMx_opt, 0x00, sizeof(pScenario->m_szMx_opt));
	memset(pScenario->m_szCC_email, 0X00, sizeof(pScenario->m_szCC_email));
	memset(pScenario->m_sz_Shop_Pw, 0X00, sizeof(pScenario->m_sz_Shop_Pw));

	// 결제 후 리턴 페이지 주소 정보 초기화
	memset(pScenario->m_szURL_YN, 0X00, sizeof(pScenario->m_szURL_YN));
	memset(pScenario->m_szSHOP_RET_URL, 0X00, sizeof(pScenario->m_szSHOP_RET_URL));

	pScenario->m_nAmount = 0;

	memset(pScenario->m_szInstallment, 0x00, sizeof(pScenario->m_szInstallment));

	CoInitialize(0);
	pScenario->m_AdoDb = new CADODB(pScenario);

	//리턴값
	//=====================
	//쿼리인자변수선언
	//=====================
	pScenario->m_bDnisInfo = 0;
	if (threadID != pScenario->threadID) {
		pScenario->m_bDnisInfo = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("sp_getAllatOrderInfoBySMS2 the line service is not valid any more.", ch);
		xprintf("[CH:%03d] sp_getAllatOrderInfoBySMS2 END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}
	if (pScenario->m_AdoDb == NULL){
		pScenario->m_bDnisInfo = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("sp_getAllatOrderInfoBySMS2 the line service Object is Null.", ch);
		xprintf("[CH:%03d] sp_getAllatOrderInfoBySMS2 END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	char szDATABASE_IP_URL[50 + 1];
	char szDATABASE[50 + 1];
	char szSID[50 + 1];
	char szPASSWORD[50 + 1];

	GetPrivateProfileString(DATABASE_SESSION, "DATABASE_IP_URL", DATABASE_IP_URL, szDATABASE_IP_URL, sizeof(szDATABASE_IP_URL), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "DATABASE", DATABASE, szDATABASE, sizeof(szDATABASE), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "SID", SID, szSID, sizeof(szSID), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "PASSWORD", PASSWORD, szPASSWORD, sizeof(szPASSWORD), PARAINI);

	if (pScenario->m_AdoDb->DBConnect(szPASSWORD, szSID, szDATABASE, szDATABASE_IP_URL) == NULL)
	{
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("sp_getAllatOrderInfoBySMS2 the line service is Conneted Error", ch);
		xprintf("[CH:%03d] sp_getAllatOrderInfoBySMS2 END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	if (pScenario->m_AdoDb->sp_getAllatOrderInfoBySMS2(pScenario->szDnis, pScenario->m_szAuth_no))
	{
		// 2015.12.10  형 변환 호환성 문제 발생
		/*_bstr_t*/ _variant_t bt = "";
		char szAmount[10 + 1];
		int iROWCOUNT;
		int iField;

		iROWCOUNT = pScenario->m_AdoDb->GetRecCount();
		if (iROWCOUNT <= 0)
		{
			pScenario->m_bDnisInfo = 0;
			(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
			ADO_Quithostio("sp_getAllatOrderInfoBySMS2 the line service is GetRecCount Warning", ch);
			xprintf("[CH:%03d] sp_getAllatOrderInfoBySMS2 END", ch);
			_endthreadex((unsigned int)pScenario->m_hThread);
			return 0;
		}

		iField = pScenario->m_AdoDb->GetFieldCount();
		for (int iROW = 0; iROW < iROWCOUNT; iROW++)
		{//무조건 하나다
			// 필요한 변수 초기화 반드시 한다.

			//2016.12.26
			//교환기로부터 정의 되지 않은 길이 및 유형의 데이터 수신 
			//대응을 위해 초기화 위치 변경
			//memset(pScenario->m_szMx_issue_no, 0x00, sizeof(pScenario->m_szMx_issue_no));
			//memset(pScenario->m_szMx_name, 0x00, sizeof(pScenario->m_szMx_name));
			//memset(pScenario->m_szMx_id, 0x00, sizeof(pScenario->m_szMx_id));
			//memset(pScenario->m_szCC_name, 0x00, sizeof(pScenario->m_szCC_name));
			//memset(pScenario->m_szCC_Prod_Desc, 0x00, sizeof(pScenario->m_szCC_Prod_Desc));
			//memset(pScenario->m_szCC_Prod_Code, 0x00, sizeof(pScenario->m_szCC_Prod_Code));
			//memset(pScenario->m_szPhone_no, 0x00, sizeof(pScenario->m_szPhone_no));
			//memset(pScenario->m_szMx_opt, 0x00, sizeof(pScenario->m_szMx_opt));
			//memset(pScenario->m_szCC_email, 0X00, sizeof(pScenario->m_szCC_email));
			//memset(pScenario->m_sz_Shop_Pw, 0X00, sizeof(pScenario->m_sz_Shop_Pw));

			//// 결제 후 리턴 페이지 주소 정보 초기화
			//memset(pScenario->m_szURL_YN, 0X00, sizeof(pScenario->m_szURL_YN));
			//memset(pScenario->m_szSHOP_RET_URL, 0X00, sizeof(pScenario->m_szSHOP_RET_URL));

			//pScenario->m_nAmount = 0;

			//memset(pScenario->m_szInstallment, 0x00, sizeof(pScenario->m_szInstallment));

			pScenario->m_AdoDb->GetRs(_variant_t(L"MX_ISSUE_NO"), bt);
			strncpy_s(pScenario->m_szMx_issue_no,sizeof(pScenario->m_szMx_issue_no), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_issue_no) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"MX_ID"), bt);
			strncpy_s(pScenario->m_szMx_id,sizeof(pScenario->m_szMx_id), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_id) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"MX_OPT"), bt);
			strncpy_s(pScenario->m_szMx_opt,sizeof(pScenario->m_szMx_opt), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_opt) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"MX_NAME"), bt);
			strncpy_s(pScenario->m_szMx_name, sizeof(pScenario->m_szMx_name),(char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_name) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"CC_NAME"), bt);
			strncpy_s(pScenario->m_szCC_name,sizeof(pScenario->m_szCC_name), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_name) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"CC_PORD_DESC"), bt);
			strncpy_s(pScenario->m_szCC_Prod_Desc,sizeof(pScenario->m_szCC_Prod_Desc), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_Prod_Desc) - 1);
			char *pTmpChr = NULL;
			pTmpChr = strchr(pScenario->m_szCC_Prod_Desc, '^');
			if (pTmpChr)
			{
				strncpy_s(pScenario->m_szCC_Prod_Code,sizeof(pScenario->m_szCC_Prod_Code), pTmpChr + 1, sizeof(pScenario->m_szCC_Prod_Code) - 1);
				pTmpChr[0] = 0x00;
				//2017.08.17
				//수정
				//memset(pTmpChr, 0x00, strlen(pScenario->m_szCC_Prod_Code) + 1);
			}
			pScenario->m_AdoDb->GetRs(_variant_t(L"PHONE_NO"), bt);
			strncpy_s(pScenario->m_szPhone_no,sizeof(pScenario->m_szPhone_no), (char*)(_bstr_t)bt, sizeof(pScenario->m_szPhone_no) - 1);
			/*pScenario->m_AdoDb->GetRs(_variant_t(L"INSTALLMENT"), bt);
			strncpy_s(pScenario->m_szInstallment,sizeof(pScenario->m_szInstallment), (char*)(_bstr_t)bt, sizeof(pScenario->m_szInstallment) - 1);*/
			/*pScenario->m_AdoDb->GetRs(_variant_t(L"SHOP_PW"), bt);
			strncpy_s(pScenario->m_sz_Shop_Pw,sizeof(pScenario->m_sz_Shop_Pw), (char*)(_bstr_t)bt, sizeof(pScenario->m_sz_Shop_Pw) - 1);*/

			memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"AMOUNT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nAmount = atoi(szAmount);

			pScenario->m_AdoDb->GetRs(_variant_t(L"CC_EMAIL"), bt);
			strncpy_s(pScenario->m_szCC_email,sizeof(pScenario->m_szCC_email), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_email) - 1);

			/*memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"SERVICEAMT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nServiceAmt = atoi(szAmount);

			memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"SUPPLYAMT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nSupplyAmt = atoi(szAmount);

			memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"GOODSVAT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nGoodsVat = atoi(szAmount);

			memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"TAXFREEAMT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nTaxFreeAmt = atoi(szAmount);*/

			// 결제 후 리턴 페이지 정보
			pScenario->m_AdoDb->GetRs(_variant_t(L"URL_YN"), bt);
			strncpy_s(pScenario->m_szURL_YN,sizeof(pScenario->m_szURL_YN), (char*)(_bstr_t)bt, sizeof(pScenario->m_szURL_YN)-1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"SHOP_RET_URL"), bt);
			strncpy_s(pScenario->m_szSHOP_RET_URL,sizeof(pScenario->m_szSHOP_RET_URL), (char*)(_bstr_t)bt, sizeof(pScenario->m_szSHOP_RET_URL) - 1);

			pScenario->m_AdoDb->Next();
		}
		pScenario->m_AdoDb->RSClose();

		pScenario->m_bDnisInfo = 1;
		ADO_Quithostio("sp_getAllatOrderInfoBySMS2 the line service is Success...........", ch);
		xprintf("[CH:%03d] sp_getAllatOrderInfoBySMS2 END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	pScenario->m_bDnisInfo = -1;
	ADO_Quithostio("sp_getAllatOrderInfoBySMS2 the line service is Fail...........", ch);
	xprintf("[CH:%03d] sp_getAllatOrderInfoBySMS2 END", ch);
	_endthreadex((unsigned int)pScenario->m_hThread);
	return 0;
}


// DNIS에 맵핑 된 해당 DLL 정보 및 시나리오 유형을 획득 한다.
// 지정된 핸드폰 번호를 키로 한다.
unsigned int __stdcall sp_getAllatOrderInfoByTel2(void *data)
{
	int			ch;
	int			threadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	threadID = pScenario->threadID;

	xprintf("[CH:%03d] sp_getAllatOrderInfoByTel2 START", ch);

	//2016.12.26
	//교환기로부터 정의 되지 않은 길이 및 유형의 데이터 수신 
	//대응을 위해 초기화 위치 변경
	memset(pScenario->m_szMx_issue_no, 0x00, sizeof(pScenario->m_szMx_issue_no));
	memset(pScenario->m_szMx_name, 0x00, sizeof(pScenario->m_szMx_name));
	memset(pScenario->m_szMx_id, 0x00, sizeof(pScenario->m_szMx_id));
	memset(pScenario->m_szCC_name, 0x00, sizeof(pScenario->m_szCC_name));
	memset(pScenario->m_szCC_Prod_Desc, 0x00, sizeof(pScenario->m_szCC_Prod_Desc));
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

	CoInitialize(0);
	pScenario->m_AdoDb = new CADODB(pScenario);

	//리턴값
	//=====================
	//쿼리인자변수선언
	//=====================
	pScenario->m_bDnisInfo = 0;
	if (threadID != pScenario->threadID) {
		pScenario->m_bDnisInfo = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("sp_getAllatOrderInfoByTel2 the line service is not valid any more.", ch);
		xprintf("[CH:%03d] sp_getAllatOrderInfoByTel2 END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}
	if (pScenario->m_AdoDb == NULL){
		pScenario->m_bDnisInfo = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("sp_getAllatOrderInfoByTel2 the line service Object is Null.", ch);
		xprintf("[CH:%03d] sp_getAllatOrderInfoByTel2 END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}
	
	char szDATABASE_IP_URL[50 + 1];
	char szDATABASE[50 + 1];
	char szSID[50 + 1];
	char szPASSWORD[50 + 1];

	GetPrivateProfileString(DATABASE_SESSION, "DATABASE_IP_URL", DATABASE_IP_URL, szDATABASE_IP_URL, sizeof(szDATABASE_IP_URL), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "DATABASE", DATABASE, szDATABASE, sizeof(szDATABASE), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "SID", SID, szSID, sizeof(szSID), PARAINI);
	GetPrivateProfileString(DATABASE_SESSION, "PASSWORD", PASSWORD, szPASSWORD, sizeof(szPASSWORD), PARAINI);

	if (pScenario->m_AdoDb->DBConnect(szPASSWORD, szSID, szDATABASE, szDATABASE_IP_URL) == NULL)
	{
		pScenario->m_DBAccess = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		ADO_Quithostio("sp_getAllatOrderInfoByTel2 the line service is Conneted Error", ch); 
		xprintf("[CH:%03d] sp_getAllatOrderInfoByTel2 END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	if (pScenario->m_AdoDb->sp_getAllatOrderInfoByTel2(pScenario->szDnis, pScenario->m_szInputTel))
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
			ADO_Quithostio("sp_getAllatOrderInfoByTel2 the line service is GetRecCount Warning", ch);
			xprintf("[CH:%03d] sp_getAllatOrderInfoByTel2 END", ch);
			_endthreadex((unsigned int)pScenario->m_hThread);
			return 0;
		}

		iField = pScenario->m_AdoDb->GetFieldCount();
		for (int iROW = 0; iROW < iROWCOUNT; iROW++)
		{//무조건 하나다
			// 필요한 변수 초기화 반드시 한다.
			
			//2016.12.26
			//교환기로부터 정의 되지 않은 길이 및 유형의 데이터 수신 
			//대응을 위해 초기화 위치 변경
			//memset(pScenario->m_szMx_issue_no, 0x00, sizeof(pScenario->m_szMx_issue_no));
			//memset(pScenario->m_szMx_name, 0x00, sizeof(pScenario->m_szMx_name));
			//memset(pScenario->m_szMx_id, 0x00, sizeof(pScenario->m_szMx_id));
			//memset(pScenario->m_szCC_name, 0x00, sizeof(pScenario->m_szCC_name));
			//memset(pScenario->m_szCC_Prod_Desc, 0x00, sizeof(pScenario->m_szCC_Prod_Desc));
			//memset(pScenario->m_szCC_Prod_Code, 0x00, sizeof(pScenario->m_szCC_Prod_Code));
			//memset(pScenario->m_szPhone_no, 0x00, sizeof(pScenario->m_szPhone_no));
			//memset(pScenario->m_szMx_opt, 0x00, sizeof(pScenario->m_szMx_opt));
			//memset(pScenario->m_szCC_email, 0X00, sizeof(pScenario->m_szCC_email));
			//memset(pScenario->m_sz_Shop_Pw, 0X00, sizeof(pScenario->m_sz_Shop_Pw));
			//pScenario->m_nAmount = 0;

			//memset(pScenario->m_szInstallment, 0x00, sizeof(pScenario->m_szInstallment));

			//// 결제 후 리턴 페이지 주소 정보 초기화
			//memset(pScenario->m_szURL_YN, 0X00, sizeof(pScenario->m_szURL_YN));
			//memset(pScenario->m_szSHOP_RET_URL, 0X00, sizeof(pScenario->m_szSHOP_RET_URL));

			pScenario->m_AdoDb->GetRs(_variant_t(L"MX_ISSUE_NO"), bt);
			strncpy_s(pScenario->m_szMx_issue_no,sizeof(pScenario->m_szMx_issue_no), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_issue_no) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"MX_ID"), bt);
			strncpy_s(pScenario->m_szMx_id,sizeof(pScenario->m_szMx_id), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_id) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"MX_OPT"), bt);
			strncpy_s(pScenario->m_szMx_opt,sizeof(pScenario->m_szMx_opt), (char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_opt) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"MX_NAME"), bt);
			strncpy_s(pScenario->m_szMx_name, sizeof(pScenario->m_szMx_name),(char*)(_bstr_t)bt, sizeof(pScenario->m_szMx_name) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"CC_NAME"), bt);
			strncpy_s(pScenario->m_szCC_name,sizeof(pScenario->m_szCC_name), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_name) - 1); 
			pScenario->m_AdoDb->GetRs(_variant_t(L"CC_PORD_DESC"), bt);
			strncpy_s(pScenario->m_szCC_Prod_Desc,sizeof(pScenario->m_szCC_Prod_Desc), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_Prod_Desc) - 1);
			char *pTmpChr = NULL;
			pTmpChr = strchr(pScenario->m_szCC_Prod_Desc, '^');
			if (pTmpChr)
			{
				strncpy_s(pScenario->m_szCC_Prod_Code,sizeof(pScenario->m_szCC_Prod_Code), pTmpChr + 1, sizeof(pScenario->m_szCC_Prod_Code) - 1);
				pTmpChr[0] = 0x00;
				//2017.08.17
				//수정
				//memset(pTmpChr, 0x00, strlen(pScenario->m_szCC_Prod_Code) + 1);
			}
			pScenario->m_AdoDb->GetRs(_variant_t(L"PHONE_NO"), bt);
			strncpy_s(pScenario->m_szPhone_no,sizeof(pScenario->m_szPhone_no), (char*)(_bstr_t)bt, sizeof(pScenario->m_szPhone_no) - 1);
			/*pScenario->m_AdoDb->GetRs(_variant_t(L"INSTALLMENT"), bt);
			strncpy_s(pScenario->m_szInstallment,sizeof(pScenario->m_szInstallment), (char*)(_bstr_t)bt, sizeof(pScenario->m_szInstallment) - 1);*/
			/*pScenario->m_AdoDb->GetRs(_variant_t(L"SHOP_PW"), bt);
			strncpy_s(pScenario->m_sz_Shop_Pw,sizeof(pScenario->m_sz_Shop_Pw), (char*)(_bstr_t)bt, sizeof(pScenario->m_sz_Shop_Pw) - 1); */

			memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"AMOUNT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nAmount = atoi(szAmount);

			pScenario->m_AdoDb->GetRs(_variant_t(L"CC_EMAIL"), bt);
			strncpy_s(pScenario->m_szCC_email,sizeof(pScenario->m_szCC_email), (char*)(_bstr_t)bt, sizeof(pScenario->m_szCC_email) - 1);

			/*memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"SERVICEAMT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nServiceAmt = atoi(szAmount);

			memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"SUPPLYAMT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nSupplyAmt = atoi(szAmount);

			memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"GOODSVAT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nGoodsVat = atoi(szAmount);

			memset(szAmount, 0x00, sizeof(szAmount));
			pScenario->m_AdoDb->GetRs(_variant_t(L"TAXFREEAMT"), bt);
			strncpy_s(szAmount,sizeof(szAmount), (char*)(_bstr_t)bt, sizeof(szAmount) - 1);
			pScenario->m_nTaxFreeAmt = atoi(szAmount);*/

			// 결제 후 리턴 페이지 정보
			pScenario->m_AdoDb->GetRs(_variant_t(L"URL_YN"), bt);
			strncpy_s(pScenario->m_szURL_YN,sizeof(pScenario->m_szURL_YN), (char*)(_bstr_t)bt, sizeof(pScenario->m_szURL_YN) - 1);
			pScenario->m_AdoDb->GetRs(_variant_t(L"SHOP_RET_URL"), bt);
			strncpy_s(pScenario->m_szSHOP_RET_URL,sizeof(pScenario->m_szSHOP_RET_URL), (char*)(_bstr_t)bt, sizeof(pScenario->m_szSHOP_RET_URL) - 1);

			pScenario->m_AdoDb->Next();
		}
		pScenario->m_AdoDb->RSClose();

		pScenario->m_bDnisInfo = 1;
		ADO_Quithostio("sp_getAllatOrderInfoByTel2 the line service is Success...........", ch);
		xprintf("[CH:%03d] sp_getAllatOrderInfoByTel2 END", ch);
		_endthreadex((unsigned int)pScenario->m_hThread);
		return 0;
	}

	pScenario->m_bDnisInfo = -1;
	ADO_Quithostio("sp_getAllatOrderInfoByTel2 the line service is Fail...........", ch);
	xprintf("[CH:%03d] sp_getAllatOrderInfoByTel2 END", ch);
	_endthreadex((unsigned int)pScenario->m_hThread);
	return 0;
}

int getOrderInfo_host(int holdm)
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
		ADO_Quithostio("DnisVsDll_Host the line service is Load Error", (*lpmt)->chanID);
		return 0;
	}

	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hThread = (HANDLE) _beginthreadex(NULL, 0, sp_getAllatOrderInfoByTel2, (LPVOID)(*lpmt), 0, &(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->threadID));

	return(0);
}

int setPayLog_host(int holdm)
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
	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_PayResult = 0;
	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_AdoDb != NULL)
	{
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_DBAccess = -1;
		(*lpmt)->ppftbl[POST_NET].postcode = HI_OK;
		ADO_Quithostio("DnisVsDll_Host the line service is Load Error", (*lpmt)->chanID);
		return 0;
	}
	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hThread = (HANDLE)_beginthreadex(NULL, 0, setPayLogPorc, (LPVOID)(*lpmt), 0, &(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->threadID));

	return(0);
}

int upOrderPayState_host(int holdm)
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
	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_PayResult = 0;
	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_AdoDb != NULL)
	{
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_DBAccess = -1;
		(*lpmt)->ppftbl[POST_NET].postcode = HI_OK;
		ADO_Quithostio("DnisVsDll_Host the line service is Load Error", (*lpmt)->chanID);
		return 0;
	}
	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hThread = (HANDLE)_beginthreadex(NULL, 0, upOrderPayStateProc, (LPVOID)(*lpmt), 0, &(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->threadID));

	return(0);
}



int getSMSOrderInfo_host(int holdm)
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
		ADO_Quithostio("DnisVsDll_Host the line service is Load Error", (*lpmt)->chanID);
		return 0;
	}
	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hThread = (HANDLE)_beginthreadex(NULL, 0, sp_getAllatOrderInfoBySMS2, (LPVOID)(*lpmt), 0, &(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->threadID));

	return(0);
}
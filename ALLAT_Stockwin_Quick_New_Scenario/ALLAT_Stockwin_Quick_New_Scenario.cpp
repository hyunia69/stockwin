/****************************************************************************
*																			*
*	주   S E R V I C E	J O B S 											*
*																			*
****************************************************************************/
/* Head files */
#include "stdafx.h"
#include "CommonDef.H"
#include "ALLATCommom.h"

#include "WowTvSocket.h"
#include "Scenaio.h"
#include "ADODB.h"
#include "ALLAT_Stockwin_Quick_New_Scenario.h"

#define  ALLAT_MID      "allat_testars"
#define  ALLAT_LICENSEKEY "4d184eff535106b071ada6b9f1940a59"
#define  ALLAT_CANCELPWD   "123456"
#define  ALLAT_DEBUG   "false"
#define  ALLAT_LOG     "./AllatPay/log/"
// 2015.12.16
// 올앳 페이먼트의 연계 모듈의 구저적 문제로 메모리 누수 현상 발견
// 해결하기 위한 코드 개발
// 최영락
#include    "AllatUtil.h"

char g_strMid[50];
char g_strLicenekey[100];
char g_strCancelPwd[100];
char gNiceDebug[10 + 1];
char gNiceLog[50 + 1];

void(*eprintf)(const char *str, ...);
void(*xprintf)(const char *str, ...);
LPMTP **lpmt = NULL, **port = NULL;

//LPMTP	*curyport=NULL;
void(*info_printf)(int chan, const char *str, ...) = NULL;
void(*new_guide)(void) = NULL;
int(*set_guide)(int vid, ...);
void(*setPostfunc)(int type, int(*func)(int), int poststate, int wtime);
int(*send_guide)(int mode);
int(*goto_hookon)(void);
int(*check_validform)(char *form, char *data);
int(*send_error)(void);
int(*check_validdtmf)(int c, char *vkeys);
int(*in_multifunc)(int chan);
int(*quitchan)(int chan);
int(*CallAdd_Host)(int holdm);
//int  (*set_guide2)(int vid, char *format, ...);//파일명을 가변 조합하는 곳을 이것을 써라 강제다...

int(*atoi2)(char *p);
int(*atoi3)(char *p);
int(*atoi4)(char *p);
int(*atoiN)(char *p, int n);
long(*atolN)(char *p, int n);
int(*atox)(char *s);
int(*check_validkey)(char *data);

int(*TTS_Play)(int chan, int holdm, const char *str, ...);

extern int  getOrderInfo_host(int holdm);
extern int  AllatPaymemt_host(int holdm);
extern int  AllatPayCancle_host(int holdm);
extern int  AllatPayRetPrc_host(int holdm);
extern int  setPayLog_host(int holdm);
extern int  upOrderPayState_host(int holdm);
extern int   getSMSOrderInfo_host(int holdm);

extern int getTcpOrderInfo_host(int holdm);
extern int getOrderInfo_NewAPI_host(int holdm);
extern int getOrderInfo_host_wrapper(int holdm);  // USE_NEW_API 분기 처리

int ALLAT_ArsScenarioStart(/* [in] */int state);
int ALLAT_SMSScenarioStart(/* [in] */int state);
int ALLAT_CID_ScenarioStart(/* [in] */int state);
int ALLAT_CIA_ScenarioStart(/* [in] */int state);

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
#define VOC_TTS_ID  503     // TTS 엔진 GW 용도
#define VOC_WAVE_ID 504
#define VOC_MAIL	20		// 안내문
#define	VOC_MESG	21		// 사서함 메세지
#define VOC_TEMP	22		// Temp
#define VOC_TTS  	23		// TTS
#define VOC_WAVE  	24		// WAVE

///////////////////////////////////////////////////////////////////////

// Port 구분
#define	SERVER_PORT		(API_PORT) + 0
///////////////////////////////////////////////////////////////////////

#ifdef WIN32

// DLL 로딩 직후 반드시 위부 함수 인터페이스 포인터를 설정 해야 한다.
// 필요한 인터페이스만 설정 하도록 한다.
extern "C" {
	SCENARIO_API void Set_pinfo_printf(void(*pinfo_printf)(int chan, const char *str, ...));
	SCENARIO_API void Set_pnew_guide(void(*pnew_guide)(void));
	SCENARIO_API void Set_peprintf(void(*peprintf)(const char *str, ...));
	SCENARIO_API void Set_pxprintf(void(*pxprintf)(const char *str, ...));
	SCENARIO_API void Set_PortInfo(LPMTP **Line_lpmt, LPMTP **Tot_port);
	SCENARIO_API void Set_pset_guide(int(*pset_guide)(int vid, ...));
	SCENARIO_API void Set_psetPostfunc(void(*psetPostfunc)(int type, int(*func)(int), int poststate, int wtime));
	SCENARIO_API void Set_psend_guide(int(*psend_guide)(int mode));
	SCENARIO_API void Set_pgoto_hookon(int(*pgoto_hookon)(void));
	SCENARIO_API void Set_pcheck_validform(int(*pcheck_validform)(char *form, char *data));
	SCENARIO_API void Set_psend_error(int(*psend_error)(void));
	SCENARIO_API void Set_pcheck_validdtmf(int(*pcheck_validdtmf)(int c, char *vkeys));
	SCENARIO_API void Set_pin_multifunc(int(*pin_multifunc)(int chan));
	SCENARIO_API void Set_pquitchan(int(*pquitchan)(int chan));
	SCENARIO_API void Set_patoi2(int(*patoi2)(char *p));
	SCENARIO_API void Set_patoi3(int(*patoi3)(char *p));
	SCENARIO_API void Set_patoi4(int(*patoi4)(char *p));
	SCENARIO_API void Set_patoiN(int(*patoiN)(char *p, int n));
	SCENARIO_API void Set_patolN(long(*patolN)(char *p, int n));
	SCENARIO_API void Set_patox(int(*patox)(char *s));
	SCENARIO_API void Set_pcheck_validkey(int(*pcheck_validkey)(char *data));
	SCENARIO_API void Set_pTTS_Play(int(*pTTS_Play)(int chan, int holdm, const char *str, ...));
	SCENARIO_API void Set_CallAdd_Host(int(*pCallAdd_Host)(int holdm));
	/*
	SCENARIO_API void Set_pset_guide2(int(*pset_guide2)(int vid, char *format, ...));
	*/
	// 채널별 개체 생성자는 무조건 필요
	SCENARIO_API IScenario* CreateEngine();
	SCENARIO_API void DestroyEngine(IScenario* pComponent);

	void Set_pinfo_printf(void(*pinfo_printf)(int chan, const char *str, ...))
	{
		info_printf = pinfo_printf;
	}

	void Set_pnew_guide(void(*pnew_guide)(void))
	{
		new_guide = pnew_guide;
	}

	void Set_peprintf(void(*peprintf)(const char *str, ...))
	{
		eprintf = peprintf;
	}

	void Set_pxprintf(void(*pxprintf)(const char *str, ...))
	{
		xprintf = pxprintf;
	}

	void Set_PortInfo(LPMTP **Line_lpmt, LPMTP **Tot_port)
	{
		lpmt = Line_lpmt;
		port = Tot_port;
	}

	void Set_pset_guide(int(*pset_guide)(int vid, ...))
	{
		set_guide = pset_guide;
	}

	void Set_psetPostfunc(void(*psetPostfunc)(int type, int(*func)(int), int poststate, int wtime))
	{
		setPostfunc = psetPostfunc;
	}

	void Set_psend_guide(int(*psend_guide)(int mode))
	{
		send_guide = psend_guide;
	}

	void Set_pgoto_hookon(int(*pgoto_hookon)(void))
	{
		goto_hookon = pgoto_hookon;
	}

	void Set_pcheck_validform(int(*pcheck_validform)(char *form, char *data))
	{
		check_validform = pcheck_validform;
	}


	void Set_psend_error(int(*psend_error)(void))
	{
		send_error = psend_error;
	}

	void Set_pcheck_validdtmf(int(*pcheck_validdtmf)(int c, char *vkeys))
	{
		check_validdtmf = pcheck_validdtmf;
	}

	void Set_pin_multifunc(int(*pin_multifunc)(int chan))
	{
		in_multifunc = pin_multifunc;
	}

	void Set_pquitchan(int(*pquitchan)(int chan))
	{
		quitchan = pquitchan;
	}


	void Set_patoi2(int(*patoi2)(char *p))
	{
		atoi2 = patoi2;
	}

	void Set_patoi3(int(*patoi3)(char *p))
	{
		atoi3 = patoi3;
	}
	void Set_patoi4(int(*patoi4)(char *p))
	{
		atoi4 = patoi4;
	}
	void Set_patoiN(int(*patoiN)(char *p, int n))
	{
		atoiN = patoiN;
	}
	void Set_patolN(long(*patolN)(char *p, int n))
	{
		atolN = patolN;
	}
	void Set_patox(int(*patox)(char *s))
	{
		atox = patox;
	}

	void Set_pcheck_validkey(int(*pcheck_validkey)(char *data))
	{
		check_validkey = pcheck_validkey;
	}

	void Set_pTTS_Play(int(*pTTS_Play)(int chan, int holdm, const char *str, ...))
	{
		TTS_Play = pTTS_Play;
	}

	void Set_CallAdd_Host(int(*pCallAdd_Host)(int holdm))
	{
		CallAdd_Host = pCallAdd_Host;
	}
	/*
	void Set_pset_guide2(int(*pset_guide2)(int vid, char *format, ...))
	{
	set_guide2 = pset_guide2;
	}
	*/
	// 채널별 개체 생성자는 무조건 필요
	IScenario* CreateEngine()
	{
		return new CALLAT_Hangung_Quick_Scenario(); // CALLAT_Hangung_Quick_Scenario 시나리오 엔진의 개체 생성
	}

	void DestroyEngine(IScenario* pComponent)
	{
		delete (CALLAT_Hangung_Quick_Scenario *)pComponent; // CALLAT_Hangung_Quick_Scenario 시나리오 엔진의 개체 해제
	}

};
#else
#define HANDLE int

#define DLLEXPORT __attribute__ ((visibility("default")))
#define DLLLOCAL   __attribute__ ((visibility("hidden")))

extern "C" DLLEXPORT IScenario* CreateEngine();
extern "C" DLLEXPORT void DestroyEngine(IScenario* object);

extern DLLLOCAL FILE *FpRelease;
#endif

/*----------------------------------------------------------------------*/
//static CRITICAL_SECTION		ALLAT_cs_handler;		// 동기화 
//static BOOL					ALLAT_cs_initialized = FALSE;
//
//void IScenario_enter_handler(void)
//{
//	if (!ALLAT_cs_initialized) {
//		InitializeCriticalSection(&ALLAT_cs_handler);
//		ALLAT_cs_initialized = TRUE;
//	}
//	EnterCriticalSection(&ALLAT_cs_handler);//Looking
//}
//void IScenario_leave_handler(void)
//{
//	if (!ALLAT_cs_initialized) {
//		InitializeCriticalSection(&ALLAT_cs_handler);
//		ALLAT_cs_initialized = TRUE;
//	}
//	LeaveCriticalSection(&ALLAT_cs_handler);
//}


int ALLAT_ExitSvc(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_ExitSvc;
		(*lpmt)->prevState = state;
	}
	switch (state)
	{
	case 0:
		new_guide();
		info_printf(localCh, "ALLAT_ExitSvc [%d] 종료 서비스...", state);
		eprintf("ALLAT_ExitSvc [%d] 종료 서비스", state);
		set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\service_end");	 // "마지막 인사말"
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0xffff, 0);
		return send_guide(NODTMF);
	case 10:
		new_guide();
		info_printf(localCh, "ALLAT_ExitSvc [%d] 종료 서비스...이용방법 확인...", state);
		eprintf("ALLAT_ExitSvc [%d] 종료 서비스:이용방법 확인...", state);
		set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\Error_end");	 // "이용방법 확인..."
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0xffff, 0);
		return send_guide(NODTMF);
	case 0xffff:
		(*lpmt)->Myexit_service = NULL;
		return  goto_hookon();
	default:
		info_printf(localCh, "ALLAT_ExitSvc [%d] 종료 서비스...> 시나리오 아이디 오류", state);
		eprintf("ALLAT_ExitSvc[%d]  종료 서비스...>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}

}
int ALLAT_jobArs(/* [in] */int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	//int     localCh = curyport->chanID;
	int     localCh = (*lpmt)->chanID;

	//if (curyport)
	if (*lpmt)
	{
		//c = *curyport->dtmfs;
		//curyport->PrevCall = ALLAT_jobArs;
		//curyport->prevState = state;
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_jobArs;
		(*lpmt)->prevState = state;
	}

	switch (state)
	{
	case 0:
		new_guide();
		info_printf(localCh, "ALLAT_jobArs [%d] 서비스 준비 중...", state);
		eprintf("ALLAT_jobArs [%d] 서비스 준비", state);
		set_guide(VOC_WAVE_ID, "ment\\Svc");	 // "서비스 준비 중"
		setPostfunc(POST_PLAY, ALLAT_jobArs, 0xffff, 0);
		return send_guide(NODTMF);
		break;
	case 0xffff:
		return  goto_hookon();
	default:
		info_printf(localCh, "ALLAT_jobArs [%d] 서비스 준비 중...> 시나리오 아이디 오류", state);
		eprintf("ALLAT_jobArs[%d] 서비스 준비 중...>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);

	}

	return 0;
}

int CreateAg()
{
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);
	CString szURL = pScenario->m_szSHOP_RET_URL;

	szURL.Replace("{replyCd}", pScenario->m_CardResInfo.m_szRESULTCODE); // 결과 코드 
	szURL.Replace("{replyMsg}", pScenario->m_CardResInfo.m_szRESULTMSG); // 결과 메시지
	szURL.Replace("{orderNo}", pScenario->m_CardResInfo.m_szMoid);       // 주문번호
	szURL.Replace("{approvalAmt}", pScenario->m_CardResInfo.m_AMT);      // 거래 금액
	szURL.Replace("{payType}", pScenario->m_CardResInfo.m_PAYMETHOD);    // 지불수단(결제 수단)
	szURL.Replace("{approvalDate}", pScenario->m_CardResInfo.m_AUTHDATE);// 승인일시
	szURL.Replace("{seqNo}", pScenario->m_CardResInfo.m_Tid);            // 거래일련번호 
	szURL.Replace("{approvalNo}", pScenario->m_CardResInfo.m_AUTHCODE);  // 송인번호
	szURL.Replace("{cardId}", pScenario->m_CardResInfo.m_CARDCODE);      // 카드사 ID
	szURL.Replace("{cardName}", pScenario->m_CardResInfo.m_CARDNAME);    // 신용카드 사 명
	szURL.Replace("{sellMm}", pScenario->m_CardResInfo.m_szCardQuota);   // 할부개월
	szURL.Replace("{zerofeeYn}", pScenario->m_CardResInfo.m_sZerofeeYn); // 무이자 여부
#if 1  //AHN 20220822 return url 에 shop_id 추가 
	szURL.Replace("{shop_id}", pScenario->m_szMx_id);
#endif
	sprintf_s(pScenario->m_szSHOP_RET_URL_AG, sizeof(pScenario->m_szSHOP_RET_URL_AG), "%s", szURL.GetBuffer());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// ALLAT_payARS : ALLAT 연동 후 처리 입력부
////////////////////////////////////////////////////////////////////////////////
int ALLAT_payARS(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_payARS;
		(*lpmt)->prevState = state;
	}
	switch (state)
	{
	case 0:
		info_printf(localCh, "ALLAT_WOWTV_Quick_payARS [%d]Allat 결제 모듈  연동 후 처리  부", state);
		eprintf("ALLAT_WOWTV_Quick_payARS [%d]Allat 결제 모듈  연동 후 처리  부", state);
		new_guide();


		if (pScenario->m_PaySysCd < 0)
		{// 연동을 아예 하지 못함
			info_printf(localCh, "ALLAT_WOWTV_Quick_payARS [%d]  결제 연동 시스템 장애", state);
			eprintf("ALLAT_WOWTV_Quick_payARS [%d]  결제 연동 시스템 장애", state);
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		//연동 성공 시 로그 쌓고

		if (strcmp(pScenario->m_szURL_YN, "Y") == 0)
		{
			memset(pScenario->m_szSHOP_RET_URL_AG, 0x00, sizeof(pScenario->m_szSHOP_RET_URL_AG));
			setPostfunc(POST_NET, ALLAT_payARS, 70, 0);
			char szMx_id[32 + 1] = { 0x00, };
			strncpy_s(szMx_id, sizeof(szMx_id), pScenario->m_szMx_id, sizeof(szMx_id) - 1);
			_strupr_s(szMx_id, sizeof(szMx_id));
			if (strcmp(szMx_id, "DASAM") == 0)
			{
				memcpy(pScenario->m_szSHOP_RET_URL_AG, (const char *)pScenario->m_szSHOP_RET_URL, sizeof(pScenario->m_szSHOP_RET_URL_AG) - 1);
			}
			else if (strlen(pScenario->m_szSHOP_RET_URL) >1)
			{
				CreateAg();
			}
			else
			{
				memcpy(pScenario->m_szSHOP_RET_URL_AG, "http://handsome.hosting.paran.com/php/6_1.php?name=ELSE", sizeof(pScenario->m_szSHOP_RET_URL_AG) - 1);
			}
			return AllatPayRetPrc_host(92);
		}

	case 70:
		setPostfunc(POST_NET, ALLAT_payARS, 1, 0);
		return setPayLog_host(92);
	case 1://연동 성공 시 로그 쌓기 후 처리부
		new_guide();
		if (pScenario->m_PayResult < 0)
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_payARS [%d]  결제 연동 후 로그 DB 기록 부 >  로그 DB 시스템 장애", state);
			eprintf("ALLAT_WOWTV_Quick_payARS [%d]  결제 연동 후 로그 DB 기록 부 >  로그 DB 시스템 장애", state);
			// 자동으로 결제 취소
			// 결제믐 
			if (strcmp(pScenario->m_CardResInfo.m_szRESULTCODE, "0000") == 0)
			{
				memset(&pScenario->m_Card_CancleInfo, 0x00, sizeof(Card_CancleInfo));
				//DB로부터 획득 시 해당 하는 값으로 한다.
				memcpy(pScenario->m_Card_CancleInfo.m_AMT, pScenario->m_CardResInfo.m_AMT, sizeof(pScenario->m_Card_CancleInfo.m_AMT) - 1);
				pScenario->m_Card_CancleInfo.m_PartialCancelCode[0] = '0';
				memcpy(pScenario->m_Card_CancleInfo.m_szCancelPwd, pScenario->m_sz_Shop_Pw, sizeof(pScenario->m_Card_CancleInfo.m_szCancelPwd) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szGOODSNAME, pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_Card_CancleInfo.m_szGOODSNAME) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_Card_CancleInfo.m_szMid) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szMoid, pScenario->m_CardResInfo.m_szMoid, sizeof(pScenario->m_Card_CancleInfo.m_szMoid) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_Tid, pScenario->m_CardResInfo.m_Tid, sizeof(pScenario->m_Card_CancleInfo.m_Tid) - 1);

				setPostfunc(POST_NET, ALLAT_payARS, 90, 0);
				return AllatPayCancle_host(92);
			}
			else
			{
				set_guide(399);
				setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
				return send_guide(NODTMF);

			}
		}
		else if (pScenario->m_PayResult == 0)
		{  // 결제 로그 쌓기 실패 후 처리 부 결제 연동 실패로 처리
			info_printf(localCh, "ALLAT_WOWTV_Quick_payARS [%d]  결제 연동 후 로그 DB 기록 부 >  로그 DB 적재 실패", state);
			eprintf("ALLAT_WOWTV_Quick_payARS [%d]  결제 연동 후 로그 DB 기록 부 >  로그 DB 적재 실패", state);
			// 자동으로 결제 취소
			if (strcmp(pScenario->m_CardResInfo.m_szRESULTCODE, "0000") == 0)
			{// 로그 쌓기는 실패나 결제 성공 시 자동으로 취소 처리 한다.
				memset(&pScenario->m_Card_CancleInfo, 0x00, sizeof(Card_CancleInfo));
				//DB로부터 획득 시 해당 하는 값으로 한다.
				memcpy(pScenario->m_Card_CancleInfo.m_AMT, pScenario->m_CardResInfo.m_AMT, sizeof(pScenario->m_Card_CancleInfo.m_AMT) - 1);
				pScenario->m_Card_CancleInfo.m_PartialCancelCode[0] = '0';
				memcpy(pScenario->m_Card_CancleInfo.m_szCancelPwd, pScenario->m_sz_Shop_Pw, sizeof(pScenario->m_Card_CancleInfo.m_szCancelPwd) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szGOODSNAME, pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_Card_CancleInfo.m_szGOODSNAME) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_Card_CancleInfo.m_szMid) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szMoid, pScenario->m_CardResInfo.m_szMoid, sizeof(pScenario->m_Card_CancleInfo.m_szMoid) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_Tid, pScenario->m_CardResInfo.m_Tid, sizeof(pScenario->m_Card_CancleInfo.m_Tid) - 1);

				setPostfunc(POST_NET, ALLAT_payARS, 91, 0);
				return AllatPayCancle_host(92);
			}
			else
			{
				set_guide(399);
				setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
				return send_guide(NODTMF);
			}
		}
		else
		{//로그 쌓기 성공 시
			new_guide();
			if (strcmp(pScenario->m_CardResInfo.m_szRESULTCODE, "0000") != 0)
			{// 연동을 성공 다만 결제가 실제로 실패 되었으므로 자동 취소할 이유 없다.
				if (TTS_Play)
				{
					setPostfunc(POST_NET, ALLAT_payARS, 91, 0);
					return TTS_Play((*lpmt)->chanID, 92, "고객님, %s 이유로 인해 ", pScenario->m_CardResInfo.m_szRESULTMSG);
				}
				else
				{
					set_guide(399);
					setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
					return send_guide(NODTMF);
				}
			}
			else
			{// 결제 성공 시
				// 주문 정보 수정 결제 완료로 
				setPostfunc(POST_NET, ALLAT_payARS, 80, 0);
				return upOrderPayState_host(92); // 결제 성공 후 처리 ==> 주문 상태 바꾸기
			}

		}
	case 80://결제 성공 후 처리 ==> 주문 상태 바꾸기
		if (pScenario->m_PayResult < 0)
		{//주문 상태 바꾸기 장애
			info_printf(localCh, "ALLAT_WOWTV_Quick_payARS [%d] 결제 연동 후 로그 DB 기록 부 >  주문 정보 결제 상태 기록 시스템 장애", state);
			eprintf("ALLAT_WOWTV_Quick_payARS [%d] 결제 연동 후 로그 DB 기록 부 >  주문 정보 결제 상태 기록 시스템 장애", state);
			// 자동으로 결제 취소
			// 결제믐 
			if (strcmp(pScenario->m_CardResInfo.m_szRESULTCODE, "0000") == 0)
			{
				memset(&pScenario->m_Card_CancleInfo, 0x00, sizeof(Card_CancleInfo));
				//DB로부터 획득 시 해당 하는 값으로 한다.
				memcpy(pScenario->m_Card_CancleInfo.m_AMT, pScenario->m_CardResInfo.m_AMT, sizeof(pScenario->m_Card_CancleInfo.m_AMT) - 1);
				pScenario->m_Card_CancleInfo.m_PartialCancelCode[0] = '0';
				memcpy(pScenario->m_Card_CancleInfo.m_szCancelPwd, pScenario->m_sz_Shop_Pw, sizeof(pScenario->m_Card_CancleInfo.m_szCancelPwd) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szGOODSNAME, pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_Card_CancleInfo.m_szGOODSNAME) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_Card_CancleInfo.m_szMid) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szMoid, pScenario->m_CardResInfo.m_szMoid, sizeof(pScenario->m_Card_CancleInfo.m_szMoid) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_Tid, pScenario->m_CardResInfo.m_Tid, sizeof(pScenario->m_Card_CancleInfo.m_Tid) - 1);

				setPostfunc(POST_NET, ALLAT_payARS, 90, 0);
				return AllatPayCancle_host(92);
			}
			else
			{
				set_guide(399);
				setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
				return send_guide(NODTMF);

			}
		}
		else if (pScenario->m_PayResult == 0)
		{  // 주문 상태 바꾸기 실패 후 처리 ==> 결제 연동 실패로 간주하여 처리
			info_printf(localCh, "ALLAT_WOWTV_Quick_payARS [%d] 결제 연동 후 로그 DB 기록 부 >  주문 정보 결제 상태 기록 변경 실패", state);
			eprintf("ALLAT_WOWTV_Quick_payARS [%d] 결제 연동 후 로그 DB 기록 부 >  주문 정보 결제 상태 기록 변경 실패", state);
			// 자동으로 결제 취소
			if (strcmp(pScenario->m_CardResInfo.m_szRESULTCODE, "0000") == 0)
			{//주문 상태 바꾸기 후 처리 ==> 실제 결제 연동 성공 시 자동으로 취소 처리
				memset(&pScenario->m_Card_CancleInfo, 0x00, sizeof(Card_CancleInfo));
				//DB로부터 획득 시 해당 하는 값으로 한다.
				memcpy(pScenario->m_Card_CancleInfo.m_AMT, pScenario->m_CardResInfo.m_AMT, sizeof(pScenario->m_Card_CancleInfo.m_AMT) - 1);
				pScenario->m_Card_CancleInfo.m_PartialCancelCode[0] = '0';
				memcpy(pScenario->m_Card_CancleInfo.m_szCancelPwd, pScenario->m_sz_Shop_Pw, sizeof(pScenario->m_Card_CancleInfo.m_szCancelPwd) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szGOODSNAME, pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_Card_CancleInfo.m_szGOODSNAME) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_Card_CancleInfo.m_szMid) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_szMoid, pScenario->m_CardResInfo.m_szMoid, sizeof(pScenario->m_Card_CancleInfo.m_szMoid) - 1);
				memcpy(pScenario->m_Card_CancleInfo.m_Tid, pScenario->m_CardResInfo.m_Tid, sizeof(pScenario->m_Card_CancleInfo.m_Tid) - 1);

				setPostfunc(POST_NET, ALLAT_payARS, 91, 0);
				return AllatPayCancle_host(92);
			}
			else
			{
				set_guide(399);
				setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
				return send_guide(NODTMF);
			}
		}
		else
		{
			new_guide();
			set_guide(VOC_WAVE_ID, "ment/_common/common_audio/pay_success_msg");// 결제가 완료 되었습니다.
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	case 90:
		new_guide();
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	case 91: //로그 DB 적재 실패 치
	{
		new_guide();
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_payARS [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_payARS [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리(TTS)
		}
		set_guide(VOC_WAVE_ID, "ment/_common/common_audio/pay_fail_msg");// 결제 실패
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////
// CardPw : 비밀 번호 입력부
////////////////////////////////////////////////////////////////////////////////
extern SSL_CTX *g_ctx = NULL;

int ALLAT_CardPw(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_CardPw;
		(*lpmt)->prevState = state;
	}

	switch (state)
	{
	case 0:
		info_printf(localCh, "ALLAT_CardPw [%d] 비밀번호 입력 부", state);
		eprintf("ALLAT_CardPw [%d] 비밀번호 입력 부", state);
		new_guide();
		set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_pass_start");	 // "카드 비밀번호 네자리중, 앞, 두자리를 입력하여 주시기 바랍니다.
		setPostfunc(POST_DTMF, ALLAT_CardPw, 1, 0);
		return send_guide(2);
	case 1:
		if ((check_validkey("1234567890")) < 0)
		{
			eprintf("ALLAT_CardPw [%d] 비밀번호 입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}
		new_guide();
		memset(pScenario->m_CardInfo.Password, 0x00, sizeof(pScenario->m_CardInfo.Password));
		strncpy_s(pScenario->m_CardInfo.Password, sizeof(pScenario->m_CardInfo.Password),(*lpmt)->dtmfs, sizeof(pScenario->m_CardInfo.Password) - 1);
		info_printf(localCh, "ALLAT_CardPw [%d]  Card 번호 입력부>결제 연동 부", state);
		eprintf("ALLAT_CardPw[%d]  Card 번호 입력부>결제 연동 부", state);
		setPostfunc(POST_NET, ALLAT_payARS, 0, 0);

	default:
		info_printf(localCh, "ALLAT_CardPw [%d]  Card 번호 입력부>시나리오 아이디 오류", state);
		eprintf("ALLAT_CardPw[%d]  Card 번호 입력부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// ALLAT_InstallmentCConfrim : 할부 개월수 확인 및 입력 부
////////////////////////////////////////////////////////////////////////////////

int ALLAT_InstallmentCConfrim(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_InstallmentCConfrim;
		(*lpmt)->prevState = state;
	}
	switch (state)
	{
	case 0:
		info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부", state);
		eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부", state);
		// 2016.05.24
		// 50000미만의 경우 바로 결제 연동 한다.
		if (pScenario->m_nAmount < (int)GetPrivateProfileInt("ALLAT_PAYMEMT", "ALLAT_MIN_AMT", 50000, PARAINI))
		{
			memset(pScenario->m_CardInfo.InstPeriod, 0x00, sizeof(pScenario->m_CardInfo.InstPeriod));
			sprintf_s(pScenario->m_CardInfo.InstPeriod,sizeof(pScenario->m_CardInfo.InstPeriod), "%02d", 0);
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d]  Card 번호 입력부>결제 연동 부", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d]  Card 번호 입력부>결제 연동 부", state);
			setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
			return AllatPaymemt_host(90);
		}
		new_guide();
		set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_halbu_start");	 // "요청하실 할부 개월수를 ... 일시불은 0번...."
		//1월  "신한,국민,현대,하나,삼성카드는 최대 3개월까지. 비씨카드는 최대 4개월까지 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리비씨카드는 비씨카드사의 무이자 정책이 적용됩니다.
		//10월  "현대,하나, 국민카드는 최대 3개월까지. 신한, 삼성, 비씨카드는 최대 5개월까지 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리비씨카드는 비씨카드사의 무이자 정책이 적용됩니다.
		//9월  "현대,하나, 국민카드는 최대 3개월까지. 신한, 삼성, 비씨카드는 최대 5개월까지 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리비씨카드는 비씨카드사의 무이자 정책이 적용됩니다.
		//8월  "현대,하나카드, 국민카드는 최대 3개월까지. 신한, 삼성, 롯데, 비씨카드는 최대 5개월까지 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리비씨카드는 비씨카드사의 무이자 정책이 적용됩니다.
		//7월  "신한,국민, 현대,하나,삼성, 롯데카드는 최대 3개월까지. 비씨카드는 최대 6개월까지 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리비씨카드는 비씨카드사의 무이자 정책이 적용됩니다.
		//6월  "신한,국민, 현대,하나카드는 최대 3개월까지. 비씨카드는 최대 4개월까지. 롯데, 삼성 카드는 최대 5개월까지 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리비씨카드는 비씨카드사의 무이자 정책이 적용됩니다.
		//2025.6월  "하나, 국민카드는 최대 3개월까지. 비씨카드는 최대 4개월까지. 삼성, 신한, 현대카드는 최대 5개월까지 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리비씨카드는 비씨카드사와 별개로, 무이자 대상에서 제외됩니다.
		//2025.6월  "신한, 국민, 비씨, 현대, 삼성카드는 최대 3개월까지. 하나, 롯데카드는 최대 5개월까지, 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리카드는 BC카드와 별도정책으로 운영되고 있습니다.
		//2025.6월  "신한, 현대, 삼성카드는 최대 3개월까지. 하나, 롯데카드는 최대 5개월까지, 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리카드는 BC카드와 별도정책으로 운영되고 있습니다.
		//2025.9월  "현대, 국민, 비씨카드는 최대 3개월까지. 신한, 삼성, 하나카드는 최대 5개월까지, 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리카드는 BC카드와 별도정책으로 운영되고 있습니다.
		//2025.11월 "현대, 국민, 신한, 삼성카드는 최대 3개월까지. 하나카드는 최대 4개월까지. 롯데카드는 최대 5개월까지 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리카드는 BC카드와 별도정책으로 운영되고 있습니다.
		//2025.12월 "신한, 삼성, 국민, 현대카드는 최대 3개월까지. 하나카드는 최대 4개월까지, 무이자할부가 가능합니다. 단, NH농협카드는 결제가 불가하며, 우리카드는 BC카드와 별도정책으로 운영되고 있습니다..
		set_guide(VOC_WAVE_ID, "ment/ALLAT_Hangung/input_halbu_start_ilsibul");
		setPostfunc(POST_DTMF, ALLAT_InstallmentCConfrim, 1, 0);
		return send_guide(3);
	case 1:
		// 2016.05.24
		// 50000미만의 경우 바로 결제 연동 한다.
		if (pScenario->m_nAmount < (int)GetPrivateProfileInt("ALLAT_PAYMEMT", "ALLAT_MIN_AMT", 50000, PARAINI))
		{
			memset(pScenario->m_CardInfo.InstPeriod, 0x00, sizeof(pScenario->m_CardInfo.InstPeriod));
			sprintf_s(pScenario->m_CardInfo.InstPeriod,sizeof(pScenario->m_CardInfo.InstPeriod), "%02d", 0);
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d]  Card 번호 입력부>결제 연동 부", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d]  Card 번호 입력부>결제 연동 부", state);
			setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
			return AllatPaymemt_host(90);
		}
		if ((check_validform("*#:1:2", (*lpmt)->refinfo)) < 0)
		{
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > 할부 개월 수  입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}

		new_guide();
		if ((atoi((*lpmt)->refinfo) == 0) || (atoi((*lpmt)->refinfo) == 1))
		{//일시불을 요청하셨습니다.
			memset(pScenario->m_szInstallment, 0x00, sizeof(pScenario->m_szInstallment));
			set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_nohalbu_msg");
		}
		else if (atoi((*lpmt)->refinfo) > 12 ||
			(atoi((*lpmt)->refinfo) < 2))
		{//일시불과 이개월에서 십이개월 사이의 할부를 지원합니다.-
			set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_halbu_err");
			set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_halbu_start");//"요청하실 할부 개월수를
			// "롯데카드는 최대 4개월까지. 삼성, 농협, 비씨, 신한, 국민카드는 최대 6개월까지. 현대카드는 최대 7개월까지. 하나카드는 최대 8개월까지 무이자할부가 가능합니다. 단, 비씨카드 중 비씨하나카드는 무이자할부가 적용되지 않습니다."
			// "롯데카드는 최대 4개월까지. 삼성, 농협, 비씨, 신한, 국민카드는 최대 6개월까지. 현대카드는 최대 7개월까지. 하나카드는 최대 8개월까지 무이자할부가 가능합니다. 단, 우리비씨카드는 비씨카드사의 무이자 정책이 적용됩니다."
			set_guide(VOC_WAVE_ID, "ment/ALLAT_Hangung/input_halbu_start_ilsibul");
			setPostfunc(POST_DTMF, ALLAT_InstallmentCConfrim, 1, 0);
			return send_guide(3);
		}
		else
		{
			if (TTS_Play)
			{
				memset(pScenario->m_szInstallment, 0x00, sizeof(pScenario->m_szInstallment));
				memcpy(pScenario->m_szInstallment, (*lpmt)->refinfo, sizeof(pScenario->m_szInstallment) - 1);
				setPostfunc(POST_NET, ALLAT_InstallmentCConfrim, 3, 0);
				return TTS_Play((*lpmt)->chanID, 92, "고객님께서 요청하신 할부 개월수는 %d개월 입니다",
					atoi(pScenario->m_szInstallment));
			}
			else
			{
				set_guide(399);
				setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
				return send_guide(NODTMF);
			}

		}
	case 30:
		info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 입력 할부 개월 수 확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 입력 할부 개월 수 확인 부", state);
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_confirm");
		setPostfunc(POST_DTMF, ALLAT_InstallmentCConfrim, 4, 0);
		return send_guide(1);
		return 0;

	case 3:
		info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 입력 할부 개월 수 확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 입력 할부 개월 수 확인 부", state);
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_confirm");
		setPostfunc(POST_DTMF, ALLAT_InstallmentCConfrim, 4, 0);
		return send_guide(1);
		return 0;
	case 4:
		if (!check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > 입력 할부 개월 수 확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		memset(pScenario->m_CardInfo.InstPeriod, 0x00, sizeof(pScenario->m_CardInfo.InstPeriod));
		if (c == '1') //예
		{
			sprintf_s(pScenario->m_CardInfo.InstPeriod,sizeof(pScenario->m_CardInfo.InstPeriod), "%02d", atoi(pScenario->m_szInstallment));
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > 입력 할부 개월 수 확인 부>맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > 입력 할부 개월 수 확인 부>맞습니다.", state);

			//return ALLAT_CardPw(0);// 올렛의 요청에 의거하여 비밀번호 입력 받지 않는다.
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d]  Card 번호 입력부>결제 연동 부", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d]  Card 번호 입력부>결제 연동 부", state);

			setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
			return AllatPaymemt_host(90);
		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > 입력 할부 개월 수 확인 부>아니오", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > 입력 할부 개월 수 확인 부>아니오", state);

			return ALLAT_InstallmentCConfrim(0);
		}
	case 9:
		info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 할부 개월 수 확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 할부 개월 수 확인 부", state);
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);	 // "할부 개월수 안내"
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리

		}
		memset(pScenario->m_CardInfo.InstPeriod, 0x00, sizeof(pScenario->m_CardInfo.InstPeriod));
		sprintf_s(pScenario->m_CardInfo.InstPeriod,sizeof(pScenario->m_CardInfo.InstPeriod), "%02d", 0);
		setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
		return AllatPaymemt_host(90);
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 할부 개월 수 확인 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d]  할부 개월 수 확인 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// JuminNo : 주민 번호 또는 법인 번호 입력부
////////////////////////////////////////////////////////////////////////////////
int ALLAT_JuminNo(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_JuminNo;
		(*lpmt)->prevState = state;
	}

	switch (state)
	{
	case 0:
		info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부", state);
		eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부", state);

		new_guide();
		set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_regno_front");	 // "생년 월일......"
		setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
		return send_guide(11);
	case 1:
		if ((check_validform("*#:6:10", (*lpmt)->refinfo)) < 0)
		{
			eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}

		if (strlen((*lpmt)->refinfo) > 6 &&
			strlen((*lpmt)->refinfo) != 10)
		{
			eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부>잘못 누르셨습니다( 6 && != 10 .....", state);
			return send_error();
		}
		// 생년월일 6자리로 입력 받을 경우, 1900년대와 2000년 대의 윤년을 특정하지 못한다.
		// 이에 따른 형식 체크가 무의미 해진다.
		// 또한 음력 생년월일 이용자도 존재 함으로써, 그에 따른 윤달 채크 및 윤년 체크가 되지 않는다.
		// 따라서 일자가 31을 초과 여부만 따질 수 있다.
		if (strlen((*lpmt)->refinfo) == 6)
		{
			int nDay = atoiN(&((*lpmt)->refinfo[4]), 2);

			if (nDay < 1 || nDay>31)
			{
				eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부(일자 체크부) >잘못 누르셨습니다.....", state);
				return send_error();
			}
		}
		new_guide();

		info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부>확인 부(TTS)", state);
		eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부>확인 부(TTS)", state);
		if (TTS_Play)
		{
			char TTSBuf[1024 + 1] = { 0x00, };
			int TTsLen = strlen((*lpmt)->refinfo);
			for (int nRep = 0, nRep2 = 0;; nRep++)
			{
				if (TTsLen < 1) break;
				TTSBuf[nRep2++] = (char)*((*lpmt)->refinfo + nRep);
				TTSBuf[nRep2++] = ',';
				TTsLen--;
			}
			setPostfunc(POST_NET, ALLAT_JuminNo, 2, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님께서 누르신 생년월일 또는 법인 번호는, %s 번 입니다.", TTSBuf);
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_JuminNo, 0, 0);
			return send_guide(NODTMF);
		}
	case 2:
		info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d] 주민 / 법인 입력부>확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] 주민 / 법인 입력부>확인 부", state);
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);
			eprintf("a4one_JuminNo [%d] 주민 / 법인 입력부>확인 부>%s", state, TTSFile);
			set_guide(VOC_TTS_ID, TTSFile);	 // "누르신 유효 기간은  XX년 XX월 입니다."
			set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_JuminNo, 3, 0);
		return send_guide(1);
	case 3:
		if (!check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		memset(pScenario->m_CardInfo.SecretNo, 0x00, sizeof(pScenario->m_CardInfo.SecretNo));
		if (c == '1') //예
		{
			strncpy_s(pScenario->m_CardInfo.SecretNo,sizeof(pScenario->m_CardInfo.SecretNo), (*lpmt)->refinfo, sizeof(pScenario->m_CardInfo.SecretNo) - 1);
			info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>맞습니다.", state);

			if ((strlen(pScenario->m_szInstallment) > 0) &&
				(atoi(pScenario->m_szInstallment)) > 0)
			{
				if (TTS_Play)
				{
					memset(pScenario->m_szInstallment, 0x00, sizeof(pScenario->m_szInstallment));
					memcpy(pScenario->m_szInstallment, "00", sizeof(pScenario->m_szInstallment) - 1);
					setPostfunc(POST_NET, ALLAT_InstallmentCConfrim, 3, 0);
					return TTS_Play((*lpmt)->chanID, 92, "고객님께서 요청하신 할부 개월수는 %d개월 입니다",
						atoi(pScenario->m_szInstallment));
				}
				else
				{
					set_guide(399);
					setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
					return send_guide(NODTMF);
				}
			}
			return ALLAT_InstallmentCConfrim(0);
		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>아니오", state);
			eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>아니오", state);

			return ALLAT_JuminNo(0);
		}
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d]  주민 / 법인 입력 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_JuminNo[%d]  주민 / 법인 입력 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// EffecDate : 카드 유효 기간 입력부
////////////////////////////////////////////////////////////////////////////////
int ALLAT_EffecDate(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_EffecDate;
		(*lpmt)->prevState = state;
	}

	switch (state)
	{
	case 0:
		info_printf(localCh, "ALLAT_WOWTV_Quick_EffecDate [%d] 유효 기간 입력 부", state);
		eprintf("ALLAT_WOWTV_Quick_EffecDate [%d] 유효 기간 입력 부", state);

		new_guide();
		// 2018.01.20
		// 정종택 요청
		//set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_cardexp_start");	 // "신용카드 앞 유효기간 년도와 월 순으로......"
		set_guide(VOC_WAVE_ID, "ment/ALLAT_Hangung/input_cardexp_start");	  // "신용카드 유효기간 4자리를, 카드에 표기된대로 월,연 순서로 입력하여 주십시오."
		setPostfunc(POST_DTMF, ALLAT_EffecDate, 1, 0);
		return send_guide(4);
	case 1:
	{
		int    tmpMonth, tmpYear;
		CTime curTime = CTime::GetCurrentTime();//GetTickCount

		// 2018.01.20
		// 정종택 요청 년월 ==> 월년순으로 입력
		//tmpMonth = atoiN(&((*lpmt)->dtmfs[2]), 2);//월
		//tmpYear = atoiN(&((*lpmt)->dtmfs[0]), 2);//년
		tmpMonth = atoiN(&((*lpmt)->dtmfs[0]), 2);//월
		tmpYear = atoiN(&((*lpmt)->dtmfs[2]), 2);//년
		tmpYear += 2000;

		if ((check_validkey("1234567890")) < 0)
		{
			eprintf("ALLAT_WOWTV_Quick_EffecDate [%d] 유효 기간 입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}

		if ((tmpMonth<1 || tmpMonth> 12) ||
			(tmpYear<curTime.GetYear()) ||       // 올해보다 이전의 년도는 이미 유효기간이 지났으므로 결제할 수 없다.
			(check_validkey && (check_validkey("1234567890")) < 0))
		{
			eprintf("ALLAT_WOWTV_Quick_EffecDate [%d] 일자 오류 유효 기간 입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}

		//2016.05.24
		//유효기간이 현재 년월보다 이전일 경우 오류 처리
		if ((tmpYear == curTime.GetYear()) &&
			(tmpMonth < curTime.GetMonth()))
		{
			eprintf("ALLAT_WOWTV_Quick_EffecDate [%d] 일자 오류 유효 기간 입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}

		new_guide();

		// 유호기간을 월년으로 바꾸어야 한다.나이스는...	
		memset(pScenario->m_CardInfo.ExpireDt, 0x00, sizeof(pScenario->m_CardInfo.ExpireDt));
		// 2018.01.20
		// 정종택 요청
		//memcpy(pScenario->m_CardInfo.ExpireDt, (*lpmt)->dtmfs, sizeof(pScenario->m_CardInfo.ExpireDt) - 1);
		sprintf_s(pScenario->m_CardInfo.ExpireDt, "%02d%02d", atoiN(&((*lpmt)->dtmfs[2]), 2), atoiN(&((*lpmt)->dtmfs[0]), 2));

		info_printf(localCh, "ALLAT_WOWTV_Quick_EffecDate [%d] 유효 기간 입력 부>확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_EffecDate [%d] 유효 기간 입력 부>확인 부", state);
		if (TTS_Play)
		{
			setPostfunc(POST_NET, ALLAT_EffecDate, 2, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님께서 누르신 유효 기간은, 20%d 년, %d월 입니다.", atoiN(&((*lpmt)->dtmfs[2]), 2), atoiN(&((*lpmt)->dtmfs[0]), 2));
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	}
	case 2:
		info_printf(localCh, "ALLAT_WOWTV_Quick_EffecDate [%d] Card 번호 입력부>확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_EffecDate [%d] Card 번호 입력부>확인 부", state);
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);	 // "누르신 유효 기간은  XX년 XX월 입니다."
			set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_EffecDate, 3, 0);
		return send_guide(1);
	case 3:
		if (!check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_EffecDate [%d] 유효 기간 입력 부>확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();
		if (c == '1') //예
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_EffecDate[%d] 유효 기간 입력 부>확인 부>맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_EffecDate[%d] 유효 기간 입력 부>확인 부>맞습니다.", state);

			//return ALLAT_JuminNo(0); // 올렛의 요청에 의거하여 주민번호 입력 받지 않는다.
			return ALLAT_InstallmentCConfrim(0);// 할부 기간으로 간다.
		}
		else if (c == '2')//아니오dl
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_EffecDate[%d] 유효 기간 입력 부>확인 부>아니오", state);
			eprintf("ALLAT_WOWTV_Quick_EffecDate[%d] 유효 기간 입력 부>확인 부>아니오", state);

			return ALLAT_EffecDate(0);
		}
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_EffecDate [%d]  유효 기간 입력 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_EffecDate[%d]  유효 기간 입력 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}

	return 0;

}


/////////////////////////////////////////////////////////////////////////////////
// CardInput : 카드 번호
////////////////////////////////////////////////////////////////////////////////
int ALLAT_CardInput(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_CardInput;
		(*lpmt)->prevState = state;
	}

	switch (state)
	{
	case 0:
		info_printf(localCh, "ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부", state);
		eprintf("ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부", state);

		if (new_guide)new_guide();
		//AHN 20220720 와우넷에서 농협카드가 결제 안되므로 멘트 추가 요청함.  
		set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_card_num_var_nonghyup");	 // "카드번호 ..우물정자를..."
		//set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_card_num_var");	 // "카드번호 ..우물정자를..."
		setPostfunc(POST_DTMF, ALLAT_CardInput, 1, 0);
		return send_guide(17);
	case 1:
		if ((check_validform) && (check_validform("*#:13:16", (*lpmt)->refinfo)) < 0)	// 눌린 dtmf값이 8~12의 길이이며 *또는#으로 끝이났다.
		{
			eprintf("ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부>잘못 누르셨습니다.....", state);
			return send_error();
		}
		new_guide();

		info_printf(localCh, "ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부>확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부>확인 부", state);
		if (TTS_Play)
		{
			char TTSBuf[1024 + 1] = { 0x00, };
			int TTsLen = strlen((*lpmt)->refinfo);
			for (int nRep = 0, nRep2 = 0;; nRep++)
			{
				if (TTsLen < 1) break;
				TTSBuf[nRep2++] = (char)*((*lpmt)->refinfo + nRep);
				TTSBuf[nRep2++] = ',';
				TTsLen--;
			}


			setPostfunc(POST_NET, ALLAT_CardInput, 2, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님께서 누르신 카드번호는, %s 번 입니다.", TTSBuf);
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	case 2:
		info_printf(localCh, "ALLAT_WOWTV_Quick_CardInput [%d] Card 번호 입력부>확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_CardInput [%d] Card 번호 입력부>확인 부", state);
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_CardInput [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_CardInput [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);	 // "누르신 카드번호는 .. 번입니다."
			set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_CardInput, 3, 0);
		return send_guide(1);
	case 3:
		if (check_validdtmf&& !check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부>확인 부> 절못 누르셨습니다.", state);
			return send_error();
		}
		if (new_guide) new_guide();

		memset(pScenario->m_CardInfo.Card_Num, 0x00, sizeof(pScenario->m_CardInfo.Card_Num));
		if (c == '1') //예
		{
			strncpy_s(pScenario->m_CardInfo.Card_Num,sizeof(pScenario->m_CardInfo.Card_Num), (*lpmt)->refinfo, sizeof(pScenario->m_CardInfo.Card_Num) - 1);
			info_printf(localCh, "ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부>확인 부> 확인 부>맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부>확인 부> 확인 부>맞습니다.", state);

			return ALLAT_EffecDate(0);// 유효기간 입력부
		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부>확인 부> 확인 부>아니오", state);
			eprintf("ALLAT_WOWTV_Quick_CardInput[%d]  Card 번호 입력부>확인 부> 확인 부>아니오", state);

			return ALLAT_CardInput(0);
		}
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_CardInput [%d]  Card 번호 입력부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_CardInput[%d]  Card 번호 입력부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}

	return 0;
}

int ALLAT_getOrderInfo(/* [in] */int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		//PrevCall = (*lpmt)->PrevCall;
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_getOrderInfo;
		(*lpmt)->prevState = state;
	}

	switch (state)
	{
	case 0:
		new_guide();
		info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부", state);
		eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부", state);
		if (pScenario->m_DBAccess == -1 || pScenario->m_bDnisInfo == -1)
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 주문 정보 시스템 장애", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 주문 정보 시스템 장애", state);
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (pScenario->m_bDnisInfo < 0)
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 주문 정보 시스템 송수신 에러", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 주문 정보 시스템 송수신 에러", state);
			set_guide(VOC_WAVE_ID, "ment\\ALLAT_WOWTV\\Tcp_Error");
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		else if (pScenario->m_bDnisInfo == 0)
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 존재하지 않은 주문 정보", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 존재하지 않은 주문 정보", state);
			new_guide();

			// CIA의 경우, ANI 정보로 정보 획득 되나...
			// ANI 정보로 한번 무조건 획득하는 구조가 아닌
			// ANI 정보가 휴대폰 인지 검증후, 입력 받도록 하고 또한 휴대 전화 검증 한다.
			// 휴대 전화의 경우, 신규 고객의 경우, 무조건 신규 등록 후, 해당 정보가 획득 되어
			// 재입력부가 없다.
			// 또한 주문 정보가 없는 경우, 방송 시간 종료 이므로 , 재 입력하여도 무조건 주문 정보 
			// 없으니...
			// 해당 재입력 할 이유 없다.
			set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\no_order_msg");	 // "주문이 접수되지 않았습니다. 상점으로 문의하여 주시기 바랍니다."

			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		else if (strcmp(pScenario->m_szretval,"6020")==0)
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > %s", state, pScenario->m_szretMsg);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > %s", state, pScenario->m_szretMsg);
			new_guide();

			setPostfunc(POST_NET, ALLAT_getOrderInfo, 20, 0);
			return TTS_Play((*lpmt)->chanID, 92, "%s 측에서는 보유하고 계신 와우캐시로 정상적으로 결제되었습니다.", pScenario->m_szpsrtner_nm);			
		}
		info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 동의서  안내 부> 파트너 면(%s)", state,
			pScenario->m_szpsrtner_nm);
		eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 동의서 안내 부> 파트너명(%s)", state,
			pScenario->m_szpsrtner_nm);
		if (TTS_Play)
		{
			setPostfunc(POST_NET, ALLAT_getOrderInfo, 1, 0);
			//return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서는 %s에서, 결제하실 금액은 %d원 입니다.",
			//2017.01.24
			//한국경제WOWTV 김상기 차창의 요청
			//전체를 TTS로 하며, 해지에 관련된 내용을 재생하도록 한다.
			return TTS_Play((*lpmt)->chanID, 92, "한국경제TV를 통해 제공되는, 각종 정보제공 및 기타 부가서비스를 위하여, %s에게, 고객님의 아이디, 필명, 휴대폰전화번호, 가입기간을, 해당 서비스 종료일 까지, 열람및 이용할 수있도록 정보제공되는것에, 동의하셔야 합니다. 동의하시겠습니까? 동의를 거부하실 권리가 있으며, 동의를 거부하실 경우, 해당 서비스를 이용하실 수 없습니다. 동의 하시려면 1번을, 동의 하지 않으시면 2번을 눌러주십시오!", pScenario->m_szpsrtner_nm);
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	case 1:
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 동의서 > 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 동의서 > 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile, sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);	 // "주문 상품 정보안내"
			//2017.01.24
			//한국경제WOWTV 김상기 차창의 요청
			//전체를 TTS로 하며, 해지에 관련된 내용을 재생하도록 한다.
			//삭제처리
			//set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_getOrderInfo, 2, 0);
		return send_guide(1);
	case 2:
		if (check_validdtmf && !check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 상품정보 안내> 확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		if (c == '1') //예
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo[%d] 동의  안내 부 > 동의 하셨습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d] 동의  안내 부 > 동의 하셨습니다.", state);

			return ALLAT_getOrderInfo(9);
		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo[%d] 동의  안내 부 > 동의 하시지 않으셨습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d] 동의  안내 부 > 동의 하시지 않으셨습니다.", state);


			if (strcmp(pScenario->szArsType, "ARS") == 0) return ALLAT_ArsScenarioStart(1);
			else if (strcmp(pScenario->szArsType, "SMS") == 0) return ALLAT_SMSScenarioStart(1);
			else if (strcmp(pScenario->szArsType, "CID") == 0) return  ALLAT_CID_ScenarioStart(1);
			else if (strcmp(pScenario->szArsType, "CIA") == 0) return  ALLAT_CIA_ScenarioStart(1);
			else return pScenario->jobArs(0);
		}
	case 9:
		if (TTS_Play)
		{
			setPostfunc(POST_NET, ALLAT_getOrderInfo, 10, 0);
			//return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서는 %s에서, 결제하실 금액은 %d원 입니다.",
			//2017.01.24
			//한국경제WOWTV 김상기 차창의 요청
			//전체를 TTS로 하며, 해지에 관련된 내용을 재생하도록 한다.
			return TTS_Play((*lpmt)->chanID, 92, "%s 고객님, %s에서 , 주문하신 %s의 결제하실 금액은 %d원 입니다. 서비스 중도해지시 해지일까지 일수 만큼의 이용요금과  가입비의 10퍼센트에 해당되는 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다. 동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
				pScenario->m_szCC_name,
				pScenario->m_szMx_name,
				pScenario->m_szCC_Prod_Desc,
				pScenario->m_nAmount);
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	case 10:
		info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 상품정보 안내(%s:%s:%s:%d)", state,
			pScenario->m_szCC_name,
			pScenario->m_szMx_name,
			pScenario->m_szCC_Prod_Desc,
			pScenario->m_nAmount);
		eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 상품정보 안내(%s:%s:%s:%d)> 확인 부", state,
			pScenario->m_szCC_name,
			pScenario->m_szMx_name,
			pScenario->m_szCC_Prod_Desc,
			pScenario->m_nAmount);

		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);	 // "주문 상품 정보안내"
			//2017.01.24
			//한국경제WOWTV 김상기 차창의 요청
			//전체를 TTS로 하며, 해지에 관련된 내용을 재생하도록 한다.
			//삭제처리
			//set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_getOrderInfo, 12, 0);
		return send_guide(1);
	case 12:
		if (check_validdtmf && !check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 상품정보 안내> 확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		if (c == '1') //예
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부>확인 부> 맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부>확인 부> 맞습니다.", state);

			return ALLAT_CardInput(0);
		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부>확인 부> 아니오", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부>확인 부> 아니오", state);


			if (strcmp(pScenario->szArsType, "ARS") == 0) return ALLAT_ArsScenarioStart(1);
			else if (strcmp(pScenario->szArsType, "SMS") == 0) return ALLAT_SMSScenarioStart(1);
			else if (strcmp(pScenario->szArsType, "CID") == 0) return  ALLAT_CID_ScenarioStart(1);
			else if (strcmp(pScenario->szArsType, "CIA") == 0) return  ALLAT_CIA_ScenarioStart(1);
			else return pScenario->jobArs(0);
		}
	case 20 :
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile, sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);	
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d]  고객 주문 정보 안내 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d]  고객 주문 정보 안내 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}

	return 0;
}

int ALLAT_ArsScenarioStart(/* [in] */int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_ArsScenarioStart;
		(*lpmt)->prevState = state;
	}
	//일단 CARD 결제 고정
	memset(pScenario->m_szpaymethod, 0x00, sizeof(pScenario->m_szpaymethod));
	memcpy(pScenario->m_szpaymethod, "CARD", sizeof(pScenario->m_szpaymethod) - 1);

	switch (state)
	{
	case 0:
	{
		char TempPath[1024 + 1] = { 0x00, };
		new_guide();
		info_printf(localCh, "ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 인사말...", state);
		eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 인사말", state);
		//sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\%s", (*lpmt)->dnis);
		sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\wownet_intro", (*lpmt)->dnis);
		set_guide(VOC_WAVE_ID, TempPath);	 // "인사말"
		setPostfunc(POST_PLAY, ALLAT_ArsScenarioStart, 1, 0);
		return send_guide(NODTMF);
	}

	case 1:
		new_guide();
		info_printf(localCh, "ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부...", state);
		eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부", state);
		set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\input_telnum_start");	 // "전화 번호 입력"
		setPostfunc(POST_DTMF, ALLAT_ArsScenarioStart, 2, 0);
		return send_guide(13);

	case 2:// 전화 번호 입력 처리
		if ((check_validform("*#:7:12", (*lpmt)->refinfo)) < 0)	// 눌린 dtmf값이 8~12의 길이이며 *또는#으로 끝이났다.
		{
			eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}
		if (strncmp((*lpmt)->dtmfs, "010", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "011", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "012", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "016", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "017", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "018", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "019", 3) != 0)
		{
			eprintf("Nice_ArsScenarioStart [%d] 고객 전화 번호 입력 부(형식 오류)>잘못 누르셨습니다.....", state);
			return send_error();
		}
		new_guide();
		memset(pScenario->m_szInputTel, 0x00, sizeof(pScenario->m_szInputTel));
		memcpy(pScenario->m_szInputTel, (*lpmt)->refinfo, sizeof(pScenario->m_szInputTel) - 1);

		info_printf(localCh, "ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부>확인 부(TTS)", state);
		eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부>확인 부(TTS)", state);
		if (TTS_Play)
		{
			char TTSBuf[1024 + 1] = { 0x00, };
			int TTsLen = strlen((*lpmt)->refinfo);
			for (int nRep = 0, nRep2 = 0;; nRep++)
			{
				if (TTsLen < 1) break;
				TTSBuf[nRep2++] = (char)*((*lpmt)->refinfo + nRep);
				TTSBuf[nRep2++] = ',';
				TTsLen--;
			}


			setPostfunc(POST_NET, ALLAT_ArsScenarioStart, 3, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님께서 누르신 전화번호는, %s 번 입니다.", TTSBuf);
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	case 3:
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);

			set_guide(VOC_TTS_ID, TTSFile);	 // 주문번호 확일을 위해 주문 번호 재생
			set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_ArsScenarioStart, 4, 0);
		return send_guide(1);
	case 4:
		if (check_validdtmf && !check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		if (c == '1') //예
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_ArsScenarioStart[%d] 고객 전화 번호 입력 부>확인 부> 확인 부>맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>맞습니다.", state);

			setPostfunc(POST_NET, ALLAT_getOrderInfo, 0, 0);
			return getOrderInfo_host(90);
		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>아니오", state);
			eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>아니오", state);

			return ALLAT_ArsScenarioStart(1);
		}

	case 0xffff:
		return  goto_hookon();
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_ArsScenarioStart [%d]  C고객 전화 번호 입력 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_ArsScenarioStart[%d]  고객 전화 번호 입력 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
	return 0;
}

int ALLAT_SMSScenarioStart(/* [in] */int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_SMSScenarioStart;
		(*lpmt)->prevState = state;
	}
	//일단 CARD 결제 고정
	memset(pScenario->m_szpaymethod, 0x00, sizeof(pScenario->m_szpaymethod));
	memcpy(pScenario->m_szpaymethod, "CARD", sizeof(pScenario->m_szpaymethod) - 1);

	switch (state)
	{
	case 0:
	{
		char TempPath[1024 + 1] = { 0x00, };
		new_guide();
		info_printf(localCh, "ALLAT_WOWTV_Quick_SMSScenarioStart [%d] 인사말...", state);
		eprintf("ALLAT_WOWTV_Quick_SMSScenarioStart [%d] 인사말", state);
		//sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\%s", (*lpmt)->dnis);
		sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\wownet_intro", (*lpmt)->dnis);
		set_guide(VOC_WAVE_ID, TempPath);	 // "인사말"
		setPostfunc(POST_PLAY, ALLAT_SMSScenarioStart, 1, 0);
		return send_guide(NODTMF);
	}
	case 1:
		new_guide();
		info_printf(localCh, "ALLAT_WOWTV_Quick_SMSScenarioStart [%d] SMS로 받은 주문 번호 입력 부...", state);
		eprintf("ALLAT_WOWTV_Quick_SMSScenarioStart [%d] SMS로 받은 주문 번호 입력 부", state);
		set_guide(VOC_WAVE_ID, "audio/input_sms_start");	 // "SMS로 받은 주문 번호 입력"
		setPostfunc(POST_DTMF, ALLAT_SMSScenarioStart, 2, 0);
		return send_guide(6);

	case 2:// 전화 번호 입력 처리
		if ((check_validkey("1234567890")) < 0)
		{
			eprintf("ALLAT_WOWTV_Quick_EffecDate [%d] 유효 기간 입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}

		new_guide();

		info_printf(localCh, "ALLAT_WOWTV_Quick_SMSScenarioStart [%d] SMS로 받은 주문 번호 입력 부>확인 부", state);
		eprintf("ALLAT_WOWTV_Quick_SMSScenarioStart [%d]SMS로 받은 주문 번호 입력 부>확인 부", state);

		memset(pScenario->m_szAuth_no, 0x00, sizeof(pScenario->m_szAuth_no));
		memcpy(pScenario->m_szAuth_no, (*lpmt)->dtmfs, sizeof(pScenario->m_szAuth_no) - 1);

		if (TTS_Play)
		{
			setPostfunc(POST_NET, ALLAT_SMSScenarioStart, 3, 0);
			return TTS_Play((*lpmt)->chanID, 92, "%c,%c,%c,%c,%c,%c, 번 입니다.",
				(*lpmt)->dtmfs[0],
				(*lpmt)->dtmfs[1],
				(*lpmt)->dtmfs[2],
				(*lpmt)->dtmfs[3],
				(*lpmt)->dtmfs[4],
				(*lpmt)->dtmfs[5]);
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	case 3:
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_SMSScenarioStart [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_SMSScenarioStart [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);

			set_guide(VOC_WAVE_ID, "audio/input_sms_msg"); //입력 하신 주문 번호는 
			set_guide(VOC_TTS_ID, TTSFile);	 // 주문번호 확일을 위해 주문 번호 재생
			set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_SMSScenarioStart, 4, 0);
		return send_guide(1);
	case 4:
		if (!check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_SMSScenarioStart [%d] SMS로 받은 주문 번호 입력 부>확인 부> 확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		if (c == '1') //예
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_SMSScenarioStart[%d] SMS로 받은 주문 번호 입력 부>확인 부> 확인 부>맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_SMSScenarioStart [%d] SMS로 받은 주문 번호 입력 부>확인 부> 확인 부>맞습니다.", state);

			setPostfunc(POST_NET, ALLAT_getOrderInfo, 0, 0);
			return getSMSOrderInfo_host(90);
		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_SMSScenarioStart [%d] SMS로 받은 주문 번호 입력 부>확인 부> 확인 부>아니오", state);
			eprintf("ALLAT_WOWTV_Quick_SMSScenarioStart [%d] SMS로 받은 주문 번호 입력 부>확인 부> 확인 부>아니오", state);

			return ALLAT_SMSScenarioStart(1);
		}

	case 0xffff:
		return  goto_hookon();
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_SMSScenarioStart [%d]  SMS로 받은 주문 번호 입력 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_SMSScenarioStart[%d]  SMS로 받은 주문 번호 입력 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
	return 0;
}

int ALLAT_CID_ScenarioStart(/* [in] */int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_CID_ScenarioStart;
		(*lpmt)->prevState = state;
	}
	//일단 CARD 결제 고정
	memset(pScenario->m_szpaymethod, 0x00, sizeof(pScenario->m_szpaymethod));
	memcpy(pScenario->m_szpaymethod, "CARD", sizeof(pScenario->m_szpaymethod) - 1);

	switch (state)
	{
	case 0:
	{
		char TempPath[1024 + 1] = { 0x00, };
		new_guide();
		pScenario->InputErrCnt = 0;
		info_printf(localCh, "ALLAT_CID_ScenarioStart [%d] 인사말...", state);
		eprintf("ALLAT_CID_ScenarioStart [%d] 인사말", state);

		//sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\%s", (*lpmt)->dnis);
		sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\wownet_intro", (*lpmt)->dnis);
		set_guide(VOC_WAVE_ID, TempPath);	 // "인사말"
		setPostfunc(POST_PLAY, ALLAT_CID_ScenarioStart, 1, 0);
		return send_guide(NODTMF);
	}

	case 1:
		new_guide();
		memset(pScenario->m_szInputTel, 0x00, sizeof(pScenario->m_szInputTel));
		memcpy(pScenario->m_szInputTel, (*lpmt)->ani, sizeof(pScenario->m_szInputTel) - 1);//Caller ID 주문 정보 조회

		setPostfunc(POST_NET, ALLAT_getOrderInfo, 0, 0);
		return getOrderInfo_host(90);

	case 0xffff:
		new_guide();
		return  goto_hookon();
	default:
		info_printf(localCh, "ALLAT_CID_ScenarioStart [%d]  C고객 회원 번호 8자리 번호 입력 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_CID_ScenarioStart[%d]  고객 회원 번호 8자리 번호 입력 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
	return 0;
}


int ALLAT_CIA_ScenarioStart(/* [in] */int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_Hangung_Quick_Scenario *pScenario = (CALLAT_Hangung_Quick_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_CIA_ScenarioStart;
		(*lpmt)->prevState = state;
	}
	//일단 CARD 결제 고정
	memset(pScenario->m_szpaymethod, 0x00, sizeof(pScenario->m_szpaymethod));
	memcpy(pScenario->m_szpaymethod, "CARD", sizeof(pScenario->m_szpaymethod) - 1);

	switch (state)
	{
	case 0:
	{
		pScenario->m_nRetryCnt = 0;

		char TempPath[1024 + 1] = { 0x00, };
		new_guide();
		pScenario->InputErrCnt = 0;
		info_printf(localCh, "ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 인사말...", state);
		eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 인사말", state);

		//sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\%s", (*lpmt)->dnis);
		sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\wownet_intro", (*lpmt)->dnis);
		set_guide(VOC_WAVE_ID, TempPath);	 // "인사말"
		setPostfunc(POST_PLAY, ALLAT_CIA_ScenarioStart, 1, 0);
		return send_guide(NODTMF);
	}

	case 1:
		new_guide();
		memset(pScenario->m_szInputTel, 0x00, sizeof(pScenario->m_szInputTel));
		memcpy(pScenario->m_szInputTel, (*lpmt)->ani, sizeof(pScenario->m_szInputTel) - 1);//Caller ID 주문 정보 조회

		if (strncmp(pScenario->m_szInputTel, "010", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "011", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "012", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "016", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "017", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "018", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "019", 3) != 0)
		{
			return ALLAT_CIA_ScenarioStart(11);
		}
		setPostfunc(POST_NET, ALLAT_getOrderInfo, 0, 0);
		return getOrderInfo_host_wrapper(90);



	case 11:
		new_guide();
		info_printf(localCh, "ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부...", state);
		eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부", state);
		set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\input_telnum_start");	 // "전화 번호 입력"
		setPostfunc(POST_DTMF, ALLAT_CIA_ScenarioStart, 12, 0);
		return send_guide(13);

	case 12:// 전화 번호 입력 처리
		if ((check_validform("*#:7:12", (*lpmt)->refinfo)) < 0)	// 눌린 dtmf값이 8~12의 길이이며 *또는#으로 끝이났다.
		{
			eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부>잘못 누르셨습니다.....", state);
			return send_error();
		}
		if (strncmp((*lpmt)->dtmfs, "010", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "011", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "012", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "016", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "017", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "018", 3) != 0 &&
			strncmp((*lpmt)->dtmfs, "019", 3) != 0)
		{
			eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부(형식 오류)>잘못 누르셨습니다.....", state);
			return send_error();
		}
		new_guide();
		memset(pScenario->m_szInputTel, 0x00, sizeof(pScenario->m_szInputTel));
		memcpy(pScenario->m_szInputTel, (*lpmt)->refinfo, sizeof(pScenario->m_szInputTel) - 1);

		info_printf(localCh, "ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부>확인 부(TTS)", state);
		eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부>확인 부(TTS)", state);
		if (TTS_Play)
		{
			char TTSBuf[1024 + 1] = { 0x00, };
			int TTsLen = strlen((*lpmt)->refinfo);
			for (int nRep = 0, nRep2 = 0;; nRep++)
			{
				if (TTsLen < 1) break;
				TTSBuf[nRep2++] = (char)*((*lpmt)->refinfo + nRep);
				TTSBuf[nRep2++] = ',';
				TTsLen--;
			}


			setPostfunc(POST_NET, ALLAT_CIA_ScenarioStart, 13, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님께서 누르신 전화번호는, %s 번 입니다.", TTSBuf);
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	case 13:
		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile,sizeof(TTSFile), "%s", pScenario->szTTSFile);

			set_guide(VOC_TTS_ID, TTSFile);	 // 주문번호 확일을 위해 주문 번호 재생
			set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_CIA_ScenarioStart, 14, 0);
		return send_guide(1);
	case 14:
		if (check_validdtmf && !check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		if (c == '1') //예
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_CIAScenarioStart[%d] 고객 전화 번호 입력 부>확인 부> 확인 부>맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>맞습니다.", state);

			setPostfunc(POST_NET, ALLAT_getOrderInfo, 0, 0);
			return getOrderInfo_host_wrapper(90);
		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>아니오", state);
			eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>아니오", state);

			return ALLAT_CIA_ScenarioStart(11);
		}

	case 0xffff:
		new_guide();
		return  goto_hookon();
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_CIAScenarioStart [%d]  C고객 회원 번호 8자리 번호 입력 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_CIAScenarioStart[%d]  고객 회원 번호 8자리 번호 입력 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
	return 0;
}

CALLAT_Hangung_Quick_Scenario::CALLAT_Hangung_Quick_Scenario()
{
	m_AdoDb = NULL;
	m_Myport = NULL;
	memset(szDnis, 0x00, sizeof(szDnis));
	memset(szAni, 0x00, sizeof(szAni));
	memset(szArsType, 0x00, sizeof(szArsType));
	memset(szSessionKey, 0x00, sizeof(szSessionKey));
	memset(m_szInputTel, 0x00, sizeof(m_szInputTel));

	nChan = 0;

	m_nRetryMaxCnt = ::GetPrivateProfileInt("RETRY", "MAXCOUNT", 0, PARAINI);
	m_nRetryCnt = 0;
}

CALLAT_Hangung_Quick_Scenario::~CALLAT_Hangung_Quick_Scenario()
{
	if (m_hThread)
	{
		xprintf("[CH:%03d] Allat DB Access 동작 중.... ", nChan);
		::WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
		xprintf("[CH:%03d] Allat DB Access 중지.... ", nChan);
	}
	if (m_hPayThread)
	{
		xprintf("[CH:%03d] Allat PAYMENT 동작 중.... ", nChan);
		::WaitForSingleObject(m_hPayThread, INFINITE);
		CloseHandle(m_hPayThread);
		m_hPayThread = NULL;
		xprintf("[CH:%03d] Allat PAYMENT 중지.... ", nChan);

	}
	if (m_Myport) m_Myport->ppftbl[POST_NET].postcode = HI_OK;
}

int CALLAT_Hangung_Quick_Scenario::ScenarioInit(LPMTP *Port, char *ArsType)
{
	(*lpmt)->pScenario = (void *)this;
	nChan = Port->chanID;
	m_Myport = Port;
	//동기화 개체 
	strncpy_s(szDnis,sizeof(szDnis), m_Myport->dnis, sizeof(szDnis) - 1);
	strncpy_s(szAni,sizeof(szAni), m_Myport->ani, sizeof(szAni) - 1);
	strncpy_s(szArsType,sizeof(szArsType), ArsType, sizeof(szArsType) - 1);
	strncpy_s(szSessionKey,sizeof(szSessionKey), m_Myport->Session_Key, sizeof(szSessionKey) - 1);
	//IScenario_enter_handler();
	//curyport = Port;
	//IScenario_leave_handler();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// 모든 시나리의 시작 부
//
int CALLAT_Hangung_Quick_Scenario::jobArs(/* [in] */int state)
{
	// 이후 객체 함수 이용에 제약이 따르므로 
	// 자체 동기 모듈이 요구 된다.
	// 별도의 DLL이므로 
	//IScenario_enter_handler();
	//curyport = m_Myport;
	//curyport->pScenario = (void *)this;
	(*lpmt)->pScenario = (void *)this;
	(*lpmt)->Myexit_service = ALLAT_ExitSvc;
	(*lpmt)->MyStartState = 10;
	memset(&m_CardInfo, 0x00, sizeof(m_CardInfo));

	if (strcmp(szArsType, "ARS") == 0) return ALLAT_ArsScenarioStart(0);
	else if (strcmp(szArsType, "SMS") == 0) return ALLAT_SMSScenarioStart(0);
	else if (strcmp(szArsType, "CID") == 0) return  ALLAT_CID_ScenarioStart(0);
	else if (strcmp(szArsType, "CIA") == 0) return  ALLAT_CIA_ScenarioStart(0);

	return ALLAT_jobArs(0);
}


#include <Windows.h>                    /**< Using Windows Data Type       */
//#include <crtdbg.h>                     /**< Using _CrtReportMode() Option */
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void InvalidParameterHandler(const wchar_t *expression, const wchar_t * function, const wchar_t * file,
	unsigned int line, uintptr_t pReserved);

_invalid_parameter_handler newHandler;
_invalid_parameter_handler oldHandler;


extern "C" BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_MID", ALLAT_MID, (char *)g_strMid, sizeof(g_strMid), PARAINI);
		GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_LICENSEKEY", ALLAT_LICENSEKEY, (char *)g_strLicenekey, sizeof(g_strLicenekey), PARAINI);
		GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_CANCELPWD", ALLAT_CANCELPWD, (char *)g_strCancelPwd, sizeof(g_strCancelPwd), PARAINI);
		GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_DEBUG", ALLAT_DEBUG, (char *)gNiceDebug, sizeof(gNiceDebug), PARAINI);
		GetPrivateProfileString("ALLAT_PAYMEMT", "ALLAT_LOG", ALLAT_LOG, (char *)gNiceLog, sizeof(gNiceLog), PARAINI);

		errno_t result = 0;
		/**< CRT가 Assertion 다이얼 로그를 출력하지 못하게 함 */
		_CrtSetReportMode(_CRT_ASSERT, 0);


		/**< 사용자 정의 핸들러 등록 */
		newHandler = InvalidParameterHandler;
		oldHandler = _set_invalid_parameter_handler(newHandler);

		//_CrtSetBreakAlloc(9686);
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		// 2015.12.16
		// 올앳 페이먼트의 연계 모듈의 구저적 문제로 메모리 누수 현상 발견
		// 해결하기 위한 코드 개발
		// 최영락			
		SSL_init();
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		// 2015.12.16
		// 올앳 페이먼트의 연계 모듈의 구저적 문제로 메모리 누수 현상 발견
		// 해결하기 위한 코드 개발
		// 최영락
		SSL_free_comp_methods();
		break;
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////
////                                                                                      
//// @param     [in]    expression : CRT 함수 내에서 발생한 테스트 실패에 대한 설명 문자열   
//// [in]    function   : 실패를 발생시킨 CRT 함수 이름                                     
//// [in]    file       : 소스 파일 이름                                                   
//// [in]    line       : 에러가 발생한 소스 코드의 행 번호                                 
//// [out]   pReserved  : 예약어                                                          
//// @return    void                                                                      
//// @note      사용자 정의한 유효 파라미터 핸들러
//// @note      이 핸들러는 CRT가 유효 파라미터 검사를 수행할 때 기본 핸들러 대신 수행
/////////////////////////////////////////////////////////////////////////////////////////////
void InvalidParameterHandler(const wchar_t *expression, const wchar_t *function, const wchar_t * file,
	unsigned int line, uintptr_t pReserved)
{
	printf("Invalid parameter detected in function %s.\n", function);
	printf("File: %s Line: %d\n", file, line);
	printf("Expression: %s\n", expression);
	exit(EXIT_FAILURE);
}

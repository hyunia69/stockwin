/****************************************************************************
*																			*
*	주   S E R V I C E	J O B S 											*
*																			*
****************************************************************************/
/* Head files */
#include "stdafx.h"
#include "CommonDef.H"
#include "ALLATCommom.h"
#include "time.h"

#include "WowTvSocket.h"
#include "Scenaio.h"
#include "ADODB.h"
#include "ALLAT_Stockwin_Billkey_New_Scenario.h"
#include "KISA_SHA256.h"
#include "PayLetterAPI.h"
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
extern int  Allat_Get_FixKey_host(int holdm);
extern int  AllatPayFixKeyCancle_host(int holdm);
extern int  AllatFixPaymemt_host(int holdm);

extern int  setPayLog_host(int holdm);
extern int  upOrderPayState_host(int holdm);
extern int   getSMSOrderInfo_host(int holdm);
extern int bill_delTcp_host(int holdm);
extern int getTcpOrderInfo_host(int holdm);
extern int getOrderInfo_host_wrapper(int holdm);  // REST API wrapper (2026.02.01 추가)

int ALLAT_ArsScenarioStart(/* [in] */int state);
int ALLAT_SMSScenarioStart(/* [in] */int state);
int ALLAT_CID_ScenarioStart(/* [in] */int state);
int ALLAT_CIA_ScenarioStart(/* [in] */int state);
int ALLAT_CQS_ScenarioStart(/* [in] */int state);
int ALLAT_consent(int state);
int ALLAT_InstallmentCConfrim(int state);

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
		return new CALLAT_WOWTV_Billkey_Easy_Scenario(); // CALLAT_WOWTV_Billkey_Easy_Scenario 시나리오 엔진의 개체 생성
	}

	void DestroyEngine(IScenario* pComponent)
	{
		delete (CALLAT_WOWTV_Billkey_Easy_Scenario *)pComponent; // CALLAT_WOWTV_Billkey_Easy_Scenario 시나리오 엔진의 개체 해제
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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);
	CString szURL = pScenario->m_szSHOP_RET_URL;

	szURL.Replace("{replyCd}", pScenario->m_CardResInfo.m_szRESULTCODE); // 결과 코드 
	szURL.Replace("{replyMsg}", pScenario->m_CardResInfo.m_szRESULTMSG); // 결과 메시지
	//szURL.Replace("{orderNo}", pScenario->m_CardResInfo.m_szMoid);       // 주문번호
	szURL.Replace("{approvalAmt}", pScenario->m_CardResInfo.m_AMT);      // 거래 금액
	szURL.Replace("{payType}", pScenario->m_CardResInfo.m_PAYMETHOD);    // 지불수단(결제 수단)
	szURL.Replace("{approvalDate}", pScenario->m_CardResInfo.m_AUTHDATE);// 승인일시
	szURL.Replace("{seqNo}", pScenario->m_CardResInfo.m_Tid);            // 거래일련번호 
	szURL.Replace("{approvalNo}", pScenario->m_CardResInfo.m_AUTHCODE);  // 송인번호
	szURL.Replace("{cardId}", pScenario->m_CardResInfo.m_CARDCODE);      // 카드사 ID
	szURL.Replace("{cardName}", pScenario->m_CardResInfo.m_CARDNAME);    // 신용카드 사 명
	szURL.Replace("{sellMm}", pScenario->m_CardResInfo.m_szCardQuota);   // 할부개월
	szURL.Replace("{zerofeeYn}", pScenario->m_CardResInfo.m_sZerofeeYn); // 무이자 여부
	/*
	13	bill_key	varchar	256	O	간편결제 용 빌키
	14	ext_data	char	64	O	간편결제 용 생년월일

	*/
	szURL.Replace("{bill_key}", pScenario->m_CardResInfo.szBill_Key);
	szURL.Replace("{ext_data}", pScenario->m_szext_data);

// AHN 2022.07.13 wownet 정기결제 개발
#if 1 //정기결제 0원결제를 위한 테스트 0원결제인 경우 PG사에 승인요청을 안하므로 주문번호와 상점아이디를 받아오지 못한다. 그래서 강제로 넣어준다.   
	xprintf("AHN CreateAg::주문번호 (%s) ", pScenario->m_szMx_issue_no);
	xprintf("AHN CreateAg::상점아이디 (%s) ", pScenario->m_szMx_id);

	szURL.Replace("{shop_id}", pScenario->m_szMx_id);
	if (pScenario->m_nAmount < 1) {
		szURL.Replace("{orderNo}", pScenario->m_szMx_issue_no);       // 주문번호
		szURL.Replace("{approvalAmt}", "0");
}
	else{
		szURL.Replace("{orderNo}", pScenario->m_CardResInfo.m_szMoid);       // 주문번호
		szURL.Replace("{approvalAmt}", pScenario->m_CardResInfo.m_AMT);      // 거래 금액
	}
#else
	szURL.Replace("{orderNo}", pScenario->m_CardResInfo.m_szMoid);       // 주문번호
	szURL.Replace("{ShopId}", pScenario->m_szMx_id);
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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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
		if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0)
		{
			if (strlen(pScenario->m_szbill_key) < 1)
			{// 빌키 없으면 체번 동의로 이동
				return ALLAT_consent(0);
			}
		}
		memset(pScenario->m_CardInfo.Password, 0x00, sizeof(pScenario->m_CardInfo.Password));
		strncpy_s(pScenario->m_CardInfo.Password, sizeof(pScenario->m_CardInfo.Password),(*lpmt)->dtmfs, sizeof(pScenario->m_CardInfo.Password) - 1);
		info_printf(localCh, "ALLAT_CardPw [%d]  Card 번호 입력부>결제 연동 부", state);
		eprintf("ALLAT_CardPw[%d]  Card 번호 입력부>결제 연동 부", state);
		setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
		return AllatPaymemt_host(90);

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
// JuminNo : 주민 번호 또는 법인 번호 입력부
////////////////////////////////////////////////////////////////////////////////
int ALLAT_JuminNo(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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
		//CDAUTH_EXP_BILL 결제 시 : 별도 인증 키 필요(협의)
		if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0)
		{//이미 빌키 발행 내지 저장하고 있어야 한다.(재결제)
			if (strlen(pScenario->m_szbill_key) < 1 && pScenario->m_bPayFlag == FALSE)
			{// 빌키가 없다면, 오류이지만 일단 무조건 생년월일 입력부로
				new_guide();
				eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부(간편결제 비인증)>빌키 생성 전", state);
				set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......" InputBirth
				setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
				return send_guide(11);
			}
			//결제용 멘트
			if (strlen(pScenario->m_szbill_key) > 0)
			{
				eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부(간편결제 비인증) >빌키 생성 후", state);
				set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......"
				setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
				return send_guide(11);
			}
			eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부 > 결제 시(빌키 재 결제 시)", state);
			set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......"
			setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
			return send_guide(11);
		}
		//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(O)->생년월일 (요구 멘트)->할부 기간->비번->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
		if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0)
		{//이미 빌키 발행 내지 저장하고 있어야 한다.(재결제)
			if (strlen(pScenario->m_szbill_key) < 1 && pScenario->m_bPayFlag == FALSE)
			{// 빌키가 없다면, 오류이지만 일단 무조건 생년월일 입력부로
				new_guide();
				eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부(간편결제 구인증)>빌키 생성 전", state);
				set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......" InputBirth
				setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
				return send_guide(11);
			}
			//결제용 멘트
			if (strlen(pScenario->m_szbill_key) > 0)
			{
				eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부(간편결제 구인증) >빌키 생성 후", state);
				set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......"
				setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
				return send_guide(11);
			}
			//결제용 멘트
			set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......"
			setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
			return send_guide(11);
		}
		//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(O)->생년월일 (요구 멘트) ->할부 기간->체번동의->체번 연동)          결제 시 : 별도 인증 키 필요(협의)
		else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN_BILL) == 0)
		{//이미 빌키 발행 내지 저장하고 있어야 한다.(재결제)
			if (strlen(pScenario->m_szbill_key) < 1 && pScenario->m_bPayFlag == FALSE)
			{// 빌키가 없다면, 오류이지만 일단 무조건 생년월일 입력부로
				new_guide();
				eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부(간편결제 부분인증)>빌키 생성 전", state);
				set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......" InputBirth
				setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
				return send_guide(11);
			}
			//결제용 멘트
			if (strlen(pScenario->m_szbill_key) > 0)
			{
				eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] > 주민/법인 입력 부(간편결제 부분인증) >빌키 생성 후", state);
				set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......"
				setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
				return send_guide(11);
			}
			//결제용 멘트
			set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/confirm");	 // "생년 월일......"
			setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
			return send_guide(11);
		}
		else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP) == 0)
		{// 올 수 없다.
			info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부 (CDAUTH_EXP)", state);
			eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부 (CDAUTH_EXP)", state);
		}
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
		if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0 ||
			strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN_BILL) == 0 ||
			strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0)
		{//이미 빌키 발행 내지 저장하고 있어야 한다.(재결제)
			if (pScenario->m_bPayFlag == FALSE)
			{
				BYTE  szSecretNo[32 + 1] = { 0x00, };//해시 엔코딩
				char  szHash_SecretNo[64 + 1] = { 0x00, };//해시 코드
				KISA_SHA256_Encrpyt((const BYTE*)(*lpmt)->refinfo, strlen((*lpmt)->refinfo), (BYTE*)szSecretNo);//해시 암호화 후, 
				KISA_SHA256_HexCode((char*)szSecretNo, szHash_SecretNo);//핵스의 문자열로 치환한다.
				strncpy_s(pScenario->m_szext_data, sizeof(pScenario->m_szext_data), szHash_SecretNo, sizeof(pScenario->m_szext_data) - 1);
			}
		}

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
			sprintf_s(TTSFile, sizeof(TTSFile), "%s", pScenario->szTTSFile);
			eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] 주민 / 법인 입력부>확인 부>%s", state, TTSFile);
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
			strncpy_s(pScenario->m_CardInfo.SecretNo, sizeof(pScenario->m_CardInfo.SecretNo), (*lpmt)->refinfo, sizeof(pScenario->m_CardInfo.SecretNo) - 1);
			info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>맞습니다.", state);
			eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>맞습니다.", state);

			//CDAUTH_ALL       입력 순서: card num(O)->유효기간(O)->생년월일(O)->할부 기간(이동) -> 비번 -> 결제 연동 : 구인증
			//CDAUTH_SNN       입력 순서: card num(O)->유효기간(O)->생년월일(O)->할부 기간(이동) -> 결제 연동 : 부분인증
			if ((strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL) == 0) ||
				(strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN) == 0))
			{
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
			// 전제조건 : 빌키가 없을 경우
			//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(O)->할부 기간(이동)->비번->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
			//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(O)->할부 기간(이동)->체번동의->체번 연동)          결제 시 : 별도 인증 키 필요(협의)
			else if ((strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0) ||
				(strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN_BILL) == 0))
			{
				if (strlen(pScenario->m_szbill_key) < 1)
				{//빌키가 없는 경우,
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

				if (pScenario->m_bPayFlag == TRUE)
				{// 재결제의 경우 현제 입력 된 생년월일과 우닉스에서 전달 받은 생년월일과 비교
					BYTE  szSecretNo[32 + 1] = { 0x00, };//해시 엔코딩
					char  szHash_SecretNo[64 + 1] = { 0x00, };//해시 코드
					KISA_SHA256_Encrpyt((const BYTE*)pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo), (BYTE*)szSecretNo);//해시 암호화 후, 
					KISA_SHA256_HexCode((char*)szSecretNo, szHash_SecretNo);//핵스의 문자열로 치환한다.

					if (strcmp(szHash_SecretNo, pScenario->m_szext_data) != 0)
					{//오입력으로 처리한다.
						pScenario->m_nRetryCnt++;
						if (pScenario->m_nRetryCnt >= pScenario->m_nRetryMaxCnt)
						{//잘못누르셨습니다.
							info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d] 간편결제(구인증/부분인증)재 결제 부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
							eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] 간편결제(구인증/부분인증)재 결제 부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
							setPostfunc(POST_PLAY, ALLAT_ExitSvc, 10, 0);
							return send_guide(NODTMF);
						}
						set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/no_Birth");	 // "생년월일이 일치하지 않습니다. 다시한번 본인 확인을 위한 법정생년월일 6자리 입력 후 우물정자 를 눌러주시기 바랍니다."
						setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
						return send_guide(11);
					}
				}
				else if (strlen(pScenario->m_szext_data) > 0)
				{
					BYTE  szSecretNo[32 + 1] = { 0x00, };//해시 엔코딩
					char  szHash_SecretNo[64 + 1] = { 0x00, };//해시 코드
					KISA_SHA256_Encrpyt((const BYTE*)pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo), (BYTE*)szSecretNo);//해시 암호화 후, 
					KISA_SHA256_HexCode((char*)szSecretNo, szHash_SecretNo);//핵스의 문자열로 치환한다.

					if (strcmp(szHash_SecretNo, pScenario->m_szext_data) != 0)
					{//오입력으로 처리한다.
						pScenario->m_nRetryCnt++;
						if (pScenario->m_nRetryCnt >= pScenario->m_nRetryMaxCnt)
						{//잘못누르셨습니다.
							info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d] 간편결제(구인증/부분인증) 최초 결제부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
							eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] 간편결제(구인증/부분인증) 최초 결제부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
							setPostfunc(POST_PLAY, ALLAT_ExitSvc, 10, 0);
							return send_guide(NODTMF);
						}
						set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/no_Birth");	 // "생년월일이 일치하지 않습니다. 다시한번 본인 확인을 위한 법정생년월일 6자리 입력 후 우물정자 를 눌러주시기 바랍니다."
						setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
						return send_guide(11);
					}
				}

				//빌키 있는 경우 바로 결제
				setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
				return AllatFixPaymemt_host(90);
			}
			//CDAUTH_EXP     결제 시 (X):  입력 순서: card num(O)->유효기간(O)->할부 기간(이동)->결제 연동 : 비인증
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP) == 0)
			{// 올 수 없다.
				info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>맞습니다(CDAUTH_EXP)", state);
				eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>맞습니다(CDAUTH_EXP)", state);
				setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
				return AllatPaymemt_host(90);
			}
			//CDAUTH_EXP_BILL 결제 시 (O: 빌키가 존재):                    결제 시 : 별도 인증 키 필요(협의)
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0)
			{
				info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>맞습니다(CDAUTH_EXP_BILL)", state);
				eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>맞습니다(CDAUTH_EXP_BILL)", state);
				if (strlen(pScenario->m_szbill_key) < 1)
				{//빌키가 없는 경우,
#if 0 //AHN 20180805 아래 태그와 동일하게 이동하면 된다. 할부개월수가 있다고 해서 다시 안내를 할 필요가 없다. 
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
#endif
#if 1 //AHN 20180730 3 주민번호를 입력받고 나서 주민번호까지 포함해서 빌키를 채번한 후 PayLetter_consent로 이동한다. 2로 이동하는 것은 동의멘트 이후 처리를 위해서이다. 
						setPostfunc(POST_NET, ALLAT_consent, 2, 0);
						return Allat_Get_FixKey_host(90);
#else
					return ALLAT_InstallmentCConfrim(0);
#endif
				}
				//빌키가 있는 경우
				if (pScenario->m_bPayFlag == TRUE)
				{// 재결제의 경우 현제 입력 된 생년월일과 우닉스에서 전달 받은 생년월일과 비교
					BYTE  szSecretNo[32 + 1] = { 0x00, };//해시 엔코딩
					char  szHash_SecretNo[64 + 1] = { 0x00, };//해시 코드
					KISA_SHA256_Encrpyt((const BYTE*)pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo), (BYTE*)szSecretNo);//해시 암호화 후, 
					KISA_SHA256_HexCode((char*)szSecretNo, szHash_SecretNo);//핵스의 문자열로 치환한다.

					if (strcmp(szHash_SecretNo, pScenario->m_szext_data) != 0)
					{//오입력으로 처리한다.
						pScenario->m_nRetryCnt++;
						if (pScenario->m_nRetryCnt >= pScenario->m_nRetryMaxCnt)
						{//잘못누르셨습니다.
							info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d] 간편 결제 비인증 재 결제 부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
							eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] 간편 결제 비인증 재 결제 부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
							setPostfunc(POST_PLAY, ALLAT_InstallmentCConfrim, 10, 0);
							return send_guide(NODTMF);
						}
						set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/no_Birth");	 // "생년월일이 일치하지 않습니다. 다시한번 본인 확인을 위한 법정생년월일 6자리 입력 후 우물정자 를 눌러주시기 바랍니다."
						setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
						return send_guide(11);
					}
				}
				else if (strlen(pScenario->m_szext_data) > 0)
				{
					BYTE  szSecretNo[32 + 1] = { 0x00, };//해시 엔코딩
					char  szHash_SecretNo[64 + 1] = { 0x00, };//해시 코드
					KISA_SHA256_Encrpyt((const BYTE*)pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo), (BYTE*)szSecretNo);//해시 암호화 후, 
					KISA_SHA256_HexCode((char*)szSecretNo, szHash_SecretNo);//핵스의 문자열로 치환한다.

					if (strcmp(szHash_SecretNo, pScenario->m_szext_data) != 0)
					{//오입력으로 처리한다.
						pScenario->m_nRetryCnt++;
						if (pScenario->m_nRetryCnt >= pScenario->m_nRetryMaxCnt)
						{//잘못누르셨습니다.
							info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo [%d] 간편 결제 비인증  최초 결제부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
							eprintf("ALLAT_WOWTV_Quick_JuminNo [%d] 간편 결제 비인증 최초 결제부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
							setPostfunc(POST_PLAY, ALLAT_ExitSvc, 10, 0);
							return send_guide(NODTMF);
						}
						set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/no_Birth");	 // "생년월일이 일치하지 않습니다. 다시한번 본인 확인을 위한 법정생년월일 6자리 입력 후 우물정자 를 눌러주시기 바랍니다."
						setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
						return send_guide(11);
					}
				}
				//빌키가 있는 경우
				setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
				return AllatFixPaymemt_host(90);
			}

		}
		else if (c == '2')//아니오
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>아니오", state);
			eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>아니오", state);

			//CDAUTH_EXP       입력 순서: card num(O)->유효기간(O)->할부 기간->결제 연동 : 비인증
			//CDAUTH_EXP_BILL (체번 시  : card num(O)->유효기간(O)->할부 기간->체번동의->체번 연동)                   결제 시 : 별도 인증 키 필요(협의)
			if (strcmp(pScenario->m_szbill_key, CDAUTH_EXP) == 0)
			{// 올 수 없다.
				info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>아니오(CDAUTH_EXP)", state);
				eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>아니오(CDAUTH_EXP)", state);
				return ALLAT_ExitSvc(0);
			}
			else if (strcmp(pScenario->m_szbill_key, CDAUTH_EXP_BILL) == 0)
			{
				info_printf(localCh, "ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>아니오(CDAUTH_EXP_BILL)", state);
				eprintf("ALLAT_WOWTV_Quick_JuminNo[%d] > 주민 / 법인 입력 부> 확인 부>아니오(CDAUTH_EXP_BILL)", state);
#if 1 //AHN 20180802 올앳의 경우 비인증도 빌키 채번에 주민번호가 필요하므로 여기로 들어올 수 있다. 아니오 라고 하면 다시 주민번호 물어보도록 이동한다.
				return ALLAT_JuminNo(0);
#else
				return ALLAT_ExitSvc(0);
#endif
			}
			//CDAUTH_ALL       입력 순서: card num(O)->유효기간(O)->생년월일(이동)->할부 기간 -> 비번 -> 결제 연동 : 구인증
			//CDAUTH_SNN       입력 순서: card num(O)->유효기간(O)->생년월일(이동)->할부 기간 -> 결제 연동 : 부분인증
			//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(이동)->할부 기간->비번->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
			//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(이동)->할부 기간->체번동의->체번 연동)          결제 시 : 별도 인증 키 필요(협의)

			return ALLAT_JuminNo(0);
		}
#if 1 //AHN 20180801 5 올앳의 경우를 위해서 새로 만들었다. 빌키를 생성한 후 생년월일을 비교하고 승인요청을 하기 위해서이다. 
	case 4:
		//빌키가 있는 경우
		eprintf("ALLAT_JuminNo[%d] > AHN CASE 4", state);
		if (pScenario->m_bPayFlag == TRUE)
		{// 재결제의 경우 현제 입력 된 생년월일과 우닉스에서 전달 받은 생년월일과 비교
			eprintf("ALLAT_JuminNo[%d] > AHN  CASE4  재결제", state);
			BYTE  szSecretNo[32 + 1] = { 0x00, };//해시 엔코딩
			char  szHash_SecretNo[64 + 1] = { 0x00, };//해시 코드
			KISA_SHA256_Encrpyt((const BYTE*)pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo), (BYTE*)szSecretNo);//해시 암호화 후, 
			KISA_SHA256_HexCode((char*)szSecretNo, szHash_SecretNo);//핵스의 문자열로 치환한다.

			if (strcmp(szHash_SecretNo, pScenario->m_szext_data) != 0)
			{//오입력으로 처리한다.
				eprintf("ALLAT_JuminNo[%d] > AHN  CASE4  재결제 오입력", state);
				pScenario->m_nRetryCnt++;
				if (pScenario->m_nRetryCnt >= pScenario->m_nRetryMaxCnt)
				{//잘못누르셨습니다.
					info_printf(localCh, "ALLAT_JuminNo [%d] 간편 결제 비인증 재 결제 부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
					eprintf("ALLAT_JuminNo [%d] 간편 결제 비인증 재 결제 부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
					setPostfunc(POST_PLAY, ALLAT_ExitSvc, 10, 0);
					return send_guide(NODTMF);
				}
				set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/no_Birth");	 // "생년월일이 일치하지 않습니다. 다시한번 본인 확인을 위한 법정생년월일 6자리 입력 후 우물정자 를 눌러주시기 바랍니다."
				setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
				return send_guide(11);
			}
		}
		else if (strlen(pScenario->m_szext_data) > 0)
		{
			eprintf("ALLAT_JuminNo[%d] > AHN  CASE4  최초결제", state);

			BYTE  szSecretNo[32 + 1] = { 0x00, };//해시 엔코딩
			char  szHash_SecretNo[64 + 1] = { 0x00, };//해시 코드
			KISA_SHA256_Encrpyt((const BYTE*)pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo), (BYTE*)szSecretNo);//해시 암호화 후, 
			KISA_SHA256_HexCode((char*)szSecretNo, szHash_SecretNo);//핵스의 문자열로 치환한다.

			if (strcmp(szHash_SecretNo, pScenario->m_szext_data) != 0)
			{//오입력으로 처리한다.
				eprintf("ALLAT_JuminNo[%d] > AHN  CASE4  최초결제 오입력", state);
				pScenario->m_nRetryCnt++;
				if (pScenario->m_nRetryCnt >= pScenario->m_nRetryMaxCnt)
				{//잘못누르셨습니다.
					info_printf(localCh, "ALLAT_JuminNo [%d] 간편 결제 비인증  최초 결제부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
					eprintf("ALLAT_JuminNo [%d] 간편 결제 비인증 최초 결제부...(초과:%d중%d 입력)", state, pScenario->m_nRetryMaxCnt, pScenario->m_nRetryCnt);
					setPostfunc(POST_PLAY, ALLAT_ExitSvc, 10, 0);
					return send_guide(NODTMF);
				}
				set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/no_Birth");	 // "생년월일이 일치하지 않습니다. 다시한번 본인 확인을 위한 법정생년월일 6자리 입력 후 우물정자 를 눌러주시기 바랍니다."
				setPostfunc(POST_DTMF, ALLAT_JuminNo, 1, 0);
				return send_guide(11);
			}
		}
		//빌키가 있는 경우
// AHN 2022.07.13 wownet 정기결제 개발
#if 1 //정기결제 0원결제를 위한 테스트  빌키는 이미 받았고 0원인 경우 승인요청을 하지 않고 결제 후 처리로 바로 이동한다. 
		if (pScenario->m_nAmount > 0){
			eprintf("ALLAT_JuminNo[%d] > AHN  CASE4 이동 ALLAT_payARS", state);
			setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
			eprintf("ALLAT_JuminNo[%d] > AHN  CASE4 리턴 AllatFixPaymemt_host ", state);
			return AllatFixPaymemt_host(90);
		}
		else{
			eprintf("ALLAT_JuminNo[%d] > AHN  CASE4 이동 ALLAT_payARS  0원결제 ", state);
			setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
			eprintf("ALLAT_JuminNo[%d] > AHN  CASE4 리턴 END", state);
			return send_guide(NODTMF);
		}
#else
		eprintf("ALLAT_JuminNo[%d] > AHN  CASE4 이동 ALLAT_payARS ", state);
		setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
		eprintf("ALLAT_JuminNo[%d] > AHN  CASE4 리턴 AllatFixPaymemt_host ", state);
		return AllatFixPaymemt_host(90);

#endif
#endif
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
// EffecDate : 간편결제 동의 입력부
////////////////////////////////////////////////////////////////////////////////
int ALLAT_consent(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_consent;
		(*lpmt)->prevState = state;
	}
// AHN 
	//setPostfunc(POST_NET, ALLAT_consent, 2, 0);
	//return ALLAT_JuminNo(0);

	switch (state)
	{
	case 0:
		if (new_guide)new_guide();

		pScenario->m_nRetryCnt = 1;//자신의 카운터 재외한 외부 카운터 초기화 한다...
		info_printf(localCh, "%s [%d] 간편결제 동의 부", __FUNCTION__, state);
		eprintf("%s [%d] 간편결제 동의  부", __FUNCTION__, state);

		eprintf("AHN > ALLAT_consent[%d] > ", state);

		set_guide(VOC_WAVE_ID, "ment/ALLAT_Hankok_RealTime/consent");	 // "...동의하시면 1번을, 동의 하지 않고 바로 일반 결제하시려면 2번"
		setPostfunc(POST_DTMF, ALLAT_consent, 1, 0);
		return send_guide(1);
	case 1:
		if (!check_validdtmf(c, "12"))	// 누른 DTMF값이 1,2만 유효하다.
		{
			eprintf("%s [%d] 간편결제 동의  부 > 잘못 누루셨습니다....", __FUNCTION__, state);
			return send_error();
		}
		new_guide();
		if (c == '1') //예
		{
			info_printf(localCh, "%s [%d] 간편결제 동의 부 > 동의 하셨습니다.", __FUNCTION__, state);
			eprintf("%s [%d] 간편결제 동의  부 > 동의 하셨습니다.", __FUNCTION__, state);
			//CDAUTH_EXP_BILL ==> 빌키체번 ==> 생년월일 입력 부 (안전성 확보 차원) ==> 결제
			//CDAUTH_ALL_BILL ==> 빌키체번 ==> 바로 결제 (카드 정보 모두 입력 후 체번 하였으므로 등록 직후 바로 결제)
			//CDAUTH_SNN_BILL  ==> 빌키체번 ==> 바로 결제( 비번 제외한 체번 방식 어느 정도 안정성 확보 하여 등록 직후 바로 결제)
			if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0)/* 간편 결제 */
			{
				if (strlen(pScenario->m_szbill_key) < 1)
				{
					//체번 성공 여부는 최종 시나리오에서 판단
					setPostfunc(POST_NET, ALLAT_consent, 2, 0);
#if 1 //AHN 20180730 2 올앳의 경우 비인증상점인 경우도 빌키채번에 생년월일이 필요해서 주민번호입력으로 이동한다.
					return ALLAT_JuminNo(0);
#else
					return Allat_Get_FixKey_host(90);
#endif
				}
			}
			else if ((strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0)/* 간편 결제 */ ||
				(strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN_BILL) == 0)/* 간편 결제 */)
			{
				if (strlen(pScenario->m_szbill_key) < 1)
				{
					//체번 성공 여부는 최종 시나리오에서 판단
					setPostfunc(POST_NET, ALLAT_consent, 2, 0);
					return Allat_Get_FixKey_host(90);
				}
			}
			// 이 외는 예외 처리 협의 부분으로 남겨두자!!!
			//return 
		}
		if (c == '2') //예
		{//일반 결제
			info_printf(localCh, "%s [%d] 간편결제 동의 부 > 동의 하지 않음 > 일반 결제", __FUNCTION__, state);
			eprintf("%s [%d] 간편결제 동의 부 > 동의 하지 않음 > 일반 결제", __FUNCTION__, state);
			strncpy_s(pScenario->m_szAUTH_TYPE, sizeof(pScenario->m_szAUTH_TYPE), pScenario->m_szAUTH_TYPE2, sizeof(pScenario->m_szAUTH_TYPE) - 1);
			strncpy_s(pScenario->m_szSHOP_RET_URL, sizeof(pScenario->m_szSHOP_RET_URL), pScenario->m_szSHOP_RET_URL2, sizeof(pScenario->m_szSHOP_RET_URL) - 1);
			strncpy_s(pScenario->m_szMx_id, sizeof(pScenario->m_szMx_id), pScenario->m_szMx_id2, sizeof(pScenario->m_szMx_id) - 1);
			strncpy_s(pScenario->m_szMx_opt, sizeof(pScenario->m_szMx_opt), pScenario->m_szMx_opt2, sizeof(pScenario->m_szMx_opt) - 1);
			memset(pScenario->m_szbill_key, 0x00, sizeof(pScenario->m_szbill_key));// 주문 내역으로 받은 빌키 삭제 처리

			if (strcmp(pScenario->m_szAUTH_TYPE2, CDAUTH_EXP) == 0)/* 비인증  결제 */
			{
				setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
				return AllatPaymemt_host(90);
			}
			if ((strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL) == 0) ||
				(strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN) == 0))
			{
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
// AHN 2022.07.13 wownet 정기결제 개발
#if 1 //정기결제  정기결제를 동의하지 않는 경우는 빌키채번이나 승인요청을 하지 않고 바로 종료한다. 
			else {
				eprintf("%s [%d] 간편결제 동의 부 > 동의 하지 않음 > 종료", __FUNCTION__, state);
				//ALLAT_ExitSvc(0);
				//set_guide(399);
				new_guide();
				set_guide(VOC_WAVE_ID, "ment/_common/common_audio/Error_end");// 결제가 완료 되었습니다.  //Error_end  pay_success_msg
				setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
				return send_guide(NODTMF);
			}
#else

#endif
		}
	case 2:
		// 최종 결제 확인을 위해...
		//체번 성공 여부
		if (pScenario->m_PaySysCd != 1)
		{// 연동을 아예 하지 못함
			info_printf(localCh, "%s [%d] 간편결제 동의 부", __FUNCTION__, state);
			eprintf("%s [%d] 간편결제 동의  부 > 잘못 누루셨습니다....", __FUNCTION__, state);
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->m_szbill_key) < 1)
		{// 체번에 실패 하였습니다.
			setPostfunc(POST_NET, ALLAT_consent, 3, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님 빌키 체번에 실패 하셨습니다.");
		}
		//체번에 성공 시
#if 1 //AHN 20180801 4 채번에 성공했다면 주민번호 case4로 이동해서 주민번호 검증을 한 후 승인요청을 한다. 주민번호로 가는 이유는 검증로직이 필요해서이다. 
		return ALLAT_JuminNo(4);
#else
		return ALLAT_JuminNo(0);
#endif
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
			sprintf_s(TTSFile, sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_PLAY, ALLAT_consent, 4, 0);
		return send_guide(NODTMF);
	case 4:
		return ALLAT_ExitSvc(0);
	default:
		info_printf(localCh, "%s [%d]  유효 기간 입력 부> 시나리오 아이디 오류", __FUNCTION__, state);
		eprintf("%s [%d]  유효 기간 입력 부>시나리오 아이디 오류", __FUNCTION__, state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
}


/////////////////////////////////////////////////////////////////////////////////
// ALLAT_InstallmentCConfrim : 할부 개월수 확인 및 입력 부
////////////////////////////////////////////////////////////////////////////////

int ALLAT_InstallmentCConfrim(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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
		pScenario->m_nRetryCnt = 1;//자신의 카운터 재외한 외부 카운터 초기화 한다...

		// 2016.05.24
		// 50000미만의 경우 바로 결제 연동 한다.
		if (pScenario->m_nAmount < (int)GetPrivateProfileInt("ALLAT_PAYMEMT", "ALLAT_MIN_AMT", 50000, PARAINI))
		{
			memset(pScenario->m_CardInfo.InstPeriod, 0x00, sizeof(pScenario->m_CardInfo.InstPeriod));
			sprintf_s(pScenario->m_CardInfo.InstPeriod,sizeof(pScenario->m_CardInfo.InstPeriod), "%02d", 0);
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d]  Card 번호 입력부>결제 연동 부", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d]  Card 번호 입력부>결제 연동 부", state);
			
			//CDAUTH_EXP_BILL (체번 시  : card num(O)->유효기간(O)->할부 기간(O)->체번동의->체번 연동) 결제 시 : 별도 인증 키 필요(협의)
			if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0)
			{
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 비인증)", state);
#if 1 //AHN 20180730 1 올앳의 경우 비인증인 경우라도 빌키채번에 반드시 생년월일이 필요하다고 해서 수정함. 할부개월 > 채번동의 > 주민번호 > 채번요청으로 진행함. 
					//빌키가 없고, 생년월일 무 입력 이면 빌키 채번 이동
				if ((strlen(pScenario->m_szbill_key) < 1) &&
					(strlen(pScenario->m_CardInfo.SecretNo) < 6))
				{
					eprintf("ALLAT_InstallmentCConfrim [%d] > AHN 빌키가 없고 생년월일이 없음 > 빌키채번으로 이동 (간편결제 비인증)", state);
#if 1 // AHN 2022.08.15 정기결제 수정  간편결제 동의를 생략해야 함. 결제승인이 조금 지연되어서 막아놓음. 다시 풀 예정임. 
					setPostfunc(POST_NET, ALLAT_consent, 2, 0);
					return ALLAT_JuminNo(0);
#else
					return ALLAT_consent(0);
#endif
				}
				// 빌키가 없고, 생년월일 존재하면 빌키 채번으로 이동 (사실상 여기로 오지는 않는다)  
				else if (strlen(pScenario->m_szbill_key) < 1 &&
					(strlen(pScenario->m_CardInfo.SecretNo) == 6))
				{
					eprintf("ALLAT_InstallmentCConfrim [%d] > AHN 빌키가 없고 생년월일이 있음 > 빌키채번으로 이동 (간편결제 비인증)", state);
					return ALLAT_consent(0);
				}
				//빌키가 있으나, 생년월일 문제이면 주민번호 입력으로 이동 
				else if (strlen(pScenario->m_CardInfo.SecretNo) < 6)
				{
					eprintf("ALLAT_InstallmentCConfrim [%d] > AHN 빌키는 있으나 생년월일 없음 > 주민번호로 이동 (간편결제 비인증)", state);
					return ALLAT_JuminNo(0);
				}
#else
				if (strlen(pScenario->m_szbill_key) < 1)
				{//빌키가 없으면, 바로 체번 동의로 이동
					return ALLAT_consent(0);
				}
				//빌키 존재
				return ALLAT_JuminNo(0);
#endif
			}
			//CDAUTH_EXP       입력 순서: card num(O)->유효기간(O)->할부 기간(O)->결제 연동 : 비인증
			//CDAUTH_SNN       입력 순서: card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)-> 결제 연동 : 부분인증
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP) == 0)
			{
				info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_EXP > 오만원 미만이라  > 결제 연동  .", state);
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_EXP > 오만원 미만이라  > 결제 연동 ", state);
				//삼성 카드 등의 비인증 카드의 경우 바로 결제 연동
				setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
				return AllatPaymemt_host(90);
			}
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN) == 0)
			{ // 비인증 가맹점 또는 부분인증 가맹점의 경우 결제 연동
				if (strlen(pScenario->m_CardInfo.SecretNo) > 5)
				{
					info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_SNN > 오만원 미만이라  > 결제 연동  .", state);
					eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_SNN > 오만원 미만이라  > 결제 연동 ", state);
					//삼성 카드 등의 비인증 카드의 경우 바로 결제 연동
					setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
					return AllatPaymemt_host(90);
				}
				return ALLAT_JuminNo(0);
			}
			//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->비번(이동)->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0)// 간편결제
			{
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 구인증)", state);
				if ((strlen(pScenario->m_szbill_key) < 1) &&
					(strlen(pScenario->m_CardInfo.SecretNo) < 6))
				{//빌키가 없으면, 생년월일 무 입력 이면 생년월일 로 이동
					return ALLAT_JuminNo(0);
				}
				// 생년월일 존재 하나, 빌키가 없으면 
				else if (strlen(pScenario->m_szbill_key) < 1 &&
					(strlen(pScenario->m_CardInfo.SecretNo) == 6)) return ALLAT_CardPw(0);
				//빌키가 있으나, 생년월일 문제
				else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
			}
			//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->체번동의(이동)->체번 연동)          결제 시 : 별도 인증 키 필요(협의)
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN_BILL) == 0)
			{
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 부분인증)", state);
				if ((strlen(pScenario->m_szbill_key) < 1) &&
					(strlen(pScenario->m_CardInfo.SecretNo) > 5))
				{//빌키가 없으면, 체번동의(이동)
					return ALLAT_consent(0);
				}
				else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
			}
			if (strlen(pScenario->m_CardInfo.SecretNo) > 5)
			{
				//CDAUTH_ALL       입력 순서: card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->비번->결제 연동 : 구인증
				return ALLAT_CardPw(0);
			}
			else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
		}
		new_guide();
		set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_halbu_start");	 // "요청하실 할부 개월수를 ... 일시불은 0번...."
		set_guide(VOC_WAVE_ID, "ment/ALLAT_Hangung/input_halbu_start_ilsibul");	  // "삼성, 신한, 국민, 비씨, 하나, 현대, 하나카드는 최대 5개월까지 무이자 할부가 가능합니다."
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
			//CDAUTH_EXP_BILL (체번 시  : card num(O)->유효기간(O)->할부 기간(O)->체번동의->체번 연동) 결제 시 : 별도 인증 키 필요(협의)
			if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0)
			{
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 비인증)", state);
				if (strlen(pScenario->m_szbill_key) < 1)
				{//빌키가 없으면, 바로 체번 동의로 이동
					return ALLAT_consent(0);
				}
				//빌키 존재
				return ALLAT_JuminNo(0);
			}
			//CDAUTH_EXP       입력 순서: card num(O)->유효기간(O)->할부 기간(O)->결제 연동 : 비인증
			//CDAUTH_SNN       입력 순서: card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)-> 결제 연동 : 부분인증
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP) == 0)
			{
				info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_EXP > 오만원 미만이라  > 결제 연동  .", state);
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_EXP > 오만원 미만이라  > 결제 연동 ", state);
				//삼성 카드 등의 비인증 카드의 경우 바로 결제 연동
				setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
				return AllatPaymemt_host(90);
			}
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN) == 0)
			{ // 비인증 가맹점 또는 부분인증 가맹점의 경우 결제 연동
				if (strlen(pScenario->m_CardInfo.SecretNo) > 5)
				{
					info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_SNN > 오만원 미만이라  > 결제 연동  .", state);
					eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_SNN > 오만원 미만이라  > 결제 연동 ", state);
					//삼성 카드 등의 비인증 카드의 경우 바로 결제 연동
					setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
					return AllatPaymemt_host(90);
				}
				return ALLAT_JuminNo(0);
			}
			//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->비번(이동)->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0)// 간편결제
			{
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 구인증)", state);
				if ((strlen(pScenario->m_szbill_key) < 1) &&
					(strlen(pScenario->m_CardInfo.SecretNo) < 6))
				{//빌키가 없으면, 생년월일 무 입력 이면 생년월일 로 이동
					return ALLAT_JuminNo(0);
				}
				// 생년월일 존재 하나, 빌키가 없으면 
				else if (strlen(pScenario->m_szbill_key) < 1 &&
					(strlen(pScenario->m_CardInfo.SecretNo) == 6)) return ALLAT_CardPw(0);
				//빌키가 있으나, 생년월일 문제
				else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
			}
			//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->체번동의(이동)->체번 연동)          결제 시 : 별도 인증 키 필요(협의)
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN_BILL) == 0)
			{
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 부분인증)", state);
				if ((strlen(pScenario->m_szbill_key) < 1) &&
					(strlen(pScenario->m_CardInfo.SecretNo) > 5))
				{//빌키가 없으면, 체번동의(이동)
					return ALLAT_consent(0);
				}
				else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
			}
			if (strlen(pScenario->m_CardInfo.SecretNo) > 5)
			{
				//CDAUTH_ALL       입력 순서: card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->비번->결제 연동 : 구인증
				return ALLAT_CardPw(0);
			}
			else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
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
			set_guide(VOC_WAVE_ID, "ment/ALLAT_Hangung/input_halbu_start_ilsibul");	  // "삼성, 신한, 국민, 비씨, 하나, 현대, 하나카드는 최대 5개월까지 무이자 할부가 가능합니다."
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

			//CDAUTH_EXP_BILL (체번 시  : card num(O)->유효기간(O)->할부 기간(O)->체번동의->체번 연동) 결제 시 : 별도 인증 키 필요(협의)
			if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0)
			{
				eprintf("AHN > CDAUTH_EXP_BILL >  ");

				if (strlen(pScenario->m_szbill_key) < 1)
				{//빌키가 없으면, 바로 체번 동의로 이동
					eprintf("AHN > CDAUTH_EXP_BILL > ALLAT_consent(0) ");
					return ALLAT_consent(0);
				}
				//빌키 존재
				eprintf("AHN > CDAUTH_EXP_BILL > ALLAT_JuminNo(0) ");
				return ALLAT_JuminNo(0);
			}
			//CDAUTH_EXP       입력 순서: card num(O)->유효기간(O)->할부 기간(O)->결제 연동 : 비인증
			//CDAUTH_SNN       입력 순서: card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)-> 결제 연동 : 부분인증
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP) == 0)
			{
				info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_EXP > 오만원 미만이라  > 결제 연동  .", state);
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_EXP > 오만원 미만이라  > 결제 연동 ", state);
				//삼성 카드 등의 비인증 카드의 경우 바로 결제 연동
				setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
				return AllatPaymemt_host(90);
			}
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN) == 0)
			{ // 비인증 가맹점 또는 부분인증 가맹점의 경우 결제 연동
				if (strlen(pScenario->m_CardInfo.SecretNo) > 5)
				{
					info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_SNN > 오만원 미만이라  > 결제 연동  .", state);
					eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_SNN > 오만원 미만이라  > 결제 연동 ", state);
					//삼성 카드 등의 비인증 카드의 경우 바로 결제 연동
					setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
					return AllatPaymemt_host(90);
				}
				return ALLAT_JuminNo(0);
			}
			//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->비번(이동)->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0)// 간편결제
			{
				if ((strlen(pScenario->m_szbill_key) < 1) &&
					(strlen(pScenario->m_CardInfo.SecretNo) > 5))
				{//빌키가 없으면, 생년월일로 이동
					return ALLAT_CardPw(0);
				}
				else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
			}
			//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->체번동의(이동)->체번 연동)          결제 시 : 별도 인증 키 필요(협의)
			else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN_BILL) == 0)
			{
				if ((strlen(pScenario->m_szbill_key) < 1) &&
					(strlen(pScenario->m_CardInfo.SecretNo) > 5))
				{//빌키가 없으면, 체번동의(이동)
					return ALLAT_consent(0);
				}
				else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
			}
			if (strlen(pScenario->m_CardInfo.SecretNo) > 5)
			{
				//CDAUTH_ALL       입력 순서: card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->비번->결제 연동 : 구인증
				return ALLAT_CardPw(0);
			}
			else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
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
		//CDAUTH_EXP_BILL (체번 시  : card num(O)->유효기간(O)->할부 기간(O)->체번동의->체번 연동) 결제 시 : 별도 인증 키 필요(협의)
		if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0)
		{
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 비인증)", state);
			if (strlen(pScenario->m_szbill_key) < 1)
			{//빌키가 없으면, 바로 체번 동의로 이동
				return ALLAT_consent(0);
			}
			//빌키 존재
			return ALLAT_JuminNo(0);
		}
		//CDAUTH_EXP       입력 순서: card num(O)->유효기간(O)->할부 기간(O)->결제 연동 : 비인증
		//CDAUTH_SNN       입력 순서: card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)-> 결제 연동 : 부분인증
		else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP) == 0)
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_EXP > 오만원 미만이라  > 결제 연동  .", state);
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_EXP > 오만원 미만이라  > 결제 연동 ", state);
			//삼성 카드 등의 비인증 카드의 경우 바로 결제 연동
			setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
			return AllatPaymemt_host(90);
		}
		else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN) == 0)
		{ // 비인증 가맹점 또는 부분인증 가맹점의 경우 결제 연동
			if (strlen(pScenario->m_CardInfo.SecretNo) > 5)
			{
				info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_SNN > 오만원 미만이라  > 결제 연동  .", state);
				eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d] > CDAUTH_SNN > 오만원 미만이라  > 결제 연동 ", state);
				//삼성 카드 등의 비인증 카드의 경우 바로 결제 연동
				setPostfunc(POST_NET, ALLAT_payARS, 0, 0);
				return AllatPaymemt_host(90);
			}
			return ALLAT_JuminNo(0);
		}
		//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->비번(이동)->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
		else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_ALL_BILL) == 0)// 간편결제
		{
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 구인증)", state);
			if ((strlen(pScenario->m_szbill_key) < 1) &&
				(strlen(pScenario->m_CardInfo.SecretNo) < 6))
			{//빌키가 없으면, 생년월일 무 입력 이면 생년월일 로 이동
				return ALLAT_JuminNo(0);
			}
			// 생년월일 존재 하나, 빌키가 없으면 
			else if (strlen(pScenario->m_szbill_key) < 1 &&
				(strlen(pScenario->m_CardInfo.SecretNo) == 6)) return ALLAT_CardPw(0);
			//빌키가 있으나, 생년월일 문제
			else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
		}
		//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->체번동의(이동)->체번 연동)          결제 시 : 별도 인증 키 필요(협의)
		else if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_SNN_BILL) == 0)
		{
			eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] > 할부 개월 수  입력 부(간편결제 부분인증)", state);
			if ((strlen(pScenario->m_szbill_key) < 1) &&
				(strlen(pScenario->m_CardInfo.SecretNo) > 5))
			{//빌키가 없으면, 체번동의(이동)
				return ALLAT_consent(0);
			}
			else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
		}
		if (strlen(pScenario->m_CardInfo.SecretNo) > 5)
		{
			//CDAUTH_ALL       입력 순서: card num(O)->유효기간(O)->생년월일(전제)->할부 기간(O)->비번->결제 연동 : 구인증
			return ALLAT_CardPw(0);
		}
		else if (strlen(pScenario->m_CardInfo.SecretNo) < 6) return ALLAT_JuminNo(0);
	default:
		info_printf(localCh, "ALLAT_WOWTV_Quick_InstallmentCConfrim [%d] 할부 개월 수 확인 부> 시나리오 아이디 오류", state);
		eprintf("ALLAT_WOWTV_Quick_InstallmentCConfrim[%d]  할부 개월 수 확인 부>시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// EffecDate : 카드 유효 기간 입력부
////////////////////////////////////////////////////////////////////////////////
int ALLAT_EffecDate(int state)
{
	int		c = 0;
	//int(*PrevCall)(int);
	int     localCh = (*lpmt)->chanID;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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

			//CDAUTH_EXP       입력 순서: card num(O)->유효기간(O)->할부 기간->결제 연동 : 비인증
			//CDAUTH_EXP_BILL (체번 시  : card num(O)->유효기간(O)->할부 기간->체번동의->체번 연동)                   결제 시 : 별도 인증 키 필요(협의)
			//CDAUTH_EXP       입력 순서: card num(O)->유효기간(O)->할부 기간->결제 연동 : 비인증
			//CDAUTH_EXP_BILL (체번 시  : card num(O)->유효기간(O)->할부 기간->체번동의->체번 연동)                   결제 시 : 별도 인증 키 필요(협의)
			if (strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP) == 0 ||
				strcmp(pScenario->m_szAUTH_TYPE, CDAUTH_EXP_BILL) == 0)
			{
				return ALLAT_InstallmentCConfrim(0);// 할부 기간으로 간다.
			}
			//CDAUTH_ALL       입력 순서: card num(O)->유효기간(O)->생년월일(이동)->할부 기간 -> 비번 -> 결제 연동 : 구인증
			//CDAUTH_SNN       입력 순서: card num(O)->유효기간(O)->생년월일(이동)->할부 기간 -> 결제 연동 : 부분인증
			//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(이동)->할부 기간->비번->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
			//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(O)->생년월일(이동)->할부 기간->체번동의->체번 연동)          결제 시 : 별도 인증 키 필요(협의)
			// 1.1. 유효기간 입력 후, 02와 03 인 경우는 생년월일  ( 그외는 생년월일 입력 부로 이동 )
			return ALLAT_JuminNo(0); 
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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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

			//CDAUTH_EXP       입력 순서: card num(O)->유효기간(이동)->할부 기간->결제 연동 : 비인증
			//CDAUTH_ALL       입력 순서: card num(O)->유효기간(이동)->생년월일->할부 기간 -> 비번 -> 결제 연동 : 구인증
			//CDAUTH_SNN       입력 순서: card num(O)->유효기간(이동)->생년월일->할부 기간 -> 결제 연동 : 부분인증
			//CDAUTH_EXP_BILL (체번 시  : card num(O)->유효기간(이동)->할부 기간->체번동의->체번 연동)                   결제 시 : 별도 인증 키 필요(협의)
			//CDAUTH_ALL_BILL (체번 시  : card num(O)->유효기간(이동)->생년월일->할부 기간->비번->체번동의->체번 연동)  결제 시 : 별도 인증 키 필요(협의)
			//CDAUTH_SNN_BILL (체번 시  : card num(O)->유효기간(이동)->생년월일->할부 기간->체번동의->체번 연동)          결제 시 : 별도 인증 키 필요(협의)
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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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

		{
			int nSkipPartnerConsent = ::GetPrivateProfileInt("SCENARIO_OPTIONS", "SKIP_PARTNER_CONSENT", 0, PARAINI);
			if (nSkipPartnerConsent == 1)
			{
				info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 동의서 안내 부> SKIP_PARTNER_CONSENT=1, 동의 멘트 스킵", state);
				eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 동의서 안내 부> SKIP_PARTNER_CONSENT=1, 동의 멘트 스킵", state);
				return ALLAT_getOrderInfo(9);
			}
		}

		// 이미 동의서에 동의한 고객은 해당 멘트를 건너 뛴다.
		if (strcmp(pScenario->m_szrenew_flag, "Y") == 0)
		{
			return ALLAT_getOrderInfo(9);
		}

		if (TTS_Play)
		{
			setPostfunc(POST_NET, ALLAT_getOrderInfo, 1, 0);
			if (strcmp((LPCTSTR)pScenario->szDnis, "4542") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "6617") == 0 ){
				return TTS_Play((*lpmt)->chanID, 92, "와우 글로벌 파트너스를 통해 제공되는, 각종 정보제공 및 기타 부가서비스를 위하여, %s에게, 고객님의 아이디, 필명, 휴대폰전화번호, 가입기간을, 해당 서비스 종료일 까지, 열람및 이용할 수있도록 정보제공되는것에, 동의하셔야 합니다. 동의를 거부하실 권리가 있으며, 동의를 거부하실 경우 해당 서비스를 이용하실 수 없습니다. 동의하시겠습니까? 동의 하시려면 1번을, 동의 하지 않으시면 2번을 눌러주십시오!", pScenario->m_szpsrtner_nm);
			}
			else if (strcmp((LPCTSTR)pScenario->szDnis, "6625") == 0){
				return TTS_Play((*lpmt)->chanID, 92, "와우 아카데미를 통해 제공되는, 각종 정보제공 및 기타 부가서비스를 위하여, %s에게, 고객님의 아이디, 필명, 휴대폰전화번호, 가입기간을, 해당 서비스 종료일 까지, 열람및 이용할 수있도록 정보제공되는것에, 동의하셔야 합니다. 동의를 거부하실 권리가 있으며, 동의를 거부하실 경우 해당 서비스를 이용하실 수 없습니다. 동의하시겠습니까? 동의 하시려면 1번을, 동의 하지 않으시면 2번을 눌러주십시오!", pScenario->m_szpsrtner_nm);
			}
			else {
				return TTS_Play((*lpmt)->chanID, 92, "와우넷 파트너스를 통해 제공되는, 각종 정보제공 및 기타 부가서비스를 위하여, %s에게, 고객님의 아이디, 필명, 휴대폰전화번호, 가입기간을, 해당 서비스 종료일 까지, 열람및 이용할 수있도록 정보제공되는것에, 동의하셔야 합니다. 동의를 거부하실 권리가 있으며, 동의를 거부하실 경우 해당 서비스를 이용하실 수 없습니다. 동의하시겠습니까? 동의 하시려면 1번을, 동의 하지 않으시면 2번을 눌러주십시오!", pScenario->m_szpsrtner_nm);
			}
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
	case 3:
		info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 간편결제(%s) 메뉴 부", state,
			pScenario->m_szcardName);
		eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 간편결제(%s) 메뉴 부", state,
			pScenario->m_szcardName);

		if (pScenario->m_TTSAccess == -1)//201701.04
		{
			new_guide();
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 간편결제(%s) 메뉴 부 현재 통화량이 많아!지연상황이 발생..", state,
				pScenario->m_szcardName);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 간편결제(%s) 메뉴 부 현재 통화량이 많아!지연상황이 발생..", state,
				pScenario->m_szcardName);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");	 // "현재 통화량이 많아!지연상황이 발생..."
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile, sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);	 // "간편결제 메뉴"
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리
		}
		setPostfunc(POST_DTMF, ALLAT_getOrderInfo, 4, 0);
		return send_guide(1);
	case 4:
		if (check_validdtmf && !check_validdtmf(c, "123"))	// 누른 DTMF값이 1,2,3만 유효하다.
		{
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부> 간편결제(%s) 메뉴 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		if (c == '1') //예
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부> 간편결제(%s) 메뉴 부 (%c)", state,
				pScenario->m_szcardName, c);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부> 간편결제(%s) 메뉴 부 (%c)", state,
				pScenario->m_szcardName, c);

			if (strlen(pScenario->m_szbill_key) < 1)
			{// 간편결제자가 아닌 경우
				return ALLAT_CardInput(0);
			}
			//간편 결제자의 경우
			return ALLAT_InstallmentCConfrim(0);// 할부 개월 수 입력 부

		}
		else if (c == '2')//간편결제 갱신
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부>확인 부> 간편결제 갱신", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부>확인 부> 간편결제 갱신", state);
			
			setPostfunc(POST_NET, ALLAT_getOrderInfo, 40, 0);
			return AllatPayFixKeyCancle_host(90);

			
		}
		else if (c == '3')//간편결제 삭제
		{
			info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부>확인 부> 간편결제 해지", state);
			eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d] 고객 주문 정보 안내 부>확인 부> 간편결제 해지", state);
			setPostfunc(POST_NET, ALLAT_getOrderInfo, 50, 0);
			return AllatPayFixKeyCancle_host(90);
		}
	case 40:
		info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d]  고객 주문 정보 안내 부> 간편결제 갱신 > 간편결제 해지 후 처리", state);
		eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d]  고객 주문 정보 안내 부> 간편결제 해지 후 처리", state);
		new_guide();
		setPostfunc(POST_NET, ALLAT_getOrderInfo, 400, 0);
		return bill_delTcp_host(90);
	case 400:
		//무조건 카드번호 입력부
		//기존의 빌키 일단 삭제
		memset(pScenario->m_szbill_key, 0x00, sizeof(pScenario->m_szbill_key));
		return ALLAT_CardInput(0);
	case 50:
		setPostfunc(POST_NET, ALLAT_getOrderInfo, 5, 0);
		return bill_delTcp_host(90);	
	case 5:
		info_printf(localCh, "ALLAT_WOWTV_Quick_getOrderInfo [%d]  고객 주문 정보 안내 부> 간편결제 해지 후 안내", state);
		eprintf("ALLAT_WOWTV_Quick_getOrderInfo[%d]  고객 주문 정보 안내 부> 간편결제 해지 후 안내", state);
		new_guide();
		if (pScenario->m_PaySysCd == 1)
		{
			setPostfunc(POST_NET, ALLAT_getOrderInfo, 6, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님께서 요청하신 간편 결제 정보를 해제하였습니다.");
		}
		else
		{
			setPostfunc(POST_NET, ALLAT_getOrderInfo, 6, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님께서 요청하신 간편 결제 정보 해제 처리에 실패 하였습니다. 한국경제 TV에 연락 하시시 바랍니다.");
		}
	case 6:
		new_guide();
		if (strlen(pScenario->szTTSFile) > 0)
		{
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile, sizeof(TTSFile), "%s", pScenario->szTTSFile);
			set_guide(VOC_TTS_ID, TTSFile);
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));// 플래이 직후 삭제 처리(TTS)
		}
		setPostfunc(POST_PLAY, ALLAT_getOrderInfo, 7, 0);
		return send_guide(NODTMF);
	case 7:
		return ALLAT_ExitSvc(0);
	case 9:
// AHN 2022.0808 정기결제 수정
#if 1
		if (TTS_Play)
		{
			if (strcmp(pScenario->m_szsub_status, "0") == 0)
			{
				eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 정기결제상태 첫결제 > %s", state, pScenario->m_szsub_status);
				if (strcmp(pScenario->m_szsub_has_trial, "N") == 0) {
					eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 체험상품유무 없음 > %s", state, pScenario->m_szsub_has_trial);

					setPostfunc(POST_NET, ALLAT_getOrderInfo, 10, 0);
					//AHN 20240207 조성현과장의 요청으로 이번 건만 처리함 추후에는 다음 달 금액을 파라미터에 추가해서 개발 할 예정. 그 때는 이 부분 삭제해야 함. 
					if (strcmp((LPCTSTR)pScenario->szDnis, "5037") == 0){
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 만료 1일 전에는 %s 상품, 금액 5만5천원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는, 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nAmount,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nAmount);
					}
					else if (strcmp((LPCTSTR)pScenario->szDnis, "5013") == 0){
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 입니다. 결제하실 금액은 %d원 입니다. 체험 서비스 만료 1일 전, 교육특강 금액 44만원이 결제됩니다. 본 상품은 상품 특성 상, 결제 후에는 중도해지 및 환불이 불가합니다. 또한, 무료로 제공 받은 모든 서비스는 일시정지, 중도해지, 타인양도가 불가합니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nAmount,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nAmount);
					}
					else if (strcmp((LPCTSTR)pScenario->szDnis, "6625") == 0){
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 만료 1일 전에는 %s 상품, 금액 %d원이 정기 결제됩니다. 본 상품은 와우아카데미 교육 상품으로, 교육 시설 규정에 따라 환불 가능합니다. 유료 서비스 이용 기간별로 환불 금액이 상이하니, 자세한 환불 규정은 홈페이지 통합 고객센터에서 확인 바랍니다. 또한 무료로 제공되는 서비스 혜택은 중도 해지 및 일시 정지, 타인 양도가 불가합니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nAmount,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nAmount);
					}
					else{
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 만료 1일 전에는 %s 상품, 금액 %d원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는, 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nAmount,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nAmount);
					}
				}
				else if (strcmp(pScenario->m_szsub_has_trial, "Y") == 0){
					eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 체험상품유무 있음 > %s", state, pScenario->m_szsub_has_trial);

					setPostfunc(POST_NET, ALLAT_getOrderInfo, 10, 0);
					//AHN 20240207 조성현과장의 요청으로 이번 건만 처리함 추후에는 다음 달 금액을 파라미터에 추가해서 개발 할 예정. 그 때는 이 부분 삭제해야 함. 
					if (strcmp((LPCTSTR)pScenario->szDnis, "4447") == 0){
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 무료체험 기간동안, 언제든지 해지하셔도, 이용요금은 부과되지 않습니다. 무료 체험 만료일에, %s 상품이 결제되고, 매월 만료 1일 전에 금액 44만원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nsub_amount);
					}
					else if (strcmp((LPCTSTR)pScenario->szDnis, "5037") == 0){
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 무료체험 기간동안, 언제든지 해지하셔도, 이용요금은 부과되지 않습니다. 무료 체험 만료일에, %s 상품이 결제되고, 매월 만료 1일 전에 금액 5만5천원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nsub_amount);
					}
					else if (strcmp((LPCTSTR)pScenario->szDnis, "5013") == 0){
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 입니다. 무료체험 기간동안, 언제든지 해지하셔도, 이용요금은 부과되지 않습니다. 무료 체험 만료일에, %s 상품이 결제되고, 매월 만료 1일 전에 금액 44만원이 정기 결제됩니다. 본 상품은 상품 특성 상, 결제 후에는 중도해지 및 환불이 불가합니다. 또한, 무료로 제공 받은 모든 서비스는 일시정지, 중도해지, 타인양도가 불가합니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nsub_amount);
					}
					else if (strcmp((LPCTSTR)pScenario->szDnis, "5039") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4618") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "5018") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4577") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4639") == 0){
						eprintf("AHN AHN");
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 예약결제 서비스입니다. 사전 무료체험 기간 중에는, 해지하셔도, 이용요금은 부과되지 않습니다. 체험 만료일 하루전에, 금액 %d원이 결제되며, 유료서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nsub_amount);
					}
					else if (strcmp((LPCTSTR)pScenario->szDnis, "4479") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4649") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4542") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "5094") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4527") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4609") == 0){
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 예약결제 서비스입니다. 사전 무료체험 기간 중에는, 해지하셔도, 이용요금은 부과되지 않습니다. 체험 만료일 하루전에, 금액 %d원이 결제되며, 상품 특성 상, 결제 후에는 중도해지 및 환불이 불가합니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nsub_amount);
					}
					else{
						return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 무료체험 기간동안, 언제든지 해지하셔도, 이용요금은 부과되지 않습니다. 무료 체험 만료일에, %s 상품이 결제되고, 매월 만료 1일 전에 금액 %d원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
							pScenario->m_szCC_name,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_szCC_Prod_Desc,
							pScenario->m_nsub_amount);
					}

				}
				else{
					eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 체험상품유무 없음> %s", state, pScenario->m_szsub_has_trial);

				}
			}
			else if (strcmp(pScenario->m_szsub_status, "1") == 0) {
				eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 정기결제상태 이용중 > %s", state, pScenario->m_szsub_status);
				setPostfunc(POST_NET, ALLAT_getOrderInfo, 6, 0);
				if (strcmp((LPCTSTR)pScenario->szDnis, "5039") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4618") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4479") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "5018") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4649") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4542") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4577") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4639") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "5094") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4527") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4609") == 0){
					return TTS_Play((*lpmt)->chanID, 92, "고객님께서는, %s 결제 서비스에 이미 등록되어 있습니다. 결제 서비스 해지는, 한국경제TV, 마이페이지에서 직접 하시거나, 고객센터 02-6676-0000 으로 문의하여 주시기 바랍니다. 한국경제TV는, 더욱 편리한 서비스가 되도록 노력하겠습니다.",
						pScenario->m_szCC_Prod_Desc);
				}
				else {
					return TTS_Play((*lpmt)->chanID, 92, "고객님께서는, %s 정기결제 서비스에 이미 등록되어 있습니다. 정기결제 서비스 해지는, 한국경제TV, 마이페이지에서 직접 하시거나, 고객센터 02-6676-0000 으로 문의하여 주시기 바랍니다. 한국경제TV는, 더욱 편리한 서비스가 되도록 노력하겠습니다.",
						pScenario->m_szCC_Prod_Desc);
				}
			}
			else if (strcmp(pScenario->m_szsub_status, "2") == 0) {
				eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 정기결제상태 해지후 재결제 > %s", state, pScenario->m_szsub_status);

				//if (strcmp(pScenario->m_szexpire_date, "") == 0) {
				//초기에는 
				//정기결제 서비스의 만료일이 지난 경우, expire_date 가 null 값으로 오며 이 경우에는 재가입이 가능하고 
				//정기결제 서비스의 만료일이 아니 지나지 않은 경우는 expire_date가 유효값으로 오며 이 경우는 재가입이 불가했다. 
				//그런데 만료일이 남아 있는 경우에도 재가입을 가능하도록 요청을 받고 수정함. 
				//2023.03.09 
				if (1) {
					eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 만료일 없음 > %s", state, pScenario->m_szexpire_date);

					if (strcmp(pScenario->m_szsub_has_trial, "N") == 0) {
						eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 체험상품유무 없음 > %s", state, pScenario->m_szsub_has_trial);

						setPostfunc(POST_NET, ALLAT_getOrderInfo, 10, 0);
						//AHN 20240207 조성현과장의 요청으로 이번 건만 처리함 추후에는 다음 달 금액을 파라미터에 추가해서 개발 할 예정. 그 때는 이 부분 삭제해야 함. 
						if (strcmp((LPCTSTR)pScenario->szDnis, "4447") == 0){
							return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 서비스 만료 1일 전에는 %s 상품, 금액 44만원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는, 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
						else if (strcmp((LPCTSTR)pScenario->szDnis, "5037") == 0){
							return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 서비스 만료 1일 전에는 %s 상품, 금액 5만5천원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는, 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
						else if (strcmp((LPCTSTR)pScenario->szDnis, "5013") == 0){
							return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 입니다. 결제하실 금액은 %d원 입니다. 체험 서비스 만료 1일 전, 교육특강 금액 44만원이 결제됩니다. 본 상품은 상품 특성 상, 결제 후에는 중도해지 및 환불이 불가합니다. 또한, 무료로 제공 받은 모든 서비스는 일시정지, 중도해지, 타인양도가 불가합니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
						else{
							return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 서비스 만료 1일 전에는 %s 상품, 금액 %d원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는, 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}

					}
					else if (strcmp(pScenario->m_szsub_has_trial, "Y") == 0){
						eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 체험상품유무 있음 > %s", state, pScenario->m_szsub_has_trial);

						setPostfunc(POST_NET, ALLAT_getOrderInfo, 10, 0);
						//AHN 20240207 조성현과장의 요청으로 이번 건만 처리함 추후에는 다음 달 금액을 파라미터에 추가해서 개발 할 예정. 그 때는 이 부분 삭제해야 함. 
						if (strcmp((LPCTSTR)pScenario->szDnis, "4447") == 0){
							return TTS_Play((*lpmt)->chanID, 92, "고객님께서는 %s 정기결제 서비스에 이미 등록하신 후, 정기결제를 해지하였습니다. 무료체험 서비스는, 최초 1회만 제공되는 서비스입니다. %s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 서비스 만료 1일 전에는 %s 상품, 금액 44만원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는, 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다. 동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
						else if (strcmp((LPCTSTR)pScenario->szDnis, "5037") == 0){
							return TTS_Play((*lpmt)->chanID, 92, "고객님께서는 %s 정기결제 서비스에 이미 등록하신 후, 정기결제를 해지하였습니다. 무료체험 서비스는, 최초 1회만 제공되는 서비스입니다. %s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 서비스 만료 1일 전에는 %s 상품, 금액 5만5천원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는, 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다. 동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
						else if (strcmp((LPCTSTR)pScenario->szDnis, "5013") == 0){
							return TTS_Play((*lpmt)->chanID, 92, "고객님께서는 %s 정기결제 서비스에 이미 등록하신 후, 정기결제를 해지하였습니다. 무료체험 서비스는, 최초 1회만 제공되는 서비스입니다. %s 고객님께서 주문하신 상품은, %s 입니다. 결제하실 금액은 %d원 입니다. 체험 서비스 만료 1일 전, 교육특강 금액 44만원이 결제됩니다. 본 상품은 상품 특성 상, 결제 후에는 중도해지 및 환불이 불가합니다. 또한, 무료로 제공 받은 모든 서비스는 일시정지, 중도해지, 타인양도가 불가합니다.  또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다. 동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
						else if (strcmp((LPCTSTR)pScenario->szDnis, "5039") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4618") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "5018") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4577") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4639") == 0){
							return TTS_Play((*lpmt)->chanID, 92, "고객님께서는 %s 예약결제 서비스에 이미 등록하신 후, 해지하였습니다. 무료체험 서비스는, 최초 1회만 제공되는 서비스입니다. %s 고객님께서 주문하신 상품은, %s 예약결제 서비스입니다. 결제하실 금액은 %d원 입니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다. 동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
						else if (strcmp((LPCTSTR)pScenario->szDnis, "4479") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4649") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4542") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "5094") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4527") == 0 || strcmp((LPCTSTR)pScenario->szDnis, "4609") == 0){
							return TTS_Play((*lpmt)->chanID, 92, "고객님께서는 %s 예약결제 서비스에 이미 등록하신 후, 해지하였습니다. 무료체험 서비스는, 최초 1회만 제공되는 서비스입니다. %s 고객님께서 주문하신 상품은, %s 예약결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 상품 특성 상, 결제 후에는 중도해지 및 환불이 불가합니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다. 동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
						else{
							return TTS_Play((*lpmt)->chanID, 92, "고객님께서는 %s 정기결제 서비스에 이미 등록하신 후, 정기결제를 해지하였습니다. 무료체험 서비스는, 최초 1회만 제공되는 서비스입니다. %s 고객님께서 주문하신 상품은, %s 정기결제 서비스입니다. 결제하실 금액은 %d원 입니다. 결제 후에는 서비스에 가입되며, 매월 서비스 만료 1일 전에는 %s 상품, 금액 %d원이 정기 결제됩니다. 서비스 중도해지시, 해지일까지 일수 만큼의 이용요금과, 가입비의 10퍼센트에 해당되는, 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다. 동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_szCC_name,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount,
								pScenario->m_szCC_Prod_Desc,
								pScenario->m_nAmount);
						}
					}
					else{
						eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 체험상품유무 없음> %s", state, pScenario->m_szsub_has_trial);

					}
				}
				else {
					//정기결제 서비스의 만료일이 아니 지나지 않은 경우는 expire_date가 유효값으로 오며 이 경우는 재가입이 불가했던 부분이며 
					//위에서  if(1) 로 수정함으로 여기로는 들어오지 않도록 했다. 추후에 다시 사용할 경우, 풀어서 사용하면 됨. 
					eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 만료일 있음 > %s", state, pScenario->m_szexpire_date);

					int nexpiredYear = atoiN(&(pScenario->m_szexpire_date[0]), 4);
					int nexpiredMonth = atoiN(&(pScenario->m_szexpire_date[4]), 2);
					int nexpiredDay = atoiN(&(pScenario->m_szexpire_date[6]), 2);

					CTime curTime = CTime::GetCurrentTime();//GetTickCount
					int ncurrentYear = curTime.GetYear();
					int ncurrentMonth = curTime.GetMonth();
					int ncurrentDay = curTime.GetDay();
					
					time_t start, end;
					struct tm exp_stime;
					struct tm cur_stime;
					int tm_day, tm_hour, tm_min, tm_sec;
					double diff;

					exp_stime.tm_year = nexpiredYear - 1900; // 년도가 1900년부터 시작하기 때문
					exp_stime.tm_mon = nexpiredMonth - 1; //월이 0부터 시작하기 때문
					exp_stime.tm_mday = nexpiredDay;
					exp_stime.tm_hour = 0;
					exp_stime.tm_min = 0;
					exp_stime.tm_sec = 0;
					exp_stime.tm_isdst = 0; //썸머타임 사용안함

					cur_stime.tm_year = ncurrentYear - 1900; // 년도가 1900년부터 시작하기 때문
					cur_stime.tm_mon = ncurrentMonth - 1; //월이 0부터 시작하기 때문
					cur_stime.tm_mday = ncurrentDay;
					cur_stime.tm_hour = 0;
					cur_stime.tm_min = 0;
					cur_stime.tm_sec = 0;
					cur_stime.tm_isdst = 0; //썸머타임 사용안함

					start = mktime(&exp_stime);
					end = mktime(&cur_stime);

					diff = difftime(start, end);
					tm_day = diff / (60 * 60 * 24);
					tm_day = tm_day; //날짜를 시간을 기준으로 계산하기 때문에 같은날도 +1 이 나이괴 때문에 -1 을 해준다. 
					eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 만료일 있음 > 만료일 %d 년  %d 월  %d 일 ", state, nexpiredYear, nexpiredMonth, nexpiredDay);
					eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 만료일 있음 > 현재일 %d 년  %d 월  %d 일 ", state, ncurrentYear, ncurrentMonth, ncurrentDay);
					eprintf("ALLAT_WOWTV_Quick_getOrderInfo [%d] 고객 주문 정보 안내 부 > 만료일 있음 > 잔여일 : %d ", state, tm_day);

					setPostfunc(POST_NET, ALLAT_getOrderInfo, 6, 0);
					return TTS_Play((*lpmt)->chanID, 92, "고객님께서는 %s 정기결제 서비스에 이미 등록하신 후, 정기결제를 해지하였습니다. 고객님께서는 사용기간이 %d일이 남아있습니다. %d월 %d일까지, 남은 사용기간을 모두 사용하신후, 다시 전화를 걸어 정기결제 등록 서비스를 이용해주시기 바랍니다.",
						pScenario->m_szCC_Prod_Desc,
						tm_day,
						nexpiredMonth,
						nexpiredDay);
				}


			}
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}

#else
		if (TTS_Play)
		{
			setPostfunc(POST_NET, ALLAT_getOrderInfo, 10, 0);
			//return TTS_Play((*lpmt)->chanID, 92, "%s 고객님께서는 %s에서, 결제하실 금액은 %d원 입니다.",
			//2017.01.24
			//한국경제WOWTV 김상기 차창의 요청
			//전체를 TTS로 하며, 해지에 관련된 내용을 재생하도록 한다.
			return TTS_Play((*lpmt)->chanID, 92, "%s 고객님, %s에서 , 주문하신 %s의 결제하실 금액은 %d원 입니다. 서비스 중도해지시 해지일까지 일수 만큼의 이용요금과  가입비의 10퍼센트에 해당되는 해지수수료, 사은품 비용이 함께 차감됩니다. 또한, 한국경제티비와, 파트너는, 금융투자업자가 아닌, 유사투자자문업자로, 개별적인 투자 상담과 자금 운영이 불가하며, 원금 손실이 발생할 수 있고, 그 손실은 투자자에게 귀속됩니다.  동의 및 결제하시려면 1번을, 취소하시려면 2번을 눌러주세요.",
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
#endif
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
// AHN 2022.07.13 wownet 정기결제 개발
#if 1 //정기결제 간편결제에서 빌키를 사용해서 재승인 요청이 없고 무조건 카드번호를 입력받는다. 
			return ALLAT_CardInput(0);
#else
			if(strlen(pScenario->m_szbill_key) < 1)
			{// 간편결제자가 아닌 경우
				return ALLAT_CardInput(0);
			}
#endif
			//간편 결제자의 경우
			if (TTS_Play)
			{
				pScenario->m_bPayFlag = TRUE;// 재결제임
				setPostfunc(POST_NET, ALLAT_getOrderInfo, 3, 0);
				return TTS_Play((*lpmt)->chanID, 92,
					"간편결제 등록하신 %s 로 결제하시려면 1번, 다른 카드로 결제하시려면 2번, 간편결제를 해지하시려면 3번을 눌러주세요.",
					pScenario->m_szcardName);
			}
			else
			{
				set_guide(399);
				setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
				return send_guide(NODTMF);
			}
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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

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

int ALLAT_CQS_ScenarioStart(/* [in] */int state)
{
	int		c = 0;
	int     localCh = (*lpmt)->chanID;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario);

	if (*lpmt)
	{
		c = *((*lpmt)->dtmfs);
		(*lpmt)->PrevCall = ALLAT_CQS_ScenarioStart;
		(*lpmt)->prevState = state;
	}
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
		info_printf(localCh, "ALLAT_CQS_ScenarioStart [%d] 인사말...", state);
		eprintf("ALLAT_CQS_ScenarioStart [%d] 인사말", state);

		sprintf_s(TempPath, sizeof(TempPath), "audio\\shop_intro\\wownet_intro", (*lpmt)->dnis);
		set_guide(VOC_WAVE_ID, TempPath);
		setPostfunc(POST_PLAY, ALLAT_CQS_ScenarioStart, 1, 0);
		return send_guide(NODTMF);
	}

	case 1:
		new_guide();
		memset(pScenario->m_szInputTel, 0x00, sizeof(pScenario->m_szInputTel));
		memcpy(pScenario->m_szInputTel, (*lpmt)->ani, sizeof(pScenario->m_szInputTel) - 1);

		if (strncmp(pScenario->m_szInputTel, "010", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "011", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "012", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "016", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "017", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "018", 3) != 0 &&
			strncmp(pScenario->m_szInputTel, "019", 3) != 0)
		{
			return ALLAT_CQS_ScenarioStart(11);
		}
		setPostfunc(POST_NET, ALLAT_getOrderInfo, 0, 0);
		return getOrderInfo_host_wrapper(90);

	case 11:
		new_guide();
		info_printf(localCh, "ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부...", state);
		eprintf("ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부", state);
		set_guide(VOC_WAVE_ID, "ment\\_common\\common_audio\\input_telnum_start");
		setPostfunc(POST_DTMF, ALLAT_CQS_ScenarioStart, 12, 0);
		return send_guide(13);

	case 12:
		if ((check_validform("*#:7:12", (*lpmt)->refinfo)) < 0)
		{
			eprintf("ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부>잘못 누르셨습니다.....", state);
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
			eprintf("ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부(형식 오류)>잘못 누르셨습니다.....", state);
			return send_error();
		}
		new_guide();
		memset(pScenario->m_szInputTel, 0x00, sizeof(pScenario->m_szInputTel));
		memcpy(pScenario->m_szInputTel, (*lpmt)->refinfo, sizeof(pScenario->m_szInputTel) - 1);

		info_printf(localCh, "ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부>확인 부(TTS)", state);
		eprintf("ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부>확인 부(TTS)", state);
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

			setPostfunc(POST_NET, ALLAT_CQS_ScenarioStart, 13, 0);
			return TTS_Play((*lpmt)->chanID, 92, "고객님께서 누르신 전화번호는, %s 번 입니다.", TTSBuf);
		}
		else
		{
			set_guide(399);
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
	case 13:
		if (pScenario->m_TTSAccess == -1)
		{
			new_guide();
			info_printf(localCh, "ALLAT_CQS_ScenarioStart [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			eprintf("ALLAT_CQS_ScenarioStart [%d] 현재 통화량이 많아!지연상황이 발생..", state);
			set_guide(VOC_WAVE_ID, "ment\\TTS_TimeOut");
			setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
			return send_guide(NODTMF);
		}
		if (strlen(pScenario->szTTSFile) > 0)
		{
			new_guide();
			char TTSFile[2048 + 1] = { 0x00, };
			sprintf_s(TTSFile, sizeof(TTSFile), "%s", pScenario->szTTSFile);

			set_guide(VOC_TTS_ID, TTSFile);
			set_guide(VOC_WAVE_ID, "ment/_common/common_audio/input_confirm");
			memset(pScenario->szTTSFile, 0x00, sizeof(pScenario->szTTSFile));
		}
		setPostfunc(POST_DTMF, ALLAT_CQS_ScenarioStart, 14, 0);
		return send_guide(1);
	case 14:
		if (check_validdtmf && !check_validdtmf(c, "12"))
		{
			eprintf("ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>잘못 누르셨습니다....", state);
			return send_error();
		}
		new_guide();

		if (c == '1')
		{
			info_printf(localCh, "ALLAT_CQS_ScenarioStart[%d] 고객 전화 번호 입력 부>확인 부> 확인 부>맞습니다.", state);
			eprintf("ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>맞습니다.", state);

			setPostfunc(POST_NET, ALLAT_getOrderInfo, 0, 0);
			return getOrderInfo_host_wrapper(90);
		}
		else if (c == '2')
		{
			info_printf(localCh, "ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>아니오", state);
			eprintf("ALLAT_CQS_ScenarioStart [%d] 고객 전화 번호 입력 부>확인 부> 확인 부>아니오", state);

			return ALLAT_CQS_ScenarioStart(11);
		}

	case 0xffff:
		new_guide();
		return  goto_hookon();
	default:
		info_printf(localCh, "ALLAT_CQS_ScenarioStart [%d] 시나리오 아이디 오류", state);
		eprintf("ALLAT_CQS_ScenarioStart[%d] 시나리오 아이디 오류", state);
		set_guide(399);
		setPostfunc(POST_PLAY, ALLAT_ExitSvc, 0, 0);
		return send_guide(NODTMF);
	}
	return 0;
}

CALLAT_WOWTV_Billkey_Easy_Scenario::CALLAT_WOWTV_Billkey_Easy_Scenario()
{
	m_AdoDb = NULL;
	m_Myport = NULL;
	memset(szDnis, 0x00, sizeof(szDnis));
	memset(szAni, 0x00, sizeof(szAni));
	memset(szArsType, 0x00, sizeof(szArsType));
	memset(szSessionKey, 0x00, sizeof(szSessionKey));
	memset(m_szInputTel, 0x00, sizeof(m_szInputTel));
	m_bPayFlag = FALSE;
	nChan = 0;

	m_nRetryMaxCnt = ::GetPrivateProfileInt("RETRY", "MAXCOUNT", 0, PARAINI);
	m_nRetryCnt = 0;
}

CALLAT_WOWTV_Billkey_Easy_Scenario::~CALLAT_WOWTV_Billkey_Easy_Scenario()
{
	if (!m_bDisconnectProcessed)
	{
		if (xprintf) xprintf("[CH:%03d] ~Destructor > Calling DisConnectProcess", nChan);
		this->DisConnectProcess();
	}

	if (m_hThread)
	{
		xprintf("[CH:%03d] Allat DB Access 동작 중.... ", nChan);
		DWORD waitResult1 = ::WaitForSingleObject(m_hThread, 10000);
		if (waitResult1 == WAIT_TIMEOUT) {
			xprintf("[CH:%03d] ~Destructor > DB thread wait timeout (10s)", nChan);
		}
		CloseHandle(m_hThread);
		m_hThread = NULL;
		xprintf("[CH:%03d] Allat DB Access 중지.... ", nChan);
	}
	if (m_hPayThread)
	{
		xprintf("[CH:%03d] Allat PAYMENT 동작 중.... ", nChan);
		DWORD waitResult2 = ::WaitForSingleObject(m_hPayThread, 10000);
		if (waitResult2 == WAIT_TIMEOUT) {
			xprintf("[CH:%03d] ~Destructor > Payment thread wait timeout (10s)", nChan);
		}
		CloseHandle(m_hPayThread);
		m_hPayThread = NULL;
		xprintf("[CH:%03d] Allat PAYMENT 중지.... ", nChan);

	}
	if (m_Myport) m_Myport->ppftbl[POST_NET].postcode = HI_OK;
}

int CALLAT_WOWTV_Billkey_Easy_Scenario::DisConnectProcess()
{
	int localCh = nChan;

	if (m_bDisconnectProcessed)
	{
		if (xprintf) xprintf("[CH:%03d] DisConnectProcess > Already processed, skipping", localCh);
		return 0;
	}
	m_bDisconnectProcessed = TRUE;

	if (info_printf) info_printf(localCh, "DisConnectProcess START");
	if (xprintf) xprintf("[CH:%03d] DisConnectProcess START", localCh);

	if (m_hPayThread)
	{
		if (info_printf) info_printf(localCh, "DisConnectProcess > Waiting for payment thread completion (max 5000ms)");
		DWORD waitResult = ::WaitForSingleObject(m_hPayThread, 5000);
		if (waitResult == WAIT_TIMEOUT)
		{
			if (xprintf) xprintf("[CH:%03d] DisConnectProcess > Payment thread wait timeout", localCh);
		}
		else if (waitResult == WAIT_OBJECT_0)
		{
			if (info_printf) info_printf(localCh, "DisConnectProcess > Payment thread completed successfully");
		}
	}

	LONG needRollback = InterlockedCompareExchange(&m_bNeedRollback, 0, 0);
	LONG paymentApproved = InterlockedCompareExchange(&m_bPaymentApproved, 0, 0);
	if (needRollback && !paymentApproved)
	{
		if (info_printf) info_printf(localCh, "DisConnectProcess > Rollback condition met (m_bNeedRollback=TRUE, m_bPaymentApproved=FALSE)");
		if (xprintf) xprintf("[CH:%03d] DisConnectProcess > Rollback condition met", localCh);

		if (m_szMx_issue_no[0] != '\0' && m_szMemberId[0] != '\0')
		{
			if (info_printf) info_printf(localCh, "DisConnectProcess > Calling PL_ReserveRollback (OrderNo=%s, MemberId=%s)", 
				m_szMx_issue_no, m_szMemberId);
			if (xprintf) xprintf("[CH:%03d] DisConnectProcess > Executing rollback for OrderNo=%s", localCh, m_szMx_issue_no);

			int rollbackResult = PL_ReserveRollback(m_szMx_issue_no, m_szMemberId);
			
			if (rollbackResult == 0)
			{
				if (info_printf) info_printf(localCh, "DisConnectProcess > Rollback succeeded");
				if (xprintf) xprintf("[CH:%03d] DisConnectProcess > Rollback completed successfully", localCh);
			}
			else
			{
				if (xprintf) xprintf("[CH:%03d] DisConnectProcess > Rollback failed (result=%d)", localCh, rollbackResult);
			}

			InterlockedExchange(&m_bNeedRollback, FALSE);
			if (info_printf) info_printf(localCh, "DisConnectProcess > m_bNeedRollback reset to FALSE");
		}
		else
		{
			if (xprintf) xprintf("[CH:%03d] DisConnectProcess > Rollback skipped - invalid OrderNo or MemberId (OrderNo=%s, MemberId=%s)", 
				localCh, m_szMx_issue_no, m_szMemberId);
		}
	}
	else if (needRollback && paymentApproved)
	{
		if (info_printf) info_printf(localCh, "DisConnectProcess > Rollback skipped - payment was approved");
		if (xprintf) xprintf("[CH:%03d] DisConnectProcess > No rollback needed (payment approved)", localCh);
	}
	else if (!needRollback)
	{
		if (info_printf) info_printf(localCh, "DisConnectProcess > No rollback needed (cache/coupon not used)");
	}

	if (info_printf) info_printf(localCh, "DisConnectProcess END");

	return 0;
}

int CALLAT_WOWTV_Billkey_Easy_Scenario::ScenarioInit(LPMTP *Port, char *ArsType)
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
int CALLAT_WOWTV_Billkey_Easy_Scenario::jobArs(/* [in] */int state)
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
	else if (strcmp(szArsType, "CQS") == 0) return  ALLAT_CQS_ScenarioStart(0);

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

/******************************************************************************
* 파일명 : NiceAccess.cpp
* 작성일 : 2015.01.05
* 작성자 : 최영락
******************************************************************************/

#include	"stdafx.h"
#include    "CommonDef.H"
#include    "ALLATCommom.h"


#include    "WowTvSocket.h"
#include    "AllatUtil.h"
#include    "Scenaio.h"
#include    "ADODB.h"
#include    "ALLAT_Stockwin_Billkey_New_Scenario.h"

#include <string.h> // strcpy_s() strlen()

#include <string.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char g_strMid[50];
extern char g_strLicenekey[100];
extern char g_strCancelPwd[100];
extern char gNiceDebug[10 + 1];
extern char gNiceLog[50 + 1];

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

void PAYquithostio(char *p, int ch)
{
	xprintf("[CH:%03d] PAYquithostio===START", ch);
	if (!in_multifunc(ch))
	{
		Sleep(500);
	}
	(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
	if ((*port)[ch].used != L_IDLE && in_multifunc(ch))
		quitchan(ch);

	xprintf("[CH:%03d] %s", ch, p);
	xprintf("[CH:%03d] PAYquithostio _endthread", ch);
}

// 결제 취소 요청 쓰레드용 함수 
unsigned int __stdcall AllatArsPayCancleProcess(void *data)
{
	int			ch;
	int			paythreadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	paythreadID = pScenario->paythreadID;

	pScenario->m_PaySysCd = 0;
	if (paythreadID != pScenario->paythreadID) {
		pScenario->m_PaySysCd = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		PAYquithostio("AllatArsPayCancleProcess the line service is not valid any more.", ch);
		xprintf("[CH:%03d] AllatArsPayCancleProcess END", ch);
		_endthreadex((unsigned int)pScenario->m_hPayThread);
		return 0;
	}

	ALLAT_ENCDATA atEnc;
	char sMsg[8191 + 1];
	char at_data[8191 + 1];
	char at_cross_key[255 + 1];

	// 결과 값 정의 
	//------------------------------------------------------------------------    
	char sReplyCd[(4 * 4) + 1];
	char sReplyMsg[(400 * 4) + 1];
	char sCancelYMDHMS[(14 * 4) + 1];
	char sPartCancelFlag[(1 * 4) + 1];
	char sRemainAmt[(10 * 4) + 1];
	char sPayType[(10 * 4) + 1];

	// 취소 요청 정보
	//------------------------------------------------------------------------    
	char szShopId[20 + 1];
	char szAmt[10 + 1];
	char szOrderNo[80 + 1];
	char szPayType[6 + 1];
	char szSeqNo[10 + 1];

	// 거래일시 설정
	memset(&pScenario->m_Cancel_ResInfo, 0x00, sizeof(pScenario->m_CardResInfo));
	CTime curTime = CTime::GetCurrentTime();
	sprintf_s(pScenario->m_Cancel_ResInfo.MX_ISSUE_DATE,sizeof(pScenario->m_Cancel_ResInfo.MX_ISSUE_DATE), "%04d%02d%02d%02d%02d%02d",
		curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(),
		curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond());

	// 초기화
	//------------------------------------------------------------------------        
	memset(sMsg, 0, sizeof(sMsg));
	memset(at_data, 0, sizeof(at_data));
	memset(at_cross_key, 0, sizeof(at_cross_key));

	memset(sReplyCd, 0, sizeof(sReplyCd));
	memset(sReplyMsg, 0, sizeof(sReplyMsg));
	memset(sCancelYMDHMS, 0, sizeof(sCancelYMDHMS));
	memset(sPartCancelFlag, 0, sizeof(sPartCancelFlag));
	memset(sRemainAmt, 0, sizeof(sRemainAmt));
	memset(sPayType, 0, sizeof(sPayType));

	memset(szShopId, 0, sizeof(szShopId));
	memset(szAmt, 0, sizeof(szAmt));
	memset(szOrderNo, 0, sizeof(szOrderNo));
	memset(szPayType, 0, sizeof(szPayType));
	memset(szSeqNo, 0, sizeof(szSeqNo));

	// 정보 입력
	//------------------------------------------------------------------------        
	// 필수 항목 
	strncpy_s(at_cross_key,sizeof(at_cross_key), g_strLicenekey, sizeof(at_cross_key) - 1);  // CrossKey값(최대200자)
	if (strlen(pScenario->m_szMx_opt) > 0)                            // 상점 별 CrossKey값(최대200자)
	{
		memset(at_cross_key, 0, sizeof(at_cross_key));
		strncpy_s(at_cross_key,sizeof(at_cross_key), pScenario->m_szMx_opt, sizeof(at_cross_key) - 1);
	}

	// 상점 관련 정보
	escapeString(szShopId, g_strMid, strlen(g_strMid));          //상점ID(최대 20자)
	if (strlen(pScenario->m_Card_CancleInfo.m_szMid) > 0)
	{
		memset(szShopId, 0x00, sizeof(szShopId));
		escapeString(szShopId, pScenario->m_Card_CancleInfo.m_szMid, strlen(pScenario->m_Card_CancleInfo.m_szMid)); // 상점 아이디 
	}

	escapeString(szAmt, pScenario->m_Card_CancleInfo.m_AMT, strlen(pScenario->m_Card_CancleInfo.m_AMT));        // 취소 금액               (최대  10 자리)
	escapeString(szOrderNo, pScenario->m_Card_CancleInfo.m_szMoid, strlen(pScenario->m_Card_CancleInfo.m_szMoid));  // 주문번호                (최대  80 자리)
	escapeString(szPayType, "CARD", 4);								// 원거래건의 결제방식[카드:CARD,계좌이체:ABANK]
	escapeString(szSeqNo, pScenario->m_Card_CancleInfo.m_Tid, strlen(pScenario->m_Card_CancleInfo.m_Tid));      // 거래일련번호:옵션필드    (최대  10자리)

	initEnc(atEnc);
	setValue(atEnc, "allat_shop_id", szShopId);
	setValue(atEnc, "allat_order_no", szOrderNo);
	setValue(atEnc, "allat_amt", szAmt);
	setValue(atEnc, "allat_pay_type", szPayType);
	setValue(atEnc, "allat_test_yn", "N");     // 테스트 :Y, 서비스 :N
	setValue(atEnc, "allat_opt_pin", "NOUSE"); // 수정금지(올앳 참조 필드)
	setValue(atEnc, "allat_opt_mod", "APP");   // 수정금지(올앳 참조 필드)
	setValue(atEnc, "allat_seq_no", szSeqNo);  // 옵션 필드( 삭제 가능함 )

	sprintf_s(at_data,sizeof(at_data), "allat_shop_id=%s"
		"&allat_enc_data=%s"
		"&allat_amt=%s"
		"&allat_cross_key=%s", szShopId, atEnc, szAmt, at_cross_key);

	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario  AllatArsPayCancleProcess START ", ch);

	if (CancelReq(at_data, "SSL", sMsg) != 0){
		/// 에러처리
		pScenario->m_PaySysCd = 1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
		PAYquithostio("the line service is AllatArsPayCancleProcess...FAIL", ch);
		xprintf("[CH:%03d] AllatArsPayCancleProcess end", ch);
		_endthreadex((unsigned int)pScenario->m_hPayThread);
		return 0;
	}

	// 결과 값 가져오기
	//--------------------------------------------------------------------------
	getValue("reply_cd=", sMsg, sReplyCd, sizeof(sReplyCd)); Allat_trim(sReplyCd);
	getValue("reply_msg=", sMsg, sReplyMsg, sizeof(sReplyMsg)); Allat_trim(sReplyMsg);

	xprintf("[CH:%03d] 결과코드        : %s", ch, sReplyCd);
	xprintf("[CH:%03d] 결과메세지      : %s", ch, sReplyMsg);

	memcpy(pScenario->m_CardResInfo.m_szRESULTCODE, sReplyCd, sizeof(pScenario->m_CardResInfo.m_szRESULTCODE) - 1); // 결과 코드
	memcpy(pScenario->m_CardResInfo.m_szRESULTMSG, sReplyMsg, sizeof(pScenario->m_CardResInfo.m_szRESULTMSG) - 1);  // 결과 메시지

	if (strcmp(sReplyCd, "0000") == 0){
		getValue("cancel_ymdhms=", sMsg, sCancelYMDHMS, sizeof(sCancelYMDHMS)); Allat_trim(sCancelYMDHMS);
		getValue("part_cancel_flag=", sMsg, sPartCancelFlag, sizeof(sPartCancelFlag)); Allat_trim(sPartCancelFlag);
		getValue("remain_amt=", sMsg, sRemainAmt, sizeof(sRemainAmt)); Allat_trim(sRemainAmt);
		getValue("pay_type=", sMsg, sPayType, sizeof(sPayType)); Allat_trim(sPayType);

		memcpy(&pScenario->m_Cancel_ResInfo.m_AUTHDATE, sCancelYMDHMS, sizeof(pScenario->m_Cancel_ResInfo.m_AUTHDATE) - 1);                     // 취소일시
		memcpy(&pScenario->m_Cancel_ResInfo.m_PartialCancelCode, sPartCancelFlag, sizeof(pScenario->m_Cancel_ResInfo.m_PartialCancelCode) - 1); // 취소구분
		memcpy(&pScenario->m_Cancel_ResInfo.sRemainAmt, sRemainAmt, sizeof(pScenario->m_Cancel_ResInfo.sRemainAmt) - 1);                        // 취소 후 잔액
		memcpy(&pScenario->m_Cancel_ResInfo.m_PAYMETHOD, sPayType, sizeof(pScenario->m_Cancel_ResInfo.m_PAYMETHOD) - 1);                        // 거래방식구분

		xprintf("[CH:%03d] 취소일시        : %s", ch, sCancelYMDHMS);
		xprintf("[CH:%03d] 취소구분        : %s", ch, sPartCancelFlag);
		xprintf("[CH:%03d] 잔액            : %s", ch, sRemainAmt);
		xprintf("[CH:%03d] 거래방식구분    : %s", ch, sPayType);
	}
	else{
		// reply_cd 가 "0000" 아닐때는 에러 (자세한 내용은 매뉴얼참조)
		// reply_msg 는 실패에 대한 메세지        
		memcpy(pScenario->m_Cancel_ResInfo.m_szMoid, pScenario->m_szMx_issue_no, sizeof(pScenario->m_Cancel_ResInfo.m_szMoid) - 1);        // 주문번호
		char sTmp[20 + 1] = { 0x00, };
		memset(sTmp, 0x00, sizeof(sTmp));
		sprintf_s(sTmp,sizeof(sTmp), "%d", pScenario->m_nAmount);
		memcpy(pScenario->m_Cancel_ResInfo.m_AMT, sTmp, sizeof(pScenario->m_Cancel_ResInfo.m_AMT) - 1);      // 거래 금액
		memcpy(pScenario->m_Cancel_ResInfo.m_szCardQuota, pScenario->m_CardInfo.InstPeriod, sizeof(pScenario->m_Cancel_ResInfo.m_szCardQuota) - 1);  // 할부개월수
		memcpy(pScenario->m_Cancel_ResInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_Cancel_ResInfo.m_szMid) - 1);//상번ID


		xprintf("[CH:%03d] 주문번호        : %s", ch, pScenario->m_Cancel_ResInfo.m_szMoid);
		xprintf("[CH:%03d] 결제금액      : %s", ch, pScenario->m_Cancel_ResInfo.m_AMT);
		xprintf("[CH:%03d] 할부개월수      : %s", ch, pScenario->m_Cancel_ResInfo.m_szCardQuota);
		xprintf("[CH:%03d] 가맹점ID      : %s", ch, pScenario->m_Cancel_ResInfo.m_szMid);
		xprintf("[CH:%03d] 전송일시      :%s", ch, pScenario->m_Cancel_ResInfo.MX_ISSUE_DATE);
	}

	pScenario->m_PaySysCd = 1;
	(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
	PAYquithostio("the line service is AllatArsPayCancleProcess.", ch);
	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario  AllatArsPayCancleProcess END ", ch);
	_endthreadex((unsigned int)pScenario->m_hPayThread);
	return 0;

}

// 결제 요청 쓰레드용 함수 
unsigned int __stdcall AllatArsPayProcess(void *data)
{
	int			ch;
	int			paythreadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);
	
	ch = lineTablePtr->chanID;
	paythreadID = pScenario->paythreadID;

	xprintf("[CH:%03d] Allat ArsPayProcess START", ch);
	//리턴값
	//=====================
	//쿼리인자변수선언
	//=====================
	pScenario->m_PaySysCd = 0;
	if (paythreadID != pScenario->paythreadID) {
		pScenario->m_PaySysCd = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		PAYquithostio("Allat ArsPayProcess the line service is not valid any more.", ch);
		xprintf("[CH:%03d] Allat ArsPayProcess end", ch);
		_endthreadex((unsigned int)pScenario->m_hPayThread);
		return 0;
	}

	ALLAT_ENCDATA atEnc;
	char sMsg[8191 + 1];
	char at_data[8191 + 1];
	char at_cross_key[255 + 1];
	char sTmp[8191 + 1];

	// 결과 값 정의 
	//------------------------------------------------------------------------
	char sReplyCd[4 + 1];
	char sReplyMsg[400 + 1];
	char sPayType[10 + 1];
	char sApprovalNo[10 + 1];
	char sOrderNo[80 + 1];
	char sAmt[10 + 1];
	char sApprovalYMDHMS[14 + 1];
	char sSeqNo[10 + 1];
	char sCardId[2 + 1];
	char sCardNm[48 + 1];
	char sSellMm[2 + 1];
	char sZerofeeYn[1 + 1];
	char sCertYn[1 + 1];
	char sContractYn[1 + 1];

	// 결제 요청 정보 
	//----------------------------승인번호--------------------------------------------
	//간편 결제용
	char szFixKey[24 + 1];
	// 일반 결제용
	char szCardNo[(16 * 4) + 1];
	char szCardValidYm[(4 * 4) + 1];
	char szPasswordNo[(2 * 4) + 1];
	// 일반 결제용 끝

	//공통
	char szSellMm[(2 * 4) + 1];
	char szAmt[(10 * 4) + 1];
	char szBusinessType[(1 * 4) + 1];
	char szRegistryNo[(13 * 4) + 1];
	char szBizNo[(20 * 4) + 1];
	char szShopId[(20 * 4) + 1];
	char szShopMemberId[(20 * 4) + 1];
	char szOrderNo[(80 * 4) + 1];
	char szProductCd[(1000 * 4) + 1];
	char szProductNm[(1000 * 4) + 1];
	char szCardCertYn[(1 * 4) + 1];
	char szZerofeeYn[(1 * 4) + 1];
	char szBuyerNm[(20 * 4) + 1];
	char szRecpNm[(20 * 4) + 1];
	char szRecpAddr[(120 * 4) + 1];
	//char szBuyerIp[(15*4) + 1]; //'.'이 이스케이프 문자로 판독 시
	// 3자리가 더 필요 하다. 12+(3*3)=21
	char szBuyerIp[(20 * 4) + 1];
	char szEmailAddr[(255 * 4) + 1];
	char szBonusYn[(1 * 4) + 1];
	char szGender[(1 * 4) + 1];
	char szBirthYmd[(8 * 4) + 1];

	// 거래일시 설정
	memset(&pScenario->m_CardResInfo, 0x00, sizeof(pScenario->m_CardResInfo));
	CTime curTime = CTime::GetCurrentTime();
	sprintf_s(pScenario->m_CardResInfo.MX_ISSUE_DATE,sizeof(pScenario->m_CardResInfo.MX_ISSUE_DATE), "%04d%02d%02d%02d%02d%02d",
		curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(),
		curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond());

	// 초기화
	//------------------------------------------------------------------------        
	memset(sMsg, 0, sizeof(sMsg));
	memset(at_data, 0, sizeof(at_data));
	memset(at_cross_key, 0, sizeof(at_cross_key));

	memset(sReplyCd, 0, sizeof(sReplyCd));
	memset(sReplyMsg, 0, sizeof(sReplyMsg));
	memset(sPayType, 0, sizeof(sPayType));
	memset(sApprovalNo, 0, sizeof(sApprovalNo));
	memset(sOrderNo, 0, sizeof(sOrderNo));
	memset(sAmt, 0, sizeof(sAmt));
	memset(sApprovalYMDHMS, 0, sizeof(sApprovalYMDHMS));
	memset(sSeqNo, 0, sizeof(sSeqNo));
	memset(sCardId, 0, sizeof(sCardId));
	memset(sCardNm, 0, sizeof(sCardNm));
	memset(sSellMm, 0, sizeof(sSellMm));
	memset(sZerofeeYn, 0, sizeof(sZerofeeYn));
	memset(sCertYn, 0, sizeof(sCertYn));
	memset(sContractYn, 0, sizeof(sContractYn));

	memset(szFixKey, 0, sizeof(szFixKey));

	memset(szCardNo, 0, sizeof(szCardNo));
	memset(szCardValidYm, 0, sizeof(szCardValidYm));
	memset(szPasswordNo, 0, sizeof(szPasswordNo));

	memset(szSellMm, 0, sizeof(szSellMm));
	memset(szAmt, 0, sizeof(szAmt));
	memset(szBusinessType, 0, sizeof(szBusinessType));
	memset(szRegistryNo, 0, sizeof(szRegistryNo));
	memset(szBizNo, 0, sizeof(szBizNo));
	memset(szShopId, 0, sizeof(szShopId));
	memset(szShopMemberId, 0, sizeof(szShopMemberId));
	memset(szOrderNo, 0, sizeof(szOrderNo));
	memset(szProductCd, 0, sizeof(szProductCd));
	memset(szProductNm, 0, sizeof(szProductNm));
	memset(szCardCertYn, 0, sizeof(szCardCertYn));
	memset(szZerofeeYn, 0, sizeof(szZerofeeYn));
	memset(szBuyerNm, 0, sizeof(szBuyerNm));
	memset(szRecpNm, 0, sizeof(szRecpNm));
	memset(szRecpAddr, 0, sizeof(szRecpAddr));
	memset(szBuyerIp, 0, sizeof(szBuyerIp));
	memset(szEmailAddr, 0, sizeof(szEmailAddr));
	memset(szBonusYn, 0, sizeof(szBonusYn));
	memset(szGender, 0, sizeof(szGender));
	memset(szBirthYmd, 0, sizeof(szBirthYmd));

	// 필수 항목 
	strncpy_s(at_cross_key,sizeof(at_cross_key), g_strLicenekey, sizeof(at_cross_key) - 1);  // CrossKey값(최대200자)
	if (strlen(pScenario->m_szMx_opt) > 0)                            // 상점 별 CrossKey값(최대200자)
	{
		memset(at_cross_key, 0, sizeof(at_cross_key));
		strncpy_s(at_cross_key,sizeof(at_cross_key), pScenario->m_szMx_opt, sizeof(at_cross_key) - 1);
	}

	if (strlen(pScenario->m_CardInfo.Fix_Key) < 1)
	{
		// 고객이 입력한 카드 정보
		escapeString(szCardNo, pScenario->m_CardInfo.Card_Num, strlen(pScenario->m_CardInfo.Card_Num));       // 카드 번호(최대 16자)
		escapeString(szCardValidYm, pScenario->m_CardInfo.ExpireDt, 4);  // 카드 유효기간(최대  4자)              : 년월
		escapeString(szPasswordNo, pScenario->m_CardInfo.Password, 2);   // 카드비밀번호(최대  2자)
	}
	else
	{
		escapeString(szFixKey, pScenario->m_CardInfo.Fix_Key, strlen(pScenario->m_CardInfo.Fix_Key));  //카드키(최대 24자)
	}
	
	escapeString(szSellMm, pScenario->m_CardInfo.InstPeriod, 2);     // 할부개월값(최대  2자)

	// 자동 입력 정보
	memset(sTmp, 0x00, sizeof(sTmp));
	sprintf_s(sTmp,sizeof(sTmp), "%d", pScenario->m_nAmount);
	escapeString(szAmt, sTmp, strlen(sTmp));                                  // 금액(최대 10자)

	escapeString(szBusinessType, "0", 1);                          // 결제자 카드종류(최대 1자) : 개인(0),법인(1) 
	escapeString(szRegistryNo, pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo)); // 주민번호(최대 13자리)     : szBusinessType=0 일경우 
	escapeString(szBizNo, "", 0);                                  // 사업자번호(최대 20자리)   : szBusinessType=0 일경우 
	escapeString(szBirthYmd, pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo));   //구매자의 생년월일(최대 8자): YYYYMMDD형식

	if (strlen(pScenario->m_CardInfo.SecretNo) > 6)     // 생년월일 구성이 아니면 법인 번호로 간주
	{
		memset(szBusinessType, 0, sizeof(szBusinessType));
		memset(szRegistryNo, 0, sizeof(szRegistryNo));
		memset(szBizNo, 0, sizeof(szBizNo));
		memset(szBirthYmd, 0, sizeof(szBirthYmd));

		escapeString(szBusinessType, "1", 1);                     // 결제자 카드종류(최대 1자) :  개인(0),법인(1)
		escapeString(szRegistryNo, "", 0);                        // 주민번호(최대 13자리)                 : szBusinessType=1 일경우 
		escapeString(szBizNo, pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo)); // 사업자번호(최대 20자리)               : szBusinessType=1 일경우 
		escapeString(szBirthYmd, "", 0);                          // 구매자의 생년월일(최대 8자)           : YYYYMMDD형식
	}

	// 상점 관련 정보
	escapeString(szShopId, g_strMid, strlen(g_strMid));          //상점ID(최대 20자)
	if (strlen(pScenario->m_szMx_id) > 0)
	{
		memset(szShopId, 0x00, sizeof(szShopId));
		escapeString(szShopId, pScenario->m_szMx_id, strlen(pScenario->m_szMx_id)); // 상점 아이디 
	}

	// 2016.09.09
	// 한국경제 TV 요청으로 수정
	//escapeString(szShopMemberId, "ARS", 3);              // 회원ID(최대 20자)                     : 쇼핑몰회원ID
	escapeString(szShopMemberId, pScenario->m_szInputTel, strlen(pScenario->m_szInputTel) );              // 회원ID(최대 20자)                     : 쇼핑몰회원ID

	// 주문 정보
	escapeString(szOrderNo, pScenario->m_szMx_issue_no, strlen(pScenario->m_szMx_issue_no));      // 주문번호(최대 80자)      : 쇼핑몰 고유 주문번호 
	if (strlen(pScenario->m_CardInfo.Fix_Key) < 1)
	{// 빌키 없으면 이전 방식 유지
		escapeString(szProductCd, pScenario->m_szCC_Prod_Code, strlen(pScenario->m_szCC_Prod_Code));  // 상품코드(최대 1000자)    : 여러 상품의 경우 구분자 이용, 구분자('||':파이프 2개)
		escapeString(szProductNm, pScenario->m_szCC_Prod_Desc, strlen(pScenario->m_szCC_Prod_Desc));  // 상품명(최대 1000자)      : 여러 상품의 경우 구분자 이용, 구분자('||':파이프 2개)
	}
	else
	{//빌키 있으면 다음의 방식 따름
		// szProductCd: 한국경제 TV의 상품 코드(최대 16바이트)^SHA256 변환
		// szProductNm:fix_key
		char szext_data[512 + 1] = { 0x00, };
		_snprintf_s(szext_data, sizeof(szext_data), sizeof(szext_data) - 1,
			"%s^%s", pScenario->m_szCC_Prod_Code, pScenario->m_szext_data);
		escapeString(szProductCd, szext_data, strlen(szext_data));

		//AHN 20260104 상품명에 빌키와 상품명을 함께 입력함. 와우넷의 정산때문에 요청받음 
		// escapeString(szProductNm, pScenario->m_CardInfo.Fix_Key, strlen(pScenario->m_CardInfo.Fix_Key));

		char szProductNm_data[512 + 1] = { 0x00, };
		_snprintf_s(szProductNm_data, sizeof(szProductNm_data), sizeof(szProductNm_data) - 1,
			"%s^%s", pScenario->m_CardInfo.Fix_Key, pScenario->m_szCC_Prod_Desc);
		escapeString(szProductNm, szProductNm_data, strlen(szProductNm_data));

	}
	escapeString(szCardCertYn, "N", 1);                          // 카드인증여부(최대 1자)    : 인증(Y),인증사용않음(N),인증만사용(X) | 공인인증서 및 기타 인증서 제도 사용 여부
	escapeString(szZerofeeYn, "N", 1);                           // 일반/무이자 할부 사용 여부(최대 1자)  : 일반(N), 무이자 할부(Y)  | ARS 결제는 일시불만 사용
	escapeString(szBuyerNm, pScenario->m_szCC_name, strlen(pScenario->m_szCC_name));          // 결제자성명(최대 20자)
	escapeString(szRecpNm, pScenario->m_szCC_name, strlen(pScenario->m_szCC_name));           // 수취인성명(최대 20자) | ARS ㅅ결제에서는 수취인을 별도로 입력 받고있지 않으므로 동일인으로 한다. 물론 추후 달라질 수 있다.
	escapeString(szRecpAddr, "", 0);                             // 수취인주소(최대 120자)        
	escapeString(szBuyerIp, "127.0.0.1", strlen("127.0.0.1"));                     // 결제자 IP(최대15자) - BuyerIp를 넣을수 없다면 "Unknown"으로 세팅
	escapeString(szEmailAddr, pScenario->m_szCC_email, strlen(pScenario->m_szCC_email));       // 결제자 이메일 주소(50자)              : 옵션 필드 (삭제 가능)
	escapeString(szBonusYn, "N", 1);                             // 보너스포인트 사용여부(최대1자)        : 사용(Y), 사용않음(N)
	escapeString(szGender, "M", 1);                              // 구매자 성별(최대 1자)                 : 남자(M)/여자(F)

	initEnc(atEnc);
	if (strlen(pScenario->m_CardInfo.Fix_Key) < 1)
	{
		setValue(atEnc, "allat_card_no", szCardNo);
		setValue(atEnc, "allat_cardvalid_ym", szCardValidYm);
		setValue(atEnc, "allat_passwd_no", szPasswordNo);
	}
	else
	{
		setValue(atEnc, "allat_card_key", szFixKey);
	}
		
	setValue(atEnc, "allat_sell_mm", szSellMm);
	setValue(atEnc, "allat_amt", szAmt);
	setValue(atEnc, "allat_business_type", szBusinessType); //szBusinessType=0 일경우 값 세팅
	setValue(atEnc, "allat_registry_no", szRegistryNo); //szBusinessType=1 일경우 값 세팅
	setValue(atEnc, "allat_biz_no", szBizNo);
	setValue(atEnc, "allat_shop_id", szShopId);
	setValue(atEnc, "allat_shop_member_id", szShopMemberId);
	setValue(atEnc, "allat_order_no", szOrderNo);
	setValue(atEnc, "allat_product_cd", szProductCd);
	setValue(atEnc, "allat_product_nm", szProductNm);
	setValue(atEnc, "allat_cardcert_yn", szCardCertYn);
	setValue(atEnc, "allat_zerofee_yn", szZerofeeYn);
	setValue(atEnc, "allat_buyer_nm", szBuyerNm);
	setValue(atEnc, "allat_recp_name", szRecpNm);
	setValue(atEnc, "allat_recp_addr", szRecpAddr);
	setValue(atEnc, "allat_user_ip", szBuyerIp);
	setValue(atEnc, "kvp_quota", szSellMm);
	setValue(atEnc, "allat_email_addr", szEmailAddr);  //옵션 필드 (삭제 가능)
	setValue(atEnc, "allat_bonus_yn", szBonusYn);
	setValue(atEnc, "allat_gender", szGender);
	setValue(atEnc, "allat_birth_ymd", szBirthYmd);
	if (strlen(pScenario->m_CardInfo.Fix_Key) < 1)
	{
		setValue(atEnc, "allat_pay_type", "NOR");  //수정금지(결제방식 정의)
		setValue(atEnc, "allat_test_yn", "N");  //테스트 :Y, 서비스 :N
	}
	else
	{
		setValue(atEnc, "allat_pay_type", "FIX");  //수정금지(결제방식 정의)
		setValue(atEnc, "allat_test_yn", "N");  //테스트 :Y, 서비스 :N
	}
	setValue(atEnc, "allat_opt_pin", "NOUSE");  //수정금지(올앳 참조 필드)
	setValue(atEnc, "allat_opt_mod", "APP");  //수정금지(올앳 참조 필드)
	sprintf_s(at_data,sizeof(at_data), "allat_shop_id=%s"
		"&allat_enc_data=%s"
		"&allat_amt=%s"
		"&allat_cross_key=%s", szShopId, atEnc, szAmt, at_cross_key);

	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario  AllatArsPayProcess START ", ch);

	if (ApprovalReq(at_data, "SSL", sMsg) != 0){
		/// 에러처리
		pScenario->m_PaySysCd = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
		PAYquithostio("the line serviMX_ISSUE_DATE is Allat ArsPayProcess...FAIL", ch);
		xprintf("[CH:%03d] Allat AllatArsPayProcess end", ch);
		_endthreadex((unsigned int)pScenario->m_hPayThread);
		return 0;
	}

	// 결과 값 가져오기
	//--------------------------------------------------------------------------
	getValue("reply_cd=", sMsg, sReplyCd, sizeof(sReplyCd)); Allat_trim(sReplyCd);
	getValue("reply_msg=", sMsg, sReplyMsg, sizeof(sReplyMsg)); Allat_trim(sReplyMsg);

	xprintf("[CH:%03d] 결과코드        : %s", ch, sReplyCd);
	xprintf("[CH:%03d] 결과메세지      : %s", ch, sReplyMsg);

	memcpy(pScenario->m_CardResInfo.m_szRESULTCODE, sReplyCd, sizeof(pScenario->m_CardResInfo.m_szRESULTCODE) - 1); // 결과 코드
	memcpy(pScenario->m_CardResInfo.m_szRESULTMSG, sReplyMsg, sizeof(pScenario->m_CardResInfo.m_szRESULTMSG) - 1);  // 결과 메시지

	if (strcmp(sReplyCd, "0000") == 0){
		if (strlen(pScenario->m_CardInfo.Fix_Key))
		{
			char sFixKey[24 + 1];  //최대 24자
			memset(sFixKey, 0x00, sizeof(sFixKey));
			getValue("fix_key=", sMsg, sFixKey, sizeof(sFixKey)); Allat_trim(sFixKey);
			if (strlen(sFixKey))
			{
				strncpy_s(pScenario->m_CardResInfo.szBill_Key, sizeof(pScenario->m_CardResInfo.szBill_Key), sFixKey, sizeof(pScenario->m_CardResInfo.szBill_Key) - 1);
				strncpy_s(pScenario->m_szbill_key, sizeof(pScenario->m_szbill_key), sFixKey, sizeof(pScenario->m_szbill_key) - 1);// 빌키
			}
			else
			{
				strncpy_s(pScenario->m_CardResInfo.szBill_Key, sizeof(pScenario->m_CardResInfo.szBill_Key), pScenario->m_CardInfo.Fix_Key, sizeof(pScenario->m_CardResInfo.szBill_Key) - 1);
				strncpy_s(pScenario->m_szbill_key, sizeof(pScenario->m_szbill_key), pScenario->m_CardInfo.Fix_Key, sizeof(pScenario->m_szbill_key) - 1);// 빌키
			}
		}
		getValue("order_no=", sMsg, sOrderNo, sizeof(sOrderNo)); Allat_trim(sOrderNo);
		getValue("amt=", sMsg, sAmt, sizeof(sAmt)); Allat_trim(sAmt);
		getValue("approval_ymdhms=", sMsg, sApprovalYMDHMS, sizeof(sApprovalYMDHMS)); Allat_trim(sApprovalYMDHMS);
		getValue("seq_no=", sMsg, sSeqNo, sizeof(sSeqNo)); Allat_trim(sSeqNo);
		getValue("card_id=", sMsg, sCardId, sizeof(sCardId)); Allat_trim(sCardId);
		getValue("card_nm=", sMsg, sCardNm, sizeof(sCardNm)); Allat_trim(sCardNm);
		getValue("sell_mm=", sMsg, sSellMm, sizeof(sSellMm)); Allat_trim(sSellMm);
		getValue("zerofee_yn=", sMsg, sZerofeeYn, sizeof(sZerofeeYn)); Allat_trim(sZerofeeYn);
		getValue("cert_yn=", sMsg, sCertYn, sizeof(sCertYn)); Allat_trim(sCertYn);
		getValue("contract_yn=", sMsg, sContractYn, sizeof(sContractYn)); Allat_trim(sContractYn);
		getValue("pay_type=", sMsg, sPayType, sizeof(sPayType)); Allat_trim(sPayType);
		getValue("approval_no=", sMsg, sApprovalNo, sizeof(sApprovalNo)); Allat_trim(sApprovalNo);

		memcpy(pScenario->m_CardResInfo.m_szMoid, sOrderNo, sizeof(pScenario->m_CardResInfo.m_szMoid) - 1);        // 주문번호
		memcpy(pScenario->m_CardResInfo.m_AMT, sAmt, sizeof(pScenario->m_CardResInfo.m_AMT) - 1);                  // 거래 금액
		memcpy(pScenario->m_CardResInfo.m_PAYMETHOD, sPayType, sizeof(pScenario->m_CardResInfo.m_PAYMETHOD) - 1);  // 지불수단(결제 수단)
		memcpy(pScenario->m_CardResInfo.m_AUTHDATE, sApprovalYMDHMS, sizeof(pScenario->m_CardResInfo.m_AUTHDATE) - 1); // 승인일시
		memcpy(pScenario->m_CardResInfo.m_Tid, sSeqNo, sizeof(pScenario->m_CardResInfo.m_Tid) - 1); // 거래일련번호

		//신용 카드 정보
		memcpy(pScenario->m_CardResInfo.m_AUTHCODE, sApprovalNo, sizeof(pScenario->m_CardResInfo.m_AUTHCODE) - 1);     // 승인번호
		memcpy(pScenario->m_CardResInfo.m_AcquCardCode, sCardId, sizeof(pScenario->m_CardResInfo.m_AcquCardCode) - 1); // 매입 카드 사 ID 
		memcpy(pScenario->m_CardResInfo.m_AcquCardName, sCardNm, sizeof(pScenario->m_CardResInfo.m_AcquCardName) - 1); // 매입 카드 사명
		memcpy(pScenario->m_CardResInfo.m_CARDCODE, sCardId, sizeof(pScenario->m_CardResInfo.m_CARDCODE) - 1);         // 신용카드 사 ID 
		memcpy(pScenario->m_CardResInfo.m_CARDNAME, sCardNm, sizeof(pScenario->m_CardResInfo.m_CARDNAME) - 1);         // 신용카드 사 명
		memcpy(pScenario->m_CardResInfo.m_szCardQuota, sSellMm, sizeof(pScenario->m_CardResInfo.m_szCardQuota) - 1);     // 할부개월
		memcpy(pScenario->m_CardResInfo.m_sZerofeeYn, sZerofeeYn, sizeof(pScenario->m_CardResInfo.m_sZerofeeYn) - 1);    // 무이자여부
		memcpy(pScenario->m_CardResInfo.m_sCertYn, sCertYn, sizeof(pScenario->m_CardResInfo.m_sCertYn) - 1);              // 인증여부
		memcpy(pScenario->m_CardResInfo.m_sContractYn, sContractYn, sizeof(pScenario->m_CardResInfo.m_sContractYn) - 1); // 직가맹여부

		memcpy(pScenario->m_CardResInfo.m_szGOODSNAME, pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_CardResInfo.m_sContractYn) - 1); // 상품명
		memcpy(pScenario->m_CardResInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_CardResInfo.m_szMid) - 1);//상번ID
		//memcpy(pScenario->m_CardResInfo.m_PartialCancelCode, pScenario->m_szCC_Prod_Desc, sizeof(pScenario->m_CardResInfo.m_sContractYn) - 1); // 상품명

		/**************  결과 값 출력 ******************/
		xprintf("[CH:%03d] 주문번호        : %s", ch, sOrderNo);
		xprintf("[CH:%03d] 승인금액        : %s", ch, sAmt);
		xprintf("[CH:%03d] 지불수단        : %s", ch, sPayType);
		xprintf("[CH:%03d] 승인일시        : %s", ch, sApprovalYMDHMS);
		xprintf("[CH:%03d] 거래일련번호    : %s", ch, sSeqNo);
		xprintf("[CH:%03d] 상품코드    : %s", ch, szProductCd);
		xprintf("[CH:%03d] ========= 신용 카드 =============", ch);
		xprintf("[CH:%03d] 승인번호        : %s", ch, sApprovalNo);
		xprintf("[CH:%03d] 카드ID          : %s", ch, sCardId);
		xprintf("[CH:%03d] 카드명          : %s", ch, sCardNm);
		xprintf("[CH:%03d] 할부개월        : %s", ch, sSellMm);
		xprintf("[CH:%03d] 무이자여부      : %s", ch, sZerofeeYn);
		xprintf("[CH:%03d] 인증여부        : %s", ch, sCertYn);
		xprintf("[CH:%03d] 직가맹여부      : %s", ch, sContractYn);
	}
	else{
		// reply_cd 가 "0000" 아닐때는 에러 (자세한 내용은 매뉴얼참조)
		// reply_msg 는 실패에 대한 메세지        
		memcpy(pScenario->m_CardResInfo.m_szMoid, pScenario->m_szMx_issue_no, sizeof(pScenario->m_CardResInfo.m_szMoid) - 1);        // 주문번호
		memset(sTmp, 0x00, sizeof(sTmp));
		sprintf_s(sTmp,sizeof(sTmp), "%d", pScenario->m_nAmount);
		memcpy(pScenario->m_CardResInfo.m_AMT, sTmp, sizeof(pScenario->m_CardResInfo.m_AMT) - 1);      // 거래 금액
		memcpy(pScenario->m_CardResInfo.m_szCardQuota, pScenario->m_CardInfo.InstPeriod, sizeof(pScenario->m_CardResInfo.m_szCardQuota) - 1);  // 할부개월수
		memcpy(pScenario->m_CardResInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_CardResInfo.m_szMid) - 1);//상번ID

		xprintf("[CH:%03d] 주문번호        : %s", ch, pScenario->m_CardResInfo.m_szMoid);
		xprintf("[CH:%03d] 결제금액      : %s", ch, pScenario->m_CardResInfo.m_AMT);
		xprintf("[CH:%03d] 할부개월수      : %s", ch, pScenario->m_CardResInfo.m_szCardQuota);
		xprintf("[CH:%03d] 가맹점ID      : %s", ch, pScenario->m_CardResInfo.m_szMid);
		xprintf("[CH:%03d] 전송일시      :%s", ch, pScenario->m_CardResInfo.MX_ISSUE_DATE);
	}

	pScenario->m_PaySysCd = 1;
	(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
	PAYquithostio("the line service is Allat ArsPayProcess.", ch);
	xprintf("[CH:%03d] Allat ArsPayProcess end", ch);
	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario  AllatArsPayProcess END ", ch);
	_endthreadex((unsigned int)pScenario->m_hPayThread);
	return 0;
}


// 간편 결제 결제키 해지(올앳) 요청 쓰레드용 함수 
unsigned int __stdcall AllatArsPayFixKeyCancleProcess(void *data)
{
	int			ch;
	int			paythreadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	paythreadID = pScenario->paythreadID;

	pScenario->m_PaySysCd = 0;
	if (paythreadID != pScenario->paythreadID) {
		pScenario->m_PaySysCd = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		PAYquithostio("AllatArsPayFixKeyCancleProcess the line service is not valid any more.", ch);
		xprintf("[CH:%03d] AllatArsPayFixKeyCancleProcess END", ch);
		_endthreadex((unsigned int)pScenario->m_hPayThread);
		return 0;
	}

	ALLAT_ENCDATA atEnc;
	char sMsg[8191 + 1];
	char at_data[8191 + 1];
	char at_cross_key[255 + 1];

	// 결과 값 정의 
	//------------------------------------------------------------------------    
	char sReplyCd[(4 * 4) + 1];
	char sReplyMsg[(400 * 4) + 1];
	char sCancelYMDHMS[(14 * 4) + 1];
	char sPartCancelFlag[(1 * 4) + 1];
	char sRemainAmt[(10 * 4) + 1];
	char sPayType[(10 * 4) + 1];
	char sFixKey[24 + 1];  //최대 24자
	char sCancelYmd[8 + 1];

	// 취소 요청 정보
	//------------------------------------------------------------------------    
	char szShopId[20 + 1];
	char szAmt[10 + 1];
	char szOrderNo[80 + 1];
	char szPayType[6 + 1];
	char szSeqNo[10 + 1];


	char szFixKey[24 + 1]; 
	char szFixType[20 + 1];
	char szTestYn[1 + 1];

	// 거래일시 설정
	memset(&pScenario->m_Cancel_ResInfo, 0x00, sizeof(pScenario->m_CardResInfo));
	CTime curTime = CTime::GetCurrentTime();
	sprintf_s(pScenario->m_Cancel_ResInfo.MX_ISSUE_DATE, sizeof(pScenario->m_Cancel_ResInfo.MX_ISSUE_DATE), "%04d%02d%02d%02d%02d%02d",
		curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(),
		curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond());

	// 초기화
	//------------------------------------------------------------------------        
	memset(sMsg, 0, sizeof(sMsg));
	memset(at_data, 0, sizeof(at_data));
	memset(at_cross_key, 0, sizeof(at_cross_key));

	memset(sFixKey, 0, sizeof(sFixKey));
	memset(sCancelYmd, 0, sizeof(sCancelYmd));

	memset(sReplyCd, 0, sizeof(sReplyCd));
	memset(sReplyMsg, 0, sizeof(sReplyMsg));
	memset(sCancelYMDHMS, 0, sizeof(sCancelYMDHMS));
	memset(sPartCancelFlag, 0, sizeof(sPartCancelFlag));
	memset(sRemainAmt, 0, sizeof(sRemainAmt));
	memset(sPayType, 0, sizeof(sPayType));

	memset(szShopId, 0, sizeof(szShopId));
	memset(szAmt, 0, sizeof(szAmt));
	memset(szOrderNo, 0, sizeof(szOrderNo));
	memset(szPayType, 0, sizeof(szPayType));
	memset(szSeqNo, 0, sizeof(szSeqNo));
	
	memset(szFixKey, 0, sizeof(szFixKey)); 
	memset(szFixType, 0, sizeof(szFixType));
	memset(szTestYn, 0, sizeof(szTestYn));

	// 정보 입력
	//------------------------------------------------------------------------        
	// 필수 항목 
	strncpy_s(at_cross_key, sizeof(at_cross_key), g_strLicenekey, sizeof(at_cross_key) - 1);  // CrossKey값(최대200자)
	if (strlen(pScenario->m_szMx_opt) > 0)                            // 상점 별 CrossKey값(최대200자)
	{
		memset(at_cross_key, 0, sizeof(at_cross_key));
		strncpy_s(at_cross_key, sizeof(at_cross_key), pScenario->m_szMx_opt, sizeof(at_cross_key) - 1);
	}

	// 상점 관련 정보
	escapeString(szShopId, g_strMid, strlen(g_strMid));          //상점ID(최대 20자)
	if (strlen(pScenario->m_Card_CancleInfo.m_szMid) > 0)
	{
		memset(szShopId, 0x00, sizeof(szShopId));
		escapeString(szShopId, pScenario->m_Card_CancleInfo.m_szMid, strlen(pScenario->m_Card_CancleInfo.m_szMid)); // 상점 아이디 
	}
	
	escapeString(szFixKey, pScenario->m_Card_CancleInfo.Fix_Key, strlen(pScenario->m_Card_CancleInfo.Fix_Key));			//(필수)카드인증Key	
	escapeString(szFixType, "", 0);		//(옵션)정기과금타입( FIX : 상점정기과금, HOF : 호스팅정기과금, D
	escapeString(szTestYn, "", 0);			//(옵션)테스트여부( Y : 테스트, N : 서비스 )

	initEnc(atEnc);
	setValue(atEnc, "allat_shop_id", szShopId);

	setValue(atEnc, "allat_fix_key", szFixKey);
	setValue(atEnc, "allat_fix_type", szFixType);
	setValue(atEnc, "allat_test_yn", "N");     // 테스트 :Y, 서비스 :N
	setValue(atEnc, "allat_opt_pin", "NOUSE"); // 수정금지(올앳 참조 필드)
	setValue(atEnc, "allat_opt_mod", "APP");   // 수정금지(올앳 참조 필드)
	//setValue(atEnc, "allat_seq_no", szSeqNo);  // 옵션 필드( 삭제 가능함 )

	sprintf_s(at_data, sizeof(at_data), "allat_shop_id=%s"
		"&allat_enc_data=%s"
		"&allat_amt=%s"
		"&allat_cross_key=%s", szShopId, atEnc, szAmt, at_cross_key);

	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario  AllatArsPayFixKeyCancleProcess START ", ch);

	if (CertCancelReq(at_data, "SSL", sMsg) != 0){
		/// 에러처리
		pScenario->m_PaySysCd = 1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
		PAYquithostio("the line service is AllatArsPayFixKeyCancleProcess...FAIL", ch);
		xprintf("[CH:%03d] AllatArsPayFixKeyCancleProcess end", ch);
		_endthreadex((unsigned int)pScenario->m_hPayThread);
		return 0;
	}

	// 결과 값 가져오기
	//--------------------------------------------------------------------------
	getValue("reply_cd=", sMsg, sReplyCd, sizeof(sReplyCd)); Allat_trim(sReplyCd);
	getValue("reply_msg=", sMsg, sReplyMsg, sizeof(sReplyMsg)); Allat_trim(sReplyMsg);

	xprintf("[CH:%03d] 결과코드        : %s", ch, sReplyCd);
	xprintf("[CH:%03d] 결과메세지      : %s", ch, sReplyMsg);

	memcpy(pScenario->m_CardResInfo.m_szRESULTCODE, sReplyCd, sizeof(pScenario->m_CardResInfo.m_szRESULTCODE) - 1); // 결과 코드
	memcpy(pScenario->m_CardResInfo.m_szRESULTMSG, sReplyMsg, sizeof(pScenario->m_CardResInfo.m_szRESULTMSG) - 1);  // 결과 메시지

	if (strcmp(sReplyCd, "0000") == 0){
		getValue("cancel_ymdhms=", sMsg, sCancelYMDHMS, sizeof(sCancelYMDHMS)); Allat_trim(sCancelYMDHMS);
		getValue("part_cancel_flag=", sMsg, sPartCancelFlag, sizeof(sPartCancelFlag)); Allat_trim(sPartCancelFlag);
		getValue("remain_amt=", sMsg, sRemainAmt, sizeof(sRemainAmt)); Allat_trim(sRemainAmt);
		getValue("pay_type=", sMsg, sPayType, sizeof(sPayType)); Allat_trim(sPayType);

		memcpy(&pScenario->m_Cancel_ResInfo.m_AUTHDATE, sCancelYMDHMS, sizeof(pScenario->m_Cancel_ResInfo.m_AUTHDATE) - 1);                     // 취소일시
		memcpy(&pScenario->m_Cancel_ResInfo.m_PartialCancelCode, sPartCancelFlag, sizeof(pScenario->m_Cancel_ResInfo.m_PartialCancelCode) - 1); // 취소구분
		memcpy(&pScenario->m_Cancel_ResInfo.sRemainAmt, sRemainAmt, sizeof(pScenario->m_Cancel_ResInfo.sRemainAmt) - 1);                        // 취소 후 잔액
		memcpy(&pScenario->m_Cancel_ResInfo.m_PAYMETHOD, sPayType, sizeof(pScenario->m_Cancel_ResInfo.m_PAYMETHOD) - 1);                        // 거래방식구분

		xprintf("[CH:%03d] 취소일시        : %s", ch, sCancelYMDHMS);
		xprintf("[CH:%03d] 취소구분        : %s", ch, sPartCancelFlag);
		xprintf("[CH:%03d] 잔액            : %s", ch, sRemainAmt);
		xprintf("[CH:%03d] 거래방식구분    : %s", ch, sPayType);
	}
	else{
		// reply_cd 가 "0000" 아닐때는 에러 (자세한 내용은 매뉴얼참조)
		// reply_msg 는 실패에 대한 메세지        
		memcpy(pScenario->m_Cancel_ResInfo.m_szMoid, pScenario->m_szMx_issue_no, sizeof(pScenario->m_Cancel_ResInfo.m_szMoid) - 1);        // 주문번호
		char sTmp[20 + 1] = { 0x00, };
		memset(sTmp, 0x00, sizeof(sTmp));
		sprintf_s(sTmp, sizeof(sTmp), "%d", pScenario->m_nAmount);
		memcpy(pScenario->m_Cancel_ResInfo.m_AMT, sTmp, sizeof(pScenario->m_Cancel_ResInfo.m_AMT) - 1);      // 거래 금액
		memcpy(pScenario->m_Cancel_ResInfo.m_szCardQuota, pScenario->m_CardInfo.InstPeriod, sizeof(pScenario->m_Cancel_ResInfo.m_szCardQuota) - 1);  // 할부개월수
		memcpy(pScenario->m_Cancel_ResInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_Cancel_ResInfo.m_szMid) - 1);//상번ID


		xprintf("[CH:%03d] 주문번호        : %s", ch, pScenario->m_Cancel_ResInfo.m_szMoid);
		xprintf("[CH:%03d] 결제금액      : %s", ch, pScenario->m_Cancel_ResInfo.m_AMT);
		xprintf("[CH:%03d] 할부개월수      : %s", ch, pScenario->m_Cancel_ResInfo.m_szCardQuota);
		xprintf("[CH:%03d] 가맹점ID      : %s", ch, pScenario->m_Cancel_ResInfo.m_szMid);
		xprintf("[CH:%03d] 전송일시      :%s", ch, pScenario->m_Cancel_ResInfo.MX_ISSUE_DATE);
	}

	pScenario->m_PaySysCd = 1;
	(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
	PAYquithostio("the line service is AllatArsPayFixKeyCancleProcess.", ch);
	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario  AllatArsPayFixKeyCancleProcess END ", ch);
	_endthreadex((unsigned int)pScenario->m_hPayThread);
	return 0;

}

// 간편 결제키 생성 요청 쓰레드용 함수 
unsigned int __stdcall AllatArsGetFixKeyProcess(void *data)
{
	int			ch;
	int			paythreadID;
	char        AnsiNO_Char[257] = { 0x00, };
	LPMTP		*lineTablePtr = (LPMTP *)data;
	CALLAT_WOWTV_Billkey_Easy_Scenario *pScenario = (CALLAT_WOWTV_Billkey_Easy_Scenario *)(lineTablePtr->pScenario);

	ch = lineTablePtr->chanID;
	paythreadID = pScenario->paythreadID;

	xprintf("[CH:%03d] Allat AllatArsGetFixKeyProcess START", ch);
	//리턴값
	//=====================
	//쿼리인자변수선언
	//=====================
	pScenario->m_PaySysCd = 0;
	if (paythreadID != pScenario->paythreadID) {
		pScenario->m_PaySysCd = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_COMM;
		PAYquithostio("Allat AllatArsGetFixKeyProcess the line service is not valid any more.", ch);
		xprintf("[CH:%03d] Allat AllatArsGetFixKeyProcess end", ch);
		_endthreadex((unsigned int)pScenario->m_hPayThread);
		return 0;
	}

	ALLAT_ENCDATA atEnc;
	char sMsg[8191 + 1];
	char at_data[8191 + 1];
	char at_cross_key[255 + 1];
	char sTmp[8191 + 1];

	// 결과 값 정의 
	//------------------------------------------------------------------------
	char sReplyCd[4 + 1];
	char sReplyMsg[400 + 1];
	char sPayType[10 + 1];
	char sApprovalNo[10 + 1];
	char sOrderNo[80 + 1];
	char sAmt[10 + 1];
	char sApprovalYMDHMS[14 + 1];
	char sSeqNo[10 + 1];
	char sCardId[2 + 1];
	char sCardNm[48 + 1];
	char sSellMm[2 + 1];
	char sZerofeeYn[1 + 1];
	char sCertYn[1 + 1];
	char sContractYn[1 + 1];	
    char sFixKey[ 24+1];  //최대 24자
	char sApplyYmd[8 + 1];

	// 결제 요청 정보 
	//----------------------------승인번호--------------------------------------------
	// URL 인코딩 시 한글 및 특수 문자 널바이트의 경우 %헥사값 두바이트로 늘어 나며
	// UTF 8 의 경우는 최대 4배가 늘어 나는 것을 가정하여 메모리를 운용해야 한다.
	char szCardNo[(16 * 4) + 1];
	char szCardValidYm[(4 * 4) + 1];
	char szPasswordNo[(2 * 4) + 1];
	char szSellMm[(2 * 4) + 1];
	char szAmt[(10 * 4) + 1];
	char szBusinessType[(1 * 4) + 1];
	char szRegistryNo[(13 * 4) + 1];
	char szBizNo[(20 * 4) + 1];
	char szShopId[(20 * 4) + 1];
	char szShopMemberId[(20 * 4) + 1];
	char szOrderNo[(80 * 4) + 1];
	char szProductCd[(1000 * 4) + 1];
	char szProductNm[(1000 * 4) + 1];
	char szCardCertYn[(1 * 4) + 1];
	char szZerofeeYn[(1 * 4) + 1];
	char szBuyerNm[(20 * 4) + 1];
	char szRecpNm[(20 * 4) + 1];
	char szRecpAddr[(120 * 4) + 1];
	//char szBuyerIp[(15*4) + 1]; //'.'이 이스케이프 문자로 판독 시
	// 3자리가 더 필요 하다. 12+(3*3)=21
	char szBuyerIp[(20 * 4) + 1];
	char szEmailAddr[(255 * 4) + 1];
	char szBonusYn[(1 * 4) + 1];
	char szGender[(1 * 4) + 1];
	char szBirthYmd[(8 * 4) + 1];

	char szFixType[(24 * 4) + 1];
	char szTestYn[(1 * 4) + 1];

	// 거래일시 설정
	memset(&pScenario->m_CardResInfo, 0x00, sizeof(pScenario->m_CardResInfo));
	CTime curTime = CTime::GetCurrentTime();
	sprintf_s(pScenario->m_CardResInfo.MX_ISSUE_DATE, sizeof(pScenario->m_CardResInfo.MX_ISSUE_DATE), "%04d%02d%02d%02d%02d%02d",
		curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(),
		curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond());

	// 초기화
	//------------------------------------------------------------------------        
	memset(sMsg, 0, sizeof(sMsg));
	memset(at_data, 0, sizeof(at_data));
	memset(at_cross_key, 0, sizeof(at_cross_key));

	memset(sReplyCd, 0, sizeof(sReplyCd));
	memset(sReplyMsg, 0, sizeof(sReplyMsg));
	memset(sPayType, 0, sizeof(sPayType));
	memset(sApprovalNo, 0, sizeof(sApprovalNo));
	memset(sOrderNo, 0, sizeof(sOrderNo));
	memset(sAmt, 0, sizeof(sAmt));
	memset(sApprovalYMDHMS, 0, sizeof(sApprovalYMDHMS));
	memset(sSeqNo, 0, sizeof(sSeqNo));
	memset(sCardId, 0, sizeof(sCardId));
	memset(sCardNm, 0, sizeof(sCardNm));
	memset(sSellMm, 0, sizeof(sSellMm));
	memset(sZerofeeYn, 0, sizeof(sZerofeeYn));
	memset(sCertYn, 0, sizeof(sCertYn));
	memset(sContractYn, 0, sizeof(sContractYn));

	memset(szCardNo, 0, sizeof(szCardNo));
	memset(szCardValidYm, 0, sizeof(szCardValidYm));
	memset(szPasswordNo, 0, sizeof(szPasswordNo));
	memset(szSellMm, 0, sizeof(szSellMm));
	memset(szAmt, 0, sizeof(szAmt));
	memset(szBusinessType, 0, sizeof(szBusinessType));
	memset(szRegistryNo, 0, sizeof(szRegistryNo));
	memset(szBizNo, 0, sizeof(szBizNo));
	memset(szShopId, 0, sizeof(szShopId));
	memset(szShopMemberId, 0, sizeof(szShopMemberId));
	memset(szOrderNo, 0, sizeof(szOrderNo));
	memset(szProductCd, 0, sizeof(szProductCd));
	memset(szProductNm, 0, sizeof(szProductNm));
	memset(szCardCertYn, 0, sizeof(szCardCertYn));
	memset(szZerofeeYn, 0, sizeof(szZerofeeYn));
	memset(szBuyerNm, 0, sizeof(szBuyerNm));
	memset(szRecpNm, 0, sizeof(szRecpNm));
	memset(szRecpAddr, 0, sizeof(szRecpAddr));
	memset(szBuyerIp, 0, sizeof(szBuyerIp));
	memset(szEmailAddr, 0, sizeof(szEmailAddr));
	memset(szBonusYn, 0, sizeof(szBonusYn));
	memset(szGender, 0, sizeof(szGender));
	memset(szBirthYmd, 0, sizeof(szBirthYmd));

	memset(szFixType, 0, sizeof(szFixType));
	memset(szTestYn, 0, sizeof(szTestYn));

	// 필수 항목 
	strncpy_s(at_cross_key, sizeof(at_cross_key), g_strLicenekey, sizeof(at_cross_key) - 1);  // CrossKey값(최대200자)
	if (strlen(pScenario->m_szMx_opt) > 0)                            // 상점 별 CrossKey값(최대200자)
	{
		memset(at_cross_key, 0, sizeof(at_cross_key));
		strncpy_s(at_cross_key, sizeof(at_cross_key), pScenario->m_szMx_opt, sizeof(at_cross_key) - 1);
	}

	// 고객이 입력한 카드 정보
	escapeString(szCardNo, pScenario->m_CardInfo.Card_Num, strlen(pScenario->m_CardInfo.Card_Num));       // 카드 번호(최대 16자)
	escapeString(szCardValidYm, pScenario->m_CardInfo.ExpireDt, 4);  // 카드 유효기간(최대  4자)              : 년월
	escapeString(szPasswordNo, pScenario->m_CardInfo.Password, 2);   // 카드비밀번호(최대  2자)
	escapeString(szSellMm, pScenario->m_CardInfo.InstPeriod, 2);     // 할부개월값(최대  2자)

	// 자동 입력 정보
	memset(sTmp, 0x00, sizeof(sTmp));
	sprintf_s(sTmp, sizeof(sTmp), "%d", pScenario->m_nAmount);
	escapeString(szAmt, sTmp, strlen(sTmp));                                  // 금액(최대 10자)

	escapeString(szBusinessType, "0", 1);                          // 결제자 카드종류(최대 1자) : 개인(0),법인(1) 
	escapeString(szRegistryNo, pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo)); // 주민번호(최대 13자리)     : szBusinessType=0 일경우 
	escapeString(szBizNo, "", 0);                                  // 사업자번호(최대 20자리)   : szBusinessType=0 일경우 
	escapeString(szBirthYmd, pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo));   //구매자의 생년월일(최대 8자): YYYYMMDD형식

	xprintf("[CH:%03d] AHN 주민번호         : %s", ch, szRegistryNo);
	xprintf("[CH:%03d] AHN 생년월일         : %s", ch, szBirthYmd);

	if (strlen(pScenario->m_CardInfo.SecretNo) > 6)     // 생년월일 구성이 아니면 법인 번호로 간주
	{
		memset(szBusinessType, 0, sizeof(szBusinessType));
		memset(szRegistryNo, 0, sizeof(szRegistryNo));
		memset(szBizNo, 0, sizeof(szBizNo));
		memset(szBirthYmd, 0, sizeof(szBirthYmd));

		escapeString(szBusinessType, "1", 1);                     // 결제자 카드종류(최대 1자) :  개인(0),법인(1)
		escapeString(szRegistryNo, "", 0);                        // 주민번호(최대 13자리)                 : szBusinessType=1 일경우 
		escapeString(szBizNo, pScenario->m_CardInfo.SecretNo, strlen(pScenario->m_CardInfo.SecretNo)); // 사업자번호(최대 20자리)               : szBusinessType=1 일경우 
		escapeString(szBirthYmd, "", 0);                          // 구매자의 생년월일(최대 8자)           : YYYYMMDD형식
	}

	// 상점 관련 정보
	escapeString(szShopId, g_strMid, strlen(g_strMid));          //상점ID(최대 20자)
	if (strlen(pScenario->m_szMx_id) > 0)
	{
		memset(szShopId, 0x00, sizeof(szShopId));
		escapeString(szShopId, pScenario->m_szMx_id, strlen(pScenario->m_szMx_id)); // 상점 아이디 
	}

	// 2016.09.09
	// 한국경제 TV 요청으로 수정
	//escapeString(szShopMemberId, "ARS", 3);              // 회원ID(최대 20자)                     : 쇼핑몰회원ID
	escapeString(szShopMemberId, pScenario->m_szInputTel, strlen(pScenario->m_szInputTel));              // 회원ID(최대 20자)                     : 쇼핑몰회원ID

	// 주문 정보
	escapeString(szOrderNo, pScenario->m_szMx_issue_no, strlen(pScenario->m_szMx_issue_no));      // 주문번호(최대 80자)      : 쇼핑몰 고유 주문번호 
	escapeString(szProductCd, pScenario->m_szCC_Prod_Code, strlen(pScenario->m_szCC_Prod_Code));  // 상품코드(최대 1000자)    : 여러 상품의 경우 구분자 이용, 구분자('||':파이프 2개)
	escapeString(szProductNm, pScenario->m_szCC_Prod_Desc, strlen(pScenario->m_szCC_Prod_Desc));  // 상품명(최대 1000자)      : 여러 상품의 경우 구분자 이용, 구분자('||':파이프 2개)
	escapeString(szCardCertYn, "N", 1);                          // 카드인증여부(최대 1자)    : 인증(Y),인증사용않음(N),인증만사용(X) | 공인인증서 및 기타 인증서 제도 사용 여부
	escapeString(szZerofeeYn, "N", 1);                           // 일반/무이자 할부 사용 여부(최대 1자)  : 일반(N), 무이자 할부(Y)  | ARS 결제는 일시불만 사용
	escapeString(szBuyerNm, pScenario->m_szCC_name, strlen(pScenario->m_szCC_name));          // 결제자성명(최대 20자)
	escapeString(szRecpNm, pScenario->m_szCC_name, strlen(pScenario->m_szCC_name));           // 수취인성명(최대 20자) | ARS ㅅ결제에서는 수취인을 별도로 입력 받고있지 않으므로 동일인으로 한다. 물론 추후 달라질 수 있다.
	escapeString(szRecpAddr, "", 0);                             // 수취인주소(최대 120자)        
	escapeString(szBuyerIp, "127.0.0.1", strlen("127.0.0.1"));                     // 결제자 IP(최대15자) - BuyerIp를 넣을수 없다면 "Unknown"으로 세팅
	escapeString(szEmailAddr, pScenario->m_szCC_email, strlen(pScenario->m_szCC_email));       // 결제자 이메일 주소(50자)              : 옵션 필드 (삭제 가능)
	escapeString(szBonusYn, "N", 1);                             // 보너스포인트 사용여부(최대1자)        : 사용(Y), 사용않음(N)
	escapeString(szGender, "M", 1);                              // 구매자 성별(최대 1자)                 : 남자(M)/여자(F)

	//실제 액세스할 길이 명시
	escapeString(szFixType, "", 0);            //(옵션)정기과금타입( FIX : 상점정기과금, HOF : 호스팅정기과금, Default : FIX )
	escapeString(szTestYn, "", 0);            //(옵션)테스트여부( Y : 테스트, N : 서비스 )

	initEnc(atEnc);
	setValue(atEnc, "allat_card_no", szCardNo);
	setValue(atEnc, "allat_cardvalid_ym", szCardValidYm);
	setValue(atEnc, "allat_passwd_no", szPasswordNo);
	setValue(atEnc, "allat_sell_mm", szSellMm);
	setValue(atEnc, "allat_amt", szAmt);
	setValue(atEnc, "allat_business_type", szBusinessType); //szBusinessType=0 일경우 값 세팅
	setValue(atEnc, "allat_registry_no", szRegistryNo); //szBusinessType=1 일경우 값 세팅

	setValue(atEnc, "allat_fix_type", szFixType);

	setValue(atEnc, "allat_biz_no", szBizNo);
	setValue(atEnc, "allat_shop_id", szShopId);
	setValue(atEnc, "allat_shop_member_id", szShopMemberId);
#if 1 //AHN 20180809 올앳에서 빌키채번에서 사용한 주문번호와 승인에서 사용할 주문번호가 중복된다는 오류가 있어서 빌키채번에 사용할 임시 주문번호를 만든다.
	char tmp_szOrderNo[(80 * 4) + 1];
	sprintf_s(tmp_szOrderNo, sizeof(tmp_szOrderNo), "%s_b", szOrderNo);
	xprintf("[CH:%03d] 원본주문번호        : [%s]", ch, szOrderNo);
	xprintf("[CH:%03d] 임시주문번호        : [%s]", ch, tmp_szOrderNo);
	setValue(atEnc, "allat_order_no", tmp_szOrderNo);
#else
	setValue(atEnc, "allat_order_no", szOrderNo);
#endif
	setValue(atEnc, "allat_product_cd", szProductCd);
	setValue(atEnc, "allat_product_nm", szProductNm);
	setValue(atEnc, "allat_cardcert_yn", szCardCertYn);
	setValue(atEnc, "allat_zerofee_yn", szZerofeeYn);
	setValue(atEnc, "allat_buyer_nm", szBuyerNm);
	setValue(atEnc, "allat_recp_name", szRecpNm);
	setValue(atEnc, "allat_recp_addr", szRecpAddr);
	setValue(atEnc, "allat_user_ip", szBuyerIp);
	setValue(atEnc, "kvp_quota", szSellMm);
	setValue(atEnc, "allat_email_addr", szEmailAddr);  //옵션 필드 (삭제 가능)
	setValue(atEnc, "allat_bonus_yn", szBonusYn);
	setValue(atEnc, "allat_gender", szGender);
	setValue(atEnc, "allat_birth_ymd", szBirthYmd);
	setValue(atEnc, "allat_pay_type", "NOR");  //수정금지(결제방식 정의)
	setValue(atEnc, "allat_test_yn", "N");  //테스트 :Y, 서비스 :N
	setValue(atEnc, "allat_opt_pin", "NOUSE");  //수정금지(올앳 참조 필드)
	setValue(atEnc, "allat_opt_mod", "APP");  //수정금지(올앳 참조 필드)
	sprintf_s(at_data, sizeof(at_data), "allat_shop_id=%s"
		"&allat_enc_data=%s"
		"&allat_amt=%s"
		"&allat_cross_key=%s", szShopId, atEnc, szAmt, at_cross_key);

	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario  AllatArsGetFixKeyProcess START ", ch);

	if (CertRegReq(at_data, "SSL", sMsg) != 0){
		/// 에러처리
		pScenario->m_PaySysCd = -1;
		(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
		PAYquithostio("the line serviMX_ISSUE_DATE is Allat AllatArsGetFixKeyProcess...FAIL", ch);
		xprintf("[CH:%03d] Allat AllatArsGetFixKeyProcess end", ch);
		_endthreadex((unsigned int)pScenario->m_hPayThread);
		return 0;
	}

	// 결과 값 가져오기
	//--------------------------------------------------------------------------
	getValue("reply_cd=", sMsg, sReplyCd, sizeof(sReplyCd)); Allat_trim(sReplyCd);
	getValue("reply_msg=", sMsg, sReplyMsg, sizeof(sReplyMsg)); Allat_trim(sReplyMsg);

	xprintf("[CH:%03d] 결과코드        : %s", ch, sReplyCd);
	xprintf("[CH:%03d] 결과메세지      : %s", ch, sReplyMsg);

	memcpy(pScenario->m_CardResInfo.m_szRESULTCODE, sReplyCd, sizeof(pScenario->m_CardResInfo.m_szRESULTCODE) - 1); // 결과 코드
	memcpy(pScenario->m_CardResInfo.m_szRESULTMSG, sReplyMsg, sizeof(pScenario->m_CardResInfo.m_szRESULTMSG) - 1);  // 결과 메시지

	memset(pScenario->m_CardResInfo.szBill_Key, 0x00, sizeof(pScenario->m_CardResInfo.szBill_Key));
	if (strcmp(sReplyCd, "0000") == 0){
		getValue("fix_key=", sMsg, sFixKey, sizeof(sFixKey)); Allat_trim(sFixKey);
		strncpy_s(pScenario->m_CardResInfo.szBill_Key, sizeof(pScenario->m_CardResInfo.szBill_Key), sFixKey, sizeof(pScenario->m_CardResInfo.szBill_Key) - 1);
		strncpy_s(pScenario->m_szbill_key, sizeof(pScenario->m_szbill_key), sFixKey, sizeof(pScenario->m_szbill_key) - 1);// 빌키
		
		getValue("apply_ymd=", sMsg, sApplyYmd, sizeof(sApplyYmd)); Allat_trim(sApplyYmd);
		getValue("order_no=", sMsg, sOrderNo, sizeof(sOrderNo)); Allat_trim(sOrderNo);
		
		/**************  결과 값 출력 ******************/
		xprintf("[CH:%03d] 주문번호        : %s", ch, sOrderNo);
		xprintf("[CH:%03d] 승인금액        : %s", ch, sAmt);
		xprintf("[CH:%03d] 지불수단        : %s", ch, sPayType);
		xprintf("[CH:%03d] 인증키           : %s", ch, sFixKey);
		xprintf("[CH:%03d] 인증일시        : %s", ch, sApplyYmd);
		xprintf("[CH:%03d] 거래일련번호    : %s", ch, sSeqNo);
		xprintf("[CH:%03d] 상품코드    : %s", ch, szProductCd);
	}
	else{
		// reply_cd 가 "0000" 아닐때는 에러 (자세한 내용은 매뉴얼참조)
		// reply_msg 는 실패에 대한 메세지        
		memcpy(pScenario->m_CardResInfo.m_szMoid, pScenario->m_szMx_issue_no, sizeof(pScenario->m_CardResInfo.m_szMoid) - 1);        // 주문번호
		memset(sTmp, 0x00, sizeof(sTmp));
		sprintf_s(sTmp, sizeof(sTmp), "%d", pScenario->m_nAmount);
		memcpy(pScenario->m_CardResInfo.m_AMT, sTmp, sizeof(pScenario->m_CardResInfo.m_AMT) - 1);      // 거래 금액
		memcpy(pScenario->m_CardResInfo.m_szCardQuota, pScenario->m_CardInfo.InstPeriod, sizeof(pScenario->m_CardResInfo.m_szCardQuota) - 1);  // 할부개월수
		memcpy(pScenario->m_CardResInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_CardResInfo.m_szMid) - 1);//상번ID

		xprintf("[CH:%03d] 주문번호        : %s", ch, pScenario->m_CardResInfo.m_szMoid);
		xprintf("[CH:%03d] 결제금액      : %s", ch, pScenario->m_CardResInfo.m_AMT);
		xprintf("[CH:%03d] 할부개월수      : %s", ch, pScenario->m_CardResInfo.m_szCardQuota);
		xprintf("[CH:%03d] 가맹점ID      : %s", ch, pScenario->m_CardResInfo.m_szMid);
		xprintf("[CH:%03d] 전송일시      :%s", ch, pScenario->m_CardResInfo.MX_ISSUE_DATE);
	}

	pScenario->m_PaySysCd = 1;
	(*port)[ch].ppftbl[POST_NET].postcode = HI_OK;
	PAYquithostio("the line service is Allat AllatArsGetFixKeyProcess.", ch);
	xprintf("[CH:%03d] Allat AllatArsGetFixKeyProcess end", ch);
	xprintf("[CH:%03d] ALLAT_WOWTV_Billkey_Easy_Scenario  AllatArsGetFixKeyProcess END ", ch);
	_endthreadex((unsigned int)pScenario->m_hPayThread);
	return 0;
}

int  AllatPaymemt_host(int holdm)
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
	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread)
	{
		WaitForSingleObject(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread, INFINITE);
		CloseHandle(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread);
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread = NULL;
	}

	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.
	
	memset(pScenario->m_CardInfo.Fix_Key, 0x00, sizeof(pScenario->m_CardInfo.Fix_Key));
	memset(pScenario->m_szbill_key, 0x00, sizeof(pScenario->m_szbill_key));
	
	pScenario->m_hPayThread = (HANDLE)_beginthreadex(NULL, 0, AllatArsPayProcess, (LPVOID)(*lpmt), 0, &(pScenario->paythreadID));
	return(0);
}

int  AllatPayCancle_host(int holdm)
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
	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread)
	{
		WaitForSingleObject(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread, INFINITE);
		CloseHandle(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread);
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread = NULL;
	}
	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.

	pScenario->m_hPayThread = (HANDLE)_beginthreadex(NULL, 0, AllatArsPayCancleProcess, (LPVOID)(*lpmt), 0, &(pScenario->paythreadID));

	return(0);
}

int  Allat_Get_FixKey_host(int holdm)
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
	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread)
	{
		WaitForSingleObject(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread, INFINITE);
		CloseHandle(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread);
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread = NULL;
	}

	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.
	pScenario->m_hPayThread = (HANDLE)_beginthreadex(NULL, 0, AllatArsGetFixKeyProcess, (LPVOID)(*lpmt), 0, &(pScenario->paythreadID));
	return(0);
}

int  AllatFixPaymemt_host(int holdm)
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
	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread)
	{
		WaitForSingleObject(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread, INFINITE);
		CloseHandle(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread);
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread = NULL;
	}

	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.
	// 빌키가 있으면
	if (strlen(pScenario->m_szbill_key))
	{
		memset(pScenario->m_CardInfo.Fix_Key, 0x00, sizeof(pScenario->m_CardInfo.Fix_Key));
		strncpy_s(pScenario->m_CardInfo.Fix_Key, sizeof(pScenario->m_CardInfo.Fix_Key), pScenario->m_szbill_key, sizeof(pScenario->m_CardInfo.Fix_Key) - 1);
	}

	pScenario->m_hPayThread = (HANDLE)_beginthreadex(NULL, 0, AllatArsPayProcess, (LPVOID)(*lpmt), 0, &(pScenario->paythreadID));
	return(0);
}

int  AllatPayFixKeyCancle_host(int holdm)
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
	if (((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread)
	{
		WaitForSingleObject(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread, INFINITE);
		CloseHandle(((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread);
		((CALLAT_WOWTV_Billkey_Easy_Scenario *)((*lpmt)->pScenario))->m_hPayThread = NULL;
	}
	(*lpmt)->ppftbl[POST_NET].postcode = HI_NCMPLT;// 종료하지 않았다.
	// 빌키가 있으면
	if (strlen(pScenario->m_szbill_key))
	{
		memset(pScenario->m_Card_CancleInfo.m_szMid, 0x00, sizeof(pScenario->m_Card_CancleInfo.m_szMid));
		memcpy(pScenario->m_Card_CancleInfo.m_szMid, pScenario->m_szMx_id, sizeof(pScenario->m_Card_CancleInfo.m_szMid) - 1); 
		memset(pScenario->m_Card_CancleInfo.Fix_Key, 0x00, sizeof(pScenario->m_Card_CancleInfo.Fix_Key));
		strncpy_s(pScenario->m_Card_CancleInfo.Fix_Key, sizeof(pScenario->m_Card_CancleInfo.Fix_Key), pScenario->m_szbill_key, sizeof(pScenario->m_Card_CancleInfo.Fix_Key) - 1);
	}
	pScenario->m_hPayThread = (HANDLE)_beginthreadex(NULL, 0, AllatArsPayFixKeyCancleProcess, (LPVOID)(*lpmt), 0, &(pScenario->paythreadID));

	return(0);
}
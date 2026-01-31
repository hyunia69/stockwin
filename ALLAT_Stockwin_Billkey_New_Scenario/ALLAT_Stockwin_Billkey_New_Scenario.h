// 다음 ifdef 블록은 DLL에서 내보내기하는 작업을 쉽게 해 주는 매크로를 만드는 
// 표준 방식입니다. 이 DLL에 들어 있는 파일은 모두 명령줄에 정의된 _EXPORTS 기호로
// 컴파일되며, 다른 프로젝트에서는 이 기호를 정의할 수 없습니다.
// 이렇게 하면 소스 파일에 이 파일이 들어 있는 다른 모든 프로젝트에서는 
// NICE_SCENARIO_API 함수를 DLL에서 가져오는 것으로 보고, 이 DLL은
// 이 DLL은 해당 매크로로 정의된 기호가 내보내지는 것으로 봅니다.
#pragma once
#ifdef WIN32

#include <openssl/ssl.h>

#else
#define STDMETHODCALLTYPE
#endif

// 이 클래스는 Nice_Scenario.dll에서 내보낸 것입니다.
class CALLAT_WOWTV_Billkey_Easy_Scenario : public IScenario
{
public:
	int  STDMETHODCALLTYPE ScenarioInit(LPMTP *Port, char *ArsType);
	int  STDMETHODCALLTYPE jobArs(/* [in] */int state);
	int  DisConnectProcess();

	// 데이터 베이스 접속 성공 여부
	int m_DBAccess;
	int m_bDnisInfo;//주문 내역 존재 여부

	CADODB *m_AdoDb;

	char  m_szpaymethod[10 + 1];
	char  m_szInputTel[127 + 1];      // 고객이 입력한 전화 번호
	char  m_szAuth_no[10 + 1];       // 고객이 입력한 주문 당시의 인증 번호
	char  m_szCC_name[64 + 1];       // 고객명
	char  m_szCC_Prod_Desc[255 + 1]; // 상품명
	char  m_szpsrtner_nm[256 + 1]; // 파트너명

	//2016.09.12
	//김상기 한국경제 TV&안현 다삼솔루션 이사&손삼민 한국경제TV 파트장 등의 요청으로 추가
	char  m_szCC_Prod_Code[11 + 1]; // 상품코드
	char  m_szMx_issue_no[80 + 1];   // 주문번호 32=>80 변경 (2016.09.07)

	char  m_szMx_id[32 + 1];         // 가맹점 아이디  (최종 선택 결제용)

	char  m_szMx_id1[32 + 1];         // 가맹점 아이디  (간편결제용)
	char  m_szMx_id2[32 + 1];         // 가맹점 아이디  (일반결제용)

	int   m_nAmount;                 // 결제금액
	char  m_szMx_name[50+1];         // 가맹점 명
	char  m_szInstallment[4+1];      // 할부 개월 수

	char  m_szMx_opt[255 + 1];       // 최종 선택 가맹점 접근키

	char  m_szMx_opt1[255 + 1];       // 간편결제용가맹점 접근키
	char  m_szMx_opt2[255 + 1];       // 일반결제용가맹점 접근키
	char  m_szPhone_no[13 + 1];      // 데이터 베이스 상의 전화 번호

	char  m_sz_Shop_Pw[10 + 1];      // 상점 별 취소 시 비번
	char  m_szCC_email[200 + 1];     // 무이자를 emaill 필드를 이용해서 추가 20141115

	char  m_szAUTH_TYPE[10 + 1];      // 최종 선택 타입
	
	char  m_szAUTH_TYPE1[10 + 1];      // 간편결제 타입
	char  m_szAUTH_TYPE2[200 + 1];     // 일반결제 타입

	int   m_nServiceAmt;             // 면세 추가 20150312
	int   m_nSupplyAmt;              // 면세 추가 20150312
	int   m_nGoodsVat;               // 면세 추가 20150312
	int   m_nTaxFreeAmt;             // 면세 추가 20150312

	char  m_szURL_YN[1 + 1];
	char  m_szSHOP_RET_URL[2048 + 1];// 최종 
	char  m_szSHOP_RET_URL_AG[2048 + 1024 + 1];

	char  m_szSHOP_RET_URL1[2048 + 1];//간편결제용	
	char  m_szSHOP_RET_URL2[2048 + 1];//일반결제용

	char m_szHttpBuffer[4096 + 1];
	CALLAT_WOWTV_Billkey_Easy_Scenario(void);
	~CALLAT_WOWTV_Billkey_Easy_Scenario(void);

	CARDINFO         m_CardInfo;
	Card_ResInfo     m_CardResInfo;
	
	Card_CancleInfo  m_Card_CancleInfo;
	Card_ResInfo     m_Cancel_ResInfo;

//	CNicePay         m_NicePay;
	int              m_PaySysCd;

	int              m_PayResult;

	int              InputErrCnt;

	HINTERNET		m_hSession;
	HINTERNET		m_hConnect;
	HINTERNET       m_hObject;
	int             m_RetCode;


	int m_nRetryMaxCnt;
	int m_nRetryCnt;

	SSL *m_ssl;
	char m_szretMsg[1024 + 1];
	char m_szretval[14 + 1];

	char	m_szbill_key[256 + 1];   // 빌키	        O
	char	m_szext_data[256 + 1];    // 간편결제 인증용 생년월일	        O
	char	m_szrenew_flag[1 + 1];  // 동의ARS 여부를 위해 상품보유 여부 값	        O
	char	m_szcardName[20 + 1 ];  // cardName
	BOOL    m_bPayFlag;

// AHN 2022.0808 정기결제 수정 
	char	m_szsub_status[8 + 1];   // 정기결제 상태 (0: 첫 결제, 1: 이용 중, 2: 해지)
	int		m_nsub_amount;   // 정기결제 금액
	char	m_szsub_has_trial[8 + 1];   // 체험 상품 유무 (Y: 체험상품 있는 상품, N: 체험상품 없는 상품)
	char	m_szexpire_date[32 + 1];   // 상품 만료 일자 (yyyyMMdd 형식)

// ========================================
// REST API 지원 멤버 변수 (2026.02.01 추가)
// ========================================
volatile LONG m_bNeedRollback;         // 롤백 필요 플래그 (InterlockedExchange용)
LONG m_bPaymentApproved;               // 결제 승인 상태
BOOL m_bDisconnectProcessed;           // DisConnectProcess 중복 호출 방지

int m_nPurchaseAmt;                    // 원가 (할인 전)
char m_szCouponUseFlag[4];             // 쿠폰 사용 여부 (Y/N)
char m_szCouponName[64];               // 쿠폰명
char m_szBonusCashUseFlag[4];          // 보너스 캐시 사용 여부 (Y/N)
int m_nBonusCashUseAmt;                // 보너스 캐시 사용 금액
char m_szMemberId[48];                 // 회원 UUID
char m_szCategoryId[20];               // 상품 카테고리 ID
char m_szPurchaseLimitFlag[4];         // 구매 제한 플래그
char m_szServiceCheckFlag[4];          // 서비스 점검 플래그
char m_szPgCode[8];                    // PG 코드 (A=올앳, P=페이레터)
int m_nServiceDivDay;                  // 남은 기간 (일) - C, D 케이스용
char m_szTrialType[16];                // 무료체험 유형 (NOFREE, 1WEEK, 2WEEK, 1MONTH)

private:


};

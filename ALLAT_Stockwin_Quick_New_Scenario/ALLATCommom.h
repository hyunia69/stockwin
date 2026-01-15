

// 카드 번호
typedef struct CarsInfo
{
	char Card_Num[16 + 1]; // 카드 번호
	char ExpireDt[4 + 1];  // 유효 기간 (년월)
	char SecretNo[10 + 1]; // 주민번호,법인번호
	char Password[2 + 1];  // 비밀 번호
	char InstPeriod[2 + 1];// 할부 개월수

} CARDINFO;

// 카드 승인 정보
typedef struct Card_ResInfo
{
	char MX_ISSUE_DATE[14 + 1];  // 거래 일시
	char m_Tid[50 + 1];	         // 거래번호
	char m_szMid[32 + 1];        // 가맹점 아이디  
	char m_szMoid[80 + 1];       // 주문번호 32 = >80 변경(2016.09.07)
	char m_szRESULTCODE[10 + 1]; // 결과코드
	char m_szRESULTMSG[200 + 1]; // 결과메시지
	char m_szGOODSNAME[40 + 1];  // 상품명
	char m_PAYMETHOD[10 + 1];    // 결제수단
	char m_AUTHDATE[12 + 1];     // 승인일시YYMMDDHH24mmss
	char m_AUTHCODE[10 + 1];     // 승인번호
	char m_AMT[12 + 1];          // 결제금액
	char m_szCardNo[18 + 1];     // 카드번호
	char m_szCardQuota[2 + 1];   // 할부개월
	char m_CARDCODE[2 + 1];      // 신용카드사 코드
	char m_CARDNAME[20 + 1];     // 신용카드사 명
	char m_AcquCardCode[2 + 1];  // 매입카드사 코드
	char m_AcquCardName[20 + 1]; // 매입카드사 명

	char m_PartialCancelCode[2 + 1]; // 부분 취소
	char m_sZerofeeYn[1 + 1];        // 무이자 할부여부
	char m_sCertYn[1 + 1];           // 인증 여부     
	char m_sContractYn[1 + 1];       // 직 가맹여부

	char sRemainAmt[10 + 1];// 취소 후 잔액
}Card_ResInfo;


//  결제 취소 승인 정보
typedef struct Card_CancleInfo
{
	char m_Tid[50 + 1];	         // 거래번호
	char m_szMid[32 + 1];        // 가맹점ID
	char m_szMoid[80 + 1];       // 주문번호 32=>80 변경 (2016.09.07)
	char m_szGOODSNAME[40 + 1];  // 상품명
	char m_AMT[12 + 1];          // 결제금액(취소 금액)

	char m_szCancelPwd[10 + 1];      // 취소시 가맹점별 비번
	char m_PartialCancelCode[2 + 1]; // 부분 취소
}Card_CancleInfo;
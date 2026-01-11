#pragma once


#pragma pack(push, 1)// 바이트 정렬을 1바이트씩 하도록 지정하되 아래 구조체에만 적용
// 1. 네트워크의 프로그램의 경우, 서로 다른 운용체간 및 기기간 메모리 패킹시  정렬 규칙이 다르기에 사용
// 2. 각 시스템 별 그 규격대로 안다고 해도, 서버 시스템에 맞추는 일을 상당한 CPU 부담, 
// 3. 서로 동일 한다고 하면, CPU 부담 줄어 드나, 트랙픽 증가의 요인이 더 많은 오버헤드 야기 

typedef struct Socket_HEADER
{
	//형식	필드명	    길이	    설명	            필수항목	비고
	char	m_szTYPE[6 + 1];     //	전문의 종류	        O	        REQ:  송신 요청 전문, RES:  수신 응답 전문
	char	m_szBODY_LEN[5 + 1]; //	바디 전문의 길이	O
} SOCKETHEADER;

typedef struct Socket_BODY_HEADER
{
	//형식	필드명	    길이	    설명	            필수항목	비고
	char	m_szCMD[15 + 1]; //	BODY 전문의 종류	    O	        PROD_INFO : 상품정보 획득
} SOCKETBODYHEADER;


typedef struct Info_PRODOC_REQ
{
	//[BODY > PROD_INFO 획득  요청 전문]
	//형식	필드명	길이	        설명	                필수항목	비고
	char	m_szDNIS[4 + 1];   //   회선번호	            O
	char	m_szHP_NO[12 + 1]; //	고객의 휴대전화 번호	O
} INFOPRODOCREQ;

typedef struct Info_PRODOC_RES
{
	//[BODY > PROD_INFO 획득  요청 전문]
	//형식	필드명	길이	             설명	            필수항목	비고
	char	m_szorder_no[70 + 1];     // 가맹점 거래번호	O	        가맹점에서 생성 - 거래고유번호
	char	m_szshop_id1[32 + 1];      // 가맹점 아이디    	O	        가맹점에게 부여된 아이디: 간편결제용
	char	m_szshop_id2[32 + 1];      // 가맹점 아이디    	O	        가맹점에게 부여된 아이디: 일반결제용
	char	m_szcc_name[64 + 1];      // 고객명	            x
	char	m_szcc_pord_desc[255 + 1];// 상품명^상품코드	O
	char	m_szpartner_nm[256 + 1];//  파트너명	O
	char	m_szcc_pord_code[50 + 1]; // 상품 코드입니다.   O
	char	m_szamount[10 + 1];       // 결제금액	        O
	char	m_szbill_key[256 + 1];   // 빌키	        O
	char	m_szext_data[256 + 1];    // 간편결제 인증용 생년월일	        O
	char	m_szrenew_flag[1 + 1];  // 동의ARS 여부를 위해 상품보유 여부 값	        O

	char	m_szcardName[20 + 1 + 1];  // cardName
} INFOPRODOCRES;



#pragma pack(pop)     //pack 복원


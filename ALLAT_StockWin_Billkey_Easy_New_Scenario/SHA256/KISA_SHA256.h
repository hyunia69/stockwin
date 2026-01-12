/**
@file KISA_SHA_256.h
@brief KISA_SHA256 암호 알고리즘
@author Copyright (c) 2013 by KISA
@remarks http://seed.kisa.or.kr/
*/

#ifndef KISA_SHA256_H
#define KISA_SHA256_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef OUT
#define OUT
#endif

#ifndef IN
#define IN
#endif

#ifndef INOUT
#define INOUT
#endif

#undef BIG_ENDIAN
#undef LITTLE_ENDIAN

#if defined(USER_BIG_ENDIAN)
	#define BIG_ENDIAN
#elif defined(USER_LITTLE_ENDIAN)
	#define LITTLE_ENDIAN
#else
	#if 0
		#define BIG_ENDIAN
	#elif defined(_MSC_VER)
		#define LITTLE_ENDIAN
	#else
		#error
	#endif
#endif
	
#define KISA_SHA256_DIGEST_BLOCKLEN	64
#define KISA_SHA256_DIGEST_VALUELEN	32

typedef struct{
	UINT uChainVar[KISA_SHA256_DIGEST_VALUELEN / 4];
	UINT uHighLength;
	UINT uLowLength;
	BYTE szBuffer[KISA_SHA256_DIGEST_BLOCKLEN];
} KISA_SHA256_INFO;

/**`
@brief 연쇄변수와 길이변수를 초기화하는 함수
@param Info : KISA_SHA256_Process 호출 시 사용되는 구조체
*/
void KISA_SHA256_Init(OUT KISA_SHA256_INFO *Info);

/**
@brief 연쇄변수와 길이변수를 초기화하는 함수
@param Info : KISA_SHA256_Init 호출하여 초기화된 구조체(내부적으로 사용된다.)
@param pszMessage : 사용자 입력 평문
@param inLen : 사용자 입력 평문 길이
*/
void KISA_SHA256_Process(OUT KISA_SHA256_INFO *Info, IN const BYTE *pszMessage, IN UINT uDataLen);

/**
@brief 메시지 덧붙이기와 길이 덧붙이기를 수행한 후 마지막 메시지 블록을 가지고 압축함수를 호출하는 함수
@param Info : KISA_SHA256_Init 호출하여 초기화된 구조체(내부적으로 사용된다.)
@param pszDigest : 암호문
*/
void KISA_SHA256_Close(OUT KISA_SHA256_INFO *Info, OUT BYTE *pszDigest);

/**
@brief 사용자 입력 평문을 한번에 처리
@param pszMessage : 사용자 입력 평문
@param pszDigest : 암호문
@remarks 내부적으로 KISA_SHA256_Init, KISA_SHA256_Process, KISA_SHA256_Close를 호출한다.
*/
void KISA_SHA256_Encrpyt( IN const BYTE *pszMessage, IN UINT uPlainTextLen, OUT BYTE *pszDigest );
void KISA_SHA256_HexCode(char *pSource, char *pOut);

#ifdef  __cplusplus
}
#endif

#endif
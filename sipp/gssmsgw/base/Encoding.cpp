#ifndef _BASE_ENCODING_H
#define _BASE_ENCODING_H

#pragma once
#include<string>
#include "baseprotocol.h"
using namespace std;
int DecodeUcs2(const unsigned char   *pSrc,   char   *pDst,   int   nSrcLength,  int   nDstMaxLength)   
{   
	int   nDstLength;
	wchar_t*   wbuf = new wchar_t[nSrcLength/2];

	for(int   i=0;   i<nSrcLength/2;   i++)   
	{   
		wbuf[i]   =  (*pSrc++  <<   8);    
		wbuf[i]   |=   *pSrc++;
	}   

	nDstLength   =   ::WideCharToMultiByte(CP_ACP,   0,   wbuf,   nSrcLength/2,   pDst,   (nDstMaxLength - 1),   NULL,   NULL);   
	pDst[nDstLength]   =   '\0';

	delete[] wbuf;

	return   nDstLength;   
}

int EncodeUcs2(const   char   *pSrc,   unsigned   char   *pDst,   int   nSrcLength,  int   nDstMaxLength)   
{   
	int   nDstLength;
	wchar_t*   wbuf = new wchar_t[nDstMaxLength];

	nDstLength   =   ::MultiByteToWideChar(CP_ACP,   0,   pSrc,   nSrcLength,   wbuf,   nDstMaxLength);   

	for(int   i=0;   i<nDstLength;   i++)   
	{   
		*pDst++   =   wbuf[i]   >>   8;   
		*pDst++   =   wbuf[i]   &   0xff;   
	}   

	delete[] wbuf;

	return   nDstLength   *   2;   
} 

int DecodeUcs2(const string&  src, string& dst)
{
	if(src.length() <= 0)
	{
		return 0;
	}
	int bufLen = src.length()*2 + 2;
	char* acTemp = new char[bufLen];
	memset(acTemp, 0, bufLen);

	int len = DecodeUcs2((const unsigned char*)&src[0], acTemp, src.length(), bufLen - 2);
	if(len > 0)
	{
		dst.assign(acTemp, len);
	}

	delete[] acTemp;
	return len;
}

int EncodeUcs2(const string&  src, string& dst)
{
	if(src.length() <= 0)
	{
		return 0;
	}
	int bufLen = src.length()*2 + 2;
	unsigned char* acTemp = new unsigned char[bufLen];
	memset(acTemp, 0, bufLen);

	int len = EncodeUcs2(&src[0], acTemp, src.length(), bufLen-2);
	if(len > 0)
	{
		dst.assign((char *)acTemp, len);
	}

	delete[] acTemp;
	return len;
}

int split_gbk_string(string& src, string& dst, int begin, int len)
{
	if( begin >= src.length() || len <= 0 || begin < 0)
	{
		return 0;
	}

	if( src.length() < (begin+len) )
	{
		dst = src.substr(begin, (src.length() - begin));
		return dst.length();
	}

	int count = len;
	int i = 0;
	int mark = 0;
	for(i = (begin + len - 1); i >= 0; i--)
	{
		if(src[i] > 0 && src[i] < 127)
		{
			break;
		}
		else
		{
			mark ++;
		}
	}

	if(mark != 0 && (mark % 2) != 0)
	{
		count = len - 1;
	}

	dst = src.substr(begin, count);

	return count;
}


#endif
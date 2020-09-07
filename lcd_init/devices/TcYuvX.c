/*=///////////////////////////////////////////////////////////////////////=*/
/*= TcYuvX.c : Defines entry point act-code for YUV查表法 application.    =*/
/*= Made By ShanChengKun.PentiumWorkingRoom   Based Made in 2014.12.08    =*/
/*= CopyRight(C) 1996-2018 . Email: sck007@163.com . Update on 2015.09.19 =*/
/*=///////////////////////////////////////////////////////////////////////=*/

//#include "stdafx.h"
//#include <stdio.h>
//#include <tchar.h>
#include "TcYuvX.h"
#include <math.h>

/*=///////////////////////////////////////////////////////////////////////=*/

/* R = Y + (V - 128) * 1.402 */
static const short YuV_R_V[256] = \
{
	-179,-178,-176,-175,-173,-172,-171,-169,-168,-166,-165,-164,-162,-161,-159,-158,
	-157,-155,-154,-152,-151,-150,-148,-147,-145,-144,-143,-141,-140,-138,-137,-135,
	-134,-133,-131,-130,-128,-127,-126,-124,-123,-121,-120,-119,-117,-116,-114,-113,
	-112,-110,-109,-107,-106,-105,-103,-102,-100,-99,-98,-96,-95,-93,-92,-91,
	-89,-88,-86,-85,-84,-82,-81,-79,-78,-77,-75,-74,-72,-71,-70,-68,
	-67,-65,-64,-63,-61,-60,-58,-57,-56,-54,-53,-51,-50,-49,-47,-46,
	-44,-43,-42,-40,-39,-37,-36,-35,-33,-32,-30,-29,-28,-26,-25,-23,
	-22,-21,-19,-18,-16,-15,-14,-12,-11,-9,-8,-7,-5,-4,-2,-1,
	0,1,2,4,5,7,8,9,11,12,14,15,16,18,19,21,
	22,23,25,26,28,29,30,32,33,35,36,37,39,40,42,43,
	44,46,47,49,50,51,53,54,56,57,58,60,61,63,64,65,
	67,68,70,71,72,74,75,77,78,79,81,82,84,85,86,88,
	89,91,92,93,95,96,98,99,100,102,103,105,106,107,109,110,
	112,113,114,116,117,119,120,121,123,124,126,127,128,130,131,133,
	134,135,137,138,140,141,143,144,145,147,148,150,151,152,154,155,
	157,158,159,161,162,164,165,166,168,169,171,172,173,175,176,178
};

/*===============================================================*/

/* G = Y - (U - 128) * 0.344 - (V - 128) * 0.714 */
static const short YuV_G_U[256] = \
{
	-44,-43,-43,-43,-42,-42,-41,-41,-41,-40,-40,-40,-39,-39,-39,-38,
	-38,-38,-37,-37,-37,-36,-36,-36,-35,-35,-35,-34,-34,-34,-33,-33,
	-33,-32,-32,-31,-31,-31,-30,-30,-30,-29,-29,-29,-28,-28,-28,-27,
	-27,-27,-26,-26,-26,-25,-25,-25,-24,-24,-24,-23,-23,-23,-22,-22,
	-22,-21,-21,-20,-20,-20,-19,-19,-19,-18,-18,-18,-17,-17,-17,-16,
	-16,-16,-15,-15,-15,-14,-14,-14,-13,-13,-13,-12,-12,-12,-11,-11,
	-11,-10,-10,-9,-9,-9,-8,-8,-8,-7,-7,-7,-6,-6,-6,-5,
	-5,-5,-4,-4,-4,-3,-3,-3,-2,-2,-2,-1,-1,-1,0,0,
	0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,
	5,5,6,6,6,7,7,7,8,8,8,9,9,9,10,10,
	11,11,11,12,12,12,13,13,13,14,14,14,15,15,15,16,
	16,16,17,17,17,18,18,18,19,19,19,20,20,20,21,21,
	22,22,22,23,23,23,24,24,24,25,25,25,26,26,26,27,
	27,27,28,28,28,29,29,29,30,30,30,31,31,31,32,32,
	33,33,33,34,34,34,35,35,35,36,36,36,37,37,37,38,
	38,38,39,39,39,40,40,40,41,41,41,42,42,43,43,43
};

/*===============================================================*/

/* G = Y - (U - 128) * 0.344 - (V - 128) * 0.714 */
static const short YuV_G_V[256] = \
{
	-91,-90,-89,-89,-88,-87,-87,-86,-85,-84,-84,-83,-82,-82,-81,-80,
	-79,-79,-78,-77,-77,-76,-75,-74,-74,-73,-72,-72,-71,-70,-69,-69,
	-68,-67,-67,-66,-65,-64,-64,-63,-62,-62,-61,-60,-59,-59,-58,-57,
	-57,-56,-55,-54,-54,-53,-52,-52,-51,-50,-49,-49,-48,-47,-47,-46,
	-45,-44,-44,-43,-42,-42,-41,-40,-39,-39,-38,-37,-37,-36,-35,-34,
	-34,-33,-32,-32,-31,-30,-29,-29,-28,-27,-27,-26,-25,-24,-24,-23,
	-22,-22,-21,-20,-19,-19,-18,-17,-17,-16,-15,-14,-14,-13,-12,-12,
	-11,-10,-9,-9,-8,-7,-7,-6,-5,-4,-4,-3,-2,-2,-1,0,
	0,0,1,2,2,3,4,4,5,6,7,7,8,9,9,10,
	11,12,12,13,14,14,15,16,17,17,18,19,19,20,21,22,
	22,23,24,24,25,26,27,27,28,29,29,30,31,32,32,33,
	34,34,35,36,37,37,38,39,39,40,41,42,42,43,44,44,
	45,46,47,47,48,49,49,50,51,52,52,53,54,54,55,56,
	57,57,58,59,59,60,61,62,62,63,64,64,65,66,67,67,
	68,69,69,70,71,72,72,73,74,74,75,76,77,77,78,79,
	79,80,81,82,82,83,84,84,85,86,87,87,88,89,89,90
};

/*===============================================================*/

/* B = Y + (U - 128) * 1.772 */
static const short YuV_B_U[256] = \
{
	-226,-225,-223,-221,-219,-217,-216,-214,-212,-210,-209,-207,-205,-203,-202,-200,
	-198,-196,-194,-193,-191,-189,-187,-186,-184,-182,-180,-178,-177,-175,-173,-171,
	-170,-168,-166,-164,-163,-161,-159,-157,-155,-154,-152,-150,-148,-147,-145,-143,
	-141,-139,-138,-136,-134,-132,-131,-129,-127,-125,-124,-122,-120,-118,-116,-115,
	-113,-111,-109,-108,-106,-104,-102,-101,-99,-97,-95,-93,-92,-90,-88,-86,
	-85,-83,-81,-79,-77,-76,-74,-72,-70,-69,-67,-65,-63,-62,-60,-58,
	-56,-54,-53,-51,-49,-47,-46,-44,-42,-40,-38,-37,-35,-33,-31,-30,
	-28,-26,-24,-23,-21,-19,-17,-15,-14,-12,-10,-8,-7,-5,-3,-1,
	0,1,3,5,7,8,10,12,14,15,17,19,21,23,24,26,
	28,30,31,33,35,37,38,40,42,44,46,47,49,51,53,54,
	56,58,60,62,63,65,67,69,70,72,74,76,77,79,81,83,
	85,86,88,90,92,93,95,97,99,101,102,104,106,108,109,111,
	113,115,116,118,120,122,124,125,127,129,131,132,134,136,138,139,
	141,143,145,147,148,150,152,154,155,157,159,161,163,164,166,168,
	170,171,173,175,177,178,180,182,184,186,187,189,191,193,194,196,
	198,200,202,203,205,207,209,210,212,214,216,217,219,221,223,225
};

/*===============================================================*/

/* RGB范围[-226, 480]内规整到[0, 255]查[707]表 */
static const BYTE _YuV_CvP_[226+480+1/*=707*/] = \
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,
	14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,
	30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,
	46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,
	62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,
	78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,
	94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,
	110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
	126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,
	142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,
	158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,
	174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,
	190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,
	206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,
	222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,
	238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,
	254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255
};
static const BYTE *YuV_C_P = &_YuV_CvP_[226];

/*===============================================================*/

/* COLOUR SPACES .17 ITU-R recommendation BT.601 标准 */
/* 确保整数在[0, 255]之间(8-Bit字节)，输出R-G-B计算   */
/* R = Y + (V - 128) * 1.402                          */
/* G = Y - (U - 128) * 0.344 - (V - 128) * 0.714      */
/* B = Y + (U - 128) * 1.772                          */
#define YUV_TO_RGB(vRgb, tY, tU, tV) \
{ \
	*(vRgb)++ = YuV_C_P[tY + YuV_R_V[tV]]; \
	*(vRgb)++ = YuV_C_P[tY - YuV_G_U[tU] - YuV_G_V[tV]]; \
	*(vRgb)++ = YuV_C_P[tY + YuV_B_U[tU]]; \
}

/*=///////////////////////////////////////////////////////////////////////=*/

/* DShow的4:2:2的首选格式(YUYV)：打包-YUY2，大小(nWd * nHi * 2) */
void RgbFromPackYUY2(BYTE *pRgb, const BYTE *pYuv, int nWd, int nHi)
{
	const BYTE *pY = pYuv, *pU = pYuv + 1, *pV = pYuv + 3;
	const BYTE *pEnd = pYuv + nWd * nHi * 2;
	
	while(pY < pEnd) /* 整幅图像逐点YUV->RGB转换 */
	{
		YUV_TO_RGB(pRgb, *pY, *pU, *pV); pY += 2;
		YUV_TO_RGB(pRgb, *pY, *pU, *pV); pY += 2;
		pU += 4; pV += 4; /* 4个字节出2个像素 */
	}
}
#if 1
/*=///////////////////////////////////////////////////////////////////////=*/

/* Yuv420SP(Y平面UV打包)：0=DShow的NV12，1=安卓NV21，大小(nWd * nHi * 3 / 2) */
void RgbFromYuv420SP(BYTE *pRgb, const BYTE *pYuv, int nWd, int nHi, int nFor)
{
	int i, j, nStp = nWd * 3;
	BYTE *pD1, *pD2, u, v;
	const BYTE *pS1, *pS2, *pU, *pV;
	
	if(nFor == 0) /* DShow默认420SP-NV12 */
	{
		pU = pYuv + nWd * nHi;
		pV = pU + 1;
	}
	else /* 安卓默认420SP-NV21 */
	{
		pV = pYuv + nWd * nHi;
		pU = pV + 1;
	}
	
	for(j=0; j<nHi; j+=2) /* 2x2的Y，使用同一组UV */
	{
		pD1 = pRgb; pD2 = pRgb + nStp;
		pS1 = pYuv; pS2 = pYuv + nWd;
		
		for(i=0; i<nWd; i+=2)
		{
			u = *pU; pU += 2; /* UV间隔顺序存放 */
			v = *pV; pV += 2;
			
			YUV_TO_RGB(pD1, *pS1, u, v); pS1++;
			YUV_TO_RGB(pD1, *pS1, u, v); pS1++;
			YUV_TO_RGB(pD2, *pS2, u, v); pS2++;
			YUV_TO_RGB(pD2, *pS2, u, v); pS2++;
		}
		
		pRgb += nStp + nStp;
		pYuv += nWd + nWd;
	}
}

/*=///////////////////////////////////////////////////////////////////////=*/

/* Yuv420P(YUV均为平面)：0=标准的I420，1=H264的YV12，大小(nWd * nHi * 3 / 2) */
void RgbFromYuv420P(BYTE *pRgb, const BYTE *pYuv, int nWd, int nHi, int nFor)
{
	int i, j, nStp = nWd * 3, nSz = nWd * nHi;
	BYTE *pD1, *pD2, u, v;
	const BYTE *pS1, *pS2, *pU, *pV;
	
	if(nFor == 0) /* 标准420P的YU12-I420 */
	{
		pU = pYuv + nSz;
		pV = pU + (nSz / 4);
	}
	else /* H264解码器420P-YV12 */
	{
		pV = pYuv + nSz;
		pU = pV + (nSz / 4);
	}
	
	for(j=0; j<nHi; j+=2) /* 2x2的Y，使用同一组UV */
	{
		pD1 = pRgb; pD2 = pRgb + nStp;
		pS1 = pYuv; pS2 = pYuv + nWd;
		
		for(i=0; i<nWd; i+=2)
		{
			u = *pU++; v = *pV++; /* U/V平面顺序存放 */
			
			YUV_TO_RGB(pD1, *pS1, u, v); pS1++;
			YUV_TO_RGB(pD1, *pS1, u, v); pS1++;
			YUV_TO_RGB(pD2, *pS2, u, v); pS2++;
			YUV_TO_RGB(pD2, *pS2, u, v); pS2++;
		}
		
		pRgb += nStp + nStp;
		pYuv += nWd + nWd;
	}
}

/*=///////////////////////////////////////////////////////////////////////=*/

/* RAW边缘的2x2小块4点，变换成RGB */
#define BrdRawToRgb(qD1, qD2, qS1, qS2) \
{ \
	switch(nWho) \
	{ \
	case 0:  /* B G */ \
		     /* G R */ \
		qD1[0] = qS2[1]; \
		qD1[1] = ((qS1[1] + qS2[0]) >> 1); \
		qD1[2] = qS1[0]; \
		\
		qD1[3] = qS2[1]; \
		qD1[4] = qS1[1]; \
		qD1[5] = qS1[0]; \
		\
		qD2[0] = qS2[1]; \
		qD2[1] = qS2[0]; \
		qD2[2] = qS1[0]; \
		\
		qD2[3] = qS2[1]; \
		qD2[4] = ((qS1[1] + qS2[0]) >> 1); \
		qD2[5] = qS1[0]; \
		break; \
	\
	case 1:  /* R G */ \
		     /* G B */ \
		qD1[0] = qS1[0]; \
		qD1[1] = ((qS1[1] + qS2[0]) >> 1); \
		qD1[2] = qS2[1]; \
		\
		qD1[3] = qS1[0]; \
		qD1[4] = qS1[1]; \
		qD1[5] = qS2[1]; \
		\
		qD2[0] = qS1[0]; \
		qD2[1] = qS2[0]; \
		qD2[2] = qS2[1]; \
		\
		qD2[3] = qS1[0]; \
		qD2[4] = ((qS1[1] + qS2[0]) >> 1); \
		qD2[5] = qS2[1]; \
		break; \
	\
	case 2:  /* G B */ \
		     /* R G */ \
		qD1[0] = qS2[0]; \
		qD1[1] = qS1[0]; \
		qD1[2] = qS1[1]; \
		\
		qD1[3] = qS2[0]; \
		qD1[4] = ((qS1[0] + qS2[1]) >> 1); \
		qD1[5] = qS1[1]; \
		\
		qD2[0] = qS2[0]; \
		qD2[1] = ((qS1[0] + qS2[1]) >> 1); \
		qD2[2] = qS1[1]; \
		\
		qD2[3] = qS2[0]; \
		qD2[4] = qS2[1]; \
		qD2[5] = qS1[1]; \
		break; \
	\
	default: /* G R */ \
		     /* B G */ \
		qD1[0] = qS1[1]; \
		qD1[1] = qS1[0]; \
		qD1[2] = qS2[0]; \
		\
		qD1[3] = qS1[1]; \
		qD1[4] = ((qS1[0] + qS2[1]) >> 1); \
		qD1[5] = qS2[0]; \
		\
		qD2[0] = qS1[1]; \
		qD2[1] = ((qS1[0] + qS2[1]) >> 1); \
		qD2[2] = qS2[0]; \
		\
		qD2[3] = qS1[1]; \
		qD2[4] = qS2[1]; \
		qD2[5] = qS2[0]; \
		break; \
	} \
}

static void /* 处理RAW四边儿的2行或2列像素转成RGB */
RawBorderRgb(BYTE *pRgb, const BYTE *pRaw, int nWd, int nHi, int nWho)
{
	int n2w = (nWd << 1), n3w = nWd * 3, n6w = (n3w << 1);
	int i, j;
	BYTE *pD1, *pD2;
	const BYTE *pS1, *pS2;
	
	/*-------------------------------------------------*/
	
	pD1 = pRgb; pD2 = pD1 + n3w;
	pS1 = pRaw; pS2 = pS1 + nWd;
	for(i=0; i<=nWd-2; i+=2) /* 上边：水平X步进2x2 */
	{
		BrdRawToRgb(pD1, pD2, pS1, pS2);
		pD1 += 6; pD2 += 6;
		pS1 += 2; pS2 += 2;
	}
	
	/*-------------------------------------------------*/
	
	pD1 = pRgb + (nHi - 2) * n3w; pD2 = pD1 + n3w;
	pS1 = pRaw + (nHi - 2) * nWd; pS2 = pS1 + nWd;
	for(i=0; i<=nWd-2; i+=2) /* 下边：水平X步进2x2 */
	{
		BrdRawToRgb(pD1, pD2, pS1, pS2);
		pD1 += 6; pD2 += 6;
		pS1 += 2; pS2 += 2;
	}
	
	/*-------------------------------------------------*/
	
	pD1 = pRgb + n6w; pD2 = pD1 + n3w;
	pS1 = pRaw + n2w; pS2 = pS1 + nWd;
	for(j=2; j<=nHi-2; j+=2) /* 左边：垂直Y步进2x2 */
	{
		BrdRawToRgb(pD1, pD2, pS1, pS2);
		pD1 += n6w; pD2 += n6w;
		pS1 += n2w; pS2 += n2w;
	}
	
	/*-------------------------------------------------*/
	
	pD1 = pRgb + n6w + n3w - 6; pD2 = pD1 + n3w;
	pS1 = pRaw + n2w + nWd - 2; pS2 = pS1 + nWd;
	for(j=2; j<=nHi-2; j+=2) /* 右边：垂直Y步进2x2 */
	{
		BrdRawToRgb(pD1, pD2, pS1, pS2);
		pD1 += n6w; pD2 += n6w;
		pS1 += n2w; pS2 += n2w;
	}
}

/*===============================================================*/

/* 3. RAW内部的2x2小块4点，变换成RGB，中心为G */
#define RawToRgb_GR_G(d, s) \
{ \
	*(d+0) = ((*(s-nWd) + *(s+nWd)) >> 1); \
	*(d+1) = *(s); \
	*(d+2) = ((*(s-1) + *(s+1)) >> 1); \
}

/* 2. RAW内部的2x2小块4点，变换成RGB，中心为G */
#define RawToRgb_GB_G(d, s) \
{ \
	*(d+0) = ((*(s-1) + *(s+1)) >> 1); \
	*(d+1) = *(s); \
	*(d+2) = ((*(s-nWd) + *(s+nWd)) >> 1); \
}

/* 1. RAW内部的2x2小块4点，变换成RGB，中心为B */
#define RawToRgb_RG_B(d, s) \
{ \
	*(d+0) = ((*(s-nWd-1) + *(s-nWd+1) + *(s+nWd-1) + *(s+nWd+1)) >> 2); \
	\
	dy = fabs(*(s-n2w) - *(s+n2w)); \
	dx = fabs(*(s-2) - *(s+2)); \
	\
	if(dy < dx) \
	{ \
		*(d+1) = ((*(s-nWd) + *(s+nWd)) >> 1); \
	} \
	else if(dy > dx) \
	{ \
		*(d+1) = ((*(s-1) + *(s+1)) >> 1); \
	} \
	else \
	{ \
		*(d+1) = ((*(s-nWd) + *(s+nWd) + *(s-1) + *(s+1)) >> 2); \
	} \
	\
	*(d+2) = *(s); \
}

/* 0. RAW内部的2x2小块4点，变换成RGB，中心为R */
#define RawToRgb_BG_R(d, s) \
{ \
	*(d+0) = *(s); \
	\
	dy = fabs(*(s-n2w) - *(s+n2w)); \
	dx = fabs(*(s-2) - *(s+2)); \
	\
	if(dy < dx) \
	{ \
		*(d+1) = ((*(s-nWd) + *(s+nWd)) >> 1); \
	} \
	else if(dy > dx) \
	{ \
		*(d+1) = ((*(s-1) + *(s+1)) >> 1); \
	} \
	else \
	{ \
		*(d+1) = ((*(s-nWd) + *(s+nWd) + *(s-1) + *(s+1)) >> 2); \
	} \
	\
	*(d+2) = ((*(s-nWd-1) + *(s-nWd+1) + *(s+nWd-1) + *(s+nWd+1)) >> 2); \
}

/* 从RAW格式使用插值的方法返回RGB数据，(0=BG,1=RG,2=GB,3=GR) */
void RgbFromRaw(BYTE *pRgb, const BYTE *pRaw, int nWd, int nHi, int nWho)
{
	int n2w = (nWd << 1), n3w = nWd * 3, n6w = (n3w << 1);
	int i, j;
	BYTE *pD1, *pD2, dx, dy;
	const BYTE *pS1, *pS2;
	
	pD1 = pRgb + n6w + 6; pD2 = pD1 + n3w;
	pS1 = pRaw + n2w + 2; pS2 = pS1 + nWd;
	
	switch(nWho)
	{
	case 0:  /* B G */
			 /* G R */
		for(j=2; j<=nHi-4; j+=2)
		{
			for(i=2; i<=nWd-4; i+=2)
			{
				RawToRgb_RG_B(pD1  , pS1  );
				RawToRgb_GR_G(pD1+3, pS1+1);
				RawToRgb_GB_G(pD2  , pS2  );
				RawToRgb_BG_R(pD2+3, pS2+1);
				
				pD1 += 6; pS1 += 2;
				pD2 += 6; pS2 += 2;
			}
			pD1 += 12 + n3w; pS1 += 4 + nWd;
			pD2 += 12 + n3w; pS2 += 4 + nWd;
		}
		break;
	
	case 1:  /* R G */
			 /* G B */
		for(j=2; j<=nHi-4; j+=2)
		{
			for(i=2; i<=nWd-4; i+=2)
			{
				RawToRgb_BG_R(pD1  , pS1  );
				RawToRgb_GB_G(pD1+3, pS1+1);
				RawToRgb_GR_G(pD2  , pS2  );
				RawToRgb_RG_B(pD2+3, pS2+1);
				
				pD1 += 6; pS1 += 2;
				pD2 += 6; pS2 += 2;
			}
			pD1 += 12 + n3w; pS1 += 4 + nWd;
			pD2 += 12 + n3w; pS2 += 4 + nWd;
		}
		break;
	
	case 2:  /* G B */
			 /* R G */
		for(j=2; j<=nHi-4; j+=2)
		{
			for(i=2; i<=nWd-4; i+=2)
			{
				RawToRgb_GR_G(pD1  , pS1  );
				RawToRgb_RG_B(pD1+3, pS1+1);
				RawToRgb_BG_R(pD2  , pS2  );
				RawToRgb_GB_G(pD2+3, pS2+1);
				
				pD1 += 6; pS1 += 2;
				pD2 += 6; pS2 += 2;
			}
			pD1 += 12 + n3w; pS1 += 4 + nWd;
			pD2 += 12 + n3w; pS2 += 4 + nWd;
		}
		break;
	
	default: /* G R */
			 /* B G */
		for(j=2; j<=nHi-4; j+=2)
		{
			for(i=2; i<=nWd-4; i+=2)
			{
				RawToRgb_GB_G(pD1  , pS1  );
				RawToRgb_BG_R(pD1+3, pS1+1);
				RawToRgb_RG_B(pD2  , pS2  );
				RawToRgb_GR_G(pD2+3, pS2+1);
				
				pD1 += 6; pS1 += 2;
				pD2 += 6; pS2 += 2;
			}
			pD1 += 12 + n3w; pS1 += 4 + nWd;
			pD2 += 12 + n3w; pS2 += 4 + nWd;
		}
		break;
	}
	
	RawBorderRgb(pRgb, pRaw, nWd, nHi, nWho);
}

#endif
/*=///////////////////////////////////////////////////////////////////////=*/
/*= The end of this file. =*/

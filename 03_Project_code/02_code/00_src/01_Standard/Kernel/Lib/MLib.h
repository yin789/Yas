#ifndef		 MLIB_H_
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		mlib_c_float.h : floatVersionMLIBFunction definition														*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		First edition   : 2000.01.14  T.Taniguchi	Mlib85E.c												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"

/****************************************************************************************************/
/*																									*/
/*		Endian conversion macro																		*/
/*																									*/
/****************************************************************************************************/
#if		defined( __CA850__ )
/*--------------------------------------------------------------------------------------------------*/
#define MlibByteSwap16( x, y )		{(y) = (USHORT)__bsh( (x) );}
/*--------------------------------------------------------------------------------------------------*/
	#if		defined( CCSW_LITTLE_ENDIAN ) 
	#define MlibCnvLittle16( x, y )	{ ; }
	#define MlibCnvBig16( x, y )	{ MlibByteSwap16( (x), (y) );}
	#endif
	/*----------------------------------------------------------------------------------------------*/
	#if		defined( CCSW_BIG_ENDIAN ) 
	#define MlibCnvBig16( x, y )		( x )
	#define MlibCnvLittle16( x, y )		{ MlibByteSwap16( (x), (y) );}
	#endif
/*--------------------------------------------------------------------------------------------------*/
#elif	defined( __RENESAS__ )
#define MlibByteSwap16( x, y )		{(y) = (USHORT)swapb( (x) );}
/*--------------------------------------------------------------------------------------------------*/
	#if		defined( CCSW_LITTLE_ENDIAN ) 
	#define MlibCnvLittle16( x, y )	{ ; }
	#define MlibCnvBig16( x, y )	{ MlibByteSwap16( (x), (y) );}
	#endif
	/*----------------------------------------------------------------------------------------------*/
	#if		defined( CCSW_BIG_ENDIAN ) 
	#define MlibCnvBig16( x, y )		{ ; }
	#define MlibCnvLittle16( x, y )		{ MlibByteSwap16( (x), (y) );}
	#endif
/*--------------------------------------------------------------------------------------------------*/
#elif	defined( __CC_ARM )
//#define MlibByteSwap16( x, y )   	__asm {  REV16  y, x  }
#define MlibByteSwap16( x, y )   	{ y = __rev( x ); }
/*--------------------------------------------------------------------------------------------------*/
	#if		(! defined( __BIG_ENDIAN ))
	#define MlibCnvLittle16( x, y )	{ ; }
	#define MlibCnvBig16( x, y )	{ MlibByteSwap16( (x), (y) );}
	#define MlibCnvBig32( x )	__rev( x )
	/*----------------------------------------------------------------------------------------------*/
	#else
	#define MlibCnvBig16( x, y )		( x )
	#define MlibCnvLittle16( x, y )		{ MlibByteSwap16( (x), (y) );}
	#endif
/*--------------------------------------------------------------------------------------------------*/
#elif	defined( _VC_DEBUG_ )
#define MlibByteSwap16( x, y )		(y) = (USHORT)( (((x) & 0xFF) << 8) + (((x) >> 8) & 0xFF) )
#define MlibByteSwap32( x, y )		(y) = (ULONG)( (((x) & 0xFF) << 24) + ((((x) >> 8) & 0xFF)<<16) + ((((x) >> 16) & 0xFF)<<8) + (((x) >> 24) & 0xFF) )
	#if		defined( CCSW_LITTLE_ENDIAN ) 
	#define MlibCnvLittle16( x, y )	{ ; }
	#define MlibCnvBig16( x, y )	( MlibByteSwap16( (x), (y) ) )
	#define MlibCnvBig32( x )		( MlibByteSwap32( (x), (x) ) )
	#endif
	/*----------------------------------------------------------------------------------------------*/
	#if		defined( CCSW_BIG_ENDIAN ) 
	#define MlibCnvBig16( x, y )		{ ; }
	#define MlibCnvLittle16( x, y )		( MlibByteSwap16( (x), (y) ) )
	#endif
/*--------------------------------------------------------------------------------------------------*/
#else
#define MlibByteSwap16( x, y )		{(y) = (USHORT)( (((x) & 0xFF) << 8) + (((x) >> 8) & 0xFF) );}
#endif
/*--------------------------------------------------------------------------------------------------*/


/****************************************************************************************************/
/*																									*/
/*		Basic Constant Definition																	*/
/*																									*/
/****************************************************************************************************/
#define	SRH_FROM_LSB			0
#define	SRH_FROM_MSB			1
#define	KSPAI					0x156487ED				/* {k,s} Type   PAI							*/
#define	KS2PAI					0x146487ED				/* {k,s} Type 2*PAI							*/
#define	KS3PAI					0x134B65F2				/* {k,s} Type 3*PAI							*/
#define	KS4PAI					0x136487ED				/* {k,s} Type 4*PAI							*/
/*--------------------------------------------------------------------------------------------------*/
#define	NORMAXVALUE				0x01000000				/* Normalized Maximum Value					*/
#define	NORMINVALUE				-0x01000000				/* Normalized Minimum Value					*/
/*--------------------------------------------------------------------------------------------------*/
#define	CCSW_HsMulgain			1					/* for MlibHsMulgain() Parameter Cal.			*/
#define	FLOAT2KF1GAIN			0x800000			/* FloattoKF1GAIN					*/

#define	PI						3.1415926535		/* π											*/
#define	DEG2RAD					0.0174532925194		/* π / 180										*/
#define	FOURTEENBIT2RAD			0.0003834951969604	/* (π / 180) * (360 / 16384)					*/

#define	C10POW9					1000000000			/* 10^9											*/
#define	C10POW7					10000000			/* 10^7											*/
#define	C2PAIE7					62831853			/* (2*PAI) * 10^7								*/

/****************************************************************************************************/
/*																									*/
/*		Basic numerical calculation macro																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Basic Macro																					*/
/*--------------------------------------------------------------------------------------------------*/
#define MlibABS( u )	(((u)>=0) ? (u) : -(u))
#define MlibMAX( a, b )	(((a)>(b)) ? (a) : (b))
#define MlibMIN( a, b )	(((a)<(b)) ? (a) : (b))
#define	MlibLIMITUL( u, ulim, llim )	(((u)>=(ulim))? (ulim) : (((u)<=(llim))? (llim) : (u)))
/*--------------------------------------------------------------------------------------------------*/
/*		64bit Compare Macro : GT(a>b),LT(a<b),GTE(a>=b),LTE(a<=b)									*/
/*--------------------------------------------------------------------------------------------------*/
#define	MlibEQ64( a, b )	( (a[1] == b[1]) && (a[0] == b[0]) )
#define	MlibGT64( a, b )	( (a[1] >  b[1]) || ((a[1] == b[1])&&((ULONG)a[0] >  (ULONG)b[0])) )
#define	MlibLT64( a, b )	( (a[1] <  b[1]) || ((a[1] == b[1])&&((ULONG)a[0] <  (ULONG)b[0])) )
#define	MlibGTE64( a, b )	( (a[1] >  b[1]) || ((a[1] == b[1])&&((ULONG)a[0] >= (ULONG)b[0])) )
#define	MlibLTE64( a, b )	( (a[1] <  b[1]) || ((a[1] == b[1])&&((ULONG)a[0] <= (ULONG)b[0])) )
/*--------------------------------------------------------------------------------------------------*/
/*		64bit Upper/Lower Limit Macro																*/
/*--------------------------------------------------------------------------------------------------*/
#define	MlibUPPERLMT64( x, ulim )	if( MlibGT64( x, ulim ) ){ x[0] = ulim[0]; x[1] = ulim[1];}
#define	MlibLOWERLMT64( x, llim )	if( MlibLT64( x, llim ) ){ x[0] = llim[0]; x[1] = llim[1];}
/*--------------------------------------------------------------------------------------------------*/
/*		Integer Data <--> Floating Data Macro							<S00A>				*/
/*--------------------------------------------------------------------------------------------------*/
#define	NORMALIZEINT( x )		((LONG)(x*(double)167772.16))		/* 167772.16 = 16777216/100		*/
#define	NORMALIZEINTX( x )		((LONG)(x*(float)150.00))		/* 150.00 = 15000/100			*/
#define	NORMALIZEFLOAT( x )		((float)(x/(double)167772.16))	/* 167772.16 = 100/16777216		*/
#define	NORMALIZEFLOATX( x )	((float)(x/(float)150.00))	/* 150.00 = 100/15000			*/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Basic numerical calculation macro																			*/
/*																									*/
/****************************************************************************************************/
extern	const	SHORT	MlibFastSinTbl[1024];				/* High-speed sine operation : Input:[360deg/1024]	*/
#define	MlibFASTSINS16( a )	(MlibFastSinTbl[(a)&0x3FF])		/* High-speed sine operation : Output:[1.000/16384]	*/
#define MlibFASTCOSS16( a ) (MlibFastSinTbl[(a+256)&0x3FF])	/* High-speed Cos calculation : Output:[1.000/16384]	*/
/*--------------------------------------------------------------------------------------------------*/
__inline LONG MlibLABS(LONG u)
{
	return 	(((u)>=0) ? (u) : -(u));
}
__inline LONG MlibAbs32(LONG u)
{
	return 	(((u)>=0) ? (u) : -(u));
}

/****************************************************************************************************/
/*																									*/
/*		64bitNumeric operation macro																			*/
/*																									*/
/****************************************************************************************************/
#if defined( __PowerPC )
/****************************************************************************************************/
/*																									*/
/*		CPU model dependent:INGRAM(PPC)For																*/
/*																									*/
/****************************************************************************************************/
__inline void	MUL3232(LONG a, LONG b, LONG *xx)
{
	xx[0] = (a)*(b);	
	xx[1] = __MULSH(a, b);		
}

__inline void	MULU32U32(ULONG a, ULONG b, ULONG *xx)
{
	xx[0] = (ULONG)((a)*(b));
	xx[1] = __MULUH(a, b);		
}

/*--------------------------------------------------------------------------------------------------*/
/*		Interrupt disable/enable																			*/
/*--------------------------------------------------------------------------------------------------*/
#undef	KPI_DI
#define	KPI_DI( )				{ ;}				/* Interrupt disable									*/
#undef	KPI_EI
#define	KPI_EI( )				{ ;}				/* Interrupt enable									*/
/*--------------------------------------------------------------------------------------------------*/

#elif	defined( WIN32 )

/***** Structure for handling floating point data *****//*<S00A>*/
typedef union F2L
{
	long				l;
	unsigned long		u;
	float				f;
	//struct
	//{
	//	long			l;
	//	long			h;
	//} sl;
	//struct
	//{
	//	unsigned long	l;
	//	unsigned long	h;
	//} ul;
	//double			df;
} F2L;
/****************************************************************************************************/
/*																									*/
/*		CPU model dependent: For PC																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
#define	MUL3232( a, b, xx )		*(INT64*)xx = ((INT64)(a))*((INT64)(b))
#define	MULU32U32( a, b, xx )	*(INT64*)xx = ((INT64)(ULONG)(a))*((INT64)(ULONG)(b))
/*--------------------------------------------------------------------------------------------------*/
/*		Interrupt disable/enable																			*/
/*--------------------------------------------------------------------------------------------------*/
#undef	KPI_DI
#define	KPI_DI( )				{ ;}				/* Interrupt disable									*/
#undef	KPI_EI
#define	KPI_EI( )				{ ;}				/* Interrupt enable									*/
/*--------------------------------------------------------------------------------------------------*/

#else
/****************************************************************************************************/
/*																									*/
/*		CPU model dependent:SGDZ-SVN(SH-4A)For															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
#define	MUL3232( a, b, xx )		*(INT64*)xx = ((INT64)(a))*((INT64)(b))
#define	MULU32U32( a, b, xx )	*(INT64*)xx = ((INT64)(ULONG)(a))*((INT64)(ULONG)(b))
/*--------------------------------------------------------------------------------------------------*/

#endif

typedef struct ull 
  { unsigned long lower, upper; }
ull;

typedef struct longlong
  { unsigned long lower; long upper; }
longlong;

#define Sub64_2(dest, a1, a2)	/* Subtract a2 from a1 result in dest  */  \
     (((dest).upper = (a1).upper - (a2).upper - ((a1).lower < (a2).lower)), \
     ((dest).lower = (a1).lower - (a2).lower))
#define Add64_2(dest, a1, a2)	/* Add a2 to a1.   */	   \
	((dest).lower = (a1).lower + (a2).lower, \
	(dest).upper = (a1).upper + (a2).upper + \
	((dest).lower < (a2).lower))
#define Add64_64_32_2(dest, a1, a2)	/* Like Add64_2, but a2 is UINT4 */ \
	((dest).lower = (a1).lower + (a2), \
	(dest).upper = (a1).upper + ((dest).lower < (a2)))

__inline void	ADD6464(LONG *a, LONG *b, LONG *xx)
{
#if 0
	xx[0] = a[0] + b[0];
//	xx[1] = a[1] + b[1] + (xx[0] < b[0]);
	xx[1] = a[1] + b[1] + ((ULONG)(xx[0]) < (ULONG)(b[0]));    /* <S053> */
#else/* <S07E> *///<S0C7>
LONG lworka0,lworka1;
LONG lworkb0,lworkb1;

lworka0 = a[0];
lworkb0 = b[0];
lworka1 = a[1];
lworkb1 = b[1];

	xx[0] = lworka0 + lworkb0;
	xx[1] = lworka1 + lworkb1 + ((ULONG)xx[0] < (ULONG)lworkb0);

#endif
}
__inline void	SUB6464(LONG *a, LONG *b, LONG *xx)
{
//	xx[1] = a[1] - b[1] - (a[0] < b[0]);
	xx[1] = a[1] - b[1] - ((ULONG)(a[0]) < (ULONG)(b[0]));   /* <S053> */
	xx[0] = a[0] - b[0];
}
__inline LONG	MlibMulhigh32( LONG a, LONG b )
{
	LONG xx[2];
	MUL3232(a,b,xx);
	return( xx[1] + ((ULONG)xx[0]>>31) );
}
__inline LONG	MlibMulhighu32( LONG a, LONG b )
{
	ULONG xx[2];
	MULU32U32(a,b,xx);
	return( xx[1] + ((ULONG)xx[0]>>31) );
}


/*--------------------------------------------------------------------------------------------------*/
/*		Interrupt disable/enable																			*/
/*--------------------------------------------------------------------------------------------------*/
#ifdef _TEST_BOARD_
#undef	KPI_DI
#define	KPI_DI( )		{  /*  TODO:  Interrupt disabling */                                \
							long st;                                                 \
							__asm {                                                  \
								MRS st, CPSR;      /* st <- CPSR */                  \
								ORR st, st, 0x80; /* set interrupt disable bit */   \
								MSR CPSR_c, st;    /* CPSR <- st */                  \
							} \
						} \

#undef	KPI_EI
#define	KPI_EI( )		{ /* TODO: Interrupt enable	*/\
							long st;                                                 \
							__asm {                                                  \
								MRS st, CPSR;      /* st <- CPSR */                  \
								BIC st, 0x80;     /* clear interrupt disable bit */ \
								MSR CPSR_c, st;    /* CPSR <- st */                  \
							} \
						}
/*--------------------------------------------------------------------------------------------------*/
#elif defined( WIN32 )
/* No allocation */
#elif defined( _MERCURY_PLATFORM_ )			/*<S190>*/
#define	KPI_DI( )		;
#define	KPI_EI( )		;
#else /* Tentatively unrestricted */
#define	KPI_DI( )		__DI();
#define	KPI_EI( )		__EI();
#endif
/*--------------------------------------------------------------------------------------------------*/


/****************************************************************************************************/
/*																									*/
/*		Basic control calculation function																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*   1) MlibLimitul()       : Upper/lower limit				rv=Limit value;							*/
/*   1-2) FlibLimitul()     : Upper/lower limit				rv=Limit value;							*//*<S00A>*/
/*   2) MlibMulgain()       : Gain multiplication(LimitStandard:2^24)	rv=(kx*u)>>sx; 0<=sx<=24				*/
/*   3) MlibMulgain27()     : Gain multiplication(LimitExtend:2^26)	rv=(kx*u)>>sx; 0<=sx<=24				*/
/*   4) MlibMulgain29()     : Gain multiplication(LimitExtend:2^28)	rv=(kx*u)>>sx; 0<=sx<=24				*/
/*   5) MlibMulgain30()     : Gain multiplication(LimitExtend:2^29)	rv=(kx*u)>>sx; 0<=sx<=24				*/
/*   6) MlibMulgain32()     : Gain multiplication(LimitExtend:2^31)	rv=(kx*u)>>sx; 0<=sx<=24  				*/
/*   7) MlibMulgainSx48()   : Gain multiplication(smaxEnlarge) 			rv=(kx*u)>>sx; 0<=sx<=48				*/
/*   8) MlibMulgainNolim()  : Gain multiplication(LimitNo capacity)			rv=(kx*u)>>sx; 0<=sx<=24				*/
/*   9) MlibIntegral()      : Integral calculation(LimitIncluded)			rv=(iu[1]>>1);iu[]=iu[]+kx*u;			*/
/*   9-2) FlibIntegral()      : Integral calculation(LimitIncluded)			rv=iu; iu+=kx*u							*//*<S00A>*/
/*--------------------------------------------------------------------------------------------------*/
/*  10) MlibPfbkxremLim()   : Position FB calculation with remainder			rv=(kx*u+pfbrem)>>sx;					*/
/*  11) MlibPfbkxremNolim() : Position FB calculation with remainder			rv=(kx*u+pfbrem)>>sx;					*/
/*  12) MlibMulkprem()      : Position gain multiplication with remainder		rv=(kx*u-((kx*pfbrem)>>24))>>sx;		*/
/*  13) MlibMulkxBiasrem()  : Gain multiplication with bias remainder	rv=(kx*u+rem+(sign(u)*bias>>s))>>sx;	*/
/*--------------------------------------------------------------------------------------------------*/
/*	14) MlibKrMulgain()     : Gain multiplication(With gain ratio)    rv=(kr*u)>>sx; kr=(kx*r)/(10000<<16)	*/
/*	15) MlibKrMulgain27()   : Gain multiplication(With gain ratio)    rv=(kr*u)>>sx; kr=(kx*r)/(10000<<16)	*/
/*	16) MlibKrIntegral()    : Integral calculation(With gain ratio)      iu=iu+(kr*u); rv=((iu[1]+x)>>1);		*/
/*--------------------------------------------------------------------------------------------------*/
/*  17) MlibSymMulgain()    : Gain multiplication(Positive/negative symmetric rounding)	rv=(kx*u)>>sx; 0<=sx<=24				*/
/*  18) MlibSymMulgainNolim : Gain multiplication(Positive/negative symmetric rounding)	rv=(kx*u)>>sx; 0<=sx<=24				*/
/*  19) MlibSymIntegral()   : Integral calculation(Positive/negative symmetric rounding)	rv=(iu[1]>>1);iu[]=iu[]+kx*u;			*/
/*																									*/
/****************************************************************************************************/
LONG	MlibLimitul( LONG u, LONG ulim, LONG llim );
#if (FLOAT_USE==TRUE)
float	FlibLimitul( float u, float ulim, float llim );
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibMulgain( LONG u, LONG k );
LONG	MlibMulgain27( LONG u, LONG k );
LONG	MlibMulgain29( LONG u, LONG k );
LONG	MlibMulgain30( LONG u, LONG k );
LONG	MlibMulgain32( LONG u, LONG k );
ULONG	MlibMulgain30U( ULONG u, LONG k );
ULONG	MlibMulgain31U( ULONG u, LONG k );
ULONG	MlibMulgain32U( ULONG u, LONG k );
LONG	MlibMulgainSx48( LONG u, LONG k );
LONG	MlibMulgain32Sx48( LONG u, LONG k );
LONG	MlibMulgainNolim( LONG u, LONG k );
LONG	MlibIntegral( LONG u, LONG k, LONG iu[2] );

#if (FLOAT_USE==TRUE)
float	FlibIntegral( float u, float k, float *iu );
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPfbkxremLim( LONG u, LONG k, LONG *pfbrem );
LONG	MlibPfbkxremNolim( LONG u, LONG k, LONG *pfbrem );
LONG	MlibMulkprem( LONG u, LONG k, LONG  pfbrem );
LONG	MlibMulkxBiasrem( LONG u, LONG k, LONG *rem, LONG bias );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibKrMulgain( LONG u, LONG k, LONG r );
LONG	MlibKrMulgain27( LONG u, LONG k, LONG r );
LONG	MlibKrIntegral( LONG u, LONG k, LONG r, LONG iu[2] );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibSymMulgain( LONG u, LONG k );
LONG	MlibSymMulgainNolim( LONG u, LONG k );
LONG	MlibSymIntegral( LONG u, LONG k, LONG iu[2] );
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Filter calculation function																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*   1) MlibLpfilter1()     : 1st order low pass filter			rv=x+((kx*(u-x))>>24);					*/
/*   1-2) FlibLpfilter1()     : 1st order low pass filter			rv=x+((kx*(u-x))>>24);					*//*<S00A>*/
/*   2) MlibHpfilter1()     : 1st order high pass filter			rv=u-z; z=z+((kx*(u-z))>>24);			*/
/*   3) MlibLpfilter2()     : Secondary low-pass filter			rv=(z[2]>>1);							*/
/*   4) MlibNxfilter2()     : 2nd order notch filter			rv=k[2]*(u-(z[2]>>1)-z[0])+(z[2]>>1);	*/
/*   5) MlibLaufilter()     : Linear accelerator(LAU)Filter		rv= z + dz or rv= z - dz;				*/
/*   6) MlibBITFILTER1()    : One-time delay bit filter		out = f( out, oldin, newin )			*/
/*   7) MlibKrLpfilter1()   : 1st order low pass filter(With ratio)	rv=x+((kr*(u-x))>>24);					*/
/*   8) MlibLowPassfilter2(): Second-order low-pass filter calculation(Bilinear transformation)									*/
/*   8-2) FlibLowPassfilter2(): Second-order low-pass filter calculation(Bilinear transformation)									*//*<S00A>*/
/*																									*/
/****************************************************************************************************/
LONG	MlibLpfilter1( LONG u, LONG k, LONG x  );
#if (FLOAT_USE==TRUE)
float	FlibLpfilter1( float u, float k, float x  );
float	FlibHpfilter1( float u, float k, float *z );
#endif /* FLOAT_USE */
LONG	MlibHpfilter1( LONG u, LONG k, LONG *z );
LONG	MlibLpfilter2( LONG u, LONG k[2], LONG z[3] );
LONG	MlibNxfilter2( LONG u, LONG k[3], LONG z[3] );
LONG	MlibLaufilter( LONG u, LONG z, LONG dz );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibKrLpfilter1( LONG u, LONG k, LONG r, LONG x );
LONG	MlibLowPassfilter2( LONG u, LONG k[4], LONG z[4] );
#if (FLOAT_USE==TRUE)
float	FlibLowPassfilter2( float u, float k[4], float z[4] );	/* Second-order low-pass filter calculation(Bilinear transformation)	*/
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
/*		One-time delay bit filter macro																*/
/*--------------------------------------------------------------------------------------------------*/
#define	MlibBITFILTER1( out, oldin, newin )\
		{ out = (((oldin | newin) & out ) | (oldin & newin)); oldin = newin;}
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Position related calculation function																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*   1) MlibPcmdIpfil()     : Position command interpolation filter			rv=(pcmdin+rem)/n						*/
/*   2) MlibPcmdMafil()     : Position command moving average filter		rv=sum/n, sum=sum+pcmdin-pmafbuf[idx]	*/
/*   3) MlibPcmdMafilSec()  : Position command moving average filter2		rv=sum/n, sum=sum+pcmdin-pmafbuf[idx]	*//*<S0C3>*/
/*   4) MlibPcmdImafil()    : Position interpolation moving average filter		rv=sum/n, sum=sum+newpcmd/n-oldpcmd/n	*/
/*   5) MlibPcmdImafilSec() : Position interpolation moving average filter2		rv=sum/n, sum=sum+newpcmd/n-oldpcmd/n	*//*<S0C3>*/
/*   6) MlibPcmdExpfil()    : Position command index acceleration/deceleration filter	rv=sum*kexp, sum=sum+pcmdin-rv			*/
/*   7) MlibPcmdLpfil1()    : Position command 1st order low-pass filter	rv=z*kf, z=z+pcmdin-rv					*/
/*   8) MlibPcmdLpfil2()    : Position command 2nd order low pass filter	rv=z[1]*kf1, z[0]=z[0]+pcmdin-rv		*/
/*   8-2) FlibPcmdLpfil2()  : Position command 2nd order low pass filter	rv=z[1]*kf1, z[0]=z[0]+pcmdin-rv		*//*<S00A>*/
/*   9) MlibPcmdNxfil2()    : Position command 2nd order notch filter	rv=pcmdin - Delta(kf2*z[1])				*/
/*  10) MlibPcmdEgear()     : Position command electronic gear calculation			rv=(B/A)*pcmda							*/
/*  10-2) FlibPcmdEgear()   : Position command electronic gear calculation			rv=(B/A)*pcmda							*//*<S00A>*/
/*  11) MlibEgearRvscnv()   : Electronic gear inversion calculation			rv=(A/B)*inpls							*/
/*  12) MlibPcalKegear()    : Electronic gear parameter calculation 		OUT: egear.k1, egear.k2, etc			*/
/*  13) MlibPerrcala()      : Position deviation calculation A(Command unit)		OUT: perra.per32s, perra.per32a, etc	*/
/*  13-2) FlibPerrcala()    : Position deviation calculation A(Command unit)		OUT: perra.per32s, perra.per32a, etc	*//*<S00A>*/
/*  14) MlibPerrcalx()      : Position deviation calculation(FB unit)		rv=per64[0]; per64[]=per64[]+pcmd-pfbk;	*/
/*  14-2) FlibPerrcalx()    : Position deviation calculation(FB unit)		rv=per64[0]; per64[]=per64[]+pcmd-pfbk;	*//*<S00A>*/
/*  15) MlibAposRg64iv()    : Initial absolute position calculation(Reverse electronic gear)	OUT: aposrg.apos[],aposrem,etc			*/
/*	16) MlibAposRg64dp()    : Absolute position update calculation(Reverse electronic gear)	OUT: aposrg.apos[],aposrem,etc			*/
/*																									*/
/****************************************************************************************************/
/*START <S0CD>*/
typedef	struct	{	
		LONG	GrvTrq;							/* Gravity Torque          */
		LONG	PosCTrq;						/* Positive Coulomb Torque */
		LONG	NegCTrq;						/* Negative Coulomb Torque */
		LONG	VisTrq;					        /* Viscous Torque          */
		LONG	MotFricEnable;	                /* Motor Friction Enable*/
		LONG    TrqCLast;						/* Torque Coulomb Last      */
		LONG    TrqGrvLast;						/* Torque Gravity Last      */
		LONG    SpdHys;				          	/* offset speed            */
		LONG    RatSpeed;				    	/* rate speed            */
		BOOL    InitEnable; 					/* Motor Friction Parameters Initialization Enable */
}FRICTION;
/*END <S0CD>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Position command interpolation filter																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{				/* Position command interpolation filter structure definition									*/
		LONG	cmdx;			/* Position command input														*/
		LONG	remx;			/* Position command output surplus													*/
} PIPFV;
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcmdIpfil( LONG pcmdin, LONG shift, LONG index, PIPFV *pipfvar );
/*--------------------------------------------------------------------------------------------------*/
/*		Position command moving average filter																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{				/* Position command moving average filter structure definition								*/
		SHORT	idx;			/* Buffer index												*/
		SHORT	ksub;			/* Subtraction amount calculation gain(0/1)											*/
		LONG	sumx;			/* Position command total														*/
		LONG	remx;			/* Position command surplus														*/
		USHORT	zcntx;			/* Payout completion check(0:Complete)										*/
		USHORT	spare;			/* Preliminary																*/
} PMAFV;
/*--------------------------------------------------------------------------------------------------*/
/*		Position command moving average filter2																	        */
/*--------------------------------------------------------------------------------------------------*//*<S0C3>*/
typedef	struct	{				/* Position command moving average filter structure definition*/
		SHORT	idxSec;			/* Buffer index												*/
		SHORT	ksubSec;		/* Subtraction amount calculation gain(0/1)											*/
		LONG	sumxSec;		/* Position command total														*/
		LONG	remxSec;		/* Position command surplus														*/
		USHORT	zcntxSec;		/* Payout completion check(0:Complete)										*/
		USHORT	spareSec;		/* Preliminary		                                                        */
} HIGHFV;
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcmdMafil( LONG pcmdin, LONG pmafnum, PMAFV *pmafvar, LONG *pmafbuf );
LONG	MlibPcmdMafilSec( LONG pcmdin, LONG pmafnum, HIGHFV *pmafvarSec, LONG *pmafbufSec );   /*<S0C3>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Position command interpolation moving average filter																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{				/* Position command interpolation moving average filter structure definition							*/
		LONG	sumx;			/* Position command total														*/
		LONG	remx;			/* Position command surplus														*/
		LONG	newpcmd;		/* Position command input														*/
		LONG	oldpcmd;		/* Position command input previous value												*/
		USHORT	zcntx;			/* Payout completion check(0:Complete)										*/
		USHORT	spare;			/* Preliminary																*/
} PIMFV;
/*--------------------------------------------------------------------------------------------------*/
/*		Position command interpolation moving average filter2																*/
/*--------------------------------------------------------------------------------------------------*/ /*<S0C3>*/
typedef	struct	{				/* High filterInterpolation moving average filter structure definition	                    */
		LONG	sumxSec;		/* Position command total														*/
		LONG	remxSec;		/* Position command surplus														*/
		LONG	newpcmdSec;		/* Position command input														*/
		LONG	oldpcmdSec;		/* Position command input previous value												*/
		USHORT	zcntxSec;		/* Payout completion check(0:Complete)										*/
		USHORT	spareSec;		/* Preliminary		                                                        */
} HIMFV;
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcmdImafil( LONG pcmdin, LONG shift, LONG index, PIMFV *pimfvar );
LONG	MlibPcmdImafilSec( LONG pcmdin, LONG shift, LONG indexSec, HIMFV *pimfvarSec );   /*<S0C3>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Position command index acceleration/deceleration filter																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{				/* Position command index acceleration/deceleration filter structure definition								*/
		LONG	sumx;			/* Position command total														*/
		LONG	remx;			/* Position command surplus(Unused)												*/
} PEXFV;
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcmdExpfil( LONG pcmdin, LONG kexp, LONG pbias, PEXFV *pexfvar );
/*--------------------------------------------------------------------------------------------------*/
/*		Position command low-pass filter																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcmdLpfil1( LONG pcmdin, LONG kf, LONG *z );
LONG	MlibPcmdLpfil2( LONG pcmdin, LONG kf[2], LONG z[2] );
#if (FLOAT_USE==TRUE)
float	FlibPcmdLpfil2( float pcmdin, float kf[2], float z[2] );
#endif /* FLOAT_USE */
LONG	MlibPcmdNxfil2( LONG pcmdin, LONG kf[3], LONG z[3] );
/*--------------------------------------------------------------------------------------------------*/
/*		Electronic gear & position deviation structure																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{				/* Electronic gear structure definition												*/
		LONG	a;				/* Electronic gear A														*/
		LONG	b;				/* Electronic gear B														*/
		LONG	k1;				/* Electronic gear gain:b/aupperbit										*/
		LONG	k2;				/* Electronic gear gain:b/abit										*/
		LONG	g1;				/* Electronic gear gain:a/bupperbit										*/
		LONG	g2;				/* Electronic gear gain:a/bbit										*/
#if (FLOAT_USE==TRUE)
		float		f_a;		/* Electronic gear A														*/
		float		f_b;		/* Electronic gear B														*/
		float		f_k1;		/* Electronic gear gain(Positive conversion):b/a										*/
		float		f_k2;		/* Electronic gear gain(Positive conversion):b/aRemainder									*/
		float		f_g1;		/* Electronic gear gain(Reverse conversion):a/b										*/
		float		f_g2;		/* Electronic gear gain(Reverse conversion):a/bRemainder									*/
#endif /* FLOAT_USE */
} EGEAR;
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{				/* Position deviation A structure definition												*/
		LONG	pfbkb;			/* Position FB(Command unit)Computation result										*/
		LONG	pfbkrem;		/* Position FB(Command unit)Computation result remainder									*/
		LONG	per64[2];		/* Position deviation64bitFor calculation												*/
		LONG	per32s;			/* Position deviation output(Rounding)											*/
		LONG	per32a;			/* Absolute value as above(Rounding)												*/
		LONG	per32sx;		/* Position deviation output(Round up)											*/
		LONG	per32ax;		/* Absolute value as above(Round up)												*/
		LONG	per32mx;		/* For position deviation monitor(Round up/0.5and zero)						*/
} PERRA;
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcmdEgear( LONG pcmda, EGEAR *egear, LONG *pcmdrem );
#if (FLOAT_USE==TRUE)
float	FlibPcmdEgear( float pcmda, EGEAR *egear, float *pcmdrem );
#endif /* FLOAT_USE */
void	MlibPcalKegear( LONG a, LONG b, EGEAR *egear );
LONG	MlibEgearRvscnv( LONG inpls, EGEAR *egear, LONG *plsrem );
void	MlibPerrcala( LONG pcmda, LONG pfbka, EGEAR *egear, PERRA *perra );
LONG	MlibPerrcalx( LONG pcmdx, LONG pfbkx, LONG per64[2] );
#if (FLOAT_USE==TRUE)
void	FlibPerrcala( LONG pcmda, LONG pfbka, EGEAR *egear, PERRA *perra );	/* Temporary treatment	*/
LONG	FlibPerrcalx( LONG pcmdx, LONG pfbkx, LONG per64[2] );	/* Temporary treatment	*/
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Absolute position calculation(Reverse electronic gear: FB unit-->Command unit)												*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{
		LONG	dposix;				/* Position difference				[Command unit]								*/
		LONG	aposrem;			/* Position surplus				[Command unit]								*/
		LONG	apos[2];			/* Absolute position(64bit)		[Command unit]								*/
} APOSRG;
/*--------------------------------------------------------------------------------------------------*/
void	MlibAposRg64iv( LONG inipos0, LONG inipos1, EGEAR *egear, APOSRG *aposrg );
void	MlibAposRg64dp( LONG dposin, EGEAR *egear, APOSRG *aposrg, LONG *latchpos );
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Position command generator function																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*   1) MlibPcmdMaker()     : Position command creator(64bit)			rv=vp, vp=vp+vpacc/vp=vp-vpdec			*/
/*   2) MlibIpTposLimit()   : Interpolation position command limit as above		rv=TRUE(Limit)/FALSE(NotLimit)			*/
/*   3) MlibRstPcmdMaker()  : Reset processing as above				pcmdout[]=pcmdset0,pcmdset1,etc			*/
/*   4) MlibIpcalPcmdMaker  : Initial parameter calculation as above		OUT: pcmkprm.osvpm, maxvpm, pshlx, etc	*/
/*   5) MlibPcalaPcmdMaker  : Positioning parameter calculation A as above	OUT: pcmkprm.vpacc, vpdec,  vpamx, etc	*/
/*   6) MlibPcalaPcmdMkrIP  : Same as above interpolation calculation parameter calculation A	OUT: pcmkprm.ipmaxspd, ipmaxacc,   etc	*/
/*   7) MlibPcalbPcmdMaker  : Positioning parameter calculation B as above	OUT: pcmkprm.vpacc, vpdec,  vpamx, etc	*/
/*   8) MlibPcalbPcmdMkrIP  : Same as above interpolation calculation parameter calculation B	OUT: pcmkprm.ipmaxspd, ipmaxacc,   etc	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Position command creator																				*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{					/* Position generator parameter structure definition								*/
		LONG	osvpm;				/* OS motor position command speed				[xpulse/scan]			*/
		LONG	maxvpm;				/* Maximum motor position command speed				[xpulse/scan]			*/
		LONG	maxipv;				/* Interpolation mode maximum position command speed			[xpulse/scan]			*/
		UCHAR	pshlx;				/* Position command calculation magnification(Shift)				[-]						*/
		UCHAR	pcmd64f;			/* Position command 64-bit flag				[TRUE/FALSE]			*/
		USHORT	iptimes;			/* InterpolateInterpolation number					[-]						*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	vpacc;				/* Position command acceleration						[xpulse/scan/scan]		*/
		LONG	vpdec;				/* Position command deceleration						[xpulse/scan/scan]		*/
		LONG	vpamx;				/* Quotient(vpacc/vpdec)						[-]						*/
		LONG	vparx;				/* Remainder(vpacc%vpdec)					[-]						*/
		LONG	vpapx;				/* Amount of acceleration deceleration and stop pulses				[xpulse]				*/
		LONG	maxspd;				/* Maximum position command speed						[2^24/OvrSpd]			*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	ipmaxspd;			/* Interpolation mode maximum speed					[ipulse/IPcycle]		*/
		LONG	ipmaxacc;			/* Interpolation mode maximum acceleration(Unused)			[ipulse/IPcycle^2]		*/
		LONG	ipstpdec;			/* Interpolation mode emergency stop deceleration				[ipulse/IPcycle^2]		*/
} PCMKP;
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{					/* Position generator calculation variable structure definition								*/
		UCHAR	calendf;			/* Computation end flag (Calculation in progress:0, Completion end:1)						*/
		UCHAR	cmderrf;			/* Command error flag (Interpolation mode position command error,etc)					*/
		UCHAR	pcmkmode;			/* Position command creation mode											*/
		UCHAR	modechgf;			/* Position command creation mode change flag									*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	vpx;				/* Position command speed							[xpulse/scan]			*/
		LONG	vpxrem;				/* Position command speed output remainder					[xpulse/scan]			*/
		LONG	pcmdout[2];			/* Position command output absolute value					[ipulse]				*/
		LONG	avp;				/* Work:Position command speed absolute value				[xpulse/scan]			*/
		LONG	n;					/* Work:Quotient(avp/vpdec)					[-]						*/
		LONG	rem;				/* Work:Remainder(avp%vpdec)					[-]						*/
		LONG	maxvp;				/* Work:Maximum position command speed				[xpulse/scan]			*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	ipvpi;				/* InterpolatePosition command speed				[ipulse/IPcycle]		*/
		LONG	ipvpx;				/* InterpolatePosition command speed				[ipulse/scan]			*/
		LONG	ipremi;				/* InterpolatePosition command speed remainder			[ipulse/scan]			*/
		LONG	ipremx;				/* InterpolatePosition command speed remainder			[ipulse/scan]			*/
		LONG	ipstopx[2];			/* IpWork:Deceleration and stop position calculation value			[ipulse]				*/
} PCMKV;
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{					/* Position generator calculation variables(P&V)Structure definition							*/
		PCMKP	P;					/* Parameter													*/
		PCMKV	V;					/* Variable for calculation													*/
} PCMKPV;
/*--------------------------------------------------------------------------------------------------*/
/*		Position command generator calculation execution function																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcmdMaker(				/* Position command generator calculation execution function									*/
		LONG	pcmdin0,			/* Position command input(Lower32bit)				[pulse]					*/
		LONG	pcmdin1,			/* Position command input(Upper level32bit)				[pulse]					*/
		LONG	pcmdspd,			/* Position command speed(Maximum/feed)				[2^24/OvrSpd]			*/
		PCMKPV	*pcmdmkr,			/* Position command generator calculation variable(P&V)		[-]						*/
		ULONG	pcmkmode	);		/* Position command creation mode					[-]						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Position command creation mode definition: Basic mode must be defined in the lower byte								*/
/*--------------------------------------------------------------------------------------------------*/
#define	PCMKMODE_NONE   0x0000		/* Position command creation mode : No capacity									*/
#define	PCMKMODE_IPA	0x0001		/* Position command creation mode : Interpolation(Input updated)						*/
#define	PCMKMODE_IPB	0x0101		/* Position command creation mode : Interpolation(Input not updated)						*/
#define	PCMKMODE_POS	0x0002		/* Position command creation mode : Positioning								*/
#define	PCMKMODE_FEED	0x0003		/* Position command creation mode : Constant speed feed								*/
#define	PCMKMODE_STOP	0x0004		/* Position command creation mode : Deceleration stop								*/
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibIpTposLimit(			/* Interpolation position command limit calculation										*/
		LONG	*tpos,				/* Target position(64bit)						[pulse]					*/
		LONG	*lmtpos,			/* Limit position(64bit)					[pulse]					*/
		LONG	lmtdir,				/* Limit direction(Positive side/negative side)				[-]						*/
		PCMKPV	*pcmdmkr	);		/* Position command generator calculation variable(P&V)		[-]						*/
/*--------------------------------------------------------------------------------------------------*/
#define	IPTPOSLMT_PSIDE		1		/* Limit direction: Positive side											*/
#define	IPTPOSLMT_NSIDE		0		/* Limit direction: Negative side											*/
/*--------------------------------------------------------------------------------------------------*/
void	MlibRstPcmdMaker(			/* Position command creator reset processing									*/
		LONG	pcmdset0,			/* Position command initial value(Lower32bit)			[pulse]					*/
		LONG	pcmdset1,			/* Position command initial value(Upper level32bit)			[pulse]					*/
		PCMKPV	*pcmdmkr	);		/* Position command generator calculation variable(P&V)		[-]						*/
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibIpcalPcmdMaker(			/* Position command generator initial parameter calculation								*/
		KSGAIN	ksosvp,				/* OS pulse speed						[xpulse/scan]			*/
		LONG	maxspdm,			/* Maximum motor speed						[2^24/OvrSpd]			*/
		LONG	maxspdi,			/* Interpolation mode maximum speed					[2^24/OvrSpd]			*/
		LONG	pcmd64f,			/* Position command 64-bit flag				[TRUE/FALSE]			*/
		PCMKP	*pcmkprm	);		/* Calculation result output structure pointer			[-]						*/
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcalaPcmdMaker(			/* Position command creator positioning parameter calculation A						*/
		LONG	maxspd,				/* Maximum speed								[2^24/OvrSpd]			*/
		LONG	acctime,			/* Acceleration time								[xs]					*/
		LONG	dectime,			/* Deceleration time								[xs]					*/
		LONG	scantime,			/* Scan time						[xs]					*/
		PCMKP	*pcmkprm	);		/* Calculation result output structure pointer			[-]						*/
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcalaPcmdMkrIP(			/* Position command generator interpolation calculation parameter calculation A						*/
		LONG	iptimes,			/* Interpolation number								[-]						*/
		LONG	maxspd,				/* Maximum speed								[2^24/OvrSpd]			*/
		LONG	ipacctm,			/* Minimum acceleration/deceleration time(Unused)				[xs]					*/
		LONG	ipdectm,			/* Emergency stop deceleration time						[xs]					*/
		LONG	scantime,			/* Scan time						[xs]					*/
		PCMKP	*pcmkprm	);		/* Calculation result output structure pointer			[-]						*/
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcalbPcmdMaker(			/* Position command creator positioning parameter calculation B						*/
		LONG	accrate,			/* Acceleration rate							[10000pulse/s/s]		*/
		LONG	decrate,			/* Deceleration rate							[10000pulse/s/s]		*/
		LONG	scantime,			/* Scan time(stm)					[us(Y=0)/ns(Y=1)]		*/
		PCMKP	*pcmkprm,			/* Calculation result output structure pointer			[-]						*/
		LONG	insel		);		/* Input selection(0xYX)						[X:Not used, Y:us/nsSelection]	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcalbPcmdMkrIP(			/* Position command generator interpolation calculation parameter calculation B						*/
		LONG	iptimes,			/* Interpolation number								[-]						*/
		LONG	ipaccrt,			/* Maximum acceleration/deceleration rate(Unused)				[10000pulse/s/s]		*/
		LONG	ipdecrt,			/* Emergency stop deceleration rate					[10000pulse/s/s]		*/
		LONG	scantime,			/* Scan time(stm)					[us(Y=0)/ns(Y=1)]		*/
		PCMKP	*pcmkprm,			/* Calculation result output structure pointer			[-]						*/
		LONG	insel		);		/* Input selection(0xYX)						[X:Not used, Y:us/nsSelection]	*/
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Parameter calculation function																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*   1) MlibPcalKxgain()    : Proportional gain calculation				{kx,sx} = (a*b/c)<<sx;					*/
/*   2) MlibPcalKskxkx()    : Extended gain calculation				{kx,sx} = {ka,sa}*b/c<<sx;				*/
/*   3) MlibPcalKxkskx()    : Extended gain calculation				{kx,sx} = a*{kb,sb}/c<<sx;				*/
/*   4) MlibPcalKxkxks()    : Extended gain calculation				{kx,sx} = a*b/{kc,sc}<<sx;				*/
/*   5) MlibPcalKskskx()    : Extended gain calculation				{kx,sx} = {ka,sa}*{kb,sb}/c<<sx;		*/
/*   6) MlibPcalKxksks()    : Extended gain calculation				{kx,sx} = a*{kb,sb}/{kc,sc}<<sx;		*/
/*   7) MlibPcalKsksks()    : Extended gain calculation				{kx,sx} = {ka,sa}*{kb,sb}/{kc,sc}<<sx;	*/
/*--------------------------------------------------------------------------------------------------*/
/*   8) MlibScalKxgain()    : Proportional gain calculation(For calculation start)	{kx,sx} = (a*b/c)<<sx; (No need to initialize the index)	*/
/*   9) MlibScalKskxkx()    : Extended gain calculation(For calculation start)	{kx,sx} = {ka,sa}*b/c<<sx;				*/
/*  10) MlibScalKxkskx()    : Extended gain calculation(For calculation start)	{kx,sx} = a*{kb,sb}/c<<sx;				*/
/*  11) MlibScalKxkxks()    : Extended gain calculation(For calculation start)	{kx,sx} = a*b/{kc,sc}<<sx;				*/
/*  12) MlibScalKskskx()    : Extended gain calculation(For calculation start)	{kx,sx} = {ka,sa}*{kb,sb}/c<<sx;		*/
/*  13) MlibScalKxksks()    : Extended gain calculation(For calculation start)	{kx,sx} = a*{kb,sb}/{kc,sc}<<sx;		*/
/*  14) MlibScalKsksks()    : Extended gain calculation(For calculation start)	{kx,sx} = {ka,sa}*{kb,sb}/{kc,sc}<<sx;	*/
/*--------------------------------------------------------------------------------------------------*/
/*  15) MlibPcalKxaddx()    : Proportional gain addition				{kx,sx} = {ka,sa}+{kb,sb};				*/
/*  16) MlibPcalKxsubx()    : Proportional gain subtraction				{kx,sx} = {ka,sa}-{kb,sb};				*/
/*  17) MlibPcalKxmulx()    : Proportional gain multiplication				{kx,sx} = (a*b*c)<<sx;					*/
/*  18) MlibPcalKxdivx()    : Proportional gain division				{kx,sx} = {ka,sa}/{kb,sb};				*/
/*--------------------------------------------------------------------------------------------------*/
/*  19) MlibPcalKf1gain()   : 1st order filter gain calculation		{kf,24} = (ts/(tx+ts))<<24;				*/
/*  19-2) FlibPcalKf1gain()   : 1st order filter gain calculation		{kf} = (ts/(tx+ts));					*//*<S00A>*/
/*  20) MlibPcalKf2gain()   : 2nd order filter gain calculation		OUT: kf[0], kf[1]						*/
/*  20-2) FlibPcalKf2gain()   : 2nd order filter gain calculation		OUT: kf[0], kf[1]						*//*<S00A>*/
/*  21) MlibPcalKnf2gain()  : 2nd order notch filter gain calculation	OUT: kf[0], kf[1], kf[2]				*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKxgain( LONG a,   LONG b,   LONG c,   LONG *psx, LONG smax );
LONG	MlibPcalKskxkx( LONG ksa, LONG b,   LONG c,   LONG *psx, LONG smax );
LONG	MlibPcalKxkskx( LONG a,   LONG ksb, LONG c,   LONG *psx, LONG smax );
LONG	MlibPcalKxkxks( LONG a,   LONG b,   LONG ksc, LONG *psx, LONG smax );
LONG	MlibPcalKskskx( LONG ksa, LONG ksb, LONG c,   LONG *psx, LONG smax );
LONG	MlibPcalKxksks( LONG a,   LONG ksb, LONG ksc, LONG *psx, LONG smax );
LONG	MlibPcalKsksks( LONG ksa, LONG ksb, LONG ksc, LONG *psx, LONG smax );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibScalKxgain( LONG a,   LONG b,   LONG c,   LONG *psx, LONG smax );
LONG	MlibScalKskxkx( LONG ksa, LONG b,   LONG c,   LONG *psx, LONG smax );
LONG	MlibScalKxkskx( LONG a,   LONG ksb, LONG c,   LONG *psx, LONG smax );
LONG	MlibScalKxkxks( LONG a,   LONG b,   LONG ksc, LONG *psx, LONG smax );
LONG	MlibScalKskskx( LONG ksa, LONG ksb, LONG c,   LONG *psx, LONG smax );
LONG	MlibScalKxksks( LONG a,   LONG ksb, LONG ksc, LONG *psx, LONG smax );
LONG	MlibScalKsksks( LONG ksa, LONG ksb, LONG ksc, LONG *psx, LONG smax );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcalKxaddx( LONG ka,  LONG sa,  LONG kb,  LONG sb,   LONG *psx );
LONG	MlibPcalKxsubx( LONG ka,  LONG sa,  LONG kb,  LONG sb,   LONG *psx );
LONG	MlibPcalKxmulx( LONG a,   LONG b,   LONG c,   LONG *psx            );
LONG	MlibPcalKxdivx( LONG ka,  LONG sa,  LONG kb,  LONG sb,   LONG smax );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibPcalKf1gain(			/* 1st order filter gain calculation										*/
		LONG	tx,					/* Time constant(Tx)/Frequency(Fx)	[xs],[0.1Hz] (X=0:Tx, X=1:Fx)		*/
		LONG	ts,					/* Scan time			[xs],[us/ns] (Y=0:us, Y=1:ns)		*/
		LONG	insel	);			/* Input selection(0xYX)			[X:Tx/FxSelection, Y:us/nsSelection]			*/

#if (FLOAT_USE==TRUE)
float	FlibPcalKf1gain(		/* 1st order filter gain calculation										*/
		LONG	tx,				/* Time constant(Tx)/Frequency(Fx)	[sec],[Hz] (X=0:Tx, X=1:Fx)			*/
		LONG	ts,				/* Calculation cycle					[sec] 								*/
		LONG	insel	);			/* Input specification(0x0X)			[X:Tx/FxSelection]						*/
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
void	MlibPcalKf2gain(			/* 2nd order filter gain calculation										*/
		LONG	hz,					/* Filter frequency			[0.1Hz]								*/
		LONG	dx,					/* Filter attenuation coefficient			[0.001]								*/
		LONG	ts,					/* Scan time			[us/ns]	(Y=0:us, Y=1:ns)			*/
		LONG	kf[2],				/* Calculation result storage pointer		[--]								*/
		LONG	insel	);			/* Input selection(0xYX) 			[X:Not used, Y:us/nsSelection]				*/
#if (FLOAT_USE==TRUE)
void	FlibPcalKf2gain(			/* 2nd order filter gain calculation										*/
		float	hz,				/* Filter frequency			[Hz]								*/
		float	dx,				/* Filter attenuation coefficient(ζ)		[-]									*/
		float	ts,				/* Scan time			[sec]								*/
		float	kf[2]);			/* Calculation result storage pointer		[--]								*/
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
void	MlibPcalKnf2gain(			/* 2nd order notch filter gain calculation									*/
		LONG	hz,					/* Filter frequency			[0.1Hz]								*/
		LONG	qx,					/* Q constant(Qx)/Attenuation coefficient(Dx)	[0.001]	(X=0:Qx, X=1:Dx)			*/
		LONG	kn,					/* Notch depth gain			[0.001]								*/
		LONG	ts,					/* Scan time			[us/ns] (Y=0:us, Y=1:ns)			*/
		LONG	kf[3],				/* Calculation result storage pointer		[--]								*/
		LONG	insel	);			/* Input selection(0xYX)			[X:Qx/DxSelection, Y:us/nsSelection]			*/
// <V069> Change Start
/*--------------------------------------------------------------------------------------------------*/
/*		Gain operation macro																				*/
/*--------------------------------------------------------------------------------------------------*/
/*			Little Endian For																			*/
#define	MlibGAINKX( k )		((LONG)((k)<<8)>>8)
#define	MlibGAINSX( k )		(((CHAR*)&(k))[3])
/*--------------------------------------------------------------------------------------------------*/
/*			Round down For																				*/
#define	MlibGAINRD( k )		(MlibGAINKX( k ) >> ((LONG) ((ULONG) (k)>>24)))
/*--------------------------------------------------------------------------------------------------*/
// <V069> Change End



/****************************************************************************************************/
/*																									*/
/*		Basic numerical calculation function																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*   1) MlibAbs32()         : Absolute value(32bit)															*/
/*   2) MlibAdd6432()       : Addition(Signed,64bit+32bit)												*/
/*   3) MlibAdd6464()       : Addition(Signed,64bit+64bit)												*/
/*   4)	MlibSub6432()       : Subtraction(Signed,64bit-32bit)												*/
/*   5)	MlibSub6464()       : Subtraction(Signed,64bit-64bit)												*/
/*   6) MlibErr6464()       : Deviation(Signed,64bit-64bit,Return value saturation function)							*/
/*   7) MlibSatAdd24()      : Saturation addition(Signed,32bit+32bit,Limit:0x01000000,0xFF000000)				*/
/*   8) MlibSatAdd28()      : Saturation addition(Signed,32bit+32bit,Limit:0x10000000,0xF0000000)				*/
/*   9) MlibSatAdd32()      : Saturation addition(Signed,32bit+32bit,Limit:0x7FFFFFFF,0x80000000)				*/
/*  10) MlibSatAddu32()     : Saturation addition(No sign,32bit+32bit,Limit:0xFFFFFFFF)							*/
/*  11) MlibMul3232()       : Multiplication(Signed,32bit*32bit)												*/
/*  12) MlibMulu32u32()     : Multiplication(No sign,32bit*32bit)												*/
/*  13) MlibMulhigh32()     : Multiplication(Signed,32bit*32bit,Return value:Upper level32bit,Rounding)					*/
/*  14) MlibMulhighu32()    : Multiplication(No sign,32bit*32bit,Return value:Upper level32bit,Rounding)					*/
/*  15) MlibDivhrem()       : Division(Signed,32bit/16bit,Output:quotient & remainder)								*/
/*  16) MlibDiv6432()       : Division(Signed,32bit*32bit/32bit,Limit:0x7FFFFFFF,Rounding)				*/
/*  17) MlibDivx3232()      : Extended division((32bit<<sx)/32bit,Limit:(qmax<<sx))							*/
/*  18) MlibSqrtu32()       : Route calculation(Input:32bit, Output:16bit(Max.0xFFFF),Rounding)				*/
/*  19) MlibSqrtu64()       : Route calculation(Input:64bit, Output:32bit(Max.0xFFFFFFFF),Rounding)			*/
/*  20) MlibSqrtu32u32()    : Route calculation(Input:32bit*32bit, Output:32bit(Max.0xFFFFFFFF),Rounding)		*/
/*  21) MlibSins16()        : Sin calculation(Input:[0.1deg],[360/16384deg],Output:[1.0/10000],[1.0/16384])	*/
/*	22) MlibAbsErrchk32()   : Absolute value deviation check(rv=(|a32-b32|<=chk32))								*/
/*	23) MlibAbsErrchk64()   : Absolute value deviation check(rv=(|a64-b64|<=chk32))								*/
/*																									*/
/****************************************************************************************************/
//LONG	MlibAbs32( LONG u );
//void	MlibAdd6432( LONG *a, LONG  b, LONG *x );
//void	MlibAdd6464( LONG *a, LONG *b, LONG *x );
/*--------------------------------------------------------------------------------------------------*/
//void	MlibSub6432( LONG *a, LONG  b, LONG *x );
//void	MlibSub6464( LONG *a, LONG *b, LONG *x );
LONG	MlibErr6464( LONG *a, LONG *b );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibSatAdd24( LONG a, LONG b );
LONG	MlibSatAdd28( LONG a, LONG b );
KSGAIN	MlibSatAdd32( KSGAIN a, KSGAIN b );		/*<S00A>*/
ULONG	MlibSatAddu32( ULONG a, ULONG b );
/*--------------------------------------------------------------------------------------------------*/
//void	MlibMul3232( LONG a, LONG b, LONG *x );
//void	MlibMulu32u32( ULONG a, ULONG b, ULONG *x );
//LONG	MlibMulhigh32( LONG a, LONG b );
//ULONG	MlibMulhighu32( ULONG a, ULONG b );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibDivhrem( LONG u, SHORT k, SHORT *rem );
LONG	MlibDiv6432( LONG a, LONG b, LONG c );
LONG	MlibDivx3232( LONG a, LONG b, LONG qmax, LONG sx );
/*--------------------------------------------------------------------------------------------------*/
USHORT	MlibSqrtu32( ULONG a );
ULONG	MlibSqrtu64( ULONG a0, ULONG a1 );
ULONG	MlibSqrtu32u32( ULONG a, ULONG b );
LONG	MlibSins16( LONG a, LONG unit );
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibAbsErrchk32( LONG  a32, LONG  b32, LONG chk32 );
LONG	MlibAbsErrchk64( LONG *a64, LONG *b64, LONG chk32 );
/*--------------------------------------------------------------------------------------------------*/
#define	MlibAdd6464( a, b, xx )		ADD6464( a, b, xx )
#define	MlibSub6464( a, b, xx )		SUB6464( a, b, xx )
#define	MlibMul3232( a, b, xx )		MUL3232( a, b, xx )
#define	MlibMulu32u32( a, b, xx )	MULU32U32( a, b, xx )



/****************************************************************************************************/
/*																									*/
/*		Special operation processing function																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*   1) MlibSrhbiton()      : Search bit ON(LSB/MSB, Return value:BitNo/NG(-1))				*/
/*	 2) MlibSetCRC16()		: Setting CRC16(MEMOBUSFor),		Return value:No capacity							*/
/*	 3) MlibChkCRC16()		: Checking CRC16(MEMOBUSFor),	Return value:TRUE(Normal)/FALSE(Abnormal)		*/
/*																									*/
/****************************************************************************************************/
LONG	MlibSrhbiton( ULONG data, ULONG dir );
#define	SRH_FROM_LSB	0
#define	SRH_FROM_MSB	1
/*--------------------------------------------------------------------------------------------------*/
#define MlibSetCRC16( Dbuf, Length) MlibSetCRC16MB( Dbuf, Length )
#define MlibChkCRC16( Dbuf, Length) MlibChkCRC16MB( Dbuf, Length )
/*--------------------------------------------------------------------------------------------------*/
void	MlibSetCRC16MB( UCHAR *MsgBuf, LONG MsgLen );
LONG	MlibChkCRC16MB( UCHAR *MsgBuf, LONG MsgLen );
/*--------------------------------------------------------------------------------------------------*/
void	MlibSetSum16SM( UCHAR *MsgBuf, LONG MsgLen );		/* for Sigma Message */
LONG	MlibChkSum16SM( UCHAR *MsgBuf, LONG MsgLen );		/* for Sigma Message */
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibCheckPowerOf2(							/* Power of 2 check : rv = BitNo/NG(-1)			*/
			ULONG	x 			);					/* Check data								*/
/*--------------------------------------------------------------------------------------------------*/


/****************************************************************************************************/
/*																									*/
/*		Memory reset function																			*/
/*																									*/
/****************************************************************************************************/
void	MlibResetByteMemory(						/* Reset byte memory							*/
			void	*pRstMem,						/* Reset memory start pointer					*/
			LONG	ByteNum		);					/* Number of reset memory bytes						*/
/*--------------------------------------------------------------------------------------------------*/
void	MlibResetLongMemory(						/* Long memory reset							*/
			void	*pRstMem,						/* Reset memory start pointer					*/
			LONG	LwdNum		);					/* Number of reset memory long words					*/
/*--------------------------------------------------------------------------------------------------*/
void	MlibCopyByteMemory(							/* Copy byte memory							*/
			void		*pDesMem,					/* Copy destination										*/
			const void	*pSrcMem,					/* Copy source										*/
			INT			ByteNum	);					/* Copy byte count								*/
/*--------------------------------------------------------------------------------------------------*/
void	MlibCopyLongMemory(							/* Long memory copy							*/
			void	*dstMem,						/* Copy destination										*/
			void	*srcMem,						/* Copy source										*/
			INT		nbyte );						/* Copy byte count								*/
/*--------------------------------------------------------------------------------------------------*/
void	memcpy( void*, void*, UINT );				/* Copy byte memory							*/

/****************************************************************************************************/
/*																									*/
/*		Others																			*/
/*																									*/
/****************************************************************************************************/
/*		Calculating the index value : 10^exp																		*/
LONG	MlibExpCal( CHAR exp );
#if (FLOAT_USE==TRUE)
void	FlibExpCal( CHAR exp, float *output );
#endif /* FLOAT_USE */
/*		KSGAIN → LONG Data Convert																	*/
LONG	MlibKsgain2Long( KSGAIN Input );

extern const	UCHAR	MlibAsciiTbl[ ];
extern const	UCHAR	MlibEvenAsciiTbl[ ]; 					/*<S02D>*/

/*--------------------------------------------------------------------------------------------------*/
/*		Decimal power table(Decimal Power Table)														*/
/*--------------------------------------------------------------------------------------------------*/
extern const ULONG MlibDecPowerTbl[];




#define MLIB_H_
#endif /* MLIB_H_ */
/***************************************** end of file **********************************************/



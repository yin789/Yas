/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Mlib85E.c : Control calculation library (C language version)													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*   1. Basic control calculation function																			*/
/*--------------------------------------------------------------------------------------------------*/
/*   1) MlibLimitul()       : Upper/lower limit				rv=Limit value;							*/
/*   2) MlibMulgain()       : Gain multiplication					rv=(kx*u)>>sx; smax=24					*/
/*   3) MlibMulgain29()     : Gain multiplication(LimitExtend)			rv=(kx*u)>>sx; smax=24					*/
/*   4) MlibMulgain30()     : Gain multiplication(LimitExtend)			rv=(kx*u)>>sx; smax=24					*/
/*   5) MlibMulgain32()     : Gain multiplication(LimitExtend)			rv=(kx*u)>>sx; smax=24  				*/
/*   6) MlibMulgainSx48()   : Gain multiplication(smaxEnlarge) 			rv=(kx*u)>>sx; smax=48					*/
/*   7) MlibMulgainNolim()  : Gain multiplication(LimitNo capacity)			rv=(kx*u)>>sx; smax=24					*/
/*   8) MlibIntegral()      : Integral calculation						rv=(iu[1]>>1);iu[]=iu[]+kx*u;			*/
/*   8-2) FlibIntegral()    : Integral calculation						rv=iu; iu+=kx*u;						*//*<S00A>*/
/*   9) MlibPfbkxremLim()   : Position FB calculation with remainder			rv=(kx*u+pfbrem)>>sx;					*/
/*  10) MlibPfbkxremNolim() : Position FB calculation with remainder			rv=(kx*u+pfbrem)>>sx;					*/
/*  11) MlibMulkprem()      : Position gain multiplication with remainder		rv=(kx*u-((kx*pfbrem)>>24))>>sx;		*/
/*  12) MlibMulkxBiasrem()  : Gain multiplication with bias remainder	rv=(kx*u+rem+(sign(u)*bias>>s))>>sx;	*/
/*  13) MlibSymMulgain()    : Gain multiplication(Positive/negative symmetric rounding)	rv=(kx*u)>>sx; smax=24					*/
/*  14) MlibSymMulgainNolim : Gain multiplication(Positive/negative symmetric rounding)	rv=(kx*u)>>sx; smax=24					*/
/*  15) MlibSymIntegral()   : Integral calculation(Positive/negative symmetric rounding)	rv=(iu[1]>>1);iu[]=iu[]+kx*u;			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*   2. Filter calculation function																			*/
/*--------------------------------------------------------------------------------------------------*/
/*   1) MlibLpfilter1()     : 1st order low pass filter			rv=x+((kx*(u-x))>>24);					*/
/*   1-2) FlibLpfilter1()     : 1st order low pass filter			rv=x+((kx*(u-x))>>24);					*//*<S00A>*/
/*   2) MlibHpfilter1()     : 1st order high pass filter			rv=u-z; z=z+((kx*(u-z))>>24);			*/
/*   2-2) FlibHpfilter1()   : 1st order high pass filter			rv=u-z; z=z+(kx*(u-z));					*//*<S00A>*/
/*   3) MlibLpfilter2()     : Secondary low-pass filter			rv=(z[2]>>1);							*/
/*   4) MlibNxfilter2()     : 2nd order notch filter			rv=k[2]*(u-(z[2]>>1)-z[0])+(z[2]>>1);	*/
/*   5) MlibLaufilter()     : Linear accelerator(LAU)Filter		rv= z + dz or rv= z - dz;				*/
/*   6) MlibBITFILTER1()    : One-time delay bit filter		out = f( out, oldin, newin )			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*   3. Control parameter calculation function																	*/
/*--------------------------------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------------------------------*/
/*   4. Position related calculation function																			*/
/*--------------------------------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------------------------------*/
/*   5. Position command generator function																			*/
/*--------------------------------------------------------------------------------------------------*/
/*   1) MlibPcmdMaker()     : Position command creator(64bit)			rv=vp, vp=vp+vpacc/vp=vp-vpdec			*/
/*   2) MlibIpTposLimit()   : Interpolation position command limit as above		rv=TRUE(Limit)/FALSE(NotLimit)			*/
/*   3) MlibRstPcmdMaker()  : Reset processing as above				pcmdout[]=pcmdset0,pcmdset1,etc			*/
/*   4) MlibIpcalPcmdMaker  : Initial parameter calculation as above		OUT: pcmkprm.osvpm, maxvpm, pshlx, etc	*/
/*   5) MlibPcalaPcmdMaker  : Positioning parameter calculation A as above	OUT: pcmkprm.vpacc, vpdec,  vpamx, etc	*/
/*   6) MlibPcalaPcmdMkrIP  : Same as above interpolation calculation parameter calculation A	OUT: pcmkprm.ipmaxspd, ipmaxacc,   etc	*/
/*   7) MlibPcalbPcmdMaker  : Positioning parameter calculation B as above	OUT: pcmkprm.vpacc, vpdec,  vpamx, etc	*/
/*   8) MlibPcalbPcmdMkrIP  : Same as above interpolation calculation parameter calculation B	OUT: pcmkprm.ipmaxspd, ipmaxacc,   etc	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*   6. Basic numerical calculation function																			*/
/*--------------------------------------------------------------------------------------------------*/
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
/*  13) MlibMulhigh32()     : Multiplication(Signed,32bit*32bit,Return value:Upper level32bit)								*/
/*  14) MlibMulhighu32()    : Multiplication(No sign,32bit*32bit,Return value:Upper level32bit)								*/
/*  15) MlibDivhrem()       : Division(Signed,32bit/16bit,Output:quotient & remainder)								*/
/*  16) MlibDiv6432()       : Division(Signed,32bit*32bit/32bit,Limit:0x7FFFFFFF)						*/
/*  17) MlibDivx3232()      : Extended division((32bit<<sx)/32bit,Limit:(qmax<<sx))							*/
/*  18) MlibSqrtu32()       : Route calculation(Input:32bit, Output:16bit(Max.0xFFFF), Rounding)				*/
/*  19) MlibSqrtu64()       : Route calculation(Input:64bit, Output:32bit(Max.0xFFFFFFFF), Rounding)			*/
/*  20) MlibSqrtu32u32()    : Route calculation(Input:32bit*32bit, Output:32bit(Max.0xFFFFFFFF), Rounding)	*/
/*  21) MlibSins16()        : Sin calculation(Input:[0.1deg],[360/16384deg],Output:[1.0/10000],[1.0/16384])	*/
/*	22) MlibAbsErrchk32()   : Absolute value deviation check(rv=(|a32-b32|<=chk32))								*/
/*	23) MlibAbsErrchk64()   : Absolute value deviation check(rv=(|a64-b64|<=chk32))								*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*   7. Special operation processing function																			*/
/*--------------------------------------------------------------------------------------------------*/
/*   1) MlibSrhbiton()      : Search bit ON(LSB/MSB, Return value:BitNo/NG(-1))				*/
/*	 2) MlibSetCRC16MB()	: Setting CRC16(MEMOBUSFor),		Return value:No capacity							*/
/*	 3) MlibChkCRC16MB()	: Checking CRC16(MEMOBUSFor),	Return value:TRUE(Normal)/FALSE(Abnormal)		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*   8. Memory operation function																				*/
/*--------------------------------------------------------------------------------------------------*/
/*	 1) MlibResetByteMemory() : Byte memory reset function											*/
/*	 2) MlibResetLongMemory() : Long memory reset function											*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*   9. Basic control calculation function(Fixed point gain version,SH2For)													*/
/*--------------------------------------------------------------------------------------------------*/
/*   1) MlibHsMulgain()       : Gain multiplication					rv=(kx*u)>>16; sx=16					*/
/*   2) MlibHsMulgainNolim()  : Gain multiplication(LimitNo capacity)		rv=(kx*u)>>16; sx=16					*/
/*   3) MlibHsIntegral()      : Integral calculation					rv=(iu[1]>>1);iu[]=iu[]+kx*u;			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*  10. C language version function(ASM version without function)																*/
/*--------------------------------------------------------------------------------------------------*/
/*	 1) MlibCheckPowerOf2()   : Power of 2 check			rv = BitNo/NG(-1)						*/
/*   2) MlibCopyByteMemory()  : Copy byte memory			Return value:No capacity								*/
/*   3) MlibCopyLongMemory()  : Long memory copy			Return value:No capacity								*/
/*	 4) Other																						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*  11. Others(Old specification library functions, etc.)																*/
/*--------------------------------------------------------------------------------------------------*/
/*	 1)	MlibSgdsPcmdLau()     : Position command creator(SGDSVersion)		rv=vp, vp=vp+vpacc/vp=vp-vpdec			*/
/*   2) MlibSgdsXxxxxxx()     : Same as above related processing function(SGDSVersion)											*/
/*	 3) Other Functions		  : Other functions															*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*  Note: Library function name supplement																		*/
/*--------------------------------------------------------------------------------------------------*/
/*	 1) ASM version function name : MlibLimitul(), MlibMulgain(), MlibXxxxxYyyyy(), etc						*/
/*	 2) C language version function name : MlibLimitul(), MlibMulgain(), MlibXxxxxYyyyy(), etc						*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		First edition   : 2000.01.14  T.Taniguchi	Mlib85E.c												*/
/*		Integrated version : 2005.05.27  T.Taniguchi	Mlib85E.c/MlibSH2.cCreate an integrated version based on					*/
/*		Revision01 : 2005.08.04	 T.Taniguchi	Source organization												*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include	"Basedef.h"
#include	"MLib.h"
#include	<math.h>



/****************************************************************************************************/
/*																									*/
/*		Unsigned 32bitScaling up/down macro												*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		SCUP31_SXINC( ) : x = (x<<sx), s = (s+sx),  0x40000000 <= x < 0x80000000					*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCUP31_SXINC( x, s, wk )\
		wk=0x00008000; if( (ULONG)x <  (ULONG)wk ){ x=(x<<16); s=s+16;}\
		wk=(wk<<8)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 8); s=s+ 8;}\
		wk=(wk<<4)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 4); s=s+ 4;}\
		wk=(wk<<2)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 2); s=s+ 2;}\
		wk=(wk<<1)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 1); s=s+ 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		SCUP31_SXDEC( ) : x = (x<<sx), s = (s-sx),  0x40000000 <= x < 0x80000000					*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCUP31_SXDEC( x, s, wk )\
		wk=0x00008000; if( (ULONG)x <  (ULONG)wk ){ x=(x<<16); s=s-16;}\
		wk=(wk<<8)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 8); s=s- 8;}\
		wk=(wk<<4)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 4); s=s- 4;}\
		wk=(wk<<2)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 2); s=s- 2;}\
		wk=(wk<<1)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 1); s=s- 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		SCUP23_SXINC( ) : x = (x<<sx), s = (s+sx),  0x00400000 <= x < 0x00800000					*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCUP23_SXINC( x, s, wk )\
		wk=0x00000080; if( (ULONG)x <  (ULONG)wk ){ x=(x<<16); s=s+16;}\
		wk=(wk<<8)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 8); s=s+ 8;}\
		wk=(wk<<4)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 4); s=s+ 4;}\
		wk=(wk<<2)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 2); s=s+ 2;}\
		wk=(wk<<1)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 1); s=s+ 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		SCUP23_SXDEC( ) : x = (x<<sx), s = (s-sx),  0x00400000 <= x < 0x00800000					*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCUP23_SXDEC( x, s, wk )\
		wk=0x00000080; if( (ULONG)x <  (ULONG)wk ){ x=(x<<16); s=s-16;}\
		wk=(wk<<8)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 8); s=s- 8;}\
		wk=(wk<<4)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 4); s=s- 4;}\
		wk=(wk<<2)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 2); s=s- 2;}\
		wk=(wk<<1)   ; if( (ULONG)x <  (ULONG)wk ){ x=(x<< 1); s=s- 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		SCDN15_SXINC( ) : x = (x>>sx), s = (s+sx),  0x00008000 > x > 0x00000000						*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCDN15_SXINC( x, s, wk )\
		wk=0x40000000; if( (ULONG)x >= (ULONG)wk ){ x=(x>>16); s=s+16;}\
		wk=(wk>>8)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 8); s=s+ 8;}\
		wk=(wk>>4)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 4); s=s+ 4;}\
		wk=(wk>>2)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 2); s=s+ 2;}\
		wk=(wk>>1)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 1); s=s+ 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		SCDN15_SXDEC( ) : x = (x>>sx), s = (s-sx),  0x00008000 > x > 0x00000000						*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCDN15_SXDEC( x, s, wk )\
		wk=0x40000000; if( (ULONG)x >= (ULONG)wk ){ x=(x>>16); s=s-16;}\
		wk=(wk>>8)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 8); s=s- 8;}\
		wk=(wk>>4)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 4); s=s- 4;}\
		wk=(wk>>2)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 2); s=s- 2;}\
		wk=(wk>>1)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 1); s=s- 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		SCDN23_SXINC( ) : x = (x>>sx), s = (s+sx),  0x00800000 > x > 0x00000000						*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCDN23_SXINC( x, s, wk )\
		wk=0x40000000; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 8); s=s+ 8;}\
		wk=(wk>>4)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 4); s=s+ 4;}\
		wk=(wk>>2)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 2); s=s+ 2;}\
		wk=(wk>>1)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 1); s=s+ 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		SCDN23_SXDEC( ) : x = (x>>sx), s = (s-sx),  0x00800000 > x > 0x00000000						*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCDN23_SXDEC( x, s, wk )\
		wk=0x40000000; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 8); s=s- 8;}\
		wk=(wk>>4)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 4); s=s- 4;}\
		wk=(wk>>2)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 2); s=s- 2;}\
		wk=(wk>>1)   ; if( (ULONG)x >= (ULONG)wk ){ x=(x>> 1); s=s- 1;}


/*--------------------------------------------------------------------------------------------------*/
/*		ASCII code table : 0 ~ ASCII code table up to F							*/
/*--------------------------------------------------------------------------------------------------*/
const	UCHAR	MlibAsciiTbl[ ] =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
/*--------------------------------------------------------------------------------------------------*/
/*		ASCII code table : 0 ~ ASCII code table with even parity up to F			*/
/*--------------------------------------------------------------------------------------------------*/
const	UCHAR	MlibEvenAsciiTbl[ ] =
		{ '0', '1'+0x80, '2'+0x80, '3', '4'+0x80, '5', '6', '7'+0x80,
		  '8'+0x80, '9', 'A', 'B', 'C'+0x80, 'D', 'E'+0x80, 'F'+0x80 };
/*--------------------------------------------------------------------------------------------------*/
/*		Decimal power table(Decimal Power Table)														*/
/*--------------------------------------------------------------------------------------------------*/
const	ULONG	MlibDecPowerTbl[10] =
		{ 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000	};
/*--------------------------------------------------------------------------------------------------*/


/****************************************************************************************************/
/*																									*/
/*		Unsigned 32bitDivision function(ASM division macro replacement)												*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		XasmDIVUA() : Unsigned 32bitDivision function(Round down)												*/
/*--------------------------------------------------------------------------------------------------*/
/*		1) Computation result : kx = (ax/bx)<<rep;															*/
/*		2) Calculation conditions : 0 < ax < bx <= 0x7FFFFFFF														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	XasmDIVUA( LONG ax, LONG bx, LONG rep )
{
LONG	i,kx;
		ax = (ax<<1);
		for( kx=0,i=0; i<(rep-1); i++ )
		{
			if( (ULONG)ax >= (ULONG)bx )
			{
				kx = kx + 1;
				ax = (ULONG)ax - (ULONG)bx;
			}
			kx = (kx<<1); ax = (ax<<1);
		}
		if( (ULONG)ax >= (ULONG)bx ){ kx = kx + 1;}
		return( kx );
}
/*--------------------------------------------------------------------------------------------------*/
/*		XasmDIVUB() : Unsigned 32bitDivision function(Rounding)												*/
/*--------------------------------------------------------------------------------------------------*/
/*		1) Computation result : kx = (ax/bx)<<rep;															*/
/*		2) Calculation conditions : 0 < ax < bx <= 0x7FFFFFFF														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	XasmDIVUB( LONG ax, LONG bx, LONG rep )
{
LONG	i,kx;
		ax = (ax<<1);
		for( kx=0,i=0; i<rep; i++ )
		{
			kx = (kx<<1);
			if( (ULONG)ax >= (ULONG)bx )
			{
				kx = kx + 1;
				ax = (ULONG)ax - (ULONG)bx;
			}
			ax = (ax<<1);
		}
		if( (ULONG)ax >= (ULONG)bx ){ kx = kx + 1;}
		return( kx );
}
/*--------------------------------------------------------------------------------------------------*/
/*		XasmDIVUXA() : Unsigned 32bitDivision function(Round down)											*/
/*--------------------------------------------------------------------------------------------------*/
/*		1) Computation result  : {kx,sx} = {(ax/bx)<<(rep-0/1), sx=sx+(rep-0/1)]};							*/
/*		2) Calculation conditions  : 0 < ax <= 0x7FFFFFFF, 0 < bx <= 0x7FFFFFFF, (ax/2) < bx < (2*ax)				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	XasmDIVUXA( LONG ax, LONG bx, LONG *sx, LONG rep )
{
LONG	i,kx;

		if( (ULONG)ax < (ULONG)bx )
		{
			ax = (ax<<1); *sx = *sx + 1;
		}
		for( kx=0,i=0; i<(rep-1); i++ )
		{
			if( (ULONG)ax >= (ULONG)bx )
			{
				kx = kx + 1;
				ax = (ULONG)ax - (ULONG)bx;
			}
			kx = (kx<<1); ax = (ax<<1);
		}
		if( (ULONG)ax >= (ULONG)bx ){ kx = kx + 1;}
		*sx = *sx + rep - 1;

		return( kx );
}
/*--------------------------------------------------------------------------------------------------*/
/*		XasmDIVUXB() : Unsigned 32bitDivision function(Rounding)											*/
/*--------------------------------------------------------------------------------------------------*/
/*		1) Computation result  : {kx,sx} = {(ax/bx)<<(rep-0/1), sx=sx+(rep-0/1)]};							*/
/*		2) Calculation conditions  : 0 < ax <= 0x7FFFFFFF, 0 < bx <= 0x7FFFFFFF, (ax/2) < bx < (2*ax)				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	XasmDIVUXB( LONG ax, LONG bx, LONG *sx, LONG rep )
{
LONG	i,kx;
		if( (ULONG)ax < (ULONG)bx )
		{
			ax = (ax<<1); *sx = *sx + 1;
		}
		for( kx=0,i=0; i<rep; i++ )
		{
			kx = (kx<<1);
			if( (ULONG)ax >= (ULONG)bx )
			{
				kx = kx + 1;
				ax = (ULONG)ax - (ULONG)bx;
			}
			ax = (ax<<1);
		}
		if( (ULONG)ax >= (ULONG)bx ){ kx = kx + 1;}
		*sx = *sx + rep - 1;
		return( kx );
}



/****************************************************************************************************/
/*																									*/
/*																									*/
/*		<<<<< Basic control calculation function >>>>>																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Upper/lower limit : rv = Limit value															*/
/*																									*/
/****************************************************************************************************/
LONG	MlibLimitul( LONG u, LONG ulim, LONG llim )
{

		if( u > ulim )
		{
			return( ulim );
		}
		if( u < llim )
		{
			return( llim );
		}
		return( u );

}
#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		Upper/lower limit : rv = Limit value (floating point)												*/
/*																									*/
/****************************************************************************************************/
float	FlibLimitul( float u, float ulim, float llim )
{

		if( u > ulim )
		{
			return( ulim );
		}
		if( u < llim )
		{
			return( llim );
		}
		return( u );

}
#endif /* FLOAT_USE */


/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(LimitStandard:2^24) : rv = (kx*u)>>sx;												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulgain( LONG u, LONG k )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		x = (xx[1] >> sx);
/*--------------------------------------------------------------------------------------------------*/
/*		Upper/lower limit check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( x > 0 )
		{
			return(  0x01000000 );
		}
		else if( x < -1 )
		{
			return( -0x01000000 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Edit gain multiplication result																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			return( (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1) );
		}

}



/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(Limit:Signed29bit) : rv = (kx*u)>>sx;											*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulgain29( LONG u, LONG k )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		x = ((xx[1]>>sx)>>4);
/*--------------------------------------------------------------------------------------------------*/
/*		Upper/lower limit check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( x > 0 )
		{
			return(  0x0FFFFFFF );
		}
		else if( x < -1 )
		{
			return( -0x10000000 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Edit gain multiplication result																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
			if( x > 0x0FFFFFFF ){ x = 0x0FFFFFFF;}
			return( x );
		}

}



/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(Limit:Signed30bit) : rv = (kx*u)>>sx;											*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulgain30( LONG u, LONG k )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		x = ((xx[1]>>sx)>>5);
/*--------------------------------------------------------------------------------------------------*/
/*		Upper/lower limit check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( x > 0 )
		{
			return(  0x1FFFFFFF );
		}
		else if( x < -1 )
		{
			return( -0x20000000 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Edit gain multiplication result																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
			if( x > 0x1FFFFFFF ){ x = 0x1FFFFFFF;}
			return( x );
		}

}

/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(Limit:Signed32bit) : rv = (kx*u)>>sx;											*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulgain32( LONG u, LONG k )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		x = ((xx[1]>>sx)>>7);
/*--------------------------------------------------------------------------------------------------*/
/*		Upper/lower limit check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( x > 0 )
		{
			return(  0x7FFFFFFF );
		}
		else if( x < -1 )
		{
			return(  0x80000000 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Edit gain multiplication result																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
			if( (xx[1]>=0)&&(x<0) ) return( 0x7fffffff );
			return( x );
		}

}

/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(Limit:Unsigned30bit) : rv = (kx*u)>>sx;											*/
/*																									*/
/****************************************************************************************************/
ULONG	MlibMulgain30U( ULONG u, LONG k )
{
ULONG	x;
ULONG	xx[2];
ULONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

		MULU32U32( u, kx, xx );
		x = ((xx[1]>>sx)>>5);

		if( x > 0 )
		{
			return(  0x1FFFFFFF );
		}
		else
		{
			x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
			if( x > 0x1FFFFFFF ){ x = 0x1FFFFFFF;}
			return( x );
		}
}

/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(Limit:Unsigned31bit) : rv = (kx*u)>>sx;											*/
/*																									*/
/****************************************************************************************************/
ULONG	MlibMulgain31U( ULONG u, LONG k )
{
ULONG	x;
ULONG	xx[2];
ULONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

		MULU32U32( u, kx, xx );
		x = ((xx[1]>>sx)>>6);

		if( x > 0 )
		{
			return(  0x3FFFFFFF );
		}
		else
		{
			x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
			if( x > 0x3FFFFFFF ){ x = 0x3FFFFFFF;}
			return( x );
		}
}

/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(Limit:Unsigned32bit) : rv = (kx*u)>>sx;											*/
/*																									*/
/****************************************************************************************************/
ULONG	MlibMulgain32U( ULONG u, LONG k )
{
ULONG	x;
ULONG	xx[2];
ULONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

		MULU32U32( u, kx, xx );
		x = ((xx[1]>>sx)>>7);

		if( x > 0 )
		{
			return(  0x7FFFFFFF );
		}
		else
		{
			x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
			if( x > 0x7FFFFFFF ){ x = 0x7FFFFFFF;}
			return( x );
		}
}

/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(smaxEnlarge:smax=48) : rv = (kx*u)>>sx;												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulgainSx48( LONG u, LONG k )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
/*--------------------------------------------------------------------------------------------------*/
/*		Exponent:sx <= 24																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( sx <= 24 )
		{
			if( (xx[1]>>sx) > 0 )
			{
				return(  0x01000000 );
			}
			else if( (xx[1]>>sx) < -1 )
			{
				return( -0x01000000 );
			}
			else
			{
				return( (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1) );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Exponent:sx > 24																				*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = (((xx[1]>>(sx-25))+1)>>1);
			if( x >= 0x01000000 )
			{
				return(  0x01000000 );
			}
			else if( x <= -0x01000000 )
			{
				return( -0x01000000 );
			}
			else
			{
				return( x );
			}
		}

}

/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(smaxEnlarge:smax=48) : rv = (kx*u)>>sx;												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulgain32Sx48( LONG u, LONG k )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
/*--------------------------------------------------------------------------------------------------*/
/*		Exponent:sx <= 24																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( sx <= 24 )
		{
			x = ((xx[1]>>sx)>>7);
			if( x > 0 )
			{
				return(  0x7fffffff );
			}
			else if( x < -1 )
			{
				return( 0x80000000 );
			}
			else
			{
				x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
				if( (xx[1]>=0)&&(x<0) ) return( 0x7fffffff );
				return( x );
			}
		}
		else
		{
			return( ((xx[1]>>(sx-25))+1)>>1 );
		}

}



/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(LimitNo capacity) : rv = (kx*u)>>sx;													*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulgainNolim( LONG u, LONG k )
{
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

		MUL3232( u, kx, xx );
		return( (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1) );

}



/****************************************************************************************************/
/*																									*/
/*		Integral calculation : iu=iu+(kx*u); rv=((iu[1]+1)>>1);													*/
/*																									*/
/****************************************************************************************************/
LONG	MlibIntegral( LONG u, LONG k, LONG iu[2] )
{
LONG	x;
LONG	xx[2];
LONG	carry;
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Input calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		x = (xx[1] >> sx);
/*--------------------------------------------------------------------------------------------------*/
/*		Integral calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( x > 0 )
		{
			iu[1] = iu[1] + (LONG)0x02000000;
		}
		else if( x < -1 )
		{
			iu[1] = iu[1] - (LONG)0x02000000;
		}
		else
		{
			carry = ( (ULONG)(iu[0]+(xx[0]<<(25-sx))) < (ULONG)iu[0] );
			iu[0] = iu[0] + (xx[0]<<(25-sx));
			iu[1] = iu[1] + (xx[1]<<(25-sx)) + (((ULONG)xx[0]>>sx)>>7) + carry;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Integral value limit																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( iu[1] >= (LONG)0x02000000 )
		{
			iu[0] = 0;
			iu[1] = 0x02000000;
		}
		else if( iu[1] < (LONG)-0x02000000 )
		{
			iu[0] =  0;
			iu[1] = -0x02000000;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Return value calculation(Rounding process)																	*/
/*--------------------------------------------------------------------------------------------------*/
		return( ((iu[1]+1)>>1) );

}



#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		Integral calculation : rv=iu; iu+=kx*u																	*/
/*																									*/
/****************************************************************************************************/
float	FlibIntegral( float u, float k, float *iu )
{
float ax;
float fwk1;											/*For forced convergence processingwork*/

//forDebugError investigation due to rounding	---->
#if 1//(Proposal2)
LONG	lwk_ax = 0;
float	fwk_iu;
#endif
//<----	forDebugError investigation due to rounding

//k = k / 16777216.0f; //todo

/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication(u*k)																					*/
/*--------------------------------------------------------------------------------------------------*/
		ax = (u * k);
//forDebugError investigation due to rounding	---->
#if 0	//(Proposal2)
		lwk_ax = (LONG)( ( ax * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		ax = (float)(lwk_ax) * ( 100.0 / 16777216.0 );	/* FloatConversion							*/
#endif
//<----	forDebugError investigation due to rounding

#if (FLOAT_USE==TRUE)
/*--------------------------------------------------------------------------------------------------*/
/*		Forced convergence processing(0Minimum operation processing in the vicinity)(Proposal1)														*/
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal1)In the case of----->
//The following process is unnecessary.??? ----> DoubleMake operation????[Temporary measures]
//Floatu=0,or,k=0when notax=0????
/*--------------------------------------------------------------------------------------------------*/
#if 0	// ---->
		if((u != (float)0.0) && ( k != (float)0.0) && (ax == (float)0.0))
		{
			if( u >= (float)0.0 )
			{
				if(k>(float)0.0)	ax = FLOAT_PULUS_MIN;
				if(k<(float)0.0)	ax = FLOAT_MINUS_MIN;
			}
			else
			{
				if(k>(float)0.0)	ax = FLOAT_MINUS_MIN;
				if(k<(float)0.0)	ax = FLOAT_PULUS_MIN;
			}
		}
#endif	/* #if 0	*/	// <----
//<-----(Proposal1)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal2)In the case of----->
//The following process is unnecessary.??? ----> DoubleMake operation????[Temporary measures]
//Floatu=0,or,k=0when notax=0????
/*--------------------------------------------------------------------------------------------------*/
#if 0	// ---->
/*--------------------------------------------------------------------------------------------------*/
/*		Integral calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		fwk1 = *iu;
		*iu = *iu + ax;
		if( ( fwk1 == *iu ) && ( ax != 0 ) )				/*If the value does not change after addition		*/
		{
			*iu = *iu + u;									/*Add integral value before integral gain multiplication		*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Integral value limit																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( *iu >= (float)INTGRAL_NORMAXVAL )
		{
			*iu = INTGRAL_NORMAXVAL;
		}
		else if( *iu < -((float)INTGRAL_NORMAXVAL) )
		{
			*iu =  - (INTGRAL_NORMAXVAL);
		}
/*--------------------------------------------------------------------------------------------------*/
		return( *iu );
#endif	/* #if 0	*/	// <----
//<-----(Proposal2)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal3)In the case of----->
/*--------------------------------------------------------------------------------------------------*/
#if 0	// ---->
/*--------------------------------------------------------------------------------------------------*/
/*		Integral calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		fwk1 = *iu;
		*iu = *iu + ax;
		if( ( fwk1 == *iu ) && ( ax != 0 ) )				/*If the value does not change after addition		*/
		{
			*iu = *iu + u * k * 1.2;						/*Integral value before integral gain multiplication*1.2	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Integral value limit																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( *iu >= (float)INTGRAL_NORMAXVAL )
		{
			*iu = INTGRAL_NORMAXVAL;
		}
		else if( *iu < -((float)INTGRAL_NORMAXVAL) )
		{
			*iu =  - (INTGRAL_NORMAXVAL);
		}
/*--------------------------------------------------------------------------------------------------*/
		return( *iu );
#endif	/* #if 0	*/	// <----
//<-----(Proposal3)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
/*		Integral calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		*iu = *iu + ax;
/*--------------------------------------------------------------------------------------------------*/
/*		Integral value limit																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( *iu >= (float)INTGRAL_NORMAXVAL )
		{
			*iu = INTGRAL_NORMAXVAL;
		}
		else if( *iu < -((float)INTGRAL_NORMAXVAL) )
		{
			*iu =  - (INTGRAL_NORMAXVAL);
		}
/*--------------------------------------------------------------------------------------------------*/
//forDebugError investigation due to rounding	---->
#if 0	//(Proposal2)
		if( *iu > 0)
		{
			lwk_ax = (LONG)( ( *iu * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		}
		if( *iu < 0)
		{
			lwk_ax = (LONG)( ( *iu * 167772.16 ) - 0.5 );		/* MLIBConversion								*/
		}
		fwk_iu = (float)(lwk_ax) * ( 100.0 / 16777216.0 );	/* FloatConversion							*/
		return( fwk_iu );
#endif
//<----	forDebugError investigation due to rounding
		return( *iu );

}
#endif /* FLOAT_USE */



/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(Positive/negative symmetric rounding) : rv = (kx*u)>>sx;												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibSymMulgain( LONG u, LONG k )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication : 32bit * 32bit --> 64bit														*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		x = (xx[1] >> sx);
/*--------------------------------------------------------------------------------------------------*/
/*		Upper/lower limit check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( x > 0 )
		{
			return(  0x01000000 );								/* Upper Limit						*/
		}
		else if( x < -1 )
		{
			return( -0x01000000 );								/* Lower Limit						*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Computation result editing : rc = (kx*u)>>sx;																*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			xx[1] = (xx[1] << (24-sx));							/* GetHiBits						*/
			xx[0] = (xx[0]==0)? xx[0] : (xx[0] + x);			/* Adjust RoundOff for Negative		*/
			xx[0] = ((((ULONG)xx[0]>>sx) + 0x0080)>>8);			/* GetLoBits and RoundOff			*/
			return( xx[1] + xx[0] );							/* HiBits + LoBits					*/
		}

}



/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(LimitNo capacity,Positive/negative symmetric rounding) : rv = (kx*u)>>sx;									*/
/*																									*/
/****************************************************************************************************/
LONG	MlibSymMulgainNolim( LONG u, LONG k )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

		MUL3232( u, kx, xx );									/* xx[] = kx * u					*/
		x = (xx[1] >> 31);										/* x = 0 or -1						*/
		xx[1] = (xx[1] << (24-sx));								/* GetHiBits						*/
		xx[0] = (xx[0]==0)? xx[0] : (xx[0] + x);				/* Adjust RoundOff for Negative		*/
		xx[0] = ((((ULONG)xx[0]>>sx) + 0x0080)>>8);				/* GetLoBits and RoundOff			*/
		return( xx[1] + xx[0] );								/* HiBits + LoBits					*/

}



/****************************************************************************************************/
/*																									*/
/*		Integral calculation(Positive/negative symmetric rounding) : iu=iu+(kx*u); rv=((iu[1]+x)>>1);								*/
/*																									*/
/****************************************************************************************************/
LONG	MlibSymIntegral( LONG u, LONG k, LONG iu[2] )
{
LONG	x;
LONG	xx[2];
LONG	carry;
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Input calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		x = (xx[1] >> sx);
/*--------------------------------------------------------------------------------------------------*/
/*		Integral calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( x > 0 )
		{
			iu[1] = iu[1] + (LONG)0x02000000;
		}
		else if( x < -1 )
		{
			iu[1] = iu[1] - (LONG)0x02000000;
		}
		else
		{
			carry = ( (ULONG)(iu[0]+(xx[0]<<(25-sx))) < (ULONG)iu[0] );
			iu[0] = iu[0] + (xx[0]<<(25-sx));
			iu[1] = iu[1] + (xx[1]<<(25-sx)) + (((ULONG)xx[0]>>sx)>>7) + carry;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Integral value limit																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( iu[1] >= (LONG)0x02000000 )
		{
			iu[0] = 0;
			iu[1] = 0x02000000;
		}
		else if( iu[1] < (LONG)-0x02000000 )
		{
			iu[0] =  0;
			iu[1] = -0x02000000;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Return value calculation(Positive/negative symmetric rounding process)															*/
/*--------------------------------------------------------------------------------------------------*/
		x = (iu[1] >= 0)? 1 : ((iu[0] != 0)? 1 : 0);
		return( ((iu[1]+x)>>1) );

}



/****************************************************************************************************/
/*																									*/
/*		Position FB calculation with remainder(For MFC) : rv = (kx*u+pfbrem)>>sx;									*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPfbkxremLim( LONG u, LONG k, LONG *pfbrem )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = ((ULONG)k)>>24;
LONG	remx = *pfbrem;

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );

/*--------------------------------------------------------------------------------------------------*/
/*		Scaling processing:sx <= 24																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( sx <= 24 )
		{
			if( (xx[1]>>sx) > 0 )
			{
				*pfbrem = 0;
				return(  0x01000000 );
			}
			else if( (xx[1]>>sx) < -1 )
			{
				*pfbrem = 0;
				return( -0x01000000 );
			}
			else
			{
				x = (xx[1]<<(24-sx)) + ((((ULONG)xx[0])>>sx)>>8);
				remx = remx + (((ULONG)xx[0]<<(24-sx))>>8);
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Scaling processing:sx > 24																	*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = xx[1]>>(sx-24);
			if( x >= 0x01000000 )
			{
				*pfbrem = 0;
				return(  0x01000000 );
			}
			else if( x <= -0x01000000 )
			{
				*pfbrem = 0;
				return( -0x01000000 );
			}
			else
			{
				remx = remx + (((ULONG)xx[1]<<(56-sx))>>8) + (((((ULONG)xx[0]>>(sx-24))>>7)+1)>>1);
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Remainder processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( remx >= (LONG)0x00800000 )
		{
			x = x + 1;
			remx = remx - (LONG)0x00800000;
			remx = remx - (LONG)0x00800000;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		*pfbrem = remx;
		return( x );

}



/****************************************************************************************************/
/*																									*/
/*		Position FB calculation with remainder(For MFC) : rv = (kx*u+pfbrem)>>sx;									*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPfbkxremNolim( LONG u, LONG k, LONG *pfbrem )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = ((ULONG)k)>>24;
LONG	remx = *pfbrem;

/*--------------------------------------------------------------------------------------------------*/
/*		Gain multiplication processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );

/*--------------------------------------------------------------------------------------------------*/
/*		Scaling processing:sx <= 24																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( sx <= 24 )
		{
			x = (xx[1]<<(24-sx)) + ((((ULONG)xx[0])>>sx)>>8);
			remx = remx + (((ULONG)xx[0]<<(24-sx))>>8);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Scaling processing:sx > 24																	*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = xx[1]>>(sx-24);
			remx = remx + (((ULONG)xx[1]<<(56-sx))>>8) + (((((ULONG)xx[0]>>(sx-24))>>7)+1)>>1);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Remainder processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( remx >= (LONG)0x00800000 )
		{
			x = x + 1;
			remx = remx - (LONG)0x00800000;
			remx = remx - (LONG)0x00800000;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		*pfbrem = remx;
		return( x );

}



/****************************************************************************************************/
/*																									*/
/*		Position gain multiplication with remainder(For MFC) : rv=(kx*u-((kx*pfbrem)>>24))>>sx;							*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulkprem( LONG u, LONG k, LONG pfbrem )
{
LONG	x;
LONG	xx1[2];
LONG	xx2[2];
LONG	carry;
LONG	kx = (k<<8);
LONG	sx = ((ULONG)k)>>24;

		MUL3232( u,      kx, xx1 );
		MUL3232( pfbrem, kx, xx2 );

		x = (xx2[1]<<8) + ((ULONG)xx2[0]>>24);
		carry  = ( (ULONG)xx1[0] < (ULONG)x );
		xx1[0] = (ULONG)xx1[0] - (ULONG)x;
		xx1[1] = xx1[1] - (xx2[1]>>24) - carry;

		if( (xx1[1]>>sx) > 0 )
		{
			return(  0x01000000 );
		}
		else if( (xx1[1]>>sx) < -1 )
		{
			return( -0x01000000 );
		}
		else
		{
			return( (xx1[1]<<(24-sx)) + (((((ULONG)xx1[0]>>sx)>>7)+1)>>1) );
		}

}



/****************************************************************************************************/
/*																									*/
/*		Gain multiplication with bias remainder																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		2004.04.08 : T.Taniguchi : Remainder processing correction														*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulkxBiasrem( LONG u, LONG k, LONG *rem, LONG bias )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);
LONG	kb = (bias<<8);
LONG	sb = (((ULONG)bias)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication processing with remainder																			*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		*rem = *rem + ((ULONG)(xx[0]<<(24-sx))>>8);
		x = (xx[1]<<(24-sx)) + (((ULONG)xx[0]>>sx)>>8);
		if( *rem >= 0x00800000 )
		{
			x = x + 1;
			*rem = *rem - 0x01000000;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Bias addition processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( u >= 0 )
		{
			x = x + (((ULONG)kb>>sb)>>8);
			*rem = *rem + ((ULONG)(kb<<(24-sb))>>8);
			if( *rem >= 0x00800000 )
			{
				x = x + 1;
				*rem = *rem - 0x01000000;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Bias subtraction processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = x - (((ULONG)kb>>sb)>>8);
			*rem = *rem - ((ULONG)(kb<<(24-sb))>>8);
			if( *rem < -0x00800000 )
			{
				x = x - 1;
				*rem = *rem + 0x01000000;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( x );

}



/****************************************************************************************************/
/*																									*/
/*		Gain multiplication with bias remainder																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Remainder processing bug 1 : Since there are two remainder additions, one-time remainder processing is insufficient							*/
/*		Remainder processing bug 2 : Remainder processing after remainder subtraction requires checking of the negative remainder							*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulkxBiasremOld( LONG u, LONG k, LONG *rem, LONG bias )
{
LONG	x;
LONG	xx[2];
LONG	kx = (k<<8);
LONG	sx = (((ULONG)k)>>24);
LONG	kb = (bias<<8);
LONG	sb = (((ULONG)bias)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication processing with remainder																			*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
		*rem = *rem + ((ULONG)(xx[0]<<(24-sx))>>8);
		x = (xx[1]<<(24-sx)) + (((ULONG)xx[0]>>sx)>>8);
/*--------------------------------------------------------------------------------------------------*/
/*		Bias addition processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( u >= 0 )
		{
			x = x + (((ULONG)kb>>sb)>>8);
			*rem = *rem + ((ULONG)(kb<<(24-sb))>>8);
			if( *rem >= 0x00800000 )
			{
				x = x + 1;
				*rem = *rem - 0x00800000;
				*rem = *rem - 0x00800000;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Bias subtraction processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = x - (((ULONG)kb>>sb)>>8);
			*rem = *rem - ((ULONG)(kb<<(24-sb))>>8);
			if( *rem >= 0x00800000 )
			{
				x = x + 1;
				*rem = *rem - 0x00800000;
				*rem = *rem - 0x00800000;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( x );

}



/****************************************************************************************************/
/*																									*/
/*																									*/
/*		<<<<< Filter calculation function >>>>>																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Low pass filter : rv = x + ((kx*(u-x))>>24);												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibLpfilter1( LONG u, LONG k, LONG x )
{
LONG	wk1;
LONG	xx[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Gain check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( k == 0 )
		{
			return( u );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Filter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( (u-x), k, xx );
		wk1 = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1);
/*--------------------------------------------------------------------------------------------------*/
		if( wk1 == 0 )
		{
			return( x + ((u-x)>0) - ((u-x)<0) );
		}
		else
		{
			return( x + wk1 );
		}

}



#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		Low pass filter : rv = x + (kx*(u-x));														*/
/*																									*/
/****************************************************************************************************/
float	FlibLpfilter1( float u, float k, float x )
{

float	ax;
float	wk1;
float	x_wk;

//forDebugError investigation due to rounding	---->
#if 0	//(Proposal2,3)
LONG	lwk_wk1 = 0;
float	fwk_u;
#endif
#if 0	//(Proposal4)
LONG	lwk_u;
LONG	lwk_x;
#endif

//<----	forDebugError investigation due to rounding

/*--------------------------------------------------------------------------------------------------*/
/*		Gain check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( k == (float)0.0 )
		{
			return( u );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Filter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
//forDebugError investigation due to rounding	---->
#if 0	//(Proposal4)
		lwk_u = (LONG)( ( u * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		u = (float)(lwk_u) * ( 100.0 / 16777216.0 );	/* FloatConversion						*/
		lwk_x = (LONG)( ( x * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		x = (float)(lwk_x) * ( 100.0 / 16777216.0 );	/* FloatConversion						*/
#endif
//<----	forDebugError investigation due to rounding
		ax = ( u - x );
		wk1 = ( ax * k );
//forDebugError investigation due to rounding	---->
#if 0	//(Proposal3)
		if (wk1 > 0)
		{
			lwk_wk1 = (LONG)( ( wk1 * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		}
		if (wk1 < 0)
		{
			lwk_wk1 = (LONG)( ( wk1 * 167772.16 ) - 0.5 );		/* MLIBConversion								*/
		}
		if( ( lwk_wk1 == 0 ) && ( ax != 0 ) )
		{
			if( ax > 0 )
			{
				x = x + ( 100.0 / 16777216.0 );
			}
			else if( ax < 0 )
			{
				x = x - ( 100.0 / 16777216.0 );
			}
			fwk_u = u;							//(Proposal3.1)
			u = ( u - x );
			if( (u * ax) < 0 )		//(Proposal3.1)
			{									//(Proposal3.1)
				x = fwk_u;						//(Proposal3.1)
//				u = ax;
			}									//(Proposal3.1)
			return ( x );
		}
		else
		{
			wk1 = (float)( lwk_wk1 ) * ( 100.0 / 16777216.0 );	/* FloatConversion						*/
		}
#endif
//<----	forDebugError investigation due to rounding

/*--------------------------------------------------------------------------------------------------*/
/*		Forced convergence processing																				*/
/*--------------------------------------------------------------------------------------------------*/
//Forced convergence processing not required??? ----> DoubleMake operation????[Temporary measures]
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal1)In the case of----->
//FloatbutDoubleax=0,or,k=0when notwk1=0????
#if 0	// ---->
		if ( (wk1 == 0) && (ax !=0) )
		{
			if ( (u-x) > 0)
			{
//				wk1 = FLOAT_PULUS_MIN;
				wk1 = 5.9604644e-08;
				x = ( x + wk1 );
				if ( u < x)
				{
					x = u;
				}
			}
			else
			{
				if ( (u-x) < 0)
				{
//					wk1 = FLOAT_MINUS_MIN;
					wk1 = -5.9604644e-08;
					x = ( x + wk1 );
					if ( u > x)
					{
						x = u;
					}
				}
			}
		}
		else
		{
			x = ( x + wk1 );
		}
		return( x );
#endif	/* #if 0	*/	// <----
//<-----(Proposal1)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal2)In the case of----->
//The following does not seem to have any effect??? For now, comment out
#if 0	// ---->
//#if 1	// ---->
		x_wk = x;
		x = ( x + wk1 );
		if ( ( x_wk == x) && (ax != 0) )
		{
			if ( ax > 0)	/* ax = (u - x) */
			{
//				wk1 = FLOAT_PULUS_MIN;
//				wk1 = 5.9604644e-08;
				wk1 = 5.9604644e-02;
				x = ( x + wk1 );
				if ( u < x)
				{
					x = u;
				}
			}
			else
			{
				if ( ax < 0)	/* ax = (u - x) */
				{
//					wk1 = FLOAT_MINUS_MIN;
//					wk1 = -5.9604644e-08;
					wk1 = -5.9604644e-02;
					x = ( x + wk1 );
					if ( u > x)
					{
						x = u;
					}
				}
			}
		}
		return( x );
#endif	/* #if 0	*/	// <----
//<-----(Proposal2)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal3)In the case of----->
//The following does not seem to have any effect??? For now, comment out
//#if 1	// ---->
#if 0	// ---->
		x_wk = x;
		x = ( x + wk1 );
		if ( x_wk == x)
		{
			x = u;
		}
		return( x );
#endif	/* #if 0	*/	// <----
//<-----(Proposal3)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal4)In the case of----->
//x_wk == x, so it will be an unnecessary comment out
//#if 1	// ---->
#if 0	// ---->
		x_wk = x;
		x = ( x + wk1 );
		if ( ( x_wk == x) && (ax != 0) )
		{
			if ( ax > 0)	/* ax = (u - x) */
			{
//				x = ( x + ( ax / 10 ) );
				x = ( x + ax / 3);
				if ( u < x )
				{
					x = u;
				}
			}
			else			// ax < 0 
			{
//				x = ( x + ( ax / 10 ) );
				x = ( x + ax / 3);
				if ( u > x )
				{
					x = u;
				}
			}
		}
		return( x );
#endif	/* #if 0 or 1	*/	// <----
//<-----(Proposal4)In the case of-----
/*--------------------------------------------------------------------------------------------------*/

		x = ( x + wk1 );
//forDebugError investigation due to rounding	---->
#if 0	//(Proposal2)
		lwk_wk1 = (LONG)( ( x * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		x = (float)(lwk_wk1) * ( 100.0 / 16777216.0 );	/* FloatConversion							*/
#endif
//<----	forDebugError investigation due to rounding
		return( x );

}
#endif /* FLOAT_USE */




/****************************************************************************************************/
/*																									*/
/*		High-pass filter : rv=u-z; z=z+((kx*(u-z))>>24);											*/
/*																									*/
/****************************************************************************************************/
LONG	MlibHpfilter1( LONG u, LONG k, LONG *z )
{
LONG	wk1;
LONG	xx[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Gain check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( k == 0 )
		{
			*z = 0;
			return( u );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Filter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( (u-*z), k, xx );
		wk1 = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1);
/*--------------------------------------------------------------------------------------------------*/
		if( wk1 == 0 )
		{
			*z = *z + ((u-*z)>0) - ((u-*z)<0);
		}
		else
		{
			*z = *z + wk1;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( u - *z );

}


#if (FLOAT_USE==TRUE)
#if 0
/****************************************************************************************************/
/*																									*/
/*		High-pass filter : rv=u-z; z=z+(kx*(u-z));												*/
/*																									*/
/****************************************************************************************************/
float	FlibHpfilter1( float u, float k, float *z )
{

float ax;
float	wk1;
//Forced convergence processing variables
float	z_wk;											

//forDebugError investigation due to rounding	---->
#if 0	//(Proposal1)Does not operate
LONG	lwk_u = u * 16777216.0 / 100.0;							/* MLIBConversion							*/
LONG	lwk_k = k * 16777216.0 / 100.0;							/* MLIBConversion							*/
LONG	lwk_z = *z * 16777216.0 / 100.0;						/* MLIBConversion							*/
LONG	lwk_wk1;
LONG	xx[2];
#endif
#if 1	//(Proposal2,3)
LONG	lwk_wk1;
float	fwk_u;
float	fwk_z;
#endif
#if 1	//(Proposal4)
LONG	lwk_u;
LONG	lwk_z;
#endif
//<----	forDebugError investigation due to rounding

/*--------------------------------------------------------------------------------------------------*/
/*		Gain check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( k == (float)0.0 )
		{
			*z = 0;
			return( u );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Filter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
//forDebugError investigation due to rounding	---->
#if 0	//(Proposal4)
		lwk_u = (LONG)( ( u * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		u = (float)(lwk_u) * ( 100.0 / 16777216.0 );	/* FloatConversion						*/
		lwk_z = (LONG)( ( *z * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		*z = (float)(lwk_z) * ( 100.0 / 16777216.0 );	/* FloatConversion						*/
#endif
//<----	forDebugError investigation due to rounding
		ax = (u -*z);
		wk1 = (ax * k);

//forDebugError investigation due to rounding	---->
#if 0	//(Proposal1)Does not operate
/*--------------------------------------------------------------------------------------------------*/
/*		Filter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( (lwk_u-lwk_z), lwk_k, xx );
		lwk_wk1 = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1);
/*--------------------------------------------------------------------------------------------------*/
		if( wk1 == 0 )
		{
			lwk_z = lwk_z + ((lwk_u-lwk_z)>0) - ((lwk_u-lwk_z)<0);
		}
		else
		{
			lwk_z = lwk_z + lwk_wk1;
		}
/*--------------------------------------------------------------------------------------------------*/
		*z = (float)lwk_z * 100.0 / 16777216.0;			/* FloatConversion							*/
		u = ( u - *z );
		return( u );
#endif
#if 0	//(Proposal2)
		lwk_wk1 = (LONG)( ( wk1 * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		wk1 = (float)(lwk_wk1) * ( 100.0 / 16777216.0 );	/* FloatConversion							*/
#endif
#if 0	//(Proposal3)
		lwk_wk1 = (LONG)( ( wk1 * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		if(( lwk_wk1 == 0 ) && ( ax != 0))
		{
			fwk_z = *z;							//(Proposal3.1)
			if( ( u - *z ) > 0)
			{
				*z = *z + ( 100.0 / 16777216.0 );
			}
			else if( ( u - *z ) < 0)
			{
				*z = *z - ( 100.0 / 16777216.0 );
			}
			fwk_u = u;							//(Proposal3.1)
			u = ( u - *z );
			if( u * ( fwk_u - fwk_z ) < 0 )		//(Proposal3.1)
			{									//(Proposal3.1)
				*z = fwk_u;						//(Proposal3.1)
				u = ax;
			}									//(Proposal3.1)
			return( u );
		}
		else
		{
			wk1 = (float)(lwk_wk1) * ( 100.0 / 16777216.0 );	/* FloatConversion						*/
		}
#endif
//<----	forDebugError investigation due to rounding

/*--------------------------------------------------------------------------------------------------*/
/*		Forced convergence processing																				*/
/*--------------------------------------------------------------------------------------------------*/
//Forced convergence processing not required??? ----> DoubleMake operation????[Temporary measures]
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal1)In the case of----->
//FloatbutDoubleax=0,or,k=0when notwk1=0????
#if 0	// ---->
		z_wk = *z;
		if ( (wk1 == 0) && (ax !=0) )
		{
			if ( (u-*z) > 0)
			{
//				wk1 = FLOAT_PULUS_MIN;
				wk1 = 5.9604644e-08;
				*z = *z + wk1;
				if ( u < *z)
				{
					*z = u;
				}
			}
			else
			{
				if ( (u-*z) < 0)
				{
//					wk1 = FLOAT_MINUS_MIN;
					wk1 = -5.9604644e-08;
					*z = ( *z + wk1 );
					if ( u > *z)
					{
						*z = u;
					}
				}
			}
		}
		else
		{
			*z = *z + wk1;
		}
		u = ( u - *z );
		return( u );
#endif	/* #if 0	*/	// <----
//<-----(Proposal1)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal2)In the case of----->
//The following does not seem to have any effect??? For now, comment out
#if 0	// ---->
		z_wk = *z;
		*z = *z + wk1;
		if ( ( z_wk == *z) && (ax != 0) )
		{
			if ( ax > 0)	/* ax = (u -*z) */
			{
//				wk1 = FLOAT_PULUS_MIN;
//				wk1 = 5.9604644e-08;
				wk1 = 5.9604644e-02;
				*z = *z + wk1;
				if ( u < *z)
				{
					*z = u;
				}
			}
			else
			{
				if ( ax < 0)	/* ax = (u -*z) */
				{
//					wk1 = FLOAT_MINUS_MIN;
//					wk1 = -5.9604644e-08;
					wk1 = -5.9604644e-02;
					*z = ( *z + wk1 );
					if ( u > *z)
					{
						*z = u;
					}
				}
			}
		}
		u = ( u - *z );
		return( u );
#endif	/* #if 0	*/	// <----
//<-----(Proposal2)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal3)In the case of----->
//The following does not seem to have any effect??? For now, comment out
//#if 1	// ---->
#if 0	// ---->
		z_wk = *z;
		*z = *z + wk1;
		if ( z_wk == *z)
		{
			*z = u;
		}
		u = ( u - *z );
		return( u );
#endif	/* #if 0	*/	// <----
//<-----(Proposal3)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal4)In the case of----->
//x_wk == x, so it will be an unnecessary comment out
//#if 1	// ---->
#if 0	// ---->
		z_wk = *z;
		*z = *z + wk1;
		if ( ( z_wk == *z ) && ( ax != 0 ) )
		{
			if ( ax > 0 )	/* ax = (u -*z) */
			{
//				*z = *z + ( ax / 10 ) ;
				*z = *z + ax/3;
				if ( u < *z )
				{
					*z = u;
				}
			}
			else			// ax < 0 
			{
//				*z = *z + ( ax / 10 ) ;
				*z = *z + ax/3;
				if ( u > *z )
				{
					*z = u;
				}
			}
		}
		u = ( u - *z );
		return( u );
#endif	/* #if 0 or 1	*/	// <----
//<-----(Proposal4)In the case of-----
/*--------------------------------------------------------------------------------------------------*/
//-----(Proposal5)In the case of----->
//x_wk == x, so it will be an unnecessary comment out
//#if 1	// ---->
#if 0	// ---->
		z_wk = *z;
		*z = *z + wk1;
		if ( ( z_wk == *z ) && ( ax != 0 ) )
		{
			if ( ax > 0 )	/* ax = (u -*z) */
			{
				*z = *z + (ax * k * 1.2);
				if ( u < *z )
				{
					*z = u;
				}
			}
			else			// ax < 0 
			{
				*z = *z + (ax * k * 1.2);
				if ( u > *z )
				{
					*z = u;
				}
			}
		}
		u = ( u - *z );
		return( u );
#endif	/* #if 0 or 1	*/	// <----
//<-----(Proposal5)In the case of-----
/*--------------------------------------------------------------------------------------------------*/

		*z = *z + wk1;
		u = ( u - *z );
//forDebugError investigation due to rounding	---->
#if 0	//(Proposal2)
		lwk_wk1 = (LONG)( ( u * 167772.16 ) + 0.5 );		/* MLIBConversion								*/
		u = (float)(lwk_wk1) * ( 100.0 / 16777216.0 );	/* FloatConversion							*/
#endif
//<----	forDebugError investigation due to rounding
		return( u );

}
#else
/****************************************************************************************************/
/*																									*/
/*		High-pass filter : rv=u-z; z=z+(kx*(u-z));												*/
/*																									*/
/****************************************************************************************************/
float	FlibHpfilter1( float u, float k, float *z )
{

		*z = FlibLpfilter1( u, k, *z );
		u = u - *z;

		return( u );

}
#endif
#endif /* FLOAT_USE */



/****************************************************************************************************/
/*																									*/
/*		Secondary low-pass filter : rv=(z[2]>>1); z[]=f(u,k[],z[]);										*/
/*																									*/
/****************************************************************************************************/
LONG	MlibLpfilter2( LONG u, LONG k[2], LONG z[3] )
{
LONG	wk1;
LONG	wk2;
LONG	xx[2];
LONG	carry;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Gain check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( k[0] == 0 )
		{
			z[0] = 0;
			z[1] = 0;
			z[2] = 0;
			return( u );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Filter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
		wk1 = u - ((z[2]+1)>>1) - z[0];
		MUL3232( wk1, k[0], xx );
		wk2 = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1);
		if( wk2==0 )
		{
			z[0] = z[0] + (wk1>0) - (wk1<0);
		}
		else
		{
			z[0] = z[0] + wk2;
		}
/*--------------------------------------------------------------------------------------------------*/
		kx = (k[1]<<8);
		sx = (((ULONG)k[1])>>24);
		MUL3232( z[0], kx, xx );
		carry = ( (ULONG)(z[1]+(xx[0]<<(25-sx))) < (ULONG)z[1] );
		z[1] = z[1] + (xx[0]<<(25-sx));
		z[2] = z[2] + (xx[1]<<(25-sx)) + (((ULONG)xx[0]>>sx)>>7) + carry;
/*--------------------------------------------------------------------------------------------------*/
		return( ((z[2]+1)>>1) );

}



/****************************************************************************************************/
/*																									*/
/*		2nd order notch filter(k[2]!=0)   :	rv=k[2]*(u-(z[2]>>1)-z[0])+(z[2]>>1); z[]=f(u,k[],z[]);		*/
/*		Secondary low-pass filter(k[2]==0) : rv=k[2]*(u-(z[2]>>1)-z[0])+(z[2]>>1); z[]=f(u,k[],z[]);		*/
/*																									*/
/****************************************************************************************************/
LONG	MlibNxfilter2( LONG u, LONG k[3], LONG z[3] )
{
LONG	wk1;
LONG	wk2;
LONG	xx[2];
LONG	carry;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Gain check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( k[0] == 0 )
		{
			z[0] = 0;
			z[1] = 0;
			z[2] = 0;
			return( u );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Filter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
		wk1 = u - ((z[2]+1)>>1) - z[0];
		MUL3232( wk1, k[0], xx );
		wk2 = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1);
		if( wk2==0 )
		{
			z[0] = z[0] + (wk1>0) - (wk1<0);
		}
		else
		{
			z[0] = z[0] + wk2;
		}
/*--------------------------------------------------------------------------------------------------*/
		kx = (k[1]<<8);
		sx = (((ULONG)k[1])>>24);
		MUL3232( z[0], kx, xx );
		carry = ( (ULONG)(z[1]+(xx[0]<<(25-sx))) < (ULONG)z[1] );
		z[1] = z[1] + (xx[0]<<(25-sx));
		z[2] = z[2] + (xx[1]<<(25-sx)) + (((ULONG)xx[0]>>sx)>>7) + carry;
/*--------------------------------------------------------------------------------------------------*/
		wk1 = ((z[2]+1)>>1);
		MUL3232( u-wk1-z[0], k[2], xx );
		wk2 = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1) + wk1;
/*--------------------------------------------------------------------------------------------------*/
		return( wk2 );
//		return( u - z[0] );

}



/****************************************************************************************************/
/*																									*/
/*		Secondary low-pass filter(Bilinear transformation)															*/
/*																									*/
/****************************************************************************************************/
LONG	MlibLowPassfilter2( LONG u, LONG k[4], LONG z[4] )
{
	
		LONG	x[6];

		x[0] = MlibMulgain29( u,    k[0] );
		x[1] = MlibMulgain29( z[0], k[3] );
		x[2] = MlibMulgain29( z[1], k[0] );
		x[3] = MlibMulgain29( z[2], k[1] );
		x[4] = MlibMulgain29( z[3], k[2] );


		x[5] = x[0] + x[1] + x[2] + x[3] - x[4];
/*--------------------------------------------------------------------------------------------------*/
		z[3] = z[2];
		z[2] = x[5];
		z[1] = z[0];
		z[0] = u;

		return( x[5] );
}


#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		Secondary low-pass filter(Bilinear transformation)															*/
/*																									*/
/****************************************************************************************************/
float	FlibLowPassfilter2( float u, float k[4], float z[4] )
{
float	x[6];

		x[0] = u * k[0];
		x[1] = z[0] * k[3];
		x[2] = z[1] * k[0];
		x[3] = z[2] * k[1];
		x[4] = z[3] * k[2];
		x[5] = x[0] + x[1] + x[2] + x[3] - x[4];
/*--------------------------------------------------------------------------------------------------*/
		z[3] = z[2];
		z[2] = x[5];
		z[1] = z[0];
		z[0] = u;

		return( x[5] );

}
#endif /* FLOAT_USE */



/****************************************************************************************************/
/*																									*/
/*		Linear accelerator(LAU)Filter : rv = z + dz or rv = z - dz;										*/
/*																									*/
/****************************************************************************************************/
LONG	MlibLaufilter(
		LONG	u,						/* Input														*/
		LONG	z,						/* Output previous value												*/
		LONG	dz )					/* Output increase/decrease												*/
{
LONG	x;

		if( dz == 0 )					/* If the output increase/decrease is zero,								*/
		{								/* LAU filter not used(Output = input)only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.					*/
			x = u;
		}
		else if( u > z )				/* Input > Output previous value In the case of									*/
		{
			x = z + dz;
			if( x > u ){ x = u;}
		}
		else							/* Input <= Output previous value In the case of								*/
		{
			x = z - dz;
			if( x < u ){ x = u;}
		}
		return( x );

}



/****************************************************************************************************/
/*																									*/
/*																									*/
/*		<<<<< Control parameter calculation function >>>>>														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Proportional gain calculation : {kx,sx} = (a*b/c)<<sx;														*/
/*																									*/
/****************************************************************************************************/
#pragma arm section code = "MlibPcal"
LONG	MlibPcalKxgain( LONG a, LONG b, LONG c, LONG *psx, LONG smax )
{
LONG	wk;
LONG	kx;
LONG	sx;
LONG	sy;
LONG	sign;
LONG	x,xx[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Input processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		sx = (psx != NULL)? (*psx) : 0;
/*--------------------------------------------------------------------------------------------------*/
/*		Zero input check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( (a==0)||(b==0)||(c==0) )
		{
			if( psx != NULL ){ *psx = 0;}
			return( 0 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Sign processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		sign = 0;
		if( a < 0 ){ a=-a; sign=sign^0x01;}
		if( b < 0 ){ b=-b; sign=sign^0x01;}
		if( c < 0 ){ c=-c; sign=sign^0x01;}
/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( (ULONG)a, (ULONG)b, (ULONG*)xx );
/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication result scaling : 0x40000000 <= x < 0x80000000											*/
/*--------------------------------------------------------------------------------------------------*/
		if( xx[1] > 0 )
		{
			sy = 32;
			x  = xx[1];
			SCUP31_SXDEC( x, sy, wk );
			x = x + (((ULONG)xx[0])>>(sy));
			sx = sx - sy;
		}
		else if( xx[0] > 0 )
		{
			x  = xx[0];
			SCUP31_SXINC( x, sx, wk );
		}
		else
		{
			sx = sx - 1;
			x  = (((ULONG)xx[0])>>1);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Division input scaling : 0x40000000 <= c < 0x80000000											*/
/*--------------------------------------------------------------------------------------------------*/
		SCUP31_SXDEC( c, sx, wk );
/*--------------------------------------------------------------------------------------------------*/
/*		Division : kx = x/c	(32bit/32bit,Resolution:24bit)													*/
/*--------------------------------------------------------------------------------------------------*/
		kx = XasmDIVUXA( x, c, &sx, 24 );
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( smax == 0 )									/* Continuation of calculation									*/
		{
			if( sign ){ kx = -kx;}						/* Sign processing									*/
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( smax > 0 )								/* Save final calculation result							*/
		{
			kx = kx>>1; sx = sx - 1;					/* kx : 24bit --> 23bit						*/
			if( (sx-smax)>31 ){ kx = 0;}				/* For very small values						*/
			if(  sx < 0      ){ kx=0x007FFFFF; sx=0;}	/* At maximum value(2^23-1),				*/
			if(  sx > smax   ){ kx=(kx>>(sx-smax)); sx=smax;}	/* smax,					*/
			if(  sign != 0   ){ kx = -kx;}				/* Sign processing									*/
			kx = (sx<<24) + (kx&0x00FFFFFF);			/* Computation result synthesis processing							*/
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( smax == -1 )							/* Intermediate calculation result storage							*/
		{
			kx = kx>>1; sx = sx - 1;					/* kx : 24bit --> 23bit						*/
			if( (sx > 127) || (sx < -128) )				/* sxCheck								*/
			{
				kx = sx = 0;							/* As a precaution, set to zero in case of abnormality				*/
			}
			if( sign != 0 ){ kx = -kx;}					/* Sign processing									*/
			kx = (sx<<24) + (kx&0x00FFFFFF);			/* Computation result synthesis processing							*/
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( (smax == -2) && CCSW_HsMulgain )		/* Fixed point gain(16bit.16bit)			*/
		{												/* for MlibHsMulgain, MlibHsIntegral,etc	*/
			sx = sx - 16;
			if( sx == 0 )      { /*kx = kx*/;        }		/* If the index is zero							*/
			else if( sx >= 24 ){ kx = 0;         }		/* For very small values						*/
			else if( sx <= -8 ){ kx = 0x7FFFFFFF;}		/* Maximum value(2^31-1),					*/
			else if( sx >= 0  ){ kx = kx>>sx;    }		/* Shift & truncation							*/
			else               { kx = kx<<(-sx); }		/* Shift & zero padding							*/
			sx = 16;									/* Fixed exponent(sx=16)							*/
			if( sign != 0 ){ kx = -kx;}					/* Sign processing									*/
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( smax == -24 )							/* LONGSave type calculation results(Rounding)				*/
		{
			if( sx < 0 )      { kx = 0x01000000;}		/* Maximum value(2^24),					*/
			else if( sx > 32 ){	kx = 0;         }		/* For very small values						*/
			else if( sx == 0 ){	/*kx = kx*/; 		}		/* If the index is zero							*/
			else   { kx = (((kx>>(sx-1))+1)>>1);}		/* Shift & round off							*/
			sx = 0;										/* Zero index(sx=0)							*/
			if( sign != 0  ){ kx = -kx;}				/* Sign processing									*/
		}
/*--------------------------------------------------------------------------------------------------*/
		else											/* LONGSave type calculation results(Rounding)				*/
		{
			kx = kx << 6; sx = sx + 6;
			if( sx < 0 )      { kx = 0x40000000;}   	/* Maximum value(2^30),					*/
			else if( sx > 32 ){	kx = 0;         }   	/* For very small values						*/
			else if( sx == 0 ){	/*kx = kx*/; 		}   	/* If the index is zero							*/
			else   { kx = (((kx>>(sx-1))+1)>>1);}		/* Shift & round off							*/
			sx = 0;										/* Zero index(sx=0)							*/
			if( sign != 0  ){ kx = -kx;}				/* Sign processing									*/
		}
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL ){ *psx = sx;}
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation : {kx,sx} = {ka,sa}*b/c<<sx;													*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKskxkx( LONG ksa, LONG b, LONG c, LONG *psx, LONG smax )
{
LONG	a;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		a  = ((ksa<<8)>>8);
		sx = (ksa>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx += sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation : {kx,sx} = a*{kb,sb}/c<<sx;													*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKxkskx( LONG a, LONG ksb, LONG c, LONG *psx, LONG smax )
{
LONG	b;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		b  = ((ksb<<8)>>8);
		sx = (ksb>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx += sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation : {kx,sx} = a*b/{kc,sc}<<sx;													*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKxkxks( LONG a, LONG b, LONG ksc, LONG *psx, LONG smax )
{
LONG	c;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		c  = ((ksc<<8)>>8);
		sx = -(ksc>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx += sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation : {kx,sx} = {ka,sa}*{kb,sb}/c<<sx;											*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKskskx( LONG ksa, LONG ksb, LONG c, LONG *psx, LONG smax )
{
LONG	a,b;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		a  = ((ksa<<8)>>8);
		b  = ((ksb<<8)>>8);
		sx = (ksa>>24) + (ksb>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx += sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation : {kx,sx} = a*{kb,sb}/{kc,sc}<<sx;											*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKxksks( LONG a, LONG ksb, LONG ksc, LONG *psx, LONG smax )
{
LONG	b,c;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		b  = ((ksb<<8)>>8);
		c  = ((ksc<<8)>>8);
		sx = (ksb>>24) - (ksc>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx += sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation : {kx,sx} = {ka,sa}*{kb,sb}/{kc,sc}<<sx;										*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKsksks( LONG ksa, LONG ksb, LONG ksc, LONG *psx, LONG smax )
{
LONG	a,b,c;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		a  = ((ksa<<8)>>8);
		b  = ((ksb<<8)>>8);
		c  = ((ksc<<8)>>8);
		sx = (ksa>>24) + (ksb>>24) - (ksc>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx += sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Proportional gain calculation(For calculation start) : {kx,sx} = {ka,sa}*b/c<<sx;										*/
/*																									*/
/****************************************************************************************************/
LONG	MlibScalKxgain( LONG a, LONG b, LONG c, LONG *psx, LONG smax )
{
LONG	kx;

/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL ){ *psx = 0;}
		kx = MlibPcalKxgain( a, b, c, psx, smax );
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation(For calculation start) : {kx,sx} = {ka,sa}*b/c<<sx;										*/
/*																									*/
/****************************************************************************************************/
LONG	MlibScalKskxkx( LONG ksa, LONG b, LONG c, LONG *psx, LONG smax )
{
LONG	a;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		a  = ((ksa<<8)>>8);
		sx = (ksa>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx = sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation(For calculation start) : {kx,sx} = a*{kb,sb}/c<<sx;										*/
/*																									*/
/****************************************************************************************************/
LONG	MlibScalKxkskx( LONG a, LONG ksb, LONG c, LONG *psx, LONG smax )
{
LONG	b;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		b  = ((ksb<<8)>>8);
		sx = (ksb>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx = sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation(For calculation start) : {kx,sx} = a*b/{kc,sc}<<sx;										*/
/*																									*/
/****************************************************************************************************/
LONG	MlibScalKxkxks( LONG a, LONG b, LONG ksc, LONG *psx, LONG smax )
{
LONG	c;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		c  = ((ksc<<8)>>8);
		sx = -(ksc>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx = sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation(For calculation start) : {kx,sx} = {ka,sa}*{kb,sb}/c<<sx;								*/
/*																									*/
/****************************************************************************************************/
LONG	MlibScalKskskx( LONG ksa, LONG ksb, LONG c, LONG *psx, LONG smax )
{
LONG	a,b;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		a  = ((ksa<<8)>>8);
		b  = ((ksb<<8)>>8);
		sx = (ksa>>24) + (ksb>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx = sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation(For calculation start) : {kx,sx} = a*{kb,sb}/{kc,sc}<<sx;								*/
/*																									*/
/****************************************************************************************************/
LONG	MlibScalKxksks( LONG a, LONG ksb, LONG ksc, LONG *psx, LONG smax )
{
LONG	b,c;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		b  = ((ksb<<8)>>8);
		c  = ((ksc<<8)>>8);
		sx = (ksb>>24) - (ksc>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx = sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Extended gain calculation(For calculation start) : {kx,sx} = {ka,sa}*{kb,sb}/{kc,sc}<<sx;							*/
/*																									*/
/****************************************************************************************************/
LONG	MlibScalKsksks( LONG ksa, LONG ksb, LONG ksc, LONG *psx, LONG smax )
{
LONG	a,b,c;
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Input decomposition processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		a  = ((ksa<<8)>>8);
		b  = ((ksb<<8)>>8);
		c  = ((ksc<<8)>>8);
		sx = (ksa>>24) + (ksb>>24) - (ksc>>24);
/*--------------------------------------------------------------------------------------------------*/
/*		call MlibPcalKxgain()																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( psx != NULL )
		{
			*psx = sx;
			kx = MlibPcalKxgain( a, b, c, psx, smax );
		}
		else
		{
			kx = MlibPcalKxgain( a, b, c, &sx, smax );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Proportional gain addition : {kx,sx} = {ka,sa} + {kb,sb}												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKxaddx( LONG ka, LONG sa, LONG kb, LONG sb, LONG *psx )
{
LONG	kx;
LONG	ds;

/*--------------------------------------------------------------------------------------------------*/
/*		Exponent part calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		ds = sa - sb;
/*--------------------------------------------------------------------------------------------------*/
		if( ka ==   0 ){ *psx = sb; return( kb );}
		if( kb ==   0 ){ *psx = sa; return( ka );}
		if( ds >=  32 ){ *psx = sb; return( kb );}
		if( ds <= -32 ){ *psx = sa; return( ka );}
/*--------------------------------------------------------------------------------------------------*/
		if( ds < 0 ){ kb = (kb>>(-ds));}
		if( ds > 0 ){ ka = (ka>>ds); sa = sb;}
/*--------------------------------------------------------------------------------------------------*/
/*		Mantissa calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		kx = ka + kb;
		if( ((ka>0)&&(kb>0)&&(kx<=0)) || ((ka<0)&&(kb<0)&&(kx>=0)) )
		{
			kx = (ka>>1) + (kb>>1); sa--;
		}
		if( kx == 0 ){ sa = 0;}
/*--------------------------------------------------------------------------------------------------*/
		*psx = sa;
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Proportional gain subtraction : {kx,sx} = {ka,sa} - {kb,sb}												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKxsubx( LONG ka, LONG sa, LONG kb, LONG sb, LONG *psx )
{
LONG	kx;
LONG	ds;

/*--------------------------------------------------------------------------------------------------*/
/*		Exponent part calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		ds = sa - sb;
/*--------------------------------------------------------------------------------------------------*/
		if( (ka == 0) || (ds >= 32)  )
		{
			if( kb == 0x80000000 )
			{
				kb = kb >> 1; sb--;
			}
			*psx = sb; return( -kb );
		}
/*--------------------------------------------------------------------------------------------------*/
		if( kb ==   0 ){ *psx = sa; return(  ka );}
		if( ds <= -32 ){ *psx = sa; return(  ka );}
/*--------------------------------------------------------------------------------------------------*/
		if( ds < 0 ){ kb = (kb>>(-ds));}
		if( ds > 0 ){ ka = (ka>>ds); sa = sb;}
/*--------------------------------------------------------------------------------------------------*/
/*		Mantissa calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		kx = ka - kb;
		if( ((ka>0)&&(kb<0)&&(kx<=0)) || ((ka<0)&&(kb>0)&&(kx>=0)) )
		{
			kx = (ka>>1) - (kb>>1); sa--;
		}
		if( kx == 0 ){ sa = 0;}
/*--------------------------------------------------------------------------------------------------*/
		*psx = sa;
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		Proportional gain multiplication : {kx,sx} = (a*b*c)<<sx;	0x40000000 <= kx < 0x80000000						*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKxmulx( LONG a, LONG b, LONG c, LONG *psx )
{
LONG	kx;
LONG	sx;
LONG	wk;
LONG	sign;
LONG	xx[2];
LONG	yy[2];
LONG	zz[2];
LONG	xxx[3];

/*--------------------------------------------------------------------------------------------------*/
/*		Zero input check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( (a==0)||(b==0)||(c==0) )
		{
			*psx = 0;
			return( 0 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Sign processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		sign = 0;
		if( a < 0 ) { a = -a; sign =  1;}
		if( b < 0 ) { b = -b; sign += 1;}
		if( c < 0 ) { c = -c; sign += 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( (ULONG)a,     (ULONG)b, (ULONG*)xx );
		MULU32U32( (ULONG)xx[0], (ULONG)c, (ULONG*)yy );
		MULU32U32( (ULONG)xx[1], (ULONG)c, (ULONG*)zz );
		xxx[0] = yy[0];
		xxx[1] = zz[0] + yy[1];
		xxx[2] = zz[1];
		if( ((ULONG)xxx[1]<(ULONG)zz[0])||((ULONG)xxx[1]<(ULONG)yy[1]) )
		{
			xxx[2] = xxx[2] + 1;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication result scaling processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( xxx[2] != 0 )
		{
			sx = 0;
			kx = xxx[2];
			SCUP31_SXINC( kx, sx, wk );
			kx = kx + (((ULONG)xxx[1])>>(32-sx));
			sx = sx - 64;
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( xxx[1] != 0 )
		{
			if( xxx[1] < 0 )
			{
				sx = -33;
				kx = (((ULONG)xxx[1])>>1);
			}
			else
			{
				sx = 0;
				kx = xxx[1];
				SCUP31_SXINC( kx, sx, wk );
				kx = kx + ((((ULONG)xxx[0])>>(31-sx))>>1);
				sx = sx - 32;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			if( xxx[0] < 0 )
			{
				sx = -1;
				kx = (((ULONG)xxx[0])>>1);
			}
			else
			{
				sx = 0;
				kx = xxx[0];
				SCUP31_SXINC( kx, sx, wk );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Termination processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		*psx = *psx + sx;
		if( sign & 0x01 )
		{
			return( -kx );
		}
		else
		{
			return(  kx );
		}

}



/****************************************************************************************************/
/*																									*/
/*		Proportional gain division : {kx,sx} = {ka,sa}/{kb,sb};													*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKxdivx( LONG ka, LONG sa, LONG kb, LONG sb, LONG smax )
{
LONG	kx;
LONG	sx;
LONG	wk;
LONG	sign;

/*--------------------------------------------------------------------------------------------------*/
/*		Zero input check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( (ka==0)||(kb==0) )
		{
			return( 0 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Sign processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		sign = 0;
		if( ka < 0 ) { ka = -ka; sign =  1;}
		if( kb < 0 ) { kb = -kb; sign += 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		Scaling processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		sx = sa - sb;
		SCUP31_SXINC( ka, sx, wk );
		SCUP31_SXDEC( kb, sx, wk );
/*--------------------------------------------------------------------------------------------------*/
/*		Division processing : kx = ka/kb	(32bit/32bit,Resolution:23bit)										*/
/*--------------------------------------------------------------------------------------------------*/
		kx = XasmDIVUXA( ka, kb, &sx, 23 );
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (sx-smax)>31 ){ kx = 0;}
		if( sx > smax    ){ kx=(kx>>(sx-smax)); sx=smax;}
		if( sx < 0       ){ kx=0x007FFFFF;      sx=0;   }
		if( sign & 0x01  ){ kx = -kx;}
		kx = (sx<<24) + (kx&0x00FFFFFF);
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}



/****************************************************************************************************/
/*																									*/
/*		1st order filter gain calculation																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		1) Time constant :  kf =  (ts/(tx+ts))<<24															*/
/*																									*/
/*						      (2*hz*ts)*(2^24)														*/
/*		2) 0.1Hz  :  kf = -------------------------  ,  (10^7/pai) = 3183098.862					*/
/*						   (2*hz*ts) + (10^7/pai)													*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalKf1gain(
		LONG	tx,			/* Time constant(Tx)/Frequency(Fx)	[xs],[0.1Hz] (X=0:Tx, X=1:Fx)				*/
		LONG	ts,			/* Calculation cycle					[xs],[us/ns] (Y=0:us, Y=1:ns)				*/
		LONG	insel	)	/* Input specification(0xYX)			[X:Tx/FxSelection, Y:us/nsSelection]					*/
{
LONG	kx;					/* Computation result																*/
LONG	ax;					/* For dividend work															*/
LONG	bx;					/* Excluding  For numerical work															*/
ULONG	xx[2];				/* For work																*/

/*--------------------------------------------------------------------------------------------------*/
/*		Input check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (tx == 0) || (ts == 0) )
		{
			return( 0 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Gain calculation processing : Time constant[xs]																	*/
/*--------------------------------------------------------------------------------------------------*/
/*				  ts																				*/
/*		kf =  ---------- * (2^24)																	*/
/*				ts + tx																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (insel & 0x0F) == 0x00 )			/* Time constant[xs]										*/
		{
			ax = ts;							/* Min( ax )= 1us									*/
			bx = ts + tx;						/* Max( bx )= 1us + 16777215us (Max(tx)=16.7 sec)	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Gain calculation processing : Frequency[0.1Hz] && Calculation cycle[us]												*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					(hz*ts)*(2^24)																	*/
/*		kf = ---------------------------  ,  (10^7/(2*pai)) = 1591549.431							*/
/*			  (hz*ts) + (10^7/(2*pai))																*/
/*																									*/
/*		Ts= 125us : Max.Frequency = 2^32 /  125 / 10 = 3435973.837 Hz								*/
/*		Ts=1000us : Max.Frequency = 2^32 / 1000 / 10 =  429496.729 Hz								*/
/*		Min( kf ) = ((hz*ts) / 1591549) * 2^24 = 10.5414 * (hz*ts) = 10.5414						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
		else if( (insel & 0xF0) == 0x00 )		/* Frequency[0.1Hz] && Calculation cycle[us]					*/
		{
			MULU32U32( ts, tx, xx );			/* xx[] = hz*ts										*/
			if( xx[1] > 0x00 ){ return( 0 );}	/* if( (hz*ts) >= 2^32 ){ kf = 0;}					*/
		/*------------------------------------------------------------------------------------------*/
			if( xx[0] >= 0x10000000 ){ ax = (xx[0]>>2); bx = ax +  397887;}
			else                     { ax = (xx[0]<<2); bx = ax + 6366198;}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Gain calculation processing : Frequency[0.1Hz] && Calculation cycle[ns]												*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					(hz*ts)*(2^24)																	*/
/*		kf = ---------------------------  ,  (10^10/(2*pai)) = 1591549431 = 0x5EDD1DF7				*/
/*			  (hz*ts) + (10^10/(2*pai))																*/
/*																									*/
/*		Ts=125us : Max.Frequency = 2^40 / 125000 / 10 = 879609.302 Hz								*/
/*		Min( kf ) = ((hz*ts) / 1591549431) * 2^24 = 0.0105414 * (hz*ts) --> Min( hz*ts ) >= 95		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
		else if( ts >= 1000 )					/* Frequency[0.1Hz] && Calculation cycle[ns]					*/
		{
			MULU32U32( ts, tx, xx );			/* xx[] = hz*ts										*/
			if( xx[1] > 0xFF ){ return( 0 );}	/* if( (hz*ts) >= 2^40 ){ kf = 0;}					*/
			bx = (1591549431 >> 2);				/* bx = (0x5EDD1DF7>>2) = 0x17B7477D				*/
		/*------------------------------------------------------------------------------------------*/
			if(      xx[1] > 0x0F ){ ax = (xx[0]>>10) + (xx[1]<<22); bx = ax + (bx>>8);}
			else if( xx[1] > 0x00 ){ ax = (xx[0]>> 6) + (xx[1]<<26); bx = ax + (bx>>4);}
			else                   { ax = (xx[0]>> 2)              ; bx = ax + (bx>>0);}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Input error : Calculation cycle[ns] < 1000[ns]															*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			return( 0 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Division : kx = (ax/bx)<<24  (32bit/32bit,Resolution:24bit)										*/
/*--------------------------------------------------------------------------------------------------*/
		kx = XasmDIVUB( ax, bx, 24 );			/* kx = (ax/bx) * 2^24								*/
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}


#if (FLOAT_USE==TRUE)
#if 0
/****************************************************************************************************/
/*																									*/
/*		1st order filter gain calculation																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		1) Time constant :  kf =  (ts/(tx+ts))																*/
/*																									*/
/*		2) Frequency :  kf =  (ts/(tx+ts))	,  tx = 1/(2*π*fx)											*/
/*						=  (ts/((1/(2*π*fx))+ts)													*/
/*						=  ((ts*fx)/((1/(2*π))+(ts*fx)))											*/
/*																									*/
/****************************************************************************************************/
float	FlibPcalKf1gain(
		LONG	tx,		/* Time constant(tx)/Frequency(fx)	[sec],[Hz] (X=0:tx, X=1:fx)					*/
		LONG	ts,		/* Calculation cycle					[sec] 										*/
		LONG	insel	)	/* Input specification(0x0X)			[X:tx/fxSelection]								*/
{
/////		return( 1.0 );	/* Temporary treatment */

float	kx;				/* Computation result																*/
float	ax;				/* For dividend work															*/
float	bx;				/* Excluding  For numerical work															*/

/*--------------------------------------------------------------------------------------------------*/
/*		Input check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (tx == (float)0.0) || (ts == (float)0.0) )
		{
			return( 0.0 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Gain calculation processing : Time constant[xs]																	*/
/*--------------------------------------------------------------------------------------------------*/
/*				  ts																				*/
/*		kf =  ----------																			*/
/*				ts + tx																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (insel & 0x0F) == 0x00 )			/* Time constant[s]										*/
		{
			ax = ts;							/* 													*/
			bx = ts + tx;						/* 													*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Gain calculation processing : Frequency[Hz] && Calculation cycle[s]													*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					(ts*fx)																			*/
/*		kf = ---------------------------  															*/
/*			  (1/(2*π)) + (ts*fx)																	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
		else									/* Frequency[Hz] && Calculation cycle[s]						*/
		{
			ax = tx * ts;						/* 													*/
			bx = (tx * ts) + (1/(2 * PAI));	/* 													*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Division : kx = (ax/bx)																			*/
/*--------------------------------------------------------------------------------------------------*/
		kx = ax / bx;							/* kx = (ax/bx)										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		return( kx );

}
#endif //↓todo
#if 1
/****************************************************************************************************/
/*																									*/
/*		1st order filter gain calculation																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		1) Time constant :  kf =  (ts/(tx+ts))<<24															*/
/*																									*/
/*						      (2*hz*ts)*(2^24)														*/
/*		2) 0.1Hz  :  kf = -------------------------  ,  (10^7/pai) = 3183098.862					*/
/*						   (2*hz*ts) + (10^7/pai)													*/
/*																									*/
/****************************************************************************************************/
float	FlibPcalKf1gain(
		LONG	tx,			/* Time constant(Tx)/Frequency(Fx)	[xs],[0.1Hz] (X=0:Tx, X=1:Fx)				*/
		LONG	ts,			/* Calculation cycle					[xs],[us/ns] (Y=0:us, Y=1:ns)				*/
		LONG	insel	)	/* Input specification(0xYX)			[X:Tx/FxSelection, Y:us/nsSelection]					*/
{
	long	Klpf;
	float	fKlpf;

	Klpf = MlibPcalKf1gain( tx, ts, insel );
	fKlpf = (float)Klpf / (float)( 1 << 24 );
	//fKlpf = (float)Klpf;

	return( fKlpf );
}
#endif
#endif /* FLOAT_USE */



/****************************************************************************************************/
/*																									*/
/*		2nd order filter gain calculation(0.1HzInput)															*/
/*																									*/
/****************************************************************************************************/
void	MlibPcalKf2gain(
		LONG	hz,			/* Filter frequency			[0.1Hz]										*/
		LONG	dx,			/* Filter attenuation coefficient(ζ)		[0.001]										*/
		LONG	ts,			/* Scan time			[us/ns] (Y=0:us, Y=1:ns)					*/
		LONG	kf[2],		/* Gain calculation result			[--]										*/
		LONG	insel	)	/* Input specification(0xYX)			[X:not used, Y=us/ns sel.]					*/
{
LONG	sx;					/* For work																*/
LONG	ax,as;				/* For multiplication/division work															*/
LONG	bx,bs;				/* For multiplication/division work															*/
LONG	kx[2];				/* For computation result work														*/
LONG	kunit = ((insel & 0xF0)==0)? 1 : 1000;

/*--------------------------------------------------------------------------------------------------*/
/*		Input check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (hz == 0) || (ts == 0) || ((insel & 0xF0)&&(ts < 1000)) )
		{
			kf[0] = kf[1] = 0;
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Input limit processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( dx <    50 ){ dx =    50;}
		if( dx > 10000 ){ dx = 10000;}
/*--------------------------------------------------------------------------------------------------*/
/*		1st order filter gain calculation : ts:[us], hz:[0.1Hz]												*/
/*--------------------------------------------------------------------------------------------------*/
/*							ts	                        ts*hz										*/
/*		Calculation formula : kf[0] = --------- * 2^24 = ---------------------------- * 2^24						*/
/*				 		  ts + tx            ts*hz + (10^10/(4*pai*dx))								*/
/*--------------------------------------------------------------------------------------------------*/
		as = bs = 0;
		ax = MlibPcalKxmulx( ts,    hz,        dx, &as ) >> 1;
		bx = MlibPcalKxmulx( kunit, 795774715, 1,  &bs ) >> 1;
		if( as > bs ){ ax = ax >> (as - bs);}		/* Min(ax) = 1us * 1[0.1Hz] * 50 = 50			*/
		if( bs > as ){ bx = bx >> (bs - as);}		/* Max(bx) = 795774715 + 50 = 795774765			*/
		kx[0] = XasmDIVUB( ax, (ax+bx), 24 );		/* kx[0]=(ax/(ax+bx))*2^24 : Min(kx[0])=1.054	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of integral gain : ts:[us], hz:[0.1Hz], 													*/
/*--------------------------------------------------------------------------------------------------*/
/*				 			   2*pai*hz*ts		  2*pai*hz*ts		  hz*ts							*/
/*		Calculation formula : kf[1] = --------------------- = -------------- = ----------------					*/
/*						  2*(dx/1000)*(10^7)	   2*dx*10^4	   dx*3183.098862					*/
/*--------------------------------------------------------------------------------------------------*/
		as = bs = 0;
		ax = MlibPcalKxmulx( ts,    hz, 100000,    &as );
		bx = MlibPcalKxmulx( kunit, dx, 318309886, &bs );
		sx = as - bs;								/* kx[1] = (ax/bx)*2^sx							*/
		kx[1] = XasmDIVUXB( ax, bx, &sx, 23 );		/* Min(kx[1]) = 1/(10000*3183.1)*2^24 = 0.527 	*/
	/*----------------------------------------------------------------------------------------------*/
		if( (sx-24) > 31 ){ kx[1] = 0; sx = 24;}
		else if( sx > 24 ){ kx[1] = (kx[1]>>(sx-24)); sx = 24;}
		if( kx[1] == 0   ){ kx[1] = 1;}				/* Min(kx)= 0.527 = 0 --> 1 (Currently unnecessary:ts>1us)	*/
		kx[1] = kx[1] + (sx<<24);					/* kx[1] = {kx,sx}								*/
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		kf[0] = kx[0];
		kf[1] = kx[1];
		return;

}


#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		2nd order filter gain calculation																		*/
/*																									*/
/****************************************************************************************************/
void	FlibPcalKf2gain(
		float	hz,			/* Filter frequency			[Hz]									*/
		float	dx,			/* Filter attenuation coefficient(ζ)		[-]										*/
		float	ts,			/* Scan time			[sec]									*/
		float	kf[2])		/* Gain calculation result			[--]									*/
{
float	ax;				/* For dividend work															*/
float	bx;				/* Excluding  For numerical work															*/

/*--------------------------------------------------------------------------------------------------*/
/*		Input check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (hz == (float)0.0) || (ts == (float)0.0) )
		{
			kf[0] = kf[1] = 0.0;
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Input limit processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( dx < (float)0.05   ){ dx =  0.05; }
		if( dx > (float)10.000 ){ dx = 10.000;}
/*--------------------------------------------------------------------------------------------------*/
/*		1st order filter gain calculation : ts:[sec], hz:[Hz]												*/
/*--------------------------------------------------------------------------------------------------*/
/*							ts	              ts*hz				        ts*hz*dx					*/
/*		Calculation formula : kf[0] = ---------  = ------------------------	= ----------------------			*/
/*				 		  ts + tx       ts*hz + (1/(4π*dx))        ts*hz*dx + (1/4π)				*/
/*--------------------------------------------------------------------------------------------------*/
//		ax = ts * hz * dx;						/* 													*/
//		bx = (ts * hz * dx) + (1/(4 * f_PAI));	/* 													*/
		ax = ts * hz * dx * 4 * PAI;			/* 													*/
		bx = ax + 1.0;							/* 													*/
		kf[0] = ax / bx;						/* kf[0] = (ax/bx)									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of integral gain : ts:[us], hz:[0.1Hz], 													*/
/*--------------------------------------------------------------------------------------------------*/
/*				 		   (2π*hz*ts)		 (π*hz*ts)		  										*/
/*		Calculation formula : kf[1] = --------------- = -------------- 											*/
/*						     (2*dx)	            dx													*/
/*--------------------------------------------------------------------------------------------------*/
		ax = (ts * hz * PAI);					/* 													*/
		bx = dx;								/* 													*/
		kf[1] = ax / bx;						/* kf[1] = (ax/bx)									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		return;

}
#endif /* FLOAT_USE */



/****************************************************************************************************/
/*																									*/
/*		2nd order notch filter gain calculation(0.1HzInput)														*/
/*																									*/
/****************************************************************************************************/
void	MlibPcalKnf2gain(
		LONG	hz,			/* Filter frequency					[0.1Hz]								*/
		LONG	qx,			/* Q constant(Qx)/Attenuation coefficient(Dx)			[0.001] (X=0:Qx, X=1:Dx)			*/
		LONG	kn,			/* Notch depth gain					[0.001]								*/
		LONG	ts,			/* Scan time					[us/ns]	(Y=0:us, Y=1:ns)			*/
		LONG	kf[3],		/* Gain calculation result					[--]								*/
		LONG	insel	)	/* Input specification(0xYX)					[X=Qx/Dx sel, Y:us/ns sel]			*/
{
LONG	sx;					/* For work																*/
LONG	ax,as;				/* For multiplication/division work															*/
LONG	bx,bs;				/* For multiplication/division work															*/
LONG	kx[3];				/* For computation result work														*/
LONG	xx[2];				/* 32bit*32bitFor multiplication work												*/
LONG	qxshl;				/* qx shift left														*/
LONG	kunit = ((insel & 0xF0)==0)? 1 : 1000;

/*--------------------------------------------------------------------------------------------------*/
/*		Input check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (hz == 0) || (ts == 0) || ((insel & 0xF0)&&(ts < 1000)) )
		{
			kf[0] = kf[1] = kf[2] = 0;
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Input selection & limit processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( (insel & 0x0F) == 0x00 )
		{
			if( qx <    50 ){ qx =    50;}			/* Lower Limit									*/
			if( qx > 10000 ){ qx = 10000;}			/* Upper Limit									*/
			qxshl = 0;								/* qx shift left								*/
		}
		else
		{
			if( qx <    50 ){ qx =    50;}			/* Lower Limit									*/
			if( qx > 10000 ){ qx = 10000;}			/* Upper Limit									*/
			qx = (500000<<12)/qx;					/* dx --> qx : (500000<<12) = 0x7A120000		*/
			qxshl = 12;								/* qx shift left								*/
		}
		if( kn > 10000 ){ kn = 10000;}
/*--------------------------------------------------------------------------------------------------*/
/*		1st order filter gain calculation : ts:[us], hz:[0.1Hz]												*/
/*--------------------------------------------------------------------------------------------------*/
/*									ts*hz							 ts*hz							*/
/*		Calculation formula : kf[0] = -------------------------- * 2^24 = ------------------------ * 2^24		*/
/*				 		  ts*hz + qx*10^4/(2*pai)			  ts*hz + qx*1591.549431				*/
/*--------------------------------------------------------------------------------------------------*/
		as = 0; bs = qxshl;
		ax = MlibPcalKxmulx( ts,    hz, 100000,    &as ) >> 1;
		bx = MlibPcalKxmulx( kunit, qx, 159154943, &bs ) >> 1;
		if( as > bs ){ ax = ax >> (as - bs);}		/* Min(ax) = 1us * 1[0.1Hz] * 100000 = 100000	*/
		if( bs > as ){ bx = bx >> (bs - as);}		/* Max(bx) = 159154943*10000 = 1591549430000	*/
		kx[0] = XasmDIVUB( ax, (ax+bx), 24 );		/* kx[0]=(ax/(ax+bx))*2^24 : Min(kx[0])=1.054	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of integral gain : ts[us], hz:[0.1Hz]														*/
/*--------------------------------------------------------------------------------------------------*/
/*				 		  qx*2*pai*ts*hz	  qx*ts*hz												*/
/*		Calculation formula : kf[1] = ---------------- = ------------											*/
/*						   1000*10^6*10      1591549431												*/
/*--------------------------------------------------------------------------------------------------*/
		as = qxshl; bs = 0;
		ax = MlibPcalKxmulx( ts,    hz,         qx, &as );
		bx = MlibPcalKxmulx( kunit, 1591549431, 1,  &bs );
		sx = as - bs;								/* kx[1] = (ax/bx)*2^sx							*/
		kx[1] = XasmDIVUXB( ax, bx, &sx, 23 );		/* Min(kx[1]) = 50/(1591549431)*2^24 = 0.527 	*/
	/*----------------------------------------------------------------------------------------------*/
		if( (sx-24) > 31 ){ kx[1] = 0; sx = 24;}
		else if( sx > 24 ){ kx[1] = (kx[1]>>(sx-24)); sx = 24;}
		if( kx[1] == 0   ){ kx[1] = 1;}				/* Min(kx)= 0.527 = 0 --> 1 (Currently unnecessary:ts>1us)	*/
		kx[1] = kx[1] + (sx<<24);					/* kx[1] = {kx,sx}								*/
/*--------------------------------------------------------------------------------------------------*/
/*		Notch depth gain calculation, Notch/LPF switching gain calculation (The calculation formula is the same)						*/
/*--------------------------------------------------------------------------------------------------*/
/*				 		   kn             kn*(2^16)*(2^40)/1000	    kn*(2^16)*1099511628			*/
/*		Calculation formula : kf[2] = ------ * 2^24 = ----------------------- = ----------------------			*/
/*						  1000					  2^32						2^32					*/
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( (kn<<16), 1099511628, (ULONG*)xx );
		kx[2] = xx[1];
/*--------------------------------------------------------------------------------------------------*/
/*		Computation end processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		kf[0] = kx[0];
		kf[1] = kx[1];
		kf[2] = kx[2];
		return;

}
#pragma arm section code



#if 0 /* Currently unused */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		<<<<< Position related calculation function >>>>>																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Position command interpolation filter																		*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcmdIpfil( LONG pcmdin, LONG shift, LONG index, PIPFV *pipfvar )
{
LONG	x;
LONG	outx;

		if( shift == 0 )
		{
			return( pcmdin );
		}

		if( index == 0 )
		{
			pipfvar->cmdx = pcmdin;
		}

		x = pipfvar->cmdx + pipfvar->remx;
		outx = (x >> shift);
		pipfvar->remx = x - (outx << shift);

		return( outx );

}
#endif



/****************************************************************************************************/
/*																									*/
/*		Position command moving average filter																	*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcmdMafil( LONG pcmdin, LONG pmafnum, PMAFV *pmafvar, LONG *pmafbuf )
{
LONG	subx;
LONG	outx;

/*--------------------------------------------------------------------------------------------------*/
/*		Check the moving average number																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( pmafnum <= 1 )
		{
			pmafvar->zcntx = (pcmdin != 0)? 1 : 0;
			return( pcmdin );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average buffer processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		subx = pmafvar->ksub * pmafbuf[pmafvar->idx];
		pmafbuf[pmafvar->idx] = pcmdin;
		if( ++pmafvar->idx >= pmafnum )
		{
			pmafvar->idx = 0;
			pmafvar->ksub = 1;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average output calculation processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		pmafvar->sumx = pmafvar->sumx - subx + pcmdin;
		outx = (pmafvar->sumx + pmafvar->remx)/pmafnum;
		pmafvar->remx = (pmafvar->sumx + pmafvar->remx) - (pmafnum * outx);
/*--------------------------------------------------------------------------------------------------*/
/*		Process moving average payout completion information																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( pcmdin != 0 )
		{
			pmafvar->zcntx = (USHORT)pmafnum;
		}
		else if( pmafvar->zcntx > 0 )
		{
			--pmafvar->zcntx;
		}

		return( outx );

}


LONG	MlibPcmdMafilSec( LONG pcmdin, LONG pmafnum, HIGHFV *pmafvarSec, LONG *pmafbufSec )   /*<S0C3>*/
{
   LONG	subx;
   LONG	outx;

/*--------------------------------------------------------------------------------------------------*/
/*		Check the moving average number2   				          													*/
/*--------------------------------------------------------------------------------------------------*/
		if( pmafnum <= 1 )
		{
			pmafvarSec->zcntxSec = (pcmdin != 0)? 1 : 0;
			return( pcmdin );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average buffer processing2                																*/
/*--------------------------------------------------------------------------------------------------*/
		subx = pmafvarSec->ksubSec * pmafbufSec[pmafvarSec->idxSec];
		pmafbufSec[pmafvarSec->idxSec] = pcmdin;
		if( ++pmafvarSec->idxSec >= pmafnum )
		{
			pmafvarSec->idxSec = 0;
			pmafvarSec->ksubSec = 1;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average output calculation processing2      																	*/
/*--------------------------------------------------------------------------------------------------*/
		pmafvarSec->sumxSec = pmafvarSec->sumxSec - subx + pcmdin;
		outx = (pmafvarSec->sumxSec + pmafvarSec->remxSec)/pmafnum;
		pmafvarSec->remxSec = (pmafvarSec->sumxSec + pmafvarSec->remxSec) - (pmafnum * outx);
/*--------------------------------------------------------------------------------------------------*/
/*		Process moving average payout completion information2          															*/
/*--------------------------------------------------------------------------------------------------*/
		if( pcmdin != 0 )
		{
			pmafvarSec->zcntxSec = (USHORT)pmafnum;
		}
		else if( pmafvarSec->zcntxSec > 0 )
		{
			--pmafvarSec->zcntxSec;
		}
		return( outx );
}



/****************************************************************************************************/
/*																									*/
/*		Position command interpolation moving average filter																*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcmdImafil( LONG pcmdin, LONG shift, LONG index, PIMFV *pimfvar )
{
LONG	x;
LONG	outx;

/*--------------------------------------------------------------------------------------------------*/
/*		Check the moving average number																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( shift == 0 )
		{
			pimfvar->zcntx = (pcmdin != 0)? 1 : 0;
			return( pcmdin );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average input processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( index == 0 )
		{
			pimfvar->oldpcmd = pimfvar->newpcmd;
			pimfvar->newpcmd = pcmdin;
			if( pcmdin != 0 )
			{
				pimfvar->zcntx = (2 << shift);
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average output processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		pimfvar->sumx = pimfvar->sumx + (pimfvar->newpcmd - pimfvar->oldpcmd);
		x = pimfvar->sumx + pimfvar->remx;
		outx = (x >> (2*shift));
		pimfvar->remx = x - (outx << (2*shift));
		if( pimfvar->zcntx > 0 )
		{
			--pimfvar->zcntx;
		}

		return( outx );

}


LONG	MlibPcmdImafilSec( LONG pcmdin, LONG shift, LONG indexSec, HIMFV *pimfvarSec )   /*<S0C3>*/
{
LONG	x;
LONG	outx;
/*--------------------------------------------------------------------------------------------------*/
/*		Check the moving average number2         																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( shift == 0 )
		{
			pimfvarSec->zcntxSec = (pcmdin != 0)? 1 : 0;
			return( pcmdin );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average input processing2      																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( indexSec == 0 )
		{
			pimfvarSec->oldpcmdSec = pimfvarSec->newpcmdSec;
			pimfvarSec->newpcmdSec = pcmdin;
			if( pcmdin != 0 )
			{
				pimfvarSec->zcntxSec = (2 << shift);
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average output processing2      																		*/
/*--------------------------------------------------------------------------------------------------*/
		pimfvarSec->sumxSec = pimfvarSec->sumxSec + (pimfvarSec->newpcmdSec - pimfvarSec->oldpcmdSec);
		x = pimfvarSec->sumxSec + pimfvarSec->remxSec;
		outx = (x >> (2*shift));
		pimfvarSec->remxSec = x - (outx << (2*shift));
		if( pimfvarSec->zcntxSec > 0 )
		{
			--pimfvarSec->zcntxSec;
		}

		return( outx );

}



/****************************************************************************************************/
/*																									*/
/*		Position command index acceleration/deceleration filter																	*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcmdExpfil( LONG pcmdin, LONG kexp, LONG pbias, PEXFV *pexfvar )
{
LONG	outx;
LONG	xx[2];
LONG	kx = (kexp<<8);
LONG	sx = (((ULONG)kexp)>>24);

/*--------------------------------------------------------------------------------------------------*/
/*		Input check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( kexp == 0 )
		{
			pexfvar->sumx = 0;
			return( pcmdin );
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Position command total calculation																			*/
/*--------------------------------------------------------------------------------------------------*/
		pexfvar->sumx = pexfvar->sumx + pcmdin;

/*--------------------------------------------------------------------------------------------------*/
/*		Exponential acceleration/deceleration filter calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( pexfvar->sumx, kx, xx );
		outx = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
/*--------------------------------------------------------------------------------------------------*/
		if( pexfvar->sumx > pbias )
		{
			outx = outx + pbias;
			if( outx > pexfvar->sumx ){ outx = pexfvar->sumx;}
		}
		else if( pexfvar->sumx < -pbias )
		{
			outx = outx - pbias;
			if( outx < pexfvar->sumx ){ outx = pexfvar->sumx;}
		}
		else
		{
			outx = pexfvar->sumx;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Forced convergence processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (outx==0)&&(pexfvar->sumx!=0) )
		{
			outx = (pexfvar->sumx > 0)? 1 : -1;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Termination processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		pexfvar->sumx = pexfvar->sumx - outx;
		return( outx );

}



/****************************************************************************************************/
/*																									*/
/*		Position command 1st order low-pass filter																*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcmdLpfil1( LONG pcmdin, LONG kf, LONG *z )
{
LONG	x;
LONG	xx[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Processing when filter gain is zero																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( kf == 0 )
		{
			*z = 0;
			return( pcmdin );
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Input addition																					*/
/*--------------------------------------------------------------------------------------------------*/
		*z = *z + pcmdin;

/*--------------------------------------------------------------------------------------------------*/
/*		1st-order lag filter gain multiplication																	*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( *z, kf, xx );
		x  = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1);

/*--------------------------------------------------------------------------------------------------*/
/*		Forced convergence processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (x==0) && (*z!=0) )
		{
			x = (*z>0)? 1 : -1;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Output subtraction																					*/
/*--------------------------------------------------------------------------------------------------*/
		*z = *z - x;
		return( x );
}



/****************************************************************************************************/
/*																									*/
/*		Position command 2nd order low pass filter																*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcmdLpfil2( LONG pcmdin, LONG kf[2], LONG z[2] )
{
LONG	x;
LONG	xx[2];
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Processing when filter gain is zero																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( kf[0] == 0 )
		{
			z[0] = z[1] = 0;
			return( pcmdin );
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Integral addition																					*/
/*--------------------------------------------------------------------------------------------------*/
		z[0] = z[0] + pcmdin;

/*--------------------------------------------------------------------------------------------------*/
/*		1st-order lag calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( (z[0]-z[1]), kf[0], xx );
		x = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1);
		if( (x==0) && (z[1]!=z[0]) )
		{
			x = (z[0]>z[1]) - (z[0]<z[1]);
		}
		z[1] = z[1] + x;

/*--------------------------------------------------------------------------------------------------*/
/*		Integral subtraction																					*/
/*--------------------------------------------------------------------------------------------------*/
		kx = (kf[1]<<8);
		sx = (((ULONG)kf[1])>>24);
		MUL3232( z[1], kx, xx );
		x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
		if( (x==0) && (z[0]!=0) )
		{
			x = (z[0] > 0)? 1 : -1;
		}
		z[0] = z[0] - x;

		return( x );

}


#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		Position command 2nd order low pass filter																*/
/*																									*/
/****************************************************************************************************/
float	FlibPcmdLpfil2( float pcmdin, float kf[2], float z[2] )
{
float	x;

/*--------------------------------------------------------------------------------------------------*/
/*		Processing when filter gain is zero																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( kf[0] == (float)0.0 )
		{
//			z[0] = z[1] = 0;
			z[0] = z[1] = 0.0;
			return( pcmdin );
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Integral addition																					*/
/*--------------------------------------------------------------------------------------------------*/
		z[0] = z[0] + pcmdin;

/*--------------------------------------------------------------------------------------------------*/
/*		1st-order lag calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
		x = ( (z[0]-z[1]) * kf[0] );
		if( (x==(float)0.0) && (z[1]!=z[0]) )
		{
			x = (z[0]>z[1]) - (z[0]<z[1]);
		}
//		z[1] = z[1] + (LONG)x;
		z[1] = z[1] + x;

/*--------------------------------------------------------------------------------------------------*/
/*		Integral subtraction																					*/
/*--------------------------------------------------------------------------------------------------*/
		x = ( z[1] * kf[1] );
//		if( (x==(float)0.0) && (z[0]!=0) )
		if( (x==(float)0.0) && (z[0]!=(float)0.0) )
		{
//			x = (z[0] > 0)? 1 : -1;
			x = (z[0] > 0)? (float)1.0 : (float)-1.0;
		}
//		z[0] = z[0] - (LONG)x;
		z[0] = z[0] - x;

//		return( (LONG)x );
		return( x );
}
#endif /* FLOAT_USE */




/****************************************************************************************************/
/*																									*/
/*		Position command 2nd order notch filter																	*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcmdNxfil2( LONG pcmdin, LONG kf[3], LONG z[3] )
{
LONG	x,rv;
LONG	xx[2];
LONG	kx,sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Processing when filter gain is zero																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( kf[0] == 0 )
		{
			z[0] = z[1] = z[2] = 0;
			return( pcmdin );
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Integral addition																					*/
/*--------------------------------------------------------------------------------------------------*/
		z[0] = z[0] + pcmdin;

/*--------------------------------------------------------------------------------------------------*/
/*		1st-order lag calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( (z[0]-z[1]), kf[0], xx );
		x = (xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1);
		if( (x==0) && (z[1]!=z[0]) )
		{
			x = (z[0]>z[1]) - (z[0]<z[1]);
		}
		z[1] = z[1] + x;

/*--------------------------------------------------------------------------------------------------*/
/*		Integral subtraction																					*/
/*--------------------------------------------------------------------------------------------------*/
		kx = (kf[1]<<8);
		sx = (((ULONG)kf[1])>>24);
		MUL3232( z[1], kx, xx );
		x = (xx[1]<<(24-sx)) + (((((ULONG)xx[0]>>sx)>>7)+1)>>1);
		if( (x==0) && (z[0]!=0) )
		{
			x = (z[0] > 0)? 1 : -1;
		}
		z[0] = z[0] - x;

/*--------------------------------------------------------------------------------------------------*/
/*		Output calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( z[1], kf[2], xx );
		x  = z[1] - ((xx[1]<<8) + (((((ULONG)xx[0])>>23)+1)>>1));
		rv = pcmdin - (x - z[2]);
		z[2] = x;

		return( rv );

}



/****************************************************************************************************/
/*																									*/
/*		Pulse command electronic gear calculation			rv=(B/A)*pcmda												*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	MlibPcmdEgear( LONG pcmda, EGEAR_DATA *Egear, LONG *pcmdrem )
{
	LONG  pls;

#if ( CSW_SVCF_EGEAR_USE == TRUE )
 /* Using electronic gear */
	INT64 wk;
	wk  = ((INT64)pcmda * (INT64)Egear->egear_motor) + (INT64)*pcmdrem;
	pls = (LONG)(wk / Egear->egear_ref);

	*pcmdrem = (LONG)( wk - ((INT64)pls * (INT64)Egear->egear_ref) );
#else
 /* Electronic gear not used */
	pls = pcmda;
	*pcmdrem = 0;
#endif

	return pls;
}
#endif

LONG	MlibPcmdEgear( LONG pcmda, EGEAR *egear, LONG *pcmdrem )
{
LONG		pcmdb;				/* Computation result(Pulse command B)												*/
LONG	wk1[2];				/* For work																*/
LONG	wk2[2];				/* For work																*/

/*--------------------------------------------------------------------------------------------------*/
/*		Position command calculation : pcmdb = k1*pcmda + ((k2*pcmda)>>31);											*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( pcmda, egear->k1, wk1 );
		MUL3232( pcmda, egear->k2, wk2 );
		pcmdb = wk1[0] + (wk2[1]<<1) + ((ULONG)wk2[0]>>31);

/*--------------------------------------------------------------------------------------------------*/
/*		Position command remainder calculation : pcmdrem = pcmdrem + ( B*pcmda - A*pcmdb )								*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( pcmda, egear->b, wk1 );
		MUL3232( pcmdb, egear->a, wk2 );
		*pcmdrem = *pcmdrem + ((ULONG)wk1[0] - (ULONG)wk2[0]);

/*--------------------------------------------------------------------------------------------------*/
/*		Position command surplus check : (-A/2) <= pcmdrem < (A/2)											*/
/*--------------------------------------------------------------------------------------------------*/
		if( *pcmdrem <  0                 ){ --pcmdb; *pcmdrem += egear->a;}
		if( *pcmdrem >= egear->a          ){ ++pcmdb; *pcmdrem -= egear->a;}
		if( *pcmdrem >= ((egear->a+1)>>1) ){ ++pcmdb; *pcmdrem -= egear->a;}

		return( pcmdb );

}

#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		Pulse command electronic gear calculation			rv=(B/A)*pcmda												*/
/*																									*/
/****************************************************************************************************/
float	FlibPcmdEgear( float pcmda, EGEAR *egear, float *pcmdrem )
{
	float	pcmdb;				/* Computation result(Pulse command B)												*/
	float	wk1;				/* For work																*/
	float	wk2;				/* For work																*/
/*--------------------------------------------------------------------------------------------------*/
/*		Position command calculation : pcmdb = k1*pcmda + ((k2*pcmda)>>31);											*/
/*--------------------------------------------------------------------------------------------------*/
		pcmdb = ((pcmda * egear->f_b) / egear->f_a);

/*--------------------------------------------------------------------------------------------------*/
/*		Position command remainder calculation : pcmdrem = pcmdrem + ( B*pcmda - A*pcmdb )								*/
/*--------------------------------------------------------------------------------------------------*/
		wk1 = (pcmda * egear->b);		/* The following is the lower order32Bitmust be calculated normally 				*/
		wk2 = (pcmdb * egear->a);		/* (Upper level32Bitcan be ignored.)								*/
		*pcmdrem = *pcmdrem + (wk1 - wk2);

/*--------------------------------------------------------------------------------------------------*/
/*		Position command surplus check : (-A/2) <= pcmdrem < (A/2)											*/
/*--------------------------------------------------------------------------------------------------*/
		if( *pcmdrem <  0                 ){ --pcmdb; *pcmdrem += egear->a;}
		if( *pcmdrem >= egear->a          ){ ++pcmdb; *pcmdrem -= egear->a;}
		if( *pcmdrem >= ((egear->a+1)>>1) ){ ++pcmdb; *pcmdrem -= egear->a;}

		return( pcmdb );

}
#endif /* FLOAT_USE */
/****************************************************************************************************/
/*																									*/
/*		Electronic gear inversion calculation				rv=(A/B)*inpls												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibEgearRvscnv( LONG inpls, EGEAR *egear, LONG *plsrem )
{
LONG	outgr;				/* Computation result(Command unit)													*/
LONG	wk1[2];				/* For work																*/
LONG	wk2[2];				/* For work																*/

/*--------------------------------------------------------------------------------------------------*/
/*		Electronic gear inversion calculation : outgr = g1*inpls + ((g2*inpls)>>31);									*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( inpls, egear->g1, wk1 );
		MUL3232( inpls, egear->g2, wk2 );
		outgr = wk1[0] + (wk2[1]<<1) + ((ULONG)wk2[0]>>31);

/*--------------------------------------------------------------------------------------------------*/
/*		Electronic gear inversion remainder calculation : plsrem = plsrem + ( A*inpls - B*outgr )							*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( inpls, egear->a, wk1 );
		MUL3232( outgr, egear->b, wk2 );
		*plsrem = *plsrem + ((ULONG)wk1[0] - (ULONG)wk2[0]);

/*--------------------------------------------------------------------------------------------------*/
/*		Electronic gear inversion remainder check : (-B/2) <= plsrem < (B/2)										*/
/*--------------------------------------------------------------------------------------------------*/
		if( *plsrem <  0                 ){ --outgr; *plsrem += egear->b;}
		if( *plsrem >= egear->b          ){ ++outgr; *plsrem -= egear->b;}
		if( *plsrem >= ((egear->b+1)>>1) ){ ++outgr; *plsrem -= egear->b;}

		return( outgr );

}



/****************************************************************************************************/
/*																									*/
/*		Electronic gear gain calculation																			*/
/*																									*/
/****************************************************************************************************/
void	MlibPcalKegear( LONG a, LONG b, EGEAR *egear )
{
ULONG	k1,k2;
ULONG	g1,g2;

/*--------------------------------------------------------------------------------------------------*/
/*		Electronic gear for position command(k1,k2)																*/
/*--------------------------------------------------------------------------------------------------*/
		k1 = b/a;								/* k1(Upper level32bit)								*/
		k2 = XasmDIVUA( (b%a), a, 31 );			/* k2(Lower31bit)								*/

/*--------------------------------------------------------------------------------------------------*/
/*		Position FB electronic gear(g1,g2)																*/
/*--------------------------------------------------------------------------------------------------*/
		g1 = a/b;								/* g1(Upper level32bit)								*/
		g2 = XasmDIVUA( (a%b), b, 31 );			/* g2(Lower31bit)								*/

/*--------------------------------------------------------------------------------------------------*/
/*		Electronic gear parameter setting																		*/
/*--------------------------------------------------------------------------------------------------*/
		egear->a  = a;
		egear->b  = b;
		egear->k1 = k1;
		egear->k2 = k2;
		egear->g1 = g1;
		egear->g2 = g2;

		return;

}


#if 0
/****************************************************************************************************/
/*																									*/
/*		Position deviation calculation A(Command unit)																	*/
/*																									*/
/****************************************************************************************************/
void	MlibPerrcala( LONG pcmda, LONG pfbka, EGEAR_DATA *Egear, PERRA *perra )
{
	INT64	wk; 
	LONG	perrx;				/* For position deviation calculation work												*/

#if ( CSW_SVCF_EGEAR_USE == TRUE )
 /* Using electronic gear */

	wk = ((INT64)pfbka * (INT64)Egear->egear_ref) + (INT64)perra->pfbkrem;
	perra->pfbkb = (LONG)( wk / Egear->egear_motor );
	perra->pfbkrem = (LONG)(wk - (perra->pfbkb * Egear->egear_motor) );

#else
/* Not using electronic gear */
	perra->pfbkb = pfbka;
	perra->pfbkrem = 0;

#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation calculation(Command unit)																		*/
/*--------------------------------------------------------------------------------------------------*/
	perrx = pcmda - perra->pfbkb;
	perra->per64.ld64 = perra->per64.ld64 + (INT64)perrx;
	perrx = (perra->per64.ld32.high<<2) + (((ULONG)perra->per64.ld32.low)>>30);
	if( perrx > 0 )
	{
		perra->per32s =  0x3FFFFFFF;
	}
	else if( perrx < -1 )
	{
		perra->per32s = -0x40000000;
	}
	else
	{
		perra->per32s = perra->per64.ld32.low;
	}
	perra->per32a = MlibLABS( perra->per32s );
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation round-up calculation(Command unit)																*/
/*--------------------------------------------------------------------------------------------------*/
	perra->per32sx = perra->per32s;
	if( (perra->pfbkrem < 0)&&( perra->per32s >= 0) )
	{
		perra->per32sx += 1;
	}
	if( (perra->pfbkrem > 0)&&( perra->per32s <= 0) )
	{
		perra->per32sx -= 1;
	}
	perra->per32ax = MlibLABS( perra->per32sx );

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation monitor calculation(Command unit,Round up/zero)													*/
/*--------------------------------------------------------------------------------------------------*/
	if( perra->per32s == 0 )
	{
		perra->per32mx = 0;
	}
	else
	{
		perra->per32mx = perra->per32sx;
	}
}
#endif



void	MlibPerrcala( LONG pcmda, LONG pfbka, EGEAR *egear, PERRA *perra )
{
LONG	perrx;				/* For position deviation calculation work													*/
LONG	carry;				/* Carry of (per64[0]+perrx)											*/
LONG	wk1[2];				/* For work																*/
LONG	wk2[2];				/* For work																*/

/*--------------------------------------------------------------------------------------------------*/
/*		Position FB calculation(Command unit)																		*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( egear->g1, pfbka, wk1 );
		MUL3232( egear->g2, pfbka, wk2 );
		perra->pfbkb = wk1[0] + (wk2[1]<<1) + ((ULONG)wk2[0]>>31);
		MUL3232( egear->a, pfbka, wk1 );
		MUL3232( egear->b, perra->pfbkb, wk2 );
		perra->pfbkrem = perra->pfbkrem + ((ULONG)wk1[0] - (ULONG)wk2[0]);

		if( perra->pfbkrem <  0                 ){ --(perra->pfbkb); perra->pfbkrem += egear->b;}
		if( perra->pfbkrem >= egear->b          ){ ++(perra->pfbkb); perra->pfbkrem -= egear->b;}
		if( perra->pfbkrem >= ((egear->b+1)>>1) ){ ++(perra->pfbkb); perra->pfbkrem -= egear->b;}

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation calculation(Command unit)																		*/
/*--------------------------------------------------------------------------------------------------*/
		perrx = pcmda - perra->pfbkb;
		carry = (((ULONG)(perra->per64[0]+perrx)) < ((ULONG)(perra->per64[0])));
		perra->per64[0] = perra->per64[0] + (perrx);
		perra->per64[1] = perra->per64[1] + (perrx>>31) + carry;
		perrx = (perra->per64[1]<<2) + (((ULONG)perra->per64[0])>>30);
		if( perrx > 0       ) perra->per32s =  0x3FFFFFFF;
		else if( perrx < -1 ) perra->per32s = -0x40000000;
		else                  perra->per32s =  perra->per64[0];
		perra->per32a = (perra->per32s>=0)? perra->per32s : -perra->per32s;

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation round-up calculation(Command unit)																*/
/*--------------------------------------------------------------------------------------------------*/
		perra->per32sx = perra->per32s;
		if( (perra->pfbkrem < 0)&&( perra->per32s >= 0) ) perra->per32sx += 1;
		if( (perra->pfbkrem > 0)&&( perra->per32s <= 0) ) perra->per32sx -= 1;
		perra->per32ax = (perra->per32sx>=0)? perra->per32sx : -perra->per32sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation monitor calculation(Command unit,Round up/zero)													*/
/*--------------------------------------------------------------------------------------------------*/
		perra->per32mx = (perra->per32s==0)? 0 : perra->per32sx;

		return;
}


#if (FLOAT_USE==TRUE)
void	FlibPerrcala( LONG pcmda, LONG pfbka, EGEAR *egear, PERRA *perra )
{
LONG	perrx;				/* For position deviation calculation work													*/
LONG	carry;				/* Carry of (per64[0]+perrx)											*/
LONG	wk1;				/* For work																*/
LONG	wk2;				/* For work															*/

/*--------------------------------------------------------------------------------------------------*/
/*		Position FB calculation(Command unit)																		*/
/*--------------------------------------------------------------------------------------------------*/

		perra->pfbkb = (LONG)((pfbka * egear->f_a) / egear->f_b);

		wk1 = (pfbka * egear->a);		/* The following is the lower order32Bitmust be calculated normally 				*/
		wk2 = (perra->pfbkb * egear->b);/* (Upper level32Bitcan be ignored.)								*/
		perra->pfbkrem = perra->pfbkrem + ((ULONG)wk1 - (ULONG)wk2);

		if( perra->pfbkrem <  0                 ){ --(perra->pfbkb); perra->pfbkrem += egear->b;}
		if( perra->pfbkrem >= egear->b          ){ ++(perra->pfbkb); perra->pfbkrem -= egear->b;}
		if( perra->pfbkrem >= ((egear->b+1)>>1) ){ ++(perra->pfbkb); perra->pfbkrem -= egear->b;}

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation calculation(Command unit)																		*/
/*--------------------------------------------------------------------------------------------------*/
		perrx = pcmda - perra->pfbkb;
		carry = (((ULONG)(perra->per64[0]+perrx)) < ((ULONG)(perra->per64[0])));
		perra->per64[0] = perra->per64[0] + (perrx);
		perra->per64[1] = perra->per64[1] + (perrx>>31) + carry;
		perrx = (perra->per64[1]<<2) + (((ULONG)perra->per64[0])>>30);
		if( perrx > 0       ) perra->per32s =  0x3FFFFFFF;
		else if( perrx < -1 ) perra->per32s = -0x40000000;
		else                  perra->per32s =  perra->per64[0];
		perra->per32a = (perra->per32s>=0)? perra->per32s : -perra->per32s;

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation round-up calculation(Command unit)																*/
/*--------------------------------------------------------------------------------------------------*/
		perra->per32sx = perra->per32s;
		if( (perra->pfbkrem < 0)&&( perra->per32s >= 0) ) perra->per32sx += 1;
		if( (perra->pfbkrem > 0)&&( perra->per32s <= 0) ) perra->per32sx -= 1;
		perra->per32ax = (perra->per32sx>=0)? perra->per32sx : -perra->per32sx;

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation monitor calculation(Command unit,Round up/zero)													*/
/*--------------------------------------------------------------------------------------------------*/
		perra->per32mx = (perra->per32s==0)? 0 : perra->per32sx;

		return;
}
#endif /* FLOAT_USE */

/****************************************************************************************************/
/*																									*/
/*		Position deviation calculation(FB unit)		per64[]=per64[]+pcmd-pfbk; rv=per64[0];						*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPerrcalx( LONG pcmdx, LONG pfbkx, LONG per64[2] )
{
LONG	perrx;				/* Pulse deviation															*/
LONG	carry;				/* Carry of (per64[0]+perrx)											*/
LONG	perrchk;			/* For pulse deviation check													*/

		perrx = pcmdx - pfbkx;
		carry = (((ULONG)(per64[0]+perrx)) < ((ULONG)(per64[0])));

		per64[0] = per64[0] + (perrx);
		per64[1] = per64[1] + (perrx>>31) + carry;

		perrchk = (per64[1]<<2) + (((ULONG)per64[0])>>30);
		if( perrchk >  0 ){ return(  0x3FFFFFFF );}
		if( perrchk < -1 ){ return( -0x40000000 );}

		return( per64[0] );

}



#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		Position deviation calculation(FB unit)		per64[]=per64[]+pcmd-pfbk; rv=per64[0];						*/
/*																									*/
/****************************************************************************************************/
LONG	FlibPerrcalx( LONG pcmdx, LONG pfbkx, LONG per64[2] )	/* Temporary treatment	*/
{
LONG	perrx;				/* Pulse deviation															*/
LONG	carry;				/* Carry of (per64[0]+perrx)											*/
LONG	perrchk;			/* For pulse deviation check													*/

		perrx = pcmdx - pfbkx;
		carry = (((ULONG)(per64[0]+perrx)) < ((ULONG)(per64[0])));

		per64[0] = per64[0] + (perrx);
		per64[1] = per64[1] + (perrx>>31) + carry;

		perrchk = (per64[1]<<2) + (((ULONG)per64[0])>>30);
		if( perrchk >  0 ){ return(  0x3FFFFFFF );}
		if( perrchk < -1 ){ return( -0x40000000 );}

		return( per64[0] );

}
#endif /* FLOAT_USE */


/****************************************************************************************************/
/*																									*/
/*		Initial absolute position calculation(Reverse electronic gear: FB unit-->Command unit)											*/
/*																									*/
/****************************************************************************************************/
void	MlibAposRg64iv( LONG inipos0, LONG inipos1, EGEAR *egear, APOSRG *aposrg )
{
LONG	i;
LONG	sign;				/* Sign of inipos1														*/
LONG	carry;				/* Carry																*/
LONG	xx[4];				/* For work																*/
LONG	y0,y1;				/* For work																*/
LONG	b = egear->b;		/* Electronic gear B															*/

/*--------------------------------------------------------------------------------------------------*/
/*		Sign check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( inipos1 >= 0 )
		{
			sign = 0;
		}
		else
		{
			sign = 1;
			inipos0 = (~inipos0) + 1;
			inipos1 = (~inipos1) + (inipos0 == 0);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication(64bit*32bit)																			*/
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( (ULONG)inipos0, (ULONG)egear->a, (ULONG*)&xx[0] );
		MULU32U32( (ULONG)inipos1, (ULONG)egear->a, (ULONG*)&xx[2] );
		carry = ( (ULONG)xx[2] > ((ULONG)xx[2]+(ULONG)xx[1]) );
		xx[2] = xx[2] + xx[1];
		xx[3] = xx[3] + carry;
/*--------------------------------------------------------------------------------------------------*/
/*		Division(96bit/32bit)																			*/
/*--------------------------------------------------------------------------------------------------*/
		for( y1=0,i=0; i<32; i++ )
		{
			y1 = (y1<<1);
			xx[3] = (xx[3]<<1) + ((ULONG)xx[2]>>31);
			xx[2] = (xx[2]<<1);
			if( (ULONG)xx[3] >= (ULONG)b )
			{
				y1 = y1 + 1;
				xx[3] = (ULONG)xx[3] - (ULONG)b;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		for( y0=0,i=0; i<32; i++ )
		{
			y0 = (y0<<1);
			xx[3] = (xx[3]<<1) + ((ULONG)xx[0]>>31);
			xx[0] = (xx[0]<<1);
			if( (ULONG)xx[3] >= (ULONG)b )
			{
				y0 = y0 + 1;
				xx[3] = (ULONG)xx[3] - (ULONG)b;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		if( xx[3] >= ((b+1)>>1) )
		{
			xx[3] = xx[3] - b;
			y0    = y0 + 1;
			y1    = y1 + (y0==0);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Computation result storage																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( sign != 0 )
		{
			y0 = (~y0) + 1;
			y1 = (~y1) + (y0 == 0);
			xx[3] = -xx[3];
		}
		aposrg->dposix  = 0;				/* Difference position						*/
		aposrg->aposrem = xx[3];			/* Position surplus						*/
		aposrg->apos[0] = y0;				/* Absolute position lower32bit			*/
		aposrg->apos[1] = y1;				/* Absolute position top32bit			*/

		return;

}



/****************************************************************************************************/
/*																									*/
/*		Absolute position update calculation(Reverse electronic gear: FB unit-->Command unit)											*/
/*																									*/
/****************************************************************************************************/
void	MlibAposRg64dp( LONG dposin, EGEAR *egear, APOSRG *aposrg, LONG *latchpos )
{
LONG	carry;							/* Carry													*/
LONG	wk1[2];							/* For work													*/
LONG	wk2[2];							/* For work													*/
LONG	dposix;							/* Difference position(Command unit)										*/
LONG	aposrem = aposrg->aposrem;		/* Absolute position remainder												*/

/*--------------------------------------------------------------------------------------------------*/
/*		Reverse electronic gear calculation(Command unit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( egear->g1, dposin, &wk1[0] );
		MUL3232( egear->g2, dposin, &wk2[0] );
		dposix = wk1[0] + (wk2[1]<<1) + ((ULONG)wk2[0]>>31);
		MUL3232( egear->a, dposin, &wk1[0] );
		MUL3232( egear->b, dposix, &wk2[0] );
		aposrem = aposrem + ((ULONG)wk1[0] - (ULONG)wk2[0]);
/*--------------------------------------------------------------------------------------------------*/
/*		Position surplus processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( aposrem <  0                 ){ --dposix; aposrem += egear->b;}
		if( aposrem >= egear->b          ){ ++dposix; aposrem -= egear->b;}
		if( aposrem >= ((egear->b+1)>>1) ){ ++dposix; aposrem -= egear->b;}
/*--------------------------------------------------------------------------------------------------*/
/*		Absolute position update calculation(Command unit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( latchpos == NULL )
		{
			aposrg->dposix  = dposix;
			aposrg->aposrem = aposrem;
			aposrg->apos[0] = aposrg->apos[0] + dposix;
			carry = ((ULONG)aposrg->apos[0] < (ULONG)dposix)? 1 : 0;
			aposrg->apos[1] = aposrg->apos[1] + (dposix>>31) + carry;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Absolute position latch calculation(Command unit)																*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			latchpos[0] = aposrg->apos[0] + dposix;
			carry = ((ULONG)latchpos[0] < (ULONG)aposrg->apos[0])? 1 : 0;
			latchpos[1] = aposrg->apos[1] + (dposix>>31) + carry;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;

}



/****************************************************************************************************/
/*																									*/
/*																									*/
/*		<<<<< Position command generator function >>>>>																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Position command generator initial parameter calculation															*/
/*																									*/
/****************************************************************************************************/
LONG	MlibIpcalPcmdMaker(
		KSGAIN	ksosvp,					/* OS pulse speed					[xpulse/scan]			*/
		LONG	maxspdm,				/* Maximum motor speed					[2^24/OvrSpd]			*/
		LONG	maxspdi,				/* Interpolation mode maximum speed				[2^24/OvrSpd]			*/
		LONG	pcmd64f,				/* Position command 64-bit flag			[TRUE/FALSE]			*/
		PCMKP	*pcmkprm	)			/* Calculation result output structure pointer		[-]						*/
{
LONG	kx,sx;
ULONG	xx[2];
ULONG	yy[2];

/*--------------------------------------------------------------------------------------------------*/
/*		OS motor pulse speed[xpulse/scan]															*/
/*--------------------------------------------------------------------------------------------------*/
		//sx = (ksosvp >> 24) + 1;
		//kx = ((ksosvp << 8 ) >> 7);
		sx = ((LONG)ksosvp >> 24) + 1;		
		kx = (((LONG)ksosvp << 8 ) >> 7);
		if( sx > 16 )
		{
			kx = (kx >> (sx - 16));
			sx = 16;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Maximum motor pulse speed[xpulse/scan] : maxvpm = osvpm * maxspdm / 2^24							*/
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( (kx << 8), maxspdm, xx );

/*--------------------------------------------------------------------------------------------------*/
/*		Interpolation mode maximum pulse speed[xpulse/scan] : maxipv = osvpm * maxspdi / 2^24						*/
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( (kx << 8), maxspdi, yy );

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter settings																				*/
/*--------------------------------------------------------------------------------------------------*/
		pcmkprm->osvpm   = kx;
		pcmkprm->maxvpm  = xx[1];
		pcmkprm->maxipv  = yy[1];
		pcmkprm->pshlx   = (UCHAR)sx;
		pcmkprm->pcmd64f = (UCHAR)pcmd64f;

		return( TRUE );

}



/****************************************************************************************************/
/*																									*/
/*		Position command creator positioning parameter calculation A														*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalaPcmdMaker(
		LONG	maxspd,					/* Maximum speed							[2^24/OvrSpd]			*/
		LONG	acctime,				/* Acceleration time							[xs]					*/
		LONG	dectime,				/* Deceleration time							[xs]					*/
		LONG	scantime,				/* Scan time					[xs]					*/
		PCMKP	*pcmkprm	)			/* Calculation result output structure pointer		[-]						*/
{
LONG	maxvp;
ULONG	xx[2];
PCMKP	wkprm;

/*--------------------------------------------------------------------------------------------------*/
/*		Maximum position command speed[xpulse/scan] : maxvp = osvpm * maxspd / 2^24								*/
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( (pcmkprm->osvpm << 8), maxspd, xx );
		maxvp = (xx[1] > 0)? xx[1] : 0;
/*--------------------------------------------------------------------------------------------------*/
/*		Position command acceleration[xpulse/scan/scan] : vpacc = maxvp * scantime / acctime						*/
/*--------------------------------------------------------------------------------------------------*/
		wkprm.vpacc = MlibScalKxgain( maxvp, scantime, acctime, NULL, -24 );
		if( wkprm.vpacc <= 0 ){ wkprm.vpacc = 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		Position command deceleration[xpulse/scan/scan] : vpdec = maxvp * scantime / dectime						*/
/*--------------------------------------------------------------------------------------------------*/
		wkprm.vpdec = MlibScalKxgain( maxvp, scantime, dectime, NULL, -24 );
		if( wkprm.vpdec <= 0 ){ wkprm.vpdec = 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		Quotient(vpamx), Remainder(vparx), Amount of acceleration deceleration and stop pulses(vpapx)										*/
/*--------------------------------------------------------------------------------------------------*/
/*		vpamx = vpacc / vpdec																		*/
/*		vparx = vpacc % vpdec																		*/
/*		vpapx = (vpamx + 1) * (vpacc + vparx) / 2													*/
/*--------------------------------------------------------------------------------------------------*/
		wkprm.vpamx = wkprm.vpacc / wkprm.vpdec;
		wkprm.vparx = wkprm.vpacc - (wkprm.vpamx*wkprm.vpdec);
		if( wkprm.vpamx & 0x0001 )
		{
			wkprm.vpapx = ((wkprm.vpamx+1)>>1)*(wkprm.vpacc+wkprm.vparx);
		}
		else
		{
			wkprm.vpapx = (wkprm.vpamx+1)*((wkprm.vpacc+wkprm.vparx)>>1);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter settings																				*/
/*--------------------------------------------------------------------------------------------------*/
//		KPI_DI( );		/* 20130828Consider commenting out to prevent interruptions during version initialization.*/
		pcmkprm->vpacc  = wkprm.vpacc;
		pcmkprm->vpdec  = wkprm.vpdec;
		pcmkprm->vpamx  = wkprm.vpamx;
		pcmkprm->vparx  = wkprm.vparx;
		pcmkprm->vpapx  = wkprm.vpapx;
		pcmkprm->maxspd = maxspd;
//		KPI_EI( );		/* 20130828Consider commenting out to prevent interruptions during version initialization.*/
/*--------------------------------------------------------------------------------------------------*/
		return( TRUE );

}



#if 0 /* Currently unused */
/****************************************************************************************************/
/*																									*/
/*		Position command generator interpolation calculation parameter calculation A														*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalaPcmdMkrIP(				/* Position command generator interpolation calculation parameter calculation A					*/
		LONG	iptimes,				/* Interpolation number							[-]						*/
		LONG	maxspd,					/* Maximum speed							[2^24/OvrSpd]			*/
		LONG	ipacctm,				/* Minimum acceleration/deceleration time					[xs]					*/
		LONG	ipdectm,				/* Emergency stop deceleration time					[xs]					*/
		LONG	scantime,				/* Scan time					[xs]					*/
		PCMKP	*pcmkprm	)			/* Calculation result output structure pointer		[-]						*/
{
LONG	xx[2];
LONG	kx,sx,sy;
LONG	ipmaxacc;
LONG	ipstpdec;
LONG	IPcycle = scantime * iptimes;

/*--------------------------------------------------------------------------------------------------*/
/*		Common term calculation : Interpolation mode maximum acceleration/deceleration & emergency stop deceleration											*/
/*--------------------------------------------------------------------------------------------------*/
/*					osvpm * maxspd * IPcycle														*/
/*		{kx,sx} = ----------------------------														*/
/*						 2^pshlx * 2^24																*/
/*--------------------------------------------------------------------------------------------------*/
		sx = pcmkprm->pshlx + 24;
		kx = MlibPcalKxmulx( maxspd, pcmkprm->osvpm, IPcycle, &sx );
/*--------------------------------------------------------------------------------------------------*/
/*		Interpolation mode maximum speed [ipulse/IPcycle]															*/
/*--------------------------------------------------------------------------------------------------*/
/*					 maxipv * iptimes																*/
/*		ipmaxspd = --------------------																*/
/*						 2^pshlx																	*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( pcmkprm->maxipv, iptimes, &xx[0] );
		xx[0] = ((xx[1]<<(31-pcmkprm->pshlx))<<1) + ((ULONG)xx[0]>>pcmkprm->pshlx);
/*--------------------------------------------------------------------------------------------------*/
/*		Interpolation mode maximum acceleration/deceleration [ipulse/IPcycle^2]													*/
/*--------------------------------------------------------------------------------------------------*/
/*					 osvpm * maxspd * IPcycle * iptimes			  iptimes							*/
/*		ipmaxacc = -------------------------------------- = kx * ---------							*/
/*						  2^pshlx * 2^24 * ipacctm				  ipacctm							*/
/*--------------------------------------------------------------------------------------------------*/
		if( ipacctm <= 0 )
		{
			ipmaxacc = xx[0];
		}
		else
		{
			sy = sx;
			ipmaxacc = MlibPcalKxgain( kx, iptimes, ipacctm, &sy, -24 );
			if( ipmaxacc <= 0 ){ ipmaxacc = 1;}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Interpolation mode emergency stop deceleration [ipulse/IPcycle^2]													*/
/*--------------------------------------------------------------------------------------------------*/
/*					 osvpm * maxspd * IPcycle * iptimes			  iptimes							*/
/*		ipstpdec = -------------------------------------- = kx * ---------							*/
/*						  2^pshlx * 2^24 * ipdectm				  ipdectm							*/
/*--------------------------------------------------------------------------------------------------*/
		if( ipdectm <= 0 )
		{
			ipstpdec = xx[0];
		}
		else
		{
			sy = sx;
			ipstpdec = MlibPcalKxgain( kx, iptimes, ipdectm, &sy, -24 );
			if( ipstpdec <= 0 ){ ipstpdec = 1;}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter settings																				*/
/*--------------------------------------------------------------------------------------------------*/
		KPI_DI( );
		pcmkprm->iptimes  = (USHORT)iptimes;
		pcmkprm->ipmaxspd = xx[0];
		pcmkprm->ipmaxacc = ipmaxacc;
		pcmkprm->ipstpdec = ipstpdec;
		KPI_EI( );
/*--------------------------------------------------------------------------------------------------*/
		return( TRUE );

}
#endif



/****************************************************************************************************/
/*																									*/
/*		Position command creator positioning parameter calculation B														*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalbPcmdMaker(
		LONG	accrate,				/* Acceleration rate					[10000pulse/s/s]			*/
		LONG	decrate,				/* Deceleration rate					[10000pulse/s/s]			*/
		LONG	scantime,				/* Scan time				[us(Y=0)/ns(Y=1)]			*/
		PCMKP	*pcmkprm,				/* Calculation result output structure pointer	[-]							*/
		LONG	insel		)			/* Input selection(0xYX)				[X:Not used, Y:us/nsSelection]		*/
{
LONG	kx,sx;
PCMKP	wkprm;
LONG	kunit = ((insel & 0xF0)==0)? 1 : 1000;

/*--------------------------------------------------------------------------------------------------*/
/*		Common term calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
/*			   10000 * scantime * scantime * 2^pshlx												*/
/*		kx = -----------------------------------------												*/
/*					kunit * kunit * 10^6 * 10^6														*/
/*--------------------------------------------------------------------------------------------------*/
		sx = pcmkprm->pshlx; kx = MlibPcalKxmulx( kunit, kunit, 100000000, &sx );
		sx = -sx;			 kx = MlibPcalKxgain( scantime, scantime, kx, &sx, -1 );
/*--------------------------------------------------------------------------------------------------*/
/*		Position command acceleration[xpulse/scan/scan]															*/
/*--------------------------------------------------------------------------------------------------*/
/*				 10000 * scantime * scantime * 2^pshlx												*/
/*		vpacc = --------------------------------------- * accrate									*/
/*					kunit * kunit * 10^6 * 10^6														*/
/*--------------------------------------------------------------------------------------------------*/
		wkprm.vpacc = MlibScalKskxkx( kx, accrate, 1, NULL, -24 );
		if( wkprm.vpacc <= 0 ){ wkprm.vpacc = 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		Position command deceleration[xpulse/scan/scan]															*/
/*--------------------------------------------------------------------------------------------------*/
/*				 10000 * scantime * scantime * 2^pshlx												*/
/*		vpdec = --------------------------------------- * decrate									*/
/*					kunit * kunit * 10^6 * 10^6														*/
/*--------------------------------------------------------------------------------------------------*/
		wkprm.vpdec = MlibScalKskxkx( kx, decrate, 1, NULL, -24 );
		if( wkprm.vpdec <= 0 ){ wkprm.vpdec = 1;}
/*--------------------------------------------------------------------------------------------------*/
/*		Quotient(vpampx), Remainder(vparx), Amount of acceleration deceleration and stop pulses(vpapx)										*/
/*--------------------------------------------------------------------------------------------------*/
/*		vpamx = vpacc / vpdec																		*/
/*		vparx = vpacc % vpdec																		*/
/*		vpapx = (vpamx + 1) * (vpacc + vparx) / 2													*/
/*--------------------------------------------------------------------------------------------------*/
		wkprm.vpamx = wkprm.vpacc / wkprm.vpdec;
		wkprm.vparx = wkprm.vpacc - (wkprm.vpamx*wkprm.vpdec);
		if( wkprm.vpamx & 0x0001 )
		{
			wkprm.vpapx = ((wkprm.vpamx+1)>>1)*(wkprm.vpacc+wkprm.vparx);
		}
		else
		{
			wkprm.vpapx = (wkprm.vpamx+1)*((wkprm.vpacc+wkprm.vparx)>>1);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter settings																				*/
/*--------------------------------------------------------------------------------------------------*/
		KPI_DI( );
		pcmkprm->vpacc = wkprm.vpacc;
		pcmkprm->vpdec = wkprm.vpdec;
		pcmkprm->vpamx = wkprm.vpamx;
		pcmkprm->vparx = wkprm.vparx;
		pcmkprm->vpapx = wkprm.vpapx;
		KPI_EI( );
/*--------------------------------------------------------------------------------------------------*/
		return( TRUE );

}



#if 0 /* Currently unused */
/****************************************************************************************************/
/*																									*/
/*		Position command generator interpolation calculation parameter calculation B														*/
/*																									*/
/****************************************************************************************************/
LONG	MlibPcalbPcmdMkrIP(				/* Position command generator interpolation calculation parameter calculation B					*/
		LONG	iptimes,				/* Interpolation number						[-]							*/
		LONG	ipaccrt,				/* Maximum acceleration/deceleration rate				[10000pulse/s/s]			*/
		LONG	ipdecrt,				/* Emergency stop deceleration rate			[10000pulse/s/s]			*/
		LONG	scantime,				/* Scan time				[us(Y=0)/ns(Y=1)]			*/
		PCMKP	*pcmkprm,				/* Calculation result output structure pointer	[--]						*/
		LONG	insel		)			/* Input selection(0xYX)				[X:Not used, Y:us/nsSelection]		*/
{
LONG	xx[2];
LONG	kx,sx,sy;
LONG	ipmaxacc;
LONG	ipstpdec;
LONG	IPcycle = scantime * iptimes;
LONG	kunit = ((insel & 0xF0)==0)? 1 : 1000;

/*--------------------------------------------------------------------------------------------------*/
/*		Common term calculation : Interpolation mode maximum acceleration/deceleration & emergency stop deceleration											*/
/*--------------------------------------------------------------------------------------------------*/
/*						10000 * IPcycle * IPcycle													*/
/*		{kx,sx} = -------------------------------------												*/
/*					1000000 * 1000000 * kunit * kunit												*/
/*--------------------------------------------------------------------------------------------------*/
		sx = 0;   kx = MlibPcalKxmulx( kunit, kunit, 100000000, &sx );
		sx = -sx; kx = MlibPcalKxgain( IPcycle, IPcycle, kx, &sx, 0 );
/*--------------------------------------------------------------------------------------------------*/
/*		Interpolation mode maximum speed [ipulse/IPcycle]															*/
/*--------------------------------------------------------------------------------------------------*/
/*					 maxipv * iptimes																*/
/*		ipmaxspd = --------------------																*/
/*						 2^pshlx																	*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( pcmkprm->maxipv, iptimes, &xx[0] );
		xx[0] = ((xx[1]<<(31-pcmkprm->pshlx))<<1) + ((ULONG)xx[0]>>pcmkprm->pshlx);
/*--------------------------------------------------------------------------------------------------*/
/*		Interpolation mode maximum acceleration/deceleration [ipulse/IPcycle^2]													*/
/*--------------------------------------------------------------------------------------------------*/
/*						 10000 * IPcycle * IPcycle													*/
/*		ipmaxacc = ------------------------------------ * ipaccrt									*/
/*					1000000 * 1000000 * kunit * kunit												*/
/*--------------------------------------------------------------------------------------------------*/
		if( ipaccrt <= 0 )
		{
			ipmaxacc = xx[0];
		}
		else
		{
			sy = sx;
			ipmaxacc = MlibPcalKxgain( kx, ipaccrt, 1, &sy, -24 );
			if( ipmaxacc <= 0 ){ ipmaxacc = 1;}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Interpolation mode emergency stop deceleration [ipulse/IPcycle^2]													*/
/*--------------------------------------------------------------------------------------------------*/
/*						 10000 * IPcycle * IPcycle													*/
/*		ipstpdec = ------------------------------------- * ipdecrt									*/
/*					 1000000 * 1000000 * kunit * kunit												*/
/*--------------------------------------------------------------------------------------------------*/
		if( ipdecrt <= 0 )
		{
			ipstpdec = xx[0];
		}
		else
		{
			sy = sx;
			ipstpdec = MlibPcalKxgain( kx, ipdecrt, 1, &sy, -24 );
			if( ipstpdec <= 0 ){ ipstpdec = 1;}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter settings																				*/
/*--------------------------------------------------------------------------------------------------*/
		KPI_DI( );
		pcmkprm->iptimes  = (USHORT)iptimes;
		pcmkprm->ipmaxspd = xx[0];
		pcmkprm->ipmaxacc = ipmaxacc;
		pcmkprm->ipstpdec = ipstpdec;
		KPI_EI( );
/*--------------------------------------------------------------------------------------------------*/
		return( TRUE );

}
#endif



/****************************************************************************************************/
/*																									*/
/*		Position command creator reset processing																	*/
/*																									*/
/****************************************************************************************************/
void	MlibRstPcmdMaker(
		LONG	pcmdset0,				/* Position command initial value(Lower32bit)		[pulse]					*/
		LONG	pcmdset1,				/* Position command initial value(Upper level32bit)		[pulse]					*/
		PCMKPV	*pcmdmkr	)			/* Position command generator calculation variable(P&V)	[-]						*/
{

		KPI_DI( );
/*--------------------------------------------------------------------------------------------------*/
		pcmdmkr->V.vpx        = 0;
		pcmdmkr->V.vpxrem     = 0;
		pcmdmkr->V.ipvpi      = 0;
		pcmdmkr->V.ipvpx      = 0;
		pcmdmkr->V.ipremi     = 0;
		pcmdmkr->V.ipremx     = 0;
/*--------------------------------------------------------------------------------------------------*/
		pcmdmkr->V.calendf    = 0;
		pcmdmkr->V.cmderrf    = 0;
		pcmdmkr->V.pcmkmode   = 0;
		pcmdmkr->V.modechgf   = 0;
/*--------------------------------------------------------------------------------------------------*/
		pcmdmkr->V.pcmdout[0] = pcmdset0;
		pcmdmkr->V.pcmdout[1] = pcmdset1;
/*--------------------------------------------------------------------------------------------------*/
		KPI_EI( );
		return;

}



#if 0 /* Currently unused */
/****************************************************************************************************/
/*																									*/
/*		Position command generator: Interpolation position command limit calculation													*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		64bit Compare Macro : GT(a>b),LT(a<b),GTE(a>=b),LTE(a<=b)									*/
/*--------------------------------------------------------------------------------------------------*/
#define	LPX_EQ64( a, b )	( (a[1] == b[1]) && (a[0] == b[0]) )
#define	LPX_GT64( a, b )	( (a[1] >  b[1]) || ((a[1] == b[1])&&((ULONG)a[0] >  (ULONG)b[0])) )
#define	LPX_LT64( a, b )	( (a[1] <  b[1]) || ((a[1] == b[1])&&((ULONG)a[0] <  (ULONG)b[0])) )
/*--------------------------------------------------------------------------------------------------*/
LONG	MlibIpTposLimit(				/* Interpolation position command limit calculation									*/
		LONG	*tpos,					/* Target position(64bit)					[pulse]					*/
		LONG	*lmtpos,				/* Limit position(64bit)				[pulse]					*/
		LONG	lmtdir,					/* Limit direction(Positive side/negative side)			[-]						*/
		PCMKPV	*pcmdmkr	)			/* Position command generator calculation variable(P&V)	[-]						*/
{
LONG	carry;							/* 64bitOperationCarryFor calculation										*/
LONG	x1,x2;							/* For position deviation calculation											*/
LONG	px64[2];						/* For position deviation calculation											*/
PCMKP	*pcmkprm = &pcmdmkr->P;			/* Position command creator parameters									*/
PCMKV	*pcmkvar = &pcmdmkr->V;			/* Position command generator calculation variable									*/

/*--------------------------------------------------------------------------------------------------*/
/*		Position command creation mode change check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( pcmkvar->pcmkmode != PCMKMODE_IPA )
		{
			MUL3232( pcmkvar->vpx, (pcmkprm->iptimes<<1), &px64[0] );
			x1 = ((px64[1]<<(31-pcmkprm->pshlx))<<1) + ((ULONG)px64[0]>>pcmkprm->pshlx);
			pcmkvar->ipvpi = (x1 + 1) >> 1;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Deceleration stop(Zero speed)Calculation of position command pulse amount up to												*/
/*--------------------------------------------------------------------------------------------------*/
		pcmkvar->avp = (pcmkvar->ipvpi >= 0) ? pcmkvar->ipvpi : -pcmkvar->ipvpi;
		pcmkvar->n   = pcmkvar->avp / pcmkprm->ipstpdec;
		pcmkvar->rem = pcmkvar->avp - pcmkvar->n * pcmkprm->ipstpdec;
		if( ((pcmkvar->n+1)&0x01) == 0x00 )
		{
			MUL3232( ((pcmkvar->n + 1)>>1), (pcmkvar->avp + pcmkvar->rem), &px64[0] );
		}
		else
		{
			MUL3232( (pcmkvar->n + 1), ((pcmkvar->avp + pcmkvar->rem)>>1), &px64[0] );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of deceleration and stop position																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( pcmkvar->ipvpi >= 0 )
		{
			pcmkvar->ipstopx[0] = pcmkvar->pcmdout[0] + px64[0];
			carry = ((ULONG)pcmkvar->ipstopx[0] < (ULONG)px64[0]);
			pcmkvar->ipstopx[1] = pcmkvar->pcmdout[1] + px64[1] + carry;
		}
		else
		{
			pcmkvar->ipstopx[0] = pcmkvar->pcmdout[0] - px64[0];
			carry = ((ULONG)pcmkvar->pcmdout[0] < (ULONG)px64[0]);
			pcmkvar->ipstopx[1] = pcmkvar->pcmdout[1] - px64[1] - carry;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of position deviation during deceleration and stop : lmtpos[] - ipstopx[]											*/
/*--------------------------------------------------------------------------------------------------*/
		px64[0] = lmtpos[0] - pcmkvar->ipstopx[0];
		carry = ((ULONG)lmtpos[0] < (ULONG)pcmkvar->ipstopx[0]);
		px64[1] = lmtpos[1] - pcmkvar->ipstopx[1] - carry;

/*--------------------------------------------------------------------------------------------------*/
/*		Area check: Position deviation,Speed,etc																*/
/*--------------------------------------------------------------------------------------------------*/
		if( LPX_EQ64( tpos, pcmkvar->pcmdout ) )
		{
			return( FALSE );
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( lmtdir == IPTPOSLMT_PSIDE )
		{
			if( pcmkvar->ipvpi < 0 )
			{
				if( LPX_GT64( tpos, lmtpos ) )
				{
					tpos[0] = lmtpos[0];
					tpos[1] = lmtpos[1];
					return( TRUE );
				}
				return( FALSE );
			}
		/*------------------------------------------------------------------------------------------*/
			if( (px64[1] > 0) || ((px64[1]==0)&&(px64[0]<0)) )
			{
				return( FALSE );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			if( pcmkvar->ipvpi > 0 )
			{
				if( LPX_LT64( tpos, lmtpos ) )
				{
					tpos[0] = lmtpos[0];
					tpos[1] = lmtpos[1];
					return( TRUE );
				}
				return( FALSE );
			}
		/*------------------------------------------------------------------------------------------*/
			px64[0] = ~px64[0] + 1;
			px64[1] = ~px64[1] + (px64[0]==0);
			if( (px64[1] > 0) || ((px64[1]==0)&&(px64[0]<0)) )
			{
				return( FALSE );
			}
		}

/*--------------------------------------------------------------------------------------------------*/
/*		When acceleration is possible																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( px64[0] > pcmkvar->n )
		{
			if( px64[0] <= (pcmkvar->avp + pcmkprm->ipstpdec) )
			{
				x1 = pcmkprm->ipstpdec - pcmkvar->rem;
				x2 = x1 * (pcmkvar->n + 1);
				if( px64[0] <= x2 )
				{
					pcmkvar->avp += (px64[0]/(pcmkvar->n + 1));
				}
				else
				{
					pcmkvar->avp += x1 + ((px64[0] - x2)/(pcmkvar->n + 2));
				}
			}
		/*------------------------------------------------------------------------------------------*/
			else
			{
				x1 = pcmkvar->avp;
			/*--------------------------------------------------------------------------------------*/
				x2 = (4*x1) + (10*pcmkprm->ipstpdec);
				if( px64[0] >= x2 )
				{
					px64[0] -= x2; x1 += 4*pcmkprm->ipstpdec;
				}
			/*--------------------------------------------------------------------------------------*/
				x2 = x1 + pcmkprm->ipstpdec;
				if( px64[0] >= x2 )
				{
					px64[0] -= x2; x1 = x2;
				}
			/*--------------------------------------------------------------------------------------*/
				x2 = x1 + pcmkprm->ipstpdec;
				if( px64[0] >= x2 )
				{
					px64[0] -= x2; x1 = x2;
				}
			/*--------------------------------------------------------------------------------------*/
				x2 = x1 + pcmkprm->ipstpdec;
				if( px64[0] >= x2 )
				{
					px64[0] -= x2; x1 = x2;
				}
			/*--------------------------------------------------------------------------------------*/
				x2 = x1 + pcmkprm->ipstpdec;
				if( px64[0] >  x2 )
				{
					pcmkvar->avp = x1;
					return( FALSE );
				}
				else
				{
					pcmkvar->avp = x1;
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		When deceleration is required																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( px64[0] < 0 )
		{
			if( px64[0] <= -pcmkvar->avp )
			{
				pcmkvar->avp -= pcmkprm->ipstpdec;
				if( pcmkvar->avp < 0 ){ pcmkvar->avp = 0;}
			}
			else if( px64[0] <= -((pcmkvar->n + 1)*pcmkvar->rem) )
			{
				pcmkvar->avp -= pcmkprm->ipstpdec - ((px64[0] + pcmkvar->avp)/pcmkvar->n);
			}
			else
			{
				pcmkvar->avp -= ((pcmkvar->n - px64[0])/(pcmkvar->n+1));
			}
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Position command limit value calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
		x1 = (lmtdir == IPTPOSLMT_PSIDE)? pcmkvar->avp : -pcmkvar->avp;
		px64[0] = pcmkvar->pcmdout[0] + x1;
		carry = ((ULONG)px64[0] < (ULONG)x1);
		px64[1] = pcmkvar->pcmdout[1] + (x1>>31) + carry;

/*--------------------------------------------------------------------------------------------------*/
/*		Positive side limit processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( lmtdir == IPTPOSLMT_PSIDE )
		{
			if( LPX_GT64( px64, lmtpos ) )
			{
				px64[0] = lmtpos[0];
				px64[1] = lmtpos[1];
			}
			if( LPX_GT64( tpos, px64 ) )
			{
				tpos[0] = px64[0];
				tpos[1] = px64[1];
				return( TRUE );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Negative side limit processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			if( LPX_LT64( px64, lmtpos ) )
			{
				px64[0] = lmtpos[0];
				px64[1] = lmtpos[1];
			}
			if( LPX_LT64( tpos, px64 ) )
			{
				tpos[0] = px64[0];
				tpos[1] = px64[1];
				return( TRUE );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( FALSE );
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Position command generator calculation execution function																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Absolute value macro																				*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_ABS( u )	(((u)>=0) ? (u) : -(u))
/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication(32bit*32bit,Truncation calculation)(ASM calculation replacement)												*/
/*--------------------------------------------------------------------------------------------------*/
LONG	XasmMulhigh32down( LONG a, LONG b )
{
LONG	xx[2];
		MUL3232( a, b, xx );
		return( xx[1] );
}
/*--------------------------------------------------------------------------------------------------*/


LONG	MlibPcmdMaker(
		LONG	pcmdin0,				/* Position command input(Lower32bit)			[pulse]					*/
		LONG	pcmdin1,				/* Position command input(Upper level32bit)			[pulse]					*/
		LONG	pcmdspd,				/* Position command speed(Maximum/feed)			[2^24/OvrSpd]			*/
		PCMKPV	*pcmdmkr,				/* Position command generator calculation variable(P&V)	[-]						*/
		ULONG	pcmkmode	)			/* Position command creation mode				[-]						*/
{
LONG	carry;							/* 64bitOperationCarryFor calculation										*/
LONG	px64[2];						/* Pulse amount until deceleration and stop									*/
LONG	per64[2];						/* Pulse deviation(64bitSigned)									*/
LONG	pex64[2];						/* Pulse deviation during deceleration and stop(per64[]-px64[])						*/
LONG	pex,x1,x2;						/* For calculation of pulse amount during deceleration and stop									*/
PCMKP	*pcmkprm = &pcmdmkr->P;			/* Position command creator parameters									*/
PCMKV	*pcmkvar = &pcmdmkr->V;			/* Position command generator calculation variable									*/


/*--------------------------------------------------------------------------------------------------*/
/*		Position command creation mode change check																*/
/*--------------------------------------------------------------------------------------------------*/
	pcmkvar->modechgf = ((pcmkmode & 0xFF) != pcmkvar->pcmkmode)? TRUE : FALSE;
	pcmkvar->pcmkmode = (UCHAR)pcmkmode;


	switch( pcmkmode )
	{
/****************************************************************************************************/
/*																									*/
/*		Positioning mode																				*/
/*																									*/
/****************************************************************************************************/
	case PCMKMODE_POS:
/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( pcmkvar->modechgf )
		{
			pcmkvar->vpxrem = 0;
			pcmkvar->cmderrf = FALSE;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Position command speed(Maximum)[xpulse/scan] : maxvp = osvpm * pcmdspd / 2^24							*/
/*--------------------------------------------------------------------------------------------------*/
		pcmkvar->maxvp = XasmMulhigh32down( (pcmkprm->osvpm << 4), (LPX_ABS(pcmdspd) << 4) );

/*--------------------------------------------------------------------------------------------------*/
/*		64bitPosition command: Calculation of position command deviation															*/
/*--------------------------------------------------------------------------------------------------*/
		if( pcmkprm->pcmd64f )
		{
			per64[0] = pcmdin0 - pcmkvar->pcmdout[0];
			carry = ((ULONG)pcmdin0 < (ULONG)pcmkvar->pcmdout[0]);
			per64[1] = pcmdin1 - pcmkvar->pcmdout[1] - carry;
		/*------------------------------------------------------------------------------------------*/
			if( (per64[1]>>(30-pcmkprm->pshlx)) > 0 )
			{
				per64[1] =  0x3FFFFFFF;
			}
			else if( (per64[1]>>(30-pcmkprm->pshlx)) < -1 )
			{
				per64[1] = -0x3FFFFFFF;
			}
			else
			{
				x1       = (((ULONG)per64[0]>>(31-pcmkprm->pshlx))>>1);
				carry    = ((ULONG)(per64[0]<<pcmkprm->pshlx) < (ULONG)pcmkvar->vpxrem);
				per64[0] = (per64[0]<<pcmkprm->pshlx) - pcmkvar->vpxrem;
				per64[1] = (per64[1]<<pcmkprm->pshlx) + x1 - (pcmkvar->vpxrem>>31) - carry;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		32bitPosition command: Calculation of position command deviation															*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x1       = pcmdin0 - pcmkvar->pcmdout[0];
			carry    = ((ULONG)(x1<<pcmkprm->pshlx) < (ULONG)pcmkvar->vpxrem);
			per64[0] = (x1<<pcmkprm->pshlx) - pcmkvar->vpxrem;
			per64[1] = ((x1>>(31-pcmkprm->pshlx))>>1) - (pcmkvar->vpxrem>>31) - carry;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Convergence condition check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( ( per64[1] >  0 ) || 
			( per64[1] < -1 ) ||
			( (per64[1]^per64[0]) < 0 ) )
		{
			;
		}
		else if( (-pcmkprm->vpdec <= per64[0]) && (per64[0] <= pcmkprm->vpdec) )
		{
			x1 = per64[0] - pcmkvar->vpx;
			if( (x1 >= -pcmkprm->vpdec) && (x1 <= pcmkprm->vpdec) )
			{
				pcmkvar->vpx = per64[0];
				pcmkvar->calendf = (per64[0]==0)? TRUE : FALSE;
				break;
			}
		}
		
/*--------------------------------------------------------------------------------------------------*/
/*		Speed check																				*/
/*--------------------------------------------------------------------------------------------------*/
		pcmkvar->calendf = FALSE;
		pcmkvar->avp = ( pcmkvar->vpx > 0 ) ? pcmkvar->vpx : -pcmkvar->vpx;
		if( pcmkvar->avp >= (pcmkvar->maxvp + pcmkprm->vpdec) )
		{
			if( pcmkvar->vpx > 0 )
			{
				pcmkvar->vpx = pcmkvar->vpx - pcmkprm->vpdec;
			}
			else
			{
				pcmkvar->vpx = pcmkvar->vpx + pcmkprm->vpdec;
			}
			break;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Deceleration stop(Zero speed)Calculation of position command pulse amount up to												*/
/*--------------------------------------------------------------------------------------------------*/
		pcmkvar->n   = pcmkvar->avp / pcmkprm->vpdec;
		pcmkvar->rem = pcmkvar->avp - pcmkvar->n * pcmkprm->vpdec;
		if( ((pcmkvar->n+1)&0x01) == 0x00 )
		{
			MUL3232( ((pcmkvar->n + 1)>>1), (pcmkvar->avp + pcmkvar->rem), &px64[0] );
		}
		else
		{
			MUL3232( (pcmkvar->n + 1), ((pcmkvar->avp + pcmkvar->rem)>>1), &px64[0] );
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Area check & calculation of position command deviation during deceleration and stop												*/
/*--------------------------------------------------------------------------------------------------*/
		if( (pcmkvar->vpx >= 0) && (per64[1] >= 0) && ((per64[1]|per64[0]) != 0) )
		{
			pex64[0] = per64[0] - px64[0];
			pex64[1] = per64[1] - px64[1] - ((ULONG)per64[0] < (ULONG)px64[0]);
			x1 = pex64[0]^pex64[1];
			if( ( x1 >= 0 )&&(( pex64[1]==0 )||( pex64[1]==-1 )) )
			{
				pex = pex64[0];
			}
			else if( pex64[1] >= 0 )
			{
				pcmkvar->vpx = pcmkvar->vpx + pcmkprm->vpacc;
				if( pcmkvar->vpx > pcmkvar->maxvp ){ pcmkvar->vpx = pcmkvar->maxvp;}
				break;
			}
			else
			{
				pcmkvar->vpx = pcmkvar->vpx - pcmkprm->vpdec;
				if( pcmkvar->vpx < -pcmkvar->maxvp ){ pcmkvar->vpx = -pcmkvar->maxvp;}
				break;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( (pcmkvar->vpx <= 0) && (per64[1] < 0) )
		{
			pex64[0] = per64[0] + px64[0];
			pex64[1] = per64[1] + px64[1] + ((ULONG)pex64[0] < (ULONG)per64[0]);
			x1 = pex64[0]^pex64[1];
			if( ( x1 >= 0 )&&(( pex64[1]==0 )||( pex64[1]==-1 )) )
			{
				pex = (pex64[0]==0x80000000) ? 0x7FFFFFFF : -pex64[0];
			}
			else if( pex64[1] >= 0 )
			{
				pcmkvar->vpx = pcmkvar->vpx + pcmkprm->vpdec;
				if( pcmkvar->vpx > pcmkvar->maxvp ){ pcmkvar->vpx = pcmkvar->maxvp;}
				break;
			}
			else
			{
				pcmkvar->vpx = pcmkvar->vpx - pcmkprm->vpacc;
				if( pcmkvar->vpx < -pcmkvar->maxvp ){ pcmkvar->vpx = -pcmkvar->maxvp;}
				break;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		else if( (pcmkvar->vpx < 0) && (per64[1] >= 0) )
		{
			pcmkvar->vpx = pcmkvar->vpx + pcmkprm->vpdec;
			if( pcmkvar->vpx > pcmkvar->maxvp ){ pcmkvar->vpx = pcmkvar->maxvp;}
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			pcmkvar->vpx = pcmkvar->vpx - pcmkprm->vpdec;
			if( pcmkvar->vpx < -pcmkvar->maxvp ){ pcmkvar->vpx = -pcmkvar->maxvp;}
			break;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation during acceleration																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( pex > pcmkvar->n )
		{
			x1 = pcmkprm->vparx + pcmkvar->rem - pcmkprm->vpdec;
			x2 = pcmkprm->vpapx + pcmkprm->vpamx*pcmkvar->avp + pcmkvar->n*pcmkprm->vparx + ((x1>0)?x1:0);
		/*------------------------------------------------------------------------------------------*/
		/*	pex >= (vpapx+vpamx*avp+n*vparx+xxx)													*/
		/*------------------------------------------------------------------------------------------*/
			if( pex >= x2 )
			{
				pcmkvar->avp = pcmkvar->avp + pcmkprm->vpacc;
			}
		/*------------------------------------------------------------------------------------------*/
		/*	pex >= (avp+vpdec)																		*/
		/*------------------------------------------------------------------------------------------*/
			else if( pex >= (pcmkvar->avp + pcmkprm->vpdec) )
			{
				if( pex >= (8*pcmkvar->avp + 36*pcmkprm->vpdec) )
				{
					pex = pex - (8*pcmkvar->avp + 36*pcmkprm->vpdec);
					pcmkvar->avp = pcmkvar->avp + 8*pcmkprm->vpdec;
				}
				if( pex >= (4*pcmkvar->avp + 10*pcmkprm->vpdec) )
				{
					pex = pex - (4*pcmkvar->avp + 10*pcmkprm->vpdec);
					pcmkvar->avp = pcmkvar->avp + 4*pcmkprm->vpdec;
				}
				if( pex >= (2*pcmkvar->avp + 3*pcmkprm->vpdec) )
				{
					pex = pex - (2*pcmkvar->avp + 3*pcmkprm->vpdec);
					pcmkvar->avp = pcmkvar->avp + 2*pcmkprm->vpdec;
				}
				if( pex >= (pcmkvar->avp + pcmkprm->vpdec) )
				{
					pcmkvar->avp = pcmkvar->avp + pcmkprm->vpdec;
				}
			}
		/*------------------------------------------------------------------------------------------*/
		/*	pex > (n+1)*(vpdec-rem)																	*/
		/*------------------------------------------------------------------------------------------*/
			else if( pex > ((pcmkvar->n + 1)*(pcmkprm->vpdec - pcmkvar->rem)) )
			{
				pcmkvar->avp = pcmkvar->avp + pcmkprm->vpdec;
				pcmkvar->avp = pcmkvar->avp - (pcmkvar->avp - pex + pcmkvar->n + 1)/(pcmkvar->n + 2);
			}
		/*------------------------------------------------------------------------------------------*/
		/*	n < pex <= (n+1)*(vpdec-rem)															*/
		/*------------------------------------------------------------------------------------------*/
			else
			{
				pcmkvar->avp = pcmkvar->avp + pex/(pcmkvar->n + 1);
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation during deceleration																				*/
/*--------------------------------------------------------------------------------------------------*/
		else if( pex < 0 )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	pex <= -avp																				*/
		/*------------------------------------------------------------------------------------------*/
			if( pex <= -pcmkvar->avp )
			{
				pcmkvar->avp = pcmkvar->avp - pcmkprm->vpdec;
			}
		/*------------------------------------------------------------------------------------------*/
		/*	-avp < pex <= -(n+1)*rem																*/
		/*------------------------------------------------------------------------------------------*/
			else if( pex <= -(pcmkvar->n + 1)*pcmkvar->rem )
			{
				pcmkvar->avp = pcmkvar->avp - pcmkprm->vpdec + (pex + pcmkvar->avp)/pcmkvar->n;
			}
		/*------------------------------------------------------------------------------------------*/
		/*	-(n+1)*rem < pex < 0																	*/
		/*------------------------------------------------------------------------------------------*/
			else
			{
				pcmkvar->avp = pcmkvar->avp - (pcmkvar->n - pex)/(pcmkvar->n + 1);
			}
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Limit & sign calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
		x1 = ( pcmkvar->avp > pcmkvar->maxvp )? pcmkvar->maxvp : pcmkvar->avp;
		pcmkvar->vpx = ( per64[1] >= 0 )? x1 : -x1;

		break;

/****************************************************************************************************/
/*		Deceleration and stop mode																				*/
/****************************************************************************************************/
	default: 
/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		pcmkvar->cmderrf = FALSE;
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of position command speed																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( pcmkvar->vpx >= 0 )
		{
			pcmkvar->vpx = pcmkvar->vpx - pcmkprm->vpdec;
			if( pcmkvar->vpx <= 0 )
			{
				pcmkvar->vpx = 0;
				pcmkvar->calendf = TRUE;
			}
			else
			{
				pcmkvar->calendf = FALSE;
			}
		}
		else
		{
			pcmkvar->vpx = pcmkvar->vpx + pcmkprm->vpdec;
			if( pcmkvar->vpx >= 0 )
			{
				pcmkvar->vpx = 0;
				pcmkvar->calendf = TRUE;
			}
			else
			{
				pcmkvar->calendf = FALSE;
			}
		}
		break;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation end processing : Position command output(Absolute value)update													*/
/*--------------------------------------------------------------------------------------------------*/
	x1 = pcmkvar->vpx + pcmkvar->vpxrem;
	x2 = (x1>=0)? (x1>>pcmkprm->pshlx) : -((-x1)>>pcmkprm->pshlx);
	pcmkvar->vpxrem = x1 - (x2<<pcmkprm->pshlx);
	pcmkvar->pcmdout[0] = pcmkvar->pcmdout[0] + x2;
	carry = ((ULONG)pcmkvar->pcmdout[0] < (ULONG)x2);
	pcmkvar->pcmdout[1] = pcmkvar->pcmdout[1] + (x2>>31) + carry;
/*--------------------------------------------------------------------------------------------------*/
	return( x2 );
}


#if 0 /* mlib.sh */

/****************************************************************************************************/
/*																									*/
/*																									*/
/*		<<<<< Basic numerical calculation function >>>>>																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Absolute value(32bit)																				*/
/*																									*/
/****************************************************************************************************/
LONG	MlibAbs32( LONG u )
{
LONG	x;

		x = (u >= 0)? u : -u;

		return( x );

}



/****************************************************************************************************/
/*																									*/
/*		Addition(64bit+32bit)																			*/
/*																									*/
/****************************************************************************************************/
void	MlibAdd6432( LONG *a, LONG b, LONG *x )
{

		ADD6432( a, b, x );

		return;

}
#endif



#if 0 /* mlib.sh */
/****************************************************************************************************/
/*																									*/
/*		Addition(64bit+64bit)																			*/
/*																									*/
/****************************************************************************************************/
void	MlibAdd6464( LONG *a, LONG *b, LONG *x )
{

		ADD6464( a, b, x );

		return;

}
#endif



#if 0 /* mlib.sh */
/****************************************************************************************************/
/*																									*/
/*		Subtraction(64bit-32bit)																			*/
/*																									*/
/****************************************************************************************************/
void	MlibSub6432( LONG *a, LONG b, LONG *x )
{

		SUB6432( a, b, x );

		return;

}
#endif



#if 0 /* mlib.sh */
/****************************************************************************************************/
/*																									*/
/*		Subtraction(64bit-64bit)																			*/
/*																									*/
/****************************************************************************************************/
void	MlibSub6464( LONG *a, LONG *b, LONG *x )
{

		SUB6464( a, b, x );

		return;

}
#endif


#if 0
/****************************************************************************************************/
/*																									*/
/*		Deviation(64bit-64bit)																			*/
/*																									*/
/****************************************************************************************************/
LONG	MlibErr6464( LONG *a, LONG *b )
{
LONG	errchk;
LONG	err64[2];

		MlibSub6464( a, b, err64 );
		if( err64[1] >  0 ){ return(  0x3FFFFFFF );}
		if( err64[1] < -1 ){ return( -0x40000000 );}

		errchk = (err64[1]<<2) + (((ULONG)err64[0])>>30);
		if( errchk   >  0 ){ return(  0x3FFFFFFF );}
		if( errchk   < -1 ){ return( -0x40000000 );}
		return( err64[0] );

}
#endif


/****************************************************************************************************/
/*																									*/
/*		Saturation addition(Signed,32bit+32bit,Limit:0x01000000,0xFF000000)									*/
/*																									*/
/****************************************************************************************************/
LONG	MlibSatAdd24( LONG a, LONG b )
{
LONG	x;

		x = a + b;

		if( (a>0)&&(b>0)&&(x<=0) ){ return( 0x01000000 );}
		if( (a<0)&&(b<0)&&(x>=0) ){ return( 0xFF000000 );}

		if( x > (LONG)0x01000000 ){ return( 0x01000000 );}
		if( x < (LONG)0xFF000000 ){ return( 0xFF000000 );}

		return( x );

}



/****************************************************************************************************/
/*																									*/
/*		Saturation addition(Signed,32bit+32bit,Limit:0x10000000,0xF0000000)									*/
/*																									*/
/****************************************************************************************************/
LONG	MlibSatAdd28( LONG a, LONG b )
{
LONG	x;

		x = a + b;

		if( (a>0)&&(b>0)&&(x<=0) ){ return( 0x10000000 );}
		if( (a<0)&&(b<0)&&(x>=0) ){ return( 0xF0000000 );}

		if( x > (LONG)0x10000000 ){ return( 0x10000000 );}
		if( x < (LONG)0xF0000000 ){ return( 0xF0000000 );}

		return( x );

}



/****************************************************************************************************/
/*																									*/
/*		Saturation addition(Signed,32bit+32bit,Limit:0x7FFFFFFF,0x80000000)									*/
/*																									*/
/****************************************************************************************************/
KSGAIN	MlibSatAdd32( KSGAIN a, KSGAIN b )		/*<S00A>*/
{
KSGAIN	x;		/*<S00A>*/

		x = a + b;

		if( (a>0)&&(b>0)&&(x<=0) ){ return( 0x7FFFFFFF );}
		if( (a<0)&&(b<0)&&(x>=0) ){ return( 0x80000000 );}

		return( x );

}



/****************************************************************************************************/
/*																									*/
/*		Saturation addition(No sign,32bit+32bit,Limit:0xFFFFFFFF)												*/
/*																									*/
/****************************************************************************************************/
ULONG	MlibSatAddu32( ULONG a, ULONG b )
{
ULONG	x;

		x = a + b;

		if( x < a ){ return( 0xFFFFFFFF );}

		return( x );

}



#if 0 /* mlib.sh */
/****************************************************************************************************/
/*																									*/
/*		Multiplication(32bit*32bit)																			*/
/*																									*/
/****************************************************************************************************/
void	MlibMul3232( LONG a, LONG b, LONG *x )
{

		MUL3232( a, b, x );
		return;

}
#endif



#if 0 /* mlib.sh */
/****************************************************************************************************/
/*																									*/
/*		Multiplication(32bit*32bit)																			*/
/*																									*/
/****************************************************************************************************/
void	MlibMulu32u32( ULONG a, ULONG b, ULONG *x )
{

		MULU32U32( a, b, x );
		return;

}



/****************************************************************************************************/
/*																									*/
/*		Multiplication(32bit*32bit)																			*/
/*																									*/
/****************************************************************************************************/
LONG	MlibMulhigh32( LONG a, LONG b )
{
LONG	xx[2];

		MUL3232( a, b, xx );
		xx[1] = xx[1] + ((ULONG)xx[0]>>31);

		return( xx[1] );

}



/****************************************************************************************************/
/*																									*/
/*		Multiplication(32bit*32bit)																			*/
/*																									*/
/****************************************************************************************************/
ULONG	MlibMulhighu32( ULONG a, ULONG b )
{
ULONG	xx[2];

		MULU32U32( a, b, xx );
		xx[1] = xx[1] + ((ULONG)xx[0]>>31);

		return( xx[1] );

}
#endif



/****************************************************************************************************/
/*																									*/
/*		Division(32bit/16bit)			rv = u32/k16, *rem = u32%k16									*/
/*																									*/
/****************************************************************************************************/
LONG	MlibDivhrem( LONG u, SHORT k, SHORT *rem )
{
LONG	x;

		x = u / k;

		*rem = (SHORT)(u - (x*k));

		return( x );

}



/****************************************************************************************************/
/*																									*/
/*		Division(32bit*32bit/32bit)		rv = a*b/c, Limit : 0x7FFFFFFF									*/
/*																									*/
/****************************************************************************************************/
LONG	MlibDiv6432( LONG a, LONG b, LONG c )
{
LONG	i;
LONG	x;
LONG	xx[2];
LONG	signx;

		signx = 1;
		if( a <  0 ){ a = -a; signx = -1*signx;}
		if( b <  0 ){ b = -b; signx = -1*signx;}
		if( c <  0 ){ c = -c; signx = -1*signx;}
		if( c == 0 ){ return( 0 );}
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( (ULONG)a, (ULONG)b, (ULONG*)xx );
		if( (ULONG)xx[1] >= (ULONG)c ){ return( signx * 0x7FFFFFFF );}
		xx[1] = (xx[1]<<1) + (xx[0]<0); xx[0] = (xx[0]<<1);
		if( (ULONG)xx[1] >= (ULONG)c ){ return( signx * 0x7FFFFFFF );}
		xx[1] = (xx[1]<<1) + (xx[0]<0); xx[0] = (xx[0]<<1);
/*--------------------------------------------------------------------------------------------------*/
		for( x=0,i=0; i<31; i++ )
		{
			x = (x<<1);
			if( (ULONG)xx[1] >= (ULONG)c )
			{
				x     = x + 1;
				xx[1] = (ULONG)xx[1] - (ULONG)c;
			}
			xx[1] = (xx[1]<<1) + (xx[0]<0);
			xx[0] = (xx[0]<<1);
		}
		if( (ULONG)xx[1] >= (ULONG)c )
		{
			x = x + 1;
			if( x < 0 )
			{
				x = 0x7FFFFFFF;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( signx * x );

}



/****************************************************************************************************/
/*																									*/
/*		Extended division(32bit/32bit)		rv = ((a/b)<<sx)												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibDivx3232( LONG a, LONG b, LONG qmax, LONG sx )
{
LONG	rv;
LONG	remx;
LONG	signx;

		signx = a ^ b;
		a = ( a < 0 )? -a : a;
		b = ( b < 0 )? -b : b;
		if( b == 0 ){ return( 0 );}

		rv = a/b;
		remx = a%b;

		if( rv < qmax )
		{
			rv = (rv<<sx) | XasmDIVUA( remx, b, sx );
		}
		else
		{
			rv = (qmax<<sx);
		}

		if( signx >= 0 )
		{
			return( rv );
		}
		else
		{
			return( -rv );
		}
}



/****************************************************************************************************/
/*																									*/
/*		Route calculation(Input:32bit, Output:16bit(Max.0x10000), Rounding)									*/
/*																									*/
/****************************************************************************************************/
USHORT	MlibSqrtu32( ULONG a )
{
ULONG	i;
ULONG	x,y;

/*--------------------------------------------------------------------------------------------------*/
/*		Route calculation processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		for( x=y=i=0; i<16; i++ )
		{
			x = ((x<<2)|(a>>30));
			a = (a<<2);
			y = (y<<1);
			if( x > y )
			{
				x = x - y - 1;
				y = y + 2;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Rounding process																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( (x<<1) > y )
		{
			y = y + 2;
		}
		y = y>>1;
		y = (y > 0xFFFF)? 0xFFFF : y;

		return( (USHORT)y );

}



/****************************************************************************************************/
/*																									*/
/*		Route calculation(Input:64bit, Output:32bit(Max.0xFFFFFFFF), Rounding)								*/
/*																									*/
/****************************************************************************************************/
ULONG	MlibSqrtu64( ULONG a0, ULONG a1 )
{
ULONG	i;
ULONG	x,y,z;
ULONG	xx,yy;

/*--------------------------------------------------------------------------------------------------*/
/*		Route calculation processing(Upper level32bit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		for( x=y=i=0; i<16; i++ )
		{
			x  = ((x<<2)|(a1>>30));
			a1 = (a1<<2);
			y  = (y<<1);
			if( x > y )
			{
				x = x - y - 1;
				y = y + 2;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Route calculation processing(Lower28bit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i<14; i++ )
		{
			x  = ((x<<2)|(a0>>30));
			a0 = (a0<<2);
			y  = (y<<1);
			if( x > y )
			{
				x = x - y - 1;
				y = y + 2;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Route calculation processing(Lower4bit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		xx = (x>>30);
		x  = ((x<<2)|(a0>>30));
		a0 = (a0<<2);
		yy = (y>>31);
		y  = (y<<1);
		if( xx > yy )
		{
			xx = xx - yy - (x<=y);
			x  = x - y - 1;
			y  = y + 2;
		}
		else if( (xx==yy)&&(x>y) )
		{
			x  = x - y - 1;
			y  = y + 2;
		}
/*--------------------------------------------------------------------------------------------------*/
		xx = (xx<<2)|(x>>30);
		x  = ((x<<2)|(a0>>30));
		yy = (yy<<1)|(y>>31);
		y  = (y<<1);
		if( xx > yy )
		{
			xx = xx - yy - (x<=y);
			x  = x - y - 1;
			y  = y + 2;
		}
		else if( (xx==yy)&&(x>y) )
		{
			xx = xx - yy;
			x  = x - y - 1;
			y  = y + 2;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Rounding process																				*/
/*--------------------------------------------------------------------------------------------------*/
		z  = (yy<<31)+(y>>1);
		xx = (xx<<1)|(x>>31); x = (x<<1);
		if( (z!=0xFFFFFFFF)&&((xx>yy)||((xx==yy)&&(x>y))) )
		{
			z = z + 1;
		}

		return( z );

}



/****************************************************************************************************/
/*																									*/
/*		Route calculation(Input:32bit*32bit, Output:32bit(Max.0xFFFFFFFF), Rounding)							*/
/*																									*/
/****************************************************************************************************/
ULONG	MlibSqrtu32u32( ULONG a, ULONG b )
{
ULONG	i;
ULONG	ab[2];
ULONG	x,y,z;
ULONG	xx,yy;

/*--------------------------------------------------------------------------------------------------*/
/*		Multiplication processing(32bit*32bit)																		*/
/*--------------------------------------------------------------------------------------------------*/
		MULU32U32( a, b, &ab[0] );

/*--------------------------------------------------------------------------------------------------*/
/*		Route calculation processing(Upper level32bit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		for( x=y=i=0; i<16; i++ )
		{
			x = ((x<<2)|(ab[1]>>30));
			ab[1] = (ab[1]<<2);
			y = (y<<1);
			if( x > y )
			{
				x = x - y - 1;
				y = y + 2;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Route calculation processing(Lower28bit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i<14; i++ )
		{
			x  = ((x<<2)|(ab[0]>>30));
			ab[0] = (ab[0]<<2);
			y  = (y<<1);
			if( x > y )
			{
				x = x - y - 1;
				y = y + 2;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Route calculation processing(Lower4bit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		xx = (x>>30);
		x  = ((x<<2)|(ab[0]>>30));
		ab[0] = (ab[0]<<2);
		y  = (y<<1);
		if( xx > 0 )
		{
			xx = xx - (x<=y);
			x  = x - y - 1;
			y  = y + 2;
		}
		else if( x > y )
		{
			x  = x - y - 1;
			y  = y + 2;
		}
/*--------------------------------------------------------------------------------------------------*/
		xx = (xx<<2)|(x>>30);
		x  = ((x<<2)|(ab[0]>>30));
		yy = (y>>31);
		y  = (y<<1);
		if( xx > yy )
		{
			xx = xx - yy - (x<=y);
			x  = x - y - 1;
			y  = y + 2;
		}
		else if( (xx==yy)&&(x>y) )
		{
			xx = xx - yy;
			x  = x - y - 1;
			y  = y + 2;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Rounding process																				*/
/*--------------------------------------------------------------------------------------------------*/
		z  = (yy<<31)+(y>>1);
		xx = (xx<<1)|(x>>31); x = (x<<1);
		if( (z!=0xFFFFFFFF)&&((xx>yy)||((xx==yy)&&(x>y))) )
		{
			z = z + 1;
		}

		return( z );

}



/****************************************************************************************************/
/*																									*/
/*		Sin calculation  (Input:[0.1deg]or[360/16384deg], Output:[1.0/10000]or[1.0/16384])					*/
/*																									*/
/****************************************************************************************************/
#if 0
SHORT	MlibSins16( LONG a, LONG unit )
{
ULONG	x,y;
ULONG	x2,xx[2];
LONG	calx;
LONG	sign = 1;
LONG	ax90,ax180,ax270,ax360,kax;

/*--------------------------------------------------------------------------------------------------*/
/*		Input unit check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( (unit & 0x01) == 0 )	/* Input : 360deg/3600		*/
		{
			ax90 =  900; ax180 = 1800; ax270 =  2700; ax360 =  3600; kax = 7496132;
		}
		else						/* Input : 360deg/16384		*/
		{
			ax90 = 4096; ax180 = 8192; ax270 = 12288; ax360 = 16384; kax = 1647099;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Input scaling processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( a >= ax360 ){ a -= ax360;}
		else if( a < 0 ){ a += ax360; if( a < 0 ){ a += ax360;} }
/*--------------------------------------------------------------------------------------------------*/
		if( a >= ax270      ){ a = ax360 - a; sign = -1;}
		else if( a >= ax180 ){ a = a - ax180; sign = -1;}
		else if( a >= ax90  ){ a = ax180 - a; sign =  1;}
/*--------------------------------------------------------------------------------------------------*/
		if(  (2*a) <= ax90  ){ x = (ULONG)a * kax; calx = 0;}
		else          { x = (ULONG)(ax90-a) * kax; calx = 1;}

/*--------------------------------------------------------------------------------------------------*/
/*		Sin calculation processing : sin(x) = x - x^3/3! + x^5/5! - x^7/7! + x^9/9!								*/
/*--------------------------------------------------------------------------------------------------*/
		if( calx == 0 )
		{
			y = x;
			MULU32U32(x,x, xx);	x2= xx[1];
			MULU32U32(x,x2,xx); x = xx[1]; MULU32U32(x,715827883,xx); y -= xx[1]; /* - x^3/3!	*/
			MULU32U32(x,x2,xx); x = xx[1]; MULU32U32(x, 35791394,xx); y += xx[1]; /* + x^5/5!	*/
			MULU32U32(x,x2,xx); x = xx[1]; MULU32U32(x,   852176,xx); y -= xx[1]; /* - x^7/7!	*/
			MULU32U32(x,x2,xx); x = xx[1]; MULU32U32(x,    11836,xx); y += xx[1]; /* + x^9/9!	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Sin calculation processing : sin(x) = cos(pai/2-x) = 1 - z^2/2! + z^4/4! - z^6/6! + z^8/8!				*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			MULU32U32(x,x, xx);	x = x2 = xx[1]; y = (x2>2)? (~(x2>>1)+1) : 0xFFFFFFFF;
			MULU32U32(x,x2,xx); x = xx[1]; MULU32U32(x,178956971,xx); y += xx[1]; /* + x^4/4!	*/
			MULU32U32(x,x2,xx); x = xx[1]; MULU32U32(x,  5965232,xx); y -= xx[1]; /* - x^6/6!	*/
			MULU32U32(x,x2,xx); x = xx[1]; MULU32U32(x,   106522,xx); y += xx[1]; /* + x^8/8!	*/
//			MULU32U32(x,x2,xx); x = xx[1]; MULU32U32(x,     1184,xx); y -= xx[1]; /* - x^10/10! */
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Output scaling processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( (unit & 0x10) == 0 )	/* Output : 1.0000/10000	*/
		{
			MULU32U32( y, 20000, xx );
			y = ((xx[1]+1)>>1);
		}
		else						/* Output : 1.0000/16384	*/
		{
			y = (((y>>17)+1)>>1);
		}

		return( (SHORT)((LONG)y * sign) );

}
#endif
LONG	MlibSins16( LONG a, LONG unit )
{
	double	rv;
	double	rad;

	if( (unit & 0x01) == 0 )	/* Input : 360deg/3600		*/
	{
		rad = DEG2RAD * (double)a;
	}
	else						/* Input : 360deg/16384		*/
	{
		rad = FOURTEENBIT2RAD * (double)a;
	}

	rv = sin( rad );

	if( (unit & 0x10) == 0 )	/* Output : 1.0000/10000	*/
	{
		rv = rv * 10000.0;
	}
	else						/* Output : 1.0000/16384	*/
	{
		rv = rv * 16384.0;
	}
	return ( (LONG)rv );
}

#if (FLOAT_USE==TRUE)
/****************************************************************************************************/
/*																									*/
/*		Calculating the index value : 10^exp		Floating point arithmetic														*/
/*																									*/
/****************************************************************************************************/
void	FlibExpCal( CHAR exp, float *output )
{

	LONG	i, x;
	LONG	aexp;

	if( exp < 0 )
	{
		aexp = -exp;
	}
	else
	{
		aexp = exp;
	}
/*--------------------------------------------------------------------------------------------------*/
	for( x=1,i=0; i < aexp; i++ )						/* 10^aexp Calculation								*/
	{
		x = 10 * x;										/* x = 1 * 10 * 10 * ......					*/
	}
/*--------------------------------------------------------------------------------------------------*/
	if( exp >= 0 )										/* {kx,sx}Convert to form						*/
	{
		*output = (float)x;								/* ksx = 10^aexp							*/
	}
	else
	{
		*output = 1.0F / (float)x;						/* ksx = 1 / (10^aexp)						*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}
#endif /* FLOAT_USE */

/****************************************************************************************************/
/*																									*/
/*		Calculating the index value : 10^exp																		*/
/*																									*/
/****************************************************************************************************/
LONG	MlibExpCal( CHAR exp )
{
LONG	i,x;
LONG	ksx;
LONG	aexp;

		aexp = ( exp >= 0 ) ? exp : -exp;					/* Sign check							*/
/*--------------------------------------------------------------------------------------------------*/
		for( x=1,i=0; i < aexp; i++ )						/* 10^aexp Calculation							*/
		{
			x = 10 * x;										/* x = 1 * 10 * 10 * ......				*/
		}
/*--------------------------------------------------------------------------------------------------*/
		if( exp >= 0 )										/* {kx,sx}Convert to form					*/
		{
			ksx = MlibScalKxgain( x, 1, 1, NULL, -1 );		/* ksx = 10^aexp						*/
		}
		else
		{
			ksx = MlibScalKxgain( 1, 1, x, NULL, -1 );		/* ksx = 1 / (10^aexp)					*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return( ksx );

}


/****************************************************************************************************/
/*																									*/
/*		Absolute value deviation check:rv = (|a32 - b32| <= chk32)												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibAbsErrchk32( LONG a32, LONG b32, LONG chk32 )
{
LONG	err;

/*--------------------------------------------------------------------------------------------------*/
/*		Deviation calculation & absolute value calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
		err = a32 - b32;
		if( err < 0 )
		{
			err = -err;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Absolute value deviation check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( (ULONG)err <= (ULONG)chk32 )
		{
			return( TRUE );
		}
		else
		{
			return( FALSE );
		}

}



/****************************************************************************************************/
/*																									*/
/*		Absolute value deviation check:rv = (|a64 - b64| <= chk32)												*/
/*																									*/
/****************************************************************************************************/
LONG	MlibAbsErrchk64( LONG *a64, LONG *b64, LONG chk32 )
{
LONG	err[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Deviation calculation & absolute value calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
		err[0] = a64[0] - b64[0];
		err[1] = a64[1] - b64[1] - ((ULONG)a64[0] < (ULONG)b64[0]);
		if( err[1] < 0 )
		{
			err[1] = ~err[1] + (err[0] == 0);
			err[0] = ~err[0] + 1;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Absolute value deviation check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( err[1] > 0 )
		{
			return( FALSE );
		}
		else if( (ULONG)err[0] <= (ULONG)chk32 )
		{
			return( TRUE );
		}
		else
		{
			return( FALSE );
		}

}



/****************************************************************************************************/
/*																									*/
/*		Setting CRC16(MEMOBUSFor), Return value:No capacity													*/
/*																									*/
/****************************************************************************************************/
void	MlibSetCRC16MB( UCHAR *MsgBuf, LONG MsgLen )
{
LONG	i;
ULONG	a = 0xFFFF;
ULONG	b = 0xA001;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate CRC16																				*/
/*		※LengthtoSUMChanged to specify the message length including the value.(Jupiter)					*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i<(MsgLen-2); i++ )
		{
			a = a ^ (ULONG)MsgBuf[i];
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 1	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 2	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 3	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 4	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 5	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 6	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 7	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 8	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set CRC16 Data (CRC16 Cal. Data : Low/High reversed)										*/
/*--------------------------------------------------------------------------------------------------*/
		MsgBuf[MsgLen-2] = (UCHAR)(a>>0);
		MsgBuf[MsgLen-1] = (UCHAR)(a>>8);
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Checking CRC16(MEMOBUSFor), Return value:TRUE(Normal)/FALSE(Abnormal)								*/
/*																									*/
/****************************************************************************************************/
LONG	MlibChkCRC16MB( UCHAR *MsgBuf, LONG MsgLen )
{
LONG	i;
ULONG	a = 0xFFFF;
ULONG	b = 0xA001;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate CRC16																				*/
/*		CRCChanged to specify message length including(Jupiter)								*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i<(MsgLen-2); i++ )
		{
			a = a ^ (ULONG)MsgBuf[i];
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 1	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 2	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 3	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 4	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 5	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 6	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 7	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 8	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Check CRC16 Data (CRC16 Cal. Data : Low/High reversed)										*/
/*--------------------------------------------------------------------------------------------------*/
		if( a == (ULONG)(MsgBuf[MsgLen-2]+(MsgBuf[MsgLen-1]<<8)) ){ return(OK);}else{ return(NG);}
}


/****************************************************************************************************/
/*																									*/
/*		SUM16 setting(SigmaMessageFor), Return value:No capacity												*/
/*																									*/
/****************************************************************************************************/
void	MlibSetSum16SM( UCHAR *MsgBuf, LONG MsgLen )
{
LONG	i;
LONG	Sum;
LONG	WdLen = (MsgLen>>1) - 1;
USHORT*	WdBuf = (USHORT*)MsgBuf;
USHORT  word;
/*--------------------------------------------------------------------------------------------------*/
/*		Calculate Sum16																				*/
/*--------------------------------------------------------------------------------------------------*/
		for( Sum=0,i=0; i<WdLen; i++ )
		{
			word = WdBuf[i];
			MlibCnvLittle16( word, word );
			Sum = Sum + word;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set SUM16 Data																				*/
/*--------------------------------------------------------------------------------------------------*/
		Sum = (USHORT)(-Sum);
		MlibCnvLittle16( Sum, Sum );
		WdBuf[WdLen] = Sum;
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		SUM16 check(SigmaMessageFor), Return value:TRUE(Normal)/FALSE(Abnormal)							*/
/*																									*/
/****************************************************************************************************/
LONG	MlibChkSum16SM( UCHAR *MsgBuf, LONG MsgLen )
{
LONG	i;
LONG	Sum;
LONG	WdLen = (MsgLen >> 1);
USHORT*	WdBuf = (USHORT*)MsgBuf;
USHORT  word;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate Sum16																				*/
/*--------------------------------------------------------------------------------------------------*/
		for( Sum=0,i=0; i<WdLen; i++ )
		{
			word = WdBuf[i];
			MlibCnvLittle16( word, word );
			Sum = Sum + word;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Check SUM16 Data																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( (USHORT)Sum == 0x0000 ){ return( OK );}else{ return( NG );}
}



/****************************************************************************************************/
/*																									*/
/*																									*/
/*		<<<<< Memory operation function >>>>>																	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Reset byte memory																		*/
/*																									*/
/****************************************************************************************************/
void	MlibResetByteMemory( void *mempx, LONG xbyte )
{
		LONG	i;

		for( i=0; i<xbyte; i++ )
		{
			((UCHAR *)mempx)[i] = 0;
		}
		return;

}



/****************************************************************************************************/
/*																									*/
/*		Long memory reset																		*/
/*																									*/
/****************************************************************************************************/
void	MlibResetLongMemory( void *mempx, LONG xlwd )
{
LONG	i;

		for( i=0; i<xlwd; i++ )
		{
			((LONG *)mempx)[i] = 0;
		}
		return;

}



/****************************************************************************************************/
/*																									*/
/*		KSGAIN → LONG Data Convert																	*/
/*																									*/
/****************************************************************************************************/
LONG	MlibKsgain2Long( KSGAIN Input )
{
	LONG	kx;
	LONG	sx;

	//kx = (LONG)((Input << 8) >> 8);
	//sx = (LONG)(Input >> 24);
	kx = (((LONG)Input << 8) >> 8);
	sx = ((LONG)Input >> 24);

	return	kx >> sx;
}



/****************************************************************************************************/
/*																									*/
/*																									*/
/*		<<<<< High-speed version: Fixed-point gain(16bit.16bit) >>>>>											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(High-speed version: Fixed-point gain)				rv = (k*u)>>16;							*/
/*																									*/
/****************************************************************************************************/
LONG	MlibHsMulgain( LONG u, LONG kx )
{
LONG	lwk;
LONG	xx[2];

/*--------------------------------------------------------------------------------------------------*/
/*		xx = u * kx (32bit*32bit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );								/* xx[] = {xh,xl} = u * kx				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Upper/lower limit check																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( xx[1] >= 0x0100 )								/* Check Upper Limit					*/
		{
			return(  0x01000000 );
		}
		else if( xx[1] < -0x0100 )							/* Check Lower Limit					*/
		{
			return( -0x01000000 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Computation result editing																				*/
/*--------------------------------------------------------------------------------------------------*/
		lwk = (LONG)(xx[1]<<16) + ((((ULONG)xx[0]>>15) + 1)>>1);
		return( lwk );

}



/****************************************************************************************************/
/*																									*/
/*		Gain multiplication(High-speed version: Fixed-point gain, LimitNo capacity)		rv = (k*u)>>16;							*/
/*																									*/
/****************************************************************************************************/
LONG	MlibHsMulgainNolim( LONG u, LONG kx )
{
LONG	lwk;
LONG	xx[2];

/*--------------------------------------------------------------------------------------------------*/
/*		xx = u * kx (32bit*32bit)																	*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );								/* xx[] = {xh,xl} = u * kx				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Computation result editing																				*/
/*--------------------------------------------------------------------------------------------------*/
		lwk = (LONG)(xx[1]<<16) + ((((ULONG)xx[0]>>15) + 1)>>1);
		return( lwk );

}



/****************************************************************************************************/
/*																									*/
/*		Integral calculation(High-speed version: Fixed-point gain)					iu=iu+(k*u); rv=((iu[1]+1)>>1);			*/
/*																									*/
/****************************************************************************************************/
LONG	MlibHsIntegral( LONG u, LONG kx, LONG iu[2] )
{
LONG	xx[2];
LONG	carry;

/*--------------------------------------------------------------------------------------------------*/
/*		Input calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		MUL3232( u, kx, xx );
/*--------------------------------------------------------------------------------------------------*/
/*		Integral calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( xx[1] > 0x0100 )
		{
			iu[1] = iu[1] + (LONG)0x02000000;
		}
		else if( xx[1] < -0x0100 )
		{
			iu[1] = iu[1] - (LONG)0x02000000;
		}
		else
		{
			carry = ( ((ULONG)(iu[0]+(xx[0]<<17))) < (ULONG)iu[0] );
			iu[0] = iu[0] + (xx[0]<<17);
			iu[1] = iu[1] + (xx[1]<<17) + ((ULONG)xx[0]>>15) + carry;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Integral value limit																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( iu[1] >= (LONG)0x02000000 )
		{
			iu[0] = 0;
			iu[1] = 0x02000000;
		}
		else if( iu[1] < (LONG)-0x02000000 )
		{
			iu[0] =  0;
			iu[1] = -0x02000000;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( ((iu[1]+1)>>1) );

}

/****************************************************************************************************/
/*																									*/
/*		Search bit ON(LSB/MSB, Return value:BitNo/NG(-1))									*/
/*																									*/
/****************************************************************************************************/
LONG	MlibSrhbiton( ULONG data, ULONG dir )
{
LONG	BitNo = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		None ON Bit : data == 0																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( data == 0 )
		{
			return( -1 );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Search from LSB																				*/
/*--------------------------------------------------------------------------------------------------*/
		else if( dir == SRH_FROM_LSB )
		{
			if( (data & 0xFFFF) == 0 ){ BitNo += 16; data = data>>16;}
			if( (data & 0x00FF) == 0 ){ BitNo +=  8; data = data>>8 ;}
			if( (data & 0x000F) == 0 ){ BitNo +=  4; data = data>>4 ;}
			if( (data & 0x0003) == 0 ){ BitNo +=  2; data = data>>2 ;}
			if( (data & 0x0001) == 0 ){ BitNo +=  1;}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Search from MSB																				*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{/*kira_Error correction_for_DataTrace_101108*/
			if( (data & 0xFFFF0000) != 0 ){ BitNo += 16; data = data>>16;}
			if( (data & 0x0000FF00) != 0 ){ BitNo +=  8; data = data>>8 ;}
			if( (data & 0x000000F0) != 0 ){ BitNo +=  4; data = data>>4 ;}
			if( (data & 0x0000000C) != 0 ){ BitNo +=  2; data = data>>2 ;}
			if( (data & 0x00000002) != 0 ){ BitNo +=  1;}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( BitNo );

}


/****************************************************************************************************/
/*																									*/
/*		Power of 2 check (Return value : BitNo/NG(-1))														*/
/*																									*/
/****************************************************************************************************/
LONG	MlibCheckPowerOf2( ULONG x )
{
LONG	BitNo;

		BitNo = MlibSrhbiton( x, SRH_FROM_LSB );

		if( (BitNo >= 0) && (x == (ULONG)(1<<BitNo)) )
		{
			return( BitNo );
		}
		else
		{
			return( -1 );
		}

}



/****************************************************************************************************/
/*																									*/
/*		Copy byte memory			//kira_for_MEMOBUS_Message										*/
/*																									*/
/****************************************************************************************************/
void	MlibCopyByteMemory( void *dstMem, const void *srcMem, INT nbyte )
{
	CHAR *dst, *src;

	dst = (CHAR*)dstMem;
	src = (CHAR*)srcMem;
	while( nbyte > 0 )
	{
		*dst = *src;
		dst++;
		src++;
		nbyte--;
	}
}



/****************************************************************************************************/
/*																									*/
/*		Long memory copy																			*/
/*																									*/
/****************************************************************************************************/
void	MlibCopyLongMemory( void *dstMem, void *srcMem, INT nbyte )
{
	ULONG *dst, *src;

	dst = (ULONG*)dstMem;
	src = (ULONG*)srcMem;
	while( nbyte > 0 )
	{
		*dst = *src;
		dst++;
		src++;
		nbyte--;
	}
}



/****************************************************************************************************/
/*																									*/
/*		High-speed sine operation table: input:[360deg/1024], Output:[1.0/16384]								*/
/*																									*/
/****************************************************************************************************/
const	SHORT	MlibFastSinTbl[1024] = {
	     0,   101,   201,   302,   402,   503,   603,   704,
	   804,   904,  1005,  1105,  1205,  1306,  1406,  1506,
	  1606,  1706,  1806,  1906,  2006,  2105,  2205,  2305,
	  2404,  2503,  2603,  2702,  2801,  2900,  2999,  3098,
	  3196,  3295,  3393,  3492,  3590,  3688,  3786,  3883,
	  3981,  4078,  4176,  4273,  4370,  4467,  4563,  4660,
	  4756,  4852,  4948,  5044,  5139,  5235,  5330,  5425,
	  5520,  5614,  5708,  5803,  5897,  5990,  6084,  6177,
	  6270,  6363,  6455,  6547,  6639,  6731,  6823,  6914,
	  7005,  7096,  7186,  7276,  7366,  7456,  7545,  7635,
	  7723,  7812,  7900,  7988,  8076,  8163,  8250,  8337,
	  8423,  8509,  8595,  8680,  8765,  8850,  8935,  9019,
	  9102,  9186,  9269,  9352,  9434,  9516,  9598,  9679,
	  9760,  9841,  9921, 10001, 10080, 10159, 10238, 10316,
	 10394, 10471, 10549, 10625, 10702, 10778, 10853, 10928,
	 11003, 11077, 11151, 11224, 11297, 11370, 11442, 11514,
	 11585, 11656, 11727, 11797, 11866, 11935, 12004, 12072,
	 12140, 12207, 12274, 12340, 12406, 12472, 12537, 12601,
	 12665, 12729, 12792, 12854, 12916, 12978, 13039, 13100,
	 13160, 13219, 13279, 13337, 13395, 13453, 13510, 13567,
	 13623, 13678, 13733, 13788, 13842, 13896, 13949, 14001,
	 14053, 14104, 14155, 14206, 14256, 14305, 14354, 14402,
	 14449, 14497, 14543, 14589, 14635, 14680, 14724, 14768,
	 14811, 14854, 14896, 14937, 14978, 15019, 15059, 15098,
	 15137, 15175, 15213, 15250, 15286, 15322, 15357, 15392,
	 15426, 15460, 15493, 15525, 15557, 15588, 15619, 15649,
	 15679, 15707, 15736, 15763, 15791, 15817, 15843, 15868,
	 15893, 15917, 15941, 15964, 15986, 16008, 16029, 16049,
	 16069, 16088, 16107, 16125, 16143, 16160, 16176, 16192,
	 16207, 16221, 16235, 16248, 16261, 16273, 16284, 16295,
	 16305, 16315, 16324, 16332, 16340, 16347, 16353, 16359,
	 16364, 16369, 16373, 16376, 16379, 16381, 16383, 16384,
/*--------------------------------------------------------------------------------------------------*/
	 16384, 16384, 16383, 16381, 16379, 16376, 16373, 16369,
	 16364, 16359, 16353, 16347, 16340, 16332, 16324, 16315,
	 16305, 16295, 16284, 16273, 16261, 16248, 16235, 16221,
	 16207, 16192, 16176, 16160, 16143, 16125, 16107, 16088,
	 16069, 16049, 16029, 16008, 15986, 15964, 15941, 15917,
	 15893, 15868, 15843, 15817, 15791, 15763, 15736, 15707,
	 15679, 15649, 15619, 15588, 15557, 15525, 15493, 15460,
	 15426, 15392, 15357, 15322, 15286, 15250, 15213, 15175,
	 15137, 15098, 15059, 15019, 14978, 14937, 14896, 14854,
	 14811, 14768, 14724, 14680, 14635, 14589, 14543, 14497,
	 14449, 14402, 14354, 14305, 14256, 14206, 14155, 14104,
	 14053, 14001, 13949, 13896, 13842, 13788, 13733, 13678,
	 13623, 13567, 13510, 13453, 13395, 13337, 13279, 13219,
	 13160, 13100, 13039, 12978, 12916, 12854, 12792, 12729,
	 12665, 12601, 12537, 12472, 12406, 12340, 12274, 12207,
	 12140, 12072, 12004, 11935, 11866, 11797, 11727, 11656,
	 11585, 11514, 11442, 11370, 11297, 11224, 11151, 11077,
	 11003, 10928, 10853, 10778, 10702, 10625, 10549, 10471,
	 10394, 10316, 10238, 10159, 10080, 10001,  9921,  9841,
	  9760,  9679,  9598,  9516,  9434,  9352,  9269,  9186,
	  9102,  9019,  8935,  8850,  8765,  8680,  8595,  8509,
	  8423,  8337,  8250,  8163,  8076,  7988,  7900,  7812,
	  7723,  7635,  7545,  7456,  7366,  7276,  7186,  7096,
	  7005,  6914,  6823,  6731,  6639,  6547,  6455,  6363,
	  6270,  6177,  6084,  5990,  5897,  5803,  5708,  5614,
	  5520,  5425,  5330,  5235,  5139,  5044,  4948,  4852,
	  4756,  4660,  4563,  4467,  4370,  4273,  4176,  4078,
	  3981,  3883,  3786,  3688,  3590,  3492,  3393,  3295,
	  3196,  3098,  2999,  2900,  2801,  2702,  2603,  2503,
	  2404,  2305,  2205,  2105,  2006,  1906,  1806,  1706,
	  1606,  1506,  1406,  1306,  1205,  1105,  1005,   904,
	   804,   704,   603,   503,   402,   302,   201,   101,
/*--------------------------------------------------------------------------------------------------*/
	     0,  -101,  -201,  -302,  -402,  -503,  -603,  -704,
	  -804,  -904, -1005, -1105, -1205, -1306, -1406, -1506,
	 -1606, -1706, -1806, -1906, -2006, -2105, -2205, -2305,
	 -2404, -2503, -2603, -2702, -2801, -2900, -2999, -3098,
	 -3196, -3295, -3393, -3492, -3590, -3688, -3786, -3883,
	 -3981, -4078, -4176, -4273, -4370, -4467, -4563, -4660,
	 -4756, -4852, -4948, -5044, -5139, -5235, -5330, -5425,
	 -5520, -5614, -5708, -5803, -5897, -5990, -6084, -6177,
	 -6270, -6363, -6455, -6547, -6639, -6731, -6823, -6914,
	 -7005, -7096, -7186, -7276, -7366, -7456, -7545, -7635,
	 -7723, -7812, -7900, -7988, -8076, -8163, -8250, -8337,
	 -8423, -8509, -8595, -8680, -8765, -8850, -8935, -9019,
	 -9102, -9186, -9269, -9352, -9434, -9516, -9598, -9679,
	 -9760, -9841, -9921,-10001,-10080,-10159,-10238,-10316,
	-10394,-10471,-10549,-10625,-10702,-10778,-10853,-10928,
	-11003,-11077,-11151,-11224,-11297,-11370,-11442,-11514,
	-11585,-11656,-11727,-11797,-11866,-11935,-12004,-12072,
	-12140,-12207,-12274,-12340,-12406,-12472,-12537,-12601,
	-12665,-12729,-12792,-12854,-12916,-12978,-13039,-13100,
	-13160,-13219,-13279,-13337,-13395,-13453,-13510,-13567,
	-13623,-13678,-13733,-13788,-13842,-13896,-13949,-14001,
	-14053,-14104,-14155,-14206,-14256,-14305,-14354,-14402,
	-14449,-14497,-14543,-14589,-14635,-14680,-14724,-14768,
	-14811,-14854,-14896,-14937,-14978,-15019,-15059,-15098,
	-15137,-15175,-15213,-15250,-15286,-15322,-15357,-15392,
	-15426,-15460,-15493,-15525,-15557,-15588,-15619,-15649,
	-15679,-15707,-15736,-15763,-15791,-15817,-15843,-15868,
	-15893,-15917,-15941,-15964,-15986,-16008,-16029,-16049,
	-16069,-16088,-16107,-16125,-16143,-16160,-16176,-16192,
	-16207,-16221,-16235,-16248,-16261,-16273,-16284,-16295,
	-16305,-16315,-16324,-16332,-16340,-16347,-16353,-16359,
	-16364,-16369,-16373,-16376,-16379,-16381,-16383,-16384,
/*--------------------------------------------------------------------------------------------------*/
	-16384,-16384,-16383,-16381,-16379,-16376,-16373,-16369,
	-16364,-16359,-16353,-16347,-16340,-16332,-16324,-16315,
	-16305,-16295,-16284,-16273,-16261,-16248,-16235,-16221,
	-16207,-16192,-16176,-16160,-16143,-16125,-16107,-16088,
	-16069,-16049,-16029,-16008,-15986,-15964,-15941,-15917,
	-15893,-15868,-15843,-15817,-15791,-15763,-15736,-15707,
	-15679,-15649,-15619,-15588,-15557,-15525,-15493,-15460,
	-15426,-15392,-15357,-15322,-15286,-15250,-15213,-15175,
	-15137,-15098,-15059,-15019,-14978,-14937,-14896,-14854,
	-14811,-14768,-14724,-14680,-14635,-14589,-14543,-14497,
	-14449,-14402,-14354,-14305,-14256,-14206,-14155,-14104,
	-14053,-14001,-13949,-13896,-13842,-13788,-13733,-13678,
	-13623,-13567,-13510,-13453,-13395,-13337,-13279,-13219,
	-13160,-13100,-13039,-12978,-12916,-12854,-12792,-12729,
	-12665,-12601,-12537,-12472,-12406,-12340,-12274,-12207,
	-12140,-12072,-12004,-11935,-11866,-11797,-11727,-11656,
	-11585,-11514,-11442,-11370,-11297,-11224,-11151,-11077,
	-11003,-10928,-10853,-10778,-10702,-10625,-10549,-10471,
	-10394,-10316,-10238,-10159,-10080,-10001, -9921, -9841,
	 -9760, -9679, -9598, -9516, -9434, -9352, -9269, -9186,
	 -9102, -9019, -8935, -8850, -8765, -8680, -8595, -8509,
	 -8423, -8337, -8250, -8163, -8076, -7988, -7900, -7812,
	 -7723, -7635, -7545, -7456, -7366, -7276, -7186, -7096,
	 -7005, -6914, -6823, -6731, -6639, -6547, -6455, -6363,
	 -6270, -6177, -6084, -5990, -5897, -5803, -5708, -5614,
	 -5520, -5425, -5330, -5235, -5139, -5044, -4948, -4852,
	 -4756, -4660, -4563, -4467, -4370, -4273, -4176, -4078,
	 -3981, -3883, -3786, -3688, -3590, -3492, -3393, -3295,
	 -3196, -3098, -2999, -2900, -2801, -2702, -2603, -2503,
	 -2404, -2305, -2205, -2105, -2006, -1906, -1806, -1706,
	 -1606, -1506, -1406, -1306, -1205, -1105, -1005,  -904,
	  -804,  -704,  -603,  -503,  -402,  -302,  -201,  -101,
};



/************************************* end of file **************************************************/

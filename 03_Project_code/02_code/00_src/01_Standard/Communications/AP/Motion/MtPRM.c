/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MtPRM.c : Motion Parameter Source File														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Histroy:																						*/
/*		Rev2.01 2007.12.14	Acceleration command addition															*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	<V744>	2010.04.16	K.Ando																		*/
/*		<< Fixing a bug where the moving average time does not work according to the parameter value >>									*/
/*		.FaultNo:A0329																			*/
/*		.Phenomenon:256.0ms - 384.0mswithin the range of256.0mswill operate									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "ScanN.h"
//#include "Basedef.h"
//#include "ServoIf.h"
//#include "Motion.h"
//#include "MlibSvc.h"

/****************************************************************************************************/
/*																									*/
/*  Motion Parameter Definitions 																	*/
/*																									*/
/****************************************************************************************************/
#if 0
#define MAX_TRQ24			0x1000000	/* MAXTRQ */
#define MAX_SPD24			0x1000000	/* MAXSpeed */
#define DISTANCE_MAX		0x7FFFFFFF	/* Travel distanceMAX */
#define ACF_MAX_SZ			512			/* Number of moving average filter buffers */
#define FOLLOWUP_MAX_REF	32767		/* FollowUpMaximum travel amount at time[ref/cyc] */
#define MG_VREMBIT_MAX		24			/* MotionGeneratorRemaining speed calculationMAXNumber of bits */
#define MG_SPDDIV_RANGE		4			/* MotionGeneratorAutomatic setting bit range for decimal point */
#endif


/****************************************************************************************************/
/*																									*/
/*  Motion Parameter Function Prototypes															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*  Grobal Functions																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG MPrmCalcInit(MTPRM* prm, MTPRM_INI* iniPrm);

LONG MPrmSetAccRate1(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
LONG MPrmSetAccRate2(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
LONG MPrmSetAccAltSpd(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
LONG MPrmSetDecRate1(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
LONG MPrmSetDecRate2(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
LONG MPrmSetDecAltSpd(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
LONG MPrmSetExpFilBias(MTPRM* prm, ACF_PRM* acfPrm, LONG bias);
LONG MPrmSetExpFilTb(MTPRM* prm, ACF_PRM* acfPrm, USHORT exptb);
LONG MPrmSetMaFilTb(MTPRM* prm, ACF_PRM* acfPrm, USHORT matb);

LONG MPrmSetStopDecRate1(MTPRM* prm, LONG rate);
LONG MPrmSetStopDecRate2(MTPRM* prm, LONG rate);
LONG MPrmSetStopDecAltSpd(MTPRM* prm, LONG rate);
LONG MPrmSetSegNum(MTPRM* prm, LONG cnt);

LONG MPrmSetSoftLimitFlag(MTPRM* prm, USHORT data);
LONG MPrmSetSoftLimitP(MTPRM* prm, LONG data);
LONG MPrmSetSoftLimitN(MTPRM* prm, LONG data);
LONG MPrmSetAbsOffset(MTPRM* prm, LONG data);
LONG MPrmSetLtRangeP(MTPRM* prm, LONG data);
LONG MPrmSetLtRangeN(MTPRM* prm, LONG data);
LONG MPrmSetZpointWidth(MTPRM* prm, LONG data);
LONG MPrmSetPsetWidth(MTPRM* prm, LONG data);
LONG MPrmSetNearWidth(MTPRM* prm, LONG data);

LONG MPrmSetZretDir(MTPRM* prm, BOOL data);
LONG MPrmSetZretSpd1(MTPRM* prm, LONG data);
LONG MPrmSetZretSpd2(MTPRM* prm, LONG data);
LONG MPrmSetZretDist(MTPRM* prm, LONG data);
LONG MPrmSetExPosingDist(MTPRM* prm, LONG data);
LONG MPrmSetRampTimer(MTPRM* prm, LONG data);

LONG MPrmSetInSigMap(MTPRM* prm, LONG data);
LONG MPrmSetLatchSeqNum(MTPRM* prm, LONG data);
LONG MPrmSetLatchSeqCnt(MTPRM* prm, LONG data);
LONG MPrmSetLatchSeqSgnl(MTPRM* prm, LONG data);
VOID MPrmCalcAccTimeRate(MTPRM* prm, LONG Tspd, LONG Tacc, LONG Tdec);

LONG MPrmSetTlimTimeOut1(MTPRM* prm, LONG data);
LONG MPrmSetTlimTimeOut2(MTPRM* prm, LONG data);
/*--------------------------------------------------------------------------------------------------*/
/* Private sub functions 																			*/
/*--------------------------------------------------------------------------------------------------*/
VOID mPrmCalcPow(LONG data, SHORT exp, LONG* num, LONG* deno);

#if 0
const OPTION_MAP OptMapTbl = {
	3,		/* Acceleration/deceleration filter selection bit position */
	0x3,	/* Acceleration/deceleration filter selection bit mask */
	8,		/* Gain selection bit position */
	0x3,	/* Gain selection bit mask */
	12,		/* P_PISelected bit position */
	0x1,	/* P_PISelected bit mask */
	13,		/* Positional integral clear bit position */
	0x1,	/* Positional integral bit mask */
	14,		/* P_CLBit position */
	0x1,	/* P_CLBit mask */
	15,		/* N_CLBit position */
	0x1,	/* N_CLBit mask */
	31,		/* OutSignalBit position */
	0x7,	/* OutSignalBit mask */
	31,		/* BankSelBit position */
	0xF,	/* BankSelBit mask */
	31,		/* LtDisableBit position */
	0x1,	/* LtDisableBit mask */
};
#endif


/****************************************************************************************************/
/* Motion parameter initial calculation process																	*/
/*																									*/
/*																									*/
/* Summary: Set the following from the currently set motion period and resolution settings								*/
/*																									*/
/*	1)Calculation of unit conversion constant																			*/
/*	PosingRateKx	;[10000ref/s^s]->[***.*ref/cyc^cyc]Speed conversion constant									*/
/*	PosingSpdKx		;[ref/s]->[***.*ref/cyc]Speed conversion constant											*/
/*	PosingSpdInvKx	;[***.*ref/cyc]->[ref/s]Speed conversion constant											*/
/*	PosingSpdMax	;POSINGMaximum speed[ref/s]															*/
/*	fspdRefKx		;[ref/Communication cycle]->[ref/s]Speed conversion constant											*/
/*	refToOsKx		;[Command unit/s]->[24bit/OS]Speed conversion constant											*/
/*	osToRefKx		;[24bit/OS]->[Command unit/s]Speed conversion constant											*/
/*	maxToOsKx		;[30bit/MAX]->[24bit/OS]Speed conversion constant											*/
/*	osToMaxKx		;[24bit/OS]->[30bit/MAX]Speed conversion constant											*/
/*	trqKx			;[24bit/MAX]->[%]Torque conversion constant													*/
/*																									*/
/*	2)Motion setting																				*/
/*	MotionCycle		;Motion control cycle[0.25us]														*/
/*	MG_BASE_PRM		;MGAll basic parameters for															*/
/*																									*/
/*	3)Servo information																					*/
/*	FullcCtrl		;Full-closed control																	*/
/*	Linear			;Linear use																		*/
/*	absPG			;AbsolutePGUsing																		*/
/*	ot_zstop;		;OTTime zero clamp																	*/
/*	trefMode		;TRef Option																	*/
/*	OptMap			;OPTIONCommand assignment																	*/
/****************************************************************************************************/
#pragma arm section code = "IXX_EXCODE"
LONG MPrmCalcInit(MTPRM* prm, MTPRM_INI* iniPrm)
{
	LONG kx, sx;
	LONG ky, sy;
	LONG maxSpd_num, maxSpd_deno;
	LONG ovSpd_num, ovSpd_deno;
	LONG pitch_num, pitch_deno;

/*--------------------------------------------------------------------------------------------------*/
/* Motion cycle set 																		*/
/*--------------------------------------------------------------------------------------------------*/
	prm->BasePrm.MotionCycle = iniPrm->Cycle;

/*--------------------------------------------------------------------------------------------------*/
/* Calculating unit conversion constant for speed[ref/Communication cycle]->[ref/s] 													*/
/*--------------------------------------------------------------------------------------------------*/
	sx = 0;
	prm->BasePrm.fspdRefKx = MlibPcalKxgain(1, 4000000, iniPrm->Cycle, &sx, 24);	

/*--------------------------------------------------------------------------------------------------*/
/* Calculation of torque unit conversion constant[24bit/MAX]->[%] */
/*--------------------------------------------------------------------------------------------------*/
	sx = 0;
	prm->BasePrm.trqKx = MlibPcalKxgain(1, iniPrm->MaxTrq, MAX_TRQ24, &sx, 24);	
/*--------------------------------------------------------------------------------------------------*/
/* Calculation of torque unit conversion constant[%]->[24bit/MAX] */
/*--------------------------------------------------------------------------------------------------*/
	sx = 0;
	prm->BasePrm.invTrqKx = MlibPcalKxgain(1, MAX_TRQ24, iniPrm->MaxTrq, &sx, 24);	

	mPrmCalcPow(1, iniPrm->MaxSpdExp, &maxSpd_num, &maxSpd_deno);
	mPrmCalcPow(1, iniPrm->OverSpdExp, &ovSpd_num, &ovSpd_deno);

/*--------------------------------------------------------------------------------------------------*/
/* Calculating unit conversion constant for speed[30bit/MAX]->[24bit/OS] 													*/
/* buf = spd * maxSpd/(ovSpd*64) 																	*/
/* buf = spd * maxSpd * (maxSpd_num/maxSpd_deno) / (ovSpd*(ovSpd_num/ovSpd_deno)*64) 				*/
/*--------------------------------------------------------------------------------------------------*/
	sx = sy = 0;
	kx = MlibPcalKxmulx(iniPrm->MaxSpd, maxSpd_num, ovSpd_deno, &sx);
	ky = MlibPcalKxmulx(iniPrm->OverSpd*64, ovSpd_num, maxSpd_deno, &sy);
	prm->BasePrm.maxToOsKx = MlibPcalKxdivx(kx, sx, ky, sy, 24);	

/*--------------------------------------------------------------------------------------------------*/
/* Calculating unit conversion constant for speed[24bit/OS]->[30bit/MAX] */
/*--------------------------------------------------------------------------------------------------*/
	prm->BasePrm.osToMaxKx = MlibPcalKxdivx(ky, sy, kx, sx, 24);

/*--------------------------------------------------------------------------------------------------*/
/* Calculating unit conversion constant for speed[Command unit/s]->[24bit/OS] */
/* Calculating unit conversion constant for speed[24bit/OS]->[Command unit/s] */
/*--------------------------------------------------------------------------------------------------*/
	sx = sy = 0;
	if(iniPrm->Linear)
	{
		mPrmCalcPow(1, iniPrm->ScalePitchExp-9, &pitch_num, &pitch_deno);
		kx = MlibPcalKxmulx(iniPrm->ScalePitch, iniPrm->EGearB, MAX_SPD24, &sx);
		kx = MlibPcalKxmulx(kx, ovSpd_deno, pitch_num, &sx);
		ky = MlibPcalKxmulx(iniPrm->EGearA, iniPrm->OverSpd, iniPrm->PgPuls, &sy);  
		ky = MlibPcalKxmulx(ky, pitch_deno, ovSpd_num, &sy);                                     
		prm->BasePrm.refToOsKx = MlibPcalKxdivx(kx, sx, ky, sy, 24);
		prm->BasePrm.osToRefKx = MlibPcalKxdivx(ky, sy, kx, sx, 24);
	}
	else
	{
		kx = MlibPcalKxmulx(60*ovSpd_deno, iniPrm->EGearB, MAX_SPD24, &sx);
		ky = MlibPcalKxmulx(iniPrm->EGearA, iniPrm->OverSpd*ovSpd_num, iniPrm->PgPuls, &sy);
		prm->BasePrm.refToOsKx = MlibPcalKxdivx(kx, sx, ky, sy, 24);
		prm->BasePrm.osToRefKx = MlibPcalKxdivx(ky, sy, kx, sx, 24);
	}

/*--------------------------------------------------------------------------------------------------*/
/* Calculating unit conversion constant for speed[**.*** ref/Control cycle]→[ref/s] 											*/
/* buf = spd * 4000000[.25us] / (2^vel_shift * cyc[.25us]) 											*/
/*--------------------------------------------------------------------------------------------------*/
	sx = sy = 0;
	kx = MlibPcalKxmulx(1, 4000000, 1, &sx);
	ky = MlibPcalKxmulx((1 << iniPrm->SpdDiv), iniPrm->Cycle, 1, &sy);
	prm->BasePrm.PosingSpdInvKx = MlibPcalKxdivx(kx, sx, ky, sy, 24);	
	prm->BasePrm.PosingSpdMax = MlibMulgain32(0x40000000, prm->BasePrm.PosingSpdInvKx);

/*--------------------------------------------------------------------------------------------------*/
/* INDEXERFor: POSINGResolution calculation 																		*/
/*		 TODO)Support for speed command gain switching from parameter												*/
/*		 TODO)Compatible with internal position command electronic gear																*/
/*       *)INDEXERThe maximum speed of99999999 * TspdGain * EgearX.b / EgearX.a ref/min                   */
/*       *)Transform,99999999 * TspdGain * 1/60 * Cycle[0.25us] / 4000000 * EgearX.b / EgearX.a     */
/*       *)simplify25 * TspdGain * Cycle[0.25us] / 60 * EgearX.b / EgearX.a                     */
/*       *)ScanNPeriod=1000us,TspdGain=1000In the case of 99999999ref/min = 196E6AH ref/cycle              */
/*       *)40000000H 196E6AH << (sx).                                    */
/*--------------------------------------------------------------------------------------------------*/
#if 1   /* TspdGainCorrespondence */
	maxSpd_num  = (25 * iniPrm->Cycle * iniPrm->TspdGain * 1/*Iprm.EgearX.b*/) / (60 * 1/*Iprm.EgearX.a*/);
	if ( maxSpd_num == 0 ){ return -1; }                                    /* parameter error */
#else
	maxSpd_num  = (99999999 / 240000) * iniPrm->Cycle ;
#endif
	for ( sx=0 ; (maxSpd_num << sx) <= 0x40000000; sx ++ ) { ; }
	maxSpd_deno = (! (iniPrm->SpdDiv)) ? (sx -1) : (iniPrm->SpdDiv -1);		/* Decimal point bit position */
#if 0
/*--------------------------------------------------------------------------------------------------*/
/* M2For:POSINGResolution calculation --OverSpeedUse level as decimal point 										*/
/*--------------------------------------------------------------------------------------------------*/
	ovSpd_num = MlibMulgain32(MAX_SPD24, prm->BasePrm.osToRefKx);	/* Over Speed [ref/s] */
	for(sx = MG_SPDDIV_RANGE; sx > 0; sx--)
	{
		maxSpd_num = prm->BasePrm.PosingSpdMax >> sx;
		if(ovSpd_num < maxSpd_num)
			break;
	}
	maxSpd_deno = iniPrm->SpdDiv + sx;	/* Decimal point bit position */
#endif


/*--------------------------------------------------------------------------------------------------*/
/* Acceleration/deceleration constant calculation[10000 x ref/s^2]→[**.*** ref/Control cycle^2] 										*/
/* buf = rate * 2^vel_shift * 10000 / (4000000[.25us]/cyc[.25us])^2 								*/
/* buf = rate * 2^vel_shift * cyc[.25us]^2 / (40000)^2 												*/
/*--------------------------------------------------------------------------------------------------*/
	sx = 0;
	kx = MlibPcalKxgain((1 << maxSpd_deno), iniPrm->Cycle, 40000, &sx, 0);
	prm->BasePrm.PosingRateKx = MlibPcalKxgain(kx, iniPrm->Cycle, 40000, &sx, 24);

/*--------------------------------------------------------------------------------------------------*/
/* INDEXERFor: Acceleration/deceleration constant calculation[1000ref/min/msec]→[**.*** ref/Control cycle^2]                         	*/
/*                                    1      Cycle[.25us]       Cycle[.25us]                        */
/* buf = rate * 2^vel_shift * 1000 * ---- * ---------------- * ----------------                     */
/*                                    60     4000000[.25us]     4000[0.25us]                        */
/*                             Cycle[.25us]     Cycle[.25us]                                        */
/*     = rate * 2^vel_shift * -------------- * ---------------                                      */
/*                              240000            4000                                              */
/*--------------------------------------------------------------------------------------------------*/
/*		 TODO)Support for speed command gain switching from parameter												*/
/*		 TODO)Compatible with internal position command electronic gear																*/
/*                                         1      Cycle[.25us]       Cycle[.25us]      EgearX.b     */
/* buf = rate * 2^vel_shift * TspdGain * ---- * ---------------- * ---------------- * ----------    */
/*                                        60     4000000[.25us]     4000[0.25us]       EgearX.a     */
/*--------------------------------------------------------------------------------------------------*/
#if 1   /* TspdGainCorrespondence */
	sx = 0;
	kx = MlibPcalKxgain((1 << maxSpd_deno), iniPrm->TspdGain, 60, &sx, 0);
	kx = MlibPcalKxgain(kx, iniPrm->Cycle, 4000000, &sx, 0);
	kx = MlibPcalKxgain(kx, iniPrm->Cycle, 4000, &sx, 0);
	prm->BasePrm.PosingRateKxIx = MlibPcalKxgain(kx, 1 /* Iprm.EgearX.b */, 1 /* Iprm.EgearX.a */, &sx, 24);
	//prm->BasePrm.PosingRateKxIx = MlibPcalKxgain(kx, iniPrm->Cycle, 4000, &sx, 24);
#else
	sx = 0;
	kx = MlibPcalKxgain((1 << maxSpd_deno), iniPrm->Cycle, 240000, &sx, 0);
	prm->BasePrm.PosingRateKxIx = MlibPcalKxgain(kx, iniPrm->Cycle, 4000, &sx, 24);
#endif

/*--------------------------------------------------------------------------------------------------*/
/* Calculation of speed unit conversion constant[ref/s]→[**.*** ref/Control cycle] 											*/
/* buf = spd * 2^vel_shift / (4000000[.25us]/cyc[.25us]) 											*/
/* buf = spd * 2^vel_shift * cyc[.25us] / 4000000[.25us] 											*/
/*--------------------------------------------------------------------------------------------------*/
	sx = 0;
	prm->BasePrm.PosingSpdKx = MlibPcalKxgain((1 << maxSpd_deno), iniPrm->Cycle, 4000000, &sx, 24);
/*--------------------------------------------------------------------------------------------------*/
/* TODO: INDEXERCalculation of speed unit conversion constant[1000ref/min]→[**.*** ref/Control cycle]                          */
/*                                    1     Cycle[.25us]                                            */
/* buf = spd * 2^vel_shift * 1000 * ---- * ----------------                                         */
/*                                   60     4000000[.25us]                                          */
/*                            Cycle[.25us]                                                          */
/*     = spd * 2^vel_shift * ---------------                                                        */
/*                            240000[.25us]                                                         */
/*		 TODO)Support for speed command gain switching from parameter												*/
/*		 TODO)Compatible with internal position command electronic gear																*/
/*                                        1     Cycle[.25us]       EgearX.b                         */
/* buf = spd * 2^vel_shift * TspdGain * ---- * ---------------- * --------------                    */
/*                                       60     4000000[.25us]     EgearX.a                         */
/*--------------------------------------------------------------------------------------------------*/
#if 1   /* TspdGainCorrespondence */
	sx = 0;
	kx = MlibPcalKxgain((1 << maxSpd_deno), iniPrm->TspdGain, 60, &sx, 0);
	kx = MlibPcalKxgain( kx, iniPrm->Cycle, 4000000, &sx, 0);
	prm->BasePrm.PosingSpdKxIx = MlibPcalKxgain( kx, 1 /* Iprm.EgearX.b */, 1 /* Iprm.EgearX.a */, &sx, 24);
	//prm->BasePrm.PosingSpdKxIx = MlibPcalKxgain( kx, iniPrm->Cycle, 4000000, &sx, 24);
#else
	sx = 0;
	prm->BasePrm.PosingSpdKxIx = MlibPcalKxgain((1 << maxSpd_deno), iniPrm->Cycle, 240000, &sx, 24);
#endif

/*--------------------------------------------------------------------------------------------------*/
/* TODO: INDEXERCalculation of speed unit conversion constant[TspdGain*ref/min]→[rpm]                                     */
/*                      EgearX.b      Egear.b(Pn20E)        1                                       */
/*      kx = TspdGain *----------- * ---------------- * --------                                    */
/*                      EgearX.a      Egear.a(Pn210)     PgPulse                                    */
/*--------------------------------------------------------------------------------------------------*/
	if( (iniPrm->FullcCtrl) || (iniPrm->Linear) )
	{
		return -1;      /* TODO: Linear, Fulk compatible */
	}
	else
	{
		sx = 0;
		kx = MlibPcalKxgain(iniPrm->TspdGain, iniPrm->EGearB, iniPrm->EGearA, &sx, 0);
		kx = MlibPcalKxgain(kx, 1 /* Iprm.EgearX.b */, 1 /* Iprm.EgearX.a */, &sx, 0);
		prm->BasePrm.RefToRpmKxIx = MlibPcalKxgain(kx, 1, iniPrm->PgPuls, &sx, 24);
	}

/*--------------------------------------------------------------------------------------------------*/
/* Calculating unit conversion constant for speed[**.*** ref/Control cycle]→[ref/s] 											*/
/* buf = spd * 4000000[.25us] / (2^vel_shift * cyc[.25us]) 											*/
/*--------------------------------------------------------------------------------------------------*/
	sx = sy = 0;
	kx = MlibPcalKxmulx(1, 4000000, 1, &sx);
	ky = MlibPcalKxmulx((1 << maxSpd_deno), iniPrm->Cycle, 1, &sy);
	prm->BasePrm.PosingSpdInvKx = MlibPcalKxdivx(kx, sx, ky, sy, 24);
	prm->BasePrm.mgSpdMax = MlibMulgain32(0x40000000, prm->BasePrm.PosingSpdInvKx);

/*--------------------------------------------------------------------------------------------------*/
/* MG_BASE_PRMSet 																				*/
/*--------------------------------------------------------------------------------------------------*/
	prm->BasePrm.vel_sft_exp = (UCHAR)iniPrm->SpdDiv;
	prm->BasePrm.vel_shift = (UCHAR)maxSpd_deno;
	prm->BasePrm.vel_shift_s = (UCHAR)(MG_VREMBIT_MAX - maxSpd_deno);
	prm->BasePrm.vel_max = 1 << (31 - maxSpd_deno);
	prm->BasePrm.seg_num = iniPrm->SegNum;
	sx = kx= 0;
	kx = MlibPcalKxgain(prm->BasePrm.PosingSpdMax, iniPrm->Cycle, 4000000, &sx, 24);
	prm->BasePrm.seg_max_ref = MlibMulgain32(iniPrm->SegNum, kx);

	prm->BasePrm.FullcCtrl = iniPrm->FullcCtrl;	/* Full-closed control */
	prm->BasePrm.Linear = iniPrm->Linear;		/* Linear use */
	prm->BasePrm.ot_zstop = iniPrm->ot_zstop;

/*--------------------------------------------------------------------------------------------------*/
/* ACC_TIMECalculation of acceleration unit conversion constant for[(ref/s)/(0.1ms)]→[**.*** ref/Control cycle^2] <Rev2.01>			*/
/*--------------------------------------------------------------------------------------------------*/
	sx = sy = 0;
	kx = MlibPcalKxmulx((1 << maxSpd_deno), iniPrm->Cycle, iniPrm->Cycle, &sx);
	ky = MlibPcalKxmulx(40000, 40000, 1, &sy);
	prm->BasePrm.taccRateKx = MlibPcalKxdivx(kx, sx, ky, sy, 24);	

/*--------------------------------------------------------------------------------------------------*/
/* ACC_TIMECalculation of acceleration unit conversion constant for[(2^7*ref/s)/(0.1ms)]→[**.*** ref/Control cycle^2] <Rev2.01>		*/
/*--------------------------------------------------------------------------------------------------*/
	sy = 0;
	ky = MlibPcalKxmulx(40000, 40000, (1<<7), &sy);
	prm->BasePrm.taccRate7Kx = MlibPcalKxdivx(kx, sx, ky, sy, 24);	

/*--------------------------------------------------------------------------------------------------*/
/* TFF/TLIM/ACC_TIMEUse selection <Rev2.01>*/
/*--------------------------------------------------------------------------------------------------*/
	prm->BasePrm.trefMode = (UCHAR)iniPrm->trefMode;
	prm->BasePrm.TrefModePos = ((iniPrm->OptEnable&0x00F0) != 0) ? iniPrm->trefMode : 0;
	prm->BasePrm.TLimModePos = ((iniPrm->OptEnable&0x0F00) == 0) ? prm->BasePrm.TrefModePos : 0;
	prm->BasePrm.SetCmdAccRate = ((iniPrm->OptEnable&0x0F00) != 0) ? &MPrmCalcAccTimeRate : NULL;

/*--------------------------------------------------------------------------------------------------*/
/* Position information relatedPRMSet */
/*--------------------------------------------------------------------------------------------------*/
	prm->PsnPrm.absPG = iniPrm->absPG;		/* AbsolutePGUsing */
	prm->PsnPrm.TdlyIn = iniPrm->dlyTime;
	prm->PsnPrm.SegNum = iniPrm->SvCycle;

/*--------------------------------------------------------------------------------------------------*/
/* OptionCommand assignment definition */
/*--------------------------------------------------------------------------------------------------*/
#if 0
	prm->OptMap = OptMapTbl;	/* Default value set */
	if((iniPrm->OptEnable & 0x01) != 0)
	{
		if((iniPrm->OptMap[0]&0x00F0) == 0) prm->OptMap.accfilBitMsk = 0;
		if((iniPrm->OptMap[0]&0xF000) == 0)	prm->OptMap.gselBitMsk = 0;
		if((iniPrm->OptMap[1]&0x00F0) == 0)	prm->OptMap.vppiBitMsk = 0;
		if((iniPrm->OptMap[1]&0xF000) == 0)	prm->OptMap.ppiclrBitMsk = 0;
		if((iniPrm->OptMap[2]&0x00F0) == 0)	prm->OptMap.pclBitMsk = 0;
		if((iniPrm->OptMap[2]&0xF000) == 0)	prm->OptMap.nclBitMsk = 0;
		if((iniPrm->OptMap[3]&0x00F0) == 0)	prm->OptMap.bankBitMsk = 0;
		if((iniPrm->OptMap[3]&0xF000) == 0)	prm->OptMap.ltBitMsk = 0;
		if((iniPrm->OptMap[4]&0xF000) == 0)	prm->OptMap.outpBitMsk = 0;
		if(iniPrm->OutSigEnable == 0)		prm->OptMap.outpBitMsk = 0;

		prm->OptMap.accfilBit = (iniPrm->OptMap[0] & 0xF);
		prm->OptMap.gselBit = (iniPrm->OptMap[0] >> 8) & 0xF;
		prm->OptMap.vppiBit = (iniPrm->OptMap[1] & 0xF);
		prm->OptMap.ppiclrBit = (iniPrm->OptMap[1] >> 8) & 0xF;
		prm->OptMap.pclBit = (iniPrm->OptMap[2] & 0xF);
		prm->OptMap.nclBit = (iniPrm->OptMap[2] >> 8) & 0xF;
		prm->OptMap.bankBit = (iniPrm->OptMap[3] & 0xF);
		prm->OptMap.ltBit = (iniPrm->OptMap[3] >> 8) & 0xF;
		prm->OptMap.outpBit = (iniPrm->OptMap[4] >> 8) & 0xF;
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
	return 0;
}

VOID mPrmCalcPow(LONG data, SHORT exp, LONG* num, LONG* deno)
{
	if(exp > 0)
	{
		while((exp--) > 0)
			data *= 10;
		*num = data;
		*deno = 1;
	}
	else if(exp < 0)
	{
		exp = MlibABS(exp);
		while((exp--) > 0)
			data *= 10;
		*num = 1;
		*deno = data;
	}
	else
	{
		*num = data;
		*deno = 1;
	}
}
#pragma arm section code

/****************************************************************************
 Acceleration1Setting(MGFor)
****************************************************************************/
LONG MPrmSetAccRate1(MTPRM* prm, MG_PRM* mgPrm, LONG rate)
{
	LONG data;
	data = MlibMulgain32(rate, prm->BasePrm.PosingRateKx);
	if(data >= 0x40000000)
		data = 0x3FFFFFFF;
	mgPrm->accrate1 = data;
	return 0;
}

/****************************************************************************
 Acceleration2Setting(MGFor)
****************************************************************************/
LONG MPrmSetAccRate2(MTPRM* prm, MG_PRM* mgPrm, LONG rate)
{
	LONG data;
	data = MlibMulgain32(rate, prm->BasePrm.PosingRateKx);
	if(data >= 0x40000000)
		data = 0x3FFFFFFF;
	mgPrm->accrate2 = data;
	return 0;
}

/****************************************************************************
 Acceleration switching speed setting(MGFor)
****************************************************************************/
LONG MPrmSetAccAltSpd(MTPRM* prm, MG_PRM* mgPrm, LONG data)
{
	mgPrm->altspd_a = MlibMulgain32(data, prm->BasePrm.PosingSpdKx);
	return 0;
}

/****************************************************************************
 Deceleration1Setting(MGFor)
****************************************************************************/
LONG MPrmSetDecRate1(MTPRM* prm, MG_PRM* mgPrm, LONG rate)
{
	LONG data;
	data = MlibMulgain32(rate, prm->BasePrm.PosingRateKx);
	if(data >= 0x40000000)
		data = 0x3FFFFFFF;
	mgPrm->decrate1 = data;
	return 0;
}

/****************************************************************************
 Deceleration2Setting(MGFor)
****************************************************************************/
LONG MPrmSetDecRate2(MTPRM* prm, MG_PRM* mgPrm, LONG rate)
{
	LONG data;
	data = MlibMulgain32(rate, prm->BasePrm.PosingRateKx);
	if(data >= 0x40000000)
		data = 0x3FFFFFFF;
	mgPrm->decrate2 = data;
	return 0;
}

/****************************************************************************
 Deceleration switching speed setting(MGFor)
****************************************************************************/
LONG MPrmSetDecAltSpd(MTPRM* prm, MG_PRM* mgPrm, LONG data)
{
	mgPrm->altspd_d = MlibMulgain32(data, prm->BasePrm.PosingSpdKx);
	return 0;
}

/****************************************************************************
 INTERPOLATE(MGFor)
****************************************************************************/
LONG MPrmSetSegNum(MTPRM* prm, LONG cnt)
{
	LONG sx,kx;

	prm->BasePrm.seg_num = cnt;
	sx = kx= 0;
	kx = MlibPcalKxgain(prm->BasePrm.PosingSpdMax, prm->BasePrm.MotionCycle, 4000000, &sx, 24);
	prm->BasePrm.seg_max_ref = MlibMulgain32(cnt, kx);
//	prm->BasePrm.seg_max_ref = (prm->BasePrm.seg_num * prm->BasePrm.vel_max);
	return 0;
}



/****************************************************************************
 Emergency stop deceleration1Setting(MGFor)
****************************************************************************/
LONG MPrmSetStopDecRate1(MTPRM* prm, LONG rate)
{
	LONG data;
	data = MlibMulgain32(rate, prm->BasePrm.PosingRateKx);
	if(data >= 0x40000000)
		data = 0x3FFFFFFF;
	prm->BasePrm.StopPrm.decrate1 = data;
	return 0;
}

/****************************************************************************
 Emergency stop deceleration2Setting(MGFor)
****************************************************************************/
LONG MPrmSetStopDecRate2(MTPRM* prm, LONG rate)
{
	LONG data;
	data = MlibMulgain32(rate, prm->BasePrm.PosingRateKx);
	if(data >= 0x40000000)
		data = 0x3FFFFFFF;
	prm->BasePrm.StopPrm.decrate2 = data;
	return 0;
}

/****************************************************************************
 Emergency stop deceleration switching speed setting(MGFor)
****************************************************************************/
LONG MPrmSetStopDecAltSpd(MTPRM* prm, LONG data)
{
	prm->BasePrm.StopPrm.altspd_d = MlibMulgain32(data, prm->BasePrm.PosingSpdKx);
	return 0;
}






/****************************************************************************
 Exponential function acceleration/deceleration bias setting(AccFilter)
****************************************************************************/
LONG MPrmSetExpFilBias(MTPRM* prm, ACF_PRM* acfPrm, LONG bias)
{
	LONG kx, sx=0;

	/* buf = spd / (4000000[.25us]/cyc[.25us]) */
	/* buf = spd * cyc[.25us] / 4000000[.25us] */
	kx = MlibPcalKxgain(1, prm->BasePrm.MotionCycle, 4000000, &sx, 24);
	acfPrm->expbias = MlibMulgain32(bias, kx);
	return 0;
}

/****************************************************************************
 Exponential acceleration/deceleration time constant setting(AccFilter)
 Summary: Calculate the time constant gain of the exponential function filter and set it as a parameter
      Kt = t / ( t + exptb)
           Kt  :Time constant gain
           t   :Sampling period[0.25us]
           exptb :Time constant[0.25us]
 Argument:
	prm		- Motion ParamterReference to structure
	exptb	- Time constant [0.1ms]
****************************************************************************/
LONG MPrmSetExpFilTb(MTPRM* prm, ACF_PRM* acfPrm, USHORT exptb)
{
	LONG buf;

	/* exptb = 2^vel_shift * (cyc /(exptb*4 + cyc)) */
	buf = ((1 << prm->BasePrm.vel_sft_exp) * prm->BasePrm.MotionCycle);	/* 2^vel_shift * cyc */
	buf = buf / ((exptb*400) + prm->BasePrm.MotionCycle);

	acfPrm->exptb = buf;	/* Parameter saving */

	return 0;
}

/****************************************************************************
 Set the moving average time constant(AccFilter)
 Summary: Calculate the time constant of the moving average filter and set it as a parameter
 Argument:
	prm		- Motion ParamterReference to structure
	matb	- Time constant [0.1ms]
****************************************************************************/
LONG MPrmSetMaFilTb(MTPRM* prm, ACF_PRM* acfPrm, USHORT matb)
{
	LONG	tb, i;
	LONG	nTb, iTb;
	MATB_PRM maPrm;

	tb = matb * 400;	/* [0.1ms]->[0.25us] */
	if(tb >= prm->BasePrm.MotionCycle)
	{/* When the time constant is greater than the control cycle */
		nTb = (tb / prm->BasePrm.MotionCycle);	/* Calculate the moving average number */
		iTb = (USHORT)(nTb / ACF_MAX_SZ);
		if(iTb > 0)
		{
			for(i = 1; i < 5; i++)
			{
/* <V744>		if((1<<i) >= iTb) break;	*/
				if((1<<i) > iTb) break;		/* <V744> */
			}
			tb = nTb / (1<<i);
			maPrm.wTb[0] = (USHORT)i;
			maPrm.wTb[1] = (USHORT)((tb > ACF_MAX_SZ) ? ACF_MAX_SZ: tb);
		}
		else
		{
			maPrm.wTb[0] = 0;
			maPrm.wTb[1] = (USHORT)((nTb > ACF_MAX_SZ) ? ACF_MAX_SZ: nTb);
		}
		acfPrm->mafil.lTb = maPrm.lTb;
	}
	else
	{/* When the time constant is less than the control cycle */
		acfPrm->mafil.lTb = 0;
	}

	return 0;
}


/****************************************************************************
 Soft limit flag setting(Postion Manager)
 Argument:data
    Bit0=0:Positive soft limit enabled, =1:Disabled
    Bit1=0:Negative side soft limit enabled, =1:Disabled
    Bit8=0:Command soft limit disabled, =1:Enabled
****************************************************************************/
LONG MPrmSetSoftLimitFlag(MTPRM* prm, USHORT data)
{
	prm->PsnPrm.sotEnableP = ((data & 0x1) != 0) ? FALSE: TRUE;
	prm->PsnPrm.sotEnableN = ((data & 0x2) != 0) ? FALSE: TRUE;
	prm->PsnPrm.tposLimitEnable = ((data & 0x100) != 0) ? TRUE: FALSE;
	return 0;
}

/****************************************************************************
 Positive soft limit setting(Postion Manager)
 Argument:data[ref]
****************************************************************************/
LONG MPrmSetSoftLimitP(MTPRM* prm, LONG data)
{
	prm->PsnPrm.softLimit_P = data;
	return 0;
}

/****************************************************************************
 Negative side soft limit setting(Postion Manager)
 Argument:data[ref]
****************************************************************************/
LONG MPrmSetSoftLimitN(MTPRM* prm, LONG data)
{
	prm->PsnPrm.softLimit_N = data;
	return 0;
}

/****************************************************************************
 Origin position offset setting(Postion Manager)
 Argument:data[ref]
****************************************************************************/
LONG MPrmSetAbsOffset(MTPRM* prm, LONG data)
{
	prm->PsnPrm.absOfsset = data;
	return 0;
}

/****************************************************************************
 Latch area setting(Postion Manager)
 Argument:data[ref]
****************************************************************************/
LONG MPrmSetLtRangeP(MTPRM* prm, LONG data)
{
	prm->PsnPrm.ltRangeP = data;
	return 0;
}

/****************************************************************************
 Latch area setting(Postion Manager)
 Argument:data[ref]
****************************************************************************/
LONG MPrmSetLtRangeN(MTPRM* prm, LONG data)
{
	prm->PsnPrm.ltRangeN = data;
	return 0;
}

/****************************************************************************
 Origin range setting(Postion Manager)
 Argument:data[ref]
****************************************************************************/
LONG MPrmSetZpointWidth(MTPRM* prm, LONG data)
{
	prm->PsnPrm.zPointWidth = data;
	return 0;
}

/****************************************************************************
 Positioning width setting(Postion Manager)
 Argument:data[ref]
****************************************************************************/
LONG MPrmSetPsetWidth(MTPRM* prm, LONG data)
{
	prm->PsnPrm.psetWidth = data;
	return 0;
}

/****************************************************************************
 Positioning vicinity width setting(Postion Manager)
 Argument:data[ref]
****************************************************************************/
LONG MPrmSetNearWidth(MTPRM* prm, LONG data)
{
	prm->PsnPrm.nearWidth = data;
	return 0;
}



/****************************************************************************
 Setting the home return direction(Motion Control)
 Argument:data=TRUE: Negative direction,FALSE:forward direction
****************************************************************************/
LONG MPrmSetZretDir(MTPRM* prm, BOOL data)
{
	if(data)
		prm->MtSeqPrm.zret_dir = -DISTANCE_MAX;
	else
		prm->MtSeqPrm.zret_dir = DISTANCE_MAX;

	return 0;
}

/****************************************************************************
 Return to origin approach speed1(Motion Control)
 Argument:data [ref/s -> ref/cycle]
****************************************************************************/
LONG MPrmSetZretSpd1(MTPRM* prm, LONG data)
{
	prm->MtSeqPrm.zretsp1 = data;
	return 0;
}

/****************************************************************************
 Return to origin approach speed2(Motion Control)
 Argument:data [ref/s -> ref/cycle]
****************************************************************************/
LONG MPrmSetZretSpd2(MTPRM* prm, LONG data)
{
	prm->MtSeqPrm.zretsp2 = data;
	return 0;
}

/****************************************************************************
 Setting the home return final travel distance(Motion Control)
 Argument:data [ref]
****************************************************************************/
LONG MPrmSetZretDist(MTPRM* prm, LONG data)
{
	prm->MtSeqPrm.zret_dist = data;
	return 0;
}

/****************************************************************************
  set homing distination       
  data : Creep speed
****************************************************************************/
LONG MPrmSetZretOSLvl(MTPRM* prm, LONG data)
{
LONG spd;
	spd = MlibMulgain( data, prm->BasePrm.RefToRpmKxIx );        /* ref/min -> rpm */
	spd = MlibMAX( spd * 2, 50 );                                /* minimum = 50rpm */
	//prm->MtSeqPrm.zretOSLevel = KpiSpeedLevelCal( spd * 10, 0 ); /* set OS level */
	//prm->MtSeqPrm.zretOSLevel = BprmSpeedLevelCal( spd * 10, 0 ); /* set OS level */
	//TODO:
	return 0;
}

/****************************************************************************
 EX_POSINGSet final mileage(Motion Control)
 Argument:data [ref]
****************************************************************************/
LONG MPrmSetExPosingDist(MTPRM* prm, LONG data)
{
	prm->MtSeqPrm.ex_posing_dist = data;
	return 0;
}

/****************************************************************************
 ServoOFFSetting timer for deceleration and stop(Motion Control)
 Argument:data [ms] (0-65535)
****************************************************************************/
LONG MPrmSetRampTimer(MTPRM* prm, LONG data)
{
	LONG sx, kx;

	sx = 0;
	kx = MlibPcalKxgain(data, 4000, prm->BasePrm.MotionCycle, &sx, 24);
	prm->MtSeqPrm.rampTimeOut = MlibMulgain32(1, kx);
//	prm->MtSeqPrm.rampTimeOut = (data*4000)/prm->BasePrm.MotionCycle;
	return 0;
}

/****************************************************************************
 Input signal monitor assignment setting(Motion API)
 Argument:data  bit00-03 IO:0Assignment(n=0:No assignment,n=1-8:SI[n-1]Signal)
             bit04-07 IO:1Assignment
                ;        ;
             bit28-31 IO:7Assignment
****************************************************************************/
#if 0
LONG MPrmSetInSigMap(MTPRM* prm, LONG data)
{
	USHORT msk = 0;
	UCHAR  map, cnt;

	for(cnt = 0; cnt < 8; cnt++)
	{
		map = (UCHAR)((data >> (cnt*4)) & 0xF);
		if(map == 0)
		{
			msk |= (0x1 << cnt);
			map = 1;
		}
		prm->InSigMap.bitNo[cnt] = map - 1;
	}
	prm->InSigMap.bitsMsk = ~msk;

	return 0;
}
#endif


/****************************************************************************
 Latch sequence number setting(Motion Control)
 Argument:data 
****************************************************************************/
#if 0
LONG MPrmSetLatchSeqNum(MTPRM* prm, LONG data)
{
	prm->LtPrm.seqNum = (USHORT)data;
	return 0;
}
#endif

/****************************************************************************
 Latch sequence number setting(Motion Control)
 Argument:data 
****************************************************************************/
#if 0
LONG MPrmSetLatchSeqCnt(MTPRM* prm, LONG data)
{
	prm->LtPrm.loopCnt = (USHORT)data;
	return 0;
}
#endif

/****************************************************************************
 Latch sequence signal setting(Motion Control)
 Argument:data 
****************************************************************************/
#if 0
LONG MPrmSetLatchSeqSgnl(MTPRM* prm, LONG data)
{
	prm->LtPrm.ltSgnl[0] = (UCHAR)(data&0xF);
	prm->LtPrm.ltSgnl[1] = (UCHAR)((data>>4)&0xF);
	prm->LtPrm.ltSgnl[2] = (UCHAR)((data>>8)&0xF);
	prm->LtPrm.ltSgnl[3] = (UCHAR)((data>>12)&0xF);

	prm->LtPrm.ltSgnl[4] = (UCHAR)((data>>16)&0xF);
	prm->LtPrm.ltSgnl[5] = (UCHAR)((data>>20)&0xF);
	prm->LtPrm.ltSgnl[6] = (UCHAR)((data>>24)&0xF);
	prm->LtPrm.ltSgnl[7] = (UCHAR)((data>>28)&0xF);
	return 0;
}
#endif

/****************************************************************************
 For acceleration/deceleration command Acceleration setting(Motion Control) <Rev2.01>
 Argument:Tacc[0.1ms], Tspd[ref/s]
****************************************************************************/
VOID MPrmCalcAccTimeRate(MTPRM* prm, LONG Tspd, LONG Tacc, LONG Tdec)
{
	ULONG spd;
	LONG rate,accrate;

	spd = (ULONG)MlibABS(Tspd);
	if(Tacc == 0){Tacc = 1;}
	if(spd < 0x1000000)
	{
		rate = (spd << 7) / Tacc;
		rate = MlibMulgain32(rate, prm->BasePrm.taccRate7Kx);
	}
	else
	{
		rate = (spd + (Tacc>>1)) / Tacc;
		rate = MlibMulgain32(rate, prm->BasePrm.taccRateKx);
	}
	accrate = MlibLIMITUL(rate, 0x3FFFFFFF, 1);

	prm->CmdRate.accrate2 = accrate;
	prm->CmdRate.decrate2 = accrate;
}

/****************************************************************************
 Setting the push detection time during push return to origin(Motion Control)
 Argument:data [ms]
****************************************************************************/
LONG MPrmSetTlimTimeOut1(MTPRM* prm, LONG data)
{
	prm->MtSeqPrm.tlimTimeCnt1 = KPI_SCANN_MS( data );
	return 0;
}

/****************************************************************************
 Setting wait time for return to origin completion during return to origin by push(Motion Control)
 Argument:data [ms]
****************************************************************************/
LONG MPrmSetTlimTimeOut2(MTPRM* prm, LONG data)
{
	prm->MtSeqPrm.tlimTimeCnt2 = KPI_SCANN_MS( data );
	return 0;
}

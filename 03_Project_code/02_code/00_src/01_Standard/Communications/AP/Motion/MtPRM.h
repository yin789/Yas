/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MtPRM.h : Motion Parameter Header File														*/
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
/*		Rev.1.00 : 2012.08.28  K.Ozaki																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#ifndef		MOTOIN_PRM_H_
#define		MOTOIN_PRM_H_
#include "PRMBankDef.h"

/****************************************************************************************************/
/*																									*/
/*  Motion Parameter Definitions 																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/* Motion operation functionSW: If disabled, comment											*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
#define PRM_BANK_ENABLE		/* Bank switching enabledSW */
#define ACC_FIL_DISABLE	/* Filter disabledSW */
#define POSING_1RATEONLY	/* POSING 1Stage invalidSW */

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
/*  Type Definitions																				*/
/*																									*/
/****************************************************************************************************/



/****************************************************************************************************/
/*																									*/
/*  Motion Parameter Function Prototypes															*/
/*																									*/
/****************************************************************************************************/
extern	LONG MPrmSetSegNum(MTPRM* prm, LONG cnt);

extern	LONG MPrmSetAccRate1(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
extern	LONG MPrmSetAccRate2(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
extern	LONG MPrmSetAccAltSpd(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
extern	LONG MPrmSetDecRate1(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
extern	LONG MPrmSetDecRate2(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
extern	LONG MPrmSetDecAltSpd(MTPRM* prm, MG_PRM* mgPrm, LONG rate);
extern	LONG MPrmSetStopDecRate1(MTPRM* prm, LONG rate);
extern	LONG MPrmSetStopDecRate2(MTPRM* prm, LONG rate);
extern	LONG MPrmSetStopDecAltSpd(MTPRM* prm, LONG rate);

extern	LONG MPrmSetExpFilBias(MTPRM* prm, ACF_PRM* acfPrm, LONG bias);
extern	LONG MPrmSetExpFilTb(MTPRM* prm, ACF_PRM* acfPrm, USHORT exptb);
extern	LONG MPrmSetMaFilTb(MTPRM* prm, ACF_PRM* acfPrm, USHORT matb);

//extern	LONG MPrmSetSoftLimitFlag(MTPRM* prm, USHORT data);
//extern	LONG MPrmSetSoftLimitP(MTPRM* prm, LONG data);
//extern	LONG MPrmSetSoftLimitN(MTPRM* prm, LONG data);
extern	LONG MPrmSetAbsOffset(MTPRM* prm, LONG data);
extern	LONG MPrmSetLtRangeP(MTPRM* prm, LONG data);
extern	LONG MPrmSetLtRangeN(MTPRM* prm, LONG data);
extern	LONG MPrmSetZpointWidth(MTPRM* prm, LONG data);
extern	LONG MPrmSetPsetWidth(MTPRM* prm, LONG data);
extern	LONG MPrmSetNearWidth(MTPRM* prm, LONG data);

//extern	LONG MPrmSetZretDir(MTPRM* prm, BOOL data);
//extern	LONG MPrmSetZretSpd1(MTPRM* prm, LONG data);
//extern	LONG MPrmSetZretSpd2(MTPRM* prm, LONG data);
//extern	LONG MPrmSetZretDist(MTPRM* prm, LONG data);
extern	LONG MPrmSetExPosingDist(MTPRM* prm, LONG data);
extern	LONG MPrmSetRampTimer(MTPRM* prm, LONG data);

extern	LONG MPrmSetInSigMap(MTPRM* prm, LONG data);
extern	LONG MPrmSetLatchSeqNum(MTPRM* prm, LONG data);
extern	LONG MPrmSetLatchSeqCnt(MTPRM* prm, LONG data);
extern	LONG MPrmSetLatchSeqSgnl(MTPRM* prm, LONG data);

#endif /* MOTOIN_PRM_H_ */


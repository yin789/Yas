#ifndef __JAT_OFFLINE_H 
#define __JAT_OFFLINE_H
/****************************************************************************
Description	: Jrate OffLine Function
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
T.kira  2011.03.01    created

*****************************************************************************/
#include "Basedef.h"
#include "ResVibCtrl.h"
#include "CtrlLoopHandler.h"

/****************************************************************************************************/
/*																									*/
/*	Constant definition										            									*/
/*																									*/
/****************************************************************************************************/
//#define	C10POW7			10000000					/* 10^7											*/
//#define	C10POW9			1000000000					/* 10^9											*/
//#define	C2PAIE7			62831853					/* (2*PAI) * 10^7								*/
#define	JSHBITNUM		7							/* Accumulated torque shift number							*/

/*--------------------------------------------------------------------------------------------------*/
/*	Identification sequence constant definition																			*/
/*--------------------------------------------------------------------------------------------------*/
#define	JATINITIAL		0x00						/* Inertia identification not executed							*/
#define	JATCWSTART		0x01						/* Forward rotation→Reverse inertia identification executed					*/
#define	JATCCWSTART		0x02						/* Reverse running→Forward rotation inertia identification executed					*/
#define	JATSUMEND		0x04						/* Torque integration execution completed							*/
#define	JATEXEEND		0x08						/* Inertia identification completed						*/

/*--------------------------------------------------------------------------------------------------*/
/*	Identification status constant definition																			*/
/*--------------------------------------------------------------------------------------------------*/
#define	JATINITIAL		0x00						/* Inertia identification not executed							*/
#define	JATCWCOMP		0x01						/* Forward rotation→Reverse inertia identification completed				*/
#define	JATCCWCOMP		0x02						/* Reverse running→Forward rotation inertia identification completed				*/
#define	JATCOMP			(JATCWCOMP|JATCCWCOMP)		/* Inertia identification completed						*/
#define	JATJRATECOMP	0x04						/* Inertia identification value update complete						*/
#define	JATEXEERR		0x40						/* Executing error									*/
#define	JATERR			0x80						/* Error status								*/

/****************************************************************************************************/
/*																									*/
/*		Variables Definition																		*/
/*																									*/
/****************************************************************************************************/
typedef struct _PRMBK {
	ULONG	kp;							/* Kp:Pn102												*/
	ULONG	jrate;						/* Inertia ratio:Pn103										*/
	ULONG	vffgn;						/* SpeedFFGain:Pn109										*/
} PRMBK;

typedef struct _JATP {
	LONG	mkv;						/* Model Kv												*/
	LONG	mkvi;						/* Model Kvi												*/
	LONG	mklpf;						/* Model Tf												*/
	LONG	mkj;						/* Model inertia gain									*/
	LONG	mjrate;						/* Model inertia											*/
	LONG	jfil;						/* Update the identified value Low-pass filter gain							*/
	LONG	jverrfil;					/* Speed deviation low-pass filter gain for identification error detection			*/
} JATP;

typedef struct _JATV {
	UCHAR	enable;						/* Inertia identification in progress flag	0:Not executed  1:Executing			*/
	UCHAR	status;						/* Inertia identification status									*/
	UCHAR	jarte_start;				/* Inertia identification sequence									*/
	USHORT	jrate;						/* Inertia identification value					[%]						*/
	LONG	mnfb;						/* Model feedback speed			[2^24/OvrSpd]			*/
	LONG	mnfbwrk[2];					/* 															*/
	LONG	mvref;						/* Model speed command					[2^24/OvrSpd]			*/
//	LONG	mner;						/* Model speed deviation											*/
	LONG	mnerp;						/* Model speed deviation(Proportional)										*/
	LONG	mneri;						/* Model speed deviation(Integral)										*/
	LONG	mtrf;						/* Model torque						[2^24/MaxTrq]			*/
	LONG	mneribf[2];					/*															*/
	LONG	jverr;						/* Speed deviation for identification error				[2^24/OvrSpd]			*/
	LONG	sum1;						/* Forward rotation→Reverse rotation actual torque command integrated value								*/
	LONG	msum1;						/* Forward rotation→Reverse model torque command integrated value							*/
	LONG	sum2;						/* Reverse running→Forward rotation actual torque command integrated value								*/
	LONG	msum2;						/* Reverse running→Forward rotation model torque command integrated value							*/
	LONG	j0;							/* Forward rotation→Reverse total inertia identification value								*/
	LONG	j1;							/* Total inertia identification value										*/
	LONG	j2;							/* Reverse running→Forward rotation total inertia identification value								*/
	LONG	TargetDist;					/* Torque integration start distance (identified travel distance×1/2)		[Command unit]	*/
	LONG	fbsum;						/* Identifying inertiaFBPosition(Absolute value from inertia start position)	[Command unit]	*/
	LONG	fbsumrem;					/* 															*/
	BOOL	JatPrmSetReq;				/*															*/
} JATV;
/*--------------------------------------------------------------------------------------------------*/
typedef struct _JATHNDL{
	JATP	jatp;							/* Inertia identification related parameters							*/
	JATP	jatp_wk;						/* Inertia identification related parameters(For parameter retention)		*/
	JATV	jatv;							/* Inertia identification related variables								*/
	PRMBK	prmbk;							/* Related parameter backup values							*/
	USHORT	EstimatJrat;					/* Load inertia ratio identification value			[%]					*/
}JATHNDL;

/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
void BpxJatOfflineCalc( JATHNDL	*jathdl, EGEAR *Egear, CTRL_CMD_PRM *CtrlCmdPrm, 
														CTRL_CMD_MNG *CtrlCmdMngr, LONG	TrqRefo_a );
void CpxJatOfflineCalc( JATHNDL *JatHdl, LONG UnMotSpdx, DETVIB	*DetVib, 
														BOOL PConActFlag, BOOL PerrClrReq );

#endif //__JAT_OFFLINE_H 
/***************************************** end of file **********************************************/

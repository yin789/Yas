#ifndef __FN_ONE_PRM_TUNE_H /* For reload prevention */
/****************************************************************************
  Description   : One Parameter Tuning (Fn203)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.02.10   created

****************************************************************************/
#include "Basedef.h"
#include "PrmAccessIf.h"



/****************************************************************************************************/
/*																									*/
/*		Structure Definition																		*/
/*																									*/
/****************************************************************************************************/
typedef	struct ONEPRMV
{
	struct {
		USHORT	SpatMode;				/* 0x2050 : 1Parameter tuning mode					*/
		USHORT	SpatLevel;				/* 0x2051 : 1Parameter tuning level					*/
		USHORT	SpatFFLevel;			/* 0x2052 : 1Paratuning FF level						*/
		USHORT	SpatLvlChgState;		/* 0x2053 : 1Paraltuning level change complete					*/
		USHORT	SpatANotchState;		/* 0x2054 : 1Paratuning automatic notch completed					*/
		USHORT	SpatAresState;			/* 0x2055 : 1Paratuning automatic type A vibration suppression completed				*/
		USHORT	OnePrmFilType;			/* 0x2095 : 1Paraltuning filter type(AATand common)		*/
		USHORT	Dummy;					/* for Alignment											*/
	} OpeReg;
/*--------------------------------------------------------------------------------------------------*/
	struct {
		BOOL	TunRun;					/* Tuning in progress										*/
		BOOL	TunSel;					/* FB/FFTuning selection									*/
		BOOL	FftWait;				/* Waiting for automatic notch execution										*/
		BOOL	NotchChk;				/* Response check after automatic notch setting								*/
		BOOL	FftReq;					/* Frequency analysis request											*/
		BOOL	ModeUnmatch;			/* Tuning mode unmatch								*/
/*--------------------------------------------------------------------------------------------------*/
		ULONG	FftWaitTimer;			/* Auto notch execution timer										*/
		ULONG	FfWaitTimer;			/* FFSetting wait timer											*/
		ULONG	NotchChkTimer;			/* Notch check timer										*/
/*--------------------------------------------------------------------------------------------------*/
		UCHAR	ModeMin;				/* Tuning mode setting minimum value								*/
		UCHAR	ModeMax;				/* Tuning mode setting maximum value								*/
/*--------------------------------------------------------------------------------------------------*/
		UCHAR	DigMax;					/* Maximum digit of tuning level movement								*/
		UCHAR	Dummy1;					/* for Alignment											*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	FfSetting;				/* FFTuning setting										*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	Jrate;					/* Moment of inertia/Weight ratio setting								*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	OrgKv;					/* before changeKv												*/
		USHORT	OrgMdlgn;				/* before changeKpm												*/
		USHORT	OrgNf1;					/* Before change1Notch filter frequency							*/
		USHORT	OrgNf2;					/* Before change2Notch filter frequency							*/
		USHORT	OrgNfQ1;				/* Before change1Notch filterQvalue								*/
		USHORT	OrgNfQ2;				/* Before change2Notch filterQvalue								*/
		USHORT	OrgAResf;				/* A-type vibration suppression frequency before change									*/
		USHORT	OrgAResDGn;				/* A-type damping damping gain before change							*/
/*--------------------------------------------------------------------------------------------------*/
		SHORT	FftStat;				/* Frequency analysis status										*/
		SHORT	Dummy2;					/* for Alignment											*/
/*--------------------------------------------------------------------------------------------------*/
		UCHAR	ANotchSetStat;			/* Automatic notch setting result status									*/
/*--------------------------------------------------------------------------------------------------*/
		UCHAR	ANotchStat;				/* Automatic notch setting execution status									*/
		UCHAR	ANotch1Stat;			/* Automatic notch 1st stage notch setting result							*/
		UCHAR	ANotch2Stat;			/* Automatic notch 2nd stage notch setting result							*/
		UCHAR	ANotchPreStat;			/* Notch setting immediately before automatic notch									*/
/*--------------------------------------------------------------------------------------------------*/
		UCHAR	AResSetStat;			/* Automatic type A setting result status										*/
/*--------------------------------------------------------------------------------------------------*/
		UCHAR	AResStat;				/* Automatic A-type setting execution status										*/
		UCHAR	ARes1Stat;				/* Automatic A-type setting result											*/
/*--------------------------------------------------------------------------------------------------*/
	} var;
} ONEPRMTUNE;


typedef	struct TUNEGAIN
{
	USHORT	loophz;					/* Pn100:Speed loop gain										*/
	USHORT	pitime;					/* Pn101:Speed loop integral time constant									*/
	USHORT	posgn;					/* Pn102:Position loop gain										*/
	USHORT	trqfil11;				/* Pn401:Torque filter time constant									*/
	USHORT	mdlgn;					/* Pn141:Model loop gain										*/
	USHORT	dobgn;					/* Pn121:Friction compensation gain											*/
	USHORT	dlpfil;					/* Pn124:Friction compensation frequency correction										*/
	USHORT	ntchq1;					/* Pn40A:No.1Notch filterQvalue									*/
	USHORT	ntchq2;					/* Pn40D:No.2Notch filterQvalue									*/
} TUNEGAIN;


typedef	struct TUNECALV
{
	ULONG		PIRate;
	ULONG		KpZeta;
	ULONG		Ti;
	TUNEGAIN	Gain;				/* Tuning gain											*/
} TUNECALV;


/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
enum OPTUNE_SEQ_STS {
	OPTUNE_SEQ_INIT				= 0,		/* Initial processing												*/
	OPTUNE_SEQ_JSET,						/* Moment of inertia ratio setting process								*/
	OPTUNE_SEQ_MODESEL,						/* Mode setting processing										*/
	OPTUNE_SEQ_FILSEL,						/* Filter type setting processing								*/
	OPTUNE_SEQ_PREDISP,						/* Pre-adjustment gain display processing									*/
	OPTUNE_SEQ_TUNING,						/* 1Para adjustment processing										*/
	OPTUNE_SEQ_VIBSEL,						/* Vibration suppression processing											*/
	OPTUNE_SEQ_POSTDISP,					/* Post-adjustment gain display process									*/
	OPTUNE_SEQ_NOTCHERR,					/* Notch detection error processing									*/
	OPTUNE_SEQ_END,							/* Termination processing												*/
};
/*--------------------------------------------------------------------------------------------------*/
enum OPTUNE_FLAG_SEL {
	OPTUNE_FLAG_FBSEL			= 0,		/* Feedback gain selection								*/
	OPTUNE_FLAG_FFSEL,						/* Feedforward gain selection							*/
};
/*--------------------------------------------------------------------------------------------------*/
enum OPTUNE_ARES_DETVIB_STS {
	OPTUNE_STS_ARES_NOVIBDET	= 0,		/* A-type vibration suppression completion status: Vibration not detected					*/
	OPTUNE_STS_ARES_DETECT,					/* A-type vibration suppression completion status: Vibration detection in progress					*/
	OPTUNE_STS_ARES_NOENABLE,				/* A-type vibration suppression completion status: Automatic setting not possible					*/
	OPTUNE_STS_ARES_DET_ERR,				/* A-type vibration suppression completion status: Detection error					*/
};
/*--------------------------------------------------------------------------------------------------*/
enum OPTUNE_ARES_SET_STS {
	OPTUNE_STS_ARES_INIT		= 0,		/* Initial setting of A-type damping										*/
	OPTUNE_STS_ARES_OK,						/* A-type vibration damping setting successful completion									*/
};
/*--------------------------------------------------------------------------------------------------*/
enum OPTUNE_ARES_COMP_STS {
	OPTUNE_STS_ARES_NODETECT	= 0,		/* A-type vibration suppression completion status: Not implemented						*/
	OPTUNE_STS_ARES_COMP,					/* A-type vibration damping completion status: Setting complete						*/
	OPTUNE_STS_ARES_NG,						/* A-type vibration suppression completion status: Setting failed						*/
};
/*--------------------------------------------------------------------------------------------------*/
enum OPTUNE_FF_STS {
	OPTUNE_STS_FFCOMP			= 0,		/* FFLevel change completion status: Change completed/Initial			*/
	OPTUNE_STS_FFWAIT,						/* FFLevel change completion status: Waiting for change						*/
	OPTUNE_STS_FFTMOUT,						/* FFLevel change completion status: Change failed(Timeout)			*/
};
/*--------------------------------------------------------------------------------------------------*/


#define		OPTUNE_JTUN_MAX		5			/* Moment of inertia/Maximum digit for weight ratio adjustment						*/
#define		OPTUNE_MFC_ACTIVE	0x0001		/* Rigid body setting(Pn140)									*/
#define		OPTUNE_MODE_MIN		0			/* 1Para mode lower limit value									*/
#define		OPTUNE_MODE_MAX		3			/* 1Para mode upper limit value									*/
#define		TUNE_MAX_TI			3000		/* TiUpper limit												*/



/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/


/****************************************************************************************************/
/*		API's																						*/
/****************************************************************************************************/
void		OnePrmReSetTuningLevel( ONEPRMTUNE *OnePrmTune, USHORT loophz, USHORT posgn, BOOL IPSpdControl );
PRM_RSLT	RpiFunSetOnePrmTuningMode( ONEPRMTUNE *OnePrmTune, USHORT Mode );
PRM_RSLT	RpiFunSetOnePrmTuningLevel( ONEPRMTUNE *OnePrmTune, USHORT Lvl );
PRM_RSLT	RpiFunSetOnePrmTuningFfLevel( ONEPRMTUNE *OnePrmTune, USHORT Lvl );
void		RpiRegSetOnePrmTuningLevel( ONEPRMTUNE *OnePrmTune, USHORT Gain );
void		RpiRegSetOnePrmTuningFfLevel( ONEPRMTUNE *OnePrmTune, USHORT Gain );



/****************************************************************************************************/
#define __FN_ONE_PRM_TUNE_H
#endif /* __FN_ONE_PRM_TUNE_H */

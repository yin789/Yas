#ifndef __FN_ATYPE_VIB_TUNE_H /* For reload prevention */
/****************************************************************************
  Description   : A Type Vibration Tuning (Fn204)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.02.15   created

****************************************************************************/
#include "Basedef.h"
#include "PrmAccessIf.h"




/****************************************************************************************************/
/*																									*/
/*		Structure Definition																		*/
/*																									*/
/****************************************************************************************************/
typedef	struct AVIBTUNE
{
	struct {
		USHORT	AvibMode;			/* 0x20A0 : AMold vibration damping function adjustment mode setting							*/
		USHORT	AvibState;			/* 0x20A1 : AMold vibration damping function adjustment complete									*/
		USHORT	AvibFreq;			/* 0x20A2 : AMold vibration damping function adjustment frequency								*/
		USHORT	AvibGain;			/* 0x20A3 : AMold vibration damping function adjustment gain								*/
	} OpeReg;
/*--------------------------------------------------------------------------------------------------*/
	struct {
		SHORT	FftStat;			/* Frequency analysis status											*/
		SHORT	dummy;				/* for Alignment												*/
		BOOL	FftReq;				/* Frequency analysis request												*/
		BOOL	Match;				/* Does the display and action match?									*/
		LONG	VibDetLvl;			/* Vibration detection level												*/
		ULONG	TimeOutCtr;			/* Timeout detection counter									*/
	} var;
} AVIBTUNE;



/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
enum AVIB_SEQ_STS {
	AVIB_SEQ_INIT		= 0,		/* Initial processing														*/
	AVIB_SEQ_MODESEL,				/* Automatic/Manual mode selection											*/
	AVIB_SEQ_TRGWAIT,				/* Vibration detection trigger wait											*/
	AVIB_SEQ_MEASURE,				/* Frequency analysis in progress													*/
	AVIB_SEQ_COMP,					/* Frequency analysis completed(Waiting for filter settings)								*/
	AVIB_SEQ_SET,					/* Filter settings in progress												*/
	AVIB_SEQ_FREQTUNE,				/* Frequency adjustment													*/
	AVIB_SEQ_GAINTUNE,				/* Damping gain adjustment											*/
	AVIB_SEQ_END,					/* Termination processing														*/
	AVIB_SEQ_NOOPWAIT,				/* No Operation Display weight									*/
};
/*--------------------------------------------------------------------------------------------------*/
enum AVIB_CMP_STS {
	AVIB_STS_NONCOMP	= 0,		/* Vibration detection completed: Detection not completed										*/
	AVIB_STS_COMP,					/* Vibration detection completed: Detection completed										*/
	AVIB_STS_TIMEOUT,				/* Vibration detection completed: Detection not possible(Timeout)							*/
	AVIB_STS_RANGEOV,				/* Vibration detection completed: Detection not possible(Out of frequency range)							*/
	AVIB_STS_NONVIB,				/* Vibration detection completed: Detection not possible(Unable to specify frequency)						*/
};
/*--------------------------------------------------------------------------------------------------*/
#define		AVIB_FRQ_MIN			100		/* Detection frequency lower limit 100Hz									*/
#define		AVIB_FRQ_MAX			1000	/* Detection frequency upper limit 1000Hz								*/
/*--------------------------------------------------------------------------------------------------*/
#define		AVIB_MODE_MIN			0		/* Mode lower limit											*/
#define		AVIB_MODE_MAX			1		/* Mode upper limit											*/
/*--------------------------------------------------------------------------------------------------*/
#define		AVIB_GTUN_MAX			2		/* Maximum gain adjustment digit										*/
#define		AVIB_FTUN_MAX			4		/* Frequency adjustment maximum digit										*/
#define		AVIB_TUN_MIN			1		/* Gain/Minimum frequency adjustment digit								*/



/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	RpiFunSetAtypeVibTuningMode( AVIBTUNE *AvibTune, USHORT Mode );
PRM_RSLT	RpiFunSetAtypeVibTuningFrequency( AVIBTUNE *AvibTune, USHORT Frq );
PRM_RSLT	RpiFunSetAtypeVibTuningGain( AVIBTUNE *AvibTune, USHORT Gain );
void		RpiRegSetAtypeVibTuningFrequency( AVIBTUNE *AvibTune, USHORT Frq );
void		RpiRegSetAtypeVibTuningGain( AVIBTUNE *AvibTune, USHORT Gain );



/****************************************************************************************************/
#define __FN_ATYPE_VIB_TUNE_H
#endif /* __FN_ATYPE_VIB_TUNE_H */

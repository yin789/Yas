#ifndef __FN_VIB_MONITOR_H /* For reload prevention */
/****************************************************************************
  Description   : Online Vibration Monitor (Fn207)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.01.28   created

****************************************************************************/
#include "Basedef.h"




/****************************************************************************************************/
/*																									*/
/*		Structure Definition																		*/
/*																									*/
/****************************************************************************************************/
typedef	struct VIBMON
{
	struct {
		USHORT	VibmState;					/* 0x2017 : Online vibration monitor completion confirmation				*/
	} OpeReg;
/*--------------------------------------------------------------------------------------------------*/
	struct {
		BOOL	FftReq;						/* Frequency analysis request										*/
		SHORT	VibDetLvl;					/* Frequency analysis detection trigger level							*/
		SHORT	FftStat;					/* Frequency analysis status									*/
		ULONG	TimeOutCtr;					/* Timeout detection counter							*/
	} var;
} VIBMON;


/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
enum VIBMON_SEQ_STS {
	VIBMON_SEQ_INIT		= 0,				/* Waiting for vibration level detection to start								*/
	VIBMON_SEQ_DETECT,						/* Frequency analysis in progress											*/
	VIBMON_SEQ_DISPLAY,						/* Frequency analysis result(Waiting for filter settings)						*/
	VIBMON_SEQ_CALC,						/* Filter settings in progress										*/
	VIBMON_SEQ_STORE,						/* Saving filter										*/
	VIBMON_SEQ_DETERR,						/* Frequency analysis error										*/
	VIBMON_SEQ_END,							/* Termination processing												*/
};
/*--------------------------------------------------------------------------------------------------*/
enum VIBMON_CMP_STS {
	VMON_STS_END		= 0,				/* Complete													*/
	VMON_STS_RUN,							/* Not completed(Executing)										*/
	VMON_STS_ERR,							/* Detection not possible												*/
	VMON_STS_STOP,							/* Detection stop												*/
};



/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/




/****************************************************************************************************/
#define __FN_VIB_MONITOR_H
#endif /* __FN_VIB_MONITOR_H */

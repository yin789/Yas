#ifndef __FN_SET_TUNE_LESS_LVL_H /* For reload prevention */
/****************************************************************************
  Description   : Set Tuning Less Level (Fn200)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.01.31   created

****************************************************************************/
#include "Basedef.h"
#include "BaseControls.h"
#include "PrmAccessIf.h"



/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
//#define	DAT_GAINLEVEL_MAX			4		/* Maximum value of gain level without adjustment						*//* <S1C7> */
#define		DAT_GAINLEVEL_MAX			7		/* Maximum value of gain level without adjustment						*//* <S1C7> */
#define		DAT_GAINLEVEL_MIN			0		/* Minimum value of gain level without adjustment						*/
#define		DAT_JRATLEVEL_MAX			2		/* Maximum inertia level without adjustment					*/
#define		DAT_JRATLEVEL_MIN			0		/* Minimum inertia level without adjustment					*/
/*--------------------------------------------------------------------------------------------------*/
#define		DAT_SEQ_INIT				0		/* Initial processing											*/
#define		DAT_SEQ_TUNING				1		/* Gain level adjustment processing								*/
#define		DAT_SEQ_END					10		/* Termination processing											*/


/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
void		RpiRegSetTuneLessLevel( ULONG Gain, TUNELESS_CTRL *TuneLessCtrl );
PRM_RSLT	RpiFunSetDATGainLevel( TUNELESS_CTRL *TuneLessCtrl, USHORT Level );
PRM_RSLT	RpiFunSetDATJratLevel( TUNELESS_CTRL *TuneLessCtrl, USHORT Level );



/****************************************************************************************************/
#define __FN_SET_TUNE_LESS_LVL_H
#endif /* __FN_SET_TUNE_LESS_LVL_H */

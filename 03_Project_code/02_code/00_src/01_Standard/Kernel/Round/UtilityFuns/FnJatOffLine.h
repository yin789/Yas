#ifndef _FN_JAT_OFFLINE_H
#define _FN_JAT_OFFLINE_H
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
#include "JatOffLine.h"
#include "PrmAccessIf.h"
#include "UnitPrmStruct.h"
#include "GainChange.h"
/****************************************************************************************************/
/*																									*/
/*	Constant definition										            									*/
/*																									*/
/****************************************************************************************************/
/*ScanBchanges dynamically, so it must be given as an argument													*/
//#define	JMDLCALCYCNS	KPI_SBCYCLENS				/* Model calculation cycle[ns]	ScanB)			*/
#define	JRATCALCYCUS	(KPI_SCCYCLEMS*1000)		/* Inertia calculation cycle[us]	(ScanC)			*/
#define	JRATVERRFIL		1000						/* Speed deviation filter frequency for identification error[0.1Hz]	*/
#define	JATFIL			100000						/* Inertia update filter time constant:100000[us]		*/
#define	JATVFFGN		100							/* Speed during inertia identificationFFGain:100[%]			*/

/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
SHORT CpiInitJatOffLine( JATHNDL *JatHdl, UNI_PRM_STR *UniPrms, REG_MANAGER_HANDLE *RegManager, 
			GAIN_CHNG_HNDL GainChange, SHORT InitSw, LONG distance, UCHAR CtrlMode, LONG SvCycleNs);
void CpiEndJatOffLine( JATHNDL *JatHdl, REG_ACC_T *AccessPrm );
CHAR CpiGetJatExeErrStatus( JATHNDL	*JatHdl );	
CHAR CpiGetJatErrStatus( JATHNDL *JatHdl );		
CHAR CpiGetJatCompStatus( JATHNDL *JatHdl );		
void CpiSetEstimateJrat( JATHNDL *JatHdl, REG_ACC_T *AccessPrm );

#endif //_FN_JAT_OFFLINE_H 
/***************************************** end of file **********************************************/

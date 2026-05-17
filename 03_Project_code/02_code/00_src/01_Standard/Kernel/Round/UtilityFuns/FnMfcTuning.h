#ifndef __FN_MFC_TUNE_H /* For reload prevention */
/****************************************************************************
  Description   : Mfc with Vibration Suppression Tuning (Fn205)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.01.25   created

****************************************************************************/
#include "Basedef.h"
#include "PrmAccessIf.h"



/****************************************************************************************************/
/*																									*/
/*		Structure Definition																		*/
/*																									*/
/****************************************************************************************************/
typedef	struct MFCTUNE {
	struct {
		USHORT	MdlVibFreq;					/* 0x20B0 : With vibration suppressionMFCAdjustment frequency					*/
		USHORT	MdlVibState;				/* 0x20B1 : With vibration suppressionMFCAdjustment completed						*/
		USHORT	MdlVibFreq2;				/* 0x20B2 : Vibration suppression function tuning frequency2				*/
		USHORT	MdlVibFil2;					/* 0x20B3 : Vibration suppression function tuning frequency2Correction			*/
		USHORT	MdlVibState2;				/* 0x20B4 : Vibration suppression function setting2Confirm completion					*/
		USHORT	Dummy;						/* For Alignment										*/
	} OpeReg;
/*--------------------------------------------------------------------------------------------------*/
	struct {
		BOOL	DispMatch;					/* Does the display and action match?							*/
		ULONG	FfWaitCntr;					/* Waiting time elapsed counter									*/
		USHORT	DetFreq;					/* Detection frequency											*/
		USHORT	OrgMdlgn;					/* before changeKpm											*/
		USHORT	OrgAntFrq;					/* before changefa											*/
		USHORT	OrgResFrq;					/* before changefr											*/
		USHORT	OrgMdlSw;					/* Switch before change										*/
		USHORT	Dummy;						/* For Alignment										*/
	} var;
} MFCTUNE;



/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
enum VIBMFC_SEQ_STS {
	VIBMFC_SEQ_INIT		= 0,				/* Initial processing												*/
	VIBMFC_SEQ_MEASURE,						/* Vibration detection in progress											*/
	VIBMFC_SEQ_ADJUST,						/* Fine adjustment of vibration frequency										*/
	VIBMFC_SEQ_SETREQ,						/* Frequency(Model following control with vibration suppression)Setting				*/
	VIBMFC_SEQ_WAITMFC,						/* Waiting for frequency setting to be valid(Model following control is already enabled)			*/
	VIBMFC_SEQ_WAITNORM,					/* Waiting for model following control with vibration suppression setting to take effect				*/
	VIBMFC_SEQ_SETCOMP,						/* Setting completed												*/
	VIBMFC_SEQ_END,							/* Termination processing												*/
	VIBMFC_SEQ_NOOPWAIT,					/* No Operation Display weight							*/
};
/*--------------------------------------------------------------------------------------------------*/
enum VIBMFC_CMP_STS {
	VIBMFC_STS_COMP		= 0,				/* Completion status: Setting completed/Initial					*/
	VIBMFC_STS_WAIT,						/* Completion status: Waiting for setting								*/
	VIBMFC_STS_TMOUT,						/* Completion status: Setting failed(Timeout)				*/
};
/*--------------------------------------------------------------------------------------------------*/
#define		VIBMFC_TUN_MAX			3		/* Maximum gain adjustment digit										*/
#define		VIBMFC_TUN_MIN			1		/* Gain/Minimum frequency adjustment digit								*/
/*--------------------------------------------------------------------------------------------------*/
#define		VIBMFC_DET_FRQ_MIN		10		/* Fn023 1.0Hz				*/
#define		VIBMFC_DET_FRQ_MAX		1000	/* Fn023 100.0Hz				*/
/*--------------------------------------------------------------------------------------------------*/
#define		VIBMFC_ONE_MASS			0x01	/* Rigid body model											*/
#define		VIBMFC_TWO_MASS1		0x02	/* 2 inertia model 1									*/
#define		VIBMFC_TWO_MASS2		0x03	/* 2 inertial model 2									*/
#define		VIBMFC_ONE_BASE			0x04	/* Rigid body + machine model									*/
/*--------------------------------------------------------------------------------------------------*/
#define		VIBMFC_PRM_ACTIVE		0x0011	/* Rigid body + machine model setting(Pn140)						*/
/*--------------------------------------------------------------------------------------------------*/
#define		VIBMFC_FF_WAITTMOUT		8000	/* FFLevel change timeout:8[sec]						*/



/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	RpiMfcTuningSetFreq( MFCTUNE *MfcTune, USHORT Frq );
PRM_RSLT	RpiMfcTuningSetFreq2( MFCTUNE *MfcTune, USHORT Frq );
PRM_RSLT	RpiMfcTuningSetFreq2Comp( MFCTUNE *MfcTune, USHORT Fil );
void		RpiRegSetVibMfcTuningFrequency( MFCTUNE *MfcTune, USHORT Frq );
void		RpiRegSetMdlVibFreq2( MFCTUNE *MfcTune, USHORT Frq2 );
void		RpiRegSetMdlVibFil2( MFCTUNE *MfcTune, USHORT Fil );



/****************************************************************************************************/
#define __FN_MFC_TUNE_H
#endif /* __FN_MFC_TUNE_H */

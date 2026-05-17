#ifndef __AUTO_NOTCH_SET_H 
#define __AUTO_NOTCH_SET_H
/****************************************************************************
Description	: Auto Notch Filter Parameter Setting Function
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
Y.Oka  2010.11.17    created

*****************************************************************************/
#include "Basedef.h"
#include "TuneLib.h"
#include "ResVibCtrl.h"
#include "PrmAccessIf.h"
#include "UnitPrmStruct.h"


/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
#define		ANOTCH_MIN_SET_FREQ			400		/* Automatic notch minimum setting frequency	[Hz]					*/
#define		ANOTCH_MIN_SET_FREQ2		200		/* Automatic notch minimum setting frequency during forced detection	[Hz]		*/
#define		ANOTCH_NOTCH_SET_CHK_TIME	3000	/* Waiting time after notch setting		[ms]					*/
/*--------------------------------------------------------------------------------------------------*/
#define		NOTCH_COMP_MAX_FREQ			800		/* Notch frequency correction maximum frequency	[Hz]				*/
#define		NOTCH_COMP_RATIO			170		/* Notch frequency correction ratio			[%]					*/
/*--------------------------------------------------------------------------------------------------*/
#define		NOTCH_BASIC_VIB_LEVEL		600		/* Vibration detection level(Rotation)			[0.1min-1]			*/
#define		NOTCH_BASIC_VIB_LEVEL_L		200		/* Vibration detection level(Linear)			[0.1mm/s]			*/
#define		NOTCH_FILTYPE				2		/* Filter type during auto notch Type2Fixed			*/
#define		NOTCH_FILTYPE2				3		/* 2000Hzor more is automatic notch filter type Type3	*/
#define		FREQDETUNIT					20		/* Frequency analysis detection frequency resolution						*/
#define		ANOTCH_FRAT_106( data )		(((data)*17)>>4)	/* 106.25%								*/




/****************************************************************************************************/
/*																									*/
/*		Struct Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		For automatic notch sequence Constant/variable data definition													*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct ANOTCHSEQ {
	struct {								/* Register definition for automatic notch sequence					*/
		USHORT		SpatANotchState;			/* 0x2054 : Auto notch status					*/
	} OpeReg;
	struct {								/* Constant definition for automatic notch sequence						*/
		USHORT		kv;							/* Speed loop gain for automatic notch calculation		[0.1Hz]		*/
		USHORT		jrate;						/* Load inertia moment for automatic notch calculation/Mass ratio[%]		*/
		BOOL		EepromWriteProtect;			/* EEPROMWrite prohibition flag							*/
		BOOL		BaseEnableSts;				/* ServoON/OFFStatus flag							*/
		BOOL		TuneLessSts;				/* Adjustmentless enabled/invalid flag							*/
		BOOL		OverTravelSts;				/* P-OT/N-OTStatus flag								*/
		LONG		VibCheckLevelForTunLess ;	/* Vibration detection level for automatic notch			[2^24/OS]	*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Variable definition for automatic notch sequence						*/
		BOOL		FftReq;						/* Frequency analysis request									*/
		BOOL		NotchChk;					/* Notch checking in progress									*/
		ULONG		NotchChkTimer;				/* Notch check timer(Vibration detection wait)				*/
		SHORT		FftStat;					/* Frequency analysis status								*/
		SHORT		ANotchSetStat;				/* Automatic notch setting result status							*/
		UCHAR		ANotchStat;					/* Automatic notch setting execution status							*/
		UCHAR		ANotch1Stat;				/* Automatic notch 1st stage notch setting result					*/
		UCHAR		ANotch2Stat;				/* Automatic notch 2nd stage notch setting result					*/
		UCHAR		ANotchPreStat;				/* Notch setting immediately before automatic notch							*/
	} var;
} ANOTCHSEQ;



/*--------------------------------------------------------------------------------------------------*/
/*		For automatic notch sequence call Pointer structure definition												*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct ANOTCHSEQCTRL {
	ANOTCHSEQ			*ANotchSeq;			/* Structure for automatic notch sequence							*/
	AUTONOTCH			*ANotch;			/* Structure for automatic notch setting								*/
	FFTANALYZE			*FftAna;			/* Vibration frequency analysis(Online FFT)					*/
	DETVIB				*DetVib;			/* Vibration detection structure										*/
	PRMDATA				*Prm;				/* Parameter data structure								*/
	REGIF_CONFIG_T		*RegCfg;			/* RegisterIFConfiguration structure				*/
} ANOTCHSEQCTRL;



/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
void	AutoNotchFilterSetting( ANOTCHSEQCTRL *ANotchCtrl );
void	AutoNotchInitialize( ANOTCHSEQ *ANotchSeq );
void	AutoNotchCalculatePrm( ANOTCHSEQ *ANotchSeq, USHORT kv, USHORT jrate);
void	AutoNotchCalculateInitPrm( ANOTCHSEQ *ANotchSeq, AUTONOTCH *ANotch, LONG AmpType, UNI_PRM_STR *UniPrm );
void	RpiResetStatusANotch1( ANOTCHSEQ *ANotchSeq );
void	RpiResetStatusANotch2( ANOTCHSEQ *ANotchSeq );
UCHAR	RpiGetANotchPreStat( ANOTCHSEQ *ANotchSeq );
USHORT	RpiGetANotchSetStat( ANOTCHSEQ *ANotchSeq );
void	RpiSetRegSpatANotchState( ANOTCHSEQ *ANotchSeq );
void	RpiANotchFftReqSet( ANOTCHSEQ *ANotchSeq, FFTANALYZE *FftAna );
void	RpiANotchTrqFilCalc( ANOTCHSEQ *ANotchSeq, AUTONOTCH *ANotch, REGIF_CONFIG_T *RegCfg, PRMDATA *Prm );
void	RpiANotchDisableReqSet( ANOTCHSEQ *ANotchSeq );
void	AutoNotchSetCondition( ANOTCHSEQ *ANotchSeq, BOOL EepromWriteProtect, 
										BOOL BaseEnableSts, BOOL TuneLessSts, BOOL OverTravelSts );


#endif //__AUTO_NOTCH_SET_H 
/***************************************** end of file **********************************************/

#ifndef __FN_EASY_FFT_H /* For reload prevention */
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
#include "UnitPrmStruct.h"



/****************************************************************************************************/
/*																									*/
/*		Structure Definition																		*/
/*																									*/
/****************************************************************************************************/
typedef	struct EASYFFT {
	struct {
		USHORT	EzFftMode;				/* 0x2060 : EasyFFTMode									*/
		USHORT	EzFftState;				/* 0x2061 : EasyFFTConfirm completion									*/
		USHORT	EzFftTrq;				/* 0x2062 : EasyFFTTorque amplitude								*/
		USHORT	EzFftDetf;				/* 0x2410 : EasyFFTDetection frequency								*/
		USHORT	EzFftFil1;				/* 0x2411 : EasyFFTFilter frequency 1							*/
		USHORT	EzFftFil2;				/* 0x2412 : EasyFFTFilter frequency 2							*/
	} OpeReg;
/*--------------------------------------------------------------------------------------------------*/
	struct {
		BOOL	UseHpFil;				/* High-pass filter usage										*/
/*--------------------------------------------------------------------------------------------------*/
		LONG	KnormFrq;				/* Frequency unit conversion gain										*/
		ULONG	Tolerance;				/* Step = 100[Hz]							*/
		USHORT	UnitMaxFreq;			/* Maximum detection frequency for unit conversion[Hz]								*/
		USHORT	RetUnitShift;			/* Shift number for unit inversion										*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	ReTryMethod;			/* Notch detection retry method									*/
		USHORT	ReTryMaxCount;			/* Number of notch detection retries										*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	*TblBufAddr;			/* Table operation command buffer address							*/
		USHORT	*TrcBufAddr;			/* Trace buffer address									*/
		USHORT	Dummy;					/* for Alignment											*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct {
		BOOL	Zoom;					/* Zoom													*/
		BOOL	DrvDir;					/* Operation direction													*/
		BOOL	Drived;					/* Operation completed												*/
		BOOL	SameDir;				/* Same direction operation												*/
		BOOL	AnalyzeState;			/* Abnormality detection													*/
		BOOL	SwpDir;					/* Sweep direction Increase(0)/Decrement(1)									*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	SeqPtr;					/* Sequence pointer										*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	NotchSw;				/* Notch switch											*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	FrqLowerLmt;			/* Lower limit of notch detection/oscillation detection frequency unit[Hz]				*/
		USHORT	FrqSwpMin;				/* Minimum value of sweep frequency unit[Hz]								*/
		USHORT	FrqSwpMax;				/* Maximum value of sweep frequency unit[Hz]								*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	OrgFrqLowerLmt;			/* Backup of lower limit of notch detection/oscillation detection frequency			*/
		USHORT	OrgFrqSwpMin;			/* Backup of minimum value of sweep frequency							*/
		USHORT	OrgFrqSwpMax;			/* Backup of maximum value of sweep frequency							*/
/*--------------------------------------------------------------------------------------------------*/
		ULONG	DetFrq0;				/* Frequency value that is the maximum absolute value(Notch frequency)				*/
		USHORT	SpcPeak0;				/* Speed ​​that is the maximum absolute valueFB/Actual torque value (absolute value)			*/
		USHORT	DetState0;				/* 															*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	PosDrvCount;			/* Number of times of operation in the positive direction											*/
		USHORT	NegDrvCount;			/* Number of times of driving in the wrong direction											*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	Step;					/* STEPStatus													*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	StepReTryCnt;			/* Retry counter:STEP							*/
		USHORT	AvrReTryCnt;			/* Retry counter:Pros and cons of forward/reverse averaging						*/
		USHORT	DetReTryCnt;			/* Retry counter:Pros and cons of detection							*/
/*--------------------------------------------------------------------------------------------------*/
		USHORT	DetCount;				/* Detection result counter											*/
		USHORT	DetState[ 4 ];			/* Detection result(Status)											*/
		ULONG	DetFrq[ 4 ];			/* Detection result(Frequency)											*/
		ULONG	DetAvrFrq[ 2 ];			/* Detection average result(Frequency)										*/
/*--------------------------------------------------------------------------------------------------*/
		ULONG	FinalDetFrq;			/* Final result frequency											*/
/*--------------------------------------------------------------------------------------------------*/
		LONG	KTrqRef;				/* Sweep torque calculation gain										*/
/*--------------------------------------------------------------------------------------------------*/
		LONG	KHpFilter;				/* High-pass filter gain									*/
/*--------------------------------------------------------------------------------------------------*/
		LONG	DeltaFrqRes;			/* Increase of sweep frequency for response processing/Decrement value						*/
		LONG	DeltaFrqRef;			/* Command creation/sweep frequency increment/Decrement value						*/
	} var;
} EASYFFT;



/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
enum EZFFT_SEQ_STS {
	EZFFT_SEQ_INIT		= 0,		/* Initial processing														*/
	EZFFT_SEQ_TRQSET,				/* Sweep torque setting												*/
	EZFFT_SEQ_READY,				/* Operation preparation														*/
	EZFFT_SEQ_DRIVE,				/* In operation													*/
	EZFFT_SEQ_RESULT,				/* Operation completed(Waiting for filter settings)									*/
	EZFFT_SEQ_END,					/* Termination processing														*/
};
/*--------------------------------------------------------------------------------------------------*/
enum EZFFT_LSEQ_STS {
	EZFFT_LSEQ_DRV		= 0,		/* Driving														*/
	EZFFT_LSEQ_WAITSTOP,			/* Wait for operation to stop													*/
	EZFFT_LSEQ_DETFRQ,				/* Frequency analysis in progress													*/
	EZFFT_LSEQ_AVRFRQ,				/* Averaging the forward/reverse difference													*/
	EZFFT_LSEQ_STPEND,				/* Operation completed													*/
	EZFFT_LSEQ_AVRSTP,				/* Averaging multiple trials											*/
	EZFFT_LSEQ_ZOOMREQ,				/* Retry request around frequency analysis results								*/
	EZFFT_LSEQ_ERR,					/* Abnormal termination														*/
	EZFFT_LSEQ_WARN,				/* Finishing the warning												*/
	EZFFT_LSEQ_DONE,				/* Normal end														*/
};
/*--------------------------------------------------------------------------------------------------*/
enum EZFFT_CMP_STS {
	EZFFT_STS_END		= 0,		/* Complete															*/
	EZFFT_STS_RUN,					/* Not completed														*/
	EZFFT_STS_DETNG,				/* Peak value cannot be detected												*/
	EZFFT_STS_ERR,					/* Abnormal termination														*/
};
/*--------------------------------------------------------------------------------------------------*/
#define		EZFFT_SIZ_TRACE			1024	/* Trace size										*/
/*--------------------------------------------------------------------------------------------------*/
#define		EZFFT_DRV_FORWARD		0		/* Table operation direction: +									*/
#define		EZFFT_DRV_REVERSE		1		/* Table operation direction: -									*/
/*--------------------------------------------------------------------------------------------------*/
#define		EZFFT_ANASTS_OK			0		/* Frequency analysis resultOK										*/
#define		EZFFT_ANASTS_NG			1		/* Frequency analysis resultNG										*/
/*--------------------------------------------------------------------------------------------------*/
#define		EZFFT_STEP_1			0		/* Frequency analysisstep1										*/
#define		EZFFT_STEP_2			1		/* Frequency analysisstep2										*/
/*--------------------------------------------------------------------------------------------------*/
#define		EZFFT_ANAOBJ_REF		0		/* Frequency analysis target: Command									*/
#define		EZFFT_ANAOBJ_RES		1		/* Frequency analysis target: Response									*/
/*--------------------------------------------------------------------------------------------------*/
#define		EZFFT_FRQ_L2H			0		/* Torque sweep direction: Low⇒High								*/
#define		EZFFT_FRQ_H2L			1		/* Torque sweep direction: High⇒Low								*/
/*--------------------------------------------------------------------------------------------------*/
#define		EZFFT_RETURN_UNIT		125		/* Constant for unit inversion										*/
#define		EZFFT_DEVLVL_DIR		250		/* Variation tolerance during forward/reverse operation [0.1%]					*/
/*--------------------------------------------------------------------------------------------------*/
#define		EZFFT_CLOSE_MANUAL		4		/* Position control Number of retries 0times 1Only operates(One side operation)		*/
#define		EZFFT_ONESTEP_RETRY_0	5		/* No retry, retry count around frequency analysis result 0times	*/
#define		EZFFT_ONESTEP_RETRY_1	6		/* No retry, retry count around frequency analysis result 1times	*/
#define		EZFFT_OPEN_MANUAL		7		/* Torque control Number of retries 0times 1Only operates(One side operation)		*/
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
void		EasyFftInitialize( EASYFFT *EasyFFT, USHORT *TableBuf, USHORT *TraceBuf, UNI_PRM_STR *prm_hndl_ptr );
PRM_RSLT	RpiFunSetEasyFftTrqLevel( EASYFFT *EasyFFT, USHORT Lvl );



/****************************************************************************************************/
#define __FN_EASY_FFT_H
#endif /* __FN_EASY_FFT_H */

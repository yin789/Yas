#ifndef __REM_VIB_FREQ_H_
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		RemVibFreq.c : Residual vibration frequency calculation process definition													*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2011.02.01	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/



/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Data type definition for residual vibration frequency calculation															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct REMVIBFREQ {					/* Parameter & variable definition for residual vibration frequency monitor			*/
	struct {								/* Parameter definition for residual vibration frequency monitor					*/
		LONG	RemVibDetWidth;					/* Residual vibration frequency detection width			[pulse]				*/
		LONG	RemVibFil;						/* Position deviation filter frequency for residual vibration frequency detection[Hz]	*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {								/* Variable data definition for residual vibration frequency monitor					*/
		SHORT	State;							/* Status status									*/
		SHORT	Dir;							/* Current actual movement direction 								*/
		SHORT	RevCnt;							/* Number of inversions											*/
		SHORT	Dummy;							/* For Alignment									*/
		ULONG	TimeOutCtr;						/* Timeout counter 							*/
		ULONG	PeakCntTimer;					/* Residual vibration time measurement timer							*/
		ULONG	PeakCnt;						/* Residual vibration time counter	Current						*/
		ULONG	LastPeakCnt;					/* Residual vibration time counter	1times before						*/
		ULONG	LLastpeakCnt;					/* Residual vibration time counter	2times before (1pieces before)	*/
		LONG	PeakValue;						/* Overshoot amount			[pulse]				*/
		ULONG	FreqCnt;						/* Residual vibration frequency counter							*/
		LONG	LastPosErr;						/* Previous position deviation					[pulse]				*/
		LONG	PosErrFilOut;					/* Position deviation after filter processing		[pulse]				*/
		LONG	PosErrFilOut2;					/* Position deviation after filter processing		[pulse]				*/
		ULONG	MeanDeno;						/* Denominator of weighted averaging 								*/
	} var;
	/*----------------------------------------------------------------------------------------------*/
	struct {								/* Residual vibration frequency monitor data definition <S0DA>				*/
		LONG	UnRvibFrequency;				/* Un107 : Residual vibration frequency		[0.1Hz]				*/
	} mon;
} REMVIBFREQ;



/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Definition for residual vibration detection																			*/
/*--------------------------------------------------------------------------------------------------*/
#define REMVIB_CYCLEMS		(KPI_SCCYCLEMS)			/* Minimum output frequency counter value(1Hz)				*/
#define REMVIB_MINOUT		(1000/REMVIB_CYCLEMS)	/* Minimum output frequency counter value(1Hz)				*/
#define REMVIB_MINDET		(REMVIB_MINOUT*12/10)	/* Minimum detection frequency counter value(1.2Hz)				*/
#define REMVIB_MINDETWIDTH	3						/* Minimum detection width					[pulse]			*/
#define REMVIB_FILFREQ		5000					/* Residual vibration detection filter frequency	[0.1Hz]			*/
#define REMVIB_WEIGHT_NUME	12						/* Averaging weighting numerator							*/
#define REMVIB_WEIGHT_DENO	10						/* Averaging weighting denominator 							*/
#define REMVIB_DENO_OFFSET	1000					/* Averaging weight accumulation unit 100100minutes1		*/
#define REMVIB_TIMEOUT		(REMVIB_MINDET/REMVIB_CYCLEMS)
													/* Timeout time				[ms]			*/
/*--------------------------------------------------------------------------------------------------*/
/*	Residual vibration detection sequence definition																		*/
/*--------------------------------------------------------------------------------------------------*/
enum REMVIBSEQ {
	RVM_INIT		= 0,					/* Initialize												*/
	RVM_COIN,								/* Waiting for positioning completion										*/
	RVM_PEAKCHK_F,							/* Peak detection(Before vibration frequency detection)							*/
	RVM_PEAKCHK_L,							/* Peak detection(Start vibration frequency detection)						*/
	RVM_COMP,								/* Vibration frequency calculation										*/
};



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void	RemVibChkCondition( REMVIBFREQ *RemVibFreq, BOOL PosCtrlMode );
void	RemVibCalculateFreq( REMVIBFREQ *RemVibFreq, LONG CtrlPosErr, BOOL CoinSts, BOOL RefzSts );

/****************************************************************************************************/
/*		API																							*/
/****************************************************************************************************/
LONG	RemVibGetRemVibFreq( REMVIBFREQ *RemVibFreq );
void	RemVibCalculateMonitorPrm( REMVIBFREQ *RemVibFreq, EGEAR *Egear,
    	                           USHORT remdetw, LONG CoinLevel, LONG ScanTime );
void	RemVibRsetRemVibrationFreq( REMVIBFREQ *RemVibFreq );

#define __REM_VIB_FREQ_H_
#endif /* __REM_VIB_FREQ_H_ */
/***************************************** end of file **********************************************/

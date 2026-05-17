#ifndef __TUNE_LIBRARY_H 
#define __TUNE_LIBRARY_H
/****************************************************************************
Description	: Tuning Function Library
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
Y.Oka  2010.11.16    created

*****************************************************************************/
#include "Basedef.h"
#include "PrmAccessIf.h"
#include "PnPrmStruct.h"
#include "ResVibCtrl.h"



/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		for Tuning Library																			*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNE_FREQ_SEARCHINIT	0		/* Frequency analysis processing completed normally							*/
#define		TUNE_FREQ_SEARCHEND		1		/* Frequency analysis processing completed normally							*/
#define		TUNE_FREQ_SEARCH		2		/* Frequency analysis processing in progress									*/
#define		TUNE_FREQ_SEARCHERR		-1		/* Frequency analysis processing abnormally completed							*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNE_MDLZETA			1000	/* Model loop gain correction 				[0.1%]		*/
#define		TUNE_MFCKTFF			1000	/* Model torque feedforward compensation	[0.1%]		*/
#define		TUNE_MFCKVFF			1000	/* Model speed feedforward compensation 	[0.1%]		*/
#define		TUNE_MFCKVFF2			900		/* Model speed feedforward compensation 2	[0.1%]		*/
#define		TUNE_DTRQGN				85		/* Estimated disturbance coefficient     = 85[%]							*/
#define		TUNE_DOBJGN				100		/* Inertia correction   = 100[%]						*/
#define		TUNE_DOBGNRATIO			100		/* Observer gain = 100[%]						*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNE_PI_TIKV			636620	/* PIBalanceTi/Kv  636620 = 4 *1000000 / 2π		*/
#define		TUNE_IP_TIKV			318310	/* IPBalanceTi/Kv  318310 = 2 *1000000 / 2π		*/
#define		TUNE_IP_KPTI			250000	/* IPBalanceKp/Ti  ζ=1.0, Kp[1/s] = 0.25 / Ti[s]*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNE_ANOTCH_DECA		10		/* 0.1Hz -> 1Hz							*/
#define		TUNE_ANOTCH_Q_MIN		50		/* Automatic notchQValue lower limit								*/
#define		TUNE_ANOTCH_Q_MAX		300		/* Automatic notchQValue upper limit 3.00							*/
#define		TUNE_ANOTCH_Q_MAX_DET	500		/* Automatic notch for upper limit detectionQValue upper limit 5.00				*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNE_MAX_DLPFIL			4000	/* KvInterlocking disturbance observer low-pass upper limit 400.0Hz		*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNE_RESFRQ_KV			100		/* KvResonant frequency for correction			[0.1Hz]				*/
#define		TUNE_ANTFRQ_KV			100		/* KvAnti-resonant frequency for correction			[0.1Hz]				*/
#define		TUNE_ANTFRQ_NTQ1		100		/* NotchQAnti-resonance frequency for value calculation	[0.1Hz]				*/
#define		TUNE_ANTFRQ_NTQ2		100		/* NotchQAnti-resonance frequency for value calculation	[0.1Hz]				*/
/*--------------------------------------------------------------------------------------------------*/
#define		AVIB_FRQ_MIN			100		/* ALower limit of mold vibration damping detection frequency 100Hz						*/
#define		AVIB_FRQ_MAX			1000	/* AUpper limit of mold vibration damping detection frequency 1000Hz						*/
#define 		AVIBDAMPGN				180		/* AMold vibration damping control adjustment damping gain setting value 150%		*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNE_ANOTCH_MINKV		600		/* Notch filterONMinimumKv		[0.1Hz]				*/
/*--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------*/
/*		for Auto Notch																				*/
/*--------------------------------------------------------------------------------------------------*/
#define		OPTUNE_STS_NOTCH_INIT		0x00	/* Initial notch setting								*/
#define		OPTUNE_STS_NOTCH1_OK		0x01	/* 1Stage notch setting completed normally						*/
#define		OPTUNE_STS_NOTCH2_OK		0x02	/* 2Stage notch setting completed normally						*/
#define		OPTUNE_STS_NOTCHQ1_ERR		0x04	/* 1Row notchQValue error							*/
#define		OPTUNE_STS_NOTCHQ2_ERR		0x08	/* 2Row notchQValue error							*/
/*--------------------------------------------------------------------------------------------------*/
#define		OPTUNE_STS_NT_NODETECT		0		/* Notch completion status: Vibration not detected				*/
#define		OPTUNE_STS_NT_DETECT		1		/* Notch completion status: Vibration detected				*/
#define		OPTUNE_STS_NT_NOENABLE		2		/* Notch completion status: Automatic setting not possible			*/
#define		OPTUNE_STS_NT_DET_ERR		3		/* Notch completion status: Detection error				*/
/*--------------------------------------------------------------------------------------------------*/
#define		OPTUNE_STS_NT1_NODETECT		0		/* Notch completion status:1Stage not executed			*/
#define		OPTUNE_STS_NT1_COMP			1		/* Notch completion status:1Stage setting completed			*/
#define		OPTUNE_STS_NT1_NG			2		/* Notch completion status:1Stage setting failure			*/
/*--------------------------------------------------------------------------------------------------*/
#define		OPTUNE_STS_NT2_NODETECT		0		/* Notch completion status:2Stage not executed			*/
#define		OPTUNE_STS_NT2_COMP			1		/* Notch completion status:2Stage setting completed			*/
#define		OPTUNE_STS_NT2_NG			2		/* Notch completion status:2Stage setting failure			*/
/*--------------------------------------------------------------------------------------------------*/
#define		OPTUNE_PSTS_NODETECT		0		/* Notch completion status: Previous notch not performed		*/
#define		OPTUNE_PSTS_NT1_COMP		1		/* Notch completion status: Previous notch1stage		*/
#define		OPTUNE_PSTS_NT2_COMP		2		/* Notch completion status: Previous notch2stage		*/


/*--------------------------------------------------------------------------------------------------*/
/*		for Online Frequency Analysis																*/
/*--------------------------------------------------------------------------------------------------*/
#define	PEAK_DETECT_NUM		5					/* Spectrum peak frequency detection number				*/
#define	WORK_AREA_NUM		200					/* Number of work areas [(Fmax-Fmin)/Sno]	*/
#define	REGULAR_SFT			16384				/* Normalization amount: When calculating spectrumOverFlowCountermeasures	*/
#define 	MINIMAM_FREQ		240					/* Detection frequency lower limit[Hz]							*/
#define	MINIMAM_SPECTRUM	3000				/* Spectrum peak lower limit value						*/
#define	MINIMAM_SPECTRUM2	30000				/* Spectrum peak lower limit value for adjustment-less automatic notch	*/
#define	ONFFT_OK_LMT		20					/* Frequency analysis threshold [x]							*/
#define	ONFFT_OK_LMT2		50					/* Adjustment-less automatic notch frequency analysis threshold[x]			*/
//#define	SAMPLETIME		4000					/* 1 / (Data sample period(sec)×2)[Hz]			*/
#define	SAMPLETIME(SBScanTimeUs) (500000/SBScanTimeUs)	/* 1/(Data sample period(sec)×2)[Hz]	*/




/****************************************************************************************************/
/*																									*/
/*		Struct Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Constant definition for automatic notch setting																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct AUTONOTCH_PRM {
	USHORT			KvrateTf;		/* AutomaticTfCalculation cut-offKv					[x]		*/
	USHORT			KvrateTfonNt;	/* NotchONTfCalculation cut-offKv		[x]		*/
	USHORT			KvrateQ;		/* NotchQCutoff for value calculationKv				[x]		*/
	USHORT			J1Tf;			/* AutomaticTfCalculation of motor side moment of inertia ratio				[%]			*/
	USHORT			J1TfonNt;		/* NotchONTfCalculation of motor side moment of inertia ratio	[%]			*/
	USHORT			J1Q;			/* NotchONtimeQMotor side moment of inertia ratio for value calculation	[%]			*/
	USHORT			TfMin;			/* AutomaticTfCalculation minimum setting value								[0.01ms]	*/
	USHORT			NotchQMin;		/* NotchQValue minimum setting value								[-]			*/
} AUTONOTCH_PRM;



/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for automatic notch setting																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct AUTONOTCH_DATA {
	struct {						/* Constant data definition for vibration frequency analysis								*/
		USHORT			freq;			/* Auto notch frequency											*/
		USHORT			KvGain;			/* Tf, NotchQGain for value calculation								*/
		USHORT			Jrate;			/* Tf, NotchQLoad inertia moment for value calculation					*/
	} conf;
	struct {						/* Variable data definition for vibration frequency analysis								*/
		USHORT			NotchSw;		/* Automatic notch setting selection										*/
		USHORT			TrqFil;			/* Torque filter setting value										*/
		USHORT			NotchFil1;		/* 1Stage notch filter frequency setting value							*/
		USHORT			NotchFil2;		/* 2Stage notch filter frequency setting value							*/
		USHORT			NotchQ1;		/* 1Stage notch filterQValue setting value								*/
		USHORT			NotchQ2;		/* 2Stage notch filterQValue setting value								*/
	} var;
} AUTONOTCH_DATA;


/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for automatic notch setting																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct AUTONOTCH {
	AUTONOTCH_PRM	ANotchPrm;			/* Constants for automatic notch setting										*/
	AUTONOTCH_DATA	ANotchData;			/* Variables for automatic notch setting										*/
} AUTONOTCH;



/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for online vibration monitor(For registers)												*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct FRQANA {
	USHORT		Freq;					/* Analysis frequency												*/
	USHORT		Amp;					/* Analysis amplitude													*/
} FRQANA;


/*--------------------------------------------------------------------------------------------------*/
/*		Definition of structure for storing frequency analysis results(For registers)													*/
/*--------------------------------------------------------------------------------------------------*/
/* Auto tuning area information:2400H - 25FFH */
typedef	struct FFTANA_RSLT {
	FRQANA		VibmRf[3];				/* 0x2400 : Online vibration monitor resonance frequency1					*/
										/* 0x2401 : Online vibration monitor resonance frequency1Peak value			*/
										/* 0x2402 : Online vibration monitor resonance frequency2					*/
										/* 0x2403 : Online vibration monitor resonance frequency2Peak value			*/
										/* 0x2404 : Online vibration monitor resonance frequency3					*/
										/* 0x2405 : Online vibration monitor resonance frequency3Peak value			*/
	FRQANA		VibmAf;					/* 0x2406 : Online vibration monitor antiresonance frequency1				*/
										/* 0x2407 : Online vibration monitor antiresonance frequency1Peak value		*/
} FFTANA_RSLT;


/*--------------------------------------------------------------------------------------------------*/
/*		Vibration frequency analysis Constant structure definition																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct FFTANA_PRM {				/* Parameter definition for vibration frequency analysis						*/
	LONG		Spectrumval;				/* Vibration analysis pass/fail judgment spectrum correction value					*/
	LONG		Spectrumval2;				/* Spectrum correction value for pass/fail judgment of vibration analysis without adjustment			*/
	BOOL		ActiveFFtOn;				/* Forced frequency analysis request flag								*/
	BOOL		TuneLessSts;				/* Adjustment-less auto enable/disable flag						*/
	LONG		SumpleNum;					/* Number of data samples										*/
	LONG		AnaMaxFreq;					/* Analysis maximum frequency										*/
	LONG		AnaMinFreq;					/* Analysis minimum frequency										*/

	LONG		SampleTime;					/* 1 / (Data sample period(sec)×2) [Hz]				*/
} FFTANA_PRM;


/*--------------------------------------------------------------------------------------------------*/
/*		Vibration frequency analysis  Variable definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct FFTANALYZE {				/* Parameter & variable definition for vibration frequency analysis					*/
	FFTANA_RSLT	FftAnaRslt;					/* For storing vibration frequency analysis results								*/
	FFTANA_PRM	FftAnaPrm;					/* Parameters for vibration frequency analysis							*/
	/*----------------------------------------------------------------------------------------------*/
	struct {								/* Variable data definition for vibration frequency analysis						*/
		ULONG	swork_max[PEAK_DETECT_NUM];		/* Spectrum peak data							*/
		ULONG	freq_max[PEAK_DETECT_NUM];		/* Frequency peak data								*/
		ULONG	swork_min;						/* Spectrum lower limit data							*/
		ULONG	freq_min;						/* Frequency lower limit data									*/
		LONG	Reals[WORK_AREA_NUM];			/* RealPartFor calculation work								*/
		LONG	Images[WORK_AREA_NUM];			/* ImagePartFor calculation work							*/
	} var;
} FFTANALYZE;


/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		for Tuning Library																			*/
/*--------------------------------------------------------------------------------------------------*/
PRM_RSLT	TunePrmWriteRam( REG_ACC_T *AccessPrm, CPRM_ATTR_T *PrmAttr, LONG Data, LONG Size );
USHORT	TunePrmLimitUnsigned( ULONG Input, CPRM_ATTR_T *PrmAttr );
SHORT	TunePrmLimitSigned( LONG Input, CPRM_ATTR_T *PrmAttr );
USHORT	TuneKvFuncTi( USHORT Kv, BOOL IPSpdControl );
USHORT	TuneKvFuncKp( USHORT Kv, USHORT Ti, BOOL IPSpdControl );
USHORT	TuneKvFuncDobsgn( USHORT Kv );
SHORT	TuneKvFuncDobLpFil( USHORT Kv, USHORT Dobsgn );
SHORT	TuneNotchQ( AUTONOTCH *ANotch, REGIF_CONFIG_T *RegCfg, USHORT kv, USHORT jrate, UCHAR sel, PRMDATA *Prm );
void	TuneGetNotchQSetting( AUTONOTCH *ANotch, USHORT *NotchQ1, USHORT *NotchQ2, USHORT *TrqFil );
USHORT	TuneNotchModKv( USHORT kv, USHORT jrate, USHORT j1rate );
SHORT	TuneNotchQset( AUTONOTCH_PRM *ANotchPrm, USHORT kv, USHORT jrate, USHORT Nfreq, USHORT Antfreq, USHORT *autoQ );
USHORT	TuneNotchTfcalc( AUTONOTCH_PRM *ANotchPrm, USHORT kv, USHORT jrate, USHORT Nfreq, USHORT autoQ_Q1 );
USHORT	TuneOnePrmTfcalc( AUTONOTCH_PRM *ANotchPrm, USHORT kv, USHORT jrate );
SHORT	TuneNotchQset2( USHORT freq, USHORT Nfreq, USHORT *autoQ );
void	TuneFilGainCalbyFilType( AUTONOTCH *ANotch, UCHAR Type );
//void	TuneMfcActive( PRMDATA *Prm );
//void	TuneMfcActiveStMode( PRMDATA *Prm );
//void	TuneDobsActive( PRMDATA *Prm );
//SHORT	TuneATypeResSet( USHORT freq, USHORT auton_sw, PRMDATA *Prm );
void	TuneMfcActive( PRMDATA *Prm, REGIF_CONFIG_T *RegCfg );										/*<S09E>*/
void	TuneMfcActiveStMode( PRMDATA *Prm, REGIF_CONFIG_T *RegCfg );								/*<S09E*/
void	TuneDobsActive( PRMDATA *Prm, REGIF_CONFIG_T *RegCfg );									/*<S09E>*/
SHORT	TuneATypeResSet( USHORT freq, USHORT auton_sw, PRMDATA *Prm, REGIF_CONFIG_T *RegCfg );	/*<S09E>*/
SHORT	TuneNotchFilterSetting( AUTONOTCH *ANotch, PRMDATA *Prm, REGIF_CONFIG_T *RegCfg );
SHORT	TuneVibrationTrace( DETVIB *DetVib, FFTANALYZE *FftAna, UCHAR TraceRequest, SHORT TrgLvl, BOOL TuneLessSts );
USHORT	TuneMfcKpmCalcAntFreq( USHORT AntFreq );
void	TuneSetNotchFilSettingData( AUTONOTCH *ANotch, USHORT freq, USHORT KvGain, USHORT Jrate );


/*--------------------------------------------------------------------------------------------------*/
/*		for Online Frequency Analysis																*/
/*--------------------------------------------------------------------------------------------------*/
void	FftCalculateInitPrm( FFTANALYZE *FftAna, LONG SampleTimeUs );
SHORT	FftAnalyzeExecute( FFTANALYZE *FftAna, SHORT *TraceData );
void	FftSetAnaCondition( FFTANALYZE *FftAna, LONG SumpleNum, LONG AnaMaxFreq, LONG AnaMinFreq, BOOL TuneLessSts );
USHORT	FftGetAnaRsltVibmRf( FFTANALYZE *FftAna );
void	FftSetForceAnaReq( FFTANALYZE *FftAna, BOOL Setting );
BOOL	FftGetForceAnaReq( FFTANALYZE *FftAna );



/****************************************************************************************************/
/*																									*/
/*		Extern Definition																			*/
/*																									*/
/****************************************************************************************************/
extern	const	AUTONOTCH_PRM	AutoNotchTbl[ 4 ];



#endif //__TUNE_LIBRARY_H 
/***************************************** end of file **********************************************/

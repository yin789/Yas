/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Parameter calculation header file for no adjustment													*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.09.28	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#ifndef	_TUNELESS_PRM_CALC_H
#define _TUNELESS_PRM_CALC_H


#include "BaseControls.h"
#include "BaseControlPrmCalc.h"
#include "ResVibCtrl.h"
#include "AutoNotchSet.h"



/****************************************************************************************************/
/*																									*/
/*		STRUCT DEFINITION																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Adjustment-less level definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct TUNELESS_GAIN_TBL {
	USHORT		Kv;				/* Speed loop gain									[0.1Hz]			*/
	USHORT		F1rate;			/* Observer gain ratio								[%]				*/
} TUNELESS_GAIN_TBL;
/*--------------------------------------------------------------------------------------------------*/
typedef struct TUNELESS_JRATE_TBL {
	USHORT		Jrate;			/* Load moment of inertia/Mass ratio						[%]				*/
	USHORT		dummy;			/* for alignment													*/
} TUNELESS_JRATE_TBL;
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		DEFINES																						*/
/*																									*/
/****************************************************************************************************/
#define		PRM_SETTING_OK			0			/* For parameter setting value determination(Normal)						*/
#define		PRM_SETTING_NG			-1			/* For parameter setting value determination(Abnormal)						*/
#define		PRMERR_TUNELESS_LVL		0x01		/* Pn170 Setting error									*/
#define		PRMERR_TUNELESS_TYPE	0x02		/* Pn14F Setting error									*/


/****************************************************************************************************/
/*		for Tuning Less Control																		*/
/****************************************************************************************************/
/* For adjustment-less gain calculation */
#define		TULESCYCLE			KPI_SACYCLENS		/* Adjustment-less calculation cycle time(ScanA)		[ns]*/
#define		TULESCYCLEUS		((TULESCYCLE + 500)/1000)	/* Adjustment-less calculation cycle time		[us]*/
/*--------------------------------------------------------------------------------------------------*/
#define		TLSPDDET13BIT		(KPI_SACOUNTMS*600000/8192)			/* 13bitEncoder speed detection resolution			*/
#define		TLSPDDET200UM		(KPI_SACOUNTMS*10000/256*200/1000)	/* Scale pitch200umSpeed detection resolution		*/
/*--------------------------------------------------------------------------------------------------*/
#define		TLESSGLVLMAX		10				/* Maximum value of gain level without adjustment						*/
#define		TLESSJLVLMAX		5				/* Maximum inertia level without adjustment					*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNELESSJRAT		30				/* No adjustmentJLTarget stability range				[x]		*/
#define		DISOBSFSTD			6000			/* No adjustmentRobustCompensator frequency1:(Reference)[0.1Hz]		*/
#define		TUNELESS_STIFFSTD	100				/* No adjustmentRobustSexual strength	100%(Reference)					*/
/*--------------------------------------------------------------------------------------------------*/
//#define	DISOBSF11			TuneLessGain	/* No adjustmentRobustCompensator frequency1: Robustness specification(Noise countermeasures)[0.1Hz]	*/
#define		DISOBSF12			3000			/* No adjustmentRobustCompensator frequency1: Robustness specification(Noise countermeasures)[0.1Hz]	*/
#define		DISOBSF21			8000			/* No adjustmentRobustCompensator frequency2: For noise cut(Standard)[0.1Hz]		*/
#define		DISOBSF22			8000			/* No adjustmentRobustCompensator frequency2: For noise cut(Noise countermeasures)[0.1Hz]	*/
#define		DISOBSF1MIN			1500			/* No adjustmentRobustCompensator frequency1Minimum value[0.1Hz]			*/
/*--------------------------------------------------------------------------------------------------*/
#define		VOBSLPFGNHZ			(DISOBSFSTD/TUNELESSJRAT)	/* Adjustment-less speed observerLPFCalculated frequency[0.1Hz]		*/
/*--------------------------------------------------------------------------------------------------*/
#define		DISOBSKJ			100				/* Adjustmentless disturbance observer coefficient gain		[%]			*/
#define		DISOBSKD			99				/* Adjustmentless disturbance observer coefficient			[%]			*/
#define		VOBSKS				60				/* Adjustment-less speed observer gain			[Hz]		*/
#define		VOBSKJ				100				/* Adjustment-less speed observer coefficient gain		[%]			*/
#define		VPHASEF2			10000			/* Phase compensation filter frequency for no adjustment2	[0.1Hz]		*/
#define		VPHASEFULIM			1				/* Lower limit of phase compensation filter frequency for no adjustment[0.1Hz]	*/
/*--------------------------------------------------------------------------------------------------*/
#define		EHVOBSTFULIM		2				/* Adjustment-less phase compensation speedOBS LpFilTime constant lower limit value[10us]	*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNE_IP_KPTI		250000			/* IPBalanceKp/Ti ζ=1.0, Kp[1/s] = 0.25 / Ti[s]	*/
#define		TUNLESS_TREFFIL		10				/* Torque command filter time constant after torque compensation[0.01ms]		*/
/*--------------------------------------------------------------------------------------------------*/
//#define	TUNELESS_STIFF1		TunLessStiff	/* Adjustment-less robustness strength	(Standard)					*/
#define		TUNELESS_STIFF2		50				/* Adjustment-less robustness strength2	50%(Stability emphasis)			*/
#define		TUNELESS_STIFF3		30				/* Adjustment-less robustness strength3	30%(Stability emphasis)			*/
/*--------------------------------------------------------------------------------------------------*/
#define		VIBOBS_KS			800				/* Vibration detection observerKsGain			[0.1Hz]		*/
/*--------------------------------------------------------------------------------------------------*/
//#define	DAT_GAINLEVEL_MAX	4				/* Maximum value of gain level without adjustment						*//* <S1C7> */
#define		DAT_GAINLEVEL_MAX	7				/* Maximum value of gain level without adjustment						*//* <S1C7> */
#define		DAT_GAINLEVEL_MIN	0				/* Minimum value of gain level without adjustment						*/
#define		DAT_JRATLEVEL_MAX	2				/* Maximum inertia level without adjustment					*/
#define		DAT_JRATLEVEL_MIN	0				/* Minimum inertia level without adjustment					*/
/*--------------------------------------------------------------------------------------------------*/
#define		TLEX_DISOBSFRQ		4000			/* New adjustmentless disturbance observer frequency					*/
#define		TLEX_DISOBSKD		99				/* New adjustmentless disturbance observer coefficient						*/
#define		TLEX_VOBSLPFGAIN	200				/* New adjustmentless phase compensation speed observer gain			*//* <S1C7> */
//#define	TLEX_VOBSLPFGAIN	150				/* New adjustmentless phase compensation speed observer gain			*//* <S1C7> */
#define		TLEX_VPHASEF1		300				/* New adjustmentless phase compensation filter frequency1				*/
#define		TLEX_VPHASEF2		600				/* New adjustmentless phase compensation filter frequency2				*/
#define		TLEX_VOBSKS			120				/* New adjustmentless phase compensation speed observer gain			*//* <S1C7> */
//#define	TLEX_VOBSKS			160				/* New adjustmentless phase compensation speed observer gain			*//* <S1C7> */
#define		TLEX_TRQLPFIL		8				/* New adjustmentless torque filter time constant					*/
#define		TLEX_BNDPASSFILZN	10000			/* Newly adjusted bandpass filter attenuation coefficient					*/
#define		TLEX_RESSUPFIL		2000			/* Without new adjustment Mechanical resonance suppression filter time constant			*/

/* For adjustmentless mode setting */
enum TUNELESS_MODE_SELECT{
	TLMODE_NORMAL		= (LONG)0x00L,			/* TuningLessMode NORMAL							*/
	TLMODE_CORELESS		= (LONG)0x01L,			/* TuningLessMode for CoreLess Linear				*/
	TLMODE_STABLE_L		= (LONG)0x10L,			/* TuningLessMode Stable Form for Linear			*/
	TLMODE_STABLE_R		= (LONG)0x20L,			/* TuningLessMode Stable Form for Rotary			*/
	TLMODE_SILENT		= (LONG)0x40L,			/* TuningLessMode Silent Form						*/
	TLMODE_SPDDETCHNG	= (LONG)0x100L,			/* TuningLessMode Speed Detect Change Request		*/
};


/****************************************************************************************************/
/*		for Disturb Observer																		*/
/****************************************************************************************************/
/* Disturbance observer calculation cycle time(Same period as velocity loop)[ns] */
#define	DOBSCYCLE		KPI_SACYCLENS


/****************************************************************************************************/
/*		for Eh Velocity Observer																	*/
/****************************************************************************************************/
/* Phase compensation speed observer calculation cycle time(Same period as velocity loop)[us] */
#define	EHSOBSCYCLE		KPI_SACYCLEUS


/****************************************************************************************************/
/*																									*/
/*		STRUCTS																						*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		Tuning Less Structures for API function Parameters											*/
/****************************************************************************************************/
typedef struct _TUNELESS_CFGPRM {
	USHORT		gnmode;					/* Pn10B : Gain-related application switch							*/
	USHORT		mfctype;				/* Pn14F : Control-related switches									*/
	USHORT		avibsw;					/* Pn160 : Damping control related switches								*/
	USHORT		DatLevel;				/* Pn170 : Dynamic auto tuning related switches		*/
	USHORT		tfuncsw;				/* Pn408 : Torque related function switch							*/
	USHORT		flg_wf;					/* PnF1E : Flag											*/
} TUNELESS_CFGPRM;

typedef struct _TUNELESS_SETPRM {
	USHORT		DatLevel;				/* Pn170 : Dynamic auto tuning related switches		*/
	USHORT		trqfil11;				/* Pn401 : 1Stage1Torque command filter time constant					*/
	USHORT		ampw;					/* PnE12 : Servopack capacity									*/
	USHORT		ignq;					/* PnE21 : Current loop gain(Q)								*/
	USHORT		kiq;					/* PnE23 : Current loop integral time constant(Q)							*/
	USHORT		fbfil2;					/* PnEA8 : No.2SpeedF/BFilter time constant							*/
	USHORT		TuneLessGain;			/* PnEDA : Adjustmentless disturbance observer gain						*/
	USHORT		TuneLessStiff;			/* PnEDB : Adjustment-less inertia correction gain						*/
	USHORT		TuneLessTrqLpf;			/* PnEDC : Adjustmentless disturbance torque low-pass filter time constant			*/
	USHORT		TuneLessVfbLpf;			/* PnEDD : Adjustment-less speedFBLow-pass filter time constant				*/
	MENCPRM		*MencP;					/* PnF00 - PnF1E : Encoder parameters						*/
	DETVIB		*DetVib;				/* Data definition for oscillation monitoring										*/
	ANOTCHSEQ 	*ANotchSeq;				/* Data definition for automatic notch									*/
} TUNELESS_SETPRM;


/****************************************************************************************************/
/*		Disturb Observer Structures for API function Parameters										*/
/****************************************************************************************************/
typedef struct _DOBS_CFGPRM {
	USHORT		jrate;					/* Pn103 : Moment of inertia ratio									*/
	USHORT		loophz;					/* Pn100 : Speed loop gain									*/
	USHORT		loophz2;				/* Pn104 : No.2Speed loop gain								*/
	USHORT		dobgn;					/* Pn121 : Friction compensation gain									*/
	USHORT		dobgn2;					/* Pn122 : No.2Friction compensation gain								*/
	USHORT		dtrqgn;					/* Pn123 : Friction compensation coefficient										*/
	SHORT		dlpfil;					/* Pn124 : Friction compensation frequency correction								*/
	USHORT		dobjgn;					/* Pn125 : Friction compensation gain correction								*/
} DOBS_CFGPRM;


/****************************************************************************************************/
/*		Eh Velocity Observer Structures for API function Parameters									*/
/****************************************************************************************************/
typedef struct _EHVOBS_CFGPRM {
	USHORT		jrate;					/* Pn103 : Moment of inertia ratio									*/
	USHORT		evobgn;					/* Pn127 : Speed observer gain								*/
	USHORT		evobjgn;				/* Pn128 : Speed observer position compensation gain						*/
	USHORT		trqfil11;				/* Pn401 : 1Stage1Torque command filter time constant					*/
} EHVOBS_CFGPRM;



/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		for Tuning Less Control																		*/
/****************************************************************************************************/
//VOID	TuneLessCalculatePrm( BASE_LOOPCTRLS *BaseLoops, BPRMDAT *Bprm, TUNELESS_SETPRM *Prm );		  /* <S01D> */
BOOL	TuneLessCalculatePrm( BASE_LOOPCTRLS *BaseLoops, BPRMDAT *Bprm,								  /* <S066> */
								TUNELESS_SETPRM *Prm, BASE_SPDLOOP_PRM *SpdCtrlPrm );
LONG	TuneLessCalculateInitPrm( BASE_CTRL_HNDL *BaseCtrlData, BPRMDAT *Bprm,
								  MENCV *MencV, ASICS *SvAsicRegs, TUNELESS_CFGPRM *CfgPrm );
BOOL	TuneLessSetTrqFil3( BOOL TuningLess, BOOL TuningLessEx, ULONG flg_wf,
							BOOL TuningLessLowGainMotor );

/****************************************************************************************************/
/*		for Disturb Observer																		*/
/****************************************************************************************************/
LONG	DobsCalculatePrmSW( BOOL *DobsAct, BOOL TuningLess, USHORT DobsSel );
VOID	DobsCalculatePrm( DOBS_PRM *DobsPrm, ASICS *SvAsicRegs,
						  BPRMDAT *Bprm, DOBS_CFGPRM *CfgPrm, LONG GselNo );

/****************************************************************************************************/
/*		for Eh Velocity Observer																	*/
/****************************************************************************************************/
BOOL	EhVobsCalculatePrmSW( BOOL *EhVobsUse, BOOL TuningLess, USHORT EhVobsSel );					  /* <S066> */
BOOL	EhVobsCalculatePrm( EHVOBS_CTRL *EhVobsCtrl, BPRMDAT *Bprm,									  /* <S066> */
							EHVOBS_CFGPRM *CfgPrm, TRANSDUCE_CTRL *TransduceCtrl );


/****************************************************************************************************/
/*																									*/
/*		EXTERN																						*/
/*																									*/
/****************************************************************************************************/
extern	void	RpiRegSetTuneLessLevel( ULONG Gain, TUNELESS_CTRL *TuneLessCtrl );




/***************************************** end of file **********************************************/
#endif /* _TUNELESS_PRM_CALC_H */

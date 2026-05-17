#ifndef	_RES_VIB_CTRL_H /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ResVibCtrl.h : Header file for vibration detection/vibration suppression control											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.11.11	Y.Oka	for INGRAM												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "JL086ServoIpReg.h"


/****************************************************************************************************/
/*																									*/
/*		Constant definition																					*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		for AType Restrain Vibration																*/
/*--------------------------------------------------------------------------------------------------*/
//#define	ARESCYCLE			KPI_SBCYCLENS		/* AMold vibration suppression control calculation cycle time(ScanB) [ns]		*/
//#define	C10POW7				10000000			/* 10^7												*/
//#define	C10POW9				1000000000			/* 10^9												*/
//#define	C2PAIE7				62831853			/* (2*PAI) * 10^7									*/

/*--------------------------------------------------------------------------------------------------*/
/*		for Detect Vibration																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	DETECT_VIB_NOT_DETECT	0x0000			/* Vibration detection warning output							*/
#define	DETECT_VIB_WARNING		0x0001			/* Vibration detection warning output							*/
#define	DETECT_VIB_ALARM		0x0010			/* Vibration detection alarm output								*/

/*--------------------------------------------------------------------------------------------------*/
/*		for Detect Vibration Trace																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	DETVIBTRC_BUFMAX		200				/* Maximum number of trace buffers							*/
#define DETVIBTRC_NOCMD			0x00			/* No trace command									*/
#define	DETVIBTRC_TRCEXE		0x10			/* Trace execution command									*/
#define	DETVIBTRC_TRCEND		0x1F			/* Trace execution end response								*/

/*--------------------------------------------------------------------------------------------------*/
/*		for Vibration Detect Observer																*/
/*--------------------------------------------------------------------------------------------------*/
//#define	DETVIBCYCLENS	KPI_SBCYCLENS	/* Vibration detection calculation cycle time(ScanB)[ns]					*/
//#define	DETVIBCYCLEUS	KPI_SBCYCLEUS	/* Vibration detection calculation cycle time(ScanB)[us]					*/
#define	VIBOBSGN		800				/* Vibration detection observer gain(80Hz)							*/
#define	DETVIBLP		3183			/* Low-pass filter for one-parameter vibration detection(3183us = 50Hz)		*/
#define	DETVIBHP		320				/* High-pass filter for oscillation detection:0.32[ms]=500[Hz]				*/


/****************************************************************************************************/
/*																									*/
/*		Variables Definition																		*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Speed observer variable definition for vibration detection															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct DETVIBOBS {					/* Speed observer parameter & variable definition for vibration detection			*/
	struct	{								/* Speed observer parameter definition for vibration detection				*/
		KSGAIN	Ks;								/* Vibration detection observer gain						*/
		LONG	Kj;								/* Observer inertia gain for vibration detection				*/
		LONG	Klpf;							/* Low-pass filter gain for vibration detection					*/
		LONG	Khpf;							/* High-pass filter gain for vibration detection					*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {								/* Speed observer variable data definition for vibration detection				*/
		LONG	Ivar64[2];						/* For integral calculation										*/
		LONG	AccSum;							/* Acceleration integrated value(Speed estimated value)							*/
		LONG	SpdObs;							/* Speed estimated value										*/
		LONG	AvSpdFbk;						/* 2times average speed										*/
		LONG	LastSpdErr;						/* Speed deviation previous value									*/
		LONG	AvSpdErr;						/* 2times average speed deviation									*/
		LONG	AvSpdErrbfFil;					/* Speed deviation before filter processing							*/
		LONG	AvSpdErrLpfOut;					/* 2times average speed deviation(Low-pass filter output)			*/
		LONG	Hpftmp;							/* For high-pass filter calculation							*/
		/*------------------------------------------------------------------------------------------*/
		LONG	TrcObsAvSpdErr;					/* Trace  : 2times average speed deviation(Trace for automatic notch)	*/
		BOOL	SpdErrClrReq;					/* Alarm  : Vibration detection differential speed initialization request flag		*/
		LONG	MaxSpdErr;						/* Alarm  : Vibration detection maximum differential speed value	[2^24/OvrSpd]	*/
		LONG	MinSpdErr;						/* Alarm  : Vibration detection minimum differential speed value	[2^24/OvrSpd]	*/
		BOOL	SpdErrFnClrReq;					/* FnFor   : Vibration detection differential speed initialization request flag		*/
		LONG	MaxSpdErrFn;					/* FnFor   : Vibration detection maximum differential speed value	[2^24/OvrSpd]	*/
		LONG	MinSpdErrFn;					/* FnFor   : Vibration detection minimum differential speed value	[2^24/OvrSpd]	*/
		LONG	UnMaxSpdErr;					/* Un10C  : Vibration detection maximum differential speed value	[2^24/OvrSpd]	*/
		LONG	UnMinSpdErr;					/* Un10C  : Vibration detection minimum differential speed value	[2^24/OvrSpd]	*/
		/*------------------------------------------------------------------------------------------*/
	} var;
} DETVIBOBS;


/*--------------------------------------------------------------------------------------------------*/
/*		Data definition for speed deviation tracing using speed observer											*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct DETVIBTRC {					/* Data definition for speed deviation tracing using speed observer		*/
		UCHAR	TrcCmd;							/* Trace execution command(Command/Response)						*/
		UCHAR	TrcInt;							/* Trace interval specification									*/
		USHORT	TrcNum;							/* Trace count specification									*/
//		LONG	*TrcVarA;						/* Trace variable specificationA [2^24/Normalized value]				*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	TrcSamp;						/* For trace interval control								*/
		USHORT	TrcIndx;						/* Trace data index							*/
		SHORT	TrcBufA[DETVIBTRC_BUFMAX];		/* Trace data bufferA [16384/Normalized value]			*/
		SHORT	TrcTrgLvl;						/* Trace trigger level								*/
} DETVIBTRC;


/*--------------------------------------------------------------------------------------------------*/
/*		Data definition for oscillation monitoring																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct DETVIB {
	DETVIBTRC	DetVibTrc;						/* Variables for speed deviation tracing using speed observer		*/
	DETVIBOBS	DetVibObs;						/* Speed observer variable for vibration detection						*/
	struct	{									/* Parameter definition for oscillation monitoring							*/
		LONG	VibCheckLevel;						/* Vibration detection level [2^24/OvrSpd]					*/
		LONG	JstVibCheckLevel;					/* Vibration detection level when inertia identification [2^27/OvrSpd]	*/
		LONG	VibCheckSel;						/* Vibration detection selection									*/
	} conf;
	struct	{									/* Variable definition for oscillation monitoring								*/
		BOOL	VibDetFlg;							/* Vibration detection flag								*/
		BOOL	LowFreqVibDetFlg;					/* Low frequency vibration detection flag							*/
		BOOL	AatAlmCntFlg;						/* AATElapsed time count flag for alarm detection		*/
		ULONG	AatAlmCnt;							/* AATElapsed time count for alarm detection			*/
		ULONG	LowFreqVibAlmCnt;					/* Elapsed time count for low frequency vibration alarm detection		*/
		LONG	SumSpdErr;							/* Difference speed integrated value for low frequency vibration detection					*/
	} var;
} DETVIB;


/*--------------------------------------------------------------------------------------------------*/
/*		Defining constants for damping control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct RESVIBP {						/* Parameter definition for damping control							*/
	LONG	Ks;										/* Vibration suppression observer gain							*/
	LONG	Kj;										/* Vibration suppression observer inertia gain				*/
	LONG	Knd;									/* Damping speed damping gain						*/
	LONG	Klpf;									/* Vibration suppression low-pass filter gain					*/
	LONG	Khpf;									/* Vibration suppression high-pass filter gain					*/
	LONG	Khpf1;								    /* <S03E> 3Inertia vibration suppression control high-pass filter gain			*/
	LONG	Knd1;									/* <S03E> 3Inertia vibration suppression control damping gain2				*/
} RESVIB_PRM;


/*--------------------------------------------------------------------------------------------------*/
/*		Defining variables for damping control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct RESVIB {							/* Parameter & variable definition for damping control					*/
		RESVIB_PRM	ResVibPrm;						/* Vibration suppression control parameters							*/
		BOOL		AresVibSetting;					/* AMold vibration suppression control setting								*/
	/*----------------------------------------------------------------------------------------------*/
	struct {									/* Variable data definition for damping control							*/
		LONG		Ivar64[2];						/* For integral calculation									*/
		LONG		SpdObs;							/* Speed estimated value									*/
		LONG		Hpftmp;							/* For high-pass filter calculation						*/
		LONG		dSpdLpfo;						/* Low-pass filter output speed vibration component				*/
		LONG		dSpdCmp;						/* Damping compensation value		[2^24/OvrSpd]				*/
		LONG		MonResVib;						/* Vibration suppression monitor signal	[2^24/OvrSpd]				*/
		LONG	    Hpftmp1;						/* <S03E> 3For inertia damping control high-pass filter calculation			*/
	} var;
} RESVIB;

/*--------------------------------------------------------------------------------------------------*/
/*		Damping control input parameters	for ResVibCalculatePrm()											*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct RESVIB_CFG_PRM {
	USHORT	jrate;								/* Pn103 : Moment of inertia ratio							*/
	USHORT	avibfrq;							/* Pn161 : A-type vibration damping frequency							*/
	USHORT	avibgn;								/* Pn162 : A type vibration suppression gain correction						*/
	USHORT	avibdamp;							/* Pn163 : A type vibration suppression damping gain					*/
	SHORT	avibfil1;							/* Pn164 : A type vibration suppression filter time constant 1 correction				*/
	SHORT	avibfil2;							/* Pn165 : A type vibration suppression filter time constant 2 correction				*/
	USHORT	avibdamp2;							/* <S03E>  Pn166 : 3 Inertia vibration suppression damping gain			*/
	USHORT	avibfrq2;							/* <S03E> Pn167 :3 Inertia damping frequency				*/
} RESVIB_CFG_PRM;


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void	ResVibATypeResVibInitialize( RESVIB *ResVib );
LONG	ResVibATypeResVibCtrl( RESVIB *ResVib, LONG MotSpd, LONG TrqRef );

BOOL	ResVibCalculatePrmSW( RESVIB *ResVib, ULONG avibsw, BOOL TuningLess, ASICS *SvAsicRegs );
void	ResVibCalculatePrm( RESVIB *ResVib, RESVIB_CFG_PRM *Prm, KSGAIN Kvx, LONG ScanTimeNs, ASICS *SvAsicRegs );

void	DetVibObsInitialize( DETVIB *DetVib );
void	DetVibSpeedObserver( DETVIB *DetVib, LONG SpdFbki, LONG TrqRefMon );
void	DetVibSetSpeedError( DETVIB *DetVib, BOOL TuningLess, BOOL TuningRun );
LONG	DetVibGetSpdErrPeak( DETVIB *DetVib );
LONG	DetVibGetVibCheckLevel( DETVIB *DetVib );
void	DetVibResetMaxMinSpeedError( DETVIB *DetVib );
void	DetVibResetMaxMinSpeedErrorFn( DETVIB *DetVib );
LONG	DetVibGetSpdErrPeakFn( DETVIB *DetVib );

void	DetVibInitialize( DETVIB *DetVib );
LONG	DetVibDetectVibration( DETVIB *DetVib );
void	DetVibResetRequest( DETVIB *DetVib );
LONG	DetVibDetectLowFreqVibration( DETVIB *DetVib );
void	DetVibLowFreqResetRequest( DETVIB *DetVib );

void	DetVibTraceInitialize( DETVIB *DetVib );
void	DetVibTraceRuntimeService( DETVIB *DetVib );
UCHAR	DetVibTraceGetStatus( DETVIB *DetVib );
SHORT*	DetVibTraceGetDataAdr( DETVIB *DetVib );
USHORT	DetVibTraceGetIndex( DETVIB *DetVib );
void	DetVibTraceStartReq( DETVIB *DetVib );
void	DetVibTraceStopReq( DETVIB *DetVib );
void	DetVibTraceSetCondition( DETVIB *DetVib, USHORT TraceNum, UCHAR TraceInt, SHORT TrigerLvl );

void	DetVibObsCalculateInitPrm( DETVIBOBS *DetVibObs, KSGAIN Kvx, ULONG jrate, LONG ScanTimeNs );
void	DetVibObsCalculateGains( DETVIBOBS *DetVibObs, LONG ObsGain, LONG Lpf, LONG Hpf, LONG ScanTimeNs );
void	DetVibObsCalculateJGain( DETVIBOBS *DetVibObs, KSGAIN Kvx, ULONG jrate, LONG ScanTimeNs );
void	DetVibObsCalculateInitPrmAAT( DETVIBOBS *DetVibObs, BOOL TuningLess, LONG ScanTimeNs );


#define _RES_VIB_CTRL_H /* For reload prevention */
#endif /* _RES_VIB_CTRL_H */
/***************************************** end of file **********************************************/

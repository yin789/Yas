#ifndef	_PCMD_FILTER_H /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PcmdFilter.h : Header file for position command filter calculation										*/
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
#include "Mlib.h"


/****************************************************************************************************/
/*																									*/
/*		Constant definition																					*/
/*																									*/
/****************************************************************************************************/
#define MAFIL_BUFNUM			256						/* Number of moving average filter buffers				*/
#define	SUMX_LIM				0x3FFFFFFF				/* ExpFil.V.Pexfvar.sumx: Overflow threshold	*/
#define	MAXOMEGA2(AvfCycleNs)	(127323954/AvfCycleNs)	/* 127323954 = 10^9 * 0.8 / (2*PAI)			*/
														/*						Frequency2Maximum value[Hz]	*/


/****************************************************************************************************/
/*																									*/
/*		Variables Definition																		*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter variable definition for exponential acceleration/deceleration filter														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct _EXPFIL {					/* Parameter & variable definition for exponential acceleration/deceleration filter				*/
	struct	{								/* Parameters for exponential acceleration/deceleration filter						*/
		LONG	Kf[2];							/* Exponential acceleration/deceleration filter gain							*/
		LONG	Pbias[2];						/* Exponential acceleration/deceleration filter bias						*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct	{								/* Variable data for exponential acceleration/deceleration filter						*/
		LONG	Filo;							/* Filter output										*/
		PEXFV	Pexfvar;						/* Variable structure for exponential acceleration/deceleration filter calculation				*/
		BOOL	RefZStatus;						/* Payout completion status									*/
	} var;
} EXPFIL;


/*--------------------------------------------------------------------------------------------------*/
/*		Parameter variable definition for moving average filter														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct _MAFIL {						/* Parameter & variable definition for moving average filter				*/
	struct	{								/* Parameters for moving average filter							*/
		LONG	PmafNum[2];						/* Number of moving averages										*/
		UCHAR	ImafSft[2];						/* Interpolation moving average number of times(2^Sft)							*/
		UCHAR	SpareByte[2];					/* Preliminary												*/
		/*----------------------------------------------------------------------------------------------*//*<S0C3>*/
		LONG	PmafNumSec[2];						/* Moving average2times										*/
		UCHAR	ImafSftSec[2];						/* Interpolation moving average2times(2^Sft)							*/
		UCHAR	SpareByteSec[2];					/* Preliminary2												*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct	{								/* Variable data for moving average filter							*/
		LONG	PcmdInX;						/* For long-term interpolation moving average filter calculation					*/
		LONG	PcmdInY;						/* For long-term interpolation moving average filter calculation					*/
		LONG	Index;							/* For interpolation moving average filter calculation						*/
		LONG	Filo;							/* Filter output										*/
		BOOL	RefZStatus;						/* Payout completion status									*/
		PMAFV	Pmafvar;						/* Variable structure for moving average filter calculation					*/
		PIMFV	Pimfvar;						/* Variable structure for interpolation moving average filter calculation				*/
		LONG	Pmafbuf[ MAFIL_BUFNUM ];		/* Buffer for moving average filter calculation					*/
		/*----------------------------------------------------------------------------------------------*//*<S0C3>*/
		LONG	PcmdInXSec;						/* Long-term interpolation moving average filter2For calculation					*/
		LONG	PcmdInYSec;						/* Long-term interpolation moving average filter2For calculation					*/
		LONG	IndexSec;							/* Interpolation moving average filter2For calculation						*/
		LONG	FiloSec;							/* Filter2Output										*/
		HIGHFV	PmafvarSec;						/* Moving average filter2Variable structure for calculation					*/
		HIMFV	PimfvarSec;						/* Interpolation moving average filter2Variable structure for calculation				*/
		LONG	PmafbufSec[ MAFIL_BUFNUM ];		/* Moving average filter2Buffer for calculation					*/
	} var;
} MAFIL;


/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for position command filter																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct _PCMDFIL {					/* Parameter & variable definition for position command filter calculation			*/
	EXPFIL	ExpFil;								/* Acceleration/deceleration filter variable								*/
	MAFIL	MaFil;								/* Variables for moving average filter							*/
} PCMDFIL;


/*--------------------------------------------------------------------------------------------------*/
/*		Parameter variable definition for vibration suppression filter														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct VIBSUPFILPRM {					/* Parameter definition for vibration suppression filter					*/
	LONG			Kff1;							/* Vibration suppression filter gain1						*/
	LONG			Kff2;							/* Vibration suppression filter gain2						*/
	LONG			Kff3;							/* Vibration suppression filter gain3						*/
	BOOL			enable;							/* Vibration suppression filter function selection						*/
} VIBSUPFILPRM;

typedef	struct VIBSUPFIL {						/* Parameter & variable definition for vibration suppression filter			*/
	struct {									/* Parameter definition for vibration suppression filter					*/
		VIBSUPFILPRM	VibSupPrm;					/* Parameters for vibration suppression filter					*/
		VIBSUPFILPRM	Pexe;						/* Parameters for vibration suppression filter for execution			*/
		BOOL			VibSupSetting;				/* Vibration suppression filter setting							*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {									/* Variable data definition for vibration suppression filter					*/
		LONG			rem1;						/* Vibration suppression filter calculation						*/
		LONG			rem2;						/* Vibration suppression filter calculation						*/
		LONG			rem3;						/* Vibration suppression filter calculation						*/
		LONG			wkbf1;						/* Vibration suppression filter calculation						*/
		LONG			wkbf2;						/* Vibration suppression filter calculation						*/
		LONG			FilioErr[2];				/* Vibration suppression filter input/output deviation					*/
		LONG			Buf;						/* Vibration suppression filter accumulation						*/
		LONG			Filo;						/* Vibration suppression filter output							*/
	} var;
} VIBSUPFIL;



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void	PcmdFilInitialize( PCMDFIL *PcmdFil, VIBSUPFIL *VibSupFil );
void	FFCmdFilInitialize( MAFIL *pMaFil );									/* <S1D0> */
LONG	FFcmdFilMoveAverageFilter( MAFIL *pMaFil, LONG FFcmdIn, BOOL FilStop );	/* <S1D0> */
LONG	PcmdFilRuntimeService( PCMDFIL *PcmdFil, LONG PcmdIn, LONG FilRest, BOOL FilStop );
/*--------------------------------------------------------------------------------------------------*/
void	PcmdFilCalculatePrmExpFilter( EXPFIL *ExpFil, USHORT PrmTime, KSGAIN KMotSpdConv,
													 UCHAR MaxVel, LONG SvCycleUs, LONG Flag );
void	PcmdFilCalculatePrmExpFilBias( EXPFIL *ExpFil, USHORT expbias1,
												LONG Egear_a, LONG Egear_b, LONG SvCycleUs );
void	PcmdFilCalculatePrmMoveAvergeFilter( MAFIL *MaFil, USHORT PrmTime, USHORT Highrad,
		                                      LONG SvCycleUs, LONG Flag );/*<S0C3>*/
void    PcmdFilCalculatePrmHighFilter( MAFIL *MaFil, USHORT PrmTime, USHORT Highrad,
		                                      LONG SvCycleUs, LONG Flag );/*<S0C3>*/
/*--------------------------------------------------------------------------------------------------*/
void	PcmdFilSetRefZStatus( PCMDFIL *PcmdFil, BOOL status );
BOOL	PcmdFilGetRefZStatus( PCMDFIL *PcmdFil );
/*--------------------------------------------------------------------------------------------------*/
LONG	PcmdFilVibSupFilter( VIBSUPFIL *VibSupFil, LONG dPcmd, BOOL *RefZSignal );
BOOL	PcmdFilCalculatePrmVibSupFilSW( VIBSUPFIL *VibSupFil, ULONG mdlsw,
												ULONG ff_feq, ULONG ff_fil, LONG ScanTimeNs );
void	PcmdFilCalculatePrmVibSupFil( VIBSUPFIL *VibSupFil, ULONG ff_frq, ULONG ff_fil, LONG ScanTimeNs );


#define _PCMD_FILTER_H /* For reload prevention */
#endif /* _PCMD_FILTER_H */
/***************************************** end of file **********************************************/

/****************************************************************************************************/
/*																									*/
/*																									*/
/*		BaseControlPrmCalc.h : BaseControl							*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.09.28	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#ifndef	_BASECTRL_PRM_CALC_H
#define _BASECTRL_PRM_CALC_H


#include "BaseControls.h"


/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
typedef struct BASE_SPDLOOP_PRM {
	USHORT	jrate;
	USHORT	loophz;					/* Pn100 : Speed loop gain										*/
	USHORT	pitime;					/* Pn101 : Speed loop integral time constant									*/
	USHORT	loophz2;				/* Pn104 : Second velocity loop gain									*/
	USHORT	pitime2;				/* Pn105 : Second velocity loop integral time constant								*/
	DBYTEX	ipg_rwt;				/* PnE44 : Lower:i-PControl ratio, Upper level:Runaway detection torque				*/
} BASE_SPDLOOP_PRM;

void	PcalBaseSpeedControl(
			BASE_CTRL_HNDL *BaseCtrlData, BPRMDAT *Bprm, BASE_SPDLOOP_PRM *Prm, LONG GselNo );

void	PcalBasePositionPControl(
			BASE_CTRL_HNDL *BaseCtrlData, BPRMDAT *Bprm, EGEAR *Egear, ULONG Posgn, LONG GselNo );

void	PcalZeroPositionControl(
			BASE_CTRL_HNDL *BaseCtrlData, BPRMDAT *Bprm, LONG Posgn, LONG GselNo );

void	PcalSpeedLimitGain(
			BASE_CTRL_HNDL *BaseCtrlData, BPRMDAT *Bprm, USHORT jrate, ULONG Loophz, LONG GselNo );

void	PcalBaseTrqLpassFilter(
			TRQFIL *TrqFilData, BASE_CTRL_HNDL *BaseCtrlData, LONG trqfil11, LONG trqfil12, LONG GselNo );

void	PcalBaseTrqLpassFilter2(
			TRQFIL *TrqFilData, BASE_CTRL_HNDL *BaseCtrlData, USHORT secfil, USHORT secq );

LONG	PcalLowfilter2( LONG hz, LONG qx, LONG ts, KSGAIN kn[4] );
#if (FLOAT_USE==TRUE)
void	f_PcalLowfilter2(								/* Second-order low-pass filter parameter calculation			*/
			float	hz,									/* Filter frequency		[Hz]				*/
			float	qx,									/* Quality factor	[-]				*/
			float	ts,									/* Calculation cycle				[sec]				*/
			float	kn[4] );							/* Calculation result storage								*/
#endif /* FLOAT_USE */
KSGAIN	PcalBaseTrqLpassFilter3( LONG Trqfil, LONG ScanTime );

KSGAIN	PcaldPosRefFFGain( KSGAIN Kpx, LONG ffgn, LONG ScanTime );

#if (FLOAT_USE==TRUE)
float	f_BpxNorObsGainCal( float ObsGain , float scantime );
float	f_BpxNorObsJGainCal( KSGAIN Kvx, float jrate, float ObsJGain , float scantime );
#endif /* FLOAT_USE */
LONG	BpxNorObsGainCal( LONG ObsGain , LONG scantime );
LONG	BpxNorObsJGainCal( KSGAIN Kvx, LONG jrate, LONG ObsJGain , LONG scantime );



/***************************************** end of file **********************************************/
#endif /* _BASECTRL_PRM_CALC_H */

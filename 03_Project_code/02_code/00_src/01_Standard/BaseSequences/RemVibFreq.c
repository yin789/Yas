/****************************************************************************************************/
/*																									*/
/*																									*/
/*		RemVibFreq.c : Residual vibration frequency calculation process definition													*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Residual vibration frequency calculation process																	*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
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
#include "Basedef.h"
#include "MLib.h"
#include "KLib.h"
#include "RemVibFreq.h"



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void	RemVibChkCondition( REMVIBFREQ *RemVibFreq, BOOL PosCtrlMode );							*/
/*	void	RemVibCalculateFreq( REMVIBFREQ *RemVibFreq, LONG CtrlPosErr,							*/
/*																BOOL CoinSts, BOOL RefzSts );		*/
/****************************************************************************************************/

/****************************************************************************************************/
/*		API																							*/
/****************************************************************************************************/
/*	LONG	RemVibGetRemVibFreq( REMVIBFREQ *RemVibFreq );											*/
/****************************************************************************************************/


/****************************************************************************************************/
/*																									*/
/*		Residual vibration frequency calculation initialization process																*/
/*																									*/
/****************************************************************************************************/
void	RemVibChkCondition( REMVIBFREQ *RemVibFreq, BOOL PosCtrlMode )
{
	if( PosCtrlMode != TRUE )
	{ /* Other than position control mode */
		/* Forcibly terminate(No frequency calculation) */
		RemVibFreq->var.State = RVM_INIT;
	}
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Residual vibration frequency calculation																			*/
/*																									*/
/*		Overview: Calculate the residual vibration frequency after completion of position command payout.									*/
/*			Residual vibration frequency is the overshoot after completion of position command payout,							*/
/*			Each amount of undershoot2times, and during overshoot,						*/
/*			Find the average value of the time between undershoots and convert the frequency to use as residual vibration.				*/
/*			Frequency calculation is10pieces peak or500ms, terminate.					*/
/*			If a position command is input during calculation, if calculation is possible, perform frequency calculation,			*/
/*			If calculation is not possible, return to initialization processing.													*/
/*																									*/
/****************************************************************************************************/
void	RemVibCalculateFreq( REMVIBFREQ *RemVibFreq, LONG CtrlPosErr, BOOL CoinSts, BOOL RefzSts )
{
	LONG	lwk;
	ULONG	ulwk;
	ULONG	Sumtime;
	ULONG	tmpdiff;

	/*----------------------------------------------------------------------------------------------*/
	/*	Position deviation filter processing(Position deviation noise cut)												*/
	/*----------------------------------------------------------------------------------------------*/
	lwk = CtrlPosErr;								/* Position amplifier deviation		[pulse]					*/
	RemVibFreq->var.PosErrFilOut2 = MlibLpfilter1( lwk,
												   RemVibFreq->conf.RemVibFil,
												   RemVibFreq->var.PosErrFilOut2 );

	RemVibFreq->var.PosErrFilOut = MlibLpfilter1( RemVibFreq->var.PosErrFilOut2,
												  RemVibFreq->conf.RemVibFil,
												  RemVibFreq->var.PosErrFilOut );

	switch( RemVibFreq->var.State )
	{
	/*----------------------------------------------------------------------------------------------*/
	/*	Initialize																						*/
	/*----------------------------------------------------------------------------------------------*/
	case RVM_INIT:
		/*------------------------------------------------------------------------------------------*/
		/*	Clear data related to residual vibration frequency calculation 														*/
		/*------------------------------------------------------------------------------------------*/
		MlibResetLongMemory( &(RemVibFreq->var), sizeof( RemVibFreq->var )>>2 );
		RemVibFreq->var.State = RVM_COIN;
		break;

	/*----------------------------------------------------------------------------------------------*/
	/*	Waiting for positioning completion																			*/
	/*----------------------------------------------------------------------------------------------*/
	case RVM_COIN:
		if( (RefzSts == TRUE) && (CoinSts == TRUE) )
		{ /* Position command dispensing complete && Positioning complete */
			RemVibFreq->mon.UnRvibFrequency = 0;				/* UnClear display	<S0DA>				*/
			/*--------------------------------------------------------------------------------------*/
			/*	Position deviation direction determination				 													*/
			/*		※If the motor is stopped, the deviation will not increase and this sequence will remain.		*/
			/*--------------------------------------------------------------------------------------*/
			if( (RemVibFreq->var.LastPosErr - RemVibFreq->var.PosErrFilOut) < 0 )
			{ /* Deviation increases in the + direction */
				RemVibFreq->var.Dir = 1;
				RemVibFreq->var.State = RVM_PEAKCHK_F;
			}
			else if( (RemVibFreq->var.LastPosErr - RemVibFreq->var.PosErrFilOut) > 0 )
			{ /* Deviation increases in the - direction */
				RemVibFreq->var.Dir = -1;
				RemVibFreq->var.State = RVM_PEAKCHK_F;
			}
			/* Overshoot timer counter initialization */
			RemVibFreq->var.PeakCnt = 0;
			/* Overshoot amount saving */
			RemVibFreq->var.PeakValue = RemVibFreq->var.PosErrFilOut;
			/* Due to peak detection timeout, the counter value will be half of the minimum detection frequency */
			KlibRstLongTimer( &RemVibFreq->var.TimeOutCtr );
			/* Residual vibration time measurement timer(ScanCPeriod)Initialize */
			KlibRstLongTimer( &RemVibFreq->var.PeakCntTimer );
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	/*	Peak detection(Before vibration frequency detection)																*/
	/*----------------------------------------------------------------------------------------------*/
	case RVM_PEAKCHK_F:
		/*------------------------------------------------------------------------------------------*/
		/*	Position command discrimination & timeout processing															*/
		/*------------------------------------------------------------------------------------------*/
		if( (RefzSts == FALSE)
			|| (KlibGetLongTimerMs(RemVibFreq->var.TimeOutCtr) >= REMVIB_TIMEOUT) )
		{ /* Position command dispensing || At timeout */
			RemVibFreq->var.State = RVM_INIT;					/* Forcibly terminate(No frequency calculation)			*/
			break;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Peak update processing																			*/
		/*		Update the peak counter when the overshoot amount is increasing in the same direction			*/
		/*------------------------------------------------------------------------------------------*/
		if( (RemVibFreq->var.Dir * (RemVibFreq->var.PosErrFilOut - RemVibFreq->var.PeakValue)) > 0 )
		{ /* Updating peak */
			/* Overshoot amount maximum value update */
			RemVibFreq->var.PeakValue = RemVibFreq->var.PosErrFilOut;
			/* Overshoot maximum value timer counter(ScanCPeriod)Update */
			RemVibFreq->var.PeakCnt = ( KlibGetLongTimerMs(RemVibFreq->var.PeakCntTimer) / REMVIB_CYCLEMS );
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Peak inversion determination																			*/
		/*------------------------------------------------------------------------------------------*/
		if( ((RemVibFreq->var.Dir * (RemVibFreq->var.PeakValue - RemVibFreq->var.PosErrFilOut))
			> RemVibFreq->conf.RemVibDetWidth) )
		{ /* Peak inversion */
			KlibRstLongTimer( &RemVibFreq->var.TimeOutCtr );	/* Timeout counter update			*/

			/*--------------------------------------------------------------------------------------*/
			/*	Peak inversion4times.											*/
			/*	(Actually peak inversion3times, but the first inversion is affected by noise, so ignore it and change it to)	*/
			/*--------------------------------------------------------------------------------------*/
			if( RemVibFreq->var.RevCnt >= 3 )
			{ /* 4th peak -> Frequency calculation */
				/* Difference between peaks in the same direction */
				lwk = (LONG)RemVibFreq->var.PeakCnt - (LONG)RemVibFreq->var.LLastpeakCnt;
				/* Clamp at minimum detection frequency */
				tmpdiff = MlibLimitul( lwk, REMVIB_MINDET, 0 );

				Sumtime = tmpdiff * REMVIB_DENO_OFFSET;
				RemVibFreq->var.MeanDeno =  REMVIB_DENO_OFFSET;

				/* Average value calculation */
				ulwk = Sumtime / RemVibFreq->var.MeanDeno;
				/* Clamp at minimum detection frequency */
				RemVibFreq->var.FreqCnt = MlibLimitul( ulwk, REMVIB_MINDET, 2 );

				/* To start vibration frequency detection */
				RemVibFreq->var.State = RVM_PEAKCHK_L;

			}
			/* Peak data update */
			RemVibFreq->var.LLastpeakCnt = RemVibFreq->var.LastPeakCnt;
			RemVibFreq->var.LastPeakCnt = RemVibFreq->var.PeakCnt;
			RemVibFreq->var.RevCnt++;
			RemVibFreq->var.Dir *= -1;
			RemVibFreq->var.PeakValue = RemVibFreq->var.PosErrFilOut;
			RemVibFreq->var.PeakCnt = ( KlibGetLongTimerMs(RemVibFreq->var.PeakCntTimer) / REMVIB_CYCLEMS );
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	/*	Peak detection(Start vibration frequency detection)																*/
	/*----------------------------------------------------------------------------------------------*/
	case RVM_PEAKCHK_L:
		/*------------------------------------------------------------------------------------------*/
		/*	Position command discrimination & timeout processing															*/
		/*------------------------------------------------------------------------------------------*/
		if( (RefzSts == FALSE)
			|| (KlibGetLongTimerMs( RemVibFreq->var.TimeOutCtr ) >= REMVIB_TIMEOUT) )
		{ /* Position command dispensing || At timeout */
			RemVibFreq->var.State = RVM_COMP;					/* Forcibly terminate(To frequency calculation)			*/
			break;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Peak update processing																			*/
		/*		Update the peak counter when the overshoot amount is increasing in the same direction			*/
		/*------------------------------------------------------------------------------------------*/
		if( (RemVibFreq->var.Dir * (RemVibFreq->var.PosErrFilOut - RemVibFreq->var.PeakValue)) > 0 )
		{	/* Updating peak */
			/* Overshoot amount maximum value update */
			RemVibFreq->var.PeakValue = RemVibFreq->var.PosErrFilOut;
			/* Overshoot maximum value timer counter(ScanCPeriod)Update */
			RemVibFreq->var.PeakCnt = ( KlibGetLongTimerMs(RemVibFreq->var.PeakCntTimer) / REMVIB_CYCLEMS );
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Peak inversion determination																			*/
		/*------------------------------------------------------------------------------------------*/
		if( ((RemVibFreq->var.Dir * (RemVibFreq->var.PeakValue - RemVibFreq->var.PosErrFilOut))
			> RemVibFreq->conf.RemVibDetWidth) )
		{ /* Peak inversion*/
			KlibRstLongTimer( &RemVibFreq->var.TimeOutCtr );	/* Timeout counter update			*/

			if (RemVibFreq->var.RevCnt >= 9)
			{	/* Peak is10or above */
				RemVibFreq->var.State = RVM_COMP;				/* Forcibly terminate(To frequency calculation)			*/
			}
			/* Difference between peaks in the same direction */
			lwk = (LONG)RemVibFreq->var.PeakCnt - (LONG)RemVibFreq->var.LLastpeakCnt;
			/* Clamp at minimum detection frequency */
			tmpdiff = MlibLimitul( lwk, REMVIB_MINDET, 0 );

			/*--------------------------------------------------------------------------------------*/
			/*	Average value calculation																			*/
			/*--------------------------------------------------------------------------------------*/
			/*	Molecular sequential calculation																		*/
			/*		SUM = Mean(n-1):Previous average value * MeanDeno(n-1):Previous denominator * r:Weight + X(n):Current value		*/
			/*																						*/
			/*		Note) Minimum detection frequency:0.8Hz, Weight1.2, Offset1000and10times sequential calculation			*/
			/*			The numerator of the following formula should be0xB23E7ECLONGWithin size								*/
			/*--------------------------------------------------------------------------------------*/
			Sumtime =
				((RemVibFreq->var.FreqCnt * RemVibFreq->var.MeanDeno * REMVIB_WEIGHT_NUME) / REMVIB_WEIGHT_DENO)
				+ (tmpdiff * REMVIB_DENO_OFFSET);

			/*--------------------------------------------------------------------------------------*/
			/*	Denominator sequential calculation																		*/
			/*		SUM = MeanDeno(n-1):Previous denominator * r:Weight + 1										*/
			/*																						*/
			/*		Note) Weight1.2, Offset1000and10times sequential calculationSUMteeth25959, so I will correct it so that it becomes				*/
			/*			65536.													*/
			/*--------------------------------------------------------------------------------------*/
			RemVibFreq->var.MeanDeno =
				((RemVibFreq->var.MeanDeno * REMVIB_WEIGHT_NUME) / REMVIB_WEIGHT_DENO) + REMVIB_DENO_OFFSET;

			/* Average value calculation */
			ulwk = Sumtime / RemVibFreq->var.MeanDeno;
			/* Clamp at minimum detection frequency*/
			RemVibFreq->var.FreqCnt = MlibLimitul( ulwk, REMVIB_MINDET, 2 );

			/* Peak data update */
			RemVibFreq->var.LLastpeakCnt = RemVibFreq->var.LastPeakCnt;
			RemVibFreq->var.LastPeakCnt = RemVibFreq->var.PeakCnt;
			RemVibFreq->var.RevCnt++;
			RemVibFreq->var.Dir *= -1;
			RemVibFreq->var.PeakValue = RemVibFreq->var.PosErrFilOut;
			RemVibFreq->var.PeakCnt = ( KlibGetLongTimerMs(RemVibFreq->var.PeakCntTimer) / REMVIB_CYCLEMS );
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	/*	Vibration frequency calculation																				*/
	/*----------------------------------------------------------------------------------------------*/
	case RVM_COMP:
			if( (RemVibFreq->var.FreqCnt <= REMVIB_MINOUT) && (RemVibFreq->var.FreqCnt > 0) )
			{
				/* Frequency calculation [0.1Hz] */
				RemVibFreq->mon.UnRvibFrequency = 10000 /(RemVibFreq->var.FreqCnt * KPI_SCCYCLEMS);	/* <S0DA> */
			}
			else
			{
				/* Less than the minimum output frequency0.0Hz */
				RemVibFreq->mon.UnRvibFrequency = 0;	/* <S0DA> */
			}
			if( RefzSts == FALSE )
			{ /* Position command dispensing */
				/* Go to initialization process */
				RemVibFreq->var.State = RVM_INIT;
			}
		break;
	default:
		break;
	}

	RemVibFreq->var.LastPosErr = RemVibFreq->var.PosErrFilOut;
	return;
}


/****************************************************************************************************/
/*																									*/
/*		For acquiring residual vibration frequencyAPI																		*/
/*																									*/
/****************************************************************************************************/
LONG	RemVibGetRemVibFreq( REMVIBFREQ *RemVibFreq )
{
	return	RemVibFreq->mon.UnRvibFrequency;		/* <S0DA> */
}

/****************************************************************************************************/
/*																									*/
/*		Residual vibration frequency clear processing 																	*/
/*																									*/
/****************************************************************************************************/
void	RemVibRsetRemVibrationFreq( REMVIBFREQ *RemVibFreq )
{
	RemVibFreq->mon.UnRvibFrequency = 0;			/* Residual vibration frequency monitor clear <S0DA>			*/
	RemVibFreq->var.FreqCnt = 0;					/* Residual vibration frequency counter clear					*/
	return;
}

/****************************************************************************************************/
/*																									*/
/*		Residual vibration frequency monitor parameter calculation															*/
/*																									*/
/****************************************************************************************************/
void	RemVibCalculateMonitorPrm( REMVIBFREQ *RemVibFreq, EGEAR *Egear,
    	                           USHORT remdetw, LONG CoinLevel, LONG ScanTime )
{
	LONG		kx, sx;

	sx = 0;
	kx = MlibScalKxgain( CoinLevel, remdetw, 1000, &sx, 0 );
	kx = MlibPcalKxgain( kx, Egear->b, Egear->a, &sx,  -24 );

	/* Limit at minimum detection width */
	RemVibFreq->conf.RemVibDetWidth = MlibLimitul( kx, 0x1000000, REMVIB_MINDETWIDTH );
	/* For position deviation noise cut */
	RemVibFreq->conf.RemVibFil = MlibPcalKf1gain( REMVIB_FILFREQ, ScanTime, 1 );
}



/***************************************** end of file **********************************************/

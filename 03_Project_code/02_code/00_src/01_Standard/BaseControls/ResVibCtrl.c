/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ResVibCtrl.c : For vibration detection/vibration suppression control	module definition											*/
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
/*	Note	:	First edition	2010.11.11	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "MLib.h"
#include "ResVibCtrl.h"
#include "BaseControlPrmCalc.h"


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void	ResVibATypeResVibInitialize( RESVIB *ResVib );											*/
/*	LONG	ResVibATypeResVibCtrl( RESVIB *ResVib, LONG MotSpd, LONG TrqRef );						*/
/*--------------------------------------------------------------------------------------------------*/
/*	void	DetVibObsInitialize( DETVIB *DetVib );													*/
/*	void	DetVibSpeedObserver( DETVIB *DetVib, LONG SpdFbki, LONG TrqRefMon );					*/
/*	void	DetVibSetSpeedError( DETVIB *DetVib, BOOL TuningLess, BOOL TuningRun );					*/
/*	LONG	DetVibGetSpdErrPeak( DETVIB *DetVib );													*/
/*	LONG	DetVibGetVibCheckLevel( DETVIB *DetVib );												*/
/*	void	DetVibResetMaxMinSpeedError( DETVIB *DetVib );											*/
/*	void	DetVibResetMaxMinSpeedErrorFn( DETVIB *DetVib );										*/
/*	LONG	DetVibGetSpdErrPeakFn( DETVIB *DetVib );												*/
/*--------------------------------------------------------------------------------------------------*/
/*	void	DetVibInitialize( DETVIB *DetVib );														*/
/*	LONG	DetVibDetectVibration( DETVIB *DetVib );												*/
/*	void	DetVibResetRequest( DETVIB *DetVib );													*/
/*	LONG	DetVibDetectLowFreqVibration( DETVIB *DetVib );											*/
/*	void	DetVibLowFreqResetRequest( DETVIB *DetVib );											*/
/*--------------------------------------------------------------------------------------------------*/
/*	void	DetVibTraceInitialize( DETVIB *DetVib );												*/
/*	void	DetVibTraceRuntimeService( DETVIB *DetVib );											*/
/*	UCHAR	DetVibTraceGetStatus( DETVIB *DetVib );													*/
/*	SHORT*	DetVibTraceGetDataAdr( DETVIB *DetVib );												*/
/*	USHORT	DetVibTraceGetIndex( DETVIB *DetVib );													*/
/*	void	DetVibTraceStartReq( DETVIB *DetVib );													*/
/*	void	DetVibTraceStopReq( DETVIB *DetVib );													*/
/*	void	DetVibTraceSetCondition( DETVIB *DetVib, USHORT TraceNum,								*/
/*															UCHAR TraceInt, SHORT TrigerLvl );		*/
/****************************************************************************************************/
extern void __EI( void );									/* Interrupt enabling setting						*/
extern void __DI( void );									/* Interrupt disable setting						*/



#if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Initializing damping control variables																			*/
/*																									*/
/****************************************************************************************************/
void	ResVibATypeResVibInitialize( RESVIB *ResVib )
{
	MlibResetLongMemory( &ResVib->var, sizeof(ResVib->var)/4 );
	return;
}
#endif /* #if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 ) */


/* 2014/04/01MDis belowAMold vibration damping processing moved to micro operation unit*/
#if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		AMold vibration suppression control calculation																				*/
/*																									*/
/*	Overview:	1Configure the next speed observer and calculate the estimated speed andFBCalculate damping signal from speed						*/
/*			The speed is calculated by multiplying the damping signal by the damping gain.FBto display [25]										*/
/*			SGDXNow the functionOFF.(Analog monitor also enabled)							*/
/*																									*/
/* arg		: LONG	MotSpd				:	Speed feedback	[2^24/OvrSpd]						*/
/*			: LONG	TrqRef				:	Torque command			[2^24/MaxTrq]						*/
/* out		:																						*/
/* g-out	: LONG	SpdFBCx				:	SpeedFBCompensation			[2^24/OvrSpd]						*/
/*																									*/
/****************************************************************************************************/
LONG	ResVibATypeResVibCtrl( RESVIB *ResVib, LONG MotSpd, LONG TrqRef )
{
	LONG	u1, u2, u3;//<S03E>
	LONG	x1, x2;

	u1 = MotSpd - ResVib->var.SpdObs;
#if (FLOAT_USE==TRUE)
	x1 = u1 * ResVib->ResVibPrm.Ks;
	x2 = TrqRef * ResVib->ResVibPrm.Kj;

	ResVib->var.SpdObs = MlibLimitul( (ResVib->var.SpdObs + x1 + x2), 0x1000000, -0x1000000 );

	u2 = MlibHpfilter1( u1, ResVib->ResVibPrm.Khpf, &ResVib->var.Hpftmp );
	ResVib->var.dSpdLpfo = MlibLpfilter1( u2, ResVib->ResVibPrm.Klpf, ResVib->var.dSpdLpfo );

	ResVib->var.dSpdCmp = ResVib->var.dSpdLpfo * ResVib->ResVibPrm.Knd;
#else
	x1 = MlibMulgainNolim( u1, ResVib->ResVibPrm.Ks );
	x2 = MlibMulgain( TrqRef, ResVib->ResVibPrm.Kj );

	ResVib->var.SpdObs = MlibLimitul( (ResVib->var.SpdObs + x1 + x2), 0x1000000, -0x1000000 );

	u2 = MlibHpfilter1( u1, ResVib->ResVibPrm.Khpf, &ResVib->var.Hpftmp );
	ResVib->var.dSpdLpfo = MlibLpfilter1( u2, ResVib->ResVibPrm.Klpf, ResVib->var.dSpdLpfo );

	u3 = MlibHpfilter1( ResVib->var.dSpdLpfo, ResVib->ResVibPrm.Khpf1, &ResVib->var.Hpftmp1 );//<S03E>

	//ResVib->var.dSpdCmp = MlibMulgain( ResVib->var.dSpdLpfo, ResVib->ResVibPrm.Knd );//<S03E>
	ResVib->var.dSpdCmp = MlibMulgain( ResVib->var.dSpdLpfo, ResVib->ResVibPrm.Knd ) + MlibMulgain( u3, ResVib->ResVibPrm.Knd1 );//<S03E>
#endif /* FLOAT_USE */

	/* Variable output for monitor */
	ResVib->var.MonResVib  = ResVib->var.dSpdLpfo;

	return	ResVib->var.dSpdCmp;
}
#endif /* #if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 ) */


#if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Damping control selection switch(Pn10B.3)																*/
/*																									*/
/****************************************************************************************************/
BOOL	ResVibCalculatePrmSW( RESVIB *ResVib, ULONG avibsw, BOOL TuningLess, ASICS *SvAsicRegs )
{
	LONG	wk;
	BOOL	PrmErr;
	PrmErr = FALSE;

	if( TuningLess == FALSE )
	{
		wk = avibsw & 0x0F;
		switch( wk )
		{
		case 0x00: /* Disabling damping control */
			/* Interim processing ASICtoMREG_RVCTRL, so clear the pending */
			ResVib->AresVibSetting = FALSE;
//			SvAsicRegs->AsicMicroReg->MREG_RVCTRL = FALSE;
			break;
		case 0x01: /* A-type vibration damping control enabled */
			/* Interim processing ASICtoMREG_RVCTRL, so clear the pending */
			ResVib->AresVibSetting = TRUE;
//			SvAsicRegs->AsicMicroReg->MREG_RVCTRL = TRUE;
			break;
		default :
			PrmErr = TRUE;
			break;
		}
	}
	else
	{
		/* Adjustmentless enabledAMold damping disabled */
		/* Interim processing ASICtoMREG_RVCTRL, so clear the pending */
		ResVib->AresVibSetting = FALSE;
//		SvAsicRegs->AsicMicroReg->MREG_RVCTRL = FALSE;
	}

	return	PrmErr;
}
#endif /* #if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 ) */



#if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Vibration control parameter calculation																		*/
/*																									*/
/****************************************************************************************************/
void	ResVibCalculatePrm( RESVIB *ResVib, RESVIB_CFG_PRM *Prm, KSGAIN Kvx, LONG ScanTimeNs, ASICS *SvAsicRegs )
{
	RESVIB_PRM	wrkp;
	LONG		kx, sx;
	LONG		wk, wk2;

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression observer gain(Damping frequency[0.1Hz]) : ResVib.P.Ks										*/
/*																									*/
/*			= ResFreq[0.1Hz]/2																		*/
/*--------------------------------------------------------------------------------------------------*/
	wrkp.Ks = BpxNorObsGainCal( (Prm->avibfrq >> 1) ,ScanTimeNs);

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression observer inertia gain([%]) : ResVib.P.Kj											*/
/*--------------------------------------------------------------------------------------------------*/
	wrkp.Kj = BpxNorObsJGainCal( Kvx, Prm->jrate, Prm->avibgn , ScanTimeNs );

/*--------------------------------------------------------------------------------------------------*/
/*		Damping speed damping gain([%]) : ResVib.P.Knd												*/
/*--------------------------------------------------------------------------------------------------*/
	wrkp.Knd = MlibScalKxgain( Prm->avibdamp, 1, 100, NULL, 24 );

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression low-pass filter gain(Damping frequency[0.1Hz],Time constant correction[0.01ms]) : ResVib.P.Klpf			*/
/*																									*/
/*									         2 * 1000000											*/
/*			Filter time constant[0.01ms] = ----------------------------- + adj[0.01ms]					*/
/*			 					  	       2*PAI * ResFreq[0.1Hz]									*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( 20000000,1, Prm->avibfrq, &sx, 0 );
	wk = MlibPcalKxgain( kx, C10POW7, C2PAIE7, &sx, 24 );

	/* Filter time constant one-parameter calculation result */
	wk2 = MlibKsgain2Long( wk );

	/* Fine adjustment amount addition */
	wk = ((LONG)Prm->avibfil1 * 10) + wk2;
	if( wk < 0 )
	{ /* Lower limit */
		wk = 0;
	}
//	wrkp.Klpf = MlibPcalKf1gain( wk, (ScanTimeNs/1000), 0 );
	wrkp.Klpf = MlibPcalKf1gain( wk, ((ScanTimeNs+500)/1000), 0 );

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression high-pass filter gain(Damping frequency[0.1Hz],Time constant correction[0.01ms]) : ResVib.P.Khpf			*/
/*																									*/
/*									         2 * 100000												*/
/*			Filter time constant[0.01ms] = ----------------------------- + adj[0.01ms]					*/
/*			 					  	       2*PAI * ResFreq[Hz]										*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	wk = (LONG)((SHORT)Prm->avibfil2 * 10) + wk2;
	if( wk < 0 )
	{ /* Lower limit */
		wk = 0;
	}
//	wrkp.Khpf = MlibPcalKf1gain( wk, (ScanTimeNs/1000), 0 );
	wrkp.Khpf = MlibPcalKf1gain( wk, ((ScanTimeNs+500)/1000), 0 );
	wrkp.Khpf1 = MlibPcalKf1gain( (LONG)Prm->avibfrq2, ((ScanTimeNs+500)/1000), 1 );//<S03E>
	wrkp.Knd1 = MlibScalKxgain( Prm->avibdamp2, 1, 100, NULL, 24 );//<S03E>

/*--------------------------------------------------------------------------------------------------*/
/*		Set Parameters																				*/
/*--------------------------------------------------------------------------------------------------*/
	KPI_DI();		/* <S03E> Disable Interrupt	*/
	ResVib->ResVibPrm = wrkp;//<S03E>
	KPI_EI();		/*<S03E>  Enable Interrupt		*/
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Writing to micro IF register															*/
/*--------------------------------------------------------------------------------------------------*/
#if 0/* Interim processingAISCtoMREG_RVKS, so there is no assignment of pending */
	MpReg = &(SvAsicRegs->AsicMicroReg->MREG_RVKS);
	tbuf[0] = (USHORT)(wrkp.Ks);
	tbuf[1] = (USHORT)(wrkp.Ks >> 16);
	tbuf[2] = (USHORT)(wrkp.Kj);
	tbuf[3] = (USHORT)(wrkp.Kj >> 16);
	tbuf[4] = (USHORT)(wrkp.Knd);
	tbuf[5] = (USHORT)(wrkp.Knd >> 16);
	tbuf[6] = (USHORT)(wrkp.Klpf);
	tbuf[7] = (USHORT)(wrkp.Klpf >> 16);
	tbuf[8] = (USHORT)(wrkp.Khpf);
	tbuf[9] = (USHORT)(wrkp.Khpf >> 16);
	MicroTranslatePrmReq( SvAsicRegs, tbuf, MpReg, 10 );
#endif
/*--------------------------------------------------------------------------------------------------*/

	return;

}
#endif /* #if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Vibration detection calculation variable initialization																		*/
/*																									*/
/****************************************************************************************************/
void	DetVibObsInitialize( DETVIB *DetVib )
{
	MlibResetLongMemory( &(DetVib->DetVibObs.var), sizeof(DetVib->DetVibObs.var)/4 );
	return;
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Vibration detection calculation																				*/
/*																									*/
/****************************************************************************************************/
void	DetVibSpeedObserver( DETVIB *DetVib, LONG SpdFbki, LONG TrqRefMon )
{
	DETVIBOBS	*DetVibObs;
	LONG		x1, x2, u1;
	LONG		SpdErr;

	DetVibObs = &(DetVib->DetVibObs);

	/* Averaging process(2Scan) */
	DetVibObs->var.AvSpdFbk = ( SpdFbki + DetVibObs->var.AvSpdFbk ) >> 1;

/*--------------------------------------------------------------------------------------------------*/
/*		Speed observer calculation																			*/
/*--------------------------------------------------------------------------------------------------*/
	u1 = DetVibObs->var.AvSpdFbk - DetVibObs->var.SpdObs;

	x1 = MlibIntegral( u1, DetVibObs->conf.Ks, DetVibObs->var.Ivar64 );
	u1 = x1 + (u1 << 1);

#if (FLOAT_USE==TRUE)
	x1 = ( (float)u1 * DetVibObs->conf.Ks);
	x2 = TrqRefMon * DetVibObs->conf.Kj;
#else
	x1 = MlibMulgainNolim( u1, DetVibObs->conf.Ks );
	x2 = MlibMulgain( TrqRefMon, DetVibObs->conf.Kj );
#endif /* FLOAT_USE */

	DetVibObs->var.SpdObs = MlibLimitul( (DetVibObs->var.SpdObs + x1 + x2), 0x1000000, -0x1000000 );

/*--------------------------------------------------------------------------------------------------*/
/*		Save current value																					*/
/*--------------------------------------------------------------------------------------------------*/
	SpdErr = DetVibObs->var.AvSpdFbk - DetVibObs->var.SpdObs;
	DetVibObs->var.AvSpdErr = ( DetVibObs->var.LastSpdErr + SpdErr ) >> 1;
	DetVibObs->var.LastSpdErr = SpdErr;
	DetVibObs->var.AvSpdFbk = SpdFbki;
	DetVibObs->var.TrcObsAvSpdErr = DetVibObs->var.AvSpdErr;
	DetVibObs->var.AvSpdErrbfFil = DetVibObs->var.AvSpdErr;

	return;
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Maximum and minimum speed deviation setting processing for oscillation detection														*/
/*																									*/
/****************************************************************************************************/
void	DetVibSetSpeedError( DETVIB *DetVib, BOOL TuningLess, BOOL TuningRun )
{
	DETVIBOBS	*DetVibObs;
	DetVibObs = &(DetVib->DetVibObs);

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration detection differential speed maximum/minimum value reset process													*/
/*--------------------------------------------------------------------------------------------------*/
	if( DetVib->DetVibObs.var.SpdErrClrReq == TRUE )
	{
		DetVib->DetVibObs.var.MaxSpdErr = 0;
		DetVib->DetVibObs.var.MinSpdErr = 0;
		DetVib->DetVibObs.var.SpdErrClrReq = FALSE;
	}
	if( DetVib->DetVibObs.var.SpdErrFnClrReq == TRUE )
	{
		DetVib->DetVibObs.var.MaxSpdErrFn = 0;
		DetVib->DetVibObs.var.MinSpdErrFn = 0;
		DetVib->DetVibObs.var.SpdErrFnClrReq = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration(Oscillation)For check --> ScanC:LpxCheckVibration( ), Tuning related						*/
/*--------------------------------------------------------------------------------------------------*/
	if( TuningRun == TRUE )
	{
		DetVibObs->var.AvSpdErr = MlibHpfilter1( DetVibObs->var.AvSpdErr,
												 DetVibObs->conf.Khpf,
												 &(DetVibObs->var.Hpftmp) );
	}
	/*------------------------------------------------------------------------------------------*/
	/*	Vibration detection level processing during adjustment-less control														*/
	/*------------------------------------------------------------------------------------------*/
	if( TuningLess == TRUE )
	{
		DetVibObs->var.AvSpdErr = MlibHpfilter1( DetVibObs->var.AvSpdErr,
												 DetVibObs->conf.Khpf,
												 &(DetVibObs->var.Hpftmp) );
	}
/*--------------------------------------------------------------------------------------------------*/
	if( DetVibObs->var.AvSpdErr > DetVibObs->var.MaxSpdErr )
	{
		DetVibObs->var.MaxSpdErr = DetVibObs->var.AvSpdErr;
	}
	if( DetVibObs->var.AvSpdErr < DetVibObs->var.MinSpdErr )
	{
		DetVibObs->var.MinSpdErr = DetVibObs->var.AvSpdErr;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		For one-parameter tuning alarm														*/
/*--------------------------------------------------------------------------------------------------*/
	DetVibObs->var.AvSpdErrLpfOut =
		MlibLpfilter1( DetVibObs->var.AvSpdErrbfFil, DetVibObs->conf.Klpf, DetVibObs->var.AvSpdErrLpfOut );
/*--------------------------------------------------------------------------------------------------*/
	if( DetVibObs->var.AvSpdErrLpfOut > DetVibObs->var.MaxSpdErrFn )
	{
		DetVibObs->var.MaxSpdErrFn = DetVibObs->var.AvSpdErrLpfOut;
	}
	if( DetVibObs->var.AvSpdErrLpfOut < DetVibObs->var.MinSpdErrFn )
	{
		DetVibObs->var.MinSpdErrFn = DetVibObs->var.AvSpdErrLpfOut;
	}
	return;
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace Peak to Peak For acquisitionAPI															*/
/*																									*/
/****************************************************************************************************/
LONG	DetVibGetSpdErrPeak( DETVIB *DetVib )
{
	return	(DetVib->DetVibObs.var.MaxSpdErr - DetVib->DetVibObs.var.MinSpdErr);
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Vibration detection level For acquisitionAPI																	*/
/*																									*/
/****************************************************************************************************/
LONG	DetVibGetVibCheckLevel( DETVIB *DetVib )
{
	return	(DetVib->conf.VibCheckLevel);
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



/****************************************************************************************************/
/*																									*/
/*		Vibration detection differential speed maximum/minimum value reset	Called from Round, ScanC							*/
/*																									*/
/****************************************************************************************************/
void	DetVibResetMaxMinSpeedError( DETVIB *DetVib )
{
	DetVib->DetVibObs.var.SpdErrClrReq = TRUE;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Vibration detection differential speed maximum/minimum value reset (for Fn)		Called from Round, ScanC				*/
/*																									*/
/****************************************************************************************************/
void	DetVibResetMaxMinSpeedErrorFn( DETVIB *DetVib )
{
	DetVib->DetVibObs.var.SpdErrFnClrReq = TRUE;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace Peak to Peak For acquisitionAPI (for Fn)													*/
/*																									*/
/****************************************************************************************************/
LONG	DetVibGetSpdErrPeakFn( DETVIB *DetVib )
{
	return	(DetVib->DetVibObs.var.MaxSpdErrFn - DetVib->DetVibObs.var.MinSpdErrFn);
}



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Servo control status monitoring variable initialization processing															*/
/*																									*/
/****************************************************************************************************/
void	DetVibInitialize( DETVIB *DetVib )
{
/*--------------------------------------------------------------------------------------------------*/
/*		All variables for servo control status monitoring clear to zero														*/
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( &(DetVib->var), sizeof( DetVib->var )/4  );

	return;
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Servo vibration(Oscillation)Check processing																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitor servo control oscillation and prevent servo control from running out of control.							*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG	DetVibDetectVibration( DETVIB *DetVib )
{
	LONG	Vib;
	LONG	VibStatus;
	VibStatus = DETECT_VIB_NOT_DETECT;

	/* Peak to Peak */
	Vib = DetVib->DetVibObs.var.MaxSpdErr - DetVib->DetVibObs.var.MinSpdErr;

	if( Vib > DetVib->conf.VibCheckLevel )
	{
		if( DetVib->conf.VibCheckSel == 1 )
		{
//			KPI_WARNINGSET( WRN_VIB );					/* A.911 : Vibration(Oscillation)Warning				*/
			VibStatus |= DETECT_VIB_WARNING;
		}
		else if( DetVib->conf.VibCheckSel == 2 )
		{
//			KPI_ALARMSET( ALM_VIB );					/* A.520 : Vibration(Oscillation)Alarm				*/
//			KPI_WARNINGCLR( WRN_VIB );					/* A.911 : Vibration(Oscillation)Warning				*/
			VibStatus |= DETECT_VIB_ALARM;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Advanced autotuning related															*/
/*--------------------------------------------------------------------------------------------------*/
/*			Do not reset while executing advanced autotuning									*/
/*			Reset if the alarm level is exceeded 												*/
/*--------------------------------------------------------------------------------------------------*/
		/* Reset if the alarm level is exceeded */
		DetVibResetMaxMinSpeedError( DetVib );
		/* Vibration detection flag set */
		DetVib->var.VibDetFlg = TRUE;
	}
	else
	{
//		KPI_WARNINGCLR( WRN_VIB );						/* A.911 : Vibration(Oscillation)Warning				*/
		/* Vibration detection flag clear */
		DetVib->var.VibDetFlg = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*			Auto tuning alarm(A.521)Detection processing												*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/* A.521Conditions(Vibration alarm level×Double)is met, count start permission						*/
	/*----------------------------------------------------------------------------------------------*/
	if( Vib > (DetVib->conf.VibCheckLevel << 1) )
	{
		DetVib->var.AatAlmCntFlg = TRUE;
	}
	/*----------------------------------------------------------------------------------------------*/
	/* ATElapsed time count increment for alarm detection												*/
	/*----------------------------------------------------------------------------------------------*/
	if( DetVib->var.AatAlmCntFlg == TRUE )
	{
		DetVib->var.AatAlmCnt++;
	}
	/*----------------------------------------------------------------------------------------------*/
	/* ATElapsed time count for alarm detection 50msPost-detection processing												*/
	/*		A.521If the conditions are met, continue processing											*/
	/*		A.521If the conditions are not met, clear the count										*/
	/*----------------------------------------------------------------------------------------------*/
	if( DetVib->var.AatAlmCnt == KPI_SCANC_MS( 50 ) )
	{
		if( Vib <= (DetVib->conf.VibCheckLevel << 1) )
		{
			DetVib->var.AatAlmCntFlg = FALSE;
			DetVib->var.AatAlmCnt = 0;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/* ATElapsed time count for alarm detection 100msPost-detection processing												*/
	/*		A.521If the conditions are met, an alarm will occur.													*/
	/*		A.521If the conditions are not met, clear the counter										*/
	/*----------------------------------------------------------------------------------------------*/
	else if( DetVib->var.AatAlmCnt == KPI_SCANC_MS( 100 ) )
	{
		DetVib->var.AatAlmCntFlg = FALSE;
		DetVib->var.AatAlmCnt = 0;
	/*----------------------------------------------------------------------------------------------*/
#if 0 /* 2010.11.15 Y.Oka SGDVA.521Output is commented out, so deleted */
		if( Vib > (Gprm.VibCheckLevel<<1) )
		{
			if(BoutV.f.InertiaTuning)
			{
				KPI_ALARMSET( ALM_AT );			/* A.521 : ATAlarm						*/
			}
		}
#endif /* #if 0 */
	}

	return	VibStatus;
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Servo vibration(Oscillation)Check reset processing														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Clear vibration check variables when servo is off.										*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	DetVibResetRequest( DETVIB *DetVib )
{
	DetVib->var.VibDetFlg = FALSE;
	DetVib->var.AatAlmCntFlg = FALSE;
	DetVib->var.AatAlmCnt = 0;
	DetVibResetMaxMinSpeedError( DetVib );
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Servo control oscillation(Low frequency)Monitoring process																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitor low-frequency servo control oscillations such as oscillations caused by position loops,								*/
/*			Prevents runaway servo control.														*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG	DetVibDetectLowFreqVibration( DETVIB *DetVib )
{

	LONG	Vib;
	LONG	VibStatus;
	VibStatus = DETECT_VIB_NOT_DETECT;

	/* Peak to Peak(Right shift uses overflow countermeasures.) */
	DetVib->var.SumSpdErr += (DetVib->DetVibObs.var.MaxSpdErrFn - DetVib->DetVibObs.var.MinSpdErrFn) >> 3;
	DetVib->var.SumSpdErr = MlibLimitul( DetVib->var.SumSpdErr, 0x70000000, 0 );
	Vib = DetVib->var.SumSpdErr;

	/*----------------------------------------------------------------------------------------------*/
	/*	Initialization process																					*/
	/*----------------------------------------------------------------------------------------------*/
	/* Clear vibration level (2ms) */
	DetVibResetMaxMinSpeedErrorFn( DetVib );
	/* Low frequency vibration detection flag clear */
	DetVib->var.LowFreqVibDetFlg = FALSE;

	DetVib->var.LowFreqVibAlmCnt++;
	/*----------------------------------------------------------------------------------------------*/
	/* Elapsed time count for alarm detection 500msPost-detection processing												*/
	/*		If the vibration detection conditions are met, alarm												*/
	/*		If vibration detection conditions are not met, counter clear									*/
	/*	Note) When changing the detection time, be careful of overflow of the integrated value									*/
	/*----------------------------------------------------------------------------------------------*/
	if( DetVib->var.LowFreqVibAlmCnt == KPI_SCANC_MS( 500 ) )
	{
		DetVib->var.LowFreqVibAlmCnt = 0;
		DetVib->var.SumSpdErr = 0;						/* Clear integrated value								*/
		/*------------------------------------------------------------------------------------------*/
		if( Vib > DetVib->conf.JstVibCheckLevel )
		{
			/* 2010.11.15 Y.Oka TuningLessis confirmed by the caller. */
//			if( Gprm.f.TuningLess )						/* Adjustmentless enabled?							*/
//			{
//				KPI_ALARMSET( ALM_AT );					/* A.521 : ATAlarm						*/
				VibStatus = DETECT_VIB_ALARM;
//			}
			DetVib->var.LowFreqVibDetFlg = TRUE;		/* Low frequency vibration detection							*/
		}
	}
	return	VibStatus;
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



#if( CSW_SVCF_VIB_DETECT_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Servo control oscillation(Low frequency)Check reset processing													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Oscillation when servo is off(Low frequency)Clear check variables.								*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	DetVibLowFreqResetRequest( DETVIB *DetVib )
{
	DetVib->var.LowFreqVibDetFlg = FALSE;				/* Low frequency vibration detection flag clear				*/
	DetVib->var.LowFreqVibAlmCnt = 0;					/* Vibration elapsed time counter clear				*/
	DetVib->var.SumSpdErr = 0;							/* Clear integrated value								*/
}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace initialization process																	*/
/*																									*/
/****************************************************************************************************/
void	DetVibTraceInitialize( DETVIB *DetVib )
{
	MlibResetLongMemory( &(DetVib->DetVibTrc), sizeof(DetVib->DetVibTrc)/4 );
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace processing(for Tuning Library)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Tuning Library .						*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	DetVibTraceRuntimeService( DETVIB *DetVib )
{
	LONG		Data;
	DETVIBTRC	*DetVibTrc;
	DetVibTrc = &(DetVib->DetVibTrc);

/*--------------------------------------------------------------------------------------------------*/
/*		Trace execution command check																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( DetVibTrc->TrcCmd != DETVIBTRC_TRCEXE )
	{
		DetVibTrc->TrcIndx = 0;								/* Reset Index							*/
		DetVibTrc->TrcSamp = 1000;							/* Reset TrcSamp for quick start		*/
		return;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Trace sampling control																	*/
/*--------------------------------------------------------------------------------------------------*/
	DetVibTrc->TrcSamp++;
	if( ++DetVibTrc->TrcSamp < DetVibTrc->TrcInt )
	{
		return;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Data trace trigger detection processing																*/
/*--------------------------------------------------------------------------------------------------*/
	if( DetVibTrc->TrcIndx == 0 )
	{
		/* 2^24 --> 2^14(16384) */
		Data = ( (DetVib->DetVibObs.var.TrcObsAvSpdErr) >> 10 );
		/* Limit Trace Data */
		Data = MlibLIMITUL( Data, 16384, -16384 );

		if( MlibAbs32( Data ) >= DetVibTrc->TrcTrgLvl )
		{ /* When exceeding trigger level */
			/* Store Trace Data to TrcBuf */
			DetVibTrc->TrcBufA[DetVibTrc->TrcIndx] = (SHORT)Data;
			DetVibTrc->TrcIndx++;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Data trace execution processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	else if( DetVibTrc->TrcIndx < DETVIBTRC_BUFMAX )
	{ /* Check Index for safety */
		/* 2^24 --> 2^14(16384) */
		Data = ((DetVib->DetVibObs.var.TrcObsAvSpdErr) >> 10);
		/* Limit Trace Data */
		Data = MlibLIMITUL( Data, 16384, -16384 );
		/* Store Trace Data to TrcBuf */
		DetVibTrc->TrcBufA[DetVibTrc->TrcIndx] = (SHORT)Data;
	/*----------------------------------------------------------------------------------------------*/
		if( ++DetVibTrc->TrcIndx < DetVibTrc->TrcNum )
		{
			/* Reset Sampling Counter */
			DetVibTrc->TrcSamp = 0;
		}
	/*----------------------------------------------------------------------------------------------*/
		else
		{ /* Data Trace End */
			DetVibTrc->TrcIndx = 0;								/* Reset Index						*/
			DetVibTrc->TrcSamp = 1000;							/* Reset TrcSamp for quick start	*/
			DetVibTrc->TrcCmd  = DETVIBTRC_TRCEND;				/* Set Trace End to TrcCmd			*/
		}
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace State acquisitionAPI																*/
/*																									*/
/****************************************************************************************************/
UCHAR	DetVibTraceGetStatus( DETVIB *DetVib )
{
	return	DetVib->DetVibTrc.TrcCmd;
}



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace Data buffer address acquisitionAPI												*/
/*																									*/
/****************************************************************************************************/
SHORT*	DetVibTraceGetDataAdr( DETVIB *DetVib )
{
	return	&(DetVib->DetVibTrc.TrcBufA[0]);
}



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace Get indexAPI														*/
/*																									*/
/****************************************************************************************************/
USHORT	DetVibTraceGetIndex( DETVIB *DetVib )
{
	return	DetVib->DetVibTrc.TrcIndx;
}



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace start requestAPI																	*/
/*																									*/
/****************************************************************************************************/
void	DetVibTraceStartReq( DETVIB *DetVib )
{
	DetVib->DetVibTrc.TrcCmd = DETVIBTRC_TRCEXE;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace stop requestAPI																	*/
/*																									*/
/****************************************************************************************************/
void	DetVibTraceStopReq( DETVIB *DetVib )
{
	DetVib->DetVibTrc.TrcCmd = DETVIBTRC_NOCMD;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Speed deviation trace data settingAPI																*/
/*																									*/
/****************************************************************************************************/
void	DetVibTraceSetCondition( DETVIB *DetVib, USHORT TraceNum, UCHAR TraceInt, SHORT TrigerLvl )
{
	DetVib->DetVibTrc.TrcNum = TraceNum;
	DetVib->DetVibTrc.TrcInt = TraceInt;
	DetVib->DetVibTrc.TrcTrgLvl = TrigerLvl;

	return;
}






/****************************************************************************************************/
/*																									*/
/*		Parameter calculation for vibration detection calculation(Initial value)														*/
/*																									*/
/****************************************************************************************************/
void	DetVibObsCalculateInitPrm( DETVIBOBS *DetVibObs, KSGAIN Kvx, ULONG jrate, LONG ScanTimeNs )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Observer gain, low-pass filter gain, high-pass filter gain settings for vibration detection				*/
/*--------------------------------------------------------------------------------------------------*/
	DetVibObsCalculateGains( DetVibObs, VIBOBSGN, DETVIBLP, 0, ScanTimeNs );

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration detection observer inertia gain setting													*/
/*--------------------------------------------------------------------------------------------------*/
	DetVibObsCalculateJGain( DetVibObs, Kvx, jrate, ScanTimeNs );

	return;
}


/****************************************************************************************************/
/*																									*/
/*		Parameter calculation for vibration detection calculation	Called from Round, ScanC									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Set the vibration detection observer gain, low-pass filter gain, and high-pass filter gain.	*/
/*			Ks   : Vibration detection observer gain(80Hz Fixed)											*/
/*			Klpf : Low-pass filter gain for one-parameter vibration detection(3183us = 50Hz Fixed)						*/
/*			Khpf : AATHigh-pass filter gain for vibration detection(Initial value is without filter)						*/
/*																									*/
/****************************************************************************************************/
void	DetVibObsCalculateGains( DETVIBOBS *DetVibObs, LONG ObsGain, LONG Lpf, LONG Hpf, LONG ScanTimeNs )
{
	/* Vibration detection observer gain : Ks */
	DetVibObs->conf.Ks = BpxNorObsGainCal( ObsGain, ScanTimeNs );

	/* Low-pass filter gain for one-parameter vibration detection : Klpf */
	DetVibObs->conf.Klpf = MlibPcalKf1gain( Lpf, ScanTimeNs/1000, 0 );

	/* AATHigh-pass filter gain for vibration detection : Khpf */
	DetVibObs->conf.Khpf = MlibPcalKf1gain( Hpf, ScanTimeNs/1000, 0 );

	return;

}


/****************************************************************************************************/
/*																									*/
/*		Inertia parameter calculation for vibration detection calculation (Called from XlsPrmXXXX.c)							*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Set the observer inertia gain for vibration detection.											*/
/*			Kj	 : Observer inertia gain for vibration detection(100% Fixed)									*/
/*																									*/
/****************************************************************************************************/
void	DetVibObsCalculateJGain( DETVIBOBS *DetVibObs, KSGAIN Kvx, ULONG jrate, LONG ScanTimeNs )
{
	/* Observer inertia gain for vibration detection : Kj */
	DetVibObs->conf.Kj = BpxNorObsJGainCal( Kvx, jrate, 100, ScanTimeNs );
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Initial calculation of vibration detection calculation parameters (for AAT)	Called from Round								*/
/*																									*/
/****************************************************************************************************/
void	DetVibObsCalculateInitPrmAAT( DETVIBOBS *DetVibObs, BOOL TuningLess, LONG ScanTimeNs )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Adjustment-less invalidationHPFNone																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* Iprm.f.TuningLess */
	if( TuningLess == FALSE)
	{ /* Adjustment-less invalidation */
		/* High-pass filter disabled */
		DetVibObsCalculateGains( DetVibObs, VIBOBSGN, DETVIBLP, 0, ScanTimeNs );
	}
	else
	{
/*--------------------------------------------------------------------------------------------------*/
/*		Adjustmentless enabledHPFcan be																		*/
/*--------------------------------------------------------------------------------------------------*/
		/* High-pass filter enabled */
		DetVibObsCalculateGains( DetVibObs, VIBOBSGN, DETVIBLP, DETVIBHP, ScanTimeNs );
	}
	return;
}

/***************************************** end of file **********************************************/

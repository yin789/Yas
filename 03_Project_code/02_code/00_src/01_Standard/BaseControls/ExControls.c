/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Extension control calculation function definition																		*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			1. Adjustment-less control calculation																		*/
/*			2. Disturbance observer(Friction compensation)Operation															*/
/*			3. Phase compensation speed observer calculation															*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2013.08.17	Y.Oka	For Mercury												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "ExControls.h"
#include "BaseControlPrmCalc.h"


/****************************************************************************************************/
/*		Local Function Definition																	*/
/****************************************************************************************************/
/* Adjustment-less robust compensator calculation */
//static	void	tuneLessRobustCompenSater( TUNELESS_DRCOMP *Drcomp, LONG MotSpd, LONG TrqRef );
static void	tuneLessRobustCompenSater( void *pdrcmp, LONG MotSpd, LONG TrqRef );	/* <S1A5> */
/* Adjustment-less phase compensation speed observer calculation */
static void tuneLessEhSpeedObserver( TUNELESS_EHVOBS *EhVobs, LONG MotSpd, LONG TrqRef );

/* <S1A5> Redesigned version of disturbance observer			*/
static void	tuneLessRobustCompensatorEx( void *pprm, LONG MotSpd, LONG TrqRef );
/* <S1A5> Secondary filter calculation(Bilinear transformation) 		*/
static LONG	tuneLessLowPassfilter2Ex( LONG u, LONG k[5], LONG z[4] );



/****************************************************************************************************/
/*																									*/
/*		Initialization of adjustmentless variables																			*/
/*																									*/
/****************************************************************************************************/
void	TuneLessCtrlInitialize( TUNELESS_CTRL *TuneLessCtrl )
{
	/* Disturbance observer variable initialization for no adjustment */
	MlibResetLongMemory( &(TuneLessCtrl->Drcomp.var), sizeof(TuneLessCtrl->Drcomp.var)>>2 );
	/* Transfer compensation speed observer variable initialization for no adjustment */
	MlibResetLongMemory( &(TuneLessCtrl->EhVobs.var), sizeof(TuneLessCtrl->EhVobs.var)>>2 );
	/* Initialization of adjustmentless variables */
	MlibResetLongMemory( &(TuneLessCtrl->var), sizeof(TuneLessCtrl->var)>>2 );
	/* <S1A5> */
	TuneLessInitRobustCompensator( TuneLessCtrl, &(TuneLessCtrl->Drcomp) );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Adjustment-less control																				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview :	Call the robust compensator and phase compensation speed observer for no adjustment						*/
/*																									*/
/*	arg		: LONG	MotSpd					: Motor speed				[2^24/OvrSpd]				*/
/*			  LONG	TrqRefi					: Torque command before filter		[2^24/MaxTrq]				*/
/*			  LONG	TrqRef					: Torque command				[2^24/MaxTrq]				*/
/*	out		: 																						*/
/*	g-out	: LONG	AoutV.TrqFBCx 			: Torque FB compensation			[2^24/MaxTrq]				*/
/*			  LONG	AoutV.SpdFbki			: Speed feedback		[2^24/OvrSpd]				*/
/*			  LONG	BoutV.AmonDisTrq		: Disturbance torque				[2^24/MaxTrq]				*/
/*			  LONG	BoutV.AmonDisTrqCmp 	: Compensation torque				[2^24/MaxTrq]				*/
/*			  LONG	BoutV.AmonSpdObs		: Estimated speed					[2^24/OvrSpd]				*/
/*																									*/
/****************************************************************************************************/
void	TuningLessCtrl( TUNELESS_CTRL *TuneLessCtrl, KSGAIN MotSpd, KSGAIN TrqRefi, KSGAIN TrqRef )			/*<S00A>*/
{
	TUNELESS_DRCOMP		*Drcomp;
	TUNELESS_EHVOBS		*EhVobs;

	Drcomp = &(TuneLessCtrl->Drcomp);
	EhVobs = &(TuneLessCtrl->EhVobs);

/*--------------------------------------------------------------------------------------------------*/
/*		Speed for no adjustmentFBFilter																	*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	TuneLessCtrl->var.SpdFbFilo = FlibLpfilter1( MotSpd, Drcomp->conf.KVfbFil, TuneLessCtrl->var.SpdFbFilo );
#else
	TuneLessCtrl->var.SpdFbFilo = MlibLpfilter1( MotSpd, Drcomp->conf.KVfbFil, TuneLessCtrl->var.SpdFbFilo );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Robust compensator calculation																			*/
/*--------------------------------------------------------------------------------------------------*/
//<S1A5>	tuneLessRobustCompenSater( Drcomp, TuneLessCtrl->var.SpdFbFilo, TrqRef );
		Drcomp->conf.RobustCompensator( Drcomp, TuneLessCtrl->var.SpdFbFilo, TrqRef );	/* <S1A5> */

/*--------------------------------------------------------------------------------------------------*/
/*		Phase compensation speed observer calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( TuneLessCtrl->var.TuneLessAct == FALSE)
	{ /* Adjustment-less invalidation */
		/* With no adjustmentJLis not known, the speed is estimated with zero torque command. */
		tuneLessEhSpeedObserver( EhVobs, MotSpd, 0 );
	}
	else
	{
		tuneLessEhSpeedObserver( EhVobs, MotSpd, TrqRefi );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Adjustmentless enabled/Set invalid state																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( TuneLessCtrl->var.TuneLessAct == TRUE)
	{ /* Adjustmentless enabled */
		TuneLessCtrl->Drcomp.var.CurrentStatus = TRUE;		/* Adjustmentless enabled status setting					*/
	}
	else
	{ /* Adjustment-less invalidation */
		TuneLessCtrl->Drcomp.var.CurrentStatus = FALSE;		/* Adjustment-less invalid state setting					*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		FunctionOFF/ONInitialization process																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( (TuneLessCtrl->Drcomp.var.CurrentStatus == FALSE)		/* Invalid this time							*/
		&& (TuneLessCtrl->Drcomp.var.LastStatus == TRUE) )		/* Last valid							*/
	{ /* Adjustmentless enabled⇒Disabled */
		/* Torque command filter integral initialization request */
		Drcomp->var.TrqFilClrReq = TRUE;
	}
	else if( (TuneLessCtrl->Drcomp.var.CurrentStatus == TRUE)	/* Enabled this time							*/
		&& (TuneLessCtrl->Drcomp.var.LastStatus == FALSE) )		/* Previous invalid							*/
	{ /* Adjustment-less invalidation⇒Enabled */
		/* Torque command filter integral initialization request */
		Drcomp->var.TrqFilClrReq = TRUE;
	}
	else
	{ /* Adjustmentless enabled⇒Enabled or Disabled⇒Disabled */
		/* No torque command filter integral initialization request */
		Drcomp->var.TrqFilClrReq = FALSE;
	}

	/* Function status update without previous adjustment */
	TuneLessCtrl->Drcomp.var.LastStatus = TuneLessCtrl->Drcomp.var.CurrentStatus;

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Adjustment-less robust compensator calculation(SGDVCompatibility method)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview :	Fluctuating torque is estimated based on the torque command and the differential speed, and									*/
/*			is added to the torque command to obtain a new torque command.												*/
/*				Tr1 = Tr0 + Kf * LPF{Tr0 - LPF(J*dωm/dt)}											*/
/*																									*/
/*	arg		: LONG	MotSpd					: Speed feedback		[2^24/OvrSpd]				*/
/*			  LONG	TrqRef					: Torque command				[2^24/MaxTrq]				*/
/*	out		: 																						*/
/*	g-out																							*/
/*																									*/
/****************************************************************************************************/
//<S1A5> static	void	tuneLessRobustCompenSater( TUNELESS_DRCOMP *Drcomp, LONG MotSpd, LONG TrqRef )
void	tuneLessRobustCompenSater( void *pdrcmp, LONG MotSpd, LONG TrqRef )	/* <S1A5> */
{
TUNELESS_DRCOMP *Drcomp = (TUNELESS_DRCOMP *)pdrcmp;
#if (FLOAT_USE==TRUE)
	float	fwk;


	Drcomp->var.VelLpfo2 = FlibLpfilter1( MotSpd, Drcomp->conf.Krlpf2, Drcomp->var.VelLpfo2 );
	Drcomp->var.VelLpfo1 = FlibLpfilter1( Drcomp->var.VelLpfo2, Drcomp->conf.Krlpf1, Drcomp->var.VelLpfo1 );

	fwk = Drcomp->var.VelLpfo2 - Drcomp->var.VelLpfo1;
	fwk = fwk * Drcomp->conf.Krsj;

	/* Torque compensation value[2^24/MaxTrq] */
	Drcomp->var.DisTrq = FlibLimitul( (fwk - Drcomp->var.TrqLpfo_old), 0x1000000, -0x1000000 );
	fwk = Drcomp->var.DisTrq * Drcomp->conf.Krd;

	/* Torque compensation value after filter[2^24/MaxTrq] */
	Drcomp->var.DisTrqCmp = FlibLpfilter1( fwk, Drcomp->conf.KCmpOutlpf, Drcomp->var.DisTrqCmp );

	Drcomp->var.TrqLpfo_old = Drcomp->var.TrqLpfo;
	Drcomp->var.TrqLpfo = FlibLpfilter1( TrqRef, Drcomp->conf.Krlpf1, Drcomp->var.TrqLpfo );

	return;
#else
	LONG	lwk;

	Drcomp->var.VelLpfo2 = MlibLpfilter1( MotSpd, Drcomp->conf.Krlpf2, Drcomp->var.VelLpfo2 );
	Drcomp->var.VelLpfo1 = MlibLpfilter1( Drcomp->var.VelLpfo2, Drcomp->conf.Krlpf1, Drcomp->var.VelLpfo1 );

	lwk = Drcomp->var.VelLpfo2 - Drcomp->var.VelLpfo1;
	lwk = MlibMulgain29( lwk, Drcomp->conf.Krsj );

	/* Torque compensation value[2^24/MaxTrq] */
	Drcomp->var.DisTrq = MlibLimitul( (lwk - Drcomp->var.TrqLpfo_old), 0x1000000, -0x1000000 );
	lwk = MlibMulgainNolim( Drcomp->var.DisTrq, Drcomp->conf.Krd );

	/* Torque compensation value after filter[2^24/MaxTrq] */
	Drcomp->var.DisTrqCmp = MlibLpfilter1( lwk, Drcomp->conf.KCmpOutlpf, Drcomp->var.DisTrqCmp );

	Drcomp->var.TrqLpfo_old = Drcomp->var.TrqLpfo;
	Drcomp->var.TrqLpfo = MlibLpfilter1( TrqRef, Drcomp->conf.Krlpf1, Drcomp->var.TrqLpfo );

	return;
#endif /* FLOAT_USE */
}



/****************************************************************************************************/
/*																									*/
/*		Adjustment-less phase compensation speed observer calculation															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview :	Calculate the estimated speed of the phase compensation speed observer.											*/
/*			Torque filter time constant is 10[μs]In the following cases, it will almost oscillate, so				*/
/*			For now, match the lower limit value to 20[μs]only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.													*/
/*			However, this lower limit value is applied only to the speed observer's ksTf calculation.					*/
/*			Therefore, the torque filter section will follow the parameter settings.								*/
/*																									*/
/*	arg			: MotSpd					: Differential speed				[2^24/nos]						*/
/*				  TrqRef					: Torque command			[2^24/Tqmax]					*/
/*	out			: EhSpdObs					: Estimated speed				[2^24/nos]						*/
/* 	g-out		: 																					*/
/*																									*/
/****************************************************************************************************/
static	void	tuneLessEhSpeedObserver( TUNELESS_EHVOBS *EhVobs, LONG MotSpd, LONG TrqRef )
{
#if (FLOAT_USE==TRUE)
	float	x1, x2, x3, x4, u1;

	/*----------------------------------------------------------------------------------------------*/
	/*		Phase compensation speed observer calculation 																*/
	/*----------------------------------------------------------------------------------------------*/
	u1 = EhVobs->var.EhSpdObs + EhVobs->var.Wkbf2;
	EhVobs->var.Wkbf1 = FlibLpfilter1( u1, EhVobs->conf.Lpf, EhVobs->var.Wkbf1 );

	u1 = MotSpd - EhVobs->var.Wkbf1;

	x1 = u1 * EhVobs->conf.Tf;
	x2 = x1 * EhVobs->conf.Kstf1;
	EhVobs->var.Wkbf2 = x2 - u1;

	x2 = x1 * EhVobs->conf.Kstf2;
	x3 = FlibIntegral( x1, EhVobs->conf.Kstf3, EhVobs->var.Ivar64 );
	x4 = TrqRef * EhVobs->conf.Kj;

	EhVobs->var.EhSpdObs = FlibLimitul( (EhVobs->var.EhSpdObs + x2 + x3 + x4), 0x1000000, -0x1000000 );

	return;
#else
	LONG	x1, x2, x3, x4, u1;

	/*----------------------------------------------------------------------------------------------*/
	/*		Phase compensation speed observer calculation 																*/
	/*----------------------------------------------------------------------------------------------*/
	u1 = EhVobs->var.EhSpdObs + EhVobs->var.Wkbf2;
	EhVobs->var.Wkbf1 = MlibLpfilter1( u1, EhVobs->conf.Lpf, EhVobs->var.Wkbf1 );

	u1 = MotSpd - EhVobs->var.Wkbf1;

	x1 = MlibMulgain32( u1 , EhVobs->conf.Tf );
	x2 = MlibMulgainNolim( x1 , EhVobs->conf.Kstf1 );
	EhVobs->var.Wkbf2 = x2 - u1;

	x2 = MlibMulgainNolim( x1 , EhVobs->conf.Kstf2 );
	x3 = MlibIntegral( x1, EhVobs->conf.Kstf3, EhVobs->var.Ivar64 );
	x4 = MlibMulgain( TrqRef, EhVobs->conf.Kj );

	EhVobs->var.EhSpdObs = MlibLimitul( (EhVobs->var.EhSpdObs + x2 + x3 + x4), 0x1000000, -0x1000000 );

	return;
#endif /* FLOAT_USE */
}



/****************************************************************************************************/
/*																									*/
/*		Disturbance observer variable initialization																	*/
/*																									*/
/****************************************************************************************************/
//<S09B> void	DisturbObserverInitialize( DOBS_CTRL *DobsCtrl )
void	DisturbObserverInitialize( DOBS_CTRL *DobsCtrl, GSELDOBS *pActGains )
{
	/* Disturbance observer variable initialization */
	MlibResetLongMemory( &(DobsCtrl->var), sizeof(DobsCtrl->var)>>2 );
	/* Initialize parameters in the first step1Set gain */
//<S09B>	DobsCtrl->DobsPrm[0] = DobsCtrl->DobsPrm[1];
	DobsCtrl->pDobsPrm = pActGains;			/* <S09B> */
	/* Disturbance observer enable/disable setting */
///*<S00A>*/	DobsCtrl->var.DobsAct = DobsCtrl->var.DobsAct;
	DobsCtrl->var.DobsAct = 0;			/*@@TODOReview initialization <S00A>*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Rigid body model 2nd order disturbance observer calculation															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview :	Configure a second-order disturbance observer and calculate the disturbance torque										*/
/*			Observer calculation is a functionOFF.(Analog monitor also enabled)								*/
/*			FunctionOFF->ONto reduce the shock at switching.									*/
/*																									*/
/*	arg		: LONG	MotSpd				:	Speed feedback	[2^24/OvrSpd]						*/
/*			  LONG	TrqRef				:	Torque command			[2^24/MaxTrq]						*/
/*	out		: 																						*/
/*	g-out	: LONG	AoutV.TrqFBCx 		:	Torque FB compensation		[2^24/MaxTrq]						*/
/*			  LONG	BoutV.AmonDisTrq	:	Disturbance torque			[2^24/MaxTrq]						*/
/*			  LONG	BoutV.AmonDisTrqCmp :	Compensation torque			[2^24/MaxTrq]						*/
/*			  LONG	BoutV.AmonDobSpd	:	Observer estimated speed	[2^24/OvrSpd]						*/
/*																									*/
/****************************************************************************************************/
void	DisturbObserver( DOBS_CTRL *DobsCtrl, KSGAIN MotSpd, KSGAIN TrqRef )				/*<S00A>*/
{
#if (FLOAT_USE==TRUE)
	float	u1, u2;
	float	x1, x2;

	u1 = MotSpd - DobsCtrl->var.SpdObs;

	u2 = FlibIntegral( u1, DobsCtrl->DobsPrm[0].Ks, DobsCtrl->var.Ivar64 );
	u1 = u2 + (u1 * 2);

	x1 = u1 * DobsCtrl->DobsPrm[0].Ks;
	x2 = TrqRef * DobsCtrl->DobsPrm[0].Kj;

	DobsCtrl->var.SpdObs = FlibLimitul( (DobsCtrl->var.SpdObs + x1 + x2), 0x1000000, -0x1000000 );

	DobsCtrl->var.DisTrq = u2 * DobsCtrl->DobsPrm[0].Ksj;

	u2 = DobsCtrl->var.DisTrq;			/*<V043>*/

	DobsCtrl->var.DTrqLpfo = FlibLpfilter1( u2, DobsCtrl->DobsPrm[0].Klpf, DobsCtrl->var.DTrqLpfo );

	DobsCtrl->var.DisTrqCmp = DobsCtrl->var.DTrqLpfo * DobsCtrl->DobsPrm[0].Kd;

	if( DobsCtrl->var.DobsAct == TRUE )
	{ /* When compensation is enabled */
#if	( SVFSEL_ANALOG_FEEDBACK == 1)
		LpxDobsLatchForAnalogfb(DobsCtrl->var.DTrqLpfo);
#endif
		DobsCtrl->var.CurrentStatus = TRUE;								/* Disturbance observer enabled state				*/
	}
	else
	{ /* When compensation is disabled */
		DobsCtrl->var.CurrentStatus = FALSE;								/* Disturbance observer disabled state				*/
	}

#else	
	LONG	u1, u2;
	LONG	x1, x2;
	GSELDOBS *pDobsPrm = DobsCtrl->pDobsPrm;						/* <S09B> */

	u1 = MotSpd - DobsCtrl->var.SpdObs;

	u2 = MlibIntegral( u1, pDobsPrm->Ks, DobsCtrl->var.Ivar64 );	/* <S09B> */
	u1 = u2 + (u1 << 1);

	x1 = MlibMulgainNolim( u1, pDobsPrm->Ks );						/* <S09B> */
	x2 = MlibMulgain( TrqRef, pDobsPrm->Kj );						/* <S09B> */

	DobsCtrl->var.SpdObs = MlibLimitul( (DobsCtrl->var.SpdObs + x1 + x2), 0x1000000, -0x1000000 );

	DobsCtrl->var.DisTrq = MlibMulgain( u2, pDobsPrm->Ksj );		/* <S09B> */

	u2 = DobsCtrl->var.DisTrq;			/*<V043>*/

	DobsCtrl->var.DTrqLpfo = MlibLpfilter1( u2, pDobsPrm->Klpf, DobsCtrl->var.DTrqLpfo );	/* <S09B> */

	DobsCtrl->var.DisTrqCmp = MlibMulgainNolim( DobsCtrl->var.DTrqLpfo, pDobsPrm->Kd );	/* <S09B> */

	if( DobsCtrl->var.DobsAct == TRUE )
	{ /* When compensation is enabled */
#if	( SVFSEL_ANALOG_FEEDBACK == 1)
		LpxDobsLatchForAnalogfb(DobsCtrl->var.DTrqLpfo);
#endif
		DobsCtrl->var.CurrentStatus = TRUE;								/* Disturbance observer enabled state				*/
	}
	else
	{ /* When compensation is disabled */
		DobsCtrl->var.CurrentStatus = FALSE;								/* Disturbance observer disabled state				*/
	}
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*	Speed deviation integral initialization & Torque command filter integrated value initialization process											*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*	Disturbance observer function disabled⇔When switching to enable, initialize integrated value to continuously switch torque			*/
/*																									*/
/*	①Speed deviation integral initialization																			*/
/*		IInitialize integrated value to output component torque with consideration of disturbance compensation torque equivalent						*/
/*		Torque compensation OFF→ONHour:																		*/
/*			Because it is necessary that "torque from speed integral control + torque compensation = previous speed control torque"		*/
/*			Initialize with speed integral torque = speed integral torque - torque compensation. Torque compensation is a minus sign.		*/
/*			Added to speed control torque																*/
/*																									*/
/*		Torque compensation ON→OFFHour:																		*/
/*			Because it is necessary that "torque from speed integral control = previous torque compensation + previous speed control torque"	*/
/*			Initialize as speed integral torque = speed integral torque + torque compensation.								*/
/*																									*/
/*		Caution)Integral()Due to rounding within the function, at initialization×2 is required							*/
/*																									*/
/*	②Torque command filter integral value initialization																*/
/*		The integrated value of the torque command low-pass filter is the torque from the speed control loop before adding torque compensation	*/
/*		Initialize.																				*/
/*																									*/
/*	※The above process changes the control loop variables, so it is executed outside this module.						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	if( (DobsCtrl->var.CurrentStatus == TRUE)				/* Disturbance observer enabled this time				*/
		&& (DobsCtrl->var.LastStatus == FALSE))				/* Previous disturbance observer disabled				*/
	{ /* Disturbance observer disabled⇒Enabled */
		DobsCtrl->var.ExchangeSts = DOBSCHNG_INV2ACT;
	}
	else if((DobsCtrl->var.CurrentStatus == FALSE)			/* Disturbance observer disabled this time				*/
		&& (DobsCtrl->var.LastStatus == TRUE))				/* Previous disturbance observer enabled				*/
	{ /* Disturbance observer enabled⇒Disabled */
		DobsCtrl->var.ExchangeSts = DOBSCHNG_ACT2INV;
	}
	else
	{
		DobsCtrl->var.ExchangeSts = DOBS_NOCHANGE;
	}

	/* Previous disturbance observer function status */
	DobsCtrl->var.LastStatus = DobsCtrl->var.CurrentStatus;

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Phase compensation speed observer variable initialization															*/
/*																									*/
/****************************************************************************************************/
void	EhSpeedObserverInitialize( EHVOBS_CTRL *EhVobsCtrl )
{
	MlibResetLongMemory( &(EhVobsCtrl->var), sizeof(EhVobsCtrl->var)>>2 );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Phase compensation speed observer calculation																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview :	Calculate the estimated speed of the phase compensation speed observer.											*/
/*			Torque filter time constant10μs], it will tend to oscillate, so the lower limit value was set to20[μs]only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.		*/
/*			However, this lower limit value isKsTfApplies only to calculation.							*/
/*			Therefore, the torque filter section operates according to the parameter setting value.								*/
/*																									*/
/*	arg			: MotSpd				Differential speed			[2^24/nos]								*/
/*				  TrqRef				Torque command			[2^24/Tqmax]							*/
/*	out			: EhVobs.V.EhSpdObs		Estimated speed			[2^24/nos]								*/
/* 	g-out		: 																					*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG	EhSpeedObserver( EHVOBS_CTRL *EhVobsCtrl, KSGAIN MotSpd, KSGAIN TrqRef )				/*<S00A>*/
{
#if (FLOAT_USE==TRUE)
	float	x1, x2, x3, x4, u1;

	/*----------------------------------------------------------------------------------------------*/
	/*	Insert a low-pass filter into the torque command input to the observer										*/
	/*----------------------------------------------------------------------------------------------*/
	EhVobsCtrl->var.TrqRefLpfo = FlibLpfilter1( TrqRef,
												EhVobsCtrl->EhVobsPrm.KTrqReflpf,
												EhVobsCtrl->var.TrqRefLpfo );

	/*----------------------------------------------------------------------------------------------*/
	/*		Phase compensation speed observer calculation 																*/
	/*----------------------------------------------------------------------------------------------*/
	u1 = EhVobsCtrl->var.EhSpdObs + EhVobsCtrl->var.Wkbf2;

	EhVobsCtrl->var.Wkbf1 = FlibLpfilter1( u1, EhVobsCtrl->EhVobsPrm.Lpf, EhVobsCtrl->var.Wkbf1 );

	u1 = MotSpd - EhVobsCtrl->var.Wkbf1;

	x1 = u1 * EhVobsCtrl->EhVobsPrm.Tf;

	x2 = x1 * EhVobsCtrl->EhVobsPrm.Kstf1;

	EhVobsCtrl->var.Wkbf2 = x2 - u1;

	x2 = x1 * EhVobsCtrl->EhVobsPrm.Kstf2 ;

	x3 = FlibIntegral( x1, EhVobsCtrl->EhVobsPrm.Kstf3, EhVobsCtrl->var.Ivar64 );

	x4 = EhVobsCtrl->var.TrqRefLpfo * EhVobsCtrl->EhVobsPrm.Kj;

	EhVobsCtrl->var.AccSum = EhVobsCtrl->var.AccSum + (x2 + x3 + x4);
	EhVobsCtrl->var.EhSpdObs = FlibLimitul( EhVobsCtrl->var.AccSum, 0x1000000, -0x1000000 );//Replaced with direct numbers

#if 0
	/*----------------------------------------------------------------------------------------------*/
	/*		Variable output for monitor																		*/
	/*----------------------------------------------------------------------------------------------*/
	BoutV.AmonSpdObs = NORMALIZEINT(EhVobsCtrl->var.EhSpdObs);
#endif

	return( EhVobsCtrl->var.EhSpdObs );

#else
	
	LONG	x1, x2, x3, x4, u1;

	/*----------------------------------------------------------------------------------------------*/
	/*	Insert a low-pass filter into the torque command input to the observer										*/
	/*----------------------------------------------------------------------------------------------*/
	EhVobsCtrl->var.TrqRefLpfo = MlibLpfilter1( TrqRef,
												EhVobsCtrl->EhVobsPrm.KTrqReflpf,
												EhVobsCtrl->var.TrqRefLpfo );

	/*----------------------------------------------------------------------------------------------*/
	/*		Phase compensation speed observer calculation 																*/
	/*----------------------------------------------------------------------------------------------*/
	u1 = EhVobsCtrl->var.EhSpdObs + EhVobsCtrl->var.Wkbf2;

	EhVobsCtrl->var.Wkbf1 = MlibLpfilter1( u1, EhVobsCtrl->EhVobsPrm.Lpf, EhVobsCtrl->var.Wkbf1 );

	u1 = MotSpd - EhVobsCtrl->var.Wkbf1;

	x1 = MlibMulgain32( u1 , EhVobsCtrl->EhVobsPrm.Tf);

	x2 = MlibMulgainNolim( x1 , EhVobsCtrl->EhVobsPrm.Kstf1 );

	EhVobsCtrl->var.Wkbf2 = x2 - u1;

	x2 = MlibMulgainNolim( x1 , EhVobsCtrl->EhVobsPrm.Kstf2 );

	x3 = MlibIntegral( x1, EhVobsCtrl->EhVobsPrm.Kstf3, EhVobsCtrl->var.Ivar64 );

	x4 = MlibMulgain( EhVobsCtrl->var.TrqRefLpfo, EhVobsCtrl->EhVobsPrm.Kj );

	EhVobsCtrl->var.AccSum = EhVobsCtrl->var.AccSum + (x2 + x3 + x4);
	EhVobsCtrl->var.EhSpdObs = MlibLimitul( EhVobsCtrl->var.AccSum, 0x1000000, -0x1000000 );

#if 0
	/*----------------------------------------------------------------------------------------------*/
	/*		Variable output for monitor																		*/
	/*----------------------------------------------------------------------------------------------*/
	BoutV.AmonSpdObs = EhVobsCtrl->var.EhSpdObs;
#endif

	return( EhVobsCtrl->var.EhSpdObs );

#endif /* FLOAT_USE */
}

/*START <S0CD> */
void InitParamFriction( BASE_LOOPCTRLS *BaseLoops  )
{
	BaseLoops->BaseCtrls->FrictionM.TrqGrvLast = BaseLoops->BaseCtrls->FrictionM.GrvTrq;
	BaseLoops->BaseCtrls->FrictionM.TrqCLast = 0;
	BaseLoops->BaseCtrls->FrictionM.InitEnable = FALSE;

}
/*END <S0CD>*/

/*START <S0CD>*/
//#if( FRICTRQ_COMP != 0 )
LONG	LpxMotorFricComp(  BOOL BaseEnable, BASE_LOOPCTRLS *BaseLoops )
{
	LONG Trqfric;
	LONG TrqGravity;
	LONG TrqCoulomb;
	LONG TrqViscous;
	LONG sx;
	LONG kx;
	LONG ky;
	LONG kz;

	/* Uninitialized variable correction<S191> */
	TrqGravity = 0;
	TrqCoulomb = 0;
	TrqViscous = 0;

	if(BaseEnable == FALSE)
		return 0;

	if((BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo > -BaseLoops->BaseCtrls->FrictionM.SpdHys) && (BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo < BaseLoops->BaseCtrls->FrictionM.SpdHys ))
	{
		TrqGravity = BaseLoops->BaseCtrls->FrictionM.TrqGrvLast;
		TrqCoulomb = BaseLoops->BaseCtrls->FrictionM.TrqCLast;
  	    TrqViscous = 0;

	}
	if(BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo >= BaseLoops->BaseCtrls->FrictionM.SpdHys)
	{
		sx = 0;

		TrqGravity = BaseLoops->BaseCtrls->FrictionM.GrvTrq;
		BaseLoops->BaseCtrls->FrictionM.TrqGrvLast = BaseLoops->BaseCtrls->FrictionM.GrvTrq;
		TrqCoulomb = BaseLoops->BaseCtrls->FrictionM.PosCTrq;
		BaseLoops->BaseCtrls->FrictionM.TrqCLast = BaseLoops->BaseCtrls->FrictionM.PosCTrq;

		ky = MlibPcalKxkskx(600000000,BaseLoops->Bprm->RatSpd,C2PAIE7,&sx,-24);
		kz = BprmSpeedLevelCal(BaseLoops->Bprm, ky*10,0);
		kx = MlibPcalKxgain(BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo,1,kz,&sx, 0);
		TrqViscous = MlibPcalKxgain(kx, BaseLoops->BaseCtrls->FrictionM.VisTrq, 1, &sx, -24);
	}
	if(BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo <= -BaseLoops->BaseCtrls->FrictionM.SpdHys)
	{
		sx = 0;
		TrqGravity = 0;
		BaseLoops->BaseCtrls->FrictionM.TrqGrvLast = 0;
		TrqCoulomb = BaseLoops->BaseCtrls->FrictionM.NegCTrq;
		BaseLoops->BaseCtrls->FrictionM.TrqCLast = BaseLoops->BaseCtrls->FrictionM.NegCTrq;

		ky = MlibPcalKxkskx(600000000,BaseLoops->Bprm->RatSpd,C2PAIE7,&sx,-24);
		kz = BprmSpeedLevelCal(BaseLoops->Bprm, ky*10,0);
		kx = MlibPcalKxgain(BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo,1,kz,&sx, 0);
		TrqViscous = MlibPcalKxgain(kx, BaseLoops->BaseCtrls->FrictionM.VisTrq, 1, &sx, -24);
	}

	Trqfric = TrqGravity + TrqViscous + TrqCoulomb;
	return Trqfric;
}

//#endif

/****************************************************************************************************/
/*																									*/
/*		<S1A5> Redesigned version of disturbance observer																*/
/*		No adjustmentstep3																				*/
/*																									*/
/*	Overview:	Fluctuating torque is estimated based on the torque command and the differential speed, and									*/
/*			is added to the torque command to obtain a new torque command.												*/
/*				Tr1 = Tr0 + Kf * LPF{Tr0 - LPF(J*dωm/dt)}											*/
/*																									*/
/* arg		: LONG	MotSpd				:	Speed feedback	[2^24/OvrSpd]						*/
/*			  LONG	TrqRef				:	Torque command			[2^24/MaxTrq]						*/
/* out		: 																						*/
/* g-out																							*/
/*																									*/
/****************************************************************************************************/
void	tuneLessRobustCompensatorEx( void *pprm, LONG MotSpd, LONG TrqRef )
{
	LONG	lwk1, lwk2;
//	LONG	sx1;
	TUNELESS_DRCOMP *pdrcmp = (TUNELESS_DRCOMP *)pprm;
	TL2PRM *pTl2 = pdrcmp->conf.pTl2;

	/*----------------------------------------------------------------------------------------------*/
	/* Torque command side path calculation 																		*/
	/*----------------------------------------------------------------------------------------------*/
	lwk1 = tuneLessLowPassfilter2Ex( TrqRef, pTl2->KrlpfCur, pdrcmp->var.Krlpftmp1 );

	lwk1 = tuneLessLowPassfilter2Ex( lwk1, pTl2->KrlpfDisObs1, pdrcmp->var.Krlpftmp2 );


	/*----------------------------------------------------------------------------------------------*/
	/* Feedback speed side path calculation 																*/
	/*----------------------------------------------------------------------------------------------*/
	pdrcmp->var.VelLpfo1 = MlibLpfilter1( MotSpd, pTl2->KrlpfResSup, pdrcmp->var.VelLpfo1 );

	lwk2 = MotSpd - pdrcmp->var.VelLpfo1;

	lwk2 = lwk2 >> 2;

	lwk2 = pdrcmp->var.VelLpfo1 + lwk2;

	lwk2 = tuneLessLowPassfilter2Ex( lwk2, pTl2->KrlpfDisObs2, pdrcmp->var.Krlpftmp3 );

	lwk2 = MlibMulgain29( lwk2, pdrcmp->conf.Krsj );


	/*----------------------------------------------------------------------------------------------*/
	/* Estimated disturbance torque calculation 																			*/
	/*----------------------------------------------------------------------------------------------*/
	pdrcmp->var.DisTrq = MlibLimitul( (lwk2 - lwk1), 0x1000000, -0x1000000 );

	lwk1 = MlibMulgainNolim( pdrcmp->var.DisTrq, pdrcmp->conf.Krd );

	pdrcmp->var.DisTrqCmp = MlibLpfilter1( lwk1, pdrcmp->conf.KCmpOutlpf, pdrcmp->var.DisTrqCmp );
	/*----------------------------------------------------------------------------------------------*/
	return;
}


/****************************************************************************************************/
/*																									*/
/*		<S1A5> 2nd order filter(Bilinear transformation)																*/
/*		No adjustmentstep3For																				*/
/*																									*/
/****************************************************************************************************/
static LONG tuneLessLowPassfilter2Ex( LONG u, LONG k[5], LONG z[4] )
{
LONG	x[6];

		x[0] = MlibMulgain29( u,    k[0] );
		x[1] = MlibMulgain29( z[0], k[1] );
		x[2] = MlibMulgain29( z[1], k[2] );
		x[3] = MlibMulgain29( z[2], k[3] );
		x[4] = MlibMulgain29( z[3], k[4] );
		x[5] = x[0] + x[1] + x[2] + x[3] - x[4];
/*--------------------------------------------------------------------------------------------------*/
		z[3] = z[2];
		z[2] = x[5];
		z[1] = z[0];
		z[0] = u;

		return( x[5] );

}


/*END <S0CD>*/

/****************************************************************************************************/
/*																									*/
/*		<S1A5> Robust compensator selection																	*/
/*		No adjustmentstep3For																				*/
/*																									*/
/****************************************************************************************************/
void	TuneLessInitRobustCompensator( TUNELESS_CTRL *TuneLessCtrl, TUNELESS_DRCOMP	*Drcomp )
{
	Drcomp->conf.pTl2 = &(TuneLessCtrl->TuneLessPrm.Tl2Prm);
	if ( TuneLessCtrl->conf.TuningLessEx == FALSE )
	{
		Drcomp->conf.RobustCompensator = tuneLessRobustCompenSater;
	}
	else
	{
		Drcomp->conf.RobustCompensator = tuneLessRobustCompensatorEx;
	}
}

/***************************************** end of file **********************************************/

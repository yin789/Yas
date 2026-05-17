/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ScanAMain.c : ScanAMain processing definition															*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : ScanAMain processing																			*/
/*																									*/
/*		ScanBASIC(μProgram).		*/
/*		Execute magnetic pole detection main processing when magnetic pole detection is not completed.											*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "BaseControls.h"
#include "SerialEncIf.h"
#include "PosManager.h"
#include "ResVibCtrl.h"
#include "GainChange.h"
#include "PowerManager.h"
#include "MotorPoleFind.h"
#include "MLib.h"
#include "HwDeviceIf.h"
#include "JL086ServoIpHw.h"
#include "AlarmManager.h"
#include "ExControls.h"
#include "ExCtrlPrmCalc.h"
#include "RippleComp.h"		/* <S1FD> */


static	void	LpxSvPositionManager( BASE_LOOPCTRLS *BaseLoops, POS_MNG_HNDL *PosManager );
static	void	LpxPosSpdFeedbackCal( BASE_LOOPCTRLS *BaseLoops );
static void	ModeSwitch( MODE_SW *ModeSwData, LONG trq_ref, LONG spd_ref, LONG pos_err );
extern void ApxMotorLessTestModeMechaModel( LONG TrqRef, BOOL BeComplete, MOTLESSTST *MotorLessTest );


#define NRFMAX	16014615
/****************************************************************************************************/
/*																									*/
/*		Position control calculation																				*//*<S00A>*/
/*																									*/
/****************************************************************************************************/
static KSGAIN	BasePositionControlA(
			BASE_LOOPCTRLS *BaseLoops,
			KSGAIN	PerClrCmd,			/*<S00A>*/
			KSGAIN	IcvClrCmd,			/*<S00A>*/
			KSGAIN	Kp,					/*<S00A>*/
			LONG	PerClrEdge /* @@ */ )
{
	KSGAIN		SpdRefx;		/* Speed command(For temporary storage) *//*<S00A>*/
	POSCTRL		*PosCtrl;		/* Position control variable */
	POS_MNG_HNDL *PosMngV;

	PosCtrl = &BaseLoops->PosCtrl;
	PosMngV = &BaseLoops->BaseCtrls->PosManager;
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation clear processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( PerClrCmd )
	{
		PosCtrl->V.PerIvar64[0] = 0;
		PosCtrl->V.PerIvar64[1] = 0;
		PosCtrl->V.LastPacOut   = 0;
		return( 0 );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position control integral clear processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( IcvClrCmd )
	{
		PosCtrl->V.PerIvar64[0] = 0;
		PosCtrl->V.PerIvar64[1] = 0;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position control: Position deviation calculation																<S14C>	*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	/*<V739>SafetyModuleCode improvement*/
#if	0 //( CCSW_SFTID == SFTID_EXIST && CCSW_HWID == HWID_SGDV )									  /*<V717>Mod:Start*/
	if (!((BoutV.ZctrlMode.b.zm == CTRLMODE_DSTOP) && (CoutSom.St.PosErrLt == 1)))
	{ PosMngV.PosErr = FlibPerrcalx( dPosRef, dPosFbk, PosMngV.Per64 ); }						  /*<V730>Mod*/
#else
	PosMngV->var.PosErr = FlibPerrcalx( BaseLoops->dPosRefi, BaseLoops->dPosFbki, PosMngV->var.Per64 ); 
#endif	

#else

/*<V739>SafetyModuleCode improvement*/
#if	0 //( CCSW_SFTID == SFTID_EXIST && CCSW_HWID == HWID_SGDV )									  /*<V717>Mod:Start*/
	if (!((BoutV.ZctrlMode.b.zm == CTRLMODE_DSTOP) && (CoutSom.St.PosErrLt == 1)))
	{ PosMngV.PosErr = MlibPerrcalx( dPosRef, dPosFbk, PosMngV.Per64 ); }						  /*<V730>Mod*/
#else
	PosMngV->var.PosErr = MlibPerrcalx( BaseLoops->dPosRefi, BaseLoops->dPosFbki, PosMngV->var.Per64 );
#endif																							  /*<V717>Mod:End  */
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
/*		Position control: Proportional control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	PosCtrl->V.PacOut = PosMngV->var.PosErr * Kp;
#else
	PosCtrl->V.PacOut = MlibMulgain( PosMngV->var.PosErr, Kp );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Position control: Integral control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( (BaseLoops->BaseCtrls)->BeComplete == FALSE )
	{ /* Base block */
		PosCtrl->V.PerIvar64[0] = 0;					/* SGDS, it is adjustable at base block,				*/
		PosCtrl->V.PerIvar64[1] = 0;					/* Only the positional integral is cleared				*/
		PosCtrl->V.IacOut       = 0;
	}
	else if( (BaseLoops->BaseCtrls)->CtrlCmdPrm.Kpi == 0 )
	{ /* When integral gain is zero(Processing time countermeasures) */
#if (FLOAT_USE==TRUE)
		PosCtrl->V.IacOut = ((PosCtrl->V.PerIvar64[1] + 1) / 2.0f);
#else
		PosCtrl->V.IacOut = ((PosCtrl->V.PerIvar64[1] + 1) >> 1);
#endif /* FLOAT_USE */
	}
	else
	{ /* When base enable(Integral gain other than zero) */
#if (FLOAT_USE==TRUE)
		PosCtrl->V.IacOut = FlibIntegral( PosCtrl->V.PacOut,
										  (BaseLoops->BaseCtrls)->CtrlCmdPrm.Kpi,
										  PosCtrl->V.PerIvar64 ); 
#else
		PosCtrl->V.IacOut = MlibIntegral( PosCtrl->V.PacOut,
										  (BaseLoops->BaseCtrls)->CtrlCmdPrm.Kpi,
										  PosCtrl->V.PerIvar64 );
#endif /* FLOAT_USE */
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position control: Differential control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( PosCtrl->P.Kpd == 0 )							/* When the differential gain is zero(Processing time countermeasures)			*/
	{
		PosCtrl->V.DacOut = 0;							/* Differential control output = Zero						*/
	}
	else
	{
#if (FLOAT_USE==TRUE)
		PosCtrl->V.DacOut = (PosCtrl->V.PacOut - PosCtrl->V.LastPacOut) * PosCtrl->P.Kpd;
#else
		PosCtrl->V.DacOut = MlibMulgain( PosCtrl->V.PacOut - PosCtrl->V.LastPacOut, PosCtrl->P.Kpd );
#endif /* FLOAT_USE */
	}
	PosCtrl->V.LastPacOut = PosCtrl->V.PacOut;
/*--------------------------------------------------------------------------------------------------*/
/*		Position control: PID control output																	*/
/*--------------------------------------------------------------------------------------------------*/
	SpdRefx = PosCtrl->V.PacOut + PosCtrl->V.IacOut + PosCtrl->V.DacOut;

/*--------------------------------------------------------------------------------------------------*/
/*		Speed bias addition calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* ★SGDVNot supported★ */
//	SpdRefx = VelocityBias( SpdRefx, PosCtrl->V.PosErr );	/* Speed bias addition calculation					*/
	if( SpdRefx == 0 )
	{
		if( PosMngV->var.PosErr > 0 )		{ SpdRefx =  1;}
		else if( PosMngV->var.PosErr < 0 )	{ SpdRefx = -1;}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position control output limit																		*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	PosCtrl->V.SpdRef = FlibLimitul( SpdRefx, NRFMAX, -NRFMAX );
#else
	PosCtrl->V.SpdRef = MlibLimitul( SpdRefx, NRFMAX, -NRFMAX );
#endif /* FLOAT_USE */

	return( PosCtrl->V.SpdRef );

}

/****************************************************************************************************/
/*																									*/
/*		Speed limit processing with position deviation when servo is on													*/
/*																									*/
/****************************************************************************************************/
static KSGAIN	SvonSpeedLimit( BOOL BaseEnable, BASE_LOOPCTRLS *BaseLoops, KSGAIN SpdRefi )/*<S00A>*/
{
#if (FLOAT_USE==TRUE)
	float	AbsSpdRefi;
	float	SpdRefx;
#else
	LONG	AbsSpdRefi;
	LONG	SpdRefx;
#endif /* FLOAT_USE */
	SPDCTRL *hSpdCtrl;

	hSpdCtrl = &BaseLoops->SpdCtrl;
	SpdRefx = SpdRefi;

	if(((BaseEnable == TRUE) && (hSpdCtrl->V.LastSvonFlag == FALSE))
			|| hSpdCtrl->V.SvonSpdLmtFlag )
	{
		AbsSpdRefi = SpdRefi;
		if( AbsSpdRefi < 0 )
		{
			AbsSpdRefi = -AbsSpdRefi;
		}

		if( AbsSpdRefi > (BaseLoops->BaseCtrls)->CtrlCmdPrm.SvonSpdLmtLevel )
		{ /* Speed command is incorrect at first servo onPn529 */
			/* ServoONHourly speed limit start */
			hSpdCtrl->V.SvonSpdLmtFlag = TRUE;
			if( SpdRefi >= 0 )
			{
				SpdRefx = (BaseLoops->BaseCtrls)->CtrlCmdPrm.SvonSpdLmtLevel;
			}
			else
			{
				SpdRefx = -(BaseLoops->BaseCtrls)->CtrlCmdPrm.SvonSpdLmtLevel;
			}
		}
		else if( AbsSpdRefi < (BaseLoops->BaseCtrls)->CtrlCmdPrm.SvonSpdLmtValue )
		{ /* Speed ​​command isPn529/10 */
			/* ServoONHourly speed limit release */
			hSpdCtrl->V.SvonSpdLmtFlag = FALSE;
		}
	}
	hSpdCtrl->V.LastSvonFlag = BaseEnable;
	return( SpdRefx );
}


/****************************************************************************************************/
/*																									*/
/*		Zero clamp control calculation processing																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Processing performed by zero clamp control mode command from scan C.							*/
/*			Process of clamping the motor by forming a position loop with position command = 0.							*/
/*			In the case of full-closed control, the speed command is calculated using semi-closed conversion.						*/
/*																									*/
/*	Note Intention : Please note that this is motor position control and does not support full closed loop control.			*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static KSGAIN	ZeroPositionControl( BASE_LOOPCTRLS *BaseLoops, KSGAIN dPosFbk )/*<S00A>*/
{
#if (FLOAT_USE==TRUE)
float	SpdRef;
#else
LONG	SpdRef;
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Zero clamp control																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* Position deviation calculation */
	BaseLoops->ZxCtrl.V.PosError -= dPosFbk;
#if (FLOAT_USE==TRUE)
	SpdRef = ( (float)BaseLoops->ZxCtrl.V.PosError * BaseLoops->GselGains->ZcKp );/* Position control calculation(PControl)		*/
#else
	SpdRef = MlibMulgain( BaseLoops->ZxCtrl.V.PosError, BaseLoops->GselGains->ZcKp );	/* Position control calculation(PControl)		*/
#endif /* FLOAT_USE */
	/*----------------------------------------------------------------------------------------------*/
	if( SpdRef == 0 )
	{ /* Correction when speed command is zero */
		if( BaseLoops->ZxCtrl.V.PosError > 0 )
		{
			SpdRef =  1;
		}
		else if( BaseLoops->ZxCtrl.V.PosError < 0 )
		{
			SpdRef = -1;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( SpdRef > BaseLoops->ZxCtrl.P.SpdLimit )
	{ /* Speed command upper limit */
		SpdRef =  BaseLoops->ZxCtrl.P.SpdLimit;
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( SpdRef < -BaseLoops->ZxCtrl.P.SpdLimit )
	{ /* Speed command lower limit */
		SpdRef = -BaseLoops->ZxCtrl.P.SpdLimit;
	}
/*--------------------------------------------------------------------------------------------------*/
	return( SpdRef );

}

/****************************************************************************************************/
/*																									*/
/*		Speed command phase lead compensation calculation when adjustment-less control is enabled												*/
/*																									*/
/****************************************************************************************************/
static KSGAIN	LpxSpdRefPhaseComp(SPDCTRL *SpdCtrl, KSGAIN SpdRefx, KSGAIN SpdRefPhFil, KSGAIN SpdRefPhFilGn)/*<S00A>*/
{
#if (FLOAT_USE==TRUE)
	float	SpdRefComp;
#else
	LONG	SpdRefComp;
#endif /* FLOAT_USE */

	/* Initialization processing at control switching		*//*<V392>*/
	if(SpdCtrl->V.SpdRefFilClrCmd == TRUE)
	{
		/* Speed command phase lead compensation	*/
		SpdCtrl->V.SpdRefPhLpfo = SpdRefx;
		SpdCtrl->V.SpdRefPhHpfTmp = SpdRefx;

		SpdCtrl->V.SpdRefFilClrCmd = FALSE;
	}
	/* Speed command	*/
#if (FLOAT_USE==TRUE)
	SpdCtrl->V.SpdRefPhLpfo = FlibLpfilter1(SpdRefx, SpdRefPhFil, SpdCtrl->V.SpdRefPhLpfo);
	SpdCtrl->V.SpdRefPhHpfo = FlibHpfilter1(SpdRefx, SpdRefPhFil, &SpdCtrl->V.SpdRefPhHpfTmp);
#else
	SpdCtrl->V.SpdRefPhLpfo = MlibLpfilter1(SpdRefx, SpdRefPhFil, SpdCtrl->V.SpdRefPhLpfo);
	SpdCtrl->V.SpdRefPhHpfo = MlibHpfilter1(SpdRefx, SpdRefPhFil, &SpdCtrl->V.SpdRefPhHpfTmp);
#endif /* FLOAT_USE */

#if (FLOAT_USE==TRUE)
	SpdRefComp = SpdCtrl->V.SpdRefPhLpfo + ((LONG)SpdCtrl->V.SpdRefPhHpfo * SpdRefPhFilGn);
#else
	SpdRefComp = SpdCtrl->V.SpdRefPhLpfo + MlibMulgain(SpdCtrl->V.SpdRefPhHpfo, SpdRefPhFilGn);
#endif /* FLOAT_USE */

	return(SpdRefComp);
}

/****************************************************************************************************/
/*																									*/
/*		Speed FF phase lead compensation calculation when adjustment-less control is enabled												*/
/*																									*/
/****************************************************************************************************/
static KSGAIN	LpxSpdFFPhaseComp(SPDCTRL *SpdCtrl, KSGAIN SpdFFC, KSGAIN P_SpdFFC, KSGAIN SpdFFPhFil, KSGAIN SpdFFPhFilGn)/*<S00A>*/
{
#if (FLOAT_USE==TRUE)
	float	SpdFFCComp;
#else
	LONG	SpdFFCComp;
#endif /* FLOAT_USE */

	/* Initialization processing at control switching		*//*<V392>*/
	if(SpdCtrl->V.SpdFFFilClrCmd == TRUE)
	{
		SpdCtrl->V.SpdFFFilClrCmd = FALSE;

		/* Speed FF phase lead compensation	*/
		SpdCtrl->V.SpdFFPhLpfo = P_SpdFFC;
		SpdCtrl->V.SpdFFPhHpfTmp = P_SpdFFC;
	}
	/* Speed FF	*/
#if (FLOAT_USE==TRUE)
	SpdCtrl->V.SpdFFPhLpfo = FlibLpfilter1(SpdFFC, SpdFFPhFil, SpdCtrl->V.SpdFFPhLpfo);
	SpdCtrl->V.SpdFFPhHpfo = FlibHpfilter1(SpdFFC, SpdFFPhFil, &SpdCtrl->V.SpdFFPhHpfTmp);
#else	
	SpdCtrl->V.SpdFFPhLpfo = MlibLpfilter1(SpdFFC, SpdFFPhFil, SpdCtrl->V.SpdFFPhLpfo);
	SpdCtrl->V.SpdFFPhHpfo = MlibHpfilter1(SpdFFC, SpdFFPhFil, &SpdCtrl->V.SpdFFPhHpfTmp);
#endif /* FLOAT_USE */

#if (FLOAT_USE==TRUE)
	SpdFFCComp = SpdCtrl->V.SpdFFPhLpfo + (SpdCtrl->V.SpdFFPhHpfo * SpdFFPhFilGn);
#else
	SpdFFCComp = SpdCtrl->V.SpdFFPhLpfo + MlibMulgain(SpdCtrl->V.SpdFFPhHpfo, SpdFFPhFilGn);
#endif /* FLOAT_USE */

	return(SpdFFCComp);
}


/****************************************************************************************************/
/*																									*/
/*		Speed control calculation																				*/
/*																									*/
/****************************************************************************************************/
static KSGAIN	BaseSpeedControl( BASE_LOOPCTRLS *BaseLoops,
							  KSGAIN SpdFFC,			/* SpeedFFCompensation value									*/
							  KSGAIN SpdFBC,			/* SpeedFBCompensation value									*/
							  BOOL BaseEnable )			/* PositionFBIncrement value (for zero clamp processing)			*/
{
#if (FLOAT_USE==TRUE)
	float	SpdRefx;
	float	d_SpdRefx, d_SpdFFC;
	float	SpdErrP;
	float	SpdErrI;
	float	TrqRef0;		/*<V263>*/
#else
	LONG	SpdRefx;
	LONG	d_SpdRefx, d_SpdFFC;
	LONG	SpdErrP;
	LONG	SpdErrI;
	LONG	TrqRef0;		/*<V263>*/
#endif /* FLOAT_USE */

	BOOL	neri_calc_on;
	POS_MNG_HNDL *PosMngV;

	PosMngV = &BaseLoops->BaseCtrls->PosManager;
/*--------------------------------------------------------------------------------------------------*/
/*		Speed command calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* Servo control command clear request check */
	/* ★Position deviation clear support required★ */
	if( BaseLoops->BaseCtrls->BBRefClrReq )
	{ /* (BaseBlock && OT) */
		/* Clear speed command */
		BaseLoops->SpdRefo = 0;
	}
	else if( (BaseLoops->CtrlMode == CTRLMODE_SPD) || (BaseLoops->CtrlMode == CTRLMODE_JOG) )
	{
		/* Soft start calculation *//* ScanB @@CHECK */
//		BaseLoops->SpdRefo = BaseSoftStartSpdRef( SpdRef, BaseLoops->SpdRefo );
		BaseLoops->SpdRefo = BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo;
	}
	else if( BaseLoops->CtrlMode == CTRLMODE_ZSRCH ) /* <S050> */
	{
		BaseLoops->SpdRefo = BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo;
	}
	else
	{
		/* No soft start during position control */
		BaseLoops->SpdRefo = BaseLoops->SpdRefi;
	}

	BaseLoops->SpdRefSum = MlibSatAdd32(BaseLoops->SpdRefo, BaseLoops->SpdFFCx);

/*--------------------------------------------------------------------------------------------------*/
/*		Processing during base block																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseEnable == FALSE )
	{
		MlibResetLongMemory( &(BaseLoops->SpdCtrl.V), sizeof(BaseLoops->SpdCtrl.V)/4 );
		BaseLoops->ZxCtrl.V.PosError = 0;
		return( 0 );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Zero speed stop, Zero clamp processing																*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseLoops->ZCtrlMode == CTRLMODE_ZSTOP )		/* Zero speed stop								*/
	{
		SpdRefx = 0;
		SpdFFC = 0;
		BaseLoops->TrqFFCx = 0;
	}
/* <S1F5> Start */
	/*----------------------------------------------------------------------------------------------*/
	else if( BaseLoops->ZCtrlMode == CTRLMODE_LDSTOP )	/* Deceleration stop by linear deceleration					*/
	{
		SpdRefx = BaseLoops->BaseCtrls->CtrlCmdMngr.SpdLdstpRefo;
	}
/* <S1F5> End */
	/*----------------------------------------------------------------------------------------------*/
	else if( BaseLoops->ZCtrlMode == CTRLMODE_ZCLMP )	/* Zero clamp								*/
	{
		/* Zero clamp control calculation */
		SpdRefx = ZeroPositionControl( BaseLoops, BaseLoops->dMotPos );
		SpdFFC = 0;
		BaseLoops->TrqFFCx = 0;
	}
	/*----------------------------------------------------------------------------------------------*/
	else												/* Normal control									*/
	{
		SpdRefx = BaseLoops->SpdRefo;
		/* 2012.10.05 Y.Oka SGDV. need? */
//		BaseLoops->ZxCtrl.V.PosError = 0;
	}
	/*----------------------------------------------------------------------------------------------*/
	if( BaseLoops->SpdCtrl.V.SvonSpdLmtFlag )			/* ServoONHourly speed limit					*/
	{
		SpdFFC = 0;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Phase advance filter processing	<V108>																*/
	/*----------------------------------------------------------------------------------------------*/
	d_SpdRefx = LpxSpdRefPhaseComp( &BaseLoops->SpdCtrl, SpdRefx,
									BaseLoops->BaseCtrls->CtrlCmdPrm.TLPrm.SpdRefPhFil,
									BaseLoops->BaseCtrls->CtrlCmdPrm.TLPrm.SpdRefPhFilGn );
	if( BaseLoops->CtrlMode == CTRLMODE_SPD )
	{
		d_SpdFFC = LpxSpdFFPhaseComp( &BaseLoops->SpdCtrl, SpdFFC, 0,
									  BaseLoops->BaseCtrls->CtrlCmdPrm.TLPrm.SpdRefPhFil,
									  BaseLoops->BaseCtrls->CtrlCmdPrm.TLPrm.SpdRefPhFilGn );
	}
	else
	{
		d_SpdFFC = LpxSpdFFPhaseComp( &BaseLoops->SpdCtrl, SpdFFC, SpdFFC,
									  BaseLoops->BaseCtrls->CtrlCmdPrm.TLPrm.SpdRefPhFil,
									  BaseLoops->BaseCtrls->CtrlCmdPrm.TLPrm.SpdRefPhFilGn );
	}
//	if( ((BaseLoops->BaseCtrls)->TuneLessCtrl.var.TuneLessAct) )									  /* <S1C7> */
	if( (TuneLessGetTuneLessSts(&((BaseLoops->BaseCtrls)->TuneLessCtrl)))			/* TuneLessAct	*/
	/*	&& !(TuneLessGetTuneLessModeEx(&((BaseLoops->BaseCtrls)->TuneLessCtrl))) */ )	/* TuningLessEx	*/
	{	/* Adjustmentless enabled && Compatibility mode adjustment-less */
		SpdRefx = d_SpdRefx;
		SpdFFC = d_SpdFFC;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Mode switch calculation	<S0BA>																	*/
/*--------------------------------------------------------------------------------------------------*/
	ModeSwitch(	&BaseLoops->BaseCtrls->ModeSwData,
				BaseLoops->TrqRefo,
				(SpdRefx + SpdFFC),
				PosMngV->var.PosErr );

/*--------------------------------------------------------------------------------------------------*/
/*		Speed control calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( (BaseLoops->ZCtrlMode != CTRLMODE_ZSTOP) && (BaseLoops->ZCtrlMode != CTRLMODE_ZCLMP) )
	{
		BaseLoops->SpdRefSum = MlibSatAdd32(SpdRefx, SpdFFC);
	}

	if( ( (SpdRefx - BaseLoops->SpdObsFbki >= 0) && (BaseLoops->SpdCtrl.V.TrqRef < 0) )
			|| ( (SpdRefx - BaseLoops->SpdObsFbki < 0) && (BaseLoops->SpdCtrl.V.TrqRef > 0) ) )
	{ /* Speed deviation and torque command have different signs */
		neri_calc_on = TRUE;
	}
	else
	{ /* Speed deviation and torque command have the same sign(Torque command = 0Include) */
		neri_calc_on = FALSE;
	}

	if( (BaseLoops->CmdCtrlBit & ENBPCTRL_BIT)
			|| ((BaseLoops->BaseCtrls)->ModeSwData.var.ModeSWFlag
				&& (!BaseLoops->BaseCtrls->CtrlCmdPrm.IPSpdControl)) )
	{ /* During proportional control */
//		BaseLoops->uCycInputs.MREG_BITDAT |= BITDAT_PCONACTFLG;
		BaseLoops->PConActFlg = TRUE; /* <S064> */
#if (FLOAT_USE==TRUE)
		BaseLoops->SpdCtrl.V.SpdFbFilo = FlibLpfilter1(	BaseLoops->SpdObsFbki + SpdFBC, 
															BaseLoops->BaseCtrls->CtrlCmdPrm.KVfbFil, 
															BaseLoops->SpdCtrl.V.SpdFbFilo );
		SpdErrP = SpdRefx - BaseLoops->SpdCtrl.V.SpdFbFilo + SpdFFC;
		BaseLoops->SpdCtrl.V.PacOut = SpdErrP * BaseLoops->GselGains->Kv ;
#else
		BaseLoops->SpdCtrl.V.SpdFbFilo = MlibLpfilter1( BaseLoops->SpdObsFbki + SpdFBC,
														BaseLoops->BaseCtrls->CtrlCmdPrm.KVfbFil,
														BaseLoops->SpdCtrl.V.SpdFbFilo );
		SpdErrP = SpdRefx - BaseLoops->SpdCtrl.V.SpdFbFilo + SpdFFC;
		BaseLoops->SpdCtrl.V.PacOut = MlibMulgain( SpdErrP, BaseLoops->GselGains->Kv );
#endif /* FLOAT_USE */
	/*------------------------------------------------------------------------------------------*/
		if( !BaseLoops->BaseCtrls->CtrlCmdPrm.SpdIctrlKeep )
		{ /* When not speed control integral hold */
			/* Clear the integral with the time constant */
#if (FLOAT_USE==TRUE)
			BaseLoops->SpdCtrl.V.IacOut = FlibLpfilter1( 0.0f,
														 BaseLoops->SpdCtrl.P.KVintegFil,
														 BaseLoops->SpdCtrl.V.IacOut );
			BaseLoops->SpdCtrl.V.Ivar64[1] = BaseLoops->SpdCtrl.V.IacOut *2.0f;
#else
			BaseLoops->SpdCtrl.V.IacOut = MlibLpfilter1( 0,
														 BaseLoops->SpdCtrl.P.KVintegFil,
														 BaseLoops->SpdCtrl.V.IacOut );
			BaseLoops->SpdCtrl.V.Ivar64[1] = BaseLoops->SpdCtrl.V.IacOut << 1;
#endif /* FLOAT_USE */
			BaseLoops->SpdCtrl.V.Ivar64[0] = 0;
		}
	}
	else
	{ /* During proportional integral control */
//		BaseLoops->uCycInputs.MREG_BITDAT &= BITDAT_PCONACTFLG;
		BaseLoops->PConActFlg = FALSE; /* <S064> */
#if (FLOAT_USE==TRUE)
		BaseLoops->SpdCtrl.V.SpdFbFilo = FlibLpfilter1(	BaseLoops->SpdObsFbki + SpdFBC, 
															BaseLoops->BaseCtrls->CtrlCmdPrm.KVfbFil, 
															BaseLoops->SpdCtrl.V.SpdFbFilo );
		SpdErrP = ( SpdRefx * BaseLoops->BaseCtrls->CtrlCmdPrm.PI_rate ) - 
													BaseLoops->SpdCtrl.V.SpdFbFilo + SpdFFC;
		BaseLoops->SpdCtrl.V.PacOut = ( SpdErrP * BaseLoops->GselGains->Kv );
#else
		BaseLoops->SpdCtrl.V.SpdFbFilo = MlibLpfilter1( BaseLoops->SpdObsFbki + SpdFBC,
														BaseLoops->BaseCtrls->CtrlCmdPrm.KVfbFil,
														BaseLoops->SpdCtrl.V.SpdFbFilo );
		SpdErrP = MlibMulgainNolim( SpdRefx, BaseLoops->BaseCtrls->CtrlCmdPrm.PI_rate);
		SpdErrP = SpdErrP - BaseLoops->SpdCtrl.V.SpdFbFilo + SpdFFC;
		BaseLoops->SpdCtrl.V.PacOut = MlibMulgain( SpdErrP, BaseLoops->GselGains->Kv );
#endif /* FLOAT_USE */
	/*------------------------------------------------------------------------------------------*/
		SpdErrI = SpdRefx - BaseLoops->SpdCtrl.V.SpdFbFilo + SpdFFC;
	/*------------------------------------------------------------------------------------------*/
		/* Integrate when torque is not being limited or when speed deviation and torque command have different signs */
		if ( (FALSE == BaseLoops->TrqClamp) || (neri_calc_on == TRUE) )
		{
//#if (FLOAT_USE==TRUE)
//			BaseLoops->SpdCtrl.V.IacOut = FlibIntegral( SpdErrI, BaseLoops->GselGains->Kvi, &BaseLoops->SpdCtrl.V.Ivar64 );
//#else
			BaseLoops->SpdCtrl.V.IacOut = MlibIntegral( SpdErrI, BaseLoops->GselGains->Kvi, BaseLoops->SpdCtrl.V.Ivar64 );
//#endif /* FLOAT_USE */
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Encoderless control										@@@ENCLESS@@@						*/
/*--------------------------------------------------------------------------------------------------*/
//	KpiMencLessSpdCtrl( &BaseLoops->SpdCtrl );				/* MencLessControl: Motor speed control correction		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Speed control output																				*/
/*--------------------------------------------------------------------------------------------------*/
	TrqRef0 = BaseLoops->SpdCtrl.V.PacOut + BaseLoops->SpdCtrl.V.IacOut;
#if (FLOAT_USE==TRUE)
	BaseLoops->SpdCtrl.V.TrqRef = ( TrqRef0 * BaseLoops->GselGains->Kv2 );
#else
	BaseLoops->SpdCtrl.V.TrqRef = MlibMulgain29( TrqRef0, BaseLoops->GselGains->Kv2 );
#endif
	return( BaseLoops->SpdCtrl.V.TrqRef );
}


/****************************************************************************************************/
/*																									*/
/*		Mode switch calculation																			*/
/*																									*/
/****************************************************************************************************/
static void ModeSwitch( MODE_SW *ModeSwData, LONG trq_ref, LONG spd_ref, LONG pos_err )
{
	LONG	ModeSWFlag;
	LONG	wk;

	/* Local variable initialization */
	ModeSWFlag = FALSE;
	wk = 0;

	switch( ModeSwData->conf.ModeSWSel )
	{
	case MODESW_TRQREF: /* Mode switch Torque command */
		wk = trq_ref;
		if( wk < 0 )
		{
			wk = -wk;
		}
		if( wk >= ModeSwData->conf.MSWTrqLevel )
		{
			ModeSWFlag = TRUE;
		}
		break;

	case MODESW_SPDREF: /* Mode switch Speed command */
		wk = spd_ref;
		if( wk < 0 )
		{
			wk = -wk;
		}
		if( wk >= ModeSwData->conf.MSWSpdLevel )
		{
			ModeSWFlag = TRUE;
		}
		break;

	case MODESW_MOTACC: /* Mode switch Acceleration */
		wk = ModeSwData->var.Acceleration;
		if( wk < 0 )
		{
			wk = -wk;
		}
		if( wk >= ModeSwData->conf.MSWAccLevel )
		{
			ModeSWFlag = TRUE;
		}
		break;

	case MODESW_POSERR: /* Mode switch Deviation pulse */
		wk = pos_err;
		if( wk < 0 )
		{
			wk = -wk;
		}
		if( wk >= ModeSwData->conf.MSWErrLevel )
		{
			ModeSWFlag = TRUE;
		}
		break;

	case MODESW_NOTUSE: /* Mode switch Not used */
	default:
		break;
	}
	ModeSwData->var.ModeSWFlag = ModeSWFlag;
}



/****************************************************************************************************/
/*																									*/
/*		Torque filter for Position loop & Speed loop calculation												*/
/*																									*/
/****************************************************************************************************/
static KSGAIN	BaseTorqueFilterForPSCtrl( BASE_LOOPCTRLS *BaseLoops, KSGAIN TrqRefi,
											KSGAIN TrqFBC, KSGAIN TrqFFC, KSGAIN BaseEnable )/*<S00A>*//* <S0B8> */
{
	TRQFIL *TrqFil;
#if (FLOAT_USE==TRUE)
	float	TrqRefx;
	float	DisTrqRef1 = 0;			/* Torque disturbance command input1 */
	float	DisTrqRef2 = 0;			/* Torque disturbance command input2 */
#else
	LONG	TrqRefx;
	LONG	DisTrqRef1 = 0;			/* Torque disturbance command input1 */
	LONG	DisTrqRef2 = 0;			/* Torque disturbance command input2 */
#endif /* FLOAT_USE */

	TrqFil = &BaseLoops->TrqFil;
/*--------------------------------------------------------------------------------------------------*/
/*		Torque disturbance command input processing during table operation														*/
/*--------------------------------------------------------------------------------------------------*/
	BaseLoops->DisTrqRef3 = 0;						/* Torque disturbance command input3							*/
#if (FLOAT_USE==TRUE)
//	switch( TrqFil->V.TrqInputPos ) /* <S0B7> */
	switch( BaseLoops->BaseCtrls->TblDrive.var.DisTrqInTiming )										  /* <S0B7> */
	{
	case 0x01:										/* Before torque command filter							*/
		DisTrqRef1 = (BaseLoops->BaseCtrls->TblDrive.var.TrqRef);
		break;
	case 0x02:										/* After low-pass filter, before notch filter			*/
		DisTrqRef2 = (BaseLoops->BaseCtrls->TblDrive.var.TrqRef);
		break;
	case 0x03:										/* After torque command filter							*/
		BaseLoops->DisTrqRef3 = (BaseLoops->BaseCtrls->TblDrive.var.TrqRef);
		break;
	case 0x00:										/* None											*/
	default :
		break;
	}
#else
	switch( BaseLoops->BaseCtrls->TblDrive.var.DisTrqInTiming )										  /* <S0B7> */
	{
	case 0x01:										/* Before torque command filter							*/
		DisTrqRef1 = BaseLoops->BaseCtrls->TblDrive.var.TrqRef;
		break;
	case 0x02:										/* After low-pass filter, before notch filter			*/
		DisTrqRef2 = BaseLoops->BaseCtrls->TblDrive.var.TrqRef;
		break;
	case 0x03:										/* After torque command filter							*/
		BaseLoops->DisTrqRef3 = BaseLoops->BaseCtrls->TblDrive.var.TrqRef;
		break;
	case 0x00:										/* None											*/
	default :
		break;
	}
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Processing during base block																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseEnable == FALSE )
	{
		TrqFil->V.TrqFilClrCmd = FALSE;				/* Integral initialization command clear						*/
		TrqFil->V.FilOut = 0;
		TrqFil->V.FilOut3 = 0;
		TrqFil->V.LpFil2[0] = 0;
		TrqFil->V.LpFil2[1] = 0;
		TrqFil->V.LpFil2[2] = 0;
		TrqFil->V.LpFil2[3] = 0;
		return( 0 );
	}
#if (FLOAT_USE==TRUE)
	TrqRefx = FlibLimitul( (TrqRefi + DisTrqRef1), (float)0x01000000, (float)-0x01000000 );
#else
	TrqRefx = MlibLimitul( (TrqRefi + DisTrqRef1), 0x01000000, -0x01000000 );
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
/*		1Order low pass filter integral initialization processing(Extension controlON/OFFsometimes used)								*/
/*--------------------------------------------------------------------------------------------------*/
	if(TrqFil->V.TrqFilClrCmd == TRUE)			/* Integral initialization command available							*/
	{
		TrqFil->V.FilOut = TrqRefx;				/* Initialize torque filter integral value with current input value			*/
		TrqFil->V.TrqFilClrCmd = FALSE;			/* Integral initialization command clear							*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		1Order low pass filter processing																		*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	TrqFil->V.FilOut = FlibLpfilter1( TrqRefx, BaseLoops->GselGains->Klpf, TrqFil->V.FilOut );
#else
	TrqFil->V.FilOut = MlibLpfilter1( TrqRefx, BaseLoops->GselGains->Klpf, TrqFil->V.FilOut );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Torque FB compensation process																			*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	TrqRefx = FlibLimitul( (TrqFil->V.FilOut + TrqFFC - TrqFBC + DisTrqRef2), 0x01000000, -0x01000000 );
#else
	TrqRefx = MlibLimitul( (TrqFil->V.FilOut + TrqFFC - TrqFBC + DisTrqRef2), 0x01000000, -0x01000000 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		1Order low pass filter processing(After torque compensation)														*/
/*			Note: The filter after torque compensation worsens the robustness of the adjustment-less filter, so the filter			*/
/* 				  To minimize.																*/
/*--------------------------------------------------------------------------------------------------*/
		if(BaseLoops->BaseCtrls->CtrlCmdPrm.LpassFil3 == TRUE)
		{ /* Torque filter enabled after torque compensation */
			if(BaseLoops->BaseCtrls->TuneLessCtrl.conf.TuningLessUse )
			{ /* Adjustmentless enabled */
#if (FLOAT_USE==TRUE)
				TrqFil->V.FilOut3 = FlibLpfilter1( TrqRefx,
												   (BaseLoops->BaseCtrls)->CtrlCmdPrm.TLPrm.Klpf3,
												   TrqFil->V.FilOut3 );
#else
				TrqFil->V.FilOut3 = MlibLpfilter1( TrqRefx,
												   (BaseLoops->BaseCtrls)->CtrlCmdPrm.TLPrm.Klpf3,
												   TrqFil->V.FilOut3 );
#endif /* FLOAT_USE */
				TrqRefx = TrqFil->V.FilOut3;
			}
			else
			{
				TrqFil->V.FilOut3 = 0;
			}
		}
		else
		{
			TrqFil->V.FilOut3 = 0;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Second-order low-pass filter processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		if(FALSE == BaseLoops->BaseCtrls->CtrlCmdPrm.LpassFil2)
		{
			TrqFil->V.LpFil2[0] = 0;
			TrqFil->V.LpFil2[1] = 0;
			TrqFil->V.LpFil2[2] = 0;
			TrqFil->V.LpFil2[3] = 0;
		}
		else
		{
#if (FLOAT_USE==TRUE)
			TrqRefx = FlibLowPassfilter2( TrqRefx,
										  (BaseLoops->BaseCtrls)->CtrlCmdPrm.Klpf2,
//										  (BaseLoops->BaseCtrls)->CtrlCmdPrm.Klpf2, 	/* S036 */
										  TrqFil->P.Klpf2, 						/* S036 */	/* TODO:Calculation result not confirmed */
										  TrqFil->V.LpFil2 );
#else
			TrqRefx = MlibLowPassfilter2( TrqRefx,
//										  (BaseLoops->BaseCtrls)->CtrlCmdPrm.Klpf2, 	/* S036 */
										  TrqFil->P.Klpf2, 						/* S036 */
										  TrqFil->V.LpFil2 );
#endif /* FLOAT_USE */
		}
/*--------------------------------------------------------------------------------------------------*/
//		BoutV.TrcCompTrqRef = TrqRefx;		/* Torque command before disturbance input									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
		return( FlibLimitul( TrqRefx, (float)0x01000000, (float)-0x01000000 ) );
#else
		return( MlibLimitul( TrqRefx, 0x01000000, -0x01000000 ) );
#endif /* FLOAT_USE */
}



/****************************************************************************************************/
/*																									*/
/*		Torque control command calculation processing		<S0B8>															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Performs torque command input processing and torque command filter processing in torque control mode.					*/
/*																									*/
/*																									*/
/*	Supplementary 1) The variables for torque filter calculation are shared with the basic servo control torque filter processing.					*/
/*			Ensure continuity of torque command when switching control mode.										*/
/*		 2) For OT sequence when OT occurs(ScanC).				*/
/*		 3) Zero speed stop and zero clamp control are left to the basic servo control processing.						*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static LONG	BaseTorqueControl( BASE_LOOPCTRLS *BaseLoops, BOOL BeComplete )
{
	LONG	TrqRefx;
//	LONG	TRefAd;//<S0C7>

/****************************************************************************************************/
/*		Table operation processing																			*/
/****************************************************************************************************/
	if( BaseLoops->BaseCtrls->TblDrive.var.TblCmd != TBLCMD_NOCMD )
	{
	/*----------------------------------------------------------------------------------------------*/
		if( BaseLoops->ZCtrlMode != CTRLMODE_ZNONE )		/* During zero speed control/zero clamp control		*/
		{
			/* Speed control calculation */
			BaseLoops->TrqRefi = BaseSpeedControl( BaseLoops,
												   BaseLoops->SpdFFCx,
												   BaseLoops->SpdFBCx,
												   BeComplete );
			TrqRefx = BaseLoops->TrqRefi;				/* Perform torque command filter processing			*/
		}
	/*----------------------------------------------------------------------------------------------*/
		else if( BeComplete == FALSE )
		{ /* During base block */
			BaseLoops->TrqRefo = 0;						/* Output = Input (No Filter)				*/
			BaseLoops->TrqFil.V.FilOut = 0;
			BaseLoops->TrqFil.V.LpFil2[0] = 0;
			BaseLoops->TrqFil.V.LpFil2[1] = 0;
			BaseLoops->TrqFil.V.LpFil2[2] = 0;
			BaseLoops->TrqFil.V.LpFil2[3] = 0;
			return	BaseLoops->TrqRefo;					/* Do not perform torque command filter processing		*/
		}
	/*----------------------------------------------------------------------------------------------*/
		else
		{
			/* Torque command input = Table operation torque command */
			BaseLoops->TrqRefi = BaseLoops->BaseCtrls->TblDrive.var.TrqRef;
			TrqRefx = BaseLoops->TrqRefi;				/* Perform torque command filter processing			*/
		}
	}

/****************************************************************************************************/
/*		Other than table operation processing																		*/
/****************************************************************************************************/
	else
	{
/*--------------------------------------------------------------------------------------------------*/
/*		Torque command input processing																			*/
/*--------------------------------------------------------------------------------------------------*/
#if	SVFSEL_ADC_USE == 1
		/* Analog command */
//		BaseLoops->TrqRefi = MlibLpfilter1( BaseLoops->TRefAd,
//											BaseLoops->TrqFil.A.Ktrflpf,
//											BaseLoops->TrqRefi );
		if( BaseLoops->MencV->PhaseReady == FALSE )						/* <S21C> */
		{/* Magnetic pole detection in progress */
			BaseLoops->TrqRefi = (BaseLoops->BaseCtrls)->CtrlCmdMngr.TrqRefo;
		}
		else
		{/* Phase detection completed */
			BaseLoops->TrqRefi = MlibLpfilter1( BaseLoops->TRefAd,
												BaseLoops->TrqFil.A.Ktrflpf,
												BaseLoops->TrqRefi );
		}																/* <S21C> */
		
#else
		/* Communication command */
		BaseLoops->TrqRefi = (BaseLoops->BaseCtrls)->CtrlCmdMngr.TrqRefo;
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Speed limit torque subtraction process																		*/
/*--------------------------------------------------------------------------------------------------*/
		TrqRefx = BaseLoops->TrqRefi - (BaseLoops->BaseCtrls)->CtrlCmdMngr.OverTrq;

/*--------------------------------------------------------------------------------------------------*/
/*		Processing during zero speed stop & zero clamp control other than during table operation									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Comment added: Due to the change below, there is no zero speed stop during torque control, so the following process is unnecessary				*/
/*			BKAt SequencePn506≠0However, during torque control, the control has been changed to continue instead of stopping at zero speed.					*/
/*			Even if a zero speed alarm occurs during torque control, it will not stop at zero speed,Pn001.0					*/
/*--------------------------------------------------------------------------------------------------*/
		if( BaseLoops->ZCtrlMode != CTRLMODE_ZNONE )
		{
			/* Return the previous torque command value as is */
			return	BaseLoops->TrqRefo;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Processing during base block																		*/
/*--------------------------------------------------------------------------------------------------*/
		else if( BeComplete == FALSE )
		{
			BaseLoops->TrqRefo = 0;
			if ( BaseLoops->BaseCtrls->BBRefClrReq )
			{
				BaseLoops->TrqRefi = 0;
			}
			BaseLoops->TrqFil.V.FilOut = 0;
			BaseLoops->TrqFil.V.LpFil2[0] = 0;
			BaseLoops->TrqFil.V.LpFil2[1] = 0;
			BaseLoops->TrqFil.V.LpFil2[2] = 0;
			BaseLoops->TrqFil.V.LpFil2[3] = 0;
			return	BaseLoops->TrqRefo;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Processing without filter		<V181>																*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseLoops->TrqFil.V.TrqFilDisable )
	{
		BaseLoops->TrqRefo   = MlibLimitul( TrqRefx, 0x01000000, -0x01000000 );
		BaseLoops->TrqFil.V.FilOut = TrqRefx;
		BaseLoops->TrqFil.V.LpFil2[0] = 0;
		BaseLoops->TrqFil.V.LpFil2[1] = 0;
		BaseLoops->TrqFil.V.LpFil2[2] = 0;
		BaseLoops->TrqFil.V.LpFil2[3] = 0;
		return	BaseLoops->TrqRefo;;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		1st-order low-pass filter integral initialization processing(Extension controlON/OFFsometimes used)								*/
/*--------------------------------------------------------------------------------------------------*/
/*			During torque control LpxInitTrqCtrlModeVar( )to perform initialization processing,							*/
/*			Here, only the integral initialization command is cleared.											*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseLoops->TrqFil.V.TrqFilClrCmd == TRUE )
	{ /* Integral initialization command available */
		/* Integral initialization command clear */
		BaseLoops->TrqFil.V.TrqFilClrCmd = FALSE;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		1st order low pass filter processing																	*/
/*--------------------------------------------------------------------------------------------------*/
	BaseLoops->TrqFil.V.FilOut = MlibLpfilter1( TrqRefx,
												BaseLoops->TrqFil.A.Klpf,
												BaseLoops->TrqFil.V.FilOut );

	TrqRefx = MlibLimitul( BaseLoops->TrqFil.V.FilOut, 0x01000000, -0x01000000 );

/*--------------------------------------------------------------------------------------------------*/
/*		Second-order low-pass filter processing																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseLoops->BaseCtrls->CtrlCmdPrm.LpassFil2 == FALSE )
	{
		BaseLoops->TrqFil.V.LpFil2[0] = 0;
		BaseLoops->TrqFil.V.LpFil2[1] = 0;
		BaseLoops->TrqFil.V.LpFil2[2] = 0;
		BaseLoops->TrqFil.V.LpFil2[3] = 0;
	}
	else
	{
		TrqRefx = MlibLowPassfilter2( TrqRefx,
									  BaseLoops->TrqFil.P.Klpf2,
									  BaseLoops->TrqFil.V.LpFil2 );
	}

	BaseLoops->TrqRefo = MlibLimitul( TrqRefx, 0x01000000, -0x01000000 );

	return	BaseLoops->TrqRefo;

}



/****************************************************************************************************/
/*																									*/
/*		Variable initialization for the first time switching to torque control mode		<S0B8>												*/
/*																									*/
/****************************************************************************************************/
void	BaseInitTrqCtrlVariables( BASE_LOOPCTRLS *BaseLoops, SHORT IqRefMon, LONG *SpdRefo )
{
	if( BaseLoops->LastCtrlMode != CTRLMODE_TRQ )
	{ /* Previous control mode is other than torque control mode */
		/* Torque command for this time = Torque command filter output = PreviousqAxis current command */
		BaseLoops->TrqFil.V.FilOut = MlibMulhigh32( (IqRefMon << 16), 73300775 );
		BaseLoops->TrqRefi = BaseLoops->TrqFil.V.FilOut;

		/* Disturbance input during torque control = 0 */
		BaseLoops->DisTrqRef3 = 0;

		/* Speed command during torque control = 0 */
		*SpdRefo = 0;
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Torque command output : Torque Reference --> ScanA AinK.IqRef/Idref								*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Phase compensation methodA : Conventional method																		*/
/*																									*/
/*		IdRef = 0;																					*/
/*						  15000		TrqRef * 15000 * 2^8											*/
/*		IqRef = TrqRef * ------- = ----------------------											*/
/*						   2^24				2^32													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Phase compensation methodB : IdRef/IqRefMethod																	*/
/*																									*/
/*						  15000		MlibFASTSINS16( Pcmps + 0   )	  TrqRef * 1875 * SinX			*/
/*		IdRef = TrqRef * ------- * ------------------------------- = -----------------------		*/
/*						   2^24				16384							2^32 * 2^3				*/
/*																									*/
/*						  15000		MlibFASTSINS16( Pcmps + 256 )	  TrqRef * 1875 * CosX			*/
/*		IqRef = TrqRef * ------- * ------------------------------- = -----------------------		*/
/*						   2^24				16384							2^32 * 2^3				*/
/*																									*/
/****************************************************************************************************/
static void	KaiOutputTrqRef(BASE_LOOPCTRLS *BaseLoops, KSGAIN TrqRef, KSGAIN IdRef, KSGAIN DisTrq )/*<S00A>*/
{
#if (FLOAT_USE==TRUE)
	float IqRef;
#else
	LONG	IqRef;
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
/*		Current command calculation : 2^24 --> 15000																*/
/*--------------------------------------------------------------------------------------------------*/
#if( SVFSEL_PCMP_IDREF != 1 )						/* Pcmps by MotPhase							*/
	IdRef = MlibMulhigh32( IdRef,  3840000 );		/* IdRef(2^24)  --> IdRef(15000)				*/
	IqRef = MlibMulhigh32( TrqRef, 3840000 );		/* TrqRef(2^24) --> IqRef(15000)				*/
/*--------------------------------------------------------------------------------------------------*/
#else /* ( SVFSEL_PCMP_IDREF == 1 ) */				/* Pcmps by IdRef/IqRef							*/
	LONG	x,y;
	LONG	Pcmps;
	LONG	Pcmpx;
	LONG	Pcmpy;

	Pcmps = MlibABS( AinK.Pcmps );					/* Pcmps [65536/360deg]							*/
	Pcmpx = Pcmps >> 6;								/* Pcmpx [ 1024/360deg]							*/
	Pcmpy = Pcmps & 0x003F;							/* Pcmpy [Rem : 6bit]							*/
	/*----------------------------------------------------------------------------------------------*/
	x = MlibFASTSINS16( Pcmpx );					/* x = Sin( Pcmpx )								*/
	y = MlibFASTSINS16( Pcmpx + 1 );				/* y = Sin( Pcmpx + 1 )							*/
	y = (x + (((y - x) * Pcmpy)>>6)) * 1875;		/* y = (Sin( Pcmpx ) + dx) * 1875				*/
	IdRef = (MlibMulhigh32( TrqRef, y )+4) >> 3;	/* IdRef = TrqRef * Sin( x )					*/
	if( IdRef > 0 )
	{ /* IdRef is Negative Compensation */
		IdRef = -IdRef;
	}
	/*----------------------------------------------------------------------------------------------*/
	x = MlibFASTSINS16( Pcmpx + 256 );				/* x = Cos( Pcmpx )								*/
	y = MlibFASTSINS16( Pcmpx + 256 + 1 );			/* y = Cos( Pcmpx + 1 )							*/
	y = (x + (((y - x) * Pcmpy)>>6)) * 1875;		/* y = (Cos( Pcmpx ) + dx) * 1875				*/
	IqRef = (MlibMulhigh32( TrqRef, y )+4) >> 3;	/* IqRef = TrqRef * Cos( x )					*/
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Current command(dMeasures for relay chatter in the vicinity of the axis &qaxis)Output																		*/
/*--------------------------------------------------------------------------------------------------*/
	BaseLoops->IdRef   = (SHORT)IdRef;				/* IdRef  --> AinK.IdRef						*/
	BaseLoops->IqRef   = (SHORT)IqRef;				/* IqRef  --> AinK.IqRef	 					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Disturbance torque command																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* DisTrq(2^24) --> AinK.IqDist(15000) */
	BaseLoops->IqDist = (SHORT)MlibMulhigh32( DisTrq, 3840000 );
/*--------------------------------------------------------------------------------------------------*/

}


/****************************************************************************************************/
/*																									*/
/*		Yaskawa servo control processing main																	*/
/*																									*/
/****************************************************************************************************/
void ApxYecServoControl( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut ) /* <S0B8> */
{
	LONG			lwk;
	BOOL			BeComplete;
	TUNELESS_CTRL	*TuneLessCtrl;
	DOBS_CTRL		*DobsCtrl;
	RPLCMP			*RippleCmp;		/* <S14B> */
//	BOOL			RippleCmpExe;	/* <S14B> *//* <S1FD> */


	/* Local variable initialization */
	BeComplete		= (BaseLoops->BaseCtrls)->BeComplete;
	TuneLessCtrl	= &(BaseLoops->BaseCtrls->TuneLessCtrl);
	DobsCtrl		= &(BaseLoops->BaseCtrls->DobsCtrl);
	RippleCmp		= &(BaseLoops->RippleCmp);			/* <S14B> */
//	RippleCmpExe	= FALSE;							/* <S14B> *//* <S1FD> */

/*--------------------------------------------------------------------------------------------------*/
/*		Control parameter transfer process																	*//* <S066> */
/*--------------------------------------------------------------------------------------------------*/
	/* Control Parameter Translate */
	if( TR_REQ_SCANA == TranlateCtrlGetReq(&(BaseLoops->BaseCtrls->TransduceCtrl)) )
	{
		TranlateCtrlPrm( &(BaseLoops->BaseCtrls->TransduceCtrl) );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Speed FB calculation, position command creation																	*/
/*--------------------------------------------------------------------------------------------------*/
	LpxPosSpdFeedbackCal( BaseLoops );				/* Motor position & speed FB calculation						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Servo expansion control																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( TRUE == TuneLessGetTuneLessUse(TuneLessCtrl) )
	{ /* Adjustmentless enabled */
		/*------------------------------------------------------------------------------------------*/
		/*	Adjustment-less control --> AoutV.SpdFbki, AoutV.TrqFBCx											*/
		/*------------------------------------------------------------------------------------------*/
		TuningLessCtrl( TuneLessCtrl,
						BaseLoops->SpdFbki,
						BaseLoops->TrqRefi,
						BaseLoops->TrqRefoLimit );

		/*------------------------------------------------------------------------------------------*/
		/*	Acquisition of adjustmentless control calculation results																*/
		/*------------------------------------------------------------------------------------------*/
		if( TRUE == TuneLessGetTuneLessSts(TuneLessCtrl) )
		{ /* Adjustmentless enabled */
			/* Speed feedback[2^24/OvrSpd] */
			BaseLoops->SpdObsFbki = TuneLessGetSpdObsFbk( TuneLessCtrl );
			/* TorqueFBCompensation[2^24/MaxTrq] */
			BaseLoops->TrqFBCx += TuneLessGetDisTrqCmp( TuneLessCtrl );

#if 0 /* 2013.08.16 Y.Oka Analog monitor not supported @@Check */
			/* Estimated speed[2^24/OvrSpd] */
			BoutV.AmonSpdObs = BaseLoops->SpdObsFbki;
			/* Disturbance torque[2^24/MaxTrq] */
			BoutV.AmonDisTrq  = -TuneLessGetDisTrq( TuneLessCtrl );
			/* Compensation torque[2^24/MaxTrq] */
			BoutV.AmonDisTrqCmp = -TuneLessGetDisTrqCmp( TuneLessCtrl );
#endif
		}
		else
		{ /* Adjustment-less invalidation */
			/* Speed feedback[2^24/OvrSpd]No update */
			/* TorqueFBCompensation[2^24/MaxTrq]No update */
			BaseLoops->TrqFil.V.TrqFilClrCmd = TRUE;

#if 0 /* 2013.08.16 Y.Oka Analog monitor not supported @@Check */
			/* Estimated speed[2^24/OvrSpd] */
			BoutV.AmonSpdObs = 0;
			/* Disturbance torque[2^24/MaxTrq] */
			BoutV.AmonDisTrq  = -TuneLessGetDisTrq( TuneLessCtrl );
			/* Compensation torque[2^24/MaxTrq] */
			BoutV.AmonDisTrqCmp = 0;
#endif
		}

		/* Torque command filter integral initialization request */
		if( TRUE == TuneLessGetTFilClrReq(TuneLessCtrl) )
		{
			BaseLoops->TrqFil.V.TrqFilClrCmd = TRUE;
		}
	}
	else
	{ /* Adjustment-less invalidation */
		/*------------------------------------------------------------------------------------------*/
		/*	Disturbance observer calculation --> AoutV.TrqFBCx													*/
		/*------------------------------------------------------------------------------------------*/
		DisturbObserver( DobsCtrl,
						 BaseLoops->SpdObsFbki,
						 BaseLoops->TrqRefoLimit );

		if( TRUE == DobsGetObserverSts(DobsCtrl) )
		{ /* When disturbance observer is enabled */
			BaseLoops->TrqFBCx += DobsGetDisTrqCmp( DobsCtrl );
		}

		switch ( DobsGetExchangeSts(DobsCtrl) )
		{

		case DOBSCHNG_INV2ACT:	/* Disabled⇒Enabled */
			/* Speed integral torque = Speed integral torque - Torque compensation (Torque compensation is added because it is negative) */
#if (FLOAT_USE==TRUE)
			lwk = ((float)DobsCtrl->var.DisTrqCmp * BaseLoops->GselGains->Kv2Inv);
#else
			lwk = MlibMulgain(DobsCtrl->var.DisTrqCmp, BaseLoops->GselGains->Kv2Inv);
#endif /* FLOAT_USE */

			BaseLoops->SpdCtrl.V.Ivar64[1] += (lwk << 1);
			/* Torque command filter integral initialization request */
			BaseLoops->TrqFil.V.TrqFilClrCmd = TRUE;
			break;

		case DOBSCHNG_ACT2INV:	/* Enabled⇒Disabled */
			/* Speed integral torque = Speed integral torque + Torque compensation (Subtracted because torque compensation is negative) */
#if (FLOAT_USE==TRUE)
			lwk = ((float)DobsCtrl->var.DisTrqCmp * BaseLoops->GselGains->Kv2Inv);
#else
			lwk = MlibMulgain(DobsCtrl->var.DisTrqCmp, BaseLoops->GselGains->Kv2Inv);
#endif /* FLOAT_USE */
			BaseLoops->SpdCtrl.V.Ivar64[1] -= (lwk << 1);
			/* Torque command filter integral initialization request */
			BaseLoops->TrqFil.V.TrqFilClrCmd = TRUE;
			break;

		case DOBS_NOCHANGE:		/* No switch */
		default:
			break;
		}

#if 0 /* 2013.08.16 Y.Oka Analog monitor not supported @@Check */
		/*--------------------------------------------------------------------------------------------------*/
		/*		Variable output for monitor				Caution):”Compensation”Sign inversion with disturbance torque to output torque		*/
		/*--------------------------------------------------------------------------------------------------*/
		/* Disturbance observer estimated speed[2^24/OvrSpd] */
		BoutV.AmonDobSpd  = DobsCtrl->var.SpdObs;
		/* Disturbance torque[2^24/MaxTrq] */
		BoutV.AmonDisTrq  = -DobsCtrl->var.DTrqLpfo;
		/* Friction compensation torque[2^24/MaxTrq] */
		BoutV.AmonDisTrqCmp = DobsCtrl->var.DisTrqCmp;
#endif


	}

	/************************************************************************************************/
	/* Control loop calculation																				*/
	/************************************************************************************************/
	switch(BaseLoops->CtrlMode)
	{
	/*----------------------------------------------------------------------------------------------*/
	/* Position control mode & PJOGMode																	*/
	/*----------------------------------------------------------------------------------------------*/
	case CTRLMODE_POS:
		/* Position command update & Position deviation calculation <S14C> */
		LpxSvPositionManager( BaseLoops, &(BaseLoops->BaseCtrls->PosManager) );

		/* Position control calculation processing */
		BaseLoops->SpdRefp = BasePositionControlA( BaseLoops,
												   (BaseLoops->BaseCtrls)->CtrlCmdMngr.PerrClrReq,
												   (BaseLoops->CmdCtrlBit & CLRPOSINTG_BIT),
												   BaseLoops->GselGains->Kp,
												   BaseLoops->f.PerClrEdge /* @@ AP */ );

		/* Speed limit processing when servo is on */
		BaseLoops->SpdRefi = SvonSpeedLimit( BeComplete, BaseLoops, BaseLoops->SpdRefp );

		/* Speed control calculation */
		BaseLoops->TrqRefi = BaseSpeedControl( BaseLoops,
											   BaseLoops->SpdFFCx,
											   BaseLoops->SpdFBCx,
											   BeComplete );
		/* Torque filter */
		BaseLoops->TrqRefo = BaseTorqueFilterForPSCtrl( BaseLoops,									  /* <S0B8> */
														BaseLoops->TrqRefi,
														BaseLoops->TrqFBCx,
														BaseLoops->TrqFFCx,
														BeComplete );

//		RippleCmpExe = FALSE;	/* Disabling the speed ripple compensation function */	/* <S14B> *//*<500>*/

		break;

	/*----------------------------------------------------------------------------------------------*/
	/* Speed control mode & JOGMode & Origin search mode												*/
	/*----------------------------------------------------------------------------------------------*/
	case CTRLMODE_SPD:	/* Speed control mode */
	case CTRLMODE_ZSRCH:	/* @@ */
	case CTRLMODE_JOG:	/* 
						 * @@check JOG MODE
						 * switchJOGSince there is no home return, no torque is generated when the auxiliary function is executed.
						 */
	/*----------------------------------------------------------------------------------------------*/
		/* Speed control calculation */
		BaseLoops->TrqRefi = BaseSpeedControl( BaseLoops,
											   0,
											   BaseLoops->SpdFBCx,
											   BeComplete );
		/* Torque filter */
		BaseLoops->TrqRefo = BaseTorqueFilterForPSCtrl( BaseLoops,									  /* <S0B8> */
														BaseLoops->TrqRefi,
														BaseLoops->TrqFBCx,
														BaseLoops->TrqFFCx,
														BeComplete );
/*<S1FD>*/
#if 0
/* <S14B> Start */
	if( RippleCmp->VelRipplePhaseReady == TRUE)	/* Magnetic pole detection complete(Phase detection completed) */
	{
		if(RippleCmp->RippleCmpThresholdSel == CMD_SPD)
		{	/* Threshold conditions for enabling the speed ripple compensation function = "Command speed" */
			if( RippleCmp->VelRippleCompSpd < BaseLoops->SpdRefo )
			{
				RippleCmpExe = TRUE;
			}
			else
			{
				RippleCmpExe = FALSE;
			}
		}
		else
		{/* Threshold conditions for enabling the speed ripple compensation function = "Feedback speed" */
			if( RippleCmp->VelRippleCompSpd < BaseLoops->SpdFbki )
			{
				RippleCmpExe = TRUE;
			}
			else
			{
				RippleCmpExe = FALSE;
			}
		}
	}
	else	/* Magnetic pole detection not completed(Phase detection incomplete) */
	{
		RippleCmpExe = FALSE;
	}
/* <S14B> End */
#endif
		break;

	/*----------------------------------------------------------------------------------------------*/
	/* Torque control mode																				*/
	/*----------------------------------------------------------------------------------------------*/
	case CTRLMODE_TRQ:	/* Torque control mode *//* <S0B8> */
		/* Torque control mode switching shock reduction processing */
		BaseInitTrqCtrlVariables( BaseLoops,
								  CtrlLoopOut->IqRefMon,
								  &(BaseLoops->BaseCtrls->CtrlCmdMngr.SpdRefo) );
		BaseLoops->TrqRefo = BaseTorqueControl( BaseLoops, BeComplete );
#if 0 /* <S0B8> */
#if	SVFSEL_ADC_USE == 1
		/* <S051> */
		BaseLoops->TrqRefi = MlibLpfilter1( BaseLoops->TRefAd, BaseLoops->TrqFil.A.Ktrflpf /* <S083> Klpf */, BaseLoops->TrqRefi );
		BaseLoops->TrqRefo = BaseLoops->TrqRefi - (BaseLoops->BaseCtrls)->CtrlCmdMngr.OverTrq;
#else
		BaseLoops->TrqRefo = /* BaseLoops->TrqRefi */
				(BaseLoops->BaseCtrls)->CtrlCmdMngr.TrqRefo -		/* @@ */
				(BaseLoops->BaseCtrls)->CtrlCmdMngr.OverTrq;

#endif
#endif /* <S0B8> */

/*<S1FD>*/
#if 0
/* <S14B> Start */
	/* Checking the threshold conditions for enabling the speed ripple compensation function */
	if( RippleCmp->VelRipplePhaseReady == TRUE)	/* Magnetic pole detection complete(Phase detection completed) */
	{
		if( RippleCmp->VelRippleCompSpd < BaseLoops->SpdFbki )
		{
			RippleCmpExe = TRUE;
		}
		else
		{
			RippleCmpExe = FALSE;
		}
	}
	else	/* Magnetic pole detection not completed(Phase detection incomplete) */
	{
		RippleCmpExe = FALSE;
	}
/* <S14B> End */
#endif
		break;

	default:
		break;
	}

	/* Motorless test mode internal mechanical model calculation *//* <S1B7> */
	if( BaseLoops->MotorLessTest->MotorLessTestMode == TRUE )
	{
		ApxMotorLessTestModeMechaModel( BaseLoops->TrqRefoLimit, BeComplete, BaseLoops->MotorLessTest );
	}

/* <S14B> Start */
	/*--------------------------------------------------------------------------------------------------*/
	/*		Speed ripple compensation	--> BaseLoops->TrqRefo														*/
	/*--------------------------------------------------------------------------------------------------*/
	/* Torque command before ripple compensation(Trace analog monitor data)Acquisition */
	RippleCmp->TrqRefBeforeComp = BaseLoops->TrqRefo;

	if(RippleCmp->RplCmpPrm.RplCmpUse == TRUE)	/* Speed ripple compensation function Enabled *//*<S1FD>*/
	{
//		if(  RippleCmpExe == TRUE )	/* Threshold conditions for enabling the speed ripple compensation function Established *//*<S1FD>*/
		{
#if 0	/* Speed ripple compensation Mechanical angle correction processing(Commented out for future use) */
			/* Mechanical angle calculation (Number of encoder bits/360°) */
			lwk = (LONG)((LONG)((BaseLoops->MencV->MotPosX[0]) - (BaseLoops->MencV->MotCposX)) >> BaseLoops->MencV->MposSftR);
			BaseLoops->MencV->MotAngle = ( lwk >= 0 ) ? lwk : lwk + BaseLoops->MencV->PulseNo;

			/* [Encoder bit number/360°] →[65536/360°]*/
			lwk = (BaseLoops->MencV->EncBitNo > 16) ?  (lwk >> ( BaseLoops->MencV->EncBitNo - 16 ))
													  :(lwk << ( 16 - BaseLoops->MencV->EncBitNo ));
#endif
			/* Checking the conditions for enabling the speed ripple compensation function */
			RippleCompChkThresholdVal( 	&(BaseLoops->RippleCmp),
										BaseLoops->CtrlMode,
										BaseLoops->SpdRefo,
										BaseLoops->SpdFbki,
										TuneLessGetTuneLessSts(&((BaseLoops->BaseCtrls)->TuneLessCtrl)));
			/* Speed ripple compensation */
			BaseLoops->TrqRefoComp = RippleCompensation( &(BaseLoops->RippleCmp),
														 0,//lwk,/*<S1FD>*/
														 BaseLoops->MencV->MotPhaseC,
														 BeComplete);

			//RippleCmp->AmonTrqComp1 = BaseLoops->TrqRefoComp;/*<S1FD>*/
			BaseLoops->TrqRefoComp  = BaseLoops->TrqRefoComp + BaseLoops->TrqRefo;
		}
//		else	/* Threshold conditions for enabling the speed ripple compensation function Fail */
//		{
//			BaseLoops->TrqRefoComp = RippleCmp->AmonTrqComp1;
//			BaseLoops->TrqRefoComp =RippleCmp->TrqRefBeforeComp;	/* Speed ripple compensation function Disabled */
//		}
	}
	else
	{
		BaseLoops->TrqRefoComp = BaseLoops->TrqRefo;	/* Speed ripple compensation function Disabled */
	}
/* <S14B> End */

	/*--------------------------------------------------------------------------------------------------*/
	/*		<S0CD> Friction model compensation												*/
	/*--------------------------------------------------------------------------------------------------*/
//#if( FRICTRQ_COMP != 0 )
   BaseLoops->BaseCtrls->TrqMotFricComp = LpxMotorFricComp( BaseLoops->BaseCtrls->FrictionM.MotFricEnable, BaseLoops );
   BaseLoops->TrqRefoComp += BaseLoops->BaseCtrls->TrqMotFricComp;
//#endif
	/*--------------------------------------------------------------------------------------------------*/
	/*		Torque command(CPU)	--> qAxis current command(μProgram)												*/
	/*--------------------------------------------------------------------------------------------------*/
	/* TrqRefo --> AinK.IqRef */
	KaiOutputTrqRef( BaseLoops, BaseLoops->TrqRefoComp, 0, BaseLoops->DisTrqRef3 );

	/*--------------------------------------------------------------------------------------------------*/
	/*		ScanBInformation update processing																			*/
	/*--------------------------------------------------------------------------------------------------*/
	/* Average torque command update(For ScanB) */
#if (FLOAT_USE==TRUE)
	BaseLoops->TrqRefo_a = ( BaseLoops->TrqRefo + BaseLoops->TrqRefo_l ) /2.0f;
#else
	BaseLoops->TrqRefo_a = ( BaseLoops->TrqRefo + BaseLoops->TrqRefo_l ) >> 1;
#endif /* FLOAT_USE */
	BaseLoops->TrqRefo_l = BaseLoops->TrqRefo;

}



/****************************************************************************************************/
/*																									*/
/*		Motor position & speed FB calculation																	*/
/*																									*/
/****************************************************************************************************/
static	void	LpxPosSpdFeedbackCal( BASE_LOOPCTRLS *BaseLoops )
{
	EHVOBS_CTRL		*EhVobsCtrl;

	EhVobsCtrl = &(BaseLoops->BaseCtrls->EhVobsCtrl);

/*--------------------------------------------------------------------------------------------------*/
/*		Position FB update																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* Not compatible with Fulk 2013.08.20 Y.Oka @@CHECK */
//	AoutV.dPosFbki = (Kprm.f.FencUse)? AinK.dFencPos : AinK.dMotPos;
//	BaseLoops->dPosFbki = BaseLoops->dMotPos;/* <S04D> */
	BaseLoops->dPosFbki = (BaseLoops->Bprm->FencUse)? BaseLoops->dFencPos : BaseLoops->dMotPos;/* <S04D> */

/*--------------------------------------------------------------------------------------------------*/
/*		Speed FB update																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* Phase compensation speed observer pre-speed generation */
	/* Not compatible with Fulk 2013.08.20 Y.Oka @@CHECK */
//	AoutV.SpdFbki = ( Iprm.f.PsFulcSelect ) ? AinK.FencSpd : AinK.MotSpd;
//	BaseLoops->SpdFbki = BaseLoops->MotSpd;/* <S04D> */
	BaseLoops->SpdFbki = ( BaseLoops->Bprm->PsFulcSelect ) ? BaseLoops->FencSpd : BaseLoops->MotSpd;

	/* Phase compensation speed observer calculation */
	if( TRUE == EhVobsGetObserverUse(EhVobsCtrl) )
	{
#if( CSW_SVCF_EHVOBS_USE != 0 )
		/* FBSpeed */
		BaseLoops->SpdObsFbki = EhSpeedObserver( EhVobsCtrl,
												 BaseLoops->SpdFbki,
												 BaseLoops->TrqRefi );
#else
		/* FBSpeed */
		BaseLoops->SpdObsFbki = BaseLoops->SpdFbki;
#endif
	}
	else
	{
		/* FBSpeed */
		BaseLoops->SpdObsFbki = BaseLoops->SpdFbki;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Average speed update(For ScanB)																		*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	BaseLoops->MotSpd_a = ( BaseLoops->MotSpd + BaseLoops->MotSpd_l ) / 2.0f;
#else
	BaseLoops->MotSpd_a = ( BaseLoops->MotSpd + BaseLoops->MotSpd_l ) >> 1;
#endif /* FLOAT_USE */
	BaseLoops->MotSpd_l = BaseLoops->MotSpd;

/*	Full Closed Encoder Position																	*//* <S04D> */
#if (FLOAT_USE==TRUE)
	BaseLoops->FencSpd_a = ( BaseLoops->FencSpd + BaseLoops->FencSpd_l ) / 2.0f
#else
	BaseLoops->FencSpd_a = ( BaseLoops->FencSpd + BaseLoops->FencSpd_l ) >> 1;
#endif /* FLOAT_USE */
	BaseLoops->FencSpd_l = BaseLoops->FencSpd;

	
#if (FLOAT_USE==TRUE)
	BaseLoops->SpdObsFbki_a = ( BaseLoops->SpdObsFbki + BaseLoops->SpdObsFbki_l ) /2.0f;
#else
	BaseLoops->SpdObsFbki_a = ( BaseLoops->SpdObsFbki + BaseLoops->SpdObsFbki_l ) >> 1;
#endif /* FLOAT_USE */

	BaseLoops->SpdObsFbki_l = BaseLoops->SpdObsFbki;

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Moving average filter processing for motor speed detection	<V259>	<S02F>										*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : As a countermeasure for reducing speed detection resolution due to speed control cycles, the detected speed is filtered using a moving average filter	*/
/*			. However, the number of moving averages is changed depending on the resolution of the connected encoder.				*/
/*			You can change the moving average number online.BpiChangeSpdDetMaFil( ).				*/
/*																									*/
/****************************************************************************************************/
static KSGAIN	LpxSpdDetMaFilter( MOTSPDMAFIL *SpdMafil, KSGAIN MotSpd )/*<S00A>*/
{
#if (FLOAT_USE==TRUE)
	float	lwk0;
	float	output;
	float	idx_max;
#else
	LONG	lwk0;
	LONG	output;
	LONG	idx_max;
#endif /* FLOAT_USE */

	idx_max = 1 << SpdMafil->manumBit;

	/* Get data in buffer */
	lwk0 = SpdMafil->ksub * SpdMafil->mabuf[SpdMafil->idx];
	/* Storing data this time */
	SpdMafil->mabuf[SpdMafil->idx] = MotSpd;

	SpdMafil->idx++;
	if( SpdMafil->idx >= idx_max )
	{
		/* Reset index */
		SpdMafil->idx = 0;
		SpdMafil->ksub = 1;
	}
	/* Update buffer integrated value */
	SpdMafil->mabufSumx = SpdMafil->mabufSumx - lwk0 + MotSpd;
	/* Output this time */
	output = ( SpdMafil->mabufSumx >> SpdMafil->manumBit );

	return( output );
}



/****************************************************************************************************/
/*																									*/
/*		Kernel Input Service for ScanA																*/
/*																									*/
/****************************************************************************************************/
void	KpxInputScanA( BASE_LOOPCTRLS *BaseLoops )/*<S0C5>*/
{
#if (FLOAT_USE==TRUE)
	float			lwk0;									/* long Work							*/
	float			MotSpd;									/* Motor Speed							*/
	float			dMotPos;
	float			PosTrqLmt;
	float			NegTrqLmt;
#else	
	LONG			lwk0;									/* long Work							*/
	LONG			MotSpd;									/* Motor Speed							*/
	LONG			dMotPos;
	LONG			PosTrqLmt;
	LONG			NegTrqLmt;
	LONG			TuneLessMode;
	BOOL			TuningLessVComp;
	BOOL			VcompSts;
#endif /* FLOAT_USE */
	MENCV			*MencV;
#if	(CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)/* <S04D> */
	MENCV			*FencV;
#endif /* (CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT) */
	BPRMDAT			*Bprm;
	BASE_CTRL_HNDL	*BaseCtrls;		/* <S004> */
	ASIC_HW_IF_STR	*pAsicHwReg;	/* <S004> */
	ALMMAN_HANDLE	*AlmManager;	/* <S018> */
	VFBTIMBASE		*Vfbtimbase;	/* <S067> */

	MencV	= BaseLoops->MencV;
	FencV	= BaseLoops->FencV;
	Bprm	= BaseLoops->Bprm;
	BaseCtrls = BaseLoops->BaseCtrls;		/* <S004> */
	pAsicHwReg = BaseLoops->pAsicHwReg;		/* <S004> */
	AlmManager = BaseLoops->AlmManager;		/* <S018> */
	Vfbtimbase = &(BaseLoops->Vfbtimbase);	/* <S067> */

/*--------------------------------------------------------------------------------------------------*/
/*		Encoder position data reading, Division pulse output processing												*/
/*--------------------------------------------------------------------------------------------------*/
	/* Execute the set encoder position data reading function *//* <S1B7> */
//	SencReadPosition( MencV, pAsicHwReg, AlmManager, BaseCtrls->BeComplete );						  /* <S018> *//* <S0C5> */
	BaseLoops->SencReadPosition( MencV, pAsicHwReg, AlmManager, BaseCtrls->BeComplete, (void*)BaseLoops );
	dMotPos = ( MencV->RcvPosX[0] - MencV->RcvPosX[1] ) >> MencV->MposSftR;

/*--------------------------------------------------------------------------------------------------*/
/*		Speed correction using change time timer																*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Normalized based on differential speed[pulse/ScanA]→[2^24/OvrSpd]												*/
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	MotSpd = dMotPos * (Bprm->KmotspdA);
#else
	MotSpd = MlibMulgain( dMotPos, Bprm->KmotspdA );					/* Motor Speed[2^24/OvrSpd]	*/
#endif /* FLOAT_USE */

	/*----------------------------------------------------------------------------------------------*/
	/*		Speed correction internal status acquisition	<S067>															*/
	/*----------------------------------------------------------------------------------------------*/
	TuneLessMode    = BaseLoops->BaseCtrls->TuneLessCtrl.conf.TuningLessMode;
	TuningLessVComp = BaseLoops->BaseCtrls->TuneLessCtrl.conf.TuningLessVComp;
	VcompSts = ( (TuneLessMode & TLMODE_SPDDETCHNG) && (TuningLessVComp == FALSE) ) ? FALSE : TRUE;
	/*----------------------------------------------------------------------------------------------*/
	/*		Speed correction validity judgment		<S067>															*/
	/*----------------------------------------------------------------------------------------------*/
	if( MencV->SpdCmpEnable == FALSE )
	{
		MencV->MotSpd = TimeBasesMotSpdCalc( Vfbtimbase, MencV, dMotPos, MotSpd, VcompSts, FALSE );
		MencV->SpdCmpEnable = TRUE;
	}
	else
	{
		MencV->MotSpd = TimeBasesMotSpdCalc( Vfbtimbase, MencV, dMotPos, MotSpd, VcompSts, TRUE );
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Speed detection moving average filter processing		<V110>													*/
	/*----------------------------------------------------------------------------------------------*/
	if( BaseLoops->MotSpdMafil->manumBit == 0 )
	{ /* Encoder17bitor more */
		MotSpd = MencV->MotSpd;
	}
	else
	{ /* Encoder16bit */
		MotSpd = LpxSpdDetMaFilter( BaseLoops->MotSpdMafil, MencV->MotSpd );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate Motor Position and Speed															*/
/*--------------------------------------------------------------------------------------------------*/
	/* Cal. Delta Position */
	lwk0 = MencV->MotPos - BaseLoops->LastMotPos;
	/* Update Motor Position */
	BaseLoops->LastMotPos = MencV->MotPos;

//<S00E>Commonization#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE				/* M2,M3,etc */
//	lwk0 = Bprm->DirSign * lwk0;
//	BaseLoops->MotPos += lwk0;							/* Update Motor Position		*/
//	BaseLoops->dMotPos = lwk0;							/* Update Motor Delta Position	*/
//	BaseLoops->MotSpd  = MotSpd;						/* Motor Speed for ScanB		*/
//<S00E>Commonization#else													/* Analog & Pulse */
	{ 
	CTRL_CMD_MNG *pcmd = &(BaseLoops->BaseCtrls->CtrlCmdMngr);	
	UINT PerClrEdge = ( pcmd->PerClrEdge == TRUE) && ( BaseLoops->f.PastInputClrEdge == FALSE );
	
	if( Bprm->RvsDir )									/* Reverse rotation mode							*/
	{
		BaseLoops->MotPos -=  lwk0;						/* Update Motor Position				*/
		if(	PerClrEdge && (Bprm->FencUse == FALSE) )
		{
			BaseLoops->dMotPos = BaseLoops->MotPos - pcmd->ClrFbPos;		//<V584>
		}
		else
		{
			BaseLoops->dMotPos = -lwk0;
		}
		BaseLoops->MotSpd  = -MotSpd;					/* Motor Speed for ScanB [2^24/OvrSpd]	*/
	}
	else												/* Positive rotation mode							*/
	{
		BaseLoops->MotPos += lwk0;						/* Update Motor Position				*/
		if(	PerClrEdge && (Bprm->FencUse == FALSE) )
		{
			BaseLoops->dMotPos = BaseLoops->MotPos - pcmd->ClrFbPos;
		}
		else
		{
			BaseLoops->dMotPos = lwk0;
		}

		BaseLoops->MotSpd  = MotSpd;					/* Motor Speed for ScanB [2^24/OvrSpd]	*/
	}
	/*----------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Input Full Closed Encoder Position															*/
/*--------------------------------------------------------------------------------------------------*//* <S04D> */
#if	(CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)
	if( Bprm->FencUse )										/* When using Fulque encoder				*/
	{
		if(Bprm->RvsDir )									/* Reverse rotation mode							*/
		{
//			lwk0 = -FencReadPosition( FencV, AlmManager, BaseCtrls->BeComplete );	/* <S0E0>:AlmManageraddition */
			lwk0 = -BaseLoops->FencReadPosition( FencV, AlmManager, BaseCtrls->BeComplete, (void*)BaseLoops );	/*<S1B7>*/
															/* Fulque encoder position data read		*/
			BaseLoops->FencSpd = -FencV->MotSpd;			/* Load Speed [2^24/OvrSpd]				*/
		}
		else												/* Positive rotation mode							*/
		{
//			lwk0 = FencReadPosition( FencV, AlmManager, BaseCtrls->BeComplete );	/* <S0E0>:AlmManageraddition */
			lwk0 = BaseLoops->FencReadPosition( FencV, AlmManager, BaseCtrls->BeComplete, (void*)BaseLoops );	/*<S1B7>*/
															/* Fulque encoder position data read		*/
			BaseLoops->FencSpd = FencV->MotSpd;				/* Load Speed [2^24/OvrSpd]				*/
		}
		/*------------------------------------------------------------------------------------------*/
		BaseLoops->FencPos += lwk0;							/* Update Fencoder Position				*/
		if(	PerClrEdge )
		{
			BaseLoops->dFencPos = BaseLoops->FencPos - pcmd->ClrFbPos;
		}
		else
		{
			BaseLoops->dFencPos = lwk0;
		}
		/*------------------------------------------------------------------------------------------*/
	}
#endif/* 	(CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)	*/
	BaseLoops->f.PastInputClrEdge = pcmd->PerClrEdge;		/* Deviation edge clear input retain previous value		*/
	BaseLoops->f.PerClrEdge = PerClrEdge;					/* Perr Clear Edge Command Input */
	/* 
	 * @@ AP
	 */
	}

//#endif	<S00E>Commonization

/*--------------------------------------------------------------------------------------------------*/
/*		Torque Reference after Torque Limit for Observer											*/
/*--------------------------------------------------------------------------------------------------*/
	PosTrqLmt = (BaseLoops->BaseCtrls)->TrqLimitData.var.PosTrqLmtOut;
	NegTrqLmt = (BaseLoops->BaseCtrls)->TrqLimitData.var.NegTrqLmtOut;
	if( BaseLoops->TrqRefo > PosTrqLmt )
	{
		BaseLoops->TrqRefoLimit = PosTrqLmt;
	}
	else if( BaseLoops->TrqRefo < NegTrqLmt )
	{
		BaseLoops->TrqRefoLimit = NegTrqLmt;
	}
	else
	{
		BaseLoops->TrqRefoLimit = BaseLoops->TrqRefo;
	}

	return;/*<S0C5>*/
}



/****************************************************************************
  Carrer frequency change Module
*****************************************************************************/
static void	BaseLoopChangeCarrerFreq( BASE_LOOPCTRLS *BaseLoops,
									  CTRL_LOOP_OUT *CtrlLoopOut,
									  ASICS *SvAsicRegs )
{
	CARRCHNG	*CarrerChange;
#if (FLOAT_USE==TRUE)
	float		AbsMotSpd;		/* FBSpeed absolute value */
	float		MotIqref2;		/* qAxis current command^2 */
	float		MotIqfb;		/* qAxis current detection value */
	float		MotIqfb2;		/* qAxis current detection value^2 */
	float		MotIdfb;		/* dAxis current detection value */
	float		MotIdfb2;		/* dAxis current detection value^2 */
	float		MotIfb2;		/* (qAxis current detection value^2) + (dAxis current detection value^2) */
	float		MotIq;			/* Max(qAxis current command^2, |qAxis currentFBVector|^2) */
#else
	LONG		AbsMotSpd;		/* FBSpeed absolute value */
	LONG		MotIqref2;		/* qAxis current command^2 */
	LONG		MotIqfb;		/* qAxis current detection value */
	LONG		MotIqfb2;		/* qAxis current detection value^2 */
	LONG		MotIdfb;		/* dAxis current detection value */
	LONG		MotIdfb2;		/* dAxis current detection value^2 */
	LONG		MotIfb2;		/* (qAxis current detection value^2) + (dAxis current detection value^2) */
	LONG		MotIq;			/* Max(qAxis current command^2, |qAxis currentFBVector|^2) */
#endif /* FLOAT_USE */

	CarrerChange = &(BaseLoops->CarrerChange);

/*--------------------------------------------------------------------------------------------------*/
/*		Carrier frequency switching processing																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( CarrerChange->conf.CarrChange )					/* Carrier switching flagON							*/
	{
		/*---------- Absolute value calculation process ----------------------------------------------------------------*/
		/* Current command absolute value calculation	*/
		MotIqref2 = (LONG)BaseLoops->IqRef * (LONG)BaseLoops->IqRef;

		/* CurrentFBAbsolute value calculation		*/
//		MotIqfb = KPX_INPUT_IQMON( );
		MotIqfb = CtrlLoopOut->IqRefMon;
		MotIqfb2 = (LONG)MotIqfb * (LONG)MotIqfb;
//		MotIdfb = KPX_INPUT_IDMON( );
		MotIdfb = CtrlLoopOut->IdRefMon;
		MotIdfb2 = (LONG)MotIdfb * (LONG)MotIdfb;
		MotIfb2 = MotIqfb2 + MotIdfb2;
		/* Command andFBdetermines the larger current threshold */
		if( MotIqref2 > MotIfb2 )
		{
			MotIq = MotIqref2;
		}
		else
		{
			MotIq = MotIfb2;
		}

		/* Motor speed absolute value calculation */
		if( BaseLoops->MotSpd < 0 )
		{
			AbsMotSpd = -BaseLoops->MotSpd;
		}
		else
		{
			AbsMotSpd = BaseLoops->MotSpd;
		}
		/*--------- Carrier frequency switching judgment & μProgram write processing --------------------------*/
		if( CarrerChange->var.CarrState )
		{ /* Current high carrier setting */
			/*--------- ThresholdBIf it becomes larger than , lower the carrier frequency.(Or) ------------------------*/
			if( ( MotIq > CarrerChange->conf.IqLimitB ) || ( AbsMotSpd > CarrerChange->conf.VelLimitB ) )
			{
				CarrerChange->var.CarrState = FALSE;
				/* ★μIFAddress change after creation★ */
				/* ★μIFis handled exclusively.★ */
				SHalDrv_SetCarrerFreq( SvAsicRegs->AsicMicroReg, CarrerChange->conf.NormCarrCnt );
			}
		}
		else
		{ /* Current normal carrier setting */
			/*--------- ThresholdAIncrease the carrier frequency if it becomes smaller than(And) ------------------------*/
			if( ( MotIq < CarrerChange->conf.IqLimitA ) && ( AbsMotSpd < CarrerChange->conf.VelLimitA ) )
			{
				CarrerChange->var.CarrState = TRUE;
				/* ★μIFAddress change after creation★ */
				/* ★μIFis handled exclusively.★ */
				SHalDrv_SetCarrerFreq( SvAsicRegs->AsicMicroReg, CarrerChange->conf.HighCarrCnt );
			}
		}
	}
	return;
}


/****************************************************************************
  Phase compensation value calculation function
*****************************************************************************/
#define SVFSEL_PCMP_IDREF			0
/*--------------------------------------------------------------------------------------------------*/
/*		Phase compensation : Pcmp = Pcmpd(Delay compensation) + Pcmps(Speed compensation=Kpcmps*MotSpd)							*/
/*--------------------------------------------------------------------------------------------------*/
/*	Caution1:	Conventional method(SVFSEL_PCMP_IDREF != 1)In this case, the speed compensation during regeneration increases the field strength,				*/
/*			OVetc. are more likely to occur.																*/
/*	Caution2:	Sign of torque command(Electric power/regeneration), speed compensation(Pcmps), in order to match the management method of				*/
/*			No load(torque command is near zero)because the speed compensation swings in positive and negative directions.				*/
/*Caution3:	For linear motor,LpxUpdateLMPhase( ).									*/
/*--------------------------------------------------------------------------------------------------*/
static void	BaseLoopMakePhaseCompValue( BASE_LOOPCTRLS *BaseLoops,
										CTRL_LOOP_OUT *CtrlLoopOut,
										MENCV *MencV,
										BOOL WfActive )
{
	BPRMDAT		*Bprm;
	PHASECMP	*PhaseCmp;
	KSGAIN		MotSpd;			/*<S00A>*/

	Bprm = BaseLoops->Bprm;
	PhaseCmp = &(BaseLoops->PhaseComp);
	MotSpd = BaseLoops->MotSpd * Bprm->DirSign;				/* Motor Speed [2^24/OvrSpd]			*/

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate Pcmpd[65536/360deg]																*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	/* Phase compensation(Delay1) */
	PhaseCmp->var.Pcmpd = (MotSpd * PhaseCmp->conf.Kpcmpd);
	/* Phase compensation(Delay2) */
	PhaseCmp->var.Pcmpd2 = ( MotSpd * PhaseCmp->conf.Kpcmpd2 );
	/* Calculation of phase interpolation amount */
	PhaseCmp->var.PcmpdItplt = ( MotSpd * PhaseCmp->conf.KpcmpdItplt );
#else
	/* Phase compensation(Delay1) */
	PhaseCmp->var.Pcmpd = (SHORT)MlibMulgainNolim( MotSpd, PhaseCmp->conf.Kpcmpd );
	/* Phase compensation(Delay2) */
	PhaseCmp->var.Pcmpd2 = (SHORT)MlibMulgainNolim( MotSpd, PhaseCmp->conf.Kpcmpd2 );
	/* Calculation of phase interpolation amount */
	PhaseCmp->var.PcmpdItplt = (SHORT)MlibMulgainNolim( MotSpd, PhaseCmp->conf.KpcmpdItplt );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate Pcmps[65536/360deg]																*/
/*--------------------------------------------------------------------------------------------------*/
	/* Phase compensation1(Speed) */
#if (FLOAT_USE==TRUE)
	if( MotSpd > PhaseCmp->conf.PcmpSpd1 )
	{
		PhaseCmp->var.Pcmps1 = ( (MotSpd - PhaseCmp->conf.PcmpSpd1) *PhaseCmp->conf.Kpcmpsp1 );
	}
	else if( MotSpd < -PhaseCmp->conf.PcmpSpd1 )
	{
		PhaseCmp->var.Pcmps1 = ( (MotSpd + PhaseCmp->conf.PcmpSpd1) * PhaseCmp->conf.Kpcmpsp1 );
	}
#else
	if( MotSpd > PhaseCmp->conf.PcmpSpd1 )
	{
		PhaseCmp->var.Pcmps1 = 
			(SHORT)MlibMulgainNolim( (MotSpd - PhaseCmp->conf.PcmpSpd1), PhaseCmp->conf.Kpcmpsp1 );
	}
	else if( MotSpd < -PhaseCmp->conf.PcmpSpd1 )
	{
		PhaseCmp->var.Pcmps1 = 
			(SHORT)MlibMulgainNolim( (MotSpd + PhaseCmp->conf.PcmpSpd1), PhaseCmp->conf.Kpcmpsp1 );
	}
#endif /* FLOAT_USE */
	else
	{
		PhaseCmp->var.Pcmps1 = 0;
	}
	/*----------------------------------------------------------------------------------------------*/
#if 0 /* SGDVFunction disabled */
	/* Phase compensation2(Speed) */
#if (FLOAT_USE==TRUE)
	if( MotSpd > PhaseCmp->conf.PcmpSpd2 )
	{
		PhaseCmp->var.Pcmps2 = 
			(SHORT)( (float)(MotSpd - PhaseCmp->conf.PcmpSpd2) * PhaseCmp->conf.Kpcmpsp2 );
	}
	else if( MotSpd < -PhaseCmp->conf.PcmpSpd2 )
	{
		PhaseCmp->var.Pcmps2 = 	(SHORT)( (float)(MotSpd + PhaseCmp->conf.PcmpSpd2)  * PhaseCmp->conf.Kpcmpsp2);
			
#else
	if( MotSpd > PhaseCmp->conf.PcmpSpd2 )
	{
		PhaseCmp->var.Pcmps2 = 
			(SHORT)MlibMulgainNolim( (MotSpd - PhaseCmp->conf.PcmpSpd2), PhaseCmp->conf.Kpcmpsp2 );
	}
	else if( MotSpd < -PhaseCmp->conf.PcmpSpd2 )
	{
		PhaseCmp->var.Pcmps2 = 
			(SHORT)MlibMulgainNolim( (MotSpd + PhaseCmp->conf.PcmpSpd2), PhaseCmp->conf.Kpcmpsp2 );
	}
#endif /* FLOAT_USE */
	else
	{
		PhaseCmp->var.Pcmps2 = 0;
	}
#endif
	/*----------------------------------------------------------------------------------------------*/
	/* Phase compensation total(Speed) [65536/360deg] */
	PhaseCmp->var.Pcmps = PhaseCmp->var.Pcmps1 + PhaseCmp->var.Pcmps2;


	if( SVFSEL_PCMP_IDREF != 1 )
	{ /* Pcmps by MotPhase */
//		if( ((Prm->dq_sw & 0x0800) == 0 )
//			|| (( Prm.flg_wf & 0x0001 ) == 0) )
		if( WfActive == TRUE )
		{ /* No speed compensation when using voltage feedback method field weakening */
//			MencV->MotPhaseX = MencV->MotPhase + PhaseCmp->var.Pcmpd;
			MencV->MotPhaseX = MencV->MotPhase;
		}
		else
		{
			MencV->MotPhaseX = MencV->MotPhase + PhaseCmp->var.Pcmpd + PhaseCmp->var.Pcmps;
		}
	}
	else if( SVFSEL_PCMP_IDREF == 1 )
	{ /* Pcmps by IdRef/IqRef */
		MencV->MotPhaseX = MencV->MotPhase + PhaseCmp->var.Pcmpd;
	}

	MencV->MotPhaseC = MencV->MotPhase + PhaseCmp->var.Pcmpd2;

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Kernel Output Service for ScanA																*/
/*																									*/
/****************************************************************************************************/
ALMID_T	KpxOutputScanA( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut, ASICS *SvAsicRegs, PRMDATA *Prm )
{
	MENCV			*MencV;
	USHORT			WfActive;
	ALMID_T			errorID;

	MencV = BaseLoops->MencV;
	errorID = NO_ALARM;

	/* Change Carrer Frequency */
	BaseLoopChangeCarrerFreq( BaseLoops, CtrlLoopOut, SvAsicRegs );

	/* Make Phase Compensetion Value */
	if( ((Prm->dq_sw & 0x0800) == 0 ) || (( Prm->MencP.flg_wf & 0x0001 ) == 0) )
	{
		WfActive = FALSE;
	}
	else
	{
		WfActive = TRUE;
	}
	BaseLoopMakePhaseCompValue( BaseLoops, CtrlLoopOut, MencV, WfActive );

	return errorID;
}




/****************************************************************************************************/
/*																									*/
/*		Initialize function for ScanA																*/
/*																									*/
/****************************************************************************************************/
void SysScanAInit(	BASE_LOOPCTRLS *BaseLoops,
					MENCV			*MencV,
					MENCV			*FencV,	/* <S04D> */
					BPRMDAT			*Bprm,
					MOTSPDMAFIL		*MotSpdMafil,		/* Variable for moving average calculation for motor speed detection		*/
					BASE_CTRL_HNDL 	*BaseCtrls,
					GSELGAINS		*GselGains,
					ASIC_HW_IF_STR	*pAsicHwReg,
					ALMMAN_HANDLE 	*AlmManager,														/* <S018> */
					MOTLESSTST 		*MotorLessTest)														/* <S1B7> */

{
	BaseLoops->MencV = MencV;
	BaseLoops->FencV = FencV;	/* <S04D> */
	BaseLoops->Bprm = Bprm;
	BaseLoops->MotSpdMafil = MotSpdMafil;
	BaseLoops->BaseCtrls = BaseCtrls;
	BaseLoops->GselGains = GselGains;
	BaseLoops->pAsicHwReg = pAsicHwReg;																	/* <S004> */
	BaseLoops->AlmManager = AlmManager;																	/* <S018> */
	BaseLoops->MotorLessTest = MotorLessTest;															/* <S1B7> */

	/* Variable initialization for carrier frequency switching */
	MlibResetLongMemory( &(BaseLoops->CarrerChange.var), sizeof(BaseLoops->CarrerChange.var)/4 );
	/* Variable initialization for phase compensation value calculation */
	MlibResetLongMemory( &(BaseLoops->PhaseComp.var), sizeof(BaseLoops->PhaseComp.var)/4 );
}



/****************************************************************************************************/
/*																									*/
/*		ScanA Cyclic data input function															*/
/*																									*/
/****************************************************************************************************/
//void SysInputScanAProcessData( BASE_LOOPCTRLS *BaseLoops, BASE_CTRL_HNDL *pBaseCtrl, BASE_CTRL_OUT *BaseCtrlOut )
void SysInputScanAProcessData( BASE_LOOPCTRLS *BaseLoops, BASE_CTRL_HNDL *pBaseCtrl,
												BASE_CTRL_OUT *BaseCtrlOut, CMD_CTRLBITS *CmdCtrl )	  /* <S050> */
{
	BaseLoops->LastCtrlMode = BaseLoops->CtrlMode;							/* <S007> */
	BaseLoops->CtrlMode = (USHORT)pBaseCtrl->CtrlCmdMngr.CtrlMode;
	BaseLoops->ZCtrlMode = (USHORT)pBaseCtrl->CtrlModeSet.ZctrlMode.zm;

	BaseLoops->CmdCtrlBit = CmdCtrl->CmdCtrlBit; /* <S050> */

//	BaseLoops->dPcmda = pBaseCtrl->CtrlCmdMngr.dPosRefo;
//	pBaseCtrl->CtrlCmdMngr.dPosRefo = 0;
//	LpxSvPositionManager( BaseLoops, &(pBaseCtrl->PosManager) );	/* <S14C> */
//	BaseLoops->SpdRefi = pBaseCtrl->CtrlCmdMngr.SpdRefo;
//	BaseLoops->TrqRefi = pBaseCtrl->CtrlCmdMngr.TrqRefo;

	BaseLoops->SpdFFCx = pBaseCtrl->CtrlCmdMngr.SpdFFCx;
	BaseLoops->TrqFFCx = pBaseCtrl->CtrlCmdMngr.TrqFFCx;
	BaseLoops->SpdFBCx = pBaseCtrl->CtrlCmdMngr.SpdFBCx;
	BaseLoops->TrqFBCx = pBaseCtrl->CtrlCmdMngr.TrqFBCx;

//	BaseLoops->TrqRefs;
//	BaseLoops->TrqClamp = FALSE; /* <S04B> */
	BaseLoops->TrqClamp = pBaseCtrl->CtrlCmdMngr.TrqClamp; /* <S04B> */

	if ( BaseLoops->LastCtrlMode == CTRLMODE_TRQ )       /* <S075> */
	{
		BaseLoops->SpdCtrl.V.SpdRefFilClrCmd = TRUE;     /* Speed command phase lead compensation filter initialization <S075> */
		BaseLoops->SpdCtrl.V.SpdFFFilClrCmd = TRUE;      /* Speed FF phase lead compensation filter initialization <S075> */
	}
}



/* 2012.12.27 Y.Oka Temporary response. Make it properly later. */
void SysOutputScanAProcessData( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut )
{
//	POSCTRL		*PosCtrl;		/* Position control variable */
	POS_MNG_HNDL *PosMngV;
//	PosCtrl = &BaseLoops->PosCtrl;
	PosMngV = &BaseLoops->BaseCtrls->PosManager;

	CtrlLoopOut->MotPosErr = PosMngV->var.PosErr;	/* Position amplifier deviation					[Pulse]				*/
	CtrlLoopOut->SpdRefo = BaseLoops->SpdRefo;		/* Speed command output						[2^24/OvrSpd]		*/
//	CtrlLoopOut->SpdFFC;							/* SpeedFFCompensation						[2^24/OvrSpd]		*/
	CtrlLoopOut->MotSpd_a = BaseLoops->MotSpd_a;
	CtrlLoopOut->TrqRefo = BaseLoops->TrqRefo;		/* Torque command output					[2^24/MaxTrq]		*/
	CtrlLoopOut->TrqRefo_a = BaseLoops->TrqRefo;	/* Torque command output(Average value)			[2^24/MaxTrq]		*/
//	CtrlLoopOut->TrqRefMon = BaseLoops->TrqRefo;	/* Torque command monitor					[2^24/MaxTrq]		*/
//	CtrlLoopOut->TrqRefMon_a = BaseLoops->TrqRefo;	/* Torque command monitor					[2^24/MaxTrq]		*/
	CtrlLoopOut->TrqRefMon = 						/* @@Add Torque command monitor [2^24/MaxTrq] */
	MlibMulhigh32( (CtrlLoopOut->IqRefMon << 16), NORM_IQ_TO_TRQ );
	CtrlLoopOut->TrqRefMon_a = 						/* @@Add  Torque command monitor(Average value) [2^24/MaxTrq] */
	MlibMulhigh32( (CtrlLoopOut->IqRefMon_a << 16), NORM_IQ_TO_TRQ );

//	CtrlLoopOut->IdRefMon;							/* dAxis current command monitor				[15000/Imax]		*/
//	CtrlLoopOut->IqRefMon;							/* qAxis current command monitor				[15000/Imax]		*/
//	CtrlLoopOut->IqRefMon_a;						/* qAxis current command monitor(Average value)		[15000/Imax]		*/
//	CtrlLoopOut->IqRefMon_l;						/* qAxis current command monitor(Previous value)		[15000/Imax]		*/
//	CtrlLoopOut->IqFbMon;							/* qAxis currentFBMonitor										*/
//	CtrlLoopOut->IdFbMon;							/* dAxis currentFBMonitor										*/
//	CtrlLoopOut->IqRefFilMon;						/* qAxis current command monitor(After filter)						*/
//	CtrlLoopOut->PConActFlg = FALSE;				/* PControl state confirmation flag									*//* <S064> */
//	CtrlLoopOut->SvonSpdLmtFlag = FALSE;			/* ServoONHourly speed limit flag							*//* <S04B> */
	CtrlLoopOut->SvonSpdLmtFlag = BaseLoops->SpdCtrl.V.SvonSpdLmtFlag;	/* ServoONHourly speed limit flag		*//* <S04B> */
//	CtrlLoopOut->SpdClamp = FALSE;					/* Speed limit flag during torque control							*//* <S04B> */
//	CtrlLoopOut->TrqClamp = FALSE;					/* Torque limit flag									*//* <S04B> */
}


/****************************************************************************************************/
/*																									*/
/*		Servo position management main function																	*/
/*																									*/
/****************************************************************************************************/
static void	LpxSvPositionManager( BASE_LOOPCTRLS *BaseLoops, POS_MNG_HNDL *PosManager )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Position command update (125usto1times. It is the integrated value of62.5us)			*/
/*--------------------------------------------------------------------------------------------------*/
	if( PosManager->RefRenewCntA != PosManager->RefRenewCntB )
	{ /* ScanB, the position command is stored in1th	*/
		/* 1thScanA */
		BaseLoops->dPosRefi = PosManager->CompdPosRefi >> 1;
		BaseLoops->dPcmda   = PosManager->CompdPcmda   >> 1;

		if( PosManager->RefRenewalChkCnt == 0 )
		{ /* 2thScanAdoes not run before [63] does not run */
			/* Previous2thScanA */
			BaseLoops->dPosRefi += BaseLoops->NextdPosRefi;
			BaseLoops->dPcmda   += BaseLoops->NextdPcmda;
		}

		/* 2thScanAof	Create position command */
		BaseLoops->NextdPosRefi = PosManager->CompdPosRefi - (PosManager->CompdPosRefi >> 1);
		BaseLoops->NextdPcmda   = PosManager->CompdPcmda   - (PosManager->CompdPcmda   >> 1);

		PosManager->RefRenewalChkCnt = 0;
	}
	else
	{ /* ScanB, the position command is stored in2th */
		if( PosManager->RefRenewalChkCnt == 0 )
		{
			/* 2thScanA */
			BaseLoops->dPosRefi = BaseLoops->NextdPosRefi;
			BaseLoops->dPcmda   = BaseLoops->NextdPcmda;
		}
		else
		{ /* ScanAbut3times. */
			/* 3th time, do not create commands */
			BaseLoops->dPosRefi = 0;
			BaseLoops->dPcmda   = 0;
		}
		PosManager->RefRenewalChkCnt++;
	}
	PosManager->RefRenewCntA = PosManager->RefRenewCntB;

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation calculation(Command unit)																<S14C>	*/
/*--------------------------------------------------------------------------------------------------*/
/* 2012.09.07 Y.Oka ★Position deviation clear function currently not supported★ */
//	if( !BoutV.f.PerClrCmd )
//	if((BaseLoops->LastCtrlMode == CTRLMODE_SPD)||(BaseLoops->LastCtrlMode == CTRLMODE_TRQ))/* <S007> */
//	{/* When switching control mode	*/
//		BaseLoops->PosCtrl.V.Per64[0] = PosManager->Per64[0];
//		BaseLoops->PosCtrl.V.Per64[1] = PosManager->Per64[1];
//	}
	if( !(BaseLoops->BaseCtrls->CtrlCmdMngr.PerrClrReq) )
	{
#if (FLOAT_USE==TRUE)
		if( PosManager->var.EgearDisable )
		{
			FlibPerrcala( BaseLoops->dPcmda, BaseLoops->dPosFbki,
							&PosManager->conf.InvalidEgear, &PosManager->var.PosErrA );
		}
		else
		{
			FlibPerrcala( BaseLoops->dPcmda, BaseLoops->dPosFbki,
							&PosManager->conf.Egear, &PosManager->var.PosErrA );
		}
	}
#else
		if( PosManager->var.EgearDisable )
		{
			MlibPerrcala( BaseLoops->dPcmda, BaseLoops->dPosFbki,
							&PosManager->conf.InvalidEgear, &PosManager->var.PosErrA );
		}
		else
		{
			MlibPerrcala( BaseLoops->dPcmda, BaseLoops->dPosFbki,
							&PosManager->conf.Egear, &PosManager->var.PosErrA );
		}
	}
#endif
	return;
}



/***************************************** end of file **********************************************/

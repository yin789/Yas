/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MotorPoleFind.c : For magnetic pole detection control	module definition											*/
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
/*	Note	:	First edition	2011.01.25	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "MotorPoleFind.h"
#include "MLib.h"



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	LONG	MpoleFindSpeedControl( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo, BOOL BeComplete );		*/
/*	ALMID_T	MpoleFindSequence( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo, BOOL PdetReq );				*/
/*	ALMID_T	MpoleFindDetectAlarm( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo, BOOL PdetReq );			*/
/*	ALMID_T	MpoleFindCheckMovingRange( MPFIND *MpFind, BASE_CTRL_HNDL *pBaseCtrl, KSGAIN Kmovpos );	*/
/****************************************************************************************************/
static ALMID_T	MpoleFindDetectAlarm( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo );
static ALMID_T	MpoleFindCheckMovingRange( MPFIND *MpFind, LONG dMotPos, KSGAIN Kmovpos );
static	void	MpoleFindDrive( MPFIND *MpFind, LONG SpdFbki, LONG TrqRefo );
static	void	CheckMpFindFail( MPFIND *MpFind, LONG SpdFbki );
static	void	MpFindLauSpdRef( MPFIND *MpFind, LONG SpdFbki );
static	void	MpFindCalPosAbout( MPFIND *MpFind );
static	void	MpFindCalPos( LONG *DegCalc, LONG *Counter, LONG *Degree, BOOL *NoGood_flag );
static	void	MpFindGetMaxTrqSpd( MPFIND *MpFind, LONG SpdFbki, LONG TrqRefo );
static	void	JudgeMpFindErrSign( MPFIND *MpFind );
static	void	UpdatePhaseCalculation( MPFIND *MpFind );
static	void	StopMpoleFind( MPFIND *MpFind, BOOL MotStopSts );
static	BOOL	ConfirmMpoleFind( MPFIND *MpFind, KSGAIN Kphasepos, LONG dMotPos,
																			LONG SpdFbki );
static	LONG	MpFindLauTrqRef( MPFIND *MpFind, BOOL *WaitFinish );
/****************************************************************************************************/



/****************************************************************************************************/
/*																									*/
/*		Speed control calculation for magnetic pole detection																		*/
/*																									*/
/****************************************************************************************************/
LONG	MpoleFindSpeedControl( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo, BOOL BeComplete )
{
	LONG			PacOut;
	LONG			IacOut;
	LONG			TrqRefx;
	LONG			work_integless;
	LONG			work;

/*--------------------------------------------------------------------------------------------------*/
/*		Processing when torque control is requested to confirm phase correction value														*/
/*--------------------------------------------------------------------------------------------------*/
	/* Check Control Mode Change Flag */
	if( MpFind->var.PhaseConfFlg == TRUE )
	{
		/* Direct Torque Reference Input */
		MpFind->var.Ivar64[1] = MpFind->var.Ivar64[0] = 0;
		return( MpFind->var.TrqRefPhaConf );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		SpeedFBFilter processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	MpFind->var.SpdFbFilo = MlibLpfilter1( MpfExeInfo->SpdFbMonitor,
										   MpfExeInfo->KVfbFil,
										   MpFind->var.SpdFbFilo );

/*--------------------------------------------------------------------------------------------------*/
/*		Speed deviation calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
	MpFind->var.SpdErr = MpFind->MpFindSpdRef - MpFind->var.SpdFbFilo;

/*--------------------------------------------------------------------------------------------------*/
/*		Processing during base block																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( (BeComplete == FALSE) || (MpFind->var.ZeroTorque == TRUE) )
	{
		MpFind->TrqFilOut = 0;
		MpFind->var.Ivar64[1] = MpFind->var.Ivar64[0] = 0;
//		pBaseCtrl->CtrlCmdMngr.SpdRefo = 0;
		return( 0 );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Speed control processing																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* Proportional term calculation */
	PacOut = MpFind->var.SpdErr;

	/* Integral term calculation */
	if( MpFind->var.ClrIntegral )
	{
		IacOut = 0;
		work_integless = 0;
		MpFind->var.Ivar64[0] = 0;
		MpFind->var.Ivar64[1] = 0;
		MpFind->var.ClrIntegral = FALSE;
	}
	else
	{
		if( !MpFind->var.InteglessON )
		{
			/* Normal state */
			work_integless = 0;
		}
		else
		{
			/* Integral less */
#if (FLOAT_USE==TRUE)
			work_integless = (LONG)( (float)(MpFind->var.Ivar64[1] >> 1) * MpFind->conf.InteglessGain );
#else
			work_integless = MlibMulgainNolim( (MpFind->var.Ivar64[1] >> 1), MpFind->conf.InteglessGain );
#endif /* FLOAT_USE */
		}
	}
	work = MpFind->var.SpdErr - work_integless;
	IacOut =
		MlibIntegral( work, MpFind->conf.Ki, MpFind->var.Ivar64 );

	work = PacOut + IacOut;

	/* Automatic adjustment of gain */
	switch( MpFind->var.KvGain )
	{
		case 0 :		/* Gain1x */
			break;

		case 1 :		/* Gain1.5x */
			work = work + (work >> 1);
			break;

		case 2 :		/* Gain2x */
			work = (work << 1);
			break;

		case -1 :		/* Gain0.75x */
			work = (work >> 1) + (work >> 2);
			break;

		case -2 :		/* Gain0.5x */
			work = (work >> 1);
			break;

		default :
			break;
	}
#if (FLOAT_USE==TRUE)
	TrqRefx = ( (float)work * MpFind->conf.Kv );
#else
	TrqRefx = MlibMulgain( work, MpFind->conf.Kv );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		1st-order low-pass filter & torque limit processing														*/
/*--------------------------------------------------------------------------------------------------*/
	MpFind->TrqFilOut = MlibLpfilter1( TrqRefx, MpFind->conf.Klpf, MpFind->TrqFilOut );
	return( MlibLimitul(MpFind->TrqFilOut, 0x01000000, -0x01000000) );

}



/****************************************************************************************************/
/*																									*/
/*		Magnetic pole erroneous detection alarm detection																		*/
/*																									*/
/****************************************************************************************************/
static ALMID_T	MpoleFindDetectAlarm( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo )
{
	ALMID_T		AlarmCode;

	if(MpfExeInfo->HaltCondition != FALSE)
	{
		MpFind->MPJudgeMonitor[0] = 0;	/* Magnetic pole detection judgment monitor reset		*/
		MpFind->MPJudgeMonitor[1] = 0;	/* Magnetic pole detection judgment monitor reset		*/
		MpFind->MPJudgeMonitor[2] = 0;	/* Magnetic pole detection judgment monitor reset		*/
		AlarmCode = ALM_PFINDSTOP;	/* A.C51 : Magnetic pole detection stop */
	}
	else if(MpFind->var.FailTimes >= TRYCOUNT)
	{
//		MpFind->var.Step = MPF_INIT;
		AlarmCode = ALM_POLE;		/* A.C50 : Magnetic pole erroneous detection */
	}
	else
	{
		AlarmCode = MpoleFindCheckMovingRange(MpFind,
		                                      MpfExeInfo->dMotPosition,
		                                      MpfExeInfo->Kmovpos);
	}

	return	AlarmCode;
}



/****************************************************************************************************/
/*																									*/
/*		Magnetic pole detection movable range confirmation process																	*/
/*																									*/
/****************************************************************************************************/
static ALMID_T	MpoleFindCheckMovingRange( MPFIND *MpFind, LONG dMotPos, KSGAIN Kmovpos )
{
	LONG		lwk;
	ALMID_T		AlarmCode;
	AlarmCode = NO_ALARM;

	MpFind->UnMpFindingTime++;

	MpFind->var.MovPos += dMotPos;

	if( MpFind->var.MovPos > MpFind->UnMpfMovePosP )
	{
		MpFind->UnMpfMovePosP = MpFind->var.MovPos;
	}
	else if( MpFind->var.MovPos < MpFind->UnMpfMovePosN )
	{
		MpFind->UnMpfMovePosN = MpFind->var.MovPos;
	}

	if( MpFind->UnMpfMovePosP >= -MpFind->UnMpfMovePosN )
	{
		lwk = MpFind->UnMpfMovePosP;
	}
	else
	{
		lwk = -MpFind->UnMpfMovePosN;
	}

	if( MpFind->conf.DetectRange <  MlibMulgain( lwk, Kmovpos ) )
	{
		/* A.C53 : Magnetic pole detection movable range exceeded */
		AlarmCode = ALM_PDET_MOVEOVER;
	}

	return	AlarmCode;
}



/****************************************************************************************************/
/*																									*/
/*		Magnetic pole detection processing sequence																		*/
/*																									*/
/****************************************************************************************************/
ALMID_T	MpoleFindSequence( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo, BOOL PdetReq, BOOL BaseEnable )
{
	ALMID_T		AlarmCode;
	AlarmCode = NO_ALARM;

	if( PdetReq == FALSE )
	{
		MpFind->var.Step = MPF_INIT;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Magnetic pole detection processing sequence																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch( MpFind->var.Step )
	{
	/*----------------------------------------------------------------------------------------------*/
	case MPF_INIT:			/* Initial Step															*/
	/*----------------------------------------------------------------------------------------------*/
		if( (PdetReq & BaseEnable) == TRUE )
		{ /* Magnetic pole detection sequence start request */

			/* Initialize variables for magnetic pole detection sequence processing */
			MlibResetLongMemory( &MpFind->var, sizeof( MpFind->var ) >> 2 );
			/* Initialization of variable for monitor */
			MpFind->UnMpFindingTime = 0;
			MpFind->UnMpfMovePosP = 0;
			MpFind->UnMpfMovePosN = 0;

			MpFind->var.Step = MPF_INIT2;
		}
		break;
	/*----------------------------------------------------------------------------------------------*/
		case MPF_INIT2:		/* Initial Step2														*/
	/*----------------------------------------------------------------------------------------------*/
			MpFind->var.Ivar64[0] = 0;
			MpFind->var.Ivar64[1] = 0;
			MpFind->var.OSFail = FALSE;
			MpFind->var.dAxisMaxTrqPlus = 0;
			MpFind->var.dAxisMaxTrqMinus = 0;
			MpFind->var.qAxisMaxTrqPlus = 0;
			MpFind->var.qAxisMaxTrqMinus = 0;

			MpFind->var.dAxisMaxSpdPlus = 0;
			MpFind->var.dAxisMaxSpdMinus = 0;
			MpFind->var.qAxisMaxSpdPlus = 0;
			MpFind->var.qAxisMaxSpdMinus = 0;

			MpFind->var.DegRev = 0;
			MpFind->var.DegFinal = 0;
			MpFind->var.AreaCheckStep = 0;
			MpFind->var.ReverseCtr = 0;
			MpFind->var.NoGood = 0;

			/* Offset speed(Speed at the start of speed command) */
			MpFind->var.SpdOffset = MpfExeInfo->SpdFbMonitor;

			MpFind->var.Step = MPF_DRIVE;
			break;
	/*----------------------------------------------------------------------------------------------*/
	case MPF_DRIVE:			/* Driving processing																*/
	/*----------------------------------------------------------------------------------------------*/
		MpoleFindDrive( MpFind, MpfExeInfo->SpdFbMonitor, MpfExeInfo->TrefMonitor );
		AlarmCode = MpoleFindDetectAlarm(MpFind, MpfExeInfo);
		break;
	/*----------------------------------------------------------------------------------------------*/
	case MPF_CAL:			/* Phase calculation																*/
	/*----------------------------------------------------------------------------------------------*/
		AlarmCode = MpoleFindDetectAlarm(MpFind, MpfExeInfo);
		UpdatePhaseCalculation( MpFind );
		break;
	/*----------------------------------------------------------------------------------------------*/
	case MPF_STOP:			/* Stopping confirmation																*/
	/*----------------------------------------------------------------------------------------------*/
		AlarmCode = MpoleFindDetectAlarm(MpFind, MpfExeInfo);
		StopMpoleFind( MpFind, MpfExeInfo->MotStopSts );
		break;
	/*----------------------------------------------------------------------------------------------*/
	case MPF_CONFIRM:		/* Operation confirmation																*/
	/*----------------------------------------------------------------------------------------------*/
		AlarmCode = MpoleFindDetectAlarm(MpFind, MpfExeInfo);
		MpFind->var.PhaseConfFlg = TRUE;		/* Flag for torque control to confirm phase correction value		*/
		if( FALSE != ConfirmMpoleFind( MpFind,
									   MpfExeInfo->Kphasepos,
									   MpfExeInfo->dMotPosition,
									   MpfExeInfo->SpdFbMonitor ) )
		{
			/* A.C54 : Magnetic pole detection confirmation not possible */
			AlarmCode = ALM_PDET_NG;
		}
		break;
	/*----------------------------------------------------------------------------------------------*/
	case MPF_END:			/* Termination processing																*/
	/*----------------------------------------------------------------------------------------------*/
		MpFind->var.PhaseConfFlg = FALSE;
		MpFind->var.Ivar64[0] = 0;
		MpFind->var.Ivar64[1] = 0;
//		MpFind->var.Step = MPF_INIT;
		break;
	/*----------------------------------------------------------------------------------------------*/
	default:				/* This case is a sequence software error										*/
	/*----------------------------------------------------------------------------------------------*/
		MpFind->var.Step = MPF_INIT;

		/* A.C50 : Magnetic pole erroneous detection */
		AlarmCode = ALM_POLE;
		break;
	}

	return	AlarmCode;
}



/****************************************************************************************************/
/*																									*/
/*		Magnetic pole detection operation processing																			*/
/*																									*/
/****************************************************************************************************/
static	void	MpoleFindDrive( MPFIND *MpFind, LONG SpdFbki, LONG TrqRefo )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Operation sequence status confirmation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( !MpFind->var.Fail )
	{
		MpFind->var.PastTime++;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Reverse running & speed overcheck processing																*/
/*--------------------------------------------------------------------------------------------------*/
	CheckMpFindFail( MpFind, SpdFbki );

/*--------------------------------------------------------------------------------------------------*/
/*		Speed command creation processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	MpFindLauSpdRef( MpFind, SpdFbki );

	if( !MpFind->var.RangeFix )
	{
/*--------------------------------------------------------------------------------------------------*/
/*		8Check all areas to be divided																	*/
/*--------------------------------------------------------------------------------------------------*/
		switch( MpFind->var.RangeFixStep )
		{
		case PAREACHECK_000:
			if ( MpFind->var.PhaseChange )
			{
				/* Processing when determining reverse running */
				MpFind->MpfPhaseOffset += PHASE_DEG180;
				MpFind->var.PhaseChange = FALSE;
				MpFind->var.RangeFixStep = PAREACHECK_001;
			}
			else if ( MpFind->var.PastTime == 0 )
			{
				if ( MpFind->var.AreaCheckStep == 7 )
				{
					/* Calculate rough position from maximum speed and reverse running position */
					MpFindCalPosAbout( MpFind );

					/* Redo check */
					switch ( MpFind->var.NoGood )
					{
					case 0:
						/* Step1End */
						MpFind->MpfPhaseOffset += MpFind->var.DegFinal + PHASE_DEG090;
						MpFind->var.RangeFix = TRUE;
						MpFind->var.Direction = FALSE;
						MpFind->var.AxisChange = FALSE;
						break;

					case 1:
						/* Increase gain */
						MpFind->var.KvGain++;
						MpFind->MpfPhaseOffset = 0;
						MpFind->var.RangeFixStep = PAREACHECK_000;
						MpFind->var.FailTimes++;						/* Number of failures1Increase */
						MpFind->var.Step = MPF_INIT2;					/* Start again from initialization process */
						break;
					}
				}
				else
				{
					MpFind->MpfPhaseOffset += PHASE_DEG045;
					MpFind->var.RangeFixStep = PAREACHECK_000;
					MpFind->var.AreaCheckStep++;
				}
			}
			break;

		case PAREACHECK_001:
			if ( MpFind->var.PastTime == 0 )
			{
				/* Reverse running position acquisition */
				if ( MpFind->var.ReverseCtr <= 3 )
				{
					MpFind->var.DegReverse[MpFind->var.ReverseCtr] = PHASE_DEG045 * MpFind->var.AreaCheckStep;
				}
				MpFind->var.ReverseCtr++;

				if ( MpFind->var.AreaCheckStep == 7 )
				{
					/* Calculate rough position from maximum speed and reverse running position */
					MpFindCalPosAbout( MpFind );

					/* Redo check */
					switch ( MpFind->var.NoGood )
					{
					case 0:
						/* Step1End */
						MpFind->MpfPhaseOffset += MpFind->var.DegFinal - PHASE_DEG090;
						MpFind->var.RangeFix = TRUE;
						MpFind->var.Direction = FALSE;
						MpFind->var.AxisChange = FALSE;
						break;

					case 1:
						/* Increase gain */
						MpFind->var.KvGain++;
						MpFind->MpfPhaseOffset = 0;
						MpFind->var.RangeFixStep = PAREACHECK_000;
						MpFind->var.FailTimes++;						/* Number of failures1Increase */
						MpFind->var.Step = MPF_INIT2;					/* Start again from initialization process */
						break;
					}
				}
				else
				{
					MpFind->MpfPhaseOffset -= PHASE_DEG135;
					MpFind->var.RangeFixStep = PAREACHECK_000;
					MpFind->var.AreaCheckStep++;
				}
			}
			break;

		case PAREACHECK_002:
			if ( MpFind->var.PastTime == 0 )
			{
				MpFind->MpfPhaseOffset = 0;
				MpFind->var.KvGain--;									/* Lower gain */
				MpFind->var.RangeFixStep = PAREACHECK_000;
				MpFind->MPJudgeMonitor[MpFind->var.FailTimes] = 5;		/* Judgment monitor set */
				MpFind->var.FailTimes++;								/* Number of failures1Increase */
				MpFind->var.Step = MPF_INIT2;							/* Start again from initialization process */
			}
			break;

		case PAREACHECK_003:
			MpFind->MpfPhaseOffset = 0;
			MpFind->var.KvGain--;										/* Lower gain */
			MpFind->var.RangeFixStep = PAREACHECK_000;
			MpFind->MPJudgeMonitor[MpFind->var.FailTimes] = 4;			/* Judgment monitor set */
			MpFind->var.FailTimes++;									/* Number of failures1Increase */
			MpFind->var.Step = MPF_INIT2;								/* Start again from initialization process */
			break;

		default :
			MpFind->var.RangeFixStep = PAREACHECK_000;
			MpFind->MpfPhaseOffset = 0;
			break;
		}
	}
	else
	{
/*--------------------------------------------------------------------------------------------------*/
/*		Estimated angle narrowing down based on d-q axis deviation															*/
/*--------------------------------------------------------------------------------------------------*/
		if ( MpFind->var.PhaseChange )
		{
			MpFind->MpfPhaseOffset += PHASE_DEG180;
			MpFind->var.Times = 0;
			MpFind->var.RangeFixStep = PAREACHECK_002;
			MpFind->var.RangeFix = FALSE;
			MpFind->var.PhaseChange = FALSE;
		}
		else
		{
			MpFindGetMaxTrqSpd( MpFind, SpdFbki, TrqRefo );

			if ( MpFind->var.PastTime == 0 )
			{
				if ( MpFind->var.AxisChange )
				{
					MpFind->MpfPhaseOffset -= PHASE_DEG090;
				}
				else
				{
					MpFind->var.Times++;
					JudgeMpFindErrSign( MpFind );

					if ( MpFind->var.Step != MPF_CAL )
					{
						switch ( MpFind->var.AxisErrSign )
						{
							case AXISERR_EQUAL :
							case AXISERR_MINUS :
								MpFind->MpfPhaseOffset += PHASE_DEG090 + (PHASE_DEG030 >> MpFind->var.Times);
								break;

							case AXISERR_PLUS :
								MpFind->MpfPhaseOffset += PHASE_DEG090 - (PHASE_DEG030 >> MpFind->var.Times);
								break;

							default :
								break;
						}
					}
				}
			}
		}
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Error detection : Performs reverse driving and overspeed judgment.											*/
/*																									*/
/****************************************************************************************************/
static	void	CheckMpFindFail( MPFIND *MpFind, LONG SpdFbki )
{
	LONG	work;
	LONG	OverSpeed;

	/* Setting the wrong running judgment level */
	if ( MpFind->var.ReverseLevel )
	{
		/* Waiting time */
		work = MpFind->conf.ReverseSpd2;
	}
	else
	{
		/* During speed command */
		work = MpFind->conf.ReverseSpd;
	}

	/* Setting runaway judgment level */
	if ( MpFind->var.OSFail )
	{
		/* Level after overspeeding */
		OverSpeed = MpFind->conf.MaxSpd;
	}
	else
	{
		/* Normal level */
		OverSpeed = MpFind->conf.OverSpd;
	}

	/* Constantly overspeed judgment */
	if ( MlibLABS( SpdFbki ) >= OverSpeed )
	{
		MpFind->var.PastTime = MpFind->conf.WaitTime;
		MpFind->var.Fail = TRUE;
		MpFind->var.OSFail = TRUE;
		MpFind->var.ZeroTorque = TRUE;
	}

	/* Reverse running processing not in progress & step1occur, reverse running detection is performed. */
	if ( (!MpFind->var.Fail) && (!MpFind->var.RangeFix) )
	{
		if ( !MpFind->var.Direction )
		{
			/* At the time of a speed command in the positive direction */
			if ( (SpdFbki - MpFind->var.SpdOffset) < ( -work ) )
			{
				MpFind->var.Fail = TRUE;
				MpFind->var.PhaseChange = TRUE;
				MpFind->var.PastTime = MpFind->conf.WaitTime;
				MpFind->var.ClrIntegral = TRUE;
			}
		}
		else
		{
			/* Negative direction speed command */
			if ( (SpdFbki - MpFind->var.SpdOffset) > work )
			{
				MpFind->var.Fail = TRUE;
				MpFind->var.PhaseChange = TRUE;
				MpFind->var.PastTime = MpFind->conf.WaitTime;
				MpFind->var.ClrIntegral = TRUE;
			}
		}
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Speed command creation : Create an operation command when detecting a magnetic pole.												*/
/*																									*/
/****************************************************************************************************/
static	void	MpFindLauSpdRef( MPFIND *MpFind, LONG SpdFbki )
{
	LONG	AbsSpdRef;

/*--------------------------------------------------------------------------------------------------*/
/*		Processing when error is detected																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( MpFind->var.Fail )
	{
		MpFind->MpFindSpdRef = 0;
		MpFind->var.InteglessON = FALSE;

		if( --MpFind->var.PastTime <= 0 )
		{
			MpFind->var.RefOutTime = 0;
			MpFind->var.Fail = FALSE;
			MpFind->var.RefOutStep = MPFLAU_ACC;
			MpFind->var.Direction = FALSE;
			MpFind->var.AxisChange = FALSE;
			MpFind->var.ZeroTorque = FALSE;
			MpFind->var.ReverseLevel = FALSE;
			MpFind->var.SpdOffset = SpdFbki;

			if ( MpFind->var.OSFail )
			{
				MpFind->var.RangeFixStep = PAREACHECK_003;
			}
		}
		return;
	}

	AbsSpdRef = MlibLABS( MpFind->MpFindSpdRef );

/*--------------------------------------------------------------------------------------------------*/
/*		Speed command creation sequence																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch( MpFind->var.RefOutStep )
	{
	/*----------------------------------------------------------------------------------------------*/
	case MPFLAU_INIT:		/* Waiting time elapsed															*/
	/*----------------------------------------------------------------------------------------------*/
		MpFind->var.RefOutStep = MPFLAU_ACC;
		MpFind->var.RefOutTime = 0;
		MpFind->var.SpdOffset = SpdFbki;
		break;

	/*----------------------------------------------------------------------------------------------*/
	case MPFLAU_ACC:		/* Acceleration command creation															*/
	/*----------------------------------------------------------------------------------------------*/
		AbsSpdRef = MlibLaufilter( MpFind->conf.MaxSpd, AbsSpdRef, MpFind->conf.dSpd );
		MpFind->var.RefOutTime++;
		if( MpFind->var.RefOutTime >= MpFind->conf.AccTime )
		{
			MpFind->var.RefOutStep = MPFLAU_CONST;
			MpFind->var.RefOutTime = 0;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	case MPFLAU_CONST:		/* Steady speed command creation														*/
	/*----------------------------------------------------------------------------------------------*/
		AbsSpdRef = MpFind->conf.MaxSpd;
		MpFind->var.RefOutTime++;
		if( MpFind->var.RefOutTime >= MpFind->conf.ClmpTime )
		{
			MpFind->var.RefOutStep = MPFLAU_DEC;
			MpFind->var.RefOutTime = 0;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	case MPFLAU_DEC:		/* Create deceleration command															*/
	/*----------------------------------------------------------------------------------------------*/
		AbsSpdRef = MlibLaufilter( 0, AbsSpdRef, MpFind->conf.dSpd );
		MpFind->var.RefOutTime++;
		if( MpFind->var.RefOutTime >= MpFind->conf.AccTime )
		{
			MpFind->var.RefOutStep = MPFLAU_WAIT;
			MpFind->var.RefOutTime = 0;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	case MPFLAU_WAIT:		/* Waiting time elapsed															*/
	/*----------------------------------------------------------------------------------------------*/
		AbsSpdRef = 0;
		MpFind->var.RefOutTime++;
		MpFind->var.ReverseLevel = TRUE;
		if ( MpFind->var.RefOutTime >= MpFind->conf.InteglessTime )
		{
			MpFind->var.InteglessON = TRUE;
		}

		if( MpFind->var.RefOutTime >= MpFind->conf.WaitTime )
		{
			if( MpFind->var.Direction )
			{
				MpFind->var.PastTime = 0;
				if( MpFind->var.AxisChange )
				{
					MpFind->var.AxisChange = FALSE;
				}
				else
				{
					MpFind->var.AxisChange = TRUE;
				}
			}
			if( MpFind->var.Direction )
			{
				MpFind->var.Direction = FALSE;
			}
			else
			{
				MpFind->var.Direction = TRUE;
			}
			MpFind->var.RefOutStep = MPFLAU_ACC;
			MpFind->var.RefOutTime = 0;
			MpFind->var.InteglessON = FALSE;
			MpFind->var.ReverseLevel = FALSE;
			MpFind->var.SpdOffset = SpdFbki;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	default:
	/*----------------------------------------------------------------------------------------------*/
		AbsSpdRef = 0;
		break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Speed command output																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( MpFind->var.Direction )
	{
		MpFind->MpFindSpdRef = -AbsSpdRef;
	}
	else
	{
		MpFind->MpFindSpdRef = AbsSpdRef;
	}

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Rough position calculation function: Calculate the rough position from the reverse running position										*/
/*			①Checking the number of reverse running																		*/
/*			②Calculate estimated position from reverse running position															*/
/*																									*/
/****************************************************************************************************/
static	void	MpFindCalPosAbout( MPFIND *MpFind )
{
	BOOL	NG_flag;

	/* Checking the number of reverse running */
	if( MpFind->var.ReverseCtr >= 5 )
	{
		/* Increase gain and try again */
		MpFind->var.NoGood = TRUE;
		MpFind->MPJudgeMonitor[MpFind->var.FailTimes] = 1;			/* Judgment monitor set */
		return;
	}

	/* Checking the number of reverse running */
	if( MpFind->var.ReverseCtr <= 1 )
	{
		/* Increase gain and try again */
		MpFind->var.NoGood = TRUE;
		MpFind->MPJudgeMonitor[MpFind->var.FailTimes] = 2;			/* Judgment monitor set */
		return;
	}

	/* Calculating reverse running position */
	NG_flag = FALSE;
	MpFindCalPos( &MpFind->var.DegRev, &MpFind->var.ReverseCtr,
													&MpFind->var.DegReverse[0], &NG_flag );
	/* Redo judgment */
	if( NG_flag )
	{
		/* Increase gain and try again */
		MpFind->var.NoGood = TRUE;
		MpFind->MPJudgeMonitor[MpFind->var.FailTimes] = 3;			/* Judgment monitor set */
		return;
	}
	MpFind->var.DegRev += PHASE_DEG180;

	MpFind->var.DegFinal = MpFind->var.DegRev;

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Position calculation function: Calculate position(MpFindCalPosAbout()Used with functions)										*/
/*																									*/
/****************************************************************************************************/
static	void	MpFindCalPos( LONG *DegCalc, LONG *Counter, LONG *Degree, BOOL *NoGood_flag )
{
	ULONG	lwork1, lwork2;
	USHORT	work[3];
	USHORT	i;

	*NoGood_flag = FALSE;
	lwork1 = lwork2 = 0;

	if( *Counter == 1 )
	{
		lwork1 = *Degree;
	}
	else if( *Counter == 2 )
	{
		lwork1 = (ULONG)*Degree + (ULONG)*(Degree + 1);
		work[0] = (USHORT)( *(Degree + 1) - *Degree );

		if( work[0] > (USHORT)PHASE_DEG180 )
		{
			lwork1 = (lwork1 - (ULONG)PHASE_DEG360) >> 1;
		}
		else if( work[0] < (USHORT)PHASE_DEG180 )
		{
			lwork1 = lwork1 >> 1;
		}
		else
		{
			/* Reverse running phase is180degSince it was out of alignment, I issued a servo-on command from the higher level withoutNoGood_flagset */
			*NoGood_flag = TRUE;
		}
	}
	else if( *Counter == 3 )
	{
		lwork1 = (ULONG)*Degree + (ULONG)*(Degree + 1) + (ULONG)*(Degree + 2);
		work[0] = (USHORT)( *(Degree + 2) - *Degree );
		work[1] = (USHORT)( *(Degree + 1) - *Degree );
		work[2] = (USHORT)( *(Degree + 2) - *(Degree + 1) );

		if( work[0] <= (USHORT)PHASE_DEG135 )
		{
			lwork1 = lwork1 / 3;
		}
		else if( work[1] >= (USHORT)PHASE_DEG225 )
		{
			lwork1 = (lwork1 + (ULONG)PHASE_DEG360) / 3;
		}
		else if( work[2] >= (USHORT)PHASE_DEG225 )
		{
			lwork1 = (lwork1 + (ULONG)PHASE_DEG360 + (ULONG)PHASE_DEG360) / 3;
		}
		else
		{
			/* Reverse running phase is135deg, so check the power ofNoGood_flagset */
			*NoGood_flag = TRUE;
		}
	}
	else if( *Counter == 4 )
	{
		lwork1 = (ULONG)*Degree + (ULONG)*(Degree + 1) + (ULONG)*(Degree + 2) + (ULONG)*(Degree + 3);
		work[0] = (USHORT)( *(Degree + 1) - *Degree );
		work[1] = (USHORT)( *(Degree + 2) - *(Degree + 1) );
		work[2] = (USHORT)( *(Degree + 3) - *(Degree + 2) );

		lwork2 = 0;
		for( i = 0; i <= 2; i++ )
		{
			if( (work[i] != (USHORT)PHASE_DEG045) && (work[i] != (USHORT)PHASE_DEG225) )
			{
				/* Reverse running phase is135deg, so check the power ofNoGood_flagset */
				*NoGood_flag = TRUE;
				break;
			}
			else if( work[i] == (USHORT)PHASE_DEG225 )
			{
				lwork2 = ((ULONG)PHASE_DEG360 * (i+1));
			}
		}

		if( *NoGood_flag == FALSE )
		{
			lwork1 = (lwork1 + lwork2) >> 2;
		}
	}
	*DegCalc = lwork1;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Maximum torque & maximum speed acquisition : Drivingd-qObtain the maximum torque and maximum speed for each axis.			*/
/*																									*/
/****************************************************************************************************/
static	void	MpFindGetMaxTrqSpd( MPFIND *MpFind, LONG SpdFbki, LONG TrqRefo )
{
	if( !MpFind->var.AxisChange )
	{
		/* qaxis */
		if( !MpFind->var.Direction )
		{
			/* At the time of a speed command in the positive direction */
			if( MpFind->var.qAxisMaxTrqPlus < TrqRefo )
			{
				MpFind->var.qAxisMaxTrqPlus = TrqRefo;
			}
			if( MpFind->var.qAxisMaxSpdPlus < SpdFbki )
			{
				MpFind->var.qAxisMaxSpdPlus = SpdFbki;
			}
		}
		else
		{
			/* Negative direction speed command */
			if( MpFind->var.qAxisMaxTrqMinus > TrqRefo )
			{
				MpFind->var.qAxisMaxTrqMinus = TrqRefo;
			}
			if( MpFind->var.qAxisMaxSpdMinus > SpdFbki )
			{
				MpFind->var.qAxisMaxSpdMinus = SpdFbki;
			}
		}
	}
	else
	{
		/* daxis */
		if( !MpFind->var.Direction )
		{
			/* At the time of a speed command in the positive direction */
			if( MpFind->var.dAxisMaxTrqPlus < TrqRefo )
			{
				MpFind->var.dAxisMaxTrqPlus = TrqRefo;
			}
			if( MpFind->var.dAxisMaxSpdPlus < SpdFbki )
			{
				MpFind->var.dAxisMaxSpdPlus = SpdFbki;
			}
		}
		else
		{
			/* Negative direction speed command */
			if( MpFind->var.dAxisMaxTrqMinus > TrqRefo )
			{
				MpFind->var.dAxisMaxTrqMinus = TrqRefo;
			}
			if( MpFind->var.dAxisMaxSpdMinus > SpdFbki )
			{
				MpFind->var.dAxisMaxSpdMinus = SpdFbki;
			}
		}
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		d-qAxis misalignment angle calculation : d-qCalculate the deviation angle from the relationship between the maximum torque of each axis.						*/
/*																									*/
/****************************************************************************************************/
static	void	JudgeMpFindErrSign( MPFIND *MpFind )
{
	LONG	lwork1, lwork2;
	LONG	lwork3, lwork4;
	LONG	kx;

	lwork1 = MlibLABS( MpFind->var.dAxisMaxTrqPlus - MpFind->var.qAxisMaxTrqPlus );
	lwork2 = MlibLABS( MpFind->var.dAxisMaxTrqMinus - MpFind->var.qAxisMaxTrqMinus );
	lwork3 = MlibLABS( MpFind->var.dAxisMaxSpdPlus - MpFind->var.qAxisMaxSpdPlus );
	lwork4 = MlibLABS( MpFind->var.dAxisMaxSpdMinus - MpFind->var.qAxisMaxSpdMinus );

	if( lwork1 == lwork2 )
	{
		if( lwork3 < lwork4 )
		{
			/* Negative direction adopted */
			lwork1 = MlibLABS( MpFind->var.dAxisMaxTrqMinus );
			lwork2 = MlibLABS( MpFind->var.qAxisMaxTrqMinus );
			lwork3 = MlibLABS( MpFind->var.dAxisMaxSpdMinus );
			lwork4 = MlibLABS( MpFind->var.qAxisMaxSpdMinus );
		}
		else
		{
			/* Adoption of the positive direction */
			lwork1 = MpFind->var.dAxisMaxTrqPlus;
			lwork2 = MpFind->var.qAxisMaxTrqPlus;
			lwork3 = MpFind->var.dAxisMaxSpdPlus;
			lwork4 = MpFind->var.qAxisMaxSpdPlus;
		}
	}
	else if( lwork1 < lwork2 )
	{
		/* Negative direction adopted */
		lwork1 = MlibLABS( MpFind->var.dAxisMaxTrqMinus );
		lwork2 = MlibLABS( MpFind->var.qAxisMaxTrqMinus );
		lwork3 = MlibLABS( MpFind->var.dAxisMaxSpdMinus );
		lwork4 = MlibLABS( MpFind->var.qAxisMaxSpdMinus );
	}
	else
	{
		/* Adoption of the positive direction */
		lwork1 = MpFind->var.dAxisMaxTrqPlus;
		lwork2 = MpFind->var.qAxisMaxTrqPlus;
		lwork3 = MpFind->var.dAxisMaxSpdPlus;
		lwork4 = MpFind->var.qAxisMaxSpdPlus;
	}

	if( lwork1 == lwork2 )
	{
		lwork2 = 16384;
		lwork1 = AXISERR_EQUAL;
	}
	else if( lwork1 > lwork2 )
	{
		kx = MlibScalKxgain( lwork2, 1, lwork1, NULL, 24 );
#if (FLOAT_USE==TRUE)
		lwork2 = 16384 * kx;
#else		
		lwork2 = MlibMulgain( 16384, kx );
#endif /* FLOAT_USE */
		lwork1 = AXISERR_MINUS;
	}
	else
	{
		kx = MlibScalKxgain( lwork1, 1, lwork2, NULL, 24 );
#if (FLOAT_USE==TRUE)
		lwork2 = 16384 * kx;
#else	
		lwork2 = MlibMulgain( 16384, kx );
#endif /* FLOAT_USE */

		lwork1 = AXISERR_PLUS;
	}

	if( lwork3 == lwork4 )
	{
		lwork4 = 16384;
		lwork3 = AXISERR_EQUAL;
	}
	else if( lwork3 > lwork4 )
	{
		kx = MlibScalKxgain( lwork4, 1, lwork3, NULL, 24 );
#if (FLOAT_USE==TRUE)
		lwork4 = 16384 * kx;
#else	
		lwork4 = MlibMulgain( 16384, kx );
#endif /* FLOAT_USE */
		lwork3 = AXISERR_PLUS;
	}
	else
	{
		kx = MlibScalKxgain( lwork3, 1, lwork4, NULL, 24 );
#if (FLOAT_USE==TRUE)
		lwork4 = 16384 * kx;
#else	
		lwork4 = MlibMulgain( 16384, kx );
#endif /* FLOAT_USE */
		lwork3 = AXISERR_MINUS;
	}

	if( MpFind->var.Times >= MpFind->conf.RepeatNum )
	{
		MpFind->MpFindSpdRef = 0;
		MpFind->var.AdjDegree = SINPHASE_045;
		MpFind->var.AdjSwpDeg = SINPHASE_045;
		MpFind->var.Step = MPF_CAL;
	}
	else
	{
		MpFind->var.dAxisMaxTrqPlus = 0;
		MpFind->var.qAxisMaxTrqPlus = 0;
		MpFind->var.dAxisMaxTrqMinus = 0;
		MpFind->var.qAxisMaxTrqMinus = 0;
		MpFind->var.dAxisMaxSpdPlus = 0;
		MpFind->var.qAxisMaxSpdPlus = 0;
		MpFind->var.dAxisMaxSpdMinus = 0;
		MpFind->var.qAxisMaxSpdMinus = 0;
	}

	if( lwork1 == lwork3 )
	{
		MpFind->var.AxisErrSign = (UCHAR)lwork1;
		MpFind->var.AxisErr = lwork2;
	}
	else if( lwork2 <= lwork4 )
	{
		MpFind->var.AxisErrSign = (UCHAR)lwork1;
		MpFind->var.AxisErr = lwork2;
	}
	else
	{
		MpFind->var.AxisErrSign = (UCHAR)lwork3;
		MpFind->var.AxisErr = lwork4;
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Phase calculation : Calculate the phase from the data obtained in the driving process and update the phase.								*/
/*																									*/
/****************************************************************************************************/
static	void	UpdatePhaseCalculation( MPFIND *MpFind )
{
	LONG	lwk;

	MpFind->var.ClrIntegral = TRUE;
	MpFind->var.AdjSwpDeg >>= 1;

	if( MpFind->var.AxisErrSign != AXISERR_EQUAL )
	{
		lwk = ( MlibSins16( MpFind->var.AdjDegree, 0x11 ) << 14 ) /
				MlibSins16( MpFind->var.AdjDegree + SINPHASE_090, 0x11 );

		if( MpFind->var.AxisErr > MlibLABS( lwk ) )
		{
			MpFind->var.AdjDegree += MpFind->var.AdjSwpDeg;
		}
		else
		{
			MpFind->var.AdjDegree -= MpFind->var.AdjSwpDeg;
		}
	}

	if( (MpFind->var.AdjSwpDeg == 1) || (MpFind->var.AxisErrSign == AXISERR_EQUAL) )
	{
		lwk = MpFind->var.AdjDegree << 2;

		switch( MpFind->var.AxisErrSign )
		{
		case AXISERR_EQUAL :
			MpFind->MpfPhaseOffset += PHASE_DEG045;
			break;

		case AXISERR_MINUS :
			MpFind->MpfPhaseOffset += PHASE_DEG090 - (SHORT)lwk;
			break;

		case AXISERR_PLUS :
			MpFind->MpfPhaseOffset += (SHORT)lwk;
			break;

		default :
			break;
		}
		MpFind->var.Step = MPF_STOP;
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Stopping confirmation processing : Performs magnetic pole detection stop confirmation processing												*/
/*																									*/
/****************************************************************************************************/
static	void	StopMpoleFind( MPFIND *MpFind, BOOL MotStopSts )
{
	if( MotStopSts == TRUE )
	{
		/* Magnetic pole detection variable initialization */
		MlibResetLongMemory( &MpFind->var, sizeof( MpFind->var ) >> 2 );

		MpFind->var.RangeFix = TRUE;
		MpFind->var.Step = MPF_CONFIRM;
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Operation confirmation processing(Current lock method) : Perform magnetic pole detection operation confirmation processing.								*/
/*																									*/
/****************************************************************************************************/
static BOOL ConfirmMpoleFind( MPFIND *MpFind, KSGAIN Kphasepos, LONG dMotPos, LONG SpdFbki )
{
	BOOL pdet_err;
	BOOL WaitFinish;
	pdet_err = FALSE;
	WaitFinish = FALSE;

	/* Thrust command creation */
	MpFind->var.TrqRefPhaConf = MpFindLauTrqRef( MpFind, &WaitFinish );

	switch( MpFind->var.ConfirmStep )
	{
		case MPFCSTEP_WAIT:
			/* ConfirmStepbutPHSCHANGE, ADJUST, check for overspeed. */
			if( MlibLABS( SpdFbki ) >= MpFind->conf.OverSpd )
			{
				pdet_err = TRUE;
			}
			/* Thrust command creation process start confirmation */
			if( MpFind->var.RefOutStep == MPFTRQ_ACC )
			{
				MpFind->var.ConfirmStep = MPFCSTEP_PHSCHANGE;
			}
			break;


		case MPFCSTEP_PHSCHANGE:
			/* ConfirmStepbutPHSCHANGE, ADJUST, check the travel distance. */
			/* Create position for operation confirmation */
			MpFind->var.MovPosPhaConf += dMotPos;
			if( MlibLABS(MpFind->var.MovPosPhaConf) >= MpFind->conf.ErrOKLevel )
			{
				pdet_err = TRUE;
			}
			/* Detection result temporary save */
			MpFind->var.PhsOffsetCalRslt = MpFind->MpfPhaseOffset;
			/* Phase change(dThe current flows to the [54] axis) */
			MpFind->MpfPhaseOffset -= PHASE_DEG090;
			MpFind->var.ConfirmStep = MPFCSTEP_ADJUST;
			break;


		case MPFCSTEP_ADJUST:
			/* ConfirmStepbutPHSCHANGE, ADJUST, check the travel distance. */
			/* Create position for operation confirmation */
			MpFind->var.MovPosPhaConf += dMotPos;
			if( MlibLABS(MpFind->var.MovPosPhaConf) >= MpFind->conf.ErrOKLevel )
			{
				pdet_err = TRUE;
			}
			/* To fix the phase, adjust the offset by the amount of movement. */
#if (FLOAT_USE==TRUE)
			MpFind->MpfPhaseOffset -= ( (float)dMotPos * Kphasepos );
#else
			MpFind->MpfPhaseOffset -= MlibMulgainNolim( dMotPos, Kphasepos );
#endif /* FLOAT_USE */

			/* WAITConfirm termination timing */
			if( WaitFinish == TRUE )
			{
				if( MpFind->var.RefOutStep == MPFTRQ_DEC )
				{ /* Constant speed operation end */
					MpFind->var.ConfirmStep = MPFCSTEP_PHSSTORE;
				}
				else if( MpFind->var.RefOutStep == MPFTRQ_END )
				{ /* Deceleration operation end */
					MpFind->var.ConfirmStep = MPFCSTEP_PHSREVERT;
				}
				else
				{ /* End of acceleration operation */
					; /* No processing when acceleration operation ends */
				}
			}
			break;


		case MPFCSTEP_PHSSTORE:
			/* ConfirmStepbutPHSCHANGE, ADJUST, check for overspeed. */
			if( MlibLABS( SpdFbki ) >= MpFind->conf.OverSpd )
			{
				pdet_err = TRUE;
			}
			/* Save final phase. */
			MpFind->var.PhsOffsetComp = MpFind->MpfPhaseOffset;
			/* To fix the phase, adjust the offset by the amount of movement. */
#if (FLOAT_USE==TRUE)
			MpFind->MpfPhaseOffset -= ( (float)dMotPos * Kphasepos );
#else
			MpFind->MpfPhaseOffset -= MlibMulgainNolim( dMotPos, Kphasepos );
#endif /* FLOAT_USE */
			MpFind->var.ConfirmStep = MPFCSTEP_ADJUST;
			break;


		case MPFCSTEP_PHSREVERT:
			/* ConfirmStepbutPHSCHANGE, ADJUST, check for overspeed. */
			if( MlibLABS( SpdFbki ) >= MpFind->conf.OverSpd )
			{
				pdet_err = TRUE;
			}
			/* Returning the phase(qThe current flows to the [54] axis) */
			MpFind->MpfPhaseOffset = MpFind->var.PhsOffsetComp + PHASE_DEG090;
			MpFind->var.ConfirmStep = MPFCSTEP_WAIT;
			MpFind->var.Step = MPF_END;
			break;

		default:
			break;
	}

	/* Detection result correction amount calculation(for Monitor) */
	MpFind->var.PhaseOffsetCalComp = MpFind->MpfPhaseOffset - MpFind->var.PhsOffsetCalRslt;

	return pdet_err;
}



/****************************************************************************************************/
/*																									*/
/*		Thrust command creation : Create a thrust command for magnetic pole detection confirmation.											*/
/*																									*/
/****************************************************************************************************/
static	LONG	MpFindLauTrqRef( MPFIND *MpFind, BOOL *WaitFinish )
{
	LONG	TrqRef;

	/* Average thrust */
	TrqRef = MpFind->var.TrqRefPhaConf;
/*--------------------------------------------------------------------------------------------------*/
/*		Thrust command creation sequence																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch( MpFind->var.RefOutStep )
	{
	/*----------------------------------------------------------------------------------------------*/
	case MPFTRQ_INIT:	/* Waiting time elapsed																*/
	/*----------------------------------------------------------------------------------------------*/
		TrqRef = 0;
		MpFind->var.ClrIntegral = TRUE;
		MpFind->var.RefOutStep = MPFTRQ_ACC;
		MpFind->var.RefOutTime = 0;
		break;

	/*----------------------------------------------------------------------------------------------*/
	case MPFTRQ_ACC:	/* Thrust command increase																*/
	/*----------------------------------------------------------------------------------------------*/
		TrqRef = MlibLaufilter( MpFind->conf.MaxTrq, TrqRef, MpFind->conf.dTrq );
		MpFind->var.RefOutTime++;
		if( MpFind->var.RefOutTime >= MpFind->conf.IncTime )
		{
			MpFind->var.RefOutStep = MPFTRQ_CONST;
			MpFind->var.RefOutTime = 0;
			*WaitFinish = TRUE;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	case MPFTRQ_CONST:	/* 	Thrust command constant															*/
	/*----------------------------------------------------------------------------------------------*/
		TrqRef = MpFind->conf.MaxTrq;
		MpFind->var.RefOutTime++;
		if( MpFind->var.RefOutTime >= MpFind->conf.ConstTime )
		{
			MpFind->var.RefOutStep = MPFTRQ_DEC;
			MpFind->var.RefOutTime = 0;
			*WaitFinish = TRUE;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	case MPFTRQ_DEC:	/* Thrust command decrease																*/
	/*----------------------------------------------------------------------------------------------*/
		TrqRef = MlibLaufilter( 0, TrqRef, MpFind->conf.dTrq );
		MpFind->var.RefOutTime++;
		if( MpFind->var.RefOutTime >= MpFind->conf.IncTime )
		{
			MpFind->var.RefOutStep = MPFTRQ_END;
			MpFind->var.RefOutTime = 0;
			*WaitFinish = TRUE;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	case MPFTRQ_END:	/* Waiting time elapsed																*/
	/*----------------------------------------------------------------------------------------------*/
		MpFind->var.ClrIntegral = TRUE;
		TrqRef = 0;
		MpFind->var.RefOutStep = MPFTRQ_ACC;
		MpFind->var.RefOutTime = 0;
		break;

	/*----------------------------------------------------------------------------------------------*/
	default :
	/*----------------------------------------------------------------------------------------------*/
		break;
	}

	return( MlibLimitul(TrqRef, MpFind->conf.MaxTrq, -MpFind->conf.MaxTrq) );
}





/****************************************************************************************************/
/*																									*/
/*		Calculation of command speed parameters for magnetic pole detection															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Rotary type parameters																			*/
/*			Prm.pdetmaxspd_r	: Pn493	Magnetic pole detection command speed			[min-1]							*/
/*																									*/
/*		Linear parameters																			*/
/*			Prm.pdetmaxspd		: Pn485	Magnetic pole detection command speed 			[mm/s]							*/
/*																									*/
/*		Common parameters																			*/
/*			Prm.pdetmaxt		: Pn486 Magnetic pole detection command acceleration/deceleration time		[ms]							*/
/*			Prm.pdetclmpt		: Pn487 Magnetic pole detection command constant speed time		[ms]							*/
/*			Prm.pdetwait		: Pn488	Magnetic pole detection command waiting time		[ms]							*/
/*			Prm.pdetgett		: Pn489	Magnetic pole detection data acquisition start time	[ms]							*/
/*			Prm.pdetinteglesst	: Pn492 Magnetic pole detection integral-less start time	[%]								*/
/*			Prm.pdetrevlvspdref	: Pn499 Reverse run level at magnetic pole detection speed command[%]								*/
/*			Prm.pdetrevlvwait	: Pn49A Reverse running level during magnetic pole detection waiting time[%]								*/
/*			Prm.pdetoslevel		: Pn49B Magnetic pole detection overspeed level		[%]								*/
/*																									*/
/****************************************************************************************************/
void	MpoleFind_CalcSpeedCommandPrm( MPFIND *MpFind, MENCV *MencV,
    	                               BPRMDAT *Bprm, MPFIND_UPRM *PdetPrm )
{
	LONG		work;

	if( MencV->AxisMotType == MOTTYPE_ROTARY )
	{ /* Rotary type motor */
		if( MencV->LowSpdMotor )
		{
			work = PdetPrm->pdetmaxspd_r;
		}
		else
		{
			work = 10 * PdetPrm->pdetmaxspd_r;
		}
		MpFind->conf.MaxSpd = BprmSpeedLevelCal( Bprm, work, 0 );
	}
	else if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* Linear motor */
		MpFind->conf.MaxSpd = BprmSpeedLevelCal( Bprm, (PdetPrm->pdetmaxspd * 10), 0 );
	}

	MpFind->conf.ReverseSpd		= MpFind->conf.MaxSpd * PdetPrm->pdetrevlvspdref / 100;
	MpFind->conf.ReverseSpd2	= MpFind->conf.MaxSpd * PdetPrm->pdetrevlvwait / 100;
	MpFind->conf.OverSpd		= MpFind->conf.MaxSpd * PdetPrm->pdetoslevel / 100;

	MpFind->conf.dSpd			= MpFind->conf.MaxSpd * Bprm->MPFCycleUs_MR / 1000 / PdetPrm->pdetmaxt;
	MpFind->conf.AccTime		= PdetPrm->pdetmaxt * 1000 / Bprm->MPFCycleUs_MR;
	MpFind->conf.ClmpTime		= PdetPrm->pdetclmpt * 1000 / Bprm->MPFCycleUs_MR;
	MpFind->conf.WaitTime		= PdetPrm->pdetwait * 1000 / Bprm->MPFCycleUs_MR;
	MpFind->conf.InteglessTime	= MpFind->conf.WaitTime * PdetPrm->pdetinteglesst / 100;

	MpFind->conf.MaxTrq		= BprmTorqueLevelCal( Bprm, (PdetPrm->pdetmaxtrq * 100), 0 );
	MpFind->conf.dTrq		= MpFind->conf.MaxTrq * Bprm->MPFCycleUs_MR / 1000 / PdetPrm->pdetinctime;
	MpFind->conf.IncTime	= PdetPrm->pdetinctime * 1000 / Bprm->MPFCycleUs_MR;
	MpFind->conf.ConstTime	= PdetPrm->pdetconsttime * 1000 / Bprm->MPFCycleUs_MR;

}



/****************************************************************************************************/
/*																									*/
/*		Speed control parameter calculation for magnetic pole detection															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			Prm.pdetloophz		: Pn481 Speed loop gain when detecting a magnetic pole		[0.1Hz]						*/
/*			Prm.pdetpitime		: Pn482 Velocity loop integration time when detecting magnetic pole	[0.01ms]					*/
/*			Prm.pdetjrate		: Pn490	Magnetic pole detection load level				[%]							*/
/*			Prm.pdetintegless	: Pn491 Magnetic pole detection integral-less gain			[0.01%]						*/
/*			When detecting a magnetic pole, use the load weight ratio for magnetic pole detection.											*/
/*			When detecting magnetic pole,i-PControl not available															*/
/*																									*/
/****************************************************************************************************/
void	MpoleFind_CalcSpeedControlPrm( MPFIND *MpFind,
    	                               USHORT trqfil11, BPRMDAT *Bprm, MPFIND_UPRM *PdetPrm  )
{
#if (FLOAT_USE==TRUE)

	float	fwk;

	fwk = Bprm->Kvx * (float)((100 + PdetPrm->pdetjrate) * PdetPrm->pdetloophz) / 1000.0F;
	MpFind->conf.Kv = fwk;

	fwk = 1.0F * (float)Bprm->MPFCycleUs_SC / (float)(10 * PdetPrm->pdetpitime);
	MpFind->conf.Ki = fwk;

	fwk = (float)PdetPrm->pdetintegless * 10.0f * (float)PdetPrm->pdetpitime / 10000.0f;
	MpFind->conf.InteglessGain = fwk / (float)Bprm->MPFCycleUs_SC;

#else
	LONG		s;
	LONG		wk;

	MpFind->conf.Kv = MlibScalKskxkx( Bprm->Kvx, (100 + PdetPrm->pdetjrate) * PdetPrm->pdetloophz, 1000, NULL, 24 );
	MpFind->conf.Ki = MlibScalKxgain( 1, Bprm->MPFCycleUs_SC, 10*PdetPrm->pdetpitime, NULL, 24 );

	wk = MlibScalKxgain( PdetPrm->pdetintegless, 10*PdetPrm->pdetpitime, 10000, &s, 0 );
	MpFind->conf.InteglessGain = MlibPcalKxgain( wk, 1, Bprm->MPFCycleUs_SC, &s, 24 );
#endif /* FLOAT_USE */

	MpFind->conf.Klpf = MlibPcalKf1gain( 10 * trqfil11, Bprm->MPFCycleUs_SC, 0 );

}



/****************************************************************************************************/
/*																									*/
/*		Calculate magnetic pole detection error level																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		For rotary type																				*/
/*			Prm.pdetdistok_r: Pn494 Magnetic pole detection movable range		[0.001rev]								*/
/*			Prm.pdetdegreeok: Pn498 Magnetic pole detection confirmation pass range	[deg(Electrical angle)]							*/
/*						  pdetdegreeokpole * (2^BitNo)												*/
/*			ErrOKLevel = ------------------------------		[pulse]									*/
/*									pole / 2 														*/
/*																									*/
/*		In the case of linear																				*/
/*			Prm.pdetdistok	: Pn48E Magnetic pole detection movable range								[mm]			*/
/*			Prm.pdetdegreeok: Pn498 Magnetic pole detection confirmation pass range							[deg(Electrical angle)]	*/
/*						  pdetdegreeokpole * (ppitch/10*10^-3 * 2) * (2^BitNo)						*/
/*			ErrOKLevel = ------------------------------------------------------		[pulse]			*/
/*									360 * ScalePitch/100*10^-6										*/
/*																									*/
/****************************************************************************************************/
void	MpoleFind_CalcErrorLevel( MPFIND *MpFind, MENCV *MencV,
    	                          MENCPRM *MencP, BPRMDAT *Bprm, MPFIND_UPRM *PdetPrm  )
{
	LONG		kx, sx;
	LONG		work;

	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* Linear motor */
		MpFind->conf.DetectRange = 100 * PdetPrm->pdetdistok;
		/* Converts electrical angle to pulse number. */
		work = MencV->NormScalePitch;
		kx = MlibScalKxgain( PdetPrm->pdetdegreeok, MencP->ppitch, 360, &sx, 0 );
		kx = MlibPcalKxgain( kx, (1 << MencP->bit_dp.b.l), work, &sx, 0 );
		kx = MlibPcalKxgain( kx, MencV->NormScaleExp, 1, &sx, 0 );
		kx = MlibPcalKxgain( kx, 200000, 1, &sx, -30 );
		MpFind->conf.ErrOKLevel = kx;
	}
	else
	{ /* Rotary type motor */
		if( MencV->LowSpdMotor )
		{
			MpFind->conf.DetectRange = PdetPrm->pdetdistok_r / 10;
		}
		else
		{
			MpFind->conf.DetectRange = PdetPrm->pdetdistok_r;
		}
		/* Converts electrical angle to pulse number. */
		MpFind->conf.ErrOKLevel =
			MlibScalKxgain( PdetPrm->pdetdegreeok, Bprm->MotPulse, (180 * MencP->oslv_pol.b.h), NULL, -30 );
	}
}




/***************************************** end of file **********************************************/

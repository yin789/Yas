/****************************************************************************************************/
/*																									*/
/*																									*/
/*		BaseEnableSequence.c : Base enable sequence management processing definition								*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Base enable sequence management processing														*/
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
/*	Note	:	First edition	2011.02.03	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "KLib.h"						/* <S053> */
#include "BaseEnableSequence.h"
#include "PowerManager.h"


#define CHARGE_PUMP_CMP_TIME 0x0080		/* Charge pump completion time:125[us]*128 =16[ms]	*//* <S094> */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
void	CpxZeroClampSequence( 						/* @@Zero Clamp Sequence 						*/
	ZC_CTRL_DATA *Zcv,								/* Zero Clamp Control							*/
	BASE_CTRL_OUT	*Bout,							/* Base Control Output 							*/
	FUN_CMN_CONTROL *Fctrl,							/* Function Control								*/
	SEQ_CTRL_OUT *Cout	);							/* Sequence Output 								*/
/*--------------------------------------------------------------------------------------------------*/
static LONG	LpxCheckServoOnCmdErr( 					/* <S0A7> Check Servo On Cmd Error (A.0B0)		*/
	BE_SEQ_HNDL 	*BeSeq, 						/* 												*/
	BOOL			ServoOnReq,						/* Servo On Command								*/
	FUN_CMN_CONTROL *FnCmnCtrl, 					/* 												*/
	CTRLMD 			CtrlMode );						/* Control Mode									*/

/****************************************************************************************************/
/*																									*/
/*		Servo ready signal setting process																	*/
/*																									*/
/****************************************************************************************************/
void BeSeqMakeServoReadySignal( BE_SEQ_HNDL *BeSeq, BOOL ready_cond, LONG PhaseReady )
{
	if(BeSeq->SvonEnable && ready_cond)
	{
		/* Servo Ready flag set (Ignore Phase) */
		BeSeq->SvReadyPhaseIgnore = TRUE;
		/* Servo Ready flag set */
		if( PhaseReady )
		{
			BeSeq->ServoReady = TRUE;
		}
		else
		{
			BeSeq->ServoReady = FALSE;
		}
	}
	else
	{
		/* Servo Ready flag set (Ignore Phase) */
		BeSeq->SvReadyPhaseIgnore = FALSE;
		/* Servo Ready flag set */
		BeSeq->ServoReady = FALSE;
	}
}



/****************************************************************************************************/
/*																									*/
/*		Sensor on/Servo on request setting process															*/
/*																									*/
/****************************************************************************************************/
ALMID_T	BeSeqMakeServoOnRequest( BE_SEQ_HNDL *BeSeq, LONG CmdSeqBit,
       	                          CTRLMD CtrlMode, FUN_CMN_CONTROL *FnCmnCtrl, MENCV *MencV )
{
	ALMID_T svon_err;
	BOOL SensOnReq, LastSensOnReq;
	BOOL ServoOnReq,PdetReq;

	svon_err = NO_ALARM;

	if(FnCmnCtrl->KnlMode != KPI_KMD_NORMAL)
	{/* Test mode */
		BeSeq->SvonReq = FALSE;
		SensOnReq = (0 != (CmdSeqBit & SENSOR_ON_BIT));
		FnCmnCtrl->State.FnSvCtrlDisable = FALSE;
	}
	else
	{/* Normal mode */
		if(FnCmnCtrl->FnSvControl != FALSE)
		{/* Fn mode */
			ServoOnReq = FnCmnCtrl->FnSvonReq;
			SensOnReq = (0 != (CmdSeqBit & SENSOR_ON_BIT));
			SensOnReq |= FnCmnCtrl->FnSensOnReq;
//			PdetReq = FnCmnCtrl->PdetReq;
			PdetReq = FnCmnCtrl->SvOnPdetReq;					/* <S21C> */

			FnCmnCtrl->State.LockNormalOpeDrive = TRUE;
		}
		else
		{/* Operation according to normal command */
			ServoOnReq = (0 != (CmdSeqBit & SERVO_ON_BIT));
			SensOnReq = (0 != (CmdSeqBit & SENSOR_ON_BIT));
			PdetReq = (0 != (CmdSeqBit & POL_DETECT_BIT));

			/* ServoONCommand check */
			if( LpxCheckServoOnCmdErr( BeSeq, ServoOnReq, FnCmnCtrl, CtrlMode ) )	/* <S0A7> */
			{
				svon_err = ALM_SVONCMD;		/* A.0B0 : SGDV!! */
				ServoOnReq = FALSE;
			}
#if 0		/* <S0A7> */
			if(FnCmnCtrl->State.LockNormalOpeDrive != FALSE)
			{/* Normal operation prohibited*/
				if( (CtrlMode.us.uwd_l == CTRLMODE_SPD) || (CtrlMode.us.uwd_l == CTRLMODE_TRQ) )
				{
					BeSeq->SenStatus = FALSE;
					FnCmnCtrl->State.LockNormalOpeDrive = FALSE;
				}
				else if( (ServoOnReq != FALSE) && (CtrlMode.us.uwd_l == CTRLMODE_POS) )
				{/* With servo on command & Position control mode command */
					svon_err = ALM_SVONCMD;	/* A.0B0 : SGDV!! */
					ServoOnReq = FALSE;
				}
				else
				{	/* do nothing. */
					;
				}
			}
#endif
		}

		if( MencV->PhaseReady != FALSE)
		{
			BeSeq->SvonReq = ServoOnReq;
			FnCmnCtrl->State.FnSvCtrlDisable = ServoOnReq;
		}
		else
		{
			BeSeq->SvonReq = PdetReq;
			FnCmnCtrl->State.FnSvCtrlDisable = ServoOnReq | PdetReq;
			if((ServoOnReq != FALSE) /*&& (FnCmnCtrl->PdetReq == FALSE)*/ )
			{
				svon_err = ALM_PDET;	/* A.C52 */
			}
		}
	}

	/* SENSER_ONRequest acquisition */
	LastSensOnReq = BeSeq->SenStatus;
	if( (MencV->AbsoEncType != MENC_INCRE_TYPE) && (MencV->AxisMotType == MOTTYPE_ROTARY) )
	{ /* When using absolute value encoder */
		if( (LastSensOnReq == FALSE) && (SensOnReq != FALSE) )
		{ /* SEN_ONFirst time */
#if	(SVFSEL_DIVOUT_USE == TRUE)  /*<S02D>*/
			if( MencV->DivInfo.DivOut == FALSE )
			{
				MencV->ReCalcAbsPosReq = TRUE;
			}
			else
			{
				;/*Do nothing(At serial encoder PAO phase serial output,MencV->ReCalcAbsPosReq = TRUE */
			}
#else/*<S02D>*/
			MencV->ReCalcAbsPosReq = TRUE;
#endif	/*(SVFSEL_DIVOUT_USE == TRUE)*/
		}
	}

	/* Internal status update */
	BeSeq->SenStatus = SensOnReq /* <S082> & MencV->EncReady */;

	return svon_err;
}


#if 0 /* PcmRlyDbControlProcess() */
/****************************************************************************************************/
/*																									*/
/*		DB circuit control																				*/
/*																									*/
/****************************************************************************************************/
void BeSeqDBControlProcess( BOOL DbOn, LONG RlyDbWay, USHORT faultState, ASIC_HW_IF_STR *pAsicHwReg,
							BOOL MotorLessTestMode )													/* <S1B7> */
{

	switch( RlyDbWay )
	{
	case DBCTRL_RLY_COMMON:				/* Inrush + DB: Relay common 								*/
		/* PcmRlyControlProcess() */
		break;

	case DBCTRL_TRIACK_SEPARATE:		/* Rush: Relay; DB: Thyristor 						*/
	case DBCTRL_RLY_SEPARATE:			/* Rush: Relay; DB: Relay 							*/
		if( ((faultState & SHAL_FAULT_BB) != 0) && (DbOn == TRUE) )
		{
			/* DBON */
			SHalDrv_DynamicBrakeON(pAsicHwReg);
		}
		else
		{
			/* If not in motorless test modeOFF *//* <S1B7> */
			/* TODO: SGD-V, the following conditions were also achieved by disabling interruptsORis							*/
			/* (1) AxisRscC->BeSequence->StopMode == STOPMODE_DBSTOPFR			*/
			/* (2) AxisRscC->BeSequence->StopMode == STOPMODE_FREERUN			*/
			/* (3) AxisRscC->FnCmnCtrl->KnlMode == KPI_KMD_ADJCURAD				*/
			if( MotorLessTestMode == FALSE )
			{
				/* DBOFF */
				SHalDrv_DynamicBrakeOFF(pAsicHwReg);
			}
		}
		break;

	case DBCTRL_TRIACK_COMMON:			/* Inrush + DB: Thyristor 								*/
		/* Not supported */
		break;

	default:
		break;
	}

}
#endif


/****************************************************************************************************/
/*																									*/
/*		Scan C control mode command																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Receiving requests from each sequence, determines the control mode based on the priority, and selects scan B			*/
/*			Commands control mode.															*/
/*			The final servo control mode is determined by scan B(BoutV.CtrlMode).				*/
/*																									*/
/****************************************************************************************************/
ULONG BeSeqControlModeManager( BE_SEQ_HNDL *BeSeq, FUN_CMN_CONTROL *FnCmnCtrl )
{
	CTRLMD	CtrlMcmd;

	if(FnCmnCtrl->FnSvControl != FALSE)
	{/* Fn mode */
		/* ScanC Control Mode <= ROUNDRequest mode */
		CtrlMcmd.dw = (ULONG)FnCmnCtrl->FnCtrlMcmd;
	}
	else
	{	/* ScanC Control Mode <= NO CMD */
		CtrlMcmd.dw = CTRLMODE_NOCMD;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		ScanC: Zero control mode command																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( BeSeq->StopMode == STOPMODE_ZEROSTOP )			/* Stop mode management						*/
	{
		CtrlMcmd.b.zm = CTRLMODE_ZSTOP;					/* Zero speed stop mode							*/
	}
	/*----------------------------------------------------------------------------------------------*/
/* <S1F5> Start */
	else if( BeSeq->StopMode == STOPMODE_LDECSTOP )		/* Stop mode management						*/
	{
		CtrlMcmd.b.zm = CTRLMODE_LDSTOP;				/* Deceleration stop mode by linear deceleration			*/
	}
	/*----------------------------------------------------------------------------------------------*/
/* <S1F5> End */
	else if( BeSeq->OtSeqData.var.OtZeroClamp )			/* OTSequence								*/
	{
		CtrlMcmd.b.zm = CTRLMODE_ZCLMP;					/* Zero clamp mode						*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( BeSeq->ZcCtrlData.ZcZeroClamp )			/* Zero clamp sequence					*/
	{
		CtrlMcmd.b.zm = CTRLMODE_ZCLMP;					/* Zero clamp mode						*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else												/* Default processing							*/
	{
		CtrlMcmd.b.zm = CTRLMODE_ZNONE;					/* No zero control								*/
	}

	return CtrlMcmd.dw;
}



/****************************************************************************************************/
/*																									*/
/*		Alarm stop mode setting process																	*/
/*																									*/
/****************************************************************************************************/
void BeSeqInputAlmStopMode( BE_SEQ_HNDL *BeSeq, LONG AlmStopMode,   /* <S1F5> */
                            CTRLMD CtrlMode, BOOL PhaseReady, SEQ_CTRL_OUT *Cout )  /* <S1F5> */
// void BeSeqInputAlmStopMode( ALM_STOP *AlmStop, LONG AlmStopMode, /* <S1F5> */
//                           CTRLMD CtrlMode, BOOL PhaseReady, BOOL HwbbSts, BOOL MotStop )  /* <S1F5> */
{
/* <S1F5> Start */
	ALM_STOP *AlmStop;
	BOOL HwbbSts;
	BOOL MotStop;

	/* Local variable initialization */
	AlmStop = &(BeSeq->AlmStop);
	HwbbSts = Cout->HwbbSts;
	MotStop = Cout->MotStop;
/* <S1F5> End */
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm stop mode setting process																*/
/*--------------------------------------------------------------------------------------------------*/
	if( AlmStopMode == STOPMODE_ZEROSTOP )						/* Alarm zero speed stop?				*/
	{
		if( (CtrlMode.b.cm == CTRLMODE_TRQ) ||					/* Torque control in progress?					*/
//			(AlmStop->conf.G2AlmStopMode == TRUE) ||			/* G2ALMStopping method isPn001.0		*/  /* <S1F5> */
			(AlmStop->conf.G2AlmStopMode == GR2STOPMODE_DEFAULT) ||	/* G2ALMStopping method isPn001.0	*/  /* <S1F5> */
			(HwbbSts == TRUE) ||								/* HWBBstate?						*/
			(PhaseReady == FALSE) )								/* Magnetic pole detection in progress?						*/
		{
			/* Pn001.0 */
			AlmStopMode = AlmStop->conf.StopMode;
		}
/* <S1F5> Start */
		else
		{	/* G2ALMStopping method	*/
			AlmStopMode = AlmStop->conf.G2AlmStopMode;
		}
/* <S1F5> End */
	}
	AlmStop->var.AlmStopMode = AlmStopMode;
#if 0	/* <S1F5> Start	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm: Zero speed stop request signal																*/
/*--------------------------------------------------------------------------------------------------*/
	if( AlmStopMode == STOPMODE_ZEROSTOP )
	{ /* Abnormal stop mode : Zero speed stop */
		if( MotStop )
		{ /* Motor stopped */
			/* Zero speed stop completion flagON */
			AlmStop->var.AlmZstopFinish = TRUE;
		}

		if( AlmStop->var.AlmZstopFinish == FALSE )
		{
			/* Zero speed stop request flagON */
			AlmStop->var.AlmZeroStop = TRUE;
		}
		else
		{
			/* Zero speed stop request flagOFF */
			AlmStop->var.AlmZeroStop = FALSE;
		}
	}
	else
	{ /* Abnormal stop mode : Other than zero speed stop */
		AlmStop->var.AlmZeroStop = FALSE;
		AlmStop->var.AlmZstopFinish = FALSE;
	}
#endif	/* <S1F5> End	*/
/* <S1F5> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm stop sequence																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( BeSeq->LdstpTimeZero == TRUE )
	{	/* Pn30A = 0, zero speed stop will occur. However, it was set asPn406is valid.	*/
		if( AlmStop->var.AlmStopMode == GR2STOPMODE_LDSTOPDB )
		{
			AlmStop->var.AlmStopMode = GR2STOPMODE_ZSTOPDB;
		}
		else if( AlmStop->var.AlmStopMode == GR2STOPMODE_LDSTOPFR )
		{
			AlmStop->var.AlmStopMode = GR2STOPMODE_ZSTOPFR;
		}
		else
		{/* do nothing */
			;
		}
	}
	
	switch( AlmStop->var.Gr2StopStep )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:	/* Initial Step																			*/
/*--------------------------------------------------------------------------------------------------*/
		AlmStop->var.Gr2TrqLimit = FALSE;
		AlmStop->var.Gr2ZeroStop = FALSE;
		AlmStop->var.Gr2LinDecStop = FALSE;
		AlmStop->var.Gr2DbopReq = GR2DBOP_NONE;
		AlmStop->var.Gr2StopStep = 1;
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 1:	/* AlmBranching by stop mode																*/
/*--------------------------------------------------------------------------------------------------*/
		switch( AlmStop->var.AlmStopMode )
		{
			case GR2STOPMODE_ZSTOPDB:			/* Zero speed stop &DBStopped state							*/
			case GR2STOPMODE_ZSTOPFR:			/* Zero speed stop & free run state						*/
				AlmStop->var.Gr2TrqLimit = TRUE;
				AlmStop->var.Gr2ZeroStop = TRUE;
				AlmStop->var.Gr2DbopReq = GR2DBOP_NONE;
				AlmStop->var.Gr2StopStep = 2;
				break;
			case GR2STOPMODE_LDSTOPDB:			/* Pn30A&DBStopped state		*/
			case GR2STOPMODE_LDSTOPFR:			/* Pn30AThe motor decelerates to a stop and coasts according to the	*/
				AlmStop->var.Gr2LinDecStop = TRUE;
				AlmStop->var.Gr2DbopReq = GR2DBOP_NONE;
				AlmStop->var.Gr2StopStep = 3;
				break;
			default:
				break;
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 2:	/* Zero speed stop, alarm stop release/motor stop wait									*/
/*--------------------------------------------------------------------------------------------------*/
		if( (AlmStop->var.AlmStopMode != GR2STOPMODE_ZSTOPDB)
		 && (AlmStop->var.AlmStopMode != GR2STOPMODE_ZSTOPFR) )
		{
			AlmStop->var.Gr2TrqLimit = FALSE;
			AlmStop->var.Gr2ZeroStop = FALSE;
			AlmStop->var.Gr2DbopReq = GR2DBOP_NONE;
			AlmStop->var.Gr2StopStep = 1;
		}
		else if( MotStop )
		{
			AlmStop->var.Gr2ZeroStop = FALSE;
			if( AlmStop->var.AlmStopMode == GR2STOPMODE_ZSTOPFR )
			{
				AlmStop->var.Gr2DbopReq = GR2DBOP_FREE;
			}
		}
		else
		{/* do nothing */
			;
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 3:	/* During linear deceleration and stop, cancel alarm stop/wait for motor stop									*/
/*--------------------------------------------------------------------------------------------------*/
		if( (AlmStop->var.AlmStopMode != GR2STOPMODE_LDSTOPDB)
		 && (AlmStop->var.AlmStopMode != GR2STOPMODE_LDSTOPFR) )
		{
			AlmStop->var.Gr2LinDecStop = FALSE;
			AlmStop->var.Gr2DbopReq = GR2DBOP_NONE;
			AlmStop->var.Gr2StopStep = 1;
		}
		else if( MotStop )
		{
			AlmStop->var.Gr2LinDecStop = FALSE;
			if( AlmStop->var.AlmStopMode == GR2STOPMODE_LDSTOPFR )
			{
				AlmStop->var.Gr2DbopReq = GR2DBOP_FREE;
			}
		}
		else
		{/* do nothing */
			;
		}
		break;
	default:
		break;
	}
/* <S1F5> End */
}

/* <S1F5> Start */
/****************************************************************************************************/
/*																									*/
/*		Forced stop mode setting process																		*/
/*																									*/
/****************************************************************************************************/
void BeSeqForceStopMode( BE_SEQ_HNDL *BeSeq, CTRLMD CtrlMode, BOOL MotStop, BOOL FstpSts )
{
	FORCE_STOP *ForceStop;
	ALM_STOP *AlmStop;

	/* Local variable initialization */
	ForceStop = &( BeSeq->ForceStop );
	AlmStop = &( BeSeq->AlmStop );

	switch( ForceStop->var.FstpStopStep )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:	/* Initial Step																			*/
/*--------------------------------------------------------------------------------------------------*/
		ForceStop->var.FstpTrqLimit = FALSE;
		ForceStop->var.FstpZeroStop = FALSE;
		ForceStop->var.FstpLinDecStop = FALSE;
		ForceStop->var.FstpDbopReq = FSTPDBOP_NONE;
		ForceStop->var.FstpMode = STOPMODE_NOTSTOP;
		ForceStop->var.FstpStopStep = 1;
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 1:	/* Forced stop monitoring (Non-forced stop)													*/
/*--------------------------------------------------------------------------------------------------*/
		if( FstpSts == TRUE )
		{
			if( (CtrlMode.b.cm == CTRLMODE_TRQ)
			 || (ForceStop->conf.ForceStopMode == FSTOPMODE_DEFAULT) )
			{
				ForceStop->var.FstpMode = AlmStop->conf.StopMode;

				break;
			}
			else
			{
				ForceStop->var.FstpMode = ForceStop->conf.ForceStopMode;

				if( BeSeq->LdstpTimeZero == TRUE )
				{	/* Pn30A = 0, zero speed stop will occur. However, it was set asPn406is valid.	*/
					if( ForceStop->var.FstpMode == FSTOPMODE_LDSTOPDB )
					{
						ForceStop->var.FstpMode = FSTOPMODE_ZSTOPDB;
					}
					else if( ForceStop->var.FstpMode == FSTOPMODE_LDSTOPFR )
					{
						ForceStop->var.FstpMode = FSTOPMODE_ZSTOPFR;
					}
					else
					{/* do nothing */
						;
					}
				}
			}
			
			switch( ForceStop->var.FstpMode )
			{
			case FSTOPMODE_ZSTOPDB:
			case FSTOPMODE_ZSTOPFR:
				ForceStop->var.FstpTrqLimit = TRUE;
				ForceStop->var.FstpZeroStop = TRUE;
				ForceStop->var.FstpDbopReq = FSTPDBOP_NONE;
				ForceStop->var.FstpStopStep = 2;
				break;
			case FSTOPMODE_LDSTOPDB:
			case FSTOPMODE_LDSTOPFR:
				ForceStop->var.FstpLinDecStop = TRUE;
				ForceStop->var.FstpDbopReq = FSTPDBOP_NONE;
				ForceStop->var.FstpStopStep = 3;
				break;
			default:
				break;
			}
		}
		else
		{
			ForceStop->var.FstpMode = STOPMODE_NOTSTOP;
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 2:	/* Zero speed stop, forced stop release/motor stop wait											*/
/*--------------------------------------------------------------------------------------------------*/
		if( FstpSts == FALSE )
		{
			ForceStop->var.FstpTrqLimit = FALSE;
			ForceStop->var.FstpZeroStop = FALSE;
			ForceStop->var.FstpDbopReq = FSTPDBOP_NONE;
			ForceStop->var.FstpStopStep = 1;
		}
		else if( MotStop )
		{
			ForceStop->var.FstpZeroStop = FALSE;
			if( ForceStop->var.FstpMode == FSTOPMODE_ZSTOPFR )		/* <S1FF> */
			{
				ForceStop->var.FstpDbopReq = FSTPDBOP_FREE;
			}
		}
		else
		{/* do nothing */
			;
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 3:	/* During linear deceleration and stop, release forced stop/wait for motor stop											*/
/*--------------------------------------------------------------------------------------------------*/
		if( FstpSts == FALSE )
		{
			ForceStop->var.FstpLinDecStop = FALSE;
			ForceStop->var.FstpDbopReq = FSTPDBOP_NONE;
			ForceStop->var.FstpStopStep = 1;
		}
		else if( MotStop )
		{
			ForceStop->var.FstpLinDecStop = FALSE;
			if( ForceStop->var.FstpMode == FSTOPMODE_LDSTOPFR )		/* <S1FF> */
			{
				ForceStop->var.FstpDbopReq = FSTPDBOP_FREE;
			}
		}
		else
		{/* do nothing */
			;
		}
		break;
	default:
		break;
	}
}
/* <S1F5> End */

/****************************************************************************************************/
/*																									*/
/*		mechanical brake sequence(BK sequence)												*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Basic for brake command processing and DB sequence when using a brake motor			*/
/*			Perform enable command processing.																*/
/*			Servo off timing when brake is on, brake timing setting when motor is stopped			*/
/*			Rotation speed and time can be set using user constants.											*/
/*			OT, base enable/base block commands for servo operation are				*/
/*			It is commanded from this sequence.															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
BOOL BeSeqMechaBrakeSequence( BE_SEQ_HNDL *BeSeq, CTRLMD CtrlMode,
    	                       LONG AbsMotSpd, BOOL HwbbStatus, USHORT faultState,
    						   BOOL BeComplete, BOOL MotorLessTestMode)								/* <S1B7> */
{
	BK_SEQ_DATA 	*BkSeqData;
//	ALM_STOP 		*alm_stop_ptr;  /* <S1F5> */
	ALM_STOP 		*AlmStop;  /* <S1F5> */
	FORCE_STOP		*ForceStop;  /* <S1F5> */

	/* Local variable initialization */
	BkSeqData = &BeSeq->BkSeqData;
//	alm_stop_ptr = &BeSeq->AlmStop;  /* <S1F5> */
	AlmStop = &BeSeq->AlmStop;  /* <S1F5> */
	ForceStop = &BeSeq->ForceStop;  /* <S1F5> */

/*--------------------------------------------------------------------------------------------------*/
/*		Brake sequence processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch( BkSeqData->var.BkStep )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 0:	/* Initial Step																			*/
/*--------------------------------------------------------------------------------------------------*/
		BkSeqData->var.Brake = TRUE;					/* Brake command output							*/
		BkSeqData->var.BkZeroStop = FALSE;				/* Zero speed stop request off						*/
		BkSeqData->var.BkLinDecStop = FALSE;			/* Linear deceleration stop request off						*/	/* <S1F5> */
		BkSeqData->var.BkWait = 0;
		BkSeqData->var.BkStep = 1;
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 1:	/* Servo off and waiting for servo to turn on										*/
/*--------------------------------------------------------------------------------------------------*/
		if( BeSeq->SvonReq && BeSeq->SvReadyPhaseIgnore )
		{
			/* Base Enable Request ON */
			BkSeqData->var.BkBeReq = TRUE;

			/* Changes judgment method depending on motorless test mode status *//* <S1B7> */
			if( MotorLessTestMode == FALSE )	/* Motorless mode disabled */
			{
				if( (faultState & SHAL_FAULT_BB) == 0 )
				{
					/* Brake Release */
					BkSeqData->var.Brake = FALSE;
					BkSeqData->var.BkWait = 0;
					BkSeqData->var.BkStep = 2;
				}
			}
			else								/* Motorless mode enabled */
			{
				if( BeComplete == TRUE )	/* Base enable */
				{
					/* Brake Release */
					BkSeqData->var.Brake = FALSE;
					BkSeqData->var.BkWait = 0;
					BkSeqData->var.BkStep = 2;
				}
			}
		}
		else
		{
			/* Base Enable Request OFF */
			BkSeqData->var.BkBeReq = FALSE;
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 2: /* Servo on and waiting for servo off or servo ready off						*/
/*--------------------------------------------------------------------------------------------------*/
		if( BeSeq->SvonReq && BeSeq->SvReadyPhaseIgnore )
		{ /* Servo-on request */
			;											/* Servo on request off wait					*/
		}
//		else if( alm_stop_ptr->var.AlmZeroStop )  /* <S1F5> */
		else if( (AlmStop->var.Gr2ZeroStop == TRUE)			/* <S1F5> */
			|| (ForceStop->var.FstpZeroStop == TRUE))		/* <S1F5> */
		{ /* Alarm zero speed stop or forced stop zero speed stop */		/* <S1F5> */
			;											/* Alarm and forced stop: Waiting for off during zero speed stop	*/   /* <S1FS1F5> */
		}
/* <S1F5> Start */
		else if( (AlmStop->var.Gr2LinDecStop == TRUE)
			|| (ForceStop->var.FstpLinDecStop == TRUE) )
		{
			BkSeqData->var.BkStep = 3;
		}
/* <S1F5> End */
		else if( AbsMotSpd >= BkSeqData->conf.BkonSpdLevel )
		{ /* Motor speed is greater than brake command */			/* Output speed(Pn507)or above					*/
			BkSeqData->var.BkBeReq = FALSE;				/* Without applying brake						*/
			BkSeqData->var.BkStep = 4;					/* Base enable request off					*/	/* <S1F5> */
		}
//		else if( (alm_stop_ptr->var.AlmStopMode > STOPMODE_ZEROSTOP)  /* <S1F5> */
		else if( (AlmStop->var.AlmStopMode >= STOPMODE_DBSTOPDB)  /* <S1F5> */
			  || (ForceStop->var.FstpMode >= STOPMODE_DBSTOPDB)   /* <S1F5> */
				|| (BkSeqData->conf.BkonDelayTime == 0)
				|| (HwbbStatus == TRUE) )
		{												/* Alarm other than zero speed stop or HWBBStatus		*/
			BkSeqData->var.BkBeReq = FALSE;				/* or Servo off delay time is zero				*/
			BkSeqData->var.Brake = TRUE;				/* Base enable request off and				*/
			BkSeqData->var.BkStep = 1;					/* Output the brake commands simultaneously					*/
		}
		else											/* Alarm: None or Zero speed stop				*/
		{												/* and Servo off wait time > 0				*/
			BkSeqData->var.Brake = TRUE;				/* Output the brake command,					*/
			if( CtrlMode.b.cm == CTRLMODE_TRQ )			/* During torque control,							*/
			{											/* Continue control while waiting for base block			*/
				BkSeqData->var.BkZeroStop = FALSE;
			}
			else
			{											/* Zero speed stop while waiting for base block		*/
				BkSeqData->var.BkZeroStop = TRUE;
			}
			BkSeqData->var.BkStep = 5;			/* <S1F5> */
		}
		break;
/* <S1F5> Start */
/*--------------------------------------------------------------------------------------------------*/
	case 3: /* Servo on and waiting for servo off or servo ready off	<S1F5>					*/
/*--------------------------------------------------------------------------------------------------*/
		if( BeSeq->SvonReq && BeSeq->SvReadyPhaseIgnore )
		{ /* Servo-on request */
			;											/* Servo on request off wait					*/
		}
		else if( (AlmStop->var.Gr2LinDecStop == TRUE)
			|| (ForceStop->var.FstpLinDecStop == TRUE))
		{ /* Forced stop Linear deceleration and stop */
			;										/* Alarm and forced stop: Waiting for off during linear deceleration and stop	*/
		}
		else if( (AlmStop->var.Gr2ZeroStop == TRUE)
			|| (ForceStop->var.FstpZeroStop == TRUE) )
		{
			BkSeqData->var.BkStep = 2;
		}
		else if( AbsMotSpd >= BkSeqData->conf.BkonSpdLevel )
		{ /* Motor speed is greater than brake command */			/* Output speed(Pn507)or above					*/
			BkSeqData->var.BkBeReq = FALSE;				/* Without applying brake						*/
			BkSeqData->var.BkStep = 4;					/* Base enable request off					*/
		}
		else if( (AlmStop->var.AlmStopMode >= STOPMODE_DBSTOPDB)
				|| (ForceStop->var.FstpMode >= STOPMODE_DBSTOPDB)
				|| (BkSeqData->conf.BkonDelayTime == 0)
				|| (HwbbStatus == TRUE) )
		{												/* Alarm other than zero speed stop or HWBBStatus		*/
			BkSeqData->var.BkBeReq = FALSE;				/* or Servo off delay time is zero				*/
			BkSeqData->var.Brake = TRUE;				/* Base enable request off and				*/
			BkSeqData->var.BkStep = 1;					/* Output the brake commands simultaneously					*/
		}
		else											/* Alarm: None or Zero speed stop				*/
		{												/* and Servo off wait time > 0				*/
			BkSeqData->var.Brake = TRUE;				/* Output the brake command,					*/
			if( CtrlMode.b.cm == CTRLMODE_TRQ )			/* During torque control,							*/
			{											/* Continue control while waiting for base block			*/
				BkSeqData->var.BkLinDecStop = FALSE;
			}
			else
			{										/* Linear deceleration stop stop while waiting for base block		*/
				BkSeqData->var.BkLinDecStop = TRUE;
			}
			BkSeqData->var.BkStep = 5;
		}
		break;
/* <S1F5> End */
/*--------------------------------------------------------------------------------------------------*/
	case 4:	/* Servo off during motor operation, waiting for motor speed to decrease					*/   /*  */
/*--------------------------------------------------------------------------------------------------*/
		BkSeqData->var.BkWait += KPI_SCCYCLEMS;
		if( (BkSeqData->var.BkWait >= BkSeqData->conf.BkonWaitTime)
			|| (AbsMotSpd <= BkSeqData->conf.BkonSpdLevel) )
		{												/* Elapsed brake command waiting time or				*/
			BkSeqData->var.Brake = TRUE;				/* Motor speed is brake command output speed			*/
			BkSeqData->var.BkWait = 0;					/* or less							*/
			BkSeqData->var.BkStep = 1;					/* Output the brake command					*/
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 5: /* Servo off while motor is stopped, waiting for BB until brake is applied				*/   /* <S1F5> */
/*--------------------------------------------------------------------------------------------------*/
		BkSeqData->var.BkWait += KPI_SCCYCLEMS;
		if( (BkSeqData->var.BkWait >= BkSeqData->conf.BkonDelayTime)
//			|| (alm_stop_ptr->var.AlmStopMode > STOPMODE_ZEROSTOP)   /* <S1F5> */
			|| (HwbbStatus == TRUE) )
		{												/* Servo off delay time elapsed					*/
			BkSeqData->var.BkBeReq = FALSE;				/* When an alarm other than zero speed stop occurs			*/
			BkSeqData->var.BkZeroStop = FALSE;			/* Base enable request off					*/
			BkSeqData->var.BkWait = 0;					/* and Zero stop request off					*/
			BkSeqData->var.BkStep = 1;					/*											*/
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	default: /*	This case is a sequence software error														*/
/*--------------------------------------------------------------------------------------------------*/
		KpiSystemError( );
		break;
	}

	return BkSeqData->var.BkBeReq;
}



/****************************************************************************************************/
/*																									*/
/*		Overtravel state detection processing																*/
/*																									*/
/****************************************************************************************************/
BOOL BeSeqDetectOTStatus( OT_SEQ_DATA *OverTravelSeq, BASE_CTRL_OUT *BaseCtrlOut, BOOL PotSig, BOOL NotSig )
{
	CTRLMD	*CtrlMode;
	LONG	lwk, SpdRef;

	CtrlMode = &BaseCtrlOut->CtrlModeOut;

/*--------------------------------------------------------------------------------------------------*/
/*		OTDetection permission determination process																			*/
/*--------------------------------------------------------------------------------------------------*/
	/*	When table operation is at zero speed and zero clampOTDoes not detect.										*/
	if( BaseCtrlOut->TblZclamp == TRUE )
	{
		/* OTDetection prohibited */
		OverTravelSeq->var.OverTrvlSts = FALSE;
		return FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		OTDetection processing																					*/
/*--------------------------------------------------------------------------------------------------*/
	switch( CtrlMode->b.cm )
	{
/*--------------------------------------------------------------------------------------------------*/
	case CTRLMODE_SPD:	/* Speed control mode 															*/
/*--------------------------------------------------------------------------------------------------*/
		/* Speed command acquisition */
		SpdRef = BaseCtrlOut->OutputSpdRef;

		if( (SpdRef >= 0) && PotSig )
		{ /* Speed command:Positive side && POTStatus */
			OverTravelSeq->var.OverTrvlSts = TRUE;
		}
		else if( (SpdRef <= 0) && NotSig )
		{ /* Speed command:Negative side && NOTStatus */
			OverTravelSeq->var.OverTrvlSts = TRUE;
		}
		else
		{ /* OTNo status */
			OverTravelSeq->var.OverTrvlSts = FALSE;
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case CTRLMODE_POS:	/* Position control mode															*/
/*--------------------------------------------------------------------------------------------------*/
		lwk = BaseCtrlOut->PositionError;
		if( lwk == 0 )
		{ /* Without position deviation */
			if( PotSig || NotSig )
			{ /* POTStatus or NOTStatus */
				OverTravelSeq->var.OverTrvlSts = TRUE;
			}
			else
			{ /* OTNo status */
				OverTravelSeq->var.OverTrvlSts = FALSE;
			}
		}
		else
		{ /* With position deviation */

			SpdRef = BaseCtrlOut->OutputSpdRef;
			if( (((lwk > 0) || (SpdRef >= 0)) && PotSig) )
			{ /* (Position deviation:Positive side || Speed command:Positive side) && POTStatus */
				OverTravelSeq->var.OverTrvlSts = TRUE;
			}
			else if( (((lwk < 0) || (SpdRef <= 0)) && NotSig) )
			{ /* (Position deviation:Negative side || Speed command:Negative side) && NOTStatus */
				OverTravelSeq->var.OverTrvlSts = TRUE;
			}
			else
			{ /* OTNo status */
				OverTravelSeq->var.OverTrvlSts = FALSE;
			}
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	case CTRLMODE_TRQ:	/* Torque control control mode														*/
/*--------------------------------------------------------------------------------------------------*/
		lwk = BaseCtrlOut->TrqCtrlRef;
		if( ((lwk >= 0) && PotSig) || ((lwk <= 0) && NotSig) )
		{ /* POTStatus or NOTStatus */
			OverTravelSeq->var.OtOffCnt = 0;
			if( OverTravelSeq->var.OtOnCnt >= KPI_SCANC_MS( 10 ) )
			{
				OverTravelSeq->var.OverTrvlSts = TRUE;
			}
			else
			{
				OverTravelSeq->var.OtOnCnt++;
//				OverTravelSeq->var.OverTrvlSts = FALSE; /* <S0B7> */
			}
		}
		else
		{
			OverTravelSeq->var.OtOnCnt = 0;
			if( OverTravelSeq->var.OtOffCnt >= KPI_SCANC_MS( 10 ) )
			{
				OverTravelSeq->var.OverTrvlSts = FALSE;
			}
			else
			{
				OverTravelSeq->var.OtOffCnt++;
//				OverTravelSeq->var.OverTrvlSts = TRUE; /* <S0B7> */
			}
		}
		break;
/*--------------------------------------------------------------------------------------------------*/
	default:	/* JOGMode,etc																	*/
/*--------------------------------------------------------------------------------------------------*/
		OverTravelSeq->var.OverTrvlSts = FALSE;
		break;
	}
	return OverTravelSeq->var.OverTrvlSts;
}



/****************************************************************************************************/
/*																									*/
/*		Overtravel sequence(OTSequence)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Perform emergency stop processing when the stroke end of the machine is exceeded.								*/
/*			Stop mode is determined by user constant settings3Can be selected.									*/
/*																									*/
/*	Return value:OTBase enable request from sequence													*/
/*																									*/
/****************************************************************************************************/
// BOOL BeSeqOverTravelSequence( OT_SEQ_DATA *OverTravelSeq, LONG CtrlMode, BOOL MotStop, BOOL BeComplete )  /* <S1F5> */
BOOL BeSeqOverTravelSequence( OT_SEQ_DATA *OverTravelSeq, LONG CtrlMode, BOOL MotStop,     /* <S1F5> */
	                                                      BOOL BeComplete, BOOL LdstpTimeZero )    /* <S1F5> */
{
	LONG	CtrlMode2Lst;
	LONG	OtStopMode;
/*--------------------------------------------------------------------------------------------------*/
/*		OTSequenceNetWork I/FSave control mode previous value												*/
/*--------------------------------------------------------------------------------------------------*/
	OtStopMode = OverTravelSeq->conf.OtStopMode;
	CtrlMode2Lst = OverTravelSeq->var.CtrlMode2;
	OverTravelSeq->var.CtrlMode2 = CtrlMode;

/*--------------------------------------------------------------------------------------------------*/
/*		OTSequence processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch( OverTravelSeq->var.OtStep )
	{
/*----------------------------------------------------------------------------------------------*/
	case 0: /* Initial Step																		*/
/*----------------------------------------------------------------------------------------------*/
		OverTravelSeq->var.OtBeReq = TRUE;
		OverTravelSeq->var.OtTrqLimit = FALSE;
		OverTravelSeq->var.OtZeroStop = FALSE;
		OverTravelSeq->var.OtLinDecStop = FALSE;   /* <S1F5> */
		OverTravelSeq->var.OtZeroClamp = FALSE;
		OverTravelSeq->var.OtDbopReq = OTDBOP_NONE;
		OverTravelSeq->var.OtWait = 0;
		OverTravelSeq->var.OtStep = 1;
		break;
/*----------------------------------------------------------------------------------------------*/
	case 1: /* OTMonitor (OTstate of not being)															*/
/*----------------------------------------------------------------------------------------------*/
		if( OverTravelSeq->var.OverTrvlSts == TRUE )	/* OTCheck							*/
		{ /* OTStatus */
			/* 2011.04.04 Y.Oka <V680> */
			if( (BeComplete == TRUE) && (OverTravelSeq->conf.OtWarningSw == TRUE) )
			{ /* BaseEnable && OTWarning detected */
				OverTravelSeq->var.OtWarningDet = TRUE;
				OverTravelSeq->var.OtWarngWaitCnt = 0;
			}
		/*--------------------------------------------------------------------------------------*/
			/* During torque controlOTStopping methodDBStopping											*/
			if( CtrlMode == CTRLMODE_TRQ )
			{ /* Torque control mode */
				OverTravelSeq->var.OtBeReq = FALSE;		/* In torque control modeOTStopping isDBStop	*/
				OverTravelSeq->var.OtDbopReq = OTDBOP_DEF;
				OverTravelSeq->var.OtStep = 2;
				break;
			}
		/*--------------------------------------------------------------------------------------*/
			switch( OtStopMode )						/* OTBranching by stop mode				*/
			{
			case OTSTOPMODE_DEFAULT:					/* Standard stop mode(Pn001.0)				*/
				OverTravelSeq->var.OtBeReq = FALSE;
				OverTravelSeq->var.OtDbopReq = OTDBOP_DEF;
				OverTravelSeq->var.OtStep = 2;
				break;
			case OTSTOPMODE_ZSTOPZC:					/* Zero speed stop & servo lock				*/
				OverTravelSeq->var.OtTrqLimit = TRUE;
				OverTravelSeq->var.OtZeroStop = TRUE;
				OverTravelSeq->var.OtDbopReq = OTDBOP_NONE;
				OverTravelSeq->var.OtStep = 3;
				break;
			case OTSTOPMODE_ZSTOPFR:					/* Zero speed stop & free run				*/
				OverTravelSeq->var.OtTrqLimit = TRUE;
				OverTravelSeq->var.OtZeroStop = TRUE;
/* <S1F5> Start */
				OverTravelSeq->var.OtDbopReq = OTDBOP_NONE;
				OverTravelSeq->var.OtStep = 5;
				break;
			case OTSTOPMODE_LDSTOPZC:					/* Linear deceleration stop & servo lock			*/
				if( LdstpTimeZero == TRUE )
				{	/* Pn30A = 0, zero speed stop will occur. However, it was set asPn406is valid.	*/
					OverTravelSeq->var.OtTrqLimit = TRUE;
					OverTravelSeq->var.OtZeroStop = TRUE;
				}
				else
				{
					OverTravelSeq->var.OtLinDecStop = TRUE;
				}
				OverTravelSeq->var.OtDbopReq = OTDBOP_NONE;
				OverTravelSeq->var.OtStep = 3;
				break;
			case OTSTOPMODE_LDSTOPFR:					/* Linear deceleration stop & free run				*/
				if( LdstpTimeZero == TRUE )
				{	/* Pn30A = 0, zero speed stop will occur. However, it was set asPn406is valid.	*/
					OverTravelSeq->var.OtTrqLimit = TRUE;
					OverTravelSeq->var.OtZeroStop = TRUE;
				}
				else
				{
					OverTravelSeq->var.OtLinDecStop = TRUE;
				}
/* <S1F5> End */
				OverTravelSeq->var.OtDbopReq = OTDBOP_NONE;
				OverTravelSeq->var.OtStep = 5;
				break;
			default:
				 break;
			}
		}
		/* 2011.04.04 Y.Oka <V680> */
		else
		{ /* OTIf not in status */
			if( OverTravelSeq->var.OtWarngWaitCnt > KPI_SCANC_MS(1000) )
			{/* OTWhen warning is detected,1Hold and clear for 58 seconds */
				OverTravelSeq->var.OtWarningDet = FALSE;
			}
			else
			{
				OverTravelSeq->var.OtWarngWaitCnt++;
			}
		}
		break;
/*----------------------------------------------------------------------------------------------*/
	case 2: /* While stopping with standard stop method,OTWaiting for release/Waiting for motor stop									*/
/*----------------------------------------------------------------------------------------------*/
		if( OverTravelSeq->var.OverTrvlSts == FALSE )		/* OTCancellation check					*/
		{
			OverTravelSeq->var.OtWait++;
			if( OverTravelSeq->var.OtWait >= KPI_SCANC_MS( 20 ) )
			{ /* OTChattering prevention */
				OverTravelSeq->var.OtBeReq = TRUE;			/* Base enable request on			*/
				OverTravelSeq->var.OtDbopReq = OTDBOP_NONE;	/* DBCancel operation request					*/
				OverTravelSeq->var.OtWait = 0;
				OverTravelSeq->var.OtStep = 1;
			}
		}
//	<2010.11.17>Changing control mode	
//		else if((( OverTravelSeq->var.CtrlMode2 == POS_CTRL_MODE ) || ( OverTravelSeq->var.CtrlMode2 == SPD_CTRL_MODE ))
//				&& ( CtrlMode2Lst == NO_CTRL_MODE ) 
//				&& ( OtStopMode == OTSTOPMODE_ZSTOPZC ))
		else if((CtrlMode != CTRLMODE_TRQ)
				&& ( CtrlMode2Lst == CTRLMODE_TRQ ) 
				&& ( OtStopMode == OTSTOPMODE_ZSTOPZC ))
		{													/* Zero speed stop & servo lock			*/
			OverTravelSeq->var.OtBeReq = TRUE;				/* Base enable request on			*/
			OverTravelSeq->var.OtTrqLimit = TRUE;
			OverTravelSeq->var.OtZeroStop = TRUE;
			OverTravelSeq->var.OtDbopReq = OTDBOP_NONE;
			OverTravelSeq->var.OtStep = 3;
		}
		else
		{
			OverTravelSeq->var.OtWait = 0;
		}
		break;
/*----------------------------------------------------------------------------------------------*/
	case 3: /* Zero speed stop,OTWaiting for release/Waiting for motor stop											*/
/*----------------------------------------------------------------------------------------------*/
		if( OverTravelSeq->var.OverTrvlSts == FALSE )		/* OTCancellation check					*/
		{
			OverTravelSeq->var.OtTrqLimit = FALSE;			/* OTis cleared andSTEP-1			*/
			OverTravelSeq->var.OtZeroStop = FALSE;
			OverTravelSeq->var.OtLinDecStop = FALSE;   /* <S1F5> */
			OverTravelSeq->var.OtStep = 1;
		}
		else if( MotStop )									/* Motor stop check				*/
		{
			OverTravelSeq->var.OtZeroStop = FALSE;			/* When the motor is stopped,STEP-5		*/
			OverTravelSeq->var.OtLinDecStop = FALSE;    /* <S1F5> */
			OverTravelSeq->var.OtZeroClamp = TRUE;
			OverTravelSeq->var.OtStep = 4;
		}
		break;
/*----------------------------------------------------------------------------------------------*/
	case 4: /* After zero speed stop, in zero clamp mode,OTWaiting for release									*/
/*----------------------------------------------------------------------------------------------*/
		if( OverTravelSeq->var.OverTrvlSts == FALSE )		/* OTCancellation check					*/
		{
			OverTravelSeq->var.OtTrqLimit = FALSE;			/* OT				*/
			OverTravelSeq->var.OtZeroClamp = FALSE;			/* OTWhen zero clamp is released				*/
			OverTravelSeq->var.OtStep = 1;					/* STEP-1						*/
		}
		break;
/*----------------------------------------------------------------------------------------------*/
	case 5: /* Zero speed stop,OTWaiting for release/Waiting for motor stop											*/
/*----------------------------------------------------------------------------------------------*/
		if( OverTravelSeq->var.OverTrvlSts == FALSE )		/* OTCancellation check					*/
		{
			OverTravelSeq->var.OtTrqLimit = FALSE;			/* OTis cleared andSTEP-1			*/
			OverTravelSeq->var.OtZeroStop = FALSE;
			OverTravelSeq->var.OtLinDecStop = FALSE;     /* <S1F5> */
			OverTravelSeq->var.OtStep = 1;
		}
		else if( MotStop )									/* Motor stop check				*/
		{
			OverTravelSeq->var.OtBeReq = FALSE;				/* When the motor is stopped,STEP-5		*/
			OverTravelSeq->var.OtTrqLimit = FALSE;
			OverTravelSeq->var.OtZeroStop = FALSE;
			OverTravelSeq->var.OtLinDecStop = FALSE;    /* <S1F5> */
			OverTravelSeq->var.OtDbopReq = OTDBOP_FREE;
			OverTravelSeq->var.OtStep = 6;
		}
		break;
/*----------------------------------------------------------------------------------------------*/
	case 6: /* After zero speed stop, in free mode,OTWaiting for release											*/
/*----------------------------------------------------------------------------------------------*/
		if( OverTravelSeq->var.OverTrvlSts == FALSE )		/* OTCancellation check					*/
		{
			/* OTChattering prevention */
			OverTravelSeq->var.OtWait++;
			if( OverTravelSeq->var.OtWait >= KPI_SCANC_MS( 20 ) )
			{												/* Command polarityOTThe condition is	*/
				OverTravelSeq->var.OtBeReq = TRUE;			/* When the command is near zero,OTis chatter	*/
				OverTravelSeq->var.OtDbopReq = OTDBOP_NONE;	/* Because it raises the ring,OTOff		*/
				OverTravelSeq->var.OtWait = 0;				/* Apply filter.				*/
				OverTravelSeq->var.OtStep = 1;
			}
		}
		else
		{
			OverTravelSeq->var.OtWait = 0;
		}
		break;
/*----------------------------------------------------------------------------------------------*/
	default: /*	This case is a sequence software error													*/
/*----------------------------------------------------------------------------------------------*/
		KpiSystemError( );
		break;
	}
	
	return OverTravelSeq->var.OtBeReq;
}



/****************************************************************************************************/
/*																									*/
/*		Servo stop mode command																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : In response to kernel alarm monitoring and stop requests from each sequence process, prioritize			*/
/*			, transfer the reference position to the DB sequence and scan C control mode command			*/
/*			Command stop mode to processing.														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG BeSeqStopModeManager( BE_SEQ_HNDL *BeSeq)
{
	LONG	StopMode;
	BOOL 	ZeroStopReq;
	BOOL 	LinDecStopReq;    /*  */
	LONG 	OtDbopReq;
	LONG	Gr2DbopReq;       /* <S1F5> */
	LONG	FstpDbopReq;    /* <S1F5> */

	StopMode = BeSeq->StopMode;
	OtDbopReq = BeSeq->OtSeqData.var.OtDbopReq;
	Gr2DbopReq = BeSeq->AlmStop.var.Gr2DbopReq;			/* <S1F5> */
	FstpDbopReq = BeSeq->ForceStop.var.FstpDbopReq;		/* <S1F5> */
//	ZeroStopReq = BeSeq->BkSeqData.var.BkZeroStop     /* <S1F5> */
//				| BeSeq->OtSeqData.var.OtZeroStop | BeSeq->ZcCtrlData.ZcZeroStop;   /* <S1F5> */
	ZeroStopReq = BeSeq->BkSeqData.var.BkZeroStop | BeSeq->AlmStop.var.Gr2ZeroStop    /* <S1F5> */
				| BeSeq->ForceStop.var.FstpZeroStop | BeSeq->OtSeqData.var.OtZeroStop   /* <S1F5> */
				| BeSeq->ZcCtrlData.ZcZeroStop;    /* <S1F5> */

	LinDecStopReq = BeSeq->AlmStop.var.Gr2LinDecStop | BeSeq->BkSeqData.var.BkLinDecStop	/* <S1F5> */
				| BeSeq->ForceStop.var.FstpLinDecStop | BeSeq->OtSeqData.var.OtLinDecStop;   /* <S1F5> */
//	if( BeSeq->AlmStop.var.AlmStopMode > STOPMODE_ZEROSTOP )	/* Alarm: Emergency stop request					*/   /* <S1F5> */
//	{          /* <S1F5> */
//		StopMode = BeSeq->AlmStop.var.AlmStopMode;			/* Stop mode other than zero speed stop(DB,etc)		*/   /* <S1F5> */
//	}    /* <S1F5> */
/*--------------------------------------------------------------------------------------------------*/
//	else if( BeSeq->BkSeqData.var.BkBeReq == FALSE )				/* When base enable request is off				*/	/* <S1F5> */
	if( (BeSeq->AlmStop.var.AlmStopMode >= STOPMODE_DBSTOPDB)	/* Alarms:DBStop/Stop free run	*/	/* <S1F5> */
	 || (BeSeq->ForceStop.var.FstpMode >= STOPMODE_DBSTOPDB))	/* Forced stop:DBStop/Stop free run	*/	/* <S1F5> */
	{
		StopMode = BeSeq->AlmStop.conf.StopMode;				/* Stop mode when servo is off					*/
	}
	else if(( BeSeq->BkSeqData.var.BkBeReq == FALSE )				/* When base enable request is off		*/	/* <S1F5> */
		 &&(BeSeq->AlmStop.var.AlmStopMode == STOPMODE_NOTSTOP )	/* Alarm: Non-stop					*/	/* <S1F5> */
		 &&(BeSeq->ForceStop.var.FstpMode == STOPMODE_NOTSTOP ))	/* Forced stop: Non-stop					*/	/* <S1F5> */
	{
		StopMode = BeSeq->AlmStop.conf.StopMode;				/* Stop mode when servo is off					*/
	}
/*--------------------------------------------------------------------------------------------------*/
	else if( (BeSeq->OtSeqData.var.OtBeReq == FALSE) && (OtDbopReq == OTDBOP_DEF) )
	{ /* OTStandard stop method at time */
		if( BeSeq->AlmStop.conf.StopMode == STOPMODE_DBSTOPDB )
		{ /* DBis the motor is stopped,DBContinuation(Pn001.0==0) */
			/* DBis the motor is stopped,DBCancellation(Pn001.0==1) */
			StopMode = STOPMODE_DBSTOPFR;
		}
		else
		{
			/* Stop mode when servo is off */
			StopMode = BeSeq->AlmStop.conf.StopMode;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	else if( ((BeSeq->OtSeqData.var.OtBeReq == FALSE) && (OtDbopReq == OTDBOP_FREE) )  /* <S1F5> */
		  || ((BeSeq->BkSeqData.var.BkBeReq == FALSE) && (Gr2DbopReq == GR2DBOP_FREE))   /* <S1F5> */
		  || ((BeSeq->BkSeqData.var.BkBeReq == FALSE) && (FstpDbopReq == FSTPDBOP_FREE)) )    /* <S1F5> */
	{													/* OTWhen free run stops						*/
		StopMode = STOPMODE_FREERUN;					/* Free run stop mode						*/
	}
/*--------------------------------------------------------------------------------------------------*/
#if 0	/* <S1F5> Start	*/
	else if( BeSeq->AlmStop.var.AlmZeroStop )					/* Alarm: Zero speed stop request				*/
	{
		StopMode = BeSeq->AlmStop.var.AlmStopMode;			/* Zero speed stop mode							*/
	}
#endif	/* <S1F5> End	*/
/*--------------------------------------------------------------------------------------------------*/
	else if( ZeroStopReq )			/* ALM, FSTP, BK, OT, ZCSequence: Zero speed stop request			*/   /* <S1F5> */
	{
		StopMode = STOPMODE_ZEROSTOP;					/* Zero speed stop mode							*/
	}
/* <S1F5> Start */
/*--------------------------------------------------------------------------------------------------*/
	else if( LinDecStopReq )		/* ALM, FSTP, OTSequence: Deceleration stop due to linear deceleration	*/
	{
		StopMode = STOPMODE_LDECSTOP;					/* Deceleration stop mode by linear deceleration			*/
	}
/* <S1F5> */
/*--------------------------------------------------------------------------------------------------*/
	else												/* When base enable request is on				*/
	{
		StopMode = STOPMODE_NOTSTOP;					/* Stop mode release							*/
	}
/*--------------------------------------------------------------------------------------------------*/
	BeSeq->StopMode = StopMode;
	return StopMode;
}



/****************************************************************************************************/
/*																									*/
/*		Dynamic brake sequence(DBSequence)												*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : OTSequence andBKBase enable request from sequence and stop mode command				*/
/*			In response to stop mode command from processing, servo base enable/base block			*/
/*			and the DB on/off timing.											*/
/*																									*/
/*			Stop mode is determined by user constants(Pn001.0).							*/
/*				Pn001.1=0 : DBStop																	*/
/*				Pn001.1=1 : DBStop, after motor stopsDBMotor free run when off							*/
/*				Pn001.1=2 : DBNone, free run stopped													*/
/*																									*/
/*	Supplementary Foot : ASIC_PUMP_ONMediumOCProcessing at the time of occurrence, and interlocking with the base enable request→(STATE5)	*/
/*			DBFor thyristor specifications, before stopping the motorDBOff											*/
/*			Considering the case of using thyristorsysf.dbon, it is also grouped withJL034_000.5(DBON)while receiving data.			*/
/*			By thyristorDB, if the thyristor is turned on once during motor rotation, it is not used inDBONis turned off,DBteeth		*/
/*			continues to work. If the motor is turned off after it has stopped, the motor will			*/
/*			If it moves again, there is a risk that the thyristor will not turn off. Therefore, due to the thyristor				*/
/*			DBDBONAfter next sampling(2ms).										*/
/*			OTWhen the servo is turned off at the same time, it is modified to follow the stopping method when the servo is turned off.						*/
/*			OTof [20] and then turned on again, there is some leeway in processing time due to processing duringOTStop mode(Free),				*/
/*																									*/
/****************************************************************************************************/
BOOL BeSeqDynamicBrakeSequence( BE_SEQ_HNDL *BeSeq, ASICS *SvAsicRegs, LONG MotStop, BOOL EncDataNG,
								BOOL MotorLessTestMode )											/* <S1B7> */
{
	BOOL				DbOnRequest;
	DB_SEQ_DATA 		*DbSeqData;
	PUMP_CTR_STR 		*PumpOnCtrl;
	/* Local variable initialization */
	DbSeqData = &BeSeq->DbSeqData;
	PumpOnCtrl = &BeSeq->ChargePump;
	DbOnRequest = BeSeq->DbOn;

/*--------------------------------------------------------------------------------------------------*/
/*		Creating base enable request																	*/
/*--------------------------------------------------------------------------------------------------*/
	DbSeqData->DbBeReq = BeSeq->BkSeqData.var.BkBeReq & BeSeq->OtSeqData.var.OtBeReq;

/*--------------------------------------------------------------------------------------------------*/
/*		DB&BESequence																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch( DbSeqData->DbStep )
	{
	/*----------------------------------------------------------------------------------------------*/
		case 0: /* Initial Step																		*/
	/*----------------------------------------------------------------------------------------------*/
			/* BaseBlock */
			SHalDrv_BaseBlock(SvAsicRegs->AsicHwReg, SvAsicRegs->AsicMicroReg); /* <S0E4> */

#ifdef CSW_CSTR_DB_REVERSE
			/* Enable DB Control <2010.12.02> */
			SHalDrv_EnableDynamicBrake(SvAsicRegs->AsicHwReg);
#endif

			PumpOnCtrl->PumpOnReq = FALSE;
			BeSeq->BeReqSeqOut = FALSE;

			DbSeqData->DbWait = 0;

			if( BeSeq->StopMode == STOPMODE_FREERUN )
			{
				DbOnRequest = FALSE;
				DbSeqData->DbStep = 1;
			}
			/* DBStop/Free after stop */
			else if( BeSeq->StopMode == STOPMODE_DBSTOPFR )
			{
				if( EncDataNG == TRUE )
				{ /* Encoder dataNGatDBTurn on */
					DbOnRequest = TRUE;
				}
				else
				{
					DbOnRequest = FALSE;
				}
				DbSeqData->DbStep = 1;
			}
			/* DBStop/Continue after stop */
			else
			{
				DbOnRequest = TRUE;
				DbSeqData->DbStep = 6;
			}
			break;

	/*----------------------------------------------------------------------------------------------*/
		case 1: /* Waiting for motor stop : After motor stopDBis turned off.										*/
	/*----------------------------------------------------------------------------------------------*/
		if(( BeSeq->StopMode == STOPMODE_DBSTOPFR )&&( EncDataNG == TRUE ))
											/* Encoder dataNGinsideDBTurn on and stay here			*/
		{									/* Encoder data remains here during initializationNGto	*/
											/* . When polling with low-speed fixed periodic tasks such as while drivingNGis a step6				*/
			DbOnRequest = TRUE;				/* Encoder dataNGinsideDBTurn on and stay here			*/
			PumpOnCtrl->PumpOnReq = FALSE;	/* Encoder dataNGinsideDBTurn on and stay here			*/
											/* Charge pump drive request off -> LpxChargePumpControl	*/
		}									/* Encoder dataNGinsideDBTurn on and stay here			*/
		else if( MotStop )					/* Check motor stop only when encoder data is reliable	*/
		{
			DbOnRequest = FALSE;
			PumpOnCtrl->PumpOnReq = FALSE;	/* Charge pump drive request off -> LpxChargePumpControl 	*/

			DbSeqData->DbWait = 0;
			DbSeqData->DbStep = 2;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
		case 2: /* Waiting for base enable request															*/
	/*				Stopping method =DBNone orDBAfterDBis turned off.					*/
	/*----------------------------------------------------------------------------------------------*/
		/* StopModewill change(BK.SeqandOT.Seq)deathDBStop,DBContinuing state change */
		if( BeSeq->StopMode == STOPMODE_DBSTOPDB )
		{
			DbOnRequest = TRUE;
			DbSeqData->DbWait = 0;
			DbSeqData->DbStep = 6;
		}
		else
		{
			/* Check base enable */
			if( DbSeqData->DbBeReq )
			{
				/* Charge pump drive request on */
				PumpOnCtrl->PumpOnReq = TRUE;
				DbSeqData->DbWait = 0;
				DbSeqData->DbStep = 3;
			}
			/* Encoder data while waiting for base enable requestNGIf the sequence becomes1to	*/
			else if(( BeSeq->StopMode == STOPMODE_DBSTOPFR )&&( EncDataNG == TRUE ))
			{
				DbSeqData->DbStep = 1;
			}
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
		case 3: /* Charge pump control																*/
	/* Control charge pump while checking whether base enable request is continuing					*/
	/*----------------------------------------------------------------------------------------------*/
		/* Base enable request discontinuation processing */
		if( DbSeqData->DbBeReq == FALSE )
		{
			PumpOnCtrl->PumpOnReq = FALSE;
			DbSeqData->DbWait = 0;
			DbSeqData->DbStep = 0;
		}
		/* Base enable request ongoing processing Wait32 = DBRelay off->PumpOnwait until(16)+PumpOnTime(16) */
		else if( PumpOnCtrl->PumpOnComp == TRUE )						/*	Charge pump control complete	*/
		{
			PumpOnCtrl->PumpOnReq = FALSE;

			SHalDrv_ResetAlarmLtStatus(SvAsicRegs->AsicHwReg);

			/* Base enable when not in motorless test mode *//* <S1B7> */
			if( MotorLessTestMode == FALSE )
			{
				SHalDrv_BaseEnable(SvAsicRegs->AsicHwReg, SvAsicRegs->AsicMicroReg);
			}

			BeSeq->BeReqSeqOut = TRUE;
			DbSeqData->DbWait = 0;
			DbSeqData->DbStep = 4;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
		case 4: /* Wait for base block request															*/
	/* (ASICfart)Stay here during base enable command													*/
	/*----------------------------------------------------------------------------------------------*/
		/* Base enable request discontinuation processing */
		if( DbSeqData->DbBeReq == FALSE )
		{
			/* During base block, the detected current value is saturated, soIFBSet the gain to 0	*/
			/* PUMPON, the switching loss of the regenerative transistor will increase if the current command or currentFBIf there is data, it is normalized toPWMof			*/
			/* Duty50%It stops charging and cannot be charged properly.					*/
			/* BaseBlock */
			SHalDrv_BaseBlock(SvAsicRegs->AsicHwReg, SvAsicRegs->AsicMicroReg); /* <S0E4> */

			BeSeq->BeReqSeqOut = FALSE;

			/*--------------------------------------------------------------*/
			/*			Stop mode determination									*/
			/*--------------------------------------------------------------*/
			/* Stop free run */
			if( BeSeq->StopMode == STOPMODE_FREERUN )
			{
				DbOnRequest = FALSE;
				DbSeqData->DbWait = 0;
				DbSeqData->DbStep = 2;
			}
			/* Stop DB/Free after stop */
			else if( BeSeq->StopMode == STOPMODE_DBSTOPFR )
			{
				if( EncDataNG == TRUE )
				{ /* Encoder dataNGIn the case of */
					/*  Encoder dataNGatSTEP1 */
					DbSeqData->DbWait = 0;
					DbSeqData->DbStep = 5;
				}
				/* Stop mode isDBWhen the motor is stopped in the case of free after stopdbondo.*/
				/* 1Not waiting for samplingOTHour command0Measures for relay chatter in the vicinity */
				else if( MotStop )
				{
					/* A.C90(Encoder communication error)When occurring1secondsDBturn on */
					DbOnRequest = FALSE;
					DbSeqData->DbWait = 0;
					DbSeqData->DbStep = 1;
				}
				else
				{
					DbSeqData->DbWait = 0;
					DbSeqData->DbStep = 5;
				}
			}
			/* Stop DB/Continue after stop */
			else
			{
				DbSeqData->DbWait = 0;
				DbSeqData->DbStep = 5;
			}
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
		case 5: /* DB on processing by base block request												*/
	/* Turn on DB one cycle after base block request										*/
	/* ※Does not come here in case of free run stop														*/
	/*----------------------------------------------------------------------------------------------*/
		/*--------------------------------------------------------------*/
		/*			Stop mode determination									*/
		/*--------------------------------------------------------------*/
		/* Stop DB/Free after stop */
		if( BeSeq->StopMode == STOPMODE_DBSTOPFR )
		{
			if( EncDataNG == TRUE )
			{ /* Encoder dataNGIn the case of */
				DbOnRequest = TRUE;				/* Encoder dataNGatDBTurn on and stay here		*/
			}
			else if( MotStop )
			{ /* Check for stop when encoder data is reliable	*/
				/* A.C90(Encoder communication error)When occurringDBturn on */
				DbOnRequest = FALSE;
				DbSeqData->DbWait = 0;
				DbSeqData->DbStep = 1;
			}
			else
			{
				DbOnRequest = TRUE;				/* While the motor is rotating, turn on the DB and wait for it to stop in state 1 	*/
				DbSeqData->DbWait = 0;
				DbSeqData->DbStep = 1;
			}
		}
		/* StopModewill change(BK.SeqandOT.Seq)deathStopModeis newly changed to free  */
		else if( BeSeq->StopMode == STOPMODE_FREERUN )
		{
			DbOnRequest = FALSE;
			DbSeqData->DbWait = 0;
			DbSeqData->DbStep = 1;
		}
		/* Stop DB/Continue after stop */
		else
		{
			DbOnRequest = TRUE;
			DbSeqData->DbWait = 0;
			DbSeqData->DbStep = 6;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
		case 6: /* Base block processing in case of DB stop/continuation after stop									*/
	/* If the motor is stopped and a base enable request comes, turn off the DB									*/
	/*----------------------------------------------------------------------------------------------*/
		if(((( EncDataNG == FALSE ) && MotStop ) && DbSeqData->DbBeReq )
		   || ( BeSeq->StopMode == STOPMODE_FREERUN )	/* orDBStopping release→OTFree run stop exists	 */
		   || ( BeSeq->StopMode == STOPMODE_DBSTOPFR ))	/* orDBStopping release→OTStandard stop (DBFree run after stop)*/
		{
			DbOnRequest = FALSE;
			DbSeqData->DbWait = 0;
			DbSeqData->DbStep = 2;
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
		default: /*	This case is a software error															*/
	/*----------------------------------------------------------------------------------------------*/
		KpiSystemError( );
		break;
	}

	BeSeq->DbOn = DbOnRequest;
	return DbOnRequest;
}

#if (CSW_AUTO_CHARGE_PUMP != FALSE)
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Charge pumpON/OFFControl														*/
/*																									*/
/*	Supplementary Foot : During PWM control(When base enable), the pump ON command is invalidated by the ASIC.				*/
/*			, interlock with base enable is unnecessary											*/
/****************************************************************************************************/
void BeSeqChargePumpControl( ASICS *SvAsicRegs, PUMP_CTR_STR *workptr, BOOL OCState,
								BOOL MotorLessTestMode,BOOL NegPumpReq )/* <S1B7> *//*<S201>*/
{
	ASIC_HW_IF_STR *pAsicHwReg = SvAsicRegs->AsicHwReg;
	ASIC_U_IF_STR *pAsicMicroReg = SvAsicRegs->AsicMicroReg;

/*--------------------------------------------------------------------------------------------------*/
/*		Current detection AD zero adjustment																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( workptr->AdjPumponReq == TRUE )					/* Zero adjustment pump ON request check				*/
	{
		/* When not in motorless test modeON	*//* <S1B7> */
		if( MotorLessTestMode == FALSE )
		{
			SHalDrv_AutoChargePumpON(pAsicHwReg);		/* Continuous charge pumpON */
		}
		workptr->SvonPumpCnt++;
		workptr->PumpOnComp = FALSE;					/* Charge pump incomplete						*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Scan C : Charge Pump Control Request													*/
/*--------------------------------------------------------------------------------------------------*/
	else if( workptr->PumpOnReq == TRUE )				/* Scan C pump ON request check			*/
	{
		if( OCState != FALSE )
		{
			SHalDrv_ChargePumpOFF(pAsicHwReg);
			return;
		}

		switch(workptr->PumpOnSeq)
		{
		case 0:	/* Wait for Relay ON */
			if( KlibGetLongTimerMs(workptr->TimerCnt) > 15 )
			{
				/*------------------------------------------------------------------------------*/
				/* Perform alarm latch reset before base enable 							*/
				/*------------------------------------------------------------------------------*/
				/* KPI_ALARM_CLEAR( );ASIC alarm clear */
				SHalDrv_ResetAlarmLtStatus(pAsicHwReg);
				/*AD conversion digitization filter alarm clear */
				SHalDrv_ResetCrtAdAlarmStatus(pAsicMicroReg);

				workptr->SvonPumpCnt = 0;
				workptr->PumpOnSeq = 1;
				KlibRstLongTimer(&workptr->TimerCnt);

				SHalDrv_StartAutoChargePump(pAsicHwReg);
			}
			break;

		case 1:	/* Charg Pump ON/OFF */
			workptr->SvonPumpCnt++;
			if((0 == SHalDrv_GetAutoChargePumpState(pAsicHwReg))
				|| ( KlibGetLongTimerMs(workptr->TimerCnt) > 16 ))
			{
				/* AD conversion digitization filter alarm clear */
				SHalDrv_ResetCrtAdAlarmStatus(pAsicMicroReg);
				workptr->PumpOnComp = TRUE;			/* Charge pump complete				*/
				workptr->PumpOnSeq = 2;
			}

			break;

		default: /* Complete state */
			break;
		}
	}
/*<S201> Start*/
/*--------------------------------------------------------------------------------------------------*/
/*		ScanC : The Charge pump ON/OF Switch Control	                                                                               */	
/*--------------------------------------------------------------------------------------------------*/
	 else if( NegPumpReq == TRUE ) /*SCAN C: N-Arm ON required to checking*/
	{
		if( workptr->NegPumpCnt <=128 )/*Hardware Requirements*/
		 {
			if( ( workptr->NegPumpCnt & 0x0001 ) == 0 )/*Bit computing*/
			 {
			     SHalDrv_ChargePumpOFF(pAsicHwReg);/*ChargePumpOFF*/
			 }
			 else
			 {
			     SHalDrv_ChargePumpON(pAsicHwReg);/*ChargePumpON*/
			 }
			 workptr->NegPumpCnt++;/*Bit computing*/
		 }
		 else
		 {
		      SHalDrv_ChargePumpON(pAsicHwReg);/*KPX_PUMPON( );*/
		 }
    }
 /*<S201> END*/	
/*--------------------------------------------------------------------------------------------------*/
/*		No request																					*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		KlibRstLongTimer(&workptr->TimerCnt);
		workptr->PumpOnSeq = 0;
		workptr->PumpOnComp = FALSE;						/* Charge pump incomplete					*/
		SHalDrv_ChargePumpOFF(pAsicHwReg);
		workptr->NegPumpCnt = 0;  /*<S201>*/
	}

}

#else	//(CSW_AUTO_CHARGE_PUMP != FALSE)

/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Charge pumpON/OFFControl														*/
/*																									*/
/*	Supplementary Foot : During PWM control(When base enable), the pump ON command is invalidated by the ASIC.				*/
/*			, interlock with base enable is unnecessary											*/
/*	★ScanBtocallBe careful!																*/
/****************************************************************************************************/
void BeSeqChargePumpControl( ASICS *SvAsicRegs, PUMP_CTR_STR *workptr, BOOL OCState,
								BOOL MotorLessTestMode,BOOL NegPumpReq )/* <S1B7> *//*<S201>*/
{
	ASIC_HW_IF_STR *pAsicHwReg = SvAsicRegs->AsicHwReg;
	ASIC_U_IF_STR *pAsicMicroReg = SvAsicRegs->AsicMicroReg;

/*--------------------------------------------------------------------------------------------------*/
/*		Current detection AD zero adjustment																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( workptr->AdjPumponReq == TRUE )					/* Zero adjustment pump ON request check				*/
	{
		if( ( workptr->SvonPumpCnt & 0x0001 ) == 0 )	/* ON/OFF in scan B cycle				*/
		{
			SHalDrv_ChargePumpOFF(pAsicHwReg);			/* Charge pumpOFF						*/
		}
		else
		{
			/* When not in motorless test modeON	*//* <S1B7> */
			if( MotorLessTestMode == FALSE )
			{
				SHalDrv_ChargePumpON(pAsicHwReg);		/* Charge pumpON							*/
			}
		}
		workptr->SvonPumpCnt++;
		workptr->PumpOnComp = FALSE;					/* Charge pump incomplete						*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Scan C : Charge Pump Control Request													*/
/*--------------------------------------------------------------------------------------------------*/
	else if( workptr->PumpOnReq == TRUE )				/* Scan C pump ON request check			*/
	{
		if( OCState != FALSE )
		{
			SHalDrv_ChargePumpOFF(pAsicHwReg);
			return;
		}

		switch(workptr->PumpOnSeq)
		{
		case 0:	/* Wait for Relay ON */
			if( KlibGetLongTimerMs(workptr->TimerCnt) > 15 )
			{
				/*------------------------------------------------------------------------------*/
				/* Perform alarm latch reset before base enable 							*/
				/*------------------------------------------------------------------------------*/
				/* KPI_ALARM_CLEAR( );ASIC alarm clear */
				SHalDrv_ResetAlarmLtStatus(pAsicHwReg);
				/*AD conversion digitization filter alarm clear */
				SHalDrv_ResetCrtAdAlarmStatus(pAsicMicroReg);

				KlibRstLongTimer(&workptr->TimerCnt);
				workptr->SvonPumpCnt = 0;
				workptr->PumpOnSeq = 1;
			}
			break;

		case 1:	/* Charg Pump ON/OFF */
			if( ( workptr->SvonPumpCnt & 0x0001 ) == 0 )
			{
				SHalDrv_ChargePumpOFF(pAsicHwReg);
			}
			else
			{
				/* When not in motorless test modeON	*//* <S1B7> */
				if( MotorLessTestMode == FALSE )
				{
					SHalDrv_ChargePumpON(pAsicHwReg);
				}
			}
			workptr->SvonPumpCnt++;
/* <S094>:ChargePumpWhen completed,PumpOff. */
/* KlibGetLongTimerMsIf it is a function, the timer reset timing orScanBprocessing time etc.PumpOnThere is a possibility that it will end.	*/
#if 0
			if( KlibGetLongTimerMs(workptr->TimerCnt) > 15 )
			{
#else
			if(workptr->SvonPumpCnt > CHARGE_PUMP_CMP_TIME)/* CHARGE_PUMP_CMP_TIME = 80H :16ms */
			{
#endif
				/* AD conversion digitization filter alarm clear */
				SHalDrv_ResetCrtAdAlarmStatus(pAsicMicroReg);
				workptr->PumpOnComp = TRUE;			/* Charge pump complete				*/
				workptr->PumpOnSeq = 2;
			}
			break;

		default: /* Complete state */
			break;
		}
	}
/*<S201> Start*/
/*--------------------------------------------------------------------------------------------------*/
/*		ScanC : The Charge pump ON/OF Switch Control	                                                                               */	
/*--------------------------------------------------------------------------------------------------*/
	 else if( NegPumpReq == TRUE ) /*SCAN C: N-Arm ON required to checking*/
	{
		if( workptr->NegPumpCnt <=128 )/*Hardware Requirements*/
		 {
			if( ( workptr->NegPumpCnt & 0x0001 ) == 0 )/*Bit computing*/
			 {
			     SHalDrv_ChargePumpOFF(pAsicHwReg);/*ChargePumpOFF*/
			 }
			 else
			 {
			     SHalDrv_ChargePumpON(pAsicHwReg);/*ChargePumpON*/
			 }
			 workptr->NegPumpCnt++;/*Bit computing*/
		 }
		 else
		 {
		      SHalDrv_ChargePumpON(pAsicHwReg);/*KPX_PUMPON( );*/
		 }
    }
 /*<S201> END*/	
/*--------------------------------------------------------------------------------------------------*/
/*		No request																					*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		KlibRstLongTimer(&workptr->TimerCnt);
		workptr->PumpOnSeq = 0;
		workptr->PumpOnComp = FALSE;						/* Charge pump incomplete					*/
		SHalDrv_ChargePumpOFF(pAsicHwReg);
		workptr->NegPumpCnt = 0;  /*<S201>*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}
#endif //(CSW_AUTO_CHARGE_PUMP != FALSE)


/*<S201>Start*/	
/****************************************************************************************************/
/*																									*/
/*		Main circuitOFFtimeIPM NSide arm control   Issuing request											         */
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency :                																			*/
/*			IPMofNControl side arm movement.															*/
/*			The control request is checked in scan B.                        							*/
/*																							*/
/****************************************************************************************************/
#define N_PUMP_NONE		0											/* NDisable side arm control			*/
#define N_PUMP_SUSPEND	1											/* NStop side arm control			*/
#define N_PUMP_START	2											/* NStart side arm control			*/
/*--------------------------------------------------------------------------------------------------*/
/******************************************************************************************************/
void BeNegativePumpSequence( POWER_STATUS*   CoutK,   ALMMAN_HANDLE*     Alm,
						     DET_HWBB_STS*   DetHwbb, CHECK_MAIN_POWER*  PowerChecker,
						     DB_SEQ_DATA*    SvSeqV,  SEQ_CTRL_OUT* CoutV1,
						     ASIC_HW_IF_STR* AsicHwReg
						                   )
{
/*--------------------------------------------------------------------------------------------------*/
/*		If any of the above occurs, turn the N side arm OFF												  */
/*		■ACDuring power input			■ACIn power supply detection dead zone	■During base enable						    */
/*		■Alarm occurrence		■HWBBStatus		    ■DCInput mode						           */
/*--------------------------------------------------------------------------------------------------*/
	if( (CoutK->Acon)||
		(!CoutK->AconDetectReady)||
	    (SHalDrv_GetFaultStatus(AsicHwReg, SHAL_FAULT_BB)==0)||
	    (Alm->Status.AlmFlag) || 
	    (DetHwbb->HwbbState) || 
	    (PowerChecker->conf.Dcbus))
	{
		SvSeqV->NegPumpState = N_PUMP_NONE;/* NDisable side arm control*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		By motor stop detection and motor interruption speed level detectionNControl side arm							*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		if( SvSeqV->NegPumpState == N_PUMP_START )
		{  
		    /*When the interruption speed level is detectedNStop side arm control*/
			SvSeqV->NegPumpState = (CoutV1->MotSuspLvl)? N_PUMP_SUSPEND : N_PUMP_START;
		}
		else
		{
			/*	By motor stop detectionNStart side arm control		*/
			SvSeqV->NegPumpState = (CoutV1->MotStop)? N_PUMP_START : N_PUMP_SUSPEND;
		}
    }
	/*--------------------------------------------------------------------------------------------------*/
	/*		NCreate side arm on request																		*/
	/*--------------------------------------------------------------------------------------------------*/
	CoutK->NegPumpReq = (SvSeqV->NegPumpState == N_PUMP_START)? TRUE : FALSE;
}

/*<S201>End*/	

/****************************************************************************************************/
/*																									*/
/*		Zero clamp sequence(ZC sequence)														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Speed control(Pn000.1=0), Speed control⇔Zero clamp control(Pn000.1=A), the external signal input		*/
/*			(/ZCLAMP)is enabled, in order to suppress drift due to speed command offset, etc.,		*/
/*			Function to clamp the motor by creating a position loop with position command = 0.							*/
/*																									*/
/*			This sequence process generates a zero speed stop request from the zero clamp request signal and the motor rotation state.		*/
/*			This is the sequence to command the zero clamp request.											*/
/*			If a request for zero clamp comes while the motor is rotating, the speed command is set to 0, and the speed command is set to 0, and the motor is rotating		*/
/*			.												*/
/*			Actually forming a position loop with position command 0 is done in scan B.								*/
/*																									*/
/*			 1. Check whether origin search is in progress															*/
/*				Does nothing while searching.--> Processed by control mode management									*/
/*																									*/
/*			 2. Confirmation of zero clamp request																*/
/*				Detect the zero clamp request signal from the input signal.										*/
/*				Note) For setting of option boardΣ-Ⅱ.									*/
/*																									*/
/*			 3. Check motor stop status																*/
/*				Check whether the motor is stopped or has already been zero-clamped to form a position loop.		*/
/*				Confirm.																			*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#define	ZCLAMP_MARGIN	12710						/* 2^24 * (5rpm / 6600rpm) = 12710				*/
/*--------------------------------------------------------------------------------------------------*/
void	CpxZeroClampSequence( 						/* Zero Clamp Sequence 							*/
	ZC_CTRL_DATA *Zcv,								/* Zero Clamp Control							*/
	BASE_CTRL_OUT	*Bout,							/* Base Control Output 							*/
	FUN_CMN_CONTROL *Fctrl,							/* Function Control								*/
	SEQ_CTRL_OUT *Cout)								/* Sequence Output 								*/
{
LONG	lwk;
UINT bZclamp = (Bout->CmdSeqBit & ZEROCLAMP_BIT);
#define LPX_CHECK_SPDCTRLMODE( bout )  (((bout)->CtrlModeOut.b.cm) == CTRLMODE_SPD)
#define LPX_CHECK_RNDSVCTRL( fctrl )    ((fctrl)->FnSvControl)
LONG	SpdRefo = Bout->OutputSpdRef;
LONG	ZcSpdLevel = Zcv->P.ZcSpdLevel;				/* <S00C>										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Zero clamp processing request input signal check														*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*	Machine Efficiency : After soft start when zero clamp signal by external input signal is enabled,				*/
/*			Speed ​​command is a parameter(Pn501)				*/
/*			Sets a request flag to start zero clamp sequence processing.								*/
/*			However, in order to prevent chattering near the detection speed, 5rpm			*/
/*			Detection is performed at the preliminary speed, and if it remains below the official detection level, a sequence request			*/
/*			Set flag																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
		if( bZclamp && LPX_CHECK_SPDCTRLMODE(Bout) && (! LPX_CHECK_RNDSVCTRL(Fctrl) ) )
		{
			lwk = MlibABS( SpdRefo );
			if( lwk < (ZcSpdLevel + ZCLAMP_MARGIN) )
			{
				if( lwk < ZcSpdLevel )
				{
					Zcv->ZcReqSignal = TRUE;
				}
			}
			else
			{
				Zcv->ZcReqSignal = FALSE;
			}
		}
		else
		{
			Zcv->ZcReqSignal = FALSE;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Zero clamp sequence																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( (Zcv->ZcReqSignal) 						/* Zero clamp level detection or					*/
												/* Zero clamp command from table operation(TODO:)		*/
		/* || (TblExeV.TblZmd.b.zm==CTRLMODE_ZCLMP) */ )	
	{
		if( (Cout->MotStop) ||					/* Motor stopped or							*/
			(Zcv->ZcZeroClamp))					/* During zero clamp									*/
		{
			Zcv->ZcZeroClamp = TRUE ;			/* Zero clamp request setting 							*/
			Zcv->ZcZeroStop = FALSE ;			/* Zero speed stop request release setting 							*/
		}
		else
		{
			Zcv->ZcZeroClamp = FALSE ;			/* Zero clamp request release setting 						*/
			Zcv->ZcZeroStop = TRUE ;			/* Zero speed stop request setting 								*/
		}
	}
	else
	{
			Zcv->ZcZeroClamp = FALSE ;			/* Zero clamp request release setting 						*/
			Zcv->ZcZeroStop = FALSE ;			/* Zero speed stop request release setting 							*/
	}
/*--------------------------------------------------------------------------------------------------*/
		return;

}

/****************************************************************************************************/
/*																									*/
/* 	<S0A7> Check Servo On Command Error (A.0B0)														*/
/*																									*/
/*	1)SGD7-APtype, after the auxiliary function is operated, an alarm is generated by the servo-on command.								*/
/*	2)SGD7-M2,M3type, for the time beingSGDV-MD. See below for detection conditions.					*/
/*																									*/
/*	※Reference:A.0B0Detection conditions																			*/
/*	4)SGDV-AP: After operation using the auxiliary function, a servo-on command was issued from the host.									*/
/*	5)SGDV-M2,M3: After operation using the auxiliary function,DISCONNECT.				*/
/*	6)SGDV-MD: Speed control after operation by auxiliary function/A servo-on command was issued from the host without switching to torque control.*/
/*																									*/
/****************************************************************************************************/
#if CSW_NETIF_TYPE	== NETIF_ANLGPLS_TYPE
LONG	LpxCheckServoOnCmdErr( 						/* Check Servo On Cmd Error (A.0B0):AP			*/
	BE_SEQ_HNDL 	*BeSeq, 						/* 												*/
	BOOL			ServoOnReq,						/* Servo On Command								*/
	FUN_CMN_CONTROL *FnCmnCtrl, 					/* 												*/
	CTRLMD 			CtrlMode )						/* Control Mode									*/
{
LONG	err = FALSE;
/*--------------------------------------------------------------------------------------------------*/
	if( FnCmnCtrl->State.LockNormalOpeDrive != FALSE )
	{	/* Normal operation prohibited*/
		if( ServoOnReq != FALSE )
		{
			err = TRUE;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	return (err);
}
#else
LONG	LpxCheckServoOnCmdErr( 						/* Check Servo On Cmd Error (A.0B0):M2M3		*/
	BE_SEQ_HNDL 	*BeSeq, 						/* 												*/
	BOOL			ServoOnReq,						/* Servo On Command								*/
	FUN_CMN_CONTROL *FnCmnCtrl, 					/* 												*/
	CTRLMD 			CtrlMode )						/* Control Mode									*/
{
LONG	err = FALSE;
/*--------------------------------------------------------------------------------------------------*/
	if(FnCmnCtrl->State.LockNormalOpeDrive != FALSE)
	{	/* Normal operation prohibited*/
		if( (CtrlMode.us.uwd_l == CTRLMODE_SPD) || (CtrlMode.us.uwd_l == CTRLMODE_TRQ) )
		{
			BeSeq->SenStatus = FALSE;
			FnCmnCtrl->State.LockNormalOpeDrive = FALSE;
		}
		else if( (ServoOnReq != FALSE) && (CtrlMode.us.uwd_l == CTRLMODE_POS) )
		{/* With servo on command & Position control mode command */
			err = TRUE;
		}
		else
		{	/* do nothing. */
			;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	return (err);
}
#endif


/***************************************** end of file **********************************************/

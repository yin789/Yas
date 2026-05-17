/****************************************************************************
  Description   : One Parameter Tuning (Fn203)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.02.10   created

****************************************************************************/
#include "FnOnePrmTuning.h"
#include "MercuryGlobals.h"
#include "ModelControl.h"
#include "PnPrmListTbl.h"
#include "KLib.h"
#include "ExControls.h"

/***************************************************************************************************
	Local Function Prototype Definition
***************************************************************************************************/
static LONG	 onePrmANotchCheck( ONEPRMTUNE *OnePrmTune, DETVIB *DetVib, BOOL BaseEnable );
static LONG	 onePrmSetTuningGain( ONEPRMTUNE *OnePrmTune, PRMDATA *Prm, REG_ACC_T *AccessPrm,
																	BOOL IPSpdControl, BOOL DobsAct );
static LONG	 onePrmSetTuningNotch( ONEPRMTUNE *OnePrmTune, AUTONOTCH *ANotch,
														REG_ACC_T *AccessPrm, PRMDATA *Prm );
static LONG	 onePrmUpdateGainInEeprom( REGIF_CONFIG_T *RegCfg, BOOL DobsAct, LONG MfcModel );
static void onePrmUndoRamGain( REGIF_CONFIG_T *RegCfg );
static void onePrmInitializeTuningMode( ONEPRMTUNE *OnePrmTune, UCHAR CtrlMode, BOOL BaseEnable );
static void onePrmCheckTuningMode( ONEPRMTUNE *OnePrmTune );



/***************************************************************************************************
	Fn203 One Parameter Tuning mode Begin
***************************************************************************************************/
PRM_RSLT	FnEnterOnePrmTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT	rc;
	ONEPRMTUNE	*OnePrmTune;
	PRMDATA		*Prm;

	/* Local variable initialization */
	OnePrmTune = Axis->OnePrmTune;
	Prm = Axis->UniPrms->Prm;
	rc = PRM_RSLT_SUCCESS;

	/* Initialize One Parameter Tuning */
	OnePrmTune->OpeReg.SpatLevel = Prm->loophz;
	OnePrmTune->OpeReg.SpatFFLevel = Prm->mdlgn;
	RpiRegSetOnePrmTuningLevel( OnePrmTune, Prm->loophz );
	RpiRegSetOnePrmTuningFfLevel( OnePrmTune, Prm->mdlgn );

	if( Axis->BaseControls->TuneLessCtrl.conf.TuningLessUse == TRUE )
	{ /* Adjustmentless enabled */
		rc = PRM_RSLT_CONDITION_ERR;
	}

	/* If not in motorless test mode, exit without entering *//* <S1B7> */
	if( Axis->MotorLessTest->MotorLessTestMode == TRUE )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* Get Original Gains */
	OnePrmTune->var.OrgKv    = Prm->loophz;		/* Pn100 : Speed loop gain 			*/
	OnePrmTune->var.OrgMdlgn = Prm->mdlgn;		/* Pn141 : Model following control gain[0.1/s]	*/
	OnePrmTune->var.OrgNf1   = Prm->ntchfil1;	/* Pn409 : 1Stage notch filter frequency 	*/
	OnePrmTune->var.OrgNf2   = Prm->ntchfil2;	/* Pn40C : 2Stage notch filter frequency 	*/
	OnePrmTune->var.OrgNfQ1  = Prm->ntchq1;		/* Pn40A : 1Stage notch filterQvalue 		*/
	OnePrmTune->var.OrgNfQ2  = Prm->ntchq2;		/* Pn40D : 1Stage notch filterQvalue 		*/
	OnePrmTune->var.Jrate    = Prm->jrate;		/* Pn103 : Moment of inertia ratio 			*/

	/* Get Atype Resvib Gain */
	OnePrmTune->var.OrgAResf   = Prm->avibfrq;	/* Pn161 : A-type vibration damping frequency 				*/
	OnePrmTune->var.OrgAResDGn = Prm->avibdamp;	/* Pn163 : A type vibration suppression damping gain 	*/

	/* Reset One Parameter Tuning Gain */
	OnePrmReSetTuningLevel( OnePrmTune,
							Prm->loophz,
							Prm->posgn,
							Axis->BaseControls->CtrlCmdPrm.IPSpdControl );

	OnePrmTune->OpeReg.SpatFFLevel = OnePrmTune->var.OrgMdlgn;
	OnePrmTune->var.FfSetting      = OnePrmTune->var.OrgMdlgn;
	OnePrmTune->var.TunRun         = TRUE;

	/* FnMode automatic end */
	FunExe->HoldFnMode = TRUE;

	return(rc);
}


/***************************************************************************************************
	Fn203 One Parameter Tuning mode End
****************************************************************************************************/
void	FnLeaveOnePrmTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	REGIF_CONFIG_T	*RegCfg;
	ONEPRMTUNE		*OnePrmTune;

	RegCfg     = Axis->RegManager->hPnReg;
	OnePrmTune = Axis->OnePrmTune;

	/* EEPROMRevert changed parameters with data */
	onePrmUndoRamGain( RegCfg );
	MlibResetLongMemory( &(OnePrmTune->var), sizeof(OnePrmTune->var)/4 );
}

/***************************************************************************************************
	Fn203 One Parameter Tuning mode Execution
***************************************************************************************************/
PRM_RSLT	FnExecuteOnePrmTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT		rc;
	ULONG			ulwork;
	SHORT			swk;
	USHORT			KvGain;
	USHORT			VibFreq;
	REG_ACC_T		AccessPrm;
	ONEPRMTUNE		*OnePrmTune;
	BASE_CTRL_HNDL	*pBaseCtrl;
	REGIF_CONFIG_T	*RegCfg;
	PRMDATA			*Prm;

	/* Local variable initialization */
	OnePrmTune = Axis->OnePrmTune;
	pBaseCtrl  = Axis->BaseControls;
	RegCfg     = Axis->RegManager->hPnReg;
	Prm        = Axis->UniPrms->Prm;
	rc         = PRM_RSLT_SUCCESS;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/* Vibration detection trace request processing */
	OnePrmTune->var.FftStat = TuneVibrationTrace( Axis->DetVib,
												  Axis->FftAna,
												  OnePrmTune->var.FftReq,
												  0,
												  TuneLessGetTuneLessSts(&Axis->BaseControls->TuneLessCtrl) );

	if( ((~Prm->tfuncsw) & Prm->anotchsw & 0x0F0F) == 0x0000 )
	{ /* Automatically configurable notch not available */
		/* Automatic setting disabled status setting */
		OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_NOENABLE;
	}

	OnePrmTune->OpeReg.SpatANotchState = (OnePrmTune->var.ANotchPreStat << 12)
									   + (OnePrmTune->var.ANotch2Stat 	<<  8)
									   + (OnePrmTune->var.ANotch1Stat 	<<  4)
									   + (OnePrmTune->var.ANotchStat         );

	/*------------------------------------------------------------------------------------------*/
	/*	Register response data creation																	*/
	/*------------------------------------------------------------------------------------------*/
	if( ((Prm->avibsw & 0x0F) == TRUE)
		|| (((Prm->avibsw >> 4) & 0x0F) == FALSE)
		|| ((pBaseCtrl->CtrlModeSet.CtrlMode.b.cm == CTRLMODE_TRQ) && (Axis->SeqCtrlOut->BaseEnableReq == TRUE)) )
	{ /* A type vibration suppression already enabled || Automatic adjustment disabled || (Torque control mode && Servo on) */
		/* Automatic setting disabled setting */
		OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOENABLE;
	}
	else if( OnePrmTune->var.AResStat == OPTUNE_STS_ARES_NOENABLE)
	{ /* A-type vibration damping enable condition is satisfied, but status cannot be set */
		/* Initialize to undetected */
		OnePrmTune->var.AResStat  = OPTUNE_STS_ARES_NOVIBDET;
		OnePrmTune->var.ARes1Stat = OPTUNE_STS_ARES_NODETECT;
	}
	/* Store Ares State on the Register */
	OnePrmTune->OpeReg.SpatAresState = (OnePrmTune->var.ARes1Stat << 4) + OnePrmTune->var.AResStat;

	switch( FunExe->State )
	{
	case OPTUNE_SEQ_INIT:
		/*--------------------------------------------------------------------------------------*/
		/*	Automatic notch execution condition determination(Initialize)														*/
		/*--------------------------------------------------------------------------------------*/
		if( ((~Prm->tfuncsw) & Prm->anotchsw & 0x0F0F) != 0x0000 )
		{ /* Automatic notch setting possible */
			/* Mask oscillation detection(Calculation function required) */
			//Prm->vibsel = 0;
			TunePrmWriteRam( &AccessPrm, &pndef_vibsel, 0, pndef_vibsel.DataLength );/*<S09E>*/
		}
		else
		{ /* Conditions for prohibiting automatic notch setting */
			OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_NOENABLE;
		}

		/*--------------------------------------------------------------------------------------*/
		/*	Automatic A-type vibration suppression setting check(Initialize)												*/
		/*--------------------------------------------------------------------------------------*/
		/* Set to undetected */
		OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOVIBDET;
		OnePrmTune->var.ARes1Stat = OPTUNE_STS_ARES_NODETECT;

		if( ((Prm->avibsw & 0x0F) == TRUE)
			|| (((Prm->avibsw >> 4) & 0x0F) == FALSE)
			|| ((pBaseCtrl->CtrlModeSet.CtrlMode.b.cm == CTRLMODE_TRQ) && (Axis->SeqCtrlOut->BaseEnableReq == TRUE)) )
		{	/* A type vibration suppression already enabled || Automatic adjustment disabled || (Torque control mode && Servo on) */
			/* Automatic setting disabled setting*/
			OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOENABLE;
		}
		else
		{
			/* Mask oscillation detection(Calculation function required) */
			//Prm->vibsel = 0;
			TunePrmWriteRam( &AccessPrm, &pndef_vibsel, 0, pndef_vibsel.DataLength );
		}
		/* Vibration detection differential speed maximum/minimum value reset */
		DetVibResetMaxMinSpeedError( Axis->DetVib );
		DetVibResetMaxMinSpeedErrorFn( Axis->DetVib );

		TuneFilGainCalbyFilType( Axis->ANotch, (UCHAR)OnePrmTune->OpeReg.OnePrmFilType );
		onePrmSetTuningGain( OnePrmTune,
							 Prm,
							 &AccessPrm,
							 pBaseCtrl->CtrlCmdPrm.IPSpdControl,
							 pBaseCtrl->DobsCtrl.var.DobsAct );
		onePrmSetTuningNotch( OnePrmTune,
							  Axis->ANotch,
							  &AccessPrm,
							  Prm );
		FunExe->State = OPTUNE_SEQ_TUNING;

		/* FFSetting variable reset */
		KlibRstLongTimer( &OnePrmTune->var.FfWaitTimer );
		OnePrmTune->OpeReg.SpatLvlChgState = OPTUNE_STS_FFCOMP;
		break;

	case OPTUNE_SEQ_TUNING:		/* Tuning level change processing	*/
		if( OnePrmTune->var.FftWait == TRUE )
		{
			if( KlibGetLongTimerMs(OnePrmTune->var.FftWaitTimer) > 2000 )
			{
				OnePrmTune->var.FftWait = FALSE;
				DetVibResetMaxMinSpeedError( Axis->DetVib );
			}
		}

		if( OnePrmTune->var.NotchChk == TRUE )
		{ /* Notch checking in progress */
			if( KlibGetLongTimerMs(OnePrmTune->var.NotchChkTimer) > 3000 )
			{
				OnePrmTune->var.NotchChk = FALSE;
				OnePrmTune->var.ANotchSetStat = OPTUNE_STS_NOTCH_INIT;
				OnePrmTune->var.AResSetStat = OPTUNE_STS_ARES_INIT;
			}
		}

		if( OnePrmTune->var.ANotchSetStat > 0x02 )
		{ /* Error status */
			if( KlibGetLongTimerMs( OnePrmTune->var.NotchChkTimer ) > 2000 )
			{
				/* ERRClear display */
				OnePrmTune->var.ANotchSetStat = OPTUNE_STS_NOTCH_INIT;
			}
		}

		if( OnePrmTune->var.AResSetStat >= 0x10 )
		{ /* Error status */
			if( KlibGetLongTimerMs( OnePrmTune->var.NotchChkTimer ) > 2000 )
			{
				/* ERRClear display */
				OnePrmTune->var.AResSetStat = OPTUNE_STS_ARES_INIT;
			}
		}

		/* Frequency analysis start*/
		if( onePrmANotchCheck( OnePrmTune, Axis->DetVib, Axis->SeqCtrlOut->BaseEnableReq ) == OK )
		{
			OnePrmTune->var.FftReq = TRUE;
			OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_DETECT;
			OnePrmTune->var.AResStat = OPTUNE_STS_ARES_DETECT;
			FunExe->State = OPTUNE_SEQ_VIBSEL;
		}

		/*----------------------------------------------------------------------------------------------*/
		/*	About disturbance observer settings																	*/
		/*		Mode2is to enable the disturbance observer unconditionally.(The settings are one-parabalance).				*/
		/*		Other modes are function switchesONAssume that the one-parabalance disturbance observer is enabled.			*/
		/*----------------------------------------------------------------------------------------------*/
		/*	FFLevel change status creation process & timeout process											*/
		/*		At timeout,FFand forcefully return to the currently running setting.					*/
		/*		As for internal processing,COINbutONFFSetting value(Display value)Parameter calculation etc.				*/
		/*		Nothing has been done, so on the displayFFOnly the level needs to be restored to its original level									*/
		/*----------------------------------------------------------------------------------------------*/
		if( OnePrmTune->OpeReg.SpatFFLevel != OnePrmTune->var.FfSetting )
		{ /* Display and operation level mismatch */
			switch( OnePrmTune->OpeReg.SpatMode )
			{ /* Tuning level */
			case 2: /* Positioning only */
				//TuneMfcActive( Prm );
				TuneMfcActive( Prm, AccessPrm.hRegCfg );/*<S09E>*/
				break;
			case 3: /* Positioning only(Overshoot emphasis) */
				//TuneMfcActiveStMode( Prm );
				TuneMfcActiveStMode( Prm, AccessPrm.hRegCfg );/*<S09E>*/
				break;
			default:
				break;
			}

			/* Tuning data setting */
			onePrmSetTuningGain( OnePrmTune,
								 Prm,
								 &AccessPrm,
								 pBaseCtrl->CtrlCmdPrm.IPSpdControl,
								 pBaseCtrl->DobsCtrl.var.DobsAct );
			onePrmSetTuningNotch( OnePrmTune,
								  Axis->ANotch,
								  &AccessPrm,
								  Prm );
			/* FFChange completion waiting status is"Execution"only time */
			OnePrmTune->OpeReg.SpatLvlChgState = OPTUNE_STS_FFWAIT;
		}

		if( MfcGetParamEnableSts( &(pBaseCtrl->MFControl) ) == TRUE )
		{
			/* FFReflecting level display for operation */
			OnePrmTune->OpeReg.SpatFFLevel = OnePrmTune->var.FfSetting;
			if( OnePrmTune->OpeReg.SpatLvlChgState == OPTUNE_STS_FFWAIT )
			{
				/* FFChange completion waiting status is"Execution"only time */
				OnePrmTune->OpeReg.SpatLvlChgState = OPTUNE_STS_FFCOMP;
			}
		}
		else
		{
			if( KlibGetLongTimerMs( OnePrmTune->var.FfWaitTimer ) > 8000 )
			{ /* FFAfter level change,8secElapsed time */
				/* FFChange failure(Timeout) */
				OnePrmTune->OpeReg.SpatLvlChgState = OPTUNE_STS_FFTMOUT;
				/* FFReturning the level to its original level */
				OnePrmTune->var.FfSetting = OnePrmTune->OpeReg.SpatFFLevel;
				/* Tuning data setting */
				onePrmSetTuningGain( OnePrmTune,
									 Prm,
									 &AccessPrm,
									 pBaseCtrl->CtrlCmdPrm.IPSpdControl,
									 pBaseCtrl->DobsCtrl.var.DobsAct );
				onePrmSetTuningNotch( OnePrmTune,
									  Axis->ANotch,
									  &AccessPrm,
									  Prm );
			}
		}

		switch( FunExe->CmdCode )
		{
		case FCMD_STOP:		/* Stop */
			ulwork = (ULONG)Prm->tfuncsw;

			switch( OnePrmTune->var.ANotchPreStat )
			{ /* Notch completion status */
			case OPTUNE_PSTS_NT1_COMP:		/* Previous1Row notchON */
				ulwork &= 0x0000FFF0;									/* No.1NotchOFF						*/
				OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_NODETECT;	/* Vibration not detected status setting			*/
				OnePrmTune->var.ANotch1Stat = OPTUNE_STS_NT1_NODETECT;	/* 1Stage status initialized to unimplemented	*/
				OnePrmTune->var.ANotchPreStat = OPTUNE_PSTS_NODETECT;	/* Initialize last notch to not executed		*/
				break;

			case OPTUNE_PSTS_NT2_COMP:		/* Previous2Row notchON	*/
				ulwork &= 0x0000F0FF;									/* No.2NotchOFF						*/
				OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_NODETECT;	/* Vibration not detected status setting			*/
				OnePrmTune->var.ANotch2Stat = OPTUNE_STS_NT2_NODETECT;	/* 2Stage status initialized to unimplemented	*/
				OnePrmTune->var.ANotchPreStat = OPTUNE_PSTS_NODETECT;	/* Initialize last notch to not executed		*/
				break;

			default:
				break;
			}

			/* Pn408 Torque related function switch */
//			rc = RpiPrmWriteRamForFn( &pndef_tfuncsw, (USHORT)ulwork, pfexe->App.Hmsg );
			TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, ulwork, 2 );

			/* Tuning data setting	*/
			onePrmSetTuningGain( OnePrmTune,
								 Prm,
								 &AccessPrm,
								 pBaseCtrl->CtrlCmdPrm.IPSpdControl,
								 pBaseCtrl->DobsCtrl.var.DobsAct );
			onePrmSetTuningNotch( OnePrmTune,
								  Axis->ANotch,
								  &AccessPrm,
								  Prm );
			DetVibResetMaxMinSpeedError( Axis->DetVib );
			break;

		case FCMD_EXEC:		/* Change gain */
			onePrmInitializeTuningMode( OnePrmTune,
										pBaseCtrl->CtrlModeSet.CtrlMode.b.cm,
										Axis->SeqCtrlOut->BaseEnableReq );
			onePrmCheckTuningMode( OnePrmTune );
			if( OnePrmTune->var.ModeUnmatch == FALSE )
			{
				onePrmSetTuningGain( OnePrmTune,
									 Prm,
									 &AccessPrm,
									 pBaseCtrl->CtrlCmdPrm.IPSpdControl,
									 pBaseCtrl->DobsCtrl.var.DobsAct );
				onePrmSetTuningNotch( OnePrmTune,
									  Axis->ANotch,
									  &AccessPrm,
									  Prm );
				KlibRstLongTimer( &OnePrmTune->var.FfWaitTimer );
			}
			else
			{
				rc = PRM_RSLT_CONDITION_ERR;
			}
			break;

		case FCMD_EXEC2:
		case FCMD_SV:
			/*------------------------------------------------------------------------------------------*/
			/*	Automatic notch execution condition determination																	*/
			/*------------------------------------------------------------------------------------------*/
			if( (OnePrmTune->var.ANotchStat != OPTUNE_STS_NT_NOENABLE)
				|| (OnePrmTune->var.AResStat != OPTUNE_STS_ARES_NOENABLE) )
			{ /* Auto notch execution possible or Automatic A-type vibration suppression possible */
				/*--------------------------------------------------------------------------------------*/
				/*	Frequency analysis processing execution processing																*/
				/*--------------------------------------------------------------------------------------*/
				if( Axis->SeqCtrlOut->BaseEnableReq == TRUE )
				{ /* Executed only when servo is on */
					OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_DETECT;	/* Vibration detection status setting		*/
					OnePrmTune->var.AResStat = OPTUNE_STS_ARES_DETECT;	/* Vibration detection status setting		*/
					OnePrmTune->var.FftReq = TRUE;						/* Frequency analysis processing start request		*/
					FunExe->State = OPTUNE_SEQ_VIBSEL;					/* Going to vibration suppression setting phase	*/
					rc = PRM_RSLT_CONTINUE;
				}
				else
				{	/* Servo off */
					rc = PRM_RSLT_CONDITION_ERR;
				}
			}
			break;

		case FCMD_EWRT:		/* EEPROMWrite */
			if( onePrmUpdateGainInEeprom(RegCfg,
										 pBaseCtrl->DobsCtrl.var.DobsAct,
//										 pBaseCtrl->MFControl.conf.MFCModelSetting) != TRUE )
										 pBaseCtrl->MFControl.conf.MFCModelSetting) != PRM_RSLT_SUCCESS )/* <S085> */
			{ /* EEPROMWrite process */
				rc = PRM_RSLT_CONDITION_ERR;
			}
			OnePrmTune->var.FftReq = FALSE;
			OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_NODETECT;
			OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOVIBDET;
			break;

		case FCMD_NONE:
			break;

		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}
		break;

	case OPTUNE_SEQ_VIBSEL:		/* Vibration suppression setting */
		if( OnePrmTune->var.FftStat == TUNE_FREQ_SEARCHEND )
		{
			/* FFTAnalysis requestOFF */
			OnePrmTune->var.FftReq = FALSE;

			/* Obtain vibration frequency */
			VibFreq = FftGetAnaRsltVibmRf( Axis->FftAna );

			if( (OnePrmTune->var.AResStat != OPTUNE_STS_ARES_NOENABLE)
				&& (VibFreq <= AVIB_FRQ_MAX)
				&& (OnePrmTune->OpeReg.SpatLevel < TUNE_ANOTCH_MINKV) )
			{ /* A-type vibration damping setting possible && The frequency is below the A-type damping upper limit && NotchONMinimumKv */
				/* Minimum notch settingKvSetting		*/
				KvGain = TUNE_ANOTCH_MINKV;
			}
			else
			{
				/* FBLevel setting value */
				KvGain = OnePrmTune->OpeReg.SpatLevel;
			}

			/* Auto notch filter setting information set */
			TuneSetNotchFilSettingData( Axis->ANotch, VibFreq, KvGain, Prm->jrate );
			/* No.1Peak frequency	*/
			swk = TuneNotchFilterSetting( Axis->ANotch, Prm, RegCfg );

			/* Set notch setting status */
			OnePrmTune->var.ANotchSetStat = (UCHAR)swk;
			/* Tuning data setting	*/
			onePrmSetTuningGain( OnePrmTune,
								 Prm,
								 &AccessPrm,
								 pBaseCtrl->CtrlCmdPrm.IPSpdControl,
								 pBaseCtrl->DobsCtrl.var.DobsAct );
			onePrmSetTuningNotch( OnePrmTune,
								  Axis->ANotch,
								  &AccessPrm,
								  Prm );

			switch( OnePrmTune->var.ANotchSetStat & 0x0F )
			{
			case OPTUNE_STS_NOTCH1_OK:		/* 1Stage notch setting completed normally	*/
				OnePrmTune->var.ANotch1Stat = OPTUNE_STS_NT1_COMP;
				OnePrmTune->var.ANotchPreStat = OPTUNE_PSTS_NT1_COMP;
				break;

			case OPTUNE_STS_NOTCH2_OK:		/* 2Stage notch setting completed normally	*/
				OnePrmTune->var.ANotch2Stat = OPTUNE_STS_NT2_COMP;
				OnePrmTune->var.ANotchPreStat = OPTUNE_PSTS_NT2_COMP;
				break;

			case OPTUNE_STS_NOTCHQ1_ERR:	/* 1Row notchQValue error		*/
				OnePrmTune->var.ANotch1Stat = OPTUNE_STS_NT1_NG;
				break;

			case OPTUNE_STS_NOTCHQ2_ERR:	/* 2Row notchQValue error		*/
				OnePrmTune->var.ANotch2Stat = OPTUNE_STS_NT2_NG;
				break;

			default:
				break;
			}

			/*------------------------------------------------------------------------------------------*/
			/*	Automatic notch execution condition determination																	*/
			/*------------------------------------------------------------------------------------------*/
			if( ((~Prm->tfuncsw) & Prm->anotchsw & 0x0F0F) == 0x0000 )
			{ /* Automatically configurable notch not available */
				/* Automatic setting disabled status setting */
				OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_NOENABLE;
			}
			else
			{
				/* Vibration not detected status setting */
				OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_NODETECT;
			}

			if( (OnePrmTune->var.ANotchSetStat == OPTUNE_STS_NOTCH1_OK)
				|| (OnePrmTune->var.ANotchSetStat == OPTUNE_STS_NOTCH2_OK) )
			{
				/* Transition to notch check state */
				OnePrmTune->var.NotchChk = TRUE;
				OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOVIBDET;
				FunExe->State = OPTUNE_SEQ_TUNING;
			}
			else
			{ /* Notch setting failure */
				/*--------------------------------------------------------------------------------------*/
				/*	Automatic A-type vibration damping																		*/
				/*--------------------------------------------------------------------------------------*/
				OnePrmTune->var.AResSetStat = (UCHAR)TuneATypeResSet( VibFreq,
																	  ((Prm->avibsw >> 4) & 0x0F),
																	  Prm,
																	  RegCfg );/*<S09E>*/

				/* Register response creation		*/
				switch( OnePrmTune->var.AResSetStat & 0xFF )
				{
				case OPTUNE_STS_ARES_OK:	/* A-type vibration damping setting successful completion	*/
					OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOVIBDET;
					OnePrmTune->var.ARes1Stat = OPTUNE_STS_ARES_COMP;
					/* Go to notch check state */
					OnePrmTune->var.NotchChk = TRUE;
					FunExe->State = OPTUNE_SEQ_TUNING;
					break;
				case 0x10:					/* A-type vibration damping upper and lower limit error	*/
					OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOVIBDET;
					OnePrmTune->var.ARes1Stat = OPTUNE_STS_ARES_NG;
					break;
				case 0x11:					/* A-type vibration damping automatic setting disabled	*/
					OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOENABLE;
					break;
				default:
					OnePrmTune->var.AResStat = OPTUNE_STS_ARES_NOVIBDET;
					OnePrmTune->var.ARes1Stat = OPTUNE_STS_ARES_NG;
					break;
				}
				if( (OnePrmTune->var.AResSetStat & 0xFF) != OPTUNE_STS_ARES_OK )
				{	/* Notch/A type togetherNG */
					OnePrmTune->var.FftWait = TRUE;
					KlibRstLongTimer( &OnePrmTune->var.FftWaitTimer );
					FunExe->State = OPTUNE_SEQ_NOTCHERR;
				}
			}
			KlibRstLongTimer( &OnePrmTune->var.NotchChkTimer );
		}
		else if( OnePrmTune->var.FftStat == TUNE_FREQ_SEARCHERR )
		{ /* Error processing */
			OnePrmTune->var.FftReq = FALSE;
			OnePrmTune->var.ANotchStat = OPTUNE_STS_NT_DET_ERR;		/* Detection error status setting			*/
			OnePrmTune->var.AResStat = OPTUNE_STS_ARES_DET_ERR;		/* Detection error status setting			*/
			FunExe->State = OPTUNE_SEQ_TUNING;
		}
		else
		{
			rc = PRM_RSLT_CONTINUE;
		}
		break;

	case OPTUNE_SEQ_NOTCHERR: /* Notch filter/When A type does not fit */
		FunExe->State = OPTUNE_SEQ_TUNING;
		break;

	default:
		break;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Low frequency vibration detection																				*/
	/*	  The target of detection is position loop oscillation due to incorrect inertia setting when the disturbance observer is enabled.				*/
	/*	  Therefore, set the detection level high														*/
	/*	  When vibration is detected, vibration alarm(A.520)												*/
	/*	If not processed here, vibration will be detected before the vibration level is cleared in the initialization process				*/
	/*----------------------------------------------------------------------------------------------*/
	if( (pBaseCtrl->DobsCtrl.var.DobsAct == TRUE)
		&& (DetVibGetSpdErrPeakFn(Axis->DetVib) > (2 * DetVibGetVibCheckLevel(Axis->DetVib))) )
	{ /* Disturbance observer enabled && Oscillation level×2 */
		ALMSetServoAlarm( Axis->AlmManager, ALM_AT );
		DetVibResetMaxMinSpeedError( Axis->DetVib );
		DetVibResetMaxMinSpeedErrorFn( Axis->DetVib );
	}

	return(rc);
}



/*****************************************************************************************************
	Tuning level resetting process
*****************************************************************************************************/
void	OnePrmReSetTuningLevel( ONEPRMTUNE *OnePrmTune, USHORT loophz, USHORT posgn, BOOL IPSpdControl )
{
	ULONG	Kv, Kp, Kp2Kv;
	ULONG	da, db;

	Kv = (ULONG)loophz;
	Kp = (ULONG)posgn;

	switch( OnePrmTune->OpeReg.SpatMode )
	{
		case 0:	/* Stability emphasis */
			/********************************/
			/* Attenuation coefficient calculation				*/
			/*			 1   Kv				*/
			/*	   ζa = - √--  * 100		*/
			/*			 2   Kp				*/
			/********************************/
			da = MlibSqrtu32(Kv * 2500) / MlibSqrtu32(Kp);
			if( IPSpdControl == TRUE )
			{ /* I-PDuring control */
				db = 133;
				Kp2Kv = Kp * 228 / 100;
			}
			else
			{ /* PIDuring control */
				db = 125;
				Kp2Kv = Kp;
			}

			if( da >= db )
			{ /* ζa ≧ ζb */
				OnePrmTune->OpeReg.SpatLevel = (USHORT)Kp2Kv;
			}
			else
			{ /* ζa < ζb */
				OnePrmTune->OpeReg.SpatLevel = (USHORT)Kv;
			}
			break;

		case 1:	/* Response emphasis */
		case 2:	/* Positioning only */
		case 3:	/* Positioning only(OV) */
		default:
			OnePrmTune->OpeReg.SpatLevel = (USHORT)Kv;
			break;
	}	
	return;
}


/*****************************************************************************************************
	Fn203 Write Gain To Ram With EEPROM Data
*****************************************************************************************************/
static void	onePrmUndoRamGain( REGIF_CONFIG_T *RegCfg )
{
	/* RAMthe value ofEEPROMbe the same as the value of */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_loophz,   0 );	/* Pn100 Speed loop gain 					*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_pitime,   0 );	/* Pn101 Speed loop integral time constant				*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_posgn,	   0 );	/* Pn102 Position loop gain					*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_jrate,    0 );	/* Pn103 Moment of inertia ratio					*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_trqfil11, 0 );	/* Pn401 1Stage1Torque command filter time constant 	*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_tfuncsw,  0 );	/* Pn408 Torque related function switch 			*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil1, 0 );	/* Pn409 1Stage notch filter frequency 			*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchq1,   0 );	/* Pn40A 1Stage notch filterQvalue				*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil2, 0 );	/* Pn40C 2Stage notch filter frequency			*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchq2,   0 );	/* Pn40D 1Stage notch filterQvalue				*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_avibsw,   0 );	/* Pn160 Damping control related switches				*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_avibfrq,  0 );	/* Pn161 AMold vibration damping frequency						*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_avibdamp, 0 );	/* Pn163 AMold vibration damping damping gain			*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_dobgn,    0 );	/* Pn121 Friction compensation gain						*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_dtrqgn,   0 );	/* Pn123 Friction compensation coefficient						*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_dlpfil,   0 );	/* Pn124 Friction compensation frequency correction					*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_dobjgn,   0 );	/* Pn125 Friction compensation gain correction					*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlsw,    0 );	/* Pn140 Model following control related switches			*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlgn,    0 );	/* Pn141 Model following control gain				*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlzeta,  0 );	/* Pn142 Model following control gain correction			*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlptff,  0 );	/* Pn143 Model following control bias(Forward rotation direction)	*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlntff,  0 );	/* Pn144 Model following control bias(Reverse direction)	*/
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlvff,   0 );	/* Pn147 Model following control speed feedforward compensation */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_vibsel,   0 );	/* Pn310 Vibration detection switch					*/

	return;
}


/*****************************************************************************************************
	Fn203 One Parameter Tuning mode Initial Set
*****************************************************************************************************/
static void onePrmInitializeTuningMode( ONEPRMTUNE *OnePrmTune, UCHAR CtrlMode, BOOL BaseEnable )
{
	if( (CtrlMode != CTRLMODE_POS) && (BaseEnable == TRUE) )
	{
		OnePrmTune->var.ModeMin = 0;
		OnePrmTune->var.ModeMax = 1;
//		OnePrmTune->OpeReg.SpatMode = 0;/*<S09E>*/
	}
	else
	{
		OnePrmTune->var.ModeMin = 0;
		OnePrmTune->var.ModeMax = OPTUNE_MODE_MAX;
//		OnePrmTune->OpeReg.SpatMode = 2;/*<S09E>*/
	}
}


/*****************************************************************************************************
	Fn203 One Parameter Tuning mode Condition Check
*****************************************************************************************************/
static LONG onePrmANotchCheck( ONEPRMTUNE *OnePrmTune, DETVIB *DetVib, BOOL BaseEnable )
{
	LONG	rc;

	if( ((OnePrmTune->var.ANotchStat != OPTUNE_STS_NT_NOENABLE)
	 || (OnePrmTune->var.AResStat != OPTUNE_STS_ARES_NOENABLE))
	 && (OnePrmTune->var.FftWait == FALSE)
	 && (OnePrmTune->var.NotchChk == FALSE)
	 && (DetVibGetSpdErrPeak(DetVib) > DetVibGetVibCheckLevel(DetVib))
	 && (BaseEnable == TRUE) )
	{ /* (Auto notch execution possible || Automatic A-type vibration suppression possible)
		  && !Waiting for automatic notch execution && !Notch checking && Vibration exceeds detection level && While the servo is on */
		rc = OK;
	}
	else
	{
		rc = NG;
	}
	return( rc );
}


/*****************************************************************************************************
	Fn203 Set One Parameter Tuning Gain
*****************************************************************************************************/
static LONG	onePrmSetTuningGain( ONEPRMTUNE *OnePrmTune, PRMDATA *Prm, REG_ACC_T *AccessPrm,
																BOOL IPSpdControl, BOOL DobsAct )
{
	LONG		rc;
	LONG		kx, sx;
	ULONG		ulwork;
	TUNECALV	TuneCalV;

	/* Local variable initialization */
	rc = OK;

	/*------------------------------------------------------------------------------*/
	/*	Setting calculation parameters														*/
	/*------------------------------------------------------------------------------*/
	switch( OnePrmTune->OpeReg.SpatMode )
	{
	case 1:
		if( IPSpdControl == TRUE )
		{ /* I-PDuring control */
			TuneCalV.PIRate = 1;
			TuneCalV.KpZeta = 100;
			TuneCalV.Ti = 200 * 70 * 70;
		}
		else
		{ /* PIDuring control */
			TuneCalV.KpZeta = 70;
		}
		break;
	case 0:
	case 2:
	case 3:
	default:
		if( IPSpdControl )
		{ /* I-PDuring control */
			TuneCalV.PIRate = 1;
			TuneCalV.KpZeta = 228;
			TuneCalV.Ti = 200 * 100 * 100;
		}
		else
		{ /* PIDuring control */
			TuneCalV.KpZeta = 100;
		}
		break;
	}

	/*------------------------------------------------------------------------------*/
	/*	Pn101, Pn102, Pn121													*/
	/*------------------------------------------------------------------------------*/
	if( (OnePrmTune->OpeReg.SpatMode == 2) || (OnePrmTune->OpeReg.SpatMode == 3) )
	{
		/* Pn101 */
		TuneCalV.Gain.pitime = TuneKvFuncTi( OnePrmTune->OpeReg.SpatLevel,
											 IPSpdControl );
		/* Pn102 */
		TuneCalV.Gain.posgn = TuneKvFuncKp( OnePrmTune->OpeReg.SpatLevel,
											TuneCalV.Gain.pitime,
											IPSpdControl );
		/* Pn121 */
		TuneCalV.Gain.dobgn = TuneKvFuncDobsgn( OnePrmTune->OpeReg.SpatLevel );
		/* Pn123, Pn125, Pn408 */
		//TuneDobsActive( Prm );
		TuneDobsActive( Prm, AccessPrm->hRegCfg );//<S09E>

		/* Disturbance observer enabled */
		DobsAct = TRUE;/*<S09E>*/
	}
	else
	{
		if( IPSpdControl == TRUE )
		{ /* I-PDuring control */
			/* Pn102 */
			kx = MlibScalKxgain( OnePrmTune->OpeReg.SpatLevel, 100, TuneCalV.KpZeta, &sx, 0 );
			kx = MlibPcalKxgain( kx, TuneCalV.PIRate, 1, &sx, 24);
			TuneCalV.Gain.posgn = (USHORT)MlibMulgain( 1, kx );

			/* Pn101 */
			kx = MlibScalKxgain( TuneCalV.Ti, 1000, 3141, &sx, 0 );
			kx = MlibPcalKxgain( kx, 1, OnePrmTune->OpeReg.SpatLevel, &sx, 24 );
			TuneCalV.Gain.pitime = (USHORT)MlibMulgain( 1, kx );
		}
		else
		{ /* PIDuring control */
			/* Pn102 */
			kx = MlibScalKxgain( OnePrmTune->OpeReg.SpatLevel, 100, TuneCalV.KpZeta, NULL, 24 );
			TuneCalV.Gain.posgn = (USHORT)MlibMulgain( 1, kx );

			/* Pn101 */
			kx = MlibPcalKxgain( 800000, 1, OnePrmTune->OpeReg.SpatLevel, NULL, 24 );
			TuneCalV.Gain.pitime = (USHORT)MlibMulgain( 1, kx );
		}

		/*--------------------------------------------------------------------------*/
		/*	Gain upper and lower limit check													*/
		/*--------------------------------------------------------------------------*/
		/* Pn101 */
		if( TuneCalV.Gain.pitime > TUNE_MAX_TI )
		{
			TuneCalV.Gain.pitime = TUNE_MAX_TI;
		}
		TuneCalV.Gain.pitime = TunePrmLimitUnsigned( TuneCalV.Gain.pitime, &pndef_pitime );

		/* Pn102 */
		TuneCalV.Gain.posgn = TunePrmLimitUnsigned( TuneCalV.Gain.posgn, &pndef_posgn );

		if( DobsAct == TRUE )
		{
			TuneCalV.Gain.dobgn  = TuneKvFuncDobsgn( OnePrmTune->OpeReg.SpatLevel );
			//TuneDobsActive( Prm );
			TuneDobsActive( Prm, AccessPrm->hRegCfg );//<S09E>
		}
	}

	/*------------------------------------------------------------------------------*/
	/*	RAM																*/
	/*------------------------------------------------------------------------------*/
//	rc = RpiPrmWriteRamForFn( &pndef_loophz, OnePrmTune->OpeReg.SpatLevel, Hmsg );
//	if(rc != OK)	{return(rc);}
	/* Pn100 */
	TunePrmWriteRam( AccessPrm, &pndef_loophz, OnePrmTune->OpeReg.SpatLevel, 2 );

//	rc = RpiPrmWriteRamForFn( &pndef_pitime, TuneCalV.Gain.pitime, Hmsg );
//	if(rc != OK)	{return(rc);}
	/* Pn101	*/
	TunePrmWriteRam( AccessPrm, &pndef_pitime, TuneCalV.Gain.pitime, 2 );

//	rc = RpiPrmWriteRamForFn( &pndef_posgn, TuneCalV.Gain.posgn, Hmsg );
//	if(rc != OK)	{return(rc);}
	/* Pn102 */
	TunePrmWriteRam( AccessPrm, &pndef_posgn, TuneCalV.Gain.posgn, 2 );

	if( DobsAct == TRUE )
	{
//		rc = RpiPrmWriteRamForFn( &pndef_dobgn, TuneCalV.Gain.dobgn, Hmsg );
//		if(rc != OK)	{return(rc);}
		/* Pn121 */
		TunePrmWriteRam( AccessPrm, &pndef_dobgn, TuneCalV.Gain.dobgn, 2 );
	}

	if( (OnePrmTune->OpeReg.SpatMode == 2) || (OnePrmTune->OpeReg.SpatMode == 3) )
	{
		/* Pn141 */
//		rc = RpiPrmWriteRamForFn( &pndef_mdlgn, OnePrmTune->var.FfSetting, Hmsg );
//		if(rc != OK)	{return(rc);}
		TunePrmWriteRam( AccessPrm, &pndef_mdlgn, OnePrmTune->var.FfSetting, 2 );

		/* Pn140 */
		ulwork = Prm->mdlsw;
		ulwork |= OPTUNE_MFC_ACTIVE;
		TunePrmWriteRam( AccessPrm, &pndef_mdlsw, ulwork, 2 );
//		rc = RpiPrmWriteRamForFn( &pndef_mdlsw, (USHORT)ulwork, Hmsg );	
//		if(rc != OK)	{return(rc);}
	}
	else
	{
		/* Pn140 */
		ulwork = Prm->mdlsw;
		ulwork = ulwork & ~OPTUNE_MFC_ACTIVE;
		TunePrmWriteRam( AccessPrm, &pndef_mdlsw, ulwork, 2 );
//		rc = RpiPrmWriteRamForFn( &pndef_mdlsw, (USHORT)ulwork, Hmsg );
//		if(rc != OK)	{return(rc);}
	}

	return(rc);
}


/*****************************************************************************************************
	Fn203 Set One Parameter Tuning Notch
*****************************************************************************************************/
static LONG	onePrmSetTuningNotch( ONEPRMTUNE *OnePrmTune, AUTONOTCH *ANotch, 
												REG_ACC_T *AccessPrm, PRMDATA *Prm )
{
	LONG		rc;
	TUNECALV	TuneCalV;

	/* Local variable initialization */
	rc = OK;

	/*------------------------------------------------------------------------------*/
	/*	Setting calculation parameters														*/
	/*------------------------------------------------------------------------------*/
	/* Notch Q value & Torque command filter setting	*/
	TuneNotchQ( ANotch,
				AccessPrm->hRegCfg,
				OnePrmTune->OpeReg.SpatLevel,
				OnePrmTune->var.Jrate,
				0,
				Prm );
	/* Getting setting results */
	TuneGetNotchQSetting( ANotch,
						  &TuneCalV.Gain.ntchq1,
						  &TuneCalV.Gain.ntchq2,
						  &TuneCalV.Gain.trqfil11 );

	/*------------------------------------------------------------------------------*/
	/*	Upper/lower limit check																*/
	/*------------------------------------------------------------------------------*/
	/* Pn401 */
	TuneCalV.Gain.trqfil11 = TunePrmLimitUnsigned( TuneCalV.Gain.trqfil11, &pndef_trqfil11 );

	/*------------------------------------------------------------------------------*/
	/*	RAM																*/
	/*------------------------------------------------------------------------------*/
//	rc = RpiPrmWriteRamForFn( &pndef_trqfil11, TuneCalV.Gain.trqfil11, Hmsg );
//	if(rc != OK)	{return(rc);}
	/* Pn401 */
	TunePrmWriteRam( AccessPrm, &pndef_trqfil11, TuneCalV.Gain.trqfil11, 2 );

//	rc = RpiPrmWriteRamForFn( &pndef_ntchq1, TuneCalV.Gain.ntchq1, Hmsg );
//	if(rc != OK)	{return(rc);}
	/* Pn40A */
	TunePrmWriteRam( AccessPrm, &pndef_ntchq1, TuneCalV.Gain.ntchq1, 2 );

//	rc = RpiPrmWriteRamForFn( &pndef_ntchq2, TuneCalV.Gain.ntchq2, Hmsg );
//	if(rc != OK)	{return(rc);}
	/* Pn40D */
	TunePrmWriteRam( AccessPrm, &pndef_ntchq2, TuneCalV.Gain.ntchq2, 2 );

	return(rc);
}



/*****************************************************************************************************
	Fn203 Update One Parameter Tuning Gain In EEPROM
*****************************************************************************************************/
static LONG	onePrmUpdateGainInEeprom( REGIF_CONFIG_T *RegCfg, BOOL DobsAct, LONG MfcModel )
{
	LONG	rc;

	/* Pn100 Speed loop gain */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_loophz, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn101 Speed loop integral time constant */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_pitime, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn102 Position loop gain */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_posgn, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn103 Moment of inertia ratio */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_jrate, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn140 Model following control related switches */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlsw, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn401 1Stage1Torque command filter time constant */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_trqfil11, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn408 Torque related function switch */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_tfuncsw, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn409 1st stage notch filter frequency */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn40A 1Stage notch filterQvalue */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchq1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn40C 2Stage notch filter frequency */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn40D 2Stage notch filterQvalue */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchq2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	if( DobsAct )
	{
		/* Pn121 Friction compensation gain */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_dobgn, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}

		/* Pn123 Friction compensation coefficient */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_dtrqgn, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}

		/* Pn124 Friction compensation frequency correction */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_dlpfil, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}

		/* Pn125 Friction compensation gain correction */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_dobjgn, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}
	}

	if( MfcModel != 0 )
	{
		/* Pn141 Model following control gain */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlgn, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}

//		rc = RpiFunPrmNvmemUpdate( &pndef_mdlzeta );	/* Pn142	*/
		/* Pn142 Model following control gain correction */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlzeta, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}

		/* Pn143 Model following control bias(Forward rotation direction) */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlptff, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}

		/* Pn144 Model following control bias(Reverse direction) */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlntff, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}

		/* Pn147 Model following control speed feedforward compensation */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlvff, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return( rc );
		}
	}

//	rc = RpiFunPrmNvmemUpdate( &pndef_avibfrq );		/* Pn 161	*/
	/* Pn161 A-type vibration damping frequency */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_avibfrq, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn163 A type vibration suppression damping gain */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_avibdamp, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn160 Damping control related switches */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_avibsw, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}


	return(rc);
}



/************************************************************************************************/
/*		Tuning mode setting confirmation process															*/
/************************************************************************************************/
static void	onePrmCheckTuningMode( ONEPRMTUNE *OnePrmTune )
{
	/*------------------------------------------------------------------------------------------*/
	/*	Tuning mode condition judgment																*/
	/*------------------------------------------------------------------------------------------*/
	if( (OnePrmTune->OpeReg.SpatMode < OnePrmTune->var.ModeMin)
		|| (OnePrmTune->OpeReg.SpatMode > OnePrmTune->var.ModeMax) )
	{	/* Tuning mode setting does not match the functional conditions */
		OnePrmTune->var.ModeUnmatch = TRUE;
	}
	else
	{
		OnePrmTune->var.ModeUnmatch = FALSE;
	}
	return;
}



/************************************************************************************************/
/*		Tuning mode setting process (RegisterIF)													*/
/************************************************************************************************/
PRM_RSLT	RpiFunSetOnePrmTuningMode( ONEPRMTUNE *OnePrmTune, USHORT Mode )
{
	PRM_RSLT	rc;
	rc = PRM_RSLT_SUCCESS;

	/*------------------------------------------------------------------------------------------*/
	/*	Tuning mode upper/lower limit check														*/
	/*------------------------------------------------------------------------------------------*/
	if( ((SHORT)Mode >= OPTUNE_MODE_MIN) && (Mode <= OPTUNE_MODE_MAX) )
	{
		OnePrmTune->var.ModeUnmatch = FALSE;
		OnePrmTune->OpeReg.SpatMode = Mode;
	}
	else
	{
		OnePrmTune->var.ModeUnmatch = TRUE;
		rc = PRM_RSLT_LIMIT_ERR;
	}
	return( rc );
}



/************************************************************************************************/
/*		Tuning level setting processing (RegisterIF)													*/
/************************************************************************************************/
PRM_RSLT	RpiFunSetOnePrmTuningLevel( ONEPRMTUNE *OnePrmTune, USHORT Lvl )
{
	PRM_RSLT	rc;
	USHORT		LvlLimit;
	rc = PRM_RSLT_SUCCESS;

	/* Upper/lower limit value confirmation */
	LvlLimit = TunePrmLimitUnsigned( Lvl, &pndef_loophz );

	if( LvlLimit == Lvl )
	{ /* Lvlis within the upper and lower limits */
		OnePrmTune->OpeReg.SpatLevel = Lvl;
	}
	else
	{
		rc = PRM_RSLT_LIMIT_ERR;
	}
	return( rc );
}



/************************************************************************************************/
/*		Tuning level setting processing (RegisterIF)													*/
/************************************************************************************************/
PRM_RSLT	RpiFunSetOnePrmTuningFfLevel( ONEPRMTUNE *OnePrmTune, USHORT Lvl )
{
	PRM_RSLT	rc;
	USHORT		LvlLimit;
	rc = PRM_RSLT_SUCCESS;

	/* Upper/lower limit value confirmation */
	LvlLimit = TunePrmLimitUnsigned( Lvl, &pndef_mdlgn );

	if( LvlLimit == Lvl )
	{ /* Lvlis within the upper and lower limits */
		OnePrmTune->var.FfSetting = Lvl;
	}
	else
	{
		rc = PRM_RSLT_LIMIT_ERR;
	}
	return( rc );
}



/************************************************************************************************/
/*		Tuning level(Pn100)Processing 														*/
/************************************************************************************************/
void	RpiRegSetOnePrmTuningLevel( ONEPRMTUNE *OnePrmTune, USHORT Gain )
{
	OnePrmTune->OpeReg.SpatLevel = Gain;
	return;
}



/************************************************************************************************/
/*		Tuning level(Pn141)Processing														*/
/************************************************************************************************/
void	RpiRegSetOnePrmTuningFfLevel( ONEPRMTUNE *OnePrmTune, USHORT Gain )
{
	if( OnePrmTune->var.TunRun == FALSE )
	{
		OnePrmTune->OpeReg.SpatFFLevel = Gain;
	}
	return;
}



/****************************************************************************************************/

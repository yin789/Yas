/****************************************************************************
Description	: Advanced Auto Tuning
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
****************************************************************************/
#include "MercuryGlobals.h"
#include "BaseControlMain.h"
#include "TuneLib.h"
#include "Mlib.h"
#include "KLib.h"
#include "AdvancedAutoTuning.h"
#include "PrmAccessIf.h"
#include "FunManager.h"
#include "JL086ServoIpHw.h"
#include "ExCtrlPrmCalc.h"
#include "BaseControlPrmCalc.h"
#include "PnPrmListTbl.h"
#include "ModelControl.h"
#include "PosManager.h"
#include "CheckMotSts.h"
#include "FnJatOffLine.h"
#include "GainChange.h"


/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
static PRM_RSLT	AdatExecuteAutoTuning( ADATHNDL *AdatHndl, FUNEXE *FunExe );
static PRM_RSLT AdatInitial( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP);

static void	 	AdatSetTuningModeMax( ADATV *AdatV, LONG XfbPulse1, LONG AxisMotType );
static PRM_RSLT AdatModeCheck( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP );
static LONG		AdatPcalMinDistance( ADATP *AdatP, LONG AxisMotType, BOOL LowSpdMotor );
LONG 	AdatPcalDefautDistance( ADATP *AdatP, LONG AxisMotType, BOOL LowSpdMotor ); /* <S145> */

static PRM_RSLT	AdatRefInModeCheck( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP );
static LONG		AdatRefInExeModeExeCheck( ADATP *AdatP );
static void 	AdatScanCSetReq( ADATV *AdatV );
static void		AdatScanCRequest( USHORT Request, ADATV *AdatV );

static PRM_RSLT	AdatJratSet( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status);
static PRM_RSLT	AdatNormVibDetect( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status );
static PRM_RSLT AdatOptGainSearch( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status );
static PRM_RSLT	AdatNotchFilSet( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status );
static PRM_RSLT	AdatKpGainSet(ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status );
static PRM_RSLT	AdatOptKpSearch( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status );
static PRM_RSLT	AdatMfcAdjustment( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status );
static PRM_RSLT	AdatFinishOpe( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT err_status );

static void		AdatGainDownforVibration( ADATV *AdatV, ADATP *AdatP, USHORT ser_kv, UCHAR down_ratio);
static SHORT	AdatNotchFilReset( ADATV *AdatV, ADATP *AdatP );
static SHORT 	AdatAresCtrlReset( ADATV *AdatV, ADATP *AdatP );
static SHORT 	TuneAdatSetSpdCtrlGain( LONG mode, ADATP *AdatP);
static SHORT	TuneAdatSetPosCtrlGain( LONG mode, ADATP *AdatP);
static SHORT	TuneAdatSetMdlCtrlGain( LONG mode, ADATP *AdatP );
static void		MfcPrmReSet( ADATV *AdatV, ADATP *AdatP );
static void		Mfc2MassReset( ADATV *AdatV, ADATP *AdatP );
static USHORT	SetMfc2MassSequence( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, ULONG ADAT_MFC );

static void		AdatUndoRamGain( ADATV *AdatV, ADATP *AdatP );
static void		AdatUndoRamGainforJrat( ADATV *AdatV, ADATP *AdatP );
static LONG		AdatUpdateGainInEeprom( ADATREG *AdatReg, ADATP *AdatP );
static SHORT	AdatPrgJogDrive( ADATV	*AdatV, ADATP *AdatP );
static void		AdatPJogInit( ADATV	*AdatV, ADATP *AdatP );
static void		AdatPJogDrvSet( ADATV *AdatV, ADATP *AdatP, ULONG pattern );

static SHORT	LpxAdatVibLvlSet( ADATV *AdatV, ADATP *AdatP );
static SHORT	LpxAdatCheckKv( ADATV *AdatV, ADATP *AdatP );
static SHORT	LpxAdatSearchKv( ADATV *AdatV, ADATP *AdatP );
static SHORT	LpxAdatKpGainSet( ADATV *AdatV, ADATP *AdatP );
static SHORT	LpxAdatSearchKp( ADATV *AdatV, ADATP *AdatP );
static SHORT	LpxAdatAdjustMfcGain( ADATV *AdatV, ADATP *AdatP );
static SHORT	LpxAdatCheckMfcGain( ADATV *AdatV, ADATP *AdatP );
static SHORT	LpxAdatRefInTypeDrvStatus( ADATHNDL *AdatHndl, ULONG DrvPattern, BOOL PrefInFlag );


/****************************************************************************/
/*		Start AAT Initialize												*/
/****************************************************************************/
void AdatSetStrPtr( void *Axis )
{
	AXIS_HANDLE *pAxis;
	ADATHNDL *AdatHndl;

	pAxis = ((AXIS_HANDLE*)Axis);
	AdatHndl = pAxis->AdatHndl;

	/* Initialize */
	AdatHndl->AdatP.FnCmnCtrl    = pAxis->FnCmnCtrl;
	AdatHndl->AdatP.UniPrms      = pAxis->UniPrms;
	AdatHndl->AdatP.MencV        = pAxis->MencV;
	AdatHndl->AdatP.BaseCtrlData = pAxis->BaseControls;
	AdatHndl->AdatP.RegManager   = pAxis->RegManager;
	AdatHndl->AdatP.SeqMotSts	 = pAxis->SeqMotSts;
	AdatHndl->AdatP.SeqCtrlOut   = pAxis->SeqCtrlOut;
	AdatHndl->AdatP.AlmManager   = pAxis->AlmManager;
	AdatHndl->AdatP.SettlingTime = &(pAxis->BaseControls->SettlingTime);
	AdatHndl->AdatP.ANotch       = pAxis->ANotch;
	AdatHndl->AdatP.RemVibFreq   = pAxis->RemVibFreq;
	AdatHndl->AdatP.FftAna       = pAxis->FftAna;
	AdatHndl->AdatP.SvAsicRegs   = pAxis->SvAsicRegs;
	AdatHndl->AdatP.MotSpdMafil  = pAxis->MotSpdMafil;
	AdatHndl->AdatP.DetVib       = pAxis->DetVib;
	AdatHndl->AdatP.SvControlIf  = pAxis->SvControlIf;
}

/****************************************************************************
	Fn201 Advanced Auto Tuning mode Begin
****************************************************************************/
PRM_RSLT FnEnterAdvAutoTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	ADATREG	 *AdatReg;
	ADATV	 *AdatV;
	ADATP	 *AdatP;
	PRM_RSLT errRes;

	AdatReg	  = &Axis->AdatHndl->AdatReg;
	AdatV	  = &Axis->AdatHndl->AdatV;
	AdatP	  = &Axis->AdatHndl->AdatP;

	/* ProgramJOGOperation Set variables */
	MlibCopyLongMemory( &AdatP->BaseCtrlData->PJogHdl.PJogV,
						&Axis->AdatHndl->OrgPJogV,						/* @@ check */
						sizeof(AdatP->BaseCtrlData->PJogHdl.PJogV)>>2 );

	/* ProgramJOGOperation Command creation variable reset */
	PrgJogReset( &AdatP->BaseCtrlData->PJogHdl );

	/* Advanced autotuning related information clear	*/
	MlibResetLongMemory( AdatV, sizeof(Axis->AdatHndl->AdatV)>>2 );
	AdatSetTuningModeMax( AdatV, AdatP->UniPrms->Bprm->XfbPulse1, AdatP->MencV->AxisMotType );
	errRes = AdatModeCheck( AdatReg, AdatV, AdatP );

	if( errRes == PRM_RSLT_SUCCESS )
	{
		/* Ends without turning on when servo is on	*/
		if(AdatP->FnCmnCtrl->State.FnSvCtrlDisable != FALSE)
		{
			return PRM_RSLT_CONDITION_ERR;
		}
		
		/* If in motorless test mode, exit without entering *//* <S1B7> */
		if( Axis->MotorLessTest->MotorLessTestMode == TRUE )
		{
			return(PRM_RSLT_CONDITION_ERR);
		}

		AdatP->FnCmnCtrl->FnSvControl = TRUE;	/* FnServo operation privilege acquisition	*/
		FunExe->HoldFnMode = TRUE;				/* FnMode automatic end */
	}
	AdatV->f.RefInType = FALSE;
	AdatReg->AdatState = ADAT_STS_INIT;			/* Status initialization		*/

	return(errRes);
}

/****************************************************************************
	Fn201 Tuning mode Check
****************************************************************************/
static void	AdatSetTuningModeMax( ADATV	*AdatV, LONG XfbPulse1, LONG AxisMotType )
{
	AdatV->TunModeMax = ADAT_LEVEL_MAX;						/* Initial value is maximum setting value	*/

		if( AxisMotType == MOTTYPE_ROTARY )					/* Rotary type motor			*/
		{
			//Not compatible with Fulk
			//if( Kprm.f.FencUse == FALSE )					/* Semi-closed control		*/
			{
				if( XfbPulse1 <= 0x2000 )					/* 13bit[p/rev]		*/
				{
					AdatV->TunModeMax = ADAT_LEVEL_MIN;		/* Adjustment mode: Standard only	*/
				}
			}
		}
		return;
}

/****************************************************************************
	Fn201 Advanced Auto Tuning mode Condition Check
****************************************************************************/
static PRM_RSLT AdatModeCheck( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP )
{
	BASE_CTRL_HNDL *BaseCtrlData;
	PRM_RSLT rc;

	BaseCtrlData = AdatP->BaseCtrlData;
	rc = PRM_RSLT_SUCCESS;

	/* Data error when servo is on, torque control, automatic gain switching enabled(REGIF_DAMTHERR)only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.		*/
	/* Even when deviation-less control, predictive control, and normal auto tuning are enabled, there is a data error, but it is not supported	*/
	if( ((BaseCtrlData->BeComplete == TRUE) && (BaseCtrlData->CtrlModeSet.CtrlMode.b.cm == CTRLMODE_TRQ))
	  || (GselGet_AtGselStatus( &BaseCtrlData->GainChange ) == TRUE))
	{
		rc = PRM_RSLT_CONDITION_ERR;
	}

	/* Tuning mode consistency check	*/
	if( AdatV->TunModeMax < AdatReg->AdatLevel )
	{
		rc = PRM_RSLT_CONDITION_ERR;
	}

	AdatV->ModeMin    = ADAT_MODE_MIN;
	AdatV->ModeMax    = ADAT_MODE_MAX;
	AdatV->ModeSetMin = ADAT_SCROLL_MIN;
	AdatV->ModeSetMax = ADAT_SCROLL_MAX;

	return( rc );
}

/****************************************************************************
	Fn201 Advanced Auto Tuning mode Execution
****************************************************************************/
PRM_RSLT	FnExecuteAdvAutoTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT rc;
	rc = AdatExecuteAutoTuning( Axis->AdatHndl, FunExe );
	return rc;
}

/****************************************************************************
	Fn201 Advanced Auto Tuning mode End
****************************************************************************/
void FnLeaveAdvAutoTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	ADATV			*AdatV;
	ADATP			*AdatP;
	REG_ACC_T		AccessPrm;
	TUNELESS_CFGPRM	TuneLessCfgPrm;

	AdatV = &Axis->AdatHndl->AdatV;
	AdatP = &Axis->AdatHndl->AdatP;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/* During round controlSENSignalONRequestI/O */
	AdatP->FnCmnCtrl->FnSensOnReq = (((AdatP->SeqCtrlOut->RspSeqBits & SENS_ON_COMP_BITMSK)) == TRUE)
																		? TRUE : FALSE;

	AdatP->FnCmnCtrl->State.TuningRun      = FALSE;
	AdatP->FnCmnCtrl->State.TuningPctrlRun = FALSE;

	/* Operation privilege release processing(ScanC IF)	*/
	AdatP->FnCmnCtrl->FnSvControl = FALSE;

	/* Even with inertia identificationKv,JLetc. are returned to their original values, so based onAdatUndoRamGain()	*/
	CpiEndJatOffLine( &AdatP->BaseCtrlData->JatHdl, &AccessPrm );

	MlibCopyLongMemory( &Axis->AdatHndl->OrgPJogV,						/* @@  check */
						&AdatP->BaseCtrlData->PJogHdl.PJogV,
						sizeof( AdatP->BaseCtrlData->PJogHdl.PJogV )>>2 );

	AdatUndoRamGain( AdatV, AdatP );

	DetVibResetMaxMinSpeedErrorFn( AdatP->DetVib );
	DetVibObsCalculateInitPrmAAT( &(AdatP->DetVib->DetVibObs),
	                              AdatP->BaseCtrlData->TuneLessCtrl.conf.TuningLessUse,
	                              AdatP->UniPrms->Bprm->SvCycleNs );

	/* Adjustment-less function switch recalculation */
	TuneLessCfgPrm.gnmode	= (AdatP->UniPrms->Prm)->gnmode;
	TuneLessCfgPrm.mfctype	= (AdatP->UniPrms->Prm)->mfctype;
	TuneLessCfgPrm.avibsw	= (AdatP->UniPrms->Prm)->avibsw;
	TuneLessCfgPrm.DatLevel	= (AdatP->UniPrms->Prm)->DatLevel;
	TuneLessCfgPrm.tfuncsw 	= (AdatP->UniPrms->Prm)->tfuncsw;
	TuneLessCfgPrm.flg_wf 	= (AdatP->UniPrms->Prm)->MencP.flg_wf;
	TuneLessCalculateInitPrm( AdatP->BaseCtrlData,
	                          AdatP->UniPrms->Bprm,
	                          AdatP->MencV,
							  AdatP->SvAsicRegs,
						      &TuneLessCfgPrm );

#if 0 /* 2012.08.31 Y.Oka ★Motor speed detection moving average Currently not supported★ */
	/* Return the moving average time constant for motor speed detection	*/
	if( AdatP->BaseCtrlData->TuneLessCtrl.conf.TuningLessUse == TRUE )
	{
		BpiChangeSpdDetMaFil( AdatP->MotSpdMafil,
							  0,
							  (USHORT*)&(AdatP->SvAsicRegs->AsicMicroReg->MREG_SPDMANUMBITI) );
	}
	else
	{
		BpiChangeSpdDetMaFil( AdatP->MotSpdMafil,
							  -1,
							  (USHORT*)&(AdatP->SvAsicRegs->AsicMicroReg->MREG_SPDMANUMBITI) );
	}
#endif

	/* Termination processing	*/
	AdatP->FnCmnCtrl->JogSpeed    = 0;
	AdatP->FnCmnCtrl->FnSvonReq   = FALSE;
	AdatP->FnCmnCtrl->FnCtrlMcmd  = CTRLMODE_NOCMD;
	AdatP->FnCmnCtrl->FnSensOnReq = FALSE;
	AdatP->FnCmnCtrl->FnSvControl = FALSE;

}

/****************************************************************************
	Fn202 Reference Input Type Advanced Auto Tuning mode Begin
****************************************************************************/
PRM_RSLT	FnEnterRefInAdvAutoTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	ADATREG		*AdatReg;
	ADATV		*AdatV;
	ADATP		*AdatP;
	PRM_RSLT 	errRes;

	AdatReg   = &Axis->AdatHndl->AdatReg;
	AdatV     = &Axis->AdatHndl->AdatV;
	AdatP     = &Axis->AdatHndl->AdatP;
	errRes    = PRM_RSLT_SUCCESS;

	/* Advanced autotuning related information clear */
	MlibResetLongMemory( AdatV, sizeof(Axis->AdatHndl->AdatV)>>2 );

	/* Adjustment mode maximum value setting */
	AdatSetTuningModeMax( AdatV, AdatP->UniPrms->Bprm->XfbPulse1, AdatP->MencV->AxisMotType );

	AdatV->f.RefInType = TRUE;
	errRes = AdatRefInModeCheck(AdatReg, AdatV, AdatP);

	/* If in motorless test mode, exit without entering *//* <S1B7> */
	if( Axis->MotorLessTest->MotorLessTestMode == TRUE )
	{
		return(PRM_RSLT_CONDITION_ERR);
	}

	AdatReg->AdatState = ADAT_STS_INIT;	/* Status initialization		*/
	FunExe->HoldFnMode = TRUE;			/* FnMode automatic end */

	return(errRes);
}


/****************************************************************************
	Fn202 Advanced Auto Tuning mode Condition Check
****************************************************************************/
static PRM_RSLT	AdatRefInModeCheck( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP )
{
	BASE_CTRL_HNDL *BaseCtrlData;
	PRM_RSLT errRes;

	BaseCtrlData = AdatP->BaseCtrlData;
	errRes = PRM_RSLT_SUCCESS;

	/* When using torque control or speed control and the servo is on, or when automatic gain switching is enabled, a data error occurs.(REGIF_DAMTHERR)only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.*/
	/* Even when deviation-less control, predictive control, and normal auto tuning are enabled, there is a data error, but it is not supported	*/
	if( ((BaseCtrlData->BeComplete == TRUE) && (BaseCtrlData->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS))
	  || (GselGet_AtGselStatus( &BaseCtrlData->GainChange ) == TRUE)
 	  || (BaseCtrlData->TuneLessCtrl.conf.TuningLessUse  == TRUE) )
	{
		errRes = PRM_RSLT_CONDITION_ERR;
	}

	/* Tuning mode consistency check	*/
	if(AdatV->TunModeMax < AdatReg->AdatLevel)
	{
		errRes = PRM_RSLT_CONDITION_ERR;
	}

	AdatV->ModeMin = ADAT_MODE_MIN;
	AdatV->ModeMax = ADAT_MODE_MAX;
	AdatV->ModeSetMin = ADAT_REFIN_SCROLL_MIN;
	AdatV->ModeSetMax = ADAT_REFIN_SCROLL_MAX;

	return(errRes);
}

/****************************************************************************
	Fn202 Advanced Auto Tuning mode Execution
****************************************************************************/
PRM_RSLT	FnExecuteRefInAdvAutoTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT rc;
	rc = AdatExecuteAutoTuning( Axis->AdatHndl, FunExe );
	return rc;
}


/****************************************************************************
	Fn202 Reference Input Type Advanced Auto Tuning mode End
****************************************************************************/
void FnLeaveRefInAdvAutoTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	ADATV	*AdatV;
	ADATP	*AdatP;
	FUN_CMN_CONTROL	*FnCmnCtrl;

	AdatV	   = &Axis->AdatHndl->AdatV;
	AdatP	   = &Axis->AdatHndl->AdatP;
	FnCmnCtrl  = Axis->FnCmnCtrl;

	FnCmnCtrl->State.TuningRun      = FALSE;
	FnCmnCtrl->State.TuningPctrlRun = FALSE;

	AdatUndoRamGain( AdatV, AdatP );

	/* Vibration detection differential speed maximum/minimum value reset */
	DetVibResetMaxMinSpeedError( AdatP->DetVib );

	/* Vibration detection differential speed maximum/minimum value reset */
	DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs,
	                         VIBOBSGN, 0, DETVIBHP, (AdatP->UniPrms->Bprm)->SvCycleNs);

	/* Termination processing	*/
	FunExe->HoldFnMode = FALSE;				/* FnMode automatic end */
}


/****************************************************************************
	Advanced Auto Tuning  Execution (Fn201,Fn202 Common)
****************************************************************************/
static PRM_RSLT	AdatExecuteAutoTuning( ADATHNDL *AdatHndl, FUNEXE *FunExe )
{
	ADATREG	*AdatReg;
	ADATV	*AdatV;
	ADATP	*AdatP;
	REG_ACC_T AccessPrm;

	PRM_RSLT rc;
	ULONG	 data;
	LONG	 UnRvibFrequency;

	AdatReg	  = &AdatHndl->AdatReg;
	AdatV	  = &AdatHndl->AdatV;
	AdatP	  = &AdatHndl->AdatP;

	rc  = PRM_RSLT_SUCCESS;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/*------------------------------------------------------------------------------------------*/
	/*	Common operation processing																			*/
	/*------------------------------------------------------------------------------------------*/
	if( AdatV->f.DrvErrDetect == TRUE )
	{
		if(   (AdatP->AlmManager->AlmCode != 0x00)
			|| (AdatP->SeqCtrlOut->MainPowerOn == FALSE)
			|| (AdatP->SeqCtrlOut->OverTrvlSts == TRUE)
			|| ((AdatV->f.RefInType == TRUE) && (AdatP->FnCmnCtrl->State.TuningRun == TRUE) && (AdatP->SeqCtrlOut->BaseEnableReq == FALSE))
			|| (AdatP->BaseCtrlData->GainChange.ActGain != 1)
			|| (AdatP->SeqCtrlOut->HwbbSts == TRUE) ) /* GAIN1 If HWBBStatus			*/
		{
			AdatV->ErrState = ADAT_NOTRDYERR;			/* Operation preparation incomplete error 					*/
			FunExe->State   = ADAT_SEQ_END;				/* Final setting state	    					*/
		}
		else if( AdatV->ErrState <= ADAT_NG )
		{
			FunExe->State = ADAT_SEQ_END;
		}
	}

	switch ( FunExe->CmdCode )
	{
		case FCMD_SV :
			if( AdatV->f.RefInType == FALSE )
			{
//				if( (AdatP->AlmManager->AlmReady    == TRUE)		/* With alarm 		*//* <S085> */
				if( (AdatP->AlmManager->AlmReady    == FALSE)		/* With alarm 		*/
				 || (AdatP->SeqCtrlOut->HwbbSts     == TRUE)		/* HWBBcan be				*/
//				 || (AdatP->SeqCtrlOut->MainPowerOn == TRUE)		/* Main circuitOFF			*//* <S085> */
				 || (AdatP->SeqCtrlOut->MainPowerOn == FALSE)		/* Main circuitOFF			*/
				 || (AdatP->SeqCtrlOut->OverTrvlSts == TRUE) )		/* OTcan be				*/
				{
					;
				}
				else
				{
//					if( AdatP->FnCmnCtrl->FnSvonReq == TRUE )
					if( AdatP->FnCmnCtrl->FnSvonReq != FALSE )		/* <S085> */
					{
						AdatP->FnCmnCtrl->FnSvonReq = FALSE;
						FunExe->State = ADAT_SEQ_INIT;
					}
				}
			}
			break;

		case FCMD_STOP :
			AdatV->ErrState = ADAT_COMERR;
			FunExe->State = ADAT_SEQ_END;
			break;

		case FCMD_UP :
		case FCMD_DOWN :
		default :
			break;
	}

	switch ( FunExe->State )
	{
		/*--------------------------------------------------------------------------------------*/
		/*	Initial state																			*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_INIT :
			AdatP->FnCmnCtrl->State.TuningRun = FALSE;

			if( (AdatV->f.RefInType == FALSE)	 					/* Internal command type				*/
			 || (AdatP->BaseCtrlData->PosCtrlSts.CoinSignal == TRUE)/* COIN						*/
			 || (AdatP->SeqCtrlOut->MotStop == TRUE) )				/* Motor stop detection 			*/
			{	/* Command stop judgment	*/
				FunExe->State = ADAT_SEQ_START;				/* After motor stop, proceed to initialization process		*/
			}
			break;
		/*--------------------------------------------------------------------------------------*/
		/*	Initialization status																		*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_START :
			AdatV->f.GainTuneRun = FALSE;

			/* AdvancedATInitialization process	*/
			rc = AdatInitial( AdatReg, AdatV, AdatP );
			TuneFilGainCalbyFilType( AdatP->ANotch, (UCHAR)AdatReg->AdatFilType );
			/*----------------------------------------------------------------------------------*/
			/*	Initialize gain																	*/
			/*----------------------------------------------------------------------------------*/
			TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );				/* Current settingsKv, Balance		*/
			data = ADAT_GRAT_50( AdatP->UniPrms->Prm->loophz );		/* Kvof1/2					*/
			TunePrmWriteRam( &AccessPrm, &pndef_posgn, (LONG)data, pndef_posgn.DataLength );

			/* Tuning mode setting is done via communication, so go to inertia setting */
			FunExe->State = ADAT_SEQ_JRATSET;

			//Data write prohibition setting function when an auxiliary function is executed
			//RpiFunDataWriteDisableSet( FPRM_IF, ON, pfexe );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Inertia setting state																	*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_JRATSET :
			AdatV->f.GainTuneRun = TRUE;

			/* AdvancedATInertia setting process */
			rc = AdatJratSet( AdatReg, AdatV, AdatP, FunExe, AdatV->ScanCStat );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Normal vibration level detection & load torque measurement status												*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_NORMVIB:
			AdatV->f.GainTuneRun = TRUE;
			AdatV->f.StopDetectMode2 = TRUE;
			AdatReg->AdatState = ADAT_STS_VIBDET; /* Vibration level measurement in progress						*/

			/* AdvancedATNormal vibration level detection process	*/
			rc = AdatNormVibDetect( AdatReg, AdatV, AdatP, FunExe, AdatV->ScanCStat );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	OptimumKv,TiGain search state															*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_SEARCHOPTKV:
			AdatV->f.GainTuneRun = TRUE;
			AdatV->f.StopDetectMode2 = TRUE;
			AdatReg->AdatState = ADAT_STS_OPTGNADJ; /* Operation evaluation gain search in progress					*/

			/* AdvancedATOptimum gain search processing */
			rc = AdatOptGainSearch( AdatReg, AdatV, AdatP, FunExe, AdatV->ScanCStat );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Notch filter setting status																*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_NOTCHSET:
			AdatV->f.GainTuneRun = TRUE;
			AdatReg->AdatState = ADAT_STS_VIBDISSET; /* Vibration suppression setting in progress						*/

			/* AdvancedATNotch filter setting processing	*/
			rc = AdatNotchFilSet( AdatReg, AdatV, AdatP, FunExe, AdatV->FftStat );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	KpSetting state																			*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_KPSET:
			AdatV->f.GainTuneRun = TRUE;
			AdatV->f.StopDetectMode2 = TRUE;
			AdatReg->AdatState = ADAT_STS_OPTGNADJ; /* Operation evaluation gain search in progress 				*/

			/* AdvancedAT KpSetting processing */
			rc = AdatKpGainSet( AdatReg, AdatV, AdatP, FunExe, AdatV->ScanCStat );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	OptimumKpGain search state																*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_SEARCHOPTKP:
			AdatV->f.GainTuneRun = TRUE;
			AdatV->f.StopDetectMode2 = FALSE;
			/* Operation evaluation gain search in progress	*/
			AdatReg->AdatState = ADAT_STS_OPTGNADJ;
			/* AdvancedATOptimumKpGain search processing	*/
			rc = AdatOptKpSearch( AdatReg, AdatV, AdatP, FunExe, AdatV->ScanCStat );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Model following control adjustment state																*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_ADJMFC:
			AdatV->f.GainTuneRun = TRUE;
			AdatV->f.StopDetectMode2 = FALSE;
			/* Operation evaluation gain search in progress */
			AdatReg->AdatState = ADAT_STS_OPTGNADJ;
			/* AdvancedATModel following control adjustment processing	*/
			rc = AdatMfcAdjustment( AdatReg, AdatV, AdatP, FunExe, AdatV->ScanCStat );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Termination display status																		*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_ENDDISP:
			FunExe->State = ADAT_SEQ_END;
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Final setting state																		*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_END:
			/* AdvancedATTermination setting processing */
			rc = AdatFinishOpe( AdatReg, AdatV, AdatP, FunExe, AdatV->ErrState );
			break;
		/*--------------------------------------------------------------------------------------*/
		/*	Waiting for termination state																		*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_WAIT:
			AdatP->FnCmnCtrl->State.TuningRun = FALSE;
			break;
#if 0
		//Σ-Ⅴ-MDNot used
		/*--------------------------------------------------------------------------------------*/
		/*	Post-processing status																			*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_RETURN:
			if( AdatV->f.RefInType == TRUE )
			{
				FnLeaveRefInAdvAutoTuning( FunExe, Axis );
			}
			else
			{
				FnLeaveAdvAutoTuning( FunExe, Axis);
			}
			break;

		case ADAT_SEQ_NOOPWAIT :
			if( KlibGetLongTimerMs( FunExe->LongTimer ) > 1000 )
			{
				FunExe->State = ADAT_SEQ_RETURN;
			}
			break;
#endif
		/*--------------------------------------------------------------------------------------*/
		/*	MFC(Rigid body+Machine base model)Setting state														*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_SEQ_2MASSSET:
			AdatV->f.GainTuneRun = TRUE;
			AdatV->f.StopDetectMode2 = FALSE;

			AdatReg->AdatState = ADAT_STS_VIBDISSET;		/* MFC(Rigid body+Machine base model)Setting in progress 	*/

			/* MFC(Rigid body+Machine base model)Set flagON 											*/
			/* Detect vibration2InertiaMFCAdatV->f.Used2MassMfc = TRUE					*/
			AdatV->f.Used2MassMfc = TRUE;

			/* Kpmis the anti-resonant frequency.Kpm [1/s] = wL / 2 [Hz]	*/
			/* then clear1.0HzKpm=1.0[1/s]			*/
			UnRvibFrequency = RemVibGetRemVibFreq( AdatP->RemVibFreq );

			if( UnRvibFrequency <= VIBMFC_DET_FRQ_MIN )
			{
				data = VIBMFC_DET_FRQ_MIN;
			}
			else
			{
				data = ( UnRvibFrequency >> 1 );
			}
			/* Pn141 */
			TunePrmWriteRam( &AccessPrm, &pndef_mdlgn, (LONG)data, pndef_mdlgn.DataLength );

			/* Set vibration frequency,MFC(Rigid body+Machine base model)ofON, */
			data = UnRvibFrequency;
			/* Pn145 */
			TunePrmWriteRam( &AccessPrm, &pndef_mdlantfrq, data, pndef_mdlantfrq.DataLength );
			/* Pn146 */
			TunePrmWriteRam( &AccessPrm, &pndef_mdlresfrq, data, pndef_mdlresfrq.DataLength );
			PrmReLoadValueFromEeprom( AdatP->RegManager->hPnReg, &pndef_mdlsw, 0 );
			data = AdatP->UniPrms->Prm->mdlsw | VIBMFC_PRM_ACTIVE;
			/* Pn140 */
			TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, data, pndef_mdlsw.DataLength );
			FunExe->State = AdatV->NextSeqPtr;				/* Restore status pointer 			*/

			break;
		default:
			break;
	}
	/*------------------------------------------------------------------------------------------*/
	/*	ProgramJOGCommand sequence																*/
	/*------------------------------------------------------------------------------------------*/
//	if(FunExe->State != ADAT_SEQ_RETURN)
	{
		if( AdatV->f.RefInType == FALSE )
		{
			AdatV->PrgJogStat = AdatPrgJogDrive( AdatV, AdatP );
			if( AdatV->PrgJogStat != ADAT_PJOG_NOP )
			{	/* Operation execution status isNOP → Driving */
				switch ( FunExe->CmdCode )
				{
					case FCMD_UP:		/* Forward revolution (Continuous execution) 						*/
						if( AdatV->MoveDistance > 0 )
						{
							KlibRstLongTimer( &AdatV->PrgJogDrvCntr );
						}
						break;

					case FCMD_DOWN:		/* Reverse revolution (Continuous execution) 						*/
						if( AdatV->MoveDistance < 0 )
						{
							KlibRstLongTimer( &AdatV->PrgJogDrvCntr );
						}
						break;

					case FCMD_NONE:		/* No instruction												*/
					default:
						if( (KlibGetLongTimerMs( AdatV->PrgJogDrvCntr ) > (ULONG)(AdatP->FnCmnCtrl->SafeStopTime * 1000)) /* <S195> */
															&& (AdatP->FnCmnCtrl->SafeStopTime != 65535) )
						{	/* Safety stop time[s]or above */
							AdatP->BaseCtrlData->PJogHdl.PJogV.Var.RoutCmd = PJOGCMD_ABORT; /* Operation interruption */
							AdatV->ErrState = ADAT_COMERR;									/* Communication error	*/
						}
						break;
				}
			}
			else
			{	/* During operation stop	*/
				if(AdatV->PrgJogStat < ADAT_PJOG_NOP )
				{/* If an abnormal setting is made, set the error status	*/
					AdatV->ErrState = AdatV->PrgJogStat;
				}

				if( AdatV->f.DrvErrDetect == FALSE )
				{/* Unconditionally initialize communication error detection wait timer while operation is stopped */
					KlibRstLongTimer( &AdatV->PrgJogDrvCntr );
				}
			}
		}
	}
	/*------------------------------------------------------------------------------------------*/
	/*	Frequency analysis processing sequence																*/
	/*------------------------------------------------------------------------------------------*/
	AdatV->FftStat = TuneVibrationTrace( AdatP->DetVib, AdatP->FftAna, AdatV->f.FftReq, 0,  FALSE);

	/*------------------------------------------------------------------------------------------*/
	/*	COINOutput status creation																	*/
	/*------------------------------------------------------------------------------------------*/
	AdatReg->AdatState |= AdatV->CoinWaitStat;						/* COINOutput status		*/

	return rc;
}

/****************************************************************************
	Fn202 Advanced Auto Tuning mode Condition Check
****************************************************************************/
static LONG	AdatRefInExeModeExeCheck( ADATP *AdatP )
{
	LONG rc;
	rc = TRUE;

//	if(((AdatP->BaseCtrlData->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS) && (AdatP->BaseCtrlData->BeComplete == TRUE))
	if((AdatP->BaseCtrlData->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS) || (AdatP->BaseCtrlData->BeComplete == FALSE)	/* <S085> */
	 || (AdatP->AlmManager->AlmCode != 0x00)
	 || (AdatP->SeqCtrlOut->HwbbSts == TRUE)
	 || (AdatP->SeqCtrlOut->MainPowerOn == FALSE)
	 || (AdatP->SeqCtrlOut->OverTrvlSts == TRUE) )
	{
		rc = FALSE;
	}
	return rc;
}

/****************************************************************************
	Fn201 Update One Parameter Tuning Gain In EEPROM
****************************************************************************/
static LONG	AdatUpdateGainInEeprom( ADATREG *AdatReg, ADATP *AdatP )
{
	PRM_RSLT		rc;
	REGIF_CONFIG_T *RegCfg;
	DOBS_CTRL	   *DobsCtrl;
	MFCTRL		   *MFControl;

//	rc        = PRM_RSLT_SUCCESS;/*<S09E>*/
	RegCfg    = AdatP->RegManager->hPnReg;
	DobsCtrl  = &AdatP->BaseCtrlData->DobsCtrl;
	MFControl = &AdatP->BaseCtrlData->MFControl;

	/* Pn100 Speed loop gain */
	rc = PrmWriteValueToEeprom( RegCfg, &pndef_loophz, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn101 Speed loop integral time constant */
	rc = PrmWriteValueToEeprom( RegCfg, &pndef_pitime, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn102 Position loop gain */
	rc = PrmWriteValueToEeprom( RegCfg, &pndef_posgn, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn103 Moment of inertia ratio */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_jrate, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn401 1Stage1Torque command filter time constant */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_trqfil11, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn409 1Stage notch filter frequency */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn40A 1Stage notch filterQvalue */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchq1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn40B 1Stage notch filter depth */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchdamp1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn40C 2Stage notch filter frequency */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn40D 2Stage notch filterQvalue */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchq2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn408 Torque related function switch */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_tfuncsw, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	if( DobsCtrl->var.DobsAct == TRUE )
	{
		/* Pn121 Friction compensation gain */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_dobgn, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}

		/* Pn123 Friction compensation coefficient */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_dtrqgn, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}

		/* Pn125 Friction compensation gain correction */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_dobjgn, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
	}

	if(   ( AdatReg->AdatLevel == ADAT_LEVEL_POS )
	   || ( AdatReg->AdatLevel == ADAT_LEVEL_POS_ST ) )
	{

		/* Pn141 Model following control gain */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlgn, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}

		/* Pn142 Model following control gain correction */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlzeta, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}

		/* Pn143 Model following control bias(Forward rotation direction) */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlptff, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}

		/* Pn144 Model following control bias(Reverse direction) */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlntff, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}

		if(MFControl->conf.MFCModelSetting > ADATMFC_ONE_MASS)
		{	/* 2InertiaMFCEnabled	*/
			/* Pn145 Vibration suppression 1 frequencyA */
			rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlantfrq, 0 );
			if( rc != PRM_RSLT_SUCCESS )
			{
				return(rc);
			}

			/* Pn146 Vibration suppression 1 frequencyB */
			rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlresfrq, 0 );
			if( rc != PRM_RSLT_SUCCESS )
			{
				return(rc);
			}
		}

		/* Pn147 Model following control speed feedforward compensation */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlvff, 0 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
	}

	/* Pn140 Model following control related switches */
	/* AATBefore executionMFCEnabled”Level=1”,Pn140=xx00ofEEPROMand the serial number of the motor connected this time. */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlsw, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn161 A-type vibration damping frequency */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_avibfrq, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn163 A type vibration suppression damping gain */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_avibdamp, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn160 Damping control related switches */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_avibsw, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	/* Pn170 Dynamic auto tuning related switches */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_DatLevel, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	return(rc);
}

/****************************************************************************
		Advanced autotuning parametersUNDOProcessing

	Overview:	Use parameter before function executionRAM.
****************************************************************************/
static void	AdatUndoRamGain( ADATV *AdatV, ADATP *AdatP )
{
	REGIF_CONFIG_T *RegCfg;
	RegCfg   = AdatP->RegManager->hPnReg;

	/* Pn100 Speed loop gain */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_loophz,	0 );

	/* Pn101 Speed loop integral time constant */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_pitime,	0 );

	/* Pn102 Position loop gain */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_posgn,		0 );

	/* Pn103 Moment of inertia ratio */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_jrate,		0 );

	/* Pn401 1Stage1Torque command filter time constant */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_trqfil11,	0 );

	/* Pn409 1Stage notch filter frequency */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil1,	0 );

	/* Pn40A 1Stage notch filterQvalue */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchq1,	0 );

	/* Pn40B 1Stage notch filter depth */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchdamp1, 0 );

	/* Pn40C 2Stage notch filter frequency */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil2,	0 );

	/* Pn40D 2Stage notch filterQvalue */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchq2,	0 );

	/* Pn408 Torque related function switch */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_tfuncsw,	0 );

	/* Pn121 Friction compensation gain */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_dobgn,		0 );

	/* Pn123 Friction compensation coefficient */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_dtrqgn,	0 );

	/* Pn125 Friction compensation gain correction */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_dobjgn,	0 );

	/* Pn141 Model following control gain */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlgn,		0 );

	/* Pn142 Model following control gain correction */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlzeta,	0 );

	/* Pn143 Model following control bias(Forward rotation direction) */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlptff,	0 );

	/* Pn144 Model following control bias(Reverse direction) */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlntff,	0 );

	/* Pn145 Vibration suppression 1 frequencyA */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlantfrq, 0 );

	/* Pn146 Vibration suppression 1 frequencyB */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlresfrq, 0 );

	/* Pn147 Model following control speed feedforward compensation */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlvff,	0 );

	/* Pn140 Model following control related switches */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlsw,		0 );

	/* Pn161 AMold vibration damping frequency */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_avibfrq,	0 );

	/* Pn163 AMold vibration damping damping gain */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_avibdamp,	0 );

	/* Pn160 Damping control related switches */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_avibsw,	0 );

	/* Pn310 Vibration detection switch */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_vibsel,	0 );

	/* Pn311 Vibration detection sensitivity */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_vibsens,	0 );

	/* Pn533 ProgramJOGMovement speed */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_pjogrotspd,0 );

	/* Pn534 ProgramJOGAcceleration/deceleration time */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_pjogacctm, 0 );

	/* Pn531 ProgramJOGTravel distance */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_pjogdistl, 0 );

	/* Pn535 ProgramJOGWaiting time */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_pjogwaittm,0 );

	/* Pn585 ProgramJOGMaximum speed(Linear) */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_pjoglinspd,0 );

	/* Pn560 Residual vibration detection width */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_remdetw,	0 );

	/* Pn170 Dynamic auto tuning related switches */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_DatLevel,	0 );

	AdatUndoRamGainforJrat( AdatV, AdatP );

}

/****************************************************************************
		Advanced autotuning Moment of inertia estimationUNDOProcessing

	Overview:	Moment of inertia estimation related parameters before function executionRAM.
****************************************************************************/
static void	AdatUndoRamGainforJrat( ADATV *AdatV, ADATP *AdatP )
{
	PRMDATA 		*Prm;
	REGIF_CONFIG_T  *RegCfg;

	Prm      = AdatP->UniPrms->Prm;
	RegCfg   = AdatP->RegManager->hPnReg;
	AdatP->BaseCtrlData->CtrlCmdPrm.IPSpdControl = (AdatV->f.OrgIPSpdControl)? TRUE : FALSE;

	/* Position command filter activation */
	BpiPcmdFilSwitch( &AdatP->BaseCtrlData->PosManager, TRUE );

	/* MFCForcibly cancel disable	 	*/
	MfcSetMfcCtrlSwitch( &AdatP->BaseCtrlData->MFControl, TRUE );

	/*----------------------------------------------------------------------*/
	/*	Automatic A-type setting execution status												*/
	/*		A type forced when inertia identification is enabledOFFIn order to be					*/
	/*		Retry,ADetermine whether mold automatic adjustment is valid									*/
	/*----------------------------------------------------------------------*/
	if( ( ((Prm->avibsw >> 4) & 0x0F)  == FALSE) 	/* Automatic adjustment disabled			*/
		|| ((Prm->avibsw & 0x0F) == TRUE) )			/* orAMold already valid	*/
	{
		AdatV->f.UsedAResCtrl = TRUE;				/* Already set				*/
	}
	else
	{
		AdatV->f.UsedAResCtrl = FALSE;				/* Not set				*/
	}

	/* Pn109 Feedforward */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ffgn, 0 );

	/* Pn408 Torque related function switch */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_tfuncsw, 0 );
	if( Prm->tfuncsw & 0xF000 )
	{
		Prm->tfuncsw = Prm->tfuncsw | 0x1000;
		PrmStoreValueToEeprom( RegCfg, &pndef_tfuncsw, 0 );
	}

	/* Pn10B Gain-related application switch */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_gnmode, 0 );

}

/****************************************************************************
		Advanced autotuning operation command creation process

	Overview:	ProgramJOG.
        	The main body does not perform anti-crash control in the programJOG
			Even if there is a request to stop operation, the entire operation pattern will be executed.
			ProgramJOGCommand complete(DEN)Wait time after300ms
			The number of repetitions is the programJOGofPJogP->TimesNot, butLEVELAccording to the settings
			Managed independently by advanced auto tuning.
****************************************************************************/
static SHORT	AdatPrgJogDrive( ADATV *AdatV, ADATP *AdatP )
{
	PJOGV	*PJogV;
	SHORT	rc;

	PJogV = &AdatP->BaseCtrlData->PJogHdl.PJogV;
	rc    = AdatV->PrgJogStat; 					/* Initialize return code to previous status 	*/

	if( (PJogV->Var.State == PJOG_ABORT) || (PJogV->Var.State == PJOG_ERROR)
	  || (AdatV->PrgJogPattern == ADAT_PJOG_STOP) )
	{	/* ProgramJOGAbort operation */
		AdatV->PrgJogPtr = 0;					/* Operation stop. Set the sequence to the initial settings	*/
		rc = ADAT_PJOG_NOP;						/* Go to operation stop state							*/
	}

	switch ( AdatV->PrgJogPtr )
	{
		/*----------------------------------------------------------------------------------*/
		/*	Initial process of Jog sequence 												*/
		/*----------------------------------------------------------------------------------*/
		case 0:
			AdatPJogInit( AdatV, AdatP );
			AdatV->PrgJogPtr = 1;
			AdatV->PrgJogPattern = AdatV->PrgJogPatternReq;
			break;

		case 1:
			AdatV->PrgJogActPattern = (AdatV->PrgJogPattern & 0x0000000F);

			if( AdatV->PrgJogActPattern == ADAT_PJOG_STOP )
			{	/* Request for operation stop	*/
				rc = ADAT_PJOG_NOP;							/* Go to operation stop state				*/
			}
			else
			{
				AdatV->PrgJogPatternCnt = 0;
				AdatPJogDrvSet( AdatV, AdatP, AdatV->PrgJogActPattern );/* Driving pattern setting	*/
				AdatV->PrgJogPtr = 2;						/* Update sequence pointer		*/
				rc = ADAT_PJOG_EXEC;
			}
			break;

		/*----------------------------------------------------------------------------------*/
		/*	Operation start processing																	*/
		/*----------------------------------------------------------------------------------*/
		case 2:
			BpiRsetCoinOffTime( AdatP->SettlingTime );
			BpiRsetMaxOverShoot( AdatP->SettlingTime );	/* Clear maximum overshoot amount		*/

			if( AdatV->f.GainTuneRun == TRUE )
			{	/* During tuning, clear once */
				DetVibResetMaxMinSpeedError( AdatP->DetVib );
			}

			PJogV->Var.RoutCmd = PJOGCMD_START;
			KlibRstLongTimer( &AdatV->PrgJogMovStartCntr );
			AdatV->PrgJogPtr = 3;							/* Waiting for movement completion				*/
			rc = ADAT_PJOG_EXEC;
			break;

		/*----------------------------------------------------------------------------------*/
		/*	Wait for movement command to complete																*/
		/*----------------------------------------------------------------------------------*/
		case 3:
			if( (PJogV->Var.RoutCmd == PJOGCMD_START) && (PJogV->Var.State == PJOG_START) )
			{
				PJogV->Var.RoutCmd = PJOGCMD_NONE;
			}

			if( KlibGetLongTimerMs( AdatV->PrgJogMovStartCntr ) > KPI_SCCYCLEMS * 2 )
			{	/* ScanCinPJOGCMD_START, setScanCis definitely1times 	*/
				if( (PJogV->Var.State == PJOG_ABORT) || (PJogV->Var.State == PJOG_END)
				 || (PJogV->Var.State == PJOG_ERROR) )
				{
					PJogV->Var.RoutCmd = PJOGCMD_NONE;
					KlibRstLongTimer( &AdatV->PrgJogWaitCntr );
					KlibRstLongTimer( &AdatV->CoinWaitCntr );
					AdatV->CoinWaitStat = ADAT_STS_COINWAIT;
					AdatV->PrgJogPtr = 4;					/* Waiting for operation completion				*/
				}
			}
			break;

		/*----------------------------------------------------------------------------------*/
		/*	Waiting for operation completion																	*/
		/*----------------------------------------------------------------------------------*/
		case 4:
			if( AdatV->f.CoinIgnore == TRUE )
			{
				if( PJogV->Cnst.RevDir == FALSE)
				{
					rc = ADAT_PJOG_PLUSEND;
				}
				else
				{
					rc = ADAT_PJOG_MINUSEND;
				}
				AdatV->PrgJogPtr = 5;						/* To wait					*/
				KlibRstLongTimer( &AdatV->PrgJogWaitCntr );
				AdatV->CoinWaitStat = ADAT_STS_COINNOWAIT;
			}
			else if( AdatP->BaseCtrlData->PosCtrlSts.CoinSignal == TRUE ) /* COINLatch completion */
			{
				if( PJogV->Cnst.RevDir == FALSE)
				{
					rc = ADAT_PJOG_PLUSEND;
				}
				else
				{
					rc = ADAT_PJOG_MINUSEND;
				}
				AdatV->PrgJogPtr = 5;					 	/* To wait					*/
				KlibRstLongTimer( &AdatV->PrgJogWaitCntr );
				AdatV->CoinWaitStat = ADAT_STS_COINNOWAIT;
			}
			else if( KlibGetLongTimerMs( AdatV->CoinWaitCntr ) >= ADATCOINWAIT1 )
			{	/* Approx.10When positioning is not completed in secondsCOINWaiting error	*/
				AdatV->ErrState = ADAT_COINERR;
				AdatV->CoinWaitStat = ADAT_STS_COINERR;		/* Error outputON					*/
			}
			else if( KlibGetLongTimerMs( AdatV->CoinWaitCntr ) >= ADATCOINWAIT2 )
			{	/* COIN Output waiting warning	*/
				AdatV->CoinWaitStat = ADAT_STS_COINWARN;	/* Warning output						*/
			}
			break;

		/*----------------------------------------------------------------------------------*/
		/*	Wait																		*/
		/*----------------------------------------------------------------------------------*/
		case 5:
			if( KlibGetLongTimerMs( AdatV->PrgJogWaitCntr )
											>=	(ULONG)((PJogV->Cnst.WaitTime + AATPJOGWAITTM)) )
			{	/* Operation completed		*/
				AdatV->PrgJogPtr = 6;					/* Prepare for next operation					*/
			}
			break;

		/*----------------------------------------------------------------------------------*/
		/*	Prepare for next operation																	*/
		/*----------------------------------------------------------------------------------*/
		case 6:
			AdatV->PrgJogPatternCnt++;
			AdatV->PrgJogActPattern = ((AdatV->PrgJogPattern
											>> (4 * AdatV->PrgJogPatternCnt)) & 0x0000000F);

			if( AdatV->PrgJogActPattern == 0x0F)
			{	/* Repeat operation	*/
				/* Repetitive processing	*/
				if( ++AdatV->PrgJogDrvCnt >= AdatV->PrgJogTimes )
				{	/* The number of repetitions executed	*/
					AdatV->PrgJogDrvCnt = 0;				/* Repeat counter initial setting		*/
					AdatV->PrgJogPatternCnt++;
				}
				else
				{	/* Less than the number of repetitions		*/
					AdatV->PrgJogPatternCnt--;
				}
				AdatV->PrgJogActPattern = ((AdatV->PrgJogPattern
											>> (4 * AdatV->PrgJogPatternCnt)) & 0x0000000F);
			}

			/* Sequence setting	*/
			if( AdatV->PrgJogActPattern == 0x00 )
			{	/* Operation completed	*/
				AdatV->PrgJogPtr = 7;			/* Operation stop. Set the sequence to the initial settings 	*/
				KlibRstLongTimer( &AdatV->PrgJogWaitCntr );
				rc = ADAT_PJOG_FIN;				/* Operation completion status set					*/
			}
			else
			{	/* Continue operation	*/
				AdatPJogDrvSet( AdatV, AdatP, AdatV->PrgJogActPattern );/* Driving pattern setting	*/
				AdatV->PrgJogPtr = 2;							/* Update sequence pointer	*/
				rc = ADAT_PJOG_EXEC;
			}
			break;

		case 7:
			if( KlibGetLongTimerMs( AdatV->PrgJogWaitCntr )
										>= (ULONG)((PJogV->Cnst.WaitTime + AATPJOGWAITTM) / 2 ))
			{	/* Operation completed	*/
				AdatV->PrgJogPtr = 0;						/* Prepare for next operation				*/
			}
			break;

		default:
			AdatV->ErrState = ADAT_PROGERR;					/* Abnormal termination						*/
			break;
	}

	return rc;
}



/****************************************************************************
	Advanced autotuning ProgramJOGInitialization process
****************************************************************************/
static void	AdatPJogInit( ADATV	*AdatV, ADATP *AdatP )
{
	PJOGHNDL *PJogHdl;
	FUN_CMN_CONTROL *FnCmnCtrl;

	PJogHdl = &AdatP->BaseCtrlData->PJogHdl;
	FnCmnCtrl = AdatP->FnCmnCtrl;

	PJogHdl->PJogV.Var.RoutCmd = PJOGCMD_INIT;
	PrgJogReset( PJogHdl );

	/* Number of movements */
	PJogHdl->PJogV.Cnst.MTimes = 1;		/* Number of movements is fixed to one			*/
	AdatV->PrgJogTimes  = 3;			/* Program repeat count setting	*/
	AdatV->PrgJogDrvCnt = 0;			/* Repeat counter initial setting		*/
	FnCmnCtrl->FnCtrlMcmd = CTRLMODE_PJOG;
}

/****************************************************************************
	Advanced autotuning ProgramJOGParameter setting processing

	Overview:	Program according to driving patternJOGRelated
			Set the travel distance, speed, acceleration, waiting time, and number of repetitions.
****************************************************************************/
static void	AdatPJogDrvSet( ADATV *AdatV, ADATP *AdatP, ULONG pattern )
{
	LONG	sx;
	LONG	kx;
	LONG	jratex;
	LONG	wk_spd;
	LONG	trq_per;						/* Rated100%Torque						*/
	CHAR	pjog_switch;					/* Acceleration selection switch						*/
	LONG	baseVel_wk;						/* Reference speed[min-1] DDMotor[0.1min-1]	*/
	LONG	VelData;
	LONG	AccData;
	LONG	AccGain;

	PJOGV	*PJogV;
	JATHNDL	*JatHdl;
	PRMDATA *Prm;
	BPRMDAT	*Bprm;
	REG_ACC_T AccessPrm;

	PJogV  = &AdatP->BaseCtrlData->PJogHdl.PJogV;
	JatHdl = &AdatP->BaseCtrlData->JatHdl;
	Prm    =  AdatP->UniPrms->Prm;
	Bprm   =  AdatP->UniPrms->Bprm;

	trq_per = TUNEACCTRQ2;
	jratex = Prm->jrate + 100;
	pjog_switch = 0;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	switch ( pattern )
	{
		case 0x03:		/* Normal feed forward rotation					*/
		case 0x04:		/* Normal feed reverse rotation					*/
			/* Stroke setting	[Command unit]				*/
			/* [1000Command unit]→[Command unit]Convert=125x		*/
			PJogV->Cnst.Distance = MlibAbs32( AdatV->MoveDistance ) / 8;

			/* Speed setting			[min-1]					*/
			/* ProgramJOGMultiplier for speed=1x		*/
			wk_spd = 1;

			/* Waiting time setting		[ms]					*/
			/* ProgramJOGStart waiting time=300ms+300ms	*/
			PJogV->Cnst.WaitTime = AATPJOGWAITTM;
			AdatV->f.CoinIgnore = FALSE;
			break;

		case 0x06:		/* Inertia identification fast-forward forward rotation		*/
		case 0x07:		/* Inertia identification fast-forward reverse		*/
			/* Stroke setting	[Command unit]				*/
			/* [1000Command unit]→[Command unit]Convert=1000x 	*/
			if( MlibAbs32( AdatV->MoveDistance ) > AdatV->DefaultDistance )
			{
				PJogV->Cnst.Distance = AdatV->DefaultDistance;
			}
			else
			{
				PJogV->Cnst.Distance = MlibAbs32( AdatV->MoveDistance );
			}

			/* Speed setting	[min-1]							*/
			/* ProgramJOGMultiplier for speed=2x		*/
			wk_spd = 2;
			/* Inertia ratio, torque ratio setting */
			if( AdatV->JratDrvCnt != 0 )
			{
				/* Since the load is known to some extent during identification, the acceleration torque is determined according to the identified value.	*/
				trq_per = JRATACCTRQ2;							/* Rated torque100%Acceleration			*/
				if( AdatV->Jrate > pndef_jrate.UpperLimit )
				{
					jratex = pndef_jrate.UpperLimit + 100;
				}
				else
				{

					jratex = JatHdl->EstimatJrat + 100;			/* Inertia identification value +100%		*/
				}
			}
			else
			{
				/* Inertia identification first operation command							*/
				/* At the first time of identification, it is unknown what kind of load is attached, so	*/
				/* "With equivalent inertia8%Acceleration torque				*/
				trq_per = JRATACCTRQ;
				jratex = (USHORT)Prm->Pn324 + 100;
			}
			pjog_switch = 1;		/* Acceleration setting selection */
			/* Waiting time setting[ms] 									*/
			/* ProgramJOGStart waiting time=0ms, Sequence transition amount subtraction 	*/
			PJogV->Cnst.WaitTime = 0;
			AdatV->f.CoinIgnore = TRUE;
			break;

		case 0x01:		/* Fast forward forward					*/
		case 0x02:		/* Fast forward reverse					*/
		default:		/* Default is fast forward forward rotation		*/
			/* Stroke setting	[Command unit]				*/
			/* [1000Command unit]→[Command unit]Convert=1000x	*/
			PJogV->Cnst.Distance = MlibAbs32( AdatV->MoveDistance );

			/* Speed setting			[min-1]					*/
			/* ProgramJOGMultiplier for speed=2x		*/
			wk_spd = 2;
			/* Waiting time setting		[ms]					*/
			/* ProgramJOGStart waiting time=300ms+300ms	*/
			PJogV->Cnst.WaitTime = AATPJOGWAITTM;
			AdatV->f.CoinIgnore = FALSE;
			break;
	}

	/* Driving direction setting				*/
	switch ( pattern )
	{
		case 0x02:		/* Fast forward reverse	*/
		case 0x04:		/* Normal feed reverse rotation	*/
			PJogV->Cnst.MTimes = 1;
			if( AdatV->MoveDistance >= 0 )
			{
				/* Operation settings				*/
				PJogV->Cnst.Pattern = TRUE;
				/* Initial setting of operation direction		*/
				PJogV->Cnst.RevDir = TRUE;
			}
			else
			{
				/* Operation settings				*/
				PJogV->Cnst.Pattern = FALSE;
				/* Initial setting of operation direction		*/
				PJogV->Cnst.RevDir = FALSE;
			}
			break;
		case 0x06:		/* Inertia identification fast-forward forward rotation	*/
		case 0x07:		/* Inertia identification fast-forward reverse	*/
			PJogV->Cnst.MTimes = 4;			/* Number of movements is2Fixed to a round trip. */
			if( AdatV->MoveDistance >= 0 )
			{
				/* Operation settings				*/
				PJogV->Cnst.Pattern = 4;
				/* Initial setting of operation direction		*/
				PJogV->Cnst.RevDir = TRUE;
			}
			else
			{
				/* Operation settings				*/
				PJogV->Cnst.Pattern = 5;
				/* Initial setting of operation direction		*/
				PJogV->Cnst.RevDir = FALSE;
			}
			break;
		case 0x01:		/* Fast forward forward	*/
		case 0x03:		/* Normal feed forward rotation	*/
		default:
			PJogV->Cnst.MTimes = 1;
			if( AdatV->MoveDistance >= 0 )
			{
				/* Operation settings				*/
				PJogV->Cnst.Pattern = FALSE;
				/* Initial setting of operation direction		*/
				PJogV->Cnst.RevDir = FALSE;
			}
			else
			{
				/* Operation settings				*/
				PJogV->Cnst.Pattern = TRUE;
				/* Initial setting of operation direction		*/
				PJogV->Cnst.RevDir = TRUE;
			}
			break;
	}

	/* Reference speed		*/
	if( AdatP->MencV->LowSpdMotor == TRUE )
	{	/* DDMotor	*/
		baseVel_wk = Bprm->NorRatSpd * 10 / 3;	/* Reference speed=Rated speed÷3 */
										/* 10is multiplied byPJOGROTSPDunit is[0.1min-1]	*/
	}
	else if( AdatP->MencV->AxisMotType == MOTTYPE_LINEAR )
	{	/* Linear motor	*/
		baseVel_wk = MlibMIN( (Bprm->NorRatSpd / 3), (Bprm->NorMaxSpd / 2) );
											/* The reference speed is the rated speed÷3and maximum speed÷2	*/
	}
	else
	{	/* Rotary type motor	*/
		baseVel_wk = Bprm->NorRatSpd / 3;							/* The reference speed is the rated speed÷3	*/
	}

	/*------------------------------------------------------------------------------------------*/
	/*	Specify acceleration torque (speed is reference speed)														*/
	/*------------------------------------------------------------------------------------------*/
	/* Speed setting	[min-1] */
#if (FLOAT_USE==TRUE)
	VelData = baseVel_wk * wk_spd * Bprm->Kmonspd ;
#else
	VelData = MlibMulgainSx48( baseVel_wk * wk_spd, Bprm->Kmonspd );
#endif /* FLOAT_USE */
	if( AdatP->MencV->AxisMotType == MOTTYPE_LINEAR )				/* Linear motor 			*/
	{
		TunePrmWriteRam( &AccessPrm, &pndef_pjoglinspd, (LONG)VelData, pndef_pjoglinspd.DataLength );
	}
	else
	{
		TunePrmWriteRam( &AccessPrm, &pndef_pjogrotspd, (LONG)VelData, pndef_pjogrotspd.DataLength );
	}

	/* Acceleration setting [ms] */
	kx = MlibScalKxgain( jratex, Bprm->PerMaxTrq, 100, &sx, 0 );	/* Bprm.PerMaxTrq[%] /100	*/
	kx = MlibPcalKxkskx( kx, Bprm->OvrSpd, 0x1000000, &sx, 0 );
	kx = MlibPcalKxksks( kx, Bprm->Jmot, Bprm->MaxTrq, &sx, 0 );
	AccGain = MlibPcalKxgain( kx, 10000, trq_per, &sx, 24 );		/* 1/Reference acceleration	[ms/Nos]	*/
#if (FLOAT_USE==TRUE)
	AccData = PJogV->Cnst.MaxSpd * AccGain;
#else
	AccData = MlibMulgain( PJogV->Cnst.MaxSpd, AccGain );
#endif /* FLOAT_USE */

	if( AccData > pndef_pjogacctm.UpperLimit )
	{
		AccData = pndef_pjogacctm.UpperLimit;
	}
	else if( AccData < pndef_pjogacctm.LowerLimit )
	{
		AccData = pndef_pjogacctm.LowerLimit;
	}
	TunePrmWriteRam( &AccessPrm, &pndef_pjogacctm, (LONG)AccData, pndef_pjogacctm.DataLength );

	if( pjog_switch == 1 )
	{	/* Inertia identification	*/
		/* Reference travel distance calculation	[Command unit]	*/
		/*--------------------------------------------------------------------------------------*/
		/*	Distance traveled when accelerating at target torque and moving at maximum operating speed for constant reference speed time			*/
		/*	Calculate the moving distance as the reference for the operation command												*/
		/*																						*/
		/*	Reference travel distance [Command unit] = Maximum operating speed[Command unit/ms] * Reference constant speed time[ms]			*/
		/*																						*/
		/*									Maximum operating speed[Command unit/ms^2]							*/
		/*								+ -------------------------------						*/
		/*										Acceleration[Command unit/ms^2]							*/
		/*																						*/
		/*--------------------------------------------------------------------------------------*/
		kx = MlibScalKxgain( PJogV->Cnst.MaxSpd, 1000, Bprm->NorOvrSpd, &sx, 0 );
		kx = MlibPcalKxkskx( kx, Bprm->OvrPspd, 1, &sx, 24 );
#if (FLOAT_USE==TRUE)
		kx = (AATCONSVELTM * kx ) + ( Prm->pjogacctm * kx );
#else
		kx = MlibMulgain( AATCONSVELTM, kx ) + MlibMulgain( Prm->pjogacctm, kx );
#endif /* FLOAT_USE */

		/*	Operation speed recalculation[min-1] */
		/*--------------------------------------------------------------------------------------*/
		/*	Recalculate operating speed from reference travel distance and command stroke under constant acceleration conditions					*/
		/*																						*/
		/*											  Command stroke(L_ref)[Command unit]			*/
		/*	Corrected speed[min-1] = Maximum operating speed[min-1] * (-----------------------------------)^0.5	*/
		/*												Reference travel distance(L_base)[Command unit]			*/
		/*																						*/
		/*--------------------------------------------------------------------------------------*/
		kx = MlibScalKxgain( PJogV->Cnst.Distance, 1, kx, &sx, 24 );/* L_ref/L_base				*/
#if (FLOAT_USE==TRUE)
		kx = (VelData * VelData) * kx ;				/* = Vmax^2 * L_ref/L_base	*/
#else
		kx = MlibMulgain( (VelData * VelData), kx );				/* = Vmax^2 * L_ref/L_base	*/
#endif /* FLOAT_USE */
		kx = MlibSqrtu32( kx );										/* Corrected speed	[min-1]			*/

		/*--------------------------------------------------------------------------------------*/
		/*	Maximum, minimum clamp																	*/
		/*--------------------------------------------------------------------------------------*/
		if( kx > 2 * VelData )
		{
			kx = 2 * VelData;								/* Reference speed×2			*/
		}
		else if( kx < VelData / 2 )
		{
			kx = VelData / 2;								/* Reference speed÷2			*/
		}

		if( AdatP->MencV->AxisMotType == MOTTYPE_LINEAR )
		{
			TunePrmWriteRam( &AccessPrm, &pndef_pjoglinspd, (LONG)kx, pndef_pjoglinspd.DataLength );
		}
		else
		{
			TunePrmWriteRam( &AccessPrm, &pndef_pjogrotspd, (LONG)kx, pndef_pjogrotspd.DataLength );
		}

		/*	Acceleration time recalculation [ms] */
#if (FLOAT_USE==TRUE)
		AccData = PJogV->Cnst.MaxSpd * AccGain ;	/* Corrected acceleration time	[ms]			*/
#else
		AccData = MlibMulgain( PJogV->Cnst.MaxSpd, AccGain );	/* Corrected acceleration time	[ms]			*/
#endif /* FLOAT_USE */
		if( AccData > pndef_pjogacctm.UpperLimit )
		{
			AccData = pndef_pjogacctm.UpperLimit;
		}
		else if( AccData < JRATPJOGACC )
		{
			AccData = JRATPJOGACC;
		}
		else if( AccData < pndef_pjogacctm.LowerLimit )
		{
			AccData = pndef_pjogacctm.LowerLimit;
		}
		TunePrmWriteRam( &AccessPrm, &pndef_pjogacctm, AccData, pndef_pjogacctm.DataLength );
	}

	if( ( PJogV->Cnst.PrmUnMatch == TRUE ) || ( PJogV->Cnst.PrmUnMatch2 == TRUE ) )
	{
		ALMSetServoAlarm( AdatP->AlmManager, ALM_PRMUNMATCH );/* A.042 : Parameter combination error */
	}

	AdatV->f.DrvPatternSet = TRUE;
}

/****************************************************************************
	Advanced autotuning initialization process

	Overview:Initialize flags and related data when starting advanced auto-tuning.
****************************************************************************/
static PRM_RSLT AdatInitial( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP)
{
	ULONG	data;
	PRM_RSLT rc;

	PRMDATA *Prm;
	MFCTRL	*MFControl;
	REG_ACC_T AccessPrm;

	Prm       =  AdatP->UniPrms->Prm;
	MFControl = &AdatP->BaseCtrlData->MFControl;
	rc = PRM_RSLT_SUCCESS;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	AdatV->ModeSetPtr      = 0;
	AdatV->JratSetPtr      = 0;
	AdatV->VibDetPtr       = 0;
	AdatV->KvTunePtr       = 0;
	AdatV->KpTunePtr       = 0;
	AdatV->KpSetPtr        = 0;
	AdatV->MfcTunePtr      = 0;
	AdatV->NotchSetPtr     = 0;
	AdatV->PrgJogPtr       = 0;
	AdatV->PrgJogPatternReq= ADAT_PJOG_STOP;
	AdatV->PrgJogPattern   = ADAT_PJOG_STOP;
	AdatV->JratChangeTimes = 0;
	AdatV->JratDrvCnt      = 0;
	AdatV->VibMaxKv        = 0;
	AdatV->VibMaxKvAresOFF = 0;			/* AtypeONPrevious gain. Since it is compared with the current gain, the initial value is0	*/
	AdatV->MaxKv = Prm->adatgmax;		/* Set search maximum gain to maximum gain  					*/
	AdatV->MaxKp = AdatV->MaxKv;		/* SettingKpMaximum initial value is search maximumKv					*/
	/* Vibration detection level initialization */
	AdatP->DetVib->conf.VibCheckLevel = ADAT_GRAT_70( AdatP->DetVib->conf.VibCheckLevel );
	/* AdatV->VibSpeedis set by vibration level measurement, but initialize it just in case.	*/
	AdatV->VibSpeed = AdatP->DetVib->conf.VibCheckLevel;
	AdatV->JratErr  = ADAT_JRAT_OK;
	AdatV->f.CoinWaitStart = FALSE;		/* COINOutput waiting process reset								*/
	AdatV->f.Used2MassMfc  = FALSE;		/* MFC(Rigid body+Machine base model)Initialize set flag			*/
	AdatV->f.Mfc2MassOK    = FALSE;		/* MFC(Rigid body+Machine base model)Initialize last adjusted flag		*/
	AdatV->f.MfcLastSet    = FALSE;		/* MFC(Rigid body+Machine base model)Adjustment completed							*/
	AdatV->Set2MassMfcSequence = 0;		/* MFC(Rigid body+Machine base model)Initializing the sequence at setting			*/
	AdatV->f.StopDetectMode2   = FALSE;	/* For stop judgmentCOIN,MotStop					*/
	AdatV->frq_buf = Prm->mdlantfrq;	/* Resonance and antiresonance frequency buffer							*/
	AdatV->Kpm_buf = Prm->mdlgn; 		/* KpmBuffer											*/
	AdatV->Kv_buf  = Prm->loophz;		/* KvBuffer											*/

	/* AlreadyMFC(Rigid body+Machine base model)When setMFC(Rigid body+Machine base model)do not adjust	*/
	if(( MFControl->conf.MFCModelSetting > ADATMFC_ONE_MASS )
			|| (((Prm->mdlsw >> 8) & 0x000F) != 0x0001))
	{
		AdatV->f.Used2MassOld = TRUE;
	}
	else
	{
		AdatV->f.Used2MassOld = FALSE;
	}

	AdatP->FnCmnCtrl->FnSensOnReq = TRUE;
	AdatReg->AdatState = ADAT_STS_INIT;	/* Status initialization	*/
	AdatV->MinDistance = AdatPcalMinDistance( AdatP,
											  AdatP->MencV->AxisMotType,
	                                          AdatP->MencV->LowSpdMotor );
	/* Bottom3Added processing to truncate digits */
	AdatV->DefaultDistance = AdatPcalDefautDistance( AdatP,
													 AdatP->MencV->AxisMotType,
	                                                 AdatP->MencV->LowSpdMotor ) / 1000 * 1000 ;

	if( (1000*MlibAbs32( AdatReg->AdatMovep.Long )) < AdatV->MinDistance )
	{
		if(AdatReg->AdatMovep.Long < 0)
		{
			AdatV->MoveDistance = -AdatV->DefaultDistance;
		}
		else if(AdatReg->AdatMovep.Long >= 0)
		{
			AdatV->MoveDistance = AdatV->DefaultDistance;
		}
		AdatReg->AdatMovep.Long = AdatV->MoveDistance / 1000;
	}
	else
	{
		AdatV->MoveDistance = 1000 * AdatReg->AdatMovep.Long;
	}

	AdatScanCRequest( ADAT_NO_OPE, AdatV );

	/* Determining whether automatic notch setting is possible	*/
	if( ((~Prm->tfuncsw) & Prm->anotchsw & 0x0F0F) == 0x0000 )
	{
		AdatV->f.UsedAllNtFil = TRUE;
	}
	else
	{
		AdatV->f.UsedAllNtFil = FALSE;
	}
	/* AutomaticADetermine whether mold setting is possible		*/
	if( ( ((Prm->avibsw >> 4) & 0x0F)  == FALSE) 	/* Automatic adjustment disabled						*/
		|| ((Prm->avibsw & 0x0F) == TRUE) )			/* orAMold already valid				*/
	{
		AdatV->f.UsedAResCtrl = TRUE;				/* Already set 						*/
	}
	else
	{
		AdatV->f.UsedAResCtrl = FALSE; 				/* Not set 							*/
	}

	AdatV->ModeSetPtr = AdatV->ModeSetMin;

	/* Oscillation detection	*/
//	data = 0x0000FFF0 & Prm->vibsens;//<S09E>
	data = 0x0000FFF0 & Prm->vibsel;
	TunePrmWriteRam( &AccessPrm, &pndef_vibsel, (LONG)data, pndef_vibsel.DataLength );

	/* Torque high-pass filter frequency when stopping	*/
	AdatV->KTrqHpFil = MlibPcalKf1gain( STPTRQFIL, (KPI_SCCYCLEMS*1000), 1 );

	if(MFControl->conf.MFCModelSetting <= ADATMFC_ONE_MASS)
	{	/* MFCDisabled or Rigid bodyMFC	*/
		/* Model following control disabled	*/
		data = 0x0000FFF0 & Prm->mdlsw;
		TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, (LONG)data, pndef_mdlsw.DataLength );
		/* Disturbance observer disabled	*/
		data = 0x00000FFF & Prm->tfuncsw;
		TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)data, pndef_tfuncsw.DataLength );
	}
	/* HPFand remove low frequency vibration */
	DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, VIBOBS_KS,
	                         	 DETVIBLP, 0, AdatP->UniPrms->Bprm->SvCycleNs );
	/*----------------------------------------------------------------------------------*/
	/*	Initialization of residual vibration frequency detection sensitivity													*/
	/*----------------------------------------------------------------------------------*/
	data = Prm->remdetw * ADAT_LFVIBDET_LVL / 100;
	if( (LONG)data > pndef_remdetw.UpperLimit )
	{
		data = pndef_remdetw.UpperLimit;
	}
	TunePrmWriteRam( &AccessPrm, &pndef_remdetw, (LONG)data, pndef_remdetw.DataLength );

	/*----------------------------------------------------------------------------------*/
	/*	Initialize gain																	*/
	/*----------------------------------------------------------------------------------*/
	TuneFilGainCalbyFilType( AdatP->ANotch, (UCHAR)AdatReg->AdatFilType );
	AdatV->Jrate = Prm->jrate;
	AdatV->f.OrgIPSpdControl = (AdatP->BaseCtrlData->CtrlCmdPrm.IPSpdControl)? TRUE : FALSE;
	return rc;
}

/****************************************************************************
	Minimum travel distance [1Command unit]
****************************************************************************/
static LONG	AdatPcalMinDistance( ADATP *AdatP, LONG AxisMotType, BOOL LowSpdMotor )
{
	BPRMDAT *Bprm;
	POS_MNG_HNDL *PosManager;
	LONG sx;
	LONG kx;
	LONG lwork;

	Bprm = AdatP->UniPrms->Bprm;
	PosManager = &AdatP->BaseCtrlData->PosManager;

	lwork = (LowSpdMotor)? (2*10) : (2*1);

	if( AxisMotType == MOTTYPE_LINEAR )
	{
		kx = MlibScalKxgain( Bprm->XfbPulse1, MINIMUM_LINEAR_MM, 1, &sx, 0 );
	}
	else
	{
		kx = MlibScalKxgain( Bprm->XfbPulse1, 1, lwork, &sx, 0 );
	}
	kx = MlibPcalKxgain( kx, PosManager->conf.Egear.a, PosManager->conf.Egear.b, &sx, -30 );
	return ( kx );

}

/****************************************************************************
	Initial movement distance calculation [1Command unit]
****************************************************************************/
LONG AdatPcalDefautDistance( ADATP *AdatP, LONG AxisMotType, BOOL LowSpdMotor ) /* <S145> */
{
	BPRMDAT *Bprm;
	POS_MNG_HNDL *PosManager;
	LONG	sx;
	LONG	kx;
	LONG	lwork;

	Bprm = AdatP->UniPrms->Bprm;
	PosManager = &AdatP->BaseCtrlData->PosManager;

	lwork = (LowSpdMotor)? (10*10) : (10*1);


	if( AxisMotType == MOTTYPE_LINEAR )
	{
		kx = MlibScalKxgain( Bprm->XfbPulse1, DEFAULT_STROKE_LINEAR, 10, &sx, 0 );
	}
	else
	{
		kx = MlibScalKxgain( Bprm->XfbPulse1, DEFAULT_STROKE_ROTARY, lwork, &sx, 0 );
	}
	kx = MlibPcalKxgain( kx, PosManager->conf.Egear.a, PosManager->conf.Egear.b, &sx, -30 );
	if( kx > ADAT_DIST_MAX )
	{
		kx = ADAT_DIST_MAX;
	}
	if( kx < -ADAT_DIST_MAX )
	{
		kx = -ADAT_DIST_MAX;
	}
	return ( kx );
}

/****************************************************************************
	Advanced autotuning ScanCSearch request(Round)

	Overview:	ScanCto
			Round.
****************************************************************************/
static void AdatScanCRequest( USHORT Request, ADATV *AdatV )
{
	AdatV->ScanCReqCmd = Request;/* Tentatively set */
	AdatV->ScanCStat = ADAT_SEAR_CONTINUE;
	AdatV->ScanCReqFlg = TRUE;
}


/****************************************************************************
	Advanced autotuning ScanCSearch reception(ScanC)

	Overview:	ScanCtuning request commands and flags
			ScanC.
****************************************************************************/
static void AdatScanCSetReq( ADATV *AdatV )
{
	if(	AdatV->ScanCReqFlg == TRUE)
	{
		AdatV->ScanCRequest = AdatV->ScanCReqCmd;	/* ScanC Processing request set		*/
		AdatV->ScanCStat    = ADAT_NO_OPE;			/* Error status clear	*/
		AdatV->ScanCReqFlg = FALSE;
	}
}

/****************************************************************************
	Speed loop gain change process

	Overview:	Change each gain of the velocity loop as required.
****************************************************************************/
static SHORT	TuneAdatSetSpdCtrlGain( LONG mode, ADATP *AdatP)
{

	PRMDATA   *Prm;
	REG_ACC_T AccessPrm;
	USHORT	kvdata;
	USHORT	data;
	SHORT	rc;
	SHORT	NotchState;

	rc  = ADAT_OK;
	Prm = AdatP->UniPrms->Prm;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	switch ( mode )
	{
		case GAIN_UP:
			kvdata = Prm->loophz + KVUP_UNIT;		/* 2.0[Hz] Increase					*/
			break;

		case GAIN_DW:
			kvdata = ADAT_GRAT_94( Prm->loophz );	/* 15/16 = 0.9375 6.25%Decrement		*/
			break;

		case GAIN_CAL:
		default:
			kvdata = Prm->loophz;					/* As is					*/
			break;
	}

	if( kvdata < KV_UNLMT )
	{
		kvdata = KV_UNLMT;							/* If less than the lower limit, use the lower limit		*/
		rc = ADAT_NG;								/* Abnormal termination				*/
	}
	else if( kvdata > Prm->adatgmax )
	{
		kvdata = Prm->adatgmax;						/* If the upper limit value is exceeded, use the upper limit value		*/
	}

	/*------------------------------------------------------------------------------*/
	/*	Speed loop gain(Kv)														*/
	/*------------------------------------------------------------------------------*/
	TunePrmWriteRam( &AccessPrm, &pndef_loophz, (LONG)kvdata, pndef_loophz.DataLength );
	/*------------------------------------------------------------------------------*/
	/*	Speed loop integral time constant(Ti)													*/
	/*------------------------------------------------------------------------------*/
	data = TuneKvFuncTi( kvdata, AdatP->BaseCtrlData->CtrlCmdPrm.IPSpdControl );
	TunePrmWriteRam( &AccessPrm, &pndef_pitime, (LONG)data, pndef_pitime.DataLength );
	/*------------------------------------------------------------------------------*/
	/*	Disturbance observer related parameters												*/
	/*------------------------------------------------------------------------------*/
	if( AdatP->BaseCtrlData->DobsCtrl.var.DobsAct == TRUE )
	{ /* Disturbance observer enabled */
		data = TuneKvFuncDobsgn( kvdata );
		TunePrmWriteRam( &AccessPrm, &pndef_dobgn, (LONG)data, pndef_dobgn.DataLength );
	}
	/*------------------------------------------------------------------------------*/
	/*	Notch filter related & Tf Parameter											*/
	/*------------------------------------------------------------------------------*/
	NotchState = TuneNotchQ( AdatP->ANotch, AdatP->RegManager->hPnReg,
	                         	 	 	 	 	 	 kvdata, Prm->jrate, 1, Prm );
	if( (NotchState < 0) && (NotchState != -3) )
	{	/* Notch filter setting error (QThe value has reached the upper limit)*/
		rc = ADAT_NTCHNG;		/* Notch calculation error		*/
	}
	return rc;
}

/****************************************************************************
		Advanced autotuning inertia setting process

	Overview:	The inertia identification process itself uses the "auto-tuning function."
			Here, in order to execute the "online auto-tuning function"
			Set the necessary data.
			The inertia ratio is the average value of the identified value during forward rotation and the identified value during reverse rotation.
			However, the difference between the identification values for forward rotation and reverse rotation31.25%, it is considered an error.
****************************************************************************/
static PRM_RSLT	AdatJratSet( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status )
{
	PRMDATA *Prm;
	REG_ACC_T AccessPrm;
	PRM_RSLT rc;
	ULONG data;
	BOOL TrqClamp;
	TUNELESS_CFGPRM TuneLessCfgPrm;

	rc  = PRM_RSLT_SUCCESS;
	Prm = AdatP->UniPrms->Prm;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	TrqClamp = (AdatP->SvControlIf->RspCtrl.RspCtrlBit >> TRQ_CLAMP_BITNO & 1);
	if( AdatV->f.RefInType == FALSE)
	{	/* JLimit check during identification is an internal commandAATonly */
		if( TrqClamp == TRUE )
		{
			AdatV->JratSetPtr = 0;
			AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;
			AdatV->ErrState = ADAT_J_ERR;
			AdatV->JratErr  = ADAT_JRAT_ERR4;
		}
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Forcible invalidation processing without adjustment																		*/
	/*		If inertia identification is used, disable adjustment-lessAAT									*/
	/*----------------------------------------------------------------------------------------------*/
	if( AdatP->BaseCtrlData->TuneLessCtrl.conf.TuningLessUse == TRUE )
	{	/* Adjustmentless enabled	*/
		if(AdatReg->AdatMode == FALSE)								/* With inertia identification			*/
		{
			/* RAMParameter function switch disabled */
			data = 0x0000FFF0 & Prm->DatLevel;
			TunePrmWriteRam( &AccessPrm, &pndef_DatLevel, (LONG)data, pndef_DatLevel.DataLength );

			/* Adjustment-less function selection switch */
			TuneLessCfgPrm.gnmode	= Prm->gnmode;
			TuneLessCfgPrm.mfctype	= Prm->mfctype;
			TuneLessCfgPrm.avibsw	= Prm->avibsw;
			TuneLessCfgPrm.DatLevel	= Prm->DatLevel;
			TuneLessCfgPrm.tfuncsw 	= Prm->tfuncsw;
			TuneLessCfgPrm.flg_wf 	= Prm->MencP.flg_wf;
			TuneLessCalculateInitPrm(AdatP->BaseCtrlData,
			                         AdatP->UniPrms->Bprm,
			                         AdatP->MencV,
									 AdatP->SvAsicRegs,
			                         &TuneLessCfgPrm);

			/* Vibration detection differential speed maximum/minimum value reset */
			DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, VIBOBSGN, 0,
			                         	 DETVIBHP, AdatP->UniPrms->Bprm->SvCycleNs);

#if 0 /* 2012.08.31 Y.Oka ★Motor speed detection moving average Currently not supported★ */
			/* Reset moving average time constant for motor speed detection to default		*/
			BpiChangeSpdDetMaFil( AdatP->MotSpdMafil, -1,
						(USHORT*)&(AdatP->SvAsicRegs->AsicMicroReg->MREG_SPDMANUMBITI) );
#endif
		}
		else
		{
			AdatV->ErrState = ADAT_MOSESETERR;						/* Mode setting error				*/
		}
	}

	switch ( FunExe->CmdCode )
	{
		case FCMD_SV :
			if( AdatV->f.RefInType == FALSE )
			{
				if( AdatP->AlmManager->Status.AlmFlag || AdatP->SeqCtrlOut->HwbbSts ||
					!AdatP->SeqCtrlOut->MainPowerOn || AdatP->SeqCtrlOut->OverTrvlSts )
				{
					rc = PRM_RSLT_CONDITION_ERR;
				}
				else
				{
					AdatP->FnCmnCtrl->FnSvonReq = ~AdatP->FnCmnCtrl->FnSvonReq;
				}
			}
			return rc;

		default :
			break;
	}

	switch ( AdatV->JratSetPtr )
	{
		/*--------------------------------------------------------------------------------------*/
		/*	Wait for movement direction input																	*/
		/*--------------------------------------------------------------------------------------*/
		case 0:
			AdatReg->AdatState = ADAT_STS_DRVWAIT;	/* Initial setting completed Input wait */
			AdatP->FnCmnCtrl->State.TuningRun = FALSE;

			/* Mode switch disabled */
			data  = Prm->gnmode;
			data &= 0x0000FFF0;
			data |= 0x00000004;
			TunePrmWriteRam( &AccessPrm, &pndef_gnmode, (LONG)data, pndef_gnmode.DataLength );

			/*----------------------------------------------------------------------------------*/
			/*	A-type vibration damping force disabled																*/
			/*----------------------------------------------------------------------------------*/
			if( (AdatV->f.RefInType == FALSE) && (AdatReg->AdatMode == FALSE) )	/* With inertia identification	*/
			{
				/*A-type vibration damping disabled */
				data = 0x0000FFF0 & Prm->avibsw;
				TunePrmWriteRam( &AccessPrm, &pndef_avibsw, (LONG)data, pndef_avibsw.DataLength );
				AdatV->f.UsedAResCtrl = TRUE;	/* A type vibration damping automatic setting function has been set.OFF	*/
			}

			switch ( FunExe->CmdCode )
			{
				case FCMD_UP :
					/* Servo on check & Check movement direction */
					if( ((AdatV->f.RefInType == FALSE) && (AdatP->SeqCtrlOut->BaseEnableReq == TRUE) && (AdatV->MoveDistance > 0)) )
					{
						AdatV->f.DrvErrDetect = TRUE;
						AdatP->FnCmnCtrl->State.TuningRun = TRUE;
						AdatV->JratSetPtr = 1;	/* When the servo is on and the movement direction and key input are the same	*/
												/* Create the operation status when starting tuning					*/
					}
					else
					{
						rc = PRM_RSLT_CONDITION_ERR;
					}
					break;

				case FCMD_DOWN :
					if( ((AdatV->f.RefInType == FALSE)
					   && (AdatP->SeqCtrlOut->BaseEnableReq == TRUE)
					   && (AdatV->MoveDistance < 0)) )
					{
						AdatV->f.DrvErrDetect = TRUE;
						AdatP->FnCmnCtrl->State.TuningRun = TRUE;
						AdatV->JratSetPtr = 1;
					}
					else
					{
						rc = PRM_RSLT_CONDITION_ERR;
					}
					break;

				case FCMD_EXEC :
					if( (AdatV->f.RefInType == TRUE) && (AdatRefInExeModeExeCheck(AdatP)) )
					{
						AdatV->f.DrvErrDetect = TRUE;
						AdatP->FnCmnCtrl->State.TuningRun = TRUE;
						AdatV->JratSetPtr = 1;
					}
					else
					{
						rc = PRM_RSLT_CONDITION_ERR;
					}
					break;

				case FCMD_NONE :
					break;

				default:
					rc = PRM_RSLT_CONDITION_ERR;
					break;
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Start operation																			*/
		/*--------------------------------------------------------------------------------------*/
		case 1:
			TunePrmWriteRam( &AccessPrm, &pndef_ffgn, JRATE_VFFGN, pndef_ffgn.DataLength );
			AdatP->BaseCtrlData->CtrlCmdPrm.IPSpdControl = FALSE;
			BpiPcmdFilSwitch( &AdatP->BaseCtrlData->PosManager, FALSE );  	/* Stop position command filter   */
			MfcSetMfcCtrlSwitch( &AdatP->BaseCtrlData->MFControl, FALSE );	/* MFCForced invalidation 		 	  */
			AdatP->BaseCtrlData->DobsCtrl.var.DobsAct = 0;					/* Disturbance observer forcedly disabled */
			AdatV->LastEstimatJrat = -1;									/* Initialize the previous identified value	  */

			if( (AdatV->f.RefInType == FALSE) && (AdatReg->AdatMode == FALSE) )
			{	/* Inertia identification enabled 	*/
				AdatReg->AdatState = ADAT_STS_JRUN;				/* Inertia identification in progress			*/
				AdatV->f.DrvPatternSet = FALSE;
				AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN5;	/* ProgramJOGOperation pattern5	*/
				AdatV->JratSetPtr = 2;							/* Go to processing during identification.					*/
				AdatV->Jrate = Prm->Pn324;
				/* Pn103 */
				TunePrmWriteRam( &AccessPrm, &pndef_jrate, AdatV->Jrate, pndef_jrate.DataLength );
			}
			else
			{	/* Inertia identification disabled	*/
				CpiEndJatOffLine( &AdatP->BaseCtrlData->JatHdl, &AccessPrm );
				AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;
				AdatV->JratSetPtr = 0;							/* Go to initial setting state				*/
				AdatUndoRamGainforJrat( AdatV, AdatP );
				FunExe->State = ADAT_SEQ_NORMVIB;				/* Go to normal vibration level search			*/
			}
			break;
		/*--------------------------------------------------------------------------------------*/
		/*	Waiting for operation to start																		*/
		/*--------------------------------------------------------------------------------------*/
		case 2:
			AdatScanCRequest( ADAT_SEQ_SEARCHOPTKV, AdatV );			   /* ScanCProcessing request */
			if( AdatV->f.DrvPatternSet == TRUE )
			{
				CpiInitJatOffLine( &AdatP->BaseCtrlData->JatHdl,
									AdatP->UniPrms,
									AdatP->RegManager,
									AdatP->BaseCtrlData->GainChange,
									1,
									AdatP->BaseCtrlData->PJogHdl.PJogV.Cnst.Distance,
									AdatP->BaseCtrlData->CtrlModeSet.CtrlMode.b.cm,
									AdatP->UniPrms->Bprm->SvCycleNs);

				if( AdatV->PrgJogStat == ADAT_PJOG_EXEC )
				{	/* Start operation	*/
					AdatV->JratSetPtr = 3;
					AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;
				}
			}
			break;
		/*--------------------------------------------------------------------------------------*/
		/*	Inertia identification in progress																	*/
		/*--------------------------------------------------------------------------------------*/
		case 3:
		/*--------------------------------------------------------------------------------------*/
		/*	Identification error determination																		*/
		/*--------------------------------------------------------------------------------------*/
			if( CpiGetJatExeErrStatus( &AdatP->BaseCtrlData->JatHdl ) == NG )
			{	/* Execution error during inertia identification	*/
				AdatV->JratSetPtr = 0;
				AdatV->ErrState = ADAT_J_ERR;
				AdatV->JratErr = ADAT_JRAT_ERR5;
			}
			if( CpiGetJatErrStatus( &AdatP->BaseCtrlData->JatHdl ) == NG )
			{
				/*------------------------------------------------------------------------------*/
				/*	Inertia initial value correction														*/
				/*		When there is large inertiaKp/KvVibration countermeasures through balance. Identification Performed only for the first time			*/
				/*------------------------------------------------------------------------------*/
				if( ( KlibGetLongTimerMs( AdatV->JratVibWaitCntr ) >= ADATVIBWAITTM3 )	/* Waiting for vibration convergence100ms	*/
					 || (AdatV->JratChangeTimes == 0) )									/* No error			*/
				{
					if( (CpiGetJatExeErrStatus( &AdatP->BaseCtrlData->JatHdl ) != NG) && (AdatV->JratDrvCnt == 0 )
							&& (AdatV->JratChangeTimes < ADAT_JRAT_CHGCNT) )
					{
						AdatV->f.VibDetectinCycle = TRUE;	/* Vibration statusON */
						AdatV->Jrate = AdatV->Jrate + ADAT_JRAT_CHG_RATE;
						TunePrmWriteRam( &AccessPrm, &pndef_jrate, (LONG)AdatV->Jrate, pndef_jrate.DataLength );
						AdatV->JratChangeTimes++;
						KlibRstLongTimer( &AdatV->JratVibWaitCntr );
					}
					else
					{
						AdatV->JratSetPtr = 0;
						AdatV->ErrState = ADAT_J_ERR;
						AdatV->JratErr = ADAT_JRAT_ERR3;
					}
				}
			}

		/*--------------------------------------------------------------------------------------*/
		/*	Vibration discrimination automatic notch																	*/
		/*--------------------------------------------------------------------------------------*/
			if( (status == ADAT_SEAR_VIBEND) || (status == ADAT_SEAR_STOPVIBEND) )
			{	/* Vibration detection finished		*/
				TuneAdatSetSpdCtrlGain( GAIN_DW, AdatP );
				if( ( AdatV->f.UsedAllNtFil == FALSE ) || ( AdatV->f.UsedAResCtrl == FALSE ) )
				{
					/* ScanCProcessing request */
					AdatV->NextSeqPtr = ADAT_SEQ_JRATSET;
					FunExe->State = ADAT_SEQ_NOTCHSET;
				}
				AdatScanCRequest( ADAT_NO_OPE, AdatV );			/* ScanCProcessing request			*/
				AdatV->f.VibDetectinCycle = TRUE;				/* Vibration statusON				*/
			}
		/*--------------------------------------------------------------------------------------*/
		/*	Inertia identification completed judgment																*/
		/*--------------------------------------------------------------------------------------*/
			if(( AdatV->PrgJogStat == ADAT_PJOG_FIN )
					||  ( AdatV->PrgJogStat == ADAT_PJOG_NOP ))	/* When returning from notch settingNOP	*/
			{
				if( CpiGetJatCompStatus( &AdatP->BaseCtrlData->JatHdl ) == OK )
				{
					KlibRstLongTimer( &AdatV->JratVibWaitCntr2 );
					if( AdatV->f.VibDetectinCycle == TRUE )
					{	/* Vibration detected during driving this time	*/
						AdatV->f.VibDetectinCycle  = FALSE;	/* Vibration status clear				*/
						AdatV->JratSetPtr = 8;				/* Wait for the start of inertia re-identification	*/
						CpiInitJatOffLine( &AdatP->BaseCtrlData->JatHdl,
											AdatP->UniPrms,
											AdatP->RegManager,
											AdatP->BaseCtrlData->GainChange,
											0,
											AdatP->BaseCtrlData->PJogHdl.PJogV.Cnst.Distance,
											AdatP->BaseCtrlData->CtrlModeSet.CtrlMode.b.cm,
											AdatP->UniPrms->Bprm->SvCycleNs);

					}
					else
					{	/* No vibration detected					*/
						/* Temporarily reflect the identified value in the parameter	*/
						CpiSetEstimateJrat( &AdatP->BaseCtrlData->JatHdl, &AccessPrm );
						AdatV->JratSetPtr = 4;					/* Wait for parameter write		*/
					}
				}
				else
				{
					AdatV->JratSetPtr = 0;
					AdatV->ErrState = ADAT_J_ERR;
					AdatV->JratErr = ADAT_JRAT_ERR1;
				}
			}
			break;

		case 4:
		/*--------------------------------------------------------------------------------------*/
		/*	Inertia identification completed																	*/
		/*--------------------------------------------------------------------------------------*/
			if( ( AdatV->PrgJogStat == ADAT_PJOG_FIN )
					||  ( AdatV->PrgJogStat == ADAT_PJOG_NOP ))	/* When returning from notch settingNOP	*/
			{
				if( (AdatV->LastEstimatJrat <= ADAT_ERR_ULMT( AdatP->BaseCtrlData->JatHdl.EstimatJrat ))
							&& (AdatV->LastEstimatJrat >= ADAT_ERR_LLMT( AdatP->BaseCtrlData->JatHdl.EstimatJrat )) )
				{	/* Forward and reverse difference3.125% → Complete		*/
					CpiEndJatOffLine( &AdatP->BaseCtrlData->JatHdl, &AccessPrm );
					AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;
					AdatV->JratSetPtr = 5;					/* Wait for parameter write			*/
				}
				else if( AdatV->JratDrvCnt > ADAT_JDRV_MAXCNT )
				{
					if( (AdatV->LastEstimatJrat <= ADAT_ERR2_ULMT( AdatP->BaseCtrlData->JatHdl.EstimatJrat ))
							&& (AdatV->LastEstimatJrat >= ADAT_ERR2_LLMT( AdatP->BaseCtrlData->JatHdl.EstimatJrat )) )
					{	/* Forward and reverse difference31.25%→ Complete*/
						CpiEndJatOffLine( &AdatP->BaseCtrlData->JatHdl, &AccessPrm );
						AdatV->JratSetPtr = 5;				/* Wait for parameter write			*/
					}
					else
					{	/* The forward/reverse difference exceeds the allowable value	*/
						AdatV->ErrState = ADAT_J_ERR;		/* Inertia identification error				*/
						AdatV->JratSetPtr = 0;				/* Go to initial setting state					*/
						AdatV->JratErr = ADAT_JRAT_ERR2;
					}
					AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;
				}
				else
				{
					AdatV->JratSetPtr = 8;					/* Wait for the start of inertia re-identification	*/
					CpiInitJatOffLine( &AdatP->BaseCtrlData->JatHdl,
										AdatP->UniPrms,
										AdatP->RegManager,
										AdatP->BaseCtrlData->GainChange,
										0,
										AdatP->BaseCtrlData->PJogHdl.PJogV.Cnst.Distance,
										AdatP->BaseCtrlData->CtrlModeSet.CtrlMode.b.cm,
										AdatP->UniPrms->Bprm->SvCycleNs);

				}
				AdatV->LastEstimatJrat = AdatP->BaseCtrlData->JatHdl.EstimatJrat;
				AdatV->JratDrvCnt++;						/* Increment the identification number counter	*/
			}
			AdatV->Jrate = AdatP->BaseCtrlData->JatHdl.EstimatJrat;
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Parameter calculation & Wait for normal vibration level search											*/
		/*--------------------------------------------------------------------------------------*/
		case 5:
			AdatReg->AdatState = ADAT_STS_JEND;					/* Inertia identification completed			*/
			if( AdatV->PrgJogStat == ADAT_PJOG_NOP )
			{
				switch ( FunExe->CmdCode )
				{
					case FCMD_UP :
						if( AdatV->MoveDistance > 0 )
						{	/* Start measurement only if movement direction and key input are the same	*/
							AdatV->JratSetPtr = 6;				/* Proceed to parameter writing process		*/
						}
						break;

					case FCMD_DOWN :
						if( AdatV->MoveDistance < 0 )
						{
							AdatV->JratSetPtr = 6;				/* Proceed to parameter writing process		*/
						}
						break;

					case FCMD_EWRT: /* <S195> */
						AccessPrm.EepWrite = TRUE;
						TunePrmWriteRam( &AccessPrm, &pndef_jrate, (LONG)AdatV->Jrate, pndef_jrate.DataLength );
						break;

					case FCMD_NONE :
						break;

					default:
						rc = PRM_RSLT_CONDITION_ERR;
						break;
				}
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Parameter calculation & EEPROMWrite													*/
		/*--------------------------------------------------------------------------------------*/
		case 6:
			AdatV->JratSetPtr = 0;								/* Go to initial setting state				*/
			/* Pn103 */
			AccessPrm.EepWrite = TRUE;							/* <S195> */
			TunePrmWriteRam( &AccessPrm, &pndef_jrate, (LONG)AdatV->Jrate, pndef_jrate.DataLength );
			AdatUndoRamGainforJrat( AdatV, AdatP );
			FunExe->State = ADAT_SEQ_NORMVIB;					/* Go to normal vibration level search			*/
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Waiting for operation to start																		*/
		/*--------------------------------------------------------------------------------------*/
		case 7:
			if( AdatV->f.DrvPatternSet == TRUE )
			{
				if( AdatV->PrgJogStat == ADAT_PJOG_EXEC )
				{	/* Start operation	*/
					AdatV->JratSetPtr = 3;
					AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;
					AdatScanCRequest( ADAT_SEQ_SEARCHOPTKV, AdatV );	/* ScanCVibration detection processing request	*/
				}
			}
			break;
		/*--------------------------------------------------------------------------------------*/
		/*	Wait																			*/
		/*--------------------------------------------------------------------------------------*/
		case 8:/* Waiting to start inertia identification500ms */
			if( KlibGetLongTimerMs( AdatV->JratVibWaitCntr2 ) >= ADATVIBWAITTM1 )
			{
				AdatV->JratSetPtr = 7;							/* Start tuning again			*/
				AdatV->f.DrvPatternSet = FALSE;
				AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN5;	/* ProgramJOGOperation pattern5	*/
			}
			break;
		default:
			break;
	}
	return rc;
}

/****************************************************************************
		Advanced autotuning normal vibration level search

	Overview:	Measure the vibration level during normal operation.
			Normal vibration level search process scanCdo it with
			Also, measure the average value of the torque at stop,
			GRVLMT%, it is determined to be the gravity axis, and the load torque is added to the disturbance torque.
****************************************************************************/
static PRM_RSLT	AdatNormVibDetect( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status )
{
	PRMDATA *Prm;
	PRM_RSLT rc;
	USHORT ret0;
	BOOL TrqClamp;
	REG_ACC_T AccessPrm;

	Prm  = AdatP->UniPrms->Prm;
	rc = PRM_RSLT_SUCCESS;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	TrqClamp = (AdatP->SvControlIf->RspCtrl.RspCtrlBit >> TRQ_CLAMP_BITNO & 1);

	/*	ScanCIf an error occurs during processing	*/
	if( (status == ADAT_SEAR_ERR) || (TrqClamp == TRUE) )
	{
		AdatV->VibDetPtr = 0;									/* Go to initial setting					*/
		AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;				/* ProgramJOGOperation stop		*/
		AdatV->ErrState = ADAT_NG;								/* Abnormal termination						*/
	}

	switch ( AdatV->VibDetPtr )
	{
		/*--------------------------------------------------------------------------------------*/
		/*	Initial setting																			*/
		/*--------------------------------------------------------------------------------------*/
		case 0:
			/*----------------------------------------------------------------------------------*/
			/* Initial setting of gain																	*/
			/*----------------------------------------------------------------------------------*/
			Prm->posgn = Prm->loophz / 2;
			TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );			/* SettingKv			*/
			TunePrmWriteRam( &AccessPrm, &pndef_posgn, (LONG)Prm->posgn, pndef_posgn.DataLength );

			AdatV->f.DrvPatternSet = FALSE;
			AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN1;		/* ProgramJOGOperation pattern1	*/
			AdatV->VibSpeed     = 0;							/* Normal vibration level initialization			*/
			AdatV->TrqRefatStop = 0;							/* Torque initialization at normal stop		*/
			AdatV->VibDetPtr    = 1;							/* Go to normal vibration level search state		*/
			RemVibRsetRemVibrationFreq( AdatP->RemVibFreq );	/* Residual vibration frequency clear			*/

			if( AdatP->MencV->AxisMotType == MOTTYPE_LINEAR )
			{
				/* HPFand remove low frequency vibration	*/
				DetVibObsCalculateGains( &(AdatP->DetVib->DetVibObs), VIBOBS_KS, DETVIBLP,
				                         DETVIBHP2, AdatP->UniPrms->Bprm->SvCycleNs );
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Forward rotation/reverse rotation load torque measurement request														*/
		/*--------------------------------------------------------------------------------------*/
		case 1:
			AdatScanCRequest( ADAT_SEQ_NORMVIB, AdatV );		/* Normal vibration level measurement request		*/
			if( AdatV->f.DrvPatternSet == TRUE )
			{
				AdatV->VibDetPtr = 5;
			}
			AdatV->f.VibDetectinCycle  = FALSE;					/* Vibration status clear			*/
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Waiting for movement completion																		*/
		/*--------------------------------------------------------------------------------------*/
		case 5:
			/*----------------------------------------------------------------------------------*/
			/*	Vibration discrimination automatic notch																*/
			/* 		If vibration is detected, move to notch processing. If the notch has been set,						*/
			/*		Lower the gain until no vibration is detected										*/
			/*----------------------------------------------------------------------------------*/
			if( (status == ADAT_SEAR_VIBEND) || (status == ADAT_SEAR_STOPVIBEND) )
			{	/* Vibration detection finished */
				if( TuneAdatSetSpdCtrlGain( GAIN_DW, AdatP ) == ADAT_NG )	/* kvGainDOWN		*/
				{
					AdatV->ErrState = ADAT_NG;
					return rc;
				}
				if( ( AdatV->f.UsedAllNtFil == FALSE ) || ( AdatV->f.UsedAResCtrl == FALSE ) )
				{
					AdatV->VibDetPtr  = 0;
					AdatV->NextSeqPtr = ADAT_SEQ_NORMVIB;
					FunExe->State = ADAT_SEQ_NOTCHSET;
				}
				AdatScanCRequest( ADAT_NO_OPE, AdatV );				/* ScanCProcessing request		*/
				AdatV->f.VibDetectinCycle = TRUE;					/* Vibration statusON			*/
			}
			/* Before tuningMFC(Rigid body+Machine base model)When not set, set state sequence movement		*/
			if( AdatV->f.Used2MassOld == FALSE )
			{
				ret0 = SetMfc2MassSequence( AdatReg, AdatV, AdatP, FunExe, ADAT_SEQ_NORMVIB );
				if( ret0 )
				{
					FunExe->State = ADAT_SEQ_2MASSSET;	 /* Model tracking(Rigid body+Machine base model)Go to setting processing*/
					AdatV->NextSeqPtr = ADAT_SEQ_NORMVIB;/* Buffer for returning to original processing after setting	 */
					AdatV->f.VibDetectinCycle = TRUE;	 /* Vibration statusON					 */
				}
			}
			if( AdatV->PrgJogStat == ADAT_PJOG_FIN )
			{	/* ProgramJOGOperation completed → Regarded as stopped state */
				if( AdatV->f.VibDetectinCycle == TRUE )
				{	/* Vibration detected during driving this time	*/
					AdatV->f.VibDetectinCycle = FALSE;			/* Vibration status clear			*/
					AdatV->VibDetPtr = 0;						/* If there is vibration, try again		*/
				}
				else
				{	/* No vibration this time	*/
					AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;
					AdatV->VibDetPtr = 0;						/* Go to initial setting					*/
					FunExe->State = ADAT_SEQ_SEARCHOPTKV;		/* Go to optimum gain search				*/
				}
			}
			break;

		default:
			AdatV->ErrState = ADAT_NG;
			break;
	}
	return rc;
}


/****************************************************************************************************
		MFC(Rigid body+Machine base model)Going to sequence
****************************************************************************************************/
static USHORT	SetMfc2MassSequence( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, ULONG ADAT_MFC )
{
	UINT set_mfc_f;
	LONG UnRvibFrequency;

	UnRvibFrequency = RemVibGetRemVibFreq( AdatP->RemVibFreq );

	set_mfc_f = ( UnRvibFrequency >= VIBMFC_DET_FRQ_MIN ) &&	/* Detection frequency >= Vibration detection lower limit value		*/
				( UnRvibFrequency <= VIBMFC_DET_FRQ_MAX ) &&	/* Detection frequency <= Vibration detection upper limit value		*/
				( AdatV->f.Used2MassMfc == FALSE ) &&			/* MFC(Rigid body+Machine base model)Not set		*/
				((AdatReg->AdatLevel == ADAT_LEVEL_POS)			/* Tuning level=Positioning only	*/
			  || (AdatReg->AdatLevel == ADAT_LEVEL_POS_ST) );	/* Tuning level=Positioning(OV)	*/

		if( set_mfc_f )
		{
			AdatV->Set2MassMfcSequence = (USHORT)ADAT_MFC;	/*	Save vibration suppressed sequences */

			switch( ADAT_MFC )
			{
				case ADAT_SEQ_NORMVIB:				/* Normal vibration level measurement processing						*/
					AdatV->VibDetPtr = 0;
					break;

#if 0	/* OptimumKvWhen the search process is performedMFC(Rigid body+Machine base model)Do not perform the settings			*/
				case ADAT_SEQ_SEARCHOPTKV:			/* OptimumKvSearch processing(Status sequence is not initialized)	*/
					break;
#endif
				case ADAT_SEQ_ADJMFC:				/* Model following adjustment processing							*/
					AdatV->MfcTunePtr = ADAT_MFC_INI;
					break;
			}
			return 1;
		}
		return 0;
}

/****************************************************************************************************
		Advanced auto tuning optimal gain search state

	Overview:	Request scan C to search for optimum gain.
			When receiving a request for notch filter setting from scan C,
			Requests notch filter setting processing next time.
****************************************************************************************************/
static PRM_RSLT	AdatOptGainSearch( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status )
{
	PRMDATA *Prm;
	REG_ACC_T AccessPrm;
	PRM_RSLT rc;
	ULONG data;
	UCHAR tmpGainRatio;

	Prm  = AdatP->UniPrms->Prm;
	rc = PRM_RSLT_SUCCESS;
	tmpGainRatio = 80;											/* Gain ratio after vibration processing Initial value=80% 			*/

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;


	/*	ScanCIf an error occurs during processing	*/
	if( status == ADAT_SEAR_ERR )
	{
		AdatV->KvTunePtr = ADAT_OPTGN_INI;						/* Go to initial setting								*/
		AdatV->ErrState  = ADAT_NG;								/* Abnormal termination									*/
	}

	switch ( AdatV->KvTunePtr )
	{
		/*--------------------------------------------------------------------------------------------------*/
		/*	Initial setting																						*/
		/*--------------------------------------------------------------------------------------------------*/
		case ADAT_OPTGN_INI:
			/* Set the maximum gain to search */
			AdatV->MaxKv = Prm->adatgmax;						/* Set search maximum gain to maximum gain		*/
			/*----------------------------------------------------------------------------------------------*/
			/*	Disturbance observer settings																			*/
			/*		In first mode, disturbance observer is unconditionally enabled(The settings are one-parabalance).			*/
			/*		Positioning only mode unconditionally uses disturbance observer,MFC(The settings are one-parabalance).	*/
			/*		Other modes are function switchesONAssume that the one-parabalance disturbance observer is enabled.			*/
			/*----------------------------------------------------------------------------------------------*/
			switch ( AdatReg->AdatLevel )
			{	/* Tuning level */
				case ADAT_LEVEL_POS:							/* Positioning only								*/
				case ADAT_LEVEL_POS_ST:							/* Positioning only(Overshoot emphasis)		*/
//					TuneDobsActive( Prm );						/* Disturbance observer initial setting					*/
					TuneDobsActive( Prm, AccessPrm.hRegCfg );	/* Disturbance observer initial setting					*///<S09E>
					break;

				default:
					if( AdatP->BaseCtrlData->MFControl.conf.MFCModelSetting != 0 )
					{	/* AATEnable model following control before execution start	*/
						/* Model following control disabled				*/
						data = 0x0000FF00 & Prm->mdlsw;
						TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, data, pndef_mdlsw.DataLength );
					}

					if( AdatP->BaseCtrlData->DobsCtrl.var.DobsAct == TRUE )
					{	/* Disturbance observer enabled */
//						TuneDobsActive( Prm );		/* Disturbance observer initial setting */
						TuneDobsActive( Prm, AccessPrm.hRegCfg );	/* Disturbance observer initial setting *///<S09E>
					}
					break;
			}
			/* Gain setting */
			TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );			/* Gain calculation								*/
			/* Vibration level setting 						*/
			/* Normal vibration level during gain search2x	*/
			AdatP->DetVib->conf.VibCheckLevel = AdatV->VibSpeed * RESGMAX / 100;
			AdatV->f.VibDetectinCycle = TRUE;					/* Vibration status initialization. The initial value is with vibration	*/
			AdatV->f.DrvPatternSet  = FALSE;
			AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN1;		/* ProgramJOGDriving pattern 1				*/
			AdatV->KvTunePtr = ADAT_OPTGN_INIWAIT;				/* Wait for vibration convergence due to gain change				*/
			AdatV->f.KvTuneWait = FALSE;						/* Tuning stop flag OFF				*/
			KlibRstLongTimer( &AdatV->KvTuneWaitCntr );			/* Initialization of motor stop wait timer				*/

			if( AdatP->MencV->AxisMotType == MOTTYPE_LINEAR )
			{
				/* HPFand remove low frequency vibration	*/
				DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, VIBOBS_KS,
				                         DETVIBLP, DETVIBHP3, AdatP->UniPrms->Bprm->SvCycleNs );
			}
			break;

		/*--------------------------------------------------------------------------------------------------*/
		/*	Wait for vibration convergence due to gain change																	*/
		/*--------------------------------------------------------------------------------------------------*/
		case ADAT_OPTGN_INIWAIT:
			if( AdatV->f.DrvPatternSet == TRUE )
			{
				if( AdatV->PrgJogStat == ADAT_PJOG_FIN )
				{	/* ProgramJOGOperation completed → Wait for one cycle	*/
					AdatV->f.DrvPatternSet  = FALSE;
					AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN1;		/* ProgramJOGOperation pattern1		*/
					AdatV->KvTunePtr        = ADAT_OPTGN_REQ;			 /* Go to optimal gain search request state		*/
				}
			}
			break;

		/*--------------------------------------------------------------------------------------------------*/
		/*	Optimum gain search request																				*/
		/*--------------------------------------------------------------------------------------------------*/
		case ADAT_OPTGN_REQ:
			AdatScanCRequest( ADAT_SEQ_SEARCHOPTKV, AdatV );			/* ScanCProcessing request				*/
			AdatV->KvTunePtr = ADAT_OPTGN_SEARCH;						/* Go to optimal gain search state				*/
			break;

		/*--------------------------------------------------------------------------------------------------*/
		/*	Optimum gain search state																				*/
		/*--------------------------------------------------------------------------------------------------*/
		case ADAT_OPTGN_SEARCH:
			if( status == ADAT_SEAR_NORMALEND )
			{	/* Successful completion. When there is no vibration even at the maximum search gain	*/
				AdatV->f.DrvPatternSet  = FALSE;
				AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;		/* ProgramJOGOperation pattern3			*/
				AdatV->KvTunePtr        = ADAT_OPTGN_CHKKV_REQ;		/* Request for safety confirmation using current as optimal gain	*/
				Prm->loophz = AdatV->MaxKv;
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );			/* Gain calculation							*/
			}
			else if( (status == ADAT_SEAR_VIBEND) || (status == ADAT_SEAR_STOPVIBEND) )
			{		/* Vibration detected or End of vibration detection when stopped	*/
				TuneAdatSetSpdCtrlGain( GAIN_DW, AdatP );			/* KvGainDOWN							*/
				if( ( AdatV->f.UsedAllNtFil == TRUE ) && ( AdatV->f.UsedAResCtrl == TRUE) )
				{
					/* Notch filter already set	*/
					AdatGainDownforVibration( AdatV, AdatP, Prm->loophz, 50 );		/* Vibration post-processing 			*/
					KlibRstLongTimer( &AdatV->KvTuneCntr );
					AdatV->f.DrvPatternSet  = FALSE;
					AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;	/* ProgramJOGOperation pattern3 			*/
					AdatV->KvTunePtr     = ADAT_OPTGN_WAIT;
					AdatV->NextSubSeqPtr = ADAT_OPTGN_CHKKV_REQ;
				}
				else
				{	/* Notch filter setting processing request	*/
					AdatV->f.DrvPatternSet = FALSE;
					AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN1;	/* ProgramJOGOperation pattern1			*/
					AdatV->KvTunePtr = ADAT_OPTGN_REQ;
					AdatV->NextSeqPtr = ADAT_SEQ_SEARCHOPTKV;
					FunExe->State = ADAT_SEQ_NOTCHSET;
				}
			}
			else if( status == ADAT_SEAR_NOVIBDETECT )
			{
				/*------------------------------------------------------------------------------------------*/
				/*	For external command,KvThe command may stop during the search									*/
				/*	In that case,Kvfrom increasing rapidly, before starting operation(Motor stopped)is					*/
				/*	Do not update sequence,Kv UP											*/
				/*	This processing is performed only when an external command is issued.															*/
				/*------------------------------------------------------------------------------------------*/
				if( AdatV->PrgJogStat != ADAT_PJOG_WAIT )
				{
					if( TuneAdatSetSpdCtrlGain( GAIN_UP, AdatP ) == ADAT_NTCHNG )	/* GainUP */
					{	/* NotchQIf the value reaches the upper limit, it is considered normal termination	*/
						if( AdatV->PrgJogStat == ADAT_PJOG_FIN )
						{
							AdatV->f.DrvPatternSet = FALSE;
							AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;	/* ProgramJOGOperation pattern3	*/
						}
						AdatV->KvTunePtr = ADAT_OPTGN_CHKKV_REQ;	/* Request for safety confirmation using current as optimal gain	*/
					}
					else
					{
						if( AdatV->PrgJogStat == ADAT_PJOG_FIN )
						{
							AdatV->f.DrvPatternSet = FALSE;
							AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN1;	/* ProgramJOGOperation pattern1	*/
						}
						AdatV->KvTunePtr = ADAT_OPTGN_REQ;					/* Go to optimal gain search request state		*/
					}
					AdatV->f.KvTuneWait = FALSE;							/* Tuning stop flagOFF	*/
					KlibRstLongTimer( &AdatV->KvTuneWaitCntr );				/* Clear timer during operation			*/
				}
				else if( ( KlibGetLongTimerMs( AdatV->KvTuneWaitCntr ) <= ADATSTOPWAITTM )/* Waiting for stop3000ms	*/
				&& (AdatV->f.KvTuneWait == FALSE) )
				{
					if( TuneAdatSetSpdCtrlGain( GAIN_UP, AdatP ) == ADAT_NTCHNG )	/* GainUP 			*/
					{	/* NotchQIf the value reaches the upper limit, it is considered normal termination	*/
						AdatV->KvTunePtr = ADAT_OPTGN_CHKKV_REQ;	/* Request for safety confirmation using current as optimal gain	*/
					}
					else
					{
						AdatV->KvTunePtr = ADAT_OPTGN_REQ;					/* Go to optimal gain search request state		*/
					}
				}
				else
				{	/* Stop waiting time elapsed→Gain tuning stop	*/
					AdatV->f.KvTuneWait = TRUE;								/* Tuning stop flagON		*/
				}
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Vibration convergence wait state																	*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_OPTGN_WAIT :
			if( KlibGetLongTimerMs( AdatV->KvTuneCntr ) >= ADATVIBWAITTM1 )/* Waiting for vibration convergence500ms	*/
			{
				AdatV->KvTunePtr = AdatV->NextSubSeqPtr;
				Prm->loophz = AdatV->MaxKv;					/* Return to the maximum value of the search gain			*/
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );	/* Gain calculation						*/
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Command response safety confirmation request																*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_OPTGN_CHKKV_REQ:
			AdatScanCRequest( ADAT_CHKKV, AdatV );			/* ScanCProcessing request				*/
			AdatV->KvTunePtr = ADAT_OPTGN_CHKKV;			/* Optimum gain search state2fart			*/
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Optimum gain search state2(Confirmation section after vibration detection)											*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_OPTGN_CHKKV:
			switch ( status )
			{
				case ADAT_SEAR_STOPVIBEND :				/* End of vibration detection when stopped					*/
					tmpGainRatio = 50;					/* Gain ratio after vibration processing Initial value=80%		*/
				/* Fall Through */
				case ADAT_SEAR_VIBEND :					/* Vibration detection finished							*/
					AdatV->f.VibDetectinCycle = TRUE;	/* Vibration statusON						*/
					if( TuneAdatSetSpdCtrlGain( GAIN_DW, AdatP ) == ADAT_NG )	/* KvGainDOWN */
					{
						AdatV->ErrState = ADAT_NG;
						return rc ;
					}

					AdatV->f.DrvPatternSet = FALSE;
					AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;/* ProgramJOGOperation pattern3	*/

					if( ( AdatV->f.UsedAllNtFil == TRUE ) && ( AdatV->f.UsedAResCtrl == TRUE) )
					{	/* Notch filter already set	→ Continue processing	*/
						/* Vibration post-processing	*/
						AdatGainDownforVibration( AdatV, AdatP, Prm->loophz, tmpGainRatio );
						KlibRstLongTimer( &AdatV->KvTuneCntr );
						AdatV->KvTunePtr = ADAT_OPTGN_WAIT;
						AdatV->NextSubSeqPtr = ADAT_OPTGN_CHKKV_REQ;
					}
					else
					{	/* Notch filter setting incomplete	*/
						AdatV->KvTunePtr = ADAT_OPTGN_CHKKV_REQ;
						AdatV->NextSeqPtr = ADAT_SEQ_SEARCHOPTKV;
						FunExe->State = ADAT_SEQ_NOTCHSET;
					}
					break;

				default :
					break;
			}
			/*----------------------------------------------------------------------------------*/
			/*	Driving pattern end																*/
			/*----------------------------------------------------------------------------------*/
			if( AdatV->PrgJogStat == ADAT_PJOG_FIN )
			{	/* ProgramJOGOperation completed */
				/*------------------------------------------------------------------------------*/
				/*	Operation status clear														*/
				/*		If you do not clear it once, when you try again after vibration, the operation status will be changed				*/
				/*		ADAT_PRGJOG_FIN , and even though it is not actually driving, vibration is not detected		*/
				/*		In order to prevent													*/
				/*------------------------------------------------------------------------------*/
				if( ( AdatV->f.VibDetectinCycle == TRUE )
						|| ( AdatV->f.DrvPatternSet == FALSE ) )/* Command pattern incomplete			*/
				{	/* Vibration occurred during this driving pattern	*/
					AdatV->f.DrvPatternSet = FALSE;
					AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;/* ProgramJOGOperation pattern3	*/
					AdatV->KvTunePtr = ADAT_OPTGN_CHKKV_REQ;	 /* Retry					*/
				}
				else
				{	/* No vibration → Safety confirmation complete */
					/*--------------------------------------------------------------------------*/
					/*	Model following control adjustment sequence determination										*/
					/*		LEVEL 2, perform model-following control adjustment							*/
					/*--------------------------------------------------------------------------*/
					AdatScanCRequest( ADAT_NO_OPE, AdatV );	/* Request standby state after processing ends		*/
					AdatV->KvTunePtr = ADAT_OPTGN_INI;		/* Next tuning sequence set	*/
					AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;

					if( AdatV->f.RefInType == TRUE )
					{	/* Command input typeAAT */
						FunExe->State = ADAT_SEQ_KPSET;				/* KpSetting processing 				*/
					}
					else
					{
						if( ( AdatReg->AdatLevel == ADAT_LEVEL_POS )
								|| ( AdatReg->AdatLevel == ADAT_LEVEL_POS_ST ) )
						{
							FunExe->State = ADAT_SEQ_ADJMFC;		/* Model following control adjustment processing	*/
						}
						else
						{
							FunExe->State = ADAT_SEQ_SEARCHOPTKP;	/* KpSearch processing 				*/
						}
					}
				}
				AdatV->f.VibDetectinCycle = FALSE;					/* Vibration status clear		*/
			}
			break;

		default :
			AdatV->ErrState = ADAT_NG;
			break;
	}
	return rc ;
}

/****************************************************************************************************
	Vibration post-processing
	Lower the gain and wait for vibration convergence
****************************************************************************************************/
static void	AdatGainDownforVibration(
	ADATV 	*AdatV,
	ADATP 	*AdatP,
	USHORT	ser_kv,								/* Search maximum gain to save			*/
	UCHAR	down_ratio							/* GainDownRate						*/
	)
{
	PRMDATA *Prm;
	Prm = AdatP->UniPrms->Prm;

	/* Vibrated gain80%as the maximum value to search	*/
	AdatV->MaxKv = ADAT_GRAT_80(ser_kv);

	switch ( down_ratio )
	{
		case 80:
			Prm->loophz = ADAT_GRAT_80( Prm->loophz );		/* Increase gain80% */
			break;

		case 50:
			Prm->loophz = ADAT_GRAT_50( Prm->loophz );		/* Increase gain50% */
			break;

		default:
			break;
	}
	TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );				/* Gain calculation			*/
	return;
}


/****************************************************************************************************
		Advanced autotuning notch filter setting status

	Overview:	Create a frequency analysis request and set a notch filter after frequency analysis.
			Set the notch filter for the frequency with the largest peak.
			QThe value isKv, automatically calculated from inertia.
			In case of frequency analysis error, if the notch remains disabled due to notch setting error,
			Set the search gain to the current gain and prevent the gain from increasing any more
****************************************************************************************************/
static PRM_RSLT	AdatNotchFilSet( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status)
{
	PRM_RSLT 		rc;
	USHORT			KvGain;
	PRMDATA 		*Prm;

	rc  = PRM_RSLT_SUCCESS;
	Prm = AdatP->UniPrms->Prm;
	AdatV->ErrState = ADAT_CONTINUE;

	switch ( AdatV->NotchSetPtr )
	{
		/*--------------------------------------------------------------------------------------*/
		/*	Initial setting																			*/
		/*--------------------------------------------------------------------------------------*/
		case 0:
			AdatV->NotchSetStat = ADAT_STS_NF_SETOK;		/* Initialize notch setting status		*/
			AdatV->MaxKvbk = Prm->loophz;					/* VibratedKv					*/
			/* Notch release processing, type A release processing */
			if( (AdatNotchFilReset( AdatV, AdatP ) == TRUE )
					|| (AdatAresCtrlReset( AdatV, AdatP ) == TRUE) )
			{	/* Notch released→Perform gain search again without notch	*/
				if(AdatV->NextSeqPtr > ADAT_SEQ_SEARCHOPTKV)
				{	/* KvAfter the search sequence, after the notch release processKvRerun the search sequence	*/
					AdatV->NextSeqPtr = ADAT_SEQ_SEARCHOPTKV;
				}
				AdatV->NotchSetPtr = 4;						/* To termination setting						*/

				/* Sequence pointer clear to execute adjustment sequence after notch release from the beginning	*/
				AdatV->KvTunePtr  = 0;
				AdatV->KpTunePtr  = 0;
				AdatV->MfcTunePtr = 0;
				AdatV->KpSetPtr   = 0;
			}
			else if( ( (AdatV->VibMaxKv > Prm->loophz) && (AdatV->VibMaxKvAresOFF > Prm->loophz) )
						|| ( ((Prm->tfuncsw & 0x0F0F) == 0x0101) && ((Prm->avibsw & 0x0F) != 0x00) ) )
			{	/* NotchorA typeONIf you change the currentKvis small.→NotchorA typeONThe gain did not increase even after   */
				/* or 2Notch enabled for both stages, A-type vibration suppression enabled													*/
				/* →No processing is performed because the resonance suppression function cannot be used											*/
				AdatV->NotchSetStat = ADAT_STS_NF_SETERR;	/* Notch abnormal					*/
				AdatV->NotchSetPtr = 4;						/* To termination setting						*/
			}
			else
			{
				AdatV->f.FftReq    = TRUE;					/* Frequency analysis request ON				*/
				AdatV->NotchSetPtr = 5;						/* Wait for trace completion				*/
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Wait for trace completion																	*/
		/*--------------------------------------------------------------------------------------*/
		case 5:
			if( DetVibTraceGetStatus( AdatP->DetVib ) == DETVIBTRC_TRCEND )
			{
				Prm->loophz = ADAT_GRAT_50( Prm->loophz );	/* Increase gain50%				*/
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );	/* Gain calculation						*/
				AdatV->NotchSetPtr = 1;						/* Wait for frequency analysis					*/
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Waiting for frequency analysis to complete																	*/
		/*--------------------------------------------------------------------------------------*/
		case 1:
			if( status == TUNE_FREQ_SEARCHEND )
			{	/* Frequency analysis completed */
				AdatV->f.FftReq = FALSE;						/* 					*/
				AdatV->NotchSetPtr = 2;							/* Go to notch filter setting			*/
				Prm->loophz = AdatV->MaxKvbk;					/* Kv						*/
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );		/* Gain calculation					*/
			}
			else if( status == TUNE_FREQ_SEARCHERR )
			{	/* Error termination */
				AdatV->f.FftReq = FALSE;						/* 					*/
				AdatV->NotchSetStat = ADAT_STS_NF_ANAERR;		/* Set frequency analysis error			*/
				AdatV->NotchSetPtr = 4;							/* To termination setting					*/
				Prm->loophz = AdatV->MaxKvbk;					/* Kv						*/
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );		/* Gain calculation					*/
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Notch filter setting																	*/
		/*--------------------------------------------------------------------------------------*/
		case 2:
				/*------------------------------------------------------------------------------*/
				/*	Notch filter setting															*/
				/*------------------------------------------------------------------------------*/
				if(AdatV->f.UsedAllNtFil == FALSE)
				{
					/*--------------------------------------------------------------------------*/
					/*	Notch filter or A-type vibration damping setting condition judgment									*/
					/*		Notch settingKvSet a lower limit. minimumKvfor notchQValue-limited frequencies are	*/
					/*		AGive priority to type vibration suppressionON												*/
					/*--------------------------------------------------------------------------*/
					if( (AdatV->f.UsedAResCtrl == FALSE)								/* AMold damping adjustable			*/
							&& (FftGetAnaRsltVibmRf( AdatP->FftAna ) <= AVIB_FRQ_MAX)	/* If the frequency is within the rangeAMold vibration damping upper limit or less	*/
							&& (Prm->loophz < TUNE_ANOTCH_MINKV) )						/* NotchONMinimumKv		*/
					{
							KvGain = TUNE_ANOTCH_MINKV;				/* Minimum notch settingKvSetting		*/
					}
					else
					{
							KvGain = Prm->loophz;					/* KvSetting value					*/
					}

					/* First peak frequency	*/
					AdatP->ANotch->ANotchData.conf.freq = FftGetAnaRsltVibmRf( AdatP->FftAna );
					AdatP->ANotch->ANotchData.conf.KvGain = KvGain;
					/* Set notch setting status		*/
					AdatV->ANotchSetStat = (CHAR)TuneNotchFilterSetting( AdatP->ANotch,
					                                                     Prm, AdatP->RegManager->hPnReg );
				}
				else
				{
					AdatV->ANotchSetStat = 0x10;		/* If all notches are in use, make a notch error	*/
				}

				if( AdatV->ANotchSetStat > 0x02 )
				{	/* Notch setting error	*/
					/*------------------------------------------------------------------------------*/
					/*	A type vibration damping setting																*/
					/*------------------------------------------------------------------------------*/
					if(AdatV->f.UsedAResCtrl == FALSE)
					{
//						AdatV->AResSetStat = (CHAR)TuneATypeResSet( FftGetAnaRsltVibmRf( AdatP->FftAna ),
//						                                            	((Prm->avibsw >> 4) & 0x0F), Prm );
						AdatV->AResSetStat = (CHAR)TuneATypeResSet( FftGetAnaRsltVibmRf( AdatP->FftAna ),
						                                            	((Prm->avibsw >> 4) & 0x0F), Prm,
																		AdatP->RegManager->hPnReg );/*<S09E>*/
					}
					else
					{
						AdatV->AResSetStat = 0x10;					/* Error when type A is in use		*/
					}

					if( AdatV->AResSetStat >= 0x10 )				/* A-type setting error					*/
					{
						AdatV->NotchSetStat = ADAT_STS_NF_SETERR;	/* Notch Q value, A type setting error		*/
						AdatV->NotchSetPtr = 4;						/* To termination setting					*/
					}
					else
					{
						AdatV->NotchSetPtr = 3;						/* To wait for vibration convergence				*/
						KlibRstLongTimer( &AdatV->NotchSetCntr );	/* Initializing vibration convergence wait counter	*/
						AdatV->VibMaxKvAresOFF = Prm->loophz;		/* A typeOFF.*/
						AdatV->AResSetStatBk = AdatV->AResSetStat;	/* Save A-type setting status	*/
					}
				}
				else
				{	/* Notch setting normal	*/
					AdatV->NotchSetPtr = 3;							/* To wait for vibration convergence					*/
					KlibRstLongTimer( &AdatV->NotchSetCntr );		/* Initializing vibration convergence wait counter		*/
					AdatV->VibMaxKv = Prm->loophz;					/* NotchOFF.	*/
					AdatV->ANotchSetStatBk = AdatV->ANotchSetStat;	/* Save notch setting status	*/
				}
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );			/* TfRequired for calculation				*/
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Waiting for vibration convergence after notch setting															*/
		/*--------------------------------------------------------------------------------------*/
		case 3:
			if( KlibGetLongTimerMs( AdatV->NotchSetCntr ) >= ADATVIBWAITTM3 ) /* Waiting for vibration convergence300ms */
			{
				AdatV->NotchSetPtr = 4;							/* To termination setting					*/
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Termination setting																			*/
		/*--------------------------------------------------------------------------------------*/
		case 4:
			switch ( AdatV->NotchSetStat )
			{
				case ADAT_STS_NF_SETERR :				/* Notch Q value error → Filter settings incomplete	*/
				case ADAT_STS_NF_ANAERR:				/* Frequency analysis error						*/
					/* Reset gain	*/
					AdatV->MaxKv = ADAT_GRAT_80( Prm->loophz );		/* Vibration gain80% 	*/
					Prm->loophz  = ADAT_GRAT_80( Prm->loophz );		/* Increase gain80%	 	*/
					break;

				case ADAT_STS_NF_SETOK:
				default:
					break;
			}
			TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );	/* Gain calculation 							*/
			AdatV->NotchSetPtr = 0;						/* Go to initial setting 							*/
			FunExe->State  = AdatV->NextSeqPtr;			/* Set the saved sequence pointer to the next pointer */
			AdatV->ErrState = ADAT_OK;					/* Normal end	 							*/
			break;

		default :
			break;
	}
	return rc ;
}


/****************************************************************************************************
	Notch filter setting cancellation process

	Overview:	NotchOFF(AdatV.VibMaxKv)of62.5%
			, the settling time is set to buffer[/51] [52] only when it reaches1, No.2Cancels the last set notch of the notch filters.
			Since there is no way to determine which notch to release,tune_inf.notch_status
			If notch is canceled, do not turn on notch in subsequent tuning
			AdatV.f.UsedAllNtFilof”Set”,
****************************************************************************************************/
static SHORT	AdatNotchFilReset( ADATV *AdatV, ADATP *AdatP )
{
	PRMDATA  *Prm;
	AUTONOTCH *ANotch;
	REGIF_CONFIG_T *RegCfg;
	REG_ACC_T AccessPrm;
	SHORT	rc;

	Prm = AdatP->UniPrms->Prm;
	ANotch = AdatP->ANotch;
	RegCfg = AdatP->RegManager->hPnReg;
	rc = FALSE;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	if( (Prm->tfuncsw & 0x0F0F) != 0x0000 )
	{	/* ONand there is a notch that is	*/
		if( ADAT_GRAT_62( AdatV->VibMaxKv ) > Prm->loophz )
		{	/* NotchOFF62.5%or less	*/
			switch ( AdatV->ANotchSetStatBk )
			{	/* Notch completion status	*/
				case 0x01:								/* Previous1Row notchON		*/
					/* No.1NotchOFF	*/
					Prm->tfuncsw &= 0xFFF0;
					/* Pn408 */
					TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, Prm->tfuncsw, pndef_tfuncsw.DataLength );
					/* Pn409 */
					PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil1, 0 );
					/* Pn40A */
					PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchq1, 0 );
					rc = TRUE;
					break;

				case 0x02:								/* Previous2Row notchON		*/
					/* No.2NotchOFF	*/
					Prm->tfuncsw &= 0xF0FF;
					/* Pn408 */
					TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, Prm->tfuncsw, pndef_tfuncsw.DataLength );
					/* Pn40C */
					PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil2, 0 );
					/* Pn40D */
					PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchq2, 0 );
					rc = TRUE;
					break;

				default:
					break;
			}
			TuneNotchQ( ANotch, RegCfg, Prm->loophz, Prm->jrate, 1, Prm );
			/*----------------------------------------------------------------------*/
			/*	Processing after notch release													*/
			/*----------------------------------------------------------------------*/
			if( rc == TRUE )
			{
				AdatV->ANotchSetStat   = 0;				/* Notch status clear	*/
				AdatV->ANotchSetStatBk = 0;				/* Notch status clear	*/
				AdatV->f.UsedAllNtFil  = TRUE;			/* Mark filter set as completed	*/
			}
		}
	}
	return rc;
}

/****************************************************************************************************
	AMold vibration damping control setting cancellation process

	Overview:	AType vibration suppressionOFF(AdatV.VibMaxKvAresOFF)of62.5%
			,ACancel the mold vibration suppression control setting.
			If canceled, perform subsequent tuningAModify typeONDo not do this
			AdatV.f.UsedAResCtrlof”Set”,

	 out		: SHORT		rc				:	FALSE	:No release
												TRUE	:A-type vibration damping release
****************************************************************************************************/
static SHORT	AdatAresCtrlReset( ADATV *AdatV, ADATP *AdatP )
{
	PRMDATA			*Prm;
	REGIF_CONFIG_T	*RegCfg;
	REG_ACC_T AccessPrm;
	SHORT	rc;
	USHORT	data;

	Prm    = AdatP->UniPrms->Prm;
	RegCfg = AdatP->RegManager->hPnReg;
	rc = FALSE;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	if( (Prm->avibsw & 0x0F) != 0x00 )
	{	/* AType vibration suppressionONis	*/
		if( ADAT_GRAT_62( AdatV->VibMaxKvAresOFF ) > Prm->loophz )
		{	/* NotchOFF62.5%or less	*/
			switch ( AdatV->AResSetStatBk )
			{	/* Notch completion status	*/
				case 0x01:							/* Previous AutomaticallyAType vibration suppressionON			*/
					/* AType vibration suppressionOFF */
					data = 0xFFFFFFF0 & Prm->avibsw;
					/* Pn160 */
					TunePrmWriteRam( &AccessPrm, &pndef_avibsw, (LONG)data, pndef_avibsw.DataLength );
					/* Pn161 */
					PrmReLoadValueFromEeprom( RegCfg, &pndef_avibfrq, 0 );
					/* Pn163 */
					PrmReLoadValueFromEeprom( RegCfg, &pndef_avibdamp, 0 );
					rc = TRUE;
					break;

				default:
					break;
			}
			/*----------------------------------------------------------------------*/
			/*	Processing after canceling A-type vibration damping												*/
			/*----------------------------------------------------------------------*/
			if( rc == TRUE )
			{
				AdatV->AResSetStat = 0;				/* Clear status				*/
				AdatV->AResSetStatBk = 0;			/* Clear status				*/
				AdatV->f.UsedAResCtrl = TRUE;		/* Set A-type vibration damping automatically	*/
			}
		}
	}
	return rc;
}


/****************************************************************************************************
		Advanced autotuningKpGain setting state

	Overview:	After optimal gain search,KpSearch,KpmBefore search
			Kp.
			KvWhile the search is in progressKpis set too low, createKp,KpmStart search
			AtKpis suddenly set to a large value.
****************************************************************************************************/
static PRM_RSLT	AdatKpGainSet(ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status )
{
	PRMDATA	*Prm;
	PRM_RSLT rc;
	UCHAR tmpGainRatio;

	Prm  = AdatP->UniPrms->Prm;
	rc = PRM_RSLT_SUCCESS;
	tmpGainRatio = 80;										/* Gain ratio after vibration processing Initial value=80% */

	/* ScanCIf an error occurs during processing */
	if( status == ADAT_SEAR_ERR )
	{
		AdatV->KpSetPtr = 0;								/* Go to initial setting 						*/
		AdatV->ErrState = ADAT_NG;							/* Abnormal termination   						*/
	}

	switch ( AdatV->KpSetPtr )
	{
		/*--------------------------------------------------------------------------------------*/
		/*	Initial setting																			*/
		/*--------------------------------------------------------------------------------------*/
		case 0:
			AdatV->f.DrvPatternSet  = FALSE;
			AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;	/* ProgramJOGOperation pattern3		*/

			/* Vibration detection filter setting	*/
			if( Prm->loophz > VIBOBS_KS )
			{/* HPFSet	*/
				DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, Prm->loophz,
				                         DETVIBLP, DETVIBHP, AdatP->UniPrms->Bprm->SvCycleNs);
			}
			else
			{/* HPFSet	*/
				DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, VIBOBS_KS,
				                         DETVIBLP, DETVIBHP, AdatP->UniPrms->Bprm->SvCycleNs );
			}
			AdatP->FnCmnCtrl->State.TuningPctrlRun = TRUE;			/* Start position loop search		*/
			AdatV->KpSetPtr = 1;									/* KpTo setting request				*/
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	KpSetting request																			*/
		/*--------------------------------------------------------------------------------------*/
		case 1:
			/* Maximum to setKp		*/
			AdatV->MaxKp = Prm->loophz;						/* SettingKpThe maximum value is the current valueKv			*/
			AdatScanCRequest( ADAT_SEQ_KPSET, AdatV );		/* ScanCProcessing request				*/
			AdatV->KpSetPtr = 2;							/* KpGo to setting state						*/
			break;
		/*--------------------------------------------------------------------------------------*/
		/*	KpSetting state																			*/
		/*--------------------------------------------------------------------------------------*/
		case 2:
			switch ( status )
			{
				case ADAT_SEAR_NORMALEND :							/* Normal end					*/
					AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;		/* ProgramJOGEnd		*/
					AdatV->KpSetPtr = 10	;						/* End processing				*/
					break;
				case ADAT_SEAR_NOVIBDETECT :						/* No vibration					*/
					/* KpGainUP */
					TuneAdatSetPosCtrlGain( GAIN_UP, AdatP );
					AdatV->KpSetPtr = 1;
					break;
				case ADAT_SEAR_STOPVIBEND:							/* End of vibration detection when stopped		*/
					/* Set gain ratio50% */
					tmpGainRatio = 50;
				/* Fall Through */
				case ADAT_SEAR_VIBEND:								/* Vibration detection finished				*/
					/* KvGainDOWN */
					if( TuneAdatSetSpdCtrlGain( GAIN_DW, AdatP ) == ADAT_NG )
					{
						AdatV->ErrState = ADAT_NG;
						return rc ;
					}
					if( ( AdatV->f.UsedAllNtFil == TRUE ) && ( AdatV->f.UsedAResCtrl == TRUE) )
					{	/* Notch filter already set → Retry	*/
						/* Vibration post-processing */
						AdatGainDownforVibration( AdatV, AdatP, Prm->loophz, tmpGainRatio );
						KlibRstLongTimer( &AdatV->KpSetCntr );
						AdatV->KpSetPtr = 3;
						AdatV->NextSubSeqPtr = 1;
					}
					else
					{	/* Notch filter setting incomplete	*/
						AdatV->KpSetPtr = 1;
						AdatV->NextSeqPtr = ADAT_SEQ_KPSET;
						FunExe->State = ADAT_SEQ_NOTCHSET;
						AdatV->ErrState = ADAT_OK;					/* Normal end					*/
					}
					break;
					
				default :
					break;
			}
			break;
		/*--------------------------------------------------------------------------------------*/
		/*	Vibration convergence wait state																	*/
		/*--------------------------------------------------------------------------------------*/
		case 3:
			if( KlibGetLongTimerMs( AdatV->KpSetCntr ) >= ADATVIBWAITTM2 ) /* Vibration convergence wait process 300ms	*/
			{
				AdatV->KpSetPtr = AdatV->NextSubSeqPtr;
				Prm->loophz = AdatV->MaxKv;							/* Return to the maximum value of the search gain	*/
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );			/* Gain calculation				*/
			}
			break;
		/*--------------------------------------------------------------------------------------*/
		/*	Termination processing																			*/
		/*--------------------------------------------------------------------------------------*/
		case 10:
			AdatScanCRequest( ADAT_NO_OPE, AdatV );				/* Request standby state after processing ends	*/
			AdatV->KpSetPtr = 0;								/* Clear sequence				*/
			AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;

			if( ( AdatReg->AdatLevel == ADAT_LEVEL_POS )
					|| ( AdatReg->AdatLevel == ADAT_LEVEL_POS_ST ) )
			{
				FunExe->State = ADAT_SEQ_ADJMFC;				/* Model following control adjustment processing		*/
			}
			else
			{
				FunExe->State = ADAT_SEQ_SEARCHOPTKP;			/* KpSearch processing					*/
			}
			break;

		default :
			AdatV->ErrState = ADAT_NG;
			break;
	}
	return rc ;
}



/****************************************************************************************************
	Position loop gain change process

	Overview:	Change the position loop gain as required.
****************************************************************************************************/
static SHORT	TuneAdatSetPosCtrlGain( LONG mode, ADATP *AdatP)
{
	PRMDATA		*Prm;
	SHORT		rc;
	LONG		data;
	REG_ACC_T 	AccessPrm;

	Prm = AdatP->UniPrms->Prm;
	rc  = ADAT_OK;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	switch ( mode )
	{
		case GAIN_UP:
			data = Prm->posgn + KPUP_UNIT;			/* 3.0 [1/s] Increase							*/
			break;

		case GAIN_DW:
			data = Prm->posgn - KPDOWN_UNIT;		/* 3.0 [1/s] Decrement							*/
			break;

		case GAIN_BAL:
		case GAIN_BAL2:
			/*----------------------------------------------------------------------------------*/
			/*	Position loop gain(Kp)															*/
			/*----------------------------------------------------------------------------------*/
			if( AdatP->BaseCtrlData->CtrlCmdPrm.IPSpdControl == TRUE )
			{	/* IPControl	*/
				data = 320000 / Prm->pitime;		/*  Kp[1/s] = 0.32 / Ti[s]					*/
			}
			else
			{	/* PIControl	*/
				data = ADAT_GRAT_150( Prm->loophz );/* Kp[1/s] = Kv[rad/s] / 4 ≒ 1.5 *Kv[Hz]	*/
			}

			if( mode == GAIN_BAL2 )
			{
				if( data >= Prm->posgn )
				{
					return rc;						/* Do nothing if it is greater than or equal to the currently set value		*/
				}
			}
			break;

		case GAIN_BAL3:								/* Model following controlKpBalance				*/
		case GAIN_BAL4:
			/*----------------------------------------------------------------------------------*/
			/*	Position loop gain(Kp)															*/
			/*----------------------------------------------------------------------------------*/
			data = TuneKvFuncKp( Prm->loophz, Prm->pitime, AdatP->BaseCtrlData->CtrlCmdPrm.IPSpdControl );

			if( mode == GAIN_BAL4 )
			{
				if( data >= Prm->posgn )
				{
					return rc;						/* Do nothing if it is greater than or equal to the currently set value		*/
				}
			}
			break;

		default:
			data = Prm->posgn;						/* As is							    */
			break;
	}

	if( data < KP_UNLMT )
	{
		data = KP_UNLMT;							/* If the lower limit value is exceeded, use the lower limit value			*/
		rc = ADAT_NG;								/* Abnormal termination							*/
	}
	else if( data > pndef_posgn.UpperLimit )
	{
		data = pndef_posgn.UpperLimit;				/* If the upper limit is exceeded, set it to the upper limit			*/
	}

	TunePrmWriteRam( &AccessPrm, &pndef_posgn, (LONG)data, pndef_posgn.DataLength );
	return rc;
}

/****************************************************************************************************
	Advanced autotuning optimalKpGain search state

	Overview:	OptimumKpScan gain searchC.
****************************************************************************************************/
static PRM_RSLT	AdatOptKpSearch( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status )
{
	PRMDATA	 *Prm;
	PRM_RSLT rc;
	LONG user_maxGain;								/* Search maximum gain ratio according to user level		*/
	UCHAR tmpGainRatio;

	Prm = AdatP->UniPrms->Prm;
	rc  = PRM_RSLT_SUCCESS;
	tmpGainRatio = 80;								/* Gain ratio after vibration processing Initial value=80%				*/

	AdatV->ErrState = ADAT_CONTINUE;

	/* ScanCIf an error occurs during processing */
	if( status == ADAT_SEAR_ERR )
	{
		AdatV->KpTunePtr = 0;						/* Go to initial setting state								*/
		AdatV->ErrState = ADAT_NG;					/* Abnormal termination										*/
	}

	switch ( AdatV->KpTunePtr )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	Initial setting																				*/
		/*------------------------------------------------------------------------------------------*/
		case ADAT_OPTKP_INI:
			/* Motor stop detection(For shock suppression at control switching) */
			if( (AdatV->PrgJogStat == ADAT_PJOG_NOP) && (AdatP->SeqCtrlOut->MotStop == TRUE) )
			{	/* ProgramJOGOperation completed	*/
				/* Setting the maximum value of the search gain		*/
				user_maxGain = ADAT_LVL_80;						/* 80% gain						*/
				AdatV->MaxKv = Prm->loophz;						/* Set current gain to maximum search gain	*/
#if (FLOAT_USE==TRUE)
				Prm->loophz = (USHORT)( (LONG)AdatV->MaxKv * user_maxGain );

				/* NotchOFF.												*/
				/* In notch release processingPrm->loophzfor comparisonuser_maxGain			*/
				AdatV->VibMaxKv = (USHORT)( (LONG)AdatV->VibMaxKv * user_maxGain );
				AdatV->VibMaxKvAresOFF = (USHORT)( (LONG)AdatV->VibMaxKvAresOFF * user_maxGain );
#else
				Prm->loophz = (USHORT)MlibMulgain( (LONG)AdatV->MaxKv, user_maxGain );

				/* NotchOFF.												*/
				/* In notch release processingPrm->loophzfor comparisonuser_maxGain			*/
				AdatV->VibMaxKv = (USHORT)MlibMulgain( (LONG)AdatV->VibMaxKv, user_maxGain );
				AdatV->VibMaxKvAresOFF = (USHORT)MlibMulgain( (LONG)AdatV->VibMaxKvAresOFF, user_maxGain );
#endif /* FLOAT_USE */

				/* Gain setting	*/
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );		/* Gain calculation     					*/
				TuneAdatSetPosCtrlGain( GAIN_BAL, AdatP );		/* KpGain calculation 					*/

				/* Vibration level setting 					 */
				/* Normal vibration level during gain search2x */
				AdatP->DetVib->conf.VibCheckLevel = AdatV->VibSpeed * RESGMAX / 100;

				/* Observer gain change for oscillation detection(Kv > 80[Hz])	*/
				AdatP->FnCmnCtrl->State.TuningPctrlRun = TRUE;	/* Start position loop search				*/
				if( Prm->loophz > VIBOBS_KS )
				{	/* ObsGain = Kv	*/
					DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, Prm->loophz,
					                         DETVIBLP, DETVIBHP, AdatP->UniPrms->Bprm->SvCycleNs );
				}
				else
				{
					DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, VIBOBS_KS,
					                         DETVIBLP, DETVIBHP, AdatP->UniPrms->Bprm->SvCycleNs );
				}

				AdatV->f.DrvPatternSet = FALSE;
				AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;	/* ProgramJOGOperation pattern3		*/
				AdatV->KpTunePtr = ADAT_OPTKP_CHK_REQ;			/* OptimumKpGo to gain search request			*/
				AdatV->f.VibDetectinCycle = TRUE;				/* Vibration status initialization.			*/
																/* Initial value is with vibration					*/
			}
			break;

			/*--------------------------------------------------------------------------------------*/
			/*	OptimumKpGain search request																*/
			/*--------------------------------------------------------------------------------------*/
			case ADAT_OPTKP_CHK_REQ:
				AdatScanCRequest( ADAT_SEQ_SEARCHOPTKP, AdatV );/* ScanCProcessing request				*/
				AdatV->KpTunePtr = ADAT_OPTKP_CHK;				/* Go to optimum gain search					*/
				break;

			/*--------------------------------------------------------------------------------------*/
			/*	OptimumKpGain search state																*/
			/*--------------------------------------------------------------------------------------*/
			case ADAT_OPTKP_CHK:
				switch ( status )
				{
					case ADAT_SEAR_STOPVIBEND:					/* End of vibration detection when stopped				*/
						tmpGainRatio = 50;						/* Set gain ratio50%			*/
					/* Fall Through */

					case ADAT_SEAR_VIBEND:						/* Vibration detection finished						*/
						AdatV->f.VibDetectinCycle = TRUE;		/* Vibration status ON				*/
						if( TuneAdatSetSpdCtrlGain( GAIN_DW, AdatP ) == ADAT_NG )	/* KvGainDWON */
						{
							AdatV->ErrState = ADAT_NG;
							return rc;
						}
						TuneAdatSetPosCtrlGain( GAIN_BAL2, AdatP ); /* KpGain calculation 				*/

						if( ( AdatV->f.UsedAllNtFil == TRUE ) && ( AdatV->f.UsedAResCtrl == TRUE) )
						{	/* Notch filter already set → Continue processing */
							AdatGainDownforVibration( AdatV, AdatP, Prm->loophz, tmpGainRatio );
							KlibRstLongTimer( &AdatV->KpTuneCntr );
							AdatV->KpTunePtr = ADAT_OPTKP_WAIT;
							AdatV->NextSubSeqPtr = ADAT_OPTKP_CHK_REQ;
																/* Vibration post-processing						*/
						}
						else
						{	/* Notch filter setting incomplete	*/
							AdatV->KpTunePtr = ADAT_OPTKP_CHK_REQ;
							AdatV->NextSeqPtr = ADAT_SEQ_SEARCHOPTKP;
							FunExe->State = ADAT_SEQ_NOTCHSET;
							AdatV->ErrState = ADAT_OK;			/* Normal end							*/
						}
						break;

					case ADAT_SEAR_COINBREAKEND:				/* COINCrack detection completed					*/
						AdatV->f.VibDetectinCycle = TRUE;		/* Vibration status ON				*/
						AdatV->KpTunePtr = ADAT_OPTKP_CHK_REQ;	/* If there is vibration, try again			*/

						if( TuneAdatSetPosCtrlGain( GAIN_DW, AdatP ) == ADAT_NG )	/* KpGainDOWN */
						{
							AdatV->ErrState = ADAT_NG;
							return rc ;
						}
						break;

					default :
						break;
				}
				/*----------------------------------------------------------------------------------*/
				/*	Driving pattern end																*/
				/*----------------------------------------------------------------------------------*/
				if( AdatV->PrgJogStat == ADAT_PJOG_FIN )
				{	/* ProgramJOGOperation completed */
					if( (AdatV->f.VibDetectinCycle == TRUE )
							|| ( AdatV->f.DrvPatternSet == FALSE ) ) /* Command pattern incomplete			*/
					{	/* Vibration detected during driving this time	*/
						AdatV->f.VibDetectinCycle = FALSE;			 /* Vibration status clear		*/
						AdatV->f.DrvPatternSet = FALSE;
						AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;/* ProgramJOGOperation pattern3	*/
						AdatV->KpTunePtr = ADAT_OPTKP_CHK_REQ;		 /* If there is vibration, try again		*/
					}
					else
					{	/* No vibration during driving this time → Search completed → To termination setting	*/
						AdatScanCRequest( ADAT_NO_OPE, AdatV );		/* Request standby state after processing ends	*/
						AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;	/* ProgramJOGOperation stop		*/
						AdatV->KpTunePtr = ADAT_OPTKP_INI;			/* Next sequence pointer set	*/
						FunExe->State = ADAT_SEQ_ENDDISP;			/* Next tuning sequence set	*/
						AdatV->ErrState = ADAT_OK;					/* Normal end						*/
					}
				}
				break;

			/*--------------------------------------------------------------------------------------*/
			/*	Vibration convergence wait state																	*/
			/*--------------------------------------------------------------------------------------*/
			case ADAT_OPTKP_WAIT:
				/* Vibration convergence wait process	300ms */
				if( KlibGetLongTimerMs( AdatV->KpTuneCntr ) >= ADATVIBWAITTM2 )
				{
					AdatV->KpTunePtr = AdatV->NextSubSeqPtr;
					Prm->loophz = AdatV->MaxKv;							/* Return to the maximum value of the search gain	*/
					TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );			/* Gain calculation				*/
				}
				break;

			default :
				AdatV->ErrState = ADAT_NG;
				break;
		}
		return rc ;
}

/****************************************************************************************************
		Advanced autotuning model follow-up control adjustment state

	Overview:	With model following control enabled, model loop gain(Kpm)and
			Adjust torque FF. Other related parameters should not be adjusted as fixed values.
****************************************************************************************************/
static PRM_RSLT	AdatMfcAdjustment( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT status )
{
	MFCTRL   *MFControl;
	PRMDATA	 *Prm;
	REG_ACC_T AccessPrm;
	PRM_RSLT rc;
	LONG	 user_maxGain;
	LONG	 UnRvibFrequency;
	LONG     SettlingTime;
	UCHAR	 tmpGainRatio;
	USHORT	 edata;
	USHORT	 ret0;

	MFControl = &AdatP->BaseCtrlData->MFControl;
	Prm = AdatP->UniPrms->Prm;
	rc = PRM_RSLT_SUCCESS;
	tmpGainRatio = 80;					/* Gain ratio after vibration processing Initial value=80% 						*/
	UnRvibFrequency = RemVibGetRemVibFreq( AdatP->RemVibFreq );

	AdatV->ErrState = ADAT_CONTINUE;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;


	/*	ScanCIf an error occurs during processing	*/
	if( status == ADAT_SEAR_ERR )
	{
		AdatV->MfcTunePtr = 0;			/* Go to initial setting state										*/
		AdatV->ErrState = ADAT_NG;		/* Abnormal termination												*/
	}

	switch ( AdatV->MfcTunePtr )
	{
		/*--------------------------------------------------------------------------------------*/
		/*	Initial setting																			*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_MFC_INI:
			/* Motor stop detection(For shock suppression at control switching) */
			if( AdatP->SeqCtrlOut->MotStop == TRUE )
			{	/* ProgramJOGOperation completed	*/
				AdatV->MaxKv = Prm->loophz;				/* Set current gain to search maximum gain	*/
				user_maxGain = ADAT_LVL_80;				/* Search maximum gain ratio					*/

#if (FLOAT_USE==TRUE)
				Prm->loophz = (USHORT)( (LONG)AdatV->MaxKv * user_maxGain );
				/*	NotchOFF.											*/
				/*	In notch release processingPrm->loophzfor comparisonADAT_LVL_80			*/
				AdatV->VibMaxKv = (USHORT)( (LONG)AdatV->VibMaxKv * user_maxGain );
				AdatV->VibMaxKvAresOFF = (USHORT)( (LONG)AdatV->VibMaxKvAresOFF * user_maxGain);
#else
				Prm->loophz = (USHORT)MlibMulgain( (LONG)AdatV->MaxKv, user_maxGain );
				/*	NotchOFF.											*/
				/*	In notch release processingPrm->loophzfor comparisonADAT_LVL_80			*/
				AdatV->VibMaxKv = (USHORT)MlibMulgain( (LONG)AdatV->VibMaxKv, user_maxGain );
				AdatV->VibMaxKvAresOFF = (USHORT)MlibMulgain( (LONG)AdatV->VibMaxKvAresOFF,user_maxGain);
#endif /* FLOAT_USE */

				/* Gain setting	*/
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );		/* Gain calculation					*/
				TuneAdatSetPosCtrlGain( GAIN_BAL3, AdatP );		/* KpGain calculation					*/

				/* Search maximumKpmInitialize	*/
				AdatV->MaxKpm = Prm->adatgmax << 1;				/* Initial value isPn540×2			*/

				/*------------------------------------------------------------------------------*/
				/*	Model following settings																*/
				/*------------------------------------------------------------------------------*/
				/* Initialize parameters					*/
				/*	ANTFRQ,RESFRQuses the set value		*/
				/*	For external command,KpmStart adjustment from the setting value	*/
				switch ( AdatReg->AdatLevel )
				{	/* Tuning level */
					case ADAT_LEVEL_POS:				/* Positioning only							*/
//						TuneMfcActive( Prm );
						TuneMfcActive( Prm, AccessPrm.hRegCfg );//<S09E>
						break;
					case ADAT_LEVEL_POS_ST:				/* Positioning only(Overshoot emphasis)	*/
//						TuneMfcActiveStMode( Prm );
						TuneMfcActiveStMode( Prm, AccessPrm.hRegCfg );//<S09E>
						break;
					default:
						break;
				}
				if( MFControl->conf.MFCModelSetting > ADATMFC_ONE_MASS )
				{	/* AATPrevious two-inertiaMFCis enabled	*/
					Prm->mdlgn = TuneMfcKpmCalcAntFreq( Prm->mdlantfrq ) / 2;
				}
				else
				{	/* AATPreviousMFCDisabledorRigid bodyMFC */
					Prm->mdlgn = Prm->posgn;
				}
				/* KpmSetting 								*/
				TuneAdatSetMdlCtrlGain( GAIN_BAL, AdatP );
				/* Vibration level setting 						*/
				/* Normal vibration level during gain search2x 	*/
				AdatP->DetVib->conf.VibCheckLevel = AdatV->VibSpeed * RESGMAX / 100;
				/* Observer gain change for oscillation detection(Kv > 80[Hz])	*/
				/* Start position loop search					*/
				AdatP->FnCmnCtrl->State.TuningPctrlRun = TRUE;
				if( Prm->loophz > VIBOBS_KS )
				{	/* ObsGain = Kv	*/
					DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, Prm->loophz,
					                         DETVIBLP, DETVIBHP, AdatP->UniPrms->Bprm->SvCycleNs );
				}
				else
				{
					DetVibObsCalculateGains( &AdatP->DetVib->DetVibObs, VIBOBS_KS,
					                         DETVIBLP, DETVIBHP, AdatP->UniPrms->Bprm->SvCycleNs );
				}

				AdatV->f.DrvPatternSet = FALSE;
				AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN1;	/* ProgramJOGOperation pattern1	*/
				AdatV->MfcTunePtr = ADAT_MFC_INIWAIT;			/* Wait for vibration convergence due to gain change	*/
				AdatV->f.VibDetectinCycle = FALSE;				/* Vibration status clear			*/
				RemVibRsetRemVibrationFreq( AdatP->RemVibFreq );/* Residual vibration frequency clear			*/
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Wait for vibration convergence due to gain change														*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_MFC_INIWAIT:
			if( (MFControl->conf.MfcPexe.MFCModel == MFControl->conf.MfcPrm[0].MFCModel)/* Is the execution model the same as the parameter?			*/
				&& (MFControl->conf.MfcPexe.Kj2   == MFControl->conf.MfcPrm[0].Kj2)		/* Is the execution model frequency the same as the parameter?	*/
				&& (MFControl->conf.MfcPexe.Kf2   == MFControl->conf.MfcPrm[0].Kf2) )	/* Is the execution model gain the same as the parameter?	*/
			{
				BpiRsetCoinOffTime( AdatP->SettlingTime );	/* COINCrack time clear				*/
				BpiRsetMaxOverShoot( AdatP->SettlingTime );	/* Overshoot amount maximum value clear	*/
				AdatV->MfcTunePtr = ADAT_MFC_MAXREQ;		/* KpmGo to tuning request			*/
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	KpmTuning request																	*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_MFC_MAXREQ:
			AdatScanCRequest( ADAT_SEQ_ADJMFC, AdatV );		/* ScanCProcessing request				*/
			AdatV->MfcTunePtr = ADAT_MFC_MAXSEARCH;			/* KpmGo to tuning				*/
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	KpmTuning status																	*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_MFC_MAXSEARCH:
			switch ( status )
			{
				case ADAT_SEAR_STOPVIBEND:				/* End of vibration detection when stopped					*/
					tmpGainRatio = 50;					/* Set gain ratio50%				*/
				/* Fall Through */
				case ADAT_SEAR_VIBEND:					/* Vibration detection finished							*/
					AdatV->f.VibDetectinCycle = TRUE;	/* One time to operate all operation patterns */
														/*	Initial value is vibration status ON	*/
					if( TuneAdatSetSpdCtrlGain( GAIN_DW, AdatP ) == ADAT_NG )	/* KvGainDOWN	*/
					{
						AdatV->ErrState = ADAT_NG;
						return rc ;
					}
					TuneAdatSetPosCtrlGain( GAIN_BAL4, AdatP ); /* KpGain calculation	*/

					/*--------------------------------------------------------------------------*/
					/*	KpmIf the command vibrates during the searchKpm=KpKpmResearch the maximum value						*/
					/*--------------------------------------------------------------------------*/
					if(Prm->mdlgn > Prm->posgn)								/* Kpm > Kp			*/
					{
						Prm->mdlgn = Prm->posgn;							/* Kpm ← Kp		*/
						TuneAdatSetMdlCtrlGain( GAIN_BAL, AdatP );			/* KpmSetting			*/
						/* SearchKpmSearch the maximum value1/2,	*/
						AdatV->MaxKpm = MlibMAX( (AdatV->MaxKpm >> 1), Prm->mdlgn );
					}

					if( ( AdatV->f.UsedAllNtFil == TRUE ) && ( AdatV->f.UsedAResCtrl == TRUE) )
					{	/* Notch filter already set → COINCrack confirmation request	*/
						/* Vibration post-processing */
						AdatGainDownforVibration( AdatV, AdatP, Prm->loophz, tmpGainRatio );
						KlibRstLongTimer( &AdatV->MfcTuneCntr );
						AdatV->MfcTunePtr = ADAT_MFC_WAIT;
						AdatV->NextSubSeqPtr = ADAT_MFC_MAXREQ;
					}
					else
					{	/* Notch filter setting incomplete	*/
						AdatV->MfcTunePtr = ADAT_MFC_MAXREQ;
						AdatV->NextSeqPtr = ADAT_SEQ_ADJMFC;
						FunExe->State = ADAT_SEQ_NOTCHSET;
						AdatV->ErrState = ADAT_OK;				/* Normal end							*/
					}
					break;

				case ADAT_SEAR_COINBREAKEND:					/* COINCrack detection completed					*/
					AdatV->MfcTunePtr = ADAT_MFC_CHK_REQ;		/* COINCrack confirmation request				*/
					AdatV->f.VibDetectinCycle = TRUE;			/* 1times,*/
																/* Initial value is vibration statusON	*/
					TuneAdatSetMdlCtrlGain( GAIN_DW, AdatP );	/* KpmGainDOWN				*/
					break;

				case ADAT_SEAR_COINOK :
					TuneAdatSetMdlCtrlGain( GAIN_UP, AdatP );	/* KpmGainUP						*/
					AdatV->MfcTunePtr = ADAT_MFC_MAXREQ;		/* KpmGo to tuning confirmation request		*/

					/*------------------------------------------------------------------------------*/
					/*	KpmUpper limit processing															*/
					/*------------------------------------------------------------------------------*/
					if( (Prm->mdlgn > AdatV->MaxKpm) || (Prm->mdlgn == pndef_mdlgn.UpperLimit) )
					{	/* Exceeded the upper limit	*/
						AdatV->MfcTunePtr = ADAT_MFC_CHK_REQ;
						AdatV->f.VibDetectinCycle = TRUE;	/* 1times,	*/
															/* Initial value is vibration statusON		*/
						AdatV->ErrState = ADAT_OK;
					}
					/*------------------------------------------------------------------------------*/
					/*  2InertiaMFCIf low frequency vibration is detected even though it is valid, lower the gain		*/
					/*------------------------------------------------------------------------------*/
					if( (UnRvibFrequency >= VIBMFC_DET_FRQ_MIN)		/* Detection frequency >= Vibration detection lower limit value	*/
						&& (UnRvibFrequency <= VIBMFC_DET_FRQ_MAX)	/* Detection frequency <= Vibration detection upper limit value	*/
						&& (MFControl->conf.MFCModelSetting > ADATMFC_ONE_MASS) ) /* 2InertiaMFCEnabled 	*/
					{
						AdatV->MfcTunePtr = ADAT_MFC_CHK_REQ;		/* COINCrack confirmation request					*/
						AdatV->f.VibDetectinCycle = TRUE;			/* 1times, vibration frequency is not calculated until*/
																	/* Initial value is vibration statusON		*/
						TuneAdatSetMdlCtrlGain( GAIN_DW, AdatP );	/* KpmGainDOWN						*/
					}
					if( MFControl->conf.MFCModelSetting > ADATMFC_ONE_MASS )
					{	/* 2InertiaMFC or Rigid body + machine baseMFC	*/
						if( Prm->mdlgn > (3 * Prm->mdlantfrq) )
						{	/* Exceeded the upper limit */
							AdatV->MfcTunePtr = ADAT_MFC_CHK_REQ;	/* COINCrack confirmation request					*/
							AdatV->f.VibDetectinCycle = TRUE;		/* 1times,	*/
																	/* Initial value is vibration statusON		*/
							TuneAdatSetMdlCtrlGain( GAIN_DW, AdatP );	/* KpmGainDOWN					*/
						}
					}
					break;

				default :
					break;
			}
			/* 2InertiaMFCSet the settings to the notch settingCOINPriority over Crack Detection */
			if( AdatV->f.Used2MassOld == FALSE )
			{
				ret0 = SetMfc2MassSequence( AdatReg, AdatV, AdatP, FunExe, ADAT_SEQ_ADJMFC );
				if( ret0 )
				{
					FunExe->State = ADAT_SEQ_2MASSSET;	/* Model tracking(Rigid body+Machine base model)Go to setting processing*/
					AdatV->NextSeqPtr = ADAT_SEQ_ADJMFC;/* Buffer for returning to original processing after setting	*/
					AdatV->f.VibDetectinCycle = TRUE;	/* Vibration statusON						*/
				}
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	Vibration convergence wait state																	*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_MFC_WAIT:
			/* Vibration convergence wait process	300ms */
			if( KlibGetLongTimerMs( AdatV->MfcTuneCntr ) >= ADATVIBWAITTM2 )
			{
				AdatV->MfcTunePtr = AdatV->NextSubSeqPtr;
				Prm->loophz = AdatV->MaxKv;						/* Return to the maximum value of the search gain		*/
				TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );		/* Gain calculation					*/
			}
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	COINCrack confirmation request status																*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_MFC_CHK_REQ:
			AdatV->settling_max = 0;							/* Resetting maximum value of settling time	*/
			AdatScanCRequest( ADAT_VIBCOIN, AdatV );			/* ScanCProcessing request			*/
			AdatV->MfcTunePtr = ADAT_MFC_CHK;					/* COINGo to crack confirmation				*/
			MfcPrmReSet( AdatV, AdatP );
			break;

		/*--------------------------------------------------------------------------------------*/
		/*	COINCrack confirmation status																	*/
		/*--------------------------------------------------------------------------------------*/
		case ADAT_MFC_CHK:
			switch ( status )
			{
				case ADAT_SEAR_STOPVIBEND:						/* End of vibration detection when stopped			*/
					tmpGainRatio = 50;							/* Set gain ratio50%		*/
				/* Fall Through */
				case ADAT_SEAR_VIBEND:							/* Vibration detection finished					*/
					AdatV->f.VibDetectinCycle = TRUE;			/* Vibration statusON				*/
					if( TuneAdatSetSpdCtrlGain( GAIN_DW, AdatP ) == ADAT_NG ) /* KvGainDOWN	*/
					{
						AdatV->ErrState = ADAT_NG;
						return rc ;
					}
					TuneAdatSetPosCtrlGain( GAIN_BAL4, AdatP );	/* KpGain calculation					*/
					TuneAdatSetMdlCtrlGain( GAIN_DW, AdatP );	/* KpmGainDOWN				*/

					if( ( AdatV->f.UsedAllNtFil == TRUE ) && ( AdatV->f.UsedAResCtrl == TRUE) )
					{	/* Notch filter already set → Continue processing	*/
						/* Vibration post-processing */
						AdatGainDownforVibration( AdatV, AdatP, Prm->loophz, tmpGainRatio );
						KlibRstLongTimer( &AdatV->MfcTuneCntr );
						AdatV->MfcTunePtr = ADAT_MFC_WAIT;
						AdatV->NextSubSeqPtr = ADAT_MFC_CHK_REQ;
					}
					else
					{	/* Notch filter setting incomplete	*/
						AdatV->MfcTunePtr = ADAT_MFC_CHK_REQ;
						AdatV->NextSeqPtr = ADAT_SEQ_ADJMFC;
						FunExe->State = ADAT_SEQ_NOTCHSET;
						AdatV->ErrState = ADAT_OK;					/* Normal end					*/
					}
					break;

				case ADAT_SEAR_COINBREAKEND:						/* COINCrack detection completed			*/
					AdatV->f.VibDetectinCycle = TRUE;				/* Vibration statusON			*/
					AdatV->MfcTunePtr = ADAT_MFC_CHK_REQ;			/* If there is vibration, try again	*/

					if( TuneAdatSetMdlCtrlGain( GAIN_DW, AdatP) == ADAT_NG  )	/* KpmGainDOWN */
					{
						AdatV->ErrState = ADAT_NG;
						Mfc2MassReset( AdatV, AdatP );
						return rc ;
					}
					break;

				default :
					break;
			}
			/* Holding maximum value of settling time */
			SettlingTime = BpiGetSettlingTime( AdatP->SettlingTime );
			if( AdatV->settling_max < SettlingTime )
			{
				AdatV->settling_max = SettlingTime;
			}

			/*----------------------------------------------------------------------------------*/
			/*	Driving pattern end																*/
			/*----------------------------------------------------------------------------------*/
			if( AdatV->PrgJogStat == ADAT_PJOG_FIN )
			{	/* ProgramJOGOperation completed */
				if( (AdatV->f.VibDetectinCycle == TRUE )
						|| ( AdatV->f.DrvPatternSet == FALSE ) )	/* Command pattern incomplete		*/
				{	/* Vibration detected during driving this time	*/
					AdatV->f.VibDetectinCycle = FALSE;				/* Vibration status clear		*/
					AdatV->f.DrvPatternSet = FALSE;
					AdatV->PrgJogPatternReq = ADAT_PJOG_PATTERN3;	/* ProgramJOGOperation pattern3 */
					AdatV->MfcTunePtr = ADAT_MFC_CHK_REQ;			/* If there is vibration, try again	  */
				}
				else
				{	/* No vibration during driving this time → Adjustment completed → Termination setting */
					if( ( AdatV->f.Used2MassMfc == TRUE )
						&& ( AdatV->f.Mfc2MassOK == FALSE )
						&& ( AdatV->f.MfcLastSet == FALSE )
						&& ( AdatV->f.Used2MassOld == FALSE ) )
					{
					/*--------------------------------------------------------------------------*/
					/*		KvAdjustment incomplete state(Normal vibration level measurement sequence)ONIf you do	*/
					/*		Rigid bodyMFC.								*/
					/*--------------------------------------------------------------------------*/
						if(AdatV->Set2MassMfcSequence == ADAT_SEQ_ADJMFC)
						{	/* MFCVibration suppression setting was performed during adjustment					*/
							/* Currently rigid body+Check whether it is set to machine base model	*/
							if( MFControl->conf.MFCModelSetting > ADATMFC_ONE_MASS )
							{
								FunExe->State = ADAT_SEQ_ADJMFC;			/* Next tuning sequence set	*/
								PrmReLoadValueFromEeprom( AdatP->RegManager->hPnReg, &pndef_mdlsw, 0 );
								edata = (USHORT)((Prm->mdlsw) & (RIGMFC_PRM_ACTIVE));	/* Pn140.1 -> 0 */
								TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, (LONG)edata, pndef_mdlsw.DataLength );

								AdatV->MfcTunePtr   = ADAT_MFC_INI;			/* Next sequence pointer set		*/
								AdatV->settling_buf = AdatV->settling_max;	/* Settling time buffer				*/
								AdatV->Kpm_buf = (USHORT)Prm->mdlgn;		/* Kpmthe buffer					*/
								AdatV->frq_buf = (USHORT)Prm->mdlantfrq;	/* Buffer resonance and antiresonance frequencies		*/
								AdatV->Kv_buf  = (USHORT)Prm->loophz;		/* Kvthe buffer						*/
							}
							else
							{
								/* MFC(Rigid body+Machine base model)is short, reset parameters	 */
								if( AdatV->settling_max > AdatV->settling_buf )
								{
									/* Get parameterMFC(Rigid body+Machine base model)*/
									TunePrmWriteRam( &AccessPrm, &pndef_mdlgn, (LONG)AdatV->Kpm_buf, pndef_mdlgn.DataLength );			/* Pn141 */
									TunePrmWriteRam( &AccessPrm, &pndef_mdlantfrq, (LONG)AdatV->frq_buf, pndef_mdlantfrq.DataLength );	/* Pn145 */
									TunePrmWriteRam( &AccessPrm, &pndef_mdlresfrq, (LONG)AdatV->frq_buf, pndef_mdlresfrq.DataLength );	/* Pn146 */

									PrmReLoadValueFromEeprom( AdatP->RegManager->hPnReg, &pndef_mdlsw, 0 );
									edata =(USHORT)( ( Prm->mdlsw ) | ( VIBMFC_PRM_ACTIVE ) );
									TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, (LONG)edata, pndef_mdlsw.DataLength );

									Prm->loophz = AdatV->Kv_buf;
									TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );
									TuneAdatSetPosCtrlGain( GAIN_BAL3, AdatP );
								}
								/*------------------------------------------------------------------------------*/
								/* Termination processing																		*/
								/*------------------------------------------------------------------------------*/
								/*	2InertiaMFCAdjustments in settings are complete. Regardless of the mechanistic model ultimately selected,		*/
								/*	AdatV->f.Mfc2MassOK = TRUE											*/
								/*------------------------------------------------------------------------------*/
								AdatV->MfcTunePtr = ADAT_MFC_CHK_REQ;	/* COINCrack confirmation request					*/
								AdatV->f.VibDetectinCycle = TRUE;		/* 1times, vibration frequency is not calculated until*/
								AdatV->f.Mfc2MassOK = TRUE;
							}
						}
						else
						{
							AdatV->f.Mfc2MassOK = TRUE;					/* 2Final inertia adjustment completed					*/
						}
					}
					else
					{
						AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;		/* ProgramJOGOperation stop				*/
						AdatScanCRequest( ADAT_NO_OPE, AdatV );			/* Request standby state after processing ends			*/
						AdatV->MfcTunePtr = ADAT_MFC_INI;				/* Next sequence pointer set			*/
						FunExe->State = ADAT_SEQ_ENDDISP;				/* Next tuning sequence set		*/
						AdatV->ErrState = ADAT_OK;						/* Normal end								*/
					}
				}
			}
			break;

		default :
			AdatV->ErrState = ADAT_NG;
			break;
	}
	return rc ;
}

/****************************************************************************************************
	Model loop gain change processing

	Overview:Change the model loop gain as required.
****************************************************************************************************/
static SHORT	TuneAdatSetMdlCtrlGain( LONG mode, ADATP *AdatP )
{
	PRMDATA *Prm;
	REG_ACC_T AccessPrm;
	LONG	data;
	SHORT	rc;

	Prm = AdatP->UniPrms->Prm;
	rc  = ADAT_OK;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	switch ( mode )
	{
		case GAIN_UP:
			data = ADAT_GRAT_106( Prm->mdlgn );		/* 17/16 = 1.0625 6.25%Increase				*/
			break;

		case GAIN_DW:
			data = ADAT_GRAT_94( Prm->mdlgn );		/* 15/16 = 0.9375 6.25%Decrement				*/
			break;

		default:
			data = Prm->mdlgn;						/* As is							*/
			break;
	}

	if( data < KPM_UNLMT )
	{
		data = KPM_UNLMT;							/* If the lower limit value is exceeded, use the lower limit value		*/
		rc = ADAT_NG;								/* Abnormal termination						*/
	}
	else if( data > pndef_mdlgn.UpperLimit )
	{
		data = pndef_mdlgn.UpperLimit;				/* If the upper limit is exceeded, clamp it	*/
	}
	TunePrmWriteRam( &AccessPrm, &pndef_mdlgn, (LONG)data, pndef_mdlgn.DataLength );
	return rc;
}


/****************************************************************************************************
		MFC(Rigid body+Machine base model)Return to the settings
		Rigid bodyMFCand return with minimumKpm, change the setting2Return to inertia
****************************************************************************************************/
static void	MfcPrmReSet( ADATV *AdatV, ADATP *AdatP )
{
	PRMDATA *Prm;
	USHORT	edata;
	REG_ACC_T AccessPrm;

	Prm = AdatP->UniPrms->Prm;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = AdatP->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/* MFC(Rigid body+Machine base model)If adjusted, set it	*/
	if( AdatV->f.MfcLastSet == TRUE )
	{
		/* Get parameterMFC(Rigid body+Machine base model)*/
		/* Pn141 */
		TunePrmWriteRam( &AccessPrm, &pndef_mdlgn, (LONG)AdatV->Kpm_buf, pndef_mdlgn.DataLength );
		/* Pn145 */
		TunePrmWriteRam( &AccessPrm, &pndef_mdlantfrq, (LONG)AdatV->frq_buf, pndef_mdlantfrq.DataLength );
		/* Pn146 */
//		TunePrmWriteRam( &AccessPrm, &pndef_mdlresfrq, (LONG)AdatV->Kpm_buf, pndef_mdlresfrq.DataLength );
		TunePrmWriteRam( &AccessPrm, &pndef_mdlresfrq, (LONG)AdatV->frq_buf, pndef_mdlresfrq.DataLength );//<S09E>

		PrmReLoadValueFromEeprom( AdatP->RegManager->hPnReg, &pndef_mdlsw, 0 );
		edata = (USHORT)( ( Prm->mdlsw ) | ( VIBMFC_PRM_ACTIVE ) );
		/* Pn140 */
//		TunePrmWriteRam( &AccessPrm, &pndef_ntchq1, (LONG)edata, pndef_ntchq1.DataLength );
		TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, (LONG)edata, pndef_mdlsw.DataLength );//<S09E>

		Prm->loophz = AdatV->Kv_buf;
		TuneAdatSetSpdCtrlGain( GAIN_CAL, AdatP );
		TuneAdatSetPosCtrlGain( GAIN_BAL3, AdatP );
	}
	return;
}


/****************************************************************************************************
		MFC(Rigid body+Machine base model)Return to the settings
****************************************************************************************************/
static void	Mfc2MassReset( ADATV *AdatV, ADATP *AdatP )
{
	/* MFC(Rigid body+Machine base model)If adjusted, set it,Normal end												*/
	/* Rigid bodyMFCand return with minimumKpm, change the setting back to 2 inertia					*/
	if( ( AdatV->f.Used2MassMfc == TRUE )
		&& ( AdatP->BaseCtrlData->MFControl.conf.MFCModelSetting <= ADATMFC_ONE_MASS) )
		{
			/* Changed to normal end */
			AdatV->f.MfcLastSet       = TRUE;
			AdatV->MfcTunePtr         = ADAT_MFC_CHK_REQ;	/* CCrack confirmation request 						*/
			AdatV->f.VibDetectinCycle = TRUE;				/* One time to operate all operation patterns */
			AdatV->ErrState           = ADAT_CONTINUE;		/* Continuous operation								*/
		}
		return;
}

/******************************************************************************************************
	Advanced autotuning end setting process

	Overview:	Perform processing for normal termination and abnormal termination.
			Upon successful completion, the adjusted parameters will beEEPROM.
			When abnormal termination occurs,EEPROM.
			Forcibly terminateopf.func_mode_clrto end the auxiliary function.
			To restore the changed parameter, use the parameterfunc_clear()Used with functions.
******************************************************************************************************/
static PRM_RSLT	AdatFinishOpe( ADATREG *AdatReg, ADATV *AdatV, ADATP *AdatP, FUNEXE *FunExe, SHORT err_status )
{
	PJOGV *PJogV;
	PRM_RSLT rc;

	PJogV = &AdatP->BaseCtrlData->PJogHdl.PJogV;
	rc = PRM_RSLT_SUCCESS;
	AdatV->f.DrvErrDetect = FALSE;

	/* Common processing when an error occurs																			*/
	if( err_status <= ADAT_NG )
	{	/* Error occurrence 																				*/
		if( AdatV->f.RefInType == FALSE )
		{
			AdatV->PrgJogPatternReq = ADAT_PJOG_STOP;		/* ProgramJOGOperation stop				*/
			PJogV->Var.RoutCmd = PJOGCMD_ABORT;				/* ProgramJOGCancellation					*/
		}
		AdatV->ScanCRequest = ADAT_NO_OPE;					/* NOPProcessing request set					*/

		if( err_status > ADAT_COMERR )
		{	/* Error code is greater than communication error → Servo off										*/
			AdatP->FnCmnCtrl->FnSvonReq = FALSE;			/* When abnormal termination occurs, the servo is turned off for the time being 	*/
			FunExe->State = ADAT_SEQ_WAIT;					/* Return to waiting for termination state					 	*/
		}
		else/* Error end after deceleration and stop																*/
		{
			/* COINLatch completion or Motor stop detection														*/
			if( (AdatP->BaseCtrlData->PosCtrlSts.CoinSignal == TRUE)
									|| (AdatP->SeqCtrlOut->MotStop == TRUE) )
			{/* COINLatch completion or Motor stop detection													*/
				FunExe->State = ADAT_SEQ_WAIT;					/* Return to waiting for termination state					*/
			}
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Error processing																					*/
	/*----------------------------------------------------------------------------------------------*/
	switch(err_status)
	{
		case ADAT_OK:											/* Normal end							*/
			AdatReg->AdatState = ADAT_STS_DONE;					/* Tuning completed					*/
			if( AdatV->f.RefInType == FALSE )
			{
				AdatP->FnCmnCtrl->FnSvonReq = FALSE;			/* Servo off when tuning is completed	*/
			}

			switch ( FunExe->CmdCode )
			{
				case FCMD_EWRT:									/* EEPROMWrite 					*/
					AdatUpdateGainInEeprom( AdatReg, AdatP );
					FunExe->State = ADAT_SEQ_WAIT;				/* Save current sequence pointer 	*/
					break;

				case FCMD_NONE :
				case FCMD_UP :
				case FCMD_DOWN :
					break;

				default:
					rc = PRM_RSLT_CONDITION_ERR;
					break;
			}
			break;

		case ADAT_JST_ERR:										/* Inertia identification start error			*/
		case ADAT_NOTRDYERR:									/* Operation preparation incomplete error				*/
			FunExe->State = ADAT_SEQ_WAIT;						/* Save current sequence pointer	*/
			AdatReg->AdatState = ADAT_STS_NG;					/* Operation failure 						*/
			break;

		case ADAT_J_ERR:										/* Inertia identification error				*/
			FunExe->State = ADAT_SEQ_WAIT;						/* Save current sequence pointer	*/
			AdatReg->AdatState = ADAT_STS_JERR;					/* Inertia identification failure				*/
			break;

		case ADAT_COMERR:										/* Communication error during operation				*/
			if( (AdatP->BaseCtrlData->PosCtrlSts.CoinSignal == TRUE)
				|| (AdatP->SeqCtrlOut->MotStop == TRUE) )
			{
				FunExe->State = ADAT_SEQ_WAIT;					/* Save current sequence pointer	*/
			}
			AdatReg->AdatState = ADAT_STS_ABORT;				/* Interruption due to abnormality during operation			*/
			break;

		default:												/* For other errors				*/
			FunExe->State = ADAT_SEQ_WAIT;						/* Save current sequence pointer	*/
			AdatReg->AdatState = ADAT_STS_ERR;					/* Tuning failure 				*/
			break;
	}
	return rc;
}

/****************************************************************************************************
	Advanced autotuning scanCMain

	Overview:	ScanC
		 	Perform the following tuning process according to the processing request.
****************************************************************************************************/
void	CpxAdvancedAutoTuning( ADATHNDL	*AdatHndl, FUN_CMN_CONTROL *FnCmnCtrl )
{
	SHORT rc;
	ULONG coinOffTimeCnt;
	LONG  wk1,wk2,wk3;
	BOOL  Prefon;

	ADATV	*AdatV;
	ADATP	*AdatP;

	rc = ADAT_SEAR_NORMALEND;
	AdatV = &AdatHndl->AdatV;
	AdatP = &AdatHndl->AdatP;

	if( AdatV->f.GainTuneRun == TRUE )
	{
		AdatScanCSetReq( AdatV );								/* Round			*/
		/*------------------------------------------------------------------------------------------*/
		/*	Vibration detection flag set																	*/
		/*------------------------------------------------------------------------------------------*/
		AdatV->f.VibDetect = (AdatP->DetVib->var.VibDetFlg == TRUE) ? TRUE : FALSE;
		/*------------------------------------------------------------------------------------------*/
		/*	COINCrack detection flag set																*/
		/*------------------------------------------------------------------------------------------*/
		/* Positioning completion failure cumulative time acquisition 															*/
		coinOffTimeCnt = BpiGetCoinOffTime( AdatP->SettlingTime );
		/* When the allowable time is exceededCOINCrack Detection																*/
		AdatV->f.CoinBreak = (coinOffTimeCnt > COINOFFLMT) ? TRUE : FALSE;
		/*------------------------------------------------------------------------------------------*/
		/*	COINLatch flag set																	*/
		/*------------------------------------------------------------------------------------------*/
		AdatV->f.CoinLatch = ( AdatP->SettlingTime->StlgCoinLatch ) ? TRUE : FALSE;
		/*------------------------------------------------------------------------------------------*/
		/*	Overshoot excessive flag set														*/
		/*------------------------------------------------------------------------------------------*/
		/*	Overshoot error detected when the allowable overshoot is exceeded									*/
		AdatV->f.OvsErr = ( AdatP->SettlingTime->UnMaxOverShoot > AdatP->SettlingTime->OvsErrDetLvl )
																						? TRUE : FALSE;
		/*------------------------------------------------------------------------------------------*/
		/*	Vibration detection processing when stopped																		*/
		/*------------------------------------------------------------------------------------------*/
		/*	High-pass filtering																	*/
		wk3 = MlibHpfilter1( (LONG)AdatP->SeqMotSts->var.TrqRefMon, AdatV->KTrqHpFil, &AdatV->KTrqHpftmp );
		if( AdatP->SettlingTime->StlgCoinLatch == FALSE )
		{
			KlibRstLongTimer( &AdatV->CoinOnCntr );
		}
		if( KlibGetLongTimerMs( AdatV->CoinOnCntr ) >= STPDETTM )
		{	/* COINFrom latch completion100[ms]Elapsed time → Regarded as stopped state 									*/
			/* Maximum and minimum amplitude measurement																		*/
			if( wk3 >= AdatV->MaxTrqRefatStop )
			{
				AdatV->MaxTrqRefatStop = wk3;
			}
			else if( wk3 <= AdatV->MinTrqRefatStop )
			{
				AdatV->MinTrqRefatStop = wk3;
			}
			/*--------------------------------------------------------------------------------------*/
			/* Vibration detection when stopping																		*/
			/*	Normal torque STPVIBDET[%]or more and ratedSTPVIBLMT[%]or above						*/
			/*--------------------------------------------------------------------------------------*/
			wk1 = AdatV->TrqRefatStop * STPVIBDET / 100;			/* Vibration threshold torque when stopped			*/
			wk2 = AdatV->MaxTrqRefatStop - AdatV->MinTrqRefatStop;	/* Vibration amplitude when stopping				*/
#if (FLOAT_USE==TRUE)
			if( wk2 < ( STPVIBLMT * AdatP->UniPrms->Bprm->Knortrq ) )
			{
				wk2 = 0;											/* If it is below the threshold, vibration amplitude is cleared	*/
			}
#else
			if( wk2 < MlibMulgain( STPVIBLMT, AdatP->UniPrms->Bprm->Knortrq ) )
			{
				wk2 = 0;											/* If it is below the threshold, vibration amplitude is cleared	*/
			}
#endif /* FLOAT_USE */
			/* If the threshold torque is exceeded, vibration detection at stop is detected */
			AdatV->f.VibDetectatStop = (wk2 > wk1) ? TRUE : FALSE;
		}
		else
		{
			/* Initializing torque amplitude when stopping	*/
			AdatV->MaxTrqRefatStop = wk3;							/* Initial value is current torque command		*/
			AdatV->MinTrqRefatStop = wk3;
			AdatV->f.VibDetectatStop = FALSE;						/* Initialization of vibration detection flag when stopped	*/
		}

		/*------------------------------------------------------------------------------------------*/
		/*	Advanced autotuning processing sequence											*/
		/*------------------------------------------------------------------------------------------*/
		switch ( AdatV->ScanCRequest )
		{
			case ADAT_NO_OPE:						/* no operation 								*/
				rc  = AdatV->ScanCStat;				/* Set the previous status						*/
				break;

			case ADAT_SEQ_NORMVIB:					/* Normal vibration level measurement							*/
				rc = LpxAdatVibLvlSet( AdatV, AdatP );
				rc = LpxAdatCheckKv( AdatV, AdatP );/* Check vibration even while measuring vibration level				*/
				break;

			case ADAT_SEQ_SEARCHOPTKV:				/* OptimumKv,TiGain search							*/
				rc  = LpxAdatSearchKv( AdatV, AdatP );
				break;

			case ADAT_CHKKV:						/* Safety confirmation by command response						*/
				rc  = LpxAdatCheckKv( AdatV, AdatP );
				break;

			case ADAT_SEQ_KPSET:					/* KpSetting adjustment									*/
				rc = LpxAdatKpGainSet( AdatV, AdatP );
				break;

			case ADAT_SEQ_SEARCHOPTKP:				/* OptimumKpGain search								*/
				rc = LpxAdatSearchKp( AdatV, AdatP );
				break;

			case ADAT_SEQ_ADJMFC:					/* Model following control adjustment							*/
				rc = LpxAdatAdjustMfcGain( AdatV, AdatP );
				break;

			case ADAT_VIBCOIN:						/* Model following controlCOINCheck for cracks					*/
				rc = LpxAdatCheckMfcGain( AdatV, AdatP );
				break;

			default:								/* Advanced autotuning disabled			*/
				rc = ADAT_SEAR_ERR;					/* Abnormal termination								*/
				break;
		}

		/*------------------------------------------------------------------------------------------*/
		/*	Error processing																				*/
		/*------------------------------------------------------------------------------------------*/
		if( AdatV->ScanCRequest != ADAT_NO_OPE )
		{	/* During tuning																		*/
			if( ( AdatV->f.RefInType == FALSE ) && ( FnCmnCtrl->FnSvonReq == FALSE ) )
			{	/* Servo off 																		*/
				rc = ADAT_SEAR_ERR;					/* If the servo turns off during the search process, it will be treated as an error	*/
			}
		}

		/*------------------------------------------------------------------------------------------*/
		/*	Initialization process																				*/
		/*------------------------------------------------------------------------------------------*/
		if((rc != ADAT_SEAR_CONTINUE) || (AdatV->ScanCRequest == ADAT_NO_OPE))
		{	/* Other than continuing processing orNOP															*/
			AdatV->ScanCRequest = ADAT_NO_OPE;		/* Go to standby state after processing ends					*/
													/* To continue processing execution,roundto			*/
													/* Command again									*/
			/* Clear differential speed peak value																	*/
			DetVibResetMaxMinSpeedError( AdatP->DetVib );

			/* Initializing torque amplitude when stopping																*/
			AdatV->MaxTrqRefatStop = wk3;						/* Initial value is current torque command			*/
			AdatV->MinTrqRefatStop = wk3;

			BpiRsetCoinOffTime( AdatP->SettlingTime );			/* COINCrack cumulative time clear			*/
			BpiRsetMaxOverShoot( AdatP->SettlingTime );			/* Overshoot amount maximum value clear	*/
		}
		AdatV->ScanCStat = rc;											/* Processing status set		*/
		AdatV->f.LastCoinLatch = (AdatV->f.CoinLatch)? TRUE : FALSE;	/* COINLatch signal save 		*/
		/*------------------------------------------------------------------------------------------*/
		/*	Operation status creation process																	*/
		/*		For external command,COIN, calculate										*/
		/*		Round,COIN or Since zero speed may not be detectedScanC				*/
		/*------------------------------------------------------------------------------------------*/
		if( AdatV->f.RefInType == TRUE )
		{
			Prefon = ( AdatP->SeqMotSts->var.dPcmdCnt != 0)? TRUE : FALSE;
			AdatV->PrgJogStat = LpxAdatRefInTypeDrvStatus( AdatHndl, AdatV->PrgJogPatternReq, Prefon );
		}
	}
}

/****************************************************************************
	Advanced autotuning amplitude level measurement

	Overview:	Measure the vibration level during normal operation.
			Measure the maximum differential speed value for oscillation detection during feeding with normal gain.
****************************************************************************/
static SHORT	LpxAdatVibLvlSet( ADATV *AdatV, ADATP *AdatP )
{
	SHORT	rc;
	LONG	lwork;
	rc = ADAT_SEAR_CONTINUE;

	/*--------------------------------------------------------------------------*/
	/* Measuring vibration level															*/
	/*--------------------------------------------------------------------------*/
	lwork = AdatP->DetVib->DetVibObs.var.MaxSpdErr - AdatP->DetVib->DetVibObs.var.MinSpdErr;
	if( lwork >= AdatV->VibSpeed )
	{
		AdatV->VibSpeed = lwork;					/* Maximum amplitude measurement				*/
	}

	/*--------------------------------------------------------------------------*/
	/*	Normal torque vibration measurement when stopped												*/
	/*--------------------------------------------------------------------------*/
	if( KlibGetLongTimerMs( AdatV->CoinOnCntr ) >= STPDETTM )
	{	/* COINFrom latch completion100[ms]Elapsed time → Regarded as stopped state	*/
		lwork = AdatV->MaxTrqRefatStop - AdatV->MinTrqRefatStop;
		if( AdatV->TrqRefatStop < lwork )
		{
			AdatV->TrqRefatStop = lwork;			/* Maximum amplitude					*/
		}
	}
	return rc;
}


/****************************************************************************
	Advanced autotuning Safety confirmation by command response

	Overview:	Vibration confirmation by command response.
			Vibration is detected during position command, and if it vibrates, change the gain6.25%Lower.
			TiisKv.
****************************************************************************/
static SHORT	LpxAdatCheckKv( ADATV *AdatV, ADATP *AdatP )
{
	SHORT	rc;
	rc = ADAT_SEAR_CONTINUE;

	/*--------------------------------------------------------------------------*/
	/* Vibration detection when stopping															*/
	/*--------------------------------------------------------------------------*/
	if( AdatV->f.VibDetectatStop )
	{	/* Oscillation detected	*/
		rc = ADAT_SEAR_STOPVIBEND;					/* End of vibration detection when stopped		*/
	}
	/*--------------------------------------------------------------------------*/
	/* Oscillation detection																	*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.VibDetect )
	{
		DetVibResetMaxMinSpeedError( AdatP->DetVib );
		rc = ADAT_SEAR_VIBEND;						/* Vibration detection finished				*/
	}
	return rc;
}


/****************************************************************************
	Advanced autotuning optimal gain search

	Overview:	By command responseKvFind the optimal value of.
			Vibration is detected during position command, and if there is no vibration,
			Kvof2.0[Hz].
			Tiiskv.
****************************************************************************/
static SHORT	LpxAdatSearchKv( ADATV *AdatV, ADATP *AdatP )
{
	SHORT	rc;
	rc  = ADAT_SEAR_CONTINUE;

	/*--------------------------------------------------------------------------*/
	/* When maximum gain search value is reached											*/
	/*--------------------------------------------------------------------------*/
	if( AdatP->UniPrms->Prm->loophz >= AdatV->MaxKv )
	{
		KlibRstLongTimer( &AdatV->GainUpCntr );		/* Clear gain search counter	*/
		rc = ADAT_SEAR_NORMALEND;					/* Normal end					*/
	}
	/*--------------------------------------------------------------------------*/
	/* Vibration detection when stopping															*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.VibDetectatStop )
	{	/* Oscillation detected	*/
		KlibRstLongTimer( &AdatV->GainUpCntr );		/* Clear gain search counter	*/
		rc = ADAT_SEAR_STOPVIBEND;					/* End of vibration detection when stopped		*/
	}
	/*--------------------------------------------------------------------------*/
	/* Oscillation detection																	*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.VibDetect )
	{
		DetVibResetMaxMinSpeedError( AdatP->DetVib );
		KlibRstLongTimer( &AdatV->GainUpCntr );		/* Clear gain search counter	*/
		rc = ADAT_SEAR_VIBEND;						/* Vibration detection finished				*/
	}
	else if( KlibGetLongTimerMs( AdatV->GainUpCntr ) > GNUPTM2 )
	{	/* Vibration was not detected for specified time → GainUP	*/
		KlibRstLongTimer( &AdatV->GainUpCntr );		/* Clear gain search counter	*/
		rc = ADAT_SEAR_NOVIBDETECT;					/* Vibration not detected Gain UP	*/
	}
	return rc;
}



/****************************************************************************
	Advanced autotuning maximumKpSearch

	Overview:	Kpat regular intervalsMaxKp
			Perform vibration detection and return the status if it is vibrating.
			COINIf the signal is detected, the process ends normally.
****************************************************************************/
static SHORT	LpxAdatKpGainSet( ADATV *AdatV, ADATP *AdatP )
{
	SHORT	rc;
	rc = ADAT_SEAR_CONTINUE;

	/*--------------------------------------------------------------------------*/
	/* Oscillation detection																	*/
	/*--------------------------------------------------------------------------*/
	if( AdatV->f.VibDetect )
	{
		DetVibResetMaxMinSpeedError( AdatP->DetVib );
		rc = ADAT_SEAR_VIBEND;						/* Vibration detection finished				*/
	}
	/*--------------------------------------------------------------------------*/
	/* No vibration																	*/
	/*--------------------------------------------------------------------------*/
	else if( KlibGetLongTimerMs( AdatV->KpGainUpCntr ) > GNUPTM2 )
	{	/* Vibration was not detected for specified time → GainUP	*/
		KlibRstLongTimer( &AdatV->KpGainUpCntr );	/* Clear gain search counter	*/
		rc = ADAT_SEAR_NOVIBDETECT;					/* Vibration not detected GainUP		*/
	}
	/*--------------------------------------------------------------------------*/
	/* When maximum gain search value is reached											*/
	/*--------------------------------------------------------------------------*/
	if( (AdatP->UniPrms->Prm->posgn >= AdatV->MaxKp)/* SearchKpIs it greater than or equal to the maximum value?	*/
			|| (AdatV->f.CoinLatch == TRUE) )		 /* COINSignal detected?	*/
	{
		KlibRstLongTimer( &AdatV->KpGainUpCntr );	/* Clear gain search counter	*/
		rc = ADAT_SEAR_NORMALEND;					/* Normal end					*/
	}
	return rc;
}

/****************************************************************************
	Advanced autotuning optimalKpGain search

	Overview:	By command responseKpFind the optimal value of.
			During positioningCOINCrack DetectionCOINIf crack tolerance exceeds
			During positioningCOINIf the crack is detected and the crack tolerance time is exceeded,
			Kpof3.0[1/s].
			Kpis lowered too muchKvis out of balance, the difference betweenKvLower it and try again.
			at this timeKp,TiisKv.
****************************************************************************/
static SHORT	LpxAdatSearchKp( ADATV *AdatV, ADATP *AdatP )
{
	SHORT	rc;
	rc = ADAT_SEAR_CONTINUE;

	/*--------------------------------------------------------------------------*/
	/* Overshoot error detection												*/
	/*--------------------------------------------------------------------------*/
	if( AdatV->f.OvsErr )
	{	/* Overshoot tolerance exceeded	*/
		BpiRsetMaxOverShoot( AdatP->SettlingTime );	/* Overshoot amount clear	*/
		rc = ADAT_SEAR_COINBREAKEND;				/* COINCrack detection completed			*/
	}
	/*--------------------------------------------------------------------------*/
	/* COINCrack Detection																*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.CoinBreak )
	{	/* COINCrack tolerance exceeded	*/
		BpiRsetCoinOffTime( AdatP->SettlingTime );	/* COINCrack cumulative time clear	*/
		rc = ADAT_SEAR_COINBREAKEND;				/* COINCrack detection completed			*/
	}
	/*--------------------------------------------------------------------------*/
	/* Oscillation detection																	*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.VibDetect )
	{	/* Oscillation detected	*/
		DetVibResetMaxMinSpeedError( AdatP->DetVib );
		rc = ADAT_SEAR_VIBEND;						/* Vibration detection finished				*/
	}
	return rc;
}

/****************************************************************************
	Advanced autotuning model follow-up control adjustment

	Overview:	By command responseKpmFind the maximum value of.
			During positioningCOINDetects cracks andCOINIf "No crack",
			Kpmof3.125%.
****************************************************************************/
static SHORT	LpxAdatAdjustMfcGain( ADATV *AdatV, ADATP *AdatP )
{
	SHORT	rc;
	rc = ADAT_SEAR_CONTINUE;

	/*--------------------------------------------------------------------------*/
	/* Overshoot error detection												*/
	/*--------------------------------------------------------------------------*/
	if( AdatV->f.OvsErr )
	{	/* Overshoot tolerance exceeded	*/
		BpiRsetMaxOverShoot( AdatP->SettlingTime );	/* Overshoot amount clear	*/
		rc = ADAT_SEAR_COINBREAKEND;				/* COINCrack detection completed			*/
	}
	/*--------------------------------------------------------------------------*/
	/* COINCrack Detection																*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.CoinBreak )
	{	/* COINCrack tolerance exceeded	*/
		BpiRsetCoinOffTime( AdatP->SettlingTime );	/* COINCrack cumulative time clear	*/
		rc = ADAT_SEAR_COINBREAKEND;				/* COINCrack detection completed			*/
	}
	/*--------------------------------------------------------------------------*/
	/* Oscillation detection																	*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.VibDetect )
	{	/* Oscillation detected	*/
		DetVibResetMaxMinSpeedError( AdatP->DetVib );
		rc = ADAT_SEAR_VIBEND;						/* Vibration detection finished				*/
	}
	/*--------------------------------------------------------------------------*/
	/* Kpm UP																	*/
	/*--------------------------------------------------------------------------*/
	else if( (AdatV->f.CoinLatch == FALSE) && (AdatV->f.LastCoinLatch == TRUE) )
	{/* COINSignalOFFEdge detection */
		rc = ADAT_SEAR_COINOK;						/* Vibration andCOINNo crack	*/
	}
	return rc;
}

/****************************************************************************
	Advanced autotuning model following controlCOINCheck for cracks

	Overview:	By command responseKpmFind the optimal value of.
			During positioningCOINDetects cracks andCOINIf "Crack",
			Kpmof3.0[1/s].
****************************************************************************/
static SHORT	LpxAdatCheckMfcGain( ADATV *AdatV, ADATP *AdatP )
{
	SHORT	rc;
	rc = ADAT_SEAR_CONTINUE;

	/*--------------------------------------------------------------------------*/
	/* Overshoot error detection												*/
	/*--------------------------------------------------------------------------*/
	if( AdatV->f.OvsErr )
	{	/* Overshoot tolerance exceeded	*/
		BpiRsetMaxOverShoot( AdatP->SettlingTime );	/* Overshoot amount clear	*/
		rc = ADAT_SEAR_COINBREAKEND;				/* COINCrack detection completed			*/
	}
	/*--------------------------------------------------------------------------*/
	/* COINCrack Detection																*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.CoinBreak )
	{	/* COINCrack tolerance exceeded */
		BpiRsetCoinOffTime( AdatP->SettlingTime );	/* COINCrack cumulative time clear	*/
		rc = ADAT_SEAR_COINBREAKEND;				/* COINCrack detection completed			*/
	}
	/*--------------------------------------------------------------------------*/
	/* Oscillation detection																	*/
	/*--------------------------------------------------------------------------*/
	else if( AdatV->f.VibDetect )
	{	/* Oscillation detected	*/
		DetVibResetMaxMinSpeedError( AdatP->DetVib );
		rc = ADAT_SEAR_VIBEND;						/* Vibration detection finished				*/
	}
	return rc;
}

/*****************************************************************************************
	Advanced autotuning operation status creation process

	Overview:	Program operation command using external controllerJOG
			only in case of [/44] [45].
        	Internal/external commands are absorbed by this function and do not affect other modules

			Operation command limit
			.COINStart the next command after checking the signal.COINStart the next command without checking the signal.
			  Tuning sequence does not move after payout
			  COINSignalON100msIn the following cases, vibration detection at stop cannot be performed.
			.No limit on operating speed and acceleration
			  However, the inertia identification and gain search results vary depending on the velocity and acceleration.
			  Affected.

			Operation stop    ...Filter front position command segment=0butCOINAfter latch300[ms]
			Positioning complete...Pre-filter position command segment=0andCOINLatch
			Forward/reverse direction  .Switch forward/reverse rotation at every stop
*****************************************************************************************/
static SHORT LpxAdatRefInTypeDrvStatus( ADATHNDL *AdatHndl, ULONG DrvPattern, BOOL PrefInFlag )
{
	SHORT	rc;
	ADATREG *AdatReg;
	ADATV	*AdatV;
	ADATP	*AdatP;

	AdatReg = &AdatHndl->AdatReg;
	AdatV = &AdatHndl->AdatV;
	AdatP = &AdatHndl->AdatP;
	rc = AdatV->PrgJogStat;					/* Initialize return code to previous status	*/

	switch ( AdatV->PrgJogPtr )
	{
		/*------------------------------------------------------------------------------*/
		/*	Initial process of Jog sequence 											*/
		/*------------------------------------------------------------------------------*/
		case 0:
			if( DrvPattern == ADAT_PJOG_STOP )
			{	/* Request for operation stop														*/
				rc = ADAT_PJOG_NOP;						/* Go to operation stop state				*/
			}
			else
			{
				AdatV->f.DrvPatternSet = TRUE;
				AdatV->PositioningTimes = 0;			/* Positioning number counter clear	*/
				AdatV->PrgJogPtr = 1;					/* Update sequence pointer		*/
				rc = ADAT_PJOG_WAIT;					/* Operation standby						*/
			}
			break;
		/*------------------------------------------------------------------------------*/
		/*	Operation start processing					 											*/
		/*------------------------------------------------------------------------------*/
		case 1:
			if( (PrefInFlag == TRUE) || (AdatP->SeqCtrlOut->TgOnSts == TRUE) )
			{	/* Movement command received															*/
				AdatV->PrgJogPtr = 2;					/* Waiting for movement completion				*/
				rc = ADAT_PJOG_EXEC;
			}
			break;
		/*------------------------------------------------------------------------------*/
		/*	Wait for movement command to complete				 											*/
		/*------------------------------------------------------------------------------*/
		case 2:
			if(( (AdatP->BaseCtrlData->PosCtrlSts.CoinSignal == TRUE)
			  || (AdatP->SeqCtrlOut->MotStop) == TRUE) 	/* COINLatch completionorMotor stop detection */
			  || ((PrefInFlag == FALSE)
			  && (AdatV->f.StopDetectMode2 == TRUE) ) )	/* Stop judgment mode2and command stop	*/
			{	/* Command stop judgment															*/
				AdatV->PrgJogPtr = 3;					/* Waiting for operation completion				*/
			}
		/* Fall Through */	/* At the same timing as "waiting for operation completion"COINTo make a determinationbreakNone*/
		/*------------------------------------------------------------------------------*/
		/*	Waiting for operation completion					 											*/
		/*------------------------------------------------------------------------------*/
		case 3:
			if(( AdatP->BaseCtrlData->PosCtrlSts.CoinSignal == TRUE )/* COINLatch completion */
			 || ((PrefInFlag == FALSE)
			 && (AdatV->f.StopDetectMode2 == TRUE)))	/* Stop judgment mode2and command stop	*/
			{
				/* Positioning completion status set by driving direction								*/
				/* Set forward/reverse rotation every time positioning is completed								*/
				AdatV->PositioningTimes++;				/* Positioning number counter clear	*/
				if( AdatV->PositioningTimes & 0x01 )
				{
					rc = ADAT_PJOG_PLUSEND;
				}
				else
				{
					rc = ADAT_PJOG_MINUSEND;
				}
				AdatV->CoinWaitStat = ADAT_STS_COINNOWAIT;
				AdatV->f.CoinWaitStart = FALSE;				/* COINOutput waiting process finished		*/
				KlibRstLongTimer( &AdatV->PrgJogWaitCntr );
				AdatV->PrgJogPtr = 4;						/* To wait processing			*/
				KlibRstLongTimer( &AdatV->CoinWaitCntr );
			}
			else
			{
				/*----------------------------------------------------------------------*/
				/*	COINOutput waiting process(Position control only)										*/
				/*----------------------------------------------------------------------*/
				if( AdatP->BaseCtrlData->CtrlModeSet.CtrlMode.b.cm == CTRLMODE_POS )
				{	/* Position control mode 													*/
					if( (PrefInFlag == FALSE) &&			/* No command					*/
						 (AdatV->f.CoinWaitStart == FALSE))	/* COINOutput waiting process in progress	*/
					{
						KlibRstLongTimer( &AdatV->CoinWaitCntr );
						AdatV->CoinWaitStat = ADAT_STS_COINWAIT;
						AdatV->f.CoinWaitStart = TRUE;			/* COINStart of output waiting process	*/
					}
					if(AdatV->f.CoinWaitStart == TRUE)			/* COINOutput waiting process in progress	*/
					{
						if( KlibGetLongTimerMs( AdatV->CoinWaitCntr ) >= ADATCOINWAIT1 )
						{
							/* Approx.10When positioning is not completed in secondsCOINWaiting error				*/
							AdatV->ErrState = ADAT_COINERR;
							AdatV->CoinWaitStat = ADAT_STS_COINERR;	/* Error outputON		*/
						}
						else if( KlibGetLongTimerMs( AdatV->CoinWaitCntr ) >= ADATCOINWAIT2 )
						{	/* COIN Output waiting warning	*/
							AdatV->CoinWaitStat = ADAT_STS_COINWARN;/* Warning output			*/
						}
					}
				}
			}
			break;
		/*------------------------------------------------------------------------------*/
		/*	Wait						 											*/
		/*------------------------------------------------------------------------------*/
		case 4:
			if( KlibGetLongTimerMs( AdatV->PrgJogWaitCntr ) >= ADATDRVWAIT )
			{	/* Operation completed																*/
				AdatV->PrgJogPtr = 5;						/* Prepare for next operation			*/
			}
			break;
		/*------------------------------------------------------------------------------*/
		/*	Prepare for next operation					 											*/
		/*------------------------------------------------------------------------------*/
		case 5:
			if( ( AdatV->f.GainTuneRun == TRUE )			/* During tuning			*/
				&& (AdatReg->AdatState != ADAT_STS_VIBDET) )/* Not measuring vibration level	*/
			{	/* During tuning, clear once									*/
				DetVibResetMaxMinSpeedError( AdatP->DetVib );
			}
			if( (AdatV->PositioningTimes & 0x0004) != 0x0000 )
			{	/* 2Complete a round trip. For operation completion, in order to ensure vibration confirmation,2Perform in a round trip.			*/
				AdatV->PositioningTimes = 0;				/* Positioning number counter clear*/
				AdatV->PrgJogPtr = 6;						/* Go to operation stop wait process	*/
				KlibRstLongTimer( &AdatV->PrgJogWaitCntr );	/* Round counter retention		*/
				rc = ADAT_PJOG_FIN;							/* Operation completion status set	*/
			}
			else
			{
				AdatV->PrgJogPtr = 1;						/* Go to operation start processing			*/
				rc = ADAT_PJOG_WAIT;						/* Operation standby					*/
			}
			break;
		/*------------------------------------------------------------------------------*/
		/*	Operation stop						 											*/
		/*------------------------------------------------------------------------------*/
		case 6:
			if( KlibGetLongTimerMs( AdatV->PrgJogWaitCntr ) >= ADATDRVWAIT )
			{	/* Operation completed																*/
				AdatV->PrgJogPtr = 0;		/* Operation stop. Set the sequence to the initial settings	*/
			}
			break;

		default:
			AdatV->ErrState = ADAT_PROGERR;	/* Abnormal termination									*/
			break;
	}
	return rc;
}



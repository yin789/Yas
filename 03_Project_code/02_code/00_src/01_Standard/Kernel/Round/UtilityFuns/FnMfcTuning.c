/****************************************************************************
  Description   : Mfc with Vibration Suppression Tuning (Fn205)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.01.25   created

****************************************************************************/
#include "FnMfcTuning.h"
#include "TuneLib.h"
#include "MercuryGlobals.h"
#include "PnPrmListTbl.h"
#include "KLib.h"
#include "ExControls.h"


/****************************************************************************************************/
/* Model Following Control with Vibration Suppression Tuning (Fn205)								*/
/****************************************************************************************************/
/* -------- Local Function Prototype Definition -------- */
static void		vibMfcInitialize( MFCTUNE *MfcTune, PRMDATA *Prm );
static void		vibMfcTuningCompCheck( MFCTUNE *MfcTune, MFCTRL *MFControl, PRMDATA *Prm );
static PRM_RSLT		vibMfcUpdateGainInEeprom( REGIF_CONFIG_T *RegCfg );//<S0C7>
static void		vibMfcUndoRamGain( REGIF_CONFIG_T *RegCfg );


/****************************************************************************************************/
/*																									*/
/*		Fn205 : Model Following Control with Vibration Suppression Tuning							*/
/*																									*/
/****************************************************************************************************/
/* -------- Fn205 Model Following Control with Vibration Suppression Tuning mode Begin	-------- */
PRM_RSLT	FnEnterMfcTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	MFCTUNE			*MfcTune;
	BASE_CTRL_HNDL	*pBaseCtrl;
	PRMDATA			*Prm;
	PRM_RSLT		rc;

	rc = PRM_RSLT_SUCCESS;
	MfcTune = Axis->MfcTune;
	pBaseCtrl = Axis->BaseControls;
	Prm = Axis->UniPrms->Prm;

	/* Initialize Register Data */
	vibMfcInitialize( MfcTune, Prm );

	if( ((pBaseCtrl->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS) && (Axis->SeqCtrlOut->BeComplete == TRUE))
		|| (TuneLessGetTuneLessSts(&Axis->BaseControls->TuneLessCtrl) == TRUE) )
	{ /* (Other than position control && While the servo is on) || Adjustmentless enabled */
		rc = PRM_RSLT_CONDITION_ERR;
	}

	/* If in motorless test mode, exit without entering *//* <S1B7> */
	if( Axis->MotorLessTest->MotorLessTestMode == TRUE )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	MfcTune->OpeReg.MdlVibState = VIBMFC_STS_COMP;

	/* Read the Current Value from Ram */
//	RpiPrmRead( &pndef_mdlgn,     &data  );
//	MfcTune->var.OrgMdlgn = (USHORT)data;
//	RpiPrmRead( &pndef_mdlantfrq, &data );
//	MfcTune->var.OrgAntFrq = (USHORT)data;
//	RpiPrmRead( &pndef_mdlresfrq, &data );
//	MfcTune->var.OrgResFrq = (USHORT)data;
//	RpiPrmRead( &pndef_mdlsw,     &data  );
//	MfcTune->var.OrgMdlSw = (USHORT)data;
	/* Pn140 Model following control related switches */
	MfcTune->var.OrgMdlSw	= Prm->mdlsw;
	/* Pn141 Model following control gain[0.1/s] */
	MfcTune->var.OrgMdlgn	= Prm->mdlgn;
	/* Pn145 Vibration suppression 1 frequency A */
	MfcTune->var.OrgAntFrq	= Prm->mdlantfrq;
	/* Pn146 Vibration suppression 1 frequency B */
	MfcTune->var.OrgResFrq	= Prm->mdlresfrq;

	/* FnMode automatic end */
	FunExe->HoldFnMode = TRUE;

	return( rc );
}


/* -------- Fn205 Model Following Control with Vibration Suppression Tuning mode end	-------- */
void	FnLeaveMfcTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	MFCTUNE			*MfcTune;
	REGIF_CONFIG_T	*RegCfg;

	MfcTune = Axis->MfcTune;
	RegCfg = Axis->RegManager->hPnReg;

	/* EEPROMRevert changed parameters with data						*/
	vibMfcUndoRamGain( RegCfg );
	MlibResetLongMemory( &(MfcTune->var), sizeof(MfcTune->var)/4 );

	/* FnMode automatic end setting */
	FunExe->HoldFnMode = FALSE;

	return;
}


/* -------- Fn205 Model Following Control with Vibration Suppression Tuning mode execution	-------- */
PRM_RSLT	FnExecuteMfcTuning( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT		rc;
	USHORT			uswork1;
	USHORT			uswork2;
	REG_ACC_T		AccessPrm;
	MFCTUNE			*MfcTune;
	MFCTRL			*MFControl;
	PRMDATA			*Prm;
	REGIF_CONFIG_T	*RegCfg;

	MfcTune = Axis->MfcTune;
	MFControl = &(Axis->BaseControls->MFControl);
	Prm = Axis->UniPrms->Prm;
	RegCfg = Axis->RegManager->hPnReg;
	rc = PRM_RSLT_SUCCESS;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr = 0;
	AccessPrm.hRegCfg = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	switch( FunExe->State )
	{
	case VIBMFC_SEQ_INIT:
		FunExe->State = VIBMFC_SEQ_ADJUST;
		break;

	case VIBMFC_SEQ_ADJUST:
		switch( FunExe->CmdCode )
		{
		case FCMD_EXEC:
			FunExe->State = VIBMFC_SEQ_SETREQ;
			MfcTune->OpeReg.MdlVibState = VIBMFC_STS_COMP;
			rc = PRM_RSLT_CONTINUE;
			break;

		case FCMD_EWRT:
//			vibMfcUpdateGainInEeprom( RegCfg );
			rc = vibMfcUpdateGainInEeprom( RegCfg );		/* <S085> */
			break;

		case FCMD_NONE:
			break;

		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}
		vibMfcTuningCompCheck( MfcTune, MFControl, Prm );
		break;

	case VIBMFC_SEQ_SETREQ:
		/* Read the Current Value from Ram */
//		RpiPrmRead( &pndef_mdlgn,     &data );
//		MfcTune->var.OrgMdlgn = (USHORT)data;
//		RpiPrmRead( &pndef_mdlantfrq, &data );
//		MfcTune->var.OrgAntFrq = (USHORT)data;
//		RpiPrmRead( &pndef_mdlresfrq, &data );
//		MfcTune->var.OrgResFrq = (USHORT)data;
//		RpiPrmRead( &pndef_mdlsw,     &data );
//		MfcTune->var.OrgMdlSw = (USHORT)data;
		/* Pn140 Model following control related switches */
		MfcTune->var.OrgMdlSw	= Prm->mdlsw;
		/* Pn141 Model following control gain[0.1/s] */
		MfcTune->var.OrgMdlgn	= Prm->mdlgn;
		/* Pn145 Vibration suppression 1 frequency A */
		MfcTune->var.OrgAntFrq	= Prm->mdlantfrq;
		/* Pn146 Vibration suppression 1 frequency B */
		MfcTune->var.OrgResFrq	= Prm->mdlresfrq;

		if( MFControl->conf.MFCModelSetting )
		{
			uswork1 = TuneMfcKpmCalcAntFreq( MfcTune->OpeReg.MdlVibFreq );
			if( MFControl->conf.MFCModelSetting == VIBMFC_ONE_MASS )
			{
				uswork2 = Prm->posgn / 2;
				if( uswork2 > uswork1 )
				{
					uswork2 = uswork1;
				}
//				if( uswork2 < RpiPrmLowerLimit(&pndef_mdlgn) )
//				{
//					uswork2 = (USHORT)(RpiPrmLowerLimit(&pndef_mdlgn));
//				}
//				else if( uswork2 > RpiPrmUpperLimit(&pndef_mdlgn) )
//				{
//					uswork2 = (USHORT)(RpiPrmUpperLimit(&pndef_mdlgn));
//				}
				uswork2 = TunePrmLimitUnsigned( uswork2, &pndef_mdlgn );
				/* uswork1anduswork2uswork1 */
				uswork1 = uswork2;
			}
			if( MfcTune->var.OrgMdlgn < uswork1 )
			{
				uswork2 = MfcTune->var.OrgMdlgn;
			}
			else
			{
				uswork2 = uswork1;
			}
			Prm->mdlgn = uswork2;
			Prm->mdlantfrq = MfcTune->OpeReg.MdlVibFreq;
			Prm->mdlresfrq = MfcTune->OpeReg.MdlVibFreq;
//			lrc = RpiPrmWriteRamForFn( &pndef_mdlsw, VIBMFC_PRM_ACTIVE, pfexe->App.Hmsg );
			TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, VIBMFC_PRM_ACTIVE, 2 );
			FunExe->State = VIBMFC_SEQ_WAITMFC;
		}
		else
		{
			uswork1 = TuneMfcKpmCalcAntFreq( MfcTune->OpeReg.MdlVibFreq );
			uswork2 = Prm->posgn / 2;
			if( uswork2 > uswork1 )
			{
				uswork2 = uswork1;
			}
//			if( uswork2 < RpiPrmLowerLimit(&pndef_mdlgn) )
//			{
//				uswork2 = (USHORT)(RpiPrmLowerLimit(&pndef_mdlgn));
//			}
//			else if( uswork2 > RpiPrmUpperLimit(&pndef_mdlgn) )
//			{
//				uswork2 = (USHORT)(RpiPrmUpperLimit(&pndef_mdlgn));
//			}
			uswork2 = TunePrmLimitUnsigned( uswork2, &pndef_mdlgn );
			if( uswork1 < uswork2 )
			{
				uswork2 = uswork1;
			}
			Prm->mdlgn = uswork2;
			Prm->mdlantfrq = MfcTune->OpeReg.MdlVibFreq;
			Prm->mdlresfrq = MfcTune->OpeReg.MdlVibFreq;
//			lrc = RpiPrmWriteRamForFn( &pndef_mdlsw, VIBMFC_PRM_ACTIVE, pfexe->App.Hmsg );
			TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, VIBMFC_PRM_ACTIVE, 2 );
			FunExe->State = VIBMFC_SEQ_WAITNORM;
		}
		MfcTune->OpeReg.MdlVibState = VIBMFC_STS_WAIT;
		KlibRstLongTimer( &MfcTune->var.FfWaitCntr );
		rc = PRM_RSLT_CONTINUE;
		break;

	case VIBMFC_SEQ_WAITMFC:
		if( (MFControl->conf.MfcPexe.MFCModel == MFControl->conf.MfcPrm[0].MFCModel)
			&& (MFControl->conf.MfcPexe.Kj2 == MFControl->conf.MfcPrm[0].Kj2)
			&& (MFControl->conf.MfcPexe.Kf2 == MFControl->conf.MfcPrm[0].Kf2) )
		{ /* Execution model, Execution model frequency, Execution model gain is the same as the parameter */
			MfcTune->OpeReg.MdlVibState = VIBMFC_STS_COMP;
			FunExe->State = VIBMFC_SEQ_SETCOMP;
		}
		else
		{
			if( KlibGetLongTimerMs( MfcTune->var.FfWaitCntr ) > VIBMFC_FF_WAITTMOUT )
			{
				MfcTune->OpeReg.MdlVibState = VIBMFC_STS_TMOUT;

				Prm->mdlgn = MfcTune->var.OrgMdlgn;
				Prm->mdlantfrq = MfcTune->var.OrgAntFrq;
				Prm->mdlresfrq = MfcTune->var.OrgResFrq;
				RpiRegSetVibMfcTuningFrequency( MfcTune, Prm->mdlantfrq );
//				lrc = RpiPrmWriteRamForFn( &pndef_mdlsw, MfcTune->var.OrgMdlSw, pfexe->App.Hmsg );
				TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, MfcTune->var.OrgMdlSw, 2 );
				FunExe->State = VIBMFC_SEQ_SETCOMP;
			}
			else
			{
				rc = PRM_RSLT_CONTINUE;
			}
		}

		switch( FunExe->CmdCode )
		{
		case FCMD_NONE:
			break;

		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}
		break;

	case VIBMFC_SEQ_WAITNORM:
		if( (MFControl->conf.MfcPexe.MFCModel == MFControl->conf.MfcPrm[0].MFCModel)
			&& (MFControl->conf.MfcPexe.Kj2 == MFControl->conf.MfcPrm[0].Kj2)
			&& (MFControl->conf.MfcPexe.Kf2 == MFControl->conf.MfcPrm[0].Kf2) )
		{ /* Execution model, Execution model frequency, Execution model gain is the same as the parameter */
			MfcTune->OpeReg.MdlVibState = VIBMFC_STS_COMP;
			FunExe->State = VIBMFC_SEQ_SETCOMP;
		}
		else
		{
			if( KlibGetLongTimerMs( MfcTune->var.FfWaitCntr ) > VIBMFC_FF_WAITTMOUT )
			{
				MfcTune->OpeReg.MdlVibState = VIBMFC_STS_TMOUT;
				Prm->mdlgn = MfcTune->var.OrgMdlgn;
				Prm->mdlantfrq = MfcTune->var.OrgAntFrq;
				Prm->mdlresfrq = MfcTune->var.OrgResFrq;
				RpiRegSetVibMfcTuningFrequency( MfcTune, Prm->mdlantfrq );
//				lrc = RpiPrmWriteRamForFn( &pndef_mdlsw, MfcTune->var.OrgMdlSw, pfexe->App.Hmsg );
				TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, MfcTune->var.OrgMdlSw, 2 );
				FunExe->State = VIBMFC_SEQ_SETCOMP;
			}
			else
			{
				rc = PRM_RSLT_CONTINUE;
			}
		}

		switch( FunExe->CmdCode )
		{
		case FCMD_NONE:
			break;

		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}
		break;

	case VIBMFC_SEQ_SETCOMP:
		MfcTune->var.DispMatch = TRUE;
		FunExe->State = VIBMFC_SEQ_ADJUST;
		break;

	case VIBMFC_SEQ_END:	/* Termination processing */
		FnLeaveMfcTuning( FunExe, Axis );
		break;

	case VIBMFC_SEQ_NOOPWAIT:
		FunExe->State = VIBMFC_SEQ_END;
		break;

	default:
		break;
	}
	return ( rc );
}

/* -------- Fn205 Model Following Control with Vibration Suppression Tuning Initialize Data	-------- */
static void	vibMfcInitialize( MFCTUNE *MfcTune, PRMDATA *Prm )
{
	RpiRegSetVibMfcTuningFrequency( MfcTune, Prm->mdlantfrq );
	RpiRegSetMdlVibFreq2( MfcTune, Prm->ff_frq );
	RpiRegSetMdlVibFil2( MfcTune, Prm->ff_fil );
	return;
}

/* -------- Fn205 Model Following Control with Vibration Suppression Tuning Check	-------- */
static void	vibMfcTuningCompCheck( MFCTUNE *MfcTune, MFCTRL *MFControl, PRMDATA *Prm )
{
	BOOL	MfcPrmEnable;

	if( (MFControl->conf.MfcPexe.MFCModel == MFControl->conf.MfcPrm[0].MFCModel)
		&& (MFControl->conf.MfcPexe.Kj2 == MFControl->conf.MfcPrm[0].Kj2)
		&& (MFControl->conf.MfcPexe.Kf2 == MFControl->conf.MfcPrm[0].Kf2) )
	{ /* Execution model, Execution model frequency, Execution model gain is the same as the parameter */
		MfcPrmEnable = TRUE;
	}
	else
	{
		MfcPrmEnable = FALSE;
	}

	if( (MFControl->conf.MFCModelSetting == VIBMFC_ONE_BASE)
		&& (MfcPrmEnable == TRUE)
		&& (MfcTune->OpeReg.MdlVibFreq == Prm->mdlantfrq)
		&& (MfcTune->OpeReg.MdlVibFreq == Prm->mdlresfrq) )
	{
		MfcTune->var.DispMatch = TRUE;
	}
	else
	{
		MfcTune->var.DispMatch = FALSE;
	}
	return;
}



/* -------- Fn205 Model Following Control with Vibration Suppression Tuning Gain In EEPROM	-------- */
static PRM_RSLT	 vibMfcUpdateGainInEeprom( REGIF_CONFIG_T *RegCfg )//<S0C7>
{
	PRM_RSLT rc;

	/* Pn140 Model following control related switches */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlsw, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn141 Model following control gain */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlgn, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn145 Vibration suppression 1 frequency A */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlantfrq, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn146 Vibration suppression 1 frequency A */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_mdlresfrq, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	return( rc );
}



/* -------- Fn205 Write Gain To Ram With EEPROM Data	-------- */
static void	vibMfcUndoRamGain( REGIF_CONFIG_T *RegCfg )
{
	/* RAMthe value ofEEPROMbe the same as the value of */
	/* Pn140 Model following control related switches */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlsw, 0 );
	/* Pn141 Model following control gain */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlgn, 0 );
	/* Pn145 Vibration suppression 1 frequency A */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlantfrq, 0 );
	/* Pn146 Vibration suppression 1 frequency B */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_mdlresfrq, 0 );

	return;
}

/****************************************************************************************************/
/*		With vibration suppressionMFCAdjustment frequency setting process (RegisterIF)												*/
/****************************************************************************************************/
PRM_RSLT	RpiMfcTuningSetFreq( MFCTUNE *MfcTune, USHORT Frq )
{
	PRM_RSLT	rc;
	USHORT		LimitVal;

	rc = PRM_RSLT_SUCCESS;
	LimitVal = TunePrmLimitUnsigned( (ULONG)Frq, &pndef_mdlantfrq );

	if( Frq == LimitVal )
	{
		MfcTune->OpeReg.MdlVibFreq = Frq;
	}
	else
	{
		rc = PRM_RSLT_LIMIT_ERR;
	}
	return( rc );
}

/****************************************************************************************************/
/*		With vibration suppressionMFCAdjustment frequency2Setting processing (RegisterIF)												*/
/****************************************************************************************************/
PRM_RSLT	RpiMfcTuningSetFreq2( MFCTUNE *MfcTune, USHORT Frq )
{
	PRM_RSLT	rc;
	USHORT		LimitVal;

	rc = PRM_RSLT_SUCCESS;
	LimitVal = TunePrmLimitUnsigned( (ULONG)Frq, &pndef_ff_frq );

	if( Frq == LimitVal )
	{
		MfcTune->OpeReg.MdlVibFreq2 = Frq;
	}
	else
	{
		rc = PRM_RSLT_LIMIT_ERR;
	}
	return( rc );
}

/****************************************************************************************************/
/*		With vibration suppressionMFCAdjustment frequency2Correction setting processing (RegisterIF)											*/
/****************************************************************************************************/
PRM_RSLT	RpiMfcTuningSetFreq2Comp( MFCTUNE *MfcTune, USHORT Fil )
{
	PRM_RSLT	rc;
	USHORT		LimitVal;

	rc = PRM_RSLT_SUCCESS;
	LimitVal = TunePrmLimitUnsigned( (ULONG)Fil, &pndef_ff_fil );

	if( Fil == LimitVal )
	{
		MfcTune->OpeReg.MdlVibFil2 = Fil;
	}
	else
	{
		rc = PRM_RSLT_LIMIT_ERR;
	}
	return( rc );
}

/****************************************************************************************************/
/*		With vibration suppressionMFCAdjustment frequency setting(Pn145)Processing 												*/
/****************************************************************************************************/
void	RpiRegSetVibMfcTuningFrequency( MFCTUNE *MfcTune, USHORT Frq )
{
	MfcTune->OpeReg.MdlVibFreq = Frq;
	return;
}

/****************************************************************************************************/
/*		Vibration suppression function tuning frequency2Setting processing(Pn14A)												*/
/****************************************************************************************************/
void	RpiRegSetMdlVibFreq2( MFCTUNE *MfcTune, USHORT Frq2 )
{
	MfcTune->OpeReg.MdlVibFreq2 = Frq2;
	return;
}

/****************************************************************************************************/
/*		Vibration suppression function tuning frequency2Correction setting processing(Pn14B)											*/
/****************************************************************************************************/
void	RpiRegSetMdlVibFil2( MFCTUNE *MfcTune, USHORT Fil )
{
	MfcTune->OpeReg.MdlVibFil2 = Fil;
	return;
}



/****************************************************************************************************/

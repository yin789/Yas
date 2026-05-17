/****************************************************************************
  Description   : Easy FFT Measurement (Fn206)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.02.17   created
		EasyFFT, trace and table operations are used internally. For this reason, in
		In this module, only trace and table operation registers are directly referenced.
		Rewriting will be performed.
****************************************************************************/
#include "FnEasyFFT.h"
#include "FnTableCommand.h"
#include "MercuryGlobals.h"
#include "BaseControls.h"
#include "DataTraceManager.h"
#include "TableDrive.h"
#include "AlarmManager.h"
#include "TuneLib.h"
#include "PnPrmListTbl.h"
#include "MLib.h"
#include "ExControls.h"


/****************************************************************************************************/
/* Local Variable Definition																		*/
/****************************************************************************************************/
static	TBLDRIVE	OrgTblDrive;



/****************************************************************************************************/
/* Easy FFT Measurement (Fn206)																		*/
/****************************************************************************************************/
/* -------- Local Function Prototype Definition -------- */
static PRM_RSLT	easyFftSetTuningGain( EASYFFT *EasyFFT, REG_ACC_T *AccessPrm );
static PRM_RSLT	easyFftUpdateGainInEeprom( REGIF_CONFIG_T *RegCfg );
static void		easyFftUndoRamGain( REGIF_CONFIG_T *RegCfg );
static void		easyFftSetPrmBackup( EASYFFT *EasyFFT );
static void		easyFftUndoPrmBackup( EASYFFT *EasyFFT );
static void		easyFftSetTraceRegister( TRCREG *TrcReg, LONG ax, USHORT ezfftsw );
static void		easyFftSetTblCmdRegister( TBLDRIVE *TblDrive );
static void		easyFftClearCounter( EASYFFT *EasyFFT );
static void		easyFftSetRefCommand( EASYFFT *EasyFFT, LONG SvCycleUs, USHORT PerMaxTrq );
static void		easyFftTuneRegClear( EASYFFT *EasyFFT );

/* -------- Local Function (for Sequence) Prototype Definition -------- */
static PRM_RSLT	easyFftSequence( EASYFFT *EasyFFT, TBLDRIVE *TblDrive, TRCREG *TrcReg, BPRMDAT *Bprm );
static void		easyFftMakeChirpRefTrace( EASYFFT *EasyFFT );
static ULONG	easyFftGetChirpFreq( EASYFFT *EasyFFT, USHORT count, LONG ObjFlag );
static void		easyFftGetFreqGainMax( EASYFFT *EasyFFT, TRCREG *TrcReg );
static void		easyFftCalcHpFilter( EASYFFT *EasyFFT, TRCREG *TrcReg );
static USHORT	easyFftGetAvrgFreq( ULONG frgavr, ULONG freq );
static void		easyFftSetZoomAnalysis( EASYFFT *EasyFFT );



/****************************************************************************************************/
/*																									*/
/*		Easy FFT Measurement (Fn206)																*/
/*																									*/
/****************************************************************************************************/
/* -------- Fn206 : Easy FFT Begin	-------*/
PRM_RSLT	FnEnterEasyFft( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT			rc;
	EASYFFT				*EasyFFT;
	PRMDATA				*Prm;
	FUN_CMN_CONTROL		*FnCmnCtrl;

	EasyFFT = Axis->EasyFFT;
	Prm = Axis->UniPrms->Prm;
	FnCmnCtrl = Axis->FnCmnCtrl;
	rc = PRM_RSLT_SUCCESS;

	/* Initialize Easy FFT */
	EasyFFT->OpeReg.EzFftTrq = Prm->ezffttrq;

	/* Ends without turning on when servo is on */
	if( TRUE == Axis->SeqCtrlOut->BaseEnableReq )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* If in motorless test mode, exit without entering *//* <S1B7> */
	if( Axis->MotorLessTest->MotorLessTestMode == TRUE )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* FnServo operation privilege acquisition */
	FnCmnCtrl->FnSvControl = TRUE;
	FnCmnCtrl->FnCtrlMcmd = CTRLMODE_NOCMD;
	FnCmnCtrl->FnSensOnReq = TRUE;
	FnCmnCtrl->JogSpeed = 0;

	/* Save Original Table Drive Data */
	MlibCopyLongMemory( &OrgTblDrive,
						&(Axis->BaseControls->TblDrive),
						sizeof(Axis->BaseControls->TblDrive)/4 );

	/* Reset Easy FFT Variables */
	MlibResetLongMemory( &(EasyFFT->var), sizeof(EasyFFT->var)/4 );

	/* Forced invalidation request without adjustment */
	TuneLessSetForceInvldReq( &(Axis->BaseControls->TuneLessCtrl) );

	/* OTSignal mask setting */
	SeqIoSetOtSigMask( Axis->IoSignals );

//	RpiPrmRead( &pndef_tfuncsw, &data );
	EasyFFT->var.NotchSw = Prm->tfuncsw;
//	RpiPrmRead( &pndef_ntchfil1, &data );
	EasyFFT->OpeReg.EzFftFil1 = Prm->ntchfil1;
//	RpiPrmRead( &pndef_ntchfil2, &data );
	EasyFFT->OpeReg.EzFftFil2 = Prm->ntchfil2;
//	RpiPrmRead( &pndef_swpminfrq, &data );
	EasyFFT->var.FrqSwpMin = Prm->swpminfrq;
//	RpiPrmRead( &pndef_swpmaxfrq, &data );
	EasyFFT->var.FrqSwpMax = Prm->swpmaxfrq;

//	RpiPrmRead( &pndef_detfrqlmt, &data );
	EasyFFT->var.FrqLowerLmt = Prm->detfrqlmt;

	/* Setting information backup */
	easyFftSetPrmBackup( EasyFFT );

	/* Register settings(EasyFFTStart) */
	EasyFFT->OpeReg.EzFftState = EZFFT_STS_END;

	/* FnMode automatic end */
	FunExe->HoldFnMode = TRUE;

	return rc;
}


/* -------- Fn206 : Easy FFT End	-------*/
void	FnLeaveEasyFft( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	REGIF_CONFIG_T		*RegCfg;
	FUN_CMN_CONTROL		*FnCmnCtrl;

	RegCfg = Axis->RegManager->hPnReg;
	FnCmnCtrl = Axis->FnCmnCtrl;

	/* EEPROMRevert changed parameters with data */
	easyFftUndoRamGain( RegCfg );

	/* OTSignal mask cancellation */
	SeqIoClearOtSigMask( Axis->IoSignals,
	                       Axis->UniPrms->Prm->pot_selsw,
	                       Axis->UniPrms->Prm->not_selsw );

	/* Restore Original Table Drive Data */
	MlibCopyLongMemory( &(Axis->BaseControls->TblDrive),
						&OrgTblDrive,
						sizeof(Axis->BaseControls->TblDrive)/4 );

	/* Wait until servo is turned off */
	FnCmnCtrl->FnSvonReq = FALSE;
	while( TRUE == Axis->SeqCtrlOut->BaseEnableReq )
	{
		;
	}

	/* Release of forced invalidation request without adjustment */
	TuneLessClearForceInvldReq( &Axis->BaseControls->TuneLessCtrl );

	/* Table operation stop */
	TblDriveSetStopRequest( &(Axis->BaseControls->TblDrive) );

	/* Table operation simultaneous trace execution request cancellation */
	/* 2011.02.28 Y.Oka Forcibly interlock trace during table operation */
//	TblDriveSetWithTrcReq( &(Axis->BaseControls->TblDrive), FALSE );

	/* FnServo operation right release */
	FnCmnCtrl->JogSpeed = 0;
	FnCmnCtrl->FnCtrlMcmd = CTRLMODE_NOCMD;
	FnCmnCtrl->FnSensOnReq = FALSE;
	FnCmnCtrl->FnSvControl = FALSE;

	return;
}


/* -------- Fn206 : Easy FFT Execution	-------*/
PRM_RSLT	FnExecuteEasyFft( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT			rc;
	REG_ACC_T			AccessPrm;
	EASYFFT				*EasyFFT;
	TBLDRIVE			*TblDrive;
	TRCREG				*TrcReg;
	PRMDATA				*Prm;
	BPRMDAT				*Bprm;
	REGIF_CONFIG_T		*RegCfg;
	FUN_CMN_CONTROL		*FnCmnCtrl;
	ULONG	i;					/* <S0C9> */
	TRCINDIVPRM	*TrcIncivPrm;	/* <S0C9> */

	EasyFFT = Axis->EasyFFT;
	TblDrive = &(Axis->BaseControls->TblDrive);
	TrcReg = &(Axis->TrcHndl->TrcReg);
	Prm = Axis->UniPrms->Prm;
	Bprm = Axis->UniPrms->Bprm;
	RegCfg = Axis->RegManager->hPnReg;
	FnCmnCtrl = Axis->FnCmnCtrl;
	rc = PRM_RSLT_SUCCESS;
	TrcIncivPrm =  &(Axis->TrcHndl->TrcIncivPrm);	/* <S0C9> */


	/* RAMWrite register information setting */
	AccessPrm.MemAddr = 0;
	AccessPrm.hRegCfg = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	rc = PRM_RSLT_SUCCESS;

	switch( FunExe->State )
	{
	case EZFFT_SEQ_INIT:
		/* Data trace & Table operation execution register calculation processing */
		easyFftSetTraceRegister( TrcReg, Axis->AxisNo, Prm->ezfftsw );

/* <S0C9>	Start */
	for( i=0; i < TRC_CH_MAXNUM; i++ )
	{
		TrcIncivPrm->TrcData[i] = TrcReg->TrcDataSel[i];		/* Selection of data to be traced(1~16)			*/
		TrcIncivPrm->TrcAxis[i] = TrcReg->TrcAxisSel[i];		/* Data trace target axis selection(1~16)				*/
		TrcIncivPrm->IOTrcData[i] = TrcReg->IOTrcDataSel[i];	/* I/OTrace1Target data selection(1~16)			*/
		TrcIncivPrm->IOTrcAxis[i] = TrcReg->IOTrcAxisSel[i];	/* I/OTrace1Target axis selection(1~16)				*/
	}
/* <S0C9>	End */

		easyFftSetTblCmdRegister( TblDrive );
		DtrcCalcParameter( Axis->TrcHndl, Axis->TraceBuffer );
		TblDriveGetTraceRegData( TblDrive,
								 TrcReg->DataSize,
								 TrcReg->SampIntv,
								 DtrcGetSampIntv(Axis->TrcHndl),
								 DtrcGetTrcDataNum(Axis->TrcHndl) );
		TblDriveCalculateParameter( TblDrive, Bprm->XfbPulse1 );

		/* Table operation simultaneous trace execution request */
		/* 2011.02.28 Y.Oka Forcibly interlock trace during table operation */
//		TblDriveSetWithTrcReq( TblDrive, TRUE );

		DetVibResetMaxMinSpeedErrorFn( Axis->DetVib );
		FunExe->State = EZFFT_SEQ_READY;
		
	
		/* Table operation start */
		rc = TblDrivePrepareProc( TblDrive );
		if( rc == PRM_RSLT_SUCCESS )
		{
			/* Control mode setting */
			FnCmnCtrl->FnCtrlMcmd = TblDriveGetControlMode( TblDrive );
		}
		else
		{
			/* To termination processing */
			FunExe->State = EZFFT_SEQ_END;
		}
		break;

	case EZFFT_SEQ_READY:
		switch( FunExe->CmdCode )
		{
		case FCMD_SV:
			if( ((Axis->AlmManager)->Status.AlmFlag != FALSE)		/* <S0E5> */ /* <S0FF> */
				|| (Axis->SeqCtrlOut->HwbbSts == TRUE)
				|| (Axis->SeqCtrlOut->MainPowerOn == FALSE) )
			{ /* Alarm occurrence || HWBB || Main circuitOFF */
				rc = PRM_RSLT_CONDITION_ERR;
			}
			else
			{
				FnCmnCtrl->FnSvonReq = (~FnCmnCtrl->FnSvonReq) & 0x01;
			}
			break;

		case FCMD_EWRT:
			easyFftSetTuningGain( EasyFFT, &AccessPrm );
			easyFftUpdateGainInEeprom( RegCfg );
			break;

		case FCMD_UP:
		case FCMD_DOWN:
		case FCMD_NONE:
			break;

		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}
		/* Processing state transition processing when the servo is on */
		if( Axis->SeqCtrlOut->BaseEnableReq == TRUE )
		{
			FunExe->State = EZFFT_SEQ_DRIVE;
		}
		break;

	case EZFFT_SEQ_DRIVE:
		switch( FunExe->CmdCode )
		{
		case FCMD_SV:
			FnCmnCtrl->FnSvonReq = FALSE;
			break;

		case FCMD_UP:
			easyFftTuneRegClear( EasyFFT );
			easyFftSetRefCommand( EasyFFT, Bprm->SvCycleUs, Bprm->PerMaxTrq );
			rc = TblDriveStartRequest( TblDrive, TBLCMD_DRV_FOR );
			if( rc == PRM_RSLT_SUCCESS )
			{
				EasyFFT->var.DrvDir = EZFFT_DRV_FORWARD;
				EasyFFT->var.PosDrvCount++;
				EasyFFT->OpeReg.EzFftState = EZFFT_STS_RUN;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;

				/* Table operation processing */
				FunExe->State = EZFFT_SEQ_RESULT;
				rc = PRM_RSLT_CONTINUE;
			}
			break;

		case FCMD_DOWN:
			easyFftTuneRegClear( EasyFFT );
			easyFftSetRefCommand( EasyFFT, Bprm->SvCycleUs, Bprm->PerMaxTrq );
			rc = TblDriveStartRequest( TblDrive, TBLCMD_DRV_REV );
			if( rc == PRM_RSLT_SUCCESS )
			{
				EasyFFT->var.DrvDir = EZFFT_DRV_REVERSE;
				EasyFFT->var.NegDrvCount++;
				EasyFFT->OpeReg.EzFftState = EZFFT_STS_RUN;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;

				/* Table operation processing */
				FunExe->State = EZFFT_SEQ_RESULT;
				rc = PRM_RSLT_CONTINUE;
			}
			break;

		case FCMD_NONE:
			break;

		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}

		/* Processing state transition processing when servo is off */
		if( Axis->SeqCtrlOut->BaseEnableReq == FALSE )
		{
			FunExe->State = EZFFT_SEQ_READY;
		}
		break;

	case EZFFT_SEQ_RESULT:			/* EasyFFTProcessing */
		rc = easyFftSequence( EasyFFT, TblDrive, TrcReg, Bprm );
		if( EasyFFT->OpeReg.EzFftState != EZFFT_STS_RUN )
		{
			if( EasyFFT->OpeReg.EzFftState == EZFFT_STS_END )
			{
				switch( Prm->tfuncsw & 0x0F0F )
				{
				case 0x0000:	/* If notch filter is not used		*/
				case 0x0100:	/* 2When using the 3rd stage notch filter	*/
					EasyFFT->OpeReg.EzFftFil1 = EasyFFT->OpeReg.EzFftDetf;
					EasyFFT->OpeReg.EzFftFil1 = TunePrmLimitUnsigned( EasyFFT->OpeReg.EzFftFil1, &pndef_ntchfil1 );
					break;

				case 0x0001:	/* 1When using the 3rd stage notch filter	*/
					EasyFFT->OpeReg.EzFftFil2 = EasyFFT->OpeReg.EzFftDetf;
					EasyFFT->OpeReg.EzFftFil2 = TunePrmLimitUnsigned( EasyFFT->OpeReg.EzFftFil2, &pndef_ntchfil2 );
					break;

				default:
					break;
				}
			}
			easyFftClearCounter( EasyFFT );

			switch( FunExe->CmdCode )
			{
			case FCMD_UP:
				easyFftTuneRegClear( EasyFFT );
				easyFftSetRefCommand( EasyFFT, Bprm->SvCycleUs, Bprm->PerMaxTrq );
				rc = TblDriveStartRequest( TblDrive, TBLCMD_DRV_FOR );
				if( rc == PRM_RSLT_SUCCESS )
				{
					EasyFFT->var.DrvDir = EZFFT_DRV_FORWARD;
					EasyFFT->var.PosDrvCount++;
					EasyFFT->OpeReg.EzFftState = EZFFT_STS_RUN;
					EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;
					rc = PRM_RSLT_CONTINUE;
				}
				break;

			case FCMD_DOWN:
				easyFftTuneRegClear( EasyFFT );
				easyFftSetRefCommand( EasyFFT, Bprm->SvCycleUs, Bprm->PerMaxTrq );
				rc = TblDriveStartRequest( TblDrive, TBLCMD_DRV_REV );
				if( rc == PRM_RSLT_SUCCESS )
				{
					EasyFFT->var.DrvDir = EZFFT_DRV_REVERSE;
					EasyFFT->var.NegDrvCount++;
					EasyFFT->OpeReg.EzFftState = EZFFT_STS_RUN;
					EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;
					rc = PRM_RSLT_CONTINUE;
				}
				break;

			case FCMD_EWRT:
				easyFftSetTuningGain( EasyFFT, &AccessPrm );
//				easyFftUpdateGainInEeprom( RegCfg );
				rc = easyFftUpdateGainInEeprom( RegCfg );		/* <S085> */
				FunExe->State = EZFFT_SEQ_READY;
				break;

			default:
				break;
			}
		}
		else
		{
			DetVibResetMaxMinSpeedErrorFn( Axis->DetVib );
		}

		switch( FunExe->CmdCode )
		{
		case FCMD_STOP:
			easyFftClearCounter( EasyFFT );
			easyFftTuneRegClear( EasyFFT );
			easyFftSetRefCommand( EasyFFT, Bprm->SvCycleUs, Bprm->PerMaxTrq );
			EasyFFT->OpeReg.EzFftState = EZFFT_STS_END;
			FunExe->State = EZFFT_SEQ_DRIVE;
			break;

		case FCMD_SV:
			FnCmnCtrl->FnSvonReq = (~FnCmnCtrl->FnSvonReq) & 0x01;
			EasyFFT->OpeReg.EzFftState = EZFFT_STS_END;
			break;

		case FCMD_NONE:
		case FCMD_UP:
		case FCMD_DOWN:
		case FCMD_EWRT:
			break;
		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}
		/* Processing state transition processing when servo is off	*/
		if( Axis->SeqCtrlOut->BaseEnableReq == FALSE )
		{
			FunExe->State = EZFFT_SEQ_READY;
		}
		break;

	case EZFFT_SEQ_END:	/* Termination processing		*/
		FnLeaveEasyFft( FunExe, Axis );
		break;

	default:
		break;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Low frequency vibration detection																				*/
	/*----------------------------------------------------------------------------------------------*/
	if( (EasyFFT->var.Drived == FALSE)
		&& (DetVibGetSpdErrPeakFn(Axis->DetVib) > (2 * DetVibGetVibCheckLevel(Axis->DetVib))) )
	{

		ALMSetServoAlarm( Axis->AlmManager, ALM_AT );
		DetVibResetMaxMinSpeedErrorFn( Axis->DetVib );
	}

	if( (ALMGetAlarmInfo(Axis->AlmManager) != FALSE)
		|| (Axis->CtrlLoopOut->TrqClamp == TRUE)
		|| (Axis->SeqCtrlOut->HwbbSts == TRUE)
		|| (Axis->SeqCtrlOut->MainPowerOn == FALSE) )
	{ /* Alarm occurrence || Torque is being limited || HWBBstate || Main circuit cutoff */
		FnCmnCtrl->FnSvonReq = FALSE;
		EasyFFT->OpeReg.EzFftState = EZFFT_STS_ERR;
	}

	return rc;
}


/* -------- Fn206  : Easy FFT Sequence Parameter Calculation	-------- */
void	EasyFftInitialize( EASYFFT *EasyFFT, USHORT *TableBuf, USHORT *TraceBuf, UNI_PRM_STR *prm_hndl_ptr )
{
	LONG		lwork;
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;

	Prm = prm_hndl_ptr->Prm;
	Bprm = prm_hndl_ptr->Bprm;
	EasyFFT->OpeReg.EzFftMode = (Prm->ezfftsw & 0x0F00) >> 8;

	if( (Prm->ezfftsw & 0x000F) == 0x0001 )
	{
		EasyFFT->conf.UseHpFil = TRUE;
	}
	else
	{
		EasyFFT->conf.UseHpFil = FALSE;
	}

	EasyFFT->conf.ReTryMethod = (Prm->ezfftsw >> 8) & 0x000F;

	switch( EasyFFT->conf.ReTryMethod )
	{
		case EZFFT_ONESTEP_RETRY_0:
		case EZFFT_OPEN_MANUAL:
		case EZFFT_CLOSE_MANUAL:
			EasyFFT->conf.ReTryMaxCount = 0;
			break;

		case EZFFT_ONESTEP_RETRY_1:
			EasyFFT->conf.ReTryMaxCount = 1;
			break;

		default:
			EasyFFT->conf.ReTryMaxCount = EasyFFT->conf.ReTryMethod;
			break;
	}

	/* Maximum detection frequency for unit conversion (ScanBFrequency1/2) */
	EasyFFT->conf.UnitMaxFreq = (USHORT)(1000000 / Bprm->SvCycleUs / 2);
	/* Frequency unit conversion gain ([2^22/UnitMaxFreq]) */
	EasyFFT->conf.KnormFrq = MlibScalKxgain( 1<<22, 1, EasyFFT->conf.UnitMaxFreq, NULL, 24 );
	/* Step (100[Hz]) */
	EasyFFT->conf.Tolerance = (100 << 22) / EasyFFT->conf.UnitMaxFreq;
	/* Overflow countermeasure Shift number for unit inversion */
	/* ((EZFFT_RETURN_UNIT * Detection frequency) >> RetUnitShift)to return the unit) */
	lwork = EasyFFT->conf.UnitMaxFreq / EZFFT_RETURN_UNIT;
	EasyFFT->conf.RetUnitShift = 0;
	while( lwork > 1 )
	{
		EasyFFT->conf.RetUnitShift++;
		lwork >>= 1;
	}
	EasyFFT->conf.RetUnitShift = 22 - EasyFFT->conf.RetUnitShift;

	/* Detection maximum frequency setting(RAMvalue) */
	Prm->swpmaxfrq = EasyFFT->conf.UnitMaxFreq;

	/* Table command buffer pointer setting */
	EasyFFT->conf.TblBufAddr = (USHORT *)TableBuf;
	/* Trace buffer pointer setting */
	EasyFFT->conf.TrcBufAddr = (USHORT *)TraceBuf;
	return;
}


/* -------- Fn206 : Easy FFT Tuning Gain	-------- */
static PRM_RSLT	easyFftSetTuningGain( EASYFFT *EasyFFT, REG_ACC_T *AccessPrm )
{
	PRM_RSLT	rc;
	rc = PRM_RSLT_SUCCESS;

	switch( EasyFFT->var.NotchSw & 0x0F0F )
	{
	case 0x0000:	/* If notch filter is not used */
	case 0x0100:	/* 2When using the 3rd stage notch filter */
		EasyFFT->var.NotchSw |= 0x0001;
		/* Pn409 : 1Stage notch filter frequency */
//		rc = RpiPrmWriteRamForFn( &pndef_ntchfil1, EasyFFT->OpeReg.EzFftFil1, Hmsg );
		rc = TunePrmWriteRam( AccessPrm, &pndef_ntchfil1, EasyFFT->OpeReg.EzFftFil1, 2 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
		break;

	case 0x0001:	/* 1When using the 3rd stage notch filter */
		EasyFFT->var.NotchSw |= 0x0100;
		/* Pn40C : 2Stage notch filter frequency */
//		rc = RpiPrmWriteRamForFn( &pndef_ntchfil2, EasyFFT->OpeReg.EzFftFil2, Hmsg );
		rc = TunePrmWriteRam( AccessPrm, &pndef_ntchfil2, EasyFFT->OpeReg.EzFftFil2, 2 );
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
		break;

	default:
		break;
	}

	/* Pn408 : Torque related function switch */
//	rc = RpiPrmWriteRamForFn( &pndef_tfuncsw, EasyFFT->var.NotchSw, Hmsg );
	rc = TunePrmWriteRam( AccessPrm, &pndef_tfuncsw, EasyFFT->var.NotchSw, 2 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}
	return(rc);
}



/* -------- Fn206 : Easy FFT Gain In EEPROM	-------- */
static PRM_RSLT	easyFftUpdateGainInEeprom( REGIF_CONFIG_T *RegCfg )
{
	PRM_RSLT	rc;
	rc = PRM_RSLT_SUCCESS;

	/* Pn408 Torque related function switch */
//	rc = RpiFunPrmNvmemUpdate( &pndef_tfuncsw );
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_tfuncsw, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn409 1Stage notch filter frequency */
//	rc = RpiFunPrmNvmemUpdate( &pndef_ntchfil1 );
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* Pn40C 2Stage notch filter frequency */
//	rc = RpiFunPrmNvmemUpdate( &pndef_ntchfil2 );
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	return(rc);
}



/* -------- Fn206 : Write Gain To Ram With EEPROM Data	-------- */
static void	easyFftUndoRamGain( REGIF_CONFIG_T *RegCfg )
{
	/* RAMthe value ofEEPROMbe the same as the value of */
	/* Pn408 Torque related function switch */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_tfuncsw, 0 );
	/* Pn409 1Stage notch filter frequency */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil1, 0 );
	/* Pn40C 2Stage notch filter frequency */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil2, 0 );

	return;
}



/* -------- Fn206  : Easy FFT Parameter Backup	-------- */
static void	easyFftSetPrmBackup( EASYFFT *EasyFFT )
{
	/* Lower limit of oscillation detection frequency */
	EasyFFT->var.OrgFrqLowerLmt = EasyFFT->var.FrqLowerLmt;
	/* Backup of minimum value of sweep frequency */
	EasyFFT->var.OrgFrqSwpMin = EasyFFT->var.FrqSwpMin;
	/* Backup of maximum value of sweep frequency */
	EasyFFT->var.OrgFrqSwpMax = EasyFFT->var.FrqSwpMax;
	return;
}



/* -------- Fn206  : Easy FFT Undo Backuped Parameter	-------- */
static void	easyFftUndoPrmBackup( EASYFFT *EasyFFT )
{
	EasyFFT->var.FrqLowerLmt = EasyFFT->var.OrgFrqLowerLmt;	/* Lower limit of oscillation detection frequency					*/
	EasyFFT->var.FrqSwpMin = EasyFFT->var.OrgFrqSwpMin;		/* Minimum value of sweep frequency					*/
	EasyFFT->var.FrqSwpMax = EasyFFT->var.OrgFrqSwpMax;		/* Maximum value of sweep frequency					*/
	return;
}



/* -------- Fn206  : Easy FFT Set Trace Registers	-------- */
static void	easyFftSetTraceRegister( TRCREG *TrcReg, LONG ax, USHORT ezfftsw )
{
	LONG	AxisSel;

	/* Select the axis to be traced */
//	AxisSel = ax << 8; /* <S1C8> */
	AxisSel = ax;

	TrcReg->TrcMode = 0x08;									/* Trace mode						*/
	TrcReg->DataSize = 1;									/* Trace data size					*/
//<S153>	TrcReg->AreaSize = EZFFT_SIZ_TRACE;				/* Trace area size					*/
	TrcReg->TrcBuffSize = EZFFT_SIZ_TRACE;					/* <S153> Trace area size			*/
	TrcReg->SampIntv = 1;									/* Trace sampling interval				*/

/* <S0C9>	Start */
	/* Select the trace target */
//	TrcReg->TrcSel[0] = (USHORT)(AxisSel | 0x01);			/* Torque/Thrust command						*/
//	TrcReg->TrcSel[2] = (USHORT)(AxisSel | 0x00);			/* No trace							*/
//	TrcReg->TrcSel[3] = (USHORT)(AxisSel | 0x00);			/* No trace							*/

//	switch( ezfftsw & 0x000F )
//	{
//	case 1:
//		TrcReg->TrcSel[1] = (USHORT)(AxisSel | 0x1E);		/* Torque command before disturbance						*/
//		break;

//	case 2:
//		TrcReg->TrcSel[1] = (USHORT)(AxisSel | 0x01);		/* Torque/Thrust command						*/
//		break;

//	case 3:
//		TrcReg->TrcSel[1] = (USHORT)(AxisSel | 0x42);		/* Load position difference							*/
//		break;

//	case 0:
//	default:
//		TrcReg->TrcSel[1] = (USHORT)(AxisSel | 0x40);		/* Position controlFBDifference						*/
//		break;
//	}

//	/* IOSelect the trace target(IONo trace) */
//	TrcReg->IOTrcSel[0] = 0;
//	TrcReg->IOTrcSel[1] = 0;
//	TrcReg->IOTrcSel[2] = 0;
//	TrcReg->IOTrcSel[3] = 0;

	/* Trace axis selection */
	TrcReg->TrcAxisSel[0] = AxisSel;		/* Torque/Thrust command						*/
	TrcReg->TrcAxisSel[2] = AxisSel;		/* No trace							*/
	TrcReg->TrcAxisSel[3] = AxisSel;		/* No trace							*/

	/* Select the trace target */
	TrcReg->TrcDataSel[0] = 0x01;			/* Torque/Thrust command						*/
	TrcReg->TrcDataSel[2] = 0x00;			/* No trace							*/
	TrcReg->TrcDataSel[3] = 0x00;			/* No trace							*/

	switch( ezfftsw & 0x000F )
	{
	case 1:
		TrcReg->TrcAxisSel[1] = AxisSel;	/* Torque command before disturbance						*/
		TrcReg->TrcDataSel[1] = 0x1E;		/* Torque command before disturbance						*/
		break;

	case 2:
		TrcReg->TrcAxisSel[1] = AxisSel;	/* Torque/Thrust command						*/
		TrcReg->TrcDataSel[1] = 0x01;		/* Torque/Thrust command						*/
		break;

	case 3:
		TrcReg->TrcAxisSel[1] = AxisSel;	/* Load position difference							*/
		TrcReg->TrcDataSel[1] = 0x42;		/* Load position difference							*/

		break;

	case 0:
	default:
		TrcReg->TrcAxisSel[1] = AxisSel;	/* Position controlFBDifference						*/
		TrcReg->TrcDataSel[1] = 0x40;		/* Position controlFBDifference						*/
		break;
	}

	/* IOSelect the trace target(IONo trace) */
	TrcReg->IOTrcAxisSel[0] = 0;
	TrcReg->IOTrcDataSel[0] = 0;

	TrcReg->IOTrcAxisSel[1] = 0;
	TrcReg->IOTrcDataSel[1] = 0;

	TrcReg->IOTrcAxisSel[2] = 0;
	TrcReg->IOTrcDataSel[2] = 0;

	TrcReg->IOTrcAxisSel[3] = 0;
	TrcReg->IOTrcDataSel[3] = 0;
/* <S0C9>	End */

	TrcReg->PtrgDnum = 0;									/* Number of pre-trigger data					*/
	TrcReg->TrgSel = 0;										/* Trigger condition							*/
	TrcReg->MTrgMode = 0;									/* Multitrigger mode					*/

}


/* -------- Fn206  : Easy FFT Table Set Drive Registers	-------- */
static void	easyFftSetTblCmdRegister( TBLDRIVE *TblDrive )
{
	TblDrive->OpeReg.TblMode = 0x05;						/* Table operation control mode				*/
	TblDrive->OpeReg.TblLimit = 1;							/* Table operation rotation amount limit			*/
	TblDrive->OpeReg.TblFuncSel = 0x0002;					/* Table operation function selection					*/
}


/* -------- Fn206  : Easy FFT Clear Counter and Step for retry	-------- */
static void	easyFftClearCounter( EASYFFT *EasyFFT )
{
	EasyFFT->var.DetCount = 0;
	EasyFFT->var.StepReTryCnt = 0;
	EasyFFT->var.AvrReTryCnt = 0;
	EasyFFT->var.DetReTryCnt = 0;
	EasyFFT->var.PosDrvCount = 0;
	EasyFFT->var.NegDrvCount = 0;

	EasyFFT->var.Step = EZFFT_STEP_1;
	EasyFFT->var.Drived = FALSE;
	EasyFFT->var.Zoom = FALSE;
	easyFftUndoPrmBackup( EasyFFT );

	return;
}


/* -------- Fn206  : Easy FFT Drive/Analyze Sequence	-------- */
static PRM_RSLT	easyFftSequence( EASYFFT *EasyFFT, TBLDRIVE *TblDrive, TRCREG *TrcReg, BPRMDAT *Bprm )
{
	PRM_RSLT	rc;
	PRM_RSLT	lrc;
	ULONG		ulwork;
	ULONG		DevStep;	/* StepDispersion */
	USHORT		DevDir;		/* Forward and reverse variation */
	USHORT		uswork;

	rc = PRM_RSLT_CONTINUE;

	switch( EasyFFT->var.SeqPtr )
	{
	case EZFFT_LSEQ_DRV:
		if( EasyFFT->var.Drived == TRUE )
		{
			if( EasyFFT->var.SameDir == FALSE )
			{
				EasyFFT->var.DrvDir = (~EasyFFT->var.DrvDir) & 0x01;
			}
			else
			{
				EasyFFT->var.SameDir = FALSE;
			}

			if( EasyFFT->var.DrvDir == EZFFT_DRV_FORWARD )
			{
				lrc = TblDriveStartRequest( TblDrive, TBLCMD_DRV_FOR );
				EasyFFT->var.PosDrvCount++;
			}
			else
			{
				lrc = TblDriveStartRequest( TblDrive, TBLCMD_DRV_REV );
				EasyFFT->var.NegDrvCount++;
			}

			if( lrc == PRM_RSLT_SUCCESS )
			{
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_WAITSTOP;
			}
			else
			{
				rc = PRM_RSLT_CONDITION_ERR;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_ERR;
			}
		}
		else
		{
			EasyFFT->var.SeqPtr = EZFFT_LSEQ_WAITSTOP;
		}
		break;

	case EZFFT_LSEQ_WAITSTOP:	/* Operation & trace completion */
		switch( TblDrive->OpeReg.TblState )
		{
			case TBLCMD_STS_POSLMT:
			case TBLCMD_STS_BB:
			case TBLCMD_STS_OT:
			case TBLCMD_STS_POWEROFF:
			case TBLCMD_STS_HWBB:
			case TBLCMD_STS_ALARM:
				/* Table operation abnormal end	*/
				rc = PRM_RSLT_CONDITION_ERR;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_ERR;
				break;

			case TBLCMD_STS_TRQLMT:
			case TBLCMD_STS_FINISH:
				EasyFFT->var.Drived = TRUE;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_DETFRQ;
				break;

			default:
				break;
		}
		break;

	case EZFFT_LSEQ_DETFRQ:	/* Frequency detection */
		if( EasyFFT->conf.UseHpFil )
		{
			/* HighPassFilterCalculation */
			easyFftCalcHpFilter( EasyFFT, TrcReg );
		}
		/* Frequency calculation */
		easyFftGetFreqGainMax( EasyFFT, TrcReg );

		/* If the detected value is small or the same as the maximum setting value, it is not0is used as detection error */
		if( EasyFFT->var.SpcPeak0 == 0 )
		{
			EasyFFT->var.AnalyzeState = EZFFT_ANASTS_NG;
		}
		else
		{
#if (FLOAT_USE==TRUE)
			if( ((EasyFFT->var.SwpDir == EZFFT_FRQ_H2L) && (EasyFFT->var.DetFrq0 == ( (LONG)EasyFFT->var.FrqSwpMin * EasyFFT->conf.KnormFrq ))) ||
				 ((EasyFFT->var.SwpDir == EZFFT_FRQ_L2H) && (EasyFFT->var.DetFrq0 == ( (LONG)EasyFFT->var.FrqSwpMax * EasyFFT->conf.KnormFrq ))) )
			{
				EasyFFT->var.AnalyzeState = EZFFT_ANASTS_NG;
			}
#else
			if( ((EasyFFT->var.SwpDir == EZFFT_FRQ_H2L) && (EasyFFT->var.DetFrq0 == MlibMulgain( EasyFFT->var.FrqSwpMin, EasyFFT->conf.KnormFrq ))) ||
				 ((EasyFFT->var.SwpDir == EZFFT_FRQ_L2H) && (EasyFFT->var.DetFrq0 == MlibMulgain( EasyFFT->var.FrqSwpMax, EasyFFT->conf.KnormFrq ))) )
			{
				EasyFFT->var.AnalyzeState = EZFFT_ANASTS_NG;
			}
#endif /* FLOAT_USE */
			else
			{
				EasyFFT->var.AnalyzeState = EZFFT_ANASTS_OK;
			}
		}

		/********************************************************************************/
		/*	== [Judgment] ==																*/
		/*		Open looporIf it is a closed loop manual operation, it will end regardless of whether it is normal or abnormal.*/
		/*		Other than that (closed loop/automatic)											*/
		/*			PIf it is within retry, try again.										*/
		/*			PIf it exceeds retry											*/
		/*				If there is a next chance in forward/reverse evaluation, proceed to the next direction rotation.			*/
		/*				There is no next chance, and there is a previousOK, continue							*/
		/*				Abort if other than the above											*/
		/********************************************************************************/
		EasyFFT->var.FinalDetFrq = EasyFFT->var.DetFrq0;

		/* Manual(Open loop, torque control), 0 retries, only one side operation */
		/* or manual(Position control), 0 retries, only one side operation */
		if( (EasyFFT->conf.ReTryMethod == EZFFT_OPEN_MANUAL)
			|| (EasyFFT->conf.ReTryMethod == EZFFT_CLOSE_MANUAL) )
		{
			if( EasyFFT->var.AnalyzeState == EZFFT_ANASTS_NG )
			{
				EasyFFT->var.DetState0 = EZFFT_STS_DETNG;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_ERR;
			}
			else
			{
				EasyFFT->var.DetState0 = EZFFT_STS_END;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_DONE;
			}
		}
		else
		{
			if( EasyFFT->var.AnalyzeState == EZFFT_ANASTS_NG )
			{
				if( EasyFFT->var.DetReTryCnt < EasyFFT->conf.ReTryMaxCount )
				{/* With or without retry */
					/* When an error occurs, if within retry, retryP++ */
					EasyFFT->var.DetReTryCnt++;
					EasyFFT->var.DetState0 = EZFFT_STS_RUN;

					EasyFFT->var.SameDir = TRUE;
					if( EasyFFT->var.DrvDir == EZFFT_DRV_FORWARD )
					{
						EasyFFT->var.PosDrvCount--;
					}
					else
					{
						EasyFFT->var.NegDrvCount--;
					}
					EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;
				}
				else
				{
					/* When exceeding retry*/
					EasyFFT->var.DetState[ EasyFFT->var.DetCount ] = EZFFT_STS_DETNG;
					EasyFFT->var.DetFrq[ EasyFFT->var.DetCount ] = EasyFFT->var.DetFrq0;
					EasyFFT->var.DetCount++;

					/* is positive/Even if the other side fails, if there is a chance, execute the ``next'' one.*/
					if( EasyFFT->var.PosDrvCount == EasyFFT->var.NegDrvCount )
					{
						/* Abnormal this time, but last timeOK */
						if( EasyFFT->var.DetState0 == EZFFT_STS_END )
						{
							EasyFFT->var.DetState0 = EZFFT_STS_ERR;
							EasyFFT->var.SeqPtr = EZFFT_LSEQ_STPEND;
						}
						else
						{
						/* This time also, last timeNG */
							EasyFFT->var.DetState0 = EZFFT_STS_DETNG;
							EasyFFT->var.SeqPtr = EZFFT_LSEQ_ERR;
						}
					}
					else
					{
						EasyFFT->var.DetState0 = EZFFT_STS_DETNG;
						EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;/* Execute "next" *//* Advance */
						EasyFFT->var.DetReTryCnt = 0;
					}
				}
			}
			else
			{
				EasyFFT->var.DetState[ EasyFFT->var.DetCount ] = EZFFT_STS_END;
				EasyFFT->var.DetFrq[ EasyFFT->var.DetCount ] = EasyFFT->var.DetFrq0;
				EasyFFT->var.DetAvrFrq[ EasyFFT->var.Step ] = EasyFFT->var.DetFrq0;
				EasyFFT->var.DetCount++;

				EasyFFT->var.DetReTryCnt = 0;

				if( EasyFFT->var.PosDrvCount == EasyFFT->var.NegDrvCount )
				{
					/* Both sides completion averaging*/
					if( EasyFFT->var.DetState0 == EZFFT_STS_END )
					{
						/* This time and last time normal*/
						EasyFFT->var.SeqPtr = EZFFT_LSEQ_AVRFRQ;	/* Both completed successfully*/
					}
					else
					{
						/* Normal this time, but abnormal last time */
						EasyFFT->var.DetState0 = EZFFT_STS_ERR;/* This is not the current result. Final result2*/
						EasyFFT->var.SeqPtr = EZFFT_LSEQ_STPEND;/* Determined on one side*/
					}
				}
				else
				{/* One side completed*/
					EasyFFT->var.DetState0 = EZFFT_STS_END;/* =1 Normal now */
					EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;/* Proceed to rotation again */
				}
			}
		}
		break;

	case EZFFT_LSEQ_AVRFRQ:	/* Averaging of forward and reverse : STEPIf both forward and reverse ends normally regardless of */
		EasyFFT->var.DetAvrFrq[ EasyFFT->var.Step ] =
			(EasyFFT->var.DetFrq[ 2*EasyFFT->var.Step ] + EasyFFT->var.DetFrq[ 2*EasyFFT->var.Step+1 ]) >> 1;
		DevDir = easyFftGetAvrgFreq( EasyFFT->var.DetAvrFrq[ EasyFFT->var.Step ],
									 EasyFFT->var.DetFrq[ 2*EasyFFT->var.Step ] );
		uswork = easyFftGetAvrgFreq( EasyFFT->var.DetAvrFrq[ EasyFFT->var.Step ],
									 EasyFFT->var.DetFrq[ 2*EasyFFT->var.Step+1 ] );

		if( uswork > DevDir )
		{
			DevDir = uswork;
		}

		if( DevDir > EZFFT_DEVLVL_DIR )
		{/* Dispersion error */
			/* When variation between forward and reverse is large */
			if( EasyFFT->var.AvrReTryCnt < EasyFFT->conf.ReTryMaxCount )
			{
				/* Retry */
				EasyFFT->var.DetCount = 2 * EasyFFT->var.Step;
				EasyFFT->var.DetReTryCnt = 0;
				EasyFFT->var.AvrReTryCnt++;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;
			}
			else
			{
				if( EasyFFT->var.Step == EZFFT_STEP_2 )
				{
					/* ZoomAnalysis error *//* Warning */
					EasyFFT->var.DetState0 = EZFFT_STS_ERR;
					EasyFFT->var.SeqPtr = EZFFT_LSEQ_WARN;
				}
				else
				{
					/* 1Error at stage */
					EasyFFT->var.DetState0 = EZFFT_STS_DETNG;
					EasyFFT->var.SeqPtr = EZFFT_LSEQ_ERR;
				}
			}
		}
		else
		{/* When variation is small */
			/* Determine Proceed */
			EasyFFT->var.DetState0 = EZFFT_STS_END;
			EasyFFT->var.SeqPtr = EZFFT_LSEQ_STPEND;
		}
		break;

	case EZFFT_LSEQ_STPEND:/* STEPComplete */
		switch( EasyFFT->conf.ReTryMethod )
		{
		case EZFFT_ONESTEP_RETRY_0:
			/* =6 This completes 5,4completed before this */
			EasyFFT->var.FinalDetFrq = EasyFFT->var.DetAvrFrq[ EasyFFT->var.Step ];
			EasyFFT->var.SeqPtr = EZFFT_LSEQ_DONE;/* This completes */
			break;

		case EZFFT_ONESTEP_RETRY_1:
			/* Zoomwill not be performed.Retryteeth1times */
			if( (EasyFFT->var.DetState0 != EZFFT_STS_END) && (EasyFFT->var.StepReTryCnt < EasyFFT->conf.ReTryMaxCount) )
			{
				/* From the beginning again */
				EasyFFT->var.Step = EZFFT_STEP_1;
				EasyFFT->var.DetCount = 2 * EasyFFT->var.Step;
				EasyFFT->var.AvrReTryCnt = 0;
				EasyFFT->var.DetReTryCnt = 0;
				EasyFFT->var.DetState0 = EZFFT_STS_RUN;
				EasyFFT->var.StepReTryCnt++;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;
			}
			else
			{
				EasyFFT->var.FinalDetFrq = EasyFFT->var.DetAvrFrq[ EasyFFT->var.Step ];
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_DONE;
			}
			break;

		default:
			if( EasyFFT->var.Step < EZFFT_STEP_2 )
			{
				/* STEP-1Complete */
				EasyFFT->var.AvrReTryCnt = 0;
				EasyFFT->var.DetReTryCnt = 0;
				EasyFFT->var.DetState0 = EZFFT_STS_RUN;
				EasyFFT->var.Step = EZFFT_STEP_2;
				EasyFFT->var.DetCount = 2 * EasyFFT->var.Step;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_ZOOMREQ;	/* STEP-1CompleteZoomJump to analysis */
			}
			else
			{/* STEP-2completed up to */
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_AVRSTP;
			}
			break;
		}
		break;

	case EZFFT_LSEQ_AVRSTP:		/* 1and2times */
		EasyFFT->var.FinalDetFrq = (EasyFFT->var.DetAvrFrq[0] + EasyFFT->var.DetAvrFrq[1]) >> 1;

		if( EasyFFT->var.FinalDetFrq > EasyFFT->var.DetAvrFrq[0] )
		{
			DevStep = EasyFFT->var.FinalDetFrq - EasyFFT->var.DetAvrFrq[0];
		}
		else
		{
			DevStep = EasyFFT->var.DetAvrFrq[0] - EasyFFT->var.FinalDetFrq;
		}

		if( EasyFFT->var.FinalDetFrq > EasyFFT->var.DetAvrFrq[1] )
		{
			ulwork = EasyFFT->var.FinalDetFrq - EasyFFT->var.DetAvrFrq[1];
		}
		else
		{
			ulwork = EasyFFT->var.DetAvrFrq[1] - EasyFFT->var.FinalDetFrq;
		}
		if( ulwork > DevStep )
		{
			DevStep = ulwork;
		}

		if( DevStep < EasyFFT->conf.Tolerance )
		{
			/* Proceed smoothly to the next */
			EasyFFT->var.StepReTryCnt = 0;
			EasyFFT->var.DetState0 = EZFFT_STS_END;
			EasyFFT->var.SeqPtr = EZFFT_LSEQ_DONE;
		}
		else
		{/* When variation is larger than allowable value */
			if( EasyFFT->conf.ReTryMaxCount > EasyFFT->var.StepReTryCnt )
			{
				/* Try from the beginning again */
				if( EasyFFT->var.Zoom == TRUE )
				{ /* ZoomReturn to previous analysis settings */
					easyFftUndoPrmBackup( EasyFFT );
				}
				EasyFFT->var.Zoom = FALSE;
				EasyFFT->var.Step = EZFFT_STEP_1;
				EasyFFT->var.DetCount = 2 * EasyFFT->var.Step;
				EasyFFT->var.AvrReTryCnt = 0;
				EasyFFT->var.DetReTryCnt = 0;
				EasyFFT->var.DetState0 = EZFFT_STS_RUN;
				EasyFFT->var.StepReTryCnt++;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;
			}
			else
			{ /* Exceeding retries */
				EasyFFT->var.DetState0 = EZFFT_STS_DETNG;
				EasyFFT->var.SeqPtr = EZFFT_LSEQ_ERR;
			}
		}
		break;

	case EZFFT_LSEQ_DONE:	/* End				*/
	case EZFFT_LSEQ_ERR:	/* Abnormal termination			*/
	case EZFFT_LSEQ_WARN:	/* Finishing the warning	*/
		rc = PRM_RSLT_SUCCESS;
		EasyFFT->OpeReg.EzFftState = EasyFFT->var.DetState0;
		/* HzConversion 2^22 * 125(EZFFT_RETURN_UNIT) = 1F400000(max) */
		EasyFFT->OpeReg.EzFftDetf =
			(USHORT)((EasyFFT->var.FinalDetFrq * EZFFT_RETURN_UNIT) >> EasyFFT->conf.RetUnitShift);
		break;

	case EZFFT_LSEQ_ZOOMREQ: /* Zoom */
		EasyFFT->var.Zoom = TRUE;
		easyFftSetPrmBackup( EasyFFT );
		easyFftSetZoomAnalysis( EasyFFT );
		easyFftSetRefCommand( EasyFFT, Bprm->SvCycleUs, Bprm->PerMaxTrq );
		EasyFFT->var.SeqPtr = EZFFT_LSEQ_DRV;
		break;

	default:
		break;
	}
	return( rc );
}


/* -------- Fn206  : Easy FFT Drive Command Set	-------- */
static void	easyFftSetRefCommand( EASYFFT *EasyFFT, LONG SvCycleUs, USHORT PerMaxTrq )
{
	SHORT	swork;
	LONG	lwork;

	swork = EZFFT_SIZ_TRACE - 1;

	if( EasyFFT->var.FrqSwpMin > EasyFFT->var.FrqSwpMax )
	{
		EasyFFT->var.SwpDir = EZFFT_FRQ_H2L;
		lwork = EasyFFT->var.FrqSwpMin - EasyFFT->var.FrqSwpMax;
	}
	else
	{
		EasyFFT->var.SwpDir = EZFFT_FRQ_L2H;
		lwork = EasyFFT->var.FrqSwpMax - EasyFFT->var.FrqSwpMin;
	}
#if (FLOAT_USE==TRUE)
	lwork = ( MlibAbs32(lwork) * EasyFFT->conf.KnormFrq );
#else
	lwork = MlibMulgain( MlibAbs32(lwork), EasyFFT->conf.KnormFrq );
#endif /* FLOAT_USE */
	EasyFFT->var.DeltaFrqRes = lwork / swork;
	EasyFFT->var.DeltaFrqRef = EasyFFT->var.DeltaFrqRes >> 1;

	EasyFFT->var.KTrqRef = MlibScalKxgain( EasyFFT->OpeReg.EzFftTrq, 1, PerMaxTrq, NULL, 24 );
	lwork = (EasyFFT->var.FrqLowerLmt + EasyFFT->var.FrqSwpMin) >> 1;
	EasyFFT->var.KHpFilter = MlibPcalKf1gain( lwork, SvCycleUs, 1 );

	easyFftMakeChirpRefTrace( EasyFFT );
	return;
}


/* -------- Fn206  : Easy FFT Tuning Registger Clear	-------- */
static void	easyFftTuneRegClear( EASYFFT *EasyFFT )
{
	EasyFFT->OpeReg.EzFftDetf = 0;
	EasyFFT->OpeReg.EzFftFil1 = 0;
	EasyFFT->OpeReg.EzFftFil2 = 0;
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Chirp command creation processing																		*/
/*																									*/
/****************************************************************************************************/
/*	Overview:ChirpCreate a command and place it on the command table														*/
/****************************************************************************************************/
static void		easyFftMakeChirpRefTrace( EASYFFT *EasyFFT )
{
	ULONG	work;
	LONG	theta;
	USHORT	i;

	for( i = 0; i < EZFFT_SIZ_TRACE; i++ )
	{
		/* Chirp command frequency acquisition */
		work = easyFftGetChirpFreq( EasyFFT, i, EZFFT_ANAOBJ_REF );
		/* 2piUnit conversions including, 360[deg]/1024 */
		theta = ((LONG)((work >> 7) * i) & 0x0000FFFF) >> 6;
		/* Table command registration(Sweep torque calculation gain conversion) */
#if (FLOAT_USE==TRUE)
		EasyFFT->conf.TblBufAddr[ i ] =
			(USHORT)( MlibFASTSINS16( (USHORT)theta ) * EasyFFT->var.KTrqRef );
#else
		EasyFFT->conf.TblBufAddr[ i ] =
			(USHORT)MlibMulgain( MlibFASTSINS16( (USHORT)theta ), EasyFFT->var.KTrqRef );
#endif /* FLOAT_USE */
	}
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Chirp frequency acquisition processing																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Summary: Find the frequency at a certain point during the sweep. (Used for both command creation and frequency estimation)					*/
/*																									*/
/*	Argument:	LONG	ObjFlag			0 : Command creation													*/
/*									1 : For frequency detection												*/
/*			USHORT	count			Trace point specification											*/
/*																									*/
/****************************************************************************************************/
static ULONG	easyFftGetChirpFreq( EASYFFT *EasyFFT, USHORT count, LONG ObjFlag )
{
	ULONG	work;
	LONG	lwork;

	if( ObjFlag == EZFFT_ANAOBJ_RES )
	{ /* Frequency detection */
		work = (ULONG)(EasyFFT->var.DeltaFrqRes * count);
	}
	else
	{ /* Command creation */
		work = (ULONG)(EasyFFT->var.DeltaFrqRef * count);
	}

	/* Frequency unit conversion */
#if (FLOAT_USE==TRUE)
	lwork = (LONG)EasyFFT->var.FrqSwpMin * EasyFFT->conf.KnormFrq ;
#else
	lwork = MlibMulgain( EasyFFT->var.FrqSwpMin, EasyFFT->conf.KnormFrq );
#endif /* FLOAT_USE */

	if( EasyFFT->var.SwpDir == EZFFT_FRQ_H2L )
	{ /* Torque sweep direction: Low⇒High */
		work = (ULONG)lwork - work;
	}
	else
	{ /* Torque sweep direction: High⇒Low */
		work = (ULONG)lwork + work;
	}
	return work;
}



/****************************************************************************************************/
/*																									*/
/*		Frequency calculation processing																				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Summary: SpeedFB or Find the value at which the absolute value of the actual torque is maximum and its frequency							*/
/*													[Hz] ==> [4000*EasyFFT->var.DetFrq >>22]		*/
/*																									*/
/****************************************************************************************************/
static void		easyFftGetFreqGainMax( EASYFFT *EasyFFT, TRCREG *TrcReg )
{
	ULONG	Freq;
	ULONG	ulwork;
	USHORT	Offset;
	SHORT	swork;
	USHORT	i;

	EasyFFT->var.SpcPeak0 = 0;
	EasyFFT->var.DetFrq0 = 0;
#if (FLOAT_USE==TRUE)
	ulwork = (LONG)EasyFFT->var.FrqLowerLmt, EasyFFT->conf.KnormFrq;
#else
	ulwork = MlibMulgain( EasyFFT->var.FrqLowerLmt, EasyFFT->conf.KnormFrq );
#endif /* FLOAT_USE */

	for( i = 0; i < EZFFT_SIZ_TRACE; i++ )
	{
		Offset = TrcReg->AreaSize / TrcReg->TrcMode / TrcReg->DataSize;		/* <S153> */
//		Offset = TrcReg->AreaSize;	/* <S153> */
		swork = ( (SHORT) ( (EasyFFT->conf.TrcBufAddr[Offset + i] & 0x7FFF) << 1 ) ) >> 1;

		/* Chirp frequency detection */
		Freq = easyFftGetChirpFreq( EasyFFT, i, EZFFT_ANAOBJ_RES );

		if( Freq >= ulwork )
		{
			if( MlibAbs32( EasyFFT->var.SpcPeak0 ) < MlibAbs32( swork ) )
			{
				EasyFFT->var.SpcPeak0 = (USHORT)MlibAbs32( swork );
				EasyFFT->var.DetFrq0  = Freq;
			}
		}
	}
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Trace table High-pass filtering processing													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Summary: Apply a high-pass filter to the trace table.												*/
/*			Compensation torque filtering is effective in cases such as notch detection.								*/
/*			Vertical axis offset and low frequency vibration can be removed.											*/
/*																									*/
/****************************************************************************************************/
static void		easyFftCalcHpFilter( EASYFFT *EasyFFT, TRCREG *TrcReg )
{
	LONG	wkcalctmp;
	SHORT	wkref, wkout;
	USHORT	Offset;
	USHORT	i;

	for( i = 0; i < EZFFT_SIZ_TRACE; i++ )
	{
//		Offset = TrcReg->AreaSize / TrcReg->TrcMode / TrcReg->DataSize;
		Offset = TrcReg->AreaSize;
		wkref = ( (SHORT) ( (EasyFFT->conf.TrcBufAddr[Offset + i] & 0x7FFF) << 1 ) ) >> 1;

		if( i != 0 )
		{
			wkout = (SHORT)MlibHpfilter1( (LONG)wkref, EasyFFT->var.KHpFilter, &wkcalctmp );
		}
		else
		{
			wkout = wkref;
		}
		EasyFFT->conf.TrcBufAddr[ Offset + i ] = wkout;
	}
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Frequency averaging processing																			*/
/*																									*/
/****************************************************************************************************/
static USHORT	easyFftGetAvrgFreq( ULONG frgavr, ULONG freq )
{
	USHORT	AvrgFreq;
	ULONG	ulwork;

	if( freq < frgavr )
	{
		ulwork = frgavr - freq;
	}
	else
	{
		ulwork = freq - frgavr;
	}
	AvrgFreq = (USHORT)(ulwork * 1000 / frgavr);

	return(AvrgFreq);
}


/****************************************************************************************************/
/*																									*/
/*		Reanalysis preparation process around frequency analysis result														*/
/*																									*/
/****************************************************************************************************/
static void		easyFftSetZoomAnalysis( EASYFFT *EasyFFT )
{
	USHORT	uswork;
	USHORT	Freq;

	/* HzConversion 2^22 * 125(EZFFT_RETURN_UNIT) = 1F400000(max) */
	Freq = (USHORT)((EasyFFT->var.FinalDetFrq * EZFFT_RETURN_UNIT) >> EasyFFT->conf.RetUnitShift);

	if( EasyFFT->var.SwpDir == EZFFT_FRQ_H2L )
	{
		uswork = Freq - 300;		/* -300Hz */
		if( uswork > EasyFFT->var.FrqSwpMax )
		{
			EasyFFT->var.FrqSwpMax = uswork;
		}
		uswork = Freq - 200;		/* -200Hz */
		if( uswork > EasyFFT->var.FrqLowerLmt )
		{
			EasyFFT->var.FrqLowerLmt = uswork;
		}
		uswork = Freq + 200;		/* +200Hz */
		if( uswork < EasyFFT->var.FrqSwpMin )
		{
			EasyFFT->var.FrqSwpMin = uswork;
		}
	}
	else
	{
		uswork = Freq - 300;		/* -300Hz */
		if( uswork > EasyFFT->var.FrqSwpMin )
		{
			EasyFFT->var.FrqSwpMin = uswork;
		}
		uswork = Freq - 200;		/* -200Hz */
		if( uswork > EasyFFT->var.FrqLowerLmt )
		{
			EasyFFT->var.FrqLowerLmt = uswork;
		}
		uswork = Freq + 200;		/* +200Hz */
		if( uswork < EasyFFT->var.FrqSwpMax )
		{
			EasyFFT->var.FrqSwpMax = uswork;
		}
	}
	return;
}


/****************************************************************************************************/
/*		EasyFFTTorque amplitude setting processing (RegisterIF)														*/
/****************************************************************************************************/
PRM_RSLT	RpiFunSetEasyFftTrqLevel( EASYFFT *EasyFFT, USHORT Lvl )
{
	PRM_RSLT	rc;
	USHORT		LvlLimit;

	rc = PRM_RSLT_SUCCESS;
	LvlLimit = TunePrmLimitUnsigned( Lvl, &pndef_ezffttrq );

	if( LvlLimit == Lvl )
	{
		EasyFFT->OpeReg.EzFftTrq = Lvl;
	}
	else
	{
		rc = PRM_RSLT_LIMIT_ERR;
	}
	return( rc );
}


/****************************************************************************************************/

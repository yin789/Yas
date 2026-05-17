/****************************************************************************
  Description   : Current detectionADAdjust(Fn00E, Fn00F)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

  Create Date   : 2010-11-18

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "FunManager.h"
#include "JL086ServoIpHw.h"

#include "PnPrmListTbl.h"
#include "KLib.h"


/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
#define KPI_RC_NOTADJ	0		/* Adjustment execution condition not met */
#define KPI_RC_ADJOK	1		/* Adjustment completed */
#define KPI_RC_ADJERR	-1		/* Adjustment failed */

#define OFFSET_ADJ		0		/* Current detection offset adjustment */
//#define GAIN_ADJ		1		/* Current detection gain adjustment */

/* <S057>	Start */
/* -------- Fn00F Current Offset Manual Adjustment -------- */
#define	ZADJI_MAX		511		/* Current detection signal offset manual adjustment value Maximum value */
#define	ZADJI_MIN		-512	/* Current detection signal offset manual adjustment value minimum value */

//#define	ACCLVL_SYSTEM	3		/* User level = System level */	/* <S101> */
#define	ACCLVL_SYSTEM	4			/* User level = System level */	/* <S101> */

//#define	OK	1					/* Current detection signal offset manual adjustment Possible	*///<S0C7>
//#define	NG	0					/* Current detection signal offset manual adjustment Not possible	*///<S0C7>
/* <S057>	End */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
static LONG	KpiAdjustCurAdZero( AXIS_HANDLE *Axis );
static LONG	TciGetCurADOffset( ASICS *SvAsicRegs, PUMP_CTR_STR *pump_if_ptr, LONG AdZero[2], LONG Mode );
static LONG	TciReadCurADMean( ASICS *SvAsicRegs, PUMP_CTR_STR *pump_if_ptr, LONG AdMean[2], LONG Mode );

/****************************************************************************
 Fn00E: Entry function of Motor Current Offset Auto Adjustment
****************************************************************************/
PRM_RSLT FnEnterMotorCurrentAutoAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL		*FnCmnCtrl;
	MOTLESSTST			*MotorLessTest;	/* Motorless test modeIFDefinition		*//* <S1B7> */

	FnCmnCtrl = Axis->FnCmnCtrl;
	MotorLessTest = Axis->MotorLessTest;									/* <S1B7> */

	/* Ends without turning on when servo is on	*/
	if(FnCmnCtrl->State.FnSvCtrlDisable != FALSE)
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* Motorless test mode judgment *//* <S1B7> */
	if( (MotorLessTest->MotorLessTestMode == TRUE)
		&& (MotorLessTest->MotorLessTestDatas.SemiEncConnect == FALSE) )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

//	FnCmnCtrl->KnlMode = KPI_KMD_ADJREFAD;
//	FnCmnCtrl->KnlMode = KPI_KMD_ADJCURAD;	/* Current detection AD adjusting *//* <S085> */

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
 Fn00E: Execute function of Motor Current Offset Auto Adjustment
****************************************************************************/
PRM_RSLT FnExecuteMotorCurrentAutoAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	SEQ_CTRL_OUT	*SeqCtrlOut;
	ASIC_HW_IF_STR	*AsicHwReg;

	LONG			AdjResult;
	FnCmnCtrl = Axis->FnCmnCtrl;
	SeqCtrlOut = Axis->SeqCtrlOut;
	AsicHwReg = Axis->SvAsicRegs->AsicHwReg;

	/* Wait until the execution request comes.	*/
	if(FunExe->CmdCode != FCMD_EXEC)
	{
		return PRM_RSLT_CONTINUE;
	}

	/* Ends without turning on when servo is on	*/
	if( (SeqCtrlOut->MainPowerOn == FALSE)
		|| (SeqCtrlOut->HwbbSts != FALSE)
		|| (SeqCtrlOut->MotStop == FALSE)
//		|| (SHalDrv_GetFaultStatus(AsicHwReg, SHAL_FAULT_BB) != 0) )	/* Base Block */
		|| (SHalDrv_GetFaultStatus(AsicHwReg, SHAL_FAULT_BB) == 0) )	/* Base Enable */
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* Adjustment execution */
	AdjResult = KpiAdjustCurAdZero(Axis);

	/* Adjustment completed */
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;

	if(AdjResult == KPI_RC_NOTADJ){
		return PRM_RSLT_CONDITION_ERR;
	}
	else if(AdjResult == KPI_RC_ADJOK){
		/* Do nothing */
	}
	else if(AdjResult == KPI_RC_ADJERR){
		return PRM_RSLT_CALC_ERR;
	}
	return PRM_RSLT_SUCCESS;
}

/* <S057>	Start */
/****************************************************************************
 Fn00E: End function of Motor Current Offset Auto Adjustment
****************************************************************************/
void FnLeaveMotorCurrentAutoAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	REGIF_CONFIG_T	*RegCfg;			/* <S057> */
	RegCfg = Axis->RegManager->hPnReg;	/* <S057> */

	/* Termination processing		*/
	/* EEPROMUpdate process	*/
	PrmStoreValueToEeprom( RegCfg, &pndef_zadjiu, 0 );		/* PnE50:Current detection zero adjustment(UPhase) */
	PrmStoreValueToEeprom( RegCfg, &pndef_zadjiv, 0 );		/* PnE51:Current detection zero adjustment(VPhase) */

	/* <S1B6>  Micro write */
	pndef_zadjiu.PrmCalCallback( 0, 0, Axis, 0 );
	pndef_zadjiv.PrmCalCallback( 0, 0, Axis, 0 );
}

/****************************************************************************
 Fn00F: Entry function of Motor Current Offset Manual Adjustment
****************************************************************************/
PRM_RSLT FnEnterMotorCurrentManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL		*FnCmnCtrl;
	FnCmnCtrl = Axis->FnCmnCtrl;

	/* Enables motorless test mode And If encoder is not connected, exit without entering */
	/* TODO:Currently does not support motorless test mode */
#if 0
	if( (Kprm.f.MotorLessTestMode && (MotorLessTestDatas.f.SemiEncConnect == FALSE)) )		/*<V245>*/
	{		
		return PRM_RSLT_CONDITION_ERR;
	}
#endif

//	FnCmnCtrl->KnlMode = KPI_KMD_ADJCURAD;	/* Current detection AD adjusting */
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;	/* normal mode */	/* <S142> */

	FunExe->HoldFnMode = TRUE;				/* FnMode automatic end */

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 Fn00F: Execute function of Motor Current Offset Manual Adjustment
****************************************************************************/
PRM_RSLT FnExecuteMotorCurrentManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRMDATA			*Prm;
	PRM_RSLT		rc;
	LONG			zadjiu_tmp;
	LONG			zadjiv_tmp;
	CHAR			Exe_flg;
	FUN_AXCOMMON	*AxCommon;

	Prm = Axis->UniPrms->Prm;
	AxCommon = ((AXIS_HANDLE*)Axis)->FnManager->AxCommon;

	rc = PRM_RSLT_SUCCESS;
	Exe_flg = NG;

	/* Confirm user level */
#if 0
	if(AxCommon->AccessLevel < ACCLVL_SYSTEM )
	{
		Exe_flg = NG;
	}
	else
	{
		Exe_flg = OK;
	}
#else
		Exe_flg = OK;
#endif
	switch( FunExe->State )
	{
	case 0:	/* Current detection zero adjustment(UPhase) Adjust	*/
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:
			if(Exe_flg == OK)
			{
				/* PnE50:Current detection zero adjustment(UPhase)Value reading */
				zadjiu_tmp = (LONG)(SHORT)((Prm->zadjiu) & 0xFFFF);

				if(zadjiu_tmp < ZADJI_MAX)
				{
					zadjiu_tmp+=1;
				}
				/* PnE50:Current detection zero adjustment(UPhase) */
				Prm->zadjiu = (SHORT)zadjiu_tmp;
				pndef_zadjiu.PrmCalCallback( 0, 0, Axis, 0 );	/* <S1B6> */
			}
			break;
		case FCMD_DOWN:
			if(Exe_flg == OK)
			{
				/* PnE50:Current detection zero adjustment(UPhase)Value reading */
				zadjiu_tmp = (LONG)(SHORT)((Prm->zadjiu) & 0xFFFF);

				if(zadjiu_tmp > ZADJI_MIN)
				{
					zadjiu_tmp-=1;
				}
				/* PnE50:Current detection zero adjustment(UPhase) */
				Prm->zadjiu = (USHORT)zadjiu_tmp;
				pndef_zadjiu.PrmCalCallback( 0, 0, Axis, 0 );	/* <S1B6> */
			}
			break;

		case FCMD_CH1:
			FunExe->State = 0;
			break;

		case FCMD_CH2:
			FunExe->State = 1;
			break;

		default:	/* other command */
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
			break;
		}
		break;

	case 1:	/* Current detection zero adjustment(VPhase) Adjust	*/
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:
			if(Exe_flg == OK)
			{
				/* PnE51:Current detection zero adjustment(VPhase)Value reading */
				zadjiv_tmp = (LONG)(SHORT)((Prm->zadjiv) & 0xFFFF);


				if(zadjiv_tmp < ZADJI_MAX)
				{
					zadjiv_tmp += 1;
				}
				/* PnE51:Current detection zero adjustment(VPhase) */
				Prm->zadjiv = (SHORT)zadjiv_tmp;
				pndef_zadjiv.PrmCalCallback( 0, 0, Axis, 0 );	/* <S1B6> */
			}
			break;

		case FCMD_DOWN:
			if(Exe_flg == OK)
			{
				/* PnE51:Current detection zero adjustment(VPhase)Value reading */
				zadjiv_tmp = (LONG)(SHORT)((Prm->zadjiv) & 0xFFFF);

				if(zadjiv_tmp > ZADJI_MIN)
				{
					zadjiv_tmp -=1;
				}
				/* PnE51:Current detection zero adjustment(UPhase) */
				Prm->zadjiv = (USHORT)zadjiv_tmp;
				pndef_zadjiv.PrmCalCallback( 0, 0, Axis, 0 );	/* <S1B6> */
			}
			break;

		case FCMD_CH1:
			FunExe->State = 0;
			break;

		default:	/* other command */
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
			break;
		}
		break;

	default:
		break;
	}

	return rc ;

}

/****************************************************************************
 Fn00F: End function of Motor Current Offset Manual Adjustment
****************************************************************************/
void FnLeaveMotorCurrentManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;
	RegCfg = Axis->RegManager->hPnReg;

	/* Termination processing		*/
	/* EEPROMUpdate process	*/
	PrmStoreValueToEeprom( RegCfg, &pndef_zadjiu, 0 );		/* PnE50:Current detection zero adjustment(UPhase) */
	PrmStoreValueToEeprom( RegCfg, &pndef_zadjiv, 0 );		/* PnE51:Current detection zero adjustment(VPhase) */

	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;					/* normal mode */
}

/* <S057>	End */

/****************************************************************************
 1103H : Entry function of the Current Gain Auto Adjustment
****************************************************************************/
PRM_RSLT FnEnterMotorCurrentGainAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL		*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;

	/* Ends without turning on when servo is on	*/
	if(FnCmnCtrl->State.FnSvCtrlDisable != FALSE)
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* FnServo operation privilege acquisition */
	FnCmnCtrl->FnSvControl = TRUE;
	FnCmnCtrl->FnCtrlMcmd = CTRLMODE_JOG;
	FnCmnCtrl->FnSensOnReq = TRUE;
	FnCmnCtrl->JogSpeed = 0;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 1103H : Execute function of the Current Gain Auto Adjustment
****************************************************************************/
PRM_RSLT FnExecuteMotorCurrentGainAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT		errRes;
	FUN_CMN_CONTROL	*FnCmnCtrl;
	SEQ_CTRL_OUT	*SeqCtrlOut;
	MENCV			*MencV;
	ALMMAN_HANDLE	*AlmManager;
	PRMDATA			*Prm;
	BPRMDAT			*Bprm;
	LONG			JogSpdP1;

	FnCmnCtrl = Axis->FnCmnCtrl;
	SeqCtrlOut = Axis->SeqCtrlOut;
	MencV = Axis->MencV;
	AlmManager = Axis->AlmManager;
	Prm = Axis->UniPrms->Prm;
	Bprm = Axis->UniPrms->Bprm;

	errRes = PRM_RSLT_CONTINUE;

	if(FunExe->State == 0)
	{/* ServoOFFMedium	*/
		JogSpdP1 = (MencV->AxisMotType == MOTTYPE_LINEAR) ? Prm->jogspd_l : Prm->jogspd;
		if(MencV->LowSpdMotor == FALSE)
		{
			JogSpdP1 = (10 * JogSpdP1);
		}
		JogSpdP1 = BprmSpeedLevelCal(Bprm, JogSpdP1, 0);

		switch(FunExe->CmdCode)
		{
		case FCMD_DOWN:	/* ServoON	*/
			JogSpdP1 = -JogSpdP1;
		case FCMD_UP:	/* Forward rotation	*/
			FnCmnCtrl->JogSpeed = JogSpdP1;
			if( (SeqCtrlOut->MainPowerOn == FALSE)
				|| (SeqCtrlOut->HwbbSts != FALSE)
				|| (AlmManager->AlmReady == FALSE) )
			{
				errRes = PRM_RSLT_CONDITION_ERR;
			}
			else
			{
				FnCmnCtrl->FnSvonReq = TRUE;
			}
			break;

		default:
			break;
		}

		if(SeqCtrlOut->BaseEnableReq != FALSE)
		{
			FunExe->State = 1;
		}
	}
	else if(FunExe->State == 1)
	{/* ServoONMedium	*/
		/* Adjustment execution */


		/* Adjustment completed->Servo off */
		FnCmnCtrl->FnSvonReq = TRUE;
		FunExe->State = 2;
	}
	else
	{/* Termination processing */
		if(SeqCtrlOut->BaseEnableReq == FALSE)
		{/* End after confirming servo off */
			errRes = PRM_RSLT_SUCCESS;
		}
	}

	return errRes;
}



/****************************************************************************************************/
/*																									*/
/*		Current detection AD zero adjustment processing(For online)															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Set the U phase/V phase current detection AD zero adjustment data.										*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static LONG KpiAdjustCurAdZero( AXIS_HANDLE *Axis )
{
	LONG				i;
	LONG				AdZero[2];
	LONG				AdjustError;
	BOOL				HwbbSts;/* <HWBB> */
	ASIC_HW_IF_STR		*pAsicHwReg;
	REG_ACC_T			AccessPrm;
	FUN_CMN_CONTROL		*FnCmnCtrl;		/* <S085> */

	/* Initialize */
	AdjustError = FALSE;
	HwbbSts = Axis->SeqCtrlOut->HwbbSts;/* <HWBB> */
	pAsicHwReg = Axis->SvAsicRegs->AsicHwReg;
	/* RAMWrite register information setting */
	AccessPrm.MemAddr	= 0;
	AccessPrm.hRegCfg	= Axis->RegManager->hPnReg;
	AccessPrm.ArrayIdx	= 0;
	AccessPrm.EepWrite	= TRUE;		/* EEPROM Write */
	AccessPrm.IfHandle	= NULL;
	FnCmnCtrl = Axis->FnCmnCtrl;		/* <S085> */

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing (Set Adjust Mode)																	*/
/*--------------------------------------------------------------------------------------------------*/
	FnCmnCtrl->KnlMode = KPI_KMD_ADJCURAD;	/* Current detection AD adjusting *//* <S085> */
	/* Do nothing here as the caller checks it */
//	if( (MainPowerOn == FALSE) || (MotStopSts == FALSE) || (HwbbSts == TRUE) )/* <HWBB> */
//	{
//		return( KPI_RC_NOTADJ );						/* Error Return								*/
//	}
	/* Since start check processing is not performed, waits are also deleted */
//	KlibWaitms( 16 );

/*--------------------------------------------------------------------------------------------------*/
/*		Base block check																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* ※knlMode == KPI_KMD_ADJCURADthen DB circuit control LpxDBControlProcess()does not process anything		*/
	/* ※Within this processDB OFF																		*/
//	pAsicHwReg->AREG_ALMSET |= BIT5;					/* DB OFF									*/
	/* When not in motorless test modeOFF *//* <S1B7> */
	/* TODO: SGD-V, the following conditions were also achieved by disabling interruptsORis						*/
	/* (1) AxisRscC->BeSequence->StopMode == STOPMODE_DBSTOPFR		*/
	/* (2) AxisRscC->BeSequence->StopMode == STOPMODE_FREERUN		*/
	/* (3) AxisRscC->FnCmnCtrl->KnlMode == KPI_KMD_ADJCURAD			*/
	if( Axis->MotorLessTest->MotorLessTestMode == FALSE )
	{
		SHalDrv_DynamicBrakeOFF(pAsicHwReg);			/* DB OFF									*/
	}
	for( i=0; ; i++ )									/* Termination conditions areBE or Timeover			*/
	{
		if( pAsicHwReg->AREG_FLTSTAT & 0x0400 )			/* FLTSTAT_10  1:Base block				*/
		{
			break;
		}
		else if( i >= 1000 )							/* Check Time Over							*/
		{
			return( KPI_RC_NOTADJ );					/* Error Return								*/
		}
		KlibWaitms( 1 );								/* Wait 1ms									*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Current detection AD zero adjustment																			*/
/*--------------------------------------------------------------------------------------------------*/
	AdjustError = TciGetCurADOffset( Axis->SvAsicRegs, &(Axis->BeSequence->ChargePump), AdZero, OFFSET_ADJ );

	if( HwbbSts == TRUE )/* <HWBB> */
	{
		AdjustError = TRUE;								/* HWBB, do not use the result. Perform first-order low-pass filter operation for					*/
	}
	else if( AdjustError == FALSE )						/* Check Adjust Error						*/
	{
		/* Create a parameter calculation functioncalland stored the adjustment results												*/
		/* PnE50 Current detection zero adjustment(UPhase)																*/
		TunePrmWriteRam( &AccessPrm, &pndef_zadjiu, AdZero[0], 2 );

		/* PnE51 Current detection zero adjustment(VPhase)																*/
		TunePrmWriteRam( &AccessPrm, &pndef_zadjiv, AdZero[1], 2 );
	}


/*--------------------------------------------------------------------------------------------------*/
/*		Current detection AD zero adjustment completion processing																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Because it is set only in initialization process, hereA/DDo not execute stop or synchronization flag setting */
//	pAsicHwReg->AREG_ADFCNT2 = 0x0000;					/* ADStop									*/
	/* KPX_MPCMD_SYNC1( ); */							/* ADSynchronous flag 1(Instead of host interrupt)	*/
	/* HereA/DSynchronous flag setting is not executed */
//	pAsicHwReg->MREG_HSUR = MPCMD_SYNC1;				/* ADSynchronous flag 1(Instead of host interrupt)	*/
	/* ※knlMode == KPI_KMD_ADJCURADthen DB circuit control LpxDBControlProcess()does not process anything		*/
	/* ※Within this processDB ON																		*/
	SHalDrv_DynamicBrakeON(pAsicHwReg);					/* DB ON									*/
//	pAsicHwReg->AREG_ALMSET &= ~(BIT5);					/* DB ON									*/

	if( AdjustError == FALSE )
	{
		return( KPI_RC_ADJOK );							/* Adjust OK								*/
	}
	else
	{
		return( KPI_RC_ADJERR );						/* Adjust Error								*/
	}
}



/****************************************************************************************************/
/*																									*/
/*		Current detection AD offset acquisition processing(For offline/online use)									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Summary Required : Obtain U-phase/V-phase current detection AD offset adjustment data.								*/
/*			The offset amount is±512If the drift after setting the offset exceeds the range				*/
/*			±2, it is considered an adjustment error and the offset amount is set to zero.						*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static LONG TciGetCurADOffset( ASICS *SvAsicRegs, PUMP_CTR_STR *PumpOnCtrl, LONG AdZero[2], LONG Mode )
{
	long	Driftx;
	long	AdMean[2];
	long	AdMeanx[2];
	long	Errflag;

	/* Local variable initialization */
	Errflag = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Current detection AD offset acquisition																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( TciReadCurADMean( SvAsicRegs, PumpOnCtrl, AdMean, Mode ) == TRUE )
	{ /* Average value reading successfully */
		if( (AdMean[0] >= -512) && (AdMean[0] <= 512) )	/* UPhase zero adjustment data normal						*/
		{
			AdZero[0] = -AdMean[0];						/* Set Adjust Data							*/
		}
		else											/* UPhase zero adjustment data abnormal						*/
		{
			AdZero[0] = 0;								/* Set Zero									*/
			Errflag = TRUE;								/* Set AdjustError Flag						*/
		}
	/*----------------------------------------------------------------------------------------------*/
		if( (AdMean[1] >= -512) && (AdMean[1] <= 512) )	/* VPhase zero adjustment data normal						*/
		{
			AdZero[1] = -AdMean[1];						/* Set Adjust Data							*/
		}
		else											/* VPhase zero adjustment data abnormal						*/
		{
			AdZero[1] = 0;								/* Set Zero									*/
			Errflag = TRUE;								/* Set AdjustError Flag						*/
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	else
	{ /* Average value reading failed(OCOccurrence) */
		AdZero[0] = 0;									/* Set Zero									*/
		AdZero[1] = 0;									/* Set Zero									*/
		Errflag = TRUE;									/* Set AdjustError Flag						*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Current detection AD drift check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( TciReadCurADMean( SvAsicRegs, PumpOnCtrl, AdMeanx, Mode ) == TRUE )
	{ /* Average value reading successfully */
		Driftx = AdMean[0] - AdMeanx[0];				/* UPhase drift check						*/
		if( (Driftx < -2) || (Driftx > 2) )				/* Drift tolerance check					*/
		{
			AdZero[0] = 0;								/* Set Zero									*/
			Errflag = TRUE;								/* Set AdjustError Flag						*/
		}
	/*----------------------------------------------------------------------------------------------*/
		Driftx = AdMean[1] - AdMeanx[1];				/* VPhase drift check						*/
		if( (Driftx < -2) || (Driftx > 2) )				/* Drift tolerance check					*/
		{
			AdZero[1] = 0;								/* Set Zero									*/
			Errflag = TRUE;								/* Set AdjustError Flag						*/
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	else
	{ /* Average value reading failed(OCOccurrence) */
		AdZero[0] = 0;									/* Set Zero									*/
		AdZero[1] = 0;									/* Set Zero									*/
		Errflag = TRUE;									/* Set AdjustError Flag						*/
	}
	return Errflag;
}



/* Offline mode processing deletion start futami */
/****************************************************************************************************/
/*																									*/
/*		Current detection AD average value reading(For online)														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Reads the current detection AD value 4096 times and returns the average value.									*/
/*																									*/
/*	Supplementary Foot : UPhase currentADClower value2bitis invalid, so, Shift2bitDiscard during processing.							*/
/*			VPhase currentADClower value2bitis invalid, so, Shift2bitDiscard during processing.							*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static LONG TciReadCurADMean( ASICS *SvAsicRegs, PUMP_CTR_STR *PumpOnCtrl, long AdMean[2], long Mode )
{
	LONG				i;
	LONG				AdBuf[2];
	LONG				Errflag;
	ASIC_HW_IF_STR		*pAsicHwReg;
	ASIC_U_IF_STR		*pAsicMicroReg;

	/* Local variable initialization */
	Errflag = FALSE;
	pAsicHwReg = SvAsicRegs->AsicHwReg;
	pAsicMicroReg = SvAsicRegs->AsicMicroReg;

/* KpiAdjustCurAdUGainAt executionJOGmode, and to start processing after starting encoder reception at regular intervals inAdjPumponReqetc. are unnecessary */
/*--------------------------------------------------------------------------------------------------*/
/*		Start Charge Pump																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( Mode == OFFSET_ADJ )							/* Motor Current Offset Adjust Mode			*/
	{
		/* KPI_ALARM_CLEAR( ); */						/* ASICAlarm clear						*/
		pAsicHwReg->AREG_ALMCLR = 0x802F;				/* ASICAlarm clear						*/
		/* Current detectionA/DExecute when adjusting offset(UsuallyScanC)	*/
#ifndef	_JL076_USE_
		pAsicMicroReg->IntAdP.FccRst = 0x01;			/* ADConversion digitization filterAlmClr		*/
#else
		pAsicMicroReg->MREG_FCCRST = 0x01;				/* ADConversion digitization filterAlmClr		*/
#endif
		PumpOnCtrl->AdjPumponReq = TRUE;				/* Set PUMPON Request						*/
		KlibWaitms( 25 );								/* Wait Start Charg Pump					*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Read AD 4096 Times 																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( Mode == OFFSET_ADJ )							/* Motor Current Offset Adjust Mode			*/
	{
		/* KPI_ALARM_CLEAR( ); */						/* ASICAlarm clear 						*/
		pAsicHwReg->AREG_ALMCLR = 0x802F;				/* ASICAlarm clear 						*/
		/* Current detectionA/DExecute when adjusting offset(UsuallyScanC)	*/
#ifndef	_JL076_USE_
		pAsicMicroReg->IntAdP.FccRst = 0x01;			/* ADConversion digitization filterAlmClr		*/
#else
		pAsicMicroReg->MREG_FCCRST = 0x01;				/* ADConversion digitization filterAlmClr		*/
#endif
		/* In initialization processA/DDo not do this here because conversion settings are being made */
	//	pAsicHwReg->AREG_ADFCNT2 = 0x0038;				/* ADRestart								*/
		/* KPX_MPCMD_SYNC1( ); */						/* ADSynchronous flag 1(Instead of host interrupt)	*/
		/* HereA/DSynchronous flag setting is not executed */
	//	pAsicHwReg->MREG_HSUR = MPCMD_SYNC1;			/* ADSynchronous flag 1(Instead of host interrupt)	*/
		/* Current detectionA/DExecute this process only when adjusting offset */
#ifndef	_JL076_USE_
		pAsicMicroReg->AdinV.IuOffsetIn = 0;			/* UPhase current detection zero adjustment						*/
		pAsicMicroReg->AdinV.IvOffsetIn = 0;			/* VPhase current detection zero adjustment						*/
#else
		pAsicMicroReg->MREG_IUOFIN = 0;					/* UPhase current detection zero adjustment						*/
		pAsicMicroReg->MREG_IUOFIN = 0;					/* VPhase current detection zero adjustment						*/
#endif
	}

	/* KPX_REFAD_LATCH( ); */							/* Latch AD Conversion Data					*/
	pAsicHwReg->AREG_ADSET1 |= BIT14;					/* Latch AD Conversion Data					*/

	AdBuf[0] = AdBuf[1] = 0;							/* Clear Buffer								*/
	for( i=0; i < 0x1000; i++ )							/* Read Loop 4096 Times						*/
	{
	/*----------------------------------------------------------------------------------------------*/
		/* KPX_REFAD_LATCH( ); */							/* Latch AD Conversion Data				*/
		pAsicHwReg->AREG_ADSET1 |= BIT14;					/* Latch AD Conversion Data				*/
		AdBuf[0] += ( (SHORT)(pAsicHwReg->AREG_AFUL)>>2 );	/* Read AD and Add Buf.(U-Phase)		*/
		AdBuf[1] += ( (SHORT)(pAsicHwReg->AREG_AFVL)>>2 );	/* Read AD and Add Buf.(V-Phase)		*/
		/* KpiResetWatchdogWaitus( 50 ); */					/* Wait 50us for AD CycleTime			*/
		/* intScanB()will execute the watchdog reset every time, so it will not be processed here. */
//		pAsicHwReg->AREG_HINT1 = 0x0000;					/* Reset Watchdog						*/
		/* Since watchdog reset processing is not performed, wait is also deleted */
//		KlibWaitus( 50 );									/* Wait 50us							*/
		KlibWaitus( 50 );									/*  <S1A7> Wait 50us	A/DConversion wait		*/
//		pAsicHwReg->AREG_HINT1 = 0x0000;					/* Reset Watchdog						*/
	/*----------------------------------------------------------------------------------------------*/
		if( (pAsicHwReg->AREG_FLTSTAT & 0x0020) != 0 )		/* Check OC(Over Current)				*/
		{
			Errflag = TRUE;
			break;
		}
	} /* for( i=0; i < 0x1000; i++ ) */

/*--------------------------------------------------------------------------------------------------*/
/*		Stop Charge Pump																			*/
/*--------------------------------------------------------------------------------------------------*/
//	if( Mode == ONLINE_ADJ)								/* Online Adjust Mode						*/
//	{
//		PumpOnCtrl->AdjPumponReq = FALSE;				/* Reset PUMPON Request						*/
//	}
//	/* ASIC_WR( MREG_FCCRST, 0x01  ); */				/* ADConversion digitization filterAlmClr		*/
//	pAsicMicroReg->MREG_FCCRST	= 0x01;					/* ADConversion digitization filterAlmClr		*/

	if( Mode == OFFSET_ADJ)								/* Online Adjust Mode						*/
	{
		PumpOnCtrl->AdjPumponReq = FALSE;				/* Reset PUMPON Request						*/
		/* Current detectionA/DExecute when adjusting offset(UsuallyScanC)	*/
#ifndef	_JL076_USE_
		pAsicMicroReg->IntAdP.FccRst = 0x01;			/* ADConversion digitization filterAlmClr		*/
#else
		pAsicMicroReg->MREG_FCCRST = 0x01;				/* ADConversion digitization filterAlmClr		*/
#endif
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate AD Mean																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( Errflag == FALSE )
	{
		AdMean[0] = ( (AdBuf[0]>>11) + 1 ) >> 1;		/* U-Phase Current AD Mean					*/
		AdMean[1] = ( (AdBuf[1]>>11) + 1 ) >> 1;		/* V-Phase Current AD Mean					*/
		return( TRUE );									/* Normal Return							*/
	}
	else
	{
		AdMean[0] = AdMean[1] = 0;						/* Clear Current AD Mean					*/
		return( FALSE );								/* Error Return								*/
	}
}
/* Offline mode processing deletion end futami */
/******************************* End of File *******************************/

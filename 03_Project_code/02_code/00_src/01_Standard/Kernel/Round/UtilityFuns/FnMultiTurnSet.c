/****************************************************************************
  Description   : Multi-turn Reset Operation(Fn008, Fn013)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	I.M.   2010.11.12     created

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "PrmAccessIf.h"
#include "FunManager.h"
#include "JL086ServoIpHw.h"
#include "KLib.h"
#include "SerialEncIf.h"




/****************************************************************************
 Fn008: Entry function of Multi Turn Data Reset
****************************************************************************/
PRM_RSLT FnEnterMultiTurnReset(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL		*FnCmnCtrl;
	MENCV				*MencV;
	BASE_CTRL_HNDL		*BaseControls;
	MOTLESSTST			*MotorLessTest;	/* Motorless test modeIFDefinition		*//* <S1B7> */

	FnCmnCtrl = Axis->FnCmnCtrl;
	MencV = Axis->MencV;
	BaseControls = Axis->BaseControls;
	MotorLessTest = Axis->MotorLessTest;									/* <S1B7> */

	/* While the servo is on,When not using multi-rotation absolute, it ends without entering	*/
//	if( (FnCmnCtrl->State.FnSvCtrlDisable != FALSE)		/* <S120> */
	if( (BaseControls->BeComplete != FALSE)
		|| (MencV->AbsoEncType != MENC_MABSO_TYPE) )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* Motorless test mode judgment *//* <S1B7> */
	if( (MotorLessTest->MotorLessTestMode == TRUE)
		&& (MotorLessTest->MotorLessTestDatas.SemiEncConnect == FALSE) )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	FunExe->HoldFnMode = TRUE;	/* Do not escape Fn mode */

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 Fn008: Execute function of Multi Turn Data Reset
****************************************************************************/
PRM_RSLT FnExecuteMultiTurnReset(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT		errRes;
	FUN_CMN_CONTROL	*FnCmnCtrl;
	MENCV			*MencV;
	SENC_ERROR		SenErr;
	BASE_CTRL_HNDL	*BaseControls;

	FnCmnCtrl = Axis->FnCmnCtrl;
	MencV = Axis->MencV;
	BaseControls = Axis->BaseControls;

	switch(FunExe->State)
	{
	case 0:	/* Protection processing */
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:	/* Protection processing	*/
			errRes = PRM_RSLT_SUCCESS;
			FunExe->State = 1;
			break;
		case FCMD_STOP:
		case FCMD_NONE:
			errRes = PRM_RSLT_SUCCESS;
			break;
		default:
			FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
			errRes = PRM_RSLT_CONDITION_ERR;
			break;
		}
		break;

	case 1:	/* Waiting for execution request */
		switch(FunExe->CmdCode)
		{
		case FCMD_EXEC:	/* Execution	*/
		//	if(FnCmnCtrl->State.FnSvCtrlDisable != FALSE)	/* <S120> */
			if(BaseControls->BeComplete != FALSE)
			{
				FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
				errRes = PRM_RSLT_CONDITION_ERR;
			}
			else
			{
				FunExe->State = 2;
				MencV->MtrSeqStep = 0;
				errRes = PRM_RSLT_CONTINUE;
			}
			break;

		case FCMD_STOP:
		case FCMD_NONE:
			errRes = PRM_RSLT_SUCCESS;
			break;
		default:
			FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
			errRes = PRM_RSLT_CONDITION_ERR;
			break;
		}
		break;

	case 2:	/* Reset processing */
		FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
		SenErr = SencResetMultiturn(MencV,
									(Axis->UniPrms->Prm)->MencP.limmlt,
									(Axis->UniPrms->Prm)->MencP.bit_dp.b.h);
		if(SenErr == SENCERR_BUSY)
		{
			errRes = PRM_RSLT_CONTINUE;
			FnCmnCtrl->OperatorSts = 0;			/* <S085> */
		}
		else if(SenErr == SENCERR_SUCCESS)
		{/* Normal end */
			FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
			FnCmnCtrl->OperatorSts = 1;			/* <S085> */
			FunExe->State = 0;
			errRes = PRM_RSLT_SUCCESS;
/* <S1C4> >>>>> */
		if( Axis->MencV->PgBaudRate == SENC_COMRATE_8MBPS )
		{
			Axis->MencV->PgBaudRate = SENC_COMRATE_4MBPS;
		}
/* <<<<< <S1C4> */
		}
		else
		{/* A.C80 : Encoder clear error */
			ALMSetServoAlarm(Axis->AlmManager, ALM_PGCLR);
			FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
			FnCmnCtrl->OperatorSts = -1;		/* <S085> */
			errRes = PRM_RSLT_CONDITION_ERR;
			FunExe->State = 0;
		}
		break;

	default:
		errRes = PRM_RSLT_SUCCESS;
		break;
	}

	return errRes;
}

/****************************************************************************
 Fn008: Leave function of Multi Turn Data Reset
****************************************************************************/
void FnLeaveMultiTurnReset(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;
	FnCmnCtrl->FnSvControl = FALSE;

	if(FunExe->State == 2)
	{
		while(SENCERR_BUSY == SencResetMultiturn(Axis->MencV,
									(Axis->UniPrms->Prm)->MencP.limmlt,
									(Axis->UniPrms->Prm)->MencP.bit_dp.b.h));
/* <S1C4> >>>>> */
		if( Axis->MencV->PgBaudRate == SENC_COMRATE_8MBPS )
		{
			Axis->MencV->PgBaudRate = SENC_COMRATE_4MBPS;
		}
/* <<<<< <S1C4> */
	}
}


/****************************************************************************
 Fn0013: Entry function of Multi Turn Limit Setting
****************************************************************************/
PRM_RSLT FnEnterMultiTurnLimitSetting(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	MENCV				*MencV;
	ALMMAN_HANDLE		*AlmMngr;
	MOTLESSTST			*MotorLessTest;	/* Motorless test modeIFDefinition		*//* <S1B7> */

	MencV = Axis->MencV;
	AlmMngr = Axis->AlmManager;
	MotorLessTest = Axis->MotorLessTest;									/* <S1B7> */

	/* When not using multi-rotation absolute, it ends without entering	*/
	if( MencV->AbsoEncType != MENC_MABSO_TYPE )
	{
		return PRM_RSLT_CONDITION_ERR;
	}
	/* If multiturn limit value mismatch has not occurred, exit without entering <S135> */
	if( !ALMCheckEachState(AlmMngr, ALM_MTLMT) )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* Motorless test mode judgment *//* <S1B7> */
	if( (MotorLessTest->MotorLessTestMode == TRUE)
		&& (MotorLessTest->MotorLessTestDatas.SemiEncConnect == FALSE) )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	FunExe->HoldFnMode = TRUE;	/* Do not escape Fn mode */

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
 Fn013: Execute function of Multi Turn Limit Setting
****************************************************************************/
PRM_RSLT FnExecuteMultiTurnLimitSetting(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT		errRes;
	SENC_ERROR		SenErr;
	MENCV			*MencV;
	FUN_CMN_CONTROL		*FnCmnCtrl;		/* <S085> */

	FnCmnCtrl = Axis->FnCmnCtrl;		/* <S085> */
	MencV = Axis->MencV;

	switch(FunExe->State)
	{
	case 0:	/* Protection processing */
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:	/* Protection processing	*/
			errRes = PRM_RSLT_SUCCESS;
			FunExe->State = 1;
			break;
		case FCMD_STOP:
		case FCMD_NONE:
			errRes = PRM_RSLT_SUCCESS;
			break;
		default:
			FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
			errRes = PRM_RSLT_CONDITION_ERR;
			break;
		}
		break;

	case 1:	/* Waiting for execution request */
		switch(FunExe->CmdCode)
		{
		case FCMD_EXEC:	/* Execution	*/
			/* When the multiturn limit values ​​of the amplifier and encoder are different */
			if( Axis->UniPrms->Prm->mtlmt != Axis->UniPrms->Prm->MencP.limmlt )
			{
				FunExe->State = 2;
				MencV->MtrSeqStep = 0;
				errRes = PRM_RSLT_CONTINUE;
			}
			else
			{	/* Normal end */
				FunExe->State = 0;
				FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
				errRes = PRM_RSLT_SUCCESS;
			}
			break;

		case FCMD_STOP:
		case FCMD_NONE:
			errRes = PRM_RSLT_SUCCESS;
			break;
		default:
			FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
			errRes = PRM_RSLT_CONDITION_ERR;
			break;
		}
		break;

	case 2:	/* Reset processing */
		FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
		SenErr = SencChangeMultiturnLimit(MencV,
									      (Axis->UniPrms->Prm)->mtlmt);
		if(SenErr == SENCERR_BUSY)
		{
			errRes = PRM_RSLT_CONTINUE;
			FnCmnCtrl->OperatorSts = 0;			/* <S085> */
		}
		else if(SenErr == SENCERR_SUCCESS)
		{/* Normal end */
			FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
			FnCmnCtrl->OperatorSts = 1;			/* <S085> */
			errRes = PRM_RSLT_SUCCESS;
			FunExe->State = 0;
/* <S1C4> >>>>> */
		if( Axis->MencV->PgBaudRate == SENC_COMRATE_8MBPS )
		{
			Axis->MencV->PgBaudRate = SENC_COMRATE_4MBPS;
		}
/* <<<<< <S1C4> */
		}
		else
		{/* A.C80 : Encoder clear error */
			ALMSetServoAlarm(Axis->AlmManager, ALM_PGCLR);
			FunExe->HoldFnMode = FALSE;	/* Escape Fn mode */
			FnCmnCtrl->OperatorSts = -1;		/* <S085> */
			errRes = PRM_RSLT_CONDITION_ERR;
			FunExe->State = 0;
		}
		break;

	default:
		errRes = PRM_RSLT_SUCCESS;
		break;
	}

	return errRes;
}


/****************************************************************************
 Fn013: Leave function of Multi Turn Limit Setting
****************************************************************************/
void FnLeaveMultiTurnLimitSetting(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;
	FnCmnCtrl->FnSvControl = FALSE;

	if(FunExe->State == 2)
	{
		while(SENCERR_BUSY == SencChangeMultiturnLimit(Axis->MencV,
									      (Axis->UniPrms->Prm)->mtlmt));
/* <S1C4> >>>>> */
		if( Axis->MencV->PgBaudRate == SENC_COMRATE_8MBPS )
		{
			Axis->MencV->PgBaudRate = SENC_COMRATE_4MBPS;
		}
/* <<<<< <S1C4> */
	}
}




/******************************* End of File *******************************/

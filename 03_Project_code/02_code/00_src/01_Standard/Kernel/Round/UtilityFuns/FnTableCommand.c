/****************************************************************************
  Description   : Table Command Drive Mode (only Register IF) (2007H)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.02.18   created

****************************************************************************/
#include "FnTableCommand.h"
#include "MercuryGlobals.h"
#include "TableDrive.h"
#include "BaseControls.h"
#include "ExControls.h"


/****************************************************************************************************/
/*																									*/
/*		Table Command Drive Mode (only Register IF) (2007H)											*/
/*																									*/
/****************************************************************************************************/
/* -------- Local Function Prototype Definition -------- */



/* -------- 2007H Table Command (only Register IF) Begin	-------- */
PRM_RSLT	FnEnterTableCommandMode( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT		rc;
	FUN_CMN_CONTROL	*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;
	rc = PRM_RSLT_SUCCESS;

	/* Ends without turning on when servo is on */
	if( TRUE == Axis->SeqCtrlOut->BaseEnableReq )
	{
		return( PRM_RSLT_CONDITION_ERR );
	}

	/* FnServo operation privilege acquisition */
	FnCmnCtrl->FnSvControl = TRUE;
	FnCmnCtrl->FnCtrlMcmd = CTRLMODE_NOCMD;
	FnCmnCtrl->JogSpeed = 0;

	/* FnMode automatic end */
	FunExe->HoldFnMode = TRUE;

	/* Status: initial setting <S212> */
	Axis->BaseControls->TblDrive.OpeReg.TblState = 0;

	return(rc);
}


/* -------- 2007H Table Command (only Register IF) End	-------- */
void	FnLeaveTableCommandMode( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	TBLDRIVE		*TblDrive;
	FUN_CMN_CONTROL	*FnCmnCtrl;

	TblDrive = &(Axis->BaseControls->TblDrive);
	FnCmnCtrl = Axis->FnCmnCtrl;

	/* FnServo operation command setting(NOCMD) */
	FnCmnCtrl->JogSpeed = 0;
	FnCmnCtrl->FnSvonReq = FALSE;
	FnCmnCtrl->FnCtrlMcmd = CTRLMODE_NOCMD;
	FnCmnCtrl->FnSensOnReq = FALSE;

	/* Wait until servo is turned off */
	while( TRUE == Axis->SeqCtrlOut->BaseEnableReq )
	{
		;
	}

	/* Table operation stop */
	TblDriveSetStopRequest( TblDrive );

	/* FnServo operation right release */
	FnCmnCtrl->FnSvControl = FALSE;

	/* Release of forced invalidation request without adjustment */
	TuneLessClearForceInvldReq( &Axis->BaseControls->TuneLessCtrl );

	/* FnMode automatic end */
	FunExe->HoldFnMode = FALSE;
}

/* -------- 2007H Table Command (only Register IF) Exe	-------- */
PRM_RSLT	FnExecuteTableCommandMode( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT		rc;
	TBLDRIVE		*TblDrive;
	FUN_CMN_CONTROL	*FnCmnCtrl;

	TblDrive = &(Axis->BaseControls->TblDrive);
	FnCmnCtrl = Axis->FnCmnCtrl;
	rc = PRM_RSLT_SUCCESS;

	switch( FunExe->State )
	{
	case 0:	/* Initialization process */
		/* Table operation preparation process */
		rc = TblDrivePrepareProc( TblDrive );
		if( rc == PRM_RSLT_SUCCESS )
		{
			/* Table operation processing */
			FunExe->State = 1;

			/* Control mode setting */
			FnCmnCtrl->FnCtrlMcmd = TblDrive->conf.CtrlMode;
			FnCmnCtrl->FnSensOnReq = TRUE;

			/* Adjustment-less invalidation determination */
			if( TblDrive->conf.TuneLessDisable )
			{
				/* Forced invalidation request without adjustment */
				TuneLessSetForceInvldReq( &(Axis->BaseControls->TuneLessCtrl) );
			}
		}
		else
		{
			FunExe->State = 4;	/* Termination processing	*/
		}
		break;

	case 1:	/* Servo off */
		switch( FunExe->CmdCode )
		{
			case FCMD_NONE:		/* No command */
				break;

			case FCMD_SV:		/* Servo-on command */
//<S0E5>		if( (!(ALMGetAlarmInfo(Axis->AlmManager)) && !(Axis->SeqCtrlOut->HwbbSts) && (Axis->SeqCtrlOut->MainPowerOn) )
//<S108>		if( (((Axis->AlmManager)->Status.AlmFlag != FALSE) && !(Axis->SeqCtrlOut->HwbbSts) && (Axis->SeqCtrlOut->MainPowerOn) )	/* <S0E5> */
				if( (((Axis->AlmManager)->Status.AlmFlag == FALSE) && !(Axis->SeqCtrlOut->HwbbSts) && (Axis->SeqCtrlOut->MainPowerOn) )	/* <S0E5> */
					&& ((TblDrive->var.TblCmd == TBLCMD_TABLESTART) || (TblDrive->var.TblCmd == TBLCMD_TBLDRV_END)) )
				{ /* When table operation start conditions are met */
					FnCmnCtrl->FnSvonReq = TRUE;
				}
				else
				{
					rc = PRM_RSLT_CONDITION_ERR;
				}
				break;

			default:			/* Other commands *//* <S191> */
//				rc = PRM_RSLT_CMD_ERR;
				break;
		}
		/* Processing state transition processing when the servo is on	*/
		if( Axis->SeqCtrlOut->BaseEnableReq == TRUE )
		{
			FunExe->State = 2;
			/* Instruction code retention clear */
			TblDrive->var.KeepTblMode = FALSE;
		}
		break;

	case 2:	/* While the servo is on */
		switch( FunExe->CmdCode )
		{
		case FCMD_NONE:		/* No command */
			break;

		case FCMD_UP:		/* Forward rotation */
		case FCMD_MTUP:		/* Forward rotation: Maintain operation mode */
			rc = TblDriveStartRequest( TblDrive, TBLCMD_DRV_FOR );
			if( rc == PRM_RSLT_SUCCESS )
			{
				/* Table operation processing */
				FunExe->State = 3;
				if( FunExe->CmdCode == FCMD_MTUP )
				{ /* Operation mode maintained */
					TblDrive->var.KeepTblMode = TRUE;
				}
			}
			else
			{
				/* Termination processing */
				FunExe->State = 4;
			}
			break;

		case FCMD_DOWN:		/* Reverse running					*/
		case FCMD_MTDOWN:	/* Reverse: Maintain operation mode	*/
			rc = TblDriveStartRequest( TblDrive, TBLCMD_DRV_REV );
			if( rc == PRM_RSLT_SUCCESS )
			{
				/* Table operation processing */
				FunExe->State = 3;
				if( FunExe->CmdCode == FCMD_MTDOWN )
				{ /* Operation mode maintained */
					TblDrive->var.KeepTblMode = TRUE;
				}
			}
			else
			{
				/* Termination processing */
				FunExe->State = 4;
			}
			break;

		case FCMD_SV:		/* Servo off */
			FnCmnCtrl->FnSvonReq = FALSE;
			break;

		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}
		/* Processing state transition processing when servo is off(Clear servo on request because it includes servo off due to external factors)	*/
		if( Axis->SeqCtrlOut->BaseEnableReq != TRUE )
		{
			FunExe->State = 1;
			FnCmnCtrl->FnSvonReq = FALSE;
		}
		break;

	case 3:	/* During table operation */
		/* Table operation end confirmation */
		if( (TblDrive->var.TblCmd == TBLCMD_TBLDRV_END) || (TblDrive->var.TblCmd == TBLCMD_TBLDRV_ERR) )
		{
			if( TblDrive->var.KeepTblMode == TRUE )
			{ /* When table operation mode continuation is requested */
				/* While the servo is on */
				FunExe->State = 2;
				TblDrive->var.KeepTblMode = FALSE;
			}
			else
			{
				FunExe->State = 4;	/* Table operation end */
			}
			/* Exit from table operation state */
		}
		else
		{
			switch( FunExe->CmdCode )
			{
			case FCMD_UP:				/* Forward rotation					*/
			case FCMD_DOWN:				/* Reverse running					*/
			case FCMD_MTUP:				/* Forward rotation: Maintain operation mode	*/
			case FCMD_MTDOWN:			/* Reverse: Maintain operation mode	*/
				break;

			case FCMD_SV:				/* Servo off			*/
				if( TblDrive->var.KeepTblMode == TRUE )
				{ /* When table operation continuation is requested */
					/* Table operation stop */
					TblDriveSetStopRequest( TblDrive );
					FnCmnCtrl->FnSvonReq = FALSE;
				}
				else
				{
					/* Table operation end	*/
					FunExe->State = 5;
				}
				break;

			default:
				rc = PRM_RSLT_CMD_ERR;
				break;
			}
			/* Processing state transition processing when servo is off */
			/* Clear servo on request because it includes servo off due to external factors */
			if( Axis->SeqCtrlOut->BaseEnableReq != TRUE )
			{
				FunExe->State = 1;
				FnCmnCtrl->FnSvonReq = FALSE;
			}
		}
		break;

	case 4:	/* Termination processing */
		FnLeaveTableCommandMode( FunExe, Axis );
		break;

	case 5:
		FnCmnCtrl->FnSvonReq = FALSE;
		if( (TblDrive->var.TblCmd == TBLCMD_TBLDRV_END)
			|| (TblDrive->var.TblCmd == TBLCMD_TBLDRV_ERR) )
		{
			/* Table operation end */
			FunExe->State = 4;
		}
		break;

	default:
		break;
	}
	return(rc);
}





/****************************************************************************************************/

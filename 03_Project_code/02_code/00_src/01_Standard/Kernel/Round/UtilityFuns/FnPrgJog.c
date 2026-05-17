/****************************************************************************
Description	: FunPrgJog.c : ProgramJOGRelated modules
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
T.Kira 2011.02.18    First edition
****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"

/****************************************************************************
		Fn004 Program Jog mode Begin
****************************************************************************/
PRM_RSLT FnEnterPrgJog( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	FUN_CMN_CONTROL *FnCmnCtrl;
	PJOGV *PJogV;

	FnCmnCtrl = Axis->FnCmnCtrl;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;

	/* Ends without turning on when servo is on	*/
	if(FnCmnCtrl->State.FnSvCtrlDisable != FALSE)
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* Parameter combination error occurring	*/
//<S0E5>	if( ALMGetAlarmInfo( Axis->AlmManager ) == ALM_PRMUNMATCH )
	if(FALSE != ALMCheckEachState( Axis->AlmManager, ALM_PRMUNMATCH ))	/* <S0E5> */
	{
		return PRM_RSLT_CONDITION_ERR;
	}

/* <S226> */
	PrgJogReset( &Axis->BaseControls->PJogHdl );
	PJogV->Var.State = PJOG_START;
	PcalPjogSw(       Axis );		/* Pn530 : ProgramJOGOperation-related switches		*/
	PcalPjogDistance( Axis );		/* Pn531,532 : ProgramJOGTravel distance[Command unit]	*/
	PcalPjogRotspd(   Axis );		/* Pn533 : ProgramJOGMovement speed[min-1]			*/
	PcalPjogAcctime(  Axis );		/* Pn534 : ProgramJOGAcceleration/deceleration time[ms]			*/
	PcalPjogWaitTime( Axis );		/* Pn535 : ProgramJOGWaiting time[ms]			*/
	PcalPjogNum(      Axis );		/* Pn536 : ProgramJOGNumber of movements				*/
	IprmcalPrgJog( &Axis->BaseControls->PJogHdl,
					Axis->UniPrms->Bprm,
				   &Axis->BaseControls->PosManager );
/* <S226> */

	/* FnServo operation privilege acquisition */
	FnCmnCtrl->FnSvControl = TRUE;
	FnCmnCtrl->FnCtrlMcmd  = CTRLMODE_PJOG;	/* <S107> */
	FnCmnCtrl->FnSensOnReq = TRUE;
	FnCmnCtrl->JogSpeed = 0;

	if ( ( PJogV->Cnst.PrmUnMatch == TRUE ) || ( PJogV->Cnst.PrmUnMatch2 == TRUE ) )
	{
		/* A.042 : Parameter combination error */
		ALMSetServoAlarm( Axis->AlmManager, ALM_PRMUNMATCH );
	}

	FunExe->HoldFnMode = TRUE;/* FnMode automatic end */

	return(PRM_RSLT_SUCCESS);
}

/****************************************************************************
		Fn004 Program Jog mode End
****************************************************************************/
void FnLeavePrgJog(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL *FnCmnCtrl;
	//SEQ_CTRL_OUT	*SeqCtrlOut;
	PJOGV *PJogV;

	FnCmnCtrl = Axis->FnCmnCtrl;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;

	PJogV->Var.RoutCmd = PJOGCMD_INIT;

	/* During round controlSENSignalONRequestI/O */
	//FnCmnCtrl->FnSensOnReq  = ((Axis->SeqCtrlData->CmdSeqBit & SENSOR_ON_BIT)== TRUE)
	//																	? TRUE : FALSE;

	/* When the servo is on, the servo is turned off.	*/
	//while(SeqCtrlOut->BaseEnableReq != FALSE);

	/* Termination processing	*/
	FnCmnCtrl->JogSpeed    = 0;
	FnCmnCtrl->FnSvonReq   = FALSE;
	FnCmnCtrl->FnCtrlMcmd  = CTRLMODE_NOCMD;
	FnCmnCtrl->FnSensOnReq = FALSE;
	FnCmnCtrl->FnSvControl = FALSE;
}

/****************************************************************************
		Fn004 Program Jog mode Execution
****************************************************************************/
LONG FnExecutePrgJog(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT rc;
	ULONG WaitTime;

	PJOGV *PJogV;
	FUN_CMN_CONTROL *FnCmnCtrl;
	SEQ_CTRL_OUT *SeqCtrlOut;

	PJogV = &Axis->BaseControls->PJogHdl.PJogV;
	FnCmnCtrl = Axis->FnCmnCtrl;
	SeqCtrlOut= Axis->SeqCtrlOut;

	rc = PRM_RSLT_SUCCESS;

	switch( FunExe->State )
	{
		case 0:	/* Initialization process	*/
			/* ProgramJOGMode setting processing */
			FnCmnCtrl->FnCtrlMcmd = CTRLMODE_PJOG;
			PJogV->Var.RoutCmd = PJOGCMD_INIT;
			PrgJogReset( &Axis->BaseControls->PJogHdl );
			FnCmnCtrl->FnSensOnReq = TRUE;
			FnCmnCtrl->FnSvonReq = FALSE;

			/* Processing state transition */
			FunExe->State = 1;
			break;

		case 1:	/* Servo off	*/
			if( (PJogV->Var.RoutCmd == PJOGCMD_INIT && PJogV->Var.State == PJOG_INIT) ||
				 PJogV->Var.State == PJOG_ABORT || PJogV->Var.State == PJOG_ERROR )
			{
				PJogV->Var.RoutCmd = PJOGCMD_NONE;
			}

			switch( FunExe->CmdCode )
			{
				/* Servo-on processing */
				case FCMD_SV:
//<S0E5>					if( ALMGetAlarmInfo( Axis->AlmManager ) != 0x00 || ( SeqCtrlOut->HwbbSts == TRUE) ||
					if( ((Axis->AlmManager)->Status.AlmFlag != FALSE) || ( SeqCtrlOut->HwbbSts == TRUE) ||	/* <S0E5> */ /* <S0FF> */
					  (SeqCtrlOut->MainPowerOn == FALSE) || (SeqCtrlOut->OverTrvlSts == TRUE) )
					{
						rc = PRM_RSLT_CONDITION_ERR;
					}
					else
					{
						FnCmnCtrl->FnSvonReq = ~FnCmnCtrl->FnSvonReq;
					}
					break;
				case FCMD_EXEC: case FCMD_CH1: case FCMD_CH2:
				case FCMD_EWRT: case FCMD_MTUP: case FCMD_MTDOWN:
					rc = PRM_RSLT_CONDITION_ERR;
					break;
				default:
					break;
			}
			/* Processing state transition processing when the servo is on */
			if( Axis->SeqCtrlOut->BaseEnableReq == TRUE )
			{
				PJogV->Var.RoutCmd = PJOGCMD_INIT;
				FunExe->State = 2;
			}
			break;

		case 2:	/* While the servo is on	(Stopped state) */
			if( (PJogV->Var.RoutCmd == PJOGCMD_INIT) && (PJogV->Var.State == PJOG_INIT) )
			{
				PJogV->Var.RoutCmd = PJOGCMD_NONE;
			}
			switch( FunExe->CmdCode )
			{
				case FCMD_NONE:
				case FCMD_STOP:
					break;
				case FCMD_UP:		/* ProgramJOGStart operation(Forward rotation)	*/
					if( !PJogV->Cnst.RevDir && Axis->SeqCtrlOut->BaseEnableReq )
					{
						PJogV->Var.RoutCmd = PJOGCMD_START;
						KlibRstLongTimer( &FunExe->LongTimer );
						FunExe->State = 3;
					}
					break;
				case FCMD_DOWN:		/* ProgramJOGStart operation(Reverse running)	*/
					if( PJogV->Cnst.RevDir && Axis->SeqCtrlOut->BaseEnableReq )
					{
						PJogV->Var.RoutCmd = PJOGCMD_START;
						KlibRstLongTimer( &FunExe->LongTimer );
						FunExe->State = 3;
					}
					break;
				case FCMD_SV:		/* Servo off */
					FnCmnCtrl->FnSvonReq = FALSE;
					break;
				case FCMD_EXEC: case FCMD_CH1:  case FCMD_CH2:
				case FCMD_EWRT: case FCMD_MTUP: case FCMD_MTDOWN:
					rc = PRM_RSLT_CONDITION_ERR;
					break;
				default:
					break;
			}
			/* Processing state transition processing when servo is off */
			if( !Axis->SeqCtrlOut->BaseEnableReq )
			{
				FunExe->State = 1;
			}
			break;

		case 3:	/* Driving */
			if( (PJogV->Var.RoutCmd == PJOGCMD_START) && (PJogV->Var.State == PJOG_START) )
			{
				PJogV->Var.RoutCmd = PJOGCMD_NONE;
			}

/* <S198> */
//			if(FnCmnCtrl->SafeStopTime == 65535)
//			{
//				WaitTime = 10 * 1000; /* 10s Wait	*/
//			}
//			else
//			{
//				WaitTime = FnCmnCtrl->SafeStopTime * 1000;	/* Safety stop time Wait */
//			}
//			if( KlibGetLongTimerMs( FunExe->LongTimer ) > WaitTime)
//			{
//				PJogV->Var.RoutCmd = PJOGCMD_ABORT;
//			}

			switch( FunExe->CmdCode )
			{
//			case FCMD_NONE:		/* <S198> */
//				break;			/* <S198> */
			case FCMD_STOP:
				PJogV->Var.RoutCmd = PJOGCMD_ABORT;
				break;
			case FCMD_UP:		/* Forward rotation	*/
				if( !PJogV->Cnst.RevDir && Axis->SeqCtrlOut->BaseEnableReq )
				{
					KlibRstLongTimer( &FunExe->LongTimer );
				}
				break;
			case FCMD_DOWN:		/* Reverse running	*/
				if( PJogV->Cnst.RevDir && Axis->SeqCtrlOut->BaseEnableReq )
				{
					KlibRstLongTimer( &FunExe->LongTimer );
				}
				break;
			case FCMD_SV:		/* Servo off	*/
				FnCmnCtrl->FnSvonReq = FALSE;
				break;
			case FCMD_EXEC: case FCMD_CH1: case FCMD_CH2:
			case FCMD_EWRT: case FCMD_MTUP: case FCMD_MTDOWN:
				rc = PRM_RSLT_CONDITION_ERR;
				break;
			case FCMD_NONE:		/* <S193> */
			default:
/* <S198> */
				if(FnCmnCtrl->SafeStopTime == 65535)
				{
					WaitTime = 10 * 1000; /* 10s Wait	*/
				}
				else
				{
					WaitTime = FnCmnCtrl->SafeStopTime * 1000;	/* Safety stop time Wait */
				}
				if( KlibGetLongTimerMs( FunExe->LongTimer ) > WaitTime)
				{
					PJogV->Var.RoutCmd = PJOGCMD_ABORT;
				}
				break;
			}

			/* Processing state transition processing when servo is off	*/
			if(!Axis->SeqCtrlOut->BaseEnableReq)
			{
				FunExe->State = 1;
			}

			/* ScanCConfirm execution */
			if( PJogV->Var.ScanCRunFlg == TRUE )
			{/* PJOGWhen restarting after an interruption,ScanCbefore it runs */	
			 /* It will come here(State=PJOG_ABORT)		*/
			 /* ScanCConfirm execution									*/

				if( (PJogV->Var.State == PJOG_ABORT) 
				 || (PJogV->Var.State == PJOG_END) 
				 ||	(PJogV->Var.State == PJOG_ERROR) )
				{
					PJogV->Var.RoutCmd = PJOGCMD_NONE;
					KlibRstLongTimer( &FunExe->LongTimer );
					FunExe->State = 4;
				}
			}
			break;

		case 4:	/* Operation end(Post-processing)	*/
			if(FnCmnCtrl->SafeStopTime == 65535)
			{
				WaitTime = 10 * 1000; /* 10s Wait	*/
			}
			else
			{
				WaitTime = FnCmnCtrl->SafeStopTime * 1000;	/* Safety stop time Wait */
			}
			if( KlibGetLongTimerMs( FunExe->LongTimer ) > WaitTime)
			{
				FunExe->State = 2;
			}
			break;

		default:
			break;
	}

	/* ScanCinON,RoundinOFFflag to	*/
	PJogV->Var.ScanCRunFlg = FALSE;
	
	return(rc);
}

/****************************************************************************
		Pn530 : ProgramJOGOperation-related switches
****************************************************************************/
void PcalPjogSw( void *AxisHdl )
{
	AXIS_HANDLE *Axis;
	PRMDATA *Prm;
	PJOGV *PJogV;

	Axis = (AXIS_HANDLE*)AxisHdl;
	Prm = Axis->UniPrms->Prm;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;

	if( PJogV->Var.State == PJOG_START )
	{
		PJogV->Var.RoutCmd = PJOGCMD_ABORT;
	}
	PJogV->Cnst.Pattern = (CHAR) ( Prm->pjogsw & 0x000F );
	PJogV->Cnst.RevDir = ( PJogV->Cnst.Pattern & 0x01 )? TRUE : FALSE;
	PJogV->Cnst.MTimes = ( PJogV->Cnst.Pattern < 2 )? Prm->pjognum : ( Prm->pjognum << 1 );
}

/****************************************************************************
		Pn531,532 : ProgramJOGTravel distance[Command unit]
****************************************************************************/
void PcalPjogDistance( void *AxisHdl )
{
	AXIS_HANDLE *Axis;
	PRMDATA *Prm;
	PJOGV *PJogV;

	Axis = (AXIS_HANDLE*)AxisHdl;
	Prm = Axis->UniPrms->Prm;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;

	if( PJogV->Var.State == PJOG_START )
	{
		PJogV->Var.RoutCmd = PJOGCMD_ABORT;
	}
	PJogV->Cnst.Distance = /*(Prm->pjogdisth << 16)*/ + Prm->pjogdistl;
	/* Prm->pjogdisthis an invalid parameter, so comment it out._kira_110223 */
}

/****************************************************************************
		Pn533 : ProgramJOGMovement speed[min-1]
****************************************************************************/
void PcalPjogRotspd( void *AxisHdl )
{
	LONG	PjogSpdP1;
	LONG	PjogSpdP2;
	LONG	kx1, sx1;
	LONG	kx2, sx2;
	LONG	Egear_a,Egear_b;

	AXIS_HANDLE *Axis;
	PRMDATA *Prm;
	BPRMDAT *Bprm;
	PJOGV	*PJogV;
	PCMKPV  *PJogPcmk;

	Axis = (AXIS_HANDLE*)AxisHdl;
	Bprm = Axis->UniPrms->Bprm;
	Prm  = Axis->UniPrms->Prm;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;
	PJogPcmk = &Axis->BaseControls->PJogHdl.PJogPcmk;
	Egear_a = Axis->BaseControls->PosManager.conf.Egear.a;
	Egear_b = Axis->BaseControls->PosManager.conf.Egear.b;

	if( Axis->MencV->AxisMotType == MOTTYPE_ROTARY )
	{
		PjogSpdP1 = (Axis->MencV->LowSpdMotor)? Prm->pjogrotspd : (10 * Prm->pjogrotspd);
		if( PJogV->Var.State == PJOG_START )
		{
			PJogV->Var.RoutCmd = PJOGCMD_ABORT;
		}

		PJogV->Cnst.MaxSpd = BprmSpeedLevelCal( Bprm, PjogSpdP1, 0 );
		PJogV->Cnst.AccTime = ( Prm->pjogacctm == 0 )? 2000 : ( Prm->pjogacctm * 1000 );
		MlibPcalaPcmdMaker( PJogV->Cnst.MaxSpd, PJogV->Cnst.AccTime,
									PJogV->Cnst.AccTime, Bprm->SvCycleUs, &PJogPcmk->P );
		kx1 = MlibScalKxgain( Egear_a, 256, Egear_b, &sx1, -1 );
		kx2 = MlibScalKskxkx( Bprm->KMotSpdConv, PjogSpdP1, 1, &sx2, -1 );
		PjogSpdP2 = MlibScalKskskx( kx1, kx2, 256, NULL, 24 );
		PjogSpdP2 = MlibGAINRD( PjogSpdP2 );
		if( PjogSpdP2 < LMTLOWSPEED )
		{
			PJogV->Cnst.PrmUnMatch = TRUE;
		}
		else
		{
			PJogV->Cnst.PrmUnMatch = FALSE;
		}
	}
}

/****************************************************************************
		Pn534 : ProgramJOGAcceleration/deceleration time[ms]
****************************************************************************/
void PcalPjogAcctime( void *AxisHdl )
{
	AXIS_HANDLE *Axis;
	PRMDATA *Prm;
	BPRMDAT *Bprm;
	PJOGV *PJogV;
	PCMKPV *PJogPcmk;

	Axis = (AXIS_HANDLE*)AxisHdl;
	Bprm = Axis->UniPrms->Bprm;
	Prm  = Axis->UniPrms->Prm;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;
	PJogPcmk = &Axis->BaseControls->PJogHdl.PJogPcmk;

	if( PJogV->Var.State == PJOG_START )
	{
		PJogV->Var.RoutCmd = PJOGCMD_ABORT;
	}
	PJogV->Cnst.AccTime = ( Prm->pjogacctm == 0 )? 2000 : ( Prm->pjogacctm * 1000 );

	MlibPcalaPcmdMaker( PJogV->Cnst.MaxSpd, PJogV->Cnst.AccTime,
								PJogV->Cnst.AccTime, Bprm->SvCycleUs, &PJogPcmk->P );
}

/****************************************************************************
		Pn535 : ProgramJOGWaiting time[ms]
****************************************************************************/
void PcalPjogWaitTime( void *AxisHdl )
{
	AXIS_HANDLE *Axis;
	PRMDATA *Prm;
	PJOGV	*PJogV;

	Axis = (AXIS_HANDLE*)AxisHdl;
	Prm = Axis->UniPrms->Prm;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;

	if( PJogV->Var.State == PJOG_START )
	{
		PJogV->Var.RoutCmd = PJOGCMD_ABORT;
	}
	if( Prm->pjogwaittm < 2 )
	{
		PJogV->Cnst.WaitTime = 0;
	}
	else
	{
		/* Sequence transition amount subtraction */
		PJogV->Cnst.WaitTime = Prm->pjogwaittm - 2;
	}
}

/****************************************************************************
		Pn536 : ProgramJOGNumber of movements
****************************************************************************/
void PcalPjogNum( void *AxisHdl )
{
	AXIS_HANDLE *Axis;
	PRMDATA *Prm;
	PJOGV	*PJogV;

	Axis = (AXIS_HANDLE*)AxisHdl;
	Prm   = Axis->UniPrms->Prm;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;

	if( PJogV->Var.State == PJOG_START )
	{
		PJogV->Var.RoutCmd = PJOGCMD_ABORT;
	}
	PJogV->Cnst.MTimes = ( PJogV->Cnst.Pattern < 2 )? Prm->pjognum : ( Prm->pjognum << 1 );
}

/****************************************************************************
		Pn585 : ProgramJOGMovement speed[mm/s] (Linear only)
****************************************************************************/
void PcalPjogLinspd( void *AxisHdl )
{
	LONG	PjogSpdP1;
	LONG	PjogSpdP2;
	LONG	kx1, sx1;
	LONG	kx2, sx2;
	LONG	Egear_a,Egear_b;

	AXIS_HANDLE *Axis;
	PRMDATA *Prm;
	BPRMDAT *Bprm;
	PJOGV *PJogV;
	PCMKPV *PJogPcmk;

	Axis = (AXIS_HANDLE*)AxisHdl;
	Bprm = Axis->UniPrms->Bprm;
	Prm  = Axis->UniPrms->Prm;
	PJogV = &Axis->BaseControls->PJogHdl.PJogV;
	PJogPcmk = &Axis->BaseControls->PJogHdl.PJogPcmk;
	Egear_a = Axis->BaseControls->PosManager.conf.Egear.a;
	Egear_b = Axis->BaseControls->PosManager.conf.Egear.b;

	if( Axis->MencV->AxisMotType == MOTTYPE_LINEAR )
	{
		PjogSpdP1 = (Axis->MencV->LowSpdMotor)? Prm->pjoglinspd : (10 * Prm->pjoglinspd);

		if( PJogV->Var.State == PJOG_START )
		{
			PJogV->Var.RoutCmd = PJOGCMD_ABORT;
		}

		PJogV->Cnst.MaxSpd  = BprmSpeedLevelCal(Bprm, PjogSpdP1, 0 );
		PJogV->Cnst.AccTime = ( Prm->pjogacctm == 0 )? 2000 : ( Prm->pjogacctm * 1000 );
		MlibPcalaPcmdMaker( PJogV->Cnst.MaxSpd, PJogV->Cnst.AccTime,
									PJogV->Cnst.AccTime, Bprm->SvCycleUs, &PJogPcmk->P );
		kx1 = MlibScalKxgain( Egear_a, 256, Egear_b, &sx1, -1 );
		kx2 = MlibScalKskxkx( Bprm->KMotSpdConv, PjogSpdP1, 1, &sx2, -1 );
		PjogSpdP2 = MlibScalKskskx( kx1, kx2, 256, NULL, 24 );
		PjogSpdP2 = MlibGAINRD( PjogSpdP2 );
		if( PjogSpdP2 < LMTLOWSPEED )
		{
			PJogV->Cnst.PrmUnMatch = TRUE;
		}
		else
		{
			PJogV->Cnst.PrmUnMatch = FALSE;
		}
    }
}

/****************************************************************************************************/


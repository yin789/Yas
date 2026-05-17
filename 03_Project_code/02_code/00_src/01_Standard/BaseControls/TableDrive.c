/****************************************************************************************************/
/*																									*/
/*																									*/
/*		TableDrive.c : For table operation	module definition												*/
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
/*	Note	:	First edition	2011.02.18	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "TableDrive.h"
#include "BaseCmnStruct.h"
#include "PosManager.h"
#include "ModelControl.h"
#include "DataTraceManager.h"
#include "MLib.h"


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/* void	TblDriveInitialize( TBLDRIVE *TblDrive );													*/
/* void	TblDriveGetTraceRegData( TBLDRIVE *TblDrive, USHORT DataSize, USHORT SampIntv,				*/
/*															ULONG DataSampIntv, ULONG DataNum );	*/
/* void	TblDriveExecute( TBLDRIVE *TblDrive, TBLCHK_SV_STS *TblChkSts, TRCHNDL *TrcHndl,			*/
/*													POS_MNG_HNDL *PosManager, MFCTRL *MFControl );	*/
/* PRM_RSLT	TblDriveCalculateParameter( TBLDRIVE *TblDrive, LONG XfbPulse1 );						*/
/* PRM_RSLT	TblDrivePrepareProc( TBLDRIVE *TblDrive );												*/
/* PRM_RSLT	TblDriveStartRequest( TBLDRIVE *TblDrive, LONG TblDir );								*/
/* PRM_RSLT	TblDriveSetStopRequest( TBLDRIVE *TblDrive );											*/
/****************************************************************************************************/
static	void tblDriveOutputRef( TBLDRIVE *TblDrive );
static	void tblDriveIPolatePosRef( TBLDRIVE *TblDrive );
static	void tblDriveSwitchPcmdFil( TBLDRIVE *TblDrive, POS_MNG_HNDL *PosManager,
																MFCTRL *MFControl, BOOL Switch );
static	void tblDriveSwitchEgear( TBLDRIVE *TblDrive, POS_MNG_HNDL *PosManager, BOOL Switch );
static	void tblDriveSetTrqFilter( TBLDRIVE *TblDrive, LONG TrqInputPos );


/****************************************************************************************************/
/*																									*/
/*		Table operation initialization processing																		*/
/*																									*/
/****************************************************************************************************/
void	TblDriveInitialize( TBLDRIVE *TblDrive, USHORT *BufferAddress , TRCHNDL *TrcHndl )
{
	/* Reset Table Drive Variables */
	MlibResetLongMemory( &(TblDrive->var), sizeof(TblDrive->var)/4 );
	/* Set Table Refference Buffer Pointer */
	TblDrive->conf.TblBuffer = BufferAddress;
	TblDrive->TrcHndl = TrcHndl;
}


/****************************************************************************************************/
/*																									*/
/*		Table operation execution parameter calculation process															*/
/*																									*/
/****************************************************************************************************/
void	TblDriveGetTraceRegData( TBLDRIVE *TblDrive, USHORT DataSize, USHORT SampIntv,
																ULONG DataSampIntv, ULONG DataNum )
{
	TblDrive->OpeReg.TblDriveDataSize = DataSize;
	TblDrive->OpeReg.TblDriveSampIntv = SampIntv;
	TblDrive->conf.DataSampIntv = DataSampIntv;
	TblDrive->conf.TblDataNum = DataNum;
}


/****************************************************************************************************/
/*																									*/
/*		Table operation execution parameter calculation process															*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	TblDriveCalculateParameter( TBLDRIVE *TblDrive, LONG XfbPulse1 )
{
	LONG	x;

	/* Table operation parameter calculation completion flag cleared */
	TblDrive->conf.TblPrmOk = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation mode setting																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch( TblDrive->OpeReg.TblMode )
	{
	case 0x01:
		TblDrive->conf.CtrlMode = CTRLMODE_TRQ;					/* Torque control						*/
		TblDrive->conf.RefOutPx = &TblDrive->var.TrqRef;		/* Command output pointer					*/
		break;
	case 0x02:
		TblDrive->conf.CtrlMode = CTRLMODE_SPD;					/* Speed control							*/
		TblDrive->conf.RefOutPx = &TblDrive->var.SpdRef;		/* Command output pointer					*/
		break;
	case 0x03:
		TblDrive->conf.CtrlMode = CTRLMODE_POS;					/* Position control:YECControl				*/
		TblDrive->conf.RefOutPx = &TblDrive->var.dPosRef;		/* Command output pointer					*/
		break;
	case 0x04:
		TblDrive->conf.CtrlMode = CTRLMODE_POS;					/* Position control:NCTControl				*/
		TblDrive->conf.RefOutPx = &TblDrive->var.dPosRef;		/* Command output pointer					*/
		return( PRM_RSLT_CONDITION_ERR );						/* Temporary treatment(NCTNot supported)				*/
	case 0x05:
		TblDrive->conf.CtrlMode = CTRLMODE_POS;					/* Position control: Torque disturbance command			*/
		TblDrive->conf.RefOutPx = &TblDrive->var.TrqRef;		/* Command output pointer					*/
		break;
	default:
		TblDrive->conf.CtrlMode = CTRLMODE_POS;					/* Default						*/
		TblDrive->conf.RefOutPx = &TblDrive->var.dPosRef;		/* Command output pointer					*/
		return( PRM_RSLT_CONDITION_ERR );						/* Abnormal return						*/
	}
	TblDrive->conf.TblMode = (UCHAR)TblDrive->OpeReg.TblMode;

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation: Command type, Number of command interpolation, Setting the number of command interpolation shifts								*/
/*--------------------------------------------------------------------------------------------------*/
	/* Number of command interpolation */
	TblDrive->conf.RefIpNum = TblDrive->OpeReg.TblDriveSampIntv;
	if( (TblDrive->conf.TblMode == 0x03) || (TblDrive->conf.TblMode == 0x04) )
	{
		/* 2Check the power of */
		x = MlibSrhbiton( TblDrive->conf.RefIpNum, SRH_FROM_LSB );
		if( (x < 0) || (TblDrive->conf.RefIpNum != (ULONG)(1 << x)) )
		{
			x = -1;
		}

		/* Interpolation number setting */
		if( TblDrive->conf.RefIpNum <= 1 )
		{
			/* Position command: No interpolation */
			TblDrive->conf.RefType = TBLREFTYPE_POSIP1;
		}
		else if( x >= 0 )
		{
			/* Position command: Number of interpolations == 2^n */
			TblDrive->conf.RefType = TBLREFTYPE_POSIP2;
			/* Number of command interpolation shifts */
			TblDrive->conf.RefIpSft = (UCHAR)x;
		}
		else
		{
			/* Position command: Number of interpolations != 2^n */
			TblDrive->conf.RefType = TBLREFTYPE_POSIP3;
		}
	}
	else
	{
		/* Speed command/torque command */
		TblDrive->conf.RefType = TBLREFTYPE_NOTPOS;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation: Motor pulse number,Setting multiturn limit										*/
/*--------------------------------------------------------------------------------------------------*/
	/* Number of motor pulses [p/rev,p/mm] */
	TblDrive->conf.MotPulse = XfbPulse1;
	/* Multiturn limit [rev,mm] */
	TblDrive->conf.MtnLimit = TblDrive->OpeReg.TblLimit;

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation: Function selection																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* Torque input position */
	TblDrive->conf.TrqInputPos = (TblDrive->OpeReg.TblMode == 0x05) ? 
										(UCHAR)TblDrive->OpeReg.TblFuncSel : 0;
	/* Disable adjustmentless control */
	TblDrive->conf.TuneLessDisable =
		(TblDrive->OpeReg.TblFuncSel & TBLFUNCBIT_TUNELESS)? TRUE : FALSE;
	/* Disable electronic gear ratio */
	TblDrive->conf.EGearDisable =
		(TblDrive->OpeReg.TblFuncSel & TBLFUNCBIT_EGEAR)? TRUE : FALSE;
	/* Position command filter disabled */
	TblDrive->conf.PcmdFilDisable =
		(TblDrive->OpeReg.TblFuncSel & TBLFUNCBIT_PCMDFIL)? TRUE : FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation parameter calculation complete																*/
/*--------------------------------------------------------------------------------------------------*/
	/* Table operation parameterOKSet */
	TblDrive->conf.TblPrmOk = TRUE;

	/* Parameter calculation completion return */
	return( PRM_RSLT_SUCCESS );
}


/****************************************************************************************************/
/*																									*/
/*		Table operation execution control																		*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation command output macro																	*/
/*--------------------------------------------------------------------------------------------------*/
static	void tblDriveOutputRef( TBLDRIVE *TblDrive )
{
	USHORT	Idx;
	LONG	TblRef;

	Idx = TblDrive->var.TblIndx;

	if( (TblDrive->OpeReg.TblDriveDataSize == TBLREFLENGTH_31BIT_IO)
		|| (TblDrive->OpeReg.TblDriveDataSize == TBLREFLENGTH_32BIT) )
	{ /* LONG Data */
		TblRef = ((LONG *)TblDrive->conf.TblBuffer)[Idx] * TblDrive->conf.KoutDir;
	}
	else if( (TblDrive->OpeReg.TblDriveDataSize == TBLREFLENGTH_15BIT_IO)
			|| (TblDrive->OpeReg.TblDriveDataSize == TBLREFLENGTH_16BIT) )
	{ /* SHORT Data */
		TblRef = ((SHORT*)TblDrive->conf.TblBuffer)[Idx] * TblDrive->conf.KoutDir;
	}
	else /* TBLREFLENGTH_NOTDEF and Others */
	{ /* Data Setting Error (SHORT) */
		TblRef = ((SHORT*)TblDrive->conf.TblBuffer)[Idx] * TblDrive->conf.KoutDir;
	}

	if( TblDrive->conf.RefType == TBLREFTYPE_NOTPOS )
	{ /* Command type: Velocity/Torque command */
		TblRef = MlibLIMITUL( TblRef, 0x01000000, -0x01000000 );
		/* Speed/Torque command output */
		*TblDrive->conf.RefOutPx = TblRef;
	}
	else
	{ /* Command type: Position command */
		/* Position command interpolation calculation input */
		TblDrive->var.InPosRef = TblRef;
		/* Table operation position command interpolation process */
		tblDriveIPolatePosRef( TblDrive );
	}
}


/****************************************************************************************************/
/*																									*/
/*		Table operation execution control function																	*/
/*																									*/
/****************************************************************************************************/
void	TblDriveExecute( TBLDRIVE *TblDrive, TBLCHK_SV_STS *TblChkSts, 
													POS_MNG_HNDL *PosManager, MFCTRL *MFControl )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation execution command check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( TblDrive->var.TblCmd == TBLCMD_NOCMD )
	{
		TblDrive->var.TblStep = 0;
		TblDrive->var.TrqRef  = 0;
		TblDrive->var.SpdRef  = 0;
		TblDrive->var.dPosRef = 0;
		TblDrive->var.TblZmd.dw = 0x0000;
		return;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation execution sequence control																*/
/*--------------------------------------------------------------------------------------------------*/
	switch( TblDrive->var.TblStep )
	{
	case 0: /* Initial Step */
		TblDrive->var.TrqRef  = 0;
		TblDrive->var.SpdRef  = 0;
		TblDrive->var.dPosRef = 0;
		TblDrive->var.MotMtn  = 0;
		TblDrive->var.MotPos  = 0;
		TblDrive->var.TblIndx = 0;
		TblDrive->var.TblStep = 1;
		TblDrive->var.TblZmd.dw = 0x0000;

		/* ZeroClamp Control */
		TblDrive->var.TblZmd.b.zm = CTRLMODE_ZCLMP;
		break;

	case 1: /* Wait Base Enable */
	case 2: /* ZeroClamp and Wait Start Table Drive */
		if( TblDrive->var.TblCmd == TBLCMD_TABLEDRIVE )
		{
			TblDrive->var.TblStep = 3;
			TblDrive->var.TblIndx = 0;
			TblDrive->var.TblSamp = 0;

			if( TblDrive->var.WithTrcReq )
			{ /* When there is a trace concurrent execution request */
				DtrcSetTrgPointToZero(  TblDrive->TrcHndl );
			}

			/* Torque command filter control */
			tblDriveSetTrqFilter( TblDrive, TblDrive->conf.TrqInputPos );
			/* Electronic gear control */
			tblDriveSwitchEgear( TblDrive, PosManager, !(TblDrive->conf.EGearDisable) );
			/* Position command filter control */
			tblDriveSwitchPcmdFil( TblDrive, PosManager, MFControl, !(TblDrive->conf.PcmdFilDisable) );

			TblDrive->var.ErrState = TBLCMD_STS_INIT;
		}
		break;

	case 3: /* Table Drive */
		/*------------------------------------------------------------------------------------------*/
		/*	Alarm occurrence check process																*/
		/*------------------------------------------------------------------------------------------*/
		if ( TblChkSts->AlmFlag )
		{
			TblDrive->var.ErrState = TBLCMD_STS_ALARM;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	HWBBStatus check processing																	*/
		/*------------------------------------------------------------------------------------------*/
		else if ( TblChkSts->HwbbSts )
		{
			TblDrive->var.ErrState = TBLCMD_STS_HWBB;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Main circuit power off check process																*/
		/*------------------------------------------------------------------------------------------*/
		else if ( !(TblChkSts->MainPowerOn) )
		{
			TblDrive->var.ErrState = TBLCMD_STS_POWEROFF;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Overtravel check processing																*/
		/*------------------------------------------------------------------------------------------*/
		else if ( TblChkSts->OverTrvlSts )
		{
			TblDrive->var.ErrState = TBLCMD_STS_OT;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Motor energization check processing																	*/
		/*------------------------------------------------------------------------------------------*/
		else if ( !(TblChkSts->BeComplete) )
		{
			TblDrive->var.ErrState = TBLCMD_STS_BB;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Torque limit check process																	*/
		/*------------------------------------------------------------------------------------------*/
		else if ( TblChkSts->TrqClamp )
		{
			TblDrive->var.ErrState = TBLCMD_STS_TRQLMT;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Multiturn limit check processing														*/
		/*------------------------------------------------------------------------------------------*/
		if( TblDrive->conf.MtnLimit > 0 )
		{
			/* Multiturn calculation */
			TblDrive->var.MotPos += TblChkSts->dMotPos;
			if( TblDrive->var.MotPos >= TblDrive->conf.MotPulse )
			{
				TblDrive->var.MotMtn += 1;
				TblDrive->var.MotPos -= TblDrive->conf.MotPulse;
			}
			else if( TblDrive->var.MotPos <= -TblDrive->conf.MotPulse )
			{
				TblDrive->var.MotMtn -= 1;
				TblDrive->var.MotPos += TblDrive->conf.MotPulse;
			}
			if( (TblDrive->var.MotMtn >= TblDrive->conf.MtnLimit)
				|| (TblDrive->var.MotMtn <= -TblDrive->conf.MtnLimit) )
			{
				/* Table Drive Error Step */
				TblDrive->var.TblStep = 5;
				/* Clear Reference Output */
				*TblDrive->conf.RefOutPx = 0;
				/* TableDriveState = MturnErr */
				TblDrive->OpeReg.TblState = TBLCMD_STS_POSLMT;
				/* ZeroClamp Control */
				TblDrive->var.TblZmd.b.zm = CTRLMODE_ZCLMP;
				break;
			}
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Table operation command creation process																*/
		/*------------------------------------------------------------------------------------------*/
		if( TblDrive->var.WithTrcReq )
		{
			if( (TblDrive->var.TblSamp == 0)
				|| (TblDrive->var.TblSamp > (TblDrive->OpeReg.TblDriveSampIntv - TblDrive->conf.DataSampIntv)) )
			{
				/* Set Trace Variable Function */
				DtrcSetTrcVar( TblDrive->TrcHndl );
			}
		}
		/*------------------------------------------------------------------------------------------*/
		if( TblDrive->var.TblSamp == 0 )
		{
			/* Table operation end check */
			if( TblDrive->var.TblIndx >= TblDrive->conf.TblDataNum )
			{
				TblDrive->var.TblStep = 4;					/* Table Drive End Step					*/
				*TblDrive->conf.RefOutPx = 0;				/* Clear Reference Output				*/
				TblDrive->var.TblZmd.b.zm = CTRLMODE_ZCLMP;	/* ZeroClamp Control					*/
				break;
			}
			/* Table operation command output processing */
			tblDriveOutputRef( TblDrive );

			/* Data trace execution processing */
			if( TblDrive->var.WithTrcReq )
			{
				/* Data Trace Procedure */
				DtrcProcedure( TblDrive->TrcHndl, TblDrive->var.TblIndx );

				/* Reset Average Variable */
				DtrcResetAverageVar( TblDrive->TrcHndl );
			}

			/* Table operation index update */
			TblDrive->var.TblIndx++;
		}
		else if( TblDrive->conf.RefType != TBLREFTYPE_NOTPOS )
		{ /* Command type check */
			/* Table operation position command interpolation process */
			tblDriveIPolatePosRef( TblDrive );
		}
		/*------------------------------------------------------------------------------------------*/
		TblDrive->var.TblSamp++;
		if( TblDrive->var.TblSamp >= TblDrive->OpeReg.TblDriveSampIntv )
		{
			TblDrive->var.TblSamp = 0;
		}
		/*------------------------------------------------------------------------------------------*/
		TblDrive->OpeReg.TblState =TBLCMD_STS_RUN;			/* TableDriveState = Execute			*/
		TblDrive->var.TblZmd.b.zm = CTRLMODE_ZNONE;			/* NoneZero Control						*/
		break;

	case 4: /* Table Drive End Step */
		/* Clear Reference Output */
		*TblDrive->conf.RefOutPx = 0;
		/* Torque command filter control */
		tblDriveSetTrqFilter( TblDrive, FALSE );
		/* Electronic gear enabled */
		tblDriveSwitchEgear( TblDrive, PosManager, TRUE );
		/* Position command filter enable */
		tblDriveSwitchPcmdFil( TblDrive, PosManager, MFControl, TRUE );
		if ( TblDrive->var.ErrState != 0 )
		{
			/* TableDriveState = End(ErrState) */
			TblDrive->OpeReg.TblState = TblDrive->var.ErrState;
		}
		else
		{
			/* TableDriveState = End */
			TblDrive->OpeReg.TblState = TBLCMD_STS_FINISH;
		}
		TblDrive->var.TblCmd  = TBLCMD_TBLDRV_END;			/* Table operation completed normally					*/
		TblDrive->var.TblZmd.b.zm = CTRLMODE_ZCLMP;			/* ZeroClamp Control					*/
		TblDrive->var.TblStep = 2;
		break;

	case 5: /* Table Drive Error Step */
		/* Clear Reference Output */
		*TblDrive->conf.RefOutPx = 0;
		/* Torque command filter control */
		tblDriveSetTrqFilter( TblDrive, FALSE );
		/* Electronic gear enabled */
		tblDriveSwitchEgear( TblDrive, PosManager, TRUE );
		/* Position command filter enable */
		tblDriveSwitchPcmdFil( TblDrive, PosManager, MFControl, TRUE );

		TblDrive->var.TblCmd  = TBLCMD_TBLDRV_ERR;			/* Table operation abnormal end					*/
		TblDrive->var.TblStep = 6;
		break;

	case 6: /* Table Drive Error Step */
		if( (TblChkSts->AlmFlag == TRUE) || (TblChkSts->BeComplete == FALSE) )
		{ /* Alarm occurrence || BaseBlock */
			/* NoneZero Control */
			TblDrive->var.TblZmd.b.zm = CTRLMODE_ZNONE;
		}
		else
		{
			/* ZeroClamp Control */
			TblDrive->var.TblZmd.b.zm = CTRLMODE_ZCLMP;
		}
		break;

	default: /* Default */
		break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Zero control mode command processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	TblDrive->var.TblZmd.b.zmchg  = TblDrive->var.TblZmd.b.zm ^ TblDrive->var.TblZmd.b.zmlast;
	TblDrive->var.TblZmd.b.zmlast = TblDrive->var.TblZmd.b.zm;

	return;
}


/****************************************************************************************************/
/*																									*/
/*		Table operation position command interpolation process																*/
/*																									*/
/****************************************************************************************************/
static	void tblDriveIPolatePosRef( TBLDRIVE *TblDrive )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Position command interpolation : No capacity																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( TblDrive->conf.RefType == TBLREFTYPE_POSIP1 )
	{
		TblDrive->var.dPosRef = TblDrive->var.InPosRef;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position command interpolation : Interpolation number == 2^n																*/
/*--------------------------------------------------------------------------------------------------*/
	else if( TblDrive->conf.RefType == TBLREFTYPE_POSIP2 )
	{
		if( TblDrive->var.TblSamp == 0 )
		{
			TblDrive->var.IxPosRef = (TblDrive->var.InPosRef >> TblDrive->conf.RefIpSft);
			TblDrive->var.IxPosRem =
				TblDrive->var.InPosRef - (TblDrive->var.IxPosRef << TblDrive->conf.RefIpSft);
			if( TblDrive->var.InPosRef < 0 )
			{
				TblDrive->var.IxPosRef += 1;
				TblDrive->var.IxPosRem -= TblDrive->conf.RefIpNum;
			}
			TblDrive->var.WkPosRem = 0;
		}
	/*------------------------------------------------------------------------------------------*/
		if( TblDrive->var.InPosRef >= 0 )
		{ /* When positive */
			TblDrive->var.WkPosRem += TblDrive->var.IxPosRem;
			if( TblDrive->var.WkPosRem > 0 )
			{
				TblDrive->var.dPosRef = TblDrive->var.IxPosRef + 1;
				TblDrive->var.WkPosRem -= TblDrive->conf.RefIpNum;
			}
			else
			{
				TblDrive->var.dPosRef = TblDrive->var.IxPosRef;
			}
		}
	/*------------------------------------------------------------------------------------------*/
		else
		{ /* If negative */
			TblDrive->var.WkPosRem += TblDrive->var.IxPosRem;
			if( TblDrive->var.WkPosRem < 0 )
			{
				TblDrive->var.dPosRef = TblDrive->var.IxPosRef - 1;
				TblDrive->var.WkPosRem += TblDrive->conf.RefIpNum;
			}
			else
			{
				TblDrive->var.dPosRef = TblDrive->var.IxPosRef;
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position command interpolation : Interpolation number != 2^n																*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		if( TblDrive->var.TblSamp == 0 )
		{
			TblDrive->var.IxPosRef = TblDrive->var.InPosRef / TblDrive->conf.RefIpNum;
			TblDrive->var.IxPosRem =
				TblDrive->var.InPosRef - (TblDrive->var.IxPosRef * TblDrive->conf.RefIpNum);
			TblDrive->var.WkPosRem = 0;
		}
	/*------------------------------------------------------------------------------------------*/
		if( TblDrive->var.InPosRef >= 0 )
		{ /* When positive */
			TblDrive->var.WkPosRem += TblDrive->var.IxPosRem;
			if( TblDrive->var.WkPosRem > 0 )
			{
				TblDrive->var.dPosRef = TblDrive->var.IxPosRef + 1;
				TblDrive->var.WkPosRem -= TblDrive->conf.RefIpNum;
			}
			else
			{
				TblDrive->var.dPosRef = TblDrive->var.IxPosRef;
			}
		}
	/*------------------------------------------------------------------------------------------*/
		else
		{ /* If negative */
			TblDrive->var.WkPosRem += TblDrive->var.IxPosRem;
			if( TblDrive->var.WkPosRem < 0 )
			{
				TblDrive->var.dPosRef = TblDrive->var.IxPosRef - 1;
				TblDrive->var.WkPosRem += TblDrive->conf.RefIpNum;
			}
			else
			{
				TblDrive->var.dPosRef = TblDrive->var.IxPosRef;
			}
		}
	}
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Table operation torque command filter control(Valid/invalid)												*/
/*																									*/
/****************************************************************************************************/
static	void tblDriveSetTrqFilter( TBLDRIVE *TblDrive, LONG TrqInputPos )
{
	if( TblDrive->conf.CtrlMode == CTRLMODE_TRQ )
	{
		switch( TrqInputPos )
		{
		case 0x00:								/* None												*/
		case 0x01:								/* Before torque command filter								*/
		default :
			TblDrive->var.TrqFilDisable = FALSE;	/* Torque command filter enable						*/
			TblDrive->var.NotchLpassDisable = FALSE;/* Notch & low-pass filter enabled					*/
//			BpiEnableCPUTorqueFilter( );		
//			KpiEnableMicroTorqueFilter( );			/* Micro torque command filter enabled				*/
			break;
		case 0x02:								/* After low-pass filter, before notch filter				*/
			TblDrive->var.TrqFilDisable = TRUE;		/* Torque command filter invalidation						*/
			TblDrive->var.NotchLpassDisable = FALSE;/* Notch & low-pass filter enabled					*/
//			BpiDisableCPUTorqueFilter( );			/* CPUTorque command filter invalidation					*/
//			KpiEnableMicroTorqueFilter( );			/* Micro torque command filter enabled				*/
			break;
		case 0x03:								/* After torque command filter								*/
			TblDrive->var.TrqFilDisable = TRUE;		/* Torque command filter invalidation						*/
			TblDrive->var.NotchLpassDisable = TRUE;	/* Notch & low-pass filter disabled					*/
//			BpiDisableCPUTorqueFilter( );			/* CPUTorque command filter invalidation					*/
//			KpiDisableMicroTorqueFilter( );			/* Micro torque command filter disabled				*/
			break;
		}
	}
	else if( TblDrive->conf.CtrlMode == CTRLMODE_POS )
	{
		/* Torque disturbance command input position selection */
		TblDrive->var.DisTrqInTiming = TrqInputPos;
	}
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Table operation electronic gear control(Valid/invalid)														*/
/*																									*/
/****************************************************************************************************/
static	void tblDriveSwitchEgear( TBLDRIVE *TblDrive, POS_MNG_HNDL *PosManager, BOOL Switch )
{
	if( TblDrive->conf.CtrlMode == CTRLMODE_POS )
	{
		BpiEgearFuncSwitch( PosManager, Switch );
	}
}


/****************************************************************************************************/
/*																									*/
/*		Table operation position command filter control(Valid/invalid)												*/
/*																									*/
/****************************************************************************************************/
static	void tblDriveSwitchPcmdFil( TBLDRIVE *TblDrive, POS_MNG_HNDL *PosManager,
																MFCTRL *MFControl, BOOL Switch )
{
	if( TblDrive->conf.CtrlMode == CTRLMODE_POS )
	{
		BpiPcmdFilSwitch( PosManager, Switch );
		MfcSetMfcCtrlSwitch( MFControl, Switch );
	}
}


/****************************************************************************************************/
/*																									*/
/*		For table operation startAPI																	*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	TblDrivePrepareProc( TBLDRIVE *TblDrive )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation processing start check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( TblDrive->var.TblCmd != TBLCMD_NOCMD )
	{
		return( PRM_RSLT_CONDITION_ERR );
	}
#if 0 /* Designed not to be used as it is separated from trace */
	else if( TrcExeV.TrcCmd != TRCCMD_NOCMD )
	{
		return( PRM_RSLT_NOACCRIGHT_ERR );
	}
	else if( TblDrive->conf.TblPrmOk != TRUE )
	{
		return( PRM_RSLT_NOACCRIGHT_ERR );
	}
	else if( TrcExeP.TrcPrmOk != TRUE )
	{
		return( PRM_RSLT_NOACCRIGHT_ERR );
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation processing start command																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Data trace dedicated variable reset process */
//	LpxResetDataTraceVarX( );

	/* Table operation processing start command */
	TblDrive->var.TblCmd = TBLCMD_TABLESTART;

	/* 2011.02.28 Y.Oka Forcibly interlock trace during table operation */
	TblDrive->var.WithTrcReq = TRUE;

	return( PRM_RSLT_SUCCESS );
}


/****************************************************************************************************/
/*																									*/
/*		Table operation execution start requestAPI																	*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	TblDriveStartRequest( TBLDRIVE *TblDrive, LONG TblDir )
{
	LONG	KoutDir;

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation execution start check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( TblDrive->conf.TblPrmOk != TRUE )
	{
		return( PRM_RSLT_CONDITION_ERR );
	}
#if 0 /* Designed not to be used as it is separated from trace */
	else if( TrcExeP.TrcPrmOk != TRUE )
	{
		return( REGIF_DAMTHERR2 );
	}
	else if( (TblDrive->var.TblCmd != TBLCMD_TABLESTART) && (TblDrive->var.TblCmd != TBLCMD_TBLDRV_END) )
	{
		return( REGIF_DAMTHERR2 );
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation command output gain calculation(Normalized gain + command direction)									*/
/*--------------------------------------------------------------------------------------------------*/
	KoutDir = (TblDir >= 0)? 1 : -1;					/* Table operation command direction						*/
	if( TblDrive->conf.RefType == TBLREFTYPE_NOTPOS )	/* Command type: Velocity or Torque				*/
	{
		if( (TblDrive->OpeReg.TblDriveDataSize != TBLREFLENGTH_31BIT_IO)
			&& (TblDrive->OpeReg.TblDriveDataSize != TBLREFLENGTH_32BIT) )
		{ /* SHORT Data */
			KoutDir = KoutDir << 10;					/* 16384 --> 2^24 : 2^10					*/
		}
	}
	TblDrive->conf.KoutDir = KoutDir;					/* Table operation command output gain				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation execution start command processing																*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /* Designed not to be used as it is separated from trace */
	if( TrcReg.TrcMode.b.h == 0x00 )					/* Command table protection mode					*/
	{
		TrcExeP.TrcObj[0].TrcExe |= 0x10;				/* TraceCH0Prohibit execution						*/
	}
	else												/* Command table overwrite mode					*/
	{
		TrcExeP.TrcObj[0].TrcExe &= 0x0F;				/* TraceCH0Execution permission						*/
	}
#endif
	TblDrive->var.TblCmd = TBLCMD_TABLEDRIVE;			/* Table operation execution start command					*/
/*--------------------------------------------------------------------------------------------------*/
	return( PRM_RSLT_SUCCESS );
}


/****************************************************************************************************/
/*																									*/
/*		Table operation stopAPI																	*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	TblDriveSetStopRequest( TBLDRIVE *TblDrive )
{
	TblDrive->var.TblCmd = TBLCMD_NOCMD;

	/* 2011.02.28 Y.Oka Forcibly interlock trace during table operation */
	TblDrive->var.WithTrcReq = FALSE;

	return( PRM_RSLT_SUCCESS );
}


/****************************************************************************************************/
/*																									*/
/*		For table operation control mode acquisitionAPI																*/
/*																									*/
/****************************************************************************************************/
USHORT	TblDriveGetControlMode( TBLDRIVE *TblDrive )
{
	return( TblDrive->conf.CtrlMode );
}


/****************************************************************************************************/
/*																									*/
/*		Table operation simultaneous trace execution request settingAPI													*/
/*																									*/
/****************************************************************************************************/
void	TblDriveSetWithTrcReq( TBLDRIVE *TblDrive, BOOL Switch )
{
	if( Switch == TRUE )
	{ /* Simultaneous trace execution request available */
		TblDrive->var.WithTrcReq = TRUE;
	}
	else
	{ /* No simultaneous trace execution request */
		TblDrive->var.WithTrcReq = FALSE;
	}
}


/***************************************** end of file **********************************************/

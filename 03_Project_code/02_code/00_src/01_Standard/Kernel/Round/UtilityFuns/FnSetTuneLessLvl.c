/****************************************************************************
  Description   : Set Tuning Less Level (Fn200)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.01.31   created

****************************************************************************/
#include "FnSetTuneLessLvl.h"
#include "MercuryGlobals.h"
#include "KLib.h"
#include "PnPrmListTbl.h"


/****************************************************************************************************/
/* Set Tuning Less Level (Fn200)																	*/
/****************************************************************************************************/
static PRM_RSLT	setTLLvlChkCondition( TUNELESS_CTRL *TuneLessCtrl );
static PRM_RSLT	setTLLvlUpdateInEeprom( REGIF_CONFIG_T *RegCfg );
static void		setTLLvlUndoRamGain( REGIF_CONFIG_T *RegCfg );
static PRM_RSLT	setTLLvlSetLevel( REG_ACC_T *AccessPrm, USHORT GainLevel, USHORT JratLevel, USHORT DetLevel );



/****************************************************************************************************/
/*																									*/
/*		Fn200 : Set Tuning Less Level																*/
/*																									*/
/****************************************************************************************************/
/* -------- Fn200 Set Tuning Less Level Begin	-------- */
PRM_RSLT	FnEnterSetTuneLessLevel( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT		rc;
	TUNELESS_CTRL	*TuneLessCtrl;
	PRMDATA			*Prm;

	TuneLessCtrl = &(Axis->BaseControls->TuneLessCtrl);
	Prm = Axis->UniPrms->Prm;
	rc = PRM_RSLT_SUCCESS;

	/* Initialize Register Data */
	RpiRegSetTuneLessLevel( Prm->DatLevel, TuneLessCtrl );

	/* Check Execution Condition */
	rc = setTLLvlChkCondition( TuneLessCtrl );

	/* If in motorless test mode, exit without entering *//* <S1B7> */
	if( Axis->MotorLessTest->MotorLessTestMode == TRUE )
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* FnMode automatic end */
	FunExe->HoldFnMode = TRUE;

	return(rc);
}

/* -------- Fn200 Set Tuning Less Level End	-------- */
void	FnLeaveSetTuneLessLevel( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	REGIF_CONFIG_T	*RegCfg;
	RegCfg = Axis->RegManager->hPnReg;

	/* EEPROMRevert changed parameters with data */
	setTLLvlUndoRamGain( RegCfg );

	/* Fn200Running flagOFF */
	Axis->BaseControls->TuneLessCtrl.var.TuningFuncRun = FALSE;

	/* FnMode automatic end setting */
	FunExe->HoldFnMode = FALSE;
}

/* -------- Fn200 Set Tuning Less Level Execution -------- */
PRM_RSLT	FnExecuteSetTuneLessLevel( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	PRM_RSLT		rc;
	ULONG			ulwork;
	REG_ACC_T		AccessPrm;
	TUNELESS_CTRL	*TuneLessCtrl;
	PRMDATA			*Prm;
	REGIF_CONFIG_T	*RegCfg;

	TuneLessCtrl = &(Axis->BaseControls->TuneLessCtrl);
	Prm = Axis->UniPrms->Prm;
	RegCfg = Axis->RegManager->hPnReg;
	rc = PRM_RSLT_SUCCESS;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr = 0;
	AccessPrm.hRegCfg = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/* Set Auto Notch Status in Register */
	RpiSetRegSpatANotchState( Axis->ANotchSeq );

	switch( FunExe->State )
	{
	case DAT_SEQ_INIT:
		/* Fn200Running flagON */
		TuneLessCtrl->var.TuningFuncRun = TRUE;
		FunExe->State = DAT_SEQ_TUNING;
		break;

	case DAT_SEQ_TUNING:	/* Gain level change process	*/
		switch( FunExe->CmdCode )
		{
		case FCMD_UP:		/* Tuning level valueUP		*/
			if( TuneLessCtrl->OpeReg.DatGnLevel < DAT_GAINLEVEL_MAX )
			{
				TuneLessCtrl->OpeReg.DatGnLevel++;
			}
			else
			{
				TuneLessCtrl->OpeReg.DatGnLevel = DAT_GAINLEVEL_MAX;
			}
			/*------------------------------------------------------------------------------------------*/
			/*	Tuning level setting																	*/
			/*------------------------------------------------------------------------------------------*/
			setTLLvlSetLevel( &(AccessPrm),
							  TuneLessCtrl->OpeReg.DatGnLevel,
							  TuneLessCtrl->OpeReg.DatJratLevel,
							  Prm->DatLevel );

			/*------------------------------------------------------------------------------------------*/
			/*	Notch setting read																		*/
			/*------------------------------------------------------------------------------------------*/
			ulwork = (ULONG)Prm->tfuncsw;

			/*------------------------------------------------------------------------------------------*/
			/*	Automatic setting notch cancellation process																	*/
			/*------------------------------------------------------------------------------------------*/
			if( Prm->anotchsw & 0x0F00 )						/* Extracting automatic setting notch					*/
			{
				ulwork &= 0x0000F0FF;							/* No.2NotchOFF							*/
			}
			RpiResetStatusANotch1( Axis->ANotchSeq );			/* Related status reset				*/
			RpiResetStatusANotch2( Axis->ANotchSeq );			/* Related status reset				*/
//			rc = RpiPrmWriteRamForFn( &pndef_tfuncsw, (USHORT)ulwork, pfexe->App.Hmsg );
			TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)ulwork, 2 );

			/*------------------------------------------------------------------------------------------*/
			/*	Torque filter calculation																		*/
			/*------------------------------------------------------------------------------------------*/
			RpiANotchTrqFilCalc( Axis->ANotchSeq, Axis->ANotch, RegCfg, Prm );

			/*------------------------------------------------------------------------------------------*/
			/*	Clear vibration level																		*/
			/*------------------------------------------------------------------------------------------*/
			DetVibResetMaxMinSpeedError( Axis->DetVib );
			break;

		case FCMD_DOWN:		/* Gain level valueDOWN		*/
			if( TuneLessCtrl->OpeReg.DatGnLevel > DAT_GAINLEVEL_MIN )
			{
				TuneLessCtrl->OpeReg.DatGnLevel--;
			}
			else
			{
				TuneLessCtrl->OpeReg.DatGnLevel = DAT_GAINLEVEL_MIN;
			}
			/*------------------------------------------------------------------------------------------*/
			/*	Tuning level setting																	*/
			/*------------------------------------------------------------------------------------------*/
			setTLLvlSetLevel( &(AccessPrm),
							  TuneLessCtrl->OpeReg.DatGnLevel,
							  TuneLessCtrl->OpeReg.DatJratLevel,
							  Prm->DatLevel );

			/*------------------------------------------------------------------------------------------*/
			/*	Notch setting read																		*/
			/*------------------------------------------------------------------------------------------*/
			ulwork = (ULONG)Prm->tfuncsw;

			/*------------------------------------------------------------------------------------------*/
			/*	Automatic setting notch cancellation process																	*/
			/*------------------------------------------------------------------------------------------*/
			if( Prm->anotchsw & 0x0F00 )						/* Extracting automatic setting notch					*/
			{
				ulwork &= 0x0000F0FF;							/* No.2NotchOFF							*/
			}
			RpiResetStatusANotch1( Axis->ANotchSeq );			/* Related status reset				*/
			RpiResetStatusANotch2( Axis->ANotchSeq );			/* Related status reset				*/
//			rc = RpiPrmWriteRamForFn( &pndef_tfuncsw, (USHORT)ulwork, pfexe->App.Hmsg );
			TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)ulwork, 2 );

			/*------------------------------------------------------------------------------------------*/
			/*	Torque filter calculation																		*/
			/*------------------------------------------------------------------------------------------*/
			RpiANotchTrqFilCalc( Axis->ANotchSeq, Axis->ANotch, RegCfg, Prm );

			/*------------------------------------------------------------------------------------------*/
			/*	Clear vibration level																		*/
			/*------------------------------------------------------------------------------------------*/
			DetVibResetMaxMinSpeedError( Axis->DetVib );
			break;

		case FCMD_STOP:		/* Stop	→ Notch forcedOFF	*/
			/*------------------------------------------------------------------------------------------*/
			/*	Notch setting read																		*/
			/*------------------------------------------------------------------------------------------*/
			ulwork = (ULONG)Prm->tfuncsw;

			/*------------------------------------------------------------------------------------------*/
			/*	Automatic setting notch cancellation process																	*/
			/*------------------------------------------------------------------------------------------*/
			switch( RpiGetANotchPreStat(Axis->ANotchSeq) )
			{ /* Get notch completion status */
				case OPTUNE_PSTS_NT1_COMP:							/* Previous1Row notchON				*/
					ulwork &= 0x0000FFF0;							/* No.1NotchOFF						*/
					RpiResetStatusANotch1( Axis->ANotchSeq );		/* Related status reset			*/
					break;

				case OPTUNE_PSTS_NT2_COMP:							/* Previous2Row notchON				*/
					ulwork &= 0x0000F0FF;							/* No.2NotchOFF						*/
					RpiResetStatusANotch2( Axis->ANotchSeq );		/* Related status reset			*/
					break;

				default :
					break;
			}
//			rc = RpiPrmWriteRamForFn( &pndef_tfuncsw, (USHORT)ulwork, pfexe->App.Hmsg );
			TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)ulwork, 2 );

			/*------------------------------------------------------------------------------------------*/
			/*	Torque filter calculation																		*/
			/*------------------------------------------------------------------------------------------*/
			RpiANotchTrqFilCalc( Axis->ANotchSeq, Axis->ANotch, RegCfg, Prm );

			/*------------------------------------------------------------------------------------------*/
			/*	Clear vibration level																		*/
			/*------------------------------------------------------------------------------------------*/
			DetVibResetMaxMinSpeedError( Axis->DetVib );

			rc = PRM_RSLT_CONTINUE;
			break;

		case FCMD_EXEC:		/* Execution→Level change		*/
			/*------------------------------------------------------------------------------------------*/
			/*	Tuning level setting																	*/
			/*------------------------------------------------------------------------------------------*/
			if( PRM_RSLT_SUCCESS != setTLLvlSetLevel(&(AccessPrm),
													 TuneLessCtrl->OpeReg.DatGnLevel,
													 TuneLessCtrl->OpeReg.DatJratLevel,
													 Prm->DatLevel) )
			{
				rc = PRM_RSLT_CALC_ERR;
			}

			/*------------------------------------------------------------------------------------------*/
			/*	Notch setting read																		*/
			/*------------------------------------------------------------------------------------------*/
			ulwork = (ULONG)Prm->tfuncsw;

			/*------------------------------------------------------------------------------------------*/
			/*	Automatic setting notch cancellation process																	*/
			/*------------------------------------------------------------------------------------------*/
			if( Prm->anotchsw & 0x0F00 )						/* Extracting automatic setting notch					*/
			{
				ulwork &= 0x0000F0FF;							/* No.2NotchOFF							*/
			}
			RpiResetStatusANotch1( Axis->ANotchSeq );			/* Related status reset				*/
			RpiResetStatusANotch2( Axis->ANotchSeq );			/* Related status reset				*/
//			rc = RpiPrmWriteRamForFn( &pndef_tfuncsw, (USHORT)ulwork, pfexe->App.Hmsg );
			rc = TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)ulwork, 2 );
			if( rc != PRM_RSLT_SUCCESS )
			{
				rc = PRM_RSLT_CALC_ERR;
			}

			/*------------------------------------------------------------------------------------------*/
			/*	Torque filter calculation																		*/
			/*------------------------------------------------------------------------------------------*/
			RpiANotchTrqFilCalc( Axis->ANotchSeq, Axis->ANotch, RegCfg, Prm );

			/*------------------------------------------------------------------------------------------*/
			/*	Clear vibration level																		*/
			/*------------------------------------------------------------------------------------------*/
			DetVibResetMaxMinSpeedError( Axis->DetVib );
			break;

		case FCMD_EXEC2 :	/* Execution2				*/
		case FCMD_SV :		/* Servo on key		*/
			/* Forced frequency analysis processing request set */
			RpiANotchFftReqSet( Axis->ANotchSeq, Axis->FftAna );
			rc = PRM_RSLT_CONTINUE;
			break;

		case FCMD_EWRT:		/* EEPROMWrite			*/
			if( setTLLvlUpdateInEeprom(RegCfg) != PRM_RSLT_SUCCESS )
			{ /* EEPROMWhen writing fails */
				rc = PRM_RSLT_CALC_ERR;
			}
			break;

		case FCMD_NONE :
			break;

		default:
			rc = PRM_RSLT_CMD_ERR;
			break;
		}
		break;

	default:
		break;
	}
	return(rc);
}


/****************************************************************************************************/
/*		Gain level(Pn170.2,Pn170.3)Processing 														*/
/****************************************************************************************************/
void	RpiRegSetTuneLessLevel( ULONG Gain, TUNELESS_CTRL *TuneLessCtrl )
{
	LONG	GainLevel;
	LONG	JratLevel;

	/*------------------------------------------------------------------------------------------*/
	/*	Getting level setting																			*/
	/*------------------------------------------------------------------------------------------*/
	GainLevel = (Gain >> 8) & 0x0F;
	JratLevel = (Gain >> 12) & 0x0F;

	/*------------------------------------------------------------------------------------------*/
	/*	Gain level upper and lower limit check																*/
	/*------------------------------------------------------------------------------------------*/
	if( GainLevel > DAT_GAINLEVEL_MAX )
	{
		GainLevel = DAT_GAINLEVEL_MAX;
	}
	else if( GainLevel < DAT_GAINLEVEL_MIN )
	{
		GainLevel = DAT_GAINLEVEL_MIN;
	}

	/*------------------------------------------------------------------------------------------*/
	/*	Load level upper and lower limit check																*/
	/*------------------------------------------------------------------------------------------*/
	if( JratLevel > DAT_JRATLEVEL_MAX )
	{
		JratLevel = DAT_JRATLEVEL_MAX;
	}
	else if( JratLevel < DAT_JRATLEVEL_MIN )
	{
		JratLevel = DAT_JRATLEVEL_MIN;
	}

	/*------------------------------------------------------------------------------------------*/
	/*	Register settings																			*/
	/*------------------------------------------------------------------------------------------*/
	TuneLessCtrl->OpeReg.DatGnLevel = (USHORT)GainLevel;
	TuneLessCtrl->OpeReg.DatJratLevel = (USHORT)JratLevel;

	return;
}


/* -------- Fn200 Check the Condition for Setting Tuning Less Level	-------- */
static PRM_RSLT	setTLLvlChkCondition( TUNELESS_CTRL *TuneLessCtrl )
{
	PRM_RSLT	rc;
	rc = PRM_RSLT_SUCCESS;

	/* When the adjustment-less function is disabled, it is treated as a data error. */
	if( TuneLessCtrl->conf.TuningLessUse != TRUE )
	{
		rc = PRM_RSLT_CONDITION_ERR;
	}
	return rc;
}



/* -------- Fn200 Update Parmeter In EEPROM -------- */
static PRM_RSLT	setTLLvlUpdateInEeprom( REGIF_CONFIG_T *RegCfg )
{
	PRM_RSLT	rc;
	rc = PRM_RSLT_SUCCESS;

	/* Pn170 Dynamic auto tuning related switches */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_DatLevel, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}
	/* Pn401 1st stage 1st torque command filter time constant */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_trqfil11, 0 );
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
	/* Pn409 1st stage notch filter frequency */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}
	/* Pn40A 1st stage notch filter Q value */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchq1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}
	/* Pn40C 2nd stage notch filter frequency */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}
	/* Pn40D 2nd stage notch filter Q value */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchq2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	return(rc);
}





/* -------- Fn200 Write Prm To Ram With EEPROM Data	-------- */
static void	setTLLvlUndoRamGain( REGIF_CONFIG_T *RegCfg )
{
	/* RAMthe value ofEEPROMbe the same as the value of */
	/* Pn170 Dynamic auto tuning related switches */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_DatLevel, 0 );
	/* Pn401 1st stage 1st torque command filter time constant */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_trqfil11, 0 );
	/* Pn408 Torque related function switch */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_tfuncsw, 0 );
	/* Pn409 1st stage notch filter frequency */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil1, 0 );
	/* Pn40A 1st stage notch filter Q value */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchq1, 0 );
	/* Pn40C 2nd stage notch filter frequency */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchfil2, 0 );
	/* Pn40D 2nd stage notch filter Q value */
	PrmReLoadValueFromEeprom( RegCfg, &pndef_ntchq2, 0 );

	return;
}


/****************************************************************************************************/
/*		Adjustment level parameter setting process 																*/
/****************************************************************************************************/
PRM_RSLT	setTLLvlSetLevel( REG_ACC_T *AccessPrm, USHORT GainLevel, USHORT JratLevel, USHORT DetLevel )
{
	PRM_RSLT	rc;
	ULONG		uswk, uswk2;
	ULONG		ulwk;

	rc = PRM_RSLT_SUCCESS;

	uswk = (GainLevel << 8);
	uswk2 = (JratLevel << 12);
	ulwk = (ULONG)DetLevel & 0xFFFF00FF;
	ulwk |= (ULONG)uswk + (ULONG)uswk2;
//	rc = RpiPrmWriteRamForFn( &pndef_DatLevel, ulwk, Hmsg );
	rc = TunePrmWriteRam( AccessPrm, &pndef_DatLevel, ulwk, 2 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return(rc);
	}

	return(rc);
}



/****************************************************************************************************/
/*		Gain level setting process (RegisterIF)															*/
/****************************************************************************************************/
PRM_RSLT	RpiFunSetDATGainLevel( TUNELESS_CTRL *TuneLessCtrl, USHORT Level )
{
	PRM_RSLT	rc;
	rc = PRM_RSLT_SUCCESS;

	/*------------------------------------------------------------------------------------------*/
	/*	Gain level upper and lower limit check																*/
	/*------------------------------------------------------------------------------------------*/
	if( ((LONG)Level >= DAT_GAINLEVEL_MIN) && ((LONG)Level <= DAT_GAINLEVEL_MAX) )
	{
		TuneLessCtrl->OpeReg.DatGnLevel = Level;
	}
	else
	{
		TuneLessCtrl->OpeReg.DatGnLevel = DAT_GAINLEVEL_MIN;
		rc = PRM_RSLT_REGNO_ERR;
	}
	return( rc );
}


/****************************************************************************************************/
/*		Load level setting process (RegisterIF)																*/
/****************************************************************************************************/
PRM_RSLT	RpiFunSetDATJratLevel( TUNELESS_CTRL *TuneLessCtrl, USHORT Level )
{
	PRM_RSLT	rc;
	rc = PRM_RSLT_SUCCESS;

	/*------------------------------------------------------------------------------------------*/
	/*	Load level upper and lower limit check																*/
	/*------------------------------------------------------------------------------------------*/
	if( ((LONG)Level >= DAT_JRATLEVEL_MIN) && ((LONG)Level <= DAT_JRATLEVEL_MAX) )
	{
		TuneLessCtrl->OpeReg.DatJratLevel = Level;
	}
	else
	{
		TuneLessCtrl->OpeReg.DatJratLevel = DAT_JRATLEVEL_MIN;
		rc = PRM_RSLT_REGNO_ERR;
	}
	return( rc );
}



/****************************************************************************************************/

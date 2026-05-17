/****************************************************************************
  Description   : Speed ripple compensation information writing mode (200DH)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2014 All Rights Reserved

  Project       : Mercury

  Create Date   : 2014-02-08

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "FunManager.h"
#include "PnPrmListTbl.h"

/****************************************************************************/
/*		DEFINES																*/
/****************************************************************************/
#define	MOTOR_SN_DATA_SIZE			8		/* Motor serialNo.Data size[word]		*/
#define	RI_PCOMP_NUM_ELE			6		/* Speed ripple compensation information(Electrical angle)Number of components			*/
#define	EEPROM_WRITE_COMPLETE		0x0000	/* Speed ripple compensation information writing confirmation : Complete	*/
#define	EEPROM_WRITE_NOTCOMPLETE	0x0000	/* Speed ripple compensation information writing confirmation : Not completed	*/
#define	EEPROM_WRITE_ERROR			0x0002	/* Speed ripple compensation information writing confirmation : Failure	*/


/****************************************************************************/
/*		PROTOTYPE															*/
/****************************************************************************/
static LONG WriteRippleCompPrmToEeprom( AXIS_HANDLE *Axis );
static LONG WriteMotorSerialNoToEeprom( AXIS_HANDLE *Axis );
static void WriteRippleCompDataToRam( AXIS_HANDLE *Axis );	/* <S163> */

/*******************************************************************************
 200DH: Entry function of Velocity Ripple Compensation Information Write Mode
********************************************************************************/
PRM_RSLT FnEnterRippleComp(FUNEXE *FunExe, AXIS_HANDLE *Axis)/*<S1FD>*/
{
//	PRM_RSLT rc;
//	FUN_CMN_CONTROL	*FnCmnCtrl;/* <S1FD> */
	RPLCMP			*RippleCmp;
	PRMDATA			*Prm;
	REG_ACC_T		AccessPrm;

//	FnCmnCtrl = Axis->FnCmnCtrl;
	RippleCmp = &(Axis->BaseLoops->RippleCmp);
	Prm       = Axis->UniPrms->Prm;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr = 0;
	AccessPrm.hRegCfg = Axis->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/* 200DhForcibly enable speed ripple compensation function during execution */
	RippleCmp->RplCmpPrm.RippleLstWrnMode = RippleCmp->RplCmpPrm.RippleWrnMode; /* Previous value storage */
	RippleCmp->RplCmpPrm.RippleWrnMode    = TRUE/*FALSE*/;

	/* Pn423Recalculation process */
	TunePrmWriteRam( &AccessPrm, &pndef_RippleCompSw, (LONG)Prm->RippleCompSw, 2 );

	FunExe->HoldFnMode = TRUE;								/* FnMode automatic end */
//	FnCmnCtrl->KnlMode = KPI_KMD_RIPPLECOMPWRITE;			/* Speed ripple compensation information writing in progress *//* <S1FD> */
	RippleCmp->RippleCmpState = EEPROM_WRITE_NOTCOMPLETE;	/* Speed ripple compensation information writing confirmation = 01H:Not completed */

//	rc = PRM_RSLT_SUCCESS;
//	return (rc);
	return PRM_RSLT_SUCCESS;
}

/*******************************************************************************
 200DH: Execute function of Velocity Ripple Compensation Information Write Mode
********************************************************************************/
PRM_RSLT FnExecuteRippleComp(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT			errRes;
	LONG				WriteRslt;
//	MENCV				*MencV;		/*<S1FD>*/
//	FUN_CMN_CONTROL		*FnCmnCtrl;	/*<S1FD>*/
	RPLCMP				*RippleCmp;

//	FnCmnCtrl = Axis->FnCmnCtrl;	/*<S1FD>*/
//	MencV = Axis->MencV;			/*<S1FD>*/
	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	switch(FunExe->State)
	{
	case 0:
		switch(FunExe->CmdCode)
		{
/* <S163> Start */
		case FCMD_EXEC:	/* RAMWrite process	*/
			errRes = PRM_RSLT_SUCCESS;			/* Uninitialized variable correction<S191> */
			WriteRippleCompDataToRam( Axis );
			break;
/* <S163> End */

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
		case FCMD_EWRT:	/* EEPROMWrite process	*/
			FunExe->State = 2;
			errRes = PRM_RSLT_CONTINUE;
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

	case 2:	/* Speed ripple compensation information writing process */

			/* Speed ripple compensation parametersEEPROM */
			WriteRslt = WriteRippleCompPrmToEeprom( Axis );

			if( WriteRslt == 0 )
			{/* EEPROMSuccessful writing to  */
				errRes = PRM_RSLT_SUCCESS;
				RippleCmp->RippleCmpState = EEPROM_WRITE_COMPLETE;	/* Speed ripple compensation information writing confirmation = 00H:Complete */
				FunExe->State = 1;
			}
			else
			{/* EEPROM  */
				errRes = PRM_RSLT_CONDITION_ERR;
				RippleCmp->RippleCmpState = EEPROM_WRITE_ERROR;		/* Speed ripple compensation information writing confirmation = 02H:Failure */
			}

			/* Serial number of connected motorEEPROM */
			WriteRslt = WriteMotorSerialNoToEeprom( Axis );

			if( WriteRslt == 0 )
			{/* EEPROMSuccessful writing to  */
				errRes = PRM_RSLT_SUCCESS;
				FunExe->State = 1;
				RippleCmp->RippleCmpState = EEPROM_WRITE_COMPLETE;	/* Speed ripple compensation information writing confirmation = 00H:Complete */
			}
			else
			{/* EEPROM  */
				errRes = PRM_RSLT_CONDITION_ERR;
				RippleCmp->RippleCmpState = EEPROM_WRITE_ERROR;		/* Speed ripple compensation information writing confirmation = 02H:Failure */
			}

		break;

	default:
		errRes = PRM_RSLT_SUCCESS;
		break;
	}

	return errRes;
}

/*******************************************************************************
 200DH: End function of Velocity Ripple Compensation Information Write Mode
********************************************************************************/
void FnLeaveRippleComp(FUNEXE *FunExe, AXIS_HANDLE *Axis)/* <S1FD> */
{
//	FUN_CMN_CONTROL	*FnCmnCtrl;
//	REGIF_CONFIG_T	*RegCfg;					/* <S1FD> */
	RPLCMP			*RippleCmp;
	PRMDATA			*Prm;
	REG_ACC_T		AccessPrm;

//	FnCmnCtrl = Axis->FnCmnCtrl;
	RippleCmp = &(Axis->BaseLoops->RippleCmp);
	Prm       = Axis->UniPrms->Prm;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr = 0;
	AccessPrm.hRegCfg = Axis->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/* Pn423of200DhReturn to pre-execution state */
	RippleCmp->RplCmpPrm.RippleWrnMode = RippleCmp->RplCmpPrm.RippleLstWrnMode; /* Return previous value */

	/* Pn423Recalculation process */
	TunePrmWriteRam( &AccessPrm, &pndef_RippleCompSw, (LONG)Prm->RippleCompSw, 2 );

	FunExe->HoldFnMode = FALSE;				/* Escape Fn mode */
//	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;	/* normal mode *//* <S1FD> */
}

/* <S163> Start */
/****************************************************************************************************/
/*																									*/
/*		Speed ripple compensation componentRAMprocess													*/
/*																									*/
/*			Machine Efficiency :	SigmaWin+and written in the register									*/
/*					"Speed ripple compensation component"RAM.										*/
/*																									*/
/****************************************************************************************************/
static void WriteRippleCompDataToRam( AXIS_HANDLE *Axis )
{
	PRMDATA			*Prm;
	RPLCMP			*RippleCmp;
	LONG			i;

	/* Initialize */
	Prm = Axis->UniPrms->Prm;
	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	/*----------------------------------------------------------------------------------------------*/
	/*		Ripple compensation parameter copy(Register⇒RAM)												*/
	/*----------------------------------------------------------------------------------------------*/
	Prm->RippleCompGainE = RippleCmp->RplCmpPrm.RplCompGainEle;

	for( i = 0; i < RI_PCOMP_NUM_ELE; i++ )
	{	/* Electrical angle correction */
		Prm->RippleCompCycleE[i]     = RippleCmp->RplCmpPrm.RplCompCycleEle[i];
		Prm->RippleCompAmplitudeE[i] = RippleCmp->RplCmpPrm.RplCompAmplitudeEle[i];
		Prm->RippleCompPhaseE[i]     = RippleCmp->RplCmpPrm.RplCompPhaseEle[i];
	}

	return;
}
/* <S163> End */

/****************************************************************************************************/
/*																									*/
/*		Speed ripple compensation parametersEEPROMprocess											*/
/*																									*/
/*			Machine Efficiency :	SigmaWin+,PnF**										*/
/*					"Speed ripple compensation parameter"EepRom.							*/
/*																									*/
/****************************************************************************************************/
static LONG WriteRippleCompPrmToEeprom( AXIS_HANDLE *Axis )
{
	LONG			rc;
	REGIF_CONFIG_T	*RegCfg;
//	PRMDATA			*Prm;		/* <S163> Delete */
//	RPLCMP			*RippleCmp;	/* <S1FD> */
//	LONG			i;			/* <S1FD> */

	/* Initialize */
	rc = PRM_RSLT_SUCCESS;
	RegCfg = Axis->RegManager->hPnReg;
//	Prm = Axis->UniPrms->Prm;					/* <S163> Delete */
//	RippleCmp = &(Axis->BaseLoops->RippleCmp);	/* <S1FD> */

/* <S163> Start */
/* Ripple compensation parameter copy(Register⇒RAM)says "Operation setting register(42H:0x80004002/40H:0x2001) = 0x0001(Execution)" Specification changed to execute. */
#if 0
	/*----------------------------------------------------------------------------------------------*/
	/*		Ripple compensation parameter copy(Register⇒RAM)												*/
	/*----------------------------------------------------------------------------------------------*/
	Prm->RippleCompGainE = RippleCmp->RplCmpPrm.RplCompCycleEle[i];

	for( i = 0; i < RI_PCOMP_NUM_ELE; i++ )
	{	/* Electrical angle correction */
		Prm->RippleCompCycleE[i] = RippleCmp->RplCmpPrm.RplCompCycleEle[i];
		Prm->RippleCompAmplitudeE[i] = RippleCmp->RplCmpPrm.RplCompAmplitudeEle[i];
		Prm->RippleCompPhaseE[i] = RippleCmp->RplCmpPrm.RplCompPhaseEle[i];
	}
#endif
/* <S163> End */

	/*----------------------------------------------------------*/
	/*		EEPROMprocess								*/
	/*----------------------------------------------------------*/
	/* PnFA0 Speed ripple compensation gain(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompGainE, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA1 Speed ripple compensation number1Component period(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompCycle1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA2 Speed ripple compensation number1Component amplitude value(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompAmplitude1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA3 Speed ripple compensation number1Component phase(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompPhase1, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA4 Speed ripple compensation number2Component period(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompCycle2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA5 Speed ripple compensation number2Component amplitude value(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompAmplitude2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA6 Speed ripple compensation number2Component phase(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompPhase2, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA7 Speed ripple compensation number3Component period(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompCycle3, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA8 Speed ripple compensation number3Component amplitude value(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompAmplitude3, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFA9 Speed ripple compensation number3Component phase(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompPhase3, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFAA Speed ripple compensation number4Component period(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompCycle4, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFAB Speed ripple compensation number4Component amplitude value(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompAmplitude4, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFAC Speed ripple compensation number4Component phase(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompPhase4, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFAD Speed ripple compensation number5Component period(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompCycle5, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFAE Speed ripple compensation number5Component amplitude value(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompAmplitude5, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFAF Speed ripple compensation number5Component phase(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompPhase5, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFB0 Speed ripple compensation number6Component period(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompCycle6, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFB1 Speed ripple compensation number6Component amplitude value(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompAmplitude6, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	/* PnFB2 Speed ripple compensation number6Component phase(Electrical angle) */
	rc = PrmStoreValueToEeprom( RegCfg, &pndef_RippleCompPhase6, 0 );
	if( rc != PRM_RSLT_SUCCESS )
	{
		return( rc );
	}

	return(rc);
}

/****************************************************************************************************/
/*																									*/
/*		Serial number of connected motorEEPROMprocess												*/
/*																									*/
/*			Machine Efficiency :	Serial number of connected motorEepRom.									*/
/*																									*/
/****************************************************************************************************/
static LONG WriteMotorSerialNoToEeprom( AXIS_HANDLE *Axis )
{

	LONG	rc;
//	EEP_DEVICE_HANDLE	*EepDevice;/* <Oka02> */
	UINT				AxisNo;
	USHORT				SerialNo[MOTOR_SN_DATA_SIZE];
	LONG				ArrayIdx;

	/* Initialize */
//	EepDevice = (Axis->RegManager->hPnReg)->hEepDev;/* <Oka02> */
	AxisNo = Axis->AxisNo;
	rc = PRM_RSLT_SUCCESS;

	/* Motor serialNo.Acquisition */
	for(ArrayIdx = 0;  ArrayIdx < MOTOR_SN_DATA_SIZE; ArrayIdx++)
	{
		SerialNo[ArrayIdx] = (USHORT)(Axis->MencV->MotInfo.SNo[ArrayIdx*2 + 1] & 0x00FF);
		SerialNo[ArrayIdx] |= (USHORT)((Axis->MencV->MotInfo.SNo[ArrayIdx*2] & 0x00FF) << 8);
	}

	/* Motor serialNo.ofEepRom */
	for(ArrayIdx = 0;  ArrayIdx < MOTOR_SN_DATA_SIZE; ArrayIdx++)
	{
		/* write to EEPROM */
		rc = EepdevWriteNoSumValues( 0,															/* EEPROM Device ID(0: SPI(IF), 1: I2C(SDA) */
									 EEP_NOSUM_ADDRESS(AxisNo, EEP_MOTORSN_OFS) + (ArrayIdx*2),	/* Eeprom Address in bytes */
									 &SerialNo[ArrayIdx],										/* Pointer to the buffer for writing */
									 1 );														/* writing size in words */

		if( rc != 0 )	/* EEPROMfailed */
		{
			break;
		}

	}

	return (rc) ;
}
/***************************************** end of file **********************************************/

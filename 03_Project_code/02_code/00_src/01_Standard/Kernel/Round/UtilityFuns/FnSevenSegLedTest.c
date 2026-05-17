/****************************************************************************
  Description   : Push button/7SEG-LEDConfirmation mode (Fn3000)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2013 All Rights Reserved

  Project       : Mercury

  Create Date   : 2013-12-xx

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "FunManager.h"
#include "JL086ServoIpHw.h"

#include "PnPrmListTbl.h"
#include "KLib.h"
#include "PnlOp.h"
#include "HwDeviceIf.h"


/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
/* Manual zero adjustment of analog monitor output Maximum value/Minimum value */
#define	ZADJAM_MAX			127
#define	ZADJAM_MIN			-128

/* Manual gain adjustment of analog monitor output Maximum value/Minimum value */
#define	GADJAM_MAX			127
#define	GADJAM_MIN			-128


#define	OK	1					/* Executable */
#define	NG	0					/* Not executable */


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/



/****************************************************************************
 Fn3000: Entry function of 7 SegLed And Push Button Check Test
****************************************************************************/
PRM_RSLT FnEnterSevenSegLedChkTest(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT rc;
	FUN_CMN_CONTROL	*FnCmnCtrl;
	PRMDATA			*Prm;

	Prm = Axis->UniPrms->Prm;
	FnCmnCtrl = Axis->FnCmnCtrl;

	/* Ends without turning on when servo is on	*/
//	if(FnCmnCtrl->State.FnSvCtrlDisable != FALSE)
//	{
//		return PRM_RSLT_CONDITION_ERR;
//	}

	FnCmnCtrl->KnlMode = KPI_KMD_PBLEDTEST;		/* Push button/7SEG-LEDConfirming  */
	FunExe->HoldFnMode = TRUE;					/* FnMode automatic end */
	rc = PRM_RSLT_SUCCESS;

	return (rc);
}

/****************************************************************************
 Fn3000: Execute function of 7 SegLed And Push Button Check Test
****************************************************************************/
PRM_RSLT FnExecuteSevenSegLedChkTest(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT				rc;
	CMN_PRMDATA				*Cprm;
	PRMDATA					*Prm;
	FUN_CMN_CONTROL			*FnCmnCtrl;

	Prm = Axis->UniPrms->Prm;
	Cprm = Axis->UniPrms->Cprm;
	FnCmnCtrl = Axis->FnCmnCtrl;
	rc = PRM_RSLT_SUCCESS;

	/* Wait until the execution request comes.	*/
	if(FunExe->CmdCode != FCMD_EXEC)
	{
		rc = PRM_RSLT_CONTINUE;
	}
	else
	{
		/*	When the execution request comes, push the button&7SegLedConfirmation flagTrueonly for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.
			Actual processing isScanC'sSyscSequenceCommon()” to execute.(NormalLEDOutput processing is not performed)
		*/
		FnCmnCtrl->PbAndLedChkTestFlag = 0x01;


	/* APShape Push button&7SegLedStart flagFalseonly for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.*/
	/* Actual processing isScanC'sPnlStatusDispMode()” to execute.(NormalLEDOutput processing is not performed) */

		FnCmnCtrl->AnlgLedChkInitFlag = 0x00;	

		/* APShape Push button&7SegLedConfirm Counter Clear*/
		FnCmnCtrl->AnlgLedChkCnt = 0x00;

		rc = PRM_RSLT_SUCCESS;
	}
//		return PRM_RSLT_CONTINUE;
	return(rc);

}

/****************************************************************************
 Fn3000: End function of 7 SegLed And Push Button Check Test
****************************************************************************/
void FnLeaveSevenSegLedChkTest(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;
	RegCfg = Axis->RegManager->hPnReg;

	/* Push button&7SegLedConfirmation flagFalseonly for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.*/
	FnCmnCtrl->PbAndLedChkTestFlag = 0x00;

	/* APShape Push button&7SegLedStart flagFalseonly for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.*/
	FnCmnCtrl->AnlgLedChkInitFlag = 0x00;

	/* APShape Push button&7SegLedConfirm Counter Clear*/
	FnCmnCtrl->AnlgLedChkCnt = 0x00;

	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;					/* normal mode */
}

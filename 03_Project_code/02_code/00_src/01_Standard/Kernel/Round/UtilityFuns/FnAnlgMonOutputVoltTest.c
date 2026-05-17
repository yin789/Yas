/****************************************************************************
  Description   : Analog monitor output voltage confirmation test mode (Fn3004)	<S0FD>
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2013 All Rights Reserved

  Project       : Mercury

  Create Date   : 2014-01-23

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
#define	TRUE	1					/* Executable */
#define	FALSE	0					/* Not executable */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/

/****************************************************************************
 Fn3004: Entry function of Analog Monitor Output Volt Check Test
****************************************************************************/
PRM_RSLT FnEnterAnlgMonOutputVoltTest(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT rc;
	FUN_CMN_CONTROL	*FnCmnCtrl;
	PRMDATA			*Prm;

	Prm = Axis->UniPrms->Prm;
	FnCmnCtrl = Axis->FnCmnCtrl;

	FnCmnCtrl->KnlMode = KPI_KMD_ANLGMONVOLTTEST;	/* Analog monitor output voltage test in progress */

	FunExe->HoldFnMode = TRUE;						/* FnMode automatic end */
	rc = PRM_RSLT_SUCCESS;

	return (rc);
}

/****************************************************************************
 Fn3004: Execute function of Analog Monitor Output Volt Check Test
****************************************************************************/
PRM_RSLT FnExecuteAnlgMonOutputVoltTest(FUNEXE *FunExe, AXIS_HANDLE *Axis)
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
		/* Analog monitor output voltage confirmation test flag = True */
		FnCmnCtrl->AnlgMonVoltTstFlag = TRUE;

		/*	★Actual processing isScanBBpxOutputAnalogMonitor():Executed every scan in "Analog monitor output processing"	*/
		/* 	(Normal analog monitor output processing is not executed until this auxiliary function is finished.)										*/

		rc = PRM_RSLT_SUCCESS;
	}

	return(rc);

}

/****************************************************************************
 Fn3004: End function of Analog Monitor Output Volt Check Test
****************************************************************************/
void FnLeaveAnlgMonOutputVoltTest(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;
	RegCfg = Axis->RegManager->hPnReg;

	/* Analog monitor output voltage confirmation test flag = False */
	FnCmnCtrl->AnlgMonVoltTstFlag = FALSE;

	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;					/* normal mode */
}

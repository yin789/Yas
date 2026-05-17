/*****************************************************************************
Description		: Built-in FAN Lifetime Reset (Fn110)	
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
K.Liu		2014.03.05		created

*****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"


#define		TEN_HOURS_TO_100MS		360000		/* Ten Hours Convert To 100ms */


/****************************************************************************************************/
/*																									*/
/*		Function List																				*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Public Function																				*/
/*--------------------------------------------------------------------------------------------------*/
PRM_RSLT FnExecuteBFanLtRst(FUNEXE *FunExe, AXIS_HANDLE *Axis);	/* Execute Function of Built-in FAN Lifetime Reset*/



/*****************************************************************************
  Fn110: Execute Function of Built-in FAN Lifetime Reset
------------------------------------------------------------------------------
  Parameters:
	FunExe;		Referenced by auxiliary function application side/Operable members
	Axis;		Global data(Axis unit)
  Return:
	rc;			Error Status
------------------------------------------------------------------------------
  Note:
	
*****************************************************************************/
PRM_RSLT FnExecuteBFanLtRst(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT		rc;
	CMN_PRMDATA		*Cprm;
	ULONG 			CurrentTimerCnt;	/* Current operating time	[100ms]*/
	USHORT 			FanOffsetTimer;		/* FAN Replacement offset time	[10h]*/
	FANLIFETIME_HADNLE		*hFanLifeTime;
	
	rc	 = PRM_RSLT_SUCCESS;
	Cprm = Axis->UniPrms->Cprm;
	hFanLifeTime = &(Axis->hLifeTime->FanLifeTimeHdl);
	
	
	/* Wait until the execution request comes.	*/
	if(FunExe->CmdCode != FCMD_EXEC)
	{
		return PRM_RSLT_CONTINUE;
	}

	CurrentTimerCnt = KlibGetTimeStamp();		/* Getting current operating time */
	FanOffsetTimer = CurrentTimerCnt / TEN_HOURS_TO_100MS;		/* [100ms]->[10h] */
	if ( Cprm->FanTimeOffset != FanOffsetTimer )
	{
		Cprm->FanTimeOffset = FanOffsetTimer;
		rc = PrmStoreValueToEeprom( Axis->RegManager->hPnReg, &pndef_FanTimeOffset, 0 );
		if ( rc == PRM_RSLT_SUCCESS )
		{
			hFanLifeTime->conf.FanTimeOffset = (float)FanOffsetTimer;
		}
	}
	
	
	return (rc);
}



/***************************************** end of file **********************************************/

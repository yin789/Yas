/****************************************************************************
  Description   : automatic testing machineA/D Gain adjustment mode (Fn3001, Fn3002,Fn3003)
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


/****************************************************************************/
/*		DEFINES																*/
/****************************************************************************/


/****************************************************************************/
/*		PROTOTYPE															*/
/****************************************************************************/


/*--------------------------------------------------------------------------------------------------*/
/****************************************************************************
 Fn3001: Entry function of Adjust Speed Ref.A/D Gain
****************************************************************************/
PRM_RSLT FnEnterAdjustVrefGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{

	PRM_RSLT rc;
	FUN_CMN_CONTROL	*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */
//	FnCmnCtrl->KnlMode = KPI_KMD_ADJGAIN;		/* A/D During gain adjustment  */
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */	/* <S142> */
//	FunExe->HoldFnMode = TRUE;					/* FnMode automatic end */	/* <S0EB> */
	rc = PRM_RSLT_SUCCESS;
#else											/* Analog IF  */
	rc = PRM_RSLT_SUCCESS;
#endif
	return (rc);
}

/****************************************************************************
 Fn3001: Execute function of Adjust Speed Ref.A/D Gain
****************************************************************************/
PRM_RSLT FnExecuteAdjustVrefGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT				rc;
	CMN_PRMDATA				*Cprm;
	PRMDATA					*Prm;
	FUN_CMN_CONTROL			*FnCmnCtrl;
	LONG					Adj_Result;

	Prm = Axis->UniPrms->Prm;
	Cprm = Axis->UniPrms->Cprm;
	FnCmnCtrl = Axis->FnCmnCtrl;
	rc = PRM_RSLT_SUCCESS;

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */
	/* Wait until the execution request comes.	*/
	if(FunExe->CmdCode != FCMD_EXEC)
	{
		rc = PRM_RSLT_CONTINUE;
	}
	else
	{
#if( SVFSEL_ADC_USE==1 )       /* <S19A> */
		/* When the execution request comes, execute "Speed command AD gain adjustment." */
		Adj_Result = TciAdjustVrefGain( Axis );
#endif
		FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */	/* <S0EB> */

		if( Adj_Result == KPI_ADC_ADJOK )
		{
			rc = PRM_RSLT_SUCCESS;			/* Normal end */
		}
		else
		{
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
		}
	}
#else											/* Analog IF  */
	rc = PRM_RSLT_SUCCESS;
#endif
	return(rc);

}

#if 0	/* <S0EB> Delete:Comment out this process to automatically terminate the adjustment function. */
/****************************************************************************
 Fn3001: End function of Adjust Speed Ref.A/D Gain
****************************************************************************/
void FnLeaveAdjustVrefGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */
	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */
#endif
}
#endif	/* <S0EB> Delete:Comment out this process to automatically terminate the adjustment function. */

/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************
 Fn3002: Entry function of Adjust Torque Positive Ref.A/D Gain
****************************************************************************/
PRM_RSLT FnEnterAdjustTrefPGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT rc;
	FUN_CMN_CONTROL	*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */
//	FnCmnCtrl->KnlMode = KPI_KMD_ADJGAIN;		/* A/D During gain adjustment  */
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */	/* <S142> */
//	FunExe->HoldFnMode = TRUE;					/* FnMode automatic end */	/* <S0EB> */
	rc = PRM_RSLT_SUCCESS;
#else											/* Analog IF  */
	rc = PRM_RSLT_SUCCESS;
#endif
	return (rc);
}

/****************************************************************************
 Fn3002: Execute function of Adjust Torque Positive Ref.A/D Gain
****************************************************************************/
PRM_RSLT FnExecuteAdjustTrefPGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT				rc;
	CMN_PRMDATA				*Cprm;
	PRMDATA					*Prm;
	FUN_CMN_CONTROL			*FnCmnCtrl;
	LONG					Adj_Result;

	Prm = Axis->UniPrms->Prm;
	Cprm = Axis->UniPrms->Cprm;
	FnCmnCtrl = Axis->FnCmnCtrl;
	rc = PRM_RSLT_SUCCESS;

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */
	/* Wait until the execution request comes.	*/
	if(FunExe->CmdCode != FCMD_EXEC)
	{
		rc = PRM_RSLT_CONTINUE;
	}
	else
	{
		/* When the execution request comes, execute "Torque command AD positive side gain adjustment." */
#if( SVFSEL_ADC_USE==1 )     /* <S19A> */
		Adj_Result = TciAdjustTrefPGain( Axis );
#endif
		FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */	/* <S0EB> */

		if( Adj_Result == KPI_ADC_ADJOK )
		{
			rc = PRM_RSLT_SUCCESS;			/* Normal end */
		}
		else
		{
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
		}
	}
#else											/* Analog IF  */
	rc = PRM_RSLT_SUCCESS;
#endif
	return(rc);

}

#if 0	/* <S0EB> Delete:Comment out this process to automatically terminate the adjustment function. */
/****************************************************************************
 Fn3002: End function of Adjust Torque Positive Ref.A/D Gain
****************************************************************************/
void FnLeaveAdjustTrefPGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */
	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */
#endif
}
#endif	/* <S0EB> Delete:Comment out this process to automatically terminate the adjustment function. */
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************
 Fn3003: Entry function of Adjust Torque Negative Ref.A/D Gain
****************************************************************************/
PRM_RSLT FnEnterAdjustTrefNGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT rc;
	FUN_CMN_CONTROL	*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */

//	FnCmnCtrl->KnlMode = KPI_KMD_ADJGAIN;		/* A/D During gain adjustment  */
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */	/* <S142> */
//	FunExe->HoldFnMode = TRUE;					/* FnMode automatic end */	/* <S0EB> */
	rc = PRM_RSLT_SUCCESS;
#else											/* Analog IF  */
	rc = PRM_RSLT_SUCCESS;
#endif

	return (rc);
}

/****************************************************************************
 Fn3003: Execute function of Adjust Torque Negative Ref.A/D Gain
****************************************************************************/
PRM_RSLT FnExecuteAdjustTrefNGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT				rc;
	CMN_PRMDATA				*Cprm;
	PRMDATA					*Prm;
	FUN_CMN_CONTROL			*FnCmnCtrl;
	LONG					Adj_Result;

	Prm = Axis->UniPrms->Prm;
	Cprm = Axis->UniPrms->Cprm;
	FnCmnCtrl = Axis->FnCmnCtrl;
	rc = PRM_RSLT_SUCCESS;

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */
	/* Wait until the execution request comes.	*/
	if(FunExe->CmdCode != FCMD_EXEC)
	{
		rc = PRM_RSLT_CONTINUE;
	}
	else
	{
		/* When the execution request comes, execute "Torque command AD negative side gain adjustment." */
#if( SVFSEL_ADC_USE==1 )      /* <S19A> */
		Adj_Result = TciAdjustTrefNGain( Axis );
#endif
		FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */	/* <S0EB> */

		if( Adj_Result == KPI_ADC_ADJOK )
		{
			rc = PRM_RSLT_SUCCESS;			/* Normal end */
		}
		else
		{
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
		}
	}
#else										/* Analog IF  */
	rc = PRM_RSLT_SUCCESS;
#endif
	return(rc);

}

#if 0	/* <S0EB> Delete:Comment out this process to automatically terminate the adjustment function. */
/****************************************************************************
 Fn3003: End function of Adjust Torque Negative Ref.A/D Gain
****************************************************************************/
void FnLeaveAdjustTrefNGain(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)		/* Analog IF */
	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */
#endif
}
#endif	/* <S0EB> Delete:Comment out this process to automatically terminate the adjustment function. */

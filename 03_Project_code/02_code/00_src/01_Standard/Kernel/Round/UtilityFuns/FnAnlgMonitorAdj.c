/****************************************************************************
  Description   : Manual zero adjustment of analog monitor output (Fn00C)
                : Manual gain adjustment of analog monitor output (Fn00D)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2013 All Rights Reserved

  Project       : Mercury

  Create Date   : 2013-12-04

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "FunManager.h"
#include "JL086ServoIpHw.h"

#include "PnPrmListTbl.h"
#include "KLib.h"


/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
//#define KPI_RC_NOTADJ	0		/* Adjustment execution condition not met */
//#define KPI_RC_ADJOK	1		/* Adjustment completed */
//#define KPI_RC_ADJERR	-1		/* Adjustment failed */
//#define OFFSET_ADJ		0		/* Current detection offset adjustment */

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
 Fn00C: Entry function of Analog Monitor Offset Manual Adjustment
****************************************************************************/
PRM_RSLT FnEnterAnlgMonOffsetsManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT rc;
	FUN_CMN_CONTROL	*FnCmnCtrl;
	PRMDATA			*Prm;

	Prm = Axis->UniPrms->Prm;
	FnCmnCtrl = Axis->FnCmnCtrl;

	rc = PRM_RSLT_SUCCESS;

	/* User constant writing prohibited Confirm */
	if(Prm->syssw3 & 0x0001)
	{
		rc = PRM_RSLT_RWACC_ERR;					/* Access restriction error */
	}
	else
	{
//		FnCmnCtrl->KnlMode = KPI_KMD_ADJANLGMON;	/* Analog monitor adjusting  */
		FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */			/* <S142> */
		FunExe->HoldFnMode = TRUE;					/* FnMode automatic end */
		rc = PRM_RSLT_SUCCESS;
	}

	return (rc);
}

/****************************************************************************
 Fn00C: Execute function of Analog Monitor Offset Manual Adjustment
****************************************************************************/
PRM_RSLT FnExecuteAnlgMonOffsetsManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	CMN_PRMDATA		*Cprm;
	PRMDATA			*Prm;
	PRM_RSLT		rc;
	LONG			mnaz_Ch1_tmp;
	LONG			mnaz_Ch2_tmp;
	UCHAR			Exe_flg;

	Prm = Axis->UniPrms->Prm;
	Cprm = Axis->UniPrms->Cprm;

	rc = PRM_RSLT_SUCCESS;
	Exe_flg = NG;

	/* User constant writing prohibited Confirm */
#if 0
	if(Prm->syssw3 & 0x0001)
	{
		Exe_flg = NG;
	}
	else
	{
		Exe_flg = OK;
	}
#else 
	Exe_flg = OK;
#endif

	switch( FunExe->State )
	{
	case 0:	/* Analog monitor(CH1)Zero adjustment data Adjust	*/
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:
			if(Exe_flg == OK)
			{
				/* PnE58:Monitor1,2Zero adjustment(Lowerbyte) */
				mnaz_Ch1_tmp = (LONG)(CHAR)((Cprm->mnaz0_1.b.l) & 0xFF);

				if(mnaz_Ch1_tmp < ZADJAM_MAX)
				{
					mnaz_Ch1_tmp += 1;
				}

				/* PnE58:Monitor1,2Zero adjustment(Lowerbyte)RAM */
				Cprm->mnaz0_1.b.l = (UCHAR)mnaz_Ch1_tmp;
			}
			else
			{
				rc = PRM_RSLT_RWACC_ERR;	/* Access restriction error */
			}
			break;

		case FCMD_DOWN:
			if(Exe_flg == OK)
			{
				/* PnE58:Monitor1,2Zero adjustment(Lowerbyte) */
				mnaz_Ch1_tmp = (LONG)(CHAR)((Cprm->mnaz0_1.b.l) & 0xFF);

				if(mnaz_Ch1_tmp > ZADJAM_MIN)
				{
					mnaz_Ch1_tmp -= 1;
				}

				/* PnE58:Monitor1,2Zero adjustment(Lowerbyte)RAM */
				Cprm->mnaz0_1.b.l = (UCHAR)mnaz_Ch1_tmp;
			}
			else
			{
				rc = PRM_RSLT_RWACC_ERR;	/* Access restriction error */
			}
			break;

		case FCMD_CH1:
			FunExe->State = 0;	/* Analog monitor(CH1)Zero adjustment data Adjust */
			break;

		case FCMD_CH2:
			FunExe->State = 1;	/* Analog monitor(CH2)Zero adjustment data Adjust */
			break;

		default:	/* other command */
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
			break;
		}
		break;

	case 1:	/* Analog monitor(CH2)Zero adjustment data Adjust */
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:
			if(Exe_flg == OK)
			{
				/* PnE58:Monitor1,2Zero adjustment(Upper levelbyte) */
				mnaz_Ch2_tmp = (LONG)(CHAR)((Cprm->mnaz0_1.b.h) & 0xFF);

				if(mnaz_Ch2_tmp < ZADJAM_MAX)
				{
					mnaz_Ch2_tmp += 1;
				}

				/* PnE58:Monitor1,2Zero adjustment(Upper levelbyte)RAM */
				Cprm->mnaz0_1.b.h = (UCHAR)mnaz_Ch2_tmp;
			}
			else
			{
				rc = PRM_RSLT_RWACC_ERR;	/* Access restriction error */
			}
			break;

		case FCMD_DOWN:
			if(Exe_flg == OK)
			{
				/* PnE58:Monitor1,2Zero adjustment(Upper levelbyte) */
				mnaz_Ch2_tmp = (LONG)(CHAR)((Cprm->mnaz0_1.b.h) & 0xFF);

				if(mnaz_Ch2_tmp > ZADJAM_MIN)
				{
					mnaz_Ch2_tmp -= 1;
				}

				/* PnE58:Monitor1,2Zero adjustment(Upper levelbyte)RAM */
				Cprm->mnaz0_1.b.h = (UCHAR)mnaz_Ch2_tmp;
			}
			else
			{
				rc = PRM_RSLT_RWACC_ERR;	/* Access restriction error */
			}
			break;

		case FCMD_CH1:
			FunExe->State = 0;	/* Analog monitor(CH1)Zero adjustment data Adjust */
			break;

		default:	/* other command */
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
			break;
		}
		break;

	default:
		break;
	}

	return rc ;

}

/****************************************************************************
 Fn00C: End function of Analog Monitor Offset Manual Adjustment
****************************************************************************/
void FnLeaveAnlgMonOffsetsManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;
	RegCfg = Axis->RegManager->hPnReg;

	/* Termination processing		*/
	/* EEPROMUpdate process	*/
	PrmStoreValueToEeprom( RegCfg, &pndef_mnaz0_1, 0 );		/* PnE58:Monitor1,2Zero adjustment */

	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;					/* normal mode */
}

/****************************************************************************
 Fn00D: Entry function of Analog Monitor Gain Manual Adjustment
****************************************************************************/
PRM_RSLT FnEnterAnlgMonGainManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRM_RSLT rc;
	FUN_CMN_CONTROL	*FnCmnCtrl;
	PRMDATA			*Prm;

	Prm = Axis->UniPrms->Prm;
	FnCmnCtrl = Axis->FnCmnCtrl;

	rc = PRM_RSLT_SUCCESS;

	/* User constant writing prohibited Confirm */
	if(Prm->syssw3 & 0x0001)
	{
		rc = PRM_RSLT_RWACC_ERR;					/* Access restriction error */
	}
	else
	{
//		FnCmnCtrl->KnlMode = KPI_KMD_ADJANLGMON;	/* Analog monitor adjusting  */
		FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */			/* <S142> */
		FunExe->HoldFnMode = TRUE;					/* FnMode automatic end */
		rc = PRM_RSLT_SUCCESS;
	}


	return (rc);
}

/****************************************************************************
 Fn00D: Execute function of Analog Monitor Gain Manual Adjustment
****************************************************************************/
PRM_RSLT FnExecuteAnlgMonGainManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	CMN_PRMDATA		*Cprm;
	PRMDATA			*Prm;
	PRM_RSLT		rc;
	LONG			mnag_Ch1_tmp;
	LONG			mnag_Ch2_tmp;
	UCHAR			Exe_flg;

	Prm = Axis->UniPrms->Prm;
	Cprm = Axis->UniPrms->Cprm;

	rc = PRM_RSLT_SUCCESS;
	Exe_flg = NG;

	/* User constant writing prohibited Confirm */
#if 0
	if(Prm->syssw3 & 0x0001)
	{
		Exe_flg = NG;
	}
	else
	{
		Exe_flg = OK;
	}
#else
	Exe_flg = OK;
#endif

	switch( FunExe->State )
	{
	case 0:	/* Analog monitor(CH1)Gain adjustment data Adjust	*/
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:
			if(Exe_flg == OK)
			{
				/* PnE59:Monitor1,2Gain adjustment(Lowerbyte) */
				mnag_Ch1_tmp = (LONG)(CHAR)((Cprm->mnag0_1.b.l) & 0xFF);

				if(mnag_Ch1_tmp < GADJAM_MAX)
				{
					mnag_Ch1_tmp += 1;
				}

				/* PnE59:Monitor1,2Gain adjustment(Lowerbyte)RAM */
				Cprm->mnag0_1.b.l = (UCHAR)mnag_Ch1_tmp;
			}
			else
			{
				rc = PRM_RSLT_RWACC_ERR;	/* Access restriction error */
			}
			break;

		case FCMD_DOWN:
			if(Exe_flg == OK)
			{
				/* PnE59:Monitor1,2Gain adjustment(Lowerbyte) */
				mnag_Ch1_tmp = (LONG)(CHAR)((Cprm->mnag0_1.b.l) & 0xFF);

				if(mnag_Ch1_tmp > GADJAM_MIN)
				{
					mnag_Ch1_tmp -= 1;
				}

				/* PnE59:Monitor1,2Gain adjustment(Lowerbyte)RAM */
				Cprm->mnag0_1.b.l = (UCHAR)mnag_Ch1_tmp;
			}
			else
			{
				rc = PRM_RSLT_RWACC_ERR;	/* Access restriction error */
			}
			break;

		case FCMD_CH1:
			FunExe->State = 0;	/* Analog monitor(CH1)Gain adjustment data Adjust */
			break;

		case FCMD_CH2:
			FunExe->State = 1;	/* Analog monitor(CH2)Gain adjustment data Adjust */
			break;

		default:	/* other command */
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
			break;
		}
		break;

	case 1:	/* Analog monitor(CH2)Gain adjustment data Adjust */
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:
			if(Exe_flg == OK)
			{
				/* PnE59:Monitor1,2Gain adjustment(Upper levelbyte) */
				mnag_Ch2_tmp = (LONG)(CHAR)((Cprm->mnag0_1.b.h) & 0xFF);

				if(mnag_Ch2_tmp < GADJAM_MAX)
				{
					mnag_Ch2_tmp += 1;
				}

				/* PnE59:Monitor1,2Gain adjustment(Upper levelbyte)RAM */
				Cprm->mnag0_1.b.h = (UCHAR)mnag_Ch2_tmp;
			}
			else
			{
				rc = PRM_RSLT_RWACC_ERR;	/* Access restriction error */
			}
			break;

		case FCMD_DOWN:
			if(Exe_flg == OK)
			{
				/* PnE59:Monitor1,2Gain adjustment(Upper levelbyte) */
				mnag_Ch2_tmp = (LONG)(CHAR)((Cprm->mnag0_1.b.h) & 0xFF);

				if(mnag_Ch2_tmp > GADJAM_MIN)
				{
					mnag_Ch2_tmp -= 1;
				}

				/* PnE59:Monitor1,2Gain adjustment(Upper levelbyte)RAM */
				Cprm->mnag0_1.b.h = (UCHAR)mnag_Ch2_tmp;
			}
			else
			{
				rc = PRM_RSLT_RWACC_ERR;	/* Access restriction error */
			}
			break;

		case FCMD_CH1:
			FunExe->State = 0;	/* Analog monitor(CH1)Gain adjustment data Adjust */
			break;

		default:	/* other command */
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
			break;
		}
		break;

	default:
		break;
	}

	return rc ;

}

/****************************************************************************
 Fn00D: End function of Analog Monitor Gain Manual Adjustment
****************************************************************************/
void FnLeaveAnlgMonGainManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;
	RegCfg = Axis->RegManager->hPnReg;

	/* Termination processing		*/
	/* EEPROMUpdate process	*/
	PrmStoreValueToEeprom( RegCfg, &pndef_mnag0_1, 0 );		/* PnE59:Monitor1,2Zero adjustment */
	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;					/* normal mode */
}


/******************************* End of File *******************************/



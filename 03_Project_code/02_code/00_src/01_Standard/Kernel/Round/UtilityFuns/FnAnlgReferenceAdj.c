/****************************************************************************
  Description   : Analog command adjustment(Fn009, Fn00A, Fn00B)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2013 All Rights Reserved

  Project       : Mercury

  Create Date   : 2013-11-26

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
#define KPI_RC_NOTADJ	0		/* Adjustment execution condition not met */
#define KPI_RC_ADJOK	1		/* Adjustment completed */
#define KPI_RC_ADJERR	-1		/* Adjustment failed */

#define OFFSET_ADJ		0		/* Current detection offset adjustment */


/* Manual adjustment value of speed command offset Maximum value/Minimum value */
//#define	ZADJV_MAX			32767	/* <S0A9>	Delete */
//#define	ZADJV_MIN			-32768	/* <S0A9>	Delete */

/* <S0A9>	Start */
#if	( SVFSEL_VREFAD_24BIT != 0 )
#define	ZADJV_MAX			52500000
#define	ZADJV_MIN			-52500000
#define	ZADJV_RATE			3500			/* 52500000÷15000 = 3500	*/
#else
#define	ZADJV_MAX			240000
#define	ZADJV_MIN			-240000
#define	ZADJV_RATE			16				/* 240000÷15000 = 16		*/
#endif
/* <S0A9>	End */


/* Manual adjustment value of torque command offset Maximum value/Minimum value */
//#define	ZADJT_MAX		0xffff	/* <S0A9> Delete */
//#define	ZADJT_MIN		0x0000	/* <S0A9> Delete */
#define	ZADJT_MAX			127		/* <S0A9> Add */
#define	ZADJT_MIN			-127	/* <S0A9> Add */


/* User level */
//#define	ACCLVL_SYSTEM	3	/* User level = System level */	/* <S101> */
#define	ACCLVL_SYSTEM	4		/* User level = System level */	/* <S101> */


//#define	OK	1					/* Current detection signal offset manual adjustment Possible *///<S0C7>
//#define	NG	0					/* Current detection signal offset manual adjustment Not possible *///<S0C7>

#define	TREFZVADP		0x0C2	/* AD value when positive side torque command AD zero V is input				*/
#define	TREFZVADN		0x098	/* AD value when negative side torque command AD zero V input				*/


/*		AD control for commands			*/
#define _UDL_REG_ADDR_  (ULONG)0xC0000000
#define KPX_REFAD_LATCH( )			(((ASIC_HW_IF_STR *)_UDL_REG_ADDR_)->AREG_ADSET1) |= (1<<14)

/* Addition(64bit+32bit) */
#define	ADD6432( a, b, xx )		*(INT64*)xx = *(INT64*)(a) + (INT64)(b)

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/* <S0BC> Start */
/* The following4functions with the same function name.\Kernel\Driver\AdcIf.c", so change the function name. */
//static LONG AdjustRefAdZero( AXIS_HANDLE *Axis );
//static void TciAdjustRefAdZero( AXIS_HANDLE *Axis );
//static void TciReadTrefVrefADMean( LONG AdMean[3] );
//static void	MlibAdd6432( LONG *a, LONG b, LONG *x );

static LONG FnAdjustRefAdZero( AXIS_HANDLE *Axis );
static void FnTciAdjustRefAdZero( AXIS_HANDLE *Axis );
static void FnTciReadTrefVrefADMean( LONG AdMean[3] );
static void	FnMlibAdd6432( LONG *a, LONG b, LONG *x );
/* <S0BC> End */


/****************************************************************************
 Fn009: Entry function of Analog Reference Offset Auto Adjustment
****************************************************************************/
PRM_RSLT FnEnterAnlgRefOffsetAutoAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	FUN_CMN_CONTROL		*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;

	/* Ends without turning on when servo is on	*/
	if(FnCmnCtrl->State.FnSvCtrlDisable != FALSE)
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	FnCmnCtrl->KnlMode = KPI_KMD_ADJREFAD;	/* reference A/D adjusting  */

//#endif	/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
 Fn009: Execute function of Analog Reference Offset Auto Adjustment
****************************************************************************/
PRM_RSLT FnExecuteAnlgRefOffsetAutoAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	FUN_CMN_CONTROL	*FnCmnCtrl;
	SEQ_CTRL_OUT	*SeqCtrlOut;
	ASIC_HW_IF_STR	*AsicHwReg;

	LONG			AdjResult;
	FnCmnCtrl = Axis->FnCmnCtrl;
	SeqCtrlOut = Axis->SeqCtrlOut;
	AsicHwReg = Axis->SvAsicRegs->AsicHwReg;

	/* Wait until the execution request comes.	*/
	if(FunExe->CmdCode != FCMD_EXEC)
	{
		return PRM_RSLT_CONTINUE;
	}

	/* Ends without turning on when servo is on	*/
//	if( (SeqCtrlOut->MainPowerOn == FALSE)	/* <S0FC> */
//		|| (SeqCtrlOut->HwbbSts != FALSE)	/* <S0FC> */
//		|| (SeqCtrlOut->MotStop == FALSE)	/* <S0FC> */
//		|| (SHalDrv_GetFaultStatus(AsicHwReg, SHAL_FAULT_BB) == 0) )	/* Base Enable */	/* <S0FC> */
	if( SHalDrv_GetFaultStatus(AsicHwReg, SHAL_FAULT_BB) == 0 )			/* Base Enable */	/* <S0FC> */
	{
		return PRM_RSLT_CONDITION_ERR;
	}

	/* Adjustment execution */
//	AdjResult = AdjustRefAdZero(Axis);		/* <S0BC> Change the function name because a function with the same name exists */
	AdjResult = FnAdjustRefAdZero(Axis);	/* <S0BC> Change the function name because a function with the same name exists */

	/* Adjustment completed */
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;

	if(AdjResult == KPI_RC_NOTADJ){
		return PRM_RSLT_CONDITION_ERR;
	}
	else if(AdjResult == KPI_RC_ADJOK){
		/* Do nothing */
	}
	else if(AdjResult == KPI_RC_ADJERR){
		return PRM_RSLT_CALC_ERR;
	}

//#endif		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */
	return PRM_RSLT_SUCCESS;
}
#if 0	/* <S0E8>	Delete:Since this function has no processing, it will be deleted.  */
/****************************************************************************
 Fn009: End function of Analog Reference Offset Auto Adjustment
****************************************************************************/
void FnLeaveAnlgRefOffsetAutoAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */

	REGIF_CONFIG_T	*RegCfg;
	RegCfg = Axis->RegManager->hPnReg;

	/* Termination processing */
	/* EEPROMUpdate process */
	//PrmStoreValueToEeprom( RegCfg, &pndef_zadjvl, 0 );		/* PnE53 : Speed command zero adjustment */					/* <S0A9>	Delete */
	//PrmStoreValueToEeprom( RegCfg, &pndef_zadjtp_n, 0 );		/* PnE56 : Torque command zero adjustment(Positive input/Negative input) */	/* <S0A9>	Delete */

//#endif		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */
}
#endif	/* <S0E8>	Delete:Since this function has no processing, it will be deleted. */


/****************************************************************************
 Fn00A: Entry function of Velocity Reference Offset Manual Adjustment
****************************************************************************/
PRM_RSLT FnEnterAnlgVelocityRefManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */
	FUN_CMN_CONTROL		*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;


//	FnCmnCtrl->KnlMode = KPI_KMD_ADJREFAD;	/* reference A/D adjusting  */
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;	/* normal mode */	/* <S142> */

	FunExe->HoldFnMode = TRUE;				/* FnMode automatic end */
//#endif		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 Fn00A: Execute function of Velocity Reference Offset Manual Adjustment
****************************************************************************/
PRM_RSLT FnExecuteAnlgVelocityRefManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRMDATA			*Prm;
	PRM_RSLT		rc;
	LONG			zadjvl_tmp;
	CHAR			Exe_flg;
	FUN_AXCOMMON	*AxCommon;
	CADC	*pAdc;	/* <S0F9> */

	Prm = Axis->UniPrms->Prm;
	AxCommon = ((AXIS_HANDLE*)Axis)->FnManager->AxCommon;
	pAdc = Axis->pAdc;	/* <S0F9> */

	rc = PRM_RSLT_SUCCESS;
	Exe_flg = NG;

//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */
	/* Confirm user level */
#if 0
	if(AxCommon->AccessLevel < ACCLVL_SYSTEM )
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
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:	/* UP,Forward rotation,forward direction */
			if(Exe_flg == OK)
			{
				/* PnE53 : Read speed command zero adjustment value */
				//zadjvl_tmp = (LONG)(SHORT)((Prm->zadjvl) & 0xFFFF);/* <S0A9> DataMold change */
				zadjvl_tmp = Prm->zadjvl;							/* <S0A9> DataMold change */

//				zadjvl_tmp += 1;
				zadjvl_tmp += ZADJV_RATE;

				if( zadjvl_tmp > ZADJV_MAX )
				{
					zadjvl_tmp = ZADJV_MAX;
				}
				/*  PnE53 : Updated value to speed command zero adjustmentRAM */
				//Prm->zadjvl = (SHORT)zadjvl_tmp;
				Prm->zadjvl = zadjvl_tmp;	/* <S0A9> Automatic tester compatible */
				pAdc->P.ZadjVelo = zadjvl_tmp;							/* <S170> */

			}
			break;
		case FCMD_DOWN:	/* DOWN,Reverse running,Reverse direction */
			if(Exe_flg == OK)
			{
				//zadjvl_tmp = (LONG)(SHORT)((Prm->zadjvl) & 0xFFFF);/* <S0A9> Automatic tester compatible:DataMold change */
				zadjvl_tmp = Prm->zadjvl;	/* <S0A9> DataMold change */

//				zadjvl_tmp -= 1;
				zadjvl_tmp -= ZADJV_RATE;

				if( zadjvl_tmp < ZADJV_MIN )
				{
					zadjvl_tmp = ZADJV_MIN;
				}
				/*  PnE53 : Write updated value to speed command zero adjustment */
				//Prm->zadjvl = (SHORT)zadjvl_tmp;
				Prm->zadjvl = zadjvl_tmp;	/* <S0A9> */

				pAdc->P.ZadjVelo = Prm->zadjvl;	/*  PnE53 : Speed command zero adjustmentZero Adjust */	/* <S0F9> */
			}
			break;

		default:	/* other command */
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
			break;
		}
//#endif		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	return rc ;
}

/****************************************************************************
 Fn00A: End function of Velocity Reference Offset Manual Adjustment
****************************************************************************/
void FnLeaveAnlgVelocityRefManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;
	RegCfg = Axis->RegManager->hPnReg;

	/* EEPROMUpdate process	*/
	PrmStoreValueToEeprom( RegCfg, &pndef_zadjvl, 0 );		/* PnE53 : Speed command zero adjustment */
//	PrmStoreValueToEeprom( RegCfg, &pndef_zadjvh, 0 );		/* PnE54 : Speed command zero adjustment */	/* <S0A9> Automatic tester compatible */


	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;					/* normal mode */

//#endif	/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */
}

/****************************************************************************
 Fn00B: Entry function of Torque Reference Offset Manual Adjustment
****************************************************************************/
PRM_RSLT FnEnterAnlgTorqueRefManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	FUN_CMN_CONTROL		*FnCmnCtrl;

	FnCmnCtrl = Axis->FnCmnCtrl;


//	FnCmnCtrl->KnlMode = KPI_KMD_ADJREFAD;	/* reference A/D adjusting  */
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;		/* normal mode */	/* <S142> */

	FunExe->HoldFnMode = TRUE;				/* FnMode automatic end */

//#endif		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 Fn00B: Execute function of Torque Reference Offset Manual Adjustment
****************************************************************************/
PRM_RSLT FnExecuteAnlgTorqueRefManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
	PRMDATA			*Prm;
	PRM_RSLT		rc;
//	USHORT			zadjtp_n_tmp;	/* <S0A9> Delete */
//	SHORT			zadjtp_n_tmp;	/* <S0A9> Add */
	DBYTEX			zadjtp_n_tmp;
	CHAR			Exe_flg;
	FUN_AXCOMMON	*AxCommon;
	CADC			*pAdc = Axis->pAdc;	/* <S170> */

	Prm = Axis->UniPrms->Prm;
	AxCommon = ((AXIS_HANDLE*)Axis)->FnManager->AxCommon;

	rc = PRM_RSLT_SUCCESS;
	Exe_flg = NG;

//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */
	/* Confirm user level */
#if 0
	if(AxCommon->AccessLevel < ACCLVL_SYSTEM )
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
		switch(FunExe->CmdCode)
		{
		case FCMD_UP:	/* UP,Forward rotation,forward direction */
			if(Exe_flg == OK)
			{
				/* PnE56 : Torque command zero adjustment(Positive input/Negative input)Value reading */
				//zadjtp_n_tmp = Prm->zadjtp_n.w;					/* <S0A9> Delete */
//				zadjtp_n_tmp = (SHORT)((Prm->zadjtp_n.w)& 0xFFFF);	/* <S0A9> Add */
				zadjtp_n_tmp.w = (SHORT)((Prm->zadjtp_n.w)& 0xFFFF);
//				if( zadjtp_n_tmp < ZADJT_MAX )
				if( ((CHAR)zadjtp_n_tmp.b.l < ZADJT_MAX) && ((CHAR)zadjtp_n_tmp.b.h < ZADJT_MAX) )
				{
//					zadjtp_n_tmp += 1;
					(CHAR)zadjtp_n_tmp.b.l++;
					(CHAR)zadjtp_n_tmp.b.h++;
				}
				/*  PnE56 : Torque command zero adjustment(Positive input/Negative input) */
//				Prm->zadjtp_n.w = zadjtp_n_tmp;
				Prm->zadjtp_n.w = zadjtp_n_tmp.w;
				pAdc->P.ZadjTrqP = (CHAR)zadjtp_n_tmp.b.l;		/* <S170> */
				pAdc->P.ZadjTrqN = (CHAR)zadjtp_n_tmp.b.h;		/* <S170> */
			}
			break;

		case FCMD_DOWN:	/* DOWN,Reverse running,Reverse direction */
			if(Exe_flg == OK)
			{
				/* PnE56 : Torque command zero adjustment(Positive input/Negative input)Value reading */
				//zadjtp_n_tmp = Prm->zadjtp_n.w;					/* <S0A9> Delete */
//				zadjtp_n_tmp = (SHORT)((Prm->zadjtp_n.w)& 0xFFFF);	/* <S0A9> Add */
				zadjtp_n_tmp.w = (SHORT)((Prm->zadjtp_n.w)& 0xFFFF);
//				if(zadjtp_n_tmp > ZADJT_MIN)
				if( ((CHAR)zadjtp_n_tmp.b.l > ZADJT_MIN) && ((CHAR)zadjtp_n_tmp.b.h > ZADJT_MIN) )
				{
//					zadjtp_n_tmp -= 1;
					(CHAR)zadjtp_n_tmp.b.l--;
					(CHAR)zadjtp_n_tmp.b.h--;
				}
				/*  PnE56 : Torque command zero adjustment(Positive input/Negative input) */
				Prm->zadjtp_n.w = zadjtp_n_tmp.w;
				pAdc->P.ZadjTrqP = (CHAR)zadjtp_n_tmp.b.l;		/* <S170> */
				pAdc->P.ZadjTrqN = (CHAR)zadjtp_n_tmp.b.h;		/* <S170> */
			}
			break;

		default:	/* other command */
			rc = PRM_RSLT_CONDITION_ERR;	/* Condition error */
			break;
		}
//#endif		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	return rc ;

}

/****************************************************************************
 Fn00B: End function of Torque Reference Offset Manual Adjustment
****************************************************************************/
void FnLeaveAnlgTorqueRefManualAdj(FUNEXE *FunExe, AXIS_HANDLE *Axis)
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */		/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */

	FUN_CMN_CONTROL	*FnCmnCtrl;
	REGIF_CONFIG_T	*RegCfg;

	FnCmnCtrl = Axis->FnCmnCtrl;
	RegCfg = Axis->RegManager->hPnReg;

	/* EEPROMUpdate process	*/
	PrmStoreValueToEeprom( RegCfg, &pndef_zadjtp_n, 0 );		/* PnE56 : Torque command zero adjustment(Positive input/Negative input) */

	/* Termination processing	*/
	FnCmnCtrl->KnlMode = KPI_KMD_NORMAL;					/* normal mode */

//#endif	/* <S0A9> Delete:APDeleted as it is not a form-limited auxiliary function */
}


/****************************************************************************************************/
/*																									*/
/*		Command AD zero adjustment process(For online)																*/
/*																									*/
/****************************************************************************************************/
//static LONG	AdjustRefAdZero( AXIS_HANDLE *Axis )	/* <S0BC> Change the function name because a function with the same name exists */
static LONG	FnAdjustRefAdZero( AXIS_HANDLE *Axis )		/* <S0BC> Change the function name because a function with the same name exists */
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	/* <S0BC> Delete:APDeleted as it is not a form-limited auxiliary function */

	LONG	i;
	CADC *pAdc;

	pAdc = Axis->pAdc;

/*--------------------------------------------------------------------------------------------------*/
/*		Command AD zero adjustment process																			*/
/*--------------------------------------------------------------------------------------------------*/
		pAdc->f.AdjustError = FALSE;					/* Reset AdjustError Flag */
		pAdc->Mode = ADCMD_ADJREFAD;					/* set adjust mode */
//		TciAdjustRefAdZero( Axis  );					/* Adjust VrefAD and TrefAD */	/* <S0BC> Change the function name because a function with the same name exists */
		FnTciAdjustRefAdZero( Axis  );					/* Adjust VrefAD and TrefAD */	/* <S0BC> Change the function name because a function with the same name exists */
		pAdc->Mode = ADCMD_NORMAL;						/* reset adjust mode */	/* <S0E9> */

/*--------------------------------------------------------------------------------------------------*/
/*		Termination processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( pAdc->f.AdjustError == FALSE )
		{
			return( KPI_RC_ADJOK );					/* Adjust OK								*/
		}
		else
		{
			return( KPI_RC_ADJERR );					/* Adjust Error								*/
		}

//#endif	/* <S0BC> Delete:APDeleted as it is not a form-limited auxiliary function */
}

/****************************************************************************************************/
/*																									*/
/*		Command AD zero adjustment process(For offline/online use)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Adjustment data when input zero for each command AD(Offset amount when command input is zero)and			*/
/*			set each parameter.															*/
/*			If the offset amount is out of the following range, the offset amount will be treated as an adjustment error.				*/
/*			Make it zero.																			*/
/*																									*/
/*			1) T-ref P [Torque commandA/D Positive side] : -64 ≦ Offset ≦ 64	(64/4096=1.56%)						*/
/*																									*/
/*			2) T-ref N [Torque commandA/D Negative side] : -64 ≦ Offset ≦ 64	(64/4096=1.56%)						*/
/*																									*/
/*			3) V-ref   [Speed commandA/D]	 : -52500000 ≦ Offset ≦ 52500000	(24Bit AD In the case of)		*/
/*										 : -200000   ≦ Offset ≦ 200000		(16Bit AD In the case of)		*/
/*																									*/
/*											24Bit AD In the case of ==> (52500000/(2^23*128)=4.889%)		*/
/*											16Bit AD In the case of ==> (  200000/(2^15*128)=4.768%)		*/
/*																									*/
/*																									*/
/****************************************************************************************************/
//static void TciAdjustRefAdZero( AXIS_HANDLE *Axis )	/* <S0BC> Change the function name because a function with the same name exists */
static void FnTciAdjustRefAdZero( AXIS_HANDLE *Axis )	/* <S0BC> Change the function name because a function with the same name exists */
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */		/* <S0BC> Delete:APDeleted as it is not a form-limited auxiliary function */

	LONG	Zerox;
	LONG	Zeroy;
	LONG	AdMean[3];
	SHORT	XSat;
	SHORT	YSat;
	CADC	*pAdc;
	PRMDATA	*Prm;
	REGIF_CONFIG_T	*RegCfg;	/* <S0A9>  */

	pAdc = Axis->pAdc;
	Prm = Axis->UniPrms->Prm;
	XSat = FALSE;
	YSat = FALSE;
	RegCfg = Axis->RegManager->hPnReg;/* <S0A9>  */

/*--------------------------------------------------------------------------------------------------*/
/*		AD average value read																			*/
/*--------------------------------------------------------------------------------------------------*/
		KPX_SET_ADSEL_TREF( );					/* Select Tref AD Select Switch */
		KlibWaitms( 1 );						/* Reset Watchdog and Wait */
//		TciReadTrefVrefADMean( AdMean );		/* Read P-Tref, N-Tref, Vref AD Mean */	/* <S0BC> Change the function name because a function with the same name exists */
		FnTciReadTrefVrefADMean( AdMean );		/* Read P-Tref, N-Tref, Vref AD Mean */	/* <S0BC> Change the function name because a function with the same name exists */


/*--------------------------------------------------------------------------------------------------*/
/*		Torque command zero adjustment data calculation & storage																*/
/*--------------------------------------------------------------------------------------------------*/

		/* Positive side/AD value when inputting non-side torque command AD zero V Creation & confirmation */
		Zerox = -AdMean[0] + TREFZVADP;			/* AD value when positive side torque command AD zero V is input  */
		//if( (Zerox < -64) || (Zerox > 64) )		/* AD value when positive side torque command AD zero V is input Confirm(10bit->12bit chk!!) */
		if( (Zerox < -127) || (Zerox > 127) )		/* AD value when positive side torque command AD zero V is input Confirm(10bit->12bit chk!!) */ /* <S169> */
		{
			pAdc->f.AdjustError = TRUE;
		}

		Zeroy = +AdMean[1] - TREFZVADN;			/* AD value when negative side torque command AD zero V input  */
		//if( (Zeroy < -64) || (Zeroy > 64) )		/* AD value when negative side torque command AD zero V input Confirm(10bit->12bit chk!!) */
		if( (Zeroy < -127) || (Zeroy > 127) )		/* AD value when negative side torque command AD zero V input Confirm(10bit->12bit chk!!) */ /* <S169> */
		{
			pAdc->f.AdjustError = TRUE;
		}


		/* Positive side/AD value when inputting non-side torque command AD zero V Creation & confirmation */
		if( Zerox >= TREFZVADP )				/* If the calculated AD value when the positive side torque command AD zero V is input is more than the set value */
		{
			XSat = TRUE;
		}
		if( Zeroy <= -(TREFZVADN) )				/* If the AD value when the calculated non-side torque command AD zero V is input is less than the set value */
		{
			YSat = TRUE;
		}
	/*----------------------------------------------------------------------------------------------*/
		if ( pAdc->f.AdjustError )
		{
			Zerox = 0;
			Zeroy = 0;
		}
		else if(XSat == TRUE && YSat == FALSE)	/* AD value when positive side torque command AD zero V is input is more than the set value */
		{
			Zerox = Zeroy;
		}
		else if(XSat == FALSE && YSat == TRUE)	/* If the AD value when the calculated non-side torque command AD zero V is input is less than the set value */
		{
			Zeroy = Zerox;
		}
		else if(XSat == TRUE && YSat == TRUE)	/* AD value when positive side torque command AD zero V is input is more than the set value, AD value when the calculated negative side torque command AD zero V is input is less than the set value */
		{
			pAdc->f.AdjustError = TRUE;
			Zerox = 0;
			Zeroy = 0;
		}
		pAdc->P.ZadjTrqP = Zerox;								/* set Zero Adjust Variable */
		pAdc->P.ZadjTrqN = Zeroy;								/* set Zero Adjust Variable */
		Zerox = (Zerox & 0xFF) + (Zeroy << 8);					/* Combine P-Tref and N-Tref		*/
		//TODO:		Prm.zadjtp_n.w = (USHORT)Zerox;				/* Write Adjust Data to RAM			*/
		/*  PnE56 : Torque command zero adjustment(Positive input/Negative input)RAM */
		Prm->zadjtp_n.w = (USHORT)Zerox;
		PrmStoreValueToEeprom( RegCfg, &pndef_zadjtp_n, 0 );	/* Write Adjust Data to EepROM */		/* <S0A9>	TODOCancellation */



/*--------------------------------------------------------------------------------------------------*/
/*		Calculate and store speed command zero adjustment data																*/
/*--------------------------------------------------------------------------------------------------*/
#if	( SVFSEL_VREFAD_24BIT != 0 )
		Zerox = -AdMean[2];									/* Speed command zero adjustment value Create */
		if ( (Zerox < -52500000) || (Zerox > 52500000) )	/* Created speed command zero adjustment value Confirm */
		{
			Zerox = 0;
			KnlV.f.AdjustError = TRUE;
		}
#else
/*--------------------------------------------------------------------------------------------------*/
/*		-240000~+240000, make an error:	(15.36/0x7FFF)×(240000/128) = 879mV			*/
/*																									*/
/*		Offset adjustment range is not specified in the manual±750mVHowever,										*/
/*		Fn00A, the offset adjustment range is15000±240000,		*/
/*--------------------------------------------------------------------------------------------------*/
		Zerox = -AdMean[2];									/* Speed command zero adjustment value Create */
		if ( (Zerox < -240000) || (Zerox > 240000) )		/* Created speed command zero adjustment value Confirm */
		{
			Zerox = 0;
			pAdc->f.AdjustError = TRUE;
		}
#endif
	/*----------------------------------------------------------------------------------------------*/
		pAdc->P.ZadjVelo = Zerox;
//TODO:		Prm.zadjvl = (USHORT)Zerox;						/* Write Adjust Data to RAM					*/
//TODO:		Prm.zadjvh = (USHORT)(Zerox>>16);				/* Write Adjust Data to RAM					*/
		Prm->zadjvl = Zerox;								/* Write Adjust Data to RAM					*/	/* <S0A9>	TODOCancellation */
//		Prm->zadjvh = (USHORT)(Zerox>>16);					/* Write Adjust Data to RAM					*/	/* <S0A9>	TODOCancellation */
		PrmStoreValueToEeprom( RegCfg, &pndef_zadjvl, 0 );	/* Write Adjust Data to EepROM				*/	/* <S0A9> */

//#endif	/* <S0BC> Delete:APDeleted as it is not a form-limited auxiliary function */
		return;
}
/****************************************************************************************************/
/*																									*/
/*		Torque command AD & speed command AD average value reading(For offline/online use)							*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Read the torque command AD value and speed command AD value 4096 times and return the average value.				*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
//static void TciReadTrefVrefADMean( LONG AdMean[3] )	/* <S0BC> Change the function name because a function with the same name exists */
static void FnTciReadTrefVrefADMean( LONG AdMean[3] )	/* <S0BC> Change the function name because a function with the same name exists */
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	/* <S0BC> Delete:APDeleted as it is not a form-limited auxiliary function */

	LONG	i;
	LONG	Adlo;
	LONG	Adhi;
	LONG	TAdBuf[2];
	LONG	VAdBuf[2];


/*--------------------------------------------------------------------------------------------------*/
/*		AD read(4096times) : 30us * 4096 = 122.880 ms												*/
/*--------------------------------------------------------------------------------------------------*/
/*		<1>CPUADofRepeat Mode, so it was set toTrefAdThe startup process for was removed.								*/
/*--------------------------------------------------------------------------------------------------*/
		TAdBuf[0] = TAdBuf[1] = 0;						/* Clear Tref AD Buffer						*/
		VAdBuf[0] = VAdBuf[1] = 0;						/* Clear Vref AD Buffer						*/
		for( i=0; i < 0x1000; i++ )						/* Read Loop 4096 Times						*/
		{
			KlibWaitus( 30 );							/* Wait 30us for AD Conversion				*/
			TAdBuf[0] += KPX_READ_CPUAD0();				/* Read and Add Buf	(P-Tref)				*/
			TAdBuf[1] += KPX_READ_CPUAD1();				/* Read and Add Buf (N-Tref)				*/
			KlibWaitus( 18 );							/* Reset Watchdog and Wait					*/
		/*------------------------------------------------------------------------------------------*/
			KPX_REFAD_LATCH( );									/* Latch Vref AD Conv. Data			*/

#if	( SVFSEL_VREFAD_24BIT != 0 )	/* @SKLT@ */
			Adlo = (USHORT)(ASIC_RD( AREG_ACVLL ));				/* Read Low Word					*/
			Adhi = ((LONG)(ASIC_RD( AREG_ACVLH )<<24))>>8;		/* Read High Word					*/
			//MlibAdd6432( VAdBuf, ((Adhi+Adlo)<<7), VAdBuf );	/* Add  Buffer	(128*AdData)		*/	/* <S0BC> Change the function name because a function with the same name exists */
			FnMlibAdd6432( VAdBuf, ((Adhi+Adlo)<<7), VAdBuf );	/* Add  Buffer	(128*AdData)		*/	/* <S0BC> Change the function name because a function with the same name exists */
#else		/* 16bit AD */
			//Adlo = (SHORT)(((ASIC_HW_IF_STR *)_UDL_REG_ADDR_)->AREG_ACUL);	/* Interim */	/* <S0BC> */
			Adlo = (USHORT)(((ASIC_HW_IF_STR *)_UDL_REG_ADDR_)->AREG_ACUL);		/* Interim */	/* <S0BC> */
			Adhi = ((( Adlo << 16 ) >> 16 ) & 0xFFFF0000 );		/* <VZADJ>Sign extension					*/
#endif
			//MlibAdd6432( VAdBuf, ((Adhi+Adlo)<<7), VAdBuf );	/* <V031>Add  Buffer (128*AdData)	*/	/* <S0BC> Change the function name because a function with the same name exists */
			FnMlibAdd6432( VAdBuf, ((Adhi+Adlo)<<7), VAdBuf );	/* <V031>Add  Buffer (128*AdData)	*/	/* <S0BC> Change the function name because a function with the same name exists */
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Average value calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		AdMean[0] = (((TAdBuf[0]>>11)+1)>>1);							/* P-Tref AD Mean			*/
		AdMean[1] = (((TAdBuf[1]>>11)+1)>>1);							/* N-Tref AD Mean			*/
	 	AdMean[2] = (VAdBuf[1]<<20) | ((((ULONG)VAdBuf[0]>>11)+1)>>1);	/* Vref AD Mean				*/

//#endif	/* <S0BC> Delete:APDeleted as it is not a form-limited auxiliary function */

		return;

}

/****************************************************************************************************/
/*																									*/
/*		Addition(64bit+32bit)																			*/
/*																									*/
/*		TODO: Erase after																				*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
//void	MlibAdd6432( LONG *a, LONG b, LONG *x )			/* <S0BC> Change the function name because a function with the same name exists */
static void FnMlibAdd6432( LONG *a, LONG b, LONG *x )	/* <S0BC> Change the function name because a function with the same name exists */
{
//#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	/* <S0BC> Delete:APDeleted as it is not a form-limited auxiliary function */

		ADD6432( a, b, x );
//#endif														/* <S0BC> Delete:APDeleted as it is not a form-limited auxiliary function */
		return;

}

/******************************* End of File *******************************/

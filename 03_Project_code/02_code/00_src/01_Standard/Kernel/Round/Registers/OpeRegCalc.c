/****************************************************************************
  Description   : Callback functions for Operation Register
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corp., Copyright (c) 2010
                  All Rights Reserved

  Project       : INGRAM

  Create Date   : 2010-10-20

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "PrmAccessIf.h"
#include "FunManager.h"
#include "BaseControlPrmCalc.h"
#include "FnMfcTuning.h"
#include "FnSetTuneLessLvl.h"
#include "FnOnePrmTuning.h"
#include "FnAtypeVibTune.h"
#include "DataTraceManager.h"
#include "PdetABSScale.h"
#include "TableDrive.h"
#include "PosManager.h"
#include "PcmdFilter.h" 		/* <S065> */
#include "PnPrmListTbl.h"
#include "PrmAccessIf.h"		/* <S0C7> */
#include "LcdOp.h"				/* <S0C9> */
#include "KnlApi.h"			/* <S0C9> */
#include "Adc.h"				/* <S0CB> */
#include "AnalogMonitorCalc.h"	/* <S0E7> */
#include "RippleComp.h"		/* <S1FD> */
/* Extern Serial Message I/F for SigmaMsg/LcdOp/Memobus */	/* <S0C9> */
extern MSGIF	SrlMsg[SIF_N_CH];							/* <S0C9> */

/****************************************************************************
 2000H:Operation mode
****************************************************************************/
PRM_RSLT fnCalcOprationMode1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT ErrRes;
	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = FunGetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager,FUN_CH1);
		ErrRes = PRM_RSLT_SUCCESS;
	}
	else
	{
		ErrRes = FunSetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager,
									FUN_CH1, (ULONG)*pValue, Axis);
	}

	return ErrRes;
}

/****************************************************************************
 2001H:Operation setting
****************************************************************************/
PRM_RSLT fnCalcOprationCmd1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT ErrRes;
	ErrRes = PRM_RSLT_SUCCESS;

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (USHORT)(((AXIS_HANDLE*)Axis)->FnManager)->FunExeState[0].ErrorResult;
	}
	else
	{
		ErrRes = FunSetOpeCommandRegister(((AXIS_HANDLE*)Axis)->FnManager,
										FUN_CH1, (ULONG)*pValue);
	}

	return ErrRes;
}

/****************************************************************************
 2002H:End operation
****************************************************************************/
PRM_RSLT fnCalcOperationFinish(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT ErrRes;
	if(Cmd == PRM_ACCCMD_READ)
	{
		ErrRes = PRM_RSLT_REGNO_ERR;
	}
	else
	{
		FunSetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager, FUN_CH1, 0, Axis);
		FunSetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager, FUN_CH2, 0, Axis);
		DtrcWriteOpeModeTrcRegister( ((AXIS_HANDLE*)Axis)->TrcHndl, 0 );
		ErrRes = PRM_RSLT_SUCCESS;
	}
	return ErrRes;
}

/****************************************************************************
 0x2020	User constant writing prohibited
****************************************************************************/
PRM_RSLT fnCalcPrmAccessLock(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT 	ErrRes;
	REG_ACC_T	AccessPrm;
	LONG		syssw3;

	ErrRes = PRM_RSLT_SUCCESS;

	AccessPrm.hRegCfg = Axis->RegManager->hPnReg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.IfHandle = NULL;
	AccessPrm.EepWrite = TRUE;
	AccessPrm.cPrmDef = &pndef_syssw3;

	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
		if(*pValue == 0)
		{
			/* Write the complete flag to PnE0D.0=0 */
			PrmReadValue(&AccessPrm, &syssw3, 2);
			syssw3 = syssw3 & 0xFFFE;
			PrmWriteValue(&AccessPrm, &syssw3, 2);
			RegMngrFreeWriteParameters(Axis->RegManager);
		}
		else if(*pValue == 1)
		{
			/* Write the complete flag to PnE0D.0=1 */
			PrmReadValue(&AccessPrm, &syssw3, 2);
			syssw3 = syssw3 | 0x0001;
			PrmWriteValue(&AccessPrm, &syssw3, 2);
			RegMngrLockWriteParameters(Axis->RegManager);
		}
		else
		{
			ErrRes = PRM_RSLT_LIMIT_ERR;
		}
		break;

	case PRM_ACCCMD_RECALC:
		PrmReadValue(&AccessPrm, &syssw3, 2);
		if(0 != (syssw3 & 1))
		{/* Lock to write parameter access */
			RegMngrLockWriteParameters(Axis->RegManager);
		}
		else
		{/* Unlock to write parameter access */
			RegMngrFreeWriteParameters(Axis->RegManager);
		}
		break;

	case PRM_ACCCMD_READ:
		PrmReadValue(&AccessPrm, &syssw3, 2);
		*pValue = syssw3 & 1;
		break;

	default:
		break;
	}

	return ErrRes;
}

/****************************************************************************
 2003H:Access level
****************************************************************************/
PRM_RSLT fnCalcAccessLevel(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG			level; /* <S21F> */
	USHORT			passwd;

/* <S056>	Start */
	FUN_AXCOMMON	*AxCommon;
	PRMDATA			*Prm;
	REGIF_CONFIG_T	*RegCfg;

	AxCommon = ((AXIS_HANDLE*)Axis)->FnManager->AxCommon;
	Prm = Axis->UniPrms->Prm;
	RegCfg = Axis->RegManager->hPnReg;


//	if(Cmd == PRM_ACCCMD_READ)
//	{
//		*pValue = (LONG)(AxCommon->AccessLevel - 1);
//	}
//	else
//	{
//		passwd = (USHORT)*pValue;
//		if(passwd == 0x1002)
//		{	AxCommon->AccessLevel = 1/*ACCLVL_USER1*/;}
//		else if(passwd == 0x2001)
//		{	AxCommon->AccessLevel = 2/*ACCLVL_USER2*/;}
//		else if(passwd == 0x2236)
//		{	AxCommon->AccessLevel = 3/*ACCLVL_SYSTEM*/;}
//		else
//		{	;}


	if(Cmd == PRM_ACCCMD_READ)
	{
//		*pValue = (LONG)(AxCommon->AccessLevel - 1);	/* <S21F> */
		level = (LONG)(AxCommon->AccessLevel - 2);		/* <S21F> */
		*pValue = (level >= 0) ? level : 0;				/* <S21F> */
	}
	else
	{
		passwd = (USHORT)*pValue;

		switch(passwd)
		{
		case WRITE_OK_SET_VAL:	/* Writing permission(No change in access level) */
				fnCalcPrmAccessLock(Cmd, ArrayIdx, Axis, pValue);
			break;

		case WRITE_NG_SET_VAL:	/* Writing prohibited(No change in access level) */
				fnCalcPrmAccessLock(Cmd, ArrayIdx, Axis, pValue);
			break;

		case ACCLVL_USER1_SET_VAL:	/* User level1Setting */
				AxCommon->AccessLevel = ACCLVL_USER1;

				/* EEPROM(  PnE0D.1 = 0 : User level1 ) */
				Prm->syssw3 = Prm->syssw3 & (~0x0002);
				PrmStoreValueToEeprom( RegCfg, &pndef_syssw3, 0 );
			break;

		case ACCLVL_USER2_SET_VAL:	/* User level2Setting */
				AxCommon->AccessLevel = ACCLVL_USER2;

				//Prm->syssw3 = Prm->syssw3 | 0x0002;					/* <S1B0> */
				//PrmStoreValueToEeprom( RegCfg, &pndef_syssw3, 0 );	/* <S1B0> */
			break;

		case ACCLVL_USER3_SET_VAL:	/* User level3Setting */
				AxCommon->AccessLevel = ACCLVL_USER3;

				/* EEPROM(  PnE0D.1 = 1 : User level3 ) */	/* <S1B0> */
				Prm->syssw3 = Prm->syssw3 | 0x0002;									/* <S1B0> */
				PrmStoreValueToEeprom( RegCfg, &pndef_syssw3, 0 );					/* <S1B0> */
			break;

		case ACCLVL_SYSTEM_SET_VAL:	/* System level setting */
				AxCommon->AccessLevel = ACCLVL_SYSTEM;
			break;

		default:
			/* Do Nothing */
			break;
		}
		
	}
/* <S056>	End */

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 2004H:No.2Operation mode
****************************************************************************/
PRM_RSLT fnCalcOprationMode2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT ErrRes;
	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)FunGetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager,FUN_CH2);
		ErrRes = PRM_RSLT_SUCCESS;
	}
	else
	{
		ErrRes = FunSetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager,
									FUN_CH2, (ULONG)*pValue, Axis);
	}

	return ErrRes;
}

/****************************************************************************
 2005H:No.2Operation setting
****************************************************************************/
PRM_RSLT fnCalcOprationCmd2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT ErrRes;
	ErrRes = PRM_RSLT_SUCCESS;

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (USHORT)(((AXIS_HANDLE*)Axis)->FnManager)->FunExeState[1].ErrorResult;
	}
	else
	{
		ErrRes = FunSetOpeCommandRegister(((AXIS_HANDLE*)Axis)->FnManager,
										FUN_CH2, (ULONG)*pValue);
	}
	return ErrRes;
}


/****************************************************************************
 2032H:Magnetic pole detection complete
****************************************************************************/
/*	0x00:MPDET_NORMAL		Normal										  */
/*	0x01:MPDET_EXE			Executing										  */
/*	0x02:MPDET_COMP			Complete										  */
/*	0x03:MPDET_ERROR		Abnormal termination									  */
/**************************************************************************/
PRM_RSLT fnGetPdetStatus(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	switch(((AXIS_HANDLE*)Axis)->PdetAbsSeq->SeqStep)
	{
	case PDETSEQ_IDOL:
	case PDETSEQ_PHASE_WR_READY:
		if(Axis->MencV->PhaseReady == FALSE)
		{
			*pValue = Axis->FnCmnCtrl->PdetReq | Axis->BeSequence->BeReqSeqOut;
		}
		else
		{/* already completed. */
			*pValue = 2;
		}
		break;
	case PDETSEQ_FINISHED:
		*pValue = 2;
		break;
	case PDETSEQ_ERROR:
		*pValue = 3;
		break;
	default:
		*pValue = 1;
		break;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 2033H:Origin position setting complete
****************************************************************************/
PRM_RSLT fnGetScaleZsetStatus(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRMDATA		*Prm;

	Prm = ((AXIS_HANDLE*)Axis)->UniPrms->Prm;
	*pValue = (0 == (Prm->syssw3 & 0x0010)) ? 0 : 1;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 2050H:1Paraltuning mode
****************************************************************************/
PRM_RSLT	FnOnePrmTuneSpatMode( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT		rc;
	ONEPRMTUNE		*OnePrmTune;
	BASE_CTRL_HNDL	*pBaseCtrl;
	PRMDATA			*Prm;

	OnePrmTune = ((AXIS_HANDLE*)Axis)->OnePrmTune;
	pBaseCtrl = ((AXIS_HANDLE*)Axis)->BaseControls;
	Prm = (((AXIS_HANDLE*)Axis)->UniPrms)->Prm;

	rc = RpiFunSetOnePrmTuningMode( OnePrmTune, (USHORT)*pValue );
	if( (rc != PRM_RSLT_SUCCESS) && (rc != PRM_RSLT_CONTINUE) )
	{
		return( rc );
	}
	/* Tuning level initial setting */
	OnePrmReSetTuningLevel( OnePrmTune,
							Prm->loophz,
							Prm->posgn,
							pBaseCtrl->CtrlCmdPrm.IPSpdControl );
	return( PRM_RSLT_SUCCESS );
}

/****************************************************************************
 2051H:1Paraltuning level
****************************************************************************/
PRM_RSLT	FnOnePrmTuneSpatLevel( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	rc;
	ONEPRMTUNE	*OnePrmTune;
	OnePrmTune = ((AXIS_HANDLE*)Axis)->OnePrmTune;

	rc = RpiFunSetOnePrmTuningLevel( OnePrmTune, (USHORT)*pValue );
	if( (rc != PRM_RSLT_SUCCESS) && (rc != PRM_RSLT_CONTINUE) )
	{
		return( rc );
	}
	return( PRM_RSLT_SUCCESS );
}

/****************************************************************************
 2052H:1ParaltuningFFLevel
****************************************************************************/
PRM_RSLT	FnOnePrmTuneSpatFFLevel( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	rc;
	ONEPRMTUNE	*OnePrmTune;
	OnePrmTune = ((AXIS_HANDLE*)Axis)->OnePrmTune;

	/* <S0DA> */
	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
		rc = RpiFunSetOnePrmTuningFfLevel( OnePrmTune, (USHORT)*pValue );
		break;
	case PRM_ACCCMD_READ:
		*pValue = (LONG)OnePrmTune->OpeReg.SpatFFLevel;
		rc = PRM_RSLT_SUCCESS;
		break;
	default:
		rc = PRM_RSLT_SUCCESS;
		break;
	}

	return rc;

#if 0	/* <S0DA> */
	rc = RpiFunSetOnePrmTuningFfLevel( OnePrmTune, (USHORT)*pValue );
	if( (rc != PRM_RSLT_SUCCESS) && (rc != PRM_RSLT_CONTINUE) )
	{
		return( rc );
	}
	return( PRM_RSLT_SUCCESS );
#endif

}

/****************************************************************************
 2062H:EasyFFTTorque amplitude
****************************************************************************/
PRM_RSLT	FnEasyFftTrqAmplitude( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	rc;
	EASYFFT		*EasyFFT;
	EasyFFT = ((AXIS_HANDLE*)Axis)->EasyFFT;

	rc = RpiFunSetEasyFftTrqLevel( EasyFFT, (USHORT)*pValue );
	if( (rc != PRM_RSLT_SUCCESS) && (rc != PRM_RSLT_CONTINUE) )
	{ /* When data is abnormal */
		return( rc );
	}
	return( PRM_RSLT_SUCCESS );
}


/****************************************************************************
 0x2080	Motor constant read/write start address
****************************************************************************/
PRM_RSLT fnCalcMotorParameterAddress(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	MENCV *MencV;

	MencV = ((AXIS_HANDLE*)Axis)->MencV;
	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
	case PRM_ACCCMD_RECALC:
		MencV->MotPrmAddress = (USHORT)*pValue;
		break;
	case PRM_ACCCMD_READ:
	default:
		*pValue = (LONG)MencV->MotPrmAddress;
		break;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 0x2081	Motor constant read/write size
****************************************************************************/
PRM_RSLT fnCalcMotorParameterSize(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	MENCV *MencV;

	MencV = ((AXIS_HANDLE*)Axis)->MencV;
	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
	case PRM_ACCCMD_RECALC:
		MencV->MotPrmSize = (USHORT)*pValue;
		break;
	case PRM_ACCCMD_READ:
	default:
		*pValue = (LONG)MencV->MotPrmSize;
		break;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 0x2082	Motor constant read/write completion
****************************************************************************/
PRM_RSLT fnGetMotorDownloadState(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	mode;

	mode = FunGetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager,FUN_CH1);
	if((mode == 0x200B) || (mode == 0x200C))
	{
		*pValue = (LONG)(((AXIS_HANDLE*)Axis)->FnManager)->FunExeState[0].State;
	}
	else
	{
		*pValue = 0;
	}

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 0x208C Parameter Download/Upload Status	<S00C>
****************************************************************************/
PRM_RSLT fnGetPrmDownloadState(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	mode;

	mode = FunGetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager,FUN_CH1);
	if((mode == 0x200F) || (mode == 0x2010))
	{
		*pValue = (LONG)(((AXIS_HANDLE*)Axis)->FnManager)->FunExeState[0].State;
	}
	else
	{
		*pValue = 0;
	}

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
 2095H:Advanced AT filter type
****************************************************************************/
PRM_RSLT	FnAdatFilType( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	ONEPRMTUNE	*OnePrmTune;
	OnePrmTune = ((AXIS_HANDLE*)Axis)->OnePrmTune;

	if( (*pValue >= 1) && (*pValue <= 3) )
	{
		OnePrmTune->OpeReg.OnePrmFilType = (USHORT)*pValue;
	}
	else
	{
		return( PRM_RSLT_LIMIT_ERR );
	}
	return( PRM_RSLT_SUCCESS );
}

/****************************************************************************
 20A0H:AMold vibration damping function adjustment mode setting
****************************************************************************/
PRM_RSLT	FnAvibTuneAvibMode( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	rc;
	AVIBTUNE	*AvibTune;
	AvibTune = ((AXIS_HANDLE*)Axis)->AvibTune;

	rc = RpiFunSetAtypeVibTuningMode( AvibTune, (USHORT)*pValue );
	if( (rc != PRM_RSLT_SUCCESS) && (rc != PRM_RSLT_CONTINUE) )
	{
		return( rc );
	}
	return( PRM_RSLT_SUCCESS );

}

/****************************************************************************
 20A2H:AMold vibration damping function adjustment frequency
****************************************************************************/
PRM_RSLT	FnAvibTuneAvibFreq( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	rc;
	AVIBTUNE	*AvibTune;
	AvibTune = ((AXIS_HANDLE*)Axis)->AvibTune;

	rc = RpiFunSetAtypeVibTuningFrequency( AvibTune, (USHORT)*pValue );
	if( (rc != PRM_RSLT_SUCCESS) && (rc != PRM_RSLT_CONTINUE) )
	{
		return( rc );
	}
	return( PRM_RSLT_SUCCESS );

}

/****************************************************************************
 20A3H:AMold vibration damping function adjustment gain
****************************************************************************/
PRM_RSLT	FnAvibTuneAvibGain( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	rc;
	AVIBTUNE	*AvibTune;
	AvibTune = ((AXIS_HANDLE*)Axis)->AvibTune;

	rc = RpiFunSetAtypeVibTuningGain( AvibTune, (USHORT)*pValue );
	if( (rc != PRM_RSLT_SUCCESS) && (rc != PRM_RSLT_CONTINUE) )
	{
		return( rc );
	}
	return( PRM_RSLT_SUCCESS );

}

/****************************************************************************
 20B0H:With vibration suppressionMFCAdjustment frequency
****************************************************************************/
PRM_RSLT FnMfcTuneMdlVibFreq( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT ErrRes;

	ErrRes = RpiMfcTuningSetFreq( (((AXIS_HANDLE*)Axis)->MfcTune), (USHORT)*pValue );
	return( ErrRes );
}

/****************************************************************************
 20B2H : Vibration suppression function tuning frequency2
****************************************************************************/
PRM_RSLT FnMfcTuneMdlVibFreq2( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	ErrRes;
	MFCTUNE		*MfcTune;
	VIBSUPFIL	*VibSupFil;
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;

	MfcTune = ((AXIS_HANDLE*)Axis)->MfcTune;
	VibSupFil = &(((AXIS_HANDLE*)Axis)->BaseControls)->VibSupFil;
	Prm = (((AXIS_HANDLE*)Axis)->UniPrms)->Prm;
	Bprm = (((AXIS_HANDLE*)Axis)->UniPrms)->Bprm;

	ErrRes = RpiMfcTuningSetFreq2( MfcTune, (USHORT)*pValue );
	if( ErrRes == PRM_RSLT_SUCCESS )
	{
		Prm->ff_frq = MfcTune->OpeReg.MdlVibFreq2;
//		PosMngCalculatePrmVibSupFil( VibSupFil, /* <S065> */
		PcmdFilCalculatePrmVibSupFil( VibSupFil,
									  Prm->ff_frq,
									  Prm->ff_fil,
									  Bprm->SvCycleNs );
	}
	return( ErrRes );
}

/****************************************************************************
 20B3H:Vibration suppression function tuning frequency2Correction
****************************************************************************/
PRM_RSLT FnMfcTuneMdlVibFil2( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	ErrRes;
	MFCTUNE		*MfcTune;
	VIBSUPFIL	*VibSupFil;
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;

	MfcTune = ((AXIS_HANDLE*)Axis)->MfcTune;
	VibSupFil = &(((AXIS_HANDLE*)Axis)->BaseControls)->VibSupFil;
	Prm = (((AXIS_HANDLE*)Axis)->UniPrms)->Prm;
	Bprm = (((AXIS_HANDLE*)Axis)->UniPrms)->Bprm;

	ErrRes = RpiMfcTuningSetFreq2Comp( MfcTune, (USHORT)*pValue );
	if( ErrRes == PRM_RSLT_SUCCESS )
	{
		Prm->ff_fil = MfcTune->OpeReg.MdlVibFil2;
//		PosMngCalculatePrmVibSupFil( VibSupFil, /* <S065> */
		PcmdFilCalculatePrmVibSupFil( VibSupFil,
									  Prm->ff_frq,
									  Prm->ff_fil,
									  Bprm->SvCycleNs );
	}
	return( ErrRes );
}

/* <S14B> Start */
/****************************************************************************
 20B5H:Speed ripple compensation information writing confirmation
****************************************************************************/
PRM_RSLT fnCalcRippleCmpInfo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	RPLCMP				*RippleCmp;
	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	*pValue = (LONG)(RippleCmp->RippleCmpState);

	return (PRM_RSLT_SUCCESS);
}
/****************************************************************************
 20B6H:Speed ripple compensation gain
****************************************************************************/
PRM_RSLT fnCalcRippleCompGain(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);


	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompGainEle);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompGainEle = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20B7H:Speed ripple compensation number1Component period
****************************************************************************/
PRM_RSLT fnCalcRippleCompCycle1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompCycleEle[0]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompCycleEle[0] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);
}

/****************************************************************************
 20B8H:Speed ripple compensation number1Component amplitude value
****************************************************************************/
PRM_RSLT fnCalcRippleCompAmplitude1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompAmplitudeEle[0]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompAmplitudeEle[0] = (SHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20B9H:Speed ripple compensation number1Component phase
****************************************************************************/
PRM_RSLT fnCalcRippleCompPhase1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompPhaseEle[0]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompPhaseEle[0] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20BAH:Speed ripple compensation number2Component period
****************************************************************************/
PRM_RSLT fnCalcRippleCompCycle2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompCycleEle[1]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompCycleEle[1] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20BBH:Speed ripple compensation number2Component amplitude value
****************************************************************************/
PRM_RSLT fnCalcRippleCompAmplitude2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompAmplitudeEle[1]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompAmplitudeEle[1] = (SHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20BCH:Speed ripple compensation number2Component phase
****************************************************************************/
PRM_RSLT fnCalcRippleCompPhase2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompPhaseEle[1]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompPhaseEle[1] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20BDH:Speed ripple compensation number3Component period
****************************************************************************/
PRM_RSLT fnCalcRippleCompCycle3(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompCycleEle[2]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompCycleEle[2] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20BEH:Speed ripple compensation number3Component amplitude value
****************************************************************************/
PRM_RSLT fnCalcRippleCompAmplitude3(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompAmplitudeEle[2]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompAmplitudeEle[2] = (SHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20BFH:Speed ripple compensation number3Component phase
****************************************************************************/
PRM_RSLT fnCalcRippleCompPhase3(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompPhaseEle[2]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompPhaseEle[2] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S191> */
}

/****************************************************************************
 20C0H:Speed ripple compensation number4Component period
****************************************************************************/
PRM_RSLT fnCalcRippleCompCycle4(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompCycleEle[3]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompCycleEle[3] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}

/****************************************************************************
 20C1H:Speed ripple compensation number4Component amplitude value
****************************************************************************/
PRM_RSLT fnCalcRippleCompAmplitude4(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT 	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompAmplitudeEle[3]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompAmplitudeEle[3] = (SHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}

/****************************************************************************
 20C2H:Speed ripple compensation number4Component phase
****************************************************************************/
PRM_RSLT fnCalcRippleCompPhase4(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompPhaseEle[3]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompPhaseEle[3] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}

/****************************************************************************
 20C3H:Speed ripple compensation number5Component period
****************************************************************************/
PRM_RSLT fnCalcRippleCompCycle5(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompCycleEle[4]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompCycleEle[4] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}

/****************************************************************************
 20C4H:Speed ripple compensation number5Component amplitude value
****************************************************************************/
PRM_RSLT fnCalcRippleCompAmplitude5(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompAmplitudeEle[4]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompAmplitudeEle[4] = (SHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}

/****************************************************************************
 20C5H:Speed ripple compensation number5Component phase
****************************************************************************/
PRM_RSLT fnCalcRippleCompPhase5(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompPhaseEle[4]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompPhaseEle[4] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}

/****************************************************************************
 20C6H:Speed ripple compensation number6Component period
****************************************************************************/
PRM_RSLT fnCalcRippleCompCycle6(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompCycleEle[5]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompCycleEle[5] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}
/****************************************************************************
 20C7H:Speed ripple compensation number6Component amplitude value
****************************************************************************/
PRM_RSLT fnCalcRippleCompAmplitude6(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompAmplitudeEle[5]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompAmplitudeEle[5] = (SHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

	//	return ErrRes;
		return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}

/****************************************************************************
 20C8H:Speed ripple compensation number6Component phase
****************************************************************************/
PRM_RSLT fnCalcRippleCompPhase6(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	PRM_RSLT	ErrRes;
	RPLCMP		*RippleCmp;

	RippleCmp = &(Axis->BaseLoops->RippleCmp);

	if(Cmd == PRM_ACCCMD_READ)
	{
		*pValue = (LONG)(RippleCmp->RplCmpPrm.RplCompPhaseEle[5]);
	}
	else
	{
		RippleCmp->RplCmpPrm.RplCompPhaseEle[5] = (USHORT)*pValue;

		/* Ripple compensation parameter calculation */
//		FnRegCalVelRippleCompE( Axis );
		FnRegCalVelRippleCompE( Axis->UniPrms->Bprm, &Axis->BaseLoops->RippleCmp );/* <S1FD> */
	}

//	return ErrRes;
	return (PRM_RSLT_SUCCESS);		/* Uninitialized variable correction<S1FD> */
}

/* <S14B> End */

/****************************************************************************
 0x20E0:Dynamic autotuning gain level setting
****************************************************************************/
PRM_RSLT	FnSetTLLvlGainLevel( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT		ErrRes;
	TUNELESS_CTRL	*TuneLessCtrl;

	TuneLessCtrl = &(((AXIS_HANDLE*)Axis)->BaseControls->TuneLessCtrl);
	ErrRes = RpiFunSetDATGainLevel( TuneLessCtrl, (USHORT)*pValue );
	return( ErrRes );
}

/****************************************************************************
 0x20E1:Dynamic auto tuning load level setting
****************************************************************************/
PRM_RSLT	FnSetTLLvlJratLevel( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	PRM_RSLT	ErrRes;
	TUNELESS_CTRL	*TuneLessCtrl;

	TuneLessCtrl = &(((AXIS_HANDLE*)Axis)->BaseControls->TuneLessCtrl);
	ErrRes = RpiFunSetDATJratLevel( TuneLessCtrl, (USHORT)*pValue );
	return( ErrRes );
}

/****************************************************************************
 2108H:Trace operation mode
****************************************************************************/
PRM_RSLT fnCalcTraceMode(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT ErrRes;

	/* Sets trace mode. */

	/* Sets trace mode to Fun Manager. */
	ErrRes = FunSetTraceModeRegister(((AXIS_HANDLE*)Axis)->FnManager,
									 (ULONG)*pValue);
	if(ErrRes == PRM_RSLT_SUCCESS)
	{/* Success */

		/* Enter the Trace mode */
		ErrRes = DtrcWriteOpeModeTrcRegister( ((AXIS_HANDLE*)Axis)->TrcHndl,
																(USHORT)*pValue );
	}
	else
	{/* Error */
			ErrRes = PRM_RSLT_CALC_ERR;
	}

	return ErrRes;
}

/****************************************************************************
 2109H:Trace operation settings
****************************************************************************/
PRM_RSLT fnCalcTraceCmd(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT ErrRes;

	/* Start the Trace mode */
	ErrRes = DtrcWriteOpeCmdTrcRegister( ((AXIS_HANDLE*)Axis)->TrcHndl,
								((AXIS_HANDLE*)Axis)->TraceBuffer,(USHORT)*pValue );
	return ErrRes;
}


/****************************************************************************
 2100H:Trace mode
 2101H:Trace data length
 2102H:Trace buffer length
 2103H:Trace sampling interval
 2104H:Trace sampling time[us]
 2105H:Trace sampling interval lower limit
 2106H:Trace sampling interval upper limit
 2107H:Maximum race buffer length
 2110H:Data trace target selection
 2120H:I/OData trace target selection
 2130H:Trace real address specification
 2200H:Number of pre-trigger data
 2201H:Trigger condition
 2202H:Trigger level
 2204H:Trigger position
 2205H:Trace completion
 2206H:Continuous trace write pointer
 2208H:Multitrigger mode
 2209H:Trigger condition2
 220AH:Trigger level2
 ****************************************************************************/
PRM_RSLT fnCalcTraceRegisters(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG		ax;
	PRM_RSLT	ErrRes;
	AXIS_HANDLE	*AxisRsc;

	/* Sets the Trace Parameters */
	ErrRes = DtrcCalcParameter( ((AXIS_HANDLE*)Axis)->TrcHndl,
											((AXIS_HANDLE*)Axis)->TraceBuffer );

	/* Sets Trace and Table Drive Common Data */
	for( ax = 0; ax < NumberOfAxes; ax++ )
	{
		AxisRsc = GetAxisHandle(ax);
		TblDriveGetTraceRegData( &(AxisRsc->BaseControls->TblDrive),
								 ((AXIS_HANDLE*)Axis)->TrcHndl->TrcReg.DataSize,
								 ((AXIS_HANDLE*)Axis)->TrcHndl->TrcReg.SampIntv,
								 ((AXIS_HANDLE*)Axis)->TrcHndl->TrcExeP.DataSampIntv,
								 ((AXIS_HANDLE*)Axis)->TrcHndl->TrcExeP.TrcDnum );
	}
	return ErrRes;
}

/* <S0C9>	Start */
/****************************************************************************
 2110H:Data,I/OData trace1~16Target selection
 ****************************************************************************/
PRM_RSLT fnCalcDataIoTraceSel(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
	LONG		ax;
	PRM_RSLT	ErrRes;
	AXIS_HANDLE	*AxisRsc;
	MSGIF	*Hmsg_Com0;
	MSGIF	*Hmsg_Com1;
	TRCHNDL	*TrcHndl;
	ULONG	i;
	TRCREG	*TrcReg;
	TRCINDIVPRM	*TrcIncivPrm;

	/* Get Serial Message I/F for SigmaMsg/LcdOp/Memobus	*/
	Hmsg_Com0 = &SrlMsg[COM0_UART];		/* Serial CH:0 */
	Hmsg_Com1 = &SrlMsg[COM1_UART];		/* Serial CH:1 */


	TrcHndl = ((AXIS_HANDLE*)Axis)->TrcHndl;
	TrcReg  = &TrcHndl->TrcReg;
	TrcIncivPrm = &TrcHndl->TrcIncivPrm;

	/* Set Axis Address & Trace Data */
	for( i=0; i < TRC_CH_MAXNUM; i++ )
	{
		if( (Hmsg_Com0->CmdBuf[1] == 0x40) || (Hmsg_Com1->CmdBuf[1] == 0x40) )	/* 40HAccess with command */
		{

			if(Hmsg_Com0->CmdBuf[1] == 0x40)		/* Serial CH:0 */
			{
				/* Get axis address */
				TrcIncivPrm->TrcAxis[i] = Hmsg_Com0->CmdBuf[5];
				TrcIncivPrm->IOTrcAxis[i] = Hmsg_Com0->CmdBuf[5];
			}
			else								/* Serial CH:1 */
			{
				/* Get axis address */
				TrcIncivPrm->TrcAxis[i] = Hmsg_Com1->CmdBuf[5];
				TrcIncivPrm->IOTrcAxis[i] = Hmsg_Com1->CmdBuf[5];
			}
		}
		else	/* 42HAccess with command */
		{
			/* Select the axis to be traced1" */
			TrcIncivPrm->TrcAxis[i] = 0x0000;
			TrcIncivPrm->IOTrcAxis[i] = 0x000;
		}

		TrcIncivPrm->TrcData[i] = TrcReg->DataIOTrcSel[i] & 0x00FF;				/* Trace data setting */
		TrcIncivPrm->IOTrcData[i] = (TrcReg->DataIOTrcSel[i] >> 8) & 0x00FF;	/* I/OInformation setting */
	}

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
 2120H:Data trace target axis selection ~ data trace elephant axis selection
 2130H:Selection of data to be traced
 0x2140:I/OSelect the axis to be traced
 0x2150:I/OTrace target data selection 
 ****************************************************************************/
PRM_RSLT fnCalcTraceDataSel(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
	TRCHNDL	*TrcHndl;
	ULONG	i;
	TRCREG	*TrcReg;
	TRCINDIVPRM	*TrcIncivPrm;
	TrcHndl = ((AXIS_HANDLE*)Axis)->TrcHndl;
	TrcReg  = &TrcHndl->TrcReg;
	TrcIncivPrm = &TrcHndl->TrcIncivPrm;

	/* Set Axis Address & Trace Data */
	for( i=0; i < TRC_CH_MAXNUM; i++ )
	{
		TrcIncivPrm->TrcData[i] = TrcReg->TrcDataSel[i];		/* Selection of data to be traced(1~16)			*/
		TrcIncivPrm->TrcAxis[i] = TrcReg->TrcAxisSel[i];		/* Data trace target axis selection(1~16)				*/
		TrcIncivPrm->IOTrcData[i] = TrcReg->IOTrcDataSel[i];	/* I/OTrace1Target data selection(1~16)			*/
		TrcIncivPrm->IOTrcAxis[i] = TrcReg->IOTrcAxisSel[i];	/* I/OTrace1Target axis selection(1~16)				*/
	}

//	return PRM_RSLT_SUCCESS;
	return( fnCalcTraceRegisters( Cmd, ArrayIdx, Axis, pValue ) );		/* <S208> */
}
/* <S0C9>	End */

/****************************************************************************
 2300H:Table operation control mode
 2301H:Table operation rotation amount limit
 2303H:Table operation function selection
 ****************************************************************************/
PRM_RSLT fnCalcTblDriveRegisters(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRM_RSLT ErrRes;

/* <S0C9>	Start */
	TRCHNDL	*TrcHndl;
	ULONG	i;
	TRCREG	*TrcReg;
	TRCINDIVPRM	*TrcIncivPrm;
	TrcHndl = ((AXIS_HANDLE*)Axis)->TrcHndl;
	TrcReg  = &TrcHndl->TrcReg;
	TrcIncivPrm = &TrcHndl->TrcIncivPrm;
/* <S0C9>	End */

	/* Sets the Table Drive Parameters */
	ErrRes =
		TblDriveCalculateParameter( &(((AXIS_HANDLE*)Axis)->BaseControls->TblDrive),
									((AXIS_HANDLE*)Axis)->UniPrms->Bprm->XfbPulse1 );

/* <S0C9>	Start */
	/* Set Axis Address & Trace Data */
	for( i=0; i < TRC_CH_MAXNUM; i++ )
	{
		TrcIncivPrm->TrcData[i] = TrcReg->TrcDataSel[i];		/* Selection of data to be traced(1~16)			*/
		TrcIncivPrm->TrcAxis[i] = TrcReg->TrcAxisSel[i];		/* Data trace target axis selection(1~16)				*/
		TrcIncivPrm->IOTrcData[i] = TrcReg->IOTrcDataSel[i];	/* I/OTrace1Target data selection(1~16)			*/
		TrcIncivPrm->IOTrcAxis[i] = TrcReg->IOTrcAxisSel[i];	/* I/OTrace1Target axis selection(1~16)				*/
	}
/* <S0C9>	End */

	/* ReCulc Trace Parameters for Trace Gain Calc */
	ErrRes = DtrcCalcParameter( ((AXIS_HANDLE*)Axis)->TrcHndl,
											((AXIS_HANDLE*)Axis)->TraceBuffer );

	return ErrRes;
}


/****************************************************************************
 2C00H:	Motor constant area
****************************************************************************/
PRM_RSLT fnCalcMotorParameter(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT	mode;

	if(ArrayIdx >= 0x200)
	{
		ArrayIdx -= 0x200;
	}

	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
		mode = FunGetOpeModeRegister(((AXIS_HANDLE*)Axis)->FnManager,FUN_CH1);
		if((mode == 0x200B) || (mode == 0x200C))
		{
			((AXIS_HANDLE*)Axis)->TraceBuffer[ArrayIdx] = (USHORT)*pValue;
		}
		break;
	case PRM_ACCCMD_READ:
		*pValue = (LONG)((AXIS_HANDLE*)Axis)->TraceBuffer[ArrayIdx];
		break;
	default:
		break;
	}
	return PRM_RSLT_SUCCESS;
}


/* <S061>	Start */
/****************************************************************************
0x2240	Remaining number of alarm trace executions
****************************************************************************/
PRM_RSLT fnAlmTrcRemainingNum(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
/* The processing of this function is enabled after the alarm trace function is completed. */
#if 0

#define	MAX_ALM_TRC_NUM	3000	/* Maximum number of alarm trace writes to non-volatile memory */

	*pValue = MAX_ALM_TRC_NUM - almTrcExecutionCount;  /* alarm trace execution count */
#endif

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x2241	Alarm trace memory write status
****************************************************************************/
PRM_RSLT fnAlmTrcWritingStat(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
/* The processing of this function is enabled after the alarm trace function is completed. */
#if 0

#define	NONE	0	/* Non-volatile memory not written */
#define	WRITING	1	/* Non-volatile memory write status */

	/* Acquisition of alarm trace status */

	if(getStatus)	/* Non-volatile memory write status */
	{
		*pValue = WRITING;
	}
	else		/* Non-volatile memory not written */
	{
		*pValue = NONE;
	}

#endif

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x2242	Alarm trace data size
****************************************************************************/
PRM_RSLT fnAlmTrcDataSize(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
/* The processing of this function is enabled after the alarm trace function is completed. */
#if 0

#define	ALM_TRC_DATA_SIZE	4096	/* Non-volatile memory not written */




/* Alarm trace data size	= Number of division traces X Number of trace points/1Main X Alarm trace header	*/
/*								= 7 x 500 + 596														*/
/*								= 4096[Register]													*/

	*pValue = ALM_TRC_DATA_SIZE;

#endif
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x2243	Alarm trace stop request
****************************************************************************/
PRM_RSLT fnAlmTrcStopReq(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
/* The processing of this function is enabled after the alarm trace function is completed. */
#if 0

#define	ALM_TRC_DATA_SIZE	4096	/* Non-volatile memory not written */




/*
	Alarm trace memory write status = 1, alarm tracing cannot be stopped.
	Internally goes into standby state
	
*/

if(alarmStatus = WRITING)  /* alarm state = writing */
{

	/* We must decide what to do in the writing state. */
}
else/* Alarm trace not being written */
{
	/* Alarm trace stop requestON */
}

#endif
	return PRM_RSLT_SUCCESS;
}
/* <S061>	End */

/* <S0CB>	Start */
/***************************************************************************
2801H:	Gain adjustment Motor current detection adjustment:UPhase
***************************************************************************/
PRM_RSLT FnSetCurDetectGainAdjU(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	PRMDATA	*Prm;
	Prm = Axis->UniPrms->Prm;

/*--------------------------------------------------------------------------------------------------*/
/*		Current detection zero adjustment																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* UPhase current zero adjustment */
//	kx = (SHORT)(uCfgPrm->zadjiu);
//	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->AdinV.IuOffsetIn), kx, NO_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* VPhase current zero adjustment */
//	kx = (SHORT)(uCfgPrm->zadjiv);
//	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->AdinV.IvOffsetIn), kx, NO_LMT_CHK );
//	*pValue = (LONG)(Axis->SvAsicReg->MicroReg->IntAdv.IuIndata);

//	*pValue = (LONG)(Axis->SvAsicRegs->AsicMicroReg->AdinV.IuOffsetIn);	/* <S1B1> */
	*pValue = (LONG)(Axis->SvAsicRegs->AsicMicroReg->IntAdV.IuInData);	/* <S1B1> */

	return PRM_RSLT_SUCCESS;
}

/***************************************************************************
2802H:	Gain adjustment Motor current detection adjustment:VPhase
***************************************************************************/
PRM_RSLT FnSetCurDetectGainAdjV(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	PRMDATA	*Prm;
	Prm = Axis->UniPrms->Prm;

//	*pValue = (LONG)(Axis->SvAsicRegs->AsicMicroReg->AdinV.IvOffsetIn);	/* <S1B1> */
	*pValue = (LONG)(Axis->SvAsicRegs->AsicMicroReg->IntAdV.IvInData);	/* <S1B1> */

	return PRM_RSLT_SUCCESS;
}

/***************************************************************************
2803H:	PNBus voltage
***************************************************************************/
PRM_RSLT FnSetDcVoltx(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (FLOAT_USE==TRUE)
	*pValue = Axis->PowerManager->PowerSts.DcVolt *
	                      Axis->PowerManager->DetectDcVolt.conf.Kdcvolt;
#else
	*pValue = MlibMulgain(Axis->PowerManager->PowerSts.DcVolt,
	                      Axis->PowerManager->DetectDcVolt.conf.Kdcvolt );
#endif /* FLOAT_USE */

	return PRM_RSLT_SUCCESS;
}

/***************************************************************************
2805H	Overheat AD input value monitor 1
***************************************************************************/
PRM_RSLT FnInputOHAdMon1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)/* <S0E1> */
{

	CADC *pAdc;

	pAdc = Axis->pAdc;

//	*pValue = pAdc->Var[ADC3_OHIAD];/* Read OH A/D */
	*pValue = MlibMulgain(pAdc->Var[ADC3_OHIAD], OH_ADC2VOL);/* <S0E1> */

	return PRM_RSLT_SUCCESS;
}
/* <S0CB>	End */

/***************************************************************************
2806H	Overheat AD input value monitor 2
***************************************************************************//* <S0E1> */
PRM_RSLT FnInputOHAdMon2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	CADC *pAdc;

	pAdc = Axis->pAdc;

	*pValue = MlibMulgain(pAdc->Var[ADC4_OHIAD], OH_ADC2VOL);

	return PRM_RSLT_SUCCESS;
}
#if 0	/* <S0FD> Delete Start : Specification change to the following2Comment out */
/* <S0E7> Start */
/***************************************************************************
2807H	Analog monitorPWMOutput adjustment monitorU
***************************************************************************/
PRM_RSLT FnPwmAdjMonU(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	AMONOUT	*AmonOut;

	AmonOut = (AMONOUT*)&(Axis->AmonOut[0]);


	if(Cmd == PRM_ACCCMD_READ)
	{
		AmonOut[0].RegVoltMon_Flag_Ch0 = FALSE;
		*pValue = AmonOut[0].RegVoltMonSetVal_Ch0;
	}
	else
	{
		AmonOut[0].RegVoltMon_Flag_Ch0 = TRUE;
		FnOutputAnalogMonitor( Axis, *pValue, 0 );
	}

	return PRM_RSLT_SUCCESS;
}

/***************************************************************************
2808H	Analog monitorPWMOutput adjustment monitorV
***************************************************************************/
PRM_RSLT FnPwmAdjMonV(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	AMONOUT	*AmonOut;

	AmonOut = (AMONOUT*)&(Axis->AmonOut[0]);


	if(Cmd == PRM_ACCCMD_READ)
	{
		AmonOut[0].RegVoltMon_Flag_Ch1 = FALSE;
		*pValue = AmonOut[1].RegVoltMonSetVal_Ch1;
	}
	else
	{
		AmonOut[0].RegVoltMon_Flag_Ch1 = TRUE;
		FnOutputAnalogMonitor( Axis, *pValue, 1 );
	}
	return PRM_RSLT_SUCCESS;
}
/* <S0E7> End */
#endif	/* <S0FD> Delete End:Specification change to the above2Comment out */
/****************************************************************************
 3000H:	Engineering area
****************************************************************************/
PRM_RSLT fnAceessEngneeringInfo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	EEP_DEVICE_HANDLE	*EepDevice; /* <S1D2> */
	UINT				AxisNo;
	USHORT				EepValue;

//	EepDevice = (Axis->RegManager->hPnReg)->hEepDev; /* <S1D2> */
	AxisNo = ((AXIS_HANDLE*)Axis)->AxisNo;
	EepValue = (USHORT)*pValue;

	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
		/* write to EEPROM */
		EepdevWriteNoSumValues( 0, /* <S1D2> */
								EEP_NOSUM_ADDRESS(AxisNo, EEP_ENGINFO_OFS) + (ArrayIdx*2),
								&EepValue,
								1 );
		break;
	case PRM_ACCCMD_READ:
		/* read from EEPROM */
		EepdevReadValues( 0, /* <S1D2> */
						  EEP_NOSUM_ADDRESS(AxisNo, EEP_ENGINFO_OFS) + (ArrayIdx*2),
						  &EepValue,
						  1 );
		*pValue = (LONG)EepValue;
		break;
	default:
		break;
	}

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x70000	EEPROMDirect access area(IF)
****************************************************************************/
static PRM_RSLT fnDirectEepromAcc(UINT devId, PRM_ACCCMD Cmd, UINT Address, LONG *pValue)
{
	USHORT EepValue;
	LONG err;

	EepValue = (USHORT)*pValue;

	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
		/* write to EEPROM */
		err = EepdevWriteNoSumValues( devId, Address, &EepValue, 1 );
		break;
	case PRM_ACCCMD_READ:
		/* read from EEPROM */
		err = EepdevReadValues( devId, Address, &EepValue, 1 );
		*pValue = (LONG)EepValue;
		break;
	default:
		err = 0;
		break;
	}

	if(err != 0)
	{
		return PRM_RSLT_RWACC_ERR;
	}
	return PRM_RSLT_SUCCESS;
}

PRM_RSLT fnDirectEepromAccessIF(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	EEP_DEVICE_HANDLE	*EepDevice; /* <S1D2> */
//	EepDevice = (Axis->RegManager->hPnReg)->hEepDev; /* <S1D2> */
	return fnDirectEepromAcc(PRMDEF_EEPROM_IF, Cmd, ArrayIdx*2, pValue);
}

/****************************************************************************
0x74000	EEPROMDirect access area(COA)
****************************************************************************/
PRM_RSLT fnDirectEepromAccessCNV(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	EEP_DEVICE_HANDLE	*EepDevice;/* <S1D2> */
//	EepDevice = (Axis->RegManager->hPnReg)->hEepDev;/* <S1D2> */
	return fnDirectEepromAcc(PRMDEF_EEPROM_COA, Cmd, ArrayIdx*2, pValue);
}

/****************************************************************************
0x74080	EEPROMDirect access area(MBB)
****************************************************************************/
PRM_RSLT fnDirectEepromAccessMB(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	EEP_DEVICE_HANDLE	*EepDevice;/* <S1D2> */
//	EepDevice = (Axis->RegManager->hPnReg)->hEepDev;/* <S1D2> */
	return fnDirectEepromAcc(PRMDEF_EEPROM_MBB, Cmd, ArrayIdx*2, pValue);
}

/****************************************************************************
0x74800	EEPROMDirect access area(SDA)
****************************************************************************/
PRM_RSLT fnDirectEepromAccessAMP(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	EEP_DEVICE_HANDLE	*EepDevice;/* <S1D2> */
	UINT				idx;

	idx = (Axis->RegManager->hPnReg)->EepromBlockAddress + (ArrayIdx*2);
//	EepDevice = (Axis->RegManager->hPnReg)->hEepDev;/* <S1D2> */
	return fnDirectEepromAcc(PRMDEF_EEPROM_SDA, Cmd, idx, pValue);
}

/***************************************************************************
0x75000	Test mode
***************************************************************************/
PRM_RSLT fnTestMode(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	POW_MNG_HNDL	*hPowerManager;
	TEST_MODE		testMode;


	ASIC_HW_IF_STR		*pAsicHwReg;		/* <S0CB> */
	CHECK_MAIN_POWER	*PowerChecker;		/* <S0CB> */
	LONG				RlyDbWay;			/* <S0CB> */
//	POWER_STATUS		*pPowSts;			/* <S0CB> */
//	USHORT				FaultState;			/* <S0CB> */
//	BOOL				DBRequest;			/* <S0CB> */
//	BOOL				AdjCurAD;			/* <S0CB> */



	hPowerManager = Axis->PowerManager;
	testMode = (TEST_MODE)*pValue;

	pAsicHwReg = Axis->SvAsicRegs->AsicHwReg;				/* <S0CB> */
	PowerChecker = &hPowerManager->MainPowerChecker;		/* <S0CB> */
	RlyDbWay = PowerChecker->conf.RlyDbWay;					/* <S0CB> */
//	pPowSts = &hPowerManager->PowerSts;						/* <S0CB> */
//	FaultState |= Axis->CheckAlarm->faultState;				/* <S0CB> */
//	DBRequest |= Axis->BeSequence->DbOn;					/* <S0CB> */
//	AdjCurAD = Axis->FnCmnCtrl->KnlMode;					/* <S0CB> */


	switch(Cmd)
	{
		case PRM_ACCCMD_WRITE:
		case PRM_ACCCMD_EEPWRITE:
/* <S0CB>	Start */
/* ifstatementswitchChange to sentence(Due to many branches) */
#if 0

			if( testMode == TEST_IF_WDTKICK )
			{
				HALdrv_KickIfWdt( );
			}
			else if( testMode == TEST_ASIC_WDTSTOP )
			{
				hPowerManager->MainPowerChecker.conf.TestMode4 = TRUE;
			}
			else if( testMode == TEST_ASIC_WDTKICK )
			{
				hPowerManager->MainPowerChecker.conf.TestMode4 = FALSE;
			}
			/* Main circuit power supply shutdown && DBCTRL_RLY_SEPARATE(DB, Rush prevention relay separation)inRELAY ON/OFFin total! */
			else if( testMode == TEST_RELAY_ON )
			{
				hPowerManager->MainPowerChecker.conf.TestMode5 = TRUE;
				/* When not in motorless test modeON *//* <S1B7> */
				/* TODO: SGD-V, the following conditions were also achieved by disabling interruptsORis						*/
				/* (1) AxisRscC->BeSequence->StopMode == STOPMODE_DBSTOPFR		*/
				/* (2) AxisRscC->BeSequence->StopMode == STOPMODE_FREERUN		*/
				/* (3) AxisRscC->FnCmnCtrl->KnlMode == KPI_KMD_ADJCURAD			*/
				if( Axis->MotorLessTest->MotorLessTestMode == FALSE )
				{
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, TRUE ); /* <S0A2> */
				}
			}
			/* Main circuit power supply shutdown && DBCTRL_RLY_SEPARATE(DB, Rush prevention relay separation)inRELAY ON/OFFin total! */
			else if( testMode == TEST_RELAY_OFF )
			{
				hPowerManager->MainPowerChecker.conf.TestMode5 = FALSE;
				HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, FALSE ); /* <S0A2> */
			}
#endif

		switch(testMode)
		{
			case TEST_IF_WDTKICK:
				HALdrv_KickIfWdt( );
				break;

			case TEST_ASIC_WDTSTOP:
				hPowerManager->MainPowerChecker.conf.TestMode4 = TRUE;
				break;

			case TEST_ASIC_WDTKICK:
				hPowerManager->MainPowerChecker.conf.TestMode4 = FALSE;
				break;

			/* Rush prevention relay ON */
			case TEST_RELAY_ON: /* <S20E> */
				hPowerManager->MainPowerChecker.conf.TestMode5 = TRUE;
				switch ( RlyDbWay )
				{
				case DBCTRL_RLY_COMMON:			/* Inrush + DB: Relay common */
				case DBCTRL_TRIACK_SEPARATE:	/* Rush: Relay; DB: Thyristor */
				case DBCTRL_RLY_SEPARATE:		/* Rush: Relay; DB: Relay */
					/* Relay On */
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, TRUE );
					break;
				case DBCTRL_TRIACK_COMMON:		/* Inrush + DB: Thyristor */
					/* Relay On */
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, TRUE );
					/* DB Pulse Output */
					SHalDrv_DynamicBrakeON(pAsicHwReg);
					break;
				}
				break;

			/* Rush prevention relay OFF */
			case TEST_RELAY_OFF: /* <S20E> */
				hPowerManager->MainPowerChecker.conf.TestMode5 = FALSE;
				switch ( RlyDbWay )
				{
				case DBCTRL_RLY_COMMON:			/* Inrush + DB: Relay common */
				case DBCTRL_TRIACK_SEPARATE:	/* Rush: Relay; DB: Thyristor */
				case DBCTRL_RLY_SEPARATE:		/* Rush: Relay; DB: Relay */
					/* Relay Off */
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, FALSE );
					break;
				case DBCTRL_TRIACK_COMMON:		/* Inrush + DB: Thyristor */
					/* Relay Off */
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, FALSE );
					/* DB Pulse Output */
					SHalDrv_DynamicBrakeON(pAsicHwReg);
					break;
				}
				break;

			/* Regenerative control ON */
			case TEST_REGENE_ON:
				hPowerManager->MainPowerChecker.conf.TestMode6 = TRUE;
				HALdrv_RegeneControl( TRUE );
				break;

			/* Regenerative control OFF */
			case TEST_REGENE_OFF:
				hPowerManager->MainPowerChecker.conf.TestMode6 = FALSE;
				HALdrv_RegeneControl( FALSE );
				break;

			/* DBControl ON */
			case TEST_DB_ON: /* <S20E> */
				hPowerManager->MainPowerChecker.conf.TestMode7 = TRUE;
				switch ( RlyDbWay )
				{
				case DBCTRL_RLY_COMMON:			/* Inrush + DB: Relay common */
					/* Relay Off */
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, FALSE );
					break;
				case DBCTRL_TRIACK_SEPARATE:	/* Rush: Relay; DB: Thyristor */
				case DBCTRL_RLY_SEPARATE:		/* Rush: Relay; DB: Relay */
					SHalDrv_DynamicBrakeON(pAsicHwReg);
					break;
				case DBCTRL_TRIACK_COMMON:		/* Inrush + DB: Thyristor */
					/* Relay Off */
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, FALSE );
					/* DB Pulse Output */
					SHalDrv_DynamicBrakeON(pAsicHwReg);
					break;
				}
				break;

			/* DBControl OFF */
			case TEST_DB_OFF: /* <S20E> */
				hPowerManager->MainPowerChecker.conf.TestMode7 = FALSE;
				switch ( RlyDbWay )
				{
				case DBCTRL_RLY_SEPARATE:		/* Rush: Relay; DB: Relay */
					/* Relay On */
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, TRUE );
					break;
				case DBCTRL_RLY_COMMON:			/* Inrush + DB: Relay common */
				case DBCTRL_TRIACK_SEPARATE:	/* Rush: Relay; DB: Thyristor */
					SHalDrv_DynamicBrakeOFF(pAsicHwReg);
					break;
				case DBCTRL_TRIACK_COMMON:		/* Inrush + DB: Thyristor */
					/* Relay On */
					HALdrv_RelayControl( (void*)Axis->SvAsicRegs->AsicHwReg, TRUE );
					/* DB Pulse Output */
					SHalDrv_DynamicBrakeON(pAsicHwReg);
					break;
				}
				break;

			/* Gate drive signal ON */
			case TEST_GATE_DRIVE_ON:
				hPowerManager->MainPowerChecker.conf.TestMode8 = TRUE;
				SHalDrv_ChargePumpON(pAsicHwReg);	/* Nside charge pump ON */
				SHalDrv_ChargePumpON_P(pAsicHwReg);	/* Pside charge pump ON */
				break;

			/* Gate drive signal OFF */
			case TEST_GATE_DRIVE_OFF:
				hPowerManager->MainPowerChecker.conf.TestMode8 = FALSE;
				SHalDrv_ChargePumpOFF(pAsicHwReg);
				break;

/* <S19F> Start */
			/* Gate drive signalScanAON/OFFStart(Duty = 50%, 8[kHz]inPWMOutput start) */
			case TEST_GATE_DRIVE_ON_OFF_START:
				hPowerManager->MainPowerChecker.conf.TestMode9 = TRUE;
				hPowerManager->MainPowerChecker.conf.GateDriveSigTestFlag = TRUE;	/* Gate drive signal status flag(ScanA) ON */
				SHalDrv_ChargePumpOFF(pAsicHwReg);									/* Gate drive signal OFF */
				break;

			/* Gate drive signalScanAON/OFFStop(Duty = 50%, 8[kHz]inPWMOutput stop) */
			case TEST_GATE_DRIVE_ON_OFF_STOP:
				hPowerManager->MainPowerChecker.conf.TestMode9 = FALSE;
				hPowerManager->MainPowerChecker.conf.GateDriveSigTestFlag = FALSE;	/* Gate drive signal status flag(ScanA) OFF */
				SHalDrv_ChargePumpOFF(pAsicHwReg);									/* Gate drive signal OFF */
				break;
/* <S19F> End */

			default:
				break;
		}
/* <S0CB>	End */
			break;

		default:
			break;
	}

	return PRM_RSLT_SUCCESS;
}

/***************************************************************************
0x2802	RAM monitor
***************************************************************************/
PRM_RSLT fnRamMonitor(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	static UINT32	RamAddress=0;

	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
		RamAddress = *pValue;
		break;

	case PRM_ACCCMD_READ:
		if(RamAddress & 1)
		{	*pValue = *((UINT8 *)RamAddress);}
		else if(RamAddress & 2)
		{	*pValue = *((UINT16 *)RamAddress);}
		else
		{	*pValue = *((UINT32 *)RamAddress);}
		break;

	default:
		break;
	}

	return PRM_RSLT_SUCCESS;
}


/******************************* End of File *******************************/


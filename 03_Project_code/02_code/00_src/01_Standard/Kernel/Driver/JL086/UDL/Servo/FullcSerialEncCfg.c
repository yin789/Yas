/*****************************************************************************************************
Description	: Serial Encoder Command Interface
------------------------------------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

------------------------------------------------------------------------------------------------------
Changes	      :
	Name   Date          Description
	--------------------------------------------------------------------------------------------------
	I.M.   2011.02.08    created
	H.H.   2013.10.08    Re-created

*****************************************************************************************************/
#include "Basedef.h"
#include "SerialEncIf.h"
#include "MLib.h"
#include "KLib.h"


#if	(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)/* <S04D> */
/*****************************************************************************************************
*																									  
*		Fulk encoder : Position sensor type setting														  
*																									  
******************************************************************************************************
*																									  
*	Machine Efficiency : Determine absolute/incremental/converter etc. from encoder parameters and set internal variables.	  
*																									  
*****************************************************************************************************/
BOOL	FencSetSensorType( MENCV *FencV, OPFBK_HNDL *FbkOpt )
{
BOOL rc;

	rc = FALSE;

	switch(FencV->TypeCode)
	{
	case 'E': /* EnDat type							*/
		FencV->SerialConvType = SCONVTYPE_ENDAT_ABS;
		FencV->AbsoEncoder = TRUE;
		FencV->OrgClrByWrAdrCmd = FALSE;
		FencV->AxisMotType = MOTTYPE_LINEAR;
		FbkOpt->var.f.PrmSumErrAsFulc = FALSE;				/* Checksum abnormality flag initialization			*/
		break;
	case 'A': /* Mitutoyo absolute type						*/
		FencV->SerialConvType = SCONVTYPE_MITUTOYO_ABS;
		FencV->AbsoEncoder = TRUE;
		FencV->OrgClrByWrAdrCmd = FALSE;
		FencV->AxisMotType = MOTTYPE_LINEAR;
		FbkOpt->var.f.PrmSumErrAsFulc = FALSE;				/* Checksum abnormality flag initialization			*/
		break;
	case 'D': /* SMS increment type 						*/
		FencV->SerialConvType = SCONVTYPE_SONY_INC;
		FencV->AbsoEncoder = FALSE;
		FencV->OrgClrByWrAdrCmd = FALSE;
		FencV->AxisMotType = MOTTYPE_LINEAR;
		FbkOpt->var.f.PrmSumErrAsFulc = FALSE;				/* Checksum abnormality flag initialization			*/
		break;
	case 'L': /* Standard increment type 						*/
		FencV->SerialConvType = SCONVTYPE_STANDARD_INC;
		FencV->AbsoEncoder = FALSE;
		FencV->OrgClrByWrAdrCmd = TRUE;
		FencV->AxisMotType = MOTTYPE_LINEAR;
		FbkOpt->var.f.PrmSumErrAsFulc = FALSE;				/* Checksum abnormality flag initialization			*/
		break;
	default: /* Abnormal system				 						*/
		if(FbkOpt->conf.OpInfo.IfType == FBKOP_SCNV_MODULE )
		{
			rc = TRUE;										/* A.CF1 : FPG communication error				*/
			FencV->SerialConvType = SCONVTYPE_NONE;
			FencV->OrgClrByWrAdrCmd = FALSE;
			FencV->AbsoEncoder = FALSE;
			FencV->AxisMotType = MOTTYPE_LINEAR;
			FbkOpt->var.f.PrmSumErrAsFulc = TRUE;			/* When detector type setting is not completedSumRegarded as abnormal	*/
		}
		break;
	}
	return rc;
}
/*****************************************************************************************************
*
*	Fulk encoder : Type code setting
*	Based on the encoder ID information, determine increment/endat.
*
*****************************************************************************************************/
static BOOL FencSetEncoderType(MENCV *FencV, OPFBK_HNDL *FbkOpt, DBYTEX BitnoDpoint)
{
BOOL rc;

	if( FbkOpt->conf.OpInfo.IfType != FBKOP_SCNV_MODULE )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	When not feedback option for serial converter type							*/
		/*------------------------------------------------------------------------------------------*/
		if((FencV->EncInfo.Model[2] == 'D') && (FencV->EncInfo.Model[3] == 'P'))
		{
			rc = FencSetSensorType( FencV, FbkOpt );				/* Fulque encoder position sensor type setting	*/
		}
		else
		{
			rc = TRUE; /* A.CF1 : FPG communication error */
		}
	}
	else
	{
		/*------------------------------------------------------------------------------------------*/
		/*	When feedback option is used for serial converter type								*/
		/*------------------------------------------------------------------------------------------*/
		rc = FencSetSensorType( FencV, FbkOpt );					/* Fulque encoder position sensor type setting	*/
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Absolute value encoder usage setting																*/
	/*----------------------------------------------------------------------------------------------*/
	if(FencV->AbsoEncoder != FALSE)
	{
		FencV->AbsoEncType = MENC_MABSO_TYPE;
		FencV->EncType = (0x0100 | BitnoDpoint.b.l);
	}
	else
	{
		FencV->AbsoEncType = MENC_INCRE_TYPE;
		FencV->EncType = (0x0000 | BitnoDpoint.b.l);
	}

	return rc;
}


/*****************************************************************************************************
*
*	Fulk encoder : Read parameter
*
*****************************************************************************************************/
static SENC_ERROR FencReadParamAll(MENCV *FencV, OPFBK_HNDL *FbkOpt, MENCPRM *MencP)
{
	UCHAR	PrmBuf[8];

	/*----------------------------------------------------------------------------------------------*/
	/*	Encoder ID reading process																	*/
	/*----------------------------------------------------------------------------------------------*/
	if(FALSE == SencReadParam8byte(&FencV->EncIf, 0x00, PrmBuf))
	{
		return (SENCERR_PGCOM);		/* A.CF1 : FPG communication error	*/
	}
	SencSetIDINFO(0x00, PrmBuf, &FencV->EncInfo);

	/*----------------------------------------------------------------------------------------------*/
	if(FALSE == SencReadParam8byte(&FencV->EncIf, 0x08, PrmBuf))
	{
		return (SENCERR_PGCOM);		/* A.CF1 : FPG communication error	*/
	}
	SencSetIDINFO(0x08, PrmBuf, &FencV->EncInfo);

	/*----------------------------------------------------------------------------------------------*/
	if(FALSE == SencReadParam8byte(&FencV->EncIf, 0x10, PrmBuf))
	{
		return (SENCERR_PGCOM);		/* A.CF1 : FPG communication error	*/
	}
	SencSetIDINFO(0x10, PrmBuf, &FencV->EncInfo);

	/*----------------------------------------------------------------------------------------------*/
	if(FALSE == SencReadParam8byte(&FencV->EncIf, 0x18, PrmBuf))
	{
		return (SENCERR_PGCOM);		/* A.CF1 : FPG communication error	*/
	}
	SencSetIDINFO(0x18, PrmBuf, &FencV->EncInfo);

	/*----------------------------------------------------------------------------------------------*/
	/*	Linear type code																			*/
	/*----------------------------------------------------------------------------------------------*/
	if(FALSE == SencReadParam8byte(&FencV->EncIf, 0x40, PrmBuf))
	{
		return (SENCERR_PGCOM);		/* A.CF1 : FPG communication error	*/
	}
	FencV->TypeCode = (LONG)(PrmBuf[0]);

	/*----------------------------------------------------------------------------------------------*/
	/*	Number of bits,Decimal point position																			*/
	/*----------------------------------------------------------------------------------------------*/
	if(FALSE == SencReadParam8byte(&FencV->EncIf, 0x48, PrmBuf))
	{
		return (SENCERR_PGCOM);		/* A.CF1 : FPG communication error	*/
	}
	MencP->BitnoDpoint.b.l = PrmBuf[2];
	MencP->BitnoDpoint.b.h = PrmBuf[3];

	/*----------------------------------------------------------------------------------------------*/
	/*	Encoder software version															*/
	/*----------------------------------------------------------------------------------------------*/
	if(FALSE == SencReadParam8byte(&FencV->EncIf, 0x58, PrmBuf))
	{
		return (SENCERR_PGCOM);		/* A.CF1 : FPG communication error	*/
	}
	if( FbkOpt->conf.OpInfo.IfType == FBKOP_SCNV_MODULE )
	{
	/* CPUYesFBOptWhen connected, option cardVerand FlukeVerSet parameter value to */
		FbkOpt->conf.OpInfo.SoftVer = PrmBuf[7];
		FencV->SoftVer = PrmBuf[7];
	}
	else
	{
	/* CPUNoneFBOptFully when connectedVerSet parameter value to */
		FencV->SoftVer = PrmBuf[7];
	}
	
	/*	Insert termination code into ID information	*/
	SencFillNullIDINFO(&FencV->EncInfo);

	/*----------------------------------------------------------------------------------------------*/
	/*	Encoder type setting																		*/
	/*----------------------------------------------------------------------------------------------*/
	if(FALSE != FencSetEncoderType(FencV, FbkOpt, MencP->BitnoDpoint))
	{
		return (SENCERR_PGCOM);		/* A.CF1 : FPG communication error	*/
	}

	return (SENCERR_SUCCESS);
}

/*****************************************************************************************************
*
*	Fulque encoder basic parameter calculation
*	Servo basic parameter calculation(BprmCalc.c)
*
*****************************************************************************************************/
void FencBasePrmCalc(MENCV *FencV, MENCPRM *MencP, USHORT b_prm2, ULONG fcplsml,BOOL rvsDir)
{
	/*----------------------------------------------------------------------------------------------*/
	/*	Setting operation direction																				*/
	/*----------------------------------------------------------------------------------------------*/
	switch((b_prm2 >> 12) & 0x0F)
	{
	case 0: /* Not used													*/
		FencV->RevAsmMotor = FALSE;
		break;
	case 1: /* Used in standard operating direction										*/
	case 2: /* A,B,C phase disconnection mask not available 										*/
		FencV->RevAsmMotor = FALSE;
		break;
	case 3: /* Used in reverse operation direction										*/
	case 4: /* A,B,C phase disconnection mask not available										*/
		FencV->RevAsmMotor = TRUE;
		break;
	default:/* Parameter setting error											*/
		FencV->RevAsmMotor = FALSE;
		break;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Setting the number of pulses calculated per motor cycle : [pulse/MotRev]												*/
	/*----------------------------------------------------------------------------------------------*/
	FencV->PulseNo = fcplsml << MencP->BitnoDpoint.b.l;	/* MotPuseNo*/
	FencV->HalfPulseNo = FencV->PulseNo >> 1;

	/*----------------------------------------------------------------------*/
	/*	Reverse rotation mode selectionFlag							<S02D>			*/
	/*----------------------------------------------------------------------*/
	FencV->DivInfo.RvsDir = rvsDir;
}


/*****************************************************************************************************
*
*	Fulque encoder initialization/connection processing
*
*****************************************************************************************************/
SENC_ERROR FencInitEncoder( MENCV *FencV, MENC_INIT_PRM *MencInitPrm, ASIC_HW_IF_STR *AsicHwReg,
										OPFBK_HNDL *FbkOpt, USHORT EncBaudrate, BOOL SencFbOptUse, BOOL rvsDir )
{
	SENC_ERROR err;

	err = SencConnect(FencV, AsicHwReg, EncBaudrate, FbkOpt, SencFbOptUse );
	if(SENCERR_SUCCESS == err)
	{
		/*	FencReadParamAll*/
		err = FencReadParamAll(FencV, FbkOpt, MencInitPrm->MencP);
		if(SENCERR_SUCCESS != err)
		{/* try again... */
			err = FencReadParamAll(FencV, FbkOpt, MencInitPrm->MencP);
		}
		if(SENCERR_SUCCESS == err)
		{
			/*----------------------------------------------------------------------------------*/
			/*		Sending origin reset command													*/
			/*----------------------------------------------------------------------------------*/
//			err = SencSendRstOrgCmd(FencV);
			/*	Calculate the base parameters for Fully closed encoder */
			FencBasePrmCalc(FencV, MencInitPrm->MencP, MencInitPrm->b_prm2, MencInitPrm->fcplsml, rvsDir);/*<S02D>*/
		}
	}

	return err;
}



/*****************************************************************************************************
*
*	Fulk encoder : Initial parameter calculation
*	Servo basic parameter calculation(BprmCalc.c), the parameters will be calculated
*
*****************************************************************************************************/
static void FencInitPrmCalc(MENCV *FencV, MENCPRM *MencP,
                            USHORT scnvorgoffset, BOOL OrgOffsetEnable, LONG KConvLoadSpd)
{
	UCHAR 	BitNo;
	USHORT 	uswk;

	BitNo = MencP->BitnoDpoint.b.l;
	/*----------------------------------------------------------------------*/
	/*	Calculation of positional calculation parameters											*/
	/*----------------------------------------------------------------------*/
	if( FencV->PosCompMode != POSCOMP_SW )/*<S0C5>rename*/
	{/* When using a position correction module, invert within the position correction module	*/
		FencV->MposSign = 1;
	}
	else
	{
		if(FencV->RevAsmMotor != FALSE)
		{
			FencV->MposSign = -1;
		}
		else
		{
			FencV->MposSign = 1;
		}
	}
	FencV->KConvLoadSpd = KConvLoadSpd;

	/*----------------------------------------------------------------------*/
	/*	C phase less flag setting													*/
	/*----------------------------------------------------------------------*/
	if(FencV->AbsoEncoder)
	{
		FencV->CphaseLess = TRUE;
	}
	else
	{
		FencV->CphaseLess = FALSE;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Shift data for position data calculation																*/
	/*----------------------------------------------------------------------------------------------*/
	/*	If in motorless test mode,BitNo=8, Dpoint=68,
	 * 	Shift amount12(=FSX_FPOS_SFT)Therefore, it is not included in the conditions
	 *----------------------------------------------------------------------*/
//	if(FbkOptP.OpInfo.IfType == FBKOP_SCNV_MODULE)
//	{
//		FencV->MposSftR = 32 - BitNo;
//		FencV->MposSftX = FencV->MposSftR - (80 - MencP->BitnoDpoint.b.h);
//	}
//	else
	{
		FencV->MposSftR = FSX_FPOS_SFT;
//		FencV->MposSftR = 32 - BitNo;
//		FencV->MposSftX = FSX_FPOS_SFT;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	PnE8C   : Serial conversion unit origin offset												*/
	/*																								*/
	/*	OrgDetDelayComp = PnE8C [0.01deg]×2^BITNO / 100 / 360[deg]									*/
	/*----------------------------------------------------------------------------------------------*/
	/*		※CTo be used in phase position calculation,LMX_MPOS_SFTif it was								*/
	/*----------------------------------------------------------------------------------------------*/
	if(FALSE != OrgOffsetEnable)
	{
//		if((FencV->SenSpec & 0xC4) == 0x84)
//		{	/* Bit7=1,Bit6=0 and Bit2(Origin offset enabled)	*/
//			uswk = FencV->SenOrgOffset * 100; /* deg -> 0.01deg Conversion */
//		}
//		else
		{
			uswk = scnvorgoffset;	/* PnE8Cwas originally0.01deg */
		}
		FencV->OrgDetDelayComp = (((uswk << BitNo) / 100) << FencV->MposSftR) / 360;
	}
	else
	{
		FencV->OrgDetDelayComp = 0;
	}

}


/*****************************************************************************************************
 *
 *	Fulk encoder : Variable initialization process
 *
 ****************************************************************************************************/
SENC_ERROR FencInitVariables(MENCV *FencV, ASIC_HW_IF_STR *AsicHwReg,
                             MENCPRM *MencP, USHORT scnvorgoffset, BOOL OrgOffsetEnable,
                             LONG KConvLoadSpd, LONG sfgfail_b_h)
{
	LONG		WorkPos, mposSign;
	SENC_ERROR	err;

	/*----------------------------------------------------------------------------------------------*/
	/*	Reading data from the serial encoder														*/
	/*----------------------------------------------------------------------------------------------*/
	err = SencReadInitialP1Data(FencV);

	/*----------------------------------------------------------------------------------------------*/
	/*	Fulque encoder initial parameter calculation															*/
	/*----------------------------------------------------------------------------------------------*/
	FencV->ComErrLimit = sfgfail_b_h;/*Prm.spgfail.b.h*/
	FencInitPrmCalc(FencV, MencP, scnvorgoffset, OrgOffsetEnable, KConvLoadSpd);

	/*----------------------------------------------------------------------------------------------*/
	/*	Initial position data(This time,Previous)																*/
	/*----------------------------------------------------------------------------------------------*/
	/* When using a position correction module, invert within the position correction moduleMposSignis always assumed to be positive.	*/
	if(FencV->RevAsmMotor != FALSE)
	{
		mposSign = -1;
	}
	else
	{
		mposSign = 1;
	}
	FencV->MotPos = 0; /* Fulke position [32bitRingPulse] */
	FencV->RxPosH[1] = FencV->RxPosH[0];
	FencV->MotPosX[0] = mposSign *
					   ((FencV->RxPosL[0].Long >> FencV->MposSftR) << FencV->MposSftR);
	FencV->MotPosX[1] = FencV->MotPosX[0];

	/*----------------------------------------------------------------------------------------------*/
	/* Initial absolute position data(Absolute position36BitLower part of data32BitExtract the data)							*/
	/*----------------------------------------------------------------------------------------------*/
	if(FencV->AbsoEncoder != FALSE)
	{
		FencV->MotAbsPos = (FencV->RxPosH[0] << (32 - FSX_FPOS_SFT))
								| ((ULONG)FencV->RxPosL[0].dw >> FSX_FPOS_SFT);
		FencV->MotAbsPos = mposSign * FencV->MotAbsPos;
	}
	else
	{
		FencV->MotAbsPos = 0;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		Divided pulse initial settings																		*/
	/*----------------------------------------------------------------------------------------------*/
#if 0/*<S02D>*/
	FencV->DivOutRem = 0;
	WorkPos = (ULONG)FencV->MotPosX[0] >>  FencV->MposSftR;
	FencV->DivOutPos = MlibPfbkxremNolim(WorkPos, FencV->DivOutGain, &FencV->DivOutRem);
	FencV->InitDivPosOffset = FencV->DivOutPos;

	if(FencV->AbsoEncoder == FALSE)
	{	/*------------------------------------------------------------------------------------------*/
		/* C phase flag related: Incremental encoder														*/
		/*------------------------------------------------------------------------------------------*/
		FencV->Cset = FALSE; 								/* C phase settingFlag							*/
		FencV->CposOk = FALSE; 								/* C-phase position setting completedFlag					*/
		if(FencV->DivOut != FALSE)
		{
			/* Pulse conversionCPhase maskONSetting	*/
			AsicHwReg->AREG_WDT1SET |= BIT8;
			FencV->DivCMaskStatus = DIV_CMASK_ON;
			/* Normal C phase output selection				*/
			AsicHwReg->AREG_DIVSET &= 0xf3ff;
			FencV->DivModeStatus = DIV_MODE_NORMAL;

			AsicHwReg->AREG_POSET0 = (USHORT)FencV->DivOutPos;
			AsicHwReg->AREG_POTEST = (USHORT)FencV->DivOutPos;
		}
	}
	else
	{	/*------------------------------------------------------------------------------------------*/
		/* C phase flag related: Absolute encoder															*/
		/*------------------------------------------------------------------------------------------*/
		FencV->Cset = TRUE; 		/* C phase settingFlag	*/
		FencV->CposOk = TRUE; 		/* C-phase position setting completedFlag */
		if(FencV->DivOut != FALSE)
		{
			/* Pulse conversionCPhase maskONSetting	*/
			AsicHwReg->AREG_WDT1SET |= BIT8;
			FencV->DivCMaskStatus = DIV_CMASK_ON;
			/* Normal C phase output selection				*/
			AsicHwReg->AREG_DIVSET &= 0xf3ff;
			FencV->DivModeStatus = DIV_MODE_NORMAL;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/* C phase flag related: C phaseless encoder															*/
	/*----------------------------------------------------------------------------------------------*/
	FencV->MotCphPass = FALSE; 								/* C phase passingFlag							*/
//	FencV->MotCphPassB = FALSE; 							/* C phase passingFlag	for ScanB				*/

	/*----------------------------------------------------------------------------------------------*/
	/* Encoder ready setting : Set after encoder initialization/parameter initialization						 	*/
	/*----------------------------------------------------------------------------------------------*/
	if(FencV->EncConnect == FALSE) 							/* When encoder is not connected					*/
	{
		FencV->SenReady = FALSE; 							/* SENSignal processing completeFlag					*/
		FencV->IncPulseReq = FALSE; 						/* Initial increment pulse output request			*/
	}
	else if(FencV->AbsoEncType != FALSE) 					/* When using absolute encoder					*/
	{
		FencV->SenReady = FALSE; 							/* SENSignal processing completeFlag					*/
		FencV->IncPulseReq = TRUE; 							/* Initial increment pulse output request			*/
	}
	else 													/* When using incremental encoder					*/
	{
		FencV->SenReady = TRUE; 							/* SENSignal processing completeFlag					*/
		FencV->IncPulseReq = FALSE; 						/* Initial increment pulse output request			*/
	}

	/*----------------------------------------------------------------------------------------------*/
	/* Select function presence/absence: Increpulse output request reset													*/
	/*----------------------------------------------------------------------------------------------*/
	FencV->PaoseqNormal = FALSE; 							/* PAOOutputSeq Initial processing settings				*/
//	FencV->IncPulseReq = FALSE; 							/* Initial increment pulse output request			*/
#else/*<S02D>*/
	FencV->DivInfo.DivOutRem = 0;
	WorkPos = (ULONG)FencV->MotPosX[0] >>  FencV->MposSftR;
	FencV->DivInfo.DivOutPos = MlibPfbkxremNolim(WorkPos, FencV->DivInfo.DivOutGain, &FencV->DivInfo.DivOutRem);
	FencV->DivInfo.InitDivPosOffset = FencV->DivInfo.DivOutPos;

	if(FencV->AbsoEncoder == FALSE)
	{	/*------------------------------------------------------------------*/
		/* C phase flag related: Incremental encoder								*/
		/*------------------------------------------------------------------*/
		FencV->Cset = FALSE; 		/* C phase settingFlag							*/
		FencV->CposOk = FALSE; 		/* C-phase position setting completedFlag					*/
		if(FencV->DivInfo.DivOut != FALSE)
		{
			/* Pulse conversionCPhase maskONSetting	*/
			AsicHwReg->AREG_WDT1SET |= BIT8;
			FencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;
			/* Normal C phase output selection				*/
			AsicHwReg->AREG_DIVSET &= 0xf3ff;
			FencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL;
			AsicHwReg->AREG_POSET0 = (ULONG)FencV->DivInfo.DivOutPos;
			AsicHwReg->AREG_POTEST = (ULONG)FencV->DivInfo.DivOutPos;
		}
	}
	else
	{	/*------------------------------------------------------------------*/
		/* C phase flag related: Absolute encoder									*/
		/*------------------------------------------------------------------*/
		FencV->Cset = TRUE; 		/* C phase settingFlag	*/
		FencV->CposOk = TRUE; 		/* C-phase position setting completedFlag */
		if(FencV->DivInfo.DivOut != FALSE)
		{
			/* Pulse conversionCPhase maskONSetting	*/
			AsicHwReg->AREG_WDT1SET |= BIT8;
			FencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;
			/* Normal C phase output selection				*/
			AsicHwReg->AREG_DIVSET &= 0xf3ff;
			FencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL;
		}
	}
	/*----------------------------------------------------------------------*/
	/* C phase flag related: C phaseless encoder									*/
	/*----------------------------------------------------------------------*/
	FencV->MotCphPass = FALSE; 		/* C phase passingFlag							*/

	/*----------------------------------------------------------------------*/
	/* Encoder ready setting : Set after encoder initialization/parameter initialization 	*/
	/*----------------------------------------------------------------------*/
	if(FencV->EncConnect == FALSE) 		/* When encoder is not connected				*/
	{
		FencV->SenReady = FALSE; 		/* SENSignal processing completeFlag				*/
		FencV->DivInfo.IncPulseReq = FALSE; 	/* Initial increment pulse output request		*/
	}
	else if(FencV->AbsoEncType != FALSE) /* When using absolute encoder				*/
	{
		FencV->SenReady = FALSE; 		/* SENSignal processing completeFlag				*/
		FencV->DivInfo.IncPulseReq = TRUE; 		/* Initial increment pulse output request		*/
	}
	else 								/* When using incremental encoder				*/
	{
		FencV->SenReady = TRUE; 		/* SENSignal processing completeFlag				*/
		FencV->DivInfo.IncPulseReq = FALSE; 	/* Initial increment pulse output request		*/
	}

	/*----------------------------------------------------------------------*/
	/* Select function presence/absence: Increpulse output request reset							*/
	/*----------------------------------------------------------------------*/
	FencV->PaoseqNormal = FALSE; 		/* PAOOutputSeq Initial processing settings			*/

	/*----------------------------------------------------------------------*/
	/* Initial incremental output request set									<S02D>	*/
	/* Even in increment modeTRUE,ScanAof1At the [7]th processingFALSE	*/
	/*----------------------------------------------------------------------*/
#if 0
	if( SVFSEL_INCPOUT == 0 )
	{
		FencV->DivInfo.IncPulseReq = FALSE;
	}
	else
	{
		if( FencP->DivOut == TRUE) /* && (Kprm.f.MotorLessTestMode==FALSE) )*/
		{
			FencV->DivInfo.IncPulseReq = TRUE;
		}
	}
#endif/*<S02D>*/
#endif
	/*----------------------------------------------------------------------------------------------*/
	/* Initial value setting of rotation speed data																	*/
	/*----------------------------------------------------------------------------------------------*/
	FencV->MotSpd = 0; 										/* Motor Speed [2^24/OvrSpd]			*/
	FencV->SpdCmpEnable = FALSE;							/* Speed correction processing enabledFlag					*/

	return err;
}





#if 0
/*****************************************************************************************************
 *
 *	Fulk encoder : Base timer monitoring
 *
 ****************************************************************************************************/
void LpxCheckFencBaseTimer(MENCV *FencV)
{
	USHORT BaseTimer;

	BaseTimer = (USHORT) (FencV->RxTimex.dw >> 8);
	if(FencV->ChkAlarmCnt >= 2)		/* Check after 3rd time */
	{
		if(BaseTimer == FencV->ChkBaseTimer)				/* Timebase timer no change? 			*/
		{
			KPI_ALARMSET(ALM_FPGCOMTIMER); 					/* A.CF2 : FPG communication timer error			*/
		}
	}
	FencV->ChkBaseTimer = BaseTimer; /* Timebase timer previous value update		*/

}
#endif

/*****************************************************************************************************
 *
 *	Fulk encoder : Alarm monitoring
 *
 ****************************************************************************************************/
UCHAR FencGetFencAlarm(MENCV *FencV)
{
	UCHAR AlmCode;

	if( (FencV->MstErrCnt == 0) &&
		((FencV->EncConnect != FALSE) && (FencV->EncDisable == FALSE)) )
	{
//		LpxCheckFencBaseTimer(FencV); 						/* Base timer check				*/
//		LpxCheckFencAlarmCode(FencV); 						/* Alarm code check				*/

		if(FencV->ChkAlarmCnt >= 2) /* Check after 3rd time */
		{
			AlmCode = (UCHAR)(FencV->RxAlmCode & 0x003B);
			MlibBITFILTER1( FencV->ChkAlmCode[0], FencV->ChkAlmCode[1], AlmCode );
		}
		else
		{
			FencV->ChkAlmCode[0] = 0;
			FencV->ChkAlmCode[1] = 0;
		}

		if(FencV->ChkAlarmCnt < 10)
		{
			FencV->ChkAlarmCnt++;
		}
	}
	else
	{
		FencV->ChkAlarmCnt = 0;
	}

	return ((UCHAR)FencV->ChkAlmCode[0]);
}

#endif/* CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT*/
/*** end of file ***/

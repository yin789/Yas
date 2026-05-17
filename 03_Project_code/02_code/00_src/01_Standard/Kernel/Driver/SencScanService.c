/****************************************************************************
Description	: Serial Encoder Service for Scan task
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes	      :
	Name   Date          Description
	------------------------------------------------------------------------
	I.M.   2011.02.08    created

****************************************************************************/
#include "Basedef.h"
#include "SerialEncIf.h"
#include "MLib.h"
#include "KLib.h"
#include "SencScanService.h"	/* <S004> */
#include "JL086ServoIpHw.h"	/* <S004> */
#include "AlarmManager.h"		/* <S018> */
#include "MercuryGlobals.h"	/* <S1B7> */
#include "MotorLessTest.h"	/* <S1B7> */

#define _TIME_BASE_TIMER_//Interim/* <S004> */

#ifdef _TIME_BASE_TIMER_
LONG	LpxTimeBasesMotPosCalc( MENCV *MencV );
#endif /* _TIME_BASE_TIMER_ */


/* Fully closed encoder position data reception processing(HW:JL-086)	*//* <S04D> */
//static ALMID_T	FencRcvPosHwProcess( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg );
static void	FencRcvPosHwProcess( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg, ALMMAN_HANDLE *AlmManager);/* <S0E0>:AlmManageraddition */
/* Fully closed encoder position data reception processing(SW)			*//* <S04D> */
static LONG	FencRcvPosSwProcess( MENCV *FencV, ALMMAN_HANDLE *AlmManager, BOOL BeComplete ); /* <S0E0>:AlmManageraddition */

/* Rotary type: Serial encoder position data reception processing(HW:JL-086) *//* <S004> */ /* <S0C5> */
static void	SencRcvPosHwProcess( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg,
															ALMMAN_HANDLE 	*AlmManager );
/* Rotary type: Serial encoder position data reception processing(SW) *//* <S004> */ /* <S0C5> */
static void	SencRcvPosSwProcess( MENCV *MencV, ALMMAN_HANDLE 	*AlmManager,
																	BOOL BeComplete );

/* Linear motor: Motor current phase update process								*/
static void	LmxUpdateMotPhase( MENCV *MencV, ALMMAN_HANDLE *AlmManager, LONG dMotPos );

/* <S1B7> >>>>> */
/* Motorless: Multiturn data acquisition processing	*/
static USHORT LpxGetSencMotLessMturnData( MENCV *MencV, MENCPRM *MencP, USHORT PosH, ULONG PosL );
/* Motor-less: Multi-turn data setting processing	*/
static void LpxSencMotLessMultiTurnSet(MENCPRM *MencP, ENCODER_DATA *EncData, SHORT MturnLimit);
static void MlibAdd6432( LONG *a, LONG b, LONG *x );	/* Provisional measures(FunctionINT64) */
/* <<<<< <S1B7> */
/****************************************************************************************************/
/*																									*/
/*		Linear motor: Encoder C phase passage check processing												*/
/*																									*/
/****************************************************************************************************//* <S029> */
void	LmxCheckSencCphasePass( MENCV *MencV )
{
//LONG	lwk;	/*<S0C7>*/
//LONG	remx;	/*<S0C7>*/
LONG	CposX;
LONG	CposChange;
/* <S21B>:Start */
LONG	lwk;
LONG	remx;
SENCX		*Enc;
Enc	= &MencV->EncIf;
/* <S21B>:end */

	CposX = 0;				/*<S0C4>*/
	CposChange = FALSE;
/*--------------------------------------------------------------------------------------------------*/
/*		Origin passing check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( MencV->CphaseLess == FALSE )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	C phase passage check																		*/
		/*------------------------------------------------------------------------------------------*/
			if( (MencV->RxAlmCode & 0x0035) == 0x0001 )/* Fixed bit+Overheat check	*/
			{
				if( (MencV->RxPosH[0] != MencV->RxPosH[1]) ||
					( (MencV->Cset==FALSE) && ((MencV->RxAlmCode & 0x0040)==0) ) )
				{
					CposChange = TRUE;
					MencV->Cset = TRUE;
					CposX = (LONG)(MencV->RxPosH[0] << HPOS_TO_XPOS);
					CposX = MencV->MposSign *(CposX + MencV->OrgDetDelayComp);
				}
			}
		/*------------------------------------------------------------------------------------------*/
		/*	C-phase position setting processing																		*/
		/*------------------------------------------------------------------------------------------*/
			if( MencV->Cset )
			{
			/*--------------------------------------------------------------------------------------*/
			/*	C phase position latch processing																	*/
			/*--------------------------------------------------------------------------------------*/
				if( CposChange )
				{
					MencV->IncreCposLatch = TRUE;				/* C-phase position latch completed   --> ScanB	*/
					CposX = ((LONG)(MencV->MotPosX[0] - CposX) >> LMX_MPOS_SFT);	/* Increment correction completedCPhase position */
					MencV->MotCposX = MencV->MotPos - CposX;
				}
#if 0
			/*--------------------------------------------------------------------------------------*/
			/*	Pulse conversion origin correction setting process															*/
			/*--------------------------------------------------------------------------------------*/
				if( MencP.f.DivOut == TRUE )
				{
					if( CposChange )
					{
						remx = 0;
						lwk = MlibPfbkxremNolim( MencV->MotCposX, MencV->DivOutGain, &remx );
						lwk += MencV->InitDivPosOffset ;											
						ASIC_WR( AREG_POSET1, (USHORT)(lwk - 1) );
						ASIC_WR( AREG_POSET2, (USHORT)(lwk + 0) );
#if	SVFSEL_MICROPROGRAM_DIVOUT == 1
						MencV->DivOutOrgX1 = lwk - 1;				/* Pulse conversion origin correction 1(For holding)	*/
						MencV->DivOutOrgX2 = lwk + 0;				/* Pulse conversion origin correction 2(For holding)	*/
#endif
					}
				/*----------------------------------------------------------------------------------*/
					lwk = MencV->MotPos - MencV->MotCposX;
					if( (lwk < MencP.DivOutCmskChk) && (lwk > -MencV->DivOutCmskChk) )
					{
						KPI_PCMASK_OFF( );					/* Pulse conversionCPhase maskOFFSetting			*/
//						KPI_PCMASK_OFF( RoutV.OutModeFlg );	/* Pulse conversionCPhase maskOFFSetting			*/
					}
					else
					{
						KPI_PCMASK_ON( );					/* Pulse conversionCPhase maskONSetting			*/
					}
				}
#endif
/* <S21B>:Start */
			/*--------------------------------------------------------------------------------------*/
			/*	Pulse conversion origin correction setting process															*/
			/*--------------------------------------------------------------------------------------*/
				if(MencV->DivInfo.DivOut != FALSE)
				{
					if(CposChange != FALSE)
					{
						remx = 0;
						lwk = MlibPfbkxremNolim(MencV->MotCposX, MencV->DivInfo.DivOutGain, &remx);
						lwk += MencV->DivInfo.InitDivPosOffset;
						/* Pulse conversion origin correction1 */
						SHalDrv_SetOriginCorrect1( MencV->EncIf.SencSetReg, (lwk - 1) );
						/* Pulse conversion origin correction2 */
						SHalDrv_SetOriginCorrect2( MencV->EncIf.SencSetReg, lwk );
#if	SVFSEL_MICROPROGRAM_DIVOUT == 1
						MencV->DivInfo.DivOutOrgX1 = lwk - 1;				/* Pulse conversion origin correction 1(For holding)	*/
						MencV->DivInfo.DivOutOrgX2 = lwk + 0;				/* Pulse conversion origin correction 2(For holding)	*/
#endif
					}
					lwk = MencV->MposFromCpos;
					if((lwk < MencV->DivInfo.DivOutCmskChk) && (lwk > -MencV->DivInfo.DivOutCmskChk))
					{	/* Pulse conversionCPhase maskOFFSetting			*/
						Enc->SencSetReg->AREG_WDT1SET &= ~BIT8;
						MencV->DivInfo.DivCMaskStatus = DIV_CMASK_OFF;
					}
					else
					{	/* Pulse conversionCPhase maskONSetting			*/
						Enc->SencSetReg->AREG_WDT1SET |= BIT8;
						MencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;
					}
				}
/* <S21B>:end */
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase less encoder(For absolute scale): Perform only C phase mask ON/OFF processing				*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
#if 0
			/*--------------------------------------------------------------------------------------*/
			/*		Absolute position is the origin(=0)Release the C-phase mask in the vicinity									*/
			/*--------------------------------------------------------------------------------------*/
			if( (MencV->MotAbsPos < MencV->DivOutCmskChk) && (MotAbsPos > -MencV->DivOutCmskChk) )
			{
//				KPI_PCMASK_OFF( RoutV.OutModeFlg );			/* Pulse conversionCPhase maskOFFSetting			*/
				KPI_PCMASK_OFF( );							/* Pulse conversionCPhase maskOFFSetting			*/
			}
			else
			{
				KPI_PCMASK_ON( );							/* Pulse conversionCPhase maskONSetting			*/
			}
#endif
/* <S21B>:Start */
			/*--------------------------------------------------------------------------------------*/
			/*		Absolute position is the origin(=0)Release the C-phase mask in the vicinity									*/
			/*--------------------------------------------------------------------------------------*/
			if( (MencV->MotAbsPos < MencV->DivInfo.DivOutCmskChk) && (MencV->MotAbsPos > -MencV->DivInfo.DivOutCmskChk) )
			{
				Enc->SencSetReg->AREG_WDT1SET &= ~BIT8;     /* Pulse conversionCPhase maskOFFSetting			*/
			}
			else
			{
				Enc->SencSetReg->AREG_WDT1SET |= BIT8;      /* Pulse conversionCPhase maskONSetting			*/
			}
/* <S21B>:end */
		}
/*--------------------------------------------------------------------------------------------------*/
		return;

}
/****************************************************************************
*
*		Linear motor: Scan B service processing(Called from ScanB)
*
****************************************************************************/
BOOL LmxSencScanServiceB( MENCV *MencV, BOOL *LatchClrReq )
{
	LONG	lwk1;
	LONG	LastMposFromCpos;

	if( MencV->CphaseLess != FALSE )
	{	/*------------------------------------------------------------------*/
		/*	AbsoEncoder : Reset Cphase Pass Signal							*/
		/*------------------------------------------------------------------*/
		MencV->CposOk = TRUE;
		LastMposFromCpos = MencV->MposFromCpos;
		MencV->MposFromCpos = MencV->MotAbsPos;
		lwk1 = LastMposFromCpos ^ MencV->MposFromCpos;
		MencV->MotCphPass = ( lwk1 < 0 ) ? TRUE : FALSE;
	}
	else
	{	/*------------------------------------------------------------------*/
		/*	IncreEncoder : Make MposFromCpos and Motor Cphase Pass Signal	*/
		/*------------------------------------------------------------------*/
		if( MencV->Cset == FALSE )
		{
			MencV->CposOk = FALSE;
			MencV->MotCphPass = FALSE;
		}
		else if( MencV->IncreCposLatch != FALSE )
		{
			MencV->CposOk = TRUE;
			MencV->IncreCposLatch = FALSE;/* <S029> : Cancel commenting out */
			*LatchClrReq = TRUE;
			MencV->MotCphPass = TRUE;

			/* <S029> : MDSpecification toSigma7Change to specifications */
			MencV->MposFromCpos = MencV->MotPos - MencV->MotCposX;
//			MencV->MposFromCpos = MencV->CphaseDist;
		}
		else
		{
			MencV->CposOk = TRUE;
			LastMposFromCpos = MencV->MposFromCpos;

			/* <S029> MDSpecification toSigma7Change to specifications */
			MencV->MposFromCpos = MencV->MotPos - MencV->MotCposX;
//			MencV->MposFromCpos = MencV->CphaseDist;
			lwk1 = LastMposFromCpos ^ MencV->MposFromCpos;
			MencV->MotCphPass = ( lwk1 < 0 ) ? TRUE : FALSE;
		}
	}

	return MencV->MotCphPass;
}


/****************************************************************************
*
*		Rotary type: Scan B service processing(Called from ScanB)
*
****************************************************************************/
BOOL RmxSencScanServiceB( MENCV *MencV, BOOL *LatchClrReq )
{
	LONG	lwk1;
	LONG	LastMotAngle;

	/*----------------------------------------------------------------------*/
	/*		Latch Encoder Data for ScanB									*/
	/*----------------------------------------------------------------------*/
//	MencV->ScanBPosH = MencV->RxPosH[0];			/* Use the data read in scan A.	*/
//	MencV->ScanBPosL = MencV->RxPosL[0].dw;

	/*----------------------------------------------------------------------*
	* Calculate Motor Angle, Motor Position from C-Phase, C-Phase Pass Signal
	*-----------------------------------------------------------------------*/
	if( (MencV->Cset != FALSE) && (MencV->CphaseLess == FALSE) )
	{
		/*------------------------------------------------------------------*/
		/*	MencV->MotAngle [pulse] : 0 ~ (PulseNo - 1)					*/
		/*------------------------------------------------------------------*/
		LastMotAngle = MencV->MotAngle;
		/* 2011.01.07 Y.Oka μProgramI/Ffor simplification,CphaseDist */
#if 1/* <S00C> */
		lwk1 = (LONG)((LONG)((MencV->MotPosX[0]) - (MencV->MotCposX)) >> MencV->MposSftR); 
#else
		lwk1 = (LONG)( MencV->CphaseDist >> MencV->MposSftR ); /* TODO: CphaseDistNot created, soUn003but0has become!! */
#endif
		MencV->MotAngle = ( lwk1 >= 0 ) ? lwk1 : lwk1 + MencV->PulseNo;

		/*------------------------------------------------------------------*/
		/*	MencV->MposFromCpos [pulse] : (-PulseNo/2) ~ (PulseNo/2 - 1)	*/
		/*------------------------------------------------------------------*/
		lwk1 = ( MencV->MotAngle < MencV->HalfPulseNo ) ?
			       MencV->MotAngle : MencV->MotAngle - MencV->PulseNo;

		MencV->MposFromCpos = lwk1;
		/*------------------------------------------------------------------*/
		/*	Make Motor Cphase Pass Signal									*/
		/*------------------------------------------------------------------*/
		/* 2010.08.25 Y.Oka Fixed latch problem when connecting incremental encoder */
		if( MencV->IncreCposLatch != FALSE )
		{ /* First-time CPhase Pass (Incremental Encoder) */
			MencV->CposOk = TRUE;
			MencV->IncreCposLatch = FALSE;/* <S029> : Cancel commenting out */
			*LatchClrReq = TRUE;
			MencV->MotCphPass = TRUE;
		}
		else
		{	/* Already CPhase Pass
				(In case of Absolute Encoder, IncreCposLatch is not TRUE) */
			MencV->CposOk = TRUE;
			lwk1 = MlibLABS( MencV->MotAngle - LastMotAngle );
			MencV->MotCphPass = ( lwk1 > MencV->HalfPulseNo ) ? TRUE : FALSE;
		}
	}
	else
	{
		MencV->CposOk = FALSE;
		MencV->MotCphPass = FALSE;
	}

	return MencV->MotCphPass;
}




/****************************************************************************
*
*		Fulk encoder : C phase passage check process
*
****************************************************************************/
static void FencCheckCphasePass(MENCV *FencV, ASIC_HW_IF_STR *AsicHwReg)
{
	LONG lwk1;
	LONG remx;
	LONG CposX;
	LONG CposChange;

	CposX = 0;
	if(FencV->CphaseLess == FALSE)
	{
		CposChange = FALSE;
		/*------------------------------------------------------------------*/
		/* C phase passage check */
		/*------------------------------------------------------------------*/
		if((FencV->RxAlmCode & 0x0035) == 0x0001)
		{
			if( (FencV->RxPosH[0] != FencV->RxPosH[1]) ||
				((FencV->Cset == FALSE) && ((FencV->RxAlmCode & 0x0040) == 0)) )
			{
				CposChange = TRUE;
				FencV->Cset = TRUE;
				CposX = FencV->MposSign *
						(FSX_HposToXpos(FencV->RxPosH[0]) + FencV->OrgDetDelayComp);
			}
		}
		/*------------------------------------------------------------------*/
		/*	C-phase position setting processing												*/
		/*------------------------------------------------------------------*/
		if(FencV->Cset != FALSE)
		{
			/*	C-phase position latch processing & Fulk position calculation	*/
			FencV->CposOk = TRUE;

			if(CposChange)
			{
				FencV->MposFromCpos = (FencV->MotPosX[0] - CposX) >> FencV->MposSftR;
				FencV->MotCposX = FencV->MotPos - FencV->MposFromCpos;
				FencV->MotCphPass = TRUE;
			}
			else
			{
				lwk1 = FencV->MposFromCpos;
				FencV->MposFromCpos = FencV->MotPos - FencV->MotCposX;
				FencV->MotCphPass = ((lwk1 ^ FencV->MposFromCpos) < 0) ? TRUE : FALSE;
			}

			/*	Pulse conversion origin correction setting process */
#if 0 /*<S02D>*/
			if(FencV->DivOut != FALSE)
			{
				if(CposChange != FALSE)
				{
					remx = 0;
					lwk1 = MlibPfbkxremNolim(FencV->MotCposX, FencV->DivOutGain, &remx);
					lwk1 += FencV->InitDivPosOffset;
					AsicHwReg->AREG_POSET1 = (USHORT)(lwk1 - 1);
					AsicHwReg->AREG_POSET2 = (USHORT)(lwk1 + 0);
				}

				lwk1 = FencV->MposFromCpos;
				if((lwk1 < FencV->DivOutCmskChk) && (lwk1 > -FencV->DivOutCmskChk))
				{	/* Pulse conversionCPhase maskOFFSetting			*/
					AsicHwReg->AREG_WDT1SET &= ~BIT8;
					FencV->DivCMaskStatus = DIV_CMASK_OFF;
				}
				else
				{	/* Pulse conversionCPhase maskONSetting			*/
					AsicHwReg->AREG_WDT1SET |= BIT8;
					FencV->DivCMaskStatus = DIV_CMASK_ON;
				}
			}
#else/*<S02D>*/
			if(FencV->DivInfo.DivOut != FALSE)
			{
				if(CposChange != FALSE)
				{
					remx = 0;
					lwk1 = MlibPfbkxremNolim(FencV->MotCposX, FencV->DivInfo.DivOutGain, &remx);
					lwk1 += FencV->DivInfo.InitDivPosOffset;
#if 0/* <S21B>:Start */
					AsicHwReg->AREG_POSET1 = (USHORT)(lwk1 - 1);
					AsicHwReg->AREG_POSET2 = (USHORT)(lwk1 + 0);
#else
					/* Pulse conversion origin correction1 */
					SHalDrv_SetOriginCorrect1( FencV->EncIf.SencSetReg, (lwk1 - 1) );
					/* Pulse conversion origin correction2 */
					SHalDrv_SetOriginCorrect2( FencV->EncIf.SencSetReg, lwk1 );
#endif/* <S21B>:end */
				}

				lwk1 = FencV->MposFromCpos;
				if((lwk1 < FencV->DivInfo.DivOutCmskChk) && (lwk1 > -FencV->DivInfo.DivOutCmskChk))
				{	/* Pulse conversionCPhase maskOFFSetting			*/
//					AsicHwReg->AREG_WDT1SET &= ~BIT8;
					FencV->EncIf.SencSetReg->AREG_WDT1SET &= ~BIT8; /* <S21B> */
					FencV->DivInfo.DivCMaskStatus = DIV_CMASK_OFF;
				}
				else
				{	/* Pulse conversionCPhase maskONSetting			*/
//					AsicHwReg->AREG_WDT1SET |= BIT8;
					FencV->EncIf.SencSetReg->AREG_WDT1SET |= BIT8; /* <S21B> */
					FencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;
				}
			}
#endif/*<S02D>*/
		}

	}
	else
	{
		/*------------------------------------------------------------------*/
		/*		C phase less encoder(For absolute scale)					*/
		/*------------------------------------------------------------------*/
#if 0 /*<S02D>*/
		if(FencV->DivOut != FALSE)
		{
			/*	Absolute position is the origin(=0)Release the C-phase mask in the vicinity */
			if( (FencV->MotAbsPos < FencV->DivOutCmskChk) &&
				(FencV->MotAbsPos > -FencV->DivOutCmskChk) )
			{	/* Pulse conversionCPhase maskOFFSetting			*/
				AsicHwReg->AREG_WDT1SET &= ~BIT8;
				FencV->DivCMaskStatus = DIV_CMASK_OFF;
			}
			else
			{	/* Pulse conversionCPhase maskONSetting			*/
				AsicHwReg->AREG_WDT1SET |= BIT8;
				FencV->DivCMaskStatus = DIV_CMASK_ON;
			}
		}
#else/*<S02D>*/
		if(FencV->DivInfo.DivOut != FALSE)
		{
			/*	Absolute position is the origin(=0)Release the C-phase mask in the vicinity */
			if( (FencV->MotAbsPos < FencV->DivInfo.DivOutCmskChk) &&
				(FencV->MotAbsPos > -FencV->DivInfo.DivOutCmskChk) )
			{	/* Pulse conversionCPhase maskOFFSetting			*/
//				AsicHwReg->AREG_WDT1SET &= ~BIT8;
				FencV->EncIf.SencSetReg->AREG_WDT1SET &= ~BIT8; /* <S21B> */
				FencV->DivInfo.DivCMaskStatus = DIV_CMASK_OFF;
			}
			else
			{	/* Pulse conversionCPhase maskONSetting			*/
//				AsicHwReg->AREG_WDT1SET |= BIT8;
				FencV->EncIf.SencSetReg->AREG_WDT1SET |= BIT8; /* <S21B> */
				FencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;
			}
		}
#endif/*<S02D>*/
		/*	Origin passing check */
		lwk1 = FencV->MposFromCpos;
		FencV->MposFromCpos = FencV->MotAbsPos;
		FencV->MotCphPass = ((lwk1 ^ FencV->MposFromCpos) < 0) ? TRUE : FALSE;
	}

}


#if 0
/****************************************************************************
*
*	Fulk encoder : Speed calculation using timebase timer
*
****************************************************************************/
LONG LpxTimeBasesFencSpdCalc(ENCVFBTIMBASE FbTimeBase, LONG dPos, LONG dPosSpd, UCHAR EnableFlag)
{
	LONG lwk;

	if(EnableFlag == FALSE)
	{
		/*------------------------------------------------------------------*/
		/*	Correction invalid processing													*/
		/*------------------------------------------------------------------*/
		FbTimeBase->Modflag = FALSE; 		/* No correction mode setting	*/
		FbTimeBase->fencspeed = dPosSpd; 	/* Output without correction	*/
	}
	else
	{
		/*------------------------------------------------------------------*/
		/*	Processing when correction is valid													*/
		/*------------------------------------------------------------------*/
		/*	Speed calculation execution determination(Less than 2 pulses is normal differential speed) */
		FbTimeBase->Modflag = (MlibABS(dPos) < VFBTINMINPLS) ? FALSE : TRUE;

		/*	Speed calculation execution */
		/*------------------------------------------------------------------*/
		if(FbTimeBase->BaseTimeInterval != 0)
		{
			/* No base timer change time -> Encoder error	*/
			/* Speed calculation using base timer */
			lwk = (dPosSpd >> 2) * SAENCCLKNUM;
			lwk = lwk / (LONG) FbTimeBase->BaseTimeInterval;
			FbTimeBase->fencspeed = lwk << 2;
		}
		else
		{
			/* If correction condition is not reached: Use encoder position difference velocity calculation */
			FbTimeBase->fencspeed = dPosSpd;
		}
	}

	/*	Save current value	*/
	FbTimeBase->LastBaseTimer = FbTimeBase->BaseTimer;
	FbTimeBase->LastModflag = FbTimeBase->Modflag;

	return (FbTimeBase->fencspeed);
}
#endif

/****************************************************************************
*
*		Fulk encoder : Communication error processing
*
****************************************************************************/
static LONG FencComErrorProc(MENCV *FencV, ALMMAN_HANDLE *AlmManager)/* <S0E0>:AlmManageraddition */
{
	LONG FencPosX;
	LONG lwk;

	FencV->SeRxErr = TRUE;	/* Set Error Flag */
	++FencV->ComErrCnt; 	/* Count up ComErrCnt for Monitor	*/

	FencPosX = FencV->MotPosX[0];
	if(FencV->EncDataNG == FALSE)
	{
		if(FencV->MstErrCnt >= FencV->ComErrLimit)
		{
			FencV->EncDataNG = TRUE; 							/* Encoder data invalid	*/
			/* <S0E0>:Alarm processing */
			ALMSetServoAlarm( AlmManager, ALM_FPGCOM );		/* A.CF1:FPG communication error			*/
			ALMSetMask( AlmManager, ALM_FPGCOMTIMER, TRUE );	/* A.CF2:FPG communication timer error	*/
		}
		else
		{
			++FencV->MstErrCnt;

			lwk = FencPosX - FencV->MotPosX[1];
			FencPosX += lwk;
		}
	}
	return FencPosX;
}

/***************************************************************************
*
*		Fulk encoder : Position data reading
*
****************************************************************************//* <S04D> */
LONG FencReadPosition(MENCV *FencV, ALMMAN_HANDLE *AlmManager, BOOL BeComplete,	/* <S0E0>:AlmManageraddition */
						void *BaseLoops)	/*<S1B7>*/
{
	LONG	dFencPos, FencPosX;
	LONG	FencSpd;
//	ALMID_T	err_code;	/*<S1B7> Unused*/

	ASIC_HW_IF_STR *sencSetReg;
	ASIC_HW_IF_STR *posCmpSetReg;

	sencSetReg = FencV->EncIf.SencSetReg;
	posCmpSetReg = FencV->EncIf.PosCmpSetReg;
	/*----------------------------------------------------------------------*/
	/*	Unconnected state															*/
	/*----------------------------------------------------------------------*/
	if((FencV->EncConnect == FALSE) || (FencV->EncDisable != FALSE))
	{
		return (0);
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Encoder position correction processing using change time timer													*/
/*--------------------------------------------------------------------------------------------------*/
	if( FencV->PosCompMode == POSCOMP_SW )/*<S0C5>rename*/
	{	/*------------------------------------------------------------------------------------------*/
		/*		SWEncoder position correction(Σ-ⅤCompatibility)													*/
		/*------------------------------------------------------------------------------------------*/
		FencPosX = FencRcvPosSwProcess( FencV, AlmManager, BeComplete );	/* <S0E0>:AlmManageraddition */
	}
	else
	{
//		err_code = FencRcvPosHwProcess( FencV, posCmpSetReg );/*<S0C5>rename*/
		FencRcvPosHwProcess( FencV, posCmpSetReg, AlmManager );/*<S0C5>rename*//* <S0E0>:AlmManageraddition */
		/*------------------------------------------------------------------------------------------*/
		/*		JL-086Encoder position correction														*/
		/*------------------------------------------------------------------------------------------*/
		if( FencV->PosCompMode == POSCOMP_ASIC )
		{
			FencPosX = ( SHal_GetCurMotPos( posCmpSetReg ) << FencV->MposSftR );
		}
		/*------------------------------------------------------------------------------------------*/
		/*		No encoder position correction																*/
		/*------------------------------------------------------------------------------------------*/
		else
		{
			FencPosX = ( SHal_GetCurRcvPos( posCmpSetReg ) << FencV->MposSftR );
		}
	}
	/*----------------------------------------------------------------------*/
	/*	Position difference calculation														*/
	/*----------------------------------------------------------------------*/
	dFencPos = (FencPosX - FencV->MotPosX[0]) >> FencV->MposSftR;
	FencV->ABphCntr = (UCHAR)(FencPosX >> FencV->MposSftR);

	FencV->MotPosX[1] = FencV->MotPosX[0];
	FencV->MotPosX[0] = FencPosX;
	/*----------------------------------------------------------------------*/
	/*	ExternalPGSpeed calculation														*/
	/*----------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	FencSpd = dFencPos * FencV->KConvLoadSpd;
#else
	FencSpd = MlibMulgain(dFencPos, FencV->KConvLoadSpd);
#endif /* FLOAT_USE */
#if 0	/* ★Speed correction using change time timer is currently not supported★ */
	FencV->MotSpd = LpxTimeBasesFencSpdCalc(dFencPos, FencSpd, FencV->SpdCmpEnable);
	FencV->SpdCmpEnable = TRUE;
#else
	FencV->MotSpd = FencSpd;
#endif
	/*----------------------------------------------------------------------*/
	/*	Position calculation [32bitRingPulse]											*/
	/*----------------------------------------------------------------------*/
	FencV->MotPos += dFencPos;
	FencV->MotAbsPos += dFencPos;

	/*----------------------------------------------------------------------*/
	/*	Initial increment pulse output processing or C phase passage check process					*/
	/*----------------------------------------------------------------------*/
#if 0
	if(FencV->IncPulseReq != FALSE)
	{
		/*--------------------------------------------------------------------
		* Countermeasures for frequency-divided pulse output when the motor is moving during initialization at power-on
		*	ScanAProcessing1Set the counter preset and pulse conversion position.
		*-------------------------------------------------------------------*/
		if(FencV->AbsoEncType == MENC_INCRE_TYPE)
		{
			/* C phase passage check process */
			if(FencV->SeRxErr == FALSE)
			{
				FencCheckCphasePass();
			}
			/*	Division pulse output processing	(Set counter preset just before)*/
			if(FencV->DivOut == TRUE)
			{
				FencV->DivOutPos += MlibPfbkxremNolim(dFencPos, FencV->DivOutGain, &FencV->DivOutRem);
				sencSetReg->AREG_POTEST = (USHORT)FencV->DivOutPos;
				sencSetReg->AREG_POSET0 = (USHORT)FencV->DivOutPos;
			}
			FencV->IncPulseReq = FALSE;
		}
		else
		{
			LpxOutFencIniIncPulse(); /* Initial increment pulse output processing	*/
		}
	}
	else
//#endif /*<S02D>Delete*/
	{
		/* C phase passage check process */
		if(FencV->SeRxErr == FALSE)
		{
			FencCheckCphasePass(FencV, sencSetReg);
		}
		/* Divided pulse output*/
		if(FencV->DivOut != FALSE)
		{
			FencV->DivOutPos += MlibPfbkxremNolim(dFencPos, FencV->DivOutGain, &FencV->DivOutRem);
			sencSetReg->AREG_POSET0 = (USHORT) FencV->DivOutPos;
		}
	}
#else/*<S02D>*/
	if(FencV->DivInfo.IncPulseReq != FALSE)
	{
		/*----------------------------------------------------------------------*/
		/* Countermeasures for frequency-divided pulse output when the motor is moving during initialization at power-on   */
		/*	ScanAProcessing1Set the counter preset and pulse conversion position.*/
		/*----------------------------------------------------------------------*/
		if(FencV->AbsoEncType == MENC_INCRE_TYPE)
		{
			/* C phase passage check process */
			if(FencV->SeRxErr == FALSE)
			{
				FencCheckCphasePass(FencV, sencSetReg);
			}
			/*	Division pulse output processing	(Set counter preset just before)*/
			if(FencV->DivInfo.DivOut == TRUE)
			{
				FencV->DivInfo.DivOutPos += MlibPfbkxremNolim(dFencPos, FencV->DivInfo.DivOutGain, &FencV->DivInfo.DivOutRem);
				sencSetReg->AREG_POTEST = (USHORT)FencV->DivInfo.DivOutPos;
				sencSetReg->AREG_POSET0 = (USHORT)FencV->DivInfo.DivOutPos;
			}
			FencV->DivInfo.IncPulseReq = FALSE;
		}
		else
		{
			/*----------------------------------------------------------------------*/
			/*	Initial increment pulse output processing	  									    */
			/*----------------------------------------------------------------------*/
			FencSencOutIniIncPulse( FencV, sencSetReg );
		}
	}
	else
	{
		/* C phase passage check process */
		if(FencV->SeRxErr == FALSE)
		{
			FencCheckCphasePass(FencV, sencSetReg);
		}
		/* Divided pulse output*/
		if( (FencV->DivInfo.DivOut != FALSE) && (FencV->DivInfo.IncPulseReq == FALSE))
		{
			FencV->DivInfo.DivOutPos += MlibPfbkxremNolim(dFencPos, FencV->DivInfo.DivOutGain, &FencV->DivInfo.DivOutRem);
			sencSetReg->AREG_POSET0 = (USHORT) FencV->DivInfo.DivOutPos;
		}
	}

#endif/*<S02D>*/
	return (dFencPos);
}


/****************************************************************************************************/
/*																									*/
/*		Fully closed encoder position data reception processing(HW:JL-086)										*/
/*																									*/
/****************************************************************************************************/
//static ALMID_T	FencRcvPosHwProcess( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg)/* <S0E0> */
static void	FencRcvPosHwProcess( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg, ALMMAN_HANDLE *AlmManager)/* <S0E0> */
{
//	ALMID_T		err_code;/* <S0E0> */

//	err_code = NO_ALARM;									/* Local variable initialization					*//* <S04D> */
	/*----------------------------------------------------------------------------------------------*/
	/*		Processing when an alarm occurs																		*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->SeRxErr   = SHal_GetSeRxErr( pAsicHwReg );		/* Update reception error flag 				*/
	MencV->EncDataNG = SHal_GetEncDataNG( pAsicHwReg );		/* Update encoder data error flag 	*/
	/*----------------------------------------------------------------------------------------------*/
	if( SHal_GetComAlarm( pAsicHwReg ) )					/* Error type determination(Communication/Acceleration)			*/
	{
//		err_code = ALM_FPGCOM;								/* A.CF1 : Encoder communication error			*//* <S0E0> */
		ALMSetServoAlarm( AlmManager, ALM_FPGCOM );			/* A.CF1:FPG communication error					*/
		ALMSetMask( AlmManager, ALM_FPGCOMTIMER, TRUE );	/* A.CF2:FPG communication timer error			*/
	}
	if( SHal_GetAccAlarm( pAsicHwReg ) )					/* Error type determination(Communication/Acceleration)			*/
	{
//		err_code = ALM_FPGCOM;								/* A.CF1 : Encoder communication error(Acceleration)	*//* <S0E0> */
		ALMSetServoAlarm( AlmManager, ALM_FPGCOM );			/* A.CF1:FPG communication error					*/
		ALMSetMask( AlmManager, ALM_FPGCOMTIMER, TRUE );	/* A.CF2:FPG communication timer error			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Update position data without correction																	*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->RcvPosX[2] = MencV->RcvPosX[1];			/*<S0C5>*/
	MencV->RcvPosX[1] = MencV->RcvPosX[0];			/*<S0C5>*/
	MencV->RcvPosX[0] = (LONG)( SHal_GetCurRcvPos( pAsicHwReg ) << MencV->MposSftR );
	/*----------------------------------------------------------------------------------------------*/
	/*		Receive dataL,HUpdate																		*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->RxPosL[1].Long = MencV->RxPosL[0].Long;	/*<S0C5>*/
	MencV->RxPosH[1] 	  = MencV->RxPosH[0];		/*<S0C5>*/
	MencV->RxPosL[0].Long = (LONG) SHal_GetCurRxPosL( pAsicHwReg );
	MencV->RxPosH[0] 	  = (SHORT)SHal_GetCurRxPosH( pAsicHwReg );
	/*----------------------------------------------------------------------------------------------*/
	SHal_SrcClr( pAsicHwReg );								/* Update reception completion flag					*/
	/*----------------------------------------------------------------------------------------------*/
//	return err_code;/* <S0E0> */
	return;
}
/****************************************************************************************************/
/*																									*/
/*		Fully closed encoder position data reception processing(SW)												*/
/*																									*/
/****************************************************************************************************/
static LONG	FencRcvPosSwProcess( MENCV *FencV, ALMMAN_HANDLE *AlmManager, BOOL BeComplete ) /* <S0E0>:AlmManageraddition */
{
	ULONG	RxFlg;
	LONG	FencPosX;
	USHORT	RxPosH_wk;
	USHORT	BaseTime;
	SENCX	*Enc;
	Enc = &FencV->EncIf;
	
	RxFlg = SHalDrv_SencGetRxStatus( (void*)Enc->RxFlg );
	if( ((RxFlg & 0x00008000) == 0) /*&& (FencV->SeRxComp == FALSE)*/ )
	{
		/* Receiving incomplete : Fulque encoder reception error processing */
		FencPosX = FencComErrorProc(FencV, AlmManager);
	}
	else
	{
		/* When reception is completed : Fulque encoder reception completion processing */
		FencV->SeRxComp = FALSE; /* Reset Rx Complete Flag			*/

		/*------------------------------------------------------------------*/
		/*	Receive data acquisition													*/
		/*------------------------------------------------------------------*/
		SHalDrv_SencGetRxData( (void*)Enc->RxBuf, (void*)&Enc->RxData );

		/*------------------------------------------------------------------*/
		/*	Set receive data as internal variable										*/
		/*------------------------------------------------------------------*/
//		RxPosL_wk = FencV->RxPosL;
		RxPosH_wk = (USHORT)FencV->RxPosH[0];
		FencV->RxPosH[1] = (LONG)RxPosH_wk;

		FencV->RxAlmCode 	= Enc->RxData.UsBuf[2];		/* Alarm Code		*/
		FencV->RxTimex.w.l 	= Enc->RxData.UsBuf[3];		/* TimeBase, etc	*/
		FencV->RxTimex.w.h 	= Enc->RxData.UsBuf[4];		/* TimeBase, etc	*/
		FencV->RxPosL[0].w.l= Enc->RxData.UsBuf[5];		/* Position Low		*/
		FencV->RxPosL[0].w.h= Enc->RxData.UsBuf[6];		/* Position High	*/
		FencV->RxPosH[0] 	= Enc->RxData.UsBuf[7];		/* Incre. Compen.	*/

		/*--------------------------------------------------------------------
		*  If the base timer is the same as the previous value, it is considered a communication error
		*--------------------------------------------------------------------*/
		BaseTime = (USHORT)(FencV->RxTimex.dw >> 8);
		if(FencV->FbTimeBase.BaseTimer == BaseTime)
		{
			/*--------------------------------------------------------------*/
			/*	When communication error occurs													*/
			/*--------------------------------------------------------------*/
			FencPosX = FencComErrorProc(FencV, AlmManager);
		}
		else
		{
			/*--------------------------------------------------------------*/
			/*	When communication is normal													*/
			/*--------------------------------------------------------------*/
			FencV->MstErrCnt = 0;
			FencV->SeRxErr = FALSE;
			FencV->EncDataNG = FALSE;

			FencPosX = FencV->MposSign *
					((FencV->RxPosL[0].Long >> FencV->MposSftR) << FencV->MposSftR);
		}

		FencV->FbTimeBase.BaseTimer = BaseTime;
	}
	
	return FencPosX;
}
/****************************************************************************************************/
/*																									*/
/*		Serial encoder communication error processing																*/
/*																									*/
/****************************************************************************************************/
static void	LpxSencComErrorProc( MENCV *MencV, ALMMAN_HANDLE *AlmManager, LONG RcvPosX, BOOL AccErr )/*<S0C5>*/
{

/*--------------------------------------------------------------------------------------------------*/
/*		Processing when an alarm occurs: Alarm settings																*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->MstErrCnt > MencV->ComErrLimit )				/* Error counter check				*/
	{
		if( MencV->ComErrCnt >= MencV->AccErrCnt )			/* Error type determination(Communication/Acceleration)			*/
		{
			MencV->EncDataNG = TRUE;						/* Encoder data invalid					*/
			ALMSetServoAlarm( AlmManager, ALM_PGCOM );		/* A.C90 : Encoder communication error			*//*<S0C5>*/
		}
		else
		{
			MencV->EncDataNG = TRUE;						/* Encoder data invalid					*/
			ALMSetServoAlarm( AlmManager, ALM_PGCOM1 );		/* A.C91 : Encoder communication error(Acceleration)	*//*<S0C5>*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Processing when non-alarm: Interpolation calculation using previous value														*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		MencV->RcvPosX[2]  = MencV->RcvPosX[1];				/* Value update							*/
		MencV->RcvPosX[1]  = MencV->RcvPosX[0];				/* Previous value updated							*/
		if( AccErr )
		{
			/* No interpolation when acceleration is abnormal */
			MencV->RcvPosX[0] = RcvPosX;
		}
		else
		{
			/* This value is calculated by interpolation */
			MencV->RcvPosX[0] += (MencV->RcvPosX[1] - MencV->RcvPosX[2]);
		}
		MencV->MstErrCnt++;									/* Count up Error Counter				*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return;/*<S0C5>*/
}


/****************************************************************************************************/
/*																									*/
/*		Rotation type: Serial encoder origin passage check													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Checks the origin passage during absolute increment and increment, and sets the origin position.		*/
/*			Outputs divided pulses.																*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/* ★Function name must be changed Y.Oka★ */
static void	RmxCheckSencOriginPass( MENCV *MencV )
{
	LONG	Orgx;
	LONG	Zsig;
	LONG	wk1, wk2, wk3;

/*--------------------------------------------------------------------------------------------------*/
/*		Phase origin setting & C phase mask cancellation processing															*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->Cset == FALSE )
	{
		if( (MencV->RxAlmCode & 0x0035)!=0x0001 )			/* Fixed bit check(bit0,2,4,5)	*/
		{
			MencV->RxPosH[0] = MencV->RxPosH[1];			/* This ink correction value will be saved for the next time	*/
		}														/* Do not exceed the amount and use the previous value				*/
		else
		{
			if( MencV->Sigma3Encoder )						/* Encoder type check			*/
			{
				Zsig = (MencV->RxPosH[0] & 0x0001);			/* ΣⅢ: Origin signal					*/
			}
			else
			{
				Zsig = (MencV->RxPosL[0].w.l & 0x0100);		/* ΣⅡ: Origin signal					*/
			}
			Orgx = (MencV->RxPosH[0] >> MencV->IncreCompSft);		/* Increment correction amount			*/
		/*------------------------------------------------------------------------------------------*/
		/*	Not passed the originFlag == 0  or Increment correction amount change												*/
		/*------------------------------------------------------------------------------------------*/
			if( ((MencV->RxAlmCode & 0x0040) == 0) ||
				(Orgx != (MencV->RxPosH[1] >> MencV->IncreCompSft)) )
			{
				MencV->Cset = TRUE;							/* C phase setting complete						*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Origin signal = 1 and Lower counter is near the origin													*/
		/*------------------------------------------------------------------------------------------*/
			else if( Zsig != 0 )
			{
				if( (MencV->RxPosL[0].w.h & 0x000c) == 0x0000 )
				{
					MencV->Cset = TRUE;						/* C phase setting complete						*/
					Orgx = MencV->RxPosL[0].w.h >> 4;
				}
				else if( (MencV->RxPosL[0].w.h & 0x000c) == 0x000c )
				{
					MencV->Cset = TRUE;						/* C phase setting complete						*/
					Orgx = (MencV->RxPosL[0].w.h >> 4) + 1;
				}
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Phase origin setting & C phase mask cancellation processing														*/
		/*------------------------------------------------------------------------------------------*/
			if( MencV->Cset )
			{
				MencV->MotOrgX = MencV->MposSign * (Orgx << MencV->MorgSft);
			/*--------------------------------------------------------------------------------------*/
				MencV->IncreCposLatch = TRUE;				/* C-phase position latch completed   --> ScanB	*/
				MencV->MotCposX = MencV->MotOrgX;			/* C phase position latch data --> ScanB	*/
			/*--------------------------------------------------------------------------------------*/
#if 0/*<S02D>*/
				if( MencV->DivOut == TRUE )					/* Motor encoder frequency division output selection?	*/
				{
					/* Increment correction origin output setting */
					Orgx = MencV->MotOrgX >> MencV->DivOutSft;
					/* Pulse conversion origin correction1 */
					SHalDrv_SetOriginCorrect1( MencV->EncIf.SencSetReg, (Orgx - 1) );	/* <S04D> Argument change */
					/* Pulse conversion origin correction2 */
					SHalDrv_SetOriginCorrect2( MencV->EncIf.SencSetReg, Orgx );		/* <S04D> Argument change */
					/* DivisionCPhase maskOFF */
					MencV->DivCMaskStatus  = DIV_CMASK_OFF;
				}
#else/*<S02D>*/
				if( MencV->DivInfo.DivOut == TRUE )			/* Motor encoder frequency division output selection?	*/
				{
					/* Increment correction origin output setting */
					Orgx = MencV->MotOrgX >> MencV->DivInfo.DivOutSft;
					/* Pulse conversion origin correction1 */
					SHalDrv_SetOriginCorrect1( MencV->EncIf.SencSetReg, (Orgx - 1) );
					/* Pulse conversion origin correction2 */
					SHalDrv_SetOriginCorrect2( MencV->EncIf.SencSetReg, Orgx );
					/* DivisionCPhase maskOFF */
					MencV->DivInfo.DivCMaskStatus  = DIV_CMASK_OFF;
					/* Pulse conversionCPhase maskOFFSetting	*/
					MencV->EncIf.SencSetReg->AREG_WDT1SET &= ~BIT8;							/*<S02D>*/
				}
#endif/*<S02D>*/
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase passage setting process																			*/
/*--------------------------------------------------------------------------------------------------*/
#if 0/*<S02D>*/
	if( MencV->Cset )
	{
		wk1 = (MencV->MotPosX[0] - MencV->MotOrgX);
		wk2 = (MencV->MotPosX[1] - MencV->MotOrgX);
		wk3 = wk1 >> MencV->DivOutSft;							/* Divided current frequency origin reference position			*/
	/*----------------------------------------------------------------------------------------------*/
		wk2 = (wk1 ^ wk2) & 0xc0000000;
		wk1 = wk1 & 0xc0000000;
		if( (wk2 == 0xc0000000) && ((wk1 == 0x00000000) || (wk1 == 0xc0000000)) )
		{
			MencV->Cpass = TRUE;								/* C phase passage setting(Polarity reversal)			*/
			MencV->DivCpassPos = wk3;						/* C phase passage time division position storage			*/
		}
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivCpass == FALSE )						/* Division origin passage check				*/
		{
			if( MencV->Cpass )
			{
				if( ((MencV->DivCpassPos >= 0) && (wk3 >  3)) ||
					((MencV->DivCpassPos <  0) && (wk3 < -4))	)
				{
					MencV->DivCpass = TRUE;					/* Divide home point passing setting					*/
				}
			}
		}
		else
		{
			MencV->OrgOk = TRUE;								/* Origin setting completed						*/
		}
	}
#else/*<S02D>*/
	if( MencV->Cset )
	{
		wk1 = (MencV->MotPosX[0] - MencV->MotCposX);
		wk2 = (MencV->MotPosX[1] - MencV->MotCposX);
		wk3 = wk1 >> MencV->DivInfo.DivOutSft;					/* Divided current frequency origin reference position			*/
	/*----------------------------------------------------------------------------------------------*/
		wk2 = (wk1 ^ wk2) & 0xc0000000;
		wk1 = wk1 & 0xc0000000;
		if( (wk2 == 0xc0000000) && ((wk1 == 0x00000000) || (wk1 == 0xc0000000)) )
		{
			MencV->Cpass = TRUE;								/* C phase passage setting(Polarity reversal)			*/
			MencV->DivInfo.DivCpassPos = wk3;					/* C phase passage time division position storage			*/
			if( (MencV->DivInfo.DivOut == TRUE) && (MencV->DivInfo.DivPass == FALSE) )
			{
				MencV->EncIf.SencSetReg->AREG_DIVSET |= (BIT11 | BIT8);/* Division C phase output selection 2			*/
			}
		}
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivInfo.DivCpass == FALSE )						/* Division origin passage check			*/
		{
			if( MencV->Cpass )
			{
				if( ((MencV->DivInfo.DivCpassPos >= 0) && (wk3 >  3)) ||
					((MencV->DivInfo.DivCpassPos <  0) && (wk3 < -4))	)
				{
					MencV->DivInfo.DivCpass = TRUE;					/* Divide home point passing setting				*/
				}
			}
		}
		else
		{
			if( MencV->DivInfo.DivOut == TRUE )					/* Motor encoder frequency division output selection?	*/
			{ 
				MencV->EncIf.SencSetReg->AREG_DINCSET &= ~(BIT10); /* Divide always clear settingOFF 			*/
			}
			MencV->OrgOk = TRUE;								/* Origin setting completed						*/
		}
	}
#endif/*<S02D>*/

}

/****************************************************************************************************/
/*																									*/
/*		Rotary type: Read serial encoder position data													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Transfers the position data received by the ASIC from the serial encoder to the ASIC register				*/
/*			Read, motor encoder variable(MencV)											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	SencReadPosition( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg,
									ALMMAN_HANDLE *AlmManager, BOOL BeComplete,	void *BaseLoops )/* <S018> *//*<S0C5>*//*<S1B7>*/
{
	LONG		MotPosX;
	LONG		LastMotPosX;
	LONG		dMotPos;
	SENCX		*Enc;
	LONG		lwk1;  		/*<S02D>*/
	LONG		DivMotPosX; /*<S079>*/

	LastMotPosX	= MencV->MotPosX[0];
	Enc			= &MencV->EncIf;

/*--------------------------------------------------------------------------------------------------*/
/*		Unconnected state																					*/
/*--------------------------------------------------------------------------------------------------*/
	if((MencV->EncConnect == FALSE) || (MencV->EncDisable != FALSE))
	{
//		LpxSencComErrorProc(MencV, AlmManager, 0, FALSE);											  /* <S0DD> */
		MotPosX    = ( SHal_GetCurRcvPos( pAsicHwReg ) << MencV->MposSftR );
		DivMotPosX = SHal_GetCurRcvPos( pAsicHwReg );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder position correction processing using change time timer													*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		switch(MencV->PosCompMode)
		{
		/*------------------------------------------------------------------------------------------*/
		/*		JL-086Encoder position correction(Normal processing)												*/
		/*------------------------------------------------------------------------------------------*/
		case POSCOMP_ASIC:
			SencRcvPosHwProcess( MencV, pAsicHwReg, AlmManager );
			MotPosX    = ( SHal_GetCurMotPos( pAsicHwReg ) << MencV->MposSftR );
			DivMotPosX = SHal_GetCurMotPos( pAsicHwReg );
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		SWEncoder position correction(Σ-ⅤCompatibility)													*/
		/*------------------------------------------------------------------------------------------*/
		case POSCOMP_SW:
			SencRcvPosSwProcess( MencV, AlmManager, BeComplete );
			MotPosX    = LpxTimeBasesMotPosCalc( MencV );
			DivMotPosX = SHal_GetCurRcvPos( pAsicHwReg );
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		No encoder position correction																*/
		/*------------------------------------------------------------------------------------------*/
		case POSCOMP_NOP:
		default:
			SencRcvPosHwProcess( MencV, pAsicHwReg, AlmManager );
			MotPosX    = ( SHal_GetCurRcvPos( pAsicHwReg ) << MencV->MposSftR );
			DivMotPosX = SHal_GetCurRcvPos( pAsicHwReg );
			break;
		}
	}
#if 0
	if( MencV->PosCompCheck == POSCOMP_SW )
	{	/*------------------------------------------------------------------------------------------*/
		/*		SWEncoder position correction(Σ-ⅤCompatibility)													*/
		/*------------------------------------------------------------------------------------------*/
		err_code = SencRcvPosSwProcess( MencV, BeComplete );
		MotPosX  = LpxTimeBasesMotPosCalc( MencV );
	}
	else
	{
		err_code = SencRcvPosHwProcess( MencV, pAsicHwReg );
		/*------------------------------------------------------------------------------------------*/
		/*		JL-086Encoder position correction														*/
		/*------------------------------------------------------------------------------------------*/
		if( MencV->PosCompCheck == POSCOMP_ASIC )
		{
			MotPosX = ( SHal_GetCurMotPos( pAsicHwReg ) << MencV->MposSftR );
			DivMotPosX = SHal_GetCurMotPos( pAsicHwReg );  /*<S079>*/
		}
		/*------------------------------------------------------------------------------------------*/
		/*		No encoder position correction																*/
		/*------------------------------------------------------------------------------------------*/
		else
		{
			MotPosX = ( SHal_GetCurRcvPos( pAsicHwReg ) << MencV->MposSftR );
		}
	}
#endif
	/*----------------------------------------------------------------------------------------------*/
	/* Update of position data after correction 																		*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->MotPosX[1]	= MencV->MotPosX[0];				/* Position data after previous correction					*/
	MencV->MotPosX[0]	= MotPosX;							/* Position data after correction this time					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Motor position output(32bitRingPulse)																*/
/*--------------------------------------------------------------------------------------------------*/
	dMotPos = ( (MencV->MotPosX[0] - LastMotPosX) >> MencV->MposSftR );
	MencV->MotPos   = MencV->MotPos + dMotPos;

	if( MencV->AxisMotType == MOTTYPE_LINEAR )/*<S017>*/
	{ /* Linear type */
		LmxUpdateMotPhase( MencV, AlmManager, dMotPos );
	}
	else
	{ /* Rotation type */
		MencV->MotPhase = (USHORT)(( (((MencV->MotPosX[0]) - (MencV->MotOrgX)) >>8) * MencV->Kmotphase)>>8);
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Feedback pulse output processing																*/
/*--------------------------------------------------------------------------------------------------*/
	// No initial increment output
#if 0/*<S02D>*/
	if( (MencV->OrgOk == FALSE) && (MencV->SeRxErr == FALSE) )
	{
		if( MencV->AxisMotType == MOTTYPE_LINEAR )/* <S029> : Linear processing added */
		{ /* Linear type */
			LmxCheckSencCphasePass(MencV);					/* Origin passing check						*/
		}
		else
		{ /* Rotation type */
			RmxCheckSencOriginPass(MencV);					/* Origin passing check						*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Division pulse output processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	// No initial increment output
	if( MencV->DivOut == TRUE )
	{
		SHalDrv_SetDivOutPosition( Enc->SencSetReg, (MencV->MotPosX[0] >> MencV->DivOutSft) );
	}
#else/*<S02D>*/
	if( MencV->DivInfo.IncPulseReq == TRUE )				/* Initial incremental output request?				*/
	{
		/*------------------------------------------------------------------------------------------*/
		/* Countermeasures for frequency-divided pulse output when the motor is moving during initialization at power-on		     	        */
		/*		ScanAProcessing1Set the counter preset and pulse conversion position.		        */
		/*------------------------------------------------------------------------------------------*/
		if( MencV->AbsoEncType == MENC_INCRE_TYPE )
		{
			if( MencV->SeRxErr == FALSE )
			{
				/*---------------------------------------------------------------------------------*/
				/*		Origin passing check process													   */
				/*---------------------------------------------------------------------------------*/
				if( MencV->AxisMotType == MOTTYPE_LINEAR )/*Linear processing omission added <S040> */		/* TODO It is strange that there is an increment mode branch in the division process! ! Correction required*/
				{ /* Linear type */
					LmxCheckSencCphasePass(MencV);
				}
				else
				{ /* Rotation type */
					RmxCheckSencOriginPass(MencV);
				}
			}
			/*-------------------------------------------------------------------------------------*/
			/*		Division pulse output processing	(Set counter preset just before)						   */
			/*-------------------------------------------------------------------------------------*/
			if( MencV->DivInfo.DivOut == TRUE )
			{
#if 0/*<S079>*/
				Enc->SencSetReg->AREG_POTEST = (ULONG)(MencV->MotPosX[0] >> MencV->DivInfo.DivOutSft);
				SHalDrv_SetDivOutPosition( Enc->SencSetReg, (MencV->MotPosX[0] >> MencV->DivInfo.DivOutSft) );
#else/*<S079>*/
				if(MencV->DivInfo.DivOutASIC != FALSE)
				{ /* JL-086 */
					Enc->SencSetReg->AREG_POTEST = (ULONG) DivMotPosX;
					SHalDrv_SetDivOutPosition( Enc->SencSetReg, DivMotPosX );
				}
				else
				{
#if 0 /* <S21B>:Start */
					Enc->SencSetReg->AREG_POTEST = (ULONG)(MencV->MotPosX[0] >> MencV->DivInfo.DivOutSft);
					SHalDrv_SetDivOutPosition( Enc->SencSetReg, (MencV->MotPosX[0] >> MencV->DivInfo.DivOutSft) );
#else
					if( MencV->AxisMotType == MOTTYPE_LINEAR )
					{ /* Linear type */
						MencV->DivInfo.DivOutPos += MlibPfbkxremNolim(dMotPos, MencV->DivInfo.DivOutGain, &MencV->DivInfo.DivOutRem);
						SHalDrv_SetDivOutPosition( Enc->SencSetReg, (ULONG)MencV->DivInfo.DivOutPos );
					}
					else
					{ /* Rotation type */
						Enc->SencSetReg->AREG_POTEST = (ULONG)(MencV->MotPosX[0] >> MencV->DivInfo.DivOutSft);
						SHalDrv_SetDivOutPosition( Enc->SencSetReg, (MencV->MotPosX[0] >> MencV->DivInfo.DivOutSft) );
					}
#endif/* <S21B>:end */
				}
#endif/*<S079>*/
			}
			MencV->DivInfo.IncPulseReq = FALSE;
		}
		else
		{
			/*-------------------------------------------------------------------------------------*/
			/*		Initial increment pulse output processing												       */
			/*-------------------------------------------------------------------------------------*/
			if( MencV->AxisMotType == MOTTYPE_LINEAR )/* <S209> : Linear processing added */
			{ /* Linear type */
				LmxOutSencIniIncPulse(MencV, pAsicHwReg, LMX_MPOS_SFT);
			}
			else
			{ /* Rotation type */
				SencOutIniIncPulse( MencV, pAsicHwReg );
			}
			if((MencV->DivInfo.DivOutASIC != FALSE) && (MencV->DivInfo.IncPulseReq == FALSE))  /*<S079>*/
			{ /* JL-086, after completion of initial increpulse outputJL-086 */
				Enc->SencSetReg->AREG_DIVSET &= ~BIT1;
			}
		}
	}
	else if( (MencV->OrgOk == FALSE) && (MencV->SeRxErr == FALSE) )
	{
		/*------------------------------------------------------------------------------------------*/
		/*		Origin passing check process												                */
		/*------------------------------------------------------------------------------------------*/
//		RmxCheckSencOriginPass(MencV);            /* <S034>DELL */
		if( MencV->AxisMotType == MOTTYPE_LINEAR )/* <S029> : Linear processing added */
		{ /* Linear type */
			LmxCheckSencCphasePass(MencV);					/* Origin passing check						*/
		}
		else
		{ /* Rotation type */
			RmxCheckSencOriginPass(MencV);					/* Origin passing check						*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Division pulse output processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( (MencV->DivInfo.DivOut == TRUE) && (MencV->DivInfo.IncPulseReq == FALSE) && (MencV->DivInfo.DivOutASIC == FALSE) )
	{
		if( MencV->AxisMotType != MOTTYPE_LINEAR )/* <S21B>:Linear processing added */
		{ /* Rotation type */
			SHalDrv_SetDivOutPosition( Enc->SencSetReg, (MencV->MotPosX[0] >> MencV->DivInfo.DivOutSft) );
		/*----------------------------------------------------------------------------------------------*/
		/*  CPhase output processing													<S02D>							*/
		/*----------------------------------------------------------------------------------------------*/
			if( MencV->DivInfo.DivPcmaskCtrl )
			{
				lwk1 = ( MencV->MotPosX[0] - MencV->MotOrgX);
				if( ( (lwk1 > -MencV->DivInfo.DivOutCmskChk) && (lwk1 < MencV->DivInfo.DivOutCmskChk)) && (MencV->Cset) )
				{   /* Pulse conversionCPhase maskOFFSetting			*/
					Enc->SencSetReg->AREG_WDT1SET &= ~BIT8;
					if( MencV->DivInfo.DivPass == FALSE )
					{
						if( MencV->OrgOk )
						{ /* Encoder position correction channel0CPhase2 */
							Enc->SencSetReg->AREG_DIVSET |= (BIT11 | BIT8);
						}
					}
				}
				else
				{	/* Pulse conversionCPhase maskONSetting			*/
					Enc->SencSetReg->AREG_WDT1SET |= BIT8;
					/* Encoder position correction channel0CSet phase to normal */
					Enc->SencSetReg->AREG_DIVSET |= BIT8;
				}
			}
		/*----------------------------------------------------------------------------------------------*/
		}
		else/* <S21B>:Linear processing added */
		{/* Linear type */
			MencV->DivInfo.DivOutPos += MlibPfbkxremNolim(dMotPos, MencV->DivInfo.DivOutGain, &MencV->DivInfo.DivOutRem);
			SHalDrv_SetDivOutPosition( Enc->SencSetReg, (ULONG)MencV->DivInfo.DivOutPos );			
		}
	}

#endif/*<S02D>*/
/*--------------------------------------------------------------------------------------------------*/
	return;
}

/****************************************************************************************************/
/*																									*/
/*		Rotary type: Serial encoder position data reception processing(HW:JL-086)										*/
/*																									*/
/****************************************************************************************************//* <S004> */
static void	SencRcvPosHwProcess( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg, ALMMAN_HANDLE *AlmManager )/*<S0C5>*/
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Processing when an alarm occurs																		*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->SeRxErr   = SHal_GetSeRxErr( pAsicHwReg );		/* Update reception error flag 				*/
	MencV->EncDataNG = SHal_GetEncDataNG( pAsicHwReg );		/* Update encoder data error flag 	*/
	/*----------------------------------------------------------------------------------------------*/
	if( SHal_GetComAlarm( pAsicHwReg ) )					/* Error type determination(Communication/Acceleration)			*/
	{
		ALMSetServoAlarm( AlmManager, ALM_PGCOM );			/* A.C90 : Encoder communication error			*//*<S0C5>*/
	}
	if( SHal_GetAccAlarm( pAsicHwReg ) )					/* Error type determination(Communication/Acceleration)			*/
	{
		ALMSetServoAlarm( AlmManager, ALM_PGCOM1 );			/* A.C91 : Encoder communication error(Acceleration)	*//*<S0C5>*/
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Update position data without correction																	*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->RcvPosX[2] = MencV->RcvPosX[1];			/*<S0C5>*/
	MencV->RcvPosX[1] = MencV->RcvPosX[0];			/*<S0C5>*/
	MencV->RcvPosX[0] = (LONG)( SHal_GetCurRcvPos( pAsicHwReg ) << MencV->MposSftR );
	/*----------------------------------------------------------------------------------------------*/
	/*		Receive dataL,HUpdate																		*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->RxPosL[1].Long = MencV->RxPosL[0].Long;	/*<S0C5>*/
	MencV->RxPosH[1] 	  = MencV->RxPosH[0];		/*<S0C5>*/
	MencV->RxPosL[0].Long = (LONG) SHal_GetCurRxPosL( pAsicHwReg );
	MencV->RxPosH[0] 	  = (SHORT)SHal_GetCurRxPosH( pAsicHwReg );
	/*----------------------------------------------------------------------------------------------*/
	/*		Change position timer, base timer update														*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->BaseTimer	  = (USHORT)SHal_GetBaseTim( pAsicHwReg );									  /* <S067> */
	MencV->HenkaItiTimer  = (USHORT)SHal_GetHenkaItiTim( pAsicHwReg );								  /* <S067> */

	/*----------------------------------------------------------------------------------------------*/
	SHal_SrcClr( pAsicHwReg );								/* Update reception completion flag					*/
	/*----------------------------------------------------------------------------------------------*/
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Rotary type: Serial encoder position data reception processing(SW)											*/
/*																									*/
/****************************************************************************************************//* <S004> */
static void	SencRcvPosSwProcess( MENCV *MencV, ALMMAN_HANDLE *AlmManager, BOOL BeComplete )	  /* <S0C5> */
{
	ULONG	ulwk;
	LONG	lwk1;
	LONG	RcvPosX;
	SENCX	*Enc;
	ULONG	RxFlg;

	Enc	= &MencV->EncIf;

/*--------------------------------------------------------------------------------------------------*/
/*		Receiving incomplete : Serial encoder reception error processing													*/
/*--------------------------------------------------------------------------------------------------*/
	RxFlg = SHalDrv_SencGetRxStatus( (void*)Enc->RxFlg );
	if( ((RxFlg & 0x8000) == 0) && (MencV->SeRxComp == FALSE) )
	{
		MencV->ComErrCnt++;									/* Count up Error Counter				*/
		MencV->SeRxErr = TRUE;								/* Set Error Flag						*/
		LpxSencComErrorProc(MencV, AlmManager, 0, FALSE);	/*<S0C5>*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Processing when encoder data is invalid															*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->EncDataNG == TRUE )
		{
			if( BeComplete == FALSE )								/* During base block				*/
			{
			/*--------------------------------------------------------------------------------------*/
			/*	Previous value/previous value update process															*/
			/*		When encoder data is invalid && During base block,									*/
			/*		Update the previous value in order to set the speed calculated from the difference between the current value and the previous value to 0.			*/
			/*--------------------------------------------------------------------------------------*/
				MencV->RxPosL[1]  = MencV->RxPosL[0];				/* Previous value updated					*/
				MencV->RxPosH[1]  = MencV->RxPosH[0];				/* Previous value updated					*/
				MencV->RcvPosX[2] = MencV->RcvPosX[1];				/* Value update					*/
				MencV->RcvPosX[1] = MencV->RcvPosX[0];				/* Previous value updated					*/
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		When reception is completed : Serial encoder reception completion processing													*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
	/*----------------------------------------------------------------------------------------------*/
	/*	Receive data acquisition																				*/
	/*----------------------------------------------------------------------------------------------*/
		SHalDrv_SencGetRxData( (void*)Enc->RxBuf, (void*)&Enc->RxData );
	/*----------------------------------------------------------------------------------------------*/
	/*	Reception completionFlagReset:IntSync Set --> ScanA Clear											*/
	/*----------------------------------------------------------------------------------------------*/
		MencV->SeRxComp = FALSE;									/* Reset Rx Complete Flag		*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Set receive data as internal variable																	*/
	/*----------------------------------------------------------------------------------------------*/
		MencV->RxPosL[1]		= MencV->RxPosL[0];					/* Previous value updated					*/
		MencV->RxPosH[1]		= MencV->RxPosH[0];					/* Previous value updated					*/
	/*----------------------------------------------------------------------------------------------*/
		MencV->RxAlmCode		= (USHORT)(Enc->RxData.UsBuf[2]);	/* Read current value:Alarm Code		*/
		MencV->RxTimex.w.l		= (USHORT)(Enc->RxData.UsBuf[3]);	/* Read current value:TimeBase, etc	*/
		MencV->RxTimex.w.h		= (USHORT)(Enc->RxData.UsBuf[4]);	/* Read current value:TimeBase, etc	*/
		MencV->RxPosL[0].w.l	= (USHORT)(Enc->RxData.UsBuf[5]);	/* Read current value:Position Low	*/
		MencV->RxPosL[0].w.h	= (USHORT)(Enc->RxData.UsBuf[6]);	/* Read current value:Position High	*/
		MencV->RxPosH[0]		= (USHORT)(Enc->RxData.UsBuf[7]);	/* Read current value:Incre. Compen.	*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Position calculation																					*/
	/*----------------------------------------------------------------------------------------------*/
		RcvPosX = MencV->MposSign * ((MencV->RxPosL[0].Long >> MencV->MposSftX) << MencV->MposSftR);
	/*----------------------------------------------------------------------------------------------*/
	/*	Acceleration calculation check																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->AccChkEnable == FALSE )
		{
			lwk1 = 0;
			MencV->AccChkWaitCnt++;
			if (MencV->AccChkWaitCnt > 3)
			{
				MencV->AccChkEnable = TRUE;
			}
			MencV->RcvPosX[0] = MencV->RcvPosX[1] = MencV->RcvPosX[2] = RcvPosX;
		}
	/*----------------------------------------------------------------------------------------------*/
	/*	Acceleration calculation : Speed this time - Previous speed (If the current acceleration is abnormal,Recalculation using data from two previous times)		*/
	/*----------------------------------------------------------------------------------------------*/
		else
		{
			lwk1 = (RcvPosX - MencV->RcvPosX[0]) - (MencV->RcvPosX[0] - MencV->RcvPosX[1]);
			if( (lwk1 > MencV->AccErrLvl) || (lwk1 < -(MencV->AccErrLvl)) )
			{
				lwk1 = ((RcvPosX - MencV->RcvPosX[1])>>1) - (MencV->RcvPosX[1] - MencV->RcvPosX[2]);
			}
		}
	/*----------------------------------------------------------------------------------------------*/
	/*	Acceleration abnormality : Serial encoder communication error processing												*/
	/*----------------------------------------------------------------------------------------------*/
		if( (lwk1 > MencV->AccErrLvl) || (lwk1 < -(MencV->AccErrLvl)) )
		{
			MencV->AccErrCnt++;								/* Count up Error Counter				*/
			MencV->SeRxErr = TRUE;							/* Set Error Flag						*/
			LpxSencComErrorProc(MencV, AlmManager, 0, FALSE);/*<S0C5>*/
		/*------------------------------------------------------------------------------------------*/
		/*		Processing when encoder data is invalid														*/
		/*------------------------------------------------------------------------------------------*/
			if( MencV->EncDataNG == TRUE )
			{
				if( BeComplete == FALSE )								/* During base block			*/
				{
				/*----------------------------------------------------------------------------------*/
				/*	Previous value/previous value update process														*/
				/*		When encoder data is invalid && During base block,								*/
				/*		Update the previous value in order to set the speed calculated from the difference between the current value and the previous value to 0.		*/
				/*----------------------------------------------------------------------------------*/
					MencV->RxPosL[1]  = MencV->RxPosL[0];				/* Previous value updated				*/
					MencV->RxPosH[1]  = MencV->RxPosH[0];				/* Previous value updated				*/
					MencV->RcvPosX[2] = MencV->RcvPosX[1];				/* Value update				*/
					MencV->RcvPosX[1] = MencV->RcvPosX[0];				/* Previous value updated				*/
				}
			}
		}
	/*----------------------------------------------------------------------------------------------*/
	/*	When acceleration is normal : Motor magnetic pole phase calculation															*/
	/*----------------------------------------------------------------------------------------------*/
		else
		{
			MencV->MstErrCnt   = 0;							/* Reset Error Counter					*/
			MencV->SeRxErr     = FALSE;						/* Reset Error Flag						*/
			MencV->EncDataNG   = FALSE;						/* Reset Encoder Data NG Flag			*/
			MencV->RcvPosX[2]  = MencV->RcvPosX[1];			/* Position data						*/
			MencV->RcvPosX[1]  = MencV->RcvPosX[0];			/* Previous position data						*/
			MencV->RcvPosX[0]  = RcvPosX;					/* Current position data						*/
		}
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Create base timer value, change position timer value			<S067>										*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->BaseTimer = (USHORT)(MencV->RxTimex.dw >> 8);
	ulwk = ((ULONG)MencV->RxPosL[0].w.l << 16) + (ULONG)MencV->RxTimex.w.h;
	MencV->HenkaItiTimer = (USHORT)(ulwk >> 8);

	return;
}


/****************************************************************************************************/
/*																									*/
/*		Linear motor: Motor current phase update process	<S018>												*/
/*																									*/
/****************************************************************************************************/
#define	LMX_MORG_CHKMAX		3
/*--------------------------------------------------------------------------------------------------*/
static void	LmxUpdateMotPhase( MENCV *MencV, ALMMAN_HANDLE *AlmManager, LONG dMotPos )
{
	LONG	lwk1;
	LONG	lwk2;
	LONG	LastMotPhaX;
	LONG	NewPoleInfo;
	LONG	MotPoleInfoX;

/*--------------------------------------------------------------------------------------------------*/
/*		Motor phase position calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
		LastMotPhaX = MencV->MotPhaX;						/* Previous value storage							*/
		lwk1 = MencV->MotPhaX + dMotPos;					/* Calculate current value							*/
	/*----------------------------------------------------------------------------------------------*/
		if( lwk1 >= (LONG)MencV->PulseNo )					/* Check Upper Limit					*/
		{													/*										*/
			lwk1 -= (LONG)MencV->PulseNo;					/* Adjust MotPos by PulseNo				*/
		}													/*										*/
		else if( lwk1 < 0 )									/* Check Lower Limit					*/
		{													/*										*/
			lwk1 += (LONG)MencV->PulseNo;					/* Adjust MotPos by PulseNo				*/
		}													/*										*/
	/*----------------------------------------------------------------------------------------------*/
		MencV->MotPhaX = lwk1;								/* Motor phase position update					*/

/*--------------------------------------------------------------------------------------------------*/
/*		With pole sensor																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( (MencV->withPoleSensor) && (!MencV->MotOrgChkEnd) && (MencV->MstErrCnt == 0) )
		{
			if( MencV->Sigma3Encoder )
			{
				NewPoleInfo = (MencV->RxPosH[0]>>1) & 0x07;
			}
			else
			{
				NewPoleInfo = (MencV->RxPosL[0].w.l>>9) & 0x07;
			}
			//if( MencV->PoleReverse == TRUE ){ NewPoleInfo = ~NewPoleInfo & 0x07; }	/* <V808>		*///I'll deal with it later! !kira
			MotPoleInfoX = MencV->MotPoleInfo[0];
			MlibBITFILTER1( MencV->MotPoleInfo[0], MencV->MotPoleInfo[1], (UCHAR)NewPoleInfo );
			MotPoleInfoX = ((MencV->MotPoleInfo[0] ^ MotPoleInfoX)<<4) | MencV->MotPoleInfo[0];
		/*------------------------------------------------------------------------------------------*/
		/*	UVW signal: Abnormality check:UVW = 000 or 111 abnormal in										*/
		/*------------------------------------------------------------------------------------------*/
			if( (MencV->MotPoleInfo[0] == 0x00) || (MencV->MotPoleInfo[0] == 0x07) )
			{
				ALMSetServoAlarm( AlmManager, ALM_POLESENSOR );	/* A.C21 : Pole sensor error 		*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*	UVW signal: U phase edge check:MotPoleInfoX = UVW-Edge(6,5,4bit) + UVW-Sts(2,1,0bit) 	*/
		/*------------------------------------------------------------------------------------------*/
			else if( (MotPoleInfoX & 0x43) == 0x41 )		/* U-Edge & W-ON : 0 deg				*/
			{
				if( MencV->MotOrgChkCnt < LMX_MORG_CHKMAX )
				{
					++MencV->MotOrgChkCnt;					/* Count up Motor Origin Check Counter	*/
				}
				if( (MencV->MotOrgChkAlways == FALSE) && (MencV->MotOrgChkCnt >= LMX_MORG_CHKMAX) )
				{
					MencV->MotOrgChkEnd = TRUE;				/* Set Motor Origin Check End Flag		*/
				}
			/*--------------------------------------------------------------------------------------*/
			/*	Motor origin check & update: Use previous value considering bit filter delay			*/
			/*--------------------------------------------------------------------------------------*/
				lwk1 = MlibABS( (LastMotPhaX - MencV->MotOrgXchk[0]) );
				lwk2 = MlibABS( (LastMotPhaX - MencV->MotOrgXchk[1]) );
				/*----------------------------------------------------------------------------------*/
				if( lwk1 > MencV->HalfPulseNo )		/*<V498>*/
				{
					lwk1 = MencV->PulseNo - lwk1;
				}
				if ( lwk2 > MencV->HalfPulseNo )		/*<V498>*/
				{
					lwk2 = MencV->PulseNo - lwk2;
				}
				/*----------------------------------------------------------------------------------*/
				if( lwk1 <= MencV->PulseNo30deg )
				{
					MencV->MotOrgX = LastMotPhaX;			/* Motor phase origin update					*/
				}
				else if( (MencV->MotOrgChkCnt >= LMX_MORG_CHKMAX) && (lwk2 > MencV->PulseNo30deg) )
				{
					ALMSetServoAlarm( AlmManager, ALM_PHASE );/* A.C20 : Phase error detection					*/
				}
			/*--------------------------------------------------------------------------------------*/
				MencV->MotOrgXchk[1] = MencV->MotOrgXchk[0];/* Update Motor Origin Check Data		*/
				MencV->MotOrgXchk[0] = LastMotPhaX;			/* Update Motor Origin Check Data		*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*	UVW signal: V phase edge check(Add as necessary,Phase sequence caution required)								*/
		/*------------------------------------------------------------------------------------------*/
			else if( 0 && (MotPoleInfoX & 0x25) == 0x24 )	/* V-Edge & U-ON : 120deg/240deg		*/
			{
				;											/* Phase sequenceUVWHour:120deg					*/
				;											/* Phase sequenceUWVHour:240deg					*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Motor phase calculation: The calculation result isUSHORTis used, so sign correction is not necessary(Confirm by test)			*/
		/*------------------------------------------------------------------------------------------*/
			lwk1 = MlibMulgainNolim( (MencV->MotPhaX - MencV->MotOrgX), MencV->Kmotphase );
			MencV->MotPhase = (USHORT)(lwk1 + MencV->PhaseOffset);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Without pole sensor																		*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			lwk1 = MlibMulgainNolim( (MencV->MotPhaX - MencV->MotOrgX), MencV->Kmotphase );
			MencV->MotPhase = (USHORT)(lwk1 + MencV->PhaseOffset + MencV->MpfPhaseOffset);
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}


#ifdef _TIME_BASE_TIMER_
/****************************************************************************************************/
/*																									*/
/*		Position correction calculation using change time timer														*/
/*																									*/
/*		Overview: Correcting the encoder position using the encoder information change time timer						*/
/*			  Measures for encoder sampling and beat phenomenon in servo control cycle((MTechniqueD1)V03-010)			*/
/*			  To compensate for sampling deviation, the encoder latches the last pulse and then			*/
/*			  Use the elapsed time (change time timer).											*/
/*			  Change time timer value is2.66us, base timer2.66usto calculate the previous position.			*/
/*																									*/
/*		Caution: Argument to prevent overflow of operation[2^32/rev]Unit[pulse], and after calculation, write to		*/
/*			  Return unit.																			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*		arg		:	IN		LONG	RcvPosX		: Current encoder position(Before correction)	[2^32/rev]			*/
/*				:	IN		LONG	LastMotPosX	: Previous encoder position(After correction)	[2^32/rev]			*/
/*				:	IN		UCHAR	MposSftR	: Number of position data shifts(32-BitNo)	[-]					*/
/*				: OUT		None																	*/
/*--------------------------------------------------------------------------------------------------*/
/*		return	:			LONG				  Encoder position this time(After correction)	[2^32/rev]			*/
/*--------------------------------------------------------------------------------------------------*/
/*		g-ref	:	MencV.RxPosL				: For change time timer reference								*/
/*					MencV.RxTimex.w.h			: Same as above												*/
/*					MencP.MposSftR				: [pulse]→[2^32]Number of shifts for conversion						*/
/*--------------------------------------------------------------------------------------------------*/
/*		MACRO	:	RMX_XposToUpos	: [2^32/rev]→[pulse]											*/
/****************************************************************************************************/
LONG	LpxTimeBasesMotPosCalc( MENCV *MencV )
{
	LONG	MotPosCmp;										/* Encoder position after correction this time[pulse]		*/
	ULONG	ulwk;											/* Work								*/
	SHORT	swk;											/* Work								*/
	USHORT	HenkaItiTimer;									/* Change position timer						*/
	LONG	LinEncPos;																				/* <S004> */

	LinEncPos = 1 << (32 - MencV->MposSftR) ;				/* Linear encoder position overflow	*//* <S004> */
		/*------------------------------------------------------------------------------------------*/
		/*	Preparation for calculation : Convert unit to avoid overflow in correction calculation							*/
		/*				[2^32/rev]→[pulse/rev]														*/
		/*------------------------------------------------------------------------------------------*/
		MotPosCmp = ((ULONG)MencV->RcvPosX[0]) >> MencV->MposSftR;	/* Initialize						*/
		if( MencV->SeRxErr == FALSE )						/* Reception error or No correction when acceleration is abnormal	*/
		{
			/*--------------------------------------------------------------------------------------*/
			/*	Change time timer acquisition																	*/
			/*																						*/
			/*				  Change position timer														*/
			/*	___________________↓____________________											*/
			/*	|         |/////////|/////////|         |											*/
			/*	-----↑---------↑----											*/
			/*	 MencV.RxPosL[0].w.l MencV.RxTimex.w.h												*/
			/*																						*/
			/*--------------------------------------------------------------------------------------*/
			ulwk = ((ULONG)MencV->RxPosL[0].w.l << 16) + (ULONG)MencV->RxTimex.w.h;
			HenkaItiTimer = (USHORT)(ulwk >> 8);
			/*--------------------------------------------------------------------------------------*/
			/*	Position correction																			*/
			/*																						*/
			/*	Calculation formula:P(t-2.66[us]) = P(t) + (P(t) - P(t-1)) × (Δt-2.66[us])/62.5[us]		*/
			/*																						*/
			/*			(Δt - 2.66[us]) / 62.5[us] = Δt / 62.5[us] - 2.66[us]/62.5[us]			*/
			/*		Encoder clock =6MHzto convert to the number of clocks.			*//* <S004> */
			/*			= (Δt*6[MHz] - 2.66[us]*6[MHz])/(62.5[us]*6[MHz])							*/
			/*																						*/
			/*		1 / (62.5[us]*6[MHz]) -> 1 / SAENCCLKNUM(375)									*/
			/*		2.66[us]*6[MHz] -> ENCLCHCLKNUM(16)												*/
			/*		Δt*6[MHz] -> Change position timer counter number											*/
			/*																						*/
			/*--------------------------------------------------------------------------------------*/
			if(HenkaItiTimer <= ENCLCHCLKNUM)
			{	/* Less than encoder latch period	*//* <S004> */
				swk = (SHORT)(HenkaItiTimer - ENCLCHCLKNUM);
				MotPosCmp = MotPosCmp + ((((MencV->RcvPosX[0] - MencV->MotPosX[0])>>MencV->MposSftR ) * swk) / SAENCCLKNUM);
				if( MotPosCmp >= 0 )
				{
					//no operation 						/* <S004> */
					//MotPosCmp = MotPosCmp;			/* <S004> */
				}
				else
				{
					if(( MencV->AxisMotType == MOTTYPE_LINEAR ))
					{
						MotPosCmp = MotPosCmp + LinEncPos;
					}
					else
					{
						MotPosCmp = MotPosCmp + MencV->PulseNo;
					}
				}
			}
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Post-processing : Return unit	[pulse/rev]	→ [2^32/rev]									*/
		/*------------------------------------------------------------------------------------------*/
		return( MotPosCmp << MencV->MposSftR );

}
#endif /* _TIME_BASE_TIMER_ */



/* <S067> */
/****************************************************************************************************/
/*																									*/
/*		Speed calculation using timebase timer and change time timer										*/
/*																									*/
/*		Overview: Detect speed using encoder information base timer and change time timer				*/
/*			  In the conventional position difference method, the position latch interval is62.5μs, but please check that it is			*/
/*			  In reality, there is a slight deviation due to the latch interval of the encoder.								*/
/*			  This may become a beat and have a negative effect.									*/
/*			  By using encoder timer information, speed detection accuracy can be expected to improve.			*/
/*			  However, if the encoder position difference is less than one pulse, the change time interval becomes abnormally small	*/
/*			  This method is used when the position difference is 2 pulses or more. If there is										*/
/*--------------------------------------------------------------------------------------------------*/
/*			  Execution request flag is setTRUE , speed correction calculation is performed.										*/
/*			  Execution request flag is setFALSEis Vfbtimbase.LastHenkaTime, Vfbtimbase.LastBaseTimer		*/
/*			  																		*/
/*			  Execution request flag is set at power-on and parameter recalculationFALSEbecomes.							*/
/*			  The reason for the new installation isP1mode.		*/
/*			  ActuallyLastModflagCreation also requires one scan, so after turning on the powerScanAstarts running,	*/
/*			  Correction starts from the third scan.													*/
/*																									*/
/*																									*/
/*		arg		:	LONG	dPos	: Encoder position difference			[pulse/scan]					*/
/*					LONG	dPosSpd	: Encoder position difference speed		[2^24/OvrSpd]					*/
/*					UCHAR	EnableFlag: Correction execution permission flag											*/
/*		g-ref	:																					*/
/*		out		:	LONG	motspeed: Motor speed					[2^24/OvrSpd]					*/
/*																									*/
/****************************************************************************************************/
LONG	TimeBasesMotSpdCalc( VFBTIMBASE *Vfbtimbase, MENCV *MencV, LONG dPos, LONG dPosSpd,
																	BOOL VcompSts, BOOL EnableFlag )
{

//	ULONG	ulwk;//<S0C7>
	LONG	lwk;

	/*----------------------------------------------------------------------------------------------*/
	/*	Change time timer acquisition																			*/
	/*																								*/
	/*				  Change position timer																*/
	/*	___________________↓____________________													*/
	/*	|         |/////////|/////////|         |													*/
	/*	-----↑---------↑----													*/
	/*	 MencV.RxPosL[0].w.l MencV.RxTimex.w.h														*/
	/*																								*/
	/*----------------------------------------------------------------------------------------------*/
//		ulwk = ((ULONG)MencV->RxPosL[0].w.l << 16) + (ULONG)MencV->RxTimex.w.h;						  /* <S067> */
//		Vfbtimbase->HenkaItiTimer = (USHORT)(ulwk >> 8);											  /* <S067> */
		Vfbtimbase->HenkaItiTimer = MencV->HenkaItiTimer;
	/*----------------------------------------------------------------------------------------------*/
	/*	Get base timer																			*/
	/*																								*/
	/*				  Base timer																	*/
	/*	___________________↓____________________													*/
	/*	|         |/////////|/////////|         |													*/
	/*	-----↑---------↑----													*/
	/*	  MencV->RxTimex.w.h    MencV->RxTimex.w.l													*/
	/*																								*/
	/*----------------------------------------------------------------------------------------------*/
//		Vfbtimbase->BaseTimer = (USHORT)(MencV->RxTimex.dw >> 8);									  /* <S067> */
		Vfbtimbase->BaseTimer = MencV->BaseTimer;
	/*----------------------------------------------------------------------------------------------*/
	/*	Change time calculation																				*/
	/*----------------------------------------------------------------------------------------------*/
		Vfbtimbase->HenkaTime = Vfbtimbase->BaseTimer - Vfbtimbase->HenkaItiTimer;
		Vfbtimbase->dPos = dPos;							/* This time difference position FB acquisition					*/

	/*----------------------------------------------------------------------------------------------*/
	/*	Correction valid determination																				*/
	/*----------------------------------------------------------------------------------------------*/
		/*------------------------------------------------------------------------------------------*/
		/*	Correction invalid processing																			*/
		/*------------------------------------------------------------------------------------------*/
		if( EnableFlag == FALSE )
		{
			Vfbtimbase->Modflag = FALSE;				/* No correction mode setting							*/
			Vfbtimbase->MotSpeed = dPosSpd;				/* Output without correction							*/
		}
		else
		{
		/*------------------------------------------------------------------------------------------*/
		/*	Processing when correction is valid																			*/
		/*------------------------------------------------------------------------------------------*/
			/*--------------------------------------------------------------------------------------*/
			/*	Change time interval calculation																	*/
			/*--------------------------------------------------------------------------------------*/
			Vfbtimbase->TimeInterval = Vfbtimbase->HenkaTime - Vfbtimbase->LastHenkaTime;
			Vfbtimbase->BaseTimeInterval = Vfbtimbase->BaseTimer - Vfbtimbase->LastBaseTimer;
			/*--------------------------------------------------------------------------------------*/
			/*	Speed calculation execution determination																	*/
			/*--------------------------------------------------------------------------------------*/
			if(MlibABS(dPos) >= VFBTINMINPLS)					/* Differential speed is2More than pulse				*/
			{
				if( Vfbtimbase->ModChgCnt < VFBTINMINCNT )		/* 5times continuously, change position timer method	*/
				{
					Vfbtimbase->ModChgCnt++;
				}
			}
			else
			{
				Vfbtimbase->ModChgCnt = 0;
			}

			/*--------------------------------------------------------------------------------------*/
			/*	Constantly changing position timer execution processing															*/
			/*--------------------------------------------------------------------------------------*/
//			if( (Iprm.TuningLessMode & 0x100)					/* Speed detection switching available				*/
//				&& ( Gprm.f.TuningLessVComp == FALSE) )			/* In use as speed control				*/
			if( VcompSts == FALSE )								/* Speed detection switching available				*/
			{													/* In use as speed control				*/
				if(Vfbtimbase->TimeInterval > SAENCCLKNUMC2)	/* Change time interval is control period×2Exceeding amount	*/
				{
					Vfbtimbase->TimeInterval = SAENCCLKNUMC2;
				}
				else if(MlibAbs32(Vfbtimbase->dPos) <= 1)		/* Current position differenceFBbut1Less than pulse		*/
				{
					if(Vfbtimbase->TimeInterval < SAENCCLKNUM)	/* Change time interval is less than control period		*/
					{
						Vfbtimbase->TimeInterval = SAENCCLKNUM;
					}
				}
				Vfbtimbase->ModChgCnt = VFBTINMINCNT;			/* Constantly changing position timer correction enabled		*/
			}

			if ( (Vfbtimbase->ModChgCnt < VFBTINMINCNT)				/* 5times continuously2Not more than pulse	*/
				|| (Vfbtimbase->TimeInterval < SAENCCLKNUMC2D3 )	/* Change time less than minimum value			*/
				|| (MencV->SeRxErr == TRUE) )						/* Set Error Flag				*/
			{
				Vfbtimbase->Modflag = FALSE;
			}
			else
			{
				Vfbtimbase->Modflag = TRUE;
			}

			if (Vfbtimbase->Modflag && Vfbtimbase->LastModflag)
			{
				/*----------------------------------------------------------------------------------*/
				/* Speed calculation using change position timer													*/
				/*----------------------------------------------------------------------------------*/
				/* Caution:Vfbtimbase->LastModflagThe initial value ofFALSETherefore, the initial power-up speed is	*/
				/*		 Detection becomes a position difference method.Vfbtimbase->LastHenkaTimeis always set	*/
				/*		 After that, the bookifEnter within sentence. ThereforeVfbtimbase->LastHenkaTimeThe initial value of0in	*/
				/*		 Good																		*/
				/*----------------------------------------------------------------------------------*/
				/*----------------------------------------------------------------------------------*/
				/*	Motor speed calculation																	*/
				/*																					*/
				/*					dPosSpd(Encoder position differential speed) * Speed calculation cycle[Base timer clock number]		*/
				/*	motorspeed = ---------------------------------------------------------------	*/
				/*							TimeInterval(Change time interval)[Base timer clock number]				*/
				/*																					*/
				/*	※Base time clock number of speed calculation cycle: Speed calculation cycle:62.5[us]at375Clock(Base timer:6MHz)	*/
				/*																					*/
				/*	motorspeed, you will getDword.					*/
				/*	Therefore, first set the encoder position differential speed to 2bitShift and multiply by the calculation period, divide by the change time	*/
				/*	2bitreturn to its original state before the thyristor turns off.	Speed detection resolution for calculation is 2bitIt falls, but it doesn't really affect it.				*/
				/*	This processing is based on the number of base time clocks in the speed calculation cycle. < 128is,Dword		*/
				/*																					*/
				/*----------------------------------------------------------------------------------*/
				if((Vfbtimbase->dPos == 0)								/* FB = zero this time			*/
					 && (Vfbtimbase->LastdPos > 0))						/* and previous FB≠Zero		*/
				{
					/* Maintain previous speed		*/
				}
				else
				{
					lwk = (dPosSpd >> 2) * SAENCCLKNUM;
					lwk = lwk / (LONG)Vfbtimbase->TimeInterval;
					Vfbtimbase->MotSpeed = lwk << 2;
				}	/*<V334>*/
			}
			else if((Vfbtimbase->BaseTimeInterval != 0)		/* No base timer change time -> Encoder error	*/
				&& (Vfbtimbase->LastSeRxErr == FALSE) )				/* Last Error Flag				*//*<V648>*/
			{
				/*----------------------------------------------------------------------------------*/
				/* Speed calculation using base timer													*/
				/*----------------------------------------------------------------------------------*/
				/*	Motor speed calculation																	*/
				/*																					*/
				/*					dPosSpd(Encoder position differential speed) * Speed calculation cycle[Base timer clock number]		*/
				/*	motorspeed = ---------------------------------------------------------------	*/
				/*						BaseTimeInterval(Base timer change time)[Base timer clock number]			*/
				/*																					*/
				/*	※Base time clock number of speed calculation cycle: Speed calculation cycle:62.5[us]at375Clock(Base timer:6MHz)	*/
				/*																					*/
				/*																					*/
				/*	motorspeed, you will getDword.					*/
				/*	Therefore, first set the encoder position differential speed to 2bitShift and multiply by the calculation period, divide by the change time	*/
				/*	2bitreturn to its original state before the thyristor turns off.	Speed detection resolution for calculation is 2bitIt falls, but it doesn't really affect it.				*/
				/*	This processing is based on the number of base time clocks in the speed calculation cycle. < 128is,Dword		*/
				/*																					*/
				/*----------------------------------------------------------------------------------*/
				lwk = (dPosSpd >> 2) * SAENCCLKNUM;
				lwk = lwk / (LONG)Vfbtimbase->BaseTimeInterval;
				Vfbtimbase->MotSpeed = lwk << 2;
			}
			else
			{
				/*----------------------------------------------------------------------------------*/
				/* If correction condition is not reached: Use encoder position difference velocity calculation								*/
				/*----------------------------------------------------------------------------------*/
				Vfbtimbase->MotSpeed = dPosSpd;
			}
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Save current value																				*/
		/*------------------------------------------------------------------------------------------*/
		Vfbtimbase->LastHenkaTime	= Vfbtimbase->HenkaTime;
		Vfbtimbase->LastBaseTimer	= Vfbtimbase->BaseTimer;
		Vfbtimbase->LastModflag		= Vfbtimbase->Modflag;
		Vfbtimbase->LastdPos		= Vfbtimbase->dPos;
		Vfbtimbase->LastSeRxErr		= MencV->SeRxErr;

		return (Vfbtimbase->MotSpeed);
}



#if 1 /*<S02D> The following functions are provisional(Need to consider when compatible with Fulk) */
#define	SVFSEL_PAOPOUT		1				/* PAO phase pulse output			*//*<S02D>*/
/****************************************************************************************************/
/*																									*/
/*		Fulque encoder: Initial incremental pulse output processing					<S02D>						*/
/*																									*/
/****************************************************************************************************/
#define FMAXPULSEOUT		170
static void	FencSencOutIniIncPulse( MENCV  *FencV, ASIC_HW_IF_STR  *pAsicHwReg )
{
LONG	lwk;
LONG	MtnCur;
LONG	MtnSub;
LONG	MtnRem;
LONG	MtnSepos;
SHORT	MturnLatchCur;
SHORT	PosH;
DWORDX	PosL;

/*--------------------------------------------------------------------------------------------------*/
/*		Communication error check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( FencV->SeRxErr == TRUE )
		{
			return;											/* Do nothing when communication error occurs				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Scale reversal(Pn002.3)Processing (36bitAbsolute value data inversion processing)										*/
/*--------------------------------------------------------------------------------------------------*/
/*	        16bit               20bit              12bit											*/
/*	 |<---------------->|<------------------->|<---------->|   										*/
/*   +------------------+---------------------+------------+   										*/
/*   | FencV->RxPosH[0] |  FencV->RxPosL.dw     xxxxxxxxxxxx| Lower12bitis an indefinite value						*/
/*   +------------------+---------------------+------------+   										*/
/*   |  PosH            |  PosL.dw             000000000000| Lower12bitteeth0Fixed value						*/
/*   +------------------+---------------------+------------+   										*/
/*--------------------------------------------------------------------------------------------------*/
		PosH    = FencV->RxPosH[0];
		PosL.dw = (FencV->RxPosL[0].dw >> FencV->MposSftR) << FencV->MposSftR; /* Lower12bitof0,	*/

		if( FencV->RevAsmMotor )							/* Motor reverse connectionFlagCheck			*/
		{ 
			PosH    = (~PosH);								/* Sign inversion processing							*/
			PosL.dw = (~PosL.dw) + 1;						/* Sign inversion processing							*/
			if( PosL.dw == 0 ){ ++PosH;}					/* Carry, go to the top+1				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Multiturn data acquisition processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( FencV->DivInfo.PaoSeqMcmd == KPX_MTLATCH_REQUEST )	/* Latch request command check		*/
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Absolute position36BitUpper part of data32BitExtract data and calculate multiturn division value		*/
		/*------------------------------------------------------------------------------------------*/
			MtnCur    = ((USHORT)PosH << 16 ) | ((USHORT)PosL.w.h );
			MtnRem    = 0;
			MtnCur    = MlibPfbkxremNolim( MtnCur, FencV->DivInfo.DivOutGain, &MtnRem );
			MtnSepos = (PosH << (32-FSX_FPOS_SFT)) | ((ULONG)PosL.dw >> FSX_FPOS_SFT);
			FencV->DivInfo.DivOutRem = 0;
			MtnSepos =  MlibPfbkxremNolim( MtnSepos, FencV->DivInfo.DivOutGain, &FencV->DivInfo.DivOutRem );
			if( ( (MtnCur>>15) & 0x00000001 ) ^ ( (MtnSepos>>19) & 0x00000001 ) )	/* <V526> */
			{
				if( (MtnCur>>15) & 0x00000001 )
				{
					MtnCur = MtnCur + 1;
				}
				else if( (MtnSepos>>19) & 0x00000001 )
				{
					MtnCur = MtnCur - 1;
				}
			}
			FencV->DivInfo.MturnLatch = (SHORT)( MtnCur >> 16);
			FencV->DivInfo.PsoMturnLatch=(SHORT)( MtnCur >> 16);/*<S189>*/
		/*------------------------------------------------------------------------------------------*/
		/*		Divided pulse reset processing																*/
		/*------------------------------------------------------------------------------------------*/
			FencV->DivInfo.DivOutPos   = 0;				/* Division output pulse reset				*/
			FencV->DivInfo.IncPulseOut = 0;				/* Incre pulse output reset			*/
			FencV->DivInfo.PaoSeqMcmd  = KPX_MTLATCH_FINISH;/* Response ==> FencSencOutputPaoSerial()  */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Initial increment pulse output processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		else if( FencV->DivInfo.PaoSeqMcmd == KPX_IPLSOUT_REQUEST )/* Pulse output command check	*/
		{
		/*------------------------------------------------------------------------------------------*/
		/*	Calculating the change in multiturn data															*/
		/*	Absolute position36BitLower part of data32BitExtract the data and calculate the initial increment frequency division value			*/
		/*	MtnSeposwhile determining the value ofMtnCur.											*/
		/*------------------------------------------------------------------------------------------*/
			MtnCur        = ((USHORT)PosH << 16 ) | ((USHORT)PosL.w.h );
			MtnRem        = 0;
			MtnCur        = MlibPfbkxremNolim( MtnCur, FencV->DivInfo.DivOutGain, &MtnRem );
			MtnSepos = (PosH << (32-FSX_FPOS_SFT)) | ((ULONG)PosL.dw >> FSX_FPOS_SFT);
			FencV->DivInfo.DivOutRem   = 0;				/* Division output pulse remainder reset			*/
			MtnSepos =  MlibPfbkxremNolim( MtnSepos, FencV->DivInfo.DivOutGain, &FencV->DivInfo.DivOutRem );
			if( ( (MtnCur>>15) & 0x00000001 ) ^ ( (MtnSepos>>19) & 0x00000001 ) )
			{
				if( (MtnCur>>15) & 0x00000001 )
				{
					MtnCur = MtnCur + 1;
				}
				else if( (MtnSepos>>19) & 0x00000001 )
				{
					MtnCur = MtnCur - 1;
				}
			}
			MturnLatchCur = (SHORT)( MtnCur >> 16);
			MtnSub = (USHORT)MturnLatchCur - (USHORT)FencV->DivInfo.MturnLatch;
		/*------------------------------------------------------------------------------------------*/
		/*		Absolute position36BitLower part of data32BitExtract the data and calculate the initial increment frequency division value		*/
		/*------------------------------------------------------------------------------------------*/
			MtnSepos = (MtnSepos & 0x000FFFFF );

			if( FencV->DivInfo.RvsDir )						/* Reverse rotation mode check					*/
			{
				MtnSepos = MtnSepos | 0xFFF00000;			/* Sign inversion processing							*/
				MtnSepos = (ULONG)MtnSepos - ( MtnSub << (32-FSX_FPOS_SFT) );
			}
			else
			{
				MtnSepos = (ULONG)MtnSepos + ( MtnSub << (32-FSX_FPOS_SFT) );
			}
		/*------------------------------------------------------------------------------------------*/
		/*		Initial increment pulse output data check & output data setting								*/
		/*------------------------------------------------------------------------------------------*/
			lwk = MtnSepos - FencV->DivInfo.IncPulseOut;
			if( lwk > FMAXPULSEOUT )						/* Pulse output upper limit						*/
			{
				FencV->DivInfo.IncPulseOut += FMAXPULSEOUT;/* Pulse output data setting					*/
			}
			else if( lwk < -FMAXPULSEOUT )					/* Pulse output lower limit						*/
			{
				FencV->DivInfo.IncPulseOut -= FMAXPULSEOUT;/* Pulse output data setting					*/
			}
			else											/* Final pulse output						*/
			{
				FencV->DivInfo.IncPulseOut = MtnSepos;		/* Pulse output data setting					*/
				FencV->DivInfo.PaoSeqMcmd  = KPX_IPLSOUT_FINISH;/* Response ==> FencSencOutputPaoSerial*/
				FencV->DivInfo.IncPulseReq = FALSE;			/* Initial increment pulse output completed			*/
			}
			FencV->DivInfo.DivOutPos = FencV->DivInfo.IncPulseOut;	/* Update of divided output pulse			*/
		/*------------------------------------------------------------------------------------------*/
			pAsicHwReg->AREG_POSET0	= FencV->DivInfo.IncPulseOut;	/* Pulse output circuit initialization			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		No command																				*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			;												/* Waiting for command							*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}

/****************************************************************************************************/
/*																									*/
/*		Fulque encoder: Absolute position data(64bit)Acquisition						<S02D>						*/
/*																									*/
/****************************************************************************************************/
static void FencSencGetAbsPos( MENCV  *FencV, BOOL RvsDir )
{
ULONG	PosL;
USHORT	PosH;
LONG	AbsPos[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Encoder data latch																		*/
/*--------------------------------------------------------------------------------------------------*/
		PosH = FencV->RxPosH[0];						/* Use the data read in scan A,	*/
		PosL = (ULONG)FencV->RxPosL[0].dw;				/* Absolute position(36Bit).				*/

		AbsPos[1] = (LONG)((SHORT)PosH) >> FSX_FPOS_SFT;
		AbsPos[0] = (PosH<<(32-FSX_FPOS_SFT))+((ULONG)PosL>>FSX_FPOS_SFT);

		/*------------------------------------------------------------------------------------------*/
		if( RvsDir ^ FencV->RevAsmMotor )				/* Reverse rotation mode XOR FencReverse rotation mode		*/
		{
			AbsPos[1] = (~AbsPos[1]);					/* Sign inversion processing								*/
			AbsPos[0] = (~AbsPos[0]) + 1;				/* Sign inversion processing								*/
			if( AbsPos[0] == 0 ){ ++AbsPos[1];}			/* Carry, go to the top+1					*/
		}
		FencV->SenOnScalePosH = AbsPos[1];
		FencV->SenOnScalePosL = AbsPos[0];
/*--------------------------------------------------------------------------------------------------*/
		return;
}


/****************************************************************************************************/
/*																									*/
/*		PAO phase multiturn data output								<S02D>							*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Convert the quotient obtained by dividing multiturn data by the specified digit to ASCII characters and output it to the specified port,		*/
/*			Save the remainder after subtracting that digit. The specified digit must be the most significant of the data.				*/
/*																									*/
/*			Example)*data = 32767 of 5th digit(digit=5)										*/
/*																									*/
/*				Output data = '3'																	*/
/*				Save data = 2767																	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static void FencSencOutputPaoMtdata( MENCV *FencV, ASIC_HW_IF_STR *pAsicHwReg, ULONG digit )
{
ULONG	x;
ULONG	outx;

/*--------------------------------------------------------------------------------------------------*/
/*		Digit data conversion																				*/
/*--------------------------------------------------------------------------------------------------*/
		switch( digit )
		{
		case  1: digit = 1;     break;
		case  2: digit = 10;    break;
		case  3: digit = 100;   break;
		case  4: digit = 1000;  break;
		case  5: digit = 10000; break;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Multiturn data output																		*/
/*--------------------------------------------------------------------------------------------------*/
		x = FencV->DivInfo.PaoSeqData / digit;
		outx = MlibEvenAsciiTbl[ x ];
		pAsicHwReg->AREG_UARTXD  = (USHORT)outx;
		FencV->DivInfo.PaoSeqData -= (USHORT)(x * digit);
/*--------------------------------------------------------------------------------------------------*/
		return;

}

/****************************************************************************************************/
/*																									*/
/*		Fulque encoder: PAO phase serial output							<S02D>						*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : When using absolute encoder, multiturn data and alarm by SEN signal				*/
/*			Perform serial output processing from PAO.													*/
/*																									*/
/*			1) Output format : 7bit even parity												*/
/*				 ASIC's TXD has the functions of adding parity and transmitting 7-bit data length					*/
/*				 it is sent as 8-bit data with parity added by software.					*/
/*																									*/
/*			2) Multiturn : P±@@@@@[CR]	( ±@@@@@ : -32768 ~ +32768 )					*/
/*																									*/
/*			3) Alarm		: ALM@@.[CR]		( @@ : A.@@ )									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	FencSencOutputPaoSerial( MENCV *FencV, BPRMDAT *Bprm, ASIC_HW_IF_STR *pAsicHwReg,
							     BOOL DbBeReq, BOOL SenReq, USHORT AlmCode, BOOL AlmFlag )
{
	USHORT	uswk;
	ASIC_HW_IF_STR *pAsicHwReg2;/*<S0C2>*/
	pAsicHwReg2 = (ASIC_HW_IF_STR*)((UCHAR*)pAsicHwReg + 0x1000);/*2-axis pointing address <S0C2>*/


/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SVFSEL_PAOPOUT == 0 )   { return;}				/* Mask Function						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Sequence initialization when servo is on																*/
/*--------------------------------------------------------------------------------------------------*/
		if( DbBeReq == TRUE )
		{
			FencV->DivInfo.PaoSeqStep = 0;
			return;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		PAO serial output sequence																*/
/*--------------------------------------------------------------------------------------------------*/
		switch( FencV->DivInfo.PaoSeqStep )
		{
	/*----------------------------------------------------------------------------------------------*/
		case  0: /* Initial Step																	*/
	/*----------------------------------------------------------------------------------------------*/
				FencV->DivInfo.PaoSeqWait = 0;
				FencV->DivInfo.PaoSeqMcmd = KPX_PAOSEQ_NOCMD;
				if( FencV->PaoseqNormal == TRUE )				/* First operation after power-on?				*/
				{
					FencV->DivInfo.PaoSeqStep = (SenReq == FALSE)? 1 : 20;
				}
				else
				{
					FencV->DivInfo.PaoSeqStep = 1;				/* First time after power-onSEN Lo->Hi	*/
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  1: /* Wait SEN Lo --> Hi (Output Multi-turn Data)										*/
	/*----------------------------------------------------------------------------------------------*/
				FencV->PaoseqNormal = TRUE;						/* Cancel initial operation mode				*/
				FencV->SenReady = FALSE;
				FencV->DivInfo.PaoSeqMcmd = KPX_PAOSEQ_NOCMD;
				FencV->DivInfo.IncPulseReq = TRUE;
				if( SenReq == TRUE )
				{
					/*------------------------------------------------------------------------------*/
					/*		DA output mode: TXD output												*/
					/*------------------------------------------------------------------------------*/
					/*		Bit13 :	DASEL(Divided outputDATerminal selection)	:  0 : DA,  1: TXD_1				*/
					/*------------------------------------------------------------------------------*/
					pAsicHwReg->AREG_PFNCSET  |= BIT13;			/* DA output mode: TXD output		*/
					pAsicHwReg2->AREG_PFNCSET &= ~BIT13;		/*PSO NO way <S0C2>	    */
					/*------------------------------------------------------------------------------*/
					pAsicHwReg->AREG_POSET0	= 0xFFFFFFFF;		/* Pulse output circuit initialization				*/
					pAsicHwReg->AREG_POTEST	= 0xFFFFFFFF;		/* Pulse output circuit initialization				*/

					pAsicHwReg->AREG_DINCSET  |= BIT10;			/* Divide constant clear bit set		*/
					pAsicHwReg->AREG_WDT1SET |= BIT8;			/* Pulse conversion C phase mask ON			*/
					FencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;/* Pulse conversion C phase mask ON		*/

					FencV->DivInfo.PaoSeqStep = 2;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  2: /* Wait 120ms																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( ++FencV->DivInfo.PaoSeqWait > KPI_SCANC_MS( 120 ) )
				{
					pAsicHwReg->AREG_DINCSET |= BIT5;			/* C-phase resynchronization enabled						*/
					pAsicHwReg->AREG_DINCSET &= ~(BIT10);		/* Divide always clear set				*/
					pAsicHwReg->AREG_DINCSET &= ~(BIT5);		/* C-phase resynchronization enabled					*/
					FencV->DivInfo.PaoSeqWait = 0;
					FencV->DivInfo.PaoSeqStep = 3;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  3: /* Multiturn Latch Request															*/
	/*----------------------------------------------------------------------------------------------*/
				FencV->DivInfo.PaoSeqMcmd = KPX_MTLATCH_REQUEST;/* MultiturnLatchCommand ==> ScanA	*/
				FencV->DivInfo.PaoSeqStep = 4;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  4: /* Wait Multiturn Latch															*/
	/*----------------------------------------------------------------------------------------------*/
				if( FencV->DivInfo.PaoSeqMcmd == KPX_MTLATCH_FINISH )	/* MultiturnLatchComplete?		*/
				{
					FencV->DivInfo.PaoSeqData = FencV->DivInfo.MturnLatch;
					FencV->DivInfo.PaoSeqStep = 5;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  5: /* Output 'P'																		*/
	/*----------------------------------------------------------------------------------------------*/
				/*----------------------------------------------------------------------------------*/
				/*	UARTXD:General purpose asynchronous port transmission data: General purpose asynchronous port transmission data bit 7-0   	    */
				/*----------------------------------------------------------------------------------*/
				pAsicHwReg->AREG_UARTXD  = 'P';
				FencV->DivInfo.PaoSeqStep = 6;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  6: /* Output '+' or '-'																*/
	/*----------------------------------------------------------------------------------------------*/
				/*----------------------------------------------------------------------------------*/
				/*	Reverse rotation mode																	*/
				/*----------------------------------------------------------------------------------*/
				if( FencV->DivInfo.RvsDir )					/* Reverse rotation mode						*/
				{
					if( (SHORT)FencV->DivInfo.PaoSeqData >= 0 )
					{
						pAsicHwReg->AREG_UARTXD  = '-';
						FencV->DivInfo.PaoSeqData = FencV->DivInfo.PaoSeqData + 1;
					}
					else
					{
						pAsicHwReg->AREG_UARTXD  = '+';
						FencV->DivInfo.PaoSeqData = ~FencV->DivInfo.PaoSeqData;
					}
				}
				/*----------------------------------------------------------------------------------*/
				/*	Positive rotation mode																	*/
				/*----------------------------------------------------------------------------------*/
				else
				{
					if( (SHORT)FencV->DivInfo.PaoSeqData >= 0 )
					{
						pAsicHwReg->AREG_UARTXD  = '+';
					}
					else
					{
						pAsicHwReg->AREG_UARTXD  = '-';
						FencV->DivInfo.PaoSeqData = ~FencV->DivInfo.PaoSeqData + 1;
					}
				}
				FencV->DivInfo.PaoSeqStep = 7;
				break;
	/*----------------------------------------------------------------------------------------------*/
	/*			Decimal data output : 5th digit,4th digit,3rd digit,2nd digit,1st digit								*/
	/*----------------------------------------------------------------------------------------------*/
		case  7: FencSencOutputPaoMtdata( FencV, pAsicHwReg, 5 ); FencV->DivInfo.PaoSeqStep =  8; break;
		case  8: FencSencOutputPaoMtdata( FencV, pAsicHwReg, 4 ); FencV->DivInfo.PaoSeqStep =  9; break;
		case  9: FencSencOutputPaoMtdata( FencV, pAsicHwReg, 3 ); FencV->DivInfo.PaoSeqStep = 10; break;
		case 10: FencSencOutputPaoMtdata( FencV, pAsicHwReg, 2 ); FencV->DivInfo.PaoSeqStep = 11; break;
		case 11: FencSencOutputPaoMtdata( FencV, pAsicHwReg, 1 ); FencV->DivInfo.PaoSeqStep = 12; break;
	/*----------------------------------------------------------------------------------------------*/
		case 12: /* Output '0x0D'																	*/
	/*----------------------------------------------------------------------------------------------*/
				pAsicHwReg->AREG_UARTXD  = (0x0d + 0x80);
				FencV->DivInfo.PaoSeqStep = 13;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 13: /* Wait 2ms																		*/
	/*----------------------------------------------------------------------------------------------*/
				FencV->DivInfo.PaoSeqStep = 14;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 14: /* IncPulse Output Request															*/
	/*----------------------------------------------------------------------------------------------*/
				/*----------------------------------------------------------------------------------*/
				/*		DA output mode: Pulse output										   		    */
				/*----------------------------------------------------------------------------------*/
				/*		Bit13 :	DASEL(Divided outputDATerminal selection)	:  0 : DA,  1: TXD_1				    */
				/*----------------------------------------------------------------------------------*/
				pAsicHwReg->AREG_PFNCSET  &= ~BIT13;			/* DA output mode: Pulse output		*/
				/*----------------------------------------------------------------------------------*/
				FencV->DivInfo.PaoSeqMcmd = KPX_IPLSOUT_REQUEST;/* Initial incremental output command ==> ScanA	*/
				FencV->DivInfo.PaoSeqStep = 15;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 15: /* Check IncPulse Output Complete													*/
	/*----------------------------------------------------------------------------------------------*/
				if( FencV->DivInfo.PaoSeqMcmd == KPX_IPLSOUT_FINISH )	/* Initial increment output completed?	*/
				{
					FencV->SenReady = TRUE;
					FencSencGetAbsPos( FencV, Bprm->RvsDir );	/* Absolute position data acquisition	*/
					pAsicHwReg->AREG_WDT1SET &= ~BIT8;			/* Pulse conversion C phase mask OFF  			*/
					FencV->DivInfo.DivCMaskStatus  = DIV_CMASK_OFF ;
					FencV->DivInfo.PaoSeqStep = 0;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 20: /* Wait SEN Hi --> Lo (Output Alarm)												*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					FencV->SenReady = FALSE;
					FencV->DivInfo.PaoSeqMcmd = KPX_PAOSEQ_NOCMD;
					FencV->DivInfo.IncPulseReq = TRUE;
					if( AlmFlag )								/* Alarm occurrence					*/
					{
						pAsicHwReg->AREG_PFNCSET  |= BIT13;		/* DA output mode: TXD output		*/
						pAsicHwReg->AREG_POSET0	= 0xFFFFFFFF;	/* Pulse output circuit initialization				*/
						pAsicHwReg->AREG_POTEST	= 0xFFFFFFFF;	/* Pulse output circuit initialization				*/

						FencV->DivInfo.PaoSeqData = AlmCode;	/* Alarm code acquisition				*/
						FencV->DivInfo.PaoSeqStep = 21;			/* Transition to alarm output step		*/
					}
					else										/* No alarm occurrence					*/
					{
						FencV->DivInfo.PaoSeqStep =  1;			/* Lo-->HiTransition to monitoring step		*/
					}
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 21: /* Wait 120ms																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					if( ++FencV->DivInfo.PaoSeqWait > KPI_SCANC_MS( 120 ) )
					{
						FencV->DivInfo.PaoSeqWait = 0;
						FencV->DivInfo.PaoSeqStep = 22;
					}
				}
				else
				{
					FencV->DivInfo.PaoSeqWait = 0;
					FencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 22: /* Output 'A'																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					pAsicHwReg->AREG_UARTXD  = 'A';
					FencV->DivInfo.PaoSeqStep = 23;
				}
				else
				{
					FencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 23: /* Output 'L'																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					pAsicHwReg->AREG_UARTXD  = ('L' + 0x80);
					FencV->DivInfo.PaoSeqStep = 24;
				}
				else
				{
					FencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 24: /* Output 'M'																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					pAsicHwReg->AREG_UARTXD  = 'M';
					FencV->DivInfo.PaoSeqStep = 25;
				}
				else
				{
					FencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 25: /* Output Alarm Code																*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					uswk = MlibEvenAsciiTbl[((FencV->DivInfo.PaoSeqData >> 8) & 0x0F)];
					pAsicHwReg->AREG_UARTXD  = uswk;
					FencV->DivInfo.PaoSeqStep = 26;
				}
				else
				{
					FencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 26: /* Output Alarm Code																*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					uswk = MlibEvenAsciiTbl[((FencV->DivInfo.PaoSeqData>> 4) & 0x0F)];
					pAsicHwReg->AREG_UARTXD  = uswk;
					FencV->DivInfo.PaoSeqStep = 27;
				}
				else
				{
					FencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 27: /* Output '.'																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					pAsicHwReg->AREG_UARTXD  = '.';
					FencV->DivInfo.PaoSeqStep = 28;
				}
				else
				{
					FencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 28: /* Output 0x0D																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					pAsicHwReg->AREG_UARTXD  = (0x0d + 0x80);
					FencV->DivInfo.PaoSeqStep = 29;
				}
				else
				{
					FencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 29: /* End of Output Alarm																*/
	/*----------------------------------------------------------------------------------------------*/
				pAsicHwReg->AREG_PFNCSET  &= ~BIT13;			/* DA output mode: Pulse output		*/
				FencV->DivInfo.PaoSeqStep = 1;
				break;
	/*----------------------------------------------------------------------------------------------*/
		default: /* Default																			*/
	/*----------------------------------------------------------------------------------------------*/
				FencV->DivInfo.PaoSeqStep = 0;
				break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;

}
#endif /*<S02D>*/

/* <S1B7> >>>>> */
/*****************************************************************************
  Description : Motorless: Encoder variable initialization processing
----------------------------------------------------------------------------
  Parameters:
	MencV;					Pointer to motor encoder data
	AsicHwReg;				Asic H/W Register address
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void LpxSencMotLessInitVariables( MENCV *MencV, ASIC_HW_IF_STR *AsicHwReg )
{

	/*----------------------------------------------------------------------*/
	/*	Initial position data(This time,Previous,Two previous times)								*/
	/*----------------------------------------------------------------------*/
	MencV->MotPos  = 0;				/* Motor Position	[32bitRing]			*/
	MencV->MotPhaX = 0;				/* Motor PhasePos.	[PulseNo/360deg]	*/
	MencV->MotCposX = 0;			/* Motor CphasePos.	[32bitRing]			*/
	/*----------------------------------------------------------------------*/
	MencV->RxPosH[1]  = MencV->RxPosH[0] = 0;	/* Previous increment correction value initial setting	*/
	MencV->RcvPosX[0] = MencV->RcvPosX[1] = MencV->RcvPosX[2] = 0;
	MencV->MotPosX[0] = MencV->MotPosX[1] = 0;

	/*----------------------------------------------------------------------*/
	/*	Initial absolute position data													*/
	/*----------------------------------------------------------------------*/
	MencV->MotAbsPos = 0;

	/*----------------------------------------------------------------------*/
	/*	Set initial multiturn data										*/
	/*----------------------------------------------------------------------*/
	MencV->IniMturn = 0;
	MencV->IniIncre = 0;

	/*----------------------------------------------------------------------*/
	/*	Initialize the phase origin													*/
	/*----------------------------------------------------------------------*/
	MencV->MotOrgX  = 0;			/* Motor origin							*/
	MencV->MotCposX = 0;			/* Motor C phase position						*/
	MencV->MotAngle = 0;			/* Motor rotation angle(Pulse)					*/
	MencV->PhaseOffset = 0;			/* Pole sensor offset amount 			*/

	/*----------------------------------------------------------------------*/
	/*	Initialization of motor origin check variables									*/
	/*----------------------------------------------------------------------*/
	MencV->MotOrgChkEnd = TRUE;		/* Set Motor Origin Check End Flag		*/

	/*----------------------------------------------------------------------*/
	/*	Divided pulse initial settings													*/
	/*----------------------------------------------------------------------*/
	MencV->DivInfo.DivOutRem = 0;
	MencV->DivInfo.DivOutPos = 0;
	MencV->DivInfo.InitDivPosOffset = 0;
	if( MencV->DivInfo.DivOut == TRUE )
	{
		AsicHwReg->AREG_POSET0 = (USHORT)MencV->DivInfo.DivOutPos;
		AsicHwReg->AREG_POTEST = (USHORT)MencV->DivInfo.DivOutPos;
	}

	/*----------------------------------------------------------------------*/
	/*	C phase flag related														*/
	/*----------------------------------------------------------------------*/
	MencV->Cset = TRUE;				/* C phase settingFlag							*/
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{
		MencV->CphaseLess = TRUE;	/* C phase lessFlag							*/
	}
//[org]	KPI_PCMASK_OFF( );					/* Pulse conversionCPhase maskOFFSetting	*/
	AsicHwReg->AREG_WDT1SET &= ~BIT8;		/* TODO: Is it necessary? !				*/
	MencV->DivInfo.DivCMaskStatus = DIV_CMASK_OFF;	/* Pulse conversionCPhase maskOFFSetting	*/

	/*----------------------------------------------------------------------*/
	/*	Encoder ready setting												*/
	/*----------------------------------------------------------------------*/
	MencV->PhaseReady = TRUE;		/* Phase detection completedFlag						*/

	return;
}


/*****************************************************************************
  Description : Motor-less: Reading rotational position data
----------------------------------------------------------------------------
  Parameters:
	MencV;					Pointer to motor encoder data
	pAsicHwReg;				Asic H/W Register address
	AlmManager;				Pointer to alarm management IF data definition
	BeComplete;				Base enable completion status
	BaseLoops;				Pointer to data for control loop calculation
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void RmxReadMotorLessPosition( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg,
									ALMMAN_HANDLE *AlmManager, BOOL BeComplete, void *BaseLoops )
{
	BPRMDAT		*Bprm;					/* Pointer to basic parameter definition			*/
	MENCPRM		*MencP;					/* Pointer to encoder parameter definition		*/
	MOTLESSTST	*MotorLessTest;			/* Motorless test modeIFPointer to definition	*/
	MOTOR_LESS	*MotorLessTestDatas;	/* EncoderIF			*/
	SENCX		*Enc;					/* Serial encoderIFPointer to definition		*/

	LONG		dEncPos;				/* Encoder position difference				*/
	LONG		dMotPos;				/* Position difference							*/
	LONG		RcvPosX;				/* Encoder current position				*/
	LONG		LastMotPosX;			/* Previous position							*/
	SHORT		Mturn;					/* Multiturn						*/

	/* Initializing variables	*/
	Bprm				= ((BASE_LOOPCTRLS*)BaseLoops)->Bprm;
	MotorLessTest		= ((BASE_LOOPCTRLS*)BaseLoops)->MotorLessTest;
	MencP				= &(MotorLessTest->Prm->MencP);
	MotorLessTestDatas	= &(MotorLessTest->MotorLessTestDatas);
	Enc					= &(MencV->EncIf);

	LastMotPosX = MencV->MotPosX[0];

	/*----------------------------------------------------------------------*/
	/*	Position data setting function execution for motorless test mode						*/
	/*----------------------------------------------------------------------*/
	ApiSetMechaDatasMotorLessTestMode( Bprm, MotorLessTest );

	/*----------------------------------------------------------------------*/
	/*	Saving previous position data												*/
	/*----------------------------------------------------------------------*/
	MencV->RxPosL[1] = MencV->RxPosL[0];			/* Previous value updated			*/
	MencV->RxPosH[1] = MencV->RxPosH[0];			/* Previous value updated			*/

	/*----------------------------------------------------------------------*/
	/*  Obtain position difference data from motor model section								*/
	/*----------------------------------------------------------------------*/
	dEncPos = (LONG)MotorLessTestDatas->dSemiModePos << MencV->MposSftX;
	MlibAdd6432( MotorLessTestDatas->SencData.Long, dEncPos, MotorLessTestDatas->SencData.Long );

	/*----------------------------------------------------------------------*/
	/*  Multiturn limit processing											*/
	/*----------------------------------------------------------------------*/
//	if( MencV->AxisMotType == MOTTYPE_ROTARY )
//	{
//	}

	if( MencP->limmlt != 65535 )
	{
		Mturn = LpxGetSencMotLessMturnData( MencV, MencP,
											MotorLessTestDatas->SencData.EncFormat.RxPosH,
											MotorLessTestDatas->SencData.EncFormat.RxPosL.dw );
		if( Mturn > MencP->limmlt )
		{
			/* When exceeding multiturn limit ⇒ To multiturn data0	*/
			LpxSencMotLessMultiTurnSet( MencP, &MotorLessTestDatas->SencData, 0 );
		}
		else if( Mturn < 0 )
		{
			/* If multiturn is negative ⇒ Set multiturn limit to multiturn data		*/
			LpxSencMotLessMultiTurnSet( MencP, &MotorLessTestDatas->SencData, MencP->limmlt );
		}
	}

	/*----------------------------------------------------------------------*/
	/*  Set position data to encoder received data							*/
	/*----------------------------------------------------------------------*/
	MencV->RxPosL[0].dw = MotorLessTestDatas->SencData.EncFormat.RxPosL.dw;
	MencV->RxPosH[0]    = MotorLessTestDatas->SencData.EncFormat.RxPosH;

	/*----------------------------------------------------------------------*/
	/*	Position calculation															*/
	/*----------------------------------------------------------------------*/
	RcvPosX = MencV->MposSign * ((MencV->RxPosL[0].Long>>MencV->MposSftX)<<MencV->MposSftR);

	/*----------------------------------------------------------------------*/
	/*	Current position data, previous position data updated									*/
	/*----------------------------------------------------------------------*/
	MencV->SeRxErr    = FALSE;				/* Reset Error Flag				*/
	MencV->EncDataNG  = FALSE;				/* Reset Encoder Data NG Flag	*/
	MencV->RcvPosX[1] = MencV->RcvPosX[0];	/* Previous position data				*/
	MencV->RcvPosX[0] = RcvPosX;			/* Current position data				*/

	/*----------------------------------------------------------------------*/
	/*	Motor position output calculation													*/
	/*----------------------------------------------------------------------*/
	MencV->MotPosX[1] = MencV->RcvPosX[1];	/* Previous position data				*/
	MencV->MotPosX[0] = MencV->RcvPosX[0];	/* Current position data				*/

	/*----------------------------------------------------------------------*/
	/*	Motor position output(32bitRingPulse)										*/
	/*----------------------------------------------------------------------*/
	dMotPos = ( (MencV->MotPosX[0] - LastMotPosX) >> MencV->MposSftR );
	MencV->MotPos    += dMotPos;
	MencV->MotAbsPos += dMotPos;

	// Move from below
	/*----------------------------------------------------------------------*/
	/*	Motor current phase update													*/
	/*----------------------------------------------------------------------*/
	MencV->MotPhase = (USHORT)(( (((MencV->MotPosX[0]) - (MencV->MotOrgX)) >>8) * MencV->Kmotphase)>>8);

	/*----------------------------------------------------------------------*/
	/*	Feedback pulse output processing										*/
	/*----------------------------------------------------------------------*/
	//	This processMercury
	/* <S20D> Start */
	if( MencV->DivInfo.IncPulseReq == TRUE )	/* Initial incremental output request?	*/
	{
		if( MencV->AxisMotType == MOTTYPE_LINEAR )
		{ /* Linear type */
			/* Initial increment pulse output processing */
			LmxOutSencIniIncPulse(MencV, pAsicHwReg, LMX_MPOS_SFT);
		}
		else
		{ /* Rotation type */
			/* Initial increment pulse output processing */
			SencOutIniIncPulse( MencV, pAsicHwReg );
		}
	}

	if((MencV->DivInfo.DivOutASIC != FALSE) && (MencV->DivInfo.IncPulseReq == FALSE))
	{ /* JL-086, after completion of initial increpulse outputJL-086 */
		Enc->SencSetReg->AREG_DIVSET &= ~BIT1;
	}
	/* <S20D> End */

	/*----------------------------------------------------------------------*/
	/*	Origin passing check process												*/
	/*----------------------------------------------------------------------*/
	if( (MencV->OrgOk == FALSE) && (MencV->SeRxErr == FALSE) )
	{
		RmxCheckSencOriginPass( MencV );		/* Origin passing check	*/
	}

//	/*----------------------------------------------------------------------*/
//	/*	Motor current phase update													*/
//	/*----------------------------------------------------------------------*/
//	// Move upward

	/*----------------------------------------------------------------------*/
	/*	Division pulse output processing													*/
	/*----------------------------------------------------------------------*/
	// No initial increment output
	if( MencV->DivInfo.DivOut == TRUE )
	{
		SHalDrv_SetDivOutPosition( Enc->SencSetReg, (MencV->MotPosX[0] >> MencV->DivInfo.DivOutSft) );
	}
	/*----------------------------------------------------------------------*/
	return;

}


/*****************************************************************************
  Description : Motor-less: Reading linear position data
----------------------------------------------------------------------------
  Parameters:
	MencV;					Pointer to motor encoder data
	pAsicHwReg;				Asic H/W Register address
	AlmManager;				Pointer to alarm management IF data definition
	BeComplete;				Base enable completion status
	BaseLoops;				Pointer to data for control loop calculation
  Return:
	none
----------------------------------------------------------------------------
  Note:
	Tentative version(Creating)
*****************************************************************************/
void LmxReadMotorLessPosition( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg,
									ALMMAN_HANDLE *AlmManager, BOOL BeComplete, void *BaseLoops )
{
	BPRMDAT		*Bprm;					/* Pointer to basic parameter definition			*/
	MOTLESSTST	*MotorLessTest;			/* Motorless test modeIFPointer to definition	*/
	MOTOR_LESS	*MotorLessTestDatas;	/* EncoderIF			*/
	SENCX		*Enc;					/* Serial encoderIFPointer to definition		*/

	LONG		dEncPos;				/* Encoder position difference				*/
	LONG		dMotPos;				/* Position difference							*/
	LONG		RcvPosX;				/* Encoder current position				*/
	LONG		LastMotPosX;			/* Previous position							*/

	/* Initializing variables	*/
	Bprm				= ((BASE_LOOPCTRLS*)BaseLoops)->Bprm;
	MotorLessTest		= ((BASE_LOOPCTRLS*)BaseLoops)->MotorLessTest;
	MotorLessTestDatas	= &(MotorLessTest->MotorLessTestDatas);
	Enc					= &(MencV->EncIf);

	LastMotPosX = MencV->MotPosX[0];

	/*----------------------------------------------------------------------*/
	/*	Position data setting function execution for motorless test mode						*/
	/*----------------------------------------------------------------------*/
	ApiSetMechaDatasMotorLessTestMode( Bprm, MotorLessTest );

	/*----------------------------------------------------------------------*/
	/*	Saving previous position data												*/
	/*----------------------------------------------------------------------*/
	MencV->RxPosL[1] = MencV->RxPosL[0];			/* Previous value updated			*/
	MencV->RxPosH[1] = MencV->RxPosH[0];			/* Previous value updated			*/

	/*----------------------------------------------------------------------*/
	/*  Obtain position difference data from motor model section								*/
	/*----------------------------------------------------------------------*/
//TODO:(Sigma-V)	dEncPos = (LONG)MotorLessTestDatas->dSemiModePos << MencV->MposSftLm;
	dEncPos = (LONG)MotorLessTestDatas->dSemiModePos << MencV->MposSftX;
	MlibAdd6432( MotorLessTestDatas->SencData.Long, dEncPos, MotorLessTestDatas->SencData.Long );

	/*----------------------------------------------------------------------*/
	/*  Set position data to encoder received data							*/
	/*----------------------------------------------------------------------*/
	MencV->RxPosL[0].dw = MotorLessTestDatas->SencData.EncFormat.RxPosL.dw;
	MencV->RxPosH[0]    = MotorLessTestDatas->SencData.EncFormat.RxPosH;

	/*----------------------------------------------------------------------*/
	/*	Position calculation															*/
	/*----------------------------------------------------------------------*/
//TODO:(Sigma-V)	RcvPosX = MencV->MposSign * ((MencV->RxPosL[0].Long>>MencV->MposSftLm)<<MencV->MposSftLm);
	RcvPosX = MencV->MposSign * ((MencV->RxPosL[0].Long>>MencV->MposSftX)<<MencV->MposSftR);

	/*----------------------------------------------------------------------*/
	/*	Current position data, previous position data updated									*/
	/*----------------------------------------------------------------------*/
	MencV->SeRxErr    = FALSE;				/* Reset Error Flag				*/
	MencV->EncDataNG  = FALSE;				/* Reset Encoder Data NG Flag	*/
	MencV->RcvPosX[1] = MencV->RcvPosX[0];	/* Previous position data				*/
	MencV->RcvPosX[0] = RcvPosX;			/* Current position data				*/

	/*----------------------------------------------------------------------*/
	/*	Motor position output calculation													*/
	/*----------------------------------------------------------------------*/
	MencV->MotPosX[1] = MencV->RcvPosX[1];	/* To match processing with rotary type	*/
	MencV->MotPosX[0] = MencV->RcvPosX[0];	/* To match processing with rotary type	*/
	/*----------------------------------------------------------------------*/
//TODO:(Sigma-V)	dMotPos = (MencV.MotPosX[0] - LastMotPosX) >> MencV->MposSftLm;
	dMotPos = ( (MencV->MotPosX[0] - LastMotPosX) >> MencV->MposSftR );

	MencV->MotPos    += dMotPos;	/* Motor Position (32bitRingPulse)		*/
	MencV->MotAbsPos += dMotPos;	/* Motor Abs Position (32bitRingPulse)	*/

	/*----------------------------------------------------------------------*/
	/*	Motor current phase update													*/
	/*----------------------------------------------------------------------*/
	LmxUpdateMotPhase( MencV, AlmManager, dMotPos );

	/*----------------------------------------------------------------------*/
	/*	Initial increment pulse output processing or Encoder C phase passage check processing		*/
	/*----------------------------------------------------------------------*/
	if( MencV->DivInfo.IncPulseReq == TRUE )		/* Initial incremental output request?		*/
	{
//TODO:(Sigma-V)		LmxOutSencIniIncPulse( MencV->MposSftLm );	/* Initial increment pulse output processing	*/
	}
	else
	{
		if( MencV->SeRxErr == FALSE )
		{
			LmxCheckSencCphasePass( MencV );	/* Encoder C phase passage check processing	*/
		}
		/*------------------------------------------------------------------*/
		/*	Divided pulse output													*/
		/*------------------------------------------------------------------*/
		if( MencV->DivInfo.DivOut == TRUE )
		{
			MencV->DivInfo.DivOutPos += MlibPfbkxremNolim( dMotPos, MencV->DivInfo.DivOutGain, &MencV->DivInfo.DivOutRem );
			SHalDrv_SetDivOutPosition( Enc->SencSetReg, MencV->DivInfo.DivOutPos );
		}
	}
	/*----------------------------------------------------------------------*/
	return;
}


/*****************************************************************************
  Description : Motorless: Encoder parameter setting processing
	Set encoder information after setting virtual motor parameters when encoder is not connected
----------------------------------------------------------------------------
  Parameters:
	MencV;					Pointer to motor encoder data
	MencP;					Pointer to encoder parameter definition
	b_prm2;					Function selection application switch 2
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void LpxSetSencInfoMotorLessTestMode( MENCV *MencV, MENCPRM *MencP, USHORT b_prm2 )
{

	/*----------------------------------------------------------------------*/
	/*	Select encoder type when encoder is not connected								*/
	/*----------------------------------------------------------------------*/
	if( MencP->typm_e.b.h & 0x10 )
	{
		MencV->AbsoEncoder = TRUE;
//TODO:		SencSetAbsoEncType(MencV, b_prm2, MencP->typm_e.w);
// I want to use the above function, butMotorPrmCfg.candstaticSince it is declared and cannot be used, it is expanded and used.
		if( 1 == ((b_prm2>>8) & 0x0F) )
		{	/* Using absolute as increment	*/
			MencV->AbsoEncType = MENC_INCRE_TYPE;
		}
		else
		{	/* Using absolute as absolute		*/
			MencV->AbsoEncType = (MencP->typm_e.w & 0x1000)? MENC_MABSO_TYPE : MENC_SABSO_TYPE;
		}
	}
	else
	{
		MencV->AbsoEncoder = FALSE;
		MencV->AbsoEncType = MENC_INCRE_TYPE;
	}
	/*----------------------------------------------------------------------*/
	/*	Disable serial converter scale pitch information						*/
	/*----------------------------------------------------------------------*/
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{
		MencV->ScalePitchExp = 0xFF;
	}

		return;
}


/*****************************************************************************
  Description : Motorless: Multiturn data acquisition processing
	Multi-turn data using decimal point position from encoder data(16bit)near the maximum rotation.
----------------------------------------------------------------------------
  Parameters:
	MencV;					Pointer to motor encoder data
	MencP;					Pointer to encoder parameter definition
	PosH;					Location data(H)
	PosL;					Location data(L)
  Return:
	Multiturn data(16bit)
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
static USHORT LpxGetSencMotLessMturnData( MENCV *MencV, MENCPRM *MencP, USHORT PosH, ULONG PosL )
{
	USHORT	Mturn;						/* Multiturn data	*/

	Mturn = (USHORT)(((PosH<<16) | ((ULONG)PosL>>16)) >> (MencP->bit_dp.b.h - 64));
	if( MencV->RevAsmMotor )
	{
		if( MencP->limmlt == 65535 )
		{
			Mturn = -Mturn;
		}
		else if( Mturn != 0 )
		{
			Mturn = MencP->limmlt - Mturn;
		}
	}

	return( Mturn );

}


/*****************************************************************************
  Description : Motor-less: Multi-turn data setting processing
	Considering multiturn limit, set multiturn data in encoder data
----------------------------------------------------------------------------
  Parameters:
	MencP;					Pointer to encoder parameter definition
	EncData;				Pointer to encoder data
	MturnLimit;				Multiturn data value
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
static void LpxSencMotLessMultiTurnSet(MENCPRM *MencP, ENCODER_DATA *EncData, SHORT MturnLimit)
{
	DWORDX	MturnData;					/* Multiturn data	*/
	UCHAR	Dpoint;						/* Position data decimal point position	*/
	UCHAR	Sftx;						/* Number of shifts				*/

	/* Initializing variables */
	Dpoint = MencP->bit_dp.b.h;
	Sftx   = 80 - Dpoint;

	/*----------------------------------------------------------------------*/
	/*	Multiturn data calculation												*/
	/*----------------------------------------------------------------------*/
	MturnData.dw = MturnLimit << (Dpoint-64);

	/*----------------------------------------------------------------------*/
	/*	Multiturn area clear												*/
	/*----------------------------------------------------------------------*/
	EncData->EncFormat.RxPosH = 0;
	EncData->EncFormat.RxPosL.Ulong = ( EncData->EncFormat.RxPosL.Ulong << Sftx) >> Sftx;

	/*----------------------------------------------------------------------*/
	/*	Multiturn area data set										*/
	/*----------------------------------------------------------------------*/
	EncData->EncFormat.RxPosH     |= MturnData.w.h;
	EncData->EncFormat.RxPosL.w.h |= MturnData.w.l;

	return;
}


/*****************************************************************************
  Description : Motorless: Encoder variable initialization processing
----------------------------------------------------------------------------
  Parameters:
	FencV;					Pointer to Fulque encoder data
	AsicHwReg;				Asic H/W Register address
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void LpxFencMotLessInitVariables( MENCV *FencV, ASIC_HW_IF_STR *AsicHwReg )
{

	/*----------------------------------------------------------------------*/
	/*	Initial position data(This time,Previous)										*/
	/*----------------------------------------------------------------------*/
	FencV->MotPos = 0;						/* Fulke position [32bitRingPulse]	*/
	/*----------------------------------------------------------------------*/
	FencV->RxPosH[1] = FencV->RxPosH[0] = 0;
	FencV->MotPosX[0] = 0;
	FencV->MotPosX[1] = 0;

	/*----------------------------------------------------------------------*/
	/*	Initial absolute position data													*/
	/*----------------------------------------------------------------------*/
	FencV->MotAbsPos = 0;

	/*----------------------------------------------------------------------*/
	/*	Divided pulse initial settings													*/
	/*----------------------------------------------------------------------*/
	FencV->DivInfo.DivOutRem = 0;
	FencV->DivInfo.DivOutPos = 0;

	/*----------------------------------------------------------------------*/
	/*	C phase flag related														*/
	/*----------------------------------------------------------------------*/
	FencV->Cset = TRUE;						/* C phase settingFlag					*/
	FencV->CposOk = TRUE;					/* C-phase position setting completedFlag			*/
	FencV->CphaseLess = TRUE;
	/*----------------------------------------------------------------------*/
	if( FencV->DivInfo.DivOut == TRUE )
	{
		AsicHwReg->AREG_POSET0 = (USHORT)FencV->DivInfo.DivOutPos;
		AsicHwReg->AREG_POTEST = (USHORT)FencV->DivInfo.DivOutPos;
	}

	/*----------------------------------------------------------------------*/
	/*	Encoder ready setting												*/
	/*----------------------------------------------------------------------*/
	//No processing
	/*----------------------------------------------------------------------*/

	/*----------------------------------------------------------------------*/
	/*	Select function presence/absence: Increpulse output request reset						*/
	/*----------------------------------------------------------------------*/
	//No processing
	/*----------------------------------------------------------------------*/

	return;
}


/*****************************************************************************
  Description : Motorless: Encoder parameter setting processing
	Set encoder information when encoder is not connected
----------------------------------------------------------------------------
  Parameters:
	FencV;					Pointer to Fulque encoder data
	MencP;					Pointer to encoder parameter definition
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void LpxSetFencInfoMotorLessTestMode( MENCV *FencV, MENCPRM *MencP )
{
	/*----------------------------------------------------------------------*/
	/*	Encoder bit number, decimal point position										*/
	/*----------------------------------------------------------------------*/
	//No processing
	/*----------------------------------------------------------------------*/
	/*	Select encoder type when encoder is not connected								*/
	/*----------------------------------------------------------------------*/
	FencV->AbsoEncoder = FALSE;

	/*----------------------------------------------------------------------*/
	/*	Absolute value encoder usage setting										*/
	/*----------------------------------------------------------------------*/
	FencV->AbsoEncType = MENC_INCRE_TYPE;

	/*----------------------------------------------------------------------*/
	/*	EnDat type selection												*/
	/*----------------------------------------------------------------------*/
	//FencP.f.EndatType = FALSE;

	/*----------------------------------------------------------------------*/
	/*	Encoder type/resolution setting										*/
	/*----------------------------------------------------------------------*/
	FencV->EncType = (0x0000 | MencP->BitnoDpoint.b.l);

	return;
}


/*****************************************************************************
  Description : Motor-less: Reading position data
----------------------------------------------------------------------------
  Parameters:
	FencV;					Pointer to Fulque encoder data
	AsicHwReg;				Asic H/W Register address
	Axis;					Axis handle
  Return:
	Location data
----------------------------------------------------------------------------
  Note:
	Tentative version(Creating)
*****************************************************************************/
LONG LpxReadMotorLessFencPosition( MENCV *FencV, ALMMAN_HANDLE *AlmManager, BOOL BeComplete, void *BaseLoops )
{
	MOTLESSTST	*MotorLessTest;
	MOTOR_LESS	*MotorLessTestDatas;	/* EncoderIF	*/

	LONG		dEncPos;				/* Encoder position difference				*/
	LONG		dFencPos;				/* Position difference							*/
	LONG		FencSpd; 				/* ExternalPGSpeed						*/

	ASIC_HW_IF_STR *sencSetReg;

	/* Initializing variables	*/
	MotorLessTest		= ((BASE_LOOPCTRLS*)BaseLoops)->MotorLessTest;
	MotorLessTestDatas	= &(MotorLessTest->MotorLessTestDatas);
	sencSetReg			= FencV->EncIf.SencSetReg;

	/*----------------------------------------------------------------------*/
	/*	Parameter reset(Recalculation)Medium										*/
	/*----------------------------------------------------------------------*/
	/* dFencPos = 0  		*/
	//if( FnCmnCtrl->KnlMode == KPI_KMD_PARAMRESET ){ return( 0 );}
	// KPI_KMD_PARAMRESETis not set
	// FnCmnCtrl *FnCmnCtrl : AxisRsc->FnCmnCtrl

	/*--------------------------------------------------------------------------*/
	/*	Reception completionFlag(IntSync Set --> ScanB Clear)/Receive error counter reset	*/
	/*--------------------------------------------------------------------------*/
	FencV->MstErrCnt = 0;						/* Reset Master Error Counter	*/
	FencV->SeRxErr = FALSE;						/* Reset Error Flag				*/
	FencV->EncDataNG = FALSE;					/* Reset Encoder Data NG Flag	*/
	/*--------------------------------------------------------------------------*/
	/*	Saving previous position data													*/
	/*--------------------------------------------------------------------------*/
	FencV->RxPosH[1] = FencV->RxPosH[0];		/* Previous value updated					*/

	/*--------------------------------------------------------------------------*/
	/*	Obtain position difference data from motor model section									*/
	/*--------------------------------------------------------------------------*/
	dEncPos = (LONG)MotorLessTestDatas->dFullModePos << FencV->MposSftR;
	MlibAdd6432( MotorLessTestDatas->FencData.Long, dEncPos, MotorLessTestDatas->FencData.Long );

	/*--------------------------------------------------------------------------*/
	/*	Set position data to encoder received data								*/
	/*--------------------------------------------------------------------------*/
	FencV->RxPosL[0].dw	= MotorLessTestDatas->FencData.EncFormat.RxPosL.dw;
	FencV->RxPosH[0]	= MotorLessTestDatas->FencData.EncFormat.RxPosH;

	/*--------------------------------------------------------------------------*/
	/*	Position calculation																*/
	/*--------------------------------------------------------------------------*/
	FencV->MotPosX[1] = FencV->MotPosX[0];		/* Previous value updated					*/
	FencV->MotPosX[0] = FencV->MposSign * ((FencV->RxPosL[0].Long >> FencV->MposSftR) << FencV->MposSftR);

	/*--------------------------------------------------------------------------*/
	/*	Position difference calculation															*/
	/*--------------------------------------------------------------------------*/
	dFencPos = (FencV->MotPosX[0] - FencV->MotPosX[1]) >> FencV->MposSftR;
	FencV->ABphCntr = (UCHAR)(FencV->MotPosX[0]) >> FencV->MposSftR;

	/*--------------------------------------------------------------------------*/
	/*	ExternalPGSpeed calculation															*/
	/*--------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	FencSpd = dFencPos * FencV->KConvLoadSpd;
#else
	FencSpd = MlibMulgain(dFencPos, FencV->KConvLoadSpd);
#endif /* FLOAT_USE */
#if 0
	FencV->MotSpd = LpxTimeBasesFencSpdCalc(dFencPos, FencSpd, FencV->SpdCmpEnable);
	FencV->SpdCmpEnable = TRUE;
#else
	FencV->MotSpd = FencSpd;
#endif

	/*--------------------------------------------------------------------------*/
	/*	Position calculation [32bitRingPulse]												*/
	/*--------------------------------------------------------------------------*/
	FencV->MotPos += dFencPos;
	FencV->MotAbsPos += dFencPos;

	/*--------------------------------------------------------------------------*/
	/*	Initial increment pulse output processing or C phase passage check process						*/
	/*--------------------------------------------------------------------------*/
	if( FencV->DivInfo.IncPulseReq != FALSE )	/* Initial incremental output request?	*/
	{
		/*----------------------------------------------------------------------*/
		/* Countermeasures for frequency-divided pulse output when the motor is moving during initialization at power-on   */
		/*	ScanAProcessing1Set the counter preset and pulse conversion position.*/
		/*----------------------------------------------------------------------*/
		if(FencV->AbsoEncType == MENC_INCRE_TYPE)
		{
			/* C phase passage check process */
			if(FencV->SeRxErr == FALSE)
			{
				FencCheckCphasePass(FencV, sencSetReg);
			}
			/*	Division pulse output processing	(Set counter preset just before)*/
			if(FencV->DivInfo.DivOut == TRUE)
			{
				FencV->DivInfo.DivOutPos += MlibPfbkxremNolim(dFencPos, FencV->DivInfo.DivOutGain, &FencV->DivInfo.DivOutRem);
				sencSetReg->AREG_POTEST = (USHORT)FencV->DivInfo.DivOutPos;
				sencSetReg->AREG_POSET0 = (USHORT)FencV->DivInfo.DivOutPos;
			}
			FencV->DivInfo.IncPulseReq = FALSE;
		}
		else
		{
			/*----------------------------------------------------------------------*/
			/*	Initial increment pulse output processing	  									    */
			/*----------------------------------------------------------------------*/
			FencSencOutIniIncPulse( FencV, sencSetReg );
		}
	}
	else
	{
		/* C phase passage check process	*/
		if( FencV->SeRxErr == FALSE )
		{
			FencCheckCphasePass(FencV, sencSetReg);
		}
		/* Divided pulse output */
		if( (FencV->DivInfo.DivOut != FALSE) && (FencV->DivInfo.IncPulseReq == FALSE) )
		{
			FencV->DivInfo.DivOutPos += MlibPfbkxremNolim(dFencPos, FencV->DivInfo.DivOutGain, &FencV->DivInfo.DivOutRem);
			sencSetReg->AREG_POSET0 = (USHORT) FencV->DivInfo.DivOutPos;
		}
	}

	return( dFencPos );
}

void	MlibAdd6432( LONG *a, LONG b, LONG *x )
{
	INT64	*src;
	INT64	*dst;

	src = (INT64*)a;
	dst = (INT64*)x;
	*dst = *src + b;

	return;
}
/* <<<<< <S1B7> */

/***************************************** end of file **********************************************/

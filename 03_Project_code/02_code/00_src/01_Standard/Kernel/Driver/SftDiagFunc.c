/*********************************** Start of file **************************************************/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SftDiagFunc.c		: Safety Diagnostic function for Sigma-7(SGD7S/W)						*/
/*																									*/
/*		Identification																				*/
/*			Module ID		: 0002(h)																*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*		Update history																				*/
/*			Module version	: 0001(h)																*/
/*			Update info.	: Created																*/
/*			Date			: 2014.03.14															*/
/*			Issue			: (M-gi-S1) Y.Tanaka													*/
/*			Update keyword	: <S1FF><S201>																*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "SequenceIo.h"
#include "SftDiagFunc.h"
#include "HwDeviceIf.h"
#include "BaseEnableSequence.h"/*<S201>*/


/****************************************************************************************************/
/*																									*/
/*		Safety diagnosis section initialization processing																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : --Safety diagnosis section initialization																*/
/*			.PnE09.bit8HWBBSet support or not										*/
/*			.PnE40Set the value as the test pulse abnormal alarm detection level.							*/
/*																									*/
/*	Pull Number : USHORT			opdetsw			:PnE09.bit8												*/
/* 			DET_HWBB_STS	*DetHwbbSts		:HWBBStatus management structure								*/
/*			USHORT			tplsfil			:PnE40Setting value											*/
/*			TEST_PULSE_DIAG *TestPulseDiag	:Test pulse diagnosis management structure								*/
/*Return value: None																					*/
/*																									*/
/****************************************************************************************************/
void SysInitSafetyDevice( USHORT opdetsw, DET_HWBB_STS *DetHwbbSts, USHORT tplsfil, TEST_PULSE_DIAG *TestPulseDiag )
{
	LONG i;
	if( (opdetsw & 0x0100) == 0x0000 )
	{
		DetHwbbSts->hwbbsupport = FALSE;
	}
	else
	{
		DetHwbbSts->hwbbsupport = TRUE;
	}

	for(i=0;i<2;i++)
	{
		TestPulseDiag->DpInErrLvl = tplsfil;
		TestPulseDiag++;
	}
}


/****************************************************************************************************/
/*																									*/
/*		Input signal timing abnormality check processing															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : BBON1LT/BBON2LTWhen the time difference between when the signal becomes active is greater than the parameter setting value		*/
/*			Set as an alarm. This makes an abnormality in the duplex signal (hardware failure, disconnection of input signal) an alarm.		*/
/*			This is a procedure for detection.Depending on the presence or absence of the safety option card,			*/
/*			Alarm code (with card)/None=A.Eb2/A.Eb1) and setting values used for alarm detection					*/
/*			(Card present/None= PnE4C/Pn620) Change.												*/
/*																									*/
/****************************************************************************************************/
BOOL	AdetCheckBbonSignal( CHECK_HWBB_TIMING *pHwbbTmg, BOOL LackOneBbon )
{
	BOOL err;

	err = FALSE;
	/*--------------------------------------------------------------------------------------------------*/
	/*		BBONSignal input timing abnormality check process														*/
	/*--------------------------------------------------------------------------------------------------*/
		if( LackOneBbon )
		{
			pHwbbTmg->var.BbonTimeDiffCnt += KPI_SCCYCLEMS;
			if( pHwbbTmg->var.BbonTimeDiffCnt > pHwbbTmg->conf.MaxBbonTimeDiff )
			{
				/* Clamp the upper limit at the set value */
				pHwbbTmg->var.BbonTimeDiffCnt = pHwbbTmg->conf.MaxBbonTimeDiff;

				/* A.Eb1 : Excessive stop signal input time difference(No card) */
				err = TRUE;
			}
		}
		else		/* <S0E5> */
		{
			pHwbbTmg->var.BbonTimeDiffCnt = 0;
		}
		return err;
}


/****************************************************************************************************/
/*																									*/
/*		Safety HWBB state detection processing																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : --Hardwired baseblock status (HWBBDetection of state)								*/
/*			BBON1LT/BBON2LTIf either or both of the signals are1(HWBBstate),				*/
/*			HWBBStatus sequence is not initializedTRUESet. both signals0(HWBBWhen status is canceled),		*/
/*			HWBBStatus sequence is not initializedFALSE. Also, setHWBBInput edge detection result when status is canceled		*/
/*			Also clear.																			*/
/*			BBONLTThe timing to clear the latch value of the signal is when the servo off input and round operation right are activated.		*/
/*			.																*/
/*			--BBONFlag set for signal input time difference abnormal alarm detection									*/
/*			BBON1LT/BBON2LTWhen the input edge of either signal is detected, the input time difference flag is set				*/
/*			TRUESet.BBON1/BBON2When both input edges of the signal are detected, the input time difference flag is set to	*/
/*			FALSESet. If both input edges cannot be detected, the previous state is retained.			*/
/*																									*/
/*	Pull Number : None																					*/
/*Return value: None																					*/
/*																									*/
/****************************************************************************************************/
BOOL SeqIoDetectHardWiredBB( DET_HWBB_STS *DetHwbb, LONG svon_req,
										ASIC_HW_IF_STR *pAsicHwReg, BOOL HwbbSupport )
{
	BOOL	Bbon1Lt, Bbon2Lt;
	BOOL	HwbbSts;

/*--------------------------------------------------------------------------------------------------*/
/*		Hardwired base block status check													*/
/*--------------------------------------------------------------------------------------------------*/
	/* Servo off input and no round operation privilege acquired and no magnetic pole detection request */
//	if( (0 == (CmdSeqBit & SERVO_ON_BIT)) && (0 == (CmdSeqBit & POL_DETECT_BIT)) && (!FnSvControl) )
	if(svon_req == 0)
	{
		/* BBONClear the latch value of the signal */
		SHalDrv_ResetBBONLtStatus( pAsicHwReg );
	}

	/* CurrentBBONSignal status reading (for monitor) */
	SHalDrv_GetBBONStatus( pAsicHwReg, &DetHwbb->Bbon1, &DetHwbb->Bbon2 );

	/* CurrentBBONSignal latch value reading (HWBBFor detection) */
	SHalDrv_GetBBONLtStatus( pAsicHwReg, &Bbon1Lt, &Bbon2Lt );

	/* BBONDetect the edge of the signal */
	if( Bbon1Lt )
	{
		DetHwbb->Bbon1Edge = TRUE;
	}
	if( Bbon2Lt )
	{
		DetHwbb->Bbon2Edge = TRUE;
	}

	/* HWBBStatus setting processing */
	if( (Bbon1Lt) || (Bbon2Lt) )
	{
		/* BBONEither signal is input */
		if( HwbbSupport == TRUE )
		{ /* HWBBModels that support the function */
			DetHwbb->HwbbState = TRUE;
			DetHwbb->GateDrvDefect = FALSE;
		}
		else
		{ /* HWBBModels that do not support the function */
			DetHwbb->HwbbState = FALSE;
			DetHwbb->GateDrvDefect = TRUE;
		}
	}
	else
	{
		/* BBONNeither signal is input */
		if( (DetHwbb->Bbon1Edge) && (DetHwbb->Bbon2Edge) )
		{
			/* BBON1/BBON2Only when both edges of the signal are detectedHWBBCanceling status */
			DetHwbb->HwbbState = FALSE;
			DetHwbb->Bbon1Edge = FALSE;
			DetHwbb->Bbon2Edge = FALSE;
		}
	}

	/* Set ScanC Output Data */
	if( DetHwbb->HwbbState )
	{
		HwbbSts = TRUE;
	}
	else
	{
		HwbbSts = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Setting the input signal time difference abnormality detection timer start flag											*/
/*--------------------------------------------------------------------------------------------------*/
	/* BBONSignal input time difference flag setting process */
	if( (DetHwbb->Bbon1Edge) ^ (DetHwbb->Bbon2Edge) )
	{
		/* BBONOnly one signal is input */
		DetHwbb->LackOneBbon = TRUE;
	}
	else
	{
		/* BBONBoth signal input */
		if( (DetHwbb->Bbon1Edge) && (DetHwbb->Bbon2Edge) )
		{
			DetHwbb->LackOneBbon = FALSE;
		}
	}

	return	HwbbSts;
}



/****************************************************************************************************/
/*																									*/
/*		Safety function Internal signal abnormality detection processing (Test Pulse Diagnostic)								*/
/*																									*/
/*		Identification																				*/
/*			Function ID		: 0001(h)																*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*		Update history																				*/
/*			Function version: 0001(h)																*/
/*			Update info.	: Created																*/
/*			Date			: 2014.02.14															*/
/*			Issue			: (M-gi-S1) Y.Tanaka													*/
/*			Update keyword	: <S1FF><S201>																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Internal signal on amplifier side(DPIN1/DPIN2Signal)is not a pulse signal, detect an abnormality					*/
/*			HWBBDo not detect abnormality if the status is the same.															*/
/*																									*/
/****************************************************************************************************/
BOOL	AdetTestPulseDiagnostic( TEST_PULSE_DIAG *TestPulseDiag )	/*<S1D1>*/
{
	BOOL err;

	err = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		DPINSignal check processing (BBON=OFF:HWBB(Do not check status)									*/
/*--------------------------------------------------------------------------------------------------*/
	if( TestPulseDiag->DpInEnable )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	DPINSignal is4msIf the pulse signal is not a cycle, it is abnormal											*/
		/*------------------------------------------------------------------------------------------*/
		if( !(TestPulseDiag->DpInState^TestPulseDiag->DpInLastState) )
		{
			TestPulseDiag->DpInDiagState = FALSE;
			if( !((TestPulseDiag->DpInDiagState)^(TestPulseDiag->DpInLastDiagState)) )
			{
				TestPulseDiag->DpInErrCnt++;
				if( (TestPulseDiag->DpInErrCnt) > 2 )
				{
					err = TRUE;
				}
			}
			else
			{
				TestPulseDiag->DpInErrCnt = 0;
			}
		}
		else
		{
			TestPulseDiag->DpInDiagState = TRUE;
			TestPulseDiag->DpInErrCnt = 0;
		}
		TestPulseDiag->DpInLastDiagState = TestPulseDiag->DpInDiagState;
	}
/* <S1D1> >>>>> */
	else if( !(TestPulseDiag->DpInLastEnable) )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	BBON=OFF(HIGH),DPINSignal isHIGHIf not, it is abnormal										*/
		/*------------------------------------------------------------------------------------------*/
		if( TestPulseDiag->DpInState )
		{
			TestPulseDiag->DpInDiagState = FALSE;
			if( !((TestPulseDiag->DpInDiagState)^(TestPulseDiag->DpInLastDiagState)) )
			{
				TestPulseDiag->DpInErrCnt++;
				if( (TestPulseDiag->DpInErrCnt) > 2 )
				{
					err = TRUE;
				}
			}
			else
			{
				TestPulseDiag->DpInErrCnt = 0;
			}
		}
		else
		{
			TestPulseDiag->DpInDiagState = TRUE;
			TestPulseDiag->DpInErrCnt = 0;
		}
		TestPulseDiag->DpInLastDiagState = TestPulseDiag->DpInDiagState;
	}
/* <<<<< <S1D1> */

	return err;
}

BOOL GetHwbbSupportSts( DET_HWBB_STS *DetHwbbSts )
{
	return DetHwbbSts->hwbbsupport;
}


/****************************************************************************************************/
/*																									*/
/*																									*/
/*		LpxSftMdlEncComErrDetect																	*/
/*							  Encoder alarm detection routine										*/
/*							  Processing in ScanB interruption										*/
/*							  Safety-related part													*/
/*																									*/
/*		Identification																				*/
/*			Function ID		: 0002(h)																*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*		Update history																				*/
/*			Function version: 0001(h)																*/
/*			Update info.	: Created																*/
/*			Date			: 2014.02.14															*/
/*			Issue			: (M-gi-S1) Y.Tanaka													*/
/*			Update keyword	: <S0F1>																*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*																									*/
/*		Check Status of Input Signal													<S0F1>		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Internal signal status detectionDPIN1, DPIN2Detect the signal status										*/
/*          DPIN1, DPIN2Signal valid/Detect invalid state													*/
/*																									*/
/*	Pull Number :																							*/
/*	Return value:																							*/
/*																									*/
/****************************************************************************************************/
void SeqIoDetectTestPulseSignal( SEQ_IO_HNDL *pIoSignals, LONG AxisNo )
{
	TEST_PULSE_DIAG *pTestPulseDiag1;
	TEST_PULSE_DIAG *pTestPulseDiag2;
	DET_HWBB_STS *pDetHwbbSts;

	pTestPulseDiag1 = &(pIoSignals->TestPulseDiag[0]);
	pTestPulseDiag2 = &(pIoSignals->TestPulseDiag[1]);
	pDetHwbbSts    = &(pIoSignals->DetHwbbSts);
/*--------------------------------------------------------------------------------------------------*/
/*		The previous value of the signal is updated.																			*/
/*--------------------------------------------------------------------------------------------------*/
	pTestPulseDiag1->DpInLastState = pTestPulseDiag1->DpInState;
	pTestPulseDiag2->DpInLastState = pTestPulseDiag2->DpInState;

/*--------------------------------------------------------------------------------------------------*/
/*		DPIN1Determining the valid/invalid state of the signal. (BBONNot in status && PreviousDPIN1Signal valid ⇒ This timeDPIN1Signal Enable	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (!(pDetHwbbSts->Bbon1)) && (pTestPulseDiag1->DpInLastEnable) )
	{
		pTestPulseDiag1->DpInEnable = TRUE;
	}
	else
	{
		pTestPulseDiag1->DpInEnable = FALSE;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		DPIN2Determining the valid/invalid state of the signal. (BBONNot in status && PreviousDPIN2Signal valid ⇒ This timeDPIN2Signal Enable	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (!(pDetHwbbSts->Bbon2)) && (pTestPulseDiag2->DpInLastEnable) )
	{
		pTestPulseDiag2->DpInEnable = TRUE;
	}
	else
	{
		pTestPulseDiag2->DpInEnable = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		DPIN1/DPIN2Get the signal status																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( 0 == AxisNo )
	{
		pTestPulseDiag1->DpInState = GET_DPIN01();
		pTestPulseDiag2->DpInState = GET_DPIN02();
	}
	else
	{
		pTestPulseDiag1->DpInState = GET_DPIN11();
		pTestPulseDiag2->DpInState = GET_DPIN12();
	}
/*--------------------------------------------------------------------------------------------------*/
/*		DPIN1/DPIN2Status(Previous value)update																*/
/*--------------------------------------------------------------------------------------------------*/
	pTestPulseDiag1->DpInLastEnable  = !(pDetHwbbSts->Bbon1);
	pTestPulseDiag2->DpInLastEnable  = !(pDetHwbbSts->Bbon2);
}


/****************************************************************************/
/* Overview: Safety function Pulse output processing for internal signal abnormality detection		<S0F1>		*/
/*       4msPeriodic pulse output(ScanCProcessing)										*/
/****************************************************************************/
/* Argument		:	-															*/
/* Return value	:	None														*/
/****************************************************************************/
void SftReverceTestPls( void )
{
	VULONG	*CheckPostAddr;

	CheckPostAddr = HALdrv_GetCheckPostAddr();
	*CheckPostAddr ^= (1<< SIL3_TEST_PLS0);
#if (CSW_BOARD_TYPE  == SGD7W_TYPE)
	*CheckPostAddr ^=(1<< SIL3_TEST_PLS1);
#endif		/* #if (CSW_BOARD_TYPE  == SGD7W_TYPE) */
}
//void HALdrv_ReverceTestPls( void )
//{
//	*HALdrv_CheckPost ^= (1<< SIL3_TEST_PLS);
//	return;
//}



/*********************************** End of file ****************************************************/

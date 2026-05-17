/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SequenceIo.c : SequenceI/OManagement processing														*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Servo sequence management processing																*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "SequenceIo.h"


/* External function */
extern void HALdrv_SetSOutC( int axis, HWSOUT hwsout );


#if 0	/*<S1F6>*/
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
#endif	//#if 0	/*<S1F6>*/

#if 0	/*<S1F6>*/
/****************************************************************************************************/
/*																									*/
/*		Safety function Internal signal state detection processing											<S0F1>		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Internal signal status detectionDPIN1, DPIN2Detect the signal status										*/
/*          DPIN1, DPIN2Signal valid/Detect invalid state													*/
/*																									*/
/*	Pull Number :																							*/
/*Return value:																							*/
/*																									*/
/****************************************************************************************************/
void SeqIoDetectTestPulseSignal( SEQ_IO_HNDL *pIoSignals)
{
	TEST_PULSE_DIAG *pTestPulseDiag;
	DET_HWBB_STS *pDetHwbbSts;

	pTestPulseDiag = &(pIoSignals->TestPulseDiag);
	pDetHwbbSts    = &(pIoSignals->DetHwbbSts);
/*--------------------------------------------------------------------------------------------------*/
/*		The previous value of the signal is updated.																			*/
/*--------------------------------------------------------------------------------------------------*/
	pTestPulseDiag->DpIn1LastState = pTestPulseDiag->DpIn1State;
	pTestPulseDiag->DpIn2LastState = pTestPulseDiag->DpIn2State;

/*--------------------------------------------------------------------------------------------------*/
/*		DPIN1Determining the valid/invalid state of the signal. (BBONNot in status && PreviousDPIN1Signal valid ⇒ This timeDPIN1Signal Enable	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (!(pDetHwbbSts->Bbon1)) && (pTestPulseDiag->DpIn1LastEnable) )
	{
		pTestPulseDiag->DpIn1Enable = TRUE;
	}
	else
	{
		pTestPulseDiag->DpIn1Enable = FALSE;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		DPIN2Determining the valid/invalid state of the signal. (BBONNot in status && PreviousDPIN2Signal valid ⇒ This timeDPIN2Signal Enable	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (!(pDetHwbbSts->Bbon2)) && (pTestPulseDiag->DpIn2LastEnable) )
	{
		pTestPulseDiag->DpIn2Enable = TRUE;
	}
	else
	{
		pTestPulseDiag->DpIn2Enable = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		DPIN1/DPIN2Get the signal status																	*/
/*--------------------------------------------------------------------------------------------------*/
	pTestPulseDiag->DpIn1State = GET_DPIN1( );
	pTestPulseDiag->DpIn2State = GET_DPIN2( );
/*--------------------------------------------------------------------------------------------------*/
/*		DPIN1/DPIN2Status(Previous value)update																*/
/*--------------------------------------------------------------------------------------------------*/
	pTestPulseDiag->DpIn1LastEnable  = !(pDetHwbbSts->Bbon1);
	pTestPulseDiag->DpIn2LastEnable  = !(pDetHwbbSts->Bbon2);
}
#endif	/*<S1F6>*/

/* <S1F5> Start */
/*****************************************************************************
  Description: FSTPState detection processing
------------------------------------------------------------------------------
  Parameters:
  	FstpSignal;			Fstp Signal reading status
	hSeqCtrlOut;		Sequence Control Message
	svon_req;			Servo off input
  Return:
	BOOL				FSTPStatus
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
BOOL SeqIoDetectFstp( BOOL FstpSignal, SEQ_CTRL_OUT *hSeqCtrlOut, LONG Svon_req )
{
	/* Servo off input and no round operation privilege acquired and no magnetic pole detection request */
	if ( Svon_req == 0 )
	{
		hSeqCtrlOut->FstpSts = FstpSignal;
	}
	/*FSTPWhen the signal is input*/
	if ( FstpSignal == TRUE )
	{
		hSeqCtrlOut->FstpSts = TRUE;
	}

	return hSeqCtrlOut->FstpSts;
}

/* <S1F5> End */

/****************************************************************************************************/
/*																									*/
/*		Holding brake control processing																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		BKSequence not detected, orNET-I/FBKReleased state				*/
/*		BKSequence detected, andNET-I/FIf there is no cancellation request fromBKStatus								*/
/*																									*/
/****************************************************************************************************/
#ifndef HW_FBA_IO_AXIS
void SeqIoBrakeControl( SEQ_IOSIGNAL *BkSignal, BOOL *BrakeSts,
											BOOL SvBrakeReq, BOOL NetBrkReleaseReq )
{
//	SEQ_IOSIGNAL		*SeqIo;/*<S0C7>*/

	/* BKNo sequence detected, orNET-I/FRequest for release */
	if( (SvBrakeReq == FALSE) || (NetBrkReleaseReq == TRUE) )
	{ /* BRAKEOpen state(BK Signal -> active) */
		*BrakeSts = TRUE;
	}
	else
	{ /* BRAKEStatus(BK Signal -> inactive) */
		*BrakeSts = FALSE;
	}
//<S045>	SeqIo = BkSignal;
//<S045>	SeqIo->SignalStatus = (*BrakeSts & SeqIo->Mask) ^ SeqIo->XorMask;
//<S045>	HALdrv_BrakeControl( SeqIo->PinNumber, SeqIo->SignalStatus );
}
#else
void SeqIoBrakeControlAx( SEQ_IOSIGNAL *BkSignal, BOOL *BrakeSts,
								BOOL SvBrakeReq, BOOL NetBrkReleaseReq, SERIAL_OUTPUT_AX *SerialOutputAx )
{
	SEQ_IOSIGNAL		*SeqIo;

	/* BKNo sequence detected, orNET-I/FRequest for release */
	if( (SvBrakeReq == FALSE) || (NetBrkReleaseReq == TRUE) )
	{ /* BRAKEOpen state(BK Signal -> active) */
		*BrakeSts = TRUE;
	}
	else
	{ /* BRAKEStatus(BK Signal -> inactive) */
		*BrakeSts = FALSE;
	}
	SeqIo = BkSignal;
	SeqIo->SignalStatus = (*BrakeSts & SeqIo->Mask) ^ SeqIo->XorMask;
	HALdrv_BrakeControlAx( SeqIo->PinNumber, SeqIo->SignalStatus, SerialOutputAx );
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Scan C network IF output processing															*/
/*																									*/
/****************************************************************************************************/
void SeqIoOutputNetIf( SEQ_IO_HNDL *SeqIo, BE_SEQ_HNDL *BeSeq, SEQ_CTRL_OUT *SeqCtrlOut,
								 ALMMAN_HANDLE *AlmManager, MENCV *MencV/* <S082> BOOL PhaseReady */, BOOL LockNormalOpeDrive,
								 BOOL MotorLessTestMode )											/* <S1B7> */
{
	LONG	RspSeqBit, RspSigBit, RspCtrlBit;
	BOOL	sts;

	RspSeqBit = 0;
	RspSigBit = 0;
	RspCtrlBit = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		Sequence control state creation																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* NetAlarm */
	RspSeqBit |= (((AlmManager->Status.AlmFlag) ? TRUE : FALSE) << ALM_DETECTED_BITNO);	/* <S0FF> */

	/* NetWarning */
	RspSeqBit |= (((AlmManager->Status.WrnFlag) ? TRUE : FALSE) << WRN_DETECTED_BITNO); /* <S0FF> */

	/* AlarmRstComp */
	RspSeqBit |= ((1^AlmManager->AlmRst) << ALM_RESET_COMP_BITNO);
	RspSeqBit |= ((1^AlmManager->AlmRst) << WRN_RESET_COMP_BITNO);

	/* ServoOnComp */
	sts = (1^SeqCtrlOut->BBSvcRefClrReq) & MencV->PhaseReady /*<S082> PhaseReady*/;
	RspSeqBit |= (sts << SERVO_ON_COMP_BITNO);

	/* SensOnComp */
//	sts = BeSeq->SenStatus;/* & BeSeq->EncReady*/;
	sts = BeSeq->SenStatus & MencV->EncReady /* <S082> */;
	RspSeqBit |= (sts << SENS_ON_COMP_BITNO);

	/* PolDetectComp */
	RspSeqBit |= (MencV->PhaseReady/* <S082> PhaseReady */<< POL_DET_COMP_BITNO);

	/* Set when not in motorless test mode *//* <S1B7> */
	/* TODO:It is unclear whether this process is appropriate */
	if( MotorLessTestMode == FALSE )
	{
		/* BKNo sequence detected, orNET-I/FRequest for release 											*/
		RspSeqBit |= (SeqCtrlOut->BrkSigSts << BRK_RELEASING_BITNO);
	}

	/* Main Power ON */
	RspSeqBit |= (SeqCtrlOut->MainPowerOn << MAIN_POWER_ON_BITNO);

	/* Servo ready signal */
	RspSeqBit |= (BeSeq->ServoReady << SERVO_READY_BITNO);

	/* MotorReady signal */
	RspSeqBit |= (BeSeq->SvReadyPhaseIgnore << MOTOR_READY_BITNO);

	SeqCtrlOut->RspSeqBits = RspSeqBit;

/*--------------------------------------------------------------------------------------------------*/
/*		Input signal status creation																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* HWBB */
	RspSigBit |= ((SeqCtrlOut->HwbbSts | SeqCtrlOut->FstpSts) << HWBB_SIG_BITNO);  /* <S1F5> */
	/* HWBB1 */
	RspSigBit |= (SeqIo->DetHwbbSts.Bbon1 << HWBB1_SIG_BITNO);
	/* HWBB2 */
	RspSigBit |= (SeqIo->DetHwbbSts.Bbon2 << HWBB2_SIG_BITNO);
	/* P-OT */
	RspSigBit |= (SeqCtrlOut->PotSigSts << P_OT_SIG_BITNO);
	/* N-OT */
	RspSigBit |= (SeqCtrlOut->NotSigSts << N_OT_SIG_BITNO);
	/* E-STOP */
	RspSigBit |= ((SeqCtrlOut->EstpSigSts | SeqCtrlOut->FstpSts) << ESTP_SIG_BITNO); /* <S1F5> */

	/* DEC */
	RspSigBit |= (SeqIo->DecSignal.SignalStatus.v << DEC_SIG_BITNO);		/* <S045> */
	/* EXT1 */
	RspSigBit |= (SeqIo->ExtSignal[0].SignalStatus.v << EXT1_SIG_BITNO);	/* <S045> */
	/* EXT2 */
	RspSigBit |= (SeqIo->ExtSignal[1].SignalStatus.v << EXT2_SIG_BITNO);	/* <S045> */
	/* EXT3 */
	RspSigBit |= (SeqIo->ExtSignal[2].SignalStatus.v << EXT3_SIG_BITNO);	/* <S045> */

	SeqCtrlOut->RspSigBits = RspSigBit;

/*--------------------------------------------------------------------------------------------------*/
/*		Control state creation																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* SAFETY_STOP_BIT */
	RspCtrlBit |= (SeqCtrlOut->HwbbSts << SAFETY_STOP_BITNO);
	/* Motor rotation status */
	RspCtrlBit |= (SeqCtrlOut->TgOnSts << MOTOR_MOVING_BITNO);
	/* Speed match detection process */
	RspCtrlBit |= (SeqCtrlOut->VcmpSts << REACH_SPD_BITNO);
	/* Command ready state	*/
	RspCtrlBit |= ((1^LockNormalOpeDrive) << OP_ENABLED_BITNO);

	SeqCtrlOut->RspCtrlBits = RspCtrlBit;
}




/****************************************************************************************************/
/*																									*/
/*		OTFor signal mask settingAPI																		*/
/*																									*/
/****************************************************************************************************/
void SeqIoSetOtSigMask( SEQ_IO_HNDL *SeqIo )
{
	SEQ_IOSIGNAL	*IoSignal;

	/* Mask P-OT Signal */
	IoSignal = &(SeqIo->PotSignal);
	IoSignal->Mask = 0;
	IoSignal->XorMask = 0;
	IoSignal->PinNumber = 0;

	/* Mask N-OT Signal */
	IoSignal = &(SeqIo->NotSignal);
	IoSignal->Mask = 0;
	IoSignal->XorMask = 0;
	IoSignal->PinNumber = 0;
}


/****************************************************************************************************/
/*																									*/
/*		OTFor signal mask cancellationAPI																		*/
/*																									*/
/****************************************************************************************************/
void SeqIoClearOtSigMask( SEQ_IO_HNDL *SeqIo, USHORT pot_selsw, USHORT not_selsw )
{
	SEQ_IOSIGNAL	*IoSignal;

	/* P-OT mapping */
	IoSignal = &(SeqIo->PotSignal);
	switch( pot_selsw >> 12 )
	{
	case 1:	/* Switch OFF => active(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = (ULONG)(pot_selsw & 0xFF);
		break;
	case 2:	/* Switch OFF => inactive(Reverse) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = (ULONG)(pot_selsw & 0xFF);
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 0;
		break;
	}

	/* N-OT mapping */
	IoSignal = &(SeqIo->NotSignal);
	switch( not_selsw >> 12 )
	{
	case 1:	/* Switch OFF => active(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = (ULONG)( not_selsw & 0xFF );
		break;
	case 2:	/* Switch OFF => inactive(Reverse) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = (ULONG)( not_selsw & 0xFF );
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 0;
		break;
	}
}

/****************************************************************************************************/
/*																									*/
/*		<S045> Scan C sequence signal input processing														*/
/*																									*/
/****************************************************************************************************/
void SeqIoInputSeqSignals( SEQ_IO_HNDL *IoSignals, ASIC_HW_IF_STR *AsicHwReg)
{
	HWSIN	SeqInputSignals = HALdrv_ReadInputSignals();
/*--------------------------------------------------------------------------------------------------*/
/*	Sequence input																					*/
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_START_INSIGX( IoSignals );							/* Start Input Signals */
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_GET_INSIGX( PotSignal, SeqInputSignals.dw );			/* P-OT */
	SEQIO_GET_INSIGX( NotSignal, SeqInputSignals.dw );			/* N-OT */
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_END_INSIGX( );										/* End Input Signals */
/*--------------------------------------------------------------------------------------------------*/
}

/* <S21D> Start */
/*****************************************************************************
  Description:	Scan C sequence signal input processing(Axis common)
------------------------------------------------------------------------------
  Parameters:
	IoSignals;				SequenceI/O
	Prm;					Parameter data
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
void SeqIoInputSeqSignalsCom( SEQ_IO_HNDL *IoSignals, PRMDATA *Prm )
{
	/*--------------------------------------------------------------------------------------------------*/
	/*	Sequence input																					*/
	/*--------------------------------------------------------------------------------------------------*/
#if CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE
	UINT			HwSin;
	HwSin = IoSignals->HwSin;
	SEQIO_START_INSIGX_AP( IoSignals );							/* Start Input Signals */
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_GET_INSIGX_AP( ClinkansSignal, HwSin, ((Prm->insel7 >> 0) & 0x0F) );		/* /CLINKANS */
	SEQIO_GET_INSIGX_AP( CmcansSignal, HwSin, ((Prm->insel7 >> 4) & 0x0F) );		/* /CMCANS */
	SEQIO_GET_INSIGX_AP( ConvreadySignal, HwSin, ((Prm->insel7 >> 8) & 0x0F) );		/* /CONV_READY */
	SEQIO_GET_INSIGX_AP( ConvmconSignal, HwSin, ((Prm->insel7 >> 12) & 0x0F) );		/* /CONV_MCON */
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_END_INSIGX_AP( );										/* End Input Signals */
/*--------------------------------------------------------------------------------------------------*/
#else
	HWSIN	SeqInputSignals = HALdrv_ReadInputSignals();
	SEQIO_START_INSIGX( IoSignals );							/* Start Input Signals */
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_GET_INSIGX( ClinkansSignal, SeqInputSignals.dw );		/* /CLINKANS */
	SEQIO_GET_INSIGX( CmcansSignal, SeqInputSignals.dw );		/* /CMCANS */
	SEQIO_GET_INSIGX( ConvreadySignal, SeqInputSignals.dw );	/* /CONV_READY */
	SEQIO_GET_INSIGX( ConvmconSignal, SeqInputSignals.dw );		/* /CONV_MCON */
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_END_INSIGX( );										/* End Input Signals */
/*--------------------------------------------------------------------------------------------------*/
#endif
}
/* <S21D> End */

/****************************************************************************************************/
/*																									*/
/*		<S045> Scan C sequence signal output processing														*/
/*																									*/
/****************************************************************************************************/
void SeqIoOutputSeqSignals( LONG ax, SEQ_IO_HNDL *SeqIo )		/* SGD7W IOI/O support<S118> */
{
	HWSOUT OutX; OutX.dw = 0;
/*--------------------------------------------------------------------------------------------------*/
/*		Sequence output																				*/
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_START_OUTSIGX( SeqIo );					/* Start Output Signal */
/*--------------------------------------------------------------------------------------------------*/
#if (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE) /* <S04B> */
	SEQIO_SET_OUTSIGX( BkSignal,   OutX.dw );		/* /BK */
	SEQIO_SET_OUTSIGX( VcmpSignal, OutX.dw );		/* /VCMP */
	SEQIO_SET_OUTSIGX( TgonSignal, OutX.dw );		/* /TGON */
	SEQIO_SET_OUTSIGX( SrdySignal, OutX.dw );		/* /S-RDY */
	SEQIO_SET_OUTSIGX( WarnSignal, OutX.dw );		/* /WARN */
#endif
/*--------------------------------------------------------------------------------------------------*/
	SEQIO_SET_OUTSIGX( PreWrnSignal, OutX.dw );		/* /PreWrn */ /* <S154> */
	SEQIO_SET_OUTSIGX( ClinkSignal, OutX.dw );		/* /CLINK *//* <S21D> */
	SEQIO_SET_OUTSIGX( CmconSignal, OutX.dw );		/* /CMCON *//* <S21D> */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE) /* <S04B> */
	SEQIO_SET_OUTSIGX( Alo1Signal, OutX.dw );		/* ALO1 */
	SEQIO_SET_OUTSIGX( Alo2Signal, OutX.dw );		/* ALO2 */
	SEQIO_SET_OUTSIGX( Alo3Signal, OutX.dw );		/* ALO3 */
#endif
/*--------------------------------------------------------------------------------------------------*/
//-	SEQIO_END_OUTSIGX_ScanC( OutX );				/* End Output Signal */
	SEQIO_END_OUTSIGX_ScanC( ax, OutX );			/* End Output Signal */
}

/***************************************** end of file **********************************************/

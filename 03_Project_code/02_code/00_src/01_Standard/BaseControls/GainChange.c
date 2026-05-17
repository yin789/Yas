/****************************************************************************************************/
/*																									*/
/*																									*/
/*		GainChange.c : Gain switching related processing definition													*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Gain switching related processing																	*/
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
#include "GainChange.h"
#include "MLib.h"

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void	GselInitAutoGainChange( ATGSEL *pAtGselA, ATGSEL *pAtGselB );							*/
/*	LONG	GselDetGainChngTiming( GSELSIG *GselSig, RSP_CTRLBITS *rspctrl_ptr,						*/
/*														POS_CTRL_STS *PosCtrlSts, LONG Condition );	*/
/*	void	GselAutoGainChange( GAIN_CHNG_HNDL *hGainChg, MFCTRL *pMfcData, LONG triger );			*/
/*	void	GselManualGainChange( GAIN_CHNG_HNDL *hGainChg, MFCTRL *pMfcData,						*/
/*																LONG GselNum, BOOL TuningLess );	*/
/****************************************************************************************************/


/****************************************************************************************************/
/*																									*/
/*		Initialize automatic gain switching																		*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S09B> */
void	GselInitAutoGainChange( ATGSEL *pAtGselA, ATGSEL *pAtGselB )
{
		pAtGselA->var.Timer = 0;					/* Waiting time1Timer initialization						*/
		pAtGselB->var.Timer = 0;					/* Waiting time2Timer initialization						*/
		return;
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Servo gain switching variable initialization																	*/
/*																									*/
/****************************************************************************************************/
void	GselInitServoGainChange( GAIN_CHNG_HNDL *hGainChg )
{
ATGSEL *pAtGsel;													/* <S09B> */

	/* Set at initialization1Set gain */
	hGainChg->GselGains[0] = hGainChg->GselGains[1];
	hGainChg->GselDobs[0] = hGainChg->GselDobs[1];					/* <S09B> */
	hGainChg->AmonActGain = GAIN1;
	hGainChg->ActGain = 1;

	/* Gain switching timer initialization */
	hGainChg->AtGselA.var.Timer = 0;
	hGainChg->AtGselB.var.Timer = 0;

#if 0 /* 2010.11.10 Y.Oka Commented out because smoothing is not supported during automatic gain switching */
	MlibResetLongMemory( &(hGainChg->AtGselA.var), sizeof(hGainChg->AtGselA.var)/4 );
	MlibResetLongMemory( &(hGainChg->AtGselB.var), sizeof(hGainChg->AtGselB.var)/4 );
	MlibResetLongMemory( &(hGainChg->AtGselDobsA.var), sizeof(hGainChg->AtGselDobsA.var)/4 );
	MlibResetLongMemory( &(hGainChg->AtGselDobsB.var), sizeof(hGainChg->AtGselDobsB.var)/4 );

	hGainChg->AtGselA.var.prevGain = &(hGainChg->GselGains[1]);
	hGainChg->AtGselA.var.nextGain = &(hGainChg->GselGains[2]);
	hGainChg->AtGselB.var.prevGain = &(hGainChg->GselGains[2]);
	hGainChg->AtGselB.var.nextGain = &(hGainChg->GselGains[1]);

	hGainChg->AtGselDobsA.var.prevGain = &(hGainChg->GselDobs[1]);
	hGainChg->AtGselDobsA.var.nextGain = &(hGainChg->GselDobs[2]);
	hGainChg->AtGselDobsB.var.prevGain = &(hGainChg->GselDobs[2]);
	hGainChg->AtGselDobsB.var.nextGain = &(hGainChg->GselDobs[1]);

	hGainChg->GselTrqFilMin = MlibPcalKf1gain( 10, KPI_SACYCLEUS, 0 );
#endif /* #if 0 */
/*--------------------------------------------------------------------------------------------------*/
/*	<S09B> Auto Gain Change																			*/
/*--------------------------------------------------------------------------------------------------*/
	GselInitAutoGainChange( &(hGainChg->AtGselA),				/* condition A */
							&(hGainChg->GselGains[1]),			/* prevGain */
							&(hGainChg->GselGains[2]),			/* nextGain */
							&(hGainChg->GselDobs[1]),			/* prevGain(Dobs) */
							&(hGainChg->GselDobs[2]) );			/* nextGain(Dobs) */
	GselInitAutoGainChange( &(hGainChg->AtGselB),				/* condition B */
							&(hGainChg->GselGains[2]),			/* prevGain */
							&(hGainChg->GselGains[1]),			/* nextGain */
							&(hGainChg->GselDobs[2]),			/* prevGain(Dobs) */
							&(hGainChg->GselDobs[1]) );			/* nextGain(Dobs) */
/*--------------------------------------------------------------------------------------------------*/
/*	<S09B> ΔGain Parameter recalculation(As a precaution)														*/
/*--------------------------------------------------------------------------------------------------*/
	pAtGsel = &(hGainChg->AtGselA);								/* condition A */
	PcalAtGseldKpSet( pAtGsel, &(pAtGsel->conf.dGain) );		/* calculate delta Kp */
	PcalAtGseldKviSet( pAtGsel, &(pAtGsel->conf.dGain) );		/* calculate delta Kv, Kvi */
	PcalAtGseldKlpfSet( pAtGsel, &(pAtGsel->conf.dGain) );		/* calculate delta Klpf */
	PcalAtGseldKsSet( pAtGsel, &(pAtGsel->conf.dGainDobs) );	/* calculate delta Ks */
	pAtGsel = &(hGainChg->AtGselB);								/* condition B */
	PcalAtGseldKpSet( pAtGsel, &(pAtGsel->conf.dGain) );		/* calculate delta Kp */
	PcalAtGseldKviSet( pAtGsel, &(pAtGsel->conf.dGain) );		/* calculate delta Kv, Kvi */
	PcalAtGseldKlpfSet( pAtGsel, &(pAtGsel->conf.dGain) );		/* calculate delta Klpf */
	PcalAtGseldKsSet( pAtGsel, &(pAtGsel->conf.dGainDobs) );	/* calculate delta Ks */
/*--------------------------------------------------------------------------------------------------*/

	/* CoinOn, NearOn, RefZOn */
//	hGainChg->GselSig.CoinOn  = TRUE;
//	hGainChg->GselSig.CoinOff = FALSE;
//	hGainChg->GselSig.NearOn  = TRUE;
//	hGainChg->GselSig.NearOff = FALSE;
//	hGainChg->GselSig.RefZOn  = TRUE;
//	hGainChg->GselSig.RefZOff = FALSE;

}


/****************************************************************************************************/
/*																									*/
/*		Gain switching condition determination processing																		*/
/*																									*/
/****************************************************************************************************/
LONG	GselDetGainChngTiming( BOOL CoinSignal, BOOL NearSignal, BOOL RefZSignal, LONG Condition )
{
	LONG	triger;

	/* If condition A is met, the second gain transition or second gain is being selected */
	switch( Condition )
	{
	case 0x00:
		triger = CoinSignal;
		break;
	case 0x01:
		triger = CoinSignal ^ 0x01;
		break;
	case 0x02:
		triger = NearSignal;
		break;
	case 0x03:
		triger = NearSignal ^ 0x01;
		break;
	case 0x04:
		triger = RefZSignal;
		break;
	case 0x05:
		triger = RefZSignal ^ 0x01;
		break;
	default:
		triger = 0;
		break;
	}
	return triger;
}
#if 0
LONG	GselDetGainChngTiming( GAIN_CHNG_HNDL *hGainChg, BOOL CoinSignal,
    	                       BOOL NearSignal, BOOL RefZSignal, LONG Condition )
{
	LONG	triger;

	/* Local variable initialization */
	triger = 0;

	/* Create trigger signal for automatic gain switching */
	GselSig->CoinOn		= PosCtrlSts->CoinSignal;
	GselSig->CoinOff	= PosCtrlSts->CoinSignal ^ 0x01;
	GselSig->NearOn		= PosCtrlSts->NearSignal;
	GselSig->NearOff	= PosCtrlSts->NearSignal ^ 0x01;
	GselSig->RefZOn		= PosCtrlSts->RefZSignal;
	GselSig->RefZOff	= PosCtrlSts->RefZSignal ^ 0x01;

	/* If condition A is met, the second gain transition or second gain is being selected */
	switch( Condition )
	{
	case 0x00:
		triger = GselSig->CoinOn;
		break;
	case 0x01:
		triger = GselSig->CoinOff;
		break;
	case 0x02:
		triger = GselSig->NearOn;
		break;
	case 0x03:
		triger = GselSig->NearOff;
		break;
	case 0x04:
		triger = GselSig->RefZOn;
		break;
	case 0x05:
		triger = GselSig->RefZOff;
		break;
	default:
		triger = 0;
		break;
	}
	return triger;
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Automatic gain switching																				*/
/*																									*/
/*		Gain switching processingμExecuted by program.CPUside performs condition judgment and wait time management, and the		*/
/*		μSend gain switching request to program(MREG_BITCMDI.BITCMD_GAINCHNGON).						*/
/*		※MREG_BITCMDI.BITCMD_GAINCHNGON = ONto switch to the second gain.								*/
/*																									*/
/*		Note that⊿Gain switching function(Pn131)is not supported.												*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S09B> AutoGainChange.c */
void	GselAutoGainChange( GAIN_CHNG_HNDL *hGainChg, MFCTRL *pMfcData, LONG triger, CTRL_CMD_PRM *CtrlCmdPrm)
{
	ATGSEL			*atgsela_ptr;
	ATGSEL			*atgselb_ptr;

	atgsela_ptr		= &( hGainChg->AtGselA );
	atgselb_ptr		= &( hGainChg->AtGselB );

	if( triger )
	{
		if( atgsela_ptr->var.Timer >= atgsela_ptr->conf.Wait )
		{ /* No.1→No.2After gain waiting time elapses */
			/* No.2Gain */
		/*------------------------------------------------------------------------------------------*/
		/*	The following processμExecuted by program.													*/
		/*		hGainChg->GselGains[0]	= hGainChg->GselGains[2];									*/
		/*		pDobsData->param[0]		= pDobsData->param[2];										*/
		/*------------------------------------------------------------------------------------------*/
			hGainChg->GselGains[0].VirtualPosKp = hGainChg->GselGains[2].VirtualPosKp;
			hGainChg->AmonActGain = GAIN2;
			hGainChg->ActGain = 2;
			atgselb_ptr->var.Timer = 0; /* Waiting time2Timer initialization */
		}
		else
		{
			atgsela_ptr->var.Timer++; /* Waiting time1Timer count up */
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		ConditionsBEstablished1Gain transition or digit1During gain selection												*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		if( atgselb_ptr->var.Timer >= atgselb_ptr->conf.Wait )
		{ /* No.2→No.1After gain waiting time elapses */
			/* No.1Gain */
		/*------------------------------------------------------------------------------------------*/
		/*	The following processμExecuted by program.													*/
		/*		hGainChg->GselGains[0]	= hGainChg->GselGains[1];									*/
		/*		pDobsData->param[0]		= pDobsData->param[1];										*/
		/*------------------------------------------------------------------------------------------*/
			hGainChg->GselGains[0].VirtualPosKp = hGainChg->GselGains[1].VirtualPosKp;
			hGainChg->AmonActGain = GAIN1;
			hGainChg->ActGain = 1;
			atgsela_ptr->var.Timer = 0; /* Waiting time 1 timer initialization */
		}
		else
		{
			atgselb_ptr->var.Timer++; /* Waiting time 2 timer count up */
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		ConditionsAbutB, the previous gain is maintained												*/
/*--------------------------------------------------------------------------------------------------*/
	/* MFCdoes not have automatic gain switching. Part1Leave gain */
	pMfcData->conf.MfcPrm[0] = pMfcData->conf.MfcPrm[1];

	/*------------------------------------------------------------------------------------------*/
	/*	The following processμExecuted by program.													*/
	/*		Speed limit gain does not have automatic gain switching. Leave the first gain unchanged							*/
	/*		pTrqCtrl->conf.OverTrqLoopGain[0]	= pTrqCtrl->conf.OverTrqLoopGain[1];			*/
	/*		pTrqCtrl->conf.OverTrqIntegGain[0]	= pTrqCtrl->conf.OverTrqIntegGain[1];			*/
	/*------------------------------------------------------------------------------------------*/
	CtrlCmdPrm->OverTrqLoopGain[0]	= CtrlCmdPrm->OverTrqLoopGain[1];
	CtrlCmdPrm->OverTrqIntegGain[0]	= CtrlCmdPrm->OverTrqIntegGain[1];

	return;
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Manual gain switching																		*/
/*																									*/
/*		Gain switching processingμExecuted by program.CPUside,MECHATROLINKBank received fromNoto	*/
/*		In response,μSend gain switching request to program(MREG_BITCMDI.BITCMD_GAINCHNGON).				*/
/*																									*/
/****************************************************************************************************/
//<S09B>void	GselManualGainChange( GAIN_CHNG_HNDL *hGainChg, MFCTRL *pMfcData, LONG GselNum, BOOL TuningLess, 
//<S09B>				CTRL_CMD_PRM *CtrlCmdPrm )
void GselManualGainChange( BASE_CTRL_HNDL *BaseControls )					/* <S09B> */
{
GAIN_CHNG_HNDL *hGainChg	= &(BaseControls->GainChange);					/* <S09B> */
MFCTRL *pMfcData			= &(BaseControls->MFControl);					/* <S09B> */
LONG GselNum				= BaseControls->GselNum;						/* <S09B> */
BOOL TuningLess				= BaseControls->TuneLessCtrl.var.TuneLessAct;	/* <S09B> */
CTRL_CMD_PRM *CtrlCmdPrm	= &(BaseControls->CtrlCmdPrm);					/* <S09B> */
//DOBS_CTRL *pDobs			= &(BaseControls->DobsCtrl);					/* <S09B> *///<S0C7>
	/*------------------------------------------------------------------------------------------*/
	/*	The following processμExecuted by program.													*/
	/*		Gain selection for no adjustment (5Bank)													*/
	/*		if( TuningLess != FALSE )															*/
	/*		{ Adjustmentless enabled -> Disable gain switching and use adjustment-less gain				*/
	/*			hGainChg->GselGains[0] = hGainChg->GselGains[5];								*/
	/*			Disturbance observer																	*/
	/*			pDobsData->param[0] = pDobsData->param[1];										*/
	/*			Speed limit loop gain															*/
	/*			pTrqCtrl->conf.OverTrqIntegGain[0] = pTrqCtrl->conf.OverTrqIntegGain[1];		*/
	/*			Speed limit integral gain																*/
	/*			pTrqCtrl->conf.OverTrqLoopGain[0] = pTrqCtrl->conf.OverTrqLoopGain[1];			*/
	/*		}																					*/
	/*------------------------------------------------------------------------------------------*/
	if( TuningLess != FALSE )
	{ /* Adjustmentless enabled -> Disable gain switching and use adjustment-less gain */
		/* Basic control gain */
		hGainChg->GselGains[0] = hGainChg->GselGains[5];
		/* Disturbance observer <S09B> */
		hGainChg->GselDobs[0] = hGainChg->GselDobs[1];					/* <S09B> */
		/* MFCGain */
		pMfcData->conf.MfcPrm[0] = pMfcData->conf.MfcPrm[1];
		/* Current gain confirmation */
		hGainChg->ActGain = 1;
		hGainChg->AmonActGain = GAIN1;
		/*			Speed limit loop gain						*/
		CtrlCmdPrm->OverTrqLoopGain[0] = CtrlCmdPrm->OverTrqLoopGain[1];
		/*			Speed limit integral gain							*/
		CtrlCmdPrm->OverTrqIntegGain[0] = CtrlCmdPrm->OverTrqIntegGain[1];
	}
	else
	{
	/*------------------------------------------------------------------------------------------*/
	/*	The following processμExecuted by program.													*/
	/*		if( GselNum < SVCDEF_NETGSEL_SETNUM )												*/
	/*		{ No.1Gain, No.2When selecting gain														*/
	/*			hGainChg->GselGains[0] = hGainChg->GselGains[GselNum+1]							*/
	/*			Disturbance observer																	*/
	/*			pDobsData->param[0] = pDobsData->param[GselNum+1];								*/
	/*			Speed limit loop gain															*/
	/*			pTrqCtrl->conf.OverTrqIntegGain[0] = pTrqCtrl->conf.OverTrqIntegGain[GselNum+1];*/
	/*			Speed limit integral gain																*/
	/*			pTrqCtrl->conf.OverTrqLoopGain[0] = pTrqCtrl->conf.OverTrqLoopGain[GselNum+1];	*/
	/*		}																					*/
	/*------------------------------------------------------------------------------------------*/
		if( GselNum < SVCDEF_NETGSEL_SETNUM )
		{ /* No.1Gain, No.2When selecting gain */
			/* Basic control gain */
			hGainChg->GselGains[0] = hGainChg->GselGains[GselNum + 1];
			/* Disturbance observer <S09B> */
			hGainChg->GselDobs[0] = hGainChg->GselDobs[GselNum + 1];			/* <S09B> */
			/* MFCGain */
			pMfcData->conf.MfcPrm[0] = pMfcData->conf.MfcPrm[GselNum + 1];
			/* Current gain confirmation */
			hGainChg->ActGain = GselNum + 1;
			hGainChg->AmonActGain = GAIN1 << GselNum;
			/*			Speed limit loop gain						*/
			CtrlCmdPrm->OverTrqLoopGain[0] = CtrlCmdPrm->OverTrqLoopGain[GselNum+1];
			/*			Speed limit integral gain							*/
			CtrlCmdPrm->OverTrqIntegGain[0] = CtrlCmdPrm->OverTrqIntegGain[GselNum+1];
		}
		else
		{ /* No.3Gain, No.4When selecting gain, select1Fixed gain */
			/* MFCGain */
			pMfcData->conf.MfcPrm[0] = pMfcData->conf.MfcPrm[1];
			/* Disturbance observer <S09B> */
			hGainChg->GselDobs[0] = hGainChg->GselDobs[1];			/* <S09B> */
			/* Current gain confirmation */
			hGainChg->ActGain = 1;
			hGainChg->AmonActGain = GAIN1;
			/*			Speed limit loop gain						*/
			CtrlCmdPrm->OverTrqLoopGain[0] = CtrlCmdPrm->OverTrqLoopGain[1];
			/*			Speed limit integral gain							*/
			CtrlCmdPrm->OverTrqIntegGain[0] = CtrlCmdPrm->OverTrqIntegGain[1];
		}
	}

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Automatic gain switching parameter calculation																*/
/*																									*/
/****************************************************************************************************/
void	GselCalculatePrm( GAIN_CHNG_HNDL *hGainChg, USHORT gnswwait1, USHORT gnswwait2, LONG ScanTime )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Gain switching waiting time 1(Pn135) [ms]															*/
/*--------------------------------------------------------------------------------------------------*/
	hGainChg->AtGselA.conf.Wait = gnswwait1 * 1000 / ScanTime;

/*--------------------------------------------------------------------------------------------------*/
/*		Gain switching waiting time 2(Pn136) [ms]															*/
/*--------------------------------------------------------------------------------------------------*/
	hGainChg->AtGselB.conf.Wait = gnswwait2 * 1000 / ScanTime;

}



/****************************************************************************************************/
/*																									*/
/*		Obtain automatic gain switching pointerAPI																*/
/*																									*/
/****************************************************************************************************/
BOOL	GselGet_AtGselStatus( GAIN_CHNG_HNDL *hGainChg )
{
	return hGainChg->AtGsel;
}

/***************************************** end of file **********************************************/

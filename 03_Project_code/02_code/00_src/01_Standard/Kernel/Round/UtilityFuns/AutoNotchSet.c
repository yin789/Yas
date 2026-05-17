/****************************************************************************************************/
/*																									*/
/*																									*/
/*		AutoNotchSet.c : Auto notch filter setting function main module									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
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
/*		Rev.1.00 : 2007.02.04  Y.Takei	<V109>														*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "AutoNotchSet.h"
#include "TuneLib.h"
#include "PrmAccessIf.h"
#include "PnPrmListTbl.h"
#include "KLib.h"
#include "MLib.h"



/****************************************************************************************************/
/*																									*/
/*		Function Definition																			*/
/*																									*/
/****************************************************************************************************/
/* void		AutoNotchFilterSetting( ANOTCHSEQCTRL *ANotchCtrl );									*/
/* void		AutoNotchInitialize( ANOTCHSEQ *ANotchSeq );											*/
/* void		AutoNotchCalculatePrm( ANOTCHSEQ *ANotchSeq, USHORT kv, USHORT jrate);					*/
/* void		AutoNotchCalculateInitPrm( ANOTCHSEQ *ANotchSeq, AUTONOTCH *ANotch,						*/
/*															LONG AmpType, UNI_PRM_STR *UniPrm );	*/
/* void		RpiResetStatusANotch1( ANOTCHSEQ *ANotchSeq );											*/
/* void		RpiResetStatusANotch2( ANOTCHSEQ *ANotchSeq );											*/
/* UCHAR	RpiGetANotchPreStat( ANOTCHSEQ *ANotchSeq );											*/
/* USHORT	RpiGetANotchSetStat( ANOTCHSEQ *ANotchSeq );											*/
/* void		RpiSetRegSpatANotchState( ANOTCHSEQ *ANotchSeq );										*/
/* void		RpiANotchFftReqSet( ANOTCHSEQ *ANotchSeq, FFTANALYZE *FftAna );							*/
/* void		RpiANotchTrqFilCalc( ANOTCHSEQ *ANotchSeq, AUTONOTCH *ANotch,							*/
/*														REGIF_CONFIG_T *RegCfg, PRMDATA *Prm );		*/
/* void		RpiANotchDisableReqSet( ANOTCHSEQ *ANotchSeq );											*/
/* void		AutoNotchSetCondition( ANOTCHSEQ *ANotchSeq, BOOL EepromWriteProtect,					*/
/* 									BOOL BaseEnableSts, BOOL TuneLessSts, BOOL OverTravelSts );		*/
/****************************************************************************************************/
static	SHORT		autoNotchCheckCondition( ANOTCHSEQ *ANotchSeq, DETVIB *DetVib, PRMDATA *Prm );
static	LONG		autoNotchFreqComp( ANOTCHSEQ *ANotchSeq, LONG DFreq );
static	PRM_RSLT	autoNotchPrmWriteEeprom( REGIF_CONFIG_T *RegCfg, BOOL EepromWriteProtect );



/****************************************************************************************************/
/*																									*/
/*		Auto notch variable initialization																		*/
/*																									*/
/****************************************************************************************************/
void	AutoNotchInitialize( ANOTCHSEQ *ANotchSeq )
{
	MlibResetLongMemory( &(ANotchSeq->var), sizeof(ANotchSeq->var) / 4 );
	return;
}



/****************************************************************************************************/
/*																									*/
/*		For automatic notchKvSetting																			*/
/*																									*/
/****************************************************************************************************/
void	AutoNotchCalculatePrm( ANOTCHSEQ *ANotchSeq, USHORT kv, USHORT jrate)
{
	/*----------------------------------------------------------------------------------------------*/
	/*	Gain parameter acquisition for automatic notch calculation														*/
	/*----------------------------------------------------------------------------------------------*/
	ANotchSeq->conf.kv = kv;
	ANotchSeq->conf.jrate = jrate;
}



/****************************************************************************************************/
/*																									*/
/*		Initial settings for automatic notch																		*/
/*																									*/
/****************************************************************************************************/
void	AutoNotchCalculateInitPrm( ANOTCHSEQ *ANotchSeq, AUTONOTCH *ANotch, LONG AmpType, UNI_PRM_STR *UniPrm )
{
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;
	LONG		VibDetectLevel;

	Prm = UniPrm->Prm;
	Bprm = UniPrm->Bprm;

	if( AmpType == MOTTYPE_LINEAR )
	{
		VibDetectLevel = NOTCH_BASIC_VIB_LEVEL_L;
	}
	else
	{
		VibDetectLevel = NOTCH_BASIC_VIB_LEVEL;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Vibration level for automatic notch																		*/
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	ANotchSeq->conf.VibCheckLevelForTunLess =
		BprmSpeedLevelCal( Bprm, (VibDetectLevel + (Bprm->SpdDetectUnit /2 )), 0 );
#else
	ANotchSeq->conf.VibCheckLevelForTunLess =
		BprmSpeedLevelCal( Bprm, (VibDetectLevel + (Bprm->SpdDetectUnit >> 1)), 0 );
#endif /* FLOAT_USE */

	/*----------------------------------------------------------------------------------------------*/
	/*	Initial value is 1stKv, Inertia ratio setting value															*/
	/*----------------------------------------------------------------------------------------------*/
	ANotchSeq->conf.kv = Prm->loophz;
	ANotchSeq->conf.jrate = Prm->jrate;
	/*----------------------------------------------------------------------------------------------*/
	/*	Automatic notch filter type initial value																*/
	/*----------------------------------------------------------------------------------------------*/
	TuneFilGainCalbyFilType( ANotch, NOTCH_FILTYPE );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Adjustment-less automatic notch sequence																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview:Perform vibration detection and frequency analysis in normal operation status and set notch filter						*/
/*		 Settings are notch frequency, notch Q value, and torque command filter.								*/
/*																									*/
/****************************************************************************************************/
void	AutoNotchFilterSetting( ANOTCHSEQCTRL *ANotchCtrl )
{
	LONG			Freq;
	LONG			FreqCmp;
	LONG			DetectFreqMin;
	ANOTCHSEQ		*ANotchSeq;
	AUTONOTCH		*ANotch;
	FFTANALYZE		*FftAna;
	DETVIB			*DetVib;
	REGIF_CONFIG_T	*RegCfg;
	PRMDATA			*Prm;

	/* Setting each pointer */
	ANotchSeq = ANotchCtrl->ANotchSeq;
	ANotch = ANotchCtrl->ANotch;
	FftAna = ANotchCtrl->FftAna;
	DetVib = ANotchCtrl->DetVib;
	RegCfg = ANotchCtrl->RegCfg;
	Prm = ANotchCtrl->Prm;

	/*----------------------------------------------------------------------------------------------*/
	/*	Auto notch execution determination																			*/
	/*----------------------------------------------------------------------------------------------*/
	if( ANotchSeq->conf.TuneLessSts == FALSE )
	{ /* Adjustment-less invalid state */
		return;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Trace for servo control																		*/
	/*----------------------------------------------------------------------------------------------*/
	ANotchSeq->var.FftStat =
		TuneVibrationTrace( DetVib, FftAna, ANotchSeq->var.FftReq, 0, ANotchSeq->conf.TuneLessSts );

	/*----------------------------------------------------------------------------------------------*/
	/*	Automatic notch execution condition determination																		*/
	/*----------------------------------------------------------------------------------------------*/
	if( ((~Prm->tfuncsw) & Prm->anotchsw & 0x0F00) == 0x0000 )
	{ /* Automatically configurable notch not available */
		/* Automatic setting disabled status setting */
		ANotchSeq->var.ANotchStat = OPTUNE_STS_NT_NOENABLE;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	OTWhile inputting, wait for notch setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( (ANotchSeq->conf.OverTravelSts == TRUE) && (FftGetForceAnaReq( FftAna ) == FALSE) )
	{ /* OTMedium && Servo adjustment notch frequency detection forced execution not performed */
		/* Vibration detection wait flagON */
		ANotchSeq->var.NotchChk = TRUE;
		/* Vibration detection wait timer reset */
		KlibRstLongTimer( &ANotchSeq->var.NotchChkTimer );
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Frequency analysis request																				*/
	/*----------------------------------------------------------------------------------------------*/
	if( autoNotchCheckCondition(ANotchSeq, DetVib, Prm) == OK )
	{
		/* As a precaution, clear forced frequency analysis request */
		FftSetForceAnaReq( FftAna, FALSE );
		ANotchSeq->var.FftReq = TRUE;
		ANotchSeq->var.ANotchStat = OPTUNE_STS_NT_DETECT;

		/* Clear differential speed peak value */
		DetVibResetMaxMinSpeedError( DetVib );
	}

	/*------------------------------------------------------------------------------------------*/
	/*	Notch filter setting																		*/
	/*------------------------------------------------------------------------------------------*/
	if( ANotchSeq->var.FftStat == TUNE_FREQ_SEARCHEND )
	{ /* When frequency analysis completes normally */
		/* Trace requestOFF */
		ANotchSeq->var.FftReq = FALSE;

		/*--------------------------------------------------------------------------------------*/
		/*	Detection frequency lower limit setting																	*/
		/*--------------------------------------------------------------------------------------*/
		if( FftGetForceAnaReq( FftAna ) == FALSE )
		{	/* Forced detectionOFF */
			DetectFreqMin = ANOTCH_MIN_SET_FREQ;
		}
		else
		{	/* Forced detection */
			DetectFreqMin = ANOTCH_MIN_SET_FREQ2;
			/* Forced frequency analysis request clear */
			FftSetForceAnaReq( FftAna, FALSE );
		}

		/*--------------------------------------------------------------------------------------*/
		/*	Notch frequency correction																	*/
		/*--------------------------------------------------------------------------------------*/
		/* Getting notch frequency correction value */
		FreqCmp = (LONG)FftGetAnaRsltVibmRf( FftAna );
		/* Auto notch frequency correction */
		Freq = autoNotchFreqComp( ANotchSeq, FreqCmp );

		/*--------------------------------------------------------------------------------------*/
		/*	Automatic notch filter type setting														*/
		/*--------------------------------------------------------------------------------------*/
		if( ANotchSeq->conf.TuneLessSts == TRUE )
		{ /* Adjustmentless enabled */
			if( Freq >= 2000 )
			{	/* 2000Hzor more is filter type3 */
				TuneFilGainCalbyFilType( ANotch, NOTCH_FILTYPE2 );
			}
			else
			{
				TuneFilGainCalbyFilType( ANotch, NOTCH_FILTYPE );
			}
		}

		/* Notch setting*/
		if( Freq >= DetectFreqMin )
		{ /* 200HzDo not use a notch filter below */
			/* Auto notch filter setting information set */
			TuneSetNotchFilSettingData( ANotch, (USHORT)Freq, ANotchSeq->conf.kv, ANotchSeq->conf.jrate );

			/* Automatic notch filter setting process */
			ANotchSeq->var.ANotchSetStat = TuneNotchFilterSetting( ANotch, Prm, RegCfg );

			/* Create notch setting status */
			switch( ANotchSeq->var.ANotchSetStat & 0x0F )
			{
			case OPTUNE_STS_NOTCH1_OK: /* 1Stage notch setting completed normally */
				/* Notch settingEEPROMWrite */
				autoNotchPrmWriteEeprom( RegCfg, ANotchSeq->conf.EepromWriteProtect );
				ANotchSeq->var.ANotch1Stat = OPTUNE_STS_NT1_COMP;
				ANotchSeq->var.ANotchPreStat = OPTUNE_PSTS_NT1_COMP;
				break;

			case OPTUNE_STS_NOTCH2_OK: /* 2Stage notch setting completed normally */
				/* Notch settingEEPROMWrite */
				autoNotchPrmWriteEeprom( RegCfg, ANotchSeq->conf.EepromWriteProtect );
				ANotchSeq->var.ANotch2Stat = OPTUNE_STS_NT2_COMP;
				ANotchSeq->var.ANotchPreStat = OPTUNE_PSTS_NT2_COMP;
				break;

			case OPTUNE_STS_NOTCHQ1_ERR: /* 1Row notchQValue error */
				ANotchSeq->var.ANotch1Stat = OPTUNE_STS_NT1_NG;
				break;

			case OPTUNE_STS_NOTCHQ2_ERR: /* 2Row notchQValue error */
				ANotchSeq->var.ANotch2Stat = OPTUNE_STS_NT2_NG;
				break;

			default:
				break;
			}

			/* Vibration not detected status setting */
			ANotchSeq->var.ANotchStat = OPTUNE_STS_NT_NODETECT;
		}
		else
		{ /* When lower than detection minimum value */
			/* Detection error status setting */
			ANotchSeq->var.ANotchStat = OPTUNE_STS_NT_DET_ERR;
		}

		/*------------------------------------------------------------------------------------------*/
		/*	Vibration detection wait processing																		*/
		/*		After frequency analysis, notchON/OFF, detection will be temporarily canceled.						*/
		/*		If you analyze the frequency again immediately after that, you will only get the same result.							*/
		/*------------------------------------------------------------------------------------------*/
		/* Vibration detection wait flagON */
		ANotchSeq->var.NotchChk = TRUE;
		/* Vibration detection wait timer reset */
		KlibRstLongTimer( &ANotchSeq->var.NotchChkTimer );
	}
	else if( ANotchSeq->var.FftStat == TUNE_FREQ_SEARCHERR )
	{	/* Error processing */
		ANotchSeq->var.FftReq = FALSE;
		/* Forced frequency analysis request clear */
		FftSetForceAnaReq( FftAna, FALSE );
		/* Detection error status setting */
		ANotchSeq->var.ANotchStat = OPTUNE_STS_NT_DET_ERR;

		/*------------------------------------------------------------------------------------------*/
		/*	Vibration detection wait processing																		*/
		/*		After frequency analysis, notchON/OFF, detection will be temporarily canceled.						*/
		/*		If you analyze the frequency again immediately after that, you will only get the same result.							*/
		/*------------------------------------------------------------------------------------------*/
		/* Vibration detection wait flagON */
		ANotchSeq->var.NotchChk = TRUE;
		/* Vibration detection wait timer reset */
		KlibRstLongTimer( &ANotchSeq->var.NotchChkTimer );
	}

	/*------------------------------------------------------------------------------------------*/
	/*	Waiting for notch setting																			*/
	/*------------------------------------------------------------------------------------------*/
	if( ANotchSeq->var.NotchChk == TRUE )
	{	/* When notch setting is completed */
		if( KlibGetLongTimerMs( ANotchSeq->var.NotchChkTimer ) > ANOTCH_NOTCH_SET_CHK_TIME )
		{
			/* Always clear peak value while waiting for vibration convergence */
			DetVibResetMaxMinSpeedError( DetVib );
			/* Release of check wait */
			ANotchSeq->var.NotchChk = FALSE;
			/* Go to initial notch setting state */
			ANotchSeq->var.ANotchSetStat = OPTUNE_STS_NOTCH_INIT;
		}
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Automatic notch condition check																		*/
/*																									*/
/****************************************************************************************************/
static	SHORT	autoNotchCheckCondition( ANOTCHSEQ *ANotchSeq, DETVIB *DetVib, PRMDATA *Prm )
{
	SHORT	rc;
	LONG	SpdErrPeak;
	rc = NG;

	if( ((~Prm->tfuncsw) & Prm->anotchsw & 0x0F00) != 0x0000 )
	{ /* Automatically configurable notch available */
		if( (ANotchSeq->var.NotchChk == FALSE) && (ANotchSeq->conf.BaseEnableSts == TRUE) )
		{ /* Release of check wait && While the servo is on */
			/* Speed deviation for vibration detection Peak to Peak Acquisition */
			SpdErrPeak = DetVibGetSpdErrPeak( DetVib );
			if( SpdErrPeak > ANotchSeq->conf.VibCheckLevelForTunLess )
			{ /* Vibration detection level exceeds vibration level */
				rc = OK;
			}
		}
	}
	return( rc );
}





/****************************************************************************************************/
/*																									*/
/*		Auto notch frequency correction(Correction formula derived from experimental results)												*/
/*																									*/
/****************************************************************************************************/
static	LONG	autoNotchFreqComp( ANOTCHSEQ *ANotchSeq, LONG DFreq )
{
	LONG	Freq;

	/* Correction frequency initialization */
	Freq = DFreq;

	if( ANotchSeq->conf.TuneLessSts == TRUE )
	{ /* Adjustmentless enabled */
		if( DFreq < NOTCH_COMP_MAX_FREQ )
		{
			Freq = DFreq;
		}
		else
		{
			/* Frequency analysis resolution×Correct 2(Detection error) */
			Freq = DFreq + (FREQDETUNIT << 1);
		}
	}

	if( Freq < 0 )
	{
		Freq = 0;
	}

	return( Freq );
}



/****************************************************************************************************/
/*																									*/
/*		Automatic notch parametersEEPROMWriting process														*/
/*																									*/
/****************************************************************************************************/
static	PRM_RSLT	autoNotchPrmWriteEeprom( REGIF_CONFIG_T *RegCfg, BOOL EepromWriteProtect )
{
	PRM_RSLT	rc = PRM_RSLT_SUCCESS;

	if( EepromWriteProtect == FALSE )
	{ /* EEPROMWriting prohibited(AATMedium) */
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil1, 0 );	/* Pn409 */
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchq1, 0 );		/* Pn40A */
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchfil2, 0 );	/* Pn40C */
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_ntchq2, 0 );		/* Pn40D */
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_trqfil11, 0 );	/* Pn401 */
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
		rc = PrmStoreValueToEeprom( RegCfg, &pndef_tfuncsw, 0 );	/* Pn408 */
		if( rc != PRM_RSLT_SUCCESS )
		{
			return(rc);
		}
	}
	return	rc;
}



/****************************************************************************************************/
/*																									*/
/*		Initializing the first notch release statusAPI															*/
/*																									*/
/****************************************************************************************************/
void	RpiResetStatusANotch1( ANOTCHSEQ *ANotchSeq )
{
	ANotchSeq->var.ANotchStat = OPTUNE_STS_NT_NODETECT;			/* Vibration not detected status setting			*/
	ANotchSeq->var.ANotch1Stat = OPTUNE_STS_NT1_NODETECT;		/* 1Stage status initialized to unimplemented	*/
	ANotchSeq->var.ANotchPreStat = OPTUNE_PSTS_NODETECT;		/* Initialize last notch to not executed		*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Second notch release status initializationAPI															*/
/*																									*/
/****************************************************************************************************/
void	RpiResetStatusANotch2( ANOTCHSEQ *ANotchSeq )
{
	ANotchSeq->var.ANotchStat = OPTUNE_STS_NT_NODETECT;			/* Vibration not detected status setting			*/
	ANotchSeq->var.ANotch2Stat = OPTUNE_STS_NT2_NODETECT;		/* 2Stage status initialized to unimplemented	*/
	ANotchSeq->var.ANotchPreStat = OPTUNE_PSTS_NODETECT;		/* Initialize last notch to not executed		*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Get auto notch last setting notch statusAPI													*/
/*																									*/
/****************************************************************************************************/
UCHAR	RpiGetANotchPreStat( ANOTCHSEQ *ANotchSeq )
{
	return	ANotchSeq->var.ANotchPreStat;
}



/****************************************************************************************************/
/*																									*/
/*		Automatic notch status acquisitionAPI																	*/
/*																									*/
/****************************************************************************************************/
USHORT	RpiGetANotchSetStat( ANOTCHSEQ *ANotchSeq )
{
	return	ANotchSeq->var.ANotchSetStat;
}



/****************************************************************************************************/
/*																									*/
/*		Automatic notch status acquisitionAPI(Registeri/F)													*/
/*																									*/
/****************************************************************************************************/
void	RpiSetRegSpatANotchState( ANOTCHSEQ *ANotchSeq )
{
	USHORT	uswk;

	uswk = (ANotchSeq->var.ANotchPreStat << 12)
			+ (ANotchSeq->var.ANotch2Stat << 8)
			+ (ANotchSeq->var.ANotch1Stat << 4)
			+ (ANotchSeq->var.ANotchStat);

	ANotchSeq->OpeReg.SpatANotchState = uswk;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Forced frequency analysis processing request set																*/
/*																									*/
/****************************************************************************************************/
void	RpiANotchFftReqSet( ANOTCHSEQ *ANotchSeq, FFTANALYZE *FftAna )
{
	ANotchSeq->var.FftReq = TRUE;
	ANotchSeq->var.ANotchStat = OPTUNE_STS_NT_DETECT;

	/* Forced frequency analysis request */
	FftSetForceAnaReq( FftAna, TRUE );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Torque filter calculation																			*/
/*																									*/
/****************************************************************************************************/
void	RpiANotchTrqFilCalc( ANOTCHSEQ *ANotchSeq, AUTONOTCH *ANotch, REGIF_CONFIG_T *RegCfg, PRMDATA *Prm )
{
	TuneNotchQ( ANotch, RegCfg, ANotchSeq->conf.kv, ANotchSeq->conf.jrate, 1, Prm );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Vibration detection forced invalidation request set		3seconds											*/
/*																									*/
/****************************************************************************************************/
void	RpiANotchDisableReqSet( ANOTCHSEQ *ANotchSeq )
{
	ANotchSeq->var.NotchChk = TRUE;									/* Vibration detection wait flagON			*/
	KlibRstLongTimer( &ANotchSeq->var.NotchChkTimer );				/* Vibration detection wait timer reset	*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Data setting for auto notch filter settingAPI														*/
/*																									*/
/****************************************************************************************************/
void	AutoNotchSetCondition( ANOTCHSEQ *ANotchSeq, BOOL EepromWriteProtect,
										BOOL BaseEnableSts, BOOL TuneLessSts, BOOL OverTravelSts )
{
	/* Setting each execution condition */
	ANotchSeq->conf.EepromWriteProtect = EepromWriteProtect;		/* EEPROMWrite prohibition flag		*/
	ANotchSeq->conf.BaseEnableSts = BaseEnableSts;					/* ServoON/OFFStatus flag		*/
	ANotchSeq->conf.TuneLessSts = TuneLessSts;						/* Adjustmentless enabled/invalid flag		*/
	ANotchSeq->conf.OverTravelSts = OverTravelSts;					/* P-OT/N-OTStatus flag			*/
}



/***************************************** end of file **********************************************/

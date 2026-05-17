/****************************************************************************************************/
/*																									*/
/*																									*/
/*		RippleComp.c : Ripple compensation module														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2013.09.25	Y.Oka	For Mercury												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include	"RippleComp.h"
#include 	"BaseCmnStruct.h"

/****************************************************************************************************/
/*																									*/
/*		Constant definition																					*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Ripple correction order definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	RIPCOMPORDER_MECHA		RPLCMP_ORDER_MECHA		/* Ripple correction maximum order					*/
#define	RIPCOMPORDER_ELE		RPLCMP_ORDER_ELE		/* Ripple correction maximum order					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Ripple spectrum analysis mode																*/
/*--------------------------------------------------------------------------------------------------*/
#define	RPLSP_ANALYZE_STOP	0					/* Stop ripple spectrum measurement					*/
#define	RPLSP_ANALYZE_SPD	2					/* Speed ripple spectrum measurement					*/
#define	RPLSP_ANALYZE_TRQ	3					/* Torque ripple spectrum measurement					*/

/*--------------------------------------------------------------------------------------------------*/
/*		 Speed ripple compensation information write mode															*/
/*--------------------------------------------------------------------------------------------------*/
#define	MOTOR_SN_DATA_SIZE	8					/* Motor serialNo.Data size[word]			*/
#define	SN_READ_SUCCESS		0x0000				/* Motor serialNo.Read normal completion				*/
#define	SN_READ_ERROR		0x0001				/* Motor serialNo.Read error completed				*/
#define	SN_DIFFERENT		0x0002				/* Motor serialNo.is different					*/


/****************************************************************************************************/
/*		Local Function Prototype Definition															*/
/****************************************************************************************************/


/****************************************************************************************************/
/*																									*/
/*		Speed ripple compensation																			*/
/*																									*/
/*	Overview:	Ripple information saved in parameters(Amplitude, phase), and use		*/
/*			Reducing speed ripple by adding the compensation torque signal to the torque command as an FF term		*/
/*																									*/
/*			Compensation torque	= Σ(Correction amplitude * sin(Motor phase * Order + Phase correction value))						*/
/*																									*/
/* arg		: USHORT	MotPhase		:	Motor phase			[65536/360deg]						*/
/* out		: LONG		TrqComp			:	Torque correction value		[2^24/MaxTrq]						*/
/* g-out	: 																						*/
/*																									*/
/****************************************************************************************************/
LONG	RippleCompensation( RPLCMP *RippleCmp, USHORT MechaPhase, USHORT MotPhase, BOOL BaseEnable )
{
	RPLCMP_PRM			*RippleCmpP;
//	RPLCMP_PRM_MECHA 	*RippleCmpP_mecha;	/*<S1FD>*/
	RPLCMP_PRM_ELE 		*RippleCmpP_ele;

	LONG				i, lwk, TrqComp;
//	LONG				TrqCompMecha		/*<S1FD>*/
	LONG				TrqCompEle;
	USHORT				uswk[RPLCMP_ORDER_MECHA];
	BOOL				RippleCompMon;

	RippleCmpP       = &(RippleCmp->RplCmpPrm);
//	RippleCmpP_mecha = &(RippleCmp->RplCmpPrm.RplCmpPrmMecha);/*<S1FD>*/
	RippleCmpP_ele   = &(RippleCmp->RplCmpPrm.RplCmpPrmEle);

/*--------------------------------------------------------------------------------------------------*/
/*		Mechanical angle correction																					*/
/*--------------------------------------------------------------------------------------------------*/
#if 0	/* Speed ripple compensation Mechanical angle correction processing(Commented out for future use) */
	lwk = 0;
	for(i=0; i<RIPCOMPORDER_MECHA; i++)
	{
		/*------------------------------------------------------------------------------------------*/
		/*		Phase calculation																			*/
		/*------------------------------------------------------------------------------------------*/
		uswk[i]  = RippleCmpP_mecha->PhaseOfset[i]  + (MechaPhase * RippleCmpP_mecha->CompFrq[i]);
		uswk[i]  = uswk[i] >> 6;									/* [65536/360] --> [1024/360]	*/

		/*------------------------------------------------------------------------------------------*/
		/*		Correction torque calculation																		*/
		/*																							*/
		/*	[15000/MaxTrq] >>14 is required, but to reduce processing time,						*/
		/*	[2^24/ MaxTrq]										*/
		/*------------------------------------------------------------------------------------------*/
		lwk += (RippleCmpP_mecha->CompAmpValue[i] * MlibFASTSINS16(uswk[i]));
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Correction torque normalization calculation[15000/MaxTrq] -> [2^24/MaxTrq]										*/
	/*																								*/
	/*		MlibMulhigh32is used to generate15000->2^24To perform normalization conversion <<16 is required, but in the above process			*/
	/*		It should be processed originally >>14 is not implemented, so perform <<2 (Processing time countermeasures)				*/
	/*----------------------------------------------------------------------------------------------*/
	lwk = MlibMulhigh32( (lwk << 2), 73300775 );					/*	[2^24/MaxTrq]				*/
	TrqCompMecha = MlibMulgainNolim( lwk, RippleCmpP_mecha->CompGain );
#endif	/* Speed ripple compensation Mechanical angle correction processing(Commented out for future use) */

	lwk = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		Electrical angle correction																					*/
/*--------------------------------------------------------------------------------------------------*/
	for(i=0; i<RIPCOMPORDER_ELE; i++)
	{
		/*------------------------------------------------------------------------------------------*/
		/*		Phase calculation																			*/
		/*------------------------------------------------------------------------------------------*/
		uswk[i]  = RippleCmpP_ele->PhaseOfset[i]  + (MotPhase * RippleCmpP_ele->CompFrq[i]);
		uswk[i]  = uswk[i] >> 6;									/* [65536/360] --> [1024/360]	*/

		/*------------------------------------------------------------------------------------------*/
		/*		Correction torque calculation																		*/
		/*																							*/
		/*	[15000/MaxTrq] >>14 is required, but to reduce processing time,						*/
		/*	[2^24/ MaxTrq]										*/
		/*------------------------------------------------------------------------------------------*/
		lwk += (RippleCmpP_ele->CompAmpValue[i] * MlibFASTSINS16(uswk[i]));

	}

#if 0
	lwk = RippleCmp->var.TrqComp[0] + RippleCmp->var.TrqComp[1] + RippleCmp->var.TrqComp[5];
	if(RippleCmpP->RplCmpMode6f == TRUE)
	{ /* 1~6When the next usage mode is selected */
		lwk = lwk + RippleCmp->var.TrqComp[2] + RippleCmp->var.TrqComp[3] + RippleCmp->var.TrqComp[4];
	}
#endif
	/*----------------------------------------------------------------------------------------------*/
	/*		Correction torque normalization calculation[15000/MaxTrq] -> [2^24/MaxTrq]										*/
	/*																								*/
	/*		MlibMulhigh32is used to generate15000->2^24To perform normalization conversion <<16 is required, but in the above process			*/
	/*		It should be processed originally >>14 is not implemented, so perform <<2 (Processing time countermeasures)				*/
	/*----------------------------------------------------------------------------------------------*/
	lwk = MlibMulhigh32( (lwk << 2), 73300775 );					/*	[2^24/MaxTrq]				*/
	TrqCompEle = MlibMulgainNolim( lwk, RippleCmpP_ele->CompGain );

/*--------------------------------------------------------------------------------------------------*/
/*		Correction torque calculation	(Mechanical angle correction torque + Electrical angle correction torque)									*/
/*--------------------------------------------------------------------------------------------------*/
//	RippleCmp->var.TrqCompAll = TrqCompMecha + TrqCompEle;	// Speed ripple compensation Mechanical angle correction processing(Commented out for future use)
	RippleCmp->var.TrqCompAll = TrqCompEle;

/*--------------------------------------------------------------------------------------------------*/
/*		Correction execution judgment																				*/
/*--------------------------------------------------------------------------------------------------*/
//	if( RippleCmpP->RplCmpUse == TRUE)/*<S1FD>*/
	if( (RippleCmpP->RplCmpUse == TRUE) && (RippleCmp->var.RippleCmpExe == TRUE) )
	{	/* Correction valid	*/
		TrqComp = RippleCmp->var.TrqCompAll;
		RippleCompMon = TRUE;
	}
	else
	{	/* Correction invalid	*/
		TrqComp = 0;
		RippleCompMon = FALSE;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Processing during base block																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseEnable == FALSE )
	{
		RippleCmp->var.TrqCompAll = 0;
		TrqComp = 0;
		RippleCompMon = FALSE;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Speed ripple compensation monitor storage																	*/
/*--------------------------------------------------------------------------------------------------*/
	RippleCmp->RippleCompMon    = RippleCompMon;
	RippleCmp->RippleTrqCompMon = TrqComp;

	return(TrqComp);

}

/****************************************************************************************************/
/*																									*/
/*		Checking the conditions for enabling the speed ripple compensation function														*/
/*																									*/
/****************************************************************************************************/
void	RippleCompChkThresholdVal( RPLCMP *RippleCmp, USHORT CtrlMode, LONG SpdRef, LONG SpdFb, BOOL TuneLessCtrl )
{
	USHORT	RippleCmpThresholdSel;
	BOOL	RippleCmpExe;

	RippleCmpExe = FALSE;
	/*----------------------------------------------------------------------------------------------*/
	/*	Threshold conditions for enabling the speed ripple compensation function(Pn423.2=0(Command speed)/1(Feedback speed))			*/
	/*		.Position control mode	:Pn423.2														*/
	/*		.Speed control mode	:Pn423.2														*/
	/*		.Torque control mode	: Force1(Feedback speed),								*/
	/*----------------------------------------------------------------------------------------------*/
	if( CtrlMode== CTRLMODE_TRQ )	/* Torque control mode */
	{
		RippleCmpThresholdSel = FB_SPD;
	}
	else
	{
		RippleCmpThresholdSel = RippleCmp->RippleCmpThresholdSel;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Sign processing																					*/
	/*----------------------------------------------------------------------------------------------*/
	if( SpdRef < 0 )
	{
		SpdRef = -SpdRef;
	}
	if( SpdFb < 0 )
	{
		SpdFb = -SpdFb;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Judgment processing																					*/
	/*----------------------------------------------------------------------------------------------*/
	if( RippleCmp->VelRipplePhaseReady == TRUE ) 	/* Magnetic pole detection complete(Phase detection completed) */
	{
		if( RippleCmpThresholdSel == CMD_SPD )
		{	/* Threshold conditions for enabling the speed ripple compensation function = "Command speed" */
			if( RippleCmp->VelRippleCompSpd <= SpdRef )
			{
				RippleCmpExe = TRUE;
			}
			else
			{
				RippleCmpExe = FALSE;
			}
		}
		else
		{/* Threshold conditions for enabling the speed ripple compensation function = "Feedback speed" */
			if( RippleCmp->VelRippleCompSpd <= SpdFb )
			{
				RippleCmpExe = TRUE;
			}
			else
			{
				RippleCmpExe = FALSE;
			}
		}
	}
	else	/* Magnetic pole detection not completed(Phase detection incomplete) */
	{
		RippleCmpExe = FALSE;
	}

	/* No speed ripple compensation when adjustmentless is enabled */
	if(TuneLessCtrl == TRUE)
	{
		RippleCmpExe = FALSE;
	}

	/* Stores valid condition check result */
	RippleCmp->var.RippleCmpExe = RippleCmpExe;
}

#if 0	/* Monitor output selection *//* @@Currently not supported */
/****************************************************************************************************/
/*																									*/
/*		Ripple compensation monitor output selection acquisitionAPI															*/
/*																									*/
/****************************************************************************************************/
LONG	RippleCompGetMonOutSel( RPLCMP_PRM *RippleCmpP )
{
	return	(RippleCmpP->RplCmpDobsMon);
}
#endif	/* Monitor output selection *//* @@Currently not supported */

/****************************************************************************************************/
/*																									*/
/*		Speed ripple compensation parameter calculation(Electrical angle)														*/
/*																									*/
/****************************************************************************************************/
BOOL	CalVelRippleCompE( RPLCMP *RippleCmp, BPRMDAT *Bprm, USHORT RippleCompGainE, 
							USHORT *RippleCompCycleE, SHORT *RippleCompAmplitudeE, USHORT *RippleCompPhaseE)
{
//	RPLCMP_PRM	wrkp;	/*<S1FD>*/
	USHORT		RippleCompCycle_ele[6];
	SHORT		RippleCompAmplitude_ele[6];
	USHORT		RippleCompPhase_ele[6];

	LONG		i;
//	BOOL		err;

//	err = FALSE;

	/*----------------------------------------------------------------------------------------------*/
	/*		Speed ripple compensation parameter copy														*/
	/*----------------------------------------------------------------------------------------------*/
	for( i = 0; i < RIPCOMPORDER_ELE; i++ )
	{	/* Electrical angle correction */
		RippleCompCycle_ele[i]     = RippleCompCycleE[i];
		RippleCompAmplitude_ele[i] = RippleCompAmplitudeE[i];
		RippleCompPhase_ele[i]     = RippleCompPhaseE[i];
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Electrical angle correction parameter 																		*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Correction gain	[%] 																		*/
	/*----------------------------------------------------------------------------------------------*/
	RippleCmp->RplCmpPrm.RplCmpPrmEle.CompGain = MlibScalKxgain( RippleCompGainE, 1, 100, NULL, 24 );

	for(i=0; i<RIPCOMPORDER_ELE; i++)
	{
		RippleCmp->RplCmpPrm.RplCmpPrmEle.CompFrq[i] = RippleCompCycle_ele[i];
		/*------------------------------------------------------------------------------------------*/
		/*		Correction amplitude calculation	[0.1%] --> [15000/MaxTrq]											*/
		/*------------------------------------------------------------------------------------------*/
		RippleCmp->RplCmpPrm.RplCmpPrmEle.CompAmpValue[i]  = BprmTorqueLevelCal( Bprm,
														(LONG)(RippleCompAmplitude_ele[i]), 1 );

		/*------------------------------------------------------------------------------------------*/
		/*		Calculate the phase data	[deg] --> [65536/360]												*/
		/*------------------------------------------------------------------------------------------*/
		RippleCmp->RplCmpPrm.RplCmpPrmEle.PhaseOfset[i]  = (USHORT)(SHORT)MlibScalKxgain
									((LONG)((SHORT)RippleCompPhase_ele[i]), 65536, 360, NULL,  -24 );
	}

//	return	(err);
	return	TRUE;
}

/****************************************************************************************************/
/*																									*/
/*		SigmaWin+(Electrical angle correction)	*/
/*																									*/
/****************************************************************************************************/
void	FnRegCalVelRippleCompE( BPRMDAT *Bprm, RPLCMP *RippleCmp )/*<S1FD>*/
{
//	RPLCMP_PRM	wrkp;		/*<S1FD>*/
	USHORT		RippleCompCycle_ele[6];
	SHORT		RippleCompAmplitude_ele[6];
	USHORT		RippleCompPhase_ele[6];
//	BPRMDAT		*Bprm;		/*<S1FD>*/
//	RPLCMP		*RippleCmp;	/*<S1FD>*/
	LONG		i;

//	Bprm = Axis->UniPrms->Bprm;/*<S1FD>*/
//	RippleCmp = &(Axis->BaseLoops->RippleCmp);


	/*----------------------------------------------------------------------------------------------*/
	/*		Ripple compensation parameter copy															*/
	/*----------------------------------------------------------------------------------------------*/
	for( i = 0; i < RIPCOMPORDER_ELE; i++ )
	{	/* Electrical angle correction */
		RippleCompCycle_ele[i]     = RippleCmp->RplCmpPrm.RplCompCycleEle[i];
		RippleCompAmplitude_ele[i] = RippleCmp->RplCmpPrm.RplCompAmplitudeEle[i];
		RippleCompPhase_ele[i]     = RippleCmp->RplCmpPrm.RplCompPhaseEle[i];
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Electrical angle correction parameter 																		*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Correction gain	[%] 																		*/
	/*----------------------------------------------------------------------------------------------*/
	RippleCmp->RplCmpPrm.RplCmpPrmEle.CompGain = MlibScalKxgain( RippleCmp->RplCmpPrm.RplCompGainEle,
																					1, 100, NULL, 24 );

	for(i=0; i<RIPCOMPORDER_ELE; i++)
	{
		RippleCmp->RplCmpPrm.RplCmpPrmEle.CompFrq[i] = RippleCompCycle_ele[i];
		/*------------------------------------------------------------------------------------------*/
		/*		Correction amplitude calculation	[0.1%] --> [15000/MaxTrq]											*/
		/*------------------------------------------------------------------------------------------*/
		RippleCmp->RplCmpPrm.RplCmpPrmEle.CompAmpValue[i]  = BprmTorqueLevelCal( Bprm,
														(LONG)(RippleCompAmplitude_ele[i]), 1 );

		/*------------------------------------------------------------------------------------------*/
		/*		Calculate the phase data	[deg] --> [65536/360]												*/
		/*------------------------------------------------------------------------------------------*/
		RippleCmp->RplCmpPrm.RplCmpPrmEle.PhaseOfset[i]  = (USHORT)(SHORT)MlibScalKxgain
									((LONG)((SHORT)RippleCompPhase_ele[i]), 65536, 360, NULL,  -24 );
	}

//	return;	/*<S1FD>*/
}

/****************************************************************************************************/
/*																									*/
/*		Serial of this time connected motor and last time connected motorNo. 											*/
/*																									*/
/****************************************************************************************************/
ULONG ChkMotorSerialNo( UINT AxisNo, MENCV *MencV )/*<S1FD>*/
{
//	EEP_DEVICE_HANDLE	*EepDevice; /* <S1D2> */
	ULONG				rc;
	ULONG				ReadRslt;
//	UINT				AxisNo;/*<S1FD>*/
	USHORT				SerialNo[MOTOR_SN_DATA_SIZE];
	USHORT				Lst_SerialNo[MOTOR_SN_DATA_SIZE];
	LONG				ArrayIdx;

	/* Initializing variables */
	rc = SN_READ_SUCCESS;
//	EepDevice = (Axis->RegManager->hPnReg)->hEepDev; /* <S1D2> */
//	AxisNo = Axis->AxisNo;/*<S1FD>*/

	/* Serial of this time connected motor and last time connected motorNo. */
	for(ArrayIdx = 0;  ArrayIdx < MOTOR_SN_DATA_SIZE; ArrayIdx++)
	{
		/* This time connected motor serialNo.Acquisition */
		SerialNo[ArrayIdx] = (USHORT)(MencV->MotInfo.SNo[ArrayIdx*2 + 1] & 0x00FF);
		SerialNo[ArrayIdx] |= (USHORT)((MencV->MotInfo.SNo[ArrayIdx*2] & 0x00FF) << 8);

		/* Serial of last connected motorNo.Acquisition */
		/* read from EEPROM */
		ReadRslt = EepdevReadValues(															/* <S1D2> */
								0,																/* EEPROM Device ID(0: SPI(IF), 1: I2C(SDA) */
								EEP_NOSUM_ADDRESS(AxisNo, EEP_MOTORSN_OFS) + (ArrayIdx*2),		/* Eeprom Address in bytes */
								&Lst_SerialNo[ArrayIdx],										/* Pointer to the buffer for reading */
								1 );															/* reading size in words */
		if(ReadRslt == SN_READ_SUCCESS)	/* Serial of last connected motorNo.Read normal completion */
		{
			if(Lst_SerialNo[ArrayIdx] != SerialNo[ArrayIdx] )	/* Serial of last connected motor and currently connected motorNo.is different */
			{
				rc = SN_DIFFERENT;
				break;
			}
		}
		else	/* Serial of last connected motorNo.Read error completed */
		{
				rc = SN_READ_ERROR;
				break;
		}
	}

	return(rc);
}

/****************************************************************************************************/
/*																									*/
/*		Speed ripple compensation information mismatch warning status acquisitionAPI												*/
/*																									*/
/****************************************************************************************************/
BOOL	RippleCompGetRippleWrnState( RPLCMP *RippleCmp )/*<S1FD>*/
{
	return	(RippleCmp->var.RippleWrnState);
}

/***************************************** end of file **********************************************/

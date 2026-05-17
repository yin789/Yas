/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ini_sub.c : SVNInitialization-related subfunction module													*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : SVNDefine initialization-related subfunctions															*/
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
/*	Note	:	First edition	2009.06.04	Y.Oka															*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "JL086ServoIpHw.h"
//#include "PowerManager.h"
#include "MLib.h"
#include "KLib.h"
#include "MicroPrmCalc.h"

/* defines */
#define	D_AXIS			0
#define	Q_AXIS			1

#define	REQ_LMT_CHK		1
#define	NO_LMT_CHK		0



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void KpxPcalMicroProgram( UNI_PRM_STR *uni_prm_ptr, ASICS *SvAsicRegs,							*/
/*													USHORT *mreg_csw_ptr, LONG caller_axis_no );	*/
/*	void KpiPcalMicroNotchFilter2( ASICS *SvAsicRegs, LONG hz, LONG qx, LONG dx, SHORT *MpReg );	*/
/*	LONG KpiChangePwmFrequency( ULONG PwmFx, PRMDATA *prm_ptr );									*/
/*	LONG LpxSetPrmToASIC( SHORT *adr, LONG prm, BOOL limchk );										*/
/*	LONG MicroTranslatePrmReq( ASICS *SvAsicRegs, void *Data, VUSHORT *Address, LONG Size );		*/
/****************************************************************************************************/
static LONG	PcalDqAcrKc( LONG Iadmax, USHORT imax, USHORT imaxs );
static LONG	PcalLDidt( KSGAIN Le, KSGAIN max_cur, KSGAIN vdc );
static LONG	PcalDqLxComp( KSGAIN Lx, KSGAIN over_spd, KSGAIN max_cur, KSGAIN vdc );
static LONG	PcalDqRxComp( KSGAIN Rx, KSGAIN max_cur, KSGAIN vdc );
static LONG	PcalDqEmfComp( KSGAIN Emf, KSGAIN over_spd, KSGAIN vdc );
static LONG	PcalOndelayLevel( LONG Level );
static LONG	PcalOndelaySlope( LONG Slope );
static LONG	PcalPwmFrequency( LONG PwmFx );
static	LONG	PcalPwmCounter( LONG CarrCnt );		/* Motor carrier frequency switching<S117> */
static LONG	PcalVFBKiLim( LONG TvLim );
static void	KpiPcalCurrentObs( ASICS *SvAsicRegs, USHORT ignq, USHORT pwmf, BPRMDAT *Bprm, SHORT *MpReg);
static void	KpiPcalMicroLpassFilter1( LONG tf, LONG ts, SHORT *MpReg );



/****************************************************************************************************/
/*																									*/
/*		μParameters calculation main function for program														*/
/*																									*/
/****************************************************************************************************/
LONG	KpxPcalMicroProgram( MAICRO_CFGPRM *uCfgPrm, BPRMDAT *Bprm,
							 ASICS *SvAsicRegs, WEAKENFIELD* WeakenField )
{
	ASIC_U_IF_STR	*pAsicMicroReg;
	LONG			kp, kx;
	LONG			rc;

#if (FLOAT_USE==FALSE)
	LONG			sx;
#endif /* FLOAT_USE */


	pAsicMicroReg = SvAsicRegs->AsicMicroReg;
	rc  = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		Setting the number of control axes																<S0E4>			*/
/*--------------------------------------------------------------------------------------------------*/
	SvAsicRegs->AsicMicroReg->AxisInfo.AxisNum = MAX_AXIS_NUMBER;

/*--------------------------------------------------------------------------------------------------*/
/*		Control flag(CSW)																		*/
/*--------------------------------------------------------------------------------------------------*/
	SvAsicRegs->MicroCsw.data = (USHORT)( uCfgPrm->dq_sw | 0x00F0 );			/* Control flag		*/
//	if( uCfgPrm->pwmf > 4000 )
	kx = PcalPwmCounter( uCfgPrm->NormCarrCnt );		/* Reverse calculation of frequency from setting carrier counter value	*//* Motor carrier frequency switching<S117> */
	if( kx > 4000 )
	{
		/* Carrier frequency4kHzor above */
		SvAsicRegs->MicroCsw.data &= 0xFFF7;
	}

	if(( uCfgPrm->flg_wf & 0x0001 ) == 0 )
	{ /* Field weakening enable/disable check on motor side */
		/* Forcibly disable if motor side is disabled */
		SvAsicRegs->MicroCsw.data &= 0xF7FF;
	}

	if( ( SvAsicRegs->MicroCsw.data & 0x0800 ) == 0 )
	{
		/* Field weakening=OFF, field weakening2=OFF, */
		SvAsicRegs->MicroCsw.data &= 0xDFFF;
	}

	if( ( SvAsicRegs->MicroCsw.data & 0x2000 ) != 0 )
	{
		/* Field weakening2=ON: Integral stop function=ON, */
		SvAsicRegs->MicroCsw.data |= 0x4000;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Current conversion gain & voltage correction gain																*/
/*--------------------------------------------------------------------------------------------------*/
	/* Current conversion gain */
	kx = PcalDqAcrKc( uCfgPrm->shant, uCfgPrm->imax, uCfgPrm->imaxs );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.Kc), kx, REQ_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* Voltage compensation gain */
	kx = uCfgPrm->vrevgn;
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.KvvIn), kx, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		dAxis current loop gain(Proportional & integral & derivative)														*/
/*--------------------------------------------------------------------------------------------------*/
	/* Proportional gain */
	kp = PcalDqAcrKp( uCfgPrm->ignd, 100, Bprm, D_AXIS );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.KdP), kp, REQ_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* Integral gain */
	kx = PcalDqAcrKi( uCfgPrm->kid, kp, Bprm->MotLd, Bprm );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.KdI), kx, REQ_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* Integral limit value */
	kx = uCfgPrm->limintd;
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.VdLim), kx, NO_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* L(di/dt)ofL */
	kx = PcalLDidt( Bprm->MotLd, Bprm->MaxCur, Bprm->Vdc );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.L_dIdt), kx, REQ_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		qAxis current loop gain(Proportional & integral & derivative)														*/
/*--------------------------------------------------------------------------------------------------*/
	/* Proportional gain */
	kp = PcalDqAcrKp( uCfgPrm->ignq, 100, Bprm, Q_AXIS );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.KqP), kp, REQ_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* Integral gain */
	kx = PcalDqAcrKi( uCfgPrm->kiq, kp, Bprm->MotLq, Bprm );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.KqI), kx, REQ_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* Integral limit value */
	kx = uCfgPrm->limintq;
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.VqLim), kx, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		Current loop output filter time constant																*/
/*--------------------------------------------------------------------------------------------------*/
	/* Output filter gain */
	kx = PcalDqAcrFilter( uCfgPrm->ifil );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.Tfil), kx, REQ_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		Inductance compensation constant(Ld,Lq)																*/
/*--------------------------------------------------------------------------------------------------*/
	/* dAxis inductance */
	kx = PcalDqLxComp( Bprm->MotLd, Bprm->OvrSpd, Bprm->MaxCur,  Bprm->Vdc );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.Ld), kx, REQ_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* qAxis inductance */
	kx = PcalDqLxComp( Bprm->MotLq, Bprm->OvrSpd, Bprm->MaxCur,  Bprm->Vdc );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.Lq), kx, REQ_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		Mechanical angle → Electrical angle conversion coefficient																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Mechanical angle → Electrical angle conversion coefficient */
#if (FLOAT_USE==TRUE) 
	kx = (LONG)Bprm->Keangle;
#else
	kx = MlibScalKskxkx( Bprm->Keangle, 1, 1, &sx, -24 );
#endif /* FLOAT_USE */
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdV.KEangle), kx, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		Armature resistance(R1)Compensation constant																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* R1Compensation constant */
	kx = PcalDqRxComp( Bprm->MotR, Bprm->MaxCur, Bprm->Vdc );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.MotResist), kx, REQ_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		EMFCompensation constant(Φ)																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* EMFCompensation constant */
	kx = PcalDqEmfComp( Bprm->MotEmf, Bprm->OvrSpd, Bprm->Vdc );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.Mag), kx, REQ_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		On-delay compensation																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* On-delay compensation time */
	kx = uCfgPrm->odt_c.b.h * (ASIC_CLKMHZ/2) / 10;
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.OnDelayComp),  kx, NO_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* On-delay compensation switching level */
	kx = PcalOndelayLevel( uCfgPrm->odlv_slp.b.l );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.OnDelayLvl), kx, NO_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* On-delay compensation slope */
	kx = PcalOndelaySlope( uCfgPrm->odlv_slp.b.h );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.OnDelaySlope), kx, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		Current detection zero adjustment																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* UPhase current zero adjustment */
	kx = (SHORT)(uCfgPrm->zadjiu);
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->AdinV.IuOffsetIn), kx, NO_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* VPhase current zero adjustment */
	kx = (SHORT)(uCfgPrm->zadjiv);
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->AdinV.IvOffsetIn), kx, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		Current detection gain adjustment																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* UPhase current gain adjustment */
	kx = (CHAR)uCfgPrm->gadjiu_v.b.l + 0x100;
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->AdinV.KcuIn), kx, NO_LMT_CHK );
	/*----------------------------------------------------------------------------------------------*/
	/* VPhase current gain adjustment */
	kx = (CHAR)uCfgPrm->gadjiu_v.b.h + 0x100;
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->AdinV.KcvIn), kx, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		PWMCarrier frequency count value																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* PWMCarrier frequency */
//	kx =  KpiChangePwmFrequency( 0, uCfgPrm->pwmf, uCfgPrm->pwmf3, uCfgPrm->flg_wf );
////	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.CrFreq), kx, NO_LMT_CHK );
	rc |= LpxSetPrmToASIC( (SHORT*)&(pAsicMicroReg->IntAdP.CrFreq), uCfgPrm->NormCarrCnt, NO_LMT_CHK );/*<S0C7>*//* Motor carrier frequency switching<S117> */

/*--------------------------------------------------------------------------------------------------*/
/*		Current loop voltage limit value																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* Voltage limit value */
	kx = uCfgPrm->lmdata;
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.Vmax), kx, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		Torque filter : First-order lag filter 1(Not used,As a precaution)										*/
/*--------------------------------------------------------------------------------------------------*/
//	KpiManageMicroTorqueFilter( CSW_LPFDSABL, FALSE, mreg_csw_ptr, pAsicMicroReg );
	SvAsicRegs->MicroCsw.data |= LPFDSABL;
	KpiPcalMicroLpassFilter1( 0, KPI_MACYCLENS, &(pAsicMicroReg->IntAdP.TLpf) );

#if 0 /* 2012.08.31 Y.Oka Commented out because the function is disabled */
/*--------------------------------------------------------------------------------------------------*/
/*		Torque filter : First-order lag filter 2(Pn411,SGDX.,As a precaution)							*/
/*--------------------------------------------------------------------------------------------------*/
//	KpiManageMicroTorqueFilter( CSW_LPFCDSABL, FALSE, mreg_csw_ptr, pAsicMicroReg  );
	SvAsicRegs->MicroCsw.data |= LPFCDSABL;
	KpiPcalMicroLpassFilter1( 0, KPI_MACYCLENS, &(pAsicMicroReg->MREG_TLPFC) );
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Notch filter for motor resonance suppression																*/
/*--------------------------------------------------------------------------------------------------*/
	if ( (uCfgPrm->motresfrq != 0) && (uCfgPrm->motresq != 0) )
	{ /* Setting frequency≠0, notch filter enabled */
	/*----------------------------------------------------------------------------------------------*/
		KpiPcalMicroNotchFilter2(						/* 2nd order notch filter parameter calculation		*/
			SvAsicRegs,									/* JL-076Access structure					*/
			uCfgPrm->motresfrq,							/* Filter frequency		[Hz]				*/
			uCfgPrm->motresq,							/* Filter Q constant		[0.01]				*/
			0,											/* Filter depth			[0.001]				*/
			&(pAsicMicroReg->IntAdP.Kf31) );			/* Filter parameter setting start address		*/
	/*----------------------------------------------------------------------------------------------*/
		/* Notch filter 3 enabled */
//		KpiManageMicroTorqueFilter( CSW_F3DSABL, TRUE, mreg_csw_ptr, pAsicMicroReg );
		SvAsicRegs->MicroCsw.data &= ~F3DSABL;
	}
	else
	{
		/* Notch filter 3 disabled */
//		KpiManageMicroTorqueFilter( CSW_F3DSABL, FALSE, mreg_csw_ptr, pAsicMicroReg );
		SvAsicRegs->MicroCsw.data |= F3DSABL;
	}

	/* Set to ASIC */
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->IntAdP.CtrlSw), SvAsicRegs->MicroCsw.data, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		Voltage loop integration limit																		*/
/*--------------------------------------------------------------------------------------------------*/
	kx = PcalVFBKiLim( uCfgPrm->idlim );
	rc |= LpxSetPrmToASIC( &(pAsicMicroReg->WeakFV.WfIntegLim), kx, NO_LMT_CHK );

/*--------------------------------------------------------------------------------------------------*/
/*		dAxis current command limit value calculation constant														*/
/*--------------------------------------------------------------------------------------------------*/
	PcalIdrefLimTerm234( WeakenField, SvAsicRegs, Bprm, uCfgPrm->oslv_pol, uCfgPrm->pnvoltgn );

/*--------------------------------------------------------------------------------------------------*/
/*		Current observer parameter calculation																*/
/*--------------------------------------------------------------------------------------------------*/
	/* Parameter settings */
//	KpiPcalCurrentObs( SvAsicRegs, uCfgPrm->ignq, uCfgPrm->pwmf, Bprm, &(pAsicMicroReg->DobsP.TsPerL) );		/* Motor carrier frequency switching<S117> */
	kx = PcalPwmCounter( uCfgPrm->NormCarrCnt );		/* Reverse calculation of frequency from setting carrier counter value	*/
	KpiPcalCurrentObs( SvAsicRegs, uCfgPrm->ignq, kx, Bprm, &(pAsicMicroReg->DobsP.TsPerL) );

	return rc;
}

#if 0		/* Deleted because unit carrier fixing processing is unnecessary<S11E> */
#if ( defined(CSW_SGD7W_TYPE) || defined(CSW_COMMON_CARRIER) )		/* Motor carrier frequency switching<S117> */
/*----------------------------------------------------------------------------------*/
/*  @@@@@@ SGD7WDedicated carrier frequency adjustment processing Very provisional until confirmation of separate carrier settings @@@@@@@ */
/*----------------------------------------------------------------------------------*/
LONG	KpxRecalcPWMparameter( MAICRO_CFGPRM *uCfgPrm, BPRMDAT *Bprm, ASICS *SvAsicRegs )
{
	ASIC_U_IF_STR	*pAsicMicroReg;
	LONG			kx;
	LONG			rc;

	pAsicMicroReg = SvAsicRegs->AsicMicroReg;
	rc  = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		Control flag(CSW)																		*/
/*--------------------------------------------------------------------------------------------------*/
	SvAsicRegs->MicroCsw.data = (USHORT)( uCfgPrm->dq_sw | 0x00F0 );			/* Control flag		*/
	kx = PcalPwmCounter( uCfgPrm->NormCarrCnt );		/* Reverse calculation of frequency from setting carrier counter value	*/
	if( kx > 4000 )
	{
		/* Carrier frequency4kHzor above */
		SvAsicRegs->MicroCsw.data &= 0xFFF7;
	}

	if(( uCfgPrm->flg_wf & 0x0001 ) == 0 )
	{ /* Field weakening enable/disable check on motor side */
		/* Forcibly disable if motor side is disabled */
		SvAsicRegs->MicroCsw.data &= 0xF7FF;
	}

	if( ( SvAsicRegs->MicroCsw.data & 0x0800 ) == 0 )
	{
		/* Field weakening=OFF, field weakening2=OFF, */
		SvAsicRegs->MicroCsw.data &= 0xDFFF;
	}

	if( ( SvAsicRegs->MicroCsw.data & 0x2000 ) != 0 )
	{
		/* Field weakening2=ON: Integral stop function=ON, */
		SvAsicRegs->MicroCsw.data |= 0x4000;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		PWMCarrier frequency count value																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* PWMCarrier frequency */
	rc |= LpxSetPrmToASIC( (SHORT*)&(pAsicMicroReg->IntAdP.CrFreq), uCfgPrm->NormCarrCnt, NO_LMT_CHK );/*<S0C7>*/

/*--------------------------------------------------------------------------------------------------*/
/*		Current observer parameter calculation																*/
/*--------------------------------------------------------------------------------------------------*/
	/* Parameter settings */
	kx = PcalPwmCounter( uCfgPrm->NormCarrCnt );		/* Reverse calculation of frequency from setting carrier counter value	*/
	KpiPcalCurrentObs( SvAsicRegs, uCfgPrm->ignq, kx, Bprm, &(pAsicMicroReg->DobsP.TsPerL) );

	return rc;
}
#endif	/* #if ( defined(CSW_SGD7W_TYPE) || defined(CSW_COMMON_CARRIER) ) *//* Motor carrier frequency switching<S117> */
#endif	/* #if 0		Deleted because unit carrier fixing processing is unnecessary<S11E> */


/*<S1F5> START*/
/*****************************************************************************
  Description: OCLevel
------------------------------------------------------------------------------
  Parameters:
	uCfgPrm             Alarm management IF data
	SvAsicRegs			ASICPointer to address table
	MencP				Encoder parameter definition
  Return:

------------------------------------------------------------------------------
  Note:

*****************************************************************************/
#if 0	/* <S1F7> */
LONG	KpxMotorOCLevel( MAICRO_CFGPRM *uCfgPrm, ASICS *SvAsicRegs, MENCPRM *MencP )
{
	LONG	kx;
	LONG	rc;
	LONG	OcLevel;

	rc  = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		Mask status																						*/
/*--------------------------------------------------------------------------------------------------*/
	kx = MencP->motfuncf.f.OCset;
	if( kx == 1 )
	{
		rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_SETMSK), kx, REQ_LMT_CHK );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		OCDetection level																					*/
/*--------------------------------------------------------------------------------------------------*/
	OcLevel = MlibDiv6432( (MencP->irat * MencP->mot_oc), (uCfgPrm->shant * 4), (uCfgPrm->imaxs * 100) );
	kx = OcLevel;
	rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_OCLVUP), kx, REQ_LMT_CHK );
	rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_OCLVUM), kx, REQ_LMT_CHK );
	rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_OCLVVP), kx, REQ_LMT_CHK );
	rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_OCLVVM), kx, REQ_LMT_CHK );
	rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_OCLVWP), kx, REQ_LMT_CHK );
	rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_OCLVWM), kx, REQ_LMT_CHK );
/*--------------------------------------------------------------------------------------------------*/
/*		Decimation filter operating mode																		*/
/*--------------------------------------------------------------------------------------------------*/
	kx = 0;
	rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_OCDCONF1), kx, REQ_LMT_CHK );
/*--------------------------------------------------------------------------------------------------*/
/*		Motor overcurrent detection filter count																		*/
/*--------------------------------------------------------------------------------------------------*/
	kx = uCfgPrm->fltrange;
	rc |= LpxSetPrmToASIC( (SHORT*)&(SvAsicRegs->AsicHwReg->AREG_FLTRANGE), kx, REQ_LMT_CHK );
	return( rc );
}
/*<S1F5> END*/
#else	/* <S1F7> */
LONG	KpxMotorOCLevel( MAICRO_CFGPRM *uCfgPrm, ASICS *SvAsicRegs, MENCPRM *MencP )
{
	LONG			rc;
	LONG			OcMaxU, OcMaxV, OcMaxW;			/* OCDetection threshold(Positive side)								*/
	LONG			OcMinU, OcMinV, OcMinW;			/* OCDetection threshold(Negative side)								*/
	LONG			OcWorkU, OcWorkV, OcWorkW;		/* OCDetection threshold calculation work						*/
	LONG			OcLevel;						/* OCDetection level(Gain adjustment, offset adjustment before)		*/
	ASIC_HW_IF_STR	*pAsicHwReg = SvAsicRegs->AsicHwReg;

	rc  = 0;
/*--------------------------------------------------------------------------------------------------*/
/*		Mask status																					*/
/*--------------------------------------------------------------------------------------------------*/
	if( (LONG)MencP->motfuncf.f.OCset == 1 )
	{
		pAsicHwReg->AREG_SETMSK		|= 0x000F;			/* OCDetection disabled								*/
/*--------------------------------------------------------------------------------------------------*/
/*		OCDetection enabled																					*/
/*--------------------------------------------------------------------------------------------------*/
		pAsicHwReg->AREG_SETREG1	= 0x00000005;		/* ASIC Write Enable						*/
		pAsicHwReg->AREG_CSTP		|= ( 1 << 26 );		/* OCDetection disabled								*/
		pAsicHwReg->AREG_SETREG1	= 0x00000001;		/* ASIC Write Disable						*/
/*--------------------------------------------------------------------------------------------------*/
		return( rc );									/* If disabled, do not make any other settings.		*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Decimation filter operating mode															*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_OCDCONF1	&= 0xFFF0;
	pAsicHwReg->AREG_OCSDMCONF3	= 0x00001B38;	/*
								bit0 - 7:OCFor detectionADIFNumber of cycles                0x38(CurrentAD)
								bit8 -11:ADBasic clock bit                  0x0B(CurrentAD)
								bit12   :MSBSelection                               0x01(CurrentAD)	*/
	pAsicHwReg->AREG_OCSDMCONF4	= 0x00003839;	/*
								bit0 - 7:OCFor detectionADControl0Rise setting 0x39(CurrentAD)
								bit8 -11:OCFor detectionADControl0Descent setting       0x38(CurrentAD)	*/
/*--------------------------------------------------------------------------------------------------*/
/*		OCDetection level																				*/
/*		0x0000(-320mV) ~ 0xFFFF(+320mV)	shant max 8192											*/
/*			Positive side = 0x8000 +( PnF2C × PnE28 × 4 )÷ PnE15										*/
/*			Negative side = 0x8000 -( PnF2C × PnE28 × 4 )÷ PnE15										*/
/*		PnF2C = MencP->mot_oc  : MotorOCDetection current[0.1A]												*/
/*		PnE28 = uCfgPrm->shant : Current detection gain														*/
/*		PnE15 = uCfgPrm->imaxs : Servopack maximum current[0.1A]											*/
/*																									*/
/*		Gain, offset correction calculation																	*/
/*			After correction = Before correction ÷ ( Current detection gain adjustment + 1.00 ) - Current detection zero adjustment × 4					*/
/*		PnE50   = uCfgPrm->zadjiu : Current detection zero adjustment(UPhase)												*/
/*		PnE51   = uCfgPrm->zadjiv : Current detection zero adjustment(VPhase)												*/
/*		PnE52.L = uCfgPrm->gadjiu_v.b.l : Current detection gain adjustment(UPhase)										*/
/*		PnE52.H = uCfgPrm->gadjiu_v.b.h : Current detection gain adjustment(VPhase)										*/
/*--------------------------------------------------------------------------------------------------*/
	/* OCDetection level */
	OcLevel = MlibDiv6432(( MencP->mot_oc ), ( uCfgPrm->shant * 4 ), ( uCfgPrm->imaxs ));
	/* UPhase correction */
	OcWorkU  = OcLevel * 0x200 / ((CHAR)uCfgPrm->gadjiu_v.b.l + 0x100 );
//	OcWorkU  = (( OcWorkU + 1 ) >> 1 ) - (SHORT)uCfgPrm->zadjiu * 4;								  /* <S1FC> */
	OcWorkU  = (( OcWorkU + 1 ) >> 1 );
	OcMaxU = 0x00008000 + OcWorkU - ((SHORT)uCfgPrm->zadjiu * 4);									  /* <S1FC> */
	if( OcMaxU > 0x0000FFFF ) OcMaxU = 0x0000FFFF;
	OcMinU = 0x00008000 - OcWorkU - ((SHORT)uCfgPrm->zadjiu * 4);									  /* <S1FC> */
	if( OcMinU < 0x00000000 ) OcMinU = 0x00000000;
	pAsicHwReg->AREG_OCLVUP = OcMaxU;
	pAsicHwReg->AREG_OCLVUM = OcMinU;
	/* VPhase correction */
	OcWorkV  = OcLevel * 0x200 / ((CHAR)uCfgPrm->gadjiu_v.b.h + 0x100 );
//	OcWorkV  = (( OcWorkV + 1 ) >> 1 ) - (SHORT)uCfgPrm->zadjiv * 4;								  /* <S1FC> */
	OcWorkV  = (( OcWorkV + 1 ) >> 1 );
	OcMaxV = 0x00008000 + OcWorkV - ((SHORT)uCfgPrm->zadjiv * 4);									  /* <S1FC> */
	if( OcMaxV > 0x0000FFFF ) OcMaxV = 0x0000FFFF;
	OcMinV = 0x00008000 - OcWorkV - ((SHORT)uCfgPrm->zadjiv * 4);									  /* <S1FC> */
	if( OcMinV < 0x00000000 ) OcMinV = 0x00000000;
	pAsicHwReg->AREG_OCLVVP = OcMaxV;
	pAsicHwReg->AREG_OCLVVM = OcMinV;
	/* WPhase correction */ /* <S1FC> */
	OcWorkW = ( OcWorkU > OcWorkV )? OcWorkU : OcWorkV;
	OcMaxW = 0x00008000 + OcWorkW + ((SHORT)uCfgPrm->zadjiu * 4) + ((SHORT)uCfgPrm->zadjiv * 4);
	if( OcMaxW > 0x0000FFFF ) OcMaxW = 0x0000FFFF;
	OcMinW = 0x00008000 - OcWorkW + ((SHORT)uCfgPrm->zadjiu * 4) + ((SHORT)uCfgPrm->zadjiv * 4);
	if( OcMinW < 0x00000000 ) OcMinW = 0x00000000;
	pAsicHwReg->AREG_OCLVWP = OcMaxW;
	pAsicHwReg->AREG_OCLVWM = OcMinW;
//	pAsicHwReg->AREG_OCLVWP = ( OcMaxU > OcMaxV )? OcMaxU : OcMaxV ;								  /* <S1FC> */
//	pAsicHwReg->AREG_OCLVWM = ( OcMinU < OcMinV )? OcMinU : OcMinV ;								  /* <S1FC> */
/*--------------------------------------------------------------------------------------------------*/
/*		Motor overcurrent detection filter count																*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_FLTRANGE = uCfgPrm->fltrange;
/*--------------------------------------------------------------------------------------------------*/
/*		OCDetection enabled																					*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_SETMSK		&= 0xFFF0;			/* OCDetection enabled									*/
	pAsicHwReg->AREG_SETREG1	= 0x00000005;		/* ASIC Write Enable							*/
	pAsicHwReg->AREG_CSTP		&= ~( 1 << 26 );	/* OCDetection enabled									*/
	pAsicHwReg->AREG_SETREG1	= 0x00000001;		/* ASIC Write Disable							*/
/*--------------------------------------------------------------------------------------------------*/
/*		OCFor detectionADCycle synchronization																		*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicHwReg->AREG_OCVADSYNC = 0x0000;
/*--------------------------------------------------------------------------------------------------*/
	return( rc );
}
#endif	/* <S1F7> */
/****************************************************************************************************/
/*																									*/
/*		Current conversion gain calculation																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			  10000 * 2^9																			*/
/*		Kc = -------------																			*/
/*			    Iadmax																				*/
/*																									*/
/*		Iadmax [-] : ADC value at maximum current															*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalDqAcrKc( LONG Iadmax, USHORT imax, USHORT imaxs )
{
#if (FLOAT_USE==TRUE)
	float fw;
#else
		LONG	kx,sx;
#endif /* FLOAT_USE */

	if( imax >= imaxs )
	{
#if (FLOAT_USE==TRUE)
		fw = (10000.0F * 512.0F) / (float)Iadmax;
#else
		kx = (10000 * 512) / Iadmax;
#endif /* FLOAT_USE */

	}
	else
	{
#if (FLOAT_USE==TRUE)
		fw = (512.0F * 10000.0F) / (float)Iadmax;
		fw = fw * (float)imaxs / (float)imax;
#else
		kx = MlibScalKxgain( 512, 10000,     Iadmax,   &sx,   0 );
		kx = MlibPcalKxgain( kx,  imaxs, imax, &sx, -24 );
#endif /* FLOAT_USE */
	}

#if (FLOAT_USE==TRUE)
	return( fw );
#else
	return( kx );
#endif /* FLOAT_USE */

}



/****************************************************************************************************/
/*																									*/
/*		Current loop proportional gain calculation																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			  2*PAI * Gain * Derate * MotLd       2^13     Imax * 512 								*/
/*		Kp = ----------------------------- * ------- * -------------								*/
/*				              100    	      Vdc/2	      15000  									*/
/*																									*/
/*		Gain  [Hz] : Current loop gain																*/
/*		Derate [%] : Current loop gain derating												*/
/*		Lx     [H] : Motor inductance															*/
/*																									*/
/****************************************************************************************************/
LONG	PcalDqAcrKp( LONG Gain, LONG Derate, BPRMDAT *Bprm, LONG axis_type )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG	kx, sx;
#endif /* FLOAT_USE */
	LONG	rc;

#if (FLOAT_USE==TRUE)
	fw = (float)Gain * (float)Derate;
#else
	kx = MlibScalKxgain( Gain, Derate, 1, &sx, 0 );
#endif /* FLOAT_USE */

	if(axis_type == D_AXIS)
	{ /* Daxis */
#if (FLOAT_USE==TRUE)
		fw = fw * Bprm->MotLd / Bprm->Vdc;
#else
		kx = MlibPcalKxksks( kx,   Bprm->MotLd,          Bprm->Vdc, &sx,   0 );
#endif /* FLOAT_USE */

	}
	else //if(axis_type == Q_AXIS)
	{ /* Qaxis */
#if (FLOAT_USE==TRUE)
				fw = fw * Bprm->MotLq / Bprm->Vdc;
#else
		kx = MlibPcalKxksks( kx,   Bprm->MotLq,          Bprm->Vdc, &sx,   0 );
#endif /* FLOAT_USE */
	}
#if (FLOAT_USE==TRUE)
	fw = fw * Bprm->MaxCur;
	fw = fw * 26353589.0F / 750000.0F;
	if( fw > (float)0x01000000 )
	{
		rc = 0x01000000;
	}
	else
	{
		rc = (LONG)fw;
	}
#else
	kx = MlibPcalKxkskx( kx,   Bprm->MaxCur, 1,        &sx,   0 );
	rc = MlibPcalKxgain( kx,   26353589,    750000,   &sx, -24 );
#endif /* FLOAT_USE */

	return( rc );
}



/****************************************************************************************************/
/*																									*/
/*		Current loop integral gain calculation																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			  Kp * Ts * 2^4																			*/
/*		Ki = ---------------																		*/
/*				1000 * Ti																			*/
/*																									*/
/*		Ts [ns] : Current loop operation period																*/
/*		Ti [us] : Current loop integral time constant																*/
/*																									*/
/****************************************************************************************************/
LONG	PcalDqAcrKi( LONG Ti, LONG Kp, KSGAIN MotL, BPRMDAT *Bprm )
{
	LONG	kx,sx;
	LONG	Tx;
	LONG	MotW;
#if (FLOAT_USE==TRUE)
	float fw;
#endif
	
/*--------------------------------------------------------------------------------------------------*/
/*		Current loop integral time constant[us](Motor electrical time constant)										*/
/*																									*/
/*				         Ld[H]		   																*/
/*		Tx[us] =  --------------- × 1000000														*/
/*				     Rx[0.001ohm]																	*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = MotL * 1000000.0;
	fw = fw / Bprm->MotR;
#else
	kx = MlibScalKskxkx( MotL, 1000000, 1, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 1,	Bprm->MotR, &sx,   -24 );
#endif /* FLOAT_USE */

	/* Limit processing	*/
#if (FLOAT_USE==TRUE)
	Tx = MlibLimitul( (long)fw, 0xFFFF, 0 );
#else
	Tx = MlibLimitul( kx, 0xFFFF, 0 );					/* 0as minimum value						*/
#endif /* FLOAT_USE */

	if( Bprm->CurrentLoopTiSw == TRUE )
	{	/* Set motor electrical integration time constant regardless of motor model */
		Ti = Tx;
	}
	else
	{	/* Sets the motor electrical integration time constant for specified models only. */
		if( Bprm->MotTypeCode == 0x67 )					/* SGMMVMotor code							*/
		{
#if (FLOAT_USE==TRUE)
			MotW = (LONG)Bprm->MotW;
#else
			MotW = MlibScalKskxkx( Bprm->MotW, 1, 1, NULL, -30 );
#endif /* FLOAT_USE */
			switch( MotW )
			{
				case 0x0003:	/* SGMMV-B3E */
				case 0x0005:	/* SGMMV-B5E */
				case 0x000B:	/* SGMMV-B9E */
					Ti = Tx;
				break;
			
				default:
					break;
			}
		}
	}

/*--------------------------------------------------------------------------------------------------*/

	if( Ti == 0 )
	{
#if (FLOAT_USE==TRUE)
		fw = 0;
#else
		kx = 0;
#endif /* FLOAT_USE */
	}
	else
	{
#if (FLOAT_USE==TRUE)
		fw = (float)Kp * 16.0 * (float)KPI_MACYCLENS / (1000.0 * (float)Ti);
#else
		kx = MlibScalKxgain( Kp, 16*KPI_MACYCLENS, 1000*Ti, NULL, -24 );
#endif /* FLOAT_USE */
	}
#if (FLOAT_USE==TRUE)
	return( fw );
#else
	return( kx );
#endif /* FLOAT_USE */
}



/****************************************************************************************************/
/*																									*/
/*		L(di/dt)Inductance parameter calculation														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*						   Imax[A]		8192			10^9										*/
/*		L_DIDT = Le[H] × --------- × -------- × -------------- × 2^9							*/
/*							15000		Vdc/2		 scantime[ns]									*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalLDidt( KSGAIN Le, KSGAIN max_cur, KSGAIN vdc )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG	kx, sx;
#endif /* FLOAT_USE */
	LONG	rc;

#if (FLOAT_USE==TRUE)
	fw = Le * max_cur / 15000.0F;
	fw = fw * 16384.0F * 512.0F / vdc;
	fw = fw * 1000000000.0F / (float)KPI_MBCYCLENS;
	if( fw > (float)0x01000000 )
	{
		rc = 0x01000000;
	}
	else
	{
		rc = (LONG)fw;
	}
#else
	kx = MlibScalKskskx( Le,   max_cur,      15000,          &sx,   0 );
	kx = MlibPcalKxkxks( kx,   16384*512,    vdc,            &sx,   0 );
	rc = MlibPcalKxgain( kx,   1000000000,   KPI_MBCYCLENS,  &sx, -24 );
#endif /* FLOAT_USE */

	return( rc );
}



/****************************************************************************************************/
/*																									*/
/*		Current loop filter gain calculation																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			    Ts * 2^14																			*/
/*		Kf = -----------------																		*/
/*			  (1000*Tf) + Ts																		*/
/*																									*/
/*		Ts [ns] : Current loop operation period																*/
/*		Tf [us] : Current loop filter time constant															*/
/*																									*/
/****************************************************************************************************/
LONG	PcalDqAcrFilter( LONG Tf )
{
#if (FLOAT_USE==TRUE) 
	float fw;

	fw = (float)KPI_MACYCLENS * 16384.0 / ((1000.0*(float)Tf) + (float)KPI_MACYCLENS);

	return( fw );
#else
	LONG	kx;

	kx = MlibScalKxgain( KPI_MACYCLENS, 16384, ((1000*Tf) + KPI_MACYCLENS), NULL, -24 );

	return( kx );
#endif /* FLOAT_USE */
}



/****************************************************************************************************/
/*																									*/
/*		Inductance compensation constant(Ld,Lq)															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			  2^15 * 2*PAI*Nos     2^15 * Imax      Lx * 2^13										*/
/*		kx = ------------------ * ------------- * --------------		--- SGDS					*/
/*			     60 * 15000        15000 * 10      10^5 * Vdc/2										*/
/*																									*/
/*																									*/
/*			  2^15 * Nmax     2^15 * Imax     Lx * 2^13     PoleNo									*/
/*		kx = ------------- * ------------- * ----------- * --------		--- SVOS					*/
/*				 15000           15000          Vdc/2         2										*/
/*																									*/
/*																									*/
/*		Nmax [rad/s] : Max. Speed																	*/
/*		Imax [A]     : Max. Current																	*/
/*		Lx   [H]     : Inductance																	*/
/*		Vdc  [V]     : DC Voltage																	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalDqLxComp( KSGAIN Lx, KSGAIN over_spd, KSGAIN max_cur, KSGAIN vdc )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG	kx, sx;
#endif /* FLOAT_USE */
	LONG	rc;

/*--------------------------------------------------------------------------------------------------*/
/*		Inductance compensation constant(Ld,Lq)															*/
/*--------------------------------------------------------------------------------------------------*/
/*		Rotary type:Bprm.Keangle = PoleNo / 2		: Motor[rad] ---> Eangle[rad]						*/
/*		Linear:Bprm.Keangle = PAI / PolePitch	: Motor[ m ] ---> Eangle[rad]						*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = over_spd * max_cur / vdc;
	fw = fw * Lx;
	fw = fw * 78187.0F;
	if( fw > (float)0x01000000 )
	{
		rc = 0x01000000;
	}
	else
	{
		rc = (LONG)fw;
	}
#else
	kx = MlibScalKsksks( over_spd,	  max_cur,  	vdc,   &sx,   0 );
	kx = MlibPcalKxkskx( kx,          Lx,           1,     &sx,   0 );
	rc = MlibPcalKxgain( kx,          78187,        1,     &sx, -24 );
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
	return( rc );
}



/****************************************************************************************************/
/*																									*/
/*		Armature resistance(R1)Compensation constant calculation																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			   2^15 * Rx      2^13     Imax															*/
/*		kx = ------------ * ------- * -------														*/
/*				   1         Vdc/2     15000														*/
/*																									*/
/*																									*/
/*		Rx   [ohm] : Resistance																		*/
/*		Imax [A]   : Max. Current																	*/
/*		Vdc  [V]   : DC Voltage																		*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalDqRxComp( KSGAIN Rx, KSGAIN max_cur, KSGAIN vdc )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG	kx, sx;
#endif /* FLOAT_USE */
	LONG	rc;
#if (FLOAT_USE==TRUE)
	fw = Rx * 35791.0F;
	fw = fw * max_cur / vdc;
	if( fw > (float)0x01000000 )
	{
		rc = 0x01000000;
	}
	else
	{
		rc = (LONG)fw;
	}
#else
	kx = MlibScalKskxkx( Rx, 35791,       1,        &sx,   0 );
	rc = MlibPcalKxksks( kx, max_cur,	  vdc,		&sx, -24 );
#endif /* FLOAT_USE */

	return( rc );
}



/****************************************************************************************************/
/*																									*/
/*		EMF compensation constant(Φ)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			   Nmax * 1.4142 * Emf * 2^15      2^13													*/
/*		kx = ------------------------------ * -------												*/
/*			               15000               Vdc/2												*/
/*																									*/
/*		Nmax [rad/s]      : Max. Speed																*/
/*		Emf  [Vrms/rad/s] : EMF Constant															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalDqEmfComp( KSGAIN Emf, KSGAIN over_spd, KSGAIN vdc )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG	kx, sx;
#endif /* FLOAT_USE */
	LONG	rc;

#if (FLOAT_USE==TRUE)
	fw = Emf * over_spd / vdc;
	fw = fw * 50617.0F;
	if( fw > (float)0x01000000 )
	{
		rc = 0x01000000;
	}
	else
	{
		rc = (LONG)fw;
	}
#else
	kx = MlibScalKsksks( Emf, over_spd,    vdc,		&sx,   0 );
	rc = MlibPcalKxgain( kx,  50617,       1,       &sx, -24 );
#endif /* FLOAT_USE */

	return( rc );
}




/****************************************************************************************************/
/*																									*/
/*		Current gain parameter calculation for ASIC micro													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/****************************************************************************************************/
void	PcalMicroDqAcrGain(	ASIC_U_IF_STR	*pAsicMicroReg,	/* Control variable pointer						*/
							DQACR_GAIN_PRM	*Prm,			/* Parameter data pointer				*/
							BPRMDAT			*Bprm,			/* Base parameter pointer 			*/
							ULONG			Loophz,			/* Speed loop gain			[0.1Hz]		*/
							ULONG			ratio)			/* Gain balance			[%]			*/
{

	LONG	kp, ki;
	LONG	kx, sx;
	LONG	Kxd, Kxq, Txd, Txq;
	LONG	kfil, Tfx;
	LONG	MinKpGain;
#if (FLOAT_USE==TRUE)
	float Fwd, Fwq, fw;
#endif /* FLOAT_USE */

	MinKpGain = 900;							/* Current loop gain minimum value	[Hz]					*/

/*--------------------------------------------------------------------------------------------------*/
/*	Current gain interlock execution judgment																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( ratio >= 2000 )
	{	/* Ratio is2000%or more, current gain interlock is disabled	*/
		/* or the torque control setting is invalid					*/
		Kxd = Prm->ignd;
		Kxq = Prm->ignq;
		Txd = Prm->kid;
		Txq = Prm->kiq;
		Tfx = Prm->ifil;
	}
	else
	{
/*--------------------------------------------------------------------------------------------------*/
/*		Current gain[Hz]																			*/
/*																									*/
/*				    Kv[0.1Hz]           ratio[%]													*/
/*		Kxd[Hz] =  ----------- × 4 × ---------- 													*/
/*				       10                 100														*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
		Fwd = (float)Loophz * 4 * (float)ratio /1000;
		Fwq = Fwd;

		/*	Limit processing (Use parameter as maximum value)	*/
		if( Fwd < (float)MinKpGain )
		{
			Fwd = (float)MinKpGain;
		}
		if( Fwd > (float)Prm->ignd )
		{ /* If maximum value < minimum value, clamp to maximum value */
			Fwd = (float)Prm->ignd;
		}
		if( Fwq < (float)MinKpGain )
		{
			Fwq = (float)MinKpGain;
		}
		if( Fwq > (float)Prm->ignq )
		{
			Fwq =(float) Prm->ignq;
		}

#else
		Kxd = MlibScalKxgain( (LONG)Loophz,   (4 * (LONG)ratio),   1000,  NULL, -24 );
		Kxq = Kxd;


		/*	Limit processing (Use parameter as maximum value)	*/
		if( Kxd < MinKpGain )
		{
			Kxd = MinKpGain;
		}
		if( Kxd > (LONG)Prm->ignd )
		{ /* If maximum value < minimum value, clamp to maximum value */
			Kxd = Prm->ignd;
		}
		if( Kxq < MinKpGain )
		{
			Kxq = MinKpGain;
		}
		if( Kxq > (LONG)Prm->ignq )
		{
			Kxq = Prm->ignq;
		}
#endif /* FLOAT_USE */
		/* Current loop integral time constant not linked	*//*<V242>*/
		Txd = Prm->kid;
		Txq = Prm->kiq;

/*--------------------------------------------------------------------------------------------------*/
/*		Voltage filter time constant[us]																*/
/*																									*/
/*			At current gain minimum valuePnEA4, and becomes the maximum current gain value(PnE21)filter time constant with=0The slope		*/
/*																									*/
/*																									*/
/*				            Prm.ifil2[us]		   													*/
/*		Tfx[us] =  --------------------------------- × (Kxq[Hz] - MinKpGain[Hz]) + Prm.ifil2[us]	*/
/*				     (MinKpGain[Hz] - Prm.ignq)[Hz])												*/
/*--------------------------------------------------------------------------------------------------*/
		if( MinKpGain != Prm->ignq )
		{
#if (FLOAT_USE==TRUE)
			fw = (float)Prm->ifil2 * ((float)Kxq - (float)MinKpGain) / ((float)MinKpGain - (float)Prm->ignq);
			(float)Tfx = fw + (float)Prm->ifil2;
#else
			kx = MlibScalKxgain( Prm->ifil2, (Kxq - MinKpGain), (MinKpGain - Prm->ignq), &sx, -24 );
			Tfx = kx + Prm->ifil2;
#endif /* FLOAT_USE */
		}
		else
		{
			Tfx = Prm->ifil2;
		}
		if( Tfx < 0 )
		{
			Tfx = 0;
		}
		if( Tfx > Prm->ifil2 )
		{
			Tfx = Prm->ifil2;
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
#ifndef _JL076_USE_
	/*----------------------------------------------------------------------------------------------*/
	/*		dAxis current gain																			*/
	/*----------------------------------------------------------------------------------------------*/
	/* Proportional gain */
	kp = PcalDqAcrKp( Kxd, 100, Bprm, D_AXIS );
	LpxSetPrmToASIC( &pAsicMicroReg->IntAdP.KdP, kp, 1 );

	/*----------------------------------------------------------------------------------------------*/
	/* Integral gain */
	ki = PcalDqAcrKi( Txd, kp, Bprm->MotLd, Bprm );
	LpxSetPrmToASIC( &pAsicMicroReg->IntAdP.KdI, ki, 1 );

	/*----------------------------------------------------------------------------------------------*/
	/*		qAxis current gain																			*/
	/*----------------------------------------------------------------------------------------------*/
	/* Proportional gain */
	kp = PcalDqAcrKp( Kxq, 100, Bprm, Q_AXIS );
	LpxSetPrmToASIC( &pAsicMicroReg->IntAdP.KqP, kp, 1 );

	/*----------------------------------------------------------------------------------------------*/
	/* Integral gain */
	ki = PcalDqAcrKi( Txq, kp, Bprm->MotLq, Bprm );
	LpxSetPrmToASIC( &pAsicMicroReg->IntAdP.KqI, ki, 1 );

	/*----------------------------------------------------------------------------------------------*/
	/*		Current loop output filter time constant															*/
	/*----------------------------------------------------------------------------------------------*/
	/* Output filter gain */
	kfil = PcalDqAcrFilter( Tfx );
	LpxSetPrmToASIC( &pAsicMicroReg->IntAdP.Tfil, kfil, 1 );

#else /* _JL076_USE_ */

	/*----------------------------------------------------------------------------------------------*/
	/*		dAxis current gain																			*/
	/*----------------------------------------------------------------------------------------------*/
	/* Proportional gain */
	kp = PcalDqAcrKp( Kxd, 100, Bprm, D_AXIS );
	LpxSetPrmToASIC( (SHORT*)&pAsicMicroReg->MREG_KDP, kp, 1 );

	/*----------------------------------------------------------------------------------------------*/
	/* Integral gain */
	ki = PcalDqAcrKi( Txd, kp, Bprm->MotLd, Bprm );
	LpxSetPrmToASIC( (SHORT*)&pAsicMicroReg->MREG_KDI, ki, 1 );

	/*----------------------------------------------------------------------------------------------*/
	/*		qAxis current gain																			*/
	/*----------------------------------------------------------------------------------------------*/
	/* Proportional gain */
	kp = PcalDqAcrKp( Kxq, 100, Bprm, Q_AXIS );
	LpxSetPrmToASIC( (SHORT*)&pAsicMicroReg->MREG_KQP, kp, 1 );

	/*----------------------------------------------------------------------------------------------*/
	/* Integral gain */
	ki = PcalDqAcrKi( Txq, kp, Bprm->MotLq, Bprm );
	LpxSetPrmToASIC( (SHORT*)&pAsicMicroReg->MREG_KQI, ki, 1 );

	/*----------------------------------------------------------------------------------------------*/
	/*		Current loop output filter time constant															*/
	/*----------------------------------------------------------------------------------------------*/
	/* Output filter gain */
	kfil = PcalDqAcrFilter( Tfx );
	LpxSetPrmToASIC( (SHORT*)&pAsicMicroReg->MREG_TFIL, kfil, 1 );

#endif /* _JL076_USE_ */
 
}



/****************************************************************************************************/
/*																									*/
/*		On-delay level calculation																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			   Level * 15000																		*/
/*		kx = -----------------		→		= limit( Level *150 ) :0 ~15000,				*/
/*			       Imax       	 																	*/
/*																									*/
/*		Level [%] : On-delay switching level														*/
/*		Imax  [%] : Maximum current[%] (=Maximum torque[%])													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalOndelayLevel( LONG Level )
{
	LONG	kx;

	kx = MlibLimitul( (Level * 150), 15000, 0 );

	return( kx );
}



/****************************************************************************************************/
/*																									*/
/*		On-delay compensation trapezoidal slope calculation																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			     2^8 * 2^13 	     2^8 * 2^13 * 1000	   139810   								*/
/*		kx = -------------------- = ------------------- = -------- 									*/
/*			  15000 * Slope/1000       15000 * Slope        Slope  									*/
/*																									*/
/*		Odslp [0.1%] : Compensation is100%If the balance between															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalOndelaySlope( LONG slope )
{
#if (FLOAT_USE==TRUE)
	float fw;

	if( slope != 0 )
	{
		fw = 139810.0F / (float)slope;
	}
	else
	{
		fw = 139810.0F;
	}

	return( fw );

#else
	LONG	kx;

	if( slope != 0 )
	{
		kx = 139810 / slope;
	}
	else
	{
		kx = 139810;
	}

	return( kx );

#endif /* FLOAT_USE */

}



/****************************************************************************************************/
/*																									*/
/*		PWM carrier frequency change service		<V356>												*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : PWMChange the frequency online															*/
/*			In argumentPnE2CIf less than the setting lower limit is specified, "PnF1E bit2: Carrier frequency selection"			*/
/*																					*/
/*			PnE2CIf it is above the setting lower limit, change the carrier frequency to the setting								*/
/*			※To restore the changed carrier frequency, use the argument.”0”							*/
/*																									*/
/****************************************************************************************************/
#define		PWM_FREQ_LOWER_LIMIT		2000					/* Carrier frequency lower limit value				*/
/****************************************************************************************************/
LONG	KpiChangePwmFrequency( ULONG PwmFx, USHORT pwmf, USHORT pwmf3, USHORT flg_wf )
{
	LONG	Tc;

	if( PwmFx < PWM_FREQ_LOWER_LIMIT )
	{ /* Parameter lower limit */
		if( (flg_wf & 0x0002 )							/* No.2Carrier frequency selection flag		*/
			&& (pwmf3 >= PWM_FREQ_LOWER_LIMIT)	)		/* No.2With carrier frequency setting		*/
		{
			Tc = PcalPwmFrequency( pwmf3 );				/* No.2Carrier frequency setting			*/
		}
		else
		{
			Tc = PcalPwmFrequency( pwmf );				/* No.1Carrier frequency setting			*/
		}
	}
	else
	{ /* With specified frequency */
		Tc = PcalPwmFrequency( PwmFx );					/* Specified carrier frequency setting			*/
	}

	return( Tc );

}



/****************************************************************************************************/
/*																									*/
/*		Calculation of PWM carrier frequency count value														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			   40000000																				*/
/*		Tc = -----------																			*/
/*				  Fc																				*/
/*																									*/
/*		Fc [Hz] : PWM frequency																		*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalPwmFrequency( LONG PwmFx )
{
	LONG	Tc;

	switch ( PwmFx )
	{
#if 0 /* Tentatively80MHzSetting(Upper is normal) */
		case 10667 : Tc = 15000; break;						/* PwmFc = 10.666666... [kHz]			*/
		case  8000 : Tc = 20000; break;						/* PwmFc =  8.0 [kHz]					*/
		case  4000 : Tc = 40000; break;						/* PwmFc =  4.0 [kHz]					*/
#else
		case 10667 : Tc = 3750; break;						/* PwmFc = 10.666666... [kHz]			*//*<V324>*/
		case  8000 : Tc = 5000; break;						/* PwmFc =  8.0 [kHz]					*/
		case  5333 : Tc = 7500; break;						/* PwmFc =  5.333333... [kHz]			*//*<V324>*/
		case  4000 : Tc = 10000; break;					/* PwmFc =  4.0 [kHz]					*/
		case  3556 : Tc = 11250; break;					/* PwmFc =  3.555555... [kHz]			*//*<V324>*/
#endif
		case	 0 : Tc = (ASIC_CLKMHZ/2) * 1000000; break;	/* for escape 0div						*/
		default    : Tc = (ASIC_CLKMHZ/2) * 1000000 / PwmFx; break;	/* PwmFc = Other				*/
	}

	return( Tc );

}



/****************************************************************************************************/
/*																									*/
/*		Calculation of PWM carrier frequency						Motor carrier frequency switching<S117>						*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			   40000000																				*/
/*		Fc = -----------																			*/
/*				  Tc																				*/
/*																									*/
/*		Tc [-] : Carrier count value																	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalPwmCounter( LONG CarrCnt )
{
	LONG	Fc;

	switch ( CarrCnt )
	{
#if 0 /* Tentatively80MHzSetting(Upper is normal) */
		case 15000 : Tc = 10667; break;						/* PwmFc = 10.666666... [kHz]			*/
		case 20000 : Tc =  8000; break;						/* PwmFc =  8.0 [kHz]					*/
		case 40000 : Tc =  4000; break;						/* PwmFc =  4.0 [kHz]					*/
#else
		case  3750 : Fc = 10667; break;						/* PwmFc = 10.666666... [kHz]			*//*<V324>*/
		case  5000 : Fc = 8000; break;							/* PwmFc =  8.0 [kHz]					*/
		case  7500 : Fc = 5333; break;							/* PwmFc =  5.333333... [kHz]			*//*<V324>*/
		case 10000 : Fc = 4000; break;							/* PwmFc =  4.0 [kHz]					*/
		case 11250 : Fc = 3556; break;							/* PwmFc =  3.555555... [kHz]			*//*<V324>*/
#endif
		case	 0 : Fc = (ASIC_CLKMHZ/2) * 1000000; break;	/* for escape 0div						*/
		default    : Fc = (ASIC_CLKMHZ/2) * 1000000 / CarrCnt; break;	/* PwmFc = Other				*/
	}

	return( Fc );

}



/****************************************************************************************************/
/*																									*/
/*		Voltage feedback loop integral limit calculation													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*							 15000																	*/
/*		TvLim = Prm.TvLim * -------																	*/
/*							  100																	*/
/*																									*/
/*		Prm.TvLim [%] : Voltage feedback loop integral time constant parameter (PnEB2)						*/
/*																									*/
/****************************************************************************************************/
static	LONG	PcalVFBKiLim( LONG TvLim )
{
	LONG	kx;

	kx = TvLim * 150;

	return( kx );
}



/* 2012.10.26 Y.Oka ★Current observer currently not supported⇒Parameter batch write support required★ */
/****************************************************************************************************/
/*																									*/
/*		Current observer parameter calculation																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Observer gain1	: Ts/L																	*/
/*				   scantime[ns]		     1	          Bprm.Vdc[Vop]/2          15000     			*/
/*		wk_TSPL = -------------- * --------------- * ----------------- * ------------------ * 2^7	*/
/*					  10^9			Bprm.MotR[Ω]         2^13            Bprm.MaxCur[Aop]			*/
/*																									*/
/*		Observer gain2	: 1 - R*L/Ts - exp( -2*pi*Kpi )											*/
/*                       Bprm.MotR[Ω]     scantime[ns]												*/
/*		wk_GOBS = 2^8 - --------------- * -------------- * 2^8										*/
/*                       Bprm.MotLq[H]        10^9													*/
/*																									*/
/*                                                  scantime[ns]									*/
/*					- 2^8 * exp( -2*pi * Kpi[Hz] * -------------- )									*/
/*                                                      10^9										*/
/*																									*/
/*		Observer gain3	: 1 - R*L/Ts															*/
/*                       Bprm.MotR[Ω]     scantime[ns]												*/
/*		wk_GOBS = 2^4 - --------------- * -------------- * 2^4										*/
/*                       Bprm.MotLq[H]        10^9													*/
/*																									*/
/*		Filter gain		: Ts / ( Ts + Tfil )													*/
/*		                       scantime[ns]															*/
/*		wk_FILOBS = ----------------------------------- * 16384										*/
/*			                               10^9														*/
/*					 scantime[ns] + -----------------												*/
/*					                 2*pi * Pwmf[Hz]												*/
/*																									*/
/*		scantime( KPI_MACYCLENS )	: Current control cycle[ns]												*/
/*		Bprm.MotLq					: qAxis motor inductance[H]									*/
/*		Bprm.MotR					: qAxis motor resistance[Ω]											*/
/*		Bprm.MaxCur					: Maximum current[Aop]													*/
/*		Bprm.Vdc					: Maximum voltage[Vop]													*/
/*		Kpi							: qAxis current loop gain[Hz]	( PnE20 : Prm.ignq )				*/
/*		Pwmf						: Carrier frequency[Hz]		( PnE2C : Prm.pwmf )				*/
/****************************************************************************************************/
static	void	KpiPcalCurrentObs( ASICS *SvAsicRegs, USHORT ignq, USHORT pwmf, BPRMDAT *Bprm, SHORT *MpReg)
{
	LONG	n0,n1,n2;							/* bitShift amount										*/
	LONG	kx, sx;								/* workVariable for										*/
	LONG	kx2, sx2;							/* workVariable for2										*/
	LONG	kx3, sx3;							/* workVariable for3										*/
	LONG	kx4, sx4;							/* workVariable for4										*/
	LONG	MaxCur;								/* Maximum current[Aop]									*/
	LONG	Vdc;								/* Maximum voltage[Vop]									*/
	LONG	MotLq;								/* qAxis motor inductance[H]						*/
	LONG	MotR;								/* qAxis motor resistance[Ω]								*/
	USHORT	wk_TSPL;							/* TSPL workVariable for									*/
	USHORT	wk_GOBS;							/* GOBS workVariable for									*/
	USHORT	wk_RLTS;							/* RLTS workVariable for									*/
	USHORT	wk_FILOBS;							/* FILOBS workVariable for								*/

	n0 = 1 << (7+8);		/* 2^(7+2) */
	n1 = 1 << (8+8);		/* 2^(8+2) */
	n2 = 1 << (4+8);		/* 2^(4+2) */
	MaxCur	= Bprm->MaxCur;


	Vdc		= Bprm->Vdc;
	MotLq	= Bprm->MotLq;
	MotR	= Bprm->MotR;

	/* TSPL	: Ts/L */
	kx		= MlibScalKxgain( KPI_MACYCLENS,	15,			2,				&sx,	0	);
	kx		= MlibPcalKxgain( kx,				n0,			8192,			&sx,	0	);
	kx		= MlibPcalKxkskx( kx,				Vdc,		1000000,		&sx,	0	);
	kx		= MlibPcalKxkxks( kx,				1,			MotLq,			&sx,	0	);
	wk_TSPL = (USHORT)MlibPcalKxkxks( kx,		1,			MaxCur,			&sx,	-24	);

	/* GOBS	: 1 - R*Ts/L - g = 1 - R*Ts/L - exp( -2*pi*Ts*w ) */
	/* ( g = 1 - x + x^2/2! - x^3/3! + x^4/4! ) */
	/* x : 2*pi*Ts*w */
	/* R*Ts/L */
	kx		= MlibScalKxksks( n1,		MotR,				MotLq,			&sx,	0	);
	kx		= MlibPcalKxgain( kx,		KPI_MACYCLENS,		1000000000,		&sx,	0	);

	/* 1 */
	kx2		= MlibScalKxgain( n1,		1,					1,				&sx2,	0	);

	/* 1 - R*Ts/L */
	kx2		= MlibPcalKxsubx( kx2,		sx2,	kx,		sx,		&sx2 );

	/*  exp( -2*pi*Ts*w ) : 4Approximation with the next Taylor expansion */
	/* x : 2*pi*Ts*w  */
	kx		= MlibScalKxgain( 62831853,		ignq,	10000000,		&sx,	0	);
	kx		= MlibPcalKxgain( kx,			KPI_MACYCLENS,	1000000000, 	&sx,	0	);
	kx3		= kx;
	sx3		= sx;
	/* exp() No.1Term : 1 */
	kx4		= MlibScalKxgain( 1,		1,		1,		&sx4,	0 );
	/* exp() ( No.1Term - No.2Term ) : 1 - x */
	kx4		= MlibPcalKxsubx( kx4,		sx4,	kx, 	sx,	&sx4	);
	/* exp() No.3Term : x^2/2! */
	sx		+= sx ;
	kx		= MlibPcalKxgain( kx,	kx,	2,		&sx,	0	);
	/* exp() ( No.1Term - No.2Term + No.3Term ) : 1 - x + x^2/2! */
	kx4		= MlibPcalKxaddx( kx4,		sx4,	kx,		sx,  &sx4	);
	/* exp() No.4Term : x^3/3! */
	sx		+= sx3;
	kx		= MlibPcalKxgain( kx,	kx3,	3,		&sx,	0	);
	/* exp() ( No.1Term - No.2Term + No.3Term - No.4Term ) : 1 - x + x^2/2! - x^3/3! */
	kx4		= MlibPcalKxsubx( kx4,		sx4,	kx,		sx,  &sx4	);
	/* exp() No.5Term : x^4/4! */
	sx		+= sx3;
	kx		= MlibPcalKxgain( kx,	kx3,	4,		&sx,	0	);
	/* exp() ( No.1Term - No.2Term + No.3Term - No.4Term + No.5Term ) : 1 - x + x^2/2! - x^3/3! + x^4/4! */
	kx4		= MlibPcalKxaddx( kx4,		sx4,	kx,		sx,  &sx4	);
	/*  exp( -2*pi*Ts*w )*2^8  */
	kx4		= MlibPcalKxgain( kx4,	n1,	1,		&sx4,	0	);
	/* 1 - R*Ts/L - exp( -2*pi*Ts*w ) */
	kx2		= MlibPcalKxsubx( kx2,		sx2,	kx4,	sx4,	&sx2 );
	wk_GOBS	= (USHORT)MlibPcalKxgain( kx2,		1,		1,		&sx2,	-24	 );

	/* RLTS	: 1 - R*Ts/L */
	/* R*Ts/L */
	kx		= MlibScalKxksks( n2,		MotR,				MotLq,			&sx,	0	);
	kx		= MlibPcalKxgain( kx,		KPI_MACYCLENS,		1000000000,		&sx,	0	);
	/* 1 */
	kx2		= MlibScalKxgain( n2,		1,					1,				&sx2,	0	);
	/* 1 - R*Ts/L */
	kx2		= MlibPcalKxsubx( kx2,		sx2,	kx,		sx,		&sx2 );
	wk_RLTS	= (USHORT)MlibPcalKxgain( kx2,		1,		1,		&sx2,	-24	 );

	/* FILOBS	: Ts/(Tf+Ts) */
	/* Tf : 10^6 / ( PWMF*2*pi ) */
	kx		= MlibScalKxgain( 1000000,	1,			pwmf,		&sx,	0	);
	kx		= MlibPcalKxgain( kx,		10000000,	62831853,	&sx,	0	);
	kx		= MlibPcalKxgain( kx,		1000,		1,			&sx,	-24	);
	/* Ts/(Ts+Tf) */
	wk_FILOBS	= (USHORT)MlibPcalKxgain( KPI_MACYCLENS,	16384,	KPI_MACYCLENS+kx,	&sx,	-24	);


#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
	if( ( wk_GOBS == 0 ) || ( wk_TSPL == 0 ) || ( wk_RLTS == 0 ) )
	{
		tbuf[0] = 0;											/* TSPL 			*/
		tbuf[1] = 0;											/* GOBS 			*/
		tbuf[2] = 0;											/* RLTS 			*/
		tbuf[3] = wk_FILOBS;									/* FILOBS 			*/
		MicroTranslatePrmReq( SvAsicRegs, tbuf, MpReg, 4 );
	}
	else
	{
		tbuf[0] = wk_TSPL;										/* TSPL 			*/
		tbuf[1] = wk_GOBS;										/* GOBS 			*/
		tbuf[2] = wk_RLTS;										/* RLTS 			*/
		tbuf[3] = wk_FILOBS;									/* FILOBS 			*/
		MicroTranslatePrmReq( SvAsicRegs, tbuf, MpReg, 4 );
	}
#else
	if( ( wk_GOBS == 0 ) || ( wk_TSPL == 0 ) || ( wk_RLTS == 0 ) )
	{
		MpReg[0] = 0;											/* TSPL 			*/
		MpReg[1] = 0;											/* GOBS 			*/
		MpReg[2] = 0;											/* RLTS 			*/
		MpReg[3] = wk_FILOBS;									/* FILOBS 			*/
	}
	else
	{
		MpReg[0] = wk_TSPL;										/* TSPL 			*/
		MpReg[1] = wk_GOBS;										/* GOBS 			*/
		MpReg[2] = wk_RLTS;										/* RLTS 			*/
		MpReg[3] = wk_FILOBS;									/* FILOBS 			*/
	}
#endif

}



/****************************************************************************************************/
/*																									*/
/*		ASIC micro: Torque filter management														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency :	Manage ON/OFF of torque filter of ASIC microprogram.					*/
/*																									*/
/*	Supplementary Foot :	Usually called from the parameter calculation routine.										*/
/*			If exclusive control of the torque filter is required, use this module to handle it.					*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void KpiManageMicroTorqueFilter( VUSHORT *mreg_csw_ptr, USHORT *CswData, USHORT FilSel, LONG FilOn )
{
	/* Get Current Status */
	*CswData = *mreg_csw_ptr;

/*--------------------------------------------------------------------------------------------------*/
/*		Set/Clear Filter Disable Flag																*/
/*--------------------------------------------------------------------------------------------------*/
	if( FilOn == TRUE )
	{
		*CswData &= ~FilSel;			/* Clear Filter Disable Flag					*/
	}
	else
	{
		*CswData |= FilSel;				/* Set   Filter Disable Flag					*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Set to ASIC Micro Register																	*/
/*--------------------------------------------------------------------------------------------------*/
	*mreg_csw_ptr = *CswData;

}



/****************************************************************************************************/
/*																									*/
/*		2nd order notch filter parameter calculation															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			     2*(wx^2 - hx^2)				  hx^2 + dx*wx*hx/qx + wx^2  						*/
/*		K1 = ------------------------   	K2 = ---------------------------						*/
/*			  hx^2 + wx*hx/qx + wx^2               hx^2 + wx*hx/qx + wx^2							*/
/*																									*/
/*																									*/
/*			  hx^2 - wx*hx/qx + wx^2 			  hx^2 - dx*wx*hx/qx + wx^2							*/
/*		K3 = ------------------------   	K4 = ---------------------------						*/
/*			  hx^2 + wx*hx/qx + wx^2               hx^2 + wx*hx/qx + wx^2							*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	KpiPcalMicroNotchFilter2(				/* 2nd order notch filter parameter calculation				*/
		ASICS	*SvAsicRegs,					/* JL-076Access structure							*/
		LONG	hz,								/* Filter frequency		[Hz]						*/
		LONG	qx,								/* Filter Q constant		[0.01]						*/
		LONG	dx,								/* Filter depth			[0.001]						*/
		SHORT	*MpReg	)						/* Filter parameter setting start address				*/
{
	LONG	ts;
	LONG	hx;
	LONG	wx;
	LONG	cx[3];
	LONG	freq;
	USHORT	kf[4];

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( qx < 50 )
	{ /* Lower Limit for qx */
		qx = 50;
	}
	if( dx > 1000 )
	{ /* Upper Limit for dx */
		dx = 1000;
	}
	/*----------------------------------------------------------------------------------------------*/
	/* Filter calculation period setting */
	/*----------------------------------------------------------------------------------------------*/
	ts = KPI_MBCYCLENS/100;
	/*----------------------------------------------------------------------------------------------*/
	/* Frequency correction																					*/
	/*----------------------------------------------------------------------------------------------*/
	if ( hz > 2000 )
	{
		freq = (LONG)( (SHORT)hz * (SHORT)(3 * hz - 1894) + 11788000) / 10000;
	}
	else
	{
		freq = hz;
	}
	/*----------------------------------------------------------------------------------------------*/
	hx = 10000000 / ts;											/* hx = (2/ts)*(1/2)				*/
	wx = (freq * 12868) >> 12;									/* wx = (hz*2*PAI)*(1/2)			*/
	/*----------------------------------------------------------------------------------------------*/
	while( hx > 23170 )
	{ /* Scaling : hx^2 < 2^29 */
		hx = hx>>1; wx = wx>>1;
	}
	while( wx > 23170 )
	{ /* Scaling : wx^2 < 2^29 */
		hx = hx>>1; wx = wx>>1;
	}
	/*----------------------------------------------------------------------------------------------*/
	cx[0] = MlibScalKxgain( 100*wx, hx, qx,   NULL, -30 );		/* cx[0] = wx*hx/qx					*/
	cx[1] = MlibScalKxgain( cx[0],  dx, 1000, NULL, -30 );		/* cx[1] = dx*wx*hx/qx				*/
	cx[2] = hx*hx + cx[0] + wx*wx;								/* cx[2] = hx*hx + wx*hx/qx + wx*wx	*/
	/*----------------------------------------------------------------------------------------------*/
	kf[0] = (USHORT)MlibScalKxgain( 2*(wx*wx - hx*hx),       (1<<13), cx[2], NULL, -24 );	/* K1	*/
	kf[1] = (USHORT)MlibScalKxgain( (hx*hx + cx[1] + wx*wx), (1<<13), cx[2], NULL, -24 );	/* K2	*/
	kf[2] = (USHORT)MlibScalKxgain( (hx*hx - cx[0] + wx*wx), (1<<13), cx[2], NULL, -24 );	/* K3	*/
	kf[3] = (USHORT)MlibScalKxgain( (hx*hx - cx[1] + wx*wx), (1<<13), cx[2], NULL, -24 );	/* K4	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Writing to micro IF register															*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /* 2012.09.04 Y.Oka ★μAdditional parameter bulk writing function to program is required★ */
	MicroTranslatePrmReq( SvAsicRegs, kf, MpReg, 4 );
#else /* Provisional measures */
	MpReg[0]	= (USHORT)kf[0];						/* Write k[0] to ASIC Micro Register		*/
	MpReg[1]	= (USHORT)kf[1];						/* Write k[1] to ASIC Micro Register		*/
	MpReg[2]	= (USHORT)kf[2];						/* Write k[2] to ASIC Micro Register		*/
	MpReg[3]	= (USHORT)kf[3];						/* Write k[3] to ASIC Micro Register		*/
#endif
/*--------------------------------------------------------------------------------------------------*/

}



/****************************************************************************************************/
/*																									*/
/*		1st order low pass filter parameter calculation														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*				Ts  																				*/
/*		kf = --------- * 2^14																		*/
/*			  Tf + Ts   																			*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	KpiPcalMicroLpassFilter1(		/* 1st order low pass filter parameter calculation				*/
		LONG	tf,								/* Filter time constant		[xs]						*/
		LONG	ts,								/* Filter calculation period		[xs]						*/
		SHORT	*MpReg )						/* Filter parameter setting start address				*/
{
	LONG	kf;

	kf = MlibScalKxgain( ts, (1<<14), (ts + tf), NULL, -24 );
	*MpReg = (USHORT)kf;						/* Write kf to ASIC Micro Register					*/

}



/****************************************************************************************************/
/*																									*/
/*		Parameter setting to ASIC(Write)															*/
/*																									*/
/****************************************************************************************************/
LONG	LpxSetPrmToASIC( SHORT *adr, LONG prm, BOOL limchk )
{
	USHORT	rdata;
	VUSHORT	dummy;
	LONG	prmx;
	LONG	rc;

	prmx = prm;
	rc = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		Check Parameter Limit																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( limchk == REQ_LMT_CHK )
	{
		if( (prm > 32767) || (prm < 0) )				/* Limit Check								*/
		{
			prmx = MlibLimitul( prm, 32767, 0 );		/* Limit : 0 <= prm <= 32767				*/
			/* 2009.06.04 Y.Oka (>_<) Parameter limit required */
//			KpiSetPrmLmtInfo( pdef );					/* Parameter limit information setting				*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Write to ASIC and Verify																	*/
/*--------------------------------------------------------------------------------------------------*/
	*adr	= (USHORT)prmx;								/* Write to ASIC Register					*/
	dummy = *adr;										/*	KPX_ASIC_DUMMY_READ( dummy );			*/
	rdata = *adr;										/* Read from ASIC Register					*/
	if( rdata != (USHORT)prmx )							/* Verify Write Data						*/
	{
		*adr	= (USHORT)prmx;							/* Write to ASIC Register					*/
		dummy = *adr;									/*	KPX_ASIC_DUMMY_READ( dummy );			*/
		rdata = *adr;									/* Read from ASIC Register					*/
		if( rdata != (USHORT)prmx )						/* Verify Write Data						*/
		{
			rc = -1;									/* A.BF2 : System alarm 2(MicroAbnormal)	*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	return(rc);

}


#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
/****************************************************************************************************/
/*																									*/
/*		Microparameter batch transfer request processing															*/
/*																									*/
/*			※Sizemaximum value is32																	*/
/*																									*/
/****************************************************************************************************/
LONG	MicroTranslatePrmReq( ASICS *SvAsicRegs, void *Data, VUSHORT *Address, LONG Size )
{
	LONG			i;
	USHORT			*TbufAddress;
	ASIC_U_IF_STR	*pAsicMicroReg;
	ULONG			Timout;

	pAsicMicroReg = SvAsicRegs->AsicMicroReg;

	/* Check Micro Program Running Status */
	if( SvAsicRegs->MicroRunningSts == TRUE )
	{
		/* Check Request Data Size */
		if( Size > MICRO_TR_DATA_MAX )
		{
			return	MICRO_TR_DATA_ERROR;
		}

		/* Check Src Data Address and Dst Data Address */
		if( (Data == NULL) || (Address == NULL) )
		{
			return	MICRO_TR_DATA_ERROR;
		}

		/* Check and Wait Finish Last Micro Translation */
		Timout = 0;
		while( pAsicMicroReg->MREG_TR_END != MICRO_TR_READY )
		{
			/* Wait for Ready */
			KlibWaitus( KPI_SACYCLEUS );
			if((++Timout) > (KPI_SCCYCLEMS*KPI_SACOUNTMS))
			{
				return	MICRO_TR_DATA_ERROR;
			}
		}

		/* Make Target Address (Micro Address) */
		pAsicMicroReg->MREG_TADDR = (USHORT)( ((LONG)Address & MICRO_ADDRESS_CHANGE) >> 1 );

		/* Set Parameter Data */
		TbufAddress = (USHORT*)&(pAsicMicroReg->MREG_TBUF_0);
		for( i=0; i<Size; i++ )
		{
			TbufAddress[i] = ( (USHORT*)Data )[i];
		}

		/* Set Prameter Number (2byte unit) */
		pAsicMicroReg->MREG_TNUM = (USHORT)Size;

		/* Set Parameter Renewal Request (2byte unit) */
		pAsicMicroReg->MREG_TR_END = MICRO_TR_SIZE_SHORT;

		/* ServoASICFailureNo.3 Correspondence(Dummy write) *//* 2008.04.02 */
		pAsicMicroReg->MREG_DUMMY = 0x0000;									/* Dummy write				*/
	}
	else
	{ /* Micro Program Stopped State */
		for( i=0; i<Size; i++ )
		{
			( (USHORT*)Address )[i] = ( (USHORT*)Data )[i];
		}
	}
	return	MICRO_TR_SUCCESS;
}
#endif



/***************************************** end of file **********************************************/

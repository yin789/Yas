/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MotorPrmCfg.c : Motor encoder related processing module											*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Define motor encoder related processing functions													*/
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
#include "SerialEncIf.h"
#include "JL086ServoIpHw.h"
#include "MLib.h"
#include "KLib.h"
#include "PnPrmListTbl.h"

#define	SVCDEF_DIVOUT_MAXFREQ		1802240				/* Division output pulse upper limit frequency definition				*/
#define	SVFSEL_INCPOUT				1					/* Incre pulse output						*/
#define	SVFSEL_PAOPOUT				1					/* PAO phase pulse output						*//*<S02D>*/

#define	UNIT_FEMT					-15
#define	UNIT_PICO					-12
#define	UNIT_NANO					-9

#define	EvenAsciiCode_A				( 'A'  + 0x00 )		/* 0 100 0001	 <S14F> */
#define	EvenAsciiCode_L				( 'L'  + 0x80 )		/* 1 100 1100	 <S14F> */
#define	EvenAsciiCode_M				( 'M'  + 0x00 )		/* 0 100 1101	 <S14F> */
#define	EvenAsciiCode_P				( 'P'  + 0x00 )		/* 0 101 0000	 <S14F> */
#define	EvenAsciiCode_Puls			( '+'  + 0x00 )		/* 0 010 1011	 <S14F> */
#define	EvenAsciiCode_Conm			( ','  + 0x80 )		/* 1 010 1100	 <S14F> */
#define	EvenAsciiCode_Mins			( '-'  + 0x00 )		/* 0 010 1101	 <S14F> */
#define	EvenAsciiCode_Prio			( '.'  + 0x00 )		/* 0 010 1110	 <S14F> */
#define	EvenAsciiCode_Cr			( 0x0D + 0x80 )		/* 1 000 1101	 <S14F> */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
static	LONG	RmxSencCalMencDivOverSpeed( LONG pgrate, KSGAIN OvrSpd );
static DIVSET_ERROR RmxSencSetMencDividedPulse( ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms );
static DIVSET_ERROR LmxSencSetMencDividedPulse( ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms );
static DIVSET_ERROR LmxSencCalMencDivOverSpeed( MENCV *MencV, UNI_PRM_STR *UniPrms );

static LONG	SencCheckMturnLimit( LONG MtnCur, MENCV *MencV );							  			  /*<S02D>*/
static void	LmxSencGetLinearAbsPos( MENCV  *MencV, BOOL RvsDir ); 									  /*<S02D>*/
static void	SencOutputPaoMtdata( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg, ULONG digit );  			  /*<S02D>*/
static DIVSET_ERROR RmxSencSetMencDividedPulseHw( ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms );/*<S02D>*/
static DIVSET_ERROR LmxSencSetMencDividedPulseHw( ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms );/*<S079>*/
static BOOL SencSetSensorType( MENCV *MencV, MENCPRM *MencP, USHORT b_prm2 );//<S0C7>
void SencOutputPsoSerial    (MENCV*,ASIC_HW_IF_STR*,SHORT,BOOL);/*<S189>*//*<S0C2>*/
void FencSencOutputPsoSerial(MENCV*,ASIC_HW_IF_STR*,SHORT,BOOL);/*<189>*//*<S0C2>*/
static void	OutputPsoMturnIncre( MENCV*,ULONG, ULONG,ASIC_HW_IF_STR*);/*<S189>*//*<S0C2>*/
static void	OutputPsoLinearIncre(LONG *PsoIncPulseOut, ULONG digit, ASIC_HW_IF_STR *pAsicHwReg);	/* <S20A>:Argument review */


/****************************************************************************************************/
/*																									*/
/*		Linear motor: Scale pitch calculation process														*/
/*																									*/
/****************************************************************************************************/
static BOOL	SencScalePitchCalc( MENCV *MencV )
{
	LONG			i;
	LONG			Index;
	ULONG			ulwk64[2];
	ULONG			ScalePitch;
	ULONG			ScaleExp;

/*--------------------------------------------------------------------------------------------------*/
/*		Scale pitch calculation	Unit:[fm]																*/
/*--------------------------------------------------------------------------------------------------*/
	ScalePitch = (MencV->ScalePitch[2]<<16) | (MencV->ScalePitch[1]<<8) | MencV->ScalePitch[0];
	ScaleExp = 1;
	for( i=0; i < MencV->ScalePitchExp; i++ )
	{
		ScaleExp = 10 * ScaleExp;
	}
	MlibMulu32u32( ScalePitch, ScaleExp, ulwk64 );

/*--------------------------------------------------------------------------------------------------*/
/*		Scale pitch upper/lower limit check																*/
/*			Lower limit:0x0000F4240(=1000000)[fm]														*/
/*			Upper limit value:0x5AF3FFFFFFFF(=100004018520063)[fm]											*/
/*--------------------------------------------------------------------------------------------------*/
	if( ulwk64[1]==0 )
	{
		if( ulwk64[0] < 0x000F4240 )
		{ /* Less than the lower limit value */
			return( FALSE );
		}
	}
	else
	{
		if( ulwk64[1] > 0x00005AF3 )
		{ /* Bigger than the upper limit */
			return( FALSE );
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Set scale pitch and scale pitch index for monitor	Unit:[pm]									*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MonScalePitch = ScalePitch;
	MencV->MonScaleExp = UNIT_FEMT - UNIT_PICO + MencV->ScalePitchExp;/* [pm]Convert to unit exponent	*/

/*--------------------------------------------------------------------------------------------------*/
/*		Scale pitch, scale pitch exponent setting	Unit:[nm]											*/
/*--------------------------------------------------------------------------------------------------*/
	Index = UNIT_FEMT - UNIT_NANO + MencV->ScalePitchExp;				/* [nm]Convert to unit exponent	*/

	if( Index >= 0 )
	{
		ScaleExp = 1;
		for( i=0; i < Index; i++ )
		{
			ScaleExp = 10 * ScaleExp;
		}
		MencV->NormScalePitch = MlibScalKxgain( ScalePitch, ScaleExp, 1, NULL, -30 );
		MencV->NormScaleExp   = 1;
	}
	else
	{
		ScaleExp = 1;
		Index = -Index;
		for( i=0; i < Index; i++ )
		{
			ScaleExp = 10 * ScaleExp;
		}

		if( ScaleExp >= 10000)
		{
			MencV->NormScalePitch = MlibScalKxgain( ScalePitch, 10000, ScaleExp, NULL, -30 );
			MencV->NormScaleExp   = 10000;
		}
		else
		{
			MencV->NormScalePitch = ScalePitch;
			MencV->NormScaleExp   = ScaleExp;
		}
	}
	return( TRUE );
}


/****************************************************************************************************/
/*																									*/
/*		Rotary type: Encoder basic parameter calculation														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Servo basic parameter calculation(BprmCalc.c).			*/
/*			Basic parameter calculation result(Bprm.Xxxx/Hprm.Xxxx)and the initial parameter calculation result(Iprm.Xxxx)		*/
/*			Parameter calculations that do not use may be performed here.										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static SENC_ERROR	SencBaseParamCalcForRotary( MENCV *MencV, MENC_INIT_PRM *MencInitPrm )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Communication error detection level setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->ComErrLimit = MencInitPrm->spgfail.b.l;

	/*----------------------------------------------------------------------------------------------*/
	/*		Encoder pulse number setting																*/
	/*----------------------------------------------------------------------------------------------*/
	/*																								*/
	/*		PulseNo = 2^BitNo	[pulse/rev]															*/
	/*																								*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->PulseNo = (1 << MencInitPrm->MencP->bit_dp.b.l);			/* PulseNo = 2^BitNo			*/
	MencV->HalfPulseNo = (MencV->PulseNo >> 1);						/* HalfPulseNo = PulseNo / 2	*/

	MencV->EncBitNo = MencInitPrm->MencP->bit_dp.b.l;				/* Encoder bit number			*/	/* <S14B> */

#if 0 /*<S02D>*/
	if( MencInitPrm->MencP->bit_dp.b.l <= 13 )						/* Pulse conversion13bitSpecification			*/
	{
		MencV->PlsTrn13bit = TRUE;
	}
	else
	{
		MencV->PlsTrn13bit = FALSE;
	}
#else/*<S02D>*/
	if( MencInitPrm->MencP->bit_dp.b.l <= 13 )						/* Pulse conversion13bitSpecification			*/
	{
		MencV->DivInfo.PlsTrn13bit = TRUE;
	}
	else
	{
		MencV->DivInfo.PlsTrn13bit = FALSE;
	}

#endif/*<S02D>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Power output motor phase sequence setting																	*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->PhaseOrderUWV = FALSE;								/* Current, unused					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Pole sensor presence/absence setting												<V250>					*/
/*			This is a form that does not take into account compatibility with rotary type motors from other companies.												*/
/*			Since Yaskawa motors have pole sensors, the presence or absence of Hall sensors is fixed.							*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->withPoleSensor = TRUE;								/* Only compatible with Yaskawa motor			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Multiturn limit check																*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->DivInfo.Limmlt = MencInitPrm->MencP->limmlt;   /*<S02D>*/
	MencV->DivInfo.Bit_dp = MencInitPrm->MencP->bit_dp;   /*<S02D>*/

#if !defined(CSW_NETIF_SYNQNET_TYPE)
	if( (MencV->AbsoEncType == MENC_MABSO_TYPE)
	/*  ||((MencV->AbsoEncType == MENC_SABSO_TYPE)&&(Kprm.NetId == NETID_DPRAM)) */ )	/*<S03F>*/
	{
		if(/* ( MencV->DivOut == TRUE ) &&*/ ( MencInitPrm->mtlmt != MencInitPrm->MencP->limmlt ) )
		{
			return SENCERR_MTLMT;/* A.CC0 : Multiturn limit value mismatch */
		}
	}
#endif
	return SENCERR_SUCCESS;
}


/****************************************************************************************************/
/*																									*/
/*		Linear motor: Encoder basic parameter calculation													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Servo basic parameter calculation(BprmCalc.c).			*/
/*			Basic parameter calculation result(Bprm.Xxxx/Hprm.Xxxx)and the initial parameter calculation result(Iprm.Xxxx)		*/
/*			Parameter calculations that do not use may be performed here.										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static SENC_ERROR	SencBaseParamCalcForLinear( MENCV *MencV, MENC_INIT_PRM *MencInitPrm )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Communication error detection level setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->ComErrLimit = MencInitPrm->spgfail.b.l;

	/*----------------------------------------------------------------------------------------------*/
	/*		Scale pitch setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	if(MencV->ScalePitchExp == 0xFF)
	{ /* Disable serial converter scale pitch information */
		/*------------------------------------------------------------------------------------------*/
		/*	Pn282																*/
		/*------------------------------------------------------------------------------------------*/
		MencV->MonScalePitch = MencInitPrm->scalepitch2l;
		MencV->MonScaleExp = SCALE_EXP_PM; /* [pm]Unit exponent			*/
		MencV->NormScalePitch = MencInitPrm->scalepitch2l * 10;
		MencV->NormScaleExp = SCALE_EXP_UM; /* [nm]Unit exponent			*/
	}
	else
	{ /* Enable serial converter scale pitch information */
		/* Set scale pitch from serial converter information */
		if(FALSE == SencScalePitchCalc(MencV))
		{
			/* Clear if calculation result is NG */
			MencV->MonScalePitch = 0;
			MencV->MonScaleExp = 0;
			MencV->NormScalePitch = 0;
			MencV->NormScaleExp = 0;
		}
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		Encoder pulse number setting																*/
	/*----------------------------------------------------------------------------------------------*/
	/*																								*/
	/*					(2*PolePitch) * (100000) * (2^BitNo)		PulseNo    : [pulse/360deg]		*/
	/*		PulseNo = -----------------------------------------		PolePitch  : [0.1mm/180deg]		*/
	/*								ScalePitch						ScalePitch : [nm]				*/
	/*																								*/
	/*		Caution:PulseNo, the calculation error of the magnetic pole position will accumulate in normal calculations.			*/
	/*			  1) PulseNo = 460800 [pulse/360deg], PolePitch=18.0mm, ScalePitch=20.00um			*/
	/*			  2) PoleErr = 360 / 460800  [deg/(2*PolePitch)]									*/
	/*			  3) PoleErr = (360/460800) * 1000/(2*18.0) = 0.02 [deg/m]							*/
	/*			  4) The cumulative error is small(0.02deg/m), so ignore it for the time being.								*/
	/*																								*/
	/*----------------------------------------------------------------------------------------------*/
	if(MencV->NormScalePitch != 0)
	{
		MencV->PulseNo = MlibDiv6432( MencInitPrm->MencP->ppitch * MencV->NormScaleExp,
									  (200000 << MencInitPrm->MencP->bit_dp.b.l),
									  MencV->NormScalePitch );
		/*------------------------------------------------------------------------------------------*/
#if 0
		if( 0 ) /* PulseNoRemainder check: Since the cumulative error is small, ignore it for the time being(SGDVSpecification).	*/
		{
			MlibMulu32u32( MencV->PulseNo, MencV->NormScalePitch, xx );
			MlibMulu32u32((prm_ptr->ppitch*MencV->NormScaleExp),(200000 << prm_ptr->bit_dp.b.l), yy );
			if( (xx[0] != yy[0]) || (xx[1] != yy[1]) ) /* Remainder check							*/
			{
				KpiSetParamError( &pndef_ppitch ); /* A.040 : Setting parameter error		*/
			}
		}
#endif
	}
	/*----------------------------------------------------------------------------------------------*/
	else
	{
		return SENCERR_SCALE;								/* A.080 : Scale pitch setting error		*/
	}
	/*----------------------------------------------------------------------------------------------*/
	MencV->HalfPulseNo = (MencV->PulseNo >> 1);				/* HalfPulseNo = PulseNo / 2			*/
//	MencV->PlsTrn13bit = FALSE;								/* Pulse conversion13bitSpecificationFlag				*/
	MencV->DivInfo.PlsTrn13bit = FALSE;						/* Pulse conversion13bitSpecificationFlag				*//*<S02D>*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Setting the number of pulses for origin check															*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->PulseNo30deg = MencV->PulseNo / 12;				/* PulseNo30deg [Pulse/30deg]			*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Pn080.0 : Pole sensor presence/absence setting															*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->withPoleSensor = (0 == (MencInitPrm->b_prm80 & 0x0F)) ? TRUE : FALSE;
	/*----------------------------------------------------------------------------------------------*/
	/*		Pn080.1 : Motor phase order setting																*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->PhaseOrderUWV = (0 == ((MencInitPrm->b_prm80 >> 4) & 0x0F)) ? FALSE : TRUE;
	/*----------------------------------------------------------------------------------------------*/
	/*		Pn080.2 : U-phase latch monitoring selection setting														*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->MotOrgChkAlways = (0 == ((MencInitPrm->b_prm80 >> 8) & 0x0F)) ? TRUE : FALSE;
	/*----------------------------------------------------------------------------------------------*/
	/*		Pn080.3 : Maximum settable speed/dividing ratio calculation selection setting										*/
	/*----------------------------------------------------------------------------------------------*/
//	MencV->PhaseOrderUWV = (0 == ((b_prm80 >> 12) & 0x0F)) ? FALSE : TRUE;

	return SENCERR_SUCCESS;
}

/****************************************************************************************************/
/*																									*/
/*		Serial encoder variable initialization process															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : In initial processing, read the initial position data of the serial encoder,					*/
/*			Set location data and previous location.														*/
/*			Also, depending on the encoder type, set the C-phase output and do not output the initial pulse.				*/
/*																									*/
/****************************************************************************************************/
SENC_ERROR	SencCalcBaseParameters(MENCV *MencV, MENC_INIT_PRM *MencInitPrm )
{
	SENC_ERROR	err;

	/*----------------------------------------------------------------------------------------------*/
	/*		Division pulse output selectionFlag															*/
	/*----------------------------------------------------------------------------------------------*/
	/* 2011.01.11 Y.Oka Division pulse output not supported */
	//	MencV->DivOut = TRUE;
#if 0 /*<S02D>*/
    	MencV->DivOut = FALSE;
	/*----------------------------------------------------------------------------------------------*/
	/*		Supported by sample version hardware																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( MencV->DivOutEnable == TRUE )
	{
		MencV->DivOut = TRUE;
	}
#endif /*<S02D>*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Processing when encoder is not connected																*/
	/*----------------------------------------------------------------------------------------------*/
	if( MencV->EncConnect == FALSE )
	{
		MencV->AbsoEncType = MENC_INCRE_TYPE;
		MencV->AxisMotType = MOTTYPE_ROTARY;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		Serial encoder basic parameter calculation													*/
	/*----------------------------------------------------------------------------------------------*/
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* Linear type */
		err = SencBaseParamCalcForLinear( MencV, MencInitPrm );
	}
	else
	{ /* Rotation type */
		err = SencBaseParamCalcForRotary( MencV, MencInitPrm );
	}

	return err;
}


/****************************************************************************************************/
/*																									*/
/*		Motor voltage code conversion																		*/
/*																									*/
/****************************************************************************************************/
static UCHAR	SencGetMotorVoltCode( USHORT volt )
{
	switch( volt )
	{
	case 100 : return( 0x00 );								/* AC 100V								*/
	case 200 : return( 0x01 );								/* AC 200V								*/
	case 400 : return( 0x02 );								/* AC 400V								*/
	case  24 : return( 0x03 );								/* DC  24V								*/
	case  48 : return( 0x04 );								/* DC  48V								*/
	default  : return( 0x00 );								/* Default : AC 100V					*/
	}
}


#pragma arm section code = "CALL_SCANA" /* <S1D2> */
/****************************************************************************************************/
/*																									*/
/*		Multiturn data acquisition processing																	*/
/*																									*/
/****************************************************************************************************/
static	USHORT	SencGetMturnData(MENCV *MencV, USHORT limmlt, DBYTEX bit_dp, USHORT PosH, ULONG PosL )
{
	USHORT	Mturn;

/*--------------------------------------------------------------------------------------------------*/
/*		Multi-rotation absolute encoder																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AbsoEncType == MENC_MABSO_TYPE )
	{
		Mturn = (USHORT)(((PosH<<16) | ((ULONG)PosL>>16)) >> (bit_dp.b.h - 64));
		if( MencV->RevAsmMotor )
		{
			if( limmlt == 65535 )
			{
				Mturn = -Mturn;
			}
			else if( Mturn != 0 )
			{
				Mturn = limmlt - Mturn;
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		One-rotation absolute encoder																		*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		Mturn = 0;
	}
/*--------------------------------------------------------------------------------------------------*/
	return( Mturn );

}
#pragma arm section code


/****************************************************************************************************/
/*																									*/
/*		Absolute encoder type setting																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Encoder format iss-abs(***A*-*****) ,Pn002.2twist						*/
/*			Absolute encoder type(MencP.AbsoEncType).										*/
/*																									*/
/*			Pn002.2 = 0 : Use according to encoder specifications												*/
/*			Pn002.2 = 1 : Using absolute as increment												*/
/*			Pn002.2 = 2 : Used as a one-turn absolute encoder				 <S16A> 					*/
/****************************************************************************************************/
static void	SencSetAbsoEncType( MENCV *MencV, USHORT b_prm2, USHORT typm_e, USHORT *limmlt )
{
	if( 1 == ((b_prm2>>8) & 0x0F) )
	{	/* Using absolute as increment	*/
		MencV->AbsoEncType = MENC_INCRE_TYPE;
	}
	else if(( 2 == ((b_prm2>>8) & 0x0F) ) && (MencV->SerialConvType == SCONVTYPE_NONE))
	{	/* Used as serial one-rotation absolute encoder	*/
		*limmlt = 0;
		MencV->AbsoEncType = MENC_SABSO_TYPE;
	}
	else
	{	/* Use according to encoder specifications */
		if(typm_e & 0x1000)
		{
			MencV->AbsoEncType = MENC_MABSO_TYPE;
		}
		else if(typm_e & 0x2000)
		{
			MencV->AbsoEncType = MENC_SABSO_TYPE;
		}
		else
		{
			MencV->AbsoEncType = MENC_INCRE_TYPE;
		}
	}
}

/****************************************************************************************************/
/*																									*/
/*		Parameter exponent calculation																		*/
/*																									*/
/****************************************************************************************************/
static BOOL	SencExpConvert( UCHAR prm, UCHAR *buf )
{
	BOOL	rc;
	SHORT	swrk;

	rc = TRUE;

/*--------------------------------------------------------------------------------------------------*/
/*		Exponent part analysis																				*/
/*--------------------------------------------------------------------------------------------------*/
	if ( prm == 0xFF )										/* Exponent part: zero(10^0)					*/
	{
		swrk = 0x0000;
	}
	else if ( prm & 0x80 )									/* Exponential part: Negative(10^-n)					*/
	{
		swrk = -(prm & 0x7F);
	}
	else													/* Exponential part: Positive(10^+n)					*/
	{
		swrk = (SHORT)prm;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Exponent part check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( (swrk > 8) || (swrk < -8) )							/* Exponent part range check					*/
	{
		rc = FALSE;
	}
/*--------------------------------------------------------------------------------------------------*/
	*buf = (UCHAR)swrk;
	return( rc );
}

/****************************************************************************************************/
/*																									*/
/*		Encoder type code setting																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Absolute/incremental/converter determination based on encoder ID information, andΣ-3					*/
/*			Perform internal settings for encoder judgment.														*/
/*																									*/
/****************************************************************************************************/
BOOL	SencSetEncoderType( MENCV *MencV, MENCPRM *MencP, USHORT b_prm2, BOOL SencFbOptUse ) /* <S16A> */
{
	UCHAR 	*Model;
	BOOL	PrmError;

	Model = &(MencV->EncInfo.Model[0]);
	PrmError = FALSE;

	/*----------------------------------------------------------------------------------------------*/
	/*		Absolute/incremental/converter judgment														*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->SerialConvType = SCONVTYPE_NONE; /* Serial converter type initialization		*/
	MencV->OrgClrByWrAdrCmd = FALSE; /* WRADR Command	*/

	if( SencFbOptUse == FALSE )
	{/*	When feedback option is not used (conventional processing)										*/
	switch(Model[3])
	{
	case 'A': /* Serial multi-rotation absolute encoder												*/
		MencV->AxisMotType = MOTTYPE_ROTARY;
		MencV->AbsoEncoder = TRUE;
		MencP->typm_e.b.h  |= 0x10; /* <S16A> */
		SencSetAbsoEncType(MencV, b_prm2, MencP->typm_e.w, &(MencP->limmlt));	/* <S179> */
		break;
			/*--------------------------------------------------------------------------------------*/
	case 'B': /* Serial one-rotation absolute encoder(Absolute specifications)									*/
		MencV->AxisMotType = MOTTYPE_ROTARY;
		MencV->AbsoEncoder = TRUE;
		MencP->limmlt = 0;
		MencP->typm_e.b.h  |= 0x20; /* <S16A> */
		SencSetAbsoEncType(MencV, b_prm2, MencP->typm_e.w, &(MencP->limmlt));	/* <S16A> */
		break;
			/*--------------------------------------------------------------------------------------*/
	case 'S': /* Serial one-rotation absolute encoder(Incre pulse specification)									*/
		MencV->AxisMotType = MOTTYPE_ROTARY;
		MencV->AbsoEncoder = TRUE;
		MencP->limmlt = 0;
		if(((b_prm2>>8) & 0x0F)== 2)
		{
			MencP->typm_e.b.h |= 0x20;
		}
		else
		{
			MencP->typm_e.b.h &= 0x0F;
		}
		SencSetAbsoEncType(MencV, b_prm2, MencP->typm_e.w, &(MencP->limmlt));	/* <S16A> */
		break;
			/*--------------------------------------------------------------------------------------*/
	case 'I': /* Serial incremental encoder													*/
		MencV->AxisMotType = MOTTYPE_ROTARY;
		MencV->AbsoEncoder = FALSE;
		MencV->AbsoEncType = MENC_INCRE_TYPE;
		MencP->typm_e.b.h &= 0x0F;
		break;
			/*--------------------------------------------------------------------------------------*/
	case 'P': /* Serial converter for Linear motor											*/
			PrmError = SencSetSensorType( MencV, MencP, b_prm2 );									/* <S04D> Convert to function */
			break;
			/*--------------------------------------------------------------------------------------*/
		default: // Default: Parameter error
			MencV->AxisMotType = MOTTYPE_ROTARY;
			/* A.CA0 : Encoder parameter error */
			PrmError = TRUE;
			break;
			/*--------------------------------------------------------------------------------------*/
		}
	
		/*------------------------------------------------------------------------------------------*/
		/*		Σ-3Encoder determination																	*/
		/*	<S04D>: Serial conversion unit or external rotary encoder(Format:JZDP)processing is added	*/
		/*------------------------------------------------------------------------------------------*/
		if( Model[3] == 'P')
		{/* Serial conversion unit or external rotary encoder(Format:JZDP)In the case of						*/
			if( MencV->SenSpecEnable == TRUE )
			{/* Encoder Prm 0x43 is enabled,0x43ofbit4,5				*/
				switch( (MencV->SenSpec >> 4) & 0x03 )
				{
			/*----------------------------------------------------------------------------------*/
				case 0x00 :	/* Σ-ⅡFormat	*/
					MencV->Sigma3Encoder = FALSE;
			break;
			/*----------------------------------------------------------------------------------*/
				case 0x01 :	/* Σ-ⅢFormat	*/
					MencV->Sigma3Encoder = TRUE;
			break;
			/*----------------------------------------------------------------------------------*/
				case 0x02 :	/* Σ-ⅤFormat	*/
					MencV->Sigma3Encoder = TRUE;
			PrmError = TRUE;
			break;
				/*----------------------------------------------------------------------------------*/
				default	  :	/* Default			*/
					MencV->Sigma3Encoder = FALSE;
		PrmError = TRUE;
		break;
	}
			}
			else
			{/* Encoder Prm 0x43 is invalid,Σ-ⅡFormat */
				MencV->Sigma3Encoder = FALSE;
			}
		}
		else
		{/* Yaskawa standard rotary encoders (Format:UT**)In the case of												*/
	switch(Model[2])
	{
	case 'D':
	case 'S':
		MencV->Sigma3Encoder = FALSE;
		break;
	case 'T':
		MencV->Sigma3Encoder = TRUE;
		break;
	default:
		PrmError = TRUE;
		MencV->Sigma3Encoder = FALSE;
		break;
	}
		}

		if( Model[3] == 'P')
		{/* JZDPIn the case of */
			if( MencV->SenSpecEnable == TRUE )
			{
				/* Parameters43His enabled43Hbit of3 */
				MencV->PoleWriteEnable = ((MencV->SenSpec & 0x08) == 0x08)? TRUE : FALSE;
			}
			else
			{
				/* Parameters43His invalid, judge by type */
				if(( MencV->SerialConvType == SCONVTYPE_MITUTOYO_ABS )||( MencV->SerialConvType == SCONVTYPE_ENDAT_ABS ))
		        {
		            MencV->PoleWriteEnable = TRUE;
				}
				else
				{
					MencV->PoleWriteEnable = FALSE;
				}
			}
		}
		else
		{/* Invalid for Yaskawa standard rotary encoders. */
			MencV->PoleWriteEnable = FALSE;
		}
	}
	else/* <S04D> */
	{/*	When using the feedback option														*/
		/*------------------------------------------------------------------------------------------*/
		/*	Encoder position sensor type setting															*/
		/*------------------------------------------------------------------------------------------*/
		PrmError = SencSetSensorType( MencV, MencP, b_prm2 );
		/*------------------------------------------------------------------------------------------*/
		/*	Σ-Ⅱ/Σ-ⅢEncoder determination															*/
		/*------------------------------------------------------------------------------------------*/
		/*	When using the feedback optionΣ-ⅡNo need to distinguish because it is only the type.FALSESetting.	*/
		/*------------------------------------------------------------------------------------------*/
		MencV->Sigma3Encoder = FALSE;
		/*------------------------------------------------------------------------------------------*/
		/*		Current phase parameter scale write permission setting											*/
		/*------------------------------------------------------------------------------------------*/
		/*	MencP.SenSpec(Addr:0x43) but bit7=1,bit6=0 Parameter information is valid only when				*/
		/*------------------------------------------------------------------------------------------*/
		if( MencV->SenSpecEnable == TRUE )
		{
			MencV->PoleWriteEnable = ((MencV->SenSpec & 0x08) == 0x08)? TRUE : FALSE;
		}
		else
		{/* Parameters43His invalid, judge by type */
			if(( MencV->SerialConvType == SCONVTYPE_MITUTOYO_ABS )||( MencV->SerialConvType == SCONVTYPE_ENDAT_ABS ))
	        {
	            MencV->PoleWriteEnable = TRUE;
			}
			else
			{
				MencV->PoleWriteEnable = FALSE;
			}
		}
	}
#if 0
/* <S04D>:Semi-closed control using feedback option				*/
/* SencSetMotorAndEncoderID()function.	*/
		/*------------------------------------------------------------------------------------------*/
	/*		Determine whether motor constant scale can be written														*/
		/*------------------------------------------------------------------------------------------*/
	if(MencV->AxisMotType == MOTTYPE_ROTARY)
	{
		MencV->ScaleWriteEnable = FALSE;
	}
	else if(MencV->AxisMotType == MOTTYPE_LINEAR)
	{
		if(((MencV->TypeCode & 0x0080) == FALSE) ||
			((MencV->EncConnect == TRUE) && (Model[1] == '\0') && (Model[0] == '\0')))
		{
			MencV->ScaleWriteEnable = TRUE;
		}
		else
		{
			MencV->ScaleWriteEnable = FALSE;
		}
	}
	else
	{
		MencV->ScaleWriteEnable = FALSE;
	}
#endif
	return PrmError;
}

/****************************************************************************************************/
/*																									*/
/*		Encoder position sensor type setting																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Determine absolute/incremental/converter etc. from encoder parameters and set internal variables.	*/
/*																									*/
/****************************************************************************************************//* <S04D> */
static BOOL SencSetSensorType( MENCV *MencV, MENCPRM *MencP, USHORT b_prm2 )
{
	BOOL	PrmError;
	
	MencV->AxisMotType = MOTTYPE_LINEAR;
	PrmError = FALSE;
	
	switch(MencV->TypeCode)
	{
	/*----------------------------------------------------------------------------------*/
	case 'A': /* Mitutoyo absolute type													*/
		MencV->SerialConvType = SCONVTYPE_MITUTOYO_ABS;
		MencV->AbsoEncoder = TRUE;
		MencP->typm_e.b.h |= 0x10; /* <S16A> */
		SencSetAbsoEncType(MencV, b_prm2, MencP->typm_e.w, &(MencP->limmlt));	/* <S16A> */
		MencV->AxisMotType = MOTTYPE_LINEAR;
		break;
	/*----------------------------------------------------------------------------------*/
	case 'B': /* Rotary type incremental encoder,INITCommand response								*/
		MencV->SerialConvType = SCONVTYPE_SONY_INC;
		MencV->AbsoEncoder = FALSE;
		MencP->typm_e.b.h &= 0x0F;
		MencV->AbsoEncType = MENC_INCRE_TYPE;
		MencV->AxisMotType = MOTTYPE_ROTARY;
		break;
	/*----------------------------------------------------------------------------------*/
	case 'C': /* Rotary absolute value encoder,	Origin setting possible								*/
		MencV->SerialConvType = SCONVTYPE_MITUTOYO_ABS;
		MencV->AbsoEncoder = TRUE;
		if( (MencV->SenSpecEnable == TRUE)&&((MencV->SenSpec & 0x02) == 0x02) )
		{
			MencP->typm_e.b.h |= 0x20;					/* Without multiturn(1Rotation absolute value)*/
			MencP->limmlt = 0;
		}
		else
		{
			MencP->typm_e.b.h |= 0x10;					/* With multiturn(Multi-rotation absolute value)*/
		}
		SencSetAbsoEncType(MencV, b_prm2, MencP->typm_e.w, &(MencP->limmlt));	/* <S16A> */
		MencV->AxisMotType = MOTTYPE_ROTARY;
		break;
	/*----------------------------------------------------------------------------------*/
	case 'D': /* SMS increment type													*/
		MencV->SerialConvType = SCONVTYPE_SONY_INC;
		MencV->AbsoEncoder = FALSE;
		MencP->typm_e.b.h &= 0x0F;
		MencV->AbsoEncType = MENC_INCRE_TYPE;
		MencV->AxisMotType = MOTTYPE_LINEAR;
		break;
	/*----------------------------------------------------------------------------------*/
	case 'E': /* EnDat type														*/
		MencV->SerialConvType = SCONVTYPE_ENDAT_ABS;
		MencV->AbsoEncoder = TRUE;
		MencP->typm_e.b.h |= 0x10;  /* <S16A> */
		SencSetAbsoEncType(MencV, b_prm2, MencP->typm_e.w, &(MencP->limmlt));	/* <S16A> */
		MencV->AxisMotType = MOTTYPE_LINEAR;
		break;
	/*----------------------------------------------------------------------------------*/
	case 'G': /* Rotary absolute value encoder, origin cannot be set									*/
		MencV->SerialConvType = SCONVTYPE_ENDAT_ABS;
		MencV->AbsoEncoder = TRUE;
		if( (MencV->SenSpecEnable == TRUE)&&((MencV->SenSpec & 0x02) == 0x02) )
		{
			MencP->typm_e.b.h |= 0x20;					/* Without multiturn(1Rotation absolute value)*/
			MencP->limmlt = 0;
		}
		else
		{
			MencP->typm_e.b.h |= 0x10;					/* With multiturn(Multi-rotation absolute value)*/
		}
		SencSetAbsoEncType(MencV, b_prm2, MencP->typm_e.w, &(MencP->limmlt));	/* <S16A> */
		MencV->AxisMotType = MOTTYPE_ROTARY;
		break;
/*--------------------------------------------------------------------------------------*/
	case 'J': /* Rotation increment encoder,WRITE_ADRCommand response							*/
		MencV->SerialConvType = SCONVTYPE_STANDARD_INC;
		MencV->AbsoEncoder = FALSE;
		MencP->typm_e.b.h &= 0x0F;
		MencV->AbsoEncType = MENC_INCRE_TYPE;
		MencV->OrgClrByWrAdrCmd = TRUE;		/* WRITE_ADREffective for origin clear by command*/
		MencV->AxisMotType = MOTTYPE_ROTARY;
		break;
	/*----------------------------------------------------------------------------------*/
	case 'L': /* Standard increment type														*/
		MencV->SerialConvType = SCONVTYPE_STANDARD_INC;
		MencV->AbsoEncoder = FALSE;
		MencV->AbsoEncType = MENC_INCRE_TYPE;
		MencP->typm_e.b.h &= 0x0F;
		MencV->OrgClrByWrAdrCmd = TRUE; 		/* WRADR CommandOrigin clearing enabled by	*/
		MencV->AxisMotType = MOTTYPE_LINEAR;
		break;
	/*----------------------------------------------------------------------------------*/
	default:
		/* A.CA0 : Encoder parameter error */
		PrmError = TRUE;
		break;
	}
	return PrmError;
}

/****************************************************************************************************/
/*																									*/
/*		Motor type code setting																		*/
/*																									*/
/****************************************************************************************************/
static void	SencSetMotorType( MENCV *MencV, MENCPRM *MencP )
 {
	UCHAR *Model;
	UCHAR MotType; /* <S03A> */
	UCHAR Seriese; /* <S03A> */

	Model = &(MencV->MotInfo.Model[0]);
	MencV->TuningLessLowGainMotor = FALSE; /* Initialize to not distinguish the model without adjustment		*/
	MencV->WeakenFieldEnable = FALSE; /* Initialize field weakening to not forcefully determine whether it is valid	*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Rotary type motor series discrimination												<S03A>			*/
	/*----------------------------------------------------------------------------------------------*/
	if( Model[3] == '7' )
	{ /* Σ-7Motor */
		Seriese = 3;
		MotType = 4;
	}
	else
	{
		Seriese = 4;
		MotType = 3;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		special motor																				*/
	/*----------------------------------------------------------------------------------------------*/
	if((Model[0] != 'S') || (Model[2] != 'M'))
	{
		MencP->typm_e.b.l = 0xFF;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Standard motor																				*/
	/*----------------------------------------------------------------------------------------------*/
	else
	{
//		switch(Model[3]) /* <S03A> */
		switch(Model[MotType])
		{
		/*------------------------------------------------------------------------------------------*/
		case 'A':
//			switch(Model[4]) /* <S03A> */
			switch(Model[Seriese])
			{
			case 'J':
				MencP->typm_e.b.l = 0x0D;
				break;
			default:
				MencP->typm_e.b.l = 0x00;
				break;
			}
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'P':
			MencP->typm_e.b.l = 0x01;
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'S':
			MencP->typm_e.b.l = 0x02;
			/* SGMS*Motor has field weakening forcibly enabled */
			MencV->WeakenFieldEnable = TRUE;
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'G':
			switch(Model[10])
			{
			case 'A':
			case 'C':
				MencP->typm_e.b.l = 0x03;
				/* SGMG*Set the motor to low-noise settings without adjustment */
				MencV->TuningLessLowGainMotor = TRUE;
				break;
			case 'B':
			case 'D':
				MencP->typm_e.b.l = 0x04;
				/* SGMG*Set the motor to low-noise settings without adjustment */
				MencV->TuningLessLowGainMotor = TRUE;
				break;
			default:
				break;
			}
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'D':
			MencP->typm_e.b.l = 0x05;
			/* SGMD*Set the motor to low-noise settings without adjustment */
			MencV->TuningLessLowGainMotor = TRUE;
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'U':
			MencP->typm_e.b.l = 0x06;
			/* SGMU*Set the motor to low-noise settings without adjustment */
			MencV->TuningLessLowGainMotor = TRUE;
			/* SGMU*Motor has field weakening forcibly enabled */
			MencV->WeakenFieldEnable = TRUE;
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'M':
//			switch(Model[4]) /* <S03A> */
			switch(Model[Seriese])
			{
			case 'J':
				MencP->typm_e.b.l = 0x11;
				break;
			default:
				MencP->typm_e.b.l = 0x07;
				break;
			}
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'B':
			switch(Model[10])
			{
			case 'A':
				MencP->typm_e.b.l = 0x08;
				break;
			case 'B':
				MencP->typm_e.b.l = 0x09;
				break;
			case 'C':
				MencP->typm_e.b.l = 0x0B;
				break;
			case 'D':
				MencP->typm_e.b.l = 0x0C;
				break;
			default:
				break;
			}
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'T':
			MencP->typm_e.b.l = 0x0A;
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'J':
			MencP->typm_e.b.l = 0x0D;
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'K':
			MencP->typm_e.b.l = 0x11;
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'C':
			switch(Model[8])
			{
			case 'C':
				MencP->typm_e.b.l = 0x12;
				break;
			case 'D':
				MencP->typm_e.b.l = 0x13;
				break;
			case 'B':
				MencP->typm_e.b.l = 0x14;
				break;
			case 'E':
				MencP->typm_e.b.l = 0x15;
				break;
			case 'L':
				MencP->typm_e.b.l = 0x16;
				break;
			case 'M':
				MencP->typm_e.b.l = 0x17;
				break;
			case 'N':
				MencP->typm_e.b.l = 0x18;
				break;
			case 'R':
				MencP->typm_e.b.l = 0x19;
				break;
			default:
				MencP->typm_e.b.l = 0x1F;
				break;
			}
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'E':
			MencP->typm_e.b.l = 0x0E;
			break;
			/*--------------------------------------------------------------------------------------*/
		case 'Z':
			MencP->typm_e.b.l = 0x0F;
			break;
			/*--------------------------------------------------------------------------------------*/
		default:
			MencP->typm_e.b.l = 0xFF;
			break;
		}
		/*------------------------------------------------------------------------------------------*/
//		switch(Model[4]) /* <S03A> */
		switch(Model[Seriese])
		{
		case 'S':
			MencP->typm_e.b.l |= 0x20;
			/* Increment encoder temperature detection disabled */
			MencV->IncDetOhFlg = FALSE; /* <S03A> */
			break;
		case 'V':
			MencP->typm_e.b.l |= 0x60;
			/* VDo not distinguish the motor model without adjustment */
			MencV->TuningLessLowGainMotor = FALSE;
			/* VMotor does not determine whether field weakening is forcibly enabled */
			MencV->WeakenFieldEnable = FALSE;
			/* Increment encoder temperature detection disabled */
			MencV->IncDetOhFlg = FALSE; /* <S03A> */
			break;
		case '7': /* <S03A> */
			MencP->typm_e.b.l |= 0xA0;
			/* 7Do not distinguish the motor model without adjustment */
			MencV->TuningLessLowGainMotor = FALSE;
			/* 7Motor does not determine whether field weakening is forcibly enabled */
			MencV->WeakenFieldEnable = FALSE;
			/* 7Motor has incre encoder temperature detection enabled */
			MencV->IncDetOhFlg = TRUE;
			break;
		default:
			break;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Linear motor																			*/
	/*----------------------------------------------------------------------------------------------*/
	if(MencV->AxisMotType == MOTTYPE_LINEAR)
	{
		if(MencV->TypeCode == 'E')
		{
			MencP->typm_e.b.l = 0x41;
		}
		else
		{
			MencP->typm_e.b.l = 0x40;
		}
		/* Initialize linear motor without adjusting model discrimination */
		MencV->TuningLessLowGainMotor = FALSE;
		/* Linear motor does not determine whether forced field weakening is enabled */
		MencV->WeakenFieldEnable = FALSE;
		/*------------------------------------------------------------------------------------------*/
		/*	Coreless linear motor type determination	<V108>													*/
		/*------------------------------------------------------------------------------------------*/
		switch(Model[3])
		{
		case 'G':
			MencV->CoreLessLinearMotor = TRUE; /* Coreless linear				*/
			break;
		default:
			MencV->CoreLessLinearMotor = FALSE;
			break;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Σ-TracFormat determination		<V335>																*/
		/*------------------------------------------------------------------------------------------*/
		switch(Model[2])
		{
		case 'T':
			switch(Model[3])
			{
			case 'M':
			case 'F':
				MencV->SigmaTracMotor = TRUE; /* Σ-Trac μ, MAG				*/
				break;
			default:
				MencV->SigmaTracMotor = FALSE;
				break;
			}
			break;
		default:
			MencV->SigmaTracMotor = FALSE;
			break;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Low speed motor flag setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	if(MencV->AxisMotType == MOTTYPE_ROTARY)
	{
//		kx = MlibExpCal(MencP->exp_motw_spd.b.h);
//		kx = MlibScalKskxkx(kx, (100 * MencP->vrat_max.b.h), 1, NULL, -30);
		MencV->LowSpdMotor = (Model[3] == 'C') ? TRUE : FALSE;
	}
	else
	{
		MencV->LowSpdMotor = FALSE;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Inversion connection flag setting																		*/
	/*----------------------------------------------------------------------------------------------*/
	if(Model[20] == 'R')
	{
		MencV->RevAsmMotor = TRUE;
	}
	else
	{
		MencV->RevAsmMotor = FALSE;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Linear: Reverse connection flag disabled setting															*/
	/*----------------------------------------------------------------------------------------------*/
	if(MencV->AxisMotType == MOTTYPE_LINEAR)
	{
		MencV->RevAsmMotor = FALSE;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Field-weakening control forced enable setting																*/
	/*----------------------------------------------------------------------------------------------*/
	if(MencV->WeakenFieldEnable == TRUE)
	{ /* Field weakening enable flagON */
		MencP->flg_wf |= 0x0001; /* VoltageFBMethod field-weakening control enabled			*/
	}
}


/****************************************************************************************************/
/*																									*/
/*		Serial encoder : Encoder & motorIDAcquisition/setting processing										*/
/*																									*/
/****************************************************************************************************/
static BOOL	SencSetMotorAndEncoderID( MENCV *MencV, MENCPRM *MencP, OPFBK_HNDL *FbkOpt,			/* <S04D>:FbkOptaddition */
										 USHORT b_prm2, UCHAR *EncPrmBuf, BOOL SencFbOptUse )	/* <S04D>:SencFbOptUseaddition */
{
	BOOL err;

	/*----------------------------------------------------------------------------------------------*/
	/*		Linear type code setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->TypeCode = EncPrmBuf[0x40];

	/*----------------------------------------------------------------------------------------------*/
	/*		Encoder type setting																	*/
	/* <S04D> mod( EncInfo -> pActEncInfo )														*/
	/*----------------------------------------------------------------------------------------------*/
	SencSetIDINFO( 0x00, &EncPrmBuf[0x00], MencV->pActEncInfo );
	SencSetIDINFO( 0x08, &EncPrmBuf[0x08], MencV->pActEncInfo );
	SencSetIDINFO( 0x10, &EncPrmBuf[0x10], MencV->pActEncInfo );
	SencSetIDINFO( 0x18, &EncPrmBuf[0x18], MencV->pActEncInfo );

	SencFillNullIDINFO( MencV->pActEncInfo );
	err = SencSetEncoderType( MencV, MencP, b_prm2, SencFbOptUse );/* <S04D>:SencFbOptUseaddition */

	/*----------------------------------------------------------------------------------------------*/
	/*		Motor type setting																		*/
	/* <S04D> mod( MotInfo -> pActMotInfo )														*/
	/*----------------------------------------------------------------------------------------------*/
	SencSetIDINFO( 0x20, &EncPrmBuf[0x20], MencV->pActMotInfo );
	SencSetIDINFO( 0x28, &EncPrmBuf[0x28], MencV->pActMotInfo );
	SencSetIDINFO( 0x30, &EncPrmBuf[0x30], MencV->pActMotInfo );
	SencSetIDINFO( 0x38, &EncPrmBuf[0x38], MencV->pActMotInfo );
	SencSetIDINFO( 0x98, &EncPrmBuf[0x98], MencV->pActMotInfo );

	SencSetMotorType( MencV, MencP);
	SencFillNullIDINFO( MencV->pActMotInfo );
	
	/*--------------------------------------------------------------------------------------------------*/
	/*		Feedback option module information setting													*/
	/*--------------------------------------------------------------------------------------------------*/
	/*		Current serial conversion type feedback option moduleIDInformation is encodedIDTo information area		*/
	/*		EncoderIDCreate from information.										<S04D>	*/
	/*--------------------------------------------------------------------------------------------------*/
	if( SencFbOptUse == TRUE )
	{
		FbOptSetIDInfo(FbkOpt, MencV, SencFbOptUse );
	}
	

	return err;
}

/****************************************************************************************************/
/*																									*/
/*		Serial encoder : Linear motor incompatibility determination process												*/
/*																									*/
/****************************************************************************************************//*<S1BE>*/
static SENC_ERROR SencCheckLinearMotor( MENCV *MencV, USHORT syssw1 )
{
	SENC_ERROR	err;
	err = SENCERR_SUCCESS;
	/*----------------------------------------------------------------------------------------------*/
	/*  Linear motor incompatibility determination process																	*/
	/*		PnE0B.1 =	0	: Rotation type, linear compatible													*/
	/*				=	1	: Rotary type compatible, linear not compatible(A.051Occurrence)									*/
	/*----------------------------------------------------------------------------------------------*/
	if(syssw1 & 0x0002)
	{
		if(MencV->AxisMotType == MOTTYPE_LINEAR)
		{/* A.051 : Product not supported alarm	*/
			err = SENCERR_NOTSUPPORT;
		}
	}
	return err;
}

/****************************************************************************************************/
/*																									*/
/*		Serial encoder : Initializing all parameters												*/
/*																									*/
/****************************************************************************************************/
static SENC_ERROR	SencInitEncParameters( MENCV *MencV, MENCPRM *MencP, UCHAR  *EncPrmBuf)
{
	USHORT		addr;
	UCHAR		*PrmBuf;
	SENC_ERROR	err;
	LONG 		kx;

	err = SENCERR_SUCCESS;
	/*----------------------------------------------------------------------------------------------*/
	/* Encoder parameter setting processing */
	/*----------------------------------------------------------------------------------------------*/
	/* Bank0.															*/
	for( addr = 0x0048; addr < 0x00A0; addr += 8 )
	{
		PrmBuf = &EncPrmBuf[addr];
		switch(addr)
		{
		case 0x48: /* Encoder bit number, decimal point position, multiturn limit					*/
			MencV->DivInfo.IncreBit_dp = (UCHAR)PrmBuf[0]; /* Compatible models:21-24Bit increment *//*<S161>*/
			MencP->bit_dp.b.l = ( PrmBuf[2] == (UCHAR)0xFF )? 0 : PrmBuf[2];
			MencP->bit_dp.b.h = PrmBuf[3];
			MencP->limmlt = ((USHORT) PrmBuf[7] << 8) | (USHORT) PrmBuf[6];

			/* Scale pitch																*/
			if(MencV->AxisMotType == MOTTYPE_LINEAR)
			{
				MencV->ScalePitch[0] = PrmBuf[4]; /* Scale pitch lower				*/
				MencV->ScalePitch[1] = PrmBuf[5]; /* Scale pitch middle				*/
				MencV->ScalePitch[2] = PrmBuf[6]; /* Scale pitch upper				*/
				MencV->ScalePitchExp = PrmBuf[7]; /* Scale pitch index				*/
			}

			/* Sampling cycle setting for encoder position correction */
			MencV->EncLchNum = 0;
			if(MencV->AxisMotType == MOTTYPE_ROTARY)
			{
				if(MencP->bit_dp.b.l > 13)
				{
					if(MencP->typm_e.b.l == 0x11)
					{/* Sigma3 motor : SGMMJ */
						MencV->EncLchNum = 28; /* 16bit,17bit : 28/6M =4.67us(for SGMMJ)	*/
					}
					else if(MencP->typm_e.b.l < 0x60)
					{/* Sigma2, Sigma3 motor */
						MencV->EncLchNum = 16; /* 16bit,17bit : 16/6M =2.67us				*/
					}
					else
					{/* Sigma5 motor or later */
						;
					}
				}
			}

			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x50: /* Phase angle correction value															*/
			if(MencV->AxisMotType == MOTTYPE_LINEAR)
			{
				MencP->phscmpM2 = ((USHORT) PrmBuf[1] << 8) | (USHORT) PrmBuf[0];
			}
			else
			{
				MencP->phscmpM2 = 0;
			}
			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x58: /* Communication speed setting, parameter expansion area selection, encoder software version	*/
			MencP->exmotprm = (USHORT) ~PrmBuf[4];				/* 0Bit inversion is valid for		*/
			MencP->chng_baudrate = (USHORT) ~PrmBuf[5];			/* 0Bit inversion is valid for		*/
			MencP->encver = (USHORT) PrmBuf[7];
			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x60: /* Rated output, number of motor poles, rated torque									*/
			MencP->motw = ((USHORT) PrmBuf[1] << 8) | (USHORT) PrmBuf[0];
			MencP->oslv_pol.b.h = PrmBuf[2];
			MencP->rattrq = ((USHORT) PrmBuf[5] << 8) | (USHORT) PrmBuf[4];
			if(TRUE != SencExpConvert(PrmBuf[6], &(MencP->exp_trat_jmot.b.l)))
			{
				err = SENCERR_PRMDATA;
			}
			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x68: /* Maximum torque, rated current, maximum current, EMF compensation							*/
			MencP->maxtrq = (((USHORT) PrmBuf[2] << 8) | (USHORT) PrmBuf[1]);
			MencP->irat = (((USHORT) PrmBuf[4] << 8) | (USHORT) PrmBuf[3]);
			MencP->imax = (((USHORT) PrmBuf[6] << 8) | (USHORT) PrmBuf[5]);
			MencP->emfcmp = (MencP->emfcmp & 0xFF00U) | (USHORT) PrmBuf[7];
			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x70: /* EMF compensation,Rated speed,Maximum speed,OS level,Inertia,Winding resistance			*/
			MencP->emfcmp = (MencP->emfcmp & 0x00FFU) | ((USHORT) PrmBuf[0] << 8);
			MencP->vrat_max.b.l = PrmBuf[1];
			MencP->vrat_max.b.h = PrmBuf[2];
			MencP->oslv_pol.b.l = PrmBuf[3];
			MencP->jmot = ((USHORT) PrmBuf[5] << 8) | (USHORT) PrmBuf[4];
			if(TRUE != SencExpConvert(PrmBuf[6], &(MencP->exp_trat_jmot.b.h)))
			{
				err = SENCERR_PRMDATA;
			}
			MencP->motr = (MencP->motr & 0xFF00U) | (USHORT) PrmBuf[7];
			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x78: /* Winding resistance, inductance, base torque, intermediate torque					*/
			MencP->motr = (MencP->motr & 0x00FFU) | ((USHORT) PrmBuf[0] << 8);
			MencP->motl = ((USHORT) PrmBuf[2] << 8) | (USHORT) PrmBuf[1];
			MencP->bastrq = (USHORT) PrmBuf[5];
			MencP->midtrq = ((USHORT) PrmBuf[7] << 8) | (USHORT) PrmBuf[6];
			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x80: /* Intermediate time, intermediate time 2, intermediate torque 2									*/
			MencP->midolt = (USHORT) PrmBuf[0];
			MencP->mtolt2 = (USHORT) PrmBuf[1];
			MencP->mtrq2 = ((USHORT) PrmBuf[3] << 8) | (USHORT) PrmBuf[2];
			MencP->flg_wf = (USHORT) PrmBuf[4];
			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x88: /* Phase compensation 1, phase compensation 2(2004.06.29), Motor voltage, Pole pitch			*/
			MencP->phscmpM1.b.l = PrmBuf[2];
			MencP->poleoffset = ((USHORT) PrmBuf[3] >= 180) ? 0 : ((USHORT) PrmBuf[3] << 1);
			MencP->typm_e.b.h
					|= SencGetMotorVoltCode(((USHORT) PrmBuf[5] << 8) | (USHORT) PrmBuf[4]);
			MencP->ppitch = ((USHORT) PrmBuf[7] << 8) | (USHORT) PrmBuf[6];
			break;
			/*--------------------------------------------------------------------------------------*/
		case 0x90: /* q-axis inductance(Lq0,Lq1), Index, phase compensation start speed(2004.06.29)		*/
			MencP->motlq0 = ((USHORT) PrmBuf[1] << 8) | (USHORT) PrmBuf[0];
			MencP->motlq1 = ((USHORT) PrmBuf[3] << 8) | (USHORT) PrmBuf[2];

			if(TRUE != SencExpConvert(PrmBuf[4], &(MencP->exp_motw_spd.b.l)))
			{
				err = SENCERR_PRMDATA;
			}

			if(TRUE != SencExpConvert(PrmBuf[5], &(MencP->exp_motw_spd.b.h)))
			{
				err = SENCERR_PRMDATA;
			}

			MencP->phscmpM1.b.h = 0;
			if((ULONG) PrmBuf[6] < 50)
			{
				MencP->motresfrq = (USHORT) (PrmBuf[6] * 100);
			}
			else if((ULONG) PrmBuf[6] > 250)
			{
				MencP->motresfrq = 0;
			}
			else
			{
				MencP->motresfrq = (USHORT) (PrmBuf[6] * 10);
			}
			break;
		default:
			break;
		}
	}

///	if( MencP->exmotprm == TRUE )		/* Bank6Validity check<S16E> */
	if( (SENCBR_EXAREA1 & MencP->exmotprm) == TRUE )		/* Bank6Validity check<S16E> *//* <S1A6> */
	{
		/* Bank6.															*//* <S0D7> */
		for( addr = 0x00A0; addr < 0x00E0; addr += 8 )
		{
			PrmBuf = &EncPrmBuf[addr];
			switch(addr - 0x00A0)/* 0x600The value obtained from the address is0x00A0		*/
			{
			case 0x00:	/* Function flag 2,PWMFrequency, low speed overload base current/intermediate current 1/2.Intermediate time		*/
				MencP->motfuncf.ul	=  (USHORT) PrmBuf[0];										/* PnF24	*//*Supports motor carrier frequency switching<S117>*/
				MencP->mot_pwmf	= ((USHORT) PrmBuf[2] << 8) | (USHORT) PrmBuf[1];			/* PnF25	*/
				MencP->mot_ibs	 = (USHORT) PrmBuf[3];										/* PnF26	*/
				MencP->mot_imd	= ((USHORT) PrmBuf[5] << 8) | (USHORT) PrmBuf[4];			/* PnF27	*/
				MencP->mot_imdt	=  (USHORT) PrmBuf[6];										/* PnF28	*/
				MencP->mot_imd2	=  (USHORT) PrmBuf[7];										/* PnF29	*/
				break;
				/*--------------------------------------------------------------------------------------*/
			case 0x08:	/* Low speed overload intermediate time 2, rotation speed threshold, motorOCDetection current, adjustment-less mode selection		*/
						/* Voltage command limit level(Voltage command limit level,dVoltage limit level for axis current command limit)	*/
				MencP->mot_imdt2		= ((USHORT) PrmBuf[1] << 8) | (USHORT) PrmBuf[0];	/* PnF2A	*/
				MencP->mot_ichgspd		=  (USHORT) PrmBuf[2];								/* PnF2B	*/
				MencP->mot_oc			= ((USHORT) PrmBuf[4] << 8) | (USHORT) PrmBuf[3];	/* PnF2C	*/
				MencP->mot_TunLess		=  (USHORT) PrmBuf[5];								/* PnF2D	*/
				MencP->RefAdjust_ForWF.b.l	=  PrmBuf[6];									/* PnF2E.l	*/
				MencP->RefAdjust_ForWF.b.h	=  PrmBuf[7];									/* PnF2E.h	*/
				break;
				/*--------------------------------------------------------------------------------------*/
			case 0x10:	/* Field weakeningLCorrection coefficient, phase advance compensation during regeneration(Start speed, maximum rotation speed)				*/
				MencP->LCompForWF = (USHORT) PrmBuf[0];										/* PnF2F	*/
				MencP->RegPhscmp.b.l = PrmBuf[1];											/* PnF30.l	*/
				MencP->RegPhscmp.b.h = PrmBuf[2];											/* PnF30.h	*/
				break;
				/*--------------------------------------------------------------------------------------*/
			case 0x18:	/* Preliminary area																		*/

				break;
				/*--------------------------------------------------------------------------------------*/
			case 0x20:	/* IPMControl parameter area schedule													*/
			case 0x28:
			case 0x30:
			case 0x38:

				break;
				/*--------------------------------------------------------------------------------------*/
			default:
				break;
			}
		}
	}
	else
	{/* bank6Initial value when invalid0xFFFF */
		MencP->motfuncf.ul	= 0xFFFF;													/* PnF24	*//*Supports motor carrier frequency switching<S117>*/
		MencP->mot_pwmf	= 0xFFFF;														/* PnF25	*/
		MencP->mot_ibs	 = 0xFFFF;														/* PnF26	*/
		MencP->mot_imd	= 0xFFFF;														/* PnF27	*/
		MencP->mot_imdt	= 0xFFFF;														/* PnF28	*/
		MencP->mot_imd2	= 0xFFFF;														/* PnF29	*/
		MencP->mot_imdt2		= 0xFFFF;												/* PnF2A	*/
		MencP->mot_ichgspd		= 0xFFFF;												/* PnF2B	*/
		MencP->mot_oc			= 0xFFFF;												/* PnF2C	*/
		MencP->mot_TunLess		= 0xFFFF;												/* PnF2D	*/
		MencP->RefAdjust_ForWF.b.l	= 0xFFFF;											/* PnF2E.l	*/
		MencP->RefAdjust_ForWF.b.h	= 0xFFFF;											/* PnF2E.h	*/
		MencP->LCompForWF = 0xFFFF;														/* PnF2F	*/
		MencP->RegPhscmp.b.l = 0xFFFF;													/* PnF30.l	*/
		MencP->RegPhscmp.b.h = 0xFFFF;													/* PnF30.h	*/
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Low speed motor flag setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	if(MencV->AxisMotType == MOTTYPE_ROTARY)
	{
		kx = MlibExpCal(MencP->exp_motw_spd.b.h);
		kx = MlibScalKskxkx(kx, (100 * MencP->vrat_max.b.h), 1, NULL, -30);
		MencV->LowSpdMotor |= ((kx <= 500) ? TRUE : FALSE);
	}

	return err;
}

/****************************************************************************************************/
/*																									*/
/*		Serial encoder : Communication connection processing															*/
/*																									*/
/****************************************************************************************************/
SENC_ERROR	SencInitMotorEncoder( MENCV *MencV, MENC_INIT_PRM *MencInitPrm,	ASIC_HW_IF_STR *AsicHwReg,
										OPFBK_HNDL *FbkOpt, USHORT EncBaudrate, BOOL SencFbOptUse )/* <S04D>:FbkOpt,SencFbOptUseaddition */
{
//	UCHAR 		EncPrmBuf[160];	/* 0xA0	*//* <S0D7> */
	UCHAR 		EncPrmBuf[224];	/* 0xE0	*//* <S0D7> */
	SENC_ERROR	err;
	err = SENCERR_SUCCESS;

	/*----------------------------------------------------------------------------------------------*/
	/*		CONNECT																					*/
	/*----------------------------------------------------------------------------------------------*/
	err |= SencConnect( MencV, AsicHwReg, EncBaudrate, FbkOpt, SencFbOptUse );

	if( SENCERR_SUCCESS == err )	/* Execute in case success encoder connect */
	{
		/*------------------------------------------------------------------------------------------*/
		/*		Read Encoder parameters																*/
		/*------------------------------------------------------------------------------------------*/
		err |= SencReadParamAll( MencV, EncPrmBuf );
		if( SENCERR_SUCCESS != err )
		{	/* Try again... */
			err |= SencReadParamAll( MencV, EncPrmBuf );
		}
		if( SENCERR_SUCCESS == err )	/* Execute in case success read encoder parameter */
		{
			/*--------------------------------------------------------------------------------------*/
			/*		Encoder/MotorIDSetting processing														*/
			/*--------------------------------------------------------------------------------------*/
			if( FALSE != SencSetMotorAndEncoderID(MencV, MencInitPrm->MencP, FbkOpt,			/* <S04D>:FbkOptaddition */
												MencInitPrm->b_prm2, EncPrmBuf, SencFbOptUse) )	/* <S04D>:SencFbOptUseaddition */
			{
				err |= SENCERR_PRMDATA;
			}
			/*--------------------------------------------------------------------------------------*/
			/*		Linear motor incompatibility determination process														*/
			/*--------------------------------------------------------------------------------------*//* <S1BE> */
			if( FALSE != SencCheckLinearMotor(MencV, MencInitPrm->syssw1) )
			{
				err |= SENCERR_NOTSUPPORT;
			}
			/*--------------------------------------------------------------------------------------*/
			/*		Sending origin reset command														*/
			/*--------------------------------------------------------------------------------------*/
			/* <S04D> Added "Semiconductor by feedback option" and "Absolute value encoder" to the conditional statement		*/
			if( (( MencV->AxisMotType == MOTTYPE_LINEAR ) || ( SencFbOptUse == TRUE ))
																&&( MencV->AbsoEncoder == TRUE ) )
			{
				err |= SencSendRstOrgCmd( MencV );
			}
			/*--------------------------------------------------------------------------------------*/
			/*		Encoder parameter initialization processing													*/
			/*--------------------------------------------------------------------------------------*/
			err |= SencInitEncParameters( MencV, MencInitPrm->MencP, EncPrmBuf );

			/*--------------------------------------------------------------------------------------*/
			/*		Encoder base parameter calculation process												*/
			/*--------------------------------------------------------------------------------------*/
			err |= SencCalcBaseParameters( MencV, MencInitPrm );
		}
	}
	else/* <S04D> */
	{
		/*--------------------------------------------------------------------------------------*/
		/*		Encoder base parameter calculation process												*/
		/*--------------------------------------------------------------------------------------*/
		err |= SencCalcBaseParameters( MencV, MencInitPrm );
	}

	/* 2013.08.29 Y.Oka P1After mode8MBpsChange after switching */
	/*----------------------------------------------------------------------------------------------*/
	/* P1Mode setting(Success of the above process/Not limited to failureP1Mode (62.5usChange to communication cycle)->ScanTo not change the cycle) */
	/*----------------------------------------------------------------------------------------------*/
//	SencChangeP1Mode( MencV );

	return err;
}



#if 0
/****************************************************************************************************/
/*																									*/
/*		Pulse IF initialization																			*/
/*																									*/
/*		Note Argument is incorrect5wereSV_OFF.Y.Oka						*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC Command counter setting value table																*/
/*--------------------------------------------------------------------------------------------------*/
const	USHORT	AsicPocsetTable[7] =
{
		0x0650,		/* 0 : ~A,~B, 101=Sign+Pulse		*/
		0x0640,		/* 1 : ~A,~B, 100=CW+CCW			*/
		0x0630,		/* 2 : ~A,~B, 011=(A+B)*1			*/
		0x0610,		/* 3 : ~A,~B, 001=(A+B)*2			*/
		0x0600,		/* 4 : ~A,~B, 000=(A+B)*4			*/
		0x0050,		/* 5 :  A, B, 101=Sign+Pulse		*/
		0x0040,		/* 6 :  A, B, 100=CW+CCW			*/
};
/*--------------------------------------------------------------------------------------------------*/
static void	InitAsicPulseIF( ALMMAN_HANDLE *AlmMngr, UNI_PRM_STR *UniPrms, ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV )
{
	PRMDATA		*prm_ptr;
	LONG		i;
	USHORT		uswk;
	BOOL		err;
	prm_ptr = UniPrms->Prm;

/*--------------------------------------------------------------------------------------------------*/
/*		Command counter setting																			*/
/*--------------------------------------------------------------------------------------------------*/
	i = prm_ptr->prstsw & 0x0F;							/* Command pulse form (Pn200)					*/
	if( i > 6 )											/* Parameter setting range check				*/
	{
		uswk = 0;
		/* Parameter setting errors */
		ALMSetPramError( AlmMngr, &pndef_prstsw );
	}
	else
	{
		uswk = AsicPocsetTable[i];						/* Count mode,etc						*/
	}
	/*----------------------------------------------------------------------------------------------*/
	if( ((prm_ptr->prstsw >> 4) & 0x0F) < 2 )
	{
		uswk = ( uswk | 0x0800 );							/* CCPolarity setting							*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		CA,CB Filter settings																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (prm_ptr->prstsw >> 12) & 0x0F )
	{
	case 0x00:											/* Command input filter 1 for line driver signals	*/
		/* (PnE0B.4-5), set with */
		 uswk = (USHORT)(uswk | ((prm_ptr->syssw1 >> 4) & 0x3) );
		 break;
	case 0x01:											/* Command input filter for open collector signal	*/
		/* (PnE0E.0-1), set with */
		 uswk = (USHORT)(uswk | ((prm_ptr->comfil)      & 0x3) );
		 break;
	case 0x02:											/* Command input filter 2 for line driver signals	*/
		/* (PnE0E.8-9), set with */
		 uswk = (USHORT)(uswk | ((prm_ptr->comfil >> 8) & 0x3) );
		 break;
	default:
		/* (PnE0B.4-5), set with */
		 uswk = (USHORT)(uswk | ((prm_ptr->syssw1 >> 4) & 0x3) );
		 break;
	}
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_POCSET	= uswk;						/* Command counter setting							*/
	uswk = ( uswk | 0x0100 );							/* CLMLatch enable setting					*/
	AsicHwReg->AREG_POCSET	= uswk ;					/* Command counter setting							*/
	AsicHwReg->AREG_LODSET	= AREG_LODSET_SETV;			/* Counter load setting						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Command filter setting																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (prm_ptr->prstsw >> 12) & 0x0F )
	{
	case 0x00:											/* Command input filter 1 for line driver signals	*/
		/* (PnE0B.8-11), set with */
		 AsicHwReg->AREG_FCCSET	= (USHORT)((prm_ptr->syssw1 & 0x0F00) );
		 break;
	case 0x01:											/* Command input filter for open collector signal	*/
		/* (PnE0E.4-7), set with */
		 AsicHwReg->AREG_FCCSET	= (USHORT)((prm_ptr->comfil << 4) & 0x0F00);
		 break;
	case 0x02:											/* Command input filter 2 for line driver signals	*/
		/* (PnE0E.12-15), set with */
		 AsicHwReg->AREG_FCCSET	= (USHORT)((prm_ptr->comfil >> 4) & 0x0F00);
		 break;
	default:
		/* (PnE0B.8-11), set with */
		 AsicHwReg->AREG_FCCSET	= (USHORT)(prm_ptr->syssw1 & 0x0F00);
		 break;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder pulse conversion setting(BitNumber,Conversion cycle) @JL-056Equivalent setting @						*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->PlsTrn13bit == TRUE )					/* Pulse conversion13bit							*/
	{
		AsicHwReg->AREG_PLSET	= 0x3100;					/* ConvertBitNumber:13bit, Conversion cycle:64us			*/
	}
	else
	{
		AsicHwReg->AREG_PLSET	= 0x3400;					/* ConvertBitNumber:16bit, Conversion cycle:64us			*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder pulse conversion setting(Position,Origin,etc)												*/
/*--------------------------------------------------------------------------------------------------*/
	// Pulse conversion C phase mask(As a precaution)
	AsicHwReg->AREG_WDT1SET	|=	BIT8;
	MencV->DivCMaskStatus = DIV_CMASK_ON;

	AsicHwReg->AREG_DIVSET	= 0x0102;					/* Pulse conversion writing side setting:HostCPU			*/
	AsicHwReg->AREG_POSET0	= 0xFFFF;					/* Pulse conversion position setting						*/
	AsicHwReg->AREG_POSET0H	= 0x0000;				/* Pulse conversion position settingbit16-23				*/
	AsicHwReg->AREG_POTEST	= 0xFFFF;					/* Pulse conversion counter setting					*/
	AsicHwReg->AREG_POSET1	= 0x0000;					/* Pulse conversion origin correction 1						*/
	AsicHwReg->AREG_POSET2	= 0xFFFF;					/* Pulse conversion origin correction 2						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Division input filter setting																		*/
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_FBCSET	= 0x0005;					/* Division input filter setting						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Motor type processing: Frequency division pulse calculation & ASIC setting												*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* Linear type */
		err = LmxSencSetMencDividedPulse( AsicHwReg, MencV, UniPrms );
	}
	else
	{ /* Rotation type */
		err = RmxSencSetMencDividedPulse( AsicHwReg, MencV, UniPrms );
	}

	if(err != FALSE)
	{
		ALMSetServoAlarm(AlmMngr, ALM_DIVPRM );		/* A.041 : Division pulse output setting error		*/
	}
}
#endif

//#if 0 /* MENCVIs it necessary to change related structures? *//*<S02D>Delete*/
/****************************************************************************************************/
/*																									*/
/*		Divided pulse IF initialization																		*/
/*																									*/
/****************************************************************************************************/
DIVSET_ERROR SencInitDivPulseIF( ASIC_HW_IF_STR* AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms )
{
	DIVSET_ERROR	err;
	LONG	BitNo;  
	LONG	pgrate;        /*<S079>*/

	BitNo  = UniPrms->Prm->MencP.bit_dp.b.l;								/* PnF03				*//*<S02D>*/

	if( MencV->AxisMotType != MOTTYPE_LINEAR )								/*<S079>*/
	{ /* Rotation type */
		pgrate = (1 << BitNo) / (UniPrms->Prm->pgrat2l << 2);				/* Pn212(Pn213)			*/
		if( pgrate > 65536 )
		{ /* Division ratio range:1/65536 , use soft division.  */
			MencV->DivInfo.DivOutASIC = FALSE;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder pulse conversion setting(BitNumber,Conversion cycle) @JL-056Equivalent setting @						*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*	PLSET:Pulse conversion setting																			*/
/*--------------------------------------------------------------------------------------------------*/
/*	VULONG	AREG_PLSET;		 0x0168 Pulse conversion setting													*/
								/*------------------------------------------------------------------*/
								/*  bit.7-0  :Pulse conversion hysteresis setting							*/
								/*------------------------------------------------------------------*/
								/*	bit.11-8 :Pulse conversion bit number setting								*/
								/*             0000:12bit											*/
								/*             0001:13bit											*/
								/*                |													*/
								/*             1011:23bit											*/
								/*       1100-1111:24bit											*/
								/*------------------------------------------------------------------*/
								/*  bit.14-12 :Pulse conversion cycle selection									*/
								/*					  Period             Operation clock DDANumber of bits		*/
								/*				000: 16us              32MHz          9				*/
								/*				001: 32us              32MHz         10				*/
								/*				010: 64us              32MHz         11				*/
								/*				011: 64us              16MHz         10(Initial value)	*/
								/*				100: 128us              8MHz         10				*/
								/*				101-111: 1.024ms   1MHz							*/
								/*------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->DivInfo.DivOutASIC != FALSE )
	{
        AsicHwReg->AREG_PLSET	= 0x00003000 + ((BitNo - 12) << 8); /* ConvertBitNumber:BitNo-12, Conversion cycle:64us*//*<S079>*/
	}
	else
	{
		if( MencV->DivInfo.PlsTrn13bit == TRUE )			/* Pulse conversion13bit							*/
		{
			AsicHwReg->AREG_PLSET	= 0x00003100;			/* ConvertBitNumber:13bit, Conversion cycle:64us			*/
		}
		else
		{
			AsicHwReg->AREG_PLSET	= 0x00003400;			/* ConvertBitNumber:16bit, Conversion cycle:64us			*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder pulse conversion setting(Position,Origin,etc)												*/
/*--------------------------------------------------------------------------------------------------*/
	// Pulse conversion C phase mask(As a precaution)
	AsicHwReg->AREG_WDT1SET	|=	BIT8;
	MencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;

/*--------------------------------------------------------------------------------------------------*/
/*	DIVSET:Division function setting																				*/
/*--------------------------------------------------------------------------------------------------*/
/*	VULONG	AREG_DIVSET;		 0x0164 Division function setting												*/
								/*------------------------------------------------------------------*/
								/*  bit.1-0 :Data pulse conversion position data write selection						*/
								/*            0: SerialPGIFFrom channel 0 (Initial value)						*/
								/*  		  1: SerialPGIFFrom channel 1								*/
								/*  		  2: HostCPUto											*/
								/*  		  3: From built-in calculation unit										*/
								/*------------------------------------------------------------------*/
								/*  bit.2 : Divide clock selection											*/
								/*            0:26.7MHz (Initial value)									*/
								/*  		  1:32MHz												*/
								/*------------------------------------------------------------------*/
								/*  bit.4-3 :Pulse conversion origin correction value setting source selection						*/
								/*            0: SerialPGIFFrom channel 0 (Initial value)						*/
								/*  		  1: SerialPGIFFrom channel 1								*/
								/*  		  2: HostCPUto											*/
								/*  		  3: From built-in calculation unit										*/
								/*------------------------------------------------------------------*/
								/*  bit.8 : DIVSEL0													*/
								/*            0:PA,PB,PC				 							*/
								/*  		  1: Data pulse conversion output (initial value)							*/
								/*------------------------------------------------------------------*/
								/*  bit.11-10 :DCSEL												*/
								/*            0: NormalCPhase (Initial value)									*/
								/*  		  1: Frequency divisionCPhase											*/
								/*  		  2: Frequency divisionCPhase2											*/
								/*  		  3: Frequency divisionCPhase2											*/
								/*------------------------------------------------------------------*/
								/*  bit.13 : DIVASEL												*/
								/*            0:DAOutput (Initial value)									*/
								/*  		  1: Serial data through output									*/
								/*------------------------------------------------------------------*/
								/*  bit.15 : DIVCBIT												*/
								/*            0:DCOutput (Initial value)									*/
								/*  		  1:DCHigh level forced output									*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->DivInfo.DivOutASIC != FALSE )
	{
		if(MencV->AbsoEncType == MENC_INCRE_TYPE)		/*<S079>*/
		{
			if( MencV->AbsoEncoder == FALSE ) /*<S0D3>*/
			{
				AsicHwReg->AREG_DIVSET	= 0x00000900;  		/* Pulse conversion write side setting:SerialPGIFFrom channel 0	*/
			}
			else
			{
				AsicHwReg->AREG_DIVSET	= 0x00000910;  		/* Pulse conversion write side setting:SerialPGIFFrom channel 0	*/
			}
		}
		else
		{ /* In the case of an appo encoder, after the initial increment output is completed, the serialPGIFFrom channel 0 */
			AsicHwReg->AREG_DIVSET	= 0x00000912;  		/* Pulse conversion write side setting:HostCPU				*/
		}
	}
	else
	{
		AsicHwReg->AREG_DIVSET	= 0x00000112;  			/* Pulse conversion write side setting:HostCPU,				*/
	}
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_POSET0	= 0xFFFFFFFF;				/* Pulse conversion position setting						*/
	AsicHwReg->AREG_POSET1	= 0x00000000;				/* Pulse conversion origin correction 1						*/
	AsicHwReg->AREG_POSET2	= 0xFFFFFFFF;				/* Pulse conversion origin correction 2						*/
	AsicHwReg->AREG_POTEST	= 0xFFFFFFFF;				/* Pulse conversion counter setting					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Division input filter setting																		*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*	FBCSET:Feedback pulse counter																		*/
/*--------------------------------------------------------------------------------------------------*/
/*	VULONG	AREG_FBCSET;	     0x0040 Division input filter setting										*/
								/*------------------------------------------------------------------*/
								/*  bit.1-0 :PA,PBFilter selection											*/
								/*            0:26.7MHzSynchronization (Initial value)								*/
								/*  		  1:37.5ns												*/
								/*  		  2:75ns												*/
								/*  		  3: Clear												*/
								/*------------------------------------------------------------------*/
								/*  bit.3-2 : PCFilter selection											*/
								/*            0:26.7MHzSynchronization (Initial value)								*/
								/*  		  1:37.5ns												*/
								/*  		  2:75ns												*/
								/*  		  3: Clear												*/
								/*------------------------------------------------------------------*/
								/*  bit.5-15 : Omitted													*/
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_FBCSET	= 0x00000005;				/* Division input filter setting						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Reverse rotation mode(Pn000.0 : Rotation direction selection(Reverse rotation mode))Setting										*//* <S21B> */
/*--------------------------------------------------------------------------------------------------*/
	MencV->DivInfo.RvsDir = UniPrms->Bprm->RvsDir;
/*--------------------------------------------------------------------------------------------------*/
/*		Motor type processing: Frequency division pulse calculation & ASIC setting												*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->DivInfo.DivOutASIC != FALSE )
	{
		if( MencV->AxisMotType == MOTTYPE_LINEAR )
		{ /* Linear type */
			err = LmxSencSetMencDividedPulseHw( AsicHwReg, MencV, UniPrms ); /*<S079>*/
		}
		else
		{ /* Rotation type */
			err = RmxSencSetMencDividedPulseHw( AsicHwReg, MencV, UniPrms );
		}
	}
	else
	{
		if( MencV->AxisMotType == MOTTYPE_LINEAR )
		{ /* Linear type */
			err = LmxSencSetMencDividedPulse( AsicHwReg, MencV, UniPrms );
		}
		else
		{ /* Rotation type */
			err = RmxSencSetMencDividedPulse( AsicHwReg, MencV, UniPrms );
		}
	}

	return( err );
}

/****************************************************************************************************/
/*																									*/
/*		Rotary type: Serial encoder frequency division output pulse calculation & ASIC setting								*//*<S02D>*/
/*				(Select position data for divided outputJL-086is used for direct pulse conversion)											*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the division ratio and division mode of the divided pulse output.										*/
/*																									*/
/*																									*/
/*			BitNo				32-BitNo 															*/
/*		|<------------------>|<---------->|		PulseNo = 2^BitNo									*/
/*		+----------------+----------------+		 													*/
/*		|	PulseData		 |000000000000|		RcvPosX : [2^32/rev]								*/
/*		+----------------+----------------+															*/
/*		|<---------------->|<------------>| 	PulseOut = RcvPosX >> DivOutSft						*/
/*			PulseOut		   DivOutSft															*/
/*																									*/
/*							   <---- i ---|															*/
/*		|0000000001XXXXXXXXXXXX10000000000|		pgrate												*/
/*		|00000000000000000001XXXXXXXXXXXX1|		ratex = pgrate >> i									*/
/*						    <---- j ------|															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*				   DivRate        2^32																*/
/*		DivOut  = --------- * ------------- = (4 * pgrate)		[4Multiplicationpulse/rev]					*/
/*				    65536      2^DivOutSft															*/
/*																									*/
/*				   4 * pgrate * 65536																*/
/*		DivRate = -------------------- = pgrate >> (14 - DivOutSft) = pgrate >> RateSft				*/
/*				   2^(32 - DivOutSft)																*/
/*																									*/
/*		RateSft = 14 - DivOutSft,	DivOutSft = 14 - RateSft										*/
/*																									*/
/*		Condition : DivRate < 65536 , (DivRate % 4) = 0												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static DIVSET_ERROR	RmxSencSetMencDividedPulseHw( ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms )
{
	LONG	i,j,x;
	LONG	BitNo;
	LONG	pulsno;
	LONG	pgrate;
	LONG	IncreSftR;   /*<S079>*/
	LONG	IncreSftX;   /*<S079>*/
	LONG	IncreLeng;   /*<S079>*/
	UCHAR 	*Model;  	 /*<S0D3>*/


	BitNo  = UniPrms->Prm->MencP.bit_dp.b.l;								/* PnF03				*/
	pgrate = UniPrms->Prm->pgrat2l;											/* Pn212(Pn213)			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Division output pulse upper limit value setting																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->EncConnect )
	{
		pulsno = (MencV->PulseNo >> 2);
	}
	else
	{
		pulsno = pndef_pgrat2l.UpperLimit;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of upper limit speed of divided output pulse frequency															*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->DivInfo.DivOvrSpd = RmxSencCalMencDivOverSpeed( (pgrate -1), UniPrms->Bprm->OvrSpd );
	if( MencV->DivInfo.DivOvrSpd == 0 )
	{
		return ( DIVSET_PRMERR_R );                        /* Return value: Parameter setting error(Rotation):A.040*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pulse number check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( MlibCheckPowerOf2( pulsno ) < 0 )					/* pulsno : Power of 2 check			*/
	{
		return ( DIVSET_DIVERR );							/* Return value: Divide pulse setting error:A.041		*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Checking PG division ratio setting range																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (pgrate > pulsno) || (pgrate < 16) )				/* pgrate : range check				*/
	{
		return ( DIVSET_DIVERR );							/* Return value: Divide pulse setting error:A.041		*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Checking PG division ratio setting conditions																*/
/*--------------------------------------------------------------------------------------------------*/
	i = MlibSrhbiton( pgrate, SRH_FROM_LSB );				/*						i<---- Search	*/
	x = ((ULONG)pgrate >> i);								/* pgrate : xxxxxxxxxxxx100000			*/
	if( (i < 0) || (x > 16383) )							/* Setting range : 1 <= x <= 16383			*/
	{														/*										*/
		return ( DIVSET_DIVERR );							/*										*/
	}														/*										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of division pulse output ratio : 11bit (for Nittoku High Speed Motor)										*/
/*--------------------------------------------------------------------------------------------------*/
	if( BitNo == 11 )
	{
		MencV->DivInfo.DivMode = FALSE;						/* Mode : Not used						*/
		MencV->DivInfo.DivPass = FALSE;						/* Pass : Divided mode					*/
		if( pgrate < pulsno )								/* Divided mode : pgrate < pulsno			*/
		{
			MencV->DivInfo.DivRate = (USHORT)(pgrate << 5);	/* Rate = pgrate << (16-(BitNo-2)-2)	*/
		}
		else
		{
			MencV->DivInfo.DivRate = (USHORT)(pulsno << 5);	/* Rate = pulsno << (16-(BitNo-2)-2)	*/
		}
		MencV->DivInfo.DivOutSft = 19;						/* OutSft = 32 - BitNo - 2				*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of division pulse output ratio : 16bit (for 13bit Encoder)									*/
/*--------------------------------------------------------------------------------------------------*/
	else if( BitNo < 16 )
	{
		if( pgrate < pulsno )								/* Divided mode : pgrate < pulsno			*/
		{
			MencV->DivInfo.DivMode = FALSE;					/* Mode : Not used						*/
			MencV->DivInfo.DivPass = FALSE;					/* Pass : Divided mode					*/
			j = 18 - BitNo;									/* j = -(14 - (32-BitNo)) = 18 - BitNo	*/
			MencV->DivInfo.DivRate = (USHORT)(pgrate << j);	/* Rate = pgrate << (16-(BitNo-2))		*/
			MencV->DivInfo.DivOutSft = (UCHAR)(32 - BitNo);	/* OutSft = 32 - BitNo					*/
		}
	/*----------------------------------------------------------------------------------------------*/
		else												/* Through mode : pgrate == pulsno		*/
		{
			MencV->DivInfo.DivMode = FALSE;					/* Mode : Not used						*/
			MencV->DivInfo.DivPass = TRUE;					/* Pass : Through mode					*/
			MencV->DivInfo.DivRate = 0;						/* Rate = 65536 = pgrate << (18-BitNo)	*/
			MencV->DivInfo.DivOutSft = (UCHAR)(32 - BitNo);	/* OutSft = 32 - BitNo					*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of division pulse output ratio : 16bitor more														*/
/*--------------------------------------------------------------------------------------------------*/
	else if( x != 1 )										/* Division mode 1 : pgrate != 2^i			*/
	{														/* Search ------------------>j			*/
		j = MlibSrhbiton( x, SRH_FROM_MSB );				/*    x : 000000...00000000001xxxx1		*/
		if( (i + j) < 13 ){ j = 13 - i;}					/* Limit j by (OutSft <= 16)			*/
		MencV->DivInfo.DivMode = FALSE;						/* Mode : Not used						*/
		MencV->DivInfo.DivPass = FALSE;						/* Pass : Divided mode					*/
#if 0 /*<S0C8>*/
		MencV->DivInfo.DivRate = (USHORT)(x << (15-j));		/* Rate = (pgrate>>i)<<(15-j)			*/
#else/*<S0C8>*/
		MencV->DivInfo.DivRate = (USHORT)((x << (15-j)) >> (BitNo - 16));
		j = BitNo - 16;										/* j = BitNo -16						*/
#endif/*<S0C8>*/
		MencV->DivInfo.DivOutSft = (UCHAR)(29 - i - j);		/* OutSft = 14 - i + 15 - j				*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( i < 14 )										/* Division mode 2 : pgrate == 2^i			*/
	{
		MencV->DivInfo.DivMode = FALSE;						/* Mode : Not used						*/
		MencV->DivInfo.DivPass = FALSE;						/* Pass : Divided mode					*/
#if 0 /*<S079>*/
		MencV->DivInfo.DivRate = (USHORT)(pgrate << 2);		/* Rate = pgrate << 2 					*/
#else/*<S079>*/
		MencV->DivInfo.DivOutSft = 16;						/* OutSft = 16							*/
		j = BitNo - 16;										/* j = BitNo -16						*/
		MencV->DivInfo.DivRate = (USHORT)((pgrate << 2)>>j);/* Rate = pgrate << (16-(BitNo-2))		*/
		if( MencV->DivInfo.DivRate == 0 )
		{
			MencV->DivInfo.DivRate = 1;
		}
#endif/*<S079>*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else													/* Through mode : pgrate == 2^i			*/
	{
		j = BitNo - 16;										/* j = BitNo -16						*//*<S0C8>*/
		MencV->DivInfo.DivMode = FALSE;						/* Mode : Not used						*/
		MencV->DivInfo.DivPass = TRUE;						/* Pass : Through mode					*/
		MencV->DivInfo.DivRate = 0;							/* Rate = 65536 = ((pgrate>>i)<<16)		*/
		MencV->DivInfo.DivOutSft = (UCHAR)(30 - i);			/* OutSft = 14 - i + 16 = 30 - i		*/
	}
#if 0/*<S079>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Division C phase output mask controlFlag															*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->DivInfo.DivPcmaskCtrl = (MencV->DivInfo.DivOutSft < 16)? TRUE : FALSE;
/*--------------------------------------------------------------------------------------------------*/
/*		Division C phase output mask range setting																*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->DivInfo.DivPcmaskCtrl )
	{
		MencV->DivInfo.DivOutCmskChk = (ULONG)0x80000000 >> (BitNo - 16);
	}
#endif/*<S079>*/
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC frequency division function setting																		*/
/*--------------------------------------------------------------------------------------------------*/
/*	DINCSET(0x0048):Divide function																		*/
/*--------------------------------------------------------------------------------------------------*/
/*	VULONG	AREG_DINCSET;	     0x0048 Division function setting												*/
								/*------------------------------------------------------------------*/
								/*  bit.0  : DA,DBThrough mode setting									*/
								/*            0: Divided output (initial value)									*/
								/*  		  1:PA PBThrough output									*/
								/*------------------------------------------------------------------*/
								/*  bit.1  : DCThrough mode setting										*/
								/*            0:PCThrough (initial value)									*/
								/*  		  1:PA&PB&PC											*/
								/*------------------------------------------------------------------*/
								/*  bit.2  : DBReverse mode setting										*/
								/*            0:DBThrough output (initial value)								*/
								/*  		  1:DBInversion output											*/
								/*------------------------------------------------------------------*/
								/*  bit.3  : Division mode selection											*/
								/*            0:16bitDDAMode(Initial value)Division ratio range:1/65536,65535/65536*/
								/*  		  1:65005												*/
								/*------------------------------------------------------------------*/
								/*  bit.4  : New Incre decoderCPhase passage 1:Edge passage					*/
								/*            0: New increment decoderCPhase passage (initial value)				*/
								/*  		  1: Edge passing											*/
								/*------------------------------------------------------------------*/
								/*  bit.5  : CLSETBit											*/
								/*            0: Origin resynchronization not possible (initial value)							*/
								/*  		  1: Origin resynchronization possible										*/
								/*------------------------------------------------------------------*/
								/*  bit.10  : Divide always clear setting									*/
								/*            0: Always clear disable (initial value)					*/
								/*  		  1: Always clear enable								*/
								/*------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
	x = (MencV->DivInfo.DivMode << 4) | MencV->DivInfo.DivPass | 0x0400;
	AsicHwReg->AREG_DINCSET	= (USHORT)x;							/* Division function setting					*/
	AsicHwReg->AREG_DIVCSEV	= (USHORT)(MencV->DivInfo.DivRate);		/* Division ratio setting					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Pulse conversion origin correction setting																		*/
/*--------------------------------------------------------------------------------------------------*/
/*	VULONG	DCTRIM;	    0x0190 Division output origin correction setting(Bit0-23)										*/
/*--------------------------------------------------------------------------------------------------*/
								/*  bit.1-0  : ”0 ”												*/
								/*------------------------------------------------------------------*/
								/*  bit.23-2 : DVCP23-2												*/
								/*  		  Division output origin correction setting bit23-2						*/
/*--------------------------------------------------------------------------------------------------*/
	if( BitNo == 13 )
	{
		AsicHwReg->AREG_DCTRIM	= (ULONG)((MencV->DivInfo.DivRate>>3)-1);
	}
	else
	{
		AsicHwReg->AREG_DCTRIM	= (ULONG)( (MencV->DivInfo.DivRate<<j) - 1 );
	}

/*--------------------------------------------------------------------------------------------------*/
/*  DINCSET bit.2 DBReverse mode	(0:DBThrough output;    1:DBInversion output)								*/
/*--------------------------------------------------------------------------------------------------*/
	if( UniPrms->Bprm->RvsDir )
	{
		AsicHwReg->AREG_DINCSET |= BIT2;					/* DBReverse mode setting						*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pulse conversion origin correction setting																		*/
/*--------------------------------------------------------------------------------------------------*/
/*	VULONG	AREG_ZDR;	     0x0150 Pulse conversion origin correction setting											*/
/*--------------------------------------------------------------------------------------------------*/
								/*  bit.3-0  : Incretion correction data Amount of right shifts						*/
								/*     ZDRSFT  Serial reception dataD10-D11of16bitinsideLSB		*/
								/*------------------------------------------------------------------*/
								/*  bit.7-4  : Incretion correction data Left shift amount						*/
								/*     ZDLSFT Increment correction data set value for lower data0)*/
								/*------------------------------------------------------------------*/
								/*  bit.11-8 : Ink correction data length									*/
								/*     ZDLNG   Data length (number of bits-1) to set						*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AbsoEncoder == FALSE )/*<S0D3>*/
	{
		if( BitNo > 20 )				/*<S079>*/
		{
			IncreSftX = 7;
//			IncreLeng = 8;
		 	IncreLeng = 95 - MencV->DivInfo.IncreBit_dp; /*<S161>*/
		}
		else if( BitNo == 13 )
		{
			IncreSftX = 0;
			IncreLeng = 10;
		}
		else if( BitNo == 17)   /*<S0D3>*/
		{
			IncreSftX = 0;
			Model = &(MencV->EncInfo.Model[0]);
			if( Model[3] == 'J' )
			{/* UTSJ□-B17 In the case of */
				IncreLeng = 9;
			}
			else if((Model[9] == 'C') && (Model[10] == 'C'))
			{/* UTSIH-B17CC(ΣⅡ)*/
				IncreLeng = 10;
			}
			else
			{
				IncreLeng = 8;
			}
		}
		else
		{
			IncreSftX = 0;
			IncreLeng = 8;
		}
		if( MencV->Sigma3Encoder )
		{
			IncreSftX =  IncreSftX + 4;
		}
		IncreSftR = BitNo-(IncreLeng+1); /*<S0D3>*/
	
		AsicHwReg->AREG_ZDR   =	  ( IncreSftX << 0 )			/* Incretion correction data Amount of right shifts		*/
								+ ( IncreSftR << 4 )			/* Incretion correction data Left shift amount		*/
								+ ( IncreLeng << 8 );			/* Ink correction data length				*/
	/*--------------------------------------------------------------------------------------------------*/
	/*	VULONG	AREG_ZENBR;	     0x0154 Pulse conversion origin correction setting											*/
	/*--------------------------------------------------------------------------------------------------*/
									/*------------------------------------------------------------------*/
									/*  bit.5-0  : Setting the origin passing bit position								*/
									/*   Receive dataD7-D11of40bitDetermine the bit position indicating passing the origin in the middle) */
									/*------------------------------------------------------------------*/
									/*  bit.6  : Ink correction data constant update mode setting					*/
									/*            0: Set only when passing the origin for the first time							*/
									/*  		  1: Updated every time the origin is passed								*/
									/*------------------------------------------------------------------*/
									/*  bit.7  : Increment correction data set completion bit clear				*/
									/*            0: Set not completed											*/
									/*  		  1: Set completed											*/
									/*------------------------------------------------------------------*/
									/*  bit.8  : Increment correction data set enable1					*/
									/*      (Home point not passed flag=0)          */
									/*            0: Invalid												*/
									/*  		  1: Enable												*/
									/*------------------------------------------------------------------*/
									/*  bit.9  : Increment correction data set enable2					*/
									/*      (Set the ink correction data based on the change from the previous value of the ink correction data.)*/
									/*            0: Invalid												*/
									/*  		  1: Enable												*/
									/*------------------------------------------------------------------*/
									/*  bit.10  : Increment correction data set enable3					*/
									/*      (Home point passing bitZ=1)			*/
									/*            0: Invalid												*/
									/*  		  1: Enable												*/
	/*--------------------------------------------------------------------------------------------------*/
	//	AsicHwReg->AREG_ZENBR = 0x0400;
		AsicHwReg->AREG_ZENBR = 0x0200;/*<S0D3>*/
		AsicHwReg->AREG_WDT1SET &= ~BIT8;				/* Pulse conversionCPhase maskOFFSetting			*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return ( DIVSET_SUCCESS );
}


/****************************************************************************************************/
/*																									*/
/*		Rotary type: Serial encoder frequency division output pulse calculation & ASIC setting								*//*<S02D>*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the division ratio and division mode of the divided pulse output.										*/
/*																									*/
/*																									*/
/*			BitNo				32-BitNo 															*/
/*		|<------------------>|<---------->|		PulseNo = 2^BitNo									*/
/*		+----------------+----------------+		 													*/
/*		|	PulseData		 |000000000000|		RcvPosX : [2^32/rev]								*/
/*		+----------------+----------------+															*/
/*		|<---------------->|<------------>| 	PulseOut = RcvPosX >> DivOutSft						*/
/*			PulseOut		   DivOutSft															*/
/*																									*/
/*							   <---- i ---|															*/
/*		|0000000001XXXXXXXXXXXX10000000000|		pgrate												*/
/*		|00000000000000000001XXXXXXXXXXXX1|		ratex = pgrate >> i									*/
/*						    <---- j ------|															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*				   DivRate        2^32																*/
/*		DivOut  = --------- * ------------- = (4 * pgrate)		[4Multiplicationpulse/rev]					*/
/*				    65536      2^DivOutSft															*/
/*																									*/
/*				   4 * pgrate * 65536																*/
/*		DivRate = -------------------- = pgrate >> (14 - DivOutSft) = pgrate >> RateSft				*/
/*				   2^(32 - DivOutSft)																*/
/*																									*/
/*		RateSft = 14 - DivOutSft,	DivOutSft = 14 - RateSft										*/
/*																									*/
/*		Condition : DivRate < 65536 , (DivRate % 4) = 0												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static DIVSET_ERROR	RmxSencSetMencDividedPulse( ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms )
{
	LONG	i,j,x;
	LONG	BitNo;
	LONG	pulsno;
	LONG	pgrate;

	BitNo  = UniPrms->Prm->MencP.bit_dp.b.l;								/* PnF03				*/
	pgrate = UniPrms->Prm->pgrat2l;											/* Pn212(Pn213)			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Division output pulse upper limit value setting																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->EncConnect )
	{
		pulsno = (MencV->PulseNo >> 2);
	}
	else
	{
		pulsno = pndef_pgrat2l.UpperLimit;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of upper limit speed of divided output pulse frequency															*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->DivInfo.DivOvrSpd = RmxSencCalMencDivOverSpeed( (pgrate -1), UniPrms->Bprm->OvrSpd );
	if( MencV->DivInfo.DivOvrSpd == 0 )
	{
		return ( DIVSET_PRMERR_R );                        /* Return value: Parameter setting error(Rotation):A.040*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pulse number check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( MlibCheckPowerOf2( pulsno ) < 0 )					/* pulsno : Power of 2 check			*/
	{
		return ( DIVSET_DIVERR );							/* Return value: Divide pulse setting error:A.041		*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Checking PG division ratio setting range																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (pgrate > pulsno) || (pgrate < 16) )				/* pgrate : range check				*/
	{
		return ( DIVSET_DIVERR );							/* Return value: Divide pulse setting error:A.041		*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Checking PG division ratio setting conditions																*/
/*--------------------------------------------------------------------------------------------------*/
	i = MlibSrhbiton( pgrate, SRH_FROM_LSB );				/*						i<---- Search	*/
	x = ((ULONG)pgrate >> i);								/* pgrate : xxxxxxxxxxxx100000			*/
	if( (i < 0) || (x > 16383) )							/* Setting range : 1 <= x <= 16383			*/
	{														/*										*/
		return ( DIVSET_DIVERR );							/*										*/
	}														/*										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of division pulse output ratio : 11bit (for Nittoku High Speed Motor)										*/
/*--------------------------------------------------------------------------------------------------*/
	if( BitNo == 11 )
	{
		MencV->DivInfo.DivMode = FALSE;						/* Mode : Not used						*/
		MencV->DivInfo.DivPass = FALSE;						/* Pass : Divided mode					*/
		if( pgrate < pulsno )								/* Divided mode : pgrate < pulsno			*/
		{
			MencV->DivInfo.DivRate = (USHORT)(pgrate << 5);	/* Rate = pgrate << (16-(BitNo-2)-2)	*/
		}
		else
		{
			MencV->DivInfo.DivRate = (USHORT)(pulsno << 5);	/* Rate = pulsno << (16-(BitNo-2)-2)	*/
		}
		MencV->DivInfo.DivOutSft = 19;						/* OutSft = 32 - BitNo - 2				*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of division pulse output ratio : 16bit (for 13bit Encoder)									*/
/*--------------------------------------------------------------------------------------------------*/
	else if( BitNo < 16 )
	{
		if( pgrate < pulsno )								/* Divided mode : pgrate < pulsno			*/
		{
			MencV->DivInfo.DivMode = FALSE;					/* Mode : Not used						*/
			MencV->DivInfo.DivPass = FALSE;					/* Pass : Divided mode					*/
			j = 18 - BitNo;									/* j = -(14 - (32-BitNo)) = 18 - BitNo	*/
			MencV->DivInfo.DivRate = (USHORT)(pgrate << j);	/* Rate = pgrate << (16-(BitNo-2))		*/
			MencV->DivInfo.DivOutSft = (UCHAR)(32 - BitNo);	/* OutSft = 32 - BitNo					*/
		}
	/*----------------------------------------------------------------------------------------------*/
		else												/* Through mode : pgrate == pulsno		*/
		{
			MencV->DivInfo.DivMode = FALSE;					/* Mode : Not used						*/
			MencV->DivInfo.DivPass = TRUE;					/* Pass : Through mode					*/
			MencV->DivInfo.DivRate = 0;						/* Rate = 65536 = pgrate << (18-BitNo)	*/
			MencV->DivInfo.DivOutSft = (UCHAR)(32 - BitNo);	/* OutSft = 32 - BitNo					*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of division pulse output ratio : 16bitor more														*/
/*--------------------------------------------------------------------------------------------------*/
	else if( x != 1 )										/* Division mode 1 : pgrate != 2^i			*/
	{														/* Search ------------------>j			*/
		j = MlibSrhbiton( x, SRH_FROM_MSB );				/*    x : 000000...00000000001xxxx1		*/
		if( (i + j) < 13 ){ j = 13 - i;}					/* Limit j by (OutSft <= 16)			*/
		MencV->DivInfo.DivMode = FALSE;						/* Mode : Not used						*/
		MencV->DivInfo.DivPass = FALSE;						/* Pass : Divided mode					*/
		MencV->DivInfo.DivRate = (USHORT)(x << (15-j));		/* Rate = (pgrate>>i)<<(15-j)			*/
		MencV->DivInfo.DivOutSft = (UCHAR)(29 - i - j);		/* OutSft = 14 - i + 15 - j				*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( i < 14 )										/* Division mode 2 : pgrate == 2^i			*/
	{
		MencV->DivInfo.DivMode = FALSE;						/* Mode : Not used						*/
		MencV->DivInfo.DivPass = FALSE;						/* Pass : Divided mode					*/
		MencV->DivInfo.DivRate = (USHORT)(pgrate << 2);		/* Rate = pgrate << 2 					*/
		MencV->DivInfo.DivOutSft = 16;						/* OutSft = 16							*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else													/* Through mode : pgrate == 2^i			*/
	{
		MencV->DivInfo.DivMode = FALSE;						/* Mode : Not used						*/
		MencV->DivInfo.DivPass = TRUE;						/* Pass : Through mode					*/
		MencV->DivInfo.DivRate = 0;							/* Rate = 65536 = ((pgrate>>i)<<16)		*/
		MencV->DivInfo.DivOutSft = (UCHAR)(30 - i);			/* OutSft = 14 - i + 16 = 30 - i		*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Division C phase output mask controlFlag															*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->DivInfo.DivPcmaskCtrl = (MencV->DivInfo.DivOutSft < 16)? TRUE : FALSE;
/*--------------------------------------------------------------------------------------------------*/
/*		Division C phase output mask range setting																*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->DivInfo.DivPcmaskCtrl )
	{
		MencV->DivInfo.DivOutCmskChk = (ULONG)0x80000000 >> (BitNo - 16);
	}
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC frequency division function setting																		*/
/*--------------------------------------------------------------------------------------------------*/
	x = (MencV->DivInfo.DivMode << 4) | MencV->DivInfo.DivPass | 0x0400;
	AsicHwReg->AREG_DINCSET	= (USHORT)x;						/* Division function setting						*/
	AsicHwReg->AREG_DIVCSEV	= (USHORT)(MencV->DivInfo.DivRate); /* Division ratio setting						*/
	/*----------------------------------------------------------------------------------------------*/
	/* Division output origin correction */
	if( BitNo <= 13 )
	{
		AsicHwReg->AREG_DCTRIM	= (ULONG)((MencV->DivInfo.DivRate>>3)-1);
	}
	else
	{
		AsicHwReg->AREG_DCTRIM	= (ULONG)( MencV->DivInfo.DivRate - 1 );
	}
	/*----------------------------------------------------------------------------------------------*/
	if( UniPrms->Bprm->RvsDir )
	{
		AsicHwReg->AREG_DINCSET |= BIT2;					/* DBReverse mode setting						*/
	}
	return ( DIVSET_SUCCESS );
}

/****************************************************************************************************/
/*																									*/
/*		Rotation type: Adjust serial encoder frequency division output pulse setting value										*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : In the divided output pulse setting, adjust the setting value so that it becomes a settable value.					*/
/*																									*/
/****************************************************************************************************/
//<S149>#define	BITNOCHK	14							/* 2^14 = 16384									*/
//<S149>#define	BITNOCHK		18							/* 2^18 = 262144								*//*<S02D>*/
/*--------------------------------------------------------------------------------------------------*/
#if 0			/* <S149> */
PRM_RSLT	RmxSencAdjustMencDivPulse( PRMDATA *Prm, MENCV *MencV, PRM_ACCCMD Cmd, LONG Data )
{
	ULONG	MaskX;
	ULONG	DataX;
	ULONG	BitNoX;
	ULONG	PulseNoX;
	ULONG	DataMinX = pndef_pgrat2l.LowerLimit;

/*--------------------------------------------------------------------------------------------------*/
/*		Division output pulse upper limit value setting																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->EncConnect )
	{
		PulseNoX = (MencV->PulseNo >> 2);
	}
	else
	{
		PulseNoX = pndef_pgrat2l.UpperLimit;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter calculation process(Parameter check)														*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{
		return( PRM_RSLT_SUCCESS );
	}
	else
	{
		DataX = Prm->pgrat2l;
		BitNoX = MlibSrhbiton( DataX, SRH_FROM_MSB );
		if( DataX > PulseNoX  ){ return( PRM_RSLT_LIMIT_ERR );}
		if( DataX < DataMinX  ){ return( PRM_RSLT_LIMIT_ERR );}
		if( BitNoX < BITNOCHK ){ return( PRM_RSLT_SUCCESS  );}
		MaskX = ( 0xFFFFFFFF << (BitNoX + 1 - BITNOCHK) );
		if( (DataX & MaskX) == DataX ){ return( PRM_RSLT_SUCCESS );}
		return( PRM_RSLT_CALC_ERR );
	}
}
#endif
/****************************************************************************************************/
/*																									*/
/*		Rotation type: Adjust serial encoder frequency division output pulse setting value (Current, unused) <S149>					*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : In the divided output pulse setting, adjust the setting value so that it becomes a settable value.					*/
/*																									*/
/*	For Route : For setting value adjustment when dividing pulse setting value in operator											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#define	BITNOCHK	14							/* 2^14 = 16384										*/
/*--------------------------------------------------------------------------------------------------*/
PRM_RSLT	RmxSencAdjustMencDivPulse( PRMDATA *Prm, MENCV *MencV, PRM_ACCCMD Cmd, LONG Data )
{
ULONG	MaskX;
ULONG	DataX;
ULONG	BitNoX;
ULONG	PulseNoX;
ULONG	DataMinX = pndef_pgrat2l.LowerLimit;

/*--------------------------------------------------------------------------------------------------*/
/*		Division output pulse upper limit value setting																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( MencV->EncConnect )
		{
			PulseNoX = (MencV->PulseNo >> 2);
		}
		else
		{
			PulseNoX = pndef_pgrat2l.UpperLimit;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter calculation process(Parameter check)														*/
/*--------------------------------------------------------------------------------------------------*/
		if( ( Cmd == PRM_ACCCMD_WRITE) || ( Cmd == PRM_ACCCMD_EEPWRITE) 
			|| ( Cmd == PRM_ACCCMD_RECALC ) )
		{
			if( MencV->AxisMotType == MOTTYPE_LINEAR )
			{
				return( TRUE );
			}
			else
			{
				DataX = Prm->pgrat2l;
				BitNoX = MlibSrhbiton( DataX, SRH_FROM_MSB );
				if( DataX > PulseNoX  ){ return( PRM_RSLT_LIMIT_ERR );}
				if( DataX < DataMinX  ){ return( PRM_RSLT_LIMIT_ERR );}
				if( BitNoX < BITNOCHK ){ return( PRM_RSLT_SUCCESS );}
				MaskX = ( 0xFFFFFFFF << (BitNoX + 1 - BITNOCHK) );
				if( (DataX & MaskX) == DataX ){ return( PRM_RSLT_SUCCESS );}
				return( PRM_RSLT_CALC_ERR );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Inc/DecData adjustment processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		DataX = Data;
		BitNoX = MlibSrhbiton( DataX, SRH_FROM_MSB );
/*--------------------------------------------------------------------------------------------------*/
/*		Setting range check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( DataX >= PulseNoX )
		{
			DataX = PulseNoX;									/* Upper limit with pulse number			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Data >= 2^BITNOCHK , adjust the setting value												*/
/*--------------------------------------------------------------------------------------------------*/
		else if( BitNoX >= BITNOCHK )
		{
			MaskX = ( 0xFFFFFFFF << (BitNoX + 1 - BITNOCHK) );
			if( Cmd == PRM_ACCCMD_ADJINCDATA )					/* Adjust Inc. Data					*/
			{
				DataX = (DataX + ~MaskX) & MaskX;				/* Round Up							*/
			}
			else if( Cmd == PRM_ACCCMD_ADJDECDATA )				/* Adjust Dec. Data					*/
			{
				DataX = DataX & MaskX;							/* Round Down						*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Data < 2^BITNOCHK All settings are configurable, so do nothing.								*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Upper/Lower Limit																			*/
/*--------------------------------------------------------------------------------------------------*/
		DataX = MlibLIMITUL( DataX, PulseNoX, DataMinX );
		return( DataX );
}



/****************************************************************************************************/
/*																									*/
/*		Linear motor : Calculation of divided output pulse & ASIC setting								<S079>		*/
/*				(Select position data for divided outputJL-086is used for direct pulse conversion)											*/
/****************************************************************************************************/
static DIVSET_ERROR LmxSencSetMencDividedPulseHw( ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms )
{
	LONG			x;
	LONG			BitNo;
	DIVSET_ERROR	err;

	BitNo  = UniPrms->Prm->MencP.bit_dp.b.l;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of upper limit speed of divided output pulse frequency															*/
/*--------------------------------------------------------------------------------------------------*/
	err = LmxSencCalMencDivOverSpeed( MencV, UniPrms );

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of frequency division pulse output gain																	*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->DivInfo.DivMode = FALSE;
	MencV->DivInfo.DivPass = TRUE;
	MencV->DivInfo.DivRate = 0;
	MencV->DivInfo.DivOutGain = MlibScalKxgain( UniPrms->Prm->scalepgrat, 1, (1 << BitNo), NULL, 24 );
	MencV->DivInfo.DivOutCmskChk = MlibScalKxkxks( 0x1000, 1, MencV->DivInfo.DivOutGain, NULL, -30 );
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC frequency division function setting																		*/
/*--------------------------------------------------------------------------------------------------*/
	x = (MencV->DivInfo.DivMode << 4) | MencV->DivInfo.DivPass | 0x0400;
	AsicHwReg->AREG_DINCSET = (USHORT)x;						/* Division function setting						*/
	AsicHwReg->AREG_DIVCSEV = (USHORT)MencV->DivInfo.DivRate;	/* Division ratio setting						*/
	AsicHwReg->AREG_DCTRIMH = 0x00;
	AsicHwReg->AREG_DCTRIM  = (USHORT)(MencV->DivInfo.DivRate - 1);
	if( UniPrms->Bprm->RvsDir )
	{
		AsicHwReg->AREG_DINCSET |= BIT2;						/* DBReverse mode setting					*/
	}
	return (err);
}

/****************************************************************************************************/
/*																									*/
/*		Linear motor : Calculation of divided output pulse & ASIC setting											*/
/*																									*/
/****************************************************************************************************/
static DIVSET_ERROR LmxSencSetMencDividedPulse( ASIC_HW_IF_STR *AsicHwReg, MENCV *MencV, UNI_PRM_STR *UniPrms )
{
	LONG			x;
	LONG			BitNo;
	DIVSET_ERROR	err;

	BitNo  = UniPrms->Prm->MencP.bit_dp.b.l;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of upper limit speed of divided output pulse frequency															*/
/*--------------------------------------------------------------------------------------------------*/
	err = LmxSencCalMencDivOverSpeed( MencV, UniPrms );

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of frequency division pulse output gain																	*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /*<S02D>*/
	MencV->DivMode = FALSE;
	MencV->DivPass = TRUE;
	MencV->DivRate = 0;
	MencV->DivOutGain = MlibScalKxgain( UniPrms->Prm->scalepgrat, 1, (1 << BitNo), NULL, 24 );
	MencV->DivOutCmskChk = MlibScalKxkxks( 0x1000, 1, MencV->DivOutGain, NULL, -30 );
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC frequency division function setting																		*/
/*--------------------------------------------------------------------------------------------------*/
	x = (MencV->DivMode << 4) | MencV->DivPass | 0x0400;
	AsicHwReg->AREG_DINCSET = (USHORT)x;							/* Division function setting						*/
	AsicHwReg->AREG_DIVCSEV = (USHORT)MencV->DivRate;			/* Division ratio setting						*/
	AsicHwReg->AREG_DCTRIMH = 0x00;
	AsicHwReg->AREG_DCTRIM  = (USHORT)(MencV->DivRate - 1);
	if( UniPrms->Bprm->RvsDir )
	{
		AsicHwReg->AREG_DINCSET |= BIT2;							/* DBReverse mode setting					*/
	}
#else/*<S02D>*/
	MencV->DivInfo.DivMode = FALSE;
	MencV->DivInfo.DivPass = TRUE;
	MencV->DivInfo.DivRate = 0;
	MencV->DivInfo.DivOutGain = MlibScalKxgain( UniPrms->Prm->scalepgrat, 1, (1 << BitNo), NULL, 24 );
	MencV->DivInfo.DivOutCmskChk = MlibScalKxkxks( 0x1000, 1, MencV->DivInfo.DivOutGain, NULL, -30 );
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC frequency division function setting																		*/
/*--------------------------------------------------------------------------------------------------*/
	x = (MencV->DivInfo.DivMode << 4) | MencV->DivInfo.DivPass | 0x0400;
	AsicHwReg->AREG_DINCSET = (USHORT)x;						/* Division function setting						*/
	AsicHwReg->AREG_DIVCSEV = (USHORT)MencV->DivInfo.DivRate;	/* Division ratio setting						*/
	AsicHwReg->AREG_DCTRIMH = 0x00;
	AsicHwReg->AREG_DCTRIM  = (USHORT)(MencV->DivInfo.DivRate - 1);
	if( UniPrms->Bprm->RvsDir )
	{
		AsicHwReg->AREG_DINCSET |= BIT2;						/* DBReverse mode setting					*/
	}

#endif/*<S02D>*/
	return (err);
}
//#endif /* MENCVIs it necessary to change related structures? *//*<S02D>Delete*/
/****************************************************************************************************/
/*																									*/
/*		Rotary type: Calculation of divided output pulse frequency upper limit speed													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the rotation speed at which the frequency division output pulse has the maximum frequency.									*/
/*																									*/
/*			Pulse frequency upper limit(SVCDEF_DIVOUT_MAXFREQ) :												*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*			MaxFreq [pps] = pgrate [pulse/rev] * DivOvrSpd [rev/s]									*/
/*																									*/
/*									       MaxFreq * 2^24											*/
/*			DivOvrSpd[2^24/OvrSpd] = ----------------------------	 								*/
/*									  pgrate  * (OvrSpd / 2*PAI) 									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	LONG	RmxSencCalMencDivOverSpeed( LONG pgrate, KSGAIN OvrSpd )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG	kx, sx;
#endif /* FLOAT_USE */
	LONG	MaxFreq;											/* [pulse/sec]						*/
	LONG	i;
	LONG	pulsno;
	LONG	rc;

	MaxFreq = SVCDEF_DIVOUT_MAXFREQ;
	pulsno = 1;
/*--------------------------------------------------------------------------------------------------*/
/*		Division upper limit speed reference value calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	for ( i = 0; pgrate > 0; i++ )
	{
		pgrate = pgrate / 2;
		pulsno = pulsno * 2;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of upper limit speed of divided output pulse frequency															*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = (float)MaxFreq * (float)0x01000000 / (float)pulsno;
	fw = fw / OvrSpd;
	fw = fw * 62832.0F / 10000.0F;
	if( fw > (float)0x01000000 )
	{
		rc = 0x01000000;
	}
	else
	{
		rc = (LONG)fw;
	}
#else
	kx = MlibScalKxgain( MaxFreq, 0x01000000, pulsno, &sx,   0 );
	kx = MlibPcalKxkxks( kx,      1,		  OvrSpd, &sx,   0 );
	rc = MlibPcalKxgain( kx,      62832,      10000,  &sx, -24 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
	return( rc );
}

/****************************************************************************************************/
/*																									*/
/*		Linear motor : Calculation of upper limit speed of divided output pulse frequency											*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the rotation speed at which the frequency division output pulse has the maximum frequency.									*/
/*																									*/
/*			Pulse frequency upper limit(SVCDEF_DIVOUT_MAXFREQ) : 												*/
/*																									*/
/*			※ SGDS Linear Source  DIVPLSMAXFRQ = 1802240 [pulse/sec]							*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*			MaxFreq [pps] = pgrate [pulse/m] * DivOvrSpd [m/s]										*/
/*																									*/
/*									       MaxFreq * 2^24											*/
/*			DivOvrSpd[2^24/OvrSpd] = ----------------------------	 								*/
/*									      pgrate  * OvrSpd 											*/
/*																									*/
/*								 scalepgrat * 100000000												*/
/*			pgrate[pulse/m] = ---------------------------- 											*/
/*								     ScalePitch * 4													*/
/*																									*/
/*								 MaxFreq * 1000														*/
/*			MaxDivSpd[mm/s] = --------------------	 												*/
/*								     pgrate 														*/
/*																									*/
/*										 MaxFreq * 2^BitNo											*/
/*			MaxDivRat[pulse/spitch] = -----------------------										*/
/*										    MaxPspd / 4												*/
/*																									*/
/*			OvrSpd     : Maximum speed [rad/s], [m/s]													*/
/*			ScalePitch : Scale pitch [0.01um]													*/
/*			scalepgrat : Encoder output resolution [pulse/spitch]										*/
/*			MaxPspd    : Motor maximum speed [pulse/sec]													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static DIVSET_ERROR LmxSencCalMencDivOverSpeed( MENCV *MencV, UNI_PRM_STR *UniPrms )
{

#if (FLOAT_USE==TRUE)
	float	fw;
#else	
	LONG	kx, sx;
#endif /* FLOAT_USE */

	KSGAIN			pgrate;
	KSGAIN			MaxFreq; /* [pulse/sec] */
	LONG			MaxSpd;  /* [r/min], [mm/s] */
	LONG			BitNo;
	DIVSET_ERROR	err;

	err = DIVSET_SUCCESS;

	MaxFreq = SVCDEF_DIVOUT_MAXFREQ;
	MaxSpd  = UniPrms->Bprm->MaxVel * 100;
	BitNo   = UniPrms->Prm->MencP.bit_dp.b.l;

/*--------------------------------------------------------------------------------------------------*/
/*		PG division ratio [pulse/m]																		*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = (float)UniPrms->Prm->scalepgrat * 250000000.0f / (float)MencV->NormScalePitch;
	pgrate = fw * (float)MencV->NormScaleExp;
#else
	kx     = MlibScalKxgain( UniPrms->Prm->scalepgrat, 250000000, MencV->NormScalePitch, &sx, 0 );
	pgrate = MlibPcalKxgain( kx, MencV->NormScaleExp, 1, &sx, -1 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Upper limit speed of divided output pulse frequency [2^24/OvrSpd] 											*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = (float)MaxFreq * (float)0x01000000 / pgrate;
	fw = fw / (float)UniPrms->Bprm->OvrSpd;
	if( fw > (float)0x01000000 )
	{
		MencV->DivInfo.DivOvrSpd = 0x01000000;
	}
	else
	{
		MencV->DivInfo.DivOvrSpd = (LONG)fw;
	}
#else
	kx = MlibScalKxkxks( MaxFreq, 0x01000000, pgrate, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 1, UniPrms->Bprm->OvrSpd, &sx, -24 );
	if( kx > 0x01000000 )
	{
		MencV->DivInfo.DivOvrSpd = 0x01000000;
	}
	else
	{
		MencV->DivInfo.DivOvrSpd = kx;
	}
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Maximum speed at upper limit of divided output pulse frequency [mm/s] 											*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /* 2010.04.13 Y.Oka MlibGAINRD,float */

	fw = (float)MaxFreq * 1000.0f / pgrate;
	if( fw > (float)MaxSpd )
	{
		MencV->MaxDivSpd = MaxSpd;
	}
	else
	{
		MencV->MaxDivSpd = (LONG)fw;
	}
#endif
#if (FLOAT_USE==TRUE)
	fw = (float)MaxFreq * 1000.0f / pgrate;
	if( fw > (float)MaxSpd )
	{
		MencV->DivInfo.MaxDivSpd = MaxSpd;
	}
	else
	{
		MencV->DivInfo.MaxDivSpd = (LONG)fw;
	}
#else
	kx = MlibScalKxkxks( MaxFreq, 1000, pgrate, NULL, 24 );
	kx = MlibGAINRD( kx );
	if( kx > MaxSpd )
	{
		MencV->DivInfo.MaxDivSpd = MaxSpd;
	}
	else
	{
		MencV->DivInfo.MaxDivSpd = kx;
	}
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Division ratio setting upper limit at maximum speed [pulse/spitch] 											*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /* 2010.04.13 Y.Oka MlibGAINRD,float */
	fw = (float)MaxFreq * (float)(4 << BitNo) / Bprm->MaxPspd;
	if( fw > (float)(1<<BitNo) )
	{
		MencV->MaxDivRat = 1 << BitNo;
	}
	else
	{
		MencV->MaxDivRat = (LONG)fw;
	}
#endif
#if (FLOAT_USE==TRUE)
	fw = (float)MaxFreq * (float)(4 << BitNo) / UniPrms->Bprm->MaxPspd;
	if( fw > (float)(1<<BitNo) )
	{
		MencV->DivInfo.MaxDivRat = 1 << BitNo;
	}
	else
	{
		MencV->DivInfo.MaxDivRat = (LONG)fw;
	}
#else
	kx = MlibScalKxkxks( MaxFreq, (4 << BitNo), UniPrms->Bprm->MaxPspd, NULL, 24 );
	kx = MlibGAINRD( kx );
	if( kx > (1<<BitNo) )
	{
		MencV->DivInfo.MaxDivRat = 1 << BitNo;
	}
	else
	{
		MencV->DivInfo.MaxDivRat = kx;
	}
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Division pulse output setting error																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( (MencV->DivInfo.MaxDivRat < (LONG)UniPrms->Prm->scalepgrat) || (MencV->DivInfo.MaxDivSpd < MaxSpd) )
	{
		err = DIVSET_DIVERR;							/* A.041 : Division pulse output setting error	*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter setting error																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->DivInfo.DivOvrSpd == 0 )
	{
		err = DIVSET_PRMERR_L;							/* A.040 : Setting parameter error	*/
	}
	return (err);
}






/****************************************************************************************************/
/*																									*/
/*		Rotary type: Encoder initial parameter calculation														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Servo basic parameter calculation(BprmCalc.c)After the parameters necessary for encoder IF processing			*/
/*			Perform calculation.																			*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static void	SencInitParamCalcForRotary( MENCV *MencV, MENCPRM *MencP, MENC_REF_PRM *MencIPrm )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG	sx;
#endif
	LONG	kx;
	UCHAR	BitNo;
	UCHAR	Dpoint;

	BitNo  = MencP->bit_dp.b.l;
	Dpoint = MencP->bit_dp.b.h;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of parameters for phase origin calculation																*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MorgSft      = 100 - Dpoint;
	MencV->MorgPassSft  =  96 - Dpoint;
	if( MencV->Sigma3Encoder )
	{
		MencV->IncreCompSft = 4;
	}
	else
	{
		MencV->IncreCompSft = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of positional calculation parameters																	*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MposSftR = 32 - BitNo;
	MencV->MposSftX = MencV->MposSftR - (80 - Dpoint);
	MencV->MposSign = (MencV->RevAsmMotor == TRUE)?  -1 : 1;
	MencV->MultiturnSft = MencP->bit_dp.b.h - 64;

/*--------------------------------------------------------------------------------------------------*/
/*		C phase less flag setting																			*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->CphaseLess = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Motor magnetic pole phase calculation coefficient																*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		MotPhase [65536/360] = (PoleNo/2) * (MotPosX [2^32/Rev] >> 16)								*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->Kmotphase = (MencP->oslv_pol.b.h >> 1);

/*--------------------------------------------------------------------------------------------------*/
/*		Acceleration abnormality detection level calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					  MaxTrq * 2^32	*  Ts  *  Ts			MaxTrq * Ts * Ts						*/
/*		AccErrLvl = ------------------------------ * Cx	 = ------------------- * Cx			 		*/
/*					   Jmot * 2*PAI	* 10^9 * 10^9			Jmot * 1462918079						*/
/*																									*/
/*		MaxTrq [Nm],[N]					: Maximum torque												*/
/*		Jmot   [kgm^2],[kg]				: Motor inertia											*/
/*		Kfbpls [rad/pulse],[m/pulse]	: FB pulse conversion coefficient										*/
/*		Ts     [ns]	   					: Calculation cycle													*/
/*		Cx     [-]	   					: Safety factor (=400.000) 										*/
/*																									*/
/*		※Safety factor is rotation typeSGDSabout400.													*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = (float)KPI_SACYCLENS * (float)KPI_SACYCLENS / 1462918.0F;
	fw = fw * MencIPrm->MaxTrq / MencIPrm->Jmot;
	fw = fw * 400000.0F / 1000000.0F;
#else	
	kx = MlibScalKxgain( KPI_SACYCLENS, KPI_SACYCLENS, 1462918, &sx,  0 );
	kx = MlibPcalKxksks( kx, MencIPrm->MaxTrq, MencIPrm->Jmot, &sx,   0 );
	kx = MlibPcalKxgain( kx, 400000,        1000000,   &sx, -30 );
#endif /* FLOAT_USE */

	if( MencV->PosCompMode == POSCOMP_SW )/* <S004> *//*<S05C>rename*/
	{/* SWEncoder position correction(Σ-ⅤCompatibility):32bitNormalization */
		MencV->AccErrLvl = kx;
	}
	else
	{/* JL-086Encoder position correction:24bitNormalization		*/
		MencV->AccErrLvl = kx >> ( 32 - 24);
		if( MencV->AccErrLvl > 0x007FFFFF )
		{
			MencV->AccErrLvl = 0x007FFFFF;
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Speed conversion gain calculation	[pulse/ScanA]→[2^24/]													*/
/*--------------------------------------------------------------------------------------------------*/
//	MencV->MotspdA = MlibScalKskxkx( bprm_ptr->Kmotspd, 1000000000, KPI_SACYCLENS, NULL, 24 );

/*--------------------------------------------------------------------------------------------------*/
/*		Movement amount conversion gain calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*				     1000																			*/
/*		Kmovpos = -----------		[0.001rev/pulse]												*/
/*				   (2^BitNo) 																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = 1000.0f / (1 << BitNo);
#else
	kx = MlibScalKxgain( 1000, 1, (1 << BitNo), NULL, 24 );
#endif /* FLOAT_USE */

#if (FLOAT_USE==TRUE)
	MencV->Kmovpos =  fw ;
#else
	MencV->Kmovpos = kx;
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Movement amount conversion gain calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					 pole * 65536																	*/
/*		Kphasepos = --------------		[65536/(360deg)/pulse]										*/
/*					(2^BitNo) * 2 																	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = MencP->oslv_pol.b.h * 32768.0f / (1 << BitNo);
#else
	kx = MlibScalKxgain( MencP->oslv_pol.b.h, 32768, (1 << BitNo), NULL, 24 );
#endif /* FLOAT_USE */

#if (FLOAT_USE==TRUE)
	MencV->Kphasepos = fw;
#else
	MencV->Kphasepos = kx;
#endif

}



/****************************************************************************************************/
/*																									*/
/*		Linear motor: Encoder initial parameter calculation													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Servo basic parameter calculation(BprmCalc.c)After the parameters necessary for encoder IF processing			*/
/*			Perform calculation.																			*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static void	SencInitParamCalcForLinear( MENCV *MencV, MENCPRM *MencP, MENC_REF_PRM *MencIPrm )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#endif
	LONG	kx, sx;
	UCHAR	BitNo;
	UCHAR	Dpoint;

	BitNo  = MencP->bit_dp.b.l;
	Dpoint = MencP->bit_dp.b.h;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of parameters for phase origin calculation																*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MorgSft      = 100 - Dpoint;
	MencV->MorgPassSft  =  96 - Dpoint;
	if( MencV->Sigma3Encoder )
	{
		MencV->IncreCompSft = 4;
	}
	else
	{
		MencV->IncreCompSft = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of positional calculation parameters																	*/
/*--------------------------------------------------------------------------------------------------*/
//	MencV->MposSftR = 32 - BitNo;											/* <S004> */
//	MencV->MposSftX = MencV->MposSftR - (80 - Dpoint);			/* <S004> */
	MencV->MposSftR = LMX_MPOS_SFT;									/* <S004> */
	MencV->MposSftX = LMX_MPOS_SFT;									/* <S004> */

	if( MencV->RevAsmMotor == TRUE )
	{
		MencV->MposSign = -1;
	}
	else
	{
		MencV->MposSign = 1;
	}
	MencV->MultiturnSft = MencP->bit_dp.b.h - 64;

/*--------------------------------------------------------------------------------------------------*/
/*		C phase less flag setting																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AbsoEncoder )
	{
		MencV->CphaseLess = TRUE;
	}
	else
	{
		MencV->CphaseLess = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Motor magnetic pole phase calculation coefficient																*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*						 	   65536							PulseNo  : [pulse/360deg]			*/
/*			Kmotphase	= ---------------						MotPhase : [65536/360deg]			*/
/*						      PulseNo																*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	MencV->Kmotphase = 65536.0f / (float)MencV->PulseNo;
	MencV->Kinvphase = (float)MencV->PulseNo / 65536.0f;
#else
	MencV->Kmotphase = MlibScalKxgain( 65536, 1, MencV->PulseNo, NULL, 24 );
	MencV->Kinvphase = MlibScalKxgain( MencV->PulseNo, 1, 65536, NULL, 24 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Acceleration abnormality detection level calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					  MaxTrq * 2^12	*  Ts  *  Ts			       MaxTrq * Ts * Ts					*/
/*		AccErrLvl = ------------------------------ * Cx	 = --------------------------------- * Cx	*/
/*					  Jmot * Kfbpls * 10^9 * 10^9			Jmot * Kfbpls * 244140625000000			*/
/*																									*/
/*		MaxTrq [Nm],[N]					: Maximum torque												*/
/*		Jmot   [kgm^2],[kg]				: Motor inertia											*/
/*		Kfbpls [rad/pulse],[m/pulse]	: FB pulse conversion coefficient										*/
/*		Ts     [ns]	   					: Calculation cycle													*/
/*		Cx     [-]	   					: Safety factor (=400.000) 										*/
/*																									*/
/*		※Safety factor is rotation typeSGDSabout400.													*/
/*		※however,SGDSAlthough the position data in linear is in pulse units, it uses the same calculated value as the rotation type, so	*/
/*		  In the case of linear,SGDSandSVOS.														*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = (float)KPI_SACYCLENS * (float)KPI_SACYCLENS / 244140625.0F;
	fw = fw * MencIPrm->MaxTrq / MencIPrm->Jmot;
	fw = fw / MencIPrm->Kfbpls;
	fw = fw * 400.0F / 1000000.0F;
#else
	kx = MlibScalKxgain( KPI_SACYCLENS, KPI_SACYCLENS, 244140625, &sx, 0 );
	kx = MlibPcalKxksks( kx, MencIPrm->MaxTrq, MencIPrm->Jmot, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 1, MencIPrm->Kfbpls, &sx, 0 );
	kx = MlibPcalKxgain( kx, 400, 1000000, &sx, -30 );
#endif /*FLOAT_USE==TRUE*/

	if( MencV->PosCompMode == POSCOMP_SW )/* <S004> *//*<S0C5>rename*/
	{/* SWEncoder position correction(Σ-ⅤCompatibility):32bitNormalization */
		MencV->AccErrLvl = kx;
	}
	else
	{/* JL-086Encoder position correction:24bitNormalization		*/
		MencV->AccErrLvl = (kx >> (32 - 24));
		if( MencV->AccErrLvl > 0x007FFFFF )
		{
			MencV->AccErrLvl = 0x007FFFFF;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Movement amount conversion gain calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					      ScalePitch					ScalePitch : [nm]							*/
/*		Kmovpos  = ------------------------				Kmovpos    : [0.01mm/pulse]					*/
/*					   (2^BitNo) * 10000															*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( MencV->NormScalePitch, 1, (10000<< BitNo), &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, MencV->NormScaleExp, &sx, 24 );

	MencV->Kmovpos = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Movement amount conversion gain calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					   (ScalePitch/10^9) * 65536		ScalePitch : [nm]							*/
/*		Kphasepos = ---------------------------------	[65536/(360deg)/pulse]						*/
/*					(2^BitNo) * (PolePitch/10^4) * 2 	PolePitch  : [0.1mm/180deg]					*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( MencV->NormScalePitch, 32768, 100000, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, (1 << BitNo), &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, MencP->ppitch, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, MencV->NormScaleExp, &sx, 24 );

	MencV->Kphasepos = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Speed conversion gain calculation	[pulse/ScanA]→[2^24/]													*/
/*--------------------------------------------------------------------------------------------------*/
//	MencV->MotspdA = MlibScalKskxkx( bprm_ptr->Kmotspd, 1000000000, KPI_SACYCLENS, NULL, 24 );

/*--------------------------------------------------------------------------------------------------*/
/*		PnE8C   : Serial conversion unit origin offset												*/
/*																									*/
/*		MencP.OrgDetDelayComp = PnE8C [0.01deg]×2^BITNO / 100 / 360[deg]							*/
/*--------------------------------------------------------------------------------------------------*/
/*		※CTo be used in phase position calculation,LMX_MPOS_SFTif it was									*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->OrgDetDelayComp = ( ((MencIPrm->scnvorgoffset << BitNo) / 100) << LMX_MPOS_SFT ) / 360;

}



/****************************************************************************************************/
/*																									*/
/*		SEN ON																	*/
/*																									*/
/****************************************************************************************************/
void	SencMakeAbsPosition(MENCV *MencV, USHORT limmlt, DBYTEX bit_dp, BOOL RvsDir)
{
	LONG	IniPos[2];
	LONG	MaxPos[2];
	UCHAR	BitNo;
	UCHAR	Sftx;

	/* Local variable initialization */
	BitNo	= bit_dp.b.l;
	Sftx	= bit_dp.b.h - BitNo - 48;

	IniPos[1] = (LONG)((USHORT)MencV->RxPosH[0] >> Sftx);			/* <S209> */
	IniPos[0] = (LONG)( ((ULONG)MencV->RxPosH[0] << (32-Sftx))
						 + ((ULONG)MencV->RxPosL[0].dw >> Sftx) );	/* <S209> */

	if( MencV->AbsoEncType == MENC_SABSO_TYPE )		/* 1Rotation absolute								*/
	{
		IniPos[1] = 0;
		IniPos[0] = ((ULONG)(IniPos[0]<<(32-BitNo)) >> (32-BitNo));
	}
	else												/* Multi-rotation absolute								*/
	{
		if( limmlt == 0xFFFF )							/* Determine sign extension using multiturn limit		*/
		{												/* Multi-rotation amount with sign							*/
			if( BitNo > 16 )							/* Set upper bit with sign bit				*/
			{
				IniPos[1] = (LONG)(IniPos[1] << (48-BitNo)) >> (48-BitNo);
			}
			else										/* Set the upper word with the sign of the lower word		*/
			{
				IniPos[1] = IniPos[0] >> 31;			/* High-order word setting(Lower word code)			*/
			}
		}
	}

	/*----------------------------------------------------------------------------------------------*/
	if( MencV->RevAsmMotor ^ RvsDir )					/* Inversion connection XOR Reverse rotation mode				*/
	{
		IniPos[1] = (~IniPos[1]);						/* Sign inversion processing								*/
		IniPos[0] = (~IniPos[0]) + 1;					/* Sign inversion processing								*/
		if( IniPos[0] == 0 )
		{	/* Carry, go to the top+1 */
			++IniPos[1];
		}
	/*----------------------------------------------------------------------------------------------*/
		if( (limmlt != 0xFFFF) && ((IniPos[0] | IniPos[1]) != 0) )
		{
			MaxPos[1] = ((LONG)limmlt + 1) >> (32-BitNo);
			MaxPos[0] = ((LONG)limmlt + 1) <<  BitNo;
			MlibAdd6464( MaxPos, IniPos, IniPos );
		}
	}


	/*	Set initial multiturn data */
	if( (MencV->AbsoEncoder == FALSE)						/* Increment encoder					*/
		|| (MencV->AbsoEncType == MENC_INCRE_TYPE) )		/* Using absolute as increment			*/
	{
		MencV->MotAbsPos = 0;
		MencV->SenOnScalePosL = 0;
		MencV->SenOnScalePosH = 0;
		MencV->SenOnMtnData = 0;
	}
	else													/* Absolute encoder						*/
	{
		MencV->MotAbsPos = IniPos[0];
		MencV->SenOnScalePosL = IniPos[0];
		MencV->SenOnScalePosH = IniPos[1];
		MencV->SenOnMtnData = SencGetMturnData(MencV,
		                                        limmlt, bit_dp,
		                                        (USHORT)MencV->RxPosH[0],
		                                        MencV->RxPosL[0].dw );
	}
	MencV->SenOnIncPulse = (ULONG)(MencV->RxPosL[0].dw >> MencV->MposSftX);
}





/****************************************************************************************************/
/*																									*/
/*		Rotation type: Serial encoder variable initialization process													*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Phase Table for Initial Phase																*/
/*--------------------------------------------------------------------------------------------------*/
static	const	USHORT	PhaseTableUVW[8] = {
/*	UVW   deg	Cal.					65536/360deg												*/
/*--------------------------------------------------------------------------------------------------*/
/*	000 :  NG : 0				*/		    0,
/*	001 : 330 : 65536*330/360	*/		60075,
/*	010 : 210 : 65536*210/360	*/		38229,
/*	011 : 270 : 65536*270/360	*/		49152,
/*	100 :  90 : 65536* 90/360	*/		16384,
/*	101 :  30 : 65536* 30/360	*/		 5461,
/*	110 : 150 : 65536*150/360	*/		27307,
/*	111 :  NG : 0				*/		    0
};
/*--------------------------------------------------------------------------------------------------*/
static SENC_ERROR	SencInitVariablesForRotary( MENCV *MencV, MENCPRM *MencP, ASIC_HW_IF_STR *AsicHwReg )
{
	LONG	i;
	LONG	lwk;
	LONG	RcvPosX;

/*--------------------------------------------------------------------------------------------------*/
/*		Initial position data(This time,Previous,Two previous times)														*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotPos = 0;									/* Motor Position [32bitRingPulse]		*/
	/*----------------------------------------------------------------------------------------------*/
	RcvPosX = MencV->MposSign * (LONG)((MencV->RxPosL[0].dw>>MencV->MposSftX)<<MencV->MposSftR);
	MencV->RcvPosX[0] = MencV->RcvPosX[1] = MencV->RcvPosX[2] = RcvPosX;
	MencV->MotPosX[0] = MencV->MotPosX[1] = RcvPosX;				/* Position initialization after correction 		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Set initial multiturn data																*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AbsoEncoder == FALSE )					/* Increment encoder					*/
	{
		MencV->IniMturn = 0;
	}
	else if( MencV->AbsoEncType == MENC_INCRE_TYPE )		/* Using absolute as increment			*/
	{
		MencV->IniMturn = 0;
	}
	else													/* Absolute encoder						*/
	{
		MencV->IniMturn = SencGetMturnData(MencV,
	                                        MencP->limmlt, MencP->bit_dp,
		                                    (USHORT)MencV->RxPosH[0],
		                                    MencV->RxPosL[0].dw);
	}
	MencV->IniIncre = ((ULONG)RcvPosX >> MencV->MposSftR);

/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related,Initial setting of phase origin: Incremental encoder										*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AbsoEncoder != TRUE )						/* Increment encoder					*/
	{
		MencV->Cset     = FALSE;							/* C phase settingFlag							*/
		MencV->Cpass    = FALSE;							/* C phase passingFlag							*/
//		MencV->DivCpass = FALSE;							/* Division C phase passFlag						*/
		MencV->DivInfo.DivCpass = FALSE;					/* Division C phase passFlag						*//*<S02D>*/
		MencV->OrgOk    = FALSE;							/* Origin setting completedFlag						*/
		MencV->CposOk   = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Initial motor phase origin calculation: Calculation from UVW signal													*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->Sigma3Encoder )
		{
			i = MencV->RxPosH[0] >> 1;
		}
		else
		{
			i = MencV->RxPosL[0].w.l >> 9;
		}
		lwk = (PhaseTableUVW[(i & 0x07)] * (65536/MencV->Kmotphase));
	/*------------------------------------------------------------------------------------------*/
		MencV->MotOrgX  = RcvPosX - lwk;					/* Motor origin position						*/
		MencV->MotCposX = 0;								/* Motor C phase position						*/
		MencV->MotAngle = 0;								/* Motor rotation angle(Pulse)					*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
//		if( MencV->DivOut == TRUE )						/* Motor encoder frequency division output selected		*/
		if( MencV->DivInfo.DivOut == TRUE )					/* Motor encoder frequency division output selected		*//*<S02D>*/
		{
			/* Normal C phase output selection(INGRAMForced output not supported) */
//			KPI_DIVSET_DCNORMAL( );
#if 0 /*<S02D>*/

			MencV->DivModeStatus = DIV_MODE_NORMAL ;

			AsicHwReg->AREG_WDT1SET |= BIT8;				/* Pulse conversion C phase mask					*/
			MencV->DivCMaskStatus = DIV_CMASK_ON;
			AsicHwReg->AREG_POSET0	= (USHORT)(RcvPosX >> MencV->DivOutSft);
			AsicHwReg->AREG_POTEST	= (USHORT)(RcvPosX >> MencV->DivOutSft);
#else/*<S02D>*/
			/* Normal C phase output selection(INGRAMForced output not supported) */
			MencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL ;

			AsicHwReg->AREG_WDT1SET |= BIT8;				/* Pulse conversion C phase mask					*/
			MencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;
			AsicHwReg->AREG_POSET0	= (ULONG)(RcvPosX >> MencV->DivInfo.DivOutSft);
			AsicHwReg->AREG_POTEST	= (ULONG)(RcvPosX >> MencV->DivInfo.DivOutSft);
			if(( MencV->DivInfo.DivPass != FALSE )&&(MencV->DivInfo.DivOutASIC != FALSE))          /*<S0D3>*/
			{/* Division through mode,H/WFor division*/
				/* Normal C phase output selection */
				AsicHwReg->AREG_DIVSET &= 0xfffff3ff;
				MencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL ;
			}
#endif/*<S02D>*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related,Initial setting of phase origin: Use absolute as increment								*/
/*--------------------------------------------------------------------------------------------------*/
	else if( MencV->AbsoEncType == MENC_INCRE_TYPE )		/* Using absolute as increment			*/
	{
		MencV->Cset     = TRUE;							/* C phase settingFlag							*/
		MencV->Cpass    = FALSE;							/* C phase passingFlag							*/
//		MencV->DivCpass = FALSE;							/* Division C phase passFlag						*/
		MencV->DivInfo.DivCpass = FALSE;					/* Division C phase passFlag						*//*<S02D>*/
		MencV->OrgOk    = FALSE;							/* Origin setting completedFlag						*/
		MencV->CposOk   = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
		MencV->MotOrgX  = 0;								/* Motor origin position						*/
		MencV->MotCposX = 0;								/* Motor C phase position						*/
		MencV->MotAngle = ((ULONG)(RcvPosX) >> MencV->MposSftR); /* Motor rotation angle(Pulse)		*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
#if 0 /*<S02D>*/
		if( MencV->DivOut == TRUE )						/* Motor encoder frequency division output selected		*/
		{
			AsicHwReg->AREG_POSET0	= (USHORT)(RcvPosX >> MencV->DivOutSft);
			AsicHwReg->AREG_POTEST	= (USHORT)(RcvPosX >> MencV->DivOutSft);
			/* Normal C phase output selection */
//			KPI_DIVSET_DCNORMAL( );
			MencV->DivModeStatus = DIV_MODE_NORMAL ;

			/* C-phase resynchronization enabled setting */
			AsicHwReg->AREG_DINCSET	|= BIT5;

			/* Divide always clear settingOFF(Is it necessary?) */
			AsicHwReg->AREG_DINCSET	&= ~(BIT10);

			/* C-phase resynchronization enabled */
			AsicHwReg->AREG_DINCSET	&= ~(BIT5);

			/* Pulse conversion C phase mask ON */
			AsicHwReg->AREG_WDT1SET |= BIT8;
			MencV->DivCMaskStatus = DIV_CMASK_ON;

			/* Pulse conversion C phase mask OFF(INGRAMForced output not supported) */
//			KPI_PCMASK_OFF( );
			MencV->DivCMaskStatus  = DIV_CMASK_OFF ;
		}
#else/*<S02D>*/
		if( MencV->DivInfo.DivOut == TRUE )					/* Motor encoder frequency division output selected		*/
		{
			AsicHwReg->AREG_POSET0	= (ULONG)(RcvPosX >> MencV->DivInfo.DivOutSft);
			AsicHwReg->AREG_POTEST	= (ULONG)(RcvPosX >> MencV->DivInfo.DivOutSft);
			/* Normal C phase output selection				*/
			AsicHwReg->AREG_DIVSET &= 0xfffff3ff;
			MencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL ;

			/* C-phase resynchronization enabled setting */
			AsicHwReg->AREG_DINCSET	|= BIT5;

			/* Divide always clear settingOFF(Is it necessary?) */
			AsicHwReg->AREG_DINCSET	&= ~(BIT10);

			/* C-phase resynchronization enabled */
			AsicHwReg->AREG_DINCSET	&= ~(BIT5);

			/* Pulse conversion C phase mask ON */
			AsicHwReg->AREG_WDT1SET |= BIT8;
			MencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;

			/* Pulse conversion C phase mask OFF */
			AsicHwReg->AREG_WDT1SET &= ~BIT8;
			MencV->DivInfo.DivCMaskStatus  = DIV_CMASK_OFF ;
		}
#endif/*<S02D>*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related,Initial setting of phase origin: Use absolute as absolute									*/
/*--------------------------------------------------------------------------------------------------*/
	else													/* Using absolute as absolute				*/
	{
#if 0/*<S02D>*/
		MencV->Cset     = TRUE;							/* C phase settingFlag							*/
		MencV->Cpass    = TRUE;							/* C phase passingFlag							*/
		MencV->DivCpass = TRUE;							/* Division C phase passFlag						*/
		MencV->OrgOk    = TRUE;							/* Origin setting completedFlag						*/
		MencV->CposOk   = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
		MencV->MotOrgX  = 0;								/* Motor origin position						*/
		MencV->MotCposX = 0;								/* Motor C phase position						*/
		MencV->MotAngle = ((ULONG)(RcvPosX) >> MencV->MposSftR); /* Motor rotation angle(Pulse)		*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivOut == TRUE )						/* Motor encoder frequency division output selected		*/
		{
			if( MencV->DivPass == FALSE )					/* Division through mode?					*/
			{
				/* Division C phase output selection 2(INGRAMForced output not supported) */
//				KPI_DIVSET_DCC2( );
				MencV->DivModeStatus = DIV_MODE_DCC2 ;
			}
			else
			{
				/* Normal C phase output selection(INGRAMForced output not supported) */
//				KPI_DIVSET_DCNORMAL( );
				MencV->DivModeStatus = DIV_MODE_NORMAL ;
			}
		}
#else/*<S02D>*/
		MencV->Cset     = TRUE;								/* C phase settingFlag							*/
		MencV->Cpass    = TRUE;								/* C phase passingFlag							*/
		MencV->DivInfo.DivCpass = TRUE;						/* Division C phase passFlag						*/
		MencV->OrgOk    = TRUE;								/* Origin setting completedFlag						*/
		MencV->CposOk   = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
		MencV->MotOrgX  = 0;								/* Motor origin position						*/
		MencV->MotCposX = 0;								/* Motor C phase position						*/
		MencV->MotAngle = ((ULONG)(RcvPosX) >> MencV->MposSftR);		/* Motor rotation angle(Pulse)		*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivInfo.DivOut == TRUE )					/* Motor encoder frequency division output selected		*/
		{
			if( MencV->DivInfo.DivPass == FALSE )			/* Division through mode?					*/
			{
				/* Division C phase output selection 2 */
				AsicHwReg->AREG_DIVSET |= (BIT11 | BIT8);
				MencV->DivInfo.DivModeStatus = DIV_MODE_DCC2 ;
			}
			else
			{
				/* Normal C phase output selection */
				AsicHwReg->AREG_DIVSET &= 0xfffff3ff;
				MencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL ;
			}
		}
#endif/*<S02D>*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder ready setting : Set after encoder initialization/parameter initialization							*/
/*--------------------------------------------------------------------------------------------------*/
#if 0/*<S02D>*/
	if( MencV->EncConnect == FALSE )						/* When encoder is not connected					*/
	{
		MencV->SenReady = FALSE;							/* SENSignal processing completeFlag					*/
		MencV->IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( MencV->DivOut == FALSE )					/* Motor encoder frequency division output: When not selected	*/
	{
		MencV->SenReady = TRUE;							/* SENSignal processing completeFlag					*/
		MencV->IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( MencV->AbsoEncType )						/* When using absolute encoder					*/
	{
		/* Since no serial output is performed, at initializationTRUEonly for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.										*/
		MencV->SenReady = TRUE;							/* SENSignal processing completeFlag					*/
		MencV->IncPulseReq = TRUE;						/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else													/* When using incremental encoder					*/
	{
		MencV->SenReady = TRUE;							/* SENSignal processing completeFlag					*/
		MencV->IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
	}
#else/*<S02D>*/
	if( MencV->EncConnect == FALSE )						/* When encoder is not connected					*/
	{
		MencV->SenReady = FALSE;							/* SENSignal processing completeFlag					*/
		MencV->DivInfo.IncPulseReq = FALSE;					/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( MencV->DivInfo.DivOut == FALSE )				/* Motor encoder frequency division output: When not selected	*/
	{
		MencV->SenReady = TRUE;								/* SENSignal processing completeFlag					*/
		MencV->DivInfo.IncPulseReq = FALSE;					/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( MencV->AbsoEncType )							/* When using absolute encoder					*/
	{
		/* Since no serial output is performed, at initializationTRUEonly for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.										*/
		MencV->SenReady = TRUE;								/* SENSignal processing completeFlag					*/
		MencV->DivInfo.IncPulseReq = TRUE;					/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else													/* When using incremental encoder					*/
	{
		MencV->SenReady = TRUE;								/* SENSignal processing completeFlag					*/
		MencV->DivInfo.IncPulseReq = FALSE;					/* Initial increment pulse output request			*/
	}

#endif/*<S02D>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder flag related: Common settings																*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotCphPass		= FALSE;					/* C phase passingFlag							*/
	MencV->IncreCposLatch	= FALSE;					/* C phase position latch signal(ScanA-->ScanB)	*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->PhaseReady		= TRUE;						/* Phase detection completedFlag						*/
//	MencV->EncDisable		= FALSE;					/* Encoder disableFlag			*/
	MencV->AccChkWaitCnt		= 0;						/* Acceleration detection start count clear			*/
	MencV->AccChkEnable		= FALSE;					/* Acceleration checkFlag					*/
	MencV->SpdCmpEnable		= FALSE;					/* Speed correction processing enabledFlag					*/
//	MencV->DivRatFix			= FALSE;					/* Maximum speed, division ratio relationship					*/
	MencV->DivInfo.DivRatFix = FALSE;						/* Maximum speed, division ratio relationship					*//*<S02D>*/
	MencV->MpfPhaseOffset	= 0;						/* Initial value setting of magnetic pole information		<V251>		*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->PaoseqNormal = FALSE;							/* PAOOutputSeq Initial processing settings				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Select function presence/absence: Increpulse output request reset												*/
/*--------------------------------------------------------------------------------------------------*/
#if 0/*<S02D>*/
	if( SVFSEL_INCPOUT == 0 )
	{
		MencV->IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
	}
#else/*<S02D>*/
	if( SVFSEL_INCPOUT == 0 )
	{
		MencV->DivInfo.IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
	}
	else
	{
		if( MencV->DivInfo.DivOut == TRUE) /* && (Kprm.f.MotorLessTestMode==FALSE) )*/
		{
			MencV->DivInfo.IncPulseReq = TRUE;					/* Initial increment pulse output request			*/
		}
	}
#endif/*<S02D>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Initial value setting of rotation speed data																	*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotSpd = 0;									/* Motor Speed [2^24/OvrSpd]			*/

	return SENCERR_SUCCESS;
}



/****************************************************************************************************/
/*																									*/
/*		Linear motor: Serial encoder variable initialization processing												*/
/*																									*/
/****************************************************************************************************/
static SENC_ERROR	SencInitVariablesForLinear( MENCV *MencV, MENCPRM *MencP,
												MENC_REF_PRM *MencIPrm, ASIC_HW_IF_STR *AsicHwReg )
{
	LONG		idx;
	LONG		lwk;
	LONG		RcvPosX;
	SENC_ERROR	err;

	err = SENCERR_SUCCESS;
/*--------------------------------------------------------------------------------------------------*/
/*		Initial position data(This time,Previous,Two previous times)														*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotPos  = 0;									/* Motor Position	[32bitRing]			*/
	MencV->MotCposX = 0;									/* Motor CphasePos.	[32bitRing]			*/
	/*----------------------------------------------------------------------------------------------*/
	MencV->RxPosH[1]  = MencV->RxPosH[0];				/* Previous increment correction value initial setting			*/
	/* LposToXpos */
	RcvPosX = MencV->MposSign * ( (MencV->RxPosL[0].Long >> LMX_MPOS_SFT) << LMX_MPOS_SFT );
	MencV->RcvPosX[0] = MencV->RcvPosX[1] = MencV->RcvPosX[2] = RcvPosX;
	MencV->MotPosX[0] = MencV->MotPosX[1] = RcvPosX;

/*--------------------------------------------------------------------------------------------------*/
/*		Initial absolute position data(Absolute position36BitLower part of data32BitExtract the data)							*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AbsoEncoder == TRUE )
	{
		MencV->MotAbsPos = (MencV->RxPosH[0]<<(32-LMX_MPOS_SFT)) | ((ULONG)MencV->RxPosL[0].dw>>LMX_MPOS_SFT);
		MencV->MotAbsPos =  MencV->MposSign * MencV->MotAbsPos;
	}
	else
	{
		MencV->MotAbsPos = 0;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Phase angle initial value		0 ≦ MencV.MotPhasX < MencP.PulseNo										*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotPhaX = MencV->MotAbsPos % MencV->PulseNo;	/* Motor PhasePos [PuleseNo/360deg]	*/
	if( MencV->MotPhaX < 0 )
	{
		MencV->MotPhaX += MencV->PulseNo;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Set initial multiturn data																*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->IniMturn = 0;
	MencV->IniIncre = RcvPosX >> LMX_MPOS_SFT;			/* XposToUpos							*/

/*--------------------------------------------------------------------------------------------------*/
/*		Initial setting of phase origin: With pole sensor														*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->withPoleSensor )							/* With pole sensor						*/
	{
	/*----------------------------------------------------------------------------------------------*/
	/*	Initial motor phase origin calculation: Calculation from UVW signal													*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->Sigma3Encoder )
		{
			lwk = MencV->RxPosH[0] >> 1;
		}
		else
		{
			lwk = MencV->RxPosL[0].w.l >> 9;
		}
		idx = MencV->MotPoleInfo[0] = MencV->MotPoleInfo[1] = (UCHAR)(lwk & 0x07);
#if (FLOAT_USE==TRUE)
		lwk = (LONG)((float)PhaseTableUVW[idx] * MencV->Kinvphase );
#else
		lwk = MlibMulgainNolim( PhaseTableUVW[idx], MencV->Kinvphase );
#endif /* FLOAT_USE */

	/*----------------------------------------------------------------------------------------------*/
		if( MencV->PhaseOrderUWV == FALSE )				/* UVW phase order							*/
		{
			/* MotOrgX [PulseNo/360deg] */
			MencV->MotOrgX = MencV->MotPhaX - lwk;
			/* Hall sensor offset amount */
			MencV->PhaseOffset = (MencP->poleoffset<<16) / 360;
		}
		else												/* UWV phase order							*/
		{
			/* MotOrgX [PulseNo/360deg] */
			MencV->MotOrgX = MencV->MotPhaX + lwk;
			/* Hall sensor offset amount*/
			MencV->PhaseOffset = ((360-MencP->poleoffset)<<16) / 360;
		}
		MencV->PhaseReady = TRUE;						/* Phase detection completedFlag						*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Pole sensor abnormality check: All Lo or all Hi is abnormal									*/
	/*----------------------------------------------------------------------------------------------*/
		if( (MencV->MotPoleInfo[1]==0) || (MencV->MotPoleInfo[1]==7) )
		{
			err = SENCERR_POLESENSOR;
		}
		MencV->MpfPhaseOffset = 0;							/* Initial value setting of magnetic pole information				*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Initial setting of phase origin: Without pole sensor														*/
/*--------------------------------------------------------------------------------------------------*/
	else													/* Without pole sensor						*/
	{
		MencV->MotOrgX = 0;									/* Motor origin						*/
		MencV->PhaseOffset = 0;								/* Hall sensor offset amount 		*/
		MencV->MotPoleInfo[0] = MencV->MotPoleInfo[1] = 0;/* Hall sensor information					*/
		if( MencV->AbsoEncType == MENC_INCRE_TYPE )			/* When increment is used					*/
		{
			MencV->PhaseReady = FALSE;						/* Phase detection completedFlag					*/
			MencV->MpfPhaseOffset = 0;						/* Initial value setting of magnetic pole information				*/
		}
		else													/* When using absolute						*/
		{
			/* Phase detection completedFlag */
			MencV->PhaseReady = (MencIPrm->syssw3 & 0x0020) ? TRUE : FALSE;
			MencV->MpfPhaseOffset = MencIPrm->phsofst;		/* Initial value of magnetic pole information(PnE8D)Setting		*/
			/*--------------------------------------------------------------------------------------*/
			/* Scale sideEEPROMand amplifier sideEEPROM									*/
			/*--------------------------------------------------------------------------------------*/
			if( MencIPrm->phsofst != MencP->phscmpM2 )			/* PnF1DandPnE8D				*/
			{
				err = SENCERR_PHASEANGLE;
				MencV->PhaseReady = FALSE;
			}
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Initialization of motor origin check variables															*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotOrgChkEnd = FALSE;
	MencV->MotOrgChkCnt   = 0;
	MencV->MotOrgXchk[0]  = MencV->MotOrgXchk[1] = MencV->MotOrgX;

/* 2009.09.24 Y.Oka I want to disable the frequency division pulse output function. */
/*--------------------------------------------------------------------------------------------------*/
/*		Divided pulse initial settings																			*/
/*--------------------------------------------------------------------------------------------------*/
//	MencV->DivOutRem = 0;
//	MencV->DivOutPos = MlibPfbkxremNolim( MencV->IniIncre, MencV->DivOutGain, &MencV->DivOutRem );
//	MencV->InitDivPosOffset = MencV->DivOutPos ;
#if 0 /*<S02D>*/
/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related: Incremental encoder															*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AbsoEncoder == FALSE )
	{
		MencV->Cset = FALSE;								/* C phase settingFlag							*/
		MencV->CposOk = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivOut == TRUE )
		{
			/* Pulse conversion C phase mask ON				*/
//			KPI_PCMASK_ON( );
			AsicHwReg->AREG_WDT1SET |= BIT8;
			MencV->DivCMaskStatus = DIV_CMASK_ON;

			/* Normal C phase output selection(INGRAMForced output not supported) */
//			KPI_DIVSET_DCNORMAL( );
			MencV->DivModeStatus = DIV_MODE_NORMAL;

			AsicHwReg->AREG_POSET0 = (USHORT)MencV->DivOutPos;
			AsicHwReg->AREG_POTEST = (USHORT)MencV->DivOutPos;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related: Absolute encoder (Using absolute as increment)								*/
/*--------------------------------------------------------------------------------------------------*/
	else if( MencV->AbsoEncType == MENC_INCRE_TYPE )
	{
		MencV->Cset = TRUE;								/* C phase settingFlag							*/
		MencV->CposOk = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivOut == TRUE )
		{
			/* Pulse conversion C phase mask ON				*/
//			KPI_PCMASK_ON( );
			AsicHwReg->AREG_WDT1SET |= BIT8;
			MencV->DivCMaskStatus = DIV_CMASK_ON;

			/* Normal C phase output selection						*/
//			KPI_DIVSET_DCNORMAL( );
			MencV->DivModeStatus = DIV_MODE_NORMAL;

			AsicHwReg->AREG_POSET0 = (USHORT)MencV->DivOutPos;
			AsicHwReg->AREG_POTEST = (USHORT)MencV->DivOutPos;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related: Absolute encoder (Using absolute as absolute)									*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		MencV->Cset = TRUE;								/* C phase settingFlag							*/
		MencV->CposOk = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivOut == TRUE )
		{
			/* Pulse conversion C phase mask ON				*/
//			KPI_PCMASK_ON( );
			AsicHwReg->AREG_WDT1SET |= BIT8;
			MencV->DivCMaskStatus = DIV_CMASK_ON;

			/* Normal C phase output selection(INGRAMForced output not supported) */
//			KPI_DIVSET_DCNORMAL( );
			MencV->DivModeStatus = DIV_MODE_NORMAL;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder ready setting : Set after encoder initialization/parameter initialization							*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->EncConnect == FALSE )						/* When encoder is not connected					*/
	{
		MencV->SenReady = FALSE;							/* SENSignal processing completeFlag					*/
		MencV->IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( MencV->DivOut == FALSE )					/* Motor encoder frequency division output: When not selected	*/
	{
		MencV->SenReady = TRUE;							/* SENSignal processing completeFlag					*/
		MencV->IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( MencV->AbsoEncType )						/* When using absolute encoder					*/
	{
//		MencV->SenReady = FALSE;							/* SENSignal processing completeFlag					*/
		MencV->SenReady = TRUE;								/* SENSignal processing completeFlag					*/
		MencV->IncPulseReq = TRUE;						/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else													/* When using incremental encoder					*/
	{
		MencV->SenReady = TRUE;							/* SENSignal processing completeFlag					*/
		MencV->IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder flag setting : Common processing																*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotCphPass		= FALSE;					/* C phase passingFlag							*/
	MencV->IncreCposLatch	= FALSE;					/* C phase position latch signal(ScanA-->ScanB)	*/
//	MencV->EncDisable		= FALSE;					/* Encoder disableFlag			*/
	MencV->AccChkEnable		= FALSE;					/* Acceleration checkFlag					*/
	MencV->SpdCmpEnable		= FALSE;					/* Speed correction processing enabledFlag					*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->PaoseqNormal		= FALSE;					/* PAOOutputSeq Initial processing settings				*/

/* 2009.09.24 Y.Oka No initial increpulse output */
/*--------------------------------------------------------------------------------------------------*/
/*		Select function presence/absence: Increpulse output request reset												*/
/*--------------------------------------------------------------------------------------------------*/
//	if( SVFSEL_INCPOUT == 0 )
//	{
//		MencV->IncPulseReq = FALSE;						/* Initial increment pulse output request			*/
//	}
#else/*<S02D>*/
/* <S21B>:Start */
	MencV->DivInfo.DivOutRem = 0;
	MencV->DivInfo.DivOutPos = MlibPfbkxremNolim( MencV->IniIncre, MencV->DivInfo.DivOutGain, &MencV->DivInfo.DivOutRem );
	MencV->DivInfo.InitDivPosOffset = MencV->DivInfo.DivOutPos ;
/* <S21B>:end */
/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related: Incremental encoder															*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AbsoEncoder == FALSE )
	{
		MencV->Cset = FALSE;								/* C phase settingFlag							*/
		MencV->CposOk = FALSE;								/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivInfo.DivOut == TRUE )
		{
			/* Pulse conversion C phase mask ON				*/
			AsicHwReg->AREG_WDT1SET |= BIT8;
			MencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;

			/* Normal C phase output selection(INGRAMForced output not supported) */
			MencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL;
			AsicHwReg->AREG_POSET0 = (ULONG)MencV->DivInfo.DivOutPos;
			AsicHwReg->AREG_POTEST = (ULONG)MencV->DivInfo.DivOutPos;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related: Absolute encoder (Using absolute as increment)								*/
/*--------------------------------------------------------------------------------------------------*/
	else if( MencV->AbsoEncType == MENC_INCRE_TYPE )
	{
		MencV->Cset = TRUE;								/* C phase settingFlag							*/
		MencV->CposOk = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivInfo.DivOut == TRUE )
		{
			/* Pulse conversion C phase mask ON				*/
			AsicHwReg->AREG_WDT1SET |= BIT8;
			MencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;

			/* Normal C phase output selection						*/
			MencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL;
			AsicHwReg->AREG_POSET0 = (ULONG)MencV->DivInfo.DivOutPos;
			AsicHwReg->AREG_POTEST = (ULONG)MencV->DivInfo.DivOutPos;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase flag related: Absolute encoder (Using absolute as absolute)									*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		MencV->Cset = TRUE;								/* C phase settingFlag							*/
		MencV->CposOk = FALSE;							/* C-phase position setting completedFlag					*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Division output initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->DivInfo.DivOut == TRUE )
		{
			/* Pulse conversion C phase mask ON				*/
			AsicHwReg->AREG_WDT1SET |= BIT8;
			MencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;

			/* Normal C phase output selection(INGRAMForced output not supported) */
			MencV->DivInfo.DivModeStatus = DIV_MODE_NORMAL;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder ready setting : Set after encoder initialization/parameter initialization							*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->EncConnect == FALSE )						/* When encoder is not connected					*/
	{
		MencV->SenReady = FALSE;							/* SENSignal processing completeFlag					*/
		MencV->DivInfo.IncPulseReq = FALSE;					/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( MencV->DivInfo.DivOut == FALSE )				/* Motor encoder frequency division output: When not selected	*/
	{
		MencV->SenReady = TRUE;								/* SENSignal processing completeFlag					*/
		MencV->DivInfo.IncPulseReq = FALSE;					/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( MencV->AbsoEncType )							/* When using absolute encoder					*/
	{
		MencV->SenReady = TRUE;								/* SENSignal processing completeFlag					*/
		MencV->DivInfo.IncPulseReq = TRUE;					/* Initial increment pulse output request			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else													/* When using incremental encoder					*/
	{
		MencV->SenReady = TRUE;								/* SENSignal processing completeFlag					*/
		MencV->DivInfo.IncPulseReq = FALSE;					/* Initial increment pulse output request			*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder flag setting : Common processing																*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotCphPass		= FALSE;						/* C phase passingFlag							*/
	MencV->IncreCposLatch	= FALSE;						/* C phase position latch signal(ScanA-->ScanB)	*/
	MencV->AccChkEnable		= FALSE;						/* Acceleration checkFlag					*/
	MencV->SpdCmpEnable		= FALSE;						/* Speed correction processing enabledFlag					*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->PaoseqNormal		= FALSE;						/* PAOOutputSeq Initial processing settings				*/

	if( SVFSEL_INCPOUT == 0 )
	{
		MencV->DivInfo.IncPulseReq = FALSE;					/* Initial increment pulse output request			*/
	}
	else
	{
		MencV->DivInfo.IncPulseReq = TRUE;					/* Initial increment pulse output request			*/
	}
#endif/*<S02D>*/

/*--------------------------------------------------------------------------------------------------*/
/*		Initial value setting of rotation speed data																	*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->MotSpd = 0;									/* Motor Speed [2^24/OvrSpd]			*/

	return err;
}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder variable initialization process															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : In initial processing, read the initial position data of the serial encoder,					*/
/*			Set location data and previous location.														*/
/*			Also, depending on the encoder type, set the C-phase output and do not output the initial pulse.				*/
/*																									*/
/****************************************************************************************************/
SENC_ERROR	SencInitMencVariables(MENCV *MencV, MENCPRM *MencP, MENC_REF_PRM *MencInPrm )
{
	SENC_ERROR		err, rc_err;

	/*----------------------------------------------------------------------------------------------*/
	/*		Reading data from the serial encoder													*/
	/*----------------------------------------------------------------------------------------------*/
	rc_err = SencReadInitialP1Data(MencV);

	/*----------------------------------------------------------------------------------------------*/
	/*		Serial encoder variable initialization process, Initial absolute position([pulse]&[Command unit])Calculation					*/
	/*----------------------------------------------------------------------------------------------*/
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* Linear type */

		/* Initial parameter calculation */
		SencInitParamCalcForLinear( MencV, MencP, MencInPrm );

		/* Variable initialization process */
		err = SencInitVariablesForLinear( MencV, MencP, MencInPrm, MencV->EncIf.SencSetReg );/* <S04D> Argument change */
		if(rc_err == SENCERR_SUCCESS)
		{	rc_err = err;}
	}
	else
	{ /* Rotation type */

		/* Initial parameter calculation */
		SencInitParamCalcForRotary( MencV, MencP, MencInPrm );

		/* Variable initialization process */
		err = SencInitVariablesForRotary( MencV, MencP, MencV->EncIf.SencSetReg );/* <S04D> Argument change */
		if(rc_err == SENCERR_SUCCESS)
		{	rc_err = err;}
	}

	return rc_err;
}


#pragma arm section code = "CALL_SCANA" /* <S1D2> */
//#if	(SVFSEL_DIVOUT_USE == TRUE)  /* <S02D> *//* <S09F> */
/****************************************************************************************************/
/*																									*/
/*		Initial increment pulse output processing																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Serial encoder PAO phase serial output sequence(SencOutputPaoSerial())					*/
/*			and output to the absolute encoder connection at the rise of the SEN signal				*/
/*			Perform multiturn data acquisition and initial increpulse output processing.								*/
/*																									*/
/*																									*/
/*	Supplementary Foot : Since this is a process when the servo is off, there is some leeway in terms of processing time.							*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#define MAXPULSEOUT		170
/*--------------------------------------------------------------------------------------------------*/
void	 SencOutIniIncPulse( MENCV  *MencV, ASIC_HW_IF_STR  *pAsicHwReg )
{
LONG	lwk;
LONG	MtnCur;
LONG	MtnSub;
LONG	MtnSepos;
#if	(SVFSEL_DIVOUT_USE == TRUE)       /* <S09F> */
/*--------------------------------------------------------------------------------------------------*/
/*		Communication error check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( MencV->SeRxErr == TRUE )
		{
			return;											/* Do nothing when communication error occurs				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Multiturn data acquisition processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( MencV->DivInfo.PaoSeqMcmd == KPX_MTLATCH_REQUEST )	/* Latch request command check		*/
		{
			MencV->DivInfo.MturnLatch = SencGetMturnData( MencV,
			                                      MencV->DivInfo.Limmlt, 
			                                      MencV->DivInfo.Bit_dp,
												  (USHORT)MencV->RxPosH[0],
												  MencV->RxPosL[0].dw );
			MencV->DivInfo.IncPulseOut = 0;				/* Incre pulse output reset			*/
			pAsicHwReg->AREG_WDT1SET &= ~BIT8;				/* Pulse conversionCPhase maskOFFSetting			*/
			MencV->DivInfo.DivCMaskStatus  = DIV_CMASK_OFF ;
			MencV->DivInfo.PaoSeqMcmd = KPX_MTLATCH_FINISH;/* Response ==> SencOutputPaoSerial()		*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Initial increment pulse output processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		else if( MencV->DivInfo.PaoSeqMcmd == KPX_IPLSOUT_REQUEST )/* Pulse output command check	*/
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Initial increment pulse output data calculation													*/
		/*------------------------------------------------------------------------------------------*/
			MtnCur = SencGetMturnData( MencV,
                                       MencV->DivInfo.Limmlt, 
                                       MencV->DivInfo.Bit_dp,
									   (USHORT)MencV->RxPosH[0],
									   MencV->RxPosL[0].dw );

			MtnSub = SencCheckMturnLimit( MtnCur, MencV );
			MtnSepos = ((ULONG)MencV->RcvPosX[0]>>MencV->DivInfo.DivOutSft) + (MtnSub<<(32-MencV->DivInfo.DivOutSft));
		/*------------------------------------------------------------------------------------------*/
		/*		Initial increment pulse output data check & output data setting								*/
		/*------------------------------------------------------------------------------------------*/
			lwk = MtnSepos - MencV->DivInfo.IncPulseOut;
			if( lwk > MAXPULSEOUT )							/* Pulse output upper limit						*/
			{
				MencV->DivInfo.IncPulseOut += MAXPULSEOUT;	/* Pulse output data setting					*/
			}
			else if( lwk < -MAXPULSEOUT )					/* Pulse output lower limit						*/
			{
				MencV->DivInfo.IncPulseOut -= MAXPULSEOUT;	/* Pulse output data setting					*/
			}
			else											/* Final pulse output						*/
			{
				MencV->DivInfo.IncPulseOut = MtnSepos;		/* Pulse output data setting					*/
				MencV->DivInfo.PaoSeqMcmd = KPX_IPLSOUT_FINISH;/* Response ==> SencOutputPaoSerial		*/
				MencV->DivInfo.IncPulseReq = FALSE;			/* Initial increment pulse output completed			*/
			}
		/*------------------------------------------------------------------------------------------*/
			pAsicHwReg->AREG_POSET0	= MencV->DivInfo.IncPulseOut;	/* Pulse output circuit initialization			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		No command																				*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			;												/* Waiting for command							*/
		}
/*--------------------------------------------------------------------------------------------------*/
#endif                           /* <S09F> */
		return;

}
#pragma arm section code

#pragma arm section code = "CALL_SCANA"
/****************************************************************************************************/
/*																									*/
/*		Linear motor(Feedback option): Initial incremental pulse output processing							*/
/*																									*/
/****************************************************************************************************//* <S209> */
void	LmxOutSencIniIncPulse( MENCV  *MencV, ASIC_HW_IF_STR  *pAsicHwReg, UCHAR Sftx )
{
LONG	lwk;
LONG	MtnCur;
LONG	MtnSub;
LONG	MtnRem;
LONG	MtnSepos;
SHORT	MturnLatchCur;

#if	(SVFSEL_DIVOUT_USE == TRUE)
/*--------------------------------------------------------------------------------------------------*/
/*		Communication error check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( MencV->SeRxErr == TRUE )
		{
			return;											/* Do nothing when communication error occurs				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Multiturn data acquisition processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( MencV->DivInfo.PaoSeqMcmd == KPX_MTLATCH_REQUEST )	/* Latch request command check		*/
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Absolute position36BitUpper part of data32BitExtract data and calculate multiturn division value		*/
		/*------------------------------------------------------------------------------------------*/
			MtnCur    = ((USHORT)MencV->RxPosH[0] << 16 ) | ((USHORT)MencV->RxPosL[0].w.h );
			MtnRem    = 0;
			MtnCur    = MlibPfbkxremNolim( MtnCur, MencV->DivInfo.DivOutGain, &MtnRem );
			MtnSepos = (MencV->RxPosH[0]<<(32-Sftx)) | ((ULONG)MencV->RxPosL[0].dw>>Sftx);
			MencV->DivInfo.DivOutRem = 0;														
			MtnSepos =  MlibPfbkxremNolim( MtnSepos, MencV->DivInfo.DivOutGain, &MencV->DivInfo.DivOutRem );
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
			MencV->DivInfo.MturnLatch = (SHORT)( MtnCur >> 16);
		/*------------------------------------------------------------------------------------------*/
		/*		Divided pulse reset processing																*/
		/*------------------------------------------------------------------------------------------*/
			MencV->DivInfo.DivOutPos   = 0;					/* Division output pulse reset				*/
			MencV->DivInfo.IncPulseOut = 0;					/* Incre pulse output reset			*/
			MencV->DivInfo.PaoSeqMcmd  = KPX_MTLATCH_FINISH;/* Response ==> LpxOutputSencPaoSerial()	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Initial increment pulse output processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		else if( MencV->DivInfo.PaoSeqMcmd == KPX_IPLSOUT_REQUEST )	/* Pulse output command check	*/
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Calculating the change in multiturn data														*/
		/*		Absolute position36BitLower part of data32BitExtract the data and calculate the initial increment frequency division value		*/
		/*------------------------------------------------------------------------------------------*/
			MtnCur        = ((USHORT)MencV->RxPosH[0] << 16 ) | ((USHORT)MencV->RxPosL[0].w.h );
			MtnRem        = 0;
			MtnCur        = MlibPfbkxremNolim( MtnCur, MencV->DivInfo.DivOutGain, &MtnRem );
			MtnSepos = (MencV->RxPosH[0]<<(32-Sftx)) | ((ULONG)MencV->RxPosL[0].dw>>Sftx);
			MencV->DivInfo.DivOutRem   = 0;						/* Division output pulse remainder reset		*/
			MtnSepos =  MlibPfbkxremNolim( MtnSepos, MencV->DivInfo.DivOutGain, &MencV->DivInfo.DivOutRem );
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
			MtnSub = (USHORT)MturnLatchCur - (USHORT)MencV->DivInfo.MturnLatch;
			MtnSepos = (MtnSepos & 0x000FFFFF );
			if( MencV->RevAsmMotor ^ MencV->DivInfo.RvsDir )/* Inversion connection XOR Reverse rotation mode			*/
			{
				MtnSepos = MtnSepos | 0xFFF00000;			/* Sign inversion processing							*/
				MtnSepos = (ULONG)MtnSepos - ( MtnSub << (32-Sftx) );
			}
			else
			{
				MtnSepos = (ULONG)MtnSepos + ( MtnSub << (32-Sftx) );
			}
		/*------------------------------------------------------------------------------------------*/
		/*		Initial increment pulse output data check & output data setting								*/
		/*------------------------------------------------------------------------------------------*/
			lwk = MtnSepos - MencV->DivInfo.IncPulseOut;
			if( lwk > MAXPULSEOUT )							/* Pulse output upper limit						*/
			{
				MencV->DivInfo.IncPulseOut += MAXPULSEOUT;	/* Pulse output data setting					*/
			}
			else if( lwk < -MAXPULSEOUT )					/* Pulse output lower limit						*/
			{
				MencV->DivInfo.IncPulseOut -= MAXPULSEOUT;	/* Pulse output data setting					*/
			}
			else											/* Final pulse output						*/
			{
				MencV->DivInfo.IncPulseOut = MtnSepos;		/* Pulse output data setting					*/
				MencV->DivInfo.PaoSeqMcmd = KPX_IPLSOUT_FINISH;/* Response ==> SencOutputPaoSerial		*/
				MencV->DivInfo.IncPulseReq = FALSE;			/* Initial increment pulse output completed			*/
			}
			MencV->DivInfo.DivOutPos = MencV->DivInfo.IncPulseOut;	/* Update of divided output pulse			*/
		/*------------------------------------------------------------------------------------------*/
			pAsicHwReg->AREG_POSET0	= MencV->DivInfo.IncPulseOut;	/* Incre pulse output setting		*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		No command																				*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			;												/* Waiting for command							*/
		}
/*--------------------------------------------------------------------------------------------------*/
#endif 
		return;
}
#pragma arm section code

#pragma arm section code = "CALL_SCANA" /* <S1D2> */
#if	(SVFSEL_DIVOUT_USE == TRUE)       /* <S09F> */
/****************************************************************************************************/
/*																									*/
/*		Calculating the change in multiturn data		<S02D>													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the change from the position data during pulse output and the position data during multi-rotation latch.				*/
/*			For pulse output data, check whether it exceeds the multi-turn limit value				*/
/*			checks and reflect them in the pulse output data.											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static LONG	 SencCheckMturnLimit( LONG MtnCur, MENCV *MencV )
{
LONG	MtnSub;
LONG	limmlt = MencV->DivInfo.Limmlt;

/*--------------------------------------------------------------------------------------------------*/
/*		Multiturn change amount calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
		MtnSub = (USHORT)MtnCur - (USHORT)MencV->DivInfo.MturnLatch;
		if( MtnSub < 0 )									/* If negative								*/
		{
			if( MtnSub < -((limmlt>>1) + 1) )				/* When exceeding multiturn limit,	*/
			{												/* Exceeding multiturn limit 	*/
				MtnSub += (limmlt + 1);						/* Adding multi-rotation amount						*/
			}
		}
		else												/* When positive								*/
		{
			if ( MtnSub > (limmlt>>1) )						/* When exceeding multiturn limit, 	*/
			{												/* Exceeding multiturn limit		*/
				MtnSub -= (limmlt + 1);						/* Adding multi-rotation amount						*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return ( MtnSub );
}
#pragma arm section code


/****************************************************************************************************/
/*																									*/
/*		Serial encoder PAO phase serial output			<S02D>										*/
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
/*			2) Multiturn : P±@@@@@[CR]	( ±@@@@@ : -32768 - +32768 )					*/
/*																									*/
/*			3) Alarm		: ALM@@.[CR]		( @@ : A.@@ )									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	SencOutputPaoSerial( MENCV *MencV, BPRMDAT *Bprm, ASIC_HW_IF_STR *pAsicHwReg,
							     BOOL DbBeReq, BOOL SenReq, USHORT AlmCode, BOOL AlmFlag )
{

	USHORT	uswk;
	ASIC_HW_IF_STR *pAsicHwReg2;/*<S0C2>*/
	pAsicHwReg2 = (ASIC_HW_IF_STR*)((UCHAR*)pAsicHwReg + 0x1000);/*2-axis pointing address <S0C2>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SVFSEL_PAOPOUT == 0 )		{ return;}				/* Mask Function					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Sequence initialization when servo is on																*/
/*--------------------------------------------------------------------------------------------------*/
		if( DbBeReq == TRUE )
		{
			MencV->DivInfo.PaoSeqStep = 0;
			return;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		PAO serial output sequence																*/
/*--------------------------------------------------------------------------------------------------*/
		switch( MencV->DivInfo.PaoSeqStep )
		{
	/*----------------------------------------------------------------------------------------------*/
		case  0: /* Initial Step																	*/
	/*----------------------------------------------------------------------------------------------*/
				MencV->DivInfo.PaoSeqWait = 0;
				MencV->DivInfo.PaoSeqMcmd = KPX_PAOSEQ_NOCMD;
				if( MencV->PaoseqNormal == TRUE )				/* First operation after power-on-				*/
				{
					MencV->DivInfo.PaoSeqStep = (SenReq == FALSE)? 1 : 20;
				}
				else
				{
					MencV->DivInfo.PaoSeqStep = 1;				/* First time after power-onSEN Lo->Hi	*/
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  1: /* Wait SEN Lo --> Hi (Output Multi-turn Data)										*/
	/*----------------------------------------------------------------------------------------------*/
				MencV->PaoseqNormal = TRUE;							/* Cancel initial operation mode			*/
				MencV->SenReady = FALSE;
				MencV->DivInfo.PaoSeqMcmd = KPX_PAOSEQ_NOCMD;
				MencV->DivInfo.IncPulseReq = TRUE;
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
					/*	For rotary type																*/
					/*------------------------------------------------------------------------------*/
					if( MencV->AxisMotType == MOTTYPE_ROTARY )
					{
						pAsicHwReg->AREG_WDT1SET &= ~BIT8;		/* Pulse conversionCPhase maskOFFSetting		*/
						MencV->DivInfo.DivCMaskStatus  = DIV_CMASK_OFF ;

						pAsicHwReg->AREG_POSET0	= 0x00000000;	/* Pulse conversion counter0000h	*/
						pAsicHwReg->AREG_POTEST	= 0x00000000;	/* Pulse conversion position setting0000h		*/
					}
					/*------------------------------------------------------------------------------*/
					/*	For linear type:JL-076Divide function not used(Through mode)In the case of					*/
					/*------------------------------------------------------------------------------*/
					else if( MencV->AxisMotType == MOTTYPE_LINEAR )
					{
						pAsicHwReg->AREG_POSET0	= 0xFFFFFFFF;	/* Pulse output circuit initialization				*/
						pAsicHwReg->AREG_POTEST	= 0xFFFFFFFF;	/* Pulse output circuit initialization				*/
						pAsicHwReg->AREG_DINCSET  |= BIT10;		/* Divide constant clear bit set		*/
						pAsicHwReg->AREG_WDT1SET |= BIT8;		/* Pulse conversion C phase mask ON			*/
						MencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;	/* Pulse conversion C phase mask ON	*/
					}
					MencV->DivInfo.PaoSeqStep = 2;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  2: /* Wait 120ms																		*/
	/*----------------------------------------------------------------------------------------------*/
				/*----------------------------------------------------------------------------------*/
				/*	For rotary type:																	*/
				/*----------------------------------------------------------------------------------*/
				if( MencV->AxisMotType == MOTTYPE_ROTARY )
				{
					/*------------------------------------------------------------------------------*/
					/*	POSET0to0000hFrom set2msAfterCPhase resynchronization possible set								*/
					/*------------------------------------------------------------------------------*/
					if( MencV->DivInfo.PaoSeqWait == 0 )
					{
						pAsicHwReg->AREG_DINCSET	|= BIT5;	/* C-phase resynchronization enabled						*/
						pAsicHwReg->AREG_POSET0	= 0xFFFFFFFF;	/* Pulse output circuit initialization				*/
					}
					if( ++MencV->DivInfo.PaoSeqWait > KPI_SCANC_MS( 120 ) )
					{
						pAsicHwReg->AREG_DINCSET	&= ~(BIT10);/* Divide always clear set				*/
						pAsicHwReg->AREG_DINCSET	&= ~(BIT5); /* C-phase resynchronization enabled					*/
						MencV->DivInfo.PaoSeqWait = 0;
						MencV->DivInfo.PaoSeqStep = 3;
					}
				}
				/*----------------------------------------------------------------------------------*/
				/*	For linear type: Frequency division function not used(Through mode)In the case of								*/
				/*----------------------------------------------------------------------------------*/
				else if( MencV->AxisMotType == MOTTYPE_LINEAR )
				{
					if( ++MencV->DivInfo.PaoSeqWait > KPI_SCANC_MS( 120 ) )
					{
						pAsicHwReg->AREG_DINCSET	|= BIT5;	/* C-phase resynchronization enabled						*/
						pAsicHwReg->AREG_DINCSET	&= ~(BIT10);/* Divide always clear set				*/
						pAsicHwReg->AREG_DINCSET	&= ~(BIT5); /* C-phase resynchronization enabled					*/
						MencV->DivInfo.PaoSeqWait = 0;
						MencV->DivInfo.PaoSeqStep = 3;
					}
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  3: /* Multiturn Latch Request															*/
	/*----------------------------------------------------------------------------------------------*/
				MencV->DivInfo.PaoSeqMcmd = KPX_MTLATCH_REQUEST;	/* MultiturnLatchCommand ==> ScanA	*/
				MencV->DivInfo.PaoSeqStep = 4;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  4: /* Wait Multiturn Latch															*/
	/*----------------------------------------------------------------------------------------------*/
				if( MencV->DivInfo.PaoSeqMcmd == KPX_MTLATCH_FINISH )	/* MultiturnLatchComplete?		*/
				{
					MencV->DivInfo.PaoSeqData = MencV->DivInfo.MturnLatch;
					MencV->DivInfo.PaoSeqStep = 5;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  5: /* Output 'P'																		*/
	/*----------------------------------------------------------------------------------------------*/
				/*----------------------------------------------------------------------------------*/
				/*	UARTXD:General purpose asynchronous port transmission data: General purpose asynchronous port transmission data bit 7-0   	    */
				/*----------------------------------------------------------------------------------*/
				//pAsicHwReg->AREG_UARTXD  = 'P';
				pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_P;		/* <S14F> */
				/*----------------------------------------------------------------------------------*/
				MencV->DivInfo.PaoSeqStep = 6;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case  6: /* Output '+' or '-'																*/
	/*----------------------------------------------------------------------------------------------*/
				/*----------------------------------------------------------------------------------*/
				/*	For rotary type																	*/
				/*----------------------------------------------------------------------------------*/
				if( MencV->AxisMotType == MOTTYPE_ROTARY )
				{
					if( (MencV->DivInfo.Limmlt != 65535) || ((SHORT)MencV->DivInfo.PaoSeqData >= 0) )
					{
						//pAsicHwReg->AREG_UARTXD  = '+';
						pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Puls;		/* <S14F> */
					}
					else
					{
						//pAsicHwReg->AREG_UARTXD  = '-';
						pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Mins;		/* <S14F> */
						MencV->DivInfo.PaoSeqData = -MencV->DivInfo.PaoSeqData;
					}
				}
				/*----------------------------------------------------------------------------------*/
				/*	For linear type																	*/
				/*----------------------------------------------------------------------------------*/
				else if( MencV->AxisMotType == MOTTYPE_LINEAR )
				{
					/*------------------------------------------------------------------------------*/
					/*	Reverse rotation mode																*/
					/*------------------------------------------------------------------------------*/
					if( MencV->RevAsmMotor ^ Bprm->RvsDir )			/* Inversion connection XOR Reverse rotation mode	*/
					{
						if( (SHORT)MencV->DivInfo.PaoSeqData >= 0 )
						{
							//pAsicHwReg->AREG_UARTXD  = '-';
							pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Mins;		/* <S14F> */
							MencV->DivInfo.PaoSeqData = MencV->DivInfo.PaoSeqData + 1;
						}
						else
						{
							//pAsicHwReg->AREG_UARTXD  = '+';
							pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Puls;		/* <S14F> */
							MencV->DivInfo.PaoSeqData = ~MencV->DivInfo.PaoSeqData;
						}
					}
					/*------------------------------------------------------------------------------*/
					/*	Positive rotation mode																*/
					/*------------------------------------------------------------------------------*/
					else
					{
						if( (SHORT)MencV->DivInfo.PaoSeqData >= 0 )
						{
							//pAsicHwReg->AREG_UARTXD  = '+';
							pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Puls;		/* <S14F> */
						}
						else
						{
							//pAsicHwReg->AREG_UARTXD  = '-';
							pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Mins;		/* <S14F> */
							MencV->DivInfo.PaoSeqData = ~MencV->DivInfo.PaoSeqData + 1;
						}
					}
				}
				MencV->DivInfo.PaoSeqStep = 7;
				break;
	/*----------------------------------------------------------------------------------------------*/
	/*			Decimal data output : 5th digit,4th digit,3rd digit,2nd digit,1st digit								*/
	/*----------------------------------------------------------------------------------------------*/
		case  7: SencOutputPaoMtdata( MencV, pAsicHwReg, 5 ); MencV->DivInfo.PaoSeqStep =  8; break;
		case  8: SencOutputPaoMtdata( MencV, pAsicHwReg, 4 ); MencV->DivInfo.PaoSeqStep =  9; break;
		case  9: SencOutputPaoMtdata( MencV, pAsicHwReg, 3 ); MencV->DivInfo.PaoSeqStep = 10; break;
		case 10: SencOutputPaoMtdata( MencV, pAsicHwReg, 2 ); MencV->DivInfo.PaoSeqStep = 11; break;
		case 11: SencOutputPaoMtdata( MencV, pAsicHwReg, 1 ); MencV->DivInfo.PaoSeqStep = 12; break;
	/*----------------------------------------------------------------------------------------------*/
		case 12: /* Output '0x0D'																	*/
	/*----------------------------------------------------------------------------------------------*/
				//pAsicHwReg->AREG_UARTXD  = (0x0d + 0x80);
				pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Cr;		/* <S14F> */
				MencV->DivInfo.PaoSeqStep = 13;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 13: /* Wait 2ms																		*/
	/*----------------------------------------------------------------------------------------------*/
				MencV->DivInfo.PaoSeqStep = 14;
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
				MencV->DivInfo.PaoSeqMcmd = KPX_IPLSOUT_REQUEST;/* Initial incremental output command ==> ScanA	*/
				MencV->DivInfo.PaoSeqStep = 15;
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 15: /* Check IncPulse Output Complete													*/
	/*----------------------------------------------------------------------------------------------*/
				if( MencV->DivInfo.PaoSeqMcmd == KPX_IPLSOUT_FINISH )	/* Initial increment output completed?	*/
				{
					/*------------------------------------------------------------------------------*/
					/*	For rotary type																*/
					/*------------------------------------------------------------------------------*/
					if( MencV->AxisMotType == MOTTYPE_ROTARY )
					{
						if( MencV->EncDisable == FALSE )
						{
							MencV->SenReady = TRUE;
							MencV->ReCalcAbsPosReq = TRUE;		/* Absolute position recalculation request set			*/
							SencMakeAbsPosition( MencV, 		/* Multi-rotation amount and1Obtaining internal rotation pulse number*/
												 MencV->DivInfo.Limmlt,
												 MencV->DivInfo.Bit_dp,
												 Bprm->RvsDir );
						}
					}
					/*------------------------------------------------------------------------------*/
					/*	For linear type																*/
					/*------------------------------------------------------------------------------*/
					else if( MencV->AxisMotType == MOTTYPE_LINEAR )
					{
						if( MencV->EncDisable == FALSE )
						{
							MencV->SenReady = TRUE;
							LmxSencGetLinearAbsPos( MencV, Bprm->RvsDir );/* Absolute position data acquisition	*/
						}
						pAsicHwReg->AREG_WDT1SET &= ~BIT8;		/* Pulse conversion C phase mask OFF  			*/
						MencV->DivInfo.DivCMaskStatus  = DIV_CMASK_OFF ;
					}
					/*------------------------------------------------------------------------------*/
					MencV->DivInfo.PaoSeqStep = 0;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 20: /* Wait SEN Hi --> Lo (Output Alarm)												*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					MencV->SenReady = FALSE;
					MencV->DivInfo.PaoSeqMcmd = KPX_PAOSEQ_NOCMD;
					MencV->DivInfo.IncPulseReq = TRUE;
					if( AlmFlag )								/* Alarm occurrence					*/
					{
						pAsicHwReg->AREG_PFNCSET  |= BIT13;		/* DA output mode: TXD output		*/
						pAsicHwReg->AREG_POSET0	= 0xFFFFFFFF;	/* Pulse output circuit initialization				*/
						pAsicHwReg->AREG_POTEST	= 0xFFFFFFFF;	/* Pulse output circuit initialization				*/

						MencV->DivInfo.PaoSeqData = AlmCode;	/* Alarm code acquisition				*/
						MencV->DivInfo.PaoSeqStep = 21;			/* Transition to alarm output step		*/
					}
					else										/* No alarm occurrence					*/
					{
						MencV->DivInfo.PaoSeqStep =  1;			/* Lo-->HiTransition to monitoring step		*/
					}
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 21: /* Wait 120ms																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					if( ++MencV->DivInfo.PaoSeqWait > KPI_SCANC_MS( 120 ) )
					{
						MencV->DivInfo.PaoSeqWait = 0;
						MencV->DivInfo.PaoSeqStep = 22;
					}
				}
				else
				{
					MencV->DivInfo.PaoSeqWait = 0;
					MencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 22: /* Output 'A'																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					//pAsicHwReg->AREG_UARTXD  = 'A';
					pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_A;		/* <S14F> */
					MencV->DivInfo.PaoSeqStep = 23;
				}
				else
				{
					MencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 23: /* Output 'L'																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					//pAsicHwReg->AREG_UARTXD  = ('L' + 0x80);
					pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_L;		/* <S14F> */
					MencV->DivInfo.PaoSeqStep = 24;
				}
				else
				{
					MencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 24: /* Output 'M'																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					//pAsicHwReg->AREG_UARTXD  = 'M';
					pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_M;		/* <S14F> */
					MencV->DivInfo.PaoSeqStep = 25;
				}
				else
				{
					MencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 25: /* Output Alarm Code																*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					uswk = MlibEvenAsciiTbl[((MencV->DivInfo.PaoSeqData >> 8) & 0x0F)];
					//pAsicHwReg->AREG_UARTXD  = uswk;
					pAsicHwReg->AREG_UARTXD  = MlibEvenAsciiTbl[ uswk ];		/* <S14F> */
					MencV->DivInfo.PaoSeqStep = 26;
				}
				else
				{
					MencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 26: /* Output Alarm Code																*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					uswk = MlibEvenAsciiTbl[((MencV->DivInfo.PaoSeqData>> 4) & 0x0F)];
					//pAsicHwReg->AREG_UARTXD  = uswk;
					pAsicHwReg->AREG_UARTXD  = MlibEvenAsciiTbl[ uswk ];		/* <S14F> */
					MencV->DivInfo.PaoSeqStep = 27;
				}
				else
				{
					MencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 27: /* Output '.'																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					//pAsicHwReg->AREG_UARTXD  = '.';
					pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Prio;		/* <S14F> */
					MencV->DivInfo.PaoSeqStep = 28;
				}
				else
				{
					MencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 28: /* Output 0x0D																		*/
	/*----------------------------------------------------------------------------------------------*/
				if( SenReq == FALSE )
				{
					//pAsicHwReg->AREG_UARTXD  = (0x0d + 0x80);
					pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Cr;		/* <S14F> */
					MencV->DivInfo.PaoSeqStep = 29;
				}
				else
				{
					MencV->DivInfo.PaoSeqStep = 1;
				}
				break;
	/*----------------------------------------------------------------------------------------------*/
		case 29: /* End of Output Alarm																*/
	/*----------------------------------------------------------------------------------------------*/
				pAsicHwReg->AREG_PFNCSET  &= ~BIT13;			/* DA output mode: Pulse output		*/
				MencV->DivInfo.PaoSeqStep = 1;
				break;
	/*----------------------------------------------------------------------------------------------*/
		default: /* Default																			*/
	/*----------------------------------------------------------------------------------------------*/
				MencV->DivInfo.PaoSeqStep = 0;
				break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;

}

/****************************************************************************************************/
/*																									*/
/*		PAO phase multiturn data output						<S02D>									*/
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
static void SencOutputPaoMtdata( MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg, ULONG digit )
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
		x = MencV->DivInfo.PaoSeqData / digit;
		outx = MlibEvenAsciiTbl[ x ];
		pAsicHwReg->AREG_UARTXD  = (USHORT)outx;
		MencV->DivInfo.PaoSeqData -= (USHORT)(x * digit);
/*--------------------------------------------------------------------------------------------------*/
		return;

}

/****************************************************************************************************/
/*																									*/
/*		Linear motor: Absolute position data(64bit)Acquisition														*/
/*																									*/
/****************************************************************************************************/
static void LmxSencGetLinearAbsPos( MENCV  *MencV, BOOL RvsDir )
{
ULONG	PosL;
USHORT	PosH;
LONG	AbsPos[2];
UCHAR	Sftx = LMX_MPOS_SFT;

/*--------------------------------------------------------------------------------------------------*/
/*		Encoder data latch																		*/
/*--------------------------------------------------------------------------------------------------*/
		PosH = MencV->RxPosH[0];						/* Use the data read in scan A,	*/
		PosL = MencV->RxPosL[0].dw;						/* Absolute position(36Bit).				*/

		AbsPos[1] = (LONG)((SHORT)PosH) >> Sftx;		
		AbsPos[0] = (PosH<<(32-Sftx))+((ULONG)PosL>>Sftx);

		/*------------------------------------------------------------------------------------------*/
		if( MencV->RevAsmMotor ^ RvsDir )				/* Inversion connection XOR Reverse rotation mode				*/
		{
			AbsPos[1] = (~AbsPos[1]);					/* Sign inversion processing								*/
			AbsPos[0] = (~AbsPos[0]) + 1;				/* Sign inversion processing								*/
			if( AbsPos[0] == 0 ){ ++AbsPos[1];}			/* Carry, go to the top+1					*/
		}

		MencV->SenOnScalePosH = AbsPos[1];
		MencV->SenOnScalePosL = AbsPos[0];
/*--------------------------------------------------------------------------------------------------*/
		return;
}

#if	(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)
//The Fulk-related processing below is provisional and needs to be moved to a Fulk-specific file.<S02D>
/****************************************************************************************************/
/*																									*/
/*		Fulk encoder : Calculation of divided output pulse frequency upper limit speed							<S02D>		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the rotation speed at which the frequency division output pulse has the maximum frequency.									*/
/*																									*/
/*			Pulse frequency upper limit(SVCDEF_DIVOUT_MAXFREQ) :												*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*			MaxFreq [pps] = pgrate [pulse/rev] * DivOvrSpd [rev/s]									*/
/*																									*/
/*									       MaxFreq * 2^24											*/
/*			DivOvrSpd[2^24/OvrSpd] = ----------------------------	 								*/
/*									  pgrate  * (OvrSpd / 2*PAI) 									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	DIVSET_ERROR FencSencCalMencDivOverSpeed( MENCV *FencV, UNI_PRM_STR *UniPrms )
{
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG	kx, sx;
#endif /* FLOAT_USE */
LONG	pgrate;													/* [pulse/rev]						*/
LONG	i;
LONG	BitNo = UniPrms->Prm->MencP.BitnoDpoint.b.l;
LONG	MaxFreq = SVCDEF_DIVOUT_MAXFREQ;						/* [pulse/sec]						*/
LONG	pulsno = 1;
DIVSET_ERROR	err;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of division output pulse rate [pulse/rev]														*/
/*--------------------------------------------------------------------------------------------------*/
		pgrate = (((FencV->PulseNo >> BitNo) * UniPrms->Prm->scalepgrat) >> 2) - 1;
/*--------------------------------------------------------------------------------------------------*/
/*		Division upper limit speed reference value calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
		for ( i = 0; pgrate > 0; i++ )
		{
			pgrate = pgrate / 2;
			pulsno = pulsno * 2;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of upper limit speed of divided output pulse frequency															*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
		fw = (float)MaxFreq * (float)0x01000000 / (float)pulsno;
		fw = fw / OvrSpd;
		fw = fw * 62832.0F / 10000.0F;
		if( fw > (float)0x01000000 )
		{
			FencV->DivInfo.DivOvrSpd = 0x01000000;
		}
		else
		{
			FencV->DivInfo.DivOvrSpd = (LONG)fw;
		}
#else
		kx = MlibScalKxgain( MaxFreq, 0x01000000, pulsno, &sx,   0 );
		kx = MlibPcalKxkxks( kx,      1,     UniPrms->Bprm->OvrSpd, &sx,   0 );
		kx = MlibPcalKxgain( kx,      62832,      10000,  &sx, -24 );
		FencV->DivInfo.DivOvrSpd = kx;
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter setting error																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( FencV->DivInfo.DivOvrSpd == 0 )
		{
			err = DIVSET_PRMERR_L;								/* A.040 : Setting parameter error	*/
		}
/*--------------------------------------------------------------------------------------------------*/
	return( err );
}

/****************************************************************************************************/
/*																									*/
/*		Fulk encoder : Calculation of divided output pulse & ASIC setting					<S02D>				*/
/*																									*/
/****************************************************************************************************/
static DIVSET_ERROR	FencSencSetMencDividedPulse( ASIC_HW_IF_STR *AsicHwReg, MENCV *FencV, UNI_PRM_STR *UniPrms )
{
LONG	x;
LONG	BitNo = UniPrms->Prm->MencP.BitnoDpoint.b.l;
LONG	pulsno;
LONG	pgrate = UniPrms->Prm->scalepgrat;
DIVSET_ERROR	err;

/*--------------------------------------------------------------------------------------------------*/
/*		Division output pulse upper limit value setting																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( BitNo > 0 )
		{
			pulsno = (1<<BitNo);
		}
		else
		{
			pulsno = pndef_scalepgrat.UpperLimit;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of upper limit speed of divided output pulse frequency															*/
/*--------------------------------------------------------------------------------------------------*/
		err = FencSencCalMencDivOverSpeed( FencV, UniPrms );

/*--------------------------------------------------------------------------------------------------*/
/*		Checking PG division ratio setting range																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( (pgrate > pulsno) || (pgrate < 1) )				/* pgrate : range check				*/
		{
			return ( DIVSET_DIVERR );						/* A.041 : Division pulse output setting error		*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of frequency division pulse output gain																	*/
/*--------------------------------------------------------------------------------------------------*/
		FencV->DivInfo.DivMode = FALSE;
		FencV->DivInfo.DivPass = TRUE;
		FencV->DivInfo.DivRate = 0;
		FencV->DivInfo.DivOutGain = MlibScalKxgain( pgrate, 1, (1 << BitNo), NULL, 24 );
		FencV->DivInfo.DivOutCmskChk = MlibScalKxkxks( 0x1000, 1, FencV->DivInfo.DivOutGain, NULL, -30 );
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC frequency division function setting																		*/
/*--------------------------------------------------------------------------------------------------*/
		x = (FencV->DivInfo.DivMode << 4) | FencV->DivInfo.DivPass | 0x0400;
		AsicHwReg->AREG_DINCSET	= (ULONG)x;						     /* Division function setting				*/
		AsicHwReg->AREG_DIVCSEV	= (ULONG)(FencV->DivInfo.DivRate);  /* Division ratio setting					*/
		AsicHwReg->AREG_DCTRIM  = (ULONG)FencV->DivInfo.DivRate - 1;/* Division output origin correction			*/
		if( UniPrms->Bprm->RvsDir )
		{
			AsicHwReg->AREG_DINCSET |= BIT2;						/* DBReverse mode setting				*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return (err);

}

/****************************************************************************************************/
/*																									*/
/*		Fulk encoder : Divided pulse IF initialization														*/
/*																									*/
/****************************************************************************************************/
DIVSET_ERROR FencSencInitDivPulseIF( ASIC_HW_IF_STR* AsicHwReg, MENCV *FencV, UNI_PRM_STR *UniPrms )
{
	DIVSET_ERROR	err;

/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder pulse conversion setting(BitNumber,Conversion cycle) @JL-056Equivalent setting @						*/
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_PLSET	= 0x00003400;				/* ConvertBitNumber:16bit, Conversion cycle:64us			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder pulse conversion setting(Position,Origin,etc)												*/
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_WDT1SET	|=	BIT8;
	FencV->DivInfo.DivCMaskStatus = DIV_CMASK_ON;
/*--------------------------------------------------------------------------------------------------*/
/*	DIVSET:Division function setting																				*/
/*--------------------------------------------------------------------------------------------------*/
/*	VULONG	AREG_DIVSET;		 0x0164 Division function setting												*/
								/*------------------------------------------------------------------*/
								/*  bit.1-0 :Data pulse conversion position data write selection						*/
								/*            0: SerialPGIFFrom channel 0 (Initial value)						*/
								/*  		  1: SerialPGIFFrom channel 1								*/
								/*  		  2: HostCPUto											*/
								/*  		  3: From built-in calculation unit										*/
								/*------------------------------------------------------------------*/
								/*  bit.2 : Divide clock selection											*/
								/*            0:26.7MHz (Initial value)									*/
								/*  		  1:32MHz												*/
								/*------------------------------------------------------------------*/
								/*  bit.4-3 :Pulse conversion origin correction value setting source selection						*/
								/*            0: SerialPGIFFrom channel 0 (Initial value)						*/
								/*  		  1: SerialPGIFFrom channel 1								*/
								/*  		  2: HostCPUto											*/
								/*  		  3: From built-in calculation unit										*/
								/*------------------------------------------------------------------*/
								/*  bit.8 : DIVSEL0													*/
								/*            0:PA,PB,PC				 							*/
								/*  		  1: Data pulse conversion output (initial value)							*/
								/*------------------------------------------------------------------*/
								/*  bit.11-10 :DCSEL												*/
								/*            0: NormalCPhase (Initial value)									*/
								/*  		  1: Frequency divisionCPhase											*/
								/*  		  2: Frequency divisionCPhase2											*/
								/*  		  3: Frequency divisionCPhase2											*/
								/*------------------------------------------------------------------*/
								/*  bit.13 : DIVASEL												*/
								/*            0:DAOutput (Initial value)									*/
								/*  		  1: Serial data through output									*/
								/*------------------------------------------------------------------*/
								/*  bit.15 : DIVCBIT												*/
								/*            0:DCOutput (Initial value)									*/
								/*  		  1:DCHigh level forced output									*/
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_DIVSET	= 0x00000112;  				/* Pulse conversion write side setting:HostCPU,				*/
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_POSET0	= 0xFFFFFFFF;				/* Pulse conversion position setting						*/
	AsicHwReg->AREG_POSET1	= 0x00000000;				/* Pulse conversion origin correction 1						*/
	AsicHwReg->AREG_POSET2	= 0xFFFFFFFF;				/* Pulse conversion origin correction 2						*/
	AsicHwReg->AREG_POTEST	= 0xFFFFFFFF;				/* Pulse conversion counter setting					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Division input filter setting																		*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*	FBCSET:Feedback pulse counter																		*/
/*--------------------------------------------------------------------------------------------------*/
/*	VULONG	AREG_FBCSET;	     0x0040 Division input filter setting										*/
								/*------------------------------------------------------------------*/
								/*  bit.1-0 :PA,PBFilter selection											*/
								/*            0:26.7MHzSynchronization (Initial value)								*/
								/*  		  1:37.5ns												*/
								/*  		  2:75ns												*/
								/*  		  3: Clear												*/
								/*------------------------------------------------------------------*/
								/*  bit.3-2 : PCFilter selection											*/
								/*            0:26.7MHzSynchronization (Initial value)								*/
								/*  		  1:37.5ns												*/
								/*  		  2:75ns												*/
								/*  		  3: Clear												*/
								/*------------------------------------------------------------------*/
								/*  bit.5-15 : Omitted													*/
/*--------------------------------------------------------------------------------------------------*/
	AsicHwReg->AREG_FBCSET	= 0x00000005;				/* Division input filter setting						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Divided pulse calculation & ASIC settings																*/
/*--------------------------------------------------------------------------------------------------*/
	err = FencSencSetMencDividedPulse( AsicHwReg, FencV, UniPrms );
/*--------------------------------------------------------------------------------------------------*/

	return( err );
}
#endif	/*(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)*/
#endif	/*(SVFSEL_DIVOUT_USE == TRUE)*/




/*<S189>_Start*//*<S0C2>*/
#define   PS0_Use   /*PS0 Use*/
#ifdef    PS0_Use
/*********************************************************************************/
/*Explanation: This is a function for PSO data output , used by Full Closed loop*/
/*********************************************************************************/
void FencSencOutputPsoSerial(
								MENCV*           FencV,      /*Incremental */
                                ASIC_HW_IF_STR*  pAsicHwReg, /*Register AREG_UARTXD  Selection*/
                                SHORT limmlt,    /*Pn205 Set*/
                                BOOL  RvsDir     /*Reverse rotation mode judgment*/
                             )
{
		ASIC_HW_IF_STR *pAsicHwReg2;
		pAsicHwReg2 = (ASIC_HW_IF_STR*)((UCHAR*)pAsicHwReg + 0x1000);/*2-axis pointing address*/
		if(FencV->SenReady == FALSE)  /*waiting for the SenReady time*/
		{
			FencV->DivInfo.PsoSeqStep = 0;
			return;
		}
		if(FencV->EncReady==FALSE)/* if Encoder Ready==FALSE Retrun*/
		{
			FencV->DivInfo.PsoSeqStep = 0;
			return;
		}
	  switch( FencV->DivInfo.PsoSeqStep )/*Step One by One*/
	  {
		case  0: /* Initial Step*/
				FencV->DivInfo.PsoSeqWait = 0;
				FencV->DivInfo.PsoSeqStep = 1;
		break;
		case  1: /* Get  Multiturn  and Incremental Data*/
				 FencV->DivInfo.PsoLinearData_H=  (USHORT)FencV->RxPosH[0];
				 FencV->DivInfo.PsoLinearData_L = ((ULONG)FencV->RxPosL[0].dw)>>FencV->MposSftR;/*Linear Value*/
				 FencV->DivInfo.PsoSeqStep = 2;
		break;
		case  2: /*Output 'P'*/
				pAsicHwReg->AREG_PFNCSET &= ~BIT13; /*Output Data Set DA2*/
				pAsicHwReg2->AREG_PFNCSET|= BIT13;  /*Output Data Set PSO  Register Selection*/
				pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_P;		/* <S14F> P*/
				FencV->DivInfo.PsoSeqStep = 3;
		break;
		case 3 : /* Output '+' or '-'*/
				/*----------------------------------------------------------------------------------*/
				/*	Reverse rotation mode																	    */
				/*----------------------------------------------------------------------------------*/
				if( FencV->DivInfo.RvsDir )
				{
					if( (SHORT)FencV->DivInfo.PsoMturnLatch >= 0 )
					{
						pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Mins;		/* <S14F>  -*/
					}
					else
					{
						pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Puls;		/* <S14F>  +*/
					}
				}
				/*----------------------------------------------------------------------------------*/
				/*	Positive rotation mode																	*/
				/*----------------------------------------------------------------------------------*/
				else
				{
					if( (SHORT)FencV->DivInfo.PsoMturnLatch >= 0 )
					{
						pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Puls;		/* <S14F>  +*/
					}
					else
					{
						pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Mins;		/* <S14F>  -*/
					}
				}
				FencV->DivInfo.PsoSeqStep = 4;
		break;
/* <S20A>: Review argument:[MencV->DivInfo.PsoLinearData_*]as pointer */
		case 4:  OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_H, 5, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  5; break;
		case 5:  OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_H, 4, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  6; break;
		case 6:  OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_H, 3, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  7; break;
		case 7:  OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_H, 2, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  8; break;
		case 8:  OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_H, 1, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  9; break;
		case 9:  OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_L, 6, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  10; break;
		case 10: OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_L, 5, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  11; break;
		case 11: OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_L, 4, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  12; break;
		case 12: OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_L, 3, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  13; break;
		case 13: OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_L, 2, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  14; break;
		case 14: OutputPsoLinearIncre((LONG*)&FencV->DivInfo.PsoLinearData_L, 1, pAsicHwReg); FencV->DivInfo.PsoSeqStep=  15; break;
/* <S20A>: So far */
		case 15: /* Output 0x0D   CR*/
				 pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Cr;		/* <S14F> 0x0d*/
				 FencV->DivInfo.PsoSeqStep = 16;break;
		case 16: /* Wait 2ms*/
				 FencV->DivInfo.PsoSeqStep = 17;break;
		case 17: /* Wait 2ms*/
				 FencV->DivInfo.PsoSeqStep = 18;break;
		case 18: /* Wait 2ms*/
				FencV->DivInfo.PsoSeqStep = 19;break;
		case 19: /* Wait 2ms*/
				FencV->DivInfo.PsoSeqStep = 20;break;
		case 20: /* End Step*/
				FencV->DivInfo.PsoSeqStep = 1;break;
		default: /* Default Step*/
				FencV->DivInfo.PsoSeqStep = 0;break;
 }/* End if MencV->AxisMotType == MOTTYPE_LINEAR*/
}
/*********************************************************************************/
/*Explanation: This is a function for PSO data output , used by Motor Closed loop*/
/*********************************************************************************/
void SencOutputPsoSerial(MENCV* MencV,               /*Multiturn  and Incremental step*/
		                 ASIC_HW_IF_STR* pAsicHwReg, /*Register AREG_UARTXD  Selection*/
		                 SHORT limmlt,                /*Pn205 Set  Multiturn limit*/
		                 BOOL  RvsDir)                /*Reverse rotation mode is determined*/
{
	ASIC_HW_IF_STR *pAsicHwReg2;
	pAsicHwReg2 = (ASIC_HW_IF_STR*)((UCHAR*)pAsicHwReg + 0x1000);/*2-axis pointing address*/
	if(MencV->SenReady==FALSE)  /*waiting for the SenReady time*/
	{
		MencV->DivInfo.PsoSeqStep = 0;
		return;
	}
	if(MencV->EncReady==FALSE)/* if Encoder Ready==FALSE Retrun*/
	{
		MencV->DivInfo.PsoSeqStep = 0;
		return;
	}
	/* MOTTYPE_ROTARY */
   if(MencV->AxisMotType == MOTTYPE_ROTARY)
   {

	   switch( MencV->DivInfo.PsoSeqStep )/*Step One by One*/
		{
			case  0: /* Initial Step*/
					MencV->DivInfo.PsoSeqWait = 0;
					MencV->DivInfo.PsoSeqStep = 1;
			break;
			case  1: /* Get  Multiturn  and Incremental Data	*/
					MencV->DivInfo.PsoMturnLatch= SencGetMturnData(MencV,MencV->DivInfo.Limmlt, MencV->DivInfo.Bit_dp,(USHORT)MencV->RxPosH[0],MencV->RxPosL[0].dw );
					MencV->DivInfo.PsoIncPulseOut = ((ULONG)MencV->RcvPosX[0])>>MencV->MposSftR;/*Determine the decimal point position*/
					MencV->DivInfo.PsoSeqStep = 2;
			break;
			case  2: /*Output 'P'*/
				    pAsicHwReg->AREG_PFNCSET &= ~BIT13; /*Output Data Set DA2*/
				    pAsicHwReg2->AREG_PFNCSET|= BIT13;  /*Output Data Set PSO Register Selection*/
				    /*SGDVUninstalled processing,Σ7Additional register processing*/
					pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_P;		/* <S14F> P*/
					MencV->DivInfo.PsoSeqStep = 3;
			break;
			case 3 : /* Output '+' or '-'*/
					if( (limmlt != 65535)/*Pn205 Set*/
						|| ((SHORT)MencV->DivInfo.PsoMturnLatch >= 0) )
					{
						pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Puls;/* <S14F> + */
					}
					else
					{
						pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Mins;/* <S14F>  - */
						MencV->DivInfo.PsoMturnLatch = -MencV->DivInfo.PsoMturnLatch;
					}
					MencV->DivInfo.PsoSeqStep = 4;
			break;
		    /*Multi-rotation data*/
			case  4: OutputPsoMturnIncre(MencV, 5, 0,pAsicHwReg); MencV->DivInfo.PsoSeqStep =  5; break;
			case  5: OutputPsoMturnIncre(MencV, 4, 0,pAsicHwReg); MencV->DivInfo.PsoSeqStep =  6; break;
			case  6: OutputPsoMturnIncre(MencV, 3, 0,pAsicHwReg); MencV->DivInfo.PsoSeqStep =  7; break;
			case  7: OutputPsoMturnIncre(MencV, 2, 0,pAsicHwReg); MencV->DivInfo.PsoSeqStep =  8; break;
			case  8: OutputPsoMturnIncre(MencV, 1, 0,pAsicHwReg); MencV->DivInfo.PsoSeqStep =  9; break;
			case  9: pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Conm; MencV->DivInfo.PsoSeqStep =  10; break;	/* <S147> , */
		     /*One-rotation position data*/
			case 10: OutputPsoMturnIncre(MencV, 8, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 11; break;
			case 11: OutputPsoMturnIncre(MencV, 7, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 12; break;
			case 12: OutputPsoMturnIncre(MencV, 6, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 13; break;
			case 13: OutputPsoMturnIncre(MencV, 5, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 14; break;
			case 14: OutputPsoMturnIncre(MencV, 4, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 15; break;
			case 15: OutputPsoMturnIncre(MencV, 3, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 16; break;
			case 16: OutputPsoMturnIncre(MencV, 2, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 17; break;
			case 17: OutputPsoMturnIncre(MencV, 1, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 18; break;
			case 18: /* Output 0x0D*/
					 pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Cr;/* <S147> 0x0D */
					 MencV->DivInfo.PsoSeqStep = 19;break;
			case 19: /* Wait 2ms*/
				     MencV->DivInfo.PsoSeqStep = 20;break;
			case 20: /* End Step*/
				     MencV->DivInfo.PsoSeqStep = 1;break;
			default: /* Default Step*/
				     MencV->DivInfo.PsoSeqStep = 0;break;
	 }/*End MencV.PsoSeqStep*/
  }/*End (MencV->AxisMotType == MOTTYPE_ROTARY)*/
/*-----------------------------------------------------------------------------------------------------------------------*/
 /* MOTTYPE_LINEAR */
	 if(MencV->AxisMotType == MOTTYPE_LINEAR)
	 {
		  switch( MencV->DivInfo.PsoSeqStep )/*Step One by One*/
		  {
			case  0: /* Initial Step*/
					MencV->DivInfo.PsoSeqWait = 0;
					MencV->DivInfo.PsoSeqStep = 1;
			break;
			case  1: /* Get  Multiturn  and Incremental Data	*/
				    MencV->DivInfo.PsoMturnLatch= SencGetMturnData(MencV,MencV->DivInfo.Limmlt, MencV->DivInfo.Bit_dp,(USHORT)MencV->RxPosH[0],MencV->RxPosL[0].dw );
				    MencV->DivInfo.PsoLinearData_H=  (USHORT)MencV->RxPosH[0];/* DATA_High*/
				    MencV->DivInfo.PsoLinearData_L = ((ULONG)MencV->RxPosL[0].dw)>>MencV->MposSftR;/*DATA_LOW*/
					MencV->DivInfo.PsoSeqStep = 2;
			break;
			case  2: /*Output 'P'*/
				     pAsicHwReg->AREG_PFNCSET &= ~BIT13; /*Output Data Set DA2*/
					 pAsicHwReg2->AREG_PFNCSET|= BIT13;  /*Output Data Set PSO Register Selection*/
					 /*SGDVUninstalled processing,Σ7Additional register processing*/
					 pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_P;/* <S147> P*/
					 MencV->DivInfo.PsoSeqStep = 3;
			break;
			case 3 : /* Output '+' or '-'*/
				    /*------------------------------------------------------------------------------*/
					/*	Reverse rotation mode																*/
					/*------------------------------------------------------------------------------*/
					if( MencV->RevAsmMotor ^ RvsDir)	/* Inversion connection XOR Reverse rotation mode	*/
					{
						if( (SHORT)MencV->DivInfo.PsoMturnLatch >= 0 )
						{
							pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Mins;/*-*/
						}
						else
						{
							pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Puls;/*+*/
						}
					}
					/*------------------------------------------------------------------------------*/
					/*	Positive rotation mode																    */
					/*------------------------------------------------------------------------------*/
					else
					{
						if( (SHORT)MencV->DivInfo.PsoMturnLatch >= 0 )
						{
							pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Puls;/*+*/
						}
						else
						{
							pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Mins;/*-*/
						}
					}
					MencV->DivInfo.PsoSeqStep = 4;
			break;
/* <S20A>: Review argument:[MencV->DivInfo.PsoLinearData_*]as pointer */
			case 4:  OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_H, 5, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 5; break;
			case 5:  OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_H, 4, pAsicHwReg); MencV->DivInfo.PsoSeqStep= 6; break;
			case 6:  OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_H, 3, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  7; break;
			case 7:  OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_H, 2, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  8; break;
			case 8:  OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_H, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  9; break;
			case 9:  OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_L, 6, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  10; break;
			case 10: OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_L, 5, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  11; break;
			case 11: OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_L, 4, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  12; break;
			case 12: OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_L, 3, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  13; break;
		    case 13: OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_L, 2, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  14; break;
			case 14: OutputPsoLinearIncre((LONG*)&MencV->DivInfo.PsoLinearData_L, 1, pAsicHwReg); MencV->DivInfo.PsoSeqStep=  15; break;
/* <S20A>:Up to here */
			case 15: /* Output 0x0D   CR*/
					 pAsicHwReg->AREG_UARTXD  = EvenAsciiCode_Cr; /* <S147>  0x0D */
					 MencV->DivInfo.PsoSeqStep = 16;break;
			case 16: /* Wait 2ms*/
				     MencV->DivInfo.PsoSeqStep = 17;break;
			case 17: /* Wait 2ms*/
				     MencV->DivInfo.PsoSeqStep = 18;break;
			case 18: /* Wait 2ms*/
				     MencV->DivInfo.PsoSeqStep = 19;break;
			case 19: /* Wait 2ms*/
				     MencV->DivInfo.PsoSeqStep = 20;break;
			case 20: /* End Step*/
				     MencV->DivInfo.PsoSeqStep = 1;break;
			default: /* Default Step*/
				     MencV->DivInfo.PsoSeqStep = 0;break;
		  }/*End MencV.PsoSeqStep*/
	 }/* End if(MencV->AxisMotType == MOTTYPE_LINEAR)*/
}

/************************************************************************************************/
/* Linear Get Incremental Data                                                                	*/
/************************************************************************************************/
static void	OutputPsoLinearIncre(LONG *PsoIncPulseOut, ULONG digit, ASIC_HW_IF_STR *pAsicHwReg)	/* <S20A>:Argument review */
{
	 long	x;
	 long	outx;
/*--------------------------------------------------------------------------------------------------*/
/*		Convert digit data																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( digit )
		{
			case  1: digit =  1;        break;
			case  2: digit =  10;       break;
			case  3: digit =  100;      break;
			case  4: digit =  1000;     break;
			case  5: digit =  10000;    break;
			case  6: digit =  100000;   break;
			default: break;
		}
/*		Output Incremental data		*/
/*--------------------------------------------------------------------------------------------------*/
	x = *PsoIncPulseOut/digit;	/* <S20A> */
	outx = MlibEvenAsciiTbl[ x ];		/* <S14F> */
	pAsicHwReg->AREG_UARTXD = (UCHAR)outx; /*Send the Data*/
	*PsoIncPulseOut -= (x * digit);	/* <S20A> */
}


/************************************************************************************************/
/*Get Multiturn and Incremental Data*/
/****************************************************************************************************/
static void	OutputPsoMturnIncre( MENCV *MencV,ULONG digit, ULONG outsel,ASIC_HW_IF_STR* pAsicHwReg)
{
	ULONG	x;
	ULONG	outx;
/*--------------------------------------------------------------------------------------------------*/
/*		Convert digit data	*/
/*--------------------------------------------------------------------------------------------------*/
		switch( digit )
		{
			case  1: digit = 1;        break;
			case  2: digit = 10;       break;
			case  3: digit = 100;      break;
			case  4: digit = 1000;     break;
			case  5: digit = 10000;    break;
			case  6: digit = 100000;   break;
			case  7: digit = 1000000;  break;
			case  8: digit = 10000000; break;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Output Multiturn data	 */
/*--------------------------------------------------------------------------------------------------*/
		if( outsel == 0 )
		{
			x = MencV->DivInfo.PsoMturnLatch/digit;
			outx =MlibEvenAsciiTbl[x];		/* <S14F> */
			pAsicHwReg->AREG_UARTXD = (UCHAR)outx;
			/*Send the Data SGDVUninstalled processing,Σ7Additional register processing*/
			MencV->DivInfo.PsoMturnLatch -= (USHORT)(x * digit);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Output Incremental data		*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			x = MencV->DivInfo.PsoIncPulseOut/digit;
			outx =MlibEvenAsciiTbl[ x ];		/* <S14F> */
			pAsicHwReg->AREG_UARTXD = (UCHAR)outx; /*Send the Data*/
			/*Send the Data SGDVUninstalled processing,Σ7Additional register processing*/
			MencV->DivInfo.PsoIncPulseOut -= (x * digit);
		}
}
#endif
/*End <S189>*/
/***************************************** end of file **********************************************/

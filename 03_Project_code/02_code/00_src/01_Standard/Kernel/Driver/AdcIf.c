/****************************************************************************************************/
/*																									*/
/*																									*/
/*		AdcIf.c : A/D Converter Interface															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function : A/D Converter Interface																*/
/*																									*/
/*			1)A/D Converter data Input 																*/
/*			2)A/D Converter Parameter Calculation 													*/
/*			3)A/D Converter Check Alarms 															*/
/*			4)																						*/
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
/*																									*/
/*		Rev.1.00 : 2013.8.13	K.Ozaki																*/
/*		Rev.1.10 : 2013.10.1	K.Ozaki			<S00C>												*/
/*		Rev.1.20 : 2013.10.23	K.Ozaki			<S051>												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include	"KnlApi.h"
#include	"PnPrmListTbl.h"	/* Register and Parameter Access Interface */	/* <S0CB> */

#if (SVFSEL_ADC_USE == 1)||(SVFSEL_ONLY_OH_ADC_USE == 1)/* <S16D> */
/****************************************************************************************************/
/*																									*/
/*		A/D Converter Register Definition															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/** ADM0 Bit Definition 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define ADCE   (UINT)(1<<0)
#define ADBSY  (UINT)(1<<1)
#define PWDWNB (UINT)(1<<2)
#define SRESB  (UINT)(1<<15)

/*--------------------------------------------------------------------------------------------------*/
/** ADM1 Bit Definition 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define TRG     (UINT)(1<<0)
#define TRGIN   (UINT)(1<<1)
#define MS      (UINT)(1<<2)
#define RPS     (UINT)(1<<3)
#define BS      (UINT)(1<<4)
#define TRGEN0  (UINT)(1<<8)
#define TRGEN1  (UINT)(1<<9)
#define EGA0    (UINT)(1<<12)
#define EGA1    (UINT)(1<<13)

/*--------------------------------------------------------------------------------------------------*/
/** ADM2 Bit Definition 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define CHSEL0   (UINT)(1<<0)

/*--------------------------------------------------------------------------------------------------*/
/** ADM3 Bit Definition 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define ADSMP0   (UINT)(1<<0)
#define EXWAIT0  (UINT)(1<<8)
#define ADCMP0   (UINT)(1<<16)
#define ADIL0    (UINT)(1<<24)

/*--------------------------------------------------------------------------------------------------*/
/** ADINT Bit Definition 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define INTEN    (UINT)(1<<0)
#define CSEEN    (UINT)(1<<16)
#define INTS     (UINT)(1<<31)

/*--------------------------------------------------------------------------------------------------*/
/** ADSTS Bit Definition 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define INTST0   (UINT)(1<<0)
#define CSEST    (UINT)(1<<16)
#define TRGS     (UINT)(1<<31)


/****************************************************************************************************/
/*																									*/
/*		Function Prototypes																			*/
/*																									*/
/****************************************************************************************************/
#if (SVFSEL_ONLY_OH_ADC_USE == 0)			/* <S0E1>:SVFSEL_ONLY_OH_ADC_USEaddition					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Public Functions																			*/
/*--------------------------------------------------------------------------------------------------*/
void	KpxAdcScanAService( CADC *pAdc	);	/* A/D Converter ScanA Service							*/
void	KpxAdcScanBService( CADC *pAdc	);	/* A/D Converter ScanB Service							*/
void	KpxAdcScanCService( CADC *pAdc  );	/* A/D Converter ScanC Service							*/
void	KpxInitAdc( void *ax );				/* Initialize A/D Interface 							*/
/*--------------------------------------------------------------------------------------------------*/
LONG	KpiInputSpdRefAd( CADC *pAdc );		/* Speed command AD input										*/
KSGAIN	KpiPcalSpdRefAdGain(  				/* Velocity Reference AD gain caluculation 				*/
	LONG	RatSpd,							/* rated speed [rad/s], [m/s] 							*/
	LONG	OvrSpd,							/* overed speed [rad/s], [m/s] 							*/
	LONG	Vrefgn,							/* Pn300: velocity input gain [0.01V/rated speed 		*/
	LONG	Gadjv );						/* PnE55: velocity gain adjust variable					*/
LONG	KpiInputTrqRefAd( CADC *pAdc );		/* Torque command AD input									*/
KSGAIN	KpiPcalTrqRefAdGain( 				/* Torque Reference AD gain calculation					*/
	LONG	MaxTrq, 						/* MaxTrq : Maximum torque[%]								*/
	LONG	Trefgn, 						/* Trefgn : Torque command input gain[0.1V/Rated torque]		*/
	LONG	Gadjtp, 						/* Gadjtp : Positive torque command gain adjustment [1/256]				*/
	LONG	Ref );							/* Ref.Voltage : TrefP:15840, TrefN:14850				*/
/*--------------------------------------------------------------------------------------------------*/
void	KriTRefOffsetInc( CADC *pAdc );		/* Increment TREF A/D Offset 							*/
void	KriTRefOffsetDec( CADC *pAdc );		/* Decrement TREF A/D Offset 							*/
void	KriVRefOffsetInc( CADC *pAdc );		/* Increment VREF A/D Offset 							*/
void	KriVRefOffsetDec( CADC *pAdc );		/* Decrement VREF A/D Offset 							*/
/*--------------------------------------------------------------------------------------------------*/
LONG	KpiAdjustRefAdZero( CADC *pAdc );	/* 														*/
/* <S0CB>	Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Automatic testing machine mode Command AD adjustment																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	TciAdjustVrefGain( AXIS_HANDLE *Axis );			/* Speed command AD gain adjustment					*/
LONG	TciAdjustTrefPGain( AXIS_HANDLE *Axis );		/* Torque command AD positive side gain adjustment				*/
LONG	TciAdjustTrefNGain( AXIS_HANDLE *Axis );		/* Torque command AD negative side gain adjustment				*/
LONG	KpiInputOHAd( CADC *pAdc );						/* Overheat AD input						*/
void	TciReadVrefAD32Mean( LONG *AdMean );			/* Read speed command average value						*/
void	TciReadTrefAD16Mean( LONG AdMean[2] );			/* Torque command average value reading					*/
/* <S0CB>	End */

/*--------------------------------------------------------------------------------------------------*/
/*		Private Functions : ADC Data Check															*/
/*--------------------------------------------------------------------------------------------------*/
static void	LpxCheckVrefAdData( CADC *pAdc );		/* Speed command AD data check processing				*/
static void	LpxPcalCheckVrefAdData( CADC *pAdc );	/* Parameter calculation for speed command AD data check	*/
static void	LpxCheckTrqRefAdAlm( CADC *pAdc );		/* Torque command AD abnormality check processing				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Private Functions : TCI 																	*/
/*--------------------------------------------------------------------------------------------------*/
static void	TciAdjustRefAdZero( CADC *pAdc );
static void	TciReadTrefVrefADMean( LONG AdMean[3] );
static void	MlibAdd6432( LONG *a, LONG b, LONG *x );

#elif (SVFSEL_ONLY_OH_ADC_USE == 1)					/* <S0E1>:SVFSEL_ONLY_OH_ADC_USEaddition			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Public Functions																			*/
/*--------------------------------------------------------------------------------------------------*/
void	KpxAdcScanCService( CADC *pAdc  );	/* A/D Converter ScanC Service							*/
void	KpxInitAdc( void *ax );				/* Initialize A/D Interface 							*/
LONG	KpiInputOHAd( CADC *pAdc );		/* Overheat AD input									*/
#endif
/****************************************************************************************************/
/*																									*/
/*		Variables																					*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
#define	TREFZVADP		0x0C2				/* AD value when positive side torque command AD zero V is input				*/
#define	TREFZVADN		0x098				/* AD value when negative side torque command AD zero V input				*/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Initialize Virtual Serial Message Interface													*/
/*																									*/
/****************************************************************************************************/
void	KpxInitAdc( void *ax )
{
#if (SVFSEL_ADC_USE == 1)||(SVFSEL_ONLY_OH_ADC_USE == 1)	/* <S09F> *//* Is a compile switch necessary here? */
AXIS_HANDLE *pAxRsc = (AXIS_HANDLE *)ax;								/* Axis handle */
tADC_REG *areg = CPU_ADC_REG0;											/* 1st channel */
//CADC *pAdc = &(LAdc[ch]);												/* I/F variable */
CADC *pAdc = pAxRsc->pAdc;												/* I/F variable */
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( pAdc, sizeof( CADC )/4 );						/* Reset Variables			*/
/*--------------------------------------------------------------------------------------------------*/
/*	Reset CPU ADC																					*/
/*--------------------------------------------------------------------------------------------------*/
	ADC_REG_WR( ADM0, 0 );						/* set ADC Reset */
	KlibWaitus(10);								/* wait 10us */
	ADC_REG_WR( ADM0, SRESB );					/* clear ADC Reset */
/*--------------------------------------------------------------------------------------------------*/
/*	Initialize CPU ADC																				*/
/*--------------------------------------------------------------------------------------------------*/
/*  1) ADConversion time is(ADSMP(0x07) + ADCMP(0x0E))*ADCLK(4Mhz)*2ch = 10,5us?							*/
/*--------------------------------------------------------------------------------------------------*/
	ADC_REG_WR( ADM1, RPS );					/* ADM1: Software Trigger, Scan Mode, 1 buffer Mode,*/
												/* Repeat Mode 										*/
//	ADC_REG_WR( ADM2, 0x03 );					/* ADM2: Using ch:0,1 								*//* <S0CB> */
#if CSW_BOARD_TYPE != SGD7W_TYPE	/* <S0E1> */
	ADC_REG_WR( ADM2, 0x07 );					/* ADM2: Using ch:0,1,2  							*//* <S0CB> */
#else/* CSW_BOARD_TYPE == SGD7W_TYPE */
	ADC_REG_WR( ADM2, 0x0F );					/* ADM2: Using ch:0,1,2,3  							*/
#endif
	ADC_REG_WR( ADM3, (0x07) | (0x0E)<<16 ); 	/* ADM3: ADSMP=0x07, ADCMP=0x0E, ADID=0 			*/
	/*----------------------------------------------------------------------------------------------*/
	ADC_REG_WR( ADINT, (0x03) );				/* ADINT: use interrupt (ch:0,1)					*/
	ADC_REG_WR( ADSTS, (0x0F) );				/* ADSTS: clear interrupt status 					*/
	ADC_REG_WR( ADIVC, (0x10) );				/* ADIVC: ADCLK = 62.5Mhz/16=  3.90625Mhz			*/
	/*----------------------------------------------------------------------------------------------*/
	ADC_REG_WR( ADM0, SRESB | PWDWNB );			/* ADM0: set ADC Normal Mode 						*/
	KlibWaitus(10);								/* wait 10us(As a precaution) 								*/
	ADC_REG_WR( ADM0, SRESB | PWDWNB | ADCE );	/* ADM0: start adc 									*/
/*--------------------------------------------------------------------------------------------------*/
	pAdc->hAlm = pAxRsc->AlmManager;			/* Erase after */
	pAdc->pPrm = pAxRsc->UniPrms->Prm;			/* Erase after */
	pAdc->pSot = pAxRsc->SeqCtrlOut;			/* Erase after */

#if (SVFSEL_ONLY_OH_ADC_USE == 0)			/* <S0E1>:SVFSEL_ONLY_OH_ADC_USEaddition					*/
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalCheckVrefAdData( pAdc );				/* Parameter Calculation							*/
/*--------------------------------------------------------------------------------------------------*/
#endif

#endif                 /* <S09F> */
	return;
}

#if (SVFSEL_ONLY_OH_ADC_USE == 0)			/* <S0E1>:SVFSEL_ONLY_OH_ADC_USEaddition					*/
/****************************************************************************************************/
/*																									*/
/*		A/D Converter ( ScanA Cycle )																*/
/*																									*/
/****************************************************************************************************/
void	KpxAdcScanAService( CADC *pAdc	)
{
#define _UDL_REG_ADDR_  (ULONG)0xC0000000
#if 1	/* <S051> */
/*--------------------------------------------------------------------------------------------------*/
/*		Read Reference A/D																			*/
/*--------------------------------------------------------------------------------------------------*/
		if ( ADC_VREFAD_USE != 0 )							
		{
			pAdc->Var[ADC0_VREFAD] = KpiInputSpdRefAd( pAdc );	/* Read Vref A/D */
		}
		if ( ADC_TREFAD_USE != 0 )							
		{
			pAdc->Var[ADC1_TREFAD] = KpiInputTrqRefAd( pAdc );	/* Read Tref A/D */
		}
#endif
	;	
}


/****************************************************************************************************/
/*																									*/
/*		A/D Converter ( ScanB Cycle )																*/
/*																									*/
/****************************************************************************************************/
void	KpxAdcScanBService( CADC *pAdc )
{
#if 0	/* <S051> */
/*--------------------------------------------------------------------------------------------------*/
/*		Read Reference A/D																			*/
/*--------------------------------------------------------------------------------------------------*/
		if ( ADC_VREFAD_USE != 0 )							
		{
			pAdc->Var[ADC0_VREFAD] = KpiInputSpdRefAd( pAdc );	/* Read Vref A/D */
		}
		if ( ADC_TREFAD_USE != 0 )							
		{
			pAdc->Var[ADC1_TREFAD] = KpiInputTrqRefAd( pAdc );	/* Read Tref A/D */
		}
#endif
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Servo control command input processing(Anlg.c)														*/
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Torque limit input(Anlg.c)																*/
/*--------------------------------------------------------------------------------------------------*/

	return;
}
#endif
/****************************************************************************************************/
/*																									*/
/*		A/D Converter ( ScanC Cycle )																*/
/*																									*/
/****************************************************************************************************/
void	KpxAdcScanCService( CADC *pAdc )
{
/*--------------------------------------------------------------------------------------------------*/
//		pAdc->Var[ADC3_OHIAD] = KpiInputOHAd( pAdc );	/* Read OH A/D								*//* <S0CB> */
		KpiInputOHAd( pAdc );							/* Read OH A/D								*//* <S0CB> *//* <S0E1>:SGD7WCorrespondence */
#if (SVFSEL_ONLY_OH_ADC_USE == 0)						/* <S0E1>:SVFSEL_ONLY_OH_ADC_USEaddition		*/
		LpxCheckVrefAdData( pAdc );						/* Check VREF A/D Data mulfunction			*/
		LpxCheckTrqRefAdAlm( pAdc );					/* Check Trq ref A/D mulfunction			*/
#endif
/*--------------------------------------------------------------------------------------------------*/
		return;
}



#if (SVFSEL_ONLY_OH_ADC_USE == 0)			/* <S0E1>:SVFSEL_ONLY_OH_ADC_USEaddition					*/
/****************************************************************************************************/
/*																									*/
/*		Speed command AD input																			*/
/*																									*/
/****************************************************************************************************/
LONG	KpiInputSpdRefAd( CADC *pAdc )
{
LONG InAd;
/*--------------------------------------------------------------------------------------------------*/
/*		Adjusting command AD																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( pAdc->Mode == ADCMD_ADJREFAD )									/* adjusting */
		{
			return (0);														/* return zero */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Read Velocity Reference A/D																	*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			InAd = SHalDrv_ReadRefAd( (ASIC_HW_IF_STR *)_UDL_REG_ADDR_ );	/* Read A/D Converter 	*/
			InAd = ((InAd<<8)>>1) + pAdc->P.ZadjVelo; 						/* Zero Adjust			*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return (InAd);
}


/****************************************************************************************************/
/*																									*/
/*		Speed command AD calculation gain calculation																	*/
/*																									*/
/*		<1>24bit AD --> 16bit AD																	*/
/*		<2>Delete correction based on actual measurement																		*/
/****************************************************************************************************/
/*																									*/
/*					  4.096 * RatSpd * 2^24 * 100 * (Gadjv + 65536)         VrefgnD					*/
/*		Kvrefad = ------------------------------------------------------ * ---------				*/
/*					  2^15 * HWgain * OvrSpd * Vrefgn * 65536 * 2^7          10000					*/
/*																									*/
/*						16384 * RatSpd * (Gadjv + 65536)			VrefgnD							*/
/*				= ---------------------------------------------- * ---------						*/
/*					  10 * HWgain * OvrSpd * Vrefgn * 65536		     10000							*/
/*																									*/
/*																									*/
/*		RatSpd  : Rated speed				[rad/s], [m/s]												*/
/*		OvrSpd  : Overspeed		[rad/s], [m/s]												*/
/*		Vrefgn  : Speed command input gain	[0.01V/Rated speed]											*/
/*		VrefgnD	: Same as above Derating	[0.01%]	(Pn309)												*/
/*		Gadjv   : Speed command gain adjustment		[1/65536]													*/
/*		HWgain  : HW circuit gain(HWgain=4/15)														*/
/*		Cx      : Correction by actual measurement(Cx=3390/4096)							====> @SKLT@ 		*/
/*		2^7     : Input magnification correction(Zero adjustment data2^7Since it is multiplied by)									*/
/*																									*/
/*		Reference Shot  : For details, refer to the speed command AD gain adjustment(Tstcmdif.c:TciAdjustVrefGain())					*/
/*																									*/
/*																									*/
/****************************************************************************************************/
KSGAIN	KpiPcalSpdRefAdGain(  
	LONG	RatSpd,							/* rated speed [rad/s], [m/s] */
	LONG	OvrSpd,							/* overed speed [rad/s], [m/s] */
	LONG	Vrefgn,							/* Pn300: velocity input gain [0.01V/rated speed */
	LONG	Gadjv )							/* PnE55: velocity gain adjust variable			*/
{
//LONG	VrefgnD;							/* Pn309(Unused)								*/
#if	( SVFSEL_VREFAD_24BIT != 0 )	/* @SKLT@ */
		// *** Compile Error: What is "correction based on actual measurement"? ***
#else
KSGAIN kx;
LONG sx;
		kx = MlibScalKxgain( 16384,	15,          4*10,         &sx,  0 );
		kx = MlibPcalKxksks( kx,	RatSpd, 	OvrSpd,  &sx,  0 );
		kx = MlibPcalKxgain( kx,	1,          Vrefgn,   &sx,  0 );
		kx = MlibPcalKxgain( kx,	((SHORT)Gadjv + 65536), 65536, &sx, 24 );
		return (kx);
#endif
}

/****************************************************************************************************/
/*																									*/
/*		Torque command AD input																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*								 AD	|   															*/
/*				  Ad-N				|   	   Ad-P													*/
/*					*				|			* 													*/
/*					  * 	  		|		  * 													*/
/*						* 			|		*  														*/
/*						  *---------+-----*-------- TREFZVADP + TREFZVADN							*/
/*						  |	* 		|	* |  														*/
/*						  |	  *		| *   |															*/
/*						  |		*   *-----+--------	TREFZVADP										*/
/*						  |		  * |     |															*/
/*						  |		*   *-----+-------- TREFZVADN										*/
/*						  |	  *  	| *   |															*/
/*						  | *		|	* |															*/
/*			--------------*---------+-----*-----------------> IN									*/
/*						  |				  |															*/
/*			------------->|<------------->|<-------------  											*/
/*				   Negative side		  Negative side+Positive side		  Positive side													*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG	KpiInputTrqRefAd( CADC *pAdc )
{
LONG	InAdP;
LONG	InAdN;
LONG	Trefx;
tADC_REG *areg = CPU_ADC_REG0;											/* @@ */
/*--------------------------------------------------------------------------------------------------*/
/*		Read A/D converter																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( pAdc->Mode == ADCMD_ADJREFAD )		/* adjusting */
		{
			return (0);							/* return zero */
		}
		else if ( pAdc->f.TrefAdReadOk == TRUE ) 	/* normal read */
		{
			ADC_REG_RD( ADCR0, InAdP ); 								/* TREFP (SGDV<V883>Reference) */
			ADC_REG_RD( ADCR1, InAdN ); 								/* TREFN */
		}
		else									/* velocity A/D checking */
		{										/* reference AD: (2*ScanC)cycle read */
			InAdP = pAdc->TrefAdinP;			/* LpxCheckVrefAdData() */
			InAdN = pAdc->TrefAdinN;			/* LpxCheckVrefAdData() */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Normalizing	[2^24/MaxTrq]																	*/
/*--------------------------------------------------------------------------------------------------*/
/*
 *		※Gain calculation moved to optional module(Anlg.c)
 */
/*--------------------------------------------------------------------------------------------------*/
#define INADPX (InAdP + pAdc->P.ZadjTrqP)					/* AD variable with Offset 				*/
#define INADNX (InAdN - pAdc->P.ZadjTrqN)					/* AD variable with Offset 				*/
		if( InAdP > (TREFZVADP + TREFZVADN) )				/* Only positive side AD used						*/
		{
			Trefx = (INADPX - TREFZVADP);					/* (SGDV<V883>Reference)						*/
		}
		else if( InAdN > (TREFZVADP + TREFZVADN) )			/* Using only negative side AD						*/
		{
			Trefx = (TREFZVADN - INADNX);					/* (SGDV<V883>Reference)						*/
		}
		else												/* Positive side AD + negative side AD					*/
		{
			Trefx = (INADPX - TREFZVADP);					/* (SGDV<V883>Reference)						*/
			Trefx = ( Trefx + ((TREFZVADN - INADNX)) ) >> 1;
		}
/*--------------------------------------------------------------------------------------------------*/
		return (Trefx);
}

/****************************************************************************************************/
/*																									*/
/*		Torque command AD calculation gain calculation																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*					 Ref[0.01V]              10              100 * 2^24     (Gadjtp + 256)			*/
/*		Ktrefad = ---------------- * -------------------- * ------------ * -----------------		*/
/*				   1000*((2^12)-1)	 Trefgn[0.1V/RatTrq]     PerMaxTrq         256					*/
/*																									*/
/*		1)JL086ofADConversion is12bit																		*/
/*		2)TREFPofRef Voltage(ADvalueFFFH)teeth15.091[V]										*/
/*		3)TREFNofRef Voltageteeth14.299[V]																*/
/*																									*/
/*		MaxTrq : Maximum torque				[%]															*/
/*		Trefgn : Torque command input gain	[0.1V/Rated torque]											*/
/*		Gadjtp : Positive torque command gain adjustment [1/256]														*/
/*		Gadjtn : Negative side torque command gain adjustment [1/256]														*/
/*																									*/
/*		Reference Shot  : For details, refer to the torque command AD positive side gain adjustment and torque command AD negative side gain adjustment					*/
/*				 (Tstcmdif.c:TciAdjustTrefPGain(),TciAdjustTrefNGain())						*/
/*																									*/
/*																									*/
/****************************************************************************************************/
KSGAIN	KpiPcalTrqRefAdGain( 
	LONG	MaxTrq, 		/*	MaxTrq : Maximum torque[%]												*/
	LONG	Trefgn, 		/*	Trefgn : Pn400:Torque command input gain[0.1V/Rated torque]				*/
	LONG	Gadjtp, 		/*	Gadjtp : Torque command gain adjustment [1/256]									*/
	LONG	Ref )			/*  Reference Voltage [0.001V]: TrefP:XXXX, TrefN:XXXX					*/
{
LONG kx,sx;
/*--------------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( Ref, 16777216, (Trefgn * MaxTrq), &sx, 0 );
	kx = MlibPcalKxgain( kx, ((CHAR)Gadjtp + 256), (256*4095), &sx, 24 );
/*--------------------------------------------------------------------------------------------------*/
	return (kx);
}


/****************************************************************************************************/
/*																									*/
/*		Torque command AD abnormality check processing																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitor the CPU interrupt request signal when the torque command AD conversion is completed, and turn the interrupt request signal OFF			*/
/*			If this is the case, it is judged as abnormal.																	*/
/*																									*/
/*	Supplementary Foot : AD of V850E/MA1 starts with scan A and is about 5us * AD conversion by number of CH			*/
/*			is completed, so when AD is operating normally, when scan C is executed, the interrupt request signal is set			*/
/*			.																			*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	LpxCheckTrqRefAdAlm( CADC *pAdc )
{
tADC_REG *areg = CPU_ADC_REG0;											/* @@ */
UINT sts;
#if   ADC_TREFAD_USE == 1                /* <S09F> */
/*--------------------------------------------------------------------------------------------------*/
/*		Torque command AD abnormality check processing																*/
/*--------------------------------------------------------------------------------------------------*/
	if( /* repeat mode it is unnecessary KnlV.f.ScanAEnable && */ (pAdc->Mode != ADCMD_ADJREFAD) )
	{
		if( ! KPX_CHK_CPUADINTF( ) )						/* AD interrupt request signal check			*/
		{
			ALMSetServoAlarm( pAdc->hAlm, ALM_TREFADERR );	/* A.B20 : Torque command AD error			*/
		}
		KPX_CLR_CPUADINTF( );								/* AD interrupt request signal reset			*/
	}
/*--------------------------------------------------------------------------------------------------*/
#endif                    /* <S09F> */
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Speed command AD data check processing																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : When base enable request is OFF, switch AD input of CPU from torque command to speed command,		*/
/*			Two speed command AD values(ASIC,CPU), determine whether the speed command AD is normal/abnormal			*/
/*			Check.																			*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	LpxCheckVrefAdData( CADC *pAdc )
{
#if  ADC_VREFAD_USE == 1                /* <S09F> */
#define INADP(x) ((x) + pAdc->P.ZadjTrqP)					/* AD variable with Offset 				*/
#define INADN(x) ((x) - pAdc->P.ZadjTrqN)					/* AD variable with Offset 				*/
LONG	lwk0;
LONG	lwk1;
UCHAR	AdSel;
LONG	VrefAd;
LONG	TrefAd;
tADC_REG *areg = CPU_ADC_REG0;											/* @@ */
SEQ_CTRL_OUT *pSeqCtrlOut = (SEQ_CTRL_OUT *)pAdc->pSot;					/* Erase after */
/*--------------------------------------------------------------------------------------------------*/
/*		Command AD adjustment status check																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( pAdc->Mode == ADCMD_ADJREFAD )				/* adjusting */
		{
			return;										/* Do nothing while adjusting command AD				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm mask check																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( ALMCheckSetMask( pAdc->hAlm, ALM_VREFADCHK ) /* KPI_ALARMMSKCHK( ALM_VREFADCHK ) */ )
		{
			KPI_DI();									/* @@ chk */
			KPX_SET_ADSEL_TREF( );						/* Torque command input selection						*/
			KPI_EI();									/* @@ chk */
			pAdc->f.TrefAdReadOk = TRUE;				/* Torque command AD reading OK					*/
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Read AD input switch																*/
/*--------------------------------------------------------------------------------------------------*/
		AdSel = KPX_READ_ADSEL( );

/*--------------------------------------------------------------------------------------------------*/
/*		Torque command AD input processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( AdSel == KPX_ADSEL_TREF )
		{
			ADC_REG_RD( ADCR0, lwk0 ); /* lwk0 += pAdc->P.ZadjTrqP; <S051><V883> */pAdc->TrefAdinP = lwk0;
			ADC_REG_RD( ADCR1, lwk1 ); /* lwk1 -= pAdc->P.ZadjTrqN; <S051><V883> */pAdc->TrefAdinN = lwk1;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Speed command AD data check processing																*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
		/*------------------------------------------------------------------------------------------*/
		/*		ASIC speed command AD reading															*/
		/*------------------------------------------------------------------------------------------*/
			VrefAd = (pAdc->Var[ADC0_VREFAD])>>7; 			/* @@ Interim */
		/*------------------------------------------------------------------------------------------*/
		/*		CPU speed command AD reading															*/
		/*------------------------------------------------------------------------------------------*/
			ADC_REG_RD( ADCR0, lwk0 ); 		       			/* read AD */
			ADC_REG_RD( ADCR1, lwk1 ); 						/* read AD */
		/*------------------------------------------------------------------------------------------*/
			if( lwk0 > (TREFZVADP + TREFZVADN) )			/* Only positive side AD used						*/
			{
				TrefAd = (INADP(lwk0) - TREFZVADP);			/* 										*/
			}
			else if( lwk1 > (TREFZVADP + TREFZVADN) )		/* Using only negative side AD						*/
			{
				TrefAd = (TREFZVADN - INADN(lwk1));			/* 										*/
			}
			else											/* Positive side AD + negative side AD					*/
			{
				TrefAd = (INADP(lwk0) - TREFZVADP);			/* 										*/
				TrefAd = ( TrefAd + ((TREFZVADN - INADN(lwk1))) ) >> 1;
			}
		/*------------------------------------------------------------------------------------------*/
			lwk0 = ((TrefAd >= 0 ) ? (pAdc->P.KvadchkP) : (pAdc->P.KvadchkN)); /* set AD check gain */
			TrefAd = MlibMulgain( TrefAd, lwk0 );
		/*------------------------------------------------------------------------------------------*/
		/*		Speed command AD data check															*/
		/*------------------------------------------------------------------------------------------*/
			lwk0 = MlibAbs32( VrefAd - TrefAd );		/* Error calculation									*/
			if( lwk0 > (pAdc->P.VrefAdAlmLevel) )		/* Error check								*/
			{
				if( ++(pAdc->VrefAdDchkCnt) > 10 )		/* Alarm for 10 consecutive times						*/
				{
					(pAdc->VrefAdDchkCnt) = 10;			/* Clamp check counter				*/
					ALMSetServoAlarm( pAdc->hAlm, 
									ALM_VREFADCHK );	/* A.B11 : Abnormal AD data for speed command			*/
				}
			}
			else
			{
				(pAdc->VrefAdDchkCnt) = 0;				/* Reset check counter				*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		CPU: AD input switching process																	*/
/*--------------------------------------------------------------------------------------------------*/
		/*------------------------------------------------------------------------------------------*/
		/* <SGDS>																	*/
		/* Change from the specification where speed command and torque command are read alternately.										*/
		/* [CoutK.f.DbBeReq==FALSE]If the speed command reading is fixed as a condition, the torque command will not be read during OT	*/
		/* When there is no servo on request and it is not OT, the CPU side AD is changed to read the speed command. 	*/
		/* Fn009(Analog command offset automatic adjustment), command AD switching is performed from the auxiliary function.		*/
		/* At the top of this function, execute the adjustment function (Fn009) usereturnand do not execute the following process.			*/
		/* 																							*/
		/* <SGDSFault horizontal deployment>																		*/
		/* CoutK.f.SvonReq(Servo on request) was set as the command AD switching condition, but						*/
		/* Pn506≠Even if the servo is turned off at 0, the command AD switches to the speed command side even though the power is on.	*/
		/* The switching condition is not the servo-on request, but the energized state.											*/
		/* Servo-on request and energized state are switching conditions.												*/
		/*------------------------------------------------------------------------------------------*/
//SGDV 	if( (CoutK.f.SvonReq == FALSE) && (CoutK.f.BaseEnable == FALSE) && (CoutK.f.Ot == FALSE) )
		if( ( pSeqCtrlOut->BaseEnableReq == FALSE ) 
			&& ( pSeqCtrlOut->BeComplete == FALSE )
			&& ( pSeqCtrlOut->OverTrvlSts == FALSE ) )
		{
			pAdc->f.TrefAdReadOk = FALSE;				/* Torque command AD reading not possible					*/
			if( AdSel == KPX_ADSEL_TREF )				/* AD input switching process							*/
			{
				KPI_DI();								/* @@ chk */
				KPX_SET_ADSEL_VREF( );					/* Speed command input selection							*/
				KPI_EI();								/* @@ chk */
			}
			else
			{
				KPI_DI();								/* @@ chk */
				KPX_SET_ADSEL_TREF( );					/* Torque command input selection						*/
				KPI_EI();								/* @@ chk */
			}
		}
	/*----------------------------------------------------------------------------------------------*/
		else											/* When servo is on								*/
		{
			if( AdSel == KPX_ADSEL_TREF )
			{
				pAdc->f.TrefAdReadOk = TRUE;			/* Torque command AD reading OK					*/
			}
			else
			{
				KPI_DI();								/* @@ chk */
				KPX_SET_ADSEL_TREF( );					/* Torque command input selection						*/
				KPI_EI();								/* @@ chk */
			}
		}
/*--------------------------------------------------------------------------------------------------*/
#endif                    /* <S09F> */
		return;
}


/****************************************************************************************************/
/*																									*/
/*		Parameter calculation for speed command AD data check													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*					   2^23      4    																*/
/*		VrefAdAsic = ------- * ---- * Input   				HW circuit gain(HWgain=4/15)				*/
/*					  4.096		15    						Correction by actual measurement(Cx=3390/4096)<-Delete		*/
/*																									*/
/*					   1023      3.3																*/
/*		VrefAdCpuP = ------- * ------- * Input				HW circuit gain(HWgain=3.3/15.09)		*/
/*					   3.3      15.09      															*/
/*																									*/
/*					   1023      3.3 																*/
/*		VrefAdCpuN = ------- * ------- * Input				HW circuit gain(HWgain=3.3/14.29)		*/
/*					   3.3      14.29 																*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*	<1> 24Bit --> 16Bit Correspondence																		*/
/*	<2> Delete correction based on actual measurement																			*/
/*	<3> JL086_CPUAD --> 12BitCorrespondence																	*/
/*																									*/
/*					 2^15 *  4            Gadjv + 65536     15.09	      256						*/
/*		KvadchkP = ------------------- * --------------- * ------- * --------------					*/
/*					4.096 * 15                 65536        4095	  Gadjtp + 256					*/
/*																									*/
/*					512 * 15.09	          Gadjv + 65536												*/
/*				 = ------------------- * ---------------											*/
/*					4.096 * 15 * 4095	   Gadjtp + 256  											*/
/*																									*/
/*																									*/
/*					 2^15 *  4            Gadjv + 65536     14.29	      256						*/
/*		KvadchkN = ------------------- * --------------- * ------- * --------------					*/
/*					4.096 * 15                 65536        4095      Gadjtn + 256					*/
/*																									*/
/*					512 * 14.29	          Gadjv + 65536												*/
/*				 = ------------------- * ---------------											*/
/*					4.096 * 15 * 4095	   Gadjtn + 256  											*/
/****************************************************************************************************/
void	LpxPcalCheckVrefAdData( CADC *pAdc )
{
LONG	kx,sx;
#if	( SVFSEL_VREFAD_24BIT != 0 )
		Compile_Error!!									/* Not supported									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of positive side calculation gain																		*/
/*--------------------------------------------------------------------------------------------------*/
		kx = MlibScalKxgain( 1,  1718323,  62853,  &sx,  0 );
		kx = MlibPcalKxgain( kx, ((SHORT)Prm.gadjv + 65536), ((CHAR)Prm.gadjtp_n.b.l + 256), &sx, 24 );
		KioP.KvadchkP = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of negative side calculation gain																		*/
/*--------------------------------------------------------------------------------------------------*/
		kx = MlibScalKxgain( 1,  1610928,  62853,  &sx,  0 );
		kx = MlibPcalKxgain( kx, ((SHORT)Prm.gadjv + 65536), ((CHAR)Prm.gadjtp_n.b.h + 256), &sx, 24 );
		KioP.KvadchkN = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Speed command AD check alarm level	: (2^23 * 12V / 18.55V) * (20% / 100%)					*/
/*--------------------------------------------------------------------------------------------------*/
		KioP.VrefAdAlmLevel = 1085318;
#else
PRMDATA	*pPrm = pAdc->pPrm;				/* @@ */
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of positive side calculation gain	(512*15.09=7726.08, 4.096*15*4095=251596.8)							*/
/*--------------------------------------------------------------------------------------------------*/
		kx = MlibScalKxgain( 1,  7726,  251597,  &sx,  0 );	
		kx = MlibPcalKxgain( kx, ((SHORT)pPrm->gadjv + 65536), ((CHAR)pPrm->gadjtp_n.b.l + 256), &sx, 24 );
		pAdc->P.KvadchkP = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of negative side calculation gain	(512*14.29=7316.48, 4.096*15*4095=251596.8)							*/
/*--------------------------------------------------------------------------------------------------*/
		kx = MlibScalKxgain( 1,  7316,  251597,  &sx,  0 );
		kx = MlibPcalKxgain( kx, ((SHORT)pPrm->gadjv + 65536), ((CHAR)pPrm->gadjtp_n.b.h + 256), &sx, 24 );
		pAdc->P.KvadchkN = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Speed command AD check alarm level	: (2^15 * 12V / 15.36V) * (20% / 100%)					*/
/*																									*/
/*			Maximum allowable input voltage(±12V)of20%as a dummy variable to the failure check level.								*/
/*			15Vinput is not theADvalue0x7D00,ADvalue0x7FFFand15.36Vinput.							*/
/*			(Correction by actual measurementCx=3390/4096is not considered.)												*/
/*--------------------------------------------------------------------------------------------------*/
		pAdc->P.VrefAdAlmLevel = 5120;
#endif
		return;
}


/****************************************************************************************************/
/*																									*/
/*		Speed command AD abnormality check processing																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : ASIC general-purpose input(IOSTAT)24bitMonitor AD timeout errors.				*/
/*			AD24ERR bit(Hi)It is considered abnormal.(Alarm reset enabled)								*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0	/* Not supported */
void	LpxCheckVrefAdAlm( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SVFSEL_VREFAD == 0 ){ return;}				/* Mask Function							*/

/*--------------------------------------------------------------------------------------------------*/
/*		Speed command AD abnormality check																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( KPX_CHK_CAD24_ERROR( ) )					/* @SKLT@ */
		{
			if( KioV.VrefAdErrChk == 0x00 )
			{
				KioV.VrefAdErrChk = 0x01;				/* The first time is not an alarm				*/
			}
			else
			{
				KPI_ALARMSET( ALM_VREFADERR );			/* A.B10 : Abnormal AD data for speed command				*/
				KPI_ALARMMSK( ALM_VREFADCHK );			/* A.B11 : SpeedA/DData abnormality mask	<V048>	*/
			}
			KPX_CAD24ERR_CLEAR( );						/* Speed command AD error clear					*//* @SKLT@ */
		}
		else
		{
			KioV.VrefAdErrChk = 0x00;					/* Error check flag clear				*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;

}
#endif
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Command AD zero adjustment process(For online)																*/
/*																									*/
/****************************************************************************************************/
LONG	KpiAdjustRefAdZero( CADC *pAdc )
{
LONG	i;
/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SVFSEL_ADC_USE == 0 )
		{
			return( KPI_ADC_NOTADJ );					/* Error Return								*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing (Set Adjust Mode)																	*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
		KPI_DI( );										/* Disable Interrupt						*/
		if( (CoutK.f.SvonReq == TRUE) || (KnlV.KnlMode != KPI_KMD_NORMAL) )
		{
			KPI_EI( );									/* Enable Interrupt							*/
			return( KPI_ADC_NOTADJ );					/* Error Return								*/
		}
		else
		{
			KnlV.KnlMode = KPI_KMD_ADJREFAD;			/* Set Adjust RefAd Mode					*/
			KPI_EI( );									/* Enable Interrupt							*/
		}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Base block check																		*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
		for( i=0; KPI_CHK_BASE_ENABLE( ); i++ )
		{
			if( i > 1000 )								/* Check Time Over							*/
			{
				KnlV.KnlMode = KPI_KMD_NORMAL;			/* Reset Adjust RefAd Mode					*/
				return( KPI_ADC_NOTADJ );				/* Error Return								*/
			}
			KlibWaitms( 1 );							/* Wait 1ms									*/
		}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Command AD zero adjustment process																			*/
/*--------------------------------------------------------------------------------------------------*/
		pAdc->f.AdjustError = FALSE;					/* Reset AdjustError Flag					*/
		pAdc->Mode = ADCMD_ADJREFAD;					/* set adjust mode							*/
		TciAdjustRefAdZero( pAdc  );					/* Adjust VrefAD and TrefAD					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Termination processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		//KnlV.KnlMode = KPI_KMD_NORMAL;				/* Reset Adjust RefAd Mode					*/
		pAdc->Mode = ADCMD_NORMAL;						/* reset adjust mode						*/
		if( pAdc->f.AdjustError == FALSE )
		{
			return( KPI_ADC_ADJOK );					/* Adjust OK								*/
		}
		else
		{
			return( KPI_ADC_ADJERR );					/* Adjust Error								*/
		}
}

/* <S0CB>	Start */
/****************************************************************************************************/
/*																									*/
/*		Speed command AD gain adjustment(For offline use)															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Speed command(Vref)Perform AD gain adjustment.													*/
/*			For speed command input,7.5Vand find the gain tone using the following formula.								*/
/*																									*/
/*			CorrectionGain = (7.5/15.36)*Cx*2^23*65536/(A/DAverage value + Zero toneData) - 65536						*/
/*																									*/
/*			Gain adjustment,-8192 ≦ CorrectionGain ≦ 8192(12.5%) If it is outside the range of						*/
/*			Correction due to adjustment errorGainto zero.Cx : Correction by actual measurement										*/
/*																									*/
/*																									*/
/*	Supplementary Foot :	This routineBURR-BROWNMade24bitA/DConverter ADS1252It is for use.								*/
/*			ADS1252is a differential input, and the gain of the input circuit is as shown in the formula below.						*/
/*																									*/
/*					      10k		1.2k															*/
/*			+Vin =   ----------- * ------ * Vref[V] + 2.5[V]										*/
/*					   20k + 10k	 3k																*/
/*																									*/
/*						  10k		1.2k															*/
/*			-Vin = - ----------- * ------ * Vref[V] + 2.5[V]										*/
/*					   20k + 10k	 3k																*/
/*																									*/
/*			From the above equation, the differential voltage gain is expressed by the following equation.											*/
/*																									*/
/*											   10k		  1.2k		  4								*/
/*			HWgain  =  +Vin - -Vin  =  2 * ----------- * ------  =  ----							*/
/*											20k + 10k	   3k		 15								*/
/*																									*/
/*			Input voltageVref[V]andADS1252Vadin[V]is expressed by the following formula:					*/
/*																									*/
/*											4														*/
/*			Vadin[V] = HWgain * Vref[V]	= ---- * Vref[V]											*/
/*										   15														*/
/*																									*/
/*			ADS1252reference input voltage is4.096[V](ADvalue=2^23)Therefore, from the above formula, if				*/
/*																									*/
/*						 2^23		 2^23	   4				  2^23								*/
/*			Vadin[V] * -------  =  ------- * ---- * Vref[V]  =  ------- * Vref[V]					*/
/*						4.096		4.096	  15				 15.36								*/
/*																									*/
/*			CorrectionGain(Gadjx)is the correction term based on actual measurement(Cx), from the above formula, add							*/
/*																									*/
/*					 2^23 * 2^16 * (7.5/15.36) * Cx				   0x7D000000 * Cx					*/
/*			Gadjx = -------------------------------- - 65536 = ---------------------- - 65536		*/
/*						(Vrefad + Zadjx)>>7						(Vrefad + Zadjx)>>14				*/
/*																									*/
/*			Correction by actual measurement : Cx = 3390/4096															*/
/*																									*/
/*			Delete correction based on actual measurement @SKLT@																*/
/****************************************************************************************************/
LONG TciAdjustVrefGain( AXIS_HANDLE *Axis )
{
	LONG			rc;
	LONG			Gadjx;
	LONG			Zadjx;
	LONG			AdMean;
	PRMDATA			*Prm;
	REGIF_CONFIG_T	*RegCfg;

	/* Initialize */
	rc = KPI_ADC_ADJOK;
	Prm = Axis->UniPrms->Prm;
	RegCfg = Axis->RegManager->hPnReg;

/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( SVFSEL_ADC_USE == 0 )
	{
		rc = KPI_ADC_NOTADJ;	/* Adjustment execution condition not met */
		return( rc );			/* Error Return */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		AD average value read																			*/
/*--------------------------------------------------------------------------------------------------*/
	TciReadVrefAD32Mean( &AdMean );		/* Read Vref AD Mean */

/*--------------------------------------------------------------------------------------------------*/
/*		Gain adjustment data calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
#if	( SVFSEL_VREFAD_24BIT != 0 )
	/* Make Adjust Data */
	Zadjx = Prm->zadjvl;								/* PnE53 : Read speed command zero adjustment value */
	Gadjx = 0x7D000000/((AdMean + Zadjx)>>14) - 65536;	/* Correct missing digit */

	/* Check Adjust Data */
	if( (Gadjx < -16384) || (Gadjx > 16384) )
	{
		Gadjx = 0;
		rc = KPI_ADC_NOTADJ;
	}
#else
	/* Make Adjust Data */
	Zadjx = Prm->zadjvl;								/* PnE53 : Read speed command zero adjustment value */
	Gadjx = 0x7D000000/((AdMean + Zadjx)>>6) - 65536;	/* Correct missing digit */

	/* Check Adjust Data */
	if( (Gadjx < -16384) || (Gadjx > 16384) )
	{
		Gadjx = 0;
		rc = KPI_ADC_NOTADJ;
	}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Gain adjustment data(PnE55 : Speed command gain adjustment)Storage												*/
/*--------------------------------------------------------------------------------------------------*/
	/* Write Adjust Data to RAM */
	Prm->gadjv = (USHORT)Gadjx;

	/* Write Adjust Data to EEPROM */
	PrmStoreValueToEeprom( RegCfg, &pndef_gadjv, 0 );

	return( rc );
}


/****************************************************************************************************/
/*																									*/
/*		Torque command AD positive side gain adjustment(For offline use)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Torque command positive input(Tref positive)Perform AD gain adjustment.									*/
/*		 	Torque command input gain+7.5Vand find the gain tone using the following formula.								*/
/*																									*/
/*			CorrectionGain = (7.5/15.84*1024*256)/(A/DAverage value + Zero toneData - 48) - 256						*/
/*																									*/
/*			Gain adjustment,-127 ≦ CorrectionGain ≦ 127(49.6%) If it is outside the range of						*/
/*			Correction due to adjustment errorGainto zero.														*/
/*																									*/
/*																									*/
/*	Supplementary Foot :	This routineV850E/MA1Built-in10bitA/DFor the converter.									*/
/*			V850E/MA1Built-in10bitA/Dis as shown in the formula below.								*/
/*																									*/
/*						  1k * 2k * 10k         1  *     10k				    1   				*/
/*		Trefp[V] = ------------------------- ( ------------------ * Tref[V] + ----- * 2.5[V] )		*/
/*				    1k*2k + 2k*10k + 10k*1k     1k * (10k + 20k)			   10k					*/
/*																									*/
/*				    20				 15																*/
/*				 = ---- * Tref[V] + ----															*/
/*				    96				 96																*/
/*																									*/
/*					1024	  1024																	*/
/*		Trefp[V] * ------ = ------- * Tref[V] + 48.4848												*/
/*					3.3		 15.84																	*/
/*																									*/
/*		CorrectionGain(Gadjx)From the above formula,																*/
/*																									*/
/*						256 * 1024 * 7.5 						  124121  							*/
/*		Gadjx = -------------------------------- - 256 = ------------------------ - 256				*/
/*				 15.84 * (Trefpad + Zadjx - 48)			  (Trefpad + Zadjx - 48)					*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG	TciAdjustTrefPGain( AXIS_HANDLE *Axis )
{
	LONG			rc;
	LONG			Gadjx;
	LONG			Zadjx;
	LONG			AdMean[2];
	PRMDATA			*Prm;
	REGIF_CONFIG_T	*RegCfg;

	/* Initialize */
	rc = KPI_ADC_ADJOK;
	Prm = Axis->UniPrms->Prm;
	RegCfg = Axis->RegManager->hPnReg;

/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( SVFSEL_ADC_USE == 0 )
	{
		rc = KPI_ADC_NOTADJ;	/* Adjustment execution condition not met */
		return( rc );			/* Error Return */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		AD average value read																			*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_SET_ADSEL_TREF( );				/* Select Tref AD Select Switch */
	//KpiResetWatchdogWaitms( 1 );		/* Reset Watchdog and Wait */
	TciReadTrefAD16Mean( AdMean );		/* Read P-Tref, N-Tref AD Mean */

/*--------------------------------------------------------------------------------------------------*/
/*		Gain adjustment data calculation : Although the calculation formula is different, it is within the adjustment range so it will match the SGDS					*/
/*		, but as per the input circuit gain. Details: Type test question report (JT05793)					*/
/*--------------------------------------------------------------------------------------------------*/
	/* Make Adjust Data */
	Zadjx = Prm->zadjtp_n.b.l;						/* PnE56 : Torque command zero adjustment(Positive input/Negative input) -Low Byte- */
//	Gadjx = 124121/(AdMean[0] + Zadjx - 48) - 256;	/* SWPF	: 256*1024*7.5/15.84 */
	Gadjx = 496484/(AdMean[0] + Zadjx - TREFZVADP) - 256;	/* SWPF	: 256*4096*7.5/15.84 */

	/* Check Adjust Data */
	if ( (Gadjx < -127) || (Gadjx > 127) )
	{
		Gadjx = 0;
		rc = KPI_ADC_NOTADJ;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Gain adjustment data(PnE57 : Torque command gain adjustment(Positive input/Negative input))Storage								*/
/*--------------------------------------------------------------------------------------------------*/
	/* Write Adjust Data to RAM */
	Prm->gadjtp_n.b.l = (USHORT)Gadjx;

	/* Write Adjust Data to EEPROM */
	PrmStoreValueToEeprom( RegCfg, &pndef_gadjtp_n, 0 );

	return( rc );
}
/****************************************************************************************************/
/*																									*/
/*		Torque command AD negative side gain adjustment(For offline use)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Torque command negative input(Tref negative)Perform AD gain adjustment.									*/
/*			Torque command input gain-7.5Vand find the gain tone using the following formula.								*/
/*																									*/
/*			CorrectionGain = (7.5/14.85*1024*256)/(A/DAverage value - Zero toneData - 38) - 256						*/
/*																									*/
/*			Gain adjustment,-127 ≦ CorrectionGain ≦ 127(49.6%) If it is outside the range of						*/
/*			Correction due to adjustment errorGainto zero.														*/
/*																									*/
/*																									*/
/*	Supplementary Foot :	This routineV850E/MA1Built-in10bitA/DFor the converter.									*/
/*			V850E/MA1Built-in10bitA/Dis as shown in the formula below.								*/
/*																									*/
/*					   1k	      10k					      1k		   1k						*/
/*		Trefn[V] = - ------ * ----------- * Tref[V] + ( 1 + ------ ) * ---------- * 2.5[V]			*/
/*				      1.5k	   10k + 20k 					 1.5k	    33k + 1k					*/
/*																									*/
/*				      2				   25															*/
/*				 = - --- * Tref[V] + -----															*/
/*				      9				  204															*/
/*																									*/
/*					1024	    1024																*/
/*		Trefn[V] * ------ = - ------- * Tref[V] + 38.0273											*/
/*					 3.3	   14.85																*/
/*																									*/
/*		CorrectionGain(Gadjx)From the above formula,																*/
/*																									*/
/*						256 * 1024 * 7.5 						  132396  							*/
/*		Gadjx = -------------------------------- - 256 = ------------------------ - 256				*/
/*				 14.85 * (Trefnad - Zadjx - 38)			  (Trefnad - Zadjx - 38)					*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG TciAdjustTrefNGain( AXIS_HANDLE *Axis )
{
	LONG			rc;
	LONG			Gadjx;
	LONG			Zadjx;
	LONG			AdMean[2];
	PRMDATA			*Prm;
	REGIF_CONFIG_T	*RegCfg;

	/* Initialize */
	rc = KPI_ADC_ADJOK;
	Prm = Axis->UniPrms->Prm;
	RegCfg = Axis->RegManager->hPnReg;

/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( SVFSEL_ADC_USE == 0 )
	{
		rc = KPI_ADC_NOTADJ;	/* Adjustment execution condition not met */
		return( rc );			/* Error Return */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		AD average value read																			*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_SET_ADSEL_TREF( );				/* Select Tref AD Select Switch */
	//KpiResetWatchdogWaitms( 1 );		/* Reset Watchdog and Wait */
	TciReadTrefAD16Mean( AdMean );		/* Read P-Tref, N-Tref AD Mean */

/*--------------------------------------------------------------------------------------------------*/
/*		Gain adjustment data calculation : Although the calculation formula is different, it is within the adjustment range so it will match the SGDS					*/
/*		, but as per the input circuit gain. Details: Type test question report (JT05793)					*/
/*--------------------------------------------------------------------------------------------------*/
	/* Make Adjust Data */
	Zadjx = Prm->zadjtp_n.b.h;						/* PnE56 : Torque command zero adjustment(Positive input/Negative input) -High Byte- */
//	Gadjx = 132396/(AdMean[1] - Zadjx - 38) - 256;	/* SWPF : 256*1024*7.5/14.85 */
	Gadjx = 529583/(AdMean[1] - Zadjx - TREFZVADP) - 256;	/* SWPF : 256*1024*7.5/14.85 */

	/* Check Adjust Data */
	if ( (Gadjx < -127) || (Gadjx > 127) )
	{
		Gadjx = 0;
		rc = KPI_ADC_NOTADJ;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Gain adjustment data(PnE57 : Torque command gain adjustment(Positive input/Negative input))Storage								*/
/*--------------------------------------------------------------------------------------------------*/
	/* Write Adjust Data to RAM */
	Prm->gadjtp_n.b.h = (USHORT)Gadjx;

	/* Write Adjust Data to EEPROM */
	PrmStoreValueToEeprom( RegCfg, &pndef_gadjtp_n, 0 );

	return( rc );
}
#endif
/****************************************************************************************************/
/*																									*/
/*		Overheat AD input																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Obtain the overheat AD input value.														*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG	KpiInputOHAd( CADC *pAdc )
{
	LONG InAd;
	tADC_REG *areg = CPU_ADC_REG0;

/*--------------------------------------------------------------------------------------------------*/
/*		Read A/D converter																			*/
/*--------------------------------------------------------------------------------------------------*/
		pAdc->Var[ADC3_OHIAD] = ADC_REG_RD( ADCR2, InAd ); 									/* 1Axis number*/
#if CSW_BOARD_TYPE == SGD7W_TYPE /* <S0A2> */
		pAdc->Var[ADC4_OHIAD] = ADC_REG_RD( ADCR3, InAd ); 									/* 2Axis number*/
#endif
/*--------------------------------------------------------------------------------------------------*/
		return (InAd);
}

#if (SVFSEL_ONLY_OH_ADC_USE == 0)			/* <S0E1>:SVFSEL_ONLY_OH_ADC_USEaddition					*/
/****************************************************************************************************/
/*																									*/
/*		Read speed command AD average value																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Speed command AD value read 4096 times and its average value(128x).								*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void TciReadVrefAD32Mean( LONG *AdMean )
{
	LONG	i;
	LONG	Adlo;
	LONG	Adhi;
	LONG	VAdBuf[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SVFSEL_ADC_USE == 0 )
		{
			return;										/* ObjectCodes Optimized by return			*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		AD read(4096times) : 30us * 4096 = 122.880 ms												*/
/*--------------------------------------------------------------------------------------------------*/

		VAdBuf[0] = VAdBuf[1] = 0;						/* Clear Vref AD Buffer						*/
		for( i=0; i < 0x1000; i++ )						/* Read Loop 4096 Times						*/
		{
			KlibWaitus( 30 );							/* Wait 30us for AD Conversion				*/
			KPX_REFAD_LATCH( );							/* Latch Vref AD Conv. Data			*/

#if	( SVFSEL_VREFAD_24BIT != 0 )	/* @SKLT@ */
			Adlo = (USHORT)(ASIC_RD( AREG_ACVLL ));				/* Read Low Word					*/
			Adhi = ((LONG)(ASIC_RD( AREG_ACVLH )<<24))>>8;		/* Read High Word					*/
#else		/* 16bit AD */
			Adlo = (USHORT)(((ASIC_HW_IF_STR *)_UDL_REG_ADDR_)->AREG_ACUL);	/* Interim */
			Adhi = ((( Adlo << 16 ) >> 16 ) & 0xFFFF0000 );		/* Sign extension					*/
#endif
			MlibAdd6432( VAdBuf, ((Adhi+Adlo)<<7), VAdBuf );	/* Add  Buffer (128*AdData)	*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Average value calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		*AdMean = (VAdBuf[1]<<20) | ((((ULONG)VAdBuf[0]>>11)+1)>>1);	/* Vref AD Mean				*/

	return;
}

/****************************************************************************************************/
/*																									*/
/*		Read the torque command AD average value																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Read the torque command AD value 4096 times and return the average value.									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void TciReadTrefAD16Mean( LONG AdMean[2] )
{
	LONG	i;
	LONG	Adlo;
	LONG	Adhi;
	LONG	TAdBuf[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SVFSEL_ADC_USE == 0 )
		{
			return;										/* ObjectCodes Optimized by return			*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		AD read(4096times) : 30us * 4096 = 122.880 ms												*/
/*--------------------------------------------------------------------------------------------------*/
/*		<1>CPUADofRepeat Mode, so it was set toTrefAdThe startup process for was removed.								*/
/*--------------------------------------------------------------------------------------------------*/
		TAdBuf[0] = TAdBuf[1] = 0;						/* Clear Tref AD Buffer						*/
		for( i=0; i < 0x1000; i++ )						/* Read Loop 4096 Times						*/
		{
			KlibWaitus( 30 );							/* Wait 30us for AD Conversion				*/
			TAdBuf[0] += KPX_READ_CPUAD0();				/* Read and Add Buf	(P-Tref)				*/
			TAdBuf[1] += KPX_READ_CPUAD1();				/* Read and Add Buf (N-Tref)				*/
			KlibWaitus( 18 );							/* Reset Watchdog and Wait					*/
		/*------------------------------------------------------------------------------------------*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Average value calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		AdMean[0] = (((TAdBuf[0]>>11)+1)>>1);							/* P-Tref AD Mean			*/
		AdMean[1] = (((TAdBuf[1]>>11)+1)>>1);							/* N-Tref AD Mean			*/

		return;

}
/* <S0CB>	End */

/****************************************************************************************************/
/*																									*/
/*		Command AD zero adjustment process(For offline/online use)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Adjustment data when input zero for each command AD(Offset amount when command input is zero)and			*/
/*			set each parameter.															*/
/*			If the offset amount is out of the following range, the offset amount will be treated as an adjustment error.				*/
/*			Make it zero.																			*/
/*																									*/
/*			1) T-ref P [Torque commandA/D Positive side] : -64 ≦ Offset ≦ 64	(64/4096=1.56%)						*/
/*																									*/
/*			2) T-ref N [Torque commandA/D Negative side] : -64 ≦ Offset ≦ 64	(64/4096=1.56%)						*/
/*																									*/
/*			3) V-ref   [Speed commandA/D]	 : -52500000 ≦ Offset ≦ 52500000	(24Bit AD In the case of)		*/
/*										 : -200000   ≦ Offset ≦ 200000		(16Bit AD In the case of)		*/
/*																									*/
/*											24Bit AD In the case of ==> (52500000/(2^23*128)=4.889%)		*/
/*											16Bit AD In the case of ==> (  200000/(2^15*128)=4.768%)		*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	TciAdjustRefAdZero( CADC *pAdc )
{
LONG	Zerox;
LONG	Zeroy;
LONG	AdMean[3];
SHORT	XSat,YSat;
/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SVFSEL_ADC_USE == 0 )
		{
			return;										/* ObjectCodes Optimized by return			*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		AD average value read																			*/
/*--------------------------------------------------------------------------------------------------*/
		KPX_SET_ADSEL_TREF( );							/* Select Tref AD Select Switch				*/
		KlibWaitms( 1 );								/* Reset Watchdog and Wait					*/
		TciReadTrefVrefADMean( AdMean );				/* Read P-Tref, N-Tref, Vref AD Mean		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Torque command zero adjustment data calculation & storage																*/
/*--------------------------------------------------------------------------------------------------*/
		Zerox = -AdMean[0] + TREFZVADP;					/* Make  P-Tref Adjust Zero Data			*/
		//if( (Zerox < -64) || (Zerox > 64) )				/* @@ (10bit->12bit chk!!)    Check P-Tref Adjust Zero Data			*/
		if( (Zerox < -127) || (Zerox > 127) )				/* @@ (10bit->12bit chk!!)    Check P-Tref Adjust Zero Data			*/ /* <S169> */
		{
			pAdc->f.AdjustError = TRUE;
		}
	/*----------------------------------------------------------------------------------------------*/
		Zeroy = +AdMean[1] - TREFZVADN;					/* Make  N-Tref Adjust Zero Data			*/
		//if( (Zeroy < -64) || (Zeroy > 64) )				/* @@ (10bit->12bit chk!!)    Check N-Tref Adjust Zero Data			*/
		if( (Zeroy < -127) || (Zeroy > 127) )				/* @@ (10bit->12bit chk!!)    Check N-Tref Adjust Zero Data			*/ /* <S169> */
		{
			pAdc->f.AdjustError = TRUE;
		}
	/*----------------------------------------------------------------------------------------------*/
		XSat = YSat = FALSE;

		if( Zerox >= TREFZVADP )						/* Check P-Tref Adjust Data is Saturated	*/
		{
			XSat = TRUE;
		}
		if( Zeroy <= -(TREFZVADN) )						/* Check N-Tref Adjust Data is Saturated	*/
		{
			YSat = TRUE;
		}
	/*----------------------------------------------------------------------------------------------*/
		if ( pAdc->f.AdjustError )
		{
			Zerox = 0;
			Zeroy = 0;
		}
		else if(XSat == TRUE && YSat == FALSE)
		{
			Zerox = Zeroy;
		}
		else if(XSat == FALSE && YSat == TRUE)
		{
			Zeroy = Zerox;
		}
		else if(XSat == TRUE && YSat == TRUE)
		{
			pAdc->f.AdjustError = TRUE;
			Zerox = 0;
			Zeroy = 0;
		}
		pAdc->P.ZadjTrqP = Zerox;								/* set Zero Adjust Variable */
		pAdc->P.ZadjTrqN = Zeroy;								/* set Zero Adjust Variable */
		Zerox = (Zerox & 0xFF) + (Zeroy << 8);					/* Combine P-Tref and N-Tref		*/
//TODO:		Prm.zadjtp_n.w = (USHORT)Zerox;							/* Write Adjust Data to RAM			*/
//TODO:EEEPROM	KriWriteEepromPrm( Prm.zadjtp_n.w, &pndef_zadjtp_n );	/* Write Adjust Data to EEPROM		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate and store speed command zero adjustment data																*/
/*--------------------------------------------------------------------------------------------------*/
#if	( SVFSEL_VREFAD_24BIT != 0 )
		Zerox = -AdMean[2];									/* Make  Vref Adjust Zero Data			*/
		if ( (Zerox < -52500000) || (Zerox > 52500000) )	/* Check Vref Adjust Zero Data			*/
		{
			Zerox = 0;
			KnlV.f.AdjustError = TRUE;
		}
#else
/*--------------------------------------------------------------------------------------------------*/
/*		-240000~+240000, make an error:	(15.36/0x7FFF)×(240000/128) = 879mV			*/
/*																									*/
/*		Offset adjustment range is not specified in the manual±750mVHowever,										*/
/*		Fn00A, the offset adjustment range is15000±240000,		*/
/*--------------------------------------------------------------------------------------------------*/
		Zerox = -AdMean[2];									/* Make  Vref Adjust Zero Data			*/
		if ( (Zerox < -240000) || (Zerox > 240000) )		/* Check Vref Adjust Zero Data			*/
		{
			Zerox = 0;
			pAdc->f.AdjustError = TRUE;
		}
#endif
	/*----------------------------------------------------------------------------------------------*/
		pAdc->P.ZadjVelo = Zerox;
//TODO:		Prm.zadjvl = (USHORT)Zerox;						/* Write Adjust Data to RAM					*/
//TODO:		Prm.zadjvh = (USHORT)(Zerox>>16);				/* Write Adjust Data to RAM					*/
//TODO:EEPROM		KriWriteEepromPrm( Zerox, &pndef_zadjvl );		/* Write Adjust Data to EEPROM				*/

		return;
}

/****************************************************************************************************/
/*																									*/
/*		Torque command AD & speed command AD average value reading(For offline/online use)							*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Read the torque command AD value and speed command AD value 4096 times and return the average value.				*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	TciReadTrefVrefADMean( LONG AdMean[3] )
{
LONG	i;
LONG	Adlo;
LONG	Adhi;
LONG	TAdBuf[2];
LONG	VAdBuf[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Check for function presence/absence																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SVFSEL_ADC_USE == 0 )
		{
			return;										/* ObjectCodes Optimized by return			*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		AD read(4096times) : 30us * 4096 = 122.880 ms												*/
/*--------------------------------------------------------------------------------------------------*/
/*		<1>CPUADofRepeat Mode, so it was set toTrefAdThe startup process for was removed.								*/
/*--------------------------------------------------------------------------------------------------*/
		TAdBuf[0] = TAdBuf[1] = 0;						/* Clear Tref AD Buffer						*/
		VAdBuf[0] = VAdBuf[1] = 0;						/* Clear Vref AD Buffer						*/
		for( i=0; i < 0x1000; i++ )						/* Read Loop 4096 Times						*/
		{
			KlibWaitus( 30 );							/* Wait 30us for AD Conversion				*/
			TAdBuf[0] += KPX_READ_CPUAD0();				/* Read and Add Buf	(P-Tref)				*/
			TAdBuf[1] += KPX_READ_CPUAD1();				/* Read and Add Buf (N-Tref)				*/
			KlibWaitus( 18 );							/* Reset Watchdog and Wait					*/
		/*------------------------------------------------------------------------------------------*/
			KPX_REFAD_LATCH( );									/* Latch Vref AD Conv. Data			*/

#if	( SVFSEL_VREFAD_24BIT != 0 )	/* @SKLT@ */
			Adlo = (USHORT)(ASIC_RD( AREG_ACVLL ));				/* Read Low Word					*/
			Adhi = ((LONG)(ASIC_RD( AREG_ACVLH )<<24))>>8;		/* Read High Word					*/
			MlibAdd6432( VAdBuf, ((Adhi+Adlo)<<7), VAdBuf );	/* Add  Buffer	(128*AdData)		*/
#else		/* 16bit AD */
			Adlo = (SHORT)(((ASIC_HW_IF_STR *)_UDL_REG_ADDR_)->AREG_ACUL);	/* Interim */
			Adhi = ((( Adlo << 16 ) >> 16 ) & 0xFFFF0000 );		/* <VZADJ>Sign extension					*/
#endif
			MlibAdd6432( VAdBuf, ((Adhi+Adlo)<<7), VAdBuf );	/* <V031>Add  Buffer (128*AdData)	*/
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Average value calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
		AdMean[0] = (((TAdBuf[0]>>11)+1)>>1);							/* P-Tref AD Mean			*/
		AdMean[1] = (((TAdBuf[1]>>11)+1)>>1);							/* N-Tref AD Mean			*/
	 	AdMean[2] = (VAdBuf[1]<<20) | ((((ULONG)VAdBuf[0]>>11)+1)>>1);	/* Vref AD Mean				*/

		return;

}

/****************************************************************************************************/
/*																									*/
/*		Addition(64bit+32bit)																			*/
/*																									*/
/*		TODO: Erase after																				*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#define	ADD6432( a, b, xx )		{							/* r0: *a, r1: b, r2: *x */			\
									long wk1; 				/* For work */						\
									long wk2; 				/* For work */						\
									long wk3; 				/* For work */						\
									__asm 														\
									{															\
										LDR wk1, [a,0] 		;/* 1:wk1 <- a[0]   */				\
										LDR wk2, [a,4] 		;/* 2:wk2 <- a[1]   */				\
 										MOV wk3, b			;/* 3:wk3 <- b                   */	\
										ASRS wk3, wk3, 31	;/* 4:wk3 <- (b>>31) = 0/-1      */	\
										ADDS wk1, wk1, b	;/* 5:wk1 <- a[0] + b            */	\
										ADCS wk2, wk2, wk3	;/* 6:wk2 <- a[1] + wk3 + carry  */	\
										STR wk1, [xx,0]		;/* 7:x[0] <- wk1  */				\
										STR wk2, [xx,4]		;/* 8:w[1] <- wk2  */				\
									}															\
								}
void	MlibAdd6432( LONG *a, LONG b, LONG *x )
{

		ADD6432( a, b, x );

		return;

}


#endif
#endif/* (SVFSEL_ADC_USE == 1)||(SVFSEL_ONLY_OH_ADC_USE == 1) */
/***************************************** end of file **********************************************/

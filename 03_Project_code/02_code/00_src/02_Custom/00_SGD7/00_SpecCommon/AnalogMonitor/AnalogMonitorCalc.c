/****************************************************************************************************/
/*																									*/
/*																									*/
/*		AnalogMonitorCalc.c : Parameter calculation process definition	<S016>										*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Parameter calculation process																		*/
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
/*	Note	:	First edition	2013.10.02	K.Sakamoto	For Mercury											*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "XlsAmonDef.h"
#include "AnalogMonitorCalc.h"
#include "DataTraceCalc.h"
#include "GainChange.h"				/* GAIN1			*/
#include "KnlApi.h"					/* KPI_SBCYCLEUS	*/
#include "Memory.h"					/* <S074> */

/****************************************************************************************************/
/*																									*/
/*			Analog monitor variable acquisition function																*/
/*																									*/
/*			Return numerical data																		*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		00 :Motor rotation speed																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonMotSpd( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->MotSts.MotSpd);/*<S0EC>*/
//	return (((AXIS_HANDLE*)Axis)->UnMonitor->AmonSpdRef);/*<S0CD>*/
}
/*--------------------------------------------------------------------------------------------------*/
/*		01 :Speed command																				*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_AmonSpdRef( void *Axis )		Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		02 :Torque command																				*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_AmonTrqRef( void *Axis )		Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		03:Position deviation																					*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_AmonPosErra( void *Axis )	Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		04:Position amplifier deviation																			*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_AmonPosErrx( void *Axis )	Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		05:Position command speed																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmondPosCmdSmp( void *Axis )
{
	//return (((AXIS_HANDLE*)Axis)->BaseControls->PosManager.var.dPcmda);
	return (((AXIS_HANDLE*)Axis)->BaseControls->AmondPosCmd);	/* <S1B6> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		07:Motor load positional deviation																		*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_sfperra_per32s( void *Axis )	Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		09:Speed feedforward																	*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_SpdFFCx( void *Axis )		Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		0A:Torque feedforward																	*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_TrqFFCx( void *Axis )		Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		0B:Valid gain																				*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_AmonActGain( void *Axis )	Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		0D:External encoder speed																		*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonFencSpdsmp( void *Axis )
{
//	return (((AXIS_HANDLE*)Axis)->FencV->MotSpd);		/* <S04D>: Match processing to motor rotation speed	*/
	return (((AXIS_HANDLE*)Axis)->BaseControls->MotSts.FencSpd);
}
/*--------------------------------------------------------------------------------------------------*/
/*		40:Position command speed before position loop				Defined in data trace(Common)	<S187>				*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_dPosRefi( void *Axis )
//{
//	/* FIXME:Provisional measures */
//	return (((AXIS_HANDLE*)Axis)->BaseControls->PosManager.var.dPcmda);
//}
/*--------------------------------------------------------------------------------------------------*/
/*		41:Friction compensation torque																			*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_AmonDisTrq( void *Axis )		Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		42:Vibration suppression monitor																				*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_MonResVib( void *Axis )		Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		43:Torque command before filter																		*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrqRefBeforeComp( void *Axis )
{
	/* FIXME:Provisional measures	SGDV:BoutV.TrqRefBeforeComp */
	//return 0;															/* <S14B> */
	//return (((AXIS_HANDLE*)Axis)->BaseLoops->RippleCmp.AmonTrqComp1);	/* <S14B> */
	return (((AXIS_HANDLE*)Axis)->BaseLoops->RippleCmp.TrqRefBeforeComp);	/* <S14B> *//* <S1FD> */
}

/*--------------------------------------------------------------------------------------------------*/
/*		44:dAxis current command																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonIdRef( void *Axis )
{
	/* FIXME:Provisional measures	SGDV:BoutV.AmonIdRef */
	return 0;
}
/*--------------------------------------------------------------------------------------------------*/
/*		****:Dummy variable																				*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	GetVarData_Dummy( void *Axis )			Defined in data trace(Common)
//
/*--------------------------------------------------------------------------------------------------*/
/*		<S074> 0xF0:Output Parameter Specified RAM Address											*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_GETRAMADDR( ax, ch )		(((AXIS_HANDLE *)(ax))->AmonOut[(ch)].Vaddr)
#define	ANLG_MON_CH0_SEL	0x0001		/* Analog monitor output voltage test mode : Ch0Selection */	/* <S0FD> */
#define	ANLG_MON_CH1_SEL	0x0002		/* Analog monitor output voltage test mode : Ch1Selection */	/* <S0FD> */

LONG GetVarData_AmonAdrOutCH0H( void *Axis ) { return (*( (USHORT *)LPX_GETRAMADDR( Axis, 0 ))); }
LONG GetVarData_AmonAdrOutCH0W( void *Axis ) { return (*(  (ULONG *)LPX_GETRAMADDR( Axis, 0 ))); }
LONG GetVarData_AmonAdrOutCH1H( void *Axis ) { return (*( (USHORT *)LPX_GETRAMADDR( Axis, 1 ))); }
LONG GetVarData_AmonAdrOutCH1W( void *Axis ) { return (*(  (ULONG *)LPX_GETRAMADDR( Axis, 1 ))); }



/****************************************************************************************************/
/*																									*/
/*			Analog monitor bit variable acquisition function														*/
/*																									*/
/*			Return the bit data of the specified axis.															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		08:Positioning completion command																			*/
/*--------------------------------------------------------------------------------------------------*/
//BOOL	GetBitData_Coin( void *Axis )			Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		0C:Command dispensing complete																			*/
/*--------------------------------------------------------------------------------------------------*/
//BOOL	GetBitData_Den( void *Axis )			Defined in data trace(Common)
/*--------------------------------------------------------------------------------------------------*/
/*		****:Dummy variable																				*/
/*--------------------------------------------------------------------------------------------------*/
//BOOL	GetBitData_Dummy( void *Axis )
//{
//	return FALSE;
//}

/****************************************************************************************************/
/*																									*/
/*			Analog monitor gain calculation function															*/
/*																									*/
/****************************************************************************************************/

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Dummy														*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalDummy( void *Axis )
{
		return( 0x00000000 );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Motor speed,Speed command (2^24/OvrSpd --> 1V/1000min-1)			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Motor rotation speed, speed FF, vibration damping monitor constant speed relationship(min-1unit)of								*/
/*			Analog monitor output gain calculation														*/
/*																									*/
/*					   OvrSpd		 60		    1			  1			   1						*/
/*			Mgain = ------------ * ------- * -------- = -------------- * ------						*/
/*					 0x01000000     2*PAI	   1000		 Bprm.Knorspd	  1000						*/
/*																									*/
/*			OvrSpd   : Maximum speed [rad/s]																*/
/*			Knorspd	 : Normalized speed calculation coefficient(r/min,mm/s --> NormalizedSpeed)							*/
/*			Voltage conversion : 1V/1000rpm = 1/1000															*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalMotorSpeed( void *Axis )
{
	LONG	kx;		/* Gain	*/
	LONG	sx;		/* Number of shifts	*/

	kx = MlibScalKxgain( 1, 1, 1000, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 1, ((AXIS_HANDLE*)Axis)->UniPrms->Bprm->Knorspd, &sx, -1 );
	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Motor rotation speed (dMotPos/ScanB --> 1V/1000min-1)				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Motor rotation speed(min-1unit)Gain calculation for analog monitor output								*/
/*																									*/
/*			MonSpd = Mgain * BinK.dMotPos		(BinK.dMotPos : pulse/ScanB)						*/
/*																									*/
/*					  Hprm.Kmotspd	    1000000		  1												*/
/*			Mgain  = -------------- * ----------- * ------											*/
/*					  Bprm.Knorspd	   SBCYCLEUS	 1000											*/
/*																									*/
/*			Kmotspd   : Motor speed calculation gain:[pulse/sec]  --> [2^24/OvrSpd]						*/
/*			Knorspd	  : Normalized speed calculation coefficient  :[r/min,mm/s] --> [NormalizedSpeed]					*/
/*			SBCYCLEUS : Scan B cycle time [us]												*/
/*			Voltage conversion  : 1V/1000rpm = 1/1000															*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalMotPfbkSpd( void *Axis )
{
	LONG	kx;		/* Gain				*/
	LONG	sx;		/* Number of shifts				*/
	LONG	plsno;	/* Motor speed calculation gain	*/
//	LONG	Egear_a;	/* Electronic gear denominator			*/	/* <S1A7> *//* <S203> */
//	LONG	Egear_b;	/* Electronic gear numerator			*/	/* <S1A7> *//* <S203> */

	/* FIXME:Provisional measures */
#if 0
	if( Kprm.f.FencUse )
	{
		plsno = Hprm.KmotspdFC;
	}
	else
	{
		plsno = Hprm.Kmotspd;
	}
#else
	plsno = ((AXIS_HANDLE*)Axis)->UniPrms->Bprm->Kmotspd;
#endif

	kx = MlibScalKxgain( 1, 1000, KPI_SBCYCLEUS, &sx, 0 );
	kx = MlibPcalKxksks( kx, plsno, ((AXIS_HANDLE*)Axis)->UniPrms->Bprm->Knorspd, &sx, -1 );

/* <S1A7> */
//	Egear_a = ((AXIS_HANDLE*)Axis)->BaseControls->PosManager.conf.Egear.a;/* <S203> */
//	Egear_b = ((AXIS_HANDLE*)Axis)->BaseControls->PosManager.conf.Egear.b;/* <S203> */
//	kx = MlibPcalKxgain( kx, Egear_b, Egear_a, &sx, -1 );/* <S203> */
/* <S1A7> */

	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Position command speed (dPosCmda/ScanB --> 1V/1000min-1)				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Position command speed(min-1unit)Gain calculation for analog monitor output								*/
/*																									*/
/*			MonSpd = Mgain * dPosCmda			(dPosCmda : pulse/ScanB)							*/
/*																									*/
/*					  Iprm.Egear.b     Hprm.Kmotspd	    1000000	       1							*/
/*			Mgain  = -------------- * -------------- * ----------- * ------							*/
/*					  Iprm.Egear.a     Bprm.Knorspd	    SBCYCLEUS     1000							*/
/*																									*/
/*			Kmotspd   : Motor speed calculation gain:[pulse/sec]  --> [2^24/OvrSpd]						*/
/*			Knorspd	  : Normalized speed calculation coefficient  :[r/min,mm/s] --> [NormalizedSpeed]					*/
/*			SBCYCLEUS : Scan B cycle time [us]												*/
/*			Voltage conversion  : 1V/1000rpm = 1/1000															*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalPosCmdaSpd( void *Axis )
{
	LONG	kx;			/* Gain				*/
	LONG	sx;			/* Number of shifts				*/
	LONG	plsno;		/* Motor speed calculation gain	*/
	LONG	Egear_a;	/* Electronic gear denominator			*/
	LONG	Egear_b;	/* Electronic gear numerator			*/

	/* FIXME:Provisional measures */
#if 0
	if( Kprm.f.FencUse )
	{
		plsno = Hprm.KmotspdFC;
	}
	else
	{
		plsno = Hprm.Kmotspd;
	}
#else
	plsno = ((AXIS_HANDLE*)Axis)->UniPrms->Bprm->Kmotspd;
#endif

	kx = MlibScalKxgain( 1, 1000, KPI_SBCYCLEUS, &sx, 0 );
	kx = MlibPcalKxksks( kx, plsno, ((AXIS_HANDLE*)Axis)->UniPrms->Bprm->Knorspd, &sx, 0 );

	/* FIXME:Provisional measures */
#if 0
#if( LESS_DEVIATION_CTRL != 0 )
	/* (Deviation-less control disabled && Model following control disabled) || PJOGEnabled */
	if( (Kprm.f.NctErrLessUse == FALSE) && (Gprm.MFCModel == 0) || (BoutV.CtrlMode.us[0] == CTRLMODE_PJOG) )
	{	
		kx = MlibPcalKxgain( kx, Iprm.Egear.b, Iprm.Egear.a,  &sx, -1 );
	}
	else											/* Other than the above */
	{
		kx = MlibPcalKxgain( kx, Iprm.NctEgear.b, Iprm.NctEgear.a,  &sx, -1 );
	}
#else
	kx = MlibPcalKxgain( kx, Iprm.Egear.b, Iprm.Egear.a,  &sx, -1 );
#endif
#else
	Egear_a = ((AXIS_HANDLE*)Axis)->BaseControls->PosManager.conf.Egear.a;
	Egear_b = ((AXIS_HANDLE*)Axis)->BaseControls->PosManager.conf.Egear.b;
	kx = MlibPcalKxgain( kx, Egear_b, Egear_a, &sx, -1 );
#endif

	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Torque command (2^24/MaxTrq --> 1V/100%)							*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Torque FF, model torque command, disturbance observer, estimated disturbance torque, etc.						*/
/*			Analog monitor output gain calculation														*/
/*																									*/
/*					   MaxTrq		 1																*/
/*			Mgain = ------------ * -----															*/
/*					 0x01000000     100																*/
/*																									*/
/*			MaxTrq   : Maximum torque [%]																*/
/*			Voltage conversion : 0.01V/% = 1/100																*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalTorqueRef( void *Axis )
{
	LONG	kx;		/* Gain				*/
	LONG	sx;		/* Number of shifts				*/

	kx = MlibScalKxgain( ((AXIS_HANDLE*)Axis)->UniPrms->Bprm->PerMaxTrq, 1, 100, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, 0x01000000, &sx, -1 );
	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Torque command (15000/MaxTrq --> 1V/100%)						*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Gain calculation for analog monitor output of torque command (final q-axis current command after current limit).				*/
/*																									*/
/*					   MaxTrq		 1																*/
/*			Mgain = ------------ * -----															*/
/*					   15000		100																*/
/*																									*/
/*			MaxTrq   : Maximum torque [%]																*/
/*			Voltage conversion : 0.01V/% = 1/100																*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalTrqRef15K( void *Axis )
{
	LONG	kx;		/* Gain				*/
	LONG	sx;		/* Number of shifts				*/

	kx = MlibScalKxgain( ((AXIS_HANDLE*)Axis)->UniPrms->Bprm->PerMaxTrq, 1, (100 * 15000), &sx, -1 );
	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Position deviation (0.05V/1Command unit)									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Gain calculation for analog monitor output of position deviation.												*/
/*																									*/
/*					   1																			*/
/*			Mgain = ------																			*/
/*					  20																			*/
/*																									*/
/*			Voltage conversion : 0.05V/1pulse = 1/20															*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalPulseError( void *Axis )
{
	LONG	kx;		/* Gain				*/
	LONG	sx;		/* Number of shifts				*/

	kx = MlibScalKxgain( 1, 1, 20, &sx, -1 );
	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Motor load positional deviation (0.01V/1Command unit)						*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Gain calculation for analog monitor output of position deviation.												*/
/*																									*/
/*					  1																				*/
/*			Mgain = -----																			*/
/*					 100																			*/
/*																									*/
/*			Voltage conversion : 0.01V/1pulse = 1/100															*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalPulseErr01( void *Axis )
{
	LONG	kx;		/* Gain				*/
	LONG	sx;		/* Number of shifts				*/

	kx = MlibScalKxgain( 1, 1, 100, &sx, -1 );
	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Bit signal (ON:5V, OFF:0V)									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Gain calculation for analog monitor output of bit signal.											*/
/*					 																				*/
/*			Mgain = 5																				*/
/*																									*/
/*			Voltage conversion : 5V/1 = 5																		*/
/*			Input signal : ON = 1, OFF = 0																*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalBitSignal( void *Axis )
{
	LONG	kx;		/* Gain				*/
	LONG	sx;		/* Number of shifts				*/

	kx = MlibScalKxgain( 1, 5, 1, &sx, -1 );
	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : Valid gain (No.1:1V, No.2:2V, No.3:3V, No.4:4V)					*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Gain calculation for analog monitor output of valid gain. Support automatic gain switching time constant			*/
/*																									*/
/*						1																			*/
/*			Mgain = --------- 																		*/
/*					  GAIN1																			*/
/*																									*/
/*			Voltage conversion : 1V/GAIN1																		*/
/*			Input signal : No.1:GAIN1=2^22, No.2:GAIN2=2*(2^22), No.3:GAIN3=3*(2^22), No.4:GAIN4=4*(2^22)	*/
/*																									*/
/****************************************************************************************************/
KSGAIN	AmonGcalActGain( void *Axis )
{
	LONG	kx;		/* Gain				*/
	LONG	sx;		/* Number of shifts				*/

	kx = MlibScalKxgain( -1, 1, GAIN1, &sx, -1 );
	return( kx );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Analog monitor parameters(Variable addresses,Data size,Output gain)Perform calculation.			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*	Supplementary1 :	Analog monitor HW specification																	*/
/*--------------------------------------------------------------------------------------------------*/
/*			 1.000V : 3369.976																		*/
/*			10.000V : 33699.76																		*/
/*--------------------------------------------------------------------------------------------------*/
/*	Supplementary2 :	Analog monitor signal selection/offset voltage													*/
/*--------------------------------------------------------------------------------------------------*/
/*			  CH0	    CH1		  Explanation																*/
/*		   -----------------------------------------------------------------------------			*/
/*			Pn006.0 / Pn007.0 : Analog monitor signal selection												*/
/*			Pn006.1 / Pn007.1 : Analog monitor signal selection												*/
/*			Pn006.2 / Pn007.2 : Preliminary																*/
/*			Pn006.3 / Pn007.3 : Preliminary																*/
/*			Pn550   / Pn551   : Offset voltage (-1000.0~+1000.0V)									*/
/*			Pn552   / Pn553   : magnification																*/
/*--------------------------------------------------------------------------------------------------*/
/*	Supplementary3 : Analog monitor output gain & shift magnification													*/
/*--------------------------------------------------------------------------------------------------*/
/*			CH0 : PnE42= n.XAAA  X(Left shift number),AAA(Gain)											*/
/*			CH1 : PnE43= n.YBBB  Y(Left shift number),BBB(Gain)											*/
/*--------------------------------------------------------------------------------------------------*/
/*	Supplementary4 : Analog monitor output zero adjustment & gain adjustment														*/
/*--------------------------------------------------------------------------------------------------*/
/*			PnE58 : Analog monitor output zero adjustment(Lo:CH0,Hi:CH1)											*/
/*			PnE59 : Analog monitor output gain adjustment(Lo:CH0,Hi:CH1)										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#define AMON_OFFSET_P1V		337						/* Offset    [0.1V=337]							*/
#define	AMON_HWGAIN_1000V	3369976					/* H/W Gain  [1000V=3369976]					*/
/*--------------------------------------------------------------------------------------------------*/
void	PcalAnalogMonitorGain( AXIS_HANDLE *Axis, LONG chx )
{
	LONG	Index;
	LONG	Gsftx;
	LONG	Gadjx;
	LONG	Xgain;
	LONG	Mgain;
	LONG	AmonPrmx;
	AMONOUT	*AmonOutx;								/* Pointer to analog monitor data			*/
	PRMDATA	*Prm;									/* Pointer to parameter setting data			*/
	CMN_PRMDATA	*Cprm;								/* Pointer to unit common parameter setting data	*/

	/* Initializing variables	*/
	Prm = Axis->UniPrms->Prm;						/* Parameter setting data			*/
	Cprm = Axis->UniPrms->Cprm;						/* Unit common parameter setting data	*/

/*--------------------------------------------------------------------------------------------------*/
/*		CH selection processing																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( chx == 0 )									/* Analog monitor CH0						*/
	{
//		AmonPrmx = Prm->b_prm6;						/* CH0 Parameter							*/
		AmonPrmx = Cprm->b_prm6;					/* CH0 Parameter							*//* 2Axis analog monitor compatible<S103> */
//		Xgain = (LONG)((SHORT)Prm->mongain1);		/* CH0 Gain Parameter						*//* <S0DF> */
		Xgain = (LONG)((SHORT)Cprm->mongain1);		/* CH0 Gain Parameter						*//* <S0DF> */
		AmonOutx = (AMONOUT*)&(Axis->AmonOut[0]);	/* OutData Pointer							*/
		Gadjx = (CHAR)Cprm->mnag0_1.b.l;				/* Gain Adjust Data							*/
//		Gsftx = -((USHORT)Prm->mnrng0 >> 12);		/* MlibPcalKxgain()(sx)The sign is reversed	*//* <S0DF> */
		Gsftx = -((USHORT)Cprm->mnrng0 >> 12);		/* MlibPcalKxgain()(sx)The sign is reversed	*//* <S0DF> */
	}
	/*----------------------------------------------------------------------------------------------*/
	else											/* Analog monitor CH1		*/
	{
//		AmonPrmx = Prm->b_prm7;						/* CH1 Parameter							*/
		AmonPrmx = Cprm->b_prm7;					/* CH1 Parameter							*//* 2Axis analog monitor compatible<S103> */
//		Xgain = (LONG)((SHORT)Prm->mongain2);		/* CH1 Gain Parameter						*//* <S0DF> */
		Xgain = (LONG)((SHORT)Cprm->mongain2);		/* CH1 Gain Parameter						*//* <S0DF> */
		AmonOutx = (AMONOUT*)&(Axis->AmonOut[1]);	/* OutData Pointer							*/
		Gadjx = (CHAR)Cprm->mnag0_1.b.h;				/* Gain Adjust Data							*/
//		Gsftx = -((USHORT)Prm->mnrng1 >> 12);		/* MlibPcalKxgain()(sx)The sign is reversed	*//* <S0DF> */
		Gsftx = -((USHORT)Cprm->mnrng1 >> 12);		/* MlibPcalKxgain()(sx)The sign is reversed	*//* <S0DF> */
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of analog monitor output gain common part														*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*				   Magni * (Gadjx + 256) * (2^Gsftx)	  -AMON_HWGAIN_1V * Func().rv				*/
/*		Mgain = -------------------------------------- * -----------------------------				*/
/*				    100	 *		256									1								*/
/*																									*/
/*		Magni : Analog monitor signal magnification	 [1/100]	--- Pn552 / Pn553(SGDV)							*/
/*		Gadjx : Analog monitor gain adjustment	 [1/256]	---	PnE59(Lo/Hi)								*/
/*		Gsftx : Analog monitor shift magnification [-]		---	PnE42.3 / PnE43.3							*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Analog monitor output gain calculation: Undefined														*/
/*--------------------------------------------------------------------------------------------------*/
	if( (Index = LpxSearchAmonDefTable( AmonPrmx & 0xFF )) < 0 )
	{
		AmonOutx->Enable = FALSE;					/* Writing in progress	*/
		AmonOutx->Mgain = 0x0000;
		AmonOutx->Vtype = XAMDEF_VxLONG;
		AmonOutx->GetVarData = GetVarData_Dummy;
		AmonOutx->Enable = TRUE;					/* Writing completed	*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Analog monitor output gain calculation:PrmSpecified variable													*/
/*--------------------------------------------------------------------------------------------------*/
	else if( AmonDefTbl[Index].Vtype == XAMDEF_VxPRMVAR )
	{
		LONG  Hgain = MlibPcalKxgain( Xgain, (Gadjx + 256), (100*256), &Gsftx, -1 );
		LpxGcalPrmVariable( Axis, AmonOutx, Hgain );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Analog monitor output gain calculation:XlsDefinitionBitVariables												*/
/*--------------------------------------------------------------------------------------------------*/
	else if( AmonDefTbl[Index].Vtype == XAMDEF_VxBIT )
	{
	/*------------------------------------------------------------------------------------------*/
		Mgain = AmonDefTbl[Index].GetVarGain( Axis );
		Mgain = MlibPcalKskxkx( Mgain, Xgain * (Gadjx + 256), (100*256), &Gsftx,   0 );
		Mgain = MlibPcalKxgain( Mgain,  AMON_HWGAIN_1000V,     1000,     &Gsftx, -24 );
	/*------------------------------------------------------------------------------------------*/
		AmonOutx->Enable = FALSE;					/* Writing in progress	*/
		AmonOutx->Mgain = Mgain;
		AmonOutx->Vtype = XAMDEF_VxBIT;
		AmonOutx->GetVarData = AmonDefTbl[Index].GetVarData;
		AmonOutx->Enable = TRUE;					/* Writing completed	*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Analog monitor output gain calculation:XlsDefinitionLong/ShortVariables											*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		Mgain = AmonDefTbl[Index].GetVarGain( Axis );
		Mgain = MlibPcalKskxkx( Mgain, Xgain * (Gadjx + 256), (100*256), &Gsftx,   0 );
		Mgain = MlibPcalKxgain( Mgain, -AMON_HWGAIN_1000V,     1000,     &Gsftx,  24 );
	/*------------------------------------------------------------------------------------------*/
		AmonOutx->Enable = FALSE;					/* Writing in progress	*/
		AmonOutx->Mgain = Mgain;
		AmonOutx->Vtype = AmonDefTbl[Index].Vtype;
		AmonOutx->GetVarData = AmonDefTbl[Index].GetVarData;
		AmonOutx->Enable = TRUE;					/* Writing completed	*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output offset calculation															*/
/*																									*/
/****************************************************************************************************/
void	PcalAnalogMonitorOffset( AXIS_HANDLE *Axis, LONG chx )
{
	LONG	Offset;
//	PRMDATA	*Prm;									/* Pointer to parameter setting data				*//* <S0DF> */
	CMN_PRMDATA	*Cprm;								/* Pointer to unit common parameter setting data	*/

	/* Initializing variables	*/
//	Prm = Axis->UniPrms->Prm;						/* Parameter setting data							*//* <S0DF> */
	Cprm = Axis->UniPrms->Cprm;						/* Unit common parameter setting data				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Analog Monitor Offset Cal.																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( chx == 0 )
	{
//		Offset = (Cprm->mnaz0_1.b.l<<6) - 1302 - ((SHORT)Prm->monoffset1 * AMON_OFFSET_P1V);		  /* <S0DF> */
//		Offset = (Cprm->mnaz0_1.b.l<<6) - 1302 - ((SHORT)Cprm->monoffset1 * AMON_OFFSET_P1V);		  /* <S0DF> */	/* <S100> */
		Offset = ((CHAR)Cprm->mnaz0_1.b.l<<6) - 1302 - ((SHORT)Cprm->monoffset1 * AMON_OFFSET_P1V);	  /* <S0DF> */	/* <S100> */
		((AMONOUT*)Axis->AmonOut)[0].Offset = Offset;
	}
	else
	{
//		Offset = (Cprm->mnaz0_1.b.h<<6) - 1302 - ((SHORT)Prm->monoffset2 * AMON_OFFSET_P1V);		  /* <S0DF> */
//		Offset = (Cprm->mnaz0_1.b.h<<6) - 1302 - ((SHORT)Cprm->monoffset2 * AMON_OFFSET_P1V);		  /* <S0DF> */	/* <S100> */
		Offset = ((CHAR)Cprm->mnaz0_1.b.h<<6) - 1302 - ((SHORT)Cprm->monoffset2 * AMON_OFFSET_P1V);	  /* <S0DF> */	/* <S100> */
		((AMONOUT*)Axis->AmonOut)[1].Offset = Offset;
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor definition table search																*/
/*																									*/
/****************************************************************************************************/
LONG	LpxSearchAmonDefTable( LONG SelNo )
{
	LONG	i;
	LONG	w;
	LONG	EntNum = AMONDEFTBL_ENTNUM;							

/*--------------------------------------------------------------------------------------------------*/
/*		Search startIndex																			*/
/*--------------------------------------------------------------------------------------------------*/
	w = (1 << MlibSrhbiton( EntNum - 1, SRH_FROM_MSB ));	/* Setting division width initial value					*/
	i = (SelNo >= AmonDefTbl[w].SelNo)? (EntNum - w) : 0;	/* Search startIndexSetting				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Table search																				*/
/*--------------------------------------------------------------------------------------------------*/
	for( w = w>>1; w > 0; w = w>>1 )						/* SearchLoop							*/
	{
		if( SelNo >= AmonDefTbl[i+w].SelNo ){ i = i + w;}	/* Comparison &IndexUpdate					*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Search result check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( SelNo != AmonDefTbl[i].SelNo )						/* Search result check					*/
	{
		i = -1;												/* Search failure(SelNoNo capacity)				*/
	}
	return( i );
}

/****************************************************************************************************/
/*																									*/
/*		Analog monitor output gain calculation : PrmSpecified variable													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Gain calculation that outputs the variable set by the following parameters to the analog monitor.					*/
/*						Addr																		*/
/*			No. 	(High)	(Low)	 Gain															*/
/*		------------------------------------------------------------------------					*/
/*			CH0		PnE49	PnE48	 PnE42= n.XAAA  X(Left shift number),AAA(Gain)							*/
/*			CH1		PnE4B 	PnE4A	 PnE43= n.YBBB  Y(Left shift number),BBB(Gain)							*/
/*		------------------------------------------------------------------------					*/
/*																									*/
/*																									*/
/*				  PnE42/PnE43(0~2digit)																*/
/*		Mgain  = ----------------------- * Hgain													*/
/*						  4096																		*/
/*																									*/
/*				 Magni     (Gadjx + 256)															*/
/*		Hgain = ------- * --------------- * (1 << PnE42.3/PnE43.3)									*/
/*				  100			 256																*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S074> */
void	LpxGcalPrmVariable( AXIS_HANDLE *Axis, AMONOUT *AmonOutx, LONG Hgain )
{
	LONG	Gainx;
	ULONG	MonAdr;
	/* FIXME:Provisional measures */
#if 0
	USHORT	*PxAsicData;
	VUSHORT	**PxAsicAddr;
#endif
	PRMDATA	*Prm;									/* Pointer to parameter setting data			*/

	/* Initializing variables	*/
	Prm = Axis->UniPrms->Prm;						/* Parameter setting data	*/


/*--------------------------------------------------------------------------------------------------*/
/*		Monitor CH individual data setting																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( AmonOutx == (AMONOUT*)&(Axis->AmonOut[0]) )				/* Monitor CH0							*/
	{
		MonAdr = Prm->mnptr0l;
		Gainx  = ((LONG)(Prm->mnrng0 << 20) >> 16);
/* FIXME:Provisional measures */
#if 0
		PxAsicData = &BinK.AsicMon0;
		PxAsicAddr = &Gprm.AsicMonAdr0;
#endif
	}
	/*----------------------------------------------------------------------------------------------*/
	else												/* Monitor CH1							*/
	{
		MonAdr = Prm->mnptr1l;
		Gainx  = ((LONG)(Prm->mnrng1 << 20) >> 16);
#if 0
		PxAsicData = &BinK.AsicMon1;
		PxAsicAddr = &Gprm.AsicMonAdr1;
#endif
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Setting variable addresses & variable sizes																*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
	switch( KpiCheckReadMemAddr( MonAdr, 1 ) )
	{
	case KPI_CHKRDMEM_NG:	/* No Variable Memory : Set to Dummy Variable						*/
		 *PxAsicAddr = (USHORT *)&ReadDummy.Ushort[0];
		 MonAdr = (ULONG)&ReadDummy.Ushort[0];
		 break;
	case KPI_CHKRDMEM_MREG:	/* ASIC Micro Register : ScanA Read Register synchronously			*/
		 *PxAsicAddr = (USHORT *)MonAdr;
		 MonAdr = (ULONG)PxAsicData;
		 break;
	default:				/* Other Variables : RAM Variable, ASIC H/W Register, etc	 		*/
		 *PxAsicAddr = (USHORT *)&ReadDummy.Ushort[0];
		 break;
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of monitor output gain																		*/
/*--------------------------------------------------------------------------------------------------*/
	Gainx = MlibScalKskxkx( Hgain, -Gainx, 4096, NULL, 24 );
/*--------------------------------------------------------------------------------------------------*/
/*		Setting of monitor output data																		*/
/*--------------------------------------------------------------------------------------------------*/
	AmonOutx->Enable = FALSE;			/* Writing in progress	*/
	AmonOutx->Mgain = Gainx;
	AmonOutx->Vtype = XAMDEF_VxSHORT;
//	AmonOutx->Vaddr = (void *)MonAdr;
	AmonOutx->GetVarData = GetVarData_Dummy;
	AmonOutx->Enable = TRUE;			/* Writing completed	*/
/*--------------------------------------------------------------------------------------------------*/
	return;
}
#endif
void	LpxGcalPrmVariable( AXIS_HANDLE *Axis, AMONOUT *AmonOutx, LONG Hgain )	/* <S074> */
{
LONG	Chx;
LONG	Sizex;
LONG	Gainx;
ULONG	MemAdr;
ULONG	*pVarAdr;
//PRMDATA	*pPrm; /* <S0DF> */
CMN_PRMDATA	*Cprm;								/* Pointer to unit common parameter setting data	*/
LONG	(*pFunc)(void *);

	/* Initializing variables	*/
//	pPrm = Axis->UniPrms->Prm; /* <S0DF> */
	Cprm = Axis->UniPrms->Cprm; /* <S0DF> */
	Chx = (( AmonOutx == (AMONOUT*)&(Axis->AmonOut[0]) ) ? 0 : 1 );

/*--------------------------------------------------------------------------------------------------*/
/*		Set MemAddr, Gain, etc																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( Chx == 0 )											/* Monitor CH: 0						*/
	{
//		pVarAdr = &(pPrm->mnptr0l);					/* <S0DF> */
//		Gainx  = ((LONG)(pPrm->mnrng0 << 20) >> 16);/* <S0DF> */
		pVarAdr = &(Cprm->mnptr0l);					/* <S0DF> */
		Gainx  = ((LONG)(Cprm->mnrng0 << 20) >> 16);/* <S0DF> */
	}
	/*----------------------------------------------------------------------------------------------*/
	else													/* Monitor CH: 1						*/
	{
//		pVarAdr = &(pPrm->mnptr1l);					/* <S0DF> */
//		Gainx  = ((LONG)(pPrm->mnrng1 << 20) >> 16);/* <S0DF> */
		pVarAdr = &(Cprm->mnptr1l);					/* <S0DF> */
		Gainx  = ((LONG)(Cprm->mnrng1 << 20) >> 16);/* <S0DF> */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Search Memory Address																		*/
/*--------------------------------------------------------------------------------------------------*/
	MemAdr = KpiFindMemNextAddress( *pVarAdr, (ULONG)(AmonOutx->Vaddr) );
	Sizex = KpiGetMemAccessSize( MemAdr );					/* Get AccessSize[byte] 				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Check Memory Address																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch ( KpiCheckReadMemAddr( MemAdr, Sizex>>1 ) )
	{
	case MEMDEF_READ_NG:									/* Read NG */
		MemAdr = (ULONG)&(ReadDummy.dw);					/* Dummy Address */
		pFunc = GetVarData_Dummy;							/* Set Dummy Function */
		*pVarAdr = MemAdr;									/* Update PnE48/PnE4A for Monitor */
		break;
	default:												/* Read OK */
		*pVarAdr = MemAdr;									/* Update PnE48/PnE4A for Monitor  */
	/*----------------------------------------------------------------------------------------------*/
	/*	Set Access Function																			*/
	/*----------------------------------------------------------------------------------------------*/
		switch( Chx )
		{
		case 0: 
			pFunc = (( Sizex == 4 ) ? GetVarData_AmonAdrOutCH0W : GetVarData_AmonAdrOutCH0H );
			break;
		case 1:
			pFunc = (( Sizex == 4 ) ? GetVarData_AmonAdrOutCH1W : GetVarData_AmonAdrOutCH1H );
			break;
		default: 
			pFunc = GetVarData_Dummy;
			break;
		}
	/*----------------------------------------------------------------------------------------------*/
		break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate Monitor Gain																		*/
/*--------------------------------------------------------------------------------------------------*/
	Gainx = MlibScalKskxkx( Hgain, -Gainx, 4096, NULL, 24 );

/*--------------------------------------------------------------------------------------------------*/
	AmonOutx->Enable = FALSE;												/* Writing in progress	*/
	AmonOutx->Mgain = Gainx;												/* Monitor Gain */
	AmonOutx->Vtype = ((Sizex == 4 ) ? XAMDEF_VxLONG : XAMDEF_VxSHORT);		/* @@ check */
	AmonOutx->GetVarData = pFunc;											/* Read Function */
	AmonOutx->Vaddr = (void *)MemAdr;										/* Memory Address */
	AmonOutx->Enable = TRUE;												/* Writing completed	*/
/*--------------------------------------------------------------------------------------------------*/
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Analog monitor output processing																		*/
/*																									*/
/****************************************************************************************************/
void	BpxOutputAnalogMonitor( AXIS_HANDLE *Axis )
{
	LONG	AmonCh0;
	LONG	AmonCh1;
	AMONOUT	*AmonOut;
	LONG	Lvalue;
	SHORT	Svalue;
	BOOL	Bvalue;
	FUN_CMN_CONTROL			*FnCmnCtrl;	/* <S0FD> */
	CMN_PRMDATA	*Cprm;					/* Pointer to unit common parameter setting data<S103>	*/
	LONG 	ax_No;						/* SGD7W <S103> */

	AmonOut = (AMONOUT*)&(Axis->AmonOut[0]);
	FnCmnCtrl = Axis->FnCmnCtrl;				/* <S0FD> */

	Cprm = Axis->UniPrms->Cprm;					/* <S103> */

#if 0	/* <S0FD> Delete Start : Comment out due to specification change */
/* <S0E7> Start */
	if( (( AmonOut[0].RegVoltMon_Flag_Ch0 == TRUE  )) || (( AmonOut[0].RegVoltMon_Flag_Ch1 == TRUE  )) )
	{
		return;
	}
/* <S0E7> End */
#endif	/* <S0FD> Delete Start : Comment out due to specification change */

/* <S0FD> Start */
	if( FnCmnCtrl->AnlgMonVoltTstFlag == TRUE )
	{
		/*	Analog monitor output voltage confirmation test flag(AnlgMonVoltTstFlag) = TRUE, write	*/
		/*	Execute "analog monitor output test mode" without performing normal analog monitor output processing.*/

		if( FnCmnCtrl->AnlgMonVoltTstChSel == ANLG_MON_CH0_SEL )
		{/* Analog monitor output voltage confirmation test target = Ch0 */
		
			/* Analog monitorCh0. */
			ax_No = ((Cprm->b_prm6 & 0x1000) >> 12);		/* Analog monitor2Supports axis selection<S103> */
			FnOutputAnalogMonitor( &Axis[ax_No], FnCmnCtrl->OutputVoltVal_Ch0, 0 );
		
		}
		else if( FnCmnCtrl->AnlgMonVoltTstChSel == ANLG_MON_CH1_SEL )
		{/* Analog monitor output voltage confirmation test target = Ch1 */

			/* Analog monitorCh1. */
			ax_No = ((Cprm->b_prm7 & 0x1000) >> 12);		/* Analog monitor2Supports axis selection<S103> */
			FnOutputAnalogMonitor( &Axis[ax_No], FnCmnCtrl->OutputVoltVal_Ch1, 1 );

		}
		
		return;
	}
/* <S0FD> end */


/*--------------------------------------------------------------------------------------------------*/
/*		Analog monitor CH0 output																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( AmonOut[0].Enable == TRUE )
	{
		ax_No = ((Cprm->b_prm6 & 0x1000) >> 12);				/* Analog monitor2Supports axis selection<S103>	*//* <S1CB> */
		if( ax_No >= MAX_AXIS_NUMBER ){							/* Disabling incorrect settings for single axis				*/
			ax_No = MAX_AXIS_NUMBER - 1; }

		switch( AmonOut[0].Vtype )
		{
		case XAMDEF_VxLONG:  /* LONG Variable	*/
			Lvalue = AmonOut[0].GetVarData( &Axis[ax_No] );
			AmonCh0 = MlibMulgain( Lvalue, AmonOut[0].Mgain );
			break;
		case XAMDEF_VxSHORT: /* SHORT Variable	*/
			Svalue = AmonOut[0].GetVarData( &Axis[ax_No] );
			AmonCh0 = MlibMulgain( Svalue, AmonOut[0].Mgain );
			break;
		case XAMDEF_VxBIT:	 /* Bit Signal		*/
			Bvalue = AmonOut[0].GetVarData( &Axis[ax_No] );
			if( Bvalue == TRUE )
			{
				AmonCh0 = AmonOut[0].Mgain;
			}
			else
			{
				AmonCh0 = 0;
			}
			break;
		default:			 /* Default			*/
			 AmonCh0 = 0;
			 break;
		}
	/*----------------------------------------------------------------------------------------------*/
		AmonCh0 += AmonOut[0].Offset;								/* OffsetAddition					*/
		Axis->SvAsicRegs->AsicHwReg->AREG_PWMU = MlibLimitul( AmonCh0, 0x7FFF, -0x7FFF );	/* Analog monitor output			*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Analog monitor CH1 output																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( AmonOut[1].Enable == TRUE )
	{
		ax_No = ((Cprm->b_prm7 & 0x1000) >> 12);				/* Analog monitor2Supports axis selection<S103>	*//* <S1C9> */
		if( ax_No >= MAX_AXIS_NUMBER ){							/* Disabling incorrect settings for single axis				*/
			ax_No = MAX_AXIS_NUMBER - 1; }

		switch( ((AMONOUT*)Axis->AmonOut)[1].Vtype )
		{
		case XAMDEF_VxLONG:  /* LONG Variable	*/
			Lvalue = AmonOut[1].GetVarData( &Axis[ax_No] );
			AmonCh1 = MlibMulgain( Lvalue, AmonOut[1].Mgain );
			break;
		case XAMDEF_VxSHORT: /* SHORT Variable	*/
			Svalue = (SHORT)AmonOut[1].GetVarData( &Axis[ax_No] );
			AmonCh1 = MlibMulgain( Svalue, AmonOut[1].Mgain );
			break;
		case XAMDEF_VxBIT:	 /* Bit Signal		*/
			Bvalue = AmonOut[1].GetVarData( &Axis[ax_No] );
			if( Bvalue == TRUE )
			{
				AmonCh1 = AmonOut[1].Mgain;
			}
			else
			{
				AmonCh1 = 0;
			}
			break;
		default:			 /* LONG Variable	*/
			AmonCh1 = 0;
			break;
		}
	/*----------------------------------------------------------------------------------------------*/
		AmonCh1 +=  AmonOut[1].Offset;								/* OffsetAddition					*/
		Axis->SvAsicRegs->AsicHwReg->AREG_PWMV = MlibLimitul( AmonCh1, 0x7FFF, -0x7FFF );	/* Analog monitor output			*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}

/* <S0E7> Start */
/****************************************************************************************************/
/*																									*/
/*		Analog monitor output processing(Register access)													*/
/*																									*/
/*		Function: Output the voltage of the specified value from the analog monitor										*/
/*																									*/
/****************************************************************************************************/
//void	FnOutputAnalogMonitor( AXIS_HANDLE *Axis, LONG Value, LONG Ch_No )
void	FnOutputAnalogMonitor( AXIS_HANDLE *Axis, SHORT Value, LONG Ch_No )
{
	LONG	Gsftx;
	LONG	Gadjx;
	LONG	Xgain;
	LONG	Mgain;
	LONG	InputVolt;
	LONG	AmonVal;
	AMONOUT	*AmonOutx;								/* Pointer to analog monitor data				*/
	PRMDATA	*Prm;									/* Pointer to parameter setting data				*/
	CMN_PRMDATA	*Cprm;								/* Pointer to unit common parameter setting data	*/

	/* Initializing variables	*/
	Prm = Axis->UniPrms->Prm;						/* Parameter setting data							*/
	Cprm = Axis->UniPrms->Cprm;						/* Unit common parameter setting data				*/
	Mgain = 1;										/* Analog monitor output gain value Initial value			*/
	InputVolt = (LONG)(SHORT)Value;					/* Input voltage										*/
/*--------------------------------------------------------------------------------------------------*/
/*		CH selection processing																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( Ch_No == 0 )								/* Analog monitor CH0							*/
	{
		Xgain = (LONG)((SHORT)Cprm->mongain1);		/* CH0 Gain Parameter(Pn552:Analog monitor1magnification)*/
		AmonOutx = (AMONOUT*)&(Axis->AmonOut[0]);	/* OutData Pointer								*/
		Gadjx = (CHAR)Cprm->mnag0_1.b.l;			/* Gain Adjust Data(PnE59:Monitor1,2Zero adjustment)		*/
		Gsftx = -((USHORT)Cprm->mnrng0 >> 12);		/* MlibPcalKxgain()(sx)The sign is reversed		*/
	}
	else											/* Analog monitor CH1							*/
	{
		Xgain = (LONG)((SHORT)Cprm->mongain2);		/* CH1 Gain Parameter(Pn552:Analog monitor1magnification)*/	
		AmonOutx = (AMONOUT*)&(Axis->AmonOut[1]);	/* OutData Pointer								*/	
		Gadjx = (CHAR)Cprm->mnag0_1.b.h;			/* Gain Adjust Data(PnE59:Monitor1,2Zero adjustment)		*/	
		Gsftx = -((USHORT)Cprm->mnrng1 >> 12);		/* MlibPcalKxgain()(sx)The sign is reversed		*/	
	}

	/* Analog monitor output gain calculation */
	Mgain = MlibPcalKskxkx( Mgain, Xgain * (Gadjx + 256), (100*256), &Gsftx,   0 );
	Mgain = MlibPcalKxgain( Mgain, AMON_HWGAIN_1000V,     100000,     &Gsftx,  24 );


	/* OffsetAcquisition */
	PcalAnalogMonitorOffset( Axis, Ch_No );

	/* Analog monitor output calculation */
	AmonVal = MlibMulgain( InputVolt, Mgain );
	AmonVal += AmonOutx->Offset;

	if( Ch_No == 0 )
	{/* Analog monitor CH0 output */
		Axis->SvAsicRegs->AsicHwReg->AREG_PWMU = MlibLimitul( AmonVal, 0x7FFF, -0x7FFF );	/* Analog monitor output Ch0 */
		//AmonOutx->RegVoltMonSetVal_Ch0 = Axis->SvAsicRegs->AsicHwReg->AREG_PWMU;	/* <S0FD> */
	}
	else
	{	/* Analog monitor CH1 output */
		Axis->SvAsicRegs->AsicHwReg->AREG_PWMV = MlibLimitul( AmonVal, 0x7FFF, -0x7FFF );	/* Analog monitor output Ch1 */
		//AmonOutx->RegVoltMonSetVal_Ch1 = Axis->SvAsicRegs->AsicHwReg->AREG_PWMV;	/* <S0FD> */
	}

	return;
}
/* <S0E7> End */
/***************************************** end of file **********************************************/

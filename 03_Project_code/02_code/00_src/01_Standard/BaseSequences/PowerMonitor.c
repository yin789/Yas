/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PowerMonitor.c : Power consumption calculation processing definition		<S020>											*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Performs power consumption monitor calculation processing.														*/
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
/*	Note	:	First edition	2013.10.7	Y.Oka	For Mercury												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "PowerManager.h"



/****************************************************************************************************/
/*		Loss calculation coefficient																				*/
/****************************************************************************************************/
#define		LOS_SLOPE1		166.0F						/* 0.162 * 1024								*/
#define		LOS_SLOPE2		4915.0F						/* 4.800 * 1024								*/
/****************************************************************************************************/
/*		ScanBConversion coefficient to average the power consumption of the cycle		<S039>											*/
/****************************************************************************************************/
#define		CONVERT_AVR		KPI_SBCYCLEUS / KPI_SCCYCLEUS
														/* ScanCPeriod / ScanBPeriod					*/
/****************************************************************************************************/
/*		ScanCPower consumption per cycle[Wh]Conversion coefficient(Including averaging)											*/
/****************************************************************************************************/
#define		CONVERT_WH		KPI_SCCYCLEMS / 60.0F / 60.0F / 1000.0F
														/* [h]→[2ms]								*/
/****************************************************************************************************/
/*		Power consumption by control power supply[W]																	*/
/****************************************************************************************************/
//#define		ONLY_CTRLLOSS	504 / MAX_AXIS_NUMBER	/* Divide in advance because it is added several times for the number of axes	*//* <S039> */
//														/* 1In minutes0.28[Wh]					*/
#define		ONLY_CTRLLOSS	17 / MAX_AXIS_NUMBER		/* Control power consumption according to catalog			*//* <S039> */
														/* Divide in advance because it is added several times for the number of axes	*/


/****************************************************************************************************/
/*																									*/
/*		Power consumption calculation constant calculation processing																	*/
/*																									*/
/****************************************************************************************************/
void	PrmCalcPowerMonitor( CALC_WATTAGE *CalcWattage, USHORT AmpImax, USHORT MotImax )
{
	CalcWattage->conf.CurrentNorm1 = 1.0f / 800000.0F;		/* (1 / 3125) >> 8						*/
	CalcWattage->conf.CurrentNorm2 = 1.0f / 5625.0F;		/* (1 / 1875) >> 3						*/
	CalcWattage->conf.CurrentNorm3 = 1.0f / 1024.0F;		/* 1 >> 10								*/
	CalcWattage->conf.Imax = ( MotImax <= AmpImax ) ? MotImax : AmpImax;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Power consumption calculation constant calculation processing																	*/
/*																									*/
/****************************************************************************************************/
void	InitializePowerMonitor( POWER_MONITOR *PowMon )
{
	MlibResetLongMemory( &PowMon->var, sizeof(PowMon->var)/4 );
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Power consumption calculation processing																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Power consumption monitor output calculation(Information loss avoidance)								< POWER_MONITOR >		*/
/*																									*/
/*				   Vq * Vdc		Iqd * Imax	   Vd * Vdc	    Idd * Imax								*/
/*		PowTmp5 = ---------- * ------------ + ---------- * ------------								*/
/*					 2^5		   2^5			 2^5		   2^5									*/
/*						 PowTmp5		↑V,IChange the shift amount to avoid information loss when calculating				*/
/*		PowTmp1 =	  ------------																	*/
/*					   3125 * 2^8																	*/
/*							   Iqd * Imax					 Iqd * Imax								*/
/*		PowTmp4 = Losslp1 * ( ------------ )^2 + Losslp2 *  ------------ 	: Amount of amplifier internal loss		*/
/*								 15000						   15000								*/
/*		PowMonPower = PowTmp1 + PowTmp4		: Output side power consumption + Internal loss in the amplifier						*/
/*																									*/
/****************************************************************************************************/
//void	CalcPowerMoniter( CALC_WATTAGE *CalcWattage, USHORT PowMonVq, USHORT PowMonIqd,				  /* <S039> */
//									USHORT PowMonVd, USHORT PowMonIdd, LONG DcVolt, BOOL BaseEnable )
void	CalcPowerMoniter( CALC_WATTAGE *CalcWattage, SHORT PowMonVq, SHORT PowMonIqd,
									SHORT PowMonVd, SHORT PowMonIdd, LONG DcVolt, BOOL BaseEnable )
{
	LONG	PowTmpL1, PowTmpL2, PowTmpL3;
	float	PowTmpF1/*, PowTmpF2, PowTmpF3*/;

	if( BaseEnable == FALSE )
	{
		PowMonVq = 0;
		PowMonIqd = 0;
		PowMonVd = 0;
		PowMonIdd = 0;
	}

	PowTmpL1 = ( PowMonVq * DcVolt ) >> 5;
	PowTmpL2 = ( PowMonIqd * CalcWattage->conf.Imax ) >> 5;
	PowTmpL3 = PowTmpL1 * PowTmpL2;

	PowTmpL1 = ( PowMonVd * DcVolt ) >> 5;
	PowTmpL2 = ( PowMonIdd * CalcWattage->conf.Imax ) >> 5;
	PowTmpL3 += ( PowTmpL1 * PowTmpL2 );

	/* Current value normalization:[0.1A]/15000, 3Phase conversion:3/2 */
	PowTmpF1 = (float)PowTmpL3 * CalcWattage->conf.CurrentNorm1;
	if( PowTmpF1 < 0 ) /* <S039> */
	{
		PowTmpF1 = 0;
	}

	/* Iqd * Imax / 15000 */
	/* @@PowMonIqdbe an absolute value? ? */
//	PowTmpF2 = (float)PowMonIqd * (float)CalcWattage->conf.Imax * CalcWattage->conf.CurrentNorm2;
//	PowTmpF3 = (( LOS_SLOPE1 * PowTmpF2 * PowTmpF2 ) + ( LOS_SLOPE2 * PowTmpF2 )) * CalcWattage->conf.CurrentNorm3;

	/* Output power consumption + Loss in the amplifier *//* @@Average value is better */
#if 0 /* Loss in the amplifier should be applied after checking the actual machine *//* <S039> */
	CalcWattage->var.PowerConsumption += PowTmpF1 + PowTmpF3;
#else /* <S039> */
	CalcWattage->var.PowerConsumption += PowTmpF1;
#endif /* <S039> */

	/* Addition of control power consumption */
	CalcWattage->var.PowerConsumption += ONLY_CTRLLOSS;

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Power consumption monitor setting processing																		*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Un032 : Power consumption[W]																			*/
/*		Un033 : Power consumption[Wh]																		*/
/*		Un034 : Cumulative power consumption(Integer part)[Wh]															*/
/*		Un035 : Cumulative power consumption(Decimal part)[0.001Wh]														*/
/*--------------------------------------------------------------------------------------------------*/
void	SetPowerMoniter( POWER_MONITOR *PowMon, float PowerConsumption )
{
	/* ScanCCalculation of power consumption per cycle */
	if( PowerConsumption < 0.0f ) /* <S039> */
	{ /* If negative0 */
		PowerConsumption = 0.0f;
	}
	PowMon->var.CumulativeEnergy += (PowerConsumption * CONVERT_WH);

	if( KlibGetLongTimerMs(PowMon->var.TimerCnt1s) > 1000 )
	{
		/* Un032 : Power consumption[W] */
		PowMon->var.UnConsumedPower = (LONG)PowerConsumption;
		/* Un033 : Power consumption[Wh] */
		PowMon->var.UnEnergyConsumption = (LONG)PowMon->var.CumulativeEnergy;
		/* Power consumption calculation from control power-on */
		PowMon->var.PonCumulativeEnergy += PowMon->var.CumulativeEnergy;

		PowMon->var.CumulativeEnergy = 0.0f;
		KlibRstLongTimer( &PowMon->var.TimerCnt1s );
	}

	if( KlibGetLongTimerMs(PowMon->var.TimerCnt10s) > 10000 )
	{
		if( PowMon->var.PonCumulativeEnergy > LONG_MAX )
		{ /* Overflow judgment */
			PowMon->var.PonCumulativeEnergy -= LONG_MAX;
		}
		/* Un034 : Cumulative power consumption(Integer part)[Wh] */
		PowMon->var.UnCumulativeEnergyI = (LONG)PowMon->var.PonCumulativeEnergy;
		/* Un035 : Cumulative power consumption(Decimal part)[0.001Wh] */
		PowMon->var.UnCumulativeEnergyD =
				(PowMon->var.PonCumulativeEnergy - PowMon->var.UnCumulativeEnergyI) * 1000; /* <S0A5> */

		KlibRstLongTimer( &PowMon->var.TimerCnt10s );
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		For power consumption monitor averagingAPI	<S039>															*/
/*																									*/
/****************************************************************************************************/
void	CalcPowerConsumptionAvr( CALC_WATTAGE *CalcWattage )
{
	CalcWattage->var.PowerConsumption_a = CalcWattage->var.PowerConsumption * (float)CONVERT_AVR;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		For power consumption initialization for multi-axis calculationAPI																*/
/*																									*/
/****************************************************************************************************/
void	ResetPowerConsumption( CALC_WATTAGE *CalcWattage )
{
	CalcWattage->var.PowerConsumption = 0;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		For power consumption acquisitionAPI																			*/
/*																									*/
/****************************************************************************************************/
float	GetPowerConsumption( CALC_WATTAGE *CalcWattage )
{
//	return	(CalcWattage->var.PowerConsumption); /* <S039> */
	return	(CalcWattage->var.PowerConsumption_a);
}


/***************************************** end of file **********************************************/

/****************************************************************************
Description	: Tuning Function Library
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
Y.Oka  2010.11.16    created

*****************************************************************************/
#include "TuneLib.h"
#include "PnPrmListTbl.h"
#include "ResVibCtrl.h"
#include "MLib.h"



/****************************************************************************************************/
/*																									*/
/*		Function Definition																			*/
/*																									*/
/****************************************************************************************************/
/* PRM_RSLT	TunePrmWriteRam( REG_ACC_T *AccessPrm, CPRM_ATTR_T *PrmAttr, LONG Data, LONG Size );	*/
/* USHORT	TunePrmLimitUnsigned( ULONG Input, CPRM_ATTR_T *PrmAttr );								*/
/* SHORT	TunePrmLimitSigned( LONG Input, CPRM_ATTR_T *PrmAttr );									*/
/* USHORT	TuneKvFuncTi( USHORT Kv, BOOL IPSpdControl );											*/
/* USHORT	TuneKvFuncKp( USHORT Kv, USHORT Ti, BOOL IPSpdControl );								*/
/* USHORT	TuneKvFuncDobsgn( USHORT Kv );															*/
/* SHORT	TuneKvFuncDobLpFil( USHORT Kv, USHORT Dobsgn );											*/
/* SHORT	TuneNotchQ( AUTONOTCH *ANotch, REGIF_CONFIG_T *RegCfg, USHORT kv,						*/
/*													USHORT jrate, UCHAR sel, PRMDATA *Prm );		*/
/* void		TuneGetNotchQSetting( AUTONOTCH *ANotch, USHORT *NotchQ1,								*/
/*															USHORT *NotchQ2, USHORT *TrqFil );		*/
/* USHORT	TuneNotchModKv( USHORT kv, USHORT jrate, USHORT j1rate );								*/
/* SHORT	TuneNotchQset( AUTONOTCH_PRM *ANotchPrm, USHORT kv, USHORT jrate, USHORT Nfreq,			*/
/*																USHORT Antfreq, USHORT *autoQ );	*/
/* USHORT	TuneNotchTfcalc( AUTONOTCH_PRM *ANotchPrm, USHORT kv,									*/
/*													USHORT jrate, USHORT Nfreq, USHORT autoQ_Q1 );	*/
/* USHORT	TuneOnePrmTfcalc( AUTONOTCH_PRM *ANotchPrm, USHORT kv, USHORT jrate );					*/
/* SHORT	TuneNotchQset2( USHORT freq, USHORT Nfreq, USHORT *autoQ );								*/
/* void		TuneFilGainCalbyFilType( AUTONOTCH *ANotch, UCHAR Type );								*/
/* void		TuneMfcActive( PRMDATA *Prm );															*/
/* void		TuneMfcActiveStMode( PRMDATA *Prm );													*/
/* void		TuneDobsActive( PRMDATA *Prm );															*/
/* SHORT	TuneATypeResSet( USHORT freq, USHORT auton_sw, PRMDATA *Prm );							*/
/* SHORT	TuneNotchFilterSetting( AUTONOTCH *ANotch, PRMDATA *Prm, REGIF_CONFIG_T *RegCfg );		*/
/* SHORT	TuneVibrationTrace( DETVIB *DetVib, FFTANALYZE *FftAna,							*/
/*											UCHAR TraceRequest, SHORT TrgLvl, BOOL TuneLessSts );	*/
/* USHORT	TuneMfcKpmCalcAntFreq( USHORT AntFreq );												*/
/****************************************************************************************************/



/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		AAT, One-paraTfCalculation, notchQPair used for value calculationKvRatio,J1because they are executed within the charge pump process.						*/
/*		The table is selected according to the rigidity level setting.												*/
/*																									*/
/*			Array elements																				*/
/*				①AutomaticTfCalculation cut-offKv					[x]							*/
/*				②NotchONTfCalculation cut-offKv		[x]							*/
/*				③NotchQCutoff for value calculationKv				[x]							*/
/*				④AutomaticTfCalculation of motor side moment of inertia ratio 				[%]								*/
/*				⑤NotchONTfCalculation of motor side moment of inertia ratio	[%]								*/
/*				⑥NotchONtimeQMotor side moment of inertia ratio for value calculation		[%]								*/
/*				⑦AutomaticTfCalculation minimum setting value								[0.01ms]						*/
/*				⑧NotchQValue setting minimum value								[0.01]							*/
/*																									*/
/****************************************************************************************************/
const AUTONOTCH_PRM ANotchTbl[ 4 ] =
{
	{	4,	6,	6,	65535,	65535,	65535,	10,		50},	/* Rigidity 0 Gain */
	{	4,	6,	6,	200,	200,	400,	10,		50},	/* Rigidity 1 Gain */
	{	4,	4,	6,	400,	400,	200,	10,		150},	/* Rigidity 2 Gain */
	{	4,	4,	6,	65535,	65535,	65535,	10,		70},	/* Rigidity 3 Gain */
};



/****************************************************************************************************/
/*																									*/
/*		Parameter informationRAMWrite(Parameter calculation present)												*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	TunePrmWriteRam( REG_ACC_T *AccessPrm, CPRM_ATTR_T *PrmAttr, LONG Data, LONG Size )
{
	PRM_RSLT	rslt;

	AccessPrm->cPrmDef = PrmAttr;
	rslt = PrmWriteValue( AccessPrm, &Data, Size );

	return	rslt;
}



/****************************************************************************************************/
/*																									*/
/*		SHORTSize parameter setting value upper and lower limit processing(For unsigned parameters)							*/
/*																									*/
/****************************************************************************************************/
USHORT	TunePrmLimitUnsigned( ULONG Input, CPRM_ATTR_T *PrmAttr )
{
	USHORT	Output;

	if( Input > (USHORT)(PrmAttr->UpperLimit) )
	{
		Output = (USHORT)(PrmAttr->UpperLimit);
	}
	else if( Input < (USHORT)(PrmAttr->LowerLimit) )
	{
		Output = (USHORT)(PrmAttr->LowerLimit);
	}
	else
	{
		Output = (USHORT)Input;
	}

	return	Output;
}



/****************************************************************************************************/
/*																									*/
/*		SHORTSize parameter setting value upper and lower limit processing(For signed parameters)							*/
/*																									*/
/****************************************************************************************************/
SHORT	TunePrmLimitSigned( LONG Input, CPRM_ATTR_T *PrmAttr )
{
	SHORT	Output;

	if( Input > (SHORT)(PrmAttr->UpperLimit) )
	{
		Output = (SHORT)(PrmAttr->UpperLimit);
	}
	else if( Input < (SHORT)(PrmAttr->LowerLimit) )
	{
		Output = (SHORT)(PrmAttr->LowerLimit);
	}
	else
	{
		Output = (SHORT)Input;
	}

	return	Output;
}



/****************************************************************************************************/
/*																									*/
/*		KvInterlockingTiSetting																				*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	AAT, and One Para.														*/
/****************************************************************************************************/
USHORT	TuneKvFuncTi( USHORT Kv, BOOL IPSpdControl )
{
	ULONG	work;
	USHORT	Ti;

	if( IPSpdControl )
	{ /* I-PDuring control */
		/* Speed loop integral time constant setting */
		work = (ULONG)(TUNE_IP_TIKV / Kv);
	}
	else
	{ /* PIDuring control */
		/* Speed loop integral time constant setting */
		work = (ULONG)(TUNE_PI_TIKV / Kv);
	}

	Ti = TunePrmLimitUnsigned( work, &pndef_pitime );

	return	Ti;
}


/****************************************************************************************************/
/*																									*/
/*		KvInterlockingKpSetting																				*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	AATBalance with model tracking, recommended balance with model tracking				*/
/*				AAT, and One Para.														*/
/****************************************************************************************************/
USHORT	TuneKvFuncKp( USHORT Kv, USHORT Ti, BOOL IPSpdControl )
{
	ULONG	work;
	USHORT	Kp;

	if( IPSpdControl )
	{ /* I-PDuring control	ζ=1.0 */
		work = (ULONG)(TUNE_IP_KPTI / Ti);
	}
	else
	{ /* PIDuring control	ζ=1.25 */
		/* Kp[1/s] = Kv[rad/s] / 4 ≒ 1.5 *Kv[Hz] */
		work = (ULONG)(Kv * 3) >> 1;
	}

	Kp = TunePrmLimitUnsigned( work, &pndef_posgn );

	return	Kp;
}


/****************************************************************************************************/
/*																									*/
/*		KvInterlocking disturbance observer setting																	*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	AAT, and One Para.														*/
/****************************************************************************************************/
USHORT	TuneKvFuncDobsgn( USHORT Kv )
{
	ULONG	work;
	USHORT	dobgn;

	/* Kds = Kv * 100% */
	work = TUNE_DOBGNRATIO;

	dobgn = TunePrmLimitUnsigned( work, &pndef_dobgn );

	return	dobgn;
}



/****************************************************************************************************/
/*																									*/
/*		KvInterlocking disturbance observer setting																	*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	AAT, and One Para.														*/
/****************************************************************************************************/
SHORT	TuneKvFuncDobLpFil( USHORT Kv, USHORT Dobsgn )
{
	ULONG	work;
	SHORT	dlpfil;

	/* Kv * 4 */
	work = 4 * Kv;

	/* 400Hz , */
	if( work > TUNE_MAX_DLPFIL )
	{
		work = TUNE_MAX_DLPFIL;
	}

	dlpfil = (SHORT)( Dobsgn - (USHORT)work );
	dlpfil = TunePrmLimitSigned( (LONG)dlpfil, &pndef_dlpfil );

	return	dlpfil;
}




/****************************************************************************************************/
/*																									*/
/*		Notch filterQValue automatic calculation selection																*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	1stage,2Determine the enable/disable of the 2nd stage notch filter and the magnitude relationship of the notch frequency,			*/
/*				Kvand according to inertiaQRecalculate the value.											*/
/*				When notch filter is disabled, returns the current setting value.										*/
/****************************************************************************************************/
SHORT	TuneNotchQ( AUTONOTCH *ANotch, REGIF_CONFIG_T *RegCfg, USHORT kv, USHORT jrate, UCHAR sel, PRMDATA *Prm )
{
	SHORT			rc;	
	USHORT			wk_notchQ;
	REG_ACC_T		AccessPrm;
	AUTONOTCH_PRM	*ANotchPrm;
	AUTONOTCH_DATA	*ANotchData;

	rc = OK;
	ANotchPrm = &(ANotch->ANotchPrm);
	ANotchData = &(ANotch->ANotchData);

	/* RAMWrite register information setting */
	AccessPrm.MemAddr = 0;
	AccessPrm.hRegCfg = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/*----------------------------------------------------------------------------------*/
	/*	Initializing variables (current setting values)													*/
	/*----------------------------------------------------------------------------------*/
	/* 1Row notchQvalue */
	ANotchData->var.NotchQ1 = Prm->ntchq1;
	/* 2Row notchQvalue */
	ANotchData->var.NotchQ2 = Prm->ntchq2;
	/* 1Stage torque command filter */
	ANotchData->var.TrqFil = Prm->trqfil11;

	if( (Prm->tfuncsw & 0x000F) == 0x0001 )
	{ /* 1Stage notch enabled */
		if( (Prm->tfuncsw & 0x0F00) == 0x0100 )
		{ /* 1stage,2Stage notch enabled */
			/*--------------------------------------------------------------------------*/
			/*	The lower notch frequency isKv.JLQCalculate the value.					*/
			/*	The higher notch frequency is linked to the lower notch frequencyQCalculate the value.	*/
			/*--------------------------------------------------------------------------*/
			if( Prm->ntchfil1 > Prm->ntchfil2 )
			{
				rc = TuneNotchQset( ANotchPrm, kv, jrate, Prm->ntchfil2, TUNE_ANTFRQ_NTQ2, &wk_notchQ );
				/*----------------------------------------------------------------------*/
				/*	2Row notchQValue calculation													*/
				/*----------------------------------------------------------------------*/
				if( (Prm->anotchsw & 0x0F00) == 0x0100 )
				{
					ANotchData->var.NotchQ2 = wk_notchQ;
					/*------------------------------------------------------------------*/
					/*	TfCalculation															*/
					/*------------------------------------------------------------------*/
					ANotchData->var.TrqFil = 
						TuneNotchTfcalc( ANotchPrm, kv, jrate, Prm->ntchfil2, ANotchData->var.NotchQ2 );
				}
				else
				{
					/*------------------------------------------------------------------*/
					/*	TfCalculation															*/
					/*------------------------------------------------------------------*/
					ANotchData->var.TrqFil = TuneOnePrmTfcalc( ANotchPrm, kv, jrate );
				}
				/*----------------------------------------------------------------------*/
				/*	1Row notchQValue calculation													*/
				/*----------------------------------------------------------------------*/
				if( (Prm->anotchsw & 0x000F) == 0x0001 )
				{
					rc |= TuneNotchQset2( Prm->ntchfil2, Prm->ntchfil1, &ANotchData->var.NotchQ1 );
				}
			}
			else
			{
				rc = TuneNotchQset( ANotchPrm, kv, jrate, Prm->ntchfil1, TUNE_ANTFRQ_NTQ1, &wk_notchQ );
				/*----------------------------------------------------------------------*/
				/*	1Row notchQValue calculation													*/
				/*----------------------------------------------------------------------*/
				if( (Prm->anotchsw & 0x000F) == 0x0001 )
				{
					ANotchData->var.NotchQ1 = wk_notchQ;
					/*------------------------------------------------------------------*/
					/*	TfCalculation															*/
					/*------------------------------------------------------------------*/
					ANotchData->var.TrqFil = 
						TuneNotchTfcalc( ANotchPrm, kv, jrate, Prm->ntchfil1, ANotchData->var.NotchQ1 );
				}
				else
				{
					/*------------------------------------------------------------------*/
					/*	TfCalculation															*/
					/*------------------------------------------------------------------*/
					ANotchData->var.TrqFil = TuneOnePrmTfcalc( ANotchPrm, kv, jrate );
				}
				/*----------------------------------------------------------------------*/
				/*	2Row notchQValue calculation													*/
				/*----------------------------------------------------------------------*/
				if( (Prm->anotchsw & 0x0F00) == 0x0100 )
				{
					rc |= TuneNotchQset2( Prm->ntchfil1,Prm->ntchfil2,&ANotchData->var.NotchQ2 );
				}
			}
		}
		else
		{ /* 1Only the row notch is valid */
			rc = TuneNotchQset( ANotchPrm, kv, jrate, Prm->ntchfil1, TUNE_ANTFRQ_NTQ1, &wk_notchQ );
			/*--------------------------------------------------------------------------*/
			/*	1Row notchQValue calculation														*/
			/*--------------------------------------------------------------------------*/
			if( (Prm->anotchsw & 0x000F) == 0x0001 )
			{
				ANotchData->var.NotchQ1 = wk_notchQ;
			/*----------------------------------------------------------------------*/
			/*	TfCalculation																*/
			/*----------------------------------------------------------------------*/
				ANotchData->var.TrqFil = 
					TuneNotchTfcalc( ANotchPrm, kv, jrate, Prm->ntchfil1, ANotchData->var.NotchQ1 );
			}
			else
			{
				/*----------------------------------------------------------------------*/
				/*	TfCalculation																*/
				/*----------------------------------------------------------------------*/
				ANotchData->var.TrqFil = TuneOnePrmTfcalc( ANotchPrm, kv, jrate );
			}
		}
	}
	else
	{ /* 1Stage notch disabled */
		if( (Prm->tfuncsw & 0x0F00) == 0x0100 )
		{ /* 2Only the row notch is valid */
			rc = TuneNotchQset( ANotchPrm, kv, jrate, Prm->ntchfil2, TUNE_ANTFRQ_NTQ2, &wk_notchQ );
			/*--------------------------------------------------------------------------*/
			/*	2Row notchQValue calculation														*/
			/*--------------------------------------------------------------------------*/
			if( (Prm->anotchsw & 0x0F00) == 0x0100 )
			{
				ANotchData->var.NotchQ2 = wk_notchQ;
				/*----------------------------------------------------------------------*/
				/*	TfCalculation																*/
				/*----------------------------------------------------------------------*/
				ANotchData->var.TrqFil = 
					TuneNotchTfcalc( ANotchPrm, kv, jrate, Prm->ntchfil2, ANotchData->var.NotchQ2 );
			}
			else
			{
				/*----------------------------------------------------------------------*/
				/*	TfCalculation																*/
				/*----------------------------------------------------------------------*/
				ANotchData->var.TrqFil = TuneOnePrmTfcalc( ANotchPrm, kv, jrate );
			}
		}
		else
		{ /* 1stage,2Both stages disabled */
			/* Notch filter disabled */
			rc = -3;
			ANotchData->var.TrqFil = TuneOnePrmTfcalc( ANotchPrm, kv, jrate );
		}
	}
	/*----------------------------------------------------------------------------------*/
	/*	Calculated valueRAM																*/
	/*----------------------------------------------------------------------------------*/
	if( (sel == 1) || ((sel == 2) && (rc == OK)) ) /* sel==2 , calculationOKchange only when	*/
	{ /* Calculated contentRAM(Caution!! EEPROM) */
		/* Pn401 */
		TunePrmWriteRam( &AccessPrm, &pndef_trqfil11, (LONG)ANotchData->var.TrqFil, 2 );
		/* Pn40A */
		TunePrmWriteRam( &AccessPrm, &pndef_ntchq1, (LONG)ANotchData->var.NotchQ1, 2 );
		/* Pn40D */
		TunePrmWriteRam( &AccessPrm, &pndef_ntchq2, (LONG)ANotchData->var.NotchQ2, 2 );
	}
	return	rc;
}



/****************************************************************************************************/
/*																									*/
/*		Notch filterQGet value calculation resultAPI															*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	Notch filterQValue automatic calculation selection process(TuneNotchQ)Get the calculation result.					*/
/*				1Row notchQvalue,2Row notchQvalue,1Returns the stage torque command filter.						*/
/****************************************************************************************************/
void	TuneGetNotchQSetting( AUTONOTCH *ANotch, USHORT *NotchQ1, USHORT *NotchQ2, USHORT *TrqFil )
{
	/* 1Row notchQvalue */
	*NotchQ1 = ANotch->ANotchData.var.NotchQ1;
	/* 2Row notchQvalue */
	*NotchQ2 = ANotch->ANotchData.var.NotchQ2;
	/* 1Stage torque command filter */
	*TrqFil = ANotch->ANotchData.var.TrqFil;
}



/****************************************************************************************************/
/*																									*/
/*		Tf.NotchQFor value calculationKvCorrection calculation															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	Specified by argumentKv, Inertia ratio, from magnificationKvdefined in [62].			*/
/*																									*/
/*							Jrate[%] + 100															*/
/*				Kv' = SQRT(-----------------) * Kv													*/
/*								J1[%]																*/
/*													kv		: Before correctionKv						[Hz]	*/
/*													Jrate	: Load moment of inertia ratio for correction	[%]		*/
/*													J1		: Motor side moment of inertia ratio		[%]		*/
/*																									*/
/****************************************************************************************************/
USHORT	TuneNotchModKv( USHORT kv, USHORT jrate, USHORT j1rate )
{
	LONG	work;
	USHORT	TLV_kv;

	if( j1rate == 0 )
	{	/* J1but0[%] → Calculated from resonance and antiresonance frequencies */
		/*----------------------------------------------------------*/
		/*	Inertia ratio correction calculation									*/
		/*				  		Freq_res[Hz]						*/
		/*		Kv'[0.1Hz] = ----------------- * Kv[0.1Hz]			*/
		/*						Freq_ant[Hz]						*/
		/*----------------------------------------------------------*/
		work = (LONG)TUNE_RESFRQ_KV * (LONG)kv / (LONG)TUNE_ANTFRQ_KV;
	}
	else if( (jrate + 100) <= j1rate )
	{
		/* J1The following are considered rigid bodies. */
		work = (LONG)kv ;
	}
	else
	{
		/*----------------------------------------------------------*/
		/*	Inertia ratio correction calculation									*/
		/*				  			Jrate[%] + 100					*/
		/*		Kv'[0.1Hz] = SQRT(-----------------) * Kv[0.1Hz]	*/
		/*								J1[%]						*/
		/*----------------------------------------------------------*/
		work = 
			(LONG)kv * (LONG)MlibSqrtu32(((LONG)jrate + 100) * 10000 / (LONG)j1rate) /100;
	}

	/*--------------------------------------------------------------*/
	/*	CorrectionKv Upper/lower limit check										*/
	/*--------------------------------------------------------------*/
	if( work > (LONG)(pndef_loophz.UpperLimit) )
	{
		TLV_kv = (USHORT)pndef_loophz.UpperLimit;
	}
	else if( work < (LONG)kv )
	{
		TLV_kv = kv;
	}
	else
	{
		TLV_kv = (USHORT)work;
	}

	return	TLV_kv;
}



/****************************************************************************************************/
/*																									*/
/*		NotchQAutomatic calculation of the value																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	Specified by argumentKv, the notch for the notch frequencyQ.						*/
/*				Notch upper and lower limit check, notchONis possible.								*/
/*																									*/
/*					   α * Kv * fn								fn: Notch frequency			[Hz]	*/
/*				Q = ------------------							kv: After correctionKv				[Hz]	*/
/*					 fn^2 - α^2 * Kv^2							α: NotchQVariable pair for value calculationKvRatio	[x]	*/
/*																									*/
/****************************************************************************************************/
SHORT	TuneNotchQset( AUTONOTCH_PRM *ANotchPrm, USHORT kv, USHORT jrate, USHORT Nfreq, 
																	USHORT Antfreq, USHORT *autoQ )
{
	SHORT	check;
	USHORT	TLV_kv;
	LONG	work1, work2, work3;
	LONG	freq_cl;
	LONG	sx, sy;
	LONG	fcl_Tf;
	SHORT	swk;

	check = OK;

	/*--------------------------------------------------------------*/
	/*	Inertia ratio correction calculation										*/
	/*--------------------------------------------------------------*/
	TLV_kv = TuneNotchModKv( kv, jrate, ANotchPrm->J1Q );

	/*--------------------------------------------------------------*/
	/*	Notch disabledTf[0.01ms]Calculation									*/
	/*--------------------------------------------------------------*/
	swk = TuneOnePrmTfcalc( ANotchPrm, kv, jrate );

	/*--------------------------------------------------------------*/
	/*	Tf						*/
	/*	 fcl[Hz] = 1000 * 100 / (2πTf[0.01ms])						*/
	/*--------------------------------------------------------------*/
	/* MinimumTfminutes */
	work1 = (LONG)swk - ((LONG)ANotchPrm->TfMin - 5);
	if(work1 <= 0)
	{ /* Tfis less than zero */
		/* Since setting is not possible, set the frequency to the maximum value */
		fcl_Tf = 10000;
	}
	else
	{
		/* Minimum notch frequency setting value 159160 = 1000 *100 /6.283 */
		fcl_Tf = (LONG)(15916 / work1);
	}

	/*--------------------------------------------------------------*/
	/*	Notch cutoff frequency calculation									*/
	/*--------------------------------------------------------------*/
	if( ANotchPrm->J1Q == 0 )
	{ /* Calculated from resonance and antiresonance frequencies */
		/* (freq_res[Hz] + freq_ant[Hz]) / 2 */
		freq_cl = ((LONG)Nfreq + (LONG)Antfreq) / 2;
	}
	else
	{ /* CorrectionKvCalculated from value */
		/* α × Kv'[Hz] */
		freq_cl = (LONG)ANotchPrm->KvrateQ * (LONG)TLV_kv / TUNE_ANOTCH_DECA;
	}

	/*--------------------------------------------------------------*/
	/*	Notch cutoff frequency judgment									*/
	/*		TfKv	*/
	/*--------------------------------------------------------------*/
	if( freq_cl < fcl_Tf )
	{ /* From the minimum setting frequencyfreq_clis small.→Notch setting not possible */
		freq_cl = fcl_Tf;
	}

	/*--------------------------------------------------------------*/
	/*	NotchQValue calculation												*/
	/*					  α * Kv'[Hz] * fn[Hz]						*/
	/*		Q[0.01] = -------------------------------				*/
	/*					fn[Hz]^2 - α^2 * Kv'[Hz]^2					*/
	/*																*/
	/*	When calculating from the resonance anti-resonance frequency, the above formulaα * Kv'[Hz]of		*/
	/*	Notch cutoff frequency(f_cl)							*/
	/*--------------------------------------------------------------*/
	if( freq_cl >= (LONG)Nfreq )
	{ /* VersusKvNotch at ratioQValue calculation not possible */
		/* QSet value upper limit */
		work1 = TUNE_ANOTCH_Q_MAX;
		check = NG;
	}
	else
	{
		/* Molecular calculation */
		/* α×Kv[0.1Hz]'×fn (×100teethQValue unit[0.01]) */
		sx = 0;
		work1 = MlibPcalKxmulx( freq_cl, (LONG)Nfreq, 100, &sx );

		/* Denominator calculation */
		/* fn[Hz]^2 */
		work2 = (LONG)Nfreq * (LONG)Nfreq;
		work3 = freq_cl;

		sy = 0;
		/* α^2×Kv[Hz]'^2 */
		work3 = MlibPcalKxmulx( work3, work3, 1, &sy );
		/* fn[Hz]^2 - α^2×Kv[Hz]'^2 */
		work2 = MlibPcalKxsubx( work2, 0, work3, sy, &sy );

		if( work2 <= 0 )
		{
			/* QSet value upper limit */
			work1 = TUNE_ANOTCH_Q_MAX;
		}
		else
		{
			/* NotchQvalue [0.01] */
			work1 = MlibPcalKxdivx( work1, sx, work2, sy, 24 );
			work1 = MlibKsgain2Long(work1);
		}
		/*--------------------------------------------------------------*/
		/*	Upper/lower limit check												*/
		/*--------------------------------------------------------------*/
		if( work1 < ANotchPrm->NotchQMin )
		{ /* QIf the value is smaller than the lower limit, clamp to the lower limit */
			work1 = ANotchPrm->NotchQMin;
		}
		else if( work1 > TUNE_ANOTCH_Q_MAX_DET )
		{ /* QIf the value is greater than the detection upper limit, clamp to the upper limit */
			work1 = TUNE_ANOTCH_Q_MAX;
			check = -2;
		}
		else if( work1 > TUNE_ANOTCH_Q_MAX )
		{ /* QIf the value is greater than the upper limit, clamp to the upper limit */
			work1 = TUNE_ANOTCH_Q_MAX;
		}
	}
	*autoQ = (USHORT)work1;

	return	check;
}



/****************************************************************************************************/
/*																									*/
/*		NotchON1Order low pass filter automatic calculation											*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	Specified by argumentKv.	*/
/*				NotchON, the equivalent delay of the notch is taken into accountTf.							*/
/*				Correction is notchOFFtimeTf.										*/
/*				However, after notch equivalent delay correctionTfis clamped at the minimum value.							*/
/*																									*/
/*								1																	*/
/*				Tn[s] = ------------------															*/
/*						 2 * π * f_cl[Hz]				f_cl: Notch equivalent cutoff frequency	[Hz]	*/
/*																									*/
/****************************************************************************************************/
USHORT	TuneNotchTfcalc( AUTONOTCH_PRM *ANotchPrm, USHORT kv, USHORT jrate, USHORT Nfreq, USHORT autoQ_Q1 )
{
	USHORT	TLV_kv;
	LONG	work1, work2;
	LONG	sx;

	/*--------------------------------------------------------------*/
	/*	Inertia ratio correction calculation										*/
	/*--------------------------------------------------------------*/
	TLV_kv = TuneNotchModKv( kv, jrate, ANotchPrm->J1TfonNt );

	/*--------------------------------------------------------------*/
	/*	Notch filter equivalent cutoff frequency	[Hz]					*/
	/*																*/
	/*					 8*Q^2										*/
	/*		f_cl = ------------------- * fn[Hz]						*/
	/*				8*Q^2 + 4*Q + 1									*/
	/*																*/
	/*	Notch filter equivalent delay time constant	[sec]						*/
	/*																*/
	/*						1										*/
	/*		Tn[s] = ------------------								*/
	/*				 2 * π * f_cl[Hz]								*/
	/*--------------------------------------------------------------*/
	/*	autoQ_Q1 < 1000 */
	work1 = 8 * (LONG)autoQ_Q1 * (LONG)autoQ_Q1;
	/* 400,10000teethautoQ_Q1but[0.01]unit */
	work2 = work1 + 400 * (LONG)autoQ_Q1 + 10000;

	/* Cutoff frequency calculation	[Hz] */
	sx = 0;
	work1 = MlibPcalKxgain(work1, (LONG)Nfreq, work2 ,&sx, 0);

	/* 15915 = 100000[s->0.01ms] / 2π */
	work1 = MlibPcalKxdivx( 15915, 0, work1, sx, 24 );
	/* Notch equivalent delay time constant	[0.01ms] */
	work1 = MlibKsgain2Long(work1);

	/*--------------------------------------------------------------*/
	/*	NotchOFF								*/
	/*	 Tf[0.01ms] = 100000 * 10 / (2πKv'[0.1Hz]* AUTOTf_KVRAT_NT)*/
	/*--------------------------------------------------------------*/
	/* 159160 = 1000 * 100 / 6.283 * 10 */
	work2 = 159160 / (LONG)ANotchPrm->KvrateTfonNt / (LONG)TLV_kv;

	if( ANotchPrm->J1TfonNt == 0 )
	{ /* Calculated from resonance and antiresonance frequencies */
		/* Notch equivalent delay not added */
		work1 = work2;
	}
	else
	{
		/* Torque command filter time constant after notch correction [0.01ms] */
		work1 = work2 - work1;
	}

	if( work1 < (LONG)ANotchPrm->TfMin )
	{
		/* TfClamping at the minimum setting value */
		work1 = (LONG)ANotchPrm->TfMin;
	}

	return	(USHORT)work1;
}


/****************************************************************************************************/
/*																									*/
/*		Torque command1Order low pass filter automatic calculation														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	Specified by argumentKv.						*/
/*																									*/
/*;							1																		*/
/*				Tf = ----------------																*/
/*						2πKv'×α																	*/
/*																									*/
/****************************************************************************************************/
USHORT	TuneOnePrmTfcalc( AUTONOTCH_PRM *ANotchPrm, USHORT kv, USHORT jrate )
{
	USHORT	TLV_kv;
	LONG	work1;

	/*--------------------------------------------------------------*/
	/*	Inertia ratio correction calculation										*/
	/*--------------------------------------------------------------*/
	TLV_kv = TuneNotchModKv( kv, jrate, ANotchPrm->J1Tf );

	/*--------------------------------------------------------------*/
	/*	Filter time constant calculation											*/
	/*	 Tf[0.01ms] = 100000 * 10 / (2πKv'[0.1Hz]* AUTOTf_KVRAT)	*/
	/*--------------------------------------------------------------*/
	/* 159160 = 1000 * 100 / 6.283 * 10 */
	work1 = 159160 / (LONG)ANotchPrm->KvrateTf / (LONG)TLV_kv;

	if( work1 < (LONG)ANotchPrm->TfMin )
	{
		/* TfClamping at the minimum setting value */
		work1 = (LONG)ANotchPrm->TfMin;
	}
	
	return	(USHORT)work1;
}



/****************************************************************************************************/
/*																									*/
/*		Notch filterQValue automatic calculation2																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	Notch whose cutoff frequency is the frequency specified by argumentQCalculate the value.					*/
/*																									*/
/*;					  Nfreq × freq																	*/
/*				Q = -----------------																*/
/*					 Nfreq^2 - freq^2																*/
/*																									*/
/****************************************************************************************************/
SHORT	TuneNotchQset2( USHORT freq, USHORT Nfreq, USHORT *autoQ )
{
	SHORT	check;
	ULONG	autoQ_Q2;
	LONG	work1, work2;

	check = OK;

	if( freq >= Nfreq )
	{	/* Vibration frequency is higher than target cutoff frequency → Unable to calculate */
		autoQ_Q2 = TUNE_ANOTCH_Q_MAX;
		check = NG;
	}
	else
	{
		/* 100,QTo match the unit of the value to the parameter */
		work1 = Nfreq * Nfreq - freq * freq;
		work2 = Nfreq * freq * 100;

		autoQ_Q2 = work2 /work1;

		/* Upper/lower limit check */
		if( autoQ_Q2 < TUNE_ANOTCH_Q_MIN )
		{ /* Qvalue1.50In the following cases */
			/* 1.50 */
			autoQ_Q2 = TUNE_ANOTCH_Q_MIN;
		}
		else if( autoQ_Q2 > TUNE_ANOTCH_Q_MAX )
		{ /* Qvalue3.00or above */
			/* 3.00 */
			autoQ_Q2 = TUNE_ANOTCH_Q_MAX;
			check = -2;
		}
	}
	*autoQ = (USHORT)autoQ_Q2;

	return	check;
}


/****************************************************************************************************/
/*																									*/
/*		AAT.One-para function synthesis level selection process															*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	AATCall the internal table from the machine rigidity level set in OnePara and set it as a variable.	*/
/****************************************************************************************************/
void	TuneFilGainCalbyFilType( AUTONOTCH *ANotch, UCHAR Type )
{
	/* AutomaticTfCalculation cut-offKv[x] */
	ANotch->ANotchPrm.KvrateTf = ANotchTbl[ Type ].KvrateTf;

	/* NotchONTfCalculation cut-offKv[x] */
	ANotch->ANotchPrm.KvrateTfonNt = ANotchTbl[ Type ].KvrateTfonNt;

	/* NotchQCutoff for value calculationKv[x] */
	ANotch->ANotchPrm.KvrateQ = ANotchTbl[ Type ].KvrateQ;

	/* AutomaticTfCalculation side moment of inertia ratio[%] */
	ANotch->ANotchPrm.J1Tf = ANotchTbl[ Type ].J1Tf;

	/* NotchONTfCalculation of motor side moment of inertia ratio[%] */
	ANotch->ANotchPrm.J1TfonNt = ANotchTbl[ Type ].J1TfonNt;

	/* NotchONtimeQMotor side moment of inertia ratio for value calculation[%] */
	ANotch->ANotchPrm.J1Q = ANotchTbl[ Type ].J1Q;

	/* AutomaticTfCalculation minimum setting value[0.01ms] */
	ANotch->ANotchPrm.TfMin = ANotchTbl[ Type ].TfMin;

	/* NotchQValue minimum setting value[-] */
	ANotch->ANotchPrm.NotchQMin = ANotchTbl[ Type ].NotchQMin;
	
	return;
}



#if 0 /* SGDVCommented out due to not being used */
/****************************************************************************************************/
/*																									*/
/*		Model tracking/Start of disturbance observer																*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	Start the model following/disturbance observer.												*/
/****************************************************************************************************/
void	TuneMfcDobsActive( HMSGIF Hmsg )
{
	/* Forced when both are in use, except whenON& Parameter overwrite */
	if( (Gprm.f.Dobs == FALSE) || (Gprm.MFCModel == 0) )
	{
		TuneMfcActive( Hmsg );
		TuneDobsActive( Hmsg );
	}

	return;
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Start model tracking																			*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	Start model tracking. At this time,ζof1,TFFof100,VFFof100.				*/
/*				AAT, and One Para.														*/
/****************************************************************************************************/
//void	TuneMfcActive( PRMDATA *Prm )
void	TuneMfcActive( PRMDATA *Prm, REGIF_CONFIG_T *RegCfg )/*<S09E>*/
{
	USHORT		data;
	REG_ACC_T	AccessPrm;

	/*--------------------------------------------------------------*/
	/* RAMWrite register information setting								*/
	/*--------------------------------------------------------------*/
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	Prm->mdlzeta = TUNE_MDLZETA; /* Pn142 */
	Prm->mdlptff = TUNE_MFCKTFF; /* Pn143 */
	Prm->mdlntff = TUNE_MFCKTFF; /* Pn144 */
	Prm->mdlvff  = TUNE_MFCKVFF; /* Pn147 */

	/* Pn140 */
	data = Prm->mdlsw;
	data |= 0x0001;
	//Prm->mdlsw = data;
	TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, (LONG)(data), 2 );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Start model tracking																			*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	Start model tracking. At this time,ζof1,TFFof100,VFFof95.				*/
/*				AAT, and One Para.														*/
/****************************************************************************************************/
//void	TuneMfcActiveStMode( PRMDATA *Prm )
void	TuneMfcActiveStMode( PRMDATA *Prm, REGIF_CONFIG_T *RegCfg )/*<S09E>*/
{
	USHORT		data;
	REG_ACC_T	AccessPrm;

	/*--------------------------------------------------------------*/
	/* RAMWrite register information setting								*/
	/*--------------------------------------------------------------*/
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	Prm->mdlzeta = TUNE_MDLZETA;  /* Pn142 */
	Prm->mdlptff = TUNE_MFCKTFF;  /* Pn143 */
	Prm->mdlntff = TUNE_MFCKTFF;  /* Pn144 */
	Prm->mdlvff  = TUNE_MFCKVFF2; /* Pn147 */

	/* Pn140 */
	data = Prm->mdlsw;
	data |= 0x0001;
	//Prm->mdlsw = data;
	TunePrmWriteRam( &AccessPrm, &pndef_mdlsw, (LONG)(data), 2 );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Start of disturbance observer																		*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	Start the disturbance observer. At this time,FBIncrease gain85%, inertia correction increases by100%	*/
/*				AAT, and One Para.													*/
/****************************************************************************************************/
//void	TuneDobsActive( PRMDATA *Prm )
void	TuneDobsActive( PRMDATA *Prm, REGIF_CONFIG_T *RegCfg )/*<S09E>*/
{
	USHORT		data;
	REG_ACC_T	AccessPrm;

	/*--------------------------------------------------------------*/
	/* RAMWrite register information setting								*/
	/*--------------------------------------------------------------*/
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	//Prm->dtrqgn = TUNE_DTRQGN; /* Pn123 */
	//Prm->dobjgn = TUNE_DOBJGN; /* Pn125 */
	TunePrmWriteRam( &AccessPrm, &pndef_dtrqgn,  (LONG)(TUNE_DTRQGN), 2 );	/* Pn123 */
	TunePrmWriteRam( &AccessPrm, &pndef_dobjgn,  (LONG)(TUNE_DOBJGN), 2 );	/* Pn125 */

	/* Pn408 */
	data = Prm->tfuncsw;
	data |= 0x1000;
	//Prm->tfuncsw = data;
	TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)(data), 2 );			/* Pn408 */

	return;
}



/****************************************************************************************************/
/*																									*/
/*		AMold vibration damping control setting process																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	Regarding the input frequency,AEnable type vibration suppression. Damping gain is a fixed value.				*/
/*																									*/
/*		arg	:	USHORT	freq			:	Vibration frequency		[Hz]									*/
/*				USHORT	auton_sw		:	Automatic adjustmentSW		[-]										*/
/*																									*/
/*		out	:	SHORT	rc				:	0x01:Normal setting completed										*/
/*											0x10:Upper/lower limit error											*/
/*											0x11:Automatic setting disabled										*/
/*											0x12:Parameter writing error								*/
/*																									*/
/****************************************************************************************************/
//SHORT	TuneATypeResSet( USHORT freq, USHORT auton_sw, PRMDATA *Prm )
SHORT	TuneATypeResSet( USHORT freq, USHORT auton_sw, PRMDATA *Prm, REGIF_CONFIG_T *RegCfg )/*<S09E>*/
{
	SHORT		rc;
	USHORT		data;
	REG_ACC_T	AccessPrm;

	/*------------------------------------------------------------------*/
	/* RAMWrite register information setting									*/
	/*------------------------------------------------------------------*/
	AccessPrm.MemAddr  = 0;
	AccessPrm.hRegCfg  = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/*------------------------------------------------------------------*/
	/*	Automatic configurability check											*/
	/*------------------------------------------------------------------*/
	data = Prm->avibsw;
	if( ((data & 0x0001) == 0x0001) || (auton_sw == FALSE) )
	{	/* AlreadyAMold vibration damping enabled or Automatic adjustment disabled */
		rc = 0x11;
		return	rc;
	}

	/*------------------------------------------------------------------*/
	/*	Frequency upper/lower limit check											*/
	/*------------------------------------------------------------------*/
	if( (freq >= AVIB_FRQ_MIN) && (freq <= AVIB_FRQ_MAX) )
	{
		/*--------------------------------------------------------------*/
		/*	AMold vibration damping parameter setting										*/
		/*--------------------------------------------------------------*/
		//Prm->avibfrq = freq * 10;	/* Pn161 */
		//Prm->avibdamp = AVIBDAMPGN; /* Pn163 */
		TunePrmWriteRam( &AccessPrm, &pndef_avibfrq,  (LONG)(freq * 10), 2 );	/* Pn161 */
		TunePrmWriteRam( &AccessPrm, &pndef_avibdamp, (LONG)AVIBDAMPGN,  2 );	/* Pn163 */

		/*--------------------------------------------------------------*/
		/*	AMold vibration damping function switch setting										*/
		/*--------------------------------------------------------------*/
		data = Prm->avibsw; /* Pn160 */
		data |= 0x0001;
		//Prm->avibsw = data;
		TunePrmWriteRam( &AccessPrm, &pndef_avibsw, (LONG)data,  2 );
		rc = 0x01;
	}
	else
	{
		rc = 0x10;
	}

	return	rc;
}



/****************************************************************************************************/
/*																									*/
/*		Notch filter setting processing																		*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	No.1, No.2Set the notch filter. NotchQThe value is automatically calculated.					*/
/****************************************************************************************************/
SHORT	TuneNotchFilterSetting( AUTONOTCH *ANotch, PRMDATA *Prm, REGIF_CONFIG_T *RegCfg )
{
	CHAR			NotchType;
	SHORT			rc;
	USHORT			NotchQ;
	USHORT			freq;
	USHORT			KvGain;
	USHORT			Jrate;
	REG_ACC_T		AccessPrm;
	AUTONOTCH_DATA	ANotchWk;
	AUTONOTCH_PRM	*ANotchPrm;

	NotchType = -1;
	rc = 0x00;
	ANotchPrm = &(ANotch->ANotchPrm);
	freq = ANotch->ANotchData.conf.freq;
	KvGain = ANotch->ANotchData.conf.KvGain;
	Jrate = ANotch->ANotchData.conf.Jrate;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr = 0;
	AccessPrm.hRegCfg = RegCfg;
	AccessPrm.ArrayIdx = 0;
	AccessPrm.EepWrite = FALSE;
	AccessPrm.IfHandle = NULL;

	/* CurrentRAMReading information */
	ANotchWk.var.NotchSw = Prm->tfuncsw;	/* Pn408 */
	ANotchWk.var.NotchQ1 = Prm->ntchq1;		/* Pn40A */
	ANotchWk.var.NotchQ2 = Prm->ntchq2;		/* Pn40D */
	ANotchWk.var.NotchFil1 = Prm->ntchfil1;	/* Pn409 */
	ANotchWk.var.NotchFil2 = Prm->ntchfil2;	/* Pn40C */
	ANotchWk.var.TrqFil = Prm->trqfil11;	/* Pn401 */

	/* Notch type setting */
	if( ((ANotchWk.var.NotchSw & 0x0F00) == 0x0000) && ((Prm->anotchsw & 0x0F00) == 0x0100) )
	{	/* 2Row notch can be set automatically */
		if( (ANotchWk.var.NotchSw & 0x000F) == 0x0000 )
		{	/* 1Stage notch not used -> 2Row notch1put it as the second eye */
			NotchType = 0;
		}
		else
		{	/* 1Stage notch in use -> 2Row notch2put it as the second eye */
			if( freq > ANotchWk.var.NotchFil1 )
			{
				NotchType = 1;
			}
			else
			{
				NotchType = 4;
			}
		}
	}
	else
	{
		if( ((ANotchWk.var.NotchSw & 0x000F) == 0x0000) && ((Prm->anotchsw & 0x000F) == 0x0001) )
		{ /* 1Row notch can be set automatically */
			if( (ANotchWk.var.NotchSw & 0x0F00) == 0x0000 )
			{	/* 2Stage notch not used -> 1Row notch1put it as the second eye */
				NotchType = 3;
			}
			else
			{	/* 2Stage notch in use -> 1Row notch2put it as the second eye */
				if( freq > ANotchWk.var.NotchFil2 )
				{
					NotchType = 2;
				}
				else
				{
					NotchType = 5;
				}
			}
		}
	}

	/*----------------------------------------------------------------------------------*/
	/*	Notch filter setting AutomaticQValue calculation													*/
	/*----------------------------------------------------------------------------------*/
	switch ( NotchType )
	{
		case 0:
		case 4:
			/*------------------------------------------------------------------------------*/
			/*	No.2Notch (low frequency) setting													*/
			/*------------------------------------------------------------------------------*/
			if( freq > pndef_ntchfil2.UpperLimit )
			{ /* If the upper limit is exceeded, set it to the upper limit */
				freq = (USHORT)pndef_ntchfil2.UpperLimit;
			}
			/*------------------------------------------------------------------------------*/
			/*	QValue calculation & Parameter calculation													*/
			/*------------------------------------------------------------------------------*/
			if( TuneNotchQset( ANotchPrm, KvGain, Jrate, freq, TUNE_ANTFRQ_NTQ2, &NotchQ ) != OK )
			{ /* QIf the value setting is abnormal, return the set value to the original value and complete the notch */
				/* 2Row notchQValue error */
				rc = 0x08;
			}
			else
			{	/* QValue calculation completed successfully */
				ANotchWk.var.TrqFil = TuneNotchTfcalc( ANotchPrm, KvGain, Jrate, freq, NotchQ );
				/* 2digit1, */
				ANotchWk.var.NotchSw |= 0x0100;
				/* Notch frequency */
				ANotchWk.var.NotchFil2 = freq;
				/* NotchQvalue */
				ANotchWk.var.NotchQ2 = NotchQ;

				if( NotchType == 4 )
				{ /* 1Row notchQValue update */
					TuneNotchQset2( ANotchWk.var.NotchFil2, ANotchWk.var.NotchFil1, &NotchQ );
					/* NotchQvalue */
					ANotchWk.var.NotchQ1 = NotchQ;
				}

				ANotchWk.var.TrqFil = TunePrmLimitUnsigned( (ULONG)ANotchWk.var.TrqFil, &pndef_trqfil11 );

				/* Pn401 */
				TunePrmWriteRam( &AccessPrm, &pndef_trqfil11, (LONG)ANotchWk.var.TrqFil, 2 );
				/* Pn408 */
				TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)ANotchWk.var.NotchSw, 2 );
				/* Pn40A */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchq1, (LONG)ANotchWk.var.NotchQ1, 2 );
				/* Pn40C */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchfil2, (LONG)ANotchWk.var.NotchFil2, 2 );
				/* Pn40D */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchq2, (LONG)ANotchWk.var.NotchQ2, 2 );

				/* 2Stage notch completed successfully */
				rc = 0x02;
			}
			break;

		case 1:
			/*------------------------------------------------------------------------------*/
			/*	No.2Notch (high frequency) setting													*/
			/*------------------------------------------------------------------------------*/
			if( freq > pndef_ntchfil2.UpperLimit )
			{ /* Limit at upper limit */
				freq = (USHORT)pndef_ntchfil2.UpperLimit;
			}
			/*------------------------------------------------------------------------------*/
			/*	QValue calculation & Parameter calculation													*/
			/*------------------------------------------------------------------------------*/
			if( TuneNotchQset2( ANotchWk.var.NotchFil1, freq, &NotchQ ) != OK )
			{ /* QIf the value setting is abnormal, return the set value to the original value and complete the notch */
				/* 2Row notchQValue error */
				rc = 0x08;
			}
			else
			{ /* QValue calculation completed successfully */
				/* 2digit1, */
				ANotchWk.var.NotchSw |= 0x0100;
				/* Notch frequency */
				ANotchWk.var.NotchFil2 = freq;
				/* NotchQvalue */
				ANotchWk.var.NotchQ2 = NotchQ;

				/* Pn408 */
				TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)ANotchWk.var.NotchSw, 2 );
				/* Pn40C */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchfil2, (LONG)ANotchWk.var.NotchFil2, 2 );
				/* Pn40D */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchq2, (LONG)ANotchWk.var.NotchQ2, 2 );

				/* 2Stage notch completed successfully */
				rc = 0x02;
			}
			break;

		case 2:
			/*------------------------------------------------------------------------------*/
			/*	No.1Notch (high frequency) setting													*/
			/*------------------------------------------------------------------------------*/
			if( freq > pndef_ntchfil1.UpperLimit )
			{ /* Limit at upper limit */
				freq = (USHORT)pndef_ntchfil1.UpperLimit;
			}
			/*------------------------------------------------------------------------------*/
			/*	QValue calculation & Parameter calculation													*/
			/*------------------------------------------------------------------------------*/
			if( TuneNotchQset2( ANotchWk.var.NotchFil2, freq, &NotchQ ) != OK )
			{ /* QIf the value setting is abnormal, return the set value to the original value and complete the notch */
				/* 1Row notchQValue error */
				rc = 0x04;
			}
			else
			{ /* QValue calculation completed successfully */
				/* 0th digit1, */
				ANotchWk.var.NotchSw |= 0x0001;
				/* Notch frequency */
				ANotchWk.var.NotchFil1 = freq;
				/* NotchQvalue */
				ANotchWk.var.NotchQ1 = NotchQ;

				/* Pn408 */
				TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)ANotchWk.var.NotchSw, 2 );
				/* Pn409 */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchfil1, (LONG)ANotchWk.var.NotchFil1, 2 );
				/* Pn40A */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchq1, (LONG)ANotchWk.var.NotchQ1, 2 );

				/* 1Stage notch completed successfully */
				rc = 0x01;
			}
			break;

		case 3:
		case 5:
			/*------------------------------------------------------------------------------*/
			/*	No.1Notch (low frequency) setting													*/
			/*------------------------------------------------------------------------------*/
			if( freq > pndef_ntchfil1.UpperLimit )
			{ /* Limit at upper limit */
				freq = (USHORT)pndef_ntchfil1.UpperLimit;
			}
			/*------------------------------------------------------------------------------*/
			/*	QValue calculation & Parameter calculation													*/
			/*------------------------------------------------------------------------------*/
			if( TuneNotchQset( ANotchPrm, KvGain, Jrate, freq, TUNE_ANTFRQ_NTQ1, &NotchQ ) != OK )
			{ /* QIf the value setting is abnormal, return the set value to the original value and complete the notch */
				/* 1Row notchQValue error */
				rc = 0x04;
			}
			else
			{ /* QValue calculation completed successfully */
				ANotchWk.var.TrqFil = TuneNotchTfcalc( ANotchPrm, KvGain, Jrate, freq, NotchQ );
				/* 0digit1, */
				ANotchWk.var.NotchSw |= 0x0001;
				/* Notch frequency */
				ANotchWk.var.NotchFil1 = freq;
				/* NotchQvalue */
				ANotchWk.var.NotchQ1 = NotchQ;

				if( NotchType == 5 )
				{ /* 2Row notchQValue update */
					TuneNotchQset2( ANotchWk.var.NotchFil1, ANotchWk.var.NotchFil2, &NotchQ );
					/* NotchQvalue */
					ANotchWk.var.NotchQ2 = NotchQ;
				}

				ANotchWk.var.TrqFil = TunePrmLimitUnsigned( (ULONG)ANotchWk.var.TrqFil, &pndef_trqfil11 );

				/* Pn401 */
				TunePrmWriteRam( &AccessPrm, &pndef_trqfil11, (LONG)ANotchWk.var.TrqFil, 2 );
				/* Pn408 */
				TunePrmWriteRam( &AccessPrm, &pndef_tfuncsw, (LONG)ANotchWk.var.NotchSw, 2 );
				/* Pn409 */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchfil1, (LONG)ANotchWk.var.NotchFil1, 2 );
				/* Pn40A */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchq1, (LONG)ANotchWk.var.NotchQ1, 2 );
				/* Pn40D */
				TunePrmWriteRam( &AccessPrm, &pndef_ntchq2, (LONG)ANotchWk.var.NotchQ2, 2 );

				/* 1Stage notch completed successfully */
				rc = 0x01;
			}
			break;

		default:
			/*------------------------------------------------------------------------------*/
			/* 1stage,2stage already enabled or Automatic notchOFF									*/
			/*	→ Do nothing																*/
			/*------------------------------------------------------------------------------*/
			rc = 0x10;									/* Condition unsatisfied						*/
			break;
	}
	return rc;
}



/****************************************************************************************************/
/*																									*/
/*			Servo control trace control for automatic notch (Round)										*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function:	Data used for automatic notch vibration analysis is sampled by tracing.					*/
/*				①When there is a trace initialization request														*/
/*						ScanB													*/
/*						ScanCof trace initialization completion												*/
/*				②Servo control trace execution request check												*/
/*					ScanC.			*/
/*						ScanB													*/
/*						ScanCof trace start completion												*/
/*				③Check for trace completion															*/
/*					ScanB, perform the following processing.								*/
/*						ScanCof trace completion													*/
/*																									*/
/*			※The sampling data obtained in this process is immediately used in vibration analysis.					*/
/*			  Vibration analysis alsoRoundDo it with																*/
/*																									*/
/****************************************************************************************************/
SHORT	TuneVibrationTrace( DETVIB *DetVib, FFTANALYZE *FftAna, 
								UCHAR TraceRequest, SHORT TrgLvl, BOOL TuneLessSts )
{
	SHORT	rc;
	SHORT	status;
	SHORT	*DataAdr;

	rc = TUNE_FREQ_SEARCH;

	/* Servo control trace initialization request check */
	if( TraceRequest == FALSE )
	{
		/* Servo control trace forced interruption */
		DetVibTraceStopReq( DetVib );
		rc = TUNE_FREQ_SEARCHINIT;
	}
	else
	{
		if( DetVibTraceGetStatus(DetVib) == DETVIBTRC_TRCEND )
		{
			/* Get trace buffer address */
			DataAdr = DetVibTraceGetDataAdr( DetVib );

			/* Data settings for vibration frequency analysis */
//			FftSetAnaCondition( FftAna, 200, 2000, 80, TuneLessSts );	/* <S21A> */
			FftSetAnaCondition( FftAna, 200, 4000, 80, TuneLessSts );	/* <S21A> */

			/* Vibration frequency analysis */
			status = FftAnalyzeExecute( FftAna, DataAdr );

			if( status != OK )
			{ /* Analysis error */
				rc = TUNE_FREQ_SEARCHERR;
			}
			else
			{ /* Frequency analysis completes normally */
				rc = TUNE_FREQ_SEARCHEND;
			}
		}
		else if( DetVibTraceGetIndex(DetVib) == 0 )
		{ /* Trace for servo control can be executed */
			/*----------------------------------------------------------------------------------*/
			/*	When servo control trace is not started, set and start the trace							*/
			/*----------------------------------------------------------------------------------*/
			/* Trace settings(Data count:200, ScanBPeriod) */
			DetVibTraceSetCondition( DetVib, 200, 1, TrgLvl );
			/* Trace start request */
			DetVibTraceStartReq( DetVib );
		}
	}
	return rc;
}



/****************************************************************************************************/
/*																									*/
/*		When two-inertia MFC is enabledKpmSetting																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	Pn11BUse a quadratic function of the settingsKpm.												*/
/*				Quadratic function:Kpm[0.1s-1] = -0.0036 * Pn11B^2 + 3.8647 * Pn11B - 28.285					*/
/*					Pn11B =  1.0[Hz] → Kpm =   1.0[s-1]											*/
/*					Pn11B = 20.0[Hz] → Kpm =  60.0[s-1]											*/
/*					Pn11B = 50.0[Hz] → Kpm = 100.0[s-1]											*/
/*				becomes. however,Pn11B > 50.0[Hz]teethKpm = 100.0[s-1].					*/
/*																									*/
/****************************************************************************************************/
USHORT	TuneMfcKpmCalcAntFreq( USHORT AntFreq )
{
	LONG	wk, wk1, wk2, wk3;

	if( AntFreq > 500 )
	{ /* If the frequency is within the range50.0Hz */
		/* Model loop gain100.0[s-1]Fixed */
		wk = 1000;
	}
	else
	{ /* If the frequency is within the range50.0HzIn the following cases */
		/*----------------------------------------------------------------------*/
		/* Caution) This quadratic function is the frequency<50.0.						*/
		/*		50Hz,KpmThe value gradually decreases						*/
		/*----------------------------------------------------------------------*/
		/*	Quadratic coefficient  362 = 0.0036 * 10000										*/
		/*----------------------------------------------------------------------*/
		wk1 = (LONG)AntFreq * (LONG)AntFreq * 36;

		/*----------------------------------------------------------------------*/
		/*	Primary coefficient  38647 = 3.8647 * 10000									*/
		/*----------------------------------------------------------------------*/
		wk2 = (LONG)AntFreq * 38647;
		wk3 = -wk1 + wk2 - 282850;			/* 282850 = 28.285 *10000			*/
		wk = wk3 / 10000;
	}

	return (USHORT)wk;
}



/****************************************************************************************************/
/*																									*/
/*		Input information setting for notch filter settingAPI															*/
/*																									*/
/****************************************************************************************************/
/*		Overview:	No.1, No.2Set the notch filter. NotchQThe value is automatically calculated.					*/
/****************************************************************************************************/
void	TuneSetNotchFilSettingData( AUTONOTCH *ANotch, USHORT freq, USHORT KvGain, USHORT Jrate )
{
	ANotch->ANotchData.conf.freq = freq;		/* Auto notch frequency									*/
	ANotch->ANotchData.conf.KvGain = KvGain;	/* Tf, NotchQGain for value calculation						*/
	ANotch->ANotchData.conf.Jrate = Jrate;		/* Tf, NotchQLoad inertia moment for value calculation			*/

	return;
}



/***************************************** end of file **********************************************/

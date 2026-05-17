/*****************************************************************************
Description		: Lifespan prediction function processing
-----------------------------------------------------------------------------
Author		: SHCORE,
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

Functions	  :
	-- APIs --

	LifeTimeSurvivorCalculateMain() :Lifetime remaining rate calculationMainFunction
	LifeTimeSwithingCountInit()		:Lifetime operation count initialization function
	AdetCheckPreWarningSigOut()		:Preventive maintenance warning/preventive maintenance signal output
	AvgInternalTemperatureCalInit()	:Average internal temperature calculation initialization process
	AvgInternalTemperatureCal()		:Average internal temperature calculation process

	-- Locals --
	LifeTimeFanSurvivorCalculate()	:FANLifetime remaining rate calculation function
	LifeTimeCapaSurvivorCalculate()	:Capacitor life remaining rate calculation function
	LifeTimeRushSurvivorCalculate()	:Bump protection life remaining rate calculation function
	LifeTimeDBSurvivorCalculate()	:DBLifetime remaining rate calculation function
	LifeTimeRushSwitchingCount()	:The number of thrust protection relay operations is set when charging is completed Count up
	LifeTimeDBSwitchingCount()		:DBRelay operation count is set while motor is rotatingDBCounts up only when turned on
	LifeTimePrmStoreValueToEeprom() :Writing parameters for lifespan functionToEeprom

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
ZH.QIN		2013.08.25		created

*****************************************************************************/

#include "LifeTimePredictionApi.h"

#define FAN_LIFE_CALC_BASE				26280.0F		/* FANLifetime calculation element */
#define FAN_LIFE_TEMPERATURE_BASE		70.0F			/* FANTemperature calculation element */
#define CAPA_LIFE_CALC_BASE				50000.0F		/* Capacitor life calculation element */
#define CAPA_LIFE_TEMPERATURE_BASE		105.0F			/* Capacitor temperature calculation element */
#define CONVERT_100MS					(10.0F * 60.0F * 60.0F)	/* (10*60*60) = 36000 1hourunit100msunit */
#define LIFE_MONITOR_CALC_FACTOR		10000.0F 		/*Monitor unit is0.01%, so10000double increase*/
#define LIFE_TIME_SURVIVOR_MAX			10000.0F 		/* Lifetime remaining rate Upper limit *//* <S192> */

const float  LifeTimeCalTbl[10] =
{
	1.0F,		/*  2^(0/10) */
	1.07F,		/*  2^(1/10) */
	1.15F,		/*  2^(2/10) */
	1.23F,		/*  2^(3/10) */
	1.31F,		/*  2^(4/10) */
	1.41F,		/*  2^(5/10) */
	1.51F,		/*	2^(6/10) */
	1.62F,		/*  2^(7/10) */
	1.74F,		/*	2^(8/10) */
	1.87F,		/*	2^(9/10) */
};


/****************************************************************************************************/
/*																									*/
/*		APIFunction list																					*/
/*																									*/
/****************************************************************************************************/
void LifeTimeSurvivorCalculateMain( LIFETIME_HANDLE *hLifeTime , REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm );	/* Lifetime remaining rate calculationMainFunction		*/
void LifeTimeSwithingCountInit( LIFETIME_HANDLE *hLifeTime );						/* Lifetime operation count initialization function		*/
//LONG AdetCheckPreWarningSigOut( LIFETIME_HANDLE *hLifeTime, CHECK_MAIN_POWER *PowerChecker );		/* <S180> */
LONG AdetCheckPreWarningSigOut( LIFETIME_HANDLE *hLifeTime, CHECK_MAIN_POWER *PowerChecker, float DBSurvivor );	/* <S180> */
void AvgInternalTemperatureCalInit( AVGTMPR_HANDLE *hAvgTmprHdl, ULONG TimerCnt, SHORT InternalTmpr );
//LONG AvgInternalTemperatureCal(EEP_DEVICE_HANDLE *EepDevice, AVGTMPR_HANDLE *hAvgTmprHdl,		/* <S192> */
//										SHORT CurrentTmpr, ULONG CurrentTimerCnt );
LONG AvgInternalTemperatureCal(EEP_QUE_HANDLE *EepQue, AVGTMPR_HANDLE *hAvgTmprHdl,				/* <S192> */
								SHORT CurrentTmpr, ULONG CurrentTimerCnt );
//void LifeTimeDBSwitchingCount( LIFETIME_HANDLE *hLifeTime , BOOL TgOnSts, BOOL CurrentDbOn );	/* DBRelay operation count is set while motor is rotatingDBCounts up only when turned on*//* <S180> */
void LifeTimeDBSwitchingCount( DBLIFETIME_HADNLE *hDBLifeTime, BOOL TgOnSts, BOOL CurrentDbOn );				/* <S180> */
//void LifeTimePrmStoreValueToEeprom( DBLIFETIME_HADNLE *hDBLifeTime, REGIF_CONFIG_T *RegCfg, CMN_PRMDATA *Cprm );		/* <S180> *//* <S186> *//* <S192> */
void LifeTimeDBPrmStoreValueToEeprom( DBLIFETIME_HADNLE *hDBLifeTime, REGIF_CONFIG_T *RegCfg, PRMDATA *Prm );		/* <S192> */
void LifeTimeDBSurvivorCalculate( DBLIFETIME_HADNLE *hDBLifeTime );			/* <S180> */
void LifeTimeDBSwithingCountInit( DBLIFETIME_HADNLE *hDBLifeTime );			/* <S180> */
void LifeTimeRushPrmStoreValueToEeprom( RUSHLIFETIME_HANDLE	*hRushLifeTime, REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm );	/* <S192> */

/****************************************************************************************************/
/*																									*/
/*		LocalFunction list																				*/
/*																									*/
/****************************************************************************************************/
static void LifeTimeFanSurvivorCalculate( LIFETIME_HANDLE *hLifeTime );		/* FANLifetime remaining rate calculation function 		*/
static void LifeTimeCapaSurvivorCalculate( LIFETIME_HANDLE *hLifeTime );	/* Capacitor life remaining rate calculation function	*/
static void LifeTimeRushSurvivorCalculate( LIFETIME_HANDLE *hLifeTime );	/* Bump protection life remaining rate calculation function		*/
//static void LifeTimeDBSurvivorCalculate( DBLIFETIME_HADNLE *hDBLifeTime );		/* DBLifetime remaining rate calculation function		*//* <S180> */
//static void LifeTimeRushSwitchingCount( LIFETIME_HANDLE *hLifeTime );		/* The number of thrust protection relay operations is set when charging is completed Count up	*//* <S180> */
static void LifeTimeRushSwitchingCount( LIFETIME_HANDLE *hLifeTime, REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm );		/* <S180> */
//static void LifeTimePrmStoreValueToEeprom( LIFETIME_HANDLE *hLifeTime , REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm ); /* Writing parameters for lifespan functionToEeprom *//* <S180> */

/*****************************************************************************
  Description:	The number of thrust protection relay operations is set when charging is completed Count up
------------------------------------------------------------------------------
  Parameters:
	hLifeTime;				Lifetime prediction function handle
	RegCfg;					Regsiter Manager
	Cprm;					Unit common parameter structure
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
//static void LifeTimeRushSwitchingCount( LIFETIME_HANDLE *hLifeTime )		/* <S180> */
static void LifeTimeRushSwitchingCount( LIFETIME_HANDLE *hLifeTime, REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm )	/* <S180> */
{

	if ( hLifeTime->RushLifeTimeHdl.var.CurrentChargeOk == TRUE )										/* Charging complete */
	{
		if ( hLifeTime->RushLifeTimeHdl.var.LastChargeOk ==  FALSE)		/* Previous charging is incomplete	*/
		{
			hLifeTime->RushLifeTimeHdl.var.RushSwitchCount ++;
			/* <S180> Start */
			/* Current anti-fault relay operation count the value ofEeprom */
//			Cprm->RushSwichCount = hLifeTime->RushLifeTimeHdl.var.RushSwitchCount;		/* <S192> */
//			PrmStoreValueToEeprom( RegCfg, &pndef_RushSwichCount, 0 );					/* <S192> */
//			hLifeTime->RushLifeTimeHdl.var.RushCountChanged = TRUE;
			/* <S180> End */
		}
	}
	hLifeTime->RushLifeTimeHdl.var.LastChargeOk = hLifeTime->RushLifeTimeHdl.var.CurrentChargeOk;
}

/*****************************************************************************
  Description:	DBRelay operation count is set while motor is rotatingDBCounts up only when turned on
------------------------------------------------------------------------------
  Parameters:
	hDBLifeTime;				DBLifetime prediction function handle
	TgOnSts						Motor rotation status
	CurrentDbOn					DB ON Flag
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
#if 0		/* <S180> */
void LifeTimeDBSwitchingCount( LIFETIME_HANDLE *hLifeTime , BOOL TgOnSts, BOOL CurrentDbOn )
{
	if ( TgOnSts == TRUE )				/* While the motor is rotating */
	{
		/*Previous state is DBOff and current stateDBWhen on*/
		if ( (hLifeTime->DBLifeTimeHdl.var.LastDbOn == FALSE) && (CurrentDbOn == TRUE ) )
		{
			hLifeTime->DBLifeTimeHdl.var.DBSwitchCount ++;
			hLifeTime->DBLifeTimeHdl.var.DBCountChanged = TRUE ;
		}
	}
	hLifeTime->DBLifeTimeHdl.var.LastDbOn = CurrentDbOn;
}
#else		/* <S180> */
void LifeTimeDBSwitchingCount( DBLIFETIME_HADNLE *hDBLifeTime, BOOL TgOnSts, BOOL CurrentDbOn )
{
	if ( TgOnSts == TRUE )				/* While the motor is rotating */
	{
		/*Previous state is DBOff and current stateDBWhen on*/
		if ( (hDBLifeTime->var.LastDbOn == FALSE) && (CurrentDbOn == TRUE ) )
		{
			hDBLifeTime->var.DBSwitchCount ++;
//			hDBLifeTime->var.DBCountChanged = TRUE ;	/* <S192> */
		}
	}
	hDBLifeTime->var.LastDbOn = CurrentDbOn;
}
#endif
/*****************************************************************************
  Description:	DBRelay operation operation count initialization function
------------------------------------------------------------------------------
  Parameters:
	hLifeTime;				Lifetime prediction function handle
	hDBLifeTime;			DBLifetime prediction function handle
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
void LifeTimeSwithingCountInit( LIFETIME_HANDLE *hLifeTime )
{
//	hLifeTime->DBLifeTimeHdl.var.LastDbOn = FALSE;	/* <S180> */
	hLifeTime->RushLifeTimeHdl.var.LastChargeOk = FALSE;
}
/* <S180> Start */
/*****************************************************************************
  Description:	DBInitialize operation count function
------------------------------------------------------------------------------
  Parameters:
	hDBLifeTime;			DBLifetime prediction function handle
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
void LifeTimeDBSwithingCountInit( DBLIFETIME_HADNLE *hDBLifeTime )
{
	hDBLifeTime->var.LastDbOn = FALSE;
}
/* <S180> End */

/*****************************************************************************
  Description:	Lifetime remaining rate calculationMainFunction
------------------------------------------------------------------------------
  Parameters:
	hLifeTime;				Lifetime prediction function handle
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
void LifeTimeSurvivorCalculateMain( LIFETIME_HANDLE *hLifeTime , REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm )
{
//	LifeTimeRushSwitchingCount( hLifeTime );			/* The number of thrust protection relay operations is set when charging is completed Count up	*//* <S180> */
	LifeTimeRushSwitchingCount( hLifeTime, RegCfg, Cprm );		/* The number of thrust protection relay operations is set when charging is completed Count up	*//* <S180> */
	LifeTimeFanSurvivorCalculate( hLifeTime );			/* FANLifetime remaining rate calculation  */
	LifeTimeCapaSurvivorCalculate( hLifeTime );			/* Capacitor life remaining rate calculation */
	LifeTimeRushSurvivorCalculate( hLifeTime );			/*Bump protection life remaining rate calculation  */
//	LifeTimeDBSurvivorCalculate( hLifeTime );			/* DBLifetime remaining rate calculation *//* <S180> */
//	LifeTimePrmStoreValueToEeprom( hLifeTime , RegCfg , Cprm ); /* Writing parameters for lifespan functionTOEeprom*//* <S180> */
}

/*****************************************************************************
  Description:	FANLifetime remaining rate calculation function
------------------------------------------------------------------------------
  Parameters:
	hLifeTime;				Lifetime prediction function handle
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static void LifeTimeFanSurvivorCalculate( LIFETIME_HANDLE *hLifeTime )
{
	float FanLifeTime;				/* Built-inFANLifetime		*/
	float FanTemperatureOffset;     /* Ambient temperature offset 	*/
	float AroundTemperature;		/*	Ambient temperature			*/
	float AvgIntTmpr;				/* Average internal temperature		*/
	float FanTimeOffset;			/* FANReplacement offset time*/
	float Timer100msCounter;
	float val;
	float ftmp;
	ULONG tmp1;
	ULONG tmp2;
	AvgIntTmpr = (float)hLifeTime->AvgTmprHdl.var.AvgIntTmpr * 0.01F ;			/* Unit is℃*/
	FanTemperatureOffset = hLifeTime->FanLifeTimeHdl.conf.FanTemperatureOffset;
	AroundTemperature = AvgIntTmpr - FanTemperatureOffset;						/* Ambient temperature : Average internal temperature - Ambient temperature offset*/
	Timer100msCounter = (float)KlibGetTimeStamp();
	if ( AroundTemperature < 0.0F)
	{
		AroundTemperature = 0.0F;
	}

	tmp1 = (ULONG)(( FAN_LIFE_TEMPERATURE_BASE - AroundTemperature ) * 0.1F ); 						/* k = (70 - x) / 10 x: Average internal temperature - Ambient temperature offset				*/
	tmp2 = (ULONG)(( FAN_LIFE_TEMPERATURE_BASE - AroundTemperature ) - (float)(tmp1 * 10U));		/* tmp2 = (70 - x) - (tmp1 * 10) Round down to the decimal point					*/
	ftmp = LifeTimeCalTbl[tmp2];																	/* ← 2^(0/10) ~ 2^(10/10)					*/
	FanLifeTime = (float)(( FAN_LIFE_CALC_BASE * ( 1 << tmp1 )) * ftmp )* CONVERT_100MS;			/* Built-inFANLifetime = 26280 * (1<<tmp1) * ftmp 	Unit is100ms				*/
	/* Calculation formula: Value unit(100ms) = FanTimeOffset(unit10hour) * 10 *CONVERT_100MS (1hourunit100msunit) 	*/
	FanTimeOffset =  hLifeTime->FanLifeTimeHdl.conf.FanTimeOffset  * 10.0F * CONVERT_100MS; 		/* Unit10hto100ms */


	if ( Timer100msCounter > FanTimeOffset )
	{
#if 0	/* <S192> */
		/* Built-inFANResidual rate = ((Cumulative operating time - FANReplacement offset time) / Built-inFANLifetime) × 100*/
		val = (((Timer100msCounter - FanTimeOffset) ) / FanLifeTime) * LIFE_MONITOR_CALC_FACTOR ;
		if (val >= 65535.0F)
		{
			hLifeTime->FanLifeTimeHdl.var.FanSurvivor = 65535.0F;
		}
		else
		{
			hLifeTime->FanLifeTimeHdl.var.FanSurvivor = val ;     			/* Built-inFANLifetime remaining rate = ((Cumulative operating time - FANReplacement offset time / Built-inFANLifetime) × 100) */
		}
#else	/* <S192> */
		/* Built-inFANLifetime remaining rate = 100 - ((Cumulative operating time - FANReplacement offset time / Built-inFANLifetime) × 100) */
		val = (((Timer100msCounter - FanTimeOffset) ) / FanLifeTime) * LIFE_MONITOR_CALC_FACTOR ;
		hLifeTime->FanLifeTimeHdl.var.FanSurvivor = LIFE_TIME_SURVIVOR_MAX - val;
		if ( hLifeTime->FanLifeTimeHdl.var.FanSurvivor < 0.0F )
		{
			hLifeTime->FanLifeTimeHdl.var.FanSurvivor = 0.0F;
		}
#endif
	}
	else
	{
//		hLifeTime->FanLifeTimeHdl.var.FanSurvivor = 0.0F;					/* <S192> */
		hLifeTime->FanLifeTimeHdl.var.FanSurvivor = LIFE_TIME_SURVIVOR_MAX;	/* <S192> */
	}
}

/*****************************************************************************
  Description:	Capacitor life remaining rate calculation function
------------------------------------------------------------------------------
  Parameters:
	hLifeTime;				Lifetime prediction function handle
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static void LifeTimeCapaSurvivorCalculate( LIFETIME_HANDLE *hLifeTime )
{
	float CapaTime;										/* Capacitor life */
	float AvgIntTmpr;									/* Average internal temperature		*/
	float val;
	float Timer100msCounter;
	float ftmp;
	ULONG tmp1;
	ULONG tmp2;
	AvgIntTmpr = (float)hLifeTime->AvgTmprHdl.var.AvgIntTmpr * 0.01F ;							/* Unit is 1℃*/
	Timer100msCounter = (float)KlibGetTimeStamp();

	tmp1 = (ULONG)(( CAPA_LIFE_TEMPERATURE_BASE - AvgIntTmpr ) * 0.1F );						/* k = (105 - Average internal temperature) / 10 									*/
	tmp2 = (ULONG)(( CAPA_LIFE_TEMPERATURE_BASE - AvgIntTmpr ) - (float)( tmp1 * 10U ));		/* tmp2 = (105 - x) - (tmp1 * 10) Round down to the decimal point				*/
	ftmp = LifeTimeCalTbl[tmp2];																/* ← 2^(0/10) ~ 2^(10/10)				*/
	CapaTime = (float)( CAPA_LIFE_CALC_BASE * ( 1 << tmp1 ) * ftmp ) * CONVERT_100MS;  			/* Capacitor life = 50000 × (1<<tmp1) * ftmp  Unit is100MS			*/
#if 0	/* <S192> */
	/* Built-in capacitor life remaining rate = ((Cumulative operating time / Capacitor life) × 100) */
	val = ( Timer100msCounter / CapaTime ) * LIFE_MONITOR_CALC_FACTOR ;
	if ( val >= 65535.0F )
	{
		hLifeTime->CapaLifeTimeHdl.var.CapaSurvivor =  65535.0F ;
	}
	else
	{
		hLifeTime->CapaLifeTimeHdl.var.CapaSurvivor = val;
	}
#else	/* <S192> */
	/* Built-in capacitor life remaining rate = 100 - ((Cumulative operating time / Capacitor life) × 100) */
	val = ( Timer100msCounter / CapaTime ) * LIFE_MONITOR_CALC_FACTOR ;
	hLifeTime->CapaLifeTimeHdl.var.CapaSurvivor = LIFE_TIME_SURVIVOR_MAX - val;
	if ( hLifeTime->CapaLifeTimeHdl.var.CapaSurvivor < 0.0F )
	{
		hLifeTime->CapaLifeTimeHdl.var.CapaSurvivor =  0.0F ;
	}
#endif
}

/*****************************************************************************
  Description:	Bump protection life remaining rate calculation function
------------------------------------------------------------------------------
  Parameters:
	hLifeTime;				Lifetime prediction function handle
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static void LifeTimeRushSurvivorCalculate( LIFETIME_HANDLE *hLifeTime )
{
	float val;
#if 0	/* <S192> */
	/* Usage status[%] = ((Current number of operations(PnE84) / Lifetime count(PnE82)) * 100) */
	val =  (float)hLifeTime->RushLifeTimeHdl.var.RushSwitchCount * hLifeTime->RushLifeTimeHdl.conf.RushLifeTimeReciprocal * LIFE_MONITOR_CALC_FACTOR ;
	if ( val  >= 65535.0F  )
	{
		hLifeTime->RushLifeTimeHdl.var.RushSurvivor =   65535.0F  ;
	}
	else
	{
		hLifeTime->RushLifeTimeHdl.var.RushSurvivor = val;
	}
#else	/* <S192> */
	/* Bump protection life remaining rate = 100 - ((Current number of operations(PnE84) / Lifetime count(PnE82)) * 100) */
	val =  (float)hLifeTime->RushLifeTimeHdl.var.RushSwitchCount *
			hLifeTime->RushLifeTimeHdl.conf.RushLifeTimeReciprocal * LIFE_MONITOR_CALC_FACTOR ;
	hLifeTime->RushLifeTimeHdl.var.RushSurvivor = LIFE_TIME_SURVIVOR_MAX - val;
	if ( hLifeTime->RushLifeTimeHdl.var.RushSurvivor < 0.0F )
	{
		hLifeTime->RushLifeTimeHdl.var.RushSurvivor = 0.0F;
	}
#endif
}

/*****************************************************************************
  Description:	DBLifetime remaining rate calculation function
------------------------------------------------------------------------------
  Parameters:
	hDBLifeTime;			DBLifetime prediction function handle
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
//static void LifeTimeDBSurvivorCalculate( DBLIFETIME_HADNLE *hDBLifeTime )		/* <S180> */
void LifeTimeDBSurvivorCalculate( DBLIFETIME_HADNLE *hDBLifeTime )				/* <S180> */
{
	float val;
#if 0	/* <S192> */
	/* Usage status[%] = ((Current number of operations(PnE86) / Lifetime count(PnE84)) * 100) */
	val = (float)hDBLifeTime->var.DBSwitchCount * hDBLifeTime->conf.DBLifeTimeReciprocal * LIFE_MONITOR_CALC_FACTOR ;
	if ( val  >= 65535.0F  )
	{
		hDBLifeTime->var.DBSurvivor =  65535.0F ;
	}
	else
	{
		hDBLifeTime->var.DBSurvivor = val;
	}
#else	/* <S192> */
	/* DBLifetime remaining rate = 100 - ((Current number of operations(PnE86) / Lifetime count(PnE84)) * 100) */
	val = (float)hDBLifeTime->var.DBSwitchCount *
			hDBLifeTime->conf.DBLifeTimeReciprocal * LIFE_MONITOR_CALC_FACTOR ;
	hDBLifeTime->var.DBSurvivor = LIFE_TIME_SURVIVOR_MAX - val;
	if ( hDBLifeTime->var.DBSurvivor < 0.0F )
	{
		hDBLifeTime->var.DBSurvivor = 0.0F;
	}
#endif
}
/* <S192> Start */
/*****************************************************************************
  Description:	 Writing parameters for the number of thrust protection relay operationsTOEeprom
------------------------------------------------------------------------------
  Parameters:
	hRushLifeTime;			Bump protection life prediction function handle
	RegCfg;					Regsiter Manager
	Prm;					Parameter data
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
void LifeTimeRushPrmStoreValueToEeprom( RUSHLIFETIME_HANDLE	*hRushLifeTime, REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm )
{
	hRushLifeTime->var.RushSwCountLatch = hRushLifeTime->var.RushSwitchCount;
	if ( Cprm->RushSwichCount !=  hRushLifeTime->var.RushSwCountLatch )		/*  When changing the number of thrust protection relay operations	*/
	{
		/* Current anti-fault relay operation count the value ofEeprom */
		Cprm->RushSwichCount = hRushLifeTime->var.RushSwCountLatch;
		PrmStoreValueToEeprom( RegCfg, &pndef_RushSwichCount, 0 );
	}
}
/* <S192> End */
/* <S180> Start */
/*****************************************************************************
  Description:	 DBWrite parameter for relay operation countTOEeprom
------------------------------------------------------------------------------
  Parameters:
	hDBLifeTime;			DBLifetime prediction function handle
	RegCfg;					Regsiter Manager
	Prm;					Parameter data
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
//void LifeTimePrmStoreValueToEeprom( DBLIFETIME_HADNLE *hDBLifeTime, REGIF_CONFIG_T *RegCfg, PRMDATA *Prm )		/* <S186> *//* <S192> */
void LifeTimeDBPrmStoreValueToEeprom( DBLIFETIME_HADNLE *hDBLifeTime, REGIF_CONFIG_T *RegCfg, PRMDATA *Prm )
{
//	if ( hDBLifeTime->var.DBCountChanged == TRUE )			/* DBWhen changing relay operation count	*/
	hDBLifeTime->var.DBSwCountLatch = hDBLifeTime->var.DBSwitchCount;	/* <S192> */
	if ( Prm->DBSwichCount != hDBLifeTime->var.DBSwCountLatch )			/* <S192> */
	{
		/* Current currentDBNumber of operations the value ofEeprom*/
//		Cprm->DBSwichCount = hDBLifeTime->var.DBSwitchCount;		/* <S186> *//* <S192> */
		Prm->DBSwichCount = hDBLifeTime->var.DBSwCountLatch;		/* <S192> */
		PrmStoreValueToEeprom( RegCfg, &pndef_DBSwichCount, 0 );
	}
//	hDBLifeTime->var.DBCountChanged = FALSE;		/* <S192> */
}
#if 0
/*****************************************************************************
  Description:	 Writing parameters for lifespan functionTOEeprom
------------------------------------------------------------------------------
  Parameters:
	hLifeTime;				Lifetime prediction function handle
	RegCfg;					Regsiter Manager
	Cprm;					Unit common parameter structure
  Return:
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static void LifeTimePrmStoreValueToEeprom( LIFETIME_HANDLE *hLifeTime , REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm )
{
	if ( hLifeTime->DBLifeTimeHdl.var.DBCountChanged == TRUE )			/* DBWhen changing relay operation count	*/
	{
		Cprm->DBSwichCount = hLifeTime->DBLifeTimeHdl.var.DBSwitchCount & 0xFFFF;
		PrmStoreValueToEeprom( RegCfg, &pndef_DBSwichCount, 0 );		/* Current currentDBNumber of operations (lower16bit) the value ofEeprom*/
	}
	if ( hLifeTime->RushLifeTimeHdl.var.RushCountChanged == TRUE )		/* When changing the number of thrust protection relay operations 	*/
	{
		Cprm->RushSwichCount = hLifeTime->RushLifeTimeHdl.var.RushSwitchCount & 0xFFFF;
		PrmStoreValueToEeprom( RegCfg, &pndef_RushSwichCount, 0 );		/* Current anti-fault relay operating count (lower16bit) the value ofEeprom*/
	}

	if (hLifeTime->RushLifeTimeHdl.var.RushCountChanged == TRUE || hLifeTime->DBLifeTimeHdl.var.DBCountChanged == TRUE )
	{
		Cprm->DRSwithingCount = (( hLifeTime->RushLifeTimeHdl.var.RushSwitchCount & 0x00FF0000 ) >> 16 )
													+ ( hLifeTime->DBLifeTimeHdl.var.DBSwitchCount & 0x00FF0000 >> 8 );
		PrmStoreValueToEeprom( RegCfg, &pndef_DRSwithingCount, 0 );		/* Current anti-fault protection/DBRelay operation count (upper) the value ofEeprom*/
	}
	hLifeTime->RushLifeTimeHdl.var.RushCountChanged = FALSE;
	hLifeTime->DBLifeTimeHdl.var.DBCountChanged = FALSE;
}
#endif
/* <S180> End */
/*****************************************************************************
  Description: Preventive maintenance warning/preventive maintenance signal output
------------------------------------------------------------------------------
  Parameters:
	hLifeTime;				Lifetime prediction function handle
	PowerChecker;			Main circuit monitoring structure definition
  Return:
	RC						Return value
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
//LONG AdetCheckPreWarningSigOut( LIFETIME_HANDLE *hLifeTime, CHECK_MAIN_POWER *PowerChecker )	/* <S180> */
LONG AdetCheckPreWarningSigOut( LIFETIME_HANDLE *hLifeTime, CHECK_MAIN_POWER *PowerChecker, float DBSurvivor )	/* <S180> */
{
	LONG	RC;
	RC = CHK_PRE_NO_OPERATION;


	/* CoolingFAN, Capacitor, Rush prevention circuit,DBThe life of one of the circuits is the threshold(90%) */
#if 0	/* <S192> */
	if( ( hLifeTime->FanLifeTimeHdl.var.FanSurvivor > 9000.0F ) ||
		( hLifeTime->CapaLifeTimeHdl.var.CapaSurvivor > 9000.0F ) ||
		( hLifeTime->RushLifeTimeHdl.var.RushSurvivor > 9000.0F ) ||
		( DBSurvivor > 9000.0F ) )		/* <S180> */
//		( hLifeTime->DBLifeTimeHdl.var.DBSurvivor > 9000.0F ) )		/* <S180> */
#else	/* <S192> */
	if( ( hLifeTime->FanLifeTimeHdl.var.FanSurvivor < 1000.0F ) ||
		( hLifeTime->CapaLifeTimeHdl.var.CapaSurvivor < 1000.0F ) ||
		( hLifeTime->RushLifeTimeHdl.var.RushSurvivor < 1000.0F ) ||
		( DBSurvivor < 1000.0F ) )
#endif
	{
		if( PowerChecker->conf.PreWarningSw == TRUE )
		{
			RC = CHK_PRE_SET_WRN;
		}
		PowerChecker->var.PreSigOut = TRUE;				/* Preventive maintenance signal output */
	}
	else
	{
		PowerChecker->var.PreSigOut = FALSE;
	}
	return( RC );
}


/*****************************************************************************
  Description: Average internal temperature calculation initialization process
------------------------------------------------------------------------------
  Parameters:
	hAvgTmprHdl			Average Internal Temperature Cal. Hnadle
	TimerCnt			Operating time
	InternalTmpr		Average internal temperature[0.01℃]value
  Return:

------------------------------------------------------------------------------
  Note:
*****************************************************************************/
void AvgInternalTemperatureCalInit( AVGTMPR_HANDLE *hAvgTmprHdl, ULONG TimerCnt, SHORT InternalTmpr )
{
	/*Average internal temperature [0.01℃]Initializing variables*/
	hAvgTmprHdl->var.AvgIntTmpr = InternalTmpr;

	/*Initializing operating time at last update*/
	hAvgTmprHdl->var.Timer100msCntLast = TimerCnt;

	return;
}


/*****************************************************************************
  Description: Average internal temperature calculation process
------------------------------------------------------------------------------
  Parameters:
	EepDevice			Eeprom Device handle
	hAvgTmprHdl			Average Internal Temperature Cal. Hnadle
	CurrentTmpr			Current temperature information [℃]
	CurrentTimerCnt		Current operating time
  Return:
	Error
------------------------------------------------------------------------------
  Note:
*****************************************************************************/
//LONG AvgInternalTemperatureCal(EEP_DEVICE_HANDLE *EepDevice, AVGTMPR_HANDLE *hAvgTmprHdl,	/* <S192> */
LONG AvgInternalTemperatureCal(EEP_QUE_HANDLE *EepQue, AVGTMPR_HANDLE *hAvgTmprHdl,			/* <S192> */
								SHORT CurrentTmpr, ULONG CurrentTimerCnt )
{
	LONG	lwk1;
	LONG	lwk2;
	SHORT	CurrentAvgTmpr;		/* New average internal temperature used */
	LONG	rslt;

	rslt = 0;

	/* Changing the unit of current temperature information0.1℃" -> "0.01℃"*/
	CurrentTmpr = CurrentTmpr * 10;

	/* Calculation of average internal temperature in use */
	lwk1 = (LONG)( hAvgTmprHdl->var.Timer100msCntLast >> 14 );		/* Average number of times to date */
	lwk2 = ( hAvgTmprHdl->var.AvgIntTmpr * lwk1 ) + CurrentTmpr;	/* Total value to date */
	CurrentAvgTmpr = (SHORT)( lwk2 / (lwk1 + 1) );					/* New average internal temperature used */

	/* Average temperatureEEPROMWrite */
	if ( CurrentAvgTmpr != hAvgTmprHdl->var.AvgIntTmpr )
	{
		/* Updating the value of the average internal temperature variable */
		hAvgTmprHdl->var.AvgIntTmpr = CurrentAvgTmpr;

		/* Average internal temperature valueEEPROM  */
//		rslt = EepdevWriteNoSumValues( 0,							/* <S192> *//* <S1D2> */
//									   EEP_NOSUMCMN_ADDRESS(EEP_AVRINTERTMPR_OFS),
//									   (USHORT*)&CurrentAvgTmpr, 1 );
		rslt = EepdevPutQueue( EepQue, 0, EEP_NOSUMCMN_ADDRESS(EEP_AVRINTERTMPR_OFS),	/* <S192> */
								(USHORT)CurrentAvgTmpr, 1 );
	}

	/* Operating time at last update = Current operating time */
	hAvgTmprHdl->var.Timer100msCntLast = CurrentTimerCnt;

	return rslt;
}








/***************************************** end of file **********************************************/



/*****************************************************************************
Description		: Lifetime prediction function head feel
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
ZH.QIN		2013.08.24		created

*****************************************************************************/

#ifndef LIFE_TIME_API_H
#define LIFE_TIME_API_H

#include "EepromIF.h"
#include "PowerManager.h"
#include "PnPrmListTbl.h"
#include "BaseEnableSequence.h"
#include "UnitPrmStruct.h"

/*--------------------------------------------------------------------------------------------------*/
/*		Definition for preventive maintenance function																			*/
/*--------------------------------------------------------------------------------------------------*/
#define	CHK_PRE_NO_OPERATION	0x00			/* No alarm(Processing not required)							*/
#define CHK_PRE_SET_WRN		0x01				/* All axes alarm output									*/

/*--------------------------------------------------------------------------------------------------*/
/*		FANLifetime prediction function handle definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _FANLIFETIME_HADNLE
{
struct{
	float FanTemperatureOffset;					/* Ambient temperature offset								*/
	float FanTimeOffset;						/* FANReplacement offset time							*/
	}conf;
struct{
	float FanSurvivor;							/* Built-inFANLifetime remaining rate								*/
	}var;
}FANLIFETIME_HADNLE;

/*--------------------------------------------------------------------------------------------------*/
/*		Capacitor life prediction function handle definition															*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _CAPALIFETIME_HANDLE
{
struct {
	float	CapaSurvivor;						/* Capacitor life remaining rate								*/
	}var;
}CAPALIFETIME_HANDLE;


/*--------------------------------------------------------------------------------------------------*/
/*		Bump protection life prediction function handle definition																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _RUSHLIFETIME_HANDLE
{
struct {
	float	RushLifeTimeReciprocal;				/* Rush protection circuit life(Unit once)reciprocal of					*/
	}conf;
struct {
	float	RushSurvivor;						/* Bump protection circuit life remaining rate								*/
	ULONG	RushSwitchCount;					/* Number of thrust protection relay operations								*/
	ULONG	RushSwCountLatch;					/* Latch of the number of thrust protection relay operations							*//* <S192> */
//	BOOL	RushCountChanged;					/* Changing the number of thrust protection relay operations							*//* <S180> */
	BOOL	LastChargeOk;						/* PreviousChargeOkStatus								*/
	BOOL 	CurrentChargeOk;					/* This timeChargeOkStatus								*/
	}var;
}RUSHLIFETIME_HANDLE;

/*--------------------------------------------------------------------------------------------------*/
/*		DBLifetime prediction function handle definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _DBLIFETIME_HADNLE
{
struct {
	float	DBLifeTimeReciprocal;				/* DBCircuit life(Unit once)reciprocal of							*/
	}conf;
struct {
	float	DBSurvivor;							/* DBCircuit life remaining rate									*/
	ULONG	DBSwitchCount;						/* DBRelay operation count									*/
	ULONG	DBSwCountLatch;						/* DBRelay operation count latch								*//* <S192> */
//	BOOL	DBCountChanged;						/* DBChange relay operation count								*//* <S192> */
	BOOL	LastDbOn;							/* Previous Dbon	Status								*/
	}var;

}DBLIFETIME_HADNLE;

/*--------------------------------------------------------------------------------------------------*/
/*		Handle definition for average temperature calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _AVG_TEMPERATURE_HANDLE
{
	struct
	{
		ULONG	Timer100msCntLast;				/* Operating time when last average temperature was updated [100mS]				*/
		SHORT	AvgIntTmpr;						/* Average internal temperature [0.01℃]							*/
		USHORT	Dummy;
	}var;

}AVGTMPR_HANDLE;


/*--------------------------------------------------------------------------------------------------*/
/*		Lifetime prediction function handle definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _LIFETIME_HANDLE
{
	FANLIFETIME_HADNLE		FanLifeTimeHdl;		/* FANLifetime prediction function									*/
	CAPALIFETIME_HANDLE		CapaLifeTimeHdl;	/* Capacitor life function								*/
	RUSHLIFETIME_HANDLE		RushLifeTimeHdl;	/* Bump protection life prediction function									*/
//	DBLIFETIME_HADNLE		DBLifeTimeHdl;		/* DBLifetime prediction function									*//* <S180> */
	AVGTMPR_HANDLE			AvgTmprHdl;			/* For average temperature calculation									*/
} LIFETIME_HANDLE;



/****************************************************************************************************/
/*																									*/
/*		Function list																					*/
/*																									*/
/****************************************************************************************************/
extern void LifeTimeSurvivorCalculateMain( LIFETIME_HANDLE *hLifeTime , REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm );// Lifetime remaining rate calculationMainFunction
//extern LONG AdetCheckPreWarningSigOut( LIFETIME_HANDLE *hLifeTime, CHECK_MAIN_POWER *PowerChecker );		/* <S180> */
LONG AdetCheckPreWarningSigOut( LIFETIME_HANDLE *hLifeTime, CHECK_MAIN_POWER *PowerChecker, float DBSurvivor );	/* <S180> */
extern void LifeTimeSwithingCountInit( LIFETIME_HANDLE *hLifeTime );					// Lifetime operation count initialization function

extern void AvgInternalTemperatureCalInit( AVGTMPR_HANDLE *hAvgTmprHdl, ULONG TimerCnt, SHORT InternalTmpr );
//LONG AvgInternalTemperatureCal(EEP_DEVICE_HANDLE *EepDevice, AVGTMPR_HANDLE *hAvgTmprHdl,		/* <S192> */
//										SHORT CurrentTmpr, ULONG CurrentTimerCnt );
LONG AvgInternalTemperatureCal(EEP_QUE_HANDLE *EepQue, AVGTMPR_HANDLE *hAvgTmprHdl,				/* <S192> */
								SHORT CurrentTmpr, ULONG CurrentTimerCnt );
//extern void LifeTimeDBSwitchingCount( LIFETIME_HANDLE *hLifeTime , BOOL TgOnSts, BOOL CurrentDbOn );		/* DBRelay operation count is set while motor is rotatingDBCounts up only when turned on*//* <S180> */
void LifeTimeDBSwitchingCount( DBLIFETIME_HADNLE *hDBLifeTime, BOOL TgOnSts, BOOL CurrentDbOn );	/* <S180> */
//void LifeTimePrmStoreValueToEeprom( DBLIFETIME_HADNLE *hDBLifeTime, REGIF_CONFIG_T *RegCfg, CMN_PRMDATA *Cprm );		/* <S180> *//* <S186> *//* <S192> */
void LifeTimeDBSurvivorCalculate( DBLIFETIME_HADNLE *hDBLifeTime );			/* <S180> */
void LifeTimeDBSwithingCountInit( DBLIFETIME_HADNLE *hDBLifeTime );			/* <S180> */
void LifeTimeDBPrmStoreValueToEeprom( DBLIFETIME_HADNLE *hDBLifeTime, REGIF_CONFIG_T *RegCfg, PRMDATA *Prm );		/* <S192> */
void LifeTimeRushPrmStoreValueToEeprom( RUSHLIFETIME_HANDLE	*hRushLifeTime, REGIF_CONFIG_T *RegCfg , CMN_PRMDATA *Cprm );	/* <S192> */
#endif /* LIFE_TIME_API_H */

/***************************************** end of file **********************************************/


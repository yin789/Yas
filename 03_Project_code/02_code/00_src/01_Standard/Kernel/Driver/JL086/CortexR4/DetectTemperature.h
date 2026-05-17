#ifndef __DETECT_TEMPERATURE_
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		DetectTemp.h : Temperature Detect IC Access Driver						<S03A>				*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Machine Efficiency : I2C/Temperature IC Device Driver for S-5851AAA-M6T1									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Author   : Yaskawa Electric Corp.,															*/
/*					Copyright (c) 2013 All Rights Reserved											*/
/*																									*/
/*		Project  : Mercury																			*/
/*																									*/
/*		Rev.1.00 : 2013.10.27  Y.Oka																*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"


/****************************************************************************************************/
/*																									*/
/*		DEFINES																						*/
/*																									*/
/****************************************************************************************************/
#define	DETECT_TEMP_TIMING		100			/* Temperature detection period[ms]										*/
											/* Σ-7So the temperatureICbut2points are the sameICAccess to	*/
											/* DETECT_TEMP_TIMING * 2[ms]becomes.					*/
#define	TEMP_IC_NUM				2			/* Temperature detectionIC										*/

enum TEMP_DEVICE_ID {						/* Temperature detectionICFor switchingID								*/
	CTRL_BOARD_TEMPIC = 0,
	POWER_BOARD_TEMPIC,
};

/****************************************************************************************************/
/*																									*/
/*		STRUCT DEFINITION																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Temperature monitoring definition																				*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _TEMP_MANAGER {
	struct {								/* Temperature monitoring variable definition									*/
		SHORT		CtrlBoardTemp;				/* Control board current temperature[℃]								*/
		SHORT		PowerBoardTemp;				/* Power board current temperature[℃]							*/
		SHORT		CtrlBoardTempMax;			/* Control board maximum temperature[℃]								*/
		SHORT		PowerBoardTempMax;			/* Power board maximum temperature[℃]							*/
		USHORT		CtrlBoardI2CErrCnt;			/* Continuous control board communication error count						*//* <S0DE> */
		USHORT		PowerBoardI2CErrCnt;		/* Number of consecutive power board communication errors						*//* <S0DE> */
	/*----------------------------------------------------------------------------------------------*/
		USHORT		DeviceID;					/* Control boardIC/Power boardIC							*/
		ULONG		DetTempTimer;				/* Timer for temperature detection timing						*/
	} var;
} TEMP_MANAGER;



#if	(CSW_DETECT_TEMPERATURE == TRUE)
/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
LONG	DetTempInitialize( TEMP_MANAGER *TempManager );
LONG	DetTempManager( TEMP_MANAGER *TempManager );

/****************************************************************************************************/
/*																									*/
/*		API Function																				*/
/*																									*/
/****************************************************************************************************/
SHORT DetTempGetInternalTemperature( TEMP_MANAGER *TempManager );
SHORT DetTempGetCtrlBoardTemperature( TEMP_MANAGER *TempManager );
SHORT DetTempGetPowerBoardTemperature( TEMP_MANAGER *TempManager );
USHORT DetTempGetCtrlBoardErrorCount( TEMP_MANAGER *TempManager );									  /* <S0DE> */
USHORT DetTempGetPowerBoardErrorCount( TEMP_MANAGER *TempManager );									  /* <S0DE> */


#else /* (CSW_DETECT_TEMPERATURE == TRUE) */

LONG	DetTempInitialize( TEMP_MANAGER *TempManager ){ };
LONG	DetTempManager( TEMP_MANAGER *TempManager ){ };
SHORT DetTempGetInternalTemperature( TEMP_MANAGER *TempManager ){ return 0 };
SHORT DetTempGetCtrlBoardTemperature( TEMP_MANAGER *TempManager ){ return 0 };
SHORT DetTempGetPowerBoardTemperature( TEMP_MANAGER *TempManager ){ return 0 };
USHORT DetTempGetCtrlBoardErrorCount( TEMP_MANAGER *TempManager ){ return 0 };						  /* <S0DE> */
USHORT DetTempGetPowerBoardErrorCount( TEMP_MANAGER *TempManager ){ return 0 };						  /* <S0DE> */
#endif /* (CSW_DETECT_TEMPERATURE == TRUE) */



#define __DETECT_TEMPERATURE_
#endif /* __DETECT_TEMPERATURE_ */
/***************************************** end of file **********************************************/

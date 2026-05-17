/****************************************************************************************************/
/*																									*/
/*																									*/
/*		DetectTemp.c : Temperature Detect IC Access Driver											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Machine Efficiency : I2C/Temperature IC Device Driver for S-5851AAA-M6T1									*/
/*																									*/
/*																									*/
/*		Global Functions :																			*/
/*			DetTempInitialize()				: Initialize the temperature detect IC					*/
/*			DetTempManager()				: Store temperature data in buffer						*/
/*			DetTempReadTemperature()		: Read 16bit temperature data							*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Author   : Yaskawa Electric Corp.,															*/
/*					Copyright (c) 2013 All Rights Reserved											*/
/*																									*/
/*		Project  : Mercury																			*/
/*																									*/
/*		Rev.1.00 : 2013.09.11  Y.Oka																*/
/*		Rev.2.00 : 2013.12.18  Y.Oka <S097>															*/
/*																									*/
/****************************************************************************************************/



/****************************************************************************************************/
/*																									*/
/*		Temperature Detect Parts Specification														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Temperature sensorICRead/write data ( S-5851AAA-M6T1 )										*/
/*																									*/
/*		1) To send Device address																	*/
/*					   7  6  5  4  3  2  1  0														*/
/*			-------------------------------------													*/
/*			I2CDATA	   1  0  0  1 A2 A1 A0 R/W														*/
/*			EPRCNT	   0x00DC (9clock)																*/
/*			-------------------------------------													*/
/*			R/W : 0 = Write, 1 = Read																*/
/*			A2~A0 : 000 = Control Board															*/
/*			A2~A0 : 010 = Power Board																*/
/*																									*/
/*																									*/
/*		2) To write data to Pointer-Register														*/
/*					   7  6  5  4  3  2  1  0														*/
/*			-------------------------------------													*/
/*			I2CDATA	   0  0  0  0  0  0  0 P0														*/
/*			EPRCNT	   0x00DC (9clock)																*/
/*			-------------------------------------													*/
/*			P0 : 0 = Temperature-Register															*/
/*				 1 = Configuration-Register															*/
/*																									*/
/*																									*/
/*		3) To write data to Configuration-Register													*/
/*					   7  6  5  4  3  2  1  0														*/
/*			-------------------------------------													*/
/*			I2CDATA	  OS  *  *  *  *  *  * SD														*/
/*			EPRCNT	   0x00DC (9clock)																*/
/*			-------------------------------------													*/
/*			OS : 1 = Shutdown mode																	*/
/*			SD : 1 = One-Shot mode																	*/
/*																									*/
/*																									*/
/*		4) Set Read Command to I2CDATA,																*/
/*					 F  E  D  C  B  A  9  8  7  6  5  4  3  2  1  0									*/
/*			--------------------------------------------------------								*/
/*			I2CDI0-7 1  1  1  1 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0  0  ACK								*/
/*			EPRCNT   0x008A	(29clock)																*/
/*			--------------------------------------------------------								*/
/*																									*/
/****************************************************************************************************/
#include "DetectTemperature.h"
#include "I2cDriver.h"
#include "KLib.h"
#include "MLib.h"

/****************************************************************************************************/
/*		Static Functions																			*/
/****************************************************************************************************/
static LONG detTempSetConfigration( LONG device, ULONG eaddr, UCHAR data );
static LONG	detTempReadTemperature( LONG device, ULONG eaddr, SHORT *rdVal );



/****************************************************************************************************/
/*																									*/
/*		Initialize the Temperature Detect IC														*/
/*																									*/
/****************************************************************************************************/
LONG	DetTempInitialize( TEMP_MANAGER *TempManager )
{
	LONG		device;						/* Device address (A2,A1,A0) of Temperature Sensor IC	*/
	ULONG		eaddr;						/* Pointer-Register address of Temperature Sensor IC	*/
	UCHAR		data;						/* 8 bit data to be written								*/
	LONG		rslt;						/* result												*/

	rslt = I2C_DRV_SUCCESS;

	/* Clear the Temperature Management Variables */
	MlibResetLongMemory( &TempManager->var, sizeof(TempManager->var)/4 );
	/* First Access Device Setting */
	TempManager->var.DeviceID = CTRL_BOARD_TEMPIC;
	/* Initialize Detect Temperature Timing Timer */
	KlibRstLongTimer( &TempManager->var.DetTempTimer );

	/* Set Pointer-Register */
	eaddr = KPX_I2CTMP_CONFIG_REG;

	/* write data to IC1 Configuration-Register */
	device = KPX_I2CTMPC_DEV_ADR;						/* Device Address							*/
	data = 0x0000;										/* OS=0FF SD=OFF							*/
	rslt = detTempSetConfigration( device, eaddr, data );
	if( rslt != I2C_DRV_SUCCESS )
	{
		return	rslt;
	}

	/* write data to IC2 Configuration-Register */
	device = KPX_I2CTMPP_DEV_ADR;
	data = 0x0000;										/* OS=0FF SD=OFF							*/
	rslt = detTempSetConfigration( device, eaddr, data );
	if( rslt != I2C_DRV_SUCCESS )
	{
		return	rslt;
	}

	return	rslt;

}



/****************************************************************************************************/
/*																									*/
/*		Get Temperature Information Main Operation									<S03A>			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Parameters :																				*/
/*			TEMP_MANAGER	*TempManager : Pointer to the Detect Temperature Variables				*/
/*																									*/
/*		Return :																					*/
/*			Error																					*/
/*																									*/
/****************************************************************************************************/
LONG	DetTempManager( TEMP_MANAGER *TempManager )
{

	LONG 		device;		/* Device address (A2,A1,A0) of Temperature Sensor IC					*/
	ULONG		eaddr;		/* Pointer-Register address of Temperature Sensor IC					*/
	LONG		rslt;		/* result																*/
	SHORT		*data;		/* read data															*/
	SHORT		*max_data;	/* max data																*/
	USHORT		*err_count;	/* I2C error counter													*/

	/* Initial value setting */
	device = KPX_I2CTMPC_DEV_ADR;
	eaddr = KPX_I2CTMP_TEMP_REG;
	data = &TempManager->var.CtrlBoardTemp;
	max_data = &TempManager->var.CtrlBoardTempMax;

	/* Check Detect Timing */
	if( DETECT_TEMP_TIMING < KlibGetLongTimerMs(TempManager->var.DetTempTimer) )
	{
		/* Initialize Detect Temperature Timing Timer */
		KlibRstLongTimer( &TempManager->var.DetTempTimer );

		/* Select Temperature IC */
		if( TempManager->var.DeviceID == CTRL_BOARD_TEMPIC )
		{
			/* Set Local Variables */
			device = KPX_I2CTMPC_DEV_ADR;
			eaddr = KPX_I2CTMP_TEMP_REG;
			data = &TempManager->var.CtrlBoardTemp;
			max_data = &TempManager->var.CtrlBoardTempMax;
			err_count = &TempManager->var.CtrlBoardI2CErrCnt;
			TempManager->var.DeviceID = POWER_BOARD_TEMPIC;
		}
		else if( TempManager->var.DeviceID == POWER_BOARD_TEMPIC )
		{
			device = KPX_I2CTMPP_DEV_ADR;
			eaddr = KPX_I2CTMP_TEMP_REG;
			data = &TempManager->var.PowerBoardTemp;
			max_data = &TempManager->var.PowerBoardTempMax;
			err_count = &TempManager->var.PowerBoardI2CErrCnt;
			TempManager->var.DeviceID = CTRL_BOARD_TEMPIC;
		}
		else
		{ /* Abnormal */
			/* Set Local Variables */
			TempManager->var.DeviceID = CTRL_BOARD_TEMPIC;
		}

		/* Get Current Temperature */
		rslt = detTempReadTemperature( device, eaddr, data );
		if( rslt != I2C_DRV_SUCCESS ) /* <S0DE> */
		{ /* When an error occurs */
			(*err_count)++;
		}
		else
		{ /* Normal state */
			*err_count = 0;
		}

		/* Save maximum value */
		if( *max_data < *data )
		{ /* If the current value is greater than the maximum value, update the maximum value */
			*max_data = *data;
		}
	}
	return	rslt;
}



#define		TEMP_IC_GAIN	0.625f		/* Temperature[0.1℃]Conversion coefficient *//* <S050> */
/****************************************************************************************************/
/*																									*/
/*		Read 16bit temperature information from temperature Detect IC								*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Parameters :																				*/
/*			ASIC_HW_IF_STR	*pAsicHwReg	: Pointer to the ASIC hardware register						*/
/*			LONG			device		: Device address (A2,A1,A0) of Temperature Sensor IC		*/
/*			ULONG			eaddr		: Pointer-Register address of Temperature Sensor IC			*/
/*			USHORT			*rdVal		: Pointer to the buffer for reading							*/
/*																									*/
/*		Return :																					*/
/*			Error																					*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Temperature Info.																			*/
/*			Temperature = 2^Data1(T11-T4) + 2^(-Data2(T3-T0))										*/
/*																									*/
/****************************************************************************************************/
static LONG	detTempReadTemperature( LONG device, ULONG eaddr, SHORT *rdVal )
{
	LONG			i;
	ULONG			time;
	ULONG			NackCnt;
	UCHAR			rdData1, rdData2;

/*--------------------------------------------------------------------------------------------------*/
/*		I2CCheck bus status																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_STOP( );									/* Prepare Write Operation					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Stop Condition End																		*/
/*--------------------------------------------------------------------------------------------------*/
	KlibRstLongTimer( &time );
	do {
		if( KlibGetLongTimerMs(time) > I2C_DRV_TIMEOUT )
		{
			return I2C_TIMEOUT_ERR;
		}
	} while( ! KPX_I2C_STOP_END_CHK( ) );

/*--------------------------------------------------------------------------------------------------*/
/*		Start condition																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_START( );									/* Start Condition							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Start Condition Finish																	*/
/*--------------------------------------------------------------------------------------------------*/
	KlibRstLongTimer( &time );
	do {
		if( KlibGetLongTimerMs(time) > I2C_DRV_TIMEOUT )
		{
			return I2C_TIMEOUT_ERR;
		}
	} while ( ! KPX_I2C_START_END_CHK( ) );

/*--------------------------------------------------------------------------------------------------*/
/*		Slave address transmission																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_TX( device | KPX_I2C_WRITE_BIT ); 			/* Send Device Address						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Transmit Finish																		*/
/*--------------------------------------------------------------------------------------------------*/
//	do { /* <S0A8> */
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_TX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_TX_END( ) );

/*--------------------------------------------------------------------------------------------------*/
/*		Temperature register specification(Pointer register setting)														*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_TX( eaddr ); 							/* Write Slave Address, data, etc..	*/\
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Transmit Finish																		*/
/*--------------------------------------------------------------------------------------------------*/
//	do { /* <S0A8> */
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_TX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_TX_END( ) );
	KPX_I2C_NackCnt( NackCnt );							/* CountUp NackCnt If Nack					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Start condition																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_START( );									/* Start Condition							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Start Condition Finish																	*/
/*--------------------------------------------------------------------------------------------------*/
	KlibRstLongTimer( &time );
	do {
		if( KlibGetLongTimerMs(time) > I2C_DRV_TIMEOUT )
		{
			return I2C_TIMEOUT_ERR;
		}
	} while ( ! KPX_I2C_START_END_CHK( ) );

/*--------------------------------------------------------------------------------------------------*/
/*		Slave address transmission																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_TX( device | KPX_I2C_READ_BIT ); 			/* Send Device Address						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Transmit Finish																		*/
/*--------------------------------------------------------------------------------------------------*/
//	KlibRstLongTimer( &time ); /* <S0A8> */
//	do {
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_TX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_TX_END( ) );

#if 0 /* <S0A8> */
/*--------------------------------------------------------------------------------------------------*/
/*		Temperature register specification(Pointer register setting)														*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_TX( eaddr ); 								/* Write Slave Address, data, etc..			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Transmit Finish																		*/
/*--------------------------------------------------------------------------------------------------*/
//	KlibRstLongTimer( &time ); /* <S0A8> */
//	do {
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_TX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_TX_END( ) );
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Read data reception																			*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_RX( );										/* Start receiving (1st data)				*/
//	KlibRstLongTimer( &time ); /* <S0A8> */
//	do {
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_RX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_RX_END( ) );
	rdData1 = KPX_I2C_GetRdData;						/* 1st Data Read (4-11bit)					*/

	KPX_I2C_RX( );										/* Start receiving (2nd data)				*/
//	KlibRstLongTimer( &time ); /* <S0A8> */
//	do {
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_RX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_RX_END( ) );
	KPX_I2C_RX_NOACK( );								/* Start receiving (2nd data)				*/
	rdData2 = KPX_I2C_GetRdData;						/* 2nd Data Read (0-3bit & invalid data)	*/

/*--------------------------------------------------------------------------------------------------*/
/*		Temperature data creation(Integer rounding)															*/
/*--------------------------------------------------------------------------------------------------*/
//	*rdVal = TmpData1 + (TmpData2 >> 7);															  /* <S050> */
	*rdVal = ((SHORT)((rdData1 << 8) + rdData2) >> 4) * TEMP_IC_GAIN;								  /* <S050> */

/*--------------------------------------------------------------------------------------------------*/
/*	Stop condition																			*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_STOP( );									/* Stop Condition 							*/

/*--------------------------------------------------------------------------------------------------*/
/*		Check Nack Count																			*/
/*--------------------------------------------------------------------------------------------------*/
;

	return	I2C_DRV_SUCCESS;

}



/****************************************************************************************************/
/*																									*/
/*		Write 8bit Configuration data to tenmpareture Detect IC										*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Parameters :																				*/
/*			ASIC_HW_IF_STR *pAsicHwReg	: Pointer to the ASIC hardware register						*/
/*			LONG 			device		: Device address (A2,A1,A0) of Temperature Sensor IC		*/
/*			ULONG 			eaddr		: Pointer-Register address of Temperature Sensor IC			*/
/*			UCHAR 			data		: 8 bit data to be written									*/
/*																									*/
/*		Return :																					*/
/*			Error																					*/
/*																									*/
/****************************************************************************************************/
static LONG	detTempSetConfigration( LONG device, ULONG eaddr, UCHAR data )
{
	LONG			i;
	ULONG			time;
//	ULONG			poll_cnt;							/* Time Stamp Unit [ms]						*/
	ULONG			NackCnt;

	/* Local variable initialization */
//	poll_cnt = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		I2CCheck bus status																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_STOP( );									/* Prepare Write Operation					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Stop Condition End																		*/
/*--------------------------------------------------------------------------------------------------*/
	KlibRstLongTimer( &time );
	do {
		if( KlibGetLongTimerMs(time) > I2C_DRV_TIMEOUT )
		{
			return I2C_TIMEOUT_ERR;
		}
	} while( ! KPX_I2C_STOP_END_CHK( ) );

/*--------------------------------------------------------------------------------------------------*/
/*		Start condition																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_START( );									/* Start Condition							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Start Condition Finish																	*/
/*--------------------------------------------------------------------------------------------------*/
	KlibRstLongTimer( &time );
	do {
		if( KlibGetLongTimerMs(time) > I2C_DRV_TIMEOUT )
		{
			return I2C_TIMEOUT_ERR;
		}
	} while ( ! KPX_I2C_START_END_CHK( ) );

/*--------------------------------------------------------------------------------------------------*/
/*		Slave address transmission																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_TX( device | KPX_I2C_WRITE_BIT ); 			/* Send Device Address						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Transmit Finish																		*/
/*--------------------------------------------------------------------------------------------------*/
//	KlibRstLongTimer( &time ); /* <S0A8> */
//	do {
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_TX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_TX_END( ) );

/*--------------------------------------------------------------------------------------------------*/
/*		Configuration register specification(Pointer register setting)										*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_TX( eaddr ); 								/* Write Slave Address, data, etc..			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Transmit Finish																		*/
/*--------------------------------------------------------------------------------------------------*/
//	KlibRstLongTimer( &time ); /* <S0A8> */
//	do {
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_TX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_TX_END( ) );
	KPX_I2C_NackCnt( NackCnt );							/* CountUp NackCnt If Nack					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Write data transmission																			*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_TX( data );									/* Write Data Low to TxBuf					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Wait Transmit Finish																		*/
/*--------------------------------------------------------------------------------------------------*/
//	KlibRstLongTimer( &time ); /* <S0A8> */
//	do {
	for( i=0; i < 50; i++ )
	{
		KlibWaitus( 1 );										/* Wait 1 us						*/
		if( KPX_I2C_CHK_TX_END() )
		{
			break;
		}
	} //while( ! KPX_I2C_CHK_TX_END( ) );
	KPX_I2C_NackCnt( NackCnt );							/* CountUp NackCnt If Nack					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Stop condition																		*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_I2C_STOP( );									/* Stop Condition 							*/

/*--------------------------------------------------------------------------------------------------*/
/*		Check Nack Count																			*/
/*--------------------------------------------------------------------------------------------------*/
	;

	/* Writing timeWAIT */
	KlibWaitms( 5 );

	return I2C_DRV_SUCCESS;
}



/****************************************************************************************************/
/*																									*/
/*		Get Current Internal Temperature											<S03A>			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Returns the internal temperature. MultipleIC, the highest temperature[0.1℃].				*/
/*																									*/
/****************************************************************************************************/
SHORT DetTempGetInternalTemperature( TEMP_MANAGER *TempManager )
{

	return	(MlibMAX(TempManager->var.CtrlBoardTemp, TempManager->var.PowerBoardTemp));
}



/****************************************************************************************************/
/*																									*/
/*		Get Current Control Board Temperature										<S03A>			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Temperature mounted on control boardICdetection temperature[0.1℃]												*/
/*																									*/
/****************************************************************************************************/
SHORT DetTempGetCtrlBoardTemperature( TEMP_MANAGER *TempManager )
{

	return	(TempManager->var.CtrlBoardTemp);
}



/****************************************************************************************************/
/*																									*/
/*		Get Current Power Board Temperature											<S03A>			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Temperature mounted on power boardICdetection temperature[0.1℃]											*/
/*																									*/
/****************************************************************************************************/
SHORT DetTempGetPowerBoardTemperature( TEMP_MANAGER *TempManager )
{

	return	(TempManager->var.PowerBoardTemp);
}



/****************************************************************************************************/
/*																									*/
/*		Get Current Control Board I2C Error Count									<S0DE>			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Temperature mounted on control boardICdetection temperature[0.1℃]												*/
/*																									*/
/****************************************************************************************************/
USHORT DetTempGetCtrlBoardErrorCount( TEMP_MANAGER *TempManager )
{

	return	(TempManager->var.CtrlBoardI2CErrCnt);
}



/****************************************************************************************************/
/*																									*/
/*		Get Current Power Board I2C Error Count										<S0DE>			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Temperature mounted on power boardICdetection temperature[0.1℃]											*/
/*																									*/
/****************************************************************************************************/
USHORT DetTempGetPowerBoardErrorCount( TEMP_MANAGER *TempManager )
{

	return	(TempManager->var.PowerBoardI2CErrCnt);
}

/***************************************** end of file **********************************************/

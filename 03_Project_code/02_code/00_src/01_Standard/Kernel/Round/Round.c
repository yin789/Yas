/****************************************************************************
Description	: Round Task Main module
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

Functions	  :
	SysRoundTask()			:Main routine of the Round Task

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
I.M.   2011.01.24     renewal

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "NetworkIf.h"
#include "AlarmManager.h"
#include "HwDeviceIf.h"
#include "KLib.h"
#include "EepromIF.h"
#include "FunManager.h"
#include "OverheatManager.h"
#include "DetectTemperature.h"	/* <S03A> */

/* --> <USB-FUNCTION> 2013.05.13 K.Sakamoto */
//#ifdef _USB_FUNC_USE_		/*<S008>*//*<S138>*/
//<S146>#include "nbpfahb32vic128.h"
//<S146>#include "usb_f.h"
//#endif	//#ifdef _USB_FUNC_USE_		/*<S008>*//*<S138>*/
/* <-- <USB-FUNCTION> 2013.05.13 K.Sakamoto */

static void RpiRoundMain( AXIS_HANDLE *AxisRscR, int *iniFlg );
static BOOL LpxSoftReset( AXIS_HANDLE *AxisRscR );		/* <S076> */

/* External function */
extern LONG CpiGetScanCCnt( void );						
extern void KpxMsgManager( void );						
extern void ScuSoftReset( AXIS_HANDLE *AxisHandle );	
extern void UsbFuncTaskControl( void );					

#if 0/* Hold */
extern BOOL FTP_FirmwareUpdateMain(void);
#endif
extern void	MbusFuncSystemDownLoadCheckSum( void );	/* Encrypted format support <S146> */

/****************************************************************************
  Description: Main routine of the Round Task
-----------------------------------------------------------------------------
  Parameters	: none
  Return		: none
*****************************************************************************/
void SysRoundTask( void )
{
	AXIS_HANDLE	*AxisRscR;
	int iniFlg = FALSE;
	/*----------------------------------------------------------------------*/
	LONG SCCnt = 0;												/* @@@ */
	LONG wkx;													/* @@@ */
	ULONG wtimer;												/* <S03F> */
	ULONG	ax;													/* <S19B> */
	/*----------------------------------------------------------------------*/

	AxisRscR = (AXIS_HANDLE*)GetAxisHandle(0);

	/********************/
	/*		Loop 		*/
	/********************/
#ifndef _VC_DEBUG_ /* 2013.01.10 Y.Oka */
	for( ; ; )
#endif	//#ifndef _VC_DEBUG_ /* 2013.01.10 Y.Oka */
	{
		/* For processing time measurementGPIO ON */
//		HALdrv_GpioOutput( TRUE 0 );

		/* ROUND */
	/*----------------------------------------------------------------------*/
		KlibRstLongTimer( &wtimer );	/* ScanCstops,RoundCorrected so that processing runs<S03F> */
		while( 1 )
		{ /* @@ wait for ScanC */
			if( SCCnt != (wkx = CpiGetScanCCnt()) )
			{
				SCCnt = wkx; 						/* @@ get scanC cnt		*/
				break;
			}
			else if( 2 < KlibGetLongTimerMs(wtimer) )
			{
				break;
			}
			else
			{
				; //Wait
			}
		}
	/*----------------------------------------------------------------------*/

		/* Round Loop Main Operation */
		RpiRoundMain( AxisRscR, &iniFlg );

		/* For processing time measurementGPIO OFF */
//		HALdrv_GpioOutput( FALSE, 0 );

#ifndef _VC_DEBUG_ /* 2013.05.31 Y.Oka */
		KpxMsgManager( ); 			/* Message Manager <XXX> */

		/* Loop for the number of axes *//* <S19B> */
		for( ax = 0; ax < NumberOfAxes; ax++ )
		{
			AxisRscR = (AXIS_HANDLE*)GetAxisHandle(ax);
			KpxDownloadEeprom( AxisRscR->pRwAllMng );		/* <S00C> Download Eeprom  */
			/* <S031> Lcd Parameter Copy *//* <S098> */
			if( I2C_DRV_SUCCESS != KpxLcdPdlEeprom(AxisRscR->pRwPdlMng) )
			{ /* A.BF6 : System alarm 6(I2CDriver error) */
				ALMSetServoAlarm( AxisRscR->AlmManager, ALM_I2C_DRVERR );
			}
		}
		/* 0axis */
		AxisRscR = (AXIS_HANDLE*)GetAxisHandle(0);/* <S19B> */
#endif	//#ifndef _VC_DEBUG_ /* 2013.05.31 Y.Oka */

		/* Stack area check *//*<S127>*/
		if( 0xFFFFFFFF != *(VULONG*)(0x67FAF61C) )
		{/* A.BF1 : System alarm 1(Internal program operation error)			*/
			ALMSetGlobalAlarm( ALM_STACK_OVF );
		}

		MbusFuncSystemDownLoadCheckSum();	/* Encrypted format support <S146> */

		/* Soft reset <S076> */
		AxisRscR->BeSequence->SoftResetReq = LpxSoftReset( AxisRscR );

		KlibWaitus(500);	/* 0.5ms */

		/* <S0CB> Start */
			/* ScanCCount acquisition : RoundProcessing always scansC. */
			SCCnt = CpiGetScanCCnt();
		/* <S0CB> End */

	}
}

/****************************************************************************
  Description: Round Main Procedure
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscR	: Pointer to Axis Handle
  Return		:
		none
*****************************************************************************/
static void RpiRoundMain( AXIS_HANDLE *AxisRscR, int *iniFlg )
{
	LONG			ax_no;
	LONG			timer;
	ULONG			timeStamp, rdTime;
	AXIS_HANDLE		*AxisRsc;
	ANOTCHSEQCTRL	ANotchCtrl;
	ALMMAN_HANDLE	*AlmManager;

#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE
	/* Network moduleRoundProcessing */
	if(FALSE == OpiRoundProcedure( AxisRscR ))
	{
#if 0/* Hold */
		/* write downloaded firmware via FTP */
		FTP_FirmwareUpdateMain();
#endif
		PnlOpSetConnectStatus( AxisRscR->PnlOpLedDataHndl, FALSE );			  /* <S0A1> */
	}
	else
	{
		PnlOpSetConnectStatus( AxisRscR->PnlOpLedDataHndl, TRUE );			  /* <S0A1> */
	}
#endif

/* --> <USB-FUNCTION> 2013.05.18 K.Sakamoto */
//#ifdef _VC_DEBUG_
/* <-- <USB-FUNCTION> 2013.05.18 K.Sakamoto */
	/* Network message communication management	*/
//<S150>	MsgIfBackgroundProdedure(AxisRscR->NetMsgIf);
/* <S150> >>>>> */
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
	M3MsgIfManager();
#endif
/* <<<<< <S150> */
/* --> <USB-FUNCTION> 2013.05.18 K.Sakamoto */
//#endif
/* <-- <USB-FUNCTION> 2013.05.18 K.Sakamoto */


/* --> <USB-FUNCTION> 2013.05.13 K.Sakamoto */
//#ifdef _USB_FUNC_USE_/*<S138>*/
	UsbFuncTaskControl();
//#endif	//#ifdef _USB_FUNC_USE_		/*<S008>*//*<S138>*/
	//	u2f_task_control();
//	u2f_task_msc();
/* <-- <USB-FUNCTION> 2013.05.13 K.Sakamoto */

	/* ROUND Each Axis Operation */
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		AxisRsc = &AxisRscR[ax_no];
		/* FnFunction execution processing */
		FunRunTimeService(AxisRsc->FnManager, FUN_CH1);
		FunRunTimeService(AxisRsc->FnManager, FUN_CH2);
//		if ( !(AxisRsc->BeSequence->ServoReady) ){ AxisRsc->FnCmnCtrl->FnSvonReq = FALSE; } /* <S0B4> */
		if( AxisRsc->MencV->PhaseReady == TRUE )		/* <S21C> Phase detection completed */
		{
			if ( !(AxisRsc->BeSequence->ServoReady) ){ AxisRsc->FnCmnCtrl->FnSvonReq = FALSE; } /* <S0B4> */
		}												/* <S21C> */
		
		/* Automatic notch filter setting sequence call pointer setting */
		ANotchCtrl.ANotchSeq = AxisRsc->ANotchSeq;
		ANotchCtrl.ANotch = AxisRsc->ANotch;
		ANotchCtrl.FftAna = AxisRsc->FftAna;
		ANotchCtrl.DetVib = AxisRsc->DetVib;
		ANotchCtrl.Prm = AxisRsc->UniPrms->Prm;
		ANotchCtrl.RegCfg = AxisRsc->RegManager->hPnReg;
		/* Data setting processing for auto notch filter setting */
		AutoNotchSetCondition( ANotchCtrl.ANotchSeq,
							   AxisRsc->BaseControls->TuneLessCtrl.var.TuningFuncRun,
							   AxisRsc->SeqCtrlOut->BaseEnableReq,
							   AxisRsc->BaseControls->TuneLessCtrl.var.TuneLessAct,
							   AxisRsc->SeqCtrlOut->OverTrvlSts );
		/* Automatic notch filter setting sequence execution process */
		AutoNotchFilterSetting( &ANotchCtrl );

		/* ABSScale magnetic pole detection result write sequence process */
		if(FALSE != PdetWriteMotorPhaseSequence(AxisRsc->PdetAbsSeq,
												AxisRsc->MencV,
												AxisRsc->SvAsicRegs->AsicHwReg,
												AxisRsc->RegManager->hPnReg))
		{
			ALMSetServoAlarm( AxisRsc->AlmManager, ALM_PGCOM );	/* A.C90: Encoder communication error */
		}
		/* DBWrite parameter for relay operation countTOEeprom */
		LifeTimeDBPrmStoreValueToEeprom( AxisRsc->hDBLifeTime,
										AxisRsc->RegManager->hPnReg,
										AxisRsc->UniPrms->Prm );			/* <S192> */
	}

	/* Writing parameters for the number of thrust protection relay operationsTOEeprom */
	LifeTimeRushPrmStoreValueToEeprom( &AxisRscR->hLifeTime->RushLifeTimeHdl,
										AxisRscR->RegManager->hPnReg,
										AxisRscR->UniPrms->Cprm );			/* <S192> */

	/* Alarm Manager : EEPROMHistory writing */
	timer = KlibGetMainPowerOffTimer();
	if((timer <= 0) || (timer >= KLIB_EEPDISABLE_TIME))
	{/* Main Power offto100ms */
		for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
		{
			AxisRsc = &AxisRscR[ax_no];
			ALMHistoryWriteService(AxisRsc->AlmManager);
		}
	}
	else
	{
		AlmManager = AxisRscR->AlmManager;
		/* Main Power offinTime StampWrite */
		rdTime = timeStamp = KlibGetTimeStamp();
		EepdevReadValues( 0,
						  //EEP_NOSUM_ADDRESS(0, EEP_TIMESTAMP_OFS),					/* <S04E>:DELL */
						  EEP_NOSUMCMN_ADDRESS(EEP_TIMESTAMP_OFS),                      /* <S04E>:ADD */
						  (USHORT*)&rdTime, 2 );
		if(timeStamp != rdTime)
		{
			EepdevWriteNoSumValues( 0,
									//EEP_NOSUM_ADDRESS(0, EEP_TIMESTAMP_OFS),			/* <S04E>:DELL */
									EEP_NOSUMCMN_ADDRESS(EEP_TIMESTAMP_OFS),            /* <S04E>:ADD */
									(USHORT*)&timeStamp,
									2 );
		}
	}

#if	(CSW_DETECT_TEMPERATURE == TRUE) /* <S03A> */
	/* Servopack temperatureICManagement */
	DetTempManager( AxisRscR->TempManager );
#endif

/* 2013/04/01MDversion,RndReadydoes not exist. TemporarilySEQ_CTRL_OUTdefine to	*/
	if(*iniFlg == FALSE)
	{/* Round ready setting processing */
		for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
		{
			AxisRsc = &AxisRscR[ax_no];
			AxisRsc->SeqCtrlOut->RndReady = TRUE;
		}
		*iniFlg = TRUE;
	}
}


/****************************************************************************
  Description: Round SoftReset Procedure		<S076>
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscR	: Pointer to Axis Handle
  Return		:
		none
*****************************************************************************/
static BOOL LpxSoftReset( AXIS_HANDLE *AxisRscR )
{
	BOOL	resetReq;

	resetReq = AxisRscR->BeSequence->SoftResetReq;
	
	if( TRUE == resetReq )
	{/* Can be reset only when "servo is not on" & "motor is not rotating"*/
		ScuSoftReset( AxisRscR );
		resetReq = FALSE;		/* */
	}

	return( resetReq );
}



/**************************** end of file **********************************/

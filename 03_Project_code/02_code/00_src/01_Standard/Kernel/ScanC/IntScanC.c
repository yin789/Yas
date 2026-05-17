/****************************************************************************
Description	: ScanC Task module
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

Functions	  :
	SysScanCTask()			:Main routine of the Scan C Task

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
I.M.   2011.01.24     renewal

****************************************************************************/
//#include <INTEGRITY.h>
//#include "Basedef.h"
#include "KnlApi.h"
#include "MercuryGlobals.h"
#include "SequenceMain.h"
#include "CheckAlarm.h"
#include "SequenceIo.h"
#include "PowerManager.h"
#include "AlarmManager.h"
#include "KLib.h"
#include "MLib.h"
#include "HwDeviceIf.h"
LONG TEST_SCANC_CNT;

/* Prototype */
void SysScanCInt( void );

extern LONG	NumberOfAxes;

#if defined(_TEST_BOARD_) && !defined(_VC_DEBUG_)
void IntScanC( void ); /* Provisional measures */
extern void ClearSeqCtrlInterrupt( void );
extern void	SetEnableInterrupt( void );

/****************************************************************************
  Description: Interrupt Operation for Sequence Scan Task
-----------------------------------------------------------------------------
  Parameters	: none
  Return		: none
*****************************************************************************/
void IntScanC( void )
{
	ULONG		clock_time, sbTime;
	UINT		netled;
	AXIS_HANDLE	*AxisRscC;

	TEST_SCANC_CNT++;

	AxisRscC = GetAxisHandle(0);

	/* For processing time measurementGPIO ON */;
#if CSW_NETIF_TYPE	!= NETIF_ANLGPLS_TYPE
	HALdrv_OutputDebugSP( INTERRUPT_ScanC );
#endif
#if 0
	clock_time = KlibGetClockTime();
#endif
	sbTime = AxisRscC->TaskProcessTime->ScanSumTime;

	KPI_EI (); 							//@@	SetEnableInterrupt( );
	ClearSeqCtrlInterrupt( );

//	if( ScanCStartCtr < 0x0F )
//	{
//		ScanCStartCtr++;
//	}
//	else
//	{
		SysScanCInt( );
//	}

	/* For processing time measurementGPIO OFF */
#if 0
	clock_time = KlibGetClockTime() - clock_time;
	if(clock_time > AxisRscC->TaskProcessTime->SC_maxTime)
	{
		AxisRscC->TaskProcessTime->SC_maxTime = clock_time;
	}
	AxisRscC->TaskProcessTime->SC_crtTime = clock_time;
	AxisRscC->TaskProcessTime->ScanSumTime = clock_time + sbTime;
#endif
	HALdrv_ClearDebugSP( INTERRUPT_ScanC );
}
#endif



/****************************************************************************
  Description: Scan C Main Procedure
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscC	: Pointer to Axis Handle
  Return		:
		none
*****************************************************************************/
void SysScanCInt( void )
{
	LONG				ax_no;
	HWSIN				SeqInputSignals;
	LONG				AlmWrnSts;
	USHORT				FaultState;
	BOOL				BaseEnableSts;
	BOOL				DBRequest;
	BOOL				MotStopSts;
	AXIS_HANDLE			*AxisRscC;
	AXIS_HANDLE			*AxisRsc;
	POW_MNG_HNDL		*PowerManager;
//	SEQ_IOSIGNAL		*SeqIo;				/*<S0C7>*/
	VULONG				ScanCStartTime;		/*<S043>*/
	TASK_MEASURE_TIME	*TaskProcessTime;	/*<S043>*/
	/* <S154> Start */
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
	AVGTMPR_HANDLE	*hAvgTmprHdl;
	ULONG 			CurrentTimerCnt;	/* Current operating time	*/
	SHORT 			CurrentTemperature;
	LONG			PreWrnSts;
#endif
	/* <S154> End */
	ASICS				*svAsicRegs;		/*<S126>*/
	volatile ULONG		SABinC_exeTime;		/*<S1CD>*/
	BASE_CTRL_OUT		*BaseCtrlOut;		/* <S21D> */
	SEQ_IO_HNDL			*IoSignals;			/* <S21D> */

	/* For processing time measurementGPIO ON */;
#if CSW_NETIF_TYPE	!= NETIF_ANLGPLS_TYPE
//	HALdrv_OutputDebugSP( INTERRUPT_ScanC );		/* Change to port output in interrupt branch processing<S042> */
#endif
	KlibRstLongTimer((ULONG*)&ScanCStartTime);		/*<S043>*//*<S0C7>*/
	/*----------------------------------------------------------------------*/
	TEST_SCANC_CNT++;			/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 	*/
	/*----------------------------------------------------------------------*/

	AxisRscC = GetAxisHandle(0);
	PowerManager = AxisRscC->PowerManager;
	TaskProcessTime = AxisRscC->TaskProcessTime;		/*<S043>*/
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
	hAvgTmprHdl = &(AxisRscC->hLifeTime->AvgTmprHdl);	/* <S154> */
#endif
	BaseCtrlOut = AxisRscC->BaseCtrlOut;		/* <S21D> */
	IoSignals = AxisRscC->IoSignals;			/* <S21D> */

	/* ScanCExecution flag set <S043> */
	TaskProcessTime->SC_exeflg = TRUE;

	/*----------------------------------------------------------------------*/
	/*	Previous2ms				<S043>						*/
	/*----------------------------------------------------------------------*/
	TaskProcessTime->crtTimePer2ms = TaskProcessTime->SA_sumTime
										+TaskProcessTime->SB_sumTime
										+TaskProcessTime->SC_crtTime;
	TaskProcessTime->SA_sumTime = 0;	/* Cumulative processing time reset */
	TaskProcessTime->SB_sumTime = 0;	/* Cumulative processing time reset */
	if( TaskProcessTime->crtTimePer2ms > TaskProcessTime->maxTimePer2ms )
	{
		TaskProcessTime->maxTimePer2ms = TaskProcessTime->crtTimePer2ms;
	}


	/* Scan count1, perform an alarm check when the network is stable. */ /*<S1DB>*/
	if( (TEST_SCANC_CNT > 1) && (OpiCheckStableState( AxisRscC ) == FALSE) )
	{
/* <S1CD> >>>>> */
		/* Scan C cycle time over check(Compare by count value) */
		if( TaskProcessTime->crtTimePer2ms > ((KPI_SCCYCLEMS * 1000000) / ITMR_NSEC_PER_COUNT) )
		{
			/* A.BF0 : System alarm 0(Internal program processing error) */
			ALMSetGlobalAlarm( ALM_SAMP_OVER );
		}
/* <<<<< <S1CD> */
	}
	else
	{
		TaskProcessTime->maxTimePer2ms = 0;		/* 2msReset maximum processing time <S1F3> */
		OpiResetSyncExe( AxisRscC );
	}

	/*----------------------------------------------------------------------*/
	/*	Common to unitsScanCInput processing											*/
	/*----------------------------------------------------------------------*/
	SeqInputSignals = HALdrv_ReadInputSignals( );
	PcmInputPowerStatus( PowerManager,
						 SeqInputSignals.flag.Acon1,
						 SeqInputSignals.flag.RegeneAlm );
	/*----------------------------------------------------------------------*/
#if (SVFSEL_ADC_USE == 1)||(SVFSEL_ONLY_OH_ADC_USE == 1)		/* <S09F> *//* <S0E1>:SVFSEL_ONLY_OH_ADC_USEaddition */
	KpxAdcScanCService( AxisRscC->pAdc );		/* A/D converter ScanC service */
#endif                     /* <S09F> */
	KpxPlsScanCService( AxisRscC->pPlsIfV );	/* Command Pulse counter ScanC service */
	/*----------------------------------------------------------------------*/
	SeqIoInputSeqSignalsCom( IoSignals, AxisRscC->UniPrms->Prm );		/* <S21D> */

	/* < S154> Start  */
	/*----------------------------------------------------------------------*/
	/*		Average internal temperature calculation process 											*/
	/*----------------------------------------------------------------------*/
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
	CurrentTimerCnt = KlibGetTimeStamp();		/* Getting current operating time */
	/* From the cumulative operating time count30minutes, consider14bitRight shift is used.	*/
	/* The unit of cumulative operating time is[100ms], so it is assumed that it is						*/
	/* 30minutes18000Count≒16384 = 2^14.				*/
	if ( ((CurrentTimerCnt - hAvgTmprHdl->var.Timer100msCntLast) >> 14) > 0 )
	{	/*30 Divide period processing*/
	
		/* Get current maximum temperature  */
		CurrentTemperature = DetTempGetInternalTemperature(AxisRscC->TempManager);

		/*	Average internal temperature calculation process 	*/
//		AvgInternalTemperatureCal(AxisRscC->AlmManager->EepDevice, hAvgTmprHdl,		/* <S192> */
//								(CurrentTemperature), CurrentTimerCnt );
		AvgInternalTemperatureCal( &AxisRscC->AlmManager->EepQue, hAvgTmprHdl,		/* <S192> */
								(CurrentTemperature), CurrentTimerCnt );
	}
#endif
	/* < S154 > End  */

	/*----------------------------------------------------------------------*/
	/* Kernel Input  Service for ScanC */
	/*----------------------------------------------------------------------*/
	AxisRsc = AxisRscC;
//	FaultState = DBRequest = MotStopSts = BaseEnableSts = 0; /* <S0A3> *//* <S1D5> */
	FaultState = DBRequest = BaseEnableSts = 0;	/* <S1D5> */
	MotStopSts = 1;	/* <S1D5> */

	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		SyscInputSeqStatus( AxisRsc );

		/* @@Things to reconsider when supporting multi-axis 2013.07.14 Y.Oka (>_<) */
		/*------------------------------------------------------------------*/
		/* Acquisition of each axis data for unit processing */
		/*------------------------------------------------------------------*/
		FaultState |= AxisRsc->CheckAlarm->faultState;
//		BaseEnableSts |= AxisRscC->SeqCtrlOut->BeComplete;					  /* <S0A3> *//* <S0F6> */
//		BaseEnableSts |= AxisRscC->SeqCtrlOut->UdlBeStatus;					  /* <S0F6> */
		BaseEnableSts |= AxisRsc->SeqCtrlOut->UdlBeStatus;					  /* <S131> */
		DBRequest |= AxisRsc->BeSequence->DbOn;
//		MotStopSts |= AxisRscC->SeqCtrlOut->MotStop;
//		MotStopSts |= AxisRsc->SeqCtrlOut->MotStop;							  /* <S131> *//* <S1D5> */
		MotStopSts &= AxisRsc->SeqCtrlOut->MotStop;							  /* <S1D5> */

		AxisRsc++;
	}

#ifdef CSW_BSP_SGDZ_IFBS80
	/*----------------------------------------------------------------------*/
	/* Over heat alarm detection to every board */
	/*----------------------------------------------------------------------*/
	AxisRsc = AxisRscC;
	faultState1 = faultState2 = 0;
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		if((OH_ALM_SLOT_GRP1 >> AxisRsc->AxisNo) & 1)
		{
			faultState1 |= AxisRsc->SeqCtrlData->faultState;
			if(0 != (SHAL_FAULT_OH & faultState1))
			{/* A.7A0 : Heat sink overheating				*/
				ALMSetServoAlarm( AxisRsc->AlmManager, ALM_OH );
			}
		}
		if((OH_ALM_SLOT_GRP2 >> AxisRsc->AxisNo) & 1)
		{
			faultState2 |= AxisRsc->SeqCtrlData->faultState;
			if(0 != (SHAL_FAULT_OH & faultState2))
			{/* A.7A0 : Heat sink overheating				*/
				ALMSetServoAlarm( AxisRsc->AlmManager, ALM_OH );
			}
		}
		AxisRsc++;
	}
#endif	//CSW_BSP_SGDZ_IFBS80

	/*----------------------------------------------------------------------*/
	/* Power management process execution(Unit unit) */
	/*----------------------------------------------------------------------*/
#if (CSW_HW_POWER_MANAGE == TRUE)
	/* <S21D> Start */
	/*----------------------------------------------------------------------*/
	/*		Read sequence input signal											*/
	/*----------------------------------------------------------------------*/
	PowerManager->PowerSts.ClinkAnsSig =
			IoSignals->ClinkansSignal.SignalStatus.v;				/* Capacitor bank connection monitoring input signal	*/
	PowerManager->PowerSts.CmcAnsSig =
			IoSignals->CmcansSignal.SignalStatus.v;					/* Rush protection relay status monitoring input signal		*/
	PowerManager->PowerSts.ConvRdySig =
			IoSignals->ConvreadySignal.SignalStatus.v;				/* Converter ready input signal			*/
	PowerManager->PowerSts.ConvMconSig =
			IoSignals->ConvmconSignal.SignalStatus.v;				/* Inrush current prevention relay input signal		*/
	/* <S21D> End */
//	PcmPowerManager( PowerManager, MotStopSts );							  /* <S0F6> */
	PcmPowerManager( PowerManager, MotStopSts, BaseEnableSts );				  /* <S0F6S0F6> */
#endif

	/* Execute regenerative overload detection processing(Unit unit) */
#if (CSW_HW_REGENERATION_CTRL == TRUE)
	/*----------------------------------------------------------------------*/
	/*	Delta Regene.Tr On Count for ScanC Alarm Check						*/
	/*----------------------------------------------------------------------*/
	/* Regeneration transistorONTime measurement */
	PcmCountRegeneOnTime( PowerManager );

	/* Regenerative overload check */
#if 0		/* <S21D> */
	if( (PowerManager->MainPowerChecker.conf.Dcbus == FALSE)
		&& (PowerManager->MainPowerChecker.conf.TestMode2 == FALSE) )
	{ /* ACpower input && !TestMode2 */
#else		/* <S21D> */
	if( ( (PowerManager->MainPowerChecker.conf.Dcbus == FALSE)			/* ACpower input */
	   || ( (PowerManager->MainPowerChecker.conf.Dcbus == TRUE)			/* DCpower input */
		 && (PowerManager->RegeneCtrl.conf.RegSyn == PNREGSYN) ) )		/* PNUse regeneration synchronization function in bus common system */
	 && (PowerManager->MainPowerChecker.conf.TestMode2 == FALSE) )		/* !TestMode2 */
	{
#endif
		if( ALMCheckGlobalAlarm(WRN_EXREG) == FALSE )					/* A.924(Regeneration warning) None *//* <S21D> */
		{
			ULONG	AlmOLInfo = (AxisRscC->AlmManager)->Status.AlmOLInfo; 		/* <S0FF> */
			AlmWrnSts = PcmCheckRegeneOverLoad( &(PowerManager->RegeneCtrl),
											   &AlmOLInfo, 						/* <S0FF> */
											   AxisRscC->UnMonitor);
			if( AlmWrnSts != CHK_REGOL_NO_OPERATION )
			{
				/* Regenerative overload alarm/warning setting */
				if(0 != (AlmWrnSts & CHK_REGOL_SET_ALARM))
				{	/* A.320 : Regeneration overload */
					ALMSetGlobalAlarm( ALM_RGOL );
				}
				else if(0 != (AlmWrnSts & CHK_REGOL_SET_WARNING))
				{	/* A.920 : Regenerative overload warning */
					ALMSetGlobalAlarm( WRN_RGOLF );
				}
				else
				{	/* A.920 : Regenerative overload warning */
					ALMClearGlobalAlarm( WRN_RGOLF );
				}
			}
		}
	}
#endif

#if 0 /* Move to each axis processing *//* <S0A2> */
	/************************************/
	/* Inrush prevention circuit control(Separation)			*/
	/************************************/
	PcmRlyControlProcess( &PowerManager->MainPowerChecker,
						  &PowerManager->PowerSts,
						  FaultState,
						  DBRequest,
						  AxisRscC->FnCmnCtrl->KnlMode );
#endif

	/* Inrush prevention resistor overload detection processing(Unit unit) */
#if (CSW_HW_CHKRUSH == TRUE)
	if( CHK_RUSH_NO_OPERATION != PcmCheckRushRxOverLoad( &(PowerManager->Rush),
														 &(PowerManager->PowerSts) ) )
	{	/* A.740 : Inrush resistance overload */
		ALMSetGlobalAlarm( ALM_RSOL );
	}
#endif

	/* Regeneration error detection processing(Unit unit) */
#if (CSW_HW_REGENERATION_CTRL == TRUE)
#if 0		/* <S21D> */
	if( PowerManager->MainPowerChecker.conf.Dcbus == FALSE )
	{
		if( CHK_REGENE_NO_OPERATION != PcmCheckRegeneAlarm( &(PowerManager->RegeneCtrl),
														    PowerManager->PowerSts.PowerOn) )
		{	/* A.300 : Regenerative abnormality */
			ALMSetGlobalAlarm( ALM_RG );
		}
	}
#else		/* <S21D> */
	if( (PowerManager->MainPowerChecker.conf.Dcbus == FALSE)
	 || ( (PowerManager->MainPowerChecker.conf.Dcbus == TRUE)			/* DCpower input */
	   && (PowerManager->RegeneCtrl.conf.RegSyn == PNREGSYN) ) )		/* PNUse regeneration synchronization function in bus common system */
	{
		if( CHK_REGENE_NO_OPERATION != PcmCheckRegeneAlarm( &(PowerManager->RegeneCtrl),
														    PowerManager->PowerSts.PowerOn) )
		{
			if( (PowerManager->RegeneCtrl.conf.RegSyn == PNEXREG)		/* Using external regeneration function	*/
			 && (PowerManager->RegeneCtrl.conf.RegRxIn == FALSE) )		/* External regenerative resistor		*/
			{
				if( PowerManager->MainPowerChecker.var.WrnExRegInhibit == FALSE )
				{ /* A.924 : Regeneration warning */
					ALMSetGlobalAlarm( WRN_EXREG );
					PowerManager->MainPowerChecker.var.WrnExRegInhibit = TRUE;
				}
			}
			else
			{ /* A.300 : Regenerative abnormality */
				ALMSetGlobalAlarm( ALM_RG );
			}
		}
	}
#endif
#endif

#if (CSW_DETECT_OH_FROM_TEMPIC == TRUE) /* <S0A3> *//* <S0ED> */
	/* Over Heat *//* <S03A> *//* <S0DE> */
	OhmCheckOverHeatAlm( AxisRscC->AlmManager,
						 AxisRscC->OverHeat,
						 DetTempGetCtrlBoardTemperature(AxisRscC->TempManager),
						 DetTempGetPowerBoardTemperature(AxisRscC->TempManager) );

	/* Detect Temperature IC Error *//* <S0DE> */
	AdetCheckTempDeviceErr( AxisRscC->AlmManager,
							DetTempGetCtrlBoardErrorCount(AxisRscC->TempManager),
							DetTempGetPowerBoardErrorCount(AxisRscC->TempManager) );
#endif


#if (CSW_HW_INTERNAL_FAN == TRUE) /* <S0A3> */
	/* Servopack built-in fan stoppage detection processing(Unit unit) */
	AlmWrnSts = PcmCheckFanLock( &(PowerManager->FanLock),
								 SeqInputSignals.flag.FanAlm,
								 BaseEnableSts );
	/* Servopack built-in fan stop alarm/warning setting */
	if(0 != (AlmWrnSts & CHK_FANLOCK_SET_ALARM))
	{	/* A.7AB : Servopack built-in fan stop alarm */
		ALMSetGlobalAlarm( ALM_FANLOCK );
	}
	else if(0 != (AlmWrnSts & CHK_FANLOCK_SET_WARNING))
	{	/* A.923 : Stopping the SERVOPACK built-in fan */
		ALMSetGlobalAlarm( WRN_FANLOCK );
	}
	else
	{
		;
	}
#endif


	/* Power consumption calculation(Unit unit) *//* <S020> */
#if (CSW_POWER_MONITOR_USE == TRUE)
	SetPowerMoniter( AxisRscC->PowerMonitor, GetPowerConsumption(AxisRscC->CalcWattage) );
#endif

	/* OCDetection processing	*//* <S126> */
	svAsicRegs = AxisRscC[0].SvAsicRegs;		/* OCThe detection is0AxisUDLUse the area.	*/
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		AxisRsc = &AxisRscC[ax_no];
		AdetCheckOCAlarmDetections(AxisRsc->AlmManager, svAsicRegs, ax_no, AxisRsc->SeqCtrlOut->UdlBeStatus);		/* <S137> */

		/* Motor overcurrent detection check processing	*//* <S1F7> */
		AdetCheckMotorOverCurrent( AxisRsc->AlmManager, AxisRsc->SvAsicRegs, AxisRsc->SeqCtrlOut->UdlBeStatus,
																								BaseEnableSts );
	}

	/* Motor overcurrent detection check processing	*//* <S1F5> */
//	AdetCheckMotorOverCurrent( AxisRsc->AlmManager, svAsicRegs, AxisRsc->SeqCtrlOut->UdlBeStatus,
//			BaseEnableSts );/*<S1F5> */


	/* < S154> Start  */
	/*----------------------------------------------------------------------*/
	/*		Lifespan prediction function processing												*/
	/*----------------------------------------------------------------------*/
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
	/* <S180> Start */
#if 0
	AxisRscC->hLifeTime->RushLifeTimeHdl.var.CurrentChargeOk = PowerManager->PowerSts.ChargeOk;
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		AxisRsc = &AxisRscC[ax_no];
		/* DBRelay operation count is set while motor is rotatingDBCounts up only when turned on , Axis support*/
		LifeTimeDBSwitchingCount( AxisRscC->hLifeTime, AxisRsc->AdatHndl->AdatP.SeqCtrlOut->TgOnSts , AxisRsc->BeSequence->DbOn);
	}
	/* Lifetime remaining rate calculationMainProcessing													*/
	LifeTimeSurvivorCalculateMain( AxisRscC->hLifeTime , AxisRscC->RegManager->hPnReg , AxisRscC->UniPrms->Cprm );

	/* Preventive maintenance warning output */
	PreWrnSts = AdetCheckPreWarningSigOut( AxisRscC->hLifeTime,
								&(AxisRscC->PowerManager->MainPowerChecker) );
	if( PreWrnSts == CHK_PRE_SET_WRN )
	{
		/* Warning (A.9B0) Output */
		ALMSetGlobalAlarm( WRN_PREVENTMAINT );
	}
#else
	AxisRscC->hLifeTime->RushLifeTimeHdl.var.CurrentChargeOk = PowerManager->PowerSts.ChargeOk;		/* <S192> */
	/* Lifetime remaining rate calculationMainProcessing													*/
	LifeTimeSurvivorCalculateMain( AxisRscC->hLifeTime, AxisRscC->RegManager->hPnReg, AxisRscC->UniPrms->Cprm );

//	AxisRscC->hLifeTime->RushLifeTimeHdl.var.CurrentChargeOk = PowerManager->PowerSts.ChargeOk;	/* <S192> */
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		AxisRsc = &AxisRscC[ax_no];
		/* DBRelay operation count is set while motor is rotatingDBCounts up only when turned on , Axis support*/
		LifeTimeDBSwitchingCount( AxisRsc->hDBLifeTime, AxisRsc->AdatHndl->AdatP.SeqCtrlOut->TgOnSts,
								AxisRsc->BeSequence->DbOn);
		/* Writing parameters for lifespan functionTOEeprom */
//		LifeTimePrmStoreValueToEeprom( AxisRsc->hDBLifeTime,AxisRsc->RegManager->hPnReg,
//								AxisRsc->UniPrms->Cprm);		/* <S186> *//* <S192> */
		/* DBLifetime remaining rate calculation function */
		LifeTimeDBSurvivorCalculate( AxisRsc->hDBLifeTime );

		/* Preventive maintenance warning output */
		PreWrnSts = AdetCheckPreWarningSigOut( AxisRscC->hLifeTime,
											&(AxisRscC->PowerManager->MainPowerChecker),
											AxisRsc->hDBLifeTime->var.DBSurvivor);
		if( PreWrnSts == CHK_PRE_SET_WRN )
		{
			/* Warning (A.9B0) Output */
//			ALMSetGlobalAlarm( WRN_PREVENTMAINT );						/* <S192> */
			ALMSetServoAlarm( AxisRsc->AlmManager, WRN_PREVENTMAINT );	/* <S192> */
		}
	}
#endif
	/* <S180> End */
#endif
	/* <S154> End  */

	/*----------------------------------------------------------------------*/
	/* Scan C main function execution */
	/*----------------------------------------------------------------------*/
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		AxisRsc = &AxisRscC[ax_no];

		SyscAlarmDetections( AxisRsc );

		SyscBaseDriveSequence( AxisRsc );

		SyscSequenceMain( AxisRsc );

		SyscSencAbsoOperation( AxisRsc ); /* Absolute value encoder processing main     *//*<S02D>*/
	}
	/*----------------------------------------------------------------------*/
	/*		ScanCCommon output processing											*/
	/*----------------------------------------------------------------------*/
	SyscSequenceCommon( AxisRscC );

/* <S150> >>>>> Below,USBDetect by polling without using function interrupts */
	/*----------------------------------------------------------------------*/
	/*	USB Function Interface												*/
	/*----------------------------------------------------------------------*/
	UsbFuncIntIntu2f();					/* Plug-in/plug-out processing				*/
	UsbFuncIntIntu2fepc();				/* Bus reset/change bus speed			*/
/* <<<<< <S150> */

	/* <S154> Start */
	/*----------------------------------------------------------------------*/
	/* Alarm latch main function execution */
	/*----------------------------------------------------------------------*/
/* <S212> */
//	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
//	{
//		AxisRsc = &AxisRscC[ax_no];
//
//		/* Alarm latch processing */
//		AlmLatchMonitorSequence( AxisRsc->hAlmLatchHndl );
//	}
	/* <S154> End */
	/*----------------------------------------------------------------------*/
	/*	@@ Serial Interface													*/
	/*----------------------------------------------------------------------*/
	KpxSifMsgManager( ); 				/* Serial Interface Manager 	 	*/
	KpxVirSifManager( ); 				/* Virtual Serial Interface Manager */
	/*----------------------------------------------------------------------*/

	/*----------------------------------------------------------------------*/
	/*		Timestamp processing												*/
	/*----------------------------------------------------------------------*/
	KlibCountTimeStamp( KPI_SCCYCLEMS, PowerManager->PowerSts.PowerOnEep );

	/*----------------------------------------------------------------------*/
	/*		ScanCProcessing for processing time monitor				<S043>					*/
	/*----------------------------------------------------------------------*/
	/* Consider interrupt ScanA,ScanBCalculate the processing total time first */ /*<S1CD>*/
	SABinC_exeTime = TaskProcessTime->SAinC_exeTime + TaskProcessTime->SBinC_exeTime;
	/* ScanCExecutingScanA,ScanBSubtract processing timeScanCUse processing time */
	TaskProcessTime->SC_crtTime = KlibGetLongCounter(ScanCStartTime)
									- SABinC_exeTime;							/*<S1CD>*/
//<S1CD>									- TaskProcessTime->SAinC_exeTime	/*<S042>*/
//<S1CD>									- TaskProcessTime->SBinC_exeTime;	/*<S042>*/
	if( TaskProcessTime->SC_crtTime > TaskProcessTime->SC_maxTime)
	{/* Updating maximum processing time */
		TaskProcessTime->SC_maxTime = TaskProcessTime->SC_crtTime;
	}

	/* ScanCExecution flag reset <S043> */
	TaskProcessTime->SC_exeflg = FALSE;
	/* ScanCInsideScanA,ScanBClear execution time */ /*<S1CD>*/
	TaskProcessTime->SAinC_exeTime = 0;
	TaskProcessTime->SBinC_exeTime = 0;



#if CSW_NETIF_TYPE	!= NETIF_ANLGPLS_TYPE
//	HALdrv_ClearDebugSP( INTERRUPT_ScanC );		/* Change to port output in interrupt branch processing<S042> */
#endif

}


/****************************************************************************
  Description: Return Scan C Counter
-----------------------------------------------------------------------------
  Parameters	:
  Return		:
		none
		

		Erase after
*****************************************************************************/
LONG	CpiGetScanCCnt(){ return TEST_SCANC_CNT; }


/**************************** end of file **********************************/

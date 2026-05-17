/****************************************************************************
Description	: Base Scan Task(ScanB) Main module
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

Functions	  :
	SysBaseScanTask()			:Main routine of the Base Scan Task

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
I.M.   2011.01.24     renewal

****************************************************************************/
//#include <INTEGRITY.h>
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "NetworkIf.h"	// <USB-FUNCTION> K.Sakamoto
#include "HwDeviceIf.h"
#include "BaseControlMain.h"
#include "MicroScanAIf.h"
#include "AlarmManager.h"
#include "KLib.h"
#include "MLib.h"
#include "DMAdrv.h"
#include "SerialEncIf.h"
#include "KnlApi.h"		/* @@ */
#include "AnalogMonitorCalc.h"	/* <S016> */
#include "Pls.h"

LONG TEST_SCANB_CNT;

/* Prototype */
void SysScanBInt( void );

static void SysBaseServoReadHWInputs( AXIS_HANDLE *AxisRscB );
static void SysBasePowerControl( AXIS_HANDLE *AxisRscB, ULONG SBCnt, ULONG SBCntInc );
static LONG SysMaekNetIfMonInSignal(AXIS_HANDLE *AxisRscB);	/* <S1C0> */
static void SysBaseServoInputProcess( AXIS_HANDLE *AxisRscB );
static void SysBaseServoOutputProcess( AXIS_HANDLE *AxisRscB );
static void	SysBaseMakeScanCMonitor( AXIS_HANDLE *AxisRscB );
//static LONG	KpiInputSpdRefAd( ASIC_HW_IF_STR *AsicHwReg, PRMDATA *Prm, LONG Kvrefad, UCHAR KnlMode );
static UCHAR GetNetworkTriggerSts( AXIS_HANDLE *AxisRscB ); /* <S012> */

static void KpxInputSeqSignals( SEQ_IO_HNDL *IoSignals );	/* <S045> */
static void KpxOutputSeqSignals( LONG ax, SEQ_IO_HNDL *IoSignals );	/* <S045> *//* SGD7W IOI/O support<S118> */


#define AREG_ESYNC			0x0027				/* Sync Encorder          */
#define MREG_HSUR			0x0104				/* Host Cpu cmd port      */
#define MPCMD_RENEWAL		0x0004				/* μ-program cmd renewal */
#define MPCMD_CURADSYNC		0x0008				/* Sync Cur Detect AD Int */

extern LONG	NumberOfAxes;

#if defined(_TEST_BOARD_) && !defined(_VC_DEBUG_)
void IntScanB( void ); /* Provisional measures */
extern void ClearBaseCtrlInterrupt( void );
extern void	SetSeqCtrlInterrupt( void );
extern void	SetEnableInterrupt( void );


/****************************************************************************
  Description: Interrupt Operation for Basecontrol Scan Task
-----------------------------------------------------------------------------
  Parameters	: none
  Return		: none
*****************************************************************************/
void IntScanB( void )
{
	AXIS_HANDLE	*AxisRscB;
	TASK_MEASURE_TIME	*TaskProcessTime;
	ULONG 				tmp_time;

	/* Local variable initialization */
	AxisRscB = GetAxisHandle(0);
	TaskProcessTime = AxisRscB->TaskProcessTime;

//	HALdrv_OutputDebugSP( INTERRUPT_ScanB );
	KPI_EI();							//@@	SetEnableInterrupt( );
	ClearBaseCtrlInterrupt( );

//	if( ScanBStartCtr < 0x0F )
//	{
//		ScanBStartCtr++;
//	}
//	else
//	{
		SysScanBInt( );
//	}
	/*------------------------------------------------------------------*/
//	KpxSifMsgManager( ); 			/* Serial Interface Manager <XXX> 	*/
	/*------------------------------------------------------------------*/

	/*------------------------------------------------------------------*/
	/*		ScanCStartup													*/
	/*------------------------------------------------------------------*/
	if( GetInterruptTiming(INTERRUPT_ScanC) )
	{
		/* Update the ScanC Task data */
		SysBaseMakeScanCMonitor( AxisRscB );

		/* Set Sequence Control Interrupt Request */
		SetSeqCtrlInterrupt( );

		/* For measuring total processing time */
		tmp_time = TaskProcessTime->ScanSumTime;
		if(tmp_time > TaskProcessTime->maxTimePer2ms)
		{
			TaskProcessTime->maxTimePer2ms = tmp_time;
		}
		TaskProcessTime->crtTimePer2ms = tmp_time;
		TaskProcessTime->ScanSumTime = 0;
	}

//	HALdrv_ClearDebugSP( INTERRUPT_ScanB );
}
#endif

#if 0 /* Interim processing 2013/04/01MDIn the version, as belowAsic */
#if	(CSW_HW_ALL_SVASIC_WR_SUPPORT != FALSE)

/*--------------------------------------------------------------------------*/
/*	 Servo ASIC Syncronyzation												*/
/*--------------------------------------------------------------------------*/
static void SyncServoASIC(void)
{
	VUINT16	*MicroAdr;

	/* Synchronize JL-076 and host cpu */
	MicroAdr = (VUINT16 *)(HW_SVASIC_ADDRESS+HW_SVASIC_COMADR_OFS);
	/* Synchronize to maicroElectric Current Detect AD Request */
	MicroAdr[MREG_HSUR] = (MPCMD_CURADSYNC | MPCMD_RENEWAL);
//	MicroAdr[AREG_ESYNC] = 0;
}

#else

/*--------------------------------------------------------------------------*/
/*	 Servo ASIC Syncronyzation												*/
/*--------------------------------------------------------------------------*/
static void SyncServoASIC(void)
{
	VUINT16	*MicroAdr;
	UINT	i;

	for(i = 0; i < MAX_AXIS_NUMBER; i++)
	{
		/* Synchronize JL-076 and host cpu */
		MicroAdr = (VUINT16 *)(HW_SVASIC_ADDRESS + (HW_SVASIC_AXIS_OFS*i));
		/* Synchronize to maicroElectric Current Detect AD Request */
		MicroAdr[MREG_HSUR] = (MPCMD_CURADSYNC | MPCMD_RENEWAL);
		/* Synchronize to Serial Encorder */
//   		MicroAdr[AREG_ESYNC] = 0x0000;
	}
}
#endif
#endif

#ifdef CSW_SCANA_ENABLE
/****************************************************************************
  Description: Main routine of the Base Scan Task
-----------------------------------------------------------------------------
  Parameters	: none
  Return		: none
*****************************************************************************/
void SysScanBInt( void )
{
	AXIS_HANDLE	*AxisRscB;
	TASK_MEASURE_TIME	*TaskProcessTime;
//	ULONG 				clock_time;
//	BOOL				SyncState;			/*<S0C7>*/
	VULONG				ScanBStartTime;		/*<S043>*/
	BOOL				Interrupt;
#if (CSW_MICRO_DMA_TRANSFER != 0)
	UINT32				dma_err;
#endif
	LONG				ax_no;
	volatile ULONG		SAinB_exeTime;		/*<S1CD>*/

//	TEST_SCANB_CNT++;
//	HALdrv_OutputDebugSP( INTERRUPT_ScanB );	/* Change to port output in interrupt branch processing<S042> */
	KlibRstLongTimer((ULONG*)&ScanBStartTime);		/*<S043>*//*<S0C7>*/

	/* Processing time measurement started	*/
//	HALdrv_OutputDebugSP( 5 );
#if 0
	clock_time = KlibGetClockTime();
#endif

	/* Local variable initialization */
	AxisRscB = GetAxisHandle(0);
	TaskProcessTime = AxisRscB->TaskProcessTime;
#if (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE)
	Interrupt = AxisRscB->NetNodeIf->InterruptReq;
#else
	Interrupt = FALSE;			/* Uninitialized variable correction<S191> */
#endif

	/* ScanBExecution flag set <S043> */
	TaskProcessTime->SB_exeflg = TRUE;

#if 0 /* Interim processing 2013/04/01MDIn the version, as belowAsic */
	/*------------------------------------------------------------------*/
	/*	Synchronize to all Servo ASICs			 						*/
	/*------------------------------------------------------------------*/
	SyncServoASIC();
#endif

	/*------------------------------------------------------------------*/
	/*	Set Interrupt Cycle P1mode					 			<S1D3>	*/
	/*------------------------------------------------------------------*/
	SencSetIntCycleP1Mode( AxisRscB->MencV );

	/*------------------------------------------------------------------*/
	/*	Read Input data from Hardware devices	 						*/
	/*------------------------------------------------------------------*/
	SysBaseServoReadHWInputs( AxisRscB );

#if 0 /* 2012.09.05 Y.Oka ★ScanBμAccess processing needs to be redesigned★ */
	/*------------------------------------------------------------------*/
	/*	Input the JL-077 uP cyclic data									*/
	/*------------------------------------------------------------------*/
#if (CSW_MAICRO_DMA_TRANSFER != 0)
	DMAdrvStartMemoryTransfer(DMA_SVDEV_CH, DMA_SVDEV_IN_LINK);
#else
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		MicroSA_InputCycResponse( AxisRscB[ax_no].SvAsicRegs,
								AxisRscB[ax_no].MencV,
								AxisRscB[ax_no].CtrlLoopOut );
	}
#endif
#endif

	/*------------------------------------------------------------------*/
	/*	Main Power Controls						 						*/
	/*------------------------------------------------------------------*/
	SysBasePowerControl( AxisRscB,
						 TaskProcessTime->SBCount,
						 TaskProcessTime->SBCountInc );
	TaskProcessTime->SBCount += (TaskProcessTime->SBCountInc+1);

	/*------------------------------------------------------------------*/
	/*	<S053> Servo control input processing/Device common control processing					*/
	/*------------------------------------------------------------------*/
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
#if (CSW_MICRO_DMA_READ != 0)
		while(DMAdrvCheckLinkStatus(DMA_SVDEV_CH, DMA_SVDEV_IN_LINK+ax_no));
#endif
		SysBaseServoInputProcess( &AxisRscB[ax_no] );
	}

	/*------------------------------------------------------------------*/
	/*	Input the Communication Process data 							*/
	/*------------------------------------------------------------------*/
	OpiInputProcess( AxisRscB, Interrupt );/*<S0C7>*/
#if (CSW_MICRO_DMA_TRANSFER != 0)
	if(0 != dma_err)
	{
		if(SyncState)
		{
			ALMSetGlobalAlarm(ALM_WDC_ERR);
		}
	}
#endif

	/*------------------------------------------------------------------*/
	/*		Data trace processing	<S053>									*/
	/*		※SGDV-MD 20130513version					*/
	/*------------------------------------------------------------------*/
#if !defined(_TEST_BOARD_) || defined(_VC_DEBUG_)	/* 2012.12.13 Y.Oka Processing time countermeasures */
	/* Get and Set Network Trigger Info. *//* <S012> */
	DtrcSetNetworkTrigger( AxisRscB->TrcHndl, GetNetworkTriggerSts(AxisRscB) );
	/* Execute Trace Operation */
	DtrcExecute( AxisRscB->TrcHndl );
#endif /* #if !defined(_TEST_BOARD_) || defined(_VC_DEBUG_) */

	/*------------------------------------------------------------------*/
	/* Execute the Network Main Procedure								*/
	/*------------------------------------------------------------------*/
	OpiRunTimeScanProcess( AxisRscB, Interrupt );

	/*------------------------------------------------------------------*/
	/*	Servo control main processing											*/
	/*------------------------------------------------------------------*/
	SysBaseServoOutputProcess( AxisRscB );

	/*------------------------------------------------------------------*/
	/*	Start DMA transfer to output the JL-077 uP cyclic data			*/
	/*------------------------------------------------------------------*/
#if (CSW_MICRO_DMA_TRANSFER != 0)
	DMAdrvStartMemoryTransfer(DMA_SVDEV_CH, DMA_SVDEV_OUT_LINK);
#endif

	/*------------------------------------------------------------------*/
	/*	Output the Communication Process data							*/
	/*------------------------------------------------------------------*/
	OpiOutputProcess( AxisRscB, Interrupt );

/* --> <USB-FUNCTION> 2013.05.20 K.Sakamoto */
//#ifdef _USB_FUNC_USE_	/*<S138>*/
//<S150>	MsgIfPollScanB( AxisRscB->NetMsgIf );
	/* USB Function Interface Manager	*//*<S150>*/
	KpxUsbMsgManager( );
//#endif	//#ifdef _USB_FUNC_USE_	/*<S138>*/
/* <-- <USB-FUNCTION> 2013.05.20 K.Sakamoto */

#if 0	/* <S053> */
#if !defined(_TEST_BOARD_) || defined(_VC_DEBUG_)	/* 2012.12.13 Y.Oka Processing time countermeasures */
	/*------------------------------------------------------------------*/
	/*		Data trace processing											*/
	/*------------------------------------------------------------------*/
	/* Get and Set Network Trigger Info. *//* <S012> */
	DtrcSetNetworkTrigger( AxisRscB->TrcHndl, GetNetworkTriggerSts(AxisRscB) );
	/* Execute Trace Operation */
	DtrcExecute( AxisRscB->TrcHndl );
#endif /* #if !defined(_TEST_BOARD_) || defined(_VC_DEBUG_) */
#endif

	/* <S016> */
	/*------------------------------------------------------------------*/
	/*		Analog monitor processing											*/
	/*------------------------------------------------------------------*/
	if( *(AxisRscB->ScanB250us) )
	{
		BpxOutputAnalogMonitor(AxisRscB);
	}

	/*------------------------------------------------------------------*/
	//KpxSifMsgManager( ); 			/* Serial Interface Manager 	 	*/
	/*------------------------------------------------------------------*/
	/* ScanBBase free run timer count up */
	KlibCountLongTimer(TaskProcessTime->SBcountInMS);

	/* <S016> */
	*(AxisRscB->ScanB250us) ^= 0x01;	/* ScanB 250us Cycle Flag		*/

	/*------------------------------------------------------------------*/
	/* For processing time measurementGPIO OFF */
	/*------------------------------------------------------------------*/
#if 0
	clock_time = KlibGetClockTime() - clock_time;
	if(clock_time > TaskProcessTime->SB_maxTime)
	{
		TaskProcessTime->SB_maxTime = clock_time;
	}
	TaskProcessTime->SB_crtTime = clock_time;
	TaskProcessTime->ScanSumTime += clock_time;
#endif

	/* DEBUG for M3	*//* Update the ScanC Task data	Scan	*/
	if(HALdrv_GetIntUDLStatus( INTERRUPT_ScanC,  AxisRscB->SvAsicRegs->AsicHwReg ))
	{
		SysBaseMakeScanCMonitor( AxisRscB );
	}
	
	/*----------------------------------------------------------------------*/
	/*		ScanBProcessing for processing time monitor					<S043>				*/
	/*----------------------------------------------------------------------*/
	/* Consider interrupt ScanAGet the total processing time first */ /*<S1CD>*/
	SAinB_exeTime = TaskProcessTime->SAinB_exeTime;
	/* ScanBExecutingScanASubtract processing timeScanBUse processing time */
	TaskProcessTime->SB_crtTime = KlibGetLongCounter(ScanBStartTime)
									- SAinB_exeTime;	/*<S1CD>*/
//<S1CD>												- TaskProcessTime->SAinB_exeTime;
	TaskProcessTime->SB_sumTime += TaskProcessTime->SB_crtTime;				/* <S046> */
	if( TaskProcessTime->SB_crtTime > TaskProcessTime->SB_maxTime)
	{/* Updating maximum processing time */
		TaskProcessTime->SB_maxTime = TaskProcessTime->SB_crtTime;
	}

	if( TaskProcessTime->SC_exeflg == TRUE )
	{/* ScanCExecutingScanBRetain processing time */
		TaskProcessTime->SBinC_exeTime += TaskProcessTime->SB_crtTime;
	}
	else
	{
		TaskProcessTime->SBinC_exeTime = 0;
	}

	/* ScanBExecution flag reset */
	TaskProcessTime->SB_exeflg = FALSE;
	/* ScanBInsideScanAClear execution time */ /*<S1CD>*/
	TaskProcessTime->SAinB_exeTime = 0;

//	HALdrv_ClearDebugSP( INTERRUPT_ScanB );		/* Change to port output in interrupt branch processing<S042> */

}
#endif

/****************************************************************************
  Description: Converter Control
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscB	: Pointer to Axis Handle
  Return		:
		none
*****************************************************************************/
static void SysBasePowerControl( AXIS_HANDLE *AxisRscB, ULONG SBCnt, ULONG SBCntInc )
{
	LONG			ax_no;
//	SHAL_LTSIGNALS	*LtSignal;
	LONG			DcVolt; /* <S039> */
	POW_MNG_HNDL	*PowerManager;
	BASE_CTRL_HNDL	*BaseControls;
	CTRL_LOOP_OUT	*CtrlLoopOut;		/* <S024> */
#if (CSW_HW_REGENERATION_CTRL == TRUE)
	BOOL			AllMotStop;
#endif
#if (CSW_POWER_MONITOR_USE == TRUE) /* <S039> */
	BASE_LOOPCTRLS	*BaseLoops;
	CALC_WATTAGE 	*CalcWattage;
#endif

	PowerManager = AxisRscB->PowerManager;
	/*----------------------------------------------------------------------*/
	/*	Converter Controls													*/
	/*----------------------------------------------------------------------*/
	SBCnt += SBCntInc; /* <S0F4> */
	if( (SBCnt&1) == 0 ) /* Σ-7250usPeriod processing */
	{
#if(CSW_HW_DC_DETECT == TRUE)
		/* P-NVoltage detection processing */
		PcmDetectDcVolt(&PowerManager->DetectDcVolt,
						&PowerManager->PowerSts);
		/* OVDetection processing */
		PcmCheckOverVoltage(&PowerManager->MainPowerChecker,
							&PowerManager->DetectDcVolt,
							&PowerManager->PowerSts,
							&PowerManager->RegeneCtrl );					  /* <S0F7> */

		/* Detect DC Volt AD Convert Start for Next Time */
//		HALdrv_StartAdConvert( );
#endif

#if (CSW_HW_REGENERATION_CTRL == TRUE)
		/* All axes motor stop confirmation */
		AllMotStop = TRUE;
		for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
		{
			if( (AxisRscB[ax_no].SeqCtrlOut)->MotStop == FALSE )
			{ /* When there is an axis in operation */
				AllMotStop = FALSE;
				break;
			}
		}

	/* <S0CB> Start */
#if 0
		/* Regenerative transistor control processing */
		if( (PowerManager->MainPowerChecker.conf.Dcbus == FALSE)
			&& (PowerManager->DetectDcVolt.conf.VdetPrmOk == TRUE)
			&& (PowerManager->MainPowerChecker.conf.TestMode2 == FALSE) )
		{ /* ACInput mode && Voltage detection setting is normal && !TestMode2 */
			PcmRegeneControl( &PowerManager->RegeneCtrl,
							  &PowerManager->PowerSts,
							  AllMotStop );
		}
#endif
	/* ---TestMode6(Automatic tester mode: Force regeneration controlON/OFF)If not, operate regeneration control in this process.--- */		/* <S0CB> */
	if( PowerManager->MainPowerChecker.conf.TestMode6 == FALSE )										/* <S0CB> */
	{
		/* Regenerative transistor control processing */
#if 0		/* <S21D> */
		if( (PowerManager->MainPowerChecker.conf.Dcbus == FALSE)
#else		/* <S21D> */
		if( ( (PowerManager->MainPowerChecker.conf.Dcbus == FALSE)			/* ACInput mode 	*/
		   || ( (PowerManager->MainPowerChecker.conf.Dcbus == TRUE)			/* DCpower input 	*/
			 && (PowerManager->RegeneCtrl.conf.RegSyn == PNREGSYN) ) )		/* PNUse regeneration synchronization function in bus common system */
#endif
			&& (PowerManager->DetectDcVolt.conf.VdetPrmOk == TRUE)
			&& (PowerManager->MainPowerChecker.conf.TestMode2 == FALSE) )
		{ /* ACInput mode && Voltage detection setting is normal && !TestMode2 */
			PcmRegeneControl( &PowerManager->RegeneCtrl,
							  &PowerManager->PowerSts,
							  AllMotStop );
		}
	}
	/* <S0CB> End */
#endif
	}
//	SBCnt += SBCntInc;
	if( (SBCnt&1) != 0 )
	{
		for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
		{
			BaseControls = AxisRscB[ax_no].BaseControls;
			CtrlLoopOut = AxisRscB[ax_no].CtrlLoopOut;		/* <S024> */
#if( CSW_SVCF_MPKVVCOMP )
			/* DcVolt Compen.Gain Axis. for Micro */
			PcmMicroKvvCalc( &BaseControls->MpKvvComp,
							 PowerManager->PowerSts.DcVolt,
							 PowerManager->MainPowerChecker.conf.UvLevel );
#endif

#if( CSW_SVCF_WEAKENFIELD_USE )
			/* For field weakening parameter calculationPNVoltage averaging */
			PcmWFPrmCalc( &BaseControls->WeakenField,
						  CtrlLoopOut->MotSpd_a,			/* <S024> */
						  PowerManager->PowerSts.DcVolt );
#endif
		}
	}

#if (CSW_POWER_MONITOR_USE == TRUE) /* <S039> */
	/*----------------------------------------------------------------------*/
	/*	Power consumption calculation														*/
	/*----------------------------------------------------------------------*/
	/* DCPower supply voltage acquisition */
	DcVolt = PcmGetDcVoltage( &(AxisRscB->PowerManager->PowerSts) );
	/* Power consumption calculation */
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		BaseControls = AxisRscB[ax_no].BaseControls;
		BaseLoops = AxisRscB[ax_no].BaseLoops;
		CalcWattage = AxisRscB[ax_no].CalcWattage;
		/* Power consumption monitor calculation */
		CalcPowerMoniter( CalcWattage, BaseLoops->PowMonVq, BaseLoops->PowMonIqd,
				BaseLoops->PowMonVd, BaseLoops->PowMonIdd, DcVolt, BaseControls->BeComplete );
	}
#endif /* (CSW_POWER_MONITOR_USE == TRUE) */

}

/****************************************************************************
  Description: Hardware Input Procedure for Base Servo Controls
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscB	: Pointer to Axis Handle
  Return		:
		none
*****************************************************************************/
static void SysBaseServoReadHWInputs( AXIS_HANDLE *AxisRscB )
{
	LONG			ax_no;
	ASIC_HW_IF_STR 		*axisHwReg;/* <S04D> */

	/*	Input the FBA and COA signal status									*/
	/*----------------------------------------------------------------------*/
	HALdrv_InputSerialIO();

	/*----------------------------------------------------------------------*/
	/*
	 *	Input Analog & Pulse hardware status
	 */
	/*----------------------------------------------------------------------*/
#if  SVFSEL_ADC_USE == 1     /* <S09F> */
	KpxAdcScanBService( AxisRscB->pAdc );		/* ADC ScanB */
#endif                       /* <S09F> */
	KpxPlsScanBService( AxisRscB->pPlsIfV );	/* Puls ScanB */

	/*----------------------------------------------------------------------*/
	/* Input Signal Latch Status */
#if	(CSW_SVCF_FBLATCH == TRUE)/* <S004> */
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		/* <S1D6> 1Fixed a bug where only the axis was latched */
		/* <S04D>:First argument changed to support fully closed encoder */
		if(AxisRscB[ax_no].UniPrms->Bprm->FencUse != FALSE)			/* <S1D6> */
		{
			axisHwReg = AxisRscB[ax_no].FencV->EncIf.PosCmpSetReg;	/* <S1D6> */
		}
		else
		{
			axisHwReg = AxisRscB[ax_no].MencV->EncIf.PosCmpSetReg;	/* <S1D6> */
		}
		SHalDrv_ReadLatchPosition( axisHwReg, AxisRscB[ax_no].LtStsAdr,
								 AxisRscB[ax_no].SvControlIf->ExtLtSts, AxisRscB[ax_no].SvControlIf->ExtLtEnable );
	}
#endif

#if	(CSW_SVCF_FULC_MODE == CSW_FULC_SB_SUPPORT)/* Not used	*/
	/*------------------------------------------------------------------*/
	/*	Input the Position of Fully-Closed Encoder						*/
	/*------------------------------------------------------------------*/
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		FencReadPosition(AxisRscB[ax_no].FencV, AxisRscB[ax_no].SvAsicRegs->AsicHwReg);
	}
#endif	/* (CSW_SVCF_FULC_MODE == CSW_FULC_SB_SUPPORT) */

	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		AxisRscB[ax_no].IoSignals->HwSin = HALdrv_ReadMonInputSignals( ax_no /*pAx-AxisRscB */ );
		AxisRscB[ax_no].IoSignals->HwSin |= (((AxisRscB[ax_no].pPlsIfV)->f.PerClrSignal == TRUE) << 15); /* Erase after */
	}

	/* I/O signal status for Option	*//* <S1C0> */
	SysMaekNetIfMonInSignal(AxisRscB);

}

/****************************************************************************
  Description: Input Procedure for Base Servo Controls
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscB	: Pointer to Axis Handle
  Return		:
		none
-----------------------------------------------------------------------------
Single axis configuration(SGDVCompatibility)
	D0~D7	:SI0~SI6
	D7~D9	:SO0~SO3
	D16~D18:PgAphase,PgBphase,PgCphase	Currently not supported
	D24, D25:HWBB1,2
	D26~D29:STOP1~4	:Safety option card input signal1~4	Currently not supported
	D30, D31:EDM2,3		:Safety option card output signal2,3	Currently not supported
	
2Axis configuration
	D0~D11	:SI01~SI06,SI11~SI16
	D12~D16	:SO0~SO5
	D17~D98:PgAphase,PgBphase,PgCphase	Currently not supported
	D24, D25:HWBB1,2
	D26~D29:STOP1~4	:Safety option card input signal1~4	Currently not supported
	D30, D31:EDM2,3		:Safety option card output signal2,3	Currently not supported

*****************************************************************************//* <S1C0> */
static LONG SysMaekNetIfMonInSignal(AXIS_HANDLE *AxisRscB)
{
	if(NumberOfAxes == 1)
	{/* Single axis configuration	*/

		AxisRscB[0].SvControlIf->NetIfMonInSignal1
			 = (ULONG)(HALdrv_ReadMonInputSignals(0) & 0x7F)					/* SISignal acquisition	*/
			 | (ULONG)(((HALdrv_ReadMonAllOutputSignals( ) & 0x0E)>>1) << 7)	/* SOSignal acquisition(Lower1Bit:ALM)	*/
			 | (ULONG)((AxisRscB[0].IoSignals->DetHwbbSts.Bbon1 & 0x01) << 24)
			 | (ULONG)((AxisRscB[0].IoSignals->DetHwbbSts.Bbon1 & 0x01) << 25)
				;/* Up to here	*/
	}
	else if(NumberOfAxes == 2)
	{/* 2Axis configuration	*/
		/* Perform processing only on the first axis(Unit processing)	*/
		AxisRscB[0].SvControlIf->NetIfMonInSignal2
		=  (HALdrv_ReadMonAllInputSignals( ) & 0xFFF)								/* SI0*,SI1*Signal acquisition	*/
			 | (ULONG)(((HALdrv_ReadMonAllOutputSignals( ) & 0x7C)>>2) << 12)	/* SOSignal acquisition(Lower2Bit:ALM)	*/
			 | (ULONG)((AxisRscB[0].IoSignals->DetHwbbSts.Bbon1 & 0x01) << 24)	/* HWBB1	*/
			 | (ULONG)((AxisRscB[0].IoSignals->DetHwbbSts.Bbon1 & 0x01) << 25)	/* HWBB2	*/
				 ;/* Up to here	*/

		/* Data of the first axis is transferred to other axes(1Axis number)	*/
		AxisRscB[1].SvControlIf->NetIfMonInSignal2 = AxisRscB[0].SvControlIf->NetIfMonInSignal2;
	}
}

/****************************************************************************
  Description: Input Procedure for Base Servo Controls
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscB	: Pointer to Axis Handle
  Return		:
		none
*****************************************************************************/
static void SysBaseServoInputProcess( AXIS_HANDLE *AxisRscB )
{
//<S053>	LONG			ax_no;
	AXIS_HANDLE		*AxisRsc;

	AxisRsc = AxisRscB;
//<S053>	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
#if 0 /*  ★μScanA→CpuScanA★ */
		/*------------------------------------------------------------------*/
		/* Read ScanA monitor values */
		/*------------------------------------------------------------------*/
		MicroSA_InputCycDataB( AxisRsc->SvAsicRegs,
								AxisRsc->MencV,
								AxisRsc->CtrlLoopOut,
								AxisRsc->UniPrms->Bprm );
#endif

#if 0 /* ★μScanA→CpuScanA★ */
#if defined(CSW_NETIF_SYNQNET_TYPE)
		if(AxisRsc->SvControlIf->CmdReady == FALSE)
#endif
		{
			MicroSA_InputCycDataExB( AxisRsc->SvAsicRegs,
									AxisRsc->CtrlLoopOut,
									AxisRsc->UniPrms->Bprm );
		}
#endif
		/*------------------------------------------------------------------*/
		/* Input Encoder position */
		/*------------------------------------------------------------------*/
		BpiInputMotorPosition( AxisRsc->BaseControls,
		                       AxisRsc->SvAsicRegs->AsicMicroReg,
		                       AxisRsc->UniPrms->Bprm,
		                       AxisRsc->UniPrms->Prm,	/* <S04D>:Argument arrangement	*/
		                       AxisRsc->CtrlLoopOut);	/* <kira>			*/

		/*------------------------------------------------------------------*/
		/* @@ Input PERR Clear Latched Position (AP ONLY) 					*/
		/*------------------------------------------------------------------*/
		KpiPlsPerClrLatchPosition( 						/* get clr position */
			AxisRsc->pPlsIfV, 							/*  				*/
			AxisRsc->BaseControls->CtrlCmdMngr.PosFbki,	/* Feedback Position */
			AxisRsc->MencV->MotPosX[0],					/* Motor Angle 		*/
			&(AxisRsc->BaseControls->ClrFbPos) );		/* Latched Position */


		/*------------------------------------------------------------------*/
		/* Output Monitor status to Network layer */
		/*------------------------------------------------------------------*/
		BpiOutputNetStatus(AxisRsc->BaseControls,
		                   AxisRsc->SvControlIf,
		                   AxisRsc->CtrlLoopOut,
		                   AxisRsc->SeqCtrlOut,
						   AxisRsc->UniPrms->Bprm,
						   AxisRsc->pPlsIfV);/* <S04D>:Argument addition *//* <S121> */

//<S053>		AxisRsc++;
	}

}


/****************************************************************************
  Description: Output Procedure for Base Servo Controls
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscB	: Pointer to Axis Handle
		BOOL Interrupt		: HINT Interrupt status
  Return		:
		none
*****************************************************************************/
static void SysBaseServoOutputProcess( AXIS_HANDLE *AxisRscB )
{

	LONG			ax_no;
	AXIS_HANDLE		*AxisRsc;
//	ASIC_HW_IF_STR	*pAsicComHwReg;
//	BASE_CTRL_OUT	*BaseCtrlOut;	/*<S0C7>*/
	BASE_CTRL_HNDL	*BaseCtrlData;
	ASIC_HW_IF_STR	*pAsicHwReg;
	FUN_CMN_CONTROL	*FnCmnCtrl;				/* <S0CB> */
	HWSOUT ForceOutSignals;					/* <S0CB> */
	POW_MNG_HNDL	*hPowerManager;;		/* <S0CB> */


	AxisRsc = AxisRscB;
	FnCmnCtrl = AxisRsc->FnCmnCtrl;			/* <S0CB> */
	hPowerManager = AxisRsc->PowerManager;	/* <S0CB> */

	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
//		BaseCtrlOut = AxisRsc->BaseCtrlOut;	/*<S0C7>*/
		BaseCtrlData = AxisRsc->BaseControls;

		KpxInputSeqSignals( AxisRsc->IoSignals ); 							/* <S045>		*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /* 2012.09.07 Y.Oka ★MercuryCurrently not supported★ */
		/* Latch processing2013/04/01MDversion as shown below, so a review is necessary. */
		SeqCtrlData = AxisRsc->SeqCtrlData;
		/* DECInput */
		SeqIo = &(SeqCtrlData->DecSignal);
		SeqIo->SignalStatus = ((SeqInputSignals.dw >> SeqIo->PinNumber) & SeqIo->Mask) ^ SeqIo->XorMask;
		sigbits = (SeqIo->SignalStatus << DEC_SIG_BITNO);
		/* EXT1Input */
		SeqIo = &(SeqCtrlData->ExtSignal[0]);
		SeqIo->SignalStatus = ((SeqInputSignals.dw >> SeqIo->PinNumber) & SeqIo->Mask) ^ SeqIo->XorMask;
		sigbits |= (SeqIo->SignalStatus << EXT1_SIG_BITNO);
		/* EXT2Input */
		SeqIo = &(SeqCtrlData->ExtSignal[1]);
		SeqIo->SignalStatus = ((SeqInputSignals.dw >> SeqIo->PinNumber) & SeqIo->Mask) ^ SeqIo->XorMask;
		sigbits |= (SeqIo->SignalStatus << EXT2_SIG_BITNO);
		/* EXT3Input */
		SeqIo = &(SeqCtrlData->ExtSignal[2]);
		SeqIo->SignalStatus = ((SeqInputSignals.dw >> SeqIo->PinNumber) & SeqIo->Mask) ^ SeqIo->XorMask;
		sigbits |= (SeqIo->SignalStatus << EXT3_SIG_BITNO);

		/* Save bit status */
		AxisRsc->SvControlIf->RspSigBit = sigbits | AxisRsc->SeqCtrlOut->RspSigBits;

/*--------------------------------------------------------------------------------------------------*/
/*		Sequence signal input																			*/
/*--------------------------------------------------------------------------------------------------*/
		KPI_SIN_START_SCANB( Inx );
		BinV.f.Ctrl.EnbPCtrl = KPI_SIN_PCON( Inx );
		BinV.f.Sv.Inhibit    = KPI_SIN_INHIBIT( Inx );
		BinV.f.Sv.GainSel0   = KPI_SIN_GAINSEL0( Inx );
		BinV.f.Sv.GainSel1   = KPI_SIN_GAINSEL1( Inx );
		BinV.f.Sv.Pcl        = KPI_SIN_PCL( Inx );										/* <V226>	*/
		BinV.f.Sv.Ncl        = KPI_SIN_NCL( Inx );										/* <V226>	*/
		BinV.f.Sv.Psel       = KPI_SIN_PSEL( Inx );		/* Command pulse input magnification switching signal  <V677>	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Gain number for gain switching																		*/
/*--------------------------------------------------------------------------------------------------*/
		BinV.GselNum = GET_BITSDATA( &BinV.f.Sv, BITNO_BI_GainSel0, 0x03 );

/*--------------------------------------------------------------------------------------------------*/
/*		PerClr Signal																				*/
/*--------------------------------------------------------------------------------------------------*/
		BinK.f.PerClrSignal = KPX_READ_PERCLRSIGNAL( );		/* Read the deviation clear interrupt signal			*/

/*--------------------------------------------------------------------------------------------------*/
/*		Divided Pulse Output Signal, ExtInterrupt Signal for Data Trace								*/
/*--------------------------------------------------------------------------------------------------*/
		BinK.DivPulseSig  = KPX_READ_DIVPULSESIG( );		/* Division pulse output signal(A,B,C)			*/
		BinK.ExtIntSignal = KPX_READ_EXTINTSIGNAL( );		/* External latch interrupt signal(Ext1,2,3)		*/
#endif /* 2012.09.07 Y.Oka ★MercuryCurrently not supported★ */


#if defined (_IO_SERVO_ON_ ) || defined (_TEST_BOARD_ ) /* Temporary support for prototype board 2012.11.05 Y.Oka */
/*--------------------------------------------------------------------------------------------------*/
/*		Sequence signal input																			*/
/*--------------------------------------------------------------------------------------------------*/
		SeqInputSignals = HALdrv_ReadInputSignals( );
		/* Set Servo ON Request */
		if( SeqInputSignals.flag.SI0 == FALSE )
		{
			BaseCtrlOut->CmdSeqBit = SERVO_ON_BIT;
		}
		else
		{
			BaseCtrlOut->CmdSeqBit = 0;
		}
#endif


#ifdef _IO_SERVO_ON_ /* @@Interim */
		/* 10rpm */
		AxisRsc->SvControlIf->NetSpdRef = 2542000;
		AxisRsc->SvControlIf->NetCtrlMcmd	= CTRLMODE_SPD;
#endif /* _IO_SERVO_ON_ */ /* @@Interim */

/*--------------------------------------------------------------------------------------------------*/
/*		@@ Provisional: Clear signal edge detection																*/
/*--------------------------------------------------------------------------------------------------*/
		AxisRscB->BaseControls->PerClrEdge = (AxisRscB->pPlsIfV->f.PerClrEdge == TRUE);
/*--------------------------------------------------------------------------------------------------*/

		AxisRsc++;
	}

#if (CSW_MAICRO_DMA_TRANSFER != 0)


	AxisRsc = AxisRscB;
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		MicroSA_InputCycResponse( AxisRsc->SvAsicRegs,
								AxisRsc->MencV,
								AxisRsc->CtrlLoopOut );
		AxisRsc++;
	}
#endif

	/*----------------------------------------------------------------------*/
	/*	ScanB Servo Control													*/
	/*----------------------------------------------------------------------*/
	AxisRsc = AxisRscB;
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		BaseCtrlData = AxisRsc->BaseControls;	 /* <S0E4> */
		BpiScanBMain( AxisRsc, ax_no );		/* <S1C0>: Add axis */

#if (CSW_AUTO_CHARGE_PUMP == FALSE)
		{
		BOOL OCState;
		/* Charge Pump Control Service */
		OCState = ALMCheckEachState(AxisRsc->AlmManager, ALM_OC);
//		BeSeqChargePumpControl( AxisRsc->SvAsicRegs, &(AxisRsc->BeSequence->ChargePump), OCState );/* <S0CB> Delete */
/* <S0CB> */
			/* ---TestMode8(Automatic tester mode: Force gate drive signal ON/OFF),
				AndTestMode9(Automatic tester mode: Force gate drive signalScanAinON/OFF), operate gate drive signal control in this process--- */
//			if( hPowerManager->MainPowerChecker.conf.TestMode8 == FALSE )		/* <S19F> */
			if(	( hPowerManager->MainPowerChecker.conf.TestMode8 == FALSE ) &&	/* <S19F> */
				 (hPowerManager->MainPowerChecker.conf.TestMode9 == FALSE) )	/* <S19F> */
			{
				BeSeqChargePumpControl(AxisRsc->SvAsicRegs, &(AxisRsc->BeSequence->ChargePump), 
				OCState,AxisRsc->MotorLessTest->MotorLessTestMode,AxisRsc->PowerManager->PowerSts.NegPumpReq);
				/* <S1B7> *//*Charge pumpON/OFFControl <S201>*/
			}
/* <S0CB> */
		}
#endif
		/*----------------------------------------------------------------------*/
		/*		Calculate Motor Speed for ScanA	[15000/OS]			<S0E4>		*/
		/*----------------------------------------------------------------------*/
		SHalDrv_SetMotSpdToMicro( AxisRsc->SvAsicRegs->AsicMicroReg, BaseCtrlData->MotSts.MotSpd );
		AxisRsc++;
	}

#ifndef CSW_SCANA_ENABLE
	/*----------------------------------------------------------------------*/
	/*	Output the Servo Control Reference to the JL-077 uProgram			*/
	/*----------------------------------------------------------------------*/
	AxisRsc = AxisRscB;
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		MicroSA_OutputCycReference( AxisRsc->SvAsicRegs,
								 AxisRsc->MencV,
								 AxisRsc->BaseCtrlData,
								 AxisRsc->SvControlIf->CmdCtrl.CmdCtrlBit );
		AxisRsc++;
	}
#endif

	/*----------------------------------------------------------------------*/
	/* <S045> Make Output Signal				 							*/
	/*----------------------------------------------------------------------*/
	AxisRsc = AxisRscB;
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
//-		KpxOutputSeqSignals( AxisRsc->IoSignals );
		KpxOutputSeqSignals( ax_no, AxisRsc->IoSignals );		/* SGD7W IOI/O support<S118> */
		AxisRsc++;
	}

	/*----------------------------------------------------------------------*/
	/* Output Serial I/O for COA and FBA									*/
	/*----------------------------------------------------------------------*/
/* <S0CB>	Start */
	if( FnCmnCtrl->OutModeFlg == TRUE )	/* Forced output mode */
	{
		ForceOutSignals.dw = FnCmnCtrl->ForceOutSignalsHW;
		HALdrv_SetSOutB( 0, ForceOutSignals );
		HALdrv_ForceOutSignalsMask( );			/* <S198> */
	}
/* <S0CB>	End  */

	HALdrv_OutputSerialIO();

	/*----------------------------------------------------------------------*/
	/* Kick JL-086 WDT														*/
	/*----------------------------------------------------------------------*/
	pAsicHwReg = AxisRscB->SvAsicRegs->AsicHwReg;
	SHalDrv_KickWDT( pAsicHwReg, (AxisRscB->PowerManager)->MainPowerChecker.conf.TestMode4 );


	/*----------------------------------------------------------------------*/
	/*	Start DMA transfer to output the JL-077 uP cyclic data				*/
	/*----------------------------------------------------------------------*/
#if (CSW_MAICRO_DMA_TRANSFER != 0)
	DMAdrvStartMemoryTransfer(DMA_SVDEV_CH, DMA_SVDEV_OUT_LINK);
#endif

}



/****************************************************************************
  Description: Make ScanC Output values
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscB	: Pointer to Axis Handle
  Return		:
		none
*****************************************************************************/
static void	SysBaseMakeScanCMonitor( AXIS_HANDLE *AxisRscB )
{
	BASE_CTRL_HNDL		*BaseControls;
	BASE_LOOPCTRLS		*BaseLoops;		/* <S020> */
	CTRL_LOOP_OUT		*CtrlLoopOut;
	BASE_CTRL_OUT		*BaseCtrlOut;
//	CALC_WATTAGE 		*CalcWattage;	/* <S020> *//* <S039> */
	LONG				ax_no;
	AXIS_HANDLE			*AxisRsc;
//	LONG				DcVolt;			/* <S020> *//* <S039> */

#if 0 /* <S039> */
	/* Common processing */
#if (CSW_POWER_MONITOR_USE == TRUE) /* <S020> */
	/* Power consumption initialization(For multi-axis calculation) */
	ResetPowerConsumption( AxisRscB->CalcWattage );
	/* DCPower supply voltage acquisition */
	DcVolt = PcmGetDcVoltage( &(AxisRscB->PowerManager->PowerSts) );
#endif /* (CSW_POWER_MONITOR_USE == TRUE) */
#endif

	/* Each axis processing */
	AxisRsc	= AxisRscB;
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		BaseLoops = AxisRsc->BaseLoops;			/* <S020> */
		BaseControls= AxisRsc->BaseControls;
		CtrlLoopOut	= AxisRsc->CtrlLoopOut;
		BaseCtrlOut	= AxisRsc->BaseCtrlOut;
//		CalcWattage = AxisRsc->CalcWattage;		/* <S020> *//* <S039> */

		/* Update data for monitor */
		AxisRsc->UnMonitor->UnSpdRef = AxisRsc->SvControlIf->NetSpdRef;

		/* ScanB->ScanC */
		BaseCtrlOut->MotPos				= BaseControls->MotSts.MotPos;
//		BaseCtrlOut->FencPos			= BaseControls->CtrlCmdMngr.PosFbki;	/* <S04D>:Dell */
		BaseCtrlOut->FencPos			= BaseControls->MotSts.FencPos;			/* <S04D>:Add */
		BaseCtrlOut->PcmdCntr			= BaseControls->CtrlCmdMngr.PosRef;
		BaseCtrlOut->MotSpdSum			= BaseControls->MotSts.MotSpdSumB;
		BaseCtrlOut->MotSpdCnt			= BaseControls->MotSts.MotSpdCntB;
		BaseCtrlOut->OutputSpdRef		= BaseControls->CtrlCmdMngr.SpdRefo;
		BaseCtrlOut->PositionError		= BaseControls->PosManager.var.PosErrA.per32s;
		BaseCtrlOut->SvonSpdLmtReq		= AxisRsc->CtrlLoopOut->SvonSpdLmtFlag;
		BaseControls->MotSts.MotSpdSumB	= 0;
		BaseControls->MotSts.MotSpdCntB	= 0;
		BaseCtrlOut->CmdSeqBit			= AxisRsc->SvControlIf->CmdSeqBit;
		BaseCtrlOut->TblZclamp			= (BaseControls->TblDrive.var.TblZmd.b.zm == CTRLMODE_ZCLMP) ? TRUE : FALSE;

//		BaseCtrlOut->IdRefMon			= CtrlLoopOut->IdRefMon;
		BaseCtrlOut->IqRefMon			= CtrlLoopOut->IqRefMon;
		BaseCtrlOut->TrqRefMon 			= CtrlLoopOut->TrqRefMon;

		/* Command data output */
		BaseCtrlOut->TrqCtrlRef = AxisRsc->SvControlIf->NetTrqRef;
		BaseCtrlOut->SpdCtrlRef = BaseControls->CtrlCmdMngr.SpdRefi;
		BaseCtrlOut->CtrlModeOut = BaseControls->CtrlModeSet.CtrlMode;

#if 0 /* <S039> */
#if (CSW_POWER_MONITOR_USE == TRUE) /* <S020> */
		/* Power consumption monitor calculation */
		CalcPowerMoniter( CalcWattage, BaseLoops->PowMonVq, BaseLoops->PowMonIqd,
				BaseLoops->PowMonVd, BaseLoops->PowMonIdd, DcVolt, BaseControls->BeComplete );
#endif /* (CSW_POWER_MONITOR_USE == TRUE) */
#endif

		AxisRsc++;
	}

 /* Add <S039> */
	/* Common processing */
#if (CSW_POWER_MONITOR_USE == TRUE) /* <S020> */
	/* Averaging power consumption values */
	CalcPowerConsumptionAvr( AxisRscB->CalcWattage );
	/* Power consumption initialization(For multi-axis calculation) */
	ResetPowerConsumption( AxisRscB->CalcWattage );
	/* DCPower supply voltage acquisition */
//	DcVolt = PcmGetDcVoltage( &(AxisRscB->PowerManager->PowerSts) );
#endif
/* Add End <S039> */

}



/****************************************************************************
  Description: Make Network Trigger Request		<S012>
-----------------------------------------------------------------------------
  Parameters	:
		AXIS_HANDLE *AxisRscB	: Pointer to Axis Handle
  Return		:
		none
*****************************************************************************/
static UCHAR GetNetworkTriggerSts( AXIS_HANDLE *AxisRscB )
{
	UCHAR				ret;
	LONG				ax_no;
	AXIS_HANDLE			*AxisRsc;

	ret = FALSE;
	AxisRsc	= AxisRscB;
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
//		ret |= (AxisRsc->SvControlIf->CmdCtrl.CmdCtrlBit >> TRACE_TRIGGER_BITNO); /* <S021> */
		ret |= ((AxisRsc->SvControlIf->CmdCtrl.CmdCtrlBit & TRACE_TRG_BIT) >> TRACE_TRIGGER_BITNO);
	}
	return ret; /* <S021> */
}

/****************************************************************************************************/
/*																									*/
/*		<S045> Scan B sequence signal input processing														*/
/*																									*/
/****************************************************************************************************/
void KpxInputSeqSignals( SEQ_IO_HNDL *IoSignals )
{
#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE /* <S04B> */
	HWSIN			SeqInputSignals;

		SeqInputSignals = HALdrv_ReadInputSignals();				/* Read Input SIgnals 			*/
/*--------------------------------------------------------------------------------------------------*/
/*		 Sequence signal input(ScanB)																	*/
/*--------------------------------------------------------------------------------------------------*/
		SEQIO_START_INSIGX( IoSignals );							/* Start Input Signals */
/*--------------------------------------------------------------------------------------------------*/
		SEQIO_GET_INSIGX( PclSignal, SeqInputSignals.dw );			/* P-CL */
		SEQIO_GET_INSIGX( NclSignal, SeqInputSignals.dw );			/* N-CL */
		SEQIO_GET_INSIGX( DecSignal, SeqInputSignals.dw );			/* DEC */
		SEQIO_GET_INSIGX( ExtSignal[0], SeqInputSignals.dw );		/* EXT1 */
		SEQIO_GET_INSIGX( ExtSignal[1], SeqInputSignals.dw );		/* EXT2 */
		SEQIO_GET_INSIGX( ExtSignal[2], SeqInputSignals.dw );		/* EXT3 */
		SEQIO_GET_INSIGX( FstpSignal, SeqInputSignals.dw);			/* FSTP */ /* <S1F5> */
/*--------------------------------------------------------------------------------------------------*/
		SEQIO_END_INSIGX( );										/* End Input Signals */
/*--------------------------------------------------------------------------------------------------*/
#endif /* <S04B> */
		return;
}


/****************************************************************************************************/
/*																									*/
/*		<S045> Scan B sequence signal output processing														*/
/*		TODO: APtype																*/
/*																									*/
/****************************************************************************************************/
//- void KpxOutputSeqSignals( SEQ_IO_HNDL *IoSignals )
void KpxOutputSeqSignals( LONG ax, SEQ_IO_HNDL *IoSignals )		/* SGD7W IOI/O support<S118> */
{
//<S081> #if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE /* <S04B> */
HWSOUT OutX; OutX.dw = 0;
/*--------------------------------------------------------------------------------------------------*/
		SEQIO_START_OUTSIGX( IoSignals );			/* Start Output Signal */
/*--------------------------------------------------------------------------------------------------*/
		SEQIO_SET_OUTSIGX( NearSignal, OutX.dw );	/* /NEAR */	/* <S1D4> */
		SEQIO_SET_OUTSIGX( CoinSignal, OutX.dw );	/* /COIN */
#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE /* <S081> */
		SEQIO_SET_OUTSIGX( CltSignal,  OutX.dw );	/* /CLT */
		SEQIO_SET_OUTSIGX( VltSignal,  OutX.dw );	/* /VLT */
//		SEQIO_SET_OUTSIGX( NearSignal, OutX.dw );	/* /NEAR */	/* <S1D4> */
#endif /* <S081> */
/*--------------------------------------------------------------------------------------------------*/
//-		SEQIO_END_OUTSIGX_ScanB( OutX );			/* End Output Signal */
		SEQIO_END_OUTSIGX_ScanB( ax, OutX );			/* End Output Signal */		/* SGD7W IOI/O support<S118> */
/*--------------------------------------------------------------------------------------------------*/
//<S081> #endif /* <S04B> */
		return;
}

/**************************** end of file **********************************/

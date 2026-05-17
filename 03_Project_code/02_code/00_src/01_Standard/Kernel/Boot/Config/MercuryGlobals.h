#ifndef GROBAL_DATA_H
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MercuryGlobals.h : Global data definition														*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.05.11	Y.Oka															*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#if 0 /* 2012.08.186 Y.Oka Make Mercury Base Software */
#include "BaseControls.h"
#include "SequenceControls.h"
#include "BaseEnableSequence.h"
#include "PrmAccessIf.h"
#include "FunManager.h"
#include "MpfHandler.h"
#include "PdetABSScale.h"
#include "AutoNotchSet.h"
#include "TuneLib.h"
#include "FnMfcTuning.h"
#include "FnVibMonitor.h"
#include "FnInitVibDetLvl.h"
#include "FnOnePrmTuning.h"
#include "FnAtypeVibTune.h"
#include "FnEasyFFT.h"
#include "DataTraceManager.h"
#include "AdvancedAutoTuning.h"
#include "NetworkIf.h"
#else /* 2012.08.18 Y.Oka Make Mercury Base Software */
#include "NetworkIf.h"
#include "HwDeviceIf.h"
#include "BaseControls.h"
#include "SequenceIo.h"
#include "BaseEnableSequence.h"
#include "FunManager.h"
#include "FnMfcTuning.h"
#include "FnVibMonitor.h"
#include "FnInitVibDetLvl.h"
#include "FnOnePrmTuning.h"
#include "FnAtypeVibTune.h"
#include "FnEasyFFT.h"
#include "FnSysConfig.h" /* <S0BE> */
#include "AutoNotchSet.h"
#include "AdvancedAutoTuning.h"
#include "PdetABSScale.h"
#include "JL086ServoIpHw.h"
#include "EepromIF.h"
#include "PrmAccessIf.h"
#include "UnitPrmStruct.h"
#include "AlarmManager.h"
#include "OverheatManager.h"
#include "Pls.h"									/* <S00C> Command Pulse Counter Driver			*/
#include "Adc.h"									/* <S00C> A/D Converter Driver					*/
#include "I2cDriver.h"									/* <S00C> EEPROM Driver							*/
#include "XlsAmonDef.h"		/* <S016>Temporary measure */
#include "SvidDef.h"		/* <S031> Erase after */
#include "DetectTemperature.h"	/* <S03A> */
#include "SvOptCardIf.h"							/* <S04D> Servo Option Card If					*/
#include "PnlOp.h"
#include "LifeTimePredictionApi.h"	/* <S154> */
#include "AlmLatch.h"				/* <S154> */
#include "MotorLessTestStr.h"						/* <S1B7> Motor Less Test Mode					*/
#endif /* 2012.08.18 Y.Oka Make Mercury Base Software */


/****************************************************************************************************/
/*		STRUCT DEFINITION(Unit unit)																*/
/****************************************************************************************************/
/* For processing time measurement */
typedef struct _TASK_MEASURE_TIME {
	ULONG	SA_maxTime;		/* ScanAMaximum processing time */
	ULONG	SA_crtTime;		/* ScanACurrent processing time */
	ULONG	SA_sumTime;		/* ScanA2msCumulative processing time	<S043> */
	ULONG	SAinB_exeTime;		/* ScanBExecutingScanAExecution count	<S043> */
	ULONG	SAinC_exeTime;		/* ScanCExecutingScanAExecution count	<S043> */
	ULONG	SB_maxTime;		/* ScanBMaximum processing time */
	ULONG	SB_crtTime;		/* ScanBCurrent processing time */
	ULONG	SB_sumTime;		/* ScanB2msCumulative processing time	<S043> */
	ULONG	SB_exeflg;		/* ScanBRunning flag	<S043> */
	ULONG	SBinC_exeTime;		/* ScanCExecutingScanBExecution count	<S043> */
	ULONG	SC_maxTime;		/* ScanCMaximum processing time */
	ULONG	SC_crtTime;		/* ScanCCurrent processing time */
	ULONG	SC_exeflg;		/* ScanCRunning flag	<S043> */
	ULONG	crtTimePer2ms;	/* 2msaroundScanA,ScanB,ScanCOccupied processing time */
	ULONG	maxTimePer2ms;	/* 2msaroundScanA,ScanB,ScanCMaximum occupied processing time */
	ULONG	ScanSumTime;	/* work */

	ULONG	SCtask_counter;
	ULONG	SCtask_wakeup_time;
	ULONG	SBcountInMS;
	ULONG	SBCount;
	ULONG	SBCountInc;
	ULONG	SACount;		/* ScanANumber of executions	<S043> */
} TASK_MEASURE_TIME;

#define	SCANA_EXE_CNT_IN_2MS	(2000000/KPI_SACYCLENS)		/* 2msinsideScanANumber of executions	<S043> */
#define	SCANB_EXE_CNT_IN_2MS	(2000000/KPI_SBCYCLENS)		/* 2msinsideScanBNumber of executions	<S043> */
#define	SCANC_EXE_CNT_IN_2MS	(2/KPI_SCCYCLEMS)			/* 2msinsideScanCNumber of executions	<S043> */

/* Unit ID Information */
typedef struct MODULE_ID_INFO {
	EEP_HEADER_INFO_T	*IfInfo;		/* IF IDInformation */
	EEP_HEADER_INFO_T	*CoaInfo;		/* COA IDInformation */
	EEP_HEADER_INFO_T	*MbuInfo;		/* MBU IDInformation */
	EEP_HEADER_INFO_T	SdaInfo;		/* SDA IDInformation */
//<S053>	HW_AXIS_MAP			*AsicNoMap;		/* Axis number andJL077ASICMap information with the number head  */
} MODULE_ID_INFO;


/****************************************************************************************************/
/*		STRUCT DEFINITION(Intra-unit axis-to-axis interface)		Supports motor carrier frequency switching<S117>								*/
/****************************************************************************************************/
/*  */
#if 0	/* Unnecessary because carrier frequency is not unified<S11E> */
#if ( defined(CSW_SGD7W_TYPE) || defined(CSW_COMMON_CARRIER) )
typedef struct AXIS_IF {
	SHORT	pwm_ax[MAX_AXIS_NUMBER];		/* Each axis settingPWMCarrier frequency							*/
	SHORT	pwm_unit;						/* Overall settingsPWMCarrier frequency							*/
}AXIS_IF;
#endif	/* #if ( defined(CSW_SGD7W_TYPE) || defined(CSW_COMMON_CARRIER) ) */
#endif		/*#if 0	 Unnecessary because carrier frequency is not unified<S11E> */



/****************************************************************************************************/
/*		STRUCT DEFINITION(Axis unit)																	*/
/****************************************************************************************************/
typedef	struct AXIS_HANDLE {
	LONG				SlotNo;				/* Axis number(=MBBoard slot number)							*/
	LONG				AxisNo;				/* Axis address(Logic axis number)								*/

	/* Hardware abstraction layer */
	MODULE_ID_INFO		*ModuleID;			/* Module ID information */
	MENCV				*MencV;				/* Pointer to motor encoder data					*/
	MENCV				*FencV;				/* Fulque encoder data								*/
	ASICS				*SvAsicRegs;		/* ASICPointer to address table						*/
	SHAL_LATCH_STS_ADR	*LtStsAdr;			/* EXTFor latch signal 										*/
//	OH_INFO				*OhInfo;			/* For board temperature detection 										*//* <S03A> */
	PLSIFV 				*pPlsIfV;			/* <S00C> Command Pulse Interface						*/
	CADC				*pAdc;				/* <S00C> A/D Converter Interface						*/
	RWALLMD 			*pRwAllMng;			/* <S00C> Parameter Download							*/
	RWPDLMD 			*pRwPdlMng;			/* <S031> Lcd Parameter Copy							*/

	/* Base servo control data(ScanA) */
	BASE_LOOPCTRLS		*BaseLoops;			/* Control loop calculation data								*/
	CTRL_LOOP_OUT		*CtrlLoopOut;		/* ScanAOutput data										*/

	/* Base servo control data(ScanB) */
	BASE_CTRL_HNDL		*BaseControls;		/* Basic control shared data									*/
	SERVO_CONTROL_IF	*SvControlIf;		/* M3 I/FData											*/
	BASE_CTRL_OUT		*BaseCtrlOut;		/* ScanBOutput data										*/
	DETVIB				*DetVib;			/* Variables for vibration detection										*/
	USHORT				*TableRefBuffer;	/* Pointer to table operation command buffer					*/
	
	MOTSPDMAFIL			*MotSpdMafil;		/* Variable for moving average calculation for motor speed detection					*/

	PDET_ABSCONTROL		*PdetAbsSeq;		/* ABSScale magnetic pole detection variable							*/
	MPFIND				*MpFind;			/* Magnetic pole detection operation variable									*/
	
	/* Servo sequence control data(ScanC) */
	SEQ_IO_HNDL			*IoSignals;			/* SequenceI/O */
	SEQ_CTRL_OUT		*SeqCtrlOut;		/* ScanCOutput data										*/
	BE_SEQ_HNDL			*BeSequence;		/* Base Enable Sequence Handle							*/
	CHECK_ALARM			*CheckAlarm;		/* Structure for alarm detection processing								*/
	SEQ_MOT_STS			*SeqMotSts;			/* ScanCMotor status					*/
	REMVIBFREQ			*RemVibFreq;		/* Residual vibration frequency monitor data							*/
	CALC_WATTAGE		*CalcWattage;		/* <S020> Data for power consumption calculation(Each axis)					*/
	ENVMON				*EnvMonitor;		/* Installation environment monitor data(Each axis)							*//* <S0ED> */
	POWER_MONITOR		*PowerMonitor;		/* <S020> Power consumption monitor variable(Common to units)				*/
	POW_MNG_HNDL		*PowerManager;		/* Power management data(Common to units)						*/
	CHECK_OH			*OverHeat;			/* Variable for overheat check(Common to units)			*//* <S0ED> */
	LIFETIME_HANDLE		*hLifeTime;			/* Lifetime prediction function											*//* <S154> */
	DBLIFETIME_HADNLE	*hDBLifeTime;		/* DBLifetime prediction function										*//* <S180> */
	ALMLATCH_HNDL		*hAlmLatchHndl;		/* Alarm latch data								*//* <S154> */
#if (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE)
	PNLOP_DATA_HANDLE	*PnlOpLedDataHndl;	/* Paneope structure for network							*//* <S0A1> */
#endif

	/* Network */
#if (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE)/* AnalogIFShould I also change it to use the structure below?			*/
	NETIF_AXIS_HANDLE	*NetAxisIf;			/* Network Interface Handle for each axis 				*/
	NETIF_SYS_HANDLE	*NetNodeIf;			/* Network Interface Handle for node device 			*/
#endif
	NETMSG_IF			*NetMsgIf;			/* Network Messages IF									*/

	/* Option module	*//* <S04D> */
	OPTCARD_HNDL		*OptCardHdl;		/* Pointer to option module structure				*/
	
	/* Parameter data */
	REG_MANAGER_HANDLE	*RegManager;		/* Register Manager */
	UNI_PRM_STR			*UniPrms;			/* Pointer to parameter handle						*/
	ALMMAN_HANDLE		*AlmManager;		/* Alarm control structure									*/

	/* FnFunction data */
	FUN_CMN_CONTROL		*FnCmnCtrl;			/* FnPointer to function processing data						*/
	FUN_MANAGER			*FnManager;			/* Fn ManagerHandle									*/
	FUN_AXCOMMON		*FnAxCommons;		/* Fn Manager						*/

	/* Utility Functions */
	ANOTCHSEQ			*ANotchSeq;			/* Structure for automatic notch sequence							*/
	AUTONOTCH			*ANotch;			/* Structure for automatic notch setting								*/
	FFTANALYZE			*FftAna;			/* Vibration frequency analysis(Online FFT)					*/
	MFCTUNE				*MfcTune;			/* Model gain adjustment structure								*/
	VIBMON				*VibMon;			/* Structure for online vibration monitor							*/
	INIVIBDET			*IniVibDet;			/* Vibration detection level initialization structure							*/
	ADATHNDL			*AdatHndl;			/* Advanced Auto Tuning	Handle							*/
	ONEPRMTUNE			*OnePrmTune;		/* Structure for one-parameter tuning					*/
	AVIBTUNE			*AvibTune;			/* AMold vibration damping control adjustment structure								*/
	EASYFFT				*EasyFFT;			/* Easy FFT										*/
	OPTCLR				*OptClr;			/* Option card detection clear structure					*//* <S0BE> */

	TEMP_MANAGER		*TempManager;		/* Pointer to temperature detection data							*//* <S03A> */
	ANLG_UN_MON_DATA	*UnMonitor;			/* Amon/UnPointer to monitor data						*/
	TASK_MEASURE_TIME	*TaskProcessTime;	/* For processing time monitor */

#if 0	/* Unnecessary because carrier frequency is not unified<S11E> */
#if ( defined(CSW_SGD7W_TYPE) || defined(CSW_COMMON_CARRIER) )			/* Supports motor carrier frequency switching<S117> */
	AXIS_IF				*AxisIf;			/* Unit internal axis interface structure			*/
#endif	/* #if ( defined(CSW_SGD7W_TYPE) || defined(CSW_COMMON_CARRIER) ) */
#endif		/*#if 0	 Unnecessary because carrier frequency is not unified<S11E> */

	/* Data trace related *///kira_101027
	TRCHNDL				*TrcHndl;			/* Data trace handle */
	USHORT				*TraceBuffer;		/* Pointer to data trace buffer */

	/* Analog monitor related *//* <S016> Temporary measure */
	AMONOUT				*AmonOut;			/* Pointer to analog monitor output information	*/
	UCHAR				*ScanB250us;		/* ScanB 250us Flag				*/

	/* Motorless test mode related	*//* <S1B7> */
	MOTLESSTST			*MotorLessTest;		/* Motorless testIFPointer to definition					*/

#if 0 /* 2012.08.06 Y.Oka Make Mercury Base Software */
	/* Application */
	TRIGIO_INFO			*TrigInfo;			/* TrigIOControl data 									*/
#endif /* 2012.08.06 Y.Oka Make Mercury Base Software */

#if defined(_VC_DEBUG_) /* 2012.08.23 Y.Oka Make Mercury Base Software */
	UCHAR				*pDummyEeprom;
#endif /* #if defined(_VC_DEBUG_) */ /* 2012.08.23 Y.Oka Make Mercury Base Software */
	
	OPACCSV 			*pAccessOp;			/* <S166> Operator Access Control */

} AXIS_HANDLE;


void* GetAxisHandle(UINT axis_number);
AXIS_HANDLE *InitializeAxisGlobals( void );


extern	LONG	NumberOfAxes;				/* Number of axes setting						*/
#if defined(_TEST_BOARD_) && !defined(_VC_DEBUG_)
extern	LONG	ScanAStartCtr;						/* ScanAProcessing counter							*/
extern	LONG	ScanBStartCtr;						/* ScanBProcessing counter							*/
extern	LONG	ScanCStartCtr;						/* ScanCProcessing counter							*/
#endif
extern DWORDX	ReadDummy;					/* <S074> Dummy Variable for Read */

#define GROBAL_DATA_H
#endif /* GROBAL_DATA_H */

//#define _INFO_REG_CHECK_	/* Compile switch for model information register */	/* <S061> */


/***************************************** end of file **********************************************/

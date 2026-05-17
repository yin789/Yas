#ifndef	_COMMON_STR_H /* For reload prevention */
//****************************************************************************************************
//
//
//		BaseCmnStruct.h : Local Header File for Common Structure
//
//
//****************************************************************************************************
//
//
//************** Copyright (C) Yaskawa Electric Corporation ******************************************
#include "ServoIf.h"

/*--------------------------------------------------------------------------------------------------*/
/*		Basic control mode definition : The upper byte must be specified as zero											*/
/*--------------------------------------------------------------------------------------------------*/
#define	CTRLMODE_SPD		0x0000			/* Speed control mode(Analog command)							*/
#define	CTRLMODE_POS		0x0001			/* Position control mode(Pulse train command)							*/
#define	CTRLMODE_TRQ		0x0002			/* Torque control mode(Analog command)						*/
#define	CTRLMODE_JOG		0x0003			/* JOG operation mode										*/
#define CTRLMODE_ZSRCH		0x0004			/* Origin search mode										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Extension control mode definition : Lower byte should specify the basic control mode								*/
/*--------------------------------------------------------------------------------------------------*/
#define CTRLMODE_CSPD		0x0100			/* Internal setting speed control mode(Contact command)						*/
#define CTRLMODE_PJOG		0x0101			/* Program JOG operation mode							*/
#define CTRLMODE_NOCMD		0xFFFF			/* Control mode not specified(ScanCDedicated)						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Zero control mode definition																			*/
/*--------------------------------------------------------------------------------------------------*/
#define CTRLMODE_ZNONE		0x00			/* No zero control											*/
#define CTRLMODE_ZSTOP		0x01			/* Zero speed stop mode										*/
#define CTRLMODE_LDSTOP		0x02			/* Deceleration stop mode by linear deceleration						*/ /* <S1F5> */
#define CTRLMODE_ZCLMP		0x03			/* Zero clamp mode									*/ /* <S1F5> */
// #define CTRLMODE_ZCLMP		0x02			/* Zero clamp mode									*/ /* <S1F5> */
/*--------------------------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------------------------*/
/*		Control mode definition																				*/
/*--------------------------------------------------------------------------------------------------*/
#ifdef __BIG_ENDIAN__
typedef union CTRLMD {						/* Control mode structure definition (4byte)						*/
	struct	{								/* Control mode member definition									*/
		UCHAR	ot;							/* OT generation flag										*/
		UCHAR	zm;							/* Zero control mode										*/
		UCHAR	cx;							/* Extension control mode										*/
		UCHAR	cm;							/* Basic control mode										*/
	} b;
	/*----------------------------------------------------------------------------------------------*/
	ULONG	dw;								/* For control mode editing(Copy,etc)							*/
	struct  {
		USHORT	uwd_h;						/* For control mode editing(Copy,etc)							*/
		USHORT	uwd_l;						/* For control mode editing(Copy,etc)							*/
	} us;
} CTRLMD;
#else  /* Little Endian */
typedef union CTRLMD {						/* Control mode structure definition (4byte)						*/
	struct	{								/* Control mode member definition									*/
		UCHAR	cm;							/* Basic control mode										*/
		UCHAR	cx;							/* Extension control mode										*/
		UCHAR	zm;							/* Zero control mode										*/
		UCHAR	ot;							/* OT generation flag										*/
	} b;
	/*----------------------------------------------------------------------------------------------*/
	ULONG	dw;								/* For control mode editing(Copy,etc)							*/
	struct  {
		USHORT	uwd_l;						/* For control mode editing(Copy,etc)							*/
		USHORT	uwd_h;						/* For control mode editing(Copy,etc)							*/
	} us;
} CTRLMD;
#endif /* __BIG_ENDIAN__ */


/*--------------------------------------------------------------------------------------------------*/
/*		Zero control mode definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct ZCTRMD {						/* Zero control mode structure definition (4byte)					*/
	UCHAR	zm;								/* Zero control mode										*/
	UCHAR	zmchg;							/* Zero control mode change information								*/
	UCHAR	spare0;							/* Preliminary													*/
	UCHAR	spare1;							/* Preliminary													*/
} ZCTRMD;


typedef struct CTRLMD_SET {					/* Control mode related definitions									*/
	CTRLMD			CtrlMode;				/* Control Mode											*/
	CTRLMD			CtrlModeLst;			/* Last Control Mode									*/
	CTRLMD			CtrlModeChg;			/* Control Mode Change Info (XOR)					*/
	ZCTRMD			ZctrlMode;				/* Zero Control Mode									*/
} CTRLMD_SET;



/*--------------------------------------------------------------------------------------------------*/
/*	Speed detection moving average filter		<V110>															*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MOTSPDMAFIL {
	KSGAIN	mabuf[16];			/* Moving average buffer	[2^24/OvrSpd]						<S00A>	*/
	LONG	mabufSumx;			/* Moving average buffer integrated value											*/
	LONG	idx;				/* Buffer index												*/
	LONG	manumBit;			/* Number of moving averages(2to the power of)											*/
	LONG	defaultNumBit;		/* Number of moving averages(2to the power of)initial value of									*/
	LONG	ksub;				/* Subtraction amount calculation gain(0/1)											*/
} MOTSPDMAFIL;


/*--------------------------------------------------------------------------------------------------*/
/*		Kernel Mode Definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef enum FUN_KNLMD_T {
	KPI_KMD_NORMAL		  = 0x00,		/* Normal mode												*/
	KPI_KMD_ADJREFAD		,			/* Adjusting command AD											*/
	KPI_KMD_ADJCURAD		,			/* Current detection AD adjusting										*/
	KPI_KMD_SOFTRESET		,			/* Soft resetting											*/
	KPI_KMD_PARAMRESET		,			/* Parameter reset in progress										*/
	KPI_KMD_PARAMCPY		,			/* <S031> Parameter copy									*/
	KPI_KMD_ADJANLGMON		,			/* Analog monitor adjusting										*/	/* <S0A9> */
	KPI_KMD_PBLEDTEST		,			/* Push button & 7SegLedConfirmation test in progress						*/	/* <S0CB>  */
	KPI_KMD_ADJGAIN			,			/* During gain adjustment												*/	/* <S0CB>  */
	KPI_KMD_ANLGMONVOLTTEST	,			/* Analog monitor output voltage test in progress								*/	/* <S0FD>  */
	KPI_KMD_RIPPLECOMPWRITE	,			/* Speed ripple compensation information writing in progress							*/	/* <S14B>  */
} FUN_KNLMD_T;


/*--------------------------------------------------------------------------------------------------*/
/*		FnFunction common Servo control data																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct FUN_CMN_CONTROL {
	volatile BOOL	FnSvControl;			/* ROUNDDriving privilege											*/
	LONG			FnCtrlMcmd;				/* ROUNDControl mode during operation						*/
	BOOL			FnSvonReq;				/* ROUNDServo-on request during operation							*/
	BOOL			FnSensOnReq;			/* ROUNDServo-on request during operation							*/
	BOOL			PdetReq;				/* Magnetic pole detection request(Request energization)								*/
	BOOL			forcedAlmReady;			/* ForcedAlmReadyFlag									*//* <S215> */
	LONG			JogSpeed;				/* JOGSpeed command +:Forward rotation,-:Reverse running							*/
	USHORT			SafeStopTime;			/* JOGSafety stop time										*/
	SHORT			OperatorSts;			/* Operator display status		*//* <S085> */
	BOOL			SvOnPdetReq;			/* Servo-on magnetic pole detection request					 	<S21C> 	*/

	USHORT			ForceOutSignals;		/* Forced signal output command										*/
	USHORT			ForceOutSignalsHW;		/* Forced signal output command(Reflected in HW output signal)					*/	/* <S0CB> */
	UCHAR			OutModeFlg;				/* Forced output mode flag									*/	/* <S0CB> */

	ULONG			LightIntervalTime;		/* 7SegLedLighting time[100ms]								*/	/* <S0CB> */
	UCHAR			PbAndLedChkTestFlag;	/* Push button&7SegLedConfirmation flag						*/	/* <S0CB> */
	USHORT			AnlgLedChkInitFlag;		/* APShape Push button&7SegLedConfirmation start flag			*/	/* <S0CB> */
	UINT			AnlgLedChkCnt;			/* APShape Push button&7SegLedConfirm Counter				*/	/* <S0CB> */
	UINT			AnlgLedChkCnt2;			/* APShape Push button&7SegLedConfirm Counter(Left bridgeLED)	*/	/* <S0CB> */
	UINT			AnlgLedChkLstPnlBtn;	/* APShape Push button&7SegLedConfirm Previously pressed button		*/	/* <S0CB> */
	CHAR			AnlgMonVoltTstFlag;		/* Analog monitor output voltage confirmation test flag				*/	/* <S0FD> */
	USHORT			AnlgMonVoltTstChSel;	/* Analog monitor output voltage confirmation test ChSelection				*/	/* <S0FD> */
	SHORT			OutputVoltVal_Ch0;		/* Analog monitor output voltage confirmation test Output voltage setting(Ch0)		*/	/* <S0FD> */
	SHORT			OutputVoltVal_Ch1;		/* Analog monitor output voltage confirmation test Output voltage setting(Ch1)		*/	/* <S0FD> */
	FUN_KNLMD_T		KnlMode;				/* Tester mode											*/	/* <S0CB> */

	//Origin search operation register definition location is hereOK? ?
	struct {
		USHORT	ZsrchState;					/* 0x2101 : 0x030: Origin search complete						*/
	} OpeReg;

	struct {
		BOOL		FnSvCtrlDisable;		/* FnOperation disabled state										*/
		BOOL		LockNormalOpeDrive;		/* FnNormal operation prohibition flag after operation							*/

		BOOL		TuningRun;				/* Servo adjustment in progress											*/
		BOOL		TuningPctrlRun;			/* Servo adjustmentPUnder control									*/
	} State;

	struct {								/* <S031> Flags */
		UCHAR PrmCopy;						/* <S031> Lcd Operator Parameter copy flag */
		UCHAR Reserved0;					/* <S031> reserved */
		UCHAR Reserved1;					/* <S031> reserved */
		UCHAR Reserved2;					/* <S031> reserved */
	} f;
} FUN_CMN_CONTROL;


/*--------------------------------------------------------------------------------------------------*/
/*		Control loop output data structure																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CTRL_LOOP_OUT {
//	LONG			MotPos;					/* Motor position						[Pulse]				*/
	LONG			MotPosErr;				/* Position amplifier deviation					[Pulse]				*/
	LONG			SpdRefo;				/* Speed command output						[2^24/OvrSpd]		*/
	LONG			SpdFFC;					/* SpeedFFCompensation						[2^24/OvrSpd]		*/
	LONG			MotSpd_a;				/* Motor speed average value					[2^24/OvrSpd]		*//* 2013/04/01VersionMD, it is commented out.	*/
	LONG			TrqRefo;				/* Torque command output					[2^24/MaxTrq]		*/
	LONG			TrqRefo_a;				/* Torque command output(Average value)			[2^24/MaxTrq]		*/
	LONG			TrqRefMon;				/* Torque command monitor					[2^24/MaxTrq]		*/
	LONG			TrqRefMon_a;			/* Torque command monitor					[2^24/MaxTrq]		*/
	SHORT			IdRefMon;				/* dAxis current command monitor				[15000/Imax]		*/
	SHORT			IqRefMon;				/* qAxis current command monitor				[15000/Imax]		*/
	SHORT			IqRefMon_a;				/* qAxis current command monitor(Average value)		[15000/Imax]		*/
	SHORT			IqRefMon_l;				/* qAxis current command monitor(Previous value)		[15000/Imax]		*/
	LONG			IqFbMon;				/* qAxis currentFBMonitor										*/
	LONG			IdFbMon;				/* dAxis currentFBMonitor										*/
	LONG			IqRefFilMon;			/* qAxis current command monitor(After filter)						*/
//	BOOL			PConActFlg;				/* PControl state confirmation flag									*//* <S064> */
	BOOL			SvonSpdLmtFlag;			/* ServoONHourly speed limit flag							*/
//	BOOL			SpdClamp;				/* Speed limit flag during torque control							*//* <S04B> */
	BOOL			TrqClamp;				/* Torque limit flag									*/
} CTRL_LOOP_OUT;


/*--------------------------------------------------------------------------------------------------*/
/*		Base control output data structure																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct BASE_CTRL_OUT {
	CTRLMD			CtrlModeOut;			/* Control mode(ScanBOutput)								*/
	LONG			TrqCtrlRef;				/* Torque command during torque control				[2^24/MaxTrq]	*/
	LONG			SpdCtrlRef;				/* Speed command during speed control					[2^24/OvrSpd]	*/
	LONG			OutputSpdRef;			/* Speed command output value(ScanB→ScanC)			[2^24/OvrSpd]	*/
//	LONG			SpdRefSum;				/* Speed command output value(Ref+FF)				[2^24/OvrSpd]	*/
	LONG			MotPos;					/* Motor position(ScanBAccumulated value)				[32bitRingPulse]*/
	LONG			FencPos;				/* Motor position(ScanBAccumulated value)				[32bitRingPulse]*/
	LONG			PcmdCntr;				/* Command pulse count value									*/
	LONG			MotSpdSum;				/* Motor speed addition value(ScanCFor average speed calculation)				*/
	LONG			MotSpdCnt;				/* ScanCFor average speed calculationScanBCounter						*/
	LONG			PositionError;			/* Position deviation												*/
	LONG			SvonSpdLmtReq;			/* ServoONHourly speed limit request								*/
	LONG			CmdSeqBit;				/* Sequence control command(ScanB→ScanC)						*/
	BOOL			TblZclamp;				/* Table operation zero clamp flag						*/
	LONG			IdRefMon;				/* dAxis current command monitor				[15000/Imax]		*/
	LONG			IqRefMon;				/* qAxis current command monitor				[15000/Imax]		*/
	LONG			TrqRefMon;				/* Torque command monitor					[2^24/MaxTrq]		*/
} BASE_CTRL_OUT;


typedef	struct SEQ_CTRL_OUT {
/* Request commands to the Control module */
	CTRLMD			CtrlModeReq;			/* ScanC Control ModeRequest								*/
	VBOOL			BaseEnableReq;			/* ScanCCreation base enable request						*//* <S193> */
	BOOL			BeComplete;				/* Base enable completion status								*/
	BOOL			UdlBeStatus;			/* Base enable completion status(UDLside status monitor)			*/
	BOOL			BBSvcRefClrReq;			/* Servo Control Ref. Clear Request Flag				*/
	BOOL			TrqLimitReq;			/* OTTorque limit request(OTSequence processing output)				*/
	LONG			ModeSwAcc;				/* Motor Acceleration fow ModeSw		[NorSpd/ScanC]	*/

/* Sequence Status */
	BOOL			HwbbSts;				/* HWBBStatus												*/
	BOOL			FstpSts;				/* FSTPStatus												*/ /* <S1F5> */
	BOOL			MainPowerOn;			/* Main circuitONStatus											*/
	BOOL			MotStop;				/* Motor stop status										*/
	BOOL			OverTrvlSts;			/* OTStatus(OTSequence processing output)							*/
	BOOL			SvRdyPhaseIgnore;		/* Servo Ready (Ignore Phase)							*/

	BOOL			PotSigSts;				/* Forward rotation drive prohibited state										*/
	BOOL			NotSigSts;				/* Reverse drive prohibited state										*/
	BOOL			BrkSigSts;				/* Brake released state										*/
	BOOL			EstpSigSts;				/* E-STOPStatus											*/

	BOOL			TgOnSts;				/* Motor rotation status										*/
	BOOL			VcmpSts;				/* Speed match state											*/
	BOOL            MotSuspLvl;                /*Motor Suspend Level Flag <S201> */
	BOOL			RndReady;				/* 2013/04/01MDversion,RndReadyis not available. Temporarily,	*/

	LONG			RspSeqBits;				/* Sequence control state(ALM/WRN/SVON/SENON/PDET/BK/SRDY/MRDY)*/
	LONG			RspSigBits;				/* Input signal status(HWBB/P-OT/N-OT)							*/
	LONG			RspCtrlBits;			/* Control state(HBB/TGON/VCMP)								*/
/*--------------------------------------------------------------------------------------------------*/
	struct 	{								/* AP	<S00C>											*/
		UINT	Vrefon              :1;		/* 0x10 : Speed  Reference Input On						*/
		UINT	Trefon              :1;		/* 0x11 : Torque Reference Input On						*/
		UINT	Prefon              :1;		/* 0x12 : Pulse  Reference Input On						*/
		UINT	PerrClr             :1;		/* 0x13 : PosErr Clear     Input On						*/
		UINT	PrefSign            :1;		/* 0x14 : Pulse  Reference Sign  (plus:1, minus:0)		*/
	}f;
/*--------------------------------------------------------------------------------------------------*/
	UINT	AlmRstLast;						/* Alarm reset signal previous value @@ Move somewhere			*/
/*--------------------------------------------------------------------------------------------------*/
} SEQ_CTRL_OUT;


typedef	struct ANLG_UN_MON_DATA {
/* Interim processing 2013/04/01MDis out of comments.	*/
	LONG			AmonSpdRef;				/* AnaMon : Speed command(Speed FF included)	[2^24/OvrSpd]		*/
	LONG			AmonTrqRef;				/* AnaMon : Torque command(After gravity compensation)	[2^24/MaxTrq]		*/
/* Interim processing 2013/04/01MDis out of comments.	*/

	LONG			UnMotSpd;				/* Un000  : Motor speed				[pulse/64ms]		*/
	LONG			UnMotSpdx;				/* Un000  : Motor speed(2msAverage speed)	[2^24/OvrSpd]		*/
	LONG			UnPfbk64ms;				/* Un000  : Motor speed				[pulse/64ms]		*/
	LONG			UnSpdRef;				/* Un001  : Speed command				[2^24/OvrSpd]		*/
	LONG			UnTrqRef;				/* Un002  : Torque command				[2^24/MaxTrq]		*/
	LONG			UnMotAngle;				/* Un003  : Rotation angle 1(Pulse)		[pulse]				*/
	LONG			UnMotPhase;				/* Un004  : Rotation angle 2(Electrical angle)		[deg]				*/
	LONG			UnPcmd64ms;				/* Un007  : Command pulse speed			[pulse/64ms]		*/
	LONG			UnIsqrSum;				/* Un009  : Cumulative load rate				[(15000/MaxTrq)/10sec]*/
	LONG			UnMeanRgPower;			/* Un00A  : Regenerative power consumption average value		[%]					*/
	LONG			UnMeanDbPower;			/* Un00B  : DB average value of power consumption 		[%]					*/
	LONG			UnPcmdCntr;				/* Un00C  : Command pulse counter		[pulse]				*/
	LONG			UnPfbkCntr;				/* Un00D  : FB pulse counter		[pulse]				*/
	LONG			UnPfbkCntr2;			/* Un013  : FB pulse counter		[Command unit]			*//* <S056> */
	LONG			RemCntr2;				/* Un013  : FB pulse counter remainder[Command unit]			*//* <S056> */
	LONG			UnInstEnvMonAmp;		/* Un025 : Installation environment monitor(2sUpdate)	[0.001%]			*//* <S1D7> */
	LONG			UnInstEnvMonMot;		/* Un026 : Installation environment monitor(2sUpdate)	[0.001%]			*//* <S1D7> */
	ULONG			UnCrntDet;				/* Un141 : Squared current FB value			[(15000/MAXTRQ)^2]	*/
	ULONG			UnCrntRef;				/* Un142 : Square current command value			[(15000/MAXTRQ)^2]	*/
	LONG			UnRgPower;				/* Un143  : Regenerative power consumption value			[%]					*/
	LONG			UnDbPower;				/* Un144  : DB power consumption value 			[%]					*/
	ULONG			UnIsqrSumMax;			/* Un145 : Maximum cumulative load rate			[(15000/MaxTrq)/10sec]*/

	LONG			UnOLRate;				/* Un1F0  : Overload level rate			[%]					*/
	LONG			UnOLType;				/* Un1F1  : Overload type (0~4)		[]					*/
											/*			0: No overload 1:Motor continuous  2:Motor instantaneous  	*/
											/*						  3:Amplifier continuous  4:Amplifier instantaneous	*/
	LONG			TrcRgPower;				/* Trace  : Regenerative power consumption value			[%]					*/
	LONG			TrcDbPower;				/* Trace  : DB power consumption value 			[%]					*/

	LONG			UnPcmd64Sum;			/* Command pulse speed <S1B6>			[64ms]				*/

	USHORT			SvStatus;				/* Servo status monitor								*//* <S049> */
	USHORT			LedDispStatus;			/* Network7SegDisplay status						*//* <S0A1> */

} ANLG_UN_MON_DATA;


/***************************************** end of file **********************************************/
#define	_COMMON_STR_H
#endif

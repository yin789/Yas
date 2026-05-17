/****************************************************************************************************/
/*																									*/
/*																									*/
/*		UnitPrmStruct.h : Parameter related structure definition													*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#ifndef	_UNIT_PRM_STRUCT_H_
#define	_UNIT_PRM_STRUCT_H_


#include "PnPrmStruct.h"
#include "Bprm.h"


#if 0
/****************************************************************************************************/
/*																									*/
/*		Initial parameter definition : Parameters calculated in initialization process (Changes take effect when the power is turned back on.)					*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Initial parameter definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct IPRMDAT
{
/*--------------------------------------------------------------------------------------------------*/
//		BOOL	RvsDir;					/* Reverse rotation mode												*/
//		BOOL	FastCtrl;				/* Servo high-speed control selection										*/
//		BOOL	WrnOutSel;				/* Warning code output selection									*/
//		BOOL	SpdIctrlKeep;			/* Speed control integral hold											*/
//		BOOL	RegRxIn;				/* Built-in regenerative resistor												*/
//		BOOL	Dcbus;					/* DC power supply input selection											*/
//		BOOL	Ac3Phase;				/* Power input 3-phase												*/
//		BOOL	DetectAcOn2;			/* Power supply phase loss detection method											*/
//		BOOL	OhdetPort;				/* Heat sink overheating detection via port						*/
//		BOOL	DbTriack;				/* DB method triac										*/
//		BOOL	PerClrMecha;			/* Standards for position deviation clear correction(Correction position based on machine position)	*/
//		BOOL	AutoRun;				/* Automatic operation mode for test										*/
//		BOOL	BatWrnMode;				/* Use encoder battery voltage drop as a warning				*/
//		BOOL	TrefTrqFF;				/* Using torque command as torque FF(Analog pulse only)		*/
//		BOOL	TrefTrqLmt;				/* Using torque command as external torque limit						*/
//		BOOL	TrefTrqLmtCL;			/* Using torque command as external torque limit(P-CL,N-CLWhen enabled)	*/
//		BOOL	VrefSpdLmt;				/* Use speed command as external speed limit							*/
//		BOOL	PosConvMode;			/* Motor position command unit conversion mode setting							*/
//		BOOL	CtrlSW3Step;			/* Control switching selection setting										*/
//		BOOL	ComCycleFil;			/* Communication cycle moving average selection setting									*/
//		BOOL	IPSpdControl;			/* I-P control selection setting										*/
//		BOOL	CmdPDT;					/* Command prediction selection setting											*/
//		BOOL	VrefSpdFF;				/* Use speed command as speed FF								*/
//		BOOL	OvrSpdLimit;			/* Speed limit value has overspeed detection speed,Pn407		*/
//		BOOL	CtrlModeCspdPos;		/* SGDB compatible control mode switching: Contact speed/position control		*/
//		BOOL	CspdChangeCtrl;			/* SGDB compatible control mode switching: Contact speed/position, speed, torque control */
//		BOOL	EhVobs;					/* Phase compensation speed observer selection setting							*/
//		BOOL	DbOHeatCheck;			/* Selecting whether to perform DB overload detection processing due to temperature rise				*/
//		BOOL	G2AlmStopMode;			/* G2How to stop the alarm										*/
//		BOOL	Vobs;					/* Speed FB correction function setting										*/
//		BOOL	MpKvvFlg;				/* Main circuit voltage correction function selection	[0:OFF,1:ON]					*/
//		BOOL	TuningLess;				/* Adjustment-less setting												*/
//		BOOL	TuningLessVComp;		/* Adjustment-less phase lead compensation setting									*/
//		BOOL	HwbbSupport	;			/* HWBB function compatible selection										*/
//		BOOL	UvTrqLmtCtrl;			/* Torque limit control selection at undervoltage warning					*/
//		BOOL	PoleDetectEnd;			/* Current zero phase position save execution flag	[0:Incomplete,1:Complete]			*/
//		BOOL	SpdDetMaFil;			/* Enable moving average filter for motor speed detection/Select invalid			*/
	/*----------------------------------------------------------------------------------------------*/
//★<you> RvsDir		CHAR	DirSign;				/* For rotation mode calculation(+1/-1)		*/
//		UCHAR	AxisAddr;				/* Axis address(For serial communication)								*/
//		UCHAR	StopMode;				/* Servo off and stop mode when alarm occurs					*/
//		UCHAR	OtStopMode;				/* Overtravel(OT)							*/
//		UCHAR	MspdMafNum;				/* Motor speed detection moving average count								*/

//		UCHAR	PcmdFilter;				/* Position command filter selection setting									*/
//		UCHAR	BacklashDir;			/* Backlash correction direction selection setting								*/
//		UCHAR	BVIBControl;			/* Sway control selection setting										*/
//		UCHAR	PDTPrmMethod;			/* Prediction parameter setting method selection setting							*/

//		UCHAR	PDTPrmTp;				/* Prediction parametersTpSelection setting									*/
//		UCHAR	PerClrMode;				/* Position deviation clear operation selection setting								*/
//		UCHAR	PerClrSigSel;			/* Position deviation clear signal format selection setting							*/
//		UCHAR	PerClrLvlSel;			/* Position deviation clear signal level selection setting							*/

//		UCHAR	CoinOut;				/* /COINOutput timing selection setting								*/
//		UCHAR	VdetMode;				/* Main circuit bus voltage detection method									*/
//		UCHAR	VdetFollowWidthMax;		/* PWMMaximum variable adjustment width when main circuit is detected by						*/
//		LONG	VdcdetAlmFilter;		/* Main circuit detection abnormality alarm filter		(1=250us)			*/

//		LONG	OvdetAlmFilter;			/* Overvoltage alarm filter				(1=250us)			*/
//		LONG	RegStartFilter;			/* Regeneration operation start filter					(1=250us)			*/
//		LONG	RegStopFilter;			/* Regeneration operation start filter					(1=250us)			*/
//		LONG	RgSumSel;				/* Initial value ratio selection for regenerative overload detection							*/
//		LONG	RlyDbWay;				/* Inrush/DBControl method selection setting 									*/

//		LONG	MFC_type;				/* MFCProcessing type											*/
//		LONG	TuningLessType;			/* Adjustment-less processing type										*/
//		UCHAR	padding;				// padding for 4byte alignment
	/*----------------------------------------------------------------------------------------------*/
//		USHORT	CtrlModeA;				/* Control mode A												*/
//		USHORT	CtrlModeB;				/* Control mode B												*/
	/*----------------------------------------------------------------------------------------------*/
//		USHORT	VdetPrmOk;				/* Main circuit detection related parameters OK	(1=OK)						*/
//		USHORT	OvLevel;				/* Overvoltage(OV)Detection level										*/
//		USHORT	UvLevel;				/* Undervoltage(UV)Detection level									*/
//		USHORT	Uv2Level;				/* UV2(DBFor brakingChargePumpDrive permission voltage)Detection level			*/
//		USHORT	RegOnLevel;				/* Regeneration Tr on level										*/
//		USHORT	RegOffLevel;			/* Regeneration Tr off level										*/
//		USHORT	UvWrnLevel;				/* Undervoltage(UV)Warning detection level							*/
//		USHORT	padding2;				// padding for 4byte alignment
//		LONG	VdetAdNorm;				/* Main circuit detection AD conversion result normalization constant							*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	mtlmt;					/* Multiturn limit setting (Prm --> Iprm)					*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	TuningLessMode;			/* Adjustmentless mode											*/
/*----------------------------------------------------------------------------------------------*/
//	KSGAIN	PcmdOvrSpd;				/* Position command overspeed(OS)			[pulse/SvCycle]				*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	TrefonLevel;			/* Torque command input ON level		[2^24/MaxTrq]				*/
//		LONG	OrgSrchSpeed;			/* Origin search speed				[2^24/OvrSpd]				*/
	/*----------------------------------------------------------------------------------------------*/
//		BOOL	CarrChange;				/* Carrier frequency switching flag [0:OFF,1:ON]				*/
//		BOOL	CarrState;				/* Carrier frequency switching status [0:Norm,1:High]			*/
//		LONG	IqLimitA;				/* Carrier switching current thresholdA	[(15000/MaxTrq)^2]			*/
//		LONG	IqLimitB;				/* Carrier switching current thresholdB	[2^24/MaxTrq]				*/
//		LONG	VelLimitA;				/* Carrier switching speed thresholdA	[2^24/OvrSpd]				*/
//		LONG	VelLimitB;				/* Carrier switching speed thresholdB	[2^24/OvrSpd]				*/
//		LONG	NormCarrCnt;			/* Normal carrier count value		[-]							*/
//		LONG	HighCarrCnt;			/* Switching high carrier count value	[-]							*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	SpdFbLpassFil;			/* Motor quiet speed FBLPFSetting	[0.01ms]		<V423>		*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	VdetAdFil;				/* Main circuit detection AD filter gain	(KIOPmove) 2010.06.08 Y.Oka Must be common to units	*/
	/*----------------------------------------------------------------------------------------------*/
//		KSGAIN	LinearCoolRate100PerGain;	/* Motor free-running cooling rate100%Gain				<V519>		*/
	/*----------------------------------------------------------------------------------------------*/
} IPRMDAT;


/****************************************************************************************************/
/*																									*/
/*		Common parameter definition : Common parameters that can be changed online(Used with multiple modules)				*/
/*																									*/
/****************************************************************************************************/
typedef	struct GPRMDAT
{
/*--------------------------------------------------------------------------------------------------*/
//		BOOL	NotchFil2;				/* 2nd stage notch filter used/not used							*/
//		BOOL	LpassFil2;				/* Second-order lag filter(CPU)Use/non-use						*/
//		BOOL	LpassFil3;				/* First-order lag filter after torque compensation(CPU)Use/non-use				*//*<V371>*/
//		BOOL	Dobs;					/* Disturbance observer selection setting									*/
//		BOOL	AresVibControl;			/* A-type vibration damping control setting											*/
//		BOOL	AvffFilter;				/* Vibration suppression filter setting										*/
//		BOOL	MFCFF;					/* External FF in MFC(Speed, torque)Valid/invalid setting				*/
//		BOOL	VelRippleComp;			/* Ripple correction function setting										*//*<V072>*/
//		BOOL	TuningLess;				/* Adjustment-less setting												*//*<V108>*/
//		BOOL	TuningLessVComp;		/* Adjustment-less phase lead compensation setting									*//*<V323>*/
//		BOOL	AtGsel;					/* Automatic gain switching selection setting									*//*<V177>*/
//		BOOL	PdetEnable;				/* Magnetic pole detection setting using parameters								*//*<V249>*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	MSW;					/* Mode switch selection										*/
//		LONG	MFCModel;				/* MFC mechanical model selection setting								*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	AtGselTrgA;				/* Automatic gain switching condition A selection setting								*//* <V177> */
	/*----------------------------------------------------------------------------------------------*/
//		LONG	MSWTrqLevel;			/* Mode switch torque level				 [2^24/MaxTrq]	*/
//		LONG	MSWSpdLevel;			/* Mode switch speed level					 [2^24/OvrSpd]	*/
//		LONG	MSWAccLevel;			/* Mode switch acceleration level				 [NorSpd/ScanC]	*/
//		LONG	MSWErrLevel;			/* Mode switch deviation level					 [Pulse, mm]	*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	InterSpdLevel[4];		/* Internal setting speed level						 [2^24/OvrSpd]	*/
//		LONG	TConSpdLmtLevel;		/* Speed limit level during torque control				 [2^24/OvrSpd]	*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	CoinLevel;				/* Positioning completion width								[Command unit]	*/
//		LONG	NearLevel;				/* Near signal width								[Command unit]	*/
//		LONG	VcmpSpdLevel;			/* Speed match detection width							 [2^24/OvrSpd]	*/
//		LONG	TgonSpdLevel;			/* Motor rotation detection level						 [2^24/OvrSpd]	*/
//		LONG	VibCheckLevel;			/* Vibration detection level						 	 [2^24/OvrSpd]	*/
//		LONG	VibCheckLevelForTunLess;/* Vibration detection level for automatic notch			 	 [2^24/OvrSpd]	*//*<V109>*/
//		LONG	JstVibCheckLevel;		/* Vibration detection level during moment of inertia identification	 	 [2^24/OvrSpd]	*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	BkonSpdLevel;			/* Brake command output speed level(Pn507)		 [2^24/OvrSpd]	*/
//		LONG	BkonWaitTime;			/* Servo off brake command wait time(Pn508)			  [ms]	*/
//		LONG	BkonDelayTime;			/* Brake command servo off delay time(Pn506)			  [ms]	*/
//		LONG	ZclampSpdLevel;			/* Zero clamp speed level(Pn501)			 [2^24/OvrSpd]	*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	PerAlmLevel;			/* Excessive position deviation alarm level					[Command unit]	*/
//		LONG	PerWrnLevel;			/* Excessive position deviation warning level					[Command unit]	*/
//		LONG	SvonPerAlmLevel;		/* Excessive position deviation alarm level when servo is on		[Command unit]	*/
//		LONG	SvonPerWrnLevel;		/* Excessive position deviation warning level when servo is on		[Command unit]	*/
//		LONG	SvonSpdLmtLevel;		/* Speed limit level when servo is on				 [2^24/OvrSpd]	*/
//		LONG	SvonSpdLmtValue;		/* Speed limit value when servo is on					 [2^24/OvrSpd]	*/
	/*----------------------------------------------------------------------------------------------*/
//		ULONG	fpgoverLevel;			/* Motor-Excessive deviation between load positions detection level			[Command unit]	*/
//		LONG	fpgcutratelv;			/* Full close1Multiply value per rotation					   [%]	*/
//		LONG	fcpgpls;				/* Number of external scale pitches						   [P/Rev]	*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	FwIntTrqLmt;			/* Forward rotation internal torque limit						 [2^24/MaxTrq]	*/
//		LONG	RvIntTrqLmt;			/* Reverse side internal torque limit						 [2^24/MaxTrq]	*/
//		LONG	FwExtTrqLmt;			/* Forward rotation external torque limit						 [2^24/MaxTrq]	*/
//		LONG	RvExtTrqLmt;			/* Reverse rotation external torque limit						 [2^24/MaxTrq]	*/
//		LONG	EstopTrqLmt;			/* Emergency stop torque							 [2^24/MaxTrq]	*/
//		LONG	GrvCompTorque;			/* Gravity compensation torque							 [2^24/MaxTrq]	*/
//		LONG	UvTrqLmt;				/* Torque limit when main circuit voltage drops				 [2^24/MaxTrq]	*//* <V200> */
//		LONG	DeltaUvTrqLmt;			/* Torque limit reset torque when main circuit voltage drops		 [2^24/MaxTrq]	*//* <V200> */
	/*----------------------------------------------------------------------------------------------*/
//		LONG	RemVibDetWidth;			/* Residual vibration frequency detection width							[pls]		*/
//		LONG	RemVibFil;				/* Position deviation filter frequency for residual vibration frequency detection		[Hz]		*/
	/*----------------------------------------------------------------------------------------------*/
//		LONG	OvsErrDetLvl;			/* Allowable overshoot amount					[Command unit]			*//* <V113> */
	/*----------------------------------------------------------------------------------------------*/
//		VUSHORT	*AsicMonAdr0;			/* Asic Register Monitor Address CH0						*/
//		VUSHORT	*AsicMonAdr1;			/* Asic Register Monitor Address CH1						*/
	/*----------------------------------------------------------------------------------------------*/
		/* For linear */
//		LONG	LinearCoolRateGain;		/* Motor free-running cooling rate gain						<V019>		*/
/*--------------------------------------------------------------------------------------------------*/
} GPRMDAT;
#endif


//****************************************************************************************************
//
//	Parameter access handle
//
//****************************************************************************************************
typedef struct UNI_PRM_STR {
//	IPRMDAT			*iprm_ptr;
//	GPRMDAT			*gprm_ptr;
	BPRMDAT			*Bprm;
	PRMDATA			*Prm;
	CMN_PRMDATA		*Cprm;
} UNI_PRM_STR;

#endif	/* _UNIT_PRM_STRUCT_H_ */
/***************************************** end of file **********************************************/

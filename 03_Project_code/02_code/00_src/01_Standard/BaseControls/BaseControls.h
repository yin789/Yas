/****************************************************************************************************/
/*																									*/
/*																									*/
/*		BaseControls.h : Local Header File for ScanB Software										*/
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
#ifndef	_BASE_CONTROLS_H_
#define	_BASE_CONTROLS_H_


#include "KLib.h"
#include "MLib.h"
#include "HwDeviceIf.h"
#include "EncData.h"
#include "JL086ServoIpHw.h"
#include "BaseCmnStruct.h"
#include "CtrlLoopHandler.h"
#include "GselHandler.h"
#include "MfcHandler.h"
#include "PcmdFilter.h" /* <S065> */
#include "ResVibCtrl.h"
#include "TableDrive.h"
#include "PnPrmStruct.h"
#include "Bprm.h"
#include "MicroPrmCalc.h"
#include "ProgramJog.h"
#include "JatOffLine.h"
#include "CtrlPrmTransducer.h"
#include "Pls.h"
#include "MotorLessTestStr.h"	/* <S1B7> */

/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Bit No. Definition for LatchSel (ULONG LatchSel)											*/
/*--------------------------------------------------------------------------------------------------*/
#define	C_PHASE_LATCH		0x01

/* <S14B> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Threshold conditions for enabling the speed ripple compensation function													*/
/*--------------------------------------------------------------------------------------------------*/
#define	CMD_SPD		0x00
#define	FB_SPD		0x01
/* <S14B> End */

/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		New adjustment-less parameter variable definition <S1A5> 														*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _TL2PRM {
	LONG		KrlpfResSup;		/* Mechanical resonance suppression filter gain									*/
	LONG		KrlpfDisObs1[5];	/* Disturbance observer2Order filter gain(For bandpass filter)		*/
	LONG		KrlpfDisObs2[5];	/* Disturbance observer2Order filter gain(For high-pass filter)			*/
	LONG		KrCurfreq;			/* Current loop approximation model frequency									*/
	LONG		KrCurZeta1;			/* Current loop approximation model damping coefficient1								*/
	LONG		KrCurZeta2;			/* Current loop approximation model damping coefficient2								*/
	KSGAIN		KrlpfCur[5];		/* Current loop approximation model2Order filter gain						*/
} TL2PRM;

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter variable definition for no adjustment																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct TUNELESS_PRM {
	USHORT		kv;				/* Speed loop gain									[0.1Hz]			*/
	USHORT		kv2;			/* Velocity loop gain after correction							[0.1Hz]			*/
	USHORT		jrate;			/* Load moment of inertia/Mass ratio						[%]				*/
	USHORT		Ti;				/* Speed loop integral time constant								[0.01ms]		*/
	USHORT		kp;				/* Position loop gain									[0.1/s]			*/
	USHORT		klpf;			/* Torque command filter time constant after torque compensation				[0.01ms]		*/
	USHORT		f1;				/* Disturbance observer low-pass filter1Frequency			[0.1Hz]			*/
	USHORT		f1Comp;			/* Disturbance observer low-pass filter after correction1Frequency		[0.1Hz]			*/
	USHORT		f2;				/* Disturbance observer low-pass filter2Frequency			[0.1Hz]			*/
	USHORT		kj_dist;		/* Disturbance observer inertia gain					[%]				*/
	USHORT		kj_dist2;		/* Disturbance observer inertia gain after correction				[%]				*/
	USHORT		kd_dist;		/* Disturbance observer coefficient								[%]				*/
	USHORT		ks_vobs;		/* Phase compensated speed observer gain						[Hz]			*/
	USHORT		kj_vobs;		/* Phase compensated speed observer inertia gain			[%]				*/
	USHORT		kj_vobs2;		/* Phase compensation after correction Speed observer inertia gain		[%]				*/
	USHORT		vobsLpf;		/* Phase compensation speed observerLPFTime constant					[0.01ms]		*/
	USHORT		Stiff;			/* Robustness reduction rate									[%]				*/
	USHORT		F1rate;			/* Disturbance observer filter frequency1Reduction rate				[%]				*/
	LONG		vffPhCmpFrq1;	/* Phase compensation filter frequency1							[0.1Hz]			*/
	LONG		vffPhCmpFrq2;	/* Phase compensation filter frequency2							[0.1Hz]			*/
/*--------------------------------------------------------------------------------------------------*/
#if 0							/* <S1A5> */
	LONG		KrlpfResSup;	/* Without new adjustment Mechanical resonance suppression filter gain							*/
	LONG		KrlpfDisObs1[5];/* Without new adjustment Disturbance observer2Order filter gain(For bandpass filter)	*/
	LONG		KrlpfDisObs2[5];/* Without new adjustment Disturbance observer2Order filter gain(For high-pass filter)	*/
	LONG		KrCurfreq;		/* Without new adjustment Current loop approximation model frequency							*/
	LONG		KrCurZeta1;		/* Without new adjustment Current loop approximation model damping coefficient1							*/
	LONG		KrCurZeta2;		/* Without new adjustment Current loop approximation model damping coefficient2							*/
	KSGAIN		KrlpfCur[5];	/* Without new adjustment Current loop approximation model2Order filter gain					*//*<S00A>*/
#endif
	TL2PRM		Tl2Prm;			/* <S1A5> 															*/
/*--------------------------------------------------------------------------------------------------*/
} TUNELESS_PRM;

/*--------------------------------------------------------------------------------------------------*/
/*		Definition of variables for robust compensator calculation for no adjustment														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct TUNELESS_DRCOM {				/* Robust compensator parameter definition						*/
	struct {								/* Register definition for no adjustment								*/
#if (FLOAT_USE==TRUE)
		float		KVfbFil;					/* Speed FB filter gain for adjustment-less					*/
		float		KCmpOutlpf;					/* Robust compensator torque compensation low-pass filter gain	*/
		float		Krlpf1;						/* Robust compensator low-pass filter gain1			*/
		float		Krlpf2;						/* Robust compensator low-pass filter gain2			*/
		float		Krd;						/* Robust compensation torque coefficient							*/
		float		Krsj;						/* Robust compensator torque compensation gain					*/
#else
		LONG		KVfbFil;					/* Speed FB filter gain for adjustment-less					*/
		LONG		KCmpOutlpf;					/* Robust compensator torque compensation low-pass filter gain	*/
		LONG		Krlpf1;						/* Robust compensator low-pass filter gain1			*/
		LONG		Krlpf2;						/* Robust compensator low-pass filter gain2			*/
		LONG		Krd;						/* Robust compensation torque coefficient							*/
		LONG		Krsj;						/* Robust compensator torque compensation gain					*/
#endif /* FLOAT_USE */
	/*----------------------------------------------------------------------------------------------*/
	TL2PRM	*pTl2;						/* Tuningless Parameter <S1A5> 								*/
	/*----------------------------------------------------------------------------------------------*/
	void	(*RobustCompensator)		/* Tuneingless Robust Compensator <S1A5> 					*/
			( void *pdrcmp, 			/* 															*/
			  LONG motspd, 				/* 															*/
			  LONG trqref );			/* 															*/
	/*----------------------------------------------------------------------------------------------*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Register definition for no adjustment								*/
		LONG		DisTrq;						/* Estimated disturbance torque value									*/
		LONG		DisTrqCmp;					/* Disturbance torque compensation value			[2^24/MaxTrq]			*/
		LONG		TrqLpfo;					/* Robust compensator low-pass filter output value				*/
		LONG		TrqLpfo_old;				/* Robust compensator previous low-pass filter output value			*/
		LONG		VelLpfo1;					/* Robust compensator differential velocity low-pass filter output value1	*/
		LONG		VelLpfo2;					/* Robust compensator differential velocity low-pass filter output value2	*/
		BOOL		TrqFilClrReq;				/* Torque command filter integral initialization request					*/
		BOOL		CurrentStatus;				/* Enabled/Current state flag for invalid switching determination			*/
		BOOL		LastStatus;					/* Enabled/Previous state flag for invalid switching determination			*/
	/*----------------------------------------------------------------------------------------------*/
		LONG		Krlpftmp1[4];				/* <S1A5> 2NextLPFFilter internal calculation variables				*/
		LONG		Krlpftmp2[4];				/* <S1A5> 2NextLPFFilter internal calculation variables				*/
		LONG		Krlpftmp3[4];				/* <S1A5> 2NextLPFFilter internal calculation variables				*/
	/*----------------------------------------------------------------------------------------------*/
	} var;
/*--------------------------------------------------------------------------------------------------*/
} TUNELESS_DRCOMP;

/*--------------------------------------------------------------------------------------------------*/
/*		Phase compensation speed observer variable definition for no adjustment													*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct TUNELESS_EHVOBS {			/* Parameter definition for phase compensation speed observer				*/
	struct {								/* Register definition for no adjustment								*/
#if (FLOAT_USE==TRUE)
		float		Kstf1;						/* Phase compensationVOBSGain1								*/
		float		Kstf2;						/* Phase compensationVOBSGain2								*/
		float		Kstf3;						/* Phase compensationVOBSGain3								*/
		float		Kj;							/* Phase compensationVOBSPhase compensation gain						*/
		float		Lpf;						/* Phase compensationVOBSLow pass filter gain (LPF + Pn401)*/
		float		Tf;							/* Phase compensationVOBSLow-pass filter time constant				*/
#else
		LONG		Kstf1;						/* Phase compensationVOBSGain1								*/
		LONG		Kstf2;						/* Phase compensationVOBSGain2								*/
		LONG		Kstf3;						/* Phase compensationVOBSGain3								*/
		LONG		Kj;							/* Phase compensationVOBSPhase compensation gain						*/
		LONG		Lpf;						/* Phase compensationVOBSLow pass filter gain (LPF + Pn401)*/
		LONG		Tf;							/* Phase compensationVOBSLow-pass filter time constant				*/
#endif /* FLOAT_USE */
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Register definition for no adjustment								*/
		KSGAIN		Ivar64[2];					/* For integral calculation										*//*<S00A>*/
		LONG		EhSpdObs;					/* Speed estimated value										*/
		LONG		Wkbf1;						/* Low-pass filter output 							*/
		LONG		Wkbf2;						/* Phase compensationVOBSWork variable for calculation						*/
	} var;
/*--------------------------------------------------------------------------------------------------*/
} TUNELESS_EHVOBS;

/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for adjustmentless control																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct TUNELESS_CTRL {
	TUNELESS_PRM		TuneLessPrm;		/* Gain parameter structure for no adjustment						*/
	TUNELESS_DRCOMP		Drcomp;				/* Robust compensation calculation variables for no adjustment						*/
	TUNELESS_EHVOBS		EhVobs;				/* Phase compensation speed for no adjustmentOBSCalculation variables					*/
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Register definition for no adjustment								*/
		USHORT			DatGnLevel;				/* 0x20E0 : DATGain level							*/
		USHORT			DatJratLevel;			/* 0x20E1 : DATLoad level							*/
	} OpeReg;
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Parameter definition for no adjustment								*/
		BOOL			TuningLessUse;			/* Adjustment-less use/Not used settings							*/
		BOOL			TuningLessEx;			/* New adjustment-less setting									*/
		BOOL			TuningLessVComp;		/* Adjustment-less phase lead compensation setting							*/
		LONG			TuningLessType;			/* Adjustment-less processing type								*/
		LONG			TuningLessMode;			/* Adjustmentless mode									*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Variable definition for no adjustment									*/
		LONG			SpdFbFilo;				/* SpeedFBFilter output								*/
		BOOL			TuneLessAct;			/* Adjustment-less request										*/
		BOOL			TuneLessVCompReq;		/* Adjustment-less phase lead compensation request							*/
		BOOL			TuneLessInvldReq;		/* Forced invalidation request without adjustment								*/
		BOOL			TuningFuncRun;			/* Fn200Running flag								*/
	} var;
/*--------------------------------------------------------------------------------------------------*/
} TUNELESS_CTRL;



/*--------------------------------------------------------------------------------------------------*/
/*		Variable definitions for disturbance observer									<V221>							*/
/*--------------------------------------------------------------------------------------------------*/
#if 0		/* <S09B> */
typedef	struct DOBS_PRM {					/* Parameter definitions for disturbance observer						*/
	KSGAIN				Ks;						/* Disturbance observer gain								*/
	KSGAIN				Kj;						/* Disturbance observer inertia gain					*/
	KSGAIN				Ksj;					/* Disturbance observer torque gain						*/
	KSGAIN				Kd;						/* Disturbance torque coefficient									*/
	KSGAIN				Klpf;					/* Disturbance observer low-pass filter gain				*/
} DOBS_PRM;
#else
typedef GSELDOBS DOBS_PRM;
#endif

typedef	struct DOBS_CTRL {					/* Parameter & variable definitions for disturbance observer					*/
//<S09B>	DOBS_PRM			DobsPrm[3];				/* Parameters for disturbance observer						*/
	DOBS_PRM			*pDobsPrm;				/* Parameters for disturbance observer <S09B>				*/
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Variable data definitions for disturbance observer						*/
		BOOL			DobsAct;				/* Disturbance observer execution request							*/
		KSGAIN			Ivar64[2];				/* For integral calculation										*//*<S00A>*/
		LONG			SpdObs;					/* Speed estimated value										*/
		LONG			DisTrq;					/* Estimated disturbance torque value									*/
		LONG			Hpftmp;					/* For high-pass filter calculation							*/
		LONG			DTrqLpfo;				/* Low-pass filter output disturbance torque estimated value				*/
		LONG			DisTrqCmp;				/* Disturbance torque compensation value			[2^24/MaxTrq]			*/
		UCHAR			CurrentStatus;			/* Enabled/Current state flag for invalid switching determination			*/
		UCHAR			LastStatus;				/* Enabled/Previous state flag for invalid switching determination			*/
		UCHAR			ExchangeSts;			/* Function enable/disable switching status(For shock reduction)		*/
		UCHAR			Dummy;					/* for Alignment									*/
	} var;
/*--------------------------------------------------------------------------------------------------*/
} DOBS_CTRL;



/*--------------------------------------------------------------------------------------------------*/
/*		Phase compensation speed observer(VOBS)Variable definition														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct EHVOBS_PRM {					/* Parameter definition for phase compensation speed observer				*/
#if (FLOAT_USE==TRUE)
	float				Kstf1;					/* Phase compensationVOBSGain1(Pn127)						*/
	float				Kstf2;					/* Phase compensationVOBSGain2(Pn127)						*/
	float				Kstf3;					/* Phase compensationVOBSGain3(Pn127)						*/
	float				Kj;						/* Phase compensationVOBSPhase compensation gain (Pn128)				*/
	float				Lpf;					/* Phase compensationVOBSLow pass filter gain(Pn13B+Pn401)	*/
	float				Tf;						/* Phase compensationVOBSLow-pass filter time constant				*/
	float				KTrqReflpf;				/* VOBSTorque command low-pass filter gain				*/
#else
	
	LONG				Kstf1;					/* Phase compensationVOBSGain1(Pn127)						*/
	LONG				Kstf2;					/* Phase compensationVOBSGain2(Pn127)						*/
	LONG				Kstf3;					/* Phase compensationVOBSGain3(Pn127)						*/
	LONG				Kj;						/* Phase compensationVOBSPhase compensation gain (Pn128)				*/
	LONG				Lpf;					/* Phase compensationVOBSLow pass filter gain(Pn13B+Pn401)	*/
	LONG				Tf;						/* Phase compensationVOBSLow-pass filter time constant				*/
	LONG				KTrqReflpf;				/* VOBSTorque command low-pass filter gain				*/
#endif /* FLOAT_USE */
} EHVOBS_PRM;

typedef	struct EHVOBS_CTRL {				/* Parameter & variable definition for phase compensation speed observer			*/
	EHVOBS_PRM			EhVobsPrm;				/* Parameters for phase compensation speed observer				*/
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Variable data definitions for disturbance observer						*/
		BOOL			EhVobsUse;				/* Phase compensation speed observer enabled/Select invalid				*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Variable data definition for phase compensation speed observer				*/
		KSGAIN			Ivar64[2];				/* For integral calculation										*//*<S00A>*/
		LONG			AccSum;					/* Acceleration integrated value(Speed estimated value)							*/
		LONG			EhSpdObs;				/* Speed estimated value										*/
		LONG			Wkbf1;					/* Low-pass filter output 							*/
		LONG			Wkbf2;					/* Phase compensationVOBSWork variable for calculation						*/
		LONG			TrqRefLpfo;				/* VOBSTorque command filter estimated value						*/
	} var;
} EHVOBS_CTRL;

/* <S14B> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Speed ripple compensation variable definition																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	RPLCMP_ORDER_MECHA	12				/* Speed ripple compensation maximum order(Mechanical angle)					*/
#define	RPLCMP_ORDER_ELE	6				/* Speed ripple compensation maximum order(Electrical angle)					*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct RPLCMP_PRM_MECHA {		/* Speed ripple compensation parameter definition(Mechanical angle)				*/
	LONG	CompGain;							/* Speed ripple compensation gain			[%]				*/
	USHORT	CompFrq[RPLCMP_ORDER_MECHA];		/* Speed ripple compensation component				[-]				*/
	LONG	CompAmpValue[RPLCMP_ORDER_MECHA];	/* Speed ripple compensation amplitude				[15000/MaxTrq]	*/
	USHORT	PhaseOfset[RPLCMP_ORDER_MECHA];		/* Speed ripple compensation phase				[65536/360deg]	*/
}RPLCMP_PRM_MECHA;

typedef	struct RPLCMP_PRM_ELE {			/* Speed ripple compensation parameter definition(Electrical angle)				*/
	LONG	CompGain;							/* Speed ripple compensation gain			[%]				*/
	USHORT	CompFrq[RPLCMP_ORDER_ELE];			/* Speed ripple compensation component				[-]				*/
	LONG	CompAmpValue[RPLCMP_ORDER_ELE];		/* Speed ripple compensation amplitude				[15000/MaxTrq]	*/
	USHORT	PhaseOfset[RPLCMP_ORDER_ELE];		/* Speed ripple compensation phase				[65536/360deg]	*/
}RPLCMP_PRM_ELE;

typedef	struct RPLCMP_PRM {				/* Speed ripple compensation parameter definition						*/
	RPLCMP_PRM_MECHA	RplCmpPrmMecha;				/* Speed ripple compensation parameter (mechanical angle)			*/
	RPLCMP_PRM_ELE		RplCmpPrmEle;				/* Speed ripple compensation parameter (electrical angle)			*/
	/*----------------------------------------------------------------------------------------------*/
	BOOL	RplCmpUse;								/* Speed ripple compensation used/Not used settings				*/
	BOOL	RippleWrnMode;							/* Speed ripple compensation information mismatch warning detection mode(0:detect,1:Does not detect.)*//* <S1FD> */
	BOOL	RippleLstWrnMode;						/* Speed ripple compensation information mismatch warning detection mode Previous value(200DhFor)*//* <S1FD> */
//	BOOL	RplCmpMode6f;							/* Speed ripple compensation mode selection(1:1f~6f/0:1f~3f)*/
//	BOOL	RplCmpDobsMon;							/* Monitor output selection(DisturbanceOBSand shared)				*/
//	USHORT	RplSpAnalyzeMode;						/* Speed ripple spectrum analysis mode selection			*/
	USHORT	RplCompGainEle;							/* Speed ripple compensation gain(Electrical angle)				*/	/* <S14B> */
	USHORT	RplCompCycleEle[RPLCMP_ORDER_ELE];		/* Speed ripple compensation component period(Electrical angle)				*/	/* <S14B> */
	SHORT	RplCompAmplitudeEle[RPLCMP_ORDER_ELE];	/* Speed ripple compensation component amplitude(Electrical angle)				*/	/* <S14B> */
	USHORT	RplCompPhaseEle[RPLCMP_ORDER_ELE];		/* Speed ripple compensation component phase(Electrical angle)				*/	/* <S14B> */
} RPLCMP_PRM;
/*--------------------------------------------------------------------------------------------------*/
typedef	struct {						/* Speed ripple compensation parameter & variable definition				*/
	RPLCMP_PRM			RplCmpPrm;					/* Speed ripple compensation parameters					*/
/*--------------------------------------------------------------------------------------------------*/
	struct {										/* Speed ripple compensation variable data definition				*/
		LONG			TrqCompAll;					/* Speed ripple compensation torque		   [2^24/MaxTrq]*/
		BOOL			RippleWrnState;				/* Speed ripple compensation information warning status(0:A.942Generate,1:Normal)	*/	/* <S1FD> */
		BOOL			RippleCmpExe;				/* Checking the threshold conditions for enabling the speed ripple compensation function */
//		LONG			TrqComp[RPLCMP_ORDER];		/* Speed ripple compensation torque(Order component)[2^24/MaxTrq]*/
	} var;
/*--------------------------------------------------------------------------------------------------*/
	USHORT	RippleCmpState;			/* Speed ripple compensation information writing confirmation(00H:Complete,01H:Not completed,02H:Failure)	*/
	BOOL	VelRipplePhaseReady;	/* For speed ripple compensation Magnetic pole detection completion flag						*/
	LONG	RippleTrqCompMon;		/* Speed ripple correction torque(For monitor)	[2^24/MaxTrq]				*/
	BOOL	RippleCompMon;			/* Speed ripple compensation in progress signal(Un110 bit23:0=Correction invalid,1=During correction)		*/
	USHORT	RippleCmpThresholdSel;	/* Selecting the threshold conditions for enabling the speed ripple compensation function					*/
	LONG	TrqRefBeforeComp;		/* Torque command before correction(Trace.AnaMonFor)[2^24/MaxTrq]				*/
	LONG	VelRippleCompSpd;		/* Speed ripple compensation effective speed[min-1]								*/
} RPLCMP;
/* <S14B> End */

/*--------------------------------------------------------------------------------------------------*/
/*		Control mode switching definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CTRL_CHANGE {
	LONG	dPosFbki;				/* Position increment value(Increment value since last response)	[pulse]					*/
	LONG	CmdErr;					/* Position deviation correction value [Command unit]									*/
	LONG	SpdFbki_l;				/* Motor speed(Previous value)					[2^24/OvrSpd]			*/
	LONG	SpdFbki_a;				/* Motor speed(Average value of previous and current)		[2^24/OvrSpd]			*/
} CTRL_CHANGE;


/*--------------------------------------------------------------------------------------------------*/
/*		ScanB																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct SERVO_CONTROL_IF {
	SV_OPTION_IF	CmdOptionIf;			/* SV-OP I/FPointer to handle						*/
	BOOL			CmdReady;				/* Command acceptance flag */

	CMD_CTRLBITS	CmdCtrl;				/* Control command												*/
	LONG			CmdSeqBit;				/* Sequence control command									*/
	LONG			CmdLtBit;				/* Latch request											*/
	RSP_CTRLBITS	RspCtrl;				/* Control state */
	LONG			RspSigBit;				/* Sequence signal										*/
	LONG			RspLtBit;				/* Latch status											*/

	BOOL			LtReqCphs;
	BOOL			LtReqExt1;
	BOOL			LtReqExt2;
	BOOL			LtReqExt3;
	BOOL			ExtLtEnable[3];
	SHAL_LATCH_STS	ExtLtSts[3];

	LONG			MonCphaLpos[2];			/* CPhase latch position	[Command unit]							*/
	LONG			MonExt1Lpos[2];			/* EXT1Latch position	[Command unit]							*/
	LONG			MonExt2Lpos[2];			/* EXT2Latch position	[Command unit]							*/
	LONG			MonExt3Lpos[2];			/* EXT3Latch position	[Command unit]							*/

//	CTRL_CHANGE		CtrlChange;				/* Control switching variable											*/
//	LONG			VirtualPositionError[2];/* Virtual position deviation		[Command unit]	(Communication response timing)	*/
//	LONG			FbPositionFB[2];		/* FBPosition			[FBunit]	(Communication response timing)	*/

	ULONG			NetSout;				/* CommunicationI/FOutput signal command									*/
	LONG			NetCtrlMcmd;			/* Control mode(Commands from communication)							*/
	LONG			NetTrqFFC;				/* TorqueFFCommand		[2^24/MaxTrq]						*/
	LONG			NetTrqRef;				/* Torque command		[2^24/MaxTrq]						*/
	LONG			NetSpdLmt;				/* Speed limit			[2^24/OvrSpd]						*/
	LONG			NetSpdRef;				/* Speed command			[2^24/OvrSpd]						*/
	LONG			NetSpdFFC;				/* Speed FF command		[2^24/OvrSpd]						*/
	LONG			NetPosRef;				/* Position command			[Command unit]							*/

	ULONG			NetIfMonInSignal1;		/* SIPortetcInput signal status for Network	Single-axis amplifier		*//* <S0AC> *//* <S1C0>:rename */
	ULONG			NetIfMonInSignal2;		/* SIPortetcInput signal status for Network	Multi-axis amplifier		*//* <S1C0> */

	LONG			*MonRamTable[0x50];	/* Extension monitor variable storage table					*//* <S1D7>: Change number of elements */
	struct {								/* Extension monitor selection										*/
		LONG		MonSel1;				/* Extension monitor selection1										*/
		LONG		MonSel2;				/* Extension monitor selection2										*/
		LONG		MonSel3;				/* Extension monitor selection3										*/
		LONG		MonSel4;				/* Extension monitor selection4										*/
		LONG		MonSel5;				/* Extension monitor selection5										*/
		LONG		MonSel6;				/* Extension monitor selection6										*/
		LONG		MonSel7;				/* Extension monitor selection7										*/
		LONG		MonSel8;				/* Extension monitor selection8										*/
	} MonSel;
} SERVO_CONTROL_IF;


typedef	struct SETTLINGTIME {			/* Variable definition for settling time-related processing								*/
	BOOL	StlgCalcReq;					/* Settling time calculation flag									*/
	BOOL	StlgLastCoinSts;				/* COINSignal previous value										*/
	BOOL	StlgCoinLatch;					/* COINLatch status										*/
	ULONG	ProcessTimer;					/* Timer for detecting settling time measurement processing execution timing			*/
	LONG	StlgLastdPcmd;					/* Previous value of difference position command									*/
	LONG	StlgTimeCntr;					/* Settling time measurement counter								*/
	LONG	TmpStlgTime;					/* Settling time counter buffer								*/
	LONG	RsltStlgTime;					/* Settling time calculation result										*/
	LONG	OverErrDir;						/* Sign of overshooting deviation						*/
	LONG	OverShoot;						/* For overshoot amount measurement			[Command unit]			*/
	LONG	UnitChngGain;					/* Gain for unit conversion[cyc]⇒[1us]							*/
	LONG	UnMaxOverShoot;					/* Un106 : Maximum overshoot amount	[Command unit]			*/
	LONG	UnCoinOffTimer;					/* Un10B : Positioning completion failure time accumulation	[ms]				*/
	LONG	OvsErrDetLvl;					/* Allowable overshoot amount			[Command unit]			*/
	LONG	LastdPcmda;						/* Previous position command increment value[Command unit]							*/
} SETTLINGTIME;


typedef	struct UV_TLIM_DATA {
	struct {							/* Torque limit parameter definition at undervoltage warning				*/
		BOOL			UvTrqLmtCtrl;		/* Torque limit control selection at undervoltage warning				*/
		LONG			UvTLimSetting;		/* Torque limit value when main circuit voltage drops			[2^24/MaxTrq]	*/
		LONG			DeltaUvTrqLmt;		/* Torque limit reset torque when main circuit voltage drops	[2^24/MaxTrq]	*/
		LONG			UvTrqLmtOffFil;		/* Torque limit release time when main circuit voltage drops	[ScanBtimes]		*/
		SHORT			Dummy;				/* for Alignment										*/
	} conf;
	struct {							/* Torque limit variable definition at undervoltage warning					*/
		LONG			UvTrqLmt;			/* Torque limit at undervoltage				[2^24/MaxTrq]	*/
		ULONG			UvTrqLmtWait;		/* Torque limit release weight counter at undervoltage			*/
	} var;
} UV_TLIM_DATA;


typedef	struct TRQ_LMT_DATA {
	struct {							/* Torque limit parameter definition									*/
		BOOL			TrefTrqLmt;			/* Using torque command as external torque limit					*/
		BOOL			TrefTrqLmtCL;		/* Using torque command as external torque limit(P-CL,N-CLWhen enabled)*/
#if (FLOAT_USE==TRUE)
		float			EstopTrqLmt;		/* Emergency stop torque					[2^24/MaxTrq]		*/
		float			FwIntTrqLmt;		/* Forward rotation internal torque limit				[2^24/MaxTrq]		*/
		float			RvIntTrqLmt;		/* Reverse side internal torque limit				[2^24/MaxTrq]		*/
		float			FwExtTrqLmt;		/* Forward rotation external torque limit				[2^24/MaxTrq]		*/
		float			RvExtTrqLmt;		/* Reverse rotation external torque limit				[2^24/MaxTrq]		*/
#else
		LONG			EstopTrqLmt;		/* Emergency stop torque					[2^24/MaxTrq]		*/
		LONG			FwIntTrqLmt;		/* Forward rotation internal torque limit				[2^24/MaxTrq]		*/
		LONG			RvIntTrqLmt;		/* Reverse side internal torque limit				[2^24/MaxTrq]		*/
		LONG			FwExtTrqLmt;		/* Forward rotation external torque limit				[2^24/MaxTrq]		*/
		LONG			RvExtTrqLmt;		/* Reverse rotation external torque limit				[2^24/MaxTrq]		*/
#endif /* FLOAT_USE */
	} conf;
	struct {							/* Torque limit variable definition										*/
		BOOL			PclSignal;			/* PCL Signal											*/
		BOOL			NclSignal;			/* NCL Signal											*/
#if (FLOAT_USE==TRUE)
		float			FwTrqLmt;			/* Forward Torque Limit(Input)		[2^24/MaxTrq]		*/
		float			RvTrqLmt;			/* Reverse Torque Limit(Input)		[2^24/MaxTrq]		*/
//		float			PosTrqLmt;			/* Positive Torque Limit(Network)	[2^24/MaxTrq]		*/
//		float			NegTrqLmt;			/* Negative Torque Limit(Network)	[2^24/MaxTrq]		*/
		float			PosTrqLmtOut;		/* Positive Torque Limit(Output)		[2^24/MaxTrq]		*/
		float			NegTrqLmtOut;		/* Negative Torque Limit(Output)		[2^24/MaxTrq]		*/
#else
		LONG			FwTrqLmt;			/* Forward Torque Limit(Input)		[2^24/MaxTrq]		*/
		LONG			RvTrqLmt;			/* Reverse Torque Limit(Input)		[2^24/MaxTrq]		*/
//		LONG			PosTrqLmt;			/* Positive Torque Limit(Network)	[2^24/MaxTrq]		*/
//		LONG			NegTrqLmt;			/* Negative Torque Limit(Network)	[2^24/MaxTrq]		*/
		LONG			PosTrqLmtOut;		/* Positive Torque Limit(Output)		[2^24/MaxTrq]		*/
		LONG			NegTrqLmtOut;		/* Negative Torque Limit(Output)		[2^24/MaxTrq]		*/
#endif /* FLOAT_USE */
	} var;
	UV_TLIM_DATA		UvTrqLimits;		/* Data for torque limitation at undervoltage warning				*/
} TRQ_LMT_DATA;


typedef	struct BASE_MOT_STS {
//	BOOL				MotCposOk;			/* CPhase passage completion flag									*/
	BOOL				MotCphPass;			/* CPhase passage flag(1scan)									*/
	LONG				MposFromCpos;		/* CPOS		 [+/-(PulseNo/2)],[LM:LONG]	*/
	LONG				MotSpd;				/* Motor speed							[2^24/OvrSpd]	*/
	LONG				FencSpd;			/* Motor speed(Fluk)						[2^24/OvrSpd]	*//* <S04D> */
	LONG				MotPos;				/* Motor position(ScanBAccumulated value)				[32bitRingPulse]*/
	LONG				dMotPos;			/* Delta Motor Position					[pulse/ScanB]	*/
	BOOL				FencCphPass;		/* CPhase passage flag(1scan)(Fluk)							*//* <S04D> */
	LONG				FposFromCpos;		/* CPOS(Fluk) [+/-(PulseNo/2)],[LM:LONG]	*//* <S04D> */
	LONG				FencPos;			/* Motor position(ScanBAccumulated value)(Fluk)			[32bitRingPulse]*//* <S04D> */
	LONG				dFencPos;			/* Delta Motor Position	(Fluk)			[pulse/ScanB]	*//* <S04D> */
//	LONG				MotPosB;			/* ScanBAcquisition motor position(PGData)		[pulse]			*//* <S04D>: Not used	*/
	LONG				MotSpdSumB;			/* Motor Speed Sum(ScanBPeriod)							*/
	LONG				MotSpdCntB;			/* Motor Speed Cnt(ScanBPeriod)							*/
//	LONG				MotSpdForOS;		/* Motor Speed OSFor abnormality detection				[2^24/OvrSpd]	*//* <S04D>: Not used	*/
	LONG				MotSpdForSeq;		/* Motor Speed For sequence(No compensation)	[2^24/OvrSpd]	*//* <S0E5> */
//	SHORT				MotSpdForMp;		/* μProgram EMFMotor speed for compensation calculation	[15000/OvrSpd]	*//* <S04D>: Not used	*/
	SHORT				Dummy;				/* for Alignment */
} BASE_MOT_STS;


typedef	struct PCMP_CALC_DATA {
	KSGAIN				Kpcmpd;				/* Phase compensation(Delay)Gain for calculation1(μFor program)			*/
	KSGAIN				Kpcmpd2;			/* Phase compensation(Delay)Gain for calculation2(μFor program)			*/
	KSGAIN				Kpcmpsp1;			/* Phase compensation(Speed)Gain for calculation1(μFor program)			*/
	KSGAIN				KpcmpdItplt;		/* Phase interpolation amount operation coefficient(μFor program)					*/
	LONG				PcmpSpd1;			/* Phase compensation1Start speed									*/
} PCMP_CALC_DATA;


typedef	struct POS_CTRL_STS {
	BOOL				RefZSignal;			/* Reference Zero Signal								*/
	BOOL				NearSignal;			/* Near Signal											*/
	BOOL				CoinSignal;			/* Coin Signal											*/
	BOOL				DenSignal;			/* Den Signal											*/
} POS_CTRL_STS;


typedef	struct BASE_CTRL_HNDL {
	MENCV				*MencV;				/* Pointer to motor encoder data			 		*//* <S004> */
	MENCV				*FencV;				/* Pointer to Fulque encoder data			 		*//* <S04D> */
	CTRL_CMD_MNG		CtrlCmdMngr;		/* Control command management data									*/
	CTRL_CMD_PRM		CtrlCmdPrm;			/* Control command parameters									*/
	BASE_MOT_STS		MotSts;				/* ScanBMotor status					*/
	CTRLMD_SET			CtrlModeSet;		/* Control mode related data									*/
	POS_MNG_HNDL		PosManager;			/* Position management data handle								*/
	POS_LOOP_CTRL		FcPosLoop;			/* Position loop control data for Fulk 						*/
	POS_CTRL_STS		PosCtrlSts;			/* Position control status(DEN/COIN/NEAR)					*/
	TRQ_LMT_DATA		TrqLimitData;		/* Torque limit related data									*/

//	MOTSPDMAFIL			MotSpdMafil;		/* Variable for moving average calculation for motor speed detection					*/
	SETTLINGTIME		SettlingTime;		/* Structure for settling time measurement related processing							*/

	ZX_CTRL_HNDL		ZeroClampCtrl;		/* Zero clamp control calculation structure							*/
	PCMDFIL				PcmdFil;			/* Variable for position command filter								*//* <S065> */
	MAFIL				VFFcmdFil;			/* Variable for speed feedforward command filter				*//* <S1D0> */
	MAFIL				TFFcmdFil;			/* Torque feedforward command filter variable			*//* <S1D0> */
	RESVIB				ResVib;				/* Vibration suppression variable										*/
	VIBSUPFIL			VibSupFil;			/* Variables for vibration suppression filter								*/
	MODE_SW				ModeSwData;			/* ModeSWVariable for										*/
	GAIN_CHNG_HNDL		GainChange;			/* Structure for gain switching								*/
	MFCTRL				MFControl;			/* Variable for model following control									*/
	TUNELESS_CTRL		TuneLessCtrl;		/* Data for adjustmentless control									*/
	DOBS_CTRL			DobsCtrl;			/* Disturbance observer control data							*/
	EHVOBS_CTRL			EhVobsCtrl;			/* Phase compensation speed observer control data					*/
	PCMP_CALC_DATA		PcmpCalcData;		/* Phase compensation calculation data									*/
	TBLDRIVE			TblDrive;			/* Table operation structure									*/
	PJOGHNDL			PJogHdl;			/* ProgramJOG								*/
	JATHNDL				JatHdl;				/* Offline moment of inertia estimation							*/

	BOOL				BeComplete;			/* Base enable completion status								*/
//	BOOL				PdetCtrlReq;		/* Magnetic pole detection control request flag								*/
	BOOL				CLatchClrReq;		/* C Phase Position Latch Clear Request to μScanA		*/
	BOOL				BBRefClrReq;		/* Servo Control Ref. Clear Request from ScanC			*/
//	BOOL				OverTravel;			/* OTStatus(OTSequence processing output) from ScanC				*/
//	BOOL				PhaseReady;			/* Motor Phase Ready (Phase Found Finish)				*/
	BOOL				VrefInSts;			/* Speed command input flag									*//* <S0A1> */
	BOOL				TrefInSts;			/* Torque command input flag								*//* <S0A1> */
	LONG				GselNum;			/* Gain number for manual gain switching						*/

	WEAKENFIELD			WeakenField;		/* Field-weakening control calculation variables								*/
	MPKVVCOMP			MpKvvComp;			/* Variable for micro voltage compensation gain function						*/

	TRANSDUCE_CTRL		TransduceCtrl;		/* Control parameter batch transfer variables						*/
/*--------------------------------------------------------------------------------------------------*/
	BOOL				PerClrEdge;			/* @@ Position Error Clear Signal Edge					*/
	LONG				ClrFbPos;			/* @@ Position Error Clear Latched FB Position[Pulse]	*/
	LONG				MonPerrLpos[2];		/* @@ Position Error Clear Latched FB Position[Ref]		*/
/*--------------------------------------------------------------------------------------------------*/
	FRICTION			FrictionM;			/* <S0CD> */
	LONG				TrqMotFricComp;		/* <S0CD> */
/*--------------------------------------------------------------------------------------------------*/
	BOOL				OverTravel;			/* <S18B> */
/*--------------------------------------------------------------------------------------------------*/
	LONG				AmondPosCmd;		/* <S1B6> Position command difference				[pulse/125us]		*/
	
} BASE_CTRL_HNDL;


/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for position control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* Position control constant & variable definition							*/
		struct	{								/* Position control constant data definition							*/
#if (FLOAT_USE==TRUE)
//		LONG	Kpi;								/* Position control proportional integral gain						*/
		float	Kpd;								/* Position control proportional differential gain						*/
		float	VelBias;							/* Position control speed bias							*/
		float	BiasAddLevel;						/* Speed bias addition width		[Pulse, mm]			*/
//		LONG	KdPff;								/* Position control position increment value FF gain					*/
//		LONG	KdPffFil;							/* Position control position increment value FF filter gain			*/
#else
//		LONG	Kpi;								/* Position control proportional integral gain						*/
		LONG	Kpd;								/* Position control proportional differential gain						*/
		LONG	VelBias;							/* Position control speed bias							*/
		LONG	BiasAddLevel;						/* Speed bias addition width		[Pulse, mm]			*/
//		LONG	KdPff;								/* Position control position increment value FF gain					*/
//		LONG	KdPffFil;							/* Position control position increment value FF filter gain			*/
#endif /* FLOAT_USE */
		} P;
	/*----------------------------------------------------------------------------------------------*/
		struct {								/* Position control variable data definition							*/
#if (FLOAT_USE==TRUE)
		float	SpdRef;								/* Position control output									*/
		float	PerIvar64[2];						/* For position deviation integral calculation							*/
		float	PacOut;								/* Proportional control output									*/
		float	IacOut;								/* Integral control output									*/
		float	DacOut;								/* Differential control output									*/
		float	LastPacOut;							/* Previous proportional control output								*/
		float	dPFfFilo;							/* Position increment value FF filter output					*/
#else
		LONG	SpdRef;								/* Position control output									*/
		LONG	PerIvar64[2];						/* For position deviation integral calculation							*/
		LONG	PacOut;								/* Proportional control output									*/
		LONG	IacOut;								/* Integral control output									*/
		LONG	DacOut;								/* Differential control output									*/
		LONG	LastPacOut;							/* Previous proportional control output								*/
		LONG	dPFfFilo;							/* Position increment value FF filter output					*/
#endif /* FLOAT_USE */
		} V;
} POSCTRL;
/*--------------------------------------------------------------------------------------------------*/
/*		Speed control calculation variable definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* Speed control constant & variable definition							*/
		struct	{								/* Speed control constant data definition							*/
//		LONG	IPSpdControl;
//		LONG	SpdIctrlKeep;
//		LONG	P_I;								/* Speed control PI control ratio							*/
//		LONG	SpdSfsAcc;							/* Soft start acceleration amount							*/
//		LONG	SpdSfsDec;							/* Soft start deceleration amount							*/
//		LONG	KVrfFil;							/* Speed command filter gain						*/
//		LONG	KVfbFil;							/* Speed FB filter gain						*/
		KSGAIN	KVintegFil;							/* Speed integral clear filter gain					*//*<S00A>*/
//		LONG	SpdRefPhFil;						/* LPF for phase lead(For speed command)					*//*<V108>*/
//		LONG	SpdRefPhFilGn;						/* Phase lead compensation gain(For speed command)				*//*<V108>*/
////		LONG	SpdFFPhFil;							/* LPF for phase lead(For speed FF)					*//*<V108>*/
////		LONG	SpdFFPhFilGn;						/* Phase lead compensation gain(For speed FF)				*//*<V108>*/
//		LONG	SvonSpdLmtLevel;
//		LONG	SvonSpdLmtValue;
		} P;
	/*----------------------------------------------------------------------------------------------*/
		struct {								/* Speed control variable data definition							*/
#if (FLOAT_USE==TRUE)
		float	TrqRef;								/* Speed control output									*/
		float	SpdFbFilo;							/* Speed FB filter output							*/
		float	PacOut;								/* Proportional control output									*/
		float	IacOut;								/* Integral control output									*/
		LONG	Ivar64[2];							/* Integral control calculation								*/
		float	SpdRefPhLpfo;						/* LPF output for phase lead(For speed command)				*//*<V108>*/
		float	SpdRefPhHpfo;						/* HPF output for phase lead(For speed command)				*//*<V108>*/
		float	SpdRefPhHpfTmp;						/* HPF calculation for phase lead(For speed command)			*//*<V108>*/

		float	SpdFFPhLpfo;						/* LPF output for phase lead(For speed FF)				*//*<V108>*/
		float	SpdFFPhHpfo;						/* HPF output for phase lead(For speed FF)				*//*<V108>*/
		float	SpdFFPhHpfTmp;						/* HPF calculation for phase lead(For speed FF)			*//*<V108>*/
#else
		LONG	TrqRef;								/* Speed control output									*/
		LONG	SpdFbFilo;							/* Speed FB filter output							*/
		LONG	PacOut;								/* Proportional control output									*/
		LONG	IacOut;								/* Integral control output									*/
		LONG	Ivar64[2];							/* Integral control calculation								*/
		LONG	SpdRefPhLpfo;						/* LPF output for phase lead(For speed command)				*//*<V108>*/
		LONG	SpdRefPhHpfo;						/* HPF output for phase lead(For speed command)				*//*<V108>*/
		LONG	SpdRefPhHpfTmp;						/* HPF calculation for phase lead(For speed command)			*//*<V108>*/
		LONG	SpdFFPhLpfo;						/* LPF output for phase lead(For speed FF)				*//*<V108>*/
		LONG	SpdFFPhHpfo;						/* HPF output for phase lead(For speed FF)				*//*<V108>*/
		LONG	SpdFFPhHpfTmp;						/* HPF calculation for phase lead(For speed FF)			*//*<V108>*/
#endif /* FLOAT_USE */

		UCHAR	SpdRefFilClrCmd;					/* Speed command phase lead compensation filter initialization command	*//*<V392>*/
		UCHAR	SpdFFFilClrCmd;						/* Speed FF phase advance compensation filter initialization command	*//*<V392>*/
		BOOL	SvonSpdLmtFlag;
		BOOL	LastSvonFlag;
		} V;
} SPDCTRL;
/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for torque control calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
//typedef	struct	{								/* Torque control constant & variable definition						*/
//		struct	{								/* Torque control constant data definition						*/
//		LONG	OverTrqLoopGain[3];					/* Loop gain for speed limit						*/
//		LONG	OverTrqIntegGain[3];				/* Integral gain for speed limit							*/
#if	( SVFSEL_ANALOG_FEEDBACK == 1)
		LONG	Kp;									/* AnalogF/BProportional gain						*/
		LONG	Kvi;								/* AnalogF/BIntegral gain						*/
		LONG	KdFF;								/* AnalogF/BLoopFF							*/
		LONG	AnlgfbLimit;						/* AnalogF/BIntegral limit value					*/
		LONG	Bias;								/* AnalogF/BBias							*/
		USHORT	OverCntVal;							/* AnalogF/BOverload detection time count value			*/
		USHORT	AnlgfbPolarSw;						/* AnalogF/BPolarity selection							*/
		LONG	KIntegPreset;						/* AnalogF/BIntegral preset coefficient				*/
#endif
//		} P;
	/*----------------------------------------------------------------------------------------------*/
//		struct {								/* Torque control variable data definition						*/
//		LONG	SpdLimit;							/* Speed limit value during torque control						*/
//		LONG	OverTrqIntegBuf[2];
#if	( SVFSEL_ANALOG_FEEDBACK == 1)
		LONG	TrqRefx;							/* AnalogF/BThrust command							*/
		LONG	PacOut;								/* Proportional control output									*/
		LONG	IacOut;								/* Integral control output									*/
		LONG	Ivar64[2];							/* Integral control calculation								*/
		LONG	AnlgFbMon;							/* AnalogF/BValue monitor[100%]					*/
		USHORT	OverCnt;							/* AnalogF/BExcess detection time counter				*/
		UCHAR	AnlgfbSts_l;						/* Previous analogF/BStatus							*/
		UCHAR	AnlgfbSts_ll;						/* AnalogF/BStatus						*/
		LONG	DTrqLpfoLatch;						/* Disturbance estimated value latch								*/
		UCHAR	SensorContact;						/* AnalogFBSensor contact							*/
		UCHAR	InitDone;							/* Added so that initialization is not performed twice when switching to torque control */
#endif
//		} V;
//} TRQCTRL;
/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for torque filter calculation																*/
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter : ScanAandScanB																*/
/*		Variable for calculation : ScanAandScanB(To ensure torque command continuity when switching control mode)				*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* Constant & variable definition for torque filter					*/
		struct {								/* ScanA : Constant definition for torque filter					*/
#if (FLOAT_USE==TRUE)
		float	Klpf;								/* 1st-order lag filter gain						*/
		float	Knxf[3];							/* Notch filter gain							*/
		float	Klpf2[4];							/* Second-order lag filter gain						*/
		float	Ktrflpf;							/* T-REFFilter								*//* <V102> */
#else
		LONG	Klpf;								/* 1st-order lag filter gain						*/
		LONG	Knxf[3];							/* Notch filter gain							*/
		LONG	Klpf2[4];							/* Second-order lag filter gain						*/
		LONG	Ktrflpf;							/* T-REFFilter								*//* <V102> */
#endif /* FLOAT_USE */
#if	( SVFSEL_ANALOG_FEEDBACK == 1)
		LONG	Kafblpf;							/* AnalogF/BFilter							*/
#endif
		} A;
	/*----------------------------------------------------------------------------------------------*/
		struct {								/* ScanB : Constant definition for torque filter					*/
#if (FLOAT_USE==TRUE)
		float	Knxf[3];							/* Notch filter gain							*/
		float	Klpf2[4];							/* Second-order lag filter gain						*/
		float	Ktrflpf;							/* T-REFFilter								*//* <V102> */
		float	Klpf3;								/* 1st-order lag filter gain(After torque compensation)			*//*<V371>*/
#else
		LONG	Knxf[3];							/* Notch filter gain							*/
		LONG	Klpf2[4];							/* Second-order lag filter gain						*/
		LONG	Ktrflpf;							/* T-REFFilter								*//* <V102> */
		LONG	Klpf3;								/* 1st-order lag filter gain(After torque compensation)			*//*<V371>*/
#endif /* FLOAT_USE */
		} P;
	/*----------------------------------------------------------------------------------------------*/
		struct {								/* Variable data definition for torque filter					*/
#if (FLOAT_USE==TRUE)
		float	FilOut;								/* Torque filter output							*/
		float	FilOut3;							/* Torque filter output(After torque compensation)				*//*<V371>*/
		float	NxFil[3];							/* For notch filter calculation							*/
		float	LpFil2[4];							/* For second-order lag filter calculation						*/
		UCHAR	TrqFilClrCmd;						/* Torque command filter integral initialization command			*/
		UCHAR	TrqFilDisable;						/* Torque command filter invalidation						*//* <V181> */
//		UCHAR	TrqInputPos;						/* Torque disturbance command input position selection					*//* <S0B7> */
#else
		LONG	FilOut;								/* Torque filter output							*/
		LONG	FilOut3;							/* Torque filter output(After torque compensation)				*//*<V371>*/
		LONG	NxFil[3];							/* For notch filter calculation							*/
		LONG	LpFil2[4];							/* For second-order lag filter calculation						*/
		UCHAR	TrqFilClrCmd;						/* Torque command filter integral initialization command			*/
		UCHAR	TrqFilDisable;						/* Torque command filter invalidation						*//* <V181> */
//		UCHAR	TrqInputPos;						/* Torque disturbance command input position selection					*//* <S0B7> */
#endif /* FLOAT_USE */
		} V;
} TRQFIL;
/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for zero speed stop & zero clamp control														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* Constant & variable definition for zero speed control						*/
		struct	{								/* Constant data definition for zero speed control						*/
#if (FLOAT_USE==TRUE)
		float	Kp;									/* Position control proportional gain							*/
		float	SpdLimit;							/* Speed control limit value							*/
#else
		LONG	Kp;									/* Position control proportional gain							*/
		LONG	SpdLimit;							/* Speed control limit value							*/
#endif /* FLOAT_USE */
		} P;
	/*----------------------------------------------------------------------------------------------*/
		struct {								/* Variable data definition for zero speed control						*/
		KSGAIN	PosError;							/* Position deviation										*//*<S00A>*/
		} V;
} ZXCTRL;

/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for carrier frequency switching															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* Constant & variable definition for carrier frequency switching			*/
		struct	{								/* Constant data definition for carrier frequency switching			*/
		BOOL	CarrChange;							/* Carrier frequency switching flag [0:OFF,1:ON]	*/
		LONG	IqLimitA;							/* Carrier switching current thresholdA	[2^24/MaxTrq]	*/
		LONG	IqLimitB;							/* Carrier switching current thresholdB	[2^24/MaxTrq]	*/
		LONG	VelLimitA;							/* Carrier switching speed thresholdA	[2^24/OvrSpd]	*/
		LONG	VelLimitB;							/* Carrier switching speed thresholdB	[2^24/OvrSpd]	*/
		SHORT	NormCarrCnt;						/* Normal carrier count value		[-]				*/
		SHORT	HighCarrCnt;						/* Switching high carrier count value	[-]				*/
		BOOL	MotCarrUse;							/* Motor carrier(PnF25)Enabled 	[0:OFF,1:ON]	*//* Motor carrier frequency switching<S117> */
		SHORT	CarrCnt;							/* Current carrier count value		[-]				*//* Motor carrier frequency switching<S117> */
		} conf;
	/*----------------------------------------------------------------------------------------------*/
		struct {								/* Variable data definition for carrier frequency switching			*/
		BOOL	CarrState;							/* Current carrier setting							*/
		} var;										/*	(0:Low carrier, 1:High carrier)				*/
} CARRCHNG;


/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for carrier frequency switching															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* Constant & variable definition for carrier frequency switching			*/
		struct	{								/* Constant data definition for carrier frequency switching			*/
#if (FLOAT_USE==TRUE)
		float	Kpcmpd;								/* Pcmpd Calculate Gain1						*/
		float	Kpcmpd2;							/* Pcmpd Calculate Gain2						*/
		float	Kpcmpsp1;							/* Pcmps Calculate Gain1						*/
		float	PcmpSpd1;							/* Pcmps Calculate Speed1						*/
		float	KpcmpdItplt;						/* Phase interpolation amount operation coefficient							*/
#else
		LONG	Kpcmpd;								/* Pcmpd Calculate Gain1						*/
		LONG	Kpcmpd2;							/* Pcmpd Calculate Gain2						*/
		LONG	Kpcmpsp1;							/* Pcmps Calculate Gain1						*/
		LONG	PcmpSpd1;							/* Pcmps Calculate Speed1						*/
		LONG	KpcmpdItplt;						/* Phase interpolation amount operation coefficient							*/
#endif /* FLOAT_USE */
		} conf;
	/*----------------------------------------------------------------------------------------------*/
		struct {								/* Variable data definition for carrier frequency switching			*/
#if (FLOAT_USE==TRUE)
		float	Pcmpd;								/* Phase compensation(Delay compensation1)			[65536/360deg]	*/
		float	Pcmpd2;								/* Phase compensation(Delay compensation2)			[65536/360deg]	*/
		float	Pcmps;								/* Phase compensation(Speed compensator)			[65536/360deg]	*/
		float	Pcmps1;								/* Phase compensation(Speed compensation1)			[65536/360deg]	*/
		float	Pcmps2;								/* Phase compensation(Speed compensation2)			[65536/360deg]	*/
		float	PcmpdItplt;							/* Phase interpolation amount									*/
#else
		SHORT	Pcmpd;								/* Phase compensation(Delay compensation1)			[65536/360deg]	*/
		SHORT	Pcmpd2;								/* Phase compensation(Delay compensation2)			[65536/360deg]	*/
		SHORT	Pcmps;								/* Phase compensation(Speed compensator)			[65536/360deg]	*/
		SHORT	Pcmps1;								/* Phase compensation(Speed compensation1)			[65536/360deg]	*/
		SHORT	Pcmps2;								/* Phase compensation(Speed compensation2)			[65536/360deg]	*/
		SHORT	PcmpdItplt;							/* Phase interpolation amount									*/
#endif /* FLOAT_USE */
		} var;
} PHASECMP;


/*--------------------------------------------------------------------------------------------------*/
/*		for Base Loop																				*/
/*--------------------------------------------------------------------------------------------------*/
/* ★Structure needs to be reconsidered★ */
typedef struct BASE_LOOPCTRLS {
	MENCV			*MencV;
	MENCV			*FencV;																			/* <S04D> */
	BPRMDAT			*Bprm;
	MOTSPDMAFIL		*MotSpdMafil;		/* Variable for moving average calculation for motor speed detection						*/
	BASE_CTRL_HNDL	*BaseCtrls;
	GSELGAINS		*GselGains;
	ASIC_HW_IF_STR	*pAsicHwReg;		/* Asic H/W Register address								*//* <S004> */
	ALMMAN_HANDLE	*AlmManager;		/* <S018> */
/* <S1B7> >>>>> */
	MOTLESSTST		*MotorLessTest;		/* Motorless testIFDefinition									*/
	void			(*SencReadPosition)(MENCV *MencV, ASIC_HW_IF_STR *pAsicHwReg,
										ALMMAN_HANDLE *AlmManager, BOOL BeComplete,
										void *BaseLoops);
	LONG 			(*FencReadPosition)(MENCV *FencV,
										ALMMAN_HANDLE *AlmManager, BOOL BeComplete,
										void *BaseLoops);
/* <<<<< <S1B7> */
	/*----------------------------------------------------------------------------------------------*/
	POSCTRL			PosCtrl;			/* Position control variable											*/
	SPDCTRL			SpdCtrl;			/* Speed control variable											*/
//	TRQCTRL			TrqCtrl;			/* Variable for torque control											*/
	TRQFIL			TrqFil;				/* Variable for torque filter										*/
	ZXCTRL			ZxCtrl;				/* Variables for zero speed stop & zero clamp control						*/
	/*----------------------------------------------------------------------------------------------*/
	CARRCHNG		CarrerChange;		/* Variable for carrier frequency switching								*/
	PHASECMP		PhaseComp;			/* Phase compensation variable											*/
	VFBTIMBASE		Vfbtimbase;			/* By base timerFBSpeed correction variable						*/
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	float			MotPos;
	float			MotSpd;
	float			dMotPos;
	float			LastMotPos;
#else
	LONG			MotPos;
	LONG			MotSpd;
	LONG			dMotPos;
	LONG			LastMotPos;
#endif /* FLOAT_USE */
/*Full Closed Encoder Define Start	*//* <S04D> */
#if (FLOAT_USE==TRUE)
	float			FencPos;
	float			FencSpd;
	float			dFencPos;
	float			LastFencPos;
#else
	LONG			FencPos;
	LONG			FencSpd;
	LONG			dFencPos;
	LONG			LastFencPos;
#endif /* FLOAT_USE */
/*Full Closed Encoder Define End	*/
	/*----------------------------------------------------------------------------------------------*/
	USHORT			CtrlMode;			/* Basic loop control mode										*/
	USHORT			LastCtrlMode;		/* Previous basic loop control mode									*//* <S007> */
	USHORT			ZCtrlMode;			/* During zero speed control/zero clamp control							*/
	USHORT			Dummy;
	/*----------------------------------------------------------------------------------------------*/
	LONG 			CmdCtrlBit;
	BOOL			PConActFlg;			/* PControl state confirmation flag										*//* <S064> */
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	float			dPcmda;				/* Position command incremental value input[Command unit]							*/
	float			dPosRefi;			/* Position control: Position command difference input		[Pulse/Scan]			*/
	float			NextdPcmda;			/* Position control: Next position command incremental value input	[Command unit]				*/
	float			NextdPosRefi;		/* Position control: Next position command difference input	[Pulse/Scan]			*/
	float			dPosFbki;			/* Position control: Position FB difference input		[Pulse/Scan]			*/
	PMAFV			pmafvar;
	float 			pmafbuf[4];
	float			SpdRefp;			/* Position control: Speed command output			[2^24/OvrSpd]			*/
	float			SpdRefi;			/* Speed control: Speed command input			[2^24/OvrSpd]			*/
	float			SpdRefo;			/* Speed control: Speed command SFS output		[2^24/OvrSpd]			*/
	float			SpdRefSum;			/* Speed control: Command speed(Ref+FF)		[2^24/OvrSpd]			*/
	float			SpdFbki;			/* Speed control: Speed FB input			[2^24/OvrSpd]			*/
	float			SpdObsFbki;			/* Speed control: Speed FB input(OBSAfter)	[2^24/OvrSpd]			*/
	float			TrqRefs;			/* Speed control: Torque command output			[2^24/MaxTrq]			*/
	float			SpdFFCx;			/* Extension control: Speed FF compensation			[2^24/OvrSpd]			*/
	float			SpdFBCx;			/* Extension control: Speed FB compensation			[2^24/OvrSpd]			*/
	float			TrqFFCx;			/* Extension control: Torque FF compensation			[2^24/MaxTrq]			*/
	float			TrqFBCx;			/* Extension control: Torque FB compensation			[2^24/MaxTrq]			*/
	float			TrqRefi;			/* Torque filter: Torque command input	[2^24/MaxTrq]			*/
	float			TrqRefo;			/* Torque filter: Torque command output	[2^24/MaxTrq]			*/
	float			TrqRefo_l;			/* Torque filter: Torque command output	[2^24/MaxTrq]			*/
	float			TrqRefo_a;			/* Torque filter: Torque command output	[2^24/MaxTrq]			*/
	float			TrqRefoLimit;		/* Torque command after torque limit			[2^24/MaxTrq]			*/
	float			TrqRefoComp;		/* Torque command output after correction				[2^24/MaxTrq]			*/
	float			DisTrqRef3;			/* Torque disturbance command input				[2^24/MaxTrq]			*/
	float			MotSpd_l;			/* Motor Speed						[2^24/OvrSpd]			*/
	float			MotSpd_a;			/* Motor Speed						[2^24/OvrSpd]			*/
#else
	LONG			dPcmda;				/* Position command incremental value input[Command unit]							*/
	LONG			dPosRefi;			/* Position control: Position command difference input		[Pulse/Scan]			*/
	LONG			NextdPcmda;			/* Position control: Next position command incremental value input	[Command unit]				*/
	LONG			NextdPosRefi;		/* Position control: Next position command difference input	[Pulse/Scan]			*/
	LONG			dPosFbki;			/* Position control: Position FB difference input		[Pulse/Scan]			*/
	PMAFV			pmafvar;
	LONG 			pmafbuf[4];
	LONG			SpdRefp;			/* Position control: Speed command output			[2^24/OvrSpd]			*/
	LONG			SpdRefi;			/* Speed control: Speed command input			[2^24/OvrSpd]			*/
	LONG			SpdRefo;			/* Speed control: Speed command SFS output		[2^24/OvrSpd]			*/
	LONG			SpdRefSum;			/* Speed control: Command speed(Ref+FF)		[2^24/OvrSpd]			*/
	LONG			SpdFbki;			/* Speed control: Speed FB input			[2^24/OvrSpd]			*/
	LONG			SpdObsFbki;			/* Speed control: Speed FB input(OBSAfter)	[2^24/OvrSpd]			*/
	LONG			TrqRefs;			/* Speed control: Torque command output			[2^24/MaxTrq]			*/
	LONG			SpdFFCx;			/* Extension control: Speed FF compensation			[2^24/OvrSpd]			*/
	LONG			SpdFBCx;			/* Extension control: Speed FB compensation			[2^24/OvrSpd]			*/
	LONG			TrqFFCx;			/* Extension control: Torque FF compensation			[2^24/MaxTrq]			*/
	LONG			TrqFBCx;			/* Extension control: Torque FB compensation			[2^24/MaxTrq]			*/
	LONG			TrqRefi;			/* Torque filter: Torque command input	[2^24/MaxTrq]			*/
	LONG			TrqRefo;			/* Torque filter: Torque command output	[2^24/MaxTrq]			*/
	LONG			TrqRefo_l;			/* Torque filter: Torque command output	[2^24/MaxTrq]			*/
	LONG			TrqRefo_a;			/* Torque filter: Torque command output	[2^24/MaxTrq]			*/
	LONG			TrqRefoLimit;		/* Torque command after torque limit			[2^24/MaxTrq]			*/
	LONG			TrqRefoComp;		/* Torque command output after correction				[2^24/MaxTrq]			*/
	LONG			DisTrqRef3;			/* Torque disturbance command input				[2^24/MaxTrq]			*/
	LONG			MotSpd_l;			/* Motor Speed						[2^24/OvrSpd]			*/
	LONG			MotSpd_a;			/* Motor Speed						[2^24/OvrSpd]			*/
#endif /* FLOAT_USE */
	/*----------------------------------------------------------------------------------------------*/
	SHORT			IdRef;				/* dAxis current command						[15000/Imax]			*/
	SHORT			IqRef;				/* qAxis current command						[15000/Imax]			*/
	SHORT			IqDist;				/* qAxis disturbance torque					[15000/MaxTrq]			*/
	BOOL			TrqClamp;
	/*----------------------------------------------------------------------------------------------*/
	SHORT			PowMonVq;			/* For power consumption monitor calculationqAxis voltage								*//* <S020> */
	SHORT			PowMonIqd;			/* For power consumption monitor calculationqAxis current								*//* <S020> */
	SHORT			PowMonVd;			/* For power consumption monitor calculationdAxis voltage								*//* <S020> */
	SHORT			PowMonIdd;			/* For power consumption monitor calculationdAxis current								*//* <S020> */
	/*----------------------------------------------------------------------------------------------*/
/*Full Closed Encoder Define Start	*//* <S04D> */
#if (FLOAT_USE==TRUE)
	float			FencSpd_l;			/* PreviousLoad Speed					[2^24/OvrSpd]			*/
	float			FencSpd_a;			/* AverageLoad Speed					[2^24/OvrSpd]			*/
	float			SpdObsFbki_l;		/* PreviousLoad Speed					[2^24/OvrSpd]			*/
	float			SpdObsFbki_a;		/* AverageLoad Speed					[2^24/OvrSpd]			*/
#else
	LONG			FencSpd_l;			/* PreviousLoad Speed					[2^24/OvrSpd]			*/
	LONG			FencSpd_a;			/* AverageLoad Speed					[2^24/OvrSpd]			*/
	LONG			SpdObsFbki_l;		/* PreviousLoad Speed					[2^24/OvrSpd]			*/
	LONG			SpdObsFbki_a;		/* AverageLoad Speed					[2^24/OvrSpd]			*/
	/*----------------------------------------------------------------------------------------------*/
#endif /* FLOAT_USE */
/*Full Closed Encoder Define End	*/
	struct {
		UCHAR	PastInputClrEdge; 		/* APDedicated flag!!											*/
		UCHAR	PerClrEdge;		 		/* APDedicated flag!!											*/
		UCHAR	Reserved1;		 		/* APDedicated flag!!											*/
		UCHAR	Reserved2;		 		/* APDedicated flag!!											*/
	} f;
	/*----------------------------------------------------------------------------------------------*/
	LONG			TRefAd;				/* Torque Reference [AD value] <S051> 						*/

	RPLCMP	RippleCmp;				/* Ripple compensation variable										*//* <S14B> */
} BASE_LOOPCTRLS;


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/* Baseloops */
void	SysScanAInit( BASE_LOOPCTRLS *BaseLoops, MENCV *MencV, MENCV *FencV, BPRMDAT *Bprm, MOTSPDMAFIL *MotSpdMafil,
		 	 BASE_CTRL_HNDL *BaseCtrls, GSELGAINS *GselGains,ASIC_HW_IF_STR *pAsicHwReg, ALMMAN_HANDLE 	*AlmManager, /* <S04D>:Argument addition */
		 	 MOTLESSTST *MotorLessTest );	/* <S1B7>:Argument addition */
void	KpxInputScanA( BASE_LOOPCTRLS *BaseLoops );/*<S0C5>*/
ALMID_T	KpxOutputScanA( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut, ASICS *SvAsicRegs, PRMDATA *Prm );
//void	SysInputScanAProcessData( BASE_LOOPCTRLS *BaseLoops, BASE_CTRL_HNDL *pBaseCtrl, BASE_CTRL_OUT *BaseCtrlOut );
void	SysInputScanAProcessData( BASE_LOOPCTRLS *BaseLoops, BASE_CTRL_HNDL *pBaseCtrl,
													BASE_CTRL_OUT *BaseCtrlOut, CMD_CTRLBITS *CmdCtrl );/* <S050> */
void 	SysOutputScanAProcessData( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut ); /* 2012.12.27 Y.Oka Provisional measures */
void	ApxYecServoControl( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut ); /* <S0B8> */


/* BaseControls */
void BpiYecServoControl( BASE_LOOPCTRLS*, SERVO_CONTROL_IF*, CTRL_LOOP_OUT*, FUN_CMN_CONTROL*, BPRMDAT* );/* <S04D> */
void	BpiMpoleFindMakePosCmd( BASE_LOOPCTRLS *BaseLoops, SERVO_CONTROL_IF *SvControlIf,
													FUN_CMN_CONTROL *FnCmnCtrl, BPRMDAT *Bprm);/* <S04D> */
void BpiTorqueLimitControl( TRQ_LMT_DATA*, BASE_CTRL_HNDL*,	BOOL, ALMMAN_HANDLE*); /* <S145> */
void BpiChangeSpdDetMaFil( MOTSPDMAFIL*, LONG num );		/*<S02F>*/
LONG BpiBasePositionControl( POS_LOOP_CTRL *PosLoop, CTRL_CMD_MNG *CtrlCmd,
								 POS_MNG_HNDL *PosManager, KSGAIN Kp, KSGAIN Kpi );
void BpiInputMotorPosition(BASE_CTRL_HNDL *BaseControls, ASIC_U_IF_STR *pAsicMicroReg,
							BPRMDAT *Bprm, PRMDATA *Prm, CTRL_LOOP_OUT *CtrlLoopOut);/* <S04D>:Argument arrangement *//* <S086> */
void BpiOutputNetStatus(BASE_CTRL_HNDL *BaseControls, SERVO_CONTROL_IF *SvControlIf,
                        	CTRL_LOOP_OUT *CtrlLoopOut, SEQ_CTRL_OUT *SeqCtrlOut, BPRMDAT *Bprm, PLSIFV *pPls);/* <S04D>:Argument addition */
void BpiInputControlCMD( BASE_CTRL_HNDL *BaseControls, SERVO_CONTROL_IF *SvControlIf,
                         CTRL_LOOP_OUT *CtrlLoopOut, SEQ_CTRL_OUT *SeqCtrlOut, USHORT uwd_l, LONG ax_no);/* <S1C0>: Add argument */

void InitBasePositionControl( POSCTRL *PosCtrl );	/* <S14C> */

#endif	/* _BASE_CONTROLS_H_*/
/***************************************** end of file **********************************************/


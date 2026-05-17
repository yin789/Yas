#ifndef CTRL_LOOP_HANDLER_H
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		CtrlLoopHandler.h : Control loop handle definition													*/
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
#include "Basedef.h"
#include "BacklashComp.h"
#include "MLib.h"


/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
#define	MAX_NETCYCLE			4000/125		/* NETRequest maximum waiting period(Transmission cycle/control cycle)			*/



/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for zero speed stop & zero clamp control														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct ZX_CTRL_HNDL {					/* Constant & variable definition for zero speed control						*/
	struct	{									/* Constant data definition for zero speed control						*/
		LONG	Kp;									/* Position control proportional gain							*/
		LONG	SpdLimit;							/* Speed control limit value							*/
		LONG	OrgSrchSpeed;						/* Origin search speed				[2^24/OvrSpd]	*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {									/* Variable data definition for zero speed control						*/
		LONG	PosError;							/* Position deviation										*/
		BOOL	OrgSrchFinish;						/* Origin search completeflg							*/
	} var;
} ZX_CTRL_HNDL;


/*--------------------------------------------------------------------------------------------------*/
/*		Mode Switch Select definition																*/
/*--------------------------------------------------------------------------------------------------*/
enum MODE_SW_TYPE {
	MODESW_TRQREF = 0x00,						/* Mode switch Torque command						*/
	MODESW_SPDREF = 0x01,						/* Mode switch Speed command							*/
	MODESW_MOTACC = 0x02,						/* Mode switch Acceleration							*/
	MODESW_POSERR = 0x03,						/* Mode switch Position deviation							*/
	MODESW_NOTUSE = 0x04,						/* Mode switch Not used							*/
};
/*--------------------------------------------------------------------------------------------------*/
/*		ModeSW																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct MODE_SW {						/* ModeSW					*/
	struct {									/* ModeSW							*/
		LONG		ModeSWSel;						/* Mode switch selection							*/
		LONG		MSWTrqLevel;					/* Mode switch torque level					*/
		LONG		MSWSpdLevel;					/* Mode switch speed level						*/
		LONG		MSWAccLevel;					/* Mode switch acceleration level					*/
		LONG		MSWErrLevel;					/* Mode switch deviation level						*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {									/* ModeSW							*/
		BOOL		ModeSWFlag;						/* Mode switchON Flag						*/
		LONG		Acceleration;					/* Acceleration for mode switch						*/
	} var;
} MODE_SW;


/*--------------------------------------------------------------------------------------------------*/
/*		Parameter definition for position deviation calculation																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct PERRB {								/* Structure definition for position deviation creation						*/
	LONG		LastdPosRef;						/* Previous position command increment value[Command unit]					*/
	LONG		PerrComp;							/* Correction value for positional calculation[Command unit]					*/
	LONG		per32s;								/* Position deviation output(Rounding)						*/
	LONG		per32a;								/* Absolute value as above(Rounding)							*/
} PERRB;

/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for position control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct POS_LOOP_CTRL {						/* Position control constant & variable definition							*/
//	struct	{									/* Position control constant data definition							*/
//		LONG	Kpi;								/* Position control proportional integral gain						*/
//		LONG	Kpd;								/* Position control proportional differential gain						*/
//		LONG	VelBias;							/* Position control speed bias							*/
//		LONG	BiasAddLevel;						/* Speed bias addition width		[Pulse, mm]			*/
//		LONG	KdPff;								/* Position control position increment value FF gain					*/
//		LONG	KdPffFil;							/* Position control position increment value FF filter gain			*/
//	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {									/* Position control variable data definition							*/
		BOOL	LoopStart;
//		LONG	SpdRef;								/* Position control output									*/
		LONG	PerIvar64[2];						/* For position deviation integral calculation							*/
//		LONG	PacOut;								/* Proportional control output									*/
//		LONG	IacOut;								/* Integral control output									*/
//		LONG	DacOut;								/* Differential control output									*/
//		LONG	LastPacOut;							/* Previous proportional control output								*/
//		LONG	dPFfFilo;							/* Position increment value FF filter output					*/
	} var;
} POS_LOOP_CTRL;

/*--------------------------------------------------------------------------------------------------*/
/*		Position management parameter definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct POS_MNG_HNDL {					/* Position management data definition								*/
	struct	{									/* Position management constant data definition							*/
		EGEAR		InvalidEgear;					/* Gear ratio when electronic gear function is disabled(1:1)				*/
		EGEAR		Egear;							/* Electronic gear ratio information								*/
		UCHAR		CoinOutSel;						/* /COINOutput timing selection setting					*/
		LONG		CoinLevel;						/* Positioning completion width					[Command unit]	*/
		LONG		NearLevel;						/* NearSignal width						[Command unit]	*/
		LONG		PosErrDelayShift;				/* Number of correction time shifts for position deviation monitor				*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct	{									/* Position management constant data definition							*/
		PERRA		PosErrA;						/* Position deviation(Command unit)					<S14C>	*/
		LONG		dPcmdEgear;						/* Position command electronic gear output							*/
		LONG		dPcmdEgrem;						/* Position command electronic gear calculation remainder						*/
		LONG		dPcmda;							/* Position command incremental value input[Command unit]					*/
		LONG		PosErr;							/* Position amplifier deviation(Pulse)				<S14C>	*/
		LONG		Per64[2];						/* Position amplifier deviation calculation(Pulse)					*/
//		LONG		LastdPcmda;						/* Previous position command increment value[Command unit]					*/
	/*----------------------------------------------------------------------------------------------*/
		BOOL		EgearDisable;					/* Electronic gear disabled									*/
		BOOL		PcmdFilStop;					/* Position command filter disabled							*/
//		UCHAR		MCphOutTime;					/* MotorCFor phase output time control						*/
//		UCHAR		FCphOutTime;					/* FulkCFor phase output time control						*/
//		UCHAR		SpareUc[2];						/* Preliminary(AlignmentFor adjustment)						*/
	/*----------------------------------------------------------------------------------------------*/
	} var;
	EGEAR		*Egear;								/* Electronic gear switching pointer					*/
	BLSHCMP		BlshCmp;							/* For backlash correction calculation						*/
	APOSRG		PgPos;								/* Mechanical coordinate systemFBPosition							*/
	APOSRG		MencPos;							/* MotorPGofFBPosition								*/
	APOSRG		FencPos;							/* FulkPGofFBPosition								*/
	LONG		VirtualPositionError;				/* Virtual position deviation(Command unit)				<S14C>	*/
/*--------------------------------------------------------------------------------------------------*/
/* 2012.09.07 Y.Oka ★Interim Review other structures or processing itself★ */
	LONG			dPosRefi;				/* Position control: Position command difference input			[Pulse/Scan]	*//* <S187> */
	LONG			CompdPosRefi;			/* Position command difference input(ScanB->ScanA)		[Pulse/Scan]	*/
	LONG			CompdPcmda;				/* Position command incremental value input(ScanB->ScanA)		[Command unit]		*/
	USHORT			RefRenewCntA;			/* For position command update processingBaseLoopCounter					*/
	USHORT			RefRenewCntB;			/* For position command update processingBaseCtrlCounter					*/
	USHORT			RefRenewalChkCnt;		/* For position command update check								*/
/* 2012.09.07 Y.Oka ★Interim Review other structures or processing itself★ */
} POS_MNG_HNDL;


/*--------------------------------------------------------------------------------------------------*/
/*		Parameter definition for control calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CTRL_CMD_PRM {
	struct	{								/* Definition for no adjustment										*/
		KSGAIN			Klpf3;					/* 1Next lag filter gain(After torque compensation)				*/
		KSGAIN			SpdRefPhFil;			/* For phase advanceLPF									*/
		KSGAIN			SpdRefPhFilGn;			/* Phase lead compensation gain								*/
	} TLPrm;
	KSGAIN			OverTrqLoopGain[3];		/* Loop gain for speed limit								*/
	KSGAIN			OverTrqIntegGain[3];	/* Integral gain for speed limit									*/
	LONG			OverTrqIntegBuf[2];		/* Over Torque Integral Gain Calculation Buffer			*/
	KSGAIN			Klpf;					/* 1Next lag filter gain								*/
	KSGAIN			Klpf2[4];				/* 2Next lag filter gain								*/
	KSGAIN			PI_rate;				/* Speed controlPIControl ratio									*/
	KSGAIN			KVfbFil;				/* SpeedFBFilter gain									*/
	KSGAIN			Kpi;					/* Position control proportional integral gain								*/
	LONG			SvonSpdLmtValue;		/* Speed limit value when servo is on								*/
	/*----------------------------------------------------------------------------------------------*/
	KSGAIN			KpiFc;					/* Position control proportional integral gain for Fulk						*/
	KSGAIN			KdPff;					/* Position control position increment valueFFGain							*/
	KSGAIN			KdPffFil;				/* Position control position increment valueFFFilter gain					*/
	BOOL			LpassFil2;				/* 2Next lag filter selection									*/
	BOOL			LpassFil3;				/* After torque compensation1Next lag filter selection						*/
	/*----------------------------------------------------------------------------------------------*/
	KSGAIN			KVrfFil;				/* Speed command filter gain								*/
	BOOL			IPSpdControl;			/* I-PControl selection setting										*/
	BOOL			SpdIctrlKeep;			/* Speed control integral hold										*/
	BOOL			VrefSpdLmt;				/* Use speed command as external speed limit						*/
#if (FLOAT_USE==TRUE)
	float			TConSpdLmtLevel;		/* Speed limit level during torque control		 [2^24/OvrSpd]		*/
	float			SpdSfsAcc;				/* Soft start acceleration amount									*/
	float			SpdSfsDec;				/* Soft start deceleration amount									*/
	float			SvonSpdLmtLevel;		/* Speed limit level when servo is on							*/
	float			SpdLdstpDec;			/* Linear deceleration stop deceleration amount(Pn30A)							*/ /* <S1F5> */
#else
	LONG			TConSpdLmtLevel;		/* Speed limit level during torque control		 [2^24/OvrSpd]		*/
	LONG			SpdSfsAcc;				/* Soft start acceleration amount									*/
	LONG			SpdSfsDec;				/* Soft start deceleration amount									*/
	LONG			SvonSpdLmtLevel;		/* Speed limit level when servo is on							*/
	LONG			SpdLdstpDec;			/* Linear deceleration stop deceleration amount(Pn30A)							*/ /* <S1F5> */
#endif /* FLOAT_USE */
} CTRL_CMD_PRM;


/*--------------------------------------------------------------------------------------------------*/
/*		Work definition for control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CTRL_CMD_MNG {
	LONG			CtrlMode;				/* Basic loop control mode									*/
	LONG			LastCtrlMode;			/* Previous basic loop control mode								*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	float			TrqRefo;				/* Torque command output					[2^24/MaxTrq]		*/
	float			SpdRefo;				/* Speed command output						[2^24/OvrSpd]		*/
	float			SpdLdstpRefo;			/* Linear deceleration stop speed command output								*/ /* <S1F5> */
	LONG			dPosRefo;				/* Position command difference output					[Pulse/Scan]		*/
/*--------------------------------------------------------------------------------------------------*/
	LONG			BaseFbkPos;				/* Motor position when switching control		[Pulse]				*/
	LONG			dPosFbki;				/* Position control: Position FB difference input		[Pulse/Scan]		*/
	LONG			PosFbki;				/* Position control: Position FB input			[Pulse/Scan]		*/
	LONG			FbkPosFromCpos;			/* Motor position from origin		 [+/-(PulseNo/2)],[LM:float]	*/
/*--------------------------------------------------------------------------------------------------*/
	BOOL			TrqFilClrCmd;			/* Torque command filter integral initialization request						*/
	BOOL			TrqFilDisable;			/* Torque command filter invalidation request							*/
//	float			TrqInputPos;			/* Torque disturbance command input position selection							*//* <S0B7> */
	float			SpdLimit;				/* Speed limit value during torque control								*/
	float			OverTrqIntegBuf[2];		/* Integral buffer for speed limit during torque control					*/
/*--------------------------------------------------------------------------------------------------*/
	LONG			OverTrq;				/* @@ Over Torque [2^24/MaxTrq]							*/
	BOOL			SpdClamp;				/* @@ Speed Clamp Flag on Torque Control Mode			*/
/*--------------------------------------------------------------------------------------------------*/
	BOOL			TrqClamp;				/* <S045> Torque Clamp Flag								*/
/*--------------------------------------------------------------------------------------------------*/
	float			SpdRefi;				/* Speed command input						[2^24/OvrSpd]		*/
	float			SpdRfFilo;				/* Speed command filter output				[2^24/OvrSpd]		*/
	float			SpdFbki;				/* SpeedFBInput						[2^24/OvrSpd]		*/
	float			SpdObsFbki;				/* SpeedFBInput(OBSAfter)				[2^24/OvrSpd]		*/
	float			VelIvar64[2];			/* Integral control calculation(For shock reduction when switching control)			*/
/*--------------------------------------------------------------------------------------------------*/
	float			PosRefLo;				/* Command position(Low)					[Command unit]			*/
	float			PosRef;					/* Command position during position control				[Command unit]			*/
	PMAFV			Pmafvar;				/* Position command moving average filter calculation variable					*/
	LONG			dPcmdFilo;				/* Position command filter output				[Pulse/Scan]		*/
	LONG			Pmafbuf[MAX_NETCYCLE];	/* Position command moving average filter calculationBUF					*/
	float			dPFfFilo;				/* Position increment valueFFFilter output								*/
	BOOL			PicvClrCmd;				/* Position deviation integral clear request								*/
	BOOL			PerrClrReq;				/* Position deviation clear request(Table operation/PJOGDedicated)			*/
	BOOL			PerClrEdge;				/* @@  Position deviation clear edge request(ANLGFor)					*/
	LONG			ClrFbPos;				/* @@  Position deviation clear latch position(ANLGFor)					*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
	LONG			SpdFFCx;				/* Extension control: SpeedFFCompensation				[2^24/OvrSpd]		*/
	float			SpdFBCx;				/* Extension control: SpeedFBCompensation				[2^24/OvrSpd]		*//* 2013/04/01VersionMD, it is commented out.	*/	
	LONG			TrqFFCx;				/* Extension control: TorqueFFCompensation			[2^24/MaxTrq]		*/
	float			TrqFBCx;				/* Extension control: TorqueFBCompensation			[2^24/MaxTrq]		*/
#else
	LONG			TrqRefo;				/* Torque command output					[2^24/MaxTrq]		*/
	LONG			SpdRefo;				/* Speed command output						[2^24/OvrSpd]		*/
	LONG			SpdLdstpRefo;			/* Linear deceleration stop speed command output								*/ /* <S1F5> */
	LONG			dPosRefo;				/* Position command difference output					[Pulse/Scan]		*/
/*--------------------------------------------------------------------------------------------------*/
	LONG			BaseFbkPos;				/* Motor position when switching control		[Pulse]				*/
	LONG			dPosFbki;				/* Position control: Position FB difference input		[Pulse/Scan]		*/
	LONG			PosFbki;				/* Position control: Position FB input			[Pulse/Scan]		*/
	LONG			FbkPosFromCpos;			/* Motor position from origin		 [+/-(PulseNo/2)],[LM:LONG]	*/
/*--------------------------------------------------------------------------------------------------*/
	BOOL			TrqFilClrCmd;			/* Torque command filter integral initialization request						*/
	BOOL			TrqFilDisable;			/* Torque command filter invalidation request							*/
//	LONG			TrqInputPos;			/* Torque disturbance command input position selection							*//* <S0B7> */
	LONG			SpdLimit;				/* Speed limit value during torque control								*/
	LONG			OverTrqIntegBuf[2];		/* Integral buffer for speed limit during torque control					*/
/*--------------------------------------------------------------------------------------------------*/
	LONG			OverTrq;				/* @@ Over Torque [2^24/MaxTrq]							*/
	BOOL			SpdClamp;				/* @@ Speed Clamp Flag on Torque Control Mode			*/
/*--------------------------------------------------------------------------------------------------*/
	BOOL			TrqClamp;				/* <S045> Torque Clamp Flag 							*/
/*--------------------------------------------------------------------------------------------------*/
	LONG			SpdRefi;				/* Speed command input						[2^24/OvrSpd]		*/
	LONG			SpdRfFilo;				/* Speed command filter output				[2^24/OvrSpd]		*/
	LONG			SpdFbki;				/* SpeedFBInput						[2^24/OvrSpd]		*/
	LONG			SpdObsFbki;				/* SpeedFBInput(OBSAfter)				[2^24/OvrSpd]		*/
	LONG			VelIvar64[2];			/* Integral control calculation(For shock reduction when switching control)			*/
/*--------------------------------------------------------------------------------------------------*/
	LONG			PosRefLo;				/* Command position(Low)					[Command unit]			*/
	LONG			PosRef;					/* Command position during position control				[Command unit]			*/
	PMAFV			Pmafvar;				/* Position command moving average filter calculation variable					*/
	LONG			dPcmdFilo;				/* Position command filter output				[Pulse/Scan]		*/
	LONG			Pmafbuf[MAX_NETCYCLE];	/* Position command moving average filter calculationBUF					*/
	LONG			dPFfFilo;				/* Position increment valueFFFilter output								*/
	BOOL			PicvClrCmd;				/* Position deviation integral clear request								*/
	BOOL			PerrClrReq;				/* Position deviation clear request(Table operation/PJOGDedicated)			*/
	BOOL			PerClrEdge;				/* @@  Position deviation clear edge request(ANLGFor)					*/
	LONG			ClrFbPos;				/* @@  Position deviation clear latch position(ANLGFor)					*/
/*--------------------------------------------------------------------------------------------------*/
	LONG			SpdFFCx;				/* Extension control: SpeedFFCompensation				[2^24/OvrSpd]		*/
	LONG			SpdFBCx;				/* Extension control: SpeedFBCompensation				[2^24/OvrSpd]		*//* 2013/04/01VersionMD, it is commented out.	*/	
	LONG			TrqFFCx;				/* Extension control: TorqueFFCompensation			[2^24/MaxTrq]		*/
	LONG			TrqFBCx;				/* Extension control: TorqueFBCompensation			[2^24/MaxTrq]		*/
#endif /* FLOAT_USE */
} CTRL_CMD_MNG;



#define CTRL_LOOP_HANDLER_H
#endif /* CTRL_LOOP_HANDLER_H */
/***************************************** end of file **********************************************/

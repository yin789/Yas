#ifndef MPF_HANDLER_H
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MpfHandler.h : Handle definition for magnetic pole detection														*/
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

/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Definition of state management variables for magnetic pole detection execution																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct MPFEXEINFO {				/* Structure definition												*/
	BOOL	HaltCondition;					/* Over Travel or not Servo Ready State					*/
	BOOL	MotStopSts;						/* Motor stop status										*/
	LONG	dMotPosition;					/* PositionFBIncrement value						[pulse/ScanB]		*/
	LONG	TrefMonitor;					/* Torque command output(Average value)			[2^24/MaxTrq]		*/
	LONG	SpdFbMonitor;					/* SpeedFBInput						[2^24/OvrSpd]		*/
	KSGAIN	KVfbFil;						/* SpeedFBFilter gain									*/
	KSGAIN	Kphasepos;						/* Gain for Motor Phase Position Cal.					*/
	KSGAIN	Kmovpos;						/* Gain for Linear Motor Move Position Cal.				*/
} MPFEXEINFO;



/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for magnetic pole detection																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct MPFIND {						/* Structure definition											*/
	struct	{								/* Constant data definition for magnetic pole detection								*/
		KSGAIN	Kv;								/* Speed loop gain				[0.1Hz]				*/
		KSGAIN	Ki;								/* Speed loop integral gain			[0.01ms]			*/
		KSGAIN	InteglessGain;					/* Integral less gain				[%]					*/
		KSGAIN	Klpf;							/* 1Next lag filter gain							*/
		LONG	MaxSpd;							/* Speed command peak value				[2^24/OvrSpd]		*/
		LONG	dSpd;							/* Speed command difference value				[2^24/OvrSpd/Cycle]	*/
		LONG	ReverseSpd;						/* Reverse running level(At speed command)		[2^24/OvrSpd]		*/
		LONG	ReverseSpd2;					/* Reverse running level(Waiting time)		[2^24/OvrSpd]		*/
		LONG	OverSpd;						/* Speed over detection value			[2^24/OvrSpd]		*/
		LONG	DetectRange;					/* Moving range						[0.01mm]			*/
		LONG	MaxTrq;							/* Thrust command peak value				[2^24/MaxTrq]		*/
		LONG	dTrq;							/* Thrust command difference value				[2^24/MaxTrq/Cycle]	*/
		LONG	ErrOKLevel;						/* Detection errorOKLevel				[Pluse]				*/
		LONG	AccTime;						/* Acceleration/deceleration time					[Times/Cycle]		*/
		LONG	ClmpTime;						/* Constant speed time					[Times/Cycle]		*/
		LONG	WaitTime;						/* Waiting time						[Times/Cycle]		*/
		LONG	InteglessTime;					/* Integral less start time				[Times/Cycle]		*/
		LONG	IncTime;						/* Thrust command adjustment time				[Times/Cycle]		*/
		LONG	ConstTime;						/* Thrust finger constant time				[Times/Cycle]		*/
		LONG	RepeatNum;						/* Number of magnetic pole detection repetitions								*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct	{								/* Constant data definition for magnetic pole detection								*/
		BOOL	Fail;							/* Error occurrence(Speed deviation or Torque limit)				*/
		BOOL	Direction;						/* Operation command direction										*/
		BOOL	AxisChange;						/* d-qAxis torque application flag							*/
		BOOL	ClrIntegral;					/* Clear integral buffer								*/
		BOOL	PhaseChange;					/* Phase change											*/
		BOOL	ZeroTorque;						/* Zero torque command output flag							*/
		BOOL	RangeFix;						/* Detection area determination										*/
		BOOL	InteglessON;					/* 0: Disable integration without1: Execute without integration					*/
		BOOL	ReverseLevel;					/* Wrong running judgment level(0: Speed command in progress,1: During waiting time)		*/
		BOOL	OSFail;							/* Overspeed									*/
		BOOL	PhaseConfFlg;					/* Torque control request flag for confirming magnetic pole detection results			*/
		BOOL	NoGood;							/* Angle calculation error information(0:OK,1: Increase gain,	*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	SpdErr;							/* Speed deviation											*/
		LONG	Ivar64[2];						/* Speed loop integral buffer							*/
		LONG	dAxisMaxTrqPlus;				/* forward directiondMaximum axis torque								*/
		LONG	qAxisMaxTrqPlus;				/* forward directionqMaximum axis torque								*/
		LONG	dAxisMaxTrqMinus;				/* Negative directiondMaximum axis torque								*/
		LONG	qAxisMaxTrqMinus;				/* Negative directionqMaximum axis torque								*/
		LONG	dAxisMaxSpdPlus;				/* forward directiondMaximum axis speed								*/
		LONG	qAxisMaxSpdPlus;				/* forward directionqMaximum axis speed								*/
		LONG	dAxisMaxSpdMinus;				/* Negative directiondMaximum axis speed								*/
		LONG	qAxisMaxSpdMinus;				/* Negative directionqMaximum axis speed								*/
		LONG	AxisErr;						/* d-qAxis misalignment										*/
		LONG	MovPos;							/* Magnetic pole detection movement amount									*/
		LONG	TrqRefPhaConf;					/* Thrust command for magnetic pole detection confirmation							*/
		LONG	MovPosPhaConf;					/* Position for magnetic pole detection confirmation								*/
		LONG	PhsOffsetComp;					/* Magnetic pole detection position correction amount								*/
		LONG	SpdOffset;						/* Speed before operation start									*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	AdjSwpDeg;						/* Adjustment phase change increments									*/
		LONG	AdjDegree;						/* Adjustment phase											*/
		LONG	PastTime;						/* Elapsed time after start of operation								*/
		LONG	RefOutTime;						/* Time work buffer(Only in speed command creation function)		*/
		LONG	PhsOffsetCalRslt;				/* Magnetic pole detection position(Theoretical value calculation result)						*/
		LONG	PhaseOffsetCalComp;				/* Correction amount after confirmation of magnetic pole detection operation(For monitor)				*/
		LONG	DegReverse[4];					/* Angle during reverse running										*/
		LONG	DegRev;							/* Composite angle during reverse running									*/
		LONG	DegFinal;						/* Final composite angle										*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	KvGain;							/* Gain automatic adjustment parameter							*/
		LONG	Step;							/* Magnetic pole detection sequence step						*/
		LONG	RefOutStep;						/* Command output sequence step						*/
		LONG	RangeFixStep;					/* Magnetic pole area detection sequence step					*/
		LONG	Times;							/* Number of repetitions										*/
		LONG	FailTimes;						/* Number of failures											*/
		LONG	AxisErrSign;					/* d-qAxis misalignment direction(Torque)							*/
		LONG	AreaCheckStep;					/* 8Division area check step						*/
		LONG	ReverseCtr;						/* Number of reverse running											*/
		LONG	ConfirmStep;					/* Magnetic pole detection result confirmation step							*/
		LONG	SpdFbFilo;						/* SpeedFBFilter output	<V302>						*/
	} var;
	BOOL	MpfSeqStartReq;						/* Magnetic pole detection sequence start request						*/
	LONG	MpFindSpdRef;						/* Magnetic pole detection speed command output								*/
	LONG	TrqFilOut;							/* Torque filter output								*/
	LONG	MpfPhaseOffset;						/* Magnetic pole detection phase offset 		[65536/360deg]		*/
	LONG	MPJudgeMonitor[3];					/* Magnetic pole detection judgment monitor								*/
	LONG	UnMpFindingTime;					/* Un082  : Magnetic pole detection time required		[0.1s]			*/
	LONG	UnMpfMovePosP;						/* Un080  : Maximum movement in the positive direction for magnetic pole detection[0.01mm]		*/
	LONG	UnMpfMovePosN;						/* Un081  : Magnetic pole detection maximum reverse direction travel amount[0.01mm]		*/
} MPFIND;


#define MPF_HANDLER_H
#endif /* MPF_HANDLER_H */
/***************************************** end of file **********************************************/

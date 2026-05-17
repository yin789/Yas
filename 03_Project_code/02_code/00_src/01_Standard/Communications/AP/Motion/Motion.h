/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Motion.h : Motion Module Common Header file 												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Motion Common header file																	*/
/*																									*/
/*		1. Global Function Prototypes ( MtXXX.h )													*/
/*																									*/
/*		2. Global Data Definition ( MtXXX.h )														*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2012.09.05  K.Ozaki																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#ifndef		_MOTION_H_
#define		_MOTION_H_

#include "KnlApi.h"					/* Kernel API */

typedef SV_CYC_CMD CYC_CMD;			/* Interim */
typedef SV_CYC_RSP CYC_RSP;			/* Interim */
typedef UINT RSP_SEQBITS;			/* Interim */
typedef UINT RSP_LTBITS;			/* Interim */
typedef UINT CMD_SEQBITS;			/* Interim */
typedef UINT CMD_LTBITS;			/* Interim */

/****************************************************************************************************/
/*																									*/
/*	Grobal Definitions																				*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/* defines : Control Mode 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define MT_NO_CTRLMD	(0)		/* Control invalid mode */
#define MT_POS_CTRLMD	(1)		/* Position control mode */
#define MT_VEL_CTRLMD	(2)		/* Speed control mode */
#define MT_TRQ_CTRLMD	(3)		/* Torque control mode */

/*--------------------------------------------------------------------------------------------------*/
/* defines : Latch mode(APIFor setting) */
/*--------------------------------------------------------------------------------------------------*/
#define MOTION_LTMD			1	/* Motion latch mode */
#define MOTION_MDL_LTMD		2	/* Motion latch mode(SVCTRL) */
#define MODAL_LTMD			3	/* Modal latch mode(Normal) */

/*--------------------------------------------------------------------------------------------------*/
/* defines : Latch mode(For internal processing) */
/*--------------------------------------------------------------------------------------------------*/
#define INTERPOL_LTMD		4	/* Motion latch latch mode(INTERPOLATE) */
#define EXPOSING_LTMD		5	/* Motion latch latch mode(EXPOSING) */
#define ZRET_LTMD			6	/* Motion latch latch mode(ZRET) */
//#define ZRET_LOCK_LTMD		7	/* Motion latch latch mode(ZRETMedium) */
#define NONE_LTMD			0	/* Without latch mode */

/*--------------------------------------------------------------------------------------------------*/
/* defines : Soft Limit                                                                             */
/*--------------------------------------------------------------------------------------------------*/
#define SOFT_LIMIT_P	0x1
#define SOFT_LIMIT_N	0x2

/*--------------------------------------------------------------------------------------------------*/
/* enums : Motion Types																				*/
/*--------------------------------------------------------------------------------------------------*/
enum MOTIONTYPE {
	MTYPE_NONE=0,	/* 0:Motion stopped */ 
	MTYPE_POSING,	/* 1:Positioning */ 
	MTYPE_EXPOSING,	/* 2:External positioning */ 
	MTYPE_HOMING,	/* 3:Return to origin */
	MTYPE_FEED,		/* 4:Constant speed feed */
	MTYPE_EXFEED,	/* 5:External signal constant speed feed */
	MTYPE_HOLD,		/* 6:Stop */
	MTYPE_INTERPOL,	/* 7:Interpolation feed */ 
	MTYPE_VELCTRL,	/* 8:Speed control */
	MTYPE_TRQCTRL,	/* 9:Torque control */
	MTYPE_FOLLOW,	/* A:Follow-up */ 
};

/*--------------------------------------------------------------------------------------------------*/
/* enums : Hold Mode																				*/
/*--------------------------------------------------------------------------------------------------*/
typedef enum _MT_HOLD_MODE {
	MT_HOLD_SLOWSTOP = 0,
	MT_HOLD_FASTSTOP = 1,
	MT_HOLD_EMGSTOP  = 2,
} MT_HOLD_MODE;

/*--------------------------------------------------------------------------------------------------*/
/* enums : Latch Signal																				*/
/*--------------------------------------------------------------------------------------------------*/
typedef enum _MT_LATCH_SGNL {
	MT_LATCH_CPHASE = 0,
	MT_LATCH_EXT1,
	MT_LATCH_EXT2,
	MT_LATCH_EXT3,
}  MT_LATCH_SGNL;


/****************************************************************************************************/
/*																									*/
/*  Motion Parameter Definitions 																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/* Motion operation functionSW: If disabled, comment											*/
/*--------------------------------------------------------------------------------------------------*/
//#define PRM_BANK_ENABLE		/* Bank switching enabledSW */
#define ACC_FIL_DISABLE	    /* Filter disabledSW */
#define POSING_1RATEONLY	/* POSING 1Stage invalidSW */

/*--------------------------------------------------------------------------------------------------*/
#define MAX_TRQ24			0x1000000	/* MAXTRQ */
#define MAX_SPD24			0x1000000	/* MAXSpeed */
#define DISTANCE_MAX		0x7FFFFFFF	/* Travel distanceMAX */
#define ACF_MAX_SZ			512			/* Number of moving average filter buffers */
#define FOLLOWUP_MAX_REF	32767		/* FollowUpMaximum travel amount at time[ref/cyc] */
#define MG_VREMBIT_MAX		24			/* MotionGeneratorRemaining speed calculationMAXNumber of bits */
#define MG_SPDDIV_RANGE		4			/* MotionGeneratorAutomatic setting bit range for decimal point */


/****************************************************************************************************/
/*																									*/
/*		Type Definitions																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*  Motion Command Structure(MtApi.c)																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MCI_CMD_ {
	LONG		TPos;		/* Target position[ref] */
	LONG		TSpeed;		/* Target speed[ref/s] */
	LONG		Velocity;	/* VFF/Speed command [ref/s],[2^30/MAX] */
	LONG		Trq;		/* Torque command[MAX_TRQ/2^30] */
	ULONG		TrqLimitP;	/* Positive torque limit[MAX_TRQ/2^30] */
	ULONG		TrqLimitN;	/* Negative side torque limit[MAX_TRQ/2^30] */
	LONG		AccRate;	/* Acceleration constant<Rev2.01> */
	LONG		DecRate;	/* Deceleration constant<Rev2.01> */
	ULONG		Option;		/* Option command */
	UCHAR		HoldMod;	/* Stopping method selection 0: Deceleration stop1,1:Rapid stop,2Deceleration stop2 */
	UCHAR		LatchSgnl;	/* Latch signal selection 0:CPhase,n:EXTn */
	BOOL		EnableRate;	/* Acceleration command enabled<Rev2.01> */
	BOOL		RstReq;		/* Motion reset request */
} MCI_CMD;

/*--------------------------------------------------------------------------------------------------*/
/*  Motion Command Structure(MtCtrl.c)																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MT_CMD_ {
	LONG	TPos;		    /* Target position[ref] */
	LONG	TSpeed;		    /* Target speed[ref/s] */
	LONG	Velocity;	    /* VFF[re/s], Speed command[2^30/MAX] */
	LONG	Trq;		    /* Torque command[MAX_TRQ/2^30] */
/*--------------------------------------------------------------------------------------------------*/
	UCHAR	Mtype;		    /* Motion type */
	UCHAR	bHold;	    	/* HOLDCommand */
	UCHAR	HoldMod;	    /* Stopping method selection 0: Deceleration stop1,1:Rapid stop,2Deceleration stop2 */
	UCHAR	LatchSgnl;	    /* Latch signal selection 0:CPhase,n:EXTn */
/*--------------------------------------------------------------------------------------------------*/
	UCHAR	AccFilSel;	    /* Filter selection 0,3: None, 1:Exponent,2:Moving average */
	UCHAR	CtrlMode;	    /* Control mode */
	UCHAR	CmdRstReq;	    /* Command reset request */
	UCHAR	MtEenable;	    /* Motion valid */
/*--------------------------------------------------------------------------------------------------*/
	UCHAR	ServoOn;	    /* ServoONCommand */
	UCHAR	SensOn;		    /* SensorONCommand */
	UCHAR	SvonLock;	    /* ServoON Lock */
	UCHAR	CmdRateOn;	    /* Acceleration/deceleration command enabledSW<Rev2.01> */
/*--------------------------------------------------------------------------------------------------*/
	LONG	Rdst;	        /* Registration distination offset [ref] */
	LONG	Rspd;		    /* Registration speed [ref/min] */
/*--------------------------------------------------------------------------------------------------*/
	LONG	TspdKx;			/* Speed Gain [spdref] -> [ref/cycle]									*/
//	LONG	Tlim;			/* Torque limit[%] */
} MT_CMD;

/*--------------------------------------------------------------------------------------------------*/
/*  Latch Command Structure																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MT_LATCH_ {
	UCHAR	Req;		/* Latch request */
	UCHAR	Cmp;		/* Latch completion */
	UCHAR	Mode;		/* Latch mode */
	UCHAR	Sgnl;		/* Selection signal */

	UCHAR	LtDisable;	/* Latch disabled */
	UCHAR	SeqReq;		/* Continuous latch request */
	UCHAR	SeqCnt;		/* Continuous latch sequence counter */
	union {				/* Continuous latch status */
		USHORT	wdCnt;
		UCHAR	cnt[2];	/* [0]:Continuous number count,[1]:Sequence count */
	}SeqSts;
} MT_LATCH;


/*--------------------------------------------------------------------------------------------------*/
/*  Motion Generator Parameter																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MG_PRM_ {
	LONG	accrate1;			/* Acceleration constant 1 ***.*[ref/cyc^2] */
	LONG	accrate2;			/* Acceleration constant 2 ***.*[ref/cyc^2] */
	LONG	altspd_a;			/* Acceleration constant switching speed ***.*[ref/cyc] */
	LONG	decrate1;			/* Deceleration constant 1 ***.*[ref/cyc^2] */
	LONG	decrate2;			/* Deceleration constant 2 ***.*[ref/cyc^2] */
	LONG	altspd_d;			/* Acceleration constant switching speed ***.*[ref/cyc] */
} MG_PRM;

/*--------------------------------------------------------------------------------------------------*/
/*  Motion Control Base Parameter																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MC_BASE_PRM_ {
	LONG	MotionCycle;		/* Motion control cycle[0.25us] */
	BOOL	FullcCtrl;			/* FullcUsing */
	BOOL	Linear;				/* Linear use */
	BOOL	ot_zstop;			/* OTTime zero clamp */
	UCHAR	trefMode;			/* Torque command use selection during speed control */
	UCHAR	TrefModePos;		/* Torque command use selection during position control(INTERPOLATESystem) */
	UCHAR	TLimModePos;		/* Torque limit use selection during position control(POSINGSystem) */

	LONG	PosingRateKx;		/* [10000ref/s^s]->[***.*ref/cyc^cyc]Acceleration/deceleration conversion normalization constant */
	LONG	PosingRateKxIx;		/* TODO:[(1000ref/min)/msec]->[***.*ref/cyc^cyc]Acceleration/deceleration conversion normalization constant */
	LONG	PosingSpdKx;		/* [ref/s]->[***.*ref/cyc]Speed conversion normalization constant */
	LONG	PosingSpdKxIx;		/* TODO:[1000ref/min]->[***.*ref/cyc]Speed conversion normalization constant */
	LONG	PosingSpdInvKx;		/* [***.*ref/cyc]->[ref/s]Speed conversion normalization constant */
	LONG	RefToRpmKxIx;		/* TODO:[1000ref/min]->[rpm]Speed conversion normalization constant */
	LONG	fspdRefKx;			/* [ref/Communication cycle]->[ref/s]Speed conversion normalization constant */
	LONG	refToOsKx;			/* [Command unit/s]->[24bit/OS]Speed conversion normalization constant */
	LONG	osToRefKx;			/* [24bit/OS]->[Command unit/s]Speed conversion normalization constant */
	LONG	maxToOsKx;			/* [30bit/MAX]->[24bit/OS]Speed conversion normalization constant */
	LONG	osToMaxKx;			/* [24bit/OS]->[30bit/MAX]Speed conversion normalization constant */
	LONG	trqKx;				/* [24bit/MAX]->[%]Normalization constant */
	LONG	invTrqKx;			/* [%]->[24bit/MAX]Normalization constant */
	LONG	taccRateKx;			/* [(ref/s)/(0.1ms)]->***.*[ref/cyc^2]<Rev2.01> */
	LONG	taccRate7Kx;		/* [2^7*(ref/s)/(0.1ms)]->***.*[ref/cyc^2]<Rev2.01> */
	VOID	(*SetCmdAccRate)();/* Acceleration command calculation function pointer<Rev2.01> */

	UCHAR	vel_sft_exp;		/* Speed resolution decimal point bitsACCFILFor [bit] */
	UCHAR	vel_shift;			/* Speed resolution decimal point bitsMGFor [bit] */
	UCHAR	vel_shift_s;		/* Speed resolution decimal point bits [bit] */
	LONG	vel_max;			/* Maximum speed(=2^(31-vel_shift))[ref/cyc] */
	LONG	seg_num;			/* Number of segment divisions */
	ULONG	seg_max_ref;		/* Maximum segment input */
	ULONG	PosingSpdMax;		/* POSINGMaximum speed */
	ULONG	mgSpdMax;			/* MGInsidePOSINGMaximum speed */
	MG_PRM	StopPrm;			/* Emergency stop deceleration parameter */
} MC_BASE_PRM;

/*--------------------------------------------------------------------------------------------------*/
/*  Position Parameter																				*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct PSN_PRM_ {
	BOOL	sotEnableP;			/* Forward rotation side soft limit enabled */
	BOOL	sotEnableN;			/* Reverse side soft limit enabled */
	BOOL	tposLimitEnable;	/* Command soft limit enabled */
	BOOL	absPG;				/* AbsolutePGUsing */
	LONG	absOfsset;			/* Absolute valuePGOffset value(Pn803) */
	LONG	softLimit_P;		/* Forward rotation side soft limit value */
	LONG	softLimit_N;		/* Reverse side soft limit value */
	LONG	ltRangeP;			/* Pn820:Positive side latch zone */
	LONG	ltRangeN;			/* Pn822:Negative side latch zone */
	ULONG	zPointWidth;		/* Origin position range */
	ULONG	psetWidth;			/* Nearrange */
	ULONG	nearWidth;			/* Psetrange */
	SHORT	TdlyIn;				/* MPOSCorrection time */
	USHORT	SegNum;				/* Number of divisions(Motion cycle/Servo period) */
} PSN_PRM;

/*--------------------------------------------------------------------------------------------------*/
/*  Motion Sequence Parameter																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MT_SEQ_PRM_ {
	LONG	zretsp1;			/* ZRET Approach speed1 [ref/sec -> ref/cycle] */
	LONG	zretsp2;			/* ZRET Approach speed2 [ref/sec -> ref/cycle] */
	LONG	zret_dist;			/* ZRETFinal mileage [ref] */
	LONG	ex_posing_dist;		/* EX_POSINGFinal mileage */
	ULONG	rampTimeOut;		/* Deceleration and stop timeout time[Motion cycle] */
	ULONG	tlimTimeCnt1;		/* Torque limit detection time for return to push origin1 */
	ULONG	tlimTimeCnt2;		/* Torque limit detection time for return to push origin2 */
	LONG	zret_dir;			/* Home return direction */
	LONG	zretOSLevel;		/* zret overspeed alarm level [2^24/MaxSpd] */
} MT_SEQ_PRM;

/*--------------------------------------------------------------------------------------------------*/
/*  Acc filter Parameter																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	union ACC_MATB_PRM_ {
	ULONG	lTb;		/* Number of moving averages(Total) */
	USHORT	wTb[2];		/* high word:Number of moving averages, low word:Number of moving average interpolation  */
} MATB_PRM;

/*--------------------------------------------------------------------------------------------------*/
/*  Acc filter Parameter																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct ACCFIL_PRM_ {
	LONG	expbias;			/* Exponential acceleration/deceleration bias */
	LONG	exptb;				/* Exponential acceleration/deceleration time constant */
	MATB_PRM mafil;				/* Number of moving averages */
} ACF_PRM;

/*--------------------------------------------------------------------------------------------------*/
/*  Option Map Parameter																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct OPTION_MAP_ {
	UCHAR	accfilBit;			/* Acceleration/deceleration filter selection bit position */
	UCHAR	accfilBitMsk;		/* Acceleration/deceleration filter selection bit mask */
	UCHAR	gselBit;			/* Gain selection bit position */
	UCHAR	gselBitMsk;			/* Gain selection bit mask */
	UCHAR	vppiBit;			/* P_PISelected bit position */
	UCHAR	vppiBitMsk;			/* P_PISelected bit mask */
	UCHAR	ppiclrBit;			/* Positional integral clear bit position */
	UCHAR	ppiclrBitMsk;		/* Positional integral bit mask */
	UCHAR	pclBit;				/* P_CLBit position */
	UCHAR	pclBitMsk;			/* P_CLBit mask */
	UCHAR	nclBit;				/* N_CLBit position */
	UCHAR	nclBitMsk;			/* N_CLBit mask */
	UCHAR	outpBit;			/* OutSignalBit position */
	UCHAR	outpBitMsk;			/* OutSignalBit mask */
	UCHAR	bankBit;			/* BankSelBit position */
	UCHAR	bankBitMsk;			/* BankSelBit mask */
	UCHAR	ltBit;				/* LtDisableBit position */
	UCHAR	ltBitMsk;			/* LtDisableBit mask */
} OPTION_MAP;

/*--------------------------------------------------------------------------------------------------*/
/*  Input Signal Parameter																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct INSIG_MAP_ {
	UCHAR	bitNo[8];
	USHORT	bitsMsk;
} INSIG_MAP;

/*--------------------------------------------------------------------------------------------------*/
/*  Sequencial Latch Parameter																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct LTSEQ_PRM_ {
	USHORT	seqNum;				/* Number of sequence signals */
	USHORT	loopCnt;			/* Continuous number of times setting */
	UCHAR	ltSgnl[8];			/* Latch signal */
} LTSEQ_PRM;

/*--------------------------------------------------------------------------------------------------*/
/*  Motion generator inout																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MG_INOUT_ {
	UCHAR	Mtype;			/* Motion Type */
	UCHAR	LastMtype;		/* Last Motion Type */
	USHORT	Ctrl;			/* MotionControlOption */
/*--------------------------------------------------------------------------------------------------*/
	LONG	DposIn;			/* Command movement amount[ref] */
	//LONG	Tspd;			/* POSING[ref/s] */
	LONG	Tspd;			/* POSING[ref/cycle] (Interim) */
	LONG	Vref;			/* Speed command[30bit/MAX] */
	LONG	Tref;			/* Torque command[30bit/MAX] */
/*--------------------------------------------------------------------------------------------------*/
	LONG	DposOut;		/* Output movement amount[ref](Vel->Pos) */
	LONG	VrefOut;		/* Output speed command[24bit/OS] */
	LONG	TrefOut;		/* Output torque command[24bit/MAX] */
} MG_INOUT;

/*--------------------------------------------------------------------------------------------------*/
/*  Motion generator handle																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MGHNDL_ {
	BOOL			cMinusDir;	/* Current dispensing direction */
	BOOL			tMinusDir;	/* Direction from the current command position to the target position */
	ULONG			dpos0;		/* DPOS for POSING */
	LONG			dposRem;	/* Transmission cycle movement amount(dpos)Remainder         */
	LONG			Cspd;		/* Current command speed */
	MC_BASE_PRM*	basePrm;	/* MGBasic parameters */
	MG_PRM*			prm;		/* MotionParameter */
} MGHNDL;

/*--------------------------------------------------------------------------------------------------*/
/*  Motion Parameter																				*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MTPRM_ {
	MC_BASE_PRM BasePrm;		/* Basic parameters */
	MG_PRM		MgPrm;			/* MGMotion parameter */
	MG_PRM		CmdRate;		/* Acceleration/deceleration command<Rev2.01> */
	PSN_PRM		PsnPrm;			/* PositionManagerParameter */
	MT_SEQ_PRM	MtSeqPrm;		/* MotionCommand creation parameters */
	//ACF_PRM		AccFilPrm;		/* Position command filter parameter */
	//OPTION_MAP	OptMap;			/* OPTIONCommand bit position setting parameter */
	//INSIG_MAP	InSigMap;		/* Input Signal0-7Assignment */
#ifdef PRM_BANK_ENABLE
	//MTBANK		Bnk;			/* Motion PRM BankData */
#endif
	//LTSEQ_PRM	LtPrm;
} MTPRM;

/*--------------------------------------------------------------------------------------------------*/
/*  Motion Initial Parameter																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MTPRM_INI_ {
	LONG	EGearA;			/* Electronic gearA(Denominator) */
	LONG	EGearB;			/* Electronic gearB(Molecular) */
	LONG	PgPuls;			/* PGPulse number[puls/rev],[puls/pitch] */
	LONG	MaxTrq;			/* MAXTorque[%] */
	LONG	OverSpd;		/* OSSpeed[rpm],[mm/s] */
	LONG	MaxSpd;			/* MAXSpeed[rpm],[mm/s] */
	LONG	ScalePitch;		/* Scale pitch[mm/pitch] */
	SHORT	OverSpdExp;		/* OSSpeed index */
	SHORT	MaxSpdExp;		/* MAXSpeed index */
	SHORT	ScalePitchExp;	/* Scale pitch index */
	USHORT	trefMode;		/* Torque controlOption */
	BOOL	ot_zstop;		/* OTTime zero clamp */
	BOOL	absPG;			/* AbsolutePGUsing */
	BOOL	FullcCtrl;		/* Full-closed control */
	BOOL	Linear;			/* Linear use */

	USHORT	Cycle;			/* Motion control cycle[0.25us] */
	USHORT	SpdDiv;			/* Resolution */
	ULONG	SegNum;			/* Number of interpolation feed segments */
	USHORT	OptEnable;		/* OptionAllocation enabled */
	USHORT	OptMap[5];		/* OptionAllocation definition */
	USHORT	OutSigEnable;	/* OptionAllocation output signal enabled/Disabled */
	SHORT	dlyTime;		/* From command outputFBTime until input */
	USHORT	SvCycle;		/* Servo period(Motion cycle/Servo period) */
/*--------------------------------------------------------------------------------------------------*/
	LONG	TspdGain;		/* Target speed gain */
/*--------------------------------------------------------------------------------------------------*/
} MTPRM_INI;

/*--------------------------------------------------------------------------------------------------*/
/*  Position Handle Structure																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct PSNHNDL_ {
	LONG		Tpos;		/* Target position */
	LONG		Ipos;		/* Command position (before filter) */
	LONG		Pos;		/* Command position (after filter) */
	LONG		Mpos;		/* FBCommand position at position input timing */
	LONG		Apos;		/* FBPosition */
	LONG		Perr;		/* Deviation */
	LONG		TlimPerr; /* Position deviation at torque limit */
	LONG		Lpos;		/* Latch position */
	LONG		LstLpos;	/* Previous latch position */
	LONG		Fbspd;		/* FBSpeed */
//	LONG		FbSpdx;		/* FBSpeed[2msAverage] */
//	LONG		FbSpdSum;	/* FBSpeed[2msAverage]ForSumvalue */
	LONG		Pgcnt;		/* Target position */
	LONG		Dpos;		/* Output travel distance of this time */
	LONG		lstDpos;	/* Previous output travel distance */
	BOOL		Den;		/* Command output complete */
	BOOL		RefEnable;	/* Reference point enabled */
	BOOL		PosEnable;	/* Coordinate position valid */
	UCHAR		SoftLimit;	/* Soft limit status */
//	UCHAR		FbSpdSumCnt;/* FBSpeed[2msAverage]Counter */	
	PSN_PRM*	Prm;		/* PotionParameter */
} PSNHNDL;

/*--------------------------------------------------------------------------------------------------*/
/*  Motion Handle Structure																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct MTHNDL_ {
	UCHAR		seqState;	/* Command generation sequence status */
	UCHAR		otState;	/* OTStatus */
//#if defined ( CCSW_SFTID ) && ( CCSW_SFTID == SFTID_EXIST )								/*<V717>Add:Start*/
//	UCHAR		amState;	/* AMStatus */
//#endif															/*<V717>Add:End  */
	UCHAR		mtState;	/* Motion status */
/*--------------------------------------------------------------------------------------------------*/
	UCHAR		crtMtype;	/* Current motion type */
	UCHAR		ctrlMode;	/* Current control mode */
	UCHAR		tlimSig[2]; /* Torque limit filter */
/*--------------------------------------------------------------------------------------------------*/
	LONG		ZretFspdSum;     /* feedback speed sum (for homing)                                 */
	LONG		ZretFspdSumCnt;  /* feedback speed sum count (for homing)                           */
/*--------------------------------------------------------------------------------------------------*/
	ULONG		timer;		/* RampStoptimer for */
	LONG		segCnt;		/* Motion payout counter */
	ULONG		tlimCnt;	/* Torque limit timer counter */
/*--------------------------------------------------------------------------------------------------*/
	MT_CMD		cmdData;	/* Motion command data */
	MG_INOUT	outData;	/* Motion output data */
	MT_LATCH	latch;		/* Latch */
	MGHNDL		mtgHdl;		/* MotionGeneratorHandle */
	PSNHNDL		psnHdl;		/* PositionHandle */
	//ACFHNDL		acfHdl;		/* AccFilterHandle */
	MTPRM		mtPrm;		/* MotionPRMHandle */
/*--------------------------------------------------------------------------------------------------*/
	struct {
		MTPRM_INI	mt;		/* Motion initial parameter data */
	} iniPrm;				/* Initial parameter load data */
/*--------------------------------------------------------------------------------------------------*/
	CYC_CMD*	svCmd;		/* Servo If Cyclic Command Data*/
	CYC_RSP*	svRsp;		/* Servo If Cyclic Response Data*/
} MTHNDL;

/****************************************************************************************************/
/*																									*/
/*		Grobal Function Prototypes																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*  MtCtrl.c: Motion Control																		*/
/*--------------------------------------------------------------------------------------------------*/
extern VOID MtcInit(MTHNDL* hdl, SV_OPTION_IF* svData);				/* initialize motion */
extern VOID MtcOutputData(MTHNDL* hdl, SV_OPTION_IF *svData);		/* output motion command */
extern VOID MtcInputData(MTHNDL* hdl, SV_OPTION_IF *svData);		/* input motion data */
extern VOID MtcCreateInCmd(MTHNDL* hdl);							/* create motion in command */
void	MtcSetHomeMethod(long prm);							        /* set homing method */
/*--------------------------------------------------------------------------------------------------*/
/*  MtGenerator.c: Motion Generator																	*/
/*--------------------------------------------------------------------------------------------------*/
extern BOOL MtgGenerate(MGHNDL* hdl, MG_INOUT* inOut);				/* generate motion */
/*--------------------------------------------------------------------------------------------------*/
/*  MtPRM.c: Motion Parameter																		*/
/*--------------------------------------------------------------------------------------------------*/
LONG MPrmCalcInit(MTPRM* prm, MTPRM_INI* iniPrm);					/* calculate motion parameters  */
LONG MPrmSetZretDir(MTPRM* prm, BOOL data);							/* set homing dir               */
LONG MPrmSetZretSpd1(MTPRM* prm, LONG data);						/* set homing approach spd      */
LONG MPrmSetZretSpd2(MTPRM* prm, LONG data);						/* set homing creap spd         */
LONG MPrmSetZretDist(MTPRM* prm, LONG data);						/* set homing distination       */
LONG MPrmSetZretOSLvl(MTPRM* prm, LONG data);						/* set homing overspeed level   */
LONG MPrmSetSoftLimitFlag(MTPRM* prm, USHORT data);					/* sot enable or disable 		*/
LONG MPrmSetSoftLimitP(MTPRM* prm, LONG data);						/* set positive sot 			*/
LONG MPrmSetSoftLimitN(MTPRM* prm, LONG data);						/* set negative sot 			*/
LONG MPrmSetTlimTimeOut1(MTPRM* prm, LONG data);					/* set homing timeout1          */
LONG MPrmSetTlimTimeOut2(MTPRM* prm, LONG data);					/* set homing timeout2          */
/*--------------------------------------------------------------------------------------------------*/
/*  Position.c: Position Manager																	*/
/*--------------------------------------------------------------------------------------------------*/
extern VOID PsnInit(PSNHNDL* hdl, PSN_PRM* prm);
extern VOID PsnUpdateFBData(PSNHNDL* hdl, LONG pgcnt);
extern BOOL PsnGetPsetSts(PSNHNDL* hdl);
extern BOOL PsnGetNearSts(PSNHNDL* hdl);
extern BOOL PsnGetZPointSts(PSNHNDL* hdl);
extern LONG PsnSetTPOS(PSNHNDL* hdl,LONG tpos);
extern LONG PsnSetTPOSDist(PSNHNDL* hdl,LONG dpos);
extern VOID PsnResetCmdPosAll(PSNHNDL* hdl, LONG err);
extern UCHAR PsnCheckSoftLimit(PSNHNDL* hdl, LONG* dpos);
extern UCHAR PsnCheckSoftLimitFB(PSNHNDL* hdl, BOOL* p_ot, BOOL* n_ot);
extern VOID PsnSetCordinate(PSNHNDL* hdl, UCHAR mode, LONG pos);
extern VOID PsnResetApos(PSNHNDL* hdl, BOOL abs);
extern BOOL PsnMakeLpos(PSNHNDL* hdl, LONG ltcnt);
#define	PsnSetRefPoint(hdl, enable)	((hdl)->RefEnable = enable)
#define	PsnResetIPOS(hdl)			((hdl)->Ipos = (hdl)->Pos)
#define	PsnSetTPOSNoLimit(hdl,dpos)	((hdl)->Tpos = dpos + (hdl)->Ipos)
#define	PsnSetPOS(hdl, dpos)		{(hdl)->Pos += dpos;\
									 (hdl)->lstDpos = (hdl)->Dpos;\
									 (hdl)->Dpos = dpos;}
#define	PsnSetIPOS(hdl, dpos)		((hdl)->Ipos += dpos)
#define	PsnSetDenSts(hdl, den)		((hdl)->Den = den)
#define	PsnGetDenSts(hdl)			((hdl)->Den)
#define	PsnGetTPOS(hdl)				((hdl)->Tpos)
#define	PsnGetIPOS(hdl)				((hdl)->Ipos)
#define	PsnGetPOS(hdl)				((hdl)->Pos)
#define	PsnGetMPOS(hdl)				((hdl)->Mpos)
#define	PsnGetLPOS(hdl)				((hdl)->Lpos)
#define	PsnGetLstLPOS(hdl)			((hdl)->LstLpos)
#define	PsnGetAPOS(hdl)				((hdl)->Apos)
#define	PsnGetPERR(hdl)				((hdl)->Perr)
#define	PsnGetSOTSts(hdl)			((hdl)->SoftLimit)
#define	PsnGetFBSPD(hdl)			((hdl)->Fbspd)
#define	PsnAttachPRM(hdl, prm)		((hdl)->Prm = prm)
/*--------------------------------------------------------------------------------------------------*/
/*  MtApi.c: Motion API																				*/
/*--------------------------------------------------------------------------------------------------*/
extern INT32 MciSetInterpolateCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern INT32 MciSetLatchCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern INT32 MciSetPosingCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern INT32 MciSetExPosingCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern INT32 MciSetHomingCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern INT32 MciSetFeedCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern INT32 MciSetHoldCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern INT32 MciSetVelCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern INT32 MciSetTrqCmd(MTHNDL* hdl, MCI_CMD* cmd);
extern VOID MciSetOptionCmd(MTHNDL* hdl, ULONG option);

extern INT32 MciEnableState(MTHNDL* hdl, BOOL enable);
extern INT32 MciResetMotion(MTHNDL* hdl);
extern INT32 MciSetServoOn(MTHNDL* hdl, BOOL enable, BOOL reset);
extern INT32 MciSetSensOn(MTHNDL* hdl, BOOL enable);

extern INT32 MciRequestMtLatch(MTHNDL* hdl, UCHAR mode, UCHAR signl);
extern INT32 MciRequestMdlLatch(MTHNDL* hdl, UCHAR mode, UCHAR signl);
extern INT32 MciCancelLatch(MTHNDL* hdl, UCHAR mode);
extern INT32 MciSetCordinatePos(MTHNDL* hdl, LONG data, BOOL ref);

extern VOID MciOutputSignals(MTHNDL* hdl, USHORT signl);
extern VOID MciOutputBkSignal(MTHNDL* hdl, BOOL enable);
/*--------------------------------------------------------------------------------------------------*/
extern UCHAR MciGetMType(MTHNDL* hdl);
extern BOOL MciGetServoOnStatus(MTHNDL* hdl);
extern BOOL MciGetSensOnStatus(MTHNDL* hdl);
extern UCHAR MciGetCtrlMode(MTHNDL* hdl);
extern LONG MciGetMonitorData(MTHNDL* hdl, USHORT sel, UCHAR unit);
extern BOOL MciGetDenStatus(MTHNDL* hdl);
extern BOOL MciGetLatchStatus(MTHNDL* hdl ,UCHAR *ltEdge);
extern USHORT MciGetLatchSeqStatus(MTHNDL* hdl);

extern BOOL MciGetInPosStatus(MTHNDL* hdl);
extern BOOL MciGetNearStatus(MTHNDL* hdl);
extern BOOL MciGetZpointStatus(MTHNDL* hdl);
extern UCHAR MciGetSoftLimitStatus(MTHNDL* hdl);
extern LONG MciGetOutputCmdDist(MTHNDL* hdl);

//extern BOOL MciGetZspdStatus(MTHNDL* hdl);
//extern BOOL MciGetMainPowerStatus(MTHNDL* hdl);
extern BOOL MciGetVlimitStatus(MTHNDL* hdl);
extern BOOL MciGetTrqLimitStatus(MTHNDL* hdl);
extern BOOL MciGetVcmpStatus(MTHNDL* hdl);

extern USHORT MciGetInputSignals(MTHNDL* hdl);
extern USHORT MciGetInPortSignals(MTHNDL* hdl);

#if defined( CCSW_SFTID  ) && ( CCSW_SFTID == SFTID_EXIST )						/*<V717>Add:Start*/
extern USHORT MciGetSomStatus(void);
#endif													/*<V717>Add:End  */

/*--------------------------------------------------------------------------------------------------*/
//#if defined(__ghs__)
//#pragma ghs startzda
//#elif !defined(WIN32)
//#pragma section sidata begin
//#endif
extern MTHNDL	MtHdl;							/* motion handle */
//extern void	(*pIxxAxctrlReqHoming)( MT_CMD *mtcmd, long tspeed,
//									 MT_LATCH_SGNL sgnl );    /* Pointer Homing request    			*/
//#if defined(__ghs__)
//#pragma ghs endzda
//#elif !defined(WIN32)
//#pragma section sidata end
//#endif

#endif /* _MOTION_H_ */

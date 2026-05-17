/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ServoIf.h : ServoControl unitI/FProcess definition															*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*						2013.02.23	K.Ozaki	IgnoreModeSWAddition, source code being organized 					*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#ifndef __SERVO_IF_H__	
#define	__SERVO_IF_H__

/****************************************************************************************************/
/*																									*/
/*	Cyclic Interface Definition 																	*/
/*																									*/
/****************************************************************************************************/
typedef struct	CMD_CTRLBITS_ {
	UCHAR	ModSel;						/* Control Mode Selection	*/
	UCHAR	SelBank;					/* Gain Selection			*/
	USHORT	CmdCtrlBit;					/* Command Control Bits		*/
	BOOL	vffFilSts;					/* VFF Moving Average Filer Status *//* <S1D0> */
	BOOL	tffFilSts;					/* TFF Moving Average Filer Status  *//* <S1D0> */
} CMD_CTRLBITS;

/*--------------------------------------------------------------------------------------------------*/
/* 	Defines of Control Mode 																		*/
/*--------------------------------------------------------------------------------------------------*/
#define POS_CTRL_MODE	(1)								/* Position Control */
#define SPD_CTRL_MODE	(2)								/* Velocity Control */
#define TRQ_CTRL_MODE	(3)								/* Torque Control */
#define NO_CTRL_MODE	(0)								/* No Control */

/*--------------------------------------------------------------------------------------------------*/
/* CMD_CTRLBITS 																					*/
/*--------------------------------------------------------------------------------------------------*/
enum _enumCmdCtrlBits {
	ENBPCTRL_BITNO = 2,									/* P/PISwitch			*/
	CLRPOSERR_BITNO,									/* Position deviation clear 		*/
	CLRPOSINTG_BITNO,									/* Positional integral clear		*/
	IGNOREMODESW_BITNO,									/* ModeSWDisabled			*/
	TRACE_TRIGGER_BITNO = 15,							/* Network trigger request			*//* <S012> */
};

#define	ENBPCTRL_BIT		(1<<ENBPCTRL_BITNO)			/* P/PISwitch			*/
#define CLRPOSERR_BIT		(1<<CLRPOSERR_BITNO)		/* Position deviation clear		*/
#define	CLRPOSINTG_BIT		(1<<CLRPOSINTG_BITNO)		/* Positional integral clear		*/
#define IGNOREMODESW_BIT	(1<<IGNOREMODESW_BITNO)		/* ModeSWDisabled			*/
#define TRACE_TRG_BIT		(1<<TRACE_TRIGGER_BITNO)	/* Network trigger request			*//* <S012> */
/* <S1D0> Start */
/* <VFF_TFF_MAFIL> */
//#define	TFFFILINV_BIT		5							/* Torque feedforward moving average filter function enable/disable*/
//#define	VFFFILINV_BIT		6							/* Speed feedforward moving average filter function enable/disable*/
/* <VFF_TFF_MAFIL> */
//#define	TFFFILINV_BITMSK	0x01
//#define	VFFFILINV_BITMSK	0x01
/* <S1D0> End */
#if 0
typedef struct	CMD_CTRLBITS_ {
	UINT	ModSel			:2;	/* Control mode selection */
	UINT	EnbPCtrl		:1;	/* P/PISwitch */
	UINT	ClrPosErr		:1;	/* Position deviation clear */
	UINT	ClrPosIntg		:1;	/* Positional integral clear */
	UINT	ClrVPosErr		:1;	/* Virtual position deviation clear request *//* <0FJ29> */
	UINT	Reserved67		:2;	/* Reservation *//* <0FJ29> */

	UINT	SelBank			:4;	/* Gain selection */
	UINT	Reserved1215	:4;	/* Reservation	*/
} CMD_CTRLBITS;
#endif

/*--------------------------------------------------------------------------------------------------*/
/* CMD_LTBITS 																						*/
/*--------------------------------------------------------------------------------------------------*/
#define	REQ_C_PHASE_BIT		0x01		/* CPhase latch request		*/
#define	REQ_EXT_SIG1_BIT	0x02		/* Ext1Latch request		*/
#define	REQ_EXT_SIG2_BIT	0x04		/* Ext2Latch request		*/
#define	REQ_EXT_SIG3_BIT	0x08		/* Ext3Latch request		*/
#define	SELENC_C_PHS_BIT	0x10		/* CEncoder selection		*/
#define	SELENC_EXT1_BIT		0x20		/* Ext1Encoder selection	*/
#define	SELENC_EXT2_BIT		0x40		/* Ext2Encoder selection	*/
#define	SELENC_EXT3_BIT		0x80		/* Ext3Encoder selection	*/

/*--------------------------------------------------------------------------------------------------*/
/* 			CMD_SEQBITS 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define	SERVO_ON_BITNO			0		/* Servo on			*/
#define	SENSOR_ON_BITNO			1		/* Sensor on			*/
#define	ALM_RESET_BITNO			2		/* Alarm reset		*/
#define	WRN_RESET_BITNO			3		/* Warning reset	*/
#define	POL_DETECT_BITNO		4		/* Magnetic pole detection				*/
#define	BRK_RELEASE_BITNO		5		/* Brake release		*/
#define	ENB_EXTPTRQLMT_BITNO	6		/* Positive torque limit enabled	*/
#define	ENB_EXTNTRQLMT_BITNO	7		/* Negative side torque limit enabled	*/
#define	POSITIVE_OT_BITNO		8		/* Positive side softwareOT			*/
#define	NEGATIVE_OT_BITNO		9		/* Negative side softOT			*/
#define	ZCLAMP_BITNO			10		/* @@ Zero Clamp		*/
/* <S03D> Start */
#define	SPDD_BITNO				12		/* SPD-D */
#define	SPDA_BITNO				13		/* SPD-A */
#define	SPDB_BITNO				14		/* SPD-B */
#define	CSEL_BITNO				15		/* C-SEL */
/* <S03D> End */



#define	SERVO_ON_BIT		0x01		/* Servo on			*/
#define	SENSOR_ON_BIT		0x02		/* Sensor on			*/
#define	ALM_RESET_BIT		0x04		/* Alarm reset		*/
#define	WRN_RESET_BIT		0x08		/* Warning reset	*/
#define	POL_DETECT_BIT		0x10		/* Magnetic pole detection				*/
#define	BRK_RELEASE_BIT		0x20		/* Brake release		*/
#define	ENB_EXTPTRQLMT_BIT	0x40		/* Positive torque limit enabled	*/
#define	ENB_EXTNTRQLMT_BIT	0x80		/* Negative side torque limit enabled	*/
#define	POSITIVE_OT_BIT		0x100		/* Positive side softwareOT			*/
#define	NEGATIVE_OT_BIT		0x200		/* Negative side softOT			*/
#define	ZEROCLAMP_BIT		0x400		/* Zero Clamp			*/
/* <S03D> Start */
#define	SPDD_BIT			0x1000		/* SPD-D */
#define	SPDA_BIT			0x2000		/* SPD-A */
#define	SPDB_BIT			0x4000		/* SPD-B */
#define	CSEL_BIT			0x8000		/* C-SEL */
/* <S03D> End */


/*--------------------------------------------------------------------------------------------------*/
/*	RSP_LTBITS 																						*/
/*--------------------------------------------------------------------------------------------------*/
#define	C_PHASE_REQLVL_BIT	0x01		/* CPhase latch request level	*/
#define	EXT_SIG1_REQLVL_BIT	0x02		/* Ext1Latch request level	*/
#define	EXT_SIG2_REQLVL_BIT	0x04		/* Ext2Latch request level	*/
#define	EXT_SIG3_REQLVL_BIT	0x08		/* Ext3Latch request level	*/
#define	C_PHASE_COMP_BIT	0x10		/* CPhase latch complete		*/
#define	EXT_SIG1_COMP_BIT	0x20		/* Ext1Latch completion		*/
#define	EXT_SIG2_COMP_BIT	0x40		/* Ext2Latch completion		*/
#define	EXT_SIG3_COMP_BIT	0x80		/* Ext3Latch completion		*/

/*--------------------------------------------------------------------------------------------------*/
/*	RSP_CTRLBITS 																					*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct	RSP_CTRLBITS_ {
	UCHAR	ExeMode;					/* Control mode selection status */
	UCHAR	dummy;						/* reserved */
	USHORT	RspCtrlBit;					/* Response control bit management */
} RSP_CTRLBITS;


enum RSP_CTRL_BITNO {
	COIN_BITNO			= 2,			/* Positioning complete			*/
	MOTOR_MOVING_BITNO,					/* Motor rotation			*/
	REACH_SPD_BITNO,					/* Speed match				*/
	SPD_CLAMP_BITNO,					/* Speed limit			*/
	TRQ_CLAMP_BITNO,					/* Torque is being limited			*/
	OP_ENABLED_BITNO,					/* Command ready state		*/
	SAFETY_STOP_BITNO,					/* Safety Stop State	*/
	PERCLREDGE_BITNO,					/* @@ Perr Clear Edge 	*/
};

#define	COIN_BITMSK				0x04	/* Positioning complete		*/
#define	MOTOR_MOVING_BITMSK		0x08	/* Motor rotation		*/
#define	REACH_SPD_BITMSK		0x10	/* Speed match			*/
#define	SPD_CLAMP_BITMSK		0x20	/* Speed limit		*/
#define	TRQ_CLAMP_BITMSK		0x40	/* Torque is being limited		*/
#define	OP_ENABLED_BITMSK		0x80	/* Command ready state	*/
#define SAFETY_STOP_BITMSK		0x100	/* Safety Stop State*/
#define PERCLREDGE_BITMSK		0x200	/* @@ Perr Clear Edge */


/*--------------------------------------------------------------------------------------------------*/
/*	RSP_SEQBITS 																					*/
/*--------------------------------------------------------------------------------------------------*/
enum RSP_SEQ_BITNO {
	ALM_DETECTED_BITNO = 0,				/* 0:Servo alarm occurrence status		*/
	WRN_DETECTED_BITNO,					/* 1:Servo warning occurrence status	*/
	ALM_RESET_COMP_BITNO,				/* 2:Alarm reset complete		*/
	WRN_RESET_COMP_BITNO,				/* 3:Warning clear completed		*/
	/*----------------------------------------------------------------------------------------------*/
	SERVO_ON_COMP_BITNO,				/* 4:Servo-on completion				*/
	SENS_ON_COMP_BITNO,					/* 5:Sensor on complete				*/
	POL_DET_COMP_BITNO,					/* 6:Magnetic pole detection complete				*/
	BRK_RELEASING_BITNO,				/* 7:Brake released				*/
	/*----------------------------------------------------------------------------------------------*/
	MAIN_POWER_ON_BITNO,				/* 8:Main circuit power supplyON				*/
	SERVO_READY_BITNO,					/* 9:Servo ready				*/
	MOTOR_READY_BITNO,					/* A:Servo ready without phase information	*/
};

#define	ALM_DETECTED_BITMSK		0x01	/* Servo alarm occurrence status	*/
#define	WRN_DETECTED_BITMSK		0x02	/* Servo warning occurrence status	*/
#define	ALM_RESET_COMP_BITMSK	0x04	/* Alarm reset complete		*/
#define	WRN_RESET_COMP_BITMSK	0x08	/* Warning clear completed		*/
#define	SERVO_ON_COMP_BITMSK	0x10	/* Servo-on completion			*/
#define	SENS_ON_COMP_BITMSK		0x20	/* Sensor on complete			*/
#define	POL_DET_COMP_BITMSK		0x40	/* Magnetic pole detection complete				*/
#define	BRK_RELEASING_BITMSK	0x80	/* Brake released			*/
#define	MAIN_POWER_ON_BITMSK	0x100	/* Main circuit power supplyON				*/
#define	SERVO_READY_BITMSK		0x200	/* Servo ready				*/
#define	MOTOR_READY_BITMSK		0x400	/* Servo ready without phase information	*/


/* RSP_SIGNLBITS */
enum RSP_SIG_BITNO {
	N_OT_SIG_BITNO= 0,					/*  0:N-OTSignal input	*/
	P_OT_SIG_BITNO,						/*  1:P-OTSignal input	*/
	DEC_SIG_BITNO,						/*  2:DECSignal input	*/
	EXT1_SIG_BITNO,						/*  3:EXT1Signal input	*/
	EXT2_SIG_BITNO,						/*  4:EXT2Signal input	*/
	EXT3_SIG_BITNO,						/*  5:EXT3Signal input	*/
	HWBB_SIG_BITNO,						/*  6:HWBBSignal input	*/
	ESTP_SIG_BITNO,						/*  7:ESTPSignal input	*/
	
	HWBB1_SIG_BITNO=24,					/* 24:HWBB1Signal input*/
	HWBB2_SIG_BITNO,					/* 25:HWBB2Signal input*/
};

#define	N_OT_SIG_BITMSK		0x01		/*  0:N-OTSignal input	*/
#define	P_OT_SIG_BITMSK		0x02		/*  1:P-OTSignal input	*/
#define	DEC_SIG_BITMSK		0x04		/*  2:DECSignal input	*/
#define	EXT1_SIG_BITMSK		0x08		/*  3:EXT1Signal input	*/
#define	EXT2_SIG_BITMSK		0x10		/*  4:EXT2Signal input	*/
#define	EXT3_SIG_BITMSK		0x20		/*  5:EXT3Signal input	*/
#define	HWBB_SIG_BITMSK		0x40		/*  6:HWBBSignal input	*/
#define	ESTP_SIG_BITMSK		0x80		/*  7:ESTPSignal input	*/

#if 0
/* Defines of Control Mode */
#define	POS_CTRL_MODE		1			/* Position control mode	*/
#define	SPD_CTRL_MODE		2			/* Speed control mode	*/
#define	TRQ_CTRL_MODE		3			/* Torque control mode	*/
#define	NO_CTRL_MODE		0			/* Control mode disabled	*/
#endif

/* Defines of Extended Command Code */
enum EXTCMD_CODE {
	EXTCMD_MONSEL1 = 0x1,				/* 1: Monitor Selector 5,6 */
	EXTCMD_MONSEL2,						/* 2: Monitor Selector 7,8 */
	EXTCMD_KPGAIN,						/* 3: Position Loop Gain */
	EXTCMD_TRQLMT_P,					/* 4: Positive Torque Limit */
	EXTCMD_TRQLMT_N,					/* 5: Negative Torque Limit */
	EXTCMD_OUTSGNL						/* 6: SOn Output Signals */
};


/* Defines of Select Monitor Code */
enum MONSEL_CODE {
	MONSEL_FBSPD = 0,					/* 00: Feedback Speed */
	MONSEL_SPDREF,						/* 01: Command Speed */
	MONSEL_TRQREF,						/* 02: Command Torque */
	MONSEL_POSERR_L,					/* 03: Position Error(Low) */
	MONSEL_POSERR_H,					/* 04: Position Error(High) */
	MONSEL_PGCNT_L = 0xA,				/* 0A: PG Counter(Low) */
	MONSEL_PGCNT_H,						/* 0B: PG Counter(High) */
	MONSEL_FPGCNT_L,					/* 0C: FPG Counter(Low) */
	MONSEL_FPGCNT_H,					/* 0D: FPG Counter(High) */
	MONSEL_FBPOS_L,						/* 0E: Actual FB Position(Low) */
	MONSEL_FBPOSH,						/* 0F: Actual FB Position(High) */

	MONSEL_FBSPD_RPM,					/* 10: FB Speed = Un000 */
	MONSEL_SPDREF_RPM,					/* 11: Command Speed = Un001 */
	MONSEL_TRQREF_RAT,					/* 12: Command Torque = Un002 */
	MONSEL_ROT_PLS,						/* 13: Number of pulses from C-Phase = Un003 */
	MONSEL_ROT_DEG,						/* 14: Angle from C-Phase = Un004 */
	MONSEL_INSGNL2,						/* 15: Input Signals = Un005 */
	MONSEL_OUTSGNL2,					/* 16: Output Signals = Un006 */
	MONSEL_POSSPDREF,					/* 17: Command Position Speed = Un007 */
	MONSEL_POSERR2,						/* 18: Position Error = Un008 */
	MONSEL_LOAD_RATE,					/* 19: Accumulated load rate = Un009 */
	MONSEL_REGENE_RATE,					/* 1A: Regenarative load rate = Un00A */
	MONSEL_DBPWR_RATE,					/* 1B: DB resistance Power rate = Un00B */
	MONSEL_POSREF_PLS,					/* 1C: Input Position Pulses = Un00C */
	MONSEL_PGPLS,						/* 1D: FB Position Pulses of PG = Un00D */
	MONSEL_FPGPLS,						/* 1E: FB Position Pulses of FPG = Un00E */
	MONSEL_FPGSPD,						/* 1F: FB Speed of FPG = Un00F */

	MONSEL_TIMECNT = 0x22,				/* 22:  = Un012 */
	MONSEL_MULTI_TURN,					/* 23: Multi turn value of Sensor ON */
	MONSEL_INCR_PLS,					/* 24: Incremental puleses of Sensor ON */
	MONSEL_ABS_SCALEPOS_L,				/* 25: Absolute Linear Scale Position of Sensor ON */
	MONSEL_ABS_SCALEPOS_H,				/* 26: Absolute Linear Scale Position of Sensor ON */
	MONSEL_ENC_TEMPERATURE = 0x28,		/* 28: Encoder_temperature	*//* <S1D7> *//* <S1DE> */

	MONSEL_LTPOS_C_L = 0x30,			/* 30: Latch Position of C-Phase(Low) */
	MONSEL_LTPOS_C_H ,					/* 31: Latch Position of C-Phase(High) */
	MONSEL_LTPOS_EXT1_L ,				/* 32: Latch Position of EXT1(Low) */
	MONSEL_LTPOS_EXT1_H ,				/* 33: Latch Position of EXT1(High) */
	MONSEL_LTPOS_EXT2_L ,				/* 34: Latch Position of EXT2(Low) */
	MONSEL_LTPOS_EXT2_H ,				/* 35: Latch Position of EXT2(High) */
	MONSEL_LTPOS_EXT3_L ,				/* 36: Latch Position of EXT3(Low) */
	MONSEL_LTPOS_EXT3_H ,				/* 37: Latch Position of EXT3(High) */
	MONSEL_VPOSERR_L ,					/* 38: Virtual Position error(Low) */
	MONSEL_VPOSERR_H ,					/* 39: Virtual Position error(High) */
	MONSEL_INSGNL,						/* 3A: Input Signals Status */
	MONSEL_ALMWRN_CODE,					/* 3B: Alarm, Warning Code */
	MONSEL_INSGNL_2AXIS,				/* 3C: Input Signals Status 2-axis type *//* <S1C0> */

/* <S1D7>:start */
	MONSEL_ENVIRONMENT_MON_AMP = 0x40,	/* 40: Install Environment monitor AMP */
	MONSEL_ENVIRONMENT_MON_MOT,			/* 41: Install Environment monitor MOT */
	MONSEL_FAN_SURVIVOR,				/* 42: Fan Survivor	*/
	MONSEL_CAPA_SURVIVOR,				/* 43: Capa Survivor */
	MONSEL_RUSH_SURVIVOR,				/* 44: Rush Survivor */
	MONSEL_DB_SURVIVOR,					/* 45: DB Surivior */
	MONSEL_CONSUMED_POWER,				/* 46: Consumed Power */
	MONSEL_CONSUMPTION_ENE,				/* 47: Energy Consumption */
	MONSEL_CUMULATIVE_ENE,				/* 48: Cumulative Energy */
/*  <S1D7> */
	MONSEL_ANALOG_AD2 = 0x3C,			/* @@: Analog A/D value2 */
	MONSEL_ANALOG_AD1 = 0x3D,			/* @@: Analog A/D value1 */
	MONSEL_PERCLRLATCHPOS = 0x3E,		/* @@: Analog Perr Clear Latch Position */
	MONSEL_ANALOG_PCNTR = 0x3F,			/* @@: Analog Pulse Counter */
};



/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/* M-III InterFace */
typedef union EXTENDED_DATA_ {
	LONG	dwVal;
//	USHORT	SelMon[2]; //uniond
} EXTENDED_DATA;


/* Cyclic CommandStructure definition */
typedef struct SV_CYC_CMD_ {
	CMD_CTRLBITS	CmdCtrl;	/* Control command */
	UINT			CmdSeqBit;	/* Sequence control command */
	UINT			CmdLtBit;	/* Latch request */
	UINT			DOSignal;	/* I/OOutput command */
	LONG			Position;	/* Position command / -------  / --------   */
	LONG			Speed;		/* SpeedFF   / Speed command / Speed limit   */
	LONG			Torque;		/* TorqueFF / TorqueFF / Torque command */
	LONG			SelMon1;	/* Monitor selection1 */
	LONG			SelMon2;	/* Monitor selection2 */
	LONG			SelMon3;	/* Monitor selection3 */
	LONG			SelMon4;	/* Monitor selection4 */
	LONG			SelMon5;	/* Monitor selection5 */
	LONG			SelMon6;	/* Monitor selection6 */
	LONG			SelMon7;	/* Monitor selection7 */
	LONG			SelMon8;	/* Monitor selection8 */
	EXTENDED_DATA	ExtCmd1;	/* Extension command1 */
	EXTENDED_DATA	ExtCmd2;	/* Extension command2 */
	EXTENDED_DATA	ExtCmd3;	/* Extension command3 */
	EXTENDED_DATA	ExtCmd4;	/* Extension command4 */
} SV_CYC_CMD;


/* Cyclic ResponseStructure definition */
typedef struct SV_CYC_RSP_ {
	RSP_CTRLBITS	RspCtrl;	/* Control state */
	UINT			RspSeqBit;	/* Sequence control state */
	UINT			RspLtBit;	/* Latch status */
	UINT			RspSigBit;	/* Input signal status */
	LONG			FbPosition;	/* FBPosition */
	LONG			PosError;	/* Position deviation */
	LONG			Mon1;		/* Monitor data1 */
	LONG			Mon2;		/* Monitor data2 */
	LONG			Mon3;		/* Monitor data3 */
	LONG			Mon4;		/* Monitor data4 */
	LONG			Mon5;		/* Monitor data5 */
	LONG			Mon6;		/* Monitor data6 */
	LONG			Mon7;		/* Monitor data7 */
	LONG			Mon8;		/* Monitor data8 */
	LONG			SelMon1;	/* Monitor selection1 */
	LONG			SelMon2;	/* Monitor selection2 */
	LONG			SelMon3;	/* Monitor selection3 */
	LONG			SelMon4;	/* Monitor selection4 */
	LONG			SelMon5;	/* Monitor selection5 */
	LONG			SelMon6;	/* Monitor selection6 */
	LONG			SelMon7;	/* Monitor selection7 */
	LONG			SelMon8;	/* Monitor selection8 */
} SV_CYC_RSP;


typedef	struct	SV_CYC_CTRL_ {
	BOOL		CmdUpdate;		/* Synchronous control: Command update request	*/
	BOOL		RspUpdate;		/* Synchronous control: Response creation request	*/

	LONG		CmdUpdateCycle;	/* Position command update cycle[ScanBCounter]	*/

	ULONG		ExtCmdSel1;		/* Extended Command1 No. */
	ULONG		ExtCmdSel2;		/* Extended Command2 No. */
	ULONG		ExtCmdSel3;		/* Extended Command3 No. */
	ULONG		ExtCmdSel4;		/* Extended Command4 No. */
} SV_CYC_CTRL;


/* OPTION - SERVO I/F Structure definition */
typedef struct SV_OPTION_IF_ {
	SV_CYC_CTRL		CycCtrl;		/* Extended Command 1~4 */
	SV_CYC_CMD		CycCmd;			/* Cyclic Command */
	SV_CYC_RSP		CycRsp;			/* Cyclic Response */
} SV_OPTION_IF;


#endif 																		/* SERVO_IF_H_ */
/***************************************** end of file **********************************************/

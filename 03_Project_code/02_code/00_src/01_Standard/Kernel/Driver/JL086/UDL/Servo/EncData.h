/****************************************************************************************************/
/*																									*/
/*																									*/
/*		EncData.h : Encoder data structure definition														*/
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
#ifndef ENC_DATA_H

#include "MLib.h"
#include "JL086ServoIpHw.h"


/*--------------------------------------------------------------------------------------------------*/
/*		Encoder & motorIDInformation definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define MOT_ENC_MODEL_STR	22				/* Model												*/
#define MOT_ENC_DATE_STR	6				/* Date													*/
#define MOT_ENC_SNO_STR		22				/* Serial No.											*/

/****************************************************************************************************/
/*		Definition for linear motor																			*/
/****************************************************************************************************/
#define	LMX_MPOS_SFT		12				/* Shift number for phase position calculation									*/
#define	HPOS_TO_XPOS		20				/* 32bit - LMX_MPOS_SFT									*/
#define	FSX_FPOS_SFT		12


/*--------------------------------------------------------------------------------------------------*/
/*		Encoder Type Definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	MENCTYPE_SERIAL			0x00		/* Serial encoder									*/
#define	MENCTYPE_PULSE			0x10		/* Pulse encoder										*/
#define	MENCTYPE_ANALOG 		0x20		/* Analog encoder									*/
#define	MENCTYPE_ENCLESS		0xFF		/* Encoderless										*/

#define	MENC_INCRE_TYPE			0x00		/* Using absolute as increment							*/
#define	MENC_MABSO_TYPE			0x10		/* Multi-rotation absolute encoder								*/
#define	MENC_SABSO_TYPE			0x20		/* One-rotation absolute encoder								*/

/*--------------------------------------------------------------------------------------------------*/
/*		Serial Converter Type Definition															*/
/*--------------------------------------------------------------------------------------------------*/
#define	SCONVTYPE_NONE			0x00		/* No serial converter								*/
#define	SCONVTYPE_STANDARD_INC	0x01		/* Standard increment type									*/
#define SCONVTYPE_MITUTOYO_ABS	0x02		/* Mitutoyo absolute type								*/
#define	SCONVTYPE_ENDAT_ABS		0x03		/* EnDatType											*/
#define	SCONVTYPE_SONY_INC		0x04		/* SMSIncre type									*/
#define	SCONVTYPE_SONY_ABS		0x05		/* SMSAbsolute type									*/

/*--------------------------------------------------------------------------------------------------*/
#define SCALE_EXP_PM			4			/* Linear scale pitch index[pm]							*/
#define SCALE_EXP_UM			1			/* Linear scale pitch index[um]							*/

/*--------------------------------------------------------------------------------------------------*/
/*		PaoSeqMcmd Interface Definition					<S02D>										*/
/*--------------------------------------------------------------------------------------------------*/
#define	KPX_PAOSEQ_NOCMD		0x00		/* No Command											*/
#define	KPX_MTLATCH_REQUEST		0x10		/* Multiturn Latch Request (ScanC ==> ScanA)			*/
#define	KPX_MTLATCH_FINISH		0x11		/* Multiturn Latch Finish  (ScanC <== ScanA)			*/
#define	KPX_MTLATCH_ERROR		0x1E		/* Multiturn Latch Error   (ScanC <== ScanA)			*/
#define	KPX_IPLSOUT_REQUEST		0x20		/* IncPulse Output Request (ScanC ==> ScanA)			*/
#define	KPX_IPLSOUT_FINISH		0x21		/* IncPulse Output Finish  (ScanC <== ScanA)			*/

/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Definition of constants for position/speed correction using change time timer					<S067>						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#define	VFBTINMINPLS	2								/* Change timer method execution minimum difference position pulse[pluse]	*/
#define	VFBTINMINCNT	5								/* Change timer method execution minimum difference position pulse consecutive number of times	*/
#define	TIMBASECLK		6								/* Encoder clock frequency[MHz]: 6 MHz			*/
#define	SAENCCLKNUM		(KPI_SACYCLENS*TIMBASECLK/1000)	/* Number of encoder clocks in calculation cycle:	62.5us=375			*/
#define	SAENCCLKNUMC2	(SAENCCLKNUM*2)					/* Zero pulse change time maximum value 375 * 2 = 750		*/
#define	SAENCCLKNUMC2D3	(SAENCCLKNUM*2/3)				/* Change time minimum value VFBTINMINPLS=2 -> 250			*/
#define	ENCLCHCLKNUM	16								/* Encoder latch period(2.667[us])Number of clocks	*/


/****************************************************************************************************/
/*																									*/
/*		Serial encoder IF definition																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Serial encoder command definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define		SPG_NOP					0x0000		/* Nop   Command									*/
#define		SPG_BREAK				0x0001		/* Break Command									*/
#define		SPG_ORGCLR				0x0007		/* Origin Clear Command								*/
#define		SPG_PREAD				0x0008		/* Parameter Read Command							*/
#define		SPG_PWRITE				0x0010		/* Parameter Write Command							*/
#define		SPG_ECHO				0x00FF		/* Echo  Command									*/
#define		SPG_BANK				0x000A		/* Bank Command										*//* <S0D7> */
/*--------------------------------------------------------------------------------------------------*/
#define		SPG_RDADR				0x0041		/* Read  Address Command							*/
#define		SPG_WRADR				0x0042		/* Write Address Command							*/
#define		SPG_OPENKEY				0x594F		/* Open  Key Command								*/
#define		SPG_CLOSEKEY			0x004F		/* Close Key Command								*/
/*--------------------------------------------------------------------------------------------------*/
#define		SPG_MTRST				0x0004		/* MultiTurn Reset Command							*/
#define		SPG_MTLMTCHG			0x0006		/* MultiTurn Change Command							*/
#define		SPG_PMODEOFF			0x00F0		/* Protect Mode Off Command							*/
#define		SPG_ZEROSET				0x0012		/* Zero Set Command									*/
#define		SPG_SETCOM				0x00FA		/* Set Com Command									*/
/*--------------------------------------------------------------------------------------------------*/
#define		KPX_CDAT_ENC0_ESYNC		0x0002		/* CH0External synchronization enabled,CH1External synchronization disabled					*//* <S04D>:rename */
//#define		KPX_CDAT_SENC_ESYNC		0x0003		/* CH0,CH1External synchronization disabled 							*/
#define		KPX_CDAT_ENC1_ESYNC		0x0001		/* CH0External synchronization disabled,CH1External synchronization enabled					*//* <S04D> */
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder read interrupt delay time definition															*/
/*--------------------------------------------------------------------------------------------------*/
#define		ENCRDDLY_4MBPS			0x1100		/* 4Mbps(54.4us)								*//* <S09C> */
#define		ENCRDDLY_8MBPS			0x8F0		/* 8Mbps(28.6us)								*/


/****************************************************************************************************/
/*																									*/
/*		Serial encoder IF definition																	*/
/*																									*/
/****************************************************************************************************/
//****************************************************************************************************
/*		Response Data Buffer																		*/
//****************************************************************************************************
/* Header Structure */
typedef struct _ENC_RSP_HEADER
{
	UCHAR	CmdCode;
	UCHAR	Status;
} ENC_RSP_HEADER;

/* Motor Position Data Low Order */
typedef union _MOT_POS_LOW
{
	struct {
		unsigned int InvalidData	: 4;
		unsigned int PositionL		: 4;
	} Data;
	unsigned char PositionL;
} MOT_POS_LOW;

/* Incre Comp. Value and Pole Center */
typedef union _INCRE_UNIQUE_DATA
{
	struct {
		unsigned int Signal_Z		: 1;
		unsigned int Pole_W			: 1;
		unsigned int Pole_V			: 1;
		unsigned int Pole_U			: 1;
		unsigned int IncreComp		: 12;
	} Data;
	unsigned char IncreData;
} INCRE_UNIQUE_DATA;

/* NOP, BREAK, RESET, ECHO, INIT */
typedef union _ENC_RSP_NOP
{
	struct {
		ENC_RSP_HEADER		header;
		UCHAR				Echo[14];
	} cdata;
	struct {
		USHORT				header;
		USHORT				Echo[7];
	} sdata;
	ULONG		ldata[4];
} ENC_RSP_NOP;

/* OPEN_KEY */
typedef union _ENC_RSP_OPENKEY
{
	struct {
		ENC_RSP_HEADER		header;
		UCHAR				Echo[2];
		UCHAR				EncVersion;
		UCHAR				dummy[11];
	} cdata;
	struct {
		USHORT				header;
		USHORT				Echo;
		USHORT				dummy[6];
	} sdata;
	ULONG		ldata[4];
} ENC_RSP_OPENKEY;

/* MUL_LIMIT */
typedef union _ENC_RSP_MTLIMIT
{
	struct {
		ENC_RSP_HEADER		header;
		UCHAR				MultiTurnLimitL;
		UCHAR				MultiTurnLimitH;
		UCHAR				CntrReloadData1;
		UCHAR				CntrReloadData2;
		UCHAR				CntrReloadData3;
		UCHAR				CntrReloadData4;
		UCHAR				PreChangeMTDataL;
		UCHAR				PreChangeMTDataH;
		UCHAR				PostChangeMTDataL;
		UCHAR				PostChangeMTDataH;
		UCHAR				dummy[4];
	} cdata;
	struct {
		USHORT				header;
		USHORT				MultiTurnLimit;
		USHORT				CntrReloadData12;
		USHORT				CntrReloadData34;
		USHORT				PreChangeMTData;
		USHORT				PostChangeMTData;
		USHORT				dummy[2];
	} sdata;
	ULONG		ldata[4];
} ENC_RSP_MTLIMIT;

/* READ_PRM, WRITE_PRM */
typedef union _ENC_RSP_PRMRD
{
	struct {
		ENC_RSP_HEADER		header;
		UCHAR				PrmAddress;
		UCHAR				ReadSize;
		UCHAR				ReadData[12];
	} cdata;
	struct {
		USHORT				header;
		USHORT				Operand;
		USHORT				PrmData[6];
	} sdata;
	ULONG		ldata[4];
} ENC_RSP_PRMRD;

/* SET_COM */
typedef union _ENC_RSP_SETCOM
{
	struct {
		ENC_RSP_HEADER		header;
		UCHAR				Echo;
		UCHAR				BaudRate;
		UCHAR				dummy2[12];
	} cdata;
	struct {
		USHORT				header;
		USHORT				Echo_Brate;
		USHORT				dummy[6];
	} sdata;
} ENC_RSP_SETCOM;

/* P1Mode */
typedef union _ENC_RSP_P1MODE
{
	struct {
		UCHAR				dummy[2];
		UCHAR				AlmCode;
		UCHAR				Data1;
		UCHAR				Data2;
		UCHAR				TimeBaseTimerL;
		UCHAR				TimeBaseTimerH;
		UCHAR				LastChangePosTimeL;
		UCHAR				LastChangePosTimeH;
//		MOT_POS_LOW			PositionL;
		UCHAR				PositionL;
		UCHAR				PositionM;
		UCHAR				PositionH;
//		INCRE_UNIQUE_DATA	IncreData;
		UCHAR				IncreData;
	} cdata;
	USHORT				sdata[8];
	ULONG				ldata[4];
} ENC_RSP_P1MODE;

/* BANK *//* <S0D7> */
typedef union _ENC_RSP_BANK
{
	struct {
		ENC_RSP_HEADER		header;			/* D0,D1	*/
		UCHAR				BankNum;		/* D2		*/
		UCHAR				Echo;			/* D3		*/
		UCHAR				BankSizeL;		/* D4		*/
		UCHAR				BankSizeH;		/* D5		*/
		UCHAR				Cdummy[10];		/* D6~D15	*/
	} cdata;
	struct {
		USHORT				header;			/* D0,D1	*/
		USHORT				Echo_BankNum;	/* D2,D3	*/
		USHORT				Sdummy[6];		/* D4~D15	*/
	} sdata;
	ULONG		ldata[4];
} ENC_RSP_BANK;


//****************************************************************************************************
/*		Command Data Buffer																			*/
//****************************************************************************************************
/* Header Structure */
typedef struct _ENC_CMD_HEADER
{
	UCHAR	CmdCode;
	UCHAR	InvalidData;
} ENC_CMD_HEADER;

/* NOP, BREAK, RESET, ECHO, INIT */
typedef union _ENC_RESP_NOP
{
	struct {
		ENC_CMD_HEADER		header;
		UCHAR				TestPattern[14];
	} cdata;
	struct {
		USHORT				CmdCode;
		USHORT				TestPattern[7];
	} sdata;
	ULONG		ldata[4];
} ENC_CMD_NOP;

/* OPEN_KEY */
typedef union _ENC_CMD_OPENKEY
{
	struct {
		UCHAR		CmdCode;
		UCHAR		KeyWord[15];
	} cdata;
	struct {
		USHORT		CmdCode;
		USHORT		KeyWord[7];
	} sdata;
	ULONG		ldata[4];
} ENC_CMD_OPENKEY;

/* MUL_LIMIT */
typedef union _ENC_CMD_MTLIMIT
{
	struct {
		ENC_CMD_HEADER		header;
		UCHAR				MultiTurnLimitL;
		UCHAR				MultiTurnLimitH;
		UCHAR				dummy[12];
	} cdata;
	struct {
		USHORT				CmdCode;
		USHORT				MultiTurnLimit;
		USHORT				dummy[6];
	} sdata;
	ULONG		ldata[4];
} ENC_CMD_MTLIMIT;

/* READ_PRM, WRITE_PRM */
typedef union _ENC_CMD_PRMRD
{
	struct {
		ENC_CMD_HEADER		header;
		UCHAR				PrmAddress;
		UCHAR				ReadSize;
		UCHAR				dummy[12];
	} cdata;
	struct {
		USHORT				CmdCode;
		USHORT				Operand;
		USHORT				PrmData[6];
	} sdata;
	ULONG		ldata[4];
} ENC_CMD_PRMRD;

/* SET_COM */
typedef union _ENC_CMD_SETCOM
{
	struct {
		ENC_CMD_HEADER		header;
		UCHAR				blank;
		UCHAR				BaudRate;
		UCHAR				dummy[12];
	} cdata;
	struct {
		USHORT				CmdCode;
		USHORT				blank_Brate;
		USHORT				dummy[6];
	} sdata;
} ENC_CMD_SETCOM;


/* BANK *//* <S0D7> */
typedef union _ENC_CMD_BANK
{
	struct {
		ENC_RSP_HEADER		header;			/* D0,D1	*/
		UCHAR				CBankNum;		/* D2		*/
		UCHAR				ReadData[13];	/* D3~D15	*/
	} cdata;
	struct {
		USHORT				CmdCode;		/* D0,D1	*/
		USHORT				SBankNum;		/* D2,D3	*/
		USHORT				dummy[6];		/* D4~D15	*/
	} sdata;
	ULONG		ldata[4];
} ENC_CMD_BANK;

//****************************************************************************************************
/*		Serial encoderIF												*/
//****************************************************************************************************
typedef union _ENC_DATA_BUF
{ /* for Encoder communication */
	ENC_RSP_NOP			EncRspNormal;
	ENC_RSP_OPENKEY		EncRspKeyCntrl;
	ENC_RSP_MTLIMIT		EncRspMTLimit;
	ENC_RSP_PRMRD		EncRspPrmAccess;
	ENC_RSP_P1MODE		EncRspP1Mode;
	ENC_RSP_SETCOM		EcnRspSetCom;
	ENC_RSP_BANK		EncRspBank;			/* <S0D7> */
	ENC_CMD_NOP			EncCmdNormal;
	ENC_CMD_OPENKEY		EncCmdKeyCntrl;
	ENC_CMD_MTLIMIT		EncDmcMTLimit;
	ENC_CMD_PRMRD		EncCmdPrmAccess;
	ENC_CMD_SETCOM		EcnCmdSetCom;
	ENC_CMD_BANK		EncCmdBank;			/* <S0D7> */
	USHORT				UsBuf[8];
	ULONG				UlBuf[8];
} ENC_DATA_BUF;


//****************************************************************************************************
/*		Serial encoderIFDefinition																	*/
//****************************************************************************************************
typedef	struct SENCX {
	VULONG			*TxBuf;					/* Transmit buffer address									*/
	VULONG			*RxBuf;					/* Receive buffer address									*/
	VULONG			*RxFlg;					/* Reception completion flag address								*/
	VULONG			*TxStart;				/* Transmission start write register address					*/
	VULONG			TxCmd;					/* Transmission start command data(ESYNC0/ESYNC1)				*/
	UINT			ch;						/* ConnectionChannel											*/
	ENC_DATA_BUF	TxData;					/* Transmission data storage buffer								*/
	ENC_DATA_BUF	RxData;					/* Receive data storage buffer								*/
	ASIC_HW_IF_STR	*SencSetReg;			/* Asic H/W Register Address							*/
											/*	Encoder I/FDedicated SGD7WIn the case of2Also use axis1Axis address	*/
	ASIC_HW_IF_STR	*PosCmpSetReg;			/* Asic H/W Register Address							*/
											/*	Position correction module I/FDedicated Ch0teeth1Axis number/Ch1teeth2Axis number		*/
} SENCX;

/****************************************************************************************************/
/*		Encoder & motorIDInformation definition																*/
/****************************************************************************************************/
typedef struct IDINFO {
	UCHAR	Model[MOT_ENC_MODEL_STR];		/* Model[22]											*/
	UCHAR	Date[MOT_ENC_DATE_STR];			/* Date[6]												*/
	UCHAR	SNo[MOT_ENC_SNO_STR];			/* Serial No.[22]										*/
} IDINFO;

/****************************************************************************************************/
/*		Motor function flag(PnF24)Information definition				Supports motor carrier frequency switching<S117>		*/
/****************************************************************************************************/
typedef union _MFUNCF {
	struct {
		UINT		chgPwm:1;					/* 0:PMWFrequency change										*/
		UINT		LspdOl:1;					/* 1:Motor low speedOLDetection									*/
		UINT		OCset:1;					/* 2:OCDetection current setting										*/
		UINT		VLimlv:1;					/* 3:PnEB3 Voltage command limit level change						*/
		UINT		IdrefLimlv:1;				/* 4:PnEB4 dVoltage limit level for axis current command limit			*/
		UINT		wfMode3:1;					/* 5:Field weakening controlmode3									*/
		UINT		rgonPhaseCmp:1;				/* 6:Phase lead compensation during regeneration									*/
		UINT		IPMmode:1;					/* 7:IPMControl											*/
		UINT		dummy:24;					/* 8-31:dummy											*/
	}f;
	ULONG	ul;
} MFUNCF;


/****************************************************************************************************/
/*		Encoder parameter definition																	*/
/****************************************************************************************************/
typedef struct MENCPRM {
	DBYTEX	typm_e;					/* PnF00 : Motor type(8bit), Input voltage(4bit), Encoder type(4bit)	*/
	USHORT	encver;					/* PnF01 : Encoder software version								*/
	USHORT	motw;					/* PnF02 : Motor capacity												*/
	DBYTEX	bit_dp;					/* PnF03 : Encoder bit number, Position data decimal point position					*/
	USHORT	limmlt;					/* PnF04 : Multiturn limit										*/
	DBYTEX	vrat_max;				/* PnF05 : Rated speed, Maximum speed										*/
	DBYTEX	oslv_pol;				/* PnF06 : OS detection level, Number of poles									*/
	USHORT	rattrq;					/* PnF07 : Rated torque												*/
	USHORT	maxtrq;					/* PnF08 : Maximum torque												*/
	USHORT	irat;					/* PnF09 : Rated current													*/
	USHORT	imax;					/* PnF0A : Instantaneous maximum current												*/
	USHORT	emfcmp;					/* PnF0B : EMF constant												*/
	USHORT	jmot;					/* PnF0C : Rotor inertia											*/
	USHORT	motr;					/* PnF0D : Armature winding resistance											*/
	USHORT	motl;					/* PnF0E : Armature inductance										*/
	USHORT	bastrq;					/* PnF0F : Overload detection base current										*/
	/*------------------------------------------------------------------------------------------------------*/
	USHORT	midtrq;					/* PnF10 : Overload detection intermediate current										*/
	USHORT	midolt;					/* PnF11 : Overload detection intermediate time										*/
	USHORT	mtrq2;					/* PnF12 : Overload detection intermediate current 2										*/
	USHORT	mtolt2;					/* PnF13 : Overload detection intermediate time 2										*/
	DBYTEX	phscmpM1;				/* PnF14 : Phase compensation 1(Compensation value,Start speed)								*/
	USHORT	ppitch;					/* PnF15 : Pole pitch												*/
	USHORT	motlq0;					/* PnF16 : qAxis inductanceLq0										*/
	USHORT	motlq1;					/* PnF17 : qAxis inductanceLq1										*/
	DBYTEX	exp_trat_jmot;			/* PnF18 : Lower: Rated torque index, Upper: Rotor inertia index			*/
	DBYTEX	exp_motw_spd;			/* PnF19 : Lower: Rated output index, Upper: Rotation speed index						*/
	DBYTEX	BitnoDpoint;			/* PnF1A : Fulque encoder bit number, Position data decimal point position			*/
	USHORT	poleoffset;				/* PnF1B : Pole sensor offset amount									*/
	USHORT	motresfrq;				/* PnF1C : Notch filter frequency for motor resonance suppression						*/
	USHORT	phscmpM2;				/* PnF1D : Phase angle correction value												*/
	USHORT	flg_wf;					/* PnF1E : Flag													*/
	USHORT	exmotprm;				/* PnF1F : Motor parameter expansion area selection								*/
	/*------------------------------------------------------------------------------------------------------*/
	USHORT	chng_baudrate;			/* PnF20 : Encoder communication speed setting(Supported communication speed, switchability)		*/
	/*------------------------------------------------------------------------------------------------------*//* <S0D7>:start */
	MFUNCF	motfuncf;				/* PnF24 : 	Motor function flag2										*//*Supports motor carrier frequency switching<S117>*/
	USHORT	mot_pwmf;				/* PnF25 : 	MotorPWMFrequency											*/
	USHORT	mot_ibs;				/* PnF26 : 	Motor low-speed overload detection base current							*/
	USHORT	mot_imd;				/* PnF27 : 	Motor low-speed overload detection intermediate current							*/
	USHORT	mot_imdt;				/* PnF28 : 	Motor low-speed overload detection intermediate time							*/
	USHORT	mot_imd2;				/* PnF29 : 	Motor low-speed overload detection intermediate current 2							*/
	USHORT	mot_imdt2;				/* PnF2A : 	Motor low-speed overload detection intermediate time 2							*/
	USHORT	mot_ichgspd;			/* PnF2B : 	Motor low-speed overload detection rotation speed threshold						*/
	USHORT	mot_oc;					/* PnF2C : 	MotorOCDetection current										*/
	USHORT	mot_TunLess;			/* PnF2D : 	Adjustmentless mode selection										*/
	DBYTEX	RefAdjust_ForWF;		/* PnF2E : 	Field weakening motor adjustment value(Voltage command,dAxis current command)				*/
	USHORT	LCompForWF;				/* PnF2F : 	Field weakeningLCorrection coefficient										*/
	DBYTEX	RegPhscmp;				/* PnF30 : 	Phase lead compensation during regeneration(Start speed,Maximum rotation speed)				*/
	/*------------------------------------------------------------------------------------------------------*//* <S0D7>:end */
} MENCPRM;

/****************************************************************************************************/
/*		Encoder initial parameter definition																*/
/****************************************************************************************************/
typedef struct MENC_REF_PRM {
//	USHORT	b_prm2;					/* Pn002 : Function selection application switch 2								*/
//	USHORT	b_prm80;				/* Pn080 : Function selection application switch 80								*/
//	USHORT	mtlmt;					/* Pn205 : Multiturn limit setting								*/
//	ULONG	scalepitch2l;			/* Pn282 : Linear scale pitch									*/
	USHORT	syssw3;					/* PnE0D : System switch 3									*/
	USHORT	scnvorgoffset;			/* PnE8C : Serial conversion unit origin offset					*/
	USHORT	phsofst;				/* PnE8D : Current zero phase position										*/

	KSGAIN	MaxTrq;					/* Max.Torque						[Nm]		[N]				*/
	KSGAIN	Kfbpls;					/* FB Pulse Factor (Menc/Fenc)		[rad/p]		[m/p]			*/
	KSGAIN	Jmot;					/* Motor Rotor Inertia				[kg*m^2]	[kg]			*/
} MENC_REF_PRM;

/****************************************************************************************************/
/*		Data for speed correction calculation using change time timer													*/
/****************************************************************************************************/
typedef struct {
	LONG	MotSpeed;										/* Motor speed			[2^24/OvrSpd]	*/
	LONG	dPos;											/* This time difference position FB		[pulse/scan]	*/
	LONG	LastdPos;										/* Previous difference position FB		[pulse/scan]	*/
	USHORT	HenkaItiTimer;									/* Change position timer						*/
	USHORT	TimeInterval;									/* Change time interval							*/
	USHORT	BaseTimeInterval;								/* Base timer change time interval				*/
	USHORT	BaseTimer;										/* Base timer							*/
	USHORT	HenkaTime;										/* Change time								*/
	USHORT	LastHenkaTime;									/* Last change time							*/
	USHORT	LastBaseTimer;									/* Previous base timer						*/
	USHORT	ModChgCnt;										/* Speed detection method switching counter			*/
	UCHAR	Modflag;										/* Base timer speed calculation execution flag			*/
	UCHAR	LastModflag;									/* Previous base timer speed calculation execution flag		*/
	UCHAR	LastSeRxErr;									/* Previous received error flag					*/
	UCHAR	Dummy;											/* for Alignment						*/
} VFBTIMBASE;

/****************************************************************************************************/
/*		Data for speed correction calculation using change time timer(For Fulk)											*/
/****************************************************************************************************/
typedef struct ENCVFBTIMBASE {
	LONG	fencspeed;										/* Load speed				[2^24/OvrSpd]	*/
	USHORT	BaseTimeInterval;								/* Base timer change time interval				*/
	USHORT	BaseTimer;										/* Base timer							*/
	USHORT	LastBaseTimer;									/* Previous base timer						*/
	UCHAR	Modflag;										/* Base timer speed calculation execution flag		*/
	UCHAR	LastModflag;									/* Previous base timer speed calculation execution flag	*/
} ENCVFBTIMBASE;

/****************************************************************************************************/
/*		Encoder related initialization parameter definition														*/
/****************************************************************************************************/
typedef struct MENC_INIT_PRM {
	MENCPRM	*MencP;					/* PnF** : Pointer to encoder parameter definition					*/
	LONG	scalepitch2l;			/* Pn282 : Linear scale pitch									*/
	USHORT	b_prm2;					/* Pn002 : Function selection application switch2								*/
	USHORT	b_prm80;				/* Pn080 : Function selection application switch80								*/
	USHORT	mtlmt;					/* Pn205 : Multiturn limit setting								*/
	USHORT	fcplsml;				/* Pn20A : Number of external scale pitches								*//* <S04D> */
	DBYTEX	spgfail;				/* PnE1E : Lower:S-PGPermissible number of reception failures, Upper level:FS-PGPermissible number of reception failures*/
	USHORT	syssw1;					/* PnE0B: System switch1										*//* <S1BE> */
} MENC_INIT_PRM;

/****************************************************************************************************/
/*		Definition for frequency division output function															<S02D>			*/
/****************************************************************************************************/
typedef struct DIVINFO {
	BOOL	DivOut;							/* Divided pulse outputFlag									*/
	BOOL	DivOutEnable;					/* Division pulse output enabledFlag (For sample hardware)			*/
	BOOL	PlsTrn13bit;					/* Pulse conversion13bitSpecificationFlag(ASICFor setting)					*/
	BOOL	DivPcmaskCtrl;					/* DivisionCPhase output mask controlFlag							*/
	BOOL	DivRatFix;						/* Division ratio/maximum speed : Fixed/calculation (Linear only)			*/
	BOOL	DivModeStatus ;					/* Division mode setting status 0:Normal 1:DCC1 2:DCC2			*/
	BOOL	DivCMaskStatus ;				/* DivisionCPhase mask status	0:Mask Off  1: Mask On			*/
//----------------------------------------------------------------------------------------------------
	BOOL	DivCpass;						/* DivCphase Pass										*/
	LONG	DivCpassPos;					/* Cphase Pass Div.Position								*/
//----------------------------------------------------------------------------------------------------
	UCHAR	DivMode;						/* Divided mode(Not used)					<S03F>			*/
	UCHAR	DivPass;						/* Division through mode						<S03F>			*/
	UCHAR	DivOutSft;						/* Divided output shift number							<S03F>		*/
	UCHAR	Dummy;							/* for Alignment						<S03F>			*/
	USHORT	DivRate;						/* Division ratio setting value							<S03F>			*/
	LONG	DivOutGain;						/* Divided output gain(Linear)								*/
	LONG	DivOutCmskChk;					/* Divided outputCPhase mask check data(Linear)				*/
	LONG	DivOvrSpd;						/* Divided output overspeed level [2^24/OvrSpd]			*/
	LONG	MaxDivSpd;						/* Maximum speed at upper limit of division output frequency [mm/s]				*/
	LONG	MaxDivRat;						/* Division ratio setting upper limit at maximum speed [pulse/spitch]			*/
//----------------------------------------------------------------------------------------------------
	CHAR	PaoSeqStep;						/* PAO Serial Output Sequence Step			<S03F>		*/
	UCHAR	PaoSeqWait;						/* PAO Serial Output Sequence Wait Counter		<S03F>	*/
	UCHAR	IncPulseReq;					/* PAO IncrePulse Output Request TRUE/FALSE		<S03F>	*/
	UCHAR	PaoSeqMcmd;						/* PAO Serial Output Sequence Cmd/Response	<S03F>		*/
	USHORT	PaoSeqData;						/* PAO Serial Output Sequence Work Data		<S03F>		*/
	SHORT	MturnLatch;						/* PAO Multiturn Latch Data				<S03F>			*/
	LONG	IncPulseOut;					/* PAO IncrePulse Output Data							*/
	UCHAR	PsoSeqStep;						/* PSO Serial Output Sequence Step	<S0C2>			 */
	UCHAR	PsoSeqWait;						/* PSO Serial Output Sequence Wait Counter<S0C2>      */
//	SHORT	PsoMturnLatch;					/* PSO Serial Multiturn Latch Data	<S0C2>			 */
	USHORT	PsoMturnLatch;					/* PSO Serial Multiturn Latch Data	<S0C2>	<S226>		 */
	ULONG   PsoLinearData_L;                /* PSO Serial Output Linear Data_L  <S0C2>            */
	SHORT   PsoLinearData_H;                /* PSO Serial Output Linear Data_H   <S0C2>            */
	LONG	PsoIncPulseOut;					/* PSO Serial IncrePulse Output Data <S0C2>         */
//----------------------------------------------------------------------------------------------------
	LONG	DivOutPos;						/* Frequency division output pulse										*/
	LONG	DivOutRem;						/* Division output pulse remainder									*/
	LONG	InitDivPosOffset;				/* Division output pulse offset								*/
//----------------------------------------------------------------------------------------------------
	DBYTEX	Bit_dp;							/* PnF03 : Encoder bit number, Position data decimal point position		*/
	USHORT	Limmlt;							/* PnF04 : Multiturn limit							*/
	BOOL	RvsDir;							/* Reverse rotation mode	(Pn000.0 : Rotation direction selection(Reverse rotation mode))	*/
//----------------------------------------------------------------------------------------------------
	BOOL	DivOutASIC;						/* Select encoder position data for divided output(0:S/W; 1:H/W)	*/
											/*   1(H/W):Select position data for divided outputJL-086is used for direct pulse conversion	*/
											/*   0(S/W):Select position data for divided outputCPUPulse conversion via		*/
//----------------------------------------------------------------------------------------------------
	UCHAR	IncreBit_dp;					/* Increment correction value minimum bit position							*//*<S161>*/
//----------------------------------------------------------------------------------------------------
} DIVINFO;


/****************************************************************************************************/
/*		EncoderIFData definition																		*/
/****************************************************************************************************/
typedef	struct MENCV {
	//Parameter area
//----------------------------------------------------------------------------------------------------
	IDINFO	MotInfo;						/* Motor Information									*/
	IDINFO	EncInfo;						/* Motor Encoder Information							*/
	IDINFO	*pActMotInfo;					/* Active Motor Information								*//* <S04D> */
	IDINFO	*pActEncInfo;					/* Active Motor Encoder Information						*//* <S04D> */
//----------------------------------------------------------------------------------------------------
	SENCX	EncIf;							/* Serial Encoder Interface Definitions					*/
//----------------------------------------------------------------------------------------------------
	LONG	AxisMotType;					/* Motor Type Rotary or Linear							*/
//----------------------------------------------------------------------------------------------------
	BOOL	EncCom8Mbps;					/* Encoder ComSpeed 8Mbps								*/
	BOOL	AbsoEncoder;					/* Absolute Encoder										*/
	BOOL	Sigma3Encoder;					/* Sigma-3 Encoder										*/
	BOOL	LowSpdMotor;					/* Low Speed Motor										*/
	BOOL	RevAsmMotor;					/* Motor reverse connectionFlag									*/
#if 0 /*<S02D>*/
	BOOL	DivOut;							/* Divided pulse outputFlag									*/
	BOOL	DivOutEnable;					/* Division pulse output enabledFlag (For sample hardware)			*/
	BOOL	PlsTrn13bit;					/* Pulse conversion13bitSpecificationFlag(ASICFor setting)					*/
	BOOL	DivPcmaskCtrl;					/* DivisionCPhase output mask controlFlag							*/
#endif/*<S02D>*/
	BOOL	CphaseLess;						/* CphaseLess Encoder									*/
	BOOL	PhaseOrderUWV;					/* Power output phase sequence UWV									*/
	BOOL	withPoleSensor;					/* With pole sensor (Linear only)						*/
	BOOL	MotOrgChkAlways;				/* Constant monitoring of the motor origin (Linear only)						*/
#if 0 /*<S02D>*/
	BOOL	DivRatFix;						/* Division ratio/maximum speed : Fixed/calculation (Linear only)			*/
#endif/*<S02D>*/
	BOOL	CoreLessLinearMotor;			/* CoreLess Linear Motor								*/
	BOOL	SigmaTracMotor;					/* Sigma Trac Linear Motor								*/
	BOOL	ScaleWriteEnable;				/* Scale write availabilityFlag								*/
	BOOL	OrgClrByWrAdrCmd;				/* Origin Clear By Write Address Command 				*/
	BOOL	TuningLessLowGainMotor;			/* Adjustment-less control Low noise setting motor(Only without adjustment)			*/
	BOOL	WeakenFieldEnable;				/* VoltageFBMethod field-weakening control forced enable flag					*/
	BOOL	SenSpecEnable;					/* EncoderPrm 0x43(Sensor specifications)(TRUE=Enabled)				*//* <S04D> */
	BOOL	PoleWriteEnable;				/* Current phase parameter scale write permission flag			*//* <S04D> */
//----------------------------------------------------------------------------------------------------
	LONG	PulseNo;						/* Pulse Number	[pulse/rev] or [pulse/360deg]			*/
	LONG	HalfPulseNo;					/* PulseNo/2											*/
	LONG	PulseNo30deg;					/* Pulse Number [pulse/30deg]							*/
	LONG	SoftVer;						/* Encoder soft version for Fluke encoder						*//* <S04D> */
//----------------------------------------------------------------------------------------------------
//	BOOL	MotType;						/* Motor Type											*/
	LONG	EncType;						/* Motor Encoder Type									*/
	LONG	AbsoEncType;					/* Absolute Encoder Type								*/
	LONG	TypeCode;						/* Type Code for EndatType								*/
	LONG	SerialConvType;					/* Serial Converter Type								*/
//----------------------------------------------------------------------------------------------------
	LONG	MorgSft;						/*														*/
	LONG	MorgPassSft;					/*														*/
	LONG	IncreCompSft;					/*														*/
	LONG	MultiturnSft;					/*														*/
//----------------------------------------------------------------------------------------------------
#if 0 /*<S02D>*/
	LONG	DivMode;						/* Divided mode(Not used)									*/
	LONG	DivPass;						/* Division through mode										*/
	LONG	DivOutSft;						/* Divided output shift number										*/
	LONG	DivRate;						/* Division ratio setting value											*/
	LONG	DivOutGain;						/* Divided output gain(Linear)								*/
	LONG	DivOutCmskChk;					/* Divided outputCPhase mask check data(Linear)				*/
//----------------------------------------------------------------------------------------------------
	LONG	DivOvrSpd;						/* Divided output overspeed level [2^24/OvrSpd]			*/
	LONG	MaxDivSpd;						/* Maximum speed at upper limit of division output frequency [mm/s]				*/
	LONG	MaxDivRat;						/* Division ratio setting upper limit at maximum speed [pulse/spitch]			*/
#endif/*<S02D>*/
//----------------------------------------------------------------------------------------------------
	LONG	MposSign;						/* Sign  Data for Motor Position Cal.					*/
	LONG	MposSftX;						/* Shift Data for Motor Position Cal.					*/
	LONG	MposSftR;						/* Shift Data for Motor Position Cal.					*/
//----------------------------------------------------------------------------------------------------
	LONG	Kmotphase;						/* Gain for Motor Phase Cal.							*/
	LONG	Kinvphase;						/* Gain for Inverse Motor Phase Cal.					*/
//----------------------------------------------------------------------------------------------------
	LONG	ComErrLimit;					/* Motor Encoder Com. Error Limit						*/
	LONG	AccErrLvl;						/* Motor Encoder Acc. Error Check						*/
	LONG	PhaChkSpd;						/* Motor Phase(UVW) Check Speed Upper Limit				*/
//----------------------------------------------------------------------------------------------------
	KSGAIN	Kmovpos;						/* Gain for Linear Motor Move Position Cal.				*/
	LONG	PhaseOffset;					/* Pole Sensor Offset 65536/360deg (Linear only)			*/
	LONG	OrgDetDelayComp;				/* Serial conversion unit origin signal detection delay correction value			*/
	LONG	MpfPhaseOffset;					/* Pole Phase Offset 65536/360deg						*/
	KSGAIN	Kphasepos;						/* Gain for Motor Phase Position Cal.					*/
//----------------------------------------------------------------------------------------------------
	LONG	ScalePitch[3];					/* Scale Pitch From Serial Converter					*/
	LONG	ScalePitchExp;					/* Scale Pitch Exponent From Serial Converter			*/
	ULONG	MonScalePitch;					/* Scale Pitch for Monitor				[pm]			*/
	LONG	MonScaleExp;					/* Scale Pitch Exponent for Monitor		[-]				*/
	ULONG	NormScalePitch;					/* Scale Pitch							[nm]			*/
	LONG	NormScaleExp;					/* Scale Pitch Exponent					[-]				*/
//----------------------------------------------------------------------------------------------------
	LONG	KConvLoadSpd;					/* Load speed conversion coefficient	[pls/ScanA] -> [2^24/OvrSpd]		*/
	//----------------------------------------------------------------------------------------------------
	USHORT	MotPrmAddress;					/* Motor Parameter Read/Write Top Address				*/
	USHORT	MotPrmSize;						/* Motor Parameter Read/Write Data Size					*/
//----------------------------------------------------------------------------------------------------
	USHORT	PosCompMode;					/* Encoder position correction mode								*//* <S004> *//*<S0C5>rename*/
	USHORT	SenSpec;						/* Bit Data for Sensor Specification					*//* <S04D> */

//----------------------------------------------------------------------------------------------------
//	Below work area
//----------------------------------------------------------------------------------------------------
	BOOL	EncReady;						/* Encoder Ready										*/
	BOOL	SenReady;						/* SEN Signal Sequence Ready							*/
	VBOOL	EncDisable;						/* Encoder Disable (Mturn Reset/Change)					*/
	BOOL	PhaseReady;						/* Motor Phase Ready (Phase Found Finish)				*/
	BOOL	EncConnect;						/* Encoder Connected									*/
	BOOL	PaoseqNormal;					/* PAO Outout Seq. FALSE = Initial Mode					*/
	LONG	PgBaudRate;						/* Encoder AB Pulse Counter for AB Phase Signal			*/
//----------------------------------------------------------------------------------------------------
	BOOL	SeRxErr;						/* Senc Receive Error									*/
	BOOL	SeRxComp;						/* Senc Receive Complete (IntSync:Set,ScanA:Clear)		*/
	BOOL	EncDataNG;						/* Encoder Data NG Flag									*/
	BOOL	AccChkEnable;					/* Encoder Acc. Check Enable Flag						*/
	BOOL	SpdCmpEnable;					/* Speed correction execution permission										*/
	BOOL	MotOrgChkEnd;					/* Motor Origin Check End Flag							*/
//----------------------------------------------------------------------------------------------------
	BOOL	SetP0ModeIntCycleReq;			/* Set Interrupt Cycle Request Flag	(P0Mode)			*//* <S1D3> */
	BOOL	SetP1ModeIntCycleReq;			/* Set Interrupt Cycle Request Flag	(P1Mode)			*/
	BOOL	ChangeP1toP0Enable;				/* Interrupt Cycle Enable Flag							*/
//----------------------------------------------------------------------------------------------------
//	BOOL	PoleZeroWrSelect;				/* PhaseOffset.OrignPosition Zero Write Request		*/
	BOOL	ReCalcAbsPosReq;				/* Abso Position Recalculate Request					*/
//----------------------------------------------------------------------------------------------------
	LONG	MstErrCnt;						/* Encoder Mst. Error Counter							*/
	LONG	ComErrCnt;						/* Encoder Com. Error Counter for Monitor				*/
	LONG	AccErrCnt;						/* Encoder Acc. Error Counter for Monitor				*/
//----------------------------------------------------------------------------------------------------
	LONG	PhaErrChkCnt;					/* Encoder Phase Error Count							*/
	LONG	PhaErrMonCnt;					/* Encoder Phase Error Count for Monitor				*/
//----------------------------------------------------------------------------------------------------
	LONG	ChkAlarmCnt;					/* Encoder Alarm Check Count							*/
	LONG	ChkBaseTimer;					/* Encoder TimeBaseTimer Check							*/
	LONG	ChkAlmCode[2];					/* Encoder Alarm Code Check								*/
	LONG	Temperature;					/* Encoder Temperature									*/
	LONG	OverHeatCnt;					/* Encoder OverHeat Check Count							*/
	BOOL	IncDetOhFlg;					/* IncEncoder OverHeat Active/NonActive Select Flag		*//* <S03A> */
	BOOL	BatWrnMode;						/* Use encoder battery voltage drop as a warning			*/
//----------------------------------------------------------------------------------------------------
	LONG	MtrSeqStep;						/* Multiturn Reset Sequence Step						*/
	ULONG	MtrSeqWait;						/* Multiturn Reset Sequence Wait Counter				*/
	LONG	MotPrmNum;						/* Motor Parameter Read/Write Data Size					*/
//----------------------------------------------------------------------------------------------------
#if 0 /*<S02D>*/
	BOOL	DivModeStatus ;					/* Division mode setting status 0:Normal 1:DCC1 2:DCC2			*/
	BOOL	DivCMaskStatus ;				/* DivisionCPhase mask status	0:Mask Off  1: Mask On			*/
	//----------------------------------------------------------------------------------------------------
	LONG	PaoSeqStep;						/* PAO Serial Output Sequence Step						*/
	LONG	PaoSeqWait;						/* PAO Serial Output Sequence Wait Counter				*/
//----------------------------------------------------------------------------------------------------
	BOOL	IncPulseReq;					/* PAO IncrePulse Output Request TRUE/FALSE				*/
	LONG	PaoSeqMcmd;						/* PAO Serial Output Sequence Cmd/Response				*/
	LONG	PaoSeqData;						/* PAO Serial Output Sequence Work Data					*/
	LONG	MturnLatch;						/* PAO Multiturn Latch Data								*/
	LONG	IncPulseOut;					/* PAO IncrePulse Output Data							*/
#endif  /*<S02D>*/
//----------------------------------------------------------------------------------------------------
	LONG	PsoSeqStep;						/* PSO Serial Output Sequence Step						*/
	LONG	PsoSeqWait;						/* PSO Serial Output Sequence Wait Counter				*/
	LONG	PsoMtnData;						/* PSO Serial Output Multiturn Data						*/
	LONG	PsoIncPulse;					/* PSO Serial Output Incremental Pulse					*/
//----------------------------------------------------------------------------------------------------
#if (FLOAT_USE==TRUE)
	float	MotPos;							/* Motor Position		  [32bitRingPulse]				*/
	LONG	MotAbsPos;						/* Motor Absolute Position[32bitRingPulse]				*/
	LONG	MotPosX[3];						/* Motor Encoder Position(Position after correction) [DependOnEncoder]	*/
	LONG	RcvPosX[3];						/* Motor Encoder Position(Receive position)	  [DependOnEncoder]	*/
#else
	LONG	MotPos;							/* Motor Position		  [32bitRingPulse]				*/
	LONG	MotAbsPos;						/* Motor Absolute Position[32bitRingPulse]				*/
	LONG	MotPosX[3];						/* Motor Encoder Position(Position after correction) [DependOnEncoder]	*/
	LONG	RcvPosX[3];						/* Motor Encoder Position(Receive position)	  [DependOnEncoder]	*/
#endif /* FLOAT_USE */
//----------------------------------------------------------------------------------------------------
	KSGAIN	MotSpd;							/* Motor Speed			  [2^24/OvrSpd]					*//*<S00A>*/
//----------------------------------------------------------------------------------------------------
	LONG	MotPhaX;						/* Motor Phase   Counter  [DependOnEncoder]				*/
	LONG	MotOrgX;						/* Motor Origin  Position [DependOnEncoder]				*/
	LONG	MotOrgXchk[2];					/* Motor Origin  PosCheck [DependOnEncoder]				*/
//----------------------------------------------------------------------------------------------------
	USHORT	MotPhase;						/* Motor Pole Phase for Monitor							*/
	USHORT	MotPhaseX;						/* Motor Pole Phase for Micro-Program					*/
	USHORT	MotPhaseC;						/* Motor Pole Phase for Trq Ripple Comp					*/
//----------------------------------------------------------------------------------------------------
	BOOL	Cset;							/* Cphase Set											*/
	BOOL	Cpass;							/* Cphase Pass											*/
#if 0 /*<S02D>*/
	BOOL	DivCpass;						/* DivCphase Pass										*/
#endif  /*<S02D>*/
	BOOL	OrgOk;							/* Motor Origin OK										*/
	BOOL	CposOk;							/* Cphase Position Ok Flag								*/
	BOOL	IncreCposLatch;					/* Incre Cphase Position Latch Signal (ScanA --> ScanB)	*/
	BOOL	MotCphPass;						/* Motor Cphase Pass Signal fro ScanControl				*/
	LONG	MotCposX;						/* Motor C-Phase Position  [DependOnEncoder]			*/
	LONG	MposFromCpos;					/* Motor Pos. from C-Phase [+-(PulseNo/2)],[LM:+-31bit]	*/
	LONG	CphaseDist;						/* Motor Pos. from C-Phase make Micro Program			*/
//----------------------------------------------------------------------------------------------------
	LONG	MotAngle;						/* Motor RM-Cphase/LM-Pole Angle for Monitor			*/
#if 0 /*<S02D>*/
	LONG	DivCpassPos;					/* Cphase Pass Div.Position								*/
//----------------------------------------------------------------------------------------------------
	LONG	DivOutPos;						/* Frequency division output pulse										*/
	LONG	DivOutRem;						/* Division output pulse remainder									*/
	LONG	InitDivPosOffset;				/* Division output pulse offset								*/
//----------------------------------------------------------------------------------------------------
#endif  /*<S02D>*/
	LONG	IniMturn;						/* Initial Multiturn Data								*/
	LONG	IniIncre;						/* Initial Incremental Pulse							*/
//----------------------------------------------------------------------------------------------------
//	APOSRG	IniAbsPos;						/* Initial Absolute Position [CmdPulseUnit]				*/
//----------------------------------------------------------------------------------------------------
	ULONG	SenOnMtnData;					/* SEN ON								*/
	ULONG	SenOnIncPulse;					/* SEN ONtime1Internal rotation pulse data						*/
//----------------------------------------------------------------------------------------------------
	LONG	SenOnScalePosL;					/* SEN ON(Lower32Bit)					*/
	LONG	SenOnScalePosH;					/* SEN ON(Upper level32Bit)					*/
//----------------------------------------------------------------------------------------------------
	LONG	PoleWrSeqStep;					/* PhaseOffset.OrignPosition Write	Sequence Step		*/
//	LONG	PoleWrState;					/* PhaseOffset.OrignPosition Wirte Status				*/
	ULONG	PoleWrSeqWait;					/* PhaseOffset.OrignPosition Write Wait Timer			*/
	LONG	PoleWrSum;						/* PhaseOffset.OrignPosition Write CheckSum Value		*/
//----------------------------------------------------------------------------------------------------
//	ULONG	ScanBPosL;						/* Latch Encoder Low  Data for ScanB					*/
//	LONG	ScanBPosH;						/* Latch Encoder High Data for ScanB					*/
//----------------------------------------------------------------------------------------------------
	USHORT	RxAlmCode;						/* Encoder Alarm Code									*/
	SHORT	RxPosH[2];						/* Encoder Position Data High							*/
	DWORDX	RxTimex;						/* Encoder Timex										*/
	DWORDX	RxPosL[2];						/* Encoder Position Data Low							*/
	USHORT	BaseTimer;						/* Basetimer value for Speed Comp. Module				*//* <S067> */
	USHORT	HenkaItiTimer;					/* Henkaititimer value for Speed Comp. Module			*//* <S067> */
//----------------------------------------------------------------------------------------------------
	UCHAR	ABphCntr;						/* Encoder AB Pulse Counter for AB Phase Signal			*/
	UCHAR	MotOrgChkCnt;					/* Motor Origin Check Counter							*/
	UCHAR	MotPoleInfo[2];					/* Motor Pole UVW Signal Information 					*/
//----------------------------------------------------------------------------------------------------
	USHORT	AccChkWaitCnt;					/* Acceleration check start count							*/
//----------------------------------------------------------------------------------------------------
	LONG	EncLchNum;						/* Encoder position sampling period		(SVN_IRIS_MODE)	*/

	ENCVFBTIMBASE	FbTimeBase;				/* Base timer										*/
//----------------------------------------------------------------------------------------------------
	DIVINFO DivInfo;						/* Division output related information										*//*<S02D>*/
	//----------------------------------------------------------------------------------------------------
	USHORT	FnMturnClrCmdSndNum;			/* Multiturn reset(Fn008) Number of command transmissions			*/	/* <S0BB> */
	LONG	EncBitNo;						/* Encoder bit number									*/	/* <S14B> */
	ULONG	EncComErrCnt;					/* Un102Save value for reset								*/
//----------------------------------------------------------------------------------------------------
	BOOL	PdetReq587;						/* Pn587.0:Magnetic pole detection request						 <S21C> 	*/

} MENCV;



#define ENC_DATA_H
#endif /* ENC_DATA_H */
/***************************************** end of file **********************************************/

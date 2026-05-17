/*****************************************************************************
Description	: Alarm Manager
-----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

-----------------------------------------------------------------------------
Changes		:
Name   Date          Description
-----------------------------------------------------------------------------
I.M.   2010.11.2     created

*****************************************************************************/
#ifndef ALARM_H
#define ALARM_H

/*--------------------------------------------------------------------------*/
/*		Alarm attribute definition													*/
/*--------------------------------------------------------------------------*/
#define ALMDEF_ATTR_WARNING		0x01		/* Warning(1),Alarm(0)	*/
#define ALMDEF_ATTR_NOEEPROM	0x02		/* EEPROMNon-storage(1),EEPROMStorage(0)*/
#define ALMDEF_ATTR_COMALARM	0x04		/* CommunicationAlarm(1),AmplifierAlarm(0)	*/
#define ALMDEF_ATTR_NOTRESET	0x08		/* Reset not possible(1),Reset possible(0)*/
#define ALMDEF_ATTR_MENCDANG	0x10		/* Invalid motor encoder data(1)*/
#define ALMDEF_ATTR_POSNG		0x20		/* Position data invalid(1)			*/
/*--------------------------------------------------------------------------*/
/*		OL alarm information definition												*/
/*--------------------------------------------------------------------------*/
#define ALMDEF_OLINFO_OLF		0x01		/* Overload						*/
#define ALMDEF_OLINFO_RGOL		0x02		/* Regeneration overload					*/
#define ALMDEF_OLINFO_DBOL		0x04		/* DB overload					*/
/*--------------------------------------------------------------------------*/
/*		Alarm extended attribute definition												*/
/*--------------------------------------------------------------------------*/
#define ALMDEF_M3ATTR_CMDALM	0x01		/* M3Command alarm(1)		*/
#define ALMDEF_M3ATTR_COMALM	0x02		/* M3Communication alarm(1)			*/
#define ALMDEF_EXATTR_INIALM	0x04		/* Startup detection alarm(1)		*/
#define ALMDEF_M3ATTR_CODEMASK	0xF0		/* M3Alarm code mask		*/


/*****************************************************************************
*	AlarmIDDefinition
*****************************************************************************/
typedef enum ALMID_T {
	NO_ALARM		= 0,		/* No Alarm */

	/* Servo alarm ID definition */
	ALM_EEPROM1,				/* A.020 : User constant checksum error							*/
	ALM_EEPROM2,				/* A.021 : Parameter format error							*/
	ALM_EEPROM3,				/* A.022 : System checksum error								*/
	ALM_EEPROM4,				/* A.023 : Parameter password error								*/
	ALM_BOOT1,					/* A.024 : FlashError at first startup(Alarm for first factory test machine)		*//* Encrypted format support <S146> */
	ALM_BOOT2,					/* A.025 : Runtime farm part error(Alarm when mirror starts)		*//* Encrypted format support <S146> */
	ALM_VDC,					/* A.030 : Main circuit detection part abnormality										*/
	ALM_PRM,					/* A.040 : Parameter setting error									*/
	ALM_DIVPRM,					/* A.041 : Division pulse output setting error								*/
	ALM_PRMUNMATCH,				/* A.042 : Parameter combination error								*/
	ALM_SEMIFULLPRM,			/* A.044 : Semi-closed parameter setting error						*/
	ALM_UNMATCH,				/* A.050 : Motor/Servo capacity unmatch							*/
	ALM_NOTSUPPORT,				/* A.051 : Product not supported alarm								*/
	ALM_PWMAMP_SLOT,			/* A.052 : PWMAmplifier connection alarm								*/
	ALM_MODULE_CHANGE,			/* A.053 : Module configuration change alarm							*/
	ALM_SCALE,					/* A.080 : Scale pitch setting error								*/
	ALM_SVONCMD,				/* A.0B0 : Servo on command invalid alarm							*/
	ALM_OC,						/* A.100 : Overcurrent(OC)											*/
	ALM_OVERCURRENT,			/* A.101 : Motor overcurrent detection										*//* <S1F5> */
	ALM_RLYON_ACK,				/* A.230 : RLYONAnswer error									*/
	ALM_RG,						/* A.300 : Regenerative abnormality												*/
	ALM_RGOL,					/* A.320 : Regeneration overload											*/
	ALM_WIR,					/* A.330 : Main circuit wiring error										*/
	/* <S21D> Start */
	ALM_CONVIO,					/* A.331 : Power related input signal error									*/
	ALM_CAPALINK,				/* A.332 : Capacitor bank connection answer error						*/
	ALM_CAPAMC,					/* A.333 : Capacitor bankMCAnswer error						*/
	/* <S21D> End */
	ALM_OV,						/* A.400 : Overvoltage(OV)											*/
	ALM_PUV,					/* A.410 : Undervoltage												*/
	ALM_CONVFLT,				/* A.42A : Converter abnormality for IDM								*/
	ALM_PCOV,					/* A.450 : Main circuit capacitor overvoltage								*/
	ALM_OSF1,					/* A.510 : Overspeed(OS)											*/
	ALM_OSDIV,					/* A.511 : Division pulse output overspeed									*/
	ALM_VIB,					/* A.520 : Vibration alarm(Oscillation alarm)							*/
	ALM_AT,						/* A.521 : Auto tuning alarm							*/
	ALM_VELPRM,					/* A.550 : Maximum speed setting error										*/
	ALM_OLF1,					/* A.710 : Overload(Instantaneous maximum load)									*/
	ALM_OLF2,					/* A.720 : Overload(Continuous maximum load)									*/
	ALM_DBOL,					/* A.730 : DB overload											*/
	ALM_DBOL_LVL,				/* A.731 : DB overload											*/
	ALM_RSOL,					/* A.740 : Inrush resistance overload										*/
	ALM_OH,						/* A.7A0 : Heat sink overheating										*/
	ALM_CTRLBRD,				/* A.7A1 : Internal temperature abnormality1(Control board temperature abnormality)					*//* <S0DE> */
	ALM_PWRBRD,					/* A.7A2 : Internal temperature abnormality2(Power board temperature abnormality)					*//* <S0DE> */
	ALM_INTTEMP,				/* A.7A3 : Internal temperature detection section error									*//* <S0DE> */
	ALM_CTRLBDOH,				/* A.7AA : Control board temperature abnormality for IDM								*/
	ALM_FANLOCK,				/* A.7AB : Stopping the SERVOPACK built-in fan							*/
	ALM_PG1,					/* A.810 : Encoder backup alarm						*/
	ALM_PG2,					/* A.820 : Encoder sum check alarm						*/
	ALM_PG3,					/* A.830 : Encoder battery alarm							*/
	ALM_PG4,					/* A.840 : Encoder data alarm								*/
	ALM_PG5,					/* A.850 : Encoder overspeed								*/
	ALM_PG6,					/* A.860 : Encoder overheat								*/
	ALM_SCL_ERR,				/* A.890 : Encoder scale error								*/
	ALM_MOD_ERR,				/* A.891 : Encoder module error							*/
	ALM_FPGFSCALE,				/* A.8A0 : Full close serial encoder scale error			*/
	ALM_FPGMODULE,				/* A.8A1 : Full-closed serial encoder module error			*/
	ALM_FPGSEN,					/* A.8A2 : Full-closed serial encoder sensor error(Incremental)	*/
	ALM_FPGPOS,					/* A.8A3 : FPG position error(Absolute)							*/
	ALM_FPGOVSPD,				/* A.8A5 : Full close serial encoder overspeed error		*/
	ALM_FPGOVHEAT,				/* A.8A6 : Full close serial encoder overheat error		*/
	ALM_VREFADERR,				/* A.B10 : Speed command A/D error									*/
	ALM_VREFADCHK,				/* A.B11 : Abnormal speed command A/D data								*/
	ALM_TREFADERR,				/* A.B20 : Torque command A/D error									*/
	ALM_CURAD_U,				/* A.B31 : Current detection error 1(U phase)									*/
	ALM_CURAD_V,				/* A.B32 : Current detection error 2(V phase)									*/
	ALM_JL056_GUP1,				/* A.B33 : Current detection error 3(Current detector)							*/
	ALM_HWBB_UNSUPPORT,			/* A.B40 : Gate drive error									*/
	ALM_SAMP_OVER,				/* A.BF0 : System alarm 0(Internal program processing error)			*/
	ALM_STACK_OVF,				/* A.BF1 : System alarm 1(Internal program operation error)			*/
	ALM_UPROG,					/* A.BF2 : System alarm 2(Current control processing unit program processing error)	*/
	ALM_SCANA_STOP,				/* A.BF3 : System alarm 3(EncoderI/FProcessing unit error)			*/
	ALM_JL056_GUP,				/* A.BF4 : System alarm 4(CPU WDTAbnormal)						*/
	ALM_RWF,					/* A.C10 : Runaway prevention detection											*/
	ALM_PHASE,					/* A.C20 : Phase error detection											*/
	ALM_POLESENSOR,				/* A.C21 : Pole sensor error										*/
	ALM_PHASEANGLE,				/* A.C22 : Phase information mismatch										*/
	ALM_POLE,					/* A.C50 : Magnetic pole detection failure											*/
	ALM_PFINDSTOP,				/* A.C51 : Overtravel detection when detecting a magnetic pole.						*/
	ALM_PDET,					/* A.C52 : Magnetic pole detection not completed											*/
	ALM_PDET_MOVEOVER,			/* A.C53 : Magnetic pole detection movable range exceeded									*/
	ALM_PDET_NG,				/* A.C54 : Magnetic pole detection failure2										*/
	ALM_PGCLR,					/* A.C80 : Encoder clear error(Multiturn limit setting error)	*/
	ALM_PGCOM,					/* A.C90 : Encoder communication error									*/
	ALM_PGCOM1,					/* A.C91 : Encoder communication position data acceleration error					*/
	ALM_PGCOM2,					/* A.C92 : Encoder communication timer error								*/
	ALM_PGP,					/* A.CA0 : Encoder parameter error								*/
	ALM_PGECHO,					/* A.CB0 : Encoder echo back error							*/
	ALM_MTLMT,					/* A.CC0 : Multiturn limit value mismatch							*/
	ALM_FPGCOM,					/* A.CF1 : Feedback option card communication error(Receive failure)		*/
	ALM_FPGCOMTIMER,			/* A.CF2 : Feedback option card communication error(Timer stop)	*/
	ALM_OF,						/* A.D00 : Excessive position deviation											*/
	ALM_BEOF,					/* A.D01 : Excessive position deviation when servo is on								*/
	ALM_BEVLMTOF,				/* A.D02 : Excessive speed limit position deviation when servo is on						*/
	ALM_FSOF,					/* A.D10 : Excessive deviation between motor and load position							*/
	ALM_POSOF,					/* A.D30 : Position data over									*/
	ALM_OPSET,					/* A.E00 : Option synchronization access request reception timeout			*/
	ALM_TIMEOUT,				/* A.E01 : Option operation start request reception timeout				*/
	ALM_WDC_ERR,				/* A.E02 : WDC+SyncFlagSynchronization error								*/
	ALM_OPDETECT,				/* A.E07 : Option board detection error								*/
	ALM_DGOP_TOUT,				/* A.E08 : DPRAM digital op communication timeout DigOpCom			*/
	ALM_SC_TOUT,				/* A.E09 : DPRAM S->C Communication timeout								*/
	ALM_CMDOP_DET,				/* A.E70 : Command option card detection failure								*/
	ALM_SFTOP_DET,				/* A.E71 : Safety option card detection failure								*/
	ALM_FBKOP_DET,				/* A.E72 : Feedback option card detection failure							*/
	ALM_CMDOP_NSUP,				/* A.E73 : Command option card not supported								*/
	ALM_SFTOP_NSUP,				/* A.E74 : Safety option card not supported								*/
	ALM_FBKOP_NSUP,				/* A.E75 : Feedback option card not supported							*/
	ALM_CMDOP_UNMATCH,			/* A.E80 : Command option card detection mismatch								*/
	ALM_SMC_ERR,				/* A.EB0 : Safety functionDrvMonitoring circuit error								*/
	ALM_BBON_ERR0,				/* A.EB1 : Input signal error for safety function								*/
	ALM_BBON_ERR1,				/* A.EB2 : Safety functionDrvInternal signal abnormality								*/
	ALM_SV_COMERR1,				/* A.EB3 : Safety functionDrvCommunication error 1								*/
	ALM_SV_COMERR2,				/* A.EB4 : Safety functionDrvCommunication error 2								*/
	ALM_SV_COMERR3,				/* A.EB5 : Safety functionDrvCommunication error 3								*/
	ALM_SV_COMDATA,				/* A.EB6 : Safety functionDrvCommunication data error							*/
	ALM_OP_SYSERR1,				/* A.EC0 : Safety option card system error 1							*/
	ALM_OP_SYSERR2,				/* A.EC1 : Safety option card system error 2							*/
	ALM_OP_PRMERR,				/* A.EC2 : Safety option card parameter setting error						*/
	ALM_OP_MOTTYP,				/* A.EC3 : Safety option card mode type mismatch							*/
	ALM_OP_COMERR1,				/* A.EC4 : Safety option card communication error 1							*/
	ALM_OP_COMERR2,				/* A.EC5 : Safety option card communication error 2							*/
	ALM_OP_COMERR3,				/* A.EC6 : Safety option card communication error 3							*/
	ALM_OP_OVERRUN,				/* A.EC7 : Safety option card stop command error							*/
	ALM_DPIN1_ERR,				/* A.EC8 : Safety functionDrvInternal signal abnormality 1					<S0F1>	*/
	ALM_DPIN2_ERR,				/* A.EC9 : Safety functionDrvInternal signal abnormality 2					<S0F1>	*/
	ALM_PWPHASE,				/* A.F10 : Power supply phase loss												*/

//	ALM_MBOH,					/* A.7A1 : Unit internal temperature overheating									*/
//	ALM_CONVOH,					/* A.7A2 : Converter temperature overheat									*/
//	ALM_IFOH,					/* A.7A3 : Control board temperature overheating										*/

	ALM_WDC_BIT_ERR,			/* A.E0A : WDCSynchronization error											*/
	ALM_PRM_TRANSLATE,			/* A.BF5 : System alarm 5(Parameter batch transfer error)			*//* <S066> */
	ALM_I2C_DRVERR,				/* A.BF6 : System alarm 6(EEPROMDriver error)				*//* <S098> */

	/*	MECHATROLINKAlarm ID definition	*/
	ALM_OPTPRM_ERR,				/* A.04A : Option parameter setting error							*/
	ALM_TRACE_SPD_OF,			/* A.51A : Following speed excessive alarm									*/
	ALM_COMLSI_ERR1,			/* A.B6A : CommunicationASICSetting error										*/
	ALM_COMLSI_ERR2,			/* A.B6B : CommunicationASICSystem error									*/
	ALM_SCANN_STOP,				/* A.BFB : ScanNStop execution										*/
	ALM_TCYC_ERR,				/* A.E40 : MECHATROLINK Transmission cycle setting error						*/
	ALM_CD_LEN_ERR,				/* A.E41 : MECHATROLINKCommunication data size setting error					*/
	ALM_AXIS_NO_ERR,			/* A.E42 : MECHATROLINKStation address setting error						*/
	ALM_COMSYNC,				/* A.E50 : Network synchronization error								*/
	ALM_COMSYNC_TOUT,			/* A.E51 : Network synchronization establishment failure								*/
	ALM_COM,					/* A.E60 : Network reception error								*/
	ALM_COM_INT_ERR,			/* A.E61 : Network synchronization interval error								*/
	ALM_COM_FCS,				/* A.E62 : NetworkFCSError								*/
	ALM_COM_GBL,				/* A.E63 : Global data reception error							*/
	ALM_MON_SLV_COM,			/* A.E64 : Other station monitor data reception error							*/
	ALM_COM_RCV_ASYNC,			/* A.E6A : Network reception error (event-driven communication)		*/
	ALM_COM_TOUT_ASYNC,			/* A.E6B : Network reception timeout (event-driven communication)	*/
	ALM_CON_SEND_ASYNC,			/* A.E6C : Network transmission error (event-driven communication)			*/
	ALM_NOANS,					/* A.EA0 : Servopack failure										*/
	ALM_SVINIT_ERR,				/* A.EA1 : Servopack initial access error							*/
	ALM_SVWDC_ERR,				/* A.EA2 : ServopackWDCAbnormal									*/
	ALM_SVANS_TOUT,				/* A.ED0 : Servopack response timeout							*/
	ALM_CMDEXEC,				/* A.ED1 : Command execution incomplete										*/

	/* Servo warning ID definition */
	WRN_OF,						/* A.900 : Excessive position deviation											*/
	WRN_BEOF,					/* A.901 : Excessive position deviation when servo is on								*/
	WRN_OLF,					/* A.910 : Overload												*/
	WRN_VIB,					/* A.911 : Vibration(Oscillation)											*/
	WRN_CTRLBRD,				/* A.912 : Internal temperature warning1(Control board temperature warning)					*//* <S0DE> */
	WRN_PWRBRD,					/* A.913 : Internal temperature warning2(Power board temperature warning)					*//* <S0DE> */
	WRN_RGOLF,					/* A.920 : Regeneration overload											*/
	WRN_DBOL_LVL,				/* A.921 : DB overload											*/
	WRN_FANLOCK,				/* A.923 : Stopping the SERVOPACK built-in fan							*//* <S0A3> */
	WRN_EXREG,					/* A.924 : Regeneration warning											*//* <S21D> */
	WRN_PGBAT,					/* A.930 : Absolute value encoder battery error							*/
	WRN_FPGBAT,					/* A.931 : FPG battery error									*/
	WRN_DATA,					/* A.940 : Data abnormality											*/
	WRN_BOOT,					/* A.941 : Power cycle required to change user constants							*/
	WRN_RIPPLE,					/* A.942 : Ripple correction information mismatch								*/
	WRN_CMD,					/* A.950 : Command error											*/
	WRN_UV,						/* A.971 : Undervoltage												*/
	WRN_OPT,					/* A.9F0 : Option error										*/

	WRN_MBOH,					/* A.981 : Unit internal temperature overheating									*/
	WRN_CONVOH,					/* A.982 : Converter temperature overheat									*/
	WRN_IFOH,					/* A.983 : Control board temperature overheating										*/
	WRN_OT,						/* A.9A0 : OTSignal detection warning									*/

	/*	MECHATROLINKWarning ID definition */
	WRN_PRM_NO,					/* A.94A : Data setting warning(User constant number)						*/
	WRN_DATA_AREA,				/* A.94B : Data setting warning(Out of data range)							*/
	WRN_DATA_CALC,				/* A.94C : Data setting warning(Operation error)								*/
	WRN_DATA_SIZE,				/* A.94D : Data setting warning(Data size error)						*/
	WRN_DATA_LTMOD,				/* A.94E : Data setting warning(Latch mode error)						*/
	WRN_CMD_CNDTN,				/* A.95A : Command warning(Out of condition)									*/
	WRN_CMD_NOSPT,				/* A.95B : Command warning(Unsupported)								*/
	WRN_CMD_PRM,				/* A.95C : Command warning(Outside of conditions based on user constants)					*/
	WRN_CMD_CNFLCT,				/* A.95D : Command warning(Command interference) 						*/
	WRN_SUBCMD,					/* A.95E : Command warning(Subcommand not possible) 						*/
	WRN_FUNC_CNFLCT,			/* A.95F : Command warning(Port interference) 							*/
	WRN_COM,					/* A.960 : MECHATROLINKCommunication warning									*/
	WRN_COM_FCS,				/* A.962 : FCSAbnormality warning											*/
	WRN_COM_GBL,				/* A.963 : Global data communication warning								*/
	WRN_SLV_COM,				/* A.964 : Other station monitor data communication warning								*/
	WRN_COM_RCV_ASYNC,			/* A.96A : Network reception warning(Event-driven communication)			*/
	WRN_COM_PHASE,				/* A.97A : Phase error											*/
	WRN_DATA_CLAMP,				/* A.97B : Data setting warning(Data clamp operation)					*/
	WRN_ACT_COP_STP,			/* A.97C : Operation warning (cooperative stop operation)								*/
	WRN_PREVENTMAINT,			/* A.9B0:Preventive maintenance warning										*/ /* <S154> */

} ALMID_T;


/* Alarm definition structure */
typedef struct ALM_TBL_DEF {
	USHORT	AlmCode;				/* Alarm code						*/
	UCHAR	AlmAttr;				/* Alarm attribute							*/
	UCHAR	StopMode;				/* Stopping method when an alarm occurs,OL attribute	*/
	UCHAR	AlmCodeOut;				/* Alarm code output signal				*/
	UCHAR	AlmLedOut;				/* Alarm LED output signal				*/
	UCHAR	AlmAttrEx;				/* Alarm extended attributes						*/
	UCHAR	Spare;					/* Spare								*/
} ALM_TBL_DEF;
typedef const ALM_TBL_DEF*	CPALM_TBL_DEF;


/* Glogals */
extern const ALM_TBL_DEF	servo_alm_tbl[];	/* Alarm table			*/
extern const LONG			AlmDefTblEntNum;	/* Number of alarm table elements	*/


#endif /* ALARM_H */

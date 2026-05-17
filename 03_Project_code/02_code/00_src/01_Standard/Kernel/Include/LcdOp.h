/****************************************************************************************************/
/*																									*/
/*																									*/
/*		LcdOp.h : LCD Operator Common Header File													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1. LcdOperator IF definition																	*/
/*																									*/
/*		2. Panel operator IF definition																	*/
/*																									*/
/*		3. 																							*/
/*																									*/
/*		4. 																							*/
/*																									*/
/*		5. 																							*/
/*																									*/
/*		6. 																							*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2013.04.15  K.Ozaki																*/
/*								LcdOp.h <= RndApi.h + RndRpx.h +LcdOp.c								*/
/*		Rev.1.10 : 2013.9.26  K.Ozaki	<S00C>														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#ifndef ___LCDOP___
#define ___LCDOP___

#include "Object.h"

#define ADJUST_STRUCT_ALIGN( x ) 
#define CCSW_HWID  (0x55AA) /* Dummy */
#define CCSW_CMDID  (0xAA55) /* Dummy */


/****************************************************************************************************/
/*																									*/
/*		Operator IF macro definition																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		SGDVCompatibility ensuring macro(Under review)																*/
/*--------------------------------------------------------------------------------------------------*/
#define KpiGetAlarmInfo() ALMGetAlarmInfo( pAxRsc->AlmManager ) /* TODO: Interim */
#define KriResetSystemAlarm() ALMSetAlarmResetRequest( pAxRsc->AlmManager ) /* TODO: Interim */

#if 0 															/* SGDV and Jupiter					*/
/*--------------------------------------------------------------------------------------------------*/
/*	Status Display Check Macro																		*/
/*--------------------------------------------------------------------------------------------------*/
#define CHK_BB()       ( ! RinV.f.BaseEnable )
#define CHK_PDET()     ( ! MencV.f.PhaseReady )
#define CHK_TUNING()   ( RoutV.f.TuningRun )
#define CHK_JOGMODE()  ( BoutV.CtrlMode.b.cm == CTRLMODE_JOG )
#define CHK_ZRETMODE() ( BoutV.CtrlMode.b.cm == CTRLMODE_ZSRCH )
#define CHK_POT()      ( CoutV.f.Sv.Pot )
#define CHK_NOT()      ( CoutV.f.Sv.Not )
#define CHK_PLS()      ( CoutV.f.Sv.Pls )
#define CHK_NLS()      ( CoutV.f.Sv.Nls )
#define CHK_ESTOP()    ( CoutV.f.Sv.EmgStop )
#define CHK_HBB()      ( CoutV.f.Sv.HwbbState )

/*--------------------------------------------------------------------------------------------------*/
/*	Status LED Check Macro																			*/
/*--------------------------------------------------------------------------------------------------*/
#define CHK_SVON()     ( ! CHK_BB() )
#define CHK_TGON()     ( CoutV.f.Ctrl.MotorMoving )
#define CHK_CHG()      ( CoutV.f.Seq.MainPowerOn )
#define CHK_COIN()     ( BoutV.f.Ctrl.Coin ) 
#define CHK_VCMP()     ( CoutV.f.Ctrl.ReachSpd )
#define CHK_TRQMODE()  ( BoutV.CtrlMode.b.cm == CTRLMODE_TRQ )
#define CHK_PREFON()   ( CoutV.f.Sv.Prefon )
#define CHK_VREFON()   ( CoutV.f.Sv.Vrefon )
#define CHK_TREFON()   ( CoutV.f.Sv.Trefon )

#elif CCSW_HWID == HWID_JPCV										/* Jupiter Converter 			*/

/*--------------------------------------------------------------------------------------------------*/
/*	Status Display Check Macro																		*/
/*--------------------------------------------------------------------------------------------------*/
#define CHK_BB()       ( ! RinV.f.BaseEnable )
#define CHK_PDET()     ( 0 ) 
#define CHK_TUNING()   ( 0 )
#define CHK_JOGMODE()  ( 0 )
#define CHK_ZRETMODE() ( 0 )
#define CHK_POT()      ( 0 )
#define CHK_NOT()      ( 0 )
#define CHK_PLS()      ( 0 )
#define CHK_NLS()      ( 0 )
#define CHK_ESTOP()    ( 0 )
#define CHK_HBB()      ( 0 )

/*--------------------------------------------------------------------------------------------------*/
/*	Status LED Check Macro																			*/
/*--------------------------------------------------------------------------------------------------*/
#define CHK_SVON()     ( ! CHK_BB() )
#define CHK_TGON()     ( RinV.f.DrvAxRunning )
#define CHK_CHG()      ( RinV.f.DcPowerOn )
#define CHK_COIN()     ( 0 ) 
#define CHK_VCMP()     ( 0 )
#define CHK_TRQMODE()  ( 0 )
#define CHK_PREFON()   ( RinV.f.DrvReady )
#define CHK_VREFON()   ( 0 )
#define CHK_TREFON()   ( 0 )

#else 																/* Mercury ???	 				*/

/*--------------------------------------------------------------------------------------------------*/
/*	Status Display Check Macro																		*/
/*--------------------------------------------------------------------------------------------------*/
#define CHK_BB()       (!(pAxRsc->SeqCtrlOut->BeComplete))
#define CHK_PDET()     (!(pAxRsc->MencV->PhaseReady))
#define CHK_TUNING()   ( pAxRsc->FnCmnCtrl->State.TuningRun )
#define CHK_JOGMODE()  ( pAxRsc->BaseCtrlOut->CtrlModeOut.b.cm == CTRLMODE_JOG )
#define CHK_ZRETMODE() ( pAxRsc->BaseCtrlOut->CtrlModeOut.b.cm == CTRLMODE_ZSRCH )
#define CHK_POT()      ( pAxRsc->SeqCtrlOut->PotSigSts | (pAxRsc->BaseCtrlOut->CmdSeqBit & POSITIVE_OT_BIT))
#define CHK_NOT()      ( pAxRsc->SeqCtrlOut->NotSigSts | (pAxRsc->BaseCtrlOut->CmdSeqBit & NEGATIVE_OT_BIT))
#define CHK_PLS()      ( 0/* TODO */ )
#define CHK_NLS()      ( 0/* TODO */ )
#define CHK_ESTOP()    ( 0/* TODO */ )
#define CHK_HBB()      ( pAxRsc->SeqCtrlOut->HwbbSts )
#define CHK_ALARM()	   ( pAxRsc->AlmManager->Status.AlmFlag )	/* <S0FF> */
#define CHK_FSTP()	   (pAxRsc->SeqCtrlOut->FstpSts )												  /* <S1F5> */
#define CHK_MOTOR_LESS_TEST()	( pAxRsc->MotorLessTest->MotorLessTestMode == TRUE )				  /* <S1B7> */

/*--------------------------------------------------------------------------------------------------*/
/*	Status LED Check Macro																			*/
/*--------------------------------------------------------------------------------------------------*/
#define CHK_SVON()     ( ! CHK_BB() )
#define CHK_TGON()     ( pAxRsc->SeqCtrlOut->TgOnSts )
#define CHK_CHG()      ( pAxRsc->PowerManager->PowerSts.PowerOn )
#define CHK_COIN()     ( pAxRsc->BaseControls->PosCtrlSts.CoinSignal )
#define CHK_VCMP()     ( pAxRsc->SeqCtrlOut->VcmpSts )
#define CHK_TRQMODE()  ( pAxRsc->BaseCtrlOut->CtrlModeOut.b.cm == CTRLMODE_TRQ ) 
#define CHK_PREFON()   ( pAxRsc->SeqCtrlOut->f.Prefon ) 
#define CHK_VREFON()   ( pAxRsc->SeqCtrlOut->f.Vrefon ) 
#define CHK_TREFON()   ( pAxRsc->SeqCtrlOut->f.Trefon ) 
#define CHK_PERRCLR()  ( pAxRsc->SeqCtrlOut->f.PerrClr )

#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Display base																					*/
/*--------------------------------------------------------------------------------------------------*/
#define DSPDEF_BASENBL			(OBJDEF_BASENBL)			/* Attribute: Nibble display						*/
#define DSPDEF_BASEDEC			(OBJDEF_BASEDEC)			/* Attribute:10Hex display						*/
#define DSPDEF_BASEHEX			(OBJDEF_BASEHEX)			/* Attribute:16Hex display						*/
#define DSPDEF_BASEBIT			(OBJDEF_BASEBIT)			/* Attribute: Bit display						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Signed or not																				*/
/*--------------------------------------------------------------------------------------------------*/
#define DSPDEF_W_SIGN			(TRUE)			/* Signed											*/
#define DSPDEF_NOSIGN			(FALSE)			/* Unsigned											*/

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator PRM/MON Display IDs															*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCDPMDSP_MAX		4					/* Number of screens											*/
#define	LCDPMDSP_PM			0					/* PRM/MONScreen										*/
#define	LCDPMDSP_ZSRCH		1					/* Origin search screen									*/
#define	LCDPMDSP_JOG		2					/* JOGScreen											*/
#define	LCDPMDSP_PJOG		3					/* PROGRAM JOGScreen									*/

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Drive Selection 																*/
/*--------------------------------------------------------------------------------------------------*/
#define DRVSEL_MAXUNIT 1
#define DRVSEL_MAXAXISPERUNIT 3

/*--------------------------------------------------------------------------------------------------*/
/*		Drive Selection Type Definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define DRVSEL_TYPE_DRVAXIS		0			/* Drive Axis											*/
#define DRVSEL_TYPE_COMMCPU		1			/* Communication CPU									*/
#define DRVSEL_TYPE_CONVERTER	2			/* Converter for Converter Operator						*/

/****************************************************************************************************/
/*																									*/
/*		Sigma Message for LCD operator Message definitions (Unit: Byte)								*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Sigma Message for LCD operator Message definitions (Unit: Byte)								*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDMSG_AXSIZ		1			/* Axis address size											*/
#define LCDMSG_FUNSIZ		1			/* Command size											*/
#define LCDMSG_LENSIZ		2			/* Message length size										*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDMSGR_LEDSIZ		2			/* LEDStatus size										*/
#define LCDMSGR_DSPSIZ		16			/* Display status size										*/
#define LCDMSGR_CHRSIZ		(5*18)		/* Display character size											*/
#define LCDMSGR_SUMSIZ		2			/* Checksum size										*/
#define LCDMSGR_FONSIZ		(5*16)		/* Font data size										*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCDMSG_HDRSIZ		(LCDMSG_AXSIZ  + LCDMSG_FUNSIZ  + LCDMSG_LENSIZ)
#define	LCDMSGR_BDYSIZ		(LCDMSGR_LEDSIZ + LCDMSGR_DSPSIZ + LCDMSGR_CHRSIZ)
/*--------------------------------------------------------------------------------------------------*/
/*		0x64/0x60Response message length																	*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDMSGR_LEN60		(LCDMSG_HDRSIZ + LCDMSGR_BDYSIZ + LCDMSGR_SUMSIZ)
#define LCDMSGR_LEN64		(LCDMSG_HDRSIZ + LCDMSGR_FONSIZ + LCDMSGR_SUMSIZ)
/*--------------------------------------------------------------------------------------------------*/
/*		Check FEXE_LCDBUF_SIZE																		*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 //TODO:
#if		FEXE_LCDBUF_SIZE != LCDMSGR_LEN60
******* Build Error : FEXE_LCDBUF_SIZE/LCDMSGR_LEN60 Unmatch Error *******
#endif
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Message Index																				*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDMSGR_AXIDX		0								/* Axis address index				*/
#define LCDMSGR_FUNIDX		(LCDMSGR_AXIDX+LCDMSG_AXSIZ)	/* Axis address index				*/
#define LCDMSGR_LENIDX		(LCDMSGR_FUNIDX+LCDMSG_FUNSIZ)	/* Message length index				*/
#define LCDMSGR_LEDIDX		(LCDMSGR_LENIDX+LCDMSG_LENSIZ)	/* LEDStatus index			*/
#define LCDMSGR_DSPIDX		(LCDMSGR_LEDIDX+LCDMSGR_LEDSIZ)	/* Display status index			*/
#define LCDMSGR_CHRIDX		(LCDMSGR_DSPIDX+LCDMSGR_DSPSIZ)	/* Display character index					*/
#define LCDMSGR_SUMIDX60	(LCDMSGR_CHRIDX+LCDMSGR_CHRSIZ)	/* Checksum index(60For)		*/
#define LCDMSGR_FONIDX64	(LCDMSGR_LENIDX+LCDMSG_LENSIZ)	/* Font data index(64For)		*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDMSGC_KEYIDX		LCDMSGR_LEDIDX					/* Key code index(Command)		*/
/*--------------------------------------------------------------------------------------------------*/
/*		Message Buffer Operation Macro																*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_GET_LCDKEY( Buf )		(*(USHORT*)&(Buf[LCDMSGC_KEYIDX]))
/*--------------------------------------------------------------------------------------------------*/
#define LPX_GET_MSGLEN( Buf )		(*(USHORT*)&(Buf[LCDMSGR_LENIDX]))
#define LPX_SET_MSGLEN( Buf, Set )	(*(USHORT*)&(Buf[LCDMSGR_LENIDX])) = Set
/*--------------------------------------------------------------------------------------------------*/
#define LPX_GET_LEDSTS( Buf )		(*(USHORT*)&(Buf[LCDMSGR_LEDIDX]))
#define LPX_SET_LEDSTS( Buf, Set )	(*(USHORT*)&(Buf[LCDMSGR_LEDIDX])) = Set
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Macro Definition																*/
/*																									*/
/****************************************************************************************************/
//#if 0 /* TODO: */
#if 1														 				/* <S18F> */
typedef	struct	{
		struct	{								/* Display information											*/
		USHORT	PnUn[4];						/* Pn/UnType	0 = Un, 1 = Pn						*/
		USHORT	UnNo[4];						/* UnNumber											*/
		USHORT	PnNo[4];						/* PnNumber											*/
		}		dispinf[LCDPMDSP_MAX];			/* index: PRM/MON Display ID						*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	accesslvl[LCDPMDSP_MAX];		/* Access level(0xFFFFis data invalid)				*/
		USHORT	Old_OpeDspMode;					/* Previous operator user constant display selection			*/
		USHORT	Old_FnNo;						/* SavedFnNumber								*/
		USHORT	chksum;							/* Checksum										*/
} LCDEEP;
#define	LCDEEP_WDSIZE		55
/*--------------------------------------------------------------------------------------------------*/
//#if		LCDEEP_WDSIZE != EEPSIZE_LCDINF									/* <S18F> */
//******* Build Error : LCDEEP_WDSIZE/EEPSIZE_LCDINF Unmatch Error *******
//#endif
#define	EEPADR_LCDINF	EEP_NOSUM_ADDRESS( 0, EEP_LCDINF_OFS )				/* <S18F> */
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Keys																			*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDKEY_MODE		0x0001
#define LCDKEY_SHUP		0x0002
#define LCDKEY_UP		0x0004
#define LCDKEY_DOWN		0x0008
#define LCDKEY_SCRL		0x0010
#define LCDKEY_SVON		0x0020
#define LCDKEY_DATA		0x0040
#define LCDKEY_SHDOWN	0x0080
#define LCDKEY_READ		0x0100
#define LCDKEY_WRITE	0x0200
#define LCDKEY_ALMRST	0x0400
#define LCDKEY_RST		(LCDKEY_UP | LCDKEY_DOWN)
/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator LED display bits																*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDLED_REF		0x0001
#define LCDLED_TGON		0x0002
#define LCDLED_SVON		0x0004
#define LCDLED_COIN		0x0008
/*--------------------------------------------------------------------------------------------------*/
#define LCDLED_READ		0x0020
#define LCDLED_WRITE	0x0040
#define LCDLED_CHG		0x0080
/*--------------------------------------------------------------------------------------------------*/
#define LCDLED_REFB		0x0100
#define LCDLED_TGONB	0x0200
#define LCDLED_SVONB	0x0400
#define LCDLED_COINB	0x0800
/*--------------------------------------------------------------------------------------------------*/
#define LCDLED_READB	0x2000
#define LCDLED_WRITEB	0x4000
#define LCDLED_CHGB		0x8000

/*--------------------------------------------------------------------------------------------------*/
/*		Online modes (for LopV.Online)																*/
/*--------------------------------------------------------------------------------------------------*/
#define LCD_OFFLINE		0				/* LCDOperator is not connected							*/
#define LCD_ONLINE1		1				/* Font loaded(0x64Command)						*/
#define LCD_ONLINE2		2				/* Communication established(Font loading andLCDRelated variable initialized)	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Edit modes (for LopV.EditMode)																*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDEM_PnUnNum	0				/* Pn/UnNumber editing											*/
#define LCDEM_PnUnSel	1				/* Pn/UnSelecting												*/
#define LCDEM_PnData	2				/* Editing parameter value										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Pn/Un selection (for Display Inf Tables)													*/
/*--------------------------------------------------------------------------------------------------*/
#define UnSelected		0				/* UnSelecting													*/
#define PnSelected		1				/* PnSelecting													*/
/*--------------------------------------------------------------------------------------------------*/
/*		Operator Parameter Write Modes																*/
/*--------------------------------------------------------------------------------------------------*/
#define	OPWM_None				0					/*												*/
#define	OPWM_UpdtWait			1					/*												*/
#define	OPWM_UpdtBlnk			2					/* LCDonly										*/
#define	OPWM_InfWrWait			3					/* LCDonly										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Function modes (for LopV.FunMode)															*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDFM_Num		0				/* FnNumber selection												*/
#define LCDFM_Exe		1				/* PnExecuting													*/
/*--------------------------------------------------------------------------------------------------*/
/*		Function number of the Password inpu mode (fn010)											*/
/*--------------------------------------------------------------------------------------------------*/
#define LCDPASSWD_FNNUM (0x010)

#if CCSW_CMDID == CMDID_EXIST			/*<11J42>*/
#define LCDFM_OPTLIST_RES	2	/* Optional auxiliary function list acquisition command response processing status			*/
#define LCDFM_OPTBEGIN_RES	3	/* Optional auxiliary function start command response processing status					*/
#define LCDFM_OPTEXE		4	/* Optional auxiliary function running											*/
#define LCDFM_OPTMODE_RES	5	/* Optional auxiliary function operation command response processing status					*/
#define LCDFM_OPTEND_RES	6	/* Optional auxiliary function end command response processing status					*/
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Drive operation status (for LopV.DrvOpStat)													*/
/*--------------------------------------------------------------------------------------------------*/
#define DRVOPST_WAITKEYSEND 0			/* Waiting for key information transfer		*/
#define DRVOPST_SENDKEYINFO 1			/* Key information transfer		*/
#define DRVOPST_FORCECON    2			/* Forced connection	*/
#define DRVOPST_ERROR	    3			/* Error status	*/


/****************************************************************************************************/
/*																									*/
/*		Lcd Operator Message I/F																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Supplement:#defineis complicated, so if the structure definition+#define(For future use)							*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Digiope communication command code																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCD_CMD_NOR				(0x60)			/* Digiope Normal communication								*/
#define	LCD_CMD_LOAD			(0x61)			/* Digiope EEPROMReading							*/
#define	LCD_CMD_SAVE			(0x62)			/* Digiope EEPROMWrite							*/
#define	LCD_CMD_DIAG			(0x63)			/* Digiope Self-diagnosis command						*/
#define	LCD_CMD_INIT			(0x64)			/* Digiope Power-on command(Initial communication)				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Normal communication: Definition of reception/transmission buffer data structure														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* CmdCode : 0x60									*/
		UCHAR	AxisAdr;						/* Axis address										*/
		UCHAR	CmdCode;						/* Command code									*/
		USHORT	Length;							/* Received message length(ByteNumber)							*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	OpeKey;							/* Operator operation keys								*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	ChkSum;							/* Checksum										*/
} RXBUF_NOR;									/* How to use:RxBuf=(RXBUF_NOR*)LopMsg.CmdBuf)		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* CmdCode : 0x60									*/
		UCHAR	AxisAdr;						/* Axis address										*/
		UCHAR	CmdCode;						/* Command code									*/
		USHORT	Length;							/* Response message length(ByteNumber)							*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	LedSts;							/* LED display information									*/
		UCHAR	DispSts[16];					/* Status display									*/
		UCHAR	DispLine[5][17+1];				/* Screen display information(Title + 4*DispLine)					*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	ChkSum;							/* Checksum										*/
} TXBUF_NOR;									/* How to use:TxBuf=(TXBUF_NOR*)LopMsg.ResBuf)		*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCD_RES_NOR_LEN  (4 + 2+16+(5*18) + 2)	/* Response message length(ByteNumber)							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Power-on: Reception/transmission buffer data structure definition(Initial communication)											*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* CmdCode : 0x64									*/
		UCHAR	AxisAdr;						/* Axis address										*/
		UCHAR	CmdCode;						/* Command code									*/
		USHORT	Length;							/* Response message length(ByteNumber)							*/
		USHORT	ChkSum;							/* Checksum										*/
} RXBUF_INIT;									/* How to use:RxBuf=(TXBUF_INIT*)LopMsg.CmdBuf)		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* CmdCode : 0x64									*/
		UCHAR	AxisAdr;						/* Axis address										*/
		UCHAR	CmdCode;						/* Command code									*/
		USHORT	Length;							/* Response message length(ByteNumber)							*/
	/*----------------------------------------------------------------------------------------------*/
		UCHAR	LcdFont[(5*16)];				/* Font loaded to LCD operator				*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	ChkSum;							/* Checksum										*/
} TXBUF_INIT;									/* How to use:TxBuf=(TXBUF_INIT*)LopMsg.ResBuf)		*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCD_RES_INIT_LEN  (4 + (5*16) + 2)		/* Response message length(ByteNumber)							*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*		<S031> LcdPrmCopy LOADCommand transmission/reception buffer data structure definition									*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* CmdCode : 0x61									*/
		UCHAR	AxisAdr;						/* Axis address										*/
		UCHAR	CmdCode;						/* Command code									*/
		USHORT	Length;							/* Received message length(ByteNumber)							*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	OpeKey;							/* Operator operation keys								*/
		USHORT	Data[8];						/* Receive data										*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	ChkSum;							/* Checksum										*/
} RXBUF_LOAD;									/* How to use:RxBuf=(RXBUF_LOAD*)LopMsg.CmdBuf)		*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCD_RES_LOAD_LEN  (LCD_RES_NOR_LEN  +  4)	/* Response message length(ByteNumber)						*/
/*--------------------------------------------------------------------------------------------------*/
/*		@@ LcdPrmCopy SAVECommand transmission/reception buffer data structure definition										*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* CmdCode : 0x62									*/
		UCHAR	AxisAdr;						/* Axis address										*/
		UCHAR	CmdCode;						/* Command code									*/
		USHORT	Length;							/* Response message length									*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	LedSts;							/* LED display information									*/
		UCHAR	DispSts[16];					/* Status display									*/
		UCHAR	DispLine[5][17+1];				/* Screen display information(Title + 4*DispLine)					*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	EepAddr;						/* EEPROMAddress(WORD)								*/
		USHORT	DataNum;						/* Number of send data(0~8)								*/
		USHORT	Data[8];						/* Send data										*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	ChkSum;							/* Checksum										*/
	/*----------------------------------------------------------------------------------------------*/
		UCHAR	DispLineBlink[5];				/* Work : Screen display blink information						*/
} TXBUF_SAVE;									/* How to use:TxBuf=(TXBUF_SAVE*)LopMsg.ResBuf)		*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCD_RES_SAVE_LEN  (LCD_RES_LOAD_LEN + 16)	/* Response message length(ByteNumber)						*/

/****************************************************************************************************/
/*																									*/
/*		Operator I/F Struct Definition																*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Operator operation key definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct {
		UINT	mode		:1;					/* Mode Key											*/
		UINT	up			:1;					/* Up Key											*/
		UINT	down		:1;					/* Down Key											*/
		UINT	shup		:1;					/* Shift-up Key : <-								*/
		UINT	shdown		:1;					/* Shift-down Key : ->								*/
		UINT	data		:1;					/* Data Key											*/
		UINT	svon		:1;					/* Svon Key											*/
		UINT	almrst		:1;					/* Alarm Reset Key									*/
		UINT	scroll		:1;					/* Scroll Key										*/
		UINT	read		:1;					/* Read Key											*/
		UINT	write		:1;					/* Write Key										*/
		UINT	rst			:1;					/* Up+DownSimultaneous press									*/
} OPKEYFLG;
/*--------------------------------------------------------------------------------------------------*/
/*		Operator flashing flag definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct {
		UINT	PrmEewrite	:1;					/* Writing Parameter to EEPROM						*/
		UINT	read		:1;					/* Read												*/
		UINT	write		:1;					/* Write											*/
		UINT	no_op		:1;					/* No Operation										*/
		UINT	error		:1;					/* Error											*/
		UINT	done		:1;					/* Done												*/
		UINT	end			:1;					/* End												*/
} BLINKFLG;
/*--------------------------------------------------------------------------------------------------*/
/*		LEDDisplay data																				*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct {
		UCHAR	LedCode[5];						/* Code to display									*/
		UCHAR	LedBlnkSw[5];					/* 1part is blinking							*/
} LEDDSP;
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Operation modes (for Lcd.OpMode)															*/
/*--------------------------------------------------------------------------------------------------*/
enum _LCDOM_DISPSTATE {
		LCDOM_StartDisp = 0,		/* 0:Start screen displayed											*/
		LCDOM_PrmMon,				/* 1:PRM/MONScreen mode											*/
		LCDOM_Fun,					/* 2:Auxiliary function screen mode											*/
		LCDOM_SelectUnit,			/* 3:Unit selection screen mode										*/
		LCDOM_SelectAxis,			/* 4:Axis selection screen mode											*/
		LCDOM_Passwd,				/* 5:Password input screen mode									*/
		LCDOM_Error,				/* 6:Error status(No response)											*/
		LCDOM_PrmCopy,				/* 7:<S031> Parameter copy									*/
		LCDOM_DISPNUM
};

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Op Blink switch: TRUEblink for a while												*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct {
	UINT	read	:1;				/* ReadKey LEDFor												*/
	UINT	write	:1;				/* WriteKey LEDFor												*/
} LCDBLNKSW;

/*--------------------------------------------------------------------------------------------------*/
/*		Display information table																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct {
	USHORT	PnUn;					/* Pn/UnType	0 = Un, 1 = Pn									*/
	USHORT	UnIdx;					/* UnIndex PnUN == UnValid only when <ROMTable onlyUnNumber>	*/
	USHORT	PnIdx;					/* PnIndex PnUN == PnValid only when <ROMTable onlyPnNumber>	*/
}	LOPDIT;

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator DispInfo Variables																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct {
	USHORT	CurFunIndx;				/* Auxiliary function current position index									*/
	LOPDIT	InfTbl[4][4];			/* Display information table(PnXxx/UnXxx)								*/
} LOPDSP;

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Drive Selection Variables	<0BJ14>												*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct {
	UCHAR	DispNumber;				/* Display Number												*/
	UCHAR	DispCurIdx;				/* Display Current Index										*/
	UCHAR	DispTopIdx;				/* Display TopLine Index										*/
	UCHAR	Spare00001;				/* Spare														*/
/*--------------------------------------------------------------------------------------------------*/
	struct	{						/* Display Drive Data											*/
		UCHAR	Type;				/* Unit Type (Converter, Drive)									*/
		UCHAR	Address;			/* Unit Address		 											*/
		UCHAR	AxisNumber;			/* Drive Axis Number											*/
		UCHAR	DispCurAxis;		/* Display Current Index										*/
/*--------------------------------------------------------------------------------------------------*/
		struct {
			UCHAR	Type;			/* Drive Type (Converter, DriveAxis)							*/
			UCHAR	AxisNo;			/* Drive Axis Number											*/
			USHORT	AlmCode;		/* Drive Alarm Code												*/
		} Axis[DRVSEL_MAXAXISPERUNIT];
/*--------------------------------------------------------------------------------------------------*/
	} Unit[DRVSEL_MAXUNIT];
/*--------------------------------------------------------------------------------------------------*/
	ADJUST_STRUCT_ALIGN( Dummy )	/* Adjust HEW Structure to Long Align							*/
} DRVSEL;

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Common Variables																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct {
	UCHAR	CurMsgCmd;				/* Message command for this time <0BJ14>								*/
	UCHAR	DrvSelDispMode;			/* Drive selection screen mode <0BJ14> 								*/
	USHORT	PasswdIdx;				/* Fn010ofindex <0BJ14>											*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	Online;					/* LCDOperator connection status										*/
	USHORT	OpMode;					/* LCDOperator operation status										*/
	ULONG	StDispTime;				/* For startup display measurement												*/
	USHORT	RawKeyLast;				/* Key data previous value												*/
	USHORT	KeyHoldCnt;				/* Key hold/repeat count								*/
	USHORT	ActiveLine;				/* Active line													*/
	USHORT	EditMode;				/* Editing mode													*/
	USHORT	DataDigit;				/* Number of data digits													*/
	USHORT	CurDigit;				/* Current editing position													*/
	USHORT	EewriteMode;			/* EEPROMWrite status												*/
	USHORT	FunMode;				/* LCDOperator auxiliary function status									*/
/*--------------------------------------------------------------------------------------------------*/
	LCDBLNKSW  LedBlnkSw;			/* Blink switch												*/
	DRVSEL	*pDrvSel;				/* Drive selection data pointer									*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT  PrmInfoGetDone;			/* <S031> Parameter Information Get Complete					*/
/*--------------------------------------------------------------------------------------------------*/
} LOPV;



/****************************************************************************************************/
/*																									*/
/*		Operator I/F Functions 																		*/
/*																									*/
/****************************************************************************************************/
LONG	RpxLcdOpMain(						/* Lcd Operator Main Program							*/
		MSGIF	*hMsg,						/* Message Interface									*/
		DRVSEL	*pDrvSel	);				/* Drive Selection Data for Multi-Axis System			*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdOpFinish( MSGIF *hMsg );		/* Lcd Operator Finish Procedure						*/
/*--------------------------------------------------------------------------------------------------*/
//void	RpxLcdOpInitDispInfo( void );		/* Lcd Operator Init. Display Info. Table				*/
void	RpxLcdOpInitDispInfo(  AXIS_HANDLE* );	/* Lcd Operator Init. Display Info. Table  <S213> 	*/

/*--------------------------------------------------------------------------------------------------*/
/*		LCD operator internal functions																	*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdClrTxBuf(						/* Clear ResBuf(TxBuf)									*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
USHORT	RpxLcdPnUnEditSub(					/* Pn/Un Edit Mode Procedure							*/
		USHORT	dispID,						/* DisplayID(LCDPMDSP_PM/JOG/etc)					*/
		OPKEYFLG *pKeyMk,					/* Key press information address								*/
		BLINKFLG *pBlnkKickFlg	);			/* Blink start flag pointer							*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdMkDspDataPnUnLines(			/* Make Display Data for Pn/Un Lines					*/
		USHORT	dispID,						/* DisplayID(LCDPMDSP_PM/JOG/etc)					*/
		USHORT	BlnkSupFlg,					/* Blink suppression flag									*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdRstPnUnDspData( void );		/* Reset Pn/UN Display Data 							*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetPnUnData(					/* Pn/Un Display Data Set								*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	IsLong,						/* When using long parameterTRUE								*/
		USHORT	Base,						/* Bottom(Display attributes)											*/
		USHORT	Sign,						/* Sign presence/absence(Display attributes)									*/
		SHORT	Decimals,					/* Number of decimal places(Display attributes)								*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetSignedDecData(				/* Lcd Operator Signed10Hex number display						*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		LONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display(Does not include decimal point)							*/
		USHORT	SupFlg,						/* TRUE=High-order digits0In the case of'0'				*/
		SHORT	Decimals,					/* Number of decimal places(Effective only when positive)						*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetStr(						/* Lcd Operator String Set								*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		UCHAR	*pStr,						/* Character string address to display								*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
LONG	RpxLcdSetSignChar(					/* Lcd Operator Sign Char Set							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		LONG	Data,						/* Data to display										*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetDecPnt(					/* Lcd Operator Decimal w/point Set						*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display(Does not include decimal point)							*/
		USHORT	SupFlg,						/* TRUE=High-order digits0In the case of'0'				*/
		SHORT	Decimals,					/* Number of decimal places(Effective only when positive)						*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetValHex(					/* Lcd Operator HEX Set									*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		ULONG	Data,						/* Display data											*/
		USHORT	Digit,						/* Number of digits to display											*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetBit(						/* Lcd Operator BIT Set									*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		UCHAR	Data,						/* Display data											*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetBlnk(						/* Lcd Operator LCD Blink Set							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		USHORT	Dpos,						/* Start digit position(Left end0)									*/
		USHORT	Length,						/* Number of characters												*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdClrLineBlnk(					/* Lcd Operator LCD Blink Stop							*/
		USHORT	Lpos,						/* Start line position(Top line0)								*/
		UCHAR	*ResBuf			);			/* Response buffer address									*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdSetResStatus( UCHAR *ResBuf );/* <S031> Lcd Operator Set Status						*/
void 	RpxLcdSetBlink (UCHAR *ResBuf);		/* <S031> Lcd Set Blink									*/


/*--------------------------------------------------------------------------------------------------*/
/*		<S031> LCD parameter copy IF function														*/
/*--------------------------------------------------------------------------------------------------*/
#if	( SVFSEL_LCDPRMCOPY_USE == 1 )
void	RpxLcdPrmCopyInit( void );			/* Parameter copy function initialization processing						*/
LONG	RpxLcdPrmInfoGet( void );			/* Parameter block information acquisition processing						*/
void	RpxLcdPrmCopyMain( MSGIF *Hmsg, DRVSEL *pDrvSel );	/* Parameter copy function main processing		*/
#define __LCD_PRM_COPY_MAIN(x)  (x)
#define RpxChkPrmCopyInfoGet( ) (LopV.PrmInfoGetDone)
#else
#define	RpxLcdPrmCopyInit( ) {;}
#define	RpxLcdPrmInfoGet( )  (TRUE)
#define	RpxLcdPrmCopyMain( ) {;}
#define __LCD_PRM_COPY_MAIN(x)  (LcdError)
#define RpxChkPrmCopyInfoGet( ) (TRUE)
#endif
/*------------------------------------------------------------------------------------------*///<S197>
/*		Operator user constant display Forced change function (Pn00B)			*/
/*------------------------------------------------------------------------------------------*/
void LpxPrmChangeOpeDspMode(void);			/* Display all user constants						*/
void LpxPrmReChangeOpeDspMode(void);		/* Restore display settings								*/

/****************************************************************************************************/
/*																									*/
/*		Operator IF Display information table																*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
// Jupiter Converter
/*--------------------------------------------------------------------------------------------------*/
#define CINITDISP_PRMMON_JPCV  { \
		UnSelected, 0x000, 0x000,	/* Un, Un000, Pn000		PRM/MON						*/ \
		UnSelected, 0x007, 0x001,	/* Un, Un007, Pn001									*/ \
		UnSelected, 0x009, 0x002,	/* Un, Un009, Pn002									*/ \
		UnSelected, 0x00A, 0x00B	/* Un, Un00A, Pn00B									*/ \
	}
#define CINITDISP_ZSET_JPCV  { \
		UnSelected, 0x000, 0x000,	/* Un, Un000, Pn000		Origin search					*/ \
		UnSelected, 0x002, 0x001,	/* Un, Un002, Pn001									*/ \
		UnSelected, 0x003, 0x002,	/* Un, Un003, Pn002									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define  CINITDISP_JOG_JPCV  { \
		PnSelected, 0x000, 0x304,	/* Pn, Un000, Pn304		JOG							*/ \
		UnSelected, 0x000, 0x001,	/* Un, Un000, Pn001									*/ \
		UnSelected, 0x002, 0x002,	/* Un, Un002, Pn002									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define  CINITDISP_PJOG_JPCV  { \
		PnSelected, 0x000, 0x531,	/* Pn, Un000, Pn531		Program JOG					*/ \
		PnSelected, 0x007, 0x533,	/* Pn, Un007, Pn533									*/ \
		PnSelected, 0x00C, 0x534,	/* Pn, Un00C, Pn534									*/ \
		PnSelected, 0x00D, 0x536	/* Pn, Un00D, Pn536									*/ \
	}
/*--------------------------------------------------------------------------------------------------*/
// Jupiter Spindle Analog I/F
/*--------------------------------------------------------------------------------------------------*/
#define CINITDISP_PRMMON_JPAN  { \
		UnSelected, 0x000, 0x006,	/* Un, Un000, Pn006		PRM/MON						*/ \
		UnSelected, 0x002, 0x007,	/* Un, Un002, Pn007									*/ \
		UnSelected, 0x008, 0x008,	/* Un, Un008, Pn008									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define CINITDISP_ZSET_JPAN  { \
		UnSelected, 0x000, 0x006,	/* Un, Un000, Pn006		Origin search					*/ \
		UnSelected, 0x002, 0x007,	/* Un, Un002, Pn007									*/ \
		UnSelected, 0x003, 0x008,	/* Un, Un003, Pn008									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define CINITDISP_JOG_JPAN  { \
		PnSelected, 0x000, 0x304,	/* Pn, Un000, Pn304		JOG							*/ \
		UnSelected, 0x000, 0x007,	/* Un, Un000, Pn007									*/ \
		UnSelected, 0x002, 0x008,	/* Un, Un002, Pn008									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define CINITDISP_PJOG_JPAN  { \
		PnSelected, 0x000, 0x531,	/* Pn, Un000, Pn531		Program JOG					*/ \
		PnSelected, 0x007, 0x533,	/* Pn, Un007, Pn533									*/ \
		PnSelected, 0x00C, 0x534,	/* Pn, Un00C, Pn534									*/ \
		PnSelected, 0x00D, 0x536	/* Pn, Un00D, Pn536									*/ \
	}
/*--------------------------------------------------------------------------------------------------*/
// SGDV Standard (Rotary)
/*--------------------------------------------------------------------------------------------------*/
#define CINITDISP_PRMMON_SGDV  { \
		UnSelected, 0x000, 0x000,	/* Un, Un000, Pn000		PRM/MON						*/ \
		UnSelected, 0x002, 0x001,	/* Un, Un002, Pn001									*/ \
		UnSelected, 0x008, 0x002,	/* Un, Un008, Pn002									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define CINITDISP_ZSET_SGDV  { \
		UnSelected, 0x000, 0x000,	/* Un, Un000, Pn000		Origin search					*/ \
		UnSelected, 0x002, 0x001,	/* Un, Un002, Pn001									*/ \
		UnSelected, 0x003, 0x002,	/* Un, Un003, Pn002									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define  CINITDISP_JOG_SGDV  { \
		PnSelected, 0x000, 0x304,	/* Pn, Un000, Pn304		JOG							*/ \
		UnSelected, 0x000, 0x001,	/* Un, Un000, Pn001									*/ \
		UnSelected, 0x002, 0x002,	/* Un, Un002, Pn002									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define  CINITDISP_PJOG_SGDV  { \
		PnSelected, 0x000, 0x531,	/* Pn, Un000, Pn531		Program JOG					*/ \
		PnSelected, 0x007, 0x533,	/* Pn, Un007, Pn533									*/ \
		PnSelected, 0x00C, 0x534,	/* Pn, Un00C, Pn534									*/ \
		PnSelected, 0x00D, 0x536	/* Pn, Un00D, Pn536									*/ \
	}

/*--------------------------------------------------------------------------------------------------*/
// SGDV Standard (Linear)
/*--------------------------------------------------------------------------------------------------*/
#define CINITDISP_PRMMON_SGDV_L  { \
		UnSelected, 0x000, 0x000,	/* Un, Un000, Pn000		PRM/MON						*/ \
		UnSelected, 0x002, 0x001,	/* Un, Un002, Pn001									*/ \
		UnSelected, 0x008, 0x002,	/* Un, Un008, Pn002									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define CINITDISP_ZSET_SGDV_L  { \
		UnSelected, 0x000, 0x000,	/* Un, Un000, Pn000		Origin search					*/ \
		UnSelected, 0x002, 0x001,	/* Un, Un002, Pn001									*/ \
		UnSelected, 0x003, 0x002,	/* Un, Un003, Pn002									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define  CINITDISP_JOG_SGDV_L  { \
		PnSelected, 0x000, 0x383,	/* Pn, Un000, Pn383		JOG							*/ \
		UnSelected, 0x000, 0x001,	/* Un, Un000, Pn001									*/ \
		UnSelected, 0x002, 0x002,	/* Un, Un002, Pn002									*/ \
		UnSelected, 0x00D, 0x00B	/* Un, Un00D, Pn00B									*/ \
	}
#define  CINITDISP_PJOG_SGDV_L  { \
		PnSelected, 0x000, 0x531,	/* Pn, Un000, Pn531		Program JOG					*/ \
		PnSelected, 0x007, 0x585,	/* Pn, Un007, Pn585									*/ \
		PnSelected, 0x00C, 0x534,	/* Pn, Un00C, Pn534									*/ \
		PnSelected, 0x00D, 0x536	/* Pn, Un00D, Pn536									*/ \
	}

/*--------------------------------------------------------------------------------------------------*/
/*  Mercury Standard ?																				*/
/*--------------------------------------------------------------------------------------------------*/
#define CINITDISP_PRMMON  CINITDISP_PRMMON_SGDV
#define CINITDISP_ZSET    CINITDISP_ZSET_SGDV
#define CINITDISP_JOG     CINITDISP_JOG_SGDV
#define CINITDISP_PJOG    CINITDISP_PJOG_SGDV

/****************************************************************************************************/
/*																									*/
/*		Operator IF function definition																		*/
/*																									*/
/****************************************************************************************************/
#if		CCSW_HWID == HWID_JPCV
void	LcdStartDisp( MSGIF *hMsg, DRVSEL *pDrvSel );	/* 0:Initial screen display							*/
void	LcdPrmMonMain( MSGIF *hMsg, DRVSEL *pDrvSel );	/* 1:Parameter/Monitor screen					*/
void	LcdFun( MSGIF *hMsg, DRVSEL *pDrvSel );			/* 2:Auxiliary function execution screen						*/
void	LcdSelectUnit( MSGIF *hMsg, DRVSEL *pDrvSel );	/* 3:Unit selection screen						*/
void	LcdSelectAxis( MSGIF *hMsg, DRVSEL *pDrvSel );	/* 4:Axis selection screen								*/
void	LcdPasswd( MSGIF *hMsg, DRVSEL *pDrvSel );		/* 5:Password input screen mode				*/
void	LcdError( MSGIF *hMsg, DRVSEL *pDrvSel );		/* 6:Error status(No response)						*/
#else
void	LcdStartDisp( MSGIF *hMsg, DRVSEL *pDrvSel );
void	LcdPrmMonMain( MSGIF *hMsg, DRVSEL *pDrvSel );
void	LcdFun( MSGIF *hMsg, DRVSEL *pDrvSel );
//#define	LcdSelectUnit LcdSelectDrive
#define LcdSelectUnit LcdError
void	LcdSelectAxis( MSGIF *hMsg, DRVSEL *pDrvSel );
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2-axis compatible only)	*//* <S0E4> */
void	LcdDspDatSelAxs( UINT ax_no );	/*	Lcd Display Data Select Axis	*/
#endif
#define	LcdPasswd LcdError
void	LcdError( MSGIF *hMsg, DRVSEL *pDrvSel );
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		CMD Option Function Definition																*/
/*--------------------------------------------------------------------------------------------------*/
#if CCSW_CMDID != CMDID_EXIST

#define	LcdOptGetNextOpPrmNo(x1, x2, x3, x4) (0)
#define	LcdOptGetNextSvPrmIdx(x1, x2, x3)    (0)
#define	LcdOptSetOpPrmReadReq(x1, x2, x3)    (0)
#define	LcdOptUpdateResponseData(x1, x2)     (0)
#define	LcdOptSetDataBlinkDPos(x1)  (0)
#define LCD_CHKCMD_RCVWAIT() ( TRUE )

#else

USHORT	LcdOptGetNextOpPrmNo(USHORT, USHORT, USHORT, DEGOP_PRM *);
USHORT	LcdOptGetNextSvPrmIdx(USHORT, USHORT, DEGOP_OPTCOM *);
LONG	LcdOptSetOpPrmReadReq(USHORT, USHORT, DEGOP_OPTCOM *);
LONG	LcdOptUpdateResponseData(USHORT, DEGOP_OPTCOM *);
USHORT	LcdOptSetDataBlinkDPos(DEGOP_PRM *);
#define LCD_CHKCMD_RCVWAIT() ( LcdOpt[LCDPMDSP_PM].Prm[LopV.ActiveLine].State.RcvWait != TRUE )

#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Initial display																					*/
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_HWID == HWID_JPCV
#define CSTR_INITDSP_L1 "Unit Detecting"
#define CSTR_INITDSP_L2 "Please Wait..."
#else
#define CSTR_INITDSP_L1 "Initializing"
#define CSTR_INITDSP_L2 "Please Wait..."
#endif
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2-axis compatible only)	*//* <S0E4> */
#define CSTR_SELAXS_TITLE "-AXIS SELECTION-"	/*	Title display on the first line of axis selection screen	*/
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(Single axis) only	*/
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Axis number display																					*/
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_HWID == HWID_JPCV
#define CCHR_AXISNUM 'C'
#else
#define CCHR_AXISNUM '1' /* Single Axis */
#endif

/* <S113> Start */
/*--------------------------------------------------------------------------------------------------*/
/*	Operator user constant display selection																*/
/*--------------------------------------------------------------------------------------------------*/
#define OPEDSP_SETUPPRM			0x0000		/* SETUP						*/
#define OPEDSP_ALLPRM			0x0001		/* All user-defined parameters								*/
/* <S113> End */


/****************************************************************************************************/
/*																									*/
/*		Parameter IF definition																			*/
/*																									*/
/****************************************************************************************************/
#if 0 /* SGDV */
/*--------------------------------------------------------------------------------------------------*/
USHORT	RpxMonSearchIdxOp(					/* Monitor table index search process					*/
		LONG	SrchNo,						/* Monitor number											*/
		LONG	BaseIdx,					/* Search start index									*/
		HMSGIF	Hmsg			);			/* MessageIFHandle									*/
/*--------------------------------------------------------------------------------------------------*/
LONG	RpxPrmSearchIdxOp(					/* Parameters table index search process				*/
		LONG	SrchNo,						/* Parameter number										*/
		LONG	BaseIndex,					/* Search start index									*/
		HMSGIF	Hmsg			);			/* IFHandle											*/
/*--------------------------------------------------------------------------------------------------*/
#define CHK_WRITEINHIBIT()  (Iprm.f.PrmWrtInhibit )
#else
//#define RpxMonSearchIdxOp( x1, x2, x3 ) (0) /* TODO */
//#define RpxPrmSearchIdxOp( x1, x2, x3 ) (0) /* TODO */

#define CHK_WRITEINHIBIT()  (pAxRsc->RegManager->hPnReg->WriteAccessLock)	/* check write inhibit */
#endif

/****************************************************************************************************/
/*																									*/
/*		Panel operator IF definition																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Panel operator IF function																	*/
/*--------------------------------------------------------------------------------------------------*/
#if SVFSEL_PANELOP_USE == 0											/* Not Use Panel */
/*--------------------------------------------------------------------------------------------------*/
#define RpxPnlOpMain( x )							{ ;}
#define RpxPnlOpInit( )								{ ;}
#define RpxPnlOpMonInit( x )						{ ;}
#define RpxPnlOpFinish( x )							{ ;}
/*--------------------------------------------------------------------------------------------------*/
#define RpxLedSetStatusBits( x1, x2 )				{ ;}
#define RpxLedSetValDec( x1, x2, x3, x4, x5, x6 )	{ ;}
#define RpxLedSetValHex( x1, x2, x3, x4 ) 			{ ;}
#define RpxLedSetStr( x1, x2, x3, x4 )				{ ;}
#define RpxLedDispMonDecWord( x1, x2, x3, x4 )		{ ;}
#define RpxLedDispMonDecLong( x1, x2, x3, x4, x5 )	{ ;}
#define RpxLedSetBlnk( x1, x2 )						{ ;}
/*--------------------------------------------------------------------------------------------------*/
#else
/*--------------------------------------------------------------------------------------------------*/
void	RpxPnlOpMain( HMSGIF Hmsg );		/* Panel Operator Main Program							*/
void	RpxPnlOpInit( void );				/* Panel Operator Initialize							*/
void	RpxPnlOpMonInit( HMSGIF Hmsg );		/* Panel Monitor Initialize								*/
void	RpxPnlOpFinish( HMSGIF Hmsg );		/* Panel Operator Finish								*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedDispMonDecWord(				/* LEDOperator Monitor display(10Hex word with sign/None)		*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	decimals,					/* Number of decimal places										*/
		USHORT	sign			);			/* Signed presence/absence											*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedDispMonDecLong(				/* LEDOperator Monitor display(10Advanced long/None)		*/
		ULONG	ldata,						/* ldata:Value to display										*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	dsp_pos,					/* Current display position											*/
		USHORT	decimals,					/* Number of decimal places										*/
		USHORT	sign			);			/* Sign presence/absence												*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetStatusBits(				/* LEDOperator Status bit display information set processing		*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		HMSGIF	Hmsg );						/* Erase after */
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetValDec(					/* LEDOperator Display value(10Base)Set processing					*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	digit,						/* Number of digits processed												*/
		USHORT	sup_flg,					/* 0Display suppression flag										*/
		USHORT	decimals		);			/* Number of decimal places										*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetValHex(					/* LEDOperator Display value(16Base)Set processing					*/
		USHORT	data,						/* Value to display											*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	digit			);			/* Number of digits processed												*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetStr(						/* LED Operator String Set								*/
		UCHAR	*str_ptr,					/* Character string address to display								*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		USHORT	startdgt,					/* Start digit												*/
		USHORT	length			);			/* Number of display characters											*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLedSetBlnk(						/* LED Operator Blink Set								*/
		LEDDSP	*leddspp,					/* LEDDisplay data address								*/
		UCHAR	blnkptn			);			/* blinking pattern(1bit=1Character minutes)							*/
/*--------------------------------------------------------------------------------------------------*/
#endif

/****************************************************************************************************/
/*																									*/
/*		Operator IF Message transfer																*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Dummy Message Declaration														*/
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_HWID == HWID_JPCV
	#define DECL_DUMMY_MSG()            \
		const UCHAR dummyMsg[] = {      \
			0x00, /* Slave Address */   \
			0x60, /* Function code */   \
			0x08, /* Message length */  \
			0x00, /* Message length */  \
			0x00, /* Key information */ \
			0x00, /* Key information */ \
			0xF8, /* check sum */       \
			0x9F  /* check sum */       \
		}
#else
	#define DECL_DUMMY_MSG() /* nothing */ 
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Message Transfar Function 														*/
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_HWID == HWID_JPCV
	#define LCD_TRANSMESSAGE() { \
		LONG Axis = pDrvSel->Unit[ pDrvSel->DispCurIdx ].DispCurAxis + 1; \
		KriAddLbusMsgQueue ( LMQUEID_MSGTRANSFER, \
							 (Axis << 16) | pDrvSel->Unit[ pDrvSel->DispCurIdx ].Address, \
							 sizeof(dummyMsg), \
							 dummyMsg ); \
		KpiRstLongTimer (&LopV.StDispTime); /* For timeout measure */  \
	}
#else
	#define LCD_TRANSMESSAGE() {;}
#endif

/****************************************************************************************************/
/*																									*/
/*		Operator IF variable definition																		*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Axis Variables																	*/
/*--------------------------------------------------------------------------------------------------*/
extern	LOPDSP	LopDsp;						/*														*/
extern	LCDEEP	LcdEep[MAX_AXIS_NUMBER];	/* LCD operator: DisplayEEPROMInformation				 <S208> */
extern	LCDEEP	*pLcdEep;					/* LCD display information pointer						 <S208> */
extern	USHORT	LcdWtReq;					/* LCD display informationEEPROMWrite request				 <S208> */
extern	USHORT	LcdWtCnt;					/* LCD display information writing counter flag		 <S208> */
extern	ULONG	LcdRomAdr;					/* LCD display information writingEEPROMAddress			 <S208> */
extern	ULONG	LcdRomBaseAdr;				/* LCD display information writingEEPROMAddress			 <S208> */
extern	USHORT	*LcdMemAdr;					/* LCD display information writing memory address		 <S208> */
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2-axis compatible only)	*//* <S0E4> */
extern	DRVSEL	DrvSel;					/* LCD operator: Drive (axis) selection	*/
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		LCD Operator Common Variables																*/
/*--------------------------------------------------------------------------------------------------*/
extern	LOPV		LopV;					/*														*/
extern	OPKEYFLG	OpKeyMk;				/*														*/
extern	OPKEYFLG	OpKeyBrk;				/*														*/
extern	OPKEYFLG	OpKeyLvl;				/*														*/
extern	BLINKFLG	OpBlnkFlg;				/*														*/
extern	BLINKFLG	OpBlnkKickFlg;			/*														*/
extern	ULONG		OpBlnkTime;				/*														*/
extern  FUNMSG		OpFunMsg;				/*														*/

extern	FUN_MANAGER	FnManagerSto;			/*														*/
#endif
/***************************************** end of file **********************************************/

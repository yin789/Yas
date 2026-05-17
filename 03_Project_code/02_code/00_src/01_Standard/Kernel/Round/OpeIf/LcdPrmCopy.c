/****************************************************************************************************/
/*																									*/
/*																									*/
/*		LcdPrmCopy.c : LCD operator Parameter copy function										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Machine Efficiency : LCD operator built-inEEPROM													*/
/*			Save user parameters(Write)/Restore(Reading).									*/
/*																									*/
/*			1)Parameter readout : LOAD-Command(0x61)				*/
/*			2)Parameter write : SAVE-Command(0x62)				*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2003.06.25  S.Tokuhisa															*/
/*		Rev.1.10 : 2008.12.16  T.Taniguchi	Change PrmDef/Eeprom for LongPrm						*/
/*		Rev.1.20 : 2013.10.24  K.Ozaki		Change for Mercury										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "KnlApi.h"
#include "LcdOp.h"
#include "LcdPdl.h"			/* Parameter Download */
#include "Information.h"	/* Information */
#include "Memory.h"			/* Trace Memory */

/****************************************************************************************************/
/*																									*/
/*		Macro Definition																			*/
/*																									*/
/****************************************************************************************************/
#define OBJ_READ( obj, idx, pdata ) ((obj)->Read( (obj), (idx), (pdata), MCHK_REG_READ, &LopMsg ))  /* <S073> */
#define OBJ_SEARCH( obj, num, pidx ) ((obj)->Search( (obj), (num), 0, (pidx), MCHK_REG_READ, &LopMsg ))

#undef MlibCnvLittle16
#define MlibCnvLittle16( x ) (x)					/* Erase after */

/* TODO: Argument specifications are incorrectSGDV,Jupiter!! This process will cause a problem, soNG */
#define MlibCopyByteMemory( x, y, z )		MlibCopyByteMemory( y, x, z )  

#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2-axis compatible only)	*//* <S19B> */
#define Lpx_ChkDoubleBlock( typeid ) (0) /* for now7S */
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(Single axis) only	*/
#define Lpx_ChkDoubleBlock( typeid ) (( (typeid) & 0x00FF) == SVID_SGD7W_M3) /* TODO: M3SGD7Waddition */
#endif/* <S19B> */

/****************************************************************************************************/
/*																									*/
/*		1. Define Const/Macro																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	[ Status list ]																					*/
/*																									*/
/*	------------------------------------------------------------------------------------------------*/
/*	Main mode(PrmCopyV.MainMode)																	*/
/*	------------------------------------------------------------------------------------------------*/
/*	PRMCOPY_MAINMODE_MODE_SELECT					Function selection 										*/
/*	PRMCOPY_MAINMODE_MODE_EXE						Function execution 										*/
/*		--------------------------------------------------------------------------------------------*/
/*		Function mode(PrmCopyV.SubMode)																*/
/*		--------------------------------------------------------------------------------------------*/
/*		PRMCOPY_SUBMODE_SAVE						SAVE(Servo -> DgOp)			 					*/
/*		PRMCOPY_SUBMODE_LOAD						LOAD(DgOp -> Servo) 							*/
/*		PRMCOPY_SUBMODE_VERIFY						VERIFY(Servo <-> DgOp)							*/
/*		PRMCOPY_SUBMODE_DELETE						DELETE(DgOp)	 								*/
/*			----------------------------------------------------------------------------------------*/
/*			Common execution sequence for each function(PrmCopyV.SubModeSeq)											*/
/*			----------------------------------------------------------------------------------------*/
/*			PRMCOPY_SUBMODE_SEQ_INIT				Function initial processing									*/
/*			PRMCOPY_SUBMODE_SEQ_BLOCK_SELECT		Parameter block selection							*/
/*			PRMCOPY_SUBMODE_SEQ_EXE_CONFIRM			Execution start confirmation 									*/
/*			PRMCOPY_SUBMODE_SEQ_EXE					Execution 											*/
/*				------------------------------------------------------------------------------------*/
/*				Execution sequence(PrmCopyV.ExeSeq)														*/
/*				------------------------------------------------------------------------------------*/
/*				----- SAVE(Servopack -> Digiope) or DELETE(Digiope) ---------------------------*/
/*				PRMCOPY_SAVE_SEQ_INIT				Execution initial process									*/
/*				PRMCOPY_SAVE_SEQ_HEADER_DEL_REQ		Header information deletion command request						*/
/*				PRMCOPY_SAVE_SEQ_HEADER_DEL_RSP		Header information deletion command response confirmation					*/
/*				PRMCOPY_SAVE_SEQ_PRM_SEND			Send parameter data transmission command				*/
/*				PRMCOPY_SAVE_SEQ_PRM_RSP			Confirm response to parameter data transmission command			*/
/*				PRMCOPY_SAVE_SEQ_HEADER_SEND		Header information registration command transmission						*/
/*				PRMCOPY_SAVE_SEQ_HEADER_RSP			Header information registration command response confirmation					*/
/*				PRMCOPY_SAVE_SEQ_COMPLETE			Execution completion screen displayed								*/
/*				PRMCOPY_SAVE_SEQ_CANCEL				While the interruption screen is displayed									*/
/*				------------------------------------------------------------------------------------*/
/*				----- LOAD(Digiope -> Servopack) or VERIFY(Servopack <-> Digiope) ----------*/
/*				PRMCOPY_LOAD_SEQ_INIT				Execution initial process									*/
/*				PRMCOPY_LOAD_SEQ_RECONFIRM			LOADReconfirm before execution								*/
/*				PRMCOPY_LOAD_SEQ_PRM_READ_REQ		ParameterREADCommand request						*/
/*				PRMCOPY_LOAD_SEQ_PRM_READ_RSP		ParameterREADCommand response confirmation & Process execution		*/
/*				PRMCOPY_LOAD_SEQ_COMPLETE			Execution completion screen displayed								*/
/*				PRMCOPY_LOAD_SEQ_CANCEL				While the interruption screen is displayed									*/
/*				PRMCOPY_VERIFY_SEQ_MATCHED			VERIFYResult Displaying matching screen						*/
/*				PRMCOPY_VERIFY_SEQ_UNMATCHED		VERIFYResult While the mismatch screen is being displayed						*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Main mode(PrmCopyV.MainMode)																*/
/*--------------------------------------------------------------------------------------------------*/
#define	PRMCOPY_MAINMODE_MODE_SELECT		0	/* Function selection 										*/
#define	PRMCOPY_MAINMODE_MODE_EXE			1	/* Function execution 										*/
#define	PRMCOPY_MAINMODE_ERROR_BIT	   0x0100	/* Error occurrence bit								*/
/*--------------------------------------------------------------------------------------------------*/
/*		Function execution mode(PrmCopyV.SubMode)															*/
/*--------------------------------------------------------------------------------------------------*/
#define	PRMCOPY_SUBMODE_SAVE				0	/* SAVE(Servo -> DgOp)			 					*/
#define	PRMCOPY_SUBMODE_LOAD				1	/* LOAD(DgOp -> Servo)			 					*/
#define	PRMCOPY_SUBMODE_VERIFY				2	/* VERIFY(Servopack <-> Digiope)				*/
#define	PRMCOPY_SUBMODE_DELETE				3	/* DELETE(Digiope) 								*/
#define	PRMCOPY_SUBMODE_MAX					4	/*													*/
/*--------------------------------------------------------------------------------------------------*/
/*		Function execution sequence(PrmCopyV.SubModeSeq)														*/
/*--------------------------------------------------------------------------------------------------*/
#define PRMCOPY_SUBMODE_SEQ_INIT			0	/* Function initial processing										*/
#define PRMCOPY_SUBMODE_SEQ_BLOCK_SELECT	1	/* Block selection 									*/
#define PRMCOPY_SUBMODE_SEQ_EXE_CONFIRM		2	/* Execution start confirmation 									*/
#define PRMCOPY_SUBMODE_SEQ_EXE				3	/* Execution 											*/
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence(PrmCopyV.ExeSeq)															*/
/*--------------------------------------------------------------------------------------------------*/
/* SAVE(Servo -> DgOp) or DELETE(DgOp)Processing								 							*/
#define	PRMCOPY_SAVE_SEQ_INIT				0	/* Execution initial process										*/
#define	PRMCOPY_SAVE_SEQ_HEADER_DEL_REQ		1	/* Header information deletion command request						*/
#define PRMCOPY_SAVE_SEQ_HEADER_DEL_RSP		2	/* Header information deletion command response confirmation					*/
#define PRMCOPY_SAVE_SEQ_PRM_SEND			3	/* Send parameter data transmission command					*/
#define PRMCOPY_SAVE_SEQ_PRM_RSP			4	/* Confirm response to parameter data transmission command				*/
#define PRMCOPY_SAVE_SEQ_HEADER_SEND		5	/* Header information registration command transmission						*/
#define PRMCOPY_SAVE_SEQ_HEADER_RSP			6	/* Header information registration command response confirmation					*/
#define PRMCOPY_SAVE_SEQ_COMPLETE			7	/* Execution completion screen displayed								*/
#define PRMCOPY_SAVE_SEQ_CANCEL				8	/* While the interruption screen is displayed									*/
/*--------------------------------------------------------------------------------------------------*/
#define PRMCOPY_SAVE_SEQ_PRM_READ_REQ		9	/* @@ Parameter dataEEPROMReading					*/
#define PRMCOPY_SAVE_SEQ_PRM_READ_WAIT		10	/* @@ Parameter dataEEPROMReading					*/
/*--------------------------------------------------------------------------------------------------*/
/* LOAD(DgOp -> Servo) or VERIFY(Servo <-> DgOp)Processing					 							*/
#define PRMCOPY_LOAD_SEQ_INIT				0	/* Execution initial process										*/
#define PRMCOPY_LOAD_SEQ_RECONFIRM			1	/* Reconfirm before execution										*/
#define PRMCOPY_LOAD_SEQ_PRM_READ_REQ		2	/* ParameterREADCommand request						*/
#define PRMCOPY_LOAD_SEQ_PRM_READ_RSP		3	/* ParameterREADCommand response confirmation					*/
#define PRMCOPY_LOAD_SEQ_COMPLETE			4	/* Execution completion screen displayed								*/
#define PRMCOPY_LOAD_SEQ_CANCEL				5	/* While the interruption screen is displayed									*/
#define PRMCOPY_VERIFY_SEQ_MATCHED			6	/* VERIFYResult Displaying matching screen						*/
#define PRMCOPY_VERIFY_SEQ_UNMATCHED		7	/* VERIFYResult While the mismatch screen is being displayed						*/
#define PRMCOPY_LOAD_SEQ_STORE_REQ          8   /* @@ Parameter Store Request						*/
#define PRMCOPY_LOAD_SEQ_STORE_WAIT         9   /* @@ Parameter Store Wait							*/
#define PRMCOPY_LOAD_SEQ_FILL_REQ			10  /* @@ Fill Parameter Request						*/       
#define PRMCOPY_LOAD_SEQ_FILL_WAIT			11  /* @@ Fill Parameter Request						*/       
/* Digiope parameter block management information acquisition					 										*/
#define PRMCOPY_INFOGET_SEQ_INIT			0	/* Execution initial process										*/
#define PRMCOPY_INFOGET_SEQ_MNG_READ_REQ	1	/* Parameter management information request							*/
#define PRMCOPY_INFOGET_SEQ_MNG_READ_RSP	2	/* Parameter management information acquisition							*/
#define PRMCOPY_INFOGET_SEQ_COMPLETE		3	/* Execution completion											*/
#define PRMCOPY_INFOGET_SEQ_CANCEL			4	/* Execution interrupted											*/
/* Error screen displayed																					*/
#define PRMCOPY_ERROR_NONE					0	/* No error display									*/
#define PRMCOPY_ERROR_NOT_AVAILABLE			1	/* Execution not possible error									*/
#define PRMCOPY_ERROR_NO_DATA				2	/* No selected data error								*/
#define PRMCOPY_ERROR_PRM_UNMATCH			3	/* Parameter type unmatch error					*/
/****************************************************************************************************/
/*		Digiope communication command relationship definition																*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Digiope communication command code																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCD_RCVCMD_ALARM_BIT			0x80	/* Digiope Reception command alarm bit				*/
#define	LCD_RCVCMD_MASK					((UCHAR)(~LCD_RCVCMD_ALARM_BIT))	/* Command reception mask	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Digiope communication interface Size by item(Byte)											*/
/*--------------------------------------------------------------------------------------------------*/
/* Header section */
#define LCD_MSG_AXIS_SIZE				1		/* Axis information size										*/
#define LCD_MSG_CMD_SIZE				1		/* Command code size								*/
#define LCD_MSG_LEN_SIZE				2		/* Send buffer length size								*/
/* Header section total size */
#define LCD_MSG_HEADER_SIZE				( LCD_MSG_AXIS_SIZE + LCD_MSG_CMD_SIZE + LCD_MSG_LEN_SIZE )
/* Display data section */
#define LCD_MSG_LED_SIZE				2		/* LEDStatus size								*/
#define LCD_MSG_DSP_SIZE				16		/* Display status size								*/
#define LCD_MSG_CHR_SIZE				90		/* Display character size(18Column*5Line)						*/
/* Display data section total size */
#define LCD_MSG_DISPDATA_SIZE			( LCD_MSG_LED_SIZE + LCD_MSG_DSP_SIZE + LCD_MSG_CHR_SIZE )
/* DigiopeEEPROMCommand part */
#define LCD_MSG_EEADR_SIZE				2		/* DigiopeEEPROMAddress size						*/
#define LCD_MSG_DATANUM_SIZE			2		/* Data count size									*/
/* Checksum */
#define LCD_MSG_CHKSUM_SIZE				2		/* Checksum size								*/
#define LCD_MSG_OP_KEY_SIZE				2		/* Key information size(Only when receiving)						*/
#define LCD_MSG_DATA_UNIT_SIZE			2		/* 1 data size(2Byte)							*/
#define LCD_MSG_DATA_GROUP_NUM			8		/* Number of data that can be sent and received in one communication(Exclude the sum value)	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Digiope communication interface Size by command(Byte)										*/
/*--------------------------------------------------------------------------------------------------*/
/* Normal command sent(0x60), Data length(Exclude the sum value) */
#define LCD_TX_NORMAL_TOTAL_SIZE	(LCD_MSG_HEADER_SIZE + LCD_MSG_DISPDATA_SIZE)
/* Normal command received(0x60), Data length */
#define LCD_RX_NORMAL_TOTAL_SIZE	(LCD_MSG_HEADER_SIZE + LCD_MSG_OP_KEY_SIZE + LCD_MSG_CHKSUM_SIZE)
/*--------------------------------------------------------------------------------------------------*/
/*		Digiope communication interface Item index												*/
/*--------------------------------------------------------------------------------------------------*/
/* Transmission/reception common area  */
#define LCD_MSG_AXIS_IDX				0
#define LCD_MSG_CMD_IDX					( LCD_MSG_AXIS_IDX + LCD_MSG_AXIS_SIZE )
#define LCD_MSG_LEN_IDX					( LCD_MSG_CMD_IDX + LCD_MSG_CMD_SIZE )
/* Transmission data item(Servo -> DgOp)Index */
#define LCD_TX_MSG_EEADR_IDX			LCD_TX_NORMAL_TOTAL_SIZE
#define LCD_TX_MSG_DATANUM_IDX			( LCD_TX_MSG_EEADR_IDX + LCD_MSG_EEADR_SIZE )
#define LCD_TX_MSG_DATA_IDX				( LCD_TX_MSG_DATANUM_IDX + LCD_MSG_DATANUM_SIZE )
#define LCD_TX_MSG_LOADSUM_IDX			LCD_TX_MSG_DATA_IDX
/* Receive data item(DgOp -> Servo)Index */
#define LCD_RX_MSG_DATANUM_IDX			( LCD_RX_NORMAL_TOTAL_SIZE - LCD_MSG_CHKSUM_SIZE )
#define LCD_RX_MSG_DATA_IDX				( LCD_RX_MSG_DATANUM_IDX + LCD_MSG_DATANUM_SIZE )
/****************************************************************************************************/
/*		Return code definition																			*/
/****************************************************************************************************/
#define	RC_RETRY_OVER					-2		/* Number of retries overflow						*/
#define	RC_NG							-1		/* Abnormal												*/
#define	RC_OK							0		/* Normal												*/
#define	RC_PASS							1		/* no processing									*/
#define	RC_COMPLETE						2		/* All processing completed										*/
/****************************************************************************************************/
/*		Other definitions																					*/
/****************************************************************************************************/
#define	TIME_SECOND_2					2000	/* 2000ms = 2Second										*/
#define	TIME_500MSEC					500		/* 500ms											*/
#define	TIME_1000MSEC					1000	/* 1000ms											*/
#define	RETRY_CNT_MAX					3		/* Digiope communication retry countMAX						*/
#define LCD_DISP_LINE_CHAR_MAX			17		/* Screen display columnMAX								*/
#define PRMCOPY_DISP_TITLE_MAX			1		/* Title display lineMAX								*/
#define PRMCOPY_DISP_LIST_MAX			4		/* List display lineMAX(Exclude title line)				*/
#define PRMCOPY_DISP_LINE_MAX			(PRMCOPY_DISP_TITLE_MAX + PRMCOPY_DISP_LIST_MAX) /*Display lineMAX*/
#define PRMCOPY_VERIFY_ERR_DISP_TBL_MAX	20		/* VERIFYNumber of unmatched error list table itemsMAX(5Screen)	*/
#define	PRMCOPY_BLOCK_WORD_MAX			512		/* DigiopeEEPROMParameter number of words in one block	*/
/* DigiopeEEPROM Parameter management information */
#define	PRMCOPY_MNG_INFO_TOP0			0xE00	/* Area 0: Start address   (Word Addr)				*/
#define	PRMCOPY_MNG_INFO_SIZ0			0x003	/* Area 0: Management information size (Word Size)				*/
#define	PRMCOPY_MNG_INFO_TOP1			0xE20	/* Area 1: Start address   (Word Addr)				*/
#define	PRMCOPY_MNG_INFO_SIZ1			0x010	/* Area 1: Management information size (Word Size)				*/
#define	PRMCOPY_MNG_INFO_BLOCK_MAX		7		/* Number of parameter blocks								*/
#define	PRMCOPY_MNG_DATA_NONE			((USHORT)0xFFFF)	/* Data without parameter management table settings	*/
/* Parameter management information processing type */
#define	PRMCOPY_MNG_INFODATA_CLEAR		0		/* Clear											*/
#define	PRMCOPY_MNG_INFODATA_SET		1		/* Data set										*/
#define	PRMCOPY_MNG_INFODATA_RECIEVE	2		/* Receive data acquisition									*/
#define	LCD_DISP_LINE_1					0		/* LCDDigiope Display line 1							*/
#define	LCD_DISP_LINE_2					1		/* LCDDigiope Display line 2							*/
#define	LCD_DISP_LINE_3					2		/* LCDDigiope Display line 3							*/
#define	LCD_DISP_LINE_4					3		/* LCDDigiope Display line 4							*/
#define	LCD_DISP_LINE_5					4		/* LCDDigiope Display line 5							*/
/****************************************************************************************************/
/*		Macro definition																					*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		DigiopeEEPROMAddress calculation : Parameter blockNO, ParameterIndexNo				*/
/*--------------------------------------------------------------------------------------------------*/
#define PRMCOPY_PRM_ADDR_GET(a,b)		(USHORT)((a * PRMCOPY_BLOCK_WORD_MAX) + b)
/*--------------------------------------------------------------------------------------------------*/
/*		Receive buffer: command code acquisition															*/
/*--------------------------------------------------------------------------------------------------*/
#define PRMCOPY_CMD_CODE_GET( )			( LopMsg.CmdBuf[LCD_MSG_CMD_IDX] )
/*--------------------------------------------------------------------------------------------------*/
/*		Receive buffer: Receive data acquisition																*/
/*--------------------------------------------------------------------------------------------------*/
#define PRMCOPY_CMD_DATA_GET( a )		MlibCnvLittle16( \
		*(USHORT *)&LopMsg.CmdBuf[ LCD_RX_MSG_DATA_IDX + (a * LCD_MSG_DATA_UNIT_SIZE) ] )
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		2. LCD Operator Message I/F																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Supplement:#defineis complicated, so if the structure definition+#define(For future use)							*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Digiope communication command code																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	LCD_CMD_ALMBIT		0x80				/* Digiope Reception command alarm bit				*/
#define	LCD_CMD_ALMMSK		(~LCD_CMD_ALMBIT)	/* Digiope Reception command alarm mask				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Define transmit buffer data structure(Currently in use)														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* CmdCode : 0x62									*/
		UCHAR	AxisAdr;						/* Axis address										*/
		UCHAR	CmdCode;						/* Command code(0x61~0x64)						*/
		USHORT	Length;							/* Send message length									*/
		USHORT	LedSts;							/* LED display information									*/
		UCHAR	DispSts[16];					/* Display format											*/
		UCHAR	DispLine[PRMCOPY_DISP_LINE_MAX][LCD_DISP_LINE_CHAR_MAX+1];	/* Screen display information			*/
		USHORT	EepAddr;						/* DigiopeEEPROMAddress(WORD)						*/
		USHORT	DataNum;						/* Number of transmission/reception data(0~8)								*/
		USHORT	Data[LCD_MSG_DATA_GROUP_NUM];	/* Transmission/reception data										*/
		USHORT	ChkSum;							/* Checksum										*/
	/*----------------------------------------------------------------------------------------------*/
		UCHAR	DispLineBlink[PRMCOPY_DISP_LINE_MAX];		/* Work : Screen display blink information			*/
} PRM_COPY_TXBUF;
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		3. Structure Definition for Variables														*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Parameters for parameter copy management															*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct	{
		USHORT	TypeID;							/* PrmCopyData product identificationID							*/
		ADJUST_STRUCT_ALIGN( Dummy )			/* Adjust HEW Structure to Long Align				*/
} PRM_COPY_MNGP;
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter copy function work variables																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct	{
		USHORT	MainMode;						/* Main mode(Function selection or Function execution)				*/
		USHORT	SubMode;						/* Function mode(SAVE ~ DELETE)						*/
		USHORT	SubModeSeq;						/* Common execution sequence for each function(Function initial processing ~ Execution)	*/
		USHORT	ExeSeq;							/* Execution sequence(By function)							*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	BlockNo;						/* Parameter blockNO								*/
		USHORT	ErrorNo;						/* Error message screenNO							*/
		USHORT	RetryCnt;						/* Digiope communication retry count							*/
//		USHORT	UserChkSum;						/* For user checksum calculation						*/
		USHORT	BlockMngAreaNo;					/* Block management information area number(Access management)		*/
	/*----------------------------------------------------------------------------------------------*/
		ULONG	DispHoldTime;					/* 2 second screen display timer							*/
		ULONG	DispKeepTime;					/* Display fixing timer									*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	PrmReadIndex;					/* ParameterReadIndex						*/
		USHORT	PrmReadIndex_Last;				/* Previous parameterReadIndex					*/
		USHORT	PrmWriteIndex;					/* ParameterWriteIndex						*/
		USHORT	PrmWriteIndex_Last;				/* Previous parameterWriteIndex					*/
	/*----------------------------------------------------------------------------------------------*/
//		USHORT	PrmIndex;						/* Parameter search index(Bit0:LongL/H Flag)	*/
//		USHORT	PrmIndex_Last;					/* Previous parameter search index					*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	VerifyErrListNo;				/* VERIFYMismatch listNO								*/
		USHORT	VerifyErrTblCnt;				/* VERIFYNumber of mismatch list table usage				*/
		USHORT	VerifyErrTotalCnt;				/* VERIFYNumber of mismatch occurrences								*/
		USHORT	VerifyErrPrmIdxTbl[PRMCOPY_VERIFY_ERR_DISP_TBL_MAX]; /* VERIFYMismatch list table	*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	*TraceBuffer;					/* @@ Pointer the Work Buffer						*/
		USHORT	VerifyNumber;					/* @@ Number of Parameters for Verify 				*/
		USHORT	Reserved1;						/* @@												*/
} PRM_COPY_INFO;
/*--------------------------------------------------------------------------------------------------*/
/*		LCD operator built-in EEPROM parameter block management information								*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct	{
		USHORT	ServoID;						/* Servopack product ID								*/
		USHORT	AmpWatt;						/* Servopack capacity[Watt]							*/
		USHORT	AmpVolt;						/* Servopack output voltage code						*/
		USHORT	FormatID;						/* Parameter format ID						*/
} PRM_MNGB_INFO;
/*--------------------------------------------------------------------------------------------------*/
typedef struct	{
		USHORT	SvPrm;							/* Number of servo user constant registration							*/
//		USHORT	UsrPrm;							/* Number of option user constants registered						*/
} PRM_ENTNUM_INFO;
/*--------------------------------------------------------------------------------------------------*/
/*		PRM_COPY_INFO.PrmIndex Operation Macro														*/
/*--------------------------------------------------------------------------------------------------*/
/*		PrmIndex :   0 bit : LongPrm LowWord/HighWord Flag											*/
/*		PrmIndex : F-1 bit : All Parameter Manage Table PnPrmTbl[] Index							*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_GET_HiWdFlag( Idx )		((Idx) & 0x01)
//#define LPX_GET_PrmDefPtr( Idx )	(PnPrmTbl[((Idx)>>1)].PrmDefPtr)
#define LPX_INC_SrhIndex( Idx )		((Idx) += 2)
//#define	LPX_CHK_VarIsLong( Idx )	(PnPrmTbl[((Idx)>>1)].PrmDefPtr->Attr.VarIsLong)
//#define LPX_INC_PrmIndex( Idx )		if( LPX_CHK_VarIsLong( Idx ) ){ (Idx) += 1;}else{ (Idx) += 2;}
/*--------------------------------------------------------------------------------------------------*/
/*		Long Parameter Operation Macro for Big/Little Endian										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Macro Argument     :   lwd    hwd															*/
/*		LongPrm Word Align : {LoNum, HiNum} : ULONGOF_REGDATA( (hwd<<16) + lwd ) <-- SGDV			*/
/*		LongPrm Word Align : {HiNum, LoNum} : ULONGOF_REGDATA( (lwd<<16) + hwd ) <-- ????			*/
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_BIG_ENDIAN == 1
#define GET_LWD_EEPDATA( xlong )	(USHORT)((xlong)>>0 )
#define GET_HWD_EEPDATA( xlong )	(USHORT)((xlong)>>16)
#define ULONGOF_EEPDATA( lwd, hwd )	(ULONG)((((USHORT)(hwd))<<16) + (USHORT)(lwd))
#else
#define GET_LWD_EEPDATA( xlong )	(USHORT)((xlong)>>0 )
#define GET_HWD_EEPDATA( xlong )	(USHORT)((xlong)>>16)
#define ULONGOF_EEPDATA( lwd, hwd )	(ULONG)((((USHORT)(hwd))<<16) + (USHORT)(lwd))
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Check EEPROM Parameter																		*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_CHK_EEP_PRM( pDef )		( (pDef->Attr.Place == PRMDEF_RAM_EEPROM) && \
									  (pDef->RamPtr != NULL) && (pDef->Attr.AliasPrm == FALSE) )
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		5. Const Data Definition																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Servo model ID table(Display string search table)												*/
/*--------------------------------------------------------------------------------------------------*/
static	const	SVIDTBL			SvIdTbl[] = { SVIDTBL_ENTRY_DEF };
#define	SVIDTBL_ENTRY_NUM		(sizeof(SvIdTbl)/sizeof(SvIdTbl[0]))
/*--------------------------------------------------------------------------------------------------*/
/*		Amplifier capacity display string search table															*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct
{
	USHORT 	AmpVolt;
	USHORT	AmpWatt;
    UCHAR 	DispStr[10];
	UCHAR	DispStrW[10];	
} AMP_WATT_TBL;
/*--------------------------------------------------------------------------------------------------*/
const	AMP_WATT_TBL	AmpWattTbl[] = {
	{	0,	30,		"1R0",	"A3"	},		/*   30 W		100V	*/
/*--------------------------------------------------------------------------------------------------*/
	{	1,	50,		"R70",	"A5"	},		/*   50 W		200V	*/
	{	1,	100,	"R90",	"01"	},		/*  100 W				*/
	{	1,	200,	"1R6",	"02"	},		/*  200 W				*/
	{	1,	400,	"2R8",	"04"	},		/*  400 W				*/
	{	1,	500,	"3R8",	"05"	},		/*  500 W				*/
	{	1,	750,	"5R5",	"08"	},		/*  750 W				*/
	{	1,	1000,	"7R6",	"10"	},		/*  1.0 kW				*/
	{	1,	1500,	"120",	"15"	},		/*  1.5 kW				*/
	{	1,	2000,	"180",	"20"	},		/*  2.0 kW				*/
	{	1,	3000,	"200",	"30"	},		/*  3.0 kW				*/
	{	1,	5000,	"330",	"50"	},		/*  5.0 kW				*/
	{	1,	6000,	"470",	"60"	},		/*  6.0 kW				*/
	{	1,	7500,	"550",	"75"	},		/*  7.5 kW				*/
	{	1,	11000,	"590",	"1A"	},		/* 11.0 kW				*/
	{	1,	15000,	"780",	"1E"	},		/* 15.0 kW				*/
/*--------------------------------------------------------------------------------------------------*/
	{	2,	500,	"1R9",	"05"	},		/*  500 W		400V	*/
	{	2,	1000,	"3R5",	"10"	},		/*  1.0 kW				*/
	{	2,	1500,	"5R4",	"15"	},		/*  1.5 kW				*/
	{	2,	2000,	"8R4",	"20"	},		/*  2.0 kW				*/
	{	2,	3000,	"120",	"30"	},		/*  3.0 kW				*/
	{	2,	5000,	"170",	"50"	},		/*  5.0 kW				*/
	{	2,	6000,	"210",	"60"	},		/*  6.0 kW				*/
	{	2,	7500,	"250",	"75"	},		/*  7.5 kW				*/
	{	2,	11000,	"280",	"1A"	},		/* 11.0 kW				*/
	{	2,	15000,	"370",	"1E"	},		/* 15.0 kW				*/
/*--------------------------------------------------------------------------------------------------*/
	{	5,	50,		"R70",	"A5"	},		/*   50 W		double voltage	*/
	{	5,	100,	"R90",	"01"	},		/*  100 W				*/
	{	5,	200,	"1R6",	"02"	},		/*  200 W				*/
	{	5,	400,	"2R8",	"04"	},		/*  400 W				*/
};
/*--------------------------------------------------------------------------------------------------*/
#define	AMP_WATT_TBL_IDX_MAX	(sizeof(AmpWattTbl)/sizeof(AMP_WATT_TBL))
/*--------------------------------------------------------------------------------------------------*/
/*		Input voltage display string search table																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct
{
	USHORT 	Code;
	UCHAR 	DispStr[10];
} AMP_VOLT_TBL;
/*--------------------------------------------------------------------------------------------------*/
const	AMP_VOLT_TBL	AmpVoltTbl[] =
{
	{	0,		"B"		},		/* AC 100V 				*/
	{	1,		"A"		},		/* AC 200V 				*/
	{	2,		"D"		},		/* AC 400V 				*/
	{	5,		"F"		},		/* double voltage(AC 100V x 2)	*/
};
/*--------------------------------------------------------------------------------------------------*/
#define	AMP_VOLT_TBL_IDX_MAX	(sizeof(AmpVoltTbl)/sizeof(AMP_VOLT_TBL))
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		5. Const String Data Definition																*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		Screen title(Title for each mode displayed at the top right of the screen)										*/
/****************************************************************************************************/
#if 0		/* <S154> */
const UCHAR PrmCopyDispTitleTbl[][LCD_DISP_LINE_CHAR_MAX+1] =
{
	"           -COPY-",			/* [00] : Main title	PRMCOPY_DISPTITLE_MAIN				*/
	"       -SERVO\176OP-",			/* [01] : SAVETitle		PRMCOPY_DISPTITLE_SAVE				*/
	"       -OP\176SERVO-",			/* [02] : LOADTitle		PRMCOPY_DISPTITLE_LOAD				*/
	"         -VERIFY-",			/* [03] : VERIFYTitle	PRMCOPY_DISPTITLE_VERIFY			*/
	"           -LIST-",			/* [04] : DELETETitle	PRMCOPY_DISPTITLE_DELETE			*/
};
#else		/* <S154> */
const UCHAR PrmCopyDispTitleTbl[][LCD_DISP_LINE_CHAR_MAX+1] =
{
	"             COPY",			/* [00] : Main title	PRMCOPY_DISPTITLE_MAIN				*/
	"         SERVO\176OP",			/* [01] : SAVETitle		PRMCOPY_DISPTITLE_SAVE				*/
	"         OP\176SERVO",			/* [02] : LOADTitle		PRMCOPY_DISPTITLE_LOAD				*/
	"           VERIFY",			/* [03] : VERIFYTitle	PRMCOPY_DISPTITLE_VERIFY			*/
	"             LIST",			/* [04] : DELETETitle	PRMCOPY_DISPTITLE_DELETE			*/
};
#endif
/*--------------------------------------------------------------------------------------------------*/
enum PrmCopyDispTitleTblIndex		/* Title display string table index NO definition				*/
{
	PRMCOPY_DISPTITLE_MAIN = 0,
	PRMCOPY_DISPTITLE_SAVE,
	PRMCOPY_DISPTITLE_LOAD,
	PRMCOPY_DISPTITLE_VERIFY,
	PRMCOPY_DISPTITLE_DELETE,
};
/****************************************************************************************************/
/*		Function selection screen(Function list)																		*/
/****************************************************************************************************/
const UCHAR PrmCopyDispModeStrTbl[][LCD_DISP_LINE_CHAR_MAX+1] =
{
	"1:SERVO\176OP",				/* [00] : SAVE 				PRMCOPY_DISPMODE_SAVE				*/
	"2:OP\176SERVO",				/* [01] : LOAD				PRMCOPY_DISPMODE_LOAD				*/
	"3:VERIFY",						/* [02] : VERIFY			PRMCOPY_DISPMODE_VERIFY				*/
	"4:LIST",						/* [03] : DELETE			PRMCOPY_DISPMODE_DELETE				*/
};
/*--------------------------------------------------------------------------------------------------*/
enum PrmCopyDispModeTblIndex		/* Function list display string table index NO definition				*/
{
	PRMCOPY_DISPMODE_SAVE = 0,
	PRMCOPY_DISPMODE_LOAD,
	PRMCOPY_DISPMODE_VERIFY,
	PRMCOPY_DISPMODE_DELETE,
};
/****************************************************************************************************/
/*		Parameter block selection screen(Display number)														*/
/****************************************************************************************************/
const UCHAR PrmCopyDispBlockStrTbl[][4] =
{
	"00:",							/* [00] : PRMCOPY_DISPBLOCK_NO_00								*/
	"01:",							/* [01] : PRMCOPY_DISPBLOCK_NO_01								*/
	"02:",							/* [02] : PRMCOPY_DISPBLOCK_NO_02								*/
	"03:",							/* [03] : PRMCOPY_DISPBLOCK_NO_03								*/
	"04:",							/* [04] : PRMCOPY_DISPBLOCK_NO_04								*/
	"05:",							/* [05] : PRMCOPY_DISPBLOCK_NO_05								*/
	"06:",							/* [06] : PRMCOPY_DISPBLOCK_NO_06								*/
};
/*--------------------------------------------------------------------------------------------------*/
enum PrmCopyDispBlockTblIndex		/* Parameter block displayNoString table indexNoDefinition	*/
{
	PRMCOPY_DISPBLOCK_NO_00 = 0,
	PRMCOPY_DISPBLOCK_NO_01,
	PRMCOPY_DISPBLOCK_NO_02,
	PRMCOPY_DISPBLOCK_NO_03,
	PRMCOPY_DISPBLOCK_NO_04,
	PRMCOPY_DISPBLOCK_NO_05,
	PRMCOPY_DISPBLOCK_NO_06,
};
/****************************************************************************************************/
/*		Other display strings																			*/
/****************************************************************************************************/
const UCHAR PrmCopyDispStrTbl[][LCD_DISP_LINE_CHAR_MAX+1] =
{
	"                 ",			/* [00]:PRMCOPY_DISPSTR_SPACE 				*/
	"  Start :[WRITE]",				/* [01]:PRMCOPY_DISPSTR_START_WRITE			*/
	"  Start :[READ]",				/* [02]:PRMCOPY_DISPSTR_START_READ			*/
	"  Return:[MODE]",				/* [03]:PRMCOPY_DISPSTR_RETURN_MODE			*/
	"   Reading",					/* [04]:PRMCOPY_DISPSTR_READING				*/
	"   Writing",					/* [05]:PRMCOPY_DISPSTR_WRITING				*/
	"   Verifying",					/* [06]:PRMCOPY_DISPSTR_VERIFYING			*/
	"   Deleting",					/* [07]:PRMCOPY_DISPSTR_DELETING			*/
	"    Parameters..",				/* [08]:PRMCOPY_DISPSTR_PARAMETERS_DOT		*/
	"   Completed",					/* [09]:PRMCOPY_DISPSTR_COMPLETED			*/
	"    Canceled",					/* [10]:PRMCOPY_DISPSTR_CANCELED			*/
	"   Unmatched",					/* [11]:PRMCOPY_DISPSTR_UNMATCHED_RIGHT		*/
	"    Parameters",				/* [12]:PRMCOPY_DISPSTR_PARAMETERS_RIGHT	*/
	"   No Data",					/* [13]:PRMCOPY_DISPSTR_NO_DATA				*/
	"  Not Available",				/* [14]:PRMCOPY_DISPSTR_NOT_AVAILABLE		*/
	"****",							/* [15]:PRMCOPY_DISPSTR_ASTER				*/
	"-",							/* [16]:PRMCOPY_DISPSTR_HIFUN				*/
	" All Parameters",				/* [17]:PRMCOPY_DISPSTR_ALL_PARAMETERS		*/
	"  are matched",				/* [18]:PRMCOPY_DISPSTR_ARE_MATCHED			*/
	"Unmatched",					/* [19]:PRMCOPY_DISPSTR_UNMATCHED_LEFT		*/
	"Parameters",					/* [20]:PRMCOPY_DISPSTR_PARAMETERS_LEFT		*/
	"<<SERVO>>",					/* [21]:PRMCOPY_DISPSTR_SERVO				*/
	"<< OP >>",						/* [22]:PRMCOPY_DISPSTR_OP					*/
	"<<OP-SERVO>>",					/* [23]:PRMCOPY_DISPSTR_OP_SERVO			*/
	"??",							/* [24]:PRMCOPY_DISPSTR_QUESTION_2			*/
	"A",							/* [25]:PRMCOPY_DISPSTR_A					*/
	"F",							/* [26]:PRMCOPY_DISPSTR_F					*/
	"?",							/* [27]:PRMCOPY_DISPSTR_QUESTION_1			*/
	"  FILE DELETE    ",			/* [28]:PRMCOPY_DISPSTR_FILE_DELETE			*/
	"Pn",							/* [29]:PRMCOPY_DISPSTR_PN					*/
	"To Many Errors",				/* [30]:PRMCOPY_DISPSTR_TO_MANY_ERRORS		*/
	"File List Loading",			/* [31]:PRMCOPY_DISPSTR_FILE_LIST_LOADING	*/
	"  Please Wait...",				/* [32]:PRMCOPY_DISPSTR_PLEASE_WAIT			*/
	"  Reconfirm",					/* [33]:PRMCOPY_DISPSTR_RECONFIRM			*/
	"   OK :[DATA]",				/* [34]:PRMCOPY_DISPSTR_OK_DATA				*/
	"   NO :[MODE]",				/* [35]:PRMCOPY_DISPSTR_NO_MODE				*/
	"????",							/* [36]:PRMCOPY_DISPSTR_QUESTION_4			*/
	"???",							/* [37]:PRMCOPY_DISPSTR_QUESTION_3			*/
};
/*--------------------------------------------------------------------------------------------------*/
enum PrmCopyDispStrTblIndex			/* Display character string table index NO definition	*/
{
	PRMCOPY_DISPSTR_SPACE = 0,
	PRMCOPY_DISPSTR_START_WRITE,
	PRMCOPY_DISPSTR_START_READ,
	PRMCOPY_DISPSTR_RETURN_MODE,
	PRMCOPY_DISPSTR_READING,
	PRMCOPY_DISPSTR_WRITING,
	PRMCOPY_DISPSTR_VERIFYING,
	PRMCOPY_DISPSTR_DELETING,
	PRMCOPY_DISPSTR_PARAMETERS_DOT,
	PRMCOPY_DISPSTR_COMPLETED,
	PRMCOPY_DISPSTR_CANCELED,
	PRMCOPY_DISPSTR_UNMATCHED_RIGHT,
	PRMCOPY_DISPSTR_PARAMETERS_RIGHT,
	PRMCOPY_DISPSTR_NO_DATA,
	PRMCOPY_DISPSTR_NOT_AVAILABLE,
	PRMCOPY_DISPSTR_ASTER,
	PRMCOPY_DISPSTR_HIFUN,
	PRMCOPY_DISPSTR_ALL_PARAMETERS,
	PRMCOPY_DISPSTR_ARE_MATCHED,
	PRMCOPY_DISPSTR_UNMATCHED_LEFT,
	PRMCOPY_DISPSTR_PARAMETERS_LEFT,
	PRMCOPY_DISPSTR_SERVO,
	PRMCOPY_DISPSTR_OP,
	PRMCOPY_DISPSTR_OP_SERVO,
	PRMCOPY_DISPSTR_QUESTION_2,
	PRMCOPY_DISPSTR_A,
	PRMCOPY_DISPSTR_F,
	PRMCOPY_DISPSTR_QUESTION_1,
	PRMCOPY_DISPSTR_FILE_DELETE,
	PRMCOPY_DISPSTR_PN,
	PRMCOPY_DISPSTR_TO_MANY_ERRORS,
	PRMCOPY_DISPSTR_FILE_LIST_LOADING,
	PRMCOPY_DISPSTR_PLEASE_WAIT,
	PRMCOPY_DISPSTR_RECONFIRM,
	PRMCOPY_DISPSTR_OK_DATA,
	PRMCOPY_DISPSTR_NO_MODE,
	PRMCOPY_DISPSTR_QUESTION_4,
	PRMCOPY_DISPSTR_QUESTION_3,
};
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		6. Variables Definition																		*/
/*																									*/
/****************************************************************************************************/
//#pragma section sidata begin
PRM_COPY_MNGP	PrmCopyP;
PRM_COPY_INFO	PrmCopyV;
PRM_MNGB_INFO	PrmMngTbl[PRMCOPY_MNG_INFO_BLOCK_MAX];
PRM_ENTNUM_INFO	PrmEntNum;
extern MSGIF	LopMsg;							/* LCD operator: Message IF data			*/
//#pragma section sidata end
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		7. Functions Prototype List																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	[ Function configuration ]																					*/
/*																									*/
/*	RpxLcdPrmCopyMain()							Parameter copy main								*/
/*	|																								*/
/*	|  PrmCopyInit()							Initial processing											*/
/*	|  |  PrmCopyCmdInit()						Digiope communication initialization process								*/
/*	|  |  PrmCopyNormalCmdSet()					Default is normal communication command set					*/
/*	|  |  |																							*/
/*	|  PrmCopyOpMain()							Key operation main processing									*/
/*	|  |  PrmCopyModeSelect()					Function selection											*/
/*	|  |  |  RpxLcdPrmCopyInit() 				Initialization process(Clear all structure variables, etc., only for the first time)			*/
/*	|  |  |  PrmCopyInit()						Initial processing(Communication buffer clear, etc., every time)				*/
/*	|  |  |  PrmCopyModeSelectCheck()			Function selection check processing			 					*/
/*	|  |  PrmCopyModeExe()						Function execution											*/
/*	|  |  |  PrmCopyBlockDispListMake() 		Block list display table creation process					*/
/*	|  |  |  PrmCopyBlockSelect()				Block selection process				 					*/
/*	|  |  |  |	PrmCopyBlockSelectCheck()		Block selection check process							*/
/*	|  |  |  |	PrmCopyBlockTypeCheck()			Block type check process							*/
/*	|  |  |  PrmCopyExeConfirm()				Execution start confirmation process									*/
/*	|  |  |  PrmCopySaveKeyOp()					SAVE(Servo -> DgOp)Medium KeyProcessing						*/
/*	|  |  |  PrmCopySaveOrDeleteExe()			SAVE OR DELETE(DgOp)Execution							*/
/*	|  |  |  |	PrmCopyPrmMngTblOp()			Digiope Parameter block management information editing process			*/
/*	|  |  |  |	PrmCopySaveCmdSet()				To communication work areaSAVECommand set					*/
/*	|  |  |  |	PrmCopyCmdRspCheck()			Digiope communication command response confirmation process					*/
/*	|  |  |  |	PrmCopyUsrPrmSearch()			User parameter search								*/
/*	|  |  |  |	KpiRstLongTimer()				Timer reset									*/
/*	|  |  |  |	KpiGetLongTimer()				Timer value acquisition										*/
/*	|  |  |  PrmCopyLoadKeyOp()					LOAD(DgOp -> Servo)Medium KeyProcessing						*/
/*	|  |  |  PrmCopyVerifyKeyOp()				VERIFY(Servo <-> DgOp)Medium KeyProcessing					*/
/*	|  |  |  PrmCopyLoadOrVerifyExe()			LOAD OR VERIFYExecution									*/
/*	|  |  |  |  PrmCopyLoadCmdSet()				To communication work areaLOADCommand set					*/
/*	|  |  |  |  PrmCopyCmdRspCheck()			Digiope communication command response confirmation process					*/
/*	|  |  |  |  PrmCopyLoadExePrmWrite()		One time reception(Maximum 8 parameters)parameters					*/
/*	|  |  |  |  |									ServopackEEPROM				*/
/*	|  |  |  |  PrmCopyVerifyExePrmVerify()		One time reception(Maximum 8 parameters)parameters and					*/
/*	|  |  |  |  |									Servopack parameters(EEPROM)andVERIFY		*/
/*	|  |  |  |  |  PrmCopyUsrPrmSearch()		User parametersIndexAcquisition							*/
/*	|  |  |  |  KpiRstLongTimer()				Timer reset									*/
/*	|  |  |  |  KpiGetLongTimer()				Timer value acquisition										*/
/*	|  |  PrmCopyErrorOp()						Error occurrence processing									*/
/*	|  |  |																							*/
/*	|  PrmCopyDispMain()						Screen display data creation main process						*/
/*	|  |  PrmCopyDispModeSelect()				Function selection screen						Display data creation	*/
/*	|  |  PrmCopyDispBlockSelect()				Block selection screen					Display data creation	*/
/*	|  |  PrmCopyDispExeConfirm()				Execution start confirmation screen					Display data creation	*/
/*	|  |  PrmCopyDispSaveExe()					SAVE(Servo -> DgOp)Running screen		Display data creation	*/
/*	|  |  PrmCopyDispLoadExe()					LOAD(DgOp -> Servo)Running screen		Display data creation	*/
/*	|  |  PrmCopyDispVerifyExe()				VERIFY(Servo <-> DgOp)Running screen	Display data creation	*/
/*	|  |  |  PrmCopyDispVerifyUnmatchList()		VERIFYResult Mismatch screen				Display data creation	*/
/*	|  |  PrmCopyDispDeleteExe()				DELETERunning screen					Display data creation	*/
/*	|  |  PrmCopyDispError()					Error message screen				Display data creation	*/
/*	|  |  |																							*/
/*	|  PrmCopySetSendBuf()						Data set from communication work area to send buffer		*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	[ Function list ]																					*/
/*																									*/
/*		-------------------------------------------------------------------------------------------	*/
/*		RpxLcdPrmCopyMain()				Parameter copy function main									*/
/*		-------------------------------------------------------------------------------------------	*/
/*		PrmCopyOpInit()					Initial processing													*/
/*		PrmCopyOpMain()					Key operation main processing											*/
/*		PrmCopyModeSelect()				Function selection													*/
/*		PrmCopyModeExe()				Function execution													*/
/*		PrmCopyBlockSelect()			Parameter block selection										*/
/*		PrmCopyBlockSelectCheck()		Block selection check process									*/
/*		PrmCopyBlockTypeCheck()			Block type check process									*/
/*		PrmCopyExeConfirm()				Execution start confirmation process											*/
/*		PrmCopySaveOrDeleteExe()		SAVE(Servo -> DgOp) or DELETE(DgOp)Execution						*/
/*		PrmCopyLoadOrVerifyExe()		LOAD(DgOp -> Servo)Execution										*/
/*		PrmCopySaveKeyOp()				SAVE(Servo -> DgOp)Medium  KeyProcessing								*/
/*		PrmCopyLoadKeyOp()				LOAD(DgOp -> Servo)Medium  KeyProcessing								*/
/*		PrmCopyVerifyKeyOp()			VERIFY(Servo <-> DgOp)Medium  KeyProcessing							*/
/*		PrmCopyLoadExePrmWrite()		One time reception(Maximum 8 parameters)parameters							*/
/*												ServopackEEPROM						*/
/*		PrmCopyVerifyExePrmVerify()		One time reception(Maximum 8 parameters and							*/
/*												Servopack parameters(EEPROM)andVERIFY			*/
/*		PrmCopyModeSelectCheck()		Function selection check processing										*/
/*		PrmCopyErrorOp()				Error occurrence processing											*/
/*		PrmCopyUsrPrmSearch()			User parameter search processing									*/
/*		ConvertHexToStr()				HEXto string									*/
/*		-------------------------------------------------------------------------------------------	*/
/*		PrmCopyDispMain()				Screen display data creation process main								*/
/*		PrmCopyDispError()				Error message screen				Display data creation			*/
/*		PrmCopyDispModeSelect()			Function selection screen						Display data creation			*/
/*		PrmCopyDispBlockSelect()		Block selection screen					Display data creation			*/
/*		PrmCopyDispExeConfirm()			Execution start confirmation screen					Display data creation			*/
/*		PrmCopyDispSaveExe()			SAVE(Servo -> DgOp)Running screen		Display data creation			*/
/*		PrmCopyDispLoadExe()			LOAD(DgOp -> Servo)Running screen		Display data creation			*/
/*		PrmCopyDispVerifyExe()			VERIFY(Servo <-> DgOp)Running screen	Display data creation			*/
/*		PrmCopyDispVerifyUnmatchList()	VERIFYResult Mismatch screen				Display data creation			*/
/*		PrmCopyDispDeleteExe()			DELETE(DgOp)Running screen				Display data creation			*/
/*		PrmCopyDispBlockHeader()		Screen after block selection					Header creation			*/
/*		PrmCopyMakeBlockDispLine()		Block display data creation process			Display data creation			*/
/*		PrmCopyDispInit()				Digiope initial screen					Display data creation			*/
/*		PrmCopyStrCopy()				String copy												*/
/*		PrmCopyStrCopy2()				String copy(Return the updated value of the copy destination address)			*/
/*		-------------------------------------------------------------------------------------------	*/
/*		RpxLcdPrmCopyInit()				Parameter copy function initialization									*/
/*		RpxLcdPrmInfoGet()				Digiope parameter block management information acquisition						*/
/*		PrmCopyCmdRspCheck()			Digiope communication command response confirmation process								*/
/*		PrmCopyPrmMngTblOp()			Digi operation parameter management information operation								*/
/*		PrmCopyCmdInit()				Digiope communication  Initialize										*/
/*		PrmCopyNormalCmdSet()			Digiope communication  Normal command(0x60)Work area set				*/
/*		PrmCopyLoadCmdSet()				Digiope communication  LOADCommand(0x61)Work area set				*/
/*		PrmCopySaveCmdSet()				Digiope communication  SAVECommand(0x62)Work area set				*/
/*		PrmCopySetSendBuf()				Transmit buffer set processing										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		LCD parameter copy IF function																*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxLcdPrmCopyInit( void );			/* Parameter copy function initialization processing						*/
LONG	RpxLcdPrmInfoGet( void );			/* Parameter block information acquisition processing						*/
//LONG	RpxLcdPrmCopyMain( void );			/* Parameter copy function main processing						*/
void	RpxLcdPrmCopyMain( MSGIF *Hmsg, DRVSEL *pDrvSel );	/* Parameter copy function main processing		*/
/*--------------------------------------------------------------------------------------------------*/
/*		LCD parameter copy local function															*/
/*--------------------------------------------------------------------------------------------------*/
void	PrmCopyInit(void);
LONG	PrmCopyOpMain(void);
LONG	PrmCopyModeSelect(void);
void	PrmCopyModeExe(void);
void	PrmCopyBlockSelect(void);
void	PrmCopyExeConfirm(void);
void	PrmCopyLoadOrVerifyExe(void);
void	PrmCopySaveOrDeleteExe(void);
SHORT	PrmCopyLoadExePrmWrite(void);
SHORT	PrmCopyVerifyExePrmVerify(void);
void	PrmCopySaveKeyOp(void);
void	PrmCopyLoadKeyOp(void);
void	PrmCopyVerifyKeyOp(void);
void	PrmCopyErrorOp(void);
SHORT	PrmCopyModeSelectCheck(void);
void	PrmCopyDispMain(void);
void	PrmCopyDispError(void);
void	PrmCopyDispModeSelect(void);
void	PrmCopyDispBlockSelectMain(void);
void	PrmCopyDispBlockSelect(void);
void	PrmCopyDispExeConfirm(void);
void	PrmCopyDispSaveExe(void);
void	PrmCopyDispLoadExe(void);
void	PrmCopyDispVerifyExe(void);
void	PrmCopyDispVerifyUnmatchList(void);
void	PrmCopyDispDeleteExe(void);
void	PrmCopyDispBlockHeader(void);
void	PrmCopyMakeBlockDispLine( UCHAR *, LONG );
void	PrmCopyDispInit(void);
/*--------------------------------------------------------------------------------------------------*/
SHORT	PrmCopyBlockSelectCheck(void);
LONG	PrmCopyBlockTypeCheck( LONG BlockNo );
void	PrmCopyCmdInit(void);
SHORT	PrmCopyCmdRspCheck( USHORT );
void	PrmCopyPrmMngTblOp( UCHAR, USHORT );
void	PrmCopyStrCopy( UCHAR *, UCHAR * );
void	PrmCopyStrCopy2( UCHAR **, UCHAR * );
//SHORT	PrmCopyUsrPrmSearch( USHORT *, const PRMTBL *, const USHORT );
void	ConvertHexToStr( USHORT, UCHAR *, USHORT );
/*--------------------------------------------------------------------------------------------------*/
void	PrmCopyNormalCmdSet( void );
void	PrmCopyLoadCmdSet( USHORT, USHORT );
void	PrmCopySaveCmdSet( USHORT, USHORT, USHORT * );
/*--------------------------------------------------------------------------------------------------*/
void	PrmCopyLoadMngInfoCmdSet( LONG, LONG );
void	PrmCopySaveMngInfoCmdSet( LONG, LONG );
/*--------------------------------------------------------------------------------------------------*/
void	PrmCopySetSendBuf(void);
USHORT	PrmCopyGetEntNum( const PRMTBL *, const USHORT );
//LONG	PrmCopyPrmRead( USHORT, const PRMTBL *, const USHORT, USHORT * );
/*--------------------------------------------------------------------------------------------------*/





/****************************************************************************************************/
/*																									*/
/*		Parameter copy function initialization processing																*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdPrmCopyInit( void )
{
LONG idx, ret;
ULONG DriveID = 0x00;					/* <S073> */
ULONG IfMdleId = 0x00;					/* <S073> */
/*--------------------------------------------------------------------------------------------------*/
/*		Clear all variables for parameter copying															*/
/*--------------------------------------------------------------------------------------------------*/
		MlibResetByteMemory( (void *)&PrmCopyV, sizeof(PRM_COPY_INFO) );
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter settings for parameter copy management														*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
#if		CCSW_HWID == HWID_JPCV
		PrmCopyP.TypeID = KsysP.DriveID;
#else
		PrmCopyP.TypeID = (((PtiReg.IfMdleId & 0x00FF) << 8) | (KsysP.DriveID & 0x00FF));
#endif
#endif
/*--------------------------------------------------------------------------------------------------*/
		ret = OBJ_SEARCH( CINFREG, 0xC000 & 0x0FFF /* <S073> */, &idx ); 	/* @@ 0xC000: DriveId */
		if ( ret == OK ) { OBJ_READ( CINFREG, idx, &DriveID ); }  			/* @@ 0xC000: DriveId */
/*--------------------------------------------------------------------------------------------------*/
/*	
 *		TODO:
 *		Register0xC013teethMercury, the search result uses the variable defined inNG.
 *
 */
/*--------------------------------------------------------------------------------------------------*/
		ret = OBJ_SEARCH( CINFREG, 0xC013 & 0x0FFF /* <S073> */, &idx ); 	/* @@ 0xC013: IfModuleId */
		if ( ret == OK ){ OBJ_READ( CINFREG, idx, &IfMdleId );  }			/* @@ 0xC013: IfModuleId */
		PrmCopyP.TypeID = (((IfMdleId & 0x00FF) << 8) | (DriveID & 0x00FF));
/*--------------------------------------------------------------------------------------------------*/
/*		Free Trace Buffer																			*/
/*--------------------------------------------------------------------------------------------------*/
		CTRCMEM->FreeBuffer( CTRCMEM, &PrmCopyV );
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Parameter copy function main processing																*/
/*																									*/
/****************************************************************************************************/
void	RpxLcdPrmCopyMain( MSGIF *Hmsg, DRVSEL *pDrvSel )
{
		PrmCopyInit();						/* Initial processing		 										*/
		/* ModeChg = */PrmCopyOpMain();		/* Key operation main processing									*/
		PrmCopyDispMain();					/* Screen display data creation main process							*/
		PrmCopySetSendBuf();				/* Data set from communication work area to send buffer			*/
		//return( ModeChg );
/*--------------------------------------------------------------------------------------------------*/
		RpxLcdSetResStatus ( (UCHAR *)Hmsg->ResBuf );	/* Set Status */
		RpxLcdSetBlink ( (UCHAR *)Hmsg->ResBuf );		/* Set Blink */
/*--------------------------------------------------------------------------------------------------*/
		return;
}


/****************************************************************************************************/
/*																									*/
/*		Checks Parameter Copy Execution Status														*/
/*																									*/
/****************************************************************************************************/
LONG RpxChkPrmCopyExe( void )
{
	return ( PrmCopyV.MainMode == PRMCOPY_MAINMODE_MODE_EXE ); 
}


/****************************************************************************************************/
/*																									*/
/*		Initial processing																					*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyInit( void )
{
		PrmCopyCmdInit();					/* Digiope communication initialization process									*/
		PrmCopyNormalCmdSet(); 				/* Default is normal communication command(0x60)Set 			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Key operation main processing																			*/
/*																									*/
/****************************************************************************************************/
LONG	PrmCopyOpMain( void )
{
LONG	ModeChg = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Key Operation Main																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.MainMode )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Function selection																			*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_MAINMODE_MODE_SELECT:
			ModeChg = PrmCopyModeSelect();
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Function execution																			*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_MAINMODE_MODE_EXE:
			PrmCopyModeExe();
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			/* Error occurrence processing */
			PrmCopyErrorOp();
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( ModeChg );
}



/****************************************************************************************************/
/*																									*/
/*		Function selection processing																				*/
/*																									*/
/*			"1:SERVO->OP",		[00] : SAVE 														*/
/*			"2:OP->SERVO",		[01] : LOAD															*/
/*			"3:VERIFY",			[02] : VERIFY														*/
/*			"4:DELETE",			[03] : DELETE														*/
/*																									*/
/****************************************************************************************************/
LONG	PrmCopyModeSelect( void )
{
LONG	ModeChg = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		[MODE] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( OpKeyMk.mode )
		{
			RpxLcdPrmCopyInit();					/* Initialization process(Clear all structure variables, etc., only for the first time)	*/
			PrmCopyInit();							/* Initial processing(Communication buffer clear, etc., every time)			*/
			ModeChg  = TRUE;						/* Mode change: Go to parameter edit/monitor screen		*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		[↓] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( OpKeyMk.down )
		{
			if( PrmCopyV.SubMode < PRMCOPY_SUBMODE_MAX - 1 )
			{
				PrmCopyV.SubMode++;								/* Go to the next function */
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		[↑] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( OpKeyMk.up )
		{
			if( PrmCopyV.SubMode > 0 )
			{
				PrmCopyV.SubMode--;								/* Go to previous function */
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		[DATA] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( OpKeyMk.data )
		{
			if( PrmCopyModeSelectCheck() == RC_OK )				/* Function selection check					*/
			{
				PrmCopyV.MainMode = PRMCOPY_MAINMODE_MODE_EXE;	/* Function execution status set				*/
				PrmCopyV.BlockNo = 0;							/* Initialize selected function number(First)		*/
				PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_INIT;	/* Go to selected function initial processing				*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( ModeChg );
}



/****************************************************************************************************/
/*																									*/
/*		Function execution processing																				*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyModeExe( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Execution process sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.SubModeSeq )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Function initial processing																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_SEQ_INIT:
			/* Go to block selection */
			PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_BLOCK_SELECT;
		/*------------------------------------------------------------------------------------------*/
		/*		Block selection																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_SEQ_BLOCK_SELECT:
			PrmCopyBlockSelect();				/* Block selection process				 					*/
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Execution start confirmation																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_SEQ_EXE_CONFIRM:
			PrmCopyExeConfirm();				/* Execution start confirmation process				 					*/
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Execution																				*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_SEQ_EXE:
			switch( PrmCopyV.SubMode )
			{
				/*----------------------------------------------------------------------------------*/
				/*		SAVE(Servo -> DgOp)															*/
				/*----------------------------------------------------------------------------------*/
				case PRMCOPY_SUBMODE_SAVE:
					PrmCopySaveKeyOp();			/* SaveMedium Key processing 									*/
					PrmCopySaveOrDeleteExe();	/* SaveProcess execution										*/
					break;
				/*----------------------------------------------------------------------------------*/
				/*		DELETE(DgOp)																*/
				/*----------------------------------------------------------------------------------*/
				case PRMCOPY_SUBMODE_DELETE:
					PrmCopySaveOrDeleteExe();	/* DeleteProcess execution									*/
					break;
				/*----------------------------------------------------------------------------------*/
				/*		LOAD(DgOp -> Servo)															*/
				/*----------------------------------------------------------------------------------*/
				case PRMCOPY_SUBMODE_LOAD:
					PrmCopyLoadKeyOp();			/* LoadMedium Key processing 									*/
					PrmCopyLoadOrVerifyExe();	/* LoadProcess execution										*/
					break;
				/*----------------------------------------------------------------------------------*/
				/*		VERIFY(Servo <-> DgOp)														*/
				/*----------------------------------------------------------------------------------*/
				case PRMCOPY_SUBMODE_VERIFY:
					PrmCopyVerifyKeyOp();		/* VERIFYMedium Key processing 								*/
					PrmCopyLoadOrVerifyExe();	/* VERIFYProcess execution									*/
					break;
				/*----------------------------------------------------------------------------------*/
				default:
					break;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Parameter block selection processing																	*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyBlockSelect(void)
{
SHORT	rc;
ULONG	wkx;
ULONG	BlockSft = (Lpx_ChkDoubleBlock( PrmCopyP.TypeID ) ? 1 : 0 ); 	/* @@ SGD7WCorrespondence */
ULONG	BlockMax = ((PRMCOPY_MNG_INFO_BLOCK_MAX >> BlockSft) - 1);		/* @@ SGD7WCorrespondence */
/*--------------------------------------------------------------------------------------------------*/
/*		[MODE] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( OpKeyMk.mode )
		{
			PrmCopyV.MainMode = PRMCOPY_MAINMODE_MODE_SELECT;	/* Go to function selection screen */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		[↑] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( OpKeyMk.up )
		{
			/* Parameter block display scroll up */
			wkx = PrmCopyV.BlockNo >> BlockSft;
			if( wkx == 0 )
			{
				wkx = BlockMax;
			}
			else
			{
				wkx--;
			}
			PrmCopyV.BlockNo = wkx << BlockSft;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		[↓] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( OpKeyMk.down )
		{
			/* Parameter block display scroll down */
			wkx = PrmCopyV.BlockNo >> BlockSft;
			if( wkx == BlockMax )
			{
				wkx = 0;
			}
			else
			{
				wkx++;
			}
			PrmCopyV.BlockNo = wkx << BlockSft;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		[DATA] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( OpKeyMk.data )
		{
			PrmCopyV.DispHoldTime = 0;					/* 2 second screen display counter clear			*/
			rc = PrmCopyBlockSelectCheck();				/* Block selectable check					*/
			if( rc != RC_OK ){ return;}
			PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_EXE_CONFIRM;	/* Go to execution start confirmation screen			*/
			PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_INIT;				/* Execution sequence to initial processing	*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Parameter block selection check processing															*/
/*																									*/
/****************************************************************************************************/
SHORT PrmCopyBlockSelectCheck(void)
{
/*--------------------------------------------------------------------------------------------------*/
/*		Execution process sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.SubMode )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		SAVE(Servo -> DgOp)																	*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_SAVE:
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		LOAD(DgOp -> Servo)																	*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_LOAD:
			/* "****"No data error when display is selected */
			if( PrmMngTbl[PrmCopyV.BlockNo].FormatID == PRMCOPY_MNG_DATA_NONE )
			{
				/* Go to selection not possible error display screen */
				PrmCopyV.ErrorNo = PRMCOPY_ERROR_NO_DATA;
				PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				return( RC_NG );
			}
			/* Incorrect parameter type */
			if( PrmCopyBlockTypeCheck( PrmCopyV.BlockNo ) != RC_OK )
			{
				/* Go to parameter type error display screen */
				PrmCopyV.ErrorNo = PRMCOPY_ERROR_PRM_UNMATCH;
				PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				return( RC_NG );
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		VERIFY(Servo <-> DgOp)																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_VERIFY:
			/* "****"No data error when display is selected */
			if( PrmMngTbl[PrmCopyV.BlockNo].FormatID == PRMCOPY_MNG_DATA_NONE )
			{
				/* Go to selection not possible error display screen */
				PrmCopyV.ErrorNo = PRMCOPY_ERROR_NO_DATA;
				PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				return( RC_NG );
			}
			/* Incorrect Servopack model */
			if( PrmMngTbl[PrmCopyV.BlockNo].ServoID != PrmCopyP.TypeID )
			{
				/* Go to parameter type error display screen */
				PrmCopyV.ErrorNo = PRMCOPY_ERROR_PRM_UNMATCH;
				PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				return( RC_NG );
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		DELETE(DgOp)																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_DELETE:
			/* "****"No data error when display is selected */
			if( PrmMngTbl[PrmCopyV.BlockNo].FormatID == PRMCOPY_MNG_DATA_NONE )
			{
				/* Go to selection not possible error display screen */
				PrmCopyV.ErrorNo = PRMCOPY_ERROR_NO_DATA;
				PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				return( RC_NG );
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( RC_OK );
}


/****************************************************************************************************/
/*																									*/
/*		Parameter block type check processing														*/
/*																									*/
/****************************************************************************************************/
LONG	PrmCopyBlockTypeCheck( LONG BlockNo )
{
PRMDATA *pPrm = LopMsg.pAxRsc->UniPrms->Prm;			/* TODO: Erase after */
/*--------------------------------------------------------------------------------------------------*/
/*		Block Type Check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( PrmMngTbl[BlockNo].ServoID != PrmCopyP.TypeID )
		{
			return( RC_NG );
		}
//		else if( PrmMngTbl[BlockNo].FormatID > FORMAT_ID )
		else if( PrmMngTbl[BlockNo].FormatID > EEP_FORMAT_ID )
		{
			return( RC_NG );
		}
		else if( PrmMngTbl[BlockNo].AmpWatt != pPrm->ampw )
		{
			return( RC_NG );
		}
		else if( (PrmMngTbl[BlockNo].AmpVolt & 0x0F) != (pPrm->styp.b.h & 0x0F) )
		{
			return( RC_NG );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( RC_OK );
}



/****************************************************************************************************/
/*																									*/
/*		Execution start confirmation process																			*/
/*																									*/
/****************************************************************************************************/
void PrmCopyExeConfirm(void)
{
AXIS_HANDLE *pAxRsc = LopMsg.pAxRsc;			/* TODO: Erase after */
/*--------------------------------------------------------------------------------------------------*/
/*		[MODE] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( OpKeyMk.mode )
		{
			/* After function initialization, return to block selection */
			PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_INIT;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		[READ] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( OpKeyMk.read )
		{
			/* SAVE(Servo -> DgOp) or VERIFY(Servo <-> DgOp)Processing */
			if( PrmCopyV.SubMode == PRMCOPY_SUBMODE_SAVE || PrmCopyV.SubMode == PRMCOPY_SUBMODE_VERIFY )
			{
				PrmCopyV.TraceBuffer = CTRCMEM->GetBuffer( CTRCMEM, &PrmCopyV );	/* get buffer 	*/
				if ( PrmCopyV.TraceBuffer != NULL )
				{
					/* To function processing execution */
					PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_EXE;
				}
				else
				{
					/* Go to execution error display screen */
					PrmCopyV.ErrorNo = PRMCOPY_ERROR_NOT_AVAILABLE;
					PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		[WRITE] key																					*/
/*--------------------------------------------------------------------------------------------------*/
		else if( OpKeyMk.write )
		{
			/* LOAD(DgOp -> Servo) or DELETE(DgOp)Processing	*/
			if( PrmCopyV.SubMode == PRMCOPY_SUBMODE_LOAD || PrmCopyV.SubMode == PRMCOPY_SUBMODE_DELETE )
			{
				/* Error that cannot be executed while the servo is on	*/
				if( ! CHK_BB() /*RinV.f.BaseEnable*/ )
				{
					/* Go to execution error display screen */
					PrmCopyV.ErrorNo = PRMCOPY_ERROR_NOT_AVAILABLE;
					PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				}
				else if ( ( PrmCopyV.TraceBuffer = CTRCMEM->GetBuffer( CTRCMEM, &PrmCopyV ) ) == NULL )
				{
					/* Go to execution error display screen */
					PrmCopyV.ErrorNo = PRMCOPY_ERROR_NOT_AVAILABLE;
					PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				}
				else
				{
					/* To function processing execution */
					PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_EXE;
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		SAVE(Servo -> DgOp) or DELETE(DgOp)Execution process													*/
/*																									*/
/****************************************************************************************************/
void	PrmCopySaveOrDeleteExe( void )
{
SHORT	rc;
USHORT	prm_data;
USHORT	cmd_data_idx;
USHORT	send_data[LCD_MSG_DATA_GROUP_NUM];
AXIS_HANDLE *pAxRsc = LopMsg.pAxRsc;		/* TODO: Erase after */
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*==========================================================================================*/
		/*	0. Initial processing																			*/
		/*==========================================================================================*/
		/*------------------------------------------------------------------------------------------*/
		/*		Execution initial process																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_INIT:
			PrmCopyV.RetryCnt = 0;								/* Communication retry count clear		*/
			PrmCopyV.DispHoldTime = 0;							/* 2 second screen display counter clear	*/
			PrmCopyV.DispKeepTime = 0;
			PrmCopyV.BlockMngAreaNo = 0;						/* Delete block management information area 0		*/
			PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_HEADER_DEL_REQ;	/* Return to header information deletion command request		*/
			break;
		/*==========================================================================================*/
		/*	1. DigiopeEEPROM								*/
		/*==========================================================================================*/
		/*------------------------------------------------------------------------------------------*/
		/*		Header information deletion command request															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_HEADER_DEL_REQ:
			/* Digiope Parameter block management information clear */
			PrmCopyPrmMngTblOp( PRMCOPY_MNG_INFODATA_CLEAR, PrmCopyV.BlockNo );
			/* To communication work areaSAVECommand(0x62)Set processing */
			PrmCopySaveMngInfoCmdSet( PrmCopyV.BlockNo, PrmCopyV.BlockMngAreaNo );
			/* Header information deletion command response confirmation */
			PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_HEADER_DEL_RSP;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Header information deletion command response confirmation														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_HEADER_DEL_RSP:
			/* Digiope communication command response confirmation process */
			rc = PrmCopyCmdRspCheck( LCD_CMD_SAVE );
			/*----- Receiving normally -----------------------------------------------------------------------*/
			if( rc == RC_OK )
			{
				/* Communication retry count clear */
				PrmCopyV.RetryCnt = 0;
//				PrmCopyV.PrmIndex = 0;
				PrmCopyV.PrmReadIndex = 0;
				PrmCopyV.PrmWriteIndex = 0;
			  /*------------------------------------------------------------------------------------*/
				if( PrmCopyV.BlockMngAreaNo == 0 )						/* For management information area 0		*/
				{
					PrmCopyV.BlockMngAreaNo = 1;						/* Delete management information area 1		*/
					PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_HEADER_DEL_REQ;	/* Header information deletion processing		*/
				}
				else if( PrmCopyV.SubMode == PRMCOPY_SUBMODE_SAVE )		/* For parameter saving processing	*/
				{
//Mercury			PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_PRM_SEND;		/* To send parameter data	*/
					PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_PRM_READ_REQ;	/* @@ Read parameter data*/
				}
				else													/* In case of parameter deletion process	*/
				{
					PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_COMPLETE;		/* Go to completion screen display			*/
				}
			}
			/*----- Alarm code received -------------------------------------------------------*/
			else if( (rc == RC_NG) || (rc == RC_PASS) )
			{
				/* Return to header information deletion command request */
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_HEADER_DEL_REQ;
			}
			/*----- Number of retries exceeded -----------------------------------------------------------*/
			else if( rc == RC_RETRY_OVER )
			{
				/* Interrupt processing */
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_CANCEL;
			}
			/*----- Received command code is different from sent code -----------------------------------*/
			else
			{
				;/* Continue waiting for reception(rc == RC_PASS) */
			}
			break;
		/*==========================================================================================*/
		/*	2. DigiopeEEPROM, send all user parameters to8WORDby [/66] while receiving [/66] [67] by							*/
		/*==========================================================================================*/
		/*------------------------------------------------------------------------------------------*/
		/*		Read Upload Data from Eeprom														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_PRM_READ_REQ:
			if ( KriRequestReadLcdPdlEeprom( pAxRsc->pRwPdlMng, PrmCopyV.TraceBuffer, 
									(void *)&LcdPdlTbl[0], LCDPDLTBL_ENTNUM  ) == OK )
			{
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_PRM_READ_WAIT;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Wait for Completing Read Data from Eeprom											*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_PRM_READ_WAIT:
			if (  KriChkDoneReadLcdPdlEeprom( pAxRsc->pRwPdlMng ) )
			{
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_PRM_SEND;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Send parameter data transmission command													*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_PRM_SEND:
			/* Save to restore index when retrying */
			PrmCopyV.PrmReadIndex_Last = PrmCopyV.PrmReadIndex;
			PrmCopyV.PrmWriteIndex_Last = PrmCopyV.PrmWriteIndex;
//			PrmCopyV.PrmIndex_Last = PrmCopyV.PrmIndex;
			/* Obtain eight parameter data for one transmission */
			for( cmd_data_idx = 0; cmd_data_idx < LCD_MSG_DATA_GROUP_NUM; cmd_data_idx++ )
			{
				if ( PrmCopyV.PrmReadIndex >= LCDPDLTBL_ENTNUM ) { break; }	/* Data End */
				prm_data = PrmCopyV.TraceBuffer[ PrmCopyV.PrmReadIndex ];
				/* Set parameter data in transmit buffer */
				send_data[cmd_data_idx] = prm_data;
				PrmCopyV.PrmReadIndex++;
			}
			/* DigiopeEEPROMUpdate parameter address */
			PrmCopyV.PrmWriteIndex += cmd_data_idx;
			/* To communication work areaSAVECommand(0x62)Set processing */
			PrmCopySaveCmdSet( PRMCOPY_PRM_ADDR_GET(PrmCopyV.BlockNo, PrmCopyV.PrmWriteIndex_Last), 
							cmd_data_idx, send_data );
			/* To confirm response to parameter data transmission command */
			PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_PRM_RSP;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Confirm response to parameter data transmission command												*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_PRM_RSP:
			/* Digiope communication command response confirmation process */
			rc = PrmCopyCmdRspCheck( LCD_CMD_SAVE );
			/*----- Receiving normally -----------------------------------------------------------------------*/
			if( rc == RC_OK )
			{
				/* Communication retry count clear */
				PrmCopyV.RetryCnt = 0;
				/* Complete sending all parameter data */
				if( PrmCopyV.PrmReadIndex >= PrmEntNum.SvPrm )
				{
					PrmCopyV.BlockMngAreaNo = 0;					/* Write management information area 0		*/
					PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_HEADER_SEND;	/* Next command(Management information transmission)fart */
				}
				else
				{
					/* Continue transmitting next parameter data */
					PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_PRM_SEND;
				}
			}
			/*----- Alarm code received -------------------------------------------------------*/
			else if( (rc == RC_NG) || (rc == RC_PASS) )
			{
				/* Restore parameter index */
				PrmCopyV.PrmReadIndex = PrmCopyV.PrmReadIndex_Last;
				PrmCopyV.PrmWriteIndex = PrmCopyV.PrmWriteIndex_Last;
//				PrmCopyV.PrmIndex = PrmCopyV.PrmIndex_Last;
				/* Return to send parameter data transmission command and retry */
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_PRM_SEND;
			}
			/*----- Number of retries exceeded -----------------------------------------------------------*/
			else if( rc == RC_RETRY_OVER )
			{
				/* Interrupt processing */
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_CANCEL;
			}
			/*----- Received command code is different from sent code -----------------------------------*/
			else /* if( rc == RC_PASS ) */
			{
				;/* Continue waiting for reception */
			}
			break;
		/*==========================================================================================*/
		/*	3. DigiopeEEPROM, and send management information for the corresponding parameter block							*/
		/*==========================================================================================*/
		/*------------------------------------------------------------------------------------------*/
		/*		Header information registration command transmission															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_HEADER_SEND:
			/* Digiope Parameter block management information set */
			PrmCopyPrmMngTblOp( PRMCOPY_MNG_INFODATA_SET, PrmCopyV.BlockNo );
			/* To communication work areaSAVECommand(0x62)Set processing */
			PrmCopySaveMngInfoCmdSet( PrmCopyV.BlockNo, PrmCopyV.BlockMngAreaNo );
			/* Header information registration command response confirmation */
			PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_HEADER_RSP;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Header information registration command response confirmation														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_HEADER_RSP:
			/* Digiope communication command response confirmation process */
			rc = PrmCopyCmdRspCheck( LCD_CMD_SAVE );
			/*----- Receiving normally -----------------------------------------------------------------------*/
			if( rc == RC_OK )
			{
				PrmCopyV.RetryCnt = 0;								/* Communication retry count clear	*/
				if( PrmCopyV.BlockMngAreaNo == 0 )					/* For management information area 0			*/
				{
					PrmCopyV.BlockMngAreaNo = 1;					/* Write management information area 1		*/
					PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_HEADER_SEND;	/* To send management information				*/
				}
				else
				{
					PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_COMPLETE;	/* Go to completion screen display				*/
				}
			}
			/*----- Alarm code received -------------------------------------------------------*/
			else if( (rc == RC_NG) || (rc == RC_PASS) )
			{
				/* Return to header information registration command transmission */
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_HEADER_SEND;
			}
			/*----- Number of retries exceeded -----------------------------------------------------------*/
			else if( rc == RC_RETRY_OVER )
			{
				/* Interrupt processing */
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_CANCEL;
			}
			/*----- Received command code is different from sent code -----------------------------------*/
			else
			{
				;/* Continue waiting for reception */
			}
			break;
		/*==========================================================================================*/
		/*	4. The execution sequence is maintained to display the DigiOp execution completion screen or interruption screen for about 2 seconds.		*/
		/*==========================================================================================*/
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope execution completion screen being displayed															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_COMPLETE:
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope interruption screen being displayed																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_CANCEL:
			/* Screen display first time */
			if( PrmCopyV.DispHoldTime == 0 )
			{
				/* Timer reset */
				KpiRstLongTimer( &PrmCopyV.DispHoldTime );
				break;
			}
			/* 2 seconds elapsed */
			if( KpiGetLongTimer(&PrmCopyV.DispHoldTime) > TIME_SECOND_2 )
			{
				CTRCMEM->FreeBuffer( CTRCMEM, &PrmCopyV );	/* @@ */
				/* After function initialization, return to block selection */
				PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_INIT;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LOAD(DgOp -> Servo)orVERIFY(Servo <-> DgOp)Execution process									*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyLoadOrVerifyExe( void )
{
SHORT	rc, rc2;
AXIS_HANDLE *pAxRsc = LopMsg.pAxRsc;		/* TODO: Erase after */
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*==========================================================================================*/
		/*	0. Initial processing																			*/
		/*==========================================================================================*/
		/*------------------------------------------------------------------------------------------*/
		/*		Execution initial process																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_INIT:
			/* Set parameter batch rewriting flag in parameter copy functionON */
			pAxRsc->FnCmnCtrl->f.PrmCopy = TRUE; /* RoutV.f.PrmCopy = TRUE; */
			/* Communication retry count clear */
			PrmCopyV.RetryCnt = 0;
			PrmCopyV.PrmReadIndex = 0;
			PrmCopyV.PrmWriteIndex = 0;
//			PrmCopyV.PrmIndex = 0;
//			PrmCopyV.UserChkSum = 0;
			PrmCopyV.VerifyErrTblCnt = 0;
			PrmCopyV.VerifyErrTotalCnt = 0;
			/* 2 second screen display counter clear */
			PrmCopyV.DispHoldTime = 0;
			PrmCopyV.DispKeepTime = 0;
			/* VERIFYDuring processing */
			if( PrmCopyV.SubMode == PRMCOPY_SUBMODE_VERIFY )
			{
				/* ParameterREADTo command request */
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_FILL_REQ;
				break;
			}
			/* LOADWhen processing, reconfirming before execution */
			PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_PRM_READ_REQ;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*	@@	Read Verify Data from Eeprom														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_FILL_REQ:
			if ( KriRequestReadLcdPdlEeprom( pAxRsc->pRwPdlMng, PrmCopyV.TraceBuffer, 
									(void*)&LcdPdlTbl[0], LCDPDLTBL_ENTNUM  ) == OK )
			{
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_FILL_WAIT;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Wait for Completing Read Data from Eeprom											*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_FILL_WAIT:
			if ( KriChkDoneReadLcdPdlEeprom( pAxRsc->pRwPdlMng ) )
			{
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_PRM_READ_REQ;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*------------------------------------------------------------------------------------------*/
		/*		LOADWhen processing, reconfirming before execution														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_RECONFIRM:
			break;
		/*==========================================================================================*/
		/*	1. DigiopeEEPROMThe user parameters of the corresponding block from								*/
		/*		8WORDWhile receiving data one by one, the servo packEEPROMor process to write to						*/
		/*		Servopack parameters andVERIFY												*/
		/*==========================================================================================*/
		/*------------------------------------------------------------------------------------------*/
		/*		ParameterREADCommand request															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_PRM_READ_REQ:
			/* To communication work areaLOADCommand(0x61)Set */
			PrmCopyLoadCmdSet( PRMCOPY_PRM_ADDR_GET(PrmCopyV.BlockNo, PrmCopyV.PrmReadIndex), 
							LCD_MSG_DATA_GROUP_NUM );
			/* Header information deletion command response confirmation */
			PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_PRM_READ_RSP;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Command response confirmation																	*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_PRM_READ_RSP:
			/* Digiope communication command response confirmation process */
			rc = PrmCopyCmdRspCheck( LCD_CMD_LOAD );
			/*----- Receiving normally -----------------------------------------------------------------------*/
			if( rc == RC_OK )
			{
				/* Updating the read destination index(+8) */
				PrmCopyV.PrmReadIndex += LCD_MSG_DATA_GROUP_NUM;
				/* Processing type determination */
				if( PrmCopyV.SubMode == PRMCOPY_SUBMODE_LOAD )
				{
					/********************************************************************************/
					/*	Up to 8 parameters for one reception and Servopack parametersEEPROM 		*/
					/********************************************************************************/
					rc2 = PrmCopyLoadExePrmWrite();
				}
				else	/* PRMCOPY_SUBMODE_VERIFY */
				{
					/********************************************************************************/
					/* 	Up to 8 parameters for one reception 											*/
					/* 		Servopack parameters(EEPROM)andVERIFY							*/
					/********************************************************************************/
					rc2 = PrmCopyVerifyExePrmVerify();
				}
				/* Parameter writing complete */
				if( rc2 == RC_OK )
				{
					/* Communication retry count clear */
					PrmCopyV.RetryCnt = 0;
					/* Continue receiving next data */
					PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_PRM_READ_REQ;
					break;
				}
				/* All parameters written completed -> All parameters have been received.<S00?> */
				else if( rc2 == RC_COMPLETE )
				{
					if( PrmCopyV.SubMode == PRMCOPY_SUBMODE_LOAD )
					{
						/* All user parametersEEPROM */
						PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_STORE_REQ;
					}
					else
					{
						/* To process completion */
						PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_COMPLETE;
					}
				}
				/* Parameter write request failed */
				else if( rc2 == RC_NG )
				{
					/* Receive the same parameter data again */
					PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_PRM_READ_REQ;
					PrmCopyV.PrmReadIndex = PrmCopyV.PrmWriteIndex;
				}
			}
			/*----- Alarm code received -------------------------------------------------------*/
			else if( (rc == RC_NG) || (rc == RC_PASS) )
			{
				/* Receive the same parameter data again */
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_PRM_READ_REQ;
			}
			/*----- Number of retries exceeded -----------------------------------------------------------*/
			else if( rc == RC_RETRY_OVER )
			{
				/* Interrupt processing */
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_CANCEL;
			}
			/*----- Received command code is different from sent code -----------------------------------*/
			else
			{
				/* Continue waiting for reception */
			}
			break;
		/*==========================================================================================*/
		/*	2. Execution sequence is maintained to display the DigioOp execution completion screen or interruption screen for about 2 seconds		*/
		/*==========================================================================================*/
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope execution completion screen being displayed															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_COMPLETE:
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope interruption screen being displayed																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_CANCEL:
			/* Screen display first time */
			if( PrmCopyV.DispHoldTime == 0 )
			{
				/* Set parameter batch rewriting flag in parameter copy functionOFF */
				//RoutV.f.PrmCopy = FALSE;
				pAxRsc->FnCmnCtrl->f.PrmCopy = FALSE;			/* @@ chk */
				/* Reset display timer for 2 seconds */
				KpiRstLongTimer( &PrmCopyV.DispHoldTime );
				break;
			}
			/* Displayed for 2 seconds */
			if( KpiGetLongTimer(&PrmCopyV.DispHoldTime) > TIME_SECOND_2 )
			{
				CTRCMEM->FreeBuffer( CTRCMEM, &PrmCopyV );	/* @@ */
				PrmCopyV.DispHoldTime = 0;
				/* LOAD During processing */
				if( PrmCopyV.SubMode == PRMCOPY_SUBMODE_LOAD )
				{
//<S0AA>			KPI_WARNINGSET(WRN_BOOT);		/* A.941 : Power cycle required to change user constants			*/
					ALMSetServoAlarm( pAxRsc->AlmManager, WRN_BOOT );	/* <S0AA> A.941 : */
					/* After function initialization, return to block selection */
					PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_INIT;
				}
				else	/* VERIFY During processing */
				{
					/* In case of cancellation */
					if( PrmCopyV.ExeSeq == PRMCOPY_LOAD_SEQ_CANCEL )
					{
						/* After function initialization, return to block selection */
						PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_INIT;
					}
					else if( PrmCopyV.VerifyErrTotalCnt == 0 )	/* VERIFYResult All matches */
					{
						/* VERIFYResult Go to matching screen display */
						PrmCopyV.ExeSeq = PRMCOPY_VERIFY_SEQ_MATCHED;
					}
					else	/* VERIFYResult Unmatched */
					{
						PrmCopyV.VerifyErrListNo = 0;
						/* VERIFYGoing to displaying result mismatching screen */
						PrmCopyV.ExeSeq = PRMCOPY_VERIFY_SEQ_UNMATCHED;
					}
				}
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		VERIFYDisplaying result matching screen															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_VERIFY_SEQ_MATCHED:
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		VERIFYDisplaying result mismatching screen															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_VERIFY_SEQ_UNMATCHED:
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Store Parameters to Eeprom															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_STORE_REQ:
		/*------------------------------------------------------------------------------------------*/
			if ( KriRequestWriteLcdPdlEeprom( pAxRsc->pRwPdlMng, PrmCopyV.TraceBuffer, 
									(void*)&LcdPdlTbl[0], LCDPDLTBL_ENTNUM  ) == OK )
			{
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_STORE_WAIT;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Store Parameters to Eeprom															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_STORE_WAIT:
			if (  KriChkDoneWriteLcdPdlEeprom( pAxRsc->pRwPdlMng ) )
			{
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_COMPLETE;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		SAVE(Servo -> DgOp)Medium  KeyProcessing																*/
/*																									*/
/****************************************************************************************************/
void	PrmCopySaveKeyOp( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		case PRMCOPY_SAVE_SEQ_COMPLETE :	/* Digiope execution completion screen being displayed */
			break;
		default :
			/*-----	[MODE] key ---------------------------------------------------------------------*/
			if( OpKeyMk.mode )
			{
				/* Stop execution process */
				PrmCopyV.ExeSeq = PRMCOPY_SAVE_SEQ_CANCEL;
			}
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		LOAD(DgOp -> Servo)Medium  KeyProcessing																*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyLoadKeyOp( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		LOADWhen processing, reconfirming before execution															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_RECONFIRM:
			/*-----	[MODE] key ---------------------------------------------------------------------*/
			if( OpKeyMk.mode )
			{
				/* After function initialization, return to block selection */
				PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_INIT;
			}
			/*-----	[DATA] key ---------------------------------------------------------------------*/
			else if( OpKeyMk.data )
			{
				/* ParameterREADTo command request */
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_PRM_READ_REQ;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		VERIFY(Servopack <-> Digiope)Medium  KeyProcessing												*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyVerifyKeyOp( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		VERIFYResult Displaying matching screen															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_VERIFY_SEQ_MATCHED:
			/*-----	[MODE] key ---------------------------------------------------------------------*/
			if( OpKeyMk.mode )
			{
				/* After function initialization, return to block selection */
				PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_INIT;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		VERIFYResult While the mismatch screen is being displayed															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_VERIFY_SEQ_UNMATCHED:
			/*----- [MODE] key ---------------------------------------------------------------------*/
			if( OpKeyMk.mode )
			{
				/* After function initialization, return to block selection */
				PrmCopyV.SubModeSeq = PRMCOPY_SUBMODE_SEQ_INIT;
			}
			/*----- [↑] key -----------------------------------------------------------------------*/
			else if( OpKeyMk.up )
			{
				/* Parameter block display scroll up */
				if( PrmCopyV.VerifyErrListNo > 0 )
					PrmCopyV.VerifyErrListNo--;
			}
			/*----- [↓] key -----------------------------------------------------------------------*/
			else if( OpKeyMk.down )
			{
				/* Parameter block display scroll down */
				if( PrmCopyV.VerifyErrListNo + PRMCOPY_DISP_LIST_MAX < PrmCopyV.VerifyErrTblCnt )
					PrmCopyV.VerifyErrListNo++;
			}
			break;
		default:
			/*-----	[MODE] key ---------------------------------------------------------------------*/
			if( OpKeyMk.mode )
			{
				/* Display processing interruption message screen */
				PrmCopyV.ExeSeq = PRMCOPY_LOAD_SEQ_CANCEL;
			}
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Up to 8 parameters for one reception and Servopack parametersEEPROM						*/
/*		<MERCURY>EEPROMWrite queue is not used for processing												*/
/*																									*/
/****************************************************************************************************/
SHORT	PrmCopyLoadExePrmWrite(void)
{
USHORT	cmd_data_idx;
USHORT	dgop_prm_data;
/*--------------------------------------------------------------------------------------------------*/
/*		Processing up to 8 parameters for one reception														*/
/*--------------------------------------------------------------------------------------------------*/
		for( cmd_data_idx = 0; cmd_data_idx < LCD_MSG_DATA_GROUP_NUM; cmd_data_idx++ )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Get received parameters															*/
		/*------------------------------------------------------------------------------------------*/
			dgop_prm_data = PRMCOPY_CMD_DATA_GET( cmd_data_idx );
		/*------------------------------------------------------------------------------------------*/
			if (( PrmCopyV.PrmWriteIndex >= 2 ) && ( PrmCopyV.PrmWriteIndex >= PrmCopyV.TraceBuffer[ 1 ] ))
			{
				return RC_COMPLETE; 
			}
			PrmCopyV.TraceBuffer[ PrmCopyV.PrmWriteIndex ] = dgop_prm_data;
			PrmCopyV.PrmWriteIndex++;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( RC_OK );
}



/****************************************************************************************************/
/*																									*/
/*		Up to 8 parameters for one reception and Servopack parameters(EEPROM)andVERIFY		*/
/*																									*/
/****************************************************************************************************/
SHORT	PrmCopyVerifyExePrmVerify(void)
{
USHORT	cmd_data_idx;
USHORT	prm_data, dgop_prm_data;
//USHORT	nPrmVerify;	
/*--------------------------------------------------------------------------------------------------*/
/*		Processing 8 parameter data for one reception													*/
/*--------------------------------------------------------------------------------------------------*/
		for( cmd_data_idx = 0; cmd_data_idx < LCD_MSG_DATA_GROUP_NUM; cmd_data_idx++ )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Get parameter																	*/
		/*------------------------------------------------------------------------------------------*/
			/* Acquisition of servo side parameters */
			prm_data = PrmCopyV.TraceBuffer[ PrmCopyV.PrmWriteIndex ];
			/* Obtain parameters received from Digiope */
			dgop_prm_data = PRMCOPY_CMD_DATA_GET( cmd_data_idx );
		/*------------------------------------------------------------------------------------------*/
		/*		Get data count																		*/
		/*------------------------------------------------------------------------------------------*/
			if ( PrmCopyV.PrmWriteIndex == 1 )
			{
				PrmCopyV.VerifyNumber = MlibMIN( prm_data, dgop_prm_data );
			}
		/*------------------------------------------------------------------------------------------*/
		/*		OldPrmand newServo: OldPrmare not compared.							*/
		/*		NewPrmand oldServo: OldServoare not compared.							*/
		/*------------------------------------------------------------------------------------------*/
			else if ( PrmCopyV.PrmWriteIndex >= 2 ) 
			{
				//nPrmVerify = MlibMIN( PrmCopyV.TraceBuffer[1], LcdPdlTbl[1].PnNo );
				if ( PrmCopyV.PrmWriteIndex >= PrmCopyV.VerifyNumber /* nPrmVerify */)
				{
					return( RC_COMPLETE );				/* Complete all searches								*/
				}
			}
		/*------------------------------------------------------------------------------------------*/
		/*		Data mismatch																		*/
		/*------------------------------------------------------------------------------------------*/
			if (( LcdPdlTbl[ PrmCopyV.PrmWriteIndex ].pPrmDef != NULL ) && ( prm_data != dgop_prm_data ))
			{
				PrmCopyV.VerifyErrTotalCnt++;			/* VERIFYIncrement number of error occurrences		*/
				if( PrmCopyV.VerifyErrTblCnt < PRMCOPY_VERIFY_ERR_DISP_TBL_MAX )
				{
					PrmCopyV.VerifyErrPrmIdxTbl[PrmCopyV.VerifyErrTblCnt] = PrmCopyV.PrmWriteIndex;
					PrmCopyV.VerifyErrTblCnt++;
				}
			}
		/*------------------------------------------------------------------------------------------*/
			PrmCopyV.PrmWriteIndex++;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( RC_OK );
}



/****************************************************************************************************/
/*																									*/
/*		Function selection check processing																		*/
/*																									*/
/****************************************************************************************************/
SHORT	PrmCopyModeSelectCheck(void)
{
AXIS_HANDLE *pAxRsc = LopMsg.pAxRsc;
/*--------------------------------------------------------------------------------------------------*/
/*		Check whether function is executable														*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.SubMode )
		{
		/*------------------------------------------------------------------------------*/
		/*		SAVE(Servo -> DgOp)														*/
		/*------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_SAVE:
			break;
		/*------------------------------------------------------------------------------*/
		/*		DELETE(DgOp)															*/
		/*------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_DELETE:
			break;
		/*------------------------------------------------------------------------------*/
		/*		LOAD(DgOp -> Servo)														*/
		/*------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_LOAD:
			/* Error that cannot be executed while the servo is on	*/
			if( ! CHK_BB() /*RinV.f.BaseEnable*/ )
			{
				/* Go to execution error display screen */
				PrmCopyV.ErrorNo = PRMCOPY_ERROR_NOT_AVAILABLE;
				PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				return( RC_NG );
			}
			/* Cannot execute error while parameter writing is prohibited */
			if( CHK_WRITEINHIBIT() /* Iprm.f.PrmWrtInhibit */ && (LopMsg.AccessLvl != ACCLVL_SYSTEM) )
			{
				/* Go to execution error display screen */
				PrmCopyV.ErrorNo = PRMCOPY_ERROR_NOT_AVAILABLE;
				PrmCopyV.MainMode |= PRMCOPY_MAINMODE_ERROR_BIT;
				return( RC_NG );
			}
			pAxRsc->FnCmnCtrl->KnlMode = KPI_KMD_PARAMCPY;	/* @@ Executing Parameter Copy */
			break;
		/*------------------------------------------------------------------------------*/
		/*		VERIFY(Servo <-> DgOp)													*/
		/*------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_VERIFY:
			break;
		/*------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}



/****************************************************************************************************/
/*																									*/
/*		Error occurrence processing																			*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyErrorOp( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Error occurrence processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ErrorNo )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Execution not possible error																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_ERROR_NOT_AVAILABLE:
		/*------------------------------------------------------------------------------------------*/
		/*		Selection not possible error																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_ERROR_NO_DATA:
		/*------------------------------------------------------------------------------------------*/
		/*		Parameter type error																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_ERROR_PRM_UNMATCH:
			/* Screen display first time */
			if( PrmCopyV.DispHoldTime == 0 )
			{
				/* Timer reset */
				KpiRstLongTimer( &PrmCopyV.DispHoldTime );
			}
			/* Displayed for 2 seconds */
			if( KpiGetLongTimer(&PrmCopyV.DispHoldTime) > TIME_SECOND_2 )
			{
				/* 2 second screen display counter clear */
				PrmCopyV.DispHoldTime = 0;
				/* Error status clear */
				PrmCopyV.ErrorNo = PRMCOPY_ERROR_NONE;
				PrmCopyV.MainMode &= ~PRMCOPY_MAINMODE_ERROR_BIT;
				break;
			}
			return;
		/*------------------------------------------------------------------------------------------*/
		/*		No error occurred																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_ERROR_NONE:
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		User parameter search processing																	*/
/*																									*/
/****************************************************************************************************/
#if 0
SHORT	PrmCopyUsrPrmSearch( USHORT *SrchIdx, const PRMTBL *PrmTbl, const USHORT EntNum )
{
CPRMDEF	*PrmDefPtr;
ULONG	EntNumX = 2 * EntNum;
/*--------------------------------------------------------------------------------------------------*/
/*		Check LongPrm																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( LPX_GET_HiWdFlag( (*SrchIdx) ) )
		{
			return( RC_OK );						/* Applicable parameter exists (LongHighWord)			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Search for user parameters from all parameters												*/
/*--------------------------------------------------------------------------------------------------*/
		for(   ; *SrchIdx < EntNumX; LPX_INC_SrhIndex( (*SrchIdx) ) )
		{
			PrmDefPtr = LPX_GET_PrmDefPtr( (*SrchIdx) );
			if(	LPX_CHK_EEP_PRM( PrmDefPtr ) )
			{
				UCHAR  AccLvl = PrmDefPtr->AccessLevel;
				if( (AccLvl == ACCLVL_USER1) || (AccLvl == ACCLVL_USER2) )
				{
					return( RC_OK );				/* Applicable parameter exists							*/
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( RC_NG );							/* Applicable parameter not found(Complete all searches)				*/
}
#endif



/****************************************************************************************************/
/*																									*/
/*		HEX to character string conversion process																*/
/*																									*/
/****************************************************************************************************/
void	ConvertHexToStr( USHORT HexData, UCHAR *str, USHORT digit )
{
LONG	i,x;

/*--------------------------------------------------------------------------------------------------*/
/*		Check Digit																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( (digit == 0) || (8 < digit) )
		{
			return;
		}
		if( digit < 8 )
		{
			HexData &= ((0x1L << (digit*4)) -1);		/* Drop high-order digits	*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Convert to Hex String																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < digit; i++, str++ )
		{
			x = (HexData >> ((digit-1-i)*4)) & 0x0F;
			if( x < 0xA ){ *str = (UCHAR)('0' + x);}
			else		 { *str = (UCHAR)('A' + (x - 0xA));}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Screen display data creation process main																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*										 01234567890123456											*/
/*		Screen display area(17Digit X 5Line)		0															*/
/*										1															*/
/*										2															*/
/*										3															*/
/*										4															*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyDispMain( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Screen display data creation process																		*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.MainMode )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		When selecting a function																			*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_MAINMODE_MODE_SELECT:
			PrmCopyDispModeSelect();
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		When executing a function																			*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_MAINMODE_MODE_EXE:
			switch( PrmCopyV.SubModeSeq )
			{
				/*----------------------------------------------------------------------------------*/
				/*		Block selection																*/
				/*----------------------------------------------------------------------------------*/
				case PRMCOPY_SUBMODE_SEQ_BLOCK_SELECT:
					PrmCopyDispBlockSelect();
					break;
				/*----------------------------------------------------------------------------------*/
				/*		Execution start confirmation																*/
				/*----------------------------------------------------------------------------------*/
				case PRMCOPY_SUBMODE_SEQ_EXE_CONFIRM:
					PrmCopyDispExeConfirm();
					break;
				/*----------------------------------------------------------------------------------*/
				/*		Process execution																	*/
				/*----------------------------------------------------------------------------------*/
				case PRMCOPY_SUBMODE_SEQ_EXE:
					switch( PrmCopyV.SubMode )
					{
						/*--------------------------------------------------------------------------*/
						/*		SAVE(Servo -> DgOp)Executing											*/
						/*--------------------------------------------------------------------------*/
						case PRMCOPY_SUBMODE_SAVE:
							PrmCopyDispSaveExe();
							break;
						/*--------------------------------------------------------------------------*/
						/*		LOAD(DgOp -> Servo)Executing											*/
						/*--------------------------------------------------------------------------*/
						case PRMCOPY_SUBMODE_LOAD:
							PrmCopyDispLoadExe();
							break;
						/*--------------------------------------------------------------------------*/
						/*		VERIFY(Servo <-> DgOp)Executing										*/
						/*--------------------------------------------------------------------------*/
						case PRMCOPY_SUBMODE_VERIFY:
							PrmCopyDispVerifyExe();
							break;
						/*--------------------------------------------------------------------------*/
						/*		DELETE(DgOp)Executing													*/
						/*--------------------------------------------------------------------------*/
						case PRMCOPY_SUBMODE_DELETE:
							PrmCopyDispDeleteExe();
							break;
						/*--------------------------------------------------------------------------*/
						default:
							break;
					}
					break;
				/*----------------------------------------------------------------------------------*/
				default:
					break;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			PrmCopyDispError();		/* Error message screen Display data creation */
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Error message screen Display data creation process													*/
/*																									*/
/****************************************************************************************************/
void PrmCopyDispError(void)
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Error message display																		*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ErrorNo )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Execution not possible error																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_ERROR_NOT_AVAILABLE:
			/* Parameter copy main title on 1st line "-COPY-"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_1],
							(UCHAR *)PrmCopyDispTitleTbl[PRMCOPY_DISPTITLE_MAIN] );
			/* 4th line "Not Available"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_NOT_AVAILABLE] );
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Selection not possible error																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_ERROR_NO_DATA:
			/* Screen header display after parameter block selection */
			PrmCopyDispBlockHeader();
			/* 4th line "No Data"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_NO_DATA] );
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Parameter type error																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_ERROR_PRM_UNMATCH:
			/* Screen header display after parameter block selection */
			PrmCopyDispBlockHeader();
			/* On the third line "Unmatched"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_UNMATCHED_RIGHT] );
			/* 4th line "Parameters"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_RIGHT] );
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		No error occurred																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_ERROR_NONE:
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Function selection screen display data creation process															*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyDispModeSelect(void)
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter copy main title on 1st line "-COPY-" 										*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_1],
							(UCHAR *)PrmCopyDispTitleTbl[PRMCOPY_DISPTITLE_MAIN] );
/*--------------------------------------------------------------------------------------------------*/
/*		Function 1 on 2nd line "1:SERVO -> OP" 														*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_2],
							(UCHAR *)PrmCopyDispModeStrTbl[PRMCOPY_DISPMODE_SAVE] );
/*--------------------------------------------------------------------------------------------------*/
/*		Function 2 on the third line "2:OP -> SERVO" 														*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
							(UCHAR *)PrmCopyDispModeStrTbl[PRMCOPY_DISPMODE_LOAD] );
/*--------------------------------------------------------------------------------------------------*/
/*		Function 3 on 4th line "3:VERIFY" 															*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispModeStrTbl[PRMCOPY_DISPMODE_VERIFY] );
/*--------------------------------------------------------------------------------------------------*/
/*		Function 4 on the 5th line "4:DELETE" 															*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
							(UCHAR *)PrmCopyDispModeStrTbl[PRMCOPY_DISPMODE_DELETE] );
/*--------------------------------------------------------------------------------------------------*/
/*		Blink set																				*/
/*--------------------------------------------------------------------------------------------------*/
		TxBuf->DispLineBlink[PrmCopyV.SubMode+1] = TRUE;
/*--------------------------------------------------------------------------------------------------*/
		return;
}


/****************************************************************************************************/
/*																									*/
/*		Block selection screen display data creation process														*/
/*																									*/
/****************************************************************************************************/
void PrmCopyDispBlockSelect(void)
{
USHORT	i, block_list_idx;
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Display title by function on 1st line																*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_1],
						(UCHAR *)PrmCopyDispTitleTbl[PrmCopyV.SubMode+1] );
/*--------------------------------------------------------------------------------------------------*/
/*		From 2nd line to 5th line, block list "0X:SGDS-XXX" 									*/
/*--------------------------------------------------------------------------------------------------*/
		block_list_idx = PrmCopyV.BlockNo;
		for( i = 0; i < PRMCOPY_DISP_LIST_MAX; i++ )
		{
			PrmCopyMakeBlockDispLine( TxBuf->DispLine[i+1], block_list_idx );
			if( (++block_list_idx) == PRMCOPY_MNG_INFO_BLOCK_MAX )
			{
				block_list_idx = 0;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		1st line of block list display(Second line on screen)										*/
/*--------------------------------------------------------------------------------------------------*/
		TxBuf->DispLineBlink[LCD_DISP_LINE_2] = TRUE;
		TxBuf->DispLineBlink[LCD_DISP_LINE_3] = 			/* @@ 2 line blink when SGD7W connected */
				Lpx_ChkDoubleBlock( PrmCopyP.TypeID ); 		
	
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Execution start confirmation screen Display data creation process														*/
/*																									*/
/****************************************************************************************************/
void PrmCopyDispExeConfirm(void)
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Screen header display after parameter block selection													*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyDispBlockHeader();
/*--------------------------------------------------------------------------------------------------*/
/*			Process execution sequence																		*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.SubMode )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		SAVE(Servo -> DgOp)Executing															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_SAVE:
		/*------------------------------------------------------------------------------------------*/
		/*		VERIFY(Servo <-> DgOp)Executing														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_VERIFY:
			/* On the third line "Start :[READ]"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_START_READ] );
			/* 4th line "Return:[MODE]"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_RETURN_MODE] );
			/* Blink on the third line */
			TxBuf->DispLineBlink[LCD_DISP_LINE_3] = TRUE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		LOAD(DgOp -> Servo)Executing															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_LOAD:
			/* On the third line "Start :[WRITE]"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_START_WRITE] );
			/* 4th line "Return:[MODE]"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_RETURN_MODE] );
			/* Blink on the third line */
			TxBuf->DispLineBlink[LCD_DISP_LINE_3] = TRUE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Delete parameters(DgOp)Executing															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SUBMODE_DELETE:
			/* On the third line "FILE DELETE"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_FILE_DELETE] );
			/* 4th line "Start :[WRITE]"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_START_WRITE] );
			/* 5th line "Return:[MODE]"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_RETURN_MODE] );
			/* Blink 4th line */
			TxBuf->DispLineBlink[LCD_DISP_LINE_4] = TRUE;
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		ParameterSAVE(Servo -> DgOp)Running screen Display data creation process									*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyDispSaveExe(void)
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Screen header display after parameter block selection													*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyDispBlockHeader();
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_HEADER_DEL_REQ:	/* Header information deletion command request						*/
		case PRMCOPY_SAVE_SEQ_HEADER_DEL_RSP:	/* Header information deletion command response confirmation					*/
		case PRMCOPY_SAVE_SEQ_HEADER_SEND:		/* Header information registration command transmission						*/
		case PRMCOPY_SAVE_SEQ_HEADER_RSP:		/* Header information registration command response confirmation					*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_PRM_SEND:			/* Send parameter data transmission command					*/
		case PRMCOPY_SAVE_SEQ_PRM_RSP:			/* Confirm response to parameter data transmission command				*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_PRM_READ_REQ:		/* @@ Parameter dataEEPROMReading					*/
		case PRMCOPY_SAVE_SEQ_PRM_READ_WAIT:	/* @@ Parameter dataEEPROMReading					*/
		/*------------------------------------------------------------------------------------------*/
			if ( KpiGetLongTimer( &PrmCopyV.DispKeepTime ) < TIME_500MSEC )
			{
				/* On the third line "Reading"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_READING] );
				/* 4th line "Parameters.."  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_DOT] );
				/* 5th line "<<SERVO>>"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_SERVO] );
			}
			else if ( KpiGetLongTimer( &PrmCopyV.DispKeepTime ) < TIME_1000MSEC )
			{
				/* On the third line "Writing"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_WRITING] );
				/* 4th line "Parameters.."  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_DOT] );
				/* 5th line "<< OP >>"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_OP] );
			}
			else
			{
				KpiRstLongTimer( &PrmCopyV.DispKeepTime );
			}
			/* [Read]Key LED blinking */
			LopV.LedBlnkSw.read = TRUE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope execution completion screen being displayed															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_COMPLETE:
			/* 4th line "Completed"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_COMPLETED] );
			/* [Read]Key LED blinking stop */
			LopV.LedBlnkSw.read = FALSE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope interruption screen being displayed																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_CANCEL:
			/* 4th line "Canceled"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_CANCELED] );
			/* [Read]Key LED blinking stop */
			LopV.LedBlnkSw.read = FALSE;
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		ParameterLOAD(DgOp -> Servo)Running screen Display data creation process									*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyDispLoadExe(void)
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Screen header display after parameter block selection													*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyDispBlockHeader();
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		LOADWhen processing, reconfirming before execution														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_RECONFIRM:
			/* On the third line "Reconfirm"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_RECONFIRM] );
			/* 4th line "OK :[DATA]"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_OK_DATA] );
			/* 5th line "NO :[MODE]"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_NO_MODE] );
			/* Blink on the third line */
			TxBuf->DispLineBlink[LCD_DISP_LINE_3] = TRUE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/* 		ParameterREADCommand request															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_PRM_READ_REQ:
		/*------------------------------------------------------------------------------------------*/
		/* 		ParameterREADCommand response confirmation														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_PRM_READ_RSP:
			if ( KpiGetLongTimer( &PrmCopyV.DispKeepTime ) < TIME_500MSEC )
			{
				/* On the third line "Reading"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_READING] );
				/* 4th line "Parameters.."  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_DOT] );
				/* 5th line "<< OP >>"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_OP] );
			}
			else if ( KpiGetLongTimer( &PrmCopyV.DispKeepTime ) < TIME_1000MSEC )
			{
				/* On the third line "Writing"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_WRITING] );
				/* 4th line "Parameters.."  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_DOT] );
				/* 5th line "<<SERVO>>"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_SERVO] );
			}
			else
			{
				KpiRstLongTimer( &PrmCopyV.DispKeepTime );
			}
			/* [Write]Key LED blinking */
			LopV.LedBlnkSw.write = TRUE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope execution completion screen being displayed															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_COMPLETE:
			/* 4th line "Completed"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_COMPLETED] );
			/* [Write]Key LED blinking stop */
			LopV.LedBlnkSw.write = FALSE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope interruption screen being displayed																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_CANCEL:
			/* 4th line "Canceled"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_CANCELED] );
			/* [Write]Key LED blinking stop */
			LopV.LedBlnkSw.write = FALSE;
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		VERIFY(Servo <-> DgOp)Running screen Display data creation process										*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyDispVerifyExe(void)
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence isVERIFYResult When the mismatch screen is not being displayed									*/
/*--------------------------------------------------------------------------------------------------*/
		if( PrmCopyV.ExeSeq != PRMCOPY_VERIFY_SEQ_UNMATCHED )
		{
			/* Screen header display after parameter block selection */
			PrmCopyDispBlockHeader();
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*------------------------------------------------------------------------------------------*/
		/* 		ParameterREADCommand requesting														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_PRM_READ_REQ:
		/*------------------------------------------------------------------------------------------*/
		/* 		ParameterREADCommand response confirmation													*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_PRM_READ_RSP:
			if ( KpiGetLongTimer( &PrmCopyV.DispKeepTime ) < TIME_500MSEC )
			{
				/* On the third line "Reading"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_READING] );
				/* 4th line "Parameters.."  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_DOT] );
				/* 5th line "<< OP >>"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_OP] );
			}
			else if ( KpiGetLongTimer( &PrmCopyV.DispKeepTime ) < TIME_1000MSEC )
			{
				/* On the third line "Verifying"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_VERIFYING] );
				/* 4th line "Parameters.."  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_DOT] );
				/* 5th line "<<OP-SERVO>>"  */
				PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_5],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_OP_SERVO] );
			}
			else
			{
				KpiRstLongTimer( &PrmCopyV.DispKeepTime );
			}
			/* [Read]Key LED blinking */
			LopV.LedBlnkSw.read = TRUE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope execution completion screen being displayed															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_COMPLETE:
			/* 4th line "Completed"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_COMPLETED] );
			/* [Read]Key LED blinking stop */
			LopV.LedBlnkSw.read = FALSE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope interruption screen being displayed																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_LOAD_SEQ_CANCEL:
			/* 4th line "Canceled"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_CANCELED] );
			/* [Read]Key LED blinking stop */
			LopV.LedBlnkSw.read = FALSE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		VERIFYResult Displaying matching screen															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_VERIFY_SEQ_MATCHED:
			/* On the third line "All Parameters"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_ALL_PARAMETERS] );
			/* 4th line "  are matched"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_ARE_MATCHED] );
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		VERIFYResult While the mismatch screen is being displayed															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_VERIFY_SEQ_UNMATCHED:
			PrmCopyDispVerifyUnmatchList();
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		VERIFYResult Mismatch screen display data creation process													*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyDispVerifyUnmatchList(void)
{
LONG	i;
LONG	PrmIdx;
LONG	list_tbl_idx;
UCHAR	*disp_str_buf;
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Display title by function on 1st line																*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_1],
						(UCHAR *)PrmCopyDispTitleTbl[PrmCopyV.SubMode+1] );
/*--------------------------------------------------------------------------------------------------*/
/*		From 2nd line to 5th line, mismatch parameterNO												*/
/*--------------------------------------------------------------------------------------------------*/
		list_tbl_idx = PrmCopyV.VerifyErrListNo;
		for( i = 0; i < PRMCOPY_DISP_LIST_MAX; i++, list_tbl_idx++ )
		{
			/* There is data to display */
			if( list_tbl_idx < PrmCopyV.VerifyErrTblCnt )
			{
				disp_str_buf = TxBuf->DispLine[i+1];
				PrmIdx = PrmCopyV.VerifyErrPrmIdxTbl[list_tbl_idx];
				if( list_tbl_idx < (PRMCOPY_VERIFY_ERR_DISP_TBL_MAX - 1) )
				{
					/* "Pn"Set */
					MlibCopyByteMemory( (UCHAR *)&PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PN][0],
										(UCHAR *)disp_str_buf, 2 );
					/* When the mismatch parameter is user level 1 */
					if( LcdPdlTbl[PrmIdx].pPrmDef->AccessLevel == ACCLVL_USER1 )
					{
						/* Parameter number set */
						ConvertHexToStr( LcdPdlTbl[PrmIdx].PnNo, (disp_str_buf+2), 3 );
					}
					else
					{
						/* "***"Set */
						MlibCopyByteMemory( (UCHAR *)&PrmCopyDispStrTbl[PRMCOPY_DISPSTR_ASTER][0],
											(UCHAR *)(disp_str_buf+2), 3 );
					}
				}
				else if( list_tbl_idx == (PRMCOPY_VERIFY_ERR_DISP_TBL_MAX - 1) )
				{
					/* "To Many Errors"Set */
					MlibCopyByteMemory( (UCHAR *)&PrmCopyDispStrTbl[PRMCOPY_DISPSTR_TO_MANY_ERRORS][0],
										(UCHAR *)disp_str_buf, 14 );
				}
			}
		/*------------------------------------------------------------------------------------------*/
			if( i == 1 )
			{
				/* On the third line "Unmatched"  */
				PrmCopyStrCopy( &TxBuf->DispLine[i+1][7],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_UNMATCHED_LEFT] );
			}
			else if( i == 2 )
			{
				/* 4th line "Parameters"  */
				PrmCopyStrCopy( &TxBuf->DispLine[i+1][7],
								(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_LEFT] );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		DELETE(DgOp)Running screen Display data creation process													*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyDispDeleteExe(void)
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Screen header display after parameter block selection													*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyDispBlockHeader( );
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Header information deletion command request															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_HEADER_DEL_REQ:
		/*------------------------------------------------------------------------------------------*/
		/*		Header information deletion command response confirmation														*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_HEADER_DEL_RSP:
			/* On the third line "Deleting"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_DELETING] );
			/* 4th line "Parameters.."  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PARAMETERS_DOT] );
			/* [Write]Key LED blinking */
			LopV.LedBlnkSw.write = TRUE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope execution completion screen being displayed															*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_COMPLETE:
			/* 4th line "Completed"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_COMPLETED] );
			/* [Write]Key LED blinking stop */
			LopV.LedBlnkSw.write = FALSE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope interruption screen being displayed																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_SAVE_SEQ_CANCEL:
			/* 4th line "Canceled"  */
			PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_4],
							(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_CANCELED] );
			/* [Write]Key LED blinking stop */
			LopV.LedBlnkSw.write = FALSE;
			break;
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Screen header display after parameter block selection Header display data creation process										*/
/*																									*/
/****************************************************************************************************/
void PrmCopyDispBlockHeader( void )
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Display title by function on 1st line																*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_1],
						(UCHAR *)PrmCopyDispTitleTbl[PrmCopyV.SubMode+1] );
/*--------------------------------------------------------------------------------------------------*/
/*		Execution target block on 2nd line "0X:SGDS-XXX" 												*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyMakeBlockDispLine( TxBuf->DispLine[LCD_DISP_LINE_2], PrmCopyV.BlockNo );
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Digiope initial screen display data creation process														*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyDispInit( void )
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		On 2nd line "File List Loading" 															*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_2],
						(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_FILE_LIST_LOADING] );
/*--------------------------------------------------------------------------------------------------*/
/*		On the third line "Please Wait..." 															*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( TxBuf->DispLine[LCD_DISP_LINE_3],
						(UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_PLEASE_WAIT] );
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Block display data creation process																	*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyMakeBlockDispLine( UCHAR *StrBuf, LONG BlockIdx )
{
LONG	i;
LONG	ServoId;

/*--------------------------------------------------------------------------------------------------*/
/*		Line clear																				*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy( StrBuf, (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_SPACE] );
/*--------------------------------------------------------------------------------------------------*/
/*		BlockNOSet																			*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy2( &StrBuf, (UCHAR *)PrmCopyDispBlockStrTbl[ BlockIdx ] );
/*--------------------------------------------------------------------------------------------------*/
/*		When Servopack unmatched : "****"Set													*/
/*--------------------------------------------------------------------------------------------------*/
		if( PrmMngTbl[BlockIdx].FormatID == PRMCOPY_MNG_DATA_NONE )
		{
			PrmCopyStrCopy2( &StrBuf, (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_ASTER] );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Servopack product name set(Example"SGDS-")															*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < SVIDTBL_ENTRY_NUM; i++ )
		{
			if( (PrmMngTbl[BlockIdx].ServoID & 0x00FF) == SvIdTbl[i].Id )
			{
				ServoId = i;	/* Used in amplifier capacity display string set*/
				PrmCopyStrCopy2( &StrBuf, (UCHAR *)&SvIdTbl[i].Name );
				break;
			}
			else if( i >= (SVIDTBL_ENTRY_NUM - 1) )		/* Search mismatch : "????"Set			*/
			{
				ServoId = i;
				PrmCopyStrCopy2( &StrBuf, (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_QUESTION_4] );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		"-" Set																					*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyStrCopy2( &StrBuf, (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_HIFUN] );
/*--------------------------------------------------------------------------------------------------*/
/*		Amplifier capacity display string set																	*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < AMP_WATT_TBL_IDX_MAX; i++ )
		{
			if(( (PrmMngTbl[BlockIdx].AmpVolt & 0x000f) == AmpWattTbl[i].AmpVolt ) &&
			   ( PrmMngTbl[BlockIdx].AmpWatt == AmpWattTbl[i].AmpWatt ))
			{
				/*----------------------------------------------------------------------------------*/
				/*	Depending on product[W]Display and[Arms]Separate symbol display 										*/
				/*----------------------------------------------------------------------------------*/
				if( SvIdTbl[ServoId].Disp == WATT)			/* [W]Display								*/
				{
					PrmCopyStrCopy2( &StrBuf, (UCHAR *)&AmpWattTbl[i].DispStrW[0] );
				}
				else if( SvIdTbl[ServoId].Disp == AMPERE)	/* [Arms]Symbolic display						*/
				{
					PrmCopyStrCopy2( &StrBuf, (UCHAR *)&AmpWattTbl[i].DispStr[0] );
				}
				else										/* Others:"???"Set					*/
				{
					PrmCopyStrCopy2( &StrBuf, (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_QUESTION_3] );
				}
				break;
			}
			else if( i >= (AMP_WATT_TBL_IDX_MAX - 1) )		/* Search mismatch : "???"Set				*/
			{
				PrmCopyStrCopy2( &StrBuf, (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_QUESTION_3] );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Input voltage display string set																	*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < AMP_VOLT_TBL_IDX_MAX; i++ )
		{
			if( (PrmMngTbl[BlockIdx].AmpVolt & 0x000f) == AmpVoltTbl[i].Code )
			{
				PrmCopyStrCopy2( &StrBuf, (UCHAR *)&AmpVoltTbl[i].DispStr[0] );
				break;
			}
			else if( i >= (AMP_VOLT_TBL_IDX_MAX - 1) )		/* Search mismatch : "?"Set				*/
			{
				PrmCopyStrCopy2( &StrBuf, (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_QUESTION_1] );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Interface specification display string set														*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < SVIDTBL_ENTRY_NUM; i++ )
		{
			if( (PrmMngTbl[BlockIdx].ServoID & 0x00FF) == SvIdTbl[i].Id )
			{
				PrmCopyStrCopy2( &StrBuf, (UCHAR *)&SvIdTbl[i].IfTyp );
				break;
			}
			else if( i >= (SVIDTBL_ENTRY_NUM - 1) )		/* Search mismatch : "????"Set			*/
			{
				PrmCopyStrCopy2( &StrBuf, (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_QUESTION_4] );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		String copy processing																			*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyStrCopy( UCHAR *ToStr, UCHAR *FromStr )
{
		while( *FromStr != 0 )
		{
			*(ToStr++) = *(FromStr++);
		}
}



/****************************************************************************************************/
/*																									*/
/*		String copy processing(Update and return the copy destination address)											*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyStrCopy2( UCHAR **ToStr, UCHAR *FromStr )
{
		while( *FromStr != 0)
		{
			*((*ToStr)++) = *(FromStr++);
		}
}



/****************************************************************************************************/
/*																									*/
/*		Digiope parameter block management information acquisition process(Performed once when Digiope is connected)						*/
/*																									*/
/****************************************************************************************************/
LONG	RpxLcdPrmInfoGet( void )
{
LONG	i;
SHORT	rc;
LONG	ModeChg = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		PrmCopyCmdInit();						/* Digiope communication initialization process								*/
		PrmCopyNormalCmdSet();					/* Normal command to communication work area(0x60)Set			*/
		PrmCopyDispInit();						/* Digiope initial screen display data set					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Process execution sequence																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( PrmCopyV.ExeSeq )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Execution initial process																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_INFOGET_SEQ_INIT:
			/* Communication retry count clear */
			PrmCopyV.RetryCnt = 0;
			PrmCopyV.BlockNo = 0;
			PrmCopyV.BlockMngAreaNo = 0;
			/* Parameter block management informationREADTo command request */
			PrmCopyV.ExeSeq = PRMCOPY_INFOGET_SEQ_MNG_READ_REQ;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Digiope parameter block management informationLOADCommand request									*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_INFOGET_SEQ_MNG_READ_REQ:
			/* To communication work areaLOADCommand(0x61)Set */
			PrmCopyLoadMngInfoCmdSet( PrmCopyV.BlockNo, PrmCopyV.BlockMngAreaNo );
			/* Parameter block management informationREADTo command response confirmation */
			PrmCopyV.ExeSeq = PRMCOPY_INFOGET_SEQ_MNG_READ_RSP;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Command response confirmation																	*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_INFOGET_SEQ_MNG_READ_RSP:
			/* Digiope communication command response confirmation process */
			rc = PrmCopyCmdRspCheck( LCD_CMD_LOAD );
			/*----- Receiving normally -----------------------------------------------------------------------*/
			if( rc == RC_OK )
			{
				/* Communication retry count clear */
				PrmCopyV.RetryCnt = 0;
				/* Digiope Parameter block management information reception data acquisition */
				PrmCopyPrmMngTblOp( PRMCOPY_MNG_INFODATA_RECIEVE, PrmCopyV.BlockNo );
			  /*------------------------------------------------------------------------------------*/
				if( PrmCopyV.BlockMngAreaNo == 0 )
				{
					PrmCopyV.BlockMngAreaNo = 1;
				}
				else
				{
					PrmCopyV.BlockNo++;
					PrmCopyV.BlockMngAreaNo = 0;
				}
			  /*------------------------------------------------------------------------------------*/
				/* All parameter management information has been obtained */
				if( PrmCopyV.BlockNo >= PRMCOPY_MNG_INFO_BLOCK_MAX )
				{
					/* Management information acquisition complete */
					PrmCopyV.ExeSeq = PRMCOPY_INFOGET_SEQ_COMPLETE;
				}
				else
				{
					/* Continue transmitting next data */
					PrmCopyV.ExeSeq = PRMCOPY_INFOGET_SEQ_MNG_READ_REQ;
				}
			}
			/*----- Alarm code received -------------------------------------------------------*/
			else if( (rc == RC_NG) || (rc == RC_PASS) )
			{
				/* Digiope parameter block management informationREADReturn to command transmission */
				PrmCopyV.ExeSeq = PRMCOPY_INFOGET_SEQ_MNG_READ_REQ;
			}
			/*----- Number of retries exceeded -----------------------------------------------------------*/
			else if( rc == RC_RETRY_OVER )
			{
				/* Digiope Parameter block management information clear */
				for( i = 0; i < PRMCOPY_MNG_INFO_BLOCK_MAX; i++ )
				{
					PrmCopyPrmMngTblOp( PRMCOPY_MNG_INFODATA_CLEAR, i );
				}
				/* Interrupt processing */
				PrmCopyV.ExeSeq = PRMCOPY_INFOGET_SEQ_CANCEL;
			}
			/*----- Received command code is different from sent code -----------------------------------*/
			else
			{
				/* Continue waiting for reception */
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Execution completion																			*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_INFOGET_SEQ_COMPLETE:
		/*------------------------------------------------------------------------------------------*/
		/*		Execution interrupted																			*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_INFOGET_SEQ_CANCEL:
			ModeChg = TRUE;
			break;
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Termination processing																					*/
/*--------------------------------------------------------------------------------------------------*/
	 	PrmCopySetSendBuf();		/* Data set from communication work area to send buffer					*/
		PrmEntNum.SvPrm = LCDPDLTBL_ENTNUM; /* PrmCopyGetEntNum( &PnPrmTbl[0], KsysP.PnPrmTblEntNum ); */
/*--------------------------------------------------------------------------------------------------*/
		return( ModeChg );
}



/****************************************************************************************************/
/*																									*/
/*		Digiope communication command response confirmation process															*/
/*																									*/
/****************************************************************************************************/
SHORT	PrmCopyCmdRspCheck( USHORT SendCmd )
{
UCHAR	RecvCmd = PRMCOPY_CMD_CODE_GET( );				/* Reception command code acquisition					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Check Receive Cmd																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( (RecvCmd & LCD_RCVCMD_MASK) != SendCmd )	/* Reception command != Send command				*/
		{
			return( RC_PASS );
		}
		else if( (RecvCmd & LCD_RCVCMD_ALARM_BIT) )		/* Alarm bit is set				*/
		{
			if( PrmCopyV.RetryCnt > RETRY_CNT_MAX )
			{
				return( RC_RETRY_OVER );				/* Number of retries exceeded the allowable range			*/
			}
			else
			{
				PrmCopyV.RetryCnt++;					/* Communication retry count increment		*/
			}
			return( RC_NG );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( RC_OK );								/* Normal										*/
}



/****************************************************************************************************/
/*																									*/
/*		Digi operation parameter management information operation																*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyPrmMngTblOp( UCHAR flag, USHORT block_no )
{
PRMDATA *pPrm = LopMsg.pAxRsc->UniPrms->Prm;			/* TODO: Erase after */
/*--------------------------------------------------------------------------------------------------*/
/*		Switch by flag																				*/
/*--------------------------------------------------------------------------------------------------*/
		switch( flag )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Clear management information																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_MNG_INFODATA_CLEAR:
			PrmMngTbl[block_no].ServoID		= PRMCOPY_MNG_DATA_NONE;
			PrmMngTbl[block_no].AmpWatt		= PRMCOPY_MNG_DATA_NONE;
			PrmMngTbl[block_no].AmpVolt		= PRMCOPY_MNG_DATA_NONE;
			PrmMngTbl[block_no].FormatID 	= PRMCOPY_MNG_DATA_NONE;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Set management information																		*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_MNG_INFODATA_SET:
			PrmMngTbl[block_no].ServoID		= PrmCopyP.TypeID;
			PrmMngTbl[block_no].AmpWatt		= pPrm->ampw;
			PrmMngTbl[block_no].AmpVolt		= (USHORT)pPrm->styp.b.h;
			PrmMngTbl[block_no].FormatID 	= EEP_FORMAT_ID; /* EepInfo.YecFmtId; */
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Management information reception data acquisition																*/
		/*------------------------------------------------------------------------------------------*/
		case PRMCOPY_MNG_INFODATA_RECIEVE:
			if( PrmCopyV.BlockMngAreaNo == 0 )
			{
				PrmMngTbl[block_no].ServoID  = 0xFFFF;
				PrmMngTbl[block_no].AmpWatt  = PRMCOPY_CMD_DATA_GET( 0 );
				PrmMngTbl[block_no].AmpVolt  = PRMCOPY_CMD_DATA_GET( 1 );
				PrmMngTbl[block_no].FormatID = PRMCOPY_CMD_DATA_GET( 2 );
			/*------------------------------------------------------------------------------------*/
				if( PrmMngTbl[block_no].AmpWatt != 0xFFFF )
				{
					PrmMngTbl[block_no].ServoID = SVID_SGDS;
				}
			}
			else if( PrmMngTbl[block_no].ServoID == 0xFFFF )
			{
				if( PrmMngTbl[block_no].FormatID != 0xFFFF )
				{
					PrmMngTbl[block_no].ServoID  = PRMCOPY_CMD_DATA_GET( 0 );
					PrmMngTbl[block_no].AmpWatt  = PRMCOPY_CMD_DATA_GET( 1 );
					PrmMngTbl[block_no].AmpVolt  = PRMCOPY_CMD_DATA_GET( 2 );
					PrmMngTbl[block_no].FormatID = PRMCOPY_CMD_DATA_GET( 3 );
				}
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		default:
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Digiope communication initialization process																		*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyCmdInit( void )
{
LONG	i;
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Clear transmit buffer																			*/
/*--------------------------------------------------------------------------------------------------*/
		MlibResetByteMemory( (void *)LopMsg.ResBuf, sizeof(PRM_COPY_TXBUF) );
/*--------------------------------------------------------------------------------------------------*/
/*		Digiope communication work area initialization process																*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < PRMCOPY_DISP_LINE_MAX; i++ )
		{
			PrmCopyStrCopy( TxBuf->DispLine[i], (UCHAR *)PrmCopyDispStrTbl[PRMCOPY_DISPSTR_SPACE] );
			TxBuf->DispLineBlink[i] = FALSE;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Digiope communication  Normal command(0x60)Work area set												*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyNormalCmdSet( void )
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;
#define LCD_CMD_NORMAL_MSGLEN  LCD_TX_NORMAL_TOTAL_SIZE + LCD_MSG_CHKSUM_SIZE

/*--------------------------------------------------------------------------------------------------*/
/*		Header settings																					*/
/*--------------------------------------------------------------------------------------------------*/
		TxBuf->CmdCode = LCD_CMD_NOR;				/* Normal command(0x60)Set					*/
		TxBuf->Length  = LCD_CMD_NORMAL_MSGLEN;			/* Send buffer length set						*/
		TxBuf->DispSts[0] = 5;							/* Display format set							*/
/*--------------------------------------------------------------------------------------------------*/
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Digiope communication  LOADCommand(0x61)Work area set												*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyLoadCmdSet( USHORT DopEepAddr, USHORT DataNum )
{
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;
#define LCD_CMD_LOAD_MSGLEN  LCD_TX_MSG_LOADSUM_IDX + LCD_MSG_CHKSUM_SIZE

/*--------------------------------------------------------------------------------------------------*/
/*		Header settings																					*/
/*--------------------------------------------------------------------------------------------------*/
		TxBuf->CmdCode = LCD_CMD_LOAD;					/* LOADCommand(0x61)Set 				*/
		TxBuf->Length  = LCD_CMD_LOAD_MSGLEN;			/* Send message length set					*/
		TxBuf->EepAddr = DopEepAddr;					/* DigiopeEEPROMAddress set 			*/
		TxBuf->DataNum = DataNum;						/* Data count set 							*/
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Digiope communication  SAVECommand(0x62)Work area set												*/
/*																									*/
/****************************************************************************************************/
void	PrmCopySaveCmdSet( USHORT DopEepAddr, USHORT DataNum, USHORT *pData )
{
LONG	i;
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;
#define LCD_CMD_SAVE_DATLEN  (DataNum * LCD_MSG_DATA_UNIT_SIZE)
#define LCD_CMD_SAVE_MSGLEN  LCD_TX_MSG_DATA_IDX + LCD_MSG_CHKSUM_SIZE + LCD_CMD_SAVE_DATLEN

/*--------------------------------------------------------------------------------------------------*/
/*		Header settings																					*/
/*--------------------------------------------------------------------------------------------------*/
		TxBuf->CmdCode = LCD_CMD_SAVE;					/* SAVECommand(0x62)Set 				*/
		TxBuf->Length  = LCD_CMD_SAVE_MSGLEN;			/* Send message length set					*/
		TxBuf->EepAddr = DopEepAddr;					/* DigiopeEEPROMAddress set 			*/
		TxBuf->DataNum = DataNum;						/* Data count set 							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Data set(MAX:8)																			*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < DataNum; i++ )
		{
			TxBuf->Data[i] = *(pData++);
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Digiope communication : Set management information load command data												*/
/*																									*/
/****************************************************************************************************/
void	PrmCopyLoadMngInfoCmdSet( LONG BlockNo, LONG BlockMngAreaNo )
{
USHORT			DataNum, EepSize, EepBase;
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Management information area 0 : Existing management information(SGDSCompatibility)														*/
/*--------------------------------------------------------------------------------------------------*/
		if( BlockMngAreaNo == 0 )
		{
			DataNum = 3;										/* Load data count					*/
			EepSize = PRMCOPY_MNG_INFO_SIZ0;					/* EEPROMManagement information size				*/
			EepBase = PRMCOPY_MNG_INFO_TOP0;					/* EEPROMBase address				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Management information area 1: Extended management information(SVOSaddition)														*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			DataNum = 8;										/* Load data count					*/
			EepSize = PRMCOPY_MNG_INFO_SIZ1;					/* EEPROMManagement information size				*/
			EepBase = PRMCOPY_MNG_INFO_TOP1;					/* EEPROMBase address				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set transmit buffer data																		*/
/*--------------------------------------------------------------------------------------------------*/
		TxBuf->CmdCode = LCD_CMD_LOAD;							/* Load command set				*/
		TxBuf->EepAddr = (USHORT)(EepBase + (EepSize*BlockNo));	/* EEPROMAddress set				*/
		TxBuf->DataNum = DataNum;								/* Load data count set				*/
		TxBuf->Length  = LCD_TX_MSG_LOADSUM_IDX + LCD_MSG_CHKSUM_SIZE;
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Digiope communication : Management information save command data settings												*/
/*																									*/
/****************************************************************************************************/
void	PrmCopySaveMngInfoCmdSet( LONG BlockNo, LONG BlockMngAreaNo )
{
USHORT			DataNum, EepSize, EepBase;
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Management information area 0 : Existing management information(SGDSCompatibility)														*/
/*--------------------------------------------------------------------------------------------------*/
/*		Caution:SGDSbut"???"to displayAmpWatt,AmpVoltdoes not exist in0xFFFF					*/
/*--------------------------------------------------------------------------------------------------*/
		if( BlockMngAreaNo == 0 )
		{
			DataNum = 3;										/* Save data count						*/
			EepSize = PRMCOPY_MNG_INFO_SIZ0;					/* EEPROMManagement information size				*/
			EepBase = PRMCOPY_MNG_INFO_TOP0;					/* EEPROMBase address				*/
		/*------------------------------------------------------------------------------------------*/
			TxBuf->Data[0] = 0xFFFF;							/* Data set(MAX:8)				*/
			TxBuf->Data[1] = 0xFFFF;							/* Data set(MAX:8)				*/
			TxBuf->Data[2] = PrmMngTbl[BlockNo].FormatID;		/* Data set(MAX:8)				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Management information area 1: Extended management information(SVOSaddition)														*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			DataNum = 8;										/* Save data count						*/
			EepSize = PRMCOPY_MNG_INFO_SIZ1;					/* EEPROMManagement information size				*/
			EepBase = PRMCOPY_MNG_INFO_TOP1;					/* EEPROMBase address				*/
		/*------------------------------------------------------------------------------------------*/
			TxBuf->Data[0] = PrmMngTbl[BlockNo].ServoID;		/* Data set(MAX:8)				*/
			TxBuf->Data[1] = PrmMngTbl[BlockNo].AmpWatt;		/* Data set(MAX:8)				*/
			TxBuf->Data[2] = PrmMngTbl[BlockNo].AmpVolt;		/* Data set(MAX:8)				*/
			TxBuf->Data[3] = PrmMngTbl[BlockNo].FormatID;		/* Data set(MAX:8)				*/
			TxBuf->Data[4] = 0xFFFF;							/* Data set(MAX:8)				*/
			TxBuf->Data[5] = 0xFFFF;							/* Data set(MAX:8)				*/
			TxBuf->Data[6] = 0xFFFF;							/* Data set(MAX:8)				*/
			TxBuf->Data[7] = 0xFFFF;							/* Data set(MAX:8)				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Set transmit buffer data																		*/
/*--------------------------------------------------------------------------------------------------*/
		TxBuf->CmdCode = LCD_CMD_SAVE;							/* Save command set				*/
		TxBuf->EepAddr = (USHORT)(EepBase + (EepSize*BlockNo));	/* EEPROMAddress set				*/
		TxBuf->DataNum = DataNum;								/* Save data count set				*/
		TxBuf->Length  = LCD_TX_MSG_DATA_IDX
						 + (DataNum * LCD_MSG_DATA_UNIT_SIZE) + LCD_MSG_CHKSUM_SIZE;
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Data set processing from communication work area to send buffer											*/
/*																									*/
/****************************************************************************************************/
void	PrmCopySetSendBuf( void )
{
LONG	i;
PRM_COPY_TXBUF	*TxBuf = (PRM_COPY_TXBUF*)LopMsg.ResBuf;

/*--------------------------------------------------------------------------------------------------*/
/*		Command set																				*/
/*--------------------------------------------------------------------------------------------------*/
		LopV.CurMsgCmd = TxBuf->CmdCode;
/*--------------------------------------------------------------------------------------------------*/
/*		Screen display data set																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i = 0; i < PRMCOPY_DISP_LINE_MAX; i++ )
		{
			if( TxBuf->DispLineBlink[i] )
			{
				RpxLcdSetBlnk( i, 0, LCD_DISP_LINE_CHAR_MAX, (UCHAR *)TxBuf );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Response data endian conversion (TxBuf->Length/ChkSumisLcdOp.c)							*/
/*--------------------------------------------------------------------------------------------------*/
#if		defined( CCSW_BIG_ENDIAN )
		switch( TxBuf->CmdCode )
		{
		case LCD_CMD_LOAD:	/* Digiope EEPROMReading	*/
			 TxBuf->EepAddr = MlibCnvLittle16( TxBuf->EepAddr );
			 TxBuf->DataNum = MlibCnvLittle16( TxBuf->DataNum );
			 break;
		case LCD_CMD_SAVE:	/* Digiope EEPROMWrite	*/
			 for( i=0; i < TxBuf->DataNum; i++ )
			 {
				TxBuf->Data[i] = MlibCnvLittle16( TxBuf->Data[i] );
			 }
		   /*---------------------------------------------------------------------------------------*/
			 TxBuf->EepAddr = MlibCnvLittle16( TxBuf->EepAddr );
			 TxBuf->DataNum = MlibCnvLittle16( TxBuf->DataNum );
			 break;
		default:
			 break;
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Calculating the number of user level registrations from the parameter table											*/
/*																									*/
/****************************************************************************************************/
#if 0
USHORT	PrmCopyGetEntNum( const PRMTBL *PrmTbl, const USHORT EntNum )
{
LONG	i;
CPRMDEF	*pPrmDef;
USHORT	PrmNum = 0;

/*--------------------------------------------------------------------------------------------------*/
/*		Count User Parameter																		*/
/*--------------------------------------------------------------------------------------------------*/
		for ( i=0; i < EntNum; i++ )
		{
			pPrmDef = PrmTbl[i].PrmDefPtr;
			if(	LPX_CHK_EEP_PRM( pPrmDef ) )
			{
				ULONG	AccLvl = pPrmDef->AccessLevel;
				if( (AccLvl == ACCLVL_USER1) || (AccLvl == ACCLVL_USER2) )
				{
					if( pPrmDef->Attr.VarIsLong ){ PrmNum += 2;}else{ PrmNum += 1;}
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( PrmNum );
}
#endif



/****************************************************************************************************/
/*																									*/
/*		ParameterEEPROMRead processing																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview: the specified parametersEEPROMtoWORDSize1pieces.									*/
/*		  Reads the specified Parameter value from EEPROM.											*/
/*	Caution: There is no size specification from the upper level. Dedicated processing for the parameter copy function									*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	PrmCopyPrmRead( USHORT Index, const PRMTBL *PrmTbl, const USHORT EntNum, USHORT *ResData )
{
ULONG	PrmData;
CPRMDEF *pPrmDef;

/*--------------------------------------------------------------------------------------------------*/
/*		Check Index																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( Index >= (2 * EntNum) )
		{
			return( PERR_NO );				/* Parameter number error */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMData reading																		*/
/*--------------------------------------------------------------------------------------------------*/
		pPrmDef = LPX_GET_PrmDefPtr( Index );
		PrmData = KriReadEepromPrm( pPrmDef );
/*--------------------------------------------------------------------------------------------------*/
/*		EEPROMRead data setting																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( pPrmDef->Attr.VarIsLong != TRUE )
		{
			*ResData = (USHORT)PrmData;
		}
		else if( LPX_GET_HiWdFlag( Index ) == 0 )
		{
			*ResData = GET_LWD_EEPDATA( PrmData );
		}
		else
		{
			*ResData = GET_HWD_EEPDATA( PrmData );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}
#endif







/***************************************** end of file **********************************************/

/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SigmaMemobus.c : Memobus I/F Module															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Memobus message processing(MessageAnalysis,Read/WriteRegisters).							*/
/*																									*/
/*			 1) Loopback Test																		*/
/*			 2) System Download																		*/
/*			 3) Read  One Register																	*/
/*			 4) Write One Register																	*/
/*			 5) Read  Sequential Registers															*/
/*			 6) Write Sequential Registers															*/
/*			 7) Read  Random     Registers															*/
/*			 8) Write Random     Registers															*/
/*			 9) Others																				*/
/*																									*/
/*	Note Intention : This module is referenced from multiple locations.												*/
/*			Do not define static variables within this module.										*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2008.02.08  T.Taniguchi															*/
/*		Rev.1.01 : 2013.07.03  K.Ozaki																*/
/*		Rev.1.02 : 2013.09.26  K.Ozaki	<S00C>														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include 	"KnlApi.h" 							//#include "Round.h"
#include	"Monitor.h"							/* Un Monitor */
#include	"Parameter.h"						/* Parameter */
#include 	"Operation.h"						/* Fn Function */
#include 	"Information.h"						/* Information */
#include	"UsbFunctionIF.h"					/* TODO: Erase after */
#include	"Memory.h"							/* Data Trace */
#include	"FirmDownload.h"					/* FirmDownload(50h)	<S077>	*/
#include	"MemobusFunctions.h"				/* <S150> */

/* <S205>	Start */
/****************************************************************************************************/
/*																									*/
/*		VARIABLE																					*/
/*																									*/
/****************************************************************************************************/
/* ReadingIDregister addressTable */
const LONG IdRegAddTbl[MBUS_IDRD_SLOT_NUM][MBUS_IDRD_ID_NUM]=
{
						/* 	ID0(Model)	ID1(Specification)	ID2(Specification№)	ID3(Serial№)	ID4(Soft Ver)	ID5(Register Ver)	ID6(BTO №)	 */
/* 00:Servopack */	{	0x80019000,	0x80018000,	0,			0x80019010,		0x8001800A,		0x80018200,			0x800191E0	},
/* 01:CMD */			{	0x800190C8,	0,			0,			0x800190E0,		0x80018082,		0,					0			},
/* 02:Safty */			{	0x800190FA,	0,			0,			0x80019112,		0x8001808A,		0,					0			},
/* 03:FB */				{	0x8001912C,	0,			0,			0x80019144,		0,				0,					0			},
/* 04:Motor */			{	0x80019022,	0,			0,			0x80019038,		0,				0,					0			},
/* 05:Encoder */		{	0x8001904A,	0,			0,			0x8001905A,		0x8001801C,		0,					0			}
};

/* ReadingIDTable[byte] */
const CHAR IdDataSizeTbl[MBUS_IDRD_SLOT_NUM][MBUS_IDRD_ID_NUM]=
{
						/* 	ID0(Model)	ID1(Specification)	ID2(Specification№)	ID3(Serial№)	ID4(Soft Ver)	ID5(Register Ver)	ID6(BTO №)	 */
/* 00:Servopack */	{	2,			2,			4,			2,				4,				4,					2			},
/* 01:CMD */			{	2,			0,			0,			2,				4,				0,					0			},
/* 02:Safty */			{	2,			0,			0,			2,				4,				0,					0			},
/* 03:FB */				{	2,			0,			0,			2,				0,				0,					0			},
/* 04:Motor */			{	2,			0,			0,			2,				0,				0,					0			},
/* 05:Encoder */		{	2,			0,			0,			2,				2,				0,					0			}
};

/* ReadingIDTable */
const CHAR IdDataNumTbl[MBUS_IDRD_SLOT_NUM][MBUS_IDRD_ID_NUM]=
{
						/* 	ID0(Model)	ID1(Specification)	ID2(Specification№)	ID3(Serial№)	ID4(Soft Ver)	ID5(Register Ver)	ID6(BTO №)	 */
/* 00:Servopack */	{	8,			1,			1,			8,				1,				1,					32			},
/* 01:CMD */			{	8,			0,			0,			8,				1,				0,					0			},
/* 02:Safty */			{	12,			0,			0,			12,				1,				0,					0			},
/* 03:FB */				{	12,			0,			0,			12,				0,				0,					0			},
/* 04:Motor */			{	11,			0,			0,			8,				0,				0,					0			},
/* 05:Encoder */		{	8,			0,			0,			8,				1,				0,					0			}
};

/*  Number(0~9)ASCII code table of */
const	UCHAR	NumAsciiTbl[ ] =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

/*  Character ASCII code table */
const	UCHAR	StrAsciiTbl[ ] =
		{ 'F', 'E', 'Y'};
/* <S205>	End */

/****************************************************************************************************/
/*																									*/
/*		Function list																					*/
/*																									*/
/****************************************************************************************************/
extern SHORT	MbusFuncSystemDownLoad( HMSGIF Hmsg );	/*<S150>*/

/*--------------------------------------------------------------------------------------------------*/
/*		RpxFunction definition: Function name cannot be changed																	*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxMsgifMainProcedure( HMSGIF Hmsg );			/* MessageI/F: Main processing					*/
void	RpxMsgifFinishProcedure( HMSGIF Hmsg );			/* MessageI/F: End processing						*/
#if 0
LONG	RpxMsgifCheckRecvMessage( HMSGIF Hmsg );		/* MessageI/F: Received message check		*/
#endif
LONG	RpxMsgifGetCmdMsgSubAddr( HMSGIF Hmsg );		/* MessageI/F:CmdMsgGet sub address		*/
void	RpxMsgifSendErrResSubAddr( HMSGIF Hmsg );		/* MessageI/F:SubAddrAbnormal response transmission processing		*/
void	RpxMsgifFuncExecResponse( HMSGIF Hmsg );		/* MessageI/F: Auxiliary function execution result response processing		*/
/*--------------------------------------------------------------------------------------------------*/
/*		RkiFunction definition:Round API Function for Kernel													*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
LONG	RkiMsgifGetCmdMsgLength(						/* Command message length acquisition						*/
		UCHAR	*CmdBuf,								/* Command Message Buffer					*/
		LONG	CmdBufLen	);							/* Command Message Buffer Length			*/
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		LpxFunction definition: Function name can be changed																	*/
/*--------------------------------------------------------------------------------------------------*/
void	LpxMbusLoopbackTest( HMSGIF Hmsg );				/* Memobus: Loopback test				*/
#if 0
void	LpxMbusSystemDownLoad( HMSGIF Hmsg );			/* Memobus: System download			*/
#endif
#if 0	/* <S150> */
void	LpxMbusSysDLRes( HMSGIF Hmsg, SHORT dlSts );	/* Memobus: System download response creation*/
#endif
/*--------------------------------------------------------------------------------------------------*/
void	LpxMbusReadOneRegister( HMSGIF Hmsg );			/* Memobus: Single register read				*/
void	LpxMbusWriteOneRegister( HMSGIF Hmsg );			/* Memobus: Single register write				*/
/*--------------------------------------------------------------------------------------------------*/
void	LpxMbusReadSeqRegisters( HMSGIF Hmsg );			/* Memobus: Continuous register read				*/
void	LpxMbusWriteSeqRegisters( HMSGIF Hmsg );		/* Memobus: Continuous register write				*/
/*--------------------------------------------------------------------------------------------------*/
void	LpxMbusReadRandomRegisters( HMSGIF Hmsg );		/* Memobus: Discontinuous register read			*/
void	LpxMbusWriteRandomRegisters( HMSGIF Hmsg );		/* Memobus: Discontinuous register write			*/
/*--------------------------------------------------------------------------------------------------*/
void	LpxMbusReadMaxNumberOfRegisters( HMSGIF Hmsg );	/* Memobus: Read the maximum number of registers			*/
/*--------------------------------------------------------------------------------------------------*/
void	LpxMbusReadSeqMemories( HMSGIF Hmsg );			/* Memobus: Continuous memory read				*/
void	LpxMbusWriteSeqMemories( HMSGIF Hmsg );			/* Memobus: Continuous memory write				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxGetMaxNumberOfRdRegisters( HMSGIF Hmsg, LONG FixedLen );
LONG	LpxGetMaxNumberOfWrRegisters( HMSGIF Hmsg, LONG FixedLen );
/*--------------------------------------------------------------------------------------------------*/
void	LpxMbusSendErrorResponse( HMSGIF Hmsg, ULONG ErrCode );
LONG	LpxRegifRegNoError( LONG RegNo, LONG Number, USHORT *RdBuf, HMSGIF Hmsg );
/*--------------------------------------------------------------------------------------------------*/
#if 0	/* <S150> */
void	LpxMbusTstUsbProcedure( HMSGIF Hmsg );			/* IngramofMemobus							*/
void	LpxMbusTstUartProcedure( HMSGIF Hmsg );			/* IngramofMemobus							*/
#endif	/* <S150> */
/****************************************************************************************************/
/* <S150> >>>>> */
void LpxMbusMechatrolink( HMSGIF Hmsg, LONG *MsgLen );	/* mechatro link command					*/
void LpxMbusLink( HMSGIF Hmsg );						/* Relay message function				*/
static void	mbusFuncReadMemory42H( HMSGIF Hmsg, LONG SubFuncCode, LONG *TrnbLength );
static void	mbusFuncWriteMemory42H( HMSGIF Hmsg, LONG SubFuncCode, LONG *TrnbLength );
static void	mbusFuncReadMaxSize42H( HMSGIF Hmsg, LONG SubFuncCode, LONG *TrnbLength );
static void mbusFuncErrorManager42H( HMSGIF Hmsg, LONG ErrCode, ULONG ErrAddress, LONG *TrnbLength );

static LONG mbusFuncLongSwap( void *Buf );
static LONG mbusFuncShortSwap( void *Buf );

static void	mbusFuncSetArrayLongData( UCHAR *Buf, ULONG data, UINT AccessSize );
static void	mbusFuncSetArrayShortData( UCHAR *Buf, USHORT data, UINT AccessSize );

static LONG mbusFuncSetValueByteData( UCHAR *Buf, LONG BlockNum, LONG dIndex );
static LONG mbusFuncSetValueShortData( UCHAR *Buf, LONG BlockNum, LONG dIndex );
static LONG mbusFuncSetValueLongData( UCHAR *Buf, LONG BlockNum, LONG dIndex );

static REG_MANAGER_HANDLE *mbusFuncConvAddress(MEMOBUS_ADDR_MAP *AxisMap, ULONG *MemAddress );
static UCHAR mbusFuncCheckBlockAccess(UCHAR *CmdBuf, ULONG CurrentAddress, 
													UINT AccessSize, LONG BlockNum, LONG dIndex);
/* <<<<< <S150> */

/* <S205> Start */
/*--------------------------------------------------------------------------------------------------*/
/*	Product information reading																				*/
/*--------------------------------------------------------------------------------------------------*/
static void mbusFuncReadProductInfo42H( HMSGIF Hmsg, LONG SubFuncCode, LONG *TrnbLength );
static void mbusFuncSetSpecialSpecIdData( HMSGIF Hmsg, LONG RspBuffOffset);
static LONG mbusFuncSetIdData( HMSGIF Hmsg, LONG ReadIdRegAdd, UCHAR ReadDataSize, UCHAR ReadDataNum, LONG RspBuffOffset, LONG *TrnbLength );
/*--------------------------------------------------------------------------------------------------*/
/* <S205> End */

/*<11J42> add start*/
#if 0 &&  CCSW_CMDID == CMDID_EXIST
void	MemobusRespTrnsStart( HMSGIF, USHORT, USHORT );	/*             Transmission start setting					*/

ULONG	RpxSmsgTransOptRspMessage( HMSGIF );			/* Option card communication response data transfer processing	*/
void	MemobusTransCmdOptCmdMsg( HMSGIF );				/* CmdOpt: Command data transfer (TOOL→OPT)	*/
ULONG	MemobusTransCmdOptRspMsg( HMSGIF );				/* CmdOpt: Response data transfer (OPT→TOOL)	*/
#define	SMBUS_MPU_MASK		0xf0
#define	SMBUS_MPU0			0x00
#define	SMBUS_MPU1			0x10
#define	SMBUS_MPU2			0x20
#define	SMBUS_MPU3			0x30
#endif
/*<11J42> add end*/
/****************************************************************************************************/

/*--------------------------------------------------------------------------------------------------*/
/*		<S166>																						*/
/*--------------------------------------------------------------------------------------------------*/
static LONG mbusCheckOperationWriteHandle( REG_ACC_T *accessprm, MSGIF *Hmsg );
static void mbusFreeOperationWriteHandle( REG_ACC_T *accessprm, MSGIF *Hmsg );
static LONG	mbusGetOpeRegWriteHandle( OPACCSV *Accs, _enum_OPMODE mode, MSGIF *Hmsg );
static void	mbusFreeOpeRegWriteHandle( OPACCSV *Accs, _enum_OPMODE mode, MSGIF *Hmsg );
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter Register Read/Write																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxReadPrmRegister(					/* Parameter readout(Register specification)						*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of read data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Read data buffer									*/
		HMSGIF	Hmsg			);			/* ReadingIFHandle										*/
LONG	LpxWritePrmRegister(				/* Parameter writing process(Register specification)					*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of write data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Write data buffer								*/
		HMSGIF	Hmsg			);			/* ReadingIFHandle										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Common Object Register Read/Write															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxReadComObject(					/* Parameter readout(Register specification)						*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of read data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Read data buffer									*/
		HMSGIF	Hmsg			);			/* ReadingIFHandle										*/
LONG	LpxWriteComObject(					/* Parameter writing process(Register specification)					*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of write data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Write data buffer								*/
		HMSGIF	Hmsg			);			/* ReadingIFHandle										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Memory Object Register Read/Write															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxReadMemObject(					/* Parameter readout(Register specification)						*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of read data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Read data buffer									*/
		HMSGIF	Hmsg			);			/* ReadingIFHandle										*/
LONG	LpxWriteMemObject(					/* Parameter writing process(Register specification)					*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of write data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Write data buffer								*/
		HMSGIF	Hmsg			);			/* ReadingIFHandle										*/



/****************************************************************************************************/
/*																									*/
/*		Register IF function table																	*/
/*																									*/
/****************************************************************************************************/
#define	RFTBLIDX( RegNo )	((RegNo>>12) & 0x0F)			/* Get RegFuncTbl Index					*/
/*--------------------------------------------------------------------------------------------------*/
typedef	LONG	(*REGFUN)( LONG RegNo, LONG Number, USHORT *RdBuf, HMSGIF Hmsg );
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{
	REGFUN	ReadRegister;			/* Read  Register Function										*/
	REGFUN	WriteRegister;			/* Write Register Function										*/
//	UCHAR	MblkWchk;				/* @@ Multi-Block Write Check									*/
//	UCHAR	Spare;					/* @@ Spare														*/
	COBJECT	*pObject;				/* Pointer to the Register Object								*/
} REGFUNTBL;
/*--------------------------------------------------------------------------------------------------*/
const	REGFUNTBL	RegFuncTbl[16] = {
/*	ReadRegister			WriteRegister		 pObject 											*/
/*--------------------------------------------------------------------------------------------------*/
 {	LpxReadPrmRegister,		LpxWritePrmRegister, (COBJECT *)CPARAMX	},	/* 0 : TODO: Parameter Non Volatile		*/
 {	LpxReadPrmRegister,		LpxWritePrmRegister, (COBJECT *)CPARAMX	},	/* 1 : Parameter Volatile		*/
 {	LpxReadComObject,		LpxWriteComObject,	 (COBJECT *)COPEREG },	/* 2 : Operation Register	*/
 {	LpxRegifRegNoError,		LpxRegifRegNoError,	NULL },					/* 3 : TODO: Enginee.  Register	*/
 {	LpxReadMemObject,		LpxWriteMemObject,	(COBJECT *)CTRCMEM },	/* 4 : TODO: DataTrace Register	*/
 {	LpxReadMemObject,		LpxWriteMemObject,	(COBJECT *)CTRCMEM },	/* 5 : TODO: DataTrace Register	*/
 {	LpxReadMemObject,		LpxWriteMemObject,	(COBJECT *)CTRCMEM },	/* 6 : TODO: DataTrace Register	*/
 {	LpxReadMemObject,		LpxWriteMemObject,	(COBJECT *)CTRCMEM },	/* 7 : TODO: DataTrace Register	*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,	NULL },					/* 8 : Reserved				*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,	NULL },					/* 9 : Reserved				*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,	NULL },					/* A : Reserved				*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,	NULL },					/* B : Reserved				*/
 {	LpxReadComObject,		LpxRegifRegNoError,	(COBJECT *)CINFREG },	/* C : TypeInfo  Register	*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,	NULL },					/* D : ParamDef  Register	*/
 {	LpxReadComObject,		LpxRegifRegNoError,	(COBJECT *)CUNREGX },	/* E : UnMonitor Register	*/
 {	LpxRegifRegNoError,		LpxRegifRegNoError,	NULL },					/* F : Parameter Download(JPDRV)	*/
#if 0
 {	RpxReadPrmRegister,		RpxWritePrmRegister,	0x00, 0x00, },	/* 0 : Parameter Normal			*/
 {	RpxReadPrmRegister,		RpxWritePrmRegister,	0x00, 0x00, },	/* 1 : Parameter Temp.			*/
 {	RpxReadOpeRegister,		RpxWriteOpeRegister,	0x01, 0x00, },	/* 2 : Operation Register		*/
 {	RpxReadEngRegister,		RpxWriteEngRegister,	0x00, 0x00, },	/* 3 : Enginee.  Register		*/
 {	RpxReadTraceRegister,	RpxWriteTraceRegister,	0x00, 0x00, },	/* 4 : DataTrace Register		*/
 {	RpxReadTraceRegister,	RpxWriteTraceRegister,	0x00, 0x00, },	/* 5 : DataTrace Register		*/
 {	RpxReadTraceRegister,	RpxWriteTraceRegister,	0x00, 0x00, },	/* 6 : DataTrace Register		*/
 {	RpxReadTraceRegister,	RpxWriteTraceRegister,	0x00, 0x00, },	/* 7 : DataTrace Register		*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,		0x00, 0x00, },	/* 8 : Reserved					*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,		0x00, 0x00, },	/* 9 : Reserved					*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,		0x00, 0x00, },	/* A : Reserved					*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,		0x00, 0x00, },	/* B : Reserved					*/
 {	RpxReadTyInfRegister,	LpxRegifRegNoError,		0x00, 0x00, },	/* C : TypeInfo  Register		*/
 {	LpxRegifRegNoError,	    LpxRegifRegNoError,		0x00, 0x00, },	/* D : ParamDef  Register		*/
 {	RpxReadMonRegister,		LpxRegifRegNoError,		0x00, 0x00, },	/* E : UnMonitor Register		*/
 {	RpxReadPdlRegister,		RpxWritePdlRegister,	0x00, 0x00, },	/* F : Parameter Download		*/
#endif
};
/*--------------------------------------------------------------------------------------------------*/
#define Lpx_GetObject( regno )   (RegFuncTbl[ (regno >> 12) & 0x0F ].pObject)
#define OBJ_WRITE( obj, idx, data, hmsg ) ((obj)->Write( (obj), (idx), (data), hmsg ))
#define OBJ_WRITE_SUB( obj, idx, subidx, data, hmsg ) ((obj)->WriteS( (obj), (idx), (subidx), (data), hmsg ))
#define OBJ_READ( obj, idx, pdata, hmsg ) ((obj)->Read( (obj), (idx), (pdata), MCHK_REG_READ, hmsg ))  /* <S073> */
#define OBJ_READ_SUB( obj, idx, subidx, pdata, hmsg ) ((obj)->ReadS( (obj), (idx), (subidx), (pdata), MCHK_REG_READ, hmsg ))  /* <S073> */
#define OBJ_SEARCH( obj, num, pidx, hmsg ) ((obj)->Search( (obj), (num), 0, pidx, MCHK_REG_READ, hmsg ))
#define OBJ_SEARCH_SUB( obj, num, pidx, psubidx, hmsg )  \
		((obj)->SearchS( (obj), (num), 0, pidx, psubidx, MCHK_REG_READ, hmsg ))  /* <S073> */
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_BIG_ENDIAN == 1
#define GET_LWD_REGDATA( xlong )		(USHORT)((xlong)>>0 )
#define GET_HWD_REGDATA( xlong )		(USHORT)((xlong)>>16)
#define ULONGOF_REGDATA( lwd, hwd )		(ULONG)((((USHORT)(hwd))<<16) + (USHORT)(lwd))
#else
#define GET_LWD_REGDATA( xlong )		(USHORT)((xlong)>>0 )
#define GET_HWD_REGDATA( xlong )		(USHORT)((xlong)>>16)
#define ULONGOF_REGDATA( lwd, hwd )		(ULONG)((((USHORT)(hwd))<<16) + (USHORT)(lwd))
#endif

/****************************************************************************************************/
/*																									*/
/*		Define Macros, etc																			*/
/*																									*/
/****************************************************************************************************/
#define LPX_CHK_SLOT( SfcH )			(SfcH == 0x01)
#define LPX_GET_SLOT_BYTELEN( SfcH )	((LPX_CHK_SLOT( SfcH ))? 2 : 0)
#define LPX_GET_SLOT_WORDLEN( SfcH )	((LPX_CHK_SLOT( SfcH ))? 1 : 0)
/*--------------------------------------------------------------------------------------------------*/
/*	Test Macro																						*/
/*--------------------------------------------------------------------------------------------------*/
#undef MlibCnvBig16
#define MlibCnvBig16( x ) ( (__rev( x )>>16) )  /* ARMv5teethREV16No instruction */
#define MSGIF_SEND( resbuf, reslen )   ( Hmsg->Sif->StxRes( Hmsg->Sif, (resbuf), (reslen) ) )

/*--------------------------------------------------------------------------------------------------*/
#define Lpx_GetMaxSubIdx( tbl, idx )  /* <S073> */\
			( (GET_OBJDEF( tbl, idx )->DataLength * GET_OBJDEF( tbl, idx )->NumberOfArray)>>1)


/****************************************************************************************************/
/*																									*/
/*		MessageI/F : Main Procedure																	*/
/*																									*/
/****************************************************************************************************/
void	RpxMsgifMainProcedure( HMSGIF Hmsg )
{
		LONG	MsgLen;									/* <S150> */
/*--------------------------------------------------------------------------------------------------*/
/*		Reset Hmsg Variables																		*/
/*--------------------------------------------------------------------------------------------------*/
		Hmsg->FunSts    = 0;							/* Auxiliary function execution result							*/
		Hmsg->LstErrRno = Hmsg->ErrRno;					/* Abnormal register number previous value					*/
		Hmsg->ErrRno    = 0xFFFF;						/* Abnormal register number reset					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Check Function Code and Call the Function													*/
/*--------------------------------------------------------------------------------------------------*/
		switch( Hmsg->CmdBuf[1] )
		{
		case MFC_LOOPBACK:
			 LpxMbusLoopbackTest( Hmsg );
			 break;
		case MFC_SVXMSG16:
#if 0 && CCSW_CMDID == CMDID_EXIST		/*<11J42>*/
			 if( (Hmsg->CmdBuf[2] == 0x01) && ((Hmsg->CmdBuf[4] & SMBUS_MPU_MASK) == SMBUS_MPU1))
			 {
				switch( Hmsg->CmdBuf[3] )
				{
				case SFC_RD_ONEREG:
				case SFC_WR_ONEREG:
				case SFC_RD_SEQREG:
				case SFC_WR_SEQREG:
				case SFC_RD_RANREG:
				case SFC_WR_RANREG:
				case SFC_RD_MAXNUM:
					MemobusTransCmdOptCmdMsg( Hmsg );
					break;
				default: LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_FNCDERR ); break;
				}
				break;
			 }
#endif
			 switch( Hmsg->CmdBuf[3] )
			 {
			 case SFC_RD_ONEREG: LpxMbusReadOneRegister( Hmsg ); break;
			 case SFC_WR_ONEREG: LpxMbusWriteOneRegister( Hmsg ); break;
			 case SFC_RD_SEQREG: LpxMbusReadSeqRegisters( Hmsg ); break;
			 case SFC_WR_SEQREG: LpxMbusWriteSeqRegisters( Hmsg ); break;
			 case SFC_RD_RANREG: LpxMbusReadRandomRegisters( Hmsg ); break;
			 case SFC_WR_RANREG: LpxMbusWriteRandomRegisters( Hmsg ); break;
			 case SFC_RD_MAXNUM: LpxMbusReadMaxNumberOfRegisters( Hmsg ); break;
			 default: LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_FNCDERR ); break;
			 }
			 break;
		case MFC_SVXMSG32:
			 switch( Hmsg->CmdBuf[3] )
		 	 {
			 case SFC_RD_SEQMEM: LpxMbusReadSeqMemories( Hmsg ); break;
			 case SFC_WR_SEQMEM: LpxMbusWriteSeqMemories( Hmsg ); break;
			 default: LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_FNCDERR ); break;
			 }
			 break;
		case MFC_MECHATROLINK:
#if 0	/*<S150>*/
			 if( Hmsg->f.UsbMsgIF ) { LpxMbusTstUsbProcedure( Hmsg ); }
			 else { LpxMbusTstUartProcedure( Hmsg ); /* TODO: */ }
#else
			 LpxMbusMechatrolink( Hmsg, &MsgLen );
			 /* Data transmission */
			 MSGIF_SEND( Hmsg->ResBuf, MsgLen );
#endif
			 break;
//#if (SVFSEL_SWDOWNLOAD != 0)
		case MFC_SYSDWNLD:
#if 0	/*<S150>*/
//			 LpxMbusSystemDownLoad( Hmsg );
			{
				MEMOBUS_IF *MbusIf;
				SHORT dlSts;
				MbusIf = (MEMOBUS_IF*)MbusFuncSysDLHmsg2MbusIf(Hmsg);
				dlSts = MbusFuncSystemDownLoad(MbusIf);
//				MbusFuncSysDLMbusIf2Hmsg(Hmsg);
				if( dlSts != MBUS_BUSY )		/* BUSY */
				{
//<S073>			LpxMbusSysDLRes( Hmsg, dlSts );
					MlibSetCRC16MB( Hmsg->ResBuf, MbusIf->TrnbLength/* <S073> <S077> */ );
					MSGIF_SEND( Hmsg->ResBuf, MbusIf->TrnbLength/*@@ CRC*/ );			/* <S073> <S077> */
				}
			}
#else
			 MbusFuncSystemDownLoad( Hmsg );
#endif
			 break;
//#endif
		case MFC_RELAY:
#if 0	/*<S150>*/
			 if( Hmsg->f.UsbMsgIF ) { LpxMbusTstUsbProcedure( Hmsg ); }
			 else{ LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_FNCDERR ); /*TODO:*/ }
#else
			 LpxMbusLink( Hmsg );
#endif
			 break;

		default:
			 LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_FNCDERR );
			 break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		MessageI/F : Command message length acquisition(for Kernel\Driver\SifMbus.c)								*/
/*																									*/
/****************************************************************************************************/
#define LPX_GET_CMDMSGLEN( i, Mul, Add )\
		if( CmdBufLen < (i+2) ){ return( 0 );}\
		else{ return( (((CmdBuf[i]<<8) + CmdBuf[i+1]) * Mul) + Add );}
/*--------------------------------------------------------------------------------------------------*/
#if 0
LONG	RkiMsgifGetCmdMsgLength( UCHAR *CmdBuf, LONG CmdBufLen )
{
LONG	s = LPX_GET_SLOT_BYTELEN( CmdBuf[2] );
/*--------------------------------------------------------------------------------------------------*/
/*		Check CmdBufLen																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( CmdBufLen < 4 )
		{
			return( 0 );								/* CmdBufLen is Short						*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Check Function Code and Calculate CmdMsgLen													*/
/*--------------------------------------------------------------------------------------------------*/
		switch( CmdBuf[1] )
		{
		case MFC_LOOPBACK:
			 return( 8 );
		case MFC_SVXMSG16:
			 switch( CmdBuf[3] )
			 {
			 case SFC_RD_ONEREG: return(  8 + s );
			 case SFC_WR_ONEREG: return( 10 + s );
			 case SFC_RD_SEQREG: return( 10 + s );
			 case SFC_WR_SEQREG: LPX_GET_CMDMSGLEN( 6 + s, 2, 12 + s );
			 case SFC_RD_RANREG: LPX_GET_CMDMSGLEN( 4 + s, 2,  8 + s );
			 case SFC_WR_RANREG: LPX_GET_CMDMSGLEN( 4 + s, 4, 10 + s );
			 case SFC_RD_MAXNUM: return(  8 + s );
			 default: return( -1 );
			 }
			 /* unreachable return( -1 ); */
		case MFC_SVXMSG32:
			 switch( CmdBuf[3] )
			 {
			 case SFC_RD_SEQMEM: return( 12 + s );
			 case SFC_WR_SEQMEM: LPX_GET_CMDMSGLEN( 8 + s, 2, 12 + s );
			 default: return( -1 );
			 }
			 /* unreachable return( -1 ); */
		case MFC_SYSDWNLD:								/* @@@ Add Later @@@						*/
			 return( -1 );
		default:
			 if( KPI_CHK_OPEMSG( CmdBuf[1] ) )
			 {
				return( CmdBuf[2] + (CmdBuf[3]<<8) );
			 }
			 return( -1 );
		}
/*--------------------------------------------------------------------------------------------------*/
		/* unreachable return( -1 ); */									/* Unknown Function Code					*/
}
#endif



/****************************************************************************************************/
/*																									*/
/*		MessageI/F : Message IF termination processing															*/
/*																									*/
/****************************************************************************************************/
void	RpxMsgifFinishProcedure( HMSGIF Hmsg )
{
		RpxFreeOpeRegWriteHandleAll( Hmsg );	/* <S166> Forcibly release operation register write right process			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		MessageI/F : Received message check															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Check the received message.														*/
/*																									*/
/*			1) Received message length check															*/
/*			2) Receive message sum check															*/
/*																									*/
/*			Note) Checking the axis address is performed using the communication driver.										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	RpxMsgifCheckRecvMessage( HMSGIF Hmsg )
{
LONG	MsgLen = Hmsg->CmdMsgLen;
/*--------------------------------------------------------------------------------------------------*/
/*		SigmaMessage (LCD-Operater)																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( KPI_CHK_OPEMSG( Hmsg->CmdBuf[1] ) )
		{
			if( MsgLen & 0x0001 ){ return( NG );}
			return( MlibChkSum16SM( Hmsg->CmdBuf, MsgLen ) );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		SigmaMemobus (PC, etc)																		*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			return( MlibChkCRC16MB( Hmsg->CmdBuf, MsgLen ) );
		}
}
#endif


/****************************************************************************************************/
/*																									*/
/*		MessageI/F : CmdMsgGet sub address															*/
/*																									*/
/****************************************************************************************************/
LONG	RpxMsgifGetCmdMsgSubAddr( HMSGIF Hmsg )
{
UCHAR	*CmdBuf = Hmsg->CmdBuf;
#ifdef MCCSW_JPDRVMX
#define CSUBADDR_IDX (4)			/* Jupiter Multi Axis Drive : CmdBuf[4] */
#define CSUBADDR_OFS (0)			/* 01H: 1st axis / 02H: 2nd axis */
#else
#define CSUBADDR_IDX (5)			/* Mercury : CmdBuf[5] */
#define CSUBADDR_OFS (1)			/* 00H: 1st axis / 01H: 2nd axis */
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Check Function Code																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch( CmdBuf[1] )
		{
		case MFC_LOOPBACK:
			 return( -1 );
		case MFC_SVXMSG16:
			 if( CmdBuf[2] == 0x01 ){ return( CmdBuf[CSUBADDR_IDX] + CSUBADDR_OFS );}
			 break;
		case MFC_SVXMSG32:
			 if( CmdBuf[2] == 0x01 ){ return( CmdBuf[CSUBADDR_IDX] + CSUBADDR_OFS );}
			 break;
		case MFC_SYSDWNLD:
			 if( CmdBuf[2] == 0x01 ){ return( CmdBuf[CSUBADDR_IDX] + CSUBADDR_OFS );}
			 break;
		default:
			 break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return( -2 );
}



/****************************************************************************************************/
/*																									*/
/*		MessageI/F : SubAddrAbnormal response transmission processing														*/
/*																									*/
/****************************************************************************************************/
void	RpxMsgifSendErrResSubAddr( HMSGIF Hmsg )
{
		LpxMbusSendErrorResponse( Hmsg, SMBUS_SEC_ACCSERR );
		return;
}



/****************************************************************************************************/
/*																									*/
/*		MessageI/F : Auxiliary function execution result response processing														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : In the case of a command message to execute an auxiliary function, the process is not completed until the auxiliary function execution is completed.			*/
/*			A response message cannot be created immediately because a processing error cannot be determined.					*/
/*			This function is called from the auxiliary function management when the execution of the auxiliary function is completed, and the auxiliary function			*/
/*			Create a response message based on the execution result.												*/
/*																									*/
/****************************************************************************************************/
void	RpxMsgifFuncExecResponse( HMSGIF Hmsg )
{
LONG	i,x;
LONG	ResLen;
USHORT	*wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT	*wResBuf = (USHORT *)(Hmsg->ResBuf);
/*--------------------------------------------------------------------------------------------------*/
/*		When an auxiliary function execution process is abnormal																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( (Hmsg->FunSts != REGIF_COMPLETE) && (Hmsg->FunSts != REGIF_CONTINUE) )
		{
			LpxMbusSendErrorResponse( Hmsg, Hmsg->FunSts );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Response message byte count calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
		ResLen = (Hmsg->CmdBuf[1] == MFC_SVXMSG16)? 10 : 12;
		ResLen = ResLen + LPX_GET_SLOT_BYTELEN( Hmsg->CmdBuf[2] );
/*--------------------------------------------------------------------------------------------------*/
/*		Create register write completion response																	*/
/*--------------------------------------------------------------------------------------------------*/
		for( x=(ResLen>>1)-1, i=0 ; i < x ; i++ )
		{
			wResBuf[i] = wCmdBuf[i];					/* Set Response Data						*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		MlibSetCRC16MB(  Hmsg->ResBuf, ResLen );		/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, ResLen );		/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Loopback test																	*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusLoopbackTest( HMSGIF Hmsg )
{
ULONG	i;
USHORT	*wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT	*wResBuf = (USHORT *)(Hmsg->ResBuf);
/*--------------------------------------------------------------------------------------------------*/
/*		Response data creation																				*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < (8/2); i++ )
		{
			wResBuf[i] = wCmdBuf[i];					/* Set LoopBack Response					*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
/*		MlibSetCRC16MB(  Hmsg->ResBuf, 8 ); */			/* CRC-16No need to calculate						*/
		MSGIF_SEND( Hmsg->ResBuf, 8 );				/* Start Transmit Response Message			*/
		return;
}


#if 0	/* <S150> */
#if (SVFSEL_SWDOWNLOAD != 0)
/****************************************************************************************************/
/*																									*/
/*		Memobus: System download																*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusSystemDownLoad( HMSGIF Hmsg )
{

LONG	errorFlg = FALSE;
USHORT	*CmdBufW = (USHORT *)(Hmsg->CmdBuf);
USHORT	*ResBufW = (USHORT *)(Hmsg->ResBuf);
USHORT	usDmy=1;
USHORT	TrnsLen;
//Jpdrv	MBUSFUNCTBL		MbusFunc = MbusFuncTbl[ FuncCodeIdx ];
USHORT	SubFuncCode = (((UCHAR)Hmsg->CmdBuf[2] << 8) | (UCHAR)Hmsg->CmdBuf[3]);
/*--------------------------------------------------------------------------------------------------*/
/*		Download command analysis & response																*/
/*--------------------------------------------------------------------------------------------------*/
		switch ( SubFuncCode & 0x00FF )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Get download conditions																*/
		/*------------------------------------------------------------------------------------------*/
		  case	SFC_DLID_RD :
				usDmy	= KsysP.SysDlFlushNum;					/* FlashNumber							*/
				ResBufW[0]	= CmdBufW[0];						/* AxisAdr, FunctionCode			*/
				ResBufW[1]	= CmdBufW[1];						/* SubFunctionCode					*/
				ResBufW[2]	= CmdBufW[2];						/* MPUNumber							*/
				ResBufW[3]	= (USHORT)MlibByteSwap16( (LONG )KsysP.SysDlFlushNum );	/* FlushNumber		*/
				ResBufW[4]	= (USHORT)MlibByteSwap16( (LONG )KsysP.SysDlSts1 );	/* DownLoad Sts1	*/
				ResBufW[5]	= (USHORT)MlibByteSwap16( (LONG )KsysP.SysDlSts2 );	/* DownLoad Sts2	*/
				ResBufW[6]	= (USHORT)MlibByteSwap16( (LONG )KsysP.SysDlSts3 );	/* DownLoad Sts3	*/
				ResBufW[7]	= (USHORT)MlibByteSwap16( (LONG )KsysP.SysDlSts4 );	/* DownLoad Sts4	*/
				ResBufW[8]	= (USHORT)MlibByteSwap16( (LONG )KsysP.DriveID   );	/* ServoID			*/
				ResBufW[9]	= (USHORT)MlibByteSwap16( (LONG )BootSw.ExtRomSizeKB );	/* Flash #0 Manu ID	*/
				ResBufW[10]	= 0;								/* Flash #0 Device ID				*/
				TrnsLen = 24;
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		Soft version reading																*/
		/*------------------------------------------------------------------------------------------*/
		  case	SFC_SWVER_RD :
				ResBufW[0] = CmdBufW[0];						/* AxisAdr, FunctionCode			*/
				ResBufW[1] = CmdBufW[1];						/* SubFunctionCode					*/
				ResBufW[2] = CmdBufW[2];						/* MPUNumber							*/
				ResBufW[3] = 0x0200;							/* VerNumber of definitions=2					*/
				ResBufW[4] = (USHORT)MlibByteSwap16((LONG )KsysP.Bt2MswVersion );	/* Boot2 Ver	*/
				ResBufW[5] = (USHORT)MlibByteSwap16((LONG )DrvSw.MswVersion ); 		/* Runtime Ver	*/
				TrnsLen = 14;
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		Get download conditions																*/
		/*------------------------------------------------------------------------------------------*/
		  case	SFC_DLWAY_RD :	/* Get download write conditions */
				ResBufW[0] = CmdBufW[0];						/* AxisAdr, FunctionCode			*/
				ResBufW[1] = CmdBufW[1];						/* SubFunctionCode					*/
				ResBufW[2] = CmdBufW[2];						/* MPUNumber							*/
				ResBufW[3] = (USHORT)MlibByteSwap16((LONG )KsysP.SysDlWay );	/* Download method	*/
				ResBufW[4] = (USHORT)MlibByteSwap16((LONG )KsysP.SysDlPrgbLen );/* 1MsgPRGWriting size	*/
				TrnsLen = 12;
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		Download start request																*/
		/*------------------------------------------------------------------------------------------*/
		  case	SFC_DL_START :	/* Download start request */
				RpxSetDlMode( Hmsg );							/* Download start					*/
				/*----------------------------------------------------------------------------------*/
				/* Once you start, you won't be coming back here..Abnormal response if start cannot be performed(Inconsistency error) 			*/
				/*----------------------------------------------------------------------------------*/
				LpxMbusSendErrorResponse( Hmsg, REGIF_DAMTHERR2 );
				errorFlg = TRUE;
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		Read download history																*/
		/*------------------------------------------------------------------------------------------*/
		  case	SFC_DL_HIS_RD :
				ResBufW[0] = CmdBufW[0];						/* AxisAdr, FunctionCode			*/
				ResBufW[1] = CmdBufW[1];						/* SubFunctionCode					*/
				ResBufW[2] = CmdBufW[2];						/* MPUNumber							*/
				ResBufW[3] = (USHORT)MlibByteSwap16((LONG )KsysP.SwUpDateHis );	/* Download history	*/
				TrnsLen = 10;
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		Unsupported subfunction														*/
		/*		Commands valid only after download start request											*/
		/*			→Condition error																	*/
		/*------------------------------------------------------------------------------------------*/
		  default:
				LpxMbusSendErrorResponse( Hmsg, REGIF_DAMTHERR2 );
				errorFlg = TRUE;
				break;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Normal response transmission																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( errorFlg == FALSE )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	CRC-16Creation & response data transmission command															*/
		/*------------------------------------------------------------------------------------------*/
			MlibSetCRC16MB( Hmsg->ResBuf, TrnsLen );
			MSGIF_SEND( Hmsg->ResBuf, TrnsLen );			/* Start Transmit Response Message	*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}
#else
/****************************************************************************************************/
/*																									*/
/*		Memobus: System download																*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusSysDLRes( HMSGIF Hmsg, SHORT dlSts )
{

LONG	errorFlg = FALSE;
USHORT	*CmdBufW = (USHORT *)(Hmsg->CmdBuf);
USHORT	*ResBufW = (USHORT *)(Hmsg->ResBuf);
USHORT	TrnsLen;
USHORT	SubFuncCode = (((UCHAR)Hmsg->CmdBuf[2] << 8) | (UCHAR)Hmsg->CmdBuf[3]);

/*--------------------------------------------------------------------------------------------------*/
/*		Download command analysis & response																*/
/*--------------------------------------------------------------------------------------------------*/
		switch ( SubFuncCode & 0x00FF )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Block erase															*/
		/*------------------------------------------------------------------------------------------*/
		  case	SFC_DL_PRGDEL :
				ResBufW[0]	= CmdBufW[0];						/* AxisAdr, FunctionCode			*/
				ResBufW[1]	= CmdBufW[1];						/* SubFunctionCode					*/
				ResBufW[2]	= CmdBufW[2];						/* MPUNumber							*/
				ResBufW[3]	= CmdBufW[3];						/* FlushNumber		*/
				ResBufW[4]	= CmdBufW[4];						/* Higher word of write address	*/
				ResBufW[5]	= CmdBufW[5];						/* Write address lower word	*/
				if(dlSts == 0)
				{
					TrnsLen = 14;
				}
				else
				{
					LpxMbusSendErrorResponse( Hmsg, REGIF_DAMTHERR2 );
					errorFlg = TRUE;
				}
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		Program write																*/
		/*------------------------------------------------------------------------------------------*/
		  case	SFC_DL_PRGWR :
				ResBufW[0]	= CmdBufW[0];						/* AxisAdr, FunctionCode			*/
				ResBufW[1]	= CmdBufW[1];						/* SubFunctionCode					*/
				ResBufW[2]	= CmdBufW[2];						/* MPUNumber							*/
				ResBufW[3]	= CmdBufW[3];						/* FlushNumber		*/
				ResBufW[4]	= CmdBufW[4];						/* Data count	*/
				ResBufW[5]	= CmdBufW[5];						/* Higher word of write address	*/
				ResBufW[6]	= CmdBufW[6];						/* Write address lower word	*/
				if(dlSts == 0)
				{
					TrnsLen = 16;
				}
				else
				{
					LpxMbusSendErrorResponse( Hmsg, REGIF_DAMTHERR2 );
					errorFlg = TRUE;
				}
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		Serial IF speed change				<S041>											*/
		/*------------------------------------------------------------------------------------------*/
		  case	SFC_DL_SIFCHG:
				ResBufW[0]	= CmdBufW[0];						/*+ AxisAdr, FunctionCode			*/
				ResBufW[1]	= CmdBufW[1];						/*+ SubFunctionCode					*/
				ResBufW[2]	= CmdBufW[2];						/*+ MPUNumber							*/
				ResBufW[3]	= CmdBufW[3];						/*+ Transfer rate upper word	*/
				ResBufW[4]	= CmdBufW[4];						/*+ Transfer rate lower word	*/
				if(dlSts == 0)
				{
					TrnsLen = 12;
				}
				else
				{
					LpxMbusSendErrorResponse( Hmsg, REGIF_DAMTHERR2 );
					errorFlg = TRUE;
				}
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		Unsupported subfunction														*/
		/*		Commands valid only after download start request											*/
		/*			→Condition error																	*/
		/*------------------------------------------------------------------------------------------*/
		  default:
				LpxMbusSendErrorResponse( Hmsg, REGIF_DAMTHERR2 );
				errorFlg = TRUE;
				break;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Normal response transmission																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( errorFlg == FALSE )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	CRC-16Creation & response data transmission command															*/
		/*------------------------------------------------------------------------------------------*/
			MlibSetCRC16MB( Hmsg->ResBuf, TrnsLen );
			MSGIF_SEND( Hmsg->ResBuf, TrnsLen );			/* Start Transmit Response Message	*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}
#endif
#endif	/* <S150> */


/****************************************************************************************************/
/*																									*/
/*		Memobus: Single register read																	*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusReadOneRegister( HMSGIF Hmsg )
{
LONG	rc;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
LONG	RegNo   = MlibCnvBig16( wCmdBuf[2+s] );
/*--------------------------------------------------------------------------------------------------*/
/*		Register read processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		rc = RegFuncTbl[ RFTBLIDX(RegNo) ].ReadRegister( RegNo, 1, &wResBuf[2+s], Hmsg );
/*--------------------------------------------------------------------------------------------------*/
/*		Register read error check																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( rc != REGIF_COMPLETE )
		{
			LpxMbusSendErrorResponse( Hmsg, rc );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Create register read completion response																	*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = MlibCnvBig16( wResBuf[2+s] );	/* Read Register Data						*/
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		MlibSetCRC16MB(  Hmsg->ResBuf, 8+(2*s) );		/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, 8+(2*s) );		/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Single register write																	*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusWriteOneRegister( HMSGIF Hmsg )
{
LONG	rc;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
LONG	RegNo   = MlibCnvBig16( wCmdBuf[2+s] );
/*--------------------------------------------------------------------------------------------------*/
/*		Register write processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[3+s] = MlibCnvBig16( wCmdBuf[3+s] );
		rc = RegFuncTbl[ RFTBLIDX(RegNo) ].WriteRegister( RegNo, 1, &wResBuf[3+s], Hmsg );
/*--------------------------------------------------------------------------------------------------*/
/*		Response check (Auxiliary function in progress)																*/
/*--------------------------------------------------------------------------------------------------*/
		if( rc == REGIF_CONTINUE )
		{												/* After the auxiliary function is executed, a response is returned		*/
			return;										/* Here, do nothing.						*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register write error check																	*/
/*--------------------------------------------------------------------------------------------------*/
		else if( rc != REGIF_COMPLETE )
		{
			LpxMbusSendErrorResponse( Hmsg, rc );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Create register write completion response																	*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = wCmdBuf[2+s];					/* RegNo									*/
		wResBuf[3+s] = wCmdBuf[3+s];					/* WriteData								*/
		wResBuf[4+s] = wCmdBuf[4+s];					/* CRC-16									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Response data transmission command																			*/
/*--------------------------------------------------------------------------------------------------*/
		MSGIF_SEND( Hmsg->ResBuf, 10+(2*s) );		/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Continuous register read																	*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusReadSeqRegisters( HMSGIF Hmsg )
{
LONG	i, rc;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
LONG	RegNo   = MlibCnvBig16( wCmdBuf[2+s] );
LONG	RegNum  = MlibCnvBig16( wCmdBuf[3+s] );
/*--------------------------------------------------------------------------------------------------*/
/*		Check number of registers read																*/
/*--------------------------------------------------------------------------------------------------*/
		if( (RegNum < 1) || (RegNum > LpxGetMaxNumberOfRdRegisters( Hmsg, 8+(2*s) )) )
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_DANUMERR );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register read processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		rc = RegFuncTbl[ RFTBLIDX(RegNo) ].ReadRegister( RegNo, RegNum, &wResBuf[3+s], Hmsg );
/*--------------------------------------------------------------------------------------------------*/
/*		Register read error check																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( rc != REGIF_COMPLETE )
		{
			LpxMbusSendErrorResponse( Hmsg, rc );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Create register read completion response																	*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = MlibCnvBig16( RegNum * 2 );		/* Data Byte Number							*/
/*--------------------------------------------------------------------------------------------------*/
//#if		defined( CCSW_LITTLE_ENDIAN ) TODO
		for( i=0; i < RegNum; i++ ){ wResBuf[3+s+i] = MlibCnvBig16( wResBuf[3+s+i] );}
//#endif
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		i = (RegNum * 2) + 8 + (2*s);					/* ResMsg Length							*/
		MlibSetCRC16MB(  Hmsg->ResBuf, i );				/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, i );				/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Continuous register write																	*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusWriteSeqRegisters( HMSGIF Hmsg )
{
LONG	i, rc;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
LONG	RegNo   = MlibCnvBig16( wCmdBuf[2+s] );
LONG	RegNum  = MlibCnvBig16( wCmdBuf[3+s] );
/*--------------------------------------------------------------------------------------------------*/
/*		Check the number of registers written																*/
/*--------------------------------------------------------------------------------------------------*/
		if( (RegNum < 1) || (RegNum > LpxGetMaxNumberOfWrRegisters( Hmsg, 12+(2*s) )) )
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_DANUMERR );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register write processing																			*/
/*--------------------------------------------------------------------------------------------------*/
#if		defined( CCSW_BIG_ENDIAN )
		rc = RegFuncTbl[ RFTBLIDX(RegNo) ].WriteRegister( RegNo, RegNum, &wCmdBuf[5+s], Hmsg );
#else
		for( i=0; i < RegNum; i++ ){ wResBuf[5+s+i] = MlibCnvBig16( wCmdBuf[5+s+i] );}
		rc = RegFuncTbl[ RFTBLIDX(RegNo) ].WriteRegister( RegNo, RegNum, &wResBuf[5+s], Hmsg );
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Response check (Auxiliary function in progress)																*/
/*--------------------------------------------------------------------------------------------------*/
		if( rc == REGIF_CONTINUE )
		{												/* After the auxiliary function is executed, a response is returned		*/
			return;										/* Here, do nothing.						*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register write error check																	*/
/*--------------------------------------------------------------------------------------------------*/
		else if( rc != REGIF_COMPLETE )
		{
			LpxMbusSendErrorResponse( Hmsg, rc );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Create register write completion response																	*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = wCmdBuf[2+s];					/* RegNo									*/
		wResBuf[3+s] = wCmdBuf[3+s];					/* Write Number								*/
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		MlibSetCRC16MB(  Hmsg->ResBuf, 10+(2*s) );		/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, 10+(2*s) );		/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Discontinuous register read																*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusReadRandomRegisters( HMSGIF Hmsg )
{
LONG	rc;
LONG	i,j,x;
LONG	RegNo;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
LONG	RegNum  = MlibCnvBig16( wCmdBuf[2+s] );
/*--------------------------------------------------------------------------------------------------*/
/*		Check number of registers read																*/
/*--------------------------------------------------------------------------------------------------*/
		if( (RegNum < 1) || (RegNum > LpxGetMaxNumberOfRdRegisters( Hmsg, 8+(2*s) )) )
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_DANUMERR );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register read processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < RegNum; i=i+j )
		{
			RegNo = MlibCnvBig16( wCmdBuf[3+s+i] );
			for( j=1; j < (RegNum - i); j++ )
			{
				x = MlibCnvBig16( wCmdBuf[3+s+i+j] );
				if( x != (RegNo + j) ) break;
			}
		/*------------------------------------------------------------------------------------------*/
			rc = RegFuncTbl[ RFTBLIDX(RegNo) ].ReadRegister( RegNo, j, &wResBuf[3+s+i], Hmsg );
			if( rc != REGIF_COMPLETE )
			{
				LpxMbusSendErrorResponse( Hmsg, rc );
				return;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Create register read completion response																	*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = MlibCnvBig16( RegNum * 2 );		/* Data Byte Number							*/
/*--------------------------------------------------------------------------------------------------*/
//#if		defined( CCSW_LITTLE_ENDIAN ) TODO
		for( i=0; i < RegNum; i++ ){ wResBuf[3+s+i] = MlibCnvBig16( wResBuf[3+s+i] );}
//#endif
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		i = (RegNum * 2) + 8 + (2*s);					/* ResMsg Length							*/
		MlibSetCRC16MB(  Hmsg->ResBuf, i );				/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, i );				/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Discontinuous register write																*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusWriteRandomRegisters( HMSGIF Hmsg )
{
LONG	rc;
LONG	i,j,x;
LONG	RegNo;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
LONG	RegNum  = MlibCnvBig16( wCmdBuf[2+s] );
/*--------------------------------------------------------------------------------------------------*/
/*		Check the number of registers written																*/
/*--------------------------------------------------------------------------------------------------*/
		if( (RegNum < 1) || (RegNum > (LpxGetMaxNumberOfWrRegisters( Hmsg, 10+(2*s) ) >> 1) ) )
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_DANUMERR );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register write processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < RegNum; i=i+j )
		{
			RegNo = MlibCnvBig16( wCmdBuf[4+s+(2*i)] );
			for( j=1; j < (RegNum - i); j++ )
			{
				x = MlibCnvBig16( wCmdBuf[4+s+(2*(i+j))] );
				if( x != (RegNo + j) ) break;
			}
		/*------------------------------------------------------------------------------------------*/
			for( x=0; x < j; x++ )
			{
				wResBuf[x] = MlibCnvBig16( wCmdBuf[4+s+(2*(i+x))+1] );
			}
		/*------------------------------------------------------------------------------------------*/
			rc = RegFuncTbl[ RFTBLIDX(RegNo) ].WriteRegister( RegNo, j, &wResBuf[0], Hmsg );
			if( rc != REGIF_COMPLETE )
			{
				LpxMbusSendErrorResponse( Hmsg, rc );
				return;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Create register write completion response																	*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = wCmdBuf[2+s];					/* RegNum									*/
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		MlibSetCRC16MB(  Hmsg->ResBuf, 8+(2*s) );		/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, 8+(2*s) );		/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Read the maximum number of registers																*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusReadMaxNumberOfRegisters( HMSGIF Hmsg )
{
LONG	i;
LONG	MaxNum;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
/*--------------------------------------------------------------------------------------------------*/
/*		Check SFC with Slot																			*/
/*--------------------------------------------------------------------------------------------------*/
		i = LPX_GET_SLOT_BYTELEN( Hmsg->CmdBuf[4+(2*s)] );
/*--------------------------------------------------------------------------------------------------*/
/*		Check Function Code and Calculate MaxNumber													*/
/*--------------------------------------------------------------------------------------------------*/
		switch( Hmsg->CmdBuf[5+(2*s)] )
		{
		case SFC_RD_ONEREG: MaxNum = 1; break;
		case SFC_WR_ONEREG: MaxNum = 1; break;
		case SFC_RD_SEQREG: MaxNum = LpxGetMaxNumberOfRdRegisters( Hmsg, i +  8 ); break;
		case SFC_WR_SEQREG: MaxNum = LpxGetMaxNumberOfWrRegisters( Hmsg, i + 12 ); break;
		case SFC_RD_RANREG: MaxNum = LpxGetMaxNumberOfRdRegisters( Hmsg, i +  8 ); break;
		case SFC_WR_RANREG: MaxNum = LpxGetMaxNumberOfWrRegisters( Hmsg, i + 10 ) >> 1; break;
		case SFC_RD_SEQMEM: MaxNum = LpxGetMaxNumberOfRdRegisters( Hmsg, i +  8 ); break;
		case SFC_WR_SEQMEM: MaxNum = LpxGetMaxNumberOfWrRegisters( Hmsg, i + 12 ); break;
		default: 			MaxNum = 0; break;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Response data creation																				*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = wCmdBuf[2+s];					/* SubFuncCode for MaxNum					*/
		wResBuf[3+s] = MlibCnvBig16( MaxNum );			/* MaxNumber of Registers					*/
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		MlibSetCRC16MB(  Hmsg->ResBuf, 10+(2*s) );		/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, 10+(2*s) );		/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Continuous memory read																	*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusReadSeqMemories( HMSGIF Hmsg )
{
//LONG	i, rc;
LONG	i;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
ULONG	MemNum  = MlibCnvBig16( wCmdBuf[4+s] );
ULONG	MemAddr = (MlibCnvBig16( wCmdBuf[2+s] ) << 16) +  MlibCnvBig16( wCmdBuf[3+s] );
/*--------------------------------------------------------------------------------------------------*/
/*		Memory read number check																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( (MemNum < 1) || (MemNum > LpxGetMaxNumberOfRdRegisters( Hmsg, 8+(2*s) )) )
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_DANUMERR );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Memory read address check																*/
/*--------------------------------------------------------------------------------------------------*/
		if ( 0 /* KpiCheckReadMemAddr( MemAddr, MemNum) == MEMDEF_READ_NG  */)
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_SEC_ACCSERR );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Memory read response data creation																	*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = wCmdBuf[4+s];					/* Data Word Number							*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < MemNum; i++ )
		{
			wResBuf[3+s+i] = MlibCnvBig16( ((USHORT*)MemAddr)[i] );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		i = (MemNum * 2) + 8 + (2*s);					/* ResMsg Length							*/
		MlibSetCRC16MB(  Hmsg->ResBuf, i );				/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, i );				/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Continuous memory write																	*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusWriteSeqMemories( HMSGIF Hmsg )
{
LONG	i;
LONG	s = LPX_GET_SLOT_WORDLEN( Hmsg->CmdBuf[2] );
USHORT*	wCmdBuf = (USHORT *)(Hmsg->CmdBuf);
USHORT*	wResBuf = (USHORT *)(Hmsg->ResBuf);
ULONG	MemNum  = MlibCnvBig16( wCmdBuf[4+s] );
ULONG	MemAddr = (MlibCnvBig16( wCmdBuf[2+s] ) << 16) +  MlibCnvBig16( wCmdBuf[3+s] );
/*--------------------------------------------------------------------------------------------------*/
/*		Memory write number check																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( (MemNum < 1) || (MemNum > LpxGetMaxNumberOfWrRegisters( Hmsg, 12+(2*s) )) )
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_EC_DANUMERR );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Memory write address check																*/
/*--------------------------------------------------------------------------------------------------*/
		if ( 0 /* KpiCheckWriteMemAddr( MemAddr, MemNum ) == MEMDEF_WRITE_NG */)
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_SEC_ACCSERR );
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Memory write processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < MemNum; i++ )
		{
			((USHORT*)MemAddr)[i] = MlibCnvBig16( wCmdBuf[5+s+i] );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Memory write response data creation																	*/
/*--------------------------------------------------------------------------------------------------*/
		wResBuf[0] = wCmdBuf[0];						/* AxisAddr, FuncCode						*/
		wResBuf[1] = wCmdBuf[1];						/* SubFuncCode								*/
		if( s ){ wResBuf[2] = wCmdBuf[2];}				/* Slot Number								*/
		wResBuf[2+s] = wCmdBuf[2+s];					/* Memory Address							*/
		wResBuf[3+s] = wCmdBuf[3+s];					/* Memory Address							*/
		wResBuf[4+s] = wCmdBuf[4+s];					/* Data Word Number							*/
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		MlibSetCRC16MB(  Hmsg->ResBuf, 12 + (2*s) );	/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, 12 + (2*s) );	/* Start Transmit Response Message			*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Getting maximum number of read registers																*/
/*																									*/
/****************************************************************************************************/
LONG	LpxGetMaxNumberOfRdRegisters( HMSGIF Hmsg, LONG FixedLen )
{
		return( ((LONG)Hmsg->SndBufSize - FixedLen) >> 1 );
}



/****************************************************************************************************/
/*																									*/
/*		Getting the maximum number of write registers																*/
/*																									*/
/****************************************************************************************************/
LONG	LpxGetMaxNumberOfWrRegisters( HMSGIF Hmsg, LONG FixedLen )
{
		return( ((LONG)Hmsg->RcvBufSize - FixedLen) >> 1 );
}



/****************************************************************************************************/
/*																									*/
/*		Memobus: Abnormal response reply processing																	*/
/*																									*/
/****************************************************************************************************/
void	LpxMbusSendErrorResponse( HMSGIF Hmsg, ULONG ErrCode )
{
UCHAR	*CmdBuf = (UCHAR *)(Hmsg->CmdBuf);
UCHAR	*ResBuf = (UCHAR *)(Hmsg->ResBuf);
/*--------------------------------------------------------------------------------------------------*/
/*		Create abnormal response data																			*/
/*--------------------------------------------------------------------------------------------------*/
		switch ( (USHORT)ErrCode )
		{
		case REGIF_CMDERR	: ErrCode = SMBUS_EC_FNCDERR;	break;
		case REGIF_REGNOERR	: ErrCode = SMBUS_SEC_ACCSERR;	break;
		case REGIF_DALMTERR	: ErrCode = SMBUS_SEC_DALMTERR;	break;
		case REGIF_DACALERR	:
		case REGIF_DAMTHERR	: ErrCode = SMBUS_SEC_DAMTHERR;	break;
		case REGIF_DAMTHERR2: ErrCode = SMBUS_SEC_CONDERR;	break;
		case REGIF_DANUMERR	: ErrCode = SMBUS_EC_DANUMERR;	break;
		case REGIF_FOPEERR1	:
		case REGIF_FOPEERR2	:
		case REGIF_FOPEERR3	: ErrCode = SMBUS_SEC_INTLOCK;	break;
		default:			 /* ErrCode = ErrCode; */		break;	/* <S004> */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Create abnormal response data																			*/
/*--------------------------------------------------------------------------------------------------*/
		ResBuf[0] = CmdBuf[0];							/* AxisAddr									*/
		ResBuf[1] = CmdBuf[1] | 0x80;					/* Function Code							*/
		ResBuf[2] = (UCHAR)(ErrCode);					/* Error Code								*/
/*--------------------------------------------------------------------------------------------------*/
/*		CRC-16Creation & response data transmission command																*/
/*--------------------------------------------------------------------------------------------------*/
		MlibSetCRC16MB(  Hmsg->ResBuf, 5 );				/* Set CRC-16 to ResBuf						*/
		MSGIF_SEND( Hmsg->ResBuf, 5 );				/* Start Transmit Response Message			*/
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Register IF function: Register number error															*/
/*																									*/
/****************************************************************************************************/
LONG	LpxRegifRegNoError( LONG RegNo, LONG Number, USHORT *RwBuf, HMSGIF Hmsg )
{
		Hmsg->ErrRno = (USHORT)RegNo;					/* Abnormal register number							*/
		return ( SMBUS_SEC_REGNOERR );					/* Register number error							*/
}

#if 0 && CCSW_CMDID == CMDID_EXIST
/****************************************************************************************************/
/*																									*/
/*		Option card communication response data transfer processing														*/
/*																									*/
/****************************************************************************************************/
ULONG	RpxSmsgTransOptRspMessage( HMSGIF Hmsg )
{
ULONG	rc;

/*--------------------------------------------------------------------------------------------------*/
/*		Serial communication data response transfer																	*/
/*--------------------------------------------------------------------------------------------------*/
		rc = MemobusTransCmdOptRspMsg( Hmsg );		/* Directive option card (SCMessage communication)		*/

		return( rc );
}



/****************************************************************************************************/
/*																									*/
/*		Command data transfer setting to command option card (SigmaWin+to a higher-level tool such as)				*/
/*																									*/
/****************************************************************************************************/
void	MemobusTransCmdOptCmdMsg( HMSGIF Hmsg )
{
LONG	size = (LONG)(Hmsg->CmdMsgLen);
USHORT	*CmdBufW = (USHORT *)(Hmsg->CmdBuf);

/*--------------------------------------------------------------------------------------------------*/
/*		Command option communication status acquisition																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( ( Hmsg->f.ComMsgIF ||  Hmsg->f.UsbMsgIF ) && (OpiGetSCMsgState() == SCMSG_ENABLE) )
		{
			OpiReqSCMsgTrans( CmdBufW, size);						/* SCMessage transmission start			*/
			KpiRstLongTimer( &Hmsg->ComTimeLong );					/* Timeout reset */
			Hmsg->RcvWait = TRUE;									/* SCMessage communication reception waiting set*/
		}
		else
		{
			LpxMbusSendErrorResponse( Hmsg, SMBUS_SEC_ACCSERR );	/* Access restriction error (0x31)	*/
		}

		return;
}



/****************************************************************************************************/
/*																									*/
/*		Response data transfer setting from command option card (SigmaWin+)				*/
/*																									*/
/****************************************************************************************************/
ULONG	MemobusTransCmdOptRspMsg( HMSGIF Hmsg )
{
LONG	rc,size;
USHORT	*ResBufW = (USHORT *)(Hmsg->ResBuf);

/*--------------------------------------------------------------------------------------------------*/
/*		SCWhen waiting for message reception, execute response acquisition process													*/
/*--------------------------------------------------------------------------------------------------*/
		if( Hmsg->RcvWait == TRUE )
		{
/*--------------------------------------------------------------------------------------------------*/
/*			MEMOBUSGetting response to message command												*/
/*--------------------------------------------------------------------------------------------------*/
			rc = OpiGetSCMsgRspData( ResBufW, &size );

/*--------------------------------------------------------------------------------------------------*/
/*			Response data analysis																			*/
/*--------------------------------------------------------------------------------------------------*/
			if( rc == SCMSG_SUCCESS )
			{/* Normal end */
				Hmsg->StxResMsg( Hmsg->ResBuf, (USHORT)size );		/* Start Transmit Response Message	*/
				Hmsg->RcvWait = FALSE;								 /* Clear reception wait			*/
			}
			else if( rc == SCMSG_BUSY )
			{/* Continuing processing */
				if ( KpiGetLongTimer( &Hmsg->ComTimeLong ) > SCM_COM_TIMEOUT ) /* 1000msecElapsed time	*/
				{
					OpiCancelSCMsgReq( );							/* SCCancel message reception wait	*/
					Hmsg->StxResMsg( Hmsg->ResBuf, 0 );					/* Start sending data without response	*/
					Hmsg->RcvWait = FALSE;								 /* Clear reception wait			*/
				}
			}
			else if( (rc == SCMSG_REQ_ERR) || (rc == SCMSG_RCV_ERR) )
			{/* Abnormal termination */
				Hmsg->StxResMsg( Hmsg->ResBuf, 0 );							/* Start sending data without response	*/
				Hmsg->RcvWait = FALSE;									 /* Clear reception wait			*/
			}
			else
			{
				LpxMbusSendErrorResponse( Hmsg, SMBUS_SEC_ACCSERR );	 /* Access restriction error(0x31)*/
				Hmsg->RcvWait = FALSE;									 /* Clear reception wait			*/
			}
			return( TRUE );		/* Hmsg->RcvWait == TRUE, anywayTRUEdue to vibration.*/
								/* Otherwise,FALSE, command analysis is re-analyzed.*/
		}
		else
		{
			return( Hmsg->RcvWait );
		}
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Parameter readout(Register specification)															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview:	Specified number of specified parameters(WORD)Read out.											*/
/*			Reads the specified Parameter value														*/
/*																									*/
/*	Return value:	Status																				*/
/*																									*/
/****************************************************************************************************/
LONG	LpxReadPrmRegister(					/* Parameter readout(Register specification)						*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of read data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Read data buffer									*/
		HMSGIF	Hmsg			)			/* ReadingIFHandle										*/
{
//LONG	rc;
LONG	i,idx;
LONG	pn_data;
LONG	RegNoX = RegNo & 0x0FFF;
COBJECT *pObj = Lpx_GetObject( RegNo );		/* Object Pointer */
//CPRMTBL	*pTbl = GET_PNPRMTBL( pObj );		/* Parameter Table */
/*--------------------------------------------------------------------------------------------------*/
/*		Register access area check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( ((RegNoX + Number) >= 0x1000)
		 ||	((RegNoX < 0x0800) && ((RegNoX + Number) >= 0x0800))
		 || ((RegNoX < 0x0D00) && ((RegNoX + Number) >= 0x0D00))	)
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_DAMTHERR );					/* Data consistency error(Group boundary)				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register number search																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( OK != OBJ_SEARCH( pObj, RegNoX, &idx, Hmsg ) )
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error(REGNo number)			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Start register consistency check																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( pObj->GetNumber( pObj, idx, Hmsg ) != RegNoX )
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_DAMTHERR );					/* Data consistency error(LONGBoundary)					*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register data read																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < Number; i++, idx++ )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	System parameter read check(Maybe you should go!)									*/
		/*------------------------------------------------------------------------------------------*/
#if 0
			if( 0 && (PnPrmTbl[idx].PrmDefPtr->AccessLevel >= ACCLVL_SYSTEM) )
			{
				if( Hmsg->AccessLvl < ACCLVL_SYSTEM )
				{
					Hmsg->ErrRno = RegNo + i;			/* Abnormal register number							*/
					return( REGIF_REGNOERR );			/* Register number error							*/
				}
			}
#endif
		/*------------------------------------------------------------------------------------------*/
		/*	Continuity check of register numbers															*/
		/*------------------------------------------------------------------------------------------*/
			if( pObj->GetNumber( pObj, idx, Hmsg ) != (RegNoX + i) )
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				return( REGIF_REGNOERR );				/* Register number error(REGNumber non-consecutive)			*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Parameter readout																		*/
		/*------------------------------------------------------------------------------------------*/
			OBJ_READ( pObj, idx, (ULONG*)&pn_data, Hmsg );
#if 0
//			rc = RpxPrmReadIdx( idx, (ULONG *)(&pn_data), PCHK_REG_READ );	/* <11J45>Delete */

			/* <11J45>addition Start */
			if(Hmsg->f.NotConvScale == TRUE)
			{
				rc = RpxPrmReadIdxNotConvScale( idx, (ULONG *)(&pn_data), PCHK_REG_READ ); /* Do not scale scale conversion */
			}
			else
			{
				rc = RpxPrmReadIdx( idx, (ULONG *)(&pn_data), PCHK_REG_READ );				/* Scale scale conversion */
			}
			/* <11J45>addition End */

			if(rc != OK)
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				switch(rc)
				{
				case PERR_DFIT:
					return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)					*/
					break;
				case PERR_PRMCOPY:
					return( REGIF_FOPEERR3 );			/* Processing not possible(Processing conflict with other CH)				*/
					break;
				default:
					return( REGIF_REGNOERR );			/* Register number error							*/
					break;
				}
			}
#endif
		/*------------------------------------------------------------------------------------------*/
		/*	Read data is short/Long?															*/
		/*------------------------------------------------------------------------------------------*/
			if( pObj->isLong( pObj, idx, Hmsg ) )
			{
				RdBuf[i+0]= GET_LWD_REGDATA( pn_data );	/* LONG  Parameter Low  Word				*/
				RdBuf[i+1]= GET_HWD_REGDATA( pn_data );	/* LONG  Parameter High Word				*/
				i++;
			}
			else
			{
				RdBuf[i] = (USHORT)(pn_data);			/* SHORT Parameter							*/
			}
#if 0
//			if( PnPrmTbl[idx].PrmDefPtr->Attr.IfIsLong == FALSE )/* <11J45>Delete */
			if (((PnPrmTbl[idx].PrmDefPtr->Attr.IfIsLong == FALSE)&&( Hmsg->f.NotConvScale == FALSE))
				||((PnPrmTbl[idx].PrmDefPtr->Attr.VarIsLong == FALSE)&&( Hmsg->f.NotConvScale == TRUE)))/* <11J45> */
			{
				RdBuf[i] = (USHORT)(pn_data);			/* SHORT Parameter							*/
			}
			else if( (i+1) < Number )
			{
				RdBuf[i+0]= GET_LWD_REGDATA( pn_data );	/* LONG  Parameter Low  Word				*/
				RdBuf[i+1]= GET_HWD_REGDATA( pn_data );	/* LONG  Parameter High Word				*/
				i++;
			}
			else
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				return( REGIF_DAMTHERR );				/* Data consistency error(LONGBoundary)					*/
			}
#endif
		}
/*--------------------------------------------------------------------------------------------------*/
		return( REGIF_COMPLETE );
}

/****************************************************************************************************/
/*																									*/
/*		Parameter writing process(Register specification)														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview:	Set parameter value specified by registerRAM.										*/
/*																									*/
/*	Return value:	Status																				*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG	LpxWritePrmRegister(				/* Parameter writing process(Register specification)					*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of write data(WORDpiece number)							*/
		USHORT	*WrBuf,						/* Write data buffer								*/
		HMSGIF	Hmsg			)			/* WriteIFHandle									*/
{
LONG	rc;
LONG	i,idx;
ULONG	pn_data;
LONG	RegNoX = RegNo & 0x0FFF;
COBJECT *pObj = Lpx_GetObject( RegNo );		/* Object Pointer */
CPRMTBL	*pTbl = GET_PNPRMTBL( pObj );		/* Parameter Table */
/*--------------------------------------------------------------------------------------------------*/
/*		Register access area check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( ((RegNoX + Number) >= 0x1000)
		 ||	((RegNoX < 0x0800) && ((RegNoX + Number) >= 0x0800))
		 || ((RegNoX < 0x0D00) && ((RegNoX + Number) >= 0x0D00))	)
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_DAMTHERR );					/* Data consistency error(Group boundary)				*/
		}
#if 0 
/*--------------------------------------------------------------------------------------------------*/
/*		System constant parameter check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( (OpeReg.PrmAccess == FALSE) &&
			((RegNoX + Number) >= 0x0D00) && (Hmsg->AccessLvl != ACCLVL_SYSTEM) )
		{
			i = (RegNoX >= 0x0D00)? 0 : (0x0D00 - RegNoX);
			Hmsg->ErrRno = RegNo + i;					/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error							*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder constant parameter check															*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
		if( ENC_PRM_WRT_DISABLE && ((RegNoX + Number) >= ENC_PRM_NUM) )
		{
			i = (RegNoX >= 0x0F00)? 0 : (ENC_PRM_NUM - RegNoX);
			Hmsg->ErrRno = RegNo + i;					/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error							*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Register number search																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( OK != OBJ_SEARCH( pObj, RegNoX, &idx, Hmsg ) )
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error(REGNo number)			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Start register consistency check																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( pObj->GetNumber( pObj, idx, Hmsg ) != RegNoX )
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_DAMTHERR );					/* Data consistency error(LONGBoundary)					*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		System constant parameter check								<12J49>addition					 	*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
/* Register number error obtained by register number searchidx" in the response buffer.		 	*/
		if( (pPrm->prmwr_spn == 0x0001) 									   /* Pn030 = 0x0001(MotorPrmWriting enabled) */
		 && (PnPrmTbl[idx].PrmDefPtr->Attr.MotorPrmAmp == PRMDEF_MOTPRM_AMP)   /* Motor in the amplifierPrm(Main shaft motorPrm) 	*/
		 && (PnPrmTbl[idx].PrmDefPtr != (&pndef_MotPrmSumChkEnb)) 			   /* PnFFF 							*/
		 && (Hmsg->f.LnkMsgIF == TRUE) )									   /* SVPRM_WRuse	 					*/
		{
			;
		}
		else if( (OpeReg.PrmAccess == FALSE) &&
				((RegNoX + Number) >= 0x0D00) && (Hmsg->AccessLvl != ACCLVL_SYSTEM) )
		{
			i = (RegNoX >= 0x0D00)? 0 : (0x0D00 - RegNoX);
			Hmsg->ErrRno = RegNo + i;					/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error							*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Register data write protection check															*/
/*--------------------------------------------------------------------------------------------------*/
//		if( RndWrtDisable.FunA.f.PrmReg || RndWrtDisable.FunB.f.PrmReg )
//		{
//			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
//			return( REGIF_REGNOERR );					/* Register number error							*/
//		}
/*--------------------------------------------------------------------------------------------------*/
/*		Write register data																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < Number; i++, idx++ )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	Check parameter access level														*/
		/*------------------------------------------------------------------------------------------*/
#if 0 /* TODO :*/
			if( (OpeReg.PrmAccess == FALSE) && (PrmAttributeChkIdx(idx, PCHK_ACCS_LVL, Hmsg) != OK) )
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				return( REGIF_REGNOERR );				/* Register number error							*/
			}
#endif
		/*------------------------------------------------------------------------------------------*/
		/*	Continuity check of register numbers															*/
		/*------------------------------------------------------------------------------------------*/
			if( pObj->GetNumber( pObj, idx, Hmsg ) != (RegNoX + i) )
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				return( REGIF_REGNOERR );				/* Register number error(REGNumber non-consecutive)			*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Write data is long?																	*/
		/*------------------------------------------------------------------------------------------*/
			if( pObj->isLong( pObj, idx, Hmsg ) )
			{
				if( (i + 1) < Number )
				{
					pn_data = ULONGOF_REGDATA( WrBuf[i], WrBuf[i+1] );
				}
				else
				{
					Hmsg->ErrRno = RegNo + i;			/* Abnormal register number							*/
					return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)					*/
				}
			}
			else
			{
				pn_data = WrBuf[i];
				if( CHK_PNPRM_SIGNED( GET_PNPRMDEF( pTbl, idx ) ) )
				{
					pn_data = (LONG)((SHORT)pn_data); 	/* For signed words, sign extend	*/
				}
			}
#if 0
			if( PnPrmTbl[idx].PrmDefPtr->Attr.IfIsLong )
			{	/* LONG Data	*/
				if( (i + 1) < Number )
				{
					pn_data = ULONGOF_REGDATA( RdBuf[i], RdBuf[i+1] );
				}
				else
				{
					Hmsg->ErrRno = RegNo + i;			/* Abnormal register number							*/
					return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)					*/
				}
			}
			else
			{	/* WORD Data		*/
				pn_data = RdBuf[i];
				/* WORDSize is signed	*/
				if( PnPrmTbl[idx].PrmDefPtr->Attr.Sign == DSPDEF_W_SIGN )
				{
					/* For signed words, sign extend	*/
					pn_data = (LONG)((SHORT)pn_data);
				}
			}
#endif
		/*------------------------------------------------------------------------------------------*/
		/*	ParameterRAMWrite																		*/
		/*------------------------------------------------------------------------------------------*/
			rc = OBJ_WRITE( pObj, idx, pn_data, Hmsg );
			if(  rc != OK )
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				switch(rc)
				{
				case OERR_DFIT:
					 return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)					*/
//					 break;//<S0C7>statement is unreachable
				case OERR_RONLY:
					 return( REGIF_REGNOERR );			/* Register number error							*/
//					 break;//<S0C7>statement is unreachable
				case OERR_CALC:
					 return( REGIF_DACALERR );			/* Data abnormality(Operation error) 					*/
//					 break;//<S0C7>statement is unreachable
				case OERR_RANGE:
					 return( REGIF_DALMTERR );			/* Data abnormality(Upper/lower limit error)					*/
//					 break;//<S0C7>statement is unreachable
				case OERR_PRMCOPY:
				case OERR_WRITE_HANDLE:
					 return( REGIF_FOPEERR3 );			/* Processing not possible(Processing conflict with other CH)				*/
//					 break;//<S0C7>statement is unreachable
				default:
					 return( REGIF_REGNOERR );			/* Register number error							*/
//					 break;//<S0C7>statement is unreachable
				}
			}
#if 0
			rc = RpxPrmWriteRamIdx( idx, pn_data, PCHK_REG_WRITE, Hmsg );
			if( rc != OK )
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				switch(rc)
				{
				case PERR_DFIT:
					 return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)					*/
					 break;
				case PERR_RONLY:
					 return( REGIF_REGNOERR );			/* Register number error							*/
					 break;
				case PERR_CALC:
					 return( REGIF_DACALERR );			/* Data abnormality(Operation error) 					*/
					 break;
				case PERR_RANGE:
					 return( REGIF_DALMTERR );			/* Data abnormality(Upper/lower limit error)					*/
					 break;
				case PERR_PRMCOPY:
				case PERR_WRITE_HANDLE:
					 return( REGIF_FOPEERR3 );			/* Processing not possible(Processing conflict with other CH)				*/
					 break;
				default:
					 return( REGIF_REGNOERR );			/* Register number error							*/
					 break;
				}
			}
#endif
		/*------------------------------------------------------------------------------------------*/
		/*	ParameterEEPROMWrite																	*/
		/*------------------------------------------------------------------------------------------*/
//#if 0		/* 40H EEPROMWrite support<S111>	*/
			if( !(RegNo & 0x1000) )
			{
#if 0 /* <12J49>Delete */
				rc = RpiPrmUpdateNvmem( PnPrmTbl[idx].PrmDefPtr );
				if( rc != OK && rc != PRM_EEPROM_RAM_NO_DIFF )
				{
					return( REGIF_DAMTHERR );
				}
			}

#endif /* <12J49>DeleteEnd */

#if 0	/* 40H EEPROMWrite support<S111>	*/ /* <12J49>addition */
                if( (rc = LpxChkSumFlagSave(PnPrmTbl[idx].PrmDefPtr, Hmsg)) == TRUE)
                {
                    rc = RpiPrmUpdateNvmem( PnPrmTbl[idx].PrmDefPtr );
                }

                if( rc != OK && rc != PRM_EEPROM_RAM_NO_DIFF )
                {
                    return( REGIF_DAMTHERR );
				}
#endif	/* 40H EEPROMWrite support<S111>	*/ /* <12J49>additionEnd */
/* 40H EEPROMWrite support<S111>	*/
				rc = PrmWriteValueToEeprom( Hmsg->pAxRsc->RegManager->hPnReg, PnPrmTbl[idx].Attribute, 0 );
				if( (rc != PRM_RSLT_SUCCESS) && (rc != PRM_RSLT_NO_DIFF) )
				{
					return( REGIF_DAMTHERR );
				}
/* 40H EEPROMWrite support<S111>	*/
			}
//#endif		/* 40H EEPROMWrite support<S111>	*/

		/*------------------------------------------------------------------------------------------*/
		/*	LONGFor data																		*/
		/*------------------------------------------------------------------------------------------*/
			if( pObj->isLong( pObj, idx, Hmsg ) )
			{
				i++;
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( REGIF_COMPLETE );
}

/****************************************************************************************************/
/*																									*/
/*		register read(Operation area, model information,etc)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview:	Specified number of specified parameters(WORD)Read out.											*/
/*			Reads the specified Parameter value														*/
/*																									*/
/*	Return value:	Status																				*/
/*																									*/
/****************************************************************************************************/
LONG	LpxReadComObject(					/* Parameter readout(Register specification)						*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of read data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Read data buffer									*/
		HMSGIF	Hmsg			)			/* ReadingIFHandle										*/
{
//LONG	rc;
LONG	i,idx;
LONG	subidx;								/* <S073> */
LONG	pn_data;
LONG	RegNoX = RegNo & 0x0FFF;
COBJECT *pObj = Lpx_GetObject( RegNo );		/* Object Pointer */
CPRMTBL	*pTbl = GET_PNPRMTBL( pObj );		/* Parameter Table */
/*--------------------------------------------------------------------------------------------------*/
/*		Register access area check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( 0 )
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_DAMTHERR );					/* Data consistency error(Group boundary)				*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register number search																			*/
/*--------------------------------------------------------------------------------------------------*/
//<S073>		if( OK != OBJ_SEARCH( pObj, RegNoX, &idx, Hmsg ) )
		if( OK != OBJ_SEARCH_SUB( pObj, RegNoX, &idx, &subidx, Hmsg ) )	/* <S073> */
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error(REGNo number)			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Start register consistency check																	*/
/*--------------------------------------------------------------------------------------------------*/
//<S073>		if( pObj->GetNumber( pObj, idx, Hmsg ) != RegNoX )
		if( (pObj->GetNumber( pObj, idx, Hmsg ) +subidx )!= RegNoX )	/* <S073> */
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_DAMTHERR );					/* Data consistency error(LONGBoundary)					*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Register data read																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < Number; i++, idx++ )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	System parameter read check(Maybe you should go!)									*/
		/*------------------------------------------------------------------------------------------*/
#if 0
			if( 0 && (PnPrmTbl[idx].PrmDefPtr->AccessLevel >= ACCLVL_SYSTEM) )
			{
				if( Hmsg->AccessLvl < ACCLVL_SYSTEM )
				{
					Hmsg->ErrRno = RegNo + i;			/* Abnormal register number							*/
					return( REGIF_REGNOERR );			/* Register number error							*/
				}
			}
#endif
		/*------------------------------------------------------------------------------------------*/
		/*	Continuity check of register numbers															*/
		/*------------------------------------------------------------------------------------------*/
//<S073>			if( pObj->GetNumber( pObj, idx, Hmsg ) != (RegNoX + i) )
			if( (pObj->GetNumber( pObj, idx, Hmsg ) +subidx) != (RegNoX + i) ) /* <S073> */
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				return( REGIF_REGNOERR );				/* Register number error(REGNumber non-consecutive)			*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*	Parameter readout																		*/
		/*------------------------------------------------------------------------------------------*/
#if 1	/* <S073> ArrayCorrespondence */
			do {
				/*----------------------------------------------------------------------------------*/
				/*	Read data is short/Long?													*/
				/*----------------------------------------------------------------------------------*/
				if( pObj->isLong( pObj, idx, Hmsg ) )
				{
					OBJ_READ_SUB( pObj, idx, subidx, (ULONG*)&pn_data, Hmsg );
					RdBuf[i+0]= GET_LWD_REGDATA( pn_data );		/* LONG  Parameter Low  Word		*/
					RdBuf[i+1]= GET_HWD_REGDATA( pn_data );		/* LONG  Parameter High Word		*/
					i++; subidx++;								/* increment index */
				}
				else
				{
					OBJ_READ_SUB( pObj, idx, subidx, (ULONG*)&pn_data, Hmsg );
					RdBuf[i] = (USHORT)(pn_data);				/* SHORT Parameter					*/
				}
				/*----------------------------------------------------------------------------------*/
				i++; subidx++;									/* increment index */
			} 
			while ( (i < Number) && (subidx < Lpx_GetMaxSubIdx( pTbl, idx ) ) );
			subidx = 0;
#else
			OBJ_READ( pObj, idx, &pn_data, Hmsg );
		/*------------------------------------------------------------------------------------------*/
		/*	Read data is short/Long?															*/
		/*------------------------------------------------------------------------------------------*/
			if( pObj->isLong( pObj, idx, Hmsg ) )
			{
				RdBuf[i+0]= GET_LWD_REGDATA( pn_data );	/* LONG  Parameter Low  Word				*/
				RdBuf[i+1]= GET_HWD_REGDATA( pn_data );	/* LONG  Parameter High Word				*/
				i++;
			}
			else
			{
				RdBuf[i] = (USHORT)(pn_data);			/* SHORT Parameter							*/
			}
#endif
		}
/*--------------------------------------------------------------------------------------------------*/
		return( REGIF_COMPLETE );
}



/****************************************************************************************************/
/*																									*/
/*		Parameter writing process(Register specification)														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview:	Set parameter value specified by registerRAM.										*/
/*																									*/
/*	Return value:	Status																				*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
LONG	LpxWriteComObject(					/* Parameter writing process(Register specification)					*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of write data(WORDpiece number)							*/
		USHORT	*WrBuf,						/* Write data buffer								*/
		HMSGIF	Hmsg			)			/* WriteIFHandle									*/
{
LONG	rc;
LONG	i,idx;
LONG	subidx;								/* <S073> */
ULONG	pn_data;
LONG	RegNoX = RegNo & 0x0FFF;
COBJECT *pObj = Lpx_GetObject( RegNo );		/* Object Pointer */
CPRMTBL	*pTbl = GET_PNPRMTBL( pObj );		/* Parameter Table */
/*--------------------------------------------------------------------------------------------------*/
/*		Register access area check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( 0 )
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_DAMTHERR );					/* Data consistency error(Group boundary)				*/
		}
#if 0 
/*--------------------------------------------------------------------------------------------------*/
/*		System constant parameter check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( (OpeReg.PrmAccess == FALSE) &&
			((RegNoX + Number) >= 0x0D00) && (Hmsg->AccessLvl != ACCLVL_SYSTEM) )
		{
			i = (RegNoX >= 0x0D00)? 0 : (0x0D00 - RegNoX);
			Hmsg->ErrRno = RegNo + i;					/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error							*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder constant parameter check															*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
		if( ENC_PRM_WRT_DISABLE && ((RegNoX + Number) >= ENC_PRM_NUM) )
		{
			i = (RegNoX >= 0x0F00)? 0 : (ENC_PRM_NUM - RegNoX);
			Hmsg->ErrRno = RegNo + i;					/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error							*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Register number search																			*/
/*--------------------------------------------------------------------------------------------------*/
//<S073>		if( OK != OBJ_SEARCH( pObj, RegNoX, &idx, Hmsg ) )
		if( OK != OBJ_SEARCH_SUB( pObj, RegNoX, &idx, &subidx, Hmsg ) )	/* <S073> */
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error(REGNo number)			*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Start register consistency check																	*/
/*--------------------------------------------------------------------------------------------------*/
//<S073>		if( pObj->GetNumber( pObj, idx, Hmsg ) != RegNoX )
		if( (pObj->GetNumber( pObj, idx, Hmsg ) +subidx )!= RegNoX )	/* <S073> */
		{
			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
			return( REGIF_DAMTHERR );					/* Data consistency error(LONGBoundary)					*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		System constant parameter check								<12J49>addition					 	*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
/* Register number error obtained by register number searchidx" in the response buffer.		 	*/
		if( (pPrm->prmwr_spn == 0x0001) 									   /* Pn030 = 0x0001(MotorPrmWriting enabled) */
		 && (PnPrmTbl[idx].PrmDefPtr->Attr.MotorPrmAmp == PRMDEF_MOTPRM_AMP)   /* Motor in the amplifierPrm(Main shaft motorPrm) 	*/
		 && (PnPrmTbl[idx].PrmDefPtr != (&pndef_MotPrmSumChkEnb)) 			   /* PnFFF 							*/
		 && (Hmsg->f.LnkMsgIF == TRUE) )									   /* SVPRM_WRuse	 					*/
		{
			;
		}
		else if( (OpeReg.PrmAccess == FALSE) &&
				((RegNoX + Number) >= 0x0D00) && (Hmsg->AccessLvl != ACCLVL_SYSTEM) )
		{
			i = (RegNoX >= 0x0D00)? 0 : (0x0D00 - RegNoX);
			Hmsg->ErrRno = RegNo + i;					/* Abnormal register number							*/
			return( REGIF_REGNOERR );					/* Register number error							*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Register data write protection check															*/
/*--------------------------------------------------------------------------------------------------*/
//		if( RndWrtDisable.FunA.f.PrmReg || RndWrtDisable.FunB.f.PrmReg )
//		{
//			Hmsg->ErrRno = (USHORT)RegNo;				/* Abnormal register number							*/
//			return( REGIF_REGNOERR );					/* Register number error							*/
//		}
/*--------------------------------------------------------------------------------------------------*/
/*		Write register data																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < Number; i++, idx++ )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	TODO: Check parameter access level													*/
		/*------------------------------------------------------------------------------------------*/
		/*------------------------------------------------------------------------------------------*/
		/*	Continuity check of register numbers															*/
		/*------------------------------------------------------------------------------------------*/
//<S073>			if( pObj->GetNumber( pObj, idx, Hmsg ) != (RegNoX + i) )
			if( (pObj->GetNumber( pObj, idx, Hmsg ) +subidx) != (RegNoX + i) )	/* <S073> */
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				return( REGIF_REGNOERR );				/* Register number error(REGNumber non-consecutive)			*/
			}
#if 1	/* <S073> Array */
		/*------------------------------------------------------------------------------------------*/
		/*	Parameter write																		*/
		/*------------------------------------------------------------------------------------------*/
			do {
				/*----------------------------------------------------------------------------------*/
				/*	Write data is long?															*/
				/*----------------------------------------------------------------------------------*/
				if( pObj->isLong( pObj, idx, Hmsg ) )
				{
					if( (i + subidx + 1) < Number )
					{
						pn_data = ULONGOF_REGDATA( WrBuf[i+subidx], WrBuf[i+subidx+1] );
					}
					else
					{
						Hmsg->ErrRno = RegNo + i;			/* Abnormal register number						*/
						return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)				*/
					}
				}
				else
				{
					pn_data = WrBuf[i];
					if( CHK_PNPRM_SIGNED( GET_PNPRMDEF( pTbl, idx ) ) )
					{
						pn_data = (LONG)((SHORT)pn_data); 	/* For signed words, sign extend	*/
					}
				}
				/*----------------------------------------------------------------------------------*/
				/*	ParameterRAMWrite																*/
				/*----------------------------------------------------------------------------------*/
				if( pObj->isLong( pObj, idx, Hmsg ) )
				{
					rc = OBJ_WRITE_SUB( pObj, idx, subidx, pn_data, Hmsg );		/* @@ */
					i++;
					subidx++;
				}
				else
				{
					rc = OBJ_WRITE_SUB( pObj, idx, subidx, pn_data, Hmsg );		/* @@ */
				}
				/*----------------------------------------------------------------------------------*/
				/*	Error Check																		*/
				/*----------------------------------------------------------------------------------*/
				if(  rc != OK )
				{
					Hmsg->ErrRno = RegNo + i;				/* Abnormal register number						*/
					switch(rc)
					{
					case OERR_DFIT:
						 return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)				*/
//						 break;//<S0C7>statement is unreachable
					case OERR_RONLY:
						 return( REGIF_REGNOERR );			/* Register number error						*/
//						 break;//<S0C7>statement is unreachable
					case OERR_CALC:
						 return( REGIF_DACALERR );			/* Data abnormality(Operation error) 				*/
//						 break;//<S0C7>statement is unreachable
					case OERR_RANGE:
						 return( REGIF_DALMTERR );			/* Data abnormality(Upper/lower limit error)				*/
//						 break;//<S0C7>statement is unreachable
					case OERR_PRMCOPY:
					case OERR_WRITE_HANDLE:
						 return( REGIF_FOPEERR3 );			/* Processing not possible(Processing conflict with other CH)			*/
//						 break;//<S0C7>statement is unreachable
					default:
						 return( REGIF_REGNOERR );			/* Register number error						*/
//						 break;//<S0C7>statement is unreachable
					}
				}
			} while ((i < Number) && (subidx < Lpx_GetMaxSubIdx( pTbl, idx ) ) );
			subidx = 0;
#else	
		/*------------------------------------------------------------------------------------------*/
		/*	Write data is long?																	*/
		/*------------------------------------------------------------------------------------------*/
			if( pObj->isLong( pObj, idx, Hmsg ) )
			{
				if( (i + 1) < Number )
				{
					pn_data = ULONGOF_REGDATA( WrBuf[i], WrBuf[i+1] );
				}
				else
				{
					Hmsg->ErrRno = RegNo + i;			/* Abnormal register number							*/
					return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)					*/
				}
			}
			else
			{
				pn_data = WrBuf[i];
				if( CHK_PNPRM_SIGNED( GET_PNPRMDEF( pTbl, idx ) ) )
				{
					pn_data = (LONG)((SHORT)pn_data); 	/* For signed words, sign extend	*/
				}
			}
		/*------------------------------------------------------------------------------------------*/
		/*	ParameterRAMWrite																		*/
		/*------------------------------------------------------------------------------------------*/
			rc = OBJ_WRITE( pObj, idx, pn_data, Hmsg );
			if(  rc != OK )
			{
				Hmsg->ErrRno = RegNo + i;				/* Abnormal register number							*/
				switch(rc)
				{
				case OERR_DFIT:
					 return( REGIF_DAMTHERR );			/* Data consistency error(LONGBoundary)					*/
					 break;
				case OERR_RONLY:
					 return( REGIF_REGNOERR );			/* Register number error							*/
					 break;
				case OERR_CALC:
					 return( REGIF_DACALERR );			/* Data abnormality(Operation error) 					*/
					 break;
				case OERR_RANGE:
					 return( REGIF_DALMTERR );			/* Data abnormality(Upper/lower limit error)					*/
					 break;
				case OERR_PRMCOPY:
				case OERR_WRITE_HANDLE:
					 return( REGIF_FOPEERR3 );			/* Processing not possible(Processing conflict with other CH)				*/
					 break;
				default:
					 return( REGIF_REGNOERR );			/* Register number error							*/
					 break;
				}
			}
		/*------------------------------------------------------------------------------------------*/
		/*	TODO: ParameterEEPROMWrite															*/
		/*------------------------------------------------------------------------------------------*/

		/*------------------------------------------------------------------------------------------*/
		/*	LONGFor data																		*/
		/*------------------------------------------------------------------------------------------*/
			if( pObj->isLong( pObj, idx, Hmsg ) )
			{
				i++;
			}
#endif
		}
/*--------------------------------------------------------------------------------------------------*/
		return( REGIF_COMPLETE );
}


/****************************************************************************************************/
/*																									*/
/*		Read Memory Object																			*/
/*																									*/
/****************************************************************************************************/
LONG	LpxReadMemObject(					/* Read Memory Object									*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of read data(WORDpiece number)							*/
		USHORT	*RdBuf,						/* Read data buffer									*/
		HMSGIF	Hmsg			)			/* ReadingIFHandle										*/
{
LONG	i;
LONG	pn_data;
COBJECT *pObj = Lpx_GetObject( RegNo );		/* Object Pointer */
LONG	idx = RegNo - 0x4000;				/* Trace Data Index */
/*--------------------------------------------------------------------------------------------------*/
/*		Register data read																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < Number; i++, idx++ )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	Parameter readout																		*/
		/*------------------------------------------------------------------------------------------*/
			OBJ_READ( pObj, idx, (ULONG*)&pn_data, Hmsg );
			RdBuf[i] = (USHORT)(pn_data);			
		}
/*--------------------------------------------------------------------------------------------------*/
		return( REGIF_COMPLETE );
}

/****************************************************************************************************/
/*																									*/
/*		Write Memory Object																			*/
/*																									*/
/****************************************************************************************************/
LONG	LpxWriteMemObject(					/* Write Memory Object									*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of write data(WORDpiece number)							*/
		USHORT	*WrBuf,						/* Write data buffer								*/
		HMSGIF	Hmsg			)			/* WriteIFHandle									*/
{
LONG	i;
ULONG	pn_data;
COBJECT *pObj = Lpx_GetObject( RegNo );		/* Object Pointer */
LONG	idx = RegNo - 0x4000;				/* Trace Data Index */
/*--------------------------------------------------------------------------------------------------*/
/*		Write register data																		*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < Number; i++, idx++ )
		{
			pn_data = WrBuf[i];
		/*------------------------------------------------------------------------------------------*/
		/*	ParameterRAMWrite																		*/
		/*------------------------------------------------------------------------------------------*/
			OBJ_WRITE( pObj, idx, pn_data, Hmsg );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( REGIF_COMPLETE );
}




 /****************************************************************************************************/
/*																									*/
/*		Memobus:IngramofMemobus																	*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S150> */
void	LpxMbusTstUsbProcedure( HMSGIF Hmsg )
{
LONG	MsgLen;
CSIF	*Sif = Hmsg->Sif;
USB_MBUS_SERVER *pUsbMbusServer = (USB_MBUS_SERVER *)Sif->Com;
	MsgLen = MbusFuncMsgProcedure( &(pUsbMbusServer->MbusIf) ); 				/* Message analysis & execution processing */
	MlibCopyByteMemory( Hmsg->ResBuf, pUsbMbusServer->MbusIf.TxBuf, MsgLen  );	/* TODO: Delete it later */
	MSGIF_SEND( Hmsg->ResBuf, MsgLen );											/* Start Tx */
	return;
}
#endif	/* <S150> */

 /****************************************************************************************************/
/*																									*/
/*		Memobus:IngramofMemobus																	*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S150> */
void	LpxMbusTstUartProcedure( HMSGIF Hmsg )
{
LONG	MsgLen;
CSIF	*Sif = Hmsg->Sif;
UART_MBUS_SERVER *pUartMbusServer = (UART_MBUS_SERVER *)Sif->ComMbus42;
	MsgLen = MbusFuncMsgProcedure( &(pUartMbusServer->MbusIf) ); 				/* Message analysis & execution processing */
	MsgLen += 2;
	MlibSetCRC16(pUartMbusServer->MbusIf.TxBuf, MsgLen);
	Hmsg->ErrRno = pUartMbusServer->MbusIf.ErrRno;								/*  register read:MEMOBUSError information */	/* <S056> */
	MlibCopyByteMemory( Hmsg->ResBuf, pUartMbusServer->MbusIf.TxBuf, MsgLen  );	/* TODO: Delete it later */
	MSGIF_SEND( Hmsg->ResBuf, MsgLen );											/* Start Tx */
	return;
}
#endif	/* <S150> */

/* <S150> >>>>> */
/****************************************************************************************************/
/*																									*/
/*		MEMOBUS MESSAGE MANAGER																		*/
/*																									*/
/****************************************************************************************************/
void LpxMbusMechatrolink( HMSGIF Hmsg, LONG *MsgLen )
{
	LONG	SubFuncCode;
	UCHAR	*CmdBuf;

	CmdBuf = Hmsg->CmdBuf;		/* Received message start address */
	SubFuncCode = (LONG)CmdBuf[4];	/* Subfunction code   */
	
	//KpiRstLongTimer( &(MbusIfPtr->ComTimeLong) );		/* For command response timeout	*/
	#if 0 /* Write prohibition currently not supported */
	RpiSetAccsLvl( &Access, MsgPtr->AccsLvl );
	#endif

	switch( SubFuncCode )/* Subfunction code */
	{
		case MBUS_RD_CMEM_42H:		/* 01H Reading from memory (continuous) */
			mbusFuncReadMemory42H( Hmsg, SubFuncCode, MsgLen );
			break;
		case MBUS_WR_CMEM_42H:		/* 02H Memory writing (continuous)	*/
			mbusFuncWriteMemory42H( Hmsg, SubFuncCode, MsgLen );
			break;
		case MBUS_RD_NCMEM_42H:		/* 03H Reading from memory (non-continuous)*/
			mbusFuncReadMemory42H( Hmsg, SubFuncCode, MsgLen );
			break;
		case MBUS_WR_NCMEM_42H:		/* 04H Memory writing (non-continuous)*/
			mbusFuncWriteMemory42H( Hmsg, SubFuncCode, MsgLen );
			break;
		case MBUS_RD_MAXSIZE_42H:	/* 11H Maximum size readout		*/
			mbusFuncReadMaxSize42H( Hmsg, SubFuncCode, MsgLen );
			break;

/* <S061>	Start */
//#ifdef _INFO_ID_READ_CHECK_	/* <S205> Delete */
		case MBUS_RD_INFO_42H:	/* ffH(Undefined) Product information reading	*/
			mbusFuncReadProductInfo42H( Hmsg, SubFuncCode, MsgLen );
			break;
//#endif						/* <S205> Delete */
/* <S061>	End */

		default:					/* Command mismatch				*/
			mbusFuncErrorManager42H( Hmsg, FUNCCODE_ERROR, 0, MsgLen );
			break;
	}

	/* Transmission data length check */
	if(( *MsgLen > MBUS_MAXSIZE ) || ( *MsgLen < MBUS_MINSIZE ) )
	{
		/* Transmission data length error	*/
		mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, 0, MsgLen );
	}

	 /* If the command is not from the mechatronic link,CRC */
	 if( !Hmsg->f.MlnkMsgIF )
	 {
		/* CRCAdditional */
		*MsgLen += 2;
		MlibSetCRC16MB( Hmsg->ResBuf, *MsgLen );
	 }
}


/****************************************************************************************************/
/*																									*/
/*		MEMOBUS MESSAGE MANAGER																		*/
/*																									*/
/****************************************************************************************************/
void LpxMbusLink( HMSGIF Hmsg )
{
	LONG	Offset;
	LONG	RouteNum;
	LONG	Length;
	LONG	MsgLen;
	UCHAR	*CmdBuf;
	UCHAR	*ResBuf;
	USHORT	RelayHdSize;

	/* Initializing variables */
	MsgLen = 0;

	CmdBuf = Hmsg->CmdBuf;		/* Received message start address */
	ResBuf = Hmsg->ResBuf;		/* Send message start address */

	/* Number of relay stages	*/
	RouteNum = (CmdBuf[5] & 0x0F);				

	/* Relay command length */
	RelayHdSize = (USHORT)(RouteNum * MBUS_RLY_PASS_DATA_SIZE + MBUS_RLY_FIX_SIZE);

	/* Get receive/send buffer pointer	*/
	Hmsg->CmdBuf = CmdBuf + RelayHdSize;
	Hmsg->ResBuf = ResBuf + RelayHdSize;

	/* Calculate header offset length */
	Offset = RelayHdSize;

	/* Data section copy */
	MlibCopyByteMemory( ResBuf, CmdBuf, RelayHdSize);

	ResBuf[4] |= 0x80;					/* Response flag						*/
	ResBuf[5]  = (UCHAR)RouteNum;		/* Clear the number of passing stages&Number of relay stages set	*/

	/* Number of relay stages exceeded or Error if the number of relay stages and the number of passing stages are different */
	if( (RouteNum > MBUS_RLY_MAX_ROUTENUM) || (RouteNum != (CmdBuf[5] >> 4)) )
	{/* Number of relay stages error */

		/* Received message error */

		/* Return to beginning of message */
		Hmsg->CmdBuf = CmdBuf;		/* Received message start address */
		Hmsg->ResBuf = ResBuf;		/* Send message start address */

		/* Data transmission (no response) */
		MSGIF_SEND( Hmsg->ResBuf, MsgLen );

		return;
	}
	else
	{/* Number of relay stages normal */

		/* Receive data size(Byte unit) */
		Hmsg->CmdMsgLen =(USHORT)( ((ULONG)CmdBuf[Offset-4] << 24) 
								 | ((ULONG)CmdBuf[Offset-3] << 16) 
								 | ((ULONG)CmdBuf[Offset-2] <<  8) 
								 | ((ULONG)CmdBuf[Offset-1] <<  0) );

		/* Receive data length(Byte unit)Check */
		if(( Hmsg->CmdMsgLen < MBUS_MINSIZE ) || ( Hmsg->CmdMsgLen > MBUS_MAXSIZE ))
		{
			/* Received message error */

			/* Return to beginning of message */
			Hmsg->CmdBuf = CmdBuf;		/* Received message start address */
			Hmsg->ResBuf = ResBuf;		/* Send message start address */

			/* Data transmission (no response) */
			MSGIF_SEND( Hmsg->ResBuf, MsgLen );

			return;	
		}
#ifdef __MYAD_CHECK__
		/* Own station address check	*/
		else if( (FALSE == mbusFuncChkMyMsg( Hmsg, CmdBuf )) 
			  || (FALSE == mbusFuncChkMyMsg( Hmsg, Hmsg->CmdBuf )))
		{
			/* Return to beginning of message */
			Hmsg->CmdBuf = CmdBuf;		/* Received message start address */
			Hmsg->ResBuf = ResBuf;		/* Send message start address */

			/* Data transmission (no response) */
			MSGIF_SEND( Hmsg->ResBuf, MsgLen );

			return;
		}
#endif
		/* Function code check */
		else if( Hmsg->CmdBuf[1] != MBUS_FC_42H )
		{
			/* Function code error */
			mbusFuncErrorManager42H( Hmsg, FUNCCODE_ERROR, 0, &MsgLen );
		}
#ifdef __EXAD_CHECK__
		/* Extended address check */
		else if(Hmsg->CmdBuf[2] != 0xFF)
		{
			mbusFuncErrorManager42H( Hmsg, SELAXIS_ERROR, 0, &MsgLen );
		}
#endif
		else
		{
			/* Message processing execution */
			LpxMbusMechatrolink( Hmsg, &MsgLen );
		}

		/* Return to beginning of message */
		Hmsg->CmdBuf = CmdBuf;		/* Received message start address */
		Hmsg->ResBuf = ResBuf;		/* Send message start address */

		/* Add relay command size to send data size */
		MsgLen += RelayHdSize;

		/* Calculate data section size */
		Length = MsgLen - Offset;

		/* Send data size(Byte unit) */
		mbusFuncSetArrayLongData( &Hmsg->ResBuf[ Offset - 4 ], (ULONG)Length, sizeof(LONG) );

		/* Data transmission */
		MSGIF_SEND( Hmsg->ResBuf, MsgLen );
	}
}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Memory read (continuous/non-continuous common)													*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncReadMemory42H( HMSGIF Hmsg, LONG SubFuncCode, LONG *TrnbLength )
{
#ifdef __RCVLEN_CHECK__
	LONG	CmdLength;
#endif
//	UCHAR	Mode; 
	UCHAR	DataType;
	UCHAR	dIndex;
	USHORT	DataNum;
	UINT	AccessSize,RegSize;
	REG_ACC_T AccessPrm;
	LONG	i;
	LONG	ret;
	LONG	Value;
	LONG	Offset;
	LONG	RspLength;
	LONG	BlockNum;
	ULONG	BeginAddress;
	ULONG	CurrentAddress;
	REG_MANAGER_HANDLE	*hRegMngr;

	/* Receive, send data start address */
	UCHAR  *CmdBuf;	
	UCHAR  *ResBuf;

	CmdBuf = Hmsg->CmdBuf;
	ResBuf = Hmsg->ResBuf;

	Value  = 0;
	//Mode      = (CmdBuf[5] >> 4);					/* Mode		*/
	DataType  = (CmdBuf[5] & 0x0F);					/* DataType */
	DataNum   = (CmdBuf[6] << 8) + CmdBuf[7];		/* DataNum	*/
	

	/* Data type check */
	if( (DataType != 1) && (DataType != 2) && (DataType != 3))	
	{
		/* Mode/Data type error*/
		mbusFuncErrorManager42H( Hmsg, MODETYPE_ERROR, 0, TrnbLength );
		return;	
	}
	AccessSize   = (UCHAR)(0x0001 << (DataType-1));					/* Access size		  */
	BeginAddress = mbusFuncLongSwap( &CmdBuf[MBUS_HEAD_FIX_SIZE]);	/* Reading data address */


	/* Check receive data length, send data length, number of data pieces */
	if (CmdBuf[4] == MBUS_RD_CMEM_42H)
	{/* Continuous memory read */ 
#ifdef __RCVLEN_CHECK__
		CmdLength = MBUS_HEAD_BASE_SIZE;	/* Receive data length(Message total length) */
#endif
		RspLength = AccessSize * DataNum;	/* Transmission data length(Data section)		*/	
	}
	else
	{/* Discontinuous memory read */ 
		//CmdLength = MBUS_HEAD_FIX_SIZE + 4 * DataNum;
		RspLength = 4 * DataNum;		
	}
#ifdef __RCVLEN_CHECK__
	if(  (CmdLength != Hmsg->CmdMsgLen )
		|| (RspLength > (MBUS_MAXSIZE - MBUS_HEAD_FIX_SIZE)) || (DataNum < MBUS_MIN_REGNUM)) 
#else
	if( (RspLength > (MBUS_MAXSIZE - MBUS_HEAD_FIX_SIZE)) || (DataNum < MBUS_MIN_REGNUM)) 
#endif
	{	/* Invalid quantity error */
		mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, BeginAddress, TrnbLength );
		return;
	}

	for ( i = 0; i < DataNum; i++ )
	{
		if (CmdBuf[4] == MBUS_RD_CMEM_42H)
		{/* Continuous memory read */ 
			CurrentAddress = BeginAddress + (AccessSize * i);/* Reading data address	*/
		}
		else
		{/* Discontinuous memory read */ 
			CurrentAddress = mbusFuncLongSwap( &CmdBuf[MBUS_HEAD_FIX_SIZE + 4*i] );			
		}

		/* virtual memory→Real address conversion */
		AccessPrm.MemAddr = CurrentAddress;
		//hRegMngr = mbusFuncConvAddress(MbusIfPtr->AxisMap,&AccessPrm.MemAddr,&ret);		
		hRegMngr = mbusFuncConvAddress(Hmsg->AxisMap,&AccessPrm.MemAddr);		
		if( hRegMngr == NULL )
		{	/* Memory address error */
			//mbusFuncErrorManager42H( MbusIfPtr, ret, CurrentAddress );//kira_1202
			mbusFuncErrorManager42H( Hmsg, SELAXIS_ERROR, CurrentAddress, TrnbLength );
			return;
		}

		/* Calculate the offset to the data position to be stored	*/
		Offset = MBUS_HEAD_FIX_SIZE + i*AccessSize;

		/* Get parameter attribute */
		if( NULL == RegMngrGetAttrByMemAddr(hRegMngr, &AccessPrm) )
		{	/* Memory address error	*/
			mbusFuncErrorManager42H( Hmsg, MEMADRES_ERROR, CurrentAddress, TrnbLength );
			return;
		}
		/* Read parameter size */
		RegSize = AccessPrm.cPrmDef->DataLength;

		/*------------------------------------------------------------------------------------------*/
		/*	Block reading																		*/
		/*------------------------------------------------------------------------------------------*/
		BlockNum = RegSize/AccessSize;		/* Block read count */

		if( (BlockNum > 0) && ( DataNum > (i + BlockNum - 1)) )
		{
			if(SubFuncCode == MBUS_RD_NCMEM_42H)
			{/* Discontinuous memory read */

				dIndex = 4;

				/* Address continuity check */
				ret = mbusFuncCheckBlockAccess( &CmdBuf[MBUS_HEAD_FIX_SIZE + dIndex*i], 
																CurrentAddress, AccessSize,BlockNum,dIndex);
				if( ret != 0 )
				{	
					/* Error if address specification is not consecutive */
					mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, CurrentAddress, TrnbLength );
					return;
				}
			}
			i = i + (BlockNum-1);				/* Block read counti	*/
		}
		else
		{	
			/* Quantity defect */
			mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, CurrentAddress, TrnbLength );
			return;
		}

		/* Data reading */
		AccessPrm.IfHandle = Hmsg;
		ret = (LONG)PrmReadValue( &AccessPrm, &Value, RegSize);
		if( PRM_RSLT_SUCCESS != ret )
		{
			mbusFuncErrorManager42H( Hmsg, ret, CurrentAddress, TrnbLength );
			return;
		}

		/* Send data set */
		switch (RegSize)
		{
			case 1:		/* bytetype */
				ResBuf[Offset] = (UCHAR)(Value);
				break;
			case 2:		/* shorttype */
				mbusFuncSetArrayShortData( &ResBuf[Offset], (USHORT)Value, AccessSize );
				break;
			case 4:		/* longtype */
				mbusFuncSetArrayLongData( &ResBuf[Offset], (ULONG)Value, AccessSize );
				break;
		}
	}

	MlibCopyByteMemory( ResBuf, CmdBuf, MBUS_HEAD_FIX_SIZE);			/* Response data creation */
	*TrnbLength = MBUS_HEAD_FIX_SIZE + AccessSize*DataNum;	/* Determine transmit buffer size */
}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Memory write (continuous/non-continuous common)													*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncWriteMemory42H( HMSGIF Hmsg, LONG SubFuncCode, LONG *TrnbLength )
{
#ifdef __RCVLEN_CHECK__
	LONG	CmdLength;
#endif
//	UCHAR	Mode;
	USHORT	DataNum;
	UCHAR	AccessSize;
	UCHAR	DataType;
	UCHAR	dIndex;
	REG_ACC_T AccessPrm;
	LONG	i;
	LONG	ret;
	LONG	Value;
	LONG	Offset;
	LONG	MaxSize;
	LONG	RspLength;
	LONG	BlockNum;
	ULONG	BeginAddress; 
	ULONG	CurrentAddress;
	UINT	RegSize;
	REG_MANAGER_HANDLE	*hRegMngr;

	UCHAR  *CmdBuf;	/* Receive buffer */
	UCHAR  *ResBuf;	/* Transmission buffer */

	/* Receive, send data start address stored */
	CmdBuf = Hmsg->CmdBuf;
	ResBuf = Hmsg->ResBuf;

//	Mode     = (CmdBuf[5] >> 4);					/* Mode	*/
	DataType = (CmdBuf[5] & 0x0F);					/* Data type */
	DataNum  = (CmdBuf[6] << 8) + CmdBuf[7];		/* Data count	*/

	/* Data type check */
	if( (DataType != 1) && (DataType != 2) && (DataType != 3))	
	{	/* Mode/Data type error*/
		mbusFuncErrorManager42H( Hmsg, MODETYPE_ERROR, 0, TrnbLength );
		return;	
	}
	AccessSize   = (UCHAR)(0x0001 << (DataType-1));					/* Access size			*/
	BeginAddress = mbusFuncLongSwap( &CmdBuf[MBUS_HEAD_FIX_SIZE] );	/* Write data address	*/

	/* Check receive data length, send data length, number of data pieces */
	if (CmdBuf[4] == MBUS_WR_CMEM_42H)
	{/* Continuous memory write */
#ifdef __RCVLEN_CHECK__
		CmdLength = MBUS_HEAD_BASE_SIZE + AccessSize * DataNum;	/* Receive data length(Message total length) */
#endif
		RspLength = AccessSize * DataNum;						/* Transmission data length(Data section)		*/
		MaxSize   = MBUS_MAXSIZE - MBUS_HEAD_BASE_SIZE;			/* Maximum buffer size(Data section)	*/
	}
	else
	{/* Discontinuous memory write */ 
#ifdef __RCVLEN_CHECK__
		CmdLength = MBUS_HEAD_FIX_SIZE + 8 * DataNum;
#endif
		RspLength = 8 * DataNum;
		MaxSize   = MBUS_MAXSIZE - MBUS_HEAD_FIX_SIZE;
	}

#ifdef __RCVLEN_CHECK__
	if( (Hmsg->CmdMsgLen != CmdLength ) || (RspLength > MaxSize) || (DataNum < MBUS_MIN_REGNUM) )
#else
	if( (RspLength > MaxSize) || (DataNum < MBUS_MIN_REGNUM) )
#endif
	{	/* Quantity defect */
		mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, BeginAddress,TrnbLength );		
		return;
	}

	for ( i = 0; i < DataNum; i++ )
	{ 
		if (CmdBuf[4] == MBUS_WR_CMEM_42H)
		{/* Continuous memory write */
			CurrentAddress   = BeginAddress + (AccessSize * i);	/* Write data address	*/
			Offset = MBUS_HEAD_BASE_SIZE + i*AccessSize;		/* Write data offset */
			dIndex = AccessSize;
		}
		else
		{/* Discontinuous memory write */ 
			CurrentAddress = mbusFuncLongSwap( &CmdBuf[MBUS_HEAD_FIX_SIZE + 8*i] );
			Offset = MBUS_HEAD_BASE_SIZE + i*sizeof(LONG)*2;
			dIndex = sizeof(LONG)*2;
		}

		/* virtual memory → Real memory address conversion */
		AccessPrm.MemAddr = CurrentAddress;
		hRegMngr = mbusFuncConvAddress(Hmsg->AxisMap,&AccessPrm.MemAddr);		
		if( hRegMngr == NULL )
		{	/* Memory address error */
			mbusFuncErrorManager42H( Hmsg, SELAXIS_ERROR, CurrentAddress, TrnbLength );
			return;
		}

		/* Get parameter attribute */
		if( NULL == RegMngrGetAttrByMemAddr(hRegMngr, &AccessPrm) )
		{	/* Memory address error	*/
			mbusFuncErrorManager42H( Hmsg, MEMADRES_ERROR, CurrentAddress, TrnbLength );
			return;
		}

		/* Parameter size acquisition */
		RegSize = AccessPrm.cPrmDef->DataLength;

		/*------------------------------------------------------------------------------------------*/
		/*	Block write																		*/
		/*------------------------------------------------------------------------------------------*/
		BlockNum = RegSize/AccessSize;		/* Block write count */

		if( (BlockNum > 0) && ( DataNum > (i + BlockNum-1)) )
		{
			if(SubFuncCode == MBUS_WR_NCMEM_42H)
			{/* Discontinuous memory write */

				/* Address continuity check */
				ret = mbusFuncCheckBlockAccess( &CmdBuf[MBUS_HEAD_FIX_SIZE + dIndex*i], 
																CurrentAddress, AccessSize, BlockNum, dIndex);
				if( ret != 0 )
				{	/* Error if address specification is not consecutive */
					mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, CurrentAddress, TrnbLength );
					return;
				}
			}
			i = i + (BlockNum-1);	/* Block write counti	*/
			Value = 0;				/* Write value initialization						*/

			switch(AccessSize)
			{
				case 1:
					Value = mbusFuncSetValueByteData( &CmdBuf[Offset], BlockNum, dIndex );
					break;
				case 2:
					Value = mbusFuncSetValueShortData( &CmdBuf[Offset], BlockNum, dIndex );
					break;
				case 4:
					Value = mbusFuncSetValueLongData( &CmdBuf[Offset], BlockNum, dIndex );
					break;
				default:/* It will not come here */
					break;
			}
		}
		else
		{	/* Quantity defect */
			mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, CurrentAddress, TrnbLength );
			return;
		}

		/*------------------------------------------------------------------------------------------*/
		/*	<S166> Obtain operation register write right														*/
		/*------------------------------------------------------------------------------------------*/
		ret = mbusCheckOperationWriteHandle( &AccessPrm, Hmsg );
		if ( ret != OK ) { 
			mbusFuncErrorManager42H( Hmsg, COMPETON_ERROR, CurrentAddress, TrnbLength );
		}
		/*------------------------------------------------------------------------------------------*/

		/* Data write */
		AccessPrm.IfHandle = Hmsg;
		ret = (LONG)PrmWriteValue( &AccessPrm, &Value, RegSize);
		if(0 > ret)
		{
			mbusFuncErrorManager42H( Hmsg, ret, CurrentAddress, TrnbLength );
			return;
		}

		/*------------------------------------------------------------------------------------------*/
		/*	<S166> Release operation register write right														*/
		/*------------------------------------------------------------------------------------------*/
		mbusFreeOperationWriteHandle( &AccessPrm, Hmsg );
	}

	MlibCopyByteMemory( ResBuf, CmdBuf, MBUS_HEAD_FIX_SIZE);	/* Response data creation */
	*TrnbLength = MBUS_HEAD_FIX_SIZE;							/* Determine transmit buffer size */

}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Maximum size read																	*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncReadMaxSize42H( HMSGIF Hmsg, LONG SubFuncCode, LONG *TrnbLength )
{
	LONG	MsgSize;

	UCHAR  *CmdBuf;//Receive buffer start pointer
	UCHAR  *ResBuf;//Send buffer start pointer

	/* Receive, send data start address stored */
	CmdBuf = Hmsg->CmdBuf;
	ResBuf = Hmsg->ResBuf;

#ifdef __RCVLEN_CHECK__
	/*----------------------------------------------------------------------------------------------*/
	/*		Check receive data length																	*/
	/*----------------------------------------------------------------------------------------------*/
	/* Invalid receive data length */
	if( Hmsg->CmdMsgLen != MBUS_HEAD_FIX_SIZE )
	{	/* Invalid quantity error	*/
		mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, BeginAddress, TrnbLength );
		return;
	}
#endif

	/* Maximum size readout size */
	MsgSize = MBUS_MAXSIZE - MBUS_HEAD_FIX_SIZE;

	/* Response data creation */
	ResBuf[0]  = CmdBuf[0];								/* 0:Slave Addrress		*/
	ResBuf[1]  = CmdBuf[1];								/* 1:Function Code		*/
	ResBuf[2]  = CmdBuf[2];								/* 2:Extended Address	*/
	ResBuf[3]  = CmdBuf[3];								/* 3:Reserve1			*/
	ResBuf[4]  = CmdBuf[4];								/* 4:Sub Function Code	*/
	ResBuf[5]  = CmdBuf[5];								/* 5:Mode,DataType 		*/
	ResBuf[6]  = 0;										/* 6:DataNumH			*/
	ResBuf[7]  = 0;										/* 7:DataNumL			*/
	mbusFuncSetArrayLongData( &ResBuf[8], (ULONG)MsgSize, 4);	/* 8-11 MaxMessageSize	*/

	/* Determine transmit buffer size */
	*TrnbLength = MBUS_HEAD_BASE_SIZE;

}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Abnormal response reply processing																	*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncErrorManager42H( HMSGIF Hmsg, LONG ErrorCode, ULONG ErrorAddress, LONG *TrnbLength )
{
	UCHAR  *CmdBuf;
	UCHAR  *ResBuf;

	/* Receive, send data start address */
	CmdBuf = Hmsg->CmdBuf;
	ResBuf = Hmsg->ResBuf;

	/* Create abnormal response data */
	ResBuf[0]  =  CmdBuf[0];						/* 0:Slave Addrress		*/
	ResBuf[1]  = (CmdBuf[1] | 0x80);				/* 1:Function Code		*/
	ResBuf[2]  =  CmdBuf[2];						/* 2:Extended Address	*/
	ResBuf[3]  =  CmdBuf[3];						/* 3:Reserve1			*/
	ResBuf[4]  =  CmdBuf[4];						/* 4:Sub Function Code	*/
	ResBuf[5]  =  (UCHAR)ErrorCode;					/* 5:Error Code			*/
	ResBuf[6]  =  0x00;								/* 6:Reserve2			*/
	ResBuf[7]  =  0x00;								/* 7:Reserve3			*/
	mbusFuncSetArrayLongData( &ResBuf[8], ErrorAddress, sizeof(LONG) );	/* 8-11:ErrorAddress	*/

	/* Error reply size=12byte	*/
	*TrnbLength = MBUS_HEAD_BASE_SIZE;

	/* register read:MEMOBUSError information(Error occurrence address) */
	Hmsg->ErrRno = ErrorAddress;
}

/****************************************************************************************************/
/*																									*/
/*		Message buffer swap function(ULONG)														*/
/*																									*/
/****************************************************************************************************/
static LONG	mbusFuncLongSwap( void *Buf )
{
	ULONG data;
#ifdef __BIG_ENDIAN__	
	/* BigEndian */
	data =  (ULONG)((*( UCHAR *)Buf	     << 24) 
		 	      + (*((UCHAR *)Buf + 1) << 16)
			      + (*((UCHAR *)Buf + 2) <<  8)
				  + (*((UCHAR *)Buf + 3)      ) );
#else					
	/* LittleEndian */
	/* Unconfirmed */
	//↓is the opposite
	//data = (ULONG)((*( UCHAR *)Buf	         ) 
	//	         + (*((UCHAR *)Buf + 1) <<  8)
	//		     + (*((UCHAR *)Buf + 2) << 16)
	//		  	 + (*((UCHAR *)Buf + 3) << 24) );
	data = MlibCnvBig32( *((ULONG *)Buf) );
#endif
	return data;
}

/****************************************************************************************************/
/*																									*/
/*		Message buffer swap function(USHORT)														*/
/*																									*/
/****************************************************************************************************/
static LONG	mbusFuncShortSwap( void *Buf )
{
	ULONG data;
#ifdef __BIG_ENDIAN__	
	/* BigEndian */
	data = (ULONG)((*( UCHAR *)Buf      << 8) 
		         + (*((UCHAR *)Buf + 1)     ) );
#else					
	/* LittleEndian */
	/* Unconfirmed */
/* <S001> */
//	data = (ULONG)((*( UCHAR *)Buf           ) 
//		 	     + (*((UCHAR *)Buf + 1) << 8 ));
	data = (ULONG)((*( UCHAR *)Buf      << 8) 
		         + (*((UCHAR *)Buf + 1)     ) );
/* <S001> */
#endif
	return data;
}

/****************************************************************************************************/
/*																									*/
/*		Message buffer storage function(ULONG)															*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncSetArrayLongData( UCHAR *Buf, ULONG data, UINT AccessSize )
{
#ifdef __BIG_ENDIAN__
/* BigEndian */
	switch(AccessSize)
	{
		case 1:
			Buf[3] = (UCHAR)((data) >> 24);
			Buf[2] = (UCHAR)((data) >> 16);
			Buf[1] = (UCHAR)((data) >>  8);
			Buf[0] = (UCHAR)( data );
			break;
		case 2:
			Buf[2] = (UCHAR)((data) >> 24);
			Buf[3] = (UCHAR)((data) >> 16);
			Buf[0] = (UCHAR)((data) >>  8);
			Buf[1] = (UCHAR)( data );
			break;
		case 4:
			Buf[0] = (UCHAR)((data) >> 24);
			Buf[1] = (UCHAR)((data) >> 16);
			Buf[2] = (UCHAR)((data) >>  8);
			Buf[3] = (UCHAR)( data );
			break;
		default:
			break;
	}
#else
/* LittleEndian */
/* <S001> */
	/* Unconfirmed */
	//↓Reverse
	//Buf[3] = (UCHAR)((data) >> 24);
	//Buf[2] = (UCHAR)((data) >> 16);
	//Buf[1] = (UCHAR)((data) >>  8);
	//Buf[0] = (UCHAR)( data );
//	*(ULONG *)Buf = MlibCnvBig32( data );

	switch(AccessSize)
	{
		case 1:
			Buf[3] = (UCHAR)((data) >> 24);
			Buf[2] = (UCHAR)((data) >> 16);
			Buf[1] = (UCHAR)((data) >>  8);
			Buf[0] = (UCHAR)( data );
			break;
		case 2:
			Buf[2] = (UCHAR)((data) >> 24);
			Buf[3] = (UCHAR)((data) >> 16);
			Buf[0] = (UCHAR)((data) >>  8);
			Buf[1] = (UCHAR)( data );
			break;
		case 4:
			Buf[0] = (UCHAR)((data) >> 24);
			Buf[1] = (UCHAR)((data) >> 16);
			Buf[2] = (UCHAR)((data) >>  8);
			Buf[3] = (UCHAR)( data );
			break;
		default:
			break;
	}
/* <S001> */
#endif
}

/****************************************************************************************************/
/*																									*/
/*		Message buffer storage function(USHORT)															*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncSetArrayShortData( UCHAR *Buf, USHORT data, UINT AccessSize)
{
#ifdef __BIG_ENDIAN__	
/* BigEndian */
	switch(AccessSize)
	{
		case 1:
			Buf[1] = (UCHAR)((data) >> 8);
			Buf[0] = (UCHAR)( data );
			break;
		case 2:
			Buf[0] = (UCHAR)((data) >> 8);
			Buf[1] = (UCHAR)( data );
			break;
		default:
			break;
	}
#else					
/* LittleEndian */
/* <S001> */
	/* ↓Reverse */
	//Buf[1] = (UCHAR)((data) >> 8);
	//Buf[0] = (UCHAR)( data );
//	Buf[0] = (UCHAR)((data) >> 8);
//	Buf[1] = (UCHAR)( data );

	switch(AccessSize)
	{
		case 1:
			Buf[1] = (UCHAR)((data) >> 8);
			Buf[0] = (UCHAR)( data );
			break;
		case 2:
			Buf[0] = (UCHAR)((data) >> 8);
			Buf[1] = (UCHAR)( data );
			break;
		default:
			break;
	}
/* <S001> */
#endif
}

/****************************************************************************************************/
/*																									*/
/*		virtual memory→Real memory conversion function																*/
/*		(Check address range and store axis number and parameter number)								*/
/*																									*/
/*			Return value:REG_MANAGER_HANDLE															*/
/*																									*/
/****************************************************************************************************/
static REG_MANAGER_HANDLE *mbusFuncConvAddress(MEMOBUS_ADDR_MAP *AxisMap, ULONG *MemAddress )
{
	ULONG	mem_address, i;
	ULONG	AxisNumber;

	mem_address = *MemAddress;
	AxisNumber = (mem_address & 0x00F00000) >> 20;

	if(AxisNumber >= MAX_AXIS_NUMBER)
	{
		return NULL;
	}
	
	for(i = 0; i < MAX_AXIS_NUMBER; i++)
	{
		if(AxisMap[i].AxisAddress == AxisNumber)
		{
			return AxisMap[i].hRegMngr;
		}
	}

	return NULL;
}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Address continuity check (non-continuous read/write common)								*/
/*																									*/
/****************************************************************************************************/
static UCHAR mbusFuncCheckBlockAccess(UCHAR *CmdBuf, ULONG CurrentAddress, 
													UINT AccessSize, LONG BlockNum, LONG dIndex)
{
	LONG  j;
	LONG  cnt;
	ULONG NextAddress;

	cnt=0;
	for( j=0; j<(BlockNum-1); j++ )
	{
		/* Address update	*/
		NextAddress = mbusFuncLongSwap( &CmdBuf[ dIndex*(j+1) ]);
		
		if( (NextAddress - CurrentAddress) == (AccessSize*(j+1)) )
		{/* If address specification is consecutive */
			cnt++;	
		}
	}
	if( cnt != (BlockNum-1) )
	{	/* Error if address specification is not consecutive */
		return MEMADRES_ERROR;
	}
	return 0;
}

/****************************************************************************************************/
/*																									*/
/*		Message data storage function(Byte)																*/
/*																									*/
/****************************************************************************************************/
static LONG	mbusFuncSetValueByteData( UCHAR *Buf, LONG BlockNum, LONG dIndex )
{
	LONG Value,j;
	LONG BitShift;
	Value = 0;

	for( j=0; j<BlockNum; j++ )
	{
		BitShift = 8 * sizeof(CHAR) * j;
		Value += (LONG)(Buf[ dIndex*j ] << BitShift );
	}
	return Value;
}

/****************************************************************************************************/
/*																									*/
/*		Message data storage function(SHORT)																*/
/*																									*/
/****************************************************************************************************/
static LONG	mbusFuncSetValueShortData( UCHAR *Buf, LONG BlockNum, LONG dIndex )
{
	LONG Value,j;
	LONG BitShift;
	Value = 0;

	for( j=0; j<BlockNum; j++ )
	{
		BitShift = 8 * sizeof(SHORT) * j;
		Value += (mbusFuncShortSwap( (void*)Buf ) << BitShift);
		Buf += dIndex;
	}
	return Value;
}

/****************************************************************************************************/
/*																									*/
/*		Message data storage function(LONG)																*/
/*																									*/
/****************************************************************************************************/
static LONG mbusFuncSetValueLongData( UCHAR *Buf, LONG BlockNum, LONG dIndex )
{
	LONG Value,j;
	LONG BitShift;
	Value = 0;

	for( j=0; j<BlockNum; j++ )
	{
		BitShift = 8 * sizeof(LONG) * j;
		Value += (mbusFuncLongSwap( (void*)Buf ) << BitShift);
		Buf += dIndex;
	}
	return Value;
}
/* <<<<< <S150> */

/****************************************************************************************************/
/*																									*/
/*		<S166> Check and Get the Operation Write Handle												*/
/*																									*/
/****************************************************************************************************/
LONG mbusCheckOperationWriteHandle( REG_ACC_T *accessprm, MSGIF *Hmsg )
{
_enum_OPMODE modex;
ULONG  memaddr = accessprm->MemAddr & 0xFF0FFFFF;
PREGIF_CONFIG_T pRegCfg = accessprm->hRegCfg;
AXIS_HANDLE *pAxRsc = (AXIS_HANDLE *)pRegCfg->PrmCalParam;				/* Interim */
/*--------------------------------------------------------------------------------------------------*/
	switch ( memaddr )
	{
	/*----------------------------------------------------------------------------------------------*/
	case 0x80004000:						/* MODE1 */
	/*----------------------------------------------------------------------------------------------*/
		modex = OPMODE_OPE1;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x80004002:						/* MODE1 */
	/*----------------------------------------------------------------------------------------------*/
		modex = OPMODE_OPE1;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x80004008:						/* MODE2 */
	/*----------------------------------------------------------------------------------------------*/
		modex = OPMODE_OPE2;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x8000400A:						/* MODE2 */
	/*----------------------------------------------------------------------------------------------*/
		modex = OPMODE_OPE2;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x8000400C:						/* TRACE */
	/*----------------------------------------------------------------------------------------------*/
		modex = OPMODE_TRACE;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x8000400E:						/* TRACE */
	/*----------------------------------------------------------------------------------------------*/
		modex = OPMODE_TRACE;
		break;
	/*----------------------------------------------------------------------------------------------*/
	default:
	/*----------------------------------------------------------------------------------------------*/
		modex = OPMODE_NULL;
		break;
	}
/*--------------------------------------------------------------------------------------------------*/
	if ( modex != OPMODE_NULL )
	{
		if ( mbusGetOpeRegWriteHandle( pAxRsc->pAccessOp, modex, Hmsg ) != TRUE )
		{
			return (NG);					/* OtherchConflict */
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	return (OK);
}

/****************************************************************************************************/
/*																									*/
/*		<S166> Free Operation Write Handle															*/
/*																									*/
/****************************************************************************************************/
void mbusFreeOperationWriteHandle( REG_ACC_T *accessprm, MSGIF *Hmsg )
{
_enum_OPMODE modex;
FUN_EXECH ch;
ULONG  memaddr = accessprm->MemAddr & 0xFF0FFFFF;
PREGIF_CONFIG_T pRegCfg = accessprm->hRegCfg;
AXIS_HANDLE *pAxRsc = (AXIS_HANDLE *)pRegCfg->PrmCalParam;				/* Interim */
FUN_MANAGER 	*FunMngr = pAxRsc->FnManager;							/* Interim */
/*--------------------------------------------------------------------------------------------------*/
	switch ( memaddr )
	{
	/*----------------------------------------------------------------------------------------------*/
	case 0x80004000:						/* MODE1 */
	/*----------------------------------------------------------------------------------------------*/
		ch = FUN_CH1;
		modex = OPMODE_OPE1;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x80004002:						/* MODE1 */
	/*----------------------------------------------------------------------------------------------*/
		ch = FUN_CH1;
		modex = OPMODE_OPE1;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x80004008:						/* MODE2 */
	/*----------------------------------------------------------------------------------------------*/
		ch = FUN_CH2;
		modex = OPMODE_OPE2;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x8000400A:						/* MODE2 */
	/*----------------------------------------------------------------------------------------------*/
		ch = FUN_CH2;
		modex = OPMODE_OPE2;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x8000400C:						/* TRACE */
	/*----------------------------------------------------------------------------------------------*/
		ch = FUN_CH1;		/* @@ */
		modex = OPMODE_TRACE;
		break;
	/*----------------------------------------------------------------------------------------------*/
	case 0x8000400E:						/* TRACE */
	/*----------------------------------------------------------------------------------------------*/
		ch = FUN_CH1;		/* @@ */
		modex = OPMODE_TRACE;
		break;
	/*----------------------------------------------------------------------------------------------*/
	default:
	/*----------------------------------------------------------------------------------------------*/
		ch = FUN_CH1;		/* @@ */
		modex = OPMODE_NULL;
		break;
	}
/*--------------------------------------------------------------------------------------------------*/
/*	Free Handle																						*/
/*--------------------------------------------------------------------------------------------------*/
	if ( ( modex == OPMODE_OPE1 ) || ( modex == OPMODE_OPE2 ) )
	{
		if ( 0x000 == FunGetOpeModeRegister( FunMngr, ch ) )
		{
			mbusFreeOpeRegWriteHandle( pAxRsc->pAccessOp, modex, Hmsg );
		}
	}
	else if ( modex == OPMODE_TRACE )
	{
		if( 0x000 == FunMngr->AxCommon->TraceMode )
		{
			mbusFreeOpeRegWriteHandle( pAxRsc->pAccessOp, modex, Hmsg );
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}

/****************************************************************************************************/
/*																									*/
/*		<S166> Obtain operation register write right																*/
/*																									*/
/****************************************************************************************************/
LONG	mbusGetOpeRegWriteHandle( OPACCSV *Accs, _enum_OPMODE mode, MSGIF *Hmsg )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Handle check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( Accs->Hmsg[mode] == NULL )
		{
			Accs->Hmsg[mode] = Hmsg;					/* Acquire if the handle is unused				*/
			return( TRUE );								/* Handle acquisition successful							*/
		}
		else if( Accs->Hmsg[mode] == Hmsg )
		{
			return( TRUE );								/* OK if the handles are the same				*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return( FALSE );								/* Handle acquisition failure							*/

}

/****************************************************************************************************/
/*																									*/
/*		Release operation register write right process																*/
/*																									*/
/****************************************************************************************************/
void	mbusFreeOpeRegWriteHandle( OPACCSV *Accs, _enum_OPMODE mode, MSGIF *Hmsg )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Free Operator Write Handle (Called from FunMngr)											*/
/*--------------------------------------------------------------------------------------------------*/
		if( Accs->Hmsg[mode] == Hmsg )
		{
			Accs->Hmsg[mode]   = NULL;					/* Free Register I/F Write Handle			*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;

}

/* <S205> Start */
/****************************************************************************************************/
/*																									*/
/*		Memobus: Read product information																	*/
/*																									*/
/****************************************************************************************************/
static void mbusFuncReadProductInfo42H( HMSGIF Hmsg, LONG SubFuncCode, LONG *TrnbLength )
{
	UCHAR	BlockSize;
	UCHAR	ReadBlockSize;
	UCHAR	ReadSlotNo;
	UCHAR	ReadAxisNo;
	UCHAR	ReadIdNo;

	LONG	CmdBuffOffset;
	LONG	NextCmdBuffOffset;
	LONG	RspBuffOffset;
	LONG	RspBuffSize;
	LONG	ReadBlockStartOffset;


	LONG	ReadIdRegAdd;
	UCHAR	ReadDataSize;
	UCHAR	ReadDataNum;

	LONG	i;
	LONG	j;
	
	LONG	ret;
	LONG	CurrentDataLength;
	LONG	BlockDataSize;


	/* Receive, send data start address */
	UCHAR  *CmdBuf;	
	UCHAR  *ResBuf;

	CmdBuf = Hmsg->CmdBuf;
	ResBuf = Hmsg->ResBuf;

	/* Initializing variables */
	ret = 0;
	NextCmdBuffOffset = 0;							/* Next block number storage buffer offset */
	BlockDataSize = 0;							/* Data size stored in each block(Slot number ~ Final data) */
	RspBuffOffset = MBUS_IDRD_RSP_HEAD_BASE_SIZE;	/* RspBuffData section offset */
	RspBuffSize = MBUS_IDRD_RSP_HEAD_BASE_SIZE;		/* RspBuffData size */
	BlockSize = CmdBuf[MBUS_IDRD_HEAD_BASE_SIZE];	/* Get the number of read blocks */

	/* Block number check  */
	if( BlockSize == 0) 
	{	/* Setting value out of range error */
		mbusFuncErrorManager42H( Hmsg, RANGOUTS_ERROR, 0, TrnbLength );
		return;
	}

	for ( i = 0; i < BlockSize; i++ )
	{
		CmdBuffOffset = MBUS_IDRD_HEAD_BASE_SIZE + 1 + NextCmdBuffOffset;	/* Block size to read is storedCmdBuffOffset calculation */
		ReadBlockSize =  CmdBuf[CmdBuffOffset];								/* Block size to read(ReadID) */

		/* Block size check */
		if( (ReadBlockSize < 4) || (ReadBlockSize > MBUS_IDRD_ID_NUM + 3) )
		{
			/* Invalid quantity error */
			mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, 0, TrnbLength );
			return;
		}
		else
		{
			ReadSlotNo = CmdBuf[CmdBuffOffset + 1];			/* Obtains "slot number" */
			/* Slot number check  */
			if( ReadSlotNo > (MBUS_IDRD_SLOT_NUM - 1) )
			{
				/* Setting value out of range error */
				mbusFuncErrorManager42H( Hmsg, RANGOUTS_ERROR, 0, TrnbLength );
				return;
			}

			ReadAxisNo = CmdBuf[CmdBuffOffset + 2];			/* Getting "axis number" */
			ReadBlockStartOffset = RspBuffOffset;			/* Block read start offset */
			ResBuf[RspBuffOffset + 1] = ReadSlotNo;			/* Stores "slot number" in response buffer */
			ResBuf[RspBuffOffset + 2] = ReadAxisNo;			/* Storing "axis number" in response buffer */

			RspBuffSize = RspBuffSize + 3;					/* Update receive data size */
			RspBuffOffset = RspBuffOffset + 3;				/* Response buffer offset update */

			/* Response buffer size check */
			if( RspBuffSize > MBUS_MAXSIZE )
			{	/* Invalid quantity error */
				mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, 0, TrnbLength );
				return;
			}

			/* Get read product information */
			for ( j = 0; j < (ReadBlockSize - 3 ) ; j++ )		/* ReadBlockSize : Block size ~ FinalID */
			{
				ReadIdNo =  CmdBuf[CmdBuffOffset + (3 + j)];	/* ReadingID */;

				/* ID-No.Check */
				if( ReadIdNo > (MBUS_IDRD_ID_NUM - 1) )
				{
					/* Setting value out of range error */
					mbusFuncErrorManager42H( Hmsg, RANGOUTS_ERROR, 0, TrnbLength );
					return;
				}

				ReadIdRegAdd = IdRegAddTbl[ReadSlotNo][ReadIdNo];		/* ReadingIDregister address acquisition */
				ReadDataSize = IdDataSizeTbl[ReadSlotNo][ReadIdNo];		/* ReadingID */
				ReadDataNum = IdDataNumTbl[ReadSlotNo][ReadIdNo];		/* ReadingID */

				if( (ReadIdRegAdd == 0) && (ReadDataSize == 0) && (ReadDataNum == 0) )	/* ReadingIDNo data */
				{
					CurrentDataLength = 1;								/* Data length = 2[byte] */
					ResBuf[RspBuffOffset] = ReadIdNo;					/* "ID" This process is brought to this location in order to use "Storing */
					ResBuf[RspBuffOffset + 1] = CurrentDataLength;		/* "ReadIDin the response buffer */
					ResBuf[RspBuffOffset + 2] = 0x00;					/* "ReadID = 0x00 */
					RspBuffOffset = RspBuffOffset + 3;					/* Response buffer(Data section)A offset update */
					RspBuffSize = RspBuffSize + 3;						/* Update receive data size */
				}
				else	/* ReadingIDData exists */
				{
					CurrentDataLength = ReadDataSize * ReadDataNum;			/* Receive data length[byte] */
					ResBuf[RspBuffOffset] = ReadIdNo;						/* "ID" This process is brought to this location in order to use "Storing */
					ResBuf[RspBuffOffset + 1] = CurrentDataLength;			/* "ReadIDin the response buffer */
					RspBuffOffset = RspBuffOffset + 2;						/* Response buffer offset update */
					RspBuffSize = RspBuffSize + CurrentDataLength + 2;		/* Update receive data size */

					/* Response buffer size check */
					if( (RspBuffSize > MBUS_MAXSIZE) || (ReadDataNum < MBUS_MIN_REGNUM))
					{	/* Invalid quantity error */
						mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, 0, TrnbLength );
						return;
					}

					if( ReadIdNo == 0x02 )	/* ID2(Specification)For reading */
					{
						mbusFuncSetSpecialSpecIdData( Hmsg, RspBuffOffset);		/* Specification acquisition */
						RspBuffOffset = RspBuffOffset + CurrentDataLength;		/* Response buffer(Data section)Offset update */
					}
					else					/* ID2(Specification)For cases other than read */
					{
						ret = mbusFuncSetIdData( Hmsg, ReadIdRegAdd, ReadDataSize, ReadDataNum, RspBuffOffset, TrnbLength);	/* Product information acquisition(ID2) */
						if(ret !=0 )
						{	/* Product information acquisition failure */
							/* Error(Error type isret) */
							mbusFuncErrorManager42H( Hmsg, ret, ReadIdRegAdd, TrnbLength );
							return;
						}
						else
						{	/* Product information acquisition success */
							RspBuffOffset = RspBuffOffset + CurrentDataLength;	/* Response buffer(Data section)Offset update */

						}
					}
				}

				BlockDataSize += CurrentDataLength + 2;	/* Reading block size[byte] : ID-No. + Data size + Data count */
			}
			/* Response block size = Block size + Slot number + Axis number + Each read data(ID-No. + Read data size + Reading data) */
			ResBuf[ReadBlockStartOffset] = 3 + BlockDataSize;			/* Block size + Stores "block size" in response buffer */
			NextCmdBuffOffset = NextCmdBuffOffset + ReadBlockSize;		/* Next block number storage buffer offset updated(Return to the above process depending on what to do with the block size) */

			BlockDataSize = 0;											/* Reading block size[byte] 0Clear */
		}
	}

	MlibCopyByteMemory( ResBuf, CmdBuf, MBUS_IDRD_HEAD_BASE_SIZE+1);	/* Response data creation */
	*TrnbLength = RspBuffSize;											/* Determine transmit buffer size */

}

/****************************************************************************************************/
/*																									*/
/*		ProductIDStorage function																				*/
/*			ID0:Product model																			*/
/*			ID1:Model typeID																				*/
/*			ID3:SerialNo.																			*/
/*			ID4:Soft Ver.																			*/
/*			ID5:Register Ver.																		*/
/*			ID6:BTONumber																				*/
/*																									*/
/****************************************************************************************************/
static LONG mbusFuncSetIdData( HMSGIF Hmsg, LONG ReadIdRegAdd, UCHAR ReadDataSize, UCHAR ReadDataNum, LONG RspBuffOffset, LONG *TrnbLength )
{
	UCHAR	DataType;
	UCHAR	dIndex;
	USHORT	DataNum;
	UINT	AccessSize,RegSize;
	REG_ACC_T AccessPrm;
	LONG	i;
	LONG	ret;
	LONG	Value;
	LONG	RspLength;
	LONG	BlockNum;
	ULONG	BeginAddress;
	ULONG	CurrentAddress;
	REG_MANAGER_HANDLE	*hRegMngr;

	/* Receive, send data start address */
	UCHAR  *CmdBuf;	
	UCHAR  *ResBuf;

	CmdBuf = Hmsg->CmdBuf;
	ResBuf = Hmsg->ResBuf;

		ret = 0;

	for ( i = 0; i < ReadDataNum; i++ )
	{
			CurrentAddress = ReadIdRegAdd + (2*i);

		/* virtual memory→Real address conversion */
		AccessPrm.MemAddr = CurrentAddress;
		hRegMngr = mbusFuncConvAddress(Hmsg->AxisMap,&AccessPrm.MemAddr);		
		if( hRegMngr == NULL )
		{	/* Memory address error */
			mbusFuncErrorManager42H( Hmsg, SELAXIS_ERROR, CurrentAddress, TrnbLength );
			ret = SELAXIS_ERROR;
			return ret;
		}

		/* Get parameter attribute */
		if( NULL == RegMngrGetAttrByMemAddr(hRegMngr, &AccessPrm) )
		{	/* Memory address error	*/
			mbusFuncErrorManager42H( Hmsg, MEMADRES_ERROR, CurrentAddress, TrnbLength );
			ret = MEMADRES_ERROR;
			return ret;
		}
		/* Read parameter size */
		RegSize = AccessPrm.cPrmDef->DataLength;

		/*------------------------------------------------------------------------------------------*/
		/*	Block reading																		*/
		/*------------------------------------------------------------------------------------------*/
		BlockNum = RegSize/ReadDataSize;		/* Block read count */

		if( (BlockNum > 0) && ( ReadDataNum > (i + BlockNum - 1)) )
		{
			i = i + (BlockNum-1);				/* Block read counti	*/
		}
		else
		{	
			/* Quantity defect */
			mbusFuncErrorManager42H( Hmsg, DATANUMB_ERROR, CurrentAddress, TrnbLength );
			ret = DATANUMB_ERROR;
			return ret;
		}

		/* Data reading */
		AccessPrm.IfHandle = Hmsg;
		ret = (LONG)PrmReadValue( &AccessPrm, &Value, RegSize);
		if( PRM_RSLT_SUCCESS != ret )
		{
			mbusFuncErrorManager42H( Hmsg, ret, CurrentAddress, TrnbLength );
			return ret;
		}

		/* Send data set */
		switch (RegSize)
		{
			case 1:		/* bytetype */
				ResBuf[RspBuffOffset + (i*RegSize)] = (UCHAR)(Value);
				break;
			case 2:		/* shorttype */
				mbusFuncSetArrayShortData( &ResBuf[RspBuffOffset + (i*RegSize)], (USHORT)Value, ReadDataSize );
				break;
			case 4:		/* longtype */
				mbusFuncSetArrayLongData( &ResBuf[RspBuffOffset + (i*RegSize)], (ULONG)Value, ReadDataSize );
				break;
		}
		
	}

	
	return ret;
}

/****************************************************************************************************/
/*																									*/
/*		ProductID2:Specification storage function																		*/
/*																									*/
/****************************************************************************************************/
static void mbusFuncSetSpecialSpecIdData( HMSGIF Hmsg, LONG RspBuffOffset)
{

	CHAR ServoSpecialSpec[4];

	/* Receive, send data start address */
	UCHAR  *ResBuf;
	ResBuf = Hmsg->ResBuf;	

	/* Special specification number(SERVO_SPECIAL_SPEC_VER)Acquisition */
	ServoSpecialSpec[0] = (CHAR)(0x000f & (SERVO_SPECIAL_SPEC_VER >> 12) );
	ServoSpecialSpec[1] = (CHAR)(0x000f & (SERVO_SPECIAL_SPEC_VER >> 8) );
	ServoSpecialSpec[2] = (CHAR)(0x000f & (SERVO_SPECIAL_SPEC_VER >> 4) );
	ServoSpecialSpec[3] = (CHAR)(0x000f & (SERVO_SPECIAL_SPEC_VER) );

	/* Storing special specification type in response buffer */
	if( ServoSpecialSpec[3] == 0x00 )	/* For standard specifications */
	{
		/* 0x00 = Standard specifications:'0' */
		ResBuf[RspBuffOffset] = NumAsciiTbl[ServoSpecialSpec[3]];

	}
	else	/* For special specifications */
	{
		/* 0x01 = FTSpecifications:'F' */
		/* 0x02 = EXSpecifications:'E' */
		/* 0x03 = YSpecification :'Y' */
		ResBuf[RspBuffOffset] = StrAsciiTbl[(ServoSpecialSpec[3] -  1)];

	}

	/* Special specification number(ASCIICode)in response buffer */
	ResBuf[RspBuffOffset + 1] = NumAsciiTbl[ServoSpecialSpec[2]];
	ResBuf[RspBuffOffset + 2] = NumAsciiTbl[ServoSpecialSpec[1]];
	ResBuf[RspBuffOffset + 3] = NumAsciiTbl[ServoSpecialSpec[0]];

	return;
}
/* <S205> End */
/***************************************** end of file **********************************************/

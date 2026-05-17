/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MsgIfDef.h : Message Interface Define Header File											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1. Message Interface Data Definition														*/
/*																									*/
/*		2. Message Interface Macro Definition														*/
/*																									*/
/*		3.																							*/
/*																									*/
/*		4.																							*/
/*																									*/
/*		5.																							*/
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
/*		Rev.1.00 : 2008.06.11  T.Taniguchi															*/
/*		Rev.1.01 : 2013.04.15  K.Ozaki																*/
/*		Rev.1.02 : 2013.08.10  K.Ozaki		##Panel Operator##										*/
/*																									*/
/*																									*/
/****************************************************************************************************/

#ifndef		MSFGIF_H_
#define		MSFGIF_H_

/****************************************************************************************************/
/*																									*/
/*		Define Function Code, Error Code, etc														*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Main Function Code Definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define	MFC_NONE			0x00				/* Not used											*/
#define	MFC_LOOPBACK		0x08				/* Loopback test								*/
#define MFC_SVXMSG16		0x40				/* Servo expansion message								*/
#define MFC_SVXMSG32		0x41				/* Servo expansion message								*/
#define	MFC_MECHATROLINK	0x42				/* MECHATROLINK Message								*/
#define	MFC_SYSDWNLD		0x50				/* System download								*/
#define	MFC_RELAY 			0x51				/* Relay command										*/

/*--------------------------------------------------------------------------------------------------*/
/*		Sub Function Code Definition for 40H/41H													*/
/*--------------------------------------------------------------------------------------------------*/
#define	SFC_RD_ONEREG		0x40				/* Single register read								*/
#define	SFC_WR_ONEREG		0x06				/* Single register write								*/
#define	SFC_RD_SEQREG		0x03				/* Continuous register read								*/
#define	SFC_WR_SEQREG		0x10				/* Continuous register write								*/
#define	SFC_RD_RANREG		0x0D				/* Non-continuous register read								*/
#define	SFC_WR_RANREG		0x0E				/* Discontinuous register write								*/
#define	SFC_RD_MAXNUM		0x41				/* Read maximum number of registers							*/
/*--------------------------------------------------------------------------------------------------*/
#define	SFC_RD_SEQMEM		0x50				/* Continuous memory read									*/
#define	SFC_WR_SEQMEM		0x51				/* Continuous memory write									*/

/*--------------------------------------------------------------------------------------------------*/
/*		Sub Function Code Definition for 42H														*/
/*--------------------------------------------------------------------------------------------------*/
#define	SFC_RD_REQREG_42	0x01				/* Reading from memory										*/
#define	SFC_WR_SEQREG_42	0x02				/* Memory writing										*/
#define	SFC_RD_RANREG_42	0x03				/* Reading from memory(Discontinuous)								*/
#define	SFC_WR_RANREG_42	0x04				/* Memory writing(Discontinuous)								*/
#define	SFC_RD_MAXNUM_42	0x11				/* Read maximum message size							*/
#define	SFC_RD_PRDCTID_42	0x7F				/* Read model information									*/
/*--------------------------------------------------------------------------------------------------*/
#define	MBUS42_HEADERSIZE	0x08				/* 42HCommand header section size								*/
#define	MBUS42_ADRSIZE		0x04				/* 42HCommand address section size								*/
#define MBUS42_DATASIZEL	0x04				/* 42HCommand data section size (during non-continuous writing)				*/
#define	MBUS42_CRC_SIZE		0x02				/* 42HCommandCRCPart size								*/
#define	MBUS42_DTYPE_B		0x01				/* Data type:Bytetype(8bit)								*/
#define	MBUS42_DTYPE_S		0x02				/* Data type:Shorttype(16bit)							*/
#define	MBUS42_DTYPE_L		0x03				/* Data type:Longtype(32bit)								*/
/* <S150> >>>>> */
/*--------------------------------------------------------------------------------------------------*/
/*		Sub Function Code Definition for 51H														*/
/*--------------------------------------------------------------------------------------------------*/
#define MBUS51_FIX_SIZE			0x0E			/* Relay command fixed size								*/
#define MBUS51_PASS_DATA_SIZE	0x08			/* Relay command variable size(varies in multiples of this size)				*/
#define MBUS51_MAX_ROUTENUM		0x0A			/* Maximum number of relay stages									*/
/* <<<<< <S150> */


/*--------------------------------------------------------------------------------------------------*/
/*		Download			SubFuncCode		   S:Support, -:Not Support								*/
/*--------------------------------------------------------------------------------------------------*/
#define	SFC_DLID_RD			0x0000				/* S : Read ID information								*/
#define	SFC_SWVER_RD		0x0001				/* S : Program write condition acquisition						*/
#define	SFC_DLWAY_RD		0x0002				/* S : Program write condition acquisition						*/
#define	SFC_DL_START		0x0003				/* S : Download start									*/
#define	SFC_DL_PRGDEL		0x0004				/* S : Block erase									*/
#define	SFC_DL_PRGWR		0x0005				/* S : Program write								*/
#define	SFC_DL_DELSND		0x0006				/* - : Block erase information(Erase information transfer only.Do not erase)	*/
#define	SFC_DL_PRGSND		0x0007				/* - : Program transmission(Transmit only.Do not write)			*/
#define	SFC_DL_PRGDELA		0x0008				/* - : Block batch erase command							*/
#define	SFC_DL_PRGWRA		0x0009				/* - : Program batch write command						*/
#define	SFC_DL_END			0x000A				/* S : Download end command								*/
#define	SFC_DL_PRGRD		0x000B				/* S : Program read command							*/
#define	SFC_DL_REBOOT		0x000C				/* S : Program restart command							*/
#define	SFC_DL_SIFCHG		0x000D				/* S : Serial IF speed change								*/
#define	SFC_DL_HIS_RD		0x000E				/* S : Download history reading							*/
#define	SFC_DL_HIS_WR		0x000F				/* S : Download history reading included							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Error Code Definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define ECD_ERR_MFC			0x00				/* Error : Main Function Code						*/
#define ECD_ERR_SFC			0x00				/* Error : Sub  Function Code						*/
/*--------------------------------------------------------------------------------------------------*/
/*		SigmaMemobus Error Code Definition															*/
/*--------------------------------------------------------------------------------------------------*/
#define	SMBUS_EC_FNCDERR	(0x01 | 0x200)		/* Function code error							*/
#define	SMBUS_EC_REGNOERR	(0x02 | 0x200)		/* Bad register number									*/
#define	SMBUS_EC_DANUMERR	(0x03 | 0x200)		/* Invalid quantity error										*/
/*--------------------------------------------------------------------------------------------------*/
#define	SMBUS_SEC_REGNOERR	(0x30 | 0x200)		/* Bad register number(Level high)						*/
#define	SMBUS_SEC_ACCSERR	(0x31 | 0x200)		/* Access restriction error								*/
#define	SMBUS_SEC_DALMTERR	(0x32 | 0x200)		/* Setting value out of range error								*/
#define	SMBUS_SEC_DAMTHERR	(0x33 | 0x200)		/* Data consistency error 								*/
#define	SMBUS_SEC_CONDERR	(0x34 | 0x200)		/* Condition error										*/
#define	SMBUS_SEC_INTLOCK	(0x35 | 0x200)		/* Processing conflict error									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Response/no response definition at transmission start																*/
/*--------------------------------------------------------------------------------------------------*/
#define	RESMT_COMPLETE		0x01				/* MsgType : Completion response								*/
#define	RESMT_NORESPONSE	0x00				/* MsgType : No response(Message error)					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Sigma message error code definition															*/
/*--------------------------------------------------------------------------------------------------*/
#define	SMSGEC_FNCDERR		0x10				/* Function code error							*/
#define	SMSGEC_SEQXERR		0x10				/* Communication sequence error(Continuing transmission)		@@@Specifications undefined@@@	*/
#define	SMSGEC_CMDXERR		0x14				/* Download start command error		@@@Specifications undefined@@@	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Auxiliary function response hold																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	REGRES_NOHOLD		0x00				/* Auxiliary function response hold release			@@@Review required@@@	*/
#define	REGRES_HOLD			0x01				/* Auxiliary function response hold				@@@Review required@@@	*/

/****************************************************************************************************/
/*																									*/
/*		Register IF Data Definition																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Register IF function normal return value definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define	REGIF_COMPLETE		0x00		/* Processing completed							*/
#define	REGIF_CONTINUE		0x01		/* Continue processing							*/
/*--------------------------------------------------------------------------------------------------*/
/*		Register IF function abnormal return value definition(Also used as Sigma message error code)	*/
/*--------------------------------------------------------------------------------------------------*/
#define	REGIF_CMDERR		0x10		/* Register IF command error			*/
#define	REGIF_REGNOERR		0x11		/* Register number error 				*/
#define	REGIF_DALMTERR		0x12		/* Data abnormality(Upper/lower limit error)   		*/
#define	REGIF_DACALERR		0x13		/* Data abnormality(Operation error) 			*/
#define	REGIF_DAMTHERR		0x14		/* Data consistency error 				*/
#define	REGIF_DAMTHERR2		(0x14 | 0x100)		/* Data consistency error 		*/
#define	REGIF_DANUMERR		0x15		/* Abnormal number of data   				*/
#define	REGIF_FOPEERR1		0x30		/* Operation mode occupied by other CH 	*/
#define	REGIF_FOPEERR2		0x31		/* Operation mode occupied by other CH	*/
#define	REGIF_FOPEERR3		0x3F		/* Processing conflict with other CH				*/
#define	REGIF_FOPEERR4		0x3F		/* Auxiliary function execution error					*/


/****************************************************************************************************/
/*																									*/
/*		Message Interface Struct Definition															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		HmsgFor identificationFlagSetting	|	PnlOpe	LcdOpe	SifMsg	UsbMsg	NetMsg  LnkMsg						*/
/*	  ----------------------+-------------------------------------------------------------------	*/
/*		XxxMsg.f.OpeMsg		|	TRUE	TRUE														*/
/*		XxxMsg.f.PopMsg		|	TRUE																*/
/*		XxxMsg.f.LopMsg		|			TRUE														*/
/*	  ----------------------+-------------------------------------------------------------------	*/
/*		XxxMsg.f.ComMsg		|					TRUE	TRUE	TRUE	TRUE						*/
/*		XxxMsg.f.SifMsg		|					TRUE												*/
/*		XxxMsg.f.UsbMsg		|							TRUE										*/
/*		XxxMsg.f.NetMsg		|									TRUE								*/
/*		XxxMsg.f.LnkMsg		|											TRUE						*/
/*	  ----------------------+-------------------------------------------------------------------	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
typedef	struct	_MSGIF {						/* Message IF Definition 							*/
	/*----------------------------------------------------------------------------------------------*/
		struct	{								/* TODO:HmsgIdentification flag							*/
		UINT	OpeMsgIF		:1;				/* Operator identification flag								*/
		UINT	PopMsgIF		:1;				/* Panel operator identification flag						*/
		UINT	LopMsgIF		:1;				/* LCD operator identification flag						*/
		UINT	XxxOpeIF		:1;				/* Preliminary												*/
	/*----------------------------------------------------------------------------------------------*/
		UINT	ComMsgIF		:1;				/* Communication message identification flag							*/
		UINT	SifMsgIF		:1;				/* Serial communication message identification flag					*/
		UINT	UsbMsgIF		:1;				/* USB communication message identification flag					*/
		UINT	NetMsgIF		:1;				/* Network message identification flag					*/
		UINT	LnkMsgIF		:1;				/* Link communication message identification flag					*/
		UINT	MlnkMsgIF		:1;				/* mechatro link communication message identification flag			*//*<MEMO_001T>*/
	/*----------------------------------------------------------------------------------------------*/
//		UINT	SpareFlags		:23;			/* Spare											*/
		UINT	SpareFlags		:22;			/* Spare											*//*<S150>*/
		} f;									/*													*/
/*--------------------------------------------------------------------------------------------------*/
	CSIF   *Sif;								/* Pointer to the Serial interface					*/
	UCHAR  *CmdBuf;								/* Pointer to the Command Message Buffer			*/
	UCHAR  *ResBuf;								/* Pointer to the Response Message Buffer			*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	RcvBufSize;							/* Receive Message Buffer Size						*/
	USHORT	SndBufSize;							/* Send Message Buffer Size							*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	CmdMsgLen;							/* Command Message Length							*/
	USHORT	AccessLvl;							/* Access Level USER1/USER2/SYSTEM)					*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	ComMode;							/* Communication Mode (LCDOPE,MSG,etc)				*/
	USHORT	ComPortType;						/* Channel Information ( not used )					*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	ErrRno;								/* Error Register Number							*/
	USHORT	LstErrRno;							/* Last Error Register Number						*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	FunSts;								/* Function Execution Status						*/
	UCHAR	LastAxisIdx;						/* Multi-Axis : Last Axis Index						*/
	UCHAR	ReserveChar0;						/* Reserved											*/
/*--------------------------------------------------------------------------------------------------*/
	ULONG	ComChkTime;							/* Timeout Timer									*/
/*--------------------------------------------------------------------------------------------------*/
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
	AXIS_HANDLE *pAxRsc;						/* TODO: Delete it later 								*/
#else	/*_MERCURY_PLATFORM_*/
	void		 *pAxRsc;						/* TODO: Delete it later 								*/
#endif	/*_MERCURY_PLATFORM_*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	OpeDspMode;							/* Operator user constant display selection					*/	/* <S113> */
	USHORT	LstOpeDspMode;						/* Operator user constant display selection					*/	/* <S197> */
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
	MEMOBUS_ADDR_MAP	*AxisMap;				/* <S150> */
#else	/*_MERCURY_PLATFORM_*/
	void	*AxisMap;				/* <S150> */
#endif	/*_MERCURY_PLATFORM_*/
//	LONG  (*StxFunc)( UCHAR *, USHORT );		/* TODO: Response message transmission start API function	*/
//	void  (*ResFunc)( struct _MSGIF * );		/* TODO: Fn execution result response message processing function*/
/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
//	USHORT	FunExeIdx;							/* TODO:Auxiliary function execution table specification index	*/
//	USHORT	FnStsToOpi;							/* TODO:Auxiliary function execution result(OPTION-SERVO I/FFor)	*/
/*----------------------------------------------------------------------------------------------*/
//	USHORT	FunResHold;							/* TODO:Auxiliary function response hold				*/
/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
//	USHORT	ComTimeShort;						/* TODO:Optional communication timeout measurement	*/
//	USHORT	RcvWait;							/* TODO:SCMessage communication reception waiting flag	*/
/*----------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------*/
//	void *PrmDef;								/* TODO: Pointor to the Parameter Definition 	*/	
//	void *MonDef;								/* TODO: Pointer to the Monitor Definition 		*/	
//	void *FunDef;								/* TODO: Pointer to the Fn Definition 			*/
//TODO: void *AxDef;							/* TODO: Pointer to the Axis Definition			*/
} MSGIF;
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*//* <S0E4> */
/****************************************************************************************************/
/*		EXTERN LIST																					*/
/****************************************************************************************************/
/* M-3 Message Interface */
extern	MSGIF	LopMsg;					/* LCD Operator Messsage I/F 						*/
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(Single axis) only	*/
#endif
typedef	MSGIF*	HMSGIF;							/* Handle definition for above				*/

#if 0											
/*--------------------------------------------------------------------------------------------------*/
/* 
 * Jupiter(For reference)								
 */
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	_MSGIF {						/* Message IF structure definition						*/
/*--------------------------------------------------------------------------------------------------*/
		struct	{								/* HmsgIdentification flag(Set at initialization)					*/
		UINT	OpeMsgIF		:1;				/* Operator identification flag								*/
		UINT	PopMsgIF		:1;				/* Panel operator identification flag						*/
		UINT	LopMsgIF		:1;				/* LCD operator identification flag						*/
		UINT	XxxOpeIF		:1;				/* Preliminary												*/
	/*----------------------------------------------------------------------------------------------*/
		UINT	ComMsgIF		:1;				/* Communication message identification flag							*/
		UINT	SifMsgIF		:1;				/* Serial communication message identification flag					*/
		UINT	UsbMsgIF		:1;				/* USB communication message identification flag					*/
		UINT	NetMsgIF		:1;				/* Network message identification flag					*/
		UINT	LnkMsgIF		:1;				/* Link communication message identification flag					*/
	/*----------------------------------------------------------------------------------------------*/
		UINT	SpareFlags		:23;			/* Spare											*/
		} f;									/*													*/
/*--------------------------------------------------------------------------------------------------*/
		UCHAR  *CmdBuf;							/* Command message buffer pointer					*/
		UCHAR  *ResBuf;							/* Response message buffer pointer					*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	CmdBufSize;						/* Command message buffer size(ByteNumber)				*/
		USHORT	ResBufSize;						/* Response message buffer size(ByteNumber)				*/
	/*----------------------------------------------------------------------------------------------*/
		UCHAR	ComMode;						/* Communication mode(Connecting/Not connected/etc)					*/
		UCHAR	AccessLvl;						/* Access level(USER1/USER2/SYSTEM)				*/
		USHORT	CmdMsgLen;						/* Command message length									*/
		ULONG	ComChkTime;						/* For communication timeout measurement							*/
	/*----------------------------------------------------------------------------------------------*/
		UCHAR	FunIdx;							/* Auxiliary function table index						*/
		USHORT	FunSts;							/* Auxiliary function execution result									*/
		USHORT	ErrRno;							/* Abnormal register number									*/
		USHORT	LstErrRno;						/* Abnormal register number of previous command					*/
	/*----------------------------------------------------------------------------------------------*/
		UCHAR	FunResHold;						/* Auxiliary function response hold								*/
		UCHAR	LastAxisIdx;					/* Multi-axis drive: Previous axis index					*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	DldAllPrmRdCount;				/* Read  Count  for Download All Parameter			*/
		USHORT	DldAllPrmRdChkSum;				/* Read  ChkSum for Download All Parameter			*/
		USHORT	DldAllPrmWrCount;				/* Write Count  for Download All Parameter			*/
		USHORT	DldAllPrmWrChkSum;				/* Write ChkSum for Download All Parameter			*/
	/*----------------------------------------------------------------------------------------------*/
		ULONG	LopAccessLvl;					/* LCD Operator access level for Converter <0BJ14>	*/
} MSGIF, *HMSGIF;								/* Structure type name & handle definition							*/
#endif

/*--------------------------------------------------------------------------------------------------*/
/*
 * 	Message Handle Check Macro Definition
 *
 *
 */
/*--------------------------------------------------------------------------------------------------*/
#define	KPI_CHK_OpeHmsg( Hmsg )		( (Hmsg)->f.OpeMsgIF )
#define	KPI_CHK_LopHmsg( Hmsg )		( (Hmsg)->f.LopMsgIF )
#define	KPI_CHK_ComHmsg( Hmsg )		( (Hmsg)->f.ComMsgIF )
#define	KPI_CHK_SifHmsg( Hmsg )		( (Hmsg)->f.SifMsgIF )
#define	KPI_CHK_UsbHmsg( Hmsg )		( (Hmsg)->f.UsbMsgIF )
#define	KPI_CHK_NetHmsg( Hmsg )		( (Hmsg)->f.NetMsgIF )
#define	KPI_CHK_LnkHmsg( Hmsg )		( (Hmsg)->f.LnkMsgIF )
#define	KPI_CHK_PopHmsg( Hmsg )		( (Hmsg)->f.PopMsgIF )

/*--------------------------------------------------------------------------------------------------*/
/*
 *	Massage Handle Initialization
 *
 *  KRI_INIT_MSGIF() : Initialize Message Handle
 *  KRI_BIND_MSGIF() : Bind a Message Handle to Other Message Handle
 *
 */
/*--------------------------------------------------------------------------------------------------*/
#define KRI_INIT_MSGIF( hmsg, csif )	{ 		/* bind CSIF to Message IF */ 	\
		MlibResetLongMemory( hmsg, sizeof(MSGIF)/4 );	/* reset memory */		\
		(hmsg)->Sif = (csif);					/* set sif */			\
		(hmsg)->CmdBuf = (csif)->RxBuf;			/* set cmd buf */ 		\
		(hmsg)->ResBuf = (csif)->TxBuf;			/* set res buf */		\
		(hmsg)->RcvBufSize = (csif)->RcvBufSize;/* set rcv buf size */ \
		(hmsg)->SndBufSize = (csif)->RcvBufSize;/* set send buf size */\
	}
/*--------------------------------------------------------------------------------------------------*/
#define KRI_BIND_MSGIF( hmsg, src )	{ 			/* bind Message IF to other Message IF */ 	\
		(hmsg)->Sif = (src)->Sif;				/* set sif */			\
		(hmsg)->CmdBuf = (src)->CmdBuf;			/* set cmd buf */ 		\
		(hmsg)->ResBuf = (src)->ResBuf;			/* set res buf */		\
		(hmsg)->RcvBufSize = (src)->RcvBufSize; /* set rcv buf size */ \
		(hmsg)->SndBufSize = (src)->RcvBufSize; /* set send buf size */\
	}

/*--------------------------------------------------------------------------------------------------*/
/*
 * Message Handle Flag Initialization
 * 
 * KRI_SET_FLG_LCDOPE() : for Lcd Operator
 * KRI_SET_FLG_SRLMSG() : for UART message 
 * KRI_SET_FLG_USBMSG() : for USB message
 * KRI_SET_FLG_PNLOPE() : for Panel Operator
 *
 */
/*--------------------------------------------------------------------------------------------------*/
#define KRI_SET_FLG_LCDOPE(hMsg)	{ \
					*(UINT *)&((hMsg)->f) = (UINT)0; \
					(hMsg)->f.OpeMsgIF = 1; \
					(hMsg)->f.LopMsgIF = 1; \
				}
#define KRI_SET_FLG_SRLMSG(hMsg)	{ \
					*(UINT *)&((hMsg)->f) = (UINT)0; \
					(hMsg)->f.ComMsgIF = 1; \
					(hMsg)->f.SifMsgIF = 1; \
				}
#define KRI_SET_FLG_USBMSG(hMsg)	{ \
					*(UINT *)&((hMsg)->f) = (UINT)0; \
					(hMsg)->f.ComMsgIF = 1; \
					(hMsg)->f.UsbMsgIF = 1; \
				}
#define KRI_SET_FLG_PNLOPE(hMsg)	{ \
					*(UINT *)&((hMsg)->f) = (UINT)0; \
					(hMsg)->f.OpeMsgIF = 1; \
					(hMsg)->f.PopMsgIF = 1; \
				}
/* <S150> >>>>> */
#define KRI_SET_FLG_MLNKMSG(hMsg)	{ \
					*(UINT *)&((hMsg)->f) = (UINT)0; \
					(hMsg)->f.ComMsgIF = 1; \
					(hMsg)->f.MlnkMsgIF = 1; \
				}
/* <<<<< <S150> */


/*--------------------------------------------------------------------------------------------------*/
/*
 *  Message Check Macro
 *
 *  KPI_CHK_OPEMSG() : LCD Operator
 *  KPI_CHK_SYSDLMSG() : Download Message
 *  KPI_CHK_M3MSG() : Mechatrolink Message
 *
 */
/*--------------------------------------------------------------------------------------------------*/
#define	KPI_CHK_OPEMSG( fc ) ((0x60 <= fc) && (fc <= 0x64))		/* Operator message check		*/
#define	KPI_CHK_SYSDLMSG( fc ) (0x50 == fc)						/* Download message check	*/
#define	KPI_CHK_M3MSG( fc ) (0x42 == fc)						/* M3Message check				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Serial Modes (for SerComMode)																*/
/*--------------------------------------------------------------------------------------------------*/
#define SCM_NOCOM		0							/* No connected device									*/
#define SCM_LCDCOM		1							/* LCDOperator								*/
#define SCM_MSGCOM		2							/* MessageEquipment(PC,etc)							*/
#define SCM_TSTCOM		3							/* TesterEquipment(Debug)							*/

/*--------------------------------------------------------------------------------------------------*/
/*		Access level(Caution:0 cannot be used)															*/
/*--------------------------------------------------------------------------------------------------*/
#define ACCLVL_USER1			1				/* User 1										*/
#define ACCLVL_USER2			2				/* User 2										*/
#define ACCLVL_USER3			3				/* User 3										*//* <S0FE> */
//#define ACCLVL_SYSTEM			3				/* System											*//* <S0FE> */
#define ACCLVL_SYSTEM			4				/* System											*//* <S0FE> *//*--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------*/
/*
 * Function Message Definition 
 * 
 * Lcd operator and panel operator use this for executing  function.
 *
 */
/*--------------------------------------------------------------------------------------------------*/
typedef struct _FUNMSG {
	MSGIF	*Super;							/* @@ Super Class (Message Interface) */
/*--------------------------------------------------------------------------------------------------*/
	UINT 	*pKeyMk;						/* Pointer to the Key Make (OFF->ON) Flag */
	UINT 	*pKeyBrk;						/* Pointer to the Key Break(ON->OFF) Flag */
	UINT 	*pKeyLvl;						/* Pointer to the Key Level Flag */
	UINT	*pBlnkKick;						/* Pointer to the Blink Flag */
	UCHAR	*pTxBuf;						/* Pointer to the Display Inforamtion Buffer */
/*--------------------------------------------------------------------------------------------------*/
	void	*pVar;							/* Pointer to the Function common variable area */
} FUNMSG;


/*--------------------------------------------------------------------------------------------------*/
/*
 * Panel Operator Message Definition
 *
 * Command :
 * 		includes panel button information. (RinK.Pnl)
 * 
 * Response :
 * 		includes panel led display code information. (RoutK.PnlLed)
 *
 */
/*--------------------------------------------------------------------------------------------------*/
#define PNL_N_LED  (5)
#define PNLCMD_PNLBTN_IDX (0)
typedef struct _PnlCmdMsg {
//	UCHAR Magic[2];						/* Magic Number { 'P', 'O' } */
	UCHAR PnlBtn;						/* Panel Operator Input Button Switch Code */
} PNLCMDMSG;

#define PNLRSP_LEDCODE_IDX (0)
typedef struct _PnlRspMsg {
//	UCHAR Magic[2];						/* Magic Number { 'P', 'O' } */
	/*----------------------------------------------------------------------------------------------*/
	/*
	 * LEDDSP;
	 *	UCHAR	LedCode[5]			:	Code to display	
	 *	UCHAR	LedBlnkSw[5]		:	1part is blinking
	 */
	/*----------------------------------------------------------------------------------------------*/
	UCHAR LedCode[PNL_N_LED];			/* Panel Operator Output Led Code */
	UCHAR LedBlnkSw[PNL_N_LED];			/* Panel Operator Blink Switch */
} PNLRSPMSG;



#endif /* MSFGIF_H_ */
/***************************************** end of file **********************************************/

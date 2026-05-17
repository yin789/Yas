#ifndef	_TRACE_MANAGER_H_
#define	_TRACE_MANAGER_H_

/****************************************************************************************************/
/*																									*/
/*																									*/
/*		DataTraceManager.h : Data trace definition header file										*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.10.26	T.Kira	For INGRAM												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "PrmAccessIf.h"
#include "MemobusIF.h"


/*--------------------------------------------------------------------------------------------------*/
/*		Variable Type Definition																	*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*		Trace execution command definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	TRCCMD_NOCMD			0x00				/* No command										*/
#define	TRCCMD_DATATRACE		0x01				/* Trace execution command								*/
#define	TRCCMD_RINGTRACE		0x02				/* Continuous trace execution command							*/
#define	TRCCMD_CONTTRACE		0x03				/* Trace execution command(Mode maintenance)					*/
/*--------------------------------------------------------------------------------------------------*/
#define TRC_CH_MAXNUM			16					/* Number of trace channels							*/
/*--------------------------------------------------------------------------------------------------*/
#define	OPEMODE_NOCMD			0x0000				/* Normal mode									*/
#define	OPEMODE_DATATRACE		0x2000				/* Data trace mode							*/
#define	OPEMODE_RINGTRACE		0x200A				/* Continuous trace mode							*/
/*--------------------------------------------------------------------------------------------------*/
#if defined(_TEST_BOARD_)
#define TRC_BUF_SIZE			0x220				/* Data trace buffer size(USHORT)			*/
#else
#define TRC_BUF_SIZE			0x4000				/* Data trace buffer size(USHORT)			*//* <S04C> */
#endif
/*--------------------------------------------------------------------------------------------------*/
#define DATATRACE_SHORT			0x00				/* Short data trace						*/
#define DATATRACE_LONG			0x01				/* Long data trace							*/
/*--------------------------------------------------------------------------------------------------*/
#define IODATA_MERGE			0x00				/* IOCombine data with variable data					*/
#define IODATA_DIV				0x01				/* IOStore data in dedicated area						*/
/*--------------------------------------------------------------------------------------------------*/
#define SINGLE_TRG				0x00				/* Single trigger data trace					*/
#define MULTI_TRG				0x01				/* Multitrigger data trace					*/
/*--------------------------------------------------------------------------------------------------*/
#define TRG_SEL2_USE			0x0F				/* Trigger data selection2use				*//* <S012> */
#define NET_TRG_SEL				0x0F				/* Use network trigger				*//* <S012> */

//#define _DATA_TRACE_DBUG_

/****************************************************************************************************/
/*																									*/
/*		Data trace definition structure																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Trace control area:2100H - 220AH															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	TRCREG{
	USHORT	TrcMode;						/* 0x2100 : Trace mode								*/
	USHORT	DataSize;						/* 0x2101 : Trace data length							*/
	//USHORT	AreaSize;					/* 0x2102 : Trace buffer length							*/	/* <S0C9> */
	USHORT	AreaSize;						/* 0x2102 : Use area size							*/	/* <S0C9> */
	USHORT	SampIntv;						/* 0x2103 : Trace sampling interval					*/
	USHORT	SampTime;						/* 0x2104 : Trace sampling unit [Us]				*/
	USHORT	SampLlim;						/* 0x2105 : Trace sampling interval lower limit				*/
	USHORT	SampUlim;						/* 0x2106 : Trace sampling interval upper limit				*/
	USHORT	MaxAreaSize;					/* 0x2107 : Maximum trace buffer length						*/
	USHORT	OpeModeTrc;						/* 0x2108 : Trace-only operation mode setting/Reference				*/
	USHORT	OpeCmdTrc;						/* 0x2109 : Trace-only operation setting						*/
	USHORT	TrcBuffSize;					/* 0x210A : Trace buffer length							*/	/* <S0C9> */
	USHORT	MaxTrcBuffSize;					/* 0x210B : Maximum trace buffer length						*/	/* <S0C9> */
	/*----------------------------------------------------------------------------------------------*/
/* <S0C9> Start */
//	USHORT	TrcSel[TRC_CH_MAXNUM];			/* 0x2110 : Data trace target selection(1~16)				*/
//	USHORT	IOTrcSel[TRC_CH_MAXNUM];		/* 0x2120 : I/OSelect the trace target(1~16)					*/
//	ULONG	*TrcAdr[TRC_CH_MAXNUM];			/* 0x2130 : Trace target address specification(Undisclosed)			*/
	USHORT	DataIOTrcSel[TRC_CH_MAXNUM];	/* 0x2110 : Data,I/OSelect the trace target(1~16)			*/
	USHORT	TrcAxisSel[TRC_CH_MAXNUM];		/* 0x2120 : Data trace target axis selection(1~16)				*/
	USHORT	TrcDataSel[TRC_CH_MAXNUM];		/* 0x2130 : Selection of data to be traced(1~16)			*/
	USHORT	IOTrcAxisSel[TRC_CH_MAXNUM];	/* 0x2140 : I/OTrace1Target axis selection(1~16)				*/
	USHORT	IOTrcDataSel[TRC_CH_MAXNUM];	/* 0x2150 : I/OTrace1Target data selection(1~16)			*/
	ULONG	*TrcAdr[TRC_CH_MAXNUM];			/* 0x2160 : Trace target address specification(Undisclosed)			*/
/* <S0C9> End */
	/*----------------------------------------------------------------------------------------------*/
	USHORT	PtrgDnum;						/* 0x2200 : Number of pre-trigger data							*/
	USHORT	TrgSel;							/* 0x2201 : Trigger condition									*/
	LONG	TrgLevel;						/* 0x2202 : Trigger level								*/
	USHORT	TrgPoint;						/* 0x2204 : Trigger position									*/
	USHORT	TrcState;						/* 0x2205 : Trace completion								*/
	ULONG 	RngTrcPtr;						/* 0x2206 : Continuous trace write position						*/
	/*----------------------------------------------------------------------------------------------*/
	USHORT	MTrgMode;						/* 0x2208 : Multitrigger setting							*/
	USHORT	MTrgSel;						/* 0x2209 : Trigger condition 2								*/
	LONG	MTrgLevel;						/* 0x220A : Trigger level 2								*/
} TRCREG;



/****************************************************************************************************/
/*																									*/
/*		Variable definition for data trace execution																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Data trace/table operation execution variables areRound:DtrcMngr.c and ScanB:DtrcExec.c			*/
/*		(Write)and other modules can only be referenced.								*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Real address definition																				*/
/*--------------------------------------------------------------------------------------------------*/
typedef	union TRCADRDATA{
	USHORT	*ShortAdrData;					/* Real address specification pointer								*/
	ULONG	*LongAdrData;					/* Real address specification pointer								*/
} TRCADRDATA;
/*--------------------------------------------------------------------------------------------------*/
/*		Trace target definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	TRCOBJ{						/* Trace target definition										*/
	USHORT	TrcExeflg;						/* Trace execution control flag(0x01:Execution,Others:Not executed)		*/
	void	*TrcBuf;						/* Trace data storage buffer							*/
	TRCADRDATA	Adr;						/* Real address definition										*/

	struct {								/* Numeric trace variable										*/
		void	*AxisPtr;					/* Numeric traceAXISHANDLE								*/
		USHORT	VxSftL;						/* Numeric trace variable left shift scale						*/
		USHORT	VxSftR;						/* Numeric trace variable right shift scale						*/
		LONG	VxData;						/* Numeric trace data									*/
		LONG	VxGain;						/* Numerical trace calculation gain								*/
		BOOL	AvrgSel;					/* Numerical trace average selection:TRUE:Average,FALSE⇒did not	*/
		LONG	(*GetData)( void *Axis );	/* Trace variable address acquisition function							*/
	} TrcV;

	struct {
		//USHORT	BtvBit;					/* Bit trace variable bit selection(Mask)					*/
		void	*AxisPtr;					/* Bit traceAXISHANDLE								*/
		BOOL	BtvData;					/* Bit trace variable address							*/
		BOOL	(*GetData)( void *Axis );	/* Trace variable address acquisition function							*/
	} TrcB;

} TRCOBJ;

/* <S0C9> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Data trace individual parameters(Target data, target axis)											*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	TRCINDIVPRM{
	USHORT	TrcAxis[TRC_CH_MAXNUM];				/* Data trace target axis selection(1~16)				*/
	USHORT	TrcData[TRC_CH_MAXNUM];				/* Selection of data to be traced(1~16)			*/
	USHORT	IOTrcAxis[TRC_CH_MAXNUM];			/* I/OTrace1Target axis selection(1~16)				*/
	USHORT	IOTrcData[TRC_CH_MAXNUM];			/* I/OTrace1Target data selection(1~16)			*/
} TRCINDIVPRM;
/* <S0C9> End */

/*--------------------------------------------------------------------------------------------------*/
/*		Trace trigger variable information definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	TRCTRG{						/* Trace execution parameter definition							*/
	USHORT	TrgSel;							/* Trace trigger selection									*/
	USHORT	TrgEdge;						/* Trace trigger edge specification								*/
	USHORT	TrgVsft;						/* Trace trigger data shift							*/
	LONG	TrgLevel;						/* Trace trigger level									*/
	ULONG	TrgBitSetMsk;					/* Trace trigger bit mask							*/
	void	*TrgBufx;						/* Trace trigger data buffer							*/
}TRCTRG;

/*--------------------------------------------------------------------------------------------------*/
/*		Trace execution parameter definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	TRCEXEP{					/* Trace execution parameter definition							*/
	USHORT	TrcPrmOk;						/* Trace parameter OK								*/
	USHORT	TrcDataType;					/* Trace data type									*/
	USHORT	TrcIOType;						/* IOTrace data type								*/
	ULONG	TrcDnum;						/* Number of trace data									*/
	ULONG	DataSampIntv;					/* Trace data sampling interval						*/
	LONG	UpperLmt;						/* Trace data upper limit									*/
	LONG	LowerLmt;						/* Trace data lower limit									*/
	ULONG	TrcBitSet;						/* For trace bit set								*/
	ULONG	TrcBitClr;						/* For trace bit clearing								*/
	/*----------------------------------------------------------------------------------------------*/
	TRCTRG	TrcTrg[2];						/* Trace trigger variable information (multi-trigger compatible)			*/
	TRCOBJ	TrcObj[TRC_CH_MAXNUM];			/* Trace variable information										*/
} TRCEXEP;

/*--------------------------------------------------------------------------------------------------*/
/*		Trace execution control variable definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	TRCEXEV{					/* Trace execution control variable definition							*/
	USHORT	TrcCmd;							/* Trace execution command										*/
	USHORT	Opened;	 						/* Trace function already open								*/
	USHORT	OpmRun;							/* Trace operation mode running(OpeModeFor control)				*/
	USHORT	TrcStep;						/* Trace execution step									*/
	USHORT	TrcSamp;						/* Trace sampling									*/
	USHORT	TrcIndx;						/* Trace index									*/
	USHORT  RngTrcCnt;						/* Continuous trace write count								*/
	LONG	TrgVar[2];						/* Trigger check variable(Current value,Previous value)					*/
	LONG	MTrgVar[2];						/* Trigger check variable(Current value,Previous value)(Multitrigger compatible)	*/
	BOOL	NetTrigger;						/* Network trigger information								*//* <S012> */
} TRCEXEV;

/*--------------------------------------------------------------------------------------------------*/
/*		Data trace management variable definition																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	TRCMNGP{					/* Trace management parameter definition							*/
	ULONG	TrcVarTblEntNum;				/* Number of registered numerical trace definition tables						*/
	ULONG	TrcVarTblSrhIniW;				/* Numeric trace definition table search division width initial value				*/
	ULONG	TrcBitTblEntNum;				/* Number of registered bit trace definition tables					*/
	ULONG	TrcBitTblSrhIniW;				/* Bit trace definition table search division width initial value			*/
} TRCMNGP;
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	TRCMNGV{					/* Trace management data variables								*/
	ULONG	TrcOpeRegWrtCnt;				/* For parameter calculation execution check							*/
	ULONG	TdrvOpeRegWrtCnt;				/* For parameter calculation execution check							*/
} TRCMNGV;

/****************************************************************************************************/
/*																									*/
/*		Trace execution variable definition																		*/
/*																									*/
/****************************************************************************************************/
typedef	struct	TRCHNDL{
	TRCREG	TrcReg;								/* Trace control area register						*/
	TRCEXEP	TrcExeP;							/* Trace execution parameters							*/
	TRCEXEV	TrcExeV;							/* Trace execution control variable							*/
	TRCMNGP	TrcMngP;							/* Trace management parameters							*/
	TRCMNGV	TrcMngV;							/* Trace management data variables							*/
	MEMOBUS_ADDR_MAP AxisMap[MAX_AXIS_NUMBER];	/* Axis address definition									*/
	TRCINDIVPRM	TrcIncivPrm;					/* Trace individual parameters							 */	/* <S0C9> */
}TRCHNDL;

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void		DtrcInitDtrcManager( TRCHNDL *TrcHndl, USHORT *TraceBuffer,	MEMOBUS_ADDR_MAP *AxisMap );
PRM_RSLT	DtrcCalcParameter( TRCHNDL *TrcHndl, USHORT *TraceBuffer );
PRM_RSLT	DtrcWriteOpeModeTrcRegister( TRCHNDL *TrcHndl, USHORT WrData );
PRM_RSLT	DtrcWriteOpeCmdTrcRegister( TRCHNDL *TrcHndl, USHORT *TraceBuffer, USHORT WrData );
void		DtrcExecute( TRCHNDL *TrcHndl );
void		DtrcProcedure( TRCHNDL *TrcHndl, LONG Idx );
void		DtrcSetTrcVar( TRCHNDL *TrcHndl );
void		DtrcResetAverageVar( TRCHNDL *TrcHndl );
void		DtrcInitExecute( TRCHNDL *TrcHndl );
void		DtrcInitExecuteRingTrc( TRCHNDL *TrcHndl );
void		DtrcSetNetworkTrigger( TRCHNDL *TrcHndl, BOOL Trigger ); /* <S012> */

/****************************************************************************************************/
/*		API's																						*/
/****************************************************************************************************/
ULONG	DtrcGetSampIntv( TRCHNDL *TrcHndl );
ULONG	DtrcGetTrcDataNum( TRCHNDL *TrcHndl );
void	DtrcSetTrgPointToZero( TRCHNDL *TrcHndl );
LONG	DtrcSearchTrcVarTbl( TRCHNDL *TrcHndl, LONG SelNo );

/***************************************** end of file **********************************************/
#endif	// _TRACE_MANAGER_H_

#ifndef	_MEMOBUS_IF_H /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MemobusIF.h : MemobusProcessing function definition															*/
/*																									*/
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
/*																									*/
/*																									*/
/*	Note	:	First edition	2010.09.13	T.Kira	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
#include "PrmAccessIf.h"
#endif	/*_MERCURY_PLATFORM_*/



/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Channel definition																				*/
/*--------------------------------------------------------------------------------------------------*/
#define	COM2_OPT1		0x01				/* CH2 : C1 message										*/
#define	COM3_OPT2		0x02				/* CH3 : C2 message										*/

/*--------------------------------------------------------------------------------------------------*/
/*	MEMOBUS Message communication header section definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define	MSG_MAXBUFSZB			582				/* Buffer size(>=MaximumMemobusSize)				*/
#define	MBUS_MINSIZE			8				/* MEMOBUSMinimum number of message bytes					*/
#define	MBUS_MAXSIZE			480				/* MEMOBUSMaximum number of message bytes(LONG)*/
#define	MBUS_HDROFS				0x00			/* Header section size(0Byte)							*/
#define	MBUS_MIN_REGNUM			1				/* Minimum register read/write number						*/





#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
typedef	struct	_MEMOBUS_ADDR_MAP {
	LONG				AxisAddress;
	REG_MANAGER_HANDLE	*hRegMngr;
	void				*Axis;		/* Start address for each axis */
} MEMOBUS_ADDR_MAP;

/*--------------------------------------------------------------------------------------------------*/
/*		Register IF data definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	_MEMOBUS_IF {					/* Message IF structure definition						*/
	UCHAR	RxBuf[ MSG_MAXBUFSZB ];				/* Receive data buffer								*//* <S01C> */
	UCHAR	TxBuf[ MSG_MAXBUFSZB ];				/* Send data buffer								*//* <S01C> */
//	UCHAR	*RxBuf;								/* Receive data buffer								*//* <S01C> */
//	UCHAR	*TxBuf;								/* Send data buffer								*//* <S01C> */
	UCHAR	*CmdBuf;							/* Command message buffer pointer					*/
	UCHAR	*ResBuf;							/* Response message buffer pointer					*/
	USHORT	RelayHdSize;						/* Relay command header size						*/
	USHORT	RcvbLength;							/* Receive data length(Byte unit)							*/
	USHORT	RcvwLength;							/* Receive data length(Word unit)							*/
	USHORT	TrnbLength;							/* Transmission data length(Byte unit)							*/
	LONG	NodeAddress;						/* Own station address										*/
	ULONG	ComTimeLong;						/* For communication timeout measurement							*/

	MEMOBUS_ADDR_MAP	AxisMap[MAX_AXIS_NUMBER];/* Axis address definition									*/
	
	ULONG	ErrRno;								/* register read:MEMOBUSError information				*/	/* <S056> */
	USHORT	OpeDspMode;							/* Operator user constant display selection					*/	/* <S056> */

} MEMOBUS_IF;



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/* MEMOBUS Message processing initialization */
void	MbusFuncInitialize(MEMOBUS_IF *MbusIfPtr, MEMOBUS_ADDR_MAP *AxisCfg, LONG node_address );

/* MEMOBUS Message analysis & execution processing	*/
USHORT	MbusFuncMsgProcedure( MEMOBUS_IF *MbusIfPtr );

/* MEMOBUS Message buffer SwapProcessing(BigEndianFor) */
void mbusFuncBufSwap( UCHAR *RxBuf );
#endif	/*_MERCURY_PLATFORM_*/




/***************************************** end of file **********************************************/
#define _MEMOBUS_IF_H /* For reload prevention */
#endif /* _MEMOBUS_IF_H */

#ifndef	_M3MsgIF_H /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		M3msgIF.h : Round processing function definition															*/
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
/*	Note	:	First edition	2010.09.03	T.Kira	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "MemobusIF.h"


/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
//#define	OPT1_PORT_USE	1					/* M-3When boardOPT1Communication portG2 i/fUse			*/
//#define	OPT2_PORT_USE	1					/* M-3When boardOPT2Communication portG2 i/fUse			*/

/****************************************************************************************************/
/*																									*/
/*		Macro Definition																			*/
/*																									*/
/****************************************************************************************************/
#define	OPT_MSG_TIMEOUT			20000			/* 20[sec]											*/
/*--------------------------------------------------------------------------------------------------*/
typedef enum M3_MSG_STEP_ {

	M3MSGSTEP_INITIAL = 0,					/* Initial step					*/
	M3MSGSTEP_START_RX,						/* Start reception						*/
	M3MSGSTEP_WAIT_RX,						/* Confirm reception completion					*/
	M3MSGSTEP_EXE,							/* Message analysis&Process execution		*/
	M3MSGSTEP_WAIT_STX,						/* Transmission start						*/
	M3MSGSTEP_WAITETX,						/* Confirm transmission completion					*/
}M3_MSG_STEP;

/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/

/*--------------------------------------------------------------------------------------------------*/
/*		Communication manager definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct	{

	/* IF data definition */
	MEMOBUS_IF	MbusIf;  

	/* For internal processing */
	UCHAR		Channel;		/* Channel information					*/
	M3_MSG_STEP	stateOfM3Msg;	/* Transmission/reception monitoring sequenceStep			*/
	LONG		Rc;				/* Variable to temporarily store the return value		*/

} M3MSGIFV;

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
//<S150>void	M3MsgIfInit(M3MSGIFV *M3MsgPtr, MEMOBUS_ADDR_MAP *AxisCfg, UCHAR	Channel);
//<S150>void	M3MsgIfManager( M3MSGIFV *M3MsgPtr );


/****************************************************************************************************/
/*		EXTERN LIST																					*/
/****************************************************************************************************/
/* M-3 Message Interface */
//extern M3MSGIFV			M3MsgC1V;						/* C1Message communication data				*/
//extern M3MSGIFV			M3MsgC2V;						/* C2Message communication data				*/

/***************************************** end of file **********************************************/
#define _M3MsgIF_H /* For reload prevention */
#endif /* _M3MsgIF_H */

/****************************************************************************************************/
/*																									*/
/*																									*/
/*		M3MsgIF.c : MECHATROLINK-ⅢMessage communication IF management											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : MECHATROLINK-ⅢPerform message communication IF processing												*/
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
/*	Note	:	First edition	2010.09.06	T.Kira	For INGRAM												*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "NetDriver.h"
#include "M3msgIF.h"
#include "MercuryGlobals.h"	/* <S150> */
#include "Sif.h"			/* <S150> */
#include "MsgIfDef.h"		/* <S150> */


/* <S150> >>>>> */
/****************************************************************************************************/
/*																									*/
/*		Variable Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Local Parameter Structure Definition														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct {							/*														*/
	LONG	Rc;								/* Variable to temporarily store the return value							*/
	UCHAR	Channel;						/* Channel information										*/
	UCHAR	Reserved[3];					/* Preliminary												*/
	UCHAR	RxBuf[MSG_MAXBUFSZB];			/* Receive data buffer									*/
	UCHAR	TxBuf[MSG_MAXBUFSZB];			/* Send data buffer									*/
} MM3SIFV;

/****************************************************************************************************/
/*																									*/
/*		Function Prototypes																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Kpx Functions																				*/
/*--------------------------------------------------------------------------------------------------*/
void	KpxInitMlinkMsgIf(						/* MECHATROLINKCommunication message IF initialization					*/
		LONG	DriveID,						/* Drive model ID									*/
		LONG	ComAddr,						/* Communication address										*/
		LONG	AxisNumber,						/* Number of multi-axis drive axes(0/1/2/...)						*/
		UCHAR	Channel							/* Channel										*/
		);
/*--------------------------------------------------------------------------------------------------*/
/*		Kpi Functions																				*/
/*--------------------------------------------------------------------------------------------------*/
#if 0	//No substance
void	KriInitMlinkHmsg(						/* MECHATROLINKCommunicationHmsgInitialization process					*/
		HMSGIF	Hmsg		);					/* Message IF handle(Pointer)					*/
#endif
/*--------------------------------------------------------------------------------------------------*/
void	KriM3MsgIfManager( CSIF *this );		/* MECHATROLINKCommunication message transmission/reception monitoring					*/
/*--------------------------------------------------------------------------------------------------*/
LONG	KriCheckRxMlinkMsg(						/* MECHATROLINKCommunication command message reception check				*/
		struct _CSIF *this,						/* this pointer										*/
		USHORT* pCmdMsgLen	);					/* Command message length storage destination pointer					*/
/*--------------------------------------------------------------------------------------------------*/
LONG	KriStartTxMlinkMsg(						/* MECHATROLINKCommunication response message transmission start command				*/
		struct _CSIF *this,						/* this pointer										*/
		UCHAR*	TxBuf,							/* Send data buffer								*/
		LONG	TxSize		);					/* Send data size(ByteSize,Zero if no response)		*/
/*--------------------------------------------------------------------------------------------------*/


MM3SIFV		MM3SifV[2];							/* Local Variables for M3 Message						*/
CSIF		M3Sif[2];							/* MECHATROLINK Interface Instance					*/
extern void KpxMsgManager2( UCHAR Channel );	/* Message processing								*/

/****************************************************************************************************/
/*																									*/
/*		MECHATROLINKCommunication message IF initialization														*/
/*																									*/
/****************************************************************************************************/
void	KpxInitMlinkMsgIf( LONG DriveID, LONG ComAddr, LONG AxisNumber, UCHAR Channel )
{
	CSIF 	*pSif;
	MM3SIFV	*pSifV;

	pSif = &M3Sif[Channel];
	pSifV = &MM3SifV[Channel];

	MlibResetByteMemory( pSifV, sizeof(MM3SIFV) );
	MlibResetLongMemory( pSif, sizeof(CSIF)/4 );		/* MECHATROLINKMessage IF variable initialization		*/
/*--------------------------------------------------------------------------------------------------*/
	pSif->Com = pSifV;									/* 										*/
	pSif->AxisNumber = AxisNumber;						/* set axis number 						*/
	pSif->DoRxTx = NULL;								/* TODO: RxTx sequence 					*/
	pSif->ChkCmd = KriCheckRxMlinkMsg;					/* check receive cmd method 			*/
	pSif->StxRes = KriStartTxMlinkMsg;					/* start tx response method 			*/
/*--------------------------------------------------------------------------------------------------*/
	pSif->RcvBufSize = MSG_MAXBUFSZB;					/* rcv buf size */
	pSif->SndBufSize = MSG_MAXBUFSZB;					/* send buf size */
/*--------------------------------------------------------------------------------------------------*/
	pSif->RxBuf = pSifV->RxBuf; 						/* rcv buf */
	pSif->TxBuf = pSifV->TxBuf; 						/* send buf */
/*--------------------------------------------------------------------------------------------------*/
	pSifV->Channel = Channel;
	return;
}

/****************************************************************************************************/
/*																									*/
/*		MECHATROLINKCommunication command message reception check														*/
/*																									*/
/****************************************************************************************************/
LONG	KriCheckRxMlinkMsg(
		struct _CSIF *this,						/* this pointer										*/
		USHORT* pCmdMsgLen )					/* pointer to the message length					*/
{
/*--------------------------------------------------------------------------------------------------*/
/*		When reception is completed																					*/
/*--------------------------------------------------------------------------------------------------*/
		*pCmdMsgLen = this->RxCmdLen;
		return( TRUE );
}

/****************************************************************************************************/
/*																									*/
/*		MECHATROLINKCommunication response message transmission start command													*/
/*																									*/
/****************************************************************************************************/
LONG	KriStartTxMlinkMsg(							/* Response message transmission start command processing				*/
		struct _CSIF *this,							/* this pointer										*/
		UCHAR*	TxBuf,								/* Send data buffer							*/
		LONG	TxSize )							/* Send data size(ByteSize,Zero if no response)	*/
{
	this->TxResLen = TxSize;						/* Send size only */
	return( TRUE );									/* Normal return									*/
}

/* <<<<< <S150> */

#if 0	/* <S150> */
/****************************************************************************************************/
/*																									*/
/*		MECHATROLINK-ⅢMessage Initialization process														*/
/*																									*/
/****************************************************************************************************/
void M3MsgIfInit(M3MSGIFV *M3MsgPtr, MEMOBUS_ADDR_MAP *AxisCfg, UCHAR Channel )
{
	ND_PARAM *nd_param;
	
	/* Communication parameter acquisition */
	nd_param = NdGetNdParamPtr(); 

	/* Memobus IF Initialize */
	MbusFuncInitialize( (MEMOBUS_IF*)&(M3MsgPtr->MbusIf), AxisCfg, (LONG)nd_param->NodeAddress);

	/* Transmission/reception monitoring sequenceStep */
	M3MsgPtr->stateOfM3Msg = M3MSGSTEP_INITIAL;

	/* Channel */
	M3MsgPtr->Channel = Channel;

}
#endif	/* <S150> */


/****************************************************************************************************/
/*																									*/
/*		MECHATROLINK-ⅢCommunication message communication fixed-cycle monitoring processing ( Round Cycle )								*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : MECHATROLINK-ⅢCommunication driverAPIM-ⅢControls the reception and transmission of communication data.	*/
/*			VersusC1Master communication and pairC2Master communication are performed in parallel. Both are independent and do not interfere.			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*	0) Initial step				: Transfer to reception start state												*/
/*	1) Start reception					: M-ⅢStart message receptionAPIis the reception start state.	*/
/*	2) Confirm reception completion				: M-ⅢMessage reception completionAPIis the reception completion state.	*/
/*  3) Message analysis&Process execution	: Analyze receive data and execute command					*/	
/*	   Send data set			: Set send data											*/
/*	4) Confirm transmission completion				: M-ⅢIssue a transmission start command to the driver.									*/
/*	5) Transmission complete					: M-ⅢReturn to the initial step after confirming that the driver has completed the transmission.	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	M3MsgIfManager( void )
{
	KriM3MsgIfManager( &M3Sif[0] );	/* C1Message */
	KriM3MsgIfManager( &M3Sif[1] );	/* C2Message */
}

void	KriM3MsgIfManager( CSIF *this )
{
	MM3SIFV	*pSifV;
	USHORT	RcvWordSize;

	pSifV = (MM3SIFV *)this->Com;

	/*----------------------------------------------------------------------------------------------*/
	/*		M-ⅢMessage communication transmission/reception monitoring sequence												*/
	/*----------------------------------------------------------------------------------------------*/
	switch( this->SqStep )
	{
		/*------------------------------------------------------------------------------------------*/
		/*						Initial step														*/
		/*------------------------------------------------------------------------------------------*/
		case M3MSGSTEP_INITIAL:
			this->SqStep = M3MSGSTEP_START_RX;	/* Start reception processingStep */
			//break;
		/*------------------------------------------------------------------------------------------*/
		/*						Start reception															*/
		/*------------------------------------------------------------------------------------------*/
		case M3MSGSTEP_START_RX:
			pSifV->Rc = NdReqRecvMsg( pSifV->Channel );
			if( pSifV->Rc == ND_SUCCESS )							/* Received? */
			{
				this->SqStep = M3MSGSTEP_WAIT_RX;					/* Wait for reception completionSTEP */
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*						Confirm reception completion														*/
		/*------------------------------------------------------------------------------------------*/
		case M3MSGSTEP_WAIT_RX:

			/* Wait for reception completion */	
			pSifV->Rc = NdGetRecvMsgSts( pSifV->Channel,
									  &RcvWordSize, &this->RxCmdLen );

			if( pSifV->Rc == ND_SUCCESS )
			{
				/* Read message data */
				NdReadMsgData( pSifV->Channel, (USHORT*)this->RxBuf, RcvWordSize );
				
				this->SqStep = M3MSGSTEP_EXE;					/* Message analysis&Process executionSTEP*/
			}
			else if( pSifV->Rc != ND_RECV_BUSY )				/* When not processingERROR				*/
			{
				this->SqStep = M3MSGSTEP_INITIAL;				/* ERROREven at initialSTEP			*/
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*						Message analysis&Process execution												*/
		/*------------------------------------------------------------------------------------------*/
		case M3MSGSTEP_EXE:
		
			/* Receive dataSwapProcessing */
			mbusFuncBufSwap( this->RxBuf);

			/* Message analysis&Process execution */
			KpxMsgManager2( pSifV->Channel );

			/* Protocol ID error	(Abort) */
			if( this->TxResLen == 0 )
			{
				NdSendMsgAbort( pSifV->Channel );		/* Send abort message	*/
				this->SqStep = M3MSGSTEP_INITIAL;		/* InitialSTEP			*/
				return;
			}

			/* Send messageSwapProcessing */
			mbusFuncBufSwap( this->TxBuf );
			
			/*------------------------------------------------------------------------------------------*/
			/*						Send data set													*/
			/*------------------------------------------------------------------------------------------*/
			/* Message data writing */
			NdWriteMsgData( pSifV->Channel, (USHORT*)pSifV->TxBuf, this->TxResLen );
			
			/* Transition to waiting for transmission completion */
			this->SqStep = M3MSGSTEP_WAIT_STX;

			break;
		/*------------------------------------------------------------------------------------------*/
		/*						Transmission start															*/
		/*------------------------------------------------------------------------------------------*/
		case M3MSGSTEP_WAIT_STX:

			/* Transmission start command	*/
			if( ND_SUCCESS != NdReqSendMsg( pSifV->Channel ))
			{
				NdSendMsgAbort( pSifV->Channel );			/* Send abort message			*/
				this->SqStep = M3MSGSTEP_INITIAL;		/* ERROR: Return to initial step		*/
			}
			else
			{
				this->SqStep = M3MSGSTEP_WAITETX;		/* Transition to waiting for transmission completion				*/
			}

			break;
		/*------------------------------------------------------------------------------------------*/
		/*						Confirm transmission completion														*/
		/*------------------------------------------------------------------------------------------*/
		case M3MSGSTEP_WAITETX: 
			if( ND_SEND_BUSY != NdGetSendMsgSts( pSifV->Channel ))
			{
				this->SqStep = M3MSGSTEP_INITIAL;		/* InitialSTEP					*/
			}			
			break;
		/*------------------------------------------------------------------------------------------*/
		/*						Default(Insurance)													*/
		/*------------------------------------------------------------------------------------------*/
		default:
			 break;
	}
	
	return;
}


/***************************************** end of file **********************************************/

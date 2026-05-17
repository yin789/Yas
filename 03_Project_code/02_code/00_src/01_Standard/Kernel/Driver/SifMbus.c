/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SifMbus.c : Serial communication interface													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Sends and receives serial communication.															*/
/*																									*/
/*			1) Receives serial communication command message and notifies completion of reception.								*/
/*			2) Receive a request to start sending a serial communication response message and start sending it.						*/
/*			3) The transmission/reception monitoring sequence controls the reception and transmission of serial communication data.				*/
/*			4) Message header analysis is performedMsgMngr.c.								*/
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
/*		Rev.2.xx : 2013.4.23	K.Ozaki																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include	"KnlApi.h"
#include	"MsgIfDef.h"
#include	"UartMsgIF.h" /* Zantei */
#include 	"FirmDownload.h"		/* <S041> */

#define SIF_DEBUG (0)		/* 1:Debug Mode */

/****************************************************************************************************/
/*																									*/
/*		Function list																					*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Kpx function definition: Function name cannot be changed																*/
/*--------------------------------------------------------------------------------------------------*/
void	KpxSifMsgManager( void );			/* Serial communication message transmission/reception monitoring						*/
void	KpxInitSifMsgManager( 				/* Serial communication message management initialization						*/
				LONG ch,					/* UART channel number (0,1,2)							*/
				CSIFCOMPORT *Com,			/* UART Com port 										*/
				LONG Axis);					/* Axis Number											*/
//<S150>				void *pUartMbusServer);		/* Zantei USBSimilarly erase later								*/
extern LONG RkiMsgifGetCmdMsgLength( UCHAR *CmdBuf, LONG CmdBufLen );	/* <S150> */

/*--------------------------------------------------------------------------------------------------*/
/*		Lpx function definition: Function name change free																*/
/*--------------------------------------------------------------------------------------------------*/
void	LpxSifStartRxDMA( CSIF *this );		/* SIF reception DMA start processing				*/
void	LpxSifStopxRxDMA( CSIF *this );		/* SIF reception DMA stop processing							*/
void	LpxSifStartTxDMA( CSIF *this );		/* SIF transmit DMA start processing							*/
/*--------------------------------------------------------------------------------------------------*/
//LONG	LpxSifChkRxHeader( CSIF *this );	/* Check receive message header							*/
//LONG	LpxSifChkRxMsgEnd( CSIF *this );	/* Check receive message end							*/
LONG	LpxSifMbusChkMyMsg( CSIF *this );	/* Check receive message address						*/
#if 0	/* RkiMsgifGetCmdMsgLength <S150> */
LONG	LpxSifGetCmdMsgLength(				/* Check recieve message length							*/
				UCHAR *CmdBuf,
				LONG CmdBufLen );
#endif
/*--------------------------------------------------------------------------------------------------*/
//LONG	LpxSifCheckRxDMA( CSIF *this );		/* DMA reception confirmation										*//* @@DMANot used */
LONG	LpxSifCheckRxData( CSIF *this,
						   LONG mode );		/* UARTReception confirmation											*/
LONG	LpxSifSetTxData( CSIF *this );		/* UARTSend processing											*/

/*--------------------------------------------------------------------------------------------------*/
/* 		Serial interface function																	*/
/*		(Argument, return value cannot be changed																		*/
/*--------------------------------------------------------------------------------------------------*/
void	LpiSifMsgManager( CSIF *this );		/* Serial communication message transmission/reception monitoring						*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpiCheckRxSerialMsg( 				/* Serial communication command message reception check				*/
		CSIF *this,							/* serial I/F class pointer								*/
		USHORT *pCmdMsgLen );				/* Command message length storage destination pointer						*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpiStartTxSerialMsg(				/* Serial communication response message transmission start command				*/
		CSIF	*this,						/* serial I/F class pointer								*/
		UCHAR*	TxBuf,						/* Send data buffer									*/
		LONG	TxSize		);				/* Send data size(ByteSize,Zero if no response)			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpiStartTxSerialMsgNoResponse(		/* Serial communication response message transmission start command(For no response)		*/
		CSIF	*this,						/* serial I/F class pointer								*/
		UCHAR*	TxBuf,						/* Send data buffer									*/
		LONG	TxSize		);				/* Send data size(ByteSize,Zero if no response)			*/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Variable definition																					*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
CSIF	LSif[SIF_N_CH];						/* Serial Communication Management IF Class				*/
/*--------------------------------------------------------------------------------------------------*/
/* <S150> >>>>> */
typedef	struct {							/*														*/
	UCHAR	RxBuf[MSG_MAXBUFSZB];			/* Receive data buffer									*/
	UCHAR	TxBuf[MSG_MAXBUFSZB];			/* Send data buffer									*/
} MSIFV;
/*--------------------------------------------------------------------------------------------------*/
MSIFV	MSifV[SIF_N_CH];					/* Local Variables for Serial Message				*/
/* <<<<< <S150> */
/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
#undef RESMT_COMPLETE
#define	RESMT_COMPLETE		(0x01)			/* MsgType : Completion response									*/
#undef RESMT_NORESPONSE
#define	RESMT_NORESPONSE	(0x00)			/* MsgType : No response(Message error)						*/
/*--------------------------------------------------------------------------------------------------*/
#define	SIFREQ_STARTTX		(0x01)			/* Transmission start command											*/
#define	SIFREQ_NOANSWER		(0x02)			/* No response command											*/
/*--------------------------------------------------------------------------------------------------*/
#define	MSG_ERROR			(0x00)			/* Message error				for MEMOBUS				*/
#define	LENGTH_FIX			(0x01)			/* Determine receive message length			for MEMOBUS				*/
#define	LENGTH_NOFIX		(0x02)			/* Received message length undetermined		for MEMOBUS				*/

#define	DIGOP_MINSIZE		(6)				/* Minimum number of Digioope message bytes						*/
#define	DIGOP_MAXSIZE		(256)			/* Maximum number of DigioOpe message bytes						*/
#define	MBUS_MINSIZE		(8)				/* MEMOBUSMinimum number of message bytes						*/
#define	MBUS_MAXSIZE		(256)			/* MEMOBUSMaximum number of message bytes						*/
/*--------------------------------------------------------------------------------------------------*/
#define	SIF_GETHEADER		(0)				/* UARTData reception Header part acquisition mode(First2Byteor more)	*/
#define	SIF_GETALLDATA		(1)				/* UARTData reception All data acquisition mode					*/
/*--------------------------------------------------------------------------------------------------*/
#define	SIF_RX_SUCCESS		(1)				/* UARTMessage reception successful								*/
#define	SIF_RX_CONTINUE		(0)				/* UARTContinue message reception								*/
#define	SIF_RX_ERROREND		(-1)			/* UARTMessage abnormal termination								*/
/*--------------------------------------------------------------------------------------------------*/
const LONG DefBaudRate[8] = {               /* Serial Communication Baudrate Definition             */
	0x00002580,     /*   9600 bps */
	0x00004b00,     /*  19200 bps */
	0x00009600,     /*  38400 bps */
	0x0000E100,     /*  57600 bps(reserved) */
	0x0001C200,     /* 115200 bps(reserved) */
	0x0001F400,     /* 128000 bps(reserved) */
	0x0003D090,     /* 250000 bps(reserved) */
	0x000F4240,     /*1000000 bps(reserved) */
};
#define LPX_CHK_BRTCHG( com, bidx )          /* check baurate parameter */                           \
		( DefBaudRate[ (bidx) ] != ((com)->BRT) )  
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		SIF error definition																			*/
/*																									*/
/****************************************************************************************************/
#define SET_SIFERR( csif, code )	{ (csif)->ErrCod = code; ++((csif)->ErrCnt);}
/*--------------------------------------------------------------------------------------------------*/
/*		Error Code Definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	SIFERR_RXTMOVR		0x01			/* Message reception timeover								*/
#define SIFERR_RXDMAERR 	0x02			/* ReceivingDMAError										*/
#define	SIFERR_RXHDRERR		0x03			/* Receive header error										*/
/*--------------------------------------------------------------------------------------------------*/
#define	SIFERR_USERERR		0x10			/* User error(No response)									*/
#define	SIFERR_OTHERERR		0xFF			/* Other errors											*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*		Timer Value Definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define SIF_TXWAITTIME		  20			/* Waiting time to start transmission after reception completion [ms]					*/
#define	SIF_RXTIMEOVER		 100			/* Message reception timeover setting value   [ms]					*/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		SIF & DMA interface macro definition													*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		DMA Receive Size Definition																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	DMA_RXSIZE			(SIF_RX_BUFSIZE - 1)  		/* DMA Receive RxBuf  Size					*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_GET_RXDMA_CNT( Com ) ( *(Com->RxDMA_CNT) )
/*--------------------------------------------------------------------------------------------------*/
#define LPX_V850_BUG1( ) {;} 
/*--------------------------------------------------------------------------------------------------*/
#define LPX_TXCHKF(Com)  (((*((Com)->TXCHKF))&0x040))	/* Check TX FIFO EMPTY						*/
#define LPX_TXCHKL(Com)  (((*((Com)->TXCHKF))&0x020))	/* Check TX LATCH EMPTY						*/
#define LPX_CHK_RXRDY( com ) ((*((com)->RXCHKF)) & 0x01)

/****************************************************************************************************/
/*																									*/
/*		Serial communication message initialization process															*/
/*																									*/
/****************************************************************************************************/
void	KpxInitSifMsgManager( LONG ch, CSIFCOMPORT *Com, LONG Axis
//<S150>			,void *pUartMbusServer				/* zantei */
		)
{
CSIF *pSif = &(LSif[ch]);										/* get serial IF pointer */
/*--------------------------------------------------------------------------------------------------*/
/*  DMAStop(As a precaution)																				*/
/*--------------------------------------------------------------------------------------------------*/
	Com->RXDISABLE( );											/* Disable SIF Receive				*/
	Com->STOP_RxDMA( );											/* Stop SIF RxDMA					*/
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( pSif, sizeof( CSIF )/4 );				/* reset data	*/
/*--------------------------------------------------------------------------------------------------*/
	pSif->Com = Com; 											/* set com port */
	pSif->AxisNumber = Axis;									/* set axis number */
	pSif->DoRxTx = LpiSifMsgManager;							/* RxTx sequence */
	pSif->ChkCmd = LpiCheckRxSerialMsg;							/* check receive cmd method */
	pSif->StxRes = LpiStartTxSerialMsg;							/* start tx response method */
/*--------------------------------------------------------------------------------------------------*/
	pSif->RcvBufSize = SIF_RX_BUFSIZE;							/* rcv buf size */
	pSif->SndBufSize = SIF_TX_BUFSIZE;							/* send buf size */
/*--------------------------------------------------------------------------------------------------*/
//	pSif->pBrtPrm = &(Iprm.SifCfg[ ch ].BaudRateSel);           /* TODO:set baudrate parameter pointer   */
/*--------------------------------------------------------------------------------------------------*/

	/*----------------------------------------------------------------------------------------------*/
	/*	Below provisionallySigmaWin+Correspondence officially42HRespond to command										*/
	/*----------------------------------------------------------------------------------------------*/
/* <S150> >>>>> */
//	pSif->ComMbus42 = pUartMbusServer;							/* TODO: Zantei(USBSimilarly erase later)	*/
////	((UART_MBUS_SERVER*)pUartMbusServer)->MbusIf.RxBuf = pSif->RxBuf;	/* <S01C> */
////	((UART_MBUS_SERVER*)pUartMbusServer)->MbusIf.TxBuf = pSif->TxBuf;	/* <S01C> */
//	pSif->RxBuf = ((UART_MBUS_SERVER*)pUartMbusServer)->MbusIf.RxBuf;	/* <S01C> */
//	pSif->TxBuf = ((UART_MBUS_SERVER*)pUartMbusServer)->MbusIf.TxBuf;	/* <S01C> */
	pSif->RxBuf = MSifV[ch].RxBuf;								/* rcv buf */
	pSif->TxBuf = MSifV[ch].TxBuf;								/* send buf */
/* <<<<< <S150> */

	return;
}

/****************************************************************************************************/
/*																									*/
/*		Serial communication message fixed-cycle monitoring process ( ScanC Cycle )										*/
/*																									*/
/****************************************************************************************************/
void	KpxSifMsgManager( void )
{
	if( UART0_PORT_USE == 1) { LSif[COM0_UART].DoRxTx( &(LSif[COM0_UART]) ); /* UART ch:0 */ }
	if( UART1_PORT_USE == 1) { LSif[COM1_UART].DoRxTx( &(LSif[COM1_UART]) ); /* UART ch:1 */ }
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Serial communication message transmission/reception monitoring sequence													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : The transmission/reception monitoring sequence controls the reception and transmission of serial communication data.				*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*			0) InitialSTEP		: Reset variables and start reception processingSTEP.					*/
/*			1) Start reception processing : Start SIF reception DMA and wait for receptionSTEP.				*/
/*			2) Wait for reception		: Monitor SIF reception DMA,1byteor more is received					*/
/*							  1byteReceivingSTEP.												*/
/*			3) 1byteReceiving    : Monitor SIF reception DMA,4byteor more is received					*/
/*							  4byteReceivingSTEP.												*/
/*							  MainSTEPmonitors the reception time-over within the message.						*/
/*			4) 4byteReceiving    : Monitor SIF reception DMA, and if all messages are received,				*/
/*							  (Judging by the message length of the header section), Reception completion processingSTEP.			*/
/*							  MainSTEPto calculate the position command pulse amount until						*/
/*			5) Reception completion processing	: Stop SIF reception DMA.										*/
/*							  If in continuous transmission mode, continuous transmission processingSTEP.					*/
/*							  In normal transmission mode, the received message is checked,				*/
/*							  If the message is addressed to your own station, set the reception completion status,			*/
/*							  Waiting for transmission startSTEP.											*/
/*							  If the message is not addressed to your own station, process reception startSTEP.			*/
/*			6) Waiting for transmission start	: Transmission start processing based on a transmission start request from the applicationSTEP.			*/
/*							  Please note that the request to start transmission from the application is realSTEPaccording to [5].		*/
/*							  Request to start transmission from application within continuous transmission mode shift check time			*/
/*							  , set it to continuous transmission mode, create a continuation response message, and only when		*/
/*							  Transmission start processingSTEP.											*/
/*			7) Transmission start processing : Start SIF transmit DMA and SIF receive DMA,						*/
/*							  Waiting for transmission completionSTEP.											*/
/*			8) Waiting for transmission completion : Monitor the completion of SIF transmit DMA transmission, and when DMA transmission is completed,				*/
/*							  Confirm transmission completionSTEP.											*/
/*			9) Confirm transmission completion	: Monitor the completion of SIF transmission, and when SIF transmission is completed, send completion processingSTEP 			*/
/*							  .															*/
/*		   10) Transmission completion processing	: Send SIFDisable, waiting for receptionSTEP.						*/
/*																									*/
/*			*) Continuous transmission processing : Performs response processing to the completion confirmation message in continuous transmission mode.			*/
/*							  After creating the response message, start transmission processingSTEP.				*/
/*																									*/
/*			*) Various error processing : Performs error processing such as reception time over.									*/
/*							  When returning an abnormal response, start transmission processSTEP.				*/
/*							  When not returning an abnormal response(SumCheckErr)is the initialSTEP.			*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
enum	SQSTEP	{
	SQSTEP_INITIAL = 0,
	SQSTEP_STARTRX,
	SQSTEP_WAITRX,
	SQSTEP_CHKHEADER,
	SQSTEP_RXCMPLT,
	SQSTEP_WAITSTX,
	SQSTEP_STARTTX,
	SQSTEP_WAITETX,
	SQSTEP_CHKASIF,
	SQSTEP_TXCMPLT,
/*--------------------------------------------------------------------------------------------------*/
	SQSTEP_SIFRXERR,
};
/*--------------------------------------------------------------------------------------------------*/
void	LpiSifMsgManager( CSIF *this )
{
USHORT	timer;
LONG	rc;
ULONG	UartSpeed;				/*+	UARTTransfer rate(bps)	<S041> */
LONG	DataAddr;				/*+	Send data address <S041>	*/
UCHAR	*pTxBuf = this->pTxRes;	/*+	Send data pointer start <S041>	*/
register CSIFCOMPORT *Com = (CSIFCOMPORT *)this->Com;
/*--------------------------------------------------------------------------------------------------*/
/*		Serial communication transmission/reception monitoring sequence 															*/
/*--------------------------------------------------------------------------------------------------*/
		switch( this->SqStep )
		{
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_INITIAL: 	/* Initial Step														*/
/*--------------------------------------------------------------------------------------------------*/
			Com->RSTRXSTS( );										/* reset recv. status			*/
//TODO		Com->SETBRT( Com, DefBaudRate[ *(this->pBrtPrm) ] );	/* set baudrate					*/
			Com->RESET( );											/* reset serial					*/
/*--------------------------------------------------------------------------------------------------*/
			this->ReqStx = 0x00;									/* reset Tx request				*/
			this->SqStep = SQSTEP_STARTRX;							/* start Rx						*/
/*--------------------------------------------------------------------------------------------------*/
			break;
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_STARTRX: 	/* Start Receive Message											*/
/*--------------------------------------------------------------------------------------------------*/
//			LpxSifStartRxDMA( this );								/* Receive DMA start			*//* @@DMANot used */
			Com->RXENABLE( );										/* Enable SIF Receive			*/
			this->SqStep = SQSTEP_WAITRX;							/* Wait for receptionSTEP			*/
			this->RxCmdCnt = 0;										/* Clear receive data count			*/
			break;
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_WAITRX:		/* Wait Receive Message												*/
/*--------------------------------------------------------------------------------------------------*/
			if( Com->CHK_RXERR() )                  				/* Check Serial Com. Error		*/
			{ /* OverRun, Parity, Framing Error */
				Com->RXDISABLE( );									/* Disable SIF Receive			*/
//				Com->STOP_RxDMA( );									/* Stop SIF RxDMA				*//* @@DMANot used */
				this->SqStep = SQSTEP_INITIAL;						/* Goto Initial Step			*/
				break;
			}
			else
			{ /* Communication Error is Notiong */
				rc = LpxSifCheckRxData( this, SIF_GETHEADER );		/* Check Receive data			*/
				if( rc == SIF_RX_SUCCESS )
				{ /* Get Header Data */
					this->SqStep = LpxSifMbusChkMyMsg( this );	/* TODO: Received message check process	*/
//					this->SqStep = SQSTEP_CHKHEADER;				/* Check the Header Data		*/
//					this->RxCmdCnt = rc;							/* Set Rx Count					*/
//					this->SqStep = SQSTEP_RXCMPLT;					/* Receive Complete				*/
				}
				else if( rc == SIF_RX_ERROREND )
				{ /* Error End */
					Com->RXDISABLE( );								/* Disable SIF Receive			*/
					this->SqStep = SQSTEP_INITIAL;					/* Goto Initial Step			*/
				}
				else /* SIF_RX_CONTINUE */
				{
//					if(	SysDl.DlStatus == 0x0001 )	/* <S041> */
					if(	SYSDL_CHK_DLMODE_ON() )	/* <S077> */
					{	/* Bps Time Out Check 10sec	*/
						if( KpiGetLongTimer( &(SysDl.BpsChkTime) ) > 10000 )
						{	/*	From download start state10seconds19200bps	*/
							if(SysDl.NowBps != 19200)
							{	/* The current BPS value is19200bps */
								KpxInitCpuSerialIF( COM0_UART, 19200, 8, 2, 1);
							}
							KpiRstLongTimer( &(SysDl.BpsChkTime) );	/* Reset BpsChkTimer			*/
							SYSDL_DLMODE_RESET();					/* DownLoad mode stop <S077>	*/
						}
					}
					; /* CONTINUE */
				}
			}
			break;
#if 0
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_CHKHEADER:	/* receive complete													*/
/*--------------------------------------------------------------------------------------------------*/
			this->SqStep = LpxSifMbusChkMyMsg( this );	/* TODO: Received message check process		*/
			break;
#endif
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_RXCMPLT:	/* receive complete													*/
/*--------------------------------------------------------------------------------------------------*/
			if( Com->CHK_RXERR() )                  				/* Check Serial Com. Error		*/
			{ /* OverRun, Parity, Framing Error */
				Com->RXDISABLE( );									/* Disable SIF Receive			*/
//				Com->STOP_RxDMA( );									/* Stop SIF RxDMA				*//* @@DMANot used */
				this->SqStep = SQSTEP_INITIAL;						/* Goto Initial Step			*/
//				break;
			}
			else
			{ /* Communication Error is Notiong */
				rc = LpxSifCheckRxData( this, SIF_GETALLDATA );		/* Check Receive data			*/
				if( rc == SIF_RX_SUCCESS )
				{ /* Get Header Data */
//					LpxSifStopxRxDMA( this );						/* Stop RxDMA					*/
					Com->RXDISABLE( );								/* Disable SIF Receive			*/
					KpiRstLongTimer( &(this->StxTimer) );			/* Reset TxTimer				*//* @@DMANot used */
//					this->SqStep = LpxSifMbusChkMyMsg( this );/* TODO: Received message check process	*/
					this->SqStep = SQSTEP_WAITSTX;					/* Check the Header Data		*/
				}
				else if( rc == SIF_RX_ERROREND )
				{ /* Error End */
					Com->RXDISABLE( );								/* Disable SIF Receive			*/
					this->SqStep = SQSTEP_INITIAL;					/* Goto Initial Step			*/
				}
				else /* SIF_RX_CONTINUE */
				{
					; /* CONTINUE */
				}
			}
			break;
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_WAITSTX:	/* Waiting for transmission start 													*/
/*--------------------------------------------------------------------------------------------------*/
			if( this->ReqStx == SIFREQ_STARTTX )					/* Transmission start request check			*/
			{
				this->ReqStx  = 0x00;								/* Reset ReqStx					*/
				this->SqStep  = SQSTEP_STARTTX;						/* Transmission start processingSTEP		*/
			}
			else if( this->ReqStx == SIFREQ_NOANSWER )				/* No response(MsgErr)Request check	*/
			{
				this->ReqStx  = 0x00;								/* Reset ReqStx					*/
				this->SqStep  = SQSTEP_INITIAL;						/* Return to initial step			*/
			}
			break;
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_STARTTX:	/* Transmission start processing														*/
/*--------------------------------------------------------------------------------------------------*/
			timer = KpiGetLongTimer( &(this->StxTimer) );	/* Reply nowMessage, it seems that it will not work unless you send		*/
			if( timer >= SIF_TXWAITTIME )					/* Since the other party may not be able to receive,	*/
			{												/* Adjust the waiting time here				*/
#if TX_DMA_USE == 1
				LpxSifStartTxDMA( this );					/* Transmission DMA start					*//* @@DMANot used */
#else
				Com->TXENABLE( );							/* SIF Transmit Enable					*/
				this->TxResCnt = this->TxResLen;			/* Get Data Size						*/
#endif
				this->SqStep = SQSTEP_WAITETX;
			}
			break;
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_WAITETX:	/* Transmission data setting													*/
/*--------------------------------------------------------------------------------------------------*/
#if TX_DMA_USE == 1
			if( Com->CHK_TxDMA_TC( ) )								/* DMA transmission completion check		*//* @@DMANot used */
#else
			if( TRUE == LpxSifSetTxData(this) )						/* UARTSend processing					*/
#endif
			{ /* When All Data Tx Complete */
				this->SqStep = SQSTEP_CHKASIF;						/* Confirm transmission completionSTEP		*/
			}
			break;
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_CHKASIF:	/* Confirm transmission completion														*/
/*--------------------------------------------------------------------------------------------------*/
			if( LPX_TXCHKF( Com ) )				    /* SIF transmission completion check					*/
			{/*Confirmation of change completion when changing serial speed<S041>*/
				DataAddr = 0;	/*	Send data start	*/
				/*Only when downloading the system*/
				if(pTxBuf[DataAddr+1] == 0x50)
				{
					/*Only when responding to serial IF speed change command*/
					if((pTxBuf[DataAddr+2]==0x00) && (pTxBuf[DataAddr+3]==0x0D))
					{
						/*Switching transfer speed data creation*/
						UartSpeed = pTxBuf[DataAddr+9];
						UartSpeed += (pTxBuf[DataAddr+8] * 0x100);
						UartSpeed += (pTxBuf[DataAddr+7] * 0x10000);
						UartSpeed += (pTxBuf[DataAddr+6] * 0x1000000);
					/*	Transfer rate switching execution	*/
					KpxInitCpuSerialIF( COM0_UART, UartSpeed, 8, 2, 1);	/*	at specified transfer speed	*/
					}
				}
				this->SqStep = SQSTEP_TXCMPLT;		/* Transmission completion processingSTEP					*/
			}
			break;
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_TXCMPLT:	/* Transmission completion processing 													*/
/*--------------------------------------------------------------------------------------------------*/
			Com->STOP_TxDMA( );							/* stop DMA 								*/
			Com->TXDISABLE( );							/* Transmit Disable							*/
			LPX_V850_BUG1( );							/* V850E2/ME3:TxDMAMalfunction measures			*/
/*--------------------------------------------------------------------------------------------------*/
//			if ( LPX_CHK_BRTCHG( Com, *(this->pBrtPrm) ) )             /* Check baudrate chg  */
//			{
//				Com->RXDISABLE( );					                 /* Disable SIF Receive	*/
//				Com->STOP_RxDMA( );					                 /* Stop SIF RxDMA		*/
//				this->SqStep = SQSTEP_INITIAL;				                 /* Initial step		*/
//			}
/*--------------------------------------------------------------------------------------------------*/
//			else 
			{
				//this->SqStep = SQSTEP_WAITRX;			/* Wait for receptionSTEP						*/
				this->SqStep = SQSTEP_INITIAL;			/* Goto Initial Step					*/
			}
			break;
/*--------------------------------------------------------------------------------------------------*/
		case SQSTEP_SIFRXERR:	/* Receiving error 														*/
/*--------------------------------------------------------------------------------------------------*/
//			LpxSifStopxRxDMA( this );					/* Stop RxDMA								*//* @@DMANot used */
			Com->RXDISABLE( );							/* Disable SIF Receive						*/
			this->SqStep = SQSTEP_INITIAL;				/* Goto Initial Step						*/
			break;
/*--------------------------------------------------------------------------------------------------*/
		default:
/*--------------------------------------------------------------------------------------------------*/
			break;
		}
/*--------------------------------------------------------------------------------------------------*/
	return;
}



/****************************************************************************************************/
/*																									*/
/*		MEMOBUSMessage reception completion confirmation API function service (Round)									*/
/*																									*/
/*			Return value:MEMOBUSWhen message is received normally	: TRUE									*/
/*					If not received							: FALSE									*/
/*																									*/
/*			The caller should check the return value of this functionTRUEand then use it after confirming that it is	*/
/*			The manager thenDpxSetSifMbusTrnsStart()receive buffer until receive command to start transmission.	*/
/*			will not be accessed.																	*/
/****************************************************************************************************/
LONG	LpiCheckRxSerialMsg( CSIF *this, USHORT *pCmdMsgLen )
{
/*--------------------------------------------------------------------------------------------------*/
/*		When reception is completed																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( this->SqStep == SQSTEP_WAITSTX )
		{
			*pCmdMsgLen = this->RxCmdCnt;
			//*pCmdMsgLen = this->RxCmdLen;
			return( TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Receiving incomplete																					*/
/*--------------------------------------------------------------------------------------------------*/
		return( FALSE );
}


/****************************************************************************************************/
/*																									*/
/*		MEMOBUSMessage transmission start request API function service (Round)									*/
/*																									*/
/*			Return value:																				*/
/*						(1)Normal end																	*/
/*						(0)Sequence error															*/
/*																									*/
/****************************************************************************************************/
LONG	LpiStartTxSerialMsg(				/* Serial communication response message transmission start command				*/
		CSIF	*this,						/* serial I/F class pointer								*/
		UCHAR*	TxBuf,						/* Send data buffer									*/
		LONG	TxSize		)				/* Send data size(ByteSize,Zero if no response)			*/
{
/*--------------------------------------------------------------------------------------------------*/
/*		Response message type check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( this->SqStep != SQSTEP_WAITSTX )		/* Sequence check							*/
		{
			return( FALSE );						/* Abnormal return									*/
		}
		else if( TxSize > 0 )						/* Send data size check						*/
		{
			this->pTxRes   = TxBuf;					/* Response buffer address setting						*/
			this->TxResLen = (USHORT)TxSize;		/* Response message length(ByteNumber)Setting					*/
			this->ReqStx   = REQSTX_STARTTX;		/* Transmission start request setting						*/
			return( TRUE );
		}
		else
		{
			this->pTxRes   = NULL;					/* Response buffer address setting						*/
			this->TxResLen = 0;						/* Response message length(ByteNumber)Setting					*/
			this->ReqStx   = REQSTX_NORESPONSE;		/* No response request setting							*/
			return( TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
//		return( FALSE );							/* Abnormal return									*/
}


/****************************************************************************************************/
/*																									*/
/*		MEMOBUSMessage transmission start request API function service (forced no response)								*/
/*																									*/
/*			Return value:																				*/
/*						(1)Normal end																	*/
/*						(0)Sequence error															*/
/*																									*/
/****************************************************************************************************/
LONG	LpiStartTxSerialMsgNoResponse(		/* Serial communication response message transmission start command(For no response)		*/
		CSIF	*this,						/* serial I/F class pointer								*/
		UCHAR*	TxBuf,						/* Send data buffer									*/
		LONG	TxSize		)				/* Send data size(ByteSize,Zero if no response)			*/
{
	return LpiStartTxSerialMsg( this, NULL, 0 );
}



/****************************************************************************************************/
/*																									*/
/*		SIF reception DMA start processing																*/
/*																									*/
/****************************************************************************************************/
void	LpxSifStartRxDMA( CSIF *this )
{
CSIFCOMPORT *pCom = (CSIFCOMPORT *)this->Com;						/* get com port 				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Set RxDMA 																					*/
/*--------------------------------------------------------------------------------------------------*/
		this->RxDmaScnt = DMA_RXSIZE;
		pCom->SETX_RxDMA( (UINT)(pCom->RXBUF), (UINT)(this->RxBuf), this->RxDmaScnt ); 
/*--------------------------------------------------------------------------------------------------*/
/*		DMA & SIF start processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		pCom->START_RxDMA( );							/* Enable RxDMA								*/
		pCom->RXENABLE( );								/* Enable SIF Receive						*/
/*--------------------------------------------------------------------------------------------------*/
		return;

}


/****************************************************************************************************/
/*																									*/
/*		SIF reception DMA stop processing																*/
/*																									*/
/****************************************************************************************************/
void	LpxSifStopxRxDMA( CSIF *this )
{
CSIFCOMPORT *Com = (CSIFCOMPORT *)this->Com;
/*--------------------------------------------------------------------------------------------------*/
/*		SIF stop processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		Com->RXDISABLE( );								/* Disable SIF Receive						*/
		Com->STOP_RxDMA( );								/* Stop SIF RxDMA							*/
/*--------------------------------------------------------------------------------------------------*/
		return;

}


/****************************************************************************************************/
/*																									*/
/*		SIF transmit DMA start processing																*/
/*																									*/
/****************************************************************************************************/
void	LpxSifStartTxDMA( CSIF *this )
{
CSIFCOMPORT *Com = this->Com;
/*--------------------------------------------------------------------------------------------------*/
/*		DMA transfer start																				*/
/*--------------------------------------------------------------------------------------------------*/
		Com->TXENABLE( );								/* SIF Transmit Enable						*/
		KlibWaitus( 1 );								/* Wait	1 us								*/
		Com->SETX_TxDMA( ((UINT)(this->pTxRes)),      	/* source address 							*/
						(UINT)(Com->TXBUF), 		    /* destination address						*/
						(this->TxResLen) ); 		    /* tx count									*/
		Com->START_TxDMA( );						    /* Start SIF TxDMA							*/
		//KlibWaitus(1);								/* Wait 1us									*/
		//Com->SWTRG_TxDMA( );							/* Start Tx ( software trigger )            */
/*--------------------------------------------------------------------------------------------------*/
		return;

}



/****************************************************************************************************/
/*																									*/
/*		UARTData transmission & confirmation																		*/
/*																									*/
/****************************************************************************************************/
LONG	LpxSifSetTxData( CSIF *this )
{
	LONG		i;
	LONG		DataAddr;
	LONG		rc = FALSE;
	UCHAR		*pTxBuf = this->pTxRes;
	CSIFCOMPORT	*Com = (CSIFCOMPORT *)this->Com;

	/* Set Current TxData Address */
	DataAddr = this->TxResLen - this->TxResCnt;

	if( this->TxResCnt != 0 )
	{ /* When Tx Request Data Exist */
//		if( LPX_TXCHKL(Com) )
		if( Com->CHK_TXEMPTY( ) )
		{ /* Tx FIFO is Empty */
			for( i=0; i<16; i++ )											/* 16 = FIFO size		*/
			{
				*(Com->TXBUF) = pTxBuf[ DataAddr++ ];						/* get a charactor		*/
				this->TxResCnt--;											/* Tx Cnt Decriment		*/

				if( this->TxResCnt == 0 )
				{ /* TXDATA is Noting */
					rc = TRUE;
					break;
				}
			}
		}
	}

	return ( rc );
}



/****************************************************************************************************/
/*																									*/
/*		UARTData reception confirmation																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		1) FIFO2msperiod.														*/
/*																									*/
/****************************************************************************************************/
LONG	LpxSifCheckRxData( CSIF *this, LONG mode )
{
	LONG		i;
	LONG		HeaderSize;
	LONG		rc = SIF_RX_CONTINUE;
	UCHAR		*pRxBuf = this->RxBuf;
	CSIFCOMPORT	*Com = (CSIFCOMPORT *)this->Com;

	if( Com->CHK_TIMEOUT( ) || Com->CHK_RXRDY( ) ) 	/* Check Charactor Timeout or Trigger Interrupt	*/
	{ /* When exist RXDATA */
		KpiRstLongTimer( &(this->RxTimer) );								/* Reset RxTimer		*/
		for( i=0; i<16; i++ )												/* 16 = FIFO size		*/
		{
			if( !LPX_CHK_RXRDY(Com) )
			{ /* RXDATA not ready */
				break;
			}
			pRxBuf[ this->RxCmdCnt++ ] = *(Com->RXBUF);						/* get a charactor		*/
		}
	}

	if( mode == SIF_GETHEADER )
	{ /* Header acquisition request */
		/* Header size acquisition */
		if( KPI_CHK_OPEMSG( pRxBuf[1]) ){ HeaderSize = DIGOP_MINSIZE; }
/* <S150> >>>>> */
		/* Header length when downloading firmware */
		else if( KPI_CHK_SYSDLMSG( pRxBuf[1] ) )
		{
			HeaderSize = MBUS_MINSIZE;	/* Normal */
			if( this->RxCmdCnt > MBUS_DL_SF_IDX )
			{
				switch( pRxBuf[MBUS_DL_SF_IDX] )
				{
				/* For program write/send commands, header size is up to the number of data */
				case MBUS_DL_PRGWR:
					/* fall through */
				case MBUS_DL_PRGSND:
					HeaderSize = MBUS_DL_DATANUM_LEN;
					break;
				default:
					break;
				}
			}
		}
/* <<<<< <S150> */
		else							{ HeaderSize = MBUS_MINSIZE; }

		if( this->RxCmdCnt >= HeaderSize )
		{
			rc = SIF_RX_SUCCESS;										/* Header reception complete		*/
		}
	}
	else /* mode == SIF_GETALLDATA */
	{ /* Request all data acquisition */
		if( this->RxCmdCnt >= this->RxCmdLen )
		{
			rc = SIF_RX_SUCCESS;										/* All data reception complete		*/
		}
	}

	/* Check Time Over */
	if( (rc == SIF_RX_CONTINUE) && (SIF_RXTIMEOVER <= KlibGetLongTimerMs(this->RxTimer)) )
	{ /* Message reception timeover */
		KpiRstLongTimer( &(this->RxTimer) );							/* Reset RxTimer		*/
		SET_SIFERR( this, SIFERR_RXTMOVR );								/* Message receptionT.O.		*/
		rc = SIF_RX_ERROREND;											/* Error termination			*/
	}


	if( mode == SIF_GETALLDATA )
	{
//		RxCmdCtr = this->RxCmdCnt;
	}

	return ( rc );
}



/****************************************************************************************************/
/*																									*/
/*		DMA reception confirmation																				*/
/*																									*/
/*		1)MICROSSPofUART FIFOuse.															*/
/*																									*/
/****************************************************************************************************/
LONG	LpxSifCheckRxDMA( CSIF *this )
{
int i;
int rxcnt;
CSIFCOMPORT *Com = (CSIFCOMPORT *)this->Com;
UCHAR *pRxBuf = this->RxBuf;

/* for debug */
//ULONG *IIRFCR_Address;
//VULONG IIRFCR_data;
//IIRFCR_Address = (ULONG*)0xEFFF4808;
/* for debug */
/*--------------------------------------------------------------------------------------------------*/
	if ( Com->CHK_TIMEOUT( ) ) 								/* Check Charactor Timeout Interrupt */
//	IIRFCR_data = ((VULONG)*IIRFCR_Address & 0x0F);
//	if ( (IIRFCR_data == 0x0C) || (IIRFCR_data == 0x04) )
//	if ( (IIRFCR_data & 0x0F) != 0x01 )
	{
	/*----------------------------------------------------------------------------------------------*/
		Com->RXDISABLE( );									/* Disable SIF Receive */
		Com->STOP_RxDMA( );									/* stop DMA */
		this->RxDmaRcnt = LPX_GET_RXDMA_CNT( Com );			/* save RxDMA count	*/
		rxcnt = this->RxDmaScnt - this->RxDmaRcnt;			/* set receive count */
//		rxcnt = 0;
	/*----------------------------------------------------------------------------------------------*/
		for( i=0; i<16; i++)								/* 16 = FIFO size */
		{
			if ( ! LPX_CHK_RXRDY( Com ) ) { break; }		/* check Rx ready flag */
			pRxBuf[ rxcnt++ ] = *(Com->RXBUF);				/* get a charactor */
		}
	/*----------------------------------------------------------------------------------------------*/
		if( rxcnt == 0 ) { Com->START_RxDMA( );	}			/* Enable RxDMA							*/
	/*----------------------------------------------------------------------------------------------*/
		return (rxcnt);
	}
	else if( Com->CHK_RxDMA_TC( ) )							/* Check Transaction Complete */
	{
		Com->RXDISABLE( );									/* Disable SIF Receive */
		Com->STOP_RxDMA( );									/* stop DMA */
		this->RxDmaRcnt = LPX_GET_RXDMA_CNT( Com );			/* save RxDMA count	*/
		rxcnt = this->RxDmaScnt - this->RxDmaRcnt;			/* set receive count */
		return (rxcnt);
	}
/*--------------------------------------------------------------------------------------------------*/
	return (0);
}



/****************************************************************************************************/
/*																									*/
/*		Check messages addressed to your own station																	*/
/*																									*/
/****************************************************************************************************/
LONG	LpxSifMbusChkMyMsg( CSIF *this )	/* Own station address check									*/
{
		if( SIF_DEBUG == 1 )
		{
			return ( SQSTEP_WAITSTX );							/* debug */
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Address part of the message received this time=="0" or ==For own station addressTRUE					*/
/*		Station address=="0", the message is processed but there is no response.									*/
/*		However, the DigiOp message and download message return a response.							*/
/*--------------------------------------------------------------------------------------------------*/
		if( KPI_CHK_OPEMSG( this->RxBuf[1] ) )
		{														/* Operator Message					*/
//<S150>			this->RxCmdLen = LpxSifGetCmdMsgLength(				/* Get Message Length				*/
			this->RxCmdLen = RkiMsgifGetCmdMsgLength(			/* Get Message Length				*/
									this->RxBuf,
									this->RcvBufSize );
			this->StxRes = LpiStartTxSerialMsg;                 /* set method for start Tx			*/
//			return ( SQSTEP_WAITSTX );							/* @@DMANot used						*/
			return ( SQSTEP_RXCMPLT );
		}
		else if ( KPI_CHK_SYSDLMSG( this->RxBuf[1] ) )
		{														/* System Download Message			*/
//<S150>			this->RxCmdLen = LpxSifGetCmdMsgLength(				/* Get Message Length				*/
			this->RxCmdLen = RkiMsgifGetCmdMsgLength(			/* Get Message Length				*/
									this->RxBuf,
									this->RcvBufSize );
			this->StxRes = LpiStartTxSerialMsg;                 /* set method for start Tx			*/
//			return ( SQSTEP_WAITSTX );							/* @@DMANot used						*/
			return ( SQSTEP_RXCMPLT );
		}
		else if( this->RxBuf[0] == 0x00 )
		{														/* Other Message( address = 0 )		*/
			this->StxRes = LpiStartTxSerialMsgNoResponse;       /* set method for no response		*/
//			return ( SQSTEP_WAITSTX );							/* @@DMANot used						*/
			return ( SQSTEP_RXCMPLT );
		}
		else if ( this->RxBuf[0] == (this->AxisNumber) )
		{														/* Other Message(address = my addr)	*/
//<S150>			this->RxCmdLen = LpxSifGetCmdMsgLength(				/* Get Message Length				*/
			this->RxCmdLen = RkiMsgifGetCmdMsgLength(			/* Get Message Length				*/
									this->RxBuf,
									this->RcvBufSize );
			this->StxRes = LpiStartTxSerialMsg;                 /* set method for start Tx			*/
//			return ( SQSTEP_WAITSTX );							/* @@DMANot used						*/
			return ( SQSTEP_RXCMPLT );
		}
		else
		{
			return ( SQSTEP_INITIAL );
		}
}



/****************************************************************************************************/
/*																									*/
/*		MessageI/F : Command message length acquisition															*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S150> Delete */
#define LPX_GET_CMDMSGLEN( i, Mul, Add )\
		if( CmdBufLen < (i+2) ){ return( 0 );}\
		else{ return( (((CmdBuf[i]<<8) + CmdBuf[i+1]) * Mul) + Add );}
#define LPX_CHK_SLOT( SfcH )			(SfcH == 0x01)
#define LPX_GET_SLOT_BYTELEN( SfcH )	((LPX_CHK_SLOT( SfcH ))? 2 : 0)
#define LPX_GET_SLOT_WORDLEN( SfcH )	((LPX_CHK_SLOT( SfcH ))? 1 : 0)
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxSifGetCmdMsgLength( UCHAR *CmdBuf, LONG CmdBufLen )
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
//			 return( -1 );//<S0C7>statement is unreachable
		case MFC_SVXMSG32:
			 switch( CmdBuf[3] )
			 {
			 case SFC_RD_SEQMEM: return( 12 + s );
			 case SFC_WR_SEQMEM: LPX_GET_CMDMSGLEN( 8 + s, 2, 12 + s );
			 default: return( -1 );
			 }
//			 return( -1 );//<S0C7>statement is unreachable
		case MFC_MECHATROLINK:
			 switch( CmdBuf[4] )
			 {
			 case SFC_RD_REQREG_42:
				 return( MBUS42_HEADERSIZE + MBUS42_ADRSIZE + MBUS42_CRC_SIZE );
			 case SFC_RD_RANREG_42:
			 	return( MBUS42_HEADERSIZE + (MBUS42_ADRSIZE * ((CmdBuf[6]<<8)|CmdBuf[7])) + MBUS42_CRC_SIZE );
			 case SFC_WR_SEQREG_42:
			 	switch( CmdBuf[5] & 0x0F )
			 	{
		 		case MBUS42_DTYPE_B: /* Data type:Byte(8bit) */
				 	return( MBUS42_HEADERSIZE + MBUS42_ADRSIZE + (1 * ((CmdBuf[6]<<8)|CmdBuf[7])) + MBUS42_CRC_SIZE );
		 		case MBUS42_DTYPE_S: /* Data type:Byte(16bit) */
				 	return( MBUS42_HEADERSIZE + MBUS42_ADRSIZE + (2 * ((CmdBuf[6]<<8)|CmdBuf[7])) + MBUS42_CRC_SIZE );
		 		case MBUS42_DTYPE_L: /* Data type:Byte(32bit) */
				 	return( MBUS42_HEADERSIZE + MBUS42_ADRSIZE + (4 * ((CmdBuf[6]<<8)|CmdBuf[7])) + MBUS42_CRC_SIZE );
			 	default: return( -1 );
			 	}
			 case SFC_WR_RANREG_42:
			 	return( MBUS42_HEADERSIZE + ((MBUS42_ADRSIZE + 4) * ((CmdBuf[6]<<8)|CmdBuf[7])) + MBUS42_CRC_SIZE );
			 case SFC_RD_MAXNUM_42:
				 	return( MBUS42_HEADERSIZE + MBUS42_CRC_SIZE );
			 case SFC_RD_PRDCTID_42:
			 default: return( -1 );
			 }
		case MFC_SYSDWNLD:								/* @@@ Add Later @@@						*/
			 return( MbusFuncSysDLGetMsgLength(CmdBuf) );
		default:
			 if( KPI_CHK_OPEMSG( CmdBuf[1] ) )
			 {
				return( CmdBuf[2] + (CmdBuf[3]<<8) );
			 }
			 return( -1 );
		}
/*--------------------------------------------------------------------------------------------------*/
		/* unreachable return( -1 );  */				/* Unknown Function Code					*/
}
#endif	/* <S150> */

/***************************************** end of file **********************************************/

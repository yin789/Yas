/****************************************************************************************************/
/*																									*/
/*																									*/
/*		UsbMsgIf.c : USB function Server	 														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : USBManage message sending and receiving for communication.													*/
/*																									*/
/*			1) Receive a USB communication command message.												*/
/*			2) Receipt completion notification is performed in USB communication command message reception confirmation.							*/
/*			3) Start sending the response in response to the USB communication response message sending start request.						*/
/*			4) The transmission/reception monitoring sequence controls the reception and transmission of serial communication data.				*/
/*																									*/
/*																									*/
/*	Supplementary Foot : (Jupiter)UsbMsgIf.c + (Ingram)UsbFunctionIF.c => UsbMsgIf								*/
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
/*		Rev.1.00 : 2005.10.17  K.Ando																*/
/*		Rev.1.10 : 2008.05.21  T.Taniguchi															*/
/*		Rev.1.20 : 2013.07.04  K.Ozaki																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/* Author        : Yaskawa Electric Corp.,															*/
/* Project       : Mercury																			*/
/* Functions	 :																					*/
/*																									*/
/**** APIs ******************************************************************************************/
/* 																									*/
/**** Locals ****************************************************************************************/
/*																									*/
/****************************************************************************************************/
#include "KnlApi.h"											/* Kernel API */
//<S146>#include "arm.h"
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
#include "UsbFunctionIF.h"
#endif	/*_MERCURY_PLATFORM_*/
#include "usb_f.h"

#ifdef	_MERCURY_PLATFORM_			/*<S190>*/
#ifndef NULL
	#define NULL 0
#endif
const UCHAR	MsgReadSvSerial[] = {0x01,0x40,0x00,0x03,0xc8,0x08,0x00,0x08,0x3a,0x67};
#define	LEN_MSGREADSVSERIAL		sizeof(MsgReadSvSerial)
const UCHAR	ResReadSvSerial[] = {0x01,0x40,0x00,0x03,0x00,0x08,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x7f,0xfa};
#define	LEN_RESREADSVSERIAL		sizeof(ResReadSvSerial)
const UCHAR	MsgReadMaxRegSizeA[] = {0x01,0x40,0x00,0x41,0x00,0x03,0x10,0x10};
#define	LEN_MSGREADMAXREGSIZEA	sizeof(MsgReadMaxRegSizeA)
const UCHAR	ResReadMaxRegSizeA[] = {0x01,0x40,0x00,0x41,0x00,0x03,0x00,0x1b,0x4c,0x07};
#define	LEN_RESREADMAXREGSIZEA	sizeof(ResReadMaxRegSizeA)
const UCHAR	MsgReadMaxRegSizeB[] = {0x01,0x40,0x00,0x41,0x00,0x0d,0x91,0xd4};
#define	LEN_MSGREADMAXREGSIZEB	sizeof(MsgReadMaxRegSizeB)
const UCHAR	ResReadMaxRegSizeB[] = {0x01,0x40,0x00,0x41,0x00,0x0d,0x00,0x1b,0x2d,0xc4};
#define	LEN_RESREADMAXREGSIZEB	sizeof(ResReadMaxRegSizeB)
const UCHAR	MsgReadMaxRegSizeC[] = {0x01,0x40,0x00,0x41,0x00,0x10,0x51,0xdd};
#define	LEN_MSGREADMAXREGSIZEC	sizeof(MsgReadMaxRegSizeC)
const UCHAR	ResReadMaxRegSizeC[] = {0x01,0x40,0x00,0x41,0x00,0x10,0x00,0x15,0x3c,0x06};
#define	LEN_RESREADMAXREGSIZEC	sizeof(ResReadMaxRegSizeC)
const UCHAR	MsgReadMaxRegSizeD[] = {0x01,0x40,0x00,0x41,0x00,0x0e,0xd1,0xd5};
#define	LEN_MSGREADMAXREGSIZED	sizeof(MsgReadMaxRegSizeD)
const UCHAR	ResReadMaxRegSizeD[] = {0x01,0x40,0x00,0x41,0x00,0x0e,0x00,0x17,0xdd,0xc1};
#define	LEN_RESREADMAXREGSIZED	sizeof(ResReadMaxRegSizeD)
#endif	/*_MERCURY_PLATFORM_*/

/****************************************************************************************************/
/*																									*/
/*		Function Prototypes																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Kpx Functions																				*/
/*--------------------------------------------------------------------------------------------------*/
void	KpxInitUsbMsgIf(						/* USB communication message IF initialization					*/
		LONG	DriveID,						/* Drive model ID									*/
		LONG	ComAddr,						/* Communication address										*/
		LONG	AxisNumber						/* Number of multi-axis drive axes(0/1/2/...)						*/
		);
/*--------------------------------------------------------------------------------------------------*/
void	KpxSetUsbComAddr(						/* USB communication address setting process						*/
		LONG	ComAddr,						/* Communication address										*/
		LONG	AxisNumber	);					/* Number of multi-axis drive axes(0/1/2/...)						*/
/*--------------------------------------------------------------------------------------------------*/
//<S150>Declaration moved void	KpxUsbMsgManager( void );				/* USB communication message transmission/reception monitoring					*/
/*--------------------------------------------------------------------------------------------------*/
/*		Kpi Functions																				*/
/*--------------------------------------------------------------------------------------------------*/
void	KriInitUsbHmsg(							/* USB communicationHmsgInitialization process							*/
		HMSGIF	Hmsg		);					/* Message IF handle(Pointer)					*/
/*--------------------------------------------------------------------------------------------------*/
void	KriUsbMsgManager( CSIF *this );			/* USB communication message transmission/reception monitoring					*/
/*--------------------------------------------------------------------------------------------------*/
LONG	KriCheckRxUsbMsg(						/* USB communication command message reception check				*/
		struct _CSIF *this,						/* this pointer										*/
		USHORT* pCmdMsgLen	);					/* Command message length storage destination pointer					*/
/*--------------------------------------------------------------------------------------------------*/
LONG	KriStartTxUsbMsg(						/* USB communication response message transmission start command				*/
		struct _CSIF *this,						/* this pointer										*/
		UCHAR*	TxBuf,							/* Send data buffer								*/
		LONG	TxSize		);					/* Send data size(ByteSize,Zero if no response)		*/
/*--------------------------------------------------------------------------------------------------*/
/*		Local Functions																				*/
/*--------------------------------------------------------------------------------------------------*/
UCHAR	LpxUsbChkRxMessage( CSIF *this );		/* Received message check							*/

/****************************************************************************************************/
/*																									*/
/*		USB hardware IF function list																*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Initialization process																					*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*		Scan processing/interrupt processing																	*/
/*--------------------------------------------------------------------------------------------------*/
//void	DpiUsbConnectSequence( void );			/* USB connection-disconnection processing sequence					*/
/*--------------------------------------------------------------------------------------------------*/


/****************************************************************************************************/
/*																									*/
/*		Variable Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Local Parameter Structure Definition														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct {							/*														*/
	USHORT	DriveID;						/* Drive model ID										*/
	UCHAR	ComAddr;						/* Communication address(Own stationAddr)								*/
	UCHAR	AxisNum;						/* Number of multi-axis drive axes										*/
	UCHAR	AxisAddrBgn;					/* Multi-axis drive first axis address							*/
	UCHAR	AxisAddrEnd;					/* Multi-axis drive last axis address							*/
} MUSBP;
/*--------------------------------------------------------------------------------------------------*/
/*		Local Variables Structure Definition														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct {							/*														*/
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
	UCHAR	RxBuf[MSG_MAXBUFSZB];			/* Receive data buffer									*/
	UCHAR	TxBuf[MSG_MAXBUFSZB];			/* Send data buffer									*/
#else	/*_MERCURY_PLATFORM_*/
	UCHAR	RxBuf[512];			/* Receive data buffer									*/
	UCHAR	TxBuf[512];			/* Send data buffer									*/
#endif	/*_MERCURY_PLATFORM_*/
} MUSBV;
/*--------------------------------------------------------------------------------------------------*/
#define	REQSTX_NONE			0x00			/* No command												*/
#define	REQSTX_STARTTX		0x01			/* Response transmission start command										*/
#define	REQSTX_NORESPONSE	0x02			/* No response command(MsgErr)									*/
/*--------------------------------------------------------------------------------------------------*/
MUSBP	MusbP;								/* Local Parameter for USB Message						*/
MUSBV	MusbV;								/* Local Variables for USB Message						*/
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
CSIF		USif[USBIF_N_CH];				/* USB Interface Instance								*/
#else	/*_MERCURY_PLATFORM_*/
CSIF		USif[1];				/* USB Interface Instance								*/
#endif	/*_MERCURY_PLATFORM_*/

/****************************************************************************************************/
/*																									*/
/*		Macro Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Error Code Definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	USBERR_RXTMOVER		0x01				/* Reception time over									*/
#define	USBERR_USERERR		0x10				/* User error(No response)								*/
#define	USBERR_OTHERERR		0xFF				/* Other errors										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Set Sif Error Macro																			*/
/*--------------------------------------------------------------------------------------------------*/
#define SET_USBERR( code )	{ this->ErrCod = code; ++(this->ErrCnt);}
/*--------------------------------------------------------------------------------------------------*/
/*		Message Buffer Size Definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define USB_CMDBUF_SIZE		(sizeof( MusbV.RxBuf ) - 2)		/* Command  Message Buffer Size			*/
#define USB_RESBUF_SIZE		(sizeof( MusbV.TxBuf ) - 2)		/* Response Message Buffer Size			*/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		USB communication message IF initialization																*/
/*																									*/
/****************************************************************************************************/
void	KpxInitUsbMsgIf( LONG DriveID, LONG ComAddr, LONG AxisNumber
				)
{
/*--------------------------------------------------------------------------------------------------*/
/*		USB communication IF initialization processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		MlibResetByteMemory( &MusbV, sizeof(MusbV)/4 );		/* USBMessage IF variable initialization		*/
/*--------------------------------------------------------------------------------------------------*/
/*		USB communication address setting process																	*/
/*--------------------------------------------------------------------------------------------------*/
		MusbP.DriveID = (USHORT)DriveID;					/* Set DriveID							*/
		KpxSetUsbComAddr( ComAddr, AxisNumber );			/* Set ComAddr, AxisNumber				*/
/*--------------------------------------------------------------------------------------------------*/
/*		USB hardware IF initialization processing																	*/
/*--------------------------------------------------------------------------------------------------*/
		UsbFuncInitializeDriver( ComAddr );					/* USBHard IF initialization					*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
	{
CSIF *pSif = &(USif[0]);
		MlibResetLongMemory( pSif, sizeof(CSIF)/4 );		/* USBMessage IF variable initialization		*/
/*--------------------------------------------------------------------------------------------------*/
		pSif->Com = NULL;									/* 										*/
		pSif->AxisNumber = AxisNumber;						/* set axis number						*/
		pSif->DoRxTx = KriUsbMsgManager;					/* RxTx sequence						*/
		pSif->ChkCmd = KriCheckRxUsbMsg;					/* check receive cmd method				*/
		pSif->StxRes = KriStartTxUsbMsg;					/* start tx response method				*/
/*--------------------------------------------------------------------------------------------------*/
		pSif->RcvBufSize = USB_CMDBUF_SIZE;					/* rcv buf size */
		pSif->SndBufSize = USB_RESBUF_SIZE;					/* send buf size */
/*--------------------------------------------------------------------------------------------------*/
		pSif->RxBuf = MusbV.RxBuf;							/* set receive buffer					*/
		pSif->TxBuf = MusbV.TxBuf;							/* set send buffer						*/
	}
		return;
}


/****************************************************************************************************/
/*																									*/
/*		USB communication address setting process																	*/
/*																									*/
/****************************************************************************************************/
void	KpxSetUsbComAddr( LONG ComAddr, LONG AxisNumber )
{
/*--------------------------------------------------------------------------------------------------*/
/*		USB communication IF parameter initialization																*/
/*--------------------------------------------------------------------------------------------------*/
		MusbP.ComAddr = (UCHAR)ComAddr;						/* Com Address							*/
		MusbP.AxisNum = (UCHAR)AxisNumber;					/* Multi-Axis Number					*/
/*--------------------------------------------------------------------------------------------------*/
		if( AxisNumber < 0 )								/* Disable USB							*/
		{
			MusbP.AxisAddrBgn = 0xFF;						/* Axis Addr Begin						*/
			MusbP.AxisAddrEnd = 0x00;						/* Axis Addr End						*/
		}
		else if( AxisNumber <= 0 )							/* For SGDV								*/
		{
			MusbP.AxisAddrBgn = (UCHAR)ComAddr;				/* Axis Addr Begin						*/
			MusbP.AxisAddrEnd = (UCHAR)ComAddr;				/* Axis Addr End						*/
		}
		else												/* For Jupiter							*/
		{
			MusbP.AxisAddrBgn = 1;							/* Axis Addr Begin						*/
			MusbP.AxisAddrEnd = (UCHAR)(AxisNumber);		/* Axis Addr End						*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}


/****************************************************************************************************/
/*																									*/
/*		USB communicationHmsgInitialization process																	*/
/*																									*/
/****************************************************************************************************/
void	KriInitUsbHmsg( HMSGIF Hmsg )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Init Hmsg Data																				*/
/*--------------------------------------------------------------------------------------------------*/
//		Hmsg->CmdBuf = MusbV.RxBuf;
//		Hmsg->ResBuf = MusbV.TxBuf;
//		Hmsg->CmdBufSize = USB_CMDBUF_SIZE;
//		Hmsg->ResBufSize = USB_RESBUF_SIZE;
//		Hmsg->ChkCmdMsg  = KriCheckRxUsbMsg;
//		Hmsg->StxResMsg  = KriStartTxUsbMsg;
/*--------------------------------------------------------------------------------------------------*/
		return;
}


/****************************************************************************************************/
/*																									*/
/*		USB communication message fixed periodic monitoring process ( ScanC Cycle )											*/
/*																									*/
/****************************************************************************************************/
void	KpxUsbMsgManager( void )
{
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
	if( USB0_PORT_USE == 1) { USif[COM0_USB].DoRxTx( &(USif[COM0_USB]) ); /* USB ch:0 */ }
#else	/*_MERCURY_PLATFORM_*/
	USif[0].DoRxTx( &(USif[0]) ); /* USB ch:0 */
#endif	/*_MERCURY_PLATFORM_*/
	return;
}

enum	SQSTEP	{
		SQSTEP_INITIAL		= 0,
		SQSTEP_CHK_USB		,
		SQSTEP_START_RX		,
		SQSTEP_WAIT_SRX		,
		SQSTEP_WAIT_ERX		,
		SQSTEP_WAIT_STX		,
		SQSTEP_START_TX		,
		SQSTEP_WAIT_ETX		,
/*------------------------------------------*/
		SQSTEP_USBRXERR,
};
/*--------------------------------------------------------------------------------------------------*/
void	KriUsbMsgManager( CSIF *this )
{
	if( UsbFuncChkAvailable() == FALSE )
	{
		return;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Initialization completion check																			*/
/*--------------------------------------------------------------------------------------------------*/
//		if( MusbP.AxisAddrBgn > MusbP.AxisAddrEnd )
//		{
//			if( KsysV.MainTimer < 20000 ){ return;}			/* 20secAfter the elapsed time,DownloadCorrespondence			*/
//		}
/*--------------------------------------------------------------------------------------------------*/
/*		USB hardware IF processing																	*/
/*--------------------------------------------------------------------------------------------------*/
//		DpiUsbConnectSequence( );							/* USBBus connection-disconnection sequence			*/
//		UsbFuncIntIntu2f();									/* Plug-in/plug-out processing			*/
//		UsbFuncIntIntu2fepc();								/* Bus reset/change bus speed	*/
															/* EP0 SETUPDetection processing					*/

		UsbFuncPollBulkOutIn( );							/* USBDriver bulk transfer service		*/
/*--------------------------------------------------------------------------------------------------*/
/*		USB message transmission/reception monitoring sequence														*/
/*--------------------------------------------------------------------------------------------------*/
		switch( this->SqStep )
		{
	/*----------------------------------------------------------------------------------------------*/
		case SQSTEP_INITIAL:		/* Initial Step													*/
	/*----------------------------------------------------------------------------------------------*/
			 UsbFuncResetBuffer( );								/* Endpoint1/2 Transmission/reception buffer clear	*/
			 this->SqStep = SQSTEP_CHK_USB;
			 break;
	/*----------------------------------------------------------------------------------------------*/
		case SQSTEP_CHK_USB:		/* Check USB Port												*/
	/*----------------------------------------------------------------------------------------------*/
			 if( UsbFuncPortConfiguration() == TRUE )			/* USBCheck port status			*/
			 {
				this->SqStep = SQSTEP_START_RX;					/* Start receptionSTEP				*/
			 }
			 break;
	/*----------------------------------------------------------------------------------------------*/
		case SQSTEP_START_RX:		/* Start Receive Message										*/
	/*----------------------------------------------------------------------------------------------*/
			 if( UsbFuncPortConfiguration() == FALSE )			/* USBCheck port status			*/
			 {
				this->SqStep = SQSTEP_INITIAL;					/* Return to initial when invalid				*/
			 }
			 else if( UsbFuncStartBkout( this->RxBuf, USB_CMDBUF_SIZE ) == TRUE )	/* Start reception		*/
			 {
				this->SqStep = SQSTEP_WAIT_SRX;					/* Wait for reception startSTEP			*/
			 }
			 break;
	/*----------------------------------------------------------------------------------------------*/
		case SQSTEP_WAIT_SRX:		/* Wait Start of Receive Message								*/
	/*----------------------------------------------------------------------------------------------*/
			 if( UsbFuncPortConfiguration() == FALSE )			/* USBCheck port status			*/
			 {
				this->SqStep = SQSTEP_INITIAL;					/* Return to initial when invalid				*/
			 }
			 else if( UsbFuncChkRcvComp() == BLKTRNS_END )		/* All data reception complete?				*/
			 {
				this->SqStep = LpxUsbChkRxMessage( this );		/* Received message check			*/
			 }
			 else if( UsbFuncGetRcvSize() > 0 )					/* Start reception?						*/
			 {
				KpiRstLongTimer( &this->RxTimer );				/* Reception monitoring timer reset			*/
				this->SqStep = SQSTEP_WAIT_ERX;					/* Wait for reception completionSTEP			*/
			 }
			 break;
	/*----------------------------------------------------------------------------------------------*/
		case SQSTEP_WAIT_ERX:		/* Wait End of Receive Message									*/
	/*----------------------------------------------------------------------------------------------*/
			 if( UsbFuncPortConfiguration() == FALSE )			/* USBCheck port status			*/
			 {
				this->SqStep = SQSTEP_INITIAL;					/* Return to initial when invalid				*/
			 }
			 else if( UsbFuncChkRcvComp() == BLKTRNS_END )		/* All data reception complete?				*/
			 {
				this->SqStep = LpxUsbChkRxMessage( this );		/* Received message check			*/
			 }
			 else if( KpiGetLongTimer(&this->RxTimer) >= 500 )	/* Reception monitoring time over				*/
			 {
				this->SqStep = SQSTEP_INITIAL;					/* When an error occursINITIAL-STEP			*/
			 }
			 break;
	/*----------------------------------------------------------------------------------------------*/
		case SQSTEP_WAIT_STX:		/* Wait Start Transmit											*/
	/*----------------------------------------------------------------------------------------------*/
			 if( this->ReqStx == REQSTX_STARTTX )				/* Transmission start request check				*/
			 {
				this->ReqStx = 0x00;							/* Reset ReqStx						*/
				this->SqStep = SQSTEP_START_TX;					/* Transmission start processingSTEP			*/
			 }
			 else if( this->ReqStx == REQSTX_NORESPONSE )		/* No response(MsgErr)Request check		*/
			 {
				this->ReqStx = 0x00;							/* Reset ReqStx						*/
//				SET_USBERR( USBERR_USERERR );					/* Set User Error					*/
//				this->SqStep = SQSTEP_START_TX;					/* Transmission start processingSTEP			*/
				this->SqStep = SQSTEP_INITIAL;					/* INITIAL-STEP			*/
			 }
			 break;
	/*----------------------------------------------------------------------------------------------*/
		case SQSTEP_START_TX:		/* Start Transmit Message										*/
	/*----------------------------------------------------------------------------------------------*/
			 if( UsbFuncPortConfiguration() == FALSE )			/* USBCheck port status			*/
			 {
				this->SqStep = SQSTEP_INITIAL;					/* Return to initial when invalid				*/
			 }
			 else if( UsbFuncStartBkin( this->TxBuf, this->TxResLen ) == TRUE )		/* Transmission start		*/
			 {
				this->SqStep = SQSTEP_WAIT_ETX;					/* Waiting for transmission completionSTEP			*/
			 }
			 break;
	/*----------------------------------------------------------------------------------------------*/
		case SQSTEP_WAIT_ETX:		/* Wait End of Transmit Message									*/
	/*----------------------------------------------------------------------------------------------*/
			 if( UsbFuncPortConfiguration() == FALSE )			/* USBCheck port status			*/
			 {
				this->SqStep = SQSTEP_INITIAL;					/* Return to initial when invalid				*/
			 }
			 else if( UsbFuncChkTrnsComp() == BLKTRNS_END )		/* All data transmission completion inquiry			*/
			 {
				this->SqStep = SQSTEP_START_RX;					/* Start receptionSTEP				*/
			 }
			 break;
	/*----------------------------------------------------------------------------------------------*/
		default:					/* Not Used														*/
	/*----------------------------------------------------------------------------------------------*/
			 break;
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}


/****************************************************************************************************/
/*																									*/
/*		USB reception message check																*/
/*																									*/
/****************************************************************************************************/
UCHAR	LpxUsbChkRxMessage( CSIF *this )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Slave address check 																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( this->RxBuf[0] == 0x00 )
		{
			;											/* OK										*/
		}
		else if( this->RxBuf[0] == MusbP.ComAddr )
		{
			;											/* OK										*/
		}
		else if( this->RxBuf[0] < MusbP.AxisAddrBgn )
		{
			return( SQSTEP_INITIAL );					/* Start reception processingStep.					*/
		}
		else if( this->RxBuf[0] > MusbP.AxisAddrEnd )
		{
			return( SQSTEP_INITIAL );					/* Start reception processingStep.					*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Function code check 																*/
/*--------------------------------------------------------------------------------------------------*/
		if( 1 )											/* Normal										*/
		{
			this->RxCmdLen = UsbFuncGetRcvSize( );		/* Get number of reception completion bytes					*/
			return(	SQSTEP_WAIT_STX );					/* Waiting for transmission startStep					*/
		}
/*--------------------------------------------------------------------------------------------------*/
//		return( SQSTEP_INITIAL );						/* Other than the above, all defaultsStep			*/
}


/****************************************************************************************************/
/*																									*/
/*		USB communication command message reception check														*/
/*																									*/
/****************************************************************************************************/
LONG	KriCheckRxUsbMsg( 
		struct _CSIF *this,						/* this pointer										*/
		USHORT* pCmdMsgLen )					/* pointer to the message length					*/
{
	if( UsbFuncChkAvailable() == FALSE )
	{
		return( FALSE );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		When reception is completed																					*/
/*--------------------------------------------------------------------------------------------------*/
		if( this->SqStep == SQSTEP_WAIT_STX )
		{
			*pCmdMsgLen = this->RxCmdLen;
			return( TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Receiving incomplete																					*/
/*--------------------------------------------------------------------------------------------------*/
	return( FALSE );
}

/****************************************************************************************************/
/*																									*/
/*		USB communication response message transmission start command														*/
/*																									*/
/****************************************************************************************************/
LONG	KriStartTxUsbMsg(							/* Response message transmission start command processing				*/
		struct _CSIF *this,						/* this pointer										*/
		UCHAR*	TxBuf,								/* Send data buffer							*/
		LONG	TxSize )							/* Send data size(ByteSize,Zero if no response)	*/
{
	if( UsbFuncChkAvailable() == FALSE )
	{
		return( FALSE );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Response message type check																*/
/*--------------------------------------------------------------------------------------------------*/
		if( this->SqStep != SQSTEP_WAIT_STX )		/* Sequence check							*/
		{
			return( FALSE );						/* Abnormal return									*/
		}
		else if( TxSize > 0 )						/* Send data size check						*/
		{
			this->ReqStx   = REQSTX_STARTTX;		/* Transmission start request setting						*/
			this->pTxRes   = TxBuf;					/* Response buffer address setting						*/
			this->TxResLen = (USHORT)TxSize;		/* Response message length(ByteNumber)Setting					*/
			return( TRUE );
		}
		else
		{
			this->ReqStx   = REQSTX_NORESPONSE;		/* No response request setting							*/
			this->pTxRes   = NULL;					/* Response buffer address setting						*/
			this->TxResLen = 0;						/* Response message length(ByteNumber)Setting					*/
			return( TRUE );
		}
/*--------------------------------------------------------------------------------------------------*/
}



/****************************************************************************************************/
/*																									*/
/*																									*/
/*		USB hardware IF processing																	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
// Separate file










/**************************** Start of Local Functions **********************************************/

/***************************************** end of file **********************************************/

/****************************************************************************
Description	: USB function driver
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2013 All Rights Reserved

Project       : Mercury

Functions	  : none

----------------------------------------------------------------------------
Changes		:
Name		Date			Description
------------------------------------------------------------------------
K.Sakamoto	2013.05.12		created

****************************************************************************/
#ifndef USB_F_H__
#define USB_F_H__

/*===========================================================================*/
/* I N C L U D E                                                             */
/*===========================================================================*/
#include "Basedef.h"
//<S150>#include "errcodes.h"	/* Encrypted format support <S146> */
#include "u2f_desc.h"
//<S146>#include "errcodes.h"

/*===========================================================================*/
/* D E F I N E                                                               */
/*===========================================================================*/
#define U2F_PLUG_OUT    0
#define U2F_PLUG_IN     1

/* <S150> */
/* define of error code */
#define ER_OK      ((ER_RET)0)				/**< Error code for Normal end (no error) */
#define ER_NG      ((ER_RET)-1)				/**< Error code for Abnormal end (error) */
#define ER_SYS     ((ER_RET)(2 * ER_NG))	/**< Error code for Undefined error */
#define ER_PARAM   ((ER_RET)(3 * ER_NG))	/**< Error code for Invalid parameter */
#define ER_NOTYET  ((ER_RET)(4 * ER_NG))	/**< Error code for Incomplete processing */
#define ER_NOMEM   ((ER_RET)(5 * ER_NG))	/**< Error code for Out of memory */
#define ER_BUSY    ((ER_RET)(6 * ER_NG))	/**< Error code for Busy */
#define ER_INVAL   ((ER_RET)(7 * ER_NG))	/**< Error code for Invalid state */
#define ER_TIMEOUT ((ER_RET)(8 * ER_NG))	/**< Error code for Timeout occurs */

/*==========================================================================*/
/* E X T E R N   V A R I A B L E                                            */
/*==========================================================================*/

/*===========================================================================*/
/* P R O T O T Y P E                                                         */
/*===========================================================================*/
/* UsbFuncDriver.c */
/* --- Initialize --- */
void	UsbFuncInitializeDriver( ULONG MyAddress );	/* USB Function Driver initialization					*/
BOOL	UsbFuncChkAvailable( void );				/* USBUsable judgment processing							*//*<S138>*/
/* --- Interrupt --- */
void    UsbFuncIntIntu2f ( void );					/* INTU2F Interrupt processing							*/
void    UsbFuncIntIntu2fepc ( void );				/* INTU2FEPC Interrupt processing						*/
/* --- Control transfer --- */
void	UsbFuncTaskControl( void );					/* Control transfer task processing					*//*<S1A2>*/
/* --- Bulk transfer task --- */
void	UsbFuncPollBulkOutIn( void );				/* Bulk transfer processing main(ScanB)		*/

ULONG	UsbFuncStartBkout( UCHAR *Buffer ,USHORT );	/* Bulk transfer(OUT)Start request function					*/
ULONG	UsbFuncStartBkin( UCHAR *Buffer ,USHORT );	/* Bulk transfer(IN) Start request function					*/
USHORT	UsbFuncGetRcvSize( void );					/* Get number of received bytes when reception is completed					*/
ULONG	UsbFuncChkRcvComp( void );					/* Bulk transfer(OUT)Status check					*/
ULONG	UsbFuncChkTrnsComp( void );					/* Bulk transfer(IN)Status check					*/
void	UsbFuncResetBuffer( void );					/* Bulk transfer buffer clearing process				*/

ULONG	UsbFuncPortConfiguration( void );			/* <S150> */

/*--------------------------------------------------------------------------------------------------*/
/*	Endpoint1or2State definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	BLKTRNS_DISABLE			0x00		/* Bulk transfer disabled										*/
#define	BLKTRNS_ENABLE			0x01		/* Bulk transfer enabled										*/
#define	BLKTRNS_ACTIVE			0x02		/* Bulk transfer in progress										*/
#define	BLKTRNS_END				0x03		/* Bulk transfer completed										*/
#define	BLKTRNS_STALL			0x10		/* An error occurred during bulk transferSTALLdid					*/

/****************************************************************************************************/
/*																									*/
/*		INTUSB0B															*/
/*																									*/
/****************************************************************************************************/
#define	REQ_ANA					0		/* CPUDECData request analysis processing using interrupts				*/
#define	EP0_INTAKEN				1		/* EP0ININ token processing using interrupts						*/
#define	EP0_INTAKEN_CMP			2		/* EP0INDTIN token completion processing using interrupts					*/

#endif // USB_F_H__
/*-------end of file----------------------------------------------------------------------------------------*/

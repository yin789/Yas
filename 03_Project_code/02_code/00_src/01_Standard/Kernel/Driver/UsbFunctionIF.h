/****************************************************************************
Description	: USB function Server for MEMBUS protocol
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2013 All Rights Reserved

Project       : Mercury

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
K.S.   2013.05.10     created

****************************************************************************/
#ifndef __USB_FUNTION_IF_H 
#define __USB_FUNTION_IF_H

#include "MemobusIF.h"

/* definition of USB function process step */
typedef enum USB_TRANS_STEP_ {
	USB_STATE_INITIAL1 = 0,			/* Initial step1 */
	USB_STATE_INITIAL2,				/* Initial step2 */
	USB_STATE_WAIT_OUT_END,			/* MEMOBUSData reception completion waiting step */
	USB_STATE_WAIT_START_IN1,		/* Transmission start waiting step */
	USB_STATE_WAIT_START_IN2,		/* RoundWaiting step to start sending from */
	USB_STATE_WAIT_IN_END,			/* Waiting for transmission completion step */
} USB_TRANS_STEP;

/* Structure of USB function I/F handle */
typedef struct USB_MBUS_SERVER_ {
	MEMOBUS_IF		MbusIf;			/* MEMOBUS function Interface */
	struct {
		UINT	BlkInStart	:1;		/* Transmission start request */
		UINT	BlkInSkip	:1;		/* Receive without transmitting(If no response) */
	}f;
	USB_TRANS_STEP	State;			/* Communication state */
} USB_MBUS_SERVER;


#ifdef __cplusplus
extern	"C"	{
#endif

/* API's */
INT32 MbusUsb_Initialize(USB_MBUS_SERVER *UsbMbusServer,
						 MEMOBUS_ADDR_MAP *AxisCfg, ULONG MyAddress);

void MbusUsb_Server(USB_MBUS_SERVER *UsbMbusServer);

void MbusUsb_PollOutIn(USB_MBUS_SERVER *UsbMbusServer);

#ifdef __cplusplus
}
#endif



#endif /*__USB_FUNTION_IF_H */


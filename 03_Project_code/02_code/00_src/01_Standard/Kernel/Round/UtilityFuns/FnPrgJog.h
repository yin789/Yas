#ifndef _FN_PRG_JOG_H
#define _FN_PRG_JOG_H
/****************************************************************************
Description	: Fn Program JOG
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
T.Kira  2011.02.18    created

*****************************************************************************/
#include "Basedef.h"
#include "SystemSetting.h"
#include "BaseCmnStruct.h"

/****************************************************************************************/
/*																						*/
/*		ProgramJOGOperation Fn												*/
/*																						*/
/****************************************************************************************/
/*--------------------------------------------------------------------------------------*/
/*		ProgramJOGOperation status definition													*/
/*--------------------------------------------------------------------------------------*/
#define	PJOG_INIT	0x00				/* Initial state								*/
#define	PJOG_ABORT	0x10				/* Interrupted state										*/
#define	PJOG_START	0x01				/* Operation status										*/
#define	PJOG_AFTER	0x11				/* Post-operation processing status								*/
#define	PJOG_END	0x02				/* Termination status										*/
#define	PJOG_ERROR	0x03				/* Abnormal termination										*/
/*--------------------------------------------------------------------------------------*/
/*		ProgramJOGOperation command definition													*/
/*--------------------------------------------------------------------------------------*/
#define	PJOGCMD_NONE	0x00			/* No command									*/
#define	PJOGCMD_INIT	0x01			/* Initialize								*/
#define	PJOGCMD_ABORT	0x02			/* interruption											*/
#define	PJOGCMD_START	0x03			/* Operation											*/

#define		LMTLOWSPEED	10
/*--------------------------------------------------------------------------------------*/
/*		ProgramJOGOperation-related function definition													*/
/*--------------------------------------------------------------------------------------*/
void PcalPjogSw( void *Axis );			/* Pn530 : ProgramJOGOperation-related switches		*/
void PcalPjogDistance( void *Axis );	/* Pn531,532 : ProgramJOGTravel distance[Command unit]	*/
void PcalPjogRotspd( void *Axis );		/* Pn533 : ProgramJOGMovement speed[min-1]			*/
void PcalPjogAcctime( void *Axis );		/* Pn534 : ProgramJOGAcceleration/deceleration time[ms]			*/
void PcalPjogWaitTime( void *Axis );	/* Pn535 : ProgramJOGWaiting time[ms]			*/
void PcalPjogNum( void *Axis );			/* Pn536 : ProgramJOGNumber of movements				*/


#endif //_FN_PRG_JOG_H 
/***************************************** end of file **********************************/

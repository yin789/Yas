/****************************************************************************
Description	: Trigger I/O functions
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2012 All Rights Reserved

Project       : SGDZ-BS63/80

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
H.N.   2012.03.05     created

****************************************************************************/
#ifndef	_TRIGIO_DEF_H
#define	_TRIGIO_DEF_H

#include "Basedef.h"

/* define */
#define TRIG_IO_MAX_TBL_NUMBER	8		/* TriggerIONumber of tables */
#define TRIG_IO_LSTPOS_MAX		32		/* PreviousFBNumber of position storage buffers */

/* struct */
typedef struct TRIGIO_INFO_T {
	LONG	trigPosition;	/* Trigger position[pulse] */
	ULONG	manualDelay;	/* Manual delay time[us] */
	SHORT	onAdjTime;		/* Output adjustment time[us] */ 
	USHORT	onTime;			/* IOOutput time[us] */
	USHORT	trigConfig;		/* TriggerIOTable, axis number, output pin, passing direction */
	USHORT	axisNo;			/* Axis number */
	USHORT	pinNo;			/* Output pin number */
	SHORT	passDir;		/* Passing direction [1:Positive direction,-1Reverse direction] */
	
	BOOL	trigIOEnable;	/* TriggerIOEnabled [0:Disabled,1:Enabled] */
	BOOL	trigIOMode;		/* Mode [0:Normal mode,1:Manual output] */
	BOOL	posReady;		/* Position ready state TRUE:Complete,FALSE:Incomplete */
	
	LONG	lstPos[TRIG_IO_LSTPOS_MAX];		/* MotAbsPos */
	USHORT	mSpdCycle;		/* Motor speed calculation cycle[62.5us] */
	USHORT	fbPosBufPtr;	/* PreviousFBPosition storage buffer */
}TRIGIO_INFO;

#endif	//	_TRIGIO_DEF_H

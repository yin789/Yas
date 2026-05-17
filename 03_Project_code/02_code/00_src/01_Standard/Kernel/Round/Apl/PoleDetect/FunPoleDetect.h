/*****************************************************************************
Description		: Magnetic Pole Adjust
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
SL.Xue		2013.06.25		created

*****************************************************************************/

#ifndef FUN_POLEDETECT_H_
#define FUN_POLEDETECT_H_

#include	"KnlApi.h"		/* Kernel API */
#include	"LcdOp.h"		/* LCD Operator */
#include 	"Function.h"	/* Fn Function */
#include	"Operation.h"	/* Servo Operation */
#include    "Parameter.h"	/* Parameter Definition Header File */
#include	"Monitor.h"		/* LCD operator monitor define head file */
#include	"PnPrmListTbl.h"


enum _enumFunPoleDetectState
{
	POLEADJ_INIT 		= 0L,		/* Initialization process */
	POLEADJ_GAINCHANGE	= 1L,		/* Magnetic pole detection speed loop gain change.EEPROMWrite process */
	POLEADJ_EXE1		= 2L,		/* Magnetic pole detection start processing */
	POLEADJ_EXE2		= 3L,		/* Magnetic pole detection execution processing */
	POLEADJ_EXEAFTER	= 4L,		/* Current phase information write request creation/write result monitoring process */
	POLEADJ_FNERR		= 5L,		/* Auxiliary function abnormal termination processing: Waiting for termination operation */
	POLEADJ_END			= 6L		/* Auxiliary function termination processing */
};

/******************************************************************************/
/*		STRUCT DEFINITION													  */
/******************************************************************************/
typedef struct _FUNPOLEDETECTV {
	struct{
		enum _enumFunPoleDetectState State;		/* PoleDetect (Fn080) Execution State */
		LONG OpeModeIdx;						/* Operation Mode Register Index */
		LONG OpeSetIdx;							/* Operation Set Register Index */
		ULONG DispTime;							/* Display Time (Panel Operator) */
		ULONG DPos;								/* Number of blinking digits */
		LONG OpePdetStateIdx;					/* Operation read Register Index 0x032 */
		ULONG ResPdetStateData;					/* Respone 0x032: Magnetic pole detection complete Date */
		LONG PrmPdetloophzIdx;					/* Prm read Register Index 0x481 */
		ULONG ResPdetloophzData;				/* Respone 0x481: Magnetic pole detection speed loop gain  Date */
		ULONG PdetKvVal;						/* Magnetic pole detection speed loop gain(Pn481):[0.1Hz] */
		BOOL PdetReq;							/* Magnetic pole detection request(Request energization) */
		LONG UnReadIdx_AlmCode;					/* Operation Set Register Index 0x50A */
		ULONG ResData_AlmCode;					/* Respone 0x50A Date */
		LONG Reserved[(256/4)-3-7]; 			/* Reserved: All functions can use ram variables at least 256 bytes. */
	}var;
} FUNPOLEDETECTV;

/***************************************** end of file **********************************************/
#endif /* FUN_POLEDETECT_H_ */

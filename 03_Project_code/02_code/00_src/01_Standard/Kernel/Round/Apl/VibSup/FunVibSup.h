/*****************************************************************************
Description		: Model Following Control with Vibration Suppression Tuning
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
SL.Xue		2013.07.01		created

*****************************************************************************/

#ifndef FUN_VIBSUP_H_
#define FUN_VIBSUP_H_

#include	"KnlApi.h"		/* Kernel API */
#include	"LcdOp.h"		/* LCD Operator */
#include 	"Function.h"	/* Fn Function */
#include	"Operation.h"	/* Servo Operation */
#include	"Monitor.h"		/* LCD operator monitor define head file */
#include    "Parameter.h"	/* Parameter Definition Header File */
#include	"ModelControl.h"/* Direct torque reference method model following control related processing definition */


enum _enumFnVibSupState
{
	VIBSUP_INIT			= 0L,			/* Initial processing */
	VIBSUP_ADJUST		= 1L,			/* Fine adjustment of vibration frequency */
	VIBSUP_SETREQ		= 2L,			/* Frequency(Model following control with vibration suppression)Setting */
	VIBSUP_WAITMFC		= 3L,			/* Waiting for frequency setting to be valid(Model following control is already enabled) */
	VIBSUP_WAITNORM		= 4L,			/* Waiting for model following control with vibration suppression setting to take effect */
	VIBSUP_SETCOMP		= 5L,			/* Setting completed */
	VIBSUP_END			= 6L			/* Termination processing */
};

/******************************************************************************/
/*		STRUCT DEFINITION													  */
/******************************************************************************/
typedef struct _FNVIBSUPV {
	struct{
		enum _enumFnVibSupState State;			/* VibSup (Fn205) Execution State */
		LONG OpeModeIdx;						/* Operation Mode Register Index */
		LONG OpeSetIdx;							/* Operation Set Register Index */
		LONG PrmMdlantfrqIdx;					/* Prm read Register Index 0x145 */
		ULONG ResMdlantfrqData;					/* Respone 0x145: Vibration suppression 1 frequencyA Date */
		LONG PrmMdlresfrqIdx;					/* Prm read Register Index 0x146 */
		ULONG ResMdlresfrqData;					/* Respone 0x146: Vibration suppression 1 frequencyB Date */
		LONG OpeMdlVibFreqIdx;					/* Operation read Register Index 0x0B0 */
		ULONG ResMdlVibFreqData;				/* Respone 0x0B0: With vibration suppressionMFCAdjustment frequency Date */
		ULONG DetFreq;							/* Detection frequency */
		ULONG DPos;								/* Number of blinking digits */
		BOOL Match;								/* Does the display and operation match? */
		ULONG FfWaitCntr;						/* Waiting time elapsed counter */
		LONG UnRvibFrequencyIdx;				/* Un read Register Index 0x107 */
		ULONG ResRvibFrequencyData;				/* Respone 0x107: Residual vibration frequency Date */
		LONG Reserved[(256/4)-3-12];			/* Reserved: All functions can use ram variables at least 256 bytes. */
	}var;
} FNVIBSUPV;

/***************************************** end of file **********************************************/
#endif /* FUN_VIBSUP_H_ */

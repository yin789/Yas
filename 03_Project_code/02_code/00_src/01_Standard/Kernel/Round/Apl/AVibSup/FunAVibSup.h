/*****************************************************************************
Description		: A-type Vibration Suppression Control Tuning
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
SL.Xue		2013.06.26		created

*****************************************************************************/

#ifndef FUN_AVIBSUP_H_
#define FUN_AVIBSUP_H_

#include	"KnlApi.h"		/* Kernel API */
#include	"LcdOp.h"		/* LCD Operator */
#include 	"Function.h"	/* Fn Function */
#include	"Operation.h"	/* Servo Operation */
#include    "Parameter.h"	/* Parameter Definition Header File */
#include	"PnPrmListTbl.h"
#include	"ResVibCtrl.h"


enum _enumFnAVibSupState
{
	AVIBSUP_INIT		= 0L,		/* Initial processing */
	AVIBSUP_MODESEL		= 1L,		/* Automatic/Manual mode selection */
	AVIBSUP_TRGWAIT		= 2l,		/* Vibration detection trigger wait */
	AVIBSUP_MEASURE		= 3L,		/* Frequency analysis in progress */
	AVIBSUP_COMP		= 4L,		/* Frequency analysis completed(Waiting for filter settings) */
	AVIBSUP_SET			= 5L,		/* Filter settings in progress */
	AVIBSUP_FREQTUNE	= 6L,		/* Frequency adjustment */
	AVIBSUP_GAINTUNE	= 7L,		/* Damping gain adjustment */
	AVIBSUP_END			= 8L		/* Termination processing */
};

/******************************************************************************/
/*		STRUCT DEFINITION													  */
/******************************************************************************/
typedef struct _FNAVIBSUPV {
	struct{
		enum _enumFnAVibSupState State;			/* AVibSup (Fn204) Execution State */
		LONG OpeModeIdx;						/* Operation Mode Register Index */
		LONG OpeSetIdx;							/* Operation Set Register Index */
		ULONG LongTimer;						/* For measuring auxiliary function execution processing time */
		LONG PrmAvibdampIdx;					/* Prm read Register Index 0x163 */
		ULONG ResAvibdampData;					/* Respone 0x163: AMold vibration damping damping gain Date */
		LONG OpeAvibModeIdx;					/* Operation read Register Index 0x0A0 */
		ULONG ResAvibModeData;					/* Respone 0x0A0: AMold vibration damping function adjustment mode Date */
		LONG OpeAvibFreqIdx;					/* Operation read Register Index 0x0A2 */
		ULONG ResAvibFreqData;					/* Respone 0x0A2: AMold vibration damping function adjustment frequency Date */
		LONG PrmAvibfrqIdx;						/* Prm read Register Index 0x161 */
		ULONG ResAvibfrqData;					/* Respone 0x161: AMold vibration damping frequency Date */
		LONG OpeAvibGainIdx;					/* Operation read Register Index 0x0A3 */
		ULONG ResAvibGainData;					/* Respone 0x0A3: AMold vibration damping function adjustment gain Date */
		ULONG AresVibControl;					/* AMold vibration suppression control setting */
		BOOL Match;								/* Does the display and operation match? */
		ULONG DPos;								/* Number of blinking digits */
		LONG OpeAvibStateIdx;					/* Operation read Register Index 0x0A1 */
		ULONG ResAvibStateData;					/* Respone 0x0A1: AMold vibration damping function adjustment complete Date */
		LONG PrmVibnormIdx;						/* Prm read Register Index 0x312 */
		ULONG ResVibnormData;					/* Respone 0x312: Vibration detection level Date */
		LONG OpeVibmRf0FreqIdx;					/* Operation read Register Index 0x400 */
		ULONG ResVibmRf0FreqData;				/* Respone 0x400: Vibration monitor resonance frequency1 Data*/
		LONG Reserved[(256/4)-3-20]; 			/* Reserved: All functions can use ram variables at least 256 bytes. */
	}var;
} FNAVIBSUPV;

/***************************************** end of file **********************************************/
#endif /* FUN_AVIBSUP_H_ */

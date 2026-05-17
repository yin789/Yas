/*****************************************************************************
Description		: Fn201  Advanced Auto Tuning Mode Headfile
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
ZH.QIN		2013.06.17		created

*****************************************************************************/

#ifndef FUN_AAT_H_
#define FUN_AAT_H_


#include	"KnlApi.h"		/* Kernel API */
#include	"LcdOp.h"		/* LCD Operator */
#include 	"Function.h"	/* Fn Function */
#include	"Operation.h"	/* Servo Operation */
#include	"Parameter.h"	/* Parameter Definition Header File*/
#include	"Monitor.h"		/* Un Monitor *///<S197>

/*--------------------------------------------------------------------------------------------------*/
/*		 Advanced Auto Tuning  State																*/
/*--------------------------------------------------------------------------------------------------*/
enum _enumFnRefAATState
{
	AAT_SEQ_INIT = 0L,							/* Initial state							*/
	AAT_SEQ_MODESET = 1L,						/* Tuning mode setting status		*/
	AAT_SEQ_JRATSET = 2L,						/* Inertia setting state				*/
	AAT_TUNNING_WAIT = 3L,						/* Tuning wait completion state			*/
	AAT_SEQ_ENDDISP = 4L,						/* Termination display status						*/
	AAT_SEQ_END = 5L,							/* Final setting state						*/
	AAT_SEQ_WAIT = 6L,							/* Waiting for termination state						*/
	AAT_SEQ_RETURN = 7L, 						/* Post-processing status						*/
};

/******************************************************************************/
/*		STRUCT DEFINITION													  */
/******************************************************************************/

typedef struct _FnAATV {
	struct {
		enum _enumFnRefAATState State;	/* Advanced Auto Tuning (Fn201) Execution State */

		LONG OpeModeIdx;			/* Operation Mode Register Index 				*/
		LONG OpeSetIdx;				/* Operation Set Register Index 				*/
		LONG TimeCntrIdex;			/*Safety stop time[s] Register Index				*/
		LONG AdatModeIdx;			/* 0x2090 : Advanced autotuning related mode setting Register Index	*/
		LONG AdatLevelIdx;			/* 0x2091 : Advanced autotuning mode setting Register Index	*/
		LONG AdatMovepIdx;			/* 0x2092 : AdvancedATTravel distance	Register Index	*/
		LONG AdatStateIdx;			/* 0x2094 : AdvancedATConfirm completion	Register Index	*/
		LONG AdatFilTypeIdx;		/* 0x2095 : AdvancedATFilter type	Register Index	*/

		BOOL AdatMode;				/* 0x2090 : AdvancedATMode						*/
		ULONG AdatLevel;			/* 0x2091 : AdvancedATLevel					*/
		LONG AdatMovep;				/* 0x2092 : AdvancedATTravel distance					*/
		LONG AdatState;				/* 0x2094 : AdvancedATConfirm completion					*/
		ULONG AdatFilType;			/* 0x2095 : AdvancedATFilter type					*/

		LONG LoophzIdex;			/* Pn100 : Speed loop gain		Register Index			*/
		LONG PitimeIdex;			/* Pn101 : Speed loop integral time constant	Register Index			*/
		LONG PosgnIdex;				/* Pn102 : Position loop gain		Register Index			*/
		LONG JrateIdex;				/* Pn103 : Moment of inertia ratio		Register Index			*/
		LONG MdlgnIdex;				/* Pn141 : Model following control gain	Register Index			*/
//		LONG Syssw1Idex;			/* PnE0B : System switch 1	Register Index			*///<S197>

		LONG IntModeStateIdex;		/* 0xE705 : Internal operating status						*///<S197

		LONG ModeSetPtr;			/* Mode setting: Sequence pointer				*/
		LONG DPos;					/* Number of blinking digits										*/
		LONG JratSetPtr;			/* Moment of inertia identification: Sequence pointer		*/
//		LONG JratErr;				/* Moment of inertia identification: Error state				*//*TODO: Delete */
		ULONG JratTimeCntr;			/* Safety stop time counter							*/
//		BOOL ServonFlag;			/* Servo on flag								*//*TODO: Delete */
		ULONG SafeTime;				/* Safe time									*/
		ULONG DispGain;				/* Disp Gain									*/
		LONG AmpType;				/* Amp   type									*/

		BOOL	DrvErrDetect;
		LONG	SafeStopCmd;														  /* <S195> */

		LONG Reserved[(256/4)-1-8-5-6-8]; 	/* Reserved: All functions can use ram variables at least 256 bytes. */
	}var;
} FNAATV;

/***************************************** end of file **********************************************/
#endif /* FUN_AAT_H_ */

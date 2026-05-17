/*****************************************************************************
Description		: Fn202  Reference Input Type Advanced Auto Tuning Mode Headfile
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

#ifndef FUN_REF_AAT_H_
#define FUN_REF_AAT_H_


#include	"KnlApi.h"		/* Kernel API */
#include	"LcdOp.h"		/* LCD Operator */
#include 	"Function.h"	/* Fn Function */
#include	"Operation.h"	/* Servo Operation */
#include	"Parameter.h"	/* Parameter Definition Header File*/

/*--------------------------------------------------------------------------------------------------*/
/*		 Advanced Auto Tuning  State																*/
/*--------------------------------------------------------------------------------------------------*/
enum _enumFnRefAATState
{
	REF_AAT_SEQ_INIT = 0L,				/* Initial state							*/
	REF_AAT_SEQ_MODESET = 1L,			/* Tuning mode setting status		*/
	REF_AAT_SEQ_JRATSET = 2L,			/* Inertia setting state				*/
	REF_AAT_TUNNING_WAIT = 3L,			/* Tuning wait completion state			*/
	REF_AAT_SEQ_ENDDISP = 4L,			/* Termination display status						*/	
	REF_AAT_SEQ_END = 5L,				/* Final setting state						*/
	REF_AAT_SEQ_WAIT = 6L,				/* Waiting for termination state						*/
	REF_AAT_SEQ_RETURN 			= 7L		/* Post-processing status									*/
};


/****************************************************************************************************/
/*		STRUCT DEFINITION													  */
/****************************************************************************************************/

typedef struct _FnREFAATV {
	struct {
		enum _enumFnRefAATState State;		/* Fn202  Reference Input Type Execution State */

		LONG OpeModeIdx;				/* Operation Mode Register Index */
		LONG OpeSetIdx;					/* Operation Set Register Index */
		LONG AdatModeIdx;					/* 0x2090: Advanced AT mode Index			*/
		LONG AdatLevelIdx;				/* 0x2091 : AdvancedATLevel	Index					*/
		LONG AdatStateIdx;				/* 0x2094 : AdvancedATConfirm completion	Index					*/
		LONG AdatFilTypeIdx;			/* 0x2095 : AdvancedATFilter type	Index					*/
		ULONG AdatMode;						/* 0x2090: Advanced AT mode				*/
		ULONG AdatLevel;			/* 0x2091 : AdvancedATLevel								*/
		ULONG AdatState;					/* 0x2094 : AdvancedATConfirm completion					*/
		ULONG AdatFilType;			/* 0x2095 : AdvancedATFilter type								*/

		LONG LoophzIdex;			/* Pn100 : Speed loop gain									*/
		LONG PitimeIdex;			/* Pn101 : Speed loop integral time constant								*/
		LONG PosgnIdex;				/* Pn102 : Position loop gain									*/
		LONG MdlgnIdex;				/* Pn141 : Model following control gain								*/

		LONG ModeSetPtr;			/* Mode setting: Sequence pointer							*/
		LONG DPos;					/* Number of blinking digits													*/
		LONG JratSetPtr;			/* Moment of inertia identification: Sequence pointer					*/
		ULONG DispGain;				/* Disp Gain				*/

		LONG Reserved[(256/4)-1-10-4-4];	/* Reserved: All functions can use ram variables at least 256 bytes. */
	}var;
} FNREFAATV;

/***************************************** end of file **********************************************/
#endif /* FUN_REF_AAT_H_ */

/*****************************************************************************
Description		: Fn200 Dynamic Auto Tuning
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
K.Liu		2013.06.20		created

*****************************************************************************/
#ifndef FUN_ONEPRMTUN_H_
#define FUN_ONEPRMTUN_H_

#include	"KnlApi.h"		/* Kernel API */
#include	"LcdOp.h"		/* LCD Operator */
#include 	"Function.h"	/* Fn Function */
#include	"Operation.h"	/* Servo Operation */
#include	"PnPrmListTbl.h"

/*--------------------------------------------------------------------------------------------------*/
/* Fn200 Dynamic Auto Tuning State																	*/
/*--------------------------------------------------------------------------------------------------*/
enum _enumFnOnePrmTunState
{
	FNONEPRMTUNST_INIT 		= 0L,			/* Initial processing						*/
	FNONEPRMTUNST_JSET		= 1L,			/* Moment of inertia ratio setting process		*/
	FNONEPRMTUNST_MODESEL	= 2L,			/* Mode setting processing				*/
	FNONEPRMTUNST_FILSEL	= 3L,			/* Filter type setting processing	 	*/
	FNONEPRMTUNST_PREDISP	= 4L,			/* Pre-adjustment gain display processing	 		*/
	FNONEPRMTUNST_TUNING	= 5L,			/* 1Para adjustment processing				*/
	FNONEPRMTUNST_POSTDISP	= 6L,			/* Post-adjustment gain display process			*/
	FNONEPRMTUNST_END		= 7L			/* Termination processing						*/
};



/*--------------------------------------------------------------------------------------------------*/
typedef struct _FnOnePrmTunV {
	struct{
		enum _enumFnOnePrmTunState	State;	/* Fn200 Dynamic Auto Tuning Execution State	*/
		LONG	OpeModeIdx;					/* Operation Mode Register Index 				*/
		LONG	OpeSetIdx;					/* Operation Set Register Index 				*/
		ULONG	ResData;					/* Response Data 								*/

		ULONG	AdatFilType;				/* Filter type (2095H) Setting */
		ULONG	SpatMode;					/* 0x2050 : One Parameter Tuning Mode			*/
		ULONG	SpatLevel;					/* 0x2051 : One Parameter Tuning Level 			*/
		ULONG	SpatFFLevel;				/* 0x2052 : One Parameter Tuning FF Level 		*/
		LONG	UnActGain;					/* Un014  : Valid gain*/
		USHORT	FfSetting;					/* FFTuning setting			*/
		USHORT	Dummy1;						/* For Alignment */
		ULONG	Jrate;						/* Pn103 : Moment of inertia ratio		*/

		LONG	OpeAdatFilTypeIdx;			/* Filter Type Setting Operation Index 			*/
		LONG	OpeSpatModeIdx;				/* 0x2050 : One Parameter Tuning Mode Operation Index 	*/
		LONG	OpeSpatLevelIdx;			/* 0x2051 : One Parameter Tuning Level Operation Index 	*/
		LONG	OpeSpatFFLevelIdx;			/* 0x2052 : One Parameter Tuning FF Level  Operation Index */
		LONG	OpeSpatLvlChgStIdx;			/* 0x2053 : One Parameter Tuning Level Changing Completed	*/
		LONG	OpeSpatANotchStIdx;			/* 0x2054 : One Parameter Tuning Auto Notch Completed	*/
		LONG	OpeSpatAresStIdx;			/* 0x2055 : One Parameter Tuning Auto 'A' Completed		*/
		LONG	PrmLoopHzIdx;				/* Pn100 : Speed loop gain	 Index			*/
		LONG	PrmPiTimeIdx;				/* Pn101 : Speed loop integral time constant Index		*/
		LONG	PrmPosGnIdx;				/* Pn102 : Position loop gain Index			*/
		LONG	PrmJratIdx;					/* Pn103 : Moment of inertia ratio Index			*/
		LONG	PrmMdlGnIdx;				/* Pn141 : Model following control gain	Index		*/
		LONG	PrmAvibSwIdx;				/* Pn160 : Damping control related switches	 Index		*/
		LONG	PrmTFuncSwIdx;				/* Pn408 : Torque related function switch Index		*/
		LONG	UnActGainIdx;				/* Un014  : Valid gain  Index 				*/

		UCHAR	DigMax;						/* Maximum digit of tuning level movement			*/
		UCHAR	ModeMin;					/* Tuning mode setting minimum value			*/
		UCHAR	ModeMax;					/* Tuning mode setting maximum value			*/
		UCHAR	DPos;						/* Number of blinking digits				*/

		SHORT	FftStat;					/* Frequency analysis status	 		*/
		USHORT	Dummy2;						/* For Alignment */

		BOOL	TunSel;						/* FB/FFTuning selection 		*/

		LONG	Reserved[(256/4)-3-26]; 	/* Reserved: All functions can use ram variables */
											/* at least 256 bytes. */
	}var;
} FNONEPRMTUNV;





/***************************************** end of file **********************************************/
#endif	/*	FUN_ONEPRMTUN_H_	*/

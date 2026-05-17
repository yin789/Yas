/*****************************************************************************
Description		: Fn206 Easy FFT Measuremen
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
K.Liu		2013.06.27		created

*****************************************************************************/
#ifndef FUN_EASYFFT_H_
#define FUN_EASYFFT_H_

#include	"KnlApi.h"		/* Kernel API */
#include	"LcdOp.h"		/* LCD Operator */
#include 	"Function.h"	/* Fn Function */
#include	"Operation.h"	/* Servo Operation */
#include	"PnPrmListTbl.h"

/*--------------------------------------------------------------------------------------------------*/
/* Fn206 Easy FFT  State																	*/
/*--------------------------------------------------------------------------------------------------*/
enum _enumFnEasyFFTState
{
	FNEASYFFTST_INIT 	= 0L,		/* Initial processing						*/
	FNEASYFFTST_READY	= 1L,		/* Operation preparation						*/
	FNEASYFFTST_DRIVE	= 2L,		/* In operation	 				*/
	FNEASYFFTST_RESULT	= 3L,		/* Operation completed(Waiting for filter settings)	*/
	FNEASYFFTST_END		= 4L,		/* Termination processing 					*/
	FNEASYFFTST_TRQSET	= 5L,		/* Sweep torque setting				*/
	FNEASYFFTST_WAIT	= 6L,
};

/*--------------------------------------------------------------------------------------------------*/
typedef struct _FnEasyFFTV {
	struct{
		enum _enumFnEasyFFTState State;			/* Fn206 Easy FFT Execution State			*/
		LONG OpeModeIdx;						/* Operation Mode Register Index 			*/
		LONG OpeSetIdx;							/* Operation Set Register Index 			*/
		ULONG ResData;							/* Response Data */

		LONG OpeEzFftStateIdx;					/* 0x2061 : EasyFFTConfirm completion	Index			*/
		LONG OpeEzFftTrqIdx;					/* 0x2062 : EasyFFTTorque amplitude	Index 		*/
		LONG OpeEzFftDetfIdx;					/* 0x2410 : EasyFFTDetection frequency Index			*/
		LONG OpeEzFftFil1Idx;					/* 0x2411 : EasyFFTFilter frequency 1 Index	*/
		LONG OpeEzFftFil2Idx;					/* 0x2412 : EasyFFTFilter frequency 2	Index 	*/
		LONG PrmTFuncSwIdx;						/* Pn408 : Torque related function switch	Index	*/
		LONG PrmEzFftTrqIdx;					/* Pn456 : Sweep torque command amplitude		Index	*/

		USHORT	EzFftStateVal;					/* 0x2061 : EasyFFTConfirm completion			*/
		USHORT EzFftTrq;						/* 0x2062 : EasyFFTTorque amplitude				*/
		UCHAR DPos;								/* Number of blinking digits									*/
		UCHAR Dummy[3];							/* For Alignment */
		ULONG DispTime;							/* Display Time (Panel Operator) */

		LONG Reserved[(256/4)-3-11]; 			/* Reserved: All functions can use ram variables */
												/* at least 256 bytes. */
	}var;
} FNEASYFFTV;





/***************************************** end of file **********************************************/
#endif	/*	FUN_EASYFFT_H_	*/

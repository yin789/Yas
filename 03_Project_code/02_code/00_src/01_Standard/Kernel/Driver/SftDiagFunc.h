/*********************************** Start of file **************************************************/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SftDiagFunc.h		: Safety Diagnostic function for Sigma-7(SGD7S/W)						*/
/*																									*/
/*		Identification																				*/
/*			Module ID		: 0002(h)																*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*		Update history																				*/
/*			Module version	: 0001(h)																*/
/*			Update info.	: Created																*/
/*			Date			: 2014.03.12															*/
/*			Issue			: (M-gi-S1) Y.Tanaka													*/
/*			Update keyword	: <S1F6>																*/
/*																									*/
/****************************************************************************************************/
#ifndef SFTDIAGFUNC_H
#define SFTDIAGFUNC_H

#include "Basedef.h"

/*--------------------------------------------------------------------------------------------------*/
/*		HWBBVariable definition for detection																			*/
/*--------------------------------------------------------------------------------------------------*/
//typedef	struct DET_HWBB_STS {
	//BOOL			HwbbState;				/* Hard-wired Baseblock Status							*//* <HWBB> */
	//BOOL			Bbon1;					/* BBON1 Signal											*//* <HWBB> */
	//BOOL			Bbon2;					/* BBON2 Signal											*//* <HWBB> */
	//BOOL			Bbon1Edge;				/* BBON1 Signal Rising Edge								*//* <HWBB> */
	//BOOL			Bbon2Edge;				/* BBON2 Signal Rising Edge								*//* <HWBB> */
	//BOOL			GateDrvDefect;			/* Gate Drive Defect Status Flag						*//* <HWBB> */
	//BOOL			LackOneBbon;			/* One BBON Signal Input Only							*//* <HWBB> */
	//BOOL			hwbbsupport;			/* HWBB Support flag(PnE0D.bit9 = 1Valid in)				*//* <HWBB> */
//} DET_HWBB_STS;

/*--------------------------------------------------------------------------------------------------*/
/*		Internal signal abnormality detection(Test Pulse Diagnostic)									<S0F1>		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct TEST_PULSE_DIAG {
	BOOL			DpInEnable;				/* DPIN Signal Enable									*/
	BOOL			DpInLastEnable;			/* DPIN Signal Enable (Previous value)							*/
	BOOL			DpInState;				/* DPIN Signal Status									*/
	BOOL			DpInLastState;			/* DPIN Signal Status (Previous value)							*/
	BOOL			DpInDiagState;			/* DPIN Signal Diag. Status								*/
	BOOL			DpInLastDiagState;		/* DPIN Signal Diag. Status (Previous value)					*/
	USHORT			DpInErrCnt;				/* DPIN Signal Error Count								*/
	USHORT			DpInErrLvl;				/* DPIN Signal Error Level								*/
} TEST_PULSE_DIAG;

/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for safety input signal timing error check										*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_HWBB_TIMING {
	struct	{							/* Constant definition for safety input signal timing error check		*/
		LONG	MaxBbonTimeDiff;			/* Input signal time difference monitoring: Maximum allowable time						*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct	{							/* Variable definition for safety input signal timing error check		*/
		LONG	BbonTimeDiffCnt;			/* Safety input signal timing error time count			*/
	} var;
} CHECK_HWBB_TIMING;


#define	GET_DPIN01( )	(HALDRV_GET_GPIO06)
#define	GET_DPIN02( )	(HALDRV_GET_GPIO07)
#define	GET_DPIN11( )	(HALDRV_GET_GPIO00)
#define	GET_DPIN12( )	(HALDRV_GET_GPIO01)

/* GPIO0Output pin */
enum	TESTPLSOUT {
	SIL3_TEST_PLS1	= 3,
	SIL3_TEST_PLS0	= 4,
};


/****************************************************************************************************/
/*		API's																						*/
/****************************************************************************************************/
BOOL AdetTestPulseDiagnostic( TEST_PULSE_DIAG *TestPulseDiag );	/*<S1D1>*/
void SftReverceTestPls( void );

#endif /* SFTDIAGFUNC_H */

/*********************************** End of file ****************************************************/

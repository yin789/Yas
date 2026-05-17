/****************************************************************************/
/*																			*/
/*																			*/
/*		ACC FILTER Header file											*/
/*																			*/
/*																			*/
/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/************** Copyright Yaskawa Electric Corporation **********************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
#ifndef		ACC_FIL_H_
#define		ACC_FIL_H_

#include "ComCfg.h"
//#include "MtPRM.h"

/* defines */
#ifdef ACC_FIL_DISABLE
 #define ACF_BUF_SZ		1
#else
 #define REAL_BUF_SZ	(512000*4)/(SCAN_N_DEFCYCLE*CFG_TIO_BASE_TIME)
 #if (REAL_BUF_SZ > ACF_MAX_SZ)
  #define ACF_BUF_SZ	ACF_MAX_SZ
 #else
  #define ACF_BUF_SZ	REAL_BUF_SZ
 #endif
#endif

#define ACF_SW_NOFIL		0
#define ACF_SW_EXPFIL		1
#define ACF_SW_MAFIL		2


/* typedefs: struct */
typedef struct ACF_MAFIL_ {
	LONG		maSum;		/* Moving average filter sum */
	LONG		maRem;		/* Moving average filter remainder */
	LONG*		maBuf;		/* Moving average filter buffer */
	USHORT		maTb;		/* Moving average filter time constant */
	USHORT		maBufPtr;	/* Moving average filter buffer pointer */
	USHORT		maBufCnt;	/* Moving average filter output time */
} ACF_MAFIL;

typedef struct ACF_IMAFIL_ {
	LONG		inPosSum;	/* Input movement amount integration */
	LONG		dPos;		/* Movement amount increment value */
	LONG		lstPos;		/* Previous command input movement amount */
	LONG		dPosSum;	/* Interpolation distance integration */
	LONG		iRem;		/* Moving average filter remainder */
	UCHAR		iTbSft;		/* Moving average filter time constant */
	UCHAR		iTb;		/* Moving average filter time constant */
	UCHAR		iCnt;		/* Interpolation number counter*/
} ACF_IMAFIL;


typedef struct ACFHNDL_ {
	UCHAR		filSw;		/* Current filter type */
	BOOL		filComp;	/* Filter delivery complete */
	USHORT		expRem;		/* Exponential function filter buffer remainder */
	LONG		expBuf;		/* Exponential function filter buffer */
	LONG		expBias;	/* Exponential function filter bias speed */
	LONG		expTb;		/* Exponential function filter time constant */
	LONG		maBuf[ACF_BUF_SZ];/* Moving average filter buffer */
	ULONG		maFilTb;	/* Moving average filter time constant */
	ACF_MAFIL	maFil;		/* Moving average filter */
	ACF_IMAFIL	maIFil;		/* Moving average interpolation filter */
	ACF_PRM*	Prm;		/* Parameter */
	MC_BASE_PRM* bPrm;		/* Basic parameters */
} ACFHNDL;

/* forward declarations */
/* Public functions */
#ifdef ACC_FIL_DISABLE
#define AcfInit(hdl, bprm, prm)			(;)
#define AcfGenerate(hdl, spd, sel, den)	(;)
#define AcfClear(hdl)					(;)
#else
extern VOID AcfInit(ACFHNDL* hdl, MC_BASE_PRM* bPrm, ACF_PRM* prm);
extern VOID AcfGenerate(ACFHNDL* hdl, LONG* spd, UCHAR sel, BOOL* den);
extern VOID AcfClear(ACFHNDL* hdl);
#endif

/* Operation function macro definition */
#define AcfAttachPRM(hdl, prm)			{(hdl)->Prm = prm;}

#endif /* ACC_FIL_H_ */

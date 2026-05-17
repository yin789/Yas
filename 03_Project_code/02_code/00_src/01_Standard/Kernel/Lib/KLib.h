#ifndef WAIT_FUNC_H
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		WainFunc.h : Timer/Wait processing definition														*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"

#ifdef _TEST_BOARD_
#define FREERUN_TIMER				0
#define TIMER_CLOCK_PER_1US			25
#endif

#define KLIB_EEPDISABLE_TIME		100		/* 100ms */

/* ===>> add <TimerAPI> 2013.08.23 K.Sakamoto */
/* Interval timer register */
#define ITMR_TMD				(*((volatile unsigned long*)(0xEFFF1000)))	/* Timer counter register		*/
#define ITMR_CMD				(*((volatile unsigned long*)(0xEFFF1004)))	/* Compare register			*/
#define ITMR_TMCD				(*((volatile unsigned long*)(0xEFFF1008)))	/* Control Register		*/
/* Setting value */
#define ITMR_CMD_SET			0xFFFFFFFF		/* Compare register setting value(0,1,..,0xFFFFFFFE,0xFFFFFFFF,0,1,..Setting)	*/
#define ITMR_TMCD_SET			0x00000003		/* Control register setting value(INCLOCKof2Division,Operation enabled)						*/
/* nsecFor calculation */
#define ITMR_NSEC_PER_COUNT		32UL			/* 1Time per count(nsec)	*/
/* usecFor calculation(Be careful as it will not work correctly if you modify only a part of the settings) */
#define ITMR_USEC_MAX			0x020C49BA		/* waitMaximum time that can be calculated with(usec) 34,359,738 usec	*/
												/* 0x02000000 as the loop condition value with a fixed count value		*/
#define ITMR_USEC_SHIFT			25				/* Number of shifts to determine whether a loop with fixed count value is necessary	*/
#define ITMR_USEC_MASK			0x01FFFFFF		/* Mask pattern to remove loop count from specified time		*/
#define ITMR_USEC_FIX_COUNT		0x3E800000		/* Fixed count value((0x02000000 * 125) / 4)			*/
#define ITMR_USEC_MUL_VAL		125UL			/* Multiplier(Originally1,000However, instead of only1,000/8=125)				*/
#define ITMR_USEC_DIV_VAL		4UL				/* Divisor(Originally32However, instead of only32/8=4)						*/
/* msecFor calculation(Be careful as it will not work correctly if you modify only a part of the settings) */
#define ITMR_MSEC_MAX			0x000218DE		/* waitMaximum time that can be calculated with(msec) 137,438 msec	*/
												/* 0x00020000 as the loop condition value with a fixed count value		*/
#define ITMR_MSEC_SHIFT			17				/* Number of shifts to determine whether a loop with fixed count value is necessary	*/
#define ITMR_MSEC_MASK			0x0001FFFF		/* Mask pattern to remove loop count from specified time		*/
#define ITMR_MSEC_FIX_COUNT		0xF4240000		/* Fixed count value(0x00020000 * 31,250)				*/
#define ITMR_MSEC_MUL_VAL		31250UL			/* Multiplier(Originally1,000,000 However, instead of only1,000,000/32=31,250)	*/
#define ITMR_MSEC_DIV_VAL		1				/* Divisor(Originally32However, instead of only32/32=1)						*/
/* <<=== add <TimerAPI> 2013.08.23 K.Sakamoto */

enum	INTERRUPT_NAME {
#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE	/* AP */
	INTERRUPT_IntSync = 1,
//	CoE_SRST = 3,		/*<S008>*/
	OP_SWRST = 3,							/* <S106> */
	SIL3_TEST_PLS	= 4,					/* <S0F1> */
	INTERRUPT_ScanA	= 5 ,
	INTERRUPT_ScanB = 6,
	INTERRUPT_ScanC = 7,
#else
//	DebugPort1		= 1,					/* Erase after */
//	DebugPort2		= 7,					/* Erase after */
	INTERRUPT_ScanA	= 1,
	OP_SWRST 		= 3,					/* <S106> */
	SIL3_TEST_PLS	= 4,					/* <S0F1> */
	INTERRUPT_ScanB = 7,
	INTERRUPT_IntSync,						/* DebugPortshould not be used!!!!!!!!! */
	INTERRUPT_ScanC,						/* DebugPortshould not be used!!!!!!!!! */
#endif
};

enum	INTERRUPT_TIMING {
	INT_TIMING_ScanB	= 0x01,
	INT_TIMING_ScanC	= 0x0F,
};


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/* ===>> add <TimerAPI> 2013.08.23 K.Sakamoto */
void	KlibInitialize( void );								/* KlibInitialize	*/
/* <<=== add <TimerAPI> 2013.08.23 K.Sakamoto */

void	KlibWaitns( LONG ns );
void	KlibWaitus( LONG us );
void	KlibWaitms( LONG ms );

void	KlibRstLongCounter( ULONG *timer );			/* Difference counter reset	<S00E>	*/
ULONG	KlibGetLongCounter( ULONG timer );			/* Counter timekeeping	<S00E>	*/
void	KlibRstLongTimer( ULONG *timer );			/* Timer reset	<S00E>	*/
ULONG	KlibGetLongTimerMs( ULONG timer );			/* Timer[ms]Time measurement	<S00E>	*/
ULONG	KlibGetLongTimerUs( ULONG timer );			/* Timer[ms]Time measurement	<S00E>	*/
void	KlibCountLongTimer( ULONG divShift );		/* Counter processing	<S00E>	*/

void KlibInitializeTimeStamp( ULONG time );
void KlibCountTimeStamp( ULONG tick_in_msec, BOOL PowerOn );
LONG KlibGetTimeAfterPowerOn( void );
ULONG KlibGetTimeStamp( void );
LONG KlibGetMainPowerOffTimer( void );

ULONG KlibClockToTime(ULONG clock_diff);
ULONG KlibGetClockTime(void);

void InitializeInterruptTiming( void );
UCHAR GetInterruptTiming( UCHAR IntName );

/* ===>> add <TimerAPI> 2013.08.23 K.Sakamoto */
void	ITmr_InitTimer( void );										/* Interval timer initialization		*/
ULONG	ITmr_GetTimerCount( void );									/* Timer counter register value acquisition	*/
ULONG	ITmr_ConvNsec2count( LONG TimeNsec );						/* Time(nsec)->Count conversion	*/
void	ITmr_WaitExceedTimeUsec( LONG *TimeUsec, ULONG *StTimer );	/* Maximum time(usec)Exceeding amountwait	*/
ULONG	ITmr_ConvUsec2count( LONG TimeUsec );						/* Time(usec)->Count conversion	*/
void	ITmr_WaitExceedTimeMsec( LONG *TimeMsec, ULONG *StTimer );	/* Maximum time(msec)Exceeding amountwait	*/
ULONG	ITmr_ConvMsec2count( LONG TimeMsec );						/* Time(msec)->Count conversion	*/
ULONG	ITmr_ConvCount2msec( ULONG Count );							/* Count->Time(msec)Convert	*/
ULONG	ITmr_ConvCount2usec( ULONG Count );							/* Count->Time(usec)Convert	*/
/* <<=== add <TimerAPI> 2013.08.23 K.Sakamoto */


/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SystemError.h : System error processing definition														*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void	KpiSystemError( void );

#define WAIT_FUNC_H
#endif /* WAIT_FUNC_H */
/***************************************** end of file **********************************************/

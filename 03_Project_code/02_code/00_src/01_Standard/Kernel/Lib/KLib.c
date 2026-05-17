/****************************************************************************
Description	: Kernel System Library
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
I.M.   2010.5.22     created

****************************************************************************/
//#include <INTEGRITY.h>
#include "Basedef.h"
#include "KLib.h"
#ifndef _MERCURY_PLATFORM_	/* Encrypted format support <S146> */
#include "HwDeviceIf.h"
#include "JL086ServoIpHw.h"
#endif

/* 2012.08.19 Y.Oka Make Mercury Base Software */
#ifdef _VC_DEBUG_
#include <stdio.h>
#include <time.h>
#endif
/* 2012.08.19 Y.Oka Make Mercury Base Software */

#if defined(_TEST_BOARD_) && !defined(_VC_DEBUG_)
#include "nbpfapbtm32.h"
#endif

#define KLIB_EEPDISABLE_TIME		100		/* 100ms */

/* globals */
#pragma arm section rwdata = "Globals", zidata = "Globals"
static LONG		StartUpTimer;
static ULONG	Timer100msCounter;
static ULONG	PowerOnTimer;
static LONG		EepDisableTimer;

static ULONG	KlibScanCounter;
static ULONG	KlibTimerValue;

/* ===>> delete <TimerAPI> 2013.08.23 K.Sakamoto */
#if 0
static ASICS	*pSvAsicRegs;
#endif
/* <<=== delete <TimerAPI> 2013.08.23 K.Sakamoto */
#if defined(_TEST_BOARD_) && !defined(_VC_DEBUG_)
//static UCHAR	IntScanBWaitCounter;/* Unused */
//static UCHAR	IntScanCWaitCounter;/* Unused */
static UCHAR 	IntScanBTimingCounter;
static UCHAR 	IntScanCTimingCounter;
#endif

#pragma arm section rwdata, zidata


/****************************************************************************
	Initialize the Klib variables
****************************************************************************/
void KlibInitialize( void )
{
	ITmr_InitTimer();
}

/****************************************************************************
	Initialize the Time Stamp values
****************************************************************************/
void KlibInitializeTimeStamp( ULONG time )
{
	Timer100msCounter = time;
	PowerOnTimer = 0;
	StartUpTimer = 0;
	EepDisableTimer = KLIB_EEPDISABLE_TIME;
}

/****************************************************************************
	Count up the Time Stamp value(Called from cyclic scan task)
****************************************************************************/
void KlibCountTimeStamp( ULONG tick_in_msec, BOOL PowerOn )
{
	/* Timestamp counter update */
	if(PowerOn != FALSE)		/* Power OnUpdate only when */
	{
		PowerOnTimer += tick_in_msec;
		if(PowerOnTimer >= 100)
		{
			Timer100msCounter++;
			PowerOnTimer = 0;
		}
		EepDisableTimer = 0;
	}
	else
	{
		if(EepDisableTimer < KLIB_EEPDISABLE_TIME)
		{	EepDisableTimer += tick_in_msec;}
	}

	/* Main timer(Timer from control power-on[ms], Max.1193H/49D)*/
	if( StartUpTimer < 0xFFFF0000 )	
	{
		StartUpTimer += tick_in_msec;
	}
}

/****************************************************************************
	Get a time stamp value
****************************************************************************/
ULONG KlibGetTimeStamp( void )
{
	return Timer100msCounter;
}

/****************************************************************************
	Get a power on time after start up 
****************************************************************************/
LONG KlibGetTimeAfterPowerOn( void )
{
	return StartUpTimer;
}

/****************************************************************************
	Get the power off time state 
****************************************************************************/
LONG KlibGetMainPowerOffTimer( void )
{
	return EepDisableTimer;
}


/****************************************************************************
	[ns]Unit weight 
****************************************************************************/
void KlibWaitns( LONG ns )
{
/* ===>> add <TimerAPI> 2013.08.23 K.Sakamoto */
	ULONG	StTime;		/* Start time	*/
	ULONG	CuTime;		/* Current time	*/
	ULONG	Count;		/* Count value	*/

	/* Get start count */
	StTime = ITmr_GetTimerCount();

	/* Execute wait when positive value */
	if( ns > 0 )
	{
		/* Convert specified time to count value */
		Count = ITmr_ConvNsec2count( ns );
		/* Time wait processing */
		do
		{
			/* Get current count */
			CuTime = ITmr_GetTimerCount();
		} while((CuTime - StTime) < Count);
	}
/* <<=== add <TimerAPI> 2013.08.23 K.Sakamoto */

	return;
}

/****************************************************************************
	[μs]Unit weight 
****************************************************************************/
void KlibWaitus( LONG us )
{

	ULONG	StTime;		/* Start time	*/
	ULONG	CuTime;		/* Current time	*/
	ULONG	Count;		/* Count value	*/
	LONG	TimeUsec;	/* Waiting time	*/

	/* Get start count */
	StTime = ITmr_GetTimerCount();

	/* Execute wait when positive value */
	if( us > 0 )
	{
		/* Waiting time save */
		TimeUsec = us;

		/* Maximum time(usec)Exceeding amount waiting process*/
		ITmr_WaitExceedTimeUsec( &TimeUsec, &StTime );

		/* Convert specified time to count value */
		Count = ITmr_ConvUsec2count( TimeUsec );

		/* Time wait processing */
		do
		{
			/* Get current count */
			CuTime = ITmr_GetTimerCount();

		} while((CuTime - StTime) < Count);
	}

	return;
}

/****************************************************************************
	[ms]Unit weight 
****************************************************************************/
void KlibWaitms( LONG ms )
{

	ULONG	StTime;		/* Start time	*/
	ULONG	CuTime;		/* Current time	*/
	ULONG	Count;		/* Count value	*/
	LONG	TimeMsec;	/* Waiting time	*/

	/* Get start count */
	StTime = ITmr_GetTimerCount();

	/* Execute wait when positive value */
	if( ms > 0 )
	{
		/* Waiting time save */
		TimeMsec = ms;

		/* Maximum time(msec)Exceeding amount waiting process*/
		ITmr_WaitExceedTimeMsec( &TimeMsec, &StTime );

		/* Convert specified time to count value */
		Count = ITmr_ConvMsec2count( TimeMsec );

		/* Time wait processing */
		do
		{
			/* Get current count */
			CuTime = ITmr_GetTimerCount();

		} while((CuTime - StTime) < Count);
	}
}

/****************************************************************************
	Long timer counter reset						<S00E>
****************************************************************************/
void KlibRstLongCounter( ULONG *timer )
{
	*timer = KlibTimerValue;
}

/****************************************************************************
	Long timer value acquisition [counter]				<S00E>
****************************************************************************/
ULONG KlibGetLongCounter( ULONG timer )
{
/* ===>> change <TimerAPI> 2013.08.23 K.Sakamoto */
#if 0	/*<S00B>*//* Processing revival<S043> */
	return (ULONG)(KlibTimerValue - timer);
#else
	ULONG	CuCount;	/* Current count		*/
	ULONG	Dcount;		/* Count difference		*/

	/* Get current count */
	CuCount = ITmr_GetTimerCount();

	/* Difference value calculation*/
	Dcount = CuCount - timer;

	/* Return counter value */
	return Dcount;
#endif
/* <<=== change <TimerAPI> 2013.08.23 K.Sakamoto */
}

/****************************************************************************
	Long timer reset
****************************************************************************/
void KlibRstLongTimer( ULONG *timer )
{
/* ===>> change <TimerAPI> 2013.08.23 K.Sakamoto */
#if 0	/*<S00B>*//*Uncommented<S00E>*/
	*timer = KlibTimerValue;
#else
	*timer = ITmr_GetTimerCount();
#endif
/* <<=== change <TimerAPI> 2013.08.23 K.Sakamoto */
}

/****************************************************************************
	Long timer value acquisition [ms] 					<S00E>
****************************************************************************/
ULONG KlibGetLongTimerMs( ULONG timer )
{
/* ===>> change <TimerAPI> 2013.08.23 K.Sakamoto */
#if 0	/*<S00B>*//*Uncommented<S00E>*/
	return (ULONG)(KlibTimerValue - timer);
#else
	ULONG	CuCount;	/* Current count		*/
	ULONG	Dcount;		/* Count difference		*/
	ULONG	TimeMsec;	/* Elapsed time(msec)	*/

	/* Get current count */
	CuCount = ITmr_GetTimerCount();

	/* Difference value calculation*/
	Dcount = CuCount - timer;

	/* Count->Time(msec)Convert */
	TimeMsec = ITmr_ConvCount2msec( Dcount );

	/* Time(msec) */
	return TimeMsec;
#endif
/* <<=== change <TimerAPI> 2013.08.23 K.Sakamoto */
}

/****************************************************************************
	Long timer value acquisition [us]						<S00E>
****************************************************************************/
ULONG KlibGetLongTimerUs( ULONG timer )
{
	ULONG	CuCount;	/* Current count		*/
	ULONG	Dcount;		/* Count difference		*/
	ULONG	TimeUsec;	/* Elapsed time(usec)	*/

	/* Get current count */
	CuCount = ITmr_GetTimerCount();

	/* Difference value calculation*/
	Dcount = CuCount - timer;

	/* Count->Time(usec)Convert */
	TimeUsec = ITmr_ConvCount2usec( Dcount );

	/* Time(usec) */
	return TimeUsec;
/* <<=== change <TimerAPI> 2013.08.23 K.Sakamoto */
}

/****************************************************************************
	Long timer value count [ms] 
	ScanB 125usthendivShift=3, 250usthendivShift=2
****************************************************************************/
void KlibCountLongTimer( ULONG CountInMS )
{
	KlibScanCounter++;
	if(KlibScanCounter >= CountInMS)
	{
		KlibScanCounter = 0;
		KlibTimerValue ++;
	}

}

/****************************************************************************
	System clock timer value acquisition(32bit Clock Countin1Second)
****************************************************************************/
 ULONG KlibGetClockTime(void)
{
//	Time t;
//	GetClockTime( HighResClock, &t);
//	return t.Fraction;
	ULONG t;
#ifdef _VC_DEBUG_ /* 2012.08.19 Y.Oka Make Mercury Base Software */
	t = clock();
#elif defined(_TEST_BOARD_) /* 2012.08.19 Y.Oka Make Mercury Base Software */
	timer_ioctl( 0, TIMER_IOCTL_GET_COUNT, (VOID *)&t);
#else
	t = 0;
#endif /* 2012.08.19 Y.Oka Make Mercury Base Software */
	return t;
}

/****************************************************************************
	System clock→[ns]Convert
****************************************************************************/
ULONG KlibClockToTime(ULONG clock_diff)
{
//	float time_in_ns;
//	time_in_ns = (float)clock_diff * (float)(1000000000.0/4294967296.0);
//	return (ULONG)time_in_ns; /* [ns] */
	ULONG time_in_ns;
#ifdef _VC_DEBUG_ /* 2012.08.19 Y.Oka Make Mercury Base Software */
	time_in_ns = clock_diff * 1000000;	/* clock_diff[ms] */
#else /* _VC_DEBUG_ */ /* 2012.08.19 Y.Oka Make Mercury Base Software */
//	time_in_ns = clock_diff * 20;	/* 1000/50MHz */
	time_in_ns = clock_diff * 32;	/* 1000/31.25MHzSoC<S043> */
#endif /* 2012.08.19 Y.Oka Make Mercury Base Software */
	return (ULONG)time_in_ns; /* [ns] */
}


/****************************************************************************
	ScanB, ScanC Interrupt timing initialization
****************************************************************************/
void InitializeInterruptTiming( void )
{
#if defined(_TEST_BOARD_) && !defined(_VC_DEBUG_)
//	IntScanBWaitCounter = 0;/* Unused */
//	IntScanCWaitCounter = 0;/* Unused */
	IntScanBTimingCounter = INT_TIMING_ScanB;
	IntScanCTimingCounter = INT_TIMING_ScanC;
#endif
	return;
}


/****************************************************************************
	Interrupt timing acquisition		※ScanA, ScanBtoCall
							※Return value : TRUE: Interrupt timing
****************************************************************************/
UCHAR GetInterruptTiming( UCHAR IntName )
{
	UCHAR	ret;
	ret = FALSE;
#if defined(_TEST_BOARD_) && !defined(_VC_DEBUG_)

	if( IntName == INTERRUPT_ScanB )
	{
		if( (IntScanBTimingCounter & INT_TIMING_ScanB) == INT_TIMING_ScanB )
		{
			ret = TRUE;
		}
		IntScanBTimingCounter++;
	}
	else if( IntName == INTERRUPT_ScanC )
	{
		if( (IntScanCTimingCounter & INT_TIMING_ScanC) == INT_TIMING_ScanC )
		{
			ret = TRUE;
		}
		IntScanCTimingCounter++;
	}
	else
	{
		/* DO NOTHING */
	}
#endif
	return ret;
}



/****************************************************************************************************/
/*																									*/
/*		Error handling in case of fatal abnormality																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Function overview : Disable interrupts and stop the program in base block state.						*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	KpiSystemError( void )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Program stop																				*/
/*--------------------------------------------------------------------------------------------------*/
	for( ; ; )											/* Program stop infinite loop					*/
	{													/*											*/
		;												/*											*/
	}													/*											*/
/*--------------------------------------------------------------------------------------------------*/

}

/* ===>> add <TimerAPI> 2013.08.23 K.Sakamoto */
/****************************************************************************
	Interval timer initialization
****************************************************************************/
void	ITmr_InitTimer( void )
{
	/* Compare register setting */
	ITMR_CMD = ITMR_CMD_SET;

	/* Control register setting */
	ITMR_TMCD = ITMR_TMCD_SET;
}

/****************************************************************************
	Timer counter register value acquisition
****************************************************************************/
ULONG	ITmr_GetTimerCount( void )
{
	ULONG	Count;		/* Count value	*/

	/* Timer counter register value acquisition */
	Count = ITMR_TMD;

	return Count;
}

/****************************************************************************
	Time(nsec)->Count conversion
****************************************************************************/
ULONG	ITmr_ConvNsec2count( LONG TimeNsec )
{
	ULONG	Count;		/* Count value	*/

	/* Count value calculation */
	Count = (ULONG)TimeNsec / ITMR_NSEC_PER_COUNT;

	return Count;
}

/****************************************************************************
	Maximum time(usec)Exceeding amountwait
****************************************************************************/
void	ITmr_WaitExceedTimeUsec( LONG *TimeUsec, ULONG *StTimer )
{
	ULONG	CuTimer;	/* Current count value	*/
	ULONG	Dcount;		/* Count difference	*/
	ULONG	LoopMax;	/* Loop count		*/

	/* Get loop count */
	LoopMax = (ULONG)(*TimeUsec) >> ITMR_USEC_SHIFT;

	/* Set remaining time */
	*TimeUsec = *TimeUsec & ITMR_USEC_MASK;

	/* Loop execution */
	for( ; LoopMax > 0; LoopMax-- )
	{
		/* Wait processing */
		do
		{
			/* Get current count value */
			CuTimer = ITmr_GetTimerCount();

			/* Count difference calculation */
			Dcount = CuTimer - *StTimer;
		} while( Dcount < ITMR_USEC_FIX_COUNT );

		/* Update start count */
		*StTimer = *StTimer + ITMR_USEC_FIX_COUNT;
	}
}

/****************************************************************************
	Time(usec)->Count conversion
****************************************************************************/
ULONG	ITmr_ConvUsec2count( LONG TimeUsec )
{
	LONG	WaitTime;	/* Specified time	*/
	ULONG	Count;		/* Count value	*/

	/* Initializing variables */
	WaitTime = TimeUsec;

	/* Check whether it exceeds the maximum value */
	if( WaitTime > ITMR_USEC_MAX )
	{
		WaitTime = ITMR_USEC_MAX;	/* Forcibly set the maximum value		*/
	}

	/* Count value calculation */
	Count = ((ULONG)WaitTime * ITMR_USEC_MUL_VAL) / ITMR_USEC_DIV_VAL;

	return Count;
}

/****************************************************************************
	Maximum time(msec)Exceeding amountwait
****************************************************************************/
void	ITmr_WaitExceedTimeMsec( LONG *TimeMsec, ULONG *StTimer )
{
	ULONG	CuTimer;	/* Current count value	*/
	ULONG	Dcount;		/* Count difference	*/
	ULONG	LoopMax;	/* Loop count		*/

	/* Get loop count */
	LoopMax = (ULONG)(*TimeMsec) >> ITMR_MSEC_SHIFT;

	/* Set remaining time */
	*TimeMsec = *TimeMsec & ITMR_MSEC_MASK;

	/* Loop execution */
	for( ; LoopMax > 0; LoopMax-- )
	{
		/* Wait processing */
		do
		{
			/* Get current count value */
			CuTimer = ITmr_GetTimerCount();

			/* Count difference calculation */
			Dcount = CuTimer - *StTimer;
		} while( Dcount < ITMR_MSEC_FIX_COUNT );

		/* Update start count */
		*StTimer = *StTimer + ITMR_MSEC_FIX_COUNT;
	}
}

/****************************************************************************
	Time(msec)->Count conversion
****************************************************************************/
ULONG	ITmr_ConvMsec2count( LONG TimeMsec )
{
	LONG	WaitTime;	/* Specified time	*/
	ULONG	Count;		/* Count value	*/

	/* Initializing variables */
	WaitTime = TimeMsec;

	/* Check whether it exceeds the maximum value */
	if( WaitTime > ITMR_MSEC_MAX )
	{
		WaitTime = ITMR_MSEC_MAX;	/* Forcibly set the maximum value		*/
	}

	/* Count value calculation */
	Count = (ULONG)WaitTime * ITMR_MSEC_MUL_VAL;

	return Count;
}

/****************************************************************************
	Count->Time(msec)Convert
****************************************************************************/
ULONG	ITmr_ConvCount2msec( ULONG Count )
{
	ULONG	TimeMsec;	/* Elapsed time(msec)	*/

	/* Calculate elapsed time */
	TimeMsec = Count / ITMR_MSEC_MUL_VAL;

	return TimeMsec;
}
/* <<=== add <TimerAPI> 2013.08.23 K.Sakamoto */

/****************************************************************************
	Count->Time(usec)Convert
****************************************************************************/
ULONG	ITmr_ConvCount2usec( ULONG Count )
{
	ULONG	TimeUsec;	/* Elapsed time(usec)	*/

	/* Calculate elapsed time */
	TimeUsec = Count / ITMR_USEC_MUL_VAL;

	return TimeUsec;
}



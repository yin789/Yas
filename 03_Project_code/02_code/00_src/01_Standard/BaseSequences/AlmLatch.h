/*****************************************************************************
Description		: Alarm latch function head feel
-----------------------------------------------------------------------------
Author		: SHCORE
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
K.Liu		2013.10.11		created

*****************************************************************************/

#ifndef ALARM_LATCH_API_H
#define ALARM_LATCH_API_H

#include "BaseControls.h"
#include "SequenceIo.h"
#include "CheckMotSts.h"
#include "PowerManager.h"
#include "Pls.h"
#include "SvOptCardIf.h"
#include "UnitPrmStruct.h"					/* <S212> */

#define EEPSIZE_ALMMON		64

/*--------------------------------------------------------------------------------------------------*/
/*	Alarm latch monitor																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct _ALMLTMON
{
	/*----------------------------------------------------------------------------------------------*/
		SHORT	MotSpd;					/* Motor position difference(2ms)				[min-1/mm/s]			*/
		SHORT	SpdRef;					/* Speed command							[min-1/mm/s]			*/
		SHORT	TrqRef;					/* Torque command						[%]						*/
		SHORT	PcmdSpd;				/* Position command difference(2ms)				[min-1/mm/s]			*/
		LONG	PosErr;					/* Position deviation output(Rounding)									*/
	/*----------------------------------------------------------------------------------------------*/
		ULONG	InterSignal;			/* Internal signal(For monitor)										*/
		ULONG	InSignal;				/* Input signal(For monitor)										*/
		ULONG	OutSignal;				/* Output signal(For monitor)										*/
	/*----------------------------------------------------------------------------------------------*/
		SHORT	DcVoltage;				/* Main circuit bus voltage					[V]						*/
		SHORT	IFBMon;					/* Current FB monitor					[15000/Imax]			*/
		USHORT	MotLoad;				/* Cumulative load rate 						[%]						*/
		USHORT	RgPower;				/* Regenerative power consumption						[%]						*/
		USHORT	DbPower;				/* DB power consumption 					[%]						*/
		USHORT	MaxMotLoad10s;			/* Maximum cumulative load factor 					[%(Period10s)]			*/
		USHORT	JLRatio;				/* Moment of inertia ratio/Weight ratio			[%]						*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	EncComErrCnt;			/* Serial encoder communication error count							*/
		USHORT	M2ComErrCnt;			/* MECHATROLINKCommunication error count									*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	SafetyIOSignal;			/* Safety input/output signal(For monitor) <V066>					*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	AlmCode;				/* Alarm code											*/
		USHORT	spare1;					/* Preliminary														*/
		ULONG	TmStamp;				/* Alarm timestamp									*/
	/*----------------------------------------------------------------------------------------------*/
		USHORT	spare2[6];				/* Preliminary														*/
	/*----------------------------------------------------------------------------------------------*/
} ALMLTMON;

/*--------------------------------------------------------------------------------------------------*/
/*	Alarm latch data definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct _ALMLATCH_HNDL
{
	struct {
		BOOL	LastAlmStatus;					/* Previous alarm status							*/
	} var;
	/*----------------------------------------------------------------------------------------------*/
	ALMLTMON			AlmLtMon;			/* Alarm latch monitor(Update)						*/
	ALMLTMON			LatchMon;			/* Alarm latch monitor(Update when an alarm occurs)			*/
	ANLG_UN_MON_DATA	*UnMonitor;			/* Amon/UnPointer to monitor data						*/
	SEQ_MOT_STS			*SeqMotSts;			/* ScanCMotor status					*/
	BPRMDAT				*Bprm;				/* Basic parameter definition									*/
	BASE_LOOPCTRLS		*BaseLoops;			/* Control loop calculation data								*/
	POW_MNG_HNDL		*PowerManager;		/* Power management data(Common to units)						*/
	SEQ_CTRL_OUT		*SeqCtrlOut;		/* ScanCOutput data										*/
	MENCV				*FencV;				/* Fulque encoder data								*/
	MENCV				*MencV;				/* Pointer to motor encoder data					*/
	BE_SEQ_HNDL			*BeSequence;		/* Base Enable Sequence Handle							*/
	BASE_CTRL_HNDL		*BaseControls;		/* Basic control shared data									*/
	BASE_CTRL_OUT		*BaseCtrlOut;		/* ScanBOutput data										*/
	ALMMAN_HANDLE		*AlmManager;		/* Alarm control structure									*/
	SEQ_IO_HNDL			*IoSignals;			/* SequenceI/O 										*/
	SERVO_CONTROL_IF	*SvControlIf;		/* M3 I/FData											*/
	PLSIFV 				*pPlsIfV;			/* <S00C> Command Pulse Interface						*/
	OPSFT_HNDL			*SftOpt;			/* Pointer to safety module data		*/
	UNI_PRM_STR			*UniPrm;			/* Pointer to parameter handle				 <S212> */

} ALMLATCH_HNDL;





/****************************************************************************************************/
/*																									*/
/*		Function list																					*/
/*																									*/
/****************************************************************************************************/
extern void AlmLatchMonitorInitial( ALMLATCH_HNDL *hAlmLatchHndl );
extern void AlmLatchMonitorSequence( ALMLATCH_HNDL *hAlmLatchHndl );




#endif	/* ALARM_LATCH_API_H */

/***************************************** end of file **********************************************/


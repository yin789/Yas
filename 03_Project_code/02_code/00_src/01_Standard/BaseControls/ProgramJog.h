#ifndef _PRGRAM_JOG_H
#define _PRGRAM_JOG_H
/****************************************************************************
Description	: Program Jog
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
T.Kira  2011.03.08    created

*****************************************************************************/
#include "Basedef.h"
#include "CtrlLoopHandler.h"
#include "BaseCmnStruct.h"
#include "AlarmManager.h"
#include "KLib.h"
#include "MLib.h"

/************************************************************************************************/
/*																								*/
/*		ProgramJOGOperation data definition															*/
/*																								*/
/************************************************************************************************/
/*----------------------------------------------------------------------------------------------*/
/*		ProgramJOGOperation data structure															*/
/*----------------------------------------------------------------------------------------------*/
typedef	struct PJOGV{
	struct{							/* ProgramJOGUsable constant data								*/
		LONG	Distance;			/* ProgramJOGOperation target position		[Command unit]				*/
		LONG	MaxSpd;				/* ProgramJOGMaximum driving speed		[2^24/OvrSpd]			*/
		LONG	AccTime;			/* ProgramJOGOperation acceleration/deceleration time		[us]					*/
		USHORT	WaitTime;			/* Waiting time							[ms]					*/
		USHORT	MTimes;				/* Number of movements													*/
		CHAR	Pattern;			/* ProgramJOGOperation command pattern							*/
		CHAR	RevDir;				/* ProgramJOGOperation mode start direction							*/
		CHAR	PrmUnMatch;			/* Parameter combination error								*/
		CHAR	PrmUnMatch2;		/* Parameter combination error2								*/
	} Cnst;
/*----------------------------------------------------------------------------------------------*/
	struct{							/* ProgramJOGUsable variable data								*/
		LONG	AbsPosCmd;			/* ProgramJOGPosition command			[Command unit	]			*/
		ULONG	TimeWork;			/* Waiting time measurement data										*/
		USHORT	DispTime;			/* Paneope display data	<V068>								*/
		USHORT	MCntr;				/* Number of repetitions												*/
		USHORT	SeqPtr;				/* ProgramJOGOperation command creation sequence pointer				*/
		UCHAR	RoutCmd;			/* ProgramJOGOperation command	(Round  -> Scan C)				*/
		UCHAR	CoutCmd;			/* ProgramJOGOperation command	(Scan C -> Scan B)				*/
		UCHAR	State;				/* ProgramJOGOperation status									*/
		BOOL	ScanCRunFlg;		/* ScanCinON,RoundinOFFflag to(Interrupt substitution)				*/
	} Var;
} PJOGV;

/*----------------------------------------------------------------------------------------------*/
/*		ProgramJOGStructure																		*/
/*----------------------------------------------------------------------------------------------*/
typedef	struct PJOGHNDL{
	PJOGV   PJogV;
	PCMKPV	PJogPcmk;
	SHORT	PJogState;
}PJOGHNDL;

/*----------------------------------------------------------------------------------------------*/
/*		ProgramJOGOperation status definition															*/
/*----------------------------------------------------------------------------------------------*/
#define	PJOG_INIT	0x00				/* Initial state										*/
#define	PJOG_ABORT	0x10				/* Interrupted state												*/
#define	PJOG_START	0x01				/* Operation status												*/
#define	PJOG_AFTER	0x11				/* Post-operation processing status										*/
#define	PJOG_END	0x02				/* Termination status												*/
#define	PJOG_ERROR	0x03				/* Abnormal termination												*/
/*----------------------------------------------------------------------------------------------*/
/*		ProgramJOGOperation command definition															*/
/*----------------------------------------------------------------------------------------------*/
#define	PJOGCMD_NONE	0x00			/* No command											*/
#define	PJOGCMD_INIT	0x01			/* Initialize										*/
#define	PJOGCMD_ABORT	0x02			/* interruption													*/
#define	PJOGCMD_START	0x03			/* Operation													*/

#define		LMTLOWSPEED	10
/*----------------------------------------------------------------------------------------------*/
/*		ProgramJOGOperation-related function definition															*/
/*----------------------------------------------------------------------------------------------*/
void PrgJogReset( PJOGHNDL *PJogHdl );
void IprmcalPrgJog( PJOGHNDL *PJogHdl, BPRMDAT *Bprm, POS_MNG_HNDL *PosManager );
void CpxPrgJogPosSequence( PJOGHNDL *PJogHdl, SEQ_CTRL_OUT *SeqCtrlOut, 
												ALMMAN_HANDLE *AlmManager, LONG FnCtrlMcmd ); 
BOOL PrgJogMakePositionReference( PJOGHNDL *PJogHdl, LONG *dPosRefo );

#endif //_PRGRAM_JOG_H 
/***************************************** end of file ******************************************/

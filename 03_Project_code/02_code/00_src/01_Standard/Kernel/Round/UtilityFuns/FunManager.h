/****************************************************************************
Description	: Utility Function Manager
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
I.M.   2010.11.12     created

*****************************************************************************/
#ifndef __FUN_MANAGER_H
#define __FUN_MANAGER_H

#include "PrmAccessIf.h"


/****************************************************************************
 Definition of the application data
*****************************************************************************/
typedef enum FCMD_CODE {
	FCMD_STOP	= 0x00,					/* Stop					*/
	FCMD_EXEC	= 0x01,					/* Execution					*/
	FCMD_UP		= 0x02,					/* UP,Forward rotation,forward direction		*/
	FCMD_DOWN	= 0x03,					/* DOWN,Reverse running,Reverse direction		*/
	FCMD_SV		= 0x04,					/* Servo on/Off		*/
	FCMD_CH1	= 0x05,					/* CH1,UPhase selection			*/
	FCMD_CH2	= 0x06,					/* CH2,VPhase selection			*/
	FCMD_EWRT	= 0x07,					/* EEPROMWrite process		*/
	FCMD_EXEC2	= 0x08,					/* Auxiliary execution				*/
	FCMD_MTEXEC	= 0x11,					/* Operation mode maintained(Execution)	*/
	FCMD_MTUP	= 0x12,					/* Operation mode maintained(Forward rotation)	*/
	FCMD_MTDOWN	= 0x13,					/* Operation mode maintained(Reverse running)	*/
	FCMD_NONE	= 0xFF,					/* No instruction				*/
} FCMD_CODE;

/*----------------------------------------------------------------------------
 Definition of the number of Function mode
----------------------------------------------------------------------------*/
#define FUN_MODE_NUM	2
typedef enum FUN_EXECH {
	FUN_CH1 = 0,	/* the first mode */
	FUN_CH2 = 1,	/* the second mode */
} FUN_EXECH;

typedef	struct FUNEXE { /* Referenced by auxiliary function application side/Operable members */
	FCMD_CODE	CmdCode;				/* Instruction code */
	UINT		State;					/* FnSequence status managed by the application */
										/* (FnAt mode start0Reset) */
	ULONG		LongTimer;				/* FnApplication timer */
	BOOL		HoldFnMode;				/* FnMode continuation flag */
	PRM_RSLT	ErrorResult;			/* FnMode execution result */
} FUNEXE;

/*----------------------------------------------------------------------------
 Parameter writing regulation table definition when an auxiliary function is executed
----------------------------------------------------------------------------*/
#define FUNWRTDISFN_MAX 20				/* FnMaximum number of registered numbers					*/
#define FUNWRTDISPN_MAX 40				/* PnMaximum number of registered numbers					*/

typedef struct FUNWRTDISFNPNTbl {
	USHORT FnNo ;						/* Writing prohibited targetFnNumber				*/
	USHORT FnMode ;						/* Writing prohibited register number		*/
	USHORT PnNo[FUNWRTDISPN_MAX] ;		/* Writing prohibited targetPnNumber				*/
} FUNWRTDISFNPNTbl ;



/****************************************************************************
 Structure of the Function Table Attribute
*****************************************************************************/
typedef PRM_RSLT (*FUN_ENTER_CALLBACK)(FUNEXE*, void*);
typedef PRM_RSLT (*FUN_EXE_CALLBACK)(FUNEXE*, void*);
typedef void (*FUN_LEAVE_CALLBACK)(FUNEXE*, void*);
//typedef LONG (*FUN_INIT_CALLBACK)(void*);

typedef struct	FUNDEF_ {
	UINT8				RunLevel;		/* No.1/2 OpeReg ParaRun(1(No.1)/2(No.2)/0(NG))		*/
	UINT8				ParaTrace;		/* Executable mode channel bits	*/
	UINT8				AxCommon;		/* Access lock to all axes 	*/
	UINT8				ExePrmProhibit;	/* Access lock in Parameter write prohibited mode */
	UINT8				AccessLevel;	/* AccessLevel(USR1/USR2/etc)	*/
	CHAR				SName[12];		/* FnXXX Short Name for Display	*/
	FUN_ENTER_CALLBACK	EnterFunc;		/* Begin Function for FnFunc */
	FUN_EXE_CALLBACK	ExecuteFunc;	/* Execute Function for FnFunc */
	FUN_LEAVE_CALLBACK	LeaveFunc;		/* End Function for FnFunc	*/
/*--------------------------------------------------------------------------------------------------*/
	LONG				(*OpeBeginFunc)(void *Fmsg);		/* Operator Begin Function				*/
	LONG				(*OpeMainFunc)(void *Fmsg);		/* Operator Main Function				*/
	void				(*OpeEndFunc) (void *Fmsg);		/* Operator End Function				*/
/*--------------------------------------------------------------------------------------------------*/
//	FUN_INIT_CALLBACK	InitFunc;		/* Initialize Function for FnFunc */
} FUNDEF;
typedef const FUNDEF	CFUNDEF;		/* const FUNDEF Type Definition	*/
typedef CFUNDEF*		CPFUNDEF;		/* const FUNDEF Type Definition	*/

typedef struct FUNTBL_ {
	USHORT	FnNo;					/* FnNo.						*/
	USHORT	FnMode;					/* FnMode for Register I/F		*/
	CFUNDEF	*FunDefAttr;			/* Pointer to FUNDEF(FnFuncDef)	*/
} FUNTBL;
typedef const FUNTBL	CFUNTBL;	/* const FUNTBL Type Definition	*/
typedef CFUNTBL*		CPFUNTBL;	/* const FUNTBL Type Definition	*/




/****************************************************************************
 Structure of the Utility Function Manager Handle
*****************************************************************************/
typedef struct FUN_AXCOMMON {
	ULONG		TraceMode;				/* Trace mode */
	BOOL		modeLock;				/* Operation mode locked */
	UINT		AccessLevel;			/* Access level */
} FUN_AXCOMMON;

typedef struct FUN_MANAGER {
	CPFUNTBL	FunTable;			/* Function Table */
	UINT		NumOfFunTable;		/* Number of the Function Table list */

	void		*CallbackParam;		/* Parameters for callback function */
	FUN_AXCOMMON *AxCommon;			/* All axes common flag information */
	void		*RegManager;		/* Register manager handle pointer */

	CPFUNTBL	cFunMode[FUN_MODE_NUM];		/* Currently executingFunctionTablepointer */
	FUNEXE		FunExeState[FUN_MODE_NUM];	/* Execution status */
} FUN_MANAGER;



/****************************************************************************
 API functions
*****************************************************************************/
void FunInitialize(FUN_MANAGER *FunMngr, void *RegMngr,
				   FUN_AXCOMMON *FunAxCommon, CPFUNTBL FunTable, UINT TableNum);
PRM_RSLT FunSetOpeModeRegister(FUN_MANAGER *FunMngr, FUN_EXECH ch, ULONG mode, void* ClbkParam);
PRM_RSLT FunSetTraceModeRegister(FUN_MANAGER *FunMngr, ULONG mode);
PRM_RSLT FunSetOpeCommandRegister(FUN_MANAGER *FunMngr, FUN_EXECH ch, ULONG cmd);
void FunRunTimeService(FUN_MANAGER *FunMngr, FUN_EXECH ch);
USHORT FunGetOpeModeRegister(FUN_MANAGER *FunMngr, FUN_EXECH ch);



#endif //__FUN_MANAGER_H


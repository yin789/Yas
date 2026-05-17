/****************************************************************************
Description	: Alarm Manager
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
I.M.   2010.11.02     created

****************************************************************************/
#ifndef ALARM_MANAGER_H

#include "alarm.h"
#include "EepromIF.h"
#include "EncData.h"

/*****************************************************************************
*		Alarm management table count definition
*****************************************************************************/
#define	ALM_BITTBL_NUM		8	/* Number of long words in alarm registration bit table	*/
#define	KPX_ALMTBLMAX	32					/* Maximum number of alarm registration tables	*/
#define	KPX_ALMTRCMAX	10					/* Maximum number of alarm history tables	*/
#define KPX_ALMDEFMAX	(32 * ALM_BITTBL_NUM)	/* Maximum number of alarm definitions	*/
#define KPX_WRNDEFMAX	(32 * WRN_BITTBL_NUM)	/* Maximum number of warning definitions	*/


/*****************************************************************************
*	AlarmHistoryManagement structure definition
*****************************************************************************/
/* Alarm Table	*/
typedef struct ALM_TBL {
	ALMID_T			AlmId;					/* Alarm ID					*/
	CPALM_TBL_DEF	AlmDef;					/* Alarm definition					*/
	ULONG			TmStamp;				/* Alarm detection timestamp	*/
} ALM_TBL;

/* Alarm Trace Header */
typedef union ALM_TRACE_HDR {
	USHORT			w;
	struct {
		UCHAR		Idx;
		UCHAR		Cnt;
	} b;
} ALM_TRACE_HDR;

/* Alarm Trace Data Table */
typedef struct ALM_TRACE_DATA {
	USHORT			AlmCode;				/* Alarm code				*/
	ULONG			TmStamp;				/* Alarm detection timestamp	*/
} ALM_TRACE_DATA;

/* Alarm Management Data */
typedef	struct ALM_HISTORY {
	UINT			AlmCnt;					/* Number of alarms(Occurring)			*/
	UINT			LstAlmCnt;				/* History registration number	<S0FF>			*/
//	UINT			LstAlmDetCnt;			/* Number of alarm occurrences	<S0FF>		*/

	ALM_TBL			AlmTbl[KPX_ALMTBLMAX];	/* Alarm registration table(A&WShared)*/

	ALM_TRACE_HDR	AlmTrcHdr;				/* Alarm historyIndex&Count		*/
	ALM_TRACE_DATA	AlmTrc[KPX_ALMTRCMAX];	/* Alarm history table			*/
	ALM_TRACE_DATA	*AlmTrcSp;				/* Alarm history previous storage pointer	*/

	ULONG			EepAlmOLInfo;			/* Alarm history overload information */
} ALM_HISTORY;


/*****************************************************************************
*	Alarm management IF structure definition
*****************************************************************************/
/* Extended alarm information definition for communication */
typedef struct ALM_EX_INFO {
	BOOL	DevAlm;						/*	Equipment alarm */
	BOOL	DevWrn;						/*	Equipment warning	*/
	BOOL	CmdAlm;						/*	Command alarm */
	BOOL	CommAlm;					/*	Communication alarm */
} ALM_EX_INFO;

/*	<S0FF>	*/
typedef struct ALM_STATUS {
	BOOL	AlmFlag			:1;			/* Alarm occurrence flag				*/
	BOOL	WrnFlag			:1;			/* Warning is occurring flag			*/
	BOOL	MencDataNG		:1;			/* Invalid motor encoder data		*/
	BOOL	PosDataNG		:1;			/* Position data invalid					*/

	BOOL	DevAlm			:1;			/*	Equipment alarm */
	BOOL	DevWrn			:1;			/*	Equipment warning	*/
	BOOL	CmdAlm			:1;			/*	Command alarm */
	BOOL	CommAlm			:1;			/*	Communication alarm */

	UINT	StopMode		:4;
	UINT	AlmOLInfo		:4;

	USHORT	AlmCode;					/* Alarm code information				*/
} ALM_STATUS;


/* Parameter error information definition	*/
typedef	struct PRMERR {
	UCHAR	ErrFlag;					/* Error flag(For online)		*/
	UCHAR	ErrCount;					/* Error count(For offline use)		*/
	USHORT	ErrPrmNo;					/* Parameter number(Parameter error)	*/
} PRMERR;

/* Alarm code information definition	*//* <S04B> */
typedef	struct ALM_CODE_OUT {
	BOOL	Alo1;						/* ALO1 */
	BOOL	Alo2;						/* ALO2 */
	BOOL	Alo3;						/* ALO3 */
} ALM_CODE_OUT;


/* Alarm management IF data definition */
typedef	struct ALMMAN_HANDLE {
/* Public member */
	BOOL			AlmRst;				/* ALMReset request					*/
	BOOL			WrnRst;				/* WarningReset request				*/
	BOOL			NetCmdAlmRst;		/* For communicationCMD_ALMReset request		*/
	BOOL			AlmEepRst;			/* EEPROMAlarm history requestFlag		*/

	ALM_STATUS		Status;				/* alarm status <S0FF>				*/
	BOOL			AlmReady;			/* Alarm ready					*/
	BOOL			WrnFlagX;			/* <S0FF>							*/
//<S0FF>	BOOL			AlmFlag;			/* Alarm occurrence flag				*/
//<S0FF>	BOOL			WrnFlag;			/* Warning is occurring flag			*/
//<S0FF>	BOOL			MencDataNG;			/* Invalid motor encoder data		*/
//<S0FF>	BOOL			PosDataNG;			/* Position data invalid					*/
//<S0FF>	ALM_EX_INFO		AlmExInfo;			/* Extended alarm information				*/
//<S0FF>	ULONG			AlmOLInfo;			/* Alarm history overload information			*/
//<S0FF>	ULONG			StopMode;			/* Stopping method when an alarm occurs			*/

	USHORT			AlmCode;			/* Alarm code information				*/
//	USHORT			AlmCodeOut;			/* Alarm code signal output			*//* <S04B> */
	ALM_CODE_OUT	AlmCodeOut;			/* Alarm code signal output			*//* <S04B> */
	PRMERR			PrmErrInfo;			/* Parameter error information				*/
	ALM_HISTORY		Trace;				/* Alarm trace information			*/

/* Private member(do not write these values from other modules) */
	ULONG			AlmSet[ALM_BITTBL_NUM];		/* Alarm registration bit		*/
	ULONG			AlmSetMsk[ALM_BITTBL_NUM];	/* Alarm mask bit		*/
	ULONG			AlmRstMsk[ALM_BITTBL_NUM];	/* Alarm reset disabled bit*/
	ULONG			WrnRstMsk[ALM_BITTBL_NUM];	/* Alarm reset disabled bit*/

	ULONG			StopModeParam;		/* Stopping method when an alarm occurs			*/
	CPALM_TBL_DEF	DefTable;			/* Pointer to Alarm table */
	UINT			TableEntryNum;		/* Number of Table entry */
	void			*mutex;				/* for Interlock */

	UINT				myAsicNo;		/* axis number */
//	EEP_DEVICE_HANDLE	*EepDevice;		/* Eeprom Device handle *//* <Oka02S1D2> */
	EEP_QUE_BUFFER		EepQueBuf[32];	/* Eeprom Queue buffer */
	EEP_QUE_HANDLE		EepQue;			/* Eeprom Queue */
	volatile BOOL		AlmEepRstBusy;	/* EEPROMAlarm historyBusyFlag		*/

	USHORT				IntAlmId;		/* Startup detection alarmID */
	BOOL				IntAlmEnable;	/* Startup detection alarm enabled */
	BOOL				OCAlmOccurFlag1;/* Overcurrent(OC1)Alarm(A.100)Occurrence flag(For internal output signal monitor) */	/* <S1A0> *//* <S1C9> */
	BOOL				OCAlmOccurFlag2;/* Overcurrent(OC2)Alarm(A.100)Occurrence flag(For internal output signal monitor) */	/* <S1C9> */
	BOOL				WrnOutSel;		/* Warning code output selection */	/* <S1A8> */

	USHORT				LtAlmReq;		/* Alarm latchEEPROMWrite request				 <S1CF> */
	USHORT				LtDtWtCt;		/* Alarm latch data writing counter flag	 <S1CF> */
	ULONG				LtRomAdr;		/* WriteEEPROMAddress						 <S1CF> */
	USHORT				*LtMemAdr;		/* Write memory address						 <S1CF> */
	USHORT				*AlmCdAdr;		/* Alarm code storage address					 <S1CF> */
	ULONG				*TmStpAdr;		/* Alarm timestamp storage address			 <S1CF> */

	void				*hAlmLatchHndl;	/* Alarm latch data						 <S212> */

	BOOL				AlmClear;		/* Request to clear specific alarm						 <S226> */
	ALMID_T				ClrAlmId;		/* Clear alarm ID							 <S226> */

} ALMMAN_HANDLE;



/* API's */

/* for all axises */
void ALMInitGlobalHandle(ALMMAN_HANDLE **AxisTopHandle, LONG AxisNum);
void ALMSetGlobalAlarm(ALMID_T alm_id);
void ALMClearGlobalAlarm(ALMID_T alm_id);
BOOL ALMCheckGlobalAlarm(ALMID_T alm_id);

/* for an axis */
void ALMInitAlarmManager(ALMMAN_HANDLE *AlmMngr, CPALM_TBL_DEF AlmDefTbl,
						 UINT AlmDefCntx, UINT AxisNumber);
ULONG ALMStartupProcedure(ALMMAN_HANDLE *AlmMngr, UCHAR StopModeParam,
										LONG AxisMotType, LONG AbsoEncType);
void ALMRuntimeProcess(ALMMAN_HANDLE *AlmMngr, UCHAR StopModeParam);
void ALMRuntimeProcess2(ALMMAN_HANDLE *AlmMngr, UCHAR StopModeParam);		/* <S226> */
void ALMHistoryWriteService(ALMMAN_HANDLE *AlmMngr);

CPALM_TBL_DEF ALMSetServoAlarm(ALMMAN_HANDLE *AlmMngr, ALMID_T alm_id);
void ALMSetMask(ALMMAN_HANDLE *AlmMngr, ALMID_T alm_id, BOOL MaskSet);
BOOL ALMCheckEachState(ALMMAN_HANDLE *AlmMngr, ALMID_T alm_id);
BOOL ALMCheckSetMask(ALMMAN_HANDLE *AlmMngr, ALMID_T alm_id);	/* @@ add */
void ALMSetPramError(ALMMAN_HANDLE *AlmMngr, USHORT ErrPrmNo);
void ALMClearWarning(ALMMAN_HANDLE *AlmMngr, ALMID_T alm_id);
BOOL ALMGetOtherAlarmChk(ALMMAN_HANDLE *AlmMngr, ALMID_T alm_id);
ULONG ALMGetStatus(ALMMAN_HANDLE *AlmMngr, BOOL *AlmFlag, BOOL *WrnFlag);
BOOL ALMGetExNetStatus(ALMMAN_HANDLE *AlmMngr, ALM_EX_INFO *AlmExInfo);
void ALMGetNetAlarmTableInfo(ALMMAN_HANDLE *AlmMngr, USHORT *AlmInfo, UINT Index);
void ALMGetNetAlarmTraceInfo(ALMMAN_HANDLE *AlmMngr, USHORT *AlmInfo, UINT Index);
USHORT ALMGetAlarmHistory(ALMMAN_HANDLE *AlmMngr, UINT Index, ULONG *TimeStamp);
LONG ALMGetNetAlarmTableCode(ALMMAN_HANDLE *AlmMngr, UCHAR *AlmCode, LONG MaxInfo);
LONG ALMGetNetAlarmTableDetailCode(ALMMAN_HANDLE *AlmMngr, USHORT *AlmCode, LONG MaxInfo);
LONG ALMGetNetAlarmNum(ALMMAN_HANDLE *AlmMngr, USHORT *AlmCode, LONG MaxInfo);
LONG ALMGetNetWarningNum(ALMMAN_HANDLE *AlmMngr, USHORT *AlmCode, LONG MaxInfo);
LONG ALMGetNetAlarmTraceCode(ALMMAN_HANDLE *AlmMngr, UCHAR *AlmCode, LONG MaxInfo);
LONG ALMGetNetAlarmTraceDetailCode(ALMMAN_HANDLE *AlmMngr, USHORT *AlmCode, LONG MaxInfo);
LONG ALMGetAlarmInfo(ALMMAN_HANDLE *AlmMngr);
void ALMSetAlarmResetRequest(ALMMAN_HANDLE *AlmMngr);
void ALMSetWarningResetRequest(ALMMAN_HANDLE *AlmMngr);
BOOL ALMGetAlarmResetStatus(ALMMAN_HANDLE *AlmMngr);
BOOL ALMGetWarningResetStatus(ALMMAN_HANDLE *AlmMngr);
void ALMSetAlarmHistResetRequest(ALMMAN_HANDLE *AlmMngr);
BOOL ALMGetAlarmHistResetStatus(ALMMAN_HANDLE *AlmMngr);
void SetOutputAloSignals( ALMMAN_HANDLE *AlmMngr );	/* <S1A8> */
void LcdEep2EepRomWrite( UINT devId );	/* <S208> */

#define ALARM_MANAGER_H
#endif /* ALARM_MANAGER_H */

#ifndef __TABLE_DRIVE_H /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		TableDrive.h : For table operation	module definition												*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2011.02.18	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "CtrlLoopHandler.h"
#include "MfcHandler.h"
#include "PrmAccessIf.h"
#include "DataTraceManager.h"



/****************************************************************************************************/
/*																									*/
/*		Structure Definition																		*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation zero control command definition																*/
/*--------------------------------------------------------------------------------------------------*/
typedef union {							/* Table operation control command definition									*/
	struct	{							/* Zero control mode member definition									*/
		UCHAR	zm;						/* Zero control mode											*/
		UCHAR	zmchg;					/* Zero control mode change information									*/
		UCHAR	zmlast;					/* Preliminary														*/
		UCHAR	Dummy;					/* Preliminary														*/
	} b;
	/*----------------------------------------------------------------------------------------------*/
	ULONG	dw;							/* For control mode editing(Copy,etc)								*/
} TBLZMD;


/*--------------------------------------------------------------------------------------------------*/
/*		Definition for table operation execution control																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct TBLDRIVE {
	struct {
		USHORT	TblDriveDataSize;		/* 0x2101 : Table operation data length							*/
		USHORT	TblDriveSampIntv;		/* 0x2103 : Table operation sampling interval					*/
		USHORT	TblMode;				/* 0x2300 : Table operation control mode							*/
		USHORT	TblLimit;				/* 0x2301 : Table operation rotation amount(Movement amount)Limit				*/
		USHORT	TblState;				/* 0x2302 : Table operation complete								*/
		USHORT	TblFuncSel;				/* 0x2303 : Table operation function selection							*/
	} OpeReg;
	/*----------------------------------------------------------------------------------------------*/
	struct {
		UCHAR	TblPrmOk;				/* Table operation parameter OK								*/
		UCHAR	TblMode;				/* Table operation mode										*/
		USHORT	CtrlMode;				/* Table operation control mode									*/
	/*----------------------------------------------------------------------------------------------*/
		UCHAR	TrqInputPos;			/* Table operation torque input position								*/
		UCHAR	TuneLessDisable;		/* Disable adjustmentless control											*/
		UCHAR	EGearDisable;			/* Disable electronic gear ratio											*/
		UCHAR	PcmdFilDisable;			/* Position command filter disabled										*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	MotPulse;				/* Number of motor pulses [pulse/rev, pulse/mm]						*/
		LONG	MtnLimit;				/* Multiturn limit										*/
	/*----------------------------------------------------------------------------------------------*/
		UCHAR	RefType;				/* Command type												*/
		UCHAR	RefIpSft;				/* Number of command interpolation shifts											*/
		USHORT	RefIpNum;				/* Number of command interpolation												*/
		ULONG	TblDataNum;				/* Number of table operation command data								*/
		ULONG	DataSampIntv;			/* Table data sampling interval							*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	KoutDir;				/* Table operation command output gain(Normalized gain + direction)			*/
		LONG	*RefOutPx;				/* Table operation command output pointer								*/
	/*----------------------------------------------------------------------------------------------*/
		void	*TblBuffer;				/* Table operation command buffer									*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {
		TBLZMD	TblZmd;					/* Table operation zero control mode command							*/
		UCHAR	TblCmd;					/* Table operation execution command										*/
		UCHAR	TblStep;				/* Table operation execution step									*/
		USHORT	TblSamp;				/* Table operation sampling									*/
		USHORT	TblIndx;				/* Table operation index									*/
		USHORT	MregCsw;				/* ASICMicro torque filter control(Disable/restore)For				*/
		USHORT	ErrState;				/* Abnormal state													*/
		USHORT	Dummy;					/* For Alignment											*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	MotMtn;					/* Table operation multi-turn			[rev,mm]				*/
		LONG	MotPos;					/* Table operation motor position			[pulse]					*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	TrqRef;					/* Table operation torque command output		[NorTrq]				*/
		LONG	SpdRef;					/* Table operation speed command output			[NorSpd]				*/
		LONG	dPosRef;				/* Table operation position command output			[pulse/scan]			*/
	/*----------------------------------------------------------------------------------------------*/
		LONG	InPosRef;				/* For table operation position command interpolation calculation	[pulse/samp]			*/
		LONG	IxPosRef;				/* For table operation position command interpolation calculation	[pulse/scan]			*/
		LONG	IxPosRem;				/* For table operation position command interpolation calculation	[pulse]					*/
		LONG	WkPosRem;				/* For table operation position command interpolation calculation	[pulse]					*/
	/*----------------------------------------------------------------------------------------------*/
		BOOL	KeepTblMode;			/* Table operation continuation flag									*/
		BOOL	TrqFilDisable;			/* Torque command filter invalidation request								*/
		BOOL	NotchLpassDisable;		/* Notch & low-pass filter disabled request							*/
		LONG	DisTrqInTiming;			/* Disturbance torque command input position									*/
	/*----------------------------------------------------------------------------------------------*/
		BOOL	WithTrcReq;				/* Table operation simultaneous trace execution request								*/
	} var;
	TRCHNDL		*TrcHndl;				/* Data trace handle */
} TBLDRIVE;


typedef	struct TBLCHK_SV_STS {
	BOOL	AlmFlag;				/* Alarm occurrence flag											*/
	BOOL	HwbbSts;				/* HWBBState detection data											*/
	BOOL	MainPowerOn;			/* Main circuitONStatus													*/
	BOOL	OverTrvlSts;			/* OTStatus(OTSequence processing output)									*/
	BOOL	BeComplete;				/* Base enable completion status										*/
	BOOL	TrqClamp;				/* Torque limit flag											*/
	LONG	dMotPos;				/* PositionFBIncrement value[pulse/ScanB]									*/
} TBLCHK_SV_STS;


/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation execution command definition																	*/
/*--------------------------------------------------------------------------------------------------*/
enum TBLCMD_EXE_STS {
	TBLCMD_NOCMD		= 0x00,				/* No command												*/
	TBLCMD_TABLESTART,						/* Table operation processing start command								*/
	TBLCMD_TABLEDRIVE,						/* Table operation execution start command								*/
	TBLCMD_TBLDRV_END,						/* Table operation execution execution completed normally								*/
	TBLCMD_TBLDRV_ERR	= 0x0E,				/* Table operation execution abnormal end								*/
};

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation state definition																		*/
/*--------------------------------------------------------------------------------------------------*/
enum TBLCMD_SERVO_STS {
	TBLCMD_STS_INIT			= 0,			/* Initial state										*/
	TBLCMD_STS_RUN,							/* During table operation										*/
	TBLCMD_STS_FINISH,						/* Table operation complete										*/
	TBLCMD_STS_POSLMT		= 100,			/* Rotation amount/Movement amount limit detection							*/
	TBLCMD_STS_TRQLMT,						/* Torque limit detection										*/
	TBLCMD_STS_BB,							/* Servo off											*/
	TBLCMD_STS_OT,							/* Overtravel										*/
	TBLCMD_STS_POWEROFF,					/* Main circuit power off										*/
	TBLCMD_STS_ALARM,						/* Alarm occurrence											*/
	TBLCMD_STS_HWBB,						/* HWBBStatus												*/
};

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation command type definition																	*/
/*--------------------------------------------------------------------------------------------------*/
enum TBLCMD_REF_TYPE {
	TBLREFTYPE_NOTPOS		= 0x00,			/* Other than position command(Speed,Torque)							*/
	TBLREFTYPE_POSIP1,						/* Position command: No interpolation									*/
	TBLREFTYPE_POSIP2,						/* Position command: Shift interpolation									*/
	TBLREFTYPE_POSIP3,						/* Position command: Division interpolation									*/
};

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation command data length definition																*/
/*--------------------------------------------------------------------------------------------------*/
enum TBLCMD_REF_LENGTH {
	TBLREFLENGTH_NOTDEF		= 0x00,			/* Undefined(SHORT)										*/
	TBLREFLENGTH_15BIT_IO,					/* Data:15bit + IO:1bit									*/
	TBLREFLENGTH_31BIT_IO,					/* Data:31bit + IO:1bit									*/
	TBLREFLENGTH_16BIT,						/* Data:16bit											*/
	TBLREFLENGTH_32BIT,						/* Data:32bit											*/
};

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation selection bit definition																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	TBLFUNCBIT_TUNELESS		0x0100		/* Adjustment-less control											*/
#define	TBLFUNCBIT_EGEAR		0x0200		/* Electronic gear ratio											*/
#define	TBLFUNCBIT_PCMDFIL		0x0400		/* Position command filter										*/


/* Table operation buffer is1axis0x1000[2byte]only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.Σ-Ⅴteeth0xC00[2byte]so there is plenty of room */
//#define	TBL_BUF_SIZE		0x30000		/* Command buffer size for table operation[2byte]				*/
//#define	AX_TBL_BUF_SIZE		0x6000		/* Table operation command buffer size for each axis[2byte]		*/
#define		AX_TBL_BUF_SIZE		0x1000		/* Table operation command buffer size for each axis[2byte]		*/



/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
void		TblDriveInitialize( TBLDRIVE *TblDrive, USHORT *BufferAddress, TRCHNDL *TrcHndl );
void		TblDriveGetTraceRegData( TBLDRIVE *TblDrive, USHORT DataSize, USHORT SampIntv,
																ULONG DataSampIntv, ULONG DataNUm );
void		TblDriveExecute( TBLDRIVE *TblDrive, TBLCHK_SV_STS *TblChkSts,
														POS_MNG_HNDL *PosManager, MFCTRL *MFControl );
PRM_RSLT	TblDriveCalculateParameter( TBLDRIVE *TblDrive, LONG XfbPulse1 );
PRM_RSLT	TblDrivePrepareProc( TBLDRIVE *TblDrive );
PRM_RSLT	TblDriveStartRequest( TBLDRIVE *TblDrive, LONG TblDir );
PRM_RSLT	TblDriveSetStopRequest( TBLDRIVE *TblDrive );
USHORT		TblDriveGetControlMode( TBLDRIVE *TblDrive );
void		TblDriveSetWithTrcReq( TBLDRIVE *TblDrive, BOOL Switch );




/****************************************************************************************************/
#define __TABLE_DRIVE_H
#endif /* __TABLE_DRIVE_H */

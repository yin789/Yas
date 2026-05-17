/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ServoSequence.h : Servo sequence management processing definition												*/
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
#ifndef __BASE_ENABLE_SEQUENCE_H_
#define __BASE_ENABLE_SEQUENCE_H_


//#include "BaseCmnStruct.h"
//#include "alarm.h"
//#include "EncData.h"
#include "HwDeviceIf.h"
#include "PowerManager.h"/*<S201>*/  
                         /*#include "JL086ServoIpReg.h"*/
                         /*#include "BaseCmnStruct.h"*/
                         /*#include "Bprm.h" -------->#include "alarm.h"*/
                         /*                  --------->#include "EncData.h"*/
#include "AlarmManager.h"/*<S201>*/
/*#include "SequenceIo.h" -------------->#include "BaseEnableSequence.h" */

/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Stop mode definition																			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Note: When defining the stop mode,0x0EIt must be defined with the following values.(Relationship with alarm definition).	*/
/*			  For stop modes, the higher the defined value, the higher the priority.										*/
/*--------------------------------------------------------------------------------------------------*/
#define	STOPMODE_NOTSTOP		0x00		/* Non-stop(Do nothing)									*/
#define	STOPMODE_ZEROSTOP		0x01		/* Zero speed stop											*/
#define	STOPMODE_LDECSTOP		0x02		/* Deceleration stop by linear deceleration								*/   /* <S1F5> */
/*--------------------------------------------------------------------------------------------------*/
// #define	STOPMODE_DBSTOPDB		0x04		/* Stop motor with DB & continue DB(Pn001.0==0)				*/  /* <S1F5> */
// #define	STOPMODE_DBSTOPFR		0x05		/* Stop motor with DB & release DB(Pn001.0==1)				*/    /* <S1F5> */
// #define	STOPMODE_FREERUN		0x06		/* Stop free run without using DB(Pn001.0==2)				*/    /* <S1F5> */
#define	STOPMODE_DBSTOPDB		0x08		/* Stop motor with DB & continue DB(Pn001.0==0)				*/   /* <S1F5> */
#define	STOPMODE_DBSTOPFR		0x09		/* Stop motor with DB & release DB(Pn001.0==1)				*/   /* <S1F5> */
#define	STOPMODE_FREERUN		0x10		/* Stop free run without using DB(Pn001.0==2)				*/   /* <S1F5> */
/*--------------------------------------------------------------------------------------------------*/
/* <S1F5> Start */
/*		Gr.2Alarm stop mode definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define	GR2STOPMODE_DEFAULT		0x03		/* Pn001.0(Pn00A.0==0)							*/
#define	GR2STOPMODE_ZSTOPDB		0x04		/* Deceleration and stop &DBStopped state(Pn00A.0==1)						*/
#define	GR2STOPMODE_ZSTOPFR		0x05		/* Deceleration stop & free run state(Pn00A.0==2)					*/
#define	GR2STOPMODE_LDSTOPDB	0x06		/* Linear deceleration stop &DBStopped state(Pn00A.0==3)					*/
#define	GR2STOPMODE_LDSTOPFR	0x07		/* Linear deceleration stop & free run state(Pn00A.0==4)				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Gr2/DBStop mode definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	GR2DBOP_NONE			0x00		/* No DB restriction request										*/
#define	GR2DBOP_FREE			0x02		/* Free run stop requested								*/
/*--------------------------------------------------------------------------------------------------*/
/*		FSTPStop mode definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	FSTOPMODE_DEFAULT		0x03		/* Pn001.0(Pn00A.1==0)							*/
#define	FSTOPMODE_ZSTOPDB		0x04		/* Deceleration and stop &DBStopped state(Pn00A.1==1)						*/
#define	FSTOPMODE_ZSTOPFR		0x05		/* Deceleration stop & free run state(Pn00A.1==2)					*/
#define	FSTOPMODE_LDSTOPDB		0x06		/* Linear deceleration stop &DBStopped state(Pn00A.1==3)					*/
#define	FSTOPMODE_LDSTOPFR		0x07		/* Linear deceleration stop & free run state(Pn00A.1==4)				*/
/*--------------------------------------------------------------------------------------------------*/
/*		FSTP/DBStop mode definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	FSTPDBOP_NONE			0x00		/* No DB restriction request										*/
#define	FSTPDBOP_FREE			0x02		/* Free run stop requested								*/
/*--------------------------------------------------------------------------------------------------*/
/* <S1F5> End */
/*		OT																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	OTSTOPMODE_DEFAULT		0x00		/* Pn001.0										*/
#define	OTSTOPMODE_ZSTOPZC		0x01		/* Deceleration and stop & servo lock								*/
#define	OTSTOPMODE_ZSTOPFR		0x02		/* Deceleration stop & free run									*/
#define	OTSTOPMODE_LDSTOPZC		0x03		/* Linear deceleration stop & servo lock							*/      /* <S1F5> */
#define	OTSTOPMODE_LDSTOPFR		0x04		/* Linear deceleration stop & free run								*/     /* <S1F5> */
/*--------------------------------------------------------------------------------------------------*/
/*		OT/DBStop mode definition																			*/
/*--------------------------------------------------------------------------------------------------*/
#define	OTDBOP_NONE				0x00		/* No DB restriction request										*/
#define	OTDBOP_DEF				0x01		/* OT standard stop											*/
#define	OTDBOP_FREE				0x02		/* Free run stop requested								*/
/*--------------------------------------------------------------------------------------------------*/
/*		OTDBControl request type																		*/
/*--------------------------------------------------------------------------------------------------*/
//#define	OTDBOP_NONE				0x00		/* No DB restriction request										*/
//#define	OTDBOP_DEF				0x01		/* OT standard stop											*/
//#define	OTDBOP_FREE				0x02		/* Free run stop requested								*/


/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm stop control structure definition																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct ALM_STOP {
	struct {
		BOOL		AlmZeroStop;			/* Alarm Zero Stop Request								*/
		BOOL		AlmZstopFinish;			/* Alarm Zero Stop Finish								*/
		LONG		AlmStopMode;			/* Alarm Stop Mode for Sequence							*/
		BOOL		Gr2TrqLimit;			/* Alarm Stop Torque Limit Request						*/    /* <S1F5> */
		BOOL		Gr2ZeroStop;			/* Alarm Stop Zero Stop  Request						*/   /* <S1F5> */
		BOOL		Gr2LinDecStop;			/* Alarm Stop Linearity Deceleration Stop  Request		*/   /* <S1F5> */
		LONG		Gr2DbopReq;				/* Alarm Stop DB Operate Request						*/   /* <S1F5> */
		LONG		Gr2StopStep;			/* Alarm Stop Step										*/   /* <S1F5> */
	} var;
	struct {
//		BOOL		G2AlmStopMode;			/* G2How to stop the alarm									*/   /* <S1F5> */
		LONG		G2AlmStopMode;			/* G2How to stop the alarm									*/   /* <S1F5> */
		LONG		StopMode;				/* Servo off and stop mode when alarm occurs				*/
	} conf;
} ALM_STOP;

/* <S1F5> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Forced stop control structure definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct FORCE_STOP {
	struct {
		LONG		FstpMode;				/* Force Stop Mode for Sequence							*/
		BOOL		FstpTrqLimit;			/* Force Stop Torque Limit Request						*/
		BOOL		FstpZeroStop;			/* Force Stop Zero Stop  Request						*/
		BOOL		FstpLinDecStop;			/* Force Stop Linearity Deceleration Stop  Request		*/
		LONG		FstpDbopReq;			/* Force Stop DB Operate Request						*/
		LONG		FstpStopStep;			/* Force Stop Step										*/
	} var;
	struct {
		LONG		ForceStopMode;			/* Forced stop stopping method									*/
	} conf;
} FORCE_STOP;
/* <S1F5> End */

/*--------------------------------------------------------------------------------------------------*/
/*		OTControl structure definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct OT_SEQ_DATA {
	struct {								/* OTSequence constant definition								*/
		BOOL			OtWarningSw;			/* OTWarning detection selection								*/
		LONG			OtStopMode;				/* Overtravel(OT)					*/
	} conf;
	struct {								/* OTSequence variable definition								*/
		BOOL			OverTrvlSts;			/* OTStatus(OTSequence processing output)						*/
		BOOL			OtBeReq;				/* OtSequence BaseEnable Request					*/
		BOOL			OtTrqLimit;				/* OT Sequence Torque Limit Request					*/
		BOOL			OtZeroStop;				/* OT Sequence Zero Stop  Request					*/
		BOOL			OtLinDecStop;			/* OT Sequence Linearity Deceleration Stop  Request	*/    /* <S1F5> */
		BOOL			OtZeroClamp;			/* OT Sequence Zero Clamp Request					*/
		LONG			OtStep;					/* OT Sequence Step									*/
		LONG			OtWait;					/* OT Sequence Wait Counter							*/
		LONG			OtOnCnt;				/* OT Sequence OT-ON Wait Counter					*/
		LONG			OtOffCnt;				/* OT Sequence OT-OFF Wait Counter					*/
		LONG			OtDbopReq;				/* OT Sequence DB Operate Request					*/
		BOOL			OtWarningDet;			/* OT Warning Detect								*/
		LONG			OtWarngWaitCnt;			/* OT Sequence Warning Clear Wait Count				*/
		LONG			CtrlMode2;				/* OT Sequence OT CtrlMode for M3 I/F				*/
	} var;
} OT_SEQ_DATA;


/*--------------------------------------------------------------------------------------------------*/
/*		Brake control structure definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct BK_SEQ_DATA {
	struct {								/* Brake control constant definition								*/
		LONG			BkonSpdLevel;			/* Brake command output speed level(Pn507)	[2^24/OS]	*/
		LONG			BkonWaitTime;			/* Servo off brake command wait time(Pn508)	 [ms]	*/
		LONG			BkonDelayTime;			/* Brake command servo off delay time(Pn506)	 [ms]	*/
	} conf;
	struct {								/* Brake control variable definition								*/
		BOOL			BkBeReq;				/* BkSequence BaseEnable Request						*/
		BOOL			Brake;					/* BK Sequence Brake Request						*/
		BOOL			BkZeroStop;				/* BK Sequence Zero Stop  Request					*/
		BOOL			BkLinDecStop;			/* BK Sequence Linearity Deceleration Stop Request	*/	/* <S1F5> */
		LONG			BkStep;					/* BK Sequence Step									*/
		LONG			BkWait;					/* BK Sequence Wait Counter							*/
	} var;
} BK_SEQ_DATA;


/*--------------------------------------------------------------------------------------------------*/
/*		DBControl structure definition																			*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct DB_SEQ_DATA {
	LONG			DbStep;					/* DB Sequence Step										*/
	LONG			DbWait;					/* DB Sequence Wait Counter								*/
	BOOL			DbBeReq;				/* DbSequence BaseEnable Request						*/
	USHORT	        NegPumpState;			/* Main circuitOFFtimeIPM NSide arm control status	<S201>				   */
} DB_SEQ_DATA;


/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for zero clamp control																*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _ZC_CTRL_PRM {				/* Zero Clamp Parameter	<S00C>							*/
	LONG			ZcSpdLevel;				/* ZC Speed Level [2^24/OS]								*/
} ZC_CTRL_PRM;
typedef	struct ZC_CTRL_DATA {
	ZC_CTRL_PRM		P;						/* Zero Clamp Parameter									*/
/*--------------------------------------------------------------------------------------------------*/
	struct {								/* Zero Clamp Flags										*/
		UCHAR Reserve0;						/*														*/
		UCHAR Reserve1;						/*														*/
		UCHAR Reserve2;						/*														*/
		UCHAR Reserve3;						/*														*/
	} f;
	BOOL			ZcReqSignal;			/* ZC Sequence Req. Input Signal						*/
	BOOL			ZcZeroStop;				/* ZC Sequence Zero Stop Request						*/
	BOOL			ZcZeroClamp;			/* ZC Sequence Zero Clamp Request						*/
	LONG			ZcStep;					/* ZC Sequence Step										*/
	LONG			ZcWait;					/* ZC Sequence Wait Counter								*/
} ZC_CTRL_DATA;

/*--------------------------------------------------------------------------------------------------*/
/*		PUMP ONControl structure definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct PUMP_CTR_STR {
	BOOL			PumpOnReq;				/* Charge Pump Control Request							*/
	BOOL			AdjPumponReq;			/* Adjust Mode PUMPON Request							*/
	BOOL			PumpOnComp;				/* Charge Pump Complete									*/
	SHORT			SvonPumpCnt;			/* Charge pump for before Base-Enable					*/
//	SHORT			SvonRelayCnt;			/* Rush prevention relay for Pre-charge pump counter				*/
	ULONG			TimerCnt;				/* Charge pump Timer Counter */
	UINT			PumpOnSeq;				/* Charge pump Sequence Number */
	USHORT          	NegPumpCnt;             /* Charge pump for before Power-On <S201>				*/
} PUMP_CTR_STR;

/*--------------------------------------------------------------------------------------------------*/
/*		Servo operation sequence variable data pointer definition(For argument setting)										*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct BE_SEQ_HNDL {
	ALM_STOP		AlmStop;				/* Alarm stop data									*/
	FORCE_STOP		ForceStop;				/* Forced stop data										*/   /* <S1F5> */
	BK_SEQ_DATA		BkSeqData;				/* BKSequence data									*/
	OT_SEQ_DATA		OtSeqData;				/* OTSequence data									*/
	DB_SEQ_DATA		DbSeqData;				/* DBSequence data									*/
	ZC_CTRL_DATA	ZcCtrlData;				/* Data for zero clamp control								*/
	PUMP_CTR_STR	ChargePump;				/* PUMP ON							*/

	BOOL			SvonReq;				/* Servo ON Request(M-III/LCD OP)				*/
	BOOL			SenStatus;				/* SEN Signal											*/
	BOOL			BeReqSeqOut;			/* BaseEnable Request(Sequence processing output)				*/
	LONG			StopMode;				/* Stop Mode											*/
	BOOL			DbOn;					/* DB ON Flag											*/
	BOOL			SvonEnable;				/* ServoOn Enable										*/
	BOOL			SvReadyPhaseIgnore;		/* Servo Ready (Ignore Phase)							*/
	BOOL			ServoReady;				/* Servo ready											*/
//	BOOL			EncReady;				/* Encoder Ready										*/
//	BOOL			ScanCEncDataNG;			/* Encoder data invalid flag(0:Enabled/1:Disabled)			*/
	BOOL			SoftResetReq;			/* Soft reset request				<S076>			*/
	BOOL			LdstpTimeZero;			/* Deceleration time for linear deceleration and stop(Pn30A)=0			*/     /* <S1F5> */
} BE_SEQ_HNDL;

/*--------------------------------------------------------------------------------------------------*/
/*		HWBBVariable definition for detection  <S201> Change definition location							     		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct DET_HWBB_STS {
	BOOL			HwbbState;				/* Hard-wired Baseblock Status							*//* <HWBB> */
	BOOL			Bbon1;					/* BBON1 Signal											*//* <HWBB> */
	BOOL			Bbon2;					/* BBON2 Signal											*//* <HWBB> */
	BOOL			Bbon1Edge;				/* BBON1 Signal Rising Edge								*//* <HWBB> */
	BOOL			Bbon2Edge;				/* BBON2 Signal Rising Edge								*//* <HWBB> */
	BOOL			GateDrvDefect;			/* Gate Drive Defect Status Flag						*//* <HWBB> */
	BOOL			LackOneBbon;			/* One BBON Signal Input Only							*//* <HWBB> */
	BOOL			hwbbsupport;			/* HWBB Support flag(PnE0D.bit9 = 1Valid in)				*//* <HWBB> */
} DET_HWBB_STS;

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void BeSeqMakeServoReadySignal( BE_SEQ_HNDL *BeSeq, BOOL ready_cond, LONG PhaseReady );
ALMID_T	BeSeqMakeServoOnRequest( BE_SEQ_HNDL *BeSeq, LONG CmdSeqBit,
       	                          CTRLMD CtrlMode, FUN_CMN_CONTROL *FnCmnCtrl, MENCV *MencV );
void BeSeqDBControlProcess( BOOL DbOn, LONG RlyDbWay, USHORT faultState, ASIC_HW_IF_STR *pAsicHwReg,
							BOOL MotorLessTestMode );	/* <S1B7> */
ULONG BeSeqControlModeManager( BE_SEQ_HNDL *BeSeq, FUN_CMN_CONTROL *FnCmnCtrl );

// void BeSeqInputAlmStopMode( ALM_STOP *AlmStop, LONG AlmStopMode,     /* <S1F5> */
//                            CTRLMD CtrlMode, BOOL PhaseReady, BOOL HwbbSts, BOOL MotStop );  /* <S1F5> */
void BeSeqInputAlmStopMode( BE_SEQ_HNDL *BeSeq, LONG AlmStopMode,     /* <S1F5> */
                            CTRLMD CtrlMode, BOOL PhaseReady, SEQ_CTRL_OUT *Cout );    /* <S1F5> */
void BeSeqForceStopMode( BE_SEQ_HNDL *BeSeq, CTRLMD CtrlMode, BOOL MotStop, BOOL FstpSts );    /* <S1F5> */
BOOL BeSeqMechaBrakeSequence( BE_SEQ_HNDL *BeSeq, CTRLMD CtrlMode,
    	                       LONG AbsMotSpd, BOOL HwbbStatus, USHORT faultState,
							   BOOL BeComplete, BOOL MotorLessTestMode );	/* <S1B7> */
// BOOL BeSeqOverTravelSequence( OT_SEQ_DATA *OverTravelSeq, LONG CtrlMode, BOOL MotStop, BOOL BeComplete );   /* <S1F5> */
BOOL BeSeqOverTravelSequence( OT_SEQ_DATA *OverTravelSeq, LONG CtrlMode, BOOL MotStop,      /* <S1F5> */
                                                          BOOL BeComplete, BOOL FstpDecTimeZero );     /* <S1F5> */
BOOL BeSeqDetectOTStatus( OT_SEQ_DATA *OverTravelSeq, BASE_CTRL_OUT *BaseCtrlOut, BOOL PotSig, BOOL NotSig );
LONG BeSeqStopModeManager( BE_SEQ_HNDL *BeSeq );
BOOL BeSeqDynamicBrakeSequence( BE_SEQ_HNDL *BeSeq, ASICS *SvAsicRegs, LONG MotStop, BOOL EncDataNG,
								BOOL MotorLessTestMode );	/* <S1B7> */
void BeSeqChargePumpControl( ASICS *SvAsicRegs, PUMP_CTR_STR *workptr, BOOL OCState,
								BOOL MotorLessTestMode,BOOL NegPumpReq );	/* <S1B7> *//*<S201>*/
void BeNegativePumpSequence( POWER_STATUS   *CoutK, ALMMAN_HANDLE *Alm,
						     DET_HWBB_STS   *DetHwbb,CHECK_MAIN_POWER *PowerChecker,
						     DB_SEQ_DATA   *SvSeqV,  SEQ_CTRL_OUT *CoutV1, ASIC_HW_IF_STR* AsicHwReg);/* <S201>*/



#endif /* __BASE_ENABLE_SEQUENCE_H_ */
/***************************************** end of file **********************************************/

#ifndef __ADV_AUTO_TUNING_H
#define __ADV_AUTO_TUNING_H
/****************************************************************************
Description	: Advanced Auto Tuning
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
----------------------------------------------------------------------------
*****************************************************************************/
#include "Basedef.h"
#include "SystemSetting.h"
#include "FnPrgJog.h"
#include "PowerManager.h"
#include "CheckMotSts.h"
#include "CheckAlarm.h"
#include "RemVibFreq.h"
#include "AlarmManager.h"

/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning gain change setting constant definition											*/
/*--------------------------------------------------------------------------------------------------*/
#define		GAIN_DW					0		/* Gain down											*/
#define		GAIN_UP					1		/* Gain up											*/
#define		GAIN_BAL				2		/* Gain balance 1										*/
#define		GAIN_BAL2				3		/* Gain balance 2										*/
#define		GAIN_CAL				4		/* Gain calculation											*/
#define		GAIN_BAL3				5		/* Gain balance 3										*/
#define		GAIN_BAL4				6		/* Gain balance 4										*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning parameter constant definition												*/
/*--------------------------------------------------------------------------------------------------*/
#define		RESGMAX					200		/* Command response oscillation detection threshold [%]								*/
#define		STPVIBDET				300		/* Vibration detection threshold when stopped [%]								*/
#define		STPVIBLMT				5		/* Vibration detection limit when stopped[%]							*/
#define		STPTRQFIL				1000	/* Torque high-pass filter frequency when stopping [0.1Hz]			*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning error status constant definition											*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_STS_INIT			0x00	/* Initial/interruption										*/
#define		ADAT_STS_DONE			0x01	/* Tuning completed										*/
#define		ADAT_STS_ERR			0x11	/* Tuning failure										*/
#define		ADAT_STS_NG				0x21	/* Operation failure												*/
#define		ADAT_STS_ABORT			0x22	/* Interruption due to abnormality										*/
#define		ADAT_STS_JERR			0x31	/* Inertia identification failure									*/
#define		ADAT_STS_DRVWAIT		0x41	/* Initial setting completed Input wait								*/
#define		ADAT_STS_JRUN			0x50	/* Inertia identification in progress									*/
#define		ADAT_STS_JEND			0x51	/* Inertia identification completed									*/
#define		ADAT_STS_VIBDET			0x60	/* Vibration level measurement in progress									*/
#define		ADAT_STS_MAXGNADJ		0x70	/* Limit gain search in progress									*/
#define		ADAT_STS_OPTGNADJ		0x80	/* Operation evaluation gain search in progress								*/
//#define		ADAT_STS_NOTCHSET		0x90	/* Notch filter automatic setting in progress						*/
#define		ADAT_STS_VIBDISSET		0x90	/* Vibration suppression setting in progress									*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_STS_COINNOWAIT		0x0000	/* COINNo waiting status								*/
#define		ADAT_STS_COINERR		0x0100	/* COINWaiting error status								*/
#define		ADAT_STS_COINWAIT		0x0200	/* COINWaiting status									*/
#define		ADAT_STS_COINWARN		0x0300	/* COINWaiting release status								*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_END				99		/* Forcibly terminate												*/
#define		ADAT_OK					0		/* Normal end												*/
#define		ADAT_CONTINUE			1		/* Executing												*/
#define		ADAT_BUSY				2		/* Continuing processing											*/
#define		ADAT_NG					-1		/* Abnormal termination												*/
#define		ADAT_NOTRDYERR			-2		/* Operation preparation incomplete error									*/
#define		ADAT_MOSESETERR			-3		/* Mode setting error										*/
#define		ADAT_JST_ERR			-4		/* Inertia identification start error								*/
#define		ADAT_J_ERR				-5		/* Inertia identification error									*/
#define		ADAT_PROGERR			-6		/* Operation command error										*/
#define		ADAT_QFILERR			-7		/* Frequency analysis error										*/
#define		ADAT_NTCHNG				-8		/* Notch setting error										*/
#define		ADAT_COINERR			-9		/* COINNot output error										*/
#define		ADAT_COMERR				-50		/* Communication error during operation									*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning processing request definition														*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_SEQ_INIT			0		/* Initial setting state											*/
#define		ADAT_SEQ_MODESET		1		/* Tuning mode setting status							*/
#define		ADAT_SEQ_JRATSET		2		/* Inertia setting state									*/
#define		ADAT_SEQ_NORMVIB		3		/* Normal vibration level search state								*/
#define		ADAT_SEQ_SEARCHOPTKV	4		/* OptimumKv,TiGain search state								*/
#define		ADAT_SEQ_NOTCHSET		5		/* Notch filter setting status								*/
#define		ADAT_SEQ_SEARCHOPTKP	6		/* OptimumKpGain search state									*/
#define		ADAT_SEQ_ADJMFC			7		/* Model following control adjustment state								*/
#define		ADAT_SEQ_ENDDISP		10		/* Termination display status											*/
#define		ADAT_SEQ_END			8		/* Final setting state											*/
#define		ADAT_SEQ_WAIT			9		/* Waiting for termination state											*/
#define		ADAT_SEQ_RETURN			11		/* Mode end status										*/
#define		ADAT_SEQ_NOOPWAIT		12		/* No Operation Display weight							*/
#define		ADAT_SEQ_START			13		/* Initialization status										*/
#define		ADAT_SEQ_2MASSSET		14		/* MFC(Rigid body+Machine base vibration)Setting state							*/
#define		ADAT_SEQ_KPSET			15		/* KpSetting state											*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_NO_OPE				100		/* no operation 										*/
#define		ADAT_CHKKV				101		/* Safety confirmation by command response								*/
#define		ADAT_VIBCOIN			102		/* Model following controlCOINCheck for cracks							*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning optimal gain search sequence definition										*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_OPTGN_INI			0		/* Optimum gain search initial setting state							*/
#define		ADAT_OPTGN_REQ			1		/* Optimum gain search request state								*/
#define		ADAT_OPTGN_SEARCH		2		/* Optimum gain search state									*/
#define		ADAT_OPTGN_WAIT			3		/* Vibration convergence wait state										*/
#define		ADAT_OPTGN_CHKKV_REQ	4		/* Command response safety confirmation request state								*/
#define		ADAT_OPTGN_CHKKV		5		/* Optimum gain search state2									*/
#define		ADAT_OPTGN_INIWAIT		6		/* Waiting for optimal gain search initial oscillation convergence						*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning optimal Kp gain search sequence definition									*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_OPTKP_INI			0		/* OptimumkpSearch initial setting state								*/
#define		ADAT_OPTKP_CHK_REQ		1		/* COINCrack confirmation request status									*/
#define		ADAT_OPTKP_CHK			2		/* COINCrack confirmation status										*/
#define		ADAT_OPTKP_WAIT			3		/* Vibration convergence wait state										*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning model follow-up adjustment sequence definition										*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_MFC_INI			0		/* Model following adjustment initial setting state							*/
#define		ADAT_MFC_MAXREQ			1		/* KpmAdjustment request status										*/
#define		ADAT_MFC_MAXSEARCH		2		/* KpmAdjustment status											*/
#define		ADAT_MFC_WAIT			3		/* Vibration convergence wait state										*/
#define		ADAT_MFC_CHK_REQ		4		/* COINCrack confirmation request status									*/
#define		ADAT_MFC_CHK			5		/* COINCrack confirmation status										*/
#define		ADAT_MFC_INIWAIT		6		/* Model following adjustment initial vibration convergence wait						*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning Scan C status definition											*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_SEAR_NORMALEND		5		/* The search process ends normally										*/
#define		ADAT_SEAR_VIBEND		1		/* Vibration detection finished											*/
#define		ADAT_SEAR_STOPVIBEND	2		/* End of vibration detection when stopped									*/
#define		ADAT_SEAR_COINBREAKEND	3		/* COINCrack detection completed										*/
#define		ADAT_SEAR_CONTINUE		4		/* In the search process											*/
#define		ADAT_SEAR_ERR			-1		/* Search processing abnormality											*/
#define		ADAT_SEAR_NOVIBDETECT	6		/* Vibration not detected end										*/
#define		ADAT_SEAR_COINOK		7		/*														*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning Program JOG status definition									*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_PJOG_NOP			0		/* During operation stop											*/
#define		ADAT_PJOG_EXEC			1		/* Driving												*/
#define		ADAT_PJOG_FIN			2		/* Operation completed												*/
#define		ADAT_PJOG_PLUSEND		3		/* Forward rotation direction command completed										*/
#define		ADAT_PJOG_MINUSEND		4		/* Reverse direction command complete										*/
#define		ADAT_PJOG_WAIT			5		/* Waiting for operation to start											*/
/*--------------------------------------------------------------------------------------------------*/
/*	Vibration detection limit value																					*/
/*--------------------------------------------------------------------------------------------------*/
#define		VIBMFC_DET_FRQ_MIN		10		/* Fn023 1.0Hz				*/
#define		VIBMFC_DET_FRQ_MAX		1000	/* Fn023 100.0Hz				*/
/*--------------------------------------------------------------------------------------------------*/
/*	MFC(Rigid body+Machine base)Parameter settings																	*/
/*--------------------------------------------------------------------------------------------------*/
#define		VIBMFC_PRM_ACTIVE		0x0011	/* Rigid body + machine model setting(Pn140)						*/
#define		RIGMFC_PRM_ACTIVE		0xff0f	/* Rigid body model setting(Pn140)								*/
/*--------------------------------------------------------------------------------------------------*/
/*	Advanced autotuning Program JOG operation pattern definition									*/
/*																									*/
/*		Pattern setting example	0x00F□□F□□														*/
/*							  ||||||														*/
/*							  ||||First operation selection											*/
/*							  |||Second operation selection											*/
/*							  |||Repeat the first and second times										*/
/*							  ||3rd operation selection												*/
/*							  |4th operation selection													*/
/*							  |Repeat 3rd and 4th time											*/
/*							  Operation stop																*/
/*		Select operation	0: Stop																			*/
/*					1: Fast forward forward rotation																	*/
/*					2: Fast forward reverse																	*/
/*					3: Normal feed forward																	*/
/*					4: Normal feed reverse																	*/
/*					6: Fast traverse forward rotation during inertia identification														*/
/*					7: Fast traverse reverse rotation while inertia identification														*/
/*					F: Repeat (the number of repetitions follows the level)										*/
/*																									*/
/*	Note) At the end of operation, create a pattern so that it is in the same position as at the start								*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_PJOG_STOP			0x00000000	/* Driving pattern 0 Operation stop							*/
#define		ADAT_PJOG_PATTERN1		0x00002121	/* Driving pattern 1 Fast forward, positive/negative direction(2 round trip)			*/
#define		ADAT_PJOG_PATTERN2		0x0000F4F3	/* Driving pattern 2 Normal feed, positive/negative direction				*/
#define		ADAT_PJOG_PATTERN3		0x0021F4F3	/* Driving pattern 3 Normal feed → Fast forward				*/
#define		ADAT_PJOG_PATTERN5		0x00000006	/* Operation pattern 5 Fast forward during identification, positive/negative direction			*/
/*--------------------------------------------------------------------------------------------------*/
#define		VIBOBS_KS				800		/* Vibration detection observerKsSwitchKvGain [0.1Hz]			*/
#define		DETVIBLP				3183	/* Low-pass filter for one-parameter vibration detection(3183us = 50Hz)	*/
#define		DETVIBHP2				3183	/* AATHigh-pass filter for vibration detection(3183us = 50Hz)			*/
#define		DETVIBHP3				1591	/* AATHigh-pass filter for vibration detection(1591us = 100Hz)		*/
#define		DETVIBHP				320		/* High-pass filter for oscillation detection:0.32[ms]=500[Hz]			*/
/*--------------------------------------------------------------------------------------------------*/
#define		TUNEACCTRQ2				1000	/* Tuning acceleration torque(New specifications)	[0.1%]				*/
#define		JRATACCTRQ				100		/* Acceleration torque when inertia identification(Start)	[0.1%]				*/
#define		JRATACCTRQ2				500		/* Acceleration torque when inertia identification(Identifying)[0.1%]				*/
#define		AATCONSVELTM			150		/* Reference constant speed time	[ms]								*/
#define		JRATE_VFFGN				100		/* Speed FF gain during inertia identification[%]					*/
#define		JRATPJOGACC				15		/* Acceleration lower limit value when inertia identification	[ms]					*/
/*--------------------------------------------------------------------------------------------------*/
#define		AATPJOGWAITTM			300		/* ProgramJOGOperation standby time		[ms]				*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADATVIBWAITTM1			500		/* Vibration convergence waiting time during tuning	500ms				*/
#define		ADATVIBWAITTM2			300		/* Vibration convergence waiting time during tuning	300ms				*/
#define		ADATVIBWAITTM3			100		/* Vibration convergence waiting time during tuning	100ms				*/
#define		ADATSTOPWAITTM			3000	/* Motor stop wait time during tuning	3000ms				*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADATCOINWAIT1			8000	/* COINError detection waiting time								*/
#define		ADATCOINWAIT2			3000	/* COINWarning detection waiting time									*/
#define		ADATDRVWAIT				300		/* Motor operation waiting time									*/
/*--------------------------------------------------------------------------------------------------*/
#define		COINOFFLMT				1000	/* Positioning failure tolerance time1ms[1us]					*/
#define		STPDETTM				100		/* Stop judgment time	100ms			[2ms]					*/
#define		ADAT_LFVIBDET_LVL		300		/* Residual vibration detection level(COINWidth ratio)	300%	[%]				*/
/*--------------------------------------------------------------------------------------------------*/
#define		GNUPTM2					400		/* Gain search interval(New specifications)	400ms	[2ms]				*/
/*--------------------------------------------------------------------------------------------------*/
#define		KVUP_UNIT				20		/* Kv UPWidth					[0.1Hz]						*/
#define		KV_UNLMT				100		/* Kv Lower limit value				[0.1Hz]						*/
#define		KPUP_UNIT				30		/* Kp UPWidth					[0.1s-1]					*/
#define		KPDOWN_UNIT				30		/* Kp DOWNWidth				[0.1s-1]					*/
#define		KP_UNLMT				100		/* Kp Lower limit value				[0.1s-1]					*/
#define		KPM_UNLMT				10		/* Kpm Lower limit value				[0.1s-1]					*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_DTUN_MAX			5		/* Movement distance adjustment digit										*/
#define		ADAT_DIST_MAX			99990000	/* Movement distance maximum value									*/
/*--------------------------------------------------------------------------------------------------*/
#define		MINIMUM_LINEAR_MM		5		/* Linear motor minimum movement distance[mm]							*/
#define		DEFAULT_STROKE_ROTARY	30		/* Rotary type motor initial travel distance[0.1rev]						*/
#define		DEFAULT_STROKE_LINEAR	900		/* Linear motor initial movement distance[0.1mm]						*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_DRV_PLUS			0		/* Driving direction: Forward											*/
#define		ADAT_DRV_MINUS			1		/* Driving direction: Reverse											*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_JDRV_MAXCNT		9		/* Maximum number of times of moment of inertia identification operation						*/
#define		ADAT_JRAT_CHGCNT		5		/* Moment of inertia identification Initial identified value correction times					*/
#define		ADAT_JRAT_CHG_RATE		200		/* Moment of inertia identification initial identification value correction per one correction value		*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_ERR_ULMT( data )	(((data)*33)>>5)	/* +3.125%									*/
#define		ADAT_ERR_LLMT( data )	(((data)*31)>>5)	/* -3.125%									*/
#define		ADAT_ERR2_ULMT( data )	(((data)*21)>>4)	/* +31.25%									*/
#define		ADAT_ERR2_LLMT( data )	(((data)*11)>>4)	/* -31.25%									*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_GRAT_80( data )	(((data)*205)>>8)	/* 80%										*/
#define		ADAT_GRAT_70( data )	(((data)*180)>>8)	/* 70%										*/
#define		ADAT_GRAT_50( data )	((data)>>1)			/* 50%										*/
#define		ADAT_GRAT_62( data )	(((data)*5)>>3)		/* 62.5%									*/
#define		ADAT_GRAT_150( data )	(((data)*3)>>1)		/* 150%										*/
#define		ADAT_GRAT_94( data )	(((data)*15)>>4)	/* 93.75%									*/
#define		ADAT_GRAT_106( data )	(((data)*17)>>4)	/* 106.25%									*/
#define		ADAT_GRAT_113( data )	(((data)*9)>>3)		/* 112.5%									*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_STS_NF_SETOK		0		/* Notch filter setting completed								*/
#define		ADAT_STS_NF_SETERR		-1		/* Notch filter setting error								*/
#define		ADAT_STS_NF_ANAERR		-2		/* Frequency analysis error										*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_LVL_50				0x0A000200	/* Gain correction: 50%									*/
#define		ADAT_LVL_70				0x0A0002CD	/* Gain correction: 70%									*/
#define		ADAT_LVL_80				0x0A000333	/* Gain correction: 80%									*/
#define		ADAT_LVL_100			0x0A000400	/* Gain correction:100%									*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADATMFC_ONE_MASS		0x01	/* Rigid body model											*/
#define		ADATMFC_TWO_MASS1		0x02	/* 2 inertia model 1									*/
#define		ADATMFC_TWO_MASS2		0x03	/* 2 inertial model 2									*/
#define		ADATMFC_ONE_BASE		0x04	/* Rigid body + machine model									*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_MODE_MIN			0		/* Mode lower limit											*/
#define		ADAT_MODE_MAX			1		/* Mode upper limit											*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_LEVEL_MIN			1		/* Level lower limit											*/
#define		ADAT_LEVEL_MAX			3		/* Level upper limit											*/
#define		ADAT_LEVEL_NOR			1		/* Adjustment level = Standard										*/
#define		ADAT_LEVEL_POS			2		/* Adjustment level = For positioning only								*/
#define		ADAT_LEVEL_POS_ST		3		/* Adjustment level = For positioning only(Overshoot emphasis)		*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_SCROLL_MIN			0		/* Internal commandAATSetting screen scroll lower limit					*/
#define		ADAT_SCROLL_MAX			3		/* Internal commandAATSetting screen scroll upper limit					*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_REFIN_SCROLL_MIN	1		/* Command input typeAATSetting screen scroll lower limit					*/
#define		ADAT_REFIN_SCROLL_MAX	2		/* Command input typeAATSetting screen scroll upper limit					*/
/*--------------------------------------------------------------------------------------------------*/
#define		ADAT_JRAT_OK			0		/* Moment of inertia identification: Normal operation						*/
#define		ADAT_JRAT_ERR1			1		/* Moment of inertia identification: Estimation start failure						*/
#define		ADAT_JRAT_ERR2			2		/* Moment of inertia identification: Estimation failure							*/
#define		ADAT_JRAT_ERR3			3		/* Moment of inertia identification: Low frequency vibration detection					*/
#define		ADAT_JRAT_ERR4			4		/* Moment of inertia identification: Torque limit error					*/
#define		ADAT_JRAT_ERR5			5		/* Moment of inertia identification: P control, deviation clear				*/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Struct Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Advanced autotuning operation register structure											*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct ADATREG{			/* Operation register														*/
	USHORT	AdatMode;						/* 0x2090 : AdvancedATMode						*/
	USHORT	AdatLevel;						/* 0x2091 : AdvancedATLevel						*/
	DWORDX	AdatMovep;						/* 0x2092 : AdvancedATTravel distance						*/
	USHORT	AdatState;						/* 0x2094 : AdvancedATConfirm completion						*/
	USHORT	AdatFilType;					/* 0x2095 : AdvancedATFilter type				*/
}ADATREG;

/*--------------------------------------------------------------------------------------------------*/
/*		Advanced autotuning data structure													*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct ADATV {
/*--------------------------------------------------------------------------------------------------*/
	struct {					/* Bit signal														*/
		UINT	RefInType		:1;			/* Command input type											*/
		UINT	UsedAllNtFil	:1;			/* All notch filters used									*/
		UINT	UsedAResCtrl	:1;			/* A-type vibration damping used											*/
		UINT	JratCalRun		:1;			/* Moment of inertia identification									*/
		UINT	GainTuneRun		:1;			/* During gain adjustment											*/
		UINT	FftReq			:1;			/* Frequency analysis request										*/
		UINT	VibDetect		:1;			/* Vibration detected												*/
		UINT	CoinBreak		:1;			/* COINCrack												*/
		UINT	VibDetectatStop	:1;			/* Vibration detection when stopping										*/
		UINT	VibDetectinCycle:1;			/* Vibration detected during this driving									*/
		UINT	CoinLatch		:1;			/* COINLatch flag										*/
		UINT	LastCoinLatch	:1;			/* PreviousCOINLatch flag									*/
		UINT	CoinIgnore		:1;			/* COINIgnore												*/
		UINT	OrgIPSpdControl	:1;			/* I-POperation in control										*/
		UINT	DrvPatternSet	:1;			/* Driving pattern setting										*/
		UINT	DrvErrDetect	:1;			/* Operation error detection										*/
		UINT	CoinWaitStart	:1;			/* COINWaiting process start flag								*/
		UINT	Used2MassMfc	:1;			/* MFC(Rigid body+Machine base model)Set flag					*/
		UINT	Mfc2MassOK		:1;			/* MFC(Rigid body+Machine base model)Final set flag					*/
		UINT	Used2MassOld	:1;			/* AATPreviousMFCSet flag								*/
		UINT	MfcLastSet		:1;			/* MFCFinal adjusted flag							*/
		UINT	OvsErr			:1;			/* Overshoot error flag							*/
		UINT	StopDetectMode2	:1;			/* Command inputAATStop judgment mode 2 flag					*/
											/* FALSE:COINTURE: Command=0		*/
		UINT	KvTuneWait 		:1;			/* KvGain tuning stop flag						*/
	} f;
/*--------------------------------------------------------------------------------------------------*/
//	USHORT	TunStartMFCModel;				/* AATAt the start of adjustmentMFCmechanical model							*/
/*--------------------------------------------------------------------------------------------------*/
	SHORT	ErrState;						/* Error status										*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	ModeSetPtr;						/* Mode setting: Sequence pointer						*/
	UCHAR	ModeMin;						/* Mode setting: Mode selection lower limit							*/
	UCHAR	ModeMax;						/* Mode setting: Mode selection upper limit							*/
	UCHAR	ModeSetMin;						/* Mode setting: Mode setting lower limit							*/
	UCHAR	ModeSetMax;						/* Mode setting: Mode setting upper limit							*/
	USHORT	TunModeMax;						/* Mode setting: Adjustment mode maximum value							*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	JratSetPtr;						/* Moment of inertia identification: Sequence pointer				*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	VibDetPtr;						/* Vibration detection: Sequence pointer							*/
	SHORT	VibDetStat;						/* Vibration detection: Status									*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	KvTunePtr;						/* KvAdjustment: Sequence pointer							*/
	USHORT	MaxKv;							/* KvAdjustment: MaximumKvvalue										*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	NextSeqPtr;						/* Next sequence pointer								*/
	USHORT	NextSubSeqPtr;					/* Next subsequence pointer							*/
	USHORT	Set2MassMfcSequence;			/* 2 inertiaMFCSet sequence pointer				*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	KpSetPtr;						/* KpSetting: Sequence pointer							*/
	USHORT	MaxKp;							/* KpSettings: MaximumKpvalue										*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	KpTunePtr;						/* KpAdjustment: Sequence pointer							*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	MaxKpm;							/* KpmSettings: MaximumKpmvalue									*/
	USHORT	MfcTunePtr;						/* Model following control adjustment: sequence pointer				*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	NotchSetPtr;					/* Notch filter setting: Sequence pointer				*/
	SHORT	NotchSetStat;					/* Notch filter setting: Notch setting status				*/
	USHORT	VibMaxKv;						/* Notch filter setting: Limit gain when notch is not set		*/
	USHORT	VibMaxKvAresOFF;				/* A-type vibration damping setting: Limit gain when A-type vibration suppression is not set			*/
	USHORT	MaxKvbk;						/* Notch filter setting: VibrationKvvalue							*/
	SHORT	FftStat;						/* Notch filter setting: Frequency analysis status				*/
	UCHAR	ANotchSetStat;					/* Notch filter setting: Automatic notch setting status			*/
	UCHAR	ANotchSetStatBk;				/* Notch filter setting: Automatic notch setting save status		*/
	UCHAR	AResSetStat;					/* Notch filter setting: Automatic A type setting status			*/
	UCHAR	AResSetStatBk;					/* Notch filter setting: Auto A type setting save status		*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	ScanCRequest;					/* ScanC								*/
	SHORT	ScanCStat;						/* ScanC						*/
/*--------------------------------------------------------------------------------------------------*/
	ULONG	PrgJogPatternReq;				/* Run command: Request pattern								*/
	ULONG	PrgJogPattern;					/* Run command: Received pattern								*/
	ULONG	PrgJogActPattern;				/* Run command: Current pattern								*/
	USHORT	PrgJogPatternCnt;				/* Run command: Pattern index						*/
	SHORT	PrgJogStat;						/* Run command: Operation status								*/
	USHORT	PrgJogPtr;						/* Run command: Sequence pointer							*/
	USHORT	PrgJogTimes;					/* Operation command: Number of operation repetitions							*/
	USHORT	PrgJogDrvCnt;					/* Operation command: Number of operation									*/
	ULONG	PrgJogDrvCntr;					/* Operation command: Counter for detecting operation duration					*/
	ULONG	PrgJogWaitCntr;					/* Operation command: Counter for generating operation stop time					*/
	ULONG	PrgJogMovStartCntr;				/* Run command: Movement start waiting counter						*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	TrqRefatStop;					/* Normal torque command value when stopped								*/
	LONG	MaxTrqRefatStop;				/* Maximum normal torque when stopped								*/
	LONG	MinTrqRefatStop;				/* Minimum normal torque when stopped								*/
	LONG	VibSpeed;						/* Normal operation amplitude level									*/
/*--------------------------------------------------------------------------------------------------*/
	ULONG	CoinWaitStat;					/* COINDetection: Status									*/
	ULONG	CoinWaitCntr;					/* COINDetection:COINCrack time detection counter					*/
	ULONG	CoinOnCntr;						/* COINDetection:COINCounter for on-time detection					*/
/*--------------------------------------------------------------------------------------------------*/
	ULONG	GainUpCntr;						/* Gain adjustment: Counter for detecting presence or absence of vibration after gain up	*/
	ULONG	KpGainUpCntr;					/* Gain adjustment: Counter for detecting presence or absence of vibration after gain up	*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	KTrqHpFil;						/* Torque high-pass filter time constant conversion value					*/
	LONG	KTrqHpftmp;						/* Torque high-pass filter buffer						*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	MinDistance;					/* Minimum travel distance											*/
	LONG	DefaultDistance;				/* Initial movement distance											*/
	LONG	MoveDistance;					/* Travel distance		[Command unit]								*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	PositioningTimes;				/* Positioning number											*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	JratDrvCnt;						/* Moment of inertia identification: Number of operations							*/
	SHORT	LastEstimatJrat;				/* Moment of inertia identification: Estimation ratio				[%]			*/
	USHORT	Jrate;							/* Moment of inertia identification: Estimated ratio used for gain adjustment	[%]			*/
	USHORT	JratChangeTimes;				/* Moment of inertia identification: Initial identification value correction number				*/
	ULONG	JratVibWaitCntr;				/* Moment of inertia identification: Counter for detecting initial vibration convergence time	*/
	ULONG	JratVibWaitCntr2;				/* Moment of inertia identification: Counter for detecting vibration convergence time		*/
	USHORT	JratErr;						/* Moment of inertia identification: Error state						*/
/*--------------------------------------------------------------------------------------------------*/
	ULONG	KvTuneCntr;						/* KvAdjustment: Time measurement counter in sequence					*/
	ULONG	KvTuneWaitCntr;					/* KvAdjustment: Time measurement counter in sequence					*/
	ULONG	NotchSetCntr;					/* Notch filter setting: Time measurement counter in sequence		*/
	ULONG	KpTuneCntr;						/* KpAdjustment: Time measurement counter in sequence					*/
	ULONG	MfcTuneCntr;					/* Model following control adjustment: In-sequence time measurement counter		*/
	ULONG	KpSetCntr;						/* KpSetting: Sequence time measurement counter					*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	frq_buf;						/* Resonance and antiresonance frequency buffer							*/
	USHORT	Kpm_buf;						/* KpmBuffer											*/
	USHORT	Kv_buf;							/* KvBuffer											*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	settling_max;					/* Maximum value of settling time										*/
	LONG	settling_buf;					/* Maximum value buffer for settling time								*/
/*--------------------------------------------------------------------------------------------------*/
	BOOL	ScanCReqFlg;
	USHORT	ScanCReqCmd;
/*--------------------------------------------------------------------------------------------------*/
} ADATV;

/*--------------------------------------------------------------------------------------------------*/
/*		Advanced autotuning Data pointer												*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct ADATP{
	FUN_CMN_CONTROL 	*FnCmnCtrl;
	UNI_PRM_STR 		*UniPrms;
	MENCV 				*MencV;
	BASE_CTRL_HNDL 		*BaseCtrlData;
	REG_MANAGER_HANDLE 	*RegManager;
//	SEQ_CTRL_HNDL 		*SeqCtrlData;
	SEQ_MOT_STS 		*SeqMotSts;
	SEQ_CTRL_OUT 		*SeqCtrlOut;
	ALMMAN_HANDLE 		*AlmManager;
	SETTLINGTIME 		*SettlingTime;
	AUTONOTCH   		*ANotch;
//	DET_HWBB_STS 		*DetHwbbSts;
	REMVIBFREQ   		*RemVibFreq;
	FFTANALYZE			*FftAna;
	ASICS 				*SvAsicRegs;
	MOTSPDMAFIL  	 	*MotSpdMafil;
	DETVIB 				*DetVib;
	SERVO_CONTROL_IF 	*SvControlIf;
} ADATP;

/*--------------------------------------------------------------------------------------------------*/
/*		Advanced autotuning structure														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct ADATHNDL {
	ADATREG AdatReg;
	ADATV	AdatV;
	ADATP	AdatP;
	PJOGV	OrgPJogV;						/* ProgramJOGVariable for									*/
}ADATHNDL;

/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
void	CpxAdvancedAutoTuning( ADATHNDL	*AdatHndl, FUN_CMN_CONTROL *FnCmnCtrl );
void 	AdatSetStrPtr( void *Axis );
LONG 	AdatPcalDefautDistance( ADATP *AdatP, LONG AxisMotType, BOOL LowSpdMotor ); /* <S145> */
#endif //__ADV_AUTO_TUNING_H
/***************************************** end of file **********************************************/

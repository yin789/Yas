/****************************************************************************************************/
/*																									*/
/*																									*/
/*		CheckAlarm.h : Alarm monitoring processing definition															*/
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
#ifndef CHECK_ALARM_H
#define CHECK_ALARM_H


#include "MLib.h"
#include "EncData.h"
#include "AlarmManager.h"
#include "SftDiagFunc.h"


/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder alarm definition																	*/
/*--------------------------------------------------------------------------------------------------*/
#define PG_OH_LEVEL				90							/* PG overheat temperature [℃]			*/

/*--------------------------------------------------------------------------------------------------*/
/*		Linear definition																				*/
/*--------------------------------------------------------------------------------------------------*/
#define	POSDATALMT				0x70000000					/* Position data over threshold[Command unit]		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Motor runaway detection definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define MOTSPD_OS_10	1677721		/* OSOf speed1/10													*/


/*--------------------------------------------------------------------------------------------------*/
/*		Runaway monitoring structure definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_OVRRUN {
	LONG	OvrChkCnt;						/* Runaway monitoring : Monitoring time count							*/
	LONG	OvrAlmCnt;						/* Runaway monitoring : Alarm count							*/
	LONG	OvrTrqRef;						/* Runaway monitoring : Previous torque command							*/
	LONG	OvrAbsMotSpd;					/* Runaway monitoring : Previous motor speed absolute value						*/
	LONG	OvrPeakMotSpd;					/* Runaway monitoring : Motor speed maximum value							*/
	LONG	OvrTrqLevel;					/* Runaway monitoring : Torque level								*/
	LONG	OvrSpdLevel;					/* Runaway monitoring : Speed level								*/
} CHECK_OVRRUN;


/*--------------------------------------------------------------------------------------------------*/
/*		Data definition for amplifier & motor overload check													*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct OLPRM {					/* Overload check basic parameters								*/
	LONG	Tmid;							/* Intermediate currentOLTime(Includes derating)	[0.1s]			*/
	LONG	Tmax;							/* Maximum currentOLTime(Includes derating)	[0.1s]			*/
	LONG	Ibse;							/* Base current(Includes derating)		[15000/Imax]	*/
	LONG	Imid;							/* IntermediateOLCurrent(Includes derating)		[15000/Imax]	*/
	LONG	Imax;							/* MaximumOLCurrent(Includes derating)		[15000/Imax]	*/
} OLPRM;


/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for overload check																	*/
/*--------------------------------------------------------------------------------------------------*/
#if 1	/* <S154> */
typedef	struct _CHECK_OLP {					/* Overload check parameter definition						*/
		LONG	Ibsqrmid;					/* Continuous overload base current squared							*/
		LONG	Ibsqrmax;					/* Instantaneous overload base current squared							*/
		LONG	AlmLvlmid;					/* Continuous overload alarm level								*/
		LONG	WrnLvlmid;					/* Continuous overload warning level							*/
		LONG	AlmLvlmax;					/* Instantaneous overload alarm level								*/
		LONG	WrnLvlmax;					/* Instantaneous overload warning level							*/
		KSGAIN	OlLvlmidGain;				/* Continuous overload detection level gain[Low speedOLCorrespondence]				*/
		KSGAIN	OlLvlmaxGain;				/* Continuous overload detection level gain[Low speedOLCorrespondence]				*/
		KSGAIN	CoolRateGain;				/* Self-running air cooling coefficient(For linear)								*/
} CHECK_OLP;/* <S154> */

/* <S154> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for overload check																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_OL {
	struct	{							/* Overload check parameter definition						*/
		CHECK_OLP	NormalArea;			/* Parameters for normal section overload check					*/
		CHECK_OLP	LowSpdArea;			/* Low speed section overload check parameters					*/
		CHECK_OLP	LockArea;			/* Lock section overload check parameters					*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct	{							/* Variable definition for overload check									*/
		LONG	WrnInfo;					/* Warning information										*/
		LONG	SumImid;					/* Square current integrated value for continuous overload							*/
		LONG	SumImax;					/* Square current integrated value for instantaneous overload							*/
	} var;
} CHECK_OL;
#else	/* <S154> */
typedef	struct CHECK_OL {
	struct	{							/* Overload check parameter definition							*/
		LONG	Ibsqrmid;					/* Continuous overload base current squared							*/
		LONG	Ibsqrmax;					/* Instantaneous overload base current squared							*/
		LONG	AlmLvlmid;					/* Continuous overload alarm level								*/
		LONG	WrnLvlmid;					/* Continuous overload warning level							*/
		LONG	AlmLvlmax;					/* Instantaneous overload alarm level								*/
		LONG	WrnLvlmax;					/* Instantaneous overload warning level							*/
		KSGAIN	OlLvlmidGain;				/* Continuous overload detection level gain[Low speedOLCorrespondence]				*/
		KSGAIN	OlLvlmaxGain;				/* Continuous overload detection level gain[Low speedOLCorrespondence]				*/
		KSGAIN	CoolRateGain;				/* Self-running air cooling coefficient(For linear)								*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct	{							/* Variable definition for overload check									*/
		LONG	WrnInfo;					/* Warning information										*/
		LONG	SumImid;					/* Square current integrated value for continuous overload							*/
		LONG	SumImax;					/* Square current integrated value for instantaneous overload							*/
	} var;
} CHECK_OL;
#endif	/* <S154> */


/*--------------------------------------------------------------------------------------------------*/
/*		DBStructure definition for overload check																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_DBOL {
	struct	{							/* DBConstant definition for overload check								*/
		BOOL	DbOHeatCheck;				/* Selecting whether to perform DB overload detection processing due to temperature rise			*/
		LONG	Kdbpower;					/* DB overload : Power calculation gain							*/
		LONG	DbolPowMax; 				/* DB overload : Maximum power value								*/
		LONG	DbolPowBase; 				/* DB overload : Power base								*/
		LONG	DbolSpdLevel;				/* DB overload : Speed level								*/
		LONG	DbolAlmLevel;				/* DB overload : Alarm level							*/
		LONG	Kundbpower;					/* DB resistance power consumption calculation gain(for Un00B)				*/
		LONG	KundbpowerCycle;			/* DB resistance power consumption calculation gain(for Un144)				*/
		LONG	DbOHeatAlmLvl;				/* DB overload(Temperature rise): Alarm level					*/
		LONG	DbOHeatWrnLvl;				/* DB overload(Temperature rise): Warning level				*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct	{							/* DBVariable definition for overload check								*/
		BOOL	DbOHeatWarn;				/* DB overload(Temperature rise)Warning detection flag			*/
		LONG	DbolPowSum;					/* DB overload : Integrated power value								*/
		LONG	DbPowerSum;					/* DB average value : Integrated power value								*/
		LONG	DbMeanCycle;				/* DB average value : Number of power integration times							*/
		LONG	DbHeat;						/* DB overload(Temperature rise)Integrated value for detection						*/
		LONG	UnDbPower;					/* Un144  : DB power consumption value 			[%]					*/
		LONG	UnMeanDbPower;				/* Un00B  : DB average value of power consumption 		[%]					*/
	} var;
} CHECK_DBOL;

/* <S154> Start */
/************************************************************************************************/
/*																								*/
/*		Variable definition for amplifier low speed overload check																*/
/*																								*/
/************************************************************************************************/
/************************************************************************************************/
/*		Amplifier low speed overload: SGDS method																	*/
/************************************************************************************************/
typedef	struct _LOWOLPRM{							/* Low-speed overload check basic parameters					*/
		OLPRM	AmpLock;						/* Basic parameters for lock section							*/
		OLPRM	AmpLow;							/* Basic parameters for low speed section							*/
} LOWOLPRM;
/*----------------------------------------------------------------------------------------------*/
typedef	struct _LOWOLCHKP{							/* Low-speed overload check parameters					*/
		struct {
		LONG	LowSpdOLchg;					/* Low speed OL inflection point speed[Low speed section]	[2^24/OvrSpd]	*/
		LONG	LockSpdOLchg;					/* Low speed OL inflection point speed[Lock section]	[2^24/OvrSpd]	*/
		LONG	LowSpdOffsetImid;				/* Low speed OL inflection point OL current[Low speed section] [15000/Imax]		*/
		LONG	LowSpdOffsetTmax;				/* Low speed OL inflection point OL time[Low speed section] [0.1s]			*/
		LONG	TransOLImid;					/* Low speed OL intermediate OL current conversion coefficient 						*/
		LONG	TransOLTmax;					/* Low-speed OL maximum OL time conversion coefficient 						*/
		UCHAR	LowSpdOLChkEnable;				/* Low-speed overload check execution flag						*/
		CHAR	Dummy1;							/* for alignment								*/
		SHORT	Dummy2;							/* for alignment 								*/
		} conf;
} LOWOLCHKP;

typedef	struct _CHECK_LOWOL{							/* Structure definition for low-speed overload check			*/
	LOWOLPRM	LowOLPrm;								/* Low-speed overload check basic parameters				*/
	LOWOLCHKP	LowOLChkP;								/* Low-speed overload check parameters				*/
	/* <S1F5> Start */
	struct {
		BOOL	BaseCurrentDerateEnable;				/* Overload detection base current derating function enabled 	*/
	}conf;
	/* <S1F5> End */
} CHECK_LOWOL;
/* <S154> End */

#if 0	/*<S1F6>*/
/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for safety input signal timing error check										*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_HWBB_TIMING {
	struct	{							/* Constant definition for safety input signal timing error check		*/
		LONG	MaxBbonTimeDiff;			/* Input signal time difference monitoring: Maximum allowable time						*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct	{							/* Variable definition for safety input signal timing error check		*/
		LONG	BbonTimeDiffCnt;			/* Safety input signal timing error time count			*/
	} var;
} CHECK_HWBB_TIMING;
#endif	//#if 0	/*<S1F6>*/


/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for excessive position deviation check															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_OVRPOSERR {
	LONG		PerAlmLevel;				/* Excessive position deviation alarm level				[Command unit]	*/
	LONG		PerWrnLevel;				/* Excessive position deviation warning level				[Command unit]	*/
	LONG		SvonPerAlmLevel;			/* Excessive position deviation alarm level when servo is on	[Command unit]	*/
	LONG		SvonPerWrnLevel;			/* Excessive position deviation warning level when servo is on	[Command unit]	*/
} CHECK_OVRPOSERR;

typedef	struct CHECK_FENCPOSERR {
	LONG	FencPosLast;					/* Excessive semi-full deviation detection: previous full position					*/
	LONG	MencPosLast;					/* Excessive semi-full deviation detection: semi-full previous position					*/
	LONG	ExtPGRem;						/* Excessive semi-full deviation detection:								*/
	LONG	MtPGRem;						/* Excessive semi-full deviation detection:								*/
	LONG	FencPosOneTurn;					/* Semi-full deviation excessive detection: Fulk 1 rotation previous position				*/
	ULONG	FSOFAlmCnt;						/* Semi-full deviation excessive detection: Detection filter					*/
	PERRA	sfperra;						/* Position deviation A structure (Semi-Full)For deviation calculation					*/

	EGEAR	Egear;							/* Semi gear structure											*/
	EGEAR	sfgear;							/* Semi/Full conversion gear structure									*/
	ULONG	fpgoverLevel;					/* Motor-Excessive deviation between load positions detection level		[Command unit]	*/
	LONG	fpgcutratelv;					/* Full close1Multiply value per rotation				   [%]	*/
	LONG	fcpgpls;						/* Number of external scale pitches					  [P/Rev]	*/
	USHORT	fpgcutrate;						/* Pn52A : Full close1Multiply value per rotation				*/
} CHECK_FENCPOSERR;

/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for alarm detection processing																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_ALARM {
//	ENVMON				EnvMon;				/* Installation environment monitor variables									*//* <S03A> *//* <S0ED> */
//	CHECK_OH			OverHeat;			/* Variable for overheat check							*//* <S03A> *//* <S0ED> */
	CHECK_OL			MotOL;				/* Variable for motor overload check							*/
	CHECK_OL			AmpOL;				/* Variables for amplifier overload check							*/
	CHECK_OVRRUN		OvrRun;				/* Runaway detection variable										*/
	CHECK_DBOL			DbOL;				/* DBVariable for overload check								*/
	CHECK_HWBB_TIMING	HwbbTiming;			/* HWBBInput signal timing abnormality check variable				*/
	CHECK_OVRPOSERR		OvrPosErr;			/* Variable for checking excessive position deviation							*/
	CHECK_FENCPOSERR	OvrFPosErr;			/* Variable for checking excessive semi-full position deviation					*/
	CHECK_LOWOL			AmpLowOL;			/* Variable definition for amplifier low speed overload							*/ /* <S154>  */
//	CHECK_RUSH			*pRush;				/* Inrush prevention resistor overload detection variable							*/
//	CHECK_REGOL			*pRegOL;			/* Pointer to variable for regenerative overload check(Common to units)	*/
//	CHECK_REGENE		*pRegChk;			/* Pointer to variable for regeneration abnormality check(Common to units)		*/
	ULONG				OSCount;			/* OSStatus continuity counter									*/
	LONG				Gup1Timer;			/* A.b33Detection timer									*/
	USHORT 				faultState;			/* Fault status of Servo ASIC 							*/

} CHECK_ALARM;




/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void	AdetInitCheckAlarm( CHECK_ALARM *pChkAlm, ULONG AlmOLInfo );
USHORT	AdetCheckAsicHwAlarm( CHECK_ALARM *pChkAlm, ASICS *SvAsic,
										BOOL EncDisable, ALMMAN_HANDLE *AlmMngr );
void	AdetCheckOCAlarmDetections( ALMMAN_HANDLE *AlmMngr, ASICS *SvAsic, LONG axisNo, BOOL BeSts );	/* <S126> *//* <S137> */
void	AdetCheckMotorOverCurrent( ALMMAN_HANDLE *AlmMngr, ASICS *SvAsic, BOOL BeSts, BOOL BaseEnableSts );/*<S1F5>*/
void	AdetCheckSencAlarm( ALMMAN_HANDLE *AlmMngr, MENCV *MencV, USHORT RxAlmCode );					/* <S03A> */
void	AdetCheckSencBaseTimer(MENCV *MencV, ALMMAN_HANDLE *AlmMngr );
void	AdetCheckSencOverHeat( MENCV *MencV, ALMMAN_HANDLE *AlmMngr );
void	AdetCheckOverSpeed( ALMMAN_HANDLE *AlmMngr, CHECK_ALARM *pChkAlm, LONG MotSpd, BPRMDAT *Bprm );
/*--------------------------------------------------------------------------------------------------*/
void	AdetCheckDivOverSpeed( ALMMAN_HANDLE *AlmMngr, LONG MotSpd, LONG DivOvrSpd );				/*<S02D>*/
/*--------------------------------------------------------------------------------------------------*/
//void	AdetCheckOverHeatAlm( ALMMAN_HANDLE *AlmMngr, CHECK_OH *CheckOh,							  /* <S03A> *//* <S0ED> */
//														SHORT CtrlBoadTemp, SHORT PowerBoadTemp );		/* <S0ED> */
//void	AdetCheckTempDeviceErr( ALMMAN_HANDLE *AlmMngr, USHORT CtrlBoadErrCnt, USHORT PowerBoadErrCnt );/* <S0DE> *//* <S0ED> */
//void	AdetInstEnvMonitor( ENVMON *EnvMon, SHORT AmpTemp, SHORT MotTemp );							  /* <S03A> *//* <S0ED> */

//void	AdetCheckOverLoadAmpMot( ALMMAN_HANDLE *AlmMngr, CHECK_ALARM *CheckAlarm,
//								LONG IdRefMon, LONG IqRefMon, LONG AbsMotSpd, BOOL EncConnect );				/* <S154> */
BOOL	AdetCheckOverLoadAmpMot( ALMMAN_HANDLE *AlmMngr, CHECK_ALARM *CheckAlarm,
								LONG IdRefMon, LONG IqRefMon, LONG AbsMotSpd, BOOL EncConnect );				/* <S154> */

void	AdetCheckOLAmpMot( LONG AbsMotSpd, LONG Ixsqr, CHECK_OL *ol_ptr, ALMMAN_HANDLE *AlmMngr , UCHAR Mode);	/* <S154> */

void	AdetCheckDbrakeOverLoad( ALMMAN_HANDLE *AlmMngr, CHECK_DBOL *DbOL, BOOL DbOn, LONG AbsMotSpd );
void	AdetCheckRippleWrn( ALMMAN_HANDLE *AlmMngr, BOOL RippleWrnState );									/* <S1FD> */
BOOL	AdetCheckBbonSignal( CHECK_HWBB_TIMING *pHwbbTmg, BOOL LackOneBbon );
//BOOL	AdetCheckGateDriveDefect( DET_HWBB_STS *pDetHwbbSts );
void	AdetCheckMotorOverrun( ALMMAN_HANDLE *AlmMngr, CHECK_OVRRUN *OvrRun,
								 LONG MotSpd, LONG TrqRefMon, LONG BaseEnableSts, BOOL MotTypeLinear );
void	AdetCheckSvonPerrOverFlow( ALMMAN_HANDLE *AlmMngr, CHECK_OVRPOSERR *OvrPosErr, LONG PositionError );
void	AdetCheckPerrOverFlow( ALMMAN_HANDLE *AlmMngr, CHECK_OVRPOSERR *OvrPosErr,
										BOOL PosCtrlMode, LONG PositionError, BOOL SvonSpdLmt );
void AdetCheckFullcEncoderAlarm(CHECK_FENCPOSERR *ChkFposErr, MENCV *FencV,
                            	BOOL PosCtrlMode, LONG MencPos, LONG FencPos, ALMMAN_HANDLE *AlmMngr );
//<S1F5>BOOL	AdetTestPulseDiagnostic( BOOL DpInState, BOOL DpInLastState, BOOL CheckEnable, BOOL Bbon );					/* <S0F1> *//*<S1DB>*/





#endif /* CHECK_ALARM_H */
/***************************************** end of file **********************************************/

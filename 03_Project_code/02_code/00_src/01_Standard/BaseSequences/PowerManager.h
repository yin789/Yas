#ifndef	_POWER_MANAGER_H_ /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PowerManager.h : Servo power management processing definition														*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.04.19	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Bprm.h"
#include "BaseCmnStruct.h"
#include "JL086ServoIpReg.h"


/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Main circuit monitoring structure definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_MAIN_POWER {
	struct	{							/* Parameter definition for main circuit power supply monitoring							*/
		ULONG	AcoffDetctTime;				/* AC Power Off Detect Time								*/
		BOOL	Dcbus;						/* DCPower input selection(Pn001.2)								*/
		BOOL	Ac3Phase;					/* power input3Phase(PnE0C.1,2)								*/
		BOOL	DetectAcOn2;				/* Power supply phase loss detection method(PnE0C.3)							*/
		LONG	RlyDbWay;					/* Inrush/DBControl method selection setting(PnE0C.4,5)					*/
		ULONG	UvFilter;					/* UVDetection filter time constant(PnE74)							*/
		LONG	OvdetAlmFilter;				/* Overvoltage alarm filter(PnE7A.2,3)	(1=250us)		*/
		ULONG	PowerChargeWaitTime;		/* DCCharging completion waiting time	<S053>							*/
		ULONG	PowerReadyWaitTime;			/* Power ready waiting time	<S053>							*/
		ULONG	DischCheckVlt;				/* Rapid discharge confirmation threshold voltage									*//* <S1C6> *//* <S1D1> */
		ULONG	DischCompVlt;				/* Rapid discharge continuation threshold voltage									*//* <S1C6> *//* <S1D1> */
		ULONG	MainDischActTime;			/* Main rapid discharge time(Same as charging time)					*//* <S1C6> */
		USHORT	UvLevel;					/* Undervoltage(UV)Detection level								*/
		USHORT	UvWrnLevel;					/* Undervoltage(UV)Warning detection level						*/
		USHORT	OvLevel;					/* Overvoltage when regeneration is used(OV)Detection level						*/
		USHORT	NonRegOvLevel;				/* Overvoltage when regeneration is not used(OV)Detection level					*//* <S0F7> */
		USHORT	TestDcLevel;				/* Automatic tester mode: Adjust reference voltage when correcting main circuit voltage detection	*/
		BOOL	TestMode1;					/* Automatic tester mode: Power management							*/
		BOOL	TestMode2;					/* Automatic tester mode: No regeneration abnormality detection					*/
		BOOL	TestMode3;					/* Automatic tester mode: Lower Tr is not turned on when DB is stopped(INGRAM.)	*/
		BOOL	TestMode4;					/* Automatic testing machine mode:ASIC WDTofkickdid not				*/
		BOOL	TestMode5;					/* Automatic tester mode: Force anti-rush relay when main circuit is cut offON/OFF	*/
		BOOL	TestMode6;					/* Automatic tester mode: Force regeneration controlON/OFF				*//* <S0CB> */
		BOOL	TestMode7;					/* Automatic testing machine mode:DBControl is forcedON/OFF					*//* <S0CB> */
		BOOL	TestMode8;					/* Automatic tester mode: Force gate drive signalON/OFF		*//* <S0CB> */
		BOOL	TestMode9;					/* Automatic tester mode: Force gate drive signalScanAON/OFF*//* <S19F> */
		BOOL	GateDriveSigTestFlag;		/* Automatic tester mode: Gate drive signal status flag(ScanA)*//* <S19F> */
		BOOL	PreWarningSw;				/* Preventive maintenance warning selection								*//* <S154> */
		BOOL	DcDischarge;				/* DCRapid discharge is selected when power is input.							*//* <S21D> */
		BOOL	CapaLinkFlag;				/* Capacitor bank connection flag								*//* <S21D> */
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct	{							/* Parameter definition for main circuit power supply monitoring							*/
		ULONG	AconWaitTimer;				/* Initial Acon Wait Timer								*/
		ULONG	AcoffChkTimer;				/* AC Power Off Check Timer								*/
		ULONG	DconChkTimer;				/* DC Power On  Check Timer								*/
		ULONG	DischChkTimer;				/* Discharge Check Timer								*/
		ULONG	DischSeqNo;					/* Discharge Sequence Number							*//* <S1C6> */
		ULONG	DischActTimer;				/* Discharge Active Time Counter						*//* <S1C6> */
		ULONG	DischStartVolt;				/* Discharge Start Timing Voltage						*//* <S1C6> */
		ULONG	OpenChkTimer[2];			/* Open Phase Check Timer								*/
		ULONG	UvChkTimer;					/* Under Voltage Check Timer							*/
		ULONG	WireAlmDetWait;				/* Remain DC Power Check Timer							*/
		BOOL	DetOpenAcOn1;				/* ACON1 signal			(Used by Detect Open-Phase)		*/
		BOOL	DetOpenAcOn2;				/* ACON2 signal			(Used by Detect Open-Phase)		*/
		BOOL	Alm330Lt;					/* A.330 in DC mode Detect Flag							*/
		LONG	AcOn1OffCount;				/* ACON1 Off Count Timer(Used by Detect Open-Phase)		*/
		LONG	AcOn2OffCount;				/* ACON2 Off Conut Timer(Used by Detect Open-Phase)		*/
		LONG	AconDetectCount;			/* Acon Active Detect Count in DC mode					*/
		BOOL	PreSigOut;					/* Preventive maintenance output signal										*//* <S154> */
		/* <S21D> Start */
		BOOL	ClinkOut;					/* CLINK Output Signal									*/
		BOOL	CmconOut;					/* CMCON Output Signal									*/
		ULONG	ConvInSigDischErrChkTimer;	/* Converter I/O Signal Discharge Error Check Timer		*/
		LONG	DcVoltOld;					/* DC Volt Before Discharge								*/
		ULONG	ClinkAnsErrChkTimer;		/* Capacitor Bank Link Answer Back Error Check Timer	*/
		ULONG	CmcAnsErrChkTimer;			/* Capacitor Bank MC Answer Back Error Check Timer		*/
		ULONG	ClinkAnsTimer;				/* Clink Answer Check Timer								*/
		BOOL	WrnExRegInhibit;			/* WRN_EXREG Inhibit Flag								*/
		BOOL	CapaChargeOk;				/* Capacitor Bank Charge OK Flag						*/
		/* <S21D> End */
	} var;
/*--------------------------------------------------------------------------------------------------*/
} CHECK_MAIN_POWER;


/*--------------------------------------------------------------------------------------------------*/
/*		DCVariable definition for power supply voltage detection																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct DET_DC_VOLT {
	struct	{							/* DCConstant definition for power supply voltage detection									*/
		UCHAR	VdetMode;					/* Main circuit bus voltage detection method(PnE0C.8,9)					*/
		USHORT	VdetMaxVolt;				/* Amp Volt detect at max input		[data/MaxV]			*/
		LONG	VdetAdNorm;					/* Main circuit detectionADConversion result normalization constant(PnE70)				*/
		LONG	VdetAdFil;					/* Main circuit detectionADFilter gain							*/
		LONG	VdcdetAlmFilter;			/* Main circuit detection abnormality alarm filter(PnE7A.0,1) (1=250us)	*/
		BOOL	VdetPrmOk;					/* Main circuit detection related parameter settings(1:OK, 0:NG)				*/
		LONG	VdetAdadj;					/* PnE5B : Lower:P-NVoltage detection level fine adjustment					*/
		LONG	Kdcvolt;					/* Main circuit detection value conversion gain[Admax] ⇒ [V]				*/
	} conf;
	LONG	DcAdFili;						/* DC Volt Ad read filter output						*/
	LONG	DcAdFilo[3];					/* DC Volt Ad read filter output						*/
	LONG	DcVoltx;						/* DC Volt												*/
	ULONG	DcvErrCnt;						/* DC Volt Error Check Counter							*/
	ULONG	DcvWaitCnt;						/* DC Volt Error Check Wait Counter						*/
	LONG	VdcdetAlmCnt;					/* Main circuit detection alarm detection filter	(1=250us)			*/
	LONG	OvdetAlmCnt;					/* Overvoltage alarm detection filter		(1=250us)			*/
} DET_DC_VOLT;


/*--------------------------------------------------------------------------------------------------*/
/*		Power status structure definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct POWER_STATUS {
	BOOL			Acon;					/* ACPower supply detection status										*/
	BOOL			Acon1Sig;				/* Converter output signal status1								*/
	BOOL			Acon1Sig_l;				/* Converter output signal status1(Previous value)						*/
	BOOL			Acon2Sig;				/* Converter output signal status2(Currently unused)					*/
	BOOL			ChargeOk;				/* DCCharging complete state										*/
	BOOL			Discharge;				/* DCRapid discharge request										*/
	BOOL			PowerOn;				/* Main power supplyONStatus(Power ready waiting time considered)				*/
	BOOL			PowerOnEep;				/* Main power supplyONStatus(EEPROMFor management)							*/
	LONG			DcVolt;					/* P-NVoltage detection value										*/
	BOOL			NegPumpReq;		        /* Negative Pump On Control Request to Kernel<S201>*/
	BOOL			AconDetectReady;	    /* AC ON Signal Detect Ready Flag(FALSE:Not Ready, TRUE:Ready)<S201>*/
	/* <S21D> Start */
	BOOL			ClinkAnsSig;			/* Capacitor bank connection monitoring signal							*/
	BOOL			CmcAnsSig;				/* Rush protection relay status monitoring signal								*/
	BOOL			ConvRdySig;				/* Converter ready signal									*/
	BOOL			ConvMconSig;			/* Inrush current prevention relay signal								*/
	/* <S21D> End */
} POWER_STATUS;

/*--------------------------------------------------------------------------------------------------*/
/*		Intermediate potential abnormality detection structure definition <S053>															*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct PCOV_ERR_DETECT {
	struct {							/* Parameter definition for intermediate potential abnormality detection							*/
		BOOL		PcovEna;				/* Intermediate potential abnormality detection enabled									*/
		ULONG		PcovErrCntLmt;			/* Intermediate potential abnormality detection time constant(Count value)					*/
	} conf;
	struct {							/* Variable definition for intermediate potential abnormality detection								*/
		BOOL		Pcov;					/* Intermediate potential abnormal signal(Positive logic)								*/
		ULONG		PcovErrCnt;				/* Intermediate potential abnormality count									*/
	} var;
} PCOV_ERR_DETECT;


/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for inrush prevention resistance overload check														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_RUSH {
	LONG	PowerOnChk;						/* Inrush resistance overload : Last PowerOn						*/
	LONG	PowerOnCnt[2];					/* Inrush resistance overload : Off-->On Counter					*/
	ULONG	PowerOnChkCycle;				/* Inrush resistance overload : Check Cycle Counter					*/
} CHECK_RUSH;


/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for regenerative overload check																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_REGOL {
	struct	{								/* Constant definition for regenerative overload check							*/
		LONG	Krgpower;						/* Regeneration overload : Power calculation gain						*/
		LONG	RgolPowBase;					/* Regeneration overload : Power base							*/
		LONG	RgolAlmLevel;					/* Regeneration overload : Alarm level						*/
		LONG	Kunrgpower;						/* Regenerative power consumption calculation gain(for Un00A)				*/
		LONG	KunrgpowerCycle;				/* Regenerative power consumption calculation gain(for Un143)				*/
		LONG	RgSumSel;						/* Initial value ratio selection for regenerative overload detection(PnE02.6,7)		*/
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	struct	{								/* Variable definition for regenerative overload check							*/
		LONG	RgolPowSum;						/* Regeneration overload : Integrated power value							*/
		BOOL	LastRgolDetect;					/* Regeneration overload : Regenerative overload detection flag at previous startup	*/
		BOOL	RgPowUp;						/* Regeneration overload : Regenerative power increase flag					*/
		LONG	RgPowerSum;						/* Regeneration average value : Integrated power value							*/
		LONG	RgMeanCycle;					/* Regeneration average value : Number of power integration times						*/
	} var;
} CHECK_REGOL;


/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for regeneration abnormality check																*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_REGENE {
	LONG	RegeneErrCnt;					/* Regenerative abnormality check count								*/
	BOOL	RegeAlmSignal;					/* Regenerative abnormality alarm flag								*/
} CHECK_REGENE;


/*--------------------------------------------------------------------------------------------------*/
/*		Regeneration control structure definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct REGENE_CTRL {
	struct	{								/* Define constants for regenerative control									*/
		ULONG			ResistorCapa;			/* Regenerative resistor capacity(Pn600)								*/
		BOOL			RegRxIn;				/* Built-in regenerative resistor(PnE0C.0)							*/
		LONG			RegStartFilter;			/* Regeneration operation start filter(PnE7A.4,5)	(1=250us)		*/
		LONG			RegStopFilter;			/* Regeneration operation start filter(PnE7A.6,7)	(1=250us)		*/
		USHORT			RegOnLevel;				/* RegenerationTr ONLevel									*/
		USHORT			RegOffLevel;			/* RegenerationTr OFFLevel									*/
		USHORT			RegSyn;					/* Regeneration synchronization function selection								*//* <S21D> */
		USHORT			RegSynWaitTime;			/* Regeneration during regeneration synchronizationONWaiting time(PnE9B)				*//* <S21D> */
	} conf;
/*--------------------------------------------------------------------------------------------------*/
	LONG			RegStartCnt;			/* Regeneration operation start wait counter								*/
	LONG			RegStopCnt;				/* Regeneration operation end wait counter								*/
	LONG			RegOnCount[2];			/* Regeneration transistorONTime 0:ScanBUpdate, 1:ScanCUpdate		*/
	LONG			dRegOnCount;			/* Differential regenerative transistorONTime			[Count/ScanC]	*/
	BOOL			RegOn;					/* Regeneration transistorONRequest								*/
	BOOL			RegAlmSig;				/* Regeneration error detection signal										*/
	BOOL			RegAlmMotStop;			/* Motor stop detection flag after regenerative overload alarm occurs			*/
	CHECK_REGOL		RegOL;					/* Pointer to variable for regenerative overload check(Common to units)	*/
	CHECK_REGENE	RegChk;					/* Variable for regeneration abnormality check(Common to units)					*/
	LONG			UnMeanRgPower;			/* Regenerative power consumption average value	(Unit common: For monitor)	[%]			*//* <S123> */
} REGENE_CTRL;


/*--------------------------------------------------------------------------------------------------*/
/*		Regeneration control setting parameter structure definition															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct REGCFG_PRM {
	USHORT			regres;					/* Pn600 : Regenerative resistor capacity									*/
	USHORT			exreger;				/* Pn603 : Regenerative resistance value(Default)						*/
	USHORT			reger;					/* PnE1A : Regenerative resistance value									*/
	USHORT			wre;					/* PnE1C : Regenerative resistor capacity(Default)						*/
	USHORT			regonlvl;				/* PnE72 : Regeneration on level								*/
	USHORT			ConvW;					/* PnE7F : Converter capacity(System)						*/
} REGCFG_PRM;


/*--------------------------------------------------------------------------------------------------*/
/*		Definition of the structure for detecting the stoppage of the built-in fan of the Servopack	<S0A3>											*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CHECK_FANLOCK {
	struct	{							/* Constant definition for built-in fan stop detection								*/
		USHORT	FanLockDetectTime;		/* Fan stop detection time(Continuous)									*/
		ULONG	FanLockAlmDetectTime;	/* Built-in fan stop alarm detection time when servo is on(Setting value)		*/	/* <S219> */
	} conf;
	struct	{							/* Variable definition for built-in fan stop detection								*/
		USHORT	FanLockStartCnt;		/* Fan stop detection start time count							*/
		USHORT	FanLockTimeCnt;			/* Fan stop detection time count								*/
		BOOL	FanLockWrnFlag;			/* Fan stop warning occurrence flag							*/	/* <S219> */
		ULONG	WrnStartTime;			/* Cumulative operating time[100ms](When fan stop warning occurs)			*/	/* <S219> */
		ULONG	FanLockWrnElapsedTime;	/* Built-in fan stop alarm detection time remaining when servo is on			*/	/* <S219> */
	} var;
} CHECK_FANLOCK;


/*--------------------------------------------------------------------------------------------------*/
/*		Servo power management structure definition																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct POW_MNG_HNDL {
	CHECK_MAIN_POWER	MainPowerChecker;			/* Main circuit monitoring variables(Common to units)				*/
	DET_DC_VOLT			DetectDcVolt;				/* DCData for power supply voltage detection(Common to units)			*/
	POWER_STATUS		PowerSts;					/* Power status(Common to units)						*/
	REGENE_CTRL			RegeneCtrl;					/* Regenerative transistor control data(Common to units)	*/
	CHECK_RUSH			Rush;						/* Inrush prevention resistor overload detection variable(Common to units)	*/
	PCOV_ERR_DETECT		PcovErrDetect;				/* Variable for intermediate potential abnormality detection(Common to units) <S053>	*/
	CHECK_FANLOCK		FanLock;					/* Servopack built-in fan stop detection variable			*//* <S0A3> */
} POW_MNG_HNDL;



/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for power consumption calculation	<S020>															*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct CALC_WATTAGE {
	struct	{								/* Constant definition for power consumption calculation								*/
		float			CurrentNorm1;			/* Normalized value1(Consider a suitable name later)		*/
		float			CurrentNorm2;			/* Normalized value2(Consider a suitable name later)		*/
		float			CurrentNorm3;			/* Normalized value3(Consider a suitable name later)		*/
		USHORT			Imax;					/* Maximum current value(Are the smaller values ​​of the amplifier and motor)				*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct	{								/* Constant definition for power consumption calculation								*/
		float			PowerConsumption;		/* Instantaneous power consumption						[W]				*/
		float			PowerConsumption_a;		/* Instantaneous power consumption						[W]				*//* <S039> */
	} var;

} CALC_WATTAGE;
/*--------------------------------------------------------------------------------------------------*/
typedef	struct POWER_MONITOR {
	struct	{								/* Constant definition for power consumption calculation								*/
		ULONG			TimerCnt1s;				/* 1sMeasurement counter									*/
		ULONG			TimerCnt10s;			/* 10sMeasurement counter								*/
		float			CumulativeEnergy;		/* 1Cumulative power consumption per second			[Wh]			*/
		float			PonCumulativeEnergy;	/* Cumulative power consumption since power-on		[Wh]			*/
		LONG			UnConsumedPower;		/* Un032 : Power consumption					[W]				*/
		LONG			UnEnergyConsumption;	/* Un033 : Power consumption 				[Wh]			*/
		LONG			UnCumulativeEnergyI;	/* Un034 : Cumulative power consumption(Integer part)	[Wh]			*/
		LONG			UnCumulativeEnergyD;	/* Un035 : Cumulative power consumption(Decimal part)	[0.001Wh]		*//* <S0A5> */
	} var;
} POWER_MONITOR;



/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		ACFor power supply phase loss detection processing																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	ACOPEN_DETECT_WAIT			KPI_SCANC_MS( 1000 )		/* Open phase detection time[ms]					*/

/*--------------------------------------------------------------------------------------------------*/
/*		DCFor power monitoring																				*/
/*		<S053>Change the processing to refer to the following parameters.													*/
/*				PnE88: DCCharging completion waiting time[ms]														*/
/*				PnE89: Power ready waiting time[ms]														*/
/*--------------------------------------------------------------------------------------------------*/
//#define	POWCHARG_WAIT				KPI_SCANC_MS( 180 )		/* DCCharging completion waiting time : 180ms		*/
//#define	POWREADY_WAIT				KPI_SCANC_MS( 240 )		/* Power ready waiting time : 240ms		*/
/* <S21D> Start */
#define CONVINSIGDISCHERRCHK_TIME	KPI_SCANC_MS( 1000 )	/* A.331(power supply monitoring input signal abnormality) Rapid discharge abnormality detection time:1s	*/
#define CLINKANSERRCHK_TIME			KPI_SCANC_MS( 500 )		/* A.332(Capacitor bank connection answer error) Detection time:500ms	*/
#define CMCANSERRCHK_TIME			KPI_SCANC_MS( 500 )		/* A.333(Capacitor bankMCAnswer error) Detection time:500ms	*/
#define CMCON_WAIT					KPI_SCANC_MS( 10000 )	/* /CMCON Signal output waiting time:10000ms		*/
#define DCOFF_FILTER				100U					/* DCMomentary power failure holding time filter:100ms		*/
/* <S21D> End */
/*--------------------------------------------------------------------------------------------------*/
/*		DCFor power supply rapid discharge processing									<S1C6>								*/
/*--------------------------------------------------------------------------------------------------*/
#define DCVOLT60V					30							/* DcVolt but 60VExceeding					*/
#define DCVOLT10V					5							/* A.331Voltage value for detection:10V			*//* <S21D> */
#define DISCH_CHK_TIME				20							/* Rapid discharge confirmation time[ms](x3times)		*/
#define DISCH_CHK_VOLT				10							/* Rapid discharge confirmation threshold voltage[V](x3times)	*/
enum {
	DISCH_INIT = 0,		/* Rapid discharge sequence initial state 												*/
	DISCH_CHK1ST,		/* Rapid discharge confirmation number1times 														*/
	DISCH_CHK2ND,		/* Rapid discharge confirmation number2times 														*/
	DISCH_CHK3RD,		/* Rapid discharge confirmation number3times 														*/
	DISCH_ACTIVE,		/* Rapid discharge in progress 															*/
	DISCH_FINISH,		/* Rapid discharge completion		 														*/
};

/*--------------------------------------------------------------------------------------------------*/
/*		DCDefinition for power supply voltage detection																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	DETECT_DC_NO_OPERATION		0x00

/*--------------------------------------------------------------------------------------------------*/
/*		Definition for regenerative overload detection																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	CHK_REGOL_NO_OPERATION		0x0000						/* No alarm(Processing not required)			*/
#define	CHK_REGOL_SET_ALARM			0x0001						/* All axes alarm output					*/
#define	CHK_REGOL_SET_WARNING		0x0010						/* All axes warning output				*/
#define	CHK_REGOL_CLR_WARNING		0x0100						/* All axes warning clear				*/
/* <S21D> Start */
#define REGSYNOFF					0U				/* Regeneration synchronization function is not used						*/
#define PNREGSYN					1U				/* PNUse regeneration synchronization function in bus common system	*/
#define PNEXREG						2U				/* PNUse external regeneration function in bus common system		*/
/* <S21D> End */

/*--------------------------------------------------------------------------------------------------*/
/*		Inrush prevention resistor overload detection definition																*/
/*--------------------------------------------------------------------------------------------------*/
#define	CHK_RUSH_NO_OPERATION		0x0000						/* No alarm(Processing not required)			*/
#define	CHK_RUSH_SET_ALARM			0x0001						/* All axes alarm output					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Regeneration error detection definition																			*/
/*--------------------------------------------------------------------------------------------------*/
#define	CHK_REGENE_NO_OPERATION		0x0000						/* No alarm(Processing not required)			*/
#define	CHK_REGENE_SET_ALARM		0x0001						/* All axes alarm output					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Definition for detecting the stoppage of the built-in fan of the Servopack	<S0A3>												*/
/*--------------------------------------------------------------------------------------------------*/
#define	CHK_FANLOCK_NO_OPERATION	0x0000						/* No alarm(Processing not required)			*/
#define	CHK_FANLOCK_SET_ALARM		0x0001						/* All axes alarm output					*/
#define	CHK_FANLOCK_SET_WARNING		0x0010						/* All axes warning output				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Initial value ratio selection definition for regenerative overload detection															*/
/*--------------------------------------------------------------------------------------------------*/
#define REGPOWSUM_RATE95PER			0x00						/* Regenerative overload alarm level95%	*/
#define REGPOWSUM_RATE50PER			0x01						/* Regenerative overload alarm level50%	*/

/*--------------------------------------------------------------------------------------------------*/
/*		Main circuit bus voltage detection method																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	VDET_NONE					0x00		/* No detection by software(SGDM/SGDHType)		*/
#define	VDET_PWM_FIX_FOLLOW			0x01		/* PWM-Comparator method : Fixed value(1)		*/
#define	VDET_PWM_VAR_FOLLOW			0x02		/* PWM-Comparator method : Variable value(1-3)		*/
#define	VDET_AD						0x03		/* A/DConversion method										*/
/*--------------------------------------------------------------------------------------------------*/
#define	KPX_VDET_PWM_MAX			255							/* 8bit PWM	(2^n - 1)				*/
#define	KPX_DCVOLT_ADMAX			1023

/*--------------------------------------------------------------------------------------------------*/
/*		Inrush/DBControl method definition																			*/
/*--------------------------------------------------------------------------------------------------*/
#define DBCTRL_RLY_COMMON			0x00		/* Inrush + DB: Relay common							*/
#define DBCTRL_TRIACK_SEPARATE		0x01		/* Rush: Relay; DB: Thyristor					*/
#define DBCTRL_RLY_SEPARATE			0x02		/* Rush: Relay; DB: Relay						*/
#define DBCTRL_TRIACK_COMMON		0x03		/* Inrush + DB:Relay + thyristor					*/
#define WAIT_RLY_DB					20			/* RLYONAfter outputDBONWait time until output[us] 		*//* <S20E> */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
//void	PcmPowerManager( POW_MNG_HNDL *hPowerManager, BOOL MotStopSts );							  /* <S0F6> */
void	PcmPowerManager( POW_MNG_HNDL *hPowerManager, BOOL MotStopSts, BOOL BeSts );				  /* <S0F6> */
void	PcmRegeneControl( REGENE_CTRL *pRegCtrl, POWER_STATUS *PowSts, BOOL MotStop );
LONG	PcmDetectDcVolt( DET_DC_VOLT *pDetectDc, POWER_STATUS *PowSts );
//LONG	PcmCheckOverVoltage( CHECK_MAIN_POWER *PowerChecker, DET_DC_VOLT *pDetectDc, POWER_STATUS *PowSts ); /* <S0F7> */
LONG	PcmCheckOverVoltage( CHECK_MAIN_POWER *PowerChecker, DET_DC_VOLT *pDetectDc,
														POWER_STATUS *PowSts, REGENE_CTRL *RegCtrl ); /* <S0F7> */
LONG	PcmCheckRegeneOverLoad( REGENE_CTRL *pRegCtrl, ULONG *AlmOLInfo, ANLG_UN_MON_DATA *MonData );
void	SetRegeneOverLoadSts( LONG RegOlSts );
BOOL	PcmCheckRushRxOverLoad( CHECK_RUSH *pRush, POWER_STATUS *pPowerSts );
BOOL	PcmCheckRegeneAlarm( REGENE_CTRL *pRegCtrl, BOOL PowerOn );
LONG	PcmCheckFanLock( CHECK_FANLOCK *FanLock, BOOL FanStop, BOOL BbSts );						  /* <S0A3> */
void	PcmInitPowerManager( POW_MNG_HNDL *hPowerManager, ULONG AlmOLInfo );
void	PcmCountRegeneOnTime( POW_MNG_HNDL *hPowerManager );
void	PcmInputPowerStatus( POW_MNG_HNDL *hPowerManager, BOOL Acon1, BOOL RegeneAlm );

//void	IprmcalRegenePowerGain( REGENE_CTRL *RegeneCtrl, BPRMDAT *Bprm, USHORT regonlvl, USHORT	reger, USHORT ConvW);
void	IprmcalRegenePowerGain( REGENE_CTRL *RegeneCtrl, BPRMDAT *Bprm, REGCFG_PRM *RegPrm );
BOOL	PcalRegeneOverLoadLevel( REGENE_CTRL *RegeneCtrl,USHORT regres, USHORT wre, USHORT ConvW );
void	PcmCalculateVdetAdFil( DET_DC_VOLT *pDetectDc, USHORT vdetadfil, LONG SvCycleUs );

void	PcmRlyDbControlProcess( ASIC_HW_IF_STR *pAsicHwReg, POW_MNG_HNDL *PowerManager,
									BOOL BbSts, BOOL DbOn, BOOL KnlMode, BOOL MotorLessTestMode );	  /* <S20E> */
LONG	PcmGetDcVoltage( POWER_STATUS *PowSts );													  /* <S020> */

void	PrmCalcPowerMonitor( CALC_WATTAGE *CalcWattage, USHORT AmpImax, USHORT MotImax );			  /* <S020> */
void	InitializePowerMonitor( POWER_MONITOR *PowMon );											  /* <S020> */
void	CalcPowerMoniter( CALC_WATTAGE *CalcWattage, SHORT PowMonVq, SHORT PowMonIqd,				  /* <S020> *//* <S039> */
							SHORT PowMonVd, SHORT PowMonIdd, LONG DcVolt, BOOL BaseEnable );
void	SetPowerMoniter( POWER_MONITOR *PowMon, float PowerConsumption );							  /* <S020> */
void	ResetPowerConsumption( CALC_WATTAGE *CalcWattage );											  /* <S020> */
float	GetPowerConsumption( CALC_WATTAGE *CalcWattage );											  /* <S020> */

#define	_POWER_MANAGER_H_
#endif	/* _POWER_MANAGER_H_ */
/***************************************** end of file **********************************************/

#ifndef _OVERHEAT_MANAGER_H
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		OverheatManager.h : Temperature overheating monitoring process definition							<S0ED>					*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Defining the process used for temperature overheating monitoring														*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2014.1.22	Y.Oka	For Mercury												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include	"Basedef.h"
#include	"AlarmManager.h"

/*--------------------------------------------------------------------------------------------------*/
/*		Definition for internal temperature																				*//* <S0DE> */
/*--------------------------------------------------------------------------------------------------*/
#define CTRL_BOARD_TMPR				0U			/* Control circuit temperature										*/
#define POWER_BOARD_TMPR			1U			/* Power circuit temperature									*/
#define SERXERR_THREE				2U			/* Communication error counter occurred three times				*/
#define CTRL_BOARD					1			/* Control board											*/
#define POWER_BOARD					2			/* Power board										*/
#define CHK_OVERHEAT_NO_OPERATION	0x0000		/* No alarm(Processing not required)							*/
#define CHK_ERROR_SET_ALARM			0x0001		/* Internal temperature detection section alarm output						*/
#define CHK_OVERHEAT_SET_ALARM		0x0002		/* Internal temperature abnormality alarm output							*/
#define CHK_OVERHEAT_SET_WARNING	0x0003		/* Internal temperature warning output							*/
#define CHK_OVERHEAT_CLR_WARNING	0x0004		/* Internal temperature warning release							*/



/*--------------------------------------------------------------------------------------------------*/
/*		Installation environment monitor structure definition												<S03A>				*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct ENVMON {
	struct	{							/* Installation environment monitor constant definition									*/
		SHORT	AmpEnvMonOffset;			/* Amplifier installation environment monitor offset value						*/
		SHORT	MotEnvMonOffset;			/* Motor installation environment monitor offset value						*/
	} conf;
	struct	{							/* Installation environment monitor variable definition									*/
		USHORT	EnvMonSumCnt20ms;			/* Installation environment monitor cumulative count(20msFor measurement)				*/
		USHORT	EnvMonSumCnt2s;				/* Installation environment monitor cumulative count(2sFor measurement)					*/
		LONG	AmpEnvMonSum;				/* Amplifier installation environment monitor integrated value							*//* <S050> */
		LONG	MotEnvMonSum;				/* Motor installation environment monitor integrated value							*//* <S050> */
		LONG	UnInstEnvMonAmp;			/* Un025 : Installation environment monitor(2sUpdate)	[0.001%]			*//* <S050> */
		LONG	UnInstEnvMonMot;			/* Un026 : Installation environment monitor(2sUpdate)	[0.001%]			*//* <S050> */
	} var;
} ENVMON;


/*--------------------------------------------------------------------------------------------------*/
/*		Structure definition for overheat monitoring											<S03A>				*/
/*--------------------------------------------------------------------------------------------------*/
enum _enumBoardTmprState/* <S0DE> */
{
	BRD_TEMP_NORMAL		= 0L,							/* Board temperature normal								*/
	BRD_WRN_DURING		= 1L,							/* Board warning in progress							*/
	BRD_ALARM_DURING	= 2L,							/* In board alarm state						*/
	BRD_ERROR			= 3L							/* BoardERROR								*/
};

typedef	struct CHECK_INTTMPR {/* <S0DE> */
	struct	{
		USHORT	BrdAlmTmpr;								/* Board abnormal temperature[10℃]						*/
		USHORT	BrdWrnTmpr;								/* Board warning temperature[10℃]						*/
		USHORT	BrdAlmClrTmpr;							/* Board abnormality clearable temperature[10℃]				*/
		USHORT	BrdAlmDetTime;							/* Board abnormal temperature detection time[10℃]				*/
	} conf;
	struct	{
		enum _enumBoardTmprState	BrdState;			/* Board status									*/
//		ULONG						IntTmprTimeCnt;		/* Counter for alarm output				*//* <S1D1> */
		ULONG						SerxErrCnt;			/* Communication error counter						*/
//		USHORT	DirectAlmDetTime;						/* Board abnormal temperature detection time[10℃]				*//* <S1D1> */
		USHORT	WrnDetectTimer;							/* Board warning temperature detection time[10Second]				*//* <S1D1> */
		USHORT	AlmDetectTimer;							/* Board abnormal temperature detection time[10Second]				*//* <S1D1> */
	} var;
} CHECK_INTTMPR;

typedef	struct CHECK_OH {
	CHECK_INTTMPR	CtrlTmprAlm;			/* For control board temperature abnormality monitoring								*//* <S0DE> */
	CHECK_INTTMPR	PwrTmprAlm;				/* For power board temperature abnormality monitoring								*//* <S0DE> */
	struct	{
		SHORT	CtrlBoadTempSum;			/* Control board ambient temperature calculation								*//* <S050> */
		SHORT	PowerBoadTempSum;			/* For power board ambient temperature calculation								*//* <S050> */
		USHORT	TemperatureSumCnt;			/* For overheat calculation									*/
		USHORT	TemperatureErrCnt;			/* Overheat abnormality count							*/
		USHORT	TemperatureMonCnt;			/* Elapsed time count for overheat check				*/
		USHORT	TemperatureScanCnt;			/* Overheat abnormal scan count					*/
		LONG	DetTempFistPastTime;		/* Initial state monitoring counter								*/
		BOOL	DetTempFirstPass;			/* Temperature detection first monitoring flag								*/
	} var;
/*--------------------------------------------------------------------------------------------------*/
	struct	{
		SHORT	UnAverageTemp;				/* Un170 : Average internal temperature				[0.01deg]			*//* <S050> */
		SHORT	UnCtrlBoadTemp;				/* Un171 : Control board ambient temperature			[0.1deg]			*//* <S050> */
		SHORT	UnPowerBoadTemp;			/* Un172 : Power board ambient temperature		[0.1deg]			*//* <S050> */
	} UnMon;
} CHECK_OH;



/****************************************************************************************************/
/*		Global's																						*/
/****************************************************************************************************/
void	OhmInitialize( CHECK_OH* pCheckOh );
void	EnvMonInitialize( ENVMON* pEnvMon );
void	OhmCheckOverHeatAlm( ALMMAN_HANDLE *AlmMngr, CHECK_OH *CheckOh,
													SHORT CtrlBoadTemp, SHORT PowerBoadTemp );
void	AdetCheckTempDeviceErr( ALMMAN_HANDLE *AlmMngr, USHORT CtrlBoadErrCnt,
																	USHORT PowerBoadErrCnt );
void	AdetInstEnvMonitor( ENVMON *EnvMon, SHORT AmpTemp, SHORT MotTemp );



#define _OVERHEAT_MANAGER_H
#endif	// _OVERHEAT_MANAGER_H
/***************************************** end of file **********************************************/

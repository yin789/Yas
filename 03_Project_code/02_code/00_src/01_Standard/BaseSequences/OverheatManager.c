/****************************************************************************************************/
/*																									*/
/*																									*/
/*		OverheatManager.c : Temperature overheating monitoring process definition					<S0ED>							*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitor the acquired temperature and perform this if the temperature overheat alarm is not acquired/Outputs warning							*/
/*																									*/
/*			Also,UnCreate temperature data to display, calculate installation environment monitor values,API			*/
/*			conduct.																					*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2014.1.22	Y.Oka	For Mercury												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include	"OverheatManager.h"
#include	"Mlib.h"



/****************************************************************************************************/
/*		API's																						*/
/****************************************************************************************************/
/*	void	OhmInitialize( CHECK_OH* pCheckOh );													*/
/*	void	EnvMonInitialize( ENVMON* pEnvMon );													*/
/*	void	OhmCheckOverHeatAlm( ALMMAN_HANDLE *AlmMngr, CHECK_OH *CheckOh,							*/
/*														SHORT CtrlBoadTemp, SHORT PowerBoadTemp );	*/
/*	void	AdetCheckTempDeviceErr( ALMMAN_HANDLE *AlmMngr, USHORT CtrlBoadErrCnt,					*/
/*																		USHORT PowerBoadErrCnt );	*/
/*	void	AdetInstEnvMonitor( ENVMON *EnvMon, SHORT AmpTemp, SHORT MotTemp );						*/
/****************************************************************************************************/



/****************************************************************************************************/
/*		Static Function Definition																	*/
/****************************************************************************************************/
static LONG	AdetCheckFirstBoardTmpr( CHECK_INTTMPR *IntTmprAlm, SHORT Temperature );
static LONG	AdetCheckBoardTmpr( CHECK_INTTMPR *IntTmprAlm, SHORT Temperature );
static void	AdetSetBoardTmprAlm( ALMMAN_HANDLE *AlmMngr, LONG DetectAlm, LONG BoardSel );
/****************************************************************************************************/



/****************************************************************************************************/
/*																									*/
/*		Overheat monitoring module initialization										<S03A>				*/
/*																									*/
/****************************************************************************************************/
void OhmInitialize( CHECK_OH* pCheckOh )
{
	MlibResetLongMemory( &pCheckOh->var, sizeof( pCheckOh->var )/4 );
	MlibResetLongMemory( &pCheckOh->CtrlTmprAlm.var, sizeof( pCheckOh->CtrlTmprAlm.var )/4 );
	MlibResetLongMemory( &pCheckOh->PwrTmprAlm.var, sizeof( pCheckOh->PwrTmprAlm.var )/4 );
	MlibResetLongMemory( &pCheckOh->var, sizeof( pCheckOh->var )/4 );
}



/****************************************************************************************************/
/*																									*/
/*		Installation environment monitor initialization													<S03A>				*/
/*																									*/
/****************************************************************************************************/
void EnvMonInitialize( ENVMON* pEnvMon )
{
	MlibResetLongMemory( &pEnvMon->var, sizeof( pEnvMon->var )/4 );

}



/****************************************************************************************************/
/*																									*/
/*		Servopack overheat monitoring											<S03A>				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Temperature detectionICto prevent damage and malfunction.								*/
/*																									*/
/*	Immediately after control power is turned onOHCompare with the alarm clearable temperature, and if it exceeds, set an immediate alarm.	*/
/*	Initial state is1seconds and then transition to normal temperature monitoring. During normal temperature monitoring, the warning level is set to		*/
/*	will output a warning, and if continued use thereafter, it will transition to an alarm.				*/
/*																									*/
/****************************************************************************************************/
void	OhmCheckOverHeatAlm( ALMMAN_HANDLE *AlmMngr, CHECK_OH *CheckOh,
														SHORT CtrlBoadTemp, SHORT PowerBoadTemp )
{
//	SHORT	MaxTemp;	/* <S050> */
	LONG	DetAlm;		/* <S0DE> */

	/*----------------------------------------------------------------------------------------------*/
	/*	Inside temperature relatedUnMonitor data creation											<S0DE>		 		*/
	/*----------------------------------------------------------------------------------------------*/
	/* Temperature detection value addition */
	CheckOh->var.CtrlBoadTempSum += CtrlBoadTemp;
	CheckOh->var.PowerBoadTempSum += PowerBoadTemp;
	if( ++CheckOh->var.TemperatureSumCnt >= 10 )
	{
		/* Un170 : Average internal temperature[0.01deg] */
		CheckOh->UnMon.UnAverageTemp =
				((CheckOh->var.CtrlBoadTempSum + CheckOh->var.PowerBoadTempSum) >> 1);
		/* Un171 : Control board ambient temperature[0.1deg] */
		CheckOh->UnMon.UnCtrlBoadTemp = (SHORT)(CheckOh->var.CtrlBoadTempSum * 0.1f);
		/* Un172 : Power board ambient temperature[0.1deg] */
		CheckOh->UnMon.UnPowerBoadTemp = (SHORT)(CheckOh->var.PowerBoadTempSum * 0.1f);
		/* Initialize */
		CheckOh->var.CtrlBoadTempSum = 0;
		CheckOh->var.PowerBoadTempSum = 0;
		CheckOh->var.TemperatureSumCnt = 0;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Overheat monitoring														<S0DE>		 		*/
	/*----------------------------------------------------------------------------------------------*/
	if( CheckOh->var.DetTempFirstPass == FALSE )
	{ /* Temperature detection first time */
		/* Control board temperature abnormality alarm check process */
		DetAlm = AdetCheckFirstBoardTmpr( &(CheckOh->CtrlTmprAlm), CtrlBoadTemp );
		/* Temperature abnormality alarm setting process */
		AdetSetBoardTmprAlm( AlmMngr, DetAlm, CTRL_BOARD );

		/* Power board temperature abnormality alarm check process */
		DetAlm = AdetCheckFirstBoardTmpr( &(CheckOh->PwrTmprAlm), PowerBoadTemp );
		/* Temperature abnormality alarm setting process */
		AdetSetBoardTmprAlm( AlmMngr, DetAlm, POWER_BOARD );

		/* Count up the elapsed time */
		CheckOh->var.DetTempFistPastTime++;
		if( CheckOh->var.DetTempFistPastTime > KPI_SCANC_MS(1000) )
		{
			/* Initial monitoring passing flag set */
			CheckOh->var.DetTempFirstPass = TRUE;
		}
	}
	else
	{ /* Normal state */
		/* Control board temperature abnormality alarm check process */
		DetAlm = AdetCheckBoardTmpr( &(CheckOh->CtrlTmprAlm), CtrlBoadTemp );
		/* Temperature abnormality alarm setting process */
		AdetSetBoardTmprAlm( AlmMngr, DetAlm, CTRL_BOARD );

		/* Power board temperature abnormality alarm check process */
		DetAlm = AdetCheckBoardTmpr( &(CheckOh->PwrTmprAlm), PowerBoadTemp );
		/* Temperature abnormality alarm setting process */
		AdetSetBoardTmprAlm( AlmMngr, DetAlm, POWER_BOARD );
	}

	return;
}



/*****************************************************************************
  Description: Internal board temperature abnormality alarm check initial processing				<S0DE>
------------------------------------------------------------------------------
  Parameters:
	IntTempAlm				Internal temperature abnormality
	Temperature				Board temperature information
  Return:
	rc						Return value
------------------------------------------------------------------------------
  Note:
	Monitoring internal board temperature abnormality alarm
*****************************************************************************/
static LONG	AdetCheckFirstBoardTmpr( CHECK_INTTMPR *IntTmprAlm, SHORT Temperature )
{
	LONG	rc;
	rc = CHK_OVERHEAT_NO_OPERATION;

	if( Temperature > IntTmprAlm->conf.BrdAlmClrTmpr )
	{ /* Internal board temperature exceeds the abnormality clearing temperature */
//		if( IntTmprAlm->var.DirectAlmDetTime > KPI_SCANC_MS( 500 ) ) /* <S1D1> */
		if( IntTmprAlm->var.AlmDetectTimer > KPI_SCANC_MS( 10000 ) ) /* <S1D1> */
		{
			IntTmprAlm->var.BrdState = BRD_ALARM_DURING;
			rc = CHK_OVERHEAT_SET_ALARM;
		}
		else
		{
//			IntTmprAlm->var.BrdState++;				/* <S109> */
			IntTmprAlm->var.AlmDetectTimer++;		/* <S109> */
		}
	}
	else
	{
//		IntTmprAlm->var.BrdState = 0;				/* <S109> */
		IntTmprAlm->var.AlmDetectTimer = 0;			/* <S109> */
	}
	return( rc );
}



/*****************************************************************************
  Description: Internal board temperature abnormal alarm check processing					<S0DE>
------------------------------------------------------------------------------
  Parameters:
	IntTempAlm				Internal temperature abnormality
	Temperature				Board temperature information
  Return:
	rc						Return value
------------------------------------------------------------------------------
  Note:
	Monitoring internal board temperature abnormality alarm
*****************************************************************************/
static LONG	AdetCheckBoardTmpr( CHECK_INTTMPR *IntTmprAlm, SHORT Temperature )
{
	LONG	rc;
	rc = CHK_OVERHEAT_NO_OPERATION;

	switch ( IntTmprAlm->var.BrdState )
	{
	case BRD_TEMP_NORMAL:
		/* When the internal temperature exceeds the warning temperature */
		if( Temperature > IntTmprAlm->conf.BrdWrnTmpr )
		{
//			if( IntTmprAlm->var.WrnDetectTimer > KPI_SCANC_MS( 500 ) ) /* <S11D> */
			if( IntTmprAlm->var.WrnDetectTimer > KPI_SCANC_MS( 10000 ) )
			{
				rc = CHK_OVERHEAT_SET_WARNING;
//				IntTmprAlm->var.IntTmprTimeCnt = 0U;					/* <S1D1> */
				IntTmprAlm->var.BrdState = BRD_WRN_DURING;
			}
			else
			{
				IntTmprAlm->var.WrnDetectTimer++;
			}
		}
		else
		{
			IntTmprAlm->var.WrnDetectTimer = 0;
		}
		break;

	case BRD_WRN_DURING:
//		IntTmprAlm->var.IntTmprTimeCnt++; /* <S1D1> */
		/* If the warning state exceeds the specified time, or if the abnormality detection threshold temperature is exceeded, an alarm occurs *//* <S1D1> */
//		if( (IntTmprAlm->var.IntTmprTimeCnt >= (IntTmprAlm->conf.BrdAlmDetTime * KPI_SCANC_MS( 1000 )))
//			|| (Temperature > IntTmprAlm->conf.BrdAlmTmpr) )
		if( (Temperature > IntTmprAlm->conf.BrdAlmTmpr) )
		{
			if( IntTmprAlm->var.AlmDetectTimer > KPI_SCANC_MS( 10000 ) )/* <S1D1> */
			{
				rc = CHK_OVERHEAT_SET_ALARM;
//				IntTmprAlm->var.IntTmprTimeCnt = 0U;					/* <S1D1> */
				IntTmprAlm->var.BrdState = BRD_ALARM_DURING;
			}
			else
			{
				IntTmprAlm->var.AlmDetectTimer++;
			}
		}
		/* When the internal temperature falls below the warning temperature */
		else if( Temperature < IntTmprAlm->conf.BrdWrnTmpr )
		{
			rc = CHK_OVERHEAT_CLR_WARNING;
//			IntTmprAlm->var.IntTmprTimeCnt = 0U;						/* <S1D1> */
			IntTmprAlm->var.BrdState = BRD_TEMP_NORMAL;
		}
		else
		{
			rc = CHK_OVERHEAT_SET_WARNING;								/* <S1D1> */
			IntTmprAlm->var.AlmDetectTimer = 0;							/* <S1D1> */
		}
		break;

	case BRD_ALARM_DURING:
		/* Internal board temperature exceeds the abnormality clearing temperature */
		if( Temperature > IntTmprAlm->conf.BrdAlmClrTmpr )
		{
			rc = CHK_OVERHEAT_SET_ALARM;
		}
		else
		{
			IntTmprAlm->var.BrdState = BRD_TEMP_NORMAL;
		}
		break;

	default:
		IntTmprAlm->var.BrdState = BRD_TEMP_NORMAL;
//		IntTmprAlm->var.IntTmprTimeCnt = 0U;							/* <S1D1> */
		break;

	}
	return( rc );
}



/*****************************************************************************
  Description: Internal temperature abnormality alarm setting processing							<S0DE>
------------------------------------------------------------------------------
  Parameters:
	AlmMngr					Alarm information
	DetectAlm				Alarm output judgment
	BoardSel				Select board
  Return:

------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static void	AdetSetBoardTmprAlm( ALMMAN_HANDLE *AlmMngr, LONG DetectAlm, LONG BoardSel )
{
	if( BoardSel == CTRL_BOARD )
	{
		if( DetectAlm != CHK_OVERHEAT_NO_OPERATION )
		{
			if( DetectAlm == CHK_OVERHEAT_SET_ALARM )
			{
				ALMClearGlobalAlarm( WRN_CTRLBRD );			/* Warning(A.912)Cancellation *//* <S11D> */
				ALMSetGlobalAlarm( ALM_CTRLBRD );			/* Alarm(A.7A1) *//* <S11D> */
			}
			else if( DetectAlm == CHK_OVERHEAT_SET_WARNING )
			{
				ALMSetGlobalAlarm( WRN_CTRLBRD );			/* Warning(A.912) *//* <S11D> */
			}
			else if( DetectAlm == CHK_OVERHEAT_CLR_WARNING )
			{
				ALMClearGlobalAlarm( WRN_CTRLBRD );			/* Warning(A.912)Cancellation *//* <S11D> */
			}
			else
			{
				/* do nothing */
			}
		}
	}
	else if( BoardSel == POWER_BOARD )
	{
		if( DetectAlm != CHK_OVERHEAT_NO_OPERATION )
		{
			if( DetectAlm == CHK_OVERHEAT_SET_ALARM )
			{
				ALMClearGlobalAlarm(WRN_PWRBRD );				/* Warning(A.913)Cancellation *//* <S11D> */
				ALMSetGlobalAlarm( ALM_PWRBRD );			/* Alarm(A.7A2) *//* <S11D> */
			}
			else if( DetectAlm == CHK_OVERHEAT_SET_WARNING )
			{
				ALMSetGlobalAlarm( WRN_PWRBRD );			/* Warning(A.913) *//* <S11D> */
			}
			else if( DetectAlm == CHK_OVERHEAT_CLR_WARNING )
			{
				ALMClearGlobalAlarm( WRN_PWRBRD );				/* Warning(A.913)Cancellation *//* <S11D> */
			}
			else
			{
				/* do nothing */
			}
		}
	}
	else
	{
		/* do nothing */
	}
}



/****************************************************************************************************/
/*																									*/
/*		Servopack temperature detection section monitoring												<S0DE>				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Temperature detectionICmonitor communication errors.														*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckTempDeviceErr( ALMMAN_HANDLE *AlmMngr, USHORT CtrlBoadErrCnt, USHORT PowerBoadErrCnt )
{
	if( (CtrlBoadErrCnt >= 3) || (PowerBoadErrCnt >= 3) )
	{
		ALMSetServoAlarm( AlmMngr, WRN_PWRBRD );			/* Warning(A.913)			*/
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Amplifier & motor installation environment monitor calculation processing									<S03A>				*/
/*																									*/
/****************************************************************************************************/
void	AdetInstEnvMonitor( ENVMON *EnvMon, SHORT AmpTemp, SHORT MotTemp )
{
	EnvMon->var.AmpEnvMonSum += AmpTemp; /* [0.1℃] */
	EnvMon->var.MotEnvMonSum += MotTemp; /* [℃] */

	EnvMon->var.EnvMonSumCnt2s++;
	if( EnvMon->var.EnvMonSumCnt2s >= 1000 )
	{
	/*----------------------------------------------------------------------------------------------*/
	/*	Installation environment monitor value calculation(Monitor value update:2000msPeriod)												*/
	/*----------------------------------------------------------------------------------------------*/
		/* Un025 : Amplifier installation environment monitor [0.001%] */
		EnvMon->var.UnInstEnvMonAmp = EnvMon->var.AmpEnvMonSum;
		/* Un026 : Motor installation environment monitor [0.001%] */
		EnvMon->var.UnInstEnvMonMot = EnvMon->var.MotEnvMonSum;
		/* Initialize */
		EnvMon->var.AmpEnvMonSum = 0;
		EnvMon->var.MotEnvMonSum = 0;
		EnvMon->var.EnvMonSumCnt2s = 0;
	}
}


/***************************************** end of file **********************************************/

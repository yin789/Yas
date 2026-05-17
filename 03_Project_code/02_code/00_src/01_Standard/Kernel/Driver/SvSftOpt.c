/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SvSftOpt.c : Safety option card detection processing												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Machine Efficiency : Performs safety option card detection processing											*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2007.02.01  (Den-S-Kai)															*/
/*					Comment rearranging for Sigma5 BaseSoft											*/
/*		Rev.1.01 : 2007.02.03	R.Uda																*/
/*					Complete revision of option variables by reviewing option structure							*/
/*		Rev.2.00 : 2014.01.07	<S0C1>																*/
/*					SigmaV->Sigma7Transfer																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/****************************************************************************************************/
#include	"SvOptCardIf.h"
#include	"HwDeviceIf.h"



/****************************************************************************************************/
/*																									*/
/*		Initialize Servo-Safety Option I/F Variables												*/
/*																									*/
/****************************************************************************************************/
void	SftOptInitSvSafetyOptVar( OPSFT_HNDL *SftOpt )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Reset SftOpt Parameters/Variables															*/
/*--------------------------------------------------------------------------------------------------*/
		MlibResetLongMemory( SftOpt, sizeof(*SftOpt)/4 );

/*--------------------------------------------------------------------------------------------------*/
/*		Initialize ID/Version/Number																*/
/*--------------------------------------------------------------------------------------------------*/
		SftOpt->conf.OpInfo.IdType = SFTOP_ID_NONE;				/* Option ID			 		*/
		SftOpt->conf.OpInfo.YNumber = 0xFFFF;					/* Option Y specification number				*/
		SftOpt->conf.OpInfo.SoftVer = 0xFFFF;					/* Option software Ver			*/
		SftOpt->conf.OpInfo.SoftYspVer = 0xFFFF;				/* Option software Y specification Ver		*/
		SftOpt->conf.OpInfo.SoftTstVer = 0xFFFF;				/* Option software test Ver		*/
		SftOpt->conf.OpInfo.PrmVer = 0xFFFF;					/* Option parameter Ver		*/

		return;
}



/****************************************************************************************************/
/*																									*/
/*		Detect Safety Option Card																	*/
/*																									*/
/****************************************************************************************************/
void	SftOptDetectSafetyOption( OPSFT_HNDL *SftOpt, ALMMAN_HANDLE *AlmManager )
{
	USHORT	i,cnt = 0;
/*--------------------------------------------------------------------------------------------------*/
/*		IDSignal reading process (3Reading majority filter)													*/
/*--------------------------------------------------------------------------------------------------*/
		for(i=0; i<3; i++) 
		{
			/* Read option card ID code */
			if( HALDRV_GET_GPIO02 )
			{
				cnt++;
			}
			
			/* Next read wait time [250us] */
//			KpiResetWatchdogWaitus( 250 );
		}

		/* 2times/3times or more, it will be considered as an option card connection. */
		if( cnt >= 2 )
		{
			SftOpt->conf.OpInfo.f.Connect = TRUE;
		}
		else
		{
			SftOpt->conf.OpInfo.f.Connect = FALSE;
		}
#if	CSW_OPT_CARD_SAFETY == CSW_SAFETY_NO_SUPPORT
		if ( SftOpt->conf.OpInfo.f.Connect == TRUE )
		{
// 			Comment: Temporary measure		
//			ALMSetServoAlarm( AlmManager, ALM_SFTOP_NSUP );	/* A.E74 : Safety option card not supported */
		}
#endif
		return;
}
/***************************************** end of file **********************************************/

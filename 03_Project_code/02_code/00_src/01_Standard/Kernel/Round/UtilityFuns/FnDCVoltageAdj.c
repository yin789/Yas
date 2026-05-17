/****************************************************************************
  Description   : DCVolatage Gain Auto Adjustment
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	F.Ueda   2011.04.26   created

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "FunManager.h"
#include "JL086ServoIpHw.h"
#include "PnPrmListTbl.h"
#include "TuneLib.h"
#include "MLib.h"
#include "FnDCVoltageAdj.h"


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/* SGDV Note that the specifications are different from the automatic adjustment mode! */
static ADJUST_STATUS TciAdjustMainDcVoltage( REGIF_CONFIG_T *hPnReg, CMN_PRMDATA *cmnprm_ptr, 
											 ASIC_HW_IF_STR *pAsicHwReg, CHECK_MAIN_POWER *pMainPowerChecker, DET_DC_VOLT *pDetectDc );
static LONG TciDetectTestVdc( LONG RepNum, CHECK_MAIN_POWER *MainPowerChecker, 
							  DET_DC_VOLT *pDetectDc );
/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/


/****************************************************************************************************/
/*		STATIC VARIABLES																			*/
/****************************************************************************************************/


/****************************************************************************
 0x2012: Entry function of DCVolatage Gain Auto Adjustment
****************************************************************************/
PRM_RSLT FnEnterDcVoltageAutoAdj( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	FUN_CMN_CONTROL		*FnCmnCtrl;
	LONG				no;

	/* Ends without turning on when servo is on	*/
	for(no = 0; no < NumberOfAxes; no++)
	{
		FnCmnCtrl = ((AXIS_HANDLE*)GetAxisHandle(no))->FnCmnCtrl;
		if(FnCmnCtrl->State.FnSvCtrlDisable != FALSE)
		{
			return PRM_RSLT_CONDITION_ERR;
		}
	}
	
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
 0x2012: Execute function of DCVolatage Gain Auto Adjustment
****************************************************************************/
PRM_RSLT FnExecuteDcVoltageAutoAdj( FUNEXE *FunExe, AXIS_HANDLE *Axis )
{
	SEQ_CTRL_OUT				*SeqCtrlOut;
	REGIF_CONFIG_T				*hPnReg;
	CMN_PRMDATA					*cmnprm_ptr;
	ASIC_HW_IF_STR				*AsicHwReg;
	CHECK_MAIN_POWER 			*MainPowerChecker;
	DET_DC_VOLT					*DetectDc;
	ADJUST_STATUS				rslt;
	FUN_CMN_CONTROL				*FnCmnCtrl;
	LONG						no;

	/* Initialize */
	SeqCtrlOut				=	Axis->SeqCtrlOut;
	hPnReg					=	Axis->RegManager->hPnReg;
	cmnprm_ptr				=	Axis->UniPrms->Cprm;
	AsicHwReg				=	Axis->SvAsicRegs->AsicHwReg;
	MainPowerChecker		=	&(Axis->PowerManager->MainPowerChecker);
	DetectDc				=	&(Axis->PowerManager->DetectDcVolt);


	/* Ends without turning on when servo is on	*/
	for(no = 0; no < NumberOfAxes; no++)
	{
		FnCmnCtrl = ((AXIS_HANDLE*)GetAxisHandle(no))->FnCmnCtrl;
		if(FnCmnCtrl->State.FnSvCtrlDisable != FALSE)
		{
			return PRM_RSLT_CONDITION_ERR;
		}
	}
	
	/* Wait until the execution request comes.	*/
	if(FunExe->CmdCode != FCMD_EXEC)
	{
		return PRM_RSLT_CONTINUE;
	}

	/* Main circuit power supplyON/OFFIn order to make it possible to implement regardless ofMainPowerOnis not monitored.				*/
	if ( (SeqCtrlOut->MotStop == FALSE)									/* Motor Moving		*/
		|| (SHalDrv_GetFaultStatus(AsicHwReg, SHAL_FAULT_BB) == 0) )	/* Base Enable		*/
	{
		return PRM_RSLT_CONDITION_ERR;
	}
	
	rslt = TciAdjustMainDcVoltage( hPnReg,
								   cmnprm_ptr, 
								   AsicHwReg, 
								   MainPowerChecker,
								   DetectDc );
	
	if( rslt == ADJ_SUCCESS )
	{
		;		/* Do nothing */
	}
	else if( rslt == ADJ_FAILURE )
	{
		return PRM_RSLT_CALC_ERR;
	}
	
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************************************/
/*																									*/
/*		Main circuit power supply voltage detection level adjustment																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency :	DC voltage from test machine(PnE72), and if the DC voltage detection value at that time is the specified value(PnE72)		*/
/*			Set the adjustment parameters as follows:(Adjustment resolution:1/256)									*/
/*			If the voltage detection value is abnormal and the voltage adjustment level is within the adjustment reference range(±20%)If the		*/
/*			Set voltage adjustment level to zero as adjustment error.												*/
/*																									*/
/*			1. Adjustment procedure																				*/
/*				1) Normalization reference voltage calculation (Reference voltage: Regeneration on level(PnE72))								*/
/*				2) Acquisition of main circuit power supply voltage average value (Voltage detection4096times)								*/
/*				3) Main power supply voltage adjustment level calculation (Resolution:1/256)										*/
/*				4) Main power supply voltage adjustment level storage (==>PnE5B:Prm.pnadj/EEPROM)							*/
/*																									*/
/*			2. Related parameters																		*/
/*				1) PnE70 : P-NVoltage detection level  [V]		(Voltage at maximum detected value)						*/
/*		 		2) PnE71 : OV detection level     [V]													*/
/*				3) PnE72 : Regeneration on level     [V]		(DC voltage input from test machine)					*/
/*		 		4) PnE73 : UV detection level     [V]													*/
/*				5) PnE5B : Lower:P-NVoltage detection level fine adjustment	(Storing adjustment results)							*/
/*																									*/
/*			3. Adjustment level calculation formula																		*/
/*				1) Symbolic definition																			*/
/*					VdcBase: Reference DC voltage,  VdcMean: Average read voltage value during adjustment							*/
/*					VdcAdj : Adjustment level value,  VdcDet : DC voltage detection value(Value after correction with adjustment level)			*/
/*				2) DC voltage detection value correction formula																*/
/*				   VdcDet = VdcRead * (256 + VdcAdj) / 256											*/
/*						  = VdcRead + (VdcRead * VdcAdj / 256)										*/
/*				3) Adjustment level calculation formula																	*/
/*				   Read voltage value is incorrect VdcMean and the voltage detection value is VdcBase . The voltage that becomes		*/
/*				   VdcBase = VdcMean + (VdcMean * VdcAdj / 256) twist								*/
/*				   VdcAdj  = (VdcBase - VdcMean) * 256 / VdcMean									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static ADJUST_STATUS TciAdjustMainDcVoltage( REGIF_CONFIG_T *hPnReg, CMN_PRMDATA *cmnprm_ptr, 
								    ASIC_HW_IF_STR *pAsicHwReg, CHECK_MAIN_POWER *pMainPowerChecker, DET_DC_VOLT *pDetectDc )
{
	LONG			i;
	LONG			VdcAdj;
	LONG			VdcBase;
	LONG			VdcMean;
	LONG			VdcTest;
	LONG			ErrFlag = FALSE;
	ADJUST_STATUS	rslt;
	REG_ACC_T		AccessPrm;

	/* Initialize */
	rslt = ADJ_SUCCESS;

	/* RAMWrite register information setting */
	AccessPrm.MemAddr	= 0;
	AccessPrm.hRegCfg	= hPnReg;
	AccessPrm.ArrayIdx	= 0;
	AccessPrm.EepWrite	= TRUE;		/* EEPROM Write */
	AccessPrm.IfHandle	= NULL;

/*--------------------------------------------------------------------------------------------------*/
/*		Check start																				*/
/*			If voltage is not detected by software, return due to adjustment error							*/
/*--------------------------------------------------------------------------------------------------*/
/* IprmcalSystemSwitch( ):VdetMode == 0A.040There is a difference between not monitoring here						*/
#if 0
		if( pDetectDc->conf.VdetMode == VDET_NONE )
		{
			rslt = ADJ_NONE;								/* Adjustment error flag setting					*/
			return rslt;									/* Return without executing process					*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Adjustment reference voltage import																		*/
/*			Adjustment standard is regeneration ON voltage																	*/
/*--------------------------------------------------------------------------------------------------*/
		/* INGRAMNow, it can be adjusted with any input voltage.												*/
		VdcBase = pMainPowerChecker->conf.TestDcLevel;		/* Adjustment reference voltage[V]						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Main power supply voltage average value acquisition process																	*/
/*--------------------------------------------------------------------------------------------------*/
		VdcTest = TciDetectTestVdc( 100, pMainPowerChecker, pDetectDc );
		if( VdcTest < 0 ){ ErrFlag = TRUE;}					/* Abnormality check							*/
	/*----------------------------------------------------------------------------------------------*/
		for( VdcMean=0,i=0; i<4096; i++ )					/* Main power supply voltage acquisition processing(4096times)			*/
		{
			if( ErrFlag == TRUE ){ break;}					/* End at abnormality							*/
			VdcTest = TciDetectTestVdc( 1, pMainPowerChecker, pDetectDc );
			if( VdcTest > 0 )								/* Main power supply voltage acquisition normal					*/
			{
				VdcMean += VdcTest;							/* Main power supply voltage addition						*/
			}
			else											/* Main power supply voltage acquisition error					*/
			{
				ErrFlag = TRUE;								/* Average value acquisition error						*/
				break;										/* End at abnormality							*/
			}
		}
		VdcMean = (((VdcMean>>11)+1)>>1);					/* Average value creation							*/

/*--------------------------------------------------------------------------------------------------*/
/*		Main power supply voltage adjustment level calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( (ErrFlag == FALSE) && (VdcMean > 0) )			/* Average value acquisition normal						*/
		{
			VdcAdj = ((VdcBase - VdcMean)<<8) / VdcMean;	/* Adjustment level calculation						*/
			if( MlibABS( VdcAdj ) > VDCADJ_ERRLVL )			/* Adjustment level upper/lower limit check				*/
			{
				VdcAdj = 0;									/* Adjustment level = zero						*/
				rslt = ADJ_FAILURE;
			}
		}
		else												/* Average value acquisition failure						*/
		{
			VdcAdj = 0;										/* Adjustment level = zero						*/
			rslt = ADJ_FAILURE;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Adjustment level storage																			*/
/*--------------------------------------------------------------------------------------------------*/
		/* Create a parameter calculation functioncalland stored the adjustment results												*/
		VdcAdj = (VdcAdj & 0x00FF);							/* Make Byte Data						*/
		/* PnE5B Lower:P-NVoltage detection level fine adjustment, Upper level:P-NVoltage detection level zero adjustment							*/
		TunePrmWriteRam( &AccessPrm, &pndef_pnadj, VdcAdj, 2 );

		return rslt;
}


/****************************************************************************************************/
/*																									*/
/*		DC power supply voltage detection process for test machine																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : The PWM output of V850 is PHC isolated and passed through an RC filter to become an analog voltage.				*/
/*			Main circuit detection section comparator reference power supply.												*/
/*																									*/
/*	Supplementary Foot : Compare the reference power supply voltage and DC voltage with a comparator, and insulate the result with PHC.					*/
/*			Transmit to V850 as input signal of V850.											*/
/*																									*/
/*			1) If the comparator output is Lo, the reference power supply voltage is lower than the DC voltage.					*/
/*			   In this case, set the PWM output data to+1													*/
/*			2) If the comparator output is Hi, the reference power supply voltage is higher than the DC voltage.					*/
/*			   In this case, set the PWM output data to-1													*/
/*																									*/
/*			With this, the reference power supply voltage is always relative to the DC voltage value±Follow with an error of 1.					*/
/*			PWM data≒DC voltage.															*/
/*			If the data reaches the maximum value, it will be treated as a detection error.											*/
/*																									*/
/*			PWM data : 255/225V…AC100VFor specifications										*/
/*			PWM data : 255/450V…AC200VFor specifications										*/
/*			PWM data : 255/900V…AC400VFor specifications										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static LONG	TciDetectTestVdc( LONG RepNum, CHECK_MAIN_POWER *pMainPowerChecker, 
							  DET_DC_VOLT *pDetectDc )
{
	LONG		i;
	LONG		AdRead;
	LONG		VdcTest;
	
	/* Initialize */
	VdcTest = 0;
//	VdcTest	=	(LONG)(pMainPowerChecker->conf.TestDcLevel);

/*--------------------------------------------------------------------------------------------------*/
/*		DC power supply voltage detection processing																		*/
/*--------------------------------------------------------------------------------------------------*/
		switch( pDetectDc->conf.VdetMode )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		PWM-comparator method															*/
		/*------------------------------------------------------------------------------------------*/
			/*--------------------------------------------------------------------------------------*/
			/*  Fixed±2v/250us(Small capacity),Variable±16v/250(medium capacity)In this case, when the follow-up time is		*/
			/*	will be set to fixed tracking because it is unrelated.															*/
			/*--------------------------------------------------------------------------------------*/
			case VDET_PWM_FIX_FOLLOW:
			case VDET_PWM_VAR_FOLLOW:
#if 0														/* Deleted as not supported				*/
				/*----------------------------------------------------------------------------------*/
				/*	PWM Dummy Output																*/
				/*----------------------------------------------------------------------------------*/
				KPX_START_VDCPWM( );						/* For power supply voltage detectionPWMStart				*/
				KpiResetWatchdogWaitus( 125 );				/* Reset Watchdog and Wait				*/
				/*----------------------------------------------------------------------------------*/
				/*	For actual measurement PWM Output															*/
				/*----------------------------------------------------------------------------------*/
				for( i=0; i < RepNum; i++ )
				{
					KPX_OUT_VDCPWM( (USHORT)VdcTest );		/* PWMOutput setting							*/
					KpiResetWatchdogWaitus( 125 );			/* Reset Watchdog and Wait				*/
				/*----------------------------------------------------------------------------------*/
					if( KPX_CHK_VDCLOW( ) )					/* LowIn case ofPWMOutput isDCLower than voltage	*/
					{
						if( VdcTest < Bprm.VdetMax )		/* Upper limit confirmation								*/
						{
							++VdcTest;
						}
						else
						{
							return( -1 );					/* Max				*/
						}
					}
				/*----------------------------------------------------------------------------------*/
					else									/* HiIn case ofPWMOutput isDCHigher than voltage	*/
					{
						if( VdcTest > 0 )					/* Lower limit check								*/
						{
							--VdcTest;
						}
						else
						{
							return( -1 );					/* 0(=0V)It is abnormal for it to drop to				*/
						}
					}
				}
#endif														/* Deleted as not supported				*/
				VdcTest = -1;								/* If it comes here, it will be an abnormal response			*/
				break;
		/*------------------------------------------------------------------------------------------*/
		/*	A/D conversion method																			*/
		/*------------------------------------------------------------------------------------------*/
			case VDET_AD:
			/*--------------------------------------------------------------------------------------*/
			/*		DC voltage AD reading																*/
			/*--------------------------------------------------------------------------------------*/
				for( i=0; i < RepNum; i++ )
				{
					AdRead = HALdrv_GetDcVoltFromAd( );				/* DC voltage AD reading			*/
#if (FLOAT_USE==TRUE)
					VdcTest = (SHORT)(( AdRead * pDetectDc->conf.VdetAdNorm ));
#else
					VdcTest = (SHORT)(MlibMulgain( AdRead, pDetectDc->conf.VdetAdNorm ));
#endif /* FLOAT_USE */

					if( VdcTest >= pDetectDc->conf.VdetMaxVolt )
					{
						return( -1 );								/* Maxvalue.		*/
					}
				}
				break;
		/*------------------------------------------------------------------------------------------*/
		/*		No detection																			*/
		/*------------------------------------------------------------------------------------------*/
			default:
				VdcTest = -1;										/* No detection is abnormal				*/
				break;
		}

		return( VdcTest );
}


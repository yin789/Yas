/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PowerManager.c : Servo power management processing definition														*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Manage the power supply of the servo drive.														*/
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
/*	Note	:	First edition	2010.04.19	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "BaseCmnStruct.h"
#include "PowerManager.h"
#include "AlarmManager.h"
#include "MLib.h"
#include "KLib.h"
#include "HwDeviceIf.h"

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void	CpxPowerManager( POW_MNG_HNDL *hPowerManager );											*/
/*	void	LpxRegeneControl( REGENE_CTRL *pRegCtrl, POWER_STATUS *PowSts, BOOL MotStop );			*/
/*	LONG	LpxDetectDcVolt( DET_DC_VOLT *pDetectDc, POWER_STATUS *PowSts, CHAR AdAdjust );			*/
/*	LONG	LpxCheckOverVoltage( CHECK_MAIN_POWER *PowerChecker, DET_DC_VOLT *pDetectDc,			*/
/*																		POWER_STATUS *PowSts );		*/
/*	LONG	LpxCheckRegeneOverLoad( REGENE_CTRL *pRegCtrl, ULONG *AlmOLInfo,						*/
/*																	ANLG_UN_MON_DATA *MonData );	*/
/*	void	SetRegeneOverLoadSts( LONG RegOlSts );													*/
/*	BOOL	LpxCheckRushRxOverLoad( CHECK_RUSH *pRush, POWER_STATUS *pPowerSts );					*/
/*	BOOL	LpxCheckRegeneAlarm( REGENE_CTRL *pRegCtrl, BOOL PowerOn,								*/
/*											ASIC_HW_IF_STR *pAsicHwReg, UNI_PRM_STR *uni_prm_ptr );	*/
/*	void	PcmInitPowerManager( POW_MNG_HNDL *hPowerManager, ULONG AlmOLInfo );					*/
/*	void	PcmCountRegeneOnTime( POW_MNG_HNDL *hPowerManager );									*/
/*	void	PcmInputPowerStatus( POW_MNG_HNDL *hPowerManager, BOOL Acon1, BOOL RegeneAlm );			*/
/*	void	IprmcalRegenePowerGain( CHECK_REGOL *RegOL,												*/
/*													CMN_PRMDATA *cmnprm_ptr, BPRMDAT *bprm_ptr );	*/
/*	void	PcmCalculateVdetAdFil( DET_DC_VOLT *pDetectDc, USHORT vdetadfil, LONG SvCycleUs );		*/
/****************************************************************************************************/
static	void	LpxAcPowerManager( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts );
//static	void	LpxDcPowerManager( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts, BOOL MotStopSts );		/* <S21D> */
static	void	LpxDcPowerManager( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts, 							/* <S21D> */
												BOOL MotStop, REGENE_CTRL *RegeneCtrl );
//static	void	LpxDischargeDcPower( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts,
//																			REGENE_CTRL *RegCtrl );		/* <S0F6> */
static	void	LpxDischargeDcPower( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts,
																	REGENE_CTRL *RegCtrl, BOOL BeSts );	/* <S0F6> */
static	void	LpxCheckAcOpenPhase( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts );
#if( CSW_HW_DCBUS == FALSE )
static	void	LpxDetectAcOpenACON1( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts );
static	void	LpxDetectAcOpenACON12( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts );
#endif	/* ( CSW_HW_DCBUS == FALSE ) */
static	void	LpxCheckDcUnderVoltage( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts );
static	void	LpxAdDetectDcVolt( DET_DC_VOLT *pDetectDc );
static	void	LpxCheckPcovAlarm( PCOV_ERR_DETECT *PcovErrDetect, POWER_STATUS *PowSts );	/* <S053> */
static	BOOL	DischargeSequence( CHECK_MAIN_POWER *PowerChecker, LONG DcVolt ); /* <S1C6> */



/****************************************************************************************************/
/*																									*/
/*		Servo power management main																		*/
/*																									*/
/****************************************************************************************************/
//void	PcmPowerManager( POW_MNG_HNDL *hPowerManager, BOOL MotStopSts )								  /* <S0F6> */
void	PcmPowerManager( POW_MNG_HNDL *hPowerManager, BOOL MotStopSts, BOOL BeSts )					  /* <S0F6> */
{
	/* AC power supply monitoring process */
	LpxAcPowerManager( &(hPowerManager->MainPowerChecker),
					   &(hPowerManager->PowerSts) );

	/* DC power supply monitoring process */
	LpxDcPowerManager( &(hPowerManager->MainPowerChecker),
					   &(hPowerManager->PowerSts),
					   MotStopSts,
					   &(hPowerManager->RegeneCtrl) );	/* <S21D> */

	/* DC power supply rapid discharge processing */
	LpxDischargeDcPower( &(hPowerManager->MainPowerChecker),
						 &(hPowerManager->PowerSts),
						 &(hPowerManager->RegeneCtrl),
						 BeSts );																	  /* <S0F6> */

	/* AC power supply phase loss detection process */
	LpxCheckAcOpenPhase( &(hPowerManager->MainPowerChecker),
						 &(hPowerManager->PowerSts) );

	/* DC power supply undervoltage monitoring process */
	LpxCheckDcUnderVoltage( &(hPowerManager->MainPowerChecker),
							&(hPowerManager->PowerSts) );

	/* Intermediate potential abnormality check process <S053> */
	LpxCheckPcovAlarm( &(hPowerManager->PcovErrDetect),
					   &(hPowerManager->PowerSts) );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		AC power supply monitoring process																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	th Target :	(1) When the AC input is turned off as required by safety standards,										*/
/*				The DC power supply must be discharged to lower it to a certain voltage or less.							*/
/*			(2) Fix the servo-on signal to active and turn on/off the AC power						*/
/*				There is a user who controls motor energization on/off								*/
/*				If AC power is not detected, the motor will continue to be energized even if the power is turned off, which is dangerous.				*/
/*																									*/
/*																									*/
/*	Machine Efficiency : Detects AC input power on/off.													*/
/*																									*/
/*  		aconThe voltage status of the AC line is monitored by a photocoupler.							*/
/*			aconinis a soft filter (125usRead twice and majority vote).			*/
/*			Since the optocoupler is unipolar, only half the power cycleaconincannot be detected.				*/
/*			Therefore,aconinis in the power-on state and repeats on/off at the power cycle.						*/
/*			aconinis turned off, the conditions for the power to be turned off for at least half a cycle of the power supply(acon1==FALSE). If set as			*/
/*																									*/
/*			There is also a function called instantaneous power failure hold, even when the power is turned off for the time set by the user constant.			*/
/*			It is designed to maintain control.																*/
/*			As a result, power off detection isaconinis turned off, wait for half a power cycle + instantaneous power failure hold time.		*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	LpxAcPowerManager( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts )
{
#if( CSW_HW_DCBUS == TRUE )
/* Mask Function */
		return;
#endif
/*<S201> Start */
/*--------------------------------------------------------------------------------------------------*/
/* Creation of flag during dead zone of AC power ON detection      */ 
/* Initial Acon Wait Timer< KPI_SCANC_MS( 4 )*/
/*--------------------------------------------------------------------------------------------------*/
 if(PowerChecker->var.AconWaitTimer < KPI_SCANC_MS( 4 ))
 {
   	PowSts->AconDetectReady=FALSE;
 }
 else
 {
   	PowSts->AconDetectReady=TRUE;
 }
/*<S201> End*/
/*--------------------------------------------------------------------------------------------------*/
/*		Only for the first time at power-on4msWaiting ACONSignal countermeasures														*/
/*--------------------------------------------------------------------------------------------------*/
	if( PowerChecker->var.AconWaitTimer < KPI_SCANC_MS( 4 ) )
	{
		PowerChecker->var.AconWaitTimer++;
		PowSts->Acon = FALSE;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		AC power ON detection process																		*/
/*--------------------------------------------------------------------------------------------------*/
#if 0		/* <S21D> */
	else if( (PowSts->Acon1Sig != FALSE) ||
			 ((PowerChecker->conf.DetectAcOn2 != FALSE) && (PowSts->Acon2Sig != FALSE)) )
#else		/* <S21D> */
	else if( ( (PowerChecker->conf.Dcbus == FALSE)
			&& ( (PowSts->Acon1Sig != FALSE)
			  || ( (PowerChecker->conf.DetectAcOn2 != FALSE) && (PowSts->Acon2Sig != FALSE) ) ) )
		  || ( (PowerChecker->conf.Dcbus == TRUE)
			&& (PowSts->ConvRdySig == TRUE) ) )
#endif

	{
		PowSts->Acon = TRUE;									/* AC Power On						*/
		PowerChecker->var.AcoffChkTimer = 0;					/* Reset Power Off Check Timer		*/
	}
	/*----------------------------------------------------------------------------------------------*/
	else
	{
		/* <S21D> Start */
		/* Momentary power failure holding process */
#if 0
		/* 2011.04.04 Y.Oka <V762> */
		if( PowerChecker->conf.Dcbus )
		{
			PowSts->Acon = FALSE;								/* AC Power Off						*/
		}
//		if( PowerChecker->var.AcoffChkTimer < PowerChecker->conf.AcoffDetctTime )
		/* 2011.04.04 Y.Oka <V762> */
		else if( PowerChecker->var.AcoffChkTimer < PowerChecker->conf.AcoffDetctTime )
#else
		if( PowerChecker->var.AcoffChkTimer < PowerChecker->conf.AcoffDetctTime )
#endif
		/* <S21D> End */

		{
			PowerChecker->var.AcoffChkTimer += KPI_SCCYCLEMS;
			if( PowerChecker->var.AcoffChkTimer > 30 )
			{
				PowSts->PowerOnEep = FALSE;						/* Power Off for EEPROM				*/
			}
		}
		else
		{
			PowSts->Acon = FALSE;								/* AC Power Off						*/
			PowSts->PowerOnEep = FALSE;							/* Power Off for EEPROM				*/
		}
	}

	return;
}



/****************************************************************************************************/
/*																									*/
/*		DC power supply monitoring process																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitors the charging status of the DC power supply after input power supply is turned on, and sets an operation ready flag.					*/
/*			After charging is completed, the charging incomplete flag is turned off and an instruction is given to short-circuit the inrush current limiting resistor.			*/
/*			Due to the capacitance, the short-circuit operation of the inrush current limiting resistor is shared with the DB circuit off operation.			*/
/*			The short-circuit operation of the inrush current limiting resistor is performed by a dedicated processing module.				*/
/*			If charging fails, a charging failure alarm is generated. With the same alarm code as conventional undervoltage			*/
/*			However, they will be distinguished by reviewing the alarm code system.									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
//static	void	LpxDcPowerManager( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts, BOOL MotStop )		/* <S21D> */
static	void	LpxDcPowerManager( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts, 						/* <S21D> */
												BOOL MotStop, REGENE_CTRL *RegeneCtrl )

{
/*--------------------------------------------------------------------------------------------------*/
/*		Test mode																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( PowerChecker->conf.TestMode1 == TRUE )
	{
		PowSts->PowerOn = TRUE;										/* Power On for Control			*/
		PowSts->PowerOnEep = FALSE;									/* Power On for EEPROM			*/
		PowSts->ChargeOk = TRUE;									/* Charge OK for rush			*/
	}
#if( CSW_HW_DCBUS == FALSE )
	else if( PowerChecker->conf.Dcbus == FALSE )
	{
	/*----------------------------------------------------------------------------------------------*/
	/*		ACPower input mode																		*/
	/*----------------------------------------------------------------------------------------------*/
//		if( PowSts->Acon )				/* <S21D> */
		if( PowSts->Acon == TRUE )		/* <S21D> */
		{
			//if( PowerChecker->var.DconChkTimer >= POWREADY_WAIT )
			if( PowerChecker->var.DconChkTimer >= PowerChecker->conf.PowerReadyWaitTime ) /* <S053>  */
			{ /* Power ready waiting time elapsed */
				PowSts->PowerOn = TRUE;								/* Power On for Control			*/
				PowSts->PowerOnEep = TRUE;							/* Power On for EEPROM			*/
			}
			//else if( PowerChecker->var.DconChkTimer >= POWCHARG_WAIT )
			else if( PowerChecker->var.DconChkTimer >= PowerChecker->conf.PowerChargeWaitTime ) /* <S053> */
			{
				if( PowSts->DcVolt >= PowerChecker->conf.UvLevel )	/* DCVoltage check				*/
				{
					/*------------------------------------------------------------------------------*/
					/* DBOperation improvement: Main circuit during rotationOFF → ONCountermeasures										*/
					/*------------------------------------------------------------------------------*/
					switch( PowerChecker->conf.RlyDbWay )
					{
					case DBCTRL_TRIACK_SEPARATE:		/* Rush: Relay;DB:Thyristor 				*/
					case DBCTRL_RLY_SEPARATE:			/* Rush: Relay;DB: Relay 				*/
					case DBCTRL_TRIACK_COMMON:			/* Inrush+DB:Relay+Thyristor 				*/
						PowSts->ChargeOk = TRUE;		/* Regardless of whether the motor is rotating					*/
						break;

					case DBCTRL_RLY_COMMON:				/* Inrush + DB: Relay common 					*/
					default:							/* When setting is impossible Same as relay common	*/
						//if(PowerChecker->var.DconChkTimer == POWCHARG_WAIT)
						if(PowerChecker->var.DconChkTimer == PowerChecker->conf.PowerChargeWaitTime )
						{
							if( MotStop == TRUE )
							{ /* Motor stopped */
								/* Charging complete when stopping */
								PowSts->ChargeOk = TRUE;
							}
							else
							{ /* While the motor is rotating */
								/* Remains during rotation */
								//PowerChecker->var.DconChkTimer = POWCHARG_WAIT - 1;
								PowerChecker->var.DconChkTimer = PowerChecker->conf.PowerChargeWaitTime - 1;	/* <S053> */
							}
						}
						break;
					}
					PowerChecker->var.DconChkTimer++;
				}
				else
				{
					ALMSetGlobalAlarm( ALM_PUV );				/* A.410 : Undervoltage					*/
					PowSts->PowerOn = FALSE;
					PowSts->PowerOnEep = FALSE;
					PowSts->ChargeOk = FALSE;
					PowerChecker->var.DconChkTimer = 0;
				}
			}
			else
			{
				PowerChecker->var.DconChkTimer++;
			}
		}
	/*----------------------------------------------------------------------------------------------*/
	/*		AC power OFF																			*/
	/*----------------------------------------------------------------------------------------------*/
		else
		{
			PowSts->PowerOn  = FALSE;
			PowSts->PowerOnEep = FALSE;
			PowSts->ChargeOk = FALSE;
			PowerChecker->var.DconChkTimer = 0;
		}
	}
#endif
	else
	{
	/*----------------------------------------------------------------------------------------------*/
	/*		DCPower input mode																		*/
	/*----------------------------------------------------------------------------------------------*/
#if 0	/* <S21D> */
		if( PowSts->DcVolt >= PowerChecker->conf.UvLevel )			/* DCVoltage check				*/
		{
			//if( PowerChecker->var.DconChkTimer >= POWREADY_WAIT )
			if( PowerChecker->var.DconChkTimer >= PowerChecker->conf.PowerReadyWaitTime ) /* <S053>  */
			{ /* Power ready waiting time elapsed */
				PowSts->PowerOn = TRUE;								/* Power On for Control			*/
				PowSts->PowerOnEep = TRUE;							/* Power On for EEPROM			*/
			}
			else
			{
				//if( PowerChecker->var.DconChkTimer >= POWCHARG_WAIT )
				if( PowerChecker->var.DconChkTimer >= PowerChecker->conf.PowerChargeWaitTime ) /* <S053> */
				{
					/*------------------------------------------------------------------------------*/
					/* DBOperation improvement: Main circuit during rotationOFF → ONCountermeasures										*/
					/*------------------------------------------------------------------------------*/
					switch( PowerChecker->conf.RlyDbWay )
					{
					case DBCTRL_RLY_COMMON:				/* Inrush + DB: Relay common 					*/
					default:							/* When setting is impossible Same as relay common	*/
						//if(PowerChecker->var.DconChkTimer == POWCHARG_WAIT)
						if(PowerChecker->var.DconChkTimer == PowerChecker->conf.PowerChargeWaitTime ) /* <S053> */
						{
							if( MotStop == TRUE )
							{ /* Motor stopped */
								/* Charging complete when stopping */
								PowSts->ChargeOk = TRUE;
							}
							else
							{ /* While the motor is rotating */
								/* Remains during rotation */
								//PowerChecker->var.DconChkTimer = POWCHARG_WAIT - 1;
								PowerChecker->var.DconChkTimer = PowerChecker->conf.PowerChargeWaitTime - 1; /* <S053> */
							}
						}
						break;
					case DBCTRL_TRIACK_SEPARATE:		/* Rush: Relay;DB:Thyristor 				*/
					case DBCTRL_RLY_SEPARATE:			/* Rush: Relay;DB: Relay 				*/
					case DBCTRL_TRIACK_COMMON:			/* Inrush+DB:Relay+Thyristor 				*/
						PowSts->ChargeOk = TRUE;		/* Calculate the time to operate the inrush circuit				*/
						break;
					}
				}
				PowerChecker->var.DconChkTimer++;
			}
		}
		else
		{
			PowSts->PowerOn = FALSE;
			PowSts->PowerOnEep = FALSE;
			PowSts->ChargeOk = FALSE;
			PowerChecker->var.DconChkTimer = 0;
		}
	}
#else	/* <S21D> */
		if( PowSts->Acon == TRUE )
		{
			if( PowSts->DcVolt >= (LONG)PowerChecker->conf.UvLevel )		/* DCVoltage check				*/
			{
				//if( PowerChecker->var.DconChkTimer >= POWREADY_WAIT )
				if( PowerChecker->var.DconChkTimer >= PowerChecker->conf.PowerReadyWaitTime ) /* <S053>  */
				{ /* Power ready waiting time elapsed */
					PowSts->PowerOn = TRUE;								/* Power On for Control			*/
					PowSts->PowerOnEep = TRUE;							/* Power On for EEPROM			*/
				}
				else
				{
					//if( PowerChecker->var.DconChkTimer >= POWCHARG_WAIT )
					if( PowerChecker->var.DconChkTimer >= PowerChecker->conf.PowerChargeWaitTime ) /* <S053> */
					{
						/*------------------------------------------------------------------------------*/
						/* DBOperation improvement: Main circuit during rotationOFF → ONCountermeasures										*/
						/*------------------------------------------------------------------------------*/
						switch( PowerChecker->conf.RlyDbWay )
						{
						case DBCTRL_RLY_COMMON:				/* Inrush + DB: Relay common 					*/
						default:							/* When setting is impossible Same as relay common	*/
							//if(PowerChecker->var.DconChkTimer == POWCHARG_WAIT)
							if(PowerChecker->var.DconChkTimer == PowerChecker->conf.PowerChargeWaitTime ) /* <S053> */
							{
								if( MotStop == TRUE )
								{ /* Motor stopped */
									/* Charging complete when stopping */
									PowSts->ChargeOk = TRUE;
								}
								else
								{ /* While the motor is rotating */
									/* Remains during rotation */
									//PowerChecker->var.DconChkTimer = POWCHARG_WAIT - 1;
									PowerChecker->var.DconChkTimer = PowerChecker->conf.PowerChargeWaitTime - 1; /* <S053> */
								}
							}
							break;
						case DBCTRL_TRIACK_SEPARATE:		/* Rush: Relay;DB:Thyristor 				*/
						case DBCTRL_RLY_SEPARATE:			/* Rush: Relay;DB: Relay 				*/
						case DBCTRL_TRIACK_COMMON:			/* Inrush+DB:Relay+Thyristor 				*/
							PowSts->ChargeOk = TRUE;		/* Calculate the time to operate the inrush circuit				*/
							break;
						}
					}
					PowerChecker->var.DconChkTimer++;
				}
			}
			else
			{
				if( PowerChecker->var.DconChkTimer >= (ULONG)PowerChecker->conf.PowerChargeWaitTime )
				{ /* DC charging completion waiting time elapsed */
					ALMSetGlobalAlarm( ALM_PUV );					/* A.410 : Undervoltage				*/
					PowSts->PowerOn = FALSE;
					PowSts->PowerOnEep = FALSE;
					PowerChecker->var.DconChkTimer = 0U;
				}
				PowerChecker->var.DconChkTimer++;

			}
		}
		else
		{
			PowSts->PowerOn = FALSE;
			PowSts->PowerOnEep = FALSE;
			PowerChecker->var.DconChkTimer = 0U;
		}
		PowSts->ChargeOk = PowSts->ConvMconSig;
#endif
	}
	/* <S21D> Start */
	/* Capacitor bank control	*/
	PowerChecker->var.ClinkOut = PowSts->ChargeOk;
	if( PowerChecker->conf.CapaLinkFlag == TRUE )			/* Capacitor bank connection flag == TRUE		*/
	{
		if( PowerChecker->var.ClinkOut == TRUE )				/* Capacitor bank connection output signal == ON	*/
		{
			if( PowSts->ClinkAnsSig == FALSE )					/* /CLINKANSbut500ms	*/
			{
				if( PowerChecker->var.ClinkAnsErrChkTimer >= (ULONG)CLINKANSERRCHK_TIME )
				{
					ALMSetGlobalAlarm( ALM_CAPALINK );			/* A.332 : Capacitor bank connection answer error	*/
				}
				else
				{
					PowerChecker->var.ClinkAnsErrChkTimer++;
				}
			}
			else
			{
				PowerChecker->var.ClinkAnsErrChkTimer = 0U;
			}
		}
		else
		{
			PowerChecker->var.ClinkAnsErrChkTimer = 0U;
		}

		if( PowSts->ClinkAnsSig == TRUE )					/* Capacitor bank connection monitoring signal (/CLINKANS)== ON	*/
		{
			if( PowerChecker->var.ClinkAnsTimer >= (ULONG)CMCON_WAIT )
			{
				PowerChecker->var.CmconOut = TRUE;
				if( PowSts->CmcAnsSig == FALSE )			/* Rush protection relay status monitoring signal (/CMCANS)/CMCON	*/
				{
					if( PowerChecker->var.CmcAnsErrChkTimer >= (ULONG)CMCANSERRCHK_TIME )
					{
						ALMSetGlobalAlarm( ALM_CAPAMC );	/* A.333 : Capacitor bankMCAnswer error	*/
					}
					else
					{
						PowerChecker->var.CmcAnsErrChkTimer++;
					}
					PowerChecker->var.CapaChargeOk = FALSE;
				}
				else
				{
					PowerChecker->var.CmcAnsErrChkTimer = 0U;
					PowerChecker->var.CapaChargeOk = TRUE;
				}
			}
			else
			{
				PowerChecker->var.CmconOut = FALSE;
				PowerChecker->var.CmcAnsErrChkTimer = 0U;
				PowerChecker->var.ClinkAnsTimer++;
			}
		}
		else
		{
			PowerChecker->var.CmconOut = FALSE;
			PowerChecker->var.CmcAnsErrChkTimer = 0U;
			PowerChecker->var.ClinkAnsTimer = 0U;
		}
	}
	else
	{
		PowerChecker->var.CapaChargeOk = TRUE;
	}
	/* <S21D> End */
}



/****************************************************************************************************/
/*																									*/
/*		DC power supply rapid discharge processing																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : When main power supply is off,DCReuse the regenerative processing function so that high voltage does not remain in the line					*/
/*			Perform rapid discharge processing																	*/
/*																									*/
/*			1) 400Wand below do not have a built-in regenerative resistor as standard, so this function does not actually work.		*/
/*			2) When control power is turned on, normal processing starts10ms.							*/
/*			   this is,ACONis viewed as a half wave, so if the control and main power are turned on at the same time, the power supply half cycle			*/
/*			   ACONfrom being undetected and discharging even though the power is on.				*/
/*			3) DCThere are input specifications, which are set using user constants.							*/
/*			   ACIf the input specification is the same,DCThe amplifier was burnt out due to input or reverse setting.			*/
/*			   .																				*/
/*			4) As a countermeasure against burnout due to incorrect settings,ACONandDCFrom voltage condition, rapid discharge					*/
/*			   Provide an interlock.																*/
/*			   ACON==FALSESo, even after rapid discharge treatment for a certain period of time,DCIf the voltage does not drop, the					*/
/*			   DCIt is assumed that the input is input, and a wiring alarmA.33to stop the rapid discharge process.			*/
/*			   However,400WThe following do not have a built-in regenerative resistor and cannot be discharged quickly, soA.33.		*/
/*			5) DCEven though the input specifications are setACEven if it is input,A.33It is set as an alarm.							*/
/*			   DCIn the input specification, the regeneration operation is stopped, soACIf input, the diode bridge is set.				*/
/*			   received from [2], there is no place for the regenerative energy to escape, resulting in overvoltage. This is to prevent this.					*/
/*			   DCWhen using the input specification, from the + and - input terminalsDCI have to enter it.							*/
/*			   DCTo stop the regeneration operation in the input specification,DCIn the input specification, multi-axisDCWhen connecting via bus				*/
/*			   , and when regeneration processing is performed, the detection level is low1axis, and the regeneration processing time		*/
/*			   This is because there is a risk that the road will fail.													*/
/*			   DCIf you want the regeneration processing function in the input specification,YSpecification.								*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
//static	void	LpxDischargeDcPower( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts,
//																			REGENE_CTRL *RegCtrl )
static	void	LpxDischargeDcPower( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts,
																REGENE_CTRL *RegCtrl, BOOL BeSts )
{
#if( CSW_HW_DCBUS == TRUE )
/* Mask Function */
	return;
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		DCPower input mode || Test mode1 : Do not perform rapid discharge processing									*/
/*--------------------------------------------------------------------------------------------------*/
	if( (PowerChecker->conf.Dcbus == TRUE) || (PowerChecker->conf.TestMode1 == TRUE) )
	{
#if 0	/* <S21D> */
		PowSts->Discharge = FALSE;
		PowerChecker->var.DischChkTimer = KPI_SCANC_MS( 500 );
#else	/* <S21D> */
		if( (PowerChecker->conf.Dcbus == TRUE) && (PowerChecker->conf.DcDischarge == TRUE) )
		{
			if( PowSts->Discharge == TRUE )
			{
				if( PowerChecker->var.ConvInSigDischErrChkTimer >= (ULONG)CONVINSIGDISCHERRCHK_TIME )
				{
					if( (PowerChecker->var.DcVoltOld - PowSts->DcVolt) < DCVOLT10V )
					{
						ALMSetGlobalAlarm( ALM_CONVIO );			/* A.331 : Power related input signal error */
					}
				}
				else
				{
					PowerChecker->var.ConvInSigDischErrChkTimer++;
				}
			}
			else
			{
				PowerChecker->var.ConvInSigDischErrChkTimer = 0U;
				PowerChecker->var.DcVoltOld = PowSts->DcVolt;
			}

			if( PowSts->Acon == TRUE )
			{ /* Converter ready */
				PowSts->Discharge = FALSE;
				PowerChecker->var.DischChkTimer = (ULONG)KPI_SCANC_MS( 500 );
			}
			else if( PowerChecker->var.DischChkTimer >= (ULONG)KPI_SCANC_MS(500) )
			{
				PowSts->Discharge = TRUE;								/* Rapid dischargeON				*/
			}
			else
			{ /* do nothing */
				;
			}
		}
		else
		{
			PowSts->Discharge = FALSE;
			PowerChecker->var.DischChkTimer = (ULONG)KPI_SCANC_MS( 500 );
		}
#endif

/* 2011.04.04 Y.Oka <V762> */
#if 0
		if( (PowerChecker->conf.Dcbus == TRUE) && (PowSts->Acon == TRUE) )
		{
			ALMSetGlobalAlarm( ALM_WIR );					/* A.330 : Main circuit wiring error				*/
		}
#endif
		if( (PowerChecker->conf.Dcbus) && (PowSts->DcVolt > PowerChecker->conf.UvLevel) )
		{ /* DCInput selection && DCVoltage increasesUVExceeds level */
			if( (!PowSts->PowerOn) && (PowSts->Acon) )
			{ /* PowerOn=TRUE240mswith external regenerative resistorACONDetection */
				/* ACONIncrement counter */
				PowerChecker->var.AconDetectCount++;
				if( PowerChecker->var.AconDetectCount >= KPI_SCANC_MS(10) )
				{ /* ACONAfter detection10msElapsed time */
					/* A.330 : Main circuit wiring error */
					ALMSetGlobalAlarm( ALM_WIR );
					/* A.330Latch detection of */
					PowerChecker->var.Alm330Lt = TRUE;
					/* ACONCounter = 10ms */
					PowerChecker->var.AconDetectCount = KPI_SCANC_MS(10);
				}
			}
			else if( PowerChecker->var.Alm330Lt )
			{ /* In previous scanA.330Detection is latched. */
				/* A.330 : Main circuit wiring error */
				ALMSetGlobalAlarm( ALM_WIR );
			}
			else
			{ /* DCpower input(Normal) */
				PowerChecker->var.Alm330Lt = FALSE;
				PowerChecker->var.AconDetectCount = 0;
			}
		}
		else
		{ /* DCIf the voltage value isUVBelow level || DCWhen not input selection(ACInput selectionTestMode) */
			PowerChecker->var.Alm330Lt = FALSE;
			PowerChecker->var.AconDetectCount = 0;
		}
/* 2011.04.04 Y.Oka <V762> */

	}
/*--------------------------------------------------------------------------------------------------*/
/*		ACPower input mode : Perform rapid discharge processing														*/
/*--------------------------------------------------------------------------------------------------*/
//	else if( PowSts->Acon == TRUE )																	  /* <S0F6> */
	else if( (PowSts->Acon == TRUE) || (BeSts == TRUE) )											  /* <S0F6> */
	{ /* ACONMedium */
		PowSts->Discharge = FALSE;
		PowerChecker->var.DischChkTimer = KPI_SCANC_MS( 500 );
		PowerChecker->var.DischSeqNo = DISCH_INIT;
	}
	else if( PowerChecker->var.DischChkTimer >= KPI_SCANC_MS(500) )
	{ /* UsuallyACONis fallenBaseBlockthen rapid discharge sequence start */
//		PowSts->Discharge = TRUE;												/* Rapid dischargeON		*//* <S1C6> */
		PowSts->Discharge = DischargeSequence( PowerChecker, PowSts->DcVolt );	/* Rapid dischargeSequence	*//* <S1C6> */
	}
/*--------------------------------------------------------------------------------------------------*/
/*		AC power input mode : The following paths pass only the first time. 											*/
/*--------------------------------------------------------------------------------------------------*/
	else if( PowerChecker->var.DischChkTimer < KPI_SCANC_MS(10) )
	{ /* When control power is turned on(Acon=LO)Processing that only goes through.10ms */
		PowerChecker->var.DischChkTimer++;
		PowSts->Discharge = FALSE;								/* Rapid dischargeOFF						*/
	}
	else if( PowerChecker->var.DischChkTimer < KPI_SCANC_MS(310) )	/* 300ms			*/
	{
		PowerChecker->var.DischChkTimer++;
		PowerChecker->var.WireAlmDetWait = 0;
		PowSts->Discharge = TRUE;								/* Rapid dischargeON						*/
	}
	else														/* 310msAfterDCVoltage increases				*/
	{															/* If it is at a high level, it is considered abnormal.	*/
		PowSts->Discharge = FALSE;								/* Rapid dischargeOFF						*/
		if( (PowSts->DcVolt > DCVOLT60V)
			&& ((RegCtrl->conf.ResistorCapa != 0) || (RegCtrl->conf.RegRxIn)) )
		{ /* Power supply half cycleACONpower supply half cycle(10ms)+2msAfter the elapsed timeA.330			*/
			if( PowerChecker->var.WireAlmDetWait > KPI_SCANC_MS(10) )
			{
				ALMSetGlobalAlarm( ALM_WIR );					/* A.330 : Main circuit wiring error			*/
			}
			else
			{
				PowerChecker->var.WireAlmDetWait++;
			}
		}
		else
		{
			PowerChecker->var.WireAlmDetWait = 0;
		}
	}
#if 0 /* 2010.04.19 Y.Oka ASICbecause it is not used. */
/*--------------------------------------------------------------------------------------------------*/
/*		Rapid discharge output processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( CoutK.f.Discharge == TRUE )
	{
		KPI_DISCHARGEON( );									/* ASIC DisCharge ON					*/
	}
	else
	{
		KPI_DISCHARGEOFF( );								/* ASIC DisCharge OFF					*/
	}
#endif
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Rapid discharge sequence processing													<S1C6>				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Perform rapid discharge effective control																*/
/*																									*/
/*			DISCH_CHK_TIME[ms],3times and check whether the main circuit voltage drops.	*/
/*			If the main circuit voltage does not drop, rapid discharge is not performed.										*/
/*																									*/
/****************************************************************************************************/
static	BOOL	DischargeSequence( CHECK_MAIN_POWER *PowerChecker, LONG DcVolt )
{
	BOOL	DischReq;	/* Rapid discharge request */
	DischReq = FALSE;

	switch	( PowerChecker->var.DischSeqNo )
	{
	case DISCH_INIT: /* Rapid discharge initial */
		PowerChecker->var.DischStartVolt = DcVolt;					/* Rapid discharge confirmation start voltage memory	*/
		PowerChecker->var.DischSeqNo = DISCH_CHK1ST;				/* Rapid discharge confirmation1th time			*/
		PowerChecker->var.DischActTimer = 1;						/* Clear rapid discharge execution time		*/
		DischReq = TRUE;											/* Rapid discharge started					*/
	break;

	case DISCH_CHK1ST: /* Rapid discharge confirmation1th */
		if( PowerChecker->var.DischActTimer == KPI_SCANC_MS(DISCH_CHK_TIME) )
		{ /* Rapid discharge timeDISCH_CHK_TIME(20ms)Elapsed time */
			/* 1ScanStop rapid discharge for 80 minutes */
			DischReq = FALSE;										/* Rapid discharge temporary stop				*/
			PowerChecker->var.DischActTimer++;
		}
		else if( PowerChecker->var.DischActTimer > KPI_SCANC_MS(DISCH_CHK_TIME) )
		{
			if( (PowerChecker->var.DischStartVolt - DcVolt) < PowerChecker->conf.DischCheckVlt )	  /* <S1D1> */
			{ /* Voltage decrease widthPowerChecker->conf.DischCheckVlt(10V)In the following cases */
				/* There is an abnormality in rapid discharge */
				DischReq = FALSE;									/* Rapid discharge stop					*/
				PowerChecker->var.DischSeqNo = DISCH_FINISH;		/* Go to completion step				*/
			}
			else
			{ /* When the decrease in voltage due to rapid discharge exceeds the confirmation threshold */
				DischReq = TRUE;									/* Rapid discharge restart					*/
				PowerChecker->var.DischStartVolt = DcVolt;			/* Rapid discharge confirmation start voltage memory	*/
				PowerChecker->var.DischActTimer = 1;				/* Clear rapid discharge execution time		*/
				PowerChecker->var.DischSeqNo = DISCH_CHK2ND;		/* Rapid discharge confirmation2th time			*/
			}
		}
		else
		{
			DischReq = TRUE;										/* Confirmation rapid discharge				*/
			PowerChecker->var.DischActTimer++;
		}
	break;

	case DISCH_CHK2ND: /* Rapid discharge confirmation2th */
		if( PowerChecker->var.DischActTimer == KPI_SCANC_MS(DISCH_CHK_TIME) )
		{ /* Rapid discharge timeDISCH_CHK_TIME(20ms)Elapsed time */
			/* 1ScanStop rapid discharge for 80 minutes */
			DischReq = FALSE;										/* Rapid discharge temporary stop				*/
			PowerChecker->var.DischActTimer++;
		}
		else if( PowerChecker->var.DischActTimer > KPI_SCANC_MS(DISCH_CHK_TIME) )
		{
			if( ((PowerChecker->var.DischStartVolt - DcVolt) < PowerChecker->conf.DischCheckVlt)	  /* <S1D1> */
				&& (DcVolt > PowerChecker->conf.DischCompVlt) )
			{ /* Voltage decrease widthPowerChecker->conf.DischCheckVlt(10V) && Residual voltageDischCompVlt(50V)or more */
				/* There is an abnormality in rapid discharge */
				DischReq = FALSE;									/* Rapid discharge stop					*/
				PowerChecker->var.DischSeqNo = DISCH_FINISH;		/* Go to completion step				*/
			}
			else
			{ /* The decrease in voltage due to rapid discharge exceeds the confirmation threshold || Residual voltageDischCompVlt(50V)		*/
				DischReq = TRUE;									/* Rapid discharge restart					*/
				PowerChecker->var.DischStartVolt = DcVolt;			/* Rapid discharge confirmation start voltage memory	*/
				PowerChecker->var.DischActTimer = 1;				/* Clear rapid discharge execution time		*/
				PowerChecker->var.DischSeqNo = DISCH_CHK3RD;		/* Rapid discharge confirmation3th time			*/
			}
		}
		else
		{
			DischReq = TRUE;										/* Confirmation rapid discharge				*/
			PowerChecker->var.DischActTimer++;
		}
		break;

	case DISCH_CHK3RD: /* Rapid discharge confirmation3th */
		if( PowerChecker->var.DischActTimer == KPI_SCANC_MS(DISCH_CHK_TIME) )
		{ /* Rapid discharge timeDISCH_CHK_TIME(20ms)Elapsed time */
			/* 1ScanStop rapid discharge for 80 minutes */
			DischReq = FALSE;										/* Rapid discharge temporary stop				*/
			PowerChecker->var.DischActTimer++;
		}
		else if( PowerChecker->var.DischActTimer > KPI_SCANC_MS(DISCH_CHK_TIME) )
		{
			if( ((PowerChecker->var.DischStartVolt - DcVolt) < PowerChecker->conf.DischCheckVlt)	  /* <S1D1> */
				&& (DcVolt > PowerChecker->conf.DischCompVlt) )
			{ /* Voltage decrease widthPowerChecker->conf.DischCheckVlt(10V) && Residual voltageDischCompVlt(50V)or more */
				/* There is an abnormality in rapid discharge */
				DischReq = FALSE;									/* Rapid discharge stop					*/
				PowerChecker->var.DischSeqNo = DISCH_FINISH;		/* Go to completion step				*/
			}
			else
			{ /* The decrease in voltage due to rapid discharge exceeds the confirmation threshold || Residual voltageDischCompVlt(50V)		*/
				DischReq = TRUE;									/* Rapid discharge restart					*/
				PowerChecker->var.DischStartVolt = DcVolt;			/* Rapid discharge confirmation start voltage memory	*/
				PowerChecker->var.DischActTimer = 1;				/* Clear rapid discharge execution time		*/
				PowerChecker->var.DischSeqNo = DISCH_ACTIVE;		/* Proceed to rapid discharge operation				*/
			}
		}
		else
		{
			DischReq = TRUE;										/* Confirmation rapid discharge				*/
			PowerChecker->var.DischActTimer++;
		}
		break;

	case DISCH_ACTIVE: /* Rapid discharge */
		if( PowerChecker->var.DischActTimer >= PowerChecker->conf.MainDischActTime )
		{ /* Main rapid discharge time(Same as charging time)Elapsed time */
			DischReq = FALSE;										/* Rapid discharge stop					*/
			PowerChecker->var.DischActTimer = 0;
			PowerChecker->var.DischSeqNo = DISCH_FINISH;			/* Go to completion step				*/
		}
		else
		{
			DischReq = TRUE;										/* Confirmation rapid discharge				*/
			PowerChecker->var.DischActTimer++;
		}
		break;

	case DISCH_FINISH: /* Rapid discharge completion or Abnormal termination (Waiting for main circuit re-input) */
		DischReq = FALSE;											/* Rapid discharge stop					*/
																	/* Step initialization by caller after completion */
		break;

	default: /* This case does not come */
		DischReq = FALSE;											/* Rapid discharge stop					*/
		break;

	}

	return	DischReq;
}



/****************************************************************************************************/
/*																									*/
/*		AC power supply phase loss detection process																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Processing is branched according to the power supply phase loss detection method.												*/
/*																									*/
/*			PnE0C bit2 = 0 : ACON1only, open phase detection is performed.										*/
/*			PnE0C bit2 = 1 : ACON1/ACON2signal is used, open phase detection is performed.									*/
/*																									*/
/****************************************************************************************************/
static	void	LpxCheckAcOpenPhase( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts )
{

#if( CSW_HW_DCBUS == TRUE )
/* Mask Function */
	return;
#endif

	if( PowerChecker->conf.DetectAcOn2 != TRUE )
	{
		/* AC power supply phase loss detection process (ACON1Detection with signal only) */
		LpxDetectAcOpenACON1( PowerChecker, PowSts );
	}
	else
	{
		/* AC power supply phase loss detection process (ACON1/ACON2Detected with signal) */
		LpxDetectAcOpenACON12( PowerChecker, PowSts );
	}
	return;
}



#if( CSW_HW_DCBUS == FALSE )
/****************************************************************************************************/
/*																									*/
/*		AC power supply phase loss detection process	(ACON1Detection with signal only)												*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Perform open phase detection for amplifiers with 3-phase input specifications.											*/
/*			If phase loss is not detected, the DC voltage will drop when the motor is at high output, and the output will decrease due to insufficient voltage.					*/
/*			There is a risk that the process will not appear.																	*/
/*			Also, the ripple current of the capacitor increases, accelerating deterioration.									*/
/*																									*/
/*			Detection method isAcon1Signal(1=ACPower supplyON, 0=ACPower supplyOFF)but										*/
/*			SolidLois normal, if it is pulsed, it is open phase, solidHithen turn off the main power.							*/
/*																									*/
/*			The open phase detection time is1000msandACONState changeLO->HIof100times or more,							*/
/*			Power supply phase loss alarm(A.F10)only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.															*/
/*			If phase loss occurs,50HzWith power supply10msLO->HIState change								*/
/*			ScanC,1000msin100Regeneration state change occurs						*/
/*			Thus100times in the same direction, power supply phase loss alarm occurs(A.F10)only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.						*/
/*																									*/
/*																									*/
/*					       __    __    __    __    __    __    __   								*/
/*			Acon1  _______|  |__|  |__|  |__|  |__|  |__|  |__|  |__								*/
/*																									*/
/*						  |<--------------------------->|											*/
/*								1000ms, alarm												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	LpxDetectAcOpenACON1( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts )
{
/*--------------------------------------------------------------------------------------------------*/
/*		AC power supply phase loss check process																	*/
/*--------------------------------------------------------------------------------------------------*/
/* 2011.04.04 Y.Oka <V762> */
//	if( (PowerChecker->conf.TestMode1==FALSE) &&
//		(PowerChecker->conf.Ac3Phase==TRUE) && (PowSts->PowerOn==TRUE) )
	if( (PowerChecker->conf.TestMode1 == FALSE)
		&& (PowerChecker->conf.Ac3Phase == TRUE)
		&& (PowSts->PowerOn == TRUE)
		&& (PowerChecker->conf.Dcbus == FALSE) )
/* 2011.04.04 Y.Oka <V762> */
	{
	/*----------------------------------------------------------------------------------------------*/
	/*		Phase loss detection timer update process																*/
	/*----------------------------------------------------------------------------------------------*/
		if( PowerChecker->var.OpenChkTimer[0] != 0 )
		{
			PowerChecker->var.OpenChkTimer[0]++;
		}
	/*----------------------------------------------------------------------------------------------*/
	/*		Acon1Signal edge detection 																	*/
	/*----------------------------------------------------------------------------------------------*/
		if( (PowSts->Acon1Sig == TRUE) && (PowSts->Acon1Sig_l == FALSE) )
		{
			PowerChecker->var.OpenChkTimer[1] = PowerChecker->var.OpenChkTimer[0];
			if( PowerChecker->var.OpenChkTimer[0] == 0 )
			{
				PowerChecker->var.OpenChkTimer[0] = 1;			/* Timer start					*/
			}
		}
		else if( (PowerChecker->var.OpenChkTimer[0] - PowerChecker->var.OpenChkTimer[1]) > KPI_SCANC_MS( 12 ) )
		{
			PowerChecker->var.OpenChkTimer[0] = 0;				/* Edge not detected12msor above	*/
			PowerChecker->var.OpenChkTimer[1] = 0;				/*									*/
		}
	/*----------------------------------------------------------------------------------------------*/
	/*		Abnormality check : Phase loss detection timer1000ms, alarm										*/
	/*----------------------------------------------------------------------------------------------*/
		if( PowerChecker->var.OpenChkTimer[0] > ACOPEN_DETECT_WAIT )
		{
			ALMSetGlobalAlarm( ALM_PWPHASE );					/*  A.F10 : Power supply phase loss				*/

			PowerChecker->var.OpenChkTimer[0] = 0;
			PowerChecker->var.OpenChkTimer[1] = 0;
		}
	}
	else
	{
		PowerChecker->var.OpenChkTimer[0] = 0;
		PowerChecker->var.OpenChkTimer[1] = 0;
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		AC power supply phase loss detection process	(ACON1/ACON2Detected with signal)												*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Perform open phase detection for amplifiers with 3-phase input specifications.											*/
/*			If phase loss is not detected, the DC voltage will drop when the motor is at high output, and the output will decrease due to insufficient voltage.					*/
/*			There is a risk that the process will not appear.																	*/
/*			Also, the ripple current of the capacitor increases, accelerating deterioration.									*/
/*																									*/
/*			ACON1Signal																				*/
/*				HI→LO→HI··in the case of: Normal															*/
/*				SolidLO In the case of       : Power supplyOFF or RPhase loss											*/
/*				SolidHI In the case of       : Unpossible due to hardware										*/
/*			※Normal case(20ms,HI→LO→HI..),AcOn1=TRUEbecomes.						*/
/*																									*/
/*			ACON2Signal																				*/
/*				HI→LO→HI··in the case of: Normal															*/
/*				SolidLOIn the case of        : Power supplyOFF or SPhase loss or TPhase loss									*/
/*				SolidHIIn the case of        : Unpossible due to hardware										*/
/*			※Normal case(20ms,HI→LO→HI..),AcOn2=TRUEbecomes.						*/
/*																									*/
/*			AcOn1/AcOn2, it is classified into the following states.												*/
/*				AcOn1= TRUE		AcOn2= TRUE ⇒ Normal													*/
/*				AcOn1=FALSE		AcOn2= TRUE ⇒ RPhase loss												*/
/*				AcOn1= TRUE		AcOn2=FALSE ⇒ SPhase or TPhase loss										*/
/*				AcOn1=FALSE		AcOn2=FALSE ⇒ Power supplyOFF												*/
/*																									*/
/*			Therefore, open phase detection isAcOn1andAcOn2(XOR)Detect with.							*/
/*			The open phase detection time is1000ms, and setting power supply phase failure alarm(A.F10)only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.								*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	LpxDetectAcOpenACON12( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts )
{
/*--------------------------------------------------------------------------------------------------*/
/*		AC power supply phase loss check process																	*/
/*--------------------------------------------------------------------------------------------------*/
/* 2011.04.04 Y.Oka <V762> */
//	if( PowerChecker->conf.TestMode1 == FALSE )
	if( (PowerChecker->conf.TestMode1 == FALSE) && (PowerChecker->conf.Dcbus == FALSE) )
/* 2011.04.04 Y.Oka <V762> */
	{
	/*----------------------------------------------------------------------------------------------*/
	/*		ACON1Signal detection																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( PowSts->Acon1Sig != FALSE)
		{
			PowerChecker->var.DetOpenAcOn1 = TRUE;
			PowerChecker->var.AcOn1OffCount = 0;
		}
		else
		{
			if( PowerChecker->var.AcOn1OffCount > KPI_SCANC_MS( 20 ) )
			{
				PowerChecker->var.DetOpenAcOn1 = FALSE;
			}
			else
			{
				PowerChecker->var.AcOn1OffCount++;
			}
		}
	/*----------------------------------------------------------------------------------------------*/
	/*		ACON2Signal detection 																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( PowSts->Acon2Sig != FALSE)
		{
			PowerChecker->var.DetOpenAcOn2 = TRUE;
			PowerChecker->var.AcOn2OffCount = 0;
		}
		else
		{
			if( PowerChecker->var.AcOn2OffCount > KPI_SCANC_MS( 20 ) )
			{
				PowerChecker->var.DetOpenAcOn2 = FALSE;
			}
			else
			{
				PowerChecker->var.AcOn2OffCount++;
			}
		}
	/*----------------------------------------------------------------------------------------------*/
	/*	Power on state : Check for phase loss															*/
	/*----------------------------------------------------------------------------------------------*/
		if( PowSts->PowerOn == TRUE )
		{
			if(PowerChecker->var.DetOpenAcOn1 ^ PowerChecker->var.DetOpenAcOn2)
			{
				PowerChecker->var.OpenChkTimer[0]++;			/* Phase loss detection timer count up	*/
			}
			else
			{
				PowerChecker->var.OpenChkTimer[0] = 0;
		}
	/*----------------------------------------------------------------------------------------------*/
	/*	Abnormality check : Phase loss detection timer1000ms, alarm											*/
	/*----------------------------------------------------------------------------------------------*/
			if( PowerChecker->var.OpenChkTimer[0] > ACOPEN_DETECT_WAIT )
			{
				ALMSetGlobalAlarm( ALM_PWPHASE );				/*  A.F10 : Power supply phase loss				*/
				PowerChecker->var.OpenChkTimer[0] = 0;
			}
		}
	/*----------------------------------------------------------------------------------------------*/
	/*	Power off state : Phase loss detection timer clear														*/
	/*----------------------------------------------------------------------------------------------*/
		else
		{
			PowerChecker->var.OpenChkTimer[0] = 0;
		}
	}
	return;
}


#endif	/* ( CSW_HW_DCBUS == FALSE ) */

/****************************************************************************************************/
/*																									*/
/*		DC power supply undervoltage(UV)Monitoring process																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Undervoltage detection is performed to prevent motor malfunction.										*/
/*			When the DC power supply voltage decreases, the motor EMF will become saturated and the motor rotation speed will be limited.			*/
/*			Torque may decrease.																*/
/*																									*/
/*			This is thought to occur when the back power of the power supply is small, but normally such a small capacity power supply			*/
/*			. Inrush relay failure, diode bridge failure, blown fuse, capacitor capacity			*/
/*			More likely to occur due to component failure such as missing quantity												*/
/*																									*/
/*			The lower limit of the input power supply voltage is200 x 0.85 = 170VAC --> 240VDC , and							*/
/*			Even considering the ripple voltage, in the case of 3-phase 240 x sin60 ≒ 208V It will never go down.			*/
/*			Considering that a single-phase case is also suitable, take a margin and set the detection level to 190V.					*/
/*			Consider the same ratio for compatibility with other power supply voltages.													*/
/*			  100V system	  95V																*/
/*			  200V system	190V																*/
/*			  400V system	380V																*/
/*																									*/
/*			Test mode(ACONAlwaysUVNo detection mode).						*/
/*																									*/
/*																									*/
/*	Supplementary Foot : DC power supply overvoltage(OV)Monitor																	*/
/*			Overvoltage(OV)Regarding monitoring, when regenerating at full output, the voltage rises quickly, so					*/
/*			Scan B(Kernel : KpxCheckDcOverVoltage()).								*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	LpxCheckDcUnderVoltage( CHECK_MAIN_POWER *PowerChecker, POWER_STATUS *PowSts )
{
	if( (PowerChecker->conf.TestMode1 == FALSE) && (PowSts->PowerOn == TRUE) )
	{
		if ( PowSts->DcVolt < PowerChecker->conf.UvLevel )
		{
			if( PowerChecker->var.UvChkTimer >= PowerChecker->conf.UvFilter )
			{
				if( PowSts->DcVolt == 0 )
				{
					ALMSetGlobalAlarm( ALM_VDC );			/* A.030 : Main circuit detection part abnormality				*/
				}
				else
				{
					ALMSetGlobalAlarm( ALM_PUV );			/* A.410 : Undervoltage						*/
				}
			}
			else
			{
				PowerChecker->var.UvChkTimer += KPI_SCCYCLEMS;
			}
		}
		else
		{
			PowerChecker->var.UvChkTimer = 0;
		}
		if ( PowSts->DcVolt < PowerChecker->conf.UvWrnLevel )
		{
			if( PowSts->DcVolt != 0 )
			{
				ALMSetGlobalAlarm( WRN_UV );				/* A.971 : Undervoltage warning detection		*/
			}
		}
		else
		{
			ALMClearGlobalAlarm( WRN_UV );					/* A.971 : Undervoltage warning clear		*/
		}
	}
	else
	{
		PowerChecker->var.UvChkTimer = 0;
	}
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Regeneration processing ( Execution cycle : 250us )																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : When the motor decelerates, energy returns to the amplifier.(Regeneration operation),DCVoltage increases.	*/
/*			DCIf the voltage rises, the main circuit capacitor will be damaged, so it is necessary to take measures to stop the rise.			*/
/*			DCInsert a regenerative resistor in the line and consume the regenerative energy with the resistorDCsuppresses the voltage rise.			*/
/*			The current of the regenerative resistor is the regenerative transistor.ON/OFFis published.DCMonitor voltage and regenerative transistor		*/
/*			.																			*/
/*			In small stepsON/OFFis used, so adding hysteresis	*/
/*			Setting																				*/
/*			200VIn the system, the main circuit capacitor withstand voltage is400VTherefore,390VinON,380VinOFF. By using another power source	*/
/*			There is a margin of the same or more for voltage correspondence, so a detection level of the same proportion is fine.			*/
/*																									*/
/*				200V system : 390V…On, 380V…Off											*/
/*				100V system : 195V…On, 190V…Off											*/
/*				400V system : 780V…On, 760V…Off											*/
/*																									*/
/****************************************************************************************************/
void	PcmRegeneControl( REGENE_CTRL *pRegCtrl, POWER_STATUS *PowSts, BOOL MotStop )
{

#if	( CSW_HW_REGENERATION_CTRL == TRUE )

#if 0 /* 2010.09.01 Y.Oka Change to check in the caller */
/*--------------------------------------------------------------------------------------------------*/
/*		When a main circuit detection parameter error occurs, do not perform regeneration to avoid danger.								*/
/*--------------------------------------------------------------------------------------------------*/
	if( VdetPrmOk == FALSE )
	{
		return;											/* Without regenerationreturn							*/
	}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Regenerative overload alarm generation check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( ALMCheckGlobalAlarm( ALM_RGOL ) )				/* For unit common alarm0Axis only	*/
	{ /* Regeneration overload(A.320)When occurring */
		if( pRegCtrl->RegAlmMotStop || MotStop )
		{
			pRegCtrl->RegAlmMotStop = TRUE;
		}
		else
		{
			pRegCtrl->RegAlmMotStop = FALSE;
		}
	}
	else
	{
		pRegCtrl->RegAlmMotStop = FALSE;
	}

/* 2010.08.17 Y.Oka Main circuit detection part abnormality(A.030), change so that regeneration processing is not performed */
/*--------------------------------------------------------------------------------------------------*/
/*		Main circuit detection part abnormality alarm generation check														*/
/*--------------------------------------------------------------------------------------------------*/
	if( ALMCheckGlobalAlarm( ALM_VDC ) )				/* For unit common alarm0Axis only	*/
	{ /* Main circuit detection part abnormality(A.030)When occurring */
		HALdrv_RegeneControl( FALSE );					/* RegenerationTrOff command							*/
		pRegCtrl->RegStopCnt = 0;						/* Regeneration operation end wait counter clear			*/
		pRegCtrl->RegStartCnt = 0;						/* Regeneration operation start wait counter cleared			*/
	}
	else
	{ /* Main circuit detection part abnormality(A.030)When not occurring */
/*--------------------------------------------------------------------------------------------------*/
/*		Regenerative voltage level check																		*/
/*--------------------------------------------------------------------------------------------------*/
//		if( PowSts->DcVolt > pRegCtrl->conf.RegOnLevel )		/* <S21D> */
		if( PowSts->DcVolt > (LONG)pRegCtrl->conf.RegOnLevel )	/* <S21D> */
		{ /* DCvoltage regeneratesOnLevel */
			pRegCtrl->RegStopCnt = 0;					/* Regeneration operation end wait counter clear			*/
#if 0	/* <S21D> */
			if( pRegCtrl->RegStartCnt > pRegCtrl->conf.RegStartFilter )
			{ /* When waiting time to start regeneration operation has passed */
				pRegCtrl->RegOn = TRUE;					/* Perform regenerative processing							*/
			}
			else
			{ /* Waiting state to start regeneration operation */
				pRegCtrl->RegStartCnt++;				/* Regeneration operation start wait counter updated				*/
			}
#else	/* <S21D> */
			if( pRegCtrl->conf.RegSyn == PNREGSYN )
			{	/* Regeneration synchronization enabled (Pn00E.3=1) For */
				if( pRegCtrl->RegStartCnt > (pRegCtrl->conf.RegStartFilter + (LONG)pRegCtrl->conf.RegSynWaitTime) )
				{ /* When waiting time to start regeneration operation has passed */
					pRegCtrl->RegOn = TRUE;					/* Perform regenerative processing							*/
				}
				else
				{ /* Waiting state to start regeneration operation */
					pRegCtrl->RegStartCnt++;				/* Regeneration operation start wait counter updated				*/
				}
			}
			else
			{
				if( pRegCtrl->RegStartCnt > pRegCtrl->conf.RegStartFilter )
				{ /* When waiting time to start regeneration operation has passed */
					pRegCtrl->RegOn = TRUE;				/* Perform regenerative processing							*/
				}
				else
				{ /* Waiting state to start regeneration operation */
					pRegCtrl->RegStartCnt++;			/* Regeneration operation start wait counter updated				*/
				}
			}
#endif
		}
		else
		{
			pRegCtrl->RegStartCnt = 0;					/* Regeneration operation start wait counter cleared			*/
//			if( PowSts->DcVolt < pRegCtrl->conf.RegOffLevel )		/* <S21D> */
			if( PowSts->DcVolt < (LONG)pRegCtrl->conf.RegOffLevel )	/* <S21D> */
			{ /* DCvoltage regeneratesOffLevelor less */
				if( pRegCtrl->RegStopCnt > pRegCtrl->conf.RegStopFilter )
				{ /* Regeneration operation end wait time elapsed */
					pRegCtrl->RegOn = FALSE;			/* Stopping regeneration processing							*/
				}
				else
				{ /* Regeneration operation end wait state */
					pRegCtrl->RegStopCnt++;				/* Regeneration operation start wait counter updated				*/
				}
			}
			else
			{ /* 2010.06.28 Y.Oka 2times continuous regenerationOFFRegeneration when the level is belowOFFonly for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor. */
				pRegCtrl->RegStopCnt = 0;				/* Regeneration operation end wait counter clear to zero		*/
			}
		}

/*--------------------------------------------------------------------------------------------------*/
/*		RegenerationTr ON/OFFOutput																			*/
/*--------------------------------------------------------------------------------------------------*/
/*		DCinput and the regeneration is not in non-operating mode, it becomes the regenerationTrare as follows.			*/
/*			1. Rapid discharge requested(PowSts->Discharge == TRUE)											*/
/*			2. Regeneration requested && No regenerative abnormality && (No regeneration overload || Motor operating after regenerative overload)			*/
/*--------------------------------------------------------------------------------------------------*/
/*		A.320After detection, regeneration when motor stopsTr. Also, turn onA.300Regeneration after detectionTris turned off.			*/
/*		MDDuring development, data is exchanged between or Regeneration only when built-in regenerative resistorTrturn on(In case of setting errorOV)	*/
/*		, but it is set as	*/
/*		There are cases where an appropriate resistor is connected, and in that case, it would be bad if the rapid discharge does not occur.	*/
/*		Σ-Ⅴ.	*/
/*--------------------------------------------------------------------------------------------------*/
		if( (PowSts->Discharge == TRUE)
			|| ((pRegCtrl->RegOn == TRUE)
//				&& ((ALMCheckGlobalAlarm(ALM_RG) == FALSE) && (pRegCtrl->RegAlmMotStop == FALSE))) )	/* <S21D> */
			&& ((ALMCheckGlobalAlarm(ALM_RG) == FALSE) && (pRegCtrl->RegAlmMotStop == FALSE))			/* <S21D> */
			&& (ALMCheckGlobalAlarm(WRN_EXREG) == FALSE) ) )		/* A.924(Regeneration warning) None	*/			/* <S21D> */
		{ /* Regeneration || Rapid discharge requested */
#if 0	/* <S21D> */
//			if( (pRegCtrl->conf.ResistorCapa != 0) || (pRegCtrl->conf.RegRxIn) )					  /* <S1CA> */
//			{
				HALdrv_RegeneControl( TRUE );
				if( pRegCtrl->RegOn )
				{
					pRegCtrl->RegOnCount[0] += 2;		/* Regeneration due to regenerative overload detectionTrOncount the	*/
				}
//			}
//			else																					  /* <S1CA> */
//			{ /* Without external regenerative resistor && Without built-in regenerative resistor */
//				HALdrv_RegeneControl( FALSE );
//			}
#else	/* <S21D> */
//			if( (pRegCtrl->conf.ResistorCapa != 0U) || (pRegCtrl->conf.RegRxIn == TRUE) )
//			{ /* With external regenerative resistor || With built-in regenerative resistor */
				if( (pRegCtrl->conf.RegSyn == PNREGSYN)
				 && (ALMCheckGlobalAlarm(WRN_RGOLF) == TRUE) )
				{ /* Regeneration synchronization enabled && Regenerative overload warning (A.920) In case of occurrence */
					HALdrv_RegeneControl( FALSE );
				}
				else
				{
					HALdrv_RegeneControl( TRUE );
					if( pRegCtrl->RegOn == TRUE )
					{
						pRegCtrl->RegOnCount[0] += 2;		/* Regeneration due to regenerative overload detectionTrOncount the	*/
					}
				}
//			}
//			else
//			{ /* Without external regenerative resistor && Without built-in regenerative resistor */
//				HALdrv_RegeneControl( FALSE );
//			}
#endif
		}
		else
		{ /* Regeneration || Rapid discharge not requested */
			HALdrv_RegeneControl( FALSE );
		}
	}
/* 2011.04.04 Y.Oka <V764> */
/*--------------------------------------------------------------------------------------------------*/
	return;

#endif	//( CSW_HW_REGENERATION_CTRL == FALSE )
}



/****************************************************************************************************/
/*																									*/
/*		DCPower supply voltage detection processing ( Execution cycle : 250us ) for SGDS & SGDX										*/
/*																									*/
/*	Function:	Use different detection functions depending on the main circuit voltage detection method. Main circuit voltage(KioV.DcVoltx)	*/
/*				.PWM - Comparator method (fixed tracking method)											*/
/*				.PWM - Comparator method (variable tracking method)											*/
/*				.A/DConversion method																		*/
/*																									*/
/*	Return value:																							*/
/*			DETECT_DC_NO_OPERATION	: Normal end(Processing not required)											*/
/*			ALM_VDC					: All axes alarm output	A.030 : Main circuit detection part abnormality					*/
/*																									*/
/****************************************************************************************************/
LONG	PcmDetectDcVolt( DET_DC_VOLT *pDetectDc, POWER_STATUS *PowSts )
{
	LONG		ret;
	LONG		DcVolt;

	/* Local variable initialization */
	ret = DETECT_DC_NO_OPERATION;

/*--------------------------------------------------------------------------------------------------*/
/*		DC power supply voltage detection processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch(pDetectDc->conf.VdetMode)
	{
	/*------------------------------------------------------------------------------------------*/
	/*		PWM-comparator method															*/
	/*------------------------------------------------------------------------------------------*/
		/*--------------------------------------------------------------------------------------*/
		/*  Fixed±2v/250us(Small capacity),Variable±16v/250(medium capacity)In the case of									*/
		/*--------------------------------------------------------------------------------------*/
		case VDET_PWM_FIX_FOLLOW:
		case VDET_PWM_VAR_FOLLOW:
			/* 2010.06.08 Y.Oka Currently not supported */
//			LpxPwmDetectDcVolt();
			break;
	/*------------------------------------------------------------------------------------------*/
	/*	A/D conversion method																			*/
	/*------------------------------------------------------------------------------------------*/
		case VDET_AD:
			LpxAdDetectDcVolt( pDetectDc );
			break;
	/*------------------------------------------------------------------------------------------*/
	/*		No detection	(SVFSEL_SWVDET != 0is abnormal DCVoltage=0and used as)				*/
	/*------------------------------------------------------------------------------------------*/
		default:
			pDetectDc->DcVoltx = 0;
			break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		DC power supply voltage detection value correction processing																	*/
/*--------------------------------------------------------------------------------------------------*/
	DcVolt = ((pDetectDc->DcVoltx * (0x100 + pDetectDc->conf.VdetAdadj)) >> 8);
	if( DcVolt >= 0 )
	{
		PowSts->DcVolt = DcVolt;
	}
	else
	{
		PowSts->DcVolt = 0;
	}


/*--------------------------------------------------------------------------------------------------*/
/*		DC power supply voltage detection value abnormality check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( PowSts->Acon == TRUE )
	{ /* Check AC Power ON */
		if( pDetectDc->DcvWaitCnt >= KPI_SCANC_MS(20) )
		{ /* Check DcVolt Wait Count */
			if( (pDetectDc->DcVoltx == 0) || (pDetectDc->DcVoltx == pDetectDc->conf.VdetMaxVolt ) )
			{
				if( pDetectDc->DcvErrCnt > KPI_SCANC_MS(2) )
				{ /* 8times * 250μ = 2ms Continuous error */
//					KPI_BASE_BLOCK( );						/* ASIC base block				*/
//					KPI_MPCMD_SVOFF( );						/* Microprogram servo OFF		*/
					ALMSetGlobalAlarm( ALM_VDC );			/* A.030 : Main circuit detection part abnormality				*/
					pDetectDc->DcvErrCnt = 0;				/* Reset Error Counter					*/
					pDetectDc->DcvWaitCnt = 0;				/* Reset DcVolt Wait Counter			*/
					ret = ALM_VDC;							/* A.030 : Main circuit detection part abnormality				*/
				}
				else
				{
					pDetectDc->DcvErrCnt++;					/* Increment Error Count				*/
				}
			}
			else
			{
				pDetectDc->DcvErrCnt = 0;				/* Reset DcVolt Wait Counter			*/
			}
		}
		else
		{
			pDetectDc->DcvWaitCnt++;						/* Increment DcVolt Wait Count			*/
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	else													/* AC Power OFF							*/
	{
		pDetectDc->DcvErrCnt = 0;			/* Reset Error Counter					*/
		pDetectDc->DcvWaitCnt = 0;			/* Reset DcVolt Wait Counter			*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return ret;
}



/****************************************************************************************************/
/*																									*/
/*		OVDetection processing ( Execution cycle : 250us )																*/
/*																									*/
/*	Function:	Overvoltage detection is performed to protect the main circuit components.												*/
/*			The main circuit capacitor has the lowest breakdown voltage.												*/
/*			Main circuit capacitors may explode if the withstand voltage is exceeded.									*/
/*			Normal regeneration processing works and reaches the overvoltage levelDCThe voltage never increases.						*/
/*			Overvoltage may occur due to a malfunction in the regenerative processing circuit or mismatch in regenerative resistance value.		*/
/*			When regenerating at full output, the voltage rises quickly, soScanB.								*/
/*			DCMonitor the voltage level and process an error if it exceeds the overvoltage detection level.							*/
/*																									*/
/*	Return value:																							*/
/*			DETECT_DC_NO_OPERATION	: Normal end(Processing not required)											*/
/*			ALM_VDC					: All axes alarm output	A.030 : Main circuit detection part abnormality					*/
/*			ALM_OV					: All axes alarm output	A.400 : Overvoltage(OV)						*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	@CHK@ : The logic of alarm detection is incorrect!!!!!														*/
/*			.Overvoltage(OV)and undervoltage(UV)Check0xFFand0x00is checked				*/
/*			.DC power supply voltage detection value abnormality check(0x00,0xFF)What is the purpose of the abnormality check?				*/
/*																									*/
/*																									*/
/****************************************************************************************************/
//LONG	PcmCheckOverVoltage( CHECK_MAIN_POWER *PowerChecker, DET_DC_VOLT *pDetectDc, POWER_STATUS *PowSts ) /* <S0F7> */
LONG	PcmCheckOverVoltage( CHECK_MAIN_POWER *PowerChecker, DET_DC_VOLT *pDetectDc,
														POWER_STATUS *PowSts, REGENE_CTRL *RegCtrl ) /* <S0F7> */
{
	LONG		ret;
	USHORT		OvLevel; /* <S0F7> */

	/* Local variable initialization */
	ret = DETECT_DC_NO_OPERATION;

/*--------------------------------------------------------------------------------------------------*/
/*		Regeneration usage status check														<S0F7>			*/
/*--------------------------------------------------------------------------------------------------*/
	if( (RegCtrl->conf.ResistorCapa != 0) || (RegCtrl->conf.RegRxIn) )
	{ /* Regeneration used */
		OvLevel = PowerChecker->conf.OvLevel;
	}
	else
	{ /* Regeneration not used */
		OvLevel = PowerChecker->conf.NonRegOvLevel;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		DC power supply overvoltage check																		*/
/*--------------------------------------------------------------------------------------------------*/
//	if( PowSts->DcVolt > (LONG)PowerChecker->conf.OvLevel )	/* DCVoltage increasesOVIf exceeds level			*//* <S0F7> */
	if( PowSts->DcVolt > (LONG)OvLevel )					/* DCVoltage increasesOVIf exceeds level			*/
	{
		if( pDetectDc->DcVoltx == pDetectDc->conf.VdetMaxVolt )
		{
			if( pDetectDc->VdcdetAlmCnt > pDetectDc->conf.VdcdetAlmFilter )
			{
//				KPI_BASE_BLOCK( );							/* ASIC base block				*/
//				KPI_MPCMD_SVOFF( );							/* Microprogram servo OFF		*/
				ALMSetGlobalAlarm( ALM_VDC );				/* A.030 : Main circuit detection part abnormality				*/
				ret = ALM_VDC;								/* A.030 : Main circuit detection part abnormality				*/
			}
			else
			{
				pDetectDc->VdcdetAlmCnt++;					/* Increment Error Counter				*/
			}
		}
		/* 2010.06.08 Y.Oka Mask check changed to be performed outside */
//		else if( !KPI_ALARMMSKCHK( ALM_OV ) )				/* Alarm mask check				*/
		else
		{
			if( pDetectDc->OvdetAlmCnt > PowerChecker->conf.OvdetAlmFilter )
			{
//				KPI_BASE_BLOCK( );							/* ASIC base block				*/
//				KPI_MPCMD_SVOFF( );							/* Microprogram servo OFF		*/
				ALMSetGlobalAlarm( ALM_OV );				/* A.400 : Overvoltage(OV)					*/
				ret = ALM_OV;								/* A.400 : Main circuit detection part abnormality				*/
			}
			else
			{
				pDetectDc->OvdetAlmCnt++;					/* Increment Error Counter				*/
			}
		}
	}
	else
	{
		pDetectDc->VdcdetAlmCnt = 0;						/* Reset Error Conter					*/
		pDetectDc->OvdetAlmCnt = 0;							/* Reset Error Conter					*/
	}

	return ret;
}



/****************************************************************************************************/
/*																									*/
/*		DCPower supply voltage detection processing ( Execution cycle : 250us ) 													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function: A/DDetect the converted main circuit detection voltage													*/
/*																									*/
/*	Revision:	2007.02.07	K.Ando		<V134>															*/
/*			Add triple root filter to AD read value														*/
/*																									*/
/****************************************************************************************************/
static	void	LpxAdDetectDcVolt( DET_DC_VOLT *pDetectDc )
{
	LONG	AdRead;
	LONG	DcVoltx;

/*--------------------------------------------------------------------------------------------------*/
/*		DC voltage AD reading																			*/
/*--------------------------------------------------------------------------------------------------*/
	AdRead = HALdrv_GetDcVoltFromAd( );						/* DC voltage AD reading					*/
	DcVoltx = MlibMulgain( AdRead, pDetectDc->conf.VdetAdNorm );

/*--------------------------------------------------------------------------------------------------*/
/*		DC voltage clamp processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( DcVoltx < 0 )
	{
		DcVoltx = 0;
	}
	else if( DcVoltx > pDetectDc->conf.VdetMaxVolt )
	{
		DcVoltx = pDetectDc->conf.VdetMaxVolt;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		DC voltage filter processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	pDetectDc->DcAdFili = DcVoltx;
	pDetectDc->DcAdFilo[0] =
		MlibLpfilter1( DcVoltx, pDetectDc->conf.VdetAdFil, pDetectDc->DcAdFilo[0] );
	pDetectDc->DcAdFilo[1] =
		MlibLpfilter1( pDetectDc->DcAdFilo[0], pDetectDc->conf.VdetAdFil, pDetectDc->DcAdFilo[1] );
	pDetectDc->DcAdFilo[2] =
		MlibLpfilter1( pDetectDc->DcAdFilo[1], pDetectDc->conf.VdetAdFil, pDetectDc->DcAdFilo[2] );
	pDetectDc->DcVoltx = pDetectDc->DcAdFilo[2];

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Regenerative overload check process (Call from ScanC)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Estimate the power consumption of the regenerative resistor from the on-time of the regenerative transistor,							*/
/*			If the overload level is exceeded, a regenerative overload alarm will be issued.									*/
/*			Also, calculate the average regenerative power value for 10 seconds												*/
/*																									*/
/*	Return value:																							*/
/*			CHK_REGOL_NO_OPERATION	: No regeneration overload(Processing not required)										*/
/*			CHK_REGOL_SET_ALARM		: All axes alarm output		A.320 : Regeneration overload						*/
/*			CHK_REGOL_SET_WARNING	: All axes warning output	A.920 : Regenerative overload warning			*/
/*			CHK_REGOL_CLR_WARNING	: All axes warning clear	A.920 : Regenerative overload warning			*/
/*																									*/
/****************************************************************************************************/
LONG	PcmCheckRegeneOverLoad( REGENE_CTRL *pRegCtrl,ULONG *AlmOLInfo, ANLG_UN_MON_DATA *MonData )
{
	LONG	RegPower;
	LONG	LastRgolPowSum;
	LONG	ret;

	/* Local variable initialization */
	ret = CHK_REGOL_NO_OPERATION;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of regenerative power																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( (pRegCtrl->conf.ResistorCapa == 0) && (pRegCtrl->conf.RegRxIn == FALSE) )
	{
		RegPower = 0;
	}
	else
	{
#if (FLOAT_USE==TRUE)
		RegPower = pRegCtrl->dRegOnCount * pRegCtrl->RegOL.conf.Krgpower;
#else
		RegPower = MlibMulgain30( pRegCtrl->dRegOnCount, pRegCtrl->RegOL.conf.Krgpower );
#endif /* FLOAT_USE */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Regeneration overload check																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* 2010.09.01 Y.Oka Change to check in the caller */
//	if( (TestMode2 == FALSE) && (prm_ptr->iprm_ptr->Dcbus == FALSE) )
	{
		/*	Previous regenerative power integrated value saving */
		LastRgolPowSum = pRegCtrl->RegOL.var.RgolPowSum;
		pRegCtrl->RegOL.var.RgolPowSum += (RegPower - pRegCtrl->RegOL.conf.RgolPowBase);
		if( pRegCtrl->RegOL.var.RgolPowSum < 0 )
		{
			pRegCtrl->RegOL.var.RgolPowSum = 0;
		}
	/*----------------------------------------------------------------------------------------------*/
		/* 2010.11.16 Y.Oka Pn600 = 0Setting countermeasures */
//		if( pRegCtrl->RegOL.var.RgolPowSum >= pRegCtrl->RegOL.conf.RgolAlmLevel )
		if( pRegCtrl->RegOL.var.RgolPowSum > pRegCtrl->RegOL.conf.RgolAlmLevel )
		{
//			KPI_ALARMSET( ALM_RGOL );					/* A.320 : Regeneration overload 						*/
			ret |= CHK_REGOL_SET_ALARM;
			pRegCtrl->RegOL.var.LastRgolDetect = FALSE;	/* Regenerative overload detection flag turned off at previous startup		*/
		}
		/* 2010.11.16 Y.Oka Pn600 = 0Setting countermeasures */
//		if( pRegCtrl->RegOL.var.RgolPowSum >= (pRegCtrl->RegOL.conf.RgolAlmLevel>>1) )
		if( pRegCtrl->RegOL.var.RgolPowSum > (pRegCtrl->RegOL.conf.RgolAlmLevel>>1) )
		{
//			KPI_WARNINGSET( WRN_RGOLF );				/* A.920 : Regenerative overload warning				*/
//			ret = CHK_REGOL_SET_WARNING;
			if( !pRegCtrl->RegOL.var.LastRgolDetect || pRegCtrl->RegOL.var.RgPowUp )
			{ /* No previous detection || Power increased */
				ret |= CHK_REGOL_SET_WARNING;			/* A.920 : Regenerative overload warning				*/
			}
			else if( LastRgolPowSum < pRegCtrl->RegOL.var.RgolPowSum )
			{ /* Increased from previous regenerated power */
				ret |= CHK_REGOL_SET_WARNING;			/* A.920 : Regenerative overload warning				*/
				pRegCtrl->RegOL.var.RgPowUp = TRUE;		/* Regenerative power increase flag on					*/
														/* RgPowUpteethTRUERegardless of whether or not they were detected last time,*/
														/* Because of the warning,FALSEis unnecessary			*/
			}
			else
			{ /* No warning output if there is previous detection and no power increase */
				ret |= CHK_REGOL_CLR_WARNING;			/* A.920 : Regenerative overload warning				*/
			}
		}
		else
		{
			if( pRegCtrl->RegOL.var.LastRgolDetect )
			{ /* Regenerative overload detection flag turned on at last startup */
				pRegCtrl->RegOL.var.LastRgolDetect = FALSE;	/* Regenerative overload detection flag turned off at previous startup	*/
				*AlmOLInfo &= ~ALMDEF_OLINFO_RGOL;			/* Regenerative overload information reset				*/
			}
//			KPI_WARNINGCLR( WRN_RGOLF );				/* A.920 : Regenerative overload warning				*/
			ret |= CHK_REGOL_CLR_WARNING;				/* A.920 : Regenerative overload warning				*/
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Un00A : Regenerative power data	(10secRegenerative power average value)	CoutV.UnMeanRgPower						*/
/*		Un143 : Regenerative power data	(2msRegenerative power value)			CoutV.UnRgPower							*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	MonData->UnRgPower = RegPower * pRegCtrl->RegOL.conf.KunrgpowerCycle;
#else
	MonData->UnRgPower = MlibMulgain( RegPower, pRegCtrl->RegOL.conf.KunrgpowerCycle );
#endif /* FLOAT_USE */
	MonData->TrcRgPower = MonData->UnRgPower;
	pRegCtrl->RegOL.var.RgPowerSum += RegPower;

	pRegCtrl->RegOL.var.RgMeanCycle++;
	if( pRegCtrl->RegOL.var.RgMeanCycle >= KPI_SCANC_MS( 10000 ) )
	{
#if 0
#if (FLOAT_USE==TRUE)
		MonData->UnMeanRgPower = pRegCtrl->RegOL.var.RgPowerSum * pRegCtrl->RegOL.conf.Kunrgpower;
#else
		MonData->UnMeanRgPower = MlibMulgain( pRegCtrl->RegOL.var.RgPowerSum, pRegCtrl->RegOL.conf.Kunrgpower );
#endif /* FLOAT_USE */
#endif
/*<S123>--->*/
#if (FLOAT_USE==TRUE)
		pRegCtrl->UnMeanRgPower = pRegCtrl->RegOL.var.RgPowerSum * pRegCtrl->RegOL.conf.Kunrgpower;
#else
		pRegCtrl->UnMeanRgPower = MlibMulgain( pRegCtrl->RegOL.var.RgPowerSum, pRegCtrl->RegOL.conf.Kunrgpower );
#endif
/*<S123><---*/
		pRegCtrl->RegOL.var.RgPowerSum  = 0;
		pRegCtrl->RegOL.var.RgMeanCycle = 0;
	}

	return ret;
}


#if 0
/****************************************************************************************************/
/*																									*/
/*		Regenerative overload abnormal alarm/warning setting process (Call from ScanC)								*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : LpxCheckRegeneOverLoad(),						*/
/*			Regenerative overload alarm set, regenerative overload warning set, regenerative overload warning clear		*/
/*			processing.																		*/
/*																									*/
/****************************************************************************************************/
void	SetRegeneOverLoadSts( LONG RegOlSts )
{
	if( RegOlSts & CHK_REGOL_SET_ALARM )
	{
		/* A.320 : Regeneration overload */
		ALMSetGlobalAlarm( ALM_RGOL );
	}
	else if( RegOlSts & CHK_REGOL_SET_WARNING )
	{
		/* A.920 : Regenerative overload warning */
		ALMSetGlobalAlarm( WRN_RGOLF );
	}
	else
	{
		/* A.920 : Regenerative overload warning */
		ALMClearGlobalAlarm( WRN_RGOLF );
	}
	return;
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Inrush resistance overload check processing (Call from ScanC)												*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Checks the frequency of main circuit power on/off											*/
/*			If it is turned on and off 10 times in 2 minutes, an inrush resistance overload alarm will be set.						*/
/*																									*/
/*			With a simple check every 2 minutes, there is a possibility of missing 18 times/2 minutes, so					*/
/*			Two-stage check with half cycle shift.														*/
/*			This is still not perfect, but it is difficult to handle it seriously.									*/
/*																									*/
/*						   2min 																	*/
/*			Cnt[0] :  |<----------->|<----------->|<----------->|<----------->|<------ 				*/
/*			Cnt[1] :  ------>|<----------->|<----------->|<----------->|<----------->|				*/
/*								  2min  															*/
/*																									*/
/*	Return value:																							*/
/*			CHK_RUSH_NO_OPERATION	: No inrush resistance overload(Processing not required)									*/
/*			CHK_RUSH_SET_ALARM		: All axes alarm output	A.740 : Inrush resistance overload						*/
/*																									*/
/****************************************************************************************************/
BOOL	PcmCheckRushRxOverLoad( CHECK_RUSH *pRush, POWER_STATUS *pPowerSts )
{
	BOOL	ret;

	/* Local variable initialization */
	ret = CHK_RUSH_NO_OPERATION;

/*--------------------------------------------------------------------------------------------------*/
/*		Power OFF==>ON detection																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( pPowerSts->PowerOn == TRUE )
	{
		if( pRush->PowerOnChk == FALSE )
		{
			pRush->PowerOnCnt[0]++;						/* Count up Power Off ==> On				*/
			pRush->PowerOnCnt[1]++;						/* Count up Power Off ==> On				*/
		}
		pRush->PowerOnChk = TRUE;
	}
	else
	{
		pRush->PowerOnChk = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Inrush resistance overload check																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( (pRush->PowerOnCnt[0] >= 10) || (pRush->PowerOnCnt[1] >= 10) )
	{
		/* A.740 : Inrush resistance overload */
//		KPI_ALARMSET( ALM_RSOL );
		ret = CHK_RUSH_SET_ALARM;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Detection cycle control & counter clear processing														*/
/*--------------------------------------------------------------------------------------------------*/
	pRush->PowerOnChkCycle++;
	if( pRush->PowerOnChkCycle >= KPI_SCANC_MS( 2*60*1000 ) )
	{
		pRush->PowerOnCnt[0] = 0;
		pRush->PowerOnChkCycle = 0;
	}
	else if( pRush->PowerOnChkCycle == KPI_SCANC_MS( 1*60*1000 ) )
	{
		pRush->PowerOnCnt[1] = 0;
	}

	return ret;
}



/****************************************************************************************************/
/*																									*/
/*		Regeneration abnormality check processing (Call from ScanC)														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Regeneration abnormality detection detects a short-circuit failure of the regeneration Tr.									*/
/*			Regeneration Tr collector when power ready is active(IGBT drain)voltage again in the order			*/
/*			Monitor with a comparator, and if the Lo state continues for a certain period of time, it is considered abnormal.						*/
/*			During normal regeneration operation, it repeats on/off in short cycles, but remains on for a long time.			*/
/*			There is no such thing.																				*/
/*																									*/
/*			1) The time to be considered abnormal is 400ms. When the regeneration Tr has a short-circuit failure,				*/
/*			   This is a period of time that does not burn out the regenerative resistor.												*/
/*			2) There is a type that does not have a built-in regenerative resistor. This is distinguished by system constants and detected.				*/
/*			   Pass.																			*/
/*			3) Σ-Ⅱ, I didn't see it in DC power input mode, but it has a built-in regeneration circuit.			*/
/*			   If the regenerative transistor has a short-circuit failure, there is a risk of burnout, so it should be detected.					*/
/*			4) Conventionally, it was also used to detect the disconnection of the regenerative resistor, but if there is no regenerative resistor due to the capacity,			*/
/*			   Some regenerative resistors are externally attached, and it becomes complicated as it requires a judgment to detect disconnection or not.				*/
/*			   Therefore, regenerative resistor disconnection shall be detected by regenerative overload detection and overvoltage detection.					*/
/*			5) Connect the regeneration Tr collector to the P line with high resistance, and keep it at Hi level even when the regeneration resistor is disconnected				*/
/*			   .																	*/
/*																									*/
/*	Return value :																						*/
/*			CHK_REGENE_NO_OPERATION	: No regenerative abnormality(Processing not required)										*/
/*			CHK_REGENE_SET_ALARM	: All axes alarm output	A.300 : Regenerative abnormality							*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	2002.01.17 : DC input mode and 6KW, do not perform alarm detection process(SGDH)Correspondence					*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
BOOL	PcmCheckRegeneAlarm( REGENE_CTRL *pRegCtrl, BOOL PowerOn )
{
	BOOL	ret;

	/* Local variable initialization */
	ret = CHK_REGENE_NO_OPERATION;

/*--------------------------------------------------------------------------------------------------*/
/*		Regeneration abnormality check processing																		*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
	if( (TestMode1 == FALSE)
		&&	((pRegCtrl->conf.RegRxIn == TRUE) || (pRegCtrl->conf.ResistorCapa != 0))
		/* 2010.09.01 Y.Oka The power input mode is confirmed by the caller. Changed so that it does not depend on the capacity. */
//		&&	((iprm_ptr->Dcbus == FALSE ) || (uni_prm_ptr->pprm_ptr->ampw < 6000))
		&&	(PowerOn == TRUE) )
//In test mode at call destinationPowerOn=FALSEand handling
#endif
	if( ((pRegCtrl->conf.RegRxIn == TRUE) || (pRegCtrl->conf.ResistorCapa != 0))
		&&	(PowerOn == TRUE) )
	{
		/*	(Regeneration abnormality signal ON && RegenerationTrCommand off) || (Regeneration abnormality signal OFF && RegenerationTrCommand on) */
//		if( ((pRegCtrl->RegAlmSig == TRUE ) && ((pAsicHwReg->AREG_IOSTAT1 & 0x0002) != 0)) ||
//			((pRegCtrl->RegAlmSig == FALSE) && ((pAsicHwReg->AREG_IOSTAT1 & 0x0002) == 0)) )
		if( ((pRegCtrl->RegAlmSig == TRUE ) && (FALSE == HALdrv_GetRegeneControlState())) ||
			((pRegCtrl->RegAlmSig == FALSE) && (FALSE != HALdrv_GetRegeneControlState())) )
		{
			if( pRegCtrl->RegChk.RegeneErrCnt <= KPI_SCANC_MS( 400 ) )
			{
				pRegCtrl->RegChk.RegeneErrCnt++;
			}
			else
			{
				/* A.300 : Regenerative abnormality */
//				KPI_ALARMSET( ALM_RG );
				ret = CHK_REGENE_SET_ALARM;
			}
			pRegCtrl->RegChk.RegeAlmSignal = TRUE;
		}
		else
		{
			pRegCtrl->RegChk.RegeneErrCnt = 0;
			pRegCtrl->RegChk.RegeAlmSignal = FALSE;
		}
	}
	else
	{
		pRegCtrl->RegChk.RegeneErrCnt = 0;
		pRegCtrl->RegChk.RegeAlmSignal = FALSE;
	}

	return ret;
}



/****************************************************************************************************/
/*																									*/
/*		Servopack built-in fan stoppage check processing		<S0A3>											*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitors the stoppage of the fan built in the servopack.									*/
/*			Although not related to direct power control, since it is a unit common process,PCMMake it part of the module.		*/
/*																									*/
/*	Revision:																							*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#define FANLOCK_START_TIME	KPI_SCANC_MS( 10000 )	/* Time to start fan stop confirmation(10s)		*/
/*--------------------------------------------------------------------------------------------------*/
LONG	PcmCheckFanLock( CHECK_FANLOCK *FanLock, BOOL FanStop, BOOL BeSts )
{
	LONG	ret;
	ULONG	CumulativeOperationTime;	/* Cumulative operating time[100ms](After fan stop warning occurs) */	/* <S219> */

	ret = CHK_FANLOCK_NO_OPERATION;

	if( FanLock->var.FanLockStartCnt >= FANLOCK_START_TIME )
	{
		if( FanStop == TRUE )
		{
			if( FanLock->var.FanLockTimeCnt >= FanLock->conf.FanLockDetectTime )
			{
				if( BeSts == TRUE )
				{ /* SVONIf there is an axis in the middle */
					ret = CHK_FANLOCK_SET_WARNING;			/* A.923 : Fan stop warning			*/

					/* <S219> Start */
					/* Fan stop warning detected 1In the case of the scan */
					if( FanLock->var.FanLockWrnFlag == FALSE )
					{
						/* Getting current cumulative operating time(When fan stop warning occurs) */
						FanLock->var.WrnStartTime = KlibGetTimeStamp();

						/* Fan stop warning occurrence flag = TRUE */
						FanLock->var.FanLockWrnFlag = TRUE;
					}

					/* Getting current cumulative operating time(100[ms]unit) */
					CumulativeOperationTime = KlibGetTimeStamp();

					/* Getting current cumulative operating time - Obtain cumulative operating time when fan stop warning occurs > PnEE6:Built-in fan stop alarm detection time when servo is on */
					if( ( CumulativeOperationTime - FanLock->var.WrnStartTime) > FanLock->conf.FanLockAlmDetectTime )
					{
						ret = CHK_FANLOCK_SET_ALARM;			/* A.7AB : Fan stop alarm			*/

						/* Fan stop warning occurrence flag = FALSE */
						FanLock->var.FanLockWrnFlag = FALSE;
					}
					/* <S219> End */
				}
				else
				{ /* All axesSVOFFIn the case of */
					ret = CHK_FANLOCK_SET_ALARM;			/* A.7AB : Fan stop alarm			*/

					/* Fan stop warning occurrence flag = FALSE */	/* <S219> */
					FanLock->var.FanLockWrnFlag = FALSE;			/* <S219> */
				}
			}
			else
			{
				/* Fan stop time count */
				FanLock->var.FanLockTimeCnt++;

				/* Fan stop warning occurrence flag = FALSE */	/* <S219> */
				FanLock->var.FanLockWrnFlag = FALSE;			/* <S219> */
			}
		}
		else
		{
			/* Clear fan stop time count */
			FanLock->var.FanLockTimeCnt = 0;

			/* Fan stop warning occurrence flag = FALSE */	/* <S219> */
			FanLock->var.FanLockWrnFlag = FALSE;			/* <S219> */
		}
	}
	else
	{
		/* After power-on,FANLOCK_START_TIMEDo not check the fan stop until the elapsed time */
		FanLock->var.FanLockStartCnt++;

		/* Fan stop warning occurrence flag = FALSE */	/* <S219> */
		FanLock->var.FanLockWrnFlag = FALSE;			/* <S219> */

	}

	return	ret;
}



/****************************************************************************************************/
/*																									*/
/*		Initialize Unit Alarm Check Variables														*/
/*																									*/
/****************************************************************************************************/
void	PcmInitPowerManager( POW_MNG_HNDL *hPowerManager, ULONG AlmOLInfo )
{
	CHECK_MAIN_POWER	*pMainPowerChecker;
	CHECK_REGOL			*pRegOL;

	/* Local variable initialization */
	pMainPowerChecker = &(hPowerManager->MainPowerChecker);
	pRegOL = &(hPowerManager->RegeneCtrl.RegOL);

/*--------------------------------------------------------------------------------------------------*/
/*		Initialization of variables for main circuit power supply monitoring																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Reset Variable Data */
	MlibResetLongMemory( &(pMainPowerChecker->var), sizeof(pMainPowerChecker->var)/4 );

/*--------------------------------------------------------------------------------------------------*/
/*		Variable initialization for regenerative overload detection																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Reset Variable Data */
	MlibResetLongMemory( &(pRegOL->var), sizeof(pRegOL->var)/4 );

	if( AlmOLInfo & ALMDEF_OLINFO_RGOL )
	{ /* When regenerative overload alarm information is available */
		/* Regenerative overload detection flag turned on at last startup */
		pRegOL->var.LastRgolDetect = TRUE;

		switch( pRegOL->conf.RgSumSel )
		{
		case REGPOWSUM_RATE95PER:					/*	Regenerative overload alarm level95%		*/
			pRegOL->var.RgolPowSum = ( pRegOL->conf.RgolAlmLevel / 100 ) * 95;
			break;
		case REGPOWSUM_RATE50PER:					/*	Regenerative overload alarm level50%		*/
			pRegOL->var.RgolPowSum = ( pRegOL->conf.RgolAlmLevel >> 1 );
			break;
		default:									/*	Regenerative overload alarm level50%		*/
			pRegOL->var.RgolPowSum = ( pRegOL->conf.RgolAlmLevel >> 1 );
			break;
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Inrush prevention resistor overload detection variable initialization															*/
/*--------------------------------------------------------------------------------------------------*/
	/* Reset Variable Data */
	MlibResetLongMemory( &(hPowerManager->Rush), sizeof(hPowerManager->Rush)/4 );

/*--------------------------------------------------------------------------------------------------*/
/*		Regeneration error detection variable initialization																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Reset Variable Data */
	MlibResetLongMemory( &(hPowerManager->RegeneCtrl.RegChk), sizeof(hPowerManager->RegeneCtrl.RegChk)/4 );

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Count Regene Transistor ON Time																*/
/*																									*/
/****************************************************************************************************/
void	PcmCountRegeneOnTime( POW_MNG_HNDL *hPowerManager )
{
	LONG	lwk0;

/*--------------------------------------------------------------------------------------------------*/
/*		Delta Regene.Tr On Count for ScanC Alarm Check												*/
/*--------------------------------------------------------------------------------------------------*/
	/* Current Regene ON Count */
	lwk0 = hPowerManager->RegeneCtrl.RegOnCount[0];
	/* Delta Regene ON Count */
	hPowerManager->RegeneCtrl.dRegOnCount = lwk0 - hPowerManager->RegeneCtrl.RegOnCount[1];
	/* Update Last Regene ON Count */
	hPowerManager->RegeneCtrl.RegOnCount[1] = lwk0;

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Count Regene Transistor ON Time																*/
/*																									*/
/****************************************************************************************************/
void	PcmInputPowerStatus( POW_MNG_HNDL *hPowerManager, BOOL Acon1, BOOL RegeneAlm )
{
	hPowerManager->PowerSts.Acon1Sig_l = hPowerManager->PowerSts.Acon1Sig;
	hPowerManager->PowerSts.Acon1Sig = Acon1;
//	hPowerManager->PowerSts.Acon2Sig = Acon2;
	hPowerManager->RegeneCtrl.RegAlmSig = RegeneAlm;

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Calculate regenerative power calculation gain																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate regenerative power calculation gain.														*/
/*			Regenerative power is[5000/Converter capacity]In units ofScanC.					*/
/*			ScanBReceive data of regenerative transistor'sON/OFFSince it counts, the regenerated power is						*/
/*			The following formula is obtained.																			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*	SGD7*,INGRAM																					*/
/*						 (DcVolt)^2  * SBCYCLEUS * 5000  											*/
/*			Regenerative power = ----------------------------------- * dRegOnCount							*/
/*						(reger/1000) * SCCYCLEUS * ConvW  											*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*	SGDV																							*/
/*						 (DcVolt)^2  * SBCYCLEUS * 5000  											*/
/*			Regenerative power = ----------------------------------- * dRegOnCount							*/
/*						(reger/1000) * SCCYCLEUS * ampw  											*/
/*--------------------------------------------------------------------------------------------------*/
/*			Pn600 : Prm.regres			Regenerative resistor capacity				[10W]							*/
/*			Pn603 : Prm.exreger			Regenerative resistance value					[10mΩ]							*/
/*			PnE1A : Prm.reger			Regenerative resistance value(Default)		[mΩ]							*/
/*			PnE1C : Prm.wre				Regenerative resistor capacity(Default)	[0.1%]							*/
/*			PnE72 : Prm.regonlvl		Regeneration on level				[V]								*/
/*			PnE7F : Prm.ConvW			Converter capacity				[W]								*/
/****************************************************************************************************/
void	IprmcalRegenePowerGain( REGENE_CTRL *RegeneCtrl, BPRMDAT *Bprm, REGCFG_PRM *RegPrm )
{
	LONG		kx, sx;
	LONG		Vdcsqr;
	LONG		reger;		/* Regenerative resistance value */

/*--------------------------------------------------------------------------------------------------*/
/*		Voltage calculation																					*/
/*--------------------------------------------------------------------------------------------------*/
	if( Bprm->AmpDcinType == 1 )							/* Amplifier DC power input				*/
	{
		Vdcsqr = 0;												/* Without regeneration							*/
	}
	else														/* Amplifier DC power input				*/
	{
		/* Vdcsqr = (PnE72 ^ 2) */
		Vdcsqr = RegPrm->regonlvl * RegPrm->regonlvl;
		if( Bprm->AmpInVcode == 0x02 )
		{														/* 400VOther than the input: Regenerative resistance value[mΩ]	*/
			Vdcsqr /= 10;										/* 400VInput	   : Regenerative resistance value[10mΩ]	*/
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Regenerative resistance value selection															<S0F4>			*/
/*--------------------------------------------------------------------------------------------------*/
	if( RegPrm->regres != 0 )
	{ /* Pn600External regenerative resistor capacity already set */
		if( RegPrm->exreger != 0 )
		{ /* Pn603External regenerative resistor value already set */
			/* Uses external regenerative resistor value */
			if( Bprm->AmpInVcode == 0x02 )
			{ /* 400VFor input */
				reger = RegPrm->exreger;						/* No unit conversion required[10mΩ]				*/
			}
			else
			{ /* 400VIf the input is other than the input */
				reger = RegPrm->exreger * 10U;					/* Unit conversion[10mΩ]→ [mΩ]			*/
			}
		}
		else
		{
			/* Use default regeneration resistance value */
			reger = RegPrm->reger;
		}
	}
	else
	{
		/* Use default regeneration resistance value */
		reger = RegPrm->reger;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate regenerative power calculation gain																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( Bprm->SvCycleNs != KPI_SBCYCLE_BASE_NS )
	{ /* ScanBPeriod 125us */
		kx = MlibScalKxgain( Vdcsqr, Bprm->SvCycleUs/2, (KPI_SCCYCLEUS * reger), &sx, 0 );
	}
	else
	{ /* ScanBPeriod 125us */
		kx = MlibScalKxgain( Vdcsqr, Bprm->SvCycleUs, (KPI_SCCYCLEUS * reger), &sx, 0 );
	}

	kx = MlibScalKxgain( Vdcsqr, Bprm->SvCycleUs, (KPI_SCCYCLEUS * reger), &sx, 0 );
	kx = MlibPcalKxgain( kx, (1000 * 5000), RegPrm->ConvW, &sx, 24 );
	RegeneCtrl->RegOL.conf.Krgpower = kx;
}




/****************************************************************************************************/
/*																									*/
/*		Calculation of regenerative overload level																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the parameters for regenerative overload check.											*/
/*																									*/
/*			1) Regenerative overload allowable power																	*/
/*			2) Regenerative overload alarm level																*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
BOOL	PcalRegeneOverLoadLevel( REGENE_CTRL *RegeneCtrl,USHORT regres, USHORT wre, USHORT ConvW )
{
	BOOL			PrmSetErr;
	LONG			kx, sx;
	CHECK_REGOL		*pRegOL;
//	CMN_PRMDATA		*Cprm;

	/* Local variable initialization */
	PrmSetErr = FALSE;
	pRegOL = &(RegeneCtrl->RegOL);

	/* 2010.09.02 Y.Oka (>_<) There is no way to know the current converter capacity, so substitute a fixed value */
//	Cprm->ConvW = 8000;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of regenerative overload allowable power																	*/
/*--------------------------------------------------------------------------------------------------*/
/*	Σ-7,INGRAM																						*/
/*		RgolPowBase = Regenerative resistor capacity * 5000 / ConvW 		[5000/Converter capacity]						*/
/*											(Σ-7,INGRAM)				*/
/*--------------------------------------------------------------------------------------------------*/
/*	SGDV																							*/
/*		RgolPowBase = Regenerative resistor capacity * 5000 / AmpW 		[5000/Servo rated capacity]						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	if( regres == 0 )									/* If setting value is zero(Pn600)					*/
	{
		pRegOL->conf.RgolPowBase = wre;					/* Regenerative resistor capacity(Default)					*/
	}
//	else if( (Prm->regres * 10) <= Prm->ampw )			/* If less than servo rated output					*/
	else if( (regres * 10) <= ConvW )					/* When within converter capacity					*/
	{
//		pRegOL->conf.RgolPowBase = (Prm->regres * 50000) / Prm->ampw;
		pRegOL->conf.RgolPowBase = (regres * 50000) / ConvW;
	}
	else												/* Cannot set higher than converter output			*/
	{
		/* Parameter setting error */
		PrmSetErr = TRUE;
	}


/*--------------------------------------------------------------------------------------------------*/
/*		Regenerative overload alarm level calculation																*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		SWPF :	RgolAlmLevel = RgolPowBase * 2 * 100 sec											*/
/*							 = RgolPowBase * 2 * 100 * 1000 / SCCYCLEMS								*/
/*																									*/
/*				if( RgolPowSum >= RgolAlmLevel ){ Regeneration overload alarm;}								*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		ΣⅡ :	rgollv = rgbas * 2 * 100 sec														*/
/*					   = rgbas * 2 * 100 * 10^6 / (CYCLE*FORCNTLMT)									*/
/*					   = 2^14 * (rgbas * 12207) / (CYCLE*FORCNTLMT);								*/
/*																									*/
/*				if( rgolsm>>14 >= rgollv ){ Regeneration overload alarm;}									*/
/*																									*/
/*				*1) Initial setting : rgolsm = rgollv << 7;   --- Is this correct?, 50%?			*/
/*																									*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( pRegOL->conf.RgolPowBase, (2*100*1000), KPI_SCCYCLEMS, NULL, -30 );
	pRegOL->conf.RgolAlmLevel = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Regenerative power consumption calculation gain(for Un00A)														*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*								100																	*/
/*		Kunrgpower = -------------------------------------											*/
/*					  RgolPowBase * KPI_SCANC_MS( 10000 )											*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( 100, 1, pRegOL->conf.RgolPowBase, &sx,  0 );
	kx = MlibPcalKxgain( kx,  1, KPI_SCANC_MS( 10000 ),    &sx, 24 );
	pRegOL->conf.Kunrgpower = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Regenerative power consumption calculation gain(for Un143)	10[%]												*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*								100							   1									*/
/*		Kunrgpower = -------------------------------------	* -----									*/
/*					  RgolPowBase * KPI_SCANC_MS( 2 )		   10									*/
/*																									*/
/*		Overflow countermeasure is 10[%]Divide by 10 to display									*/
/*--------------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( 10, 1, pRegOL->conf.RgolPowBase,   &sx,  24 );
	pRegOL->conf.KunrgpowerCycle = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Regenerative resistor capacity acquisition																			*/
/*--------------------------------------------------------------------------------------------------*/
	RegeneCtrl->conf.ResistorCapa = regres;

	return PrmSetErr;
}


/****************************************************************************************************/
/*																									*/
/*		Triple root low-pass filter for main circuit detection AD	Filter time constant parameter calculation						*/
/*																									*/
/****************************************************************************************************/
void	PcmCalculateVdetAdFil( DET_DC_VOLT *pDetectDc, USHORT vdetadfil, LONG SvCycleUs )
{
	if( (SvCycleUs*1000) != KPI_SBCYCLE_BASE_NS )
	{ /* ScanBPeriod 125us */
		pDetectDc->conf.VdetAdFil = MlibPcalKf1gain( vdetadfil, SvCycleUs, 0 );
	}
	else
	{ /* ScanBPeriod 125us */
		pDetectDc->conf.VdetAdFil = MlibPcalKf1gain( vdetadfil, (2 * SvCycleUs), 0 );
	}
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Inrush current prevention circuit control/DB circuit control										<S20E>				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : In response to requests from the main circuit charging sequence and DB sequence, inrush/DB control method selection	*/
/*			.												*/
/*																									*/
/*	Variation Update : INGRAM, the anti-crash controlDBcontrol is separated and anti-shock control is a common process for all units.Σ-7series	*/
/*			was set to [/9] [10] regardless ofΣ-7Wwill be multi-axis, butH/WBecause it is interlocked, it can be controlled for each axis.*/
/*			Because of this,Σ-Ⅴ.													*/
/*																									*/
/*			TestMode5is a function for the test machine, and as it forcibly performs anti-shock control through register access,		*/
/*			this process. SimilarlyTestMode7is forcedDBFunctions for controlling.			*/
/*																									*/
/*			Forcibly protects against collision when motorless test mode is enabledOFF,DBONstate.						*/
/*																									*/
/****************************************************************************************************/
void	PcmRlyDbControlProcess( ASIC_HW_IF_STR *pAsicHwReg, POW_MNG_HNDL *PowerManager,
								BOOL BbSts, BOOL DbOn, BOOL KnlMode, BOOL MotorLessTestMode )
{
	CHECK_MAIN_POWER	*PowerChecker;
	POWER_STATUS		*pPowSts;
	BOOL				RelaySts;
	BOOL				DbSts;

	PowerChecker = &PowerManager->MainPowerChecker;
	pPowSts = &PowerManager->PowerSts;
	RelaySts = FALSE;
	DbSts = TRUE;

/*--------------------------------------------------------------------------------------------------*/
/*		Servo inrush circuit control/DB circuit control															*/
/*--------------------------------------------------------------------------------------------------*/
	switch( PowerChecker->conf.RlyDbWay )
	{
	case DBCTRL_RLY_COMMON:				/* Inrush + DB: Relay common 									*/
		if( (BbSts == TRUE)
			&& ((pPowSts->ChargeOk == FALSE) || ((KnlMode != KPI_KMD_ADJCURAD) && (DbOn == TRUE))) )
		{ /* BB && (Charging incomplete || (!CurrentADDuring adjustment && DBONRequired) ) */
			RelaySts = FALSE;			/* Relay OFF / DB ON */
		}
		else
		{
			RelaySts = TRUE;			/* Relay ON / DB OFF */
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	/*	When the following parameters are set, the parameter calculation will cause a parameter setting error.								*/
	/*	■Inrush + DB: Relay common																	*/
	/*	■Inrush + DB:Relay+Thyristor																*/
	/*----------------------------------------------------------------------------------------------*/
	case DBCTRL_TRIACK_SEPARATE:		/* Rush: Relay; DB: Thyristor 							*/
	case DBCTRL_RLY_SEPARATE:			/* Rush: Relay; DB: Relay 								*/
		/* Rush protection control */
		if( (BbSts == TRUE) && (pPowSts->ChargeOk == FALSE) )
		{ /* BB && Charging incomplete */
			RelaySts = FALSE;			/* Relay OFF */
		}
		else
		{ /* BE || Charging complete */
			RelaySts = TRUE;			/* Relay ON */
		}

		/* DBControl */
		if( (BbSts == TRUE) && ( (KnlMode != KPI_KMD_ADJCURAD) && (DbOn == TRUE)) )
		{ /* BB && (!CurrentADDuring adjustment && DBONRequired) */
			DbSts = TRUE;				/* DB ON */
		}
		else
		{
			DbSts = FALSE;				/* DB OFF */
		}
		break;

	/*----------------------------------------------------------------------------------------------*/
	/*	Inrush + DB: For thyristor,DBONfor controlling pulse output,RelayOnA sudden defense or DBofONA mechanism to do so.	*/
	/*	■DBOFF				: No pulse output, soRelayOn/OffRegardless of the attackOFF/DBON				*/
	/*	■DBON && RelayOn	: Pulse output, anti-bulbON/DBOFF											*/
	/*	■DBON && RelayOff	: Pulse output, anti-bulbOFF/DBON											*/
	/*----------------------------------------------------------------------------------------------*/
	case DBCTRL_TRIACK_COMMON:			/* Inrush + DB: Thyristor 									*/
		if( !((BbSts == TRUE)
			  && ((pPowSts->ChargeOk == FALSE) || ((KnlMode != KPI_KMD_ADJCURAD) && (DbOn == TRUE)))) )
		{
			RelaySts = TRUE;			/* Relay ON */
			DbSts = TRUE;				/* Pulse Output */
		}
		else if( DbOn == TRUE )
		{
			RelaySts = FALSE;			/* Relay OFF */
			DbSts = TRUE;				/* Pulse Output */
		}
		else
		{
			RelaySts = FALSE;			/* Relay OFF */
			DbSts = FALSE;				/* Pulse Stop */
		}
		break;

	default:
		break;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Forcibly protects against collision when motorless test mode is enabledOFF,DBONstate.							*/
	/*----------------------------------------------------------------------------------------------*/
	if( MotorLessTestMode == TRUE )
	{ /* Motorless enabled */
		RelaySts = FALSE;				/* Relay OFF */
		DbSts = TRUE;					/* DB ON */
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Rush protection circuit & DBCircuit output setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( PowerChecker->conf.TestMode5 == FALSE )
	{
		if( ((PowerChecker->conf.RlyDbWay == DBCTRL_TRIACK_COMMON) || (PowerChecker->conf.RlyDbWay == DBCTRL_RLY_COMMON))
			&& (PowerChecker->conf.TestMode7 == TRUE) )
		{ /* Rush + DB: When relays are common,DBUses anti-rush control signal for control */
			; /* DBDo nothing during forced output */
		}
		else
		{
			HALdrv_RelayControl( (void*)pAsicHwReg, RelaySts );	/* Relay ON/OFF */
		}
	}

//	if( (PowerChecker->conf.TestMode7 == FALSE) && (CSW_HW_DB_CTRL == TRUE) )
	if((( PowerChecker->conf.TestMode7 == FALSE )&&( CSW_HW_DB_CTRL == TRUE ))&&
									( PowerChecker->conf.RlyDbWay != DBCTRL_RLY_COMMON ))	/* <S212> */
	{
		if( (PowerChecker->conf.RlyDbWay == DBCTRL_TRIACK_COMMON)
			&& (PowerChecker->conf.TestMode5 == TRUE) )
		{ /* Inrush + DB: For thyristor, rush protection controlDBONUse Signal */
			; /* Does nothing during forced output of thrust protection control */
		}
		else
		{
			if( DbSts == TRUE )
			{
				if( PowerChecker->conf.RlyDbWay == DBCTRL_TRIACK_COMMON )
				{
					KlibWaitus( WAIT_RLY_DB );
				}
				SHalDrv_DynamicBrakeON( pAsicHwReg );			/* DBON */
			}
			else
			{
				SHalDrv_DynamicBrakeOFF( pAsicHwReg );			/* DBOFF */
			}
		}
	}
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Intermediate potential abnormality check process(A.450)	<S053>														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Signal indicating that the intermediate potential of the main circuit capacitor of the 400V class servo pack has shifted (PCOV)		*/
/*			, and change the signal ON to specify the message length including unit conversion [/35] [36] that includesPnE7A.2[100ms]If this continues forA.450Defined as “main circuit capacitor overvoltage.”	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	LpxCheckPcovAlarm( PCOV_ERR_DETECT *PcovErrDetect, POWER_STATUS *PowSts )
{
		if( PcovErrDetect->conf.PcovEna == TRUE )
		{
			if( (PcovErrDetect->var.Pcov == TRUE) && (PowSts->PowerOn == TRUE) )
			{
				if( PcovErrDetect->var.PcovErrCnt >= PcovErrDetect->conf.PcovErrCntLmt )
				{
					ALMSetGlobalAlarm( ALM_PCOV );		/* A.450 : Main circuit capacitor overvoltage alarm	*/
				}
				else
				{
					PcovErrDetect->var.PcovErrCnt++;
				}
			}
			else
			{
				PcovErrDetect->var.PcovErrCnt = 0;
			}
		}
		return;
}



/****************************************************************************************************/
/*																									*/
/*		DCFor voltage acquisitionAPI		<S020>																	*/
/*																									*/
/****************************************************************************************************/
LONG	PcmGetDcVoltage( POWER_STATUS *PowSts )
{
	return	PowSts->DcVolt;
}
/***************************************** end of file **********************************************/

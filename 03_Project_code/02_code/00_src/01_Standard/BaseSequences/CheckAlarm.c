/****************************************************************************************************/
/*																									*/
/*																									*/
/*		CheckAlarm.c : Alarm monitoring processing definition															*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Alarm monitoring processing																		*/
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
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "MLib.h"
#include "KLib.h"
#include "HwDeviceIf.h"
#include "SerialEncIf.h"
#include "AlarmManager.h"
#include "CheckAlarm.h"

/* <S154> Start */
/*--------------------------------------------------------------------------------------------------*/
/*			Overload calculation area selection																			*/
/*--------------------------------------------------------------------------------------------------*/
#define NORMAL_AREA			1U
#define LOWSPD_AREA			2U
#define LOCK_AREA			3U

/*--------------------------------------------------------------------------------------------------*/
/*		Internal control board temperature abnormal alarm check processing definition												*/
/*--------------------------------------------------------------------------------------------------*/
#define MINUTE		60000U

/* <S154> End */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
static void	AdetCheckSencAlarmCode( MENCV *MencV, ALMMAN_HANDLE *AlmMngr );
//static void	AdetExeCheckOLAmpMot( LONG AbsMotSpd, LONG Ixsqr, CHECK_OL *ol_ptr, ALMMAN_HANDLE *AlmMngr );			/* <S154> */
static void	AdetExeCheckOLAmpMot( LONG AbsMotSpd, LONG Ixsqr, CHECK_OL *ol_ptr, ALMMAN_HANDLE *AlmMngr, UCHAR Mode );	/* <S154> */
static BOOL	AdetPcalOverLoadLevelAmpLow( CHECK_ALARM *CheckAlarm, LONG AbsMotSpd); 										/* <S154> */




/****************************************************************************************************/
/*																									*/
/*		Initialize Servo Alarm Check Variables														*/
/*																									*/
/****************************************************************************************************/
void	AdetInitCheckAlarm( CHECK_ALARM *pChkAlm, ULONG AlmOLInfo )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Clear all variables to zero for alarm check															*/
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( &(pChkAlm->MotOL.var), sizeof( pChkAlm->MotOL.var )/4 );
	MlibResetLongMemory( &(pChkAlm->AmpOL.var), sizeof( pChkAlm->AmpOL.var )/4 );
	MlibResetLongMemory( &(pChkAlm->HwbbTiming.var), sizeof( pChkAlm->HwbbTiming.var )/4 );

/*--------------------------------------------------------------------------------------------------*/
/*		Initialization of variables for amplifier & motor overload check													*/
/*--------------------------------------------------------------------------------------------------*/
	if( AlmOLInfo & ALMDEF_OLINFO_OLF )
	{
#if 1	/* <S154> */
		pChkAlm->MotOL.var.SumImax = (pChkAlm->MotOL.conf.NormalArea.AlmLvlmax >> 1);
		pChkAlm->MotOL.var.SumImid = (pChkAlm->MotOL.conf.NormalArea.AlmLvlmid >> 1);
		pChkAlm->AmpOL.var.SumImax = (pChkAlm->AmpOL.conf.NormalArea.AlmLvlmax >> 1);
		pChkAlm->AmpOL.var.SumImid = (pChkAlm->AmpOL.conf.NormalArea.AlmLvlmid >> 1);
#else	/* <S154> */
		pChkAlm->MotOL.var.SumImax = (pChkAlm->MotOL.conf.AlmLvlmax >> 1);
		pChkAlm->MotOL.var.SumImid = (pChkAlm->MotOL.conf.AlmLvlmid >> 1);
		pChkAlm->AmpOL.var.SumImax = (pChkAlm->AmpOL.conf.AlmLvlmax >> 1);
		pChkAlm->AmpOL.var.SumImid = (pChkAlm->AmpOL.conf.AlmLvlmid >> 1);
#endif	/* <S154> */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Initialize variables for DB overload check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( AlmOLInfo & ALMDEF_OLINFO_DBOL )
	{
		pChkAlm->DbOL.var.DbolPowSum = (pChkAlm->DbOL.conf.DbolAlmLevel >> 1);
	}

}



/****************************************************************************************************/
/*																									*/
/*		ASIC hardware detection abnormality check process															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitoring abnormalities detected by ASIC hardware											*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
USHORT	AdetCheckAsicHwAlarm( CHECK_ALARM *pChkAlm, ASICS *SvAsic,
										BOOL EncDisable, ALMMAN_HANDLE *AlmMngr )
{
	USHORT 	faultState;
	USHORT	microFault;

	faultState = SHalDrv_GetFaultStatus(SvAsic->AsicHwReg, SHAL_FAULTS);
/*----------------------------------------------------------------------------------------------*/
/*		MicroAbnormal																				*/
/*----------------------------------------------------------------------------------------------*/
	if( (faultState & SHAL_FAULT_BB) == 0 )
	{ /* FLTSTAT_bit10  0:Base block normal */
//		microFault = SvAsic->AsicMicroReg->MREG_FCCSTMON;
		microFault = SHalDrv_GetMicroFaultSts( SvAsic->AsicMicroReg );
		if( (microFault & 0x0001) != 0 )
		{
			ALMSetServoAlarm( AlmMngr, ALM_CURAD_U );	/* A.B31 : Current detection error 1(U phase)			*/
		}
		if( (microFault & 0x0002) != 0 )
		{
			ALMSetServoAlarm( AlmMngr, ALM_CURAD_V );	/* A.B32 : Current detection error 2(V phase)			*/
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Host-CPU : Watchdog error																*/
/*--------------------------------------------------------------------------------------------------*/
	if( (faultState & SHAL_FAULT_WDOUT) != 0 )
	{ /* FLTSTAT_bit9  1:HostCPUForWDTTimeover */
		/* A.BF4 : System alarm 4(CpuWDT) */
		ALMSetServoAlarm( AlmMngr, ALM_JL056_GUP );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Asic-CPU : Watchdog error																*/
/*--------------------------------------------------------------------------------------------------*/
	if( ((faultState & SHAL_FAULT_GUP1) != 0) && (EncDisable == FALSE) )
	{ /* FLTSTAT_bit11 1:Micro calculation unitWDTTimeover */
		if( pChkAlm->Gup1Timer >= KPI_SCANC_MS( 8 ))
		{
			/* A.B33 : Current detection error 3(MicroWDT) */
			//ALMSetServoAlarm( AlmMngr, ALM_JL056_GUP1 );
			ALMSetGlobalAlarm( ALM_JL056_GUP1 );/*<S1BC>*/
			pChkAlm->Gup1Timer = KPI_SCANC_MS( 8 );
		}
		pChkAlm->Gup1Timer++;
	}
	else
	{
		pChkAlm->Gup1Timer = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Overcurrent(OC)Check																			*/
/*--------------------------------------------------------------------------------------------------*/
#if 0/* <S126>:This function does not perform this function. */
	{/* 0Axis number	*/
	if( (SHalDrv_GetMicroWdtStatus(SvAsic->AsicHwReg) == 0)
		&& ((faultState & SHAL_FAULT_OC) != 0) )
	{ /* WDT1SET_bit6  0:For micro calculation unitWDTIn operation && FLTSTAT_bit5  1:Overcurrent abnormality occurrence */
		/* A.100 : Overcurrent(OC) */
		ALMSetServoAlarm( AlmMngr, ALM_OC );
	}
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Heat sink overheating(OH)Check																*/
/*--------------------------------------------------------------------------------------------------*/
#if 0	/* MD <S058> */
	if( (faultState & SHAL_FAULT_OH) != 0 )
	{ /* FLTSTAT_bit4  1:Heat sink overheating error occurred(INGRAMJL-077BofOVAt portOHDetection) */
		ALMSetServoAlarm( AlmMngr, ALM_OH );				/* A.7A0 : Heat sink overheating				*/
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Overvoltage(OV)Check <S058>																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (faultState & SHAL_FAULT_OV) != 0 )
	{
//-		SHalDrv_BaseBlock( SvAsic->AsicHwReg );		/* ASIC base block						*/
		/* BaseBlock */
		SHalDrv_BaseBlock(SvAsic->AsicHwReg, SvAsic->AsicMicroReg); /* <S0E4> */

		ALMSetServoAlarm( AlmMngr, ALM_OV );		/* A.400 : Overvoltage(OV)							*/
	}

	pChkAlm->faultState = faultState;
	return faultState;
}


/****************************************************************************************************/
/*																									*/
/*		ASIC hardware detection abnormality check process															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitoring abnormalities detected by ASIC hardware											*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckOCAlarmDetections( ALMMAN_HANDLE *AlmMngr, ASICS *SvAsic, LONG axisNo, BOOL BeSts )
{
	USHORT 	faultState;

	if(axisNo == 0)
	{/* 0Axis number	*/
		faultState = SHalDrv_GetFaultStatus(SvAsic->AsicHwReg, SHAL_FAULTS);
//		if( (SHalDrv_GetMicroWdtStatus(SvAsic->AsicHwReg) == 0)		/* <S137> */
		if(( BeSts == TRUE )
			&& ((faultState & SHAL_FAULT_OC1) != 0) )
		{ /* WDT1SET_bit6  0:For micro calculation unitWDTIn operation && FLTSTAT_bit5  1:Overcurrent abnormality occurrence */
			/* A.100 : Overcurrent(OC) */
			ALMSetServoAlarm( AlmMngr, ALM_OC );
		}
	}
	else
	{/* 1Axis number	*/
		faultState = SHalDrv_GetSetreg1Status(SvAsic->AsicHwReg);
//		if( (SHalDrv_GetMicroWdtStatus((ASIC_HW_IF_STR *)(0xC0001000)) == 0)		/* <S137> */
		if(( BeSts == TRUE )
			&& ((faultState & SHAL_FAULT_OC2) != 0) )
		{ /* WDT1SET_bit6  0:For micro calculation unitWDTIn operation && FLTSTAT_bit5  1:Overcurrent abnormality occurrence */
			/* A.100 : Overcurrent(OC) */
			ALMSetServoAlarm( AlmMngr, ALM_OC );
		}
	}

	/* Get OC Status for Auto Test *//* <S1C9> */
	faultState = SHalDrv_GetFaultStatus(SvAsic->AsicHwReg, SHAL_FAULTS);
	AlmMngr->OCAlmOccurFlag1 = ((faultState & SHAL_FAULT_OC1) != 0) ? TRUE : FALSE;					  /* <Oka05> */
	faultState = SHalDrv_GetSetreg1Status(SvAsic->AsicHwReg);
	AlmMngr->OCAlmOccurFlag2 = ((faultState & SHAL_FAULT_OC2) != 0) ? TRUE : FALSE;					  /* <Oka05> */

}



/*<S1F5> START*/
/*****************************************************************************
  Description: Motor overcurrent detection check processing
------------------------------------------------------------------------------
  Parameters:
	AlmMngr             Alarm management IF data
	SvAsic				ASICPointer to address table
	MencP				Encoder parameter definition
	BeSts               Base enable completion status
  Return:

------------------------------------------------------------------------------
  Note:

*****************************************************************************/
void	AdetCheckMotorOverCurrent( ALMMAN_HANDLE *AlmMngr, ASICS *SvAsic, BOOL BeSts, BOOL BaseEnableSts )
{
	USHORT 	faultState;

//	if( BaseEnableSts == TRUE )
//	{ /* SVONIf there is an axis in the middle */
//		faultState = SHalDrv_GetSetreg1Status(SvAsic->AsicHwReg);
//		if(( BeSts == TRUE ) && ( ((faultState & SHAL_FAULT_OCALMULT) != 0) ||
//				((faultState & SHAL_FAULT_OCALMVLT) != 0) || ((faultState & SHAL_FAULT_OCALMWLT) != 0) ) )
//		{/* U phase orV phase orWphase is "abnormal occurrence" */
//			ALMSetServoAlarm( AlmMngr, ALM_OVERCURRENT );		/* A.101 : Motor overcurrent				*/
//		}
//	}
/* <S1F7> */
	if( BeSts == TRUE )
	{
		faultState = SHalDrv_GetSetreg1Status(SvAsic->AsicHwReg);
		if( faultState & ( SHAL_FAULT_OCALMULT | SHAL_FAULT_OCALMVLT | SHAL_FAULT_OCALMWLT ))
		{
			ALMSetServoAlarm( AlmMngr, ALM_OVERCURRENT );		/* A.101 : Motor overcurrent				*/
		}
	}
}
/*<S1F5> END*/



/****************************************************************************************************/
/*																									*/
/*		Serial encoder alarm monitoring process(Called from ScanC)										*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Serial encoder communication error,Monitor overheat and alarm codes.					*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckSencAlarm( ALMMAN_HANDLE *AlmMngr, MENCV *MencV, USHORT RxAlmCode )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Update Motor Encoder Ready Flag																*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->EncConnect && (!MencV->EncDisable) )
	{
		MencV->EncReady = MencV->SenReady;
	}
	else
	{
		MencV->EncReady = FALSE;
	}
	MencV->RxAlmCode = RxAlmCode;
/*--------------------------------------------------------------------------------------------------*/
/*		Check execution : Communication normal and Scan A running												*/
/*--------------------------------------------------------------------------------------------------*/
	if( (MencV->SeRxErr == FALSE) && (MencV->EncDisable == FALSE) )
	{
		AdetCheckSencBaseTimer( MencV, AlmMngr );			/* Encoder base timer monitoring<S0B9>	*/
		AdetCheckSencAlarmCode( MencV, AlmMngr );			/* Encoder alarm code monitoring			*/
	/*----------------------------------------------------------------------------------------------*/
//		if( MencV->AbsoEncoder == TRUE ) /* <S03A> */
		if( (MencV->AbsoEncoder == TRUE) || (MencV->IncDetOhFlg == TRUE) )
		{
			AdetCheckSencOverHeat( MencV, AlmMngr );		/* Encoder overheat monitoring			*/
		}
	/*----------------------------------------------------------------------------------------------*/
		if( MencV->ChkAlarmCnt < 10 )
		{
			MencV->ChkAlarmCnt++;							/* Update check counter				*/
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Check not executed : Communication error or Scan A stopped												*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		MencV->ChkAlarmCnt = 0;								/* Clear check counter				*/
	}

}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder base timer monitoring<S0B9>													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : When the serial encoder is in P1 mode, the encoder timebase timer changes				*/
/*			If you do not do this, an encoder communication error alarm will be generated.									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckSencBaseTimer( MENCV *MencV, ALMMAN_HANDLE *AlmMngr )
{
	LONG	BaseTimer;
	BaseTimer = MencV->BaseTimer;								/* Timebase timer current value			*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->ChkAlarmCnt >= 2 )								/* Check from the third time onwards			*/
	{
		if( BaseTimer == MencV->ChkBaseTimer )					/* Timebase timer no change?		*/
		{
			/* Encoder communication error check */
			if( !ALMCheckEachState( AlmMngr, ALM_PGCOM ) )
			{
				ALMSetServoAlarm( AlmMngr, ALM_PGCOM2 );		/* A.C92 : Encoder communication timer error	*/
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	MencV->ChkBaseTimer = BaseTimer;							/* Timebase timer previous value update		*/

}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder alarm code monitoring														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : P1 mode serial encoder reception alarm code with absolute encoder						*/
/*			Incremental encoders are used for discrimination.											*/
/*			Variable for alarm code if reception fails(p1mode_d0)Clear.						*/
/*			Check the alarm only if reception is successful, but also check if reception fails immediately before reading.			*/
/*			After reading the read alarm code twice and applying filter, absolute or incremental			*/
/*			and call each alarm code analysis function.										*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static void	AdetCheckSencAlarmCode( MENCV *MencV, ALMMAN_HANDLE *AlmMngr )
{
	LONG	AlmCode;
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder alarm code monitoring processing															*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->ChkAlarmCnt >= 2 )						/* Check from the third time onwards				*/
	{
		MencV->ChkAlmCode[0] =
			( ((MencV->ChkAlmCode[1] | MencV->RxAlmCode) & MencV->ChkAlmCode[0] )
				| (MencV->ChkAlmCode[1] & MencV->RxAlmCode) );
		MencV->ChkAlmCode[1] = MencV->RxAlmCode;

		/* Alarm code assignment */
		AlmCode = MencV->ChkAlmCode[0];

		if( MencV->AbsoEncoder == TRUE )
		{ /* Absolute encoder */
			if( MencV->AxisMotType == MOTTYPE_LINEAR )
			{ /* Linear type */
			/*--------------------------------------------------------------------------------------------------*/
			/*		EnDatMask during scale																		*/
			/*--------------------------------------------------------------------------------------------------*/
				if( MencV->SerialConvType == SCONVTYPE_ENDAT_ABS )
				{
					AlmCode = ( AlmCode & 0x0b );
				}
			/*--------------------------------------------------------------------------------------------------*/
			/*		Mitutoyo, SMSMask during scale																*/
			/*--------------------------------------------------------------------------------------------------*/
				else
				{
					AlmCode = ( AlmCode & 0x3a );
				}

				if( AlmCode & 0x01 )
				{
					/* A.890 : Encoder scale error */
					ALMSetServoAlarm( AlmMngr, ALM_SCL_ERR );
				}
			/*----------------------------------------------------------------------------------------------*/
				if( AlmCode & 0x02 )
				{
					/* A.891 : Encoder module error */
					ALMSetServoAlarm( AlmMngr, ALM_MOD_ERR );
				}
			/*----------------------------------------------------------------------------------------------*/
				if( AlmCode & 0x08 )
				{
					/* A.840 : Encoder data alarm */
					ALMSetServoAlarm( AlmMngr, ALM_PG4 );
				}
			/*----------------------------------------------------------------------------------------------*/
				if( AlmCode & 0x10 )
				{
					/* A.850 : Encoder overspeed */
					ALMSetServoAlarm( AlmMngr, ALM_PG5 );
				}
			/*----------------------------------------------------------------------------------------------*/
				if( AlmCode & 0x20 )
				{
					/* A.860 : Encoder overheat */
					ALMSetServoAlarm( AlmMngr, ALM_PG6 );
				}
			}
			else
			{ /* Rotation type */
				if( MencV->AbsoEncType != MENC_MABSO_TYPE )
				{ /* When increment is used or One-rotation absolute mask */
					AlmCode = ( AlmCode & 0x3a );
				}

				/* Alarm code check processing */
				if( AlmCode & 0x01 )
				{ /* A.810 : Encoder backup alarm */
					ALMSetServoAlarm( AlmMngr, ALM_PG1 );
				}
				if( AlmCode & 0x02 )
				{ /* A.820 : Encoder sum check alarm */
					ALMSetServoAlarm( AlmMngr, ALM_PG2 );
				}
				if( AlmCode & 0x04 )
				{
					if( (4000 > KlibGetTimeAfterPowerOn()) && (MencV->BatWrnMode == FALSE) )
					{ /* A.830 : Encoder battery alarm */
						ALMSetServoAlarm( AlmMngr, ALM_PG3 );
					}
			/*------------------------------------------------------------------------------------------*/
					else if(MencV->BatWrnMode == TRUE)
					{ /* A.930 : Encoder battery warning */
						ALMSetServoAlarm( AlmMngr, WRN_PGBAT );
					}
				}
				else
				{ /* A.930 : Encoder battery warning */
					ALMClearWarning( AlmMngr, WRN_PGBAT );
				}
			/*----------------------------------------------------------------------------------------------*/
				if( AlmCode & 0x08 )
				{ /* A.840 : Encoder data alarm */
					ALMSetServoAlarm( AlmMngr, ALM_PG4 );
				}
			/*----------------------------------------------------------------------------------------------*/
				if( AlmCode & 0x10 )
				{ /* A.850 : Encoder overspeed */
					ALMSetServoAlarm( AlmMngr, ALM_PG5 );
				}
			/*----------------------------------------------------------------------------------------------*/
				if( AlmCode & 0x20 )
				{ /* A.860 : Encoder overheat */
					ALMSetServoAlarm( AlmMngr, ALM_PG6 );
				}
			}
		}
		else
		{ /* Increment encoder */
			/*	Alarm code check processing */
			if( AlmCode & 0x02 )
			{ /* A.820 : Encoder sum check alarm */
				ALMSetServoAlarm( AlmMngr, ALM_PG2 );
			}
			if( AlmCode & 0x08 )
			{ /* A.840 : Encoder data alarm */
				ALMSetServoAlarm( AlmMngr, ALM_PG4 );
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	else													/* Do not check until the second time			*/
	{
		MencV->ChkAlmCode[0] = 0;						/* Alarm code reset				*/
		MencV->ChkAlmCode[1] = 0;						/* Alarm code reset				*/
	}

}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder overheat monitoring															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Detects encoder internal temperature and overheating.									*/
/*																									*/
/*			0x00 ~ 0xD7 :  0 ~ 215 deg															*/
/*			0xFF ~ 0xD8 : -1 ~ -40 deg															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckSencOverHeat( MENCV *MencV, ALMMAN_HANDLE *AlmMngr )
{
//	LONG	enc_temperatur; /* <S03A> */
	USHORT	enc_temperatur;

//	enc_temperatur = MencV->RxAlmCode & 0xff00;	/* <S03A> */
//	enc_temperatur = enc_temperatur >> 8;		/* <S03A> */
	enc_temperatur = (MencV->RxAlmCode >> 8) & 0x00ff;

/*--------------------------------------------------------------------------------------------------*/
/*		Encoder internal temperature setting																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( enc_temperatur < 0xd8 )
	{
//		MencV->Temperature = enc_temperatur;					/* Encoder internal temperature(Correct)				*//* <S03A> */
		MencV->Temperature = (UCHAR)enc_temperatur;				/* Encoder internal temperature(Correct)				*/
	}
	else
	{
//		MencV->Temperature = -enc_temperatur;					/* Encoder internal temperature(Negative)				*//* <S03A> */
		MencV->Temperature = (CHAR)enc_temperatur;				/* Encoder internal temperature(Negative)				*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Encoder internal temperature check																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->Temperature > PG_OH_LEVEL )					/* Temperature level check					*/
	{
		if( MencV->OverHeatCnt < KPI_SCANC_MS( 500 ) )		/* Error counter check				*/
		{
			MencV->OverHeatCnt++;							/* Count up Error Counter				*/
		}
		else												/* 500ms, it will cause an alarm.				*/
		{
			ALMSetServoAlarm( AlmMngr, ALM_PG6 );			/* A.860 : Encoder overheat		*/
		}
	}
	else
	{
		MencV->OverHeatCnt = 0;							/* Error counter reset				*/
	}
}



/****************************************************************************************************/
/*																									*/
/*		Overspeed(OS)Check processing																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : If the motor rotation speed exceeds the overspeed detection level, an overspeed alarm is generated.				*/
/*			Overspeed detection level is the encoder parameter motor maximum rotation speed(PnF05.The upper byte),		*/
/*			OS detection level(PnF06.Lower byte)From the system constant OS detection level(PnE62.The upper byte)		*/
/*			, but if both forward and reverse ends normally, regardless of the receive buffer[/78] [79] or [/79] [80] until the transmission start command is received by24bit(0x1000000).							*/
/*																									*/
/*	Caution: The update cycle of the motor rotation speed being used is250usTherefore, it is necessary to review						*/
/****************************************************************************************************/
void	AdetCheckOverSpeed( ALMMAN_HANDLE *AlmMngr, CHECK_ALARM *pChkAlm, LONG MotSpd, BPRMDAT *Bprm )
{
	LONG	AbsMotSpd;

	AbsMotSpd = MlibLABS( MotSpd );
/*--------------------------------------------------------------------------------------------------*/
/*		Overspeed(OS)Check																		*/
/*--------------------------------------------------------------------------------------------------*/
//	if( AbsMotSpd >= Bprm->NorOvrSpd )
	if( AbsMotSpd >= Bprm->OsAlmLvl )	/* <S1D0> */
	{
		pChkAlm->OSCount++;
		if( pChkAlm->OSCount >= Bprm->OSCountMax )
		{
			pChkAlm->OSCount = Bprm->OSCountMax;
			ALMSetServoAlarm( AlmMngr, ALM_OSF1 );			/* A.510 : Overspeed(OS)Alarm			*/
		}
	}
	else
	{
		pChkAlm->OSCount = 0;
	}
}



/****************************************************************************************************/
/*																									*/
/*		A.511:Division pulse output overspeed check process									<S02D>				*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : If the motor rotation speed exceeds the overspeed detection level, an overspeed alarm is generated.				*/
/*			Overspeed detection level is the encoder parameter motor maximum rotation speed(PnF05.The upper byte),		*/
/*			OS detection level(PnF06.Lower byte)From the system constant OS detection level(PnE62.The upper byte)		*/
/*			, but if both forward and reverse ends normally, regardless of the receive buffer[/78] [79] or [/79] [80] until the transmission start command is received by24bit(0x1000000).							*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckDivOverSpeed( ALMMAN_HANDLE *AlmMngr, LONG MotSpd, LONG DivOvrSpd )
{
	LONG	AbsMotSpd;

	AbsMotSpd = MlibLABS( MotSpd );

/*--------------------------------------------------------------------------------------------------*/
/*		Division output pulse overspeed check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( ALMCheckEachState( AlmMngr, ALM_OSF1 ) ) /*<S079>*/
	{
		;											/* A.510, there is no divided output pulse overspeed check	*/
	}
	else if( AbsMotSpd >= DivOvrSpd )
	{
		ALMSetServoAlarm( AlmMngr, ALM_OSDIV );				/* A.511 : Division pulse output overspeed			*/
	}
/*--------------------------------------------------------------------------------------------------*/

}



/****************************************************************************************************/
/*																									*/
/*		Amplifier & motor overload check processing		Low speed OL: SGDS method								*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : If the current flows beyond the allowable range of the motor or amplifier, process the error.						*/
/*			Motors and amplifiers have different overload characteristics, so they are handled separately.								*/
/*																									*/
/*			Torque command∝Current, current^2∝Assuming the heat value, calculate the heat value by integrating the square of the torque command.			*/
/*			Calculate the heat value. Considering that the temperature is saturated within the allowable value below the rated torque,						*/
/*																									*/
/*			Σ(Torque command^2- Rated torque^2)> Overload detection level										*/
/*																									*/
/*			, it is considered as an overload abnormality.														*/
/*																									*/
/*			Since the actual overload characteristic does not match the square calculation of the torque command, it is approximated by two square calculations.			*/
/*			Instantaneous overload, when overload is detected in a short time with large torque, when the rated torque is exceeded			*/
/*			Overload detection that takes time is distinguished from continuous overload.									*/
/*			This was detected due to the processing method, but instantaneous overload is caused by accidents such as machine jamming			*/
/*			has a strong implication of an abnormality, and continuous overload is interpreted as the load becoming larger due to changes over time.			*/
/*			. Therefore, the two overload detection alarm codes will be separated and distinguished.			*/
/*																									*/
/*			If overload detection is masked with system auxiliary function, the detection process will not pass.						*/
/*																									*/
/****************************************************************************************************/
//void	AdetCheckOverLoadAmpMot( ALMMAN_HANDLE *AlmMngr, CHECK_ALARM *CheckAlarm,
//								 LONG IdRefMon, LONG IqRefMon, LONG AbsMotSpd, BOOL EncConnect )	/* <S154> */
BOOL	AdetCheckOverLoadAmpMot( ALMMAN_HANDLE *AlmMngr, CHECK_ALARM *CheckAlarm,
								 LONG IdRefMon, LONG IqRefMon, LONG AbsMotSpd, BOOL EncConnect )	/* <S154> */
{
	LONG		Ixsqr;
	CHECK_OL	*MotOL;
	CHECK_OL	*AmpOL;
	CHECK_LOWOL *LowOL;		/* <S154> */
	LOWOLCHKP	*LowOLChkP;	/* <S154> */
	BOOL		ret;		/* <S154> */


	/* Local variable initialization */
	Ixsqr =	(IdRefMon * IdRefMon) + (IqRefMon * IqRefMon);
	MotOL = &(CheckAlarm->MotOL);
	AmpOL = &(CheckAlarm->AmpOL);
	LowOL = &(CheckAlarm->AmpLowOL);	/* <S154> */
	LowOLChkP = &(LowOL->LowOLChkP);	/* <S154> */
	ret = TRUE;							/* <S154> */


	/****************************************************************************************/
	/*		Motor overload alarm check													*/
	/****************************************************************************************/
	/*--------------------------------------------------------------------------------------*/
	/* 	Motor normal section OL check															*/
	/*--------------------------------------------------------------------------------------*/
//	AdetExeCheckOLAmpMot( AbsMotSpd, Ixsqr, MotOL, AlmMngr );				/* <S154> */
	AdetExeCheckOLAmpMot( AbsMotSpd, Ixsqr, MotOL, AlmMngr, NORMAL_AREA );	/* <S154> */

/* <S154> Start */
	/****************************************************************************************/
	/*		Amplifier overload alarm check													*/
	/****************************************************************************************/
#if	CSW_SVCF_CHKAMPLOL == 1	/* With low speed OL:CSW_SVCF_CHKAMPLOL(Presence or absence of low-speed OL function depending on model definition)	*/
	/*--------------------------------------------------------------------------------------*/
	/*	Amplifier normal section OL check: Low speed OL cannot be executed or the motor speed is in the normal section		*/
	/*--------------------------------------------------------------------------------------*/
	if( (LowOLChkP->conf.LowSpdOLChkEnable == (UCHAR)FALSE) || (AbsMotSpd > LowOLChkP->conf.LowSpdOLchg) )
	{
		AdetExeCheckOLAmpMot( AbsMotSpd, Ixsqr, AmpOL, AlmMngr, NORMAL_AREA);
	}
	/*--------------------------------------------------------------------------------------*/
	/*	Amplifier low speed section OL check: Motor speed is low speed section									*/
	/* 		AdetPcalOverLoadLevelAmpLow( )…Parameter calculation function for amplifier low speed section overload check	*/
	/*		AdetExeCheckOLAmpMot( )…Amplifier low speed section OL check function							*/
	/*--------------------------------------------------------------------------------------*/
	else if( (AbsMotSpd <= LowOLChkP->conf.LowSpdOLchg) && (LowOLChkP->conf.LockSpdOLchg <= AbsMotSpd) )
	{
		ret = AdetPcalOverLoadLevelAmpLow( CheckAlarm, AbsMotSpd);
		AdetExeCheckOLAmpMot( AbsMotSpd, Ixsqr, AmpOL, AlmMngr, LOWSPD_AREA);
	}
	/*--------------------------------------------------------------------------------------*/
	/*	Amplifier lock section OL check: Motor speed locked section								*/
	/*--------------------------------------------------------------------------------------*/
	else
	{
		AdetExeCheckOLAmpMot( AbsMotSpd, Ixsqr, AmpOL, AlmMngr,LOCK_AREA);
	}
#else
	/*--------------------------------------------------------------------------------------*/
	/*	Without low speed OL																		*/
	/* 	Amplifier normal section OL check															*/
	/*--------------------------------------------------------------------------------------*/
		AdetExeCheckOLAmpMot( AbsMotSpd, Ixsqr, AmpOL, AlmMngr, NORMAL_AREA);
#endif
/* <S154> End */

	/****************************************************************************************/
	/*		Overload warning check														*/
	/****************************************************************************************/
	if( EncConnect == TRUE )				/* Warning not detected if encoder is not connected	*/
	{
		if( MotOL->var.WrnInfo || AmpOL->var.WrnInfo )
		{
			ALMSetServoAlarm( AlmMngr, WRN_OLF );			/* A.910 : Overload warning detection	*/
		}
		else
		{
			ALMClearWarning( AlmMngr, WRN_OLF );			/* A.910 : Overload warning release	*/
		}
	}
	return ret;		/* <S154> */
}

/* <S154> Start */
#if		( CSW_SVCF_CHKAMPLOL == 1 ) 	
/*****************************************************************************
  Description:		Calculation of amplifier low speed overload level(Online)
------------------------------------------------------------------------------
  Parameters:
	AlmMngr;		Alarm management data points
	CheckAlarm;		Alarm management data points
	AbsMotSpd;		Absolute Motor Speed
  Return:
	BOOL			TRUE No alarm
					FALSE	With alarm
------------------------------------------------------------------------------
  Note:
	When the motor rotates at low speed, the OL parameters are continuously interpolated according to the speed and the OL curve is continuously switched.
	If the motor rotation speed is within the low speed section, calculate the intermediate OL current and maximum OL time according to the motor speed.
	Using the calculated intermediate OL current and maximum OL time, set the OL detection alarm level for the low speed section.
*****************************************************************************/
static BOOL	AdetPcalOverLoadLevelAmpLow( CHECK_ALARM *CheckAlarm, LONG AbsMotSpd)
{
	LONG	Ix;
	LONG	Iy;
	LONG	Ibsqr;
	LONG	Ixsqr;
	LONG	Iysqr;
	LONG	Izsqr;
	LONG	wk;
	BOOL	ret;

	OLPRM *AmpLow;
	CHECK_OLP *AmpLowOLP;
	CHECK_OL *MotOLP;
	CHECK_OL *AmpOLP;
	CHECK_LOWOL *LowOL;
	LowOL = &(CheckAlarm->AmpLowOL);
	MotOLP = &(CheckAlarm->MotOL);
	AmpOLP = &(CheckAlarm->AmpOL);
	AmpLow = &(LowOL->LowOLPrm.AmpLow);
	AmpLowOLP = &(AmpOLP->conf.LowSpdArea);
	ret = TRUE;


	/********************************************************************************************/
	/*		Calculation of low speed section intermediate OL current and low speed section maximum OL time									*/
	/********************************************************************************************/
	/*------------------------------------------------------------------------------------------*/
	/*	Calculation of low speed section intermediate OL current:	Ix														*/
	/*------------------------------------------------------------------------------------------*/
	/*	wk = (Motor speed - Lock section inflection point speed) * (Intermediate OL current conversion coefficient) 						*/
	/*	Low speed section intermediate OL current = Lock section inflection point OL current + wk									*/
	/*------------------------------------------------------------------------------------------*/
	wk = MlibMulgain( (AbsMotSpd - LowOL->LowOLChkP.conf.LockSpdOLchg), LowOL->LowOLChkP.conf.TransOLImid );
	Ix = LowOL->LowOLChkP.conf.LowSpdOffsetImid + wk;

	/*------------------------------------------------------------------------------------------*/
	/*	Low speed section maximum OL time calculation:	Iy														*/
	/*------------------------------------------------------------------------------------------*/
	/*	wk = (Motor speed-Lock section inflection point speed) * (Maximum OL time conversion coefficient) 						*/
	/*	Low speed section maximum OL time = Lock section inflection point OL time +wk										*/
	/*------------------------------------------------------------------------------------------*/
	wk = (LONG)MlibSqrtu32( AbsMotSpd - LowOL->LowOLChkP.conf.LockSpdOLchg );
	wk = MlibMulgain( wk, LowOL->LowOLChkP.conf.TransOLTmax );
	Iy = LowOL->LowOLChkP.conf.LowSpdOffsetTmax + wk;

	/*------------------------------------------------------------------------------------------*/
	/*	Low speed section continuous base current^2:		Ibsqrmid,	Ibsqr										*/
	/*------------------------------------------------------------------------------------------*/
	Ibsqr = AmpOLP->conf.NormalArea.Ibsqrmid;
	AmpLowOLP->Ibsqrmid = Ibsqr;						/* Continuous OL base current^2				*/

	/********************************************************************************************/
	/*		Low speed section continuous overload level calculation														*/
	/********************************************************************************************/
	/*																							*/
	/*		Ibsqr = Ibse * Ibse																	*/
	/*																							*/
	/*				  (Imid^2 - Ibse^2) * Tmid													*/
	/*		AlmLvl = ----------------------------												*/
	/*						  Cycle																*/
	/*																							*/
	/********************************************************************************************/
	/*------------------------------------------------------------------------------------------*/
	/* 	Checking intermediate OL current:	Ix															*/
	/*------------------------------------------------------------------------------------------*/
	Ix = (Ix > 0x7FFF)? 0x7FFF : Ix;
	Ixsqr = (Ix * Ix) >> 16;						/* Intermediate OL current^2							*/

	/*------------------------------------------------------------------------------------------*/
	/*	Calculation of continuous OL detection level and level gain													*/
	/*------------------------------------------------------------------------------------------*/
	/*	Izsqr		: Numerator term of continuous OL detection alarm level(Intermediate OL current^2 - Continuous OL base current^2)	*/
	/*	AlmLvlmid	: Continuous OL alarm detection level												*/
	/*	OlLvlmidGain: Continuous OL detection level gain													*/
	/*------------------------------------------------------------------------------------------*/
	Izsqr = Ixsqr - Ibsqr;
	AmpLowOLP->AlmLvlmid = MlibScalKxgain( Izsqr, 100*AmpLow->Tmid, KPI_SCCYCLEMS, NULL, -30 );
	AmpLowOLP->OlLvlmidGain = MlibScalKxgain( AmpOLP->conf.NormalArea.AlmLvlmid, 1, AmpLowOLP->AlmLvlmid, NULL, 24 );
	if( AmpLowOLP->AlmLvlmid >= 0x40000000 )
	{
//			ALMSetPramError( AlmMngr, pndef_olspdlvl.RegNumber);			/* A.040 : Parameter setting error				*/
		ret = FALSE;
	}

	/********************************************************************************************/
	/*		Low speed section instantaneous overload level calculation														*/
	/********************************************************************************************/
	/*																							*/
	/*				 (Imid^2 * Tmid) - (Imax^2 * Tmax)											*/
	/*		Ibsqr = -----------------------------------											*/
	/*						 (Tmid - Tmax)														*/
	/*																							*/
	/*				  (Imax^2 - Ibsqr) * Tmax													*/
	/*		AlmLvl = ----------------------------												*/
	/*						   Cycle															*/
	/*																							*/
	/********************************************************************************************/
	/*------------------------------------------------------------------------------------------*/
	/*	Checking the maximum OL current in normal section:	OlPrm->Imax											*/
	/*------------------------------------------------------------------------------------------*/
	if( AmpLow->Imax > 0x7FFF )						/* Match the following value to the setting value of one side.			*/
	{
		AmpLowOLP->Ibsqrmax = MotOLP->conf.NormalArea.Ibsqrmax;		/* Instantaneous OL base current in the low speed section				*/
		AmpLowOLP->AlmLvlmax = AmpOLP->conf.NormalArea.AlmLvlmax;	/* Instantaneous OL level in low speed section					*/
		AmpLowOLP->WrnLvlmax = AmpOLP->conf.NormalArea.WrnLvlmax;	/* Instantaneous OL warning level in low speed section		*/
//			return;
	}
	else
	{
		Ix = AmpLow->Imax;

		Iysqr = (Ix * Ix) >> 16;						/* Iysqr: Maximum OL current^2					*/

		/*------------------------------------------------------------------------------------------*/
		/*	Instantaneous OL base current^2:	Ibsqr													*/
		/*------------------------------------------------------------------------------------------*/
		wk = (Ixsqr * AmpLow->Tmid) - (Iysqr * Iy);		/* Molecular term calculation of instantaneous OL base current 			*/
		Ibsqr = ( wk ) / ( AmpLow->Tmid - Iy);			/* Ibsqr: Instantaneous OL base current^2				*/

		/*------------------------------------------------------------------------------------------*/
		/* Instantaneous OL base current^2Check the numerator term of 													*/
		/*------------------------------------------------------------------------------------------*/
		if( wk > 0 )
		{
			/*--------------------------------------------------------------------------------------*/
			/* Instantaneous OL base current^2														*/
			/*--------------------------------------------------------------------------------------*/
			if( Ibsqr < 0 )									/* Match the following value to the setting value of one side.		*/
			{
				AmpLowOLP->Ibsqrmax = MotOLP->conf.NormalArea.Ibsqrmax;		/* Instantaneous OL base current 					*/
			}
			/*--------------------------------------------------------------------------------------*/
			/* Instantaneous OL base current^2Continuous OL base current^2 						*/
			/*--------------------------------------------------------------------------------------*/
			if( Ibsqr < AmpLowOLP->Ibsqrmid )					/* Match the following value to the continuous OL setting value.	*/
			{
				AmpLowOLP->Ibsqrmax  = AmpLowOLP->Ibsqrmid;		/* Instantaneous OL base current^2					*/
				AmpLowOLP->OlLvlmaxGain = AmpLowOLP->OlLvlmidGain;/* Instantaneous OL level gain				*/
			}
			/*--------------------------------------------------------------------------------------*/
			/* Instantaneous OL base current^2settings, instantaneous OL level, level gain calculation						*/
			/*--------------------------------------------------------------------------------------*/
			/*	Izsqr		: Molecular term of instantaneous OL detection alarm level(Maximum OL current^2 - Instantaneous base current^2)	*/
			/*	Ibsqrmax	: Instantaneous OL base current^2													*/
			/*	AlmLvlmax	: Instantaneous OL alarm detection level											*/
			/*	OlLvlmaxGain: Instantaneous OL detection level gain												*/
			/*--------------------------------------------------------------------------------------*/
			else
			{
				Izsqr = Iysqr - Ibsqr;
				AmpLowOLP->Ibsqrmax  = Ibsqr;
				AmpLowOLP->AlmLvlmax = MlibScalKxgain( Izsqr, (100*Iy), KPI_SCCYCLEMS, NULL, -30 );
				AmpLowOLP->OlLvlmaxGain = MlibScalKxgain( AmpOLP->conf.NormalArea.AlmLvlmax, 1, AmpLowOLP->AlmLvlmax, NULL, 24 );
				if( AmpLowOLP->AlmLvlmax >= 0x40000000 )
				{
//						ALMSetPramError( AlmMngr, pndef_olspdlvl.RegNumber);		/* A.040 : Parameter setting error			*/
					ret = FALSE;
				}
			}
		}

		/********************************************************************************************/
		/*	Continuous OL level in low speed section, instantaneous OL level, continuous warning level, instantaneous warning level setting*/
		/*		Set the detection level of the low speed section to the detection level of the normal section								*/
		/********************************************************************************************/
		AmpLowOLP->AlmLvlmid = AmpOLP->conf.NormalArea.AlmLvlmid;		/* Continuous OL level in low speed section					*/
		AmpLowOLP->AlmLvlmax = AmpOLP->conf.NormalArea.AlmLvlmax;		/* Instantaneous OL level in low speed section					*/
		AmpLowOLP->WrnLvlmid = AmpOLP->conf.NormalArea.WrnLvlmid;		/* Continuous OL warning level in low speed section		*/
		AmpLowOLP->WrnLvlmax = AmpOLP->conf.NormalArea.WrnLvlmax;		/* Instantaneous OL warning level in low speed section		*/
	}
	return	ret;
}
/*--------------------------------------------------------------------------------------------------*/
#endif
/* <S154> End */

/****************************************************************************************************/
/*																									*/
/*		Amplifier & motor overload check processing execution function													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Add up the value obtained by subtracting the base value from the square of the current command.									*/
/*			If the integrated value exceeds the overload alarm detection level, an overload alarm will be generated.						*/
/*			If the integrated value is negative, it is determined that the temperature rise has stopped, and the integrated value is limited to 0.			*/
/*			A warning occurs when the accumulated value reaches a value corresponding to the ratio determined by the system constant.			*/
/*																									*/
/****************************************************************************************************/
//static void	AdetExeCheckOLAmpMot( LONG AbsMotSpd, LONG Ixsqr, CHECK_OL *ol_ptr, ALMMAN_HANDLE *AlmMngr )			/* <S154> */
static void	AdetExeCheckOLAmpMot( LONG AbsMotSpd, LONG Ixsqr, CHECK_OL *ol_ptr, ALMMAN_HANDLE *AlmMngr, UCHAR Mode )	/* <S154> */
{
	CHECK_OLP	*CheckOLP;		/* <S154> */
	LONG	Sumx;
	LONG	Ixsqr_s;
	LONG	reduceLoad;
//	LONG	reduceLoad_s;
	LONG	ltmp;

	/* Local variable initialization */
	Ixsqr_s = Ixsqr >> 16;

/* <S154> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Overload calculation area selection																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( Mode == NORMAL_AREA )
	{
		CheckOLP = &(ol_ptr->conf.NormalArea);
	}
	else if( Mode == LOWSPD_AREA )
	{
		CheckOLP = &(ol_ptr->conf.LowSpdArea);
	}
	else if( Mode == LOCK_AREA )
	{
		CheckOLP = &(ol_ptr->conf.LowSpdArea);
	}
	else
	{
		CheckOLP = &(ol_ptr->conf.NormalArea);
	}
/* <S154> End */

#if (FLOAT_USE==TRUE)
//	reduceLoad = 16384 - (LONG)( (float)AbsMotSpd * ol_ptr->conf.CoolRateGain);		/* Self-running cooling rate	*/	/* <S154> */
	reduceLoad = 16384 - (LONG)( (float)AbsMotSpd * CheckOLP->CoolRateGain);		/* Self-running cooling rate	*/	/* <S154> */
#else
//	reduceLoad = 16384 - MlibMulgain( AbsMotSpd, ol_ptr->conf.CoolRateGain );		/* Self-running cooling rate	*/	/* <S154> */
	reduceLoad = 16384 - MlibMulgain( AbsMotSpd, CheckOLP->CoolRateGain );			/* Self-running cooling rate	*/	/* <S154> */
#endif /* FLOAT_USE */
//	reduceLoad_s = reduceLoad >> 14;

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
	ol_ptr->var.WrnInfo = 0x00;					/* Warning information reset					*/

	/********************************************************************************************/
	/*		Instantaneous overload check																*/
	/********************************************************************************************/
#if 1	/* <S154> */
#if (FLOAT_USE==TRUE)
	if( CheckOLP->CoolRateGain == 0 )
	{
		ltmp = (LONG)( (float)(Ixsqr_s - CheckOLP->Ibsqrmax) * CheckOLP->OlLvlmaxGain );
	}
	else
	{
		ltmp = (LONG)( (float)((((SHORT)Ixsqr_s * (SHORT)reduceLoad)>>14) - CheckOLP->Ibsqrmax) * CheckOLP->OlLvlmaxGain );
	}
#else
	if( CheckOLP->CoolRateGain == 0 )
	{
		ltmp = MlibMulgain( (Ixsqr_s - CheckOLP->Ibsqrmax), CheckOLP->OlLvlmaxGain );
	}
	else
	{
	//	ltmp = MlibMulgain( (((SHORT)Ixsqr_s * (SHORT)reduceLoad_s) - CheckOLP->Ibsqrmax), CheckOLP->OlLvlmaxGain );
		ltmp = MlibMulgain( ((((SHORT)Ixsqr_s * (SHORT)reduceLoad)>>14) - CheckOLP->Ibsqrmax), CheckOLP->OlLvlmaxGain );
	}
#endif /* FLOAT_USE */
#else	/* <S154> */
#if (FLOAT_USE==TRUE)
	if( ol_ptr->conf.CoolRateGain == 0 )
	{
		ltmp = (LONG)( (float)(Ixsqr_s - ol_ptr->conf.Ibsqrmax) * ol_ptr->conf.OlLvlmaxGain );
	}
	else
	{
		ltmp = (LONG)( (float)((((SHORT)Ixsqr_s * (SHORT)reduceLoad)>>14) - ol_ptr->conf.Ibsqrmax) * ol_ptr->conf.OlLvlmaxGain );
	}
#else
	if( ol_ptr->conf.CoolRateGain == 0 )
	{
		ltmp = MlibMulgain( (Ixsqr_s - ol_ptr->conf.Ibsqrmax), ol_ptr->conf.OlLvlmaxGain );
	}
	else
	{
	//	ltmp = MlibMulgain( (((SHORT)Ixsqr_s * (SHORT)reduceLoad_s) - ol_ptr->conf.Ibsqrmax), ol_ptr->conf.OlLvlmaxGain );
		ltmp = MlibMulgain( ((((SHORT)Ixsqr_s * (SHORT)reduceLoad)>>14) - ol_ptr->conf.Ibsqrmax), ol_ptr->conf.OlLvlmaxGain );
	}
#endif /* FLOAT_USE */
#endif	/* <S154> */
	Sumx = (LONG)( ol_ptr->var.SumImax + ltmp );

	/*------------------------------------------------------------------------------------------*/
//	if( Sumx >= ol_ptr->conf.AlmLvlmax )			/* Accumulated value >= Alarm level					*/	/* <S154> */
	if( Sumx >= CheckOLP->AlmLvlmax )				/* Accumulated value >= Alarm level					*/	/* <S154> */
	{
		ol_ptr->var.SumImax = Sumx;
		ALMSetServoAlarm( AlmMngr, ALM_OLF1 );		/* A.710 : Overload(Instantaneous maximum load)				*/
	}
//	if( Sumx >= ol_ptr->conf.WrnLvlmax )			/* Accumulated value >= Warning level 				*/	/* <S154> */
	if( Sumx >= CheckOLP->WrnLvlmax )				/* Accumulated value >= Warning level 				*/	/* <S154> */
	{
		ol_ptr->var.SumImax = Sumx;
		ol_ptr->var.WrnInfo |= 0x01;				/* Warning information setting						*/
	}
	else if( Sumx >= 0 )							/* Accumulated value >= 0								*/
	{
		ol_ptr->var.SumImax = Sumx;
	}
	else											/* Accumulated value < 0								*/
	{
		ol_ptr->var.SumImax = 0;
	}

	/********************************************************************************************/
	/*		Continuous overload check																*/
	/********************************************************************************************/
#if 1	/* <S154> */
#if (FLOAT_USE==TRUE)
	if( CheckOLP->CoolRateGain == 0 )
	{
		ltmp = (LONG)( (float)(Ixsqr_s - CheckOLP->Ibsqrmid) * CheckOLP->OlLvlmidGain );
	}
	else
	{
		ltmp = (LONG)( (float)((((SHORT)Ixsqr_s * (SHORT)reduceLoad)>>14) - CheckOLP->Ibsqrmid) * CheckOLP->OlLvlmidGain );
	}
#else
	if( CheckOLP->CoolRateGain == 0 )
	{
		ltmp = MlibMulgain( (Ixsqr_s - CheckOLP->Ibsqrmid), CheckOLP->OlLvlmidGain );
	}
	else
	{
	//	ltmp = MlibMulgain( ( ((SHORT)Ixsqr_s * (SHORT)reduceLoad_s) - CheckOLP->Ibsqrmid), CheckOLP->OlLvlmidGain );
		ltmp = MlibMulgain( ((((SHORT)Ixsqr_s * (SHORT)reduceLoad)>>14) - CheckOLP->Ibsqrmid), CheckOLP->OlLvlmidGain );
	}
#endif /* FLOAT_USE */
#else	/* <S154> */
#if (FLOAT_USE==TRUE)
	if( ol_ptr->conf.CoolRateGain == 0 )
	{
		ltmp = (LONG)( (float)(Ixsqr_s - ol_ptr->conf.Ibsqrmid) * ol_ptr->conf.OlLvlmidGain );
	}
	else
	{
		ltmp = (LONG)( (float)((((SHORT)Ixsqr_s * (SHORT)reduceLoad)>>14) - ol_ptr->conf.Ibsqrmid) * ol_ptr->conf.OlLvlmidGain );
	}
#else
	if( ol_ptr->conf.CoolRateGain == 0 )
	{
		ltmp = MlibMulgain( (Ixsqr_s - ol_ptr->conf.Ibsqrmid), ol_ptr->conf.OlLvlmidGain );
	}
	else
	{
	//	ltmp = MlibMulgain( ( ((SHORT)Ixsqr_s * (SHORT)reduceLoad_s) - ol_ptr->conf.Ibsqrmid), ol_ptr->conf.OlLvlmidGain );
		ltmp = MlibMulgain( ((((SHORT)Ixsqr_s * (SHORT)reduceLoad)>>14) - ol_ptr->conf.Ibsqrmid), ol_ptr->conf.OlLvlmidGain );
	}
#endif /* FLOAT_USE */
#endif	/* <S154> */
	Sumx = (LONG)( ol_ptr->var.SumImid + ltmp );

	/*------------------------------------------------------------------------------------------*/
//	if( Sumx >= ol_ptr->conf.AlmLvlmid )			/* Accumulated value >= Alarm level					*/	/* <S154> */
	if( Sumx >= CheckOLP->AlmLvlmid )				/* Accumulated value >= Alarm level					*/	/* <S154> */
	{
		ol_ptr->var.SumImid = Sumx;
		ALMSetServoAlarm( AlmMngr, ALM_OLF2 );		/* A.720 : Overload(Continuous maximum load)				*/
	}
//	if( Sumx >= ol_ptr->conf.WrnLvlmid )			/* Accumulated value >= Warning level				*/	/* <S154> */
	if( Sumx >= CheckOLP->WrnLvlmid )				/* Accumulated value >= Warning level				*/	/* <S154> */
	{
		ol_ptr->var.SumImid = Sumx;
		ol_ptr->var.WrnInfo |= 0x02;				/* Warning information setting						*/
	}
	else if( Sumx >= 0 )							/* Accumulated value >= 0								*/
	{
		ol_ptr->var.SumImid = Sumx;
	}
	else											/* Accumulated value < 0								*/
	{
		ol_ptr->var.SumImid = 0;
	}
}



/****************************************************************************************************/
/*																									*/
/*		DB overload check processing																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the DB power integrated value, and if it exceeds the overload level, set a DB overload alarm.			*/
/*			It also calculates the average DB power value.														*/
/*			DB overload due to temperature rise is also detected.													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckDbrakeOverLoad( ALMMAN_HANDLE *AlmMngr, CHECK_DBOL *DbOL, BOOL DbOn, LONG AbsMotSpd )
{
	LONG	x;
	LONG	DbPower;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculating DB power																				*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					 PowerMax * Speed * Speed														*/
/*		DbPower  = --------------------------- = Kdbpower * (Speed>>9) * (Speed>>9)					*/
/*					   SpdLevel * SpdLevel															*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	if( DbOn )
	{
		if( AbsMotSpd >= DbOL->conf.DbolSpdLevel )
		{
			DbPower = DbOL->conf.DbolPowMax;
		}
		else
		{
			x = (AbsMotSpd >> 9);
#if (FLOAT_USE==TRUE)
			DbPower = x * x * DbOL->conf.Kdbpower;
#else
			DbPower = MlibMulgainSx48( (x * x), DbOL->conf.Kdbpower );
#endif /* FLOAT_USE */
		}
	}
	else
	{
		DbPower = 0;
		DbOL->var.DbHeat = 0;							/* Due to temperature riseDBOverload integrated value clear	*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of DB power integrated value																		*/
/*--------------------------------------------------------------------------------------------------*/
	DbOL->var.DbolPowSum += (DbPower - DbOL->conf.DbolPowBase );
	/*----------------------------------------------------------------------------------------------*/
	/*	DBCountermeasures against overflow of power integration value																*/
	/*	⇒DBIf it continues to be turned by an external force in the stateDBAdded limit processing to prevent integrated power value from overflowing	*/
	/*		Maximum value accumulated in one sample from current motor constant(DbOL->conf.DbolPowMax)is				*/
	/*		Results of study18bit, set the limit level2^30only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.								*/
	/*----------------------------------------------------------------------------------------------*/
	if( DbOL->var.DbolPowSum > 0x40000000)
	{
		DbOL->var.DbolPowSum = 0x40000000;
	}
	if( DbOL->var.DbolPowSum < 0 )
	{
		DbOL->var.DbolPowSum = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		DB overload check																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( DbOL->var.DbolPowSum >= DbOL->conf.DbolAlmLevel )
	{
		/* A.730 : DB overload */
		ALMSetServoAlarm( AlmMngr, ALM_DBOL );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Un00B : Data for DB resistance power consumption(10secDB average value of power consumption)	CoutV.UnMeanDbPower				*/
/*		Un144 : Data for DB resistance power consumption(2msDB power consumption value)			CoutV.UnDBPower					*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	DbOL->var.UnDbPower = DbPower * DbOL->conf.KundbpowerCycle;
#else
	DbOL->var.UnDbPower = MlibMulgain( DbPower, DbOL->conf.KundbpowerCycle );
#endif /* FLOAT_USE */
	DbOL->var.DbPowerSum += DbPower;

	DbOL->var.DbMeanCycle++;
	if( DbOL->var.DbMeanCycle >= KPI_SCANC_MS( 10000 ) )
	{
#if (FLOAT_USE==TRUE)
		DbOL->var.UnMeanDbPower = DbOL->var.DbPowerSum * DbOL->conf.Kundbpower;
#else
		DbOL->var.UnMeanDbPower = MlibMulgain( DbOL->var.DbPowerSum, DbOL->conf.Kundbpower );
#endif /* FLOAT_USE */
		DbOL->var.DbPowerSum  = 0;
		DbOL->var.DbMeanCycle = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		DB overload warning/alarm detection processing due to temperature rise										*/
/*--------------------------------------------------------------------------------------------------*/
	if ( DbOL->conf.DbOHeatCheck )
	{
		DbOL->var.DbHeat += (DbPower - DbOL->conf.DbolPowBase );
		if ( DbOL->var.DbHeat >= DbOL->conf.DbOHeatAlmLvl )
		{
			/* A.921 : DB overload warning cleared due to temperature rise */
			ALMClearWarning( AlmMngr, WRN_DBOL_LVL );
			/* Warning detection flag clear */
			DbOL->var.DbOHeatWarn = FALSE;
			/* A.731 : DB overload set due to temperature rise */
			ALMSetServoAlarm( AlmMngr, ALM_DBOL_LVL );
		}
		else if ( (DbOL->var.DbHeat >= DbOL->conf.DbOHeatWrnLvl)
				&& (DbOL->var.DbOHeatWarn == FALSE) )
		{
			/* A.921 : DB overload warning set due to temperature rise */
			ALMSetServoAlarm( AlmMngr, WRN_DBOL_LVL );
			/* Warning detection flag set */
			DbOL->var.DbOHeatWarn = TRUE;
		}
	}

	if ( (DbOL->var.DbolPowSum == 0) && (DbOL->var.DbOHeatWarn == TRUE) )
	{
		/* A.921 : DB overload warning cleared due to temperature rise */
		ALMClearWarning( AlmMngr, WRN_DBOL_LVL );
		/* Warning detection flag clear */
		DbOL->var.DbOHeatWarn = FALSE;
		/* Clear integrated value */
		DbOL->var.DbHeat = 0;
	}

}

/****************************************************************************************************/
/*																									*/
/*		Speed ripple compensation information mismatch warning check processing											*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : EEPROMmust be saved in	*/
/*          If they do not match, detect speed ripple compensation information mismatch warning						*/
/*																									*/
/*																									*/
/****************************************************************************************************//*<S1FD>*/
void	AdetCheckRippleWrn( ALMMAN_HANDLE *AlmMngr, BOOL RippleWrnState )
{
	/* Speed ripple compensation information mismatch warning status acquisition */
	if( RippleWrnState == FALSE )
	{
		ALMSetServoAlarm( AlmMngr, WRN_RIPPLE );	/* A.942 : Speed ripple compensation information mismatch detection */
	}
	else
	{
		ALMClearWarning( AlmMngr, WRN_RIPPLE );		/* A.942 : Speed ripple compensation information mismatch release */
	}
}

#if	0	/*<S1F6>*/
/****************************************************************************************************/
/*																									*/
/*		Input signal timing abnormality check processing															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : BBON1LT/BBON2LTWhen the time difference between when the signal becomes active is greater than the parameter setting value		*/
/*			Set as an alarm. This makes an abnormality in the duplex signal (hardware failure, disconnection of input signal) an alarm.		*/
/*			This is a procedure for detection.Depending on the presence or absence of the safety option card,			*/
/*			Alarm code (with card)/None=A.Eb2/A.Eb1) and setting values used for alarm detection					*/
/*			(Card present/None= PnE4C/Pn620) Change.												*/
/*																									*/
/****************************************************************************************************/
BOOL	AdetCheckBbonSignal( CHECK_HWBB_TIMING *pHwbbTmg, BOOL LackOneBbon )
{
	BOOL err;

	err = FALSE;
	/*--------------------------------------------------------------------------------------------------*/
	/*		BBONSignal input timing abnormality check process														*/
	/*--------------------------------------------------------------------------------------------------*/
		if( LackOneBbon )
		{
			pHwbbTmg->var.BbonTimeDiffCnt += KPI_SCCYCLEMS;
			if( pHwbbTmg->var.BbonTimeDiffCnt > pHwbbTmg->conf.MaxBbonTimeDiff )
			{
				/* Clamp the upper limit at the set value */
				pHwbbTmg->var.BbonTimeDiffCnt = pHwbbTmg->conf.MaxBbonTimeDiff;

				/* A.Eb1 : Excessive stop signal input time difference(No card) */
				err = TRUE;
			}
		}
		else		/* <S0E5> */
		{
			pHwbbTmg->var.BbonTimeDiffCnt = 0;
		}
		return err;
}
#endif	/*<S1F6>*/

#if 0	/*<S1F5>*/
/****************************************************************************************************/
/*																									*/
/*		Safety function Internal signal abnormality detection processing (Test Pulse Diagnostic)					<S0F1>		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Internal signal on amplifier side(DPIN1/DPIN2Signal)is not a pulse signal, detect an abnormality					*/
/*			HWBBDo not detect abnormality if the status is the same.															*/
/*																									*/
/****************************************************************************************************/
BOOL	AdetTestPulseDiagnostic( BOOL DpInState, BOOL DpInLastState, BOOL CheckEnable, BOOL Bbon )	/*<S1DC>*/
{
	BOOL err;

	err = FALSE;

#if((PRODUCT_SERIES_SEVEN  ==  TRUE) && ( CSW_BOARD_TYPE == SGD7S_TYPE ) ) || ( CSW_HW_HWBB_SUPPORT == TRUE)		/* SGD7WinA.EC8,A.EC9Correct false detection<S105> */
/*--------------------------------------------------------------------------------------------------*/
/*		DPINSignal check processing (BBON=OFF:HWBB(Do not check status)									*/
/*--------------------------------------------------------------------------------------------------*/
	if( CheckEnable )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	DPINSignal is4msIf the pulse signal is not a cycle, it is abnormal											*/
		/*------------------------------------------------------------------------------------------*/
		if( !(DpInState^DpInLastState) )
		{
			err = TRUE;
		}
	}
/* <S1DC> >>>>> */
	else if( Bbon )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	BBON=OFF(HIGH),DPINSignal isHIGHIf not, it is abnormal										*/
		/*------------------------------------------------------------------------------------------*/
		if( DpInState )
		{
			err = TRUE;
		}
	}
/* <<<<< <S1DC> */
#endif

	return err;
}
#endif	/*<S1F5>*/

#if 0
/****************************************************************************************************/
/*																									*/
/*		Gate drive error check processing																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : HardwiseHWBBDetection circuit exists and softwareHWBBFor models with this function disabled,		*/
/*			HWBBAn alarm is generated when a condition is detected. Due to noise etc.BBONWhen the signal becomes active and hard		*/
/*			This takes into consideration the case where the wired base block state occurs, and can also be used in softwareHWBBDetection		*/
/*			Let the process run,HWBBstate is detected due to noise etc., "HBB" is not displayed and the gate drive is abnormal		*/
/*			Originally, it was done in hardwareHWBBA measure has been taken to prevent the status from becoming the status		*/
/*			Also keep interlocks in software.											*/
/*																									*/
/****************************************************************************************************/
BOOL	AdetCheckGateDriveDefect( DET_HWBB_STS *pDetHwbbSts )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Gate drive error check processing																*/
/*--------------------------------------------------------------------------------------------------*/
	return pDetHwbbSts->GateDrvDefect;
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Motor runaway check processing																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Motor line(U,V,W phase)Detects runaway caused by incorrect wiring.									*/
/*			Alarm detection only looks at the first start of motor movement when the power is turned on.							*/
/*																									*/
/*			Detection conditions are																			*/
/*			<For rotating type>																		*/
/*				Runaway alarm not masked														*/
/*			  ∩After power-on, the servo is turned on and the motor starts moving0.5msec							*/
/*			  ∩SpeedFBThe torque command has the opposite polarity															*/
/*			  ∩Previous torque command and current torque command have the same polarity												*/
/*			  ∩Torque command≧Runaway detection torque															*/
/*			  ∩SpeedFB≧Runaway detection speed																*/
/*			  ∩torque command is near zero0																	*/
/*			  ∩Speed this time≧Previous speed(Accelerating)															*/
/*			  ∩Speed this time≧Maximum speed that occurred in the past													*/
/*			is set in the state of20msSet as an alarm if it occurs continuously.											*/
/*																									*/
/*			<For linear type>																		*/
/*				SpeedFBThe torque command has the opposite polarity															*/
/*			  ∩Previous torque command and current torque command have the same polarity												*/
/*			  ∩Torque command≧Runaway detection torque															*/
/*			  ∩SpeedFB≧Runaway detection speed																*/
/*			  ∩torque command is near zero0																	*/
/*			  ∩Speed this time≧Previous speed(Accelerating)															*/
/*			  ∩Speed this time≧Maximum speed that occurred in the past													*/
/*			is set in the state of14msSet as an alarm if it occurs continuously.											*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckMotorOverrun( ALMMAN_HANDLE *AlmMngr, CHECK_OVRRUN *OvrRun,
								 LONG MotSpd, LONG TrqRefMon, LONG BaseEnableSts, BOOL MotTypeLinear )
{
	LONG	Condition0;
	LONG	Condition1;
	LONG	DetectTime;
	LONG	AbsTrqRefMon;
	LONG	AbsMotSpd;

	AbsTrqRefMon = MlibLABS( TrqRefMon );
	AbsMotSpd = MlibLABS( MotSpd );

	if(MotTypeLinear != FALSE)
	{ /* Linear type */
		Condition0 = TRUE;
		Condition1 = FALSE;
		DetectTime = KPI_SCANC_MS( 14 );
		OvrRun->OvrPeakMotSpd = 0;				/* Maximum speed always clear */
	}
	else
	{ /* Rotation type */
		/* BaseEnable && From power on0.5sec */
		if( (BaseEnableSts == 0) && (OvrRun->OvrChkCnt < KPI_SCANC_MS( 500 )) )
		{
			Condition0 = TRUE;
		}
		else
		{
			Condition0 = FALSE;
		}
		/* Speed ≧ OSOf speed1/10? */
		if( (OvrRun->OvrChkCnt != 0) || (AbsMotSpd >= MOTSPD_OS_10) )
		{
			Condition1 = TRUE;
		}
		else
		{
			Condition1 = FALSE;
		}
		DetectTime = KPI_SCANC_MS( 20 );
	}

	if( Condition0 )
	{
		if( Condition1 )
		{
			OvrRun->OvrChkCnt++;
		}
	/*----------------------------------------------------------------------------------------------*/
		if( ((MotSpd ^ TrqRefMon) < 0)						/* SpeedFBThe polarity of the torque command has the opposite?	*/
			&& ((TrqRefMon ^ OvrRun->OvrTrqRef) >= 0)		/* The previous torque and this torque are the same polarity?	*/
			&& (AbsTrqRefMon >= OvrRun->OvrTrqLevel)		/* Torque command ≧ Runaway detection torque?	*/
			&& (AbsMotSpd >= OvrRun->OvrSpdLevel)			/* SpeedFB     ≧ Runaway detection speed?		*/
			&& (TrqRefMon != 0) )							/* torque command is near zero0?				*/
		{
			if( AbsMotSpd >= OvrRun->OvrAbsMotSpd )
			{ /* Speed this time ≧ Previous speed? */
				if( AbsMotSpd >= OvrRun->OvrPeakMotSpd )
				{ /* Speed this time ≧ Maximum speed in the past? */
					if( OvrRun->OvrAlmCnt >= DetectTime )
					{ /* Detection time elapsed? */
						/* A.C10 : Runaway prevention detection */
						ALMSetServoAlarm( AlmMngr, ALM_RWF );
					}
					else
					{
						/* Counting up runaway detection counter */
						OvrRun->OvrAlmCnt++;
					}
				}
				else
				{
					/* Clearing runaway detection counter */
					OvrRun->OvrAlmCnt = 0;
				}
			}
			else
			{
				if( OvrRun->OvrAbsMotSpd > OvrRun->OvrPeakMotSpd )
				{ /* Previous speed ≧ Maximum speed? */
					/* Maximum speed update */
					OvrRun->OvrPeakMotSpd = OvrRun->OvrAbsMotSpd;
				}
				/* Clearing runaway detection counter */
				OvrRun->OvrAlmCnt = 0;
			}
		}
		else
		{
			/* Clearing runaway detection counter */
			OvrRun->OvrAlmCnt = 0;
		}
	}

	/* Update the previous torque */
	OvrRun->OvrTrqRef = TrqRefMon;
	/* Update of previous motor speed */
	OvrRun->OvrAbsMotSpd = AbsMotSpd;
}



/****************************************************************************************************/
/*																									*/
/*		Excessive position deviation check processing when servo is on														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitor the position deviation when the servo is turned on in position control mode.									*/
/*																									*/
/*			1) If the position deviation exceeds the excessive position deviation alarm level, an alarm is generated.				*/
/*			2) If the position deviation exceeds the excessive position deviation warning level, a warning will be issued.			*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckSvonPerrOverFlow( ALMMAN_HANDLE *AlmMngr, CHECK_OVRPOSERR *OvrPosErr, LONG PositionError )
{
	LONG	AbsPosError;

	/* Local variable initialization */
	AbsPosError = MlibLABS( PositionError );

#if 0
/*--------------------------------------------------------------------------------------------------*/
/*		Control mode check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( pSeqCtrl->CtrlMode.b.cm != CTRLMODE_POS )
	{
		ALMClearWarning( AlmMngr, WRN_BEOF );			/* A.901 : Excessive position deviation when servo is on			*/
		return;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Base enable request check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( (pSeqCtrl->DbBeReq == FALSE) || (pSeqCtrl->BeReqSeqOut == TRUE) )
	{ /* No base enable request || Servo-on completion */
		ALMClearWarning( AlmMngr, WRN_BEOF );			/* A.901 : Excessive position deviation when servo is on			*/
		return;
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation check(Base enable request exists && If servo is not turned on)							*/
/*--------------------------------------------------------------------------------------------------*/
	if( AbsPosError >= OvrPosErr->SvonPerAlmLevel )
	{
		ALMSetServoAlarm( AlmMngr, ALM_BEOF );			/* A.D01 : Excessive position deviation when servo is on			*/
	}
	/*----------------------------------------------------------------------------------------------*/
	if( AbsPosError >= OvrPosErr->SvonPerWrnLevel )
	{
		ALMSetServoAlarm( AlmMngr, WRN_BEOF );			/* A.901 : Excessive position deviation when servo is on			*/
	}
	else
	{
		ALMClearWarning( AlmMngr, WRN_BEOF );			/* A.901 : Excessive position deviation when servo is on			*/
	}
}



/****************************************************************************************************/
/*																									*/
/*		Excessive position deviation check process																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Monitor position deviation in position control mode.													*/
/*																									*/
/*			1) If the position deviation exceeds the excessive position deviation alarm level, an alarm is generated.				*/
/*			2) If the position deviation exceeds the excessive position deviation warning level, a warning will be issued.			*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	AdetCheckPerrOverFlow( ALMMAN_HANDLE *AlmMngr, CHECK_OVRPOSERR *OvrPosErr,
										BOOL PosCtrlMode, LONG PositionError, BOOL SvonSpdLmt )
{
	LONG	AbsPosError;

	/* Local variable initialization */
	AbsPosError = MlibLABS( PositionError );

/*--------------------------------------------------------------------------------------------------*/
/*		Control mode check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( FALSE == PosCtrlMode )
	{
		/* A.900 : Excessive position deviation warning */
		ALMClearWarning( AlmMngr, WRN_OF );
		return;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( AbsPosError >= OvrPosErr->PerAlmLevel )
	{
		if( SvonSpdLmt )
		{
			/* A.D02 : Excessive speed limit position deviation when servo is on */
			ALMSetServoAlarm( AlmMngr, ALM_BEVLMTOF );
		}
		else
		{
			/* A.D00 : Excessive position deviation */
			ALMSetServoAlarm( AlmMngr, ALM_OF );
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	if( AbsPosError >= OvrPosErr->PerWrnLevel )
	{
		/* A.900 : Excessive position deviation warning */
		ALMSetServoAlarm( AlmMngr, WRN_OF );
	}
	else
	{
		/* A.900 : Excessive position deviation warning */
		ALMClearWarning( AlmMngr, WRN_OF );
	}
}

#if	(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)
/****************************************************************************************************/
/*																									*/
/*		Motor load positional deviation excessive check process														*/
/*																									*/
/****************************************************************************************************//* <S04D> */
static BOOL LpxCheckSemiFullPerrOverFlow(CHECK_FENCPOSERR *ChkFposErr,
                                         	   BOOL PosCtrlMode, LONG MencPos, LONG FencPos)
{
	LONG	err_d10;
	LONG	wk;
	LONG	dMotPos, dFencPos;
	LONG	MtPGSubUnit, ExtPGSubUnit;

	err_d10 = FALSE;

	dMotPos = MencPos - ChkFposErr->MencPosLast;
	dFencPos = FencPos - ChkFposErr->FencPosLast;
	ChkFposErr->MencPosLast = MencPos;
	ChkFposErr->FencPosLast = FencPos;

	if( FALSE == PosCtrlMode )
	{/* Other than position control mode	*/
		MlibResetLongMemory( &ChkFposErr->sfperra, sizeof(ChkFposErr->sfperra)/4 );
		KlibRstLongTimer(&ChkFposErr->FSOFAlmCnt);
		return err_d10;
	}

	MtPGSubUnit = MlibEgearRvscnv( dMotPos, &ChkFposErr->Egear, &ChkFposErr->MtPGRem );		/*  Real position	[Command unit]	*/
	ExtPGSubUnit = MlibEgearRvscnv( dFencPos, &ChkFposErr->Egear, &ChkFposErr->ExtPGRem );	/*  Real position	[Command unit]	*/
	MlibPerrcala( ExtPGSubUnit, MtPGSubUnit, &ChkFposErr->sfgear, &ChkFposErr->sfperra );

	if ( MlibAbs32( FencPos- ChkFposErr->FencPosOneTurn ) >= ChkFposErr->fcpgpls )
	{
		ChkFposErr->FencPosOneTurn = FencPos;

		/* Motor1Decrease rate of deviation during rotation(Pn52A)reduce by minute	*/
		if ( ChkFposErr->fpgcutrate == 100 )
		{
			MlibResetLongMemory( &ChkFposErr->sfperra, sizeof(ChkFposErr->sfperra)/4 );	/* Position management variable initialization	*/
		}
		else
		{
#if (FLOAT_USE==TRUE)
			wk = ChkFposErr->sfperra.per32s * ChkFposErr->fpgcutratelv;
#else
			wk = MlibMulgain32( ChkFposErr->sfperra.per32s, ChkFposErr->fpgcutratelv );
#endif /* FLOAT_USE */
			MlibPerrcala( -wk, 0, &ChkFposErr->sfgear, &ChkFposErr->sfperra );
		}
	}

	if ( ChkFposErr->fpgoverLevel != 0 )
	{/* Full-closed control && Pn51C:Pn51B != 0	*/
		if ( (ChkFposErr->sfperra.per64[1] > 0) ||
			 (ChkFposErr->sfperra.per64[1] < -1) ||
			 (MlibAbs32(ChkFposErr->sfperra.per64[0]) >= (LONG)ChkFposErr->fpgoverLevel) )
		{
			/* 20ms Alarm on continuation	*/
			if ( KlibGetLongTimerMs(ChkFposErr->FSOFAlmCnt) >= 20 )
			{	/* A.D10 : Excessive deviation between motor and load position		*/
//				KPI_ALARMSET( ALM_FSOF );
				err_d10 = TRUE;
			}
		}
		else
		{
			KlibRstLongTimer(&ChkFposErr->FSOFAlmCnt);
		}
	}
	else
	{
		KlibRstLongTimer(&ChkFposErr->FSOFAlmCnt);
	}

	return err_d10;
}

/****************************************************************************************************/
/*																									*/
/*		Motor load positional deviation excessive check process														*/
/*																									*/
/****************************************************************************************************//* <S04D> */
void AdetCheckFullcEncoderAlarm(CHECK_FENCPOSERR *ChkFposErr, MENCV *FencV,
                            	BOOL PosCtrlMode, LONG MencPos, LONG FencPos, ALMMAN_HANDLE *AlmMngr )
{
	UCHAR AlmCode;

	/*----------------------------------------------------------------------*/
	/*		Alarm code check (BIT 0~3)								*/
	/*----------------------------------------------------------------------*/
	AlmCode = FencGetFencAlarm(FencV);
	switch(AlmCode & 0x0F)
	{
	case 0x01:
		if(FencV->SerialConvType == SCONVTYPE_ENDAT_ABS)
		{	/* A.8A0 : FPG scale error */
			ALMSetServoAlarm(AlmMngr, ALM_FPGFSCALE);
		}
		break;
	case 0x02:
		/* A.8A1 : FPG module error	*/
		ALMSetServoAlarm(AlmMngr, ALM_FPGMODULE);
		break;
	case 0x03:
		if(FencV->SerialConvType == SCONVTYPE_ENDAT_ABS)
		{	/* A.8A0 : FPG scale error */
			ALMSetServoAlarm(AlmMngr, ALM_FPGFSCALE);
		}
		/* A.8A1 : FPG module error	*/
		ALMSetServoAlarm(AlmMngr, ALM_FPGMODULE);
		break;
	case 0x08:
		if(FencV->AbsoEncoder != FALSE)
		{	/* A.8A3 : FPG position error(Absolute) */
			ALMSetServoAlarm(AlmMngr, ALM_FPGPOS);
		}
		else
		{	/* A.8A2 : FPG sensor error(Increment) */
			ALMSetServoAlarm(AlmMngr, ALM_FPGSEN);
		}
		break;
	case 0x09:
		if(FencV->AbsoEncoder != FALSE)
		{
			if(FencV->SerialConvType == SCONVTYPE_ENDAT_ABS)
			{	/* A.8A0 : FPG scale error */
				ALMSetServoAlarm(AlmMngr, ALM_FPGFSCALE);
			}
			/* A.8A3 : FPG position error(Absolute) */
			ALMSetServoAlarm(AlmMngr, ALM_FPGPOS);
		}
		else
		{	 /* A.8A2 : FPG sensor error(Increment) */
			ALMSetServoAlarm(AlmMngr, ALM_FPGSEN);
		}
		break;
	case 0x0A:
		/* A.8A1 : FPG module error */
		ALMSetServoAlarm(AlmMngr, ALM_FPGMODULE);
		if(FencV->AbsoEncoder != FALSE)
		{	/* A.8A3 : FPG position error(Absolute) */
			ALMSetServoAlarm(AlmMngr, ALM_FPGPOS);
		}
		else
		{	/* A.8A2 : FPG sensor error(Increment) */
			ALMSetServoAlarm(AlmMngr, ALM_FPGSEN);
		}
		break;
	case 0x0B:
		/* A.8A1 : FPG module error	*/
		ALMSetServoAlarm(AlmMngr, ALM_FPGMODULE);
		if(FencV->AbsoEncoder != FALSE)
		{
			if(FencV->SerialConvType == SCONVTYPE_ENDAT_ABS)
			{	/* A.8A0 : FPG scale error */
				ALMSetServoAlarm(AlmMngr, ALM_FPGFSCALE);
			}
			/* A.8A3 : FPG position error(Absolute)	*/
			ALMSetServoAlarm(AlmMngr, ALM_FPGPOS);
		}
		else
		{	/* A.8A2 : FPG sensor error(Increment) */
			ALMSetServoAlarm(AlmMngr, ALM_FPGSEN);
		}
		break;
	default:
		break; /* No alarm	*/
	}

	/*----------------------------------------------------------------------*/
	/*		Alarm code check (BIT 4~7)								*/
	/*----------------------------------------------------------------------*/
	if(0 != (AlmCode & 0x10))
	{	/* A.8A5 : FPG overspeed error */
		ALMSetServoAlarm(AlmMngr, ALM_FPGOVSPD);
	}
	if(0 != (AlmCode & 0x20))
	{	/* A.8A6 : FPG overheat error */
		ALMSetServoAlarm(AlmMngr, ALM_FPGOVHEAT);
	}

	/*----------------------------------------------------------------------*/
	/*		Motor load positional deviation excessive check								*/
	/*----------------------------------------------------------------------*/
	if(FALSE != LpxCheckSemiFullPerrOverFlow(ChkFposErr, PosCtrlMode, MencPos, FencPos))
	{	/* A.D10 : Excessive deviation between motor and load position */
		ALMSetServoAlarm(AlmMngr, ALM_FSOF);
	}

}


#endif	// (CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)



/***************************************** end of file **********************************************/

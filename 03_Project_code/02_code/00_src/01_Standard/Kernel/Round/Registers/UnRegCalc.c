/****************************************************************************
  Description   : Callback functions for Monitor Register
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corp., Copyright (c) 2010
                  All Rights Reserved

  Project       : INGRAM

  Create Date   : 2010-10-20

****************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "PrmAccessIf.h"
#include "HwDeviceIf.h"
#include "JL086ServoIpHw.h"
#include "KLib.h"
/* ---futami---> */
#include "OverheatManager.h"
/* <---futami--- */
#include "KnlApi.h"

/* <S056>	Start */
#include "HwDeviceIf.h"
#if (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE)	/* AnalogIF */
#include "MtCtrl.h"
#include"MtApi.h"
extern 	MCI_CMD		cmd;
#else
//#include "Anlg.h"	/* <S0D0> Delete */
#include "ScanN.h"	/* <S0D0> :Anlg.hteethScanN.hinincludeso I will comment it out.*/
#endif

extern MSGIF	SrlMsg[SIF_N_CH];			/* Serial Message I/F for SigmaMsg/LcdOp/Memobus	*/
/* <S056>	End */

/****************************************************************************
0x000	Motor rotation/Movement speed	SpdFbk	min-1, mm/s
****************************************************************************/
PRM_RSLT UnReadMotorSpeed(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG    val ;

	/* Tentatively commented out101227_kira ScanC */
	//val = MlibMulgainSx48(Axis->pMonData->UnMotSpd, (Axis->prm_hndl_ptr->iprm_ptr)->Kmonspd);
	val = Axis->UnMonitor->UnMotSpd;//kiraaddition

	*pValue = MlibLimitul(val, 0x00007FFF, 0xFFFF8000);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x001	Speed command	SpdRef	min-1, mm/s
****************************************************************************/
PRM_RSLT UnReadVelocityRef(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG    val ;

//	val = MlibMulgainSx48(Axis->pMonData->UnSpdRef, (Axis->prm_hndl_ptr->iprm_ptr)->Kmonspd);
#if (FLOAT_USE==TRUE)
	val = Axis->BaseControls->CtrlCmdMngr.SpdRefo * (Axis->UniPrms->Bprm)->Kmonspd;
#else
	val = MlibMulgainSx48(Axis->BaseControls->CtrlCmdMngr.SpdRefo, (Axis->UniPrms->Bprm)->Kmonspd);
#endif /* FLOAT_USE */
	*pValue = MlibLimitul(val, 0x00007FFF, 0xFFFF8000);
	return PRM_RSLT_SUCCESS;
}
PRM_RSLT UnReadSpeedFeedForward(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG    val ;

//	val = MlibMulgainSx48(Axis->pMonData->UnSpdRef, (Axis->prm_hndl_ptr->iprm_ptr)->Kmonspd);
#if (FLOAT_USE==TRUE)
	val = Axis->BaseControls->CtrlCmdMngr.SpdFFCx * (Axis->UniPrms->Bprm)->Kmonspd;
#else
	val = MlibMulgainSx48(Axis->BaseControls->CtrlCmdMngr.SpdFFCx, (Axis->UniPrms->Bprm)->Kmonspd);
#endif /* FLOAT_USE */
	*pValue = MlibLimitul(val, 0x00007FFF, 0xFFFF8000);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x002	Internal torque/Thrust command	TrqRef	%
****************************************************************************/
PRM_RSLT UnReadTorqueRef(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	/* Tentatively commented out101227_kira ScanC */
	//*pValue = MlibMulgain(Axis->pMonData->UnTrqRef, (Axis->prm_hndl_ptr->iprm_ptr)->Kmontrq);
	*pValue = Axis->UnMonitor->UnTrqRef;//kiraaddition
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x003	Electrical angle1(32Bit10Hex display)	MotAngPls	pulse
****************************************************************************/
PRM_RSLT UnReadMotAngle(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnMotAngle;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x005	Electrical angle2(UPhase0deg)	MotAngDeg	deg
****************************************************************************/
PRM_RSLT UnReadMotPhase(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnMotPhase;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x006	Input signal monitor	InSignal
****************************************************************************/
#if 0
PRM_RSLT UnReadDigitalInputSignals(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	HWSIN sig;
	sig = HALdrv_ReadInputSignals( );
	*pValue = (LONG)sig.dw;
	return PRM_RSLT_SUCCESS;
}
#else
PRM_RSLT UnReadDigitalInputSignals(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = HALdrv_ReadMonInputSignals( Axis->AxisNo );
	return PRM_RSLT_SUCCESS;
}
#endif

/****************************************************************************
0x007	Output signal monitor	OutSignal
****************************************************************************/
#if 0
PRM_RSLT UnReadDigitalOutputSignals(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	HWSOUT sig;
	sig = HALdrv_ReadOuputSignals();
	*pValue = (LONG)sig.dw;
	return PRM_RSLT_SUCCESS;
}
#else
PRM_RSLT UnReadDigitalOutputSignals(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	*pValue = HALdrv_ReadMonOutputSignals( Axis->AxisNo );	/* <S1D7>:Un006Specification change */
	*pValue = HALdrv_ReadMonAllOutputSignals( );			/* <S1D7>:Un006Specification change */
	return PRM_RSLT_SUCCESS;
}
#endif

/****************************************************************************
0x008	Input command pulse speed	PosRefSpd	min-1, mm/s
****************************************************************************/
PRM_RSLT UnReadCmdSpeedCounter(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnPcmd64ms;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x00B	Cumulative load rate	MotLoad %
****************************************************************************/
PRM_RSLT UnReadIsqrSum(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnIsqrSum;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x00C	Regeneration load rate	RegLoad	%
****************************************************************************/
PRM_RSLT UnReadRegenePower(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	*pValue = Axis->UnMonitor->UnMeanRgPower;
	*pValue = Axis->PowerManager->RegeneCtrl.UnMeanRgPower; /*<S11E>*/
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x00D	DB resistance power consumption	DbrxPower	%
****************************************************************************/
PRM_RSLT UnReadDBPower(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnMeanDbPower;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x010	Motor encoder position	PfbkCounter	Encoder pulse unit
****************************************************************************/
PRM_RSLT UnReadMotorEncoderPosition(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//	*pValue = Axis->BaseCtrlOut->MotPos;
	*pValue = Axis->UnMonitor->UnPfbkCntr;
	if( Cmd == PRM_ACCCMD_WRITE )		/* <S144> */
	{/* reset monitor value */
		Axis->UnMonitor->UnPfbkCntr = 0;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x012	Fully closed encoder position	FposCounter	Fulque encoder pulse unit
****************************************************************************/
PRM_RSLT UnReadFullcEncoderPosition(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (Axis->UniPrms->Bprm->FencUse != FALSE) ? Axis->BaseCtrlOut->FencPos : 0;
	if( Cmd == PRM_ACCCMD_WRITE )		/* <S144> */
	{/* reset monitor value */
		if( Axis->UniPrms->Bprm->FencUse != FALSE )
		{
			Axis->BaseCtrlOut->FencPos = 0;
		}
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x016	Total operating time	PowerOnTime	100ms
****************************************************************************/
PRM_RSLT UnReadTimeStamp(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = KlibGetTimeStamp();
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x01A	Hall sensor information
****************************************************************************/
PRM_RSLT UnReadPoleSensorInfo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
	*pValue = (Axis->SvAsicRegs->AsicMicroReg)->MREG_POLEINFO;
#endif
	return PRM_RSLT_SUCCESS;
}

/* <S056>	Start */
/****************************************************************************
0x01B	Feedback position[Command unit]
****************************************************************************/
PRM_RSLT UnReadPfbkCntr(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnPfbkCntr2;
	if( Cmd == PRM_ACCCMD_WRITE )		/* <S144> */
	{/* reset monitor value */
		Axis->UnMonitor->UnPfbkCntr2 = 0;
		Axis->UnMonitor->RemCntr2    = 0;
	}
	return PRM_RSLT_SUCCESS;
}

/* <S056>	End */


/****************************************************************************
0x01D	Effective gain monitor
****************************************************************************/
PRM_RSLT UnReadActGain(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->BaseControls->GainChange.ActGain;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x01E	Safety input/output signal monitor
****************************************************************************/
PRM_RSLT UnReadSafetySignals(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG hwbb;

	if( Axis->OptCardHdl->SftOpt.conf.OpInfo.f.Connect )	/* <S0C1> */
	{
			/* With safety option */
		*pValue = 0;
	}
	else
	{
			/* When there is no safety option */
		hwbb = Axis->IoSignals->DetHwbbSts.Bbon1;
		hwbb |= (Axis->IoSignals->DetHwbbSts.Bbon2 << 1);
		*pValue = hwbb ^ 0x03;
	}

	return PRM_RSLT_SUCCESS;
}

/* <S154> Start */
/*****************************************************************************
  Description:	0x050 : Built-inFANLifetime remaining rate
------------------------------------------------------------------------------
  Parameters:
	Cmd;			PRM_ACCCMD
	ArrayIdx;		Array Index
	AXIS_HANDLE;	axis Handle
	pValue;			value
  Return:
	PRM_RSLT;		PRM_RSLT_SUCCESS:Normal end,PRM_RSLT_CONTINUE: Normal end Continuing(FnFor)
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
PRM_RSLT UnReadFanLifeTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
 #if ( CSW_SVCF_LIFEPRED_USE == TRUE )
    *pValue =  (LONG)Axis->hLifeTime->FanLifeTimeHdl.var.FanSurvivor ;
#else
    *pValue =  0;
#endif
	return PRM_RSLT_SUCCESS;
}
/*****************************************************************************
  Description:	0x051 : Capacitor life remaining rate
------------------------------------------------------------------------------
  Parameters:
	Cmd;			PRM_ACCCMD
	ArrayIdx;		Array Index
	AXIS_HANDLE;	axis Handle
	pValue;			value
  Return:
	PRM_RSLT;		PRM_RSLT_SUCCESS:Normal end,PRM_RSLT_CONTINUE: Normal end Continuing(FnFor)
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
PRM_RSLT UnReadCapaLifeTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
	*pValue =  (LONG)Axis->hLifeTime->CapaLifeTimeHdl.var.CapaSurvivor ;
#else
	*pValue =  0;
#endif
	return PRM_RSLT_SUCCESS;
}

/*****************************************************************************
  Description:	0x052 : Bump protection circuit life remaining rate
------------------------------------------------------------------------------
  Parameters:
	Cmd;			PRM_ACCCMD
	ArrayIdx;		Array Index
	AXIS_HANDLE;	axis Handle
	pValue;			value
  Return:
	PRM_RSLT;		PRM_RSLT_SUCCESS:Normal end,PRM_RSLT_CONTINUE: Normal end Continuing(FnFor)
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
PRM_RSLT UnReadRushLifeTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
	*pValue =  (LONG)Axis->hLifeTime->RushLifeTimeHdl.var.RushSurvivor ;
#else
	*pValue =  0;
#endif
	return PRM_RSLT_SUCCESS;
}

/*****************************************************************************
  Description:	0x053 : DBCircuit life remaining rate
------------------------------------------------------------------------------
  Parameters:
	Cmd;			PRM_ACCCMD
	ArrayIdx;		Array Index
	AXIS_HANDLE;	axis Handle
	pValue;			value
  Return:
	PRM_RSLT;		PRM_RSLT_SUCCESS:Normal end,PRM_RSLT_CONTINUE: Normal end Continuing(FnFor)
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
PRM_RSLT UnReadDBLifeTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
//	*pValue =  (LONG)Axis->hLifeTime->DBLifeTimeHdl.var.DBSurvivor ;	/* <S180> */
	*pValue =  (LONG)Axis->hDBLifeTime->var.DBSurvivor ;				/* <S180> */
#else
	*pValue =  0;
#endif
	return PRM_RSLT_SUCCESS;
}

/*****************************************************************************
  Description:	0x054 : Number of thrust protection relay operations
------------------------------------------------------------------------------
  Parameters:
	Cmd;			PRM_ACCCMD
	ArrayIdx;		Array Index
	AXIS_HANDLE;	axis Handle
	pValue;			value
  Return:
	PRM_RSLT;		PRM_RSLT_SUCCESS:Normal end,PRM_RSLT_CONTINUE: Normal end Continuing(FnFor)
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
PRM_RSLT UnReadRushSwitchingCount(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
	*pValue =  (LONG)Axis->hLifeTime->RushLifeTimeHdl.var.RushSwitchCount ;
#else
	*pValue =  0;
#endif
	return PRM_RSLT_SUCCESS;
}

/*****************************************************************************
  Description:	0x056 : DBRelay operation count
------------------------------------------------------------------------------
  Parameters:
	Cmd;			PRM_ACCCMD
	ArrayIdx;		Array Index
	AXIS_HANDLE;	axis Handle
	pValue;			value
  Return:
	PRM_RSLT;		PRM_RSLT_SUCCESS:Normal end,PRM_RSLT_CONTINUE: Normal end Continuing(FnFor)
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
PRM_RSLT UnReadDBSwitchingCount(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
//	*pValue =  (LONG)Axis->hLifeTime->DBLifeTimeHdl.var.DBSwitchCount ;		/* <S180> */
	*pValue =  (LONG)Axis->hDBLifeTime->var.DBSwitchCount;					/* <S180> */
#else
	*pValue =  0;
#endif
	return PRM_RSLT_SUCCESS;
}
/* <S154> End */

/****************************************************************************
0x02C	Motor rated speed [Rotary type:min-1,Linear type:mm/s]			<S11C>
****************************************************************************/
PRM_RSLT UnReadMotorRatedSpeed(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
	LONG	lwk;

	lwk = 100 * (LONG)(((AXIS_HANDLE*)Axis)->UniPrms->Prm)->MencP.vrat_max.b.l;
	*pValue = MlibScalKskxkx( (((AXIS_HANDLE*)Axis)->UniPrms->Bprm)->ExpSpd, lwk, 1, NULL, -24 );

	return PRM_RSLT_SUCCESS;

}

/****************************************************************************
0x02C	Motor maximum speed [Rotary type:min-1,Linear type:mm/s]		<S11C>
****************************************************************************/
PRM_RSLT UnReadMotorMaxSpeed(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
	LONG	lwk;
	
	lwk = 100 * (LONG)(((AXIS_HANDLE*)Axis)->UniPrms->Bprm)->MaxVel;
	*pValue = MlibScalKskxkx( (LONG)(((AXIS_HANDLE*)Axis)->UniPrms->Bprm)->ExpSpd, lwk, 1, NULL, -24 );
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x02C	Servopack installation environment monitor [%]						<S03A>
****************************************************************************/
PRM_RSLT	UnAmpInstEnvMon( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	LONG	lwk;

	/*----------------------------------------------------------------------*/
	/*		Installation environment monitor calculation	:0.1℃×2ms×1000times				*/
	/*----------------------------------------------------------------------*/
//	lwk = Axis->CheckAlarm->EnvMon.var.UnInstEnvMonAmp/1000
//					+ Axis->CheckAlarm->EnvMon.conf.AmpEnvMonOffset;		  /* <S050> */
#if 0/* <S1D7>:SyscMakeMonitorData( ) */
	lwk = (Axis->EnvMonitor->var.UnInstEnvMonAmp * 0.0001)
									+ Axis->EnvMonitor->conf.AmpEnvMonOffset;
	*pValue = lwk;
#endif
	*pValue = Axis->UnMonitor->UnInstEnvMonAmp;	/* <S1D7> */

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x02D	Servo motor installation environment monitor [%]						<S03A>
****************************************************************************/
PRM_RSLT	UnMotInstEnvMon( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	LONG	lwk;

	/*----------------------------------------------------------------------*/
	/*		Installation environment monitor calculation	:1℃×2ms×1000times						*/
	/*----------------------------------------------------------------------*/
//	lwk = Axis->CheckAlarm->EnvMon.var.UnInstEnvMonMot/1000
//					+ Axis->CheckAlarm->EnvMon.conf.MotEnvMonOffset;		  /* <S050> */
#if 0/* <S1D7>:SyscMakeMonitorData( ) */
	lwk = (Axis->EnvMonitor->var.UnInstEnvMonMot * 0.001)
										+ Axis->EnvMonitor->conf.MotEnvMonOffset;
	*pValue = lwk;
#endif
	*pValue = Axis->UnMonitor->UnInstEnvMonMot;	/* <S1D7> */

	return PRM_RSLT_SUCCESS;
}

/* <S056>	Start */
/****************************************************************************
0x002E	Main circuit voltage value
****************************************************************************/
PRM_RSLT UnReadDcVolt(PRM_ACCCMD Cmd, UINT ArrayIdx, void *Axis, LONG *pValue)
{
	*pValue = GET_PN_DC_VOLT( );

	return PRM_RSLT_SUCCESS;
}
/* <S056>	End */

/****************************************************************************
0x050-0x53	Lifetime monitor [0.01%] Tentative version TODO						<S03A>
****************************************************************************/
PRM_RSLT	UnLifeTimeMon( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	*pValue = 1;

	return PRM_RSLT_SUCCESS;
}

/* <S056>	Start */
/****************************************************************************
0x054	Number of thrust protection relay operations
****************************************************************************/
PRM_RSLT	UnReadRelayCnt( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	/* TODO */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x056	DBRelay operation count
****************************************************************************/
PRM_RSLT	UnReadDBRelayCnt( PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue )
{
	/* TODO */
	return PRM_RSLT_SUCCESS;
}
/* <S056>	End */

/****************************************************************************
0x080	Maximum amount of movement in the forward direction when detecting a magnetic pole	MpfMovePosP	0.01mm
****************************************************************************/
PRM_RSLT UnReadMpfMaxPositiveMoveDist(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	lwk;
	LONG	lwk2;

	if( Axis->MencV->AxisMotType == MOTTYPE_ROTARY )
	{
		lwk = Axis->MpFind->UnMpfMovePosP / 10;
#if (FLOAT_USE==TRUE)
		lwk2 = ( (float)lwk * Axis->MencV->Kmovpos );
#else
		lwk2 = MlibMulgain( lwk, Axis->MencV->Kmovpos );
#endif /* FLOAT_USE */
	}
	else
	{
		lwk = Axis->MpFind->UnMpfMovePosP;
#if (FLOAT_USE==TRUE)
		lwk2 = ( (float)lwk * Axis->MencV->Kmovpos );
#else
		lwk2 = MlibMulgain( lwk, Axis->MencV->Kmovpos );
#endif /* FLOAT_USE */
		if( lwk2 > 32767 )
		{
			lwk2 = 32767;
		}
	}
	*pValue = lwk2;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x081	Maximum amount of movement in the reverse direction when detecting a magnetic pole	MpfMovePosN	0.01mm
****************************************************************************/
PRM_RSLT UnReadMpfMaxNegativeMoveDist(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	lwk;
	LONG	lwk2;

	if( Axis->MencV->AxisMotType == MOTTYPE_ROTARY )
	{
		lwk = Axis->MpFind->UnMpfMovePosN / 10;
#if (FLOAT_USE==TRUE)
		lwk2 = ( (float)lwk * Axis->MencV->Kmovpos );
#else
		lwk2 = MlibMulgain( lwk, Axis->MencV->Kmovpos );
#endif /* FLOAT_USE */
	}
	else
	{
		lwk = Axis->MpFind->UnMpfMovePosN;
#if (FLOAT_USE==TRUE)
		lwk2 = ( (float)lwk * Axis->MencV->Kmovpos );
#else
		lwk2 = MlibMulgain( lwk, Axis->MencV->Kmovpos );
#endif /* FLOAT_USE */
		if( lwk2 < -32767 )
		{
			lwk2 = -32767;
		}
	}
	*pValue = lwk2;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x082	Magnetic pole detection time required	MpfFindingTime	0.1s
****************************************************************************/
PRM_RSLT UnReadMpfDetectingTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG scantime;
	scantime = (Axis->UniPrms->Bprm)->SvCycleUs;
	*pValue = (Axis->MpFind->UnMpFindingTime * scantime) / 100000;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x083	Magnetic pole detection phase offset amount	MpfPhaseOffset	0.1deg
****************************************************************************/
PRM_RSLT UnReadMpfPhaseOffset(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	lwk;

	lwk = (Axis->MpFind->MpfPhaseOffset * (SHORT)3600) >> 16;
	if( lwk < 0 )
	{
		lwk += 3600;
	}
	*pValue = lwk;
	return PRM_RSLT_SUCCESS;
}

/* <S056>	Start */
/****************************************************************************
0xE084	Linear scale pitch[pm]
****************************************************************************/
PRM_RSLT UnReadLinearScalePitch(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)(Axis->UniPrms->Bprm)->MonScalePitch;
	*pValue = (LONG)((ArrayIdx == 0) ? x : ((ULONG)x >> 16));
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0xE086	Linear scale pitch index[10]
****************************************************************************/
PRM_RSLT UnReadLinearScalePitchExp(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Bprm)->MonScaleExp;
	return PRM_RSLT_SUCCESS;
}
/* <S056>	End */

/****************************************************************************
0x087	Magnetic pole detection judgment monitor
****************************************************************************/
PRM_RSLT UnReadMpfStatus(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG *MPJudgeMonitor;

	MPJudgeMonitor = Axis->MpFind->MPJudgeMonitor;
	*pValue = MPJudgeMonitor[0] + (MPJudgeMonitor[1] * 10) + (MPJudgeMonitor[2] * 100);

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x104	Serial encoder communication error count
****************************************************************************/
PRM_RSLT UnReadEncoderErrorCount(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = ((Axis->SvAsicRegs->AsicHwReg)->AREG_COMERRCNT - Axis->MencV->EncComErrCnt);		/*<S009>*//* <S217> */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x107	Residual vibration frequency	[0.1Hz]
****************************************************************************/
PRM_RSLT UnReadRvibFrequency(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->RemVibFreq->mon.UnRvibFrequency;				/* <S0DA> */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x110	Internal signal monitor
****************************************************************************/
PRM_RSLT UnReadInterSignalX(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	sig, foo;

/* <S0D0> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	
	NINV *pninv = &NinV;
#endif
/* <S0D0> End */

	/* 0bit Main circuit on */
	sig = (Axis->SeqCtrlOut->MainPowerOn << 0);
	/* bit1 Encoder ready */
	sig |= (Axis->MencV->EncReady << 1);
	/* bit2 Motor energization(Request) */
	sig |= (Axis->BeSequence->DbSeqData.DbBeReq << 2);
	/* bit3 Dynamic brake on */
	sig |= (Axis->BeSequence->DbOn << 3);

	/* bit4 Reverse rotation (movement) mode */
	sig |= (Axis->UniPrms->Bprm->RvsDir << 4);
	/* bit5 Mode switch on */
	sig |= (Axis->BaseControls->ModeSwData.var.ModeSWFlag << 5);

	switch( Axis->BaseCtrlOut->CtrlModeOut.b.cm )
	{
	case CTRLMODE_POS:
		/* bit8 Position command input in progress */
		sig |= ((Axis->SeqMotSts->var.dPcmdCnt != 0) << 8);
		/* bit9 Position command direction */
		sig |= ((Axis->SeqMotSts->var.dPcmdCnt >= 0) << 9);
		break;
	case CTRLMODE_TRQ:
		/* bit7 Torque command input in progress */
		sig |= ((Axis->BaseControls->CtrlCmdMngr.TrqRefo != 0) << 7);
		break;
	case CTRLMODE_SPD:
	default:
		/* bit6 Speed command input in progress */
		sig |= ((Axis->BaseControls->CtrlCmdMngr.SpdRefi != 0) << 6);
		break;
	}

	/* bit12 AC power turned on */
	sig |= (Axis->PowerManager->PowerSts.Acon << 12);
	/* bit13 Inrush resistance short circuit relay on */
	sig |= (Axis->PowerManager->PowerSts.ChargeOk << 13);
	/* bit14 Regeneration Tr on */
	sig |= (HALdrv_GetRegeneControlState() << 14);
	/* bit15 Regeneration error detection on */
	sig |= (Axis->PowerManager->RegeneCtrl.RegChk.RegeAlmSignal << 15);

	/* bit16 Motor energization(Status) */
	sig |= (Axis->BeSequence->BeReqSeqOut << 16);
	/* bit17 Reservation */
	/* bit18 Overcurrent */
	sig |= (ALMCheckEachState(Axis->AlmManager, ALM_OC) << 18);
	/* bit19 Origin point not passed complete */
	sig |= ((Axis->MencV->RxAlmCode & 0x0040) << (19-6));

	/* bit20 Inertia identification in progress */
//	sig |= (BoutV.f.InertiaTuning << 20);
	/* bit21 Magnetic pole detection in progress */
	foo = ( (Axis->BeSequence->BeReqSeqOut != FALSE)
			&& (Axis->MencV->PhaseReady == FALSE) );
	sig |= (foo << 21);
	/* bit22 Complete magnetic pole recognition */
	sig |= (Axis->MencV->PhaseReady << 22);

	/* bit23 Ripple compensation in progress  */
//	sig |= ( ( Axis->BaseLoops->RippleCmp.RplCmpPrm.RplCmpUse ) << 23 );	/* <S14B> */
	sig |= ( ( Axis->BaseLoops->RippleCmp.RippleCompMon ) << 23 );/* <S1FD> */


/* <S0D0> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */	
	/* bit26 CLRSignal  */
	sig |= ( ( pninv->NrmBits.CLR ) << 26 );
#endif
/* <S0D0> End */

	/**
	 * @@ bit24: PULS signal
	 *    bit25: SIGN signal
	 *    bit27: PSEL signal
	 */
	*pValue = sig;
	return PRM_RSLT_SUCCESS;
}

/* <S21D> Start*/
/****************************************************************************
0x110	Internal signal monitor 1
****************************************************************************/
PRM_RSLT UnReadInterSignalX1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	sig, foo;

/* <S0D0> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */
	NINV *pninv = &NinV;
#endif
/* <S0D0> End */

	/* 0bit Main circuit on */
	sig = (Axis->SeqCtrlOut->MainPowerOn << 0);
	/* bit1 Encoder ready */
	sig |= (Axis->MencV->EncReady << 1);
	/* bit2 Motor energization(Request) */
	sig |= (Axis->BeSequence->DbSeqData.DbBeReq << 2);
	/* bit3 Dynamic brake on */
	sig |= (Axis->BeSequence->DbOn << 3);

	/* bit4 Reverse rotation (movement) mode */
	sig |= (Axis->UniPrms->Bprm->RvsDir << 4);
	/* bit5 Mode switch on */
	sig |= (Axis->BaseControls->ModeSwData.var.ModeSWFlag << 5);

	switch( Axis->BaseCtrlOut->CtrlModeOut.b.cm )
	{
	case CTRLMODE_POS:
		break;
	case CTRLMODE_TRQ:
		/* bit7 Torque command input in progress */
		sig |= ((Axis->BaseControls->CtrlCmdMngr.TrqRefo != 0) << 7);
		break;
	case CTRLMODE_SPD:
	default:
		/* bit6 Speed command input in progress */
		sig |= ((Axis->BaseControls->CtrlCmdMngr.SpdRefi != 0) << 6);
		break;
	}

	*pValue = sig;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x111	Internal signal monitor 2
****************************************************************************/
PRM_RSLT UnReadInterSignalX2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	sig, foo;

/* <S0D0> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */
	NINV *pninv = &NinV;
#endif
/* <S0D0> End */

	sig = 0;
	switch( Axis->BaseCtrlOut->CtrlModeOut.b.cm )
	{
	case CTRLMODE_POS:
		/* bit0 Position command input in progress */
		sig |= ((Axis->SeqMotSts->var.dPcmdCnt != 0) << 0);
		/* bit1 Position command direction */
		sig |= ((Axis->SeqMotSts->var.dPcmdCnt >= 0) << 1);
		break;
	case CTRLMODE_TRQ:
		break;
	case CTRLMODE_SPD:
	default:
		break;
	}

	/* bit4 AC power turned on */
	sig |= (Axis->PowerManager->PowerSts.Acon << 4);
	/* bit5 Inrush resistance short circuit relay on */
	sig |= (Axis->PowerManager->PowerSts.ChargeOk << 5);
	/* bit6 Regeneration Tr on */
	sig |= (HALdrv_GetRegeneControlState() << 6);
	/* bit7 Regeneration error detection on */
	sig |= (Axis->PowerManager->RegeneCtrl.RegChk.RegeAlmSignal << 7);

	*pValue = sig;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x112	Internal signal monitor 3
****************************************************************************/
PRM_RSLT UnReadInterSignalX3(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	sig, foo;

/* <S0D0> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */
	NINV *pninv = &NinV;
#endif
/* <S0D0> End */

	sig = 0;
	/* bit0 Motor energization(Status) */
	sig |= (Axis->BeSequence->BeReqSeqOut << 0);
	/* bit1 Reservation */
	/* bit2 Overcurrent */
	sig |= (ALMCheckEachState(Axis->AlmManager, ALM_OC) << 2);
	/* bit3 Origin point not passed complete */
	sig |= ((Axis->MencV->RxAlmCode & 0x0040) >> 3);

	/* bit20 Inertia identification in progress */
//	sig |= (BoutV.f.InertiaTuning << 20);
	/* bit5 Magnetic pole detection in progress */
	foo = ( (Axis->BeSequence->BeReqSeqOut != FALSE)
			&& (Axis->MencV->PhaseReady == FALSE) );
	sig |= (foo << 5);
	/* bit6 Complete magnetic pole recognition */
	sig |= (Axis->MencV->PhaseReady << 6);

	/* bit7 Ripple compensation in progress  */
//	sig |= ( ( Axis->BaseLoops->RippleCmp.RplCmpPrm.RplCmpUse ) << 23 );	/* <S14B> */
	sig |= ( ( Axis->BaseLoops->RippleCmp.RippleCompMon ) << 7 );/* <S1FD> */


	*pValue = sig;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x113	Internal signal monitor4
****************************************************************************/
PRM_RSLT UnReadInterSignalX4(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	sig, foo;

/* <S0D0> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */
	NINV *pninv = &NinV;
#endif
/* <S0D0> End */

	sig = 0;
/* <S0D0> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)	/* AnalogIF */
	/* bit2 CLRSignal  */
	sig |= ( ( pninv->NrmBits.CLR ) << 2 );
#endif
/* <S0D0> End */

	/**
	 * @@ bit24: PULS signal
	 *    bit25: SIGN signal
	 *    bit27: PSEL signal
	 */
	*pValue = sig;
	return PRM_RSLT_SUCCESS;
}
/* <S21D> End*/
/****************************************************************************
0x120	Internal input signal monitor
****************************************************************************/
PRM_RSLT UnReadInternalInputSignals(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;

	/* Bit0 to Bit7 */
	rv = ((0 != (Axis->BaseCtrlOut->CmdSeqBit & SERVO_ON_BIT)) << 0);
	//rv |= ((0 != (Axis->BaseCtrlOut->CmdSeqBit & ENBPCTRL_BIT)) << 1);
	rv |= ((0 != (Axis->SvControlIf->CmdCtrl.CmdCtrlBit & ENBPCTRL_BIT)) << 1);		/* <S162>  */
	//rv |= (Axis->SeqCtrlOut->PotSigSts << 2);
	//rv |= (Axis->SeqCtrlOut->NotSigSts << 3);
	rv |= (((Axis->BaseCtrlOut->CmdSeqBit >> POSITIVE_OT_BITNO) & 0x1) << 2);
	rv |= (((Axis->BaseCtrlOut->CmdSeqBit >> NEGATIVE_OT_BITNO) & 0x1) << 3);

	//rv |= (Axis->AlmManager->AlmRst << 4);
	rv |= ((0 != (Axis->BaseCtrlOut->CmdSeqBit & ALM_RESET_BIT)) << 4);				/* <S162>  */
	rv |= (Axis->BaseControls->TrqLimitData.var.PclSignal << 5);
	rv |= (Axis->BaseControls->TrqLimitData.var.NclSignal << 6);
	rv |= ((0 != (Axis->BaseCtrlOut->CmdSeqBit & SENSOR_ON_BIT)) << 7);

	/* Bit8 to Bit13, Bit15 are reserved */

	/* Bit14 */
	rv |= (Axis->BaseControls->GselNum & 0x3) << 14;

	/* Bit16 to Bit21 */
	rv |= ((0 != (Axis->BaseCtrlOut->CmdSeqBit & POL_DETECT_BIT)) << 16);
	rv |= (Axis->IoSignals->FstpSignal.SignalStatus.v << 17);		/* <S1F5> */
	rv |= (Axis->IoSignals->DecSignal.SignalStatus.v << 18);		/* <S045> */
	rv |= (Axis->IoSignals->ExtSignal[0].SignalStatus.v << 19);		/* <S045> */

	rv |= (Axis->IoSignals->ExtSignal[1].SignalStatus.v << 20);		/* <S045> */
	rv |= (Axis->IoSignals->ExtSignal[2].SignalStatus.v << 21);		/* <S045> */

	/* Bit22 to Bit31 are reserved */

	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}

/* <S21D> Start*/
/****************************************************************************
0x120	Internal input signal monitor1
****************************************************************************/
PRM_RSLT UnReadInternalInputSignals1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;

	/* Bit0 to Bit7 */
	rv = ((0 != (Axis->BaseCtrlOut->CmdSeqBit & SERVO_ON_BIT)) << 0);
	//rv |= ((0 != (Axis->BaseCtrlOut->CmdSeqBit & ENBPCTRL_BIT)) << 1);
	rv |= ((0 != (Axis->SvControlIf->CmdCtrl.CmdCtrlBit & ENBPCTRL_BIT)) << 1);		/* <S162>  */
	//rv |= (Axis->SeqCtrlOut->PotSigSts << 2);
	//rv |= (Axis->SeqCtrlOut->NotSigSts << 3);
	rv |= (((Axis->BaseCtrlOut->CmdSeqBit >> POSITIVE_OT_BITNO) & 0x1) << 2);
	rv |= (((Axis->BaseCtrlOut->CmdSeqBit >> NEGATIVE_OT_BITNO) & 0x1) << 3);

	//rv |= (Axis->AlmManager->AlmRst << 4);
	rv |= ((0 != (Axis->BaseCtrlOut->CmdSeqBit & ALM_RESET_BIT)) << 4);				/* <S162>  */
	rv |= (Axis->BaseControls->TrqLimitData.var.PclSignal << 5);
	rv |= (Axis->BaseControls->TrqLimitData.var.NclSignal << 6);
	rv |= ((0 != (Axis->BaseCtrlOut->CmdSeqBit & SENSOR_ON_BIT)) << 7);


	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x121	Internal input signal monitor2
****************************************************************************/
PRM_RSLT UnReadInternalInputSignals2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;

	/* Bit0 to Bit5, Bit7 are reserved */

	/* Bit7 */
	rv = (Axis->BaseControls->GselNum & 0x3) << 6;

	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x122	Internal input signal monitor3
****************************************************************************/
PRM_RSLT UnReadInternalInputSignals3(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;


	/* Bit0 to Bit5 */
	rv = ((0 != (Axis->BaseCtrlOut->CmdSeqBit & POL_DETECT_BIT)) << 0);
	rv |= (Axis->IoSignals->FstpSignal.SignalStatus.v << 1);		/* <S1F5> */
	rv |= (Axis->IoSignals->DecSignal.SignalStatus.v << 2);		/* <S045> */
	rv |= (Axis->IoSignals->ExtSignal[0].SignalStatus.v << 3);		/* <S045> */

	rv |= (Axis->IoSignals->ExtSignal[1].SignalStatus.v << 4);		/* <S045> */
	rv |= (Axis->IoSignals->ExtSignal[2].SignalStatus.v << 5);		/* <S045> */


	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x123	Internal input signal monitor4
****************************************************************************/
UnReadInternalInputSignals4(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;
	HWSIN	SeqInputSignals;
	USHORT	faultState;

	SeqInputSignals = HALdrv_ReadInputSignals( );

	/* Bit0 : Regeneration error detection input */
	rv = SeqInputSignals.flag.RegeneAlm << 0;		/* ALRGSignal				*/

	/* Bit1 : Converter output signal status1 */
	rv |= SeqInputSignals.flag.Acon1 << 1;			/* ACON1Signal			*/

	/* Bit2 : Converter output signal status2 */
	rv |= SeqInputSignals.flag.Acon2 << 2;			/* ACON2Signal			*//* <S1C9> */

	/* Bit3 : Overcurrent detection input(A.100:Overcurrent(OC1)Occurrence condition flag) */
	rv |= Axis->AlmManager->OCAlmOccurFlag1 << 3;	/* OC1Signal				*//* <S1C9> */

	/* Bit4 : Overcurrent detection input(A.100:Overcurrent(OC2)Occurrence condition flag) */
	rv |= Axis->AlmManager->OCAlmOccurFlag2 << 4;	/* OC2Signal				*//* <S1C9> */

	/* Bit5 : Fan alarm detection input signal */
	rv |= SeqInputSignals.flag.FanAlm << 5;			/* FANALMSignal			*/

	/* Bit6 to Bit31 are reserved */
	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}
/* <S21D> End*/

#if 0	/* <S21D> Start */
/* <S1A0> Start */
/****************************************************************************
0x122	Internal input signal monitor2
****************************************************************************/
UnReadInternalInputSignals2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;
	HWSIN	SeqInputSignals;
	USHORT	faultState;

	SeqInputSignals = HALdrv_ReadInputSignals( );

	/* Bit0 : Regeneration error detection input */
	rv = SeqInputSignals.flag.RegeneAlm << 0;		/* ALRGSignal				*/

	/* Bit1 : Converter output signal status1 */
	rv |= SeqInputSignals.flag.Acon1 << 1;			/* ACON1Signal			*/

	/* Bit2 : Converter output signal status2 */
	rv |= SeqInputSignals.flag.Acon2 << 2;			/* ACON2Signal			*//* <S1C9> */

	/* Bit3 : Overcurrent detection input(A.100:Overcurrent(OC1)Occurrence condition flag) */
	rv |= Axis->AlmManager->OCAlmOccurFlag1 << 3;	/* OC1Signal				*//* <S1C9> */

	/* Bit4 : Overcurrent detection input(A.100:Overcurrent(OC2)Occurrence condition flag) */
	rv |= Axis->AlmManager->OCAlmOccurFlag2 << 4;	/* OC2Signal				*//* <S1C9> */

	/* Bit5 : Fan alarm detection input signal */
	rv |= SeqInputSignals.flag.FanAlm << 5;			/* FANALMSignal			*/

	/* Bit6 to Bit31 are reserved */
	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}
/* <S1A0> End */
#endif	/* <S21D> End */

/****************************************************************************
0x130	Internal output signal monitor
****************************************************************************/
PRM_RSLT UnReadInternalOutputSignals(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;
	BOOL	alm, wrn;
	VULONG	AregFbcset;			/* ASIC JL-086 SERVOIP H/WRegister Division input filter setting(0x0040) */	/* <S1FA> */
	AregFbcset = ((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicHwReg->AREG_FBCSET;							/* <S1FA> */

	ALMGetStatus(Axis->AlmManager, &alm, &wrn);										/* bit00 : ALARM */
	rv = alm << 0;
	rv |= (Axis->BaseControls->PosCtrlSts.CoinSignal << 1);							/* bit01 : COIN */
	rv |= (Axis->SeqCtrlOut->VcmpSts << 2);											/* bit02 : V-CMP */
	rv |= (Axis->SeqCtrlOut->TgOnSts << 3);											/* bit03 : TGON */

	rv |= (Axis->BeSequence->ServoReady << 4);										/* bit04 : S-RDY */
	rv |= (((Axis->SvControlIf->RspCtrl.RspCtrlBit >> TRQ_CLAMP_BITNO) & 1) << 5);	/* bit05 : CLT */
	rv |= (((Axis->SvControlIf->RspCtrl.RspCtrlBit >> SPD_CLAMP_BITNO) & 1) << 6);	/* bit06 : VLT */
	rv |= ((!(Axis->SeqCtrlOut->BrkSigSts)) << 7);									/* bit07 : BK */

	rv |= wrn << 8;																	/* bit08 : WARN */
	rv |= (Axis->BaseControls->PosCtrlSts.NearSignal << 9);							/* bit09 : NEAR */
	if(Axis->UniPrms->Bprm->FencUse != FALSE)										/* bit10 : C-PULSE */
	{
		rv |= (Axis->FencV->CposOk & (Axis->FencV->MposFromCpos == 0)) << 10;
	}
	else
	{
		rv |= (Axis->MencV->CposOk & (Axis->MencV->MposFromCpos == 0)) << 10;
	}

	rv |= (((Axis->PowerManager->MainPowerChecker.var.PreSigOut) & 1) << 11);		/* bit11 : Preventive maintenance signal output */	/* <S172> */

	/* bit12 ~ bit13 : Preliminary */
	/* bit14 : DAW1(★TO DO) */
	/* bit15 : DAW1(★TO DO) */

	/* <S1FA> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	rv |= (Axis->AlmManager->AlmCodeOut.Alo1) << 16;								/* bit16 : ALO1(APFor type only) */
	rv |= (Axis->AlmManager->AlmCodeOut.Alo2) << 17;								/* bit17 : ALO2(APFor type only) */
	rv |= (Axis->AlmManager->AlmCodeOut.Alo3) << 18;								/* bit18 : ALO3(APFor type only) */
#endif	/* CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE :APShape */

	rv |= ((AregFbcset >> 9) & 0x0001) << 19;										/* bit19 : PAO */
	rv |= ((AregFbcset >> 10) & 0x0001) << 20;										/* bit20 : PBO */
	rv |= ((AregFbcset >> 11) & 0x0001) << 21;										/* bit21 : PCO */

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	rv |= ( (SqoV.Sigs & SQO_SIGS_PSELA ) >> 19 ) << 22;							/* bit22 : PSELA(APFor type only) */
#endif	/* CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE :APShape */
	/* <S1FA> End */

	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}

/* <S21D> Start */
/****************************************************************************
0x130	Internal output signal monitor1
****************************************************************************/
PRM_RSLT UnReadInternalOutputSignals1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;
	BOOL	alm, wrn;
	VULONG	AregFbcset;			/* ASIC JL-086 SERVOIP H/WRegister Division input filter setting(0x0040) */	/* <S1FA> */
	AregFbcset = ((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicHwReg->AREG_FBCSET;							/* <S1FA> */

	ALMGetStatus(Axis->AlmManager, &alm, &wrn);										/* bit00 : ALARM */
	rv = alm << 0;
	rv |= (Axis->BaseControls->PosCtrlSts.CoinSignal << 1);							/* bit01 : COIN */
	rv |= (Axis->SeqCtrlOut->VcmpSts << 2);											/* bit02 : V-CMP */
	rv |= (Axis->SeqCtrlOut->TgOnSts << 3);											/* bit03 : TGON */

	rv |= (Axis->BeSequence->ServoReady << 4);										/* bit04 : S-RDY */
	rv |= (((Axis->SvControlIf->RspCtrl.RspCtrlBit >> TRQ_CLAMP_BITNO) & 1) << 5);	/* bit05 : CLT */
	rv |= (((Axis->SvControlIf->RspCtrl.RspCtrlBit >> SPD_CLAMP_BITNO) & 1) << 6);	/* bit06 : VLT */
	rv |= ((!(Axis->SeqCtrlOut->BrkSigSts)) << 7);									/* bit07 : BK */


	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x131	Internal output signal monitor2
****************************************************************************/
PRM_RSLT UnReadInternalOutputSignals2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;
	BOOL	alm, wrn;
	VULONG	AregFbcset;			/* ASIC JL-086 SERVOIP H/WRegister Division input filter setting(0x0040) */	/* <S1FA> */
	AregFbcset = ((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicHwReg->AREG_FBCSET;							/* <S1FA> */

	ALMGetStatus(Axis->AlmManager, &alm, &wrn);										/* bit00 : ALARM */

	rv = wrn << 0;																	/* bit00 : WARN */
	rv |= (Axis->BaseControls->PosCtrlSts.NearSignal << 1);							/* bit01 : NEAR */
	if(Axis->UniPrms->Bprm->FencUse != FALSE)										/* bit02 : C-PULSE */
	{
		rv |= (Axis->FencV->CposOk & (Axis->FencV->MposFromCpos == 0)) << 2;
	}
	else
	{
		rv |= (Axis->MencV->CposOk & (Axis->MencV->MposFromCpos == 0)) << 2;
	}

	rv |= (((Axis->PowerManager->MainPowerChecker.var.PreSigOut) & 1) << 3);		/* bit03 : Preventive maintenance signal output */	/* <S172> */

	/* bit12 ~ bit13 : Preliminary */
	/* bit14 : DAW1(★TO DO) */
	/* bit15 : DAW1(★TO DO) */


	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x132	Internal output signal monitor3
****************************************************************************/
PRM_RSLT UnReadInternalOutputSignals3(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG	rv;
	BOOL	alm, wrn;
	VULONG	AregFbcset;			/* ASIC JL-086 SERVOIP H/WRegister Division input filter setting(0x0040) */	/* <S1FA> */
	AregFbcset = ((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicHwReg->AREG_FBCSET;							/* <S1FA> */

	rv = 0;
	/* <S1FA> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	rv |= (Axis->AlmManager->AlmCodeOut.Alo1) << 0;								/* bit0 : ALO1(APFor type only) */
	rv |= (Axis->AlmManager->AlmCodeOut.Alo2) << 1;								/* bit1 : ALO2(APFor type only) */
	rv |= (Axis->AlmManager->AlmCodeOut.Alo3) << 2;								/* bit2 : ALO3(APFor type only) */
#endif	/* CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE :APShape */

	rv |= ((AregFbcset >> 9) & 0x0001) << 3;										/* bit3 : PAO */
	rv |= ((AregFbcset >> 10) & 0x0001) << 4;										/* bit4 : PBO */
	rv |= ((AregFbcset >> 11) & 0x0001) << 5;										/* bit5 : PCO */

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	rv |= ( (SqoV.Sigs & SQO_SIGS_PSELA ) >> 19 ) << 6;							/* bit6 : PSELA(APFor type only) */
#endif	/* CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE :APShape */
	/* <S1FA> End */

	*pValue = rv;
	return PRM_RSLT_SUCCESS;
}
/* <S21D> End */

/****************************************************************************
0x140	Main circuit bus voltage V
****************************************************************************/
PRM_RSLT UnReadDCVoltage(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (FLOAT_USE==TRUE)
	*pValue = Axis->PowerManager->PowerSts.DcVolt *
	                      Axis->PowerManager->DetectDcVolt.conf.Kdcvolt;
#else
	*pValue = MlibMulgain(Axis->PowerManager->PowerSts.DcVolt,
	                      Axis->PowerManager->DetectDcVolt.conf.Kdcvolt );
#endif /* FLOAT_USE */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x141	Current detection value	IqRate	%
****************************************************************************/
PRM_RSLT UnReadIqRate(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
//<S0DA>	*pValue = 0;
	*pValue = 
		MlibMulgain( Axis->UnMonitor->UnCrntDet, (Axis->UniPrms->Bprm)->Kmoncrnt ); /* <S0DA> */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x142	Cumulative load rate	MotLoad2ms	%(Period2ms)
****************************************************************************/
PRM_RSLT UnReadMotorLoad2ms(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (FLOAT_USE==TRUE)
	*pValue = Axis->UnMonitor->UnCrntRef * Axis->UniPrms->Bprm->Kmoncrnt;
#else
	*pValue = MlibMulgain( Axis->UnMonitor->UnCrntRef, Axis->UniPrms->Bprm->Kmoncrnt );
#endif /* FLOAT_USE */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x143	Regeneration load rate	RegerRate2ms	%(Period2ms)
****************************************************************************/
PRM_RSLT UnReadRegeneLoad2ms(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnRgPower;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x144	DB resistance power consumption	DbrxPower2ms	%(Period2ms)
****************************************************************************/
PRM_RSLT UnReadDbPower2ms(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnDbPower;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x145	Maximum cumulative load factor	MotLoad10s	%(Period10s)
****************************************************************************/
PRM_RSLT UnReadMaxMotorLoad10s(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UnMonitor->UnIsqrSumMax;
	return PRM_RSLT_SUCCESS;
}

/* <S056>	Start */
/****************************************************************************
0xE148	T-REFMonitor
****************************************************************************/
PRM_RSLT UnReadTrefAdMon(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	ANLGV *panlg = &AnlgV;
	LONG Torque;

	Torque = panlg->TrqRefAd;
	*pValue = MlibMulgain( Torque, (Axis->UniPrms->Bprm)->Kmontrq ); /* [2^24/MaxTrq → [ % ]	*/
#endif
	return PRM_RSLT_SUCCESS;

}

/****************************************************************************
0xE149	V-REFMonitor
****************************************************************************/
PRM_RSLT UnReadVrefAdMon(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	ANLGV *panlg = &AnlgV;
	LONG Speed;

	Speed = panlg->SpdRefAd;
	*pValue = MlibMulgainSx48( Speed, (Axis->UniPrms->Bprm)->Kmonspd );	/* [2^24/MaxSpd → [ min-1 , mm/s ] */
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xE14A	Input command pulse frequency
****************************************************************************/
PRM_RSLT UnReadDPulseHwCntMon(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	ANLGV *panlg = &AnlgV;

	/* Countermeasures for overflow caused by adding command pulse input magnification function */
	//*pValue = MlibMulgain30( Axis->UnMonitor->UnPcmd64ms, 0x137d0000 );		   /* 1000÷64=15.625 ==> 0x137d0000   */
	*pValue = MlibMulgain30( Axis->UnMonitor->UnPcmd64Sum, 0x137d0000 );	/* <S1B6> */
#endif
	return PRM_RSLT_SUCCESS;
}

/* <S056>	End */

/****************************************************************************
0x14C	dAxis current command monitor	IdrefRate	%
****************************************************************************/
PRM_RSLT UnReadIdRefRate(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (FLOAT_USE==TRUE)
	*pValue = Axis->CtrlLoopOut->IdRefMon * 
	                       Axis->UniPrms->Bprm->Kmoncrnt;
#else
	*pValue = MlibMulgain( Axis->CtrlLoopOut->IdRefMon,
	                       Axis->UniPrms->Bprm->Kmoncrnt );
#endif /* FLOAT_USE */
	return PRM_RSLT_SUCCESS;
}

/* <S056>	Start */
/****************************************************************************
0xE14D	MEMOBUSError information (error occurrence address)※When there is no error information0xFFFF
****************************************************************************/
PRM_RSLT UnReadMemobusErrorInfo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	LONG	ErrRegAdd;
	MSGIF	*Hmsg_Com0;
	MSGIF	*Hmsg_Com1;

	/* Get Serial Message I/F for SigmaMsg/LcdOp/Memobus	*/
	Hmsg_Com0 = &SrlMsg[COM0_UART];		/* Serial CH:0 */
	Hmsg_Com1 = &SrlMsg[COM1_UART];		/* Serial CH:1 */

	if( (Hmsg_Com0->CmdBuf[1] != 0x00) || (Hmsg_Com1->CmdBuf[1] != 0x00) )
	{
		if(Hmsg_Com0->CmdBuf[1] != 0x00)	/* Serial CH:0 */
		{
			//ErrRegAdd = (LONG)(Hmsg_Com0->ErrRno);
			ErrRegAdd = (LONG)(Hmsg_Com0->LstErrRno);
		}
		else								/* Serial CH:1 */
		{
			//ErrRegAdd = (LONG)(Hmsg_Com1->ErrRno);
			ErrRegAdd = (LONG)(Hmsg_Com1->LstErrRno);
		}
	}

	if(ErrRegAdd == 0)
	{
		*pValue = 0xffff;
	}
	else
	{
		*pValue = ErrRegAdd;
	}
	return PRM_RSLT_SUCCESS;
}
/* <S056>	End */

/* <S154>	Start  */
/*****************************************************************************
0x170	 Average internal temperature		[0.01℃]
*****************************************************************************/
PRM_RSLT UnReadAvgIntTmrp(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if ( CSW_SVCF_LIFEPRED_USE == TRUE )
	*pValue = Axis->hLifeTime->AvgTmprHdl.var.AvgIntTmpr;
#else
	*pValue = Axis->OverHeat->UnMon.UnAverageTemp;		/* <S183>Displaying the average value of control board and power board when lifespan diagnosis is disabled */
#endif
	return PRM_RSLT_SUCCESS;
}
/* <S154>	End */
/* <S154>	Start  */
/****************************************************************************
0x1D5 : Internal signal monitor [-]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInterSig0(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = (Axis->hAlmLatchHndl->LatchMon.InterSignal & 0x000000FF);

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1D6 : Internal signal monitor [Second]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInterSig1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = ( (Axis->hAlmLatchHndl->LatchMon.InterSignal & 0x0000FF00) >> 8 );

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1D7 : Internal signal monitor [Three]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInterSig2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = ( (Axis->hAlmLatchHndl->LatchMon.InterSignal & 0x00FF0000) >> 16 );

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1D8 : Internal input signal monitor [-]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInSig0(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = (Axis->hAlmLatchHndl->LatchMon.InSignal & 0x000000FF);

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1D9 : Internal input signal monitor [Second]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInSig1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = ( (Axis->hAlmLatchHndl->LatchMon.InSignal & 0x0000FF00) >> 8 ) ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1DA : Internal input signal monitor [Three]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInSig2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = ( (Axis->hAlmLatchHndl->LatchMon.InSignal & 0x00FF0000) >> 16 ) ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1DB : Internal input signal monitor [Four]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInSig3(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = ( (Axis->hAlmLatchHndl->LatchMon.InSignal & 0xFF000000) >> 24 ) ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1DC : Internal output signal monitor [-]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonOutSig0(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = (Axis->hAlmLatchHndl->LatchMon.OutSignal & 0x000000FF) ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1DD : Internal output signal monitor [Second]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonOutSig1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = ( (Axis->hAlmLatchHndl->LatchMon.OutSignal & 0x0000FF00) >> 8 ) ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x1DE : Internal output signal monitor [Three]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonOutSig2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = ( (Axis->hAlmLatchHndl->LatchMon.OutSignal & 0x00FF0000) >> 16 ) ;

	return PRM_RSLT_SUCCESS;
}
/* <S154>	End  */
/****************************************************************************
0x200	Encoder internal temperature temperature monitor	MencTemp	℃
****************************************************************************/
PRM_RSLT UnReadMotroEncoderTemperature(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->MencV->Temperature;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x210	User parametersCRCCalculated value
****************************************************************************/

/*--------------------------------------------------------------------------*/
/*		CRC16Calculation local function												*/
/*--------------------------------------------------------------------------*/
static void cal_crc16(UCHAR *Dbuf, LONG Length, ULONG *pa)
{
LONG	i;
ULONG	a = *pa;
ULONG	b = 0xA001;	/* generator polynomial */

		for( i=0; i<Length; i++ )
		{
			a = a ^ (ULONG)Dbuf[i];
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 1	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 2	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 3	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 4	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 5	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 6	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 7	*/
			a = (a & 0x01)? ((a>>1)^b) : (a>>1);		/* 8	*/
		}
		*pa = a;
		return;
}

/*--------------------------------------------------------------------------*/
/*		0x210	User parametersCRCCalculation register callback function			*/
/*--------------------------------------------------------------------------*/
PRM_RSLT UnReadUsrPrmCrcValue(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	CPRMTBL		*pTbl;
	void		*pDstData;
	ULONG		i,ArraySize;
	ULONG		crData=0xFFFF;	/* CRC-16 Initial value */
	UINT		accLv,usSize;

	for(pTbl=Axis->RegManager->hPnReg->PrmListTable; pTbl->Number != 0xFFFF; pTbl++)
	{
		accLv = pTbl->Attribute->AccessLevel;
		if(accLv == 1)
		{/* Access level1parameters are subject to calculation */
			if(NULL != pTbl->Attribute->GetRamPtrCallback)
			{
				usSize = pTbl->Attribute->DataLength;
				ArraySize = (pTbl->Attribute->NumberOfArray != 0) ? 
							pTbl->Attribute->NumberOfArray : 1;
				for(i = 0; i < ArraySize; i++)
				{	/* Get Parameter ram address */
					pDstData = pTbl->Attribute->GetRamPtrCallback(i, Axis->RegManager->hPnReg->GetRamPtrParam);
					cal_crc16( (UCHAR*)pDstData, usSize, &crData);	/* CRC-16Calculation */
				}	
			}
		}
	}
	*pValue = (USHORT)crData;
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x300	2msaroundCPULoad factor	CurExeTimeS	ns
****************************************************************************/
PRM_RSLT UnReadCPULoad(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)KlibClockToTime(Axis->TaskProcessTime->crtTimePer2ms);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x302	2msaroundCPUMaximum load factor	MaxExeTimeS	ns
****************************************************************************/
PRM_RSLT UnReadMaxCPULoad(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	if(Cmd == PRM_ACCCMD_READ)
	{/* read access */
		*pValue = (LONG)KlibClockToTime(Axis->TaskProcessTime->maxTimePer2ms);
	}
	else
	{/* reset monitor value */
		Axis->TaskProcessTime->maxTimePer2ms = 0;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x304	ScanBCurrent processing time	CurExeTimeB	ns
****************************************************************************/
PRM_RSLT UnReadScanBProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)KlibClockToTime(Axis->TaskProcessTime->SB_crtTime);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x306	ScanBMaximum processing time	MaxExeTimeB	ns
****************************************************************************/
PRM_RSLT UnReadScanBMaxProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	if(Cmd == PRM_ACCCMD_READ)
	{/* read access */
		*pValue = (LONG)KlibClockToTime(Axis->TaskProcessTime->SB_maxTime);
	}
	else
	{/* reset monitor value */
		Axis->TaskProcessTime->SB_maxTime = 0;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x308	ScanCCurrent processing time	CurExeTimeC	ns
****************************************************************************/
PRM_RSLT UnReadScanCProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)KlibClockToTime(Axis->TaskProcessTime->SC_crtTime);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x30A	ScanCMaximum processing time	MaxExeTimeC	ns
****************************************************************************/
PRM_RSLT UnReadScanCMaxProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	if(Cmd == PRM_ACCCMD_READ)
	{/* read access */
		*pValue = (LONG)KlibClockToTime(Axis->TaskProcessTime->SC_maxTime);
	}
	else
	{/* reset monitor value */
		Axis->TaskProcessTime->SC_maxTime = 0;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x310	μIntADCurrent processing time	CurExeTimeMI	ns
****************************************************************************/
PRM_RSLT UnReadMIntADProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 2012.08.31 Y.Oka ★μProcessing time measurement currently not supported★ */
	*pValue = (LONG)SHal_ClockToTime(Axis->SvAsicRegs->AsicMicroReg->MREG_INTADCTIME);
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x312	μIntADMaximum processing time	MaxExeTimeMI	ns
****************************************************************************/
PRM_RSLT UnReadMIntADMaxProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 2012.08.31 Y.Oka ★μProcessing time measurement currently not supported★ */
	if(Cmd == PRM_ACCCMD_READ)
	{/* read access */
		*pValue = (LONG)SHal_ClockToTime(Axis->SvAsicRegs->AsicMicroReg->MREG_INTADMTIME);
	}
	else
	{/* reset monitor value */
		Axis->SvAsicRegs->AsicMicroReg->MREG_INTADMTIME = 0;
	}
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x314	μScanACurrent processing time	CurExeTimeMA	ns
****************************************************************************/
PRM_RSLT UnReadMScanAProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 2012.08.31 Y.Oka ★μProcessing time measurement currently not supported★ */
	*pValue = (LONG)SHal_ClockToTime(Axis->SvAsicRegs->AsicMicroReg->MREG_SCANACTIME);
#endif
	*pValue = (LONG)KlibClockToTime(Axis->TaskProcessTime->SA_crtTime);		/* Although it is for micro, add it here for now	<S043> */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x316	μScanAMaximum processing time	MaxExeTimeMA	ns
****************************************************************************/
PRM_RSLT UnReadMScanAMaxProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 2012.08.31 Y.Oka ★μProcessing time measurement currently not supported★ */
	if(Cmd == PRM_ACCCMD_READ)
	{/* read access */
		*pValue = (LONG)SHal_ClockToTime(Axis->SvAsicRegs->AsicMicroReg->MREG_SCANAMTIME);
	}
	else
	{/* reset monitor value */
		Axis->SvAsicRegs->AsicMicroReg->MREG_SCANAMTIME = 0;
	}
#else		/* Although it is for micro, add it here for now	<S043> */
	if(Cmd == PRM_ACCCMD_READ)
	{/* read access */
		*pValue = (LONG)KlibClockToTime(Axis->TaskProcessTime->SA_maxTime);
	}
	else
	{/* reset monitor value */
		Axis->TaskProcessTime->SA_maxTime = 0;
	}
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x318	μRoundCurrent processing time	CurExeTimeMR	ns
****************************************************************************/
PRM_RSLT UnReadMRoundProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 2012.08.31 Y.Oka ★μProcessing time measurement currently not supported★ */
	*pValue = (LONG)SHal_ClockToTime(Axis->SvAsicRegs->AsicMicroReg->MREG_ROUNDCTIME);
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x31A	μRoundMaximum processing time	MaxExeTimeMR	ns
****************************************************************************/
PRM_RSLT UnReadMRoundMaxProcessTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 2012.08.31 Y.Oka ★μProcessing time measurement currently not supported★ */
	if(Cmd == PRM_ACCCMD_READ)
	{/* read access */
		*pValue = (LONG)SHal_ClockToTime(Axis->SvAsicRegs->AsicMicroReg->MREG_ROUNDMTIME);
	}
	else
	{/* reset monitor value */
		Axis->SvAsicRegs->AsicMicroReg->MREG_ROUNDMTIME = 0;
	}
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x500	Alarm history data	AlmTrace
****************************************************************************/
PRM_RSLT UnReadAlarmHistory(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	ULONG tmStamp;
	*pValue = (LONG)ALMGetAlarmHistory(Axis->AlmManager, ArrayIdx, &tmStamp);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x50B	Data for speed command adjustment	<S00C>
****************************************************************************/
PRM_RSLT uncal_VrefAdjData(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if SVFSEL_ADC_USE == 1

LONG	x;
	#if ( SVFSEL_VREFAD_24BIT != 0 )
	LONG	y = 3500;
	#else
	LONG	y = 16;
	#endif

	x = Axis->UniPrms->Prm->zadjvl;				/* Zadjv : Max.52500000 */
	*pValue = ( x / y );						/* 52500000 / 3500 = 15000 */

#if 0
	CADC *pAdc = Axis->pAdc;

	x = pAdc->P.ZadjVelo;			/* Zadjv : Max.52500000						*/
	*pValue = ( x / y );			/* 52500000 / 3500 = 15000					*/
#endif

#else /* SVFSEL_ADC_USE == 1 */
	*pValue = 0;
#endif

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x50C	Torque command adjustment data		<S00C>
****************************************************************************/
PRM_RSLT uncal_TrefAdjData(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if SVFSEL_ADC_USE == 1
//LONG	x;
//	CADC *pAdc = &LAdc[Axis->AxisNo];

//	x = pAdc->P.ZadjTrqP;
	//*pValue = x;
//	*pValue = Axis->pAdc->P.ZadjTrqP;

	*pValue = (Axis->UniPrms->Prm->zadjtp_n.b.l & 0xFF);

#else
	*pValue = 0;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************************************/
/*		0x050D : Analog monitor (CH1) Zero adjustment data													*/
/****************************************************************************************************/
PRM_RSLT uncal_AmonZadjCh1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (Axis->UniPrms->Cprm->mnaz0_1.b.l & 0xFF);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************************************/
/*		0x050E : Analog monitor (CH2) Zero adjustment data													*/
/****************************************************************************************************/
PRM_RSLT uncal_AmonZadjCh2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (Axis->UniPrms->Cprm->mnaz0_1.b.h & 0xFF);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************************************/
/*		0x050F : Analog monitor (CH1) Gain adjustment data												*/
/****************************************************************************************************/
PRM_RSLT uncal_AmonGadjCh1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (Axis->UniPrms->Cprm->mnag0_1.b.l & 0xFF);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************************************/
/*		0x0510 : Analog monitor (CH2) Gain adjustment data												*/
/****************************************************************************************************/
PRM_RSLT uncal_AmonGadjCh2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (Axis->UniPrms->Cprm->mnag0_1.b.h & 0xFF);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************************************/
/*		0x0511 : Current detection (UPhase) Zero adjustment data														*/
/****************************************************************************************************/
PRM_RSLT uncal_CurZadjUphase(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (SHORT)Axis->UniPrms->Prm->zadjiu;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************************************/
/*		0x0512 : Current detection (VPhase) Zero adjustment data														*/
/****************************************************************************************************/
PRM_RSLT uncal_CurZadjVphase(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (SHORT)Axis->UniPrms->Prm->zadjiv;
	return PRM_RSLT_SUCCESS;
}


/* <S056>	Start */
/****************************************************************************
0x513	Motor type
****************************************************************************/
PRM_RSLT UnReadMotorID(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Prm)->MencP.typm_e.b.l;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x514	Motor capacity
****************************************************************************/
PRM_RSLT UnReadMotorWattage(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)(Axis->UniPrms->Bprm)->MotW;
	x = MlibScalKskxkx( x, 1, 1, NULL, -30 );

	*pValue = (LONG)((ArrayIdx == 0) ? x : ((ULONG)x >> 16));

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x515	Encoder type
****************************************************************************/
PRM_RSLT UnReadEncoderType(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG x;
	x = (LONG)(Axis->UniPrms->Prm)->MencP.typm_e.b.h;
	*pValue = ((x >> 4) & 0x0F) - 1;		/* 2,1 --> 1,0					*/
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x516	YSpecification
****************************************************************************/
PRM_RSLT UnReadYspecVer(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG YspecVer;

	YspecVer = (LONG)SERVO_SPECIAL_SPEC_VER;

	*pValue = YspecVer;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x517	Servopack software version
****************************************************************************/
PRM_RSLT UnReadFirmVerBase(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG ServoFirmVer;

//	ServoFirmVer = (LONG)SERVO_FIRM_VER>>16;				/* <S0D1> */
	ServoFirmVer = (LONG)((USHORT)(SERVO_FIRM_VER>>16));	/* <S0D1> */

	*pValue = ServoFirmVer;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x518	Encoder software version
****************************************************************************/
PRM_RSLT UnReadEncoderVer(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UniPrms->Prm)->MencP.encver;
	return PRM_RSLT_SUCCESS;
}
/* <S056>	End */

/* <S049>	Start */
/****************************************************************************
0x51B	Servo status monitor
****************************************************************************/
PRM_RSLT UnReadSvStatus(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(Axis->UnMonitor->SvStatus);
	return PRM_RSLT_SUCCESS;
}
/* <S049>	End */

/****************************************************************************
0x51C	Control mode monitor	CtrlMode
****************************************************************************/
PRM_RSLT UnReadServoControlMode(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG   CtrlModex;
	CtrlModex = (LONG)Axis->BaseCtrlOut->CtrlModeOut.b.cm;
	switch(Axis->BaseCtrlOut->CtrlModeOut.b.cm)
	{
	case CTRLMODE_JOG:  /* JOG operation mode-Do*/
	case CTRLMODE_ZSRCH:/* Origin search */
		*pValue = CTRLMODE_SPD;
		break;

	case CTRLMODE_POS:  /* Position control mode */
	case CTRLMODE_SPD:  /* Speed control mode */
	case CTRLMODE_TRQ:  /* Torque control mode */
	default:            /* This case does not exist */
		*pValue = CtrlModex;
		break;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x52A	Alarm history timestamp	AlmTmStamp
****************************************************************************/
PRM_RSLT UnReadAlarmTimeStamp(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	ALMGetAlarmHistory(Axis->AlmManager, ArrayIdx, (ULONG*)pValue);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x700	Sampling time unit	SamplingTime
****************************************************************************/
PRM_RSLT UnReadSamplingTimeUnit(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = Axis->UniPrms->Bprm->SvCycleUs;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x701	Data is16384 MaxTorque
****************************************************************************/
PRM_RSLT UnReadMaxTorqueConvertValue(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	/* Maximum torque[%] */
	*pValue = (LONG)(Axis->UniPrms->Bprm->PerMaxTrq);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x702	Data is16384 MaxSpeed
****************************************************************************/
PRM_RSLT UnReadMaxSpeedConvertValue(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (FLOAT_USE==TRUE)
	*pValue = 0x01000000 * (LONG)Axis->UniPrms->Bprm->Kmonspd ;
#else
	*pValue = MlibMulgainSx48( 0x01000000, (LONG)Axis->UniPrms->Bprm->Kmonspd );
#endif /* FLOAT_USE */
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x704	Data is16384  MaxSpeed
****************************************************************************/
PRM_RSLT UnReadMaxSpeedExpConvertValue(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = 0;
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x705	Internal operating status	NewServoOpmode
****************************************************************************/
PRM_RSLT UnReadInternalServoMode(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	LONG Opmode = 0;
	MOTLESSTST		*MotorLessTest;	/* Motorless test modeIFDefinition		*//* <S1B7> */

	MotorLessTest = Axis->MotorLessTest;									/* <S1B7> */

	/* Bit0~7  :Preliminary */
	/* Bit8~11 : */
	if((Axis->UniPrms->Bprm)->FencUse != FALSE)
	{ /* Fulque closed */
		Opmode |= 0x00000100;
	}

	if( Axis->MencV->LowSpdMotor )
	{ /* DDMotor*/
		Opmode |= 0x00000800;
	}
	else
	{
		if( Axis->MencV->AxisMotType == MOTTYPE_ROTARY )
		{/* Rotation */
			Opmode |= 0x00000200;
		}
		if( Axis->MencV->AxisMotType == MOTTYPE_LINEAR )
		{/* Linear */
			Opmode |= 0x00000400;
		}
	}

	/* Bit12~15 */
	if( Axis->MencV->ScaleWriteEnable )
	{/* Scale write availability */
		Opmode |= 0x00002000;
	}

	/* Motorless test mode setting *//* <S1B7> */
	if( MotorLessTest->MotorLessTestMode == TRUE )
	{/* Motorless test mode */
		Opmode |= 0x00008000;
	}

	*pValue = Opmode;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0x710	RSW/DSWInput status
****************************************************************************/
PRM_RSLT UnReadHSWInputsStatus(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	ULONG rsw,dsw;
	dsw = (ULONG)HALdrv_GetDipSwitchSetting();
	rsw = (ULONG)HALdrv_GetStationAddress();
	*pValue = (LONG)(rsw | (dsw << 8));
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x712	FBBoard information
****************************************************************************/
PRM_RSLT UnReadFBboardID(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 0x712 is only INGRAM */
	*pValue = (LONG)HALdrv_ReadBoardID(HW_FBB_MODULE);
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x713	MBBoard information
****************************************************************************/
PRM_RSLT UnReadMBboardID(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 0x713 is only INGRAM */
	*pValue = (LONG)HALdrv_ReadBoardID(HW_MB_MODULE);
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x714	Converter load current(ADvalue)
****************************************************************************/
PRM_RSLT UnReadConverterCurrentAD(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 0x714 is only INGRAM */
	*pValue = HALdrv_GetConvCurFromAd();
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x716	Converter serial input data
****************************************************************************/
PRM_RSLT UnReadConverterSerialInputs(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if 0 /* 2012.09.07 Y.Oka for Mercury Base (only INGRAM) */
	CONV_INPUT CnvInputs;
	CnvInputs = HALdrv_ReadConverterSignals();
	*pValue = CnvInputs.dw;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x718	Unit internal temperature
****************************************************************************/
PRM_RSLT UnReadMbTemperature(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = 0;//OhmGetTemperature( &Axis->OhInfo[MBOH] );
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x719	Converter temperature
****************************************************************************/
PRM_RSLT UnReadCnvTemperature(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue =0;// OhmGetTemperature( &Axis->OhInfo[COAOH] );
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x71A	Control board temperature
****************************************************************************/
PRM_RSLT UnReadIfTemperature(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue =0;// OhmGetTemperature( &Axis->OhInfo[COAOH] );/* Interim processing	*/
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x720	(R)Boot software version
****************************************************************************/
PRM_RSLT UnReadBootsoftVersion(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)HALdrv_GetBootVersion( ArrayIdx, R_BOOTSOFT_VERSION );
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x724	(R)Bootstrap version
****************************************************************************/
PRM_RSLT UnReadBootstrapVersion(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (LONG)(HALdrv_GetBootVersion( ArrayIdx, R_BOOTSTRAP_VERSION ));
	return PRM_RSLT_SUCCESS;
}
/* <S154> Start */
/****************************************************************************
0x800 : Motor rotation/Movement speed [min-1, mm/s]
****************************************************************************/
PRM_RSLT UnReadAlmLtSpdFbk(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.MotSpd;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x801 : Speed command [min-1, mm/s]
****************************************************************************/
PRM_RSLT UnReadAlmLtSpdRef(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.SpdRef;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x802 : Internal torque/Thrust command [%]
****************************************************************************/
PRM_RSLT UnReadAlmLtTrqRef(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.TrqRef;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x803 : Input command pulse speed [min-1, mm/s]
****************************************************************************/
PRM_RSLT UnReadAlmLtPosRefSpd(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.PcmdSpd;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x804 : Deviation counter(Position deviation amount) [pulse]
****************************************************************************/
PRM_RSLT UnReadAlmLtPosErrCntr(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.PosErr;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x810 : Internal signal monitor
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInterSig(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.InterSignal ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x814 : Internal input signal monitor
****************************************************************************/
PRM_RSLT UnReadAlmLtMonInSig(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.InSignal ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x818 : Internal output signal monitor
****************************************************************************/
PRM_RSLT UnReadAlmLtMonOutSig(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.OutSignal ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x820 : Main circuit bus voltage [V]
****************************************************************************/
PRM_RSLT UnReadAlmLtDcVoltx(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.DcVoltage ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x821 : Current detection value [%]
****************************************************************************/
PRM_RSLT UnReadAlmLtIqRate(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.IFBMon ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x822 : Cumulative load rate [%(Period2ms)]
****************************************************************************/
PRM_RSLT UnReadAlmLtMotLoad2ms(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.MotLoad ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x823 : Regeneration load rate [%(Period2ms)]
****************************************************************************/
PRM_RSLT UnReadAlmLtRegerRate2ms(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.RgPower ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x824 : DB resistance power consumption [%(Period2ms)]
****************************************************************************/
PRM_RSLT UnReadAlmLtDbrxPower2ms(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.DbPower ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x825 : Maximum cumulative load factor [%]
****************************************************************************/
PRM_RSLT UnReadAlmLtMotLoad10s(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.MaxMotLoad10s ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x826 : Moment of inertia ratio/Weight ratio [%]
****************************************************************************/
PRM_RSLT UnReadAlmLtJLRatio(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.JLRatio ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x830 : Serial encoder communication error count [-]
****************************************************************************/
PRM_RSLT UnReadAlmLtSencComErrCnt(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.EncComErrCnt ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x831 : MECHATROLINKCommunication error count [-]
****************************************************************************/
PRM_RSLT UnReadAlmLtM2ComErrCnt(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.M2ComErrCnt ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x832 : Safety input/output signal monitor [-]
****************************************************************************/
PRM_RSLT UnReadAlmLtMonSafetyIOSig(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.SafetyIOSignal ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x833 : Alarm number [-]
****************************************************************************/
PRM_RSLT UnReadAlmLtAlmCode(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.AlmCode ;

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0x834 : Timestamp [-]
****************************************************************************/
PRM_RSLT UnReadAlmLtTmStamp(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	*pValue = Axis->hAlmLatchHndl->LatchMon.TmStamp ;

	return PRM_RSLT_SUCCESS;
}
/* <S154> End */
/* <S056>	Start */
/****************************************************************************
0xC00	Feedback position(APOS)
****************************************************************************/
PRM_RSLT UnReadApos(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

	*pValue = mt_hdl->psnHdl.Apos;

#endif

	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0xC02	Current command position(CPOS)
****************************************************************************/
PRM_RSLT UnReadCpos(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

	*pValue = mt_hdl->psnHdl.Pos;
#endif


	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC04	Position deviation(PERR)
****************************************************************************/
PRM_RSLT UnReadPerr(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

	*pValue = mt_hdl->psnHdl.Perr;
#endif
	return PRM_RSLT_SUCCESS;
}
/****************************************************************************
0xC06	Target position(TPOS)
****************************************************************************/
PRM_RSLT UnReadTpos(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

	*pValue = mt_hdl->psnHdl.Tpos;
#endif

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC08	Latch position 1(LPOS1)
****************************************************************************/
PRM_RSLT UnReadLpos1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)	/* M3IF */
	*pValue = mt_hdl->psnHdl.Lpos[0];
#else
	*pValue = mt_hdl->psnHdl.Lpos;		/* M2IF */
#endif

#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC0A	Latch position 2(LPOS2)
****************************************************************************/
PRM_RSLT UnReadLpos2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if(CSW_NETIF_TYPE == NETIF_M3_TYPE)	/* M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

	*pValue = mt_hdl->psnHdl.Lpos[1];

#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC0C	Latch position 3(LPOS3)
****************************************************************************/
PRM_RSLT UnReadLpos3(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)	/* M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

	*pValue = mt_hdl->psnHdl.Lpos[2];

#endif

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC0E	Target speed(TSPD)
****************************************************************************/
PRM_RSLT UnReadTspd(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

	*pValue = mt_hdl->cmdData.TSpeed;

#endif

	return PRM_RSLT_SUCCESS;

}

/****************************************************************************
0xC10	Feedback speed(FSPD)
****************************************************************************/
PRM_RSLT UnReadFbspd(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;

	*pValue = mt_hdl->psnHdl.Fbspd;

#endif


	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC12	Current position command speed(CSPD)
****************************************************************************/
PRM_RSLT UnReadCspd(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
	MT_HNDL		*mt_hdl;

	mt_hdl  = &Axis->NetAxisIf->mt_hdl;
	*pValue = mt_hdl->cmdData.Velocity;

#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC14	Torque limit value(TRQ_LIM)
****************************************************************************/
PRM_RSLT UnReadTrqLim(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */
/* TODO */
#endif

	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC16	Speed limit value(SPD_LIM)
****************************************************************************/
PRM_RSLT UnReadSpdLim(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )	/* M-2IF or M-3IF */

	SERVO_CONTROL_IF	*SvControlIf;

	SvControlIf		= Axis->SvControlIf;

	*pValue = SvControlIf->NetSpdLmt;
#endif

	return PRM_RSLT_SUCCESS;
}
/* <S056>	End */

/****************************************************************************
0xC30	M-ⅢCommunication phase
****************************************************************************/
PRM_RSLT UnReadM3CommPhase(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
	*pValue = Axis->NetAxisIf->com_hdl.Phase;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC31	M-ⅢCurrently executing command code(RCMD)
****************************************************************************/
PRM_RSLT UnReadM3MainCommandCode(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
	*pValue = Axis->NetAxisIf->cmd_hdl.cmdInfo.crtCmd;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xC32	M-ⅢCurrently executing subcommand code(SUB_RCMD)
****************************************************************************/
PRM_RSLT UnReadM3SubCommandCode(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
	EX_CMD_INFO *exCmdInfo;
	exCmdInfo = &(Axis->NetAxisIf->cmd_hdl.exCmdInfo);
	*pValue = exCmdInfo->cmdBuf.stdData.mon.cmd;
#endif
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
		UnMonitor Dummy function		<S020>
****************************************************************************/
PRM_RSLT UnReadDummy(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = 0;
	return PRM_RSLT_SUCCESS;
}


/****************************************************************************
0xF0000050	Input signal monitor	InSignal
****************************************************************************//* <S1C2> */
PRM_RSLT UnReadDigitalAllInputSignals1(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = (HALdrv_ReadMonAllInputSignals(  ) & 0x000000FF);
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xF0000051	Input signal monitor	InSignal
****************************************************************************//* <S1C2> */
PRM_RSLT UnReadDigitalAllInputSignals2(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = ((HALdrv_ReadMonAllInputSignals( )>>8) & 0x000000FF);
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xF0000052	Output signal monitor	OutSignal
****************************************************************************//* <S1C2> */
PRM_RSLT UnReadDigitalAllOutputSignals(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	*pValue = HALdrv_ReadMonAllOutputSignals( );
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************
0xF0000010	Output signal monitor	OutSignal                                 <S225> 
****************************************************************************/
PRM_RSLT UnMaxSpdOrMaxDiv(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	if( Axis->MencV->AxisMotType == MOTTYPE_LINEAR )
	{
		if( Axis->UniPrms->Prm->b_prm80 & 0xF000 )
		{/* Calculate maximum speed with fixed frequency division output setting */
			*pValue = Axis->MencV->DivInfo.MaxDivSpd;
		}
		else
		{/* Calculate divided output setting with maximum speed fixed */
			*pValue = Axis->MencV->DivInfo.MaxDivRat;
		}
		
	}
	else
	{
		*pValue = 0;
	}
	
	return PRM_RSLT_SUCCESS;
}


/******************************* End of File *******************************/

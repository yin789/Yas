/*****************************************************************************
Description		: Alarm latch function processing
-----------------------------------------------------------------------------
Author		: SHCORE,
			Copyright (c) 2013 All Rights Reserved

Project		: Mercury

Functions	  :
	-- APIs --
		AlmLatchMonitorInitial		Alarm latch initialization processing
		AlmLatchMonitorSequence		Alarm latch processing

	-- Locals --
		AlmLatchMonitorData			Create alarm latch monitor data
		AlmLatchMonDataWrite		Alarm latch monitor, EEPROM

-----------------------------------------------------------------------------
Changes		:
Name		Date			Description
-----------------------------------------------------------------------------
K.Liu		2013.10.11		created

*****************************************************************************/

#include "AlmLatch.h"
#if ( CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE )
#include "Sqi.h"
#endif
#define	MONSETBIT( sig, bit )	((sig)? (1<<bit) : 0x0000)
#if ( CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE )
#define LPX_CHECK_ON( psqi, sig )  (((psqi)->Sigs & (sig)))
#endif
#if ( CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE )
extern SQIV	SqiV;
#endif
/****************************************************************************************************/
/*																									*/
/*		APIFunction list																					*/
/*																									*/
/****************************************************************************************************/
void AlmLatchMonitorInitial( ALMLATCH_HNDL *hAlmLatchHndl );
void AlmLatchMonitorSequence( ALMLATCH_HNDL *hAlmLatchHndl );

/****************************************************************************************************/
/*																									*/
/*		LocalFunction list																				*/
/*																									*/
/****************************************************************************************************/
static void AlmLatchMonitorData( ALMLATCH_HNDL *hAlmLatchHndl );
//static void AlmLatchMonDataWrite( ALMLTMON *hAlmMonitor, ALMMAN_HANDLE *hAlmHndl );



/*****************************************************************************
  Description: Alarm latch initialization processing
------------------------------------------------------------------------------
  Parameters:
	hAlmLatchHndl			Alarm Latch handle

  Return:
	none
------------------------------------------------------------------------------
  Note:
*****************************************************************************/
void AlmLatchMonitorInitial( ALMLATCH_HNDL *hAlmLatchHndl )
{
	MlibResetLongMemory( &(hAlmLatchHndl->var), sizeof( hAlmLatchHndl->var )/4 );
	
	return ;
}



/*****************************************************************************
  Description: Alarm latch processing
------------------------------------------------------------------------------
  Parameters:
	hAlmLatchHndl			Alarm Latch handle

  Return:
	none
------------------------------------------------------------------------------
  Note:
*****************************************************************************/
void AlmLatchMonitorSequence( ALMLATCH_HNDL *hAlmLatchHndl )
{

/*--------------------------------------------------------------------------------------------------*/
/*		When the main circuit bus voltage falls below the UV level, no data update & write request is made					*/
/*--------------------------------------------------------------------------------------------------*/
	if ( hAlmLatchHndl->PowerManager->PowerSts.DcVolt >=
			hAlmLatchHndl->PowerManager->MainPowerChecker.conf.UvLevel )
	{
#if 0	/* <S212> */
		/*----------------------------------------------------------------------------------*/
		/*		Data update if no alarm is detected											*/
		/*----------------------------------------------------------------------------------*/
		if ( hAlmLatchHndl->AlmManager->Status.AlmFlag == FALSE )
		{
			AlmLatchMonitorData( hAlmLatchHndl );

		}
		/*----------------------------------------------------------------------------------*/
		/*		No previous alarm & current alarm detected,EEPROMRegistration to write queue			*/
		/*----------------------------------------------------------------------------------*/
		else if ( hAlmLatchHndl->var.LastAlmStatus == FALSE )
		{
#else	/* <S212> */
		if( hAlmLatchHndl->AlmManager->LtAlmReq == 4 )
		{
			hAlmLatchHndl->var.LastAlmStatus    = FALSE;
			hAlmLatchHndl->AlmManager->LtAlmReq = 0;
		}
		if( hAlmLatchHndl->var.LastAlmStatus == FALSE )
		{
			hAlmLatchHndl->var.LastAlmStatus = TRUE;

			AlmLatchMonitorData( hAlmLatchHndl );
#endif	/* <S212> */
			/*--------------------------------------------------------------------------*/
			/*		0x833 : Alarm number [-]											*/
			/*		0x834 : Timestamp [-]											*/
			/*--------------------------------------------------------------------------*/
#if 0	/* <S1CF>  Latch data is written. However, it is ignored during writing.	*/
			hAlmLatchHndl->AlmLtMon.AlmCode = ALMGetAlarmHistory( hAlmLatchHndl->AlmManager, 0,
										&hAlmLatchHndl->AlmLtMon.TmStamp );
//			AlmLatchMonDataWrite( &hAlmLatchHndl->AlmLtMon, hAlmLatchHndl->AlmManager );	  /* <S194> */

			/* Monitor copy */
			MlibCopyLongMemory( &(hAlmLatchHndl->LatchMon), &(hAlmLatchHndl->AlmLtMon),
								(sizeof(ALMLTMON) >> 2) );
#else
			if( hAlmLatchHndl->AlmManager->LtAlmReq == 0 )
			{
				/* Monitor copy */
				MlibCopyLongMemory( &(hAlmLatchHndl->LatchMon), &(hAlmLatchHndl->AlmLtMon),
																	(sizeof( ALMLTMON ) >> 2) );

				/* Write request */
				hAlmLatchHndl->AlmManager->LtRomAdr = EEP_NOSUM_ADDRESS( hAlmLatchHndl->AlmManager->myAsicNo, EEP_ALMMON_OFS );
				hAlmLatchHndl->AlmManager->LtMemAdr = (USHORT*)&( hAlmLatchHndl->LatchMon );
				hAlmLatchHndl->AlmManager->AlmCdAdr = (USHORT*)&( hAlmLatchHndl->LatchMon.AlmCode );
				hAlmLatchHndl->AlmManager->TmStpAdr = (ULONG *)&( hAlmLatchHndl->LatchMon.TmStamp );
				hAlmLatchHndl->AlmManager->LtDtWtCt = sizeof( ALMLTMON ) >> 1;
				hAlmLatchHndl->AlmManager->LtAlmReq = 1;
			}
#endif
		}
		else
		{
			/* Do Nothing */
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Alarm status storage																			*/
/*--------------------------------------------------------------------------------------------------*/
#if 0	/* <S212> */
	if ( hAlmLatchHndl->AlmManager->Status.AlmFlag == TRUE )
	{
		hAlmLatchHndl->var.LastAlmStatus = TRUE;
	}
	else
	{
		hAlmLatchHndl->var.LastAlmStatus = FALSE;
	}
#endif
	return;
}

/*****************************************************************************
  Description: Create alarm latch monitor data
------------------------------------------------------------------------------
  Parameters:
	hAlmLtUnitHndl		Alarm Latch Unit Handle

  Return:
	none
------------------------------------------------------------------------------
  Note:
*****************************************************************************/
static void AlmLatchMonitorData( ALMLATCH_HNDL *hAlmLatchHndl )
{
	LONG	lwk;
	LONG	val;
	ALMLTMON	*pAlmLtMon;
	PLSIFV		*pPls;
	
	pAlmLtMon	= &hAlmLatchHndl->AlmLtMon;
	pPls		= hAlmLatchHndl->pPlsIfV;
/*--------------------------------------------------------------------------------------------------*/
/*		0x800 : Motor rotation/Movement speed [min-1, mm/s]													*/
/*--------------------------------------------------------------------------------------------------*/
#if 0	/* <S1CF> */
	pAlmLtMon->MotSpd =
			(SHORT)MlibLimitul(hAlmLatchHndl->UnMonitor->UnMotSpdx, 0x00007FFF, 0xFFFF8000);
#else
	pAlmLtMon->MotSpd = hAlmLatchHndl->UnMonitor->UnMotSpd;
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		0x801 : Speed command [min-1, mm/s]																*/
/*--------------------------------------------------------------------------------------------------*/
#if 0	/* <S1CF> */
	val = MlibMulgainSx48( hAlmLatchHndl->UnMonitor->UnSpdRef, hAlmLatchHndl->Bprm->Kmonspd );
	pAlmLtMon->SpdRef = (SHORT)MlibLimitul(val, 0x00007FFF, 0xFFFF8000) ;
#else
	val = MlibMulgainSx48( hAlmLatchHndl->BaseControls->CtrlCmdMngr.SpdRefo, hAlmLatchHndl->Bprm->Kmonspd);
	pAlmLtMon->SpdRef = MlibLimitul(val, 0x00007FFF, 0xFFFF8000);
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		0x802 : Internal torque/Thrust command [%]																*/
/*--------------------------------------------------------------------------------------------------*/
	pAlmLtMon->TrqRef = (SHORT)( hAlmLatchHndl->UnMonitor->UnTrqRef );

/*--------------------------------------------------------------------------------------------------*/
/*		0x803 : Input command pulse speed [min-1, mm/s]													*/
/*--------------------------------------------------------------------------------------------------*/
#if 0	/* <S1CF> */
	val = MlibMulgain( hAlmLatchHndl->SeqMotSts->var.dPcmdCnt,
							hAlmLatchHndl->Bprm->Kmonpcmdspd2ms );
	pAlmLtMon->PcmdSpd = (SHORT)MlibLimitul(val, 0x00007FFF, 0xFFFF8000) ;
#else
	pAlmLtMon->PcmdSpd = hAlmLatchHndl->UnMonitor->UnPcmd64ms;
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		0x804 : Deviation counter(Position deviation amount) [pulse]													*/
/*--------------------------------------------------------------------------------------------------*/
	pAlmLtMon->PosErr = hAlmLatchHndl->BaseLoops->BaseCtrls->PosManager.var.PosErrA.per32s;

/*--------------------------------------------------------------------------------------------------*/
/*		0x810 : Internal signal monitor [-]																	*/
/*--------------------------------------------------------------------------------------------------*/
	pAlmLtMon->InterSignal = (
								MONSETBIT( hAlmLatchHndl->SeqCtrlOut->MainPowerOn,	0	) |			/* Main circuitONStatus */
								MONSETBIT( hAlmLatchHndl->MencV->EncReady,					1	) |	/* Encoder Ready */
								MONSETBIT( hAlmLatchHndl->BeSequence->DbSeqData.DbBeReq,	2	) |	/* Motor energization(Request) */
								MONSETBIT( hAlmLatchHndl->BeSequence->DbOn,					3	) |	/* DB ON */
								MONSETBIT( hAlmLatchHndl->Bprm->RvsDir,						4	) |	/* Reverse rotation mode */
								MONSETBIT( hAlmLatchHndl->BaseControls->ModeSwData.var.ModeSWFlag,	5	) |	/* Mode switchON Flag	 */
								MONSETBIT( hAlmLatchHndl->SeqCtrlOut->f.Vrefon,				6	) |	/* Speed  Reference Input On */
								MONSETBIT( hAlmLatchHndl->SeqCtrlOut->f.Trefon,				7	) |	/* Torque Reference Input On */
								MONSETBIT( hAlmLatchHndl->SeqCtrlOut->f.Prefon,				8	) |	/* Torque Reference Input On */
								MONSETBIT( hAlmLatchHndl->SeqCtrlOut->f.PrefSign,			9	) |	/* Pulse  Reference Sign  (plus:1, minus:0) */
								MONSETBIT( hAlmLatchHndl->SeqCtrlOut->f.PerrClr,			10	) |	/* PosErr Clear Input On */
								MONSETBIT( 0,							11	) |	/*MONSETBIT( BoutV.f.BiasONFlag,	11	),0Fixed */
								MONSETBIT( hAlmLatchHndl->PowerManager->PowerSts.Acon,		12	) |	/* ACPower supply detection status */
								MONSETBIT( hAlmLatchHndl->PowerManager->PowerSts.ChargeOk,	13	) |	/* DCCharging complete state*/
								MONSETBIT( HALdrv_GetRegeneControlState(),					14	) |	/* RegenerationTrCommand on */
								MONSETBIT( hAlmLatchHndl->PowerManager->RegeneCtrl.RegChk.RegeAlmSignal,	15	) |	/* Regeneration error detection on */
								MONSETBIT( hAlmLatchHndl->BeSequence->BeReqSeqOut,			16	) |	/* BaseEnable Request(Sequence processing output) */
								MONSETBIT( 0,												17	) |
								MONSETBIT( ALMCheckEachState(hAlmLatchHndl->AlmManager, ALM_OC),			18	) |	/* Overcurrent */
								MONSETBIT((hAlmLatchHndl->MencV->RxAlmCode & 0x0040),		19	) |	/* Origin point not passed complete */
								MONSETBIT( hAlmLatchHndl->BaseControls->JatHdl.jatv.enable,	20	) |	/* Inertia identification in progress */
								MONSETBIT( (hAlmLatchHndl->BeSequence->BeReqSeqOut != FALSE)
										&& (hAlmLatchHndl->MencV->PhaseReady == FALSE),		21	) |	/* Magnetic pole detection in progress */
								MONSETBIT( hAlmLatchHndl->MencV->PhaseReady,				22	) 	/* Complete magnetic pole recognition */
								);

/*--------------------------------------------------------------------------------------------------*/
/*		0x814 : Internal input signal monitor [-]																*/
/*--------------------------------------------------------------------------------------------------*/
	KPX_READ_PCMD_STATUS( lwk );
	pAlmLtMon->InSignal = (
							MONSETBIT( (0 != (hAlmLatchHndl->BaseCtrlOut->CmdSeqBit & SERVO_ON_BIT)),	0	) |	/* Servo on */
							MONSETBIT( (0 != (hAlmLatchHndl->BaseCtrlOut->CmdSeqBit & ENBPCTRL_BIT)),	1	) |	/* P/PISwitch */
							MONSETBIT( hAlmLatchHndl->SeqCtrlOut->PotSigSts,							2	) |	/* Forward rotation drive prohibited state */
							MONSETBIT( hAlmLatchHndl->SeqCtrlOut->NotSigSts,							3	) |	/* Reverse drive prohibited state */
							MONSETBIT( hAlmLatchHndl->AlmManager->AlmRst,								4	) |	/* ALMReset request */
							MONSETBIT( hAlmLatchHndl->BaseControls->TrqLimitData.var.PclSignal ,		5	) |	/* PCL Signal */
							MONSETBIT( hAlmLatchHndl->BaseControls->TrqLimitData.var.NclSignal,			6	) |	/* PCL Signal */
							MONSETBIT( (0 != (hAlmLatchHndl->BaseCtrlOut->CmdSeqBit & SENSOR_ON_BIT)),	7	) |	/* Sensor on	*/
#if ( CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE )
							/* Internal setting speed control */
							MONSETBIT( LPX_CHECK_ON( &SqiV, SQI_SIGS_SPDD ),		8	) |	/* SPD-D Signal */
							MONSETBIT( LPX_CHECK_ON( &SqiV, SQI_SIGS_SPDA ),		9	) |	/* SPD-A Signal */
							MONSETBIT( LPX_CHECK_ON( &SqiV, SQI_SIGS_SPDB ),		10	) |	/* SPD-B Signal */
							/* Control method switching */
							MONSETBIT( LPX_CHECK_ON( &SqiV, SQI_SIGS_CSEL ),		11	) |	/* C-SEL Signal */
#else
							MONSETBIT( 0,		8	) |
							MONSETBIT( 0,		9	) |
							MONSETBIT( 0,		10	) |
							MONSETBIT( 0,		11	) |
#endif
							MONSETBIT( hAlmLatchHndl->BaseCtrlOut->CmdSeqBit & ZEROCLAMP_BIT,	12	) |	/* Zero Clamp */
#if ( CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE )
							MONSETBIT( LPX_CHECK_ON( &SqiV, SQI_SIGS_INHIBIT ),					13	) |	/* INHIBIT Signal */
#else
							MONSETBIT( 0,		13	) |
#endif
							MONSETBIT( hAlmLatchHndl->BaseControls->GselNum & 0x03,				14	) |	/* Gain number for manual gain switching(bit14,15) */
							MONSETBIT( (0 != (hAlmLatchHndl->BaseCtrlOut->CmdSeqBit & POL_DETECT_BIT)),		16	) |	/* P-DET Signal */
							MONSETBIT( hAlmLatchHndl->IoSignals->EstpSignal.SignalStatus.v,		17	) |	/* E-Stop Signal *//* <S045> */
							MONSETBIT( hAlmLatchHndl->IoSignals->DecSignal.SignalStatus.v,		18	) |	/* E-Stop Signal *//* <S045> */
							/* External latch internal input status(Ext1,Ext2,Ext3) */
							MONSETBIT( hAlmLatchHndl->IoSignals->ExtSignal[0].SignalStatus.v,	19	) | /* <S045> */
							MONSETBIT( hAlmLatchHndl->IoSignals->ExtSignal[1].SignalStatus.v,	20	) | /* <S045> */
							MONSETBIT( hAlmLatchHndl->IoSignals->ExtSignal[2].SignalStatus.v,	21	) | /* <S045> */

							MONSETBIT( KPX_CHECK_PCMD_CADAT( lwk ),	24	) |
							MONSETBIT( KPX_CHECK_PCMD_CBDAT( lwk ),	25	) |
							MONSETBIT( hAlmLatchHndl->BaseControls->CtrlCmdMngr.PerrClrReq,		26	) |	/* Position deviation clear request(Table operation/PJOGDedicated) */
#if ( CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE )
							MONSETBIT( LPX_CHECK_ON( &SqiV, SQI_SIGS_PSEL ),					27	)	/* Command pulse input switching(/PSEL) */
#else
							MONSETBIT( 0,		27	)
#endif
							);

/*--------------------------------------------------------------------------------------------------*/
/*		0x818 : Internal output signal monitor [-]																*/
/*--------------------------------------------------------------------------------------------------*/
	val = 0;
	if(hAlmLatchHndl->Bprm->FencUse != FALSE)
	{
		val = hAlmLatchHndl->FencV->CposOk & (hAlmLatchHndl->FencV->MposFromCpos == 0);
	}
	else
	{
		val = hAlmLatchHndl->MencV->CposOk & (hAlmLatchHndl->MencV->MposFromCpos == 0);
	}
	
	pAlmLtMon->OutSignal = (
							MONSETBIT( hAlmLatchHndl->AlmManager->Status.AlmFlag,			0	) |	/* Alarm occurrence */
							MONSETBIT( hAlmLatchHndl->BaseControls->PosCtrlSts.CoinSignal,	1	) |	/* Positioning complete */
							MONSETBIT( hAlmLatchHndl->SeqCtrlOut->VcmpSts,					2	) |	/* Speed match */
							MONSETBIT( hAlmLatchHndl->SeqCtrlOut->TgOnSts,					3	) |	/* While the motor is rotating */
							MONSETBIT( hAlmLatchHndl->BeSequence->ServoReady,				4	) |/* Servo ready */
							/* Torque is being limited */
							MONSETBIT( ((hAlmLatchHndl->SvControlIf->RspCtrl.RspCtrlBit >> TRQ_CLAMP_BITNO) & 1) ,	5	) |
							/* Speed limit */
							MONSETBIT( ((hAlmLatchHndl->SvControlIf->RspCtrl.RspCtrlBit >> SPD_CLAMP_BITNO) & 1),	6	) |
							MONSETBIT( (!(hAlmLatchHndl->SeqCtrlOut->BrkSigSts)),			7	) |	/* Brake released */
							MONSETBIT( hAlmLatchHndl->AlmManager->Status.WrnFlag,			8	) |	/* Warning is occurring */
							MONSETBIT( hAlmLatchHndl->BaseControls->PosCtrlSts.NearSignal,	9	) |	/* Near Signal */
							MONSETBIT( val,													10	) |	/* PgCphase Signal */
							/* Alarm code signal output	*/
							MONSETBIT( hAlmLatchHndl->AlmManager->AlmCodeOut.Alo1,			16	) |
							MONSETBIT( hAlmLatchHndl->AlmManager->AlmCodeOut.Alo2,			17	) |
							MONSETBIT( hAlmLatchHndl->AlmManager->AlmCodeOut.Alo3,			18	) |
						/*	((BinK.DivPulseSig & 0x07)			 << 19  ) |		*//*Division pulse output not supported*/
#if ( CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE )
							MONSETBIT( LPX_CHECK_ON( &SqiV, SQI_SIGS_PSEL ),				22	)	/* Command pulse input switching response(/PSELA ) */
#else
							MONSETBIT( 0 /*BoutV.f.Sv.PselA */,			22	)
#endif
							);

/*--------------------------------------------------------------------------------------------------*/
/*		0x820 : Main circuit bus voltage [V]																	*/
/*--------------------------------------------------------------------------------------------------*/
#if ( FLOAT_USE == TRUE )
	pAlmLtMon->DcVoltage = hAlmLatchHndl->PowerManager->PowerSts.DcVolt *
							hAlmLatchHndl->PowerManager->DetectDcVolt.conf.Kdcvolt;
#else
	pAlmLtMon->DcVoltage = MlibMulgain(hAlmLatchHndl->PowerManager->PowerSts.DcVolt,
								hAlmLatchHndl->PowerManager->DetectDcVolt.conf.Kdcvolt );
#endif /* FLOAT_USE */
	
/*--------------------------------------------------------------------------------------------------*/
/*		0x821 : Current detection value [%]																		*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	pAlmLtMon->IFBMon = hAlmLatchHndl->UnMonitor->UnCrntDet *
						hAlmLatchHndl->Bprm->Kmoncrnt;
#else
	pAlmLtMon->IFBMon =
			MlibMulgain( hAlmLatchHndl->UnMonitor->UnCrntDet, hAlmLatchHndl->Bprm->Kmoncrnt );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		0x822 : Cumulative load rate [%(Period2ms)]																*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	pAlmLtMon->MotLoad = hAlmLatchHndl->UnMonitor->UnCrntRef *
						hAlmLatchHndl->Bprm->Kmoncrnt;
#else
	pAlmLtMon->MotLoad =
			MlibMulgain( hAlmLatchHndl->UnMonitor->UnCrntRef, hAlmLatchHndl->Bprm->Kmoncrnt );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		0x823 : Regeneration load rate [10%(Period2ms)]															*/
/*--------------------------------------------------------------------------------------------------*/
	pAlmLtMon->RgPower = hAlmLatchHndl->UnMonitor->UnRgPower;

/*--------------------------------------------------------------------------------------------------*/
/*		0x824 : DB resistance power consumption [%(Period2ms)]														*/
/*--------------------------------------------------------------------------------------------------*/
	pAlmLtMon->DbPower = hAlmLatchHndl->UnMonitor->UnDbPower;

/*--------------------------------------------------------------------------------------------------*/
/*		0x825 : Maximum cumulative load factor [%]																	*/
/*--------------------------------------------------------------------------------------------------*/
	pAlmLtMon->MaxMotLoad10s = (USHORT)hAlmLatchHndl->UnMonitor->UnIsqrSumMax ;

/*--------------------------------------------------------------------------------------------------*/
/*		0x826 : Moment of inertia ratio/Weight ratio [%]															*/
/*--------------------------------------------------------------------------------------------------*/
	/*	Stores the inertia ratio used in control	*/
//	pAlmLtMon->JLRatio = hAlmLatchHndl->BaseControls->JatHdl.EstimatJrat;
	if( hAlmLatchHndl->BaseControls->JatHdl.jatv.enable == TRUE )		/* Inertia identification enabled		*/
	{
		pAlmLtMon->JLRatio = hAlmLatchHndl->BaseControls->JatHdl.EstimatJrat;
	}
	else
	{
		pAlmLtMon->JLRatio = hAlmLatchHndl->UniPrm->Prm->jrate;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		0x830 : Serial encoder communication error count [-]													*/
/*--------------------------------------------------------------------------------------------------*/
	pAlmLtMon->EncComErrCnt = hAlmLatchHndl->MencV->ComErrCnt;

/*--------------------------------------------------------------------------------------------------*/
/*		0x832 : Safety input/output signal monitor [-]														*/
/*--------------------------------------------------------------------------------------------------*/
	/* Without safety option card */
	if( hAlmLatchHndl->SftOpt->conf.OpInfo.f.Connect == TRUE )
	{
		/* With safety option card */
		pAlmLtMon->SafetyIOSignal = (
									MONSETBIT( FALSE,					0	) |
									MONSETBIT( FALSE,					1	) |
									MONSETBIT( FALSE,					2	) |
									MONSETBIT( FALSE,					3	) |
									MONSETBIT( FALSE,					4	) |
									MONSETBIT( FALSE,					5	) );
	}
	else
	{
		/* Without safety option card */
		pAlmLtMon->SafetyIOSignal = (
									MONSETBIT( hAlmLatchHndl->IoSignals->DetHwbbSts.Bbon1,	0	) |
									MONSETBIT( hAlmLatchHndl->IoSignals->DetHwbbSts.Bbon2,	1	) |
									MONSETBIT( FALSE,					2	) |
									MONSETBIT( FALSE,					3	) |
									MONSETBIT( FALSE,					4	) |
									MONSETBIT( FALSE,					5	) );
	}

	return;
}

#if 0
/*****************************************************************************
  Description: Alarm latch monitor, EEPROM
------------------------------------------------------------------------------
  Parameters:
	hAlmMonitor			Alarm latch monitor pointer
	hAlmHndl			Alarm data pointer

  Return:
	none
------------------------------------------------------------------------------
  Note:
*****************************************************************************/
static void AlmLatchMonDataWrite( ALMLTMON *hAlmMonitor, ALMMAN_HANDLE *hAlmHndl )
{
	ULONG *pMonitorVal;
	ULONG MonitorVal;
	ULONG Offset;
	USHORT MonitorSize;		/* Alarm latch monitor size */
	USHORT i;

	pMonitorVal = (ULONG *)(hAlmMonitor);
	/* Calculate alarm latch monitor size value */
	MonitorSize = EEPSIZE_ALMMON >> 2 ;

	for(i = 0U; i < MonitorSize; i++)
	{
		/* Write Alarm Latch Monitor to EEPROM */
		Offset = EEP_NOSUM_ADDRESS( hAlmHndl->myAsicNo, EEP_ALMMON_OFS ) + (i << 2);
		MonitorVal = *(pMonitorVal + i);	/* Get alarm latch monitor value */
		EepdevPutQueue(&hAlmHndl->EepQue, 0, Offset, MonitorVal, 4U );
	}

	return ;
}
#endif



/***************************************** end of file **********************************************/

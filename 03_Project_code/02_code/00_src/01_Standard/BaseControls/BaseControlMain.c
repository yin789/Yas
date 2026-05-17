/****************************************************************************************************/
/*																									*/
/*																									*/
/*		BaseControlMain.c : Servo base control main processing definition									*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
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
#include "BaseControlMain.h"
#include "SerialEncIf.h"
#include "PosManager.h"
#include "ExControls.h"
#include "GainChange.h"
#include "MotorPoleFind.h"
#include "TableDrive.h"


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void	InitScanBData( AXIS_HANDLE* );															*/
/*	void	PrstScanBData( AXIS_HANDLE* );															*/
/*	void	KpxInputScanB( AXIS_HANDLE* );															*/
/*	void	BpiScanBMain( AXIS_HANDLE* );															*/
/*	void	KpxOutputScanB( AXIS_HANDLE* );															*/
/****************************************************************************************************/


/****************************************************************************************************/
/*																									*/
/*		Initialize ScanB 																			*/
/*																									*/
/****************************************************************************************************/
void	InitScanBData( AXIS_HANDLE *AxisRscB )
{
//	MENCV				*MencV;
	BASE_CTRL_HNDL		*BaseControls;
	SERVO_CONTROL_IF	*SvControlIf;
//	POS_MNG_HNDL		*PosManager;

	/* Local variable initialization */
//	MencV	= AxisRscB->MencV;
	BaseControls	= AxisRscB->BaseControls;
	SvControlIf		= AxisRscB->SvControlIf;
//	PosManager		= &(BaseControls->PosManager);


/*--------------------------------------------------------------------------------------------------*/
/*		Pointer initialization																		 <S004> */
/*--------------------------------------------------------------------------------------------------*/
	BaseControls->MencV = AxisRscB->MencV;
	BaseControls->FencV = AxisRscB->FencV;															/* <S04D> */
	
/*--------------------------------------------------------------------------------------------------*/
/*		Motor status initialization																	*/
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( &(BaseControls->MotSts), sizeof(BaseControls->MotSts)/4 );

/*--------------------------------------------------------------------------------------------------*/
/*		Servo basic control variable initialization																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Position control variable initialization */
//	MlibResetLongMemory( &(AxisRscB->hPosCtrl->var), sizeof(AxisRscB->hPosCtrl->var)/4 );
	/* Speed control variable initialization */
//	MlibResetLongMemory( &(AxisRscB->hSpdCtrl->var), sizeof(AxisRscB->hSpdCtrl->var)/4 );
	/* Torque filter variable initialization */
//	MlibResetLongMemory( &(BaseControls->TrqFilter.var), sizeof(BaseControls->TrqFilter.var)/4 );
	/* Zero speed stop control variable initialization */
	MlibResetLongMemory( &(BaseControls->ZeroClampCtrl.var), sizeof(BaseControls->ZeroClampCtrl.var)/4 );
	/* Servo gain switching variable initialization */
	GselInitServoGainChange( &(BaseControls->GainChange) );
#if (CSW_SVCF_EHVOBS_USE != 0)
	/* Phase compensation speed observer variable initialization */
	EhSpeedObserverInitialize( &(BaseControls->EhVobsCtrl) );
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Servo control position management initialization																	*/
/*--------------------------------------------------------------------------------------------------*/
#if (CSW_SVCF_BACKLASHCOMP != 0)
	/* Backlash correction disabled */
	BlshInitializeCompensate( &(BaseControls->PosManager.BlshCmp) );
#endif
	/* Position management variable initialization process */
	PosMngInitPositionManager( BaseControls );
	InitBasePositionControl( &(AxisRscB->BaseLoops->PosCtrl) );		/* <S14C> */
	
	/* Position command difference output[Pulse/Scan] */
	BaseControls->CtrlCmdMngr.dPosRefo = 0;
/* <S1D0> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Variable initialization for speed feedforward moving average function														*/
/*--------------------------------------------------------------------------------------------------*/
	FFCmdFilInitialize( &(BaseControls->VFFcmdFil) );

/*--------------------------------------------------------------------------------------------------*/
/*		Torque feedforward moving average function variable initialization														*/
/*--------------------------------------------------------------------------------------------------*/
	FFCmdFilInitialize( &(BaseControls->TFFcmdFil) );
/* <S1D0> End */
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation execution process initialization																	*/
/*--------------------------------------------------------------------------------------------------*/
	TblDriveInitialize( &(BaseControls->TblDrive), AxisRscB->TableRefBuffer, AxisRscB->TrcHndl );

/*--------------------------------------------------------------------------------------------------*/
/*		Servo expansion control variable initialization																	*/
/*--------------------------------------------------------------------------------------------------*/
#if (CSW_SVCF_DISTURBOBS != 0)
	/* Disturbance observer variable initialization <S09B> */
	DisturbObserverInitialize( &(BaseControls->DobsCtrl), &(BaseControls->GainChange.GselDobs[0]) );
#endif

	/* Adjustment-less related variable initialization */
	TuneLessCtrlInitialize( &(BaseControls->TuneLessCtrl) );

#if( CSW_SVCF_VIB_DETECT_USE != 0 )
	/* Initialize variables for vibration detection calculation */
	DetVibObsInitialize( AxisRscB->DetVib );
#endif

#if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 )
	/* Initializing damping control variables */
	ResVibATypeResVibInitialize( &(BaseControls->ResVib) );
#endif

	/* Speed deviation trace initialization using vibration detection observer */
	DetVibTraceInitialize( AxisRscB->DetVib );


	/* NETControl code variable initialization */
	SvControlIf->NetCtrlMcmd = CTRLMODE_NOCMD;
	BaseControls->TrqLimitData.var.FwTrqLmt = NORMAXVALUE;
	BaseControls->TrqLimitData.var.RvTrqLmt = NORMAXVALUE;

/*--------------------------------------------------------------------------------------------------*/
/*		Initialization of settling time measurement related process variables																*/
/*--------------------------------------------------------------------------------------------------*/
	/* Initialize Settling Time Variable and Start Settling Time Process */
	StlgInitSettlingTime( &(AxisRscB->BaseControls->SettlingTime),
						  AxisRscB->UniPrms->Bprm->SvCycleUs );

/*--------------------------------------------------------------------------------------------------*/
/*		Control parameter conversion service initialization															*/
/*--------------------------------------------------------------------------------------------------*/
	TranlateCtrlPrmInitialize( &(AxisRscB->BaseControls->TransduceCtrl) );

/*--------------------------------------------------------------------------------------------------*/
/*		Initialize other uninitialized variables(Interim)																*/
/*--------------------------------------------------------------------------------------------------*/
	AxisRscB->PowerManager->MainPowerChecker.conf.TestMode1 = FALSE;
	AxisRscB->PowerManager->MainPowerChecker.conf.TestMode2 = FALSE;
	AxisRscB->PowerManager->MainPowerChecker.conf.TestMode3 = FALSE;
	AxisRscB->PowerManager->MainPowerChecker.conf.TestMode4 = FALSE;
	AxisRscB->PowerManager->MainPowerChecker.conf.TestMode5 = FALSE;

	AxisRscB->SvControlIf->NetTrqFFC = 0;
	AxisRscB->SvControlIf->NetSpdFFC = 0;
}




/****************************************************************************************************/
/*																									*/
/*		ScanB Main Program																			*/
/*																									*/
/****************************************************************************************************/
void	BpiScanBMain( AXIS_HANDLE *AxisRscB, LONG ax_no )/* <S1C0> */
{
//	LONG				lwk;/*<S0C7>*/
	ALMID_T				AlarmCode;
	BOOL				PoleDetectReq;
	MPFEXEINFO			MpfExeCondition;
//	ASIC_U_IF_STR		*pAsicMicroReg;
	MENCV				*MencV;
	BASE_CTRL_HNDL		*BaseControls;
	SERVO_CONTROL_IF	*SvControlIf;
	DETVIB				*DetVib;
	CTRL_LOOP_OUT		*CtrlLoopOut;
	SEQ_CTRL_OUT		*SeqCtrlOut;
	MPFIND				*MpFind;		/* <S02F> */
	ANLG_UN_MON_DATA	*UnMonitor;
#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE
	SEQ_IO_HNDL 		*IoSignals;		/* <S045> */
#endif
	BPRMDAT				*Bprm;					/* <S04D> */
	BASE_LOOPCTRLS		*BaseLoops;

#if( CSW_SVCF_VIB_DETECT_USE != 0 )
	FUN_CMN_CONTROL		*FnCmnCtrl;

	FnCmnCtrl		= AxisRscB->FnCmnCtrl;
#endif
//	pAsicMicroReg	= AxisRscB->SvAsicRegs->AsicMicroReg;
	MencV			= AxisRscB->MencV;
	BaseControls		= AxisRscB->BaseControls;
	SvControlIf		= AxisRscB->SvControlIf;
	DetVib			= AxisRscB->DetVib;
	CtrlLoopOut		= AxisRscB->CtrlLoopOut;
	SeqCtrlOut		= AxisRscB->SeqCtrlOut;
	MpFind			= AxisRscB->MpFind;		/* Uncommented out<S02F> */
	UnMonitor		= AxisRscB->UnMonitor;
#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE
	IoSignals 		= AxisRscB->IoSignals;	/* <S045> */
#endif
	Bprm			= AxisRscB->UniPrms->Bprm;	/* <S04D> */
	BaseLoops		= AxisRscB->BaseLoops;

#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
/*--------------------------------------------------------------------------------------------------*/
/*		Set Encoder Error Alarm																		*/
/*--------------------------------------------------------------------------------------------------*/
	lwk = (LONG)AxisRscB->SvAsicRegs->MicroBitDat.data;
	if( lwk & BITDAT_ABF3ALM )
	{ /* A.BF3 : ScanASynchronization error */
		ALMSetServoAlarm( AxisRscB->AlmManager, ALM_SCANA_STOP );
	}
	if( lwk & BITDAT_AC90ALM )
	{ /* A.C90 : Encoder communication error */
		ALMSetServoAlarm( AxisRscB->AlmManager, ALM_PGCOM );
	}
	if( lwk & BITDAT_AC91ALM )
	{ /* A.C91 : Encoder acceleration error */
		ALMSetServoAlarm( AxisRscB->AlmManager, ALM_PGCOM1 );
	}
	if( lwk & BITDAT_AC20ALM )
	{ /* A.C20 : ScanASynchronization error */
		ALMSetServoAlarm( AxisRscB->AlmManager, ALM_PHASE );
	}
	if( lwk & BITDAT_AC21ALM )
	{ /* A.C21 : ScanASynchronization error */
		ALMSetServoAlarm( AxisRscB->AlmManager, ALM_POLESENSOR );
	}
#endif
	SvControlIf->CmdReady = MencV->PhaseReady;

/*--------------------------------------------------------------------------------------------------*/
/*		Control parameter transfer process																	*//* <S066> */
/*--------------------------------------------------------------------------------------------------*/
	/* Control Parameter Translate */
	if( TR_REQ_SCANB == TranlateCtrlGetReq(&(BaseControls->TransduceCtrl)) )
	{
		TranlateCtrlPrm( &(BaseControls->TransduceCtrl) );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Control command input processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	BpiInputControlCMD(BaseControls, SvControlIf, CtrlLoopOut,
						SeqCtrlOut, ((AxisRscB->UniPrms->Prm->b_prm0>>4) & 0x0F), ax_no);/* <S1C0>:Add axis */

/*--------------------------------------------------------------------------------------------------*/
/* 		<S045> H/WPCL/NCLSignal input(Erase if unnecessary) 											*/
/*--------------------------------------------------------------------------------------------------*/
#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE
	BaseControls->TrqLimitData.var.PclSignal |= IoSignals->PclSignal.SignalStatus.v;
	BaseControls->TrqLimitData.var.NclSignal |= IoSignals->NclSignal.SignalStatus.v;
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Scan B servo control processing																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->PhaseReady == FALSE )
	{
	/*----------------------------------------------------------------------------------------------*/
	/*		Magnetic pole detection control processing																		*/
	/*----------------------------------------------------------------------------------------------*/
		/* Position Reference for Motor Pole Find */
		BpiMpoleFindMakePosCmd( BaseLoops, SvControlIf, AxisRscB->FnCmnCtrl, Bprm );		/*<S02F>*/

		/* Prepare Motor Pole Find */
		/* Change the moving average time constant for motor speed detection */
		BpiChangeSpdDetMaFil( AxisRscB->MotSpdMafil, 4 );		/*<S02F>*/

		/* Tuning Less Invalid Setting */
		BaseControls->TuneLessCtrl.var.TuneLessAct = FALSE;
		BaseControls->CtrlCmdMngr.TrqFilDisable = TRUE;

		/* Get Motor Pole Find Execution Condition */
		MpfExeCondition.HaltCondition = (!SeqCtrlOut->SvRdyPhaseIgnore)
										| SeqCtrlOut->PotSigSts | SeqCtrlOut->NotSigSts;
		MpfExeCondition.MotStopSts = SeqCtrlOut->MotStop;
		MpfExeCondition.dMotPosition = BaseControls->MotSts.dMotPos;
		MpfExeCondition.TrefMonitor = CtrlLoopOut->TrqRefo_a;
		MpfExeCondition.SpdFbMonitor = BaseControls->CtrlCmdMngr.SpdFbki;
		MpfExeCondition.KVfbFil = BaseControls->CtrlCmdPrm.KVfbFil;		/*<S02F>*/
		MpfExeCondition.Kphasepos = MencV->Kphasepos;
		MpfExeCondition.Kmovpos = MencV->Kmovpos;

//		PdetServoOnMake( SvControlIf, FnCmnCtrl, MencV, AxisRscB->AlmManager->Status.AlmFlag );	/* <S21C> */
		AlarmCode = PdetServoOnMake( SvControlIf, FnCmnCtrl, MencV, AxisRscB );	/* <S226> */
		if( AlarmCode != NO_ALARM )																	/* <S226> */
		{
			ALMSetServoAlarm( AxisRscB->AlmManager, AlarmCode );
		}																							/* <S226> */

		/* Get Pole Detect Request */
//		PoleDetectReq = FnCmnCtrl->PdetReq | ((SvControlIf->CmdSeqBit >> POL_DETECT_BITNO) & 0x1);
		PoleDetectReq = FnCmnCtrl->SvOnPdetReq | ((SvControlIf->CmdSeqBit >> POL_DETECT_BITNO) & 0x1);	/* <S21C> */


		/* Motor Pole Find Main Operation */
		AlarmCode = MpoleFindSequence( MpFind,
									   &MpfExeCondition,
									   PoleDetectReq, BaseControls->BeComplete);
		if( AlarmCode != NO_ALARM )
		{
			ALMSetServoAlarm( AxisRscB->AlmManager, AlarmCode );
		}

		/* Termination Process for Motor Pole Find */
		if( MpFind->var.Step == MPF_END )
		{
			if(FALSE != PdetStartPhaseOffsetWrite(AxisRscB->PdetAbsSeq,
												MencV, MpFind->MpfPhaseOffset))
			{
				/* Return the moving average time constant for motor speed detection */
				if ( BaseControls->TuneLessCtrl.conf.TuningLessUse == TRUE )
				{
					BpiChangeSpdDetMaFil( AxisRscB->MotSpdMafil, 0 );		/*<S02F>*/
				}
				else
				{
					BpiChangeSpdDetMaFil( AxisRscB->MotSpdMafil, -1 );		/*<S02F>*/
				}

				/* Turn Back the Tuning Less Setting */
				BaseControls->TuneLessCtrl.var.TuneLessAct = BaseControls->TuneLessCtrl.conf.TuningLessUse;

				/* Motor Pole Find Complete */
				MencV->PhaseReady = TRUE;
				BaseControls->CtrlCmdMngr.TrqFilDisable = FALSE;
			}
		}

		/* Set Phase Offset Information to Micro <S02F>	*/
//		pAsicMicroReg->MREG_MPFPHASEOFST = (USHORT)MpFind->MpfPhaseOffset;
//		pAsicMicroReg->MREG_MPFPHASEOFSTH = (USHORT)(MpFind->MpfPhaseOffset << 16);
		MencV->MpfPhaseOffset = MpFind->MpfPhaseOffset;

		/* Update Speed Reference */
		BaseControls->CtrlCmdMngr.SpdRefi = MpFind->MpFindSpdRef;
		BaseControls->CtrlCmdMngr.SpdRefo = BaseControls->CtrlCmdMngr.SpdRefi;
		BaseControls->CtrlCmdMngr.SpdLimit = 0x1000000;	/* No speed limit	*/

		/* Speed Loop Control for Motor Pole Find */
		BaseControls->CtrlCmdMngr.TrqRefo = MpoleFindSpeedControl( MpFind,
																&(MpfExeCondition),
																BaseControls->BeComplete );

		BaseControls->CtrlCmdMngr.SpdLimit = 0x1000000;	/* No speed limit	*/
		BaseControls->CtrlCmdMngr.CtrlMode = CTRLMODE_TRQ;	/* Torque control mode */
	}
	else
	{
	/*----------------------------------------------------------------------------------------------*/
	/*		Servo expansion control																			*/
	/*----------------------------------------------------------------------------------------------*/
#if !defined(_TEST_BOARD_) || defined(_VC_DEBUG_)	/* 2012.12.13 Y.Oka Processing time countermeasures */

#if( CSW_SVCF_VIB_DETECT_USE != 0 )
		/* Vibration detection speed observer calculation */
		if(AxisRscB->UniPrms->Bprm->PsFulcSelect != FALSE)/* <S04D>:Addition of Fluke processing */
		{/* Full-closed control	*/
			DetVibSpeedObserver( DetVib, BaseControls->MotSts.FencSpd, CtrlLoopOut->TrqRefMon_a );
		}
		else
		{/* Semi-closed control	*/
		DetVibSpeedObserver( DetVib, BaseControls->MotSts.MotSpd, CtrlLoopOut->TrqRefMon_a );
		}
		/* Maximum and minimum speed deviation setting processing for oscillation detection */
		//110208_kira_for_AAT
		if( (FnCmnCtrl->State.TuningPctrlRun) || (FnCmnCtrl->State.TuningRun) )
		{ /* AATExecuting */
			DetVibSetSpeedError( DetVib, BaseControls->TuneLessCtrl.conf.TuningLessUse, TRUE );
		}
		else
		{ /* AATNot running */
			DetVibSetSpeedError( DetVib, BaseControls->TuneLessCtrl.conf.TuningLessUse, FALSE );
		}
#endif /* #if( CSW_SVCF_VIB_DETECT_USE != 0 ) */

		/* Speed deviation trace using vibration detection observer */
		DetVibTraceRuntimeService( DetVib );

#if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 )
		if( BaseControls->ResVib.AresVibSetting == TRUE )
		{ /* AMold vibration damping enabled */
			/* AMold vibration suppression control calculation */
			BaseControls->CtrlCmdMngr.SpdFBCx -=
				ResVibATypeResVibCtrl( &(BaseControls->ResVib),
									   BaseControls->CtrlCmdMngr.SpdObsFbki,
									   CtrlLoopOut->TrqRefo_a );
		}
#endif /* #if( CSW_SVCF_ATYPE_RES_VIB_USE != 0 ) */

#endif /* #if defined(_TEST_BOARD_) && !defined(_VC_DEBUG_) */

	/*----------------------------------------------------------------------------------------------*/
	/*		Yaskawa servo control main																	*/
	/*----------------------------------------------------------------------------------------------*/
		BpiYecServoControl( BaseLoops, SvControlIf, CtrlLoopOut, FnCmnCtrl, Bprm);	/* <S04D> */

	/*----------------------------------------------------------------------------------------------*/
	/*		Servo response position management																<S14C>	*/
	/*----------------------------------------------------------------------------------------------*/
		PosMngResponseManager( BaseControls );

	/*----------------------------------------------------------------------------------------------*/
	/*		Fulke position loop control																	*/
	/*----------------------------------------------------------------------------------------------*/
#if	(CSW_SVCF_FULC_MODE == CSW_FULC_SB_SUPPORT)/* Not used: Can be deleted when organizing software */
		if((AxisRscB->UniPrms->Bprm)->FencUse != FALSE)
		{
			BaseControls->CtrlCmdMngr.PicvClrCmd |= (BaseControls->BeComplete^0x1);

			BasePositionControl( &BaseControls->FcPosLoop,
								 &BaseControls->CtrlCmdMngr,
								 &BaseControls->PosManager,
								 BaseControls->GainChange.GselGains[0].Kp,
								 BaseControls->CtrlCmdPrm.KpiFc );
		}
#endif	/*(CSW_SVCF_FULC_MODE == CSW_FULC_SB_SUPPORT) */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Torque limit calculation process																		*/
/*--------------------------------------------------------------------------------------------------*/
	BpiTorqueLimitControl( &(BaseControls->TrqLimitData),
						   BaseControls,
						   SeqCtrlOut->TrqLimitReq,
						   AxisRscB->AlmManager );	/* <S145> */


/*--------------------------------------------------------------------------------------------------*/
/*		Settling time related processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	StlgCalculateSettlingTime( &BaseControls->SettlingTime,
							   BaseControls->PosManager.var.dPcmda,
							   BaseControls->PosCtrlSts.CoinSignal );

/*--------------------------------------------------------------------------------------------------*/
/*		Maximum overshoot amount creation --> Un106														*/
/*--------------------------------------------------------------------------------------------------*/
	StlgMakeMaxOverShoot( &BaseControls->SettlingTime,
						  &(BaseControls->PosManager),
						  (CTRLMODE_POS == BaseControls->CtrlModeSet.CtrlMode.b.cm) );

/*--------------------------------------------------------------------------------------------------*/
/*		Positioning completion failure cumulative time creation --> Un10B														*/
/*--------------------------------------------------------------------------------------------------*/
	StlgMakeCoinOffTime( &BaseControls->SettlingTime,
						 BaseControls->PosManager.var.dPcmda,
						 BaseControls->PosCtrlSts.CoinSignal,
						 (CTRLMODE_POS == BaseControls->CtrlModeSet.CtrlMode.b.cm) );
						 
/* 2013/04/01MDversion, the following monitors are not created. why?	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Variable output for monitor																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* Speed command(Analog monitor data setting) */
	UnMonitor->AmonSpdRef = BaseControls->CtrlCmdMngr.SpdRefo + BaseControls->CtrlCmdMngr.SpdFFCx;
//	UnMonitor->AmonSpdRef = BaseControls->CtrlCmdMngr.SpdRefo;//S0CD
	/* Torque command(Analog monitor data setting) */
	UnMonitor->AmonTrqRef = CtrlLoopOut->TrqRefMon;
	/* Position command monitor	[Command unit/Scan]		*/
	BaseControls->AmondPosCmd = BaseControls->PosManager.var.dPcmda;		/* <S226> */
//START S0CD
	BaseControls->FrictionM.MotFricEnable = AxisRscB->SeqCtrlOut->BaseEnableReq;

	if((AxisRscB->SeqCtrlOut->BaseEnableReq == TRUE) && BaseControls->FrictionM.InitEnable)
	{
		InitParamFriction( AxisRscB->BaseLoops  );
	}
	if(AxisRscB->SeqCtrlOut->BaseEnableReq == FALSE)
	{
		BaseControls->FrictionM.InitEnable = TRUE;//in order to initialize one time when servo on
	}
//END S0CD
}


/****************************************************************************************************/
/*																									*/
/*		Create magnetic pole detection servo ON request													 <S21C> 	*/
/*																									*/
/****************************************************************************************************/
#if CSW_NETIF_TYPE	!= NETIF_ANLGPLS_TYPE
//void	PdetServoOnMake( SERVO_CONTROL_IF *SvControlIf, FUN_CMN_CONTROL *FnCmnCtrl, MENCV *MencV, BOOL Alm )
ALMID_T	PdetServoOnMake( SERVO_CONTROL_IF *SvControlIf, FUN_CMN_CONTROL *FnCmnCtrl, MENCV *MencV, AXIS_HANDLE *Axis )	/* <S226> */
{
	BOOL	bwk;
	BOOL	AC22;						/* <S226> */
	BOOL	Alm;						/* <S226> */
	ALMID_T	AlmCode	= NO_ALARM;			/* <S226> */
	
	Alm  = ( Axis->AlmManager->AlmReady == FALSE );					/* <S226> */
	AC22 = ALMGetOtherAlarmChk( Axis->AlmManager, ALM_PHASEANGLE );	/* Magnetic pole detection permission alarmA.C22 <S226> */
	
	if( MencV->AxisMotType == MOTTYPE_LINEAR )						/* Linear motor					*/
	{
		if( FnCmnCtrl->FnSvControl == FALSE )						/* During normal						*/
		{
			if( MencV->PdetReq587 == TRUE )							/* Pn587.0:Magnetic pole detection request <S226> */
			{
				if(( Alm == TRUE )&&( AC22 == FALSE ))				/* <S226> */
				{
					AlmClearReq( Axis->AlmManager, ALM_PHASEANGLE );	/* A.C22Clear alarm  */
					SvControlIf->CmdSeqBit |= POL_DETECT_BIT;
				}
				else
				{
					if( Alm == FALSE )
					{
						SvControlIf->CmdSeqBit |= POL_DETECT_BIT;
					}
				}													/* <S226> */
			}
			else
			{
				SvControlIf->CmdSeqBit &= ~( POL_DETECT_BIT );
			}
			if(( SvControlIf->CmdSeqBit & SERVO_ON_BIT ) != 0 )		/* Servo-on request		 <S226> */
			{
				AlmCode = ALM_PDET;									/* Magnetic pole detection not completed					*/
			}														/*						 <S226> */
		}
		else														/* ROUNDUnder control					*/
		{
			if( Alm == FALSE )
			{
				bwk = FnCmnCtrl->PdetReq;
				if( FnCmnCtrl->FnSvonReq == TRUE )					/* Servo-on request		 <S226> */
				{
					AlmCode = ALM_PDET;								/* Magnetic pole detection not completed					*/
				}													/* 						 <S226> */
			}
			else
			{
				bwk = FALSE;
				if( AC22 == FALSE )									/* <S226> */
				{
					bwk = FnCmnCtrl->PdetReq;
				}													/* <S226> */
			}
			FnCmnCtrl->SvOnPdetReq = bwk;
		}
	}
	return( AlmCode );		/* <S226> */
}
#endif

/***************************************** end of file **********************************************/

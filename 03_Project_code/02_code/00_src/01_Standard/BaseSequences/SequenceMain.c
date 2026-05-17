/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SequenceMain.c : Servo sequence main processing definition												*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : SequenceMain processing																		*/
/*																									*/
/*		Performs alarm monitoring, alarm management, operation sequence control, etc.								*/
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
#include "MercuryGlobals.h"
#include "SerialEncIf.h"
#include "SequenceIo.h"
#include "PowerManager.h"
#include "CheckMotSts.h"
#include "CheckAlarm.h"
#include "RemVibFreq.h"
#include "AlarmManager.h"
#include "HwDeviceIf.h"
#include "MLib.h"
#include "KLib.h"
#include "JL086ServoIpHw.h"
#include "AdvancedAutoTuning.h"
#include "SftDiagFunc.h"	/*<S1F6>*/
#include "RippleComp.h"	/*<S1FD>*/
#if CSW_NETIF_TYPE	== NETIF_ANLGPLS_TYPE    /* <S1F5> */
#include "Sqi.h"
#endif

#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE
#include "PnlOp.h"	/* Network */
#else
/* TODO: AP */
#endif

/* ---futami---> */
//#include "OverheatManager.h"
/*<---futami---> */



/****************************************************************************************************/
/*		LOCAL FUNCTION																				*/
/****************************************************************************************************/
static void SyscMakeMonitorData( AXIS_HANDLE *AxisRscC );
static void SyscUpdateBaseEnableStatus( BE_SEQ_HNDL *BeSeq, LONG AxisNo,
                                       SEQ_CTRL_OUT *SeqCtrlOut, BOOL PowerOn, BOOL faultState,
									   BOOL MotorLessTestMode );									/* <S1B7> */

extern void	CpxZeroClampSequence( ZC_CTRL_DATA *Zcv, BASE_CTRL_OUT	*Bout,
													 FUN_CMN_CONTROL *Fctrl, SEQ_CTRL_OUT *Cout );
extern void SencOutputPaoSerial( MENCV *MencV, BPRMDAT *Bprm, ASIC_HW_IF_STR *pAsicHwReg,
									 BOOL DbBeReq, BOOL SenReq, USHORT AlmCode, BOOL AlmFlag );
extern void SencOutputPsoSerial( MENCV*, ASIC_HW_IF_STR*, SHORT, BOOL );
extern void FencSencOutputPaoSerial( MENCV *MencV, BPRMDAT *Bprm, ASIC_HW_IF_STR *pAsicHwReg,
									   BOOL DbBeReq, BOOL SenReq, USHORT AlmCode, BOOL AlmFlag );
extern void FencSencOutputPsoSerial( MENCV*, ASIC_HW_IF_STR*, SHORT, BOOL );
extern void HALdrv_SetAlmOut( int axis, UINT x );
extern void HALdrv_NetConLedControl( BOOL ledOut );
extern void HALdrv_TstOutputPnlLedC( void );
extern BOOL LedChkTest(  PNLOP_DATA_HANDLE *hLedData, ULONG LightIntervalTime );

static void	LpxMakeOpDispAttr( AXIS_HANDLE *AxisRscC );		/* <S00C> */
#if CSW_NETIF_TYPE	== NETIF_ANLGPLS_TYPE
static void	LpxAlmRstSequence( AXIS_HANDLE *AxisRscC );		/* <S00C> */
#else
#define LpxAlmRstSequence( x ) { ;}
#endif	
//static USHORT MakeServoStatus( AXIS_HANDLE *AxisRscC );		/* <S049> *//* <S0A1> */
static void MakeServoStatus( AXIS_HANDLE *AxisRscC );		/* <S049> *//* <S0A1> */

#ifdef CSW_SVON_INTERLOCK

#define SVON_LOCK_GRP1			0x00000006		// Axis 1 and 2 on CB2 board
#define SVON_LOCK_GRP2			0x000000f8		// Axis 3 and 8 on CC board
#define SVON_LOCK_GRP1_MAX		1
#define SVON_LOCK_GRP2_MAX		3

typedef struct SYS_BE_INTERLOCK {
	UINT	SvOnAxisBits;
	UINT	waitForSvOnAxisBits;
	UINT	LockAxesBits;
	UINT	BeReqAxesBits;
} SYS_BE_INTERLOCK;

static SYS_BE_INTERLOCK SysBeInterlock = {0};

static UINT sysGetBitONCount(ULONG bits, ULONG mask)
{
	UINT cnt;
	ULONG tmp;

	cnt = 0;
	for(tmp = (bits&mask); tmp != 0; tmp = (tmp >> 1))
	{
		if(0 != (tmp & 1))
		{
			cnt++;
		}
	}
	return cnt;
}

/****************************************************************************************************/
/*		Special functions at Multi-Axis SERVO ON													*/
/****************************************************************************************************/
static BOOL SysInterlockServoOnRequest(SYS_BE_INTERLOCK *BELock, BOOL BkBeReq, BOOL BeReqSeqOut, LONG ax_no)
{
	UINT ax_bit;
	ax_bit = (1 << ax_no);

	if( BkBeReq )
	{
		if(BeReqSeqOut == FALSE)
		{
			BELock->waitForSvOnAxisBits |= ax_bit;
			BELock->SvOnAxisBits &= ~ax_bit;

			if(0 != (SVON_LOCK_GRP1 & ax_bit))
			{
				BELock->LockAxesBits |= ax_bit;
				if(SVON_LOCK_GRP1_MAX < sysGetBitONCount(BELock->LockAxesBits, SVON_LOCK_GRP1))
				{
					BkBeReq = (BELock->BeReqAxesBits >> ax_no) & 1;
				}
				else
				{
					BELock->BeReqAxesBits |= ax_bit;
				}
			}
			if(0 != (SVON_LOCK_GRP2 & ax_bit))
			{
				BELock->LockAxesBits |= ax_bit;
				if(SVON_LOCK_GRP2_MAX < sysGetBitONCount(BELock->LockAxesBits, SVON_LOCK_GRP2))
				{
					BkBeReq = (BELock->BeReqAxesBits >> ax_no) & 1;
				}
				else
				{
					BELock->BeReqAxesBits |= ax_bit;
				}
			}
		}
		else
		{
			BELock->SvOnAxisBits |= ax_bit;
			BELock->LockAxesBits &= ~ax_bit;
			BELock->BeReqAxesBits &= ~ax_bit;
		}
	}
	else
	{
		BELock->LockAxesBits &= ~ax_bit;
		BELock->waitForSvOnAxisBits &= ~ax_bit;
		BELock->SvOnAxisBits &= ~ax_bit;
		BELock->BeReqAxesBits &= ~ax_bit;
	}

	return BkBeReq;
}


static BOOL SysMakeServoONInterlockState(SYS_BE_INTERLOCK *BELock, BOOL BkBeReq, BOOL BeReqSeqOut, LONG ax_no)
{
	UINT	SvOnAxisBits;
	UINT	waitForSvOnAxisBits;

	SvOnAxisBits = BELock->SvOnAxisBits;
	waitForSvOnAxisBits = BELock->waitForSvOnAxisBits;

	if(waitForSvOnAxisBits == (waitForSvOnAxisBits & SvOnAxisBits))
	{
		BELock->waitForSvOnAxisBits = 0;
	}
	else
	{
		BeReqSeqOut = FALSE;
	}

	return BeReqSeqOut;
}

#endif	// CSW_SVON_INTERLOCK



/****************************************************************************************************/
/*																									*/
/*		Initialize Servo Sequence Variables															*/
/*																									*/
/****************************************************************************************************/
void SyscInitServoSequence( AXIS_HANDLE *AxisRscC )
{
	BE_SEQ_HNDL		*BeSeqData;
	BeSeqData = AxisRscC->BeSequence;

/*--------------------------------------------------------------------------------------------------*/
/*		Reset Servo Sequence Variables																*/
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( &(BeSeqData->AlmStop.var), sizeof( BeSeqData->AlmStop.var )/4 );
	MlibResetLongMemory( &(BeSeqData->OtSeqData.var), sizeof( BeSeqData->OtSeqData.var )/4 );
	MlibResetLongMemory( &(BeSeqData->BkSeqData.var), sizeof( BeSeqData->BkSeqData.var )/4 );
	MlibResetLongMemory( &(BeSeqData->DbSeqData), sizeof( BeSeqData->DbSeqData )/4 );
	MlibResetLongMemory( &(BeSeqData->ZcCtrlData.f), 		/* <S00C> */
				( sizeof( BeSeqData->ZcCtrlData ) - sizeof( BeSeqData->ZcCtrlData.P ))/4 );
	MlibResetLongMemory( &(BeSeqData->ChargePump), sizeof( BeSeqData->ChargePump )/4 );

	/* Reset Servo Sequence Special Variables */
	AxisRscC->SeqCtrlOut->CtrlModeReq.dw = CTRLMODE_NOCMD;
	AxisRscC->CheckAlarm->faultState |= SHAL_FAULT_BB;	/* <S00D> */
	BeSeqData->DbOn = TRUE;
	BeSeqData->SvonEnable = TRUE;

	/* <S154> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Creating data for alarm latch processing function																	*/
/*--------------------------------------------------------------------------------------------------*/
	AxisRscC->hAlmLatchHndl->UnMonitor		= AxisRscC->UnMonitor;
	AxisRscC->hAlmLatchHndl->SeqMotSts		= AxisRscC->SeqMotSts;
	AxisRscC->hAlmLatchHndl->Bprm			= AxisRscC->UniPrms->Bprm;
	AxisRscC->hAlmLatchHndl->BaseLoops		= AxisRscC->BaseLoops;
	AxisRscC->hAlmLatchHndl->PowerManager	= AxisRscC->PowerManager;
	AxisRscC->hAlmLatchHndl->SeqCtrlOut		= AxisRscC->SeqCtrlOut;
	AxisRscC->hAlmLatchHndl->MencV			= AxisRscC->MencV;
	AxisRscC->hAlmLatchHndl->BeSequence		= AxisRscC->BeSequence;
	AxisRscC->hAlmLatchHndl->BaseControls	= AxisRscC->BaseControls;
	AxisRscC->hAlmLatchHndl->BaseCtrlOut	= AxisRscC->BaseCtrlOut;
	AxisRscC->hAlmLatchHndl->AlmManager		= AxisRscC->AlmManager;
	AxisRscC->hAlmLatchHndl->IoSignals		= AxisRscC->IoSignals;
	AxisRscC->hAlmLatchHndl->SvControlIf	= AxisRscC->SvControlIf;
	AxisRscC->hAlmLatchHndl->pPlsIfV		= AxisRscC->pPlsIfV;
	AxisRscC->hAlmLatchHndl->SftOpt			= &(AxisRscC->OptCardHdl->SftOpt);
	AxisRscC->hAlmLatchHndl->UniPrm			= AxisRscC->UniPrms;					/* <S212> */
	/* <S154> End */

/*--------------------------------------------------------------------------------------------------*/
/*		Initialize other uninitialized variables(Interim)																	*/
/*--------------------------------------------------------------------------------------------------*/
	AxisRscC->IoSignals->DetHwbbSts.GateDrvDefect = FALSE;
	AxisRscC->PowerManager->DetectDcVolt.DcAdFilo[0] = 0;
	AxisRscC->PowerManager->DetectDcVolt.DcAdFilo[1] = 0;
	AxisRscC->PowerManager->DetectDcVolt.DcAdFilo[2] = 0;
	MlibResetLongMemory( &(AxisRscC->CheckAlarm->DbOL.var), sizeof(AxisRscC->CheckAlarm->DbOL.var)/4 );

}


/****************************************************************************************************/
/*																									*/
/*		Kernel Input Service for ScanC																*/
/*																									*/
/****************************************************************************************************/
void SyscInputSeqStatus( AXIS_HANDLE *AxisRscC )
{
	BASE_CTRL_OUT		*BaseCtrlOut;
	SEQ_CTRL_OUT		*SeqCtrlOut;
	LONG				lwk0;
//	BOOL				hw_hwbb_support;/* <S04F> */
//	FBA_INPUT			SeqInputSignals;
//	SEQ_IOSIGNAL		*SeqIo;
	SEQ_IO_HNDL			*IoSignals;
	BOOL FstpSignal;   /* <S1F5> */

	BaseCtrlOut = AxisRscC->BaseCtrlOut;
	SeqCtrlOut = AxisRscC->SeqCtrlOut;
	IoSignals = AxisRscC->IoSignals;

	/* Update Motor status for Sequence modules */
	SMotSts_InputMotorStatus(AxisRscC->SeqMotSts,
	                         BaseCtrlOut,
	                         SeqCtrlOut,
	                         AxisRscC->UniPrms->Bprm);

	/* Update Id reference value for Sequence modules */
	AxisRscC->CtrlLoopOut->IdRefMon = (LONG)((SHORT)((AxisRscC->SvAsicRegs->AsicMicroReg)->WeakFV.IdOut));		/* Interim */
	BaseCtrlOut->IdRefMon			= AxisRscC->CtrlLoopOut->IdRefMon;

#if 1 /* <S0E4> *//*<S1F6>*/
	/* @@<S1F5> 2Axis numberFSTPlwk0=svon_reqis undefined */
	/* Get HWBB Status for Safety */
	lwk0 = (BaseCtrlOut->CmdSeqBit & (SERVO_ON_BIT | POL_DETECT_BIT)) | (AxisRscC->FnCmnCtrl)->FnSvControl;
	SeqCtrlOut->HwbbSts = SeqIoDetectHardWiredBB( &(IoSignals->DetHwbbSts),
											 lwk0,
											 (ASIC_HW_IF_STR *)HW_SVASIC_ADDRESS,		/* HWBBteeth1Input with fixed axis<S1F6>*/
											 IoSignals->DetHwbbSts.hwbbsupport);
//												 hw_hwbb_support /* <S0F0> CSW_HW_HWBB_SUPPORT */);

	/* Get DPIN1,DPIN2 Status for Safety <S0F1> */
	SeqIoDetectTestPulseSignal( AxisRscC->IoSignals, AxisRscC->AxisNo );
#endif /* <S0E4> */

	/* Get FSTP Status	*/ /* <S1F5> Start */
#if CSW_NETIF_TYPE	== NETIF_ANLGPLS_TYPE
	FstpSignal = ((SqiV.Sigs & SQI_SIGS_FSTP ) >> SQI_BNO_FSTP);	/* Fstp */
	SeqCtrlOut->FstpSts = SeqIoDetectFstp( FstpSignal, SeqCtrlOut , lwk0 );
#else
	FstpSignal =  IoSignals->FstpSignal.SignalStatus.v;
 	SeqCtrlOut->FstpSts = SeqIoDetectFstp( FstpSignal, SeqCtrlOut , lwk0 );
#endif

#if 1 /*  CHECKInterim processing  Fixed pin assignment support for shipping value settings <S02E><S032> */
	/* Update I/O status */
	SeqIoInputSeqSignals(IoSignals, AxisRscC->SvAsicRegs->AsicHwReg);
	SeqCtrlOut->PotSigSts = IoSignals->PotSignal.SignalStatus.v;		/* <S045> */
	SeqCtrlOut->NotSigSts = IoSignals->NotSignal.SignalStatus.v;		/* <S045> */
	SeqCtrlOut->EstpSigSts = IoSignals->EstpSignal.SignalStatus.v;		/* <S045> */
#endif
	/* Make Monitor Data for Un*** */
	SyscMakeMonitorData( AxisRscC );

	/* AP Alarm Reset <S00C> */
	LpxAlmRstSequence( AxisRscC );
}

/****************************************************************************************************/
/*																									*/
/*		Alarm detections <S00C>																		*/
/*																									*/
/****************************************************************************************************/
#if CSW_NETIF_TYPE	== NETIF_ANLGPLS_TYPE
void	LpxAlmRstSequence( AXIS_HANDLE *AxisRscC )
{
LONG AlmRst;
BASE_CTRL_OUT		*BaseCtrlOut;
SEQ_CTRL_OUT		*SeqCtrlOut;
BaseCtrlOut = AxisRscC->BaseCtrlOut;
SeqCtrlOut = AxisRscC->SeqCtrlOut;
AlmRst = (BaseCtrlOut->CmdSeqBit & ALM_RESET_BIT);

	if( AlmRst && (SeqCtrlOut->AlmRstLast == FALSE) )	/* Reset command:OFF --> ON ?			*/
	{
		ALMSetAlarmResetRequest( AxisRscC->AlmManager );
	}
	SeqCtrlOut->AlmRstLast = (AlmRst) ? TRUE : FALSE;	
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Alarm detections 																			*/
/*																									*/
/****************************************************************************************************/
void SyscAlarmDetections( AXIS_HANDLE *AxisRscC )
{
	LONG			SvStatus;
	ALMMAN_HANDLE	*AlmManager;
	BE_SEQ_HNDL		*BeSeqData;
	BASE_CTRL_OUT	*BaseCtrlOut;
	CHECK_ALARM		*CheckAlarm;
	MENCV			*MencV;
	ENVMON			*EnvMonitor; /* <S0ED> */
	TEST_PULSE_DIAG *TestPulseDiag1;	/* <S0F1> *//*<S1F6>*/
	TEST_PULSE_DIAG *TestPulseDiag2;	/* <S1F6> */

	AlmManager = AxisRscC->AlmManager;
	BeSeqData = AxisRscC->BeSequence;
	BaseCtrlOut = AxisRscC->BaseCtrlOut;
	CheckAlarm = AxisRscC->CheckAlarm;
	MencV = AxisRscC->MencV;
	EnvMonitor = AxisRscC->EnvMonitor; /* <S0ED> */
	TestPulseDiag1 = &(AxisRscC->IoSignals->TestPulseDiag[0]);	/* <S1F6> */
	TestPulseDiag2 = &(AxisRscC->IoSignals->TestPulseDiag[1]);	/* <S1F6> */

	/*------------------------------------------------------------------*/
	/*		Detect JL-077 Alarms (Watch-dog, OC, OH)					*/
	/*------------------------------------------------------------------*/
	AdetCheckAsicHwAlarm( CheckAlarm, AxisRscC->SvAsicRegs, MencV->EncDisable, AlmManager );

/*--------------------------------------------------------------------------------------------------*/
/*		Servo alarm monitoring & servo control monitoring processing														*/
/*--------------------------------------------------------------------------------------------------*/
	/* Check the Encoder Alarm */
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only<S102>	*/
//+ Caution)Serial encoderI/Fteeth1Axis servoIPSince it is shared,AsicHwReg = 1axis
	AdetCheckSencAlarm( AlmManager,
						MencV,
						SHal_GetEncAlmCode(AxisRscC->MencV->EncIf.SencSetReg, AxisRscC->AxisNo) );
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
	AdetCheckSencAlarm( AlmManager,
						MencV,
						SHal_GetEncAlmCode(AxisRscC->SvAsicRegs->AsicHwReg, 0) );
#endif	/* #if CSW_BOARD_TYPE == SGD7W_TYPE <S102> */

	/* Check Over Speed */
	if( MencV->EncDisable == FALSE )
	{ /* Encoder ready */
		AdetCheckOverSpeed(AxisRscC->AlmManager,
		                   CheckAlarm,
		                   AxisRscC->SeqMotSts->var.AbsMotSpd,
						   AxisRscC->UniPrms->Bprm );

/*--------------------------------------------------------------------------*/
#if	(SVFSEL_DIVOUT_USE == TRUE)  /*<S02D>*/
		/*------------------------------------------------------------------*/
		/*		A.511:Division pulse output overspeed monitoring process							*/
		/*------------------------------------------------------------------*/
		if( AxisRscC->UniPrms->Bprm->FencUse == FALSE )
		{ /* Semi-closed control */
			if( MencV->DivInfo.DivOut == TRUE )
			{
				AdetCheckDivOverSpeed(AxisRscC->AlmManager,
									  AxisRscC->SeqMotSts->var.AbsMotSpd,
									  MencV->DivInfo.DivOvrSpd );
			}
		}
/*--------------------------------------------------------------------------*/
#if	(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)
		else
		{
			if( AxisRscC->FencV->DivInfo.DivOut == TRUE )
			{ /* Divide output enabled during full-closed control */
				AdetCheckDivOverSpeed(AxisRscC->AlmManager,
									  AxisRscC->SeqMotSts->var.AbsMotSpd,
									  AxisRscC->FencV->DivInfo.DivOvrSpd );
			}
		}
		/*------------------------------------------------------------------*/
#endif	//(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)
/*--------------------------------------------------------------------------*/
#endif	/*(SVFSEL_DIVOUT_USE == TRUE)*/
/*--------------------------------------------------------------------------*/
	}

#if 0 /* <S0DE> */
	/* Over Heat *//* <S03A> */
	AdetCheckOverHeatAlm( AlmManager,
						  &(CheckAlarm->OverHeat),
						  DetTempGetCtrlBoardTemperature(AxisRscC->TempManager),
						  DetTempGetPowerBoardTemperature(AxisRscC->TempManager) );
	/* Installation environment monitor TODO Are the amplifiers separated by unit and the motor by axis? ? *//* <S03A> */
	AdetInstEnvMonitor( &(CheckAlarm->EnvMon),
						DetTempGetInternalTemperature(AxisRscC->TempManager),
						MencV->Temperature );
#endif

	/* Check Over Load */
	/* Amplifier & motor overload check processing(SGDSMethod) */
	AdetCheckOverLoadAmpMot( AlmManager,
							CheckAlarm,
							AxisRscC->CtrlLoopOut->IdRefMon,
							AxisRscC->CtrlLoopOut->IqRefMon,
							AxisRscC->SeqMotSts->var.AbsMotSpd,
							MencV->EncConnect );

	/* DBOverload check processing */
	AdetCheckDbrakeOverLoad( AlmManager,
							&(CheckAlarm->DbOL),
							BeSeqData->DbOn,
							AxisRscC->SeqMotSts->var.AbsMotSpd );

	/* Speed ripple compensation information mismatch warning check processing *//* <S1FD> */
	AdetCheckRippleWrn( AlmManager,
						RippleCompGetRippleWrnState( &AxisRscC->BaseLoops->RippleCmp) );

/*--------------------------------------------------------------------------------------------------*/
/*		Hardware error related alarm monitoring																	*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /* OHDetection is changed to unit processing. *//* <S0ED> */
	/* Over Heat *//* <S03A> *//* <S0DE> */
	AdetCheckOverHeatAlm( AlmManager,
						  &(CheckAlarm->OverHeat),
						  DetTempGetCtrlBoardTemperature(AxisRscC->TempManager),
						  DetTempGetPowerBoardTemperature(AxisRscC->TempManager) );

	/* Detect Temperature IC Error *//* <S0DE> */
	AdetCheckTempDeviceErr( AlmManager,
							DetTempGetCtrlBoardErrorCount(AxisRscC->TempManager),
							DetTempGetPowerBoardErrorCount(AxisRscC->TempManager) );
#endif /* <S0ED> */

	/* Installation environment monitor *//* <S03A> *//* <S0DE> */
	AdetInstEnvMonitor( EnvMonitor,
						DetTempGetInternalTemperature(AxisRscC->TempManager),
						MencV->Temperature );

	/* Check Control Error */
	/* Motor runaway check processing */
	AdetCheckMotorOverrun( AlmManager,
						  &(CheckAlarm->OvrRun),
						  AxisRscC->SeqMotSts->var.MotSpd,
						  AxisRscC->CtrlLoopOut->TrqRefMon,
						  CheckAlarm->faultState & SHAL_FAULT_BB,
						  (MOTTYPE_LINEAR == MencV->AxisMotType) );

	/* Excessive position deviation check processing when servo is on */
	if( (BaseCtrlOut->CtrlModeOut.b.cm != CTRLMODE_POS) ||
		((BeSeqData->DbSeqData.DbBeReq == FALSE) || (BeSeqData->BeReqSeqOut == TRUE)) )
	{
		ALMClearWarning( AlmManager, WRN_BEOF );	/* A.901 : Excessive position deviation when servo is on	*/
	}
	else
	{
		AdetCheckSvonPerrOverFlow( AlmManager,
								   &(CheckAlarm->OvrPosErr),
								   BaseCtrlOut->PositionError );
	}

	/* Excessive position deviation check process */
	AdetCheckPerrOverFlow( AlmManager,
						  &(CheckAlarm->OvrPosErr),
						  (CTRLMODE_POS == BaseCtrlOut->CtrlModeOut.b.cm),
						  BaseCtrlOut->PositionError,
						  BaseCtrlOut->SvonSpdLmtReq );

	/* Position data over monitoring(Linear) */
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{
		if( MlibLABS( MencV->MotAbsPos ) > POSDATALMT )
		{
			ALMSetServoAlarm( AlmManager, ALM_POSOF );
		}
	}

#if	(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)
	/* Position data over monitoring(Fulk) */
	if( AxisRscC->UniPrms->Bprm->FencUse != FALSE )
	{
		AdetCheckFullcEncoderAlarm(&(CheckAlarm->OvrFPosErr),
		                       AxisRscC->FencV,
							  (CTRLMODE_POS == BaseCtrlOut->CtrlModeOut.b.cm),
		                       AxisRscC->SeqMotSts->var.MotPos,
		                       AxisRscC->BaseCtrlOut->FencPos,
		                       AlmManager );
	}
#endif	//(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)

	if( TRUE == AxisRscC->IoSignals->DetHwbbSts.hwbbsupport )	/*Revising processing order<S1F6>*/
	{
		/* Check Safety Input Error */
		/* Input signal timing abnormality check processing */
		if( FALSE != AdetCheckBbonSignal(&(CheckAlarm->HwbbTiming), (AxisRscC->IoSignals)->DetHwbbSts.LackOneBbon) )
		{
			/* A.Eb1 : Excessive stop signal input time difference(No card) */
			ALMSetServoAlarm( AlmManager, ALM_BBON_ERR0 );
		}

		/* Check DPIN1 Error <S0F1> */
		if( FALSE != AdetTestPulseDiagnostic(TestPulseDiag1) )
		{
			/* A.EC8 : Safety functionDrvInternal signal abnormality 1 */
			ALMSetServoAlarm( AlmManager, ALM_DPIN1_ERR );
		}

		/* Check DPIN2 Error <S0F1> */
		if( FALSE != AdetTestPulseDiagnostic(TestPulseDiag2) )
		{
			/* A.EC9 : Safety functionDrvInternal signal abnormality 2 */
			ALMSetServoAlarm( AlmManager, ALM_DPIN2_ERR );
		}
	}
	else
	{
		/* Check Gate Drive Error */
		/* Gate drive error check processing */
		if( FALSE != (AxisRscC->IoSignals)->DetHwbbSts.GateDrvDefect )
		{
			/* A.B40 : Gate drive error alarm */
			ALMSetServoAlarm( AlmManager, ALM_HWBB_UNSUPPORT );
		}
	}

	/* Detect Vibration */
	if( BeSeqData->BeReqSeqOut == FALSE )
	{ /* ServoOFFMedium */
		/* A.911 : Vibration(Oscillation)Warning */
		ALMClearWarning( AlmManager, WRN_VIB );
		/* Servo vibration(Oscillation)Check reset processing */
		DetVibResetRequest( AxisRscC->DetVib );
		/* Servo control oscillation(Low frequency)Check reset processing */
		DetVibLowFreqResetRequest( AxisRscC->DetVib );
	}
	else
	{ /* ServoONMedium */
		/* Servo vibration(Oscillation)Check processing */
		SvStatus = DetVibDetectVibration( AxisRscC->DetVib );
		if( SvStatus & DETECT_VIB_ALARM )
		{
			/* A.520 : Vibration(Oscillation)Alarm */
			ALMSetServoAlarm( AlmManager, ALM_VIB );
			/* A.911 : Vibration(Oscillation)Warning */
			ALMClearWarning( AlmManager, WRN_VIB );
		}
		else if( SvStatus & DETECT_VIB_WARNING )
		{
			/* A.911 : Vibration(Oscillation)Warning */
			ALMSetServoAlarm( AlmManager, WRN_VIB );
		}
		else
		{
			/* A.911 : Vibration(Oscillation)Warning */
			ALMClearWarning( AlmManager, WRN_VIB );
		}

		/* Servo control oscillation(Low frequency)Monitoring process */
		SvStatus = DetVibDetectLowFreqVibration( AxisRscC->DetVib );
		if( (SvStatus & DETECT_VIB_ALARM)
			&& (AxisRscC->BaseControls->TuneLessCtrl.var.TuneLessAct == TRUE) )
		{ /* Alarm detection && Adjustmentless enabled */
			/* A.521 : ATAlarm */
			ALMSetServoAlarm( AlmManager, ALM_AT );
		}
	}

	/* Kernel alarm management processing */
	ALMRuntimeProcess( AlmManager, (UCHAR)(BeSeqData->AlmStop.conf.StopMode) );
	ALMRuntimeProcess2( AlmManager, (UCHAR)(BeSeqData->AlmStop.conf.StopMode) );	/* <S226> */

}

/****************************************************************************************************/
/*																									*/
/*		ScanC Main Function																			*/
/*																									*/
/****************************************************************************************************/
void SyscSequenceMain( AXIS_HANDLE *AxisRscC )
{
	BASE_CTRL_HNDL	*BaseCtrlData;
	SEQ_CTRL_OUT	*SeqCtrlOut;
	BE_SEQ_HNDL		*BeSequence;


	/* Local variable initialization */
	BaseCtrlData =AxisRscC->BaseControls;
	SeqCtrlOut = AxisRscC->SeqCtrlOut;
	BeSequence = AxisRscC->BeSequence;
/*------------------------------------------------------------------------------------------------------*/
/*		Servo operation sequence																			*/
/*------------------------------------------------------------------------------------------------------*/

	/* ProgramJOGSequence */
	CpxPrgJogPosSequence( &BaseCtrlData->PJogHdl,
						  AxisRscC->SeqCtrlOut,
						  AxisRscC->AlmManager,
						  AxisRscC->FnCmnCtrl->FnCtrlMcmd ); 

	/* Offline inertia identification */
	CpxJatOfflineCalc(	&BaseCtrlData->JatHdl,
						AxisRscC->UnMonitor->UnMotSpdx,
						AxisRscC->DetVib,
//						AxisRscC->CtrlLoopOut->PConActFlg, /* <S064> */
						AxisRscC->BaseLoops->PConActFlg,
						BaseCtrlData->CtrlCmdMngr.PerrClrReq);

	/* Advanced autotuning */
	CpxAdvancedAutoTuning(	AxisRscC->AdatHndl,
							AxisRscC->FnCmnCtrl);
	
/*------------------------------------------------------------------------------------------------------*/
/*		Residual vibration monitor processing																			*/
/*------------------------------------------------------------------------------------------------------*/
	/* Calculate Remaining Vibration Frequency */
	/* Check the RemVibCalculate Execution Condition */
	RemVibChkCondition( AxisRscC->RemVibFreq,
						(CTRLMODE_POS == BaseCtrlData->CtrlModeSet.CtrlMode.b.cm) );
	/* Calculate RemVibration */
	RemVibCalculateFreq( AxisRscC->RemVibFreq,
	                     BaseCtrlData->PosManager.var.PosErr,
						 BaseCtrlData->PosCtrlSts.CoinSignal,
						 BaseCtrlData->PosCtrlSts.RefZSignal );


	/* Outputs Sequence I/O signals */
#ifndef HW_FBA_IO_AXIS
	SeqIoBrakeControl( &(AxisRscC->IoSignals->BkSignal),
						 &(SeqCtrlOut->BrkSigSts),
						 BeSequence->BkSeqData.var.Brake,
						 ((AxisRscC->BaseCtrlOut->CmdSeqBit & BRK_RELEASE_BIT) >> BRK_RELEASE_BITNO) );
#else
	SeqIoBrakeControlAx( &(AxisRscC->IoSignals->BkSignal),
						 &(SeqCtrlOut->BrkSigSts),
						 BeSequence->BkSeqData.var.Brake,
						 ((AxisRscC->BaseCtrlOut->CmdSeqBit & BRK_RELEASE_BIT) >> BRK_RELEASE_BITNO),
						 &(AxisRscC->IoSignals->SerialOutputAx));

	HALdrv_AlmOutAx( AxisRscC->AlmManager->AlmFlag, &(AxisRscC->IoSignals->SerialOutputAx) ); /* <S145> */
	HALdrv_OutputSerialIOAx( &(AxisRscC->IoSignals->SerialOutputAx), AxisRscC->SvAsicRegs->AsicHwReg);
#endif

	/* Make Response data for Network Interface */
	SeqIoOutputNetIf( AxisRscC->IoSignals,
	                 BeSequence,
					 SeqCtrlOut,
					 AxisRscC->AlmManager,
					 AxisRscC->MencV/* <S082> AxisRscC->MencV->PhaseReady*/,
					 AxisRscC->FnCmnCtrl->State.LockNormalOpeDrive,
					 AxisRscC->MotorLessTest->MotorLessTestMode );						/* <S1B7> */

}


/****************************************************************************************************/
/*																									*/
/*		Servo Sequence Main Program																	*/
/*																									*/
/****************************************************************************************************/
 void SyscBaseDriveSequence( AXIS_HANDLE *AxisRscC )
{
	BASE_CTRL_OUT		*BaseCtrlOut;
	SEQ_CTRL_OUT		*SeqCtrlOut;
	ASICS				*SvAsicRegs;
	MENCV				*MencV;
	BE_SEQ_HNDL			*BeSequence;
	ALMMAN_HANDLE		*AlmManager;
//	CHECK_MAIN_POWER	*PowerChecker;
	POW_MNG_HNDL		*PowerManager;
//	BOOL				RelaySts;
	BOOL				spot, snot;
	BOOL				ready_cond;
	ALMID_T				alm_id;
	BOOL				almflag;	/* <S215> */

//	SeqCtrlData = AxisRscC->SeqCtrlData;
	BaseCtrlOut = AxisRscC->BaseCtrlOut;
	SeqCtrlOut = AxisRscC->SeqCtrlOut;
	BeSequence = AxisRscC->BeSequence;
	SvAsicRegs = AxisRscC->SvAsicRegs;
	AlmManager = AxisRscC->AlmManager;
//	PowerChecker = &(AxisRscC->PowerManager->MainPowerChecker);
	PowerManager = AxisRscC->PowerManager;
	MencV = AxisRscC->MencV;

	/************************************/
	/* Create basic signal for sequence control	*/
	/************************************/
	/* Make Servo Ready Status */
	almflag = (AxisRscC->FnCmnCtrl->forcedAlmReady != TRUE)?  AlmManager->AlmReady : TRUE;/* <S215> */
	ready_cond = PowerManager->PowerSts.PowerOn
				& AxisRscC->PowerManager->MainPowerChecker.var.CapaChargeOk				/* <S21D> */
				& almflag & MencV->EncReady & (!(SeqCtrlOut->HwbbSts)) & (!(SeqCtrlOut->FstpSts)); 	/* <S1F5> *//* <S215> */
	BeSeqMakeServoReadySignal( BeSequence, ready_cond, MencV->PhaseReady );

	/* Make Servo On and Sensor On Request */
	alm_id = BeSeqMakeServoOnRequest(BeSequence,
	                                 BaseCtrlOut->CmdSeqBit,
	                                 BaseCtrlOut->CtrlModeOut,
	                                 AxisRscC->FnCmnCtrl, MencV);
	if( NO_ALARM != alm_id )
	{
		if(ALM_PDET == alm_id)
		{/* A.C52 */
			if(FALSE == PdetCheckPdetCondition(AxisRscC->PdetAbsSeq, MencV))
			{	ALMSetServoAlarm( AlmManager, alm_id );}
		}
		else
		{/* A.0B0 */
			ALMSetServoAlarm( AlmManager, alm_id );
		}
	}

	/************************************/
	/* Set Alarm Stop Mode for Sequence Control */
	/************************************/
//	BeSeqInputAlmStopMode( &BeSequence->AlmStop,    /* <S1F5> */
	BeSeqInputAlmStopMode( BeSequence,     /* <S1F5> */
	                       AlmManager->Status.StopMode,	/* <S0FF> */
	                       BaseCtrlOut->CtrlModeOut,
	                       MencV->PhaseReady,
//	                       SeqCtrlOut->HwbbSts,    /* <S1F5> */
//	                       SeqCtrlOut->MotStop );    /* <S1F5> */
	                       SeqCtrlOut );    /* <S1F5> */
	/************************************/
	/* Set Force Stop Mode for Sequence Control */
	/************************************/
	BeSeqForceStopMode( BeSequence, 
	                    BaseCtrlOut->CtrlModeOut, 
	                    SeqCtrlOut->MotStop, 
	                    SeqCtrlOut->FstpSts );

	/************************************/
	/* mechanical brake sequence		*/
	/************************************/
	BeSeqMechaBrakeSequence(BeSequence,
	                        BaseCtrlOut->CtrlModeOut,
	                        AxisRscC->SeqMotSts->var.AbsMotSpd,
	                        SeqCtrlOut->HwbbSts,
	                        AxisRscC->CheckAlarm->faultState,
	    					SeqCtrlOut->BeComplete,							/* <S1B7> */
							AxisRscC->MotorLessTest->MotorLessTestMode);	/* <S1B7> */


#ifdef CSW_SVON_INTERLOCK
	/* Multiple axes simultaneous servo on interlock */
	SeqCtrlData->BkBeReq = SysInterlockServoOnRequest(&SysBeInterlock,
	                                                  SeqCtrlData->BkBeReq,
	                                                  SeqCtrlData->BeReqSeqOut,
	                                                  AxisRscC->AxisNo);
#endif //CSW_SVON_INTERLOCK

	/************************************/
	/* Overtravel sequence		*/
	/************************************/
	/* OTDetection processing */
	spot = (BaseCtrlOut->CmdSeqBit >> POSITIVE_OT_BITNO) & 0x1;
	snot = (BaseCtrlOut->CmdSeqBit >> NEGATIVE_OT_BITNO) & 0x1;
	SeqCtrlOut->OverTrvlSts = BeSeqDetectOTStatus( &BeSequence->OtSeqData,
	                                               BaseCtrlOut,
	                                               AxisRscC->IoSignals->PotSignal.SignalStatus.v | spot,		/* <S045> */
	                                               AxisRscC->IoSignals->NotSignal.SignalStatus.v | snot);		/* <S045> */
	/* OTSequence processing */
	BeSeqOverTravelSequence(&BeSequence->OtSeqData,
							BaseCtrlOut->CtrlModeOut.b.cm,
							SeqCtrlOut->MotStop,
							SeqCtrlOut->BeComplete,						/* <S1F5> */
							BeSequence->LdstpTimeZero);     			/* <S1F5> */
	/* Set OT Torque Limit Request */
	SeqCtrlOut->TrqLimitReq = BeSequence->OtSeqData.var.OtTrqLimit		/* <S1F5> */
							| BeSequence->AlmStop.var.Gr2TrqLimit		/* <S1F5> */
							| BeSequence->ForceStop.var.FstpTrqLimit;	/* <S1F5> */
	/* Set OT Warning */
	if( BeSequence->OtSeqData.var.OtWarningDet == TRUE )
	{	/* A.9A0 : OTSignal detection warning */
		ALMSetServoAlarm( AlmManager, WRN_OT );
	}
	else
	{
		ALMClearWarning( AlmManager, WRN_OT );
	}
/*--------------------------------------------------------------------------------------------------*/
/* @@ Zero Clamp Sequence 																			*/
/*--------------------------------------------------------------------------------------------------*/
	CpxZeroClampSequence( &(BeSequence->ZcCtrlData),				/* Zero Clamp Seuqence  		*/
						BaseCtrlOut,								/* Base Control Output 			*/
						AxisRscC->FnCmnCtrl,						/* Function Control				*/
						SeqCtrlOut);								/* Sequence Output 				*/
/*--------------------------------------------------------------------------------------------------*/
	/************************************/
	/* Servo stop mode management			*/
	/************************************/
	BeSeqStopModeManager(BeSequence);

	/************************************/
	/* AutoCharge pump processing				*/
	/************************************/
#if (CSW_AUTO_CHARGE_PUMP != FALSE)
	{
	BOOL OCState = ALMCheckEachState(AxisRscC->AlmManager, ALM_OC);
	BeSeqChargePumpControl( SvAsicRegs, &(BeSequence->ChargePump), OCState,
								AxisRscC->MotorLessTest->MotorLessTestMode );	/* <S1B7> */
	}
#endif

	/************************************/
	/* Dynamic brake sequence	*/
	/************************************/
	BeSeqDynamicBrakeSequence( BeSequence,
							   SvAsicRegs, SeqCtrlOut->MotStop,
							   (((AlmManager->Status.MencDataNG) ? TRUE : FALSE) | MencV->EncDataNG),	/* <S0FF> */
							   AxisRscC->MotorLessTest->MotorLessTestMode );		/* <S1B7> */

#if 0 /* PcmRlyDbControlProcess() */
	/************************************/
	/* Rush prevention relay control					*//* <S0A2> */
	/************************************/
	RelaySts = PcmRlyControlProcess( &PowerManager->MainPowerChecker,
									 &PowerManager->PowerSts,
									 AxisRscC->CheckAlarm->faultState,
									 BeSequence->DbOn,
									 AxisRscC->FnCmnCtrl->KnlMode );
	/* When not in motorless test modeON *//* <S1B7> */
	/* TODO: SGD-V, the following conditions were also achieved by disabling interruptsORis						*/
	/* (1) AxisRscC->BeSequence->StopMode == STOPMODE_DBSTOPFR		*/
	/* (2) AxisRscC->BeSequence->StopMode == STOPMODE_FREERUN		*/
	/* (3) AxisRscC->FnCmnCtrl->KnlMode == KPI_KMD_ADJCURAD			*/
	if( (RelaySts == FALSE) || (AxisRscC->MotorLessTest->MotorLessTestMode == FALSE) )
	{
		HALdrv_RelayControl( (void*)SvAsicRegs->AsicHwReg, RelaySts );
	}

	/************************************/
	/* DBCircuit control						*/
	/************************************/
#if( CSW_HW_DB_CTRL == TRUE )
	if(AxisRscC->FnCmnCtrl->KnlMode != KPI_KMD_ADJCURAD)
	{
//		BeSeqDBControlProcess( BeSequence->DbOn,								/* <S0CB> Delete */
//							   PowerManager->MainPowerChecker.conf.RlyDbWay,	/* <S0CB> Delete */
//							   AxisRscC->CheckAlarm->faultState,				/* <S0CB> Delete */
//							   SvAsicRegs->AsicHwReg );							/* <S0CB> Delete */

/* <S0CB> Start */
		/* ---TestMode7(Automatic testing machine mode:DBControl is forced ON/OFF), in this processDBControl is operated--- */
		if( PowerChecker->conf.TestMode7 == FALSE )
		{
			BeSeqDBControlProcess( BeSequence->DbOn,
								   PowerManager->MainPowerChecker.conf.RlyDbWay,
								   AxisRscC->CheckAlarm->faultState,
								   SvAsicRegs->AsicHwReg,
								   AxisRscC->MotorLessTest->MotorLessTestMode );	/* <S1B7> */
		}
/* <S0CB> End */
	}
#endif
#endif /* #if 0  PcmRlyDbControlProcess() */

	/************************************/
	/* Rush protection circuit/DBCircuit control				*//* <S20E> */
	/************************************/
	PcmRlyDbControlProcess( SvAsicRegs->AsicHwReg,
							PowerManager,
							((AxisRscC->CheckAlarm->faultState & SHAL_FAULT_BB) ? TRUE : FALSE),
							BeSequence->DbOn,
							AxisRscC->FnCmnCtrl->KnlMode,
							AxisRscC->MotorLessTest->MotorLessTestMode );

	/************************************/
	/* Scan C control mode command			*/
	/************************************/
	SeqCtrlOut->CtrlModeReq.dw = BeSeqControlModeManager(BeSequence, AxisRscC->FnCmnCtrl);
	/*Main circuitOFFtimeIPM NRequest side arm control <S201>*/
	BeNegativePumpSequence( &(AxisRscC->PowerManager->PowerSts),
							AxisRscC->AlmManager,
							&(AxisRscC->IoSignals->DetHwbbSts),
							&(AxisRscC->PowerManager->MainPowerChecker),
							&(AxisRscC->BeSequence->DbSeqData),
							AxisRscC->SeqCtrlOut,
							AxisRscC->SvAsicRegs->AsicHwReg );

	/************************************/
	/* Sequence status update				*/
	/************************************/
	SyscUpdateBaseEnableStatus( BeSequence,
								AxisRscC->AxisNo,
								SeqCtrlOut,
								AxisRscC->PowerManager->PowerSts.PowerOn,
								AxisRscC->CheckAlarm->faultState,
								AxisRscC->MotorLessTest->MotorLessTestMode );				/* <S1B7> */

}


/****************************************************************************************************/
/*																									*/
/*		ScanC Common Sequence																		*/
/*																									*/
/****************************************************************************************************/
void SyscSequenceCommon( AXIS_HANDLE *AxisRscC )
{
	LONG		ax_no;
//	UCHAR		LedDisp;
	AXIS_HANDLE	*AxisRsc;
//	HWSOUT ForceOutSignals;					/* <S0CB> */
#ifndef _TEST_BOARD_
	#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE
	FUN_CMN_CONTROL	*FnCmnCtrl;				/* <S0CB> */
	#endif
#endif
	AxisRsc = &AxisRscC[0];
#ifndef _TEST_BOARD_
	#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE
	FnCmnCtrl  = AxisRscC->FnCmnCtrl;		/* <S0CB> */
	#endif
#endif

	/*----------------------------------------------------------------------*/
	/*		7segOutput processing	<S0A1>											*/
	/*----------------------------------------------------------------------*/
#ifndef _TEST_BOARD_
	#if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE

/* <S0CB> Start */
#if 0
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
	{
		/* Set Servo Status for LED Display */
		PnlOpSetLedStatus( AxisRsc->PnlOpLedDataHndl,
						   AxisRsc->UnMonitor->LedDispStatus );
		/* Make LED Display Data */
		if( TRUE == PnlOpDisplayLedData(AxisRsc->PnlOpLedDataHndl,
										AxisRsc->AlmManager) )
		{
			/* Output Serial I/O Info. */
			HALdrv_Net7SegLedControl(
					ax_no,
					PnlOpGetLedCode(AxisRsc->PnlOpLedDataHndl) );
		}
		AxisRsc++;
	}
	/* Output Connect LED Info. */
	HALdrv_NetConLedControl( PnlOpGetConnectStatus(AxisRscC->PnlOpLedDataHndl) );
#endif
/* <S0CB> End */

/* <S0CB> Start */
			if(FnCmnCtrl->PbAndLedChkTestFlag)
			{/* 7Seg-LEDConfirmation mode */
				for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
				{
					if( TRUE == LedChkTest(AxisRsc->PnlOpLedDataHndl,
												FnCmnCtrl->LightIntervalTime) )
					{
						/* Output Serial I/O Info. */
						HALdrv_Net7SegLedControl(
								ax_no,
								PnlOpGetLedCode(AxisRsc->PnlOpLedDataHndl) );
					}
					AxisRsc++;
				}

				/* Output Connect LED Info. */
				HALdrv_NetConLedControl( TRUE );

			}
			else
			{/* Normal mode */
				for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )
				{
					/* Set Servo Status for LED Display */
					PnlOpSetLedStatus( AxisRsc->PnlOpLedDataHndl,
									   AxisRsc->UnMonitor->LedDispStatus );
					/* Make LED Display Data */
					if( TRUE == PnlOpDisplayLedData(AxisRsc->PnlOpLedDataHndl,
													AxisRsc->AlmManager) )
					{
						/* Output Serial I/O Info. */
						HALdrv_Net7SegLedControl(
								ax_no,
								PnlOpGetLedCode(AxisRsc->PnlOpLedDataHndl) );
					}
					AxisRsc++;
				}
				/* Output Connect LED Info. */
				HALdrv_NetConLedControl( PnlOpGetConnectStatus(AxisRscC->PnlOpLedDataHndl) );
			}
			AxisRsc = &AxisRscC[0];		/* Restore pointer */
/* <S0CB> End */

	#else
	HALdrv_TstOutputPnlLedC( );			/* @@ Panel Operator */
	#endif
#else /* _TEST_BOARD_ Delete scheduled */
	HALdrv_OutputSerialIO_ScanC();
#endif
	/*----------------------------------------------------------------------*/
	/*		<S045> Output Signals											*/
	/*----------------------------------------------------------------------*/
	//SeqIoOutputSeqSignals( AxisRscC->IoSignals );				/* <S045> 	*/	/* <S0CB> Delte */
/* <S0CB>	Start */
/* <S198> */
//	if( FnCmnCtrl->OutModeFlg)	/* Forced output mode */
//	{
//		ForceOutSignals.dw = FnCmnCtrl->ForceOutSignalsHW;
//		HALdrv_SetSOutC( 0, ForceOutSignals );
//	}
//	else
	{
		for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )		/* SGD7W IOI/O support<S118> */
		{
//-			SeqIoOutputSeqSignals( AxisRsc->IoSignals );
			SeqIoOutputSeqSignals( ax_no, AxisRsc->IoSignals );
			AxisRsc++;
		}
		AxisRsc = &AxisRscC[0];		/* Restore pointer */
	}
/* <S0CB> End */

	/*----------------------------------------------------------------------*/
	/*		Output Alarm 													*/
	/*----------------------------------------------------------------------*/
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )		/* ALM0,ALM1Output response<S104> */
	{
		HALdrv_SetAlmOut( ax_no, AxisRsc->AlmManager->Status.AlmFlag );		/* 1st Axis	<S0FF> */
		AxisRsc++;
	}
	AxisRsc = &AxisRscC[0];		/* Restore pointer */

	/*----------------------------------------------------------------------*/
	/*		Read Temperature <TempSensorIC>									*/
	/*----------------------------------------------------------------------*/
//	TempICI2Cdrv_Read_Temperature((ASIC_HW_IF_STR*)((AxisRscC->ModuleID->AsicNoMap)->asicAddr[0]));

	/*----------------------------------------------------------------------*/
	/*		Make Operator Display Attributes 								*/
	/*----------------------------------------------------------------------*/
	for( ax_no = 0; ax_no < NumberOfAxes; ax_no++ )		/* 2Axis compatible<S1AD> */
	{
		LpxMakeOpDispAttr( AxisRsc );
		AxisRsc++;
	}
	AxisRsc = &AxisRscC[0];		/* Restore pointer */
	/*----------------------------------------------------------------------*/
	/*		Test Pulse Send for SIL3				<S0F1>					*/
	/*----------------------------------------------------------------------*/
//<S1F6>	HALdrv_ReverceTestPls();
	SftReverceTestPls();	/*<S1F6>*/

}

/****************************************************************************************************/
/*																									*/
/*		TODO: Create signal for operator status display															*/
/*		Update signal status of Operator Status Display												*/
/*																									*/
/****************************************************************************************************/
void	LpxMakeOpDispAttr( AXIS_HANDLE *AxisRscC )
{
CTRL_CMD_MNG *CtrlCmdMngr = &(AxisRscC->BaseControls->CtrlCmdMngr);
SEQ_IO_HNDL *IoSignals = AxisRscC->IoSignals; /* <S0B6> */
BPRMDAT	*Bprm = AxisRscC->UniPrms->Bprm;
SEQ_CTRL_OUT *SeqCtrlOut = AxisRscC->SeqCtrlOut;	
SEQ_MOT_STS	*MotSts = AxisRscC->SeqMotSts;
/*--------------------------------------------------------------------------------------------------*/
	switch( CtrlCmdMngr->CtrlMode )
	{
	  /*--------------------------------------------------------------------------------------------*/
		case CTRLMODE_POS:	/* Position control mode														*/
	  /*--------------------------------------------------------------------------------------------*/
			 SeqCtrlOut->f.Vrefon  = FALSE;
			 SeqCtrlOut->f.Trefon  = FALSE;
			 SeqCtrlOut->f.Prefon  = (MotSts->var.dPcmdCnt != 0)? TRUE : FALSE;
//			 SeqCtrlOut->f.PerrClr = ( CtrlCmdMngr->PerrClrReq /* BinV.f.PerClrSignal */) ? TRUE : FALSE; /* <S0B6> */
			 SeqCtrlOut->f.PerrClr = (IoSignals->HwSin >> 15) & 1;
			 break;
	  /*--------------------------------------------------------------------------------------------*/
		case CTRLMODE_SPD:	/* Speed control mode														*/
		case CTRLMODE_JOG:	/* JOG operation mode														*/
	  /*--------------------------------------------------------------------------------------------*/
			 if( ( SeqCtrlOut->BeComplete ) 
				&& ( MlibAbs32( CtrlCmdMngr->SpdRefi ) > MotSts->conf.TgonSpdLevel ) )
			 {
				SeqCtrlOut->f.Vrefon = TRUE;
			 }
			 else
			 {
				SeqCtrlOut->f.Vrefon = FALSE;
			 }
			 SeqCtrlOut->f.Prefon  = FALSE;
			 SeqCtrlOut->f.PerrClr = FALSE;
			 break;
	  /*--------------------------------------------------------------------------------------------*/
		case CTRLMODE_TRQ:	/* Torque control mode														*/
	  /*--------------------------------------------------------------------------------------------*/
			 if( ( SeqCtrlOut->BeComplete ) 
					&& ( MlibAbs32( CtrlCmdMngr->TrqRefo /* AoutV.TrqRefi */ ) > Bprm->TrefonLevel ) )
			 {
				SeqCtrlOut->f.Trefon = TRUE;
			 }
			 else
			 {
				SeqCtrlOut->f.Trefon = FALSE;
			 }
			 SeqCtrlOut->f.Vrefon  = FALSE;
			 SeqCtrlOut->f.Prefon  = FALSE;
			 SeqCtrlOut->f.PerrClr = FALSE;
			 break;
	  /*--------------------------------------------------------------------------------------------*/
		default:			/* Others																*/
	  /*--------------------------------------------------------------------------------------------*/
			 SeqCtrlOut->f.Vrefon  = FALSE;
			 SeqCtrlOut->f.Trefon  = FALSE;
			 SeqCtrlOut->f.Prefon  = FALSE;
			 SeqCtrlOut->f.PerrClr = FALSE;
			 break;
	}
}

/****************************************************************************************************/
/*																									*/
/*		Update status of Base Servo Sequence														*/
/*																									*/
/****************************************************************************************************/
static void SyscUpdateBaseEnableStatus( BE_SEQ_HNDL *BeSeq, LONG AxisNo,
                                       SEQ_CTRL_OUT *SeqCtrlOut, BOOL PowerOn, BOOL faultState,
    								   BOOL MotorLessTestMode )											/* <S1B7> */
{
	BOOL 			BeReqSeqOut;

/*--------------------------------------------------------------------------------------------------*/
/*		Set Main Power Status																		*/
/*--------------------------------------------------------------------------------------------------*/
	SeqCtrlOut->MainPowerOn = PowerOn;

	BeReqSeqOut = BeSeq->BeReqSeqOut;
#ifdef CSW_SVON_INTERLOCK
	/* Multiple axes simultaneous servo on interlock */
	BeReqSeqOut = SysMakeServoONInterlockState(&SysBeInterlock,
	                                           BeSeq->BkBeReq,
	                                           BeReqSeqOut,
	                                           AxisRscC->AxisNo);
#endif //CSW_SVON_INTERLOCK

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation/control command clear request at base block(OT, do not clear)							*/
/*--------------------------------------------------------------------------------------------------*/
#ifdef _VC_DEBUG_ /* Provisional measures */
	BeSeq->BkSeqData.var.BkBeReq = TRUE;
#endif

	if( BeSeq->BkSeqData.var.BkBeReq == FALSE )
	{
		SeqCtrlOut->BBSvcRefClrReq = TRUE;				/* ServoOnComp = FALSE */
	}
	else if( BeReqSeqOut || (!BeSeq->OtSeqData.var.OtBeReq) )
	{
		SeqCtrlOut->BBSvcRefClrReq = FALSE;				/* ServoOnComp = TRUE */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Set Sequence Control Output Data															*/
/*--------------------------------------------------------------------------------------------------*/
	SeqCtrlOut->BaseEnableReq = BeSeq->BeReqSeqOut;
	/* Only when not in motorless test mode faultState to the condition *//* <S1B7> */
	if( MotorLessTestMode == FALSE )
	{
		SeqCtrlOut->BeComplete	= BeSeq->BeReqSeqOut & ((faultState & SHAL_FAULT_BB) == 0);
	}
	else
	{
		SeqCtrlOut->BeComplete	= BeSeq->BeReqSeqOut;
	}
	SeqCtrlOut->SvRdyPhaseIgnore = BeSeq->SvReadyPhaseIgnore;
	SeqCtrlOut->UdlBeStatus = ((faultState & SHAL_FAULT_BB) == 0) ? TRUE : FALSE;
}



/****************************************************************************************************/
/*																									*/
/*		Create data for monitor																			*/
/*																									*/
/****************************************************************************************************/
static void SyscMakeMonitorData( AXIS_HANDLE *AxisRscC )
{
	LONG				lwk;
	ANLG_UN_MON_DATA	*UnMonitor;
	SEQ_MOT_STS			*MotSts;
//	ASIC_U_IF_STR		*pAsicMicroReg;/*<S0C7>*/
	MENCV				*MencV;
	BPRMDAT				*Bprm;
	POS_MNG_HNDL		*PosManager;	/* <S056> */

	/* Local variable initialization */
	UnMonitor = AxisRscC->UnMonitor;
	MotSts = AxisRscC->SeqMotSts;
//	pAsicMicroReg = AxisRscC->SvAsicRegs->AsicMicroReg;/*<S0C7>*/
	MencV = AxisRscC->MencV;
	Bprm = AxisRscC->UniPrms->Bprm;
	PosManager = &(AxisRscC->BaseControls->PosManager);			/* <S056> */


/*--------------------------------------------------------------------------------------------------*/
/*		Un000,Un007 : Motor speed,Command pulse speed														*/
/*--------------------------------------------------------------------------------------------------*/
	UnMonitor->UnMotSpd = MotSts->var.UnMotSpd;
	UnMonitor->UnPfbk64ms = MotSts->var.UnPfbk64ms;
	UnMonitor->UnPcmd64ms = MotSts->var.UnPcmd64ms;
	UnMonitor->UnMotSpdx = MotSts->var.MotSpd;					/* 2msAverage motor speed				*/
	UnMonitor->UnPcmd64Sum = MotSts->var.UnPcmd64Sum;	/* <S1B6> */

/*--------------------------------------------------------------------------------------------------*/
/*		Un002 : Torque command monitor																	*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	UnMonitor->UnTrqRef = ( AxisRscC->CtrlLoopOut->TrqRefMon * Bprm->Kmontrq );
#else
	UnMonitor->UnTrqRef = MlibMulgain( AxisRscC->CtrlLoopOut->TrqRefMon, Bprm->Kmontrq );
#endif /* FLOAT_USE */

#if 0 /*  CHECKInterim processing */
	/*  2013/04/01ofSigmaV-MD */

	/*--------------------------------------------------------------------------------------------------*/
	/*		Un003 : Rotation angle 1(Rotary type: C phase position, linear: Electrical angle)	[pulse]									*/
	/*		Un004 : Rotation angle 2(Electrical angle)	[deg]															*/
	/*--------------------------------------------------------------------------------------------------*/
	lwk = pAsicMicroReg->MREG_MOTPHASE + (pAsicMicroReg->MREG_MOTPHASEH << 16);
	UnMonitor->UnMotPhase = SencGetMotPhase( MencV, lwk, Bprm->RvsDir, &UnMonitor->UnMotAngle );
#else
/*--------------------------------------------------------------------------------------------------*/
/*		Un003 : Rotation angle 1(Rotary type: C phase position, linear: Electrical angle)	[pulse]									*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Make Phase Infomation and Motor Angle for Linear										*/
	/*----------------------------------------------------------------------------------------------*/
/* <S214> */
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{
		MencV->MotAngle = MlibMulgainNolim( MencV->MotPhase, MencV->Kinvphase );
	}
/* <S214> */
	lwk = MencV->MotAngle;									/* Pulse/Rev						*/
	if( Bprm->RvsDir )
	{ /* Reverse Dir Mode */
		lwk = MencV->PulseNo - lwk;
	}
	if( lwk >= MencV->PulseNo )
	{ /* PulseNo --> 0 */
		UnMonitor->UnMotAngle = 0;
	}
	else
	{
		UnMonitor->UnMotAngle = lwk;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Un004 : Rotation angle 2(Electrical angle)	[deg]															*/
/*--------------------------------------------------------------------------------------------------*/
	/* 65536 --> 360 deg */
	lwk = ((((USHORT)MencV->MotPhase * 360) >> 15) + 1) >> 1;
	if( Bprm->RvsDir )
	{ /* Reverse Dir Mode */
		lwk = 360 - lwk;
	}
	if( lwk >= 360 )
	{ /* 360 --> 0 */
		UnMonitor->UnMotPhase = 0;
	}
	else
	{
		UnMonitor->UnMotPhase = lwk;
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Un009 : Effective torque data 	( 10secSquared current command average value )			UnIsqrSum					*/
/*		Un142 : Effective torque data	( 2msSquare current command value)				UnCrntRef					*/
/*		Un145 : Effective torque data	( 10secSquared current command average maximum value)		UnIsqrSumMax				*/
/*--------------------------------------------------------------------------------------------------*/
	UnMonitor->UnCrntRef = MotSts->var.UnCrntRef;
	UnMonitor->UnIsqrSum = MotSts->var.UnIsqrSum;
	UnMonitor->UnIsqrSumMax = MotSts->var.UnIsqrSumMax;

/*--------------------------------------------------------------------------------------------------*/
/*		@@ Un00C : Command pulse counter [pulse]														*/
/*--------------------------------------------------------------------------------------------------*/
	UnMonitor->UnPcmdCntr = UnMonitor->UnPcmdCntr + MotSts->var.dPcmdCnt;

/*--------------------------------------------------------------------------------------------------*/
/*		Un00D : FB pulse counter [pulse]															*/
/*--------------------------------------------------------------------------------------------------*/
	UnMonitor->UnPfbkCntr = UnMonitor->UnPfbkCntr + MotSts->var.dMotPos;

/* <S056>	Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Un013 : FB pulse counter [Command unit]														*/
/*--------------------------------------------------------------------------------------------------*/
	UnMonitor->UnPfbkCntr2 += MlibEgearRvscnv( MotSts->var.dMotPos, &PosManager->conf.Egear, &UnMonitor->RemCntr2 ) ;
/* <S056>	End */


/*--------------------------------------------------------------------------------------------------*/
/*		Un00B : Data for DB resistance power consumption(10secDB average value of power consumption)	CoutV.UnMeanDbPower				*/
/*		Un144 : Data for DB resistance power consumption(2msDB power consumption value)			CoutV.UnDBPower					*/
/*--------------------------------------------------------------------------------------------------*/
	UnMonitor->UnDbPower = AxisRscC->CheckAlarm->DbOL.var.UnDbPower;
	UnMonitor->TrcDbPower = AxisRscC->CheckAlarm->DbOL.var.UnDbPower;
	UnMonitor->UnMeanDbPower = AxisRscC->CheckAlarm->DbOL.var.UnMeanDbPower;

/*--------------------------------------------------------------------------------------------------*/
/*		Un141 : Current detection value (squared current FB value) <S0DA>													*/
/*--------------------------------------------------------------------------------------------------*/
	lwk = (AxisRscC->CtrlLoopOut->IdFbMon * AxisRscC->CtrlLoopOut->IdFbMon)
			+ (AxisRscC->CtrlLoopOut->IqFbMon * AxisRscC->CtrlLoopOut->IqFbMon);
	UnMonitor->UnCrntDet = MlibSqrtu32( lwk );

/*--------------------------------------------------------------------------------------------------*/
/*		Un141 : Current detection value (squared current FB value) 														*/
/*--------------------------------------------------------------------------------------------------*//* <S1D7> */
	UnMonitor->UnInstEnvMonAmp = (AxisRscC->EnvMonitor->var.UnInstEnvMonAmp * 0.0001)
									+ AxisRscC->EnvMonitor->conf.AmpEnvMonOffset;		/* Un025	*/

	UnMonitor->UnInstEnvMonMot = (AxisRscC->EnvMonitor->var.UnInstEnvMonMot * 0.001)
										+ AxisRscC->EnvMonitor->conf.MotEnvMonOffset;	/* Un026	*/
/* <S049>	Start */
/*--------------------------------------------------------------------------------------------------*/
/*		Data creation for servo status monitor															*/
/*--------------------------------------------------------------------------------------------------*/
//	UnMonitor->SvStatus = MakeServoStatus(AxisRscC); /* <S0A1> */
	MakeServoStatus( AxisRscC );
/* <S049>	End */
}

/****************************************************************************************************/
/*																									*/
/*		ScanC Absolute value encoder processing main									<S02D>					*/
/*																									*/
/****************************************************************************************************/
void SyscSencAbsoOperation( AXIS_HANDLE *AxisRscC )
{
#if	(SVFSEL_DIVOUT_USE == TRUE) 
/*--------------------------------------------------------------------------------------------------*/
BPRMDAT 		*Bprm;
/*--------------------------------------------------------------------------------------------------*/
/* DbSequence BaseEnable Request */
BOOL	DbBeReq = AxisRscC->BeSequence->DbSeqData.DbBeReq;
/* Alarm occurrence flag */
BOOL	almFlag = ((AxisRscC->AlmManager->Status.AlmFlag) ? TRUE : FALSE);	/* <S0FF> */
/* SEN Signal */
//BOOL	SenReq = AxisRscC->BeSequence->SenStatus;		/* <S03F> */
//<S082> BOOL	SenReq = (AxisRscC->BaseCtrlOut->CmdSeqBit & SENSOR_ON_BIT) >> SENSOR_ON_BITNO;
BOOL	SenReq = AxisRscC->BeSequence->SenStatus;		/* <S082> */
/* Alarm code information */
USHORT	AlmCode = AxisRscC->AlmManager->AlmCode;

/*--------------------------------------------------------------------------------------------------*/
Bprm  			= AxisRscC->UniPrms->Bprm;
/*--------------------------------------------------------------------------------------------------*/
//#if	(SVFSEL_DIVOUT_USE == TRUE) 

   if( Bprm->FencUse == FALSE )
   { /* In the case of semi-closed control */
	   	if( (AxisRscC->MencV->AbsoEncType != MENC_INCRE_TYPE) && (AxisRscC->MencV->DivInfo.DivOut != FALSE) )
		{ /* When the absolute value encoder is connected and the division output function is enabled */
			/*------------------------------------------------------------------------------------------*/
			/*	Rotary type motor & linear motor: Serial encoder PAO phase serial output					    */
			/*------------------------------------------------------------------------------------------*/
			SencOutputPaoSerial( AxisRscC->MencV,
								 Bprm,
								 AxisRscC->SvAsicRegs->AsicHwReg,
								 DbBeReq,
								 SenReq,
								 AlmCode,
								 almFlag );
			/* <S189> PSO_Output*//*<S0C2>*/
			SencOutputPsoSerial(
								 AxisRscC->MencV,
								 AxisRscC->SvAsicRegs->AsicHwReg,
								 AxisRscC->UniPrms->Prm->MencP.limmlt,
								 Bprm->RvsDir
								 );
		}
   }
#if	(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)
   else
   { /* For full-close control */
	   	if( (AxisRscC->FencV->AbsoEncType != MENC_INCRE_TYPE) && (AxisRscC->FencV->DivInfo.DivOut != FALSE) )
		{ /* When the absolute value encoder is connected and the division output function is enabled */
			/*------------------------------------------------------------------------------------------*/
			/*	Full-closed control:	Serial encoder PAO phase serial output   					        */
			/*------------------------------------------------------------------------------------------*/
			FencSencOutputPaoSerial( AxisRscC->FencV,
									 Bprm,
									 AxisRscC->SvAsicRegs->AsicHwReg,
									 DbBeReq,
									 SenReq,
									 AlmCode,
									 almFlag );
			/*<S189> PSO_Output*//*<S0C2>*/
			FencSencOutputPsoSerial(
									AxisRscC->FencV,
									AxisRscC->SvAsicRegs->AsicHwReg,
									AxisRscC->UniPrms->Prm->MencP.limmlt,
									Bprm->RvsDir
								   );
		}
   }
#endif//(CSW_SVCF_FULC_MODE != CSW_FULC_NO_SUPPORT)

#endif	/*(SVFSEL_DIVOUT_USE == TRUE)*/

/*--------------------------------------------------------------------------------------------------*/
}

/* <S049>	Start */
/****************************************************************************************************/
/*																									*/
/*		Data creation for servo status monitor															*/
/*																									*/
/*			A/P, this process will disable7SegLED										*/
/*																									*/
/****************************************************************************************************/
//USHORT MakeServoStatus( AXIS_HANDLE *AxisRscC )/* <S0A1> */
void MakeServoStatus( AXIS_HANDLE *AxisRscC )
{
	MENCV				*MencV;
	ANLG_UN_MON_DATA	*UnMonitor;
	BASE_LOOPCTRLS		*BaseLoops;
	BASE_CTRL_OUT		*BaseCtrlOut;
	SEQ_CTRL_OUT		*SeqCtrlOut;
	USHORT				SvStatus;
	USHORT				LedDispStatus;
	BOOL				BeComplete;
	BOOL				spot;
	BOOL				snot;

	/* Local variable initialization */
	UnMonitor = AxisRscC->UnMonitor;
	MencV = AxisRscC->MencV;
	BeComplete = AxisRscC->BaseLoops->BaseCtrls->BeComplete;
	BaseLoops = AxisRscC->BaseLoops;
	BaseCtrlOut = AxisRscC->BaseCtrlOut;
	SeqCtrlOut = AxisRscC->SeqCtrlOut;

	SvStatus = 0x0000;
	LedDispStatus = 0x00;

	if( ALMGetAlarmInfo( AxisRscC->AlmManager ) != 0x00 )	/* Alarm/warning occurring */
	{
		SvStatus = 0x0001;
		if(BeComplete) /* <S14D> */
		{
			LedDispStatus |= STS_SV_ON;
		}
	}
	else
	{
		/* RUN or BB	*/
		if(BeComplete)
		{
			if( !MencV->PhaseReady )	/* Phase detection completedFlag = On */
			{ /* Magnetic pole detection in progress */
				SvStatus = 0x0008;
//				LedDispStatus |= STS_SV_ON; /* <S0A1> *//* <S10C> */
			}
			else						/* Phase detection completedFlag = Off */
			{ /* Normal servo on */
				SvStatus = 0x0007;
			}
			//STS_SV_ON;//<S0C7>
			LedDispStatus |= STS_SV_ON; /* <S0A1> *//* <S10C> */
		}
		else
		{
			SvStatus = 0x0006;
		}

		/* P-OTSignal,N-OTGet the signal status */
		spot = (BaseCtrlOut->CmdSeqBit >> POSITIVE_OT_BITNO) & 0x1;
		snot = (BaseCtrlOut->CmdSeqBit >> NEGATIVE_OT_BITNO) & 0x1;

#if defined(MCCSW_SGDVCMOP)
/* TODO */
/* Add processing when command options are supported. */
#else
		if( BaseLoops->CtrlMode != CTRLMODE_JOG && BaseLoops->CtrlMode != CTRLMODE_ZSRCH)
		{
			/* POT/NOT	*/
			if( (AxisRscC->IoSignals->PotSignal.SignalStatus.v | spot) && (AxisRscC->IoSignals->NotSignal.SignalStatus.v | snot) )
			{
				SvStatus = 0x0003;
				LedDispStatus |= STS_P_OT | STS_N_OT; /* <S0A1> */
			}
			else if(AxisRscC->IoSignals->PotSignal.SignalStatus.v | spot)
			{
				SvStatus = 0x0004;
				LedDispStatus |= STS_P_OT;
			}
			else if(AxisRscC->IoSignals->NotSignal.SignalStatus.v | snot)
			{
				SvStatus = 0x0005;
				LedDispStatus |= STS_N_OT;
			}
		}
#endif
	}

	/* HWBB */
	if( SeqCtrlOut->HwbbSts == TRUE ) /* <S0A1> */
	{
		LedDispStatus |= STS_HBB;
	}

	/* TGON */
	if( SeqCtrlOut->TgOnSts == TRUE ) /* <S0A1> */
	{
		LedDispStatus |= STS_TGON;
	}

	/* DEN / VREF / TREF */
	switch (BaseLoops->BaseCtrls->CtrlModeSet.CtrlMode.b.cm) /* <S0A1> */
	{
	case CTRLMODE_POS:
		LedDispStatus |= (BaseLoops->BaseCtrls->PosCtrlSts.DenSignal) ? STS_DEN : 0;
		break;
#if 0 /* For future use *//* <S10C> */
	case CTRLMODE_SPD:
		LedDispStatus |= (BaseLoops->BaseCtrls->VrefInSts) ? 0 : STS_DEN;
		break;
	case CTRLMODE_TRQ:
		LedDispStatus |= (BaseLoops->BaseCtrls->TrefInSts) ? 0 : STS_DEN;
		break;
#endif
	default:
		break;
	}

	if ( SeqCtrlOut->FstpSts == TRUE)   /* <S1F5> */
	{
		SvStatus = 0x000A;
		LedDispStatus |= STS_FSTP;
	}
	/* TODO :0x010A,0x020AReason: Safety function not implemented*/


	/*SafetyModuleCode*/
	/* TODO */
	/*SafetyModulePlease add processing when compatible.*/


	UnMonitor->SvStatus = SvStatus;
	UnMonitor->LedDispStatus = LedDispStatus;

	return;
}
/* <S049>	End */


/***************************************** end of file **********************************************/

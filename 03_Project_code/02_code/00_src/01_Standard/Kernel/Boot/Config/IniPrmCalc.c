/****************************************************************************************************/
/*																									*/
/*																									*/
/*		IniPrmCalc.c : Initial parameter calculation process definition													*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Initial parameter calculation processing at power-on														*/
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
#include "MercuryGlobals.h"
#include "AlarmManager.h"
#include "MLib.h"
#include "MicroPrmCalc.h"
#include "SysConfig.h"
#include "BaseControlPrmCalc.h"
#include "ExCtrlPrmCalc.h"
#include "PnPrmListTbl.h"
#include "TuneLib.h"
#include "FnPrgJog.h"
#include "FnEasyFFT.h"
#include "ModelControl.h"
#include "JL086ServoIpHw.h"/* <S004> */
#include "RippleComp.h"	/* <S1FD> */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void		pIprmcalMain( AXIS_HANDLE *Axis, LONG caller_id );									*/
/*	void		LpxPcalKioManager( LONG ax_no );													*/
/****************************************************************************************************/
static	void	LpxPcalSpdDetMaFilter( AXIS_HANDLE *Axis );		/* Moving average initialization for motor speed detection	*/
static	void	IprmcalElectronicGear( AXIS_HANDLE *Axis );		/* Position command electronic gear calculation				*/
static	void	IprmcalAlarmMask( AXIS_HANDLE *Axis );			/* Alarm mask setting(PnE00)		*/
static	void	IprmcalSystemSwitch( AXIS_HANDLE *Axis );		/* System switch setting(PnE01)		*/
static	void	IprmcalVdetfilSwitch( AXIS_HANDLE *Axis );		/* Voltage detection filter selectionSWSetting(PnE7A)*/
static	void	IprmcalACInput( AXIS_HANDLE *Axis );			/* ACPower input selection(PnE0C,Pn00B)		*/
static	void	IprmcalBitParam000( AXIS_HANDLE *Axis );		/* Function selection basicSW(Pn000)			*/
static	void	IprmcalBitParam001( AXIS_HANDLE *Axis );		/* Function selection applicationSW(Pn001)			*/
static	void	IprmcalBitParam002( AXIS_HANDLE *Axis );		/* Function selection applicationSW(Pn002)			*/
static	void	IprmcalBitParam008( AXIS_HANDLE *Axis );		/* Function selection applicationSW(Pn008)			*/
static	void	IprmcalBitParam009( AXIS_HANDLE *Axis );		/* Function selection application switch(Pn009)		*/
static	void	IprmcalBitParam00A( AXIS_HANDLE *Axis );		/* Function selection application switch(Pn00A)		*/    /* <S1F5> */
static	void	IprmcalBitParam00B( AXIS_HANDLE *Axis );		/* Function selection application switch(Pn00B)		*/
static	void	IprmcalBitParam00D( AXIS_HANDLE *Axis );		/* Function selection application switch(Pn00D)		*/
static	void	IprmcalBitParam00E( AXIS_HANDLE *Axis );		/* Function selection application switch(Pn00E)		*//* <S21D> */
static	void	IprmcalBitParam00F( AXIS_HANDLE *Axis );		/* Function selection application switch(Pn00F)		*/	/* < S154 > */
static	void	IprmcalBitParam10B( AXIS_HANDLE *Axis );		/* Gain-related applicationSW(Pn10B)			*/
static	void	IprmcalBitParam200( AXIS_HANDLE *Axis );		/* Position control command form selectionSW(Pn200)	*/
static	void	IprmcalBitParam207( AXIS_HANDLE *Axis );		/* Position control functionSW(Pn207)			*/
#if (CSW_SVCF_BACKLASHCOMP == TRUE)
static	void	IprmcalBitParam230( AXIS_HANDLE *Axis );		/* Position control extended function switch(Pn230)	*/
#endif
//static	void	IprmcalBitParam408( AXIS_HANDLE *Axis );		/* Torque related functionSW(Pn408)		*/
static	void	IprmcalBitVelRippleCompSW( AXIS_HANDLE *Axis );	/* Speed ripple compensation function selectionSW(Pn423)*/	/* <S14B> */
static	void	IprmcalVelRippleCompSpd( AXIS_HANDLE *Axis );	/* Speed ripple compensation function selectionSW(Pn423)*/	/* <S14B> */

static	void	IprmcalBitParam514( AXIS_HANDLE *Axis );		/* Output signal selection4(Pn514)				*//* <S21D> */
static	void	IprmcalVoltageCheckLevel( AXIS_HANDLE *Axis );	/* DC power supply voltage monitoring level calculation		*/
static	void	IprmcalInstEnvMon( AXIS_HANDLE *Axis );			/* Installation environment monitor parameter calculation	*//* <S03A> */
static	void	IprmcalOverLoadLevelAmpMot( AXIS_HANDLE *Axis );/* Calculation of amplifier & motor overload level	*/
static	void	IprmcalMotorOverrun( AXIS_HANDLE *Axis );		/* Motor runaway monitoring parameter calculation		*/
static	void	IprmcalMaxBbonTimeDiff( AXIS_HANDLE *Axis );	/* HWBBTiming abnormality parameter calculation */
static	void	IprmcalSeqInputMap( AXIS_HANDLE *Axis );		/* <S045> Sequence input map calculation	*/
/* SGD7W IOI/O support<S118> */
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*/
static	void	LpxPcalSetInputMap2( AXIS_HANDLE *Axis, SEQ_IOSIGNAL *IoSignal, 	/* <S045> Sequence input map calculation (7W)	*/
						ULONG prm, ULONG LogicH );
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
static	void	LpxPcalSetInputMap( SEQ_IOSIGNAL *IoSignal, 	/* <S045> Sequence input map calculation 	*/
						ULONG prm, ULONG LogicH ); 
#endif
static	void	IprmcalSeqOutputMap( AXIS_HANDLE *Axis );		/* <S045> Sequence output map calculation	*/
/* SGD7W IOI/O support<S118> */
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	*/
static  void	LpxPcalSetSoutMap2( AXIS_HANDLE *Axis, SEQ_IOSIGNAL *IoSignal, 		/* Sequence output map calculation			*//* <S04B> */
						ULONG prm, ULONG OppositeReq );
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
//static  void	LpxPcalSetSoutMap( SEQ_IOSIGNAL *IoSignal, 		/* <S045> Sequence output map calculation	*//* <S04B> */
//						ULONG prm ); 
static  void	LpxPcalSetSoutMap( SEQ_IOSIGNAL *IoSignal, 		/* Sequence output map calculation			*//* <S04B> */
						ULONG prm, ULONG OppositeReq );
#endif

/*--------------------------------------------------------------------------------------------------*/
/* SGD7W IOI/O support2<S1B5> 																		*/
/*--------------------------------------------------------------------------------------------------*/
static	void	LpxSetInputMapCmn( AXIS_HANDLE *Axis, LONG Mode );						/* <S1B5> */
static	void	LpxSetInputMapW( AXIS_HANDLE *Axis );									/* <S1B5> */
static  void	LpxPcalSetInputMap3( SEQ_IOSIGNAL *IoSignal, ULONG prm, ULONG LogicH );	/* <S1B5> */
static	void	LpxSetOutputMapCmn( AXIS_HANDLE *Axis );								/* <S1B5> */
static	void	LpxSetOutputMapW( AXIS_HANDLE *Axis );									/* <S1B5> */
static  void	LpxPcalSetSoutMap3( SEQ_IOSIGNAL *IoSignal, ULONG prm, ULONG LogicH );	/* <S1B5> */
static  LONG	LpxGetInSel( LONG Axis, ULONG prm );									/* <S1B5> */
static  void	LpxInitLatchSignal( AXIS_HANDLE *Axis );								/* <S1B5> */
/*--------------------------------------------------------------------------------------------------*/
static	void	IprmcalPcmdOvrSpd( AXIS_HANDLE *Axis );			/* Position command overspeed(OS)		*/
static	void	IprmcalZeroStopControl( AXIS_HANDLE *Axis );	/* Zero speed stop control parameter calculation		*/
static	void	IprmcalCarrerChange( AXIS_HANDLE *Axis );		/* Carrier switching parameter calculation	*/
static	void	IprmcalTorqueLevel( AXIS_HANDLE *Axis );		/* Torque level calculation				*/
static	void	IprmcalOtMask( AXIS_HANDLE *Axis );				/* Overtravel mask provisional! ! !<S032>*/
static	void	IprmcalPowerCircuitSetting( AXIS_HANDLE *Axis );/* Power board related settings				*/
static	void	IprmcalCopyParamToIprm( AXIS_HANDLE *Axis );	/* Copy parameter(Prm --> Iprm)	*/
static	void	LpxPcalPcmpCalGain( AXIS_HANDLE	*Axis );		/* Calculation of phase compensation constant				*/
static	void	IprmcalVoltageCheckTime( AXIS_HANDLE *Axis );	/* DCPower supply voltage monitoring time calculation <S053>	*/
static	void	IprmcalFanLockDetectTime( AXIS_HANDLE *Axis );	/* Built-in fan stop detection time calculation		*//* <S0A3> */
static	void	IprmcalSpdFbFil( AXIS_HANDLE *Axis );			/* Motor quiet speed FBLPFInitial setting <S0DA>	*/
static  void	AdetCheckMotorOverSpeed(PRMDATA *Prm, BPRMDAT *Bprm, LONG AxisMotType );	/* <S1D0> */
static void	LpxMakeCarrCnt( PRMDATA *Prm, CARRCHNG	*CarrerChange );	/* Carrier frequency count creation process	*/
static	void	IprmcalFanLockAlmDetectTime( AXIS_HANDLE *Axis );	/* Built-in fan stop alarm detection time setting when servo is on	*//* <S219> */


/* External function */
extern void InitInterruptStart( void );
extern LONG KpiPcalPerClrMode( PLSIFV *pPls, LONG data );

/****************************************************************************************************/
/*																									*/
/*		Initial common parameter calculation main(Common to units)													*/
/*																									*/
/****************************************************************************************************/
void	IprmcalMainCommon( AXIS_HANDLE *Axis )
{
	IprmcalVdetfilSwitch( Axis );				/* Voltage detection filter selection switch setting(PnE7A)			*/
	IprmcalACInput( Axis );						/* ACPower input selection(PnE0C,Pn00B)						*/

	IprmcalVoltageCheckLevel( Axis );			/* DCPower supply voltage monitoring level calculation							*/
	IprmcalPowerCircuitSetting( Axis );			/* Power board related settings								*/

	IprmcalVoltageCheckTime( Axis );			/* DCPower supply voltage monitoring time calculation <S053>					*/

	IprmcalFanLockDetectTime( Axis );			/* Built-in fan stop detection time calculation						*//* <S0A3> */
}

/****************************************************************************************************/
/*																									*/
/*		Initial common parameter calculation main (axis dependent)														*/
/*																									*/
/****************************************************************************************************/
void	IprmcalMainAxis( AXIS_HANDLE *Axis )
{
	BOOL			enable;
	LONG			PrmSetting;
	TUNELESS_CFGPRM	TuneLessCfgPrm;
#if ( CSW_SVCF_MFC_USE == TRUE )
	MFC_CFG_PRM 	MfcCfgPrm;
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Control basics,Unit conversion parameter																	*/
/*--------------------------------------------------------------------------------------------------*/
	IprmcalElectronicGear( Axis );					/* Position command electronic gear calculation							*/

/*--------------------------------------------------------------------------------------------------*/
/*		Function selection bit parameter																	*/
/*--------------------------------------------------------------------------------------------------*/
	IprmcalAlarmMask( Axis );						/* Alarm mask setting(PnE00)					*/
	IprmcalSystemSwitch( Axis );					/* System switch setting(PnE01)					*/
//	IprmcalVdetfilSwitch( Axis );					/* Voltage detection filter selection switch setting(PnE7A)		*/
//	IprmcalACInput( Axis );							/* ACPower input selection(PnE0C,Pn00B)					*/
	IprmcalBitParam000( Axis );						/* Function selection basic switch(Pn000)					*/
	IprmcalBitParam001( Axis );						/* Function selection application switch(Pn001)					*/
	IprmcalBitParam002( Axis );						/* Function selection application switch(Pn002)					*/
	IprmcalBitParam008( Axis );						/* Function selection application switch(Pn008)					*/
	IprmcalBitParam009( Axis );						/* Function selection application switch(Pn009)					*/
	IprmcalBitParam00A( Axis );						/* Function selection application switch(Pn00A)					*/   /* <S1F5> */
	IprmcalBitParam00B( Axis );						/* Function selection application switch(Pn00B)					*/
	IprmcalBitParam00D( Axis );						/* Function selection application switch(Pn00D)					*/
	IprmcalBitParam00E( Axis );						/* Function selection application switch(Pn00E)					*//* <S21D> */
	IprmcalBitParam10B( Axis );						/* Gain-related application switch(Pn10B)				*/
	IprmcalBitParam00F( Axis );						/* Function selection application switch(Pn00F)					*/	/* < S154 > */
	IprmcalBitParam200( Axis );						/* Position control command form selection switch(Pn200)			*/
	IprmcalBitParam207( Axis );						/* Position control function switch(Pn207)					*/
#if (CSW_SVCF_BACKLASHCOMP == TRUE)
	IprmcalBitParam230( Axis );						/* Position control extended function switch(Pn230)				*/
#endif
//#if( FRICTRQ_COMP != 0 )	/*<S0CD>*/
	 Axis->BaseControls->FrictionM.InitEnable = TRUE;					/* <S0CD>Friction torque correction function related parameter calculation			*//*<S007>*/
//#endif /*<S0CD>*/
	//	IprmcalBitParam408( Axis );						/* Torque related function switch(Pn408)			*/

	IprmcalBitVelRippleCompSW( Axis );				/* Speed ripple compensation function selectionSW(Pn423)			*/	/* <S14B> */
	IprmcalVelRippleCompSpd( Axis );				/* Speed ripple compensation effective speed(Pn427)				*/	/* <S14B> */
	IprmcalBitParam514( Axis );						/* Output signal selection4(Pn514)						*//* <S21D> */

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration detection speed observer parameter calculation														*/
/*--------------------------------------------------------------------------------------------------*/
	DetVibObsCalculateInitPrm( &(Axis->DetVib->DetVibObs),
							   Axis->UniPrms->Bprm->Kvx,
							   Axis->UniPrms->Prm->jrate,
							   Axis->UniPrms->Bprm->SvCycleNs );

/*--------------------------------------------------------------------------------------------------*/
/*		Adjustment-less function selection																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* Adjustment-less function selection switch(Pn170/Pn14F) */
	TuneLessCfgPrm.gnmode	= (Axis->UniPrms->Prm)->gnmode;
	TuneLessCfgPrm.mfctype	= (Axis->UniPrms->Prm)->mfctype;
	TuneLessCfgPrm.avibsw	= (Axis->UniPrms->Prm)->avibsw;
	TuneLessCfgPrm.DatLevel	= (Axis->UniPrms->Prm)->DatLevel;
	TuneLessCfgPrm.tfuncsw 	= (Axis->UniPrms->Prm)->tfuncsw;
	TuneLessCfgPrm.flg_wf 	= (Axis->UniPrms->Prm)->MencP.flg_wf;
	PrmSetting = TuneLessCalculateInitPrm( Axis->BaseControls,
										   Axis->UniPrms->Bprm,
										   Axis->MencV,
										   Axis->SvAsicRegs,
										   &TuneLessCfgPrm );
	if( PrmSetting & PRMERR_TUNELESS_LVL )
	{
		ALMSetPramError( Axis->AlmManager, pndef_DatLevel.RegNumber );
	}
	if( PrmSetting & PRMERR_TUNELESS_TYPE )
	{
		ALMSetPramError( Axis->AlmManager, pndef_mfctype.RegNumber );
	}
	/* Torque filter selection after torque compensation for no adjustment */
	Axis->BaseControls->CtrlCmdPrm.LpassFil3 = TuneLessSetTrqFil3(
						Axis->BaseControls->TuneLessCtrl.conf.TuningLessUse,
						Axis->BaseControls->TuneLessCtrl.conf.TuningLessEx,
						(Axis->UniPrms->Prm)->MencP.flg_wf,
						Axis->MencV->TuningLessLowGainMotor );

#if ( CSW_SVCF_MFC_USE == TRUE )
/*--------------------------------------------------------------------------------------------------*/
/*		Model following control function selection																		*/
/*--------------------------------------------------------------------------------------------------*/
	PcalCopyMFCCfgParam(&MfcCfgPrm, Axis->UniPrms->Prm);
	if( FALSE != MfcCalculateInitPrm( &(Axis->BaseControls->MFControl),
									  &MfcCfgPrm,
									  Axis->UniPrms->Prm->jrate,
									  Axis->UniPrms->Bprm,
									  Axis->UniPrms->Prm->mfctype) )
	{
		ALMSetPramError( Axis->AlmManager, pndef_mfctype.RegNumber );
	}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Alarm monitoring related parameters																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* DC power supply voltage monitoring level calculation */
//	IprmcalVoltageCheckLevel( Axis );
	/* Calculation of amplifier & motor overload level */
	IprmcalOverLoadLevelAmpMot( Axis );
	/* Motor runaway monitoring parameter calculation */
	IprmcalMotorOverrun( Axis );
	/* Input signal timing abnormality parameter calculation */
	IprmcalMaxBbonTimeDiff( Axis );
	/* Installation environment monitor parameter calculation *//* <S03A> */
	IprmcalInstEnvMon( Axis );

/*--------------------------------------------------------------------------------------------------*/
/*		Sequence input/output map																		*/
/*--------------------------------------------------------------------------------------------------*/
	IprmcalSeqInputMap( Axis );					/* <S045> Sequence input map calculation					*/
	IprmcalSeqOutputMap( Axis );				/* <S045> Sequence output map calculation					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Control-related parameters																			*/
/*--------------------------------------------------------------------------------------------------*/
	IprmcalSpdFbFil( Axis );					/* <S0DA> Motor quiet speed FBLPFInitial setting		*/
	IprmcalPcmdOvrSpd( Axis );					/* Position command overspeed(OS)					*/
	IprmcalZeroStopControl( Axis );				/* Zero speed stop control parameter calculation					*/

	/* Vibration frequency analysis initial settings */
	FftCalculateInitPrm( Axis->FftAna, (Axis->UniPrms->Bprm)->SvCycleUs );

	/* Carrier frequency switching parameter calculation */
	IprmcalCarrerChange( Axis );

	/* Initial settings for automatic notch */
	AutoNotchCalculateInitPrm( Axis->ANotchSeq,
							   Axis->ANotch,
							   Axis->MencV->AxisMotType,
							   Axis->UniPrms );

#if (CSW_SVCF_BACKLASHCOMP == TRUE)
	/* Backlash correction amount limit value calculation */
	BlshCalcMaxBlshCompPulse( &(Axis->BaseControls->PosManager.BlshCmp),
							  &(Axis->BaseControls->PosManager.conf.Egear),
							  Axis->UniPrms->Bprm );
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		ProgramJOGInitial parameter calculation																*/
/*--------------------------------------------------------------------------------------------------*/
	IprmcalPrgJog( &Axis->BaseControls->PJogHdl,
					Axis->UniPrms->Bprm,
				   &Axis->BaseControls->PosManager );

/*--------------------------------------------------------------------------------------------------*/
/*		Others																						*/
/*--------------------------------------------------------------------------------------------------*/
	IprmcalTorqueLevel( Axis );					/* Torque level calculation							*/
//	IprmcalVdetPrmChk( Axis );					/* Main circuit detection parameter check					*/
	IprmcalOtMask( Axis );						/* Overtravel mask provisional! ! ! !kira<S032>		*/
	IprmcalCopyParamToIprm( Axis );				/* Copy parameter(Prm --> Iprm)				*/

	/* EasyFFTInitial parameter calculation */
	EasyFftInitialize( Axis->EasyFFT,
					   Axis->TableRefBuffer,
					   Axis->TraceBuffer,
					   Axis->UniPrms );

/*--------------------------------------------------------------------------------------------------*/
/*		Reset and Init motor speed ma filetr detas													*/
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalSpdDetMaFilter( Axis );

/*----------------------------------------------------------------------------------------------*/
/*		PnE39 Bit2 : Main circuit voltage correction function selection															*/
/*----------------------------------------------------------------------------------------------*/
	enable = FALSE;
	if( ((Axis->UniPrms->Cprm->syssw2 >> 8) & 0x03) == VDET_AD )
	{
		enable = (0 != (Axis->UniPrms->Prm->dq_sw2 & 0x0004));
	}
	PcmKvvIniPrmCalc(&(Axis->BaseControls->MpKvvComp),
	                 Axis->UniPrms->Bprm,
	                 Axis->UniPrms->Cprm->kvvfil,
	                 Axis->UniPrms->Cprm->pnvoltgn,
	                 Axis->UniPrms->Prm->vrevgn,
	                 enable );

	/* Calculation of phase compensation constant */
	LpxPcalPcmpCalGain( Axis );

/*<S1D0> START*/
/*----------------------------------------------------------------------------------------------*/
/*		Motor maximum speed processing																				*/
/*----------------------------------------------------------------------------------------------*/
	AdetCheckMotorOverSpeed( Axis->UniPrms->Prm, Axis->UniPrms->Bprm, Axis->MencV->AxisMotType);
/*<S1D0> START*/

/* <S219> Start */
/*----------------------------------------------------------------------------------------------*/
/*		 PnEE6 : Built-in fan stop alarm detection time when servo is on 									*/
/*----------------------------------------------------------------------------------------------*/
	IprmcalFanLockAlmDetectTime( Axis );
/* <S219> End */


}


/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Voltage detection filter selection switch setting(PnE7A)														*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		PnE7A.Bit0,1 : Main circuit voltage detection abnormality detection filter	( 00-11 / 0us-750us , 250usIncrements )			*/
/*		PnE7A.Bit2,3 : Overvoltage alarm detection filter		( 00-11 / 0us-750us , 250usIncrements )			*/
/*		PnE7A.Bit4,5 : Regeneration operation start filter				( 00-11 / 0us-750us , 250usIncrements )			*/
/*		PnE7A.Bit6,7 : Regeneration operation end filter				( 00-11 / 0us-750us , 250usIncrements )			*/
/*		PnE7A.Bit8-14: Empty																			*/
/*		PnE7A.Bit15	 : Main circuit AD detection (before filter) trace selection (=1:Trace selection0x05for traceability)	*/
/*						※For debugging																*/
/*--------------------------------------------------------------------------------------------------*/
void	IprmcalVdetfilSwitch( AXIS_HANDLE *Axis )
{
	LONG				swbuf;
	REGENE_CTRL			*RegCtrl;
	DET_DC_VOLT			*DetectDc;
	CHECK_MAIN_POWER	*PowerChecker;
	UNI_PRM_STR			*UniPrm;
	PCOV_ERR_DETECT		*PcovErrDetect;									/* <S053> */

	UniPrm = Axis->UniPrms;
	swbuf = UniPrm->Cprm->vdefilsw;
	RegCtrl = &(Axis->PowerManager->RegeneCtrl);
	DetectDc = &(Axis->PowerManager->DetectDcVolt);
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);
	PcovErrDetect= &(Axis->PowerManager->PcovErrDetect);				/* <S053> */

/*--------------------------------------------------------------------------------------------------*/
/*		Various filter settings																			*/
/*--------------------------------------------------------------------------------------------------*/
	DetectDc->conf.VdcdetAlmFilter		= swbuf & 0x0003;				/* Main circuit abnormality detection filter	*/
	PowerChecker->conf.OvdetAlmFilter	= ( swbuf >> 2 ) & 0x0003;		/* Overvoltage detection filter		*/
	RegCtrl->conf.RegStartFilter		= ( swbuf >> 4 ) & 0x0003;		/* Regeneration operation start filter		*/
	RegCtrl->conf.RegStopFilter			= ( swbuf >> 6 ) & 0x0003;		/* Regeneration operation end filter		*/
	

#if 0 /* 2010.06.25 Y.Oka INGRAMIntermediate potential abnormality detection function currently not supported */
/*--------------------------------------------------------------------------------------------------*/
/*		Intermediate potential abnormality detection function enabled			<V576>													*/
/*							Detection time constant		<V582>													*/
/*--------------------------------------------------------------------------------------------------*/
	wk = ( swbuf >> 8 ) & 0x0f;

	if( wk == 0 )
	{
		Iprm->PcovEna = FALSE;								/* Intermediate potential abnormality detection disabled					*/
	}
	else
	{
		Iprm.f.PcovEna = TRUE;								/* Intermediate potential abnormality detection enabled					*/
	}
	Iprm.PcovErrCntLmt = wk * (100/KPI_SCCYCLEMS);			/* Intermediate potential abnormality detection time constant				*/
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		<S053> Intermediate potential abnormality detection time constant, function enabled/Disabled												*/
/*--------------------------------------------------------------------------------------------------*/
	PcovErrDetect->conf.PcovErrCntLmt = (( swbuf >> 8 ) & 0x0f) * (100/KPI_SCCYCLEMS);

	if( PcovErrDetect->conf.PcovErrCntLmt == 0 )
	{
		PcovErrDetect->conf.PcovEna = FALSE;				/* Intermediate potential abnormality detection disabled					*/
	}
	else
	{
		PcovErrDetect->conf.PcovEna = TRUE;					/* Intermediate potential abnormality detection enabled					*/
	}
	
}


/****************************************************************************************************/
/*																									*/
/*		ACPower input selection(PnE0C,Pn00B)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		PnE0C.Bit1-2 : Servopack power input specifications													*/
/*		PnE0C.Bit3   : Power supply phase loss detection method																*/
/*--------------------------------------------------------------------------------------------------*/
/*		Pn00B.2      : Power input selection for SERVOPACK with three-phase input specifications	 									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalACInput( AXIS_HANDLE *Axis )
{
	LONG				lwk;
	CMN_PRMDATA			*Cprm;
	PRMDATA				*Prm;
	CHECK_MAIN_POWER	*PowerChecker;
	CHECK_LOWOL			*pLowOL;		/* <S1F5> */

	Prm = Axis->UniPrms->Prm;
	Cprm = Axis->UniPrms->Cprm;		// Parameter setting data
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);
	pLowOL = &(Axis->CheckAlarm->AmpLowOL);					/* <S1F5> */

/*--------------------------------------------------------------------------------------------------*/
/*	Bit 1,2: ACPower input selection																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (Cprm->syssw2 >> 1) & 0x03 )
	{
		case 0x00:											/* Only single-phase power input is possible					*/
			PowerChecker->conf.Ac3Phase = FALSE;
			pLowOL->conf.BaseCurrentDerateEnable = FALSE;			/* <S1F5> */
			break;
		case 0x01:											/* Only three-phase power input is possible					*/
			PowerChecker->conf.Ac3Phase = TRUE;
			pLowOL->conf.BaseCurrentDerateEnable = FALSE;			/* <S1F5> */
			break;
			/* <S1F5> Start */
		case 0x02:
			lwk = (Prm->b_prmB>>8) & 0x0F;
			if( lwk <= 1 )
			{
				if(lwk == 0)
				{
					PowerChecker->conf.Ac3Phase = TRUE;		/* Used with three-phase power input				*/
					pLowOL->conf.BaseCurrentDerateEnable = FALSE;
				}
				else
				{
					PowerChecker->conf.Ac3Phase = FALSE;	/* Used with single-phase power input				*/
					/* With base current derating when single-phase power input is selected */
					pLowOL->conf.BaseCurrentDerateEnable = TRUE;
				}
			}
			else
			{
				PowerChecker->conf.Ac3Phase = TRUE;
				pLowOL->conf.BaseCurrentDerateEnable = FALSE;
				ALMSetPramError( Axis->AlmManager, pndef_b_prmB.RegNumber );
			}
			break;
			/* <S1F5> End */
		case 0x03:											/* Single-phase power input is also possible with three-phase input specifications		*/
			lwk = (Prm->b_prmB>>8) & 0x0F;
			if( lwk <= 1 )
			{
				if(lwk == 0)
				{
					PowerChecker->conf.Ac3Phase = TRUE;		/* Used with three-phase power input				*/
					pLowOL->conf.BaseCurrentDerateEnable = FALSE;			/* <S1F5> */
				}
				else
				{
					PowerChecker->conf.Ac3Phase = FALSE;	/* Used with single-phase power input				*/
					pLowOL->conf.BaseCurrentDerateEnable = FALSE;			/* <S1F5> */
				}
			}
			else
			{
				pLowOL->conf.BaseCurrentDerateEnable = FALSE;			/* <S1F5> */
				PowerChecker->conf.Ac3Phase = TRUE;
				ALMSetPramError( Axis->AlmManager, pndef_b_prmB.RegNumber );
			}
			break;
		default:											/* Reservation									*/
			pLowOL->conf.BaseCurrentDerateEnable = FALSE;			/* <S1F5> */
			PowerChecker->conf.Ac3Phase = FALSE;
			ALMSetPramError( Axis->AlmManager, pndef_syssw2.RegNumber );
			break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*	Bit 3: Power supply phase loss detection method																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( Cprm->syssw2 & 0x0008 )
	{ /* PnE0C.bit3 */
		PowerChecker->conf.DetectAcOn2	= TRUE;
	}
	else
	{
		PowerChecker->conf.DetectAcOn2	= FALSE;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Power supply phase loss detection method:ACON1/ACON2In the case of3Parameter setting error if it is not a SERVOPACK with phase input specification		*/
	/*----------------------------------------------------------------------------------------------*/
	if( PowerChecker->conf.DetectAcOn2 )
	{
		if( ((Cprm->syssw2 >> 1) & 0x01) == 0 )							/* PnE0C.bit1Changed to direct reference	*/
		{
			ALMSetPramError( Axis->AlmManager, pndef_syssw2.RegNumber );
		}
	}
}


/****************************************************************************************************/
/*																									*/
/*		Function selection basic switch(Pn000)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn000.0 : Rotation direction selection																		*/
/*		Pn000.1 : Control method selection																		*/
/*		Pn000.2 : Axis address																		*/
/*		Pn000.3 : Rotation type, linear type selection	2009.09.17 Y.Oka											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam000( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;

	Prm  = Axis->UniPrms->Prm;		// Parameter setting data
	Bprm = Axis->UniPrms->Bprm;		// Initial parameter definition(Changes take effect when the power is turned back on.)

/*--------------------------------------------------------------------------------------------------*/
/*		Pn000.0 : Rotation direction selection(Reverse rotation mode)														*/
/*--------------------------------------------------------------------------------------------------*/
	switch( Prm->b_prm0 & 0x0F )
	{
	case 0 :
		Bprm->RvsDir = FALSE;
		Bprm->DirSign = +1;
		break;
	case 1 :
		Bprm->RvsDir = TRUE;
		Bprm->DirSign = -1;
		break;
	default:
		ALMSetPramError( Axis->AlmManager, pndef_b_prm0.RegNumber );
		break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn000.1 : Control method(Control mode)Selection															*/
/*--------------------------------------------------------------------------------------------------*/
// M3

/*--------------------------------------------------------------------------------------------------*/
/*		Pn000.3 : Rotation type, linear type selection	2009.09.17 Y.Oka											*/
/*--------------------------------------------------------------------------------------------------*/
// Set in initial processing

}


/****************************************************************************************************/
/*																									*/
/*		Function selection application switch(Pn001)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn001.0 : Stop method when servo is off and alarm occurs											*/
/*		Pn001.1 : Overshoot label(OT)													*/
/*		Pn001.2 : Select AC/DC power input															*/
/*		Pn001.3 : Warning code output selection														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam001( AXIS_HANDLE *Axis )
{
	PRMDATA				*Prm;
	BPRMDAT				*Bprm;
	CHECK_MAIN_POWER	*PowerChecker;
	ALM_STOP			*AlmStop;
	OT_SEQ_DATA			*OtSeqData;

	Prm  = Axis->UniPrms->Prm;		// Parameter setting data
	Bprm = Axis->UniPrms->Bprm;		// Initial parameter definition(Changes take effect when the power is turned back on.)
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);
	AlmStop = &(Axis->BeSequence->AlmStop);
	OtSeqData = &(Axis->BeSequence->OtSeqData);

/*--------------------------------------------------------------------------------------------------*/
/*		Pn001.0 : Stop method when servo is off and alarm occurs											*/
/*--------------------------------------------------------------------------------------------------*/
	switch( Prm->b_prm1 & 0x0F )
	{
		case 0x00: AlmStop->conf.StopMode = STOPMODE_DBSTOPDB; break;
		case 0x01: AlmStop->conf.StopMode = STOPMODE_DBSTOPFR; break;
		case 0x02: AlmStop->conf.StopMode = STOPMODE_FREERUN;  break;
		default:   ALMSetPramError( Axis->AlmManager, pndef_b_prm1.RegNumber ); break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn001.1 : Overtravel(OT)													*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (Prm->b_prm1>>4) & 0x0F )
	{
		case 0x00: OtSeqData->conf.OtStopMode = OTSTOPMODE_DEFAULT; break;
		case 0x01: OtSeqData->conf.OtStopMode = OTSTOPMODE_ZSTOPZC; break;
		case 0x02: OtSeqData->conf.OtStopMode = OTSTOPMODE_ZSTOPFR; break;
		case 0x03: OtSeqData->conf.OtStopMode = OTSTOPMODE_LDSTOPZC; break;    /* <S1F5> */
		case 0x04: OtSeqData->conf.OtStopMode = OTSTOPMODE_LDSTOPFR; break;    /* <S1F5> */
		default:   ALMSetPramError( Axis->AlmManager, pndef_b_prm1.RegNumber ); break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn001.2 : AC/DCPower input selection																*/
/*--------------------------------------------------------------------------------------------------*/
/* 2013.01.07 Y.Oka Compile the followingSWUnnecessary */
//#if( SVFSEL_DCBUS == TRUE )	/* ForciblyDCWhen setting to power input mode */
	switch( (Prm->b_prm1>>8) & 0x0F )
	{
	case 0x00: /* ACPower input selection */
		PowerChecker->conf.Dcbus = FALSE;								/* ACpower input				*/
		break;

	case 0x01: /* DC power supply input selection	*/
		if( Bprm->AmpInVcode == 0x00 )
		{ /* Double voltage specification,100For V input specificationDCPower input selection not possible */
			PowerChecker->conf.Dcbus = FALSE;
		}
		else
		{
			PowerChecker->conf.Dcbus = TRUE;							/* DCpower input				*/
		}
		break;
	default:
		PowerChecker->conf.Dcbus = FALSE;
		break;
	}
//#endif
/* 2013.01.07 Y.Oka Compile the followingSWUnnecessary */

//#if 0	/* <S1A8> : "Pn001.3 : Return to "warning code output selection" */
/*--------------------------------------------------------------------------------------------------*/
/*		Pn001.3 : Warning code output selection														*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (Prm->b_prm1>>12) & 0x0F )
	{
//		case 0x00: Iprm->WrnOutSel = FALSE; break;			/* <S1A8> */
//		case 0x01: Iprm->WrnOutSel = TRUE;  break;			/* <S1A8> */
		case 0x00: Axis->AlmManager->WrnOutSel = FALSE; break;	/* <S1A8> */
		case 0x01: Axis->AlmManager->WrnOutSel = TRUE;  break;	/* <S1A8> */
		default:   ALMSetPramError( Axis->AlmManager, pndef_b_prm1.RegNumber ); break;
	}
//#endif	/* <S1A8> : "Pn001.3 : Return to "warning code output selection" */
}


/****************************************************************************************************/
/*																									*/
/*		Function selection application switch(Pn002)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn002.0 : Speed control option(T-REFAssignment) 		 										*/
/*		Pn002.1 : Torque control option(V-REFAssignment)												*/
/*		Pn002.2 : How to use absolute value encoder														*/
/*		Pn002.3 : How to use full-closed encoder												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam002( AXIS_HANDLE *Axis )
{
	TRQ_LMT_DATA	*TrqLimits;
	PRMDATA			*Prm;

	TrqLimits = &(Axis->BaseControls->TrqLimitData);
	Prm  = Axis->UniPrms->Prm;		// Parameter setting data

/*--------------------------------------------------------------------------------------------------*/
/*		Pn002.0 : Speed control option(T-REFAssignment) 		 										*/
/*--------------------------------------------------------------------------------------------------*/
	switch( Prm->b_prm2 & 0x0F )
	{
	case 0x00:	/* None							*/
//		TrqLimits->conf.TrefTrqFF    = FALSE;
		TrqLimits->conf.TrefTrqLmt   = FALSE;
		TrqLimits->conf.TrefTrqLmtCL = FALSE;
		break;
	case 0x01:	/* Used as external torque limit input */
//		TrqLimits->conf.TrefTrqFF    = FALSE;
		TrqLimits->conf.TrefTrqLmt   = TRUE;
		TrqLimits->conf.TrefTrqLmtCL = FALSE;
		break;
	case 0x02:	/* Used as torque feedforward input */
//		TrqLimits->conf.TrefTrqFF    = TRUE;
		TrqLimits->conf.TrefTrqLmt   = FALSE;
		TrqLimits->conf.TrefTrqLmtCL = FALSE;
		break;
	case 0x03:	/* External torque limit input(Limit signal(P-CL,N-CL)When enabled) */
//		TrqLimits->conf.TrefTrqFF    = FALSE;
		TrqLimits->conf.TrefTrqLmt   = FALSE;
		TrqLimits->conf.TrefTrqLmtCL = TRUE;
		break;
	default:
		ALMSetPramError( Axis->AlmManager, pndef_b_prm2.RegNumber );
		break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn002.1 : Torque control option(V-REFAssignment)												*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (Prm->b_prm2>>4) & 0x0F )
	{
	case 0x00:	/* None							*/
		Axis->BaseControls->CtrlCmdPrm.VrefSpdLmt = FALSE;
		break;
	case 0x01:	/* Used as external speed limit input */
		Axis->BaseControls->CtrlCmdPrm.VrefSpdLmt = TRUE;
		break;
	default:
		ALMSetPramError( Axis->AlmManager, pndef_b_prm2.RegNumber );
		break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn002.2 : How to use absolute value encoder			--> Sencif.c								*/
/*		Pn002.3 : How to use full-closed encoder	--> Fencif.c								*/
/*--------------------------------------------------------------------------------------------------*/

}


/****************************************************************************************************/
/*																									*/
/*		Function selection application switch(Pn008)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn008.0 : Battery voltage drop alarm/warning selection										*/
/*		Pn008.1 : 									 		 										*/
/*		Pn008.2 : Warning detection selection				 		 										*/
/*		Pn008.3 : 																					*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam008( AXIS_HANDLE *Axis )
{
	UV_TLIM_DATA	*UvTrqLimits;
//	IPRMDAT			*Iprm;
//	CMN_PRMDATA		*Cprm;
	PRMDATA			*Prm;
	ULONG			uswk;			/* <S0AA> */

	UvTrqLimits = &(Axis->BaseControls->TrqLimitData.UvTrqLimits);
//	Cprm = Axis->UniPrms->Cprm;		// Parameter setting data
	Prm  = Axis->UniPrms->Prm;		// Parameter setting data
//	Iprm = Axis->prm_hndl_ptr->iprm_ptr;		// Initial parameter definition(Changes take effect when the power is turned back on.)

	/*--------------------------------------------------------------------------------------------------*/
	/*		Battery voltage drop alarm/warning selection													*/
	/*--------------------------------------------------------------------------------------------------*/
//	switch( Cprm->b_prm8 & 0x0F )
	switch( Prm->b_prm8 & 0x0F )
	{
	case 0x00:	/* Battery voltage drop as alarm		*/
		Axis->MencV->BatWrnMode = FALSE;
		break;
	case 0x01:	/* Battery voltage drop as warning	*/
		Axis->MencV->BatWrnMode = TRUE;
		break;
	default:	/* Parameter setting error					*/
		ALMSetPramError( Axis->AlmManager, pndef_b_prm8.RegNumber );
		break;
	}

	/*--------------------------------------------------------------------------------------------------*/
	/*		Function selection when main circuit voltage drops																	*//* <V200> */
	/*--------------------------------------------------------------------------------------------------*/
//	switch(( Cprm->b_prm8 >> 4 ) & 0x0F )
	switch(( Prm->b_prm8 >> 4 ) & 0x0F )
	{
	case 0x00:	/* Does not detect main circuit voltage drop warning */
		UvTrqLimits->conf.UvTrqLmtCtrl = FALSE;
		ALMSetMask( Axis->AlmManager, WRN_UV, TRUE );	/* A.971 : Undervoltage warning		*/
		break;
	case 0x01:	/* Detects main circuit voltage drop warning */
		UvTrqLimits->conf.UvTrqLmtCtrl = FALSE;
		ALMSetMask( Axis->AlmManager, WRN_UV, FALSE );	/* A.971 : Undervoltage warning		*/
		break;
	case 0x02:	/* Detects main circuit voltage drop warning,Pn424,Pn425 */
		UvTrqLimits->conf.UvTrqLmtCtrl = TRUE;
		ALMSetMask( Axis->AlmManager, WRN_UV, FALSE );	/* A.971 : Undervoltage warning		*/
		break;
	default:	/* Parameter setting error					*/
		ALMSetPramError( Axis->AlmManager, pndef_b_prm8.RegNumber );
		break;
	}

	/*--------------------------------------------------------------------------------------------------*/
	/*		Warning detection selection																			*/
	/*--------------------------------------------------------------------------------------------------*/
//	switch( (Cprm->b_prm8>>8) & 0x0F )
	switch( (Prm->b_prm8>>8) & 0x0F )
	{
	case 0x00:	/* Detects warning		*/
		ALMSetMask( Axis->AlmManager, WRN_OLF,      FALSE );/* A.910 : Overload warning					*/
		ALMSetMask( Axis->AlmManager, WRN_VIB,      FALSE );/* A.911 : Oscillation warning						*/
		ALMSetMask( Axis->AlmManager, WRN_CTRLBRD,  FALSE );/* A.912 : Internal temperature warning1(Control board temperature warning)*//* <S0DE> */
		ALMSetMask( Axis->AlmManager, WRN_PWRBRD,   FALSE );/* A.913 : Internal temperature warning2(Power board temperature warning)*//* <S0DE> */
		ALMSetMask( Axis->AlmManager, WRN_RGOLF,    FALSE );/* A.920 : Regenerative overload warning				*/
		ALMSetMask( Axis->AlmManager, WRN_DATA,     FALSE );/* A.940 : Data abnormality warning				*/
		ALMSetMask( Axis->AlmManager, WRN_CMD,      FALSE );/* A.950 : Command error warning				*/
		ALMSetMask( Axis->AlmManager, WRN_OPT,      FALSE );/* A.9F0 : Optional error warning			*/
		ALMSetMask( Axis->AlmManager, WRN_PGBAT,    FALSE );/* A.930 : Absolute value encoder battery error		*/
		ALMSetMask( Axis->AlmManager, WRN_FPGBAT,   FALSE );/* A.930 : FPG battery error warning		*/
		ALMSetMask( Axis->AlmManager, WRN_OF,       FALSE );/* A.900 : Excessive position deviation warning				*/
		ALMSetMask( Axis->AlmManager, WRN_BEOF,     FALSE );/* A.901 : Excessive position deviation when servo is on			*/
		ALMSetMask( Axis->AlmManager, WRN_BOOT,     FALSE );/* A.941 : Power-off required user parameter change warning	*/
		ALMSetMask( Axis->AlmManager, WRN_DBOL_LVL, FALSE );/* A.921 : DB overload warning				*/
//		ALMSetMask( Axis->AlmManager, WRN_RIPPLE,   FALSE );/* A.942 : Ripple correction information mismatch warning	*//* <S1FD>Delete */
		/* ---futami---> */
		ALMSetMask( Axis->AlmManager, WRN_MBOH,     FALSE );/* A.981 : Unit internal temperature overheating warning	*/
		ALMSetMask( Axis->AlmManager, WRN_CONVOH,   FALSE );/* A.982 : Converter temperature overheat warning		*/
		ALMSetMask( Axis->AlmManager, WRN_IFOH,     FALSE );/* A.983 : Control board temperature overheating					*/
		/* <---futami--- */
		ALMSetMask( Axis->AlmManager, WRN_PREVENTMAINT, FALSE );/* A.9B0:Preventive maintenance warning				*/ /* < S154 > */
		break;
	case 0x01:	/* Does not detect warning	*/
		ALMSetMask( Axis->AlmManager, WRN_OLF,      TRUE  );/* A.910 : Overload warning					*/
		ALMSetMask( Axis->AlmManager, WRN_VIB,      TRUE  );/* A.911 : Oscillation warning						*/
		ALMSetMask( Axis->AlmManager, WRN_CTRLBRD,  TRUE  );/* A.912 : Internal temperature warning1(Control board temperature warning)*//* <S0DE> */
		ALMSetMask( Axis->AlmManager, WRN_PWRBRD,   TRUE  );/* A.913 : Internal temperature warning2(Power board temperature warning)*//* <S0DE> */
		ALMSetMask( Axis->AlmManager, WRN_RGOLF,    TRUE  );/* A.920 : Regenerative overload warning				*/
		ALMSetMask( Axis->AlmManager, WRN_DATA,     TRUE  );/* A.940 : Data abnormality warning				*/
		ALMSetMask( Axis->AlmManager, WRN_CMD,      TRUE  );/* A.950 : Command error warning				*/
		ALMSetMask( Axis->AlmManager, WRN_OPT,      TRUE  );/* A.9F0 : Optional error warning			*/
		ALMSetMask( Axis->AlmManager, WRN_PGBAT,    TRUE  );/* A.930 : Absolute value encoder battery error		*/
		ALMSetMask( Axis->AlmManager, WRN_FPGBAT,   TRUE  );/* A.930 : FPG battery error warning		*/
		ALMSetMask( Axis->AlmManager, WRN_OF,       TRUE  );/* A.900 : Excessive position deviation warning				*/
		ALMSetMask( Axis->AlmManager, WRN_BEOF,     TRUE  );/* A.901 : Excessive position deviation when servo is on			*/
		ALMSetMask( Axis->AlmManager, WRN_BOOT,     TRUE  );/* A.941 : Power-off required user parameter change warning	*/
		ALMSetMask( Axis->AlmManager, WRN_DBOL_LVL, TRUE  );/* A.921 : DB overload warning				*/
//		ALMSetMask( Axis->AlmManager, WRN_RIPPLE,   TRUE  );/* A.942 : Ripple correction information mismatch warning	*//* <S1FD>Delete */
		/* ---futami---> */
		ALMSetMask( Axis->AlmManager, WRN_MBOH,     TRUE  );/* A.981 : Unit internal temperature overheating warning	*/
		ALMSetMask( Axis->AlmManager, WRN_CONVOH,   TRUE  );/* A.982 : Converter temperature overheat warning		*/
		ALMSetMask( Axis->AlmManager, WRN_IFOH,     TRUE  );/* A.983 : Control board temperature overheating					*/
		/* <---futami--- */
		ALMSetMask( Axis->AlmManager, WRN_PREVENTMAINT,   TRUE  );/* A.9B0:Preventive maintenance warning			*/ /* < S154 > */
		break;
	default:	/* Parameter setting error		*/
		ALMSetPramError( Axis->AlmManager, pndef_b_prm8.RegNumber );
		break;
	}

	/*--------------------------------------------------------------------------------------------------*/
	/*		Warning detection mask	<S0AA> SGDV.				 						*/
	/*--------------------------------------------------------------------------------------------------*/
//<S0AA>	ALMSetMask( Axis->AlmManager, WRN_BOOT, TRUE );			/* A.941 : Power-off required user parameter change warning	*/
	uswk = (Prm->b_prm8>>12) & 0x0F;
	if( uswk <= 7 )
	{
		if( uswk & 0x01 )
		{
			ALMSetMask( Axis->AlmManager, WRN_BEOF, TRUE  ); 	/* A.901 : Excessive position deviation when servo is on		*/
		}
		if( uswk & 0x02 )
		{
			ALMSetMask( Axis->AlmManager, WRN_VIB, TRUE  ); 	/* A.911 : Oscillation warning				*/
		}
		if( uswk & 0x04 )
		{
			ALMSetMask( Axis->AlmManager, WRN_BOOT, TRUE );		/* A.941 : Power-off required user parameter change warning		*/
		}
	}
	else
	{
		ALMSetPramError( Axis->AlmManager, pndef_b_prm8.RegNumber );
	}
	/*--------------------------------------------------------------------------------------------------*/

	/*--------------------------------------------------------------------------------------------------*/
	/*		Motorless test mode alarm mask	<S1B7>						 						*/
	/*--------------------------------------------------------------------------------------------------*/
	if( Axis->MotorLessTest->MotorLessTestMode == TRUE )	/* Processing during motorless test					*/
	{
		ALMSetMask(Axis->AlmManager, WRN_DBOL_LVL, TRUE);	/* A.921 : DB overload warning				*/
	}
	/* <S21D> Start */
	/*--------------------------------------------------------------------------------------------------*/
	/*		Warning detection mask	A.924Do not mask													*/
	/*--------------------------------------------------------------------------------------------------*/
	ALMSetMask( Axis->AlmManager, WRN_EXREG, FALSE );		/* A.924 : Regeneration warning							*/
	/* <S21D> End */
#if defined(CSW_NETIF_SYNQNET_TYPE)
	/*--------------------------------------------------------------------------------------------------*/
	/*		Overspeed detection cycle setting		SGDZ-BS61											*/
	/*--------------------------------------------------------------------------------------------------*/
//	uswk = ( Cprm->b_prm8 >> 12 ) & 0x0F;
	uswk = ( Prm->b_prm8 >> 12 ) & 0x0F;
	if( uswk == 0 )
	{
		Axis->UniPrms->Bprm->OSCountMax = 32;				/* 2.000msPeriod = 62.5us×32					*/
	}
	else
	{
		Axis->UniPrms->Bprm->OSCountMax = uswk * 4 + 2;		/* 3.125msPeriod = 62.5us×(12×4 + 2)		*/
	}
#else
	Axis->UniPrms->Bprm->OSCountMax = 0;					/* 2.000msPeriod <S0E5> 						*/
#endif

}


/****************************************************************************************************/
/*																									*/
/*		Function selection application switch(Pn009)		<V259>														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn009.0 : Reservation constant (quiet mode selection)	※ΣNo function at V									*/
/*		Pn009.1 : Current control mode selection			--> IprmcalCarrerChange()							*/
/*		Pn009.2 : Speed detection moving average filter selection	 													*/
/*		Pn009.3 : Reserved constants																			*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam009( AXIS_HANDLE *Axis )
{
	UNI_PRM_STR		*UniPrm;
	UniPrm = Axis->UniPrms;
/*--------------------------------------------------------------------------------------------------*/
/*		Speed detection moving average filter selection																*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( UniPrm->Prm->b_prm9 >> 8 ) & 0x0F )
	{
		case 0x00:	/* Speed detection moving average filter disabled(The number of moving averages is determined by the encoder resolution)	*/
					UniPrm->Bprm->SpdDetMaFil = FALSE;
					break;

		case 0x01:	/* Speed detection moving average filter enabled(Number of moving averages regardless of encoder resolution16times)		*/
					UniPrm->Bprm->SpdDetMaFil = TRUE;
					break;

		default:	/* Parameter setting error		*/
					ALMSetPramError( Axis->AlmManager, pndef_b_prm9.RegNumber );
					break;
	}

}

/* <S1F5> Start */
/****************************************************************************************************/
/*																									*/
/*		Function selection application switch(Pn00A)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn00A.0 : Gr.2Selecting a stopping method when an alarm occurs													*/
/*		Pn00A.1 : Stopping method selection for forced stop				 		 									*/
/*		Pn00A.2 : Reservation parameters							 										*/
/*		Pn00A.3 : Reservation parameters																	*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam00A( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
	ALM_STOP	*AlmStop;
	FORCE_STOP	*ForceStop;

	Prm = Axis->UniPrms->Prm;		// Parameter setting data
	AlmStop = &(Axis->BeSequence->AlmStop);
	ForceStop = &(Axis->BeSequence->ForceStop);

/*--------------------------------------------------------------------------------------------------*/
/*		G2 alarm stop method selection																	*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( Prm->b_prmB >> 4 ) & 0x0F )
	{
		case 0x00:	/* Pn406The motor is decelerated and stopped using the set torque as the maximum value.								*/
					switch( Prm->b_prm1 & 0x0F )
					{
						case 0x00:	/* Pn406Decelerates and stops the motor with the set torque set to the maximum value.DBStopped state	*/
									AlmStop->conf.G2AlmStopMode = GR2STOPMODE_ZSTOPDB ;
									break;
						case 0x01:
						case 0x02:	/* Pn406The motor is decelerated to a stop with the set torque set to the maximum value and free run state.	*/
									AlmStop->conf.G2AlmStopMode = GR2STOPMODE_ZSTOPFR ;
									break;
						default:	/* Parameter setting error		*/
									ALMSetPramError( Axis->AlmManager, pndef_b_prm1.RegNumber );
									break;
					}
					break;

		case 0x01:	/* Stop DB or stop free run(Pn001.0)						*/
					AlmStop->conf.G2AlmStopMode = GR2STOPMODE_DEFAULT ;
					break;

		case 0x02:
				switch( Prm->b_prmA & 0x0F )
				{
					case 0x00:	/* Stop DB or stop free run(Pn001.0)						*/
								AlmStop->conf.G2AlmStopMode = GR2STOPMODE_DEFAULT ;
								break;

					case 0x01:	/* Pn406The motor is decelerated and stopped using the set torque as the maximum value.								*/
								switch( Prm->b_prm1 & 0x0F )
								{
									case 0x00:	/* Pn406Decelerates and stops the motor with the set torque set to the maximum value.DBStopped state	*/
												AlmStop->conf.G2AlmStopMode = GR2STOPMODE_ZSTOPDB ;
												break;
									case 0x01:
									case 0x02:	/* Pn406The motor is decelerated to a stop with the set torque set to the maximum value and free run state.	*/
												AlmStop->conf.G2AlmStopMode = GR2STOPMODE_ZSTOPFR ;
												break;
									default:	/* Parameter setting error		*/
												ALMSetPramError( Axis->AlmManager, pndef_b_prm1.RegNumber );
												break;
								}
								break;

					case 0x02:	/* Pn406The motor is decelerated to a stop with the set torque set to the maximum value and free run state.				*/
								AlmStop->conf.G2AlmStopMode = GR2STOPMODE_ZSTOPFR ;
								break;

					case 0x03:	/* Pn30A												*/
								switch( Prm->b_prm1 & 0x0F )
								{
									case 0x00:	/* Pn30A&DBStopped state					*/
												AlmStop->conf.G2AlmStopMode = GR2STOPMODE_LDSTOPDB ;
												break;
									case 0x01:
									case 0x02:	/* Pn30AThe motor decelerates to a stop and coasts according to the				*/
												AlmStop->conf.G2AlmStopMode = GR2STOPMODE_LDSTOPFR ;
												break;
									default:	/* Parameter setting error		*/
												ALMSetPramError( Axis->AlmManager, pndef_b_prm1.RegNumber );
												break;
								}
								break;

					case 0x04:	/* Pn30AThe motor decelerates to a stop and coasts according to the								*/
								AlmStop->conf.G2AlmStopMode = GR2STOPMODE_LDSTOPFR ;
								break;

					default:	/* Parameter setting error		*/
								ALMSetPramError( Axis->AlmManager, pndef_b_prmA.RegNumber );
								break;
				}
				break;
		default:	/* Parameter setting error		*/
					ALMSetPramError( Axis->AlmManager, pndef_b_prmB.RegNumber );
					break;

	}

/*--------------------------------------------------------------------------------------------------*/
/*		Stopping method selection for forced stop																	*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( Prm->b_prmA >> 4 ) & 0x0F )
	{
		case 0x00:	/* Stop DB or stop free run(Pn001.0)						*/
					ForceStop->conf.ForceStopMode = FSTOPMODE_DEFAULT ;
					break;

		case 0x01:	/* Pn406The motor is decelerated and stopped using the set torque as the maximum value.								*/
					switch( Prm->b_prm1 & 0x0F )
					{
						case 0x00:	/* Pn406Decelerates and stops the motor with the set torque set to the maximum value.DBStopped state	*/
									ForceStop->conf.ForceStopMode = FSTOPMODE_ZSTOPDB ;
									break;
						case 0x01:
						case 0x02:	/* Pn406The motor is decelerated to a stop with the set torque set to the maximum value and free run state.	*/
									ForceStop->conf.ForceStopMode = FSTOPMODE_ZSTOPFR ;
									break;
						default:	/* Parameter setting error		*/
									ALMSetPramError( Axis->AlmManager, pndef_b_prm1.RegNumber );
									break;
					}
					break;

		case 0x02:	/* Pn406The motor is decelerated to a stop with the set torque set to the maximum value and free run state.				*/
					ForceStop->conf.ForceStopMode = FSTOPMODE_ZSTOPFR ;
					break;

		case 0x03:	/* Pn30A												*/
					switch( Prm->b_prm1 & 0x0F )
					{
						case 0x00:	/* Pn30A&DBStopped state					*/
									ForceStop->conf.ForceStopMode = FSTOPMODE_LDSTOPDB ;
									break;
						case 0x01:
						case 0x02:	/* Pn30AThe motor decelerates to a stop and coasts according to the				*/
									ForceStop->conf.ForceStopMode = FSTOPMODE_LDSTOPFR ;
									break;
						default:	/* Parameter setting error		*/
									ALMSetPramError( Axis->AlmManager, pndef_b_prm1.RegNumber );
									break;
					}
					break;

		case 0x04:	/* Pn30AThe motor decelerates to a stop and coasts according to the								*/
					ForceStop->conf.ForceStopMode = FSTOPMODE_LDSTOPFR ;
					break;

		default:	/* Parameter setting error		*/
					ALMSetPramError( Axis->AlmManager, pndef_b_prmA.RegNumber );
					break;
	}
}
/* <S1F5> End */

/****************************************************************************************************/
/*																									*/
/*		Function selection application switch(Pn00B)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn00B.0 : Operator user constant display selection													*/
/*		Pn00B.1 : G2 alarm stop method selection			 		 									*/
/*		Pn00B.2 : Power input selection for SERVOPACK with three-phase input specifications	 										*/
/*		Pn00B.3 : Reserved constants (how to use semi-closed encoders)										*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam00B( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
//	ALM_STOP	*AlmStop;

	Prm = Axis->UniPrms->Prm;		// Parameter setting data
//	AlmStop = &(Axis->BeSequence->AlmStop);

/* <S113> Start*/
/*--------------------------------------------------------------------------------------------------*/
/*	Bit 0: Operator user constant display selection															*/
/*--------------------------------------------------------------------------------------------------*/
		if( (Prm->b_prmB & 0x000F) > 2 )
		{
			/* Parameter setting error */
			ALMSetPramError( Axis->AlmManager, pndef_b_prmB.RegNumber );
			
		}
/* <S113> End */
#if 0	/* <S1F5> Start	*/
/*--------------------------------------------------------------------------------------------------*/
/*		G2 alarm stop method selection																	*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( Prm->b_prmB >> 4 ) & 0x0F )
	{
		case 0x00:	/* Zero speed stop										*/
					AlmStop->conf.G2AlmStopMode = FALSE ;
					break;

		case 0x01:	/* Stop DB or stop free run(Pn001.0)	*/
					AlmStop->conf.G2AlmStopMode = TRUE ;
					break;

		default:	/* Parameter setting error		*/
					ALMSetPramError( Axis->AlmManager, pndef_b_prmB.RegNumber );
					break;
	}
#endif /* <S1F5> End	*/
}


/****************************************************************************************************/
/*																									*/
/*		Function selection application switch(Pn00D)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn00B.3 : Overtravel warning detection selection												*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam00D( AXIS_HANDLE *Axis )
{
	PRMDATA			*Prm;
	OT_SEQ_DATA		*OverTravelSeq;

	Prm = Axis->UniPrms->Prm;
	OverTravelSeq = &(Axis->BeSequence->OtSeqData);

/*--------------------------------------------------------------------------------------------------*/
/*		Overtravel warning detection selection															*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( Prm->b_prmD >> 12 ) & 0x0F )
	{
		case 0x00:	/* OTDoes not detect warning */
			OverTravelSeq->conf.OtWarningSw = FALSE;
			break;

		case 0x01:	/* OTDetects warning */
			OverTravelSeq->conf.OtWarningSw = TRUE;
			break;

		default:	/* Parameter setting error */
			ALMSetPramError( Axis->AlmManager, pndef_b_prmD.RegNumber );
			break;
	}
}
/* <S21D> Start */
/*****************************************************************************
  Description: Function selection application switch(Pn00E)
------------------------------------------------------------------------------
  Parameters:
	Axis;		Pointer to Axis Handle
  Return:
	none
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static	void	IprmcalBitParam00E( AXIS_HANDLE *Axis )
{
	CMN_PRMDATA			*Cprm;
	BPRMDAT				*Bprm;
	CHECK_MAIN_POWER	*PowerChecker;
	REGENE_CTRL			*RegCtrl;

	Cprm  = Axis->UniPrms->Cprm;	/* Unit common parameter setting data */
	Bprm = Axis->UniPrms->Bprm;		/* Initial parameter definition(Changes take effect when the power is turned back on.) */
	RegCtrl = &(Axis->PowerManager->RegeneCtrl);
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);

/*--------------------------------------------------------------------------------------------------*/
/*		Pn00E.1 : DCRapid discharge is selected when power is input.														*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (Cprm->b_prmE >> 4) & 0x0F )
	{
	case 0x00: /* DCRapid discharge is not performed when power is input. */
		PowerChecker->conf.DcDischarge = FALSE;
		break;

	case 0x01: /* DCWhen power is input, rapid discharge is performed according to the state of the converter ready signal. */
		if( PowerChecker->conf.Dcbus == TRUE )
		{
			PowerChecker->conf.DcDischarge = TRUE;
		}
		else
		{
			PowerChecker->conf.DcDischarge = FALSE;
		}
		break;

	default:
		ALMSetPramError( Axis->AlmManager, pndef_b_prmE.RegNumber );
		break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn00E.3 : How to use the regeneration synchronization function															*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (Cprm->b_prmE >> 12) & 0x0F )
	{
	case 0x00: /* Regeneration synchronization function is not used */
		RegCtrl->conf.RegSyn = REGSYNOFF;
		break;

	case 0x01: /* PNUse regeneration synchronization function in bus common system */
		RegCtrl->conf.RegSyn = PNREGSYN;
		break;

	case 0x02: /* PNUse external regeneration function in bus common system */
		RegCtrl->conf.RegSyn = PNEXREG;
		break;

	default:
		ALMSetPramError( Axis->AlmManager, pndef_b_prmE.RegNumber );
		break;
	}
}
/* <S21D> End */

/* < S154 > Start */
/*****************************************************************************
  Description: Function selection application switch(Pn00F)
------------------------------------------------------------------------------
  Parameters:
	Axis;		Pointer to Axis Handle
  Return:
	none
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static	void	IprmcalBitParam00F( AXIS_HANDLE *Axis )
{
	CMN_PRMDATA		*Cprm;
	CHECK_MAIN_POWER	*PowerChecker;

	Cprm  = Axis->UniPrms->Cprm;		/* Parameter setting data */
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);

/*--------------------------------------------------------------------------------------------------*/
/*		Pn00F.0 : Preventive maintenance warning selection															*/
/*--------------------------------------------------------------------------------------------------*/
	switch( Cprm->b_prmF & 0x0FU )
	{
	case 0x00U: /* Do not detect preventive maintenance warning */
		PowerChecker->conf.PreWarningSw = FALSE;
		break;
	case 0x01U: /* Detect preventive maintenance warning */
		PowerChecker->conf.PreWarningSw = TRUE;
		break;
	default:
		ALMSetPramError( Axis->AlmManager, pndef_b_prmF.RegNumber );
		break;
	}
}
/* < S154 > End  */

/****************************************************************************************************/
/*																									*/
/*		Gain-related application switch(Pn10B)																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn10B.0 : Mode switch selection switch --> PcalBitParamMSW()									*/
/*		Pn10B.1 : Speed loop control selection switch														*/
/*		Pn10B.2 : Position loop control selection switch														*/
/*		Pn10B.3 : Damping control selection switch																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam10B( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
	BOOL		PrmSetting; /* <S066> */

	Prm = Axis->UniPrms->Prm;		// Parameter setting data

/*--------------------------------------------------------------------------------------------------*/
/*		Pn10B.1 : Speed loop control selection setting															*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( Prm->gnmode >> 4 ) & 0x0F )
	{
		case 0x00:
			Axis->BaseControls->CtrlCmdPrm.IPSpdControl = FALSE;
			break;
		case 0x01:
			Axis->BaseControls->CtrlCmdPrm.IPSpdControl = TRUE;
			break;
		default :
			ALMSetPramError( Axis->AlmManager, pndef_gnmode.RegNumber );
			break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn10B.3 : Phase compensation speed observer setting														*/
/*--------------------------------------------------------------------------------------------------*/
	PrmSetting = EhVobsCalculatePrmSW( &(Axis->BaseControls->EhVobsCtrl.conf.EhVobsUse),
									   Axis->BaseControls->TuneLessCtrl.conf.TuningLessUse,
									   (Prm->gnmode >> 12) );

	if( PrmSetting == PRM_SETTING_NG )
	{
		ALMSetPramError( Axis->AlmManager, pndef_gnmode.RegNumber );
	}
}


/****************************************************************************************************/
/*																									*/
/*		Position control command form selection switch(Pn200)															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn200.0 : Command pulse form selection switch --> JL056HW.C											*/
/*		Pn200.1 : Clear signal format selection switch --> V850E.C											*/
/*		Pn200.2 : Clear action selection switch	 														*/
/*		Pn200.3 : Filter selection switch																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam200( AXIS_HANDLE *Axis )
{
#if SVFSEL_CMDPLS_USE == 1		/* <S182>Code organization */
	if(OK != KpiPcalPerClrMode( Axis->pPlsIfV, Axis->UniPrms->Prm->prstsw ) ) /* Pn200: Perr Clear Mode */
	{
		ALMSetPramError( Axis->AlmManager, pndef_prstsw.RegNumber );
	}
#endif
}


/****************************************************************************************************/
/*																									*/
/*		Position control function switch(Pn207)																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn207.0 : Position command filter selection switch														*/
/*		Pn207.1 : Position control option selection switch													*/
/*		Pn207.2 : Backlash correction selection switch														*/
/*		Pn207.3 : /COINOutput timing selection switch													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam207( AXIS_HANDLE *Axis )
{
	ULONG		ucwk;
	PRMDATA		*Prm;
//	IPRMDAT		*Iprm;

	Prm  = Axis->UniPrms->Prm;		// Parameter setting data
//	Iprm = Axis->prm_hndl_ptr->iprm_ptr;		// Initial parameter definition(Changes take effect when the power is turned back on.)

/*--------------------------------------------------------------------------------------------------*/
/*		Pn207.0 : Position command filter selection setting															*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
	ucwk = Prm->cmfilsw & 0x0F;
	switch( ucwk )
	{
		case 0x00:					/* Acceleration/deceleration filter												*/
		case 0x01:					/* Moving average filter												*/
			Iprm->PcmdFilter = (UCHAR)ucwk;
			break;
		default :
			ALMSetPramError( Axis->AlmManager, &pndef_cmfilsw );
			break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn207.1 : Position control option selection setting														*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( Prm->cmfilsw >> 4 ) & 0x0F )
	{
		case 0x00:
			Iprm->VrefSpdFF = FALSE;
			break;
		case 0x01:
			Iprm->VrefSpdFF = TRUE;
			break;
		default :
			ALMSetPramError( Axis->AlmManager, &pndef_cmfilsw );
			break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Pn207.2 : Backlash correction selection setting															*/
/*--------------------------------------------------------------------------------------------------*/
	ucwk = ( Prm->cmfilsw >> 8 ) & 0x0F;
	switch( ucwk )
	{
		case 0x00:					/* Backlash correction disabled											*/
		case 0x01:					/* Backlash correction in the forward rotation direction									*/
		case 0x02:					/* Backlash correction in reverse direction									*/
			Iprm->BacklashDir = (UCHAR)ucwk;
			break;
		default :
			ALMSetPramError( Axis->AlmManager, &pndef_cmfilsw );
			break;
	}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Pn207.3 : /COINOutput timing selection setting														*/
/*--------------------------------------------------------------------------------------------------*/
	ucwk = (Prm->cmfilsw >> 12) & 0x0F;
	switch( ucwk )
	{
		case 0x00:
		case 0x01:
		case 0x02:
			Axis->BaseControls->PosManager.conf.CoinOutSel = (UCHAR)ucwk;
			break;
		default :
			ALMSetPramError( Axis->AlmManager, pndef_cmfilsw.RegNumber );
			break;
	}
}

#if (CSW_SVCF_BACKLASHCOMP == TRUE)
/****************************************************************************************************/
/*																									*/
/*		Position control extended function switch(Pn230)																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn230.0 : Backlash correction direction																*/
/*		Pn230.1 : Reserved constants																			*/
/*		Pn230.2 : Reserved constants																			*/
/*		Pn230.3 : Reserved constants																			*/
/*																									*/
/****************************************************************************************************/
void	IprmcalBitParam230( AXIS_HANDLE *Axis )
{
	UCHAR		ucwk;
	PRMDATA		*Prm;
	BLSHCMP		*BlshCmp;

	Prm = Axis->UniPrms->Prm;
	BlshCmp = &(Axis->BaseControls->PosManager.BlshCmp);

/*--------------------------------------------------------------------------------------------------*/
/*		Pn230.0 : Backlash correction direction																*/
/*--------------------------------------------------------------------------------------------------*/
	ucwk = (Prm->cmctrlexsw) & 0x0F;
	switch( ucwk )
	{
	case 0x00: /* Correct with the forward rotation direction command */
	case 0x01: /* Correct with reverse direction command */
		BlshCmp->conf.BacklashDir = ucwk;
		break;
	default :
		ALMSetPramError( Axis->AlmManager, pndef_cmctrlexsw.RegNumber );
		break;
	}
}
#endif

#if 0	/* Pn408.1:Change to online */
/****************************************************************************************************/
/*																									*/
/*		Torque related function switch(Pn408)																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn408.0 : Notch filter selection 1 switch (online)--> pncal_tfuncsw()						*/
/*		Pn408.1 : Speed limit selection switch (offline)												*/
/*		Pn408.2 : Notch filter selection 2 switch (online)--> pncal_tfuncsw()						*/
/*		Pn408.3 : Disturbance observer selection switch (offline) (not used)								*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitParam408( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
	IPRMDAT		*Iprm;

	Prm  = Axis->UniPrms->Prm;		// Parameter setting data
	Iprm = Axis->UniPrms->iprm_ptr;		// Initial parameter definition(Changes take effect when the power is turned back on.)

/*--------------------------------------------------------------------------------------------------*/
/*		Pn408.1 : Speed limit selection setting																	*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( Prm->tfuncsw >> 4 ) & 0x0F )
	{
		case 0x00:	Iprm->OvrSpdLimit = FALSE; break;
		case 0x01:	Iprm->OvrSpdLimit = TRUE;  break;
		default :	ALMSetPramError( Axis->AlmManager, pndef_tfuncsw.RegNumber ); break;
	}

}
#endif

/* <S14B> Start */
/****************************************************************************************************/
/*		Pn423 : Speed ripple compensation function selection switch 		[-]										*/
/****************************************************************************************************/
/*																									*/
/*		Pn423.0 : Selecting whether or not to use the speed ripple compensation function				(online)				*/
/*		Pn423.1 : Speed ripple compensation information mismatch warning(A.942)Detection selection	(off-line)				*/
/*		Pn423.2 : Selecting the threshold conditions for enabling the speed ripple compensation function		(off-line)				*/
/*		Pn423.3 : Speed ripple compensation function phase selection (not disclosed)				(off-line)				*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalBitVelRippleCompSW( AXIS_HANDLE *Axis )/* <S1FD> */
{
//	ALMMAN_HANDLE	*AlmManager;	/* <S1FD> */
	PRMDATA			*Prm;
	RPLCMP			*RippleCmp;
	ULONG			MotorSnReadRslt;

//	AlmManager	= Axis->AlmManager;	/* <S1FD> */
	Prm			= Axis->UniPrms->Prm;
	RippleCmp	= &(Axis->BaseLoops->RippleCmp);

/*--------------------------------------------------------------------------------------------------*/
/*	Pn423.1 : Speed ripple compensation information mismatch warning(A.942)Detection selection									*/
/*--------------------------------------------------------------------------------------------------*/
	if( (( Prm->RippleCompSw >> 4 ) & 0x0F ) == 0x00 )
	{
		/* Detects warning		*/
		ALMSetMask( Axis->AlmManager, WRN_RIPPLE, FALSE );/* A.942 : Ripple correction information mismatch warning	*/
		RippleCmp->RplCmpPrm.RippleWrnMode = FALSE;	/*<S1FD>*/
	}
	else
	{
		/* Does not detect warning		*/
		ALMSetMask( Axis->AlmManager, WRN_RIPPLE, TRUE  );/* A.942 : Ripple correction information mismatch warning	*/
		RippleCmp->RplCmpPrm.RippleWrnMode = TRUE;	/*<S1FD>*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*	Pn423.2 : Selecting the threshold conditions for enabling the speed ripple compensation function										*/
/*--------------------------------------------------------------------------------------------------*/
	if( (( Prm->RippleCompSw >> 8 ) & 0x0F ) == 0x00 )
	{
		/* Threshold conditions for enabling the speed ripple compensation function = "Command speed" */
		RippleCmp->RippleCmpThresholdSel = CMD_SPD;
	}
	else
	{
		/* Threshold conditions for enabling the speed ripple compensation function = "Feedback speed" */
		RippleCmp->RippleCmpThresholdSel = FB_SPD;
	}

/*--------------------------------------------------------------------------------------------------*/
/*	Pn423.3 : Speed ripple compensation function phase selection (not disclosed)	<TODO>										*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
	if( (( Prm->RippleCompSw >> 12 ) & 0x0F ) == 0x00 )
	{
		/* Motor electrical angle (0~360°) using */

	}
	else
	{
		/* Motor mechanical angle (0~360°) using (Invalid for linear motor) */

	}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*	Pn423.0 : Selecting whether or not to use the speed ripple compensation function												*/
/*--------------------------------------------------------------------------------------------------*/
	if( (Prm->RippleCompSw & 0x0F) == 0x00 )
	{
		/* Speed ripple compensation information status : Normal */
		RippleCmp->var.RippleWrnState = TRUE;

		/* Pn423.0 = 0 : Do not use the speed ripple compensation function */
		RippleCmp->RplCmpPrm.RplCmpUse = FALSE;
	}
	else
	{
		/* Motor serial obtained this timeNo., Previously acquired motor serialNo. */
		MotorSnReadRslt = ChkMotorSerialNo( Axis->AxisNo, Axis->MencV );
//		if( MotorSnReadRslt != 0 )/*<S1FD>*/
		if( (MotorSnReadRslt != 0) && (RippleCmp->RplCmpPrm.RippleWrnMode == FALSE) )
		{	/* Motor serialNo.is different && Pn423.1=0  */
//			ALMSetServoAlarm( AlmManager, WRN_RIPPLE );
			/* Speed ripple compensation information status : A.942Generate */
			RippleCmp->var.RippleWrnState = FALSE;

			/* Pn423.0 = 0 : Do not use the speed ripple compensation function */
			RippleCmp->RplCmpPrm.RplCmpUse = FALSE;
		}
		else
		{
			/* Speed ripple compensation information status : Normal */
			RippleCmp->var.RippleWrnState = TRUE;

			/* Pn423.0 = 1 : Use the speed ripple compensation function*/
			RippleCmp->RplCmpPrm.RplCmpUse = TRUE;
		}
	}
//	return;/* <S1FD> */
}

/****************************************************************************************************/
/*		Pn427 : Speed ripple compensation effective speed 		[min-1]												*/
/****************************************************************************************************/
static	void	IprmcalVelRippleCompSpd( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;
	RPLCMP		*RippleCmp;

	Prm			= Axis->UniPrms->Prm;
	Bprm = Axis->UniPrms->Bprm;
	RippleCmp	= &(Axis->BaseLoops->RippleCmp);


	/* Speed [0.1r/min, 0.1mm/s] --> NormalizedSpeed [2^24/OvrSpd] */
	RippleCmp->VelRippleCompSpd = BprmSpeedLevelCal( Bprm, 10 * Prm->VelRippleCompSpd, 0 );
}
/* <S14B> End */
/* <S21D> Start */
/*****************************************************************************
  Description: Output signal selection4(Pn514)
------------------------------------------------------------------------------
  Parameters:
	Axis;		Pointer to Axis Handle
  Return:
	none
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static	void	IprmcalBitParam514( AXIS_HANDLE *Axis )
{
	PRMDATA				*Prm;
	CHECK_MAIN_POWER	*PowerChecker;

	Prm  = Axis->UniPrms->Prm;		/* Parameter setting data */
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);

	switch( (Prm->outsel4 >> 4) & 0x0F )
	{
		case 0x00:
			PowerChecker->conf.CapaLinkFlag = FALSE;
			break;
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
			PowerChecker->conf.CapaLinkFlag = TRUE;
			break;
		default:
		ALMSetPramError( Axis->AlmManager, pndef_outsel4.RegNumber );
		break;
	}
}
/* <S21D> End*/

/****************************************************************************************************/
/*																									*/
/*		DC power supply voltage monitoring level calculation																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the DC power supply voltage monitoring level.													*/
/*																									*/
/*			1. Voltage level																			*/
/*				1) OV detection level																	*/
/*				2) UV detection level																	*/
/*				3) UV detection filter time constant															*/
/*				4) Regeneration on level/Off level														*/
/*																									*/
/*			2. Parameter																			*/
/*				1) PnE70 : P-NVoltage detection level														*/
/*		 		2) PnE71 : OVDetection level   [V]														*/
/*				3) PnE72 : Regeneration on level [V]														*/
/*		 		4) PnE73 : UVDetection level   [V]														*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalVoltageCheckLevel( AXIS_HANDLE *Axis )
{
	LONG				s;
	CMN_PRMDATA			*Cprm;
	BPRMDAT				*Bprm;
	REGENE_CTRL			*RegCtrl;
	DET_DC_VOLT			*DetectDc;
	CHECK_MAIN_POWER	*PowerChecker;

	s = 0;
	Cprm  = Axis->UniPrms->Cprm;		// Parameter setting data
	Bprm = Axis->UniPrms->Bprm;		// Initial parameter definition(Changes take effect when the power is turned back on.)
	RegCtrl = &(Axis->PowerManager->RegeneCtrl);
	DetectDc = &(Axis->PowerManager->DetectDcVolt);
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);

	/*------------------------------------------------------------------------------------------*/
	/*		OVLevel calculation																		*/
	/*------------------------------------------------------------------------------------------*/
	/* When using regeneration */
	PowerChecker->conf.OvLevel = (USHORT)(Cprm->ovlvl * Bprm->VdetMax / Cprm->pnvoltgn);
	/* Regeneration not used */
	PowerChecker->conf.NonRegOvLevel = (USHORT)(Cprm->NonRegOvlvl * Bprm->VdetMax / Cprm->pnvoltgn); /* <S0F7> */

	/*------------------------------------------------------------------------------------------*/
	/*		UVLevel calculation																		*/
	/*------------------------------------------------------------------------------------------*/
	PowerChecker->conf.UvLevel = (USHORT)(Cprm->uvlvl * Bprm->VdetMax / Cprm->pnvoltgn);
	PowerChecker->conf.UvWrnLevel = (USHORT)(Cprm->UvWrnLvl * Bprm->VdetMax / Cprm->pnvoltgn);
	PowerChecker->conf.UvFilter = Cprm->uvfil;

	/*------------------------------------------------------------------------------------------*/
	/*		Main circuit voltage detection correction Calculation of reference voltage level																		*/
	/*------------------------------------------------------------------------------------------*/
	PowerChecker->conf.TestDcLevel = (USHORT)(Cprm->DcBaseLvl * Bprm->VdetMax / Cprm->pnvoltgn);

	/*------------------------------------------------------------------------------------------*/
	/*		Regeneration level calculation																	*/
	/*------------------------------------------------------------------------------------------*/
	/* <S21D> Start */
#if 0
	RegCtrl->conf.RegOnLevel  = (USHORT)( Cprm->regonlvl * Bprm->VdetMax / Cprm->pnvoltgn );
#else
	if( ((Cprm->b_prmE >> 12) & 0x0F) == PNREGSYN )
	{	/* When regeneration synchronization is enabled */
		RegCtrl->conf.RegOnLevel = (USHORT)( (Cprm->regonlvl + Cprm->regsynonlvl) * (Bprm->VdetMax / Cprm->pnvoltgn) );
	}
	else
	{
//		RegCtrl->conf.RegOnLevel = (USHORT)( Cprm->regonlvl * (Bprm->VdetMax / Cprm->pnvoltgn) );/* <S21E> */
		RegCtrl->conf.RegOnLevel = (USHORT)( (Cprm->regonlvl * Bprm->VdetMax) / Cprm->pnvoltgn );/* <S21E> */
	}
#endif
	/* <S21D> End */
	RegCtrl->conf.RegOffLevel = (USHORT)( ((Cprm->regonlvl - Cprm->regoffhys) * Bprm->VdetMax) / Cprm->pnvoltgn );

	/*------------------------------------------------------------------------------------------*/
	/*		ADNormalization constant for value read by conversion method													*/
	/*------------------------------------------------------------------------------------------*/
	/* 2010.06.16 Y.Oka (>_<) for INGRAM PnE70[V]At the time of inputA/DThe value should be parameterized. IncidentallyVdetMaxtoo! */
//	Iprm->VdetAdNorm = MlibPcalKxgain( Bprm->VdetMax, 1, Bprm->VdetAdMax, &s, 24 );
//	DetectDc->conf.VdetAdNorm = MlibPcalKxgain( Bprm->VdetMax, 1, PRM_VDETADMAX, &s, 24 );
	DetectDc->conf.VdetAdNorm = MlibPcalKxgain( Bprm->VdetMax, 1, Cprm->VdetADMax, &s, 24 );

	/*------------------------------------------------------------------------------------------*/
	/*		Obtaining the maximum voltage detection value																*/
	/*------------------------------------------------------------------------------------------*/
	DetectDc->conf.VdetMaxVolt = Bprm->VdetMax;

	/* Main circuit detectionAD */
	/* 2011.03.29 Y.Oka ScanBSupports cycle change */
//	DetectDc->conf.VdetAdFil = MlibPcalKf1gain( Cprm->vdetadfil, (2 * Bprm->SvCycleUs), 0 );
	PcmCalculateVdetAdFil( DetectDc, Cprm->vdetadfil, Bprm->SvCycleUs );

	/*------------------------------------------------------------------------------------------*/
	/*		Rapid discharge completion level Rapid discharge time calculation								<S1C6>			*/
	/*------------------------------------------------------------------------------------------*/
	PowerChecker->conf.DischCheckVlt = (USHORT)(DISCH_CHK_VOLT * Bprm->VdetMax / Cprm->pnvoltgn);	  /* <S1D1> */
	PowerChecker->conf.DischCompVlt = (USHORT)(Cprm->DischCompVlt * Bprm->VdetMax / Cprm->pnvoltgn);  /* <S1D1> */
	PowerChecker->conf.MainDischActTime = KPI_SCANC_MS( Cprm->ChgWaitTime );

/*--------------------------------------------------------------------------------------------------*/
/*							P-NVoltage detection level[V]   													*/
/*		KioP.Kdcvolt = -------------------------- 													*/
/*						  		VdetMax	   			VdetMax: Normalization constant								*/
/*--------------------------------------------------------------------------------------------------*/
	DetectDc->conf.Kdcvolt = MlibScalKxgain( Cprm->pnvoltgn, 1, Bprm->VdetMax, NULL, 24 );

}

/****************************************************************************************************/
/*																									*/
/*		DC power supply voltage monitoring time calculation <S053>																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate the DC power supply voltage monitoring time.														*/
/*																									*/
/*			1. Voltage monitoring time																			*/
/*				1) DC charging completion waiting time																*/
/*				2) Power ready waiting time																*/
/*				3) Ground fault waiting time before turning on																*/
/*																									*/
/*			2. Parameter																			*/
/*				1) PnE88 : DC charging completion waiting time [ms]												*/
/*		 		2) PnE89 : Power ready waiting time [ms]												*/
/*				3) PnE8A : Ground fault waiting time before turning on   [ms]												*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalVoltageCheckTime( AXIS_HANDLE *Axis )
{
	CMN_PRMDATA			*Cprm;
	CHECK_MAIN_POWER	*PowerChecker;

	Cprm  = Axis->UniPrms->Cprm;
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);

	/*------------------------------------------------------------------------------------------*/
	/*		Calculation of DC charging completion waiting time															*/
	/*------------------------------------------------------------------------------------------*/
	PowerChecker->conf.PowerChargeWaitTime = KPI_SCANC_MS(Cprm->ChgWaitTime);

	/*------------------------------------------------------------------------------------------*/
	/*		Power ready waiting time calculation															*/
	/*------------------------------------------------------------------------------------------*/
	PowerChecker->conf.PowerReadyWaitTime = KPI_SCANC_MS(Cprm->PrdyWaitTime);

	/*------------------------------------------------------------------------------------------*/
	/*		Calculating ground fault waiting time before turning on															*/
	/*------------------------------------------------------------------------------------------*/
//	PowerChecker->conf.GndFaultWaitTime = KPI_SCANC_MS(Cprm->GFWaitTime);

	/*------------------------------------------------------------------------------------------*/
	/*		@@ Parameter consistency check															*/
	/*		PowerReadyWaitTime <= PowerChargeWaitTime In the case of sequence mismatch		*/
	/*		(ChargeOkbutTRUEwill no longer be), so it is treated as a parameter calculation error.				*/
	/*------------------------------------------------------------------------------------------*/
	if ( ( PowerChecker->conf.PowerChargeWaitTime == 0 ) || 
		( PowerChecker->conf.PowerReadyWaitTime <= PowerChecker->conf.PowerChargeWaitTime ) )
	{
		/* A.040 : Parameter setting error */
		ALMSetPramError( Axis->AlmManager, pndef_ChgWaitTime.RegNumber );
	}
}


/****************************************************************************************************/
/*																									*/
/*		Servopack built-in fan stoppage detection time <S0A3>													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate fan stop detection time.														*/
/*																									*/
/*			1. Parameter																			*/
/*				1) PnEE5 : Fan stop alarm transition time 0.1[s]										*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalFanLockDetectTime( AXIS_HANDLE *Axis )
{
	CMN_PRMDATA			*Cprm;
	Cprm  = Axis->UniPrms->Cprm;

	/* Initialization of variable for calculation */
	MlibResetLongMemory( &(Axis->PowerManager->FanLock.var), sizeof( Axis->PowerManager->FanLock.var )/4 );

	Axis->PowerManager->FanLock.conf.FanLockDetectTime = KPI_SCANC_MS( Cprm->FanLockTime * 100 );
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Calculation of amplifier & motor overload level															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		LpxPcalOverLoadLevelAmpMot(				Amplifier & motor overload level calculation function(Low speedOL:SGDSMethod)	*/
/*			OLPRM	*OlPrm,						Overload parameter									*/
/*			CHKOLP	*ChkOLP1,					Overload check parameters(Computation result)				*/
/*			CHKOLP	*ChkOLP2,					Overload check parameters(For abnormality setting)			*/
/*			CHKOLP	*ChkOLP3					Overload check parameters(For detection level gain calculation)		*/
/*			PRMDATA *prm_ptr																		*/
/*			IPRMDAT *iprm_ptr	);																	*/
/*																									*/
/*		Supplement)ChkOLP1 == ChkOLP2, the exception handling is	ChkOLP2⇒ChkOLP1Copy processing obtains			*/
/*			  Do not do this and it will be an error.																*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
//static	LONG	LpxPcalOverLoadLevelAmpMot( OLPRM *OlPrm, CHECK_OL *ChkOLP1, CHECK_OL *ChkOLP2,
//											CHECK_OL *ChkOLP3, PRMDATA *prm_ptr, KSGAIN	LinearCoolRate100PerGain )		/* <S154> */
static	LONG	LpxPcalOverLoadLevelAmpMot( OLPRM *OlPrm, CHECK_OLP *ChkOLP1, CHECK_OLP *ChkOLP2,
											CHECK_OLP *ChkOLP3, PRMDATA *prm_ptr, KSGAIN	LinearCoolRate100PerGain )	/* <S154> */
{
	LONG	Ix;
	LONG	Ibsqr;
	LONG	Ixsqr;
	LONG	Iysqr;
	LONG	Izsqr;
	LONG	rc;
	LONG	wk;
	LONG	cflg;

	rc = TRUE;

	/********************************************************************************************/
	/*		Startup condition setting																		*/
	/*		  FLASE :ChkOLP2														*/
	/*		  TRUE  :ChkOLP2														*/
	/********************************************************************************************/
	if( ChkOLP1 != ChkOLP2 )
	{
		cflg = TRUE;
	}
	else
	{
		cflg = FALSE;
	}

	/********************************************************************************************/
	/*		Continuous overload level calculation																*/
	/********************************************************************************************/
	/*																							*/
	/*		Ibsqr = Ibse * Ibse																	*/
	/*																							*/
	/*				  (Imid^2 - Ibse^2) * Tmid[0.1s]											*/
	/*		AlmLvl = --------------------------------											*/
	/*						  Cycle[ms]															*/
	/*																							*/
	/********************************************************************************************/
	/*------------------------------------------------------------------------------------------*/
	/* 	Check base current:	OlPrm->Ibse														*/
	/*------------------------------------------------------------------------------------------*/
	if( OlPrm->Ibse > 15000 )						/* Match the following value to the setting value of one side.			*/
	{
		if( cflg == TRUE )
		{
#if 1	/* <S154> */
			/* Continuous OL base current  */
			ChkOLP1->Ibsqrmid = ChkOLP2->Ibsqrmid;
			/* Continuous OL detection level */
			ChkOLP1->AlmLvlmid = ChkOLP2->AlmLvlmid;
			/* Continuous OL warning level  */
			ChkOLP1->WrnLvlmid = ChkOLP2->WrnLvlmid;
			/* Continuous OL detection level gain */
			ChkOLP1->OlLvlmidGain = ChkOLP2->OlLvlmidGain;
			/* Instantaneous OL base current  */
			ChkOLP1->Ibsqrmax = ChkOLP2->Ibsqrmax;
			/* Instantaneous OL detection level */
			ChkOLP1->AlmLvlmax = ChkOLP2->AlmLvlmax;
			/* Instantaneous OL warning level */
			ChkOLP1->WrnLvlmax = ChkOLP2->WrnLvlmax;
			/* Instantaneous OL detection level gain */
			ChkOLP1->OlLvlmaxGain = ChkOLP2->OlLvlmaxGain;
#else	/* <S154> */
			/* Continuous OL base current  */
			ChkOLP1->conf.Ibsqrmid = ChkOLP2->conf.Ibsqrmid;
			/* Continuous OL detection level */
			ChkOLP1->conf.AlmLvlmid = ChkOLP2->conf.AlmLvlmid;
			/* Continuous OL warning level  */
			ChkOLP1->conf.WrnLvlmid = ChkOLP2->conf.WrnLvlmid;
			/* Continuous OL detection level gain */
			ChkOLP1->conf.OlLvlmidGain = ChkOLP2->conf.OlLvlmidGain;
			/* Instantaneous OL base current  */
			ChkOLP1->conf.Ibsqrmax = ChkOLP2->conf.Ibsqrmax;
			/* Instantaneous OL detection level */
			ChkOLP1->conf.AlmLvlmax = ChkOLP2->conf.AlmLvlmax;
			/* Instantaneous OL warning level */
			ChkOLP1->conf.WrnLvlmax = ChkOLP2->conf.WrnLvlmax;
			/* Instantaneous OL detection level gain */
			ChkOLP1->conf.OlLvlmaxGain = ChkOLP2->conf.OlLvlmaxGain;
#endif	/* <S154> */

			/*----------------------------------------------------------------------------------*/
			/*	Motor free-running air cooling coefficient setting															*/
			/*		Self-running air cooling when matching amplifier overload coefficient to motor overload coefficient100%				*/
			/*		to lower the level of amplifier overload than motor overload.						*/
			/*		ChkOLP1is a motor,Pn586Since it will be overwritten by the calculation, the						*/
			/*		Here,ChkOLP1to100%											*/
			/*----------------------------------------------------------------------------------*/
//			ChkOLP1->conf.CoolRateGain = LinearCoolRate100PerGain;	/* <S154> */
			ChkOLP1->CoolRateGain = LinearCoolRate100PerGain;		/* <S154> */

			return( TRUE );								/* Do not set parameter setting error			*/
		}
		else
		{
			return( FALSE );							/* Make parameter setting error				*/
		}
	}
	else
	{
		Ix = OlPrm->Ibse;
	}
	Ibsqr = (Ix * Ix) >> 16;							/* Ibsqr: Base current^2					*/

	/*------------------------------------------------------------------------------------------*/
	/* 	Checking intermediate OL current:	OlPrm->Imid													*/
	/*------------------------------------------------------------------------------------------*/
	if( OlPrm->Imid > 0x7FFF )
	{
		Ix = 0x7FFF;
	}
	else
	{
		Ix = OlPrm->Imid;
	}
	Ixsqr = (Ix * Ix) >> 16;							/* Ixsqr: Intermediate OL current^2				*/

	/*------------------------------------------------------------------------------------------*/
	/* 	Continuous OL base current^2, Calculation of alarm detection level, warning level, level gain			*/
	/*------------------------------------------------------------------------------------------*/
	/*	Izsqr		: Numerator term of continuous OL detection alarm level ( = Intermediate OL current^2 - Base current^2)		*/
	/*	Ibsqrmid	: Continuous OL base current^2														*/
	/*	AlmLvlmid	: Continuous OL alarm detection level												*/
	/*	WrnLvlmid	: Continuous OL warning level													*/
	/*	OlLvlmidGain: Continuous OL detection level gain													*/
	/*------------------------------------------------------------------------------------------*/
	Izsqr = Ixsqr - Ibsqr;
#if 1	/* <S154> */
	ChkOLP1->Ibsqrmid  = Ibsqr;
#if (FLOAT_USE==TRUE)
	ChkOLP1->AlmLvlmid = ( (float)Izsqr * (float)(100*OlPrm->Tmid) / (float)KPI_SCCYCLEMS );
	ChkOLP1->WrnLvlmid = ( (float)ChkOLP1->AlmLvlmid * (float)prm_ptr->olwarlvl / (float)100 );
	ChkOLP1->OlLvlmidGain = (float)ChkOLP3->AlmLvlmid / (float)ChkOLP1->AlmLvlmid;
#else
	ChkOLP1->AlmLvlmid = MlibScalKxgain( Izsqr, (100*OlPrm->Tmid), KPI_SCCYCLEMS, NULL, -30 );
	ChkOLP1->WrnLvlmid = MlibScalKxgain( ChkOLP1->AlmLvlmid, prm_ptr->olwarlvl, 100, NULL, -30 );
	ChkOLP1->OlLvlmidGain = MlibScalKxgain( ChkOLP3->AlmLvlmid, 1, ChkOLP1->AlmLvlmid, NULL, 24 );
#endif /* FLOAT_USE */
#else	/* <S154> */
	ChkOLP1->conf.Ibsqrmid  = Ibsqr;
#if (FLOAT_USE==TRUE)
	ChkOLP1->conf.AlmLvlmid = ( (float)Izsqr * (float)(100*OlPrm->Tmid) / (float)KPI_SCCYCLEMS );
	ChkOLP1->conf.WrnLvlmid = ( (float)ChkOLP1->conf.AlmLvlmid * (float)prm_ptr->olwarlvl / (float)100 );
	ChkOLP1->conf.OlLvlmidGain = (float)ChkOLP3->conf.AlmLvlmid / (float)ChkOLP1->conf.AlmLvlmid;
#else
	ChkOLP1->conf.AlmLvlmid = MlibScalKxgain( Izsqr, (100*OlPrm->Tmid), KPI_SCCYCLEMS, NULL, -30 );
	ChkOLP1->conf.WrnLvlmid = MlibScalKxgain( ChkOLP1->conf.AlmLvlmid, prm_ptr->olwarlvl, 100, NULL, -30 );
	ChkOLP1->conf.OlLvlmidGain = MlibScalKxgain( ChkOLP3->conf.AlmLvlmid, 1, ChkOLP1->conf.AlmLvlmid, NULL, 24 );
#endif /* FLOAT_USE */
#endif	/* <S154> */

//	if( (ChkOLP1->conf.AlmLvlmid >= 0x40000000) || (ChkOLP1->conf.AlmLvlmid == 0) )		/* <S154> */
	if( (ChkOLP1->AlmLvlmid >= 0x40000000) || (ChkOLP1->AlmLvlmid == 0) )		/* <S154> */
	{
		rc = FALSE;
	}

	/********************************************************************************************/
	/*		Calculation of instantaneous overload level																*/
	/********************************************************************************************/
	/*																							*/
	/*				 (Imid^2 * Tmid) - (Imax^2 * Tmax)											*/
	/*		Ibsqr = -----------------------------------											*/
	/*						 (Tmid - Tmax)														*/
	/*																							*/
	/*				  (Imax^2 - Imsqr) * Tmax													*/
	/*		AlmLvl = ----------------------------												*/
	/*						   Cycle															*/
	/*																							*/
	/********************************************************************************************/
	/*------------------------------------------------------------------------------------------*/
	/* 	Maximum OL current check:	 OlPrm->Imax												*/
	/*------------------------------------------------------------------------------------------*/
	if( OlPrm->Imax > 0x7FFF)						/* Match the following value to the setting value of one side.			*/
	{
		if( cflg == TRUE )
		{
#if 1	/* <S154> */
			/* Instantaneous OL base current */
			ChkOLP1->Ibsqrmax = ChkOLP2->Ibsqrmax;
			/* Instantaneous OL detection level */
			ChkOLP1->AlmLvlmax = ChkOLP2->AlmLvlmax;
			/* Instantaneous OL warning level */
			ChkOLP1->WrnLvlmax = ChkOLP2->WrnLvlmax;
			/* Instantaneous OL detection level gain */
			ChkOLP1->OlLvlmaxGain = ChkOLP2->OlLvlmaxGain;
#else	/* <S154> */
			/* Instantaneous OL base current */
			ChkOLP1->conf.Ibsqrmax = ChkOLP2->conf.Ibsqrmax;
			/* Instantaneous OL detection level */
			ChkOLP1->conf.AlmLvlmax = ChkOLP2->conf.AlmLvlmax;
			/* Instantaneous OL warning level */
			ChkOLP1->conf.WrnLvlmax = ChkOLP2->conf.WrnLvlmax;
			/* Instantaneous OL detection level gain */
			ChkOLP1->conf.OlLvlmaxGain = ChkOLP2->conf.OlLvlmaxGain;
#endif	/* <S154> */

			/*----------------------------------------------------------------------------------*/
			/*	Motor free-running air cooling coefficient setting															*/
			/*		Self-running air cooling when matching amplifier overload coefficient to motor overload coefficient100%				*/
			/*		to lower the level of amplifier overload than motor overload.						*/
			/*		ChkOLP1is a motor,Pn586Since it will be overwritten by the calculation, the						*/
			/*		Here,ChkOLP1to100%											*/
			/*----------------------------------------------------------------------------------*/
//			ChkOLP1->conf.CoolRateGain = LinearCoolRate100PerGain;	/* <S154> */
			ChkOLP1->CoolRateGain = LinearCoolRate100PerGain;		/* <S154> */

			return( TRUE );								/* Do not set parameter setting error			*/
		}
		else
		{
			return( FALSE );							/* Make parameter setting error				*/
		}
	}
	else
	{
		Ix = OlPrm->Imax;
	}
	Iysqr = (Ix * Ix) >> 16;						/* Iysqr: Maximum OL current^2					*/

	/*------------------------------------------------------------------------------------------*/
	/* 	Instantaneous OL base current^2:	 Ibsqr													*/
	/*------------------------------------------------------------------------------------------*/
	wk = ( Ixsqr * OlPrm->Tmid ) - ( Iysqr * OlPrm->Tmax );		/* Instantaneous OL base current^2numerator term of */
//	Ibsqr = MlibScalKxgain( wk, 1, (OlPrm->Tmid - OlPrm->Tmax), NULL, -30 );
	Ibsqr = (LONG)( (float)wk / (float)(OlPrm->Tmid - OlPrm->Tmax) );

	/*------------------------------------------------------------------------------------------*/
	/* 	Instantaneous OL base current^2Check the numerator term of 													*/
	/*------------------------------------------------------------------------------------------*/
	if( wk < 0 )
	{
#if 1	/* <S154> */
		ChkOLP1->Ibsqrmax		= 0;			/* Instantaneous OL base current^2	= 0					*/
		ChkOLP1->AlmLvlmax		= 0;			/* Instantaneous OL level = 0						*/
		ChkOLP1->WrnLvlmax		= 0;			/* Instantaneous OL warning level = 0				*/
		ChkOLP1->OlLvlmaxGain	= 0;			/* Instantaneous OL warning level = 0				*/
#else	/* <S154> */
		ChkOLP1->conf.Ibsqrmax		= 0;			/* Instantaneous OL base current^2	= 0					*/
		ChkOLP1->conf.AlmLvlmax		= 0;			/* Instantaneous OL level = 0						*/
		ChkOLP1->conf.WrnLvlmax		= 0;			/* Instantaneous OL warning level = 0				*/
		ChkOLP1->conf.OlLvlmaxGain	= 0;			/* Instantaneous OL warning level = 0				*/
#endif	/* <S154> */
		return( FALSE );							/* Make parameter setting error					*/
	}
	else
	{
		/*--------------------------------------------------------------------------------------*/
		/* 	Instantaneous OL base current^2														*/
		/*--------------------------------------------------------------------------------------*/
		if( Ibsqr > 0x7fff)							/* This condition does not occur, but it is included for the time being	*/
		{
			if(cflg == TRUE)
			{
//				Ibsqr = ChkOLP2->conf.Ibsqrmax;		/* InstantaneousOLAdjust base current to one side setting value	*/	/* <S154> */
				Ibsqr = ChkOLP2->Ibsqrmax;		/* InstantaneousOLAdjust base current to one side setting value	*/		/* <S154> */
			}
			else
			{
				return( FALSE );					/* Make parameter setting error					*/
			}
		}

		/*--------------------------------------------------------------------------------------*/
		/* Instantaneous OL base current^2Continuous OL base current^2 						*/
		/*--------------------------------------------------------------------------------------*/
//		if( Ibsqr < ChkOLP1->conf.Ibsqrmid )		/* Match the following value to the continuous OL setting value.	*/	/* <S154> */
		if( Ibsqr < ChkOLP1->Ibsqrmid )				/* Match the following value to the continuous OL setting value.	*/	/* <S154> */
		{
#if 1 /* <S154> */
			/* Instantaneous OL base current^2 */
			ChkOLP1->Ibsqrmax  = ChkOLP1->Ibsqrmid;
			/* Instantaneous OL level */
			ChkOLP1->AlmLvlmax = ChkOLP1->AlmLvlmid;
			/* Instantaneous OL warning level */
			ChkOLP1->WrnLvlmax = ChkOLP1->WrnLvlmid;
			 /* Instantaneous OL level gain */
#if (FLOAT_USE==TRUE)
			ChkOLP1->OlLvlmaxGain =
					(float)ChkOLP3->AlmLvlmax / (float)ChkOLP1->AlmLvlmax;
#else
			ChkOLP1->OlLvlmaxGain =
					MlibScalKxgain( ChkOLP3->AlmLvlmax, 1, ChkOLP1->AlmLvlmax, NULL, 24 );
#endif /* FLOAT_USE */
#else /* <S154> */
			/* Instantaneous OL base current^2 */
			ChkOLP1->conf.Ibsqrmax  = ChkOLP1->conf.Ibsqrmid;
			/* Instantaneous OL level */
			ChkOLP1->conf.AlmLvlmax = ChkOLP1->conf.AlmLvlmid;
			/* Instantaneous OL warning level */
			ChkOLP1->conf.WrnLvlmax = ChkOLP1->conf.WrnLvlmid;
			 /* Instantaneous OL level gain */
#if (FLOAT_USE==TRUE)
			ChkOLP1->conf.OlLvlmaxGain =
					(float)ChkOLP3->conf.AlmLvlmax / (float)ChkOLP1->conf.AlmLvlmax;
#else
			ChkOLP1->conf.OlLvlmaxGain =
					MlibScalKxgain( ChkOLP3->conf.AlmLvlmax, 1, ChkOLP1->conf.AlmLvlmax, NULL, 24 );
#endif /* FLOAT_USE */
#endif /* <S154> */
		}
		/*--------------------------------------------------------------------------------------*/
		/* 	Instantaneous OL base current^2Calculating instantaneous OL level, warning level, level gain	*/
		/*--------------------------------------------------------------------------------------*/
		/*	Izsqr		: Molecular term of instantaneous OL detection alarm level(Maximum OL current^2 - Instantaneous base current^2)	*/
		/*	Ibsqrmax	: Instantaneous OL base current^2													*/
		/*	AlmLvlmax	: Instantaneous OL alarm detection level											*/
		/*	WrnLvlmax	: Instantaneous OL warning level												*/
		/*	OlLvlmaxGain: Instantaneous OL detection level gain												*/
		/*--------------------------------------------------------------------------------------*/
		else
		{
			Izsqr = Iysqr - Ibsqr;
#if 1 /* <S154> */
			ChkOLP1->Ibsqrmax  = Ibsqr;
#if (FLOAT_USE==TRUE)
			ChkOLP1->AlmLvlmax = ( (float)Izsqr * (float)(100*OlPrm->Tmax) / (float)KPI_SCCYCLEMS );
			ChkOLP1->WrnLvlmax = ( (float)ChkOLP1->AlmLvlmax * (float)prm_ptr->olwarlvl / (float)100 );
			ChkOLP1->OlLvlmaxGain = (float)ChkOLP3->AlmLvlmax / (float)ChkOLP1->AlmLvlmax;
#else
			ChkOLP1->AlmLvlmax = MlibScalKxgain( Izsqr, (100*OlPrm->Tmax), KPI_SCCYCLEMS, NULL, -30 );
			ChkOLP1->WrnLvlmax = MlibScalKxgain( ChkOLP1->AlmLvlmax, prm_ptr->olwarlvl, 100, NULL, -30 );
			ChkOLP1->OlLvlmaxGain = MlibScalKxgain( ChkOLP3->AlmLvlmax, 1, ChkOLP1->AlmLvlmax, NULL, 24 );
#endif /* FLOAT_USE */
#else /* <S154> */
			ChkOLP1->conf.Ibsqrmax  = Ibsqr;
#if (FLOAT_USE==TRUE)
			ChkOLP1->conf.AlmLvlmax = ( (float)Izsqr * (float)(100*OlPrm->Tmax) / (float)KPI_SCCYCLEMS );
			ChkOLP1->conf.WrnLvlmax = ( (float)ChkOLP1->conf.AlmLvlmax * (float)prm_ptr->olwarlvl / (float)100 );
			ChkOLP1->conf.OlLvlmaxGain = (float)ChkOLP3->conf.AlmLvlmax / (float)ChkOLP1->conf.AlmLvlmax;
#else
			ChkOLP1->conf.AlmLvlmax = MlibScalKxgain( Izsqr, (100*OlPrm->Tmax), KPI_SCCYCLEMS, NULL, -30 );
			ChkOLP1->conf.WrnLvlmax = MlibScalKxgain( ChkOLP1->conf.AlmLvlmax, prm_ptr->olwarlvl, 100, NULL, -30 );
			ChkOLP1->conf.OlLvlmaxGain = MlibScalKxgain( ChkOLP3->conf.AlmLvlmax, 1, ChkOLP1->conf.AlmLvlmax, NULL, 24 );
#endif /* FLOAT_USE */
#endif /* <S154> */

//			if( ChkOLP1->conf.AlmLvlmax >= 0x40000000 )	/* <S154> */
			if( ChkOLP1->AlmLvlmax >= 0x40000000 )		/* <S154> */
			{
				rc = FALSE;
			}
		}
	}
	return( rc );
}


/****************************************************************************************************/
/*																									*/
/*		Installation environment monitor parameter calculation												<S03A>			*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalInstEnvMon( AXIS_HANDLE *Axis )
{
	CMN_PRMDATA			*Cprm;
	Cprm	= Axis->UniPrms->Cprm;

	/*----------------------------------------------------------------------------------------------*/
	/*	PnE0F(LowerByte): Installation environment monitor reference temperature (Setting environment monitor100%)					*/
	/*----------------------------------------------------------------------------------------------*/
	Axis->EnvMonitor->conf.AmpEnvMonOffset = 100 - (CHAR)(Cprm->InstEnvTmp_sw1.b.l);
	Axis->EnvMonitor->conf.MotEnvMonOffset = 100 - (CHAR)(Cprm->InstEnvTmp_sw1.b.l); /* TODO */
	return;

}



/****************************************************************************************************/
/*																									*/
/*		Calculation of amplifier & motor overload level															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		PnF09 : USHORT	irat;			Motor rated current						[0.1Ao-p]				*/
/*		PnF0A : USHORT	imax;			Motor instantaneous maximum current					[0.1Ao-p]				*/
/*		PnF0F : USHORT	bastrq;			Motor overload detection base current			[%]						*/
/*		PnF10 : USHORT	midtrq;			Motor overload detection intermediate current			[%]						*/
/*		PnF11 : USHORT	midolt;			Motor overload detection intermediate time			[10s]					*/
/*		PnF12 : USHORT	mtrq2;			Motor overload detection intermediate current 2			[%]						*/
/*		PnF13 : USHORT	mtolt2;			Motor overload detection intermediate time 2			[s]						*/
/*																									*/
/*		PnE14 :	USHORT	irats;			Amplifier rated current						[0.1Ao-p]				*/
/*		PnE15 : USHORT	imaxs;			Amplifier instantaneous maximum current					[0.1Ao-p]				*/
/*		PnE17 : DBYTE	ibs_md.b.l;		Amplifier overload detection base current			[%]						*/
/*						ibs_md.b.h;		Amplifier overload detection intermediate current			[10%]					*/
/*		PnE18 : DBYTE	imdt_mxt.b.l;	Amplifier overload detection intermediate time			[10s]					*/
/*						imdt_mxt.b.h;	Maximum amplifier overload detection time			[s]						*/
/*																									*/
/*		Pn52C : USHORT	mtbastrq_drt;	Motor overload detection base current derating					*/
/*																									*/
/*		PnE60 : DBYTE	ratbt_mt.b.l;	OL characteristic derating(BaseCurrent)	[%]						*/
/*						ratbt_mt.b.h;	OL characteristic derating(Intermediate current)	[%]						*/
/*		PnE61 : DBYTE	ratmtt_2.b.l;	OL characteristic derating(Intermediate time)	[%]						*/
/*						ratmtt_2.b.h;	OL characteristic derating(Maximum current)	[%]						*/
/*		PnE62 : DBYTE	ratmt2_os.b.l;	OL characteristic derating(Maximum time)	[%]						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		PnE90 : DBYTE	olspdlvl.b.l;	Electrical angular velocity at the inflection point of the amplifier low speed OL low speed section	[0.1Hz]			*/
/*						olspdlvl.b.h;	Electrical angular velocity at the inflection point of the amplifier low speed OL lock section	[0.1Hz]			*/
/*		PnE91 : DBYTE	ollspdprm.b.l;	Amplifier low speed OL detection intermediate current					[%]				*/
/*						ollspdprm.b.h;	Amplifier low speed OL detection maximum time					[0.1s]			*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalOverLoadLevelAmpMot( AXIS_HANDLE *Axis )
{
	OLPRM		Mot, Amp;
	BOOL		prmchk;
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;
	CHECK_OL	*pMotOL;
	CHECK_OL	*pAmpOL;
	KSGAIN		ks, ks2;
#if (FLOAT_USE==FALSE)
	LONG		kx, sx;
#endif /* FLOAT_USE */

	KSGAIN		LinearCoolRate100PerGain;

/* <S154> Start */
#if CSW_SVCF_CHKAMPLOL == 1
	CHECK_LOWOL *pLowOL;
	LONG	wk;
	LONG	s;
	LONG	calcwk;
	OLPRM	AmpLock,AmpLow;
	CHECK_OLP	*AmpLockOLP;
	LOWOLPRM	*LowOLPrm;
	LOWOLCHKP	*LowOLChkP;
	pLowOL    = &(Axis->CheckAlarm->AmpLowOL);
	AmpLockOLP = &(Axis->CheckAlarm->AmpOL.conf.LockArea);
	LowOLPrm = &(pLowOL->LowOLPrm);
	LowOLChkP = &(pLowOL->LowOLChkP);
#endif
/* <S154> End */

	Prm       = Axis->UniPrms->Prm;		// Parameter setting data
	Bprm      = Axis->UniPrms->Bprm;		// Basic parameter definition
	pMotOL    = &(Axis->CheckAlarm->MotOL);
	pAmpOL    = &(Axis->CheckAlarm->AmpOL);


	/********************************************************************************************/
	/*		Overload basic parameter calculation ==> [0.1s], [15000/MaxTrq(Imax)]							*/
	/********************************************************************************************/
	/*	Motor side OL related basic parameter calculation	*/
	Mot.Tmid = (Prm->MencP.midolt * Prm->ratmtt_2.b.l);
	Mot.Tmax = (Prm->MencP.mtolt2 * Prm->ratmt2_os.b.l) / 10;
#if (FLOAT_USE==TRUE)
	ks = (float)100 * (float)Bprm->PerMaxTrq / (float)15000;
	ks2 = (float)10000 * (float)Bprm->PerMaxTrq / (float)(15000 * Prm->mtbastrq_drt);
	Mot.Ibse = ( (float)Prm->MencP.bastrq * (float)Prm->ratbt_mt.b.l / ks2 );
	Mot.Imid = ( (float)Prm->MencP.midtrq * (float)Prm->ratbt_mt.b.h / ks );
	Mot.Imax = ( (float)Prm->MencP.mtrq2 * (float)Prm->ratmtt_2.b.h / ks );
#else
	ks = MlibScalKxgain( 100, Bprm->PerMaxTrq, 15000, NULL, -1 );
	ks2 = MlibScalKxgain( 10000, Bprm->PerMaxTrq, 15000 * Prm->mtbastrq_drt, NULL, -1 );
	Mot.Ibse = MlibScalKxkxks( Prm->MencP.bastrq, Prm->ratbt_mt.b.l, ks2, NULL, -30 );
	Mot.Imid = MlibScalKxkxks( Prm->MencP.midtrq, Prm->ratbt_mt.b.h, ks, NULL, -30 );
	Mot.Imax = MlibScalKxkxks( Prm->MencP.mtrq2,  Prm->ratmtt_2.b.h, ks, NULL, -30 );
#endif /* FLOAT_USE */
	/*-------------------------------------------------------------------------------------------*/
	/*	Calculation of basic parameters related to amplifier side OL	*/
	Amp.Tmid = (Prm->imdt_mxt.b.l * Prm->ratmtt_2.b.l);
	Amp.Tmax = (Prm->imdt_mxt.b.h * Prm->ratmt2_os.b.l) / 10;
#if (FLOAT_USE==TRUE)
	ks = (float)100 * (float)(Prm->MencP.irat*Bprm->PerMaxTrq) / (float)15000;
	/* <S1F5> Start */
	if ( pLowOL->conf.BaseCurrentDerateEnable == TRUE )
	{	/* With base current derating when single-phase power input is selected */
		ks2 = (float)10000 * (float)(Prm->MencP.irat*Bprm->PerMaxTrq) /
					(float)( 15000 * Prm->svbastrq_drt );
	}
	else
	{	/* Without base current derating when single-phase power input is selected */
		ks2 = ks ;
	}
//	Amp.Ibse = ( (float)(Prm->ibs_md.b.l * Prm->irats) * Prm->ratbt_mt.b.l / ks );
	Amp.Ibse = ( (float)(Prm->ibs_md.b.l * Prm->irats) * Prm->ratbt_mt.b.l / ks2 );
	/* <S1F5> End */
	Amp.Imid = ( (float)(10*Prm->ibs_md.b.h * Prm->irats) * Prm->ratbt_mt.b.h / ks );
	Amp.Imax = ( (float)(Prm->ratmtt_2.b.h * Prm->imaxs) * Prm->ratmtt_2.b.h / ks );
#else
	ks = MlibScalKxgain( 100, (Prm->MencP.irat*Bprm->PerMaxTrq), 15000, NULL, -1 );
	/* <S1F5> Start */
	if ( pLowOL->conf.BaseCurrentDerateEnable == TRUE )
	{	/* With base current derating when single-phase power input is selected */
		ks2 = MlibScalKxgain( 10000, (Prm->MencP.irat*Bprm->PerMaxTrq), (15000 * Prm->svbastrq_drt), NULL, -1 );
	}
	else
	{	/* Without base current derating when single-phase power input is selected */
		ks2 = ks;
	}
//	Amp.Ibse = MlibScalKxkxks( (Prm->ibs_md.b.l * Prm->irats), Prm->ratbt_mt.b.l, ks, NULL, -30 );
	Amp.Ibse = MlibScalKxkxks( (Prm->ibs_md.b.l * Prm->irats), Prm->ratbt_mt.b.l, ks2, NULL, -30 );
	/* <S1F5> End */
	Amp.Imid = MlibScalKxkxks( (10*Prm->ibs_md.b.h * Prm->irats), Prm->ratbt_mt.b.h, ks, NULL, -30 );
	Amp.Imax = MlibScalKxkxks( (Prm->ratmtt_2.b.h  * Prm->imaxs), Prm->ratmtt_2.b.h, ks, NULL, -30 );
#endif /* FLOAT_USE */

	/********************************************************************************************/
	/*		Overload parameter consistency check														*/
	/********************************************************************************************/
	prmchk = TRUE;								/* prmchk: Parameter check confirmation flag			*/
	/*------------------------------------------------------------------------------------------*/
	/*	Motor parameter consistency check															*/
	/*------------------------------------------------------------------------------------------*/
	if( Mot.Tmid <= Mot.Tmax )					/* Intermediate current OL time <= Maximum current OL time			*/
	{
		ALMSetPramError( Axis->AlmManager, pndef_midolt.RegNumber );
		prmchk = FALSE;
	}
	if( Mot.Imid >= Mot.Imax )					/* Intermediate OL current > Maximum OL current					*/
	{
		ALMSetPramError( Axis->AlmManager, pndef_midtrq.RegNumber );
		prmchk = FALSE;
	}
	if( Mot.Imid <= Mot.Ibse )					/* Intermediate OL current < Base OL current				*/
	{
		ALMSetPramError( Axis->AlmManager, pndef_bastrq.RegNumber );
		prmchk = FALSE;
	}
	/*------------------------------------------------------------------------------------------*/
	/*	Amplifier parameter consistency check															*/
	/*------------------------------------------------------------------------------------------*/
	if( Amp.Tmid <= Amp.Tmax )					/* Intermediate current OL time < Maximum current OL time			*/
	{
		ALMSetPramError( Axis->AlmManager, pndef_imdt_mxt.RegNumber );
		prmchk = FALSE;
	}
	if( Amp.Imid >= Amp.Imax )					/* Intermediate OL current > Maximum OL current					*/
	{
		ALMSetPramError( Axis->AlmManager, pndef_ibs_md.RegNumber );
		prmchk = FALSE;
	}
	if( Amp.Imid <= Amp.Ibse )					/* Intermediate OL current < Base OL current				*/
	{
		ALMSetPramError( Axis->AlmManager, pndef_ibs_md.RegNumber );
		prmchk = FALSE;
	}

	/********************************************************************************************/
	/*	Motor free-running cooling rate parameter calculation															*/
	/*		Calculated with initial setting value as necessary for initial calculation												*/
	/********************************************************************************************/
//	IprmcalCoolRateGainMot( );
	/*------------------------------------------------------------------------------------------*/
	/*	Amplifier free-running cooling rate parameter calculation															*/
	/*------------------------------------------------------------------------------------------*/
//	pAmpOL->conf.CoolRateGain = 0;											/* 0Fixed			*/	/* <S154> */
	pAmpOL->conf.NormalArea.CoolRateGain = 0;								/* 0Fixed			*/	/* <S154> */

#if 0 /* 2009.09.25 Y.Oka Low speedOLNone */
	if(LowOlChkP.LowSpdOLChkEnable == TRUE)		/* Low-speed OL detection execution flag: Detectable?		*/
	{
	    LowOlChkP.AmpLockOLP.CoolRateGain = ChkAlmP.AmpOLP.CoolRateGain;
	}
#endif

	/*----------------------------------------------------------------------------------*/
	/*	Motor free-running cooling rate 100%Calculation														*/
	/*----------------------------------------------------------------------------------*/
	if( Axis->MencV->AxisMotType == MOTTYPE_LINEAR )
	{
#if (FLOAT_USE==TRUE)
		LinearCoolRate100PerGain = 16384.0f / (float)Bprm->NorMaxSpd;
#else
		kx = MlibScalKxgain( 16384, 100, 100, &sx, 0 );
		LinearCoolRate100PerGain = MlibPcalKxgain( kx, 1, Bprm->NorMaxSpd, &sx, 24 );
#endif /* FLOAT_USE */
	}
	else
	{
		LinearCoolRate100PerGain = 0;
	}

	/********************************************************************************************/
	/*		Overload check parameter calculation														*/
	/********************************************************************************************/
	if( prmchk == TRUE )						/* Determine whether parameter setting error exists					*/
	{
		/*--------------------------------------------------------------------------------------*/
		/*	Parameter calculation for each overload check													*/
		/*--------------------------------------------------------------------------------------*/
		if( Amp.Imax > Mot.Imax )
		{
			/*----------------------------------------------------------------------------------*/
			/*	Parameter calculation for motor overload check											*/
			/*----------------------------------------------------------------------------------*/
//			if( LpxPcalOverLoadLevelAmpMot( &Mot, pMotOL, pMotOL, pMotOL,
//			                                Prm, LinearCoolRate100PerGain ) != TRUE )		/* <S154> */
			if( LpxPcalOverLoadLevelAmpMot( &Mot, &(pMotOL->conf.NormalArea), &(pMotOL->conf.NormalArea),&(pMotOL->conf.NormalArea),
			                                Prm, LinearCoolRate100PerGain ) != TRUE )		/* <S154> */
			{
				/* A.040 : Parameter setting error */
				ALMSetPramError( Axis->AlmManager, pndef_midtrq.RegNumber );
			}
			/*----------------------------------------------------------------------------------*/
			/*	Parameter calculation for amplifier overload check											*/
			/*----------------------------------------------------------------------------------*/
//			if( LpxPcalOverLoadLevelAmpMot( &Amp, pAmpOL, pMotOL, pAmpOL,
//			                                Prm, LinearCoolRate100PerGain ) != TRUE )		/* <S154> */
			if( LpxPcalOverLoadLevelAmpMot( &Amp, &(pAmpOL->conf.NormalArea), &(pMotOL->conf.NormalArea), &(pAmpOL->conf.NormalArea),
			                                Prm, LinearCoolRate100PerGain ) != TRUE )		/* <S154> */

			{
				/* A.040 : Parameter setting error */
				ALMSetPramError( Axis->AlmManager, pndef_ibs_md.RegNumber );
			}
		}
		else
		{
			/*----------------------------------------------------------------------------------*/
			/*	Parameter calculation for amplifier overload check											*/
			/*----------------------------------------------------------------------------------*/
//			if( LpxPcalOverLoadLevelAmpMot( &Amp, pAmpOL, pAmpOL, pAmpOL,
//			                                Prm, LinearCoolRate100PerGain ) != TRUE )		/* <S154> */
			if( LpxPcalOverLoadLevelAmpMot( &Amp, &(pAmpOL->conf.NormalArea), &(pAmpOL->conf.NormalArea), &(pAmpOL->conf.NormalArea),
			                                Prm, LinearCoolRate100PerGain ) != TRUE )		/* <S154> */
			{
				/* A.040 : Parameter setting error */
				ALMSetPramError( Axis->AlmManager, pndef_ibs_md.RegNumber );
			}
			/*----------------------------------------------------------------------------------*/
			/*	Parameter calculation for motor overload check											*/
			/*----------------------------------------------------------------------------------*/
//			if( LpxPcalOverLoadLevelAmpMot( &Mot, pMotOL, pAmpOL, pMotOL,
//			                                Prm, LinearCoolRate100PerGain ) != TRUE )		/* <S154> */
			if( LpxPcalOverLoadLevelAmpMot( &Mot, &(pMotOL->conf.NormalArea), &(pAmpOL->conf.NormalArea), &(pMotOL->conf.NormalArea),
			                                Prm, LinearCoolRate100PerGain ) != TRUE )		/* <S154> */
			{
				/* A.040 : Parameter setting error */
				ALMSetPramError( Axis->AlmManager, pndef_midtrq.RegNumber );
			}
		}
/* <S154> Start */
#if CSW_SVCF_CHKAMPLOL == 1 			/* CSW_SVCF_CHKAMPLOL: Model definition (low-speed OL function depending on model)	*/
		/****************************************************************************************/
		/*		Amplifier low speed overload execution condition determination													*/
		/****************************************************************************************/
		/*		1.( (PnE90.L==0)&&(PnE90.H==0) ):PnE90is both0								*/
		/*			→Low-speed OL execution is not possible because there is no inflection point speed for each section							*/
		/*		2.( PnE90.L <= PnE90.H ):Lock section inflection point speed≦Low speed section inflection point speed				*/
		/*			→Low-speed OL execution NG because the magnitude relationship of inflection point speed is reversed							*/
		/*		3.( Amp.Ibse >= 0x4000 ): Continuous OL base current is the maximum current(15000≒16384)or more		*/
		/*			→Low-speed OL execution NG because the OL curve is in the range above the normalized current				*/
		/*			(Calculation results in a meaningless OL curve)									*/
		/*	⇒If low speed OL execution is NG, normal overload detection is performed. Low speed OL parameters are not calculated.	*/
		/*--------------------------------------------------------------------------------------*/
		if(	((Prm->olspdlvl.b.l!= (UCHAR)0)||(Prm->olspdlvl.b.h!= (UCHAR)0)) && ( Prm->olspdlvl.b.l > Prm->olspdlvl.b.h )
			&& ( Amp.Ibse < 0x4000 ) )
		{
			/************************************************************************************/
			/*		Amplifier low speed overload  Detection execution flag processing										*/
			/************************************************************************************/
			/*	Low-speed OL detection execution flagTRUE: Detectable										*/
			/*		LowSpdOLChkEnable.					*/
			/*----------------------------------------------------------------------------------*/
			LowOLChkP->conf.LowSpdOLChkEnable = (UCHAR)TRUE;			/* Low-speed OL detection execution flag: Detectable		*/

			/************************************************************************************/
			/*		Amplifier low speed overload  Lock section parameter calculation									*/
			/************************************************************************************/
			/*----------------------------------------------------------------------------------*/
			/*	Amplifier low-speed OL: Lock section side basic parameter calculation									*/
			/*----------------------------------------------------------------------------------*/
			/*	Tmid:Lock section intermediate OL time…Same as normal section									*/
			/*	Imid:Lock section intermediate OL current…Calculated using the following formula									*/
			/*			Formula:(PnE91.L*PnE14*PnE60.H*15000)/(PnF09*100*Bprm.PerMaxTrq)			*/
			/*	Tmax: Lock section maximum OL time…Calculated using the following formula									*/
			/*			Formula:(PnE91.H*PnE62.L)/100												*/
			/*	Imax: Lock section maximum OL current…Same as normal section									*/
			/*	Ibse: Lock section continuous OL base current…Same as normal section								*/
			/*----------------------------------------------------------------------------------*/
			AmpLock.Tmid = Amp.Tmid;
			ks = MlibScalKxgain( 100, (LONG)(Prm->MencP.irat*Bprm->PerMaxTrq), 15000, NULL, -1 );
			AmpLock.Imid = MlibScalKxkxks( (LONG)(Prm->ollspdprm.b.l * Prm->irats), (LONG)Prm->ratbt_mt.b.h, ks, NULL, -30 );
			AmpLock.Tmax = MlibScalKxgain( (LONG)Prm->ollspdprm.b.h, (LONG)Prm->ratmt2_os.b.l, 100, NULL, -30 );
			AmpLock.Imax = Amp.Imax;
			AmpLock.Ibse = Amp.Ibse;
			LowOLPrm->AmpLock = AmpLock;

			/*----------------------------------------------------------------------------------*/
			/*	Amplifier low-speed OL: Parameter calculation for lock section check								*/
			/*----------------------------------------------------------------------------------*/
			if( LpxPcalOverLoadLevelAmpMot( &AmpLock, AmpLockOLP , &(pMotOL->conf.NormalArea),
				&(pAmpOL->conf.NormalArea) , Prm , LinearCoolRate100PerGain) != TRUE )
			{
				ALMSetPramError( Axis->AlmManager, pndef_midtrq.RegNumber );		/* A.040 : Parameter setting error			*/
			}

			/*----------------------------------------------------------------------------------*/
			/*	Amplifier low-speed OL: Lock section processing (unification of OL detection level for normal section and lock section)	*/
			/*----------------------------------------------------------------------------------*/
			AmpLockOLP->AlmLvlmid = pAmpOL->conf.NormalArea.AlmLvlmid;/* Continuous OL level			*/
			AmpLockOLP->AlmLvlmax = pAmpOL->conf.NormalArea.AlmLvlmax;/* Instantaneous OL level			*/
			AmpLockOLP->WrnLvlmid = pAmpOL->conf.NormalArea.WrnLvlmid;/* Continuous OL warning level		*/
			AmpLockOLP->WrnLvlmax = pAmpOL->conf.NormalArea.WrnLvlmax;/* Instant OL warning level		*/

			/************************************************************************************/
			/*		Amplifier low speed overload  Low speed section parameter calculation									*/
			/************************************************************************************/
			/*----------------------------------------------------------------------------------*/
			/*	Amplifier low-speed OL: Low-speed section side basic parameter calculation									*/
			/*----------------------------------------------------------------------------------*/
			/*	Tmid: Low-speed section intermediate OL time…Same as normal section										*/
			/*	Imid: Low-speed section intermediate OL current…Calculated online											*/
			/*	Tmax: Low-speed section maximum OL time…Calculated online											*/
			/*	Imax: Low-speed section maximum OL current…Same as normal section										*/
			/*	Ibse: Low-speed section continuous OL base current…Same as normal section								*/
			/*----------------------------------------------------------------------------------*/
			AmpLow.Tmid = Amp.Tmid;
			AmpLow.Imax = Amp.Imax;
			AmpLow.Ibse = Amp.Ibse;
			LowOLPrm->AmpLow = AmpLow;

			/*----------------------------------------------------------------------------------*/
			/*	Amplifier low-speed OL: Coefficient calculation for low-speed interval interpolation calculation 1									*/
			/*----------------------------------------------------------------------------------*/
			/*	calcwk			: Inflection point speed conversion gain											*/
			/* 	LowSpdOLchg		: Low-speed section inflection point speed(PnE90.L)	[0.1Hz]→[2^24/OvrSpd]			*/
			/*	LockSpdOLchg	:Lock section inflection point speed(PnE90.H) [0.1Hz]→[2^24/OvrSpd]			*/
			/*	LowSpdOffsetImid:Lock section inflection point intermediate OL current	[0.1sec]						*/
			/*	LowSpdOffsetTmax:Lock section inflection point maximum OL time	[15000/MaxTrq]					*/
			/*----------------------------------------------------------------------------------*/
			calcwk = MlibScalKxgain( 1, 1, 10, &s, 0 );
			calcwk = MlibPcalKxkxks( calcwk, 1, Bprm->Keangle, &s, 0 );
			calcwk = MlibPcalKxgain( calcwk, C2PAIE7, 10000000, &s, 0 );
			calcwk = MlibPcalKxkxks( calcwk, 0x1000000, Bprm->OvrSpd, &s, 24 );
			LowOLChkP->conf.LowSpdOLchg = MlibMulgain((LONG)Prm->olspdlvl.b.l, calcwk);
			LowOLChkP->conf.LockSpdOLchg = MlibMulgain((LONG)Prm->olspdlvl.b.h, calcwk);
			LowOLChkP->conf.LowSpdOffsetImid = AmpLock.Imid;
			LowOLChkP->conf.LowSpdOffsetTmax = AmpLock.Tmax;

			/*----------------------------------------------------------------------------------*/
			/*	Amplifier low-speed OL: Coefficient calculation for low-speed interval interpolation calculation 2									*/
			/*----------------------------------------------------------------------------------*/
			/*	TransOLImid:[Electrical angular velocity]→[Intermediate OL current]Interpolation coefficient								*/
			/*					(Middle of normal sectionOLCurrent―Lock section middleOLCurrent)						*/
			/*			=	----------------------------------------------------				*/
			/*					(Low speed section inflection point speed―Lock section inflection point speed)						*/
			/*																					*/
			/*	TransOLTmax:[Electrical angular velocity]→[OL detection time]Interpolation coefficient								*/
			/*						(Instantaneous intermediateOLTime―Low speed section maximumOLTime)						*/
			/*			=	---------------------------------------------------					*/
			/*					√(Low speed section inflection point speed―Lock section inflection point speed)					*/
			/*		.Instantaneous intermediateOLTime(PnE18.H[sec])			:	imdt_mxt.b.h					*/
			/*		.Low speed section maximumOLTime(PnE91.H[0.1sec])	:	ollspdprm.b.h					*/
			/*----------------------------------------------------------------------------------*/
			wk = LowOLChkP->conf.LowSpdOLchg - LowOLChkP->conf.LockSpdOLchg;
			LowOLChkP->conf.TransOLImid = MlibScalKxgain( (Amp.Imid - AmpLock.Imid), 1, wk, NULL, 24 );
			wk = (LONG)MlibSqrtu32( (ULONG)LowOLChkP->conf.LowSpdOLchg - (ULONG)LowOLChkP->conf.LockSpdOLchg );
			LowOLChkP->conf.TransOLTmax = MlibScalKxgain( (((LONG)Prm->imdt_mxt.b.h*10) - (LONG)Prm->ollspdprm.b.h), 1, wk, NULL, 24 );
		}
		else
		{
			/************************************************************************************/
			/*		Amplifier low speed overload  Detection execution flag processing										*/
			/************************************************************************************/
			/*	Low-speed OL detection execution flagFALSE: Undetectable									*/
			/*		LowSpdOLChkEnable.					*/
			/*----------------------------------------------------------------------------------*/
			LowOLChkP->conf.LowSpdOLChkEnable = (UCHAR)FALSE;		/* Low-speed OL detection execution flag: Not detectable		*/
		}
#endif
/* <S154> End */
	}
}


/****************************************************************************************************/
/*																									*/
/*		Motor runaway monitoring parameter calculation(Rotary type motor)													*//* <V051> */
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Calculate runaway monitoring torque level and speed level											*/
/*																									*/
/*			Runaway monitoring torque		:PnE44.h															*/
/*			Runaway monitoring speed level	: Overspeed20%												*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalMotorOverrun( AXIS_HANDLE *Axis )
{
	ULONG			work1;
	PRMDATA			*Prm;
	BPRMDAT			*Bprm;
	CHECK_OVRRUN	*pOvrRun;

	Prm = Axis->UniPrms->Prm;		// Parameter setting data
	Bprm = Axis->UniPrms->Bprm;		// Basic parameter definition
	pOvrRun = &(Axis->CheckAlarm->OvrRun);
	work1 = Prm->ipg_rwt.b.h;

	/* Runaway monitoring torque command level */
	if ( work1 > (ULONG)Bprm->PerMaxTrq )
	{
		pOvrRun->OvrTrqLevel = 0x01000000;
	}
	else
	{
#if (FLOAT_USE==TRUE)
		pOvrRun->OvrTrqLevel = work1 * 0x01000000 / Bprm->PerMaxTrq;
#else
		pOvrRun->OvrTrqLevel = MlibScalKxgain( work1, 0x01000000, Bprm->PerMaxTrq, NULL, -24);
#endif /* FLOAT_USE */

	}

	/* Runaway monitoring motor speed level */
	pOvrRun->OvrSpdLevel = 0x01000000 / 5;

}


/****************************************************************************************************/
/*																									*/
/*		Calculating the maximum allowable input signal timing time														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Pn620 : USHORT	bbondiff0;			STOPSignal timing maximum allowable time (Pn620)[10ms]			*/
/*		PnE4C : USHORT	bbondiff1;			Internal signal timing maximum allowable time (PnE4C)[10ms]			*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalMaxBbonTimeDiff( AXIS_HANDLE *Axis )
{
	CHECK_HWBB_TIMING *HwbbTiming;
	PRMDATA		*Prm;

	Prm = Axis->UniPrms->Prm;		// Parameter setting data
	HwbbTiming = &(Axis->CheckAlarm->HwbbTiming);
	HwbbTiming->conf.MaxBbonTimeDiff = 10 * Prm->bbondiff0;
}


/****************************************************************************************************/
/*																									*/
/*		Position command overspeed(OS)																	*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalPcmdOvrSpd( AXIS_HANDLE *Axis )
{
#if 0	// Currently not supported
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;
	IPRMDAT		*Iprm;
#if (FLOAT_USE==TRUE)
	float		fw;
#else
	LONG		kx, sx;
#endif /* FLOAT_USE */

	Prm = Axis->UniPrms->Prm;		// Parameter setting data
	Bprm = Axis->UniPrms->Bprm;		// Basic parameter definition
	Iprm = Axis->UniPrms->iprm_ptr;		// Basic parameter definition

#if (FLOAT_USE==TRUE)
	fw = Bprm->OvrSpd / Bprm->Kfbpls;
	fw = fw * (float)Prm->rata2l / (float)Prm->ratb2l;
	Iprm->PcmdOvrSpd = fw * (float)Iprm->SvCycleNs / 1000000000.0F;
#else

	kx = MlibScalKxksks( 1,  Bprm->OvrSpd,    Bprm->Kfbpls,  &sx,  0 );
	kx = MlibPcalKxgain( kx, Prm->rata2l,     Prm->ratb2l,   &sx,  0 );
	kx = MlibPcalKxgain( kx, Iprm->SvCycleNs, 1000000000,    &sx, 24 );
	Iprm->PcmdOvrSpd = kx;
#endif /* FLOAT_USE */
#endif
}


/****************************************************************************************************/
/*																									*/
/*		Torque level calculation	<S00C>																	*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalTorqueLevel( AXIS_HANDLE *Axis )
{
	BPRMDAT		*Bprm;

	Bprm = Axis->UniPrms->Bprm;		// Basic parameter definition

	Bprm->TrefonLevel = MlibScalKxgain( 0x01000000, 10, Bprm->PerMaxTrq, NULL, -24 );
}

/****************************************************************************************************/
/*																									*/
/*		Overtravel mask 	Temporary! ! !<S032>																*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalOtMask( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
	Prm = Axis->UniPrms->Prm;

	Axis->IoSignals->pot_mask = (( Prm->insel1 & 0xF000 ) == 0x8000)? 0 : 1;
	Axis->IoSignals->not_mask = (( Prm->insel2 & 0x000F ) == 0x0008)? 0 : 1;

}

/****************************************************************************************************/
/*																									*/
/*		Power board related settings																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		PnE0C.Bit0 : Built-in regenerative resistor																	*/
/*		PnE0C.Bit4 : Inrush/DB control method																*/
/*		PnE0C.Bit5 : Inrush/DB control method																*/
/*		PnE0C.Bit6 : Initial value ratio selection for regenerative overload detection													*/
/*		PnE0C.Bit7 : Initial value ratio selection for regenerative overload detection													*/
/*		PnE0C.Bit8 : Main circuit bus voltage detection method															*/
/*		PnE0C.Bit9 : Main circuit bus voltage detection method															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Main circuit detection parameter consistency check																*/
/*			If the following conditions are met, it will be considered abnormal. When an error occurs, turn on the regeneration operation prohibition flag.					*/
/*				For PWM method: Main circuit detection maximum value > PWMOutput width(KPX_VDET_PWM_MAX)					*/
/*				For AD method: Main circuit detection maximum value > ADConverter maximum read value(KPX_DCVOLT_ADMAX)			*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalPowerCircuitSetting( AXIS_HANDLE *Axis )
{
//	PRMDATA				*Prm;
	BPRMDAT				*Bprm;
	CMN_PRMDATA			*Cprm;
	REGENE_CTRL			*RegCtrl;
	DET_DC_VOLT			*DetectDc;
	CHECK_MAIN_POWER	*PowerChecker;

	Bprm 	= Axis->UniPrms->Bprm;		// Basic parameter definition
	Cprm	= Axis->UniPrms->Cprm;
	RegCtrl = &(Axis->PowerManager->RegeneCtrl);
	DetectDc = &(Axis->PowerManager->DetectDcVolt);
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);

	/* Initialize */
	DetectDc->conf.VdetPrmOk = TRUE;

/*--------------------------------------------------------------------------------------------------*/
/*		PnE0C : System switch																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( Cprm->syssw2 & 0x0001 )
	{
		RegCtrl->conf.RegRxIn = TRUE;
	}
	else
	{
		RegCtrl->conf.RegRxIn = FALSE;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		bit4,5 : Inrush/DBControl method																*/
	/*			00	Inrush+DB: Relay(×1)															*/
	/*			01 	Rush: Relay;DB:Thyristor														*/
	/*			10	Rush: Relay;DB: Relay														*/
	/*			11	Inrush+DB: Relay+Thyristor														*/
	/*----------------------------------------------------------------------------------------------*/
	switch( (Cprm->syssw2 >> 4) & 0x03 )
	{
	case 0x00:
		PowerChecker->conf.RlyDbWay = DBCTRL_RLY_COMMON;
		break;
	case 0x01:
		PowerChecker->conf.RlyDbWay = DBCTRL_TRIACK_SEPARATE;
		break;
	case 0x02:
		PowerChecker->conf.RlyDbWay = DBCTRL_RLY_SEPARATE;
		break;
	case 0x03:
		PowerChecker->conf.RlyDbWay = DBCTRL_TRIACK_COMMON;
		break;
	default:
		ALMSetPramError( Axis->AlmManager, pndef_syssw2.RegNumber );
		break;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		bit6,7 : Initial value ratio selection for regenerative overload detection													*/
	/*			00:RgolAlmLevel * 0.95																*/
	/*			01:RgolAlmLevel * 0.50																*/
	/*			10: Reserved (01)																	*/
	/*			11: Reserved (01)																	*/
	/*----------------------------------------------------------------------------------------------*/
	switch( (Cprm->syssw2 >> 6) & 0x03 )
	{
	case 0x00:
		RegCtrl->RegOL.conf.RgSumSel = REGPOWSUM_RATE95PER;
		break;
	case 0x01:
	case 0x02:
	case 0x03:
		RegCtrl->RegOL.conf.RgSumSel = REGPOWSUM_RATE50PER;
		break;
	default:
		ALMSetPramError( Axis->AlmManager, pndef_syssw2.RegNumber );
		break;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		Bit8,9 : Main circuit bus voltage detection method															*/
	/*----------------------------------------------------------------------------------------------*/
	switch((UCHAR)( Cprm->syssw2 >> 8 ) & 0x03 )
	{
		case 0x01:									/* PWM-Comparator method : Fixed value(1)	*/
			DetectDc->conf.VdetMode = VDET_PWM_FIX_FOLLOW;
			break;
		case 0x02:									/* PWM-Comparator method : Variable value(1-3)	*/
			DetectDc->conf.VdetMode = VDET_PWM_VAR_FOLLOW;
//			Iprm->VdetFollowWidthMax = 0x03;		/* Following width maximum addition width								*/
			break;
		case 0x03:									/* A/DConversion method									*/
			DetectDc->conf.VdetMode = VDET_AD;
			break;
		default:									/* No detection by software					*/
			DetectDc->conf.VdetMode = VDET_NONE;
//			Iprm->VdetFollowWidthMax = 0;

			/* When there is no detection, parameter error */
			ALMSetPramError( Axis->AlmManager, pndef_syssw2.RegNumber );
			break;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Relationship check between upper limit value by HW and software detection upper limit value											*/
/*--------------------------------------------------------------------------------------------------*/
	switch( DetectDc->conf.VdetMode )
	{
	/*------------------------------------------------------------------------------------------*/
	/*		For PWM method																	*/
	/*------------------------------------------------------------------------------------------*/
		case VDET_PWM_FIX_FOLLOW:
		case VDET_PWM_VAR_FOLLOW:
			if( Bprm->VdetMax > KPX_VDET_PWM_MAX )
			{
				DetectDc->conf.VdetPrmOk = FALSE;
			}
			break;
	/*------------------------------------------------------------------------------------------*/
	/*		For AD method																		*/
	/*------------------------------------------------------------------------------------------*/
		case VDET_AD:
			if( Bprm->VdetMax > KPX_DCVOLT_ADMAX )
			{
				DetectDc->conf.VdetPrmOk = FALSE;
			}
			break;
	/*------------------------------------------------------------------------------------------*/
	/*		When voltage is not detected by software														*/
	/*------------------------------------------------------------------------------------------*/
		default:
			DetectDc->conf.VdetPrmOk = FALSE;
			break;
	}

	if( DetectDc->conf.VdetPrmOk == FALSE )
	{
		ALMSetPramError( Axis->AlmManager, pndef_syssw2.RegNumber );
	}

}


/****************************************************************************************************/
/*																									*/
/*		Copy parameter(Prm --> Iprm)																*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalCopyParamToIprm( AXIS_HANDLE *Axis )
{
#if 0
	PRMDATA		*Prm;
	IPRMDAT		*Iprm;

	Prm = Axis->UniPrms->Prm;		// Basic parameter definition
	Iprm = Axis->UniPrms->iprm_ptr;	// Basic parameter definition

	Iprm->mtlmt = Prm->mtlmt;		/* Multiturn limit setting						*/
#endif
}



/****************************************************************************************************/
/*																									*/
/*		Position command electronic gear calculation																		*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalElectronicGear( AXIS_HANDLE *Axis )
{
	EGEAR		*Egear;
	EGEAR		*InvalidEgear;
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;
	LONG		Ratb_a, Rata_b;
	LONG		i, sht;

	Prm = Axis->UniPrms->Prm;
	Bprm = Axis->UniPrms->Bprm;
	Egear = &(Axis->BaseControls->PosManager.conf.Egear);
	InvalidEgear = &(Axis->BaseControls->PosManager.conf.InvalidEgear);

/*--------------------------------------------------------------------------------------------------*/
/*		Electronic gear calculation																				*/
/*--------------------------------------------------------------------------------------------------*/
	MlibPcalKegear( Prm->rata2l, Prm->ratb2l, Egear );

/*--------------------------------------------------------------------------------------------------*/
/*		Operation error check																			*/
/*--------------------------------------------------------------------------------------------------*/
	Ratb_a = (Egear->k2 == 0) ? Egear->k1 : Egear->k1 + 1;
	Rata_b = (Egear->g2 == 0) ? Egear->g1 : Egear->g1 + 1;

	if( ( Prm->rata2l > 0x40000000 ) || ( Prm->ratb2l > 0x40000000 ) ||
		( Egear->a == 0 ) || ( Egear->b == 0 ) ||
//		( Ratb_a > 4000 ) || ( Rata_b > 1000 ) )	/* <S17C> */
		( Ratb_a > 64000 ) || ( Rata_b > 1000 ) )	/* <S17C> Change the upper limit value(4000⇒64000) */
	{
		MlibPcalKegear( 1, 1, Egear );
		ALMSetPramError( Axis->AlmManager, pndef_rata2l.RegNumber );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Time correction calculation for position deviation monitor																*/
/*--------------------------------------------------------------------------------------------------*/
	sht = 0;
	for(i = 2; i <= (Bprm->SvCycleNs/CFG_POSERR_DELAY); i*=2)
	{	sht++;}
	Axis->BaseControls->PosManager.conf.PosErrDelayShift = sht;
/*--------------------------------------------------------------------------------------------------*/
/*		Gear ratio calculation for disabling electronic gear function																*/
/*--------------------------------------------------------------------------------------------------*/
	MlibPcalKegear( 1, 1, InvalidEgear );
}


/****************************************************************************************************/
/*																									*/
/*		Alarm mask setting(PnE05/E06)																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Alarm mask parameter(PnE05/PnE06).			*/
/*																									*/
/*																									*/
/*	Supplementary Foot : It seems that the alarm mask cancellation function is not necessary, but for now, let's match it to SGDS.			*/
/*																									*/
/*			1) servo_alarm_mask( ) : Setting/cancelling alarm mask(Set/cancel specified by argument)			*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalAlarmMask( AXIS_HANDLE *Axis )
{
	ULONG		Mskx;
	PRMDATA		*Prm;

	Prm		= Axis->UniPrms->Prm;

/*--------------------------------------------------------------------------------------------------*/
/*		Alarm mask : PnE05																		*/
/*--------------------------------------------------------------------------------------------------*/
	Mskx = Prm->almmsk1;
	/* Encoder connection error, motor already connected/Servo capacity unmatch(A.050)is masked *//* <S19C> */
	if( !ALMCheckSetMask( Axis->AlmManager, ALM_UNMATCH ) )
	{	/* Only when not maskedPnE05 */
		ALMSetMask(Axis->AlmManager, ALM_UNMATCH,	((Mskx & 0x0001)!=0));/* A.050 : Type unmatch			*/
	}	/* <S19C> */
	ALMSetMask(Axis->AlmManager, ALM_RGOL,		((Mskx & 0x0002)!=0));/* A.320 : Regeneration overload 				*/
	ALMSetMask(Axis->AlmManager, ALM_PUV,		((Mskx & 0x0004)!=0));/* A.410 : Undervoltage 					*/
	ALMSetMask(Axis->AlmManager, ALM_OSF1,		((Mskx & 0x0008)!=0));/* A.510 : Overspeed(OS)				*/
	if((Mskx & 0x0010)!=0)
	{	ALMSetMask(Axis->AlmManager, ALM_OLF1,	TRUE);}				  /* A.710 : Overload(Instantaneous maximum load)		*/
	if((Mskx & 0x0020)!=0)
	{	ALMSetMask(Axis->AlmManager, ALM_OLF2,	TRUE);}				  /* A.720 : Overload(Continuous maximum load)		*/
	ALMSetMask(Axis->AlmManager, ALM_DBOL,		((Mskx & 0x0040)!=0));/* A.730 : DB overload 				*/
	ALMSetMask(Axis->AlmManager, ALM_RSOL,		((Mskx & 0x0080)!=0));/* A.740 : Inrush resistance overload 			*/
	ALMSetMask(Axis->AlmManager, ALM_RWF,		((Mskx & 0x0100)!=0));/* A.C10 : Runaway prevention detection				*/
	ALMSetMask(Axis->AlmManager, ALM_OF,		((Mskx & 0x0200)!=0));/* A.D00 : Excessive position deviation 				*/
	ALMSetMask(Axis->AlmManager, ALM_PWPHASE,	((Mskx & 0x0400)!=0));/* A.F10 : Power supply phase loss			 		*/
	ALMSetMask(Axis->AlmManager, ALM_FSOF,		((Mskx & 0x0800)!=0));/* A.D10 : Motor-Excessive deviation between load positions	*/
	ALMSetMask(Axis->AlmManager, ALM_OPSET,		((Mskx & 0x1000)!=0));/* A.E00 : SynchronizationAccRequest receptionTimeOut		*/
	ALMSetMask(Axis->AlmManager, ALM_TIMEOUT,	((Mskx & 0x1000)!=0));/* A.E01 : Reception of operation start requestTimeOut	*/
	ALMSetMask(Axis->AlmManager, ALM_WDC_ERR,	((Mskx & 0x2000)!=0));/* A.E02 : WDC+SyncFalgSynchronization error		*/
	ALMSetMask(Axis->AlmManager, ALM_WIR,		((Mskx & 0x4000)!=0));/* A.330 : Main circuit wiring error			*/
	ALMSetMask(Axis->AlmManager, ALM_VREFADCHK,	((Mskx & 0x8000)!=0));/* A.B11 : Speed command AD data error		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Alarm mask : PnE06																		*/
/*--------------------------------------------------------------------------------------------------*/
	Mskx = Prm->almmsk2;
	ALMSetMask(Axis->AlmManager, ALM_NOTSUPPORT,((Mskx & 0x0001)!=0));/* A.051 : Product not supported				*/
	ALMSetMask(Axis->AlmManager, ALM_VREFADERR, ((Mskx & 0x0002)!=0));/* A.B10 : Abnormal AD data for speed command			*/
	ALMSetMask(Axis->AlmManager, ALM_POSOF,     ((Mskx & 0x0004)!=0));/* A.D30 : Position data overflow	*/
	ALMSetMask(Axis->AlmManager, ALM_FANLOCK,	((Mskx & 0x0008)!=0));/* A.7AB : Stopping the SERVOPACK built-in fan	*/
	ALMSetMask(Axis->AlmManager, ALM_CTRLBRD,	((Mskx & 0x0010)!=0));/* A.7A1 : Internal temperature abnormality1(Control board)	*//* <S0DE> */
	ALMSetMask(Axis->AlmManager, ALM_PWRBRD,	((Mskx & 0x0010)!=0));/* A.7A2 : Internal temperature abnormality2(Power board)	*//* <S0DE> */

/*--------------------------------------------------------------------------------------------------*/
/*		Motorless test mode alarm mask processing	<S1B7>											*/
/*--------------------------------------------------------------------------------------------------*/
	if( Axis->MotorLessTest->MotorLessTestMode == TRUE )				/* Processing during motorless test			*/
	{
		ALMSetMask(Axis->AlmManager, ALM_DBOL, TRUE);					/* A.730 : DB overload 				*/
		ALMSetMask(Axis->AlmManager, ALM_DBOL_LVL, TRUE);				/* A.731 : DB overload 				*/
	}

}



/****************************************************************************************************/
/*																									*/
/*		System switch setting(PnE0B/E0C/E0D)															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		PnE0B.Bit0 : Semi-closed/Full-closed format selection												*/
/*		PnE0B.Bit1 : Rotation type/Linear type startup selection														*/
/*		PnE0B.Bit4 : CA,CBFilter settings		--> JL056HW.c											*/
/*		PnE0B.Bit5 : CA,CBFilter settings		--> JL056HW.c											*/
/*		PnE0B.Bit8 : Command pulse filter setting --> JL056HW.c											*/
/*		PnE0B.Bit9 : Command pulse filter setting --> JL056HW.c											*/
/*		PnE0B.BitA : Command pulse filter setting --> JL056HW.c											*/
/*		PnE0B.BitB : Command pulse filter setting --> JL056HW.c											*/
/*--------------------------------------------------------------------------------------------------*/
/*		PnE0B.BitC : Speed control integral hold																*/
/*		PnE0B.BitD : Serial encoder clock talk		--> JL056HW.c											*/
/*		PnE0B.BitE : Encoder position correction selection1														*/
/*		PnE0B.BitF : Encoder position correction selection2														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		PnE0C.Bit1 : Power input 3-phase																	*/
/*		PnE0C.Bit2 : Power supply phase loss detection method																*/
/*		PnE0C.BitB : Heat sink overheating detection method														*/
/*		PnE0C.BitC : Enable DB overload detection due to temperature rise														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		PnE0D.Bit0 : User constant writing prohibited															*/
/*		PnE0D.Bit1 : User access level selection														*/
/*		PnE0D.Bit4 : Origin position setting execution																*/
/*		PnE0D.Bit5 : Current zero phase position save execution															*/
/*		PnE0D.Bit8 : Division output selection																	*//*<S02D>*/
/*		PnE39.Bit2 : Main circuit voltage correction function selection		<V079>												*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalSystemSwitch( AXIS_HANDLE *Axis )
{
	PRMDATA				*Prm;
	CMN_PRMDATA			*Cprm;
	FUN_AXCOMMON	*AxCommon;				/* <S056> */

	Prm		= Axis->UniPrms->Prm;
	Cprm	= Axis->UniPrms->Cprm;
	AxCommon = Axis->FnManager->AxCommon;	/* <S056> */


/*--------------------------------------------------------------------------------------------------*/
/*		PnE0B : System switch																	*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		BitC : Speed control integral hold																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( Prm->syssw1 & 0x1000 )
	{
		Axis->BaseControls->CtrlCmdPrm.SpdIctrlKeep = TRUE;
	}
	else
	{
		Axis->BaseControls->CtrlCmdPrm.SpdIctrlKeep = FALSE;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		BitE : Encoder position correction selection(0:Correct,1:No correction)									*/
	/*		BitF : Encoder position correction selection(0:JL-086(Normal processing),1:SW(Σ-ⅤCompatibility))		*/
	/*----------------------------------------------------------------------------------------------*//*<S0C5>rename*/
	if( Prm->syssw1 & 0x4000 )
	{	/* Do not correct position */
		(Axis->MencV)->PosCompMode = POSCOMP_NOP;
	}
	else
	{	/* Position correction */
		if( Prm->syssw1 & 0x8000 )
		{	/* SW		*/
			(Axis->MencV)->PosCompMode = POSCOMP_SW;
		}
		else
		{	/* JL-086 	*/
			(Axis->MencV)->PosCompMode = POSCOMP_ASIC;
		}
	}
	(Axis->FencV)->PosCompMode = (Axis->MencV)->PosCompMode;		/* <S04D> */
/*--------------------------------------------------------------------------------------------------*/
/*		PnE0C : System switch																	*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Bit12 : Whether or not to detect DB overload due to temperature rise												*/
	/*----------------------------------------------------------------------------------------------*/
	if( Cprm->syssw2 & 0x1000 )
	{
		(Axis->CheckAlarm)->DbOL.conf.DbOHeatCheck = TRUE;
	}
	else
	{
		(Axis->CheckAlarm)->DbOL.conf.DbOHeatCheck = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		PnE0D : System switch																	*/
/*--------------------------------------------------------------------------------------------------*/

/* <S056>	Start */
	/*----------------------------------------------------------------------------------------------*/
	/*		Bit1 : User access level selection															*/
	/*----------------------------------------------------------------------------------------------*/
	if( Prm->syssw3  & 0x0002 )	/* PnE0D.1 = 1 User level3 */
	{
//		AxCommon->AccessLevel = 2;	/* ACCLVL_USER2 */	/* <S1B0> */
		AxCommon->AccessLevel = 3;	/* ACCLVL_USER3 */	/* <S1B0> */

	}
	else	/* PnE0D.1 = 0 User level1 */
	{
//		AxCommon->AccessLevel = 1;	/* ACCLVL_USER1 */
		AxCommon->AccessLevel = 2;	/* ACCLVL_USER2 *//* <S20B> */
	}
/* <S056>	End */

	/*----------------------------------------------------------------------------------------------*/
	/*		Bit8 : Division output selection	(1:JL-086,0:SW(Σ-ⅤCompatibility))							*//*<S02D>*/
	/*----------------------------------------------------------------------------------------------*/
	if( Prm->syssw3 & 0x0100 )
	{
		(Axis->MencV)->DivInfo.DivOutASIC = TRUE; 
	}
	else
	{
		(Axis->MencV)->DivInfo.DivOutASIC = FALSE;
	}

}


/****************************************************************************************************/
/*																									*/
/*		Zero speed stop control parameter calculation																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Initial parameter calculation for zero speed stop control												*/
/*																									*/
/*			1) Origin search speed																		*/
/*			   Calculate the JOG speed used in origin search.											*/
/*																									*/
/*			2) Speed command limit																		*/
/*			   Set the speed command limit at half the OS ratio0x1000000/Normalize to OS speed.			*/
/*			   This speed command limit is used in zero clamp and origin search.						*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalZeroStopControl( AXIS_HANDLE *Axis )
{
	LONG	SpeedX;
	BPRMDAT	*Bprm;

	Bprm	= Axis->UniPrms->Bprm;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of origin search speed : 60.0r/min, 15.0mm/s --> NorSpeed [2^24/OvrSpd]						*/
/*--------------------------------------------------------------------------------------------------*/
	if( Axis->MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* Linear type */
		SpeedX = 15;											/* Linear type motor : 15.0 r/min		*/
	}
	else
	{ /* Rotation type */
		SpeedX = 60;											/* Rotary type motor : 60.0 r/min		*/
	}
/*--------------------------------------------------------------------------------------------------*/
	if( Axis->MencV->LowSpdMotor )
	{ /* Low speed motor(DD motor) */
		;
	}
	else
	{ /* Low speed motor(DD motor) */
		SpeedX = 10 * SpeedX;
	}
/*--------------------------------------------------------------------------------------------------*/
	/* Normalized speed calculation [2^24/OvrSpd] */
	Axis->BaseControls->ZeroClampCtrl.conf.OrgSrchSpeed = BprmSpeedLevelCal( Bprm, SpeedX, 0 );

/*--------------------------------------------------------------------------------------------------*/
/*		Speed command limit calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	Axis->BaseControls->ZeroClampCtrl.conf.SpdLimit = 0x01000000 - (Bprm->PerOvrSpd * 838);
	Axis->BaseLoops->ZxCtrl.P.SpdLimit = Axis->BaseControls->ZeroClampCtrl.conf.SpdLimit;			/* <S1C2> */
}



/****************************************************************************************************/
/*																									*/
/*		Carrier frequency switching parameter calculation									<V057>				*/
/*																									*/
/****************************************************************************************************/
void	IprmcalCarrerChange( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
//	CMN_PRMDATA	*Cprm;
	CARRCHNG	*CarrerChange;
	LONG		MotStopDef;		/* Motor stop detection speed */
	LONG		kx, sx, sx2;
	USHORT		Imax;
	SHORT		tmp_ilim;

	Prm = Axis->UniPrms->Prm;
//	Cprm = Axis->UniPrms->Cprm;
	CarrerChange = &(Axis->BaseLoops->CarrerChange);
	MotStopDef = 60000;

	/* Carrier frequency switching function switch flag */
	if( (Prm->b_prm9 & 0x0010) && (Prm->pwmf <= 4000) )
	{ /* (Pn009.1 == 1) && (PnE2C <= 4000) */
		CarrerChange->conf.CarrChange = TRUE;
	}
	else
	{
		CarrerChange->conf.CarrChange = FALSE;
	}

	/* Maximum currentImaxand definition */
	if( Prm->MencP.imax <= Prm->imaxs )
	{
		Imax = Prm->MencP.imax;
	}
	else
	{
		Imax = Prm->imaxs;
	}

#if (FLOAT_USE==TRUE)
	/* Torque thresholdA:PnEA6 */
	kx = 15000 * Prm->MencP.irat / Imax;
	tmp_ilim = (SHORT)(kx * Prm->ilimlvl / 100.0f);
	CarrerChange->conf.IqLimitA = (LONG)tmp_ilim * (LONG)tmp_ilim;
		
	/* Torque thresholdA:PnEA6+10% */
	tmp_ilim = (SHORT)( kx * (Prm->ilimlvl+10) / 100.0f);
	CarrerChange->conf.IqLimitB = (LONG)tmp_ilim * (LONG)tmp_ilim;
#else
	/* Torque thresholdA:PnEA6 */
	kx = MlibScalKxgain( 15000, Prm->MencP.irat, Imax, &sx, 0 );
	sx2 = sx;
	tmp_ilim = (SHORT)MlibPcalKxgain( kx, Prm->ilimlvl, 100, &sx, -24 );
	CarrerChange->conf.IqLimitA = (LONG)tmp_ilim * (LONG)tmp_ilim;
		
	/* Torque thresholdA:PnEA6+10% */
	tmp_ilim = (SHORT)MlibPcalKxgain( kx, Prm->ilimlvl+10, 100, &sx2, -24 );
	CarrerChange->conf.IqLimitB = (LONG)tmp_ilim * (LONG)tmp_ilim;
#endif /* FLOAT_USE */
		
	/* Speed thresholdA:MOTSPD_DBLVL/2 */
	CarrerChange->conf.VelLimitA = MotStopDef >> 1;
		
	/* Speed thresholdB:MOTSPD_DBLVL */
	CarrerChange->conf.VelLimitB = MotStopDef;

	/* Normal carrier count selection		SGD7W IOI/O support<S118> */
#if 0	/* If the motor carrier frequency is on, it is always checked, so no system switch is required<S11E> */
	if( (Cprm->syssw2 & 0x2000) != 0 )
	{/* @@zantei Enable carrier switching function using encoder parameters(PnE0C.bit13) */
		if( (0 == Prm->MencP.motfuncf.f.chgPwm) && (0 != Prm->MencP.mot_pwmf) )
		{/* PnF24.bit0=0is the motor,PWMFrequency(PnF25)but0, use the lower of the motor frequency and amplifier frequency */
			if( Prm->MencP.mot_pwmf >= Prm->pwmf )
			{/* MotorPWM>=AmplifierPWMIn the case of amplifierPWMuse */
				CarrerChange->conf.MotCarrUse = FALSE;
			}
			else
			{/* MotorPWM<AmplifierPWM, check the value of the motor carrier frequency and select which carrier frequency to use for the motorPWMuse */
				CarrerChange->conf.MotCarrUse = TRUE;
			}
		}
		else
		{
			CarrerChange->conf.MotCarrUse = FALSE;
		}
	}
	else
	{
		CarrerChange->conf.MotCarrUse = FALSE;
	}
#else	/* #if 0	 If the motor carrier frequency is on, it is always checked, so no system switch is required<S11E> */
	if( 0 == Prm->MencP.motfuncf.f.chgPwm )
	{/* PnF24.bit0=0In this case, check the motor carrier frequency value and select which carrier frequency to use. */
		if( (0 < Prm->MencP.mot_pwmf) || (16000 >= Prm->MencP.mot_pwmf) )		/* Upper limit16KHSet as z(Code organization)  <S191> */
		{/* Motor carrierPWMIf the frequency is within the range, use the motor frequency */
			CarrerChange->conf.MotCarrUse = TRUE;
		}
		else
		{/* Motor carrierPWMIf the frequency is outside the range, use the amplifier frequency */
			CarrerChange->conf.MotCarrUse = FALSE;
		}
	}
	else
	{/* PnF24.bit0=1, then the amplifier frequency cannot be used */
		CarrerChange->conf.MotCarrUse = FALSE;
	}
#endif	/* #if 0	 If the motor carrier frequency is on, it is always checked, so no system switch is required<S11E> */
	LpxMakeCarrCnt( Prm, CarrerChange );			/* Carrier frequency count creation */

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Carrier frequency count creation process							SGD7W IOI/O support<S118>					*/
/*																									*/
/****************************************************************************************************/
static void	LpxMakeCarrCnt( PRMDATA *Prm, CARRCHNG *CarrerChange )
{
	if( TRUE == CarrerChange->conf.MotCarrUse )
	{/* MotorPWMuse */
		/* PnF25 */
		CarrerChange->conf.NormCarrCnt =
			(SHORT)KpiChangePwmFrequency( 0, Prm->MencP.mot_pwmf, Prm->pwmf3, Prm->MencP.flg_wf );

		/* PnE25 */
		CarrerChange->conf.HighCarrCnt =
			(SHORT)KpiChangePwmFrequency( Prm->pwmf2, Prm->MencP.mot_pwmf, Prm->pwmf3, Prm->MencP.flg_wf );
	}
	else
	{/* AmplifierPWMuse */
		/* PnE2C */
		CarrerChange->conf.NormCarrCnt =
			(SHORT)KpiChangePwmFrequency( 0, Prm->pwmf, Prm->pwmf3, Prm->MencP.flg_wf );

		/* PnE25 */
		CarrerChange->conf.HighCarrCnt =
			(SHORT)KpiChangePwmFrequency( Prm->pwmf2, Prm->pwmf, Prm->pwmf3, Prm->MencP.flg_wf );
	}

	CarrerChange->conf.CarrCnt = CarrerChange->conf.NormCarrCnt;		/* Valid carrier count set 	*/

	return;
}


/****************************************************************************************************/
/*																									*/
/*		Sequence input map calculation																	*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S045> */
static	void	IprmcalSeqInputMap( AXIS_HANDLE *Axis )
{
	PRMDATA				*Prm;
#if 0/* SequenceIOis not supported */
	BPRMDAT				*Bprm;
	SERVO_CONTROL_IF	*SvControlIf;
	SHAL_LTSIGNALS		*LatchSignals;
	ULONG				i;
#endif
	SEQ_IOSIGNAL		*IoSignal;

	Prm	= Axis->UniPrms->Prm;
#if 0/* SequenceIOis not supported */
	Bprm= Axis->UniPrms->Bprm;
	LatchSignals = Axis->LatchSignals;
	SvControlIf = Axis->SvControlIf;
#endif

	/* P-OT mapping */
	IoSignal = &(Axis->IoSignals->PotSignal);
	switch(Prm->pot_selsw >> 12)
	{
	case 1:	/* Switch OFF => active(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = (ULONG)(Prm->pot_selsw & 0xFF);
		break;
	case 2:	/* Switch OFF => inactive(Reverse) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = (ULONG)(Prm->pot_selsw & 0xFF);
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 0;
		break;
	}

	/* N-OT mapping */
	IoSignal = &(Axis->IoSignals->NotSignal);
	switch(Prm->not_selsw >> 12)
	{
	case 1:	/* Switch OFF => active(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = (ULONG)(Prm->not_selsw & 0xFF);
		break;
	case 2:	/* Switch OFF => inactive(Reverse) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = (ULONG)(Prm->not_selsw & 0xFF);
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 0;
		break;
	}

	/* DEC mapping */
	IoSignal = &(Axis->IoSignals->DecSignal);
	switch(Prm->dec_selsw >> 12)
	{
	case 1:	/* Switch ON => active(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = (ULONG)(Prm->dec_selsw & 0xFF);
		break;
	case 2:	/* Switch ON => inactive(Reverse) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = (ULONG)(Prm->dec_selsw & 0xFF);
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 0;
		break;
	}

#if	(CSW_SVCF_FBLATCH == TRUE)
	/* <S04D>:First argument changed to support fully closed encoder */
	if(Axis->UniPrms->Bprm->FencUse != FALSE)
	{
		axisHwReg = Axis->FencV->EncIf.PosCmpSetReg;
	}
	else
	{
		axisHwReg = Axis->MencV->EncIf.PosCmpSetReg;
	}
	if(TRUE != SHalDrv_InitLatchSignal(axisHwReg, Axis->LtStsAdr, Prm->inselA, Axis->SvControlIf->ExtLtEnable))
	{
		/* A.040 : Parameter setting error */
//		ALMSetPramError( Axis->AlmManager, pndef_midtrq.RegNumber );
		ALMSetPramError( Axis->AlmManager, pndef_inselA.RegNumber );	/* <S226> */
	}
#endif

#ifdef CSW_BSP_SGDVMD_B01	/* <S053> */
	/* E-STOP mapping */
	IoSignal = &(Axis->IoSignals->EstpSignal);
	switch(Prm->emg_selsw >> 12)
	{
	case 1:	/* Switch ON => active(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = (ULONG)(Prm->emg_selsw & 0xFF);
		break;
	case 2:	/* Switch ON => inactive(Reverse) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = (ULONG)(Prm->emg_selsw & 0xFF);
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 0;
		break;
	}
#endif

#ifdef CSW_BSP_SGDVMD_B01 /* <S053> */
	/* BK mapping */
	IoSignal = &(Axis->IoSignals->BkSignal);
	switch(Prm->bk_selsw >> 12)
	{
	case 1:	/* Active low(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = KPX_SO1BITNO;
		break;
	case 2:	/* Active high(Reverse) */ // Active Highis dangerous so do not use
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = KPX_SO1BITNO;
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 31;	/* Selects a reserved pin */
		break;
	}
#else
	/* BK mapping */
	IoSignal = &(Axis->IoSignals->BkSignal);
	switch(Prm->bk_selsw >> 12)
	{
	case 1:	/* Active low(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = (ULONG)(Prm->bk_selsw & 0xFF);
		break;
	case 2:	/* Active high(Reverse) */ // Active Highis dangerous so do not use
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = (ULONG)(Prm->bk_selsw & 0xFF);
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 31;	/* Selects a reserved pin */
		break;
	}
#endif

}
#endif

/****************************************************************************************************/
/*																									*/
/*		Sequence input map calculation(M2,M3,CMDDedicated) <S045>												*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalSeqInputMap( AXIS_HANDLE *Axis )
{
#define Lpx_GetDefInSig( prm, pin ) (((((prm) & 0x0F) < 7) || (((prm) & 0x0F) > 8)) ? (pin) : ((prm) & 0x0F))
#if CSW_BOARD_TYPE == SGD7W_TYPE	/* <S200-0> */
#define Lpx_GetExtInSig( prm )      ((((prm) & 0x0F) < 3) || ((((prm) & 0x0F) > 8) && (((prm) & 0x0F) < 0x0C))) \
									? 8 : ((prm) & 0x0F)
#else
#define Lpx_GetExtInSig( prm )      ((((prm) & 0x0F) < 4) || ((((prm) & 0x0F) > 8) && (((prm) & 0x0F) < 0x0D))) \
									? 8 : ((prm) & 0x0F)
#endif
	LONG				Mode;
	PRMDATA				*Prm;

	Prm	= Axis->UniPrms->Prm;
/*--------------------------------------------------------------------------------------------------*/
/*		Input signal assignment mode																			*/
/*--------------------------------------------------------------------------------------------------*/
	Mode = Prm->insel1 & 0x0F;

	if ( Mode == 2 )
	{
		LpxSetInputMapW( Axis );						/* Only SGD7W */
	}
	else
	{
		LpxSetInputMapCmn( Axis, Mode );				/* Common */
	}

/*--------------------------------------------------------------------------------------------------*/
/* <S04A> Estp?Signal assignment. Just in case															 	*/
/*--------------------------------------------------------------------------------------------------*/
#ifdef CSW_BSP_SGDVMD_IFECAT						
	/* E-STOP mapping */
	IoSignal = &(Axis->IoSignals->EstpSignal);
	switch(Prm->emg_selsw >> 12)
	{
	case 1:	/* Switch ON => active(Normal) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = (ULONG)(Prm->emg_selsw & 0xFF);
		break;
	case 2:	/* Switch ON => inactive(Reverse) */
		IoSignal->Mask		= 1;
		IoSignal->XorMask   = 1;
		IoSignal->PinNumber = (ULONG)(Prm->emg_selsw & 0xFF);
		break;
	default:/* Always inactive */
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 0;
		break;
	}
#endif

}

/****************************************************************************************************/
/*																									*/
/*		Sequence input map calculation(Common) <S045>														*/
/*																									*/
/****************************************************************************************************/
void	LpxSetInputMapCmn( AXIS_HANDLE *Axis, LONG Mode )
{
LONG				lwk;
PRMDATA				*Prm;
SEQ_IO_HNDL	 		*IoSignals;

	Prm	= Axis->UniPrms->Prm;
	IoSignals = Axis->IoSignals;
/*--------------------------------------------------------------------------------------------------*/
/*		Input signal default assignment																		*/
/*--------------------------------------------------------------------------------------------------*/
	if ( Mode == 0 )
	{
	/*----------------------------------------------------------------------------------------------*/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	 SGD7W IOI/O support<S118> */
		lwk = Lpx_GetDefInSig( (Prm->insel1 >> 12), 1 );				/* P-OT (M2,M3,CmdteethSI1) */
		LpxPcalSetInputMap2( Axis, &(IoSignals->PotSignal), lwk, 0x01 );	/* P-OT (M2,M3,CmdteethSI1) */
		lwk = Lpx_GetDefInSig( (Prm->insel2 >> 0), 2 );					/* N-OT (M2,M3,CmdteethSI2) */
		LpxPcalSetInputMap2( Axis, &(IoSignals->NotSignal), lwk, 0x01 );	/* N-OT (M2,M3,CmdteethSI2) */
		LpxPcalSetInputMap2( Axis, &(IoSignals->PclSignal), 8, 0x00 );	/* P-CL (M2,M3,CmdteethNA) */
		LpxPcalSetInputMap2( Axis, &(IoSignals->NclSignal), 8, 0x00 );	/* N-CL (M2,M3,CmdteethNA) */
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
		lwk = Lpx_GetDefInSig( (Prm->insel1 >> 12), 1 );			/* P-OT (M2,M3,CmdteethSI1) */
		LpxPcalSetInputMap( &(IoSignals->PotSignal), lwk, 0x01 );	/* P-OT (M2,M3,CmdteethSI1) */
		lwk = Lpx_GetDefInSig( (Prm->insel2 >> 0), 2 );				/* N-OT (M2,M3,CmdteethSI2) */
		LpxPcalSetInputMap( &(IoSignals->NotSignal), lwk, 0x01 );	/* N-OT (M2,M3,CmdteethSI2) */
		LpxPcalSetInputMap( &(IoSignals->PclSignal), 8, 0x00 );		/* P-CL (M2,M3,CmdteethNA) */
		LpxPcalSetInputMap( &(IoSignals->NclSignal), 8, 0x00 );		/* N-CL (M2,M3,CmdteethNA) */
#endif
	/*----------------------------------------------------------------------------------------------*/
	}
	else
	{
	/*----------------------------------------------------------------------------------------------*/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only  SGD7W IOI/O support<S118> */
		LpxPcalSetInputMap2( Axis, &(IoSignals->PotSignal), (Prm->insel1 >> 12), 0x01 );	/* P-OT */
		LpxPcalSetInputMap2( Axis, &(IoSignals->NotSignal), (Prm->insel2 >> 0),  0x01 );	/* N-OT */
		LpxPcalSetInputMap2( Axis, &(IoSignals->PclSignal), (Prm->insel2 >> 8),  0x00 );	/* P-CL */
		LpxPcalSetInputMap2( Axis, &(IoSignals->NclSignal), (Prm->insel2 >> 12), 0x00 );	/* N-CL */
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
		LpxPcalSetInputMap( &(IoSignals->PotSignal), (Prm->insel1 >> 12), 0x01 );	/* P-OT */
		LpxPcalSetInputMap( &(IoSignals->NotSignal), (Prm->insel2 >> 0),  0x01 );	/* N-OT */
		LpxPcalSetInputMap( &(IoSignals->PclSignal), (Prm->insel2 >> 8),  0x00 );	/* P-CL */
		LpxPcalSetInputMap( &(IoSignals->NclSignal), (Prm->insel2 >> 12), 0x00 );	/* N-CL */
#endif
	/*----------------------------------------------------------------------------------------------*/
	}
/*--------------------------------------------------------------------------------------------------*/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only  SGD7W IOI/O support<S118> */
	LpxPcalSetInputMap2( Axis, &(IoSignals->DecSignal), (Prm->inselA >> 0), 0x00 );	/* DEC */
	lwk = Lpx_GetExtInSig( (Prm->inselA >> 4) );
	LpxPcalSetInputMap2( Axis, &(IoSignals->ExtSignal[0]), lwk , 0x00 );				/* EXT1 */
	lwk = Lpx_GetExtInSig( (Prm->inselA >> 8) );
	LpxPcalSetInputMap2( Axis, &(IoSignals->ExtSignal[1]), lwk , 0x00 );				/* EXT2 */
	lwk = Lpx_GetExtInSig( (Prm->inselA >> 12) );
	LpxPcalSetInputMap2( Axis, &(IoSignals->ExtSignal[2]), lwk , 0x00 );				/* EXT3 */
	lwk = /*Lpx_GetExtInSig( (*/Prm->insel6/* >> 0) )*/;		/* <S1F5> */
	LpxPcalSetInputMap2( Axis, &(IoSignals->FstpSignal), lwk , 0x01/* @@ 0x00 */); 					/* FSTP */   /* <S1F5> */
	/* <S21D> Start */
	LpxPcalSetInputMap2( Axis, &(IoSignals->ClinkansSignal), Prm->insel7 >> 0 , 0x01 ); 	/* /CLINKANS */
	LpxPcalSetInputMap2( Axis, &(IoSignals->CmcansSignal), Prm->insel7 >> 4 , 0x01 ); 		/* /CMCANS */
	LpxPcalSetInputMap2( Axis, &(IoSignals->ConvreadySignal), Prm->insel7 >> 8 , 0x01 ); 	/* /CONV_READY */
	LpxPcalSetInputMap2( Axis, &(IoSignals->ConvmconSignal), Prm->insel7 >> 12 , 0x01 ); 	/* /CONV_MCON */
	/* <S21D> End */
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
	LpxPcalSetInputMap( &(IoSignals->DecSignal), (Prm->inselA >> 0), 0x00 );	/* DEC */
	lwk = Lpx_GetExtInSig( (Prm->inselA >> 4) );
	LpxPcalSetInputMap( &(IoSignals->ExtSignal[0]), lwk , 0x00 );				/* EXT1 */
	lwk = Lpx_GetExtInSig( (Prm->inselA >> 8) );
	LpxPcalSetInputMap( &(IoSignals->ExtSignal[1]), lwk , 0x00 );				/* EXT2 */
	lwk = Lpx_GetExtInSig( (Prm->inselA >> 12) );
	LpxPcalSetInputMap( &(IoSignals->ExtSignal[2]), lwk , 0x00 );				/* EXT3 */
	lwk = /*Lpx_GetExtInSig( (*/Prm->insel6 /*>> 0) )*/;	/* <S1F5> */
	LpxPcalSetInputMap( &(IoSignals->FstpSignal), lwk , 0x01/* @@ 0x00 */ ); 				/* FSTP */ /* <S1F5> */
	/* <S21D> Start */
	LpxPcalSetInputMap( &(IoSignals->ClinkansSignal), Prm->insel7 >> 0 , 0x00 ); 	/* /CLINKANS */
	LpxPcalSetInputMap( &(IoSignals->CmcansSignal), Prm->insel7 >> 4 , 0x00 ); 		/* /CMCANS */
	LpxPcalSetInputMap( &(IoSignals->ConvreadySignal), Prm->insel7 >> 8 , 0x00 ); 	/* /CONV_READY */
	LpxPcalSetInputMap( &(IoSignals->ConvmconSignal), Prm->insel7 >> 12 , 0x00 ); 	/* /CONV_MCON */
	/* <S21D> End */
#endif
/*--------------------------------------------------------------------------------------------------*/
/*	<S04A> Initialize External Signal Latch Assign													*/
/*--------------------------------------------------------------------------------------------------*/
#if	(CSW_SVCF_FBLATCH == TRUE)
	if(TRUE != SHalDrv_InitLatchSignal(Axis->SvAsicRegs->AsicHwReg, 
										Axis->LtStsAdr, Prm->inselA, Axis->SvControlIf->ExtLtEnable))
	{
		/* A.040 : Parameter setting error */
//		ALMSetPramError( Axis->AlmManager, pndef_midtrq.RegNumber );
		ALMSetPramError( Axis->AlmManager, pndef_inselA.RegNumber );	/* <S226> */
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Sequence input map calculation(SGD7WDedicated) <S1B5>													*/
/*																									*/
/****************************************************************************************************/
void	LpxSetInputMapW( AXIS_HANDLE *Axis )
{
PRMDATA				*Prm;
SEQ_IO_HNDL	 		*IoSignals;

	Prm	= Axis->UniPrms->Prm;
	IoSignals = Axis->IoSignals;
/*--------------------------------------------------------------------------------------------------*/
/*	Set Input Signal Map for SGD7W																	*/
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalSetInputMap3( &(IoSignals->PotSignal), Prm->pot_selsw, 1 );
	LpxPcalSetInputMap3( &(IoSignals->NotSignal), Prm->not_selsw, 1 );
	LpxPcalSetInputMap3( &(IoSignals->DecSignal), Prm->dec_selsw, 0 );
	LpxPcalSetInputMap3( &(IoSignals->ExtSignal[0]), Prm->ext_selsw[0], 0 );
	LpxPcalSetInputMap3( &(IoSignals->ExtSignal[1]), Prm->ext_selsw[1], 0 );
	LpxPcalSetInputMap3( &(IoSignals->ExtSignal[2]), Prm->ext_selsw[2], 0 );
	/*----------------------------------------------------------------------------------------------*/
	LpxPcalSetInputMap3( &(IoSignals->FstpSignal), Prm->fstp_selsw, 1 );
	LpxPcalSetInputMap3( &(IoSignals->PclSignal), Prm->pcl_selsw, 0 );
	LpxPcalSetInputMap3( &(IoSignals->NclSignal), Prm->ncl_selsw, 0 );
	LpxPcalSetInputMap3( &(IoSignals->ClinkansSignal), Prm->clinkans_selsw, 0 );
	LpxPcalSetInputMap3( &(IoSignals->CmcansSignal), Prm->cmcans_selsw, 0 );
	LpxPcalSetInputMap3( &(IoSignals->ConvreadySignal), Prm->convready_selsw, 0 );
	LpxPcalSetInputMap3( &(IoSignals->ConvmconSignal), Prm->convmcon_selsw, 0 );
/*--------------------------------------------------------------------------------------------------*/
/*	Initialize External Signal Latch Assign															*/
/*--------------------------------------------------------------------------------------------------*/
	LpxInitLatchSignal( Axis );
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Sequence input map calculation3(SGD7WDedicated) <S1B5>													*/
/*																									*/
/****************************************************************************************************/
void	LpxPcalSetInputMap3( SEQ_IOSIGNAL *IoSignal, ULONG prm, ULONG LogicH ) 
{
LONG	Pin;
ULONG	LogicX;
ULONG	LogicL = (LogicH) ? 0x00 : 0x01;
/*--------------------------------------------------------------------------------------------------*/
#define Lpx_GetBcd( prm )			((((prm) & 0x0F) * 1) 			/* <S1BF> */					\
									+ ((((prm)>>4) & 0x0F) * 10))
/*--------------------------------------------------------------------------------------------------*/
#define INPUT_MAP3_LGC_NONACTIVE 	(0)
#define INPUT_MAP3_LGC_NORMAL 		(1)
#define INPUT_MAP3_LGC_OPPOSITE 	(2)
#define INPUT_MAP3_LGC_ACTIVE 		(3)
/*--------------------------------------------------------------------------------------------------*/
	Pin = HALdrv_GetPsBitNumber( Lpx_GetBcd( prm ) );				/* Get Pin <S1BF> */
	LogicX = ( prm >> 12 ) & 0x0F;									/* Get Logic */
//	LogicX = (( Pin < 0 ) ? ((LogicH) ? INPUT_MAP3_LGC_ACTIVE 		/* Disable Pin <S1BF> */
//					: INPUT_MAP3_LGC_NONACTIVE) : LogicX);
	LogicX = (( Pin < 0 ) ? INPUT_MAP3_LGC_NONACTIVE : LogicX);		/* Disable Pin <S1F5> */
/*--------------------------------------------------------------------------------------------------*/
/*	Set Input Signal Map																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch ( LogicX )
	{
	/*----------------------------------------------------------------------------------------------*/
	case INPUT_MAP3_LGC_NONACTIVE:									/* 0: Non Active				*/
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = KPI_SINLOBITNO;
			IoSignal->Mask		= 0;
			IoSignal->XorMask	= 0;
			break;
	/*----------------------------------------------------------------------------------------------*/
	case INPUT_MAP3_LGC_NORMAL:										/* 1: Normal					*/
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = (ULONG)Pin;
			IoSignal->Mask		= 1;
			IoSignal->XorMask	= LogicL;
			break;
	/*----------------------------------------------------------------------------------------------*/
	case INPUT_MAP3_LGC_OPPOSITE:									/* 2: Opposite					*/
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = (ULONG)Pin;
			IoSignal->Mask		= 1;
			IoSignal->XorMask	= LogicH;
			break;
	/*----------------------------------------------------------------------------------------------*/
	case INPUT_MAP3_LGC_ACTIVE:										/* 3: Always active				*/
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = KPI_SINHIBITNO;
			IoSignal->Mask		= 1;
			IoSignal->XorMask	= 0;
			break;
	/*----------------------------------------------------------------------------------------------*/
	default:
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = KPI_SINLOBITNO;
			IoSignal->Mask		= 0;
			IoSignal->XorMask	= 0;
			break;
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}

/****************************************************************************************************/
/*																									*/
/*		<S1B5> Get insel Parameter Number															*/
/*																									*/
/****************************************************************************************************/
LONG	LpxGetInSel( LONG Axis, ULONG prm )
{
LONG	wkx;
LONG	Pin;
ULONG	LogicX;
/*--------------------------------------------------------------------------------------------------*/
	Pin = HALdrv_GetPsBitNumber( Lpx_GetBcd( prm ) );				/* Get Pin <S1BF> */
	LogicX = ( prm >> 12 ) & 0x0F;									/* Get Logic */
	LogicX = (( Pin < 0 ) ? INPUT_MAP3_LGC_NONACTIVE : LogicX);		/* Disable Pin */
/*--------------------------------------------------------------------------------------------------*/
	if ( LogicX == INPUT_MAP3_LGC_NONACTIVE )
	{
		return (8);													/* Not Available */
	}
	else if ( ( LogicX == INPUT_MAP3_LGC_ACTIVE ) )
	{
		return (7);
	}
	else if ( LogicX == INPUT_MAP3_LGC_NORMAL ) 
	{
		wkx = 0;
	}
	else if ( LogicX == INPUT_MAP3_LGC_OPPOSITE )
	{
		wkx = 9;
	}
	else 
	{
		return (8);													/* Not Available */
	}
/*--------------------------------------------------------------------------------------------------*/
	if ( Axis == 0 ) 												/* Axis 0 */
	{
		switch ( Pin )
		{
		case KPI_SI5BITNO: 	wkx++;
		case KPI_SI4BITNO: 	wkx++;
		case KPI_SI3BITNO: 	wkx++;
		case KPI_SI2BITNO: 	wkx++;
		case KPI_SI1BITNO: 	wkx++;
		case KPI_SI0BITNO: 	break;
		default: return (8);										/* Not available */
		}
	}
	else															/* Axis 1 */
	{
		switch ( Pin )
		{
		case KPI_SI11BITNO:	wkx++;
		case KPI_SI10BITNO:	wkx++;
		case KPI_SI9BITNO: 	wkx++;
		case KPI_SI8BITNO: 	wkx++;
		case KPI_SI7BITNO: 	wkx++;
		case KPI_SI6BITNO: 	break;
		default: return (8);										/* Not available */
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	return (wkx);
}

/****************************************************************************************************/
/*																									*/
/*		Latch signal assignment(SGD7WDedicated) <S1B5>															*/
/*																									*/
/****************************************************************************************************/
void	LpxInitLatchSignal( AXIS_HANDLE *Axis )
{
LONG				inselA;
PRMDATA				*Prm;

	Prm	= Axis->UniPrms->Prm;
/*--------------------------------------------------------------------------------------------------*/
/*	Make inselA																						*/
/*--------------------------------------------------------------------------------------------------*/
	inselA = 0;
	inselA |= (LpxGetInSel( Axis->AxisNo, Prm->ext_selsw[0] ) << 4 );
	inselA |= (LpxGetInSel( Axis->AxisNo, Prm->ext_selsw[1] ) << 8 );
	inselA |= (LpxGetInSel( Axis->AxisNo, Prm->ext_selsw[2] ) << 12);
/*--------------------------------------------------------------------------------------------------*/
/*	 Initialize External Signal Latch Assign														*/
/*--------------------------------------------------------------------------------------------------*/
#if	(CSW_SVCF_FBLATCH == TRUE)
	if(TRUE != SHalDrv_InitLatchSignal(Axis->SvAsicRegs->AsicHwReg, 
										Axis->LtStsAdr, inselA, Axis->SvControlIf->ExtLtEnable))
	{
		/* A.040 : Parameter setting error */
//		ALMSetPramError( Axis->AlmManager, pndef_midtrq.RegNumber );
		ALMSetPramError( Axis->AlmManager, pndef_inselA.RegNumber );	/* <S226> */
	}
#endif
}

/****************************************************************************************************/
/*																									*/
/*		Sequence input map calculation(M2,M3,CMDDedicated) <S045>												*/
/*																									*/
/****************************************************************************************************/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only SGD7W IOI/O support<S118>	*/
static	void	LpxPcalSetInputMap2( AXIS_HANDLE *Axis, SEQ_IOSIGNAL *IoSignal, ULONG prm, ULONG LogicH )
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
static	void	LpxPcalSetInputMap( SEQ_IOSIGNAL *IoSignal, ULONG prm, ULONG LogicH ) 
#endif
{
ULONG	LogicL = (LogicH) ? 0x00 : 0x01;
/*--------------------------------------------------------------------------------------------------*/
	IoSignal->Mask		= 1;
/*--------------------------------------------------------------------------------------------------*/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	SGD7W IOI/O support<S118>	*/
	if(Axis->AxisNo == 0 )	/*	1Axis number	*/
	{	/*	1Axis number	*/
		switch ( prm & 0x0F )
		{
			case 0x00: IoSignal->PinNumber = KPI_SI0BITNO; IoSignal->XorMask = LogicL; break;
			case 0x01: IoSignal->PinNumber = KPI_SI1BITNO; IoSignal->XorMask = LogicL; break;
			case 0x02: IoSignal->PinNumber = KPI_SI2BITNO; IoSignal->XorMask = LogicL; break;
			case 0x03: IoSignal->PinNumber = KPI_SI3BITNO; IoSignal->XorMask = LogicL; break;
			case 0x04: IoSignal->PinNumber = KPI_SI4BITNO; IoSignal->XorMask = LogicL; break;
			case 0x05: IoSignal->PinNumber = KPI_SI5BITNO; IoSignal->XorMask = LogicL; break;
//-defaultfart	case 0x06: IoSignal->PinNumber = KPI_SI6BITNO; IoSignal->XorMask = LogicL; break;
			case 0x07:
			default  : IoSignal->PinNumber = KPI_SINHIBITNO; IoSignal->XorMask = 0x00; break;
			case 0x08: IoSignal->PinNumber = KPI_SINLOBITNO; IoSignal->XorMask = 0x00; break;
			case 0x09: IoSignal->PinNumber = KPI_SI0BITNO; IoSignal->XorMask = LogicH; break;
			case 0x0A: IoSignal->PinNumber = KPI_SI1BITNO; IoSignal->XorMask = LogicH; break;
			case 0x0B: IoSignal->PinNumber = KPI_SI2BITNO; IoSignal->XorMask = LogicH; break;
			case 0x0C: IoSignal->PinNumber = KPI_SI3BITNO; IoSignal->XorMask = LogicH; break;
			case 0x0D: IoSignal->PinNumber = KPI_SI4BITNO; IoSignal->XorMask = LogicH; break;
			case 0x0E: IoSignal->PinNumber = KPI_SI5BITNO; IoSignal->XorMask = LogicH; break;
//--defaultfartcase 0x0F: IoSignal->PinNumber = KPI_SI6BITNO; IoSignal->XorMask = LogicH; break;
		}
	}else
	{	/*	2Axis number	*/
		switch ( prm & 0x0F )
		{
			case 0x00: IoSignal->PinNumber = KPI_SI0BITNO - 6; IoSignal->XorMask = LogicL; break;
			case 0x01: IoSignal->PinNumber = KPI_SI1BITNO - 6; IoSignal->XorMask = LogicL; break;
			case 0x02: IoSignal->PinNumber = KPI_SI2BITNO - 6; IoSignal->XorMask = LogicL; break;
			case 0x03: IoSignal->PinNumber = KPI_SI3BITNO - 6; IoSignal->XorMask = LogicL; break;
			case 0x04: IoSignal->PinNumber = KPI_SI4BITNO - 6; IoSignal->XorMask = LogicL; break;
			case 0x05: IoSignal->PinNumber = KPI_SI5BITNO - 6; IoSignal->XorMask = LogicL; break;
//-defaultfart	case 0x06: IoSignal->PinNumber = KPI_SI6BITNO; IoSignal->XorMask = LogicL; break;
			case 0x07:
			default  : IoSignal->PinNumber = KPI_SINHIBITNO; IoSignal->XorMask = 0x00; break;
			case 0x08: IoSignal->PinNumber = KPI_SINLOBITNO; IoSignal->XorMask = 0x00; break;
			case 0x09: IoSignal->PinNumber = KPI_SI0BITNO - 6; IoSignal->XorMask = LogicH; break;
			case 0x0A: IoSignal->PinNumber = KPI_SI1BITNO - 6; IoSignal->XorMask = LogicH; break;
			case 0x0B: IoSignal->PinNumber = KPI_SI2BITNO - 6; IoSignal->XorMask = LogicH; break;
			case 0x0C: IoSignal->PinNumber = KPI_SI3BITNO - 6; IoSignal->XorMask = LogicH; break;
			case 0x0D: IoSignal->PinNumber = KPI_SI4BITNO - 6; IoSignal->XorMask = LogicH; break;
			case 0x0E: IoSignal->PinNumber = KPI_SI5BITNO - 6; IoSignal->XorMask = LogicH; break;
//-defaultfart	case 0x0F: IoSignal->PinNumber = KPI_SI6BITNO; IoSignal->XorMask = LogicH; break;
		}
	}
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
	switch ( prm & 0x0F )
	{
		case 0x00: IoSignal->PinNumber = KPI_SI0BITNO; IoSignal->XorMask = LogicL; break;
		case 0x01: IoSignal->PinNumber = KPI_SI1BITNO; IoSignal->XorMask = LogicL; break;
		case 0x02: IoSignal->PinNumber = KPI_SI2BITNO; IoSignal->XorMask = LogicL; break;
		case 0x03: IoSignal->PinNumber = KPI_SI3BITNO; IoSignal->XorMask = LogicL; break;
		case 0x04: IoSignal->PinNumber = KPI_SI4BITNO; IoSignal->XorMask = LogicL; break;
		case 0x05: IoSignal->PinNumber = KPI_SI5BITNO; IoSignal->XorMask = LogicL; break;
		case 0x06: IoSignal->PinNumber = KPI_SI6BITNO; IoSignal->XorMask = LogicL; break;
		case 0x07:
		default  : IoSignal->PinNumber = KPI_SINHIBITNO; IoSignal->XorMask = 0x00; break;
		case 0x08: IoSignal->PinNumber = KPI_SINLOBITNO; IoSignal->XorMask = 0x00; break;
		case 0x09: IoSignal->PinNumber = KPI_SI0BITNO; IoSignal->XorMask = LogicH; break;
		case 0x0A: IoSignal->PinNumber = KPI_SI1BITNO; IoSignal->XorMask = LogicH; break;
		case 0x0B: IoSignal->PinNumber = KPI_SI2BITNO; IoSignal->XorMask = LogicH; break;
		case 0x0C: IoSignal->PinNumber = KPI_SI3BITNO; IoSignal->XorMask = LogicH; break;
		case 0x0D: IoSignal->PinNumber = KPI_SI4BITNO; IoSignal->XorMask = LogicH; break;
		case 0x0E: IoSignal->PinNumber = KPI_SI5BITNO; IoSignal->XorMask = LogicH; break;
		case 0x0F: IoSignal->PinNumber = KPI_SI6BITNO; IoSignal->XorMask = LogicH; break;
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
}

/****************************************************************************************************/
/*																									*/
/*		Sequence output map calculation(M2,M3,CMDDedicated) <S045>												*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalSeqOutputMap( AXIS_HANDLE *Axis )
{
/*--------------------------------------------------------------------------------------------------*/
#define LPX_SETSOUTPTR( signal, ptr ) 	\
		SeqIo = &(IoSignals->##signal); \
		SeqIo->SignalStatus.p = (ptr);
/*--------------------------------------------------------------------------------------------------*/
	PRMDATA				*Prm;
	SEQ_IOSIGNAL		*SeqIo;
	SEQ_IO_HNDL	 		*IoSignals;
	BASE_CTRL_HNDL		*BaseControls;
	SEQ_CTRL_OUT		*SeqCtrlOut;
	BE_SEQ_HNDL			*BeSequence;
	ALMMAN_HANDLE		*AlmManager;
	CHECK_MAIN_POWER	*PowerChecker;/* <S154> */

	Prm	= Axis->UniPrms->Prm;
	IoSignals = Axis->IoSignals;
	BaseControls = Axis->BaseControls;
	SeqCtrlOut = Axis->SeqCtrlOut;
	BeSequence = Axis->BeSequence;
	AlmManager = Axis->AlmManager;
	PowerChecker = &(Axis->PowerManager->MainPowerChecker);/* <S154> */

/*--------------------------------------------------------------------------------------------------*/
/*		Input signal assignment mode																			*/
/*--------------------------------------------------------------------------------------------------*/
	if ( ( Prm->insel1 & 0x0F ) == 2 )
	{
		LpxSetOutputMapW( Axis );						/* Only SGD7W */
	}
	else
	{
		LpxSetOutputMapCmn( Axis );						/* Common */
	}

//<S081>  #if CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE /* <S04B> */
/*--------------------------------------------------------------------------------------------------*/
/* Set Output Signal Pointers (M2/M3/CMD Only) 														*/
/*--------------------------------------------------------------------------------------------------*/
	LPX_SETSOUTPTR( CoinSignal, &(BaseControls->PosCtrlSts.CoinSignal) );	/* /COIN */
	LPX_SETSOUTPTR( VcmpSignal, &(SeqCtrlOut->VcmpSts) );					/* /VCMP */
	LPX_SETSOUTPTR( TgonSignal, &(SeqCtrlOut->TgOnSts) );					/* /TGON */
	LPX_SETSOUTPTR( SrdySignal, &(BeSequence->ServoReady) );				/* /S-RDY */
/*--------------------------------------------------------------------------------------------------*/
	LPX_SETSOUTPTR( CltSignal,  &(BaseControls->CtrlCmdMngr.TrqClamp) );	/* /CLT */
	LPX_SETSOUTPTR( VltSignal,  &(BaseControls->CtrlCmdMngr.SpdClamp) );	/* /VLT */
	LPX_SETSOUTPTR( BkSignal,   &(SeqCtrlOut->BrkSigSts) );					/* /BK */
	LPX_SETSOUTPTR( WarnSignal, &(AlmManager->WrnFlagX) );					/* /WARN <S0FF> */
/*--------------------------------------------------------------------------------------------------*/
	LPX_SETSOUTPTR( NearSignal, &(BaseControls->PosCtrlSts.NearSignal) );	/* /NEAR */
/*--------------------------------------------------------------------------------------------------*/
	LPX_SETSOUTPTR( PreWrnSignal,  &(PowerChecker->var.PreSigOut) );		/* /PreWrn */ /* <S154> */
	LPX_SETSOUTPTR( ClinkSignal,  &(PowerChecker->var.ClinkOut) );		/* /CLINK *//* <S21D> */
	LPX_SETSOUTPTR( CmconSignal,  &(PowerChecker->var.CmconOut) );		/* /CMCON *//* <S21D> */
//<S081>  #endif /* CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE */

#if CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE /* <S04B> */
/*--------------------------------------------------------------------------------------------------*/
/* Set Signal Maps (A/P Only)																		*/
/*--------------------------------------------------------------------------------------------------*/
	LPX_SETSOUTPTR( Alo1Signal,  &(AlmManager->AlmCodeOut.Alo1) );			/* ALO1 */
	LPX_SETSOUTPTR( Alo2Signal,  &(AlmManager->AlmCodeOut.Alo2) );			/* ALO2 */
	LPX_SETSOUTPTR( Alo3Signal,  &(AlmManager->AlmCodeOut.Alo3) );			/* ALO3 */
#endif /* CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE */

}

/****************************************************************************************************/
/*																									*/
/*		<S1B5> Sequence output map calculation(Common) 														*/
/*																									*/
/****************************************************************************************************/
void	LpxSetOutputMapW( AXIS_HANDLE *Axis )
{
	PRMDATA				*Prm;
	SEQ_IO_HNDL	 		*IoSignals;

	Prm	= Axis->UniPrms->Prm;
	IoSignals = Axis->IoSignals;
/*--------------------------------------------------------------------------------------------------*/
/* Set Signal Map 																					*/
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalSetSoutMap3( &(IoSignals->CoinSignal), Prm->coin_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->VcmpSignal), Prm->vcmp_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->TgonSignal), Prm->tgon_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->SrdySignal), Prm->srdy_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->CltSignal), Prm->clt_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->VltSignal), Prm->vlt_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->BkSignal), Prm->bk_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->WarnSignal), Prm->warn_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->NearSignal), Prm->near_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->CpulseSignal), Prm->cpulse_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->ClinkSignal), Prm->clink_selsw, 0 );
	LpxPcalSetSoutMap3( &(IoSignals->CmconSignal), Prm->cmcon_selsw, 0 );
	//	LpxPcalSetSoutMap3( &(IoSignals->PmSignal), Prm->pm_selsw, 0 );			/* <S21D> */
	LpxPcalSetSoutMap3( &(IoSignals->PreWrnSignal), Prm->pm_selsw, 0 );			/* <S21D> */
	LpxPcalSetSoutMap3( &(IoSignals->DaoSignal[0]), Prm->dao_selsw[0], 0 );
	LpxPcalSetSoutMap3( &(IoSignals->DaoSignal[1]), Prm->dao_selsw[1], 0 );
/*--------------------------------------------------------------------------------------------------*/
	return; 
}

/****************************************************************************************************/
/*																									*/
/*		<S1B5> Sequence output map calculation(Common) 														*/
/*																									*/
/****************************************************************************************************/
void	LpxSetOutputMapCmn( AXIS_HANDLE *Axis )
{
	PRMDATA				*Prm;
	SEQ_IO_HNDL	 		*IoSignals;
	ULONG				OppositeMode; /* <S04B> */

	Prm	= Axis->UniPrms->Prm;
	IoSignals = Axis->IoSignals;
	OppositeMode = Prm->sorev1 + (Prm->sorev2 << 16); /* <S04B> */

/*--------------------------------------------------------------------------------------------------*/
/* Set Signal Maps (M2/M3/CMD Only) 																*/
/*--------------------------------------------------------------------------------------------------*/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	 SGD7W IOI/O support<S118>	*/
	LpxPcalSetSoutMap2( Axis, &(IoSignals->CoinSignal), (Prm->outsel1 >> 0),  OppositeMode );	/* /COIN */
	LpxPcalSetSoutMap2( Axis, &(IoSignals->VcmpSignal), (Prm->outsel1 >> 4),  OppositeMode );	/* /VCMP */
	LpxPcalSetSoutMap2( Axis, &(IoSignals->TgonSignal), (Prm->outsel1 >> 8),  OppositeMode );	/* /TGON */
	LpxPcalSetSoutMap2( Axis, &(IoSignals->SrdySignal), (Prm->outsel1 >> 12), OppositeMode );	/* /S-RDY */
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalSetSoutMap2( Axis, &(IoSignals->CltSignal),  (Prm->outsel2 >> 0),  OppositeMode );	/* /CLT */
	LpxPcalSetSoutMap2( Axis, &(IoSignals->VltSignal),  (Prm->outsel2 >> 4),  OppositeMode );	/* /VLT */
	LpxPcalSetSoutMap2( Axis, &(IoSignals->BkSignal),   (Prm->outsel2 >> 8),  OppositeMode );	/* /BK */
	LpxPcalSetSoutMap2( Axis, &(IoSignals->WarnSignal), (Prm->outsel2 >> 12), OppositeMode );	/* /WARN */
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalSetSoutMap2( Axis, &(IoSignals->NearSignal), (Prm->outsel3 >> 0), OppositeMode );	/* /NEAR */

	LpxPcalSetSoutMap2( Axis, &(IoSignals->PreWrnSignal), (Prm->outsel4 >> 8), OppositeMode );	/* /PreWrn */ /*<S154>*/
	LpxPcalSetSoutMap2( Axis, &(IoSignals->ClinkSignal), (Prm->outsel4 >> 0), OppositeMode );	/* /CLINK *//* <S21D> */
	LpxPcalSetSoutMap2( Axis, &(IoSignals->CmconSignal), (Prm->outsel4 >> 4), OppositeMode );	/* /CMCON *//* <S21D> */
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
	LpxPcalSetSoutMap( &(IoSignals->CoinSignal), (Prm->outsel1 >> 0),  OppositeMode );	/* /COIN */
	LpxPcalSetSoutMap( &(IoSignals->VcmpSignal), (Prm->outsel1 >> 4),  OppositeMode );	/* /VCMP */
	LpxPcalSetSoutMap( &(IoSignals->TgonSignal), (Prm->outsel1 >> 8),  OppositeMode );	/* /TGON */
	LpxPcalSetSoutMap( &(IoSignals->SrdySignal), (Prm->outsel1 >> 12), OppositeMode );	/* /S-RDY */
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalSetSoutMap( &(IoSignals->CltSignal),  (Prm->outsel2 >> 0),  OppositeMode );	/* /CLT */
	LpxPcalSetSoutMap( &(IoSignals->VltSignal),  (Prm->outsel2 >> 4),  OppositeMode );	/* /VLT */
	LpxPcalSetSoutMap( &(IoSignals->BkSignal),   (Prm->outsel2 >> 8),  OppositeMode );	/* /BK */
	LpxPcalSetSoutMap( &(IoSignals->WarnSignal), (Prm->outsel2 >> 12), OppositeMode );	/* /WARN */
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalSetSoutMap( &(IoSignals->NearSignal), (Prm->outsel3 >> 0), OppositeMode );	/* /NEAR */

	LpxPcalSetSoutMap( &(IoSignals->PreWrnSignal), (Prm->outsel4 >> 8), OppositeMode );	/* /PreWrn */ /*<S154>*/
	LpxPcalSetSoutMap( &(IoSignals->ClinkSignal), (Prm->outsel4 >> 0), OppositeMode );	/* /CLINK *//* <S21D> */
	LpxPcalSetSoutMap( &(IoSignals->CmconSignal), (Prm->outsel4 >> 4), OppositeMode );	/* /CMCON *//* <S21D> */
#endif
/*--------------------------------------------------------------------------------------------------*/

#if CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE /* <S04B> */
/*--------------------------------------------------------------------------------------------------*/
/* Set Signal Maps (A/P Only)																		*/
/*--------------------------------------------------------------------------------------------------*/
	LpxPcalSetSoutMap( &(IoSignals->Alo1Signal), (Prm->outsel6 >> 0), OppositeMode );	/* ALO1 *//* <S04B> */
	LpxPcalSetSoutMap( &(IoSignals->Alo2Signal), (Prm->outsel6 >> 4), OppositeMode );	/* ALO2 *//* <S04B> */
	LpxPcalSetSoutMap( &(IoSignals->Alo3Signal), (Prm->outsel6 >> 8), OppositeMode );	/* ALO3 *//* <S04B> */
#endif /* CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE */
				
	return;
}

/****************************************************************************************************/
/*																									*/
/*		Sequence output map calculation <S1B5>																*/
/*																									*/
/****************************************************************************************************/
void	LpxPcalSetSoutMap3( SEQ_IOSIGNAL *IoSignal, ULONG prm, ULONG LogicH )
{
LONG	Pin;
ULONG	LogicX;
//ULONG	LogicL = (LogicH) ? 0x00 : 0x01;
#define OUTPUT_MAP3_LGC_NONACTIVE 	(0)
#define OUTPUT_MAP3_LGC_NORMAL 		(1)
#define OUTPUT_MAP3_LGC_OPPOSITE 	(2)
/*--------------------------------------------------------------------------------------------------*/
	Pin = HALdrv_GetSpBitNumber( Lpx_GetBcd( prm ) );				/* Get Pin <S1BF> */
	LogicX = ( prm >> 12 ) & 0x0F;									/* Get Logic */
	LogicX = (( Pin < 0 ) ? OUTPUT_MAP3_LGC_NONACTIVE : LogicX);	/* Disable Pin */
/*--------------------------------------------------------------------------------------------------*/
/*	Set Output Signal Map																			*/
/*--------------------------------------------------------------------------------------------------*/
	switch ( LogicX )
	{
	/*----------------------------------------------------------------------------------------------*/
	case OUTPUT_MAP3_LGC_NONACTIVE:									/* 0: Non Active				*/
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = 31;								/* Interim */
			IoSignal->Mask		= 0;
			IoSignal->XorMask	= 0;
			break;
	/*----------------------------------------------------------------------------------------------*/
	case OUTPUT_MAP3_LGC_NORMAL:									/* 1: Normal					*/
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = (ULONG)Pin;
			IoSignal->Mask		= 1;
			IoSignal->XorMask	= 0/*LogicL*/;
			break;
	/*----------------------------------------------------------------------------------------------*/
	case OUTPUT_MAP3_LGC_OPPOSITE:									/* 2: Opposite					*/
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = (ULONG)Pin;
			IoSignal->Mask		= 1;
			IoSignal->XorMask	= 1/*LogicH*/;
			break;
	/*----------------------------------------------------------------------------------------------*/
	default:
	/*----------------------------------------------------------------------------------------------*/
			IoSignal->PinNumber = 31;								/* Interim */
			IoSignal->Mask		= 0;
			IoSignal->XorMask	= 0;
			break;
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}

/****************************************************************************************************/
/*																									*/
/*		Sequence output map calculation <S045>																*/
/*																									*/
/****************************************************************************************************/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	SGD7W IOI/O support<S118>	*/
void	LpxPcalSetSoutMap2( AXIS_HANDLE *Axis, SEQ_IOSIGNAL *IoSignal, ULONG prm, ULONG OppositeReq )
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
void	LpxPcalSetSoutMap( SEQ_IOSIGNAL *IoSignal, ULONG prm, ULONG OppositeReq )
#endif
{
/*--------------------------------------------------------------------------------------------------*/
	IoSignal->Mask		= 1;
//	IoSignal->XorMask   = 0; /* <S04B> */
/*--------------------------------------------------------------------------------------------------*/
#if CSW_BOARD_TYPE == SGD7W_TYPE	/*	SGD7W(2Axis compatible) only	SGD7W IOI/O support<S118>	*/
		switch( prm & 0x0F )
		{
		/*--------------------------------------------------------------------------------------------------*/
			case 1:													/* Output to SO1 */
		/*--------------------------------------------------------------------------------------------------*/
				if(Axis->AxisNo == 0 )	/*	1Axis number	*/
				{	/*	1Axis number	*/
					IoSignal->PinNumber = KPI_SO1BITNO;		/*	CN1-23,24	*/
					//IoSignal->XorMask = ((OppositeReq >> 4) & 0x01) ? 1 : 0;	/* <S1DA> */
					IoSignal->XorMask = ((OppositeReq >> 0) & 0x01) ? 1 : 0;	/* <S1DA> */
				}else
				{	/*	2Axis number	*/
					IoSignal->PinNumber = KPI_SO2BITNO;		/*	CN1-25,26	*/
					//IoSignal->XorMask = ((OppositeReq >> 8) & 0x01) ? 1 : 0;	/* <S1DA> */
					IoSignal->XorMask = ((OppositeReq >> 0) & 0x01) ? 1 : 0;	/* <S1DA> */
				}
				break;
			case 2:													/* Output to SO2 */
		/*--------------------------------------------------------------------------------------------------*/
				if(Axis->AxisNo == 0 )	/*	1Axis number	*/
				{	/*	1Axis number	*/
					IoSignal->PinNumber = KPI_SO3BITNO;		/*	CN1-27,28	*/
					//IoSignal->XorMask = ((OppositeReq >> 12) & 0x01) ? 1 : 0;	/* <S1DA> */
					IoSignal->XorMask = ((OppositeReq >> 4) & 0x01) ? 1 : 0;	/* <S1DA> */
				}else
				{	/*	2Axis number	*/
					IoSignal->PinNumber = KPI_SO4BITNO;		/*	CN1-29,30	*/
					//IoSignal->XorMask = ((OppositeReq >> 16) & 0x01) ? 1 : 0;	/* <S1DA> */
					IoSignal->XorMask = ((OppositeReq >> 4) & 0x01) ? 1 : 0;	/* <S1DA> */
				}
				break;
		/*--------------------------------------------------------------------------------------------------*/
			default:												/* Always inactive */
		/*--------------------------------------------------------------------------------------------------*/
				IoSignal->Mask		= 0;
				IoSignal->XorMask   = 0;
				IoSignal->PinNumber = 31;			/* Selects a reserved pin */
				break;
		}
#elif CSW_BOARD_TYPE == SGD7S_TYPE	/*	SGD7S(1Axis compatible) only	*/
	switch( prm & 0x0F )
	{
/*--------------------------------------------------------------------------------------------------*/
	case 1:													/* Output to SO1 */
/*--------------------------------------------------------------------------------------------------*/
		IoSignal->PinNumber = KPI_SO1BITNO;
		IoSignal->XorMask = ((OppositeReq >> 0) & 0x01) ? 1 : 0; /* <S04B> */
		break;
	case 2:													/* Output to SO2 */
/*--------------------------------------------------------------------------------------------------*/
		IoSignal->PinNumber = KPI_SO2BITNO;
		IoSignal->XorMask = ((OppositeReq >> 4) & 0x01) ? 1 : 0; /* <S04B> */
		break;
/*--------------------------------------------------------------------------------------------------*/
	case 3:													/* Output to SO3 */
/*--------------------------------------------------------------------------------------------------*/
		IoSignal->PinNumber = KPI_SO3BITNO;
		IoSignal->XorMask = ((OppositeReq >> 8) & 0x01) ? 1 : 0; /* <S04B> */
		break;

#if CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE /* <S04B> */
/* <S1AA> : SO4~SO6Eliminate output signal inversion of */
/*--------------------------------------------------------------------------------------------------*/
	case 4:													/* Output to SO4 *//* <S04B> */
/*--------------------------------------------------------------------------------------------------*/
		IoSignal->PinNumber = KPI_SO4BITNO;
		//IoSignal->XorMask = ((OppositeReq >> 12) & 0x01) ? 0 : 1; /* <S04B> */	/* <S1AA> */
		IoSignal->XorMask = ((OppositeReq >> 12) & 0x01) ? 1 : 0;					/* <S1AA> */

		break;
/*--------------------------------------------------------------------------------------------------*/
	case 5:													/* Output to SO5 *//* <S04B> */
/*--------------------------------------------------------------------------------------------------*/
		IoSignal->PinNumber = KPI_SO5BITNO;
		//IoSignal->XorMask = ((OppositeReq >> 16) & 0x01) ? 0 : 1; /* <S04B> */	/* <S1AA> */
		IoSignal->XorMask = ((OppositeReq >> 16) & 0x01) ? 1 : 0;					/* <S1AA> */

		break;
/*--------------------------------------------------------------------------------------------------*/
	case 6:													/* Output to SO6 *//* <S04B> */
/*--------------------------------------------------------------------------------------------------*/
		IoSignal->PinNumber = KPI_SO6BITNO;
		//IoSignal->XorMask = ((OppositeReq >> 20) & 0x01) ? 0 : 1; /* <S04B> */	/* <S1AA> */
		IoSignal->XorMask = ((OppositeReq >> 20) & 0x01) ? 1 : 0;					/* <S1AA> */
		break;
#endif /* CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE */

/*--------------------------------------------------------------------------------------------------*/
	default:												/* Always inactive */
/*--------------------------------------------------------------------------------------------------*/
		IoSignal->Mask		= 0;
		IoSignal->XorMask   = 0;
		IoSignal->PinNumber = 31;			/* Selects a reserved pin */
		break;
	}
#endif
/*--------------------------------------------------------------------------------------------------*/
}

/****************************************************************************************************/
/*																									*/
/*		Moving average initialization processing for motor speed detection	<V259>													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Parameter settings(Pn009.2), and the resolution of the connected encoder to control the initial value of the number of moving averages	*/
/*			Set. However, if no adjustment is enabled, set the moving average number0times.				*/
/*			You can change the moving average number online.BpiChangeSpdDetMaFil( ).				*/
/*																									*/
/*			<Rotary type>							Number of moving averages										*/
/*			Encoder resolution	Speed resolution		Normal(Pn009.2=0)	(Pn009.2=1)	Magnetic pole detection	No adjustment	*/
/*			  13bit			117.12min-1or more		4times				16times	  16times		  0times		*/
/*			  16bit				 14.64min-1			2times				16times	  16times		  0times		*/
/*			  17bitor more			  7.32min-1		0times				16times	  16times		  0times		*/
/*																									*/
/*			<Linear>							Number of moving averages										*/
/*			Scale pitch		Speed resolution		Normal(Pn009.2=0)	(Pn009.2=1)	Magnetic pole detection	No adjustment	*/
/*			  200umor more			 12.50mm/sor more		4times				16times	  16times		  0times		*/
/*			  100um				  6.25mm/s			2times				16times	  16times		  0times		*/
/*			  100um			  6.25mm/s		0times				16times	  16times		  0times		*/
/*																									*/
/*			※Speed resolution(1Speed per pulse), set the value of sampling period62.5usIn the case of						*/
/*			※For linear motor,256Calculate by multiplying													*/
/*																									*/
/****************************************************************************************************/
#define	SPDDET13BIT		KPI_SACOUNTMS*600000/8192			/* 13bitEncoder speed detection resolution		*/
#define	SPDDET16BIT		KPI_SACOUNTMS*600000/65536			/* 16bitEncoder speed detection resolution		*/
#define	SPDDET100UM		KPI_SACOUNTMS*10000/256*100/1000	/* Scale pitch100umSpeed detection resolution	*/
#define	SPDDET200UM		KPI_SACOUNTMS*10000/256*200/1000	/* Scale pitch200umSpeed detection resolution	*/
/*--------------------------------------------------------------------------------------------------*/
static void	LpxPcalSpdDetMaFilter( AXIS_HANDLE *Axis )
{
	BPRMDAT			*Bprm;

	Bprm	= Axis->UniPrms->Bprm;
	MlibResetLongMemory( Axis->MotSpdMafil, sizeof( *Axis->MotSpdMafil )/4 );
/*--------------------------------------------------------------------------------------------------*/
/*		Set the initial value of the moving average number																	*/
/*--------------------------------------------------------------------------------------------------*/
	if ( Bprm->SpdDetMaFil == TRUE )
	{ /* Pn009.2 = 1In the case of16times */
		Axis->MotSpdMafil->defaultNumBit = 4;
	}
	else if( Axis->MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* In the case of linear */
		if( Bprm->SpdDetectUnit >= SPDDET200UM )
		{ /* 200umor more is4times */
			Axis->MotSpdMafil->defaultNumBit = 2;
		}
		else if( Bprm->SpdDetectUnit >= SPDDET100UM )
		{ /* 100um~200umteeth2times */
			Axis->MotSpdMafil->defaultNumBit = 1;
		}
		else
		{ /* 100umless than0times */
			Axis->MotSpdMafil->defaultNumBit = 0;
		}
	}
	else
	{ /* For rotary type */
		if( Bprm->SpdDetectUnit >= SPDDET13BIT )
		{ /* 13bitThe following are4times */
			Axis->MotSpdMafil->defaultNumBit = 2;
		}
		else if( Bprm->SpdDetectUnit >= SPDDET16BIT )
		{ /* 14~16bitteeth2times */
			Axis->MotSpdMafil->defaultNumBit = 1;
		}
		else
		{ /* 17bitor more is0times */
			Axis->MotSpdMafil->defaultNumBit = 0;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Set the moving average number																			*/
/*--------------------------------------------------------------------------------------------------*/
	Axis->MotSpdMafil->manumBit = Axis->MotSpdMafil->defaultNumBit;
	if ( Axis->BaseControls->TuneLessCtrl.conf.TuningLessUse == TRUE )
	{ /* When no adjustment is required0Fixed */
		Axis->MotSpdMafil->manumBit = 0;
	}
	Axis->MotSpdMafil->idx = 0;
	Axis->MotSpdMafil->ksub = 0;
	Axis->MotSpdMafil->mabufSumx = 0;
/*--------------------------------------------------------------------------------------------------*/

}



/****************************************************************************************************/
#define SVCDEF_PCMP_DLYUS			( 80 )		/* Phase delay compensation delay time(Encoder:50us+ScanA:30us)	*/
#define SVCDEF_PCMP_DLYCX			( 16 )				/* Phase lag compensation correction coefficient [-]					*/
/****************************************************************************************************/
/*																									*/
/*		Calculation of phase compensation calculation coefficient																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Delay time(Encoder, etc.).							*/
/*			Calculate the phase velocity compensation calculation coefficient from the phase compensation by the parameter.							*/
/*																									*/
/*	Supplementary Foot : If the SERVOPACK parameter is 0, use the motor parameter for phase compensation calculation.				*/
/*			(In case the motor side parameters are set to F, use the SERVOPACK parameters)				*/
/*			If SERVOPACK parameters are other than 0, use SERVOPACK parameters.					*/
/*																									*/
/*	Note Intention : Servopack parameters cannot be used unless they are 0.								*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	void	LpxPcalPcmpCalGain( AXIS_HANDLE	*Axis )
{
#if (FLOAT_USE==TRUE)
	float			fx;
#else
	LONG			kx;
	LONG			sx;
#endif /* FLOAT_USE */
	LONG			PhsCmpDegx;
	BPRMDAT			*Bprm;
	PRMDATA			*Prm;

	Bprm	= Axis->UniPrms->Bprm;
	Prm		= Axis->UniPrms->Prm;

/*--------------------------------------------------------------------------------------------------*/
/*		Phase compensation parameter calculation: Delay compensation															*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*				         80 * nos * Pole * 65536       PhsCmp * 65536 * (PerOvrSpd+10000)			*/
/*		SGDS  Leang = -----------------------------	+ ------------------------------------			*/
/*		Rotary	       10^6 * 60 * 2 * 15000 * Cx             360 * 10000 * 15000	       			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					      Dx * OvrSpd * Pole * 65536		  Dx * OvrSpd * Pole					*/
/*		SVOS  Kpcmpd = --------------------------------	= ---------------------------				*/
/*		Rotary		    10^6 * 2 * PAI * 2 * 2^24 * Cx		10^6 * 3216.990877 * Cx					*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		PerOvrSpd : OS detection level [0.01%]															*/
/*		Pole      : Number of motor poles																	*/
/*		nos       : OS speed [r/min],[mm/s]															*/
/*		OvrSpd    : OS speed [rad/s],[m/s]															*/
/*		Dx        : Delay time [us] --> Kernel configuration definition(*.cfg)									*/
/*		Cx        : Bug correction [--] --> Kernel configuration definition(*.cfg)									*/
/*		Caution(Cx=16) : If the motor meets these conditions, the bug cannot be fixed, and there is no bug in linear.			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	if( Axis->MencV->AxisMotType == MOTTYPE_ROTARY )
	{
		if( ((Prm->dq_sw & 0x0800) == 0 ) || ((Prm->MencP.flg_wf & 0x0001 ) == 0) )
		{
#if (FLOAT_USE==TRUE)
			fx = Bprm->OvrSpd * (LONG)SVCDEF_PCMP_DLYUS / 321699088.0f;
			Axis->BaseControls->PcmpCalcData.Kpcmpd = fx * (float)Prm->MencP.oslv_pol.b.h / (10.0f * (LONG)SVCDEF_PCMP_DLYCX);
#else
			kx = MlibScalKskxkx( Bprm->OvrSpd, (LONG)SVCDEF_PCMP_DLYUS, 321699088, &sx, 0 );
			Axis->BaseControls->PcmpCalcData.Kpcmpd =
					MlibPcalKxgain( kx, (LONG)Prm->MencP.oslv_pol.b.h, (10 * (LONG)SVCDEF_PCMP_DLYCX), &sx, 24 );
#endif /* FLOAT_USE */

		}
		else
		{
#if (FLOAT_USE==TRUE)
			fx = Bprm->OvrSpd * (float)SVCDEF_PCMP_DLYUS / 321699088.0f;
			Axis->BaseControls->PcmpCalcData.Kpcmpd = fx * (float)Prm->MencP.oslv_pol.b.h / 10.0f;
#else
			kx = MlibScalKskxkx( Bprm->OvrSpd, (LONG)SVCDEF_PCMP_DLYUS, 321699088, &sx, 0 );
			Axis->BaseControls->PcmpCalcData.Kpcmpd =
											MlibPcalKxgain( kx, (LONG)Prm->MencP.oslv_pol.b.h, 10, &sx, 24 );
#endif /* FLOAT_USE */
		}
	}
	else if( Axis->MencV->AxisMotType == MOTTYPE_LINEAR )
	{
#if (FLOAT_USE==TRUE)
		fx = Bprm->OvrSpd * (LONG)SVCDEF_PCMP_DLYUS / 51200.0f;
		Axis->BaseControls->PcmpCalcData.Kpcmpd = fx / Prm->MencP.ppitch;
		Axis->BaseControls->PcmpCalcData.Kpcmpd2 = Axis->BaseControls->PcmpCalcData.Kpcmpd;
#else
		kx = MlibScalKskxkx( Bprm->OvrSpd, (LONG)SVCDEF_PCMP_DLYUS, 51200, &sx, 0 );
		Axis->BaseControls->PcmpCalcData.Kpcmpd = MlibPcalKxgain( kx, 1, Prm->MencP.ppitch, &sx, 24 );
		Axis->BaseControls->PcmpCalcData.Kpcmpd2 = Axis->BaseControls->PcmpCalcData.Kpcmpd;
#endif /* FLOAT_USE */
	}

	/* Phase interpolation amount operation coefficient calculation */
#if (FLOAT_USE==TRUE) /* SVN_IRIS_MODE || INGRAM_MODE */
		fx = Bprm->OvrSpd * KPI_MACYCLENS / 160849544.0f;
		Axis->BaseControls->PcmpCalcData.KpcmpdItplt = fx * Bprm->Keangle / 10000.0f;
#else
	kx = MlibScalKskxkx( Bprm->OvrSpd, KPI_MACYCLENS, 160849544, &sx, 0 );
	Axis->BaseControls->PcmpCalcData.KpcmpdItplt =
							MlibPcalKxkskx( kx, Bprm->Keangle, 10000, &sx, 24 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Phase compensation parameter calculation: Phase compensation 1															*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		PcmpSpd1 = NorMaxSpd * PhsCmpSpd1 / MotMaxSpd		Phase compensation start speed						*/
/*																									*/
/*						  PhsCmpDeg1 * 65536														*/
/*		Kpcmpsp1 = --------------------------------			Phase compensation calculation gain						*/
/*					 360 * (NorMaxSpd - PcmpSpd1)													*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		PhsCmpDeg1 : Phase compensation value 1			[deg/MaxSpd]											*/
/*		PhsCmpSpd1 : Phase compensation start speed 1		[100r/min]												*/
/*		MotMaxSpd  : Motor maximum speed			[100r/min]												*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	if( Prm->phscmp != 0 )							/* SERVOPACK PARAMETERS != 0				*/
	{
		PhsCmpDegx = (LONG)Prm->phscmp;				/* Using SERVOPACK parameters				*/
	}
	else if( Prm->MencP.phscmpM1.b.l == 0xFF )			/* If the motor parameters are F-packed			*/
	{
		PhsCmpDegx = (LONG)Prm->phscmp;				/* Using SERVOPACK parameters				*/
	}
	else											/* If the motor parameters are not F-packed		*/
	{
		PhsCmpDegx = (LONG)Prm->MencP.phscmpM1.b.l;		/* Using motor parameters						*/
	}
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fx = (KSGAIN)Bprm->NorMaxSpd * (KSGAIN)Prm->MencP.phscmpM1.b.h / (KSGAIN)Bprm->MaxVel;
	Axis->BaseControls->PcmpCalcData.PcmpSpd1 = fx;
#else
	kx = MlibScalKxgain( Bprm->NorMaxSpd, Prm->MencP.phscmpM1.b.h, Bprm->MaxVel, NULL, -30 );
	Axis->BaseControls->PcmpCalcData.PcmpSpd1 = kx;
#endif /* FLOAT_USE */

	/*----------------------------------------------------------------------------------------------*/
	if( Prm->MencP.phscmpM1.b.h >= Bprm->MaxVel )		/* Starting speed check						*/
	{
		Axis->BaseControls->PcmpCalcData.Kpcmpsp1 = 0;
		/* A.040 : Parameter setting error */
		ALMSetPramError( Axis->AlmManager, pndef_phscmpM1.RegNumber );
	}
	else
	{
#if (FLOAT_USE==TRUE)
		fx = (KSGAIN)(Bprm->NorMaxSpd - Axis->BaseControls->PcmpCalcData.PcmpSpd1) * 360.0f;
		Axis->BaseControls->PcmpCalcData.Kpcmpsp1 = (KSGAIN)PhsCmpDegx * 65536.0f / fx;
#else
		kx = MlibScalKxgain( PhsCmpDegx, 65536, 360, &sx,  0 );
		kx = MlibPcalKxgain( kx, 1, (Bprm->NorMaxSpd - Axis->BaseControls->PcmpCalcData.PcmpSpd1), &sx, 24 );
		Axis->BaseControls->PcmpCalcData.Kpcmpsp1 = kx;
#endif /* FLOAT_USE */
	}

#ifdef CSW_SCANA_ENABLE
	Axis->BaseLoops->PhaseComp.conf.Kpcmpd = Axis->BaseControls->PcmpCalcData.Kpcmpd;
	Axis->BaseLoops->PhaseComp.conf.Kpcmpd2 = Axis->BaseControls->PcmpCalcData.Kpcmpd2;
	Axis->BaseLoops->PhaseComp.conf.Kpcmpsp1 = Axis->BaseControls->PcmpCalcData.Kpcmpsp1;
	Axis->BaseLoops->PhaseComp.conf.PcmpSpd1 = Axis->BaseControls->PcmpCalcData.PcmpSpd1;
	Axis->BaseLoops->PhaseComp.conf.KpcmpdItplt = Axis->BaseControls->PcmpCalcData.KpcmpdItplt;
#endif /* CSW_SCANA_ENABLE */

/*--------------------------------------------------------------------------------------------------*/
/*	Phase compensation 2 is deleted because its function is unused and to shorten the initial processing time.								*/
/*--------------------------------------------------------------------------------------------------*/

}


/****************************************************************************************************/
/*																									*/
/*		 Motor quiet speed FBLPFInitial setting <S0DA>														*/
/*																									*/
/*		Overview: Adding a low-pass filter to the speed FB using encoder parameters						*/
/*				Actual processing isPn308: SpeedFBAdd motor quiet filter time constant to filter time constant		*/
/*																									*/
/****************************************************************************************************/
static void IprmcalSpdFbFil( AXIS_HANDLE *Axis )
{
	PRMDATA		*Prm;
	BPRMDAT		*Bprm;
	MENCV		*MencV;

	Prm   = Axis->UniPrms->Prm;
	Bprm  = Axis->UniPrms->Bprm;
	MencV = Axis->MencV;

	if( ( Prm->MencP.flg_wf & 0x0004 )								/* Motor quiet setting selection flag		*/
			|| (MencV->TuningLessLowGainMotor == TRUE))				/* Motor model determination				*/
	{
		Bprm->SpdFbLpassFil = Prm->fbfil2;							/* SpeedFBAdd filter				*/
	}
	else
	{
		Bprm->SpdFbLpassFil = 0;									/* SpeedFBFilter0.1[ms]addition		*/
	}

	return;

}

/*<S1D0> START*/
/*****************************************************************************
  Description: Motor maximum speed processing
------------------------------------------------------------------------------
  Parameters:
	CfgPrm              cfgParameter definition
	Bprm				Basic parameter definition
  Return:

------------------------------------------------------------------------------
  Note:

*****************************************************************************/
static void	AdetCheckMotorOverSpeed(PRMDATA *Prm, BPRMDAT *Bprm, LONG AxisMotType )
{
	if( AxisMotType == MOTTYPE_LINEAR )
	{/* Linear type */
		Bprm->OsAlmLvl = Bprm->NorOvrSpd;
	}
	else
	{/* Rotation type */
		Bprm->OsAlmLvl = MlibDiv6432( Prm->maxvel, Bprm->NorOvrSpd, ( Bprm->MaxVel * 110 ) );
	}
}
/*<S1D0> END*/

/* <S219> Start */
/****************************************************************************************************/
/*																									*/
/*		Setting of built-in fan stop alarm detection time when servo is on											*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Time after fan stop warning occurs(Transition time to alarm)					*/
/*																									*/
/*			1. Parameter																			*/
/*				1) PnEE6 : Built-in fan stop alarm detection time when servo is on [min]							*/
/*																									*/
/****************************************************************************************************/
static	void	IprmcalFanLockAlmDetectTime( AXIS_HANDLE *Axis )
{
	CMN_PRMDATA		*Cprm;
	POW_MNG_HNDL	*PowerManager;

	Cprm = Axis->UniPrms->Cprm;
	PowerManager = Axis->PowerManager;

	/* Acquisition of built-in fan stop alarm detection time when servo is on(unit:[min]) */
	PowerManager->FanLock.conf.FanLockAlmDetectTime = (ULONG)(Cprm->SvonFanLock_Time * 60 * 10);
	return;
}
/* <S219> End */

/***************************************** end of file **********************************************/

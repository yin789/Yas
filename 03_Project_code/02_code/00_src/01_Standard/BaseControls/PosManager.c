/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PosManager.c : Position management related processing definition															*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Position management related processing																		*/
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
/*				<S00C> Source code organization																*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "SerialEncIf.h"			/* <S053> */
#include "BaseControls.h"
#include "PosManager.h"
#include "ModelControl.h"

/* 2012.08.18 Y.Oka Make Mercury Base Software */
#include "SerialEncIf.h"
/* 2012.08.18 Y.Oka Make Mercury Base Software */

#include "KnlApi.h"	/* @@ Erase after */

/****************************************************************************************************/
/*		for Position Reference Filter																*/
/****************************************************************************************************/
//#define	AVFILCALCYCLE	KPI_SBCYCLENS				/* Vibration suppression filter calculation cycle time(ScanB)	[ns]	*/
//#define	MAXOMEGA2(AvfCycleNs)		(127323954/AvfCycleNs)	/* 127323954 = 10^9 * 0.8 / (2*PAI)	Frequency2Maximum value[Hz]	*//* <S065> */

/****************************************************************************************************/
/*		for Position Reference Filter																*/
/****************************************************************************************************/
#define MAFIL_BUFNUM	256									/* Number of moving average filter buffers			*/


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void	PosMngInitPositionManager(BASE_CTRL_HNDL *BaseControls );									*/
/*	LONG	PosMngNetPosCmdManager( BASE_LOOPCTRLS *BaseLoops, SERVO_CONTROL_IF *SvControlIf );		*/
/*	void	PosMngSenOnReCalcAbsoPos( BASE_CTRL_HNDL *BaseControls, MENCV *MencV,					*/
/*																	UNI_PRM_STR *uni_prm_ptr );		*/
/*	void	PosMngLatchNetFbkPos( BASE_CTRL_HNDL *BaseControls,										*/
/*												 SERVO_CONTROL_IF *SvControlIf, BPRMDAT *Bprm  );	*/
/*	void	StlgInitSettlingTime( SETTLINGTIME *SettlingTime, LONG CycleTimeUs );					*/
/*	void	StlgCalculateSettlingTime( SETTLINGTIME *SettlingTime, LONG dPcmda, BOOL CoinSts );		*/
/*	void	StlgMakeMaxOverShoot( SETTLINGTIME *SettlingTime, POS_MNG_HNDL *PosManager,				*/
/*																			BOOL PosCtrlMode );		*/
/*	void	StlgMakeCoinOffTime( SETTLINGTIME *SettlingTime, LONG dPcmda,							*/
/*																BOOL CoinSts, BOOL PosCtrlMode );	*/
/****************************************************************************************************/
/*	void	BpiEgearFuncSwitch( POS_MNG_HNDL *PosManager, BOOL Switch );							*/
/*	void	BpiPcmdFilSwitch( POS_MNG_HNDL *PosManager, BOOL Switch );								*/
/****************************************************************************************************/
/*	void	BpiRsetMaxOverShoot( SETTLINGTIME *SettlingTime );										*/
/*	void	BpiRsetCoinOffTime( SETTLINGTIME *SettlingTime );										*/
/****************************************************************************************************/
//#if ( CSW_SVCF_RESVIB_FIL_USE == TRUE ) /* <S065> */
//static	void	posMngInitPcmdFilter( VIBSUPFIL *VibSupFil );
//static	LONG	posMngVibSupFilter( VIBSUPFIL *VibSupFil, LONG dPcmd, BOOL *RefZSignal );
//#endif
static	void	posMngCtrlPosSignal( POS_MNG_HNDL *PosManager, POS_CTRL_STS *PosCtrlSts
									, PCMDFIL *pPcmdFil /* <S081> */);

//Currently not supported	static	void	BpxMakeMotABCphaseSignal( BASE_CTRL_HNDL *BaseControls );
/****************************************************************************************************/
static void	BpxPerClrSignalProcedure( BASE_LOOPCTRLS *BaseLoops, BPRMDAT *Bprm );/* @@ Position deviation clear signal processing */
static void	BpxEdgeClrSvcPositionManager( BASE_LOOPCTRLS *BaseLoops ); 	/* @@ Servo control position deviation edge clear variable initialization	*/
/****************************************************************************************************/




/****************************************************************************************************/
/*																									*/
/*		Servo control position management variable initialization																*/
/*																									*/
/****************************************************************************************************/
void	PosMngInitPositionManager( BASE_CTRL_HNDL *BaseControls )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Servo control position management variable initialization																*/
/*--------------------------------------------------------------------------------------------------*/
	/* Position management variable initialization */
	MlibResetLongMemory( &(BaseControls->PosManager.var), sizeof(BaseControls->PosManager.var)/4 );
/*--------------------------------------------------------------------------------------------------*/
//	MlibResetLongMemory( &(BaseControls->PosManager.PosErrA), sizeof(PERRB)/4 );	/* @@ CLRSignal */ /* <S107> */
/*--------------------------------------------------------------------------------------------------*/
	BaseControls->PosManager.Egear = &(BaseControls->PosManager.conf.Egear);

/* 2012.09.07 Y.Oka AmonCurrently not supported */
//	BoutV.AmondPosCmd  = 0;								/* Position command monitor variable						*/
/* 2012.09.07 Y.Oka ★ScanAare initialized with two, so they are not initialized here.★ */
	BaseControls->PosManager.dPosRefi = 0;				/* Position command difference input[Pulse/Scan]		*//* <S187> */
	BaseControls->PosManager.CompdPosRefi = 0;			/* Position command difference input(ScanB->ScanA)			*/
	BaseControls->PosManager.CompdPcmda = 0;			/* Position command incremental value input(ScanB->ScanA)			*/

	/* ★SGDV. If unnecessary, delete★ */
	if( BaseControls->CtrlModeSet.CtrlMode.b.cm == CTRLMODE_POS )
	{
		BaseControls->PosCtrlSts.CoinSignal = TRUE;
		BaseControls->PosCtrlSts.NearSignal = TRUE;
	}
	else
	{
		BaseControls->PosCtrlSts.CoinSignal = FALSE;
		BaseControls->PosCtrlSts.NearSignal = FALSE;
	}

	/* Position command filter initialization processing */
//	posMngInitPcmdFilter( &(BaseControls->VibSupFil) );												  /* <S065> */
	PcmdFilInitialize( &(BaseControls->PcmdFil), &(BaseControls->VibSupFil) );
	/* MFCInitializing variables */
	MfcInitModelControl( &(BaseControls->MFControl) );

#if (CSW_SVCF_BACKLASHCOMP == TRUE)
		/* Backlash correction disabled */
		BlshInvaildCompensate( &(BaseControls->PosManager.BlshCmp) );	/* <S18B> */
#endif

	BaseControls->CtrlCmdMngr.dPFfFilo = 0;
	BaseControls->CtrlCmdMngr.PicvClrCmd = FALSE;
}

/****************************************************************************************************/
/*																									*/
/*		Servo communication position command management																		*/
/*																									*/
/*			※APUse this also for type																	*/
/*																									*/
/****************************************************************************************************/
LONG	PosMngNetPosCmdManager( BASE_LOOPCTRLS *BaseLoops, SERVO_CONTROL_IF *SvControlIf,
															FUN_CMN_CONTROL *FnCmnCtrl, BPRMDAT *Bprm )/* <S04D> */
{
	LONG			dPosRefo;
	BASE_CTRL_HNDL	*BaseControls;
	POS_MNG_HNDL	*PosManager;
	MFCTRL			*MFControl;
	BOOL 			den;
#if ( CSW_SVCF_BACKLASHCOMP )
	BOOL			BlshActive;
#endif

	BaseControls = BaseLoops->BaseCtrls;	
	PosManager	= &(BaseControls->PosManager);
	MFControl	= &(BaseControls->MFControl);

/*--------------------------------------------------------------------------------------------------*/
/*		Position command input processing(Communication,PJOG,etc):PosManager->var.dPcmda	[Command unit/Scan]						*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseControls->TblDrive.var.TblCmd != TBLCMD_NOCMD )
	{ /* During table operation */
		BaseControls->CtrlCmdMngr.PicvClrCmd = FALSE;			/* Position control integral clear command reset		*/
	/*----------------------------------------------------------------------------------------------*/
		if( BaseControls->BeComplete == TRUE )
		{
			/* Table operation position command */
			dPosRefo = BaseControls->TblDrive.var.dPosRef;
			/* Position deviation clear request cancellation */
			BaseControls->CtrlCmdMngr.PerrClrReq = FALSE;
/* <S212> */
			/* Position command ring counter update */
			BaseControls->CtrlCmdMngr.PosRef += dPosRefo;
			/* Updating division position command */
			PosManager->var.dPcmda = dPosRefo;
//			BaseControls->AmondPosCmd = dPosRefo;		/* <S226> */
/* <S212> */
		}
		else
		{
			/* Servo control position management variable initialization */
			PosMngInitPositionManager( BaseControls );
			/* Position deviation clear request */
			BaseControls->CtrlCmdMngr.PerrClrReq = TRUE;
			/* Position control end */
			return( 0 );
		}
	}
	else if( BaseControls->CtrlModeSet.CtrlMode.dw == CTRLMODE_PJOG )
	{ /* ProgramJOGIn mode */
		BaseControls->CtrlCmdMngr.PicvClrCmd = FALSE;			/* Position control integral clear command reset		*/
		BaseControls->PosCtrlSts.DenSignal   = FALSE;			/* Den Signal							*/

		if( BaseControls->BeComplete == TRUE )
		{ /* Position control execution */
			/* Position command creation processing	[Command unit/Scan]	*/
			den = PrgJogMakePositionReference( &BaseControls->PJogHdl, &dPosRefo );

			/* Den Signal */
			BaseControls->PosCtrlSts.DenSignal  = den;
			BaseControls->PosCtrlSts.RefZSignal = den; /* <S066> */

			/* Position deviation clear request */
			BaseControls->CtrlCmdMngr.PerrClrReq = FALSE;

			/* Position command ring counter update */
			BaseControls->CtrlCmdMngr.PosRef += dPosRefo;	/* <S1BA> */
			/* Updating division position command */
			PosManager->var.dPcmda = dPosRefo;				/* <S1BA> */
//			BaseControls->AmondPosCmd = dPosRefo;			/* <S1BA> */	/* <S226> */
		}
		else
		{ /* Position control end */
			/* Servo control position management variable initialization */
			PosMngInitPositionManager( BaseControls );
			/* Position deviation clear request */
			BaseControls->CtrlCmdMngr.PerrClrReq = TRUE;
			return( 0 );							
		}
	}
	else
	{ /* During normal position control */
	/*----------------------------------------------------------------------------------------------*/
	/*	@@ Deviation clear processing																				*/
	/*----------------------------------------------------------------------------------------------*/
/* <S1B6> */
//		if( ((SvControlIf->CmdCtrl.CmdCtrlBit) >> CLRPOSERR_BITNO) & 1 )
//		{
//			PosMngInitPositionManager( BaseControls ); 		/* Servo control position management variable initialization			*/
//			BaseControls->CtrlCmdMngr.PerClrEdge = FALSE; 	/* Position deviation clear edge signal reset		*/
//			BaseControls->CtrlCmdMngr.PerrClrReq = TRUE; 	/* Position deviation clear request 					*/
//			return( 0 );									/* Position control end							*/
//		}
	/*----------------------------------------------------------------------------------------------*/
#if (CSW_CMDRENEW_PCMD_FIL_USE == TRUE) /* <S065> */
		/* Position command moving average processing(ScanBperiod andScanNInterpolation processing when the periods are different) */
		dPosRefo = MlibPcmdMafil( (SvControlIf->NetPosRef - BaseControls->CtrlCmdMngr.PosRefLo),
								  SvControlIf->CmdOptionIf.CycCtrl.CmdUpdateCycle,
								  &(BaseControls->CtrlCmdMngr.Pmafvar),
								  &(BaseControls->CtrlCmdMngr.Pmafbuf[0]) );
#else
		dPosRefo = SvControlIf->NetPosRef - BaseControls->CtrlCmdMngr.PosRefLo;
#endif

		/* Position command ring counter update */
		BaseControls->CtrlCmdMngr.PosRef += dPosRefo;	/* <S1B6> */
		/* Updating division position command */
		PosManager->var.dPcmda = dPosRefo;				/* <S1B6> */
//		BaseControls->AmondPosCmd = dPosRefo;			/* <S1B6> */	/* <S226> */
		
/* <S1B6> */
		if( ((SvControlIf->CmdCtrl.CmdCtrlBit) >> CLRPOSERR_BITNO) & 1 )
		{
			PosMngInitPositionManager( BaseControls ); 		/* Servo control position management variable initialization			*/
			BaseControls->CtrlCmdMngr.PerClrEdge = FALSE; 	/* Position deviation clear edge signal reset		*/
			BaseControls->CtrlCmdMngr.PerrClrReq = TRUE; 	/* Position deviation clear request 					*/
			return( 0 );									/* Position control end							*/
		}

		/* Position deviation clear request cancellation */
		BaseControls->CtrlCmdMngr.PerrClrReq = FALSE;

	/*----------------------------------------------------------------------------------------------*/
	/*	@@ Position deviation edge clear processing																	*/
	/*----------------------------------------------------------------------------------------------*/
		if( BaseControls->PerClrEdge )
		{
			BpxPerClrSignalProcedure( BaseLoops, Bprm ); /* <S04D>:Argument addition */
			BaseControls->CtrlCmdMngr.PerClrEdge = TRUE; /* Position deviation clear edge signal set			*/
			dPosRefo = 0;
		}
		else
		{
			BaseControls->CtrlCmdMngr.PerClrEdge = FALSE; /* Position deviation clear edge signal reset		*/
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		@@ When the deviation is cleared, the following processing is not performed															*/
/*--------------------------------------------------------------------------------------------------*/

	/* Position command ring counter update */
//	BaseControls->CtrlCmdMngr.PosRef += dPosRefo;	/* <S1B6> */
	/* Updating division position command */
//	PosManager->var.dPcmda = dPosRefo;				/* <S1B6> */

/*--------------------------------------------------------------------------------------------------*/
/*		Position command electronic gear conversion:PosManager->var.dPcmdEgear [Pulse/Scan]								*/
/*--------------------------------------------------------------------------------------------------*/
	PosManager->var.dPcmdEgear = MlibPcmdEgear( dPosRefo,
												PosManager->Egear,
												&(PosManager->var.dPcmdEgrem) );

/*--------------------------------------------------------------------------------------------------*/
/*		Position command filter calculation(Exponential acceleration/deceleration,Moving average):BaseControls->CtrlCmdMngr.dPcmdFilo [Pulse/Scan]	*//* <S065> */
/*--------------------------------------------------------------------------------------------------*/
	/* Payout completion signal processing(Unfinished payout moduleFALSE,) */
//	BaseControls->PosCtrlSts.RefZSignal = ( PosManager->var.dPcmdEgear != 0 ) ? FALSE : TRUE;		  /* <S065> */
	PcmdFilSetRefZStatus( &(BaseControls->PcmdFil), TRUE );
	/* Position command filter processing */
	BaseControls->CtrlCmdMngr.dPcmdFilo = PcmdFilRuntimeService( &(BaseControls->PcmdFil),
																 PosManager->var.dPcmdEgear,
																 PosManager->var.PosErrA.per32s,
																 PosManager->var.PcmdFilStop );
	/* Getting the payout completion status */
	BaseControls->PosCtrlSts.DenSignal = PcmdFilGetRefZStatus( &(BaseControls->PcmdFil) );
	BaseControls->PosCtrlSts.RefZSignal = BaseControls->PosCtrlSts.DenSignal;

/*--------------------------------------------------------------------------------------------------*/
/*		Extend(FF)Control calculation(MFC,Sway prevention control)															*/
/*--------------------------------------------------------------------------------------------------*/
/*		BaseControls->CtrlCmdMngr.dPosRefo [Pulse/Scan]: Position control position command difference input					*/
/*--------------------------------------------------------------------------------------------------*/

	/*----------------------------------------------------------------------------------------------*/
	/*		Backlash correction processing																	*/
	/*----------------------------------------------------------------------------------------------*/
#if ( CSW_SVCF_BACKLASHCOMP )
	if( (BaseControls->BeComplete == FALSE)
		|| (BaseControls->MencV->PhaseReady == FALSE)		/* <S18B> */
		|| (BaseControls->OverTravel == TRUE) )
	{
		BlshActive = FALSE;
	}
	else
	{
		BlshActive = TRUE;
	}
	BlshCompensateBacklash( &(PosManager->BlshCmp),
							&(PosManager->conf.Egear),
//							&(PosManager->var.dPcmdEgear),											  /* <S065> */
							&(BaseControls->CtrlCmdMngr.dPcmdFilo),									  /* <S065> */
							&(PosManager->var.dPcmda),/* &(dPosRefo),*/			/* <S18B> */
							&(BaseControls->PosCtrlSts.RefZSignal),
							BlshActive );
#endif

	/*----------------------------------------------------------------------------------------------*/
	/*		Vibration suppression filter processing																	*/
	/*----------------------------------------------------------------------------------------------*/
#if ( CSW_SVCF_RESVIB_FIL_USE == TRUE )
//	if( PosManager->var.PcmdFilStop == FALSE ) /* <S065> */
	{ /* No request to disable position command filter */
		/* Vibration suppression filter forced invalidation process(Disabled for other than position control) */
		if( BaseControls->CtrlModeSet.CtrlModeLst.b.cm != CTRLMODE_POS )
		{
			BaseControls->VibSupFil.conf.Pexe.enable = 0;
		}
		/* Vibration suppression filter calculation */
//		BaseControls->CtrlCmdMngr.dPcmdFilo = posMngVibSupFilter(									  /* <S065> */
		BaseControls->CtrlCmdMngr.dPcmdFilo = PcmdFilVibSupFilter(
												&(BaseControls->VibSupFil),
//												PosManager->var.dPcmdEgear,							  /* <S065> */
												BaseControls->CtrlCmdMngr.dPcmdFilo,				  /* <S065> */
												&(BaseControls->PosCtrlSts.RefZSignal) );
	}
//	else /* <S065> */
//	{ /* Position command filter disable is requested */
//		BaseControls->CtrlCmdMngr.dPcmdFilo = PosManager->var.dPcmdEgear;
//	}
#else
//		BaseControls->CtrlCmdMngr.dPcmdFilo = PosManager->var.dPcmdEgear; /* <S065> */
#endif

	/*----------------------------------------------------------------------------------------------*/
	/*		MFCProcessing																					*/
	/*----------------------------------------------------------------------------------------------*/
#if ( CSW_SVCF_MFC_USE == TRUE )
	/* MFCForced invalidation process(Disabled for other than position control) */
	if( BaseControls->CtrlModeSet.CtrlModeLst.b.cm != CTRLMODE_POS )
	{ /* If previous control mode is not position control */
		/* MFCInitialize to disabled */
		MFControl->conf.MfcPexe.MFCModel = 0;
	}
	/* MFCOperation */
	dPosRefo = MfcControlMain( &(BaseControls->MFControl),
							   BaseControls->CtrlCmdMngr.dPcmdFilo,
							   &(BaseControls->CtrlCmdMngr.SpdFFCx),
							   &(BaseControls->CtrlCmdMngr.TrqFFCx) );
#else
	MFControl->conf.MfcPexe.MFCModel = 0;
	dPosRefo = BaseControls->CtrlCmdMngr.dPcmdFilo;
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Control position signal processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* Make Coin and Near Status */
	posMngCtrlPosSignal( PosManager, &(BaseControls->PosCtrlSts)
					,&(BaseControls->PcmdFil) /* <S081> */ );

	return (dPosRefo);
}

/****************************************************************************************************/
/*																									*/
/*		SEN ON																	*/
/*																									*/
/****************************************************************************************************/
void	PosMngSenOnReCalcAbsoScalePos(MENCV *MencV, APOSRG *EncPos, EGEAR *Egear, BOOL RvsDir)
{
	LONG	IniPos[2];
	UCHAR	Sftx;

	/* Local variable initialization */
	Sftx	= FSX_FPOS_SFT;

	if( MencV->AbsoEncType == MENC_INCRE_TYPE )
	{
		MlibResetLongMemory(EncPos, sizeof(APOSRG)/4);
	}
	else
	{
		IniPos[1] = (LONG)((SHORT)MencV->RxPosH[0] >> Sftx);		/* <S209> *//* <S211> */
		IniPos[0] = (LONG)( ((ULONG)MencV->RxPosH[0] << (32-Sftx))
						+ ((ULONG)MencV->RxPosL[0].dw >> Sftx) );	/* <S209> */

		if( MencV->RevAsmMotor ^ RvsDir )/* Inversion connection XOR Reverse rotation mode */
		{
			IniPos[1] = (~IniPos[1]);		/* Sign inversion processing	*/
			IniPos[0] = (~IniPos[0]) + 1;	/* Sign inversion processing */
			if( IniPos[0] == 0 )
			{	/* Carry, go to the top+1 */
				++IniPos[1];
			}
		}
		/*	Use this function even though it takes a long time.
		 * (ServoOFF)	*/
		MlibAposRg64iv( IniPos[0], IniPos[1], Egear, EncPos );
	}
}

/****************************************************************************************************/
/*																									*/
/*		SEN ON																	*/
/*																									*/
/****************************************************************************************************/
void	PosMngSenOnReCalcAbsoPos( MENCV *MencV, APOSRG *EncPos,
								  EGEAR *Egear, DBYTEX bit_dp, USHORT limmlt, BOOL RvsDir )
{
	SencMakeAbsPosition(MencV, limmlt, bit_dp, RvsDir);
	MlibAposRg64iv( MencV->SenOnScalePosL, MencV->SenOnScalePosH, Egear, EncPos );
}



/****************************************************************************************************/
/*																									*/
/*		Position FB latch processing																			*/
/*																									*/
/****************************************************************************************************/
void	PosMngLatchNetFbkPos( BASE_CTRL_HNDL *BaseControls, SERVO_CONTROL_IF *SvControlIf, BPRMDAT *Bprm )/* <S04D> */
{
	POS_MNG_HNDL	*PosManager;
	LONG			RspLtBit = 0;
	LONG			dPosCmp;
	LONG			motLpos, motPos;																/* <S004> */
	LONG			posSftR;																		/* <S04D> */
	LONG			rvsDir;																			/* <S221> */

	PosManager = &(BaseControls->PosManager);
	motPos = BaseControls->MencV->MotPosX[0];														/* <S007> */
	rvsDir = 1;																						/* <S221> */

	if(Bprm->FencUse != FALSE)/* <S04D>:Addition of Fluke processing */
	{/* Fluk	*/
		posSftR = BaseControls->FencV->MposSftR;
	}
	else
	{/* SemiC	*/
		posSftR = BaseControls->MencV->MposSftR;
		if(Bprm->RvsDir)/* <S221> */
		{
			rvsDir = -1;
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		C phase latch processing																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( SvControlIf->LtReqCphs != FALSE )
	{ /* Latch request present */
//		if(BaseControls->MotSts.MotCphPass != FALSE)/* <S04D> */
		if((Bprm->FencUse != FALSE)&&(BaseControls->MotSts.FencCphPass != FALSE)
			||(Bprm->FencUse == FALSE)&&(BaseControls->MotSts.MotCphPass != FALSE))/* <S04D>:Fluke support */
		{/* Latch completion */
			dPosCmp = BaseControls->CtrlCmdMngr.FbkPosFromCpos;

			/* Absolute position update calculation */
			MlibAposRg64dp( -dPosCmp,
							&PosManager->conf.Egear,
							&PosManager->PgPos,
							SvControlIf->MonCphaLpos );

			SvControlIf->LtReqCphs = FALSE;
			RspLtBit |= ((SvControlIf->LtReqCphs^1) * C_PHASE_COMP_BIT);
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Ext1Latch processing																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( SvControlIf->LtReqExt1 != FALSE )
	{ /* Latch request present */
		if((SvControlIf->ExtLtSts[0]).Complete != FALSE)
		{/* Latch completion */
			motLpos = (LONG)SvControlIf->ExtLtSts[0].LatchPosition << posSftR;	/* <S004> *//* <S04D> */
			dPosCmp = rvsDir * ((motPos - motLpos) >> posSftR);					/* <S004> *//* <S04D> *//* <S221> */
			/* Absolute position update calculation */
			MlibAposRg64dp( -dPosCmp,
							&PosManager->conf.Egear,
							&PosManager->PgPos,
							SvControlIf->MonExt1Lpos);

			SvControlIf->LtReqExt1 = FALSE;
			RspLtBit |= ((SvControlIf->LtReqExt1^1) * EXT_SIG1_COMP_BIT);
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Ext2Latch processing																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( SvControlIf->LtReqExt2 != FALSE )
	{ /* Latch request present */
		if((SvControlIf->ExtLtSts[1]).Complete != FALSE)
		{/* Latch completion */
			motLpos = (LONG)SvControlIf->ExtLtSts[1].LatchPosition << posSftR;	/* <S004> *//* <S04D> */
			dPosCmp = rvsDir * ((motPos - motLpos) >> posSftR);					/* <S004> *//* <S04D> *//* <S221> */
			/* Absolute position update calculation */
			MlibAposRg64dp( -dPosCmp,
							&PosManager->conf.Egear,
							&PosManager->PgPos,
							SvControlIf->MonExt2Lpos);

			SvControlIf->LtReqExt2 = FALSE;
			RspLtBit |= ((SvControlIf->LtReqExt2^1) * EXT_SIG2_COMP_BIT);
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Ext3Latch processing																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( SvControlIf->LtReqExt3 != FALSE )
	{ /* Latch request present */
		if(SvControlIf->ExtLtSts[2].Complete != FALSE)
		{/* Latch completion */
			motLpos = (LONG)SvControlIf->ExtLtSts[2].LatchPosition << posSftR;	/* <S004> *//* <S04D> */
			dPosCmp = rvsDir * ((motPos - motLpos) >> posSftR);					/* <S004> *//* <S04D> *//* <S221> */
			/* Absolute position update calculation */
			MlibAposRg64dp( -dPosCmp,
							&PosManager->conf.Egear,
							&PosManager->PgPos,
							SvControlIf->MonExt3Lpos);

			SvControlIf->LtReqExt3 = FALSE;
			RspLtBit |= ((SvControlIf->LtReqExt3^1) * EXT_SIG3_COMP_BIT);
		}
	}

	/* Latch completion state setting */
//	RspLtBit = ((SvControlIf->LtReqCphs^1) * C_PHASE_COMP_BIT)
//			 | ((SvControlIf->LtReqExt1^1) * EXT_SIG1_COMP_BIT)
//			 | ((SvControlIf->LtReqExt2^1) * EXT_SIG2_COMP_BIT)
//			 | ((SvControlIf->LtReqExt3^1) * EXT_SIG3_COMP_BIT);

	/* Latch request level setting */
	SvControlIf->RspLtBit = RspLtBit | (SvControlIf->CmdLtBit & 0xF);
}


/****************************************************************************************************/
/*																									*/
/*		Control position signal processing																			*/
/*																									*/
/****************************************************************************************************/
static	void	posMngCtrlPosSignal( POS_MNG_HNDL *PosManager, POS_CTRL_STS *PosCtrlSts,
									PCMDFIL	*pPcmdFil /* <S081> */ )
{
	LONG	abs_poserr;

	abs_poserr = PosManager->var.PosErrA.per32a;

/*--------------------------------------------------------------------------------------------------*/
/*		Positioning completion signal creation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( abs_poserr <= PosManager->conf.CoinLevel )
	{
		switch( PosManager->conf.CoinOutSel )
		{
		case POSERR:		/* With position deviation/COINOutput */
			PosCtrlSts->CoinSignal = TRUE;
			break;

		case POSERR_REFOUT:	/* With position deviation and position command filter output/COINOutput */
//<S081>			PosCtrlSts->CoinSignal = PosCtrlSts->RefZSignal;
			PosCtrlSts->CoinSignal = PcmdFilGetRefZStatus( pPcmdFil );	/* <S081> */
			break;

		case POSERR_REFIN:	/* With position deviation and position command input/COINOutput */
			PosCtrlSts->CoinSignal = (PosManager->var.dPcmda == 0) ? TRUE : FALSE;
			break;

		default:
			break;
		}
	}
	else
	{
		PosCtrlSts->CoinSignal = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Positioning vicinity signal creation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( abs_poserr <= PosManager->conf.NearLevel )
	{
		PosCtrlSts->NearSignal = TRUE;
	}
	else
	{
		PosCtrlSts->NearSignal = FALSE;
	}
}


#if 0 /* <S065> */
#if ( CSW_SVCF_RESVIB_FIL_USE == TRUE )
/****************************************************************************************************/
/*																									*/
/*		Position command filter initialization																		*/
/*																									*/
/****************************************************************************************************/
static	void	posMngInitPcmdFilter( VIBSUPFIL *VibSupFil )
{
	MlibResetLongMemory( &VibSupFil->var, sizeof(VibSupFil->var)/4 );
	VibSupFil->conf.Pexe = VibSupFil->conf.VibSupPrm;
}


/****************************************************************************************************/
/*																									*/
/*		Vibration suppression filter calculation																		*/
/*																									*/
/*	Overview:	The vibration suppression filter is a filter with notch characteristics for the set frequency.			*/
/*			Suppresses vibration during positioning such as machine base vibration.												*/
/*			Moving average equal position command filter,MFCBoth can be connected in series.										*/
/*																									*/
/****************************************************************************************************/
static	LONG	posMngVibSupFilter( VIBSUPFIL *VibSupFil, LONG dPcmd, BOOL *RefZSignal )
{
	LONG	AvffFili;
	LONG	AvffFilo;
	LONG	x1;
	LONG	x2;
	LONG	x3;
	LONG	wk = 0;

/*--------------------------------------------------------------------------------------------------*/
/*	Command completion & dispensing completion check																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (dPcmd == 0) && (VibSupFil->var.Buf == 0) )
	{ /* Command complete && No filter accumulation->Payout completion */
		VibSupFil->conf.Pexe = VibSupFil->conf.VibSupPrm;		/* Execute parameter switching				*/
	}
	AvffFili = dPcmd;											/* Vibration suppression filter←Position command pulse	*/

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression filter calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( VibSupFil->conf.Pexe.enable )
	{	/* Function enabled	*/
		x1 = MlibPfbkxremNolim( AvffFili, VibSupFil->conf.Pexe.Kff1, &VibSupFil->var.rem1 );
		x2 = MlibPfbkxremNolim( VibSupFil->var.wkbf1, VibSupFil->conf.Pexe.Kff2, &VibSupFil->var.rem2 );
		x3 = MlibPfbkxremNolim( VibSupFil->var.wkbf2, VibSupFil->conf.Pexe.Kff3, &VibSupFil->var.rem3 );

		VibSupFil->var.wkbf1 += (x1 - x3 - AvffFili);
		VibSupFil->var.wkbf2 += (x1 - x3 + x2);

		AvffFilo = x1 - x3;										/* Vibration suppression filter output				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Filter input/output deviation(Accumulation)																	*/
/*--------------------------------------------------------------------------------------------------*/
		VibSupFil->var.Buf = MlibPerrcalx( AvffFili, AvffFilo, VibSupFil->var.FilioErr );
		if( VibSupFil->var.Buf != 0 )
		{ /* Position command payout not completed */
			*RefZSignal = FALSE;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression filter variable initialization																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( (AvffFilo == 0)	&& (VibSupFil->var.wkbf1 == 0) && (VibSupFil->var.wkbf2 == 0) )
		{ /* Vibration suppression filter output=0 && Vibration suppression filterBUF=0 */
			/* Vibration suppression filter variable initialization */
			MlibResetLongMemory( &VibSupFil->var, sizeof(VibSupFil->var)/4 );
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Filter output processing																			*/
/*--------------------------------------------------------------------------------------------------*/
		wk += AvffFilo;											/* When the function is disabled					*/
		VibSupFil->var.Filo = wk;								/* Output filter output + command input		*/
	}
	else
	{	/* Function disabled	*/
		wk = AvffFili;											/* Outputs input as is				*/
		VibSupFil->var.Filo = wk;
	}

	return( wk );
}


/****************************************************************************************************/
/*																									*/
/*		Vibration suppression filter selection																		*/
/*																									*/
/****************************************************************************************************/
BOOL	PosMngCalculatePrmVibSupFilSW( VIBSUPFIL *VibSupFil, ULONG mdlsw, ULONG ff_feq, ULONG ff_fil, LONG ScanTimeNs )
{
	BOOL	PrmSetErr;
	LONG	wk;

	PrmSetErr = FALSE;
	wk = (mdlsw >> 4) & 0x0F;

	switch( wk )
	{
	case 0x00: /* Function disabled */
	case 0x01:
		VibSupFil->conf.VibSupSetting = FALSE;
		break;
	case 0x02: /* Vibration suppression filter enabled */
		VibSupFil->conf.VibSupSetting = TRUE;
		break;
	default :
		PrmSetErr = TRUE;
		break;
	}

	/*------------------------------------------------------------------------------------------*/
	/*	MFC Vibration suppression filter when switch is disabledOFF													*/
	/*------------------------------------------------------------------------------------------*/
	wk = mdlsw & 0x0F;
	switch( wk )
	{
	case 0x00: /* Function disabled */
		/* Vibration suppression filter forcedOFF */
		VibSupFil->conf.VibSupSetting = FALSE;
		break;
	}

	/* Reflect settings to function selection switch for execution */
	PosMngCalculatePrmVibSupFil( VibSupFil, ff_feq, ff_fil, ScanTimeNs );

	return	PrmSetErr;
}




/****************************************************************************************************/
/*																									*/
/*		Vibration suppression filter parameter calculation																*/
/*																									*/
/****************************************************************************************************/
void	PosMngCalculatePrmVibSupFil( VIBSUPFIL *VibSupFil, ULONG ff_frq, ULONG ff_fil, LONG ScanTimeNs )
{
	VIBSUPFILPRM	wrkp;
	LONG			u1;
	LONG			s, sx;
	LONG			kx;
	LONG			wk;
	LONG			omega2;
//	LONG			scantime;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation cycle setting																				*/
/*--------------------------------------------------------------------------------------------------*/
//	scantime = AVFILCALCYCLE;							/* Calculation cycle time	[ns]				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Function selection																					*/
/*--------------------------------------------------------------------------------------------------*/
	wrkp.enable = VibSupFil->conf.VibSupSetting;
	u1 = ff_fil;

/*--------------------------------------------------------------------------------------------------*/
/*		ω2 Maximum value																					*/
/*																									*/
/*					      (ff_frq/10)[Hz] * (ff_fil[%]/100) * (80/100)* 10^9						*/
/*			omega2_limit = --------------------------------------------------------					*/
/*				   		               2 * PAI * CycleNs											*/
/*--------------------------------------------------------------------------------------------------*/
	wk = (LONG)ff_frq * (LONG)ff_fil;					/* Parameter settingsω2*1000[Hz]				*/
	omega2 = MlibMIN( wk, (MAXOMEGA2(ScanTimeNs) * 1000) );	/* *1000[Hz]Minimum value of limit and parameter value	*/

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression filter gain1 Avff.P.Kff1															*/
/*--------------------------------------------------------------------------------------------------*/
/*					      ff_fil^2																	*/
/*				Kff1 = --------------																*/
/*				   		   10000																	*/
/*--------------------------------------------------------------------------------------------------*/
	wrkp.Kff1 = MlibScalKxgain( u1 * u1, 1, 10000, NULL, 24 );

/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression filter gain2 Avff.P.Kff2															*/
/*--------------------------------------------------------------------------------------------------*/
/*		Vibration suppression filter gain3 Avff.P.Kff3															*/
/*--------------------------------------------------------------------------------------------------*/
/*					     PAI * omega2 * CycleNs														*/
/*				Kff2 = --------------------------													*/
/*				   		       1000 * 10^9															*/
/*--------------------------------------------------------------------------------------------------*/
/*					     4*PAI * omega2 * CycleNs													*/
/*				Kff3 = -----------------------------												*/
/*				   		       1000 * 10^9															*/
/*--------------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( omega2, ScanTimeNs, C10POW9, &s, 0 );
	kx = MlibPcalKxgain( kx, 1, 1000000000, &s, 0 );
	sx = s;

	wrkp.Kff2 = MlibPcalKxgain( kx, 3141593, 1, &s, 24 );				/* 3141593  =   PAI*1000000	*/
	wrkp.Kff3 = MlibPcalKxgain( kx, 12566371, 1, &sx , 24 );			/* 12566371 = 4*PAI*1000000	*/

/*--------------------------------------------------------------------------------------------------*/
/*		Set Parameters																				*/
/*--------------------------------------------------------------------------------------------------*/
//	KPI_DI();		/* Disable Interrupt	*/
	VibSupFil->conf.VibSupPrm = wrkp;
//	KPI_EI();		/* Enable Interrupt		*/

}

#endif /* #if( CSW_SVCF_RESVIB_FIL_USE != 0 ) */
#endif /* #if 0 */ /* <S065> */


/****************************************************************************************************/
/*																									*/
/*		Electronic gear function enabled/Disable switching processing															*/
/*																									*/
/****************************************************************************************************/
void	BpiEgearFuncSwitch( POS_MNG_HNDL *PosManager, BOOL Switch )
{
	if( Switch == TRUE )
	{ /* When requesting to enable electronic gear function */
		PosManager->var.EgearDisable = FALSE;
		PosManager->Egear = &(PosManager->conf.Egear);
	}
	else
	{ /* When requesting to disable electronic gear function */
		PosManager->var.EgearDisable = TRUE;
		PosManager->Egear = &(PosManager->conf.InvalidEgear);
	}
}


/****************************************************************************************************/
/*																									*/
/*		Position command filter enable/Disable switching processing														*/
/*																									*/
/****************************************************************************************************/
void	BpiPcmdFilSwitch( POS_MNG_HNDL *PosManager, BOOL Switch )
{
	if( Switch == TRUE )
	{ /* When position command filter activation is requested */
		PosManager->var.PcmdFilStop = FALSE;
	}
	else
	{ /* When position command filter is requested to be disabled */
		PosManager->var.PcmdFilStop = TRUE;
	}
}


/****************************************************************************************************/
/*																									*/
/*		Initialization of variables for settling time-related processing															*/
/*																									*/
/****************************************************************************************************/
void	StlgInitSettlingTime( SETTLINGTIME *SettlingTime, LONG CycleTimeUs )
{
	/* Reset Settling Time Variable */
	MlibResetLongMemory( SettlingTime, sizeof(*SettlingTime)/4 );

	/* Set Unit Change Gain [cyc] ⇒ [1us] */
	//SettlingTime->UnitChngGain = MlibPcalKxgain( CycleTimeUs, 1, 100, NULL, 24 );//kiraModification_forAAT_110418
	SettlingTime->UnitChngGain = CycleTimeUs;

#if 0		/* <S0DA> */
	/* Start Settling Time Process */
	KlibRstLongTimer( &(SettlingTime->ProcessTimer) );
#endif

}


/****************************************************************************************************/
/*																									*/
/*		Settling time calculation function																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview: Between the completion of position command payout and the start of the next position command payout, or when the counter isMAXuntil the value	*/
/*		Finally/COINSignal isOFF(Counter value).								*/
/*																									*/
/****************************************************************************************************/
void	StlgCalculateSettlingTime( SETTLINGTIME *SettlingTime, LONG dPcmda, BOOL CoinSts )
{
#if 0		/* <S0DA> */
	/* Check Timing of Measuring Settling Time */
	if( 1 > KlibGetLongTimerMs(SettlingTime->ProcessTimer) )			/* 1[ms]Process every				*/
	{
		return;
	}
	/* Reset Timing of Measuring Settling Time */
	KlibRstLongTimer( &(SettlingTime->ProcessTimer) );
#endif

	/*----------------------------------------------------------------------------------------------*/
	/*	Creating settling time calculation calculation flag																*/
	/*----------------------------------------------------------------------------------------------*/
	if( dPcmda != 0 )
	{ /* In operation */
		if( SettlingTime->StlgLastdPcmd == 0 )
		{ /* For the first scan when a position command is input */
			/* Assign the held settling time to the output variable */
			SettlingTime->RsltStlgTime = SettlingTime->TmpStlgTime;
		}

		/*------------------------------------------------------------------------------------------*/
		/*	Clear variable during operation																*/
		/*------------------------------------------------------------------------------------------*/
		SettlingTime->StlgCalcReq = FALSE;				/* Settling time calculation request flagOFF				*/
		SettlingTime->StlgLastCoinSts = FALSE;			/* PreviousCOINSignalOFF						*/
		SettlingTime->StlgTimeCntr = 0;					/* Clear counter							*/
	}
	else if( SettlingTime->StlgLastdPcmd != 0 )
	{ /* When position command payout is completed(Only first) */
		SettlingTime->StlgCalcReq = TRUE;				/* Settling time calculation request flagON				*/
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Settling time calculation calculation																			*/
	/*----------------------------------------------------------------------------------------------*/
	if( SettlingTime->StlgCalcReq == TRUE )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	SettlingTime->StlgTimeCntr = 65535	(Approx.8[s])			*/
		/*------------------------------------------------------------------------------------------*/
		if( SettlingTime->StlgTimeCntr == 65535 )
		{
			SettlingTime->RsltStlgTime = SettlingTime->TmpStlgTime;
		}

		/*------------------------------------------------------------------------------------------*/
		/*	Positioning completion signal is detectedONBuffer settling time only when									*/
		/*------------------------------------------------------------------------------------------*/
		if( ( CoinSts == TRUE ) && ( SettlingTime->StlgLastCoinSts == FALSE ) )
		{
			SettlingTime->TmpStlgTime = SettlingTime->StlgTimeCntr;
		}

		/*------------------------------------------------------------------------------------------*/
		/*	Increment counter(however65535,)											*/
		/*------------------------------------------------------------------------------------------*/
		if( SettlingTime->StlgTimeCntr != 65535 )
		{
			SettlingTime->StlgTimeCntr++;
		}
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Hold previous value																				*/
	/*----------------------------------------------------------------------------------------------*/
	SettlingTime->StlgLastdPcmd = dPcmda;						/* Hold difference position command				*/
	SettlingTime->StlgLastCoinSts = CoinSts;					/* COINHold Signal					*/

//	BoutV.OutSettlingTime = SettlingTime->RsltStlgTime;

}


/****************************************************************************************************/
/*																									*/
/*		Maximum overshoot amount creation --> Un106														*/
/*																									*/
/****************************************************************************************************/
void	StlgMakeMaxOverShoot( SETTLINGTIME *SettlingTime, POS_MNG_HNDL *PosManager, BOOL PosCtrlMode )
{

	if( (PosCtrlMode == TRUE) && (PosManager->var.dPcmda == 0) )
	{
/*--------------------------------------------------------------------------------------------------*/
/*		Positioning direction determination																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( SettlingTime->LastdPcmda > 0 )		/* Positioning in the positive direction - First time command issued			*/
		{
			SettlingTime->UnMaxOverShoot = 0;		/* Maximum overshoot amount clear					*/
			SettlingTime->OverErrDir = -1;			/* Sign of overshooting deviation(-)			*/
		}
		else if( SettlingTime->LastdPcmda < 0 )	/* Positioning in the negative direction - Initial command output			*/
		{
			SettlingTime->UnMaxOverShoot = 0;		/* Maximum overshoot amount clear					*/
			SettlingTime->OverErrDir = 1;			/* Sign of overshooting deviation(+)			*/
		}

		SettlingTime->OverShoot = SettlingTime->OverErrDir * PosManager->var.PosErrA.per32s;

/*--------------------------------------------------------------------------------------------------*/
/*		Overshoot amount determination																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( SettlingTime->UnMaxOverShoot < SettlingTime->OverShoot )
		{
			SettlingTime->UnMaxOverShoot = SettlingTime->OverShoot;
		}
	}
	else
	{
		BpiRsetMaxOverShoot( SettlingTime );			/* Overshoot amount clearing process				*/
	}
	SettlingTime->LastdPcmda = PosManager->var.dPcmda;
}


/****************************************************************************************************/
/*																									*/
/*		Positioning completion failure cumulative time creation --> Un10B														*/
/*																									*/
/****************************************************************************************************/
void	StlgMakeCoinOffTime( SETTLINGTIME *SettlingTime, LONG dPcmda, BOOL CoinSts, BOOL PosCtrlMode )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Positioning completion confirmation																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( (PosCtrlMode == TRUE) && (dPcmda == 0) )
	{
		if( CoinSts == TRUE )
		{
			SettlingTime->StlgCoinLatch = TRUE;
		}
	}
	else
	{
		SettlingTime->StlgCoinLatch = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		COINCrack Detection --> Positioning completion failure cumulative time creation												*/
/*--------------------------------------------------------------------------------------------------*/
	if( (SettlingTime->StlgCoinLatch == TRUE) && (CoinSts == FALSE) )
	{
		/* Counter upper limit */
		if( SettlingTime->UnCoinOffTimer < 0x1000000 )
		{
			/* Positioning completion failure cumulative time[cycle] */
			SettlingTime->UnCoinOffTimer++;
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Positioning completion failure cumulative time clearing process 														*/
/*			.Fn01C --> Un10B = 0																	*/
/*			.Other than position control mode																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( PosCtrlMode == FALSE )
	{
		BpiRsetCoinOffTime( SettlingTime );
	}

}

/****************************************************************************************************/
/*																									*/
/*		Allowable overshoot amount parameter calculation														*/
/*																									*/
/****************************************************************************************************/
void	StlgCalculateOverShootErrLevel( SETTLINGTIME *SettlingTime, LONG CoinLevel, LONG ovserrdetlvl )
{
	LONG	sx;
	sx = 0;
	SettlingTime->OvsErrDetLvl = MlibScalKxgain( CoinLevel,ovserrdetlvl, 100, &sx, -24 );
}


/****************************************************************************************************/
/*																									*/
/*		Overshoot amount clearing process																*/
/*																									*/
/****************************************************************************************************/
void	BpiRsetMaxOverShoot( SETTLINGTIME *SettlingTime )
{
	SettlingTime->UnMaxOverShoot = 0;
	SettlingTime->OverErrDir = 0;
}


/****************************************************************************************************/
/*																									*/
/*		Positioning completion failure cumulative time clearing process															*/
/*																									*/
/****************************************************************************************************/
void	BpiRsetCoinOffTime( SETTLINGTIME *SettlingTime )
{
	SettlingTime->UnCoinOffTimer = 0;
	SettlingTime->StlgCoinLatch = FALSE;
	SettlingTime->StlgCoinLatch = FALSE;
}


/****************************************************************************************************/
/*																									*/
/*		Positioning completion failure cumulative time acquisition process															*/
/*																									*/
/****************************************************************************************************/
LONG	BpiGetCoinOffTime( SETTLINGTIME *SettlingTime )
{
#if (FLOAT_USE==TRUE)
	return ( SettlingTime->UnCoinOffTimer * SettlingTime->UnitChngGain );
#else
	return ( MlibMulgain( SettlingTime->UnCoinOffTimer, SettlingTime->UnitChngGain) );
#endif /* FLOAT_USE */
}


/****************************************************************************************************/
/*																									*/
/*		Maximum overshoot amount[Command unit]Acquisition processing													*/
/*																									*/
/****************************************************************************************************/
LONG	BpiGetMaxOverShoot( SETTLINGTIME *SettlingTime )
{
	return (SettlingTime->UnMaxOverShoot);
}


/****************************************************************************************************/
/*																									*/
/*		COINLatch status acquisition processing																		*/
/*																									*/
/****************************************************************************************************/
LONG	BpiGetCoinLatchSts( SETTLINGTIME *SettlingTime )
{
	return (SettlingTime->StlgCoinLatch);
}


/****************************************************************************************************/
/*																									*/
/*		Settling time calculation result acquisition process																	*/
/*																									*/
/****************************************************************************************************/
LONG	BpiGetSettlingTime( SETTLINGTIME *SettlingTime )
{
	return (SettlingTime->RsltStlgTime);
}


/****************************************************************************************************/
/*																									*/
/*		Servo control net position response management														<S14C>	*/
/*																									*/
/****************************************************************************************************/
void	PosMngResponseManager( BASE_CTRL_HNDL *BaseControls )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Encoder ABC phase signal creation																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Motor encoder ABC phase signal creation */
	/* 2009.06.04 Y.Oka (>_<) Currently not supported */
//	BpxMakeMotABCphaseSignal( BaseControls );

/*--------------------------------------------------------------------------------------------------*/
/*		Other than position control mode																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseControls->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS )
	{
		/* Servo control position management variable initialization */
		PosMngInitPositionManager( BaseControls );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Zero speed stop mode(Detected in scan C)														*/
/*--------------------------------------------------------------------------------------------------*/
	else if( BaseControls->CtrlModeSet.ZctrlMode.zm == CTRLMODE_ZSTOP )
	{
		;
	}

}


#if 0 /* Currently not supported */
/****************************************************************************************************/
/*																									*/
/*		Motor encoder ABC phase signal creation															*/
/*																									*/
/****************************************************************************************************/
static	void	BpxMakeMotABCphaseSignal( BASE_CTRL_HNDL *BaseControls )
{
	MENCV			*MencV;
	POS_MNG_HNDL	*PosManager;
	BASE_CTRL_IO	*BaseCtrlIo;

	MencV		= BaseControls->MencV;
	PosManager		= BaseControls->sv_ctrl_ptr->PosMngV_ptr;
	BaseCtrlIo		= &( BaseControls->scanb_io );

/*--------------------------------------------------------------------------------------------------*/
/*		C phase passage check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseControls->ScanBMotCposOk )
	{
		if( (BaseControls->ScanBMotCphPass == TRUE) )
		{
			/* C phase output time : 750[us] */
			PosManager->MCphOutTime = ( ((750) * US_CHANGE_MS) / KPI_SBCYCLENS );
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		C phase output signal creation																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( PosManager->MCphOutTime > 0 )
	{
		PosManager->MCphOutTime--;
		BaseCtrlIo->PgCphase = TRUE;
	}
	else
	{
		BaseCtrlIo->PgCphase = FALSE;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		A-phase,B phase signal creation																			*/
/*--------------------------------------------------------------------------------------------------*/
/*		MencV.ABphCntr	APhase,BPhase																		*/
/*		  xxxxxx00  -->  1 , 1 																		*/
/*		  xxxxxx01	-->  1 , 0 																		*/
/*		  xxxxxx10	-->  0 , 0 																		*/
/*		  xxxxxx11	-->  0 , 1 																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch( MencV->ABphCntr & 0x03 )
	{
	case 0x00: BaseCtrlIo->PgAphase = TRUE;  BaseCtrlIo->PgBphase = TRUE;  break;
	case 0x01: BaseCtrlIo->PgAphase = TRUE;  BaseCtrlIo->PgBphase = FALSE; break;
	case 0x02: BaseCtrlIo->PgAphase = FALSE; BaseCtrlIo->PgBphase = FALSE; break;
	default  : BaseCtrlIo->PgAphase = FALSE; BaseCtrlIo->PgBphase = TRUE;  break;
	}
	return;
}
#endif /* #if 0 */



/***************************************** From hereA/P **********************************************/



#if 0 //(CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
/****************************************************************************************************/
/*																									*/
/*		Servo control position command management																		*/
/*																									*/
/****************************************************************************************************/
LONG	BpxSvcPosCommandManager( BASE_CTRL_HNDL *BaseControls, FUN_CMN_CONTROL *FnCmnCtrl )
{
	POS_MNG_HNDL	*PosManager;
	MFCTRL			*MFControl;
	LONG			dPosRefo;
	LONG			rc;
#if 0 /* 2012.09.06 Y.Oka ★Position deviation clear currently not supported★ */
	LONG			lwk_dPcmdHwCntr;
#endif

	PosManager	= &(BaseControls->PosManager);
	MFControl	= &(BaseControls->MFControl);

#if 0 /* 2012.09.06 Y.Oka ★Position deviation clear signal input not supported★ */
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation clear signal monitoring																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( SVFSEL_LTM_PERCLR == 1 )
	{
		KpiChkPerClearSignal( );						/* Kernel position deviation clear signal monitoring			*/
	}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Other than position control mode																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseControls->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS )
	{
		BaseControls->PosCtrlSts.DenSignal = FALSE;
		return( 0 );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Payout completion signal processing: Payout incomplete module sets it to FALSE.								*/
/*--------------------------------------------------------------------------------------------------*/
	BaseControls->PosCtrlSts.DenSignal = TRUE;

/*--------------------------------------------------------------------------------------------------*/
/*		Position command input processing(Pulse train,PJOG,etc):PosMngV.dPcmda	[Command unit/Scan]							*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Table operation mode																		*/
	/*----------------------------------------------------------------------------------------------*/
	if( BaseControls->TblDrive.var.TblCmd != TBLCMD_NOCMD )
	{ /* During table operation */
#if 0 /* 2012.09.06 Y.Oka ★Position deviation clear currently not supported★ */
		BoutV.f.PerClrEdge = FALSE;							/* Position deviation clear edge signal reset		*/
		BoutK.f.PerClrEdge = FALSE;							/* Position deviation clear edge signal reset		*/
#endif
	/*------------------------------------------------------------------------------------------*/
		if( BaseControls->BeComplete == TRUE )
		{
			/* Table operation position command */
			dPosRefo = BaseControls->TblDrive.var.dPosRef;
			/* Position deviation clear command */
			BaseControls->CtrlCmdMngr.PerrClrReq = FALSE;
		}
		else
		{
#if ( SVFSEL_BLSH_COMMON_PROC )		/* <V868> Standard.Y522Common use	*/
			/* Backlash correction disabled */
			BlshInvaildCompensate( &(BaseControls->PosManager.BlshCmp) );
#endif
			/* Servo control position management variable initialization */
			PosMngInitPositionManager( BaseControls );
			/* Position deviation clear request */
			BaseControls->CtrlCmdMngr.PerrClrReq = TRUE;
			/* ScanBPosition deviation clear */
			MlibResetLongMemory( &(BaseControls->PosManager.PosErrA), sizeof(BaseControls->PosManager.PosErrA)/4 ); 	/* <S107> */
			MlibResetLongMemory( &(BaseControls->PosManager.PosErrAmp), sizeof(BaseControls->PosManager.PosErrAmp)/4 );
			/* Position control end */
			return( 0 );
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		ProgramJOGOperation mode																	*/
	/*----------------------------------------------------------------------------------------------*/
	else if( FnCmnCtrl->FnCtrlMcmd == CTRLMODE_PJOG )
	{ /* ProgramJOGIn mode */
#if 0 /* 2012.09.06 Y.Oka ★Position deviation clear currently not supported★ */
		BoutV.f.PerClrEdge = FALSE;						/* Position deviation clear edge signal reset		*/
		BoutK.f.PerClrEdge = FALSE;						/* Position deviation clear edge signal reset		*/
#endif
	/*------------------------------------------------------------------------------------------*/
		if( FnCmnCtrl->FnCtrlMcmd != CTRLMODE_PJOG )
		{												/* When control mode is switched			*/
			PrgJogReset( &(BaseControls->PJogHdl) );		/* ProgramJOGOperation data reset		*/
		}
	/*------------------------------------------------------------------------------------------*/
		if( BaseControls->BeComplete == TRUE )
		{ /* Position control execution */

			/* Position command creation processing	[Command unit/Scan]	*/
			rc = PrgJogMakePositionReference( &BaseControls->PJogHdl, &dPosRefo );

			if( rc == TRUE )
			{/* ProgramJOGCompletion end */
				BaseControls->PosCtrlSts.DenSignal  = TRUE;
			}
			else
			{/* ProgramJOGCalculation in progress */
				BaseControls->PosCtrlSts.DenSignal  = FALSE;
				BaseControls->PosCtrlSts.RefZSignal = FALSE;
			}
			BaseControls->CtrlCmdMngr.PerrClrReq = FALSE;
		}
		else
		{
#if ( SVFSEL_BLSH_COMMON_PROC )
			/* Backlash correction disabled */
			BlshInitializeCompensate( &(BaseControls->PosManager.BlshCmp) );
#endif
			/* Servo control position management variable initialization */
			PosMngInitPositionManager( BaseControls );
			/* Position deviation clear request */
			BaseControls->CtrlCmdMngr.PerrClrReq = TRUE;
			/* ScanBPosition deviation clear */
			MlibResetLongMemory( &(BaseControls->PosManager.PosErrA), sizeof(BaseControls->PosManager.PosErrA)/4 );	/* <S107> */
			MlibResetLongMemory( &(BaseControls->PosManager.PosErrAmp), sizeof(BaseControls->PosManager.PosErrAmp)/4 );
			return( 0 );
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Normal operation mode																			*/
	/*----------------------------------------------------------------------------------------------*/
	else
	{
/* 2012.09.06 Y.Oka ★Pulse input magnification change function currently not supported★ */
#if SVFSEL_PULSE_INPUT_SWITCHING == 1
	/*------------------------------------------------------------------------------------------*/
	/*		Command pulse multiplier processing															<V677>	*/
	/*		BoutK.PcmdRingCntrupdate process isLpxMultiplyInputPulse()					*/
	/*------------------------------------------------------------------------------------------*/
		/* ★★dPosRefo! !★★ */
		lwk_dPcmdHwCntr     = LpxMultiplyInputPulse( );	/* Position command difference value update[Command unit/Scan]	*/
														/* Position command ring counter update			*/
#else
#if 0 /* 2012.09.06 Y.Oka ★Position deviation clear currently not supported★ */
		lwk_dPcmdHwCntr = PosManager->var.dPcmdHwCntr;	/* Position command difference value update[Command unit/Scan]	*/
#endif
//		BoutK.PcmdRingCntr += BinK.dPcmdInputHwCntr;	/* Position command ring counter update			*/
		dPosRefo = PosManager->var.dPcmdInputHwCntr;	/* Position command difference value update[Command unit/Scan]	*/
#endif

#if 0 /* 2012.09.06 Y.Oka ★Position deviation clear currently not supported★ */
	/*------------------------------------------------------------------------------------------*/
	/*		Position deviation clear processing																	*/
	/*------------------------------------------------------------------------------------------*/
		BoutV.f.PerClrCmd = BinV.f.PerClrCmd;			/* Position deviation clear command					*/
		if( BoutV.f.PerClrCmd )							/* Position deviation clear command					*/
		{
			BpxEdgeClrSvcPositionManager( );			/* Position deviation edge clear variable initialization		*/
		//	BoutV.AmondPosCmd = BinK.dPcmdHwCntr;		/* For position command monitor	[Command unit/Scan]		*/
			BoutV.AmondPosCmd =  lwk_dPcmdHwCntr;		/* For position command monitor	[Command unit/Scan]		*//*<V677>*/
			BoutV.f.PerClrEdge = FALSE;					/* Position deviation clear edge signal reset		*/
			BoutK.f.PerClrEdge = FALSE;
			return( FALSE );							/* Position control end							*/
		}
#endif

#if 0 /* 2012.09.06 Y.Oka ★INHIBITFunction currently not supported★ */
	/*------------------------------------------------------------------------------------------*/
	/*		Pulse train position command input																*/
	/*------------------------------------------------------------------------------------------*/
		dPosRefo = ( BinV.f.Sv.Inhibit ) ? 0 : dPosRefo;
#endif /* 2012.09.06 Y.Oka ★INHIBITFunction currently not supported★ */

#if 0 /* 2012.09.06 Y.Oka ★Position deviation clear currently not supported★ */
	/*------------------------------------------------------------------------------------------*/
	/*		Position deviation edge clear processing															*/
	/*------------------------------------------------------------------------------------------*/
		if( Iprm.f.PerClrEdge && BinK.f.PerClrEdge )
		{
			BoutV.f.PerClrEdge = TRUE;					/* Position deviation clear edge signal set		*/
			BpxPerClrSignalProcedure( );				/* Position deviation edge clear signal processing			*/
			BoutK.f.PerClrEdge = TRUE;					/* FBPosition/command position update flag			*/
		}
		else
		{
			BoutV.f.PerClrEdge = FALSE;					/* Position deviation clear edge signal reset		*/
			BoutK.f.PerClrEdge = FALSE;					/* FBPosition/command position update flag			*/
		}
	/*------------------------------------------------------------------------------------------*/
#endif
	}
	/* Position command ring counter update */
	BaseControls->CtrlCmdMngr.PosRef += dPosRefo;
	/* Updating division position command */
	PosManager->var.dPcmda = dPosRefo;


/*--------------------------------------------------------------------------------------------------*/
/*		Position command electronic gear conversion:PosMngV.dPcmdEgear [Pulse/Scan]										*/
/*--------------------------------------------------------------------------------------------------*/
	if( PosManager->var.EgearDisable )
	{ /* Electronic gear disabled */
		PosManager->var.dPcmdEgear = PosManager->var.dPcmda;
	}
	else
	{ /* Electronic gear enabled */
		/* Electronic gear calculation */
		PosManager->var.dPcmdEgear = MlibPcmdEgear( PosManager->var.dPcmda,
													PosManager->Egear,
													&(PosManager->var.dPcmdEgrem) );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Payout completion signal processing: Payout incomplete moduleFALSE.										*/
/*--------------------------------------------------------------------------------------------------*/
	BaseControls->PosCtrlSts.RefZSignal = ( PosManager->var.dPcmdEgear != 0 ) ? FALSE : TRUE;

/*--------------------------------------------------------------------------------------------------*/
/*		Position command filter calculation(Acceleration/deceleration,Moving average,Vibration suppression):BoutV.dPcmdFilo  [Pulse/Scan]				*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /* 2012.08.07 Y.Oka Position command filter currently not supported */
	BaseControls->CtrlCmdMngr.dPcmdFilo = BpxPcmdFilter( PosManager->var.dPcmdEgear );
#else
	BaseControls->CtrlCmdMngr.dPcmdFilo = PosManager->var.dPcmdEgear;
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Extend(FF)Control calculation(MFC,Sway prevention control)														*/
/*--------------------------------------------------------------------------------------------------*/
/*		BoutV.dPosRefi [Pulse/Scan]: Position control position command difference input										*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Backlash correction processing																	*/
	/*----------------------------------------------------------------------------------------------*/
#if ( CSW_SVCF_BACKLASHCOMP )
	if( (BaseControls->BeComplete == FALSE)
		|| (BaseControls->PhaseReady == FALSE)
		|| (BaseControls->OverTravel == TRUE) )
	{
		BlshActive = FALSE;
	}
	else
	{
		BlshActive = TRUE;
	}
	BlshCompensateBacklash( &(PosManager->BlshCmp),
							&(PosManager->conf.Egear),
							&(PosManager->var.dPcmdEgear),
							&(dPosRefo),
							&(BaseControls->PosCtrlSts.RefZSignal),
							BlshActive );
#endif

	/*----------------------------------------------------------------------------------------------*/
	/*		Vibration suppression filter processing																	*/
	/*----------------------------------------------------------------------------------------------*/
#if ( CSW_SVCF_RESVIB_FIL_USE == TRUE )
	if( PosManager->var.PcmdFilStop == FALSE )
	{ /* No request to disable position command filter */
		/* Vibration suppression filter forced invalidation process(Disabled for other than position control) */
		if( BaseControls->CtrlModeSet.CtrlModeLst.b.cm != CTRLMODE_POS )
		{
			BaseControls->VibSupFil.conf.Pexe.enable = 0;
		}
		/* Vibration suppression filter calculation */
		BaseControls->CtrlCmdMngr.dPcmdFilo = posMngVibSupFilter(
												&(BaseControls->VibSupFil),
												PosManager->var.dPcmdEgear,
												&(BaseControls->PosCtrlSts.RefZSignal) );
	}
	else
	{ /* Position command filter disable is requested */
		BaseControls->CtrlCmdMngr.dPcmdFilo = PosManager->var.dPcmdEgear;
	}
#else
		BaseControls->CtrlCmdMngr.dPcmdFilo = PosManager->var.dPcmdEgear;
#endif

	/*----------------------------------------------------------------------------------------------*/
	/*		MFCProcessing																					*/
	/*----------------------------------------------------------------------------------------------*/
#if ( CSW_SVCF_MFC_USE == TRUE )
	/* MFCForced invalidation process(Disabled for other than position control) */
	if( BaseControls->CtrlModeSet.CtrlModeLst.b.cm != CTRLMODE_POS )
	{ /* If previous control mode is not position control */
		/* MFCInitialize to disabled */
		MFControl->conf.MfcPexe.MFCModel = 0;
	}
	/* MFCOperation */
	dPosRefo = MfcControlMain( &(BaseControls->MFControl),
							   BaseControls->CtrlCmdMngr.dPcmdFilo,
							   &(BaseControls->CtrlCmdMngr.SpdFFCx),
							   &(BaseControls->CtrlCmdMngr.TrqFFCx) );
#else
	MFControl->conf.MfcPexe.MFCModel = 0;
	dPosRefo = BaseControls->CtrlCmdMngr.dPcmdFilo;
#endif


/*--------------------------------------------------------------------------------------------------*/
/*		Control position signal processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* Make Coin and Near Status */
	posMngCtrlPosSignal( PosManager, &(BaseControls->PosCtrlSts) );

	return dPosRefo;
}
#endif /* (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE) */



#if 0 /* 2012.08.07 Y.Oka Position command filter currently not supported */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
/****************************************************************************************************/
/*																									*/
/*		Position command filter processing main function																*/
/*																									*/
/****************************************************************************************************/
LONG	BpxPcmdFilter( LONG pcmdin )
{
LONG	pcmdout;
LONG	orgpcmdin;

/*--------------------------------------------------------------------------------------------------*/
/*		Position command filter selection processing																	*/
/*--------------------------------------------------------------------------------------------------*/
	FilMng.FilSel = Iprm.PcmdFilter;

/*--------------------------------------------------------------------------------------------------*/
/*		Position command filter switching prohibition processing: Switching disabled module must be set to TRUE					*/
/*--------------------------------------------------------------------------------------------------*/
	FilMng.FilChgDisable = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Position command filter execution/stop processing																*/
/*--------------------------------------------------------------------------------------------------*/
	if ( FilMng.FilStop == TRUE )
	{
		orgpcmdin = pcmdin;
		pcmdin = 0;
	}
	else
	{
		orgpcmdin = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Position command filter selection Filter selection and execution process using user constants									*/
/*--------------------------------------------------------------------------------------------------*/
	pcmdout = PcmdMoveAvergeFilter( pcmdin );			/* Moving average filter calculation						*/
	pcmdout = PcmdExpFilter( pcmdout );					/* Acceleration/deceleration filter calculation						*/
	pcmdout += orgpcmdin;
	
	return( pcmdout );
}
#endif /* (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE) */
#endif /* 2012.08.07 Y.Oka Position command filter currently not supported */



/****************************************************************************************************/
/*																									*/
/*		Position deviation clear signal processing																		*/
/*																									*/
/****************************************************************************************************/
void	BpxPerClrSignalProcedure( BASE_LOOPCTRLS *BaseLoops, BPRMDAT *Bprm )
{
BASE_CTRL_HNDL *BaseControls;
LONG	lwk;

		BaseControls = BaseLoops->BaseCtrls;	
		/*------------------------------------------------------------------------------------------*/
		/*	Servo control position management variable initialization															*/
		/*------------------------------------------------------------------------------------------*/
		BpxEdgeClrSvcPositionManager( BaseLoops );

		/*------------------------------------------------------------------------------------------*/
		/*	FB position correction calculation		-> (PlsIf.c)													*/
		/*------------------------------------------------------------------------------------------*/
		/*------------------------------------------------------------------------------------------*/
		/*	FB position calculation			-> (PlsIf.c)													*/
		/*------------------------------------------------------------------------------------------*/
		BaseControls->CtrlCmdMngr.ClrFbPos = BaseControls->ClrFbPos;	/* FB position at edge input	*/
		if(Bprm->FencUse != FALSE)/* <S04D>:Addition of Fluke processing */
		{
			lwk = BaseControls->MotSts.FencPos - BaseControls->ClrFbPos;/* set /CLR Position 		*/
		}
		else
		{
			lwk = BaseControls->MotSts.MotPos - BaseControls->ClrFbPos;	/* set /CLR Position 		*/
		}
		MlibAposRg64dp(
					-lwk,
					&(BaseControls->PosManager.conf.Egear), 
					&(BaseControls->PosManager.PgPos), 
					&(BaseControls->MonPerrLpos[0]) );
		/*------------------------------------------------------------------------------------------*/
		/*	<S112> Set position error (Un008)														*/
		/*------------------------------------------------------------------------------------------*/
#if	0	/* Unnecessary? <S14C> */
		lwk = (BaseControls->PosManager.PgPos.apos[0] - BaseControls->MonPerrLpos[0]);
		BaseControls->PosManager.var.PosErrA.per32s = -lwk;			/* set position error [ref]		*/
		BaseControls->PosManager.var.PosErrA.per32a = MlibABS(lwk);	/* set abs. position error [ref]*/
#endif
		/*------------------------------------------------------------------------------------------*/
		return;
}

/****************************************************************************************************/
/*																									*/
/*		Servo control position deviation edge clear variable initialization													*/
/*																									*/
/****************************************************************************************************/
void	BpxEdgeClrSvcPositionManager( BASE_LOOPCTRLS *BaseLoops )
{
BASE_CTRL_HNDL	*BaseControls;
POSCTRL			*PosCtrl;
POS_MNG_HNDL	*PosMngV;
	
		BaseControls = BaseLoops->BaseCtrls;	
		PosCtrl = &(BaseLoops->PosCtrl);
		PosMngV = &(BaseControls->PosManager);
/*--------------------------------------------------------------------------------------------------*/
/*		Servo control position management variable initialization																*/
/*--------------------------------------------------------------------------------------------------*/
//TODO:Also clear position command for analog monitor	BoutV.AmondPosCmd  = 0;		/* Position command monitor variable		*/
		/* C phase output time was also cleared, so clear other variables								*/
		MlibResetLongMemory( &(PosMngV->var.Per64),sizeof(PosMngV->var.Per64)/4); /* <S14D>			*/
		MlibResetLongMemory( &(PosMngV->var.PosErrA), sizeof(PERRA)/4 );/* Position management variable initialization<S14C> */
		PosMngV->var.dPcmdEgear = 0;								/* Position command electronic gear output			*/
		PosMngV->var.dPcmdEgrem = 0;								/* Position command electronic gear calculation remainder		*/
		PosMngV->var.dPcmda = 0;									/* Position command incremental value input[Command unit]	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Servo control position management variable initialization																*/
/*--------------------------------------------------------------------------------------------------*/
		BaseControls->SettlingTime.LastdPcmda = 0;			/* Previous position command increment value[Command unit]			*/
		
		MlibResetLongMemory( &(PosMngV->BlshCmp.var), sizeof(PosMngV->BlshCmp.var)/4 );	/* Backlash correction variable initialization		*/

		PosCtrl->V.PerIvar64[0] = 0;						/* Clear positional integral buffer		<S14C>	*/
		PosCtrl->V.PerIvar64[1] = 0;						/* Clear positional integral buffer				*/
		PosCtrl->V.LastPacOut = 0;							/* Clear previous position differential value				*/
		PosCtrl->V.dPFfFilo = 0;							/* Position FF filter buffer clear		*/

		BaseControls->CtrlCmdMngr.dPFfFilo = 0;				/* Clear previous position differential value				*/

		BaseControls->PosManager.dPosRefi = 0;				/* Position command difference input[Pulse/Scan]			*/
		BaseControls->PosManager.CompdPosRefi = 0;			/* Position command difference input(ScanB->ScanA)		*/
		BaseControls->PosManager.CompdPcmda = 0;			/* Position command incremental value input(ScanB->ScanA)		*/
	/*----------------------------------------------------------------------------------------------*/
	if( BaseControls->CtrlModeSet.CtrlMode.b.cm == CTRLMODE_POS )
	{
		BaseControls->PosCtrlSts.CoinSignal = TRUE;			/* @CHK@:SGDStailored to					*/
		BaseControls->PosCtrlSts.NearSignal = TRUE;			/* @CHK@:SGDStailored to					*/
	}
	else
	{
		BaseControls->PosCtrlSts.CoinSignal = FALSE;		/* @CHK@:SGDStailored to					*/
		BaseControls->PosCtrlSts.NearSignal = FALSE;		/* @CHK@:SGDStailored to					*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position command related variable initialization: Position command filter,MFC etc.											*/
/*--------------------------------------------------------------------------------------------------*/
	/* Position command filter initialization processing */
//	posMngInitPcmdFilter( &(BaseControls->VibSupFil) );												  /* <S065> */
	PcmdFilInitialize( &(BaseControls->PcmdFil), &(BaseControls->VibSupFil) );
	/* MFCInitializing variables */
	MfcInitModelControl( &(BaseControls->MFControl) );

	return;

}
/***************************************** end of file **********************************************/

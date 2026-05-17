/****************************************************************************************************/
/*																									*/
/*																									*/
/*		BaseControls.c : Servo base control management processing definition										*/
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
#include "BaseControls.h"
#include "SerialEncIf.h"
#include "PosManager.h"
#include "ResVibCtrl.h"
#include "GainChange.h"
//#include "SequenceControls.h"
#include "PowerManager.h"
#include "MotorPoleFind.h"
#include "MLib.h"
#include "HwDeviceIf.h"
#include "JL086ServoIpHw.h"
#include "AlarmManager.h"



/****************************************************************************************************/
/*		DEFINES																					*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Output Mapping Data and Output Macro Definition												*/
/*--------------------------------------------------------------------------------------------------*/
#define	SOUT00	5
#define	SOUT01	6
#define	SOUT02	7
#define	SOUT03	4

/*--------------------------------------------------------------------------------------------------*/
/*		Mechatrolink Monitor Table																	*/
/*--------------------------------------------------------------------------------------------------*/
#define MONTBL10	0x10
#define MONTBL20	0x20
#define MONTBL30	0x30
#define MONTBL40	0x40
#define MONTBL50	0x50
#define MONTBL60	0x60


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void BpxBinputNetIf( BASE_CTRL_HNDL*, SERVO_CONTROL_IF*, TRQ_LMT_DATA* );						*/
/*	void BpxYecServoControl( BASE_CTRL_HNDL*, SERVO_CONTROL_IF*, CTRL_LOOP_OUT*, FUN_CMN_CONTROL* );*/
/*	void BpiMpoleFindMakePosCmd( BASE_LOOPCTRLS*, SERVO_CONTROL_IF*, FN_CMN_CONTROL* );		<S02F>	*/
/*	void BpxTorqueLimitControl( TRQ_LMT_DATA*, BASE_CTRL_HNDL*, BOOL );								*/
/*	void BpxBoutputNetIf( SERVO_CONTROL_IF*, SEQ_CTRL_OUT*, LONG, LONG );							*/
/*	void BpiChangeSpdDetMaFil( MOTSPDMAFIL*, LONG num );								<S02F>		*/
/****************************************************************************************************/
static void LpxNetScanBInputExtCommand( TRQ_LMT_DATA *TrqLimits, USHORT ExtSel,
																		EXTENDED_DATA ExtData );
#if	(CSW_SVCF_FBLATCH == TRUE)
static void LpxLatchRequestGet( SERVO_CONTROL_IF *SvControlIf, LONG CmdLtBit );
#endif
//static void ModeSwitch( MODE_SW *ModeSwData, LONG trq_ref, LONG spd_ref, LONG pos_err );<S0BA>
static LONG BaseSoftStartSpdRef( CTRL_CMD_PRM *CtrlCmdPrm, LONG SpdRef, LONG SpdRefx );
static LONG	BaseLinDecStopSpdRef( CTRL_CMD_PRM *CtrlCmdPrm, LONG SpdRefo ); /* <S1F5> */
static LONG ZeroSearchControl( ZX_CTRL_HNDL *ZeroClampCtrl, BASE_MOT_STS *MotSts,
																	KSGAIN ZcKp, LONG JogSpeed );
static LONG OverTorqueCalculation( CTRL_CMD_MNG *CtrlCmdMng, CTRL_CMD_PRM *CtrlCmdPrm );
static void LpxSetNetSelMonitor( LONG **MonTbl, LONG SelMon,
															LONG *RspSelMon, LONG *RspMonData );
static void PosRefRenewal( POS_MNG_HNDL *PosManager, CTRLMD CtrlMode );

static void	BpxBinputNetIf( BASE_CTRL_HNDL*, SERVO_CONTROL_IF*, TRQ_LMT_DATA*, LONG ax_no );/* <S1C0>: Add axis */
static void	BpxBoutputNetIf( SERVO_CONTROL_IF*, SEQ_CTRL_OUT*, LONG, LONG );

extern void HALdrv_SetSOut( int axis, UINT x );
/****************************************************************************************************/


/****************************************************************************************************/
/*																									*/
/*		Scan B extended command input processing																	*/
/*																									*/
/****************************************************************************************************/
static void LpxNetScanBInputExtCommand( TRQ_LMT_DATA *TrqLimits, USHORT ExtSel, EXTENDED_DATA ExtData )
{
	switch ( ExtSel )
	{
	case	EXTCMD_TRQLMT_P :	/* Positive torque limit	*/
		TrqLimits->var.FwTrqLmt = ExtData.dwVal;
		break ;

	case	EXTCMD_TRQLMT_N :	/* Reverse side torque limit	*/
		TrqLimits->var.RvTrqLmt = ExtData.dwVal;
		break ;

	default					:	/* Others			*/
		break ;
	}
}

#if	(CSW_SVCF_FBLATCH == TRUE)
/****************************************************************************************************/
/*																									*/
/*		Latch request acquisition processing																			*/
/*																									*/
/****************************************************************************************************/
static void LpxLatchRequestGet( SERVO_CONTROL_IF *SvControlIf, LONG CmdLtBit )
{
	LONG	CmdLtEdge;

	CmdLtEdge = SvControlIf->CmdLtBit ^ CmdLtBit;
/*--------------------------------------------------------------------------------------------------*/
/*		C phase latch request detection processing(Change edge)															*/
/*--------------------------------------------------------------------------------------------------*/
	if( 0 != (CmdLtEdge & REQ_C_PHASE_BIT) )
	{/* Previous latch request≠Latch request for this time */
		SvControlIf->LtReqCphs = TRUE;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Ext1Latch request detection processing(Change edge)															*/
/*--------------------------------------------------------------------------------------------------*/
	if( 0 != (CmdLtEdge & REQ_EXT_SIG1_BIT) )
	{/* Previous latch request≠Latch request for this time */
		SvControlIf->LtReqExt1 = SvControlIf->ExtLtEnable[0];
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Ext2Latch request detection processing(Change edge)															*/
/*--------------------------------------------------------------------------------------------------*/
	if( 0 != (CmdLtEdge & REQ_EXT_SIG2_BIT) )
	{/* Previous latch request≠Latch request for this time */
		SvControlIf->LtReqExt2 = SvControlIf->ExtLtEnable[1];
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Ext3Latch request detection processing(Change edge)															*/
/*--------------------------------------------------------------------------------------------------*/
	if( 0 != (CmdLtEdge & REQ_EXT_SIG3_BIT) )
	{/* Previous latch request≠Latch request for this time */
		SvControlIf->LtReqExt3 = SvControlIf->ExtLtEnable[2];
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Save previous latch request																		*/
/*--------------------------------------------------------------------------------------------------*/
	SvControlIf->CmdLtBit = CmdLtBit;
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Communication monitor data setting																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Summary Omitted :	Monitor selection(RspSelMon), Monitor data(RspMonData)together.					*/
/*			For low-speed monitor data,NetIfCmdMonSel[].						*/
/*																									*/
/****************************************************************************************************/
static void LpxSetNetSelMonitor( LONG **MonTbl, LONG SelMon, LONG *RspSelMon, LONG *RspMonData )
{
/* <S1DE>:start */
//		*RspSelMon  = (UCHAR)SelMon&(0x4F);		 /* <S1D7>:Change number of elements */
//		*RspMonData = *(MonTbl[SelMon&(0x4F)]);	 /* <S1D7>:Change number of elements */

	if(SelMon < 0x50)
	{
		*RspSelMon  = (UCHAR)SelMon;
		*RspMonData = *(MonTbl[SelMon]);
	}
	else
	{
		*RspSelMon  = (UCHAR)SelMon;
		*RspMonData = 0;
	}
/* <S1DE>:end */}


/****************************************************************************************************/
/*																									*/
/*		Scan B network IF input processing															*/
/*																									*/
/****************************************************************************************************/
static void BpxBinputNetIf( BASE_CTRL_HNDL *BaseControls, SERVO_CONTROL_IF *SvControlIf,
																		TRQ_LMT_DATA *TrqLimits, LONG ax_no )
{
	SV_CYC_CMD	*CyclicCMD;
	SV_CYC_CTRL	*CyclicCTRL;

	CyclicCMD	= &( SvControlIf->CmdOptionIf.CycCmd );
	CyclicCTRL	= &( SvControlIf->CmdOptionIf.CycCtrl );

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Command data reception check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( CyclicCTRL->CmdUpdate == FALSE )
	{
		return;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Sequence signal input																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* From networkPCL/NCLSignal input */
	TrqLimits->var.PclSignal = (SvControlIf->CmdSeqBit >> ENB_EXTPTRQLMT_BITNO) & 0x1;
	TrqLimits->var.NclSignal = (SvControlIf->CmdSeqBit >> ENB_EXTNTRQLMT_BITNO) & 0x1;

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Bit signal input																		*/
/*--------------------------------------------------------------------------------------------------*/
	SvControlIf->CmdCtrl	= CyclicCMD->CmdCtrl;
	SvControlIf->CmdSeqBit	= CyclicCMD->CmdSeqBit;

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Scan B extension command input processing															*/
/*--------------------------------------------------------------------------------------------------*/
	LpxNetScanBInputExtCommand( TrqLimits, (USHORT)CyclicCTRL->ExtCmdSel1, CyclicCMD->ExtCmd1 );
	LpxNetScanBInputExtCommand( TrqLimits, (USHORT)CyclicCTRL->ExtCmdSel2, CyclicCMD->ExtCmd2 );

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Monitor selection command input processing																*/
/*--------------------------------------------------------------------------------------------------*/
	SvControlIf->MonSel.MonSel1 = CyclicCMD->SelMon1 ;
	SvControlIf->MonSel.MonSel2 = CyclicCMD->SelMon2 ;
	SvControlIf->MonSel.MonSel3 = CyclicCMD->SelMon3 ;
	SvControlIf->MonSel.MonSel4 = CyclicCMD->SelMon4 ;
	SvControlIf->MonSel.MonSel5 = CyclicCMD->SelMon5 ;
	SvControlIf->MonSel.MonSel6 = CyclicCMD->SelMon6 ;
	SvControlIf->MonSel.MonSel7 = CyclicCMD->SelMon7 ;
	SvControlIf->MonSel.MonSel8 = CyclicCMD->SelMon8 ;

/*--------------------------------------------------------------------------------------------------*/
/*		Communication:I/OOutput command																			*/
/*--------------------------------------------------------------------------------------------------*/
//#if (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE)						/* <S045> */
	//ulwk = (CyclicCMD->DOSignal) & 0x0000000F;
	//SvControlIf->NetSout = ( (((ulwk >> 0) & 1) << SOUT01)
	//				  	   | (((ulwk >> 1) & 1) << SOUT02)
	//				  	   | (((ulwk >> 2) & 1) << SOUT03) );
//#else																/* <S045> */
//	HALdrv_SetSOut ( 0, (CyclicCMD->DOSignal) & 0x0000000F );		/* Erase after */
	HALdrv_SetSOut ( (int)ax_no, (CyclicCMD->DOSignal) & 0x0000007F );		/* Erase after *//* <S04B> *//* <S1C0>:Add axis */
//#endif															/* <S045> */

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Gain number for gain switching																*/
/*--------------------------------------------------------------------------------------------------*/
	if(CyclicCMD->CmdCtrl.SelBank < SVCDEF_NETGSEL_SETNUM)
	{	BaseControls->GselNum = (LONG)CyclicCMD->CmdCtrl.SelBank;}	/* <S145> */

#if	(CSW_SVCF_FBLATCH == TRUE)
/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Latch selection bit signal(Cphase,Ext1,Ext2,Ext3)											*/
/*--------------------------------------------------------------------------------------------------*/
	LpxLatchRequestGet( SvControlIf, CyclicCMD->CmdLtBit);
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Control mode & servo control command input processing													*/
/*--------------------------------------------------------------------------------------------------*/
	switch( CyclicCMD->CmdCtrl.ModSel )
	{
	case NO_CTRL_MODE:	/*--- Control mode disabled ---------------------------------------------------*/
		SvControlIf->NetCtrlMcmd	= CTRLMODE_TRQ;
		SvControlIf->NetTrqRef		= 0;
		SvControlIf->NetSpdLmt		= 0x01000000;
		TrqLimits->var.FwTrqLmt	= NORMAXVALUE;
		TrqLimits->var.RvTrqLmt	= NORMAXVALUE;
		SvControlIf->NetSpdRef		= 0;
		SvControlIf->CmdSeqBit		&= ~(SERVO_ON_BIT);
		break;
	case POS_CTRL_MODE:	/*--- Position control mode ---------------------------------------------------*/
		SvControlIf->NetCtrlMcmd	= CTRLMODE_POS;
		SvControlIf->NetPosRef		= CyclicCMD->Position;
		SvControlIf->NetSpdFFC		= CyclicCMD->Speed;
		SvControlIf->NetTrqFFC		= CyclicCMD->Torque;
		SvControlIf->NetSpdRef		= CyclicCMD->Speed;
		break;
	case SPD_CTRL_MODE:	/*--- Speed control mode ---------------------------------------------------*/
		/* Note) Torque limit is set by expansion command. */
		SvControlIf->NetCtrlMcmd	= CTRLMODE_SPD;
		SvControlIf->NetSpdRef		= CyclicCMD->Speed;
		SvControlIf->NetTrqFFC		= CyclicCMD->Torque;
		break;
	case TRQ_CTRL_MODE:	/*--- Torque control mode -------------------------------------------------*/
		SvControlIf->NetCtrlMcmd	= CTRLMODE_TRQ;
		SvControlIf->NetTrqRef		= CyclicCMD->Torque;
		SvControlIf->NetSpdLmt		= CyclicCMD->Speed;
		TrqLimits->var.FwTrqLmt	= NORMAXVALUE;
		TrqLimits->var.RvTrqLmt	= NORMAXVALUE;
		SvControlIf->NetSpdRef		= CyclicCMD->Speed;
		break;
	default:	/*--------------- Default(Disabled)Mode ---------------------------------------*/
		SvControlIf->NetCtrlMcmd	= CTRLMODE_TRQ;
		SvControlIf->NetTrqRef		= 0;
		SvControlIf->NetSpdLmt		= 0;
		TrqLimits->var.FwTrqLmt	= NORMAXVALUE;
		TrqLimits->var.RvTrqLmt	= NORMAXVALUE;
		SvControlIf->CmdSeqBit		&= ~(SERVO_ON_BIT);
		SvControlIf->NetSpdRef		= 0;
		break;
	}

}


/****************************************************************************************************/
/*																									*/
/*		Scan B network IF output processing															*/
/*																									*/
/****************************************************************************************************/
static void BpxBoutputNetIf( SERVO_CONTROL_IF *SvControlIf, SEQ_CTRL_OUT *SeqCtrlOut,
																		LONG apos, LONG PosErr )
{
	SV_CYC_RSP			*SvCycRsp;
	LONG				**MonTbl;

	SvCycRsp	= &( SvControlIf->CmdOptionIf.CycRsp );

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Response data transmission check																*/
/*--------------------------------------------------------------------------------------------------*/
	if( SvControlIf->CmdOptionIf.CycCtrl.RspUpdate == FALSE )
	{
		return;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Control mode response																		*/
/*--------------------------------------------------------------------------------------------------*/
	SvCycRsp->RspCtrl.ExeMode = SvControlIf->CmdCtrl.ModSel;
	SvCycRsp->RspCtrl.RspCtrlBit =
		(USHORT)(SeqCtrlOut->RspCtrlBits | SvControlIf->RspCtrl.RspCtrlBit);

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Response bit signal output																	*/
/*--------------------------------------------------------------------------------------------------*/
	SvCycRsp->RspSeqBit	= SeqCtrlOut->RspSeqBits;
	SvCycRsp->RspLtBit	= SvControlIf->RspLtBit;
//	SvCycRsp->RspSigBit	= SvControlIf->RspSigBit;
	SvCycRsp->RspSigBit	= SeqCtrlOut->RspSigBits;

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Response numerical data output																	*/
/*--------------------------------------------------------------------------------------------------*/
	SvCycRsp->FbPosition = apos;
	SvCycRsp->PosError = PosErr;

/*--------------------------------------------------------------------------------------------------*/
/*		Communication: Monitor data output																		*/
/*--------------------------------------------------------------------------------------------------*/
	MonTbl = SvControlIf->MonRamTable;
	LpxSetNetSelMonitor( MonTbl, SvControlIf->MonSel.MonSel1,
							&SvCycRsp->SelMon1, &SvCycRsp->Mon1 );
	LpxSetNetSelMonitor( MonTbl, SvControlIf->MonSel.MonSel2,
							&SvCycRsp->SelMon2, &SvCycRsp->Mon2 );
	LpxSetNetSelMonitor( MonTbl, SvControlIf->MonSel.MonSel3,
							&SvCycRsp->SelMon3, &SvCycRsp->Mon3 );
	LpxSetNetSelMonitor( MonTbl, SvControlIf->MonSel.MonSel4,
							&SvCycRsp->SelMon4, &SvCycRsp->Mon4 );
	LpxSetNetSelMonitor( MonTbl, SvControlIf->MonSel.MonSel5,
							&SvCycRsp->SelMon5, &SvCycRsp->Mon5 );
	LpxSetNetSelMonitor( MonTbl, SvControlIf->MonSel.MonSel6,
							&SvCycRsp->SelMon6, &SvCycRsp->Mon6 );
	LpxSetNetSelMonitor( MonTbl, SvControlIf->MonSel.MonSel7,
							&SvCycRsp->SelMon7, &SvCycRsp->Mon7 );
	LpxSetNetSelMonitor( MonTbl, SvControlIf->MonSel.MonSel8,
							&SvCycRsp->SelMon8, &SvCycRsp->Mon8 );

}



/****************************************************************************************************/

/****************************************************************************************************/
/*																									*/
/*		Kernel Input Service for ScanB																*/
/*																									*/
/****************************************************************************************************//* <S04D>:Argument arrangement *//* <S086>*/
void BpiInputMotorPosition(BASE_CTRL_HNDL *BaseControls, ASIC_U_IF_STR *pAsicMicroReg,
											BPRMDAT *Bprm, PRMDATA *Prm, CTRL_LOOP_OUT *CtrlLoopOut)
{
	LONG				dMotPos, MotPos;
	LONG			dFencPos, FencPos;	/* <S04D> */
	MENCV 			*MencV, *FencV;		/* <S04D> */
	POS_MNG_HNDL		*PosManager;

	PosManager		= &(BaseControls->PosManager);
	MencV	= BaseControls->MencV;	/* <S04D> */
	FencV	= BaseControls->FencV;	/* <S04D> */
/*--------------------------------------------------------------------------------------------------*/
/*		Calculate Motor Encoder Data for Servo Control												*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* Linear type */
		BaseControls->MotSts.MotCphPass = LmxSencScanServiceB( MencV,
							 	 	 	 	 	 	 	 	&(BaseControls->CLatchClrReq) );
	}
	else
	{ /* Rotation type */
		BaseControls->MotSts.MotCphPass = RmxSencScanServiceB( MencV,
															&(BaseControls->CLatchClrReq) );
	}
	BaseControls->MotSts.MposFromCpos = Bprm->DirSign * MencV->MposFromCpos;

/*--------------------------------------------------------------------------------------------------*/
/*		FB position update																				*/
/*--------------------------------------------------------------------------------------------------*/
//	MotPos = MencV->MotPos;									/* Load MotorPosition					*//* <S038> */
	MotPos = Bprm->DirSign * MencV->MotPos;					/* Load MotorPosition					*/
	dMotPos = MotPos - BaseControls->MotSts.MotPos;			/* Cal. Delta Position					*/
	BaseControls->MotSts.MotPos += dMotPos;					/* Update Motor Position				*/
	BaseControls->MotSts.dMotPos = dMotPos;					/* Update Delta Motor Position			*/

/*--------------------------------------------------------------------------------------------------*/
/*		<S0E5> FB speed update (For sequence processing)														*/
/*--------------------------------------------------------------------------------------------------*/
	BaseControls->MotSts.MotSpdForSeq = MlibMulgain( dMotPos, Bprm->KmotspdB );

/*--------------------------------------------------------------------------------------------------*/
/*		Position FB difference input(Semi/FullSwitching), Motor absolute position update, Fulk absolute position update					*/
/*--------------------------------------------------------------------------------------------------*/
#if	(CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)/* <S04D> */
	if(Bprm->FencUse != FALSE)
	{
		/*------------------------------------------------------------------------------------------*/
		/*		FB position update																		*/
		/*------------------------------------------------------------------------------------------*/
		BaseControls->MotSts.FencCphPass = FencV->MotCphPass;	/* Set FencCphPass					*//* <S220> */
		FencPos = Bprm->DirSign * FencV->MotPos;								/* Load MotorPosition					*/
		dFencPos = FencPos - BaseControls->MotSts.FencPos;		/* Cal. Delta Position					*/
		BaseControls->MotSts.FencPos += dFencPos;				/* Update Motor Position				*/
		BaseControls->MotSts.dFencPos = dFencPos;				/* Update Delta Motor Position			*/

		MlibAposRg64dp( dFencPos, &(PosManager->conf.Egear), &(PosManager->FencPos), NULL );
#if (CSW_SVCF_BACKLASHCOMP == TRUE)
			MlibAposRg64dp( (dFencPos - PosManager->BlshCmp.var.BlshCmpAmountPls),		/* <S18B> */
							&(PosManager->conf.Egear),
							&(PosManager->PgPos),
							NULL );
#else
		PosManager->PgPos = PosManager->FencPos;										/* [Command unit]	*/
#endif/* (CSW_SVCF_BACKLASHCOMP == TRUE) */
		BaseControls->CtrlCmdMngr.dPosFbki = dFencPos;
		BaseControls->CtrlCmdMngr.PosFbki = BaseControls->MotSts.FencPos;
		BaseControls->CtrlCmdMngr.FbkPosFromCpos = BaseControls->MotSts.FposFromCpos;
	}
	else
#endif	/* (CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT) */
	{
		if( MencV->ReCalcAbsPosReq != FALSE )			/* Absolute position recalculation requested?				*/
		{
			/* Encoder initial position acquisition	*/
			/* RxPos*[0] -> RmxReadSencPositionhas already been obtained */
			/* SEN ON	*/
			PosMngSenOnReCalcAbsoPos( MencV,
			                          &(PosManager->MencPos),
			                          &(PosManager->conf.Egear),
			                          Prm->MencP.bit_dp,
			                          Prm->MencP.limmlt,
									  Bprm->RvsDir);
			MencV->ReCalcAbsPosReq = FALSE;
			PosManager->PgPos = PosManager->MencPos;								/* [Command unit]	*/
		}
		else
		{
			MlibAposRg64dp( dMotPos, &(PosManager->conf.Egear), &(PosManager->MencPos), NULL );
#if (CSW_SVCF_BACKLASHCOMP == TRUE)
			MlibAposRg64dp( (dMotPos - PosManager->BlshCmp.var.BlshCmpAmountPls),	/* <S18B> */
							&(PosManager->conf.Egear),
							&(PosManager->PgPos),
							NULL );
#else
			PosManager->PgPos = PosManager->MencPos;								/* [Command unit]	*/
#endif/* (CSW_SVCF_BACKLASHCOMP == TRUE) */
		}
		BaseControls->CtrlCmdMngr.dPosFbki = dMotPos;
		BaseControls->CtrlCmdMngr.PosFbki = BaseControls->MotSts.MotPos;
//		pBaseCtrl->CtrlCmdMngr.PosErr32s = CtrlLoopOut->MotPosErr;
		BaseControls->CtrlCmdMngr.FbkPosFromCpos = BaseControls->MotSts.MposFromCpos;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Motor speed FB calculation																			*/
/*--------------------------------------------------------------------------------------------------*/
#if	(CSW_SVCF_FULC_MODE == CSW_FULC_BASIC_SUPPORT)/* <S04D>:Addition of Fluke processing */
	if(Bprm->PsFulcSelect != FALSE)
	{
		BaseControls->CtrlCmdMngr.SpdFbki = FencV->MotSpd;
		BaseControls->CtrlCmdMngr.SpdObsFbki = FencV->MotSpd;
	}
	else
#endif/* (CSW_SVCF_BACKLASHCOMP == TRUE) */
	{
//		BaseControls->CtrlCmdMngr.SpdFbki = MencV->MotSpd;
//		BaseControls->CtrlCmdMngr.SpdObsFbki = MencV->MotSpd;
		BaseControls->CtrlCmdMngr.SpdFbki = CtrlLoopOut->MotSpd_a;
		BaseControls->CtrlCmdMngr.SpdObsFbki = CtrlLoopOut->MotSpd_a;/*<S086>*/
	}

}

/****************************************************************************************************/
/*																									*/
/*		Output Network monitor value																*/
/*																									*/
/****************************************************************************************************/
void BpiOutputNetStatus(BASE_CTRL_HNDL *BaseControls, SERVO_CONTROL_IF *SvControlIf,
                        	CTRL_LOOP_OUT *CtrlLoopOut, SEQ_CTRL_OUT *SeqCtrlOut, BPRMDAT *Bprm, PLSIFV *pPls)/* <S04D> *//* <S121> */
{
//	LONG				dPcmda;
	LONG				MotSpd;
	LONG				VPosErrKp;
	USHORT				RspCtrlBits;
	POS_MNG_HNDL		*PosManager;

	PosManager		= &(BaseControls->PosManager);

	/* For virtual position deviation compensationFBSpeed calculation */
	if(Bprm->PsFulcSelect != FALSE)/* <S04D>:Addition of Fluke processing */
	{/* Full-closed control	*/
#if (FLOAT_USE==TRUE)
		MotSpd = (BaseControls->MotSts.FencSpd + BaseControls->CtrlCmdMngr.SpdFbki) / 2.0f;
#else
		MotSpd = (BaseControls->MotSts.FencSpd + BaseControls->CtrlCmdMngr.SpdFbki) >> 1;
#endif
		BaseControls->MotSts.FencSpd = BaseControls->CtrlCmdMngr.SpdFbki;
		BaseControls->MotSts.MotSpd = BaseControls->MencV->MotSpd;
	}
	else
	{/* Semi-closed control	*/
#if (FLOAT_USE==TRUE)
	MotSpd = (BaseControls->MotSts.MotSpd + BaseControls->CtrlCmdMngr.SpdFbki) / 2.0f;
#else
	MotSpd = (BaseControls->MotSts.MotSpd + BaseControls->CtrlCmdMngr.SpdFbki) >> 1;
#endif
	BaseControls->MotSts.MotSpd = BaseControls->CtrlCmdMngr.SpdFbki;
	}
//<S0E5>	BaseControls->MotSts.MotSpdSumB += BaseControls->MotSts.MotSpd;
	BaseControls->MotSts.MotSpdSumB += BaseControls->MotSts.MotSpdForSeq;	/* <S0E5> */
	BaseControls->MotSts.MotSpdCntB++;

/*--------------------------------------------------------------------------------------------------*/
/*		Virtual position deviation update																	<S14C>	*/
/*--------------------------------------------------------------------------------------------------*/
	if(BaseControls->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS)
	{ /* Speed/During torque control */
		/* Virtual position loop gain acquisition */
		VPosErrKp = 1 ^ (SeqCtrlOut->OverTrvlSts & SeqCtrlOut->MotStop);
		VPosErrKp *= (BaseControls->GainChange.GselGains[0].VirtualPosKp);

		/* Virtual position deviation calculation  */
#if (FLOAT_USE==TRUE)
		PosManager->VirtualPositionError = MotSpd * VPosErrKp;
#else
		PosManager->VirtualPositionError = MlibMulgain32( MotSpd, VPosErrKp );
#endif /* FLOAT_USE */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Status update																				*/
/*--------------------------------------------------------------------------------------------------*/
	/*	Current limit flag : Base enable && μProgram current limit status	*/
//	RspCtrlBits = (SeqCtrlOut->BeComplete & CtrlLoopOut->TrqClamp) << TRQ_CLAMP_BITNO; /* <S04B> */
//	BaseControls->CtrlCmdMngr.TrqClamp = (SeqCtrlOut->BeComplete & CtrlLoopOut->TrqClamp);	/* <S045> Torque Limit *//* <S04B> */
	RspCtrlBits = BaseControls->CtrlCmdMngr.TrqClamp << TRQ_CLAMP_BITNO; /* <S04B> */
	/* Speed Clamp Status */
	//RspCtrlBits |= ( 0 != CtrlLoopOut->SpdClamp ) << SPD_CLAMP_BITNO;
	RspCtrlBits |= ( 0 != BaseControls->CtrlCmdMngr.SpdClamp ) << SPD_CLAMP_BITNO;/* @@ VLT status */
/*--------------------------------------------------------------------------------------------------*/
	RspCtrlBits |= ( 0 != BaseControls->PosCtrlSts.CoinSignal ) << COIN_BITNO;	/* @@ COIN status */
/*--------------------------------------------------------------------------------------------------*/
	RspCtrlBits |= ( 0 != BaseControls->PerClrEdge ) << PERCLREDGE_BITNO;		/* @@ CLREDGE status */

	/* @@ CLREDGE status (AP ONLY) */
/*--------------------------------------------------------------------------------------------------*/
	SvControlIf->RspCtrl.RspCtrlBit = RspCtrlBits;

#if !defined(CSW_NETIF_SYNQNET_TYPE)

#if	(CSW_SVCF_FBLATCH == TRUE)
/*--------------------------------------------------------------------------------------------------*/
/*		Position FB latch processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* Cphase, Ext1 Latch */
	PosMngLatchNetFbkPos( BaseControls, SvControlIf, Bprm );									/* <S04D> */
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Network monitor data update																*/
/*--------------------------------------------------------------------------------------------------*/
	BpxBoutputNetIf( SvControlIf,
					 SeqCtrlOut,
					 PosManager->PgPos.apos[0],
					 PosManager->var.PosErrA.per32s );
//					 pBaseCtrl->CtrlCmdMngr.PosErr32s );
#endif
}


/****************************************************************************************************/
/*																									*/
/*		Input ScanB Control command																	*/
/*																									*/
/****************************************************************************************************/
void BpiInputControlCMD( BASE_CTRL_HNDL *BaseControls, SERVO_CONTROL_IF *SvControlIf,
                         CTRL_LOOP_OUT *CtrlLoopOut, SEQ_CTRL_OUT *SeqCtrlOut, USHORT uwd_l, LONG ax_no )/* <S1C0>: Add axis */
{
	LONG				GainChgTriger;
	TBLCHK_SV_STS		TblDriveChkSts;
	CTRLMD				CtrlMode;
//	POS_MNG_HNDL		*PosManager;	//<S0C7>
//	LONG				lwk0, lwk1;		//<S0C7>

//	PosManager			= &(BaseControls->PosManager);//<S0C7>


/*--------------------------------------------------------------------------------------------------*/
/*		Current limit status acquisition																			*/
/*--------------------------------------------------------------------------------------------------*/
	/*	Current limit flag : Base enable && μProgram current limit status	*/
	BaseControls->CtrlCmdMngr.TrqClamp = (SeqCtrlOut->BeComplete & CtrlLoopOut->TrqClamp);			  /* <S04B> */

#ifndef _IO_SERVO_ON_
/*--------------------------------------------------------------------------------------------------*/
/*		Network command data input																	*/
/*--------------------------------------------------------------------------------------------------*/
#if !defined(CSW_NETIF_SYNQNET_TYPE)
	BpxBinputNetIf( BaseControls, SvControlIf, &(BaseControls->TrqLimitData), ax_no );/* <S1C0>: Add axis */
#endif
#endif /* _IO_SERVO_ON_ */

	/* Obtain positional integral clear command */
	BaseControls->CtrlCmdMngr.PicvClrCmd =
			( SvControlIf->CmdCtrl.CmdCtrlBit >> CLRPOSINTG_BITNO ) & 1;

	/* Acceleration data update for mode switch */
	BaseControls->ModeSwData.var.Acceleration = SeqCtrlOut->ModeSwAcc;

/*--------------------------------------------------------------------------------------------------*/
/*		Servo control mode management processing																	*/
/*--------------------------------------------------------------------------------------------------*/
	/* Control mode: Scan C command */
	CtrlMode.dw = SeqCtrlOut->CtrlModeReq.dw;

	/* Scan C control mode command check */
	if( SeqCtrlOut->CtrlModeReq.us.uwd_l == CTRLMODE_NOCMD )
	{
		CtrlMode.us.uwd_l = (USHORT)SvControlIf->NetCtrlMcmd;
	}
	else
	{
		SvControlIf->CmdReady = FALSE;
	}
	/*----------------------------------------------------------------------------------------------*/
	/* Previous control mode value */
	BaseControls->CtrlModeSet.CtrlModeLst.dw = BaseControls->CtrlModeSet.CtrlMode.dw;
	/* Control mode change information */
	BaseControls->CtrlModeSet.CtrlModeChg.dw = CtrlMode.dw ^ BaseControls->CtrlModeSet.CtrlMode.dw;
	/* Control mode current value */
	BaseControls->CtrlModeSet.CtrlMode.dw	 = CtrlMode.dw;

/*--------------------------------------------------------------------------------------------------*/
/*		Get Sequence Control Output																	*/
/*--------------------------------------------------------------------------------------------------*/
	BaseControls->BBRefClrReq = SeqCtrlOut->BBSvcRefClrReq;
	BaseControls->BeComplete = SeqCtrlOut->BeComplete;
	BaseControls->OverTravel = SeqCtrlOut->OverTrvlSts;			/* <S18B> */

#ifdef _VC_DEBUG_ /* 2012.09.11 Y.Oka ★Forced servo on Provisional measures★ */
	BaseControls->BeComplete = TRUE;
#endif /* 2012.09.11 Y.Oka ★Forced servo on Provisional measures★ */
	

/*--------------------------------------------------------------------------------------------------*/
/*		Servo gain switching process																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* Servo gain switching process */
	if( (BaseControls->GainChange.AtGsel == TRUE)
		&& (BaseControls->TuneLessCtrl.var.TuneLessAct == FALSE) 
		&& (SVFSEL_ATGAINCHANGE_USE == 1) )							/* <S09B> */
	{ /* Automatic gain switching && Adjustment-less invalidation */
		/* Switching judgment processing */
		GainChgTriger = GselDetGainChngTiming( BaseControls->PosCtrlSts.CoinSignal,
											   BaseControls->PosCtrlSts.NearSignal,
											   BaseControls->PosCtrlSts.RefZSignal,
											   BaseControls->GainChange.AtGselTrgA );
		/* Gain switching calculation processing */
//<S09B>	GselAutoGainChange( &(BaseControls->GainChange),
//							&(BaseControls->MFControl),
//							GainChgTriger,
//							&(BaseControls->CtrlCmdPrm) );
			GselAutoGainChange( BaseControls, GainChgTriger );	/* <S09B> */
	}
	else
	{ /* Manual gain switching(No gain switching) */
		/* Manual gain switching process */
//<S09B>		GselManualGainChange( &(BaseControls->GainChange),
//							  &(BaseControls->MFControl),
//							  BaseControls->GselNum,
//							  BaseControls->TuneLessCtrl.var.TuneLessAct,
//							  &(BaseControls->CtrlCmdPrm) );
			GselManualGainChange( BaseControls );					/* <S09B> */
		/* Automatic gain switching initialization */
//<S09B>	GselInitAutoGainChange(
//							&(BaseControls->GainChange.AtGselA),
//							&(BaseControls->GainChange.AtGselB) );
			GselRstAutoGainChange( &(BaseControls->GainChange) );	/* <S09B> */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Table operation execution process																		*/
/*--------------------------------------------------------------------------------------------------*/
//	TblDriveChkSts.AlmFlag = AxisRscB->AlmManager->AlmFlag;
	TblDriveChkSts.AlmFlag = (SeqCtrlOut->RspSeqBits >> ALM_DETECTED_BITNO) & 0x1;
	TblDriveChkSts.BeComplete = BaseControls->BeComplete;
	TblDriveChkSts.HwbbSts = SeqCtrlOut->HwbbSts;
	TblDriveChkSts.MainPowerOn = SeqCtrlOut->MainPowerOn;
	TblDriveChkSts.OverTrvlSts = SeqCtrlOut->OverTrvlSts;
//	TblDriveChkSts.TrqClamp = CtrlLoopOut->TrqClamp; /* <S04B> */
	TblDriveChkSts.TrqClamp = BaseControls->CtrlCmdMngr.TrqClamp; /* <S04B> */
	TblDriveChkSts.dMotPos = BaseControls->MotSts.dMotPos;
	TblDriveExecute( &(BaseControls->TblDrive),
					 &TblDriveChkSts,
					 &(BaseControls->PosManager),
					 &(BaseControls->MFControl) );

/*--------------------------------------------------------------------------------------------------*/
/*		Zero control mode command processing: ScanBCommands from common input processing										*/
/*--------------------------------------------------------------------------------------------------*/
	if( (BaseControls->TblDrive.var.TblCmd == TBLCMD_NOCMD)
		|| (BaseControls->CtrlModeSet.CtrlMode.b.zm != CTRLMODE_ZNONE) )
	{ /* No table operation || ScanBCommon input processing command available */
		BaseControls->CtrlModeSet.ZctrlMode.zm    = BaseControls->CtrlModeSet.CtrlMode.b.zm;
		BaseControls->CtrlModeSet.ZctrlMode.zmchg = BaseControls->CtrlModeSet.CtrlModeChg.b.zm;
	}
	else
	{
		BaseControls->CtrlModeSet.ZctrlMode.zm    = BaseControls->TblDrive.var.TblZmd.b.zm;
		BaseControls->CtrlModeSet.ZctrlMode.zmchg = BaseControls->TblDrive.var.TblZmd.b.zmchg;
	}

	/* Extension control output variable zero clear */
	BaseControls->CtrlCmdMngr.SpdFFCx = 0;		/* Extension control: SpeedFFCompensation				*/
	BaseControls->CtrlCmdMngr.SpdFBCx = 0;		/* Extension control: SpeedFBCompensation				*/
	BaseControls->CtrlCmdMngr.TrqFFCx = 0;		/* Extension control: TorqueFFCompensation			*/
	BaseControls->CtrlCmdMngr.TrqFBCx = 0;		/* Extension control: TorqueFBCompensation			*/

}



/****************************************************************************************************/
/*																									*/
/*		Yaskawa servo control processing main																	*/
/*																									*/
/****************************************************************************************************/
void BpiYecServoControl( BASE_LOOPCTRLS *BaseLoops, SERVO_CONTROL_IF *SvControlIf,
							CTRL_LOOP_OUT *CtrlLoopOut, FUN_CMN_CONTROL *FnCmnCtrl, BPRMDAT *Bprm )/* <S04D> */
{
	BASE_CTRL_HNDL		*BaseControls;
	POS_MNG_HNDL		*PosManager;
	MFCTRL				*MFControl;
	LONG				SpdFFx;
	KSGAIN				lwk;		/*<S00A>*/
	CMD_CTRLBITS		*CmdCtrl;			/* <S1D0> */

	/* Local variable initialization */
	BaseControls= BaseLoops->BaseCtrls;
	PosManager	= &(BaseControls->PosManager);
	MFControl	= &(BaseControls->MFControl);
	CmdCtrl = &(SvControlIf->CmdOptionIf.CycCmd.CmdCtrl);		/* <S1D0> */

/*--------------------------------------------------------------------------------------------------*/
/*		Motor speed FB calculation																			*/
/*--------------------------------------------------------------------------------------------------*/
//	pBaseCtrl->CtrlCmdMngr.SpdFbki		= pBaseCtrl->MotSts.MotSpd;
//	pBaseCtrl->CtrlCmdMngr.SpdObsFbki	= pBaseCtrl->MotSts.MotSpd;

/*--------------------------------------------------------------------------------------------------*/
/*		Position command management																				*/
/*																									*/
/*		Servo basic controlScanA.															*/
/*		The position command updated hereScanA, it becomes the input for servo basic control					*/
/*		Update of position command125us62.5usBe sure to perform the following.										*/
/*		For details, refer to the separate specifications. Specification No.:○○○○○○○○○○								*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/

	/* Torque command filter integral initialization request clear */
	BaseControls->CtrlCmdMngr.TrqFilClrCmd = FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Branching by control mode																		*/
/*--------------------------------------------------------------------------------------------------*/
	switch( BaseControls->CtrlModeSet.CtrlMode.b.cm )
	{
/****************************************************************************************************/
	case CTRLMODE_POS:	/* Position control mode															*/
/****************************************************************************************************/
	/*----------------------------------------------------------------------------------------------*/
	/*		Processing when switching control(Speed control or Torque control ===> Position control)							*/
	/*----------------------------------------------------------------------------------------------*/
		switch(BaseControls->CtrlModeSet.CtrlModeLst.b.cm)
		{
		case CTRLMODE_TRQ:
			if( BaseControls->DobsCtrl.var.DobsAct != FALSE)
			{ /* Disturbance observer enabled -> Torque command filter integral initialization */
				BaseControls->CtrlCmdMngr.TrqFilClrCmd = TRUE;
			}

			/* Adjustment-less request setting */
			if( BaseControls->TuneLessCtrl.var.TuneLessInvldReq == FALSE )
			{
				BaseControls->TuneLessCtrl.var.TuneLessAct = BaseControls->TuneLessCtrl.conf.TuningLessUse;
			}
			/* Speed command/SpeedFFPhase lead compensation filter initializationμExecuted by program */
			/* breakNone, continue down */

		case CTRLMODE_SPD:		/* Change due to review related to position deviation <S14C> */
			/* Speed command filter output initialization */
			BaseControls->CtrlCmdMngr.SpdRfFilo = 0;

			/* Create reference position by switching control mode */
			lwk = MlibPcmdEgear( PosManager->VirtualPositionError,
								 &(PosManager->conf.Egear),
								 &(PosManager->var.dPcmdEgrem) );
#if 0/* SigamV-MD Micro	*/								/* <S007> */
			BaseControls->CtrlCmdMngr.BaseFbkPos = lwk + BaseControls->CtrlCmdMngr.PosFbki;
#else/* Sigam7 ScanA.	*/							/* <S007> */
			PosManager->var.Per64[0] = 0;
			PosManager->var.Per64[1] = 0;
			MlibPerrcalx(lwk, 0, PosManager->var.Per64);
#endif
			MlibPerrcala( PosManager->VirtualPositionError, 0,
							&PosManager->conf.Egear, &PosManager->var.PosErrA );
			/* Initialize current command position */
			BaseControls->CtrlCmdMngr.PosRefLo = PosManager->VirtualPositionError + PosManager->PgPos.apos[0];

			/* Auto notch frequency analysis forced disable processing scheduled to be added */
			break;

		default:
			break;
		}

	/*----------------------------------------------------------------------------------------------*/
	/*		Position command generation																			*/
	/*----------------------------------------------------------------------------------------------*/
		/* Servo communication position command management processing */
		PosManager->dPosRefi = PosMngNetPosCmdManager( BaseLoops, SvControlIf, FnCmnCtrl, Bprm );  /* <S04D> *//* <S187> */

	/*----------------------------------------------------------------------------------------------*/
	/*		SpeedFFCompensation/TorqueFFCompensation generation																*/
	/*----------------------------------------------------------------------------------------------*/
		/* Position commandFFOperation --> pBaseCtrl->CtrlCmdMngr.SpdFFCx */
		if( MFControl->var.CurMFCModel == 0 )
		{ /* MFCDisabled -> MFCWhen enabledFFDisable function */
			/* Position commandFFOperation */
#if (FLOAT_USE==TRUE)
			SpdFFx = (float)PosManager->dPosRefi * BaseControls->CtrlCmdPrm.KdPff;
#else
			SpdFFx = MlibMulgain( PosManager->dPosRefi, BaseControls->CtrlCmdPrm.KdPff );
#endif /* FLOAT_USE */

#if (FLOAT_USE==TRUE)
			/* Primary low-pass filter calculation */
			BaseControls->CtrlCmdMngr.dPFfFilo = FlibLpfilter1(SpdFFx,
														BaseControls->CtrlCmdPrm.KdPffFil,
														BaseControls->CtrlCmdMngr.dPFfFilo );
#else
			/* Primary low-pass filter calculation */
			BaseControls->CtrlCmdMngr.dPFfFilo = MlibLpfilter1(SpdFFx,
														BaseControls->CtrlCmdPrm.KdPffFil,
														BaseControls->CtrlCmdMngr.dPFfFilo );
#endif /* FLOAT_USE */

			/* SpeedFFCompensation [2^24/OvrSpd] */
			BaseControls->CtrlCmdMngr.SpdFFCx += BaseControls->CtrlCmdMngr.dPFfFilo;
		}
		if( (BaseControls->TblDrive.var.TblCmd == TBLCMD_NOCMD)
			&& ((MFControl->var.CurMFCModel == 0) || (MFControl->conf.MFCFF == TRUE)) )
		{ /* Other than table operation && (MFCDisabled || MFCinside/outsideFFEnabled) */
			/* Speed feedforward command moving average filter processing main function		*/			/* <S1D0> */
			SvControlIf->NetSpdFFC = FFcmdFilMoveAverageFilter( &(BaseControls->VFFcmdFil),
																SvControlIf->NetSpdFFC,
																CmdCtrl->vffFilSts);
			BaseControls->CtrlCmdMngr.SpdRfFilo = SvControlIf->NetSpdFFC;
			/* SpeedFFCommand addition */
			BaseControls->CtrlCmdMngr.SpdFFCx += BaseControls->CtrlCmdMngr.SpdRfFilo;
			/* 	Torque feedforward command moving average filter processing main function		*/			/* <S1D0> */
			SvControlIf->NetTrqFFC = FFcmdFilMoveAverageFilter( &(BaseControls->TFFcmdFil),
																SvControlIf->NetTrqFFC,
																CmdCtrl->tffFilSts);
			/* TorqueFFCommand addition */
			BaseControls->CtrlCmdMngr.TrqFFCx += SvControlIf->NetTrqFFC;
		}

	/*----------------------------------------------------------------------------------------------*/
	/*		Get Speed Reference for Monitor															*/
	/*----------------------------------------------------------------------------------------------*/
		BaseControls->CtrlCmdMngr.SpdRefi = CtrlLoopOut->SpdRefo;
		BaseControls->CtrlCmdMngr.SpdRefo = CtrlLoopOut->SpdRefo;

	/*------------------------------------------------------------------------------------------*/
	/*		Offline inertia identification															*/
	/*------------------------------------------------------------------------------------------*/
		BpxJatOfflineCalc(	&BaseControls->JatHdl, 
							&BaseControls->PosManager.conf.Egear,
							&BaseControls->CtrlCmdPrm,
							&BaseControls->CtrlCmdMngr,
							BaseLoops->TrqRefo_a );									/* <S160> */
//<S0BA>
#if 0
	/*----------------------------------------------------------------------------------------------*/
	/*		Mode switch calculation																		*/
	/*----------------------------------------------------------------------------------------------*/
		ModeSwitch( &(BaseControls->ModeSwData),
					CtrlLoopOut->TrqRefo,
					BaseControls->CtrlCmdMngr.SpdRefo,
//					(CtrlLoopOut->SpdRefo + CtrlLoopOut->SpdFFC),
					BaseControls->PosManager.PosErrA.per32s );
#endif
	/*----------------------------------------------------------------------------------------------*/
	/*		Get Speed Reference for Monitor															*/
	/*----------------------------------------------------------------------------------------------*/
		BaseControls->CtrlCmdMngr.SpdRefo = CtrlLoopOut->SpdRefo;
/* <S1F5> Start */
	/*----------------------------------------------------------------------------------------------*/
	/*		Save speed value for linear deceleration stop																*/
	/*----------------------------------------------------------------------------------------------*/
		if( BaseControls->CtrlModeSet.ZctrlMode.zm != CTRLMODE_LDSTOP )
		{
			BaseControls->CtrlCmdMngr.SpdLdstpRefo = BaseControls->CtrlCmdMngr.SpdRefo;
		}
/* <S1F5> End */

		break;


/****************************************************************************************************/
	case CTRLMODE_SPD:	/* Speed control mode															*/
/****************************************************************************************************/
	/*----------------------------------------------------------------------------------------------*/
	/*		Processing when switching control(Position control or Torque control ===> Speed control)							*/
	/*----------------------------------------------------------------------------------------------*/
		switch(BaseControls->CtrlModeSet.CtrlModeLst.b.cm)
		{
		case CTRLMODE_TRQ:
			if( BaseControls->DobsCtrl.var.DobsAct != FALSE)
			{ /* Disturbance observer enabled -> Torque command filter integral initialization */
				BaseControls->CtrlCmdMngr.TrqFilClrCmd = TRUE;
			}

			/* Adjustment-less request setting */
			if( BaseControls->TuneLessCtrl.var.TuneLessInvldReq == FALSE )
			{
				BaseControls->TuneLessCtrl.var.TuneLessAct = BaseControls->TuneLessCtrl.conf.TuningLessUse;
			}

			/* Shock reduction processing when switching control mode */
			lwk = SvControlIf->NetSpdRef - BaseControls->CtrlCmdMngr.SpdObsFbki;
#if (FLOAT_USE==TRUE)
			lwk = FlibIntegral( lwk,
								BaseControls->GainChange.GselGains[0].Kvi,
								BaseControls->CtrlCmdMngr.VelIvar64 );
#else
			lwk = MlibIntegral( lwk,
								BaseControls->GainChange.GselGains[0].Kvi,
								BaseControls->CtrlCmdMngr.VelIvar64 );
#endif /* FLOAT_USE */

			/* Speed command/SpeedFFPhase lead compensation filter initializationμExecuted by program */
			/* breakNone, continue down */

		case CTRLMODE_POS:
			/* Speed commandfilterOutput = SpeedFB, Speed commandSFSOutput = SpeedFB */
			BaseControls->CtrlCmdMngr.SpdRfFilo = BaseControls->CtrlCmdMngr.SpdFbki;
			BaseControls->CtrlCmdMngr.SpdRefo = BaseControls->CtrlCmdMngr.SpdFbki;
			/* Auto notch frequency analysis forced disable processing scheduled to be added */
			break;

		default:
			break;
		}

	/*----------------------------------------------------------------------------------------------*/
	/*		Speed command generation																			*/
	/*----------------------------------------------------------------------------------------------*/
		if( BaseControls->TblDrive.var.TblCmd != TBLCMD_NOCMD )
		{ /* During table operation */
			/* Table operation speed command setting */
			BaseControls->CtrlCmdMngr.SpdRfFilo = 0;
			BaseControls->CtrlCmdMngr.SpdRefi = BaseControls->TblDrive.var.SpdRef;
			BaseControls->VrefInSts = FALSE; /* <S0A1> */
		}
		else
		{ /* During normal operation */
			/* Command input status setting */
			BaseControls->VrefInSts = (SvControlIf->NetSpdRef != 0) ? TRUE : FALSE; /* <S0A1> */

#if (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE) /* ★Interim To be common processing★ */
			/* Primary low-pass filter calculation */
			BaseControls->CtrlCmdMngr.SpdRfFilo = MlibLpfilter1(
													SvControlIf->NetSpdRef,
													BaseControls->CtrlCmdPrm.KVrfFil,
													BaseControls->CtrlCmdMngr.SpdRfFilo );
#else /* (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE) */
			/* Primary low-pass filter was applied on the communication side */
			BaseControls->CtrlCmdMngr.SpdRfFilo = SvControlIf->NetSpdRef;

#endif /* (CSW_NETIF_TYPE != NETIF_ANLGPLS_TYPE) */

			/* Speed command acquisition */
			BaseControls->CtrlCmdMngr.SpdRefi = BaseControls->CtrlCmdMngr.SpdRfFilo;
			/* 	Torque feedforward command moving average filter processing main function		*/			/* <S1D0> */
			SvControlIf->NetTrqFFC = FFcmdFilMoveAverageFilter( &(BaseControls->TFFcmdFil),
																SvControlIf->NetTrqFFC,
																CmdCtrl->tffFilSts);
			/* TorqueFFCompensation generation */
			BaseControls->CtrlCmdMngr.TrqFFCx += SvControlIf->NetTrqFFC;
		}

	/*----------------------------------------------------------------------------------------------*/
	/*		Soft start processing																		*/
	/*----------------------------------------------------------------------------------------------*/
		BaseControls->CtrlCmdMngr.SpdRefo = BaseSoftStartSpdRef(
												&(BaseControls->CtrlCmdPrm),
												BaseControls->CtrlCmdMngr.SpdRefi,
												BaseControls->CtrlCmdMngr.SpdRefo );
//<S0BA>
/* <S1F5> Start*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Save speed value for linear deceleration stop																*/
	/*----------------------------------------------------------------------------------------------*/
		if( BaseControls->CtrlModeSet.ZctrlMode.zm != CTRLMODE_LDSTOP )
		{
			BaseControls->CtrlCmdMngr.SpdLdstpRefo = BaseControls->CtrlCmdMngr.SpdRefo;
		}
/* <S1F5> End */
#if 0
	/*----------------------------------------------------------------------------------------------*/
	/*		Mode switch calculation																		*/
	/*----------------------------------------------------------------------------------------------*/
		ModeSwitch( &(BaseControls->ModeSwData),
					CtrlLoopOut->TrqRefo,
					BaseControls->CtrlCmdMngr.SpdRefo,
					BaseControls->PosManager.PosErrA.per32s );
#endif
		break;


/****************************************************************************************************/
	case CTRLMODE_TRQ:	/* Torque control mode  (Execute speed control for zero speed stop/zero clamp)			*/
/****************************************************************************************************/
	/*----------------------------------------------------------------------------------------------*/
	/*		Torque command generation																			*/
	/*----------------------------------------------------------------------------------------------*/
		/* Command input status setting */
		BaseControls->TrefInSts = (SvControlIf->NetTrqRef != 0) ? TRUE : FALSE; /* <S0A1> */
		/* Torque command acquisition */
		BaseControls->CtrlCmdMngr.TrqRefo = SvControlIf->NetTrqRef;
		/* Low pass filter processing */
		BaseControls->CtrlCmdMngr.SpdRfFilo = MlibLpfilter1(
												SvControlIf->NetSpdLmt,
												BaseControls->CtrlCmdPrm.KVrfFil,
												BaseControls->CtrlCmdMngr.SpdRfFilo );
		/* No adjustment during torque controlOFFSetting */
		BaseControls->TuneLessCtrl.var.TuneLessAct = FALSE;

	/*----------------------------------------------------------------------------------------------*/
	/*		Speed limit calculation during torque control																*/
	/*----------------------------------------------------------------------------------------------*/
		BaseControls->CtrlCmdMngr.OverTrq = OverTorqueCalculation(
												&(BaseControls->CtrlCmdMngr),
												&(BaseControls->CtrlCmdPrm) );
		break;


/****************************************************************************************************/
	case CTRLMODE_JOG:	/* JOG operation mode															*/
/****************************************************************************************************/
		SvControlIf->NetSpdRef = FnCmnCtrl->JogSpeed;
		BaseControls->CtrlCmdMngr.SpdRefi = SvControlIf->NetSpdRef;

		/* Restoring the adjustment-less function switch <S0DA> */
		BaseControls->TuneLessCtrl.var.TuneLessAct = BaseControls->TuneLessCtrl.conf.TuningLessUse;

		/* Soft start processing */
		BaseControls->CtrlCmdMngr.SpdRefo = BaseSoftStartSpdRef(
												&(BaseControls->CtrlCmdPrm),
												BaseControls->CtrlCmdMngr.SpdRefi,
												BaseControls->CtrlCmdMngr.SpdRefo );
//<S0BA>
/* <S1F5> Start */
	/*----------------------------------------------------------------------------------------------*/
	/*		Save speed value for linear deceleration stop																*/
	/*----------------------------------------------------------------------------------------------*/
		if( BaseControls->CtrlModeSet.ZctrlMode.zm != CTRLMODE_LDSTOP )
		{
			BaseControls->CtrlCmdMngr.SpdLdstpRefo = BaseControls->CtrlCmdMngr.SpdRefo;
		}
/* <S1F5> End */
#if 0
	/*----------------------------------------------------------------------------------------------*/
	/*		Mode switch calculation																		*/
	/*----------------------------------------------------------------------------------------------*/
		ModeSwitch( &(BaseControls->ModeSwData),
					CtrlLoopOut->TrqRefo,
					BaseControls->CtrlCmdMngr.SpdRefo,
					BaseControls->PosManager.PosErrA.per32s );
#endif
		break;


/****************************************************************************************************/
	case CTRLMODE_ZSRCH:/* Origin search																*/
/****************************************************************************************************/
		/*	Initializing variables : When changing control mode for the first time */
		if( (0 != BaseControls->CtrlModeSet.CtrlModeChg.b.cm)
			|| (FALSE == BaseControls->BeComplete) )
		{
			BaseControls->ZeroClampCtrl.var.PosError = 0;
			BaseControls->ZeroClampCtrl.var.OrgSrchFinish = FALSE;
		}

		/* Restoring the adjustment-less function switch <S0DA> */
		BaseControls->TuneLessCtrl.var.TuneLessAct = BaseControls->TuneLessCtrl.conf.TuningLessUse;

		BaseControls->CtrlCmdMngr.SpdRefi = ZeroSearchControl(
											&(BaseControls->ZeroClampCtrl),
											&(BaseControls->MotSts),
											BaseControls->GainChange.GselGains[0].ZcKp,
											FnCmnCtrl->JogSpeed );
		BaseControls->CtrlCmdMngr.SpdRefo = BaseControls->CtrlCmdMngr.SpdRefi;

//<S0BA>
/* <S1F5> Start */
	/*----------------------------------------------------------------------------------------------*/
	/*		Save speed value for linear deceleration stop																*/
	/*----------------------------------------------------------------------------------------------*/
		if( BaseControls->CtrlModeSet.ZctrlMode.zm != CTRLMODE_LDSTOP )
		{
			BaseControls->CtrlCmdMngr.SpdLdstpRefo = BaseControls->CtrlCmdMngr.SpdRefo;
		}
/* <S1F5> End */
#if 0
	/*----------------------------------------------------------------------------------------------*/
	/*		Mode switch calculation																		*/
	/*----------------------------------------------------------------------------------------------*/
		ModeSwitch( &(BaseControls->ModeSwData),
					CtrlLoopOut->TrqRefo,
					BaseControls->CtrlCmdMngr.SpdRefo,
					BaseControls->PosManager.PosErrA.per32s );
#endif
		break;


/****************************************************************************************************/
	default:			/* This case does not exist															*/
/****************************************************************************************************/
		break;
	}

#if 0
/*--------------------------------------------------------------------------------------------------*/
/*		Update Speed Clamp Status																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( 0 != hTrqCtrl->var.SpdClamp )
	{
		SvControlIf->RspCtrl.RspCtrlBit |= SPD_CLAMP_BITMSK;
	}
	else
	{
		SvControlIf->RspCtrl.RspCtrlBit &= ~(SPD_CLAMP_BITMSK);
	}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Position command update																				*/
/*--------------------------------------------------------------------------------------------------*/
	/* Position command update processing */
#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
	BaseControls->CtrlCmdMngr.dPosRefo = dPosRefo;
#else /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
	PosRefRenewal( PosManager, BaseControls->CtrlModeSet.CtrlMode );	/* <S187> */
#endif /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */

	BaseControls->CtrlCmdMngr.PosRefLo = SvControlIf->NetPosRef;

/*--------------------------------------------------------------------------------------------------*/
/*		Initialize position control variables when other than position control mode and zero speed stop mode								*/
/*--------------------------------------------------------------------------------------------------*/
	if( (BaseControls->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS) ||
		(BaseControls->CtrlModeSet.ZctrlMode.zm == CTRLMODE_ZSTOP) ||  /* <S1F5> */
		(BaseControls->CtrlModeSet.ZctrlMode.zm == CTRLMODE_LDSTOP) )   /* <S1F5> */
	{
		/* Position control variable initialization <S14C> */
		InitBasePositionControl( &(BaseLoops->PosCtrl) );
	}
/* <S1F5> Start */

	/*----------------------------------------------------------------------------------------------*/
	/*		Linear deceleration stop speed command calculation																*/
	/*----------------------------------------------------------------------------------------------*/
	if( BaseControls->CtrlModeSet.ZctrlMode.zm == CTRLMODE_LDSTOP )
	{
		BaseControls->CtrlCmdMngr.SpdLdstpRefo = BaseLinDecStopSpdRef( &(BaseControls->CtrlCmdPrm),
															BaseControls->CtrlCmdMngr.SpdLdstpRefo );
	}
/* <S1F5> End */

	BaseControls->CtrlCmdMngr.CtrlMode = BaseControls->CtrlModeSet.CtrlMode.b.cm;

}


/* <S1F5> Start */
/*****************************************************************************
  Description: Deceleration stop speed command calculation by linear deceleration
------------------------------------------------------------------------------
  Parameters:
	SpdRefo;			Speed control: Speed command SFS output
	SpdLinZeroStopDec;	Linear deceleration stop deceleration amount
  Return:
	SpdRefx				Speed command
------------------------------------------------------------------------------
  Note:
    Speed command soft start calculation main functionBaseSoftStartSpdRef
*****************************************************************************/
static LONG	BaseLinDecStopSpdRef( CTRL_CMD_PRM *CtrlCmdPrm, LONG SpdRefo )
{
	if( SpdRefo > 0 )
	{
		SpdRefo = SpdRefo - CtrlCmdPrm->SpdLdstpDec;
		if( SpdRefo < 0 )
		{
			SpdRefo = 0;
		}
	}
	else if( SpdRefo < 0 )
	{
		SpdRefo = SpdRefo + CtrlCmdPrm->SpdLdstpDec;
		if( SpdRefo > 0 )
		{
			SpdRefo = 0;
		}
	}
	else
	{
		SpdRefo = 0;
	}
	
	return( SpdRefo );
}
/* <S1F5> End */


/****************************************************************************************************/
/*																									*/
/*		Position control variable initialization																	<S14C>	*/
/*																									*/
/****************************************************************************************************/
void InitBasePositionControl( POSCTRL *PosCtrl )
{

	MlibResetLongMemory( &(PosCtrl->V), sizeof(PosCtrl->V)/4 );

}

#if	0
#define NRFMAX	16014615
/****************************************************************************************************/
/*																									*/
/*		Position control calculation	Unused																		*/
/*																									*/
/****************************************************************************************************/
LONG BpiBasePositionControl( POS_LOOP_CTRL *PosLoop, CTRL_CMD_MNG *CtrlCmd,
							 POS_MNG_HNDL *PosManager, KSGAIN Kp, KSGAIN Kpi )
{
	LONG	PosErr;
	LONG	PacOut, IacOut;
	LONG	SpdRefx;

	if(CtrlCmd->CtrlMode != CTRLMODE_POS)
	{
		PosLoop->var.PerIvar64[0] = 0;
		PosLoop->var.PerIvar64[1] = 0;
		PosLoop->var.Per64[0] = 0;
		PosLoop->var.Per64[1] = 0;
//		CtrlCmd->PosErr32s = 0;
		PosManager->PosErrA.per32s = 0;
		PosLoop->var.LoopStart = FALSE;
		return( CtrlCmd->SpdRefo );
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation clear processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( CtrlCmd->PerrClrReq != FALSE )
	{
		PosLoop->var.PerIvar64[0] = 0;
		PosLoop->var.PerIvar64[1] = 0;
		PosLoop->var.Per64[0] = 0;
		PosLoop->var.Per64[1] = 0;
		CtrlCmd->SpdRefo = 0;
		CtrlCmd->PosErr32s = 0;
		PosManager->PosErrA.per32s = 0;
		return( 0 );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position control integral clear processing																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( CtrlCmd->PicvClrCmd != FALSE )
	{
		PosLoop->var.PerIvar64[0] = 0;
		PosLoop->var.PerIvar64[1] = 0;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position control: Position deviation calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( PosLoop->var.LoopStart == FALSE )
	{
		PosLoop->var.LoopStart = TRUE;
#if (FLOAT_USE==TRUE)
		PosErr = FlibPerrcalx( CtrlCmd->BaseFbkPos + CtrlCmd->dPosRefo,
		                       CtrlCmd->PosFbki, PosLoop->var.Per64 );
		PosManager->PosErrA.per32s += (PosManager->var.dPcmda);
	}
	else
	{
		PosErr = FlibPerrcalx( CtrlCmd->dPosRefo, CtrlCmd->dPosFbki, PosLoop->var.Per64 );
		PosManager->PosErrA.per32s += (PosManager->var.dPcmda - PosManager->PgPos.dposix);
	}
#else
		PosErr = MlibPerrcalx( CtrlCmd->BaseFbkPos + CtrlCmd->dPosRefo,
		                       CtrlCmd->PosFbki, PosLoop->var.Per64 );
		PosManager->PosErrA.per32s += (PosManager->var.dPcmda);
	}
	else
	{
		PosErr = MlibPerrcalx( CtrlCmd->dPosRefo, CtrlCmd->dPosFbki, PosLoop->var.Per64 );
		PosManager->PosErrA.per32s += (PosManager->var.dPcmda - PosManager->PgPos.dposix);
	}
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
/*		Position control: Proportional control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	PacOut = ( (float)PosErr * Kp );
#else
	PacOut = MlibMulgain( PosErr, Kp );
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
/*		Position control: Integral control calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( Kpi == 0 )
	{
		IacOut = ((PosLoop->var.PerIvar64[1] + 1) >> 1);
	}
	else
	{
		IacOut = MlibIntegral( PacOut, Kpi, PosLoop->var.PerIvar64 );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position control: PI control output																	*/
/*--------------------------------------------------------------------------------------------------*/
	SpdRefx = PacOut + IacOut;

/*--------------------------------------------------------------------------------------------------*/
/*		Speed bias addition calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( SpdRefx == 0 )
	{
		if( PosErr > 0 )
		{
			SpdRefx =  1;
		}
		else if( PosErr < 0 )
		{
			SpdRefx = -1;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position control output limit																		*/
/*--------------------------------------------------------------------------------------------------*/
	else if( SpdRefx > NRFMAX )
	{
		SpdRefx =  NRFMAX;
	}
	else if( SpdRefx < -NRFMAX )
	{
		SpdRefx = -NRFMAX;
	}

	CtrlCmd->SpdRefo = SpdRefx;
	CtrlCmd->PosErr32s = PosErr;

	return SpdRefx;
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Position command management processing during magnetic pole detection control																*/
/*																									*/
/****************************************************************************************************/
void	BpiMpoleFindMakePosCmd( BASE_LOOPCTRLS *BaseLoops, SERVO_CONTROL_IF *SvControlIf,
													FUN_CMN_CONTROL *FnCmnCtrl, BPRMDAT *Bprm)
{
	BASE_CTRL_HNDL *BaseControls;
	LONG	dPosRefo;
	LONG	lwk;

	/* Local variable initialization */
	BaseControls = BaseLoops->BaseCtrls;	
	dPosRefo = 0;

	/* Speed FB calculation -> Motor differential speed fixed */
//	pBaseCtrl->CtrlCmdMngr.SpdFbki = pBaseCtrl->MotSts.MotSpd;

	if( BaseControls->CtrlModeSet.CtrlMode.b.cm == CTRLMODE_POS )
	{
		if( BaseControls->CtrlModeSet.CtrlModeLst.b.cm != CTRLMODE_POS )
		{
			/* Create reference position by switching control mode */
			/* Change due to review related to position deviation <S14C> */
			lwk = MlibPcmdEgear( BaseControls->PosManager.VirtualPositionError,
								 &(BaseControls->PosManager.conf.Egear),
								 &(BaseControls->PosManager.var.dPcmdEgrem) );
#if 0/* SigamV-MD Micro	*/								/* <S02F> */
			BaseControls->CtrlCmdMngr.BaseFbkPos = lwk + BaseControls->MotSts.MotPos;
#else/* Sigam7 ScanA.	*/							/* <S02F> */
			BaseControls->PosManager.var.Per64[0] = 0;
			BaseControls->PosManager.var.Per64[1] = 0;
			MlibPerrcalx(lwk, 0, BaseControls->PosManager.var.Per64);
#endif
			MlibPerrcala( BaseControls->PosManager.VirtualPositionError, 0,
							&BaseControls->PosManager.conf.Egear, &BaseControls->PosManager.var.PosErrA );
			/* Initialize current command position */
			BaseControls->CtrlCmdMngr.PosRefLo = BaseControls->PosManager.VirtualPositionError
											+ BaseControls->PosManager.PgPos.apos[0];
		}
		else
		{
			BaseControls->CtrlCmdMngr.BaseFbkPos += BaseControls->CtrlCmdMngr.dPosRefo;
		}

		/* Servo communication position command management(dPosRefo,etc) */
		dPosRefo = PosMngNetPosCmdManager( BaseLoops, SvControlIf, FnCmnCtrl, Bprm );/* <S04D> */
	}
	BaseControls->CtrlCmdMngr.dPosRefo = dPosRefo;
	BaseControls->CtrlCmdMngr.PosRefLo = SvControlIf->NetPosRef;

/*--------------------------------------------------------------------------------------------------*/
/*		Initialize position control variables when other than position control mode													*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseControls->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_POS )
	{
		/* Position management variable initialization process */
		PosMngInitPositionManager( BaseControls );
	}
}

//<S0BA>
#if 0
/****************************************************************************************************/
/*																									*/
/*		Mode switch calculation																			*/
/*																									*/
/****************************************************************************************************/
static void ModeSwitch( MODE_SW *ModeSwData, LONG trq_ref, LONG spd_ref, LONG pos_err )
{
	LONG	ModeSWFlag;
	LONG	wk;

	/* Local variable initialization */
	ModeSWFlag = FALSE;
	wk = 0;

	switch( ModeSwData->conf.ModeSWSel )
	{
	case MODESW_TRQREF: /* Mode switch Torque command */
		wk = trq_ref;
		if( wk < 0 )
		{
			wk = -wk;
		}
		if( wk >= ModeSwData->conf.MSWTrqLevel )
		{
			ModeSWFlag = TRUE;
		}
		break;

	case MODESW_SPDREF: /* Mode switch Speed command */
		wk = spd_ref;
		if( wk < 0 )
		{
			wk = -wk;
		}
		if( wk >= ModeSwData->conf.MSWSpdLevel )
		{
			ModeSWFlag = TRUE;
		}
		break;

	case MODESW_MOTACC: /* Mode switch Acceleration */
		wk = ModeSwData->var.Acceleration;
		if( wk < 0 )
		{
			wk = -wk;
		}
		if( wk >= ModeSwData->conf.MSWAccLevel )
		{
			ModeSWFlag = TRUE;
		}
		break;

	case MODESW_POSERR: /* Mode switch Deviation pulse */
		wk = pos_err;
		if( wk < 0 )
		{
			wk = -wk;
		}
		if( wk >= ModeSwData->conf.MSWErrLevel )
		{
			ModeSWFlag = TRUE;
		}
		break;

	case MODESW_NOTUSE: /* Mode switch Not used */
	default:
		break;
	}
	ModeSwData->var.ModeSWFlag = ModeSWFlag;
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Speed command soft start calculation																	*/
/*																									*/
/****************************************************************************************************/
static LONG BaseSoftStartSpdRef( CTRL_CMD_PRM *CtrlCmdPrm, LONG SpdRef, LONG SpdRefx )
{
	LONG	LastSpdRefx;

	LastSpdRefx = SpdRefx;

/*--------------------------------------------------------------------------------------------------*/
/*		Soft start calculation																			*/
/*--------------------------------------------------------------------------------------------------*/
	if( SpdRef > SpdRefx )
	{
		if( SpdRefx >= 0 )
		{
			SpdRefx = SpdRefx + CtrlCmdPrm->SpdSfsAcc;
			if( SpdRefx > SpdRef )
			{
				SpdRefx = SpdRef;
			}
		}
		else
		{
			SpdRefx = SpdRefx + CtrlCmdPrm->SpdSfsDec;
			if( SpdRefx > SpdRef )
			{
				SpdRefx = SpdRef;
			}
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	else if( SpdRef < SpdRefx )
	{
		if( SpdRefx <= 0 )
		{
			SpdRefx = SpdRefx - CtrlCmdPrm->SpdSfsAcc;
			if( SpdRefx < SpdRef )
			{
				SpdRefx = SpdRef;
			}
		}
		else
		{
			SpdRefx = SpdRefx - CtrlCmdPrm->SpdSfsDec;
			if( SpdRefx < SpdRef )
			{
				SpdRefx = SpdRef;
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Zero speed via processing : I'm not sure why it's necessary, but I'll set it to SGDS for now					*/
/*--------------------------------------------------------------------------------------------------*/
//	if( (SpdRef != SpdRefx) && (CtrlCmdPrm->SpdSfsAcc != CtrlCmdPrm->SpdSfsDec) )/* SGDVHorizontal expansion */
	if(CtrlCmdPrm->SpdSfsAcc != CtrlCmdPrm->SpdSfsDec)
	{
		if( (LastSpdRefx != 0) && ((LastSpdRefx ^ SpdRefx) < 0) )
		{
			SpdRefx = 0;
		}
	}
	return( SpdRefx );
}



/****************************************************************************************************/
/*																									*/
/*		Origin search control calculation process																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : 60rpmspeed command													*/
/*			At scan B, the origin is(C phase)detection and zero clamp command.								*/
/*			In the round, key operation sequence processing is performed.											*/
/*																									*/
/*	Note Intention : Please note that this is motor position control and does not support full closed loop control.			*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static LONG ZeroSearchControl( ZX_CTRL_HNDL *ZeroClampCtrl, BASE_MOT_STS *MotSts, KSGAIN ZcKp, LONG JogSpeed )
{
	LONG	spd_ref;

/*--------------------------------------------------------------------------------------------------*/
/*		JOG speed operation: until the origin is detected															*/
/*--------------------------------------------------------------------------------------------------*/
	if( !(MotSts->MotCphPass || ZeroClampCtrl->var.OrgSrchFinish) )	/* CPhase passage check				*/
	{
		ZeroClampCtrl->var.PosError = 0;
		spd_ref = JogSpeed;											/* Origin search speed command			*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Zero clamp control: Origin search complete															*/
/*--------------------------------------------------------------------------------------------------*/
	else															/* CAfter phase passage detection				*/
	{
		if( ZeroClampCtrl->var.OrgSrchFinish == FALSE )
		{ /* For preventing origin variations in special motors */
			/* Position deviation latch(First time) */
			ZeroClampCtrl->var.PosError = -MotSts->MposFromCpos;
		}
		else
		{ /* From the second time onwards, use the motor position difference */
			/* Position deviation calculation(After the second time) */
			ZeroClampCtrl->var.PosError -= MotSts->dMotPos;
		}
	/*----------------------------------------------------------------------------------------------*/
		/* Origin search completeFlag */
		ZeroClampCtrl->var.OrgSrchFinish = TRUE;
		/* Position control calculation(P control) */
#if (FLOAT_USE==TRUE)
		spd_ref = (LONG)((float)ZeroClampCtrl->var.PosError * ZcKp);
#else
		spd_ref = MlibMulgain( ZeroClampCtrl->var.PosError, ZcKp );
#endif /* FLOAT_USE */
	/*----------------------------------------------------------------------------------------------*/
		if( spd_ref == 0 )											/* Correction when speed command is zero			*/
		{
			if( ZeroClampCtrl->var.PosError > 0 )
			{
				spd_ref =  1;
			}
			else if( ZeroClampCtrl->var.PosError < 0 )
			{
				spd_ref = -1;
			}
		}
	/*----------------------------------------------------------------------------------------------*/
		else if( spd_ref > ZeroClampCtrl->conf.SpdLimit )			/* Speed command upper limit			*/
		{
			spd_ref =  ZeroClampCtrl->conf.SpdLimit;
		}
	/*----------------------------------------------------------------------------------------------*/
		else if( spd_ref < -ZeroClampCtrl->conf.SpdLimit )			/* Speed command lower limit			*/
		{
			spd_ref = -ZeroClampCtrl->conf.SpdLimit;
		}
	}
/*--------------------------------------------------------------------------------------------------*/
	return( spd_ref );

}



/****************************************************************************************************/
/*																									*/
/*		Torque (surplus torque) calculation for speed limit during torque control											*/
/*																									*/
/****************************************************************************************************/
#if 0
/* Mercury(SGDV-MD) (Speed limit does not work) */
static LONG OverTorqueCalculation( LONG SpdRfFilo, LONG TConSpdLmtLevel, BOOL VrefSpdLmt )
{
	LONG	vref_limit;

	/*----------------------------------------------------------------------------------------------*/
	/*		Speed limit value TrqCtrl.V.SpdLimit Selection														*/
	/*----------------------------------------------------------------------------------------------*/
	if( VrefSpdLmt )							/* V-REFas an external speed limit input		*/
	{
		vref_limit = MlibLABS( SpdRfFilo );

		/* Limited by external and internal small speed */
		if( vref_limit > TConSpdLmtLevel )
		{
			vref_limit = TConSpdLmtLevel;
		}
	}
	else
	{
		/* Pn407 */
		vref_limit = TConSpdLmtLevel;
	}

	return vref_limit;
}

#else

/* SGDV */
/****************************************************************************************************/
/*																									*/
/*		Torque (surplus torque) calculation for speed limit during torque control											*/
/*			Since there are some points that do not make sense, except for variable replacementSGDSLeave it as software				*/
/*																	2003.04.16	S.Takamatsu			*/
/****************************************************************************************************/
#define	SPDLMTSW	0x40000					/* OSOf speed1/64											*/
#define	INTDECVAL	(2*SPDLMTSW)			/* OSOf speed1/32(SPDLMTSW)		*/
#define	OVERTRQLMT	19573419				/* Upper limit value of surplus torque(=2^24*17500/15000)				*/
LONG OverTorqueCalculation( CTRL_CMD_MNG *CtrlCmdMng, CTRL_CMD_PRM *CtrlCmdPrm )
{
LONG	SpdLimitx;
LONG	nerp;
LONG	neri;
LONG	nerpi;
LONG	lwrk;
LONG	overtorque;
LONG	Status;
LONG	SpdFbk = CtrlCmdMng->SpdFbki;
LONG	TrqRefi = CtrlCmdMng->TrqRefo;
	/*----------------------------------------------------------------------------------------------*/
	/*		Speed limit value TrqCtrl.V.SpdLimit Selection														*/
	/*----------------------------------------------------------------------------------------------*/
		if( CtrlCmdPrm->VrefSpdLmt )			/* V-REFas an external speed limit input		*/
		{
			SpdLimitx = MlibAbs32( CtrlCmdMng->SpdRfFilo );	/* Limited by external and internal small speed			*/
			if( SpdLimitx < CtrlCmdPrm->TConSpdLmtLevel )	
			{ 
				CtrlCmdMng->SpdLimit = SpdLimitx; 
			}
			else
			{ 
				CtrlCmdMng->SpdLimit = CtrlCmdPrm->TConSpdLmtLevel; 
			}
		}
		else
		{
			CtrlCmdMng->SpdLimit = CtrlCmdPrm->TConSpdLmtLevel;	/* Pn407 					*/
		}

	/*----------------------------------------------------------------------------------------------*/
	/*		Create speed deviation between speed FB and limit speed														*/
	/*----------------------------------------------------------------------------------------------*/
		if ( SpdFbk > 0 )
		{
			nerp = SpdFbk - CtrlCmdMng->SpdLimit;
		}
		else if ( SpdFbk < 0 )
		{
			nerp = SpdFbk + CtrlCmdMng->SpdLimit;
		}
		else
		{
			nerp = ( TrqRefi >= 0 ) ?	-CtrlCmdMng->SpdLimit : CtrlCmdMng->SpdLimit;
		}

		/* Speed deviation proportional term */
		lwrk = MlibMulgain29( nerp, CtrlCmdPrm->OverTrqLoopGain[0] );
		
		/* Speed deviation integral term */
		neri = MlibIntegral( nerp, CtrlCmdPrm->OverTrqIntegGain[0], CtrlCmdPrm->OverTrqIntegBuf );

		nerpi = lwrk + neri;	/* 2^29+2^24: No overflow */

		/*  Processing when the integral value calculated from the speed deviation is different from the actual motor rotation direction */
		if ( (((SpdFbk ^ nerpi) < 0) && (TrqRefi >= 0)) || ((((SpdFbk - 1) ^ nerpi) < 0) && (TrqRefi < 0)) )
		{
			/* Processing when speed limit is low */
			if ( CtrlCmdMng->SpdLimit <= SPDLMTSW )
			{
				/* Gradual deceleration */
				if ( neri >= 0 )
				{
					lwrk = -INTDECVAL;
					neri = MlibIntegral( lwrk, CtrlCmdPrm->OverTrqIntegGain[0], CtrlCmdPrm->OverTrqIntegBuf );

					/* Integral pool is0and the time until it reaches the speed limit */
					Status = ( neri >= 0 ) ? TRUE : FALSE;
				}
				else
				{
					lwrk = INTDECVAL;
					neri = MlibIntegral( lwrk, CtrlCmdPrm->OverTrqIntegGain[0], CtrlCmdPrm->OverTrqIntegBuf );

					/* Integral pool is0and the time until it reaches the speed limit */
					Status = ( neri < 0 ) ? TRUE : FALSE;
				}
			}
			else
			{
				Status = FALSE;
			}
		}
		else
		{
			Status = TRUE;
		}

		if ( Status )
		{
			CtrlCmdMng->SpdClamp = TRUE;
		}
		else
		{
			CtrlCmdPrm->OverTrqIntegBuf[ 1 ] = 0;
			CtrlCmdPrm->OverTrqIntegBuf[ 0 ] = 0;
			nerpi = 0;
			CtrlCmdMng->SpdClamp = FALSE;
		}

		/* Amount that can be decelerated even if maximum reverse torque is applied */
		overtorque = MlibLimitul( nerpi, OVERTRQLMT, -OVERTRQLMT );

		return overtorque;

}
#endif


/****************************************************************************************************/
/*																									*/
/*		Torque limit calculation processing at undervoltage															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : To limit torque when undervoltage warning occurs and If not overtravel			*/
/*				Warning is occurring	:Pn424										*/
/*				Canceling the warning	:Pn425									*/
/*				After canceling the warning	:MAXLimitation by torque											*/
/*					=Pn425After time elapsed																*/
/*			When not limiting			:MAXLimitation by torque											*/
/*																									*/
/****************************************************************************************************/
static LONG LpxUvTrqLmtControl( UV_TLIM_DATA *UvTrqLimits, BOOL OtStatus, ALMMAN_HANDLE *AlmMngr ) /* <S145> */
{
	if ( (UvTrqLimits->conf.UvTrqLmtCtrl != FALSE) && (OtStatus == FALSE) )
	{
		if ( FALSE != ALMCheckEachState( AlmMngr, WRN_UV ) ) /* <S145> */
		{
			UvTrqLimits->var.UvTrqLmt = UvTrqLimits->conf.UvTLimSetting;
			UvTrqLimits->var.UvTrqLmtWait = UvTrqLimits->conf.UvTrqLmtOffFil;
		}
		else
		{
			if ( UvTrqLimits->var.UvTrqLmtWait == 0 )
			{
				UvTrqLimits->var.UvTrqLmt = NORMAXVALUE;
			}
			else
			{
				UvTrqLimits->var.UvTrqLmtWait--;
				UvTrqLimits->var.UvTrqLmt += UvTrqLimits->conf.DeltaUvTrqLmt;
			}
		}
	}
	else
	{
		UvTrqLimits->var.UvTrqLmt = NORMAXVALUE;
		UvTrqLimits->var.UvTrqLmtWait = 0;
	}
	return UvTrqLimits->var.UvTrqLmt;
}


/****************************************************************************************************/
/*																									*/
/*		Torque limit calculation process																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Internal torque limit set by the parameter(Pn402,Pn403), External torque limit(Pn404,Pn405)			*/
/*			and the torque command AD, calculate the torque limit value under the conditions shown below.					*/
/*			In addition, when OT(Signal from OT sequence)The emergency stop torque(Pn406).			*/
/*																									*/
/*																									*/
/*		Forward rotation side torque limit calculation block diagram (Same for reverse side)												*/
/*																									*/
/*		Pn406 : EstopTrqLmt -----||-------------------------------+-------> BoutV.FwTrqLmt			*/
/*							    OT-TrqLmt   			 		  | 								*/
/*													   			  | 								*/
/*												+-------+   	  | 								*/
/*		Pn402 : FwIntTrqLmt ------------------->|     	|   	  | 								*/
/*												|   	|   	  | 								*/
/*												|   	|   	  | 								*/
/*		Pn404 : FwExtTrqLmt -----||------------>|   	|   	  | 								*/
/*							    P-CL			|  Min  +---|/|---+ 								*/
/*												|   	|  OT-TrqLmt								*/
/*				TrqRefAd    -----||------------>|   	|   										*/
/*							  Pn002.0=1  		|   	|   										*/
/*												|   	|   										*/
/*				TrqRefAd    -----||-------||--->|   	|   										*/
/*							  Pn002.0=3  P-CL	+-------+   										*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void BpiTorqueLimitControl( TRQ_LMT_DATA *TrqLimits, BASE_CTRL_HNDL *pBaseCtrl,	BOOL OtTrqLmtReq, ALMMAN_HANDLE *AlmMngr ) /* <S145> */
{
	LONG			lwk;
	LONG			FwTrqLmt;
	LONG			RvTrqLmt;
	LONG			UvTrqLmt;

/*--------------------------------------------------------------------------------------------------*/
/*		Torque limit calculation during OT																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( OtTrqLmtReq )
	{
		FwTrqLmt = TrqLimits->conf.EstopTrqLmt;
		RvTrqLmt = TrqLimits->conf.EstopTrqLmt;

		LpxUvTrqLmtControl( &(TrqLimits->UvTrqLimits), TRUE, AlmMngr );	/* <S145> */
	}
	else
	{
/*--------------------------------------------------------------------------------------------------*/
/*		Torque limit calculation when main circuit power supply voltage drops													*/
/*--------------------------------------------------------------------------------------------------*/
		UvTrqLmt = LpxUvTrqLmtControl( &(TrqLimits->UvTrqLimits), FALSE, AlmMngr );	/* <S145> */

/*--------------------------------------------------------------------------------------------------*/
/*		Torque limit calculation at non-OT																*/
/*--------------------------------------------------------------------------------------------------*/
		if( TrqLimits->var.PclSignal )
		{
			FwTrqLmt = ( TrqLimits->conf.FwIntTrqLmt < TrqLimits->conf.FwExtTrqLmt ) ?
						 TrqLimits->conf.FwIntTrqLmt:
						 TrqLimits->conf.FwExtTrqLmt;
		}
		else
		{
			FwTrqLmt = TrqLimits->conf.FwIntTrqLmt;
		}

		if( TrqLimits->var.NclSignal )
		{
			RvTrqLmt = ( TrqLimits->conf.RvIntTrqLmt < TrqLimits->conf.RvExtTrqLmt ) ?
						 TrqLimits->conf.RvIntTrqLmt:
						 TrqLimits->conf.RvExtTrqLmt;
		}
		else
		{
			RvTrqLmt = TrqLimits->conf.RvIntTrqLmt;
		}
	/*------------------------------------------------------------------------------------------*/
		if( UvTrqLmt < FwTrqLmt )
		{
			FwTrqLmt = UvTrqLmt;
		}

		if( UvTrqLmt < RvTrqLmt )
		{
			RvTrqLmt =  UvTrqLmt;
		}
	/*------------------------------------------------------------------------------------------*/
		if( (pBaseCtrl->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_TRQ)
			&& (pBaseCtrl->CtrlModeSet.CtrlMode.b.cm != CTRLMODE_JOG) )
		{
			if( TrqLimits->conf.TrefTrqLmt || (TrqLimits->conf.TrefTrqLmtCL && TrqLimits->var.PclSignal) )
			{ /* (Pn002.0=1:TLIM) || ((Pn002.0=3:PCL/NCL) && (PCLStatus)) */
				lwk = MlibLABS( TrqLimits->var.FwTrqLmt );
				if( lwk < FwTrqLmt )
				{
					FwTrqLmt = lwk;
				}
			}
		/*--------------------------------------------------------------------------------------*/
			if( TrqLimits->conf.TrefTrqLmt || (TrqLimits->conf.TrefTrqLmtCL && TrqLimits->var.NclSignal) )
			{ /* (Pn002.0=1:TLIM) || ((Pn002.0=3:PCL/NCL) && (NCLStatus)) */
				lwk = MlibLABS( TrqLimits->var.RvTrqLmt );
				if( lwk < RvTrqLmt )
				{
					RvTrqLmt = lwk;
				}
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Torque limit calculation result output --> Kernel														*/
/*--------------------------------------------------------------------------------------------------*/
	TrqLimits->var.PosTrqLmtOut =  FwTrqLmt;				/* Forward rotation side torque limit					*/
	TrqLimits->var.NegTrqLmtOut = -RvTrqLmt;				/* Reverse side torque limit					*/

}



/****************************************************************************************************/
/*																									*/
/*		Service to change the moving average time constant for motor speed detection		<V259>										*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Change the number of moving averages of the moving average filter for motor speed detection online.					*/
/*			In argument0~4is specified, a response is returned if it is other than the parameterPn009.2From the encoder resolution				*/
/*			A determined initial value is set.																*/
/*																									*/
/*		(Caution)																					*/
/*			If the change is made when the speed is present, the moving average buffer will also be cleared, so				*/
/*			The speed momentarily becomes zero. Change the number of moving averages with speed = 0.					*/
/*																									*/
/****************************************************************************************************/
void BpiChangeSpdDetMaFil( MOTSPDMAFIL *MotSpdMafil, LONG num )		/* <S02F> */
{
	LONG	lwk0;

	if( ( num >= 0 ) && ( num <= 4 ) )
	{
		lwk0 = num;
	}
	else
	{
		lwk0 = MotSpdMafil->defaultNumBit;
	}
/*--------------------------------------------------------------------------------------------------*/
	if( MotSpdMafil->manumBit != lwk0 )
	{
		MotSpdMafil->manumBit = lwk0;
		MotSpdMafil->idx = 0;
		MotSpdMafil->ksub = 0;
		MotSpdMafil->mabufSumx = 0;
	}
}



/****************************************************************************************************/
/*																									*/
/*		Position command update processing																			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*			Recalculate parameters, write current zero phase information, etc.ScanAstops,							*/
/*			ScanAwill not be able to receive the position command, causing positional deviation.								*/
/*			ScanAcannot receive the position command,ScanB.			*/
/****************************************************************************************************/
void	PosRefRenewal( POS_MNG_HNDL *PosManager, CTRLMD CtrlMode )
{
	if( CtrlMode.b.cm == CTRLMODE_POS )
	{ /* ScanAcannot receive the position command, */
		if( PosManager->RefRenewCntA != PosManager->RefRenewCntB )
		{
			PosManager->CompdPosRefi += PosManager->dPosRefi;							/* <S187>	*/
			PosManager->CompdPcmda   += PosManager->var.dPcmda;
		}
		else												/* Normal processing								*/
		{
			PosManager->CompdPosRefi = PosManager->dPosRefi;							/* <S187>	*/
			PosManager->CompdPcmda   = PosManager->var.dPcmda;
		}
	}
	else													/* Clear for other than position control		*/
	{
			PosManager->CompdPosRefi = 0;
			PosManager->CompdPcmda   = 0;
	}
	PosManager->RefRenewCntB++;							/* ScanAFor position command update 					*/

	return;
}



/***************************************** end of file **********************************************/

/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MicroScanAIf.c : μProgramScanA I/FProcess definition												*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : μProgramScanAandCPUI/FProcessing														*/
/*																									*/
/*		μProgram -CPU.														*/
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
#include "MicroScanAIf.h"
#include "BaseControlPrmCalc.h"
#include "JL086ServoIpHw.h"


/* External function */
extern void ApiSetCurDatasMotorLessTestMode( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut );


#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
/****************************************************************************************************/
/*																									*/
/*		Initialize microprogram IF variables															*/
/*																									*/
/****************************************************************************************************/
//void	KpxInitMicroIFVariables( AXIS_HANDLE *Axis )
void MicroSA_InitVariables( BASE_CTRL_HNDL *pBaseCtrl,
    	                    MENCV *MencV, ASICS *SvAsicRegs, MICRO_SA_IPRMS *uIniPrm )
{
	USHORT			uswk;
	ASIC_U_IF_STR	*pAsicMicroReg;
	pAsicMicroReg	= SvAsicRegs->AsicMicroReg;

/*--------------------------------------------------------------------------------------------------*/
/*		Writing to micro IF register															*/
/*--------------------------------------------------------------------------------------------------*/
	/* bitInformation setting */
	uswk = SvAsicRegs->MicroBitIprm.data;
	if( MencV->Sigma3Encoder == TRUE ){
		uswk |= BITIPRM_SGM3ENC;}
	if( pBaseCtrl->CtrlCmdPrm.IPSpdControl == TRUE ){
		uswk |= BITIPRM_IPSPDCONTROL;}
	if( pBaseCtrl->CtrlCmdPrm.SpdIctrlKeep == TRUE ){
		uswk |= BITIPRM_SPDICTRLKEEP;}
	if( MencV->DivOut == TRUE ){
		uswk |= BITIPRM_DIVOUT;}
	if( uIniPrm->RvsDir == TRUE ){
		uswk |= BITIPRM_RVSDIR;}
	if( MencV->AxisMotType == MOTTYPE_LINEAR ){
		uswk |= BITIPRM_SERIALCONV;}
	if( MencV->withPoleSensor == TRUE ){
		uswk |= BITIPRM_POLESENSOR;}
	if( MencV->PhaseOrderUWV == TRUE ){
		uswk |= BITIPRM_PHSUWV;}
	if( pBaseCtrl->TuneLessCtrl.conf.TuningLessUse == TRUE ){
		uswk |= BITIPRM_TUNELESSON;}
	if( pBaseCtrl->TuneLessCtrl.conf.TuningLessVComp == TRUE ){
		uswk |= BITIPRM_TLSPDPHACOMP;}
	if( (pBaseCtrl->TuneLessCtrl.conf.TuningLessMode & TLMODE_SPDDETCHNG) != FALSE ){
		uswk |= BITIPRM_TLSPDDETCHNG;}
	if( pBaseCtrl->CtrlCmdPrm.LpassFil3 == TRUE ){
		uswk |= BITIPRM_LPASSFIL3ON;}
	if( (uIniPrm->flg_wf && 0x0011) != FALSE ){
		uswk |= BITIPRM_MOTWEAKENFIELD;}
	if( (uIniPrm->CarrChange) == TRUE ){
		uswk |= BITIPRM_CARRERCHNG;}
	SvAsicRegs->MicroBitIprm.data = uswk;

	/* BITDAT */
	uswk = SvAsicRegs->MicroBitDat.data;
	if( MencV->Cset == TRUE ){
		uswk |= BITDAT_CSET;}
	if( MencV->Cpass == TRUE ){
		uswk |= BITDAT_CPASS;}
	if( MencV->OrgOk == TRUE ){
		uswk |= BITDAT_ORGOK;}
	if( MencV->EncDataNG == TRUE ){
		uswk |= BITDAT_AC90ALM;}
	if( MencV->IncreCposLatch == TRUE ){
		uswk |= BITDAT_CPOSLATCH;}
	if( MencV->DivCpass == TRUE ){
		uswk |= BITDAT_DIVCPASS;}
	if( MencV->AccChkEnable == TRUE ){
		uswk |= BITDAT_ACCCHKENABLE;}
	SvAsicRegs->MicroBitDat.data = uswk;
/*--------------------------------------------------------------------------------------------------*/
	pAsicMicroReg->MREG_BITIPRM		= (USHORT)( SvAsicRegs->MicroBitIprm.data );
	pAsicMicroReg->MREG_BITDAT		= (USHORT)( SvAsicRegs->MicroBitDat.data );
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{ /* Linear type */
		pAsicMicroReg->MREG_MPOSSFTR		= (USHORT)(LMX_MPOS_SFT);
		pAsicMicroReg->MREG_MPOSSFTX		= (USHORT)(LMX_MPOS_SFT);
		pAsicMicroReg->MREG_ABSMOTPOSPG		= (USHORT)(MencV->MotAbsPos);
		pAsicMicroReg->MREG_ABSMOTPOSPGH	= (USHORT)(MencV->MotAbsPos>>16);
		pAsicMicroReg->MREG_MOTPHAX			= (USHORT)(MencV->MotPhaX);
		pAsicMicroReg->MREG_MOTPHAXH		= (USHORT)(MencV->MotPhaX>>16);
		pAsicMicroReg->MREG_PHASEOFST		= (USHORT)(MencV->PhaseOffset);
		pAsicMicroReg->MREG_PHASEOFSTH		= (USHORT)(MencV->PhaseOffset>>16);
		pAsicMicroReg->MREG_MPFPHASEOFST	= (USHORT)(MencV->MpfPhaseOffset);
		pAsicMicroReg->MREG_MPFPHASEOFSTH	= (USHORT)(MencV->MpfPhaseOffset>>16);
		pAsicMicroReg->MREG_MOTPULSENUM		= (USHORT)(MencV->PulseNo);
		pAsicMicroReg->MREG_MOTPULSENUMH	= (USHORT)(MencV->PulseNo>>16);
		pAsicMicroReg->MREG_HALFPULSENUM	= (USHORT)(MencV->HalfPulseNo);
		pAsicMicroReg->MREG_HALFPULSENUMH	= (USHORT)(MencV->HalfPulseNo>>16);
		pAsicMicroReg->MREG_PULSE30DEG		= (USHORT)(MencV->PulseNo30deg);
		pAsicMicroReg->MREG_PULSE30DEGH		= (USHORT)(MencV->PulseNo30deg>>16);
		pAsicMicroReg->MREG_SCALEPITCH		= (USHORT)(MencV->NormScalePitch);
		pAsicMicroReg->MREG_SCALEPITCHH		= (USHORT)(MencV->NormScalePitch>>16);
		pAsicMicroReg->MREG_MOTORGXCHK0		= (USHORT)(MencV->MotOrgXchk[0]);
		pAsicMicroReg->MREG_MOTORGXCHK0H	= (USHORT)(MencV->MotOrgXchk[0]>>16);
		pAsicMicroReg->MREG_MOTORGXCHK1		= (USHORT)(MencV->MotOrgXchk[1]);
		pAsicMicroReg->MREG_MOTORGXCHK1H	= (USHORT)(MencV->MotOrgXchk[1]>>16);
		pAsicMicroReg->MREG_ORGDETDELAY		= (USHORT)(MencV->OrgDetDelayComp);
		pAsicMicroReg->MREG_ORGDETDELAYH	= (USHORT)(MencV->OrgDetDelayComp>>16);
		pAsicMicroReg->MREG_POLEINFO		= (USHORT)(MencV->MotPoleInfo[0]);
		pAsicMicroReg->MREG_LSTPOLEINFO		= (USHORT)(MencV->MotPoleInfo[1]);
		pAsicMicroReg->MREG_DETMOTORGREQ	= (USHORT)(MencV->MotOrgChkAlways);
		pAsicMicroReg->MREG_CPHASELESS		= (USHORT)(MencV->CphaseLess);
		pAsicMicroReg->MREG_MOTORGCHKCNT	= (USHORT)(MencV->MotOrgChkCnt);
		pAsicMicroReg->MREG_MOTORGCHKEND	= (USHORT)(MencV->MotOrgChkEnd);
		pAsicMicroReg->MREG_DIVOUTGAIN		= (USHORT)(MencV->DivOutGain);
		pAsicMicroReg->MREG_DIVOUTGAINH		= (USHORT)(MencV->DivOutGain>>16);
	}
	else
	{ /* Rotation type */
		pAsicMicroReg->MREG_MPOSSFTR		= (USHORT)(MencV->MposSftR);
		pAsicMicroReg->MREG_MPOSSFTX		= (USHORT)(MencV->MposSftX);
		pAsicMicroReg->MREG_INCRECMPSFT		= (USHORT)(MencV->IncreCompSft);
		pAsicMicroReg->MREG_MORGSFT			= (USHORT)(MencV->MorgSft);
		pAsicMicroReg->MREG_MOTPULSENUM		= (USHORT)(MencV->PulseNo);
		pAsicMicroReg->MREG_MOTPULSENUMH	= (USHORT)(MencV->PulseNo>>16);
	}
/*--------------------------------------------------------------------------------------------------*/
	/* Rotation type/Common to linear type */
	pAsicMicroReg->MREG_MPOSSIGN		= (USHORT)(MencV->MposSign);
	pAsicMicroReg->MREG_KMOTPHASE		= (USHORT)(MencV->Kmotphase);
	pAsicMicroReg->MREG_KMOTPHASEH		= (USHORT)(MencV->Kmotphase>>16);
	pAsicMicroReg->MREG_ACCERRLVL		= (USHORT)(MencV->AccErrLvl);
	pAsicMicroReg->MREG_ACCERRLVLH		= (USHORT)(MencV->AccErrLvl>>16);
	pAsicMicroReg->MREG_DIVOUTSFT		= (USHORT)(MencV->DivOutSft);
	pAsicMicroReg->MREG_ENCLCHNUM		= (USHORT)(MencV->EncLchNum);
	pAsicMicroReg->MREG_SPDMANUMBITI	= (USHORT)(uIniPrm->SpdMaNumBit);
	pAsicMicroReg->MREG_SPGFAIL			= (USHORT)(uIniPrm->spgfail.b.l);
	pAsicMicroReg->MREG_MOTPOS			= (USHORT)(MencV->MotPos);
	pAsicMicroReg->MREG_MOTPOSH			= (USHORT)(MencV->MotPos>>16);
	pAsicMicroReg->MREG_MOTPOSX0		= (USHORT)(MencV->MotPosX[0]);
	pAsicMicroReg->MREG_MOTPOSX0H		= (USHORT)(MencV->MotPosX[0]>>16);
	pAsicMicroReg->MREG_MOTPOSX1		= (USHORT)(MencV->MotPosX[1]);
	pAsicMicroReg->MREG_MOTPOSX1H		= (USHORT)(MencV->MotPosX[1]>>16);
	pAsicMicroReg->MREG_MOTPOSX			= (USHORT)(MencV->MotPosX[2]);
	pAsicMicroReg->MREG_MOTPOSXH		= (USHORT)(MencV->MotPosX[2]>>16);
	pAsicMicroReg->MREG_MOTORGX			= (USHORT)(MencV->MotOrgX);
	pAsicMicroReg->MREG_MOTORGXH		= (USHORT)(MencV->MotOrgX>>16);
	pAsicMicroReg->MREG_MTURN_SHIFT		= (USHORT)(MencV->MultiturnSft);
/*--------------------------------------------------------------------------------------------------*/
	pAsicMicroReg->MREG_KMOTSPDA		= (USHORT)(uIniPrm->KmotspdA);
	pAsicMicroReg->MREG_KMOTSPDAH		= (USHORT)(uIniPrm->KmotspdA>>16);
	pAsicMicroReg->MREG_KPCMPD			= (USHORT)(pBaseCtrl->PcmpCalcData.Kpcmpd);
	pAsicMicroReg->MREG_KPCMPDH			= (USHORT)(pBaseCtrl->PcmpCalcData.Kpcmpd>>16);
	pAsicMicroReg->MREG_KPCMPSP1		= (USHORT)(pBaseCtrl->PcmpCalcData.Kpcmpsp1);
	pAsicMicroReg->MREG_KPCMPSP1H		= (USHORT)(pBaseCtrl->PcmpCalcData.Kpcmpsp1>>16);
	pAsicMicroReg->MREG_PCMPSPD1		= (USHORT)(pBaseCtrl->PcmpCalcData.PcmpSpd1);
	pAsicMicroReg->MREG_PCMPSPD1H		= (USHORT)(pBaseCtrl->PcmpCalcData.PcmpSpd1>>16);
	pAsicMicroReg->MREG_KPCMPDITPLT		= (USHORT)(pBaseCtrl->PcmpCalcData.KpcmpdItplt);
	pAsicMicroReg->MREG_KPCMPDITPLTH	= (USHORT)(pBaseCtrl->PcmpCalcData.KpcmpdItplt>>16);
/*--------------------------------------------------------------------------------------------------*/
	pAsicMicroReg->MREG_SBCYCLE			= (USHORT)(uIniPrm->SvCycleNs/KPI_SBCYCLE_BASE_NS);
#if defined(CSW_NETIF_SYNQNET_TYPE)
	pAsicMicroReg->MREG_ASYNC_HSUR2		= (USHORT)(1);
	pAsicMicroReg->MREG_SBMONTIMING		= (USHORT)(0);
#else
	pAsicMicroReg->MREG_ASYNC_HSUR2		= (USHORT)(0);
	pAsicMicroReg->MREG_SBMONTIMING		= (USHORT)(1);
#endif
/*--------------------------------------------------------------------------------------------------*/
	pAsicMicroReg->MREG_DUMMY = 0x0000;									/* Dummy write				*/

}
#endif


#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
/****************************************************************************************************/
/*																									*/
/*		Position/speed gain initial parameter micro writing process										*/
/*																									*/
/****************************************************************************************************/
void MicroSA_InitGainParameters( ASIC_U_IF_STR *pAsicMicroReg )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Writing to micro IF register															*/
/*--------------------------------------------------------------------------------------------------*/
	pAsicMicroReg->MREG_SVGKP		= pAsicMicroReg->MREG_BNK1SVGKP;
	pAsicMicroReg->MREG_SVGKPH		= pAsicMicroReg->MREG_BNK1SVGKPH;
	pAsicMicroReg->MREG_SVGKV		= pAsicMicroReg->MREG_BNK1SVGKV;
	pAsicMicroReg->MREG_SVGKVH		= pAsicMicroReg->MREG_BNK1SVGKVH;
	pAsicMicroReg->MREG_SVGKVI		= pAsicMicroReg->MREG_BNK1SVGKVI;
	pAsicMicroReg->MREG_SVGKVIH		= pAsicMicroReg->MREG_BNK1SVGKVIH;
	pAsicMicroReg->MREG_SVGKV2		= pAsicMicroReg->MREG_BNK1SVGKV2;
	pAsicMicroReg->MREG_SVGKV2H		= pAsicMicroReg->MREG_BNK1SVGKV2H;
	pAsicMicroReg->MREG_SVGKV2IV	= pAsicMicroReg->MREG_BNK1SVGKV2IV;
	pAsicMicroReg->MREG_SVGKV2IVH	= pAsicMicroReg->MREG_BNK1SVGKV2IVH;
	pAsicMicroReg->MREG_KTRQFIL		= pAsicMicroReg->MREG_BNK1KTRQFIL;
	pAsicMicroReg->MREG_KTRQFILH	= pAsicMicroReg->MREG_BNK1KTRQFILH;
/*--------------------------------------------------------------------------------------------------*/
	pAsicMicroReg->MREG_DOBSKS		= pAsicMicroReg->MREG_BNK1DOBSKS;
	pAsicMicroReg->MREG_DOBSKSH		= pAsicMicroReg->MREG_BNK1DOBSKSH;
	pAsicMicroReg->MREG_DOBSKSJ		= pAsicMicroReg->MREG_BNK1DOBSKSJ;
	pAsicMicroReg->MREG_DOBSKSJH	= pAsicMicroReg->MREG_BNK1DOBSKSJH;
	pAsicMicroReg->MREG_DOBSKJ		= pAsicMicroReg->MREG_BNK1DOBSKJ;
	pAsicMicroReg->MREG_DOBSKJH		= pAsicMicroReg->MREG_BNK1DOBSKJH;
	pAsicMicroReg->MREG_DOBSKD		= pAsicMicroReg->MREG_BNK1DOBSKD;
	pAsicMicroReg->MREG_DOBSKDH		= pAsicMicroReg->MREG_BNK1DOBSKDH;
	pAsicMicroReg->MREG_DOBSKLPF	= pAsicMicroReg->MREG_BNK1DOBSKLPF;
	pAsicMicroReg->MREG_DOBSKLPFH	= pAsicMicroReg->MREG_BNK1DOBSKLPFH;
/*--------------------------------------------------------------------------------------------------*/
	pAsicMicroReg->MREG_OVTRQIG		= pAsicMicroReg->MREG_BNK1OVTRQIG;
	pAsicMicroReg->MREG_OVTRQIGH	= pAsicMicroReg->MREG_BNK1OVTRQIGH;
	pAsicMicroReg->MREG_OVTRQKV		= pAsicMicroReg->MREG_BNK1OVTRQKV;
	pAsicMicroReg->MREG_OVTRQKVH	= pAsicMicroReg->MREG_BNK1OVTRQKVH;

	pAsicMicroReg->MREG_DUMMY = 0x0000;									/* Dummy write				*/

}
#endif


#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
/****************************************************************************************************/
/*																									*/
/*		Micro response data input processing																	*/
/*																									*/
/****************************************************************************************************/
void MicroSA_InputCycDataB( ASICS *SvAsicRegs, MENCV *MencV, CTRL_LOOP_OUT *CtrlLoopOut, BPRMDAT *Bprm )
{
//	ASIC_HW_IF_STR		*pAsicHwReg;
	MICRO_BITDAT		*pMicroBitDat;
	BOOL				CposLatch;
	MICRO_CYC_INPUTS	*uCycInputs;
#if (CSW_MAICRO_DMA_TRANSFER == 0)
	ASIC_U_IF_STR		*pAsicMicroReg;
	pAsicMicroReg	= SvAsicRegs->AsicMicroReg;
	uCycInputs		= (MICRO_CYC_INPUTS*)&pAsicMicroReg->MREG_BITDAT;
#else
	uCycInputs		= SvAsicRegs->uDMAInBuffer;
#endif

	/* Local variable initialization */
	pMicroBitDat	= &( SvAsicRegs->MicroBitDat );

/*--------------------------------------------------------------------------------------------------*/
/*		Get Encoder Information																		*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->CphaseDist	= uCycInputs->MREG_CPHDIST + ( (uCycInputs->MREG_CPHDISTH) << 16 );
	MencV->RxAlmCode	= uCycInputs->MREG_RXALMCODE;	/* Read current value:Alarm Code			*/
//	MencV->RxTimex.dw	= pAsicMicroReg->MREG_RXTIMEBASE0;	/* Read current value:TimeBase			*/
//	MencV->MstErrCnt	= pAsicMicroReg->MREG_MSTERRCNT;
//	MencV->AccErrCnt	= pAsicMicroReg->MREG_ACCERRCNT;
//	MencV->ComErrCnt	= pAsicMicroReg->MREG_COMERRCNT;

/*--------------------------------------------------------------------------------------------------*/
/*		Get Control Loop Output																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* Reference and Feedback Monitor */
	CtrlLoopOut->MotSpd_a	= uCycInputs->MREG_MOTSPDA + ( (uCycInputs->MREG_MOTSPDAH) << 16 );
	CtrlLoopOut->IdRefMon = (LONG)((SHORT)uCycInputs->MREG_IDO);
//	CtrlLoopOut->TrqRefMon_a = CtrlLoopOut->TrqRefMon;
//	CtrlLoopOut->IqRefMon_a = CtrlLoopOut->IdRefMon;


	/* Control Loop Status Monitor */
	pMicroBitDat->data		= uCycInputs->MREG_BITDAT;
/*--------------------------------------------------------------------------------------------------*/
/*		Make Bit Status																				*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->SeRxErr	= pMicroBitDat->data & BITDAT_SERXERR;
	MencV->Cset		= pMicroBitDat->data & BITDAT_CSET;
	MencV->Cpass	= pMicroBitDat->data & BITDAT_CPASS;
	MencV->OrgOk	= pMicroBitDat->data & BITDAT_ORGOK;
	CposLatch		= pMicroBitDat->data & BITDAT_CPOSLATCH;
	if( MencV->IncreCposLatch != CposLatch )
	{
		MencV->IncreCposLatch = CposLatch;
	}
	MencV->DivCpass	= pMicroBitDat->data & BITDAT_DIVCPASS;

	CtrlLoopOut->SvonSpdLmtFlag	= pMicroBitDat->data & BITDAT_SVONSPDLMT;
	CtrlLoopOut->SpdClamp			= pMicroBitDat->data & BITDAT_SPDCLAMP;
	CtrlLoopOut->PConActFlg		= pMicroBitDat->data & BITDAT_PCONACTFLG;
	CtrlLoopOut->TrqClamp = ((uCycInputs->MREG_CTSTRW & 0x0020) != 0) ? TRUE : FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Get Encoder Error Status																	*/
/*--------------------------------------------------------------------------------------------------*/
	MencV->EncDataNG = (0 != ( pMicroBitDat->data & BITDAT_ENCDATANG ));

}

void MicroSA_InputCycDataExB( ASICS *SvAsicRegs, CTRL_LOOP_OUT *CtrlLoopOut, BPRMDAT *Bprm )
{
	MICRO_CYC_INPUTS	*uCycInputs;
#if (CSW_MAICRO_DMA_TRANSFER == 0)
	ASIC_U_IF_STR		*pAsicMicroReg;
	pAsicMicroReg	= SvAsicRegs->AsicMicroReg;
	uCycInputs		= (MICRO_CYC_INPUTS*)&pAsicMicroReg->MREG_BITDAT;
#else
	uCycInputs		= SvAsicRegs->uDMAInBuffer;
#endif

	CtrlLoopOut->MotPosErr	= uCycInputs->MREG_MOTPOSERR + ( (uCycInputs->MREG_MOTPOSERRH) << 16 );
	CtrlLoopOut->SpdRefo	= uCycInputs->MREG_MONSPDREF + ( (uCycInputs->MREG_MONSPDREFH) << 16 );
	CtrlLoopOut->SpdFFC	= uCycInputs->MREG_MONSPDFFC + ( (uCycInputs->MREG_MONSPDFFCH) << 16 );
	CtrlLoopOut->TrqRefo	= uCycInputs->MREG_TRQFILOUT + (uCycInputs->MREG_TRQFILOUTH << 16);
	CtrlLoopOut->TrqRefo_a	= uCycInputs->MREG_TRQREFOA + ( (uCycInputs->MREG_TRQREFOAH) << 16 );

/*--------------------------------------------------------------------------------------------------*/
/*		Get Current Loop Output																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* μqAxis current command(Average value) */
	CtrlLoopOut->IqRefMon_a = (LONG)((SHORT)uCycInputs->MREG_IQMONA) * Bprm->DirSign;
	CtrlLoopOut->TrqRefMon_a = MlibMulhigh32( (CtrlLoopOut->IqRefMon_a << 16), NORM_IQ_TO_TRQ );

}
#endif


#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
/****************************************************************************************************/
/*																									*/
/*		Micro servo control command output processing																*/
/*																									*/
/****************************************************************************************************/
void MicroSA_OutputCycDataB( ASICS *SvAsicRegs, MENCV *MencV, BASE_CTRL_HNDL *pBaseCtrl, LONG CmdCtrlBit )
{
	ASIC_U_IF_STR		*pAsicMicroReg;
//	MICRO_BITCMD		*pMicroBitCmd;
	USHORT				MicroBitCmd;
	MICRO_CYC_OUTPUTS	*uCycOutputs;

	/* Local variable initialization */
	pAsicMicroReg		= SvAsicRegs->AsicMicroReg;
//	pMicroBitCmd		= &(SvAsicRegs->MicroBitCmd);

#if (CSW_MAICRO_DMA_TRANSFER != 0)
	uCycOutputs		= SvAsicRegs->uDMAOutBuffer;
#else
	uCycOutputs		= (MICRO_CYC_OUTPUTS*)&pAsicMicroReg->MREG_BITCMDI;
#endif

	/* bitInformation setting */
	MicroBitCmd = 0;
	/* P/PISwitch */
	MicroBitCmd |= (((CmdCtrlBit >> ENBPCTRL_BITNO) & 0x01) * BITCMD_PCON);
	/* ModeSWSetting */
	MicroBitCmd |= (pBaseCtrl->ModeSwData.var.ModeSWFlag * BITCMD_MODESWON);
	/* Position deviation clear(Table operation/PJOGDedicated) */
	MicroBitCmd |= (pBaseCtrl->CtrlCmdMngr.PerrClrReq * BITCMD_PERCLRCMD);
	/* Positional integral clear */
	MicroBitCmd |= (((CmdCtrlBit >> CLRPOSINTG_BITNO) & 0x01) * BITCMD_PICVCLRCMD);
	/* ScanAStopping request */
	MicroBitCmd |= (MencV->EncDisable * BITCMD_SCANAPASSREQ);
	/* Table operation request */
	if( pBaseCtrl->TblDrive.var.TblCmd != TBLCMD_NOCMD )
	{
		MicroBitCmd |= BITCMD_TBLDRVON;
	}
	/* Cphase Latch Clear Request */
	MicroBitCmd |= (pBaseCtrl->CLatchClrReq * BITCMD_CLATCHCLR);
	pBaseCtrl->CLatchClrReq = pBaseCtrl->CLatchClrReq^0x1;
	/* Adjustmentless enabled/Select invalid */
	MicroBitCmd |= (pBaseCtrl->TuneLessCtrl.var.TuneLessAct * BITCMD_TUNELESSREQ);
	/* Friction compensation selection (1:Friction compensation enabled) */
	MicroBitCmd |= (pBaseCtrl->DobsCtrl.DobsAct * BITCMD_DOBSON);
	/* Automatic gain switching selection */
	MicroBitCmd |= (pBaseCtrl->GainChange.AtGsel * BITCMD_AUTOGAINCHNG);
	/* Gain switching request */
	if( pBaseCtrl->GainChange.ActGain == 2 )
	{
		MicroBitCmd |= BITCMD_GAINCHNGON;
	}
	/* BBSvcRefClr */
	MicroBitCmd |= (pBaseCtrl->BBRefClrReq * BITCMD_BBSVCREFCLR);
	/* Torque command filter invalid selection */
	MicroBitCmd |= (pBaseCtrl->TblDrive.var.TrqFilDisable * BITCMD_TRQFILINVLD);
	/* PrimaryLPFIntegral initialization request */
	MicroBitCmd |= (pBaseCtrl->CtrlCmdMngr.TrqFilClrCmd * BITCMD_LPFINTEGINIT);
	/* QuadraticLPFRequest */
	MicroBitCmd |= (pBaseCtrl->CtrlCmdPrm.LpassFil2 * BITCMD_LPASSFIL2ON);

/*--------------------------------------------------------------------------------------------------*/
	uCycOutputs->MREG_BITCMDI  = (USHORT)MicroBitCmd;
	uCycOutputs->MREG_ZCTRMDI  = (USHORT)(pBaseCtrl->CtrlModeSet.ZctrlMode.zm);

	switch( pBaseCtrl->CtrlCmdMngr.CtrlMode )
	{
	case CTRLMODE_TRQ:	/* Torque control mode */
		uCycOutputs->MREG_CTRLMDI	= (USHORT)(CTRLMODE_TRQ);
//		uCycOutputs->MREG_TRQREFI	= (USHORT)(pBaseCtrl->CtrlCmdMngr.TrqRefo);
//		uCycOutputs->MREG_TRQREFIH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.TrqRefo>>16);
		uCycOutputs->MREG_SPDREFI	= (USHORT)(pBaseCtrl->CtrlCmdMngr.SpdLimit);
		uCycOutputs->MREG_SPDREFIH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.SpdLimit>>16);
		break;

	case CTRLMODE_POS:	/* Position control mode */
		uCycOutputs->MREG_CTRLMDI	= (USHORT)(CTRLMODE_POS);
		uCycOutputs->MREG_DPOSREFI	= (USHORT)(pBaseCtrl->CtrlCmdMngr.dPosRefo);
		uCycOutputs->MREG_DPOSREFIH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.dPosRefo>>16);
		uCycOutputs->MREG_SPDREFI	= (USHORT)(pBaseCtrl->CtrlCmdMngr.SpdFFCx);
		uCycOutputs->MREG_SPDREFIH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.SpdFFCx>>16);
		uCycOutputs->MREG_TRQREFI	= (USHORT)(pBaseCtrl->CtrlCmdMngr.TrqFFCx);
		uCycOutputs->MREG_TRQREFIH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.TrqFFCx>>16);
		if( pBaseCtrl->CtrlCmdMngr.LastCtrlMode != CTRLMODE_POS )
		{
			pAsicMicroReg->MREG_BASEMOTPOS	= (USHORT)(pBaseCtrl->CtrlCmdMngr.BaseFbkPos);
			pAsicMicroReg->MREG_BASEMOTPOSH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.BaseFbkPos>>16);
		}
		break;

	case CTRLMODE_SPD:	/* Speed control mode */
	case CTRLMODE_JOG:	/* JOG operation mode */
	case CTRLMODE_ZSRCH:/* Origin search mode */
		uCycOutputs->MREG_CTRLMDI	= (USHORT)(CTRLMODE_SPD);
		uCycOutputs->MREG_SPDREFI	= (USHORT)(pBaseCtrl->CtrlCmdMngr.SpdRefo);
		uCycOutputs->MREG_SPDREFIH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.SpdRefo>>16);
		uCycOutputs->MREG_TRQREFI	= (USHORT)(pBaseCtrl->CtrlCmdMngr.TrqFFCx);
		uCycOutputs->MREG_TRQREFIH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.TrqFFCx>>16);
		if( pBaseCtrl->CtrlCmdMngr.LastCtrlMode == CTRLMODE_TRQ )
		{ /* Speed integral value when switching control */
			pAsicMicroReg->MREG_IVARL	= (USHORT)((LONG)pBaseCtrl->CtrlCmdMngr.VelIvar64[0]);
			pAsicMicroReg->MREG_IVARLH	= (USHORT)(((LONG)pBaseCtrl->CtrlCmdMngr.VelIvar64[0])>>16);
		}
		break;

	default:
		break;
	}

	pBaseCtrl->CtrlCmdMngr.LastCtrlMode = pBaseCtrl->CtrlCmdMngr.CtrlMode;

	/*----------------------------------------------------------------------------------------------*/
	/*		Set Torque Limit Value to Micro Program													*/
	/*----------------------------------------------------------------------------------------------*/
	/* Positive torque(Current)Limit value */
	uCycOutputs->MREG_TLMTP24IN		= (USHORT)(pBaseCtrl->TrqLimitData.var.PosTrqLmtOut);
	uCycOutputs->MREG_TLMTP24INH	= (USHORT)(pBaseCtrl->TrqLimitData.var.PosTrqLmtOut >> 16);
	/* Negative side torque(Current)Limit value */
	uCycOutputs->MREG_TLMTM24IN		= (USHORT)(pBaseCtrl->TrqLimitData.var.NegTrqLmtOut);
	uCycOutputs->MREG_TLMTM24INH	= (USHORT)(pBaseCtrl->TrqLimitData.var.NegTrqLmtOut >> 16);

	/*----------------------------------------------------------------------------------------------*/
	/*		Set DC Volt Avarage Value to Micro Program for Weaken Field Param Calc					*/
	/*----------------------------------------------------------------------------------------------*/
	uCycOutputs->MREG_DCVOLTA = pBaseCtrl->WeakenField->var.VdcAsic;

	/*----------------------------------------------------------------------------------------------*/
	/*		Set Voltage Compensation Gain to Micro Program											*/
	/*----------------------------------------------------------------------------------------------*/
	uCycOutputs->MREG_KVVIN = pBaseCtrl->MpKvvComp->var.MpKvv;

#if (CSW_MAICRO_DMA_TRANSFER == 0)
	/* ServoASICFailureNo.3 Correspondence(Dummy write) *//* 2008.04.02 */
	pAsicMicroReg->MREG_DUMMY = 0x0000;									/* Dummy write				*/
#endif

}

void MicroSA_OutputCycDataExB( ASICS *SvAsicRegs, BASE_CTRL_HNDL *pBaseCtrl)
{
	ASIC_U_IF_STR		*pAsicMicroReg;
	MICRO_CYC_OUTPUTS	*uCycOutputs;

	/* Local variable initialization */
	pAsicMicroReg		= SvAsicRegs->AsicMicroReg;

#if (CSW_MAICRO_DMA_TRANSFER != 0)
	uCycOutputs		= SvAsicRegs->uDMAOutBuffer;
#else
	uCycOutputs		= (MICRO_CYC_OUTPUTS*)&pAsicMicroReg->MREG_BITCMDI;
#endif

	/* Table operation request */
	if( pBaseCtrl->TblDrive.var.TblCmd != TBLCMD_NOCMD )
	{
		pAsicMicroReg->MREG_DISTRQINTMG = (USHORT)pBaseCtrl->TblDrive.var.DisTrqInTiming;
		if( pBaseCtrl->TblDrive.var.NotchLpassDisable == TRUE )
		{ /* Notch & low-pass filter disabled request */
			pAsicMicroReg->MREG_CSW = SvAsicRegs->MicroCsw.data | 0x00F0;
		}
		else
		{
			pAsicMicroReg->MREG_CSW = SvAsicRegs->MicroCsw.data;
		}
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Extension control output setting(SpeedFF/TorqueFFis already set as a control command)									*/
/*--------------------------------------------------------------------------------------------------*/
	uCycOutputs->MREG_SPDFBI		= (USHORT)(pBaseCtrl->CtrlCmdMngr.SpdFBCx);
	uCycOutputs->MREG_SPDFBIH		= (USHORT)(pBaseCtrl->CtrlCmdMngr.SpdFBCx>>16);

	/*----------------------------------------------------------------------------------------------*/
	/*		Set Table Torque reference													*/
	/*----------------------------------------------------------------------------------------------*/
	uCycOutputs->MREG_TRQTBL	= (USHORT)(pBaseCtrl->TblDrive.var.TrqRef);
	uCycOutputs->MREG_TRQTBLH	= (USHORT)(pBaseCtrl->TblDrive.var.TrqRef>>16);

}
#endif


#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
/****************************************************************************
  Input the monitor data from JL-077 ScanA
*****************************************************************************/
void MicroSA_InputCycDataA(ASICS *SvAsicRegs, MENCV *MencV, CTRL_LOOP_OUT *CtrlLoopOut, BPRMDAT *Bprm)
{
	MICRO_CYC_INPUTS	*uCycInputs;
	ASIC_U_IF_STR		*pAsicMicroReg;
	DWORDX				pos;

	pAsicMicroReg	= SvAsicRegs->AsicMicroReg;
	uCycInputs		= (MICRO_CYC_INPUTS*)&pAsicMicroReg->MREG_BITDAT;

	pos.w.l = uCycInputs->MREG_RSPMOTPOS;
	pos.w.h = uCycInputs->MREG_RSPMOTPOSH;

	/* Torque limit/μqAxis current command	*/
	CtrlLoopOut->IqRefMon = (LONG)((SHORT)uCycInputs->MREG_IQMON) * Bprm->DirSign;
	CtrlLoopOut->IdRefMon = (LONG)((SHORT)uCycInputs->MREG_IDO);
	CtrlLoopOut->TrqRefMon = MlibMulhigh32( (CtrlLoopOut->IqRefMon << 16), NORM_IQ_TO_TRQ );
	MencV->MotAbsPos += (pos.Long - MencV->MotPos);
	MencV->MotPos = pos.Long;
}
#endif


#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
/****************************************************************************
  Output the command to JL-077 ScanA
*****************************************************************************/
void MicroSA_OutputCycDataA(ASICS *SvAsicRegs, BASE_CTRL_HNDL *pBaseCtrl)
{
	ASIC_U_IF_STR		*pAsicMicroReg;
	MICRO_CYC_OUTPUTS	*uCycOutputs;

	pAsicMicroReg	= SvAsicRegs->AsicMicroReg;
	uCycOutputs		= (MICRO_CYC_OUTPUTS*)&pAsicMicroReg->MREG_BITCMDI;

	if( pBaseCtrl->CtrlCmdMngr.CtrlMode == CTRLMODE_TRQ )	/* Torque control mode */
	{
		uCycOutputs->MREG_TRQREFI	= (USHORT)(pBaseCtrl->CtrlCmdMngr.TrqRefo);
		uCycOutputs->MREG_TRQREFIH	= (USHORT)(pBaseCtrl->CtrlCmdMngr.TrqRefo>>16);
	}
}
#endif


/****************************************************************************
  Baseloop Output to JL-086 MicroProgram
*****************************************************************************/
void MicroIfInputCycData( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut, ASICS *SvAsicRegs )
{
	BPRMDAT			*Bprm;
	BOOL			BeComplete;
	SHORT			wkIqfb;
	SHORT			wkIdfb;

	Bprm = BaseLoops->Bprm;
	BeComplete = (BaseLoops->BaseCtrls)->BeComplete;

/*--------------------------------------------------------------------------------------------------*/
/*		Latch ASIC Pulse Counter																	*/
/*--------------------------------------------------------------------------------------------------*/
	//SHalDrv_LatchPulseCntr( SvAsicRegs->AsicHwReg );

#ifndef _JL076_USE_
/*--------------------------------------------------------------------------------------------------*/
/*		Final current command acquisition process in micro from ASIC microprogram								*/
/*--------------------------------------------------------------------------------------------------*/
	if( Bprm->RvsDir )
	{ /* Reverse rotation mode */
		/* Final in microprogramdAxis current command */
		CtrlLoopOut->IdRefMon = SvAsicRegs->AsicMicroReg->WeakFV.IdOut;
		/* Final in microprogramqAxis current command */
		CtrlLoopOut->IqRefMon = -SvAsicRegs->AsicMicroReg->IntAdV.IqMon;

#if CSW_POWER_MONITOR_USE == TRUE /* for Power consumption monitor <S020> */
		BaseLoops->PowMonVq = -SvAsicRegs->AsicMicroReg->VcmpV.VqOut;
		BaseLoops->PowMonIqd = -SvAsicRegs->AsicMicroReg->IntAdV.IqInData;
		BaseLoops->PowMonVd = SvAsicRegs->AsicMicroReg->VcmpV.VdOut;
		BaseLoops->PowMonIdd = SvAsicRegs->AsicMicroReg->IntAdV.IdInData;
#endif

	}
	else
	{ /* Positive rotation mode */
		/* Final in microprogramdAxis current command */
		CtrlLoopOut->IdRefMon =  SvAsicRegs->AsicMicroReg->WeakFV.IdOut;
		/* Final in microprogramqAxis current command */
		CtrlLoopOut->IqRefMon =  SvAsicRegs->AsicMicroReg->IntAdV.IqMon;

#if CSW_POWER_MONITOR_USE == TRUE /* for Power consumption monitor <S020> */
		BaseLoops->PowMonVq = SvAsicRegs->AsicMicroReg->VcmpV.VqOut;
		BaseLoops->PowMonIqd = SvAsicRegs->AsicMicroReg->IntAdV.IqInData;
		BaseLoops->PowMonVd = SvAsicRegs->AsicMicroReg->VcmpV.VdOut;
		BaseLoops->PowMonIdd = SvAsicRegs->AsicMicroReg->IntAdV.IdInData;
#endif
	}
	/*----------------------------------------------------------------------------------------------*/
	CtrlLoopOut->IqRefMon_a = ( CtrlLoopOut->IqRefMon + CtrlLoopOut->IqRefMon_l ) >> 1;
	CtrlLoopOut->IqRefMon_l = CtrlLoopOut->IqRefMon;

/*--------------------------------------------------------------------------------------------------*/
/*		Current for monitorFBValue processing:BB, monitor currentFBThe value should be parameterized.0,										*/
/*			⇒UnBecause the final held value of current detection is output when the servo is off in monitor and trace data		*/
/*--------------------------------------------------------------------------------------------------*/
	wkIqfb = SvAsicRegs->AsicMicroReg->IntAdV.IqInData;			/* Inside microqAxis currentFB				*/
	wkIdfb = SvAsicRegs->AsicMicroReg->IntAdV.IdInData;			/* Inside microdAxis currentFB				*/
	if( BeComplete == FALSE )
	{ /* BaseBlockMedium */
		/* Current for monitorFBvalue = 0 */
		CtrlLoopOut->IqFbMon = 0;
		CtrlLoopOut->IdFbMon = 0;
	}
	else
	{ /* Base Enable */
		/* Current for monitorFBValue setting */
		CtrlLoopOut->IqFbMon = wkIqfb;
		CtrlLoopOut->IdFbMon = wkIdfb;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		qAxis current command monitor value acquisition process																	*/
/*--------------------------------------------------------------------------------------------------*/
	CtrlLoopOut->IqRefFilMon = SvAsicRegs->AsicMicroReg->IntAdV.IqMonFil;
	/* qAxis current command monitor(After filter)Sign calculation */
	CtrlLoopOut->IqRefFilMon *= Bprm->DirSign;

/*--------------------------------------------------------------------------------------------------*/
/*		Torque limit information acquisition process																	*/
/*--------------------------------------------------------------------------------------------------*/
	CtrlLoopOut->TrqClamp = ((SvAsicRegs->AsicMicroReg->StsFlg.CtrlStsRW & 0x0020) != 0) ? TRUE : FALSE; /* <S04B> */

#else /* _JL076_USE_ */

/*--------------------------------------------------------------------------------------------------*/
/*		Final current command acquisition process in micro from ASIC microprogram								*/
/*--------------------------------------------------------------------------------------------------*/
	if( Bprm->RvsDir )
	{ /* Reverse rotation mode */
		/* Final in microprogramdAxis current command */
		CtrlLoopOut->IdRefMon = SvAsicRegs->AsicMicroReg->MREG_IDO;
		/* Final in microprogramqAxis current command */
		CtrlLoopOut->IqRefMon = -SvAsicRegs->AsicMicroReg->MREG_IQMON;
	}
	else
	{ /* Positive rotation mode */
		/* Final in microprogramdAxis current command */
		CtrlLoopOut->IdRefMon =  SvAsicRegs->AsicMicroReg->MREG_IDO;
		/* Final in microprogramqAxis current command */
		CtrlLoopOut->IqRefMon =  SvAsicRegs->AsicMicroReg->MREG_IQMON;
	}
	/*----------------------------------------------------------------------------------------------*/
	CtrlLoopOut->IqRefMon_a = ( CtrlLoopOut->IqRefMon + CtrlLoopOut->IqRefMon_l ) >> 1;
	CtrlLoopOut->IqRefMon_l = CtrlLoopOut->IqRefMon;

/*--------------------------------------------------------------------------------------------------*/
/*		Current for monitorFBValue processing:BB, monitor currentFBThe value should be parameterized.0,										*/
/*			⇒UnBecause the final held value of current detection is output when the servo is off in monitor and trace data		*/
/*--------------------------------------------------------------------------------------------------*/
	wkIqfb = SvAsicRegs->AsicMicroReg->MREG_IQD;				/* Inside microqAxis currentFB				*/
	wkIdfb = SvAsicRegs->AsicMicroReg->MREG_IDD;				/* Inside microdAxis currentFB				*/
	if( BeComplete == FALSE )
	{ /* BaseBlockMedium */
		/* Current for monitorFBvalue = 0 */
		CtrlLoopOut->IqFbMon = 0;
		CtrlLoopOut->IdFbMon = 0;
	}
	else
	{ /* Base Enable */
		/* Current for monitorFBValue setting */
		CtrlLoopOut->IqFbMon = wkIqfb;
		CtrlLoopOut->IdFbMon = wkIdfb;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		qAxis current command monitor value acquisition process																	*/
/*--------------------------------------------------------------------------------------------------*/
	CtrlLoopOut->IqRefFilMon = SvAsicRegs->AsicMicroReg->MREG_IQMONFIL;
	/* qAxis current command monitor(After filter)Sign calculation */
	CtrlLoopOut->IqRefFilMon *= Bprm->DirSign;

#endif /* _JL076_USE_ */

/* <S1B7> */
/*--------------------------------------------------------------------------------------------------*/
/*		Current data setting for motorless test mode														*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseLoops->MotorLessTest->MotorLessTestMode == TRUE )
	{
		ApiSetCurDatasMotorLessTestMode( BaseLoops, CtrlLoopOut );
	}

	return;
}


/****************************************************************************
  Baseloop Output to JL-086 MicroProgram
*****************************************************************************/
void MicroIfOutputCycData( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut, ASICS *SvAsicRegs )
{
	BASE_CTRL_HNDL	*BaseCtrls;
	PHASECMP		*PhaseComp;
	MENCV			*MencV;
	BPRMDAT			*Bprm;

	BaseCtrls = BaseLoops->BaseCtrls;
	PhaseComp = &(BaseLoops->PhaseComp);
	MencV = BaseLoops->MencV;
	Bprm = BaseLoops->Bprm;

#ifndef _JL076_USE_
/*--------------------------------------------------------------------------------------------------*/
/*		Motor phase output to ASIC microprogram [65536/360deg]									*/
/*--------------------------------------------------------------------------------------------------*/
	if( MencV->PhaseOrderUWV == FALSE )
	{
		/* Phase sequence:UVW */
		SvAsicRegs->AsicMicroReg->AdinV.PhaseHIn = MencV->MotPhaseX;
		/* Phase interpolation amount */
		SvAsicRegs->AsicMicroReg->PhaseV.PhaseIpIn = PhaseComp->var.PcmpdItplt;
		/* Phase interpolation flag */
		SvAsicRegs->AsicMicroReg->PhaseV.PhaseIpFIn = 0;
	}
	else
	{
		/* Phase sequence:UWV */
		SvAsicRegs->AsicMicroReg->AdinV.PhaseHIn = (0x8000 - MencV->MotPhaseX);
		/* Phase interpolation amount */
		SvAsicRegs->AsicMicroReg->PhaseV.PhaseIpIn = -PhaseComp->var.PcmpdItplt;
		/* Phase interpolation flag */
		SvAsicRegs->AsicMicroReg->PhaseV.PhaseIpFIn = 0;
	}
#else /* _JL076_USE_ */
	if( MencV->PhaseOrderUWV == FALSE )
	{
		/* Phase sequence:UVW */
		SvAsicRegs->AsicMicroReg->MREG_PHAIN = MencV->MotPhaseX;
		/* Phase interpolation amount */
		SvAsicRegs->AsicMicroReg->MREG_PHAIPIN = PhaseComp->var.PcmpdItplt;
		/* Phase interpolation flag */
		SvAsicRegs->AsicMicroReg->MREG_PHAIPFLGIN = 0;
	}
	else
	{
		/* Phase sequence:UWV */
		SvAsicRegs->AsicMicroReg->MREG_PHAIN = (0x8000 - MencV->MotPhaseX);
		/* Phase interpolation amount */
		SvAsicRegs->AsicMicroReg->MREG_PHAIPIN = -PhaseComp->var.PcmpdItplt;
		/* Phase interpolation flag */
		SvAsicRegs->AsicMicroReg->MREG_PHAIPFLGIN = 0;
	}
#endif /* _JL076_USE_ */

/*--------------------------------------------------------------------------------------------------*/
/*		Current command/current limit value output processing to ASIC microprogram									*/
/*--------------------------------------------------------------------------------------------------*/
#ifndef _JL076_USE_
	if( Bprm->RvsDir )
	{ /* Reverse rotation mode */
		/* dAxis current command [15000/Imax] */
		SvAsicRegs->AsicMicroReg->AdinV.IdIn = BaseLoops->IdRef;
		/* qAxis current command [15000/Imax] */
		SvAsicRegs->AsicMicroReg->AdinV.IqIn = -BaseLoops->IqRef;

		/* Positive torque(Current)Limit value */
		SvAsicRegs->AsicMicroReg->AdinV.TLimPIn =
					(USHORT)MlibMulhigh32( BaseCtrls->TrqLimitData.var.NegTrqLmtOut, -3840000 );
		/* Negative side torque(Current)Limit value */
		SvAsicRegs->AsicMicroReg->AdinV.TLimMIn =
					(USHORT)MlibMulhigh32( BaseCtrls->TrqLimitData.var.PosTrqLmtOut,  3840000 );
	}
	else
	{ /* Positive rotation mode */
		/* dAxis current command [15000/Imax] */
		SvAsicRegs->AsicMicroReg->AdinV.IdIn = BaseLoops->IdRef;
		/* qAxis current command [15000/Imax] */
		SvAsicRegs->AsicMicroReg->AdinV.IqIn = +BaseLoops->IqRef;

		/* Positive torque(Current)Limit value */
		SvAsicRegs->AsicMicroReg->AdinV.TLimPIn =
					(USHORT)MlibMulhigh32( BaseCtrls->TrqLimitData.var.PosTrqLmtOut,  3840000 );
		/* Negative side torque(Current)Limit value */
		SvAsicRegs->AsicMicroReg->AdinV.TLimMIn =
					(USHORT)MlibMulhigh32( BaseCtrls->TrqLimitData.var.NegTrqLmtOut, -3840000 );
	}


#else /* _JL076_USE_ */
	if( Bprm->RvsDir )
	{ /* Reverse rotation mode */
		/* dAxis current command [15000/Imax] */
		SvAsicRegs->AsicMicroReg->MREG_IDIN = BaseLoops->IdRef;
		/* qAxis current command [15000/Imax] */
		SvAsicRegs->AsicMicroReg->MREG_IQIN = -BaseLoops->IqRef;

		/* Positive torque(Current)Limit value */
		SvAsicRegs->AsicMicroReg->MREG_TLMTPIN =
					(USHORT)MlibMulhigh32( BaseCtrls->TrqLimitData.var.NegTrqLmtOut, -3840000 );
		/* Negative side torque(Current)Limit value */
		SvAsicRegs->AsicMicroReg->MREG_TLMTMIN =
					(USHORT)MlibMulhigh32( BaseCtrls->TrqLimitData.var.PosTrqLmtOut,  3840000 );
	}
	else
	{ /* Positive rotation mode */
		/* dAxis current command [15000/Imax] */
		SvAsicRegs->AsicMicroReg->MREG_IDIN = BaseLoops->IdRef;
		/* qAxis current command [15000/Imax] */
		SvAsicRegs->AsicMicroReg->MREG_IQIN = +BaseLoops->IqRef;

		/* Positive torque(Current)Limit value */
		SvAsicRegs->AsicMicroReg->MREG_TLMTPIN =
					(USHORT)MlibMulhigh32( BaseCtrls->TrqLimitData.var.PosTrqLmtOut,  3840000 );
		/* Negative side torque(Current)Limit value */
		SvAsicRegs->AsicMicroReg->MREG_TLMTMIN =
					(USHORT)MlibMulhigh32( BaseCtrls->TrqLimitData.var.NegTrqLmtOut, -3840000 );
	}

#endif /* _JL076_USE_ */

// <S1B7> Complete deletion (MicroIfInputCycData(), and output the set voltage with)
//#if 0 /* ★Currently does not support motorless test mode★ */
///*--------------------------------------------------------------------------------------------------*/
///*		Current data setting for motorless test mode														*/
///*--------------------------------------------------------------------------------------------------*/
//	if(Kprm.f.MotorLessTestMode)
//	{
//		ApiSetCurDatasMotorLessTestMode( );
//	}
//#endif /* Currently does not support motorless test mode */

/*--------------------------------------------------------------------------------------------------*/
/*		Voltage compensation gain output to ASIC microprogram											*/
/*--------------------------------------------------------------------------------------------------*/
	if( BaseCtrls->MpKvvComp.conf.MpKvvFlg )
	{
		/* Voltage compensation gain output [8192/VdcBase] */
#ifndef _JL076_USE_
		SvAsicRegs->AsicMicroReg->IntAdP.KvvIn = BaseCtrls->MpKvvComp.var.MpKvv;
#else /* _JL076_USE_ */
		SvAsicRegs->AsicMicroReg->MREG_KVVIN = BaseCtrls->MpKvvComp.var.MpKvv;
#endif /* _JL076_USE_ */
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Field weakening control related variable output to ASIC microprogram		<S024>							*/
/*--------------------------------------------------------------------------------------------------*/
	/* VoltageFBProportional gain(Lower16bit) */
	SvAsicRegs->AsicMicroReg->WeakFV.WfKpVLIn = BaseCtrls->WeakenField.var.KpvL;
	/* VoltageFBProportional gain(Upper level16bit) */
	SvAsicRegs->AsicMicroReg->WeakFV.WfKpVHIn = BaseCtrls->WeakenField.var.KpvH;
	/* VoltageFBIntegral gain(Lower16bit) */
	SvAsicRegs->AsicMicroReg->WeakFV.WfKiVLIn = BaseCtrls->WeakenField.var.KivL;
	/* VoltageFBIntegral gain(Upper level16bit) */
	SvAsicRegs->AsicMicroReg->WeakFV.WfKiVHIn = BaseCtrls->WeakenField.var.KivH;
	/* Voltage command limit value */
	SvAsicRegs->AsicMicroReg->WeakFV.WfV1MaxIn = BaseCtrls->WeakenField.conf.V1Max;
	/* dAxis current command limit */
	SvAsicRegs->AsicMicroReg->WeakFV.WfIdRefLimIn = BaseCtrls->WeakenField.var.IdrefLim;

/*--------------------------------------------------------------------------------------------------*/
/*		qAxis current command monitor,qAxis disturbance torque writing process												*/
/*--------------------------------------------------------------------------------------------------*/
	/* qAxis disturbance torque command sign calculation */
	BaseLoops->IqDist *= (SHORT)Bprm->DirSign;
	/* qAxis disturbance torque command setting */
#ifndef _JL076_USE_
	SvAsicRegs->AsicMicroReg->IntAdV.IqDistIn = BaseLoops->IqDist;
#else /* _JL076_USE_ */
	SvAsicRegs->AsicMicroReg->MREG_IQDISTIN = BaseLoops->IqDist;
#endif /* _JL076_USE_ */

#if 0 /* 2012.09.04 Y.Oka ★Analog monitor currently not supported★ */
/*--------------------------------------------------------------------------------------------------*/
/*		ASIC final current command & ASIC data reading for analog monitor									*/
/*--------------------------------------------------------------------------------------------------*/
	BinK.AsicMon0 = ASIC_RD( *Gprm.AsicMonAdr0 );		/* Analog monitorCH0						*/
	BinK.AsicMon1 = ASIC_RD( *Gprm.AsicMonAdr1 );		/* Analog monitorCH1						*/
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Synchronization command to ASIC micro																*/
/*--------------------------------------------------------------------------------------------------*/
	SvAsicRegs->AsicHwReg->MREG_HSUR = MPCMD_SYNC1;		/* Synchronization command : Host-CPU --> Asic-CPU			*/
	return;
}



/***************************************** end of file **********************************************/

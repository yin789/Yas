/****************************************************************************************************/
/*																									*/
/*																									*/
/*		DataTraceCalc.c : Parameter calculation process definition													*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Parameter calculation process																		*/
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
/*	Note	:	First edition	2010.11.01	T.Kira	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "MercuryGlobals.h"
#include "NetworkIf.h"
#include "DataTraceCalc.h"
#include "DataTraceManager.h"
#include "UnMonFunc.h"
/* <S164> Start */
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
#include "ScanN.h"
#endif
/* <S164> End */

/****************************************************************************************************/
/*																									*/
/*			Trace variable acquisition function (numeric trace table)											*/
/*																									*/
/*			Returns numerical data to be stored in the trace buffer[-]											*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		0x01 :Torque/Thrust monitor																		*/
/*		0x81 :Torque/Thrust monitor																		*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonTrqRef( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->UnMonitor->AmonTrqRef);
	//return (((AXIS_HANDLE*)Axis)->BaseControls->TrqMotFricComp);//S0CD
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x02:Motor speed																			*/
/*		0x82:Motor speed																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_MotSpdSmp( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->MotSts.MotSpd);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x03:Speed command																				*/
/*		0x83:Speed command																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonSpdRef( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->UnMonitor->AmonSpdRef);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x04:Position command speed																			*/
/*		0x84:Position command speed																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_dPosCmdSmp( void *Axis )
{
	//return (((AXIS_HANDLE*)Axis)->BaseControls->PosManager.var.dPcmda);
	return (((AXIS_HANDLE*)Axis)->BaseControls->AmondPosCmd);	/* <S1B6> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x05:Position deviation																				*/
/*		0x85:Position deviation																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonPosErra( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosManager.var.PosErrA.per32s);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x13:Friction compensation torque/thrust														<S187>		*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonDisTrq( void *Axis )
{
	LONG	AmonDisTrq;

	if(	((AXIS_HANDLE*)Axis)->BaseControls->TuneLessCtrl.conf.TuningLessUse == TRUE )
	{
		AmonDisTrq = -(((AXIS_HANDLE*)Axis)->BaseControls->TuneLessCtrl.Drcomp.var.DisTrqCmp);
	}
	else
	{
		AmonDisTrq = -(((AXIS_HANDLE*)Axis)->BaseLoops->BaseCtrls->DobsCtrl.var.DisTrqCmp);
	}

	return (AmonDisTrq);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x14:Damping differential speed																				*/
/*		0x94:Damping differential speed																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_MonResVib( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->ResVib.var.MonResVib);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x15:TrqGravity																		<S0CD>	*/
/*		0x95:TrqGravity																		<S0CD>	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrqGravity( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->TrqMotFricComp);
	
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x16:Position amplifier deviation																			*/
/*		0x96:Position amplifier deviation																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonPosErrx( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosManager.var.PosErr);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x17:Motor load positional deviation																	*/
/*		0x97:Motor load positional deviation																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_sfperra_per32s( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->CheckAlarm->OvrFPosErr.sfperra.per32s);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x18:Speed feedforward																	*/
/*		0x98:Speed feedforward																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_SpdFFCx( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->CtrlCmdMngr.SpdFFCx);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x19:Torque/thrust feedforward															*/
/*		0x99:Torque/thrust feedforward															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrqFFCx( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->CtrlCmdMngr.TrqFFCx);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x1E:Torque command(Before disturbance)																<S187>	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrefBeforeDtrq( void *Axis )
{
	LONG TrefBeforeDtrq;

	switch ( ((AXIS_HANDLE*)Axis)->BaseControls->TblDrive.conf.TrqInputPos )
	{
	case 1: /* Disturbance input: Before torque command filter */
		TrefBeforeDtrq = (((AXIS_HANDLE*)Axis)->BaseLoops->TrqRefi);
		break;

	case 2: /* Disturbance input: After low-pass filter and before notch filter */
		TrefBeforeDtrq = ((AXIS_HANDLE*)Axis)->BaseLoops->TrqFil.V.FilOut;
		break;

	case 3: /* Disturbance input: After torque command filter */
//		TrefBeforeDtrq = ((AXIS_HANDLE*)Axis)->CtrlLoopOut->IqRefFilMon24;
		TrefBeforeDtrq = ((AXIS_HANDLE*)Axis)->CtrlLoopOut->IqRefFilMon;
		break;

	default:
		TrefBeforeDtrq = 0;
		break;
	}

	return (TrefBeforeDtrq);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x1F:Torque command - Friction compensation torque														<S187>	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrcCtrlTrqRef( void *Axis )
{
	LONG	AmonDisTrq;
	LONG	TrcCtrlTrqRef;

	AmonDisTrq = GetVarData_AmonDisTrq( (void*)Axis );

	TrcCtrlTrqRef = ((AXIS_HANDLE*)Axis)->BaseLoops->TrqFil.V.FilOut
		          + ((AXIS_HANDLE*)Axis)->BaseLoops->TrqFFCx - AmonDisTrq;

	return (TrcCtrlTrqRef);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x20:Torque command(Before disturbance/before filter)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrqRefi( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseLoops->TrqRefi);									/* <S187>	*/
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x21:Valid gain																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AmonActGain( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->GainChange.AmonActGain);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x22:Position command speed before position loop														<S187>	*/
/*		0xA2:Position command speed before position loop																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_dPosRefi( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosManager.dPosRefi);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x23:Main circuit bus voltage																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_DcVoltx( void *Axis )
{
	LONG pValue;
	UnReadDCVoltage(PRM_ACCCMD_READ, 0, Axis, &pValue);
	return pValue;
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x24:Current detection value																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_UnCrntDet( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->UnMonitor->UnCrntDet);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x26:Cumulative load rate																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_UnCrntRef( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->UnMonitor->UnCrntRef);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x27:Regeneration load rate																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrcRgPower( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->UnMonitor->TrcRgPower);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x28:DBResistor power consumption																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrcDbPower( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->UnMonitor->TrcDbPower);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x29:External encoder speed																		*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_FencSpdSmp( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->MotSts.MotSpd);		/* <S04D> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x41:Command difference																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_dPcmdEgear( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosManager.var.dPcmdEgear);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x42:Motor position difference																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_DeltaPos( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->MotSts.dMotPos);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x43:Load position difference																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_dFencPos( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->CtrlCmdMngr.PosFbki);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x44:Current phase																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_MotPhase( void *Axis )
{
#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
	LONG MotPhase;

	MotPhase = ((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicMicroReg->MREG_MOTPHASEX;
	MotPhase += (((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicMicroReg->MREG_MOTPHASEXH << 16);
	return MotPhase;
#else
//<S164>	return 0;
	return (((AXIS_HANDLE*)Axis)->MencV->MotPhase);	/* <S164> */
#endif
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x60:Internal signal monitor(Lower)																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_InterSignalX_L( void *Axis )
{
	LONG pValue;
	UnReadInterSignalX( PRM_ACCCMD_READ, 0, Axis, &pValue);

	return (pValue & 0x0000FFFF);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x61:Internal signal monitor(Upper level)																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_InterSignalX_H( void *Axis )
{
	LONG pValue;
	UnReadInterSignalX( PRM_ACCCMD_READ, 0, Axis, &pValue);

	return (pValue >> 16);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x62:Internal input signal monitor(Lower)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_InternalInputSignals_L( void *Axis )
{
	LONG pValue;
	UnReadInternalInputSignals( PRM_ACCCMD_READ, 0, Axis, &pValue);

	return (pValue & 0x0000FFFF);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x63:Internal input signal monitor(Upper level)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_InternalInputSignals_H( void *Axis )
{
	LONG pValue;
	UnReadInternalInputSignals( PRM_ACCCMD_READ, 0, Axis, &pValue);

	return (pValue >> 16);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x64:Internal output signal monitor(Lower)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_InternalOutputSignals_L( void *Axis )
{
	LONG pValue;
	UnReadInternalOutputSignals( PRM_ACCCMD_READ, 0, Axis, &pValue);

	return (pValue & 0x0000FFFF);
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x65:Internal output signal monitor(Upper level)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_InternalOutputSignals_H( void *Axis )
{
	LONG pValue;
	UnReadInternalOutputSignals( PRM_ACCCMD_READ, 0, Axis, &pValue);

	return (pValue >> 16);
}

/* <S14B> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		0xAA:Speed ripple Torque before compensation/Thrust command														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_VelRippleTrqRefBeforeComp( void *Axis )
{
//	return (((AXIS_HANDLE*)Axis)->BaseLoops->RippleCmp.AmonTrqComp1);
	return (((AXIS_HANDLE*)Axis)->BaseLoops->RippleCmp.RippleTrqCompMon);/*<S1FD>*/
}

/*--------------------------------------------------------------------------------------------------*/
/*		0xC4:Current phase																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_CurrentPhase( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->MencV->MotPhaseX);
}
/* <S14B> End */

/* <S164> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		0xC5:Main circuit bus voltage(Before filter)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_TrcDcVoltFili( void *Axis )
{
#if (FLOAT_USE==TRUE)
	float	DcVoltFili;

	DcVoltFili = ((AXIS_HANDLE*)Axis)->PowerManager->DetectDcVolt.DcVoltx *
	                      ((AXIS_HANDLE*)Axis)->PowerManager->DetectDcVolt.conf.Kdcvolt;

	return( DcVoltFili );

#else
	LONG	DcVoltFili;

	DcVoltFili = MlibMulgain(((AXIS_HANDLE*)Axis)->PowerManager->DetectDcVolt.DcVoltx,
	                      ((AXIS_HANDLE*)Axis)->PowerManager->DetectDcVolt.conf.Kdcvolt );

	return( DcVoltFili );

#endif /* FLOAT_USE */
//	return PRM_RSLT_SUCCESS;
}
/* <S164> End */

#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
/*--------------------------------------------------------------------------------------------------*/
/*		0xD0:M-ⅢFeedback position(APOS)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Apos( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.psnHdl.Apos);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD1:M-ⅢCurrent command position(CPOS)																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cpos( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.mtgHdl.Cspd);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD2:M-ⅢTarget position(TPOS)																		*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Tpos( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.psnHdl.Tpos);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD3:M-ⅢLatch position1(LPOS1)																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Lpos1( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.psnHdl.Lpos[0]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD4:M-ⅢLatch position2(LPOS2)																	*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Lpos2( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.psnHdl.Lpos[1]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD5:M-ⅢFeedback speed(FSPD)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Fspd( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.psnHdl.Fbspd);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD6:M-ⅢCurrent position command speed(CSPD)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cspd( void *Axis )
{
	//Hold
	return 0;
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD7:M-ⅢTorque/Thrust command(TRQ)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Trq( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.cmdData.Trq);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD8:M-ⅢTorque limit command(TRQ_LIM)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_TrqLim( void *Axis )
{
	//Hold
	return 0;
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xD9:M-ⅢSpeed limit command(V_LIM)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_VLim( void *Axis )
{
	if( ((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.cmdData.CtrlMode == MT_TRQ_CTRLMD )
	{
		return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.cmdData.Velocity);
	}
	else
	{
		return 0;
	}
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xDA:M-ⅢCurrently occurring alarm code(ALARM)														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_M3_Alam( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->alm_hdl.errCode);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xDB:M-ⅢCurrently executing command code(RCMD)														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_RCmd( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->cmd_hdl.cmdInfo.crtCmd);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xDC:M-ⅢCurrently executing subcommand code(SUB_RCMD)												*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_SubRCmd( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->cmd_hdl.exCmdInfo.cmdBuf.stdData.mon.cmd);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE0:M-ⅢCommand data( 0~ 3byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_0( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[0]);
}

/*--------------------------------------------------------------------------------------------------*/
/*		0xE1:M-ⅢCommand data( 4~ 7byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_1( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[1]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE2:M-ⅢCommand data( 8~11byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_2( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[2]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE3:M-ⅢCommand data(12~15byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_3( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[3]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE4:M-ⅢCommand data(16~19byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_4( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[4]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE5:M-ⅢCommand data(20~23byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_5( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[5]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE6:M-ⅢCommand data(24~27byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_6( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[6]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE7:M-ⅢCommand data(28~31byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_7( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[7]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE8:M-ⅢCommand data(32~35byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_8( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[8]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xE9:M-ⅢCommand data(36~39byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_9( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[9]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xEA:M-ⅢCommand data(40~43byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_a( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[10]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xEB:M-ⅢCommand data(44~47byte)															*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Cmd_b( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.CmdBufPtr[11]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF0:M-ⅢResponse data( 0~ 3byte) 														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_0( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[0]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF1:M-ⅢResponse data( 4~ 7byte) 														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_1( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[1]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF2:M-ⅢResponse data( 8~11byte) 														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_2( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[2]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF3:M-ⅢResponse data(12~15byte) 														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_3( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[3]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF4:M-ⅢResponse data(16~19byte) 														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_4( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[4]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF5:M-ⅢResponse data(20~23byte)														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_5( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[5]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF6:M-ⅢResponse data(24~27byte)														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_6( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[6]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF7:M-ⅢResponse data(28~31byte) 														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_7( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[7]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF8:M-ⅢResponse data(32~35byte)														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_8( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[8]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xF9:M-ⅢResponse data(36~39byte)														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_9( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[9]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xFA:M-ⅢResponse data(40~43byte) 														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_a( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[10]);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xFB:M-ⅢResponse data(44~47byte) 														*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_M3_Rsp_b( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->com_hdl.RspBufPtr[11]);
}
#endif //CSW_NETIF_TYPE == NETIF_M3_TYPE

/*--------------------------------------------------------------------------------------------------*/
/*		0x7F:Real address specification																			*/
/*		0xFF:Real address specification																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_AdrShortData( void *Axis )
{/*	SHORTData	*/

	return (*(((AXIS_HANDLE*)Axis)->TrcHndl->TrcExeP.TrcObj->Adr.ShortAdrData));
}
LONG	GetVarData_AdrLongData( void *Axis )
{/*LONGData	*/
	return (*(((AXIS_HANDLE*)Axis)->TrcHndl->TrcExeP.TrcObj->Adr.LongAdrData));
}

/*--------------------------------------------------------------------------------------------------*/
/*		****:Dummy variable																				*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_Dummy( void *Axis )
{
	return 0;
}

#if 0 /* 2012.08.19 Y.Oka Make Mercury Base Software */
#if (CSW_NETIF_TYPE == NETIF_SYNQNET_TYPE)
/*--------------------------------------------------------------------------------------------------*/
/*		****:Real address specification(SynqNet)																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	GetVarData_SqnetAdrData( void *Axis )
{
	return ( *((((AXIS_HANDLE*)Axis)->SqnetHandle->SqnetTrace.SqTrcObj[0].PhyAddr)));
}
LONG	GetVarData_SqnetAdrData2( void *Axis )
{
	return ( *((((AXIS_HANDLE*)Axis)->SqnetHandle->SqnetTrace.SqTrcObj[1].PhyAddr)));
}
#endif
#endif /* 2012.08.19 Y.Oka Make Mercury Base Software */

/****************************************************************************************************/
/*																									*/
/*			Trace variable gain calculation function (numeric trace table)									*/
/*																									*/
/*			Gain calculation for unit conversion of trace variable													*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		0x02:Motor speed																			*/
/*		0x82:Motor speed																			*/
/*--------------------------------------------------------------------------------------------------*/
/*							1																		*/
/*		Kx = ----------------------------										 					*/
/*				 TrcExeP.DataSampIntv 																*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxPcalMotSpeedCalGain2( void* Axis )
{
	LONG	kx;
	TRCHNDL *TrcHndl;

	TrcHndl = ((AXIS_HANDLE*)Axis)->TrcHndl;
	kx = MlibScalKxgain( 1, 1, TrcHndl->TrcExeP.DataSampIntv, NULL, 24 );
	return( kx );
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x04:Position command speed																			*/
/*		0x84:Position command speed																			*/
/*--------------------------------------------------------------------------------------------------*/
/*							Egear.b * Hprm.Kmotspd * 1000000										*/
/*		Kpcmdspd = ---------------------------------------------------								*/
/*					 Egear.a * KPI_SBCYCLEUS * TrcExeP.DataSampIntv 								*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxPcalPcmdSpeedCalGain( void* Axis )
{
	LONG			kx, sx, plsno;
	LONG			Egear_a, Egear_b;
	AXIS_HANDLE		*AxHdl;
	TRCEXEP			*TrcExeP;
	LONG			SBCycleUs;
	BOOL			EgearDisable;

	AxHdl = ((AXIS_HANDLE*)Axis);
	TrcExeP = &((AXIS_HANDLE*)Axis)->TrcHndl->TrcExeP;
	SBCycleUs = (((AXIS_HANDLE*)Axis)->UniPrms->Bprm)->SvCycleUs;
	EgearDisable = (((AXIS_HANDLE*)Axis)->BaseControls->TblDrive.OpeReg.TblFuncSel & TBLFUNCBIT_EGEAR);

/*	//Currently not compatible with Fulk_kira_101117
	if( Kprm.f.FencUse )
	{
		plsno = AxHdl->prm_hndl_ptr->hprm_ptr->KmotspdFC;
	}
	else
	{
*/		plsno = AxHdl->UniPrms->Bprm->Kmotspd;
//	}

	if( EgearDisable == FALSE )
	{
		Egear_a = AxHdl->BaseControls->PosManager.conf.Egear.a;
		Egear_b = AxHdl->BaseControls->PosManager.conf.Egear.b;
		kx = MlibScalKskxkx( plsno, Egear_b, Egear_a, &sx,  0 );
	}
	else
	{ /* Electronic gear disabled */
		kx = MlibScalKskxkx( plsno, 1, 1, &sx,  0 );
	}
	kx = MlibPcalKxgain( kx, 1000000, (SBCycleUs * TrcExeP->DataSampIntv), &sx, 24 );
	return( kx );
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x22:Command speed before position loop																	*/
/*--------------------------------------------------------------------------------------------------*/
/*							Hprm.Kmotspd * 1000000													*/
/*		Kmotspd = ------------------------------------------					 					*/
/*					 KPI_SBCYCLEUS * TrcExeP.DataSampIntv 											*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxPcalMotSpeedCalGain( void* Axis )
{
	LONG	kx,plsno;
	AXIS_HANDLE *AxHdl;
	TRCEXEP *TrcExeP;
	LONG	SBCycleUs;

	AxHdl   =  ((AXIS_HANDLE*)Axis);
	TrcExeP = &((AXIS_HANDLE*)Axis)->TrcHndl->TrcExeP;
	SBCycleUs = (((AXIS_HANDLE*)Axis)->UniPrms->Bprm)->SvCycleUs;

	//Currently not compatible with Fulk_kira_101117
	//if( Kprm.f.FencUse )
	//{
	//	plsno = AxHdl->prm_hndl_ptr->hprm_ptr->KmotspdFC;
	//}
	//else
	//{
		plsno = AxHdl->UniPrms->Bprm->Kmotspd;
	//}

	kx = MlibScalKskxkx( plsno, 1000000,  (SBCycleUs * TrcExeP->DataSampIntv),  NULL, 24 );
	return( kx );
}


/****************************************************************************************************/
/*																									*/
/*			Bit trace variable acquisition function (bit trace table)									*/
/*																									*/
/*			Return the bit data of the specified axis.[-]														*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		0x01:Servo-on command(/S-ON)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_SvonReq( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BeSequence->SvonReq);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x02:PControl command(/P-CON)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pcon( void *Axis )
{
//	return (((AXIS_HANDLE*)Axis)->CtrlLoopOut->PConActFlg); /* <S064> */
	return ((((AXIS_HANDLE*)Axis)->BaseLoops->CmdCtrlBit & ENBPCTRL_BIT) ? TRUE : FALSE);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x03:Forward rotation prohibited(P-OT)																			*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pot( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE) /* <S064> */
	/* A/PFor typeNetIfsideI/OTo confirm the assignment,SoftOT */
	return ((((AXIS_HANDLE*)Axis)->BaseCtrlOut->CmdSeqBit) >> POSITIVE_OT_BITNO) & 0x1;
#else
	return (((AXIS_HANDLE*)Axis)->SeqCtrlOut->PotSigSts);
#endif
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x04:Reverse rotation prohibited(N-OT)																			*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Not( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE) /* <S064> */
	/* A/PFor typeNetIfsideI/OTo confirm the assignment,SoftOT */
	return ((((AXIS_HANDLE*)Axis)->BaseCtrlOut->CmdSeqBit) >> NEGATIVE_OT_BITNO) & 0x1;
#else
	return (((AXIS_HANDLE*)Axis)->SeqCtrlOut->NotSigSts);
#endif
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x05:Alarm reset(/ALMRST)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_AlmRst( void *Axis )
{
	//return (((AXIS_HANDLE*)Axis)->AlmManager->AlmRst);
	return( 0 != (((AXIS_HANDLE*)Axis)->BaseCtrlOut->CmdSeqBit & ALM_RESET_BIT ));		/* <S1A7>  */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x06:Positive torque limit(/P-CL)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pcl( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->TrqLimitData.var.PclSignal);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x07:Negative side torque limit(/N-CL)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Ncl( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->TrqLimitData.var.NclSignal);
}

/* <S164> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		0x08:SPD-D signal(/SPD-D)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_SpdD( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	return ( ((((AXIS_HANDLE*)Axis)->BaseCtrlOut->CmdSeqBit) >> SPDD_BITNO) & 0x1 );
#else
	return (0);
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x09:SPD-A signal(/SPD-A)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_SpdA( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	return ( ((((AXIS_HANDLE*)Axis)->BaseCtrlOut->CmdSeqBit) >> SPDA_BITNO) & 0x1 );
#else
	return (0);
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x0A:SPD-B signal(/SPD-B)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_SpdB( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	return ( ((((AXIS_HANDLE*)Axis)->BaseCtrlOut->CmdSeqBit) >> SPDB_BITNO) & 0x1 );
#else
	return (0);
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x0B:C-SEL signal(/C-SEL)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Csel( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	return ( ((((AXIS_HANDLE*)Axis)->BaseCtrlOut->CmdSeqBit) >> CSEL_BITNO) & 0x1 );
#else
	return (0);
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x0C:Zero clamp signal(/ZCLAMP)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Zclamp( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	return ( (SqiV.Sigs & SQI_SIGS_ZCLAMP ) >> SQI_BNO_ZCLAMP );
#else
	return (0);
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x0D:Pulse command ignored signal(/INHIBIT)															*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Inhibit( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	return ( (SqiV.Sigs & SQI_SIGS_INHIBIT ) >> SQI_BNO_INHIBIT );
#else
	return (0);
#endif
}
/* <S164> End */

/*--------------------------------------------------------------------------------------------------*/
/*		0x0E:G-SEL1Signal(/G-SEL1)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Gsel( void *Axis )
{
	//if( (((AXIS_HANDLE*)Axis)->BaseControls->GselNum) == 2)	/* <S1B4> */
	if( (((AXIS_HANDLE*)Axis)->BaseControls->GselNum) == 1)		/* <S1B4> */
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x0F:Magnetic pole detection command(/P-DET)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pdet( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->FnCmnCtrl->PdetReq);
}

/* <S164> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		0x11:Emergency stop(/E-STP)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Estp( void *Axis )
{
	return ( ((AXIS_HANDLE*)Axis)->IoSignals->EstpSignal.SignalStatus.v );
}
/* <S164> End */

/*--------------------------------------------------------------------------------------------------*/
/*		0x12:Deceleration signal(/DEC)																			*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Dec( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->IoSignals->DecSignal.SignalStatus.v);		/* <S045> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x13:EXT1Interrupt request(/EXT1)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Ext1( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->IoSignals->ExtSignal[0].SignalStatus.v);    /* <S053> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x14:EXT2Interrupt request(/EXT2)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Ext2( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->IoSignals->ExtSignal[1].SignalStatus.v);    /* <S053> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x15:EXT3Interrupt request(/EXT3)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Ext3( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->IoSignals->ExtSignal[2].SignalStatus.v);    /* <S053> */
}
/* <S1F5> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		0x16:FSTPInterrupt request(/FSTP)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Fstp( void *Axis)
{
//<S1F5>#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
//<S1F5>	return ( (SqiV.Sigs & SQI_SIGS_FSTP ) >> SQI_BNO_FSTP );
//<S1F5>#else
	return (((AXIS_HANDLE*)Axis)->IoSignals->FstpSignal.SignalStatus.v);
//<S1F5>#endif
}
/* <S1F5> End */

/*--------------------------------------------------------------------------------------------------*/
/*		0x19:SENSignal(SEN)																			*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Sen( void *Axis )
{
//	return (((AXIS_HANDLE*)Axis)->MencV->SenReady); /* <S064> */
	return ((((AXIS_HANDLE*)Axis)->BaseCtrlOut->CmdSeqBit) >> SENSOR_ON_BITNO) & 0x1;
}

/* <S164> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		0x1A:Pulse command input presence/absence(PULS)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Puls( void *Axis )
{
	return ((((AXIS_HANDLE*)Axis)->pPlsIfV->f.TrcPcmdInon) ? TRUE : FALSE);
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x1B:Sign command input presence/absence(SIGN)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Sign( void *Axis )
{
	return ((((AXIS_HANDLE*)Axis)->pPlsIfV->f.TrcPcmdSign) ? TRUE : FALSE);
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x1C:Deviation clear signal presence/absence(CLR)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Clr( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	NINV *pninv = &NinV;
	return ( pninv->NrmBits.CLR );
#else
	return 0;
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x1D:Command pulse input magnification switching signal(/PSEL)														*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Psel( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	return ( (SqiV.Sigs & SQI_SIGS_PSEL ) >> SQI_BNO_PSEL );
#else
	return 0;
#endif
}
/* <S164> End */

/*--------------------------------------------------------------------------------------------------*/
/*		0x20:HWBB1Signal(/HWBB1)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_HWBB1( void *Axis )
{
	/* Low Active */
	return ((((AXIS_HANDLE*)Axis)->IoSignals->DetHwbbSts.Bbon1) ? FALSE : TRUE);
//	return (((AXIS_HANDLE*)Axis)->SeqCtrlOut->HwbbSts); /* <S064> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x21:HWBB2Signal(/HWBB2)																		*//* S064 */
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_HWBB2( void *Axis )
{
	/* Low Active */
	return ((((AXIS_HANDLE*)Axis)->IoSignals->DetHwbbSts.Bbon2) ? FALSE : TRUE);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x80:Alarm occurrence(ALM)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_AlmFlag( void *Axis )
{
	return ((((AXIS_HANDLE*)Axis)->AlmManager->Status.AlmFlag) ? TRUE : FALSE); /* <S0FF> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x81:Positioning complete(/COIN)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Coin( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosCtrlSts.CoinSignal);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x82:Speed match(/V-CMP)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Vcmp( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->SeqCtrlOut->VcmpSts);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x83:Motor rotation detection(/TGON)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Tgon( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->SeqCtrlOut->TgOnSts);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x84:Servo ready(/S-RDY)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_SvRdy( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->SeqCtrlOut->SvRdyPhaseIgnore);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x85:Current limiting(CLT)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Clt( void *Axis )
{
//	return (((AXIS_HANDLE*)Axis)->CtrlLoopOut->TrqClamp);
	return (((AXIS_HANDLE*)Axis)->BaseControls->CtrlCmdMngr.TrqClamp); /* <S04B> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x86:Speed limit(/VLT)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Vlt( void *Axis )
{
//	return (((AXIS_HANDLE*)Axis)->CtrlLoopOut->SpdClamp);
	return (((AXIS_HANDLE*)Axis)->BaseControls->CtrlCmdMngr.SpdClamp); /* <S04B> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x87:Brake hold(/BK)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Bk( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->SeqCtrlOut->BrkSigSts);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x88:Warning occurs(/WARN)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_WrnFlag( void *Axis )
{
	return ((((AXIS_HANDLE*)Axis)->AlmManager->Status.WrnFlag) ? TRUE : FALSE);  /* <S0FF> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x89:Positioning vicinity(/NEAR)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Near( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosCtrlSts.NearSignal);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0x8A:CPhase output(/C-PLUS)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Cplue( void *Axis )
{
//	return (((AXIS_HANDLE*)Axis)->BaseControls->MotSts.MotCphPass);	/* <S04D> */
AXIS_HANDLE *axis;
BOOL motCphPhass;
	
	axis = (AXIS_HANDLE*)Axis;
	if(axis->UniPrms->Bprm->FencUse != FALSE)
	{
		motCphPhass = axis->BaseControls->MotSts.FencCphPass;
	}
	else
	{
		motCphPhass = axis->BaseControls->MotSts.MotCphPass;
	}
	return motCphPhass;

}

/* <S164> Start */
/*--------------------------------------------------------------------------------------------------*/
/*		0x8E:Alarm code output 1(AL01)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Alo1( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
//	return (( (((AXIS_HANDLE*)Axis)->AlmManager->AlmCodeOut.Alo1)? TRUE : FALSE));	/* <S1AF> */
	return (( (((AXIS_HANDLE*)Axis)->AlmManager->AlmCodeOut.Alo1)? FALSE : TRUE));	/* <S1AF> */
#else
	return (0);
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x8F:Alarm code output 2(AL02)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Alo2( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
//	return (( (((AXIS_HANDLE*)Axis)->AlmManager->AlmCodeOut.Alo2)? TRUE : FALSE));	/* <S1AF> */
	return (( (((AXIS_HANDLE*)Axis)->AlmManager->AlmCodeOut.Alo2)? FALSE : TRUE));	/* <S1AF> */
#else
	return (0);
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x90:Alarm code output 3(AL03)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Alo3( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
//	return (( (((AXIS_HANDLE*)Axis)->AlmManager->AlmCodeOut.Alo3)? TRUE : FALSE));	/* <S1AF> */
	return (( (((AXIS_HANDLE*)Axis)->AlmManager->AlmCodeOut.Alo3)? FALSE : TRUE));	/* <S1AF> */
#else
	return (0);
#endif
}


/*--------------------------------------------------------------------------------------------------*/
/*		0x91:Divided pulse output A phase(PAO)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pao( void *Axis )
{
	VULONG	AregFbcset;			/* ASIC JL-086 SERVOIP H/WRegister Division input filter setting(0x0040) */
	BOOL	PaoSig;

	AregFbcset = ((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicHwReg->AREG_FBCSET;
	PaoSig = (BOOL)((AregFbcset >> 9) & 0x0001);

	return( PaoSig );
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x92:Divided pulse output B phase(PBO)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pbo( void *Axis )
{
	VULONG	AregFbcset;			/* ASIC JL-086 SERVOIP H/WRegister Division input filter setting(0x0040) */
	BOOL	PboSig;

	AregFbcset = ((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicHwReg->AREG_FBCSET;
	PboSig = (BOOL)((AregFbcset >> 10) & 0x0001);

	return( PboSig );
}

/*--------------------------------------------------------------------------------------------------*/
/*		0x93:Divided pulse output C phase(PCO)																*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pco( void *Axis )
{
	VULONG	AregFbcset;			/* ASIC JL-086 SERVOIP H/WRegister Division input filter setting(0x0040) */
	BOOL	PcoSig;

	AregFbcset = ((AXIS_HANDLE*)Axis)->SvAsicRegs->AsicHwReg->AREG_FBCSET;
	PcoSig = (BOOL)((AregFbcset >> 11) & 0x0001);

	return( PcoSig );
}


/*--------------------------------------------------------------------------------------------------*/
/*		0x94:Command pulse input magnification switching response(/PSELA)														*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Psela( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_ANLGPLS_TYPE)
	return ( (SqoV.Sigs & SQO_SIGS_PSELA ) >> 19 );
#else
	return (0);
#endif
}
/* <S164> End */

/*--------------------------------------------------------------------------------------------------*/
/*		0xC0:Main circuit power on(ACON)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Acon( void *Axis )
{
	//return (((AXIS_HANDLE*)Axis)->SeqCtrlOut->MainPowerOn);	/* <S1B3> */
	return (((AXIS_HANDLE*)Axis)->PowerManager->PowerSts.Acon);	/* <S1B3> */
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xC1:Command dispensing zero(REFZ)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Refz( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosCtrlSts.RefZSignal);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xC2:Magnetic pole detection complete(PDETCMP)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pdetcmp( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->MencV->PhaseReady);
}
/*--------------------------------------------------------------------------------------------------*/
/*		0xC8:Command dispensing complete(DEN)																	*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Den( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
	if( (((AXIS_HANDLE*)Axis)->FnCmnCtrl->FnCtrlMcmd == CTRLMODE_PJOG) )
	{/* ProgramJOGDuring driving */
		return (((AXIS_HANDLE*)Axis)->BaseControls->PosCtrlSts.DenSignal);
	}
	else
	{/* MECHATROLINKWhen commanded */
		return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.psnHdl.Den);
	}
#else
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosCtrlSts.DenSignal);
#endif
}

/*--------------------------------------------------------------------------------------------------*/
/*		0xC9:Positioning complete(PSET)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_Pset( void *Axis )
{
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->mt_hdl.psnHdl.Pset);
#else
	return (((AXIS_HANDLE*)Axis)->BaseControls->PosCtrlSts.CoinSignal);
#endif
}
#if (CSW_NETIF_TYPE == NETIF_M3_TYPE)
/*--------------------------------------------------------------------------------------------------*/
/*		0xCA:M-ⅢCommand ready(CMDRDY)																		*/
/*--------------------------------------------------------------------------------------------------*/
BOOL	GetBitData_M3CmdRdy( void *Axis )
{
	return (((AXIS_HANDLE*)Axis)->NetAxisIf->cmd_hdl.cmdInfo.cmdRdy);
}
#endif //CSW_NETIF_TYPE == NETIF_M3_TYPE

/*--------------------------------------------------------------------------------------------------*/
/*		****:Dummy variable																				*/
/*--------------------------------------------------------------------------------------------------*/
//BOOL	GetBitData_Dummy( void *Axis )
//{
//	return FALSE;
//}

/***************************************** end of file **********************************************/

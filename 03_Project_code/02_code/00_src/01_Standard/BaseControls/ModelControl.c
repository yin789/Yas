/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ModelControl.c : Direct torque reference method model following control related processing definition					*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Model following control related processing																	*/
/*																									*/
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
#include "ModelControl.h"
#include "MLib.h"

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void	MfcInitModelControl( MFCTRL *MFControl );												*/
/*	LONG	MfcControlMain( MFCTRL *MFControl, EXP_CTRL_DATA *hExpCtrl,								*/
/*														BASE_CTRL_HNDL *pBaseCtrl, LONG dPcmd );	*/
/*	SHORT	MfcGetParamEnableSts( MFCTRL *MFControl );												*/
/*	void	MfcSetMfcCtrlSwitch( MFCTRL *MFControl, BOOL Switch );									*/
/****************************************************************************************************/
static	void	mfcControlOneMass( MFCTRL *MFControl, LONG dPcmd );
static	void	mfcControlTwoMass( MFCTRL *MFControl, LONG dPcmd );
static	LONG	mfcDtrCmdFilter( MFCTRL *MFControl, LONG pcmd );
/****************************************************************************************************/



/****************************************************************************************************/
/*																									*/
/*		MFC variable initialization																			*/
/*																									*/
/****************************************************************************************************/
void	MfcInitModelControl( MFCTRL *MFControl )
{
	MlibResetLongMemory( &(MFControl->var), sizeof(MFControl->var)/4 );
	/* Initialize parameters in the first step1Gain */
	MFControl->conf.MfcPrm[0] = MFControl->conf.MfcPrm[1];
	/* Initialize parameters for execution */
	MFControl->conf.MfcPexe = MFControl->conf.MfcPrm[0];
	return;
}



/****************************************************************************************************/
/*																									*/
/*		MFC calculation																					*/
/*																									*/
/*	Overview:	Perform MFC calculations for each reference model according to parameter settings									*/
/*			When a parameter is changed or the functionOFF->ON, FunctionON->OFFWhen					*/
/*			Switch with "position command complete" and "no accumulated pulses". To prevent positional deviation and reduce shock	*/
/*			Vibration suppression filter processing is performed before MFC processing. The input to the MFC is the position command after the vibration suppression filter.	*/
/*																									*/
/*																									*/
/* arg		: LONG	dPcmd				:	Position difference command		[pulse]								*/
/* out		: LONG	pcmdout				:	MFC post-position command	[pulse]								*/
/* g-out	: LONG	BoutV.SpdFFCx 		:	Speed FF compensation		[2^24/OvrSpd]						*/
/*			  LONG	BoutV.TrqFFCx		:	Torque FF compensation		[2^24/MaxTrq]						*/
/*																									*/
/****************************************************************************************************/
LONG	MfcControlMain( MFCTRL *MFControl, LONG dPcmd, LONG *SpdFFCx, LONG *TrqFFCx )
{
	LONG	pcmdout;

/*--------------------------------------------------------------------------------------------------*/
/*	MFC forced invalidation process(MFCStop flag)																*/
/*--------------------------------------------------------------------------------------------------*/
	if( MFControl->var.MfcStopReq == TRUE )
	{
		MFControl->conf.MfcPrm[0].MFCModel = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*	Command completion & dispensing completion check																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (dPcmd == 0)
		&& (MFControl->var.MfcPosErr == 0)
		&& (MFControl->var.FCmd[0] == 0)
		&& (MFControl->var.FCmd[1] == 0)
		&& (MFControl->var.FCmd[2] == 0) )
	{ /* Command complete && No model position deviation->Payout completion */
		/* Execute parameter switching */
		MFControl->conf.MfcPexe = MFControl->conf.MfcPrm[0];
	}

/*--------------------------------------------------------------------------------------------------*/
/*	MFC calculation																						*/
/*--------------------------------------------------------------------------------------------------*/
	/* MFCProcessing counter update */
	MFControl->var.execnt++;

	switch( MFControl->conf.MfcPexe.MFCModel )
	{
	case ONE_MASS:		/*	Rigid body model			*/
		mfcControlOneMass( MFControl, dPcmd );
		MFControl->var.CurMFCModel = MFControl->conf.MfcPexe.MFCModel;
		break;
	case TWO_MASS1:		/*	2 inertia model			*/
	case TWO_MASS2:		/*	2 inertia model			*/
	case ONE_BASE:		/*	Rigid body + machine model		*/
		mfcControlTwoMass( MFControl, dPcmd );
		MFControl->var.CurMFCModel = MFControl->conf.MfcPexe.MFCModel;
		break;
	default:
		MFControl->conf.MfcPexe.MFCModel = 0;
		MFControl->var.CurMFCModel = 0;
		break;
	}

	if( MFControl->conf.MfcPexe.MFCModel != 0 )
	{ /* Model following enabled */
/*--------------------------------------------------------------------------------------------------*/
/*		Speed FF compensation, Add to torque FF compensation value														*/
/*--------------------------------------------------------------------------------------------------*/
		if( (MFControl->var.PffCmd != 0) || (MFControl->conf.MfcPexe.MFCType == SGDX_TYPE) )
		{	/* FF valid only when there is a position command or MFCProcessing type(SGDXCompatibility)FF enabled */
			*SpdFFCx += MFControl->var.VffCmd;				/* MFC speed FF addition		*/
			*TrqFFCx += MFControl->var.TffCmd;				/* MFC torque FF addition		*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Accumulation pulse calculation	Σ(Position command - Position command after MFC)										*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
		MFControl->var.MfcPosErr = FlibPerrcalx( dPcmd, MFControl->var.PffCmd, MFControl->var.Per64 );
#else
		MFControl->var.MfcPosErr = MlibPerrcalx( dPcmd, MFControl->var.PffCmd, MFControl->var.Per64 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Position command output																				*/
/*--------------------------------------------------------------------------------------------------*/
		pcmdout = MFControl->var.PffCmd;
	}
	else
	{ /* Model following disabled */
		MlibResetLongMemory( &(MFControl->var), sizeof(MFControl->var)/4 );
		MFControl->var.PffCmd = 0;
		MFControl->var.VffCmd = 0;
		MFControl->var.TffCmd = 0;
/*--------------------------------------------------------------------------------------------------*/
/*		Position command output																				*/
/*--------------------------------------------------------------------------------------------------*/
		pcmdout = dPcmd;								/* When the function is disabled, the command input is output as is		*/
	}

	return pcmdout;
}



/****************************************************************************************************/
/*																									*/
/*		Rigid body MFC calculation																				*/
/*																									*/
/*	Overview:	Perform rigid body MFC calculation																	*/
/*			2nd order filter×Create a post-MFC position command through the 1st stage.										*/
/*			For FF first-order differential, use another secondary filter to increase resolution							*/
/*																									*/
/* arg		: LONG	dPcmd		:Position difference command	[pluse]												*/
/* out		: 																						*/
/* g-out	: 																						*/
/*																									*/
/****************************************************************************************************/
static	void	mfcControlOneMass( MFCTRL *MFControl, LONG dPcmd )
{
	LONG	wk;
	KSGAIN	wk_cmd;
	LONG	tffcmd;

	MFControl->var.PCmdsub = dPcmd;							/* Position command set						*/

	/*----------------------------------------------------------------------------------------------*/
	/*		High-order filter calculation	Rigid body: Quadratic filter												*/
	/*----------------------------------------------------------------------------------------------*/
	/* Rigid body model → 2nd order filter×1 */
	/* FCmd(k-1) */
	MFControl->var.FCmd[1] = MFControl->var.FCmd[0];
#if (FLOAT_USE==TRUE)
	/* Use position as 4th order filter */
	wk_cmd = FlibPcmdLpfil2( MFControl->var.PCmdsub, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk1 ); //todo
	/* FCmd(k) */
	MFControl->var.FCmd[0] = FlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk2 );
#else
	/* Use position as 4th order filter */
	wk_cmd = MlibPcmdLpfil2( MFControl->var.PCmdsub, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk1 );
	/* FCmd(k) */
	MFControl->var.FCmd[0] = MlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk2 );
#endif /* FLOAT_USE */


	/* First-order differential value calculation → Second-order filter for first-order differential value */
	/* Resolution of 1st differentialUP */
#if (FLOAT_USE==TRUE)
	wk_cmd = wk_cmd * MFControl->conf.MfcPexe.Fsft;
#else
	wk_cmd = MlibMulgain32( wk_cmd, MFControl->conf.MfcPexe.Fsft );
#endif /* FLOAT_USE */
	/* 1st differential value dFCmd(k) */
	MFControl->var.dFCmd[0] = mfcDtrCmdFilter( MFControl, wk_cmd );
#if (FLOAT_USE==TRUE)
	/* ResolutionUP FCmd(k) */
	MFControl->var.FCmdF = FlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk4 ); //todo
#else
	/* ResolutionUP FCmd(k) */
	MFControl->var.FCmdF = MlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk4 );
#endif /* FLOAT_USE */

	/* Position data acquisition */
	MFControl->var.FCmdFilo[1] = MFControl->var.FCmdFilo[0];
	MFControl->var.FCmdFilo[0] = MFControl->var.FCmdF;

	/*----------------------------------------------------------------------------------------------*/
	/*		Mechanical coefficient calculation																			*/
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	/* Return resolution */
	wk = MFControl->var.dFCmd[0] * MFControl->conf.MfcPexe.InvFsft;
	/* TrqRef(k) = dFCmd(k) * Kj1 */
	MFControl->var.TrqRef = wk * MFControl->conf.MfcPexe.Kj1;
#else
	/* Return resolution */
	wk = MlibMulgain30( MFControl->var.dFCmd[0], MFControl->conf.MfcPexe.InvFsft );
	/* TrqRef(k) = dFCmd(k) * Kj1 */
	MFControl->var.TrqRef = MlibMulgain( wk, MFControl->conf.MfcPexe.Kj1 );
#endif /* FLOAT_USE */
	/*----------------------------------------------------------------------------------------------*/
	/*		FF command calculation																			*/
	/*----------------------------------------------------------------------------------------------*/
	/* Pfbk(k) = FCmd(k-1) */
	MFControl->var.Pfbk = MFControl->var.FCmd[1];
	/*	Position FF command */
	MFControl->var.PffCmd = MFControl->var.Pfbk;
	/*----------------------------------------------------------------------------------------------*/
	if( MFControl->conf.MfcPexe.MFCType == SGDX_TYPE )
	{ /* MFCProcessing type(SGDXCompatibility) Resolution remains unchanged */
		/* Vref(k) = FCmd(k-1) */
		MFControl->var.Vref = MFControl->var.FCmd[1];
	}
	else
	{ /* ResolutionUPspeedFF */
		/* Vref(k) = FCmd(k-1) */
		MFControl->var.Vref = MFControl->var.FCmdFilo[1];
	}

	/*	Speed FF command */
#if (FLOAT_USE==TRUE)
	MFControl->var.VffCmd = MFControl->var.Vref * MFControl->conf.MfcPexe.Kvff;
#else
	MFControl->var.VffCmd = MlibMulgain( MFControl->var.Vref, MFControl->conf.MfcPexe.Kvff );
#endif /* FLOAT_USE */

	/* Command direction determination → TorqueFFGain switching */
	if( MFControl->var.PCmdsub > 0 )
	{
		MFControl->var.Ktffcmdx = MFControl->conf.MfcPexe.Ktff1;
	}
	else if( MFControl->var.PCmdsub < 0 )
	{
		MFControl->var.Ktffcmdx = MFControl->conf.MfcPexe.Ktff2;
	}

	/* Torque FF command */
	MFControl->var.FricCmpTrq = 0;

#if (FLOAT_USE==TRUE)
	tffcmd = (MFControl->var.TrqRef * MFControl->var.Ktffcmdx ) + MFControl->var.FricCmpTrq;
	MFControl->var.TffCmd = FlibLimitul( tffcmd, 0x01000000, 0xFF000000 );
#else
	tffcmd = MlibMulgain( MFControl->var.TrqRef, MFControl->var.Ktffcmdx ) + MFControl->var.FricCmpTrq;
	MFControl->var.TffCmd = MlibLimitul( tffcmd, 0x01000000, 0xFF000000 );
#endif /* FLOAT_USE */

}



/****************************************************************************************************/
/*																									*/
/*		Two-inertia, rigid body + machine MFC calculation																*/
/*																									*/
/*	Overview:	Performing two-inertia MFC calculation																	*/
/*			2nd order filter×Create a post-MFC position command through two stages.										*/
/*			For FF first-order differential, use another secondary filter to increase resolution							*/
/*																									*/
/* arg		: LONG	dPcmd		:Position difference command	[pluse]												*/
/* out		: 																						*/
/* g-out	: 																						*/
/*																									*/
/****************************************************************************************************/
static	void	mfcControlTwoMass( MFCTRL *MFControl, LONG dPcmd )
{
	LONG	wk;
	KSGAIN 	wk_cmd;
	LONG	posref, posref1, posref2;
	LONG	posref_sft;
	LONG	trqrf1;
	LONG	tffcmd;

/*--------------------------------------------------------------------------------------------------*/
/*		DTR 250us Cycle	Control																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( (MFControl->var.execnt & 0x0001) == 1 )
	{
/*--------------------------------------------------------------------------------------------------*/
/*	First half cycle processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		/* Saving current cycle position command */
		MFControl->var.PCmdsub = dPcmd;

	/*----------------------------------------------------------------------------------------------*/
	/*		Mechanical coefficient calculation																			*/
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
		/* Return resolution */
		wk = MFControl->var.dFCmd[1] * MFControl->conf.MfcPexe.InvFsft;
		/* Return resolution */
		trqrf1 = MFControl->var.d3FCmd * MFControl->conf.MfcPexe.Kj3;
		/* TrqRef(k) = ( dfCmd(k-1) + d3FCmd(k)*Kj3 ) * Kj1 */
		MFControl->var.TrqRef = ( wk + trqrf1) * MFControl->conf.MfcPexe.Kj1;
#else
		/* Return resolution */
		wk = MlibMulgain30( MFControl->var.dFCmd[1], MFControl->conf.MfcPexe.InvFsft);

		/* Return resolution */
		trqrf1 = MlibMulgain30( MFControl->var.d3FCmd, MFControl->conf.MfcPexe.Kj3 );
		/* TrqRef(k) = ( dfCmd(k-1) + d3FCmd(k)*Kj3 ) * Kj1 */
		MFControl->var.TrqRef = MlibMulgain( wk + trqrf1, MFControl->conf.MfcPexe.Kj1 );
#endif /* FLOAT_USE */

		/* Return resolution */
		posref = MlibPfbkxremNolim( MFControl->var.d2FCmd[1], MFControl->conf.MfcPexe.Kj2, &MFControl->var.wkrem1 );
		posref_sft = MlibPfbkxremNolim( MFControl->var.d2FCmd[1], MFControl->conf.MfcPexe.Kj2_sft, &MFControl->var.wkrem2 );

		if( MFControl->conf.MfcPexe.Fccflag )
		{ /* Full-Closed && 2-mass */
			posref1 = 0;
		}
		else
		{
			posref1 = posref;
		}

		if(MFControl->conf.MfcPexe.MFCType == SGDX_TYPE)
		{ /* MFCProcessing type(SGDXCompatibility) Resolution remains unchanged */
			if( MFControl->conf.MfcPexe.PSFccflag )
			{/* PS-Full-Closed && 2-mass */
				posref2 = 0;
			}
			else
			{
				posref2 = posref;
			}
		}
		else
		{ /* ResolutionUPcalculated */
			if( MFControl->conf.MfcPexe.PSFccflag )
			{ /* PS-Full-Closed && 2-mass */
				posref2 = 0;
			}
			else
			{
				posref2 = posref_sft;
			}
		}

	/*----------------------------------------------------------------------------------------------*/
	/*		Forced discharging process																		*/
	/*----------------------------------------------------------------------------------------------*/
		if( (posref1 == 0)
			&& (MFControl->var.FCmd[0] == MFControl->var.FCmd[1])
			&& (MFControl->var.PosSum[0] != 0) )
		{ /* Current position command addition value =0 && 1Level differential value =0 && Accumulated value≠0 */
			if( MFControl->var.PosSum[0] > 0 )
			{
				posref1 = -1;
			}
			else
			{
				posref1 = 1;
			}
		}
		/* Position command addition value integration */
		wk = MlibPerrcalx( posref1, 0, MFControl->var.PosSum );

	/*----------------------------------------------------------------------------------------------*/
	/*		FF command calculation																			*/
	/*----------------------------------------------------------------------------------------------*/
		/* Position FF command Pfbk(k-1) */
		MFControl->var.PffCmd = MFControl->var.Pfbk - MFControl->var.PffCmd;
		/* Pfbk(k) */
		MFControl->var.Pfbk = posref1 + MFControl->var.FCmd[2];
	/*----------------------------------------------------------------------------------------------*/
		if(MFControl->conf.MfcPexe.MFCType == SGDX_TYPE)
		{ /* MFCProcessing type(SGDXCompatibility) Resolution remains unchanged */
			/* Vref(k) = d2FCmd[1]*Kj2 + FCmd(k-2) */
			MFControl->var.Vref = posref2 + MFControl->var.FCmd[2];
		}
		else
		{ /* ResolutionUPspeedFF */
			/* Vref(k) = d2FCmd[1]*Kj2 + FCmd(k-2) */
			MFControl->var.Vref = posref2 + MFControl->var.FCmdFilo[2];
		}
		/* Speed FF command Vref(k-1)*Kvff *//* VFFby half a sample */
#if (FLOAT_USE==TRUE)
		MFControl->var.VffCmd = MFControl->var.Vref * MFControl->conf.MfcPexe.Kvff;
#else
		MFControl->var.VffCmd = MlibMulgain( MFControl->var.Vref, MFControl->conf.MfcPexe.Kvff );
#endif /* FLOAT_USE */

		/* Command direction determination → TorqueFFGain switching */
		if( MFControl->var.PCmdsub > 0 )
		{
			MFControl->var.Ktffcmdx = MFControl->conf.MfcPexe.Ktff1;
		}
		else if( MFControl->var.PCmdsub < 0 )
		{
			MFControl->var.Ktffcmdx = MFControl->conf.MfcPexe.Ktff2;
		}

		/* Torque FF command */
#if (FLOAT_USE==TRUE)
		MFControl->var.FricCmpTrq = 0;
		tffcmd = (MFControl->var.TrqRef * MFControl->var.Ktffcmdx) + MFControl->var.FricCmpTrq;
		MFControl->var.TffCmd = FlibLimitul( tffcmd, 0x01000000, 0xFF000000 );
#else
		MFControl->var.FricCmpTrq = 0;
		tffcmd = MlibMulgain(MFControl->var.TrqRef, MFControl->var.Ktffcmdx) + MFControl->var.FricCmpTrq;
		MFControl->var.TffCmd = MlibLimitul( tffcmd, 0x01000000, 0xFF000000 );
#endif /* FLOAT_USE */
		}
	else
	{
/*--------------------------------------------------------------------------------------------------*/
/*	Late cycle processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		/* Add position commands of previous and current cycle */
		MFControl->var.PCmdsub = MFControl->var.PCmdsub + dPcmd;

	/*----------------------------------------------------------------------------------------------*/
	/*	Position command calculation																				*/
	/*----------------------------------------------------------------------------------------------*/
		/* Calculation cycle conversion */
		MFControl->var.PffCmd = (MFControl->var.Pfbk >> 1);
	/*----------------------------------------------------------------------------------------------*/
	/*	Speed FF command calculation																			*/
	/*----------------------------------------------------------------------------------------------*/
		/*	Speed FF command */
#if (FLOAT_USE==TRUE)
		MFControl->var.VffCmd = MFControl->var.Vref * MFControl->conf.MfcPexe.Kvff;
#else
		MFControl->var.VffCmd = MlibMulgain( MFControl->var.Vref, MFControl->conf.MfcPexe.Kvff );
#endif /* FLOAT_USE */

	/*----------------------------------------------------------------------------------------------*/
	/*		High-order filter calculation	2 inertia: Quadratic filter×2											*/
	/*----------------------------------------------------------------------------------------------*/
	/* 2 inertia model → 2nd order filter×2 */
		/* FCmd(k-2) */
		MFControl->var.FCmd[2] = MFControl->var.FCmd[1];
		/* FCmd(k-1) */
		MFControl->var.FCmd[1] = MFControl->var.FCmd[0];
		/* FCmd(k) */
#if (FLOAT_USE==TRUE)
		wk_cmd = FlibPcmdLpfil2( MFControl->var.PCmdsub, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk1 );
		MFControl->var.FCmd[0] = FlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk2 );
#else
		wk_cmd = MlibPcmdLpfil2( MFControl->var.PCmdsub, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk1 );
		MFControl->var.FCmd[0] = MlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk2 );
#endif /* FLOAT_USE */

	/* First-order differential value calculation → 2nd order filter×Secondary filter for 1+1st-order differential */
		/* 1st differential value dFCmd(k-1) */
		MFControl->var.dFCmd[1] = MFControl->var.dFCmd[0];
#if (FLOAT_USE==TRUE)
		/* Resolution of 1st differentialUP */
		wk_cmd = MFControl->var.PCmdsub * MFControl->conf.MfcPexe.Fsft;
#else
		/* Resolution of 1st differentialUP */
		wk_cmd = MlibMulgain32(MFControl->var.PCmdsub, MFControl->conf.MfcPexe.Fsft);
#endif /* FLOAT_USE */
		
		/* 1st differential value dFCmd(k) */
#if (FLOAT_USE==TRUE)
		wk_cmd = FlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk3 );
		MFControl->var.dFCmd[0] = mfcDtrCmdFilter( MFControl, wk_cmd );
		/* ResolutionUP FCmd(k) */
		MFControl->var.FCmdF = FlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk4 );
#else
		wk_cmd = MlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk3 );
		MFControl->var.dFCmd[0] = mfcDtrCmdFilter( MFControl, wk_cmd );
		/* ResolutionUP FCmd(k) */
		MFControl->var.FCmdF = MlibPcmdLpfil2( wk_cmd, MFControl->conf.MfcPexe.Kf1, MFControl->var.Fwk4 );
#endif /* FLOAT_USE */

		/* ResolutionUPPosition data acquisition */
		MFControl->var.FCmdFilo[2] = MFControl->var.FCmdFilo[1];
		MFControl->var.FCmdFilo[1] = MFControl->var.FCmdFilo[0];
		MFControl->var.FCmdFilo[0] = MFControl->var.FCmdF;

	/*----------------------------------------------------------------------------------------------*/
	/*		Differential calculation																				*/
	/*----------------------------------------------------------------------------------------------*/
		/* Second derivative value d2FCmd(k-1) */
		MFControl->var.d2FCmd[1] = MFControl->var.d2FCmd[0];
		/* Second derivative value d2FCmd(k) */
		MFControl->var.d2FCmd[0] = MFControl->var.dFCmd[0] - MFControl->var.dFCmd[1];
		/* Third-order differential value d3FCmd(k) */
		MFControl->var.d3FCmd = MFControl->var.d2FCmd[0] - MFControl->var.d2FCmd[1];
	}
}



/****************************************************************************************************/
/*																									*/
/*		Second-order filter calculation for first-order differential value calculation															*/
/*																									*/
/****************************************************************************************************/
static	LONG	mfcDtrCmdFilter( MFCTRL *MFControl, LONG pcmd )
{
#if (FLOAT_USE==TRUE)
	float	wk;
	float	dFilo;

	wk = ( pcmd - MFControl->var.Filo ) - MFControl->var.dFilwk;
	wk = wk * MFControl->conf.MfcPexe.Kf2;
	MFControl->var.dFilwk += wk;

	dFilo = MFControl->var.dFilwk * MFControl->conf.MfcPexe.Kf3;
	
	wk = MFControl->var.dFilwk * MFControl->conf.MfcPexe.Kf1[1];
	MFControl->var.Filo += wk;

#else
	LONG	wk;
	LONG	dFilo;

	wk = ( pcmd - MFControl->var.Filo ) - MFControl->var.dFilwk;
	wk = MlibPfbkxremNolim( wk, MFControl->conf.MfcPexe.Kf2, &MFControl->var.Fwkrem1 );
	MFControl->var.dFilwk += wk;

	dFilo = MlibMulgain32( MFControl->var.dFilwk, MFControl->conf.MfcPexe.Kf3 );

	wk = MlibPfbkxremNolim( MFControl->var.dFilwk, MFControl->conf.MfcPexe.Kf1[1], &MFControl->var.Fwkrem2 );
	MFControl->var.Filo += wk;
#endif /* FLOAT_USE */

	return( dFilo );
}



/****************************************************************************************************/
/*																									*/
/*		MFCGet parameter switching completion statusAPI													*/
/*																									*/
/*			Used to confirm switching in one-parameter, etc. Compare model type, model gain, and model frequency*/
/*			Since the adjustment function does not handle model FF, set the model type, model gain, and model frequency		*/
/*			Just monitor. Please note that it is not possible to discriminate based on other parameters such as model FF gain.		*/
/*																									*/
/****************************************************************************************************/
SHORT	MfcGetParamEnableSts( MFCTRL *MFControl )
{
	SHORT	rc;

	if( (MFControl->conf.MfcPexe.MFCModel == MFControl->conf.MfcPrm[0].MFCModel)
		&& (MFControl->conf.MfcPexe.Kj2 == MFControl->conf.MfcPrm[0].Kj2)
		&& (MFControl->conf.MfcPexe.Kf2 == MFControl->conf.MfcPrm[0].Kf2) )
	{ /* Execution model/Execution model frequency/If the running model gain is the same as the parameter */
		rc = TRUE;
	}
	else
	{
		rc = FALSE;
	}
	return rc;
}



/****************************************************************************************************/
/*																									*/
/*		Model following control stopAPI																		*/
/*			※ActuallyMFCwill stop only after the motor stops									*/
/*																									*/
/****************************************************************************************************/
void	MfcSetMfcCtrlSwitch( MFCTRL *MFControl, BOOL Switch )
{
	if( Switch == TRUE )
	{ /* Model following control enablement request */
		MFControl->var.MfcStopReq = FALSE;
	}
	else
	{ /* Model following control disablement request */
		MFControl->var.MfcStopReq = TRUE;
	}
}





/****************************************************************************************************/
/*																									*/
/*		MFC type selection																			*/
/*																									*/
/****************************************************************************************************/
/*		Return value	:	Error Status (FALSE:No error, TRUE:Parameter setting error)						*/
/****************************************************************************************************/
BOOL	MfcCalculateInitPrm( MFCTRL *MFControl, MFC_CFG_PRM *MfcCfgPrm,
    	                     USHORT jrate, BPRMDAT *Bprm, USHORT mfctype )
{
	BOOL	PrmErr;

	PrmErr = FALSE;

	switch(mfctype & 0x0F)
	{
	case 0x00: /* MFCProcessing type(SGDXCompatibility) */
		MFControl->conf.MFCTypeSetting = SGDX_TYPE;
		break;
	case 0x01: /* MFCProcessing type(SGDV) */
		MFControl->conf.MFCTypeSetting = SGDV_TYPE;
		break;
	default:
		PrmErr = TRUE;
		break;
	}

	/* First bank	*/
	MfcCalculatePrm( MFControl, MfcCfgPrm, jrate, Bprm, 0, 0 );
	/* Second bank	*/
	MfcCalculatePrm( MFControl, MfcCfgPrm, jrate, Bprm, 0, 1 );

	return	PrmErr;
}



/****************************************************************************************************/
/*																									*/
/*		MFC selection																					*/
/*																									*/
/****************************************************************************************************/
/*		Return value	:	Error Status (FALSE:No error, TRUE:Parameter setting error)						*/
/****************************************************************************************************/
BOOL	MfcCalculatePrmSW( MFCTRL *MFControl, MFC_CFG_PRM *MfcCfgPrm, USHORT jrate, BPRMDAT *Bprm )
{
	BOOL	PrmErr;
	LONG	wk;

	PrmErr = FALSE;
	wk = MfcCfgPrm->mdlsw & 0xFF;

	switch( wk )
	{
	case 0x00: /* Function disabled */
	case 0x10: /* Function disabled */
	case 0x20: /* Function disabled */
		MFControl->conf.MFCModelSetting = 0;
		break;
	case 0x01: /* Model following control */
		/* Rigid body model following control */
		MFControl->conf.MFCModelSetting = ONE_MASS;
		break;
	case 0x11: /* Model following control with vibration suppression */
	case 0x21: /* Model following control with vibration suppression 2 */
		/* Rigid body + machine model following control */
		MFControl->conf.MFCModelSetting = ONE_BASE;
		break;
	default:
		PrmErr = TRUE;
		break;
	}

	wk = (MfcCfgPrm->mdlsw >> 12) & 0x0F;
	switch(wk)
	{
	case 0x00:
		MFControl->conf.MFCFF = FALSE;
		break;
	case 0x01:
		MFControl->conf.MFCFF = TRUE;
		break;
	default:
		PrmErr = TRUE;
		break;
	}

	/* First bank	*/
	MfcCalculatePrm( MFControl, MfcCfgPrm, jrate, Bprm, 0, 0 );
	/* Second bank	*/
	MfcCalculatePrm( MFControl, MfcCfgPrm, jrate, Bprm, 0, 1 );

	return	PrmErr;
}


/****************************************************************************************************/
/*																									*/
/*		MFC parameter calculation																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			  LONG		cfric			:	Coulomb friction torque parameter							*/
/*			  LONG		GeslNo			:	Parameter bank										*/
/*												0: 1st1Bank,1:No.2Bank							*/
/*																									*/
/****************************************************************************************************/
void	MfcCalculatePrm( MFCTRL *MFControl, MFC_CFG_PRM	*MfcCfgPrm,
						 USHORT jrate, BPRMDAT *Bprm, LONG cfric, LONG GselNo )
{
	MFCPRM		wrkp;
	USHORT		mdlgn;
	USHORT		mdlzeta;
	KSGAIN		hz;				/*<S00A>*/
	LONG		kx, sx;
	LONG		ky, sy;
	LONG		wk;
	KSGAIN		scantime;		/*<S00A>*/
	KSGAIN		maxpls;			/*<S00A>*/
#if (FLOAT_USE==TRUE)
	float fw;
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		GainNo.Selection																				*/
/*--------------------------------------------------------------------------------------------------*/
	if( GselNo == 0 )
	{ /* When setting the first gain */
		mdlgn = MfcCfgPrm->mdlgn;
		mdlzeta = MfcCfgPrm->mdlzeta;
	}
	else
	{ /* When setting the second gain */
		mdlgn = MfcCfgPrm->mdlgn2;
		mdlzeta = MfcCfgPrm->mdlzeta2;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Machine model																					*/
/*--------------------------------------------------------------------------------------------------*/
	wrkp.MFCModel = MFControl->conf.MFCModelSetting;

/*--------------------------------------------------------------------------------------------------*/
/*		MFCType		<V509>																		*/
/*--------------------------------------------------------------------------------------------------*/
	wrkp.MFCType = MFControl->conf.MFCTypeSetting;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation cycle time	[ns]																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( wrkp.MFCModel == ONE_MASS )							/* Rigid body model							*/
	{
		scantime = (LONG)Bprm->SvCycleNs;
//		scantime = (LONG)MFCONEMASSCYCLE;
	}
	else													/* 2 inertia model							*/
	{
		scantime = (LONG)MFCTWOMASSCYCLE(Bprm->SvCycleNs);
//		scantime = (LONG)MFCTWOMASSCYCLE;
	}

#if 0
	if( (Kprm.f.FencUse == TRUE)							/* Full-Closed Control					*/
		&& (wrkp.MFCModel != ONE_BASE) )					/* Rigid body + machine base→Two-inertia model		*/
	{
		wrkp.Fccflag = 1;
	}
	else
#endif
	{
		wrkp.Fccflag = 0;
	}

#if 0 /* PSFlag for Fulk(Not supported) */
	wrkp.PSFccflag = (Iprm.f.PsFulcSelect) ? 1 : 0 ;		/* PS-Full-Closed Control				*/
#else
	wrkp.PSFccflag = 0;
#endif

/* 2011.04.04 Y.Oka <V731> */
/*--------------------------------------------------------------------------------------------------*/
/*		MotorOSSpeed pulse calculation	[Pluse/Scan]				Bprm.MaxPspd:[Pulse/sec]				*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	maxpls = (scantime * Bprm->MaxPspd / (float)C10POW9);
#else
	maxpls = MlibScalKxkskx( scantime, Bprm->MaxPspd, C10POW9, NULL, -30 );
#endif /* FLOAT_USE */

#if 0
/*--------------------------------------------------------------------------------------------------*/
/*		DTR high-order filter resolution shift amount Fsft														*/
/*--------------------------------------------------------------------------------------------------*/
/*						   2^29																		*/
/*				Fsft = ------------		InvFsft = 1/Fsft											*/
/*				   		  FbPulse																	*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	wrkp.Fsft = 0x20000000 / Bprm->FbPulse;
	if( wrkp.Fsft < 1.0 )
	{
		wrkp.Fsft = 1.0;
	}
	wrkp.InvFsft = 1.0F / wrkp.Fsft;
#else
	wrkp.Fsft = 0x20000000 / Bprm->FbPulse;
	if( wrkp.Fsft < 1 )
	{
		wrkp.Fsft = 1;
	}
	wrkp.InvFsft = MlibScalKxgain( 1, 1, wrkp.Fsft, NULL, 24 );
#endif /* FLOAT_USE */

#endif
/* 2011.04.04 Y.Oka <V731> */

/*--------------------------------------------------------------------------------------------------*/
/*		Secondary filter gain 1 Kf1																	*/
/*--------------------------------------------------------------------------------------------------*/
/*						   Ts[us] * 0x1000000		    Ts[us] * 0x1000000							*/
/*				Kf1[0] = ---------------------- = --------------------------------					*/
/*				    	    Tf[us] + Ts[us]	        (10^6 / 2ζωn)[us] + Ts[us]					*/
/*																									*/
/*						     ωn * Ts[us]															*/
/*				Kf1[1] = ----------------------														*/
/*						   2 * ζ[1.0] * 10^6														*/
/*--------------------------------------------------------------------------------------------------*/
/*				Ts[us]  = SvCycleUs[us]																*/
/*																									*/
/*							mdlzeta[0.0001]															*/
/*				ζ[1.0] = -------------------														*/
/*							    10000																*/
/*		(Rigid body)																						*/
/*											    2*mdlzeta[0.001]*mdlgn[0.1/s]						*/
/*				ωn = 2 * ζ[1.0] * Km[1/s] = ----------------------------------					*/
/*														1000 * 10									*/
/*		(Two-inertia, Rigid body + machine base)																		*/
/*		 								        4*mdlzeta[0.001]*mdlgn[0.1/s]						*/
/*				ωn = 4 * ζ[1.0] * Km[1/s] = ----------------------------------					*/
/*														1000 * 10									*/
/*																									*/
/*					  Km : Model loop gain(Command filter gain)									*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Filter frequency hz[0.1Hz] = ωn*10 / 2*PAI 										*/
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)

	if( wrkp.MFCModel == ONE_MASS )							/* Rigid body model							*/
	{
		hz = 4.0f * (float)mdlzeta * (float)mdlgn * 10.0f / 62832.0f;
	}
	else													/* Two-inertia, rigid body + machine base model				*/
	{
		hz = 4.0f * (float)mdlzeta * (float)mdlgn * 10.0f / 62832.0f;
	}

#else	
	if( wrkp.MFCModel == ONE_MASS )							/* Rigid body model							*/
	{
		hz = MlibScalKxgain( 4*mdlzeta, mdlgn*10, 62832, NULL, -24 );
	}
	else													/* Two-inertia, rigid body + machine base model				*/
	{
		hz = MlibScalKxgain( 4*mdlzeta, mdlgn*10, 62832, NULL, -24 );
	}
#endif /* FLOAT_USE */

	/*----------------------------------------------------------------------------------------------*/
	/*		2nd order filter gain Kf1[0], Kf1[1] Calculation													*/
	/*----------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	FlibPcalKf2gain( hz, mdlzeta, scantime, wrkp.Kf1);
#else
	MlibPcalKf2gain( hz, mdlzeta, scantime, wrkp.Kf1, 0x10 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Secondary filter gain 2 Kf2																	*/
/*--------------------------------------------------------------------------------------------------*/
/*						    Kf1[0]																	*/
/*				Kf2 = -----------------																*/
/*				    	  0x1000000																	*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	wrkp.Kf2 = (wrkp.Kf1[0] / (float)0x1000000);
#else
	wrkp.Kf2 = MlibScalKxgain( wrkp.Kf1[0], 1, 0x1000000, NULL, 24 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Secondary filter gain 3 Kf3																	*/
/*--------------------------------------------------------------------------------------------------*/
/*						Kf1[1] * 1000000000															*/
/*				Kf3 = --------------------															*/
/*				    	 SvCycleUs[ns]																*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	wrkp.Kf3 = wrkp.Kf1[1] * (float)C10POW9 / scantime;
#else
	wrkp.Kf3 = MlibScalKskxkx( wrkp.Kf1[1], C10POW9, scantime, NULL, 24 );
#endif /* FLOAT_USE */

/* 2011.04.04 Y.Oka <V731> */
/*--------------------------------------------------------------------------------------------------*/
/*		DTRHigh-order filter resolution shift amount Fsft															*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*				Fsft = 2^30/((maxpls/kf[1]))			maxpls: Maximum pulse/Control cycle					*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	kx = wrkp.Kf1[1] * 0x3FFFFFFF / maxpls;

	wk = 0x20000000 / Bprm->FbPulse;
	wrkp.Fsft = FlibLimitul( kx, wk, 1.0f );								/* 1 < kx < (2^29/plsno)	*/

	wrkp.InvFsft = (1.0F / wrkp.Fsft);			/* invFsft = 1/ Fsft		*/
/* 2011.04.04 Y.Oka <V731> */
#else
	kx = MlibScalKskxkx( wrkp.Kf1[1], 0x3FFFFFFF, maxpls, NULL, -24 );

	wk = 0x20000000 / Bprm->FbPulse;
	wrkp.Fsft = MlibLimitul( kx, wk, 1 );								/* 1 < kx < (2^29/plsno)	*/

	wrkp.InvFsft = MlibScalKxgain( 1, 1, wrkp.Fsft, NULL, 24 );			/* invFsft = 1/ Fsft		*/
/* 2011.04.04 Y.Oka <V731> */
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Mechanical coefficient gain 1 Kj1																		*/
/*--------------------------------------------------------------------------------------------------*/
/*		             			  1000000000       100 + jrate   									*/
/*				Kj1 =  Kpvx * ---------------- * ---------------									*/
/*		            			SvCycleUs[ns]         100        									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	wrkp.Kj1 = Bprm->Kpvx * C10POW9 / scantime * ((jrate + 100)/100.0f);
#else
	kx = MlibScalKskxkx( Bprm->Kpvx, C10POW9, scantime, &sx, 0 );
	wrkp.Kj1 = MlibPcalKxgain( kx, (jrate + 100), 100, &sx, 24 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Mechanical coefficient gain 2 Kj2																		*/
/*--------------------------------------------------------------------------------------------------*/
/*		             		           100 * 1000000000    											*/
/*				Kj2 = -----------------------------------------------------							*/
/*		            	antfrq[0.1Hz]^2 * (2*PAI)^2 * SvCycleUs[ns] * Fsft							*/
/*																									*/
/*		             	 	           100 * 1000000000    											*/
/*				Kj2_sft = ----------------------------------------------							*/
/*		               	   antfrq[0.1Hz]^2 * (2*PAI)^2 * SvCycleUs[ns]								*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	//wrkp.Kj2 = 1.0 / ((MfcCfgPrm->mdlantfrq*0.1)*(MfcCfgPrm->mdlantfrq*0.1) * (2*PAI)*(2*PAI) * scantime * wrkp.Fsft);
	wrkp.Kj2 = 100.0f * E9C2PAI2 / (MfcCfgPrm->mdlantfrq * MfcCfgPrm->mdlantfrq * scantime / wrkp.Fsft);

	//wrkp.Kj2_sft = 1.0 / ((MfcCfgPrm->mdlantfrq*0.1)*(MfcCfgPrm->mdlantfrq*0.1) * (2*PAI)*(2*PAI) * scantime * wrkp.Fsft);
	wrkp.Kj2_sft = 100.0f * E9C2PAI2 / (MfcCfgPrm->mdlantfrq * MfcCfgPrm->mdlantfrq * scantime);
#else
	sx = 0;
	sy = 0;
	wk = MfcCfgPrm->mdlantfrq * MfcCfgPrm->mdlantfrq;
	kx = MlibPcalKxmulx( 100, E9C2PAI2, 1, &sx );
	ky = MlibPcalKxmulx( wk, scantime, wrkp.Fsft, &sy );
	wrkp.Kj2 = MlibPcalKxdivx( kx, sx, ky, sy, 24 );

	sx = 0;
	sy = 0;
	wk = MfcCfgPrm->mdlantfrq * MfcCfgPrm->mdlantfrq;
	kx = MlibPcalKxmulx( 100, E9C2PAI2, 1, &sx );
	ky = MlibPcalKxmulx( wk, scantime, 1, &sy );
	wrkp.Kj2_sft = MlibPcalKxdivx( kx, sx, ky, sy, 24 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Mechanical coefficient gain 3 Kj3																		*/
/*--------------------------------------------------------------------------------------------------*/
/*		             		           100 * 1000000000^2  											*/
/*				Kj3 = --------------------------------------------------------						*/
/*		            	resfrq[0.1Hz]^2 * (2*PAI)^2 * SvCycleUs[ns]^2 * Fsft						*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	//wrkp.Kj3 = 1.0 / ((MfcCfgPrm->mdlresfrq*0.1)*(MfcCfgPrm->mdlresfrq*0.1) * (2*PAI)*(2*PAI) * scantime*scantime * wrkp.Fsft);
	wrkp.Kj3 = (100 * E9C2PAI2 * 1000000000 / wrkp.Fsft) / (MfcCfgPrm->mdlresfrq * MfcCfgPrm->mdlresfrq * scantime * scantime);
#else
	sx = 0;
	sy = 0;
	kx = MlibPcalKxmulx( 100, E9C2PAI2, 1000000000, &sx);
	kx = MlibPcalKxgain( kx, 1, wrkp.Fsft, &sx , 0 );

	wk = MfcCfgPrm->mdlresfrq * MfcCfgPrm->mdlresfrq;
	ky = MlibPcalKxmulx( wk, scantime, scantime, &sy);
	wrkp.Kj3 = MlibPcalKxdivx(  kx, sx, ky, sy, 24 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		DTR speed feed forward gain Kvff														*/
/*--------------------------------------------------------------------------------------------------*/
/*		             		   mfckvff[0.1%] * 1000000000  											*/
/*				Kvff = Kpx * ---------------------------											*/
/*		            			 1000 * SvCycleUs[ns]												*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	if(wrkp.MFCType == SGDX_TYPE)
	{ /* MFCProcessing type(SGDXCompatibility) */
		wrkp.Kvff = Bprm->Kpx * 1000000000.0f / scantime * MfcCfgPrm->mdlvff / 1000.0f ;
	}
	else
	{
		wrkp.Kvff = Bprm->Kpx * 1000000000.0f / scantime / wrkp.Fsft * MfcCfgPrm->mdlvff / 1000.0f;
	}
#else
	kx = MlibScalKskxkx( Bprm->Kpx, 1000000000, scantime, &sx, 0 );
	if(wrkp.MFCType == SGDX_TYPE)
	{ /* MFCProcessing type(SGDXCompatibility) */
		wrkp.Kvff = MlibPcalKxgain( kx, MfcCfgPrm->mdlvff, 1000, &sx, 24 );
	}
	else
	{
		kx = MlibPcalKxgain( kx, 1, wrkp.Fsft, &sx, 0 );
		wrkp.Kvff = MlibPcalKxgain( kx, MfcCfgPrm->mdlvff, 1000, &sx, 24 );
	}
#endif /* FLOAT_USE */
/*--------------------------------------------------------------------------------------------------*/
/*		DTR torque feed forward gain Ktff1, Ktff2											*/
/*--------------------------------------------------------------------------------------------------*/
/*		                   mfcktff[0.1%]    			  mfctffgn2[0.1%]							*/
/*				Ktff1 = ------------------		Ktff2 = -------------------							*/
/*		       (For forward rotation)	       1000 		   (For reverse direction)		   1000									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	wrkp.Ktff1 = MfcCfgPrm->mdlptff / 1000.0f;
	wrkp.Ktff2 = MfcCfgPrm->mdlntff / 1000.0f;
#else
	wrkp.Ktff1 = MlibScalKxgain( MfcCfgPrm->mdlptff, 1, 1000, NULL, 24 );
	wrkp.Ktff2 = MlibScalKxgain( MfcCfgPrm->mdlntff, 1, 1000, NULL, 24 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Coulomb friction compensation torque Cfric 																*/
/*--------------------------------------------------------------------------------------------------*/
/*				Torque level calculation KpiTorqueLevelCal() : Torque[0.01%] --> NorTrq[2^24/MaxTrq]		*/
/*														cfric[0.1%]									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	wrkp.Cfric = cfric*10 * Bprm->Ktrqlvl;
#else
	wrkp.Cfric = MlibMulgain( cfric*10, Bprm->Ktrqlvl );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Coulomb friction compensation filter gain																*/
/*--------------------------------------------------------------------------------------------------*/
/*							    SvCycleUs[us]														*/
/*				Kfcf = -----------------------------------											*/
/*						     5000000																*/
/*						 ---------------- + SvCycleUs[us]											*/
/*						   mdlgn[0.1/s]																*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = 5000000.0f / mdlgn;
	if( fw > 100000.0f )
	{
		fw = 100000.0f;
	}
	wrkp.Kfcf = FlibPcalKf1gain( fw, (scantime/1000.0f), 0 );
#else
	kx = (LONG)( 5000000 / (SHORT)mdlgn );
	if( kx > 100000 )
	{
		ky = 100000;
	}
	else
	{
		ky = kx;
	}
	wrkp.Kfcf = MlibPcalKf1gain( ky, (scantime/1000), 0 );
#endif /* FLOAT_USE */

/* 2009.12.24 Y.Oka Is it necessary to disable interrupts? */
/*--------------------------------------------------------------------------------------------------*/
/*		Set Parameters																				*/
/*--------------------------------------------------------------------------------------------------*/
//	KPI_DI();		/* Disable Interrupt	*/
	MFControl->conf.MfcPrm[GselNo+1] = wrkp;
//	KPI_EI();		/* Enable Interrupt		*/

}



/***************************************** end of file **********************************************/

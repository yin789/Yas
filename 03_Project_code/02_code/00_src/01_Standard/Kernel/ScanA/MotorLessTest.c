/****************************************************************************
Description	: Motor Less Test Mode
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2013 All Rights Reserved

Project       : Mercury

Functions	  :
	-- APIs --
	 - Motorless test -
	ApxMotorLessTestModeMechaModel()	: Motorless test mode mechanical model calculation
	PcalMotorLessTestMode()				: Motorless test mode parameter calculation
	ApxInitMotorLessTestMode()			: Motorless test mode variable initialization
	ApiSetCurDatasMotorLessTestMode()	: Motorless test mode current data setting
	ApiSetMechaDatasMotorLessTestMode()	: Motorless test mode speed/position data setting
	ApiReadMotPrmMotorLessTestMode()	: Motor parameter calculation for motorless test mode: When encoder is not connected
	ApiReadFencPrmMotorLessTestMode()	: Motor parameter calculation for motorless test mode: When Fulck encoder is not connected

	 - Motorless test mode -
	MltSetMotorLessTestMode()			: Motorless test mode flag setting

	- Motorless test modeIFGet definition address(Currently unused) -
	MltGetMotorLessTestMode()			: Motorless test mode status acquisition
	MltGetMotorLessTestHandle()			: Motorless test modeIFGet definition address

----------------------------------------------------------------------------
	 - Semi-closed encoder related - (SencScanService.c)
	LpxSencMotLessInitVariables()		: Encoder variable initialization processing
	RmxReadMotorLessPosition()			: Rotary type position data reading
	LmxReadMotorLessPosition()			: Linear position data reading
	LpxSetSencInfoMotorLessTestMode()	: Encoder parameter setting processing
	LpxGetSencMotLessMturnData()		: Multiturn data acquisition processing
	LpxSencMotLessMultiTurnSet()		: Multiturn data setting processing

	 - Fully closed encoder related - (SencScanService.c)
	LpxFencMotLessInitVariables()		: Encoder variable initialization processing
	LpxSetFencInfoMotorLessTestMode()	: Encoder parameter setting processing
	LpxReadMotorLessFencPosition()		: Position data reading

----------------------------------------------------------------------------
Changes		:
Name		Date			Description
------------------------------------------------------------------------
K.Sakamoto	2013.11.11		created	<S00X>

****************************************************************************/
#include "Basedef.h"
#include "config.h"
#include "MercuryGlobals.h"
#include "BaseControlPrmCalc.h"
#include "MotorLessTest.h"
#include "KnlApi.h"
#include "Mlib.h"

/*==========================================================================*/
/* defines																	*/
/*==========================================================================*/
#define	MDLCYCLE		KPI_SACYCLENS			/* Mechanical model calculation cycle time(Same scan as speed loop)	[ns]*/
#define	C10POW9			1000000000				/* 10^9												*/
#define	C10POW7			10000000				/* 10^7												*/
#define	C2PAIE7			62831853				/* (2*PAI) * 10^7									*/


/*==========================================================================*/
/* PROTOTYPE																*/
/*==========================================================================*/


/*==========================================================================*/
/* PROGRAM																	*/
/*==========================================================================*/

/*****************************************************************************
  Description : Motorless test mode mechanical model calculation
	Calculates the speed and position response of the rigid body mechanical model to the input torque command.

	MotorLessMdl.V.MotMdlSpdFbk		: Speed FB				[2^24/OvrSpd]
	MotorLessMdl.V.MotMdldPosFbk	: Motor position FB difference	[pluse/scan]
	MotorLessMdl.V.MotMdlPosFbk		: Motor position FB		[pluse]
	MotorLessMdl.V.MechaMdldPosFbk	: Position FB difference			[pluse/scan]
	MotorLessMdl.V.MechaMdlPosFbk	: Position FB				[pluse]
----------------------------------------------------------------------------
  Parameters:
	TrqRef;					Torque command	[2^24/MaxTrq]
	BeComplete;				Base enable completion status
	MotorLessTest;			Motorless test modeIFPointer to definition
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void ApxMotorLessTestModeMechaModel( LONG TrqRef, BOOL BeComplete, MOTLESSTST *MotorLessTest )
{
	MOTLESSMDL	*MotorLessMdl;		/* Pointer to data for internal mechanical model	*/
	MOTLESSMDLP	*MotorLessMdl_P;	/* Pointer to valid parameter			*/

	LONG		MotSpd;				/* Motor speed							*/
	LONG		dMotPos;			/* Motor position FB difference					*/
	LONG		dFbPos;				/* Position FB difference							*/
	LONG		MotAcc;				/* Acceleration FB							*/

	/* Initializing variables	*/
	MotorLessMdl	= &(MotorLessTest->MotorLessMdl);
	MotorLessMdl_P	= MotorLessMdl->PrmPtr;

	/*----------------------------------------------------------------------*/
	/*	Processing during base block												*/
	/*----------------------------------------------------------------------*/
	if( BeComplete == FALSE )
	{
		MotorLessMdl->V.MotMdlpfbrem = 0;
		MotorLessMdl->V.MotMdlpfbremM = 0;
		MotorLessMdl->V.Mvar = 0;
		MotorLessMdl->V.MotMdlSfbrem = 0;
	}

	/*----------------------------------------------------------------------*/
	/*	Rigid body mechanical model														*/
	/*----------------------------------------------------------------------*/
	/* Acceleration FB	*/
	MotAcc = MlibPfbkxremNolim( TrqRef, MotorLessMdl_P->MdlKj, &MotorLessMdl->V.MotMdlSfbrem );
	/* Speed FB		*/
	MotorLessMdl->V.Mvar = MlibLimitul( (MotorLessMdl->V.Mvar + MotAcc), 0x2000000, -0x2000000 );
	MotSpd = MotorLessMdl->V.Mvar;

	/* Motor position FB difference → Return Fulk position difference when using Fulk		*/
	dFbPos = MlibPfbkxremNolim( MotorLessMdl->V.Mvar, MotorLessMdl_P->MdlKvp, &MotorLessMdl->V.MotMdlpfbrem );

	/* Motor position FB difference → Return motor position difference even when using Fulk	*/
	dMotPos = MlibPfbkxremNolim( MotorLessMdl->V.Mvar, MotorLessMdl_P->MdlKvpM, &MotorLessMdl->V.MotMdlpfbremM );

	/*----------------------------------------------------------------------*/
	/*	Create speed and position FB													*/
	/*----------------------------------------------------------------------*/
	MotorLessMdl->V.MotMdlSpdFbk = MotSpd;		/* Speed FB				[2^24/OvrSpd]	*/
	MotorLessMdl->V.MotMdldPosFbk = dMotPos;	/* Motor position FB difference	[pluse/scan]	*/
	MotorLessMdl->V.MotMdlPosFbk += dMotPos;	/* Motor position FB		[pluse]			*/
	MotorLessMdl->V.MechaMdldPosFbk = dFbPos;	/* Position FB difference			[pluse/scan]	*/
	MotorLessMdl->V.MechaMdlPosFbk += dFbPos;	/* Position FB				[pluse]			*/

	/*----------------------------------------------------------------------*/
	/*	Variable output for monitor													*/
	/*----------------------------------------------------------------------*/
//[org]	BoutV.AmonMotorLessTestMdlSpd = MotorLessMdl->V.MotMdlSpdFbk;

	return;

}


/*****************************************************************************
  Description : Motorless test mode parameter calculation
----------------------------------------------------------------------------
  Parameters:
	Fenc;					Pointer to Fulque encoder data
	Bprm;					Pointer to basic parameter definition
	Prm;					Pointer to parameter definition
	MOTLESSMDL;				Pointer to data for internal mechanical model
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void PcalMotorLessTestMode( MENCV *FencV, BPRMDAT *Bprm, PRMDATA *Prm, MOTLESSMDL *MotorLessMdl )
{

	LONG		sx;						/* Gain calculation work				*/
	LONG		kx;						/* Gain calculation work				*/
	LONG		index;					/* Editing side parameter index		*/
	MOTLESSMDLP	*wrkp;					/* Pointer to editing side parameter		*/

	/* Get the index of the updating side */
	index = 0;
	if( MotorLessMdl->PrmPtr == &(MotorLessMdl->P[index]) )
	{
		index = 1;
	}

	/* Getting editing side parameter address	*/
	wrkp = &(MotorLessMdl->P[index]);

	/*----------------------------------------------------------------------*/
	/*	Model calculation	: NormalizedSpeed -> Pulse/Scan							*/
	/*----------------------------------------------------------------------*/
	/*																		*/
	/*			  scantime[ns]												*/
	/*	MdlKvp = -----------------											*/
	/*			 Hprm.Kpx * 10^9  											*/
	/*																		*/
	/*----------------------------------------------------------------------*/
	wrkp->MdlKvp = MlibScalKxkxks( 1, 1, Bprm->Kpx, &sx,  0 );
	wrkp->MdlKvp = MlibPcalKxgain( wrkp->MdlKvp, MDLCYCLE, C10POW9, &sx, 24 );

	/*--------------------------------------------------------------------------------------*/
	/*	Motor model calculation	: NormalizedSpeed -> Pulse/Scan									*/
	/*--------------------------------------------------------------------------------------*/
	/*																						*/
	/*		  	    scantime[ns]		Bprm.MotPulse										*/
	/*	MdlKvpM = -----------------	* ------------------	Bprm.MotPulse	:[pulse/rev]	*/
	/*			   Hprm.Kpx * 10^9  	     kx 			FencP.MotPulseNo:[pulse/rev]	*/
	/*																						*/
	/*										kx : Full-Closed -> FencP.MotPulseNo			*/
	/*											 Semi-Closed -> Bprm.MotPulse				*/
	/*--------------------------------------------------------------------------------------*/
	if( Bprm->FencUse )
	{
		kx = FencV->PulseNo;							/* Full-Closed Control				*/
	}
	else
	{
		kx = Bprm->MotPulse;							/* Semi-Closed Control				*/
	}
	wrkp->MdlKvpM = MlibScalKxkxks( 1, 1, Bprm->Kpx, &sx,  0 );
	wrkp->MdlKvpM = MlibPcalKxgain( wrkp->MdlKvpM, Bprm->MotPulse, kx, &sx,  0 );
	wrkp->MdlKvpM = MlibPcalKxgain( wrkp->MdlKvpM, MDLCYCLE, C10POW9, &sx, 24 );

	/*----------------------------------------------------------------------*/
	/*	Model calculation	:  NormalizedObsJGain									*/
	/*----------------------------------------------------------------------*/
	wrkp->MdlKj = BpxNorObsJGainCal( Bprm->Kvx, Prm->jrate, 100 , MDLCYCLE );

	/*----------------------------------------------------------------------*/
	/*	Set Parameters														*/
	/*----------------------------------------------------------------------*/
	/* Update the reference pointer */
	MotorLessMdl->PrmPtr = &(MotorLessMdl->P[index]);

	return;
}


/*****************************************************************************
  Description : Motorless test mode variable initialization
----------------------------------------------------------------------------
  Parameters:
	AxisRsc;				Axis handle
  Return:
	none
----------------------------------------------------------------------------
  Note:
	Tentative version
*****************************************************************************/
void ApxInitMotorLessTestMode( AXIS_HANDLE *AxisRsc )
{
	MOTLESSTST	*MotorLessTest;		/* Motorless test modeIFPointer to definition	*/
	MOTLESSMDL	*MotorLessMdl;		/* Pointer to data for internal mechanical model		*/

	/* Initializing variables */
	MotorLessTest	= AxisRsc->MotorLessTest;
	MotorLessMdl 	= &(MotorLessTest->MotorLessMdl);

	/* APIInitialize a pointer to data that is difficult to specify with an argument	*/
	MotorLessTest->MencV	= AxisRsc->MencV;
	MotorLessTest->FencV	= AxisRsc->FencV;
	MotorLessTest->Prm		= AxisRsc->UniPrms->Prm;

	/* Area initialization */
	MlibResetLongMemory( &(MotorLessMdl->V), sizeof(MotorLessMdl->V)/4 );

	return;
}


/*****************************************************************************
  Description : Motorless test mode current data setting
	Substituting current data into FB data
----------------------------------------------------------------------------
  Parameters:
	BaseLoops;				Pointer to data for control loop calculation
	CtrlLoopOut;			ScanAPointer to output data
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void ApiSetCurDatasMotorLessTestMode( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut )
{
	/*----------------------------------------------------------------------*/
	/*	Current command, FB														*/
	/*----------------------------------------------------------------------*/
	/* q-axis current command			[15000/Imax]	*/
	CtrlLoopOut->IqRefMon = MlibMulhigh32( BaseLoops->TrqRefoLimit, 3840000 );
	/* d-axis current command			[15000/Imax]	*/
	CtrlLoopOut->IdRefMon = 0;

	CtrlLoopOut->IqRefMon_a = ( CtrlLoopOut->IqRefMon + CtrlLoopOut->IqRefMon_l ) >> 1;
	CtrlLoopOut->IqRefMon_l = CtrlLoopOut->IqRefMon;

	/* Q-axis current FB = q-axis current command			*/
	CtrlLoopOut->IqFbMon = CtrlLoopOut->IqRefMon;
	/* d-axis current FB = d-axis current command			*/
	CtrlLoopOut->IdFbMon = CtrlLoopOut->IdRefMon;

	return;
}


/*****************************************************************************
  Description : Motorless test mode speed/position data setting
	Substitute mechanical model data to FB data
----------------------------------------------------------------------------
  Parameters:
	Bprm;					Pointer to basic parameter definition
	MotorLessTest;			Motorless test modeIFPointer to definition
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void ApiSetMechaDatasMotorLessTestMode( BPRMDAT *Bprm, MOTLESSTST *MotorLessTest )
{
	MENCV		*MencV;					/* Pointer to motor encoder data	*/
	MENCV		*FencV;					/* Pointer to Fulque encoder data	*/
	MOTLESSMDL	*MotorLessMdl;			/* Pointer to data for internal mechanical model	*/
	MOTOR_LESS	*MotorLessTestDatas;	/* EncoderIF		*/

	/* Initializing variables	*/
	MencV				= MotorLessTest->MencV;
	FencV				= MotorLessTest->FencV;
	MotorLessMdl		= &(MotorLessTest->MotorLessMdl);
	MotorLessTestDatas	= &(MotorLessTest->MotorLessTestDatas);

	/*----------------------------------------------------------------------*/
	/*	Motor position FB difference													*/
	/*----------------------------------------------------------------------*/
	if( MencV->RevAsmMotor ^ Bprm->RvsDir )	/* Inversion connection XOR Reverse rotation mode	*/
	{
		MotorLessTestDatas->dSemiModePos =-MotorLessMdl->V.MotMdldPosFbk;
	}
	else
	{
		MotorLessTestDatas->dSemiModePos = MotorLessMdl->V.MotMdldPosFbk;
	}

	/*----------------------------------------------------------------------*/
	/*	Fulke position FB difference	-->		Same as motor position FB difference				*/
	/*----------------------------------------------------------------------*/
	if( FencV->RevAsmMotor ^ Bprm->RvsDir )	/* FencReverse rotation mode XOR Reverse rotation mode	*/
	{
		MotorLessTestDatas->dFullModePos =-MotorLessMdl->V.MechaMdldPosFbk;
	}
	else
	{
		MotorLessTestDatas->dFullModePos = MotorLessMdl->V.MechaMdldPosFbk;
	}

	return;

}


/*****************************************************************************
  Description : Motor parameter calculation for motorless test mode
	Set virtual motor parameters when encoder is not connectedPnFxx
----------------------------------------------------------------------------
  Parameters:
	MencV;					Pointer to motor encoder data
	Prm;					Pointer to parameter definition
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void ApiReadMotPrmMotorLessTestMode( MENCV *MencV, PRMDATA *Prm )
{
	LONG	kx;						/* Gain calculation work					*/
	LONG	sx;						/* Gain calculation work					*/
	USHORT	ivcode;					/* Input voltage code						*/
	USHORT	EncBitNo;				/* Encoder bit number					*/
	USHORT	EncType;				/* Encoder type						*/
	MENCPRM	*MencP;					/* Pointer to encoder parameter definition	*/

	/* Initializing variables	*/
	MencP = &(Prm->MencP);

	/****************************************************************************/
	/*	PnF00 : Motor type(8bit), Input voltage(4bit), Encoder type(4bit) [-]	*/
	/****************************************************************************/

	/* Motor type	*/
	MencP->typm_e.b.l = 0xFF;						/* Undefined					*/

	/* Input voltage	*/
	ivcode = ( Prm->styp.b.h & 0xFF );				/* PnE11.H					*/
	if( ivcode == 0x05 )	ivcode = 0x01;			/* 100VDouble voltage is200V, 	*/

	/* Encoder type	*/
	switch((Prm->b_prmC >> 8) & 0x0F)				/* Pn00C.2					*/
	{
		case 0x00:
		default :
				EncType = 0;						/* Incremental			*/
			break;
		case 0x01:
				EncType = 1;						/* Absolute			*/
			break;
	}

	MencP->typm_e.b.h = ivcode + (EncType << 4);

	/****************************************************************************/
	/*	PnF01 : Encoder software version [-]									*/
	/****************************************************************************/
	MencP->encver = 0x0000;

	/****************************************************************************/
	/*	PnF02 : Motor capacity [W]													*/
	/****************************************************************************/
	MencP->motw = Prm->ampw;						/* = PnE12					*/

	/****************************************************************************/
	/*	PnF03 : Encoder bit number, Position data decimal point position [-]					*/
	/****************************************************************************/
/* <S1BB> >>>>> */
	/* Pn000.3The value determines the rotation type/linear type in motorless test mode and updates the encoder information. */
	switch( (Prm->b_prm0 >> 12) & 0xF )				/* Pn000.3					*/
	{
	case 0: /* Rotary type */
		/* fall through */
	default:
		MencV->AxisMotType = MOTTYPE_ROTARY;
		break;
	case 1:	/* Linear */
		MencV->AxisMotType = MOTTYPE_LINEAR;
		break;
	}
/* <<<<< <S1BB> */
	if( MencV->AxisMotType == MOTTYPE_LINEAR )
	{
		MencP->bit_dp.b.l = 0x08;
		MencP->bit_dp.b.h = 0x44;					/* 68digit fixed				*/
	}
	else
	{
		/* Encoder resolution	*/
		switch((Prm->b_prmC >> 4) & 0x0F)			/* Pn00C.1					*/
		{
			case 0x00:	
					EncBitNo = 13;					/* 13bit					*/
				break;
			case 0x01:
					EncBitNo = 20;					/* 20bit					*/
				break;
			case 0x02:
					EncBitNo = 22;					/* 22bit					*/
				break;
			case 0x03:
			default :
					EncBitNo = 24;					/* 24bit					*/
				break;
		}

		MencP->bit_dp.b.l = EncBitNo;
		MencP->bit_dp.b.h = 0x50;					/* 80digit fixed				*/
	}

	/****************************************************************************/
	/*	PnF04 : Multiturn limit [Rev]										*/
	/****************************************************************************/
	MencP->limmlt = Prm->mtlmt;						/* = Pn205					*/

	/****************************************************************************/
	/*	PnF05 : Rated speed, Maximum speed [100min-1]									*/
	/****************************************************************************/
	MencP->vrat_max.w = Prm->MLess_vrat_max.w;		/* = PnEE0					*/

	/****************************************************************************/
	/*	PnF06 : OS detection level, Number of poles [%, -]									*/
	/****************************************************************************/
	MencP->oslv_pol.w = Prm->MLess_oslv_pol.w;		/* = PnEE1					*/

	/****************************************************************************/
	/*	PnF07 : Rated torque [0.01Nm]												*/
	/*																			*/
	/*						Motor capacity[W] * 60			Motor capacity:PnF02(=PnE12)	*/
	/*		Rated torque[Nm] = -----------------------	Rated speed  :PnF05.L(=PnEE0.L)*/
	/*						2π * Rated speed[min-1]								*/
	/*																			*/
	/****************************************************************************/
	kx = MlibScalKxgain( (LONG)MencP->motw, (60*C10POW7),  C2PAIE7, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, MencP->vrat_max.b.l, &sx, -24 );

	MencP->rattrq = MlibLimitul( kx, 0xFFFF, 0x0000 );

	/****************************************************************************/
	/*	PnF08 : Maximum torque [%]													*/
	/*																			*/
	/*		Maximum torque[%] = Maximum current * 100 / Rated current		Maximum current:PnE15			*/
	/*													Rated current:PnE14			*/
	/****************************************************************************/
	MencP->maxtrq = (((Prm->imaxs * 200)/Prm->irats) + 1) >> 1;

	/****************************************************************************/
	/*	PnF09 : Rated current [0.1A]													*/
	/****************************************************************************/
	MencP->irat = Prm->irats;						/* PnF09 = PnE14			*/

	/****************************************************************************/
	/*	PnF0A : Instantaneous maximum current [0.1A]												*/
	/****************************************************************************/
	MencP->imax = Prm->imaxs;						/* PnF0A = PnE15			*/

	/****************************************************************************/
	/*	PnF0B : EMF constant [0.1mV/min-1]										*/
	/****************************************************************************/
	MencP->emfcmp = 100;							/* Unused					*/

	/****************************************************************************/
	/*	PnF0C : Rotor inertia [10-6kgm2]										*/
	/****************************************************************************/
	MencP->jmot = Prm->Mless_jmot;					/* PnF0C = PnEE2			*/

	/****************************************************************************/
	/*	PnF0D : Armature winding resistance [0.001Ω]										*/
	/****************************************************************************/
	MencP->motr = 1000;								/* Unused					*/

	/****************************************************************************/
	/*	PnF0E : Armature inductance [0.01mH]									*/
	/****************************************************************************/
	MencP->motl = 1000;								/* Unused					*/

	/****************************************************************************/
	/*	PnF0F : Overload detection base current [%]										*/
	/****************************************************************************/
	MencP->bastrq = Prm->ibs_md.b.l;				/* PnF0F = PnE17.l			*/

	/****************************************************************************/
	/*	PnF10 : Overload detection intermediate current [%]											*/
	/****************************************************************************/
	MencP->midtrq = Prm->ibs_md.b.h * 10;			/* PnF10 = PnE17.h			*/

	/****************************************************************************/
	/*	PnF11 : Overload detection intermediate time [10s]										*/
	/****************************************************************************/
	MencP->midolt = Prm->imdt_mxt.b.l;				/* PnF11 = PnE18.l			*/

	/****************************************************************************/
	/*	PnF12 : Overload detection intermediate current 2 [%]										*/
	/****************************************************************************/
													/* PnF12 = PnE61.h	* imaxs/irats	*/
	kx = (LONG)Prm->ratmtt_2.b.h * (LONG)MencP->maxtrq / 100;
	MencP->mtrq2 = (USHORT)kx;

	/****************************************************************************/
	/*	PnF13 : Overload detection intermediate time 2 [s]										*/
	/****************************************************************************/
	MencP->mtolt2 = Prm->imdt_mxt.b.h;				/* PnF13 = PnE18.h			*/

	/****************************************************************************/
	/*	PnF14 : Phase compensation 1(Compensation value,Start speed) [deg,100min-1]						*/
	/****************************************************************************/
	MencP->phscmpM1.w = 0;							/* Unused					*/

	/****************************************************************************/
	/*	PnF15 : Pole pitch [0.1mm/180deg]										*/
	/****************************************************************************/
	MencP->ppitch = 225;							/* 22.5mmFixed				*/

	/****************************************************************************/
	/*	PnF16 : qAxis inductanceLq0 [0.01mH]									*/
	/****************************************************************************/
	MencP->motlq0 = 1000;							/* Unused					*/

	/****************************************************************************/
	/*	PnF17 : qAxis inductanceLq1 [0.01mH]									*/
	/****************************************************************************/
	MencP->motlq1 = 1000;							/* Unused					*/

	/****************************************************************************/
	/*	PnF18 : Lower: Rated torque index, Upper: Rotor inertia index [10n]			*/
	/****************************************************************************/
	MencP->exp_trat_jmot.b.l = 0;					/* Unused					*/
													/* PnF18.h = PnEE3.h		*/
	MencP->exp_trat_jmot.b.h = Prm->Mless_exp_spd_jmot.b.h;

	/****************************************************************************/
	/*	PnF19 : Lower: Rated output index, Upper: Rotation speed index [10n]						*/
	/****************************************************************************/
	MencP->exp_motw_spd.b.l = 0;					/* Unused					*/
													/* PnF19.h = PnEE3.l		*/
	MencP->exp_motw_spd.b.h = Prm->Mless_exp_spd_jmot.b.l;

	/****************************************************************************/
	/*	PnF1A : Fulque encoder bit number, Position data decimal point position [-]				*/
	/****************************************************************************/
	/* Not set here */

	/****************************************************************************/
	/*	PnF1B : Pole sensor offset amount [deg]									*/
	/****************************************************************************/
	MencP->poleoffset = 0;							/* Unused					*/

	/****************************************************************************/
	/*	PnF1C : Notch filter frequency for motor resonance suppression [Hz]						*/
	/****************************************************************************/
	MencP->motresfrq = 0;							/* Unused					*/

	/****************************************************************************/
	/*	PnF1D : Phase angle correction value [0.01deg]											*/
	/****************************************************************************/
	MencP->phscmpM2 = 0;							/* Unused					*/

	/****************************************************************************/
	/*	PnF1E : Flag [-]														*/
	/****************************************************************************/
	MencP->flg_wf = 0;								/* Unused					*/

}


/*****************************************************************************
  Description : Fulck parameter calculation for motorless test mode
	Virtual encoder parameters when Fulque encoder is not connectedPnFxx
----------------------------------------------------------------------------
  Parameters:
	MencP;					Pointer to encoder parameter definition
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void ApiReadFencPrmMotorLessTestMode( MENCPRM *MencP )
{

	/*	PnF1A : Fulque encoder bit number, Position data decimal point position [-]			*/
	MencP->BitnoDpoint.b.l = 8;				/* 8bitFixed					*/
	MencP->BitnoDpoint.b.h = 0x44;			/* 68th column fixed					*/

	return;
}

#if 0	// Currently unused
/*****************************************************************************
  Description : Motorless test mode status acquisition
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	TRUE;					Motorless test mode
	FALSE;					Normal mode
----------------------------------------------------------------------------
  Note:
	Tentative version
*****************************************************************************/
BOOL MltGetMotorLessTestMode( void )
{
	AXIS_HANDLE		*AxisRsc;			/* Axis information	*/
	MOTLESSTST		*MotorLessTest;

	/* Get the axis handle */
	/* TODO:Multiple axes support required */
	AxisRsc = GetAxisHandle(0);
	MotorLessTest = AxisRsc->MotorLessTest;

	/* Return mode status */
	return( MotorLessTest->MotorLessTestMode );
}
#endif

/*****************************************************************************
  Description : Motorless test mode flag setting
----------------------------------------------------------------------------
  Parameters:
	AxisRscI;					Axis handle(All axes)
	AxisRscNum;					Number of axes
  Return:
	none
----------------------------------------------------------------------------
  Note:
*****************************************************************************/
void MltSetMotorLessTestMode( AXIS_HANDLE *AxisRscI, INT AxisRscNum )
{
	AXIS_HANDLE 	*AxisRsc;
	PRMDATA 		*Prm;
	MOTLESSTST		*MotorLessTest;

	INT	 			ax_no;
	BOOL			flag;

	/*----------------------------------------------------------------------*/
	/*	Motorless test mode setting											*/
	/*----------------------------------------------------------------------*/
	for( ax_no = 0; ax_no < AxisRscNum; ax_no++ )
	{
		AxisRsc = &AxisRscI[ax_no];
		Prm = AxisRsc->UniPrms->Prm;
		MotorLessTest = AxisRsc->MotorLessTest;

		/* Motorless test mode */
		flag = FALSE;
		if( Prm->b_prmC & 0x0001)
		{
			flag = TRUE;
		}
		MotorLessTest->MotorLessTestMode = flag;

		/* Full-closed encoder types */
		flag = FALSE;
		if( Prm->b_prmC & 0x0100)
		{
			flag = TRUE;
		}
		MotorLessTest->MotorLessTestDatas.FencType = flag;
	}

	return;
}


#if 0	// Currently unused
/*****************************************************************************
  Description : Motorless test modeIFGet definition address
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	Motorless test modeIFAddress of definition
----------------------------------------------------------------------------
  Note:
	Tentative version
*****************************************************************************/
MOTLESSTST *MltGetMotorLessTestHandle( void )
{
	AXIS_HANDLE		*AxisRsc;			/* Axis information	*/

	/* Get the axis handle */
	/* TODO:Multiple axes support required */
	AxisRsc = GetAxisHandle(0);

	/* Return address of motorless test structure */
	return( AxisRsc->MotorLessTest );
}
#endif

/******************************* end of file  *******************************/

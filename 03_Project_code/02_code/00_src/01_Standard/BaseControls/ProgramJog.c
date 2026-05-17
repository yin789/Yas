/****************************************************************************
Description	: ProgramJog.c : ProgramJOGRelated modules
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
T.Kira 2011.03.8    First edition
****************************************************************************/
//#include "SystemSetting.h"
#include "ProgramJog.h"

/****************************************************************************
		ProgramJOGInitial parameter calculation
****************************************************************************/
/****************************************************************************
	MlibIpcalPcmdMaker()1Argument iskxgain{kx,sx}:-2^23≦kx≦2^23,0≦sx≦24However,	
	Intermediate variablekxgain{kx,sx}:-2^23≦kx≦2^23,-127≦sx≦127becomes possible,<V507>of	
	Need to delete<V527>
****************************************************************************/
void	IprmcalPrgJog( PJOGHNDL *PJogHdl, BPRMDAT *Bprm, POS_MNG_HNDL *PosManager )
{
	LONG PcmdOvrSpd, kx, sx;
	LONG Egear_a, Egear_b;

	PCMKPV  *PJogPcmk;
	PJOGV	*PJogV;

	PJogV = &PJogHdl->PJogV;
	PJogPcmk = &PJogHdl->PJogPcmk;
	Egear_a = PosManager->conf.Egear.a;
	Egear_b = PosManager->conf.Egear.b;

	kx = MlibScalKxksks( 1,  Bprm->OvrSpd,    Bprm->Kfbpls,  &sx,  0 );
	kx = MlibPcalKxgain( kx, Egear_a, Egear_b, &sx, 0 );
	kx = MlibPcalKxgain( kx, Bprm->SvCycleNs, 1000000000,   &sx, 24 );
	PcmdOvrSpd = kx;

	MlibIpcalPcmdMaker( PcmdOvrSpd, Bprm->NorMaxSpd, Bprm->NorOvrSpd, FALSE, &PJogPcmk->P );
	/* <V507> add start */
	if( PcmdOvrSpd == 0x7FFFFF )
	{
		PJogV->Cnst.PrmUnMatch2 = TRUE;
	}
	/* <V507> add end */
}

/****************************************************************************
		ProgramJOGReset operation command creation variable
****************************************************************************/
void PrgJogReset( PJOGHNDL *PJogHdl )
{
	PJogHdl->PJogV.Var.AbsPosCmd = 0;
	MlibResetLongMemory( &PJogHdl->PJogPcmk.V, sizeof( PJogHdl->PJogPcmk.V )>>2 );
}
/****************************************************************************************************/
/*																									*/
/*		ProgramJOGOperation command sequence(ScanCinCALL)												*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Program from roundJOGFrom operation operation, set operation pattern with constants					*/
/*			Scan the commandB.																	*/
/*																									*/
/*		 Sequence 0: Waiting for operation to start																	*/
/*				Sequence of operation pattern implementation according to the operation start command from round						*/
/*				Perform the process. If you use sequence1.														*/
/*																									*/
/*		 Sequence 1: Waiting for waiting time to elapse																*/
/*				Scan first operation command after waiting time elapsesB.									*/
/*				Sequence2.																		*/
/*																									*/
/*		 Sequence 2: Waiting for operation command issue															*/
/*				ScanB.												*/
/*				Command dispensing completion determines the sequence3.												*/
/*																									*/
/*		 Sequence 3: Operation end judgment																	*/
/*				It is determined whether the operation has ended by comparing the number of repetitions and the number of operations.						*/
/*				Sequence when operation is completed5.														*/
/*				Sequence if driving is not completed4.													*/
/*																									*/
/*		 Sequence 4: Branching depending on operation pattern														*/
/*				Scan driving command according to driving patternB.									*/
/*				Sequence2. For the abnormal system, output to the sequence5.												*/
/*																									*/
/*		 Sequence 5: Operation end																		*/
/*				After processing, sequence0.																*/
/*																									*/
/*		 Sequence 6: Operation interruption																		*/
/*				After processing, sequence0.																*/
/*																									*/
/*		 Sequence 7: When an alarm occurs																*/
/*				After processing, sequence0.																*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	CpxPrgJogPosSequence( PJOGHNDL *PJogHdl, SEQ_CTRL_OUT *SeqCtrlOut,
													ALMMAN_HANDLE *AlmManager, LONG FnCtrlMcmd ) 
{
	PJOGV	*PJogV;
	PCMKPV  *PJogPcmk;

	PJogV = &PJogHdl->PJogV;
	PJogPcmk = &PJogHdl->PJogPcmk;

	/* ProgramJOGMode check */
	if( FnCtrlMcmd != CTRLMODE_PJOG )
	{
		return;
	}
	
	/* ProgramJOGChecking driving conditions						*/
	/* Alarm occurrence/HWBBstate/Main circuit cutoff/DrivingOTEnabled	*/	
	if( ((AlmManager)->Status.AlmFlag != FALSE)			/* <S0E5> */ /* <S0FF> */
		|| (SeqCtrlOut->HwbbSts == TRUE)
		|| (SeqCtrlOut->MainPowerOn != TRUE)
		|| ((PJogV->Var.State == PJOG_START) && (SeqCtrlOut->OverTrvlSts == TRUE)) )
	{
		PJogV->Var.SeqPtr = 7;
	}
	else if( PJogV->Var.RoutCmd == PJOGCMD_ABORT )	/* Operation interruption */
	{
		PJogV->Var.SeqPtr = 6;
	}
	else if( (PJogV->Var.State == PJOG_START) && (SeqCtrlOut->BaseEnableReq == FALSE) )	/* Servo off during operation */
	{
		PJogV->Var.SeqPtr = 6;
	}
	if( PJogV->Var.RoutCmd == PJOGCMD_INIT )
	{
		PJogV->Var.SeqPtr = 0;
		PJogV->Var.CoutCmd = PJOGCMD_INIT;
		PJogV->Var.State = PJOG_INIT;
	}

	/* ProgramJOGOperation command sequence */
	switch ( PJogV->Var.SeqPtr )
	{
		case 0 :	/* Operation initial state	*/
			if( (PJogV->Var.RoutCmd == PJOGCMD_START) && (SeqCtrlOut->BaseEnableReq == TRUE) )
			{
				PJogV->Var.MCntr = 0;
				KlibRstLongTimer(&PJogV->Var.TimeWork);
				if( (PJogV->Cnst.MTimes == 0) && ((PJogV->Cnst.Pattern == 2) || (PJogV->Cnst.Pattern == 3)) )
				{
					PJogV->Var.SeqPtr = 7;
				}
				else
				{
					PJogV->Var.SeqPtr = 1;
				}
				PJogV->Var.State = PJOG_START;
			}
			break;

		case 1 :	/* Operation standby state	*/
			if( KlibGetLongTimerMs(PJogV->Var.TimeWork) >= (ULONG)PJogV->Cnst.WaitTime )
			{
				PrgJogReset( PJogHdl );
				PJogV->Var.CoutCmd = PJOGCMD_START;
				if( PJogV->Cnst.Pattern & 0x01 )
				{
					PJogV->Var.AbsPosCmd = -PJogV->Cnst.Distance;
				}
				else
				{
					PJogV->Var.AbsPosCmd = PJogV->Cnst.Distance;
				}
				PJogV->Var.SeqPtr = 2;
			}
			break;

		case 2 :	/* Position command payout in progress	*/
			if( PJogPcmk->V.calendf )
			{
				PJogV->Var.SeqPtr = 3;
			}
			break;

		case 3 :	/* Operation end judgment	*/
			if( (++PJogV->Var.MCntr >= PJogV->Cnst.MTimes) && (PJogV->Cnst.MTimes != 0) )
			{
				PJogV->Var.SeqPtr = 5;
			}
			else
			{
				KlibRstLongTimer(&PJogV->Var.TimeWork);
				PJogV->Var.SeqPtr = 4;
			}
			break;

		case 4 :	/* Driving pattern branch	*/
			if( KlibGetLongTimerMs(PJogV->Var.TimeWork) >= (ULONG)PJogV->Cnst.WaitTime )
			{
				switch ( PJogV->Cnst.Pattern )
				{
					case 0 :
						PJogV->Var.AbsPosCmd += PJogV->Cnst.Distance;
						break;
					case 1 :
						PJogV->Var.AbsPosCmd -= PJogV->Cnst.Distance;
						break;
					case 2 :
						if( PJogV->Var.MCntr < (PJogV->Cnst.MTimes>>1) )
						{
							PJogV->Var.AbsPosCmd += PJogV->Cnst.Distance;
						}
						else
						{
							PJogV->Var.AbsPosCmd -= PJogV->Cnst.Distance;
						}
						break;
					case 3 :
						if( PJogV->Var.MCntr < (PJogV->Cnst.MTimes>>1) )
						{
							PJogV->Var.AbsPosCmd -= PJogV->Cnst.Distance;
						}
						else
						{
							PJogV->Var.AbsPosCmd += PJogV->Cnst.Distance;
						}
						break;
					case 4 :
						if( PJogV->Var.MCntr & 0x01 )
						{
							PJogV->Var.AbsPosCmd -= PJogV->Cnst.Distance;
						}
						else
						{
							PJogV->Var.AbsPosCmd += PJogV->Cnst.Distance;
						}
						break;
					case 5 :
						if( PJogV->Var.MCntr & 0x01 )
						{
							PJogV->Var.AbsPosCmd += PJogV->Cnst.Distance;
						}
						else
						{
							PJogV->Var.AbsPosCmd -= PJogV->Cnst.Distance;
						}
						break;
					default :
						break;
				}
				if( PJogV->Cnst.Pattern < 6 )
				{
					PJogV->Var.SeqPtr = 2;
				}
				else
				{
					PJogV->Var.SeqPtr = 5;
				}
			}
			break;

		case 5 :	/* Operation end	*/
			PJogV->Var.State = PJOG_END;
			PJogV->Var.CoutCmd = PJOGCMD_INIT;
			PJogV->Var.SeqPtr = 0;
			break;

		case 6 :	/* Operation interruption	*/
			PrgJogReset( PJogHdl );		/* Stop	*/
			PJogV->Var.CoutCmd = PJOGCMD_ABORT;
			PJogV->Var.State = PJOG_ABORT;
			PJogV->Var.SeqPtr = 0;
			break;

		case 7 :	/* Alarm occurrence	*/
			PrgJogReset( PJogHdl );		/* Stop	*/
			PJogV->Var.CoutCmd = PJOGCMD_ABORT;
			PJogV->Var.State = PJOG_ERROR;
			PJogV->Var.SeqPtr = 0;
			break;

		default :
			break;
	}

	/* Set operation register	*/
	PJogHdl->PJogState = (PJogV->Var.State & 0x0F);

	/* ScanCinON,RoundinOFFflag to	*/
	PJogV->Var.ScanCRunFlg = TRUE;
}

/****************************************************************************
		Position command input processing(ProgramJOGOperation mode)(ScanBinCALL)
****************************************************************************/
BOOL PrgJogMakePositionReference( PJOGHNDL *PJogHdl, LONG *dPosRefo )
{
	PJOGV	*PJogV;
	PCMKPV  *PJogPcmk;

	PJogV = &PJogHdl->PJogV;
	PJogPcmk = &PJogHdl->PJogPcmk;

	/* Position command creation processing	[Command unit/Scan]	*/
	*dPosRefo = MlibPcmdMaker( PJogV->Var.AbsPosCmd, 0, PJogPcmk->P.maxspd, PJogPcmk, PCMKMODE_POS );
	return( (BOOL)(PJogPcmk->V.calendf ));
}

/****************************************************************************************************/


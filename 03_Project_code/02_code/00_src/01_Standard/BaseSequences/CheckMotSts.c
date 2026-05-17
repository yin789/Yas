/****************************************************************************************************/
/*																									*/
/*																									*/
/*		CheckMotSts.c : Motor status monitoring processing definition														*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Motor operating state monitoring process																	*/
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
#include "CheckMotSts.h"
#include "MLib.h"


/****************************************************************************************************/
/*																									*/
/*		Motor status detection processing																			*/
/*																									*/
/****************************************************************************************************/
void SMotSts_InputMotorStatus( SEQ_MOT_STS *MotSts, BASE_CTRL_OUT *BaseCtrlOut,
                               SEQ_CTRL_OUT *SeqCtrlOut, BPRMDAT *Bprm )
{
//	MENCV				*MencV;
	LONG				lwk0;

//	MencV = AxisRscC->MencV;

/*--------------------------------------------------------------------------------------------------*/
/*		Input Motor Pulse Data																		*/
/*--------------------------------------------------------------------------------------------------*/
	lwk0 = BaseCtrlOut->MotPos - MotSts->var.MotPos;			/* Cal. Delta Position				*/
	MotSts->var.MotPos  = BaseCtrlOut->MotPos;					/* Update Motor Position			*/
	MotSts->var.dMotPos = lwk0;									/* Update Delta Motor Position		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Input Pulse Command Data																	*/
/*--------------------------------------------------------------------------------------------------*/
	lwk0 = BaseCtrlOut->PcmdCntr - MotSts->var.PcmdCntr;		/* Cal. Delta Pulse					*/
	MotSts->var.PcmdCntr = BaseCtrlOut->PcmdCntr;				/* Update Pulse Command Counter		*/
	MotSts->var.dPcmdCnt = lwk0;								/* Update Delta Pulse Command		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Calculate Motor Speed [2^24/OvrSpd]															*/
/*--------------------------------------------------------------------------------------------------*/
	MotSts->var.LstMotSpd = MotSts->var.MotSpd;
	if( BaseCtrlOut->MotSpdCnt > 0 )
	{
		MotSts->var.MotSpd = BaseCtrlOut->MotSpdSum / BaseCtrlOut->MotSpdCnt;
	}
#if 0
	if( MencV->EncDataNG )									/* Encoder Data NG					*/
	{
		MotSts->var.MotSpd = 0;						/* Signed Motor Speed				*/
	}
#endif
	/* Absolute Motor Speed */
	MotSts->var.AbsMotSpd = MlibLABS( MotSts->var.MotSpd );

	/* Make Acceleration for Mode Switch */
	SeqCtrlOut->ModeSwAcc = MotSts->var.MotSpd - MotSts->var.LstMotSpd;

/*--------------------------------------------------------------------------------------------------*/
/*		Detect Motor Status																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* Motor rotation detection processing(TGON) */
	SeqCtrlOut->TgOnSts = LpxCheckMotorRun( MotSts, SeqCtrlOut->TgOnSts );

	/* Motor stop detection processing(MOTSTOP) */
	SeqCtrlOut->MotStop = LpxCheckMotorStop( MotSts->var.AbsMotSpd );

	/* Speed match detection process(VCMP) */
	SeqCtrlOut->VcmpSts = LpxCheckMotorVcmp( MotSts,
											 BaseCtrlOut->SpdCtrlRef,
	/*										 (CTRLMODE_SPD == BaseCtrlOut->CtrlModeOut.b.cm) );		*/
										 (( CTRLMODE_SPD == BaseCtrlOut->CtrlModeOut.b.cm ) ||
										  ( CTRLMODE_JOG == BaseCtrlOut->CtrlModeOut.b.cm ) ));	/* <S144> */
      /*Motor Suspend Level Flag <S201>*/
	SeqCtrlOut->MotSuspLvl = LpxCheckMotorSuspendLevel(MotSts,Bprm->NorSuspSpd);
/*--------------------------------------------------------------------------------------------------*/
/*		Un000,Un007 : Motor speed,Command pulse speed														*/
/*--------------------------------------------------------------------------------------------------*/
	MotSts->var.Pfbk64Sum += MotSts->var.dMotPos;
	MotSts->var.Pcmd64Sum += MotSts->var.dPcmdCnt;
	MotSts->var.MotSpd64Sum += MotSts->var.MotSpd;

	MotSts->var.Pfbk64SumCnt++;
	if( MotSts->var.Pfbk64SumCnt >= SVCDEF_UNSPD_SCMEANNUM )		/* 64msAverage calculation processing					*/
	{
#if (FLOAT_USE==TRUE)
		/* Motor speed		[pulse/64ms] -> [rpm, mm/s]	*/
		lwk0 = ((MotSts->var.MotSpd64Sum >> 4) + 1) >> 1;
		MotSts->var.UnMotSpd = lwk0 * Bprm->Kmonspd;
		MotSts->var.UnPfbk64ms = MotSts->var.Pfbk64Sum;

		/* Command pulse speed	[pulse/64ms] -> [rpm, mm/s]	*/
		MotSts->var.UnPcmd64ms = MotSts->var.Pcmd64Sum * Bprm->Kmonpcmdspd ;
		MotSts->var.UnPcmd64Sum = MotSts->var.Pcmd64Sum;		/* <S1B6> */
#else		
		/* Motor speed		[pulse/64ms] -> [rpm, mm/s]	*/
		lwk0 = ((MotSts->var.MotSpd64Sum >> 4) + 1) >> 1;
		MotSts->var.UnMotSpd = MlibMulgainSx48( lwk0, Bprm->Kmonspd );
		MotSts->var.UnPfbk64ms = MotSts->var.Pfbk64Sum;

		/* Command pulse speed	[pulse/64ms] -> [rpm, mm/s]	*/
		MotSts->var.UnPcmd64ms = MlibMulgain( MotSts->var.Pcmd64Sum, Bprm->Kmonpcmdspd );
		MotSts->var.UnPcmd64Sum = MotSts->var.Pcmd64Sum;		/* <S1B6> */
#endif /* FLOAT_USE */

		MotSts->var.Pfbk64Sum = 0;
		MotSts->var.Pcmd64Sum = 0;
		MotSts->var.Pfbk64SumCnt = 0;
		MotSts->var.MotSpd64Sum = 0;
	}

	MotSts->var.TrqRefMon = BaseCtrlOut->TrqRefMon;
/*--------------------------------------------------------------------------------------------------*/
/*		Un009 : Effective torque data 	( 10secSquared current command average value )			UnIsqrSum					*/
/*		Un142 : Effective torque data	( 2msSquare current command value)				UnCrntRef					*/
/*		Un145 : Effective torque data	( 10secSquared current command average maximum value)		UnIsqrSumMax				*/
/*--------------------------------------------------------------------------------------------------*/
	lwk0 = (BaseCtrlOut->IdRefMon * BaseCtrlOut->IdRefMon)
			+ (BaseCtrlOut->IqRefMon * BaseCtrlOut->IqRefMon);
	MotSts->var.UnCrntRef = MlibSqrtu32( lwk0 );
	MotSts->var.IsqrSum += (lwk0 >>14 );

	MotSts->var.IsqrSumCnt++;
	if( MotSts->var.IsqrSumCnt >= KPI_SCANC_MS( 10000 ) )
	{
		lwk0 = MlibSqrtu32( MotSts->var.IsqrSum );
#if (FLOAT_USE==TRUE)
		MotSts->var.UnIsqrSum = lwk0 * Bprm->Kcurload;
#else
		MotSts->var.UnIsqrSum = MlibMulgain( lwk0, Bprm->Kcurload );
#endif /* FLOAT_USE */

		if( MotSts->var.UnIsqrSum > (LONG)MotSts->var.UnIsqrSumMax )	/* Maximum value of squared current integration				*/
		{
			MotSts->var.UnIsqrSumMax = MotSts->var.UnIsqrSum;
		}
		MotSts->var.IsqrSum = 0;
		MotSts->var.IsqrSumCnt = 0;
	}
}


/****************************************************************************************************/
/*																									*/
/*		Motor rotation detection processing																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Absolute value of motor rotation speed and rotation detection level(Pn502)Compared to			*/
/*			When it is large, rotation detection output(/TGON)from connector CN1.						*/
/*			before, the rotation detection level(Pn502)was called the zero speed level.								*/
/*																									*/
/*			In this function, the motor rotation detection signal flag is an output variable.(CoutV.f.Tgon)is set up.					*/
/*			Rotation detection level(Pn502)The initial value of20rpm, so if you use it as is,								*/
/*			/TGON=LOWis rotating,/TGON=HIGH,										*/
/*																									*/
/*			Tgon = OFF→ON The conditions are																*/
/*				Motor speed is at rotation detection level(TgonSpdLevel)or above								*/
/*				orTgonSpdLevel but TGMGN The motor speed is TgonSpdLevel In the following cases				*/
/*																									*/
/*			Tgon = ON→OFF The conditions are																*/
/*				Rotation detection level(TgonSpdLevel)but TGMGN and								*/
/*				Motor speed is at detection level(TgonSpdLevel-TGMGN)or less							*/
/*																									*/
/*			TgonSpdLevel : Rotation detection level(Pn502)converted to normalized speed							*/
/*			TGMGN        : Margin including hysteresis width for chattering prevention					*/
/*							( 12710 / 16777216(24bit) * 6600(OS) ≒ 5rpm )							*/
/*																									*/
/*																									*/
/****************************************************************************************************/
BOOL LpxCheckMotorRun( SEQ_MOT_STS *MotSts, LONG LstTgonSts )
{
	BOOL	TgOn;
	TgOn = LstTgonSts;

	if( MotSts->var.AbsMotSpd >= MotSts->conf.TgonSpdLevel )
	{ /* Motor speed ≧ Rotation detection level */
		TgOn = TRUE;
	}
	else
	{ /* Motor speed < Rotation detection level */
		if( MotSts->conf.TgonSpdLevel > MOTSPD_TGMGN )
		{ /* Rotation detection level > 5[min-1] */
			if( MotSts->var.AbsMotSpd < (MotSts->conf.TgonSpdLevel - MOTSPD_TGMGN) )
			{ /* Motor speed < (Rotation detection level - 5[min-1]) */
				TgOn = FALSE;
			}
		}
		else
		{ /* Rotation detection level ≦ 5[min-1] */
			TgOn = TRUE;
		}
	}
	return TgOn;
}



/****************************************************************************************************/
/*																									*/
/*		Motor stop detection processing																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Detects that the motor has stopped.														*/
/*																									*/
/*			Motor stop detection signal can be detected by DB sequence, OT sequence, zero clamp sequence,			*/
/*			Emergency stop processing, soft reset processing, automatic offset adjustment of motor current detection signal(Fn00E)			*/
/*			used in																			*/
/*			The DB circuit is performed by rectification using a relay + diode or triac.					*/
/*			The reason why the DB current is cut off is that the motor induced voltage reduces the on-voltage of the diode or triac.			*/
/*			When the value is below.																			*/
/*			Motor stop detection signal detects that DB current is cut off and allows gate signal drive			*/
/*			Used as a condition. (It is also used for other conditions, but it should match the DB conditions.)			*/
/*																									*/
/*			The voltage at which a diode or triac turns off is approximately 0.OS rotation speed assuming around 7V			*/
/*			is considered to be a voltage saturation level of 175V, detect the runaway due to incorrect wiring of											*/
/*																									*/
/*			MOTSPD_DBLVL = 16777216(OvrSpd) x 0.7/175 = 67108 --> Look at the margin60000only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.			*/
/*																									*/
/*			When the motor rotation speed falls below the DB level and a sufficient amount of time has passed, consider that the DB circuit is disconnected			*/
/*			Good.																					*/
/*			In SGD, the motor wire was directly shorted with a relay during DB, so the DB current was transferred to the motor			*/
/*			It continues to flow until it completely stops.For this reason, it is assumed that the motor stops completely40msIf you wait, the motor will go to standby			*/
/*																		*/
/*			In SGDM, DB is a diode or triac, so even if the motor does not stop completely,		*/
/*			The DB current stops. Therefore,40msThere is no need to wait.											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
BOOL LpxCheckMotorStop( LONG AbsMotSpd )
{
	BOOL	MotStop;

	if( AbsMotSpd < MOTSPD_DBLVL )
	{
		MotStop = TRUE;								/* Setting motor stop detection signal(ON)					*/
	}
	else
	{
		MotStop = FALSE;							/* Setting motor stop detection signal(OFF)				*/
	}

	return	MotStop;
}



/****************************************************************************************************/
/*																									*/
/*		Speed match detection process																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Motor rotation speed and speed command(Before soft start)is the speed match detection width(Pn503)twist				*/
/*			If it is small, set the speed match detection signal flag.											*/
/*			Speed match detection signal output allocation(/V-CMP)is set, an external signal will be output.			*/
/*																									*/
/*																									*/
/****************************************************************************************************/
BOOL	LpxCheckMotorVcmp( SEQ_MOT_STS *MotSts, LONG SpdRef, BOOL SpdCtrlMode )
{
	BOOL	Vcmp;

	if( TRUE == SpdCtrlMode )
	{
		if( MlibLABS(SpdRef - MotSts->var.MotSpd) <= MotSts->conf.VcmpSpdLevel )
		{
			Vcmp = TRUE;
		}
		else
		{
			Vcmp = FALSE;
		}
	}
	else
	{
		Vcmp = FALSE;
	}
	return Vcmp;
}



/****************************************************************************************************/
/*																									*/
/*		Interruption speed level detection processing	<S201>																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency :    																			            */
/*			Motor speed is at maximum speed10%, set the interruption speed detection signal flag toTRUE.			*/
/*																									*/
/*			Maximum speed10%(Bprm.NorSuspSpd) = Bprm.NorMaxSpd / 10									*/
/*																							        */
/****************************************************************************************************/
BOOL  LpxCheckMotorSuspendLevel( SEQ_MOT_STS *MotSts,LONG NorSuspSpd)
{
	BOOL MotSuspLvl;  /*define a BOOL */
	if( MotSts->var.AbsMotSpd > NorSuspSpd)
	{
		MotSuspLvl = TRUE;/* Setting the interruption speed detection signal(TRUE)*/
	}
	else
	{
		MotSuspLvl = FALSE; /* Setting the interruption speed detection signal(FALSE)*/
	}
	return MotSuspLvl;

}


/***************************************** end of file **********************************************/

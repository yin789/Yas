/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PcmdFilter.c : For position command filter calculation	Module definition										*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2013.11.22	Y.Oka	For Mercury												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "PcmdFilter.h"
#include "Mlib.h"


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
/*	void	PcmdFilInitialize( PCMDFIL *PcmdFil, VIBSUPFIL *VibSupFil );							*/
/*	LONG	PcmdFilRuntimeService( PCMDFIL *PcmdFil, LONG PcmdIn, LONG FilRest, BOOL FilStop );		*/
/*--------------------------------------------------------------------------------------------------*/
/*	void	PcmdFilCalculatePrmExpFilter( EXPFIL *ExpFil, USHORT PrmTime, KSGAIN KMotSpdConv,		*/
/*														 UCHAR MaxVel, LONG SvCycleUs, LONG Flag );	*/
/*	void	PcmdFilCalculatePrmExpFilBias( EXPFIL *ExpFil, USHORT expbias1,							*/
/*													LONG Egear_a, LONG Egear_b, LONG SvCycleUs );	*/
/*	void	PcmdFilCalculatePrmMoveAvergeFilter( MAFIL *MaFil, USHORT PrmTime, USHORT Highrad,		*/
/*																	LONG SvCycleUs, LONG Flag );	*//*<S0C3>*/
/*  void    PcmdFilCalculatePrmHighFilter( MAFIL *MaFil, USHORT PrmTime, USHORT Highrad,			*/
/*																	LONG SvCycleUs, LONG Flag );	*//*<S0C3>*/
/*--------------------------------------------------------------------------------------------------*/
/*	void	PcmdFilSetRefZStatus( PCMDFIL *PcmdFil, BOOL status );									*/
/*	BOOL	PcmdFilGetRefZStatus( PCMDFIL *PcmdFil );												*/
/*--------------------------------------------------------------------------------------------------*/
/*	LONG	PcmdFilVibSupFilter( VIBSUPFIL *VibSupFil, LONG dPcmd, BOOL *RefZSignal );				*/
/*	BOOL	PcmdFilCalculatePrmVibSupFilSW( VIBSUPFIL *VibSupFil, ULONG mdlsw,						*/
/*													ULONG ff_feq, ULONG ff_fil, LONG ScanTimeNs );	*/
/*	void	PcmdFilCalculatePrmVibSupFil( VIBSUPFIL *VibSupFil, ULONG ff_frq, ULONG ff_fil,			*/
/*																				LONG ScanTimeNs );	*/
/****************************************************************************************************/
static void	pcmdFilExpFilterPrmChange( EXPFIL *ExpFil, LONG PcmdIn, LONG FilRest );
static LONG	pcmdFilExpFilter( EXPFIL *ExpFil, LONG PcmdIn );
static LONG	pcmdFilMoveAverageFilter( MAFIL *MaFil, LONG PcmdIn );
static LONG	pcmdFilLongMAFilter( MAFIL *MaFil, LONG PcmdIn );
#if	(CSW_SERVO_PCMD_HIGH_FIL_USE == TRUE)/*<S0C3>*/
static LONG	pcmdFilHighFilter( MAFIL *MaFil, LONG PcmdIn );
static LONG	pcmdFilLongHFilter( MAFIL *MaFil, LONG PcmdIn );
#endif


#if( CSW_SERVO_PCMD_FIL_USE != 0 )
/****************************************************************************************************/
/*																									*/
/*		Initializing damping control variables																			*/
/*																									*/
/****************************************************************************************************/
void	PcmdFilInitialize( PCMDFIL *PcmdFil, VIBSUPFIL *VibSupFil )
{
	/* Initialization of variables for exponential acceleration/deceleration filter */
	MlibResetLongMemory( &(PcmdFil->ExpFil.var), sizeof(PcmdFil->ExpFil.var)/4 );
	PcmdFil->ExpFil.conf.Kf[0] = PcmdFil->ExpFil.conf.Kf[1];
	PcmdFil->ExpFil.conf.Pbias[0] = PcmdFil->ExpFil.conf.Pbias[1];

	/* Initialization of variables for moving average filter */
	MlibResetLongMemory( &(PcmdFil->MaFil.var), sizeof(PcmdFil->MaFil.var)/4 );
	PcmdFil->MaFil.conf.PmafNum[0] = PcmdFil->MaFil.conf.PmafNum[1];
	PcmdFil->MaFil.conf.ImafSft[0] = PcmdFil->MaFil.conf.ImafSft[1];

	/* high filter Variable initialization for*//*<S0C3>*/
	MlibResetLongMemory( &(PcmdFil->MaFil.var), sizeof(PcmdFil->MaFil.var)/4 );
	PcmdFil->MaFil.conf.PmafNumSec[0] = PcmdFil->MaFil.conf.PmafNumSec[1];
	PcmdFil->MaFil.conf.ImafSftSec[0] = PcmdFil->MaFil.conf.ImafSftSec[1];

	/* Vibration suppression filter variable initialization */
	MlibResetLongMemory( &VibSupFil->var, sizeof(VibSupFil->var)/4 );
	VibSupFil->conf.Pexe = VibSupFil->conf.VibSupPrm;

	return;
}
/* <S1D0> Start */
/*****************************************************************************
  Description: Velocity feedforward,Initialize variables for torque feedforward moving average function
------------------------------------------------------------------------------
  Parameters:
	pMaFil;		Data for moving average filter calculation
  Return:
	NONE
------------------------------------------------------------------------------
  Note:

*****************************************************************************/
void	FFCmdFilInitialize( MAFIL *pMaFil )
{
	/* Initialization of variables for moving average filter */
	MlibResetLongMemory( &(pMaFil->var), sizeof(pMaFil->var)/4 );
	pMaFil->conf.PmafNum[0] = pMaFil->conf.PmafNum[1];
	pMaFil->conf.ImafSft[0] = pMaFil->conf.ImafSft[1];

	return;
}

/*****************************************************************************
  Description: Speed/Torque feedforward command moving average filter calculation
------------------------------------------------------------------------------
  Parameters:
	pMaFil;		Data for moving average filter calculation
	FFcmdIn		Speed before moving average filter/Torque feedforward command
	FilStop		Speed/Torque feedforward moving average filter disabled
  Return:
	LONG		Speed after moving average filter/Torque feedforward command
------------------------------------------------------------------------------
  Note:
	Machine Efficiency : Speed/Perform torque feedforward command moving average filter processing.
		1)Moving average time ≦ (MAFIL_BUFNUM * Cycle) : Moving average filter
		2)Moving average time > (MAFIL_BUFNUM * Cycle) : Moving average filter + Interpolation moving average filter
*****************************************************************************/
LONG	FFcmdFilMoveAverageFilter( MAFIL *pMaFil, LONG FFcmdIn, BOOL FilStop )
{
	/* <S1DF> Start */
	LONG SaveFFcmd;

	SaveFFcmd = FFcmdIn;
	/* 2^24→15000 */
	FFcmdIn = MlibMulhigh32( FFcmdIn, 3840000 );
	/* <S1DF> End */
/*--------------------------------------------------------------------------------------------------*/
/*		Moving average filter calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( pMaFil->conf.ImafSft[0] == 0 )
	{
		/* Moving average filter */
		pMaFil->var.Filo = MlibPcmdMafil( FFcmdIn,
										 pMaFil->conf.PmafNum[0],
										 &pMaFil->var.Pmafvar,
										 &pMaFil->var.Pmafbuf[0] );
	}
	else
	{
		/* Long-term moving average filter */
		pMaFil->var.Filo = pcmdFilLongMAFilter( pMaFil, FFcmdIn );
	}
	/* 15000→2^24 *//* <S200> */
	pMaFil->var.Filo = MlibMulhigh32( (pMaFil->var.Filo<<16), 73300775 );		/* <S200> */
/*--------------------------------------------------------------------------------------------------*/
/*		Change parameters&Payout completion signal processing															*/
/*--------------------------------------------------------------------------------------------------*/
	if ( FilStop == TRUE )
	{	/* Function disabled processing */
//		pMaFil->var.Filo = FFcmdIn;		/* <S1DF> */
		pMaFil->var.Filo = SaveFFcmd;	/* <S1DF> */
	}
/*--------------------------------------------------------------------------------------------------*/
	/* 15000→2^24 *//* <S1DF> */
//	pMaFil->var.Filo = MlibMulhigh32( (pMaFil->var.Filo<<16), 73300775 );		/* <S1DF> *//* <S200> */
	return( pMaFil->var.Filo );

}
/* <S1D0> End */


/****************************************************************************************************/
/*																									*/
/*		Position command filter processing main function																*/
/*																									*/
/****************************************************************************************************/
LONG	PcmdFilRuntimeService( PCMDFIL *PcmdFil, LONG PcmdIn, LONG FilRest, BOOL FilStop )
{
	LONG	PcmdOut;
	LONG	OrgPcmdIn;

/*--------------------------------------------------------------------------------------------------*/
/*		Execute position command filter/Stop processing																*/
/*--------------------------------------------------------------------------------------------------*/
	if ( FilStop == TRUE )
	{
		OrgPcmdIn = PcmdIn;
		PcmdIn = 0;
	}
	else
	{
		OrgPcmdIn = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Filter selection by position command filter selection user constant&Execution processing									*/
/*--------------------------------------------------------------------------------------------------*/
	/* Moving average filter calculation */
	PcmdOut = pcmdFilMoveAverageFilter( &(PcmdFil->MaFil), PcmdIn );

	/* high filter Calculation *//*<S0C3>*/
#if	(CSW_SERVO_PCMD_HIGH_FIL_USE == TRUE)
	PcmdOut = pcmdFilHighFilter( &(PcmdFil->MaFil), PcmdOut );
#endif

	/* Exponential acceleration/deceleration filter calculation */
	pcmdFilExpFilterPrmChange( &(PcmdFil->ExpFil), PcmdOut, FilRest );
	PcmdOut = pcmdFilExpFilter( &(PcmdFil->ExpFil), PcmdOut );

/*--------------------------------------------------------------------------------------------------*/
/*		Adding actual position command to filter calculation result														*/
/*--------------------------------------------------------------------------------------------------*/
	PcmdOut += OrgPcmdIn;

	return( PcmdOut );
}



/****************************************************************************************************/
/*																									*/
/*		Position command index acceleration/deceleration filter parameter change processing												*/
/*																									*/
/****************************************************************************************************/
static void	pcmdFilExpFilterPrmChange( EXPFIL *ExpFil, LONG PcmdIn, LONG FilRest )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter change processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	if ( (PcmdIn == 0) && (FilRest == 0) )
	{													/* Position command = 0 && Filter accumulation = 0		*/
		ExpFil->conf.Kf[0] = ExpFil->conf.Kf[1];		/* Parameter change enabled						*/
		ExpFil->conf.Pbias[0] = ExpFil->conf.Pbias[1];	/* Parameter change enabled						*/
		if( ExpFil->conf.Kf[0] == 1 )					/* Gain1bias when0Force setting to			*/
		{
			ExpFil->conf.Pbias[0] = 0;
		}
	}
//	else /* Σ-7toFilChgDisableBecause there is no process to confirm,Delete */
//	{
//		FilMng.FilChgDisable = TRUE;					/* Filter switching prohibition setting					*/
//	}
	/*--------------------------------------------------------------------------------------------------*/
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Position command index acceleration/deceleration filter calculation																*/
/*																									*/
/****************************************************************************************************/
static LONG	pcmdFilExpFilter( EXPFIL *ExpFil, LONG PcmdIn )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Acceleration/deceleration filter calculation																			*/
/*--------------------------------------------------------------------------------------------------*/
	ExpFil->var.Pexfvar.sumx = ExpFil->var.Pexfvar.sumx + PcmdIn;
	if ( MlibAbs32( ExpFil->var.Pexfvar.sumx ) > ExpFil->conf.Pbias[0] )
	{
		ExpFil->var.Filo = MlibMulkxBiasrem( ExpFil->var.Pexfvar.sumx,
											 ExpFil->conf.Kf[0],
											 &ExpFil->var.Pexfvar.remx,
											 ExpFil->conf.Pbias[0] );
	}
	else
	{
		ExpFil->var.Filo = ExpFil->var.Pexfvar.sumx;
	}
	ExpFil->var.Pexfvar.sumx = ExpFil->var.Pexfvar.sumx - ExpFil->var.Filo;
/*--------------------------------------------------------------------------------------------------*/
/*		Payout complete signal processing																			*/
/*--------------------------------------------------------------------------------------------------*/
	if ( ( PcmdIn != 0 ) || ( ExpFil->var.Pexfvar.sumx != 0 ) )
	{
//		BoutV.f.Sv.Den = FALSE;							/* Position command payout incomplete					*/
		ExpFil->var.RefZStatus = FALSE;					/* Position command payout incomplete					*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return( ExpFil->var.Filo );
}



/****************************************************************************************************/
/*																									*/
/*		Position command moving average filter calculation																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Perform position command moving average filter processing.													*/
/*																									*/
/*			1)Moving average time ≦ (MAFIL_BUFNUM * Cycle) : Moving average filter								*/
/*			2)Moving average time > (MAFIL_BUFNUM * Cycle) : Moving average filter + Interpolation moving average filter		*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static LONG	pcmdFilMoveAverageFilter( MAFIL *MaFil, LONG PcmdIn )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Moving average filter calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( MaFil->conf.ImafSft[0] == 0 )
	{
		/* Moving average filter */
		MaFil->var.Filo = MlibPcmdMafil( PcmdIn,
										 MaFil->conf.PmafNum[0],
										 &MaFil->var.Pmafvar,
										 &MaFil->var.Pmafbuf[0] );
	}
	else
	{
		/* Long-term moving average filter */
		MaFil->var.Filo = pcmdFilLongMAFilter( MaFil, PcmdIn );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Change parameters&Payout completion signal processing															*/
/*--------------------------------------------------------------------------------------------------*/
	if( (PcmdIn | MaFil->var.PcmdInX | MaFil->var.Pmafvar.zcntx | MaFil->var.Pimfvar.zcntx) == 0 )
	{
		MaFil->conf.PmafNum[0] = MaFil->conf.PmafNum[1];	/* Position command=0 && Payout complete			*/
		MaFil->conf.ImafSft[0] = MaFil->conf.ImafSft[1];	/* Parameter change enabled					*/
	}
	else
	{
//		BoutV.f.Sv.Den = FALSE;							/* Position command payout incomplete					*/
		MaFil->var.RefZStatus = FALSE;					/* Position command payout incomplete					*/
//		FilMng.FilChgDisable = TRUE;					/* Filter switching prohibition setting					*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return( MaFil->var.Filo );

}

/****************************************************************************************************/
/*	                                                         Position command high filter Processing                                                                                                          *//*<S0C3>*/
/****************************************************************************************************/
#if	(CSW_SERVO_PCMD_HIGH_FIL_USE == TRUE)
static LONG	pcmdFilHighFilter( MAFIL *MaFil, LONG PcmdIn )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Moving average filter2Calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	if( MaFil->conf.ImafSftSec[0] == 0 )
	{
		/* Moving average filter */
		MaFil->var.FiloSec = MlibPcmdMafilSec( PcmdIn,
										 MaFil->conf.PmafNumSec[0],
										 &MaFil->var.PmafvarSec,
										 &MaFil->var.PmafbufSec[0] );
	}
	else
	{
		/* Long-term moving average filter */
		MaFil->var.FiloSec = pcmdFilLongHFilter( MaFil, PcmdIn );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Change parameters&Payout completion signal processing															*/
/*--------------------------------------------------------------------------------------------------*/
	if( (PcmdIn | MaFil->var.PcmdInXSec | MaFil->var.PmafvarSec.zcntxSec | MaFil->var.PimfvarSec.zcntxSec) == 0 )
	{
		MaFil->conf.PmafNumSec[0] = MaFil->conf.PmafNumSec[1];	/* Position command=0 && Payout complete			*/
		MaFil->conf.ImafSftSec[0] = MaFil->conf.ImafSftSec[1];	/* Parameter change enabled					*/
	}
	else
	{
//		BoutV.f.Sv.Den = FALSE;							/* Position command payout incomplete					*/
		MaFil->var.RefZStatus = FALSE;					/* Position command payout incomplete					*/
//		FilMng.FilChgDisable = TRUE;					/* Filter switching prohibition setting					*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return( MaFil->var.FiloSec );

}
#endif


/****************************************************************************************************/
/*																									*/
/*		Long-term moving average filter calculation																	*/
/*																									*/
/****************************************************************************************************/
static LONG	pcmdFilLongMAFilter( MAFIL *MaFil, LONG PcmdIn )
{
	LONG	pcmdfilo;

/*--------------------------------------------------------------------------------------------------*/
/*		Moving average filter : Until output starts,Interpolation moving average time((1<<ImafSft)*Cycle)Required.				*/
/*--------------------------------------------------------------------------------------------------*/
	MaFil->var.PcmdInX += PcmdIn;
	if( ++MaFil->var.Index >= (1 << MaFil->conf.ImafSft[0]) )
	{
		MaFil->var.PcmdInY = MlibPcmdMafil( MaFil->var.PcmdInX,
											MaFil->conf.PmafNum[0],
											&MaFil->var.Pmafvar,
											&MaFil->var.Pmafbuf[0] );
	/*----------------------------------------------------------------------------------------------*/
		MaFil->var.Index   = 0;
		MaFil->var.PcmdInX = 0;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Interpolation moving average filter																		*/
/*--------------------------------------------------------------------------------------------------*/
	pcmdfilo = MlibPcmdImafil( MaFil->var.PcmdInY,
							   MaFil->conf.ImafSft[0],
							   MaFil->var.Index,
							   &MaFil->var.Pimfvar );
/*--------------------------------------------------------------------------------------------------*/
	return( pcmdfilo );

}

/****************************************************************************************************/
/*		                                                        Long-term moving average filter Calculation				               			*/
/****************************************************************************************************//*<S0C3>*/
#if	(CSW_SERVO_PCMD_HIGH_FIL_USE == TRUE)
static LONG	pcmdFilLongHFilter( MAFIL *MaFil, LONG PcmdIn )
{
	LONG	pcmdfilo;

/*--------------------------------------------------------------------------------------------------*/
/*		                     Moving average filter : Until output starts,Interpolation moving average time((1<<ImafSft)*Cycle)Required.	     			*/
/*--------------------------------------------------------------------------------------------------*/
	MaFil->var.PcmdInXSec += PcmdIn;
	if( ++MaFil->var.IndexSec >= (1 << MaFil->conf.ImafSftSec[0]) )
	{
		MaFil->var.PcmdInYSec = MlibPcmdMafilSec( MaFil->var.PcmdInXSec,
												  MaFil->conf.PmafNumSec[0],
												  &MaFil->var.PmafvarSec,
												  &MaFil->var.PmafbufSec[0] );
	/*----------------------------------------------------------------------------------------------*/
			MaFil->var.IndexSec   = 0;
			MaFil->var.PcmdInXSec = 0;
	}
/*--------------------------------------------------------------------------------------------------*/
/*		        Interpolation moving average filter																			*/
/*--------------------------------------------------------------------------------------------------*/
	pcmdfilo = MlibPcmdImafilSec( MaFil->var.PcmdInYSec,
								   MaFil->conf.ImafSftSec[0],
								   MaFil->var.IndexSec,
								   &MaFil->var.PimfvarSec );
/*--------------------------------------------------------------------------------------------------*/
	return( pcmdfilo );

}
#endif


/****************************************************************************************************/
/*																									*/
/*		Position command acceleration/deceleration filter parameter calculation														*/
/*																									*/
/****************************************************************************************************/
void	PcmdFilCalculatePrmExpFilter( EXPFIL *ExpFil, USHORT PrmTime, KSGAIN KMotSpdConv,
														UCHAR MaxVel, LONG SvCycleUs, LONG Flag )
{
	LONG	Unit;
	LONG	PrmTimeUs;
	LONG	kx,sx;

	Unit = ( !Flag ) ? 10 : 100;
	PrmTimeUs = PrmTime * Unit;

	/*----------------------------------------------------------------------------------------------*/
	/* kx(Upper limit value of position command acceleration/deceleration filter)is not dependent on the control cycle										*/
	/* Maximum speed and number of calculation bits(Σ-V/Σ-7In the case of31bit)depends on											*/
	/*----------------------------------------------------------------------------------------------*/
	kx = MlibScalKxkxks( SUMX_LIM, 100, KMotSpdConv, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, MaxVel*100, &sx, -24 );
	PrmTimeUs = ( kx < PrmTimeUs ) ? kx : PrmTimeUs ;
	/* <V507> add end */
	if( PrmTimeUs <= SvCycleUs )				/* Time constant ≦ Up to the control cycle,Gain=1				*/
	{
		ExpFil->conf.Kf[1] = 1;
	}
	else										/* Kf = SvCycleUs / PrmTimeUsApproximate calculation with				*/
	{
		ExpFil->conf.Kf[1] = MlibScalKxgain( SvCycleUs, 1, PrmTimeUs, NULL, 24 );
	}
/*--------------------------------------------------------------------------------------------------*/
	return;

}



/****************************************************************************************************/
/*																									*/
/*		Position command acceleration/deceleration filter bias parameter calculation												*/
/*																									*/
/****************************************************************************************************/
void	PcmdFilCalculatePrmExpFilBias( EXPFIL *ExpFil, USHORT expbias1,
														LONG Egear_a, LONG Egear_b, LONG SvCycleUs )
{
	LONG	kx;
	LONG	sx;

	kx = MlibScalKxgain( expbias1, Egear_b, Egear_a, &sx, 0 );
	kx = MlibPcalKxgain( kx, SvCycleUs, 1000000, &sx, 0 );
	ExpFil->conf.Pbias[1] = (sx<=31)? (kx>>sx) : 0;
	/*--------------------------------------------------------------------------------------------------*/
	return;

}



/****************************************************************************************************/
/*																									*/
/*		Position command moving average filter parameter calculation														*/
/*																									*/
/****************************************************************************************************/
void	PcmdFilCalculatePrmMoveAvergeFilter( MAFIL *MaFil, USHORT PrmTime, USHORT Highrad, LONG SvCycleUs, LONG Flag )/*<S0C3>*/
{
	LONG	i,x;
	LONG	MaTimeUs;

#if	(CSW_SERVO_PCMD_HIGH_FIL_USE == TRUE)/*<S0C3>*/
	USHORT  rad; /*<S0C3>*/

	rad = Highrad>>1;
	MaTimeUs= (Flag==0)? (10*PrmTime-0.1*PrmTime*rad) : (100*PrmTime-PrmTime*rad);/* Flag=0:[0.01ms], Flag=1:[0.1ms]		*/
#else
	MaTimeUs= (Flag==0)? (10*PrmTime) : (100*PrmTime);		/* Flag=0:[0.01ms], Flag=1:[0.1ms]		*/
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Short time filter:Normal moving average filter														*/
/*--------------------------------------------------------------------------------------------------*/
	if( MaTimeUs <= (MAFIL_BUFNUM * SvCycleUs) )
	{
		MaFil->conf.ImafSft[1] = 0;
		MaFil->conf.PmafNum[1] = MaTimeUs / SvCycleUs;
		if( MaFil->conf.PmafNum[1] < 1 ){ MaFil->conf.PmafNum[1] = 1;}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Long-term filter:Moving average filter+Interpolation moving average filter										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Maximum interpolation moving average time = Cycle * (1<<imax) = 125us * 32 = 4ms								*/
/*		Maximum total moving average time = Maximum interpolation moving average time * MAFIL_BUFNUM = 4ms * 256 = 1024ms			*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		x = MAFIL_BUFNUM * SvCycleUs;						/* Interpolation moving average count calculation					*/
		for( i=1; i < 5; i++ )								/* The number of interpolation moving averages is,					*/
		{													/* (1<<i) = 2,4,8,...					*/
			if( (x << i) >= MaTimeUs ){ break;}				/*										*/
		}													/*										*/
	/*----------------------------------------------------------------------------------------------*/
		MaFil->conf.ImafSft[1] = (UCHAR)i;
		MaFil->conf.PmafNum[1] = MaTimeUs / (SvCycleUs << i);
		if( MaFil->conf.PmafNum[1] > MAFIL_BUFNUM ){ MaFil->conf.PmafNum[1] = MAFIL_BUFNUM;}
	}
/*--------------------------------------------------------------------------------------------------*/
	return;

}

#if	(CSW_SERVO_PCMD_HIGH_FIL_USE == TRUE)
/*-----------------------------------------------------------------------------------high filter---------*//*<S0C3>*/
void	PcmdFilCalculatePrmHighFilter( MAFIL *MaFil, USHORT PrmTime, USHORT Highrad, LONG SvCycleUs, LONG Flag )
{
	LONG	i,x;
	LONG	MaTimeUs;
	USHORT  rad;

    rad = Highrad>>1;
    MaTimeUs= (Flag==0)? (0.1*PrmTime*rad) : (PrmTime*rad);   /* Flag=0:[0.01ms], Flag=1:[0.1ms]		*/
/*--------------------------------------------------------------------------------------------------*/
/*		        Short time filter:Normal moving average filter                														*/
/*--------------------------------------------------------------------------------------------------*/
    if( MaTimeUs <= (MAFIL_BUFNUM * SvCycleUs) )
    {
    	MaFil->conf.ImafSftSec[1] = 0;
    	MaFil->conf.PmafNumSec[1] = MaTimeUs / SvCycleUs;
    	if( MaFil->conf.PmafNumSec[1] < 1 ){ MaFil->conf.PmafNumSec[1] = 1;}
    }
/*--------------------------------------------------------------------------------------------------*/
/*		Long-term filter:Moving average filter+Interpolation moving average filter										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Maximum interpolation moving average time = Cycle * (1<<imax) = 125us * 32 = 4ms								*/
/*		Maximum total moving average time = Maximum interpolation moving average time * MAFIL_BUFNUM = 4ms * 256 = 1024ms			*/
/*--------------------------------------------------------------------------------------------------*/
    else
    {
    	x = MAFIL_BUFNUM * SvCycleUs;						/*  Interpolation moving average count calculation 					*/
    	for( i=1; i < 5; i++ )								/*  The number of interpolation moving averages is,				*/
    	{													/*  (1<<i) = 2,4,8,...					*/
    		if( (x << i) >= MaTimeUs ){ break;}				/*										*/
    	}													/*										*/
    /*----------------------------------------------------------------------------------------------*/
    		MaFil->conf.ImafSftSec[1] = (UCHAR)i;
    		MaFil->conf.PmafNumSec[1] = MaTimeUs / (SvCycleUs << i);
    		if( MaFil->conf.PmafNumSec[1] > MAFIL_BUFNUM ){ MaFil->conf.PmafNumSec[1] = MAFIL_BUFNUM;}
    }
/*--------------------------------------------------------------------------------------------------*/
    return;

}
#endif
#endif /* #if( CSW_SERVO_PCMD_FIL_USE != 0 ) */



/****************************************************************************************************/
/*																									*/
/*		Setting position command filter dispensing completion stateAPI														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		In case of position control mode,Payout completion status(RefZ)are set from the upper module.				*/
/*		However,,Depending on the state of position command filter calculation, output may not be completed.In this case,,		*/
/*		Obtain position command filter dispensing completion stateAPI PcmdFilGetRefZStatus(),Get the status.		*/
/*																									*/
/****************************************************************************************************/
void	PcmdFilSetRefZStatus( PCMDFIL *PcmdFil, BOOL status )
{
	PcmdFil->ExpFil.var.RefZStatus = status;
	PcmdFil->MaFil.var.RefZStatus = status;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Obtain position command filter dispensing completion stateAPI														*/
/*																									*/
/****************************************************************************************************/
BOOL	PcmdFilGetRefZStatus( PCMDFIL *PcmdFil )
{
	return	(PcmdFil->ExpFil.var.RefZStatus & PcmdFil->MaFil.var.RefZStatus);
}



#if ( CSW_SVCF_RESVIB_FIL_USE == TRUE )
/****************************************************************************************************/
/*																									*/
/*		Vibration suppression filter calculation																		*/
/*																									*/
/*	Overview:	Vibration suppression filter,It is a filter with notch characteristics for the set frequency.			*/
/*			Suppressing vibrations during positioning such as machine vibration.												*/
/*			Moving average equal position command filter,MFCcan also be connected in series.										*/
/*																									*/
/****************************************************************************************************/
LONG	PcmdFilVibSupFilter( VIBSUPFIL *VibSupFil, LONG dPcmd, BOOL *RefZSignal )
{
	LONG	AvffFili;
	LONG	AvffFilo;
	LONG	x1;
	LONG	x2;
	LONG	x3;
	LONG	wk = 0;

/*--------------------------------------------------------------------------------------------------*/
/*	Command completed&Payout completion check																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( (dPcmd == 0) && (VibSupFil->var.Buf == 0) )
	{ /* Command complete && No filter accumulation->Payout complete */
		VibSupFil->conf.Pexe = VibSupFil->conf.VibSupPrm;		/* Parameter switching execution				*/
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
		{ /* Position command payout incomplete */
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
		wk += AvffFilo;											/* When function is disabled					*/
		VibSupFil->var.Filo = wk;								/* Filter output+Output command input		*/
	}
	else
	{	/* Function disabled	*/
		wk = AvffFili;											/* Output input as is				*/
		VibSupFil->var.Filo = wk;
	}

	return( wk );
}


/****************************************************************************************************/
/*																									*/
/*		Vibration suppression filter selection																		*/
/*																									*/
/****************************************************************************************************/
BOOL	PcmdFilCalculatePrmVibSupFilSW( VIBSUPFIL *VibSupFil, ULONG mdlsw,
													ULONG ff_feq, ULONG ff_fil, LONG ScanTimeNs )
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
		/* Vibration suppression filter enforcementOFF */
		VibSupFil->conf.VibSupSetting = FALSE;
		break;
	}

	/* Settings reflected to the function selection switch for execution */
//	PosMngCalculatePrmVibSupFil( VibSupFil, ff_feq, ff_fil, ScanTimeNs ); /* <S065> */
	PcmdFilCalculatePrmVibSupFil( VibSupFil, ff_feq, ff_fil, ScanTimeNs );

	return	PrmSetErr;
}




/****************************************************************************************************/
/*																									*/
/*		Vibration suppression filter parameter calculation																*/
/*																									*/
/****************************************************************************************************/
void	PcmdFilCalculatePrmVibSupFil( VIBSUPFIL *VibSupFil, ULONG ff_frq, ULONG ff_fil, LONG ScanTimeNs )
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
/*		ω2maximum value																					*/
/*																									*/
/*					      (ff_frq/10)[Hz] * (ff_fil[%]/100) * (80/100)* 10^9						*/
/*			omega2_limit = --------------------------------------------------------					*/
/*				   		               2 * PAI * CycleNs											*/
/*--------------------------------------------------------------------------------------------------*/
	wk = (LONG)ff_frq * (LONG)ff_fil;					/* Parameter settingω2*1000[Hz]				*/
	omega2 = MlibMIN( wk, (MAXOMEGA2(ScanTimeNs) * 1000) );	/* *1000[Hz]Limits and minimum parameter values	*/

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



/***************************************** end of file **********************************************/

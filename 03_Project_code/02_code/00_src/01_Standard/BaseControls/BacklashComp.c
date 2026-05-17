/****************************************************************************************************/
/*																									*/
/*																									*/
/*		BacklashComp.c : Backlash correction function	module definition										*/
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
/*	Note	:	First edition	2011.04.05	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "BacklashComp.h"



#if (CSW_SVCF_BACKLASHCOMP == TRUE)
/****************************************************************************************************/
/*																									*/
/*		Backlash correction processing																		*/
/*																									*/
/****************************************************************************************************/
void	BlshCompensateBacklash( BLSHCMP *BlshCmp, EGEAR *Egear,
									LONG *dPosRef, LONG *dPcmd, BOOL *RefZ, BOOL BlshActive )
{
	LONG	dAmountPls;
	LONG	work;		/* <S1D8> */

	/*------------------------------------------------------------------------------------------*/
	/*		During base block, etc.: Return to the state without correction(Return to motor position)						*/
	/*------------------------------------------------------------------------------------------*/
	if( BlshActive == FALSE )
	{
		*dPosRef -= BlshCmp->var.LastFilo;
#if (FLOAT_USE==TRUE)
		*dPcmd += FlibEgearRvscnv( -(BlshCmp->var.LastFilo), Egear, &BlshCmp->var.Cmpgrrem );
#else
		*dPcmd += MlibEgearRvscnv( -(BlshCmp->var.LastFilo), Egear, &BlshCmp->var.Cmpgrrem );
#endif /* FLOAT_USE */
		BlshCmp->var.BlshCmpAmountPls = -(BlshCmp->var.LastFilo);
		work = BlshCmp->var.BlshCmpAmountPls;								/* <S1D8> */
		MlibResetLongMemory( &BlshCmp->var, sizeof(BlshCmp->var)/4 );
		BlshCmp->var.BlshCmpAmountPls = work;			/* Backlash correction amount difference value is not applicable	*//* <S1D8> */

	}
	/*------------------------------------------------------------------------------------------*/
	/*		When base is enabled: Backlash correction processing calculation										*/
	/*------------------------------------------------------------------------------------------*/
	else
	{
		/*--------------------------------------------------------------------------------------*/
		/*	Backlash correction direction determination, correction amount setting												*/
		/*--------------------------------------------------------------------------------------*/
		if( *dPosRef != 0 )
		{
			if( (*dPosRef > 0) && (BlshCmp->conf.BacklashDir == FOR_BACKLASH) )
			{
				BlshCmp->var.Cmppos = BlshCmp->conf.CompPulse;
			}
			else if( (*dPosRef < 0) && (BlshCmp->conf.BacklashDir == REV_BACKLASH) )
			{
				BlshCmp->var.Cmppos = -BlshCmp->conf.CompPulse;
			}
			else
			{
				BlshCmp->var.Cmppos = 0;
			}
		}
		/*--------------------------------------------------------------------------------------*/
		/*	Backlash correction amount filter processing														*/
		/*--------------------------------------------------------------------------------------*/
		dAmountPls = MlibMulkxBiasrem( BlshCmp->var.Cmppos - BlshCmp->var.LastFilo,
									   BlshCmp->conf.Kfil,
									   &BlshCmp->var.remx,
									   0 );
		BlshCmp->var.Filo = BlshCmp->var.LastFilo + dAmountPls;		/* Correction amount to date			*/
		BlshCmp->var.LastFilo = BlshCmp->var.Filo;					/* Update filter output previous value	*/
		/*--------------------------------------------------------------------------------------*/
		/*	Backlash correction amount command addition process													*/
		/*		*dPosRef: Add to pulse unit position command,*dPcmd: Add to position command in command unit			*/
		/*--------------------------------------------------------------------------------------*/
		*dPosRef += dAmountPls;
#if (FLOAT_USE==TRUE)
		*dPcmd += FlibEgearRvscnv( dAmountPls, Egear, &BlshCmp->var.Cmpgrrem );
#else
		*dPcmd += MlibEgearRvscnv( dAmountPls, Egear, &BlshCmp->var.Cmpgrrem );
#endif /* FLOAT_USE */
		/*--------------------------------------------------------------------------------------*/
		/*	FBUpdate backlash correction amount difference value for position information monitor processing									*/
		/*		BoutV.BlshCmpAmountPls: Pulse unitFBFor subtraction from position difference value					*/
		/*--------------------------------------------------------------------------------------*/
		BlshCmp->var.BlshCmpAmountPls = dAmountPls;
	}
	/*------------------------------------------------------------------------------------------*/
	/*	Position command payout complete (DENSignal) update														*/
	/*		Command with added correction amount!=0 or Correction amount filter buffer!=0:DENSignal=FALSE(Incomplete)			*/
	/*------------------------------------------------------------------------------------------*/
	if( (*dPosRef!=0) || ((BlshCmp->var.Cmppos - BlshCmp->var.LastFilo) != 0) )
	{
		*RefZ = FALSE;
	}
	else
	{
		/* In cases other than the above, correction filter gain parameter is enabled */
		BlshCmp->conf.Kfil = BlshCmp->conf.KfilTmp;
	}

	return;
}
#endif


#if (CSW_SVCF_BACKLASHCOMP == TRUE)
/****************************************************************************************************/
/*																									*/
/*		Backlash correction Correction amount limit value calculation															*/
/*																									*/
/****************************************************************************************************/
void	BlshCalcMaxBlshCompPulse( BLSHCMP *BlshCmp, EGEAR *Egear, BPRMDAT *Bprm )
{
	/*----------------------------------------------------------------------------------------------*/
	/*	2×Motor maximum speed pulse calculation (MaxCompPulse): Pulse unit/sec→Pulse unit/ScanB[us]			*/
	/*		Bprm.MaxPspd:[Pulse/sec]																*/
	/*----------------------------------------------------------------------------------------------*/
	BlshCmp->conf.MaxCompPulse = MlibScalKxkskx( Bprm->SvCycleUs, Bprm->MaxPspd, 500000, NULL, -30 );
//	blcm2_UpLim = ULONGOF( pndef_blcm2l.UpperLimit, pndef_blcm2h.UpperLimit );

	/*----------------------------------------------------------------------------------------------*/
	/*	Backlash correction amount limit value calculation (for monitor)   : Pulse unit/ScanB[us]→0.1Command unit/ScanB[us]		*/
	/*----------------------------------------------------------------------------------------------*/
	BlshCmp->conf.MaxCompRef =
		(ULONG)MlibPcalKxgain( BlshCmp->conf.MaxCompPulse*10, Egear->a, Egear->b, NULL, -30 );
	BlshCmp->conf.MaxCompRef =
		( BlshCmp->conf.MaxCompRef > BLSHCOMP_UPLIM )? BLSHCOMP_UPLIM : BlshCmp->conf.MaxCompRef;

	return;
}
#endif


#if (CSW_SVCF_BACKLASHCOMP == TRUE)
/****************************************************************************************************/
/*																									*/
/*		Backlash correction Correction amount (Pn231) Parameter calculation												*/
/*																									*/
/****************************************************************************************************/
void	BlshCalcCompensateAmount( BLSHCMP *BlshCmp, EGEAR *Egear, LONG blcm2l )
{
	LONG	lwk;
	LONG	kx;
	LONG	sx;

	/*----------------------------------------------------------------------------------------------*/
	/*	Pn231,Pn232Calculation (PrmCompPulse): Command unit→Pulse unit										*/
	/*----------------------------------------------------------------------------------------------*/
	kx = MlibScalKxgain( blcm2l, 1, 10, &sx, 0 );
	lwk = MlibPcalKxgain( kx, Egear->b, Egear->a, &sx, -30 );

	/*----------------------------------------------------------------------------------------------*/
	/*	Correction amount pulse limit processing (BlshCmp->conf.CompPulse)											*/
	/*		Correction amount is motor maximum speed2times the motor maximum speed,2times the pulse		*/
	/*----------------------------------------------------------------------------------------------*/
	BlshCmp->conf.CompPulse = MlibLimitul(lwk, BlshCmp->conf.MaxCompPulse, -BlshCmp->conf.MaxCompPulse);
	return;
}
#endif


#if (CSW_SVCF_BACKLASHCOMP == TRUE)
/****************************************************************************************************/
/*																									*/
/*		Backlash correction Correction time constant (Pn233) Parameter calculation											*/
/*																									*/
/****************************************************************************************************/
void	BlshCalcCompensateTime( BLSHCMP *BlshCmp, LONG SvCycleUs, USHORT blfiltime )
{
	BlshCmp->conf.KfilTmp = MlibScalKxgain( SvCycleUs, 1, (SvCycleUs + (blfiltime * 10)), NULL, 24 );
	return;
}
#endif


#if (CSW_SVCF_BACKLASHCOMP == TRUE)
/****************************************************************************************************/
/*																									*/
/*		Backlash correction Correction status invalid processing															*/
/*																									*/
/****************************************************************************************************/
void	BlshInvaildCompensate( BLSHCMP *BlshCmp )
{
	LONG work;							/* <S1D8> */

	/* Correction status invalid processing:APOS=FBSet to position */
	BlshCmp->var.BlshCmpAmountPls = -BlshCmp->var.LastFilo;
	/* Correction filter gain parameter valid */
	BlshCmp->conf.Kfil = BlshCmp->conf.KfilTmp;
	/* Backlash correction variable initialization */
	work = BlshCmp->var.BlshCmpAmountPls;							/* <S1D8> */
	MlibResetLongMemory( &BlshCmp->var, sizeof(BlshCmp->var)/4 );			
	BlshCmp->var.BlshCmpAmountPls = work;			/* Backlash correction amount difference value is not applicable	*/	/* <S1D8> */
	return;
}
#endif


#if (CSW_SVCF_BACKLASHCOMP == TRUE)
/****************************************************************************************************/
/*																									*/
/*		Backlash correction Correction amount initialization processing															*/
/*																									*/
/****************************************************************************************************/
void	BlshInitializeCompensate( BLSHCMP *BlshCmp )
{
	/* Correction filter gain parameter valid */
	BlshCmp->conf.Kfil = BlshCmp->conf.KfilTmp;
	/* Backlash correction variable initialization */
	MlibResetLongMemory( &BlshCmp->var, sizeof(BlshCmp->var)/4 );
	return;
}
#endif



/***************************************** end of file **********************************************/

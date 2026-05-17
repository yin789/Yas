/****************************************************************************/
/*																			*/
/*																			*/
/*		Motion Generator 													*/
/*																			*/
/*																			*/
/****************************************************************************/
/*																			*/
/* MtgInit()			:Motion GeneratorInitialize								*/
/* MtgAttachPRM()		:MotionRegister parameter data							*/
/* MtgGetDctDistance()	:Deceleration distance acquisition										*/
/* MtgSetCspd()			:Set current command speed(For speed/torque control)				*/
/*																			*/
/* MtgNone()			:Motion neutral(NOP)Processing					*/
/* MtgFollowup()		:Follow-up motion							*/
/* MtgVelctrl()			:Speed/torque control motion							*/
/* MtgInterpolate()		:Complementary feed motion									*/
/* MtgPosing()			:Positioning motion									*/
/* MtgHold()			:Deceleration and stop motion									*/
/*																			*/
/*																			*/
/************** Copyright Yaskawa Electric Corporation **********************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/

/* includes */
#include "Motion.h"
//#include "Basedef.h"
//#include "ServoIf.h"
//#include "Motion.h"
//#include "MtPRM.h"
#include "MtGenerator.h"
//#include "MlibSvc.h"
//#include "UtilityLib.h" /* <V715> */

/* defines */
#define VELREM_BIT_MSK	((1<<MG_VREMBIT_MAX)-1)
#define VELREM_CARRYUP	(1<<MG_VREMBIT_MAX)


typedef struct MG_POSWK_ {
	LONG	err_dist;	/* Remaining distance (distance from command position to target position) ****.[ref] */
	ULONG	err_dist0;	/* Remaining distance (decimal part) (distance from command position to target position) .**[ref] */
	ULONG	sd_dist0;	/* Deceleration distance (decimal part) (Deceleration distance from current command speed to stop) .**[ref] */
	LONG	sd_dist;	/* Deceleration distance (integer part) (Deceleration distance from current command speed to stop) ****[ref] */
	LONG	sd_rem;		/* Deceleration fraction (a fraction paid out due to the difference between the deceleration distance and the remaining distance) ***.*[ref/cyc] */
	LONG	spdrem1;	/* Speed fraction payout during 1st stage deceleration ***.*[ref/cyc] */
	LONG	spdrem2;	/* Distribute speed fraction during 2nd stage deceleration ***.*[ref/cyc] */
	LONG	spd;		/* Current command speed ***.*[ref/cyc] */
	LONG	tspd;		/* Target speed ***.*[ref/cyc] */
	LONG	lspd;		/* Previous command speed ***.*[ref/cyc] */
	LONG	bspd;		/* Command output speed(dpos) ***.*[ref/cyc] */
} MG_POSWK;


/* forward declarations */
/* Public functions */
VOID MtgInit(MGHNDL* hdl, MC_BASE_PRM* bPrm, MG_PRM* prm);
VOID MtgAttachPRM(MGHNDL* hdl, MG_PRM* prm);
BOOL MtgGenerate(MGHNDL* hdl, MG_INOUT* inOut);
LONG MtgGetDctDistance(MGHNDL* hdl, LONG spd, USHORT ctrl);
LONG MtgSetCspd(MGHNDL* hdl, LONG spd);
LONG MtgAccelerateSpd(LONG spd, LONG tspd, MGHNDL* hdl);

/* Public Motion functions */
BOOL MtgNone(MGHNDL* hdl, MG_INOUT* inOut);
BOOL MtgFollowup(MGHNDL* hdl, MG_INOUT* inOut);
BOOL MtgVelctrl(MGHNDL* hdl, MG_INOUT* inOut);
BOOL MtgInterpolate(MGHNDL* hdl, MG_INOUT* inOut);
BOOL MtgPosing(MGHNDL* hdl, MG_INOUT* inOut);
BOOL MtgHold(MGHNDL* hdl, MG_INOUT* inOut);

/* Private sub functions */
LONG mtgCheckDecelaration(MG_POSWK*	wk, MG_PRM* prm, MC_BASE_PRM* bPrm);
LONG mtgAccelerateDecRate(LONG lspd, LONG Tspd, MG_PRM* prm);
LONG mtgAccelerateMinRate(LONG lspd, LONG Tspd, MG_PRM* prm);
VOID mtgDecreaseSpd(MG_POSWK* wk, MG_PRM* prm, MC_BASE_PRM* bPrm);
LONG mtgAccelerateSpd(LONG spd, LONG tspd, MG_PRM* prm);
BOOL mtgCalcStpDistance(MG_POSWK* wk, MG_PRM* prm, MC_BASE_PRM* bPrm);


/* motion function table */
typedef BOOL (*MTGFUNC)(MGHNDL* hdl, MG_INOUT* inOut);
//const MTGFUNC _mtgOutputMotion[] = {	/* <V715> */
#if 0
const MTGFUNC mtgOutputMotion[] = {	/* <V715> */
	MtgNone,		/* NOPMotion */
	MtgInterpolate,	/* INTERPOLATE */
	MtgPosing,		/* POSING */ 
	MtgHold,		/* HOLD */
	MtgFollowup,	/* FOLLOW UP */
	MtgVelctrl,		/* VELCTRL */
	MtgVelctrl,		/* TRQCTRL */
};
#endif

/* variable <V715> */
#ifdef	__ghs__
#pragma ghs startzda
#endif
//MTGFUNC mtgOutputMotion[sizeof(_mtgOutputMotion)/sizeof(MTGFUNC)];
#ifdef	__ghs__
#pragma ghs endzda
#endif

/****************************************************************************
 Initialization process
****************************************************************************/
#pragma arm section code = "IXX_EXCODE"
VOID MtgInit(MGHNDL* hdl, MC_BASE_PRM* bPrm, MG_PRM* prm)
{
	hdl->cMinusDir = 0;	/* Current dispensing direction */
	hdl->tMinusDir = 0;	/* Direction from the current command position to the target position */
	hdl->dpos0 = 0;		/* DPOS for POSING */
	hdl->dposRem = 0;	/* Transmission cycle movement amount(dpos)Remainder         */
	hdl->Cspd = 0;		/* Current command speed */
	hdl->basePrm = bPrm;/* Basic parameter data reference registration */
	hdl->prm = prm;		/* Register parameter data reference destination */

	/* ConstData tableRAM *//* <V715> */
	//UtilMemCpy((MTGFUNC *)mtgOutputMotion, (MTGFUNC *)_mtgOutputMotion, sizeof(mtgOutputMotion));
	//MlibCopyByteMemory ( (UCHAR *)_mtgOutputMotion, (UCHAR *)mtgOutputMotion, sizeof(mtgOutputMotion) );
}
#pragma arm section code

/****************************************************************************
 Send parameter dataMGparameters
****************************************************************************/
VOID MtgAttachPRM(MGHNDL* hdl, MG_PRM* prm)
{
	hdl->prm = prm;		/* Register parameter data reference destination */
}



/****************************************************************************
 Motion generation main processing
****************************************************************************/
BOOL MtgGenerate(MGHNDL* hdl, MG_INOUT* inOut)
{
	BOOL out_cmp;

	//out_cmp = mtgOutputMotion[inOut->Mtype](hdl, inOut);
	switch( inOut->Mtype )
	{
	case 0: out_cmp = MtgNone( hdl, inOut ); break;
	case 1: out_cmp = MtgInterpolate( hdl, inOut ); break;
	case 2: out_cmp = MtgPosing( hdl, inOut ); break;
	case 3: out_cmp = MtgHold( hdl, inOut ); break;
	case 4: out_cmp = MtgFollowup( hdl, inOut ); break;
	case 5: out_cmp = MtgVelctrl( hdl, inOut ); break;
	case 6: out_cmp = MtgVelctrl( hdl, inOut ); break;
	default:
		out_cmp = FALSE; break;
	}


	return out_cmp;
}


/****************************************************************************
 Deceleration distance acquisition
****************************************************************************/
LONG MtgGetDctDistance(MGHNDL* hdl, LONG spd, USHORT ctrl)
{
	MG_POSWK	wk;
	LONG		dist;
	MG_PRM*		prm = hdl->prm;

	if(ctrl == MG_HOLD_CTRL2)
		prm = &hdl->basePrm->StopPrm;

	wk.spd = MlibABS(spd);
	mtgCalcStpDistance(&wk, prm, hdl->basePrm);	/* Deceleration distance calculation */

	dist = wk.sd_dist;                                  /* buf = sd_dist (Integer part) */
	if(((wk.sd_dist | wk.sd_dist0) != 0) && ((wk.sd_dist0 | hdl->dpos0) != 0))
		dist = dist + 1;	/* Deceleration distance round up the decimal part */

	if(hdl->cMinusDir) return (-dist); 
	return dist;
}

/****************************************************************************
 Torque/During speed control Set current command speed
****************************************************************************/
LONG MtgSetCspd(MGHNDL* hdl, LONG spd)
{
	LONG maxSpd = hdl->basePrm->vel_max >> 1;

	hdl->cMinusDir = (spd < 0) ? TRUE: FALSE;
	spd = MlibABS(spd);
	if(spd > maxSpd) spd = maxSpd;
	hdl->Cspd = spd << hdl->basePrm->vel_shift;

	return hdl->Cspd;
}


/****************************************************************************
 NOPMotion processing
****************************************************************************/
BOOL MtgNone(MGHNDL* hdl, MG_INOUT* inOut)
{
	hdl->cMinusDir = 0;
	hdl->dpos0 = 0;
	hdl->dposRem = 0;
	hdl->Cspd = 0;
	inOut->DposOut = 0;
	inOut->VrefOut = 0;
	inOut->TrefOut = 0;

	return TRUE;
}

/****************************************************************************
 Follow up Motion processing
****************************************************************************/
BOOL MtgFollowup(MGHNDL* hdl, MG_INOUT* inOut)
{
	hdl->cMinusDir = 0;
	hdl->dpos0 = 0;
	hdl->dposRem = 0;
	hdl->Cspd = 0;
	inOut->DposOut = inOut->DposIn;
	inOut->VrefOut = 0;
	inOut->TrefOut = 0;

	if(MlibABS(inOut->DposIn) >= FOLLOWUP_MAX_REF)
		return FALSE;

	return TRUE;
}

/****************************************************************************
 Speed/torque control Motion processing
****************************************************************************/
BOOL MtgVelctrl(MGHNDL* hdl, MG_INOUT* inOut)
{
	hdl->dpos0 = 0;
	hdl->dposRem = 0;
	inOut->DposOut = 0;
	inOut->VrefOut = inOut->Vref /* TODO: MlibMulgain(inOut->Vref, hdl->basePrm->maxToOsKx) */;
	inOut->TrefOut = inOut->Tref /* >> 6 */;
	if(hdl->Cspd != 0) return FALSE;
	return TRUE;
}


/****************************************************************************
 INTERPOLATEMotion processing
****************************************************************************/
BOOL MtgInterpolate(MGHNDL* hdl, MG_INOUT* inOut)
{
	LONG dpos, cSpd;
	BOOL comp = FALSE;

/* Payout completion check */
	if(inOut->DposIn == 0)
	{
		hdl->dposRem = 0;
		comp = TRUE;
	}
/* Payout amount creation */
	dpos = inOut->DposIn + hdl->dposRem;
	inOut->DposOut = dpos / hdl->basePrm->seg_num;
	hdl->dposRem = dpos % hdl->basePrm->seg_num;
/* POSINGSpeed setting */
	cSpd = MtgSetCspd(hdl, inOut->DposOut);
	hdl->dpos0 = 0;

	//inOut->Tspd = MlibMulgain32(cSpd, hdl->basePrm->PosingSpdInvKx);		// Interim
	inOut->Tspd = cSpd;														// Interim
	inOut->VrefOut = inOut->Vref /* @@ MlibMulgain(inOut->Vref, hdl->basePrm->refToOsKx) */;
	inOut->TrefOut = inOut->Tref /* @@ >> 6 */;

	return comp;
}

/****************************************************************************
 PosingMotion processing
****************************************************************************/
BOOL MtgPosing(MGHNDL* hdl, MG_INOUT* inOut)
{
	MG_POSWK	wk;
	LONG		buf;
	BOOL		rc;
	BOOL		check = TRUE;

	hdl->dposRem = 0;
	inOut->VrefOut = 0;
	inOut->TrefOut = 0;
/* POSINGInitial setting */
	//wk.tspd = MlibMulgain32(inOut->Tspd, hdl->basePrm->PosingSpdKx);		// Interim
	wk.tspd = inOut->Tspd;													// Interim
	if(wk.tspd >= 0x40000000) {wk.tspd = 0x3FFFFFFF;}
	wk.spd =  hdl->Cspd;				/* Copy command speed***.*[ref/cyc] */
	wk.lspd = wk.spd;					/* Save command speed */
	wk.bspd = wk.spd;					/* Command speed(dpos) */
	wk.err_dist = inOut->DposIn;		/* Remaining distance calculation (with direction) */
	hdl->tMinusDir = FALSE;				/* +Direction */
	if(wk.err_dist < 0)
	{/* -Movement direction */
		hdl->tMinusDir = TRUE;			/* -Direction */
		wk.err_dist = -inOut->DposIn;	/* Remaining distance */
	}
/* POSINGStart/end check */
	if(wk.spd == 0)
	{/* Command speed=0then, POSINGReset to initial state */
		hdl->cMinusDir = hdl->tMinusDir;
		hdl->dpos0 = 0;
		inOut->DposOut = 0;
		if(wk.err_dist == 0)	return TRUE;	/* Payout completion */
	}
/* Previous surplus correction */
	if(hdl->dpos0 != 0) 
		wk.err_dist = wk.err_dist - 1;
	wk.err_dist0 = (0 - hdl->dpos0) & VELREM_BIT_MSK;

/* Deceleration distance calculation */
	rc = mtgCalcStpDistance(&wk, hdl->prm, hdl->basePrm);	/* Deceleration distance calculation */
	if((hdl->tMinusDir != hdl->cMinusDir) || (rc != 0))
	{/* Target position in opposite direction or Deceleration distance over → Deceleration */
		wk.sd_rem = 0;									/* Do not make unnecessary payments */
		mtgDecreaseSpd(&wk, hdl->prm, hdl->basePrm);	/* Deceleration processing */
		check = FALSE; 									/* POSINGState where termination is not possible */
	}
	else
	{
		buf = mtgCheckDecelaration(&wk, hdl->prm, hdl->basePrm);	/* buf = Remaining distance - Deceleration distance - Command speed */
		if(buf < 0)
		{/* When remaining distance is less than deceleration distance, decelerate */
			wk.sd_rem = buf + wk.spd;							/* Calculate deceleration fraction */
			mtgDecreaseSpd(&wk, hdl->prm, hdl->basePrm);		/* Deceleration processing */
			if(wk.sd_rem < 0)	check = FALSE;					/* Target position is nearer in the same direction→Pullback */
		}
		else
		{/* When remaining distance is greater than deceleration distance */
			if(wk.spd < wk.tspd)
			{/* Current speed < Target speed not reached */
				wk.spd = mtgAccelerateSpd(wk.spd, wk.tspd, hdl->prm);	/* Acceleration processing */

				rc = mtgCalcStpDistance(&wk, hdl->prm, hdl->basePrm);	/* Deceleration distance calculation */
				buf = mtgCheckDecelaration(&wk, hdl->prm, hdl->basePrm);/* buf = Remaining distance - Deceleration distance - Command speed */
				if((buf < 0) || (rc != 0))
				{/* Cannot accelerate with acceleration constant */
					wk.spd = mtgAccelerateDecRate(wk.lspd, wk.tspd, hdl->prm);	/* Acceleration with deceleration constant */
					rc = mtgCalcStpDistance(&wk, hdl->prm, hdl->basePrm);		/* Deceleration distance calculation */
					buf = mtgCheckDecelaration(&wk, hdl->prm, hdl->basePrm);	/* buf = Remaining distance - Deceleration distance - Command speed */
					if((buf < 0) || (rc != 0))
					{/* Cannot accelerate */
						if(wk.lspd != 0)
						{/* Was the previous speed 0? */
							wk.bspd = wk.spd = wk.lspd;	/* Set to previous speed */
						}
						else
						{/* Previous speed=0, send several pulses */
							wk.spd = mtgAccelerateMinRate(wk.lspd, wk.tspd, hdl->prm);	/* Accelerate with minimum constant */
							buf = wk.err_dist << hdl->basePrm->vel_shift;
							if(wk.spd >= buf)	wk.bspd = wk.spd = buf;	/* Convert remaining distance to speed */
							else				wk.bspd = wk.spd;		/* Accelerate with minimum acceleration */
						}
					}
					else
					{
						wk.bspd = wk.spd;
						if( wk.spd == wk.lspd )
						{/* After all,Cannot accelerate */
							wk.sd_rem = (buf << hdl->basePrm->vel_shift);	/* bufteeth****.[ref] */
							if(wk.sd_rem > wk.lspd)  wk.bspd = wk.sd_rem;	/* Pay out the deceleration fraction. */
						}
					}
				}
				else
				{
					wk.bspd = wk.spd;
				}
			}
			else if(wk.spd > wk.tspd)
			{/* Current speed > Target speed exceeded (Target speed changed)*/
				wk.sd_rem = 0;
				mtgDecreaseSpd(&wk, hdl->prm, hdl->basePrm);	/* Deceleration processing */
				if(wk.spd < wk.tspd) wk.spd = wk.tspd;			/* Clamp to target speed */
				wk.bspd = wk.spd;								/* for dpos */
			}
			else
			{/* Current speed = Target speed has been reached */
				wk.bspd = wk.spd;		/* for dpos */
			}
		}
	}
	hdl->Cspd = wk.spd;		/* Command speed */
	buf = hdl->dpos0 + ((wk.bspd << hdl->basePrm->vel_shift_s) & VELREM_BIT_MSK);	/* Integrate decimal part */
	hdl->dpos0 = buf & VELREM_BIT_MSK;
	if(buf > VELREM_BIT_MSK){ buf = (wk.bspd >> hdl->basePrm->vel_shift) + 1;}
	else					{ buf = (wk.bspd >> hdl->basePrm->vel_shift);}
	if(hdl->cMinusDir)	{ inOut->DposOut = (~buf) + 1;}	/* Sign extension */
	else				{ inOut->DposOut = buf;}

/* Termination processing */
	if((inOut->DposOut == inOut->DposIn) && (check == TRUE))
	{
		hdl->Cspd = 0;		/* Command speed */
		return TRUE;
	}

	return FALSE;
}

/****************************************************************************
 HoldMotion processing
****************************************************************************/
BOOL MtgHold(MGHNDL* hdl, MG_INOUT* inOut)
{
	MG_PRM*	prm = hdl->prm;
	BOOL	rc;

	if(inOut->Ctrl == MG_HOLD_CTRL2)
		hdl->prm = &hdl->basePrm->StopPrm;

	rc = MtgPosing(hdl, inOut);

	hdl->prm = prm;

	return rc;
}


/****************************************************************************
 Data calculation processing for deceleration determination
  Overview:  Calculate remaining distance - deceleration distance - command speed.
         When the value is negative, calculate up to the decimal part.
         When it becomes a plus, leave it as is (no need for decimal places in deceleration check)
         If deceleration distance is huge, use minimum value(-2147483648)only for masking regeneration abnormalities in medium-capacity servo packs that do not have a built-in regenerative resistor.
         -2147483648,Remaining distance-2147483648At the time of
         Forcibly214748367.
****************************************************************************/
LONG mtgCheckDecelaration(MG_POSWK*	wk, MG_PRM* prm, MC_BASE_PRM* bPrm)
{
	LONG result;
	LONG buf0;

	result = wk->err_dist - wk->sd_dist - (wk->spd >> bPrm->vel_shift);
	if( result > -bPrm->vel_max/*-1048576*/)
	{/* Deceleration distance is huge(11Check with minimum data at bit shift) */
		if(result <= 1)
		{/* err_distIf it is huge, there is no need to slow down,1, subtracting the decimal part does not result in a negative value. */
			buf0 = (LONG)(wk->err_dist0 - wk->sd_dist0) 
					- ((wk->spd << bPrm->vel_shift_s) & VELREM_BIT_MSK);  /* Decimal part */
			result = (result << bPrm->vel_shift) + (buf0 >> bPrm->vel_shift_s); /* buf(***.*) = err_dist - sd_dist - spd */
			if(result > 0)
				result = result >> bPrm->vel_shift;  /* Result plus means no need for decimal places */
		}
	}
	else
	{
		if(wk->err_dist == 0x80000000/*-2147483648*/)
			result = 0x7fffffff/*2147483647*/;    /* this is-214748368 */
		else
			result = 0x80000000/*-2147483648*/;
	}
	return result;
}

/****************************************************************************
  Acceleration/deceleration judgment based on deceleration
  Overview:  Determine whether it can accelerate from the current speed with the deceleration constant and output the speed command.
         <Conditions for acceleration>
         If deceleration constant < acceleration constant, it seems possible to accelerate.
         If lower than target speed, OK (Target speed has been checked with acceleration)
         OK if less than acceleration change rate (acceleration change rate has been checked with acceleration)
****************************************************************************/
LONG mtgAccelerateDecRate(LONG lspd, LONG Tspd, MG_PRM* prm)
{
	LONG spd;
	LONG rate;
	
 /* Select acceleration or deceleration */
 	rate = ((lspd >= prm->altspd_d) ? prm->decrate2: prm->decrate1);
	if(lspd >= prm->altspd_a )
	{/* For 2nd stage */
		if(rate >= prm->accrate2) rate = 0;        /* If deceleration > = acceleration, use the previous speed */
		spd = lspd + rate;
	}
	else
	{/* At 1st stage */
		if(rate >= prm->accrate1)	rate = 0;		/* If deceleration > = acceleration, use the previous speed */
		spd = lspd + rate;
		if(spd >= prm->altspd_a)	spd = lspd;	/* If the result of acceleration is equal to or higher than the acceleration change speed, set the previous speed */
	}
	if(spd >= Tspd) spd = lspd;           /* If the result of acceleration is equal to or higher than the target speed, set the previous speed */

	return spd;
}

/****************************************************************************
  Comparison selection of acceleration and deceleration
  Overview:  Comparing acceleration and deceleration,The smaller one accelerates.
         If target speed or higher, target speed
         Acceleration change rate if acceleration change rate
****************************************************************************/
LONG mtgAccelerateMinRate(LONG lspd, LONG Tspd, MG_PRM* prm)
{
	LONG spd;
	LONG rate;
	
 /* Select acceleration or deceleration */
	rate = ((lspd >= prm->altspd_d) ? prm->decrate2: prm->decrate1);
	if(lspd >= prm->altspd_a )
	{/* For 2nd stage */
		if(rate >= prm->accrate2) rate = prm->accrate2;        /* If deceleration > = acceleration, use acceleration */
		spd = lspd + rate;
	}
	else
	{/* At 1st stage */
		if(rate >= prm->accrate1) rate = prm->accrate1;   /* If deceleration > = acceleration, use acceleration */
		spd = lspd + rate;
		if(spd >= prm->altspd_a)  spd = prm->altspd_a;      /* If the result of acceleration is equal to or higher than the acceleration change speed, set the acceleration change speed */
	}
	if(spd >= Tspd) spd = Tspd;           /* If the result of acceleration is equal to or higher than the target speed, set it to the target speed */

	return spd;
}

/****************************************************************************
  Deceleration processing
  Calculate the command speed that is decelerated from the current command speed.
  Clears after paying out the fraction when decelerating.            
****************************************************************************/
VOID mtgDecreaseSpd(MG_POSWK* wk, MG_PRM* prm, MC_BASE_PRM* bPrm)
{
	LONG buf;          /* Work */
	LONG lspd;
	
	lspd = wk->spd;
	if( wk->spd > prm->altspd_d )
	{/* 2nd stage deceleration processing */
		buf = ((wk->spdrem2 != 0) ? wk->spdrem2: prm->decrate2);	/*If there is a speed fraction, decelerate with the speed fraction. */
		wk->spd -= buf;

		if(wk->spd < prm->altspd_d)   wk->spd = prm->altspd_d;	/* When deceleration is less than the change speed */
		
		if(wk->sd_rem > 0)
		{ 
			if(wk->spd <= wk->sd_rem)
			{/* Can you pay out fractions when decelerating? */
				wk->bspd = wk->sd_rem;	/* Pay out a fraction when decelerating. */
				wk->spd = lspd;
				wk->sd_rem = 0;
			}
			else
			{
				LONG mask = (1 << bPrm->vel_shift) - 1;
				wk->bspd = wk->spd + (wk->sd_rem & mask);	/* Forcibly pays out the decimal part of the fraction when decelerating. */
			}
		}
		else
		{/* Since the fraction is not paid out when decelerating, the second stage speed fraction will be paid out. */
			wk->bspd = wk->spd;
			wk->spdrem2 = 0;	/* The speed fraction is only paid out at the beginning of deceleration, and is not thereafter. */
		}
	}
	else
	{/* 1st stage deceleration processing */
		buf = ((wk->spdrem1 != 0) ? wk->spdrem1: prm->decrate1);	/*If there is a speed fraction, decelerate with the speed fraction. */
		wk->spd -= buf;

		if(wk->spd < 0)	wk->spd = 0;

		if(wk->sd_rem > 0)
		{ 
			if(wk->spd <= wk->sd_rem)
			{/* Can you pay out fractions when decelerating? */
				wk->bspd = wk->sd_rem;	/* Pay out a fraction when decelerating. */
				wk->spd = lspd;
				wk->sd_rem = 0;
			}
			else
			{
				LONG mask = (1 << bPrm->vel_shift) - 1;
				wk->bspd = wk->spd + (wk->sd_rem & mask);	/* Forcibly pays out the decimal part of the fraction when decelerating. */
			}
		}
		else
		{/* Since the fraction is not paid out when decelerating, the second stage speed fraction will be paid out. */
			wk->bspd = wk->spd;
			wk->spdrem1 = 0;	/* The speed fraction is only paid out at the beginning of deceleration, and is not thereafter. */
		}
	}
}


/****************************************************************************
  Acceleration processing
****************************************************************************/
LONG mtgAccelerateSpd(LONG spd, LONG tspd, MG_PRM* prm)
{
	if( spd < prm->altspd_a )
	{   /* Try 1st stage acceleration processing */
		spd += prm->accrate1;
		if(spd > prm->altspd_a)
			spd = prm->altspd_a;
	}
	else
	{/* Try 2nd stage acceleration processing */
		spd += prm->accrate2;
	}

	if(spd > tspd) spd = tspd;

	return spd;
}


/****************************************************************************
  Deceleration distance calculation
****************************************************************************/
BOOL mtgCalcStpDistance(MG_POSWK* wk, MG_PRM* prm, MC_BASE_PRM* bPrm)
{
	ULONG cnt;       /* Deceleration number */
	LONG work1[2] = {0};            /* For computation result storageNo-Kude-Ta */
	//LONG work2[2] = {0};    /* For computation result storageNo-Kude-Ta */
	LONG work3 /*,spd */;               /* Work data for storing calculation results */
	
#ifndef POSING_1RATEONLY
	if(prm->altspd_d != 0)
	{/* 1,2Stage valid */
		spd = wk->spd;
		if( wk->spd > prm->altspd_d )
		{/* Is the command speed greater than the deceleration change speed? */
		/* 2nd stage deceleration distance calculation ------------------------------------ */
			cnt = ( wk->spd - prm->altspd_d ) / prm->decrate2;					/* Deceleration number calculation */
			wk->spdrem2 = wk->spd - ( prm->decrate2 * cnt ) - prm->altspd_d;	/* Deceleration 2 fraction */
			if(wk->spdrem2 != 0)   cnt = cnt + 1;
			MlibMul3232( prm->altspd_d*2, cnt , work1 );			/* work1 = altspd * 2 * cnt */
			work3  = prm->decrate2*(cnt-1);
			MlibMul3232( work3, cnt , work2 );						/* work2 = rate * ((cnt*(cnt-1))) */
			MlibAdd6464( work1, work2, work2);						/* work2 = work2 + work1 */
			spd = prm->altspd_d;							/* For calculation of 1st stage part */
		}
		/* 1st stage deceleration distance calculation ------------------------------------ */
		cnt = spd / prm->decrate1;						/* Deceleration number calculation */
		wk->spdrem1 = spd - ( prm->decrate1 * cnt );	/* Deceleration 1 fraction */
		if(wk->spdrem1 != 0)   cnt = cnt + 1;
		work3 = prm->decrate1*(cnt-1);
		MlibMul3232( work3, cnt , work1 );		/* work1 = rate * ((cnt*(cnt+1))) */

		MlibAdd6464( work1, work2, work2);		/* work2 = work2 + work1 2nd stage + 1st stage deceleration distance section result */
		wk->sd_dist  = ((work2[1] << (31-bPrm->vel_shift))		/* Deceleration distance(Integer part)= (2stage+1Stage deceleration distance section result)/2 */
					| ((work2[0] >> (bPrm->vel_shift+1)) & (bPrm->vel_max-1)));
		wk->sd_dist0 = (work2[0] << (bPrm->vel_shift_s-1)) & VELREM_BIT_MSK;/* Deceleration distance(Decimal part) */

		work3 = work2[1] >> (bPrm->vel_shift-1);	/* sd_dist>0x40000000 */
		if(work3 != 0)	return TRUE;		/* Deceleration distance overflow */
	}
	else
#endif
	{/* 2stage only enabled */
		cnt = wk->spd / prm->decrate2;						/* Deceleration number calculation */
		wk->spdrem2 = wk->spd - ( prm->decrate2 * cnt );	/* Deceleration 1 fraction */
		if(wk->spdrem2 != 0)   cnt = cnt + 1;
		work3 = prm->decrate2*(cnt-1);
		MlibMul3232( work3, cnt , work1 );		/* work1 = rate * ((cnt*(cnt+1))) */

		wk->sd_dist  = ((work1[1] << (31-bPrm->vel_shift))  		/* Deceleration distance(Integer part)= (1Stage deceleration distance section result)/2 */
					| ((work1[0] >> (bPrm->vel_shift+1)) & (bPrm->vel_max-1)));
		wk->sd_dist0 = (work1[0] << (bPrm->vel_shift_s-1)) & VELREM_BIT_MSK;/* Deceleration distance(Decimal part)= (1Stage deceleration distance section result)/2 */

		work3 = work1[1] >> (bPrm->vel_shift-1);	/* sd_dist>0x40000000 */
		if(work3 != 0)	return TRUE;		/* Deceleration distance overflow */
	}
	return FALSE;
}
/****************************************************************************
  Acceleration processing
****************************************************************************/
LONG MtgAccelerateSpd(LONG spd, LONG tspd, MGHNDL* hdl)
{
LONG speed;

	speed = mtgAccelerateSpd(spd, tspd, hdl->prm);			/* Acceleration processing */

	return speed;
}

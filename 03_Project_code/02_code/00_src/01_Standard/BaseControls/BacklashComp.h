#ifndef __BACKLASH_COMP_H /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		BacklashComp.h : Backlash correction function	module definition										*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2011.04.05	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "Bprm.h"
#include "MLib.h"

/****************************************************************************************************/
/*																									*/
/*		Structure Definition																		*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for backlash correction																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{								/* Constant & variable definition for backlash correction					*/
	struct {									/* Constant data definition for backlash correction					*/
		BOOL	BacklashDir;					/* Backlash correction direction selection setting						*/
		LONG	CompPulse;							/* Backlash correction amount			[Pulse]			*/
		LONG	Kfil;								/* Backlash filter gain					*/
		LONG	KfilTmp;							/* Backlash filter gain calculation				*/
		LONG	MaxCompPulse;						/* Backlash correction amount limit value		[Pulse]			*/
		ULONG	MaxCompRef;							/* Backlash correction amount limit value		[Command unit]		*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {									/* Variable data definition for backlash correction					*/
		LONG	BlshCmpAmountPls;					/* Backlash correction amount difference value						*/
		LONG	Filo;								/* Backlash correction filter output					*/
		LONG	LastFilo;							/* Previous backlash correction filter output				*/
		LONG	Cmppls;								/* Backlash correction amount			[Pulse]			*/
		LONG	Cmpgrrem;							/* Remaining backlash correction amount after electronic gear conversion			*/
		LONG	Cmppos;								/* Backlash correction position			[Pulse]			*/
		LONG	remx;								/* Backlash correction filter calculation remainder				*/
	} var;
} BLSHCMP;


/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
#define		BLSHCOMP_UPLIM		500000				/* Backlash correction amount maximum value						*/
#define		FOR_BACKLASH		0x00				/* Backlash correction for forward rotation						*/
#define		REV_BACKLASH		0x01				/* Backlash correction for reverse side						*/


/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/
void	BlshCompensateBacklash( BLSHCMP *BlshCmp, EGEAR *Egear, LONG *dPosRef, LONG *dPcmd, BOOL *RefZ, BOOL BlshActive );
void	BlshCalcMaxBlshCompPulse( BLSHCMP *BlshCmp, EGEAR *Egear, BPRMDAT *Bprm );
void	BlshCalcCompensateAmount( BLSHCMP *BlshCmp, EGEAR *Egear, LONG blcm2l );
void	BlshCalcCompensateTime( BLSHCMP *BlshCmp, LONG SvCycleUs, USHORT blfiltime );
void	BlshInvaildCompensate( BLSHCMP *BlshCmp );
void	BlshInitializeCompensate( BLSHCMP *BlshCmp );



/****************************************************************************************************/
#define __BACKLASH_COMP_H
#endif /* __BACKLASH_COMP_H */

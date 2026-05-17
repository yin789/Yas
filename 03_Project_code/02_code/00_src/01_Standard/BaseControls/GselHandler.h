#ifndef	GSEL_HANDLER_H
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		GselHandler.h : Gain switching handle definition												*/
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

/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Servo gain switching parameter variable definition														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct GSELGAINS {
	KSGAIN		Kp;									/* Position loop gain								*/
	KSGAIN		Kv;									/* Speed loop gain								*/
	KSGAIN		Kv2;								/* Speed loop gain 2							*/
	KSGAIN		Kv2Inv;								/* Speed loop gain 2 reciprocal						*/
	KSGAIN		Kvi;								/* Speed loop integral gain							*/
	KSGAIN		Klpf;								/* 1st-order lag torque filter gain					*/
	KSGAIN		KpInv;								/* Position loop gain reciprocal SvCycleunit(SGDVUnused)	*/
	KSGAIN		VirtualPosKp;						/* Gain for virtual position deviation calculation						*/
	KSGAIN		ZcKp;								/* Position loop gain for zero clamp				*/
} GSELGAINS;


#if 0 /* 2010.11.10 Y.Oka Commented out because unused */
typedef	struct MICROGAINS {
	long		Kp;									/* Position loop gain								*/
	long		Kv;									/* Speed loop gain								*/
	long		Kvi;								/* Speed loop integral gain							*/
	long		Klpf;								/* 1st-order lag torque filter gain					*/
} MICROGAINS;
#endif /* #if 0 */


#if 0		/* <S09B> */
typedef	struct GSELDOBS {
	KSGAIN		Ks;									/* Disturbance observerKsGain						*/
	KSGAIN		Ksj;								/* Disturbance observerKsjGain						*/
} GSELDOBS;
#else 		/* <S09B> */
typedef	struct GSELDOBS {					/* Parameter definitions for disturbance observer						*/
	KSGAIN				Ks;						/* Disturbance observer gain								*/
	KSGAIN				Kj;						/* Disturbance observer inertia gain					*/
	KSGAIN				Ksj;					/* Disturbance observer torque gain						*/
	KSGAIN				Kd;						/* Disturbance torque coefficient									*/
	KSGAIN				Klpf;					/* Disturbance observer low-pass filter gain				*/
} GSELDOBS;
#endif

#if 0	// Delete as unnecessary
typedef struct GSELSIG {						/* Automatic gain switching condition signal							*/
	BOOL		CoinOn;								/* COIN Ontoggle								*/
	BOOL		CoinOff;							/* COIN Offtoggle								*/
	BOOL		NearOn;								/* NEAR Ontoggle								*/
	BOOL		NearOff;							/* NEAR Offtoggle								*/
	BOOL		RefZOn;								/* REFZ Ontoggle								*/
	BOOL		RefZOff;							/* REFZ Offtoggle								*/
} GSELSIG;
#endif

#if 0	/* <S09B> */
typedef	struct ATGSEL {							/* Parameter & variable definition for automatic gain switching				*/
		struct	{								/* Parameters for automatic gain switching						*/
		LONG		Wait;							/* Gain switching waiting time							*/
/* 2010.11.10 Y.Oka Commented out because smoothing is not supported during automatic gain switching*/
//		LONG		AmondGain;						/* For analog monitor output ⊿Gain				*/
//		GSELGAINS	dGain;							/* ⊿Gain										*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct	{									/* Variable for automatic gain switching								*/
		LONG		Timer;							/* Gain switching waiting time timer					*/
/* 2010.11.10 Y.Oka Commented out because smoothing is not supported during automatic gain switching*/
//		GSELGAINS	*prevGain;						/* Previous gain									*/
//		GSELGAINS	*nextGain;						/* Next gain									*/
	} var;
} ATGSEL;
#endif


#if 0 /* 2010.11.10 Y.Oka Commented out because smoothing is not supported during automatic gain switching*/
typedef	struct ATGSELDOBS {						/* Parameters for automatic gain switching&Variable definition(Friction compensation only)*/
		struct	{								/* Parameters for automatic gain switching						*/
		GSELDOBS	dGain;							/* ⊿Gain										*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct	{									/* Variable for automatic gain switching								*/
		GSELDOBS	*prevGain;						/* Previous gain									*/
		GSELDOBS	*nextGain;						/* Next gain									*/
	} var;
} ATGSELDOBS;
#endif /* #if 0 */

/*--------------------------------------------------------------------------------------------------*/
/*		<S09B> Parameter variable definition for automatic gain switching													*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct ATGSEL {							/* Parameter & variable definition for automatic gain switching				*/
		struct	{								/* Parameters for automatic gain switching						*/
		LONG		Wait;							/* Gain switching waiting time							*/
		LONG		AmondGain;						/* For analog monitor output ⊿Gain				*/
		LONG		SwTime;							/* Gain switching time[ms]							*/
		LONG		SvCycleUs;						/* Control cycle[us]									*/
		LONG		GselTrqFilMin;					/* @@											*/
		GSELGAINS	dGain;							/* ⊿Gain										*/
		GSELDOBS	dGainDobs;						/* ⊿Gain(Disturbance observer)						*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct	{									/* Variable for automatic gain switching								*/
		LONG		Timer;							/* Gain switching waiting time timer					*/
		GSELGAINS	*prevGain;						/* Previous gain									*/
		GSELGAINS	*nextGain;						/* Next gain									*/
		GSELDOBS	*prevGainDobs;					/* Previous gain(Disturbance observer)					*/
		GSELDOBS	*nextGainDobs;					/* Next gain(Disturbance observer)					*/
	} var;
} ATGSEL;

typedef	struct GAIN_CHNG_HNDL {
//	GSELSIG			GselSig;			/* Automatic gain switching condition signal									*/
	ATGSEL			AtGselA;			/* Variable for automatic gain switching(TrgA)								*/
	ATGSEL			AtGselB;			/* Variable for automatic gain switching(TrgB)								*/
	GSELGAINS		GselGains[6];		/* Servo gain switching parameter variable							*/
	GSELDOBS		GselDobs[3];		/* Parameter variable for gain switching(Friction compensation only)				*/
	BOOL			AtGsel;				/* Automatic gain switching selection setting									*/
	LONG			AtGselTrgA;			/* Automatic gain switching condition A selection setting								*/
	LONG			AmonActGain;		/* AnaMon : Valid gain										*/
	LONG			ActGain;			/* Un014  : Valid gain										*/
} GAIN_CHNG_HNDL;



#define	GSEL_HANDLER_H
#endif /* GSEL_HANDLER_H */
/***************************************** end of file **********************************************/

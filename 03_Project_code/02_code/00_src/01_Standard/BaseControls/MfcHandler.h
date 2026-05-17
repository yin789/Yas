#ifndef	_MFC_HANDLER_H /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MfcHandler.h : Handle definition for model following control													*/
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
/*	Note	:	First edition	2009.12.22	Y.Oka	SGDV-M3 ⇒ SVN											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"


/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Constant definition for MFC calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct MFCPRM {							/* MFC parameter definition							*/
#if (FLOAT_USE==TRUE)
	float	Fsft;									/* DTR high-order filter resolution shift amount				*/
	float	InvFsft;								/* Reciprocal of DTR high-order filter resolution shift amount		*/
	float	Kf1[2];									/* Secondary filter gain 1							*/
	float	Kf2;									/* Secondary filter gain 2(For 1st differential filter)		*/
	float	Kf3;									/* Secondary filter gain 3(For 1st differential filter)		*/
	float	Kj1;									/* Mechanical coefficient gain 1								*/
	float	Kj2;									/* Mechanical coefficient gain 2								*/
	float	Kj2_sft;								/* Mechanical coefficient gain 2(ResolutionUP)			*/
	float	Kj3;									/* Mechanical coefficient gain 3								*/
	float	Kvff;									/* Speed feedforward gain					*/
	float	Ktff1;									/* Torque feedforward gain 1(For forward rotation)		*/
	float	Ktff2;									/* Torque feedforward gain 2(For reverse direction)		*/
	float	Cfric;									/* Coulomb friction compensation torque						*/
	float	Kfcf;									/* Coulomb friction compensation filter gain				*/
	float	Fccflag;								/* 2-mass && Full-Closed Control Flag			*/
	float	PSFccflag;								/* 2-mass && PS Type Full-Closed Control Flag	*/
	LONG	MFCModel;								/* Machine model									*/
	float	MFCType;								/* MFCProcessing type								*/
#else
	LONG	Fsft;									/* DTR high-order filter resolution shift amount				*/
	LONG	InvFsft;								/* Reciprocal of DTR high-order filter resolution shift amount		*/
	LONG	Kf1[2];									/* Secondary filter gain 1							*/
	LONG	Kf2;									/* Secondary filter gain 2(For 1st differential filter)		*/
	LONG	Kf3;									/* Secondary filter gain 3(For 1st differential filter)		*/
	LONG	Kj1;									/* Mechanical coefficient gain 1								*/
	LONG	Kj2;									/* Mechanical coefficient gain 2								*/
	LONG	Kj2_sft;								/* Mechanical coefficient gain 2(ResolutionUP)			*/
	LONG	Kj3;									/* Mechanical coefficient gain 3								*/
	LONG	Kvff;									/* Speed feedforward gain					*/
	LONG	Ktff1;									/* Torque feedforward gain 1(For forward rotation)		*/
	LONG	Ktff2;									/* Torque feedforward gain 2(For reverse direction)		*/
	LONG	Cfric;									/* Coulomb friction compensation torque						*/
	LONG	Kfcf;									/* Coulomb friction compensation filter gain				*/
	LONG	Fccflag;								/* 2-mass && Full-Closed Control Flag			*/
	LONG	PSFccflag;								/* 2-mass && PS Type Full-Closed Control Flag	*/
	LONG	MFCModel;								/* Machine model									*/
	LONG	MFCType;								/* MFCProcessing type								*/
#endif /* FLOAT_USE */
} MFCPRM;



/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for MFC calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct MFCTRL {							/* MFC parameter & variable definition						*/
	struct	{									/* MFC parameter definition							*/
		MFCPRM		MfcPrm[3];						/* MFC parameters							*/
		MFCPRM		MfcPexe;						/* Parameters for MFC for execution						*/
		LONG		MFCTypeSetting;					/* MFCProcessing type(Parameter setting value)				*/
		LONG		MFCModelSetting;				/* Machine model(Parameter setting value)					*/
		BOOL		MFCFF;							/* External FF in MFC(Speed, torque)Valid/invalid setting	*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {									/* MFC variable data definition							*/
		LONG		CurMFCModel;					/* Machine model(Current set value)						*/
		LONG		execnt;							/* Execute Count for MFC Execute Control		*/
		LONG		MfcPosErr;						/* Model position deviation(For calculation)	[pulse]				*/
		LONG		PosErr;							/* Model position deviation(For monitor)	[pulse]				*/
		LONG		Per64[2];						/* Model position deviation(For calculation)	[pulse]				*/
		LONG		Vref;							/* Model speed command			[2^24/OvrSpd]		*/
		LONG		Pfbk;							/* Model position FB			[pulse/Scan]		*/
	/*----------------------------------------------------------------------------------------------*/
		KSGAIN		Fwk1[2];						/* For secondary filter calculation							*//*<S00A>*/
		KSGAIN		Fwk2[2];						/* For secondary filter calculation							*//*<S00A>*/
		KSGAIN		Fwk3[2];						/* For secondary filter calculation							*//*<S00A>*/
		KSGAIN		Fwk4[2];						/* For secondary filter calculation							*//*<S00A>*/
		LONG		Filo;							/* Position command after first-order differential second-order filter				*/
		LONG		FCmdF;							/* ResolutionUPPosition command after secondary filter				*/
		LONG		dFilwk;							/* First-order differential second-order filter calculation					*/
		LONG		Fwkrem1;						/* First-order differential second-order filter calculation					*/
		LONG		Fwkrem2;						/* First-order differential second-order filter calculation					*/
	/*----------------------------------------------------------------------------------------------*/
		KSGAIN		PCmdsub;						/* Position command input									*//*<S00A>*/
		KSGAIN		FCmd[3];						/* Position command after high-order filter						*//*<S00A>*/
		LONG		dFCmd[2];						/* First-order differential value of position command after high-order filter				*/
		LONG		d2FCmd[2];						/* Second-order differential value of position command after high-order filter				*/
		LONG		d3FCmd;							/* Third-order differential value of position command after high-order filter				*/
		LONG		wkrem1;							/* For position command calculation								*/
		LONG		wkrem2;							/* For position command calculation								*/
		LONG		PosSum[2];						/* Position command addition integration value							*/
		LONG		TrqRef;							/* Model torque command(Monitoring possible)					*/
		LONG		Ktffcmdx;						/* Torque FF gain								*/
		LONG		FricCmpTrq;						/* Coulomb friction compensation torque						*/
		LONG		FCmdFilo[3];					/* Position command after high-order filter(ResolutionUPCalculation)			*/
	/*----------------------------------------------------------------------------------------------*/
		LONG		PffCmd;							/* Position FF command(Monitoring possible)						*/
		LONG		VffCmd;							/* Speed FF command 								*/
		LONG		TffCmd;							/* Torque FF command(Monitoring possible)						*/
	/*----------------------------------------------------------------------------------------------*/
		BOOL		MfcStopReq;						/* MFCStopping request(Fn)					*/
	} var;
} MFCTRL;



#define _MFC_HANDLER_H /* For reload prevention */
#endif /* _MFC_HANDLER_H */
/***************************************** end of file **********************************************/

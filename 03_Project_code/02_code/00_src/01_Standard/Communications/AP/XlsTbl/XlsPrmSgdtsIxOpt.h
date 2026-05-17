/****************************************************************************************************/
/*																									*/
/*																									*/
/*		XlsPrmSgdtsIxOpt.h : Parameter definition header file											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1. Parameter definition structure																		*/
/*																									*/
/*		2. Parameter setting data structure																*/
/*																									*/
/*		3. Standard parameter calculation function																	*/
/*																									*/
/*		4. Parameter definitionExternDeclaration																	*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		2013.01.11 : Made by Excel Macro															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#ifndef ___XLS_PRM_SGDTS___
#define  ___XLS_PRM_SGDTS___

#define FORMAT_ID			0x0808





/****************************************************************************************************/
/*																									*/
/*		Excel table common definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Display base																					*/
/*--------------------------------------------------------------------------------------------------*/
#define DSPDEF_BASENBL			0				/* Attribute: Nibble display									*/
#define DSPDEF_BASEDEC			1				/* Attribute:10Hex display									*/
#define DSPDEF_BASEHEX			2				/* Attribute:16Hex display									*/
#define DSPDEF_BASEBIT			3				/* Attribute: Bit display									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Signed or not																				*/
/*--------------------------------------------------------------------------------------------------*/
#define DSPDEF_W_SIGN			TRUE			/* Signed											*/
#define DSPDEF_NOSIGN			FALSE			/* Unsigned											*/
/*--------------------------------------------------------------------------------------------------*/
/*		Access level(Caution:0 cannot be used)															*/
/*--------------------------------------------------------------------------------------------------*/
#define ACCLVL_USER1			1				/* User 1										*/
#define ACCLVL_USER2			2				/* User 2										*/
#define ACCLVL_USER3			3				/* User 3										*//* <S0FE> */
//#define ACCLVL_SYSTEM			3				/* System											*//* <S0FE> */
#define ACCLVL_SYSTEM			4				/* System											*//* <S0FE> */
/*--------------------------------------------------------------------------------------------------*/


/****************************************************************************************************/
/*																									*/
/*		Parameter setting data structure definition																*/
/*																									*/
/****************************************************************************************************/
typedef struct
{
//	LONG	OTStop;				/* PnB1F : How to stop overtravel										*/
/*--------------------------------------------------------------------------------------------------*/
//	LONG	LinerCircle;		/* PnB20 : Movement method													*/
//	LONG	PosLimP;			/* PnB21 : Linear type(PnB20=0) : Soft limit forward rotation side(P-LS) Rotation type(PnB20≠0) : Ending point of rotating coordinates	*/
//	LONG	PosLimN;			/* PnB23 : Linear type(PnB20=0) : Soft limit reverse rotation side (N-LS) Rotation type(PnB20≠0) : Starting point of rotating coordinates	*/
//	LONG	HomePos;			/* PnB25 : Origin position (For incremental encoder) Absolute encoder offset (For absolute encoder)	*/
//	LONG	PosSpd;				/* PnB27 : Positioning/Registration speed									*/
	LONG	AccRate;			/* PnB29 : Acceleration													*/
	LONG	DecRate;			/* PnB2B : Deceleration													*/
	LONG	InPosWidth;			/* PnB2D : /INPOSITIONWidth											*/
	LONG	NearWidth;			/* PnB2F : NEARWidth													*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	HomeMethod;			/* PnB31 : Home return method												*/
	USHORT	bHomeDir;			/* PnB32 : Home return direction												*/
	LONG	HomeSpd;			/* PnB33 : Home return travel speed											*/
	LONG	ApproachSpd;		/* PnB35 : Return to origin approach speed										*/
	LONG	CreepSpd;			/* PnB37 : Return to origin creep speed										*/
	LONG	FinalDst;			/* PnB39 : Home return final travel distance										*/
//	LONG	SetupInfo0;			/* PnB3B : Reservation parameter(Setup information0)									*/
//	LONG	SetupInfo1;			/* PnB3D : Reservation parameter(Setup information1)									*/
//	LONG	SetupInfo2;			/* PnB3F : Reservation parameter(Setup information2)									*/
/*--------------------------------------------------------------------------------------------------*/
//	LONG	SetupInfo3;			/* PnB41 : Reservation parameter(Setup information3)									*/
//	LONG	SetupInfo4;			/* PnB43 : Reservation parameter(Setup information4)									*/
//	LONG	SetupInfo5;			/* PnB45 : Reservation parameter(Setup information5)									*/
//	LONG	SetupInfo6;			/* PnB47 : Reservation parameter(Setup information6)									*/
//	LONG	SetupInfo7;			/* PnB49 : Reservation parameter(Setup information7)									*/
//	LONG	SetupInfo8;			/* PnB4B : Reservation parameter(Setup information8)									*/
//	LONG	SetupInfo9;			/* PnB4D : Reservation parameter(Setup information9)									*/
//	LONG	bPOutInitZone;		/* PnB4F : ZONESignal specification												*/
/*--------------------------------------------------------------------------------------------------*/
//	LONG	Backlash;			/* PnB50 : Backlash correction												*/
/*--------------------------------------------------------------------------------------------------*/
//	LONG	SIMode;				/* PnB90 : Signal input method												*/
/*--------------------------------------------------------------------------------------------------*/
//	USHORT	MODE01Lgc;			/* PnBA0 : /MODE 0/1 Signal assignment									*/
//	USHORT	STARTHOMELgc;		/* PnBA1 : /START-STOP, /HOME Signal assignment							*/
//	USHORT	PGMRESJOGPLgc;		/* PnBA2 : /PGMRES , /JOGP Signal assignment								*/
//	USHORT	SEL0JOGNLgc;		/* PnBA3 : /SEL0 , /JOGN Signal assignment								*/
//	USHORT	SEL1JOG0Lgc;		/* PnBA4 : /SEL1 Signal assignment										*/
//	USHORT	SEL2JOG1Lgc;		/* PnBA5 : /SEL2 Signal assignment										*/
//	USHORT	SEL3JOG2Lgc;		/* PnBA6 : /SEL3 Signal assignment										*/
//	USHORT	SEL4JOG3Lgc;		/* PnBA7 : Reserved parameters (SEL4)									*/
//	USHORT	SEL5Lgc;			/* PnBA8 : Reserved parameters (SEL5)									*/
//	USHORT	SEL6Lgc;			/* PnBA9 : Reserved parameters (SEL6)									*/
//	USHORT	SEL7Lgc;			/* PnBAA : Reserved parameters (SEL7)									*/
	USHORT	SONLgc;				/* PnBAB : /S-ONSignal assignment										*/
	USHORT	POTLgc;				/* PnBAC : Reserved parameters (P-OT Signal assignment)						*/
	USHORT	NOTLgc;				/* PnBAD : Reserved parameters (N-OT Signal assignment)						*/
//	USHORT	DECLgc;				/* PnBAE : /DEC Signal assignment										*/
	USHORT	RTRGLgc;			/* PnBAF : Reserved parameters (/RGRTSignal assignment)						*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	ALMRSTLgc;			/* PnBB0 : /ALM-RST Signal assignment									*/
	USHORT	PCLLgc;				/* PnBB1 : /P-CLSignal assignment										*/
	USHORT	NCLLgc;				/* PnBB2 : /N-CLSignal assignment										*/
//	USHORT	CLRLgc;				/* PnBB3 : /CLRSignal assignment											*/
	USHORT	PCONLgc;			/* PnBB4 : /PCONSignal assignment										*/
	USHORT	ZCLAMPLgc;			/* PnBB5 : /ZCLAMPSignal assignment										*/
	USHORT	INHIBITLgc;			/* PnBB6 : /INHIBITSignal assignment										*/
	USHORT	PSELLgc;			/* PnBB7 : /PSELSignal assignment										*/
/* <S03D> Start */
/*--------------------------------------------------------------------------------------------------*/
	USHORT	SPDDLgc;			/* /SPD-D Signal assignment												*/
	USHORT	SPDALgc;			/* /SPD-A Signal assignment												*/
	USHORT	SPDBLgc;			/* /SPD-B Signal assignment												*/
	USHORT	CSELLgc;			/* /C-SEL Signal assignment												*/
/* <S03D> End */
/*--------------------------------------------------------------------------------------------------*/
	USHORT	GSELLgc;			/* /G-SEL Signal assignment <S09B> 										*/
	USHORT	SENLgc;				/* /SEN Signal assignment <S168>											*/
	USHORT	FstpLgc;			/* /Fstp Signal assignment <S1F5>										*/
	USHORT	PdetLgc;			/* /P-DET(Pn50D.3)										 <S21C> 	*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	bINPOSLgc;			/* PnBC0 : /INPOSITION Signal assignment									*/
//	USHORT	bPOUT0Lgc;			/* PnBC1 : /POUT0 Signal assignment										*/
//	USHORT	bPOUT1Lgc;			/* PnBC2 : /POUT1 Signal assignment										*/
//	USHORT	bPOUT2Lgc;			/* PnBC3 : /POUT2 Signal assignment										*/
//	USHORT	bPOUT3Lgc;			/* PnBC4 : /POUT3 Signal assignment										*/
//	USHORT	bPOUT4Lgc;			/* PnBC5 : Reserved parameters (/POUT4 Signal assignment)					*/
//	USHORT	bPOUT5Lgc;			/* PnBC6 : Reserved parameters (/POUT5 Signal assignment)					*/
//	USHORT	bPOUT6Lgc;			/* PnBC7 : Reserved parameters (/POUT6 Signal assignment)					*/
//	USHORT	bPOUT7Lgc;			/* PnBC8 : Reserved parameters (/POUT7 Signal assignment)					*/
	USHORT	bWRNLgc;			/* PnBC9 : /WARN Signal assignment										*/
	USHORT	bBKLgc;				/* PnBCA : /BK Signal assignment											*/
	USHORT	bSRDYLgc;			/* PnBCB : /S-RDY Signal assignment										*/
	USHORT	CLTLgc;				/* PnBCC : /CLT Signal assignment										*/
	USHORT	TGONLgc;			/* PnBCD : /TGON Signal assignment										*/
	USHORT	NEARLgc;			/* PnBCE : /NEAR Signal assignment										*/
	USHORT	VCMPLgc;			/* PnBCF : /V-CMP Signal assignment										*/
//	USHORT	BUSYLgc;			/* PnBD0 : /BUSY Signal assignment										*/
//	USHORT	POSRDYLgc;			/* PnBD1 : /POSRDY Signal assignment										*/
//	USHORT	PCOLgc;				/* PnBD2 : PCO Signal assignment											*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	VLTLgc;				/* PnBD3 : /VLT Signal assignment										*/
	USHORT	PSELALgc;			/* PnBDA : /PSELA Signal assignment										*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	ZretPressTrq;		/* PnBDA : Push torque during return to origin by push						*/
	USHORT	ZretPressTime1;		/* PnBDB : Push detection time during return to origin by push						*/
	USHORT	ZretPressTime2;		/* PnBDC : Waiting time for completion of return to origin during push return to origin					*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	TspdGain;			/* PnBE0 : Command speed gain											*/
/*--------------------------------------------------------------------------------------------------*/
	USHORT	spdDivision;		/* PnBF1 : Speed resolution setting											*/
	USHORT	IntEgear;			/* PnBF2 : Internal electronic gear												*/
} OPPRMDAT;
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Parameter setting data variableExtern																*/
/*																									*/
/****************************************************************************************************/
extern	OPPRMDAT	OpPrm[];

#endif
/***************************************** end of file **********************************************/

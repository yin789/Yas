/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MicroScanAIf.c : μProgramScanA I/FProcess definition												*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : μProgramScanAandCPUI/FProcessing														*/
/*																									*/
/*		μProgram -CPU.														*/
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
#ifndef MICRO_PRM_CALC_H
#define MICRO_PRM_CALC_H


#include "JL086ServoIpReg.h"
#include "Bprm.h"

/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
#define		MICRO_TR_READY			0		/* Microprogram response(READYStatus)					*/
#define		MICRO_TR_SIZE_SHORT		1		/* Write data size for micro parameter batch transfer		*/
#define		MICRO_TR_SIZE_LONG		2		/* Write data size for micro parameter batch transfer		*/
#define		MICRO_TR_DATA_MAX		32		/* Number of write data for microparameter batch transfer[WORD]	*/
#define		MICRO_TR_DATA_ERROR		-1		/* Micro parameter batch transfer error status			*/
#define		MICRO_TR_SUCCESS		0		/* Micro parameter batch transfer data setting complete				*/
#define		MICRO_ADDRESS_CHANGE	0x0fff	/* Micro address change								*/
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Field-weakening control variable definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct WEAKENFIELD {
	struct {						/* Constant data definition for field weakening										*/
		KSGAIN	KVdcAsic;				/* Main circuit detection value conversion gain [Admax] ⇒ [8192/Vdc]				*/
		KSGAIN	KpvBeforeDiv;			/* Voltage feedback loop gain calculation constant(Before dividing by speed)	*/
		KSGAIN	KivBeforeDiv;			/* Voltage feedback loop integral gain calculation constant(Before dividing by speed)*/
		LONG	IdrefLimTerm1;			/* dAxis current command limit value calculation constant(Speed^2)			*/
		LONG	IdrefLimTerm23;			/* dAxis current command limit value calculation constant(Terms unrelated to speed)		*/
		LONG	IdrefLimTerm4;			/* dAxis current command limit value calculation constant(Before dividing by speed)			*/
		SHORT	V1Max;					/* Voltage command limit level										*/
		SHORT	dummy1;					/* for alignment											*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {						/* Variable data definition for field weakening										*/
		LONG	Kpv;					/* Voltage feedback loop gain(After dividing by speed)			*/
		LONG	Kiv;					/* Voltage feedback loop integral gain(After dividing by speed)		*/
		LONG	VdcSum;					/* Main circuit voltage integrated value											*/
		SHORT	IdrefLim;				/* dAxis current command limit value									*/
		SHORT	KpvL;					/* Kpv16bit											*/
		SHORT	KpvH;					/* Kpvupper16bit											*/
		SHORT	KivL;					/* Kiv16bit											*/
		SHORT	KivH;					/* Kivupper16bit											*/
		SHORT	VdcAsic;				/* Main circuit voltage average value											*/
		SHORT	VdcAvgCnt;				/* Main circuit voltage average number of times										*/
		SHORT	VdcSumCnt;				/* Number of main circuit voltage integrations										*/
	} var;
} WEAKENFIELD;


/*--------------------------------------------------------------------------------------------------*/
/*		Variable definition for micro voltage compensation gain function														*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct MPKVVCOMP {
	struct {						/* Constant data definition for micro voltage compensation gain function					*/
		LONG	CompLevel;				/* Main circuit voltage correction level										*/
		LONG	KdMpKvv;				/* Calculate Gain for MpKvv									*/
		LONG	KfMpKvv;				/* Filter Gain for MpKvv									*/
		BOOL	MpKvvFlg;				/* Main circuit voltage correction function selection	[0:OFF,1:ON]					*/
		USHORT	vrevgn;					/* Voltage compensation gain											*/
	} conf;
	/*----------------------------------------------------------------------------------------------*/
	struct {						/* Variable data definition for micro voltage compensation gain function					*/
		SHORT	MpKvv;					/* Voltage compensation gain command			[8192/VdcBase]				*/
		SHORT	Dummy;					/* Dummy for Alignment										*/
	} var;
} MPKVVCOMP;


/* Input Parameter of "KpxPcalMicroProgram()" */
typedef struct MAICRO_CFGPRM {
	USHORT	imaxs;					/* PnE15 : Servopack maximum current									*/
	USHORT	ignd;					/* PnE20 : Current loop gain(D)									*/
	USHORT	ignq;					/* PnE21 : Current loop gain(Q)									*/
	USHORT	kid;					/* PnE22 : Current loop integral time constant(D)								*/
	USHORT	kiq;					/* PnE23 : Current loop integral time constant(Q)								*/
	USHORT	limintd;				/* PnE24 : Current loop integral limit value(D)								*/
	USHORT	limintq;				/* PnE25 : Current loop integral limit value(Q)								*/
	USHORT	ifil;					/* PnE26 : Current loop filter time constant								*/
	USHORT	shant;					/* PnE28 : Current detection gain										*/
	USHORT	pwmf;					/* PnE2C : PWM frequency											*/
	USHORT	dq_sw;					/* PnE2F : DQ switch										*/
	USHORT	vrevgn;					/* PnE2A : Voltage compensation gain										*/
	USHORT	lmdata;					/* PnE2B : Voltage limit value											*/
	DBYTEX	odt_c;					/* PnE2D : Lower:On-delay time, Upper level:On-delay compensation constant		*/
	DBYTEX	odlv_slp;				/* PnE2E : Lower:On-delay compensation change level, Upper level:Compensation slope		*/
	USHORT	motresq;				/* PnE3B : Notch filter for motor resonance suppressionQvalue					*/
	USHORT	fltrange;				/* PnE3F : Motor overcurrent detection filter count							*/	/* <S1F5> */
	USHORT	zadjiu;					/* PnE50 : Current detection zero adjustment(UPhase)									*/
	USHORT	zadjiv;					/* PnE51 : Current detection zero adjustment(VPhase)									*/
	DBYTEX	gadjiu_v;				/* PnE52 : Current detection gain adjustment(U,VPhase)								*/
	USHORT	pnvoltgn;				/* PnE70 : P-NVoltage detection level									*/
	USHORT	pwmf2;					/* PnEA5 : Quiet modePWMFrequency									*/
	USHORT	ilimlvl;				/* PnEA6 : Carrier frequency switching current threshold						*/
	USHORT	pwmf3;					/* PnEA7 : No.2PWMFrequency											*/
	USHORT	idlim;					/* PnEB2 : Voltage feedback integral limit						*/
	DBYTEX	oslv_pol;				/* PnF06 : OS detection level, Number of poles								*/
	USHORT	irat;					/* PnF09 : Rated current												*/
	USHORT	imax;					/* PnF0A : Instantaneous maximum current											*/
	USHORT	motresfrq;				/* PnF1C : Notch filter frequency for motor resonance suppression					*/
	USHORT	flg_wf;					/* PnF1E : Flag												*/
	MFUNCF	motfuncf;				/* PnF24 : Motor function flag2									*//*Motor carrier frequency switching<S117>*/
	USHORT	mot_pwmf;				/* PnF25 : MotorPWMFrequency										*//*Motor carrier frequency switching<S117>*/
#if 0	/* Unnecessary because carrier frequency is not unified<S11E> */
	BOOL	motCarrUse;				/* Motor carrier(PnF25)Enabled 	[0:OFF,1:ON]					*//*Motor carrier frequency switching<S117>*/
#endif		/*#if 0	 Unnecessary because carrier frequency is not unified<S11E> */
	SHORT	NormCarrCnt;			/* Normal carrier count value		[-]								*//*Motor carrier frequency switching<S117>*/
	SHORT	HighCarrCnt;			/* High carrier count value			[-]								*//*Motor carrier frequency switching<S117>*/
} MAICRO_CFGPRM;


/* Input Parameter of "PcalMicroDqAcrGain()"  */
typedef struct DQACR_GAIN_PRM {
	USHORT	ignd;					/* PnE20 : Current loop gain(D)									*/
	USHORT	ignq;					/* PnE21 : Current loop gain(Q)									*/
	USHORT	kid;					/* PnE22 : Current loop integral time constant(D)								*/
	USHORT	kiq;					/* PnE23 : Current loop integral time constant(Q)								*/
	USHORT	ifil;					/* PnE26 : Current loop filter time constant								*/
	USHORT	ifil2;					/* PnEA4 : Quiet mode current loop filter time constant					*/
} DQACR_GAIN_PRM;


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/

LONG	KpxPcalMicroProgram( MAICRO_CFGPRM *uCfgPrm, BPRMDAT *Bprm,
							 ASICS *SvAsicRegs, WEAKENFIELD* WeakenField );
LONG	KpxMotorOCLevel( MAICRO_CFGPRM *uCfgPrm, ASICS *SvAsicRegs, MENCPRM *MencP );	/*<S1F5>*/
void	KpiPcalMicroNotchFilter2( ASICS *SvAsicRegs, LONG hz, LONG qx, LONG dx, SHORT *MpReg );
LONG	KpiChangePwmFrequency( ULONG PwmFx, USHORT pwmf, USHORT pwmf3, USHORT flg_wf );
void	KpiManageMicroTorqueFilter( VUSHORT *mreg_csw_ptr, USHORT *CswData, USHORT FilSel, LONG FilOn );
LONG	LpxSetPrmToASIC( SHORT *adr, LONG prm, BOOL limchk );
void	PcalMicroDqAcrGain(	ASIC_U_IF_STR	*pAsicMicroReg,	/* Control variable pointer						*/
							DQACR_GAIN_PRM	*Prm,			/* Parameter data pointer				*/
							BPRMDAT			*Bprm,			/* Base parameter pointer 			*/
							ULONG			Loophz,			/* Speed loop gain			[0.1Hz]		*/
							ULONG			ratio);			/* Gain balance			[%]			*/

LONG	PcalDqAcrKp( LONG Gain, LONG Derate, BPRMDAT *Bprm, LONG axis_type );
LONG	PcalDqAcrKi( LONG Ti, LONG Kp, KSGAIN MotL, BPRMDAT *Bprm );
LONG	PcalDqAcrFilter( LONG Tf );
#if 0 /* 2012.08.31 Y.Oka ★μScanA→CpuScanA★ */
LONG	MicroTranslatePrmReq( ASICS *SvAsicRegs, void *Data, VUSHORT *Address, LONG Size );
#endif


void PcmKvvIniPrmCalc( MPKVVCOMP *MpKvvComp, BPRMDAT *Bprm,
					   USHORT kvvfil, USHORT pnvoltgn, USHORT vrevgn, BOOL enable );
void	PcmWFPrmCalc( WEAKENFIELD *wf_ptr, LONG MotSpd, LONG DcVolt );		/* <S024> */
void	PcmMicroKvvCalc( MPKVVCOMP *MpKvvComp, LONG DcVolt, USHORT UvLevel );
void	PcalVFBKp( WEAKENFIELD *WeakenField, ASICS *SvAsicRegs, BPRMDAT *Bprm, USHORT kv, USHORT tv );
void	PcalVFBKi( WEAKENFIELD *WeakenField, ASICS *SvAsicRegs, BPRMDAT *Bprm, USHORT kv);
void	PcalIdrefLimTerm1( WEAKENFIELD *WeakenField, ASICS *SvAsicRegs, BPRMDAT *Bprm, LONG v1max, LONG vmaxid);
void	PcalIdrefLimTerm234(WEAKENFIELD* weaken_ptr, ASICS *SvAsicRegs, BPRMDAT* Bprm, DBYTEX oslv_pol, USHORT pnvoltgn);


#endif /* MICRO_PRM_CALC_H */
/***************************************** end of file **********************************************/

#ifndef MODEL_CONTROL_H
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ModelControl.h : Direct torque reference method model following control related processing definition					*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.02.23	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "MfcHandler.h"
#include "Bprm.h"

/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/
//#define	MFCONEMASSCYCLE		KPI_SBCYCLENS			/* Rigid body MFC calculation cycle time(ScanB))		[ns]	*/
#define	MFCTWOMASSCYCLE(CycleTimeNs)	(CycleTimeNs << 1)	/* Two-inertia MFC calculation cycle time(ScanB×2)	[ns]	*/
#define	C10POW9				1000000000				/* 10^9											*/
#define	E9C2PAI2			25330296				/* 10^9 /(2*PAI)^2								*/

/*--------------------------------------------------------------------------------------------------*/
#define SGDV_TYPE		0x00						/* SGDVType									*/
#define SGDX_TYPE		0x01						/* SGDXCompatibility mode								*/
/*--------------------------------------------------------------------------------------------------*/
#define ONE_MASS		0x01						/* Rigid body model									*/
#define TWO_MASS1		0x02						/* 2 inertia model 1							*/
#define TWO_MASS2		0x03						/* 2 inertial model 2							*/
#define ONE_BASE		0x04						/* Rigid body + machine model							*/


typedef struct MFC_CFG_PRM {
	USHORT	mdlsw;				/* Pn140 : Model following control related switches								*/
	USHORT	mdlgn;				/* Pn141 : Model following control gain [0.1/s]								*/
	USHORT	mdlzeta;			/* Pn142 : Model following control gain correction									*/
	USHORT	mdlptff;			/* Pn143 : Model following control bias(Forward rotation direction)							*/
	USHORT	mdlntff;			/* Pn144 : Model following control bias(Reverse direction)							*/
	USHORT	mdlantfrq;			/* Pn145 : Vibration suppression 1 frequency A										*/
	USHORT	mdlresfrq;			/* Pn146 : Vibration suppression 1 frequency B										*/
	USHORT	mdlvff;				/* Pn147 : Model following control speed feedforward compensation					*/
	USHORT	mdlgn2;				/* Pn148 : Second model following control gain [0.1/s]							*/
	USHORT	mdlzeta2;			/* Pn149 : Second model following control gain correction								*/
} MFC_CFG_PRM;


/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
void	MfcInitModelControl( MFCTRL *MFControl );
LONG	MfcControlMain( MFCTRL *MFControl, LONG dPcmd, LONG *SpdFFCx,  LONG *TrqFFCx );

/****************************************************************************************************/
/*		API's																						*/
/****************************************************************************************************/
SHORT	MfcGetParamEnableSts( MFCTRL *MFControl );
void	MfcSetMfcCtrlSwitch( MFCTRL *MFControl, BOOL Switch );

BOOL	MfcCalculateInitPrm( MFCTRL *MFControl, MFC_CFG_PRM *MfcCfgPrm,
    	                     USHORT jrate, BPRMDAT *Bprm, USHORT mfctype );
BOOL	MfcCalculatePrmSW( MFCTRL *MFControl, MFC_CFG_PRM *MfcCfgPrm, USHORT jrate, BPRMDAT *Bprm );
void	MfcCalculatePrm( MFCTRL *MFControl, MFC_CFG_PRM	*MfcCfgPrm,
    	                 USHORT jrate, BPRMDAT *Bprm, LONG cfric, LONG GselNo );


#define MODEL_CONTROL_H
#endif /* MODEL_CONTROL_H */
/***************************************** end of file **********************************************/

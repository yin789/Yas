#ifndef RIPPLE_COMP_H
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		RippleComp.h : Ripple compensation related processing definition														*/
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
#include "BaseControls.h"
#include "SerialEncIf.h"	/*<S1FD>*/
#include "Bprm.h"			/*<S1FD>*/
//#include "MercuryGlobals.h"
/****************************************************************************************************/
/*		DEFINES																						*/
/****************************************************************************************************/



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
LONG	RippleCompensation( RPLCMP *RippleCmp, USHORT MechaPhase, USHORT MotPhase, BOOL BaseEnable);
void	RippleCompChkThresholdVal( RPLCMP *RippleCmp, USHORT CtrlMode, LONG SpdRef,
																	LONG SpdFb, BOOL TuneLessCtrl );
BOOL	CalVelRippleCompE( RPLCMP *RippleCmp, BPRMDAT *Bprm, USHORT RippleCompGainE, 
							USHORT *RippleCompCycleE, SHORT *RippleCompAmplitudeE, USHORT *RippleCompPhaseE);
ULONG	ChkMotorSerialNo( UINT AxisNo, MENCV *MencV );
void	FnRegCalVelRippleCompE( BPRMDAT *Bprm, RPLCMP *RippleCmp );
BOOL	RippleCompGetRippleWrnState( RPLCMP *RippleCmp );/*<S1FD>*/

#if 0/* TODO : Speed ripple compensation Mechanical angle correction processing(Commented out for future use) */
BOOL	CalVelRippleCompM( RPLCMP *RippleCmp, BPRMDAT *Bprm, USHORT RippleCompGainM, 
							USHORT *RippleCompCycleM, SHORT *RippleCompAmplitudeM, USHORT *RippleCompPhaseM);

void	FnRegCalVelRippleCompM( AXIS_HANDLE *Axis );
#endif/* TODO : Speed ripple compensation Mechanical angle correction processing(Commented out for future use) */

//LONG	RippleCompGetMonOutSel( RPLCMP_PRM *RippleCmpP );	/* Monitor output selection *//* @@Currently not supported */



#define RIPPLE_COMP_H
#endif /* RIPPLE_COMP_H */
/***************************************** end of file **********************************************/

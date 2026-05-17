/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Disturbance observer definition header file															*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2013.08.17	Y.Oka	For Mercury												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#ifndef	_DISTURB_OBSERVER_H
#define _DISTURB_OBSERVER_H


#include "BaseControls.h"






/****************************************************************************************************/
/*																									*/
/*		DEFINES																						*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		for Tuning Less Control																		*/
/****************************************************************************************************/


/****************************************************************************************************/
/*		for Disturb Observer																		*/
/****************************************************************************************************/
enum DOBS_CHANGE_STS{
	DOBS_NOCHANGE		= (LONG)0x00L,			/* Disturbance observer disabled⇔No enabling switching				*/
	DOBSCHNG_INV2ACT	= (LONG)0x01L,			/* Disturbance observer disabled⇒Enabling switching					*/
	DOBSCHNG_ACT2INV	= (LONG)0x02L,			/* Disturbance observer enabled⇒Disable switching					*/
};

/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		Globals																						*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		for Tuning Less Control																		*/
/****************************************************************************************************/
void	TuneLessCtrlInitialize( TUNELESS_CTRL *TuneLessCtrl );
void	TuningLessCtrl( TUNELESS_CTRL *TuneLessCtrl, KSGAIN MotSpd, KSGAIN TrqRefi, KSGAIN TrqRef );			/*<S00A>*/

/****************************************************************************************************/
/*		for Disturb Observer																		*/
/****************************************************************************************************/
void	DisturbObserverInitialize( DOBS_CTRL *DobsCtrl, GSELDOBS *pActGains );			/* <S09B> */
void	DisturbObserver( DOBS_CTRL *DobsCtrl, KSGAIN MotSpd, KSGAIN TrqRef );				/*<S00A>*/

/****************************************************************************************************/
/*		for Eh Velocity Observer																	*/
/****************************************************************************************************/
void	EhSpeedObserverInitialize( EHVOBS_CTRL *EhVobsCtrl );
LONG	EhSpeedObserver( EHVOBS_CTRL *EhVobsCtrl, KSGAIN MotSpd, KSGAIN TrqRef );				/*<S00A>*/
void	InitParamFriction( BASE_LOOPCTRLS *BaseLoops );/*<S0CD>*/
LONG	LpxMotorFricComp( BOOL BaseEnable, BASE_LOOPCTRLS *BaseLoops );/*<S0CD>*/

/****************************************************************************************************/
/*		API's																						*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		for Tuning Less Control																		*/
/****************************************************************************************************/
/* Adjustment-less use/For obtaining non-used settingsAPI */
__inline	BOOL	TuneLessGetTuneLessUse( TUNELESS_CTRL *TuneLessCtrl )
{
	return (TuneLessCtrl->conf.TuningLessUse);
}
/* For state acquisition without adjustmentAPI */
__inline	BOOL	TuneLessGetTuneLessSts( TUNELESS_CTRL *TuneLessCtrl )
{
	return (TuneLessCtrl->var.TuneLessAct);
}
/* For obtaining torque compensation value using adjustmentless disturbance observerAPI */
__inline	LONG	TuneLessGetDisTrqCmp( TUNELESS_CTRL *TuneLessCtrl )
{
	return (TuneLessCtrl->Drcomp.var.DisTrqCmp);
}
/* For obtaining estimated disturbance using adjustmentless disturbance observerAPI */
__inline	LONG	TuneLessGetDisTrq( TUNELESS_CTRL *TuneLessCtrl )
{
	return (TuneLessCtrl->Drcomp.var.DisTrq);
}
/* For acquisition of torque command filter integral initialization request by adjustmentless disturbance observerAPI */
__inline	BOOL	TuneLessGetTFilClrReq( TUNELESS_CTRL *TuneLessCtrl )
{
	return (TuneLessCtrl->Drcomp.var.TrqFilClrReq);
}
/* For estimated speed acquisition by adjustment-less phase compensation speed observerAPI */
__inline	LONG	TuneLessGetSpdObsFbk( TUNELESS_CTRL *TuneLessCtrl )
{
	return (TuneLessCtrl->EhVobs.var.EhSpdObs);
}
/* Release of forced invalidation request without adjustmentAPI */
__inline	void	TuneLessClearForceInvldReq( TUNELESS_CTRL *TuneLessCtrl )
{
	TuneLessCtrl->var.TuneLessInvldReq = FALSE;
}
/* Forced invalidation request setting without adjustmentAPI */
__inline	void	TuneLessSetForceInvldReq( TUNELESS_CTRL *TuneLessCtrl )
{
	TuneLessCtrl->var.TuneLessAct = FALSE;
	TuneLessCtrl->var.TuneLessInvldReq = TRUE;
}
/* For state acquisition without adjustmentAPI */
__inline	BOOL	TuneLessGetTuneLessModeEx( TUNELESS_CTRL *TuneLessCtrl ) /* <S1C7> */
{
	return (TuneLessCtrl->conf.TuningLessEx);
}


/****************************************************************************************************/
/*		for Disturb Observer																		*/
/****************************************************************************************************/
/* For obtaining disturbance observer statusAPI */
__inline	BOOL	DobsGetObserverSts( DOBS_CTRL *DobsCtrl )
{
	return (DobsCtrl->var.DobsAct);
}
/* For acquisition of torque compensation value by disturbance observerAPI */
__inline	LONG	DobsGetDisTrqCmp( DOBS_CTRL *DobsCtrl )
{
	return (DobsCtrl->var.DisTrqCmp);
}
/* For acquisition of switching state by disturbance observerAPI */
__inline	UCHAR	DobsGetExchangeSts( DOBS_CTRL *DobsCtrl )
{
	return (DobsCtrl->var.ExchangeSts);
}


/****************************************************************************************************/
/*		for Eh Velocity Observer																	*/
/****************************************************************************************************/
/* Adjustment-less use/For obtaining non-used settingsAPI */
__inline	BOOL	EhVobsGetObserverUse( EHVOBS_CTRL *EhVobsCtrl )
{
	return (EhVobsCtrl->conf.EhVobsUse);
}


/****************************************************************************************************/
/*																									*/
/*		EXTERN																						*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		for Tuning Less Control																		*/
/****************************************************************************************************/
extern	BOOL	TuneLessGetTuneLessSts( TUNELESS_CTRL *TuneLessCtrl );
extern	LONG	TuneLessGetSpdObsFbk( TUNELESS_CTRL *TuneLessCtrl );
extern	LONG	TuneLessGetDisTrqCmp( TUNELESS_CTRL *TuneLessCtrl );
extern	LONG	TuneLessGetDisTrq( TUNELESS_CTRL *TuneLessCtrl );
extern	BOOL	TuneLessGetTFilClrReq( TUNELESS_CTRL *TuneLessCtrl );
extern	void	TuneLessSetForceInvldReq( TUNELESS_CTRL *TuneLessCtrl );
extern	void	TuneLessClearForceInvldReq( TUNELESS_CTRL *TuneLessCtrl );
/* <S1A5> */
extern  void	TuneLessInitRobustCompensator( 	TUNELESS_CTRL *TuneLessCtrl, 
												TUNELESS_DRCOMP	*Drcomp );	

/****************************************************************************************************/
/*		for Disturb Observer																		*/
/****************************************************************************************************/
extern	BOOL	DobsGetObserverSts( DOBS_CTRL *DobsCtrl );
extern	LONG	DobsGetDisTrqCmp( DOBS_CTRL *DobsCtrl );
extern	UCHAR	DobsGetExchangeSts( DOBS_CTRL *DobsCtrl );



/***************************************** end of file **********************************************/
#endif /* _DISTURB_OBSERVER_H */

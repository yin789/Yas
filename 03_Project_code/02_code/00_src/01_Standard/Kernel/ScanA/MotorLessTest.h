/****************************************************************************
Description	: Motor Less Test Mode
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2013 All Rights Reserved

Project       : Mercury

Functions	  : none

----------------------------------------------------------------------------
Changes		:
Name		Date			Description
------------------------------------------------------------------------
K.Sakamoto	2013.11.11		created <S00X>

****************************************************************************/
#ifndef MOTORLESSTEST_H__
#define MOTORLESSTEST_H__

/*==========================================================================*/
/* I N C L U D E															*/
/*==========================================================================*/
#include "Basedef.h"
#include "PnPrmStruct.h"
#include "EncData.h"
#include "MercuryGlobals.h"
#include "BaseControls.h"

/*==========================================================================*/
/* D E F I N E																*/
/*==========================================================================*/

/*==========================================================================*/
/* S T R U C T																*/
/*==========================================================================*/

/*==========================================================================*/
/* E X T E R N   V A R I A B L E											*/
/*==========================================================================*/

/*==========================================================================*/
/* P R O T O T Y P E														*/
/*==========================================================================*/
/* --- Motorless test --- */
/* Motorless test mode mechanical model calculation	*/
void ApxMotorLessTestModeMechaModel( LONG TrqRef, BOOL BeComplete, MOTLESSTST *MotorLessTest );
/* Motorless test mode parameter calculation	*/
void PcalMotorLessTestMode( MENCV *FencV, BPRMDAT *Bprm, PRMDATA *Prm, MOTLESSMDL *MotorLessMdl );
/* Motorless test mode variable initialization	*/
void ApxInitMotorLessTestMode( AXIS_HANDLE *AxisRsc );
/* Motorless test mode current data setting	*/
void ApiSetCurDatasMotorLessTestMode( BASE_LOOPCTRLS *BaseLoops, CTRL_LOOP_OUT *CtrlLoopOut );
/* Motorless test mode speed/position data setting	*/
void ApiSetMechaDatasMotorLessTestMode( BPRMDAT *Bprm, MOTLESSTST *MotorLessTest );
/* Motor parameter calculation for motorless test mode: When encoder is not connected	*/
void ApiReadMotPrmMotorLessTestMode( MENCV *MencV, PRMDATA *Prm );
/* Motor parameter calculation for motorless test mode: When Fulck encoder is not connected	*/
void ApiReadFencPrmMotorLessTestMode( MENCPRM *MencP );

/* --- Motorless test mode --- */
/* Motorless test mode flag setting	*/
void MltSetMotorLessTestMode( AXIS_HANDLE *AxisRscI, INT AxisRscNum );

#if 0
/* --- Motorless test modeIFGet definition address(Interim) --- */
/* Motorless test mode status acquisition		*/
BOOL MltGetMotorLessTestMode( void );
/* Motorless test modeIFGet definition address	*/
MOTLESSTST *MltGetMotorLessTestHandle( void );
#endif

#endif // MOTORLESSTEST_H__
/*-------end of file----------------------------------------------------------------------------------------*/

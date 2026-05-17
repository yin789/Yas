#ifndef __FN_TABLE_COMMAND_H /* For reload prevention */
/****************************************************************************
  Description   : Table Command Drive Mode (only Register IF) (2007H)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.02.18   created

****************************************************************************/
#include "Basedef.h"



/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Table operation torque input position definition																*/
/*--------------------------------------------------------------------------------------------------*/
enum TBLCMD_TRQ_INPUT {
	TBLTRQIN_NONE			= 0x00,			/* None													*/
	TBLTRQIN_BEF_FIL,						/* Before torque command filter									*/
	TBLTRQIN_BEF_NTCHFIL,					/* 1Order after filter, before notch filter						*/
	TBLTRQIN_AFT_FIL,						/* After torque command filter									*/
};


/*--------------------------------------------------------------------------------------------------*/
/*		Table operation command direction definition																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	TBLCMD_DRV_FOR			 1			/* Operation in the forward direction											*/
#define	TBLCMD_DRV_REV			-1			/* Reverse operation											*/



/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/




/****************************************************************************************************/
#define __FN_TABLE_COMMAND_H
#endif /* __FN_TABLE_COMMAND_H */

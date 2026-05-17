#ifndef __FN_INIT_VIB_DET_LVL_H /* For reload prevention */
/****************************************************************************
  Description   : Initialize Vibration Detect Level (Fn01B)
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation. 
                  Copyright (c) 2010 All Rights Reserved

  Project       : INGRAM

----------------------------------------------------------------------------
  Changes		:
	Name   Date          Description
----------------------------------------------------------------------------
	Y.Oka   2011.01.31   created

****************************************************************************/
#include "Basedef.h"



/****************************************************************************************************/
/*																									*/
/*		Structure Definition																		*/
/*																									*/
/****************************************************************************************************/
typedef	struct INIVIBDET {
	struct {
		BOOL	DetSpdActive;					/* Vibration detection enabled/Disabled								*/
	} var;
} INIVIBDET;


/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
#define		VIBDET_LVL		(0x1000000/10)		/* Vibration detection effective speed(OSLevel10%)					*/

enum VIBDETLVL_SEQ_STS {
	VIBDETLVL_SEQ_INIT		= 0,				/* Waiting for vibration level detection to start							*/
	VIBDETLVL_SEQ_DETECT,						/* Detecting vibration level									*/
	VIBDETLVL_SEQ_STORE,						/* Save vibration level									*/
	VIBDETLVL_SEQ_ERROR,						/* Vibration detection error									*/
	VIBDETLVL_SEQ_END,							/* Termination processing											*/
};


/****************************************************************************************************/
/*																									*/
/*		Function Prototype Definition																*/
/*																									*/
/****************************************************************************************************/




/****************************************************************************************************/
#define __FN_INIT_VIB_DET_LVL_H
#endif /* __FN_INIT_VIB_DET_LVL_H */

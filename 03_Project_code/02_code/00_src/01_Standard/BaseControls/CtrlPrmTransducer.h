/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Control parameter variable batch conversion process definition header file										*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2010.09.28	Y.Oka	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#ifndef	_CTRLPRM_TRANSDUCER_H
#define _CTRLPRM_TRANSDUCER_H


#include "Basedef.h"



/****************************************************************************************************/
/*																									*/
/*		STRUCT DEFINITION																			*/
/*																									*/
/****************************************************************************************************/
typedef struct _TRANSDUCE_CTRL {
	BOOL		TranslateMode;		/* Mode selection: Use interrupt handling(1)/Not used(0)					*/
	UCHAR		TranslateSts;		/* Conversion status: Requesting execution(1 or 2)/Complete(0)							*/
	UCHAR		TranslateSize;		/* Conversion data size												*/
	USHORT		Dummy;				/* for Alignment												*/
	ULONG		*TargetAddress;		/* Conversion destination address												*/
	ULONG		DataArray[32];		/* Conversion data temporary buffer								*/
} TRANSDUCE_CTRL;


/****************************************************************************************************/
/*																									*/
/*		DEFINES																						*/
/*																									*/
/****************************************************************************************************/
/****************************************************************************************************/
/*		Transfer size definition																			*/
/****************************************************************************************************/
#define		TR_DATA_MAX				32			/* Number of write data for batch transfer[WORD]					*/

/****************************************************************************************************/
/*		Transfer method																					*/
/****************************************************************************************************/
enum TR_MODE {
	TMODE_DIRECT		= 0,					/* Conversion mode:DIRECT(Before interrupt activation)				*/
	TMODE_INTERRUPT		= 1,					/* Conversion mode:INTERRUPT(After interrupt activation)			*/
};

/****************************************************************************************************/
/*		Transfer error definition																				*/
/****************************************************************************************************/
enum TR_ERR_STATUS {
	TR_DATA_ERROR		= -1,					/* Batch transfer error status							*/
	TR_SUCCESS			= 0,					/* Batch transfer data setting complete							*/
};

/****************************************************************************************************/
/*		Transfer state definition																				*/
/****************************************************************************************************/
enum TR_STATUS {
	TR_READY			= 0,					/* Batch transfer free status(READYStatus)						*/
	TR_REQ_SCANA		= 1,					/* ScanA(BUSYStatus)					*/
	TR_REQ_SCANB		= 2,					/* ScanB(BUSYStatus)					*/
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
void	TranlateCtrlPrmInitialize( TRANSDUCE_CTRL *TransDuceCtrl );
LONG	TranlateCtrlPrmRequest( TRANSDUCE_CTRL *TransDuceCtrl, void *Data, ULONG *Address, UCHAR Size, CHAR Timing );
void	TranlateCtrlPrm( TRANSDUCE_CTRL *TransDuceCtrl );

/****************************************************************************************************/
/*		API's																						*/
/****************************************************************************************************/
/* For changing transfer modeAPI */
__inline	void	TranlateCtrlSetIntMode( TRANSDUCE_CTRL *TransDuceCtrl )
{
	TransDuceCtrl->TranslateMode = TMODE_INTERRUPT;
}
/* For transfer request acquisitionAPI */
__inline	UCHAR	TranlateCtrlGetReq( TRANSDUCE_CTRL *TransDuceCtrl )
{
	return (TransDuceCtrl->TranslateSts);
}



/****************************************************************************************************/
/*																									*/
/*		EXTERN																						*/
/*																									*/
/****************************************************************************************************/



/***************************************** end of file **********************************************/
#endif /* _CTRLPRM_TRANSDUCER_H */

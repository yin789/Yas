/*****************************************************************************
  Description   : Fn005, Fn006, Fn014, Fn030, Parameter Recalc.
------------------------------------------------------------------------------
  Author        : Yaskawa Electric Corporation.
                  Copyright (c) 2010 All Rights Reserved

  Project       : MERCURY

  Create Date   : 2014-01-08

*****************************************************************************/

enum OptCardDetClearMode
{
	OPTCLR_CARD_NOT,				/* Mode Nothing							*/
	OPTCLR_CARD_COMMAND,			/* COMMAND Mode							*/
	OPTCLR_CARD_SAFE,				/* SAFE Mode							*/
	OPTCLR_CARD_FB					/* FB Mode								*/
};

/****************************************************************************/
/*																			*/
/*		Struct Definition													*/
/*																			*/
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*		For option card detection clear Constant/variable data definition					*/
/*--------------------------------------------------------------------------*/
typedef struct OPTCLR{
	struct {
		USHORT	OptCardNo;			/* 0x20D0:Option card selection			*/
	} OpeReg;
	struct {
		USHORT	CmdOptId;			/* Command optionID						*/
		USHORT	SftOptId;			/* Safety optionID				*/
		USHORT	FbkOptId;			/* Feedback optionID			*/
	} var;
}OPTCLR;


/******************************* End of File ********************************/

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
#ifndef MOTORLESSTESTSTR_H__
#define MOTORLESSTESTSTR_H__

/*==========================================================================*/
/* I N C L U D E															*/
/*==========================================================================*/
#include "Basedef.h"
#include "PnPrmStruct.h"
#include "EncData.h"

/*==========================================================================*/
/* D E F I N E																*/
/*==========================================================================*/

/*==========================================================================*/
/* S T R U C T																*/
/*==========================================================================*/
/*****************************************************************************
*	Variable definition for motorless test mode
*****************************************************************************/
/* Parameter definition for internal mechanical model	*/
typedef	struct MOTLESSMDLP {
	LONG	MdlKj;					/* Model inertia gain									*/
	LONG	MdlKvp;					/* Speed[2^24/OvrSpd]→Position difference[pulse/Scan](Control)Conversion coefficient	*/
	LONG	MdlKvpM;				/* Speed[2^24/OvrSpd]→Position difference[pulse/Scan](Motor)Conversion coefficient	*/
} MOTLESSMDLP;

/* Parameter & variable definition for internal mechanical model	*/
typedef	struct MOTLESSMDL {
	MOTLESSMDLP	P[2];				/* Parameters for motorless test mode				*/
	MOTLESSMDLP	*PrmPtr;			/* Pointer to valid parameter						*/

	struct {						/* Variable data definition for motorless test mode	*/
		LONG	MotMdlSpdFbk;			/* Model speed FB				[2^24/OvrSpd]	*/
		LONG	MotMdldPosFbk;			/* Motor model position FB difference		[pluse/scan]	*/
		LONG	MotMdlPosFbk;			/* Motor model position FB			[pluse]			*/
		LONG	MechaMdldPosFbk;		/* Model position FB difference			[pluse/scan]	*/
		LONG	MechaMdlPosFbk;			/* Model position FB				[pluse]			*/
		LONG	Mvar;					/* For model speed integral calculation							*/
		LONG	MotMdlSfbrem;			/* Model speed FB excess							*/
		LONG	MotMdlpfbrem;			/* Model position FB too much							*/
		LONG	MotMdlpfbremM;			/* Model motor position FB too much					*/
	} V;
} MOTLESSMDL;

/*****************************************************************************
*	Motorless test mode data definition
*****************************************************************************/
/* Encoder data	*/
typedef	union ENCODER_DATA {
	LONG		Long[2];			/* Location data(32bit×2=64bit)Definition		*/
	struct {						/* Position data encoder format definition	*/
		DWORDX	RxPosL;					/* Encoder format data(Low)  32bit	*/
		SHORT	RxPosH;					/* Encoder format data(High) 16bit	*/
		SHORT	Dummy;					/* Dummy data						  16bit	*/
	} EncFormat;
} ENCODER_DATA;

/* EncoderIF */
typedef	struct MOTOR_LESS {
	BOOL			SemiEncConnect;	/* Semi-Closed Encoder Connected		*/
	BOOL			FullEncConnect;	/* Full-Closed Encoder Connected		*/
	BOOL			FencType;		/* Full-Closed Encoder Type				*/
	LONG			dSemiModePos;	/* Semi-position difference data					*/
	LONG			dFullModePos;	/* Full position difference data					*/
	ENCODER_DATA	SencData;		/* Semi-encoder data					*/
	ENCODER_DATA	FencData;		/* Full encoder data					*/
} MOTOR_LESS;

/*****************************************************************************
*	Motorless test modeIFDefinition
*****************************************************************************/
typedef struct MOTLESSTST {
	BOOL		MotorLessTestMode;	/* Motorless test mode flag			*/

	/* APIPointer to data that is difficult to specify with an argument(Set at initialization)	*/
	MENCV		*MencV;				/* Pointer to motor encoder data	*/
	MENCV		*FencV;				/* Pointer to Fulque encoder data	*/
	PRMDATA		*Prm;				/* Pointer to parameter data			*/

	/* Workpiece for motorless test mode	*/
	MOTLESSMDL	MotorLessMdl;		/* Data for internal mechanical model				*/
	MOTOR_LESS	MotorLessTestDatas;	/* EncoderIF					*/
} MOTLESSTST;

/*==========================================================================*/
/* E X T E R N   V A R I A B L E											*/
/*==========================================================================*/

/*==========================================================================*/
/* P R O T O T Y P E														*/
/*==========================================================================*/

#endif // MOTORLESSTESTSTR_H__
/*-------end of file----------------------------------------------------------------------------------------*/

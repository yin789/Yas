/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MotorPoleFind.h : Header file for magnetic pole detection control											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2011.01.25	Y.Oka	for INGRAM												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#ifndef	_MOT_POLE_FIND_H
#define _MOT_POLE_FIND_H


#include "Basedef.h"
#include "MpfHandler.h"
#include "alarm.h"
#include "EncData.h"
#include "Bprm.h"



/****************************************************************************************************/
/*		DEFINES																					*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Phase constant																					*/
/*--------------------------------------------------------------------------------------------------*/
#define	PHASE_DEG000		0
#define PHASE_DEG022_5		4096
#define PHASE_DEG030		5461
#define PHASE_DEG045		8192
#define PHASE_DEG090		16384
#define PHASE_DEG135		24576
#define PHASE_DEG180		32768
#define PHASE_DEG225		40960
#define PHASE_DEG315		57344
#define PHASE_DEG360		65536
/*--------------------------------------------------------------------------------------------------*/
#define SINPHASE_360		16384
#define SINPHASE_090		4096
#define SINPHASE_045		2048


/*--------------------------------------------------------------------------------------------------*/
/*		Magnetic pole detection sequence step																	*/
/*--------------------------------------------------------------------------------------------------*/
enum POLE_FIND_SEQ_STEP {
	MPF_INIT		= 0,
	MPF_INIT2,
	MPF_DRIVE,
	MPF_CAL,
	MPF_STOP,
	MPF_CONFIRM,
	MPF_END,
};

/*--------------------------------------------------------------------------------------------------*/
/*		Speed command creation sequence step																*/
/*--------------------------------------------------------------------------------------------------*/
enum POLE_FIND_VREF_STEP {
	MPFLAU_INIT		= 0,
	MPFLAU_ACC,
	MPFLAU_CONST,
	MPFLAU_DEC,
	MPFLAU_WAIT,
};

/*--------------------------------------------------------------------------------------------------*/
/*		Position command creation sequence step																*/
/*--------------------------------------------------------------------------------------------------*/
enum POLE_FIND_PREF_STEP {
	MPFPOS_INIT		= 0,
	MPFPOS_PLUS,
	MPFPOS_WAIT1,
	MPFPOS_MINUS,
	MPFPOS_WAIT2,
};

/*--------------------------------------------------------------------------------------------------*/
/*		Thrust command creation sequence step																*/
/*--------------------------------------------------------------------------------------------------*/
enum POLE_FIND_TREF_STEP {
	MPFTRQ_INIT		= 0,
	MPFTRQ_ACC,
	MPFTRQ_CONST,
	MPFTRQ_DEC,
	MPFTRQ_END,
};

/*--------------------------------------------------------------------------------------------------*/
/*		Operation confirmation sequence step																	*/
/*--------------------------------------------------------------------------------------------------*/
enum POLE_FIND_CONF_STEP {
	MPFCSTEP_WAIT		= 0,
	MPFCSTEP_PHSCHANGE,
	MPFCSTEP_ADJUST,
	MPFCSTEP_PHSSTORE,
	MPFCSTEP_PHSREVERT,
};

/*--------------------------------------------------------------------------------------------------*/
/*		8Division area check sequence step															*/
/*--------------------------------------------------------------------------------------------------*/
enum POLE_FIND_ZONE_CHK_STEP {
	PAREACHECK_000	= 0,
	PAREACHECK_001,
	PAREACHECK_002,
	PAREACHECK_003,
};

/*--------------------------------------------------------------------------------------------------*/
/*		d-qAxis misalignment direction																				*/
/*--------------------------------------------------------------------------------------------------*/
#define AXISERR_EQUAL		0				/* dAxisMax = qAxisMax									*/
#define AXISERR_MINUS		1				/* dAxisMax > qAxisMax									*/
#define AXISERR_PLUS		2				/* dAxisMax < qAxisMax									*/

/*--------------------------------------------------------------------------------------------------*/
/*		Other constants																				*/
/*--------------------------------------------------------------------------------------------------*/
#define TRYCOUNT			3



/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
LONG	MpoleFindSpeedControl( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo, BOOL BeComplete );
ALMID_T	MpoleFindSequence( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo, BOOL PdetReq, BOOL BaseEnable );
//ALMID_T	MpoleFindDetectAlarm( MPFIND *MpFind, MPFEXEINFO *MpfExeInfo, BOOL NetPdetReq );
//ALMID_T	MpoleFindCheckMovingRange( MPFIND *MpFind, LONG dMotPos, KSGAIN Kmovpos );


/****************************************************************************************************/
/*		for Parameter Calculations																	*/
/****************************************************************************************************/
typedef struct MPFIND_UPRM {
	USHORT	pdetloophz;				/* Pn481 : Magnetic pole detection speed loop gain									*/
	USHORT	pdetpitime;				/* Pn482 : Magnetic pole detection speed loop integral time								*/
	USHORT	pdetmaxspd;				/* Pn485 : Magnetic pole detection command speed											*/
	USHORT	pdetmaxt;				/* Pn486 : Magnetic pole detection command acceleration/deceleration time									*/
	USHORT	pdetclmpt;				/* Pn487 : Magnetic pole detection command constant speed time									*/
	USHORT	pdetwait;				/* Pn488 : Magnetic pole detection command waiting time										*/
	USHORT	pdetrepeat;				/* Pn48D : Number of magnetic pole detection repetitions										*/
	USHORT	pdetdistok;				/* Pn48E : Magnetic pole detection movable range											*/
/*------------------------------------------------------------------------------------------------------*/
	USHORT	pdetjrate;				/* Pn490 : Magnetic pole detection load level										*/
	USHORT	pdetintegless;			/* Pn491 : Magnetic pole detection integral-less gain									*/
	USHORT	pdetinteglesst;			/* Pn492 : Magnetic pole detection integral-less start time									*/
	USHORT	pdetmaxspd_r;			/* Pn493 : Magnetic pole detection command speed											*/
	USHORT	pdetdistok_r;			/* Pn494 : Magnetic pole detection movable range											*/
	USHORT	pdetmaxtrq;				/* Pn495 : Magnetic pole detection confirmation torque command									*/
	USHORT	pdetinctime;			/* Pn496 : Magnetic pole detection confirmation torque command adjustment time							*/
	USHORT	pdetconsttime;			/* Pn497 : Magnetic pole detection confirmation torque command constant time							*/
	USHORT	pdetdegreeok;			/* Pn498 : Magnetic pole detection error tolerance										*/
	USHORT	pdetrevlvspdref;		/* Pn499 : Reverse run level at magnetic pole detection speed command								*/
	USHORT	pdetrevlvwait;			/* Pn49A : Reverse running level during magnetic pole detection waiting time								*/
	USHORT	pdetoslevel;			/* Pn49B : Overspeed level when detecting magnetic pole									*/
} MPFIND_UPRM;

void	MpoleFind_CalcSpeedCommandPrm( MPFIND *MpFind, MENCV *MencV,
    	                           BPRMDAT *Bprm, MPFIND_UPRM *PdetPrm );
void	MpoleFind_CalcSpeedControlPrm( MPFIND *MpFind,
    	                           USHORT trqfil11, BPRMDAT *Bprm, MPFIND_UPRM *PdetPrm  );
void	MpoleFind_CalcErrorLevel( MPFIND *MpFind, MENCV *MencV,
    	                         MENCPRM *MencP, BPRMDAT *Bprm, MPFIND_UPRM *PdetPrm  );



#endif /* _MOT_POLE_FIND_H */
/***************************************** end of file **********************************************/

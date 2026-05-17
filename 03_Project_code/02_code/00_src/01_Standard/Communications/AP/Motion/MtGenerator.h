/****************************************************************************/
/*																			*/
/*																			*/
/*		Motion Generator Header file										*/
/*																			*/
/*																			*/
/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/************** Copyright Yaskawa Electric Corporation **********************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
#ifndef		MOTOIN_GEN_H_
#define		MOTOIN_GEN_H_

//#include "MtPRM.h"

/* defines */
#define MG_HOLD_CTRL1	0	/* Deceleration stop1(Normal parameter use) */
#define MG_HOLD_CTRL2	1	/* Deceleration stop2(Using stop parameters) */

/* motoin type */
enum MG_MTYPE {
	MNONE = 0,		/* NOPMotion */
	MINTERPOLATE,	/* INTERPOLATE */
	MPOSING,		/* POSING */ 
	MHOLD,			/* HOLD */
	MFOLLOW,		/* FOLLOW UP */
	MVELCTRL,		/* VELCTRL */
	MTRQCTRL,		/* TRQCTRL */
};



/* forward declarations */
/* Public functions */
extern VOID MtgInit(MGHNDL* hdl, MC_BASE_PRM* bPrm, MG_PRM* prm);
extern VOID MtgAttachPRM(MGHNDL* hdl, MG_PRM* prm);
extern BOOL MtgGenerate(MGHNDL* hdl, MG_INOUT* inOut);
extern LONG MtgGetDctDistance(MGHNDL* hdl, LONG spd, USHORT ctrl);
extern LONG MtgSetCspd(MGHNDL* hdl, LONG spd);
extern LONG MtgAccelerateSpd(LONG spd, LONG tspd, MGHNDL* hdl);

/* Public Motion functions */
extern BOOL MtgNone(MGHNDL* hdl, MG_INOUT* inOut);
extern BOOL MtgFollowup(MGHNDL* hdl, MG_INOUT* inOut);
extern BOOL MtgVelctrl(MGHNDL* hdl, MG_INOUT* inOut);
extern BOOL MtgInterpolate(MGHNDL* hdl, MG_INOUT* inOut);
extern BOOL MtgPosing(MGHNDL* hdl, MG_INOUT* inOut);
extern BOOL MtgHold(MGHNDL* hdl, MG_INOUT* inOut);

/* Operation function macro definition */
#define	MtgGetCspd(hdl)		((hdl)->Cspd)


#endif /* MOTOIN_GEN_H_ */

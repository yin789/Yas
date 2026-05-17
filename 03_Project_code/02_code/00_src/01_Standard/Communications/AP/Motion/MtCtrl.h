/****************************************************************************/
/*																			*/
/*																			*/
/*		Motion Control Header file										*/
/*																			*/
/*																			*/
/****************************************************************************/
/*	Histroy:																*/
/*		Rev2.01 2007.12.14	POSING							*/
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
#ifndef		MOTION_CTRL_H_
#define		MOTION_CTRL_H_

//#include "DataType.h"
//#include "ComCfg.h"
//#include "System.h"
#include "MtPRM.h"
#include "MtGenerator.h"
#include "Position.h"
#include "ServoIf.h"
//#include "AccFilter.h"
#include "PRMBank.h"


/* defines */

enum MT_OTSTATE {
	OTSTAT_NORMAL=0,	/* DisableStatus */
	OTSTAT_STOP,		/* Stopping state */
	OTSTAT_WAITNORMAL,	/* OTWaiting for release */
	OTSTAT_RETRACTION,	/* Pulling back operation status <V679> */
};

#if defined( CCSW_SFTID ) && ( CCSW_SFTID == SFTID_EXIST )					/*<V717>Add:Start*/
enum MT_AMSTATE {
	AMSTAT_NORMAL=0,	/* DisableStatus */
	AMSTAT_STOP,		/* Stopping state */
	AMSTAT_WAITNORMAL,	/* OTWaiting for release */
};
#endif												/*<V717>Add:End  */


#define MTLT_NO_SEQ_REQ	0	/* No latch sequence */
#define MTLT_SEQ_REQ	1	/* Latch sequence activation status */
#define MTLT_SEQ_DONE	2	/* Latch sequence completion status */

#define MTLT_CMP_STS	0x11	/* Latch completion state */


/* forward declarations */
/* Public functions */
extern INT32 MtcReqLatch(MTHNDL* hdl, UCHAR mode, UCHAR sgnl);
extern VOID MtcCancelLatch(MTHNDL* hdl, UCHAR mode);

/* Operation function macro definition */

/*--------------------------------------------------------------------------------------------------*/
/* Latch monitor numberTable <V715> 																	*/
/*--------------------------------------------------------------------------------------------------*/
extern UCHAR LtMonTbl[];


#endif /* MOTION_CTRL_H_ */

/************************************************************************************/
/*																					*/
/*																					*/
/*		AlarmIDDefinition Header file												*/
/*																					*/
/*																					*/
/************************************************************************************/
/*																					*/
/*																					*/
/*																					*/
/*																					*/
/*																					*/
/*																					*/
/************** Copyright Yaskawa Electric Corporation ******************************/
/*																					*/
/*																					*/
/*																					*/
/*																					*/
/************************************************************************************/
#ifndef		ERR_NO_H_
#define		ERR_NO_H_

//#include "XlsAlmDef.h"

/* defines */
#define ALM_ID_MASK	0xFF		/* AlarmIDMask 							*/

#define ERR_NONE		0x0
#define ERR_WAIT_COMP	0x1

enum ERROR_NO_ {
	ERR_COM_PRM = (LONG)0x80000000,	/* A.04A : Parameter setting error					*//*<S0C7>*/
	ERR_COM_SCAN,					/* A.BFB : Processing time exceeded(ScanN2)				*/
	ERR_COM_LSISET,					/* A.B6A : JL-080Initialization failure						*/
	ERR_COM_LSISYS,					/* A.B6B : JL-080Internal system error				*/
	ERR_COM_TCYC,					/* A.E40 : MECHATROLINKTransmission cycle setting error			*/
	ERR_COM_WDC,					/* A.E50 : MECHATROLINK WDTAbnormal					*/
	ERR_SYNC_TOUT,					/* A.E51 : MECHATROLINK WDTAbnormal(Synchronization incomplete)		*/
	ERR_COM_RCVSTP,					/* A.E60 : MECHATROLINKCommunication error					*/
	ERR_GBL_SYNC,					/* A.E61 : MECHATROLINKTransmission cycle deviation				*/
	ERR_SYN_REQ,					/* A.EA0 : Servopack failure						*/
	ERR_SYN_FAIL,					/* A.EA1 : Servo initial access error				*/
	ERR_SV_WDC,						/* A.EA2 : ServoWDCAbnormal						*/
	ERR_SV_NOANS,					/* A.ED0 : Servo not responding							*/
	ERR_CMD_FAIL,					/* A.ED1 : Command execution incomplete						*/

	ERR_COM_RCV,					/* A.960 : MECHATROLINKCommunication warning					*/
	ERR_PRM_NO,						/* A.94A : Data setting warning(User constant number)		*/
	ERR_DATA_SET,					/* A.94B : Data setting warning(Out of data range)			*/
	ERR_DATA_CALC,					/* A.94C : Data setting warning(Operation error)				*/
	ERR_DATA_SZ,					/* A.94D : Data setting warning(Data size error)		*/
	ERR_DATA_LTMOD,					/* A.94E : Data setting warning(Latch mode error)		*/
	ERR_CMD_CNDTN,					/* A.95A : Command warning(Out of condition)					*/
	ERR_CMD_NOSUP,					/* A.95B : Command warning(Unsupported)				*/
	ERR_CMD_PRM,					/* A.95C : Command warning(Outside of conditions based on user constants)	*/
	ERR_CMD_CNFLCT,					/* A.95D : Command warning(Latch command interference)	*/
	ERR_SUBCMD,						/* A.95E : Command warning(Main-Subcommand interference)*/
	ERR_FUNC_CNFLCT,				/* A.95F : Command warning(Port interference)			*/
};

#endif		/*ERR_NO_H_*/


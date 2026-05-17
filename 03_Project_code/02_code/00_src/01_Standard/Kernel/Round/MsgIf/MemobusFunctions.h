#ifndef	_MEMOBUS_FUNCTIONS_H /* For reload prevention */
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MemobusFunctions.h : MemobusProcessing function definition													*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*																									*/
/*																									*/
/*	Note	:	First edition	2010.09.03	T.Kira	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
/* MEMOBUS Message	Compile switch for product information reading */
//#define _INFO_ID_READ_CHECK_	1	/* <S061>	 */	/* <S205> Delete */


/****************************************************************************************************/
/*																									*/
/*		Macro Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		MEMOBUS Message	Function code													*/
/*--------------------------------------------------------------------------------------------------*/
#define	MBUS_LBAK_08H			0x08			/* Loopback command						*/
#define	MBUS_FC_42H				0x42			/* MEMOBUS Message function code			*/
#define	MBUS_SYSDL_50H			0x50			/* MEMOBUS Firm download code			*/
#define	MBUS_RELAY_51H 			0x51			/* Relay command										*/


/*--------------------------------------------------------------------------------------------------*/
/*		MEMOBUS Message	Subfunction code												*/
/*--------------------------------------------------------------------------------------------------*/
#define	MBUS_RD_CMEM_42H		0x01			/* Memory content read (continuous)						*/
#define	MBUS_WR_CMEM_42H		0x02			/* Writing to memory (continuous)						*/
#define	MBUS_RD_NCMEM_42H		0x03			/* Memory content read (non-continuous)						*/
#define	MBUS_WR_NCMEM_42H		0x04			/* Writing to memory (non-continuous)						*/
#define	MBUS_RD_MAXSIZE_42H		0x11			/* Maximum size readout								*/
//#ifdef __INFO_ID_READ_CHECK__	/* <S205> Delete */
#define	MBUS_RD_INFO_42H		0x7f			/* Product information reading									*/	/* <S061> */	/* <S205> :0xff(Interim) ⇒ 0x7f */
												/* (Subfunction code is undefined		*/	/* <S061> */
												/* This is a provisional value.)							*/	/* <S061> */
//#endif	/* <S205> Delete */

#if 0	//FirmDownload.h
/*--------------------------------------------------------------------------------------------------*/
/*		MEMOBUS Firm download	Subfunction code									*/
/*--------------------------------------------------------------------------------------------------*/
#define	MBUS_DL_ID_RD			0x0000			/* IDReading information										*/
#define	MBUS_DL_SWVER_RD		0x0001			/* Version read										*/
#define	MBUS_DL_WAY_RD			0x0002			/* Program write condition acquisition								*/
#define	MBUS_DL_START			0x0003			/* Download start command									*/
#define	MBUS_DL_PRGDEL			0x0004			/* Block erase command										*/
#define	MBUS_DL_PRGWR			0x0005			/* Program write command									*/
#define	MBUS_DL_DELSND			0x0006			/* Block erase information command(Erase information transfer only.Do not erase)		*/
#define	MBUS_DL_PRGSND			0x0007			/* Program transmission command(Program transmission only.Do not write)		*/
#define	MBUS_DL_PRGDELA			0x0008			/* Block bulk erase command									*/
#define	MBUS_DL_PRGWRA			0x0009			/* Program batch write command								*/
#define	MBUS_DL_END				0x000A			/* Download end command									*/
#define	MBUS_DL_PRGRD			0x000B			/* Program read command									*/
#define	MBUS_DL_REBOOT			0x000C			/* Program restart command									*/
#define	MBUS_DL_SIFCHG			0x000D			/* Serial IF speed change									*/
#define	MBUS_DL_HIS_RD			0x000E			/* Read download history								*/
#define	MBUS_DL_HIS_WR			0x000F			/* Write download history								*/
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Error code definition																			*/
/*--------------------------------------------------------------------------------------------------*/
#define	FUNCCODE_ERROR			0x01			/* Function code error						*/
#define	MEMADRES_ERROR			0x02			/* Memory address error								*/
#define	MODETYPE_ERROR			0x03			/* Mode/data type error							*/
#define	DATANUMB_ERROR			0x04			/* Quantity defect											*/
#define	LMITACCS_ERROR			0x05			/* Access restriction error								*/
#define	RANGOUTS_ERROR			0x06			/* Setting value out of range error								*/
#define	DATAADJT_ERROR			0x07			/* Data consistency error									*/
#define	CONDTION_ERROR			0x08			/* Condition error										*/
#define	COMPETON_ERROR			0x09			/* Processing conflict error									*/
#define SELAXIS_ERROR			0x10			/* Axis specification error										*/
/*--------------------------------------------------------------------------------------------------*/
#define MBUS_HEAD_BASE_SIZE		0x0C			/* MEMOBUSHeader basic size(Byte)Includes address		*/
#define MBUS_HEAD_FIX_SIZE		0x08			/* MEMOBUSHeader fixed size(Byte)					*/
/*--------------------------------------------------------------------------------------------------*/
#define MBUS_RLY_FIX_SIZE		0x0E			/* Relay command fixed size							*/
#define MBUS_RLY_PASS_DATA_SIZE	0x08			/* Relay command variable size(varies in multiples of this size)	*/
#define MBUS_RLY_MAX_ROUTENUM	0x0A			/* Maximum number of relay stages									*/
/*--------------------------------------------------------------------------------------------------*/
//#define __EXAD_CHECK__
//#define __MYAD_CHECK__
//#define __RCVLEN_CHECK__

/* <S061>	Start */
//#ifdef __INFO_ID_READ_CHECK__	/* <S205> Delete */
/*----------------------------------------------------------------------------------------------------------*/
/*		MEMOBUS Message	Product information reading Address information													*/
/*----------------------------------------------------------------------------------------------------------*/
#define MBUS_IDRD_HEAD_BASE_SIZE		0x0C		/* MEMOBUS(Product information reading) Header basic size(Byte)		*/
#define MBUS_IDRD_RSP_HEAD_BASE_SIZE	0x0D		/* MEMOBUS(Product information reading) Response header basic size(Byte)	*/
#define MBUS_IDRD_SLOT_NUM				6			/* MEMOBUS(Product information reading) Number of slots					*/
#define MBUS_IDRD_ID_NUM				7			/* MEMOBUS(Product information reading) IDNumber						*/

/*----------------------------------------------------------------------------------------------------------*/
/*		Operator parameter display selection Confirm																	*/
/*----------------------------------------------------------------------------------------------------------*/
#define OPEDSP_SETUPPRM		0x0000		/* SETUP									*/
#define OPEDSP_ALLPRM		0x0001		/* All user-defined parameters											*/
#define	SETUP_PRM_MASK		0x80		/* Setup parameter mask										*/
#define WRITE_NG			0x0000		/* WriteOK														*/
#define WRITE_OK			0x0001		/* WriteNG														*/
//#endif	/* <S205> Delete */
/* <S061>	End */

/****************************************************************************************************/
/*		STRUCT DEFINITION																			*/
/****************************************************************************************************/



/****************************************************************************************************/
/*		EXTERN LIST																					*/
/****************************************************************************************************/





/***************************************** end of file **********************************************/
#define _MEMOBUS_FUNCTIONS_H /* For reload prevention */
#endif /* _MEMOBUS_FUNCTIONS_H */


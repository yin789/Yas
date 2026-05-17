/****************************************************************************************************/
/*																									*/
/*																									*/
/*		XlsTrcDef.h : Trace definition header file													*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1. Trace definition structure																		*/
/*																									*/
/*		2. Trace definition tableExternDeclaration															*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		2010.02.12 : Made by Excel Macro															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"




/****************************************************************************************************/
/*																									*/
/*		Trace definition structure																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Numerical trace definition structure																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct  {
//		UCHAR	SelNo;								/* SelectionNo										*/	/* <S17A> */
		USHORT	SelNo;								/* SelectionNo										*/	/* <S17A> */
//		UCHAR	VxType;								/* Variable types									*/
		UCHAR	VxSftL;								/* Trace variable left shift							*/
		UCHAR	VxSftR;								/* Trace variable right shift							*/
		BOOL	AvrgSel;							/* Trace averaging selection:TRUE⇒Average			*/	
		LONG	(*GetVarData)( void* Axis );		/* Trace variable address acquisition function					*/
		LONG	(*GetVarGain)( void* Axis );		/* Trace variable gain calculation function					*/
} TVXDEF;
/*--------------------------------------------------------------------------------------------------*/
#define XTRCDEF_TxSHORT			0x02				/* Type:SHORT									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Bit trace definition structure																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct  {
		UCHAR	SelNo;								/* SelectionNo										*/
//		UCHAR	BitNo;								/* BitNo.									*/
		BOOL	(*GetBitData)( void* Axis );		/* Trace variable address acquisition function					*/
} TBXDEF;
/*--------------------------------------------------------------------------------------------------*/
/*		TVXDEF/TBXDEF Version																		*/
/*--------------------------------------------------------------------------------------------------*/
#define TVXDEF_VERSION			0x0001				/* TVXDEF Version								*/
#define TBXDEF_VERSION			0x0001				/* TBXDEF Version								*/
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Trace definition tableExternDeclaration																*/
/*																									*/
/****************************************************************************************************/
extern  const TVXDEF TrcVarTbl[];					/* Numeric trace definition table						*/
extern  const TBXDEF TrcBitTbl[];					/* Bit trace definition table					*/
extern  const USHORT TrcVarTblEntNum;				/* Number of registered numerical trace definition tables				*/
extern  const USHORT TrcBitTblEntNum;				/* Number of registered bit trace definition tables			*/
//extern  const TBXDEF TrcBitTblUser[];				/* Bit trace definition table					*/
/*--------------------------------------------------------------------------------------------------*/
extern  const USHORT TrcVarTblUserEntNum;			/* Number of registered numerical trace definition tables				*/
extern  const USHORT TrcBitTblUserEntNum;			/* Number of registered bit trace definition tables			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Define Entry Number																			*/
/*--------------------------------------------------------------------------------------------------*/
#if		OPENSW_TRCDEF == 0
#define TRCVARTBL_ENTNUM	TrcVarTblEntNum
#define TRCBITTBL_ENTNUM	TrcBitTblEntNum
#else
#define TRCVARTBL_ENTNUM	(TrcVarTblEntNum + TrcVarTblUserEntNum)
#define TRCBITTBL_ENTNUM	(TrcBitTblEntNum + TrcBitTblUserEntNum)
#endif
/*--------------------------------------------------------------------------------------------------*/







/***************************************** end of file **********************************************/

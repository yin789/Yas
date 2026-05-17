/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Function.h : Function Management Module														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1) Function Data Type Definition															*/
/*																									*/
/*		2) Function Resource Macro Definition														*/
/*																									*/
/*		3) 																							*/
/*																									*/
/*		4) 																							*/
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
/*		Rev.1.00 : 2013.5.08  K.Ozaki																*/
/*		Rev.1.10 : 2013.9.26  K.Ozaki	<S00C>														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#ifndef ___FUNCTION___
#define  ___FUNCTION___
#include "KnlApi.h"
#include "Object.h"
#include "FunListDef.h"

/****************************************************************************************************/
/*																									*/
/*		Macro Definition																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/* 			ONLY IN MERCURY 																		*/
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Get Function Table Pointer																	*/
/*--------------------------------------------------------------------------------------------------*/
#define GET_FUNTBL( p )             (CFUNTBL *)((p)->pFunTbl)

/*--------------------------------------------------------------------------------------------------*/
/*		Get Object Definition																		*/
/*--------------------------------------------------------------------------------------------------*/
#define GET_FUNDEF( p, idx )		(((CFUNTBL *)(p))[(idx)].FunDefAttr)

/*--------------------------------------------------------------------------------------------------*/
/*		Get Object Access Level																		*/
/*--------------------------------------------------------------------------------------------------*/
#define GET_FNACCLVL( fndef )		((fndef)->AccessLevel)

/*--------------------------------------------------------------------------------------------------*/
/*		Get Object Name 																			*/
/*--------------------------------------------------------------------------------------------------*/
#define GET_FNFUNNAME( fndef )		((fndef)->SName)



/****************************************************************************************************/
/*																									*/
/*		Function Access Class Definition															*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Auxiliary function execution structure																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct _CFunction{					/* Function Access Class Definition						*/
	USHORT	FnTblEntNum;					/* Function Table Entry Number							*/
	USHORT  Reserved;						/* Reserved 											*/
/*--------------------------------------------------------------------------------------------------*/
	CFUNTBL	*pFunTbl;						/* Pointer to the Function Table						*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*Read)(  						/* Read Method ( by using DefPtr ) 						*/
				struct _CObject *this,		/* this Pointer 										*/
				COBJDEF *pObjDef,  			/* Pointer to the Parameter Defitnion 					*/
				ULONG *pResData,			/* Pointer to the Read Value 							*/
				ULONG CheckType,			/* Error Check Type 									*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*ReadX)(						/* Read Method ( by using Index ) 						*/
				struct _CObject *this,		/* this Pointer 										*/
				LONG Index,					/* Monitor Table Index 									*/
				ULONG *pResData,			/* Pointer to the Read Value 							*/
				ULONG CheckType,			/* Error Check Type 									*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*Search)(						/* Index Search Method									*/
				struct _CFunction *this,	/* this Pointer 										*/
				LONG	SrchNo,				/* Search Number										*/
				LONG	BaseIdx,			/* Search Base Index									*/
				LONG	*pFindIdx,			/* Pointer to the Found Index							*/
				ULONG	CheckType,			/* Error Check Type										*/
				MSGIF	*hMsg			);	/* Message Interface									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*SearchB)(						/* Index Search Method ( by using Base Index ) 			*/
				struct _CFunction *this,	/* this Pointer 										*/
				LONG SrchNo,				/* Number of Un Monitor 								*/
				LONG BaseIdx,				/* Start Index of Searching 							*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*ReadArray)(					/* Read Array Object ( by using Index ) 				*/
				struct _CObject *this,		/* this Pointer 										*/
				LONG Index,					/* Index of the Object 									*/
				LONG ArrayIndex,			/* Index of the Array 									*/
				LONG ReadSize,				/* Size of the Array 									*/
				ULONG *pResData,			/* Pointer to the Read Value 							*/
				ULONG CheckType,			/* Error Check Type 									*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*Write)(  						/* Write Method 										*/
				struct _CObject *this,		/* this Pointer 										*/
				LONG Index,  				/* Index of the Object Defitnion 						*/
				ULONG Data,					/* Write Value 											*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*GetNumber)(					/* Get Object Number from Object Index					*/
				struct _CFunction *this,	/* this Pointer 										*/
				LONG Index,  				/* Index of the Object Defitnion 						*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*isLong)(						/* Check Object Size is Long or Not						*/
				struct _CFunction *this,	/* this Pointer 										*/
				LONG Index,  				/* Index of the Object Defitnion 						*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*SearchS)(						/* <S073> Index Search witch Sub Index 					*/
				struct _CObject *this,		/* this Pointer 										*/
				LONG	SrchNo,				/* Search Number										*/
				LONG	BaseIdx,			/* Search Base Index									*/
				LONG	*pFindIdx,			/* Pointer to the Found Index							*/
				LONG	*pFindSubIdx,		/* Pointer to the Found Sub Index						*/
				ULONG	CheckType,			/* Error Check Type										*/
				MSGIF	*hMsg			);	/* Message Interface									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*SearchBS)(					/* <S073> Next Index Search Method 						*/
				struct _CObject *this,		/* this Pointer 										*/
				LONG 	SrchNo,				/* Number of Un Monitor 								*/
				LONG 	BaseIdx,			/* Start Index of Searching 							*/
				LONG 	BaseSubIdx,			/* Start Sub Index of Searching 						*/
				LONG 	*pFindSubIdx,		/* Pointer to the Found Sub Index						*/
				MSGIF 	*hMsg );			/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*WriteArray)(					/* <S073> Write Array Data Method 						*/
				struct _CObject *this,		/* this Pointer 										*/
				LONG Index,  				/* Index of the Object Defitnion 						*/
				LONG ArrayIndex,			/* Index of the Array 									*/
				LONG WriteSize,				/* Size of the Array 									*/
				ULONG *pWrData,				/* Pointer to the Write Value 							*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
/*	extended																						*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*Next)(						/* Next Index Search Method								*/
				struct _CFunction *this,	/* this Pointer 										*/
				LONG BaseIdx,				/* Start Index of Searching 							*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*Prev)(						/* Next Index Search Method								*/
				struct _CFunction *this,	/* this Pointer 										*/
				LONG BaseIdx,				/* Start Index of Searching 							*/
				MSGIF *hMsg );				/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*Begin)(						/* Begin Function										*/
				struct _CFunction *this,	/* this Pointer 										*/
				LONG	Index,				/* Auxiliary function management table index					*/
				UINT 	*FLcdKeyMk,			/* LCDKeyMAKE(OFF->ON)Flag							*/
				UINT 	*FLcdKeyBrk,		/* LCDKeyBREAK(ON->OFF)Flag							*/
				UINT 	*FLcdKeyLvl,		/* LCDKey level flag									*/
				UINT 	*FOpBlnkKickFlg,	/* LCDFlag for blinking										*/
				UCHAR	*FTxBuff,			/* LCDOperator display information								*/
				HMSGIF	Hmsg );				/* Using source identification handle									*/
/*--------------------------------------------------------------------------------------------------*/
	LONG	(*Exec)(						/* Execute Function										*/
				struct _CFunction *this,	/* this Pointer 										*/
				HMSGIF Hmsg );				/* Message Interface									*/
/*--------------------------------------------------------------------------------------------------*/
	void	(*Finish)(						/* Finish Function										*/
				struct _CFunction *this,	/* this Pointer 										*/
				HMSGIF Hmsg );				/* Message Interface									*/
/*--------------------------------------------------------------------------------------------------*/
} CFUNCTION;

/*--------------------------------------------------------------------------------------------------*/
#define CFUN_ENT_NUM (1)					/* Function Instance Entry Number 						*/
extern	CFUNCTION CFunction[];				/* Function Access Class Instance						*/
/*--------------------------------------------------------------------------------------------------*/
#define FNFUNTBL_ENTNUM   (FnFunTblEntNum)
#define FNFUNTBL_PTR      (FnFunTbl)
/*--------------------------------------------------------------------------------------------------*/
#define CLCDFUN (&CFunction[0])				/* LCD Operator Function Access Class 					*/
#define CPNLFUN (CLCDFUN)					/* Panel Operator Function Access Class					*/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Auxiliary function management IF definition(RndRpx.h)																*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Servo Function Check Flag																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	FCHK_NONE			0x00
#define	FCHK_ACCS_LVL		0x01
#define	FCHK_AMP_TYPE		0x02
#define	FCHK_PWINH			0x04
#define	FCHK_MTR_TYPE		0x08
#define	FCHK_ALL			0xFF
#define	FCHK_DEF			FCHK_MTR_TYPE | FCHK_ACCS_LVL
/*--------------------------------------------------------------------------------------------------*/
/*		Servo Function Object Return Code															*/
/*--------------------------------------------------------------------------------------------------*/
#define	FERR_FOUND			0x0001
#define	FERR_FULL			0x0002
#define	FERR_DATA			0x0003
#define	FERR_LEVEL			0x0004
#define	FERR_AMP_TYPE		0x0005
#define	FERR_INHIBIT		0x0006
#define	FERR_MTR_TYPE		0x0007

/****************************************************************************************************/
/*																									*/
/*		Function Prototypes																			*/
/*																									*/
/****************************************************************************************************/
void	RpxInitFunManager( void );			/* Auxiliary function management initialization process								*/
void	RpxPrstFunManager( void );			/* Auxiliary function management parameter reset processing					*/

/****************************************************************************************************/
/*																									*/
/*		Fn auxiliary function management API definition(RndRpx.h)														*/
/*																									*/
/****************************************************************************************************/
#define FEXE_LCDBUF_SIZE	114					/* LcdDspBuff[] Size Definition						*/

/*--------------------------------------------------------------------------------------------------*/
/*	Operator Application Execution State 															*/
/*--------------------------------------------------------------------------------------------------*/
#define OPEAPP_INIT		  0						/* Before execution											*/
#define OPEAPP_EXEC		  1						/* Executing											*/
#define OPEAPP_END		  2						/* End execution											*/

/*--------------------------------------------------------------------------------------------------*/
/*	Servo Function Reference Data																	*/
/*--------------------------------------------------------------------------------------------------*/
#define FREFER_INIT		  0						/* Before execution											*/
#define FREFER_EXEC		  1						/* Executing											*/
#define FREFER_END		  2						/* End execution											*/

/*--------------------------------------------------------------------------------------------------*/
/*	Servo Function Command Code																		*/
/*--------------------------------------------------------------------------------------------------*/
//#define FCMD_NONE		0x00					/* No instruction											*/
#define FCMD_NONE		0xFF					/* No instruction											*//* <S14E> */
#define FCMD_EXEC		0x01					/* Execution												*/
#define FCMD_UP			0x02					/* UP,Forward rotation,forward direction									*/
#define FCMD_DOWN		0x03					/* DOWN,Reverse running,Reverse direction									*/
#define FCMD_SV			0x04					/* Servo on/Off									*/
#define FCMD_CH1		0x05					/* CH1,UPhase selection										*/
#define FCMD_CH2		0x06					/* CH2,VPhase selection										*/
#define FCMD_EWRT		0x07					/* EEPROMWrite process									*/
//#define FCMD_STOP		0x08					/* Stop												*/
#define FCMD_STOP		0x00					/* Stop												*//* <S14E> */
#define FCMD_PMODE		0x09					/* LCD MODE/SETPress									*/
#define FCMD_LMODE		0x0A					/* Panel MODE/SETPress								*/
#define FCMD_PDATA		0x0B					/* LCD DATAPress										*/
#define FCMD_LDATA		0x0C					/* Panel DATAPress									*/
#define FCMD_RSET		0x0D					/* Reset											*/
#define FCMD_PSHUP		0x0E					/* Panel Shift up								*/
#define FCMD_LSHUP		0x0F					/* LCD Shift up									*/
#define FCMD_SHDOWN		0x10					/* Shift down										*/
#define FCMD_ALMRST		0x11					/* Alarm reset									*/
#define FCMD_MTUP		0x12					/* Operation mode maintained(Forward rotation)								*/
#define FCMD_MTDOWN		0x13					/* Operation mode maintained(Reverse running)								*/
#define FCMD_SCROLL		0x14					/* Scroll										*/
#define FCMD_READ		0x15					/* READ												*/
#define FCMD_WRITE		0x16					/* WRITE											*/
#define FCMD_SGNONE		0x17					/* SigmaMsgNo command								*/
//#define FCMD_EXEC2		0x18					/* Auxiliary execution											*/
#define FCMD_EXEC2		0x08					/* Auxiliary execution											*//* <S14E> */
//#define FCMD_MTEXEC		0x19					/* Operation mode maintained(Execution)								*/
#define FCMD_MTEXEC		0x11					/* Operation mode maintained(Execution)								*//* <S14E> */
#define FCMD_HIWND		0x1A					/* High-speed winding selection										*/
#define FCMD_LOWND		0x1B					/* Low-speed winding selection										*/
#if 0 /* MessageSharing definitions with communication(Commented out because the operation has not been confirmed) */
//#define FCMD_STOP		0x08					/* Stop												*/
#define FCMD_STOP		0x00					/* Stop												*/
#define FCMD_EXEC		0x01					/* Execution												*/
#define FCMD_UP			0x02					/* UP,Forward rotation,forward direction									*/
#define FCMD_DOWN		0x03					/* DOWN,Reverse running,Reverse direction									*/
#define FCMD_SV			0x04					/* Servo on/Off									*/
#define FCMD_CH1		0x05					/* CH1,UPhase selection										*/
#define FCMD_CH2		0x06					/* CH2,VPhase selection										*/
#define FCMD_EWRT		0x07					/* EEPROMWrite process									*/
//#define FCMD_EXEC2	0x18					/* Auxiliary execution											*/
#define FCMD_EXEC2		0x08					/* Auxiliary execution											*/
#define FCMD_PMODE		0x09					/* LCD MODE/SETPress									*/
#define FCMD_LMODE		0x0A					/* Panel MODE/SETPress								*/
#define FCMD_PDATA		0x0B					/* LCD DATAPress										*/
#define FCMD_LDATA		0x0C					/* Panel DATAPress									*/
#define FCMD_RSET		0x0D					/* Reset											*/
#define FCMD_PSHUP		0x0E					/* Panel Shift up								*/
#define FCMD_LSHUP		0x0F					/* LCD Shift up									*/
#define FCMD_SHDOWN		0x10					/* Shift down										*/
//#define FCMD_MTEXEC	0x19					/* Operation mode maintained(Execution)								*/
#define FCMD_MTEXEC		0x11					/* Operation mode maintained(Execution)								*/
#define FCMD_MTUP		0x12					/* Operation mode maintained(Forward rotation)								*/
#define FCMD_MTDOWN		0x13					/* Operation mode maintained(Reverse running)								*/
#define FCMD_SCROLL		0x14					/* Scroll										*/
#define FCMD_READ		0x15					/* READ												*/
#define FCMD_WRITE		0x16					/* WRITE											*/
#define FCMD_SGNONE		0x17					/* SigmaMsgNo command								*/
//#define FCMD_ALMRST	0x11					/* Alarm reset									*/
#define FCMD_ALMRST		0x18					/* Alarm reset									*/
#define FCMD_HIWND		0x1A					/* High-speed winding selection										*/
#define FCMD_LOWND		0x1B					/* Low-speed winding selection										*/
//#define FCMD_NONE		0x00					/* No instruction											*/
#define FCMD_NONE		0xFF					/* No instruction											*/
#endif

/*--------------------------------------------------------------------------------------------------*/
/*	Servo Function End Method																		*/
/*--------------------------------------------------------------------------------------------------*/
#define FNORMAL_END		0x00
#define FFORCE_END		0x01

/*--------------------------------------------------------------------------------------------------*/
/*		Data write prohibitionIFDefinition																		*/
/*		Note: Data write management structure(RNDWRTD)Must be arranged according to member						*/
/*--------------------------------------------------------------------------------------------------*/
#define FPRMOPE_IF		0x01					/* Operator writeIF					[0bitth]	*/
#define FPRMREG_IF		0x02					/* Parameter register writingIF			[1bitth]	*/
#define FOPEREG_IF		0x04					/* Write general operation registerIF				[2bitth]	*/
												/* (2000H, 2001H, 2002H )						*/
#define FALLOPEREG_IF	0x08					/* Write all operation registersIF				[3bitth]	*/
#define FENGREG_IF		0x10					/* Engineering register writeIF		[4bitth]	*/
#define FTRCREG_IF		0x20					/* Write trace data registerIF		[5bitth]	*/
#define FSPRMREG_IF		0x40					/* Special parameter register writeIF		[6bitth]	*/

/*--------------------------------------------------------------------------------------------------*/
/*		FPRM_IF			Write user parametersIF													*/
/*		FEXCEPT_FN_IF	Register	2000H, 2001H, 2002H Writing other thanIF								*/
/*		FALL_IF			Write all dataIF														*/
/*--------------------------------------------------------------------------------------------------*/
#define FPRM_IF			FPRMOPE_IF | FPRMREG_IF | FSPRMREG_IF
#define FEXCEPT_FN_IF	FPRM_IF	| FOPEREG_IF | FENGREG_IF | FTRCREG_IF
#define FALL_IF			FEXCEPT_FN_IF | FALLOPEREG_IF

#endif
/***************************************** end of file **********************************************/

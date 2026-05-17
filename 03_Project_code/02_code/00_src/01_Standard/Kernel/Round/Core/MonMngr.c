/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MonMngr.c : Monitor Management Module														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1) Monitor data read processing																	*/
/*																									*/
/*		2) Monitor variable clearing process																		*/
/*																									*/
/*		3) Monitor table index search process														*/
/*																									*/
/*		4) Monitor register read process(Register IF)													*/
/*																									*/
/*																									*/
/*																									*/
/*	Caution: Monitor management tableUnMonTbl[] and UnRegTbl[] Please note that (Rev.1.10)		*/
/*		  Processing using table indexes must be done independently for each table! ! !					*/
/*			UnMonTbl[] : For operator IF															*/
/*			UnRegTbl[] : For register IF(SigmaMessage,etc)											*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2002.10.01  M.Kawatani															*/
/*		Rev.1.10 : 2005.03.04  T.Taniguchi															*/
/*		Rev.1.20 : 2009.06.03  T.Taniguchi															*/
/*		Rev.1.30 : 2011.04.06  K.Ozaki <0BJ14>														*/
/*		Rev.1.40 : 2013.09.26  K.Ozaki	<S00C>														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "Monitor.h"



/****************************************************************************************************/
/*																									*/
/*		Function list																					*/
/*																									*/
/****************************************************************************************************/
void	RpxInitMonManager( void );			/* Monitor Manager Initialize							*/
void	RpxPrstMonManager( void );			/* Monitor Manager Parameter Reset						*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxMonClearVarIdx(					/* Monitor variable clearing process									*/
		COBJECT *this,						/* this Pointer 										*/
		LONG	Index,						/* Monitor management tableIndex								*/
		MSGIF *hMsg );						/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Register IF																				*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
LONG	RpxReadMonRegister(					/* Monitor register read function								*/
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of read data									*/
		USHORT	*RdBuf,						/* Reading data buffer pointer						*/
		HMSGIF	Hmsg			);			/* MessageIFHandle									*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxReadMonRegister(					/* Monitor register read process(Index specification)				*/
		LONG	Index,						/* Monitor management tableIndex								*/
		ULONG	*pResData,					/* Address to store read monitor value					*/
		ULONG	CheckType,					/* Error check type									*/
		HMSGIF	Hmsg			);			/* MessageIFHandle									*/
/*--------------------------------------------------------------------------------------------------*/
LONG	RpxMonSearchRegIdx(					/* Monitor register index search process (Round)	*/
		LONG	RegNo,						/* Register number											*/
		LONG	*pFindIdx,					/* Search result index									*/
		ULONG	CheckType		);			/* Error check type									*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxMonSearchRegIdx(					/* Monitor register index search processing					*/
		LONG	RegNo,						/* Register number											*/
		LONG	*pFindIdx		);			/* Search result index									*/
/*--------------------------------------------------------------------------------------------------*/
#endif



/****************************************************************************************************/
/*																									*/
/*		Private Macro Definition																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Get UnMonitor Variable Address for Single/Multi Drive										*/
/*--------------------------------------------------------------------------------------------------*/
#ifndef CFG_GET_AXIS_VAR_ADR
#define LPX_GET_UNVAR_ADR( pVar )		(pVar)
#else
#define LPX_GET_UNVAR_ADR( pVar )		CFG_GET_AXIS_VAR_ADR( (pVar) )
#endif
/*--------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*		Get Un Monitor Number	 																	*/
/*--------------------------------------------------------------------------------------------------*/
#define GET_UNMONNUMBER( tbl, idx )  ( (tbl)[(idx)].Number )


/****************************************************************************************************/
/*																									*/
/*		Monitor management variable definition																		*/
/*																									*/
/****************************************************************************************************/
CMONITOR LMonitor[MONITOR_NUMBERX];				/* Monitor Accesser Instance 						*/
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Dummy																						*/
/*																									*/
/****************************************************************************************************/
LONG	LpxMonWriteDummy( void *this, LONG Index, ULONG data, MSGIF *hMsg ) { return OK; }



/****************************************************************************************************/
/*																									*/
/*		Monitor management initialization processing																		*/
/*																									*/
/****************************************************************************************************/
void	RpxInitMonManager( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Set	Table Entry Number																		*/
/*--------------------------------------------------------------------------------------------------*/
		CUNMONX->UnTblEntNum = UNMONTBL_ENTNUM;		    /* Set UnMonTbl Entry Number				*/
		CUNMONX->pUnMonTbl = UNMONTBL_PTR;				/* Set UnMonTbl Pointer						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Set	Monitor Access Method 																	*/
/*--------------------------------------------------------------------------------------------------*/
		CUNMONX->Read  = RpxObjRead; 					/* Read UnMonitor */
//		CUNMONX->ReadX = RpxObjReadIdx; 				/* Read UnMonitor */
		CUNMONX->ReadS  = RpxObjReadS; 					/* <S073> Read UnMonitor */
		CUNMONX->Search = RpxObjSearch; 				/* Search Index (Binary Search) */
		CUNMONX->SearchB = RpxObjSearchIdx; 			/* Search Index (Linear Search) */
		CUNMONX->ReadArray = RpxObjReadArray;			/* Read UnMonitor(Array) */
		CUNMONX->Write = LpxMonWriteDummy;				/* Dummy Function */
		CUNMONX->WriteS = LpxMonWriteDummy;				/* <S073> Dummy Function */
/*--------------------------------------------------------------------------------------------------*/
		CUNMONX->Clear = RpxMonClearVarIdx; 			/* Clear Monitor */
/*--------------------------------------------------------------------------------------------------*/
		CUNMONX->GetNumber = RpxObjGetNumber;			/* Get Object Number */
		CUNMONX->isLong = RpxObjChkLong;				/* Check Object is Long */
/*--------------------------------------------------------------------------------------------------*/
		CUNMONX->SearchS = RpxObjSearchSub;				/* <S073> */
		CUNMONX->SearchBS = RpxObjSearchIdxSub;			/* <S073> */
		CUNMONX->WriteArray = RpxObjWriteArray;			/* <S073> */
#if MONITOR_NUMBERX >= 2 
/*--------------------------------------------------------------------------------------------------*/
/*		Set	Table Entry Number																		*/
/*--------------------------------------------------------------------------------------------------*/
		CUNREGX->UnTblEntNum = UNREGTBL_ENTNUM;		    /* Set UnRegTbl Entry Number				*/
		CUNREGX->pUnMonTbl = UNREGTBL_PTR;				/* Set UnRegTbl Pointer						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Set	Monitor Access Method 																	*/
/*--------------------------------------------------------------------------------------------------*/
		CUNREGX->Read  = RpxObjRead; 					/* Read UnRegister */
//		CUNREGX->ReadX = RpxObjReadIdx; 				/* Read UnRegister */
		CUNREGX->ReadS  = RpxObjReadS; 					/* <S073> Read UnRegister */
		CUNREGX->Search = RpxObjSearch; 				/* Search Index */
		CUNREGX->SearchB = RpxObjSearchIdx;				/* Search Index */
		CUNREGX->ReadArray = RpxObjReadArray;			/* Read UnRegister(Array) */
		CUNREGX->Write = LpxMonWriteDummy;				/* Dummy Function */
		CUNREGX->WriteS = LpxMonWriteDummy;				/* <S073> Dummy Function */
		CUNREGX->Clear = RpxMonClearVarIdx; 			/* Clear Monitor */
/*--------------------------------------------------------------------------------------------------*/
		CUNREGX->GetNumber = RpxObjGetNumber;			/* Get Object Number */
		CUNREGX->isLong = RpxObjChkLong;				/* Check Object is Long */
/*--------------------------------------------------------------------------------------------------*/
		CUNREGX->SearchS = RpxObjSearchSub;				/* <S073> */
		CUNREGX->SearchBS = RpxObjSearchIdxSub;			/* <S073> */
		CUNREGX->WriteArray = RpxObjWriteArray;			/* <S073> */
#endif
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Monitor management parameter reset processing															*/
/*																									*/
/****************************************************************************************************/
void	RpxPrstMonManager( void )
{
		return;
}


/****************************************************************************************************/
/*																									*/
/*		Monitor variable clearing process																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Clear the monitor variable of the specified index to zero.											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	RpxMonClearVarIdx(
		COBJECT *this,							/* this Pointer */
		LONG	Index,							/* Monitor management tableIndex							*/
		MSGIF *hMsg )							/* Message Interface */
{
#if 0	/* <S144> */
LONG	rc;
CMONDEF	*pUnDef = GET_UNMONDEF( this->pTbl, Index ) 	/* UnMonTbl[Index].MonDefPtr */; 
/*--------------------------------------------------------------------------------------------------*/
/*		Check Clear Enable																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( CHK_UNCLEAR_ENABLE( pUnDef ) )
		{
			return;
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Checking monitor target CPU																	*/
/*--------------------------------------------------------------------------------------------------*/
#if		CFG_MASTER_CPU == 1
		if( pUnDef->MtcIsComIf )
		{
			rc = DpiDegOpClearUnMonData( UnMonTbl[Index].UnNo );
			return;
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		@@@ This process is NG : If you change the Excel table, it will not work properly. @@@							*/
/*--------------------------------------------------------------------------------------------------*/
//		if( Index == 0x0012 )			// Un013is cleared, clear the remainder used
//		{										/* If you change the Excel table,IndexThe value changes			*/
//			CoutRrst.RemCntr2 = 0;				/* such bugs are difficult to find				*/
//		}										/* IndexSoftware that directly uses numbers cannot be reused.	*/
/*--------------------------------------------------------------------------------------------------*/
/*		Clear Moniter Data																			*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /* TODO: Mercury  */
		if( pUnDef->ClrVarFun )
		{
			pUnDef->FuncPtr( UNCAL_CLRVAR, pUnDef, NULL );
		}
		else if( pUnDef->IsLong )
		{
			*(ULONG *)LPX_GET_UNVAR_ADR( pUnDef->UnVar.Pulong ) = 0;
		}
		else
		{
			*(USHORT *)LPX_GET_UNVAR_ADR( pUnDef->UnVar.Pushort ) = 0;
		}
#endif
#else
LONG	pVal;
void	*ptr;
CMONDEF	*pUnDef = GET_UNMONDEF( this->pTbl, Index ) 	/* UnMonTbl[Index].MonDefPtr */; 
/*--------------------------------------------------------------------------------------------------*/
		if( CHK_UNCLEAR_ENABLE( pUnDef ) )
		{
			if( NULL != pUnDef->GetRamPtrCallback )
			{
				ptr = pUnDef->GetRamPtrCallback( Index, hMsg->pAxRsc );
				if( pUnDef->DataLength == 4 )
				{
					*(ULONG  *)(ptr) = 0;
				}
				else if( pUnDef->DataLength == 2 )
				{
					*(USHORT *)(ptr) = 0;
				}
				else if( pUnDef->DataLength == 1 )
				{
					*(UCHAR  *)(ptr) = 0;
				}
			}
			else
			{
				if( NULL != pUnDef->PrmCalCallback )
				{
					pUnDef->PrmCalCallback( PRM_ACCCMD_WRITE, Index, hMsg->pAxRsc, &pVal );
				}
			}
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Monitor table index search process														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Search the monitor table index from the specified monitor number.						*/
/*			<0BJ14> Changed the process to search using a binary search. If it is not found,*pFindIdx=0becomes	*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	RpiMonSearch(						/* Return value : Status									*/
		COBJECT *this,						/* this Pointer 										*/
		LONG	SrchNo,						/* Monitor number											*/
		LONG	BaseIdx,					/* Search start index									*/
		LONG	*pFindIdx,					/* Search result index									*/
		ULONG	CheckType,					/* Error check type									*/
		HMSGIF	Hmsg		)				/* MessageIFHandle									*/
{
CMONTBL *pUnMonTbl = GET_UNMONTBL( this );
	if ( LpxSearchUnMonTblIdx( 
						pUnMonTbl /* &UnMonTbl[0] */, 
						this->TblEntNum  /* MonMngP.UnMonTblEntNum */, 
						SrchNo, 
						pFindIdx ) == OK ) 
	{
/*--------------------------------------------------------------------------------------------------*/
/*		Index check																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( *pFindIdx >= this->UnTblEntNum /* (LONG)MonMngP.UnMonTblEntNum */ )
		{
			;				/* Monitor number error										*/
		}
		else
/*--------------------------------------------------------------------------------------------------*/
		if ( LpxMonCheckAttrIdx( GET_UNMONDEF( pUnMonTbl, *pFindIdx ), *pFindIdx, CheckType, Hmsg ) == OK ) {
			return (OK);
		}
	}
	*pFindIdx = 0; 
	return (NG); 								/* return (MERR_NO); */
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Monitor table index search process														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Search the monitor table index from the specified monitor number.						*/
/*			If the applicable number is not found, select the next one that meets the conditionsIndex.							*/
/*			<0BJ14> Code organization.																	*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0
USHORT	RpxMonSearchIdxOp(					/* Return value : Search result index							*/
		COBJECT *this,						/* this Pointer 										*/
		LONG	SrchNo,						/* Monitor number											*/
		LONG	BaseIdx,					/* Search start index									*/
		HMSGIF	Hmsg		)				/* MessageIFHandle									*/
{
	return (USHORT)LpxSearchUnMonTblNextIdx( 
					GET_UNMONTBL( this ) /* &UnMonTbl[0] */, 
					this->UnTblEntNum,
					SrchNo, 
					BaseIdx, 
					Hmsg );
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Monitor management table search																		*/
/*																									*/
/****************************************************************************************************/
#if		CFG_MASTER_CPU == 1
LONG	LpxSearchUnMonTblMdef(				/* Monitor management table search								*/
		CMONDEF	*pUnDef,					/* Monitor definition											*/
		LONG	*pFindIdx		)			/* Search result index									*/
{
LONG	i;
/*--------------------------------------------------------------------------------------------------*/
/*		Search UnMonTbl[]																			*/
/*--------------------------------------------------------------------------------------------------*/
		for( i=0; i < MonMngP.UnMonTblEntNum; i++ )
		{
			if( pUnDef == UnMonTbl[i].MonDefPtr )
			{
				*pFindIdx = i; return( OK );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( NG );
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Monitor management table search(Monitor number specification)														*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	LpxSearchUnMonTblIdx(				/* Monitor management table search(Monitor number specification)				*/
		CMONTBL *MonTbl,					/* Search monitor definition										*/
		LONG	Size,						/* Monitor table size									*/
		LONG	UnNumber,					/* UnMonitor number											*/
		LONG	*pFindIdx		)			/* Search result index									*/
{
LONG is;	/* start index */
LONG ie;	/* end index */
LONG in;	/* intermediate index */
/*--------------------------------------------------------------------------------------------------*/
	is=0;
	ie=Size-1;
/*--------------------------------------------------------------------------------------------------*/
	while (TRUE)  /* binary search */
	{
		if (ie < is) { return (NG); }
		in = (is+ie)>>1;
		if (UnNumber == GET_UNMONNUMBER( MonTbl, in ) /* MonTbl[in].UnNo */ ){ *pFindIdx = in; return (OK); }
		if (UnNumber < GET_UNMONNUMBER( MonTbl, in ) /* MonTbl[in].UnNo */){ ie = in-1; }
		else { is = in+1; }
	}
/*--------------------------------------------------------------------------------------------------*/
}
#endif
/****************************************************************************************************/
/*																									*/
/*		Monitor management table search(Monitor number specification. If it is not found, the followingINDEX)					*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	LpxSearchUnMonTblNextIdx(			/* Monitor management table search(Monitor number specification)				*/
		CMONTBL	*MonTbl,					/* Search monitor definition table								*/
		LONG	EntNum,						/* Table Entry Number									*/
		LONG	UnNo,						/* Monitor number											*/
		LONG	StIdx,						/* Search start index									*/
		HMSGIF	Hmsg)						/* MessageIFHandle									*/
{
#define UNSEARCH_FORWARD (1)
#define UNSEARCH_BACKWARD (-1)
LONG idx;
LONG rtIdx; /* return index */
LONG dir; /* Search direction */
/*--------------------------------------------------------------------------------------------------*/
	rtIdx = StIdx;
/*--------------------------------------------------------------------------------------------------*/
	if ( UnNo >= GET_UNMONNUMBER( MonTbl, StIdx ) /* UnMonTbl[StIdx].UnNo */ )
	{ /* forward search */
		dir = UNSEARCH_FORWARD;
	}
	else
	{ /* backword search */
		dir = UNSEARCH_BACKWARD;
	}
/*--------------------------------------------------------------------------------------------------*/
	for ( idx = StIdx; (idx < EntNum /* MonMngP.UnMonTblEntNum */) && (idx >= 0); idx = idx + dir )
	{
		if ( LpxMonCheckAttrIdx( GET_UNMONDEF( MonTbl, idx ), idx, MCHK_OPE_READ, Hmsg ) != OK ) { continue; }
		if ( GET_UNMONNUMBER( MonTbl, idx ) /* MonTbl[idx].UnNo */ > MONDEF_MAX_UnNO ){ break; }
		/*------------------------------------------------------------------------------------------*/
		rtIdx = idx; /* visible index */
		if ( (dir == UNSEARCH_FORWARD ) && ( GET_UNMONNUMBER( MonTbl, idx ) /* MonTbl[idx].UnNo */ >= UnNo ) ){ break; }
		else if ( (dir == UNSEARCH_BACKWARD ) && ( GET_UNMONNUMBER( MonTbl, idx ) /* MonTbl[idx].UnNo */ <= UnNo ) ){ break; }
		/*------------------------------------------------------------------------------------------*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return rtIdx;
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Checking monitor attributes(Specifying monitor definition)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : CheckType.					*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	LpxMonCheckAttrMdef(
		CMONDEF	*pUnDef,					/* Monitor definition											*/
		ULONG	CheckType,					/* Check type										*/
		HMSGIF	Hmsg		)				/* MessageIFHandle									*/
{
/*--------------------------------------------------------------------------------------------------*/
/*		Check AccessLevel																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( (CheckType & MCHK_ACCS_LVL) && (Hmsg != NULL) )
		{
			if(	(pUnDef->AccessLevel) > Hmsg->AccessLvl )
			{
				return( MERR_LEVEL );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Check Special Use																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( CheckType & MCHK_SP_USE )
		{
			if( CHK_UNSPUSE() )
			{
				return( MERR_CHKSPUSE );
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}
#endif


/****************************************************************************************************/
/*																									*/
/*		Checking monitor attributes																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : CheckType.					*/
/*			UnMonTblFor																				*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	LpxMonCheckAttrIdx(					/* Return value : Status									*/
		CMONDEF	*pUnDef,					/* Monitor definition											*/
		LONG	Index,						/* Parameter management tableIndex							*/
		ULONG	CheckType,					/* Error check type									*/
		HMSGIF	Hmsg		)				/* MessageIFHandle									*/
{
LONG	rc = OK ;
/*--------------------------------------------------------------------------------------------------*/
/*		Index check																			*/
/*--------------------------------------------------------------------------------------------------*/
#if 0
		if( Index >= (LONG)MonMngP.UnMonTblEntNum )
		{
			return( MERR_NO );				/* Monitor number error										*/
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Checking monitor attributes																			*/
/*--------------------------------------------------------------------------------------------------*/
		rc = LpxMonCheckAttrMdef( pUnDef /* UnMonTbl[Index].MonDefPtr */, CheckType, Hmsg );
/*--------------------------------------------------------------------------------------------------*/
		return( rc );
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Monitor register read function																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Reads the specified number of register data from the specified register number.							*/
/*																									*/
/*	Note Intention : Specifying the size of the monitor register from the upper order(SHORT/LONG)does not exist									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	RpxReadMonRegister(
		LONG	RegNo,						/* Register number											*/
		LONG	Number,						/* Number of read data										*/
		USHORT	*RdBuf,						/* Read data buffer pointer							*/
		HMSGIF	Hmsg		)				/* MessageIFHandle									*/
{
LONG	i;
LONG	j;
ULONG	UnData;
LONG	FindIndex;
LONG	RegNoX = RegNo & 0x0FFF;
/*--------------------------------------------------------------------------------------------------*/
/*		Get monitor index from register number														*/
/*--------------------------------------------------------------------------------------------------*/
		if( LpxMonSearchRegIdx( RegNoX, &FindIndex ) != OK )
		{
			Hmsg->ErrRno = (USHORT)RegNo;					/* Abnormal register number						*/
			return( REGIF_REGNOERR );						/* Register number error(StartREGNumber)		*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Read specified size from specified register number														*/
/*--------------------------------------------------------------------------------------------------*/
		for( j=FindIndex, i=0; i < Number; j++ )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Read Register																		*/
		/*------------------------------------------------------------------------------------------*/
			if( LpxReadMonRegister( j, &UnData, MCHK_REG_READ, Hmsg ) != OK )
			{
				Hmsg->ErrRno = (USHORT)(RegNo + i);			/* Abnormal register number						*/
				return( REGIF_REGNOERR );					/* Register number error						*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*		Check RegNo																			*/
		/*------------------------------------------------------------------------------------------*/
			if( UnRegTbl[j].UnNo != (RegNoX + i) )
			{
				Hmsg->ErrRno = (USHORT)(RegNo + i);			/* Abnormal register number						*/
				return( REGIF_REGNOERR );					/* Register number error						*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*		SHORT Register																		*/
		/*------------------------------------------------------------------------------------------*/
			if( UnRegTbl[j].MonDefPtr->IsLong != TRUE )
			{
				RdBuf[i++] = (USHORT)UnData;				/* Read data							*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*		LONG  Register																		*/
		/*------------------------------------------------------------------------------------------*/
			else
			{
				if( (i+1) < Number )
				{
					RdBuf[i+0] = (USHORT)UnData;			/* Read data(Lower word)				*/
					RdBuf[i+1] = (USHORT)(UnData >> 16);	/* Read data(The upper word)				*/
					i = i + 2;
				}
				else
				{
					Hmsg->ErrRno = (USHORT)(RegNo + i);		/* Abnormal register number						*/
					return( REGIF_DAMTHERR );				/* Data consistency error(LONGBoundary)				*/
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Monitor register read process																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Read one piece of specified monitor data.													*/
/*																									*/
/*	Note Intention : Specifying the size of the monitor register from the upper order(SHORT/LONG)does not exist									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	LpxReadMonRegister(					/* Monitor register read process									*/
		LONG	Index,						/* Monitor management tableIndex								*/
		ULONG	*pResData,					/* Address to store the read monitor value					*/
		ULONG	CheckType,					/* Error check type									*/
		HMSGIF	Hmsg		)				/* MessageIFHandle									*/
{
LONG	rc;
CMONDEF	*pUnDef;
/*--------------------------------------------------------------------------------------------------*/
/*		Check RegTblIndex																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( Index >= MonMngP.UnRegTblEntNum )
		{
			return( REGIF_REGNOERR );				/* Register number error(StartREGNumber)				*/
		}
		pUnDef = UnRegTbl[Index].MonDefPtr;
/*--------------------------------------------------------------------------------------------------*/
/*		Checking monitor attributes																		*/
/*--------------------------------------------------------------------------------------------------*/
		rc = LpxMonCheckAttrMdef( pUnDef, CheckType, Hmsg ) ;
		if( rc != OK )
		{
			if( rc == MERR_CHKSPUSE )
			{
				*pResData = 0;						/* Read data									*/
				return( OK );						/* Normal return							*/
			}
			else
			{
				return( REGIF_REGNOERR );			/* Register number error								*/
			}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Checking monitor target CPU																	*/
/*--------------------------------------------------------------------------------------------------*/
#if		CFG_MASTER_CPU == 1
		if( pUnDef->MtcIsComIf )
		{
			LONG RdByteSize = ( pUnDef->IsLong )? 4 : 2;
			rc = DpiDegOpReadOneRegister( (UnRegTbl[Index].UnNo | 0xE000), RdByteSize, pResData );
			return( rc );
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Read Monitor Data																			*/
/*--------------------------------------------------------------------------------------------------*/
		*pResData = pUnDef->FuncPtr( UNCAL_MONCAL, pUnDef, Hmsg );
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Monitor register index search process (Round)											*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Finds the index of the monitor register with the specified register number.						*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	RpxMonSearchRegIdx(					/* Return value : Status									*/
		LONG	RegNo,						/* Register number											*/
		LONG	*pFindIdx,					/* Search result index									*/
		ULONG	CheckType		)			/* Error check type									*/
{
LONG	rc;
/*--------------------------------------------------------------------------------------------------*/
/*		RegNoSearch																					*/
/*--------------------------------------------------------------------------------------------------*/
		rc = LpxMonSearchRegIdx( RegNo, pFindIdx );
		if( rc != OK ){ return( REGIF_REGNOERR );}
/*--------------------------------------------------------------------------------------------------*/
/*		Attribute check																				*/
/*--------------------------------------------------------------------------------------------------*/
		rc = LpxMonCheckAttrMdef( UnRegTbl[*pFindIdx].MonDefPtr, CheckType, NULL );
		if( rc != OK ){ return( REGIF_REGNOERR );}
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}
#endif



/****************************************************************************************************/
/*																									*/
/*		Monitor register index search processing														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Finds the index of the monitor register with the specified register number.						*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#if 0
LONG	LpxMonSearchRegIdx(					/* Return value : Status									*/
		LONG	RegNo,						/* Register number											*/
		LONG	*pFindIdx		)			/* Search result index									*/
{
LONG	i,w;
LONG	EntNum = MonMngP.UnRegTblEntNum;
/*--------------------------------------------------------------------------------------------------*/
/*		Search startIndex																			*/
/*--------------------------------------------------------------------------------------------------*/
		w = (1 << MlibSrhbiton( EntNum - 1, SRH_FROM_MSB ));	/* Setting division width initial value					*/
		i = ( RegNo >= UnRegTbl[w].UnNo )? (EntNum - w) : 0;	/* Search startIndexSetting				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Monitor table search(Binary search)																*/
/*--------------------------------------------------------------------------------------------------*/
		for( w = w>>1; w > 0; w = w>>1 )						/* SearchLoop							*/
		{
			if( RegNo >= UnRegTbl[i+w].UnNo ){ i = i + w;}		/* Comparison &IndexUpdate					*/
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Search result processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( RegNo != UnRegTbl[i+w].UnNo )
		{
			return( NG );										/* Number mismatch, error return		*/
		}
		else
		{
			*pFindIdx = i;										/* Search result index storage			*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}
#endif







/***************************************** end of file **********************************************/

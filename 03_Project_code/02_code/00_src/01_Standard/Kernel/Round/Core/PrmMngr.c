/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PrmMngr.c : Parameter management module														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Machine Efficiency :	Manage all drive software parameters.										*/
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
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2002.08.21  K.Hamasuna															*/
/*		Rev.1.10 : 2008.04.24  T.Taniguchi															*/
/*		Rev.1.20 : 2011.04.06  K.Ozaki <0BJ14>														*/
/*		Rev.1.30 : 2013.05.08  K.Ozaki 																*/
/*		Rev.1.40 : 2013.09.26  K.Ozaki <S00C>														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "Parameter.h"		/* #include "Round.h" */



/****************************************************************************************************/
/*																									*/
/*		Global Variable Declaration																	*/
/*																									*/
/****************************************************************************************************/
CPARAM	CParam[CPARAM_ENT_NUM];						/* Parameter Accessor Instance */
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Private Macro Definition																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter Number Define																		*/
/*--------------------------------------------------------------------------------------------------*/
#define	ENC_PRM_NUM						0x0f00
#define ENC_PRM_WRT_DISABLE				0
/*--------------------------------------------------------------------------------------------------*/
/*		Long Parameter Operation Macro for Big/Little Endian										*/
/*--------------------------------------------------------------------------------------------------*/
/*		Macro Argument     :   lwd    hwd															*/
/*		LongPrm Word Align : {LoNum, HiNum} : ULONGOF_REGDATA( (hwd<<16) + lwd ) <-- SGDV			*/
/*		LongPrm Word Align : {HiNum, LoNum} : ULONGOF_REGDATA( (lwd<<16) + hwd ) <-- ????			*/
/*--------------------------------------------------------------------------------------------------*/
#if		CCSW_BIG_ENDIAN == 1
#define GET_LWD_REGDATA( xlong )		(USHORT)((xlong)>>0 )
#define GET_HWD_REGDATA( xlong )		(USHORT)((xlong)>>16)
#define ULONGOF_REGDATA( lwd, hwd )		(ULONG)((((USHORT)(hwd))<<16) + (USHORT)(lwd))
#else
#define GET_LWD_REGDATA( xlong )		(USHORT)((xlong)>>0 )
#define GET_HWD_REGDATA( xlong )		(USHORT)((xlong)>>16)
#define ULONGOF_REGDATA( lwd, hwd )		(ULONG)((((USHORT)(hwd))<<16) + (USHORT)(lwd))
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Get Parameter Variable Address for Single/Multi Drive										*/
/*--------------------------------------------------------------------------------------------------*/
#if 1		/* Mercury */
#define LPX_SET_PRMVAR_RAM( pndef, data, hmsg, save ) {                                             \
		if( NULL != (pndef)->GetRamPtrCallback )                                                    \
		{                                                                                           \
		void *ptr = (pndef)->GetRamPtrCallback( 0, hmsg->pAxRsc );                                  \
				switch (((pndef)->DataLength) )                                                     \
				{                                                                                   \
				case 4:                                                                             \
					(save) = _OBJ_LONG( ptr, (pndef)->Sign );                                       \
					*(ULONG *)(ptr) = (data);                                                       \
					break;                                                                          \
				case 2:                                                                             \
					(save) = _OBJ_SHORT( ptr, (pndef)->Sign );                                      \
					*(USHORT *)(ptr) = (data);                                                      \
					break;                                                                          \
				default:                                                                            \
					(save) = _OBJ_CHAR( ptr, (pndef)->Sign );                                       \
					*(UCHAR *)(ptr) = (data);                                                       \
					break;                                                                          \
				}                                                                                   \
		}                                                                                           \
}
#endif
#if 0		/* Jupiter */
#ifndef CFG_GET_AXIS_VAR_ADR
#define LPX_GET_PRMVAR_ADR( pVar )		((USHORT*)(pVar))
#else
#define LPX_GET_PRMVAR_ADR( pVar )		((USHORT*)CFG_GET_AXIS_VAR_ADR( (pVar) ))
#endif

#define LPX_SET_PRMVAR_RAM( pndef, data, hmsg, save ) {                                             \
		pPrmVar = LPX_GET_PRMVAR_ADR( pRealDef->RamPtr );		/* Getting variable addresses					*/
		if( pRealDef->Attr.VarIsLong )							/* LONGFor parameters 			*/
		{
			PrmSave = *(ULONG*)pPrmVar;							/* LONGData save					*/
			*(ULONG*)pPrmVar = PrmData;							/* LONGData write					*/
		}
		else
		{
			PrmSave = *(USHORT*)pPrmVar;						/* SHORTData save					*/
			*(USHORT*)pPrmVar = (USHORT)PrmData;				/* SHORTData write				*/
		}
}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Restore Parameter Variable 																	*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_RESTORE_PARAMETER( pndef, hmsg, save ) {                                                \
		if( NULL != (pndef)->GetRamPtrCallback )                                                    \
		{                                                                                           \
		void *ptr = (pndef)->GetRamPtrCallback( 0, hmsg->pAxRsc );                                  \
				switch (((pndef)->DataLength) )                                                     \
				{                                                                                   \
				case 4:                                                                             \
					*(ULONG *)(ptr) = (save);                                                       \
					break;                                                                          \
				case 2:                                                                             \
					*(USHORT *)(ptr) = (save);                                                      \
					break;                                                                          \
				default:                                                                            \
					*(UCHAR *)(ptr) = (save);                                                       \
					break;                                                                          \
				}                                                                                   \
		}                                                                                           \
}
#if 0 /* Jupiter */
			if( pRealDef->Attr.VarIsLong )
			{
				*(ULONG*)pPrmVar = PrmSave;						/* LONGData recovery					*/
			}
			else
			{
				*(USHORT*)pPrmVar = (USHORT)PrmSave;			/* SHORTData recovery					*/
			}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		 Parameter Calculation Function 															*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_CALC_PARAMETER( pndef, rc, hmsg ) {                                                     \
		if ( NULL != (pndef)->PrmCalCallback )                                                      \
		{                                                                                           \
			(rc) = ((((pndef)->PrmCalCallback)( PRM_ACCCMD_WRITE,                                   \
						0 /* @@array index@@ */,                                                    \
						hmsg->pAxRsc, &PrmData ) >= 0) ? TRUE : FALSE );                            \
		}                                                                                           \
		else                                                                                        \
		{                                                                                           \
			(rc) = TRUE;                                                                            \
		}                                                                                           \
}

/*--------------------------------------------------------------------------------------------------*/
/*		Check the parameter is long or not		 													*/
/*--------------------------------------------------------------------------------------------------*/
#define CHK_PNPRM_ISLONG( pndef )   (((pndef)->DataLength) == 4 ? TRUE : FALSE)

/*--------------------------------------------------------------------------------------------------*/
/*		Adjust Operator Input <S149>																*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_ADJOPEINC( pndef, aidx, rc, hmsg ) {                                                    \
		if ( NULL != (pndef)->PrmCalCallback )                                                      \
		{                                                                                           \
			(rc) = ((pndef)->PrmCalCallback)( PRM_ACCCMD_ADJINCDATA,                                \
						(aidx) ,	                                                                \
						hmsg->pAxRsc, &(rc) );                                                      \
		}                                                                                           \
		else                                                                                        \
		{                                                                                           \
			(rc) = 0;                                                                               \
		}                                                                                           \
}

/*--------------------------------------------------------------------------------------------------*/
/*		Adjust Operator Input <S149>																*/
/*--------------------------------------------------------------------------------------------------*/
#define LPX_ADJOPEDEC( pndef, aidx, rc, hmsg ) {                                                    \
		if ( NULL != (pndef)->PrmCalCallback )                                                      \
		{                                                                                           \
			(rc) = ((pndef)->PrmCalCallback)( PRM_ACCCMD_ADJDECDATA,                                \
						(aidx) ,	                                                                \
						hmsg->pAxRsc, &(rc) );                                                      \
		}                                                                                           \
		else                                                                                        \
		{                                                                                           \
			(rc) = 0;                                                                               \
		}                                                                                           \
}


/****************************************************************************************************/
/*																									*/
/*		Function List																				*/
/*																									*/
/****************************************************************************************************/

/*--------------------------------------------------------------------------------------------------*/
/*		Public Functions																			*/
/*--------------------------------------------------------------------------------------------------*/
void	RpxInitPrmManager( void );			/* Parameter management initialization processing								*/
void	RpxPrstPrmManager( void );			/* Parameter management parameter reset processing					*/

/*--------------------------------------------------------------------------------------------------*/
/*		Public Functions																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	RpiPrmWriteRam(						/* Write a Parameter (RAM)								*/
		CPARAM  *this,						/* this Pointer											*/
		CPRMDEF *pPrmDef,					/* Pointer to the PrmDef								*/
		ULONG	Data,						/* Parameter Value 										*/
		HMSGIF	Hmsg );						/* Message Interface									*/
/*--------------------------------------------------------------------------------------------------*/
LONG	RpxPrmDigitInc(						/* Increment Parameter Digit (RAM)						*/
		CPARAM  *this,						/* this Pointer											*/
		LONG	Index,						/* Parameter Index										*/
		ULONG	Digit,						/* Digit(0~)											*/
		MSGIF  *hMsg );						/* Message Interface 									*/
/*--------------------------------------------------------------------------------------------------*/
LONG	RpxPrmDigitDec(						/* Decrement Parameter Digit (RAM)						*/
		CPARAM  *this,						/* this Pointer											*/
		LONG	Index,						/* Parameter Index										*/
		ULONG	Digit,						/* Digit(0~)											*/
		MSGIF  *hMsg );						/* Message Interface 									*/

/*--------------------------------------------------------------------------------------------------*/
/*		Private Functions																			*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxPrmLimitCheckPdef(				/* Parameters upper and lower limit check processing						*/
		CPRMDEF *pPrmDef,					/* Parameter definition pointer								*/
		ULONG	Data			);			/* Data												*/
/*--------------------------------------------------------------------------------------------------*/
LONG	LpxPrmWriteRam(						/* Parameter writing process								*/
		CPRMDEF *pRealDef,					/* Parameter definition pointer								*/
		ULONG	PrmData,					/* Parameter value to write									*/
		HMSGIF	Hmsg			);			/* Message handle									*/
/*--------------------------------------------------------------------------------------------------*/
ULONG	LpxPrmDataDigitInc(					/* Digit increment of data value							*/
		ULONG	Data,						/* Original data value											*/
		ULONG	Digit,						/* Specified digit(0~)											*/
		ULONG	UpperLimit,					/* Maximum value												*/
		ULONG	Base,						/* Bottom of data(DSPDEF_BASENBL, BASEDEC, BASEHEX)			*/
		ULONG	IsLong,						/* Long flag(TRUE, FALSE)							*/
		ULONG	Sign			);			/* Sign presence/absence(DSPDEF_W_SIGN, NOSIGN)						*/
/*--------------------------------------------------------------------------------------------------*/
ULONG	LpxPrmDataDigitDec(					/* Digit decrement of data value								*/
		ULONG	Data,						/* Original data value											*/
		ULONG	Digit,						/* Specified digit(0~)											*/
		ULONG	LowerLimit,					/* Minimum value												*/
		ULONG	Base,						/* Bottom of data(DSPDEF_BASENBL, BASEDEC, BASEHEX)			*/
		ULONG	IsLong,						/* Long flag(TRUE, FALSE)							*/
		ULONG	Sign			);			/* Sign presence/absence(DSPDEF_W_SIGN, NOSIGN)						*/
/*--------------------------------------------------------------------------------------------------*/
USHORT	RpxPrmGetDecimals(					/* <S14F> 												*/
		CPARAM  *this,						/* this Pointer											*/
		CPRMDEF	*prmdef,					/* Pointer to parameter definition							*/
		MSGIF	*hMsg			);			/* Message handle									*/
/*--------------------------------------------------------------------------------------------------*/





/****************************************************************************************************/
/*																									*/
/*		Parameter management initialization processing																	*/
/*																									*/
/****************************************************************************************************/
void	RpxInitPrmManager( void )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Set	Table Entry Number																		*/
/*--------------------------------------------------------------------------------------------------*/
		CPARAMX->PrmTblEntNum = PRMTBL_ENTNUM;		    /* Set PrmTbl Entry Number					*/
		CPARAMX->pPrmTbl = PRMTBL_PTR;					/* Set PrmTbl Pointer						*/
/*--------------------------------------------------------------------------------------------------*/
/*		Set	Parameter Access Method 																*/
/*--------------------------------------------------------------------------------------------------*/
		CPARAMX->Read  = RpxObjRead; 					/* TODO: */
//		CPARAMX->ReadX = RpxObjReadIdx; 				/* TODO: */
		CPARAMX->ReadS  = RpxObjReadS; 					/* <S073> */
		CPARAMX->Search = RpxObjSearch; 				/* TODO: */
		CPARAMX->SearchB = RpxObjSearchIdx; 			/* TODO: */
		CPARAMX->ReadArray = RpxObjReadArray; 			/* TODO: */
		CPARAMX->Write = RpxObjWrite;					/* Write Ram Parameter */
		CPARAMX->WriteS = RpxObjWriteS;					/* <S073> */ 
/*--------------------------------------------------------------------------------------------------*/
		CPARAMX->DigitInc = RpxPrmDigitInc;				/* Increment Ram Parameter */
		CPARAMX->DigitDec = RpxPrmDigitDec;				/* Decrement Ram Parameter */
/*--------------------------------------------------------------------------------------------------*/
		CPARAMX->GetNumber = RpxObjGetNumber;			/* Get Object Number */
		CPARAMX->isLong = RpxObjChkLong;				/* Check Object is Long */
/*--------------------------------------------------------------------------------------------------*/
		CPARAMX->SearchS = RpxObjSearchSub;				/* <S073> */
		CPARAMX->SearchBS = RpxObjSearchIdxSub;			/* <S073> */
		CPARAMX->WriteArray = RpxObjWriteArray;			/* <S073> */
/*--------------------------------------------------------------------------------------------------*/
		CPARAMX->PrmGetDecimals = RpxPrmGetDecimals;	/* <S14F> */
/*--------------------------------------------------------------------------------------------------*/
		return;
}



/****************************************************************************************************/
/*																									*/
/*		Parameter management parameter reset processing														*/
/*																									*/
/****************************************************************************************************/
void	RpxPrstPrmManager( void )
{
		return;
}

/****************************************************************************************************/
/*																									*/
/*		Write Parameter to RAM																		*/
/*																									*/
/****************************************************************************************************/
LONG	RpiPrmWriteRam(						/* Write Parameter to RAM								*/
		CPARAM  *this,						/* this Pointer											*/
		CPRMDEF *pPrmDef,					/* Pointer to the PrmDef								*/
		ULONG	Data,						/* Parameter Value 										*/
		HMSGIF	Hmsg )						/* Message Interface									*/
{
LONG	i;
LONG	rc;
/*TODO: WRPINF	WrPrmInf; */
CPRMDEF	*pRealDef = LPX_GET_pRealDef( pPrmDef );
/*--------------------------------------------------------------------------------------------------*/
/*		Check when copying parameters																*/
/*--------------------------------------------------------------------------------------------------*/
		if( 0 /* TODO: RoutV.f.PrmCopy */ )
		{
			return( PERR_PRMCOPY );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter writing permission check	@@@Review required@@@												*/
/*--------------------------------------------------------------------------------------------------*/
		if( 0 /* TODO: LpxCheckPrmWriteHandle( Hmsg, FnPnChk, pPrmDef ) != OK */ )					
		{
			return( PERR_WRITE_HANDLE );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		FnChecking parameter changes during execution															*/
/*--------------------------------------------------------------------------------------------------*/
		if( 0 /* TODO: RpiFunPrmWriteInhibitChk( ) */ )
		{
			return( PERR_INH );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Data range error																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( LpxPrmLimitCheckPdef( pPrmDef, Data ) != OK )
		{
			return( PERR_RANGE );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Normal parameter writing process																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( 1 /* TODO: pPrmDef->Attr.FrwAliasPrm == FALSE */ )
		{
			rc = LpxPrmWriteRam( pRealDef, Data /* LpxPrmIfDataToPrmVarData( pPrmDef, Data ) */, Hmsg );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Alias special parameter writing process																*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			if( 1 /* TODO: pPrmDef->FuncPtr( PNCAL_WRITEALIASPRM, Data, (ULONG)&WrPrmInf ) != TRUE */ )
			{
				return( PERR_RANGE );
			}
		/*------------------------------------------------------------------------------------------*/
//			for( rc=OK, i=0; i < 0; i++ )/* TODO: WrPrmInf.Count */
//			{
//				rc = NG /* LpxCheckPrmWriteHandle( Hmsg, FnPnChk, WrPrmInf.pPrmDef[i] ) */;
//				if( rc != OK ){ return( PERR_WRITE_HANDLE );}
//				rc = NG /* LpxPrmWriteRam( WrPrmInf.pPrmDef[i], WrPrmInf.WrtData[i], Hmsg ) */;
//				if( rc != OK ){ return( rc );}
//			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( rc );
}

/****************************************************************************************************/
/*																									*/
/*		Digit increment of parameter value																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview:	Increment the specified digit.																*/
/*																									*/
/*	Caution:	Changed to a process that reads, limits, increments, and rewrites parameters all at once.		*/
/*																									*/
/*	Return value:	Error status similar to parameter rewriting.											*/
/*																									*/
/****************************************************************************************************/
LONG	RpxPrmDigitInc(						/* Increment Parameter Digit (RAM)						*/
		CPARAM  *this,						/* this Pointer											*/
		LONG	Index,						/* Parameter Index										*/
		ULONG	Digit,						/* Digit(0~)											*/
		MSGIF  *hMsg )						/* Message Interface 									*/
{
ULONG	t_data;
ULONG   data; 
CPRMDEF *pPrmDef  = GET_PNPRMDEF( this->pPrmTbl, Index ) /* PnPrmTbl[Index].PrmDefPtr */;
CPRMDEF	*pRealDef = LPX_GET_pRealDef( pPrmDef );
/*--------------------------------------------------------------------------------------------------*/
/*		Read Parameter Value 																		*/
/*--------------------------------------------------------------------------------------------------*/
		this->Read( (COBJECT *)this, Index, &data, PCHK_OPE_READ, hMsg );
/*--------------------------------------------------------------------------------------------------*/
/*		Increment Digit																				*/
/*--------------------------------------------------------------------------------------------------*/
		t_data = LpxPrmDataDigitInc( 
					data                          /* Current Data */, 
					Digit                         /* Digit(0~) */, 
					GET_PNPRM_UPPERLMT( pPrmDef ) /* RpxPrmUpperLimitIdx(Index) */,
					GET_PNPRMBASE( pPrmDef )      /* (UCHAR)pPrmDef->Attr.Base */,
					CHK_PNPRM_ISLONG( pPrmDef )   /* (UCHAR)pPrmDef->Attr.IfIsLong */,
					CHK_PNPRM_SIGNED( pPrmDef )   /* (UCHAR)pPrmDef->Attr.Sign */ );
/*--------------------------------------------------------------------------------------------------*/
/*		Limit Check																					*/
/*--------------------------------------------------------------------------------------------------*/
//		if( pPrmDef->Attr.Base != DSPDEF_BASENBL )
//		{
//			if( PrmLimitCheckIdx(Index, t_data) != OK )
//			{
//				t_data = RpxPrmUpperLimitIdx(Index);
//			}
//		}
/*--------------------------------------------------------------------------------------------------*/
/*		Adjust Operator Input <S149>																*/
/*--------------------------------------------------------------------------------------------------*/
		if( GET_PNPRM_ADJOPE( pPrmDef ) /* pPrmDef->Attr.AdjOpeIn */ )
		{
			LPX_ADJOPEINC( pPrmDef, Index, t_data, hMsg );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Write Incremented Parameter																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( data ^ t_data )
		{
			return (this->Write( this, Index, t_data, hMsg ));
		}
		else
		{
			return (OK);
		}
/*--------------------------------------------------------------------------------------------------*/
//	return( t_data );
}



/****************************************************************************************************/
/*																									*/
/*		Digit increment of data value																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview:	Gets the value by incrementing the specified digit.												*/
/*			If the incremented value exceeds the maximum value, return the maximum value.							*/
/*																									*/
/*	Return value:	Incremented value or maximum value														*/
/*																									*/
/****************************************************************************************************/
ULONG	LpxPrmDataDigitInc(					/* Digit increment of data value							*/
		ULONG	Data,						/* Original data value											*/
		ULONG	Digit,						/* Specified digit(0~)											*/
		ULONG	UpperLimit,					/* Maximum value												*/
		ULONG	Base,						/* Bottom of data(DSPDEF_BASENBL, BASEDEC, BASEHEX)			*/
		ULONG	IsLong,						/* Long flag(TRUE, FALSE)							*/
		ULONG	Sign			)			/* Sign presence/absence(DSPDEF_W_SIGN, NOSIGN)						*/
{
LONG	nbl_data;
ULONG	t_data = Data;
/*--------------------------------------------------------------------------------------------------*/
/*		For nibbles																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( Base == OBJDEF_BASENBL )
		{
			nbl_data = (CHAR)((Data & (0x0f << (Digit*4))) >> (Digit*4));
			nbl_data += 1;
			if(nbl_data > (CHAR)((UpperLimit & (0x0f << (Digit*4))) >> (Digit*4)))
			{
				nbl_data = (CHAR)((UpperLimit & (0x0f << (Digit*4))) >> (Digit*4));
			}
			t_data = t_data - (t_data & (0x0f << (Digit*4))) + (nbl_data << (Digit*4));
		}
/*--------------------------------------------------------------------------------------------------*/
/*		For cases other than nibble																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
		/*------------------------------------------------------------------------------------------*/
		/*		WORDSize is signed																	*/
		/*------------------------------------------------------------------------------------------*/
			if(!IsLong && (Sign == OBJDEF_W_SIGN))
			{
				/* For signed words, sign extend	*/
				t_data = (LONG)((SHORT)(Data & 0xFFFF));
			}
		/*------------------------------------------------------------------------------------------*/
		/*		Increment data																*/
		/*------------------------------------------------------------------------------------------*/
			t_data += ((Base == OBJDEF_BASEDEC)? MlibDecPowerTbl[Digit]: ((ULONG)0x01 << (Digit*4)));
		/*------------------------------------------------------------------------------------------*/
		/*		If unsigned																*/
		/*------------------------------------------------------------------------------------------*/
			if(Sign == OBJDEF_NOSIGN)
			{
				if(t_data < Data || t_data > UpperLimit)
				{	/* When overflow occurs or If larger than the upper limit	*/
					t_data = UpperLimit;
				}
			}
		/*------------------------------------------------------------------------------------------*/
		/*		If signed																		*/
		/*------------------------------------------------------------------------------------------*/
			else if(Sign == OBJDEF_W_SIGN)
			{
				if((LONG)t_data < (LONG)Data || (LONG)t_data > (LONG)UpperLimit)
				{	/* When positive side overflow occurs or If larger than the upper limit	*/
					t_data = UpperLimit;
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( t_data );
}


/****************************************************************************************************/
/*																									*/
/*		Upper and lower parameter limit check														*/
/*																									*/
/****************************************************************************************************/
LONG	LpxPrmLimitCheckPdef(				/* Check Parameter Limit								*/
		CPRMDEF *pPrmDef,					/* Pointer to the PrmDef								*/
		ULONG	Data			)			/* Check Data											*/
{
ULONG	i;
/*--------------------------------------------------------------------------------------------------*/
/*		Nibble Parameter																			*/
/*--------------------------------------------------------------------------------------------------*/
		if( GET_PNPRMBASE( pPrmDef ) /* ->Attr.Base */ == OBJDEF_BASENBL ) 
		{
			ULONG	mask = 0x0F;
			ULONG	imax = ( CHK_PNPRM_ISLONG( pPrmDef ) /* ->Attr.IfIsLong */ )? 8 : 4;
		/*------------------------------------------------------------------------------------------*/
			for( i=0; i < imax; i++ )
			{
				if( (Data & mask) < (pPrmDef->LowerLimit & mask) ){ return(NG);}
				if( (Data & mask) > (pPrmDef->UpperLimit & mask) ){ return(NG);}
				mask = (mask << 4);
			}
			return(OK);
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Data range check: Signed																*/
/*--------------------------------------------------------------------------------------------------*/
		else if( CHK_PNPRM_SIGNED( pPrmDef ) /*->Attr.Sign */ == OBJDEF_W_SIGN )
		{
			if( (LONG)Data > (LONG)pPrmDef->UpperLimit ){ return( NG );}
			if( (LONG)Data < (LONG)pPrmDef->LowerLimit ){ return( NG );}
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Data range check: Unsigned																*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
			if( (ULONG)Data > (ULONG)pPrmDef->UpperLimit ){ return( NG );}
			if( (ULONG)Data < (ULONG)pPrmDef->LowerLimit ){ return( NG );}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}

/****************************************************************************************************/
/*																									*/
/*		Write Parameter to RAM 																		*/
/*																									*/
/****************************************************************************************************/
LONG	LpxPrmWriteRam(						/* Parameter writing process								*/
		CPRMDEF *pRealDef,					/* Parameter definition pointer								*/
		ULONG	PrmData,					/* Parameter value to write									*/
		HMSGIF	Hmsg			)			/* Message handle									*/
{
LONG	rc;
ULONG	PrmSave;
/* void	*pPrmVar; */
/* ULONG	ComIF; */
ULONG	ErrFlag = 0;
/*--------------------------------------------------------------------------------------------------*/
/*		Communication IF dedicated parameter processing																	*/
/*--------------------------------------------------------------------------------------------------*/
#if		CFG_MASTER_CPU == 1
		if( (pRealDef->Attr.PrmDefIdx == 1) && (pRealDef->Attr.DrvComPrm != TRUE) )
		{
			USHORT	PrmSize = pRealDef->Attr.VarIsLong? 4:2;
			rc = DpiDegOpWriteOneRegister( pRealDef->PnNo, PrmSize, PrmData );
			if( rc != OK ){ return( rc );}
		}
#endif

/*--------------------------------------------------------------------------------------------------*/
/*		Update Parameter RAM Area																	*/
/*--------------------------------------------------------------------------------------------------*/
		LPX_SET_PRMVAR_RAM( pRealDef, PrmData, Hmsg, PrmSave );

/*--------------------------------------------------------------------------------------------------*/
/*		Parameter calculation execution check																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( 0 /* TODO: (pRealDef->ChkSpUse & Bprm.ChkSpUse) == 0x00 */ ){ return( OK );}
#if		CFG_MASTER_CPU == 1
		if( (pRealDef->Attr.PrmDefIdx == 1) && (pRealDef->Attr.DrvComPrm != TRUE) ){ return( OK );}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Parameter calculation execution process																		*/
/*--------------------------------------------------------------------------------------------------*/
/*		Caution: Processing when a parameter calculation error occurs												*/
/*		Encoder communication error/Encoder communication position data acceleration error/Encoder communication timer error/			*/
/*		Do not process calculation errors when encoder echo back error alarm occurs.(SGDStwist)				*/
/*--------------------------------------------------------------------------------------------------*/
		KriResetParamError( );									/* Reset Error Flag					*/
		/* ComIF = (KPI_CHK_ComHmsg( Hmsg ))? 1 : 0 ;*/				/* ComIf(1)/Operator(0)				*/
		LPX_CALC_PARAMETER( pRealDef, rc, Hmsg ); 				/* rc = (*pRealDef->FuncPtr)( PNCAL_PRMCAL, ComIF, 0 );	*/ /* Parameter calculation					*/
		//TODO: Calculation error avoidance processing when encoder communication error occurs(INGRAM)
		if( (/* (Alm.PrmErrInfo.ErrFlag == TRUE) ||*/ (rc != TRUE)) /* && !Alm.AlmAttr.f.NoPceRepair */)
		{
			ErrFlag = 1;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Drive&ComIFCommon parameter processing																*/
/*--------------------------------------------------------------------------------------------------*/
#if		CFG_MASTER_CPU == 1
		if( (ErrFlag == 0) && (pRealDef->Attr.DrvComPrm == TRUE) )
		{
			USHORT	PrmSize = pRealDef->Attr.VarIsLong? 4:2;
			if( DpiDegOpWriteOneRegister( pRealDef->PnNo, PrmSize, PrmData ) != OK ){ ErrFlag = 1;}
		}
#endif
/*--------------------------------------------------------------------------------------------------*/
/*		Processing when a parameter calculation error occurs														*/
/*--------------------------------------------------------------------------------------------------*/
		if( ErrFlag != 0 )
		{
			LPX_RESTORE_PARAMETER( pRealDef, Hmsg, PrmSave );
		/*------------------------------------------------------------------------------------------*/
			if( rc == TRUE )
			{
				LPX_CALC_PARAMETER( pRealDef, rc, Hmsg ); /* (*pRealDef->FuncPtr)( PNCAL_PRMCAL, ComIF, 0 );*/	/* Parameter calculation					*/
			}
			return( PERR_CALC );
		}
/*--------------------------------------------------------------------------------------------------*/
		return( OK );
}

/****************************************************************************************************/
/*																									*/
/*		Digit decrement of parameter value																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	 Overview:	Gets the value by decrementing the specified digit.													*/
/*			If the decremented value is less than the minimum value, return the minimum value.							*/
/*																									*/
/*	Caution:	This function does not have a function to check the user level, etc.									*/
/*			In advance,RpxPrmSearchIdx() or PrmAttributeChkIdx()for checking,				*/
/*			OKPlease use it after confirming that it is.												*/
/*																									*/
/*	Return value:	Decremented value or minimum value														*/
/*																									*/
/****************************************************************************************************/
LONG	RpxPrmDigitDec(						/* Decrement Parameter Digit (RAM)						*/
		CPARAM  *this,						/* this Pointer											*/
		LONG	Index,						/* Parameter Index										*/
		ULONG	Digit,						/* Digit(0~)											*/
		MSGIF   *hMsg )						/* Message Interface 									*/
{
ULONG	t_data;
ULONG 	data;
CPRMDEF *pPrmDef = GET_PNPRMDEF( this->pPrmTbl, Index ) /* PnPrmTbl[Index].PrmDefPtr */;
CPRMDEF	*pRealDef = LPX_GET_pRealDef( pPrmDef );
/*--------------------------------------------------------------------------------------------------*/
/*		Read Parameter Value 																		*/
/*--------------------------------------------------------------------------------------------------*/
		this->Read( (COBJECT *)this, Index, &data, PCHK_OPE_READ, hMsg );
/*--------------------------------------------------------------------------------------------------*/
/*		Decrement Digit																				*/
/*--------------------------------------------------------------------------------------------------*/
		t_data = LpxPrmDataDigitDec( 
					data							/* Data */, 
					Digit							/* Digit (0~) */, 
					GET_PNPRM_LOWERLMT( pPrmDef )	/* RpxPrmLowerLimitIdx(Index) */,
					GET_PNPRMBASE( pPrmDef )		/* (UCHAR)pPrmDef->Attr.Base */,
					CHK_PNPRM_ISLONG( pPrmDef ) 	/* (UCHAR)pPrmDef->Attr.IfIsLong */,
					CHK_PNPRM_SIGNED( pPrmDef )		/* (UCHAR)pPrmDef->Attr.Sign */			);
/*--------------------------------------------------------------------------------------------------*/
/*		Limit Check																					*/
/*--------------------------------------------------------------------------------------------------*/
//		if( pPrmDef->Attr.Base != DSPDEF_BASENBL )
//		{
//			if(PrmLimitCheckIdx(Index, t_data) != OK)
//			{
//				t_data = RpxPrmLowerLimitIdx(Index);
//			}
//		}
/*--------------------------------------------------------------------------------------------------*/
/*		Adjust Operator Input <S149>																*/
/*--------------------------------------------------------------------------------------------------*/
		if( GET_PNPRM_ADJOPE( pPrmDef ) /* pPrmDef->Attr.AdjOpeIn */ )
		{
			LPX_ADJOPEDEC( pPrmDef, Index, t_data, hMsg );
		}
/*--------------------------------------------------------------------------------------------------*/
/*		Write Decremented Parameter																	*/
/*--------------------------------------------------------------------------------------------------*/
		if( data ^ t_data )
		{
			return (this->Write( this, Index, t_data, hMsg ));
		}
		else
		{
			return (OK);
		}
/*--------------------------------------------------------------------------------------------------*/
//		return( t_data );
}



/****************************************************************************************************/
/*																									*/
/*		Digit decrement of data value																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview:	Gets the value by decrementing the specified digit.													*/
/*			If the decremented value is less than the minimum value, return the minimum value.							*/
/*																									*/
/*	Return value:	Decremented value or minimum value														*/
/*																									*/
/****************************************************************************************************/
ULONG	LpxPrmDataDigitDec(					/* Digit decrement of data value								*/
		ULONG	Data,						/* Original data value											*/
		ULONG	Digit,						/* Specified digit(0~)											*/
		ULONG	LowerLimit,					/* Minimum value												*/
		ULONG	Base,						/* Bottom of data(DSPDEF_BASENBL, BASEDEC, BASEHEX)			*/
		ULONG	IsLong,						/* Long flag(TRUE, FALSE)							*/
		ULONG	Sign			)			/* Sign presence/absence(DSPDEF_W_SIGN, NOSIGN)						*/
{
LONG	nbl_data;
ULONG	t_data = Data;
/*--------------------------------------------------------------------------------------------------*/
/*		For nibbles																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( Base == OBJDEF_BASENBL )
		{
			nbl_data = (CHAR)((Data & (0x0f << (Digit*4))) >> (Digit*4));
			nbl_data -= 1;
			if(nbl_data < (CHAR)((LowerLimit & (0x0f << (Digit*4))) >> (Digit*4)))
			{
				nbl_data = (CHAR)((LowerLimit & (0x0f << (Digit*4))) >> (Digit*4));
			}
			t_data = t_data - (t_data & (0x0f << (Digit*4))) + (nbl_data << (Digit*4));
		}
/*--------------------------------------------------------------------------------------------------*/
/*		For cases other than nibble																			*/
/*--------------------------------------------------------------------------------------------------*/
		else
		{
		/*------------------------------------------------------------------------------------------*/
		/*		WORDSize is signed																	*/
		/*------------------------------------------------------------------------------------------*/
			if(!IsLong && (Sign == OBJDEF_W_SIGN))
			{
				/* For signed words, sign extend	*/
				t_data = (LONG)((SHORT)(Data & 0xFFFF));
			}
		/*------------------------------------------------------------------------------------------*/
		/*		Decrement data																	*/
		/*------------------------------------------------------------------------------------------*/
			t_data -= ((Base == OBJDEF_BASEDEC)? MlibDecPowerTbl[Digit]: ((ULONG)0x01 << (Digit*4)));
		/*------------------------------------------------------------------------------------------*/
		/*		If unsigned																*/
		/*------------------------------------------------------------------------------------------*/
			if( Sign == OBJDEF_NOSIGN )
			{
				if(t_data > Data || t_data < LowerLimit)
				{	/* When overflow occurs or If smaller than the lower limit value	*/
					t_data = LowerLimit;
				}
			}
		/*------------------------------------------------------------------------------------------*/
 		/*		If signed																		*/
		/*------------------------------------------------------------------------------------------*/
 			else if( Sign == OBJDEF_W_SIGN )
			{
				if((LONG)t_data > (LONG)Data || (LONG)t_data < (LONG)LowerLimit)
				{	/* When negative side overflow occurs or If smaller than the lower limit value	*/
					t_data = LowerLimit;
				}
			}
		}
/*--------------------------------------------------------------------------------------------------*/
		return( t_data );
}

/* <S14F> */
/****************************************************************************************************/
/*																									*/
/*		Get parameter decimal point position																	*/
/*																									*/
/****************************************************************************************************/
USHORT	RpxPrmGetDecimals(
		CPARAM  *this,						/* this Pointer											*/
		CPRMDEF	*prmdef,					/* Pointer to parameter definition							*/
		MSGIF	*hMsg		)				/* Message handle									*/
{
	AXIS_HANDLE *pAxRsc		= hMsg->pAxRsc;
	USHORT		Decimals	= prmdef->Decimals;

	if( pAxRsc->MencV->LowSpdMotor )
	{
		switch( prmdef->RegNumber )
		{
			case	0x0301:
			case	0x0302:
			case	0x0303:
			case	0x0304:
			case	0x0493:
			case	0x0494:
			case	0x0533:
							Decimals++;
							break;
			default:
							break;
		}
	}
	
	return( Decimals );
}

/***************************************** end of file **********************************************/

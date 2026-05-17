/****************************************************************************************************/
/*																									*/
/*																									*/
/*		DataTraceManager.c : Data trace processing														*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Data trace processing																		*/
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
/*	Note	:	First edition	2010.10.26	T.Kira	For INGRAM												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "MLib.h"
#include "XlsTrcDef.h"
#include "DataTraceCalc.h"
#include "DataTraceManager.h"
#include "Memory.h"					/* @@ */

/****************************************************************************************************/
/*		PROTOTYPE																					*/
/****************************************************************************************************/
static BOOL	dtrcCalcTraceObject( TRCHNDL *TrcHndl, LONG Tno );
static LONG	dtrcSearchTrcBitTbl( TRCHNDL *TrcHndl, LONG SelNo );
//static LONG	dtrcCalcSetSampLlim( void* Axis, LONG Axno );
static void dtrcSetTrgParameter( LONG TrgNo, SHORT Adj, TRCEXEP *TrcExeP, TRCREG *TrcReg );
static void dtrcClearTrgParameter( TRCTRG *TrcTrg );
//
static LONG	dtrcGetMergeData( TRCEXEP *TrcExeP, TRCOBJ *Tobjpx );
static void	dtrcGetTrcData( TRCEXEP *TrcExeP, TRCOBJ *Tobjpx, LONG *VerData, LONG *BitData, LONG Idx );
static BOOL	dtrcCheckTriggerProcedure( TRCHNDL *TrcHndl );
static BOOL	dtrcCheckTrigger( LONG TrgNo, TRCHNDL *TrcHndl, LONG *TrgVar );
static BOOL	dtrcFinishOpeModeOperation( TRCHNDL *TrcHndl );
static void* dtrcConvAxisAddress( MEMOBUS_ADDR_MAP *AxisMap, LONG Address, LONG *AxisNum );
static void* dtrcGetTopAxisAddress( MEMOBUS_ADDR_MAP *AxisMap );
//
static BOOL	dtrcSetDataTraceCmd( TRCHNDL *TrcHndl, ULONG OpeModeTrc, ULONG OpeCmdTrc );
static BOOL	dtrcStartDataTrace( TRCHNDL *TrcHndl, UCHAR mode );
static BOOL	dtrcStopDataTrace( TRCHNDL *TrcHndl );
static BOOL	dtrcStartRingTrace( TRCHNDL *TrcHndl );
static BOOL	dtrcStopRingTrace( TRCHNDL *TrcHndl, LONG finmode );
//
static void	DtrcSelSetInitValue( TRCHNDL *TrcHndl );	/* <S0C9> */

/****************************************************************************************************/
/*																									*/
/*		Data trace management initialization process																*/
/*																									*/
/****************************************************************************************************/
void	DtrcInitDtrcManager( TRCHNDL *TrcHndl, USHORT *TraceBuffer, MEMOBUS_ADDR_MAP *AxisMap )
{
	LONG	EntNum,i;	
	TRCMNGP	*TrcMngP;
	TrcMngP = &TrcHndl->TrcMngP;

	/*----------------------------------------------------------------------------------------------*/
	/*		Numerical trace definition binary search width setting														*/
	/*----------------------------------------------------------------------------------------------*/
	EntNum = TRCVARTBL_ENTNUM;
	TrcMngP->TrcVarTblEntNum = (ULONG)EntNum;
	TrcMngP->TrcVarTblSrhIniW = (1 << MlibSrhbiton( EntNum - 1, SRH_FROM_MSB ));
	/*----------------------------------------------------------------------------------------------*/
	/*		Bit trace definition binary search width setting														*/
	/*----------------------------------------------------------------------------------------------*/
	EntNum = TRCBITTBL_ENTNUM;
	TrcMngP->TrcBitTblEntNum = (ULONG)EntNum;
	TrcMngP->TrcBitTblSrhIniW = (1 << MlibSrhbiton( EntNum - 1, SRH_FROM_MSB ));
	/*----------------------------------------------------------------------------------------------*/
	/*		Data trace management data variable reset													*/
	/*----------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( &TrcHndl->TrcMngV, sizeof(TrcHndl->TrcMngV)/4 );
	MlibResetLongMemory( &TrcHndl->TrcExeV, sizeof(TrcHndl->TrcExeV)/4 );
	/*----------------------------------------------------------------------------------------------*/
	/*		Get axis address definition																	*/
	/*----------------------------------------------------------------------------------------------*/
	for(i = 0; i < MAX_AXIS_NUMBER; i++)
	{
		TrcHndl->AxisMap[i] = AxisMap[i];
	}

/* <S0C9>	Start */
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace target data selection,I/OInitial value setting for trace target data selection							*/
	/*----------------------------------------------------------------------------------------------*/
	DtrcSelSetInitValue( TrcHndl );
/* <S0C9>	End */

#ifdef _DATA_TRACE_DBUG_
//For debugging
	TrcReg->TrcMode     = 0x0010;
	TrcReg->DataSize    = 0x0003;
	TrcReg->AreaSize    = 8192;
	TrcReg->SampIntv    = 0x0001;
	TrcReg->SampTime    = 250;
	TrcReg->SampLlim    = 1;
	TrcReg->SampUlim    = 255;
	TrcReg->MaxAreaSize = 8192;
	TrcReg->OpeModeTrc  = 0;
	TrcReg->OpeCmdTrc   = 0;

	TrcReg->PtrgDnum  = 0x0064;
	TrcReg->TrgSel    = 0x0100;
	TrcReg->TrgLevel  = 0x100;
	TrcReg->TrgPoint  = 0x0000;
	TrcReg->TrcState  = 0x0000;
	TrcReg->MTrgMode  = 0;
	TrcReg->MTrgSel   = 0;
	TrcReg->MTrgLevel = 0;
#endif
	/*----------------------------------------------------------------------------------------------*/
	/*		Data trace parameter calculation															*/
	/*----------------------------------------------------------------------------------------------*/
	DtrcCalcParameter( TrcHndl, TraceBuffer );			/* Data trace execution parameter calculation			*/

}

/****************************************************************************************************/
/*																									*/
/*		Data trace execution parameter calculation process														*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT DtrcCalcParameter( TRCHNDL *TrcHndl, USHORT *TraceBuffer )
{
	ULONG	i;
	ULONG	x,y,z;
	SHORT	Adj;

	TRCREG	*TrcReg;
	TRCEXEP	*TrcExeP;
	TRCINDIVPRM	*TrcIncivPrm;	/* <S0C9> */

	TrcReg  = &TrcHndl->TrcReg;
	TrcExeP = &TrcHndl->TrcExeP;
	TrcIncivPrm = &TrcHndl->TrcIncivPrm;	/* <S0C9> */

	TrcExeP->TrcPrmOk = FALSE;								/* Data trace parameter setOK		*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace sampling period minimum value setting														*/
	/*----------------------------------------------------------------------------------------------*/
	/*for( i=0; i < TRC_CH_MAXNUM; i++ )
	{
		x = dtrcCalcSetSampLlim( TrcHndl, i );
	
		if(x > TrcReg->SampLlim)
		{
			TrcReg->SampLlim = (USHORT)x;
		}
	}*/
/* <S0C9>	Delete Start */
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace area size																	*/
	/*----------------------------------------------------------------------------------------------*/
	//if ( TrcReg->AreaSize > TrcReg->MaxAreaSize )
	//{
	//	return PRM_RSLT_CALC_ERR;
	//}
/* <S0C9>	Delete End */
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace data type																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrcReg->DataSize == 1 || TrcReg->DataSize == 3 )				/* Short data trace	*/
	{
		TrcExeP->TrcDataType = DATATRACE_SHORT;
	}
	else if( TrcReg->DataSize == 2 || TrcReg->DataSize == 4 )			/* Long data trace		*/
	{
		TrcExeP->TrcDataType = DATATRACE_LONG;
	}
	else
	{
		return PRM_RSLT_CONDITION_ERR;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		IOTrace type																		*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrcReg->DataSize == 1 || TrcReg->DataSize == 2 )
	{
		TrcExeP->TrcIOType = IODATA_MERGE;						/* IOCombine data with variable data		*/
		Adj = 0;												/* Number of traces, bit shift adjustment value */
	}
	else if( TrcReg->DataSize == 3 || TrcReg->DataSize == 4 )
	{
		TrcExeP->TrcIOType = IODATA_DIV;						/* IOStore data in dedicated area			*/
		Adj = 1;												/* Number of traces, bit shift adjustment value */
	}
	else
	{ /* Undefined */
		TrcExeP->TrcIOType = IODATA_MERGE;						/* IOCombine data with variable data		*/
		Adj = 0;												/* Number of traces, bit shift adjustment value */
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Trace data upper/lower limit data, bit set/clear data										*/
	/*----------------------------------------------------------------------------------------------*/
	if(TrcExeP->TrcIOType == IODATA_MERGE )							/* IOCombine data with variable data	*/
	{	 
		if( TrcExeP->TrcDataType == DATATRACE_SHORT )
		{	/* Short data trace */
			TrcExeP->UpperLmt =  16383;
			TrcExeP->LowerLmt = -16384;
			TrcExeP->TrcBitSet = 0x8000;
			TrcExeP->TrcBitClr = ~TrcExeP->TrcBitSet;
		}
		else
		{	/* Long data trace */
			TrcExeP->UpperLmt =  1073741823;
			TrcExeP->LowerLmt = -1073741824;
			TrcExeP->TrcBitSet = 0x80000000;
			TrcExeP->TrcBitClr = ~TrcExeP->TrcBitSet;
		}
	}
	else															/* IOStore data in dedicated area		*/ 
	{
		if( TrcExeP->TrcDataType == DATATRACE_SHORT )
		{	/* Short data trace */
			TrcExeP->UpperLmt =  32767;
			TrcExeP->LowerLmt = -32768;
			TrcExeP->TrcBitSet = 1;
			TrcExeP->TrcBitClr = 0;
		}
		else
		{	/* Long data trace	*/
			TrcExeP->UpperLmt =  0x7FFFFFFF; /*  2147483647	*/
			TrcExeP->LowerLmt =  0x80000000; /* -2147483648 */
			TrcExeP->TrcBitSet = 1;
			TrcExeP->TrcBitClr = 0;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Number of trace data																		*/
	/*----------------------------------------------------------------------------------------------*/
	x = TrcReg->TrcMode;
	if( (x < 1) || (x > TRC_CH_MAXNUM ) )
	{
		return PRM_RSLT_CALC_ERR;
	}

/* <S0C9>	Start */
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace buffer length, trace area size												*/
	/*																								*/
	/*		Trace buffer length isΣ-7series.							*/
	/*		Trace buffer length = 0, link withΣ-Ⅴ"Used area size" as compatibility mode		*/
	/*		Enabled.																		*/
	/*																								*/
	/*----------------------------------------------------------------------------------------------*/
	if ( TrcReg->TrcBuffSize != 0 )		/* Trace buffer length Enabled (Trace buffer length ≠ 0) */
	{
		if ( TrcReg->TrcBuffSize > TrcReg->MaxTrcBuffSize )
		{
			return PRM_RSLT_CALC_ERR;
		}
		else
		{
			TrcReg->AreaSize = TrcReg->TrcBuffSize * TrcReg->TrcMode;
		}
	}
	else		/* Trace area size Enabled (Trace buffer length = 0) */
	{
		if ( TrcReg->AreaSize > TrcReg->MaxAreaSize )
		{
			return PRM_RSLT_CALC_ERR;
		}
	}
/* <S0C9>	End */

	/*----------------------------------------------------------------------------------------------*/
	/*		Pre-trigger data number check																*/
	/*----------------------------------------------------------------------------------------------*/
//#if CSW_NET_TRG_USE == TRUE		/* <S012> *//* <S021> */
//	y = ( TrcExeP->TrcDataType == DATATRACE_SHORT ) ? x : (2*x);
//#else /* CSW_NET_TRG_USE */
//	y = ( TrcExeP->TrcDataType == DATATRACE_SHORT ) ? 1 : 2;		/* <S0C9>	Delete */
//#endif /* CSW_NET_TRG_USE */
	y = ( TrcExeP->TrcDataType == DATATRACE_SHORT ) ? x : (2*x);	/* <S0C9> */

	TrcExeP->TrcDnum = TrcReg->AreaSize / y;

	if( TrcReg->PtrgDnum >= TrcExeP->TrcDnum )
	{
		return PRM_RSLT_CALC_ERR;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace data buffer																	*/
	/*----------------------------------------------------------------------------------------------*/
	x = ( TrcExeP->TrcDataType == DATATRACE_SHORT ) ? TrcExeP->TrcDnum : ( 2 * TrcExeP->TrcDnum );
//	y = TRC_CH_MAXNUM + Adj;
	y = TRC_CH_MAXNUM;			/* <S1B6>  +Adjwill cause the area to overflow */
	z = TrcReg->TrcMode + Adj;
	for( i=0; i < y; i++ )
	{
		if( z > i )
		{
			TrcExeP->TrcObj[i].TrcBuf = TraceBuffer + (i*x);
		}
		else
		{
			TrcExeP->TrcObj[i].TrcBuf = NULL;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace data sampling interval															*/
	/*----------------------------------------------------------------------------------------------*/
	if(TrcReg->SampIntv > 64)			/* Sampling interval without overflow 2^(30-24)=64		*/
	{
		TrcExeP->DataSampIntv = 64;
	}
	else
	{
		TrcExeP->DataSampIntv = TrcReg->SampIntv;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace target individual parameter calculation															*/
	/*----------------------------------------------------------------------------------------------*/
	for( i=0; i < TRC_CH_MAXNUM; i++ )
	{
		if( dtrcCalcTraceObject( TrcHndl, i ) == FALSE )
		{
			return PRM_RSLT_CALC_ERR;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trigger condition setting																			*/
	/*----------------------------------------------------------------------------------------------*/
	dtrcSetTrgParameter( SINGLE_TRG, Adj,TrcExeP, TrcReg );

	/*----------------------------------------------------------------------------------------------*/
	/*		Multitrigger condition setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( (TrcReg->MTrgMode == 1) || (TrcReg->MTrgMode == 2) )
	{
		dtrcSetTrgParameter( MULTI_TRG, Adj, TrcExeP, TrcReg );
	}
	else
	{
		dtrcClearTrgParameter( &TrcExeP->TrcTrg[MULTI_TRG] );
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Data trace parameter calculation complete														*/
	/*----------------------------------------------------------------------------------------------*/
	TrcExeP->TrcPrmOk = TRUE;							/* Data trace parameter OK set		*/
	return( PRM_RSLT_SUCCESS );							/* Parameter calculation completion return				*/

}

/****************************************************************************************************/
/*																									*/
/*		Data trace Trigger condition setting																*/
/*																									*/
/****************************************************************************************************/
static void dtrcSetTrgParameter( LONG TrgNo, SHORT Adj, TRCEXEP *TrcExeP, TRCREG *TrcReg )
{
#if 0	/* <S0C9> Delete Start */
	LONG x,y,z;
	TRCTRG *TrcTrg;
#if CSW_NET_TRG_USE == FALSE		/* <S012> */
	TrcTrg = &TrcExeP->TrcTrg[TrgNo];
	
	x = (TrcReg->TrgSel       & 0x000F);									/* Trigger data selection		*/
	y = (TrcReg->TrgSel >> 4) & 0x000F;										/* Select trigger target		*/
	z = (TrcReg->TrgSel >> 8) & 0x000F;										/* Select trigger target		*/

	if(z > 0)
	{
		if(y == 0)															/* Numerical level trigger		*/
		{
			TrcTrg->TrgEdge  = (USHORT)z;
			TrcTrg->TrgBitSetMsk = 0;
			TrcTrg->TrgBufx  = TrcExeP->TrcObj[x].TrcBuf;
			TrcTrg->TrgVsft  = ( TrcExeP->TrcDataType == DATATRACE_SHORT ) ? ( 17-Adj ) : ( 1-Adj );
			TrcTrg->TrgLevel = TrcReg->TrgLevel;
			TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 
																			0 : TrcReg->TrgSel;
		}
		else if( (0 < y)&&(y < TrcReg->TrcMode) )							/* Bit trigger			*/
		{
			TrcTrg->TrgEdge  = (USHORT)z;
			TrcTrg->TrgBitSetMsk = ( TrcExeP->TrcIOType == IODATA_MERGE ) ? 
																TrcExeP->TrcBitSet : ( 0x01 << x );
			x  = ( TrcExeP->TrcIOType == IODATA_MERGE )? x : TrcReg->TrcMode;
			TrcTrg->TrgBufx  = TrcExeP->TrcObj[x].TrcBuf;
			TrcTrg->TrgVsft  = 0;
			TrcTrg->TrgLevel = 0;
			TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 
																			0 : TrcReg->TrgSel;
		}
	}
	else																	/* Trigger not specified		*/
	{	/* Clear trigger variable */
		dtrcClearTrgParameter( TrcTrg );
	}
#else /* CSW_NET_TRG_USE */		/* <S012> */
	TrcTrg = &TrcExeP->TrcTrg[TrgNo];

	x = (TrcReg->TrgSel       & 0x000F);									/* Trigger data selection		*/
	y = (TrcReg->TrgSel >> 4) & 0x000F;										/* Trigger type selection		*/
//	y = TrcReg.TrgLevel.Long;												/* Trigger data selection		*/

	if( (1 <= x)&&(x <= 7) )
	{ /* Numerical level trigger */
		TrcTrg->TrgEdge  = (USHORT)y;
		TrcTrg->TrgBufx  = TrcExeP->TrcObj[x-1].TrcBuf;
		TrcTrg->TrgVsft  = (TrcExeP->TrcDataType == DATATRACE_SHORT) ? ( 17-Adj ) : ( 1-Adj );
		TrcTrg->TrgLevel = TrcReg->TrgLevel;
		TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
	}
	else if( (x == 9)&&(1 <= TrcReg->TrgLevel)&&(TrcReg->TrgLevel <= 7) )
	{ /* Bit trigger */
		TrcTrg->TrgEdge  = (USHORT)y;
		TrcTrg->TrgBufx  = TrcExeP->TrcObj[y-1].TrcBuf;
		TrcTrg->TrgVsft  = 0;
		TrcTrg->TrgLevel = 0;
		TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0x00)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
	}
	else if( y == NET_TRG_SEL ) /* <S021> */
	{ /* Network trigger */
		TrcTrg->TrgEdge  = 0;
		TrcTrg->TrgBufx  = NULL;
		TrcTrg->TrgVsft  = 0;
		TrcTrg->TrgLevel = 0;
		TrcTrg->TrgSel   = TrcReg->TrgSel;
	}
	else
	{ /* Trigger not specified */
		TrcTrg->TrgEdge  = 0;
		TrcTrg->TrgBufx  = NULL;
		TrcTrg->TrgVsft  = 0;
		TrcTrg->TrgLevel = 0;
		TrcTrg->TrgSel   = 0;
	}
#endif
#endif	/* <S0C9> Delete End */


/* <S0C9> Start */
//	LONG x,y,z; /* <Oka02> */
	LONG	TrgDataSel;
	LONG	TrgTypeSel;
	LONG	TrgDataSel2;
	LONG	TrgTypeSel2;
	TRCTRG	*TrcTrg;

#if 0	/* <S142> */
	TrcTrg = &TrcExeP->TrcTrg[TrgNo];

/* <Oka02> */
	if( TrgNo == MULTI_TRG )
	{
		TrcReg->TrgSel   = TrcReg->MTrgSel;
		TrcReg->TrgLevel = TrcReg->MTrgLevel;
	}
/* <Oka02> */

	/* Trigger information acquisition */
	TrgDataSel = (TrcReg->TrgSel & 0x000F);				/* Trigger data selection		*/
	TrgTypeSel = (TrcReg->TrgSel >> 4) & 0x000F;		/* Trigger condition selection		*/
	TrgDataSel2 = (TrcReg->TrgSel >> 8) & 0x000F;		/* Trigger data selection2	*/
	TrgTypeSel2 = (TrcReg->TrgSel >> 12) & 0x000F;		/* Select trigger target2		*/
#else
	USHORT	TrgSel;
	LONG	TrgLevel;
	
	TrcTrg = &TrcExeP->TrcTrg[TrgNo];

	if( TrgNo == SINGLE_TRG )
	{
		TrgSel   = TrcReg->TrgSel;
		TrgLevel = TrcReg->TrgLevel;
	}
	else
	{
		TrgSel   = TrcReg->MTrgSel;
		TrgLevel = TrcReg->MTrgLevel;
	}

	/* Trigger information acquisition */
	TrgDataSel = (TrgSel & 0x000F);				/* Trigger data selection		*/
	TrgTypeSel = (TrgSel >> 4) & 0x000F;		/* Trigger condition selection		*/
	TrgDataSel2 =(TrgSel >> 8) & 0x000F;		/* Trigger data selection2	*/
	TrgTypeSel2 = (TrgSel >> 12) & 0x000F;		/* Select trigger target2		*/
#endif

/* <S0D2> Start */
#if 0
/* Commented out due to process reconfiguration Start */
	if( TrgDataSel != 0 )
	{	/* Trigger specified */

		if( TrgDataSel < 15 )
		{	/* Trigger2Unused */
			if( TrgTypeSel == NET_TRG_SEL )
			{ /* Network trigger */
				TrcTrg->TrgEdge  = 0;
				TrcTrg->TrgBitSetMsk = 0;
				TrcTrg->TrgBufx  = NULL;
				TrcTrg->TrgVsft  = 0;
				TrcTrg->TrgLevel = 0;
				TrcTrg->TrgSel   = TrcReg->TrgSel;
			}
			else
			{
				if( (1 <= TrgDataSel)&&(TrgDataSel <= 8) )
				{ /* Numerical level trigger */
					TrcTrg->TrgEdge  = (USHORT)TrgTypeSel;
					TrcTrg->TrgBitSetMsk = 0;
					TrcTrg->TrgBufx  = TrcExeP->TrcObj[TrgDataSel-1].TrcBuf;
					TrcTrg->TrgVsft  = (TrcExeP->TrcDataType == DATATRACE_SHORT) ? ( 17-Adj ) : ( 1-Adj );
					TrcTrg->TrgLevel = TrcReg->TrgLevel;
					TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
				}
				else if( (TrgDataSel == 9)&&(1 <= TrcReg->TrgLevel)&&(TrcReg->TrgLevel <= 7) )
				{ /* Bit trigger */
					TrcTrg->TrgEdge  = (USHORT)TrgTypeSel;
					TrcTrg->TrgBitSetMsk = ( TrcExeP->TrcIOType == IODATA_MERGE ) ? TrcExeP->TrcBitSet : ( 0x01 << x );
					TrcTrg->TrgBufx  = TrcExeP->TrcObj[TrgTypeSel-1].TrcBuf;
					TrcTrg->TrgVsft  = 0;
					TrcTrg->TrgLevel = 0;
					TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0x00)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
				}
			}
		}
		else
		{/* Trigger2Using */
			TrcTrg->TrgEdge  = (USHORT)TrgTypeSel2;
			TrcTrg->TrgBitSetMsk = 0;
			TrcTrg->TrgBufx  = TrcExeP->TrcObj[TrgDataSel2-1].TrcBuf;
			TrcTrg->TrgVsft  = (TrcExeP->TrcDataType == DATATRACE_SHORT) ? ( 17-Adj ) : ( 1-Adj );
			TrcTrg->TrgLevel = TrcReg->TrgLevel;
			TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
		}
	}
	else
	{ /* Trigger not specified */
		TrcTrg->TrgEdge  = 0;
		TrcTrg->TrgBitSetMsk = 0;
		TrcTrg->TrgBufx  = NULL;
		TrcTrg->TrgVsft  = 0;
		TrcTrg->TrgLevel = 0;
		TrcTrg->TrgSel   = 0;
	}
/* Commented out due to process reconfiguration End */
#endif
/* <S0C9> End */


/*****************************************/
/*	Recreated process from here <S0D2>		*/
/****************************************/
	if( TrgTypeSel == NET_TRG_SEL )
	{ /* Network trigger */
		TrcTrg->TrgEdge  = 0;
		TrcTrg->TrgBitSetMsk = 0;
		TrcTrg->TrgBufx  = NULL;
		TrcTrg->TrgVsft  = 0;
		TrcTrg->TrgLevel = 0;
//		TrcTrg->TrgSel   = TrcReg->TrgSel;
		TrcTrg->TrgSel   = TrgSel;	/* <S142> */
	}
	else
	{ /* Other than network trigger */
		if( TrgDataSel != 0 )
		{ /* Trigger specified */
			if( TrgDataSel < 15 )
			{	/* Trigger2Unused */
					if( (1 <= TrgDataSel)&&(TrgDataSel <= 8) )
					{ /* Numerical level trigger */
						TrcTrg->TrgEdge  = (USHORT)TrgTypeSel;
						TrcTrg->TrgBitSetMsk = 0;
						TrcTrg->TrgBufx  = TrcExeP->TrcObj[TrgDataSel-1].TrcBuf;
						TrcTrg->TrgVsft  = (TrcExeP->TrcDataType == DATATRACE_SHORT) ? ( 17-Adj ) : ( 1-Adj );
//						TrcTrg->TrgLevel = TrcReg->TrgLevel;
						TrcTrg->TrgLevel = TrgLevel;			/* <S142> */
//						TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
						TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 0 : TrgSel;		/* <S142> */
					}
//					else if( (TrgDataSel == 9)&&(1 <= TrcReg->TrgLevel)&&(TrcReg->TrgLevel <= 7) )
					else if( (TrgDataSel == 9)&&(1 <= TrgLevel)&&(TrgLevel <= 7) )	/* <S142> */
					{ /* Bit trigger */
						TrcTrg->TrgEdge  = (USHORT)TrgTypeSel;
//						TrcTrg->TrgBitSetMsk = ( TrcExeP->TrcIOType == IODATA_MERGE ) ? TrcExeP->TrcBitSet : ( 0x01 << x ); /* <Oka02> */
						TrcTrg->TrgBitSetMsk = ( TrcExeP->TrcIOType == IODATA_MERGE ) ? TrcExeP->TrcBitSet : ( 0x01 << TrgDataSel ); /* <Oka02> */
						TrcTrg->TrgBufx  = TrcExeP->TrcObj[TrgDataSel-1].TrcBuf;
						TrcTrg->TrgVsft  = 0;
						TrcTrg->TrgLevel = 0;
//						TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0x00)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
						TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0x00)||(TrcTrg->TrgBufx == NULL))? 0 : TrgSel;	/* <S142> */
					}
			}
			else
			{ /* Trigger2Using */
				if( TrgTypeSel2 == 0 )
				{/* Numerical level trigger */
						TrcTrg->TrgEdge  = (USHORT)TrgTypeSel;
						TrcTrg->TrgBitSetMsk = 0;
						TrcTrg->TrgBufx  = TrcExeP->TrcObj[TrgDataSel2].TrcBuf;
						TrcTrg->TrgVsft  = (TrcExeP->TrcDataType == DATATRACE_SHORT) ? ( 17-Adj ) : ( 1-Adj );
//						TrcTrg->TrgLevel = TrcReg->TrgLevel;
						TrcTrg->TrgLevel = TrgLevel;	/* <S142> */
//						TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
						TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0)||(TrcTrg->TrgBufx == NULL))? 0 : TrgSel;	/* <S142> */
				}
				else
				{ /* Bit trigger */
					TrcTrg->TrgEdge  = (USHORT)TrgTypeSel;
//					TrcTrg->TrgBitSetMsk = ( TrcExeP->TrcIOType == IODATA_MERGE ) ? TrcExeP->TrcBitSet : ( 0x01 << x ); /* <Oka02> */
					TrcTrg->TrgBitSetMsk = ( TrcExeP->TrcIOType == IODATA_MERGE ) ? TrcExeP->TrcBitSet : ( 0x01 << TrgDataSel2 ); /* <Oka02> */
//					TrcTrg->TrgBufx  = TrcExeP->TrcObj[TrgDataSel2].TrcBuf;		/* <Oka02> */
					TrcTrg->TrgBufx  = TrcExeP->TrcObj[TrcReg->TrcMode].TrcBuf;	/* <Oka02> */
					TrcTrg->TrgVsft  = 0;
					TrcTrg->TrgLevel = 0;
//					TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0x00)||(TrcTrg->TrgBufx == NULL))? 0 : TrcReg->TrgSel;
					TrcTrg->TrgSel   = ((TrcTrg->TrgEdge == 0x00)||(TrcTrg->TrgBufx == NULL))? 0 : TrgSel;	/* <S142> */
				}
			}
		}
		else
		{ /* Trigger not specified */
			TrcTrg->TrgEdge  = 0;
			TrcTrg->TrgBitSetMsk = 0;
			TrcTrg->TrgBufx  = NULL;
			TrcTrg->TrgVsft  = 0;
			TrcTrg->TrgLevel = 0;
			TrcTrg->TrgSel   = 0;
		}
	}
/* <S0D2> End */

}

/****************************************************************************************************/
/*																									*/
/*		Data trace Clear trigger variable																*/
/*																									*/
/****************************************************************************************************/
static void dtrcClearTrgParameter( TRCTRG *TrcTrg )
{
	TrcTrg->TrgSel   = 0;
	TrcTrg->TrgEdge  = 0;
	TrcTrg->TrgVsft  = 0;
	TrcTrg->TrgLevel = 0;
	TrcTrg->TrgBitSetMsk = 0;
	TrcTrg->TrgBufx  = NULL;
}

/****************************************************************************************************/
/*																									*/
/*		Data trace Axis number conversion function																*/
/*																									*/
/****************************************************************************************************/
static void* dtrcConvAxisAddress( MEMOBUS_ADDR_MAP *AxisMap, LONG Address, LONG *AxisNum )
{
	LONG i;
//	for(i = 0; i < NumberOfAxes; i++)
	for(i = 0; i < MAX_AXIS_NUMBER; i++)
	{
		if(AxisMap[i].AxisAddress == Address)
		{
			*AxisNum = i;
			return AxisMap[i].Axis;
		}
	}
	*AxisNum = -1;
	return NULL;
}

/****************************************************************************************************/
/*																									*/
/*		Data trace AXISStart address acquisition function														*/
/*																									*/
/****************************************************************************************************/
static void* dtrcGetTopAxisAddress( MEMOBUS_ADDR_MAP *AxisMap )
{
	return AxisMap[0].Axis;
}

/****************************************************************************************************/
/*																									*/
/*		Data trace target individual parameter calculation process													*/
/*																									*/
/****************************************************************************************************/
BOOL	dtrcCalcTraceObject( TRCHNDL *TrcHndl, LONG Tno )
{
	LONG	i,AxisAddress;
	LONG	TrcSelNum,AxisNum;
	void	*AxisPtr;

	TRCREG	*TrcReg;
	TRCEXEP	*TrcExeP;
	TRCOBJ	*TrcObj;
	TRCINDIVPRM	*TrcIncivPrm;				/* <S0C9> */

	TrcReg  = &TrcHndl->TrcReg;
	TrcExeP = &TrcHndl->TrcExeP;
	TrcObj  = &TrcHndl->TrcExeP.TrcObj[Tno];
	TrcIncivPrm = &TrcHndl->TrcIncivPrm;	/* <S0C9> */
	AxisPtr = NULL;

	/*----------------------------------------------------------------------------------------------*/
	/*		Trace selection check																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrcObj->TrcBuf == NULL )
	{
		return( TRUE );
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Initialization of trace object															*/
	/*----------------------------------------------------------------------------------------------*/
	TrcObj->TrcExeflg = 0x00;
	TrcObj->Adr.LongAdrData = NULL;
	MlibResetByteMemory( &TrcObj->TrcV, sizeof(TrcObj->TrcV) );
	MlibResetByteMemory( &TrcObj->TrcB, sizeof(TrcObj->TrcB) );
	/*----------------------------------------------------------------------------------------------*/
	/*		Numerical trace target data setting																*/
	/*----------------------------------------------------------------------------------------------*/
/* <S0C9>	Start */
//	TrcSelNum = TrcReg->TrcSel[Tno] & 0x00FF;
//	AxisAddress = (TrcReg->TrcSel[Tno] >> 8) & 0x00FF;					

	TrcSelNum = TrcIncivPrm->TrcData[Tno];
	AxisAddress = TrcIncivPrm->TrcAxis[Tno];
/* <S0C9>	End */
	AxisPtr = dtrcConvAxisAddress( TrcHndl->AxisMap, AxisAddress, &AxisNum );
	if( (TrcSelNum == 0x7F) || (TrcSelNum == 0xFF) )							/* Real address specification	*/
	{		
		if( (LONG*)(TrcReg->TrcAdr[Tno]) == 0x00000000 )
		{
			/* Trace dummy variable if address specification is initial value */
			TrcObj->TrcV.GetData = GetVarData_Dummy;
		}
		else
		{
			if( TrcExeP->TrcDataType == DATATRACE_SHORT )
			{
				/* Short data trace	*/
				TrcObj->Adr.ShortAdrData = (USHORT*)TrcReg->TrcAdr[Tno];	
				TrcObj->TrcV.GetData = GetVarData_AdrShortData;
			}
			else
			{
				/* Long data trace		*/
				TrcObj->Adr.LongAdrData = TrcReg->TrcAdr[Tno];	
				TrcObj->TrcV.GetData = GetVarData_AdrLongData;
			}
		}
		TrcObj->TrcV.AvrgSel = FALSE;
		TrcObj->TrcV.VxSftL = 0;
		TrcObj->TrcV.VxSftR = ( TrcExeP->TrcDataType == DATATRACE_SHORT )? 16 : 0;
		TrcObj->TrcV.VxGain = 1;
		TrcObj->TrcV.AxisPtr = dtrcGetTopAxisAddress( TrcHndl->AxisMap );
/* <S0C9>	Start */
//		TrcReg->IOTrcSel[Tno] = 0x00;
		TrcIncivPrm->IOTrcData[Tno] = 0x00;
/* <S0C9>	End */	}
	else if( (AxisNum < 0) || (AxisNum > (MAX_AXIS_NUMBER - 1)) )				/* Out of axis setting range		*/
	{	
		return( FALSE );
	}
	else if( (i = DtrcSearchTrcVarTbl( TrcHndl, TrcSelNum )) >= 0 )				/* Numerical trace setting	*/
	{
		/*------------------------------------------------------------------------------------------*/
		/*	Trace averaging selection (TRUE⇒Average,FALSE⇒Do not average)								*/
		/*------------------------------------------------------------------------------------------*/
		TrcObj->TrcV.AvrgSel = TrcVarTbl[i].AvrgSel;
		/*------------------------------------------------------------------------------------------*/
		/*	Set function pointer to obtain trace variable												*/
		/*------------------------------------------------------------------------------------------*/
		if( NULL != TrcVarTbl[i].GetVarData )
		{
			TrcObj->TrcV.GetData = TrcVarTbl[i].GetVarData;
			TrcObj->TrcV.AxisPtr = AxisPtr;
		}
		else
		{
			TrcObj->TrcV.GetData = GetVarData_Dummy;
			TrcObj->TrcV.AxisPtr = NULL;
		}
		/*------------------------------------------------------------------------------------------*/
		/* Set the function pointer to calculate the gain of the trace variable (default:Gain=1)					*/
		/*------------------------------------------------------------------------------------------*/
		if( NULL != TrcVarTbl[i].GetVarGain && TrcObj->TrcV.AxisPtr != NULL )
		{
			TrcObj->TrcV.VxGain = TrcVarTbl[i].GetVarGain( TrcObj->TrcV.AxisPtr );
		}
		else
		{
			TrcObj->TrcV.VxGain = 1;
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Set bit shift amount																	*/
		/*------------------------------------------------------------------------------------------*/
		TrcObj->TrcV.VxSftL = TrcVarTbl[i].VxSftL;
		TrcObj->TrcV.VxSftR = TrcVarTbl[i].VxSftR;
	}
	else
	{	/* No trace target */
		return( FALSE );
	}

#if 0 /* The address may be an odd number @@CHECK */  /* <S001> */
	/*----------------------------------------------------------------------------------------------*/
	/*		Numeric trace variable address check														*/
	/*----------------------------------------------------------------------------------------------*/
	if( (LONG)TrcObj->TrcV.GetData & 0x01 )
	{	/* Setting address error */
		TrcObj->TrcExeflg  = 0x00;
		TrcObj->Adr.LongAdrData = NULL;
		MlibResetByteMemory( &TrcObj->TrcV, sizeof(TrcObj->TrcV) );
		return( FALSE );
	}
#endif /* <S001> */

	/*----------------------------------------------------------------------------------------------*/
	/*		Bit trace target data setting															*/
	/*----------------------------------------------------------------------------------------------*/
/* <S0C9>	Start */
//	TrcSelNum = (TrcReg->IOTrcSel[Tno] & 0xFF);
//	AxisAddress = (TrcReg->IOTrcSel[Tno] >> 8) & 0xFF;

	TrcSelNum = TrcIncivPrm->IOTrcData[Tno];
	AxisAddress = TrcIncivPrm->IOTrcAxis[Tno];
/* <S0C9>	End */

	AxisPtr = dtrcConvAxisAddress( TrcHndl->AxisMap, AxisAddress, &AxisNum );

	if( (AxisNum < 0) || (AxisNum > MAX_AXIS_NUMBER - 1) )					/* Out of axis setting range			*/
	{	
		return( FALSE );
	}
	if( (i = dtrcSearchTrcBitTbl( TrcHndl, TrcSelNum )) >= 0 )				/* Bit trace setting	*/
	{
		if( NULL != TrcBitTbl[i].GetBitData )
		{
			TrcObj->TrcB.GetData = TrcBitTbl[i].GetBitData;
			TrcObj->TrcB.AxisPtr = AxisPtr;
		}
		else
		{
			TrcObj->TrcB.GetData = NULL;
			TrcObj->TrcB.AxisPtr = NULL;
		}
	}
	else
	{
		return( FALSE );
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace execution flag setting																	*/
	/*----------------------------------------------------------------------------------------------*/
	TrcObj->TrcExeflg = 0x01;
	return( TRUE );
}

/****************************************************************************************************/
/*																									*/
/*		Data trace target variable table search process														*/
/*																									*/
/****************************************************************************************************/
LONG	DtrcSearchTrcVarTbl( TRCHNDL *TrcHndl, LONG SelNo )
{
	LONG	i,w;
	LONG	EntNum;

	const	TVXDEF *TvxTbl;

	TRCMNGP	*TrcMngP;
	TrcMngP = &TrcHndl->TrcMngP;

	/*----------------------------------------------------------------------------------------------*/
	/*		For searchData																		*/
	/*----------------------------------------------------------------------------------------------*/
	TvxTbl = TrcVarTbl;										/* Definition table pointer					*/
	EntNum = TrcMngP->TrcVarTblEntNum;						/* Number of definition table registrations					*/
	w = TrcMngP->TrcVarTblSrhIniW;							/* Setting division width initial value						*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Search startIndex																		*/
	/*----------------------------------------------------------------------------------------------*/
		i = ( SelNo >= TvxTbl[w].SelNo )? (EntNum - w) : 0;	/* Search startIndexSetting					*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Table search																			*/
	/*----------------------------------------------------------------------------------------------*/
	for( w = w>>1; w > 0; w = w>>1 )						/* SearchLoop								*/
	{
		if( SelNo >= TvxTbl[i+w].SelNo ){ i = i + w; }		/* Comparison &IndexUpdate						*/
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Search result check																		*/
	/*----------------------------------------------------------------------------------------------*/
	if( SelNo == TvxTbl[i].SelNo )							/* Search result check						*/
	{
		return( i );										/* Search completed								*/
	}
	else
	{
		return( -1 );										/* Search failure(SelNoNo capacity)					*/
	}

}

/****************************************************************************************************/
/*																									*/
/*		Data trace target bit table search process													*/
/*																									*/
/****************************************************************************************************/
LONG	dtrcSearchTrcBitTbl( TRCHNDL *TrcHndl, LONG SelNo )
{
	LONG	i,w;
	LONG	EntNum;

	const	TBXDEF* TbxTbl;
	
	TRCMNGP	*TrcMngP;
	TrcMngP = &TrcHndl->TrcMngP;

	/*----------------------------------------------------------------------------------------------*/
	/*		For searchData																		*/
	/*----------------------------------------------------------------------------------------------*/
	TbxTbl = TrcBitTbl;										/* Definition table pointer					*/
	EntNum = TrcMngP->TrcBitTblEntNum;						/* Number of definition table registrations					*/
	w = TrcMngP->TrcBitTblSrhIniW;							/* Setting division width initial value						*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Search startIndex																		*/
	/*----------------------------------------------------------------------------------------------*/
	i = ( SelNo >= TbxTbl[w].SelNo )? (EntNum - w) : 0;		/* Search startIndexSetting					*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Table search																			*/
	/*----------------------------------------------------------------------------------------------*/
	for( w = w>>1; w > 0; w = w>>1 )						/* SearchLoop								*/
	{
		if( SelNo >= TbxTbl[i+w].SelNo ){ i = i + w;}		/* Comparison &IndexUpdate						*/
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Search result check																		*/
	/*----------------------------------------------------------------------------------------------*/
	if( SelNo == TbxTbl[i].SelNo )							/* Search result check						*/
	{
		return( i );										/* Search completed								*/
	}
	else
	{
		return( -1 );										/* Search failure(SelNoNo capacity)					*/
	}
}

/****************************************************************************************************/
/*																									*/
/*		Trace operation mode register write processing														*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	DtrcWriteOpeModeTrcRegister( TRCHNDL *TrcHndl, USHORT WriteData )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Normal mode setting																			*/
	/*----------------------------------------------------------------------------------------------*/
	if( WriteData == 0 )
	{
		dtrcFinishOpeModeOperation( TrcHndl );				/* Operation mode operation end processing				*/
		return( PRM_RSLT_SUCCESS );
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace mode setting: Single shot or Continuous														*/
	/*----------------------------------------------------------------------------------------------*/
	else if( (WriteData == OPEMODE_DATATRACE) || (WriteData == OPEMODE_RINGTRACE) )
	{
		if ( TrcHndl->TrcExeV.Opened == TRUE )
		{
			return ( PRM_RSLT_CALC_ERR );
		}
		/*------------------------------------------------------------------------------------------*/
		/* @@ PrmCopyFunctionTraceBufferMake it exclusive to use 									*/
		/*------------------------------------------------------------------------------------------*/
		//else if ( CTRCMEM->GetBuffer( CTRCMEM, (void *)TrcHndl ) == NULL )
		//{
		//	return ( PRM_RSLT_NOACCRIGHT_ERR );								/* Confirmation required */
		//}
		/*------------------------------------------------------------------------------------------*/
		else
		{
			TrcHndl->TrcExeV.Opened = TRUE;
			TrcHndl->TrcExeV.OpmRun = TRUE;
			return( PRM_RSLT_SUCCESS );
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Others																					*/
	/*----------------------------------------------------------------------------------------------*/
	else
	{
		return( PRM_RSLT_CALC_ERR );
	}
}

/****************************************************************************************************/
/*																									*/
/*		Operation mode operation end processing																		*/
/*																									*/
/****************************************************************************************************/
static BOOL	dtrcFinishOpeModeOperation( TRCHNDL *TrcHndl )
{
	TRCREG	*TrcReg;
	TrcReg = &TrcHndl->TrcReg;

	/*----------------------------------------------------------------------------------------------*/
	/*		Mode check & stop processing																*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrcReg->OpeModeTrc == OPEMODE_NOCMD )
	{														
		dtrcStopDataTrace( TrcHndl );								/* Processing when there is no command		*/
	}
	else if( TrcReg->OpeModeTrc == OPEMODE_DATATRACE )
	{														
		dtrcStopDataTrace( TrcHndl );								/* Single-shot data trace stop processing	*/
	}
	else if( TrcReg->OpeModeTrc == OPEMODE_RINGTRACE )				
	{
		dtrcStopRingTrace( TrcHndl, TRUE );							/* Continuous data trace stop processing	*/
	}
	//TrcHndl->TrcReg.OpeModeTrc = 0x0000;							/* Reset operation mode			*/
	TrcHndl->TrcReg.OpeCmdTrc  = 0x0000;							/* Reset operation command			*/

	//CTRCMEM->FreeBuffer( CTRCMEM, (void *)TrcHndl );				/* @@ Free TraceBuffer			*/
	return( TRUE );
}

/****************************************************************************************************/
/*																									*/
/*		Trace operation setting register write process															*/
/*																									*/
/****************************************************************************************************/
PRM_RSLT	DtrcWriteOpeCmdTrcRegister( TRCHNDL *TrcHndl, USHORT *TraceBuffer, USHORT WriteData )
{
	PRM_RSLT ErrRes;
	LONG size;

	TRCREG	*TrcReg;
	TrcReg = &TrcHndl->TrcReg;

	/*----------------------------------------------------------------------------------------------*/
	/*		Trace mode: Single shot or Continuous															*/
	/*----------------------------------------------------------------------------------------------*/
	if( (TrcReg->OpeModeTrc == OPEMODE_DATATRACE ) || (TrcReg->OpeModeTrc == OPEMODE_RINGTRACE) )
	{
		/* Buffer size check */
//#if CSW_NET_TRG_USE == TRUE		/* <S012> *//* <S021> */
//		size = TrcReg->AreaSize;
//#else /* CSW_NET_TRG_USE */
//		size = TrcReg->TrcMode * TrcReg->AreaSize;	/* <S0C9>	Delete */
		size = TrcReg->AreaSize;					/* <S0C9> */
//#endif /* CSW_NET_TRG_USE */
		if(size > TRC_BUF_SIZE)
		{
			return PRM_RSLT_CALC_ERR;
		}

		/* Trace operation command setting processing */
		if( TRUE == dtrcSetDataTraceCmd( TrcHndl, TrcReg->OpeModeTrc, WriteData ) )
		{ 
			ErrRes = PRM_RSLT_SUCCESS;
		}
		else
		{
			ErrRes = PRM_RSLT_CALC_ERR;		
		}
		return( ErrRes );
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Other modes																			*/
	/*----------------------------------------------------------------------------------------------*/
	else
	{
		return( PRM_RSLT_CALC_ERR );
	}
}

/****************************************************************************************************/
/*																									*/
/*		Trace operation command setting processing																*/
/*																									*/
/****************************************************************************************************/
static BOOL	dtrcSetDataTraceCmd( TRCHNDL *TrcHndl, ULONG OpeModeTrc, ULONG WriteData )
{
	LONG	rc;
	TRCREG	*TrcReg;
	TrcReg = &TrcHndl->TrcReg;

	/*----------------------------------------------------------------------------------------------*/
	/*		Trace mode: Single shot																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrcReg->OpeModeTrc == OPEMODE_DATATRACE )
	{
		if( WriteData == 0x0000 )
		{
			rc = dtrcStopDataTrace( TrcHndl );						/* Single-shot data trace stop processing	*/
		}
		else if( WriteData == 0x0001 )
		{
			rc = dtrcStartDataTrace( TrcHndl, TRCCMD_DATATRACE );	/* Single-shot data trace start processing	*/
		}
		else if ( WriteData == 0x0011 )					
		{
			rc = dtrcStartDataTrace( TrcHndl, TRCCMD_CONTTRACE );
		}
		else
		{
			return( FALSE );										/* Command error					*/
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace mode: Continuous																	*/
	/*----------------------------------------------------------------------------------------------*/
	else
	{
		if( WriteData == 0x0000 )
		{
			rc = dtrcStopRingTrace( TrcHndl, FALSE );				/* Continuous data trace stop processing	*/
		}
		else if( WriteData == 0x0001 )
		{
			rc = dtrcStartRingTrace( TrcHndl );						/* Continuous data trace start processing	*/
		}
		else
		{
			return( FALSE );										/* Command error					*/
		}
	}
	return( rc );
}

/****************************************************************************************************/
/*																									*/
/*		Single data trace start processing																*/
/*																									*/
/****************************************************************************************************/
static BOOL	dtrcStartDataTrace( TRCHNDL *TrcHndl, UCHAR mode )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Single-shot trace start check															*/
	/*----------------------------------------------------------------------------------------------*/
	if ( (TrcHndl->TrcExeV.TrcCmd != TRCCMD_CONTTRACE) || (TrcHndl->TrcExeV.OpmRun != TRUE) )
	{
		if( TrcHndl->TrcExeV.TrcCmd != TRCCMD_NOCMD )
		{
			return( FALSE );
		}
		else if( TrcHndl->TrcExeP.TrcPrmOk != TRUE )
		{
			return( FALSE );
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Single-shot trace start processing																*/
	/*----------------------------------------------------------------------------------------------*/
	DtrcResetAverageVar( TrcHndl );							/* Averaging variable initialization						*/
	TrcHndl->TrcReg.TrcState = 0;							/* Trace execution state reset				*/
	//TrcHndl->TrcExeP.TrcObj[0].TrcExeflg &= 0x0F;			/* Trace CH0 execution permission				*/
	TrcHndl->TrcExeV.TrcCmd = mode;							/* Single-shot trace execution command					*/
	DtrcInitExecute( TrcHndl );								/* Data trace execution control Initialization process	*/
															/* Trace execution command(orMode maintenance)		*/

	
	return( TRUE );
}

/****************************************************************************************************/
/*																									*/
/*		Single data trace stop processing																*/
/*																									*/
/****************************************************************************************************/
static BOOL	dtrcStopDataTrace( TRCHNDL *TrcHndl )
{
		TrcHndl->TrcExeV.TrcCmd = TRCCMD_NOCMD;
		TrcHndl->TrcExeV.OpmRun = FALSE;
		TrcHndl->TrcExeV.Opened = FALSE;
	
		return( TRUE );
}

/****************************************************************************************************/
/*																									*/
/*		Continuous data trace start processing																*/
/*																									*/
/****************************************************************************************************/
static BOOL	dtrcStartRingTrace( TRCHNDL *TrcHndl )
{
		/*------------------------------------------------------------------------------------------*/
		/*		Continuous trace start check														*/
		/*------------------------------------------------------------------------------------------*/
		if( TrcHndl->TrcExeV.TrcCmd != TRCCMD_NOCMD )
		{
			return( FALSE );
		}
		else if( TrcHndl->TrcExeP.TrcPrmOk != TRUE )
		{
			return( FALSE );
		}

		/*------------------------------------------------------------------------------------------*/
		/*		Continuous trace start processing															*/
		/*------------------------------------------------------------------------------------------*/
		DtrcResetAverageVar( TrcHndl );							/* Averaging variable initialization					*/
		TrcHndl->TrcReg.TrcState   = 0;							/* Trace execution state reset			*/
		//TrcHndl->TrcReg.RngTrcPtr  = 0;						/* Continuous trace write position reset	*/
		//TrcHndl->TrcExeV.RngTrcCnt = 0;						/* Continuous trace write count reset	*/
		//TrcHndl->TrcExeP.TrcObj[0].TrcExeflg &= 0x0F;			/* Trace CH0 execution permission			*/
		TrcHndl->TrcExeV.TrcCmd = TRCCMD_RINGTRACE;				/* Continuous trace execution command				*/
		DtrcInitExecuteRingTrc( TrcHndl );

		return( TRUE );

}

/****************************************************************************************************/
/*																									*/
/*		Continuous data trace stop processing																*/
/*																									*/
/****************************************************************************************************/
static BOOL	dtrcStopRingTrace( TRCHNDL *TrcHndl, LONG finmode )
{
		TrcHndl->TrcExeV.TrcCmd  = TRCCMD_NOCMD;
		TrcHndl->TrcExeV.OpmRun  = FALSE;
		TrcHndl->TrcReg.TrcState = 3;

		if(finmode == TRUE)	
		{
			TrcHndl->TrcExeV.Opened = FALSE;
		}
		return( TRUE );
}

/****************************************************************************************************/
/*																									*/
/*		Data trace execution control Initialization processing (single data trace)									*/
/*																									*/
/****************************************************************************************************/
void DtrcInitExecute( TRCHNDL *TrcHndl )
{
	TrcHndl->TrcReg.TrgPoint = 0;
	TrcHndl->TrcExeV.TrcIndx = 0;
	TrcHndl->TrcExeV.TrcStep = ((TrcHndl->TrcExeP.TrcTrg[0].TrgSel != 0x0000) 
								|| (TrcHndl->TrcExeP.TrcTrg[1].TrgSel != 0x0000)) ? 1 : 5;
}

/****************************************************************************************************/
/*																									*/
/*		Data trace execution control Initialization processing (continuous data trace)									*/
/*																									*/
/****************************************************************************************************/
void DtrcInitExecuteRingTrc( TRCHNDL *TrcHndl )
{
	TrcHndl->TrcReg.TrgPoint   = 0;							/* Reset trigger position				*/
	TrcHndl->TrcExeV.TrcIndx   = 0;							/* Trace index reset		*/
	TrcHndl->TrcExeV.TrcStep   = 7;							/* Trace execution step reset		*/
	TrcHndl->TrcExeV.RngTrcCnt = 0;							/* Continuous trace write count reset	*/
	TrcHndl->TrcReg.RngTrcPtr  = 0;							/* Continuous trace write position reset	*/
}

/****************************************************************************************************/
/*																									*/
/*		Data trace execution control																		*/
/*																									*/
/****************************************************************************************************/
void	DtrcExecute( TRCHNDL *TrcHndl )
{
	ULONG	ulwk;

	TRCREG	*TrcReg;
	TRCEXEP	*TrcExeP;
	TRCEXEV	*TrcExeV;

	TrcReg  = &TrcHndl->TrcReg;
	TrcExeP = &TrcHndl->TrcExeP;
	TrcExeV = &TrcHndl->TrcExeV;

	/*----------------------------------------------------------------------------------------------*/
	/*		Trace execution command check																*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrcExeV->TrcCmd == TRCCMD_NOCMD )
	{
		TrcExeV->TrcStep = 0;
		TrcExeV->TrcSamp = 0;
		return;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace data update processing																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrcExeV->TrcSamp >= (TrcReg->SampIntv - TrcExeP->DataSampIntv) )
	{
		DtrcSetTrcVar( TrcHndl );
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace execution sampling check														*/
	/*----------------------------------------------------------------------------------------------*/
	if( (++TrcExeV->TrcSamp >= TrcReg->SampIntv) || (TrcExeV->TrcStep == 0) )
	{
		TrcExeV->TrcSamp = 0;
	}
	else
	{
		return;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace execution sequence control																*/
	/*----------------------------------------------------------------------------------------------*/
	switch( TrcExeV->TrcStep )
	{
		/*------------------------------------------------------------------------------------------*/
		/*		Data Trace for PreTrigger															*/
		/*------------------------------------------------------------------------------------------*/
		case  1:
			TrcReg->TrcState = 1;
			DtrcProcedure( TrcHndl, TrcExeV->TrcIndx );
			if( ++TrcExeV->TrcIndx >= TrcReg->PtrgDnum )
			{
				TrcExeV->TrcStep = 2;
			}
			if( TrcExeV->TrcIndx >= TrcExeP->TrcDnum ){ TrcExeV->TrcIndx = 0;}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Data Trace and Wait Trigger															*/
		/*------------------------------------------------------------------------------------------*/
		case  2:
			TrcExeV->TrgVar[1]  = TrcExeV->TrgVar[0];
			TrcExeV->MTrgVar[1] = TrcExeV->MTrgVar[0];
			DtrcProcedure( TrcHndl, TrcExeV->TrcIndx );
			if( dtrcCheckTriggerProcedure( TrcHndl ) == TRUE )
			{
				TrcReg->TrcState = 2;
				TrcReg->TrgPoint = (USHORT)TrcExeV->TrcIndx;
				TrcExeV->TrcStep = ((TrcExeP->TrcDnum - TrcReg->PtrgDnum) > 1)? 3 : 4;
			}
			if( ++TrcExeV->TrcIndx >= TrcExeP->TrcDnum ){ TrcExeV->TrcIndx = 0;}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Data Trace and Check End WithTrigger												*/
		/*------------------------------------------------------------------------------------------*/
		case  3:
			DtrcProcedure( TrcHndl, TrcExeV->TrcIndx );
			if( ++TrcExeV->TrcIndx >= TrcExeP->TrcDnum ){ TrcExeV->TrcIndx = 0;}
			if( TrcReg->TrgPoint >= TrcReg->PtrgDnum )
			{
				ulwk = TrcReg->TrgPoint - TrcReg->PtrgDnum;
			}
			else
			{
				ulwk = TrcReg->TrgPoint + TrcExeP->TrcDnum - TrcReg->PtrgDnum;
			}
			if( TrcExeV->TrcIndx == ulwk )
			{
				TrcExeV->TrcStep = 4;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Data Trace End Procedure															*/
		/*------------------------------------------------------------------------------------------*/
		case  4:
			TrcReg->TrcState = 3;
			TrcExeV->TrcStep = 0;
			TrcExeV->TrcCmd  = TRCCMD_NOCMD;
			if ( TrcExeV->TrcCmd == TRCCMD_DATATRACE ) { TrcExeV->OpmRun = FALSE; }
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Data Trace Procedure(TriggerLess)													*/
		/*------------------------------------------------------------------------------------------*/
		case  5:
			TrcReg->TrcState = 1;
			DtrcProcedure( TrcHndl, TrcExeV->TrcIndx );
			if( ++TrcExeV->TrcIndx >= TrcExeP->TrcDnum )
			{
				TrcExeV->TrcIndx = 0;
				TrcExeV->TrcStep = 6;
			}
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Trace End Procedure(TriggerLess)													*/
		/*------------------------------------------------------------------------------------------*/
		case  6:
			TrcReg->TrcState = 3;
			TrcExeV->TrcStep = 0;
			TrcExeV->TrcCmd  = TRCCMD_NOCMD;
			if ( TrcExeV->TrcCmd == TRCCMD_DATATRACE ) { TrcExeV->OpmRun = FALSE; }
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Ring Data Trace																		*/
		/*------------------------------------------------------------------------------------------*/
		case  7:
			TrcReg->TrcState = 1;
			DtrcProcedure( TrcHndl, TrcExeV->TrcIndx );
			if( ++TrcExeV->TrcIndx >= TrcExeP->TrcDnum )
			{
				TrcExeV->TrcIndx = 0;
				TrcExeV->RngTrcCnt++;
			}
			TrcReg->RngTrcPtr = (TrcExeV->RngTrcCnt << 16) +TrcExeV->TrcIndx;
			break;
		/*------------------------------------------------------------------------------------------*/
		/*		Default(No Operation)																*/
		/*------------------------------------------------------------------------------------------*/
		case  0:
		default:
			break;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trace-only variable: Reset processing															*/
	/*----------------------------------------------------------------------------------------------*/
	DtrcResetAverageVar( TrcHndl );
}

/****************************************************************************************************/
/*																									*/
/*		Set Trace Variable Function																	*/
/*																									*/
/****************************************************************************************************/
void	DtrcSetTrcVar( TRCHNDL *TrcHndl )
{
	LONG i;

	TRCREG	*TrcReg;
	TRCOBJ	*Tobjpx;

	TrcReg  = &TrcHndl->TrcReg;

	for( i=0; i<TrcReg->TrcMode; i++ )
	{
		Tobjpx = &TrcHndl->TrcExeP.TrcObj[i];
		
		/* Numeric trace */
		if(Tobjpx->TrcV.AvrgSel == TRUE )
		{
			/* Add Average Trace Variable */
			Tobjpx->TrcV.VxData += Tobjpx->TrcV.GetData( Tobjpx->TrcV.AxisPtr );
		}
		else
		{
			/* Set Normal Trace Variable */
			Tobjpx->TrcV.VxData = Tobjpx->TrcV.GetData( Tobjpx->TrcV.AxisPtr );
		}

		/* Bit trace */
		if( Tobjpx->TrcB.GetData != NULL )
		{
			/* Set Bit Trace Variable */
			Tobjpx->TrcB.BtvData = Tobjpx->TrcB.GetData( Tobjpx->TrcB.AxisPtr );
		}
		else
		{
			Tobjpx->TrcB.BtvData = FALSE;		
		}
	}
}

/****************************************************************************************************/
/*																									*/
/*		Reset Average Variable Function																*/
/*																									*/
/****************************************************************************************************/
void	DtrcResetAverageVar( TRCHNDL *TrcHndl )
{
	LONG i;

	TRCREG	*TrcReg;
	TRCOBJ	*Tobjpx;

	TrcReg  = &TrcHndl->TrcReg;

	for(i=0;i<TrcReg->TrcMode;i++)
	{
		Tobjpx = &TrcHndl->TrcExeP.TrcObj[i];
		Tobjpx->TrcV.VxData = 0;
	}
}
/****************************************************************************************************/
/*																									*/
/*		Get Trace Data (Bit Data Merged)															*/
/*																									*/
/****************************************************************************************************/
static LONG	dtrcGetMergeData( TRCEXEP *TrcExeP, TRCOBJ *Tobjpx )
{
	LONG	Data;

	if( Tobjpx->TrcExeflg == 0x01 )
	{
#if (FLOAT_USE==TRUE)
		Data = Tobjpx->TrcV.VxData * Tobjpx->TrcV.VxGain;
#else
		Data = MlibMulgainNolim( Tobjpx->TrcV.VxData , Tobjpx->TrcV.VxGain );
#endif /*FLOAT_USE==TRUE*/
		Data = ( Data << Tobjpx->TrcV.VxSftL) >> Tobjpx->TrcV.VxSftR;
		Data = MlibLIMITUL( Data, TrcExeP->UpperLmt, TrcExeP->LowerLmt );

		if( Tobjpx->TrcB.GetData != NULL )
		{
			if( Tobjpx->TrcB.BtvData )
			{
				Data = (Data | TrcExeP->TrcBitSet);
			}
			else
			{
				Data = (Data & TrcExeP->TrcBitClr);
			}
		}
		return Data;
	}
	return 0;
}

/****************************************************************************************************/
/*																									*/
/*		Get Each Trace Data ( Veriable Data and Bit Data )											*/
/*																									*/
/****************************************************************************************************/
static void	dtrcGetTrcData( TRCEXEP *TrcExeP, TRCOBJ *Tobjpx, LONG *VerData, LONG *BitData, LONG Idx )
{
	LONG	Data;
	ULONG	TrcBitSet, TrcBitClr;

	if( Tobjpx->TrcExeflg == 0x01 )
	{
#if (FLOAT_USE==TRUE)
		Data = Tobjpx->TrcV.VxData * Tobjpx->TrcV.VxGain;
#else
		Data = MlibMulgainNolim( Tobjpx->TrcV.VxData , Tobjpx->TrcV.VxGain );
#endif /* FLOAT_USE */
		Data = ( Data << Tobjpx->TrcV.VxSftL) >> Tobjpx->TrcV.VxSftR;
		*VerData = MlibLIMITUL( Data, TrcExeP->UpperLmt, TrcExeP->LowerLmt );

		TrcBitSet = (0x00000001 << Idx );
		TrcBitClr = ~TrcBitSet;

		if( Tobjpx->TrcB.BtvData )
		{
			*BitData = (*BitData | TrcBitSet);
		}
		else
		{
			*BitData = (*BitData & TrcBitClr);
		}
	}
	else
	{
		*VerData = 0;
		*BitData = 0;
	}
}

/****************************************************************************************************/
/*																									*/
/*		Data Trace Procedure																		*/
/*																									*/
/****************************************************************************************************/
void	DtrcProcedure( TRCHNDL *TrcHndl, LONG Idx )
{

	ULONG i;
	LONG VerData, BitData;
	SHORT x,y;

	TRCREG	*TrcReg;
	TRCEXEP	*TrcExeP;
	TRCEXEV	*TrcExeV;
	TRCOBJ	*Tobjpx;
	TRCOBJ	*TobjpxB;

	BitData = 0;				/* Uninitialized variable correction<S191> */

	TrcReg  = &TrcHndl->TrcReg;
	TrcExeP = &TrcHndl->TrcExeP;
	TrcExeV = &TrcHndl->TrcExeV;

	/*----------------------------------------------------------------------------------------------*/
	/*		Short data trace																	*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrcExeP->TrcDataType == DATATRACE_SHORT )	
	{
		if( TrcExeP->TrcIOType == IODATA_MERGE )						/* For the upper bitIOData storage	*/
		{
			for(i=0;i<TrcReg->TrcMode;i++)
			{
				Tobjpx = &TrcExeP->TrcObj[i];		
				VerData = dtrcGetMergeData( TrcExeP, Tobjpx );	
				((SHORT*)Tobjpx->TrcBuf)[Idx] = (SHORT)VerData;
			}
		}
		else															/* DedicatedIOStored in data area	*/
		{
			TobjpxB = &TrcExeP->TrcObj[TrcReg->TrcMode];		

			for(i=0;i<TrcReg->TrcMode;i++)
			{
				Tobjpx = &TrcExeP->TrcObj[i];		

				dtrcGetTrcData( TrcExeP, Tobjpx, &VerData, &BitData, i );
				((SHORT*)Tobjpx->TrcBuf)[Idx]  = (SHORT)(VerData); 
				((SHORT*)TobjpxB->TrcBuf)[Idx] = (SHORT)(BitData);
			}
		}
	
		if(TrcExeP->TrcTrg[0].TrgBufx != NULL)
		{
			TrcExeV->TrgVar[0] = ((SHORT*)TrcExeP->TrcTrg[0].TrgBufx)[Idx];	/* Update Trigger1 Data	*/	
		}
		if(TrcExeP->TrcTrg[1].TrgBufx != NULL)
		{
			TrcExeV->MTrgVar[0] = ((SHORT*)TrcExeP->TrcTrg[1].TrgBufx)[Idx];/* Update Trigger2 Data	*/
		}

	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Long data trace																	*/
	/*----------------------------------------------------------------------------------------------*/
	else											
	{
		if( TrcExeP->TrcIOType == IODATA_MERGE )						/* For the upper bitIOData storage	*/
		{
			for(i=0;i<TrcReg->TrcMode;i++)
			{
				Tobjpx = &TrcExeP->TrcObj[i];		
				VerData = dtrcGetMergeData( TrcExeP, Tobjpx );	
				((SHORT *)Tobjpx->TrcBuf)[2*Idx    ] = (SHORT)(VerData);	
				((SHORT *)Tobjpx->TrcBuf)[2*Idx + 1] = (SHORT)(VerData >> 16);	
			}
		}
		else															/* DedicatedIOStored in data area	*/
		{
			TobjpxB = &TrcExeP->TrcObj[TrcReg->TrcMode];		

			for(i=0;i<TrcReg->TrcMode;i++)
			{
				Tobjpx = &TrcExeP->TrcObj[i];		

				dtrcGetTrcData( TrcExeP, Tobjpx, &VerData, &BitData, i );
				((SHORT*)Tobjpx->TrcBuf)[2*Idx    ] = (SHORT)(VerData);	
				((SHORT*)Tobjpx->TrcBuf)[2*Idx + 1] = (SHORT)(VerData >> 16);	

				((SHORT*)TobjpxB->TrcBuf)[2*Idx    ] = (SHORT)(BitData);	
				((SHORT*)TobjpxB->TrcBuf)[2*Idx + 1] = (SHORT)(BitData >> 16);	
			}
		}
		
		if(TrcExeP->TrcTrg[0].TrgBufx != NULL)
		{
			x = ((SHORT*)TrcExeP->TrcTrg[0].TrgBufx)[2*Idx];					
			y = ((SHORT*)TrcExeP->TrcTrg[0].TrgBufx)[2*Idx +1];
			TrcExeV->TrgVar[0] = (y << 16) + x;							/* Update Trigger1 Data		*/
		}
		if(TrcExeP->TrcTrg[1].TrgBufx != NULL)
		{
			x = ((SHORT*)TrcExeP->TrcTrg[1].TrgBufx)[2*Idx];					
			y = ((SHORT*)TrcExeP->TrcTrg[1].TrgBufx)[2*Idx+1];			
			TrcExeV->MTrgVar[0] = (y << 16) + x;						/* Update Trigger2 Data		*/
		}
	}
}

/****************************************************************************************************/
/*																									*/
/*		Data trace trigger check processing															*/
/*																									*/
/****************************************************************************************************/
static BOOL	dtrcCheckTriggerProcedure( TRCHNDL *TrcHndl )
{
	LONG	x,y;
	TRCREG	*TrcReg;
	TRCEXEP	*TrcExeP;
	TRCEXEV *TrcExeV;

	TrcReg  = &TrcHndl->TrcReg;
	TrcExeP = &TrcHndl->TrcExeP;
	TrcExeV = &TrcHndl->TrcExeV;

	/*----------------------------------------------------------------------------------------------*/
	/*		Trigger check 1																		*/
	/*----------------------------------------------------------------------------------------------*/
#if CSW_NET_TRG_USE == TRUE		/* <S021> */
	if(((TrcExeP->TrcTrg[0].TrgSel >> 4) & 0x000F) == NET_TRG_SEL)
	{ /* Network trigger */
		x = TrcExeV->NetTrigger;
	}
	else if((TrcExeP->TrcTrg[0].TrgSel == 0x00))
#else
	if((TrcExeP->TrcTrg[0].TrgSel == 0x00))
#endif
	{
		x = TRUE;
		if( TrcReg->MTrgMode == 1 )	/* <S148> */
		{
			x = TRUE;										/* ANDTrigger							*/
		}
		else if( TrcReg->MTrgMode == 2 )
		{
			x = FALSE;										/* ORTrigger								*/
		}
	}
	else
	{
		x = dtrcCheckTrigger( SINGLE_TRG, TrcHndl, TrcExeV->TrgVar);
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Trigger check 2																		*/
	/*----------------------------------------------------------------------------------------------*/
	if(TrcExeP->TrcTrg[1].TrgSel == 0x00)
	{
		y = TRUE;
		if( TrcReg->MTrgMode == 1 )	/* <S148> */
		{
			y = TRUE;										/* ANDTrigger							*/
		}
		else if( TrcReg->MTrgMode == 2 )
		{
			y = FALSE;										/* ORTrigger								*/
		}
	}
	else
	{
		y = dtrcCheckTrigger( MULTI_TRG, TrcHndl, TrcExeV->MTrgVar);
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Multitrigger check																	*/
	/*----------------------------------------------------------------------------------------------*/
	if(TrcReg->MTrgMode == 1)								/* ANDTrigger							*/
	{
		return( x & y );
	}
	/*----------------------------------------------------------------------------------------------*/
	else if(TrcReg->MTrgMode == 2)							/* ORTrigger								*/
	{
		return( x | y );
	}
	/*----------------------------------------------------------------------------------------------*/
#if 0 /* <S021> */
#if CSW_NET_TRG_USE == TRUE		/* <S012> */
	else if(TrcReg->MTrgMode == NET_TRG_SEL)				/* Network trigger					*/
	{
		return( TrcExeV->NetTrigger );
	}
#endif
#endif
	else													/* Single trigger.							*/
	{
		return( x );
	}
}


/****************************************************************************************************/
/*																									*/
/*		Trigger check processing																			*/
/*																									*/
/****************************************************************************************************/
static BOOL	dtrcCheckTrigger( LONG TrgNo, TRCHNDL *TrcHndl, LONG *TrgVar )
{
	LONG	Data0;
	LONG	Data1;
//	ULONG	x;				/* <S0D2> */
	ULONG	TrgData;		/* <S0D2> */
	ULONG	TrgEdge;		/* <S0D2> */
	ULONG	TrgTarget;		/* <S0D2> */


	TRCREG	*TrcReg;
	TRCTRG	*TrcTrg;

	TrcReg  = &TrcHndl->TrcReg;
	TrcTrg  = TrcHndl->TrcExeP.TrcTrg;
	
#if 0	/* <S142> */
//	x = (TrcReg->TrgSel >> 4) & 0x0000000F;	/* Select trigger target */
	TrgData = (TrcReg->TrgSel) & 0x0000000F;			/* Trigger data selection */	/* <S0D2> */
	TrgEdge = (TrcReg->TrgSel >> 4 ) & 0x0000000F;		/* Trigger condition selection */	/* <S0D2> */
	TrgTarget = (TrcReg->TrgSel >> 12) & 0x0000000F;	/* Select trigger target */	/* <S0D2> */
#else
	TrgData   = (TrcTrg[TrgNo].TrgSel      ) & 0x0000000F;	/* Trigger data selection */
	TrgEdge   = (TrcTrg[TrgNo].TrgSel >> 4 ) & 0x0000000F;	/* Trigger condition selection */
	TrgTarget = (TrcTrg[TrgNo].TrgSel >> 12) & 0x0000000F;	/* Select trigger target */
#endif

	/* <S0D2>  Start*/
#if CSW_NET_TRG_USE == TRUE		/* <S012> */
	/*----------------------------------------------------------------------------------------------*/
	/*		Network trigger check(No check item)											*/
	/*----------------------------------------------------------------------------------------------*/
	if( TrgEdge == NET_TRG_SEL )
	{
		return( TRUE );
	}
#endif /* CSW_NET_TRG_USE */
	/* <S0D2> End */

	/*----------------------------------------------------------------------------------------------*/
	/*		Bit trigger check																	*/
	/*----------------------------------------------------------------------------------------------*/
//	if( (0 < x)&&(x < TrcReg->TrcMode) )	/* <S0D2> */
	if( ( TrgData == 0x0009) || ((TrgData == 0x000f)&&(TrgTarget == 0x0001) ) ) 	/* <S0D2> */
	{
		Data0 = (TrgVar[0] ^ TrgVar[1]) & TrcTrg[TrgNo].TrgBitSetMsk;
	/*----------------------------------------------------------------------------------------------*/
		if( Data0 == 0 )									/* No bit change						*/
		{
			return( FALSE );
		}
	/*----------------------------------------------------------------------------------------------*/
		if( TrcTrg[TrgNo].TrgEdge & 0x01 )					/* Rising edge trigger					*/
		{
			if( TrgVar[0] & TrcTrg[TrgNo].TrgBitSetMsk )
			{
				return( TRUE );
			}
		}
	/*----------------------------------------------------------------------------------------------*/
		if( TrcTrg[TrgNo].TrgEdge & 0x02 )					/* Falling edge trigger					*/
		{
			if( TrgVar[1] & TrcTrg[TrgNo].TrgBitSetMsk )
			{
				return( TRUE );
			}
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*		Numeric level trigger check																*/
	/*----------------------------------------------------------------------------------------------*/
	//else if(x == 0)	/* <S0D2> */
	else if( (( TrgData > 0 ) && (TrgData <= 8)) || ( (( TrgData == 0x000f ) && (TrgTarget == 0x0000)) ) ) 	/* <S0D2> */
	{
		Data0 = (LONG)(TrgVar[0] << TrcTrg[TrgNo].TrgVsft) >> TrcTrg[TrgNo].TrgVsft;
		Data1 = (LONG)(TrgVar[1] << TrcTrg[TrgNo].TrgVsft) >> TrcTrg[TrgNo].TrgVsft;
	/*----------------------------------------------------------------------------------------------*/
		if( TrcTrg[TrgNo].TrgEdge == 0x04 )					/* Level trigger(or more)					*/
		{
			if( Data0 >= TrcTrg[TrgNo].TrgLevel )
			{
				return( TRUE );
			}
		}
	/*----------------------------------------------------------------------------------------------*/
		if( TrcTrg[TrgNo].TrgEdge == 0x05 )					/* Level trigger()					*/
		{
			if( Data0 <= TrcTrg[TrgNo].TrgLevel )
			{
				return( TRUE );
			}
		}
	/*----------------------------------------------------------------------------------------------*/
	/* Since change edges are also processed,bit0,1andANDSet as processing(Bring level trigger judgment to front)	*/
	/*----------------------------------------------------------------------------------------------*/
		if( TrcTrg[TrgNo].TrgEdge & 0x01 )					/* Rising edge trigger					*/
		{
			if( (Data1 < TrcTrg[TrgNo].TrgLevel) && (Data0 >= TrcTrg[TrgNo].TrgLevel) )
			{
				return( TRUE );
			}
		}
	/*----------------------------------------------------------------------------------------------*/
		if( TrcTrg[TrgNo].TrgEdge & 0x02 )					/* Falling edge trigger					*/
		{
			if( (Data1 > TrcTrg[TrgNo].TrgLevel) && (Data0 <= TrcTrg[TrgNo].TrgLevel) )
			{
				return( TRUE );
			}
		}
	}

#if 0	/* <S0D2> Delete Start */
#if CSW_NET_TRG_USE == TRUE		/* <S012> */
	/*----------------------------------------------------------------------------------------------*/
	/*		Network trigger check(No check item)											*/
	/*----------------------------------------------------------------------------------------------*/
	else if( x == NET_TRG_SEL )
	{
		return( TRUE );
	}
#endif /* CSW_NET_TRG_USE */
#endif	/* <S0D2> Delete End */

	/*----------------------------------------------------------------------------------------------*/
		return( FALSE );
}

/****************************************************************************************************/
/*																									*/
/*		For trace sampling interval acquisitionAPI															*/
/*																									*/
/****************************************************************************************************/
ULONG	DtrcGetSampIntv( TRCHNDL *TrcHndl )
{
	return	(TrcHndl->TrcExeP.DataSampIntv);
}


/****************************************************************************************************/
/*																									*/
/*		For trace sampling interval acquisitionAPI															*/
/*																									*/
/****************************************************************************************************/
ULONG	DtrcGetTrcDataNum( TRCHNDL *TrcHndl )
{
	return	(TrcHndl->TrcExeP.TrcDnum);
}


/****************************************************************************************************/
/*																									*/
/*		For trace trigger position zero settingAPI																*/
/*																									*/
/****************************************************************************************************/
void	DtrcSetTrgPointToZero( TRCHNDL *TrcHndl )
{
	TrcHndl->TrcReg.TrgPoint = 0;
}


#if 0
/****************************************************************************************************/
/*																									*/
/*		Trace sampling period minimum value calculation								<V508>						*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*					Egear.b * Hprm.Kmotspd  * 1000000												*/
/*		SampLlim = -----------------------------------												*/
/*					Egear.a * KPI_SBCYCLEUS *  2^23													*/
/*																									*/
/****************************************************************************************************/
LONG	dtrcCalcSetSampLlim( void* Axis, LONG Axno )
{
	LONG	kx,sx,plsno;
	LONG	Egear_a,Egear_b;
	BOOL	EgearDisable;
	AXIS_HANDLE *AxHdl;

	AxHdl =  ((AXIS_HANDLE*)Axis + Axno);
	EgearDisable = (((AXIS_HANDLE*)Axis + Axno)->BaseCtrlData->TblDrive.OpeReg.TblFuncSel & TBLFUNCBIT_EGEAR);

	/* Currently not compatible with Fulc	*/	
	//if( Kprm.f.FencUse )
	//{
	//	plsno = Hprm.KmotspdFC;
	//}
	//else
	//{
		plsno = AxHdl->prm_hndl_ptr->hprm_ptr->Kmotspd;
	//}

	if( EgearDisable == FALSE )
	{
		Egear_a = AxHdl->BaseCtrlData->PosManager.conf.Egear.a;
		Egear_b = AxHdl->BaseCtrlData->PosManager.conf.Egear.b;
		kx = MlibScalKskxkx( plsno, Egear_b, Egear_a, &sx,  0 );
	}
	else
	{ /* Electronic gear disabled */
		kx = MlibScalKskxkx( plsno, 1, 1, &sx,  0 );
	}
	kx = MlibPcalKxgain( kx, 1000000, 0x7FFFFF, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, KPI_SBCYCLEUS, &sx, 0 );
	kx = MlibPcalKxaddx( kx, sx, 4194304, 23, &sx );		// kx = kx + 0.5
	kx = MlibPcalKxgain( kx, 1, 1, &sx, -24 );
	return( kx );
}
#endif



void	DtrcSetNetworkTrigger( TRCHNDL *TrcHndl, BOOL Trigger )		/* <S012> */
{
	TrcHndl->TrcExeV.NetTrigger = Trigger;
	return;
}

/* <S0C9>	Start */
/****************************************************************************************************/
/*																									*/
/*		Trace target data selection,I/OInitial value setting for trace target data selection								*/
/*																									*/
/****************************************************************************************************/
void	DtrcSelSetInitValue( TRCHNDL *TrcHndl )
{
	LONG	i;
	TRCREG	*TrcReg;
	TRCINDIVPRM	*TrcIncivPrm;

	TrcReg = &TrcHndl->TrcReg;
	TrcIncivPrm = &TrcHndl->TrcIncivPrm;


	/* Data,I/OTrace1~3Target data selection Initial value setting */
	TrcReg->DataIOTrcSel[0] = 0x0101;
	TrcReg->DataIOTrcSel[1] = 0x0204;
	TrcReg->DataIOTrcSel[2] = 0x8105;

	/* Trace1~3Target data selection Initial value setting */
	TrcReg->TrcDataSel[0] = 0x0001;
	TrcReg->TrcDataSel[1] = 0x0002;
	TrcReg->TrcDataSel[2] = 0x0003;

	/* I/OTrace1~3Target data selection Initial value setting */
	TrcReg->IOTrcDataSel[0] = 0x0001;
	TrcReg->IOTrcDataSel[1] = 0x0002;
	TrcReg->IOTrcDataSel[2] = 0x0081;


}
/* <S0C9>	End */

/***************************************** end of file **********************************************/

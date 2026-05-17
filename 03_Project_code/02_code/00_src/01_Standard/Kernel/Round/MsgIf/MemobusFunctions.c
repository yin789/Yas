/****************************************************************************************************/
/*																									*/
/*																									*/
/*		MemobusFunctions.c : MEMOBUSMessage communication management												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : MEMOBUSPerform message communication IF processing													*/
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
/*	Note	:	First edition	2010.09.06	T.Kira	For INGRAM												*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "KnlApi.h"
#include "MemobusIF.h"
#include "MemobusFunctions.h"
//#include "WaitFunc.h"
#include "PrmAccessIf.h"
//#include "MLib.h"

/* <S205> Delete Start */
/* <S150>:Due to the "commonization of message communication processing", "SigmaMemobus.c" Comment out or delete "Move to */
#if 0
/* <S061>	Start */
#ifdef _INFO_ID_READ_CHECK_
/****************************************************************************************************/
/*																									*/
/*		VARIABLE																					*/
/*																									*/
/****************************************************************************************************/

/* ReadingIDregister addressTable */
const LONG IdRegAddTbl[MBUS_IDRD_SLOT_NUM][MBUS_IDRD_ID_NUM]=
{
						/* 	ID0(Model)	ID1(Specification)	ID2(Specification№)	ID3(Serial№)	ID4(Soft Ver)	ID5(Register Ver)	ID6(BTO №)	 */
/* 00:Servopack */	{	0x80019000,	0x80018000,	0,			0x80019010,		0x8001800A,		0x80018200,			0x800191E0	},
/* 01:CMD */			{	0x800190C8,	0,			0,			0x800190E0,		0x80018082,		0,					0			},
/* 02:Safty */			{	0x800190FA,	0,			0,			0x80019112,		0x8001808A,		0,					0			},
/* 03:FB */				{	0x8001912C,	0,			0,			0x80019144,		0,				0,					0			},
/* 04:Motor */			{	0x80019022,	0,			0,			0x80019038,		0,				0,					0			},
/* 05:Encoder */		{	0x8001904A,	0,			0,			0x8001905A,		0x8001801C,		0,					0			}
};

/* ReadingIDTable[byte] */
const CHAR IdDataSizeTbl[MBUS_IDRD_SLOT_NUM][MBUS_IDRD_ID_NUM]=
{
						/* 	ID0(Model)	ID1(Specification)	ID2(Specification№)	ID3(Serial№)	ID4(Soft Ver)	ID5(Register Ver)	ID6(BTO №)	 */
/* 00:Servopack */	{	2,			2,			4,			2,				2,				4,					2			},
/* 01:CMD */			{	2,			0,			0,			2,				2,				0,					0			},
/* 02:Safty */			{	2,			0,			0,			2,				2,				0,					0			},
/* 03:FB */				{	2,			0,			0,			2,				0,				0,					0			},
/* 04:Motor */			{	2,			0,			0,			2,				0,				0,					0			},
/* 05:Encoder */		{	2,			0,			0,			2,				2,				0,					0			}
};

/* ReadingIDTable */
const CHAR IdDataNumTbl[MBUS_IDRD_SLOT_NUM][MBUS_IDRD_ID_NUM]=
{
						/* 	ID0(Model)	ID1(Specification)	ID2(Specification№)	ID3(Serial№)	ID4(Soft Ver)	ID5(Register Ver)	ID6(BTO №)	 */
/* 00:Servopack */	{	8,			1,			1,			8,				2,				1,					32			},
/* 01:CMD */			{	8,			0,			0,			8,				2,				0,					0			},
/* 02:Safty */			{	12,			0,			0,			12,				2,				0,					0			},
/* 03:FB */				{	12,			0,			0,			12,				0,				0,					0			},
/* 04:Motor */			{	11,			0,			0,			8,				0,				0,					0			},
/* 05:Encoder */		{	8,			0,			0,			8,				1,				0,					0			}
};

/*  Number(0~9)ASCII code table of */
const	UCHAR	NumAsciiTbl[ ] =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

/*  Character ASCII code table */
const	UCHAR	StrAsciiTbl[ ] =
		{ 'F', 'E', 'Y'};
#endif
/* <S061>	End */
#endif
/* <S205> Delete End */


/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S150> */
static void	  mbusFuncMsgManager( MEMOBUS_IF *MbusIfPtr );
static UCHAR *mbusFuncGetRcvAdr(MEMOBUS_IF *MbusIfPtr);
static UCHAR *mbusFuncGetTrnsAdr(MEMOBUS_IF *MbusIfPtr);
#ifdef __MYAD_CHECK__
static ULONG  mbusFuncChkMyMsg( MEMOBUS_IF *MbusIfPtr, UCHAR *CmdBuf );
#endif

static void	mbusLoopbackTest( MEMOBUS_IF *MbusIfPtr );

static void	mbusFuncReadMemory42H( MEMOBUS_IF *MbusIfPtr, LONG SubFuncCode );
static void	mbusFuncWriteMemory42H( MEMOBUS_IF *MbusIfPtr, LONG SubFuncCode );
static void	mbusFuncReadMaxSize42H( MEMOBUS_IF *MbusIfPtr, LONG SubFuncCode );
static void mbusFuncErrorManager42H( MEMOBUS_IF *MbusIfPtr, LONG ErrCode, ULONG ErrAddress );

//
static LONG mbusFuncShortSwap( void *Buf );
static LONG mbusFuncLongSwap( void *Buf );

//
static void	mbusFuncSetArrayLongData( UCHAR *Buf, ULONG data, UINT AccessSize );
static void	mbusFuncSetArrayShortData( UCHAR *Buf, USHORT data, UINT AccessSize );

//
static LONG mbusFuncSetValueByteData( UCHAR *Buf, LONG BlockNum, LONG dIndex );
static LONG mbusFuncSetValueShortData( UCHAR *Buf, LONG BlockNum, LONG dIndex );
static LONG mbusFuncSetValueLongData( UCHAR *Buf, LONG BlockNum, LONG dIndex );

static REG_MANAGER_HANDLE *mbusFuncConvAddress(MEMOBUS_ADDR_MAP *AxisMap, ULONG *MemAddress );
static UCHAR mbusFuncCheckBlockAccess(UCHAR *CmdBuf, ULONG CurrentAddress, 
													UINT AccessSize, LONG BlockNum, LONG dIndex);

#ifdef _INFO_ID_READ_CHECK_
static void mbusFuncReadProductInfo42H( MEMOBUS_IF *MbusIfPtr, LONG SubFuncCode );		/* <S061> */
static void mbusFuncSetSpecialSpecIdData( MEMOBUS_IF *MbusIfPtr, LONG RspBuffOffset);	/* <S061> */
static LONG mbusFuncSetIdData( MEMOBUS_IF *MbusIfPtr, LONG ReadIdRegAdd, UCHAR ReadDataSize, UCHAR ReadDataNum, LONG RspBuffOffset);		/* <S061> */
#endif
#endif	/*<S150>*/

/****************************************************************************************************/
/*																									*/
/*		Memobus IF Initialize																		*/
/*																									*/
/****************************************************************************************************/
#if 0	/* <S150> */
void	MbusFuncInitialize(MEMOBUS_IF *MbusIfPtr, MEMOBUS_ADDR_MAP *AxisCfg, LONG node_address )
{	
	INT i;

	for(i = 0; i < MAX_AXIS_NUMBER; i++)
	{
		MbusIfPtr->AxisMap[i] = AxisCfg[i];
	}

	/* Node address	*/
	MbusIfPtr->NodeAddress = node_address;
}
#endif	/* <S150> */

#if 0	/* <S150> */
/****************************************************************************************************/
/*																									*/
/*		Message analysis & execution processing																	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Return value: Send buffer length																		*/
/*				(0, the transmission is aborted and there is no response)												*/
/*																									*/
/****************************************************************************************************/
USHORT	MbusFuncMsgProcedure( MEMOBUS_IF *MbusIfPtr )
{
	LONG	Offset;
	LONG	RouteNum;
	LONG	Length;
	UCHAR  *RxBuf;
	UCHAR  *TxBuf;

	MbusIfPtr->RelayHdSize = 0;	/* Relay command header size initialization	*/
	MbusIfPtr->TrnbLength  = 0;	/* Initialize transmit buffer size			*/

	/* Receive, send data start address */
	RxBuf = &MbusIfPtr->RxBuf[MBUS_HDROFS];
	TxBuf = &MbusIfPtr->TxBuf[MBUS_HDROFS];

	/* Receive, get send buffer pointer	*/
	MbusIfPtr->CmdBuf = mbusFuncGetRcvAdr(  MbusIfPtr );
	MbusIfPtr->ResBuf = mbusFuncGetTrnsAdr( MbusIfPtr );

	/*----------------------------------------------------------------------------------------------*/
	/* MEMOBUSCommand analysis																		*/
	/*----------------------------------------------------------------------------------------------*/
	switch( RxBuf[1] )
	{	
		case MBUS_LBAK_08H:
			/*--------------------------------------------------------------------------------------*/
			/*		Loopback command																	*/
			/*--------------------------------------------------------------------------------------*/
			mbusLoopbackTest( MbusIfPtr );
			break;

		case MBUS_FC_42H:
			/*--------------------------------------------------------------------------------------*/
			/*		MEMOBUSFunction code														*/
			/*--------------------------------------------------------------------------------------*/
			/* Receive data length(Byte unit)Check */
			if(( MbusIfPtr->RcvbLength < MBUS_MINSIZE ) || ( MbusIfPtr->RcvbLength > MBUS_MAXSIZE ))
			{
				return(0);
			}
#ifdef __MYAD_CHECK__
			/* Own station address check	*/
			else if(FALSE == mbusFuncChkMyMsg( MbusIfPtr, RxBuf ))
			{
				return(0);	
			}
#endif	
#ifdef __EXAD_CHECK__
			/* Extended address check */
			else if(MbusIfPtr->CmdBuf[2] != 0xFF)
			{
				mbusFuncErrorManager42H( MbusIfPtr, SELAXIS_ERROR, 0);
			}
#endif	
			else
			{
				/* Message processing execution */
				mbusFuncMsgManager( MbusIfPtr );
			}
			break;	
		
#if 1	//2013.09.04
		case MBUS_SYSDL_50H:
			/*--------------------------------------------------------------------------------------*/
			/*		MEMOBUSFirm download code														*/
			/*--------------------------------------------------------------------------------------*/
			/* Receive data length(Byte unit)Check */
			if(( MbusIfPtr->RcvbLength < MBUS_MINSIZE ) || ( MbusIfPtr->RcvbLength > MBUS_MAXSIZE ))
			{
				return(0);
			}
			else
			{
				/* Execute firmware download processing */
				MbusFuncSystemDownLoad( MbusIfPtr );
			}
			break;
#endif

		case MBUS_RELAY_51H:		
			/*--------------------------------------------------------------------------------------*/
			/*		Relay message function													*/
			/*--------------------------------------------------------------------------------------*/
			/* Number of relay stages	*/
			RouteNum = (RxBuf[5] & 0x0F);				

			/* Relay command length */
			MbusIfPtr->RelayHdSize = (USHORT)(RouteNum * MBUS_RLY_PASS_DATA_SIZE + MBUS_RLY_FIX_SIZE);

			/* Get receive/send buffer pointer	*/
			MbusIfPtr->CmdBuf = mbusFuncGetRcvAdr( MbusIfPtr );
			MbusIfPtr->ResBuf = mbusFuncGetTrnsAdr( MbusIfPtr );
			
			/* Calculate header offset length */
			Offset = MbusIfPtr->RelayHdSize + MBUS_HDROFS;					
			
			/* Data section copy */
			MlibCopyByteMemory( TxBuf, RxBuf, MbusIfPtr->RelayHdSize);

			TxBuf[4] |= 0x80;					/* Response flag						*/
			TxBuf[5]  = (UCHAR)RouteNum;		/* Clear the number of passing stages&Number of relay stages set	*/

			/* Number of relay stages exceeded or Error if the number of relay stages and the number of passing stages are different */
			if( (RouteNum > MBUS_RLY_MAX_ROUTENUM) || (RouteNum != (RxBuf[5] >> 4)) )
			{/* Number of relay stages error */
#if 0
				MbusIfPtr->TxBuf[Offset - 6] |= 0x80;
				MbusIfPtr->TxBuf[Offset + 0] = 0x00;
				MbusIfPtr->TxBuf[Offset + 1] = MsgPtr->RxBuf[MBUS_HDROFS + 5];
				MbusIfPtr->TxBuf[Offset + 2] = 0x00;
				MbusIfPtr->TxBuf[Offset + 3] = 0x03;
				//MbusIfPtr->TrnwLength = Offset + 2;		/* Error reply size=2Word				*/
				MbusIfPtr->TrnbLength = Offset + 4;			/* Error reply size=4byte				*/
				//MbusIfPtr->MsgAnaSts = M3MSG_APLLAYER_ERR;
#endif
				/* Received message error */
				return(0);		
			}
			else
			{/* Number of relay stages normal */

				/* Receive data size(Byte unit) */
				MbusIfPtr->RcvbLength =(USHORT)(((ULONG)MbusIfPtr->RxBuf[Offset-4] << 24) 
											 | ( (ULONG)MbusIfPtr->RxBuf[Offset-3] << 16) 
											 | ( (ULONG)MbusIfPtr->RxBuf[Offset-2] <<  8) 
											 |   (ULONG)MbusIfPtr->RxBuf[Offset-1] <<  0);

				/* Receive data length(Byte unit)Check */
				if(( MbusIfPtr->RcvbLength < MBUS_MINSIZE ) || ( MbusIfPtr->RcvbLength > MBUS_MAXSIZE ))
				{
					/* Received message error */
					return(0);	
				}
#ifdef __MYAD_CHECK__
				/* Own station address check	*/
				else if( (FALSE == mbusFuncChkMyMsg( MbusIfPtr, RxBuf )) 
					  || (FALSE == mbusFuncChkMyMsg( MbusIfPtr, MbusIfPtr->CmdBuf )))
				{
					return(0);	
				}
#endif
				/* Function code check */
				else if( MbusIfPtr->CmdBuf[1] != MBUS_FC_42H )
				{
					/* Function code error */
					mbusFuncErrorManager42H( MbusIfPtr, FUNCCODE_ERROR, 0);	
				}
#ifdef __EXAD_CHECK__
				/* Extended address check */
				else if(MbusIfPtr->CmdBuf[2] != 0xFF)
				{
					mbusFuncErrorManager42H( MbusIfPtr, SELAXIS_ERROR, 0);
				}
#endif
				else
				{
					/* Message processing execution */
					mbusFuncMsgManager( MbusIfPtr );			
				}

				/* Add relay command size to send data size */
				MbusIfPtr->TrnbLength = MbusIfPtr->TrnbLength + MBUS_HDROFS + MbusIfPtr->RelayHdSize;

				/* Calculate relay command size */
				Length = MbusIfPtr->TrnbLength - Offset;

				/* Send data size(Byte unit) */
				mbusFuncSetArrayLongData( &MbusIfPtr->TxBuf[ Offset - 4 ], (ULONG)Length, sizeof(LONG) );
			}
			break;

		default:
			/*--------------------------------------------------------------------------------------*/
			/*		Function code not supported												*/
			/*--------------------------------------------------------------------------------------*/
			/* Own station address check	*/
#ifdef __MYAD_CHECK__
			if(FALSE == mbusFuncChkMyMsg( MbusIfPtr, RxBuf ))
			{
				/* Not addressed to your own station→Abort		*/
				return(MbusIfPtr->TrnbLength);	
			}
			else
			{
				/* To own station→Function error	*/
				mbusFuncErrorManager42H( MbusIfPtr, FUNCCODE_ERROR, 0);
			}
#else
			/* To own station→Function error	*/
			mbusFuncErrorManager42H( MbusIfPtr, FUNCCODE_ERROR, 0);
#endif
			break;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	When sending an odd number of bytes, the last one is garbage, so0Keep clear									*/
	/*----------------------------------------------------------------------------------------------*/
	if( MbusIfPtr->TrnbLength & 1 )
	{
		/* 0Clear	*/
		MbusIfPtr->TxBuf[ MbusIfPtr->TrnbLength ] = 0;
	}

	return( MbusIfPtr->TrnbLength );
}

#ifdef __MYAD_CHECK__
/****************************************************************************************************/
/*																									*/
/*		Check messages addressed to your own station																	*/
/*																									*/
/****************************************************************************************************/
ULONG	mbusFuncChkMyMsg( MEMOBUS_IF *MbusIfPtr,UCHAR *CmdBuf )		
{

	/*----------------------------------------------------------------------------------------------*/
	/*		Address of the message received this time=="0" or For own station addressTRUE					*/
	/*----------------------------------------------------------------------------------------------*/
	if(( CmdBuf[0] == 0x00 )
	 ||( CmdBuf[0] == MbusIfPtr->NodeAddress ))
	{
		/* To own station */
		return( TRUE );	
	}
	else
	{
		/* Not addressed to your own station */
		return( FALSE );			
	}

}
#endif

/****************************************************************************************************/
/*																									*/
/*		MEMOBUS MESSAGE MANAGER																		*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncMsgManager( MEMOBUS_IF *MbusIfPtr )
{
	LONG	SubFuncCode;
	UCHAR  *CmdBuf;	
	
	CmdBuf = MbusIfPtr->CmdBuf;		/* Received message start address */
	SubFuncCode = (LONG)CmdBuf[4];	/* Subfunction code   */
	
	//KpiRstLongTimer( &(MbusIfPtr->ComTimeLong) );		/* For command response timeout	*/
	#if 0 /* Write prohibition currently not supported */
	RpiSetAccsLvl( &Access, MsgPtr->AccsLvl );
	#endif

	switch( SubFuncCode )/* Subfunction code */
	{
		case MBUS_RD_CMEM_42H:		/* 01H Reading from memory (continuous) */
			mbusFuncReadMemory42H( MbusIfPtr, SubFuncCode );
			break;
		case MBUS_WR_CMEM_42H:		/* 02H Memory writing (continuous)	*/
			mbusFuncWriteMemory42H( MbusIfPtr, SubFuncCode );
			break;
		case MBUS_RD_NCMEM_42H:		/* 03H Reading from memory (non-continuous)*/
			mbusFuncReadMemory42H( MbusIfPtr, SubFuncCode );
			break;
		case MBUS_WR_NCMEM_42H:		/* 04H Memory writing (non-continuous)*/
			mbusFuncWriteMemory42H( MbusIfPtr, SubFuncCode );
			break;
		case MBUS_RD_MAXSIZE_42H:	/* 11H Maximum size readout		*/
			mbusFuncReadMaxSize42H( MbusIfPtr, SubFuncCode );
			break;

/* <S061>	Start */
#ifdef _INFO_ID_READ_CHECK_
		case MBUS_RD_INFO_42H:	/* ffH(Undefined) Product information reading	*/
			mbusFuncReadProductInfo42H( MbusIfPtr, SubFuncCode );
			break;
#endif
/* <S061>	End */

		default:					/* Command mismatch				*/
			mbusFuncErrorManager42H( MbusIfPtr, FUNCCODE_ERROR, 0 );
			break;
	}

	/* Transmission data length check */
	if(( MbusIfPtr->TrnbLength > (USHORT)MBUS_MAXSIZE ) || ( MbusIfPtr->TrnbLength < MBUS_MINSIZE ) )
	{
		/* Transmission data length error	*/
		mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, 0 );
	}
}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Abnormal response reply processing																	*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncErrorManager42H( MEMOBUS_IF *MbusIfPtr, LONG ErrorCode, ULONG ErrorAddress )
{
	UCHAR  *CmdBuf;
	UCHAR  *ResBuf;

	/* Receive, send data start address */
	CmdBuf = MbusIfPtr->CmdBuf;
	ResBuf = MbusIfPtr->ResBuf;

	/* Create abnormal response data */
	ResBuf[0]  =  CmdBuf[0];						/* 0:Slave Addrress		*/
	ResBuf[1]  = (CmdBuf[1] | 0x80);				/* 1:Function Code		*/
	ResBuf[2]  =  CmdBuf[2];						/* 2:Extended Address	*/
	ResBuf[3]  =  CmdBuf[3];						/* 3:Reserve1			*/
	ResBuf[4]  =  CmdBuf[4];						/* 4:Sub Function Code	*/
	ResBuf[5]  =  (UCHAR)ErrorCode;					/* 5:Error Code			*/
	ResBuf[6]  =  0x00;								/* 6:Reserve2			*/
	ResBuf[7]  =  0x00;								/* 7:Reserve3			*/
	mbusFuncSetArrayLongData( &ResBuf[8], ErrorAddress, sizeof(LONG) );	/* 8-11:ErrorAddress	*/

	/* Error reply size=12byte	*/
	MbusIfPtr->TrnbLength = MBUS_HEAD_BASE_SIZE;

/* <S056>	Start */
	/* register read:MEMOBUSError information(Error occurrence address) */
	MbusIfPtr->ErrRno = ErrorAddress;
/* <S056>	End */
}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Loopback test																	*/
/*																									*/
/****************************************************************************************************/
static void	mbusLoopbackTest( MEMOBUS_IF *MbusIfPtr )
{
ULONG	i;
USHORT	*CmdBuf = (USHORT *)(MbusIfPtr->CmdBuf);
USHORT	*ResBuf = (USHORT *)(MbusIfPtr->ResBuf);

/*--------------------------------------------------------------------------------------------------*/
/*		Response data creation (CRC-16Calculation unnecessary)															*/
/*--------------------------------------------------------------------------------------------------*/
		for ( i = 0; i < /*MbusStdCmd[ FuncCodeIdx ].PacketSize*/ 8 / 2; i++ )
		{
			ResBuf[i] = CmdBuf[i];
		}
//		MbusIfPtr->TrnbLength = sizeof(MbusIfPtr->RxBuf[ MSG_MAXBUFSZB ]);		/* Set size of Response data	*/
		MbusIfPtr->TrnbLength = 8;		/* Set size of Response data	*/

		return;
}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Address continuity check (non-continuous read/write common)								*/
/*																									*/
/****************************************************************************************************/
static UCHAR mbusFuncCheckBlockAccess(UCHAR *CmdBuf, ULONG CurrentAddress, 
													UINT AccessSize, LONG BlockNum, LONG dIndex)
{
	LONG  j;
	LONG  cnt;
	ULONG NextAddress;

	cnt=0;
	for( j=0; j<(BlockNum-1); j++ )
	{
		/* Address update	*/
		NextAddress = mbusFuncLongSwap( &CmdBuf[ dIndex*(j+1) ]);
		
		if( (NextAddress - CurrentAddress) == (AccessSize*(j+1)) )
		{/* If address specification is consecutive */
			cnt++;	
		}
	}
	if( cnt != (BlockNum-1) )
	{	/* Error if address specification is not consecutive */
		return MEMADRES_ERROR;
	}
	return 0;
}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Memory read (continuous/non-continuous common)													*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncReadMemory42H( MEMOBUS_IF *MbusIfPtr, LONG SubFuncCode )
{
#ifdef __RCVLEN_CHECK__
	LONG	CmdLength;
#endif
//	UCHAR	Mode; 
	UCHAR	DataType;
	UCHAR	dIndex;
	USHORT	DataNum;
	UINT	AccessSize,RegSize;
	REG_ACC_T AccessPrm;
	LONG	i;
	LONG	ret;
	LONG	Value;
	LONG	Offset;
	LONG	RspLength;
	LONG	BlockNum;
	ULONG	BeginAddress;
	ULONG	CurrentAddress;
	REG_MANAGER_HANDLE	*hRegMngr;

	/* Receive, send data start address */
	UCHAR  *CmdBuf;	
	UCHAR  *ResBuf;

	CmdBuf = MbusIfPtr->CmdBuf;
	ResBuf = MbusIfPtr->ResBuf;

	Value  = 0;
	//Mode      = (CmdBuf[5] >> 4);					/* Mode		*/
	DataType  = (CmdBuf[5] & 0x0F);					/* DataType */
	DataNum   = (CmdBuf[6] << 8) + CmdBuf[7];		/* DataNum	*/
	

	/* Data type check */
	if( (DataType != 1) && (DataType != 2) && (DataType != 3))	
	{
		/* Mode/Data type error*/
		mbusFuncErrorManager42H( MbusIfPtr, MODETYPE_ERROR, 0 );
		return;	
	}
	AccessSize   = (UCHAR)(0x0001 << (DataType-1));					/* Access size		  */
	BeginAddress = mbusFuncLongSwap( &CmdBuf[MBUS_HEAD_FIX_SIZE]);	/* Reading data address */


	/* Check receive data length, send data length, number of data pieces */
	if (CmdBuf[4] == MBUS_RD_CMEM_42H)
	{/* Continuous memory read */ 
#ifdef __RCVLEN_CHECK__
		CmdLength = MBUS_HEAD_BASE_SIZE;	/* Receive data length(Message total length) */
#endif
		RspLength = AccessSize * DataNum;	/* Transmission data length(Data section)		*/	
	}
	else
	{/* Discontinuous memory read */ 
		//CmdLength = MBUS_HEAD_FIX_SIZE + 4 * DataNum;
		RspLength = 4 * DataNum;		
	}
#ifdef __RCVLEN_CHECK__
	if(  (CmdLength != MbusIfPtr->RcvbLength )
		|| (RspLength > (MBUS_MAXSIZE - MBUS_HEAD_FIX_SIZE)) || (DataNum < MBUS_MIN_REGNUM)) 
#else
	if( (RspLength > (MBUS_MAXSIZE - MBUS_HEAD_FIX_SIZE)) || (DataNum < MBUS_MIN_REGNUM)) 
#endif
	{	/* Invalid quantity error */
		mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, BeginAddress );	
		return;
	}

	for ( i = 0; i < DataNum; i++ )
	{
		if (CmdBuf[4] == MBUS_RD_CMEM_42H)
		{/* Continuous memory read */ 
			CurrentAddress = BeginAddress + (AccessSize * i);/* Reading data address	*/
		}
		else
		{/* Discontinuous memory read */ 
			CurrentAddress = mbusFuncLongSwap( &CmdBuf[MBUS_HEAD_FIX_SIZE + 4*i] );			
		}

		/* virtual memory→Real address conversion */
		AccessPrm.MemAddr = CurrentAddress;
		//hRegMngr = mbusFuncConvAddress(MbusIfPtr->AxisMap,&AccessPrm.MemAddr,&ret);		
		hRegMngr = mbusFuncConvAddress(MbusIfPtr->AxisMap,&AccessPrm.MemAddr);		
		if( hRegMngr == NULL )
		{	/* Memory address error */
			//mbusFuncErrorManager42H( MbusIfPtr, ret, CurrentAddress );//kira_1202
			mbusFuncErrorManager42H( MbusIfPtr, SELAXIS_ERROR, CurrentAddress );
			return;
		}

		/* Calculate the offset to the data position to be stored	*/
		Offset = MBUS_HEAD_FIX_SIZE + i*AccessSize;

		/* Get parameter attribute */
		if( NULL == RegMngrGetAttrByMemAddr(hRegMngr, &AccessPrm) )
		{	/* Memory address error	*/
			mbusFuncErrorManager42H( MbusIfPtr, MEMADRES_ERROR, CurrentAddress );
			return;
		}
		/* Read parameter size */
		RegSize = AccessPrm.cPrmDef->DataLength;

		/*------------------------------------------------------------------------------------------*/
		/*	Block reading																		*/
		/*------------------------------------------------------------------------------------------*/
		BlockNum = RegSize/AccessSize;		/* Block read count */

		if( (BlockNum > 0) && ( DataNum > (i + BlockNum - 1)) )
		{
			if(SubFuncCode == MBUS_RD_NCMEM_42H)
			{/* Discontinuous memory read */

				dIndex = 4;

				/* Address continuity check */
				ret = mbusFuncCheckBlockAccess( &CmdBuf[MBUS_HEAD_FIX_SIZE + dIndex*i], 
																CurrentAddress, AccessSize,BlockNum,dIndex);
				if( ret != 0 )
				{	
					/* Error if address specification is not consecutive */
					mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, CurrentAddress );
					return;
				}
			}
			i = i + (BlockNum-1);				/* Block read counti	*/
		}
		else
		{	
			/* Quantity defect */
			mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, CurrentAddress );
			return;
		}

		/* Data reading */
		AccessPrm.IfHandle = MbusIfPtr;
		ret = (LONG)PrmReadValue( &AccessPrm, &Value, RegSize);
		if( PRM_RSLT_SUCCESS != ret )
		{
			mbusFuncErrorManager42H( MbusIfPtr, ret, CurrentAddress );
			return;
		}

		/* Send data set */
		switch (RegSize)
		{
			case 1:		/* bytetype */
				ResBuf[Offset] = (UCHAR)(Value);
				break;
			case 2:		/* shorttype */
				mbusFuncSetArrayShortData( &ResBuf[Offset], (USHORT)Value, AccessSize );
				break;
			case 4:		/* longtype */
				mbusFuncSetArrayLongData( &ResBuf[Offset], (ULONG)Value, AccessSize );
				break;
		}
	}

	MlibCopyByteMemory( ResBuf, CmdBuf, MBUS_HEAD_FIX_SIZE);			/* Response data creation */
	MbusIfPtr->TrnbLength = MBUS_HEAD_FIX_SIZE + AccessSize*DataNum;	/* Determine transmit buffer size */
}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Memory write (continuous/non-continuous common)													*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncWriteMemory42H( MEMOBUS_IF *MbusIfPtr, LONG SubFuncCode )
{
#ifdef __RCVLEN_CHECK__
	LONG	CmdLength;
#endif
//	UCHAR	Mode;
	USHORT	DataNum;
	UCHAR	AccessSize;
	UCHAR	DataType;
	UCHAR	dIndex;
	REG_ACC_T AccessPrm;
	LONG	i;
	LONG	ret;
	LONG	Value;
	LONG	Offset;
	LONG	MaxSize;
	LONG	RspLength;
	LONG	BlockNum;
	ULONG	BeginAddress; 
	ULONG	CurrentAddress;
	UINT	RegSize;
	REG_MANAGER_HANDLE	*hRegMngr;

	UCHAR  *CmdBuf;	/* Receive buffer */
	UCHAR  *ResBuf;	/* Transmission buffer */

	/* Receive, send data start address stored */
	CmdBuf = MbusIfPtr->CmdBuf;
	ResBuf = MbusIfPtr->ResBuf;

//	Mode     = (CmdBuf[5] >> 4);					/* Mode	*/
	DataType = (CmdBuf[5] & 0x0F);					/* Data type */
	DataNum  = (CmdBuf[6] << 8) + CmdBuf[7];		/* Data count	*/

	/* Data type check */
	if( (DataType != 1) && (DataType != 2) && (DataType != 3))	
	{	/* Mode/Data type error*/
		mbusFuncErrorManager42H( MbusIfPtr, MODETYPE_ERROR, 0 );
		return;	
	}
	AccessSize   = (UCHAR)(0x0001 << (DataType-1));					/* Access size			*/
	BeginAddress = mbusFuncLongSwap( &CmdBuf[MBUS_HEAD_FIX_SIZE] );	/* Write data address	*/

	/* Check receive data length, send data length, number of data pieces */
	if (CmdBuf[4] == MBUS_WR_CMEM_42H)
	{/* Continuous memory write */
#ifdef __RCVLEN_CHECK__
		CmdLength = MBUS_HEAD_BASE_SIZE + AccessSize * DataNum;	/* Receive data length(Message total length) */
#endif
		RspLength = AccessSize * DataNum;						/* Transmission data length(Data section)		*/
		MaxSize   = MBUS_MAXSIZE - MBUS_HEAD_BASE_SIZE;			/* Maximum buffer size(Data section)	*/
	}
	else
	{/* Discontinuous memory write */ 
#ifdef __RCVLEN_CHECK__
		CmdLength = MBUS_HEAD_FIX_SIZE + 8 * DataNum;
#endif
		RspLength = 8 * DataNum;
		MaxSize   = MBUS_MAXSIZE - MBUS_HEAD_FIX_SIZE;
	}
#ifdef __RCVLEN_CHECK__
	if( (MbusIfPtr->RcvbLength != CmdLength ) || (RspLength > MaxSize) || (DataNum < MBUS_MIN_REGNUM) )
#else
	if( (RspLength > MaxSize) || (DataNum < MBUS_MIN_REGNUM) )
#endif
	{	/* Quantity defect */
		mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, BeginAddress );		
		return;
	}

	for ( i = 0; i < DataNum; i++ )
	{ 
		if (CmdBuf[4] == MBUS_WR_CMEM_42H)
		{/* Continuous memory write */
			CurrentAddress   = BeginAddress + (AccessSize * i);	/* Write data address	*/
			Offset = MBUS_HEAD_BASE_SIZE + i*AccessSize;		/* Write data offset */
			dIndex = AccessSize;
		}
		else
		{/* Discontinuous memory write */ 
			CurrentAddress = mbusFuncLongSwap( &CmdBuf[MBUS_HEAD_FIX_SIZE + 8*i] );
			Offset = MBUS_HEAD_BASE_SIZE + i*sizeof(LONG)*2;
			dIndex = sizeof(LONG)*2;
		}

		/* virtual memory → Real memory address conversion */
		AccessPrm.MemAddr = CurrentAddress;
		hRegMngr = mbusFuncConvAddress(MbusIfPtr->AxisMap,&AccessPrm.MemAddr);		
		if( hRegMngr == NULL )
		{	/* Memory address error */
			//mbusFuncErrorManager42H( MbusIfPtr, ret, CurrentAddress );
			mbusFuncErrorManager42H( MbusIfPtr, SELAXIS_ERROR, CurrentAddress );//kira_1202
			return;
		}

		/* Get parameter attribute */
		if( NULL == RegMngrGetAttrByMemAddr(hRegMngr, &AccessPrm) )
		{	/* Memory address error	*/
			mbusFuncErrorManager42H( MbusIfPtr, MEMADRES_ERROR, CurrentAddress );
			return;
		}

		/* Parameter size acquisition */
		RegSize = AccessPrm.cPrmDef->DataLength;

		/*------------------------------------------------------------------------------------------*/
		/*	Block write																		*/
		/*------------------------------------------------------------------------------------------*/
		BlockNum = RegSize/AccessSize;		/* Block write count */

		if( (BlockNum > 0) && ( DataNum > (i + BlockNum-1)) )
		{
			if(SubFuncCode == MBUS_WR_NCMEM_42H)
			{/* Discontinuous memory write */

				/* Address continuity check */
				ret = mbusFuncCheckBlockAccess( &CmdBuf[MBUS_HEAD_FIX_SIZE + dIndex*i], 
																CurrentAddress, AccessSize, BlockNum, dIndex);
				if( ret != 0 )
				{	/* Error if address specification is not consecutive */
					mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, CurrentAddress );
					return;
				}
			}
			i = i + (BlockNum-1);	/* Block write counti	*/
			Value = 0;				/* Write value initialization						*/

			switch(AccessSize)
			{
				case 1:
					Value = mbusFuncSetValueByteData( &CmdBuf[Offset], BlockNum, dIndex );
					break;
				case 2:
					Value = mbusFuncSetValueShortData( &CmdBuf[Offset], BlockNum, dIndex );
					break;
				case 4:
					Value = mbusFuncSetValueLongData( &CmdBuf[Offset], BlockNum, dIndex );
					break;
				default:/* It will not come here */
					break;
			}
		}
		else
		{	/* Quantity defect */
			mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, CurrentAddress );
			return;
		}

		/* Data write */
		AccessPrm.IfHandle = MbusIfPtr;
		ret = (LONG)PrmWriteValue( &AccessPrm, &Value, RegSize);
		if(0 > ret)
		{
			mbusFuncErrorManager42H( MbusIfPtr, ret, CurrentAddress );
			return;
		}
	}

	MlibCopyByteMemory( ResBuf, CmdBuf, MBUS_HEAD_FIX_SIZE);	/* Response data creation */
	MbusIfPtr->TrnbLength = MBUS_HEAD_FIX_SIZE;					/* Determine transmit buffer size */

}

/****************************************************************************************************/
/*																									*/
/*		Memobus: Maximum size read																	*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncReadMaxSize42H( MEMOBUS_IF *MbusIfPtr, LONG SubFuncCode )
{
	LONG	MsgSize;

	UCHAR  *CmdBuf;//Receive buffer start pointer
	UCHAR  *ResBuf;//Send buffer start pointer

	/* Receive, send data start address stored */
	CmdBuf = MbusIfPtr->CmdBuf;
	ResBuf = MbusIfPtr->ResBuf;

#ifdef __RCVLEN_CHECK__
	/*----------------------------------------------------------------------------------------------*/
	/*		Check receive data length																	*/
	/*----------------------------------------------------------------------------------------------*/
	/* Invalid receive data length */
	if( MbusIfPtr->RcvbLength != MBUS_HEAD_FIX_SIZE )
	{	/* Invalid quantity error	*/
		mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, BeginAddress );		
		return;
	}
#endif

	/* Maximum size readout size */
	MsgSize = MBUS_MAXSIZE - MBUS_HEAD_FIX_SIZE;

	/* Response data creation */
	ResBuf[0]  = CmdBuf[0];								/* 0:Slave Addrress		*/
	ResBuf[1]  = CmdBuf[1];								/* 1:Function Code		*/
	ResBuf[2]  = CmdBuf[2];								/* 2:Extended Address	*/
	ResBuf[3]  = CmdBuf[3];								/* 3:Reserve1			*/
	ResBuf[4]  = CmdBuf[4];								/* 4:Sub Function Code	*/
	ResBuf[5]  = CmdBuf[5];								/* 5:Mode,DataType 		*/
	ResBuf[6]  = 0;										/* 6:DataNumH			*/
	ResBuf[7]  = 0;										/* 7:DataNumL			*/
	mbusFuncSetArrayLongData( &ResBuf[8], (ULONG)MsgSize, 4);	/* 8-11 MaxMessageSize	*/

	/* Determine transmit buffer size */
	MbusIfPtr->TrnbLength = MBUS_HEAD_BASE_SIZE;

}
#endif	/* <S150> */

/****************************************************************************************************/
/*																									*/
/*	Message buffer swap function																	*/
/*																									*/
/****************************************************************************************************/
void	mbusFuncBufSwap( UCHAR *Buf )
{
#ifdef __BIG_ENDIAN__
/* BigEndian */

	LONG	i;
	LONG	Num;
	UCHAR	tmp[4];

	Num = MBUS_MAXSIZE / 4;

	for( i=0; i<Num; i++ )
	{
		tmp[0] = Buf[i*4    ]; 
		tmp[1] = Buf[i*4 + 1]; 
		tmp[2] = Buf[i*4 + 2]; 
		tmp[3] = Buf[i*4 + 3]; 

		Buf[i*4    ] = tmp[3];
		Buf[i*4 + 1] = tmp[2];
		Buf[i*4 + 2] = tmp[1];
		Buf[i*4 + 3] = tmp[0];
	}
#else
/* LittleEndian */
	/* Unconfirmed */
	/*Do nothing*/

#endif
}

#if 0	/* <S150> */
/****************************************************************************************************/
/*																									*/
/*		Message buffer swap function(ULONG)														*/
/*																									*/
/****************************************************************************************************/
static LONG	mbusFuncLongSwap( void *Buf )
{
	ULONG data;
#ifdef __BIG_ENDIAN__	
	/* BigEndian */
	data =  (ULONG)((*( UCHAR *)Buf	     << 24) 
		 	      + (*((UCHAR *)Buf + 1) << 16)
			      + (*((UCHAR *)Buf + 2) <<  8)
				  + (*((UCHAR *)Buf + 3)      ) );
#else					
	/* LittleEndian */
	/* Unconfirmed */
	//↓is the opposite
	//data = (ULONG)((*( UCHAR *)Buf	         ) 
	//	         + (*((UCHAR *)Buf + 1) <<  8)
	//		     + (*((UCHAR *)Buf + 2) << 16)
	//		  	 + (*((UCHAR *)Buf + 3) << 24) );
	data = MlibCnvBig32( *((ULONG *)Buf) );
#endif
	return data;
}

/****************************************************************************************************/
/*																									*/
/*		Message buffer swap function(USHORT)														*/
/*																									*/
/****************************************************************************************************/
static LONG	mbusFuncShortSwap( void *Buf )
{
	ULONG data;
#ifdef __BIG_ENDIAN__	
	/* BigEndian */
	data = (ULONG)((*( UCHAR *)Buf      << 8) 
		         + (*((UCHAR *)Buf + 1)     ) );
#else					
	/* LittleEndian */
	/* Unconfirmed */
/* <S001> */
//	data = (ULONG)((*( UCHAR *)Buf           ) 
//		 	     + (*((UCHAR *)Buf + 1) << 8 ));
	data = (ULONG)((*( UCHAR *)Buf      << 8) 
		         + (*((UCHAR *)Buf + 1)     ) );
/* <S001> */
#endif
	return data;
}
 
/****************************************************************************************************/
/*																									*/
/*		Message buffer storage function(USHORT)															*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncSetArrayShortData( UCHAR *Buf, USHORT data, UINT AccessSize)
{
#ifdef __BIG_ENDIAN__	
/* BigEndian */
	switch(AccessSize)
	{
		case 1:
			Buf[1] = (UCHAR)((data) >> 8);
			Buf[0] = (UCHAR)( data );
			break;
		case 2:
			Buf[0] = (UCHAR)((data) >> 8);
			Buf[1] = (UCHAR)( data );
			break;
		default:
			break;
	}
#else					
/* LittleEndian */
/* <S001> */
	/* ↓Reverse */
	//Buf[1] = (UCHAR)((data) >> 8);
	//Buf[0] = (UCHAR)( data );
//	Buf[0] = (UCHAR)((data) >> 8);
//	Buf[1] = (UCHAR)( data );

	switch(AccessSize)
	{
		case 1:
			Buf[1] = (UCHAR)((data) >> 8);
			Buf[0] = (UCHAR)( data );
			break;
		case 2:
			Buf[0] = (UCHAR)((data) >> 8);
			Buf[1] = (UCHAR)( data );
			break;
		default:
			break;
	}
/* <S001> */
#endif
}

/****************************************************************************************************/
/*																									*/
/*		Message buffer storage function(ULONG)															*/
/*																									*/
/****************************************************************************************************/
static void	mbusFuncSetArrayLongData( UCHAR *Buf, ULONG data, UINT AccessSize )
{
#ifdef __BIG_ENDIAN__
/* BigEndian */
	switch(AccessSize)
	{
		case 1:
			Buf[3] = (UCHAR)((data) >> 24);
			Buf[2] = (UCHAR)((data) >> 16);
			Buf[1] = (UCHAR)((data) >>  8);
			Buf[0] = (UCHAR)( data );
			break;
		case 2:
			Buf[2] = (UCHAR)((data) >> 24);
			Buf[3] = (UCHAR)((data) >> 16);
			Buf[0] = (UCHAR)((data) >>  8);
			Buf[1] = (UCHAR)( data );
			break;
		case 4:
			Buf[0] = (UCHAR)((data) >> 24);
			Buf[1] = (UCHAR)((data) >> 16);
			Buf[2] = (UCHAR)((data) >>  8);
			Buf[3] = (UCHAR)( data );
			break;
		default:
			break;
	}
#else
/* LittleEndian */
/* <S001> */
	/* Unconfirmed */
	//↓Reverse
	//Buf[3] = (UCHAR)((data) >> 24);
	//Buf[2] = (UCHAR)((data) >> 16);
	//Buf[1] = (UCHAR)((data) >>  8);
	//Buf[0] = (UCHAR)( data );
//	*(ULONG *)Buf = MlibCnvBig32( data );

	switch(AccessSize)
	{
		case 1:
			Buf[3] = (UCHAR)((data) >> 24);
			Buf[2] = (UCHAR)((data) >> 16);
			Buf[1] = (UCHAR)((data) >>  8);
			Buf[0] = (UCHAR)( data );
			break;
		case 2:
			Buf[2] = (UCHAR)((data) >> 24);
			Buf[3] = (UCHAR)((data) >> 16);
			Buf[0] = (UCHAR)((data) >>  8);
			Buf[1] = (UCHAR)( data );
			break;
		case 4:
			Buf[0] = (UCHAR)((data) >> 24);
			Buf[1] = (UCHAR)((data) >> 16);
			Buf[2] = (UCHAR)((data) >>  8);
			Buf[3] = (UCHAR)( data );
			break;
		default:
			break;
	}
/* <S001> */
#endif
}

/****************************************************************************************************/
/*																									*/
/*		Message data storage function(Byte)																*/
/*																									*/
/****************************************************************************************************/
static LONG	mbusFuncSetValueByteData( UCHAR *Buf, LONG BlockNum, LONG dIndex )
{
	LONG Value,j;
	LONG BitShift;
	Value = 0;

	for( j=0; j<BlockNum; j++ )
	{
		BitShift = 8 * sizeof(CHAR) * j;
		Value += (LONG)(Buf[ dIndex*j ] << BitShift );
	}
	return Value;
}

/****************************************************************************************************/
/*																									*/
/*		Message data storage function(SHORT)																*/
/*																									*/
/****************************************************************************************************/
static LONG	mbusFuncSetValueShortData( UCHAR *Buf, LONG BlockNum, LONG dIndex )
{
	LONG Value,j;
	LONG BitShift;
	Value = 0;

	for( j=0; j<BlockNum; j++ )
	{
		BitShift = 8 * sizeof(SHORT) * j;
		Value += (mbusFuncShortSwap( (void*)Buf ) << BitShift);
		Buf += dIndex;
	}
	return Value;
}

/****************************************************************************************************/
/*																									*/
/*		Message data storage function(LONG)																*/
/*																									*/
/****************************************************************************************************/
static LONG mbusFuncSetValueLongData( UCHAR *Buf, LONG BlockNum, LONG dIndex )
{
	LONG Value,j;
	LONG BitShift;
	Value = 0;

	for( j=0; j<BlockNum; j++ )
	{
		BitShift = 8 * sizeof(LONG) * j;
		Value += (mbusFuncLongSwap( (void*)Buf ) << BitShift);
		Buf += dIndex;
	}
	return Value;
}

/****************************************************************************************************/
/*																									*/
/*		MEMOBUSMessage reception buffer address provision function												*/
/*																									*/
/*			Return value: Start address of send buffer														*/
/*																									*/
/****************************************************************************************************/
static UCHAR	*mbusFuncGetRcvAdr( MEMOBUS_IF *MbusIfPtr )
{
	return( &MbusIfPtr->RxBuf[ MbusIfPtr->RelayHdSize + MBUS_HDROFS ] );		
}

/****************************************************************************************************/
/*																									*/
/*		MEMOBUSMessage transmission buffer address provision function												*/
/*																									*/
/*			Return value: Start address of receive buffer														*/
/*																									*/
/****************************************************************************************************/
 static UCHAR	*mbusFuncGetTrnsAdr( MEMOBUS_IF *MbusIfPtr )
{
	return( &MbusIfPtr->TxBuf[ MbusIfPtr->RelayHdSize + MBUS_HDROFS ] );		
}

/****************************************************************************************************/
/*																									*/
/*		virtual memory→Real memory conversion function																*/
/*		(Check address range and store axis number and parameter number)								*/
/*																									*/
/*			Return value:REG_MANAGER_HANDLE															*/
/*																									*/
/****************************************************************************************************/
static REG_MANAGER_HANDLE *mbusFuncConvAddress(MEMOBUS_ADDR_MAP *AxisMap, ULONG *MemAddress )
{
	ULONG	mem_address, i;
	ULONG	AxisNumber;

	mem_address = *MemAddress;
	AxisNumber = (mem_address & 0x00F00000) >> 20;

	if(AxisNumber >= MAX_AXIS_NUMBER)
	{
		return NULL;
	}
	
	for(i = 0; i < MAX_AXIS_NUMBER; i++)
	{
		if(AxisMap[i].AxisAddress == AxisNumber)
		{
			return AxisMap[i].hRegMngr;
		}
	}

	return NULL;
}

/* <S061>	Start */
#ifdef _INFO_ID_READ_CHECK_
/****************************************************************************************************/
/*																									*/
/*		Memobus: Read product information																	*/
/*																									*/
/****************************************************************************************************/
static void mbusFuncReadProductInfo42H( MEMOBUS_IF *MbusIfPtr, LONG SubFuncCode )
{
	UCHAR	BlockSize;
	UCHAR	ReadBlockSize;
	UCHAR	ReadSlotNo;
	UCHAR	ReadAxisNo;
	UCHAR	ReadIdNo;

	LONG	CmdBuffOffset;
	LONG	NextCmdBuffOffset;
	LONG	RspBuffOffset;
	LONG	RspBuffSize;

	LONG	ReadIdRegAdd;
	UCHAR	ReadDataSize;
	UCHAR	ReadDataNum;

	LONG	i;
	LONG	j;
	
	LONG	ret;
	LONG	CurrentDataLength;

	/* Receive, send data start address */
	UCHAR  *CmdBuf;	
	UCHAR  *ResBuf;

	CmdBuf = MbusIfPtr->CmdBuf;
	ResBuf = MbusIfPtr->ResBuf;

	/* Initializing variables */
	ret = 0;
	NextCmdBuffOffset = 0;							/* Next block number storage buffer offset */
	RspBuffOffset = MBUS_IDRD_RSP_HEAD_BASE_SIZE;	/* RspBuffData section offset */
	RspBuffSize = MBUS_IDRD_RSP_HEAD_BASE_SIZE;		/* RspBuffData size */
	BlockSize = CmdBuf[MBUS_IDRD_HEAD_BASE_SIZE];	/* Get the number of read blocks */

	/* Block number check  */
	if( BlockSize == 0) 
	{	/* Setting value out of range error  */
		mbusFuncErrorManager42H( MbusIfPtr, RANGOUTS_ERROR, 0 );	
		return;
	}

	for ( i = 0; i < BlockSize; i++ )
	{
		CmdBuffOffset = MBUS_IDRD_HEAD_BASE_SIZE + 1 + NextCmdBuffOffset;	/* Block size to read is storedCmdBuffOffset calculation */
		ReadBlockSize =  CmdBuf[CmdBuffOffset];								/* Getting block size to read */

		/* Block size check */
		if( (ReadBlockSize < 3) || (ReadBlockSize > MBUS_IDRD_ID_NUM + 2) )
		{
			mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, 0 );	
			return;
		}
		else
		{
			ReadSlotNo = CmdBuf[CmdBuffOffset + 1];			/* Obtains "slot number" */
			/* Slot number check  */
			if( ReadSlotNo > (MBUS_IDRD_SLOT_NUM - 1) )
			{
				/* Setting value out of range error */
				mbusFuncErrorManager42H( MbusIfPtr, RANGOUTS_ERROR, 0 );	
				return;
			}

			ReadAxisNo = CmdBuf[CmdBuffOffset + 2];			/* Getting "axis number" */
			ResBuf[RspBuffOffset] = ReadBlockSize;			/* Stores "block size" in response buffer */
			ResBuf[RspBuffOffset + 1] = ReadSlotNo;			/* Stores "slot number" in response buffer */
			ResBuf[RspBuffOffset + 2] = ReadAxisNo;			/* Storing "axis number" in response buffer */

			RspBuffSize = RspBuffSize + 3;					/* Update receive data size */
			RspBuffOffset = RspBuffOffset + 3;				/* Response buffer offset update */

			/* Response buffer size check */
			if( RspBuffSize > MBUS_MAXSIZE )
			{	/* Invalid quantity error */
				mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, 0 );	
				return;
			}

			/* Get read product information */
			for ( j = 0; j < (ReadBlockSize - 2 ) ; j++ )
			{
				ReadIdNo =  CmdBuf[CmdBuffOffset + (3 + j)];	/* ReadingID */;

				/* ID-No.Check */
				if( ReadIdNo > (MBUS_IDRD_ID_NUM - 1) )
				{
					/* Setting value out of range error */
					mbusFuncErrorManager42H( MbusIfPtr, RANGOUTS_ERROR, 0 );	
					return;
				}

				ReadIdRegAdd = IdRegAddTbl[ReadSlotNo][ReadIdNo];		/* ReadingIDregister address acquisition */
				ReadDataSize = IdDataSizeTbl[ReadSlotNo][ReadIdNo];		/* ReadingID */
				ReadDataNum = IdDataNumTbl[ReadSlotNo][ReadIdNo];		/* ReadingID */

				if( (ReadIdRegAdd == 0) && (ReadDataSize == 0) && (ReadDataNum == 0) )	/* ReadingIDNo data */
				{
					CurrentDataLength = 1;								/* Data length = 2[byte] */
					ResBuf[RspBuffOffset] = ReadIdNo;					/* "ID" This process is brought to this location in order to use "Storing */
					ResBuf[RspBuffOffset + 1] = CurrentDataLength;		/* "ReadIDin the response buffer */
					ResBuf[RspBuffOffset + 2] = 0x00;					/* "ReadID = 0x00 */
					RspBuffOffset = RspBuffOffset + 3;					/* Response buffer(Data section)A offset update */
					RspBuffSize = RspBuffSize + 3;						/* Update receive data size */


				}
				else	/* ReadingIDData exists */
				{
					CurrentDataLength = ReadDataSize * ReadDataNum;			/* Receive data length[byte] */
					ResBuf[RspBuffOffset] = ReadIdNo;						/* "ID" This process is brought to this location in order to use "Storing */
					ResBuf[RspBuffOffset + 1] = CurrentDataLength;			/* "ReadIDin the response buffer */
					RspBuffOffset = RspBuffOffset + 2;						/* Response buffer offset update */
					RspBuffSize = RspBuffSize + CurrentDataLength + 2;		/* Update receive data size */

					/* Response buffer size check */
					if( (RspBuffSize > MBUS_MAXSIZE) || (ReadDataNum < MBUS_MIN_REGNUM))
					{	/* Invalid quantity error */
						mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, ReadIdRegAdd );	
						return;
					}


					if( ReadIdNo == 0x02 )	/* ID2(Specification)For reading */
					{
						mbusFuncSetSpecialSpecIdData( MbusIfPtr, RspBuffOffset);	/* Specification acquisition */
						RspBuffOffset = RspBuffOffset + CurrentDataLength;			/* Response buffer(Data section)Offset update */
					}
					else					/* ID2(Specification)For cases other than read */
					{
						ret = mbusFuncSetIdData( MbusIfPtr, ReadIdRegAdd, ReadDataSize, ReadDataNum, RspBuffOffset);
						if(ret !=0 )
						{
							/* Error */
							mbusFuncErrorManager42H( MbusIfPtr, ret, ReadIdRegAdd );
							return;
						}
						else
						{
							RspBuffOffset = RspBuffOffset + CurrentDataLength;	/* Response buffer(Data section)Offset update */

						}
					}
				}
			}
			
			NextCmdBuffOffset = NextCmdBuffOffset + ReadBlockSize + 1;	/* Next block number storage buffer offset updated */
		}
	}

	MlibCopyByteMemory( ResBuf, CmdBuf, MBUS_IDRD_HEAD_BASE_SIZE+1);	/* Response data creation */
	MbusIfPtr->TrnbLength = RspBuffSize;								/* Determine transmit buffer size */
	
}

/****************************************************************************************************/
/*																									*/
/*		ProductIDStorage function																				*/
/*			ID0:Product model																			*/
/*			ID1:Model typeID																				*/
/*			ID3:SerialNo.																			*/
/*			ID4:Soft Ver.																			*/
/*			ID5:Register Ver.																		*/
/*			ID6:BTONumber																				*/
/*																									*/
/****************************************************************************************************/
static LONG mbusFuncSetIdData( MEMOBUS_IF *MbusIfPtr, LONG ReadIdRegAdd, UCHAR ReadDataSize, UCHAR ReadDataNum, LONG RspBuffOffset)
{
	UCHAR	DataType;
	UCHAR	dIndex;
	USHORT	DataNum;
	UINT	AccessSize,RegSize;
	REG_ACC_T AccessPrm;
	LONG	i;
	LONG	ret;
	LONG	Value;
	LONG	RspLength;
	LONG	BlockNum;
	ULONG	BeginAddress;
	ULONG	CurrentAddress;
	REG_MANAGER_HANDLE	*hRegMngr;

	/* Receive, send data start address */
	UCHAR  *CmdBuf;	
	UCHAR  *ResBuf;

	CmdBuf = MbusIfPtr->CmdBuf;
	ResBuf = MbusIfPtr->ResBuf;

		ret = 0;

	for ( i = 0; i < ReadDataNum; i++ )
	{
			CurrentAddress = ReadIdRegAdd + (2*i);

		/* virtual memory→Real address conversion */
		AccessPrm.MemAddr = CurrentAddress;
		hRegMngr = mbusFuncConvAddress(MbusIfPtr->AxisMap,&AccessPrm.MemAddr);		
		if( hRegMngr == NULL )
		{	/* Memory address error */
			mbusFuncErrorManager42H( MbusIfPtr, SELAXIS_ERROR, CurrentAddress );
			ret = SELAXIS_ERROR;
			return ret;
		}

		/* Get parameter attribute */
		if( NULL == RegMngrGetAttrByMemAddr(hRegMngr, &AccessPrm) )
		{	/* Memory address error	*/
			mbusFuncErrorManager42H( MbusIfPtr, MEMADRES_ERROR, CurrentAddress );
			ret = MEMADRES_ERROR;
			return ret;
		}
		/* Read parameter size */
		RegSize = AccessPrm.cPrmDef->DataLength;

		/*------------------------------------------------------------------------------------------*/
		/*	Block reading																		*/
		/*------------------------------------------------------------------------------------------*/
		BlockNum = RegSize/ReadDataSize;		/* Block read count */

		if( (BlockNum > 0) && ( ReadDataNum > (i + BlockNum - 1)) )
		{
			i = i + (BlockNum-1);				/* Block read counti	*/
		}
		else
		{	
			/* Quantity defect */
			mbusFuncErrorManager42H( MbusIfPtr, DATANUMB_ERROR, CurrentAddress );
			ret = DATANUMB_ERROR;
			return ret;
		}

		/* Data reading */
		AccessPrm.IfHandle = MbusIfPtr;
		ret = (LONG)PrmReadValue( &AccessPrm, &Value, RegSize);
		if( PRM_RSLT_SUCCESS != ret )
		{
			mbusFuncErrorManager42H( MbusIfPtr, ret, CurrentAddress );
			return ret;
		}

		/* Send data set */
		switch (RegSize)
		{
			case 1:		/* bytetype */
				ResBuf[RspBuffOffset + (i*RegSize)] = (UCHAR)(Value);
				break;
			case 2:		/* shorttype */
				mbusFuncSetArrayShortData( &ResBuf[RspBuffOffset + (i*RegSize)], (USHORT)Value, ReadDataSize );
				break;
			case 4:		/* longtype */
				mbusFuncSetArrayLongData( &ResBuf[RspBuffOffset + (i*RegSize)], (ULONG)Value, ReadDataSize );
				break;
		}
		
	}

	
	return ret;
}

/****************************************************************************************************/
/*																									*/
/*		ProductID2:Specification storage function																		*/
/*																									*/
/****************************************************************************************************/
static void mbusFuncSetSpecialSpecIdData( MEMOBUS_IF *MbusIfPtr, LONG RspBuffOffset)
{

	CHAR ServoSpecialSpec[4];

	/* Receive, send data start address */
	UCHAR  *ResBuf;
	ResBuf = MbusIfPtr->ResBuf;	

	/* Special specification number(SERVO_SPECIAL_SPEC_VER)Acquisition */
	ServoSpecialSpec[0] = (CHAR)(0x000f & (SERVO_SPECIAL_SPEC_VER >> 12) );
	ServoSpecialSpec[1] = (CHAR)(0x000f & (SERVO_SPECIAL_SPEC_VER >> 8) );
	ServoSpecialSpec[2] = (CHAR)(0x000f & (SERVO_SPECIAL_SPEC_VER >> 4) );
	ServoSpecialSpec[3] = (CHAR)(0x000f & (SERVO_SPECIAL_SPEC_VER) );

	/* Storing special specification type in response buffer */
	if( ServoSpecialSpec[3] == 0x00 )	/* For standard specifications */
	{
		/* 0x00 = Standard specifications:'0' */
		ResBuf[RspBuffOffset] = NumAsciiTbl[ServoSpecialSpec[3]];

	}
	else	/* For special specifications */
	{
		/* 0x01 = FTSpecifications:'F' */
		/* 0x02 = EXSpecifications:'E' */
		/* 0x03 = YSpecification :'Y' */
		ResBuf[RspBuffOffset] = StrAsciiTbl[(ServoSpecialSpec[3] -  1)];

	}

	/* Special specification number(ASCIICode)in response buffer */
	ResBuf[RspBuffOffset + 1] = NumAsciiTbl[ServoSpecialSpec[2]];
	ResBuf[RspBuffOffset + 2] = NumAsciiTbl[ServoSpecialSpec[1]];
	ResBuf[RspBuffOffset + 3] = NumAsciiTbl[ServoSpecialSpec[0]];

	return;
}

#endif
/* <S061>	End */
#endif	/* <S150> */

/***************************************** end of file **********************************************/

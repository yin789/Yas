/****************************************************************************
Description	: Serial Encoder Command Interface
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : Mercury

----------------------------------------------------------------------------
Changes	      :
	Name   Date          Description
	------------------------------------------------------------------------
	Y.Oka  2010.02.23    created
	I.M.   2011.02.08    reviewed
	Y.Oka  2012.08.20    remaked

****************************************************************************/
#include "Basedef.h"
#include "SerialEncIf.h"
#include "MLib.h"
#include "KLib.h"

#define	ERR5DEG		910			/* Electrical angle5°[2^16/360deg] */
#define CMD_SND_NUM	50			/* Multiturn reset Number of command transmissions */		/* <S0BB> */
#define BUSSY		2			/* Multiturn reset Number of command transmissions */		/* <S0BB> */


/* Open Key Table : Password to system mode 'ASKAWA SABS'	*/
static const USHORT OpenKeyTbl[8] =
{
	0x0000, 0x5341, 0x414B, 0x4157, 0x5320, 0x4241, 0x2053, 0x2020
};


#if defined(_VC_DEBUG_) /* 2012.08.20 Y.Oka Make Mercury Base Software */
extern	const UCHAR MotorPrm[];
#endif



/****************************************************************************
  Description: P0Mode switching processing
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
		UINT ch						: Channel number of the ASICS
		ASIC_HW_IF_STR *AsicHwReg	: Pointer to the ASIC register
  Return		:
		none
*****************************************************************************/
void SencInitChannel( MENCV *MencV, UINT ch, ASIC_HW_IF_STR *encAsicHwReg, ASIC_HW_IF_STR *posCmpAsicHwReg )
{
	MlibResetByteMemory(MencV, sizeof(MENCV));

	if(ch == 0)
	{
		MencV->EncIf.ch = 0;
		MencV->EncIf.SencSetReg = encAsicHwReg;/* <S04D> */
		MencV->EncIf.PosCmpSetReg = posCmpAsicHwReg;/* <S04D> */

		/* 2012.08.20 Y.Oka Make Mercury Base Software */
		/* ★Serial encoderI/FReexamination required★ */

		/* Ch0Transmit buffer address */
		MencV->EncIf.TxBuf = (VULONG*)&(encAsicHwReg->AREG_SE0_TXD01);	/* <S04D>:rename */
		/* Ch0Receive buffer address */
		MencV->EncIf.RxBuf = (VULONG*)&(encAsicHwReg->AREG_SE0_RXD01);	/* <S04D>:rename */

		/* ★Serial encoderI/FReexamination required★ */
		/* 2012.08.20 Y.Oka Make Mercury Base Software */

		/* Ch0Reception completion flag address */
		MencV->EncIf.RxFlg = &(encAsicHwReg->AREG_SEPGSET01);	/* <S04D>:rename */
		/* ChTransmission start write register address */
		MencV->EncIf.TxStart = &(encAsicHwReg->AREG_ESYNC);	/* <S04D>:rename */
		/* Ch0Transmission start command data(ESYNC0) */
		MencV->EncIf.TxCmd = KPX_CDAT_ENC0_ESYNC;/* <S04D>:rename */
	}
	else
	{
		MencV->EncIf.ch = 1;
		MencV->EncIf.SencSetReg = encAsicHwReg;/* <S04D> */
		MencV->EncIf.PosCmpSetReg = posCmpAsicHwReg;/* <S04D> */

		/* 2012.08.20 Y.Oka Make Mercury Base Software */
		/* ★Serial encoderI/FReexamination required★ */

		/* Ch1Transmit buffer address */
		MencV->EncIf.TxBuf = (VULONG*)&(encAsicHwReg->AREG_SE1_TXD01);	/* <S04D>:rename */
		/* Ch1Receive buffer address */
		MencV->EncIf.RxBuf = (VULONG*)&(encAsicHwReg->AREG_SE1_RXD01);	/* <S04D>:rename */

		/* ★Serial encoderI/FReexamination required★ */
		/* 2012.08.20 Y.Oka Make Mercury Base Software */

		/* Ch1Reception completion flag address */
		MencV->EncIf.RxFlg = &(encAsicHwReg->AREG_SEPGSET11);	/* <S04D>:rename */
		/* ChTransmission start write register address */
		MencV->EncIf.TxStart = &(encAsicHwReg->AREG_ESYNC);	/* <S04D>:rename */
		/* Ch1Transmission start command data(ESYNC1) */
		MencV->EncIf.TxCmd = KPX_CDAT_ENC1_ESYNC;
	}
}


/****************************************************************************
  Description: P0Mode switching processing(For initialization process)				<S0F8>
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
  Return		:
		none
*****************************************************************************/
void SencInitChangeP0Mode( MENCV *MencV )
{
	/* Change P0 Mode */
	SHalDrv_InitChangeToP0Mode( MencV->EncIf.SencSetReg, MencV->EncIf.ch );
}


/****************************************************************************
  Description: P0Mode switching processing
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
  Return		:
		none
*****************************************************************************/
void SencChangeP0Mode( MENCV *MencV )
{
	/* Change P0 Mode */
	SHalDrv_ChangeToP0Mode( MencV->EncIf.SencSetReg, MencV->EncIf.ch );	/* <S04D> Argument change */

	/* Set Interrupt Cycle (P0mode) Request <S1D3> */
	if( MencV->EncIf.ch == 0 )
	{
		MencV->SetP0ModeIntCycleReq = TRUE;
	}
}

/****************************************************************************
  Description: P1Mode switching processing
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
  Return		:
		none
*****************************************************************************/
void SencChangeP1Mode( MENCV *MencV )
{
	/* Change P1 Mode */
	SHalDrv_ChangeToP1Mode( MencV->EncIf.SencSetReg, MencV->EncIf.ch );	/* <S04D> Argument change */
	MencV->EncComErrCnt = SHalDrv_EncAlmReset( MencV->EncIf.SencSetReg );	/* <S217> */

	/* Start ScanA */
	MencV->AccChkWaitCnt = 0;
	MencV->AccChkEnable = FALSE;

	/* Set Interrupt Cycle (P1mode) Request <S1D3> */
	if( MencV->EncIf.ch == 0 )
	{
		MencV->SetP1ModeIntCycleReq = TRUE;
	}
}

/****************************************************************************
  Description: P0Mode interrupt cycle setting process						<S1D3>
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
  Return		:
		none
*****************************************************************************/
void SencSetIntCycleP0Mode( MENCV *MencV )
{
	if( (MencV->SetP0ModeIntCycleReq == TRUE) && (MencV->ChangeP1toP0Enable == TRUE) )
	{
		/* P0:Transmission cycle 125us */
		MencV->EncIf.SencSetReg->AREG_SRCNT0 = 0x000003E7;
		/* ScanA=125us, ScanB=125us, ScanC=2ms, ScanN=125us(Unused)	*/
		MencV->EncIf.SencSetReg->AREG_INTCYCLE = 0x00000f00;
		MencV->SetP0ModeIntCycleReq = FALSE;
	}
	MencV->ChangeP1toP0Enable = FALSE;
}


/****************************************************************************
  Description: P1Mode interrupt cycle setting process						<S1D3>
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
  Return		:
		none
*****************************************************************************/
void SencSetIntCycleP1Mode( MENCV *MencV )
{
	if( MencV->SetP1ModeIntCycleReq == TRUE )
	{
		/* P1:Transmission cycle 62.5us */
//		MencV->EncIf.SencSetReg->AREG_SRCNT0 = 0x000001F3;
		/* ScanA=62.5us, ScanB=125us, ScanC=2ms, ScanN=125us(Unused) */
		MencV->EncIf.SencSetReg->AREG_INTCYCLE = 0x00011f10;
		MencV->SetP1ModeIntCycleReq = FALSE;
	}
	MencV->ChangeP1toP0Enable = TRUE;
}

/****************************************************************************
*
*		Serial encoder : P0 mode command transmission processing
*
****************************************************************************/
static void SencSendP0Command( SENCX *Enc, USHORT Cmd, USHORT data )
{
	LONG			i;

	/*	Send data settings */
	switch(Cmd)
	{
	case SPG_PREAD:
	case SPG_PWRITE:
	case SPG_MTLMTCHG:
//		Enc->TxBuf[1] = data;
		Enc->TxData.UsBuf[1] = data;
		break;

	case SPG_OPENKEY:		/* Sequence review <S03F>*/
		for( i=1; i < 8; i++ )
		{
//			Enc->TxBuf[i] = OpenKeyTbl[i];
			Enc->TxData.UsBuf[i] = OpenKeyTbl[i];
		}
		break;

	case SPG_ZEROSET:
	case SPG_CLOSEKEY:
	default:
		for( i=1; i < 8; i++ )
		{
//			Enc->TxBuf[i] = data;
			Enc->TxData.UsBuf[i] = data;
		}
		break;
	}

	/*	Command code setting */
	Enc->TxData.UsBuf[0] = Cmd;

	/* Send Command Data to Encoder */
	SHalDrv_SencSetTxData( (void*)Enc->TxBuf, (void*)&(Enc->TxData) );

#if	0 /* P0While in modeScanA  Provisional measures <S0F2> */
	/* Set Encode Transmission Start Request to JL-086 */
	SHalDrv_SencStartTx( (void*)Enc->TxStart, (void*)&(Enc->TxCmd) );
#endif

	return;
}


/****************************************************************************
*
*		Serial encoder : P0 mode command transmission start & reception data acquisition processing
*
****************************************************************************/
static ULONG SencSendAndReceiveData( SENCX *Enc )
{
	ULONG	RxFlg;

	/* 2012.11.07 Y.Oka ★Sent? Confirmation required★ */
	/* Send Command Data to Encoder */
//	SHalDrv_SencSetTxData( (void*)Enc->TxBuf, (void*)&(Enc->TxData) );
	/* Encoder communication Transmission start */
//	SHalDrv_SencStartTx( (void*)Enc->TxStart, (void*)&(Enc->TxCmd) );

	/* Encoder communication Reception status acquisition */
	RxFlg = SHalDrv_SencGetRxStatus( (void*)Enc->RxFlg );
	/* Encoder communication Receive data acquisition */
	SHalDrv_SencGetRxData( (void*)Enc->RxBuf, (void*)&(Enc->RxData) );

	return RxFlg;
}


/****************************************************************************
*
*		Serial encoder : P0 mode command response check processing
*
****************************************************************************/
static LONG SencCheckP0CmdEchoBak( ENC_DATA_BUF *RxData, ULONG RxFlg, USHORT Cmd, USHORT data )
{
	LONG	ret;
	ret = OK;

	/*----------------------------------------------------------------------*/
	/*	Checking command response reception completion										*/
	/*----------------------------------------------------------------------*/
	if( (RxFlg & 0x8000) == 0 )
	{ /* Receiving incomplete */
		//ret = NG;		/* <S0BB> Delete */
		ret = BUSSY;	/* Multiturn reset Number of command transmissions */		/* <S0BB> */
	}
	else
	{
		switch(Cmd)
		{
		/*----------------------------------------------------------------------*
		*	Checking command response reception details (BUSY=0,COMERR=0,Command echo back available)
		*   ※CLOSEKEYteethCOMERRR
		*----------------------------------------------------------------------*/
		case SPG_CLOSEKEY:
			if( (RxData->EncRspKeyCntrl.sdata.header & 0x01FF) != SPG_CLOSEKEY )
			{
				ret = NG;
			}
			break;

		/*----------------------------------------------------------------------*/
		/*	READ/WRITECommand reception data check (Address+The number of bytes is correct)		*/
		/*----------------------------------------------------------------------*/
		case SPG_PREAD:
		case SPG_PWRITE:
			if( ((RxData->EncRspPrmAccess.sdata.header & 0x03FF) != (Cmd & 0x00FF))
				|| ((RxData->EncRspPrmAccess.sdata.Operand) != data) )
			{
				ret = NG;
			}
			break;
		/*----------------------------------------------------------------------*/
		/*	SGD7Wof2At the axisFn008, Fn013will fail.If there is an interim measure			<S134>		*/
		/*----------------------------------------------------------------------*/
#if	0
		case SPG_PMODEOFF:
		case SPG_MTRST:
		case SPG_MTLMTCHG:
			if( (RxData->EncRspNormal.sdata.header & 0x02FF) != (Cmd & 0x00FF) )
			{
				/* Echo back is0x00 */
				if( (RxData->EncRspNormal.sdata.header & 0x3FFF) == 0x0000)
				{
					ret = BUSSY;
				}
				else
				{
					ret = NG;
				}
			}
			break;
#endif
		default:
			if( (RxData->EncRspNormal.sdata.header & 0x03FF) != (Cmd & 0x00FF) )
			{
				ret = NG;
			}
			break;
		}
	}

	return( ret );
}

/****************************************************************************
*
*		Serial encoder : Command transmission & response check
*
****************************************************************************/
static BOOL SencSend1Command( SENCX *Enc, USHORT cmd )
{
	LONG			i;
	ULONG			RxFlg;

	for( i = 0; i < 128; i++ )
	{
		/* Set Command Code */
		Enc->TxData.EncCmdNormal.sdata.CmdCode = cmd;

		/* Send Command Data to Encoder */
		SHalDrv_SencSetTxData( (void*)Enc->TxBuf, (void*)&(Enc->TxData) );
		/* Set Encode Transmission Start Request to JL-086 */
		SHalDrv_SencStartTx( (void*)Enc->TxStart, (void*)&(Enc->TxCmd) );

		KlibWaitus( 250 );

		/* Get Encode Receive Status from JL-086 */
		RxFlg = SHalDrv_SencGetRxStatus( (void*)Enc->RxFlg );
		/* Get Encode Received Data from JL-086 */
		SHalDrv_SencGetRxData( (void*)Enc->RxBuf, (void*)&(Enc->RxData) );

		/* Check Receive Status and Response Header */
//		if( ((RxFlg & 0x8000) != 0)
//			&& ((Enc->RxData.EncRspNormal.sdata.header & 0x03FF) == cmd) )
		/* cmd = SPG_OPENKEY(0x594F),TRUE */ /*<S018>*/
		if( ((RxFlg & 0x8000) != 0)
			&& ((Enc->RxData.EncRspNormal.sdata.header & 0x03FF) == (cmd & 0x00FF)) )
		{
			return( TRUE );
		}


	}

	return( FALSE );
}



/****************************************************************************
*
*		Serial encoder : Send extended command (Nop + Cmd)
*
****************************************************************************/
static BOOL SencSendXCommand( SENCX *Enc, USHORT cmd )
{
	LONG	i;

	for( i = 0; i < 4; i++ )
	{
		if( SencSend1Command( Enc, SPG_NOP ) != FALSE )
		{
			if( SencSend1Command( Enc, cmd ) != FALSE )
			{
				return( TRUE );
			}
		}
		else
		{
			return( FALSE );
		}
	}
	/*----------------------------------------------------------------------*/
	/*	Error : EchoBack Command											*/
	/*----------------------------------------------------------------------*/
	return( FALSE );
}



/****************************************************************************
*
*		Serial encoder : Echo back check
*
****************************************************************************/
static SENC_ERROR SencCheckEchoBack( SENCX *Enc, USHORT data )
{
	LONG			i, j;
	SENC_ERROR		err;

	/* Initialize local value */
	err = SENCERR_SUCCESS;
	for( j = 0; j < ECHO_DATA_NUM; j++ )
	{
		Enc->TxData.EncCmdNormal.sdata.TestPattern[j] = data;
	}

	/*----------------------------------------------------------------------*/
	/*	Send EchoBack Command and Check Answer								*/
	/*----------------------------------------------------------------------*/
	for( i = 0; i < 4; i++ )
	{
		if( SencSend1Command( Enc, SPG_NOP ) != FALSE )
		{
			if( SencSend1Command( Enc, SPG_ECHO ) != FALSE )
			{
				/* Check the Echo Back */
				for( j = 0; j < ECHO_DATA_NUM; j++ )
				{
					if( Enc->RxData.EncRspNormal.sdata.Echo[j] != data )
					{
						err = SENCERR_PGECHO;
						break;
					}
				}
			}
			else
			{
				err = SENCERR_PGECHO;
			}
		}
		else
		{
			err = SENCERR_PGCOM;
		}
	}

	return err;
}



/****************************************************************************
*
*		Serial encoder : Parameter(8byte)Reading
*
****************************************************************************/
BOOL SencReadParam8byte( SENCX *Enc, USHORT Addr, UCHAR *PrmBuf )
{
#if !defined(_VC_DEBUG_) /* 2012.08.20 Y.Oka Make Mercury Base Software */
	LONG			i;
	USHORT			Operand;
	USHORT			rdVal;
	ULONG			RxFlg;

	/*----------------------------------------------------------------------*/
	/*	NOPCommand transmission & response reception										*/
	/*----------------------------------------------------------------------*/
	if(FALSE != SencSend1Command(Enc, SPG_NOP))
	{
		/*------------------------------------------------------------------*/
		/*	Command transmission & response reception										*/
		/*------------------------------------------------------------------*/
		Operand = Addr | 0x0800;								/* Make  Operand */
		Enc->TxData.EncCmdPrmAccess.sdata.Operand = Operand;	/* Write Operand */
		Enc->TxData.EncCmdPrmAccess.sdata.CmdCode = SPG_PREAD;	/* Write Command */

		for( i = 0; i < 128; i++ )
		{
			/* Send Command Data to Encoder */
			SHalDrv_SencSetTxData( (void*)Enc->TxBuf, (void*)&(Enc->TxData) );
			/* Set Encode Transmission Start Request to JL-086 */
			SHalDrv_SencStartTx( (void*)Enc->TxStart, (void*)&(Enc->TxCmd) );

			KlibWaitus( 250 );

			/* Get Encode Receive Status from JL-086 */
			RxFlg = SHalDrv_SencGetRxStatus( (void*)Enc->RxFlg );
			/* Get Encode Received Data from JL-086 */
			SHalDrv_SencGetRxData( (void*)Enc->RxBuf, (void*)&(Enc->RxData) );


			if( ((RxFlg & 0x8000) != 0)
				&& ((Enc->RxData.EncRspPrmAccess.sdata.header & 0x03FF) == SPG_PREAD)
				&& (Enc->RxData.EncRspPrmAccess.sdata.Operand == Operand) )
			{
				rdVal = Enc->RxData.EncRspPrmAccess.sdata.PrmData[0];
				PrmBuf[0] = (UCHAR)rdVal;
				PrmBuf[1] = (UCHAR)(rdVal >> 8);

				rdVal = Enc->RxData.EncRspPrmAccess.sdata.PrmData[1];
				PrmBuf[2] = (UCHAR)rdVal;
				PrmBuf[3] = (UCHAR)(rdVal >> 8);

				rdVal = Enc->RxData.EncRspPrmAccess.sdata.PrmData[2];
				PrmBuf[4] = (UCHAR)rdVal;
				PrmBuf[5] = (UCHAR)(rdVal >> 8);

				rdVal = Enc->RxData.EncRspPrmAccess.sdata.PrmData[3];
				PrmBuf[6] = (UCHAR)rdVal;
				PrmBuf[7] = (UCHAR)(rdVal >> 8);

				return( TRUE );
			}
		}
	}

	return( FALSE );

#else /* !defined(_VC_DEBUG_) */ /* 2012.08.20 Y.Oka Make Mercury Base Software */

	*(ULONG*)&(PrmBuf[0]) = *(ULONG*)&MotorPrm[Addr];
	*(ULONG*)&(PrmBuf[4]) = *(ULONG*)&MotorPrm[Addr+4];
	return( TRUE );

#endif /* !defined(_VC_DEBUG_) */ /* 2012.08.20 Y.Oka Make Mercury Base Software */
}



/****************************************************************************
*
*		Serial encoder : Switch target bank
*
****************************************************************************//* <S0D7> */
BOOL SencBankChange( SENCX *Enc, USHORT BankNum )
{
	LONG			i;
	ULONG			RxFlg;

	/*----------------------------------------------------------------------*/
	/*	NOPCommand transmission & response reception										*/
	/*----------------------------------------------------------------------*/
	if(FALSE != SencSend1Command(Enc, SPG_NOP))
	{
		/* Set Command Code */
		Enc->TxData.EncCmdBank.sdata.CmdCode = SPG_BANK;
		/* Set Bank Number */
		Enc->TxData.EncCmdBank.sdata.SBankNum = (BankNum & 0x00FF);
		
		for( i = 0; i < 128; i++ )
		{	
			/* Send Command Data to Encoder */
			SHalDrv_SencSetTxData( (void*)Enc->TxBuf, (void*)&(Enc->TxData) );
			/* Set Encode Transmission Start Request to JL-086 */
			SHalDrv_SencStartTx( (void*)Enc->TxStart, (void*)&(Enc->TxCmd) );
	
			KlibWaitus( 250 );
	
			/* Get Encode Receive Status from JL-086 */
			RxFlg = SHalDrv_SencGetRxStatus( (void*)Enc->RxFlg );
			/* Get Encode Received Data from JL-086 */
			SHalDrv_SencGetRxData( (void*)Enc->RxBuf, (void*)&(Enc->RxData) );
	
			/* Check Receive Status and Response Header */
			if( ((RxFlg & 0x8000) != 0)
				&& ((Enc->RxData.EncRspBank.sdata.header & 0x03FF) == SPG_BANK)
				&& ((Enc->RxData.EncRspBank.sdata.Echo_BankNum & 0x00FF) == (BankNum & 0x00FF)) )
			{
				return( TRUE );
			}
	
	
		}
	}
	return( FALSE );
	
}


#define	MOTPRM_EX_ADDR		0x005C											  /* <S0DC>	*/
#define	BANK6_FULL_SIZE		0x0100											  /* <S0DC>	*/
#define	BANK6_USE_SIZE		0x0040											  /* <S0DC>	*/
#define	BANK0_USE_SIZE		0x00A0											  /* <S0DC>	*/
#define	BANK6_NOUSE_SUM		(BANK6_FULL_SIZE - BANK6_USE_SIZE - 1) * 0xFF	  /* <S0DC>	*/
/****************************************************************************
*
*		Serial encoder : Reading all parameters
*
****************************************************************************/
SENC_ERROR SencReadParamAll( MENCV *MencV, UCHAR *PrmBuf )
{
	USHORT		addr;
	USHORT		sumx;
	UCHAR		*tmpBuf;
	UCHAR		workBuf[8];
	SENC_ERROR	err;

	sumx = 0;
	err = SENCERR_SUCCESS;
	/*----------------------------------------------------------------------*/
	/*		Motor parameter reading process(Bank0:Σ-ⅤCompatibility area)			*/
	/*----------------------------------------------------------------------*/
	/* Bank0												*/
//	for( addr = 0; addr < 0x00A0; addr += 8 )								  /* <S0DC> */
	for( addr = 0; addr < BANK0_USE_SIZE; addr += 8 )						  /* <S0DC> */
	{
		tmpBuf = &PrmBuf[addr];
		if( SencReadParam8byte( &MencV->EncIf, addr, tmpBuf ) == FALSE )
		{
			MencV->EncConnect = FALSE;
			return( SENCERR_PGCOM );	/* Abnormal return(Communication error) */
		}
		sumx += (tmpBuf[0] + tmpBuf[1] + tmpBuf[2] + tmpBuf[3]
					+ tmpBuf[4] + tmpBuf[5] + tmpBuf[6] + tmpBuf[7]);
	}
	/*----------------------------------------------------------------------*/
	/*		Sum check													*/
	/*----------------------------------------------------------------------*/
#if !defined(_VC_DEBUG_) /* 2012.08.20 Y.Oka Make Mercury Base Software */
	/* Bank0(00h~9Eh)sum check										*/
	if( (sumx & 0x00FF) != 0 )
	{
		MencV->EncConnect = FALSE;
		err = SENCERR_PRMSUM;	/* Abnormal return(Sum check) */
	}
#endif /* #if !defined(_VC_DEBUG_) */ /* 2012.08.20 Y.Oka Make Mercury Base Software */

/* <S0D7>:start */
	/*----------------------------------------------------------------------*/
	/*		Motor parameter reading process(Bank6:Σ-7Extended area)				*/
	/*----------------------------------------------------------------------*/
	if( (PrmBuf[MOTPRM_EX_ADDR] & 0x01) == 0 )					/* <S0DC>	*//* <S0ED> */
	{ /* Bank6 */
		sumx = 0;
		if(SencBankChange( &MencV->EncIf, 6 ) != FALSE)	/* Bankof6	*/
		{
//			for( addr = 0; addr < 0x0040; addr += 8 )							  /* <S0DC> */
			for( addr = 0; addr < BANK6_USE_SIZE; addr += 8 )					  /* <S0DC> */
			{
//				tmpBuf = &PrmBuf[addr + 0x00A0];								  /* <S0DC> */
				tmpBuf = &PrmBuf[addr + BANK0_USE_SIZE];						  /* <S0DC> */
				if( SencReadParam8byte( &MencV->EncIf, addr, tmpBuf ) == FALSE )
				{
					MencV->EncConnect = FALSE;
					return( SENCERR_PGCOM );	/* Abnormal return(Communication error) */
				}
				sumx += (tmpBuf[0] + tmpBuf[1] + tmpBuf[2] + tmpBuf[3]
						 + tmpBuf[4] + tmpBuf[5] + tmpBuf[6] + tmpBuf[7]);
			}

			/* Unused area data addition(Do not read to reduce processing time)				*//* <S0DC> */
			sumx += BANK6_NOUSE_SUM;

			/* Bank6sum check.									*/
//			addr = 0x00FF;														  /* <S0DC> */
			addr = 0x00F8;														  /* <S0DC> */
			if( SencReadParam8byte( &MencV->EncIf, addr, workBuf ) == FALSE )
			{
				MencV->EncConnect = FALSE;
				return( SENCERR_PGCOM );			/* Abnormal return(Communication error) */
			}
			sumx += workBuf[7];
			/* Bank0														*/
			SencBankChange( &MencV->EncIf, 0 );			/* Bankof0	*/
		}

		/*----------------------------------------------------------------------*/
		/*		Sum check													*/
		/* All acquired valuesFFh,Bank6	*/
		/*----------------------------------------------------------------------*/
		/* Bank6(00h~3Fh)sum check										*/
		if( ((sumx & 0x00FF) != 0)			/* Bank6		*/
				&& (sumx != (0xFF * 0x41)) )	/* Bank6	*/
		{
			MencV->EncConnect = FALSE;
			err = SENCERR_PRMSUM;				/* Abnormal return(Sum check)	*/
		}
	}
/* <S0D7>:end */	

	return( err );
}



/****************************************************************************
*
*		Serial encoder : Communication speed setting
*
****************************************************************************/
static BOOL SencSendSetComCommand( SENCX *Enc, UCHAR BaudRate )
{
	LONG			i;
	ULONG			RxFlg;

	for( i = 0; i < 128; i++ )
	{
		/* Set Command Code */
		Enc->TxData.EcnCmdSetCom.sdata.CmdCode = SPG_SETCOM;

		/* Set Baud Rate */
		Enc->TxData.EcnCmdSetCom.cdata.blank = 0x00;
		Enc->TxData.EcnCmdSetCom.cdata.BaudRate = BaudRate;

		/* Send Command Data to Encoder */
		SHalDrv_SencSetTxData( (void*)Enc->TxBuf, (void*)&(Enc->TxData) );
		/* Set Encode Transmission Start Request to JL-086 */
		SHalDrv_SencStartTx( (void*)Enc->TxStart, (void*)&(Enc->TxCmd) );

		KlibWaitus( 250 );

		/* Get Encode Receive Status from JL-086 */
		RxFlg = SHalDrv_SencGetRxStatus( (void*)Enc->RxFlg );
		/* Get Encode Received Data from JL-086 */
		SHalDrv_SencGetRxData( (void*)Enc->RxBuf, (void*)&(Enc->RxData) );

		/* Check Receive Status and Response Header */
		if( ((RxFlg & 0x8000) != 0)
			&& ((Enc->RxData.EcnRspSetCom.sdata.header & 0x03FF) == SPG_SETCOM)
			&& (Enc->RxData.EcnRspSetCom.cdata.Echo == SPG_SETCOM)
			&& (Enc->RxData.EcnRspSetCom.cdata.BaudRate == BaudRate) )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder : Connection confirmation process															*/
/*																									*/
/****************************************************************************************************/
/*		Continue sending BREAK command for 2 seconds to check encoder connection.						*/
/****************************************************************************************************/
SENC_ERROR SencChkConnect( SENCX *Enc )/* <S04D> */
{
	ULONG		PastTime;	/* <S223> */
	ULONG		doTimer;	/* <S223> */
	SENC_ERROR	err;

	err = SENCERR_SUCCESS;

	/*	Send Break Command and Check Answer 1st time */
	if( FALSE == SencSend1Command(Enc, SPG_BREAK) )
	{ /* Connection Error */
		/*	Send Break Command and Check Answer Retry */
		if( FALSE == SencSend1Command(Enc, SPG_BREAK) )
		{
			err = SENCERR_PGCOM;
		}
		else{ /* Valid Connection */ }
	}
	else{ /* Valid Connection */ }

	/*----------------------------------------------------------------------------------------------*/
	/*	Send Break Command and Check Answer															*/
	/*	(For Mitutoyo-Abs Scale and EnDat Interface Scale )											*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Waiting time 2sec																				*/
	/*		Mitutoyo absolute scale,EnDat I/FFor scale, for startup2secWaiting is required				*/
	/*	ToDo:Need to confirm along with actual measurement results 													*/
	/*	ToDo:BREAKCommand responseSUCCESS, it seems that no problem will occur.		*/
	/*----------------------------------------------------------------------------------------------*//* <S223> */
	if( err != SENCERR_SUCCESS )
	{
		/* Timer initialization */
		KlibRstLongTimer(&doTimer);
		do
		{
			if( TRUE == SencSend1Command(Enc, SPG_BREAK) )				/* Wait 16ms=125us*128		*/
			{/* Response confirmationOK */
				err = SENCERR_SUCCESS;
			}
			PastTime = KlibGetLongTimerMs( doTimer );					/* unit:[32ns/count]->[ms] */
		}while( (ENCODER_WAIT_TIMEOUT > PastTime)&&(err != SENCERR_SUCCESS) );

	}
	return	err;
}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder : Echo back confirmation processing													*/
/*																									*/
/****************************************************************************************************/
static SENC_ERROR SencChkEchoback( SENCX *Enc )
{
	SENC_ERROR	err;
	err = SENCERR_SUCCESS;

	/*	Send ECHO Command and Check Answer	*/
	err = SencCheckEchoBack( Enc, 0xFFFF );
	if( err == SENCERR_SUCCESS )
	{
	/*----------------------------------------------------------------------------------------------*/
	/* Initialize Communication																		*/
	/*----------------------------------------------------------------------------------------------*/
		/* Send Nop Command and Check Answer */
		if( FALSE == SencSend1Command(Enc, SPG_NOP) )
		{
			/* Send Nop Command and Check Answer Retry */
			if( FALSE == SencSend1Command(Enc, SPG_NOP) )
			{
				err = SENCERR_PGCOM;
			}
			else{ /* Valid Connection */ }
		}
		else{ /* Valid Connection */ }

		if( err == SENCERR_SUCCESS )
		{
			/*	Send ECHO Command and Check Answer	*/
			err = SencCheckEchoBack( Enc, 0x0000 );
		}
		else{ /* Connection Error */ }
	}
	else{ /* Connection Error */ }

	return( err );
}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder : Connection process															*/
/*																									*/
/****************************************************************************************************/
SENC_ERROR SencConnect( MENCV *MencV, ASIC_HW_IF_STR *AsicHwReg, USHORT EncBaudrate,  
										OPFBK_HNDL *FbkOpt, BOOL SencFbOptUse )/* <S04D>:FbkOpt,SencFbOptUseaddition */
{
#if !defined(_VC_DEBUG_) /* 2012.08.20 Y.Oka Make Mercury Base Software */

	SENCX 		*Enc;
	SENC_ERROR 	err;

	Enc = &MencV->EncIf;
	MencV->EncConnect = FALSE;			 	/* Encoder connectionFlag		*/
	MencV->AbsoEncType = MENC_INCRE_TYPE;	/* Absolute value encoder type	*/
	MencV->EncType = MENCTYPE_SERIAL;
	err = SENCERR_SUCCESS;

/* 2013.09.17 Y.Oka Move outward to improve readability */ /* <S003> */
/*--------------------------------------------------------------------------------------------------*/
/*		Start processing(P0Mode setting)																		*/
/*--------------------------------------------------------------------------------------------------*/
//	SencChangeP0Mode( MencV );

/*--------------------------------------------------------------------------------------------------*/
/*		Communication speed8Mbps/4MbpsSetting																		*/
/*--------------------------------------------------------------------------------------------------*/
//	SHalDrv_SetSencComRate( AsicHwReg, EncBaudrate );												/* <S04D> */
//	SHalDrv_SetSencComRate( AsicHwReg, EncBaudrate, MencV->EncIf.ch );
	SHalDrv_SetSencComRate( MencV->EncIf.SencSetReg, EncBaudrate, MencV->EncIf.ch );				  /* <S0E4> */			

/*--------------------------------------------------------------------------------------------------*/
/*		Encoder connection check																			*/
/*--------------------------------------------------------------------------------------------------*/
	if((SencFbOptUse == TRUE ) && (FbkOpt->var.f.ChekedConnect))/* <S04D> Addition of processing for feedback option */
	{
		err = SENCERR_SUCCESS/*FbkOpt->var.f.Connect*/;//<S0C7>
	}
	else
	{
		err = SencChkConnect( Enc );
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Processing when encoder is not connected																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( err != SENCERR_SUCCESS )
	{
		; /* @@Alarm mask addition */
		/* TODO:Will the following process be performed when not connected?MencV->EncConnect = FALSE;,ReturnShould I respond?*/
		/* TODO:Error values ​​are overwritten by echo back check.*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Echo back confirmation																			*/
/*--------------------------------------------------------------------------------------------------*/
	err = SencChkEchoback( Enc );

	if( err == SENCERR_SUCCESS )
	{
		MencV->EncConnect = TRUE;									 	/* Encoder connectionFlag		*/
	}
	else
	{
		MencV->EncConnect = FALSE;									 	/* Encoder connectionFlag		*/
	}

	return	err;

#else /* !defined(_VC_DEBUG_) */ /* 2012.08.20 Y.Oka Make Mercury Base Software */

//	SHalDrv_SetSencComRate( AsicHwReg, SENC_COMRATE_4MBPS );										/* <S04D> */
	SHalDrv_SetSencComRate( AsicHwReg, EncBaudrate, MencV->EncIf->ch );
	MencV->EncConnect = TRUE;			 	/* Encoder connectionFlag		*/
	return SENCERR_SUCCESS;

#endif /* !defined(_VC_DEBUG_) */ /* 2012.08.20 Y.Oka Make Mercury Base Software */

}



/****************************************************************************************************/
/*																									*/
/*		Serial encoder : Communication speed switching processing													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Overview :	Requests switching to the requested communication speed. Switching isSET_COM(0xFA)use.			*/
/*			SET_COMBefore sending a command, change the communication speed and check whether the requested communication speed is supported		*/
/*			Determine from the encoder constant.													*/
/*																									*/
/*			If the model does not support either communication speed switching or requested communication speed,SET_COM			*/
/*			Do not send command and return without error.					*/
/*																									*/
/****************************************************************************************************//*<S09C>*/
SENC_ERROR SencExchngBaudrate( MENCV *MencV, MENC_INIT_PRM *MencInitPrm,
								ASIC_HW_IF_STR *AsicHwReg, UCHAR BaudRate, BOOL *EncExchgSupported )
{
	BOOL		Supported;
	SENCX 		*Enc;
	SENC_ERROR	err;

	Enc = &MencV->EncIf;
	err = SENCERR_SUCCESS;
	Supported = TRUE;

	/*----------------------------------------------------------------------------------------------*/
	/*		Check if the Encoder Supports Baud Rate Exchange										*/
	/*----------------------------------------------------------------------------------------------*/
	if( (SENCBR_SUPPORT & MencInitPrm->MencP->chng_baudrate) == FALSE )
	{ /* Connected Encoder doesn't support Baud Rate Exchange */
		Supported = FALSE;
	}
	else
	{ /* Connected Encoder doesn't support Baud Rate Exchange */
		/*------------------------------------------------------------------------------------------*/
		/*		Check if the Encoder Supports Request Baud Rate										*/
		/*------------------------------------------------------------------------------------------*/
		if( BaudRate == SENC_COMRATE_8MBPS )			/* Exchange to 8MBps						*/
		{
			if( (SENCBR_8MBPS_SUP & MencInitPrm->MencP->chng_baudrate) == FALSE )
			{
				Supported = FALSE;
			}
			else
			{
				Supported = TRUE;
			}
		}
		else if( BaudRate == SENC_COMRATE_4MBPS )			/* Exchange to 4MBps		<S076>		*/
		{
			if( (SENCBR_8MBPS_SUP & MencInitPrm->MencP->chng_baudrate) == FALSE )
			{
				Supported = FALSE;
			}
			else
			{
				Supported = TRUE;
			}
		}
		else										/* Exchange to other speed (Not Supported)	*/
		{ 
			Supported = FALSE;
		}
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		Request Baud Rate Exchange to Encoder													*/
	/*----------------------------------------------------------------------------------------------*/
	if( Supported == TRUE )
	{
		if( FALSE == SencSendSetComCommand(Enc, BaudRate+1) )		/* +1teethSET_COMCorrection for command	*/
		{ /* SET_COM Command Answer Back Error */
			err |= SENCERR_BRCHNGFAIL;
		}
		else
		{ /* SET_COM Command Answer Back OK */
			/* Wait 40ms */
			KlibWaitms( 40 );

			/*--------------------------------------------------------------------------------------*/
			/*		Baud Rate Setting to Servo UDL													*/
			/*--------------------------------------------------------------------------------------*/
//			SHalDrv_SetSencComRate( AsicHwReg, BaudRate, MencV->EncIf.ch );								/* <S04D>:Argument addition */
			SHalDrv_SetSencComRate( MencV->EncIf.SencSetReg, BaudRate, MencV->EncIf.ch );								/* <S04D>:Argument addition */
			MencV->PgBaudRate = BaudRate;			/* <S192> */

			/*--------------------------------------------------------------------------------------*/
			/*		Send ECHO Command and Check Answer												*/
			/*--------------------------------------------------------------------------------------*/
			err |= SencCheckEchoBack( Enc, 0x0000 );
		}
	}
	else
	{
		; /* Connected Encoder doesn't support Baud Rate Exchange */
	}

	/*----------------------------------------------------------------------------------------------*/
	/*		Set Baud Rate Exchange State															*/
	/*----------------------------------------------------------------------------------------------*/
	*EncExchgSupported = Supported;

	return err;
}

/****************************************************************************
*
*		Serial encoder : Read initial position data
*
****************************************************************************/
SENC_ERROR SencReadInitialP1Data( MENCV *MencV )
{
	LONG	i, cnt;
	VBOOL	ReadOk;
	SENCX	*Enc;
	ULONG	RxFlg;

	Enc = &MencV->EncIf;
	RxFlg = 0;

	/* Register synchronization release */
//	AsicHwReg->AREG_SEPGSET01 |= (BIT8);

	for( i = 0; i < 5; i++ )	/* Retry 5 Times for Linear			*/
	{
		/* Start Send Command */
		ReadOk = TRUE;
		if(MencV->EncIf.ch == 0 )/* <S04D>:Ch1Mode addition */
		{/* ch0Check the connection with	*/
			Enc->TxCmd = KPX_CDAT_ENC0_ESYNC;		/* <S04D>:rename */
		}
		else
		{/* ch1Check the connection with	*/
			Enc->TxCmd = KPX_CDAT_ENC1_ESYNC;
		}

#if 0
		/* Transmission start check(Receiving current value=0OK) */
		cnt = 0;
		while( (Enc->RxFlg[0] & 0x1000) != 0 )
		{
			KlibWaitus(1);
			if((++cnt) > 65)
			{	ReadOk = FALSE; break;	}
		}
#endif
		/* Check reception completion(Latch value=Current value=1OK) */
		cnt = 0;
		while( (RxFlg & 0x9000) != 0x9000 )
		{
			/* Get Encode Receive Status from JL-086 */
			RxFlg = SHalDrv_SencGetRxStatus( (void*)Enc->RxFlg );

			KlibWaitus(1);
			if((++cnt) > 65)
			{
				ReadOk = FALSE;
				break;
			}
		}

		if(ReadOk != FALSE)
		{
			/* 2012.08.18 Y.Oka Make Mercury Base Software */
			/* Get Encode Received Data from JL-086 */
			SHalDrv_SencGetRxData( (void*)Enc->RxBuf, (void*)&(Enc->RxData) );

			/* Set Encoder Data to Internal Variables */
			MencV->RxAlmCode		= Enc->RxData.EncRspP1Mode.sdata[2];
			MencV->RxTimex.w.l		= Enc->RxData.EncRspP1Mode.sdata[3];
			MencV->RxTimex.w.h		= Enc->RxData.EncRspP1Mode.sdata[4];
			MencV->RxPosL[0].w.l	= Enc->RxData.EncRspP1Mode.sdata[5];
			MencV->RxPosL[0].w.h	= Enc->RxData.EncRspP1Mode.sdata[6];
			MencV->RxPosH[0]		= Enc->RxData.EncRspP1Mode.sdata[7];

			/* 2012.08.18 Y.Oka Make Mercury Base Software */


			/*	Alarm check : Incre pulse mode (Not required when using absolute)	*/
			if( (MencV->AbsoEncoder != FALSE)
				|| ((MencV->RxAlmCode & 0x00B5) == 0x0001) )
			{ /* Position data reading complete */
//				ReadOk = TRUE;
				break;
			}
			else
			{
				ReadOk = FALSE;
			}
		}
	}

	/* Register synchronization release */
//	AsicHwReg->AREG_SEPGSET01 &= ~(BIT8);

	/*	Data read normality check	*/
	if( ReadOk == FALSE )
	{
		MencV->EncDataNG = TRUE;	/* Encoder data invalid				*/
		return( SENCERR_PGCOM );
	}

	return SENCERR_SUCCESS;
}

/****************************************************************************
*
*		ID information setting from serial encoder information
*
****************************************************************************/
void SencSetIDINFO( USHORT addr, UCHAR *buffer, IDINFO *idinfo )
{
	LONG	i;
	SHORT	swk;

	switch ( addr )
	{
	case 0x00 :
	case 0x20 :
		 for ( i = 0; i < 8; i++ )
		 {
			idinfo->Model[i] = buffer[i];
		 }
		 break;

	case 0x08 :
	case 0x28 :
		 for ( i = 0; i < 6; i++ )
		 {
			idinfo->Model[8+i] = buffer[i];
		 }
		 swk = (SHORT) ( buffer[6] / 10 );
		 idinfo->Date[0] = MlibAsciiTbl[swk];
		 idinfo->Date[1] = MlibAsciiTbl[buffer[6]-(swk*10)];
		 idinfo->Date[2] = '.';
		 swk = (SHORT) ( buffer[7] / 10 );
		 idinfo->Date[3] = MlibAsciiTbl[swk];
		 idinfo->Date[4] = MlibAsciiTbl[buffer[7]-(swk*10)];
		 break;

	case 0x10 :
	case 0x30 :
		 for ( i = 0; i < 8; i++ )
		 {
			idinfo->SNo[i] = buffer[i];
		 }
		 break;

	case 0x18 :
	case 0x38 :
		 for ( i = 0; i < 6; i++ )
		 {
			idinfo->SNo[8+i] = buffer[i];
		 }
		 swk = ((SHORT)buffer[7] << 8) | (SHORT)buffer[6];
		 if((USHORT)swk > 9999 ) swk = 0;					/* <S214> */
		 idinfo->SNo[14] = MlibAsciiTbl[swk/1000];
		 swk = swk % 1000;
		 idinfo->SNo[15] = MlibAsciiTbl[swk/100];
		 swk = swk % 100;
		 idinfo->SNo[16] = MlibAsciiTbl[swk/10];
		 swk = swk % 10;
		 idinfo->SNo[17] = MlibAsciiTbl[swk];
		 break;

	case 0x98 :
		 for ( i = 0; i < 7; i++ )
		 {
			idinfo->Model[14+i] = buffer[i];
		 }
		 break;
	default :
		 break;
	}
}


/****************************************************************************
*
*		Insert termination code into ID information
*
****************************************************************************/
void SencFillNullIDINFO( IDINFO *idinfo )
{
	INT i;
	UCHAR ucwkl;

	/* Fill Null to Model Information End */
	for(i = 0; i < sizeof(idinfo->Model); i++)
	{
		ucwkl = idinfo->Model[i];
		if((ucwkl == ' ') || (ucwkl == '\0') || (ucwkl > 0x7F))
		{
			break;
		}
	}
	for( /*i = 0*/; i < sizeof(idinfo->Model); i++)
	{
		idinfo->Model[i] = '\0';
	}

	/* Fill Null to Date Information End */
	idinfo->Date[sizeof(idinfo->Date) - 1] = '\0';

	/* Fill Null to Serial No. Information End */
	for(i = 0; i < sizeof(idinfo->SNo); i++)
	{
		ucwkl = idinfo->SNo[i];
		if((ucwkl == ' ') || (ucwkl == '\0') || (ucwkl > 0x7F))
		{
			break;
		}
	}
	for( /*i = 0*/; i < sizeof(idinfo->SNo); i++)
	{
		idinfo->SNo[i] = '\0';
	}

}



/*--------------------------------------------------------------------------*/
/*	Origin Reset Data Table													*/
/*--------------------------------------------------------------------------*/
#define	SCNVID_JL029		0x0000		/* Serial Converter with JL-029	*/
#define	SCNVID_JL059		0x0001		/* Serial Converter with JL-059	*/
#define	SCNVID_UNKNOWN		0xFFFF		/* Unknown Serial Converter		*/
/*--------------------------------------------------------------------------*/
static const USHORT OrgRstTbl[2][4] = {
    { SCNVID_JL029, 0x8047, 0x0006, 0x0000 }, /* Serial Converter with JL-029	*/
    { SCNVID_JL059, 0x8029, 0x0011, 0x0000 }, /* Serial Converter with JL-059	*/
};

/****************************************************************************
*
*	Serial encoder: Obtain origin reset data table
*
****************************************************************************/
static CUSHORT *SencGetOrgRstTbl( IDINFO *pEncInfo )
{
	LONG TblIdx;
	USHORT ScnvId;

	/* Check Serial Converter Model for ScnvId */
	ScnvId = SCNVID_JL059;
	if((pEncInfo->Model[5] == 'A') || (pEncInfo->Model[5] == 'C'))
	{
		ScnvId = SCNVID_JL029;
	}

	/* Search Origin Reset Table */
	for(TblIdx = 0; TblIdx < sizeof(OrgRstTbl)/sizeof(OrgRstTbl[0]); TblIdx++)
	{
		if(OrgRstTbl[TblIdx][0] == ScnvId) /* Check ScnvID	*/
		{
			return (&OrgRstTbl[TblIdx][0]); /* Get Origin Reset Table */
		}
	}
	return (NULL);
}


/****************************************************************************
 *
 *	Serial encoder: Send origin reset command
 *	Reset the origin for the serial converter
 *
 ****************************************************************************/
SENC_ERROR SencSendRstOrgCmd( MENCV *MencV )
{
	LONG 			i;
	SENC_ERROR		rcx;
	CUSHORT			*OrgRstTbl;
	SENCX			*Enc;

	Enc = &MencV->EncIf;
	rcx = SENCERR_SUCCESS; /* Send Complete */

	if(MencV->OrgClrByWrAdrCmd != FALSE)
	{
		/*------------------------------------------------------------------*/
		/*	Get Origin Reset Table											*/
		/*------------------------------------------------------------------*/
		OrgRstTbl = SencGetOrgRstTbl(MencV->pActEncInfo);						/* <S04D> mod */
		if(OrgRstTbl == NULL)
		{/* No Table (Unknown Serial Converter)	*/
			return SENCERR_PGCOM;/* A.CF1 : FPG communication error	*/
		}
		/*------------------------------------------------------------------*/
		/*	Send Open Key Command											*/
		/*------------------------------------------------------------------*/
		//for(i = 0; i < 7; i++)/*<S018>*/
		for(i = 1; i < 8; i++)
		{
			//Enc->TxData.EncCmdKeyCntrl.sdata.KeyWord[i] = OpenKeyTbl[i];
			Enc->TxData.UsBuf[i]  = OpenKeyTbl[i];
		}

		if(FALSE == SencSendXCommand(Enc, SPG_OPENKEY))
		{
			rcx = SENCERR_PGCOM;/* A.CF1 : FPG communication error	*/
		}
		/*------------------------------------------------------------------*/
		/*	Set Send Data for WriteAddress Command							*/
		/*------------------------------------------------------------------*/
		Enc->TxData.UsBuf[1] = OrgRstTbl[1];			/* Set Send Data	*/
		Enc->TxData.UsBuf[2] = OrgRstTbl[2];			/* Set Send Data	*/
		for(i = 3; i < 8; i++)
		{
			Enc->TxData.UsBuf[i] = 0x0000U;
		}
		/*------------------------------------------------------------------*/
		/*	Send Write Address Command										*/
		/*------------------------------------------------------------------*/
		if(FALSE == SencSendXCommand(Enc, SPG_WRADR))
		{
			rcx = SENCERR_PGCOM;/* A.CF1 : FPG communication error	*/
		}
		/*------------------------------------------------------------------*/
		/*	Send Close Key Command											*/
		/*------------------------------------------------------------------*/
		if(FALSE == SencSendXCommand(Enc, SPG_CLOSEKEY))
		{
			rcx = SENCERR_PGCOM;/* A.CF1 : FPG communication error	*/
		}
	}
	else
	{
		/*------------------------------------------------------------------*/
		/*	Send Origin Clear Command										*/
		/*------------------------------------------------------------------*/
		if(MencV->AbsoEncoder == FALSE)
		{
			if(FALSE == SencSendXCommand(Enc, SPG_ORGCLR))
			{
				rcx = SENCERR_PGCOM; /* A.CF1 : FPG communication error	*/
			}
		}
	}

	return (rcx);
}



/****************************************************************************
  Description: Serial encoder multiturn(Multi-rotation amount)Reset processing
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
		ASIC_HW_IF_STR *AsicHwReg	: Pointer to the ASIC register
  Return		:
		SENC_ERROR
*****************************************************************************/
SENC_ERROR SencResetMultiturn(MENCV *MencV, ULONG limmlt, ULONG bit_dp_b_h)
{
	ULONG			ulwk0;
	ULONG			ulwk1;
	SENC_ERROR		rslt;
	SENCX			*Enc;
	ULONG			RxFlg;
	LONG			RsltChkEncRsp;	/* <S0BB> */

	Enc = &MencV->EncIf;
	rslt = SENCERR_BUSY;
#if 0		/*  <S0BB> Delete Start*/
	switch(MencV->MtrSeqStep)
	{
	case  0: /* Initial Step */
		/* Stop ScanA */
		MencV->SenReady = FALSE;	/* Reset SenReady		*/
		MencV->EncDisable = TRUE;	/* Set Encoder Disable	*/

		/*	Serial encoderP0Mode setting */
		SencChangeP0Mode(MencV);

		/* Check SigmaIII Encoder */
		if(MencV->Sigma3Encoder != FALSE)
		{
			/* Cancel protect mode	*/
			SencSendP0Command(Enc, SPG_PMODEOFF, 0);
			MencV->MtrSeqStep = 1;
		}
		else
		{
			/* Multiturn reset command	*/
			SencSendP0Command(Enc, SPG_MTRST, 0);
			MencV->MtrSeqStep = 2;
		}
		KlibRstLongTimer(&MencV->MtrSeqWait);
		break;

	case  1: /* SigmaIII Encoder Mode */
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 100)
		{ /* Protect mode release completed */
			KlibRstLongTimer(&MencV->MtrSeqWait);

			/* Encoder communication Transmission start & Receive data acquisition */
			SencSendP0Command(Enc, SPG_PMODEOFF, 0);		/*<S03F>*/

			/* @@Super provisional response@@ Multi-axisNG, please review	<S03F> */
			KlibWaitus( 250 );

			RxFlg = SencSendAndReceiveData( Enc );
			/* Encoder communication Response check */
			if( 0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_PMODEOFF, 0) )		/*<S03F>*/
			{
				/* Multiturn reset command	*/
				SencSendP0Command(Enc, SPG_MTRST, 0);
				MencV->MtrSeqStep = 2;
			}
			else
			{/* Protect mode release failure	*/
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);

				MencV->MtrSeqStep = 0;
				rslt = SENCERR_PGCOM;
			}
		}
		break;

	case  2: /* Multi-turn Reset Command */
		/* Waiting for protect mode release completion<S03F> */
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 100)
		{
			KlibRstLongTimer(&MencV->MtrSeqWait);

			/* Encoder communication Transmission start & Receive data acquisition */
			SencSendP0Command(Enc, SPG_MTRST, 0);		/*<S03F>*/

			/* @@Super provisional response@@ Multi-axisNG, please review <S03F>*/
			KlibWaitus( 250 );

			/* Encoder communication Transmission start & Receive data acquisition */
			RxFlg = SencSendAndReceiveData( Enc );
			/* Encoder communication Response check */
			if( 0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_MTRST, 0) )		/*<S03F>*/
			{/* Reset completed */
				MencV->MtrSeqStep = 3;
			}
			else
			{/* Protect mode release failure	*/
				MencV->MtrSeqStep = 0;
				rslt = SENCERR_PGCOM;
			}
			/*	Serial encoderP1Mode setting */
			SencChangeP1Mode(MencV);
		}
		break;

#if 0	/*<S03F>*/
	case  3: /* Check Multi-turn data */
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 4)
		{
			/* 2012.08.18 Y.Oka Make Mercury Base Software */
			/* ★Serial Encoder I/F Scheduled change.★ */

			/*	Read receive data */
//			ulwk0 = MencV->RxPosL[0].w.h = (USHORT)((MencV->EncIf.AsicHwReg)->AREG_SE0_RXD67);
//			ulwk1 = MencV->RxPosH[0] = ((MencV->EncIf.AsicHwReg)->AREG_SE0_RXD67 >> 16);

			/* ScanA Y.Oka */
			/* ★How to disable interrupts...★ */
			ulwk0 = MencV->RxPosL[0].w.h;
			ulwk1 = MencV->RxPosH[0];
			/* ★How to disable interrupts...★ */

			ulwk1 = (USHORT)(((ulwk1 << 16) | (USHORT)ulwk0) >> (bit_dp_b_h - 64));

			if( (ulwk1 <= 1) ||			/* ulwk1 = 1      : CCW1 rotation to	*/
				(ulwk1 == limmlt) )		/* ulwk1 = LIMMTT : CW 1 rotation to	*/
			{
				rslt = SENCERR_SUCCESS;
			}
			else
			{
				rslt = SENCERR_PGCOM;
			}
			MencV->MtrSeqStep = 0;
		}
		break;
#endif	/*#if 0 <S03F> */

	case  3: /* Check Multi-turn data <S03F> */
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 4)
		{
			/*	Read receive data */
			ulwk0 = MencV->RxPosL[0].w.h = (USHORT)(MencV->EncIf.SencSetReg)->AREG_SE0_RXD67;
			ulwk1 = MencV->RxPosH[0] = (USHORT)((MencV->EncIf.SencSetReg)->AREG_SE0_RXD67 >> 16);

			ulwk1 = (USHORT)(((ulwk1 << 16) | (USHORT)ulwk0) >> (bit_dp_b_h - 64));

			if( (ulwk1 <= 1) ||			/* ulwk1 = 1      : CCW1 rotation to	*/
				(ulwk1 == limmlt) )		/* ulwk1 = LIMMTT : CW 1 rotation to	*/
			{
				rslt = SENCERR_SUCCESS;
			}
			else
			{
				rslt = SENCERR_PGCOM;
			}
			MencV->MtrSeqStep = 0;
		}
		break;

	default:
		break;
	}
#endif	/* <S0BB> Delete End */
/*------------------------------------------------------------------------------------------------------------------------------------------------*/
/* <S0BB>  Add Start*/
	switch(MencV->MtrSeqStep)
	{

	/* Initial Step */
	case  0:
		/* Stop ScanA */
		MencV->SenReady = FALSE;		/* Reset SenReady		*/
		MencV->EncDisable = TRUE;		/* Set Encoder Disable	*/

		/*	Serial encoderP0Mode setting */
		SencChangeP0Mode(MencV);

		MencV->MtrSeqStep = 1;
		break;

	/* Check SigmaIII Encoder Step */
	case  1:
		/* Check SigmaIII Encoder */
		if(MencV->Sigma3Encoder != FALSE)
		{
			MencV->MtrSeqStep = 2;
		}
		else
		{
			MencV->MtrSeqStep = 5;
		}
		break;

	/* SigmaIII Encoder : Clear Protect Mode Step */
	case  2:
		/* Cancel protect mode Transmission */
		SencSendP0Command(Enc, SPG_PMODEOFF, 0);
		MencV->MtrSeqStep = 3;
		break;

	/* SigmaIII Encoder : Protect mode release command Transmission Step */
	case  3:
		/* Cancel protect mode Transmission */
		SencSendP0Command(Enc, SPG_PMODEOFF, 0);

		/* Number of transmissions Update */
		MencV->FnMturnClrCmdSndNum ++;

		MencV->MtrSeqStep = 4;
		break;

	/* SigmaIII Encoder : Check Protect Mode Step */
	case  4:
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );

		/* Encoder communication Response check */
		RsltChkEncRsp = SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_PMODEOFF, 0);
		if( RsltChkEncRsp == 0 )			/* Receiving normally completed Protected mode release successful */
		{
			MencV->FnMturnClrCmdSndNum = 0;	/* Number of transmissions 0Clear */
			MencV->MtrSeqStep = 5;
		}
		else if( RsltChkEncRsp == BUSSY )	/* Encoder communication response Receiving */
		{
			MencV->MtrSeqStep = 3;

			/* Number of transmissions is100times, it will be an error.  */
			if( MencV->FnMturnClrCmdSndNum > CMD_SND_NUM )
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);

				MencV->MtrSeqStep = 0;
				//rslt = SENCERR_PGCOM;
				rslt = SENCERR_PGCLR;/* <S1BD> */
			}
		}
		else
		{/* Protect mode release failure	*/
			/*	Serial encoderP1Mode setting */
			SencChangeP1Mode(MencV);

			MencV->MtrSeqStep = 0;
			//rslt = SENCERR_PGCOM;
			rslt = SENCERR_PGCLR;/* <S1BD> */
		}
		break;

	/* Multi-turn Reset Command Step */
	case  5:
		/* Multiturn reset command	*/
		SencSendP0Command(Enc, SPG_MTRST, 0);
		MencV->MtrSeqStep = 6;
		break;

	/* Multi-turn Reset Command : Multiturn reset command Transmission Step */
	case  6:
		/* Multiturn reset command Transmission */
		SencSendP0Command(Enc, SPG_MTRST, 0);

		/* Number of transmissions Update */
		MencV->FnMturnClrCmdSndNum ++;

		MencV->MtrSeqStep = 7;
		break;

	/* Check Multi-turn Reset Step */
	case  7:
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );

        /* Reset processing in progress(BUSYStatus) Provisional measures require review <S10F> */
        KlibRstLongTimer(&MencV->MtrSeqWait);
        while( ((Enc->RxData.UsBuf[0] & 0x0300) == 0x0100) &&
                (KlibGetLongTimerMs(MencV->MtrSeqWait) < 100) )
        {
            RxFlg = SencSendAndReceiveData( Enc );
        }

		/* Encoder communication Response check */
		RsltChkEncRsp= SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_MTRST, 0);

		if( RsltChkEncRsp == 0 )			/* Receiving normally completed Multiturn reset completion */
		{/* Reset completed */
			MencV->FnMturnClrCmdSndNum = 0;	/* Number of transmissions 0Clear */
			MencV->MtrSeqStep = 8;
		}
		else if( RsltChkEncRsp == BUSSY )	/* Encoder communication response Receiving */
		{
			MencV->MtrSeqStep = 6;

			/* Number of transmissions is100times, it will be an error.  */
			if( MencV->FnMturnClrCmdSndNum > CMD_SND_NUM )
			{
				MencV->MtrSeqStep = 0;
				//rslt = SENCERR_PGCOM;
				rslt = SENCERR_PGCLR;/* <S1BD> */
			}
		}
		else
		{/* Protect mode release failure	*/
			MencV->MtrSeqStep = 0;
			//rslt = SENCERR_PGCOM;
			rslt = SENCERR_PGCLR;/* <S1BD> */
		}
		/*	Serial encoderP1Mode setting */
		SencChangeP1Mode(MencV);

		/* Getting current elapsed time */
		KlibRstLongTimer(&MencV->MtrSeqWait);
		break;

	/* Wait 2ms Step */
	case  8:
		/* From multiturn reset completion2[ms]Check the elapsed time */
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 2)
		{
			MencV->MtrSeqStep = 9;
		}
		break;


	/* Check Multi-turn data Step */
	case  9:
		/*	Read receive data */
		if( Enc->ch == 0 )			/* <S143> */
		{
			ulwk0 = MencV->RxPosL[0].w.h = (USHORT)(MencV->EncIf.SencSetReg)->AREG_SE0_RXD67;
			ulwk1 = MencV->RxPosH[0] = (USHORT)((MencV->EncIf.SencSetReg)->AREG_SE0_RXD67 >> 16);
		}
		else
		{
			ulwk0 = MencV->RxPosL[0].w.h = (USHORT)(MencV->EncIf.SencSetReg)->AREG_SE1_RXD67;
			ulwk1 = MencV->RxPosH[0] = (USHORT)((MencV->EncIf.SencSetReg)->AREG_SE1_RXD67 >> 16);
		}

		ulwk1 = (USHORT)(((ulwk1 << 16) | (USHORT)ulwk0) >> (bit_dp_b_h - 64));

		if( (ulwk1 <= 1) ||			/* ulwk1 = 1      : CCW1 rotation to	*/
			(ulwk1 == limmlt) )		/* ulwk1 = LIMMTT : CW 1 rotation to	*/
		{
			rslt = SENCERR_SUCCESS;
		}
		else
		{
			//rslt = SENCERR_PGCOM;
			rslt = SENCERR_PGCLR;/* <S1BD> */
		}
		MencV->MtrSeqStep = 0;
		break;

	default:
		break;
	}
/* <S0BB>  Add End*/

	return rslt;
}

/****************************************************************************
  Description: Serial encoder multiturn limit change processing
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
		ASIC_HW_IF_STR *AsicHwReg	: Pointer to the ASIC register
  Return		:
		SENC_ERROR
*****************************************************************************/
SENC_ERROR SencChangeMultiturnLimit(MENCV *MencV, USHORT mtlmt)
{
	SENC_ERROR		rslt;
	ULONG			RxFlg;
	SENCX			*Enc;

	Enc = &MencV->EncIf;
	rslt = SENCERR_BUSY;

	switch(MencV->MtrSeqStep)
	{
	case  0: /* Initial Step */
		/* Stop ScanA */
		MencV->SenReady = FALSE;	/* Reset SenReady		*/
		MencV->EncDisable = TRUE;	/* Set Encoder Disable	*/

		/*	Serial encoderP0Mode setting */
		SencChangeP0Mode(MencV);

		/* Check SigmaIII Encoder */
		if(MencV->Sigma3Encoder != FALSE)
		{
			/* Cancel protect mode	*/
			SencSendP0Command(Enc, SPG_PMODEOFF, 0);
			MencV->MtrSeqStep = 1;
		}
		else
		{
			/* Multiturn limit change command	*/
			SencSendP0Command(Enc, SPG_MTLMTCHG, mtlmt);
			MencV->MtrSeqStep = 2;
		}
		KlibRstLongTimer(&MencV->MtrSeqWait);
		break;

	case  1: /* SigmaIII Encoder Mode */
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 100)
		{
			KlibRstLongTimer(&MencV->MtrSeqWait);

			/* Encoder communication Transmission start & Receive data acquisition */
			SencSendP0Command(Enc, SPG_PMODEOFF, 0);	/*<S03F>*/

			/* @@Super provisional response@@ Multi-axisNG, please review <S03F> */
			KlibWaitus( 250 );

			/* Encoder communication Transmission start & Receive data acquisition <S03F> */
			RxFlg = SencSendAndReceiveData( Enc );
			/* Encoder communication Response check */
			if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_PMODEOFF, 0))
			{
				/* Multiturn limit change command	*/
				SencSendP0Command(Enc, SPG_MTLMTCHG, mtlmt);
				MencV->MtrSeqStep = 2;
			}
			else	/* Protect mode release failure	*/
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);

				MencV->MtrSeqStep = 0;
				rslt = SENCERR_PGCOM;
			}
		}
		break;

	case  2: /* Multi-turn Reset Command */
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 100)
		{
			KlibRstLongTimer(&MencV->MtrSeqWait);

			/* Encoder communication Transmission start & Receive data acquisition */
			SencSendP0Command(Enc, SPG_MTLMTCHG, 0);	/*<S03F>*/

			/* @@Super provisional response@@ Multi-axisNG, please review <S03F> */
			KlibWaitus( 250 );

			/* Encoder communication Transmission start & Receive data acquisition */
			RxFlg = SencSendAndReceiveData( Enc );
			/* Encoder communication Response check */
			if( (0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_MTLMTCHG, 0))
				&& (mtlmt == Enc->RxData.EncRspMTLimit.sdata.MultiTurnLimit) )
			{
				rslt = SENCERR_SUCCESS;
			}
			else	/* Protect mode release failure	*/
			{
				MencV->MtrSeqStep = 0;
				rslt = SENCERR_PGCOM;
			}
			/*	Serial encoderP1Mode setting */
			SencChangeP1Mode(MencV);
		}
		break;

	default:
		break;
	}

	return rslt;
}

/****************************************************************************
  Description: Current zero phase setting
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
		ASIC_HW_IF_STR *AsicHwReg	: Pointer to the ASIC register
  Return		:
		SENC_ERROR
*****************************************************************************/
SENC_ERROR SencWriteMotorPhaseOffset( MENCV *MencV, USHORT PhaseOffset )
{
	LONG			wkl1, wkl2;
	USHORT			wkus;
	SENCX			*Enc;
	ULONG			RxFlg;
	SENC_ERROR		err;

	Enc = &MencV->EncIf;
	err = SENCERR_BUSY;

	switch(MencV->PoleWrSeqStep)
	{
	/*----------------------------------------------------------------------*/
	case  0: /* NOPCommand issue processing											*/
	/*----------------------------------------------------------------------*/
		SencChangeP0Mode(MencV);
		SencSendP0Command(Enc, SPG_NOP, 0x0000);
		MencV->PoleWrSeqStep = 1;
		KlibRstLongTimer(&MencV->PoleWrSeqWait);
		break;
	/*----------------------------------------------------------------------*/
	case  1: /* NOPCommand echo confirmation processing & OPEN_KEYCommand issue processing		*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_NOP, 0x0000))
		{
			/* OPEN_KEYCommand issue */
			SencSendP0Command(Enc, SPG_OPENKEY, 0x0000);
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 2;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  2: /* OPEN_KEYCommand echo confirmation processing								*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_OPENKEY, 0x0000))
		{
			/* READ_PRMCommand issue */
			SencSendP0Command(Enc, SPG_PREAD, 0x029e);
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 6;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  6: /* READ_PRMCommand echo confirmation processing & NOPCommand issue processing		*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_PREAD, 0x029e))
		{
			/* Checksum reading	*/
			MencV->PoleWrSum = Enc->RxData.EncRspPrmAccess.sdata.PrmData[0];
			/* NOPCommand issue */
			SencSendP0Command(Enc, SPG_NOP, 0x0000);
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 7;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  7: /* NOPCommand echo confirmation processing & READ_PRMCommand issue processing		*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_NOP, 0x0000))
		{
			/* READ_PRMCommand issue */
			SencSendP0Command(Enc, SPG_PREAD, 0x0250);
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 8;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  8: /* READ_PRMCommand echo confirmation processing & WRITE_PRMCommand issue processing	*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_PREAD, 0x0250))
		{
			/*	Checksum calculation for write */
			wkus				= Enc->RxData.EncRspPrmAccess.sdata.PrmData[0];
			wkl1				= (wkus & 0x00ff);
			wkl2				= ((wkus & 0xff00) >> 8);
			wkus				= (USHORT)(wkl1 + wkl2);
			wkl1				= (PhaseOffset & 0x00ff);
			wkl2				= ((PhaseOffset & 0xff00) >> 8);
			wkus				= (USHORT)((wkl1 + wkl2 - wkus) << 8);
			MencV->PoleWrSum	= (MencV->PoleWrSum & 0x00ff) | ((MencV->PoleWrSum - wkus) & 0xff00);

			/* Set current phase information		*/
			Enc->TxData.EncCmdPrmAccess.sdata.PrmData[0] = (USHORT)PhaseOffset;
			/* WRITE_PRMCommand issue	*/
			SencSendP0Command(Enc, SPG_PWRITE, 0x0250);
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 9;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  9: /* WRITE_PRMCommand echo check and verification processing			*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if( (0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_PWRITE, 0x0250))
			&& ((Enc->RxData.EncRspPrmAccess.sdata.PrmData[0] & 0xffff) == (PhaseOffset & 0xffff)) )
		{
			/* NOPCommand issue */
			SencSendP0Command(Enc, SPG_NOP, 0x0000 );
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 10;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 200)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  10: /* NOPCommand echo confirmation processing & WRITE_PRMCommand issue processing		*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_NOP, 0x0000))
		{
			/* Set checksum value */
			Enc->TxData.EncCmdPrmAccess.sdata.PrmData[0] = (USHORT)MencV->PoleWrSum;
			/* WRITE_PRMCommand issue */
			SencSendP0Command(Enc, SPG_PWRITE, 0x029e );
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 11;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  11: /* WRITE_PRMCommand echo confirmation and verification processing			*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if( (0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_PWRITE, 0x029e))
			&& (Enc->RxData.EncRspPrmAccess.sdata.PrmData[0] == MencV->PoleWrSum) )
		{
			/* CLOSE_KEYCommand issue */
			SencSendP0Command(Enc, SPG_CLOSEKEY, 0x0000 );
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 15;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 200)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  15: /* CLOSE_KEYCommand echo check processing						*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_CLOSEKEY, 0x0000))
		{
			MencV->PoleWrSeqStep = 16;
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			SencChangeP1Mode(MencV);
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 40)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  16: /* Scale write completion notification process									*/
	/*----------------------------------------------------------------------*/
		if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
		{
			err = SENCERR_SUCCESS;
			MencV->PoleWrSeqStep	= 0;
		}
		break;

	default:
		break;
	}

	/* Error processing:P1Go to mode */
	if((err != SENCERR_SUCCESS) && (err != SENCERR_BUSY))
	{
		SencChangeP1Mode(MencV);
		MencV->PoleWrSeqStep	= 0;
	}

	return err;
}

/****************************************************************************
  Description: Scale origin setting
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV				: Pointer to the Encoder work Handle
		ASIC_HW_IF_STR *AsicHwReg	: Pointer to the ASIC register
  Return		:
		SENC_ERROR
*****************************************************************************/
SENC_ERROR SencResetScaleZeroPosition(MENCV *MencV)
{
	LONG			wkl1, wkl2;
	SENCX			*Enc;
	ULONG			RxFlg;
	SENC_ERROR		err;

	Enc = &MencV->EncIf;
	err = SENCERR_BUSY;

	switch(MencV->PoleWrSeqStep)
	{
	/*----------------------------------------------------------------------*/
	case  0: /* NOPCommand issue processing											*/
	/*----------------------------------------------------------------------*/
		SencChangeP0Mode(MencV);
		SencSendP0Command(Enc, SPG_NOP, 0x0000);
		MencV->PoleWrSeqStep = 1;
		KlibRstLongTimer(&MencV->PoleWrSeqWait);
		break;
	/*----------------------------------------------------------------------*/
	case  1: /* NOPCommand echo confirmation processing & OPEN_KEYCommand issue processing		*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_NOP, 0x0000))
		{
			/* OPEN_KEYCommand issue */
			SencSendP0Command(Enc, SPG_OPENKEY, 0x0000);
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 2;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;

	/*----------------------------------------------------------------------*/
	case  2: /* OPEN_KEYCommand echo confirmation processing								*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_OPENKEY, 0x0000))
		{
			/* ZEROSETCommand issue */
			SencSendP0Command(Enc, SPG_ZEROSET, 0x0000 );
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 13;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;

	/*----------------------------------------------------------------------*/
	case  13: /* ZEROSETCommand echo confirmation									*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_ZEROSET, 0x0000))
		{
			/* CLOSE_KEYCommand issue */
			SencSendP0Command(Enc, SPG_CLOSEKEY, 0x0000 );
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			MencV->PoleWrSeqStep = 15;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 800)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;

	/*----------------------------------------------------------------------*/
	case  15: /* CLOSE_KEYCommand echo check processing						*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_CLOSEKEY, 0x0000))
		{
			MencV->PoleWrSeqStep = 16;
			KlibRstLongTimer(&MencV->PoleWrSeqWait);
			SencChangeP1Mode(MencV);
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 40)
			{
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  16: /* Position data import comparison										*/
	/*----------------------------------------------------------------------*/
		if(KlibGetLongTimerMs(MencV->PoleWrSeqWait) > 20)
		{
			MencV->PoleWrSeqStep	= 0;
	//		wkl1 = MencV->RcvPosX[0] >> LMX_MPOS_SFT;
			wkl1 = 0;
			wkl2 = MlibMulgainNolim( ERR5DEG, MencV->Kinvphase );
			if( wkl2 > MlibLABS(wkl1) )
			{	/* ±5°Without position deviation */
				err = SENCERR_SUCCESS;
			}
			else
			{	/* ±5°With position deviation */
				err = SENCERR_PRMDATA;
			}
		}
		break;

	default:
		break;
	}

	/* Error processing:P1Go to mode */
	if((err != SENCERR_SUCCESS) && (err != SENCERR_BUSY))
	{
		SencChangeP1Mode(MencV);
		MencV->PoleWrSeqStep	= 0;
	}

	return err;
}


#define MPRM_TRANS_BYTE		12		/* Number of motor constant transfer bytes		*/
/****************************************************************************
  Description: Read motor constant scale
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV		: Pointer to the Encoder work Handle
		USHORT address		: Address of the Encoder Parameter
		UINT num			: byte size to be read
		USHORT *buffer		: Pointer to buffer
  Return		:
		SENC_ERROR
*****************************************************************************/
SENC_ERROR SencReadMotorParameter(MENCV *MencV, USHORT address, UINT num, USHORT *buffer)
{
	SENC_ERROR		err;
	UINT 			MotPrmAddress, MotPrmSize;
	UINT 			i;
	UINT			offset;
	USHORT 			rdval;
	SENCX			*Enc;
	ULONG			RxFlg;

	Enc = &MencV->EncIf;
	err = SENCERR_BUSY;

	switch(MencV->MtrSeqStep)
	{
	/*----------------------------------------------------------------------*/
	case 0: /* Initialization process/NOPCommand issue processing								*/
	/*----------------------------------------------------------------------*/
		MencV->EncDisable = TRUE;
		SencChangeP0Mode(MencV);

		KlibRstLongTimer(&MencV->MtrSeqWait);
		MencV->MotPrmNum = 0;
		MencV->MtrSeqStep = 1;

		SencSendP0Command(Enc, SPG_NOP, 0x0000);
		break;
	/*----------------------------------------------------------------------*/
	case 1: /* NOPCommand echo check&READ_RRMIssuance processing					*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_NOP, 0x0000))
		{
			/* READ_PRMIssue command. */
			offset = MencV->MotPrmNum * MPRM_TRANS_BYTE;
			MotPrmAddress = address + offset;
			MotPrmSize = ((num - offset) > MPRM_TRANS_BYTE) ?
			             MPRM_TRANS_BYTE : num - offset;

			SencSendP0Command(Enc, SPG_PREAD,
			                  (USHORT)(MotPrmSize << 8 | MotPrmAddress));

			KlibRstLongTimer(&MencV->MtrSeqWait);
			MencV->MtrSeqStep = 2;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 20)
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);
				MencV->MtrSeqStep = 0;
				err = SENCERR_PGCOM;
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case 2: /* READ_PRMCommand echo check and data read processing		*/
	/*----------------------------------------------------------------------*/
		offset = MencV->MotPrmNum * MPRM_TRANS_BYTE;
		MotPrmSize = ((num - offset) > MPRM_TRANS_BYTE) ?
		             MPRM_TRANS_BYTE : num - offset;

		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_PREAD,
								Enc->TxData.EncCmdPrmAccess.sdata.Operand))
		/* 2012.08.18 Y.Oka Make Mercury Base Software */
		{
			for(i = 0; i < ((MotPrmSize+1)>>1); i++)
			{
				/* 2012.08.18 Y.Oka Make Mercury Base Software */
				rdval = Enc->RxData.EncRspPrmAccess.sdata.PrmData[i];
				/* 2012.08.18 Y.Oka Make Mercury Base Software */
				buffer[(offset>>1)+i] = rdval;
			}
			MencV->MotPrmNum++;
			offset = MencV->MotPrmNum * MPRM_TRANS_BYTE;
			if(num > offset)
			{
				/* NOPCommand issue */
				SencSendP0Command(Enc, SPG_NOP, 0x0000);
				MencV->MtrSeqStep = 1;
			}
			else
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);
				MencV->MtrSeqStep = 3;
			}
			KlibRstLongTimer(&MencV->MtrSeqWait);
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 20)
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);
				MencV->MtrSeqStep = 0;
				err = SENCERR_PGCOM;
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  3: /* Completion notification process									*/
	/*----------------------------------------------------------------------*/
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 20)
		{
			MencV->EncDisable = FALSE;
			err = SENCERR_SUCCESS;
			MencV->MtrSeqStep	= 0;
		}
		break;

	default:
		break;
	}

	return err;
}

/****************************************************************************
  Description: Motor constant scale writing execution
-----------------------------------------------------------------------------
  Parameters	:
		MENCV *MencV		: Pointer to the Encoder work Handle
		USHORT address		: Address of the Encoder Parameter
		UINT num			: byte size to be read
		USHORT *buffer		: Pointer to buffer
  Return		:
		SENC_ERROR
*****************************************************************************/
SENC_ERROR SencWriteMotorParameter(MENCV *MencV, USHORT address, UINT num, USHORT *buffer)
{
	SENC_ERROR		err;
	UINT 			MotPrmAddress, MotPrmSize;
	UINT 			i, offset;
	SENCX			*Enc;
	ULONG			RxFlg;

	Enc = &MencV->EncIf;
	err = SENCERR_BUSY;

	switch(MencV->MtrSeqStep)
	{
	/*----------------------------------------------------------------------*/
	case 0: /* Initialization process/NOPCommand issue processing								*/
	/*----------------------------------------------------------------------*/
		MencV->SenReady = FALSE; /* The servo will not be turned on until the power is turned back on. */
		MencV->EncDisable = TRUE;
		SencChangeP0Mode(MencV);

		KlibRstLongTimer(&MencV->MtrSeqWait);
		MencV->MotPrmNum = 0;
		MencV->MtrSeqStep = 1;

		SencSendP0Command(Enc, SPG_NOP, 0x0000);
		break;
	/*----------------------------------------------------------------------*/
	case  1: /* NOPCommand echo confirmation processing & OPEN_KEYCommand issue processing		*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_NOP, 0x0000))
		{
			/* OPEN_KEYCommand issue */
			SencSendP0Command(Enc, SPG_OPENKEY, 0x0000);
			KlibRstLongTimer(&MencV->MtrSeqWait);
			MencV->MtrSeqStep = 2;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 20)
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);
				MencV->MtrSeqStep = 0;
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  2: /* OPEN_KEYCommand echo confirmation processing								*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_OPENKEY, 0x0000))
		{
			/* NOPCommand issue */
			SencSendP0Command(Enc, SPG_NOP, 0x0000);
			KlibRstLongTimer(&MencV->MtrSeqWait);
			MencV->MtrSeqStep = 3;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 20)
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);
				MencV->MtrSeqStep = 0;
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case 3: /* NOPCommand echo check&WRITE_RRMIssuance processing					*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_NOP, 0x0000))
		{
			/* WRITE_PRMIssue command. */
			offset = MencV->MotPrmNum * MPRM_TRANS_BYTE;
			MotPrmAddress = address + offset;
			MotPrmSize = ((num - offset) > MPRM_TRANS_BYTE) ?
						  MPRM_TRANS_BYTE : num - offset;

			for(i = 0; i < ((MotPrmSize+1)>>1); i++)
			{
				Enc->TxData.EncCmdPrmAccess.sdata.PrmData[i] = buffer[(offset>>1)+i];
			}

			SencSendP0Command(Enc, SPG_PWRITE, (USHORT)(MotPrmSize << 8 | MotPrmAddress));

			KlibRstLongTimer(&MencV->MtrSeqWait);
			MencV->MtrSeqStep = 4;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 20)
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);
				MencV->MtrSeqStep = 0;
				err = SENCERR_PGCOM;
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case 4: /* WRITE_PRMCommand echo check and data read processing		*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg,
				SPG_PWRITE, Enc->TxData.EncCmdPrmAccess.sdata.Operand))
		{
			MencV->MotPrmNum++;
			offset = MencV->MotPrmNum * MPRM_TRANS_BYTE;
			if(num > offset)
			{
				/* NOPCommand issue */
				SencSendP0Command(Enc, SPG_NOP, 0x0000);
				MencV->MtrSeqStep = 3;
			}
			else
			{
				/* CLOSE_KEYCommand issue */
				SencSendP0Command(Enc, SPG_CLOSEKEY, 0x0000 );
				MencV->MtrSeqStep = 5;
			}
			KlibRstLongTimer(&MencV->MtrSeqWait);
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->MtrSeqWait) > MPRM_TRANS_BYTE * 20)
			{
				/*	Serial encoderP1Mode setting */
				SencChangeP1Mode(MencV);
				MencV->MtrSeqStep = 0;
				err = SENCERR_PGCOM;
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  5: /* CLOSE_KEYCommand echo check processing						*/
	/*----------------------------------------------------------------------*/
		/* Encoder communication Transmission start & Receive data acquisition */
		RxFlg = SencSendAndReceiveData( Enc );
		/* Encoder communication Response check */
		if(0 == SencCheckP0CmdEchoBak(&Enc->RxData, RxFlg, SPG_CLOSEKEY, 0x0000))
		{
			/*	Serial encoderP1Mode setting */
			SencChangeP1Mode(MencV);
			KlibRstLongTimer(&MencV->MtrSeqWait);
			MencV->MtrSeqStep = 6;
		}
		else
		{
			if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 40)
			{
				SencChangeP1Mode(MencV);
				MencV->MtrSeqStep = 0;
				err = SENCERR_PGCOM;/* Proceed to write failure process	*/
			}
		}
		break;
	/*----------------------------------------------------------------------*/
	case  6: /* Completion notification process									*/
	/*----------------------------------------------------------------------*/
		if(KlibGetLongTimerMs(MencV->MtrSeqWait) > 20)
		{
			MencV->EncDisable = FALSE;
			err = SENCERR_SUCCESS;
			MencV->MtrSeqStep	= 0;
		}
		break;

	default:
		break;
	}

	return err;
}



/***************************************** end of file **********************************************/

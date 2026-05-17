/****************************************************************************************************/
/*																									*/
/*																									*/
/*		FirmDownload.c : MEMOBUSFirm download management											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : MEMOBUSPerform firmware download communication IF processing.											*/
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
/*	Note	:	First edition	2013.07.08	Y.Tanaka	For Mercury											*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "MemobusIF.h"
#include "MemobusFunctions.h"
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
#include "PrmAccessIf.h"
#endif	/*_MERCURY_PLATFORM_*/
#include "MLib.h"
#include "FirmDownload.h"

//@@Temporary response
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
#include "MercuryGlobals.h"
#endif	/*_MERCURY_PLATFORM_*/
#include "Sif.h"
#include "MsgIfDef.h"
#include "KnlApi.h"

#include "elf.h"	/* Encrypted format support <S146> */
#ifdef _MERCURY_PLATFORM_	/* <S190> */
#include "SpecCommon.h"
//<S1A2>#define SERVO_ID			0x071	/* Servo modelID : SFD7S_M2 = 0x0071 */
extern USHORT	ServoId4Boot;				/* Servo modelID (Set when boot fails)*/
#define SERVO_ID	ServoId4Boot
#endif	/* <S190> */


/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
SHORT	MbusFuncSystemDownLoad( HMSGIF Hmsg );
LONG	MbusFuncSysDLGetMsgLength( UCHAR *CmdBuf );
void	MbusFuncSystemDownLoadCheckSum( void );	/* <S146> */
void	MbusFuncSysDlInitWork( void );	/* <S1A2> */

static SHORT	mbusFuncSysDLReadID50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );				/* 00H IDReading information	*/
static SHORT	mbusFuncSysDLReadSwVersion50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );		/* 01H Version read	*/
static SHORT	mbusFuncSysDLReadCondition50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );		/* 02H Program write condition acquisition	*/
static SHORT	mbusFuncSysDLStart50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );				/* 03H Download start command		*/
static SHORT	mbusFuncSysDLEraseBlock50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );			/* 04H Block erase command	*/
static SHORT	mbusFuncSysDLWriteBlock50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );			/* 05H Program write command		*/
static SHORT	mbusFuncSysDLRecvEraseInfo50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );		/* 06H Block erase information command(Erase information transfer only.Do not erase)		*/
static SHORT	mbusFuncSysDLRecvWriteInfo50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );		/* 07H Program transmission command(Program transmission only.Do not write)		*/
static SHORT	mbusFuncSysDLEraseAll50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );			/* 08H Block bulk erase command		*/
static SHORT	mbusFuncSysDLWriteAll50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength );			/* 09H Program batch write command		*/
static SHORT	mbusFuncSysDLFinish50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength);				/* 0AH Download end command		*/
static SHORT	mbusFuncSysDLReadProg50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength);			/* 0BH Program read command		*/
static SHORT	mbusFuncSysDLReboot50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength);				/* 0CH Program restart command		*/
static SHORT	mbusFuncSysDLChangeSifComSpd50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength);		/* 0DH Serial IF speed change		*/
static SHORT	mbusFuncSysDLReadHistory50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength);			/* 0EH Read download history		*/
static SHORT	mbusFuncSysDLWriteHistory50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength);		/* 0FH Write download history		*/
static SHORT	mbusFuncSysDLGetCheckSumResult50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength);	/* 10H Checksum result acquisition		*//*<S146>*/
static SHORT	mbusFuncErrorManager50H( HMSGIF Hmsg, LONG ErrorCode, USHORT *TrnbLength);			/* Abnormal response reply processing */
/*--------------------------------------------------------------------------------------------------*/
#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
static void	mbusFuncSetArrayLongData( UCHAR *Buf, ULONG data, UINT AccessSize );
#endif	/*_MERCURY_PLATFORM_*/
/*--------------------------------------------------------------------------------------------------*/
static SHORT	SysDLEraseBlock( ULONG EraseAddr, UCHAR MpuNo, UCHAR FlashNum );					/* Block erase process				*/
static SHORT	SysDLWritePage( ULONG WriteAddr, ULONG* WriteBuf, UCHAR MpuNo, UCHAR FlashNum );	/* Page write processing			*/
/*--------------------------------------------------------------------------------------------------*/
/* <S146> >>>>> */
static SHORT	checkElfZipSum( UCHAR *pTopAddress );												/* ELFZIPCheck the file sum value	*/
static SHORT	checkAreaSum( UCHAR *pTopAddress, LONG Length );									/* Checking the sum value of the specified area			*/
static SHORT	checkSum( void );																	/* Receive binary sum value check		*/
/* <<<<< <S146> */

/****************************************************************************************************/
/*																									*/
/*		VARIABLE																					*/
/*																									*/
/****************************************************************************************************/
SYSDL	SysDl;

/****************************************************************************************************/
/*																									*/
/*		MEMOBUS FirmDownload MANAGER																*/
/*																									*/
/****************************************************************************************************/
SHORT	MbusFuncSystemDownLoad( HMSGIF Hmsg )
{
	UCHAR	*CmdBuf;
	UCHAR	*ResBuf;
	UCHAR	SubFuncCode;
	UCHAR	MpuNo;
	USHORT	TrnbLength;
	SHORT	rc = MBUS_BUSY;

//	SysDl.DlStatus = 0x0001;	/* for debug */
	if(SYSDL_CHK_DLMODE_ON())			/* <S077> */
	{
		KpiRstLongTimer( &(SysDl.BpsChkTime) );	/* Reset BpsChkTimer	<S041>		*/
	}

	CmdBuf = Hmsg->CmdBuf;		/* Received message start address */
	ResBuf = Hmsg->ResBuf;		/* Send message start address */

	SubFuncCode = CmdBuf[3];	/* Subfunction code   */
	MpuNo = CmdBuf[4];
	
	//KpiRstLongTimer( &(Hmsg->ComTimeLong) );		/* For command response timeout	*/
	#if 0 /* Write prohibition currently not supported */
	RpiSetAccsLvl( &Access, MsgPtr->AccsLvl );
	#endif

	/* Common response data creation */
	ResBuf[0] = CmdBuf[0];	/* slave address	*/
	ResBuf[1] = CmdBuf[1];	/* function code	*/
	ResBuf[2] = CmdBuf[2];	/* sub function code high	*/
	ResBuf[3] = CmdBuf[3];	/* sub function code low	*/
	ResBuf[4] = CmdBuf[4];	/* MPU No	*/
	ResBuf[5] = 0x00;	/* reserved	*/

	switch( SubFuncCode )/* Subfunction code */
	{
		case MBUS_DL_ID_RD:			/* 00H IDReading information */
			rc = mbusFuncSysDLReadID50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_SWVER_RD:			/* 01H Version read */
			rc = mbusFuncSysDLReadSwVersion50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_WAY_RD:		/* 02H Program write condition acquisition	*/
			rc = mbusFuncSysDLReadCondition50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_START:		/* 03H Download start command		*/
			rc = mbusFuncSysDLStart50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_PRGDEL:		/* 04H Block erase command	*/
			rc = mbusFuncSysDLEraseBlock50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_PRGWR:	/* 05H Program write command		*/
			rc = mbusFuncSysDLWriteBlock50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_DELSND:	/* 06H Block erase information command(Erase information transfer only.Do not erase)		*/
			rc = mbusFuncSysDLRecvEraseInfo50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_PRGSND:	/* 07H Program transmission command(Program transmission only.Do not write)		*/
			rc = mbusFuncSysDLRecvWriteInfo50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_PRGDELA:	/* 08H Block bulk erase command		*/
			rc = mbusFuncSysDLEraseAll50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_PRGWRA:	/* 09H Program batch write command		*/
			rc = mbusFuncSysDLWriteAll50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_END:	/* 0AH Download end command		*/
			rc = mbusFuncSysDLFinish50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_PRGRD:	/* 0BH Program read command		*/
			rc = mbusFuncSysDLReadProg50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_REBOOT:	/* 0CH Program restart command		*/
			rc = mbusFuncSysDLReboot50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_SIFCHG:	/* 0DH Serial IF speed change		*/
			rc = mbusFuncSysDLChangeSifComSpd50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_HIS_RD:	/* 0EH Read download history		*/
			rc = mbusFuncSysDLReadHistory50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_HIS_WR:	/* 0FH Write download history		*/
			rc = mbusFuncSysDLWriteHistory50H( Hmsg, MpuNo, &TrnbLength );
			break;
		case MBUS_DL_CHKSUM:	/* 10H Checksum result acquisition		*/ /* <S146> */
			rc = mbusFuncSysDLGetCheckSumResult50H( Hmsg, MpuNo, &TrnbLength );
			break;
		default:					/* Command mismatch				*/
			rc = mbusFuncErrorManager50H( Hmsg, FUNCCODE_ERROR, &TrnbLength );
			break;
	}

	if( rc != MBUS_BUSY )
	{
		/* Transmission data length check */
		if(( TrnbLength > MBUS_MAXSIZE ) || ( TrnbLength < MBUS_DL_MIN_RSPSIZE ) )
		{
			/* Transmission data length error	*/
			rc = mbusFuncErrorManager50H( Hmsg, DATANUMB_ERROR, &TrnbLength );
		}

		/* CRCAdditional */
		MlibSetCRC16MB( Hmsg->ResBuf, TrnbLength );
	}
	else
	{
		/* No response */
		TrnbLength = 0;
	}

	/* Data transmission */
	Hmsg->Sif->StxRes( Hmsg->Sif, Hmsg->ResBuf, TrnbLength );

	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		Memobus:00H IDReading information																	*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLReadID50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
	UCHAR	*ResBuf;
	SHORT	rc = MBUS_OK;

	/* Send data start address		*/
	ResBuf = Hmsg->ResBuf;

	*TrnbLength = MBUS_DL_ID_RD_RSPSIZE;		/* Determine transmit buffer size */

	switch( MpuNo )
	{
	case 0x00:	 /* MainCPU(SoC JL-086) */
		ResBuf[6] = 0x00;	/* reserved	*/
		ResBuf[7] = FLASH_NUM;	/* Flash Num	*/
#if 0	//Finally prepare structure
		ResBuf[8] = (SysDl.DlSts1 >> 8);	/* Download Status1	*/
		ResBuf[9] = (SysDl.DlSts1 & 0x00FF);	/* Download Status1	*/
		ResBuf[10] = (SysDl.DlSts2 >> 8);	/* Download Status2	*/
		ResBuf[11] = (SysDl.DlSts2 & 0x00FF);	/* Download Status2	*/
		ResBuf[12] = (SysDl.DlSts3 >> 8);	/* Download Status3	*/
		ResBuf[13] = (SysDl.DlSts3 & 0x00FF);	/* Download Status3	*/
		ResBuf[14] = (SysDl.DlSts4 >> 8);	/* Download Status4	*/
		ResBuf[15] = (SysDl.DlSts4 & 0x00FF);	/* Download Status4	*/
//		ResBuf[16] = 0x00;	/* Servo ID	*///Standard type(Analog/Pulse train type)
//		ResBuf[17] = 0x28;	/* Servo ID	*///Standard type(Analog/Pulse train type)
		ResBuf[16] = (UCHAR)(SERVO_ID>>0);	/* Servo ID	*/
		ResBuf[17] = (UCHAR)(SERVO_ID>>2);	/* Servo ID	*/
#else	//For debugging (fixed value)
		ResBuf[8] = (SYSDL_STS1 >> 8);	/* Download Status1	*/
		ResBuf[9] = (SYSDL_STS1 & 0x00FF);	/* Download Status1	*/
		ResBuf[10] = (SYSDL_STS2 >> 8);	/* Download Status2	*/
		ResBuf[11] = (SYSDL_STS2 & 0x00FF);	/* Download Status2	*/
		ResBuf[12] = (SYSDL_STS3 >> 8);	/* Download Status3	*/
		ResBuf[13] = (SYSDL_STS3 & 0x00FF);	/* Download Status3	*/
		ResBuf[14] = (SYSDL_STS4 >> 8);	/* Download Status4	*/
		ResBuf[15] = (SYSDL_STS4 & 0x00FF);	/* Download Status4	*/
//		ResBuf[16] = 0x00;	/* Servo ID	*///Standard type(Analog/Pulse train type)
//		ResBuf[17] = 0x28;	/* Servo ID	*///Standard type(Analog/Pulse train type)
		ResBuf[16] = (UCHAR)(SERVO_ID>>4);	/* Servo ID <S150>	*/
		ResBuf[17] = (UCHAR)(SERVO_ID>>0);	/* Servo ID <S150>	*/
#endif
		ResBuf[18] = 0x00;	/* Flash0 Manufacture ID	*///FlashNumber2It is necessary to consider expansion when the number of2Will there be more than one? )
		ResBuf[19] = 0x00;	/* Flash0 Manufacture ID	*/
		ResBuf[20] = 0x00;	/* Flash0 Device ID	*/
		ResBuf[21] = 0x00;	/* Flash0 Device ID	*/
		break;
	case 0x01:		/* OptionCPU (NetX etc...) */
		/* Leave it blank for the time being.	*/
		break;
	default:
		mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
		rc = MBUS_NG;

		break;
	}

	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		Memobus:01H Version read																*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLReadSwVersion50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
	UCHAR	*ResBuf;

	/* Send data start address		*/
	ResBuf = Hmsg->ResBuf;

	*TrnbLength = MBUS_DL_SWVER_RD_RSPSIZE;			/* Determine transmit buffer size */

	switch( MpuNo )
	{
	case 0x00:	 /* MainCPU(SoC JL-086) */
		ResBuf[6] = 0x00;						/* VerNumber of definitions=2					*/
		ResBuf[7] = 0x02;						/* VerNumber of definitions=2					*/
#if 0	//Finally prepare structure
		ResBuf[8] = ( Kprm.Bt2MswVersion >> 8 );		/* Boot2 Ver		*/
		ResBuf[9] = ( Kprm.Bt2MswVersion & 0x00FF );	/* Boot2 Ver		*/
		ResBuf[10] = ( SvSw.MswVersion >> 8 );	 		/* Runtime Ver		*/
		ResBuf[11] = ( SvSw.MswVersion & 0x00FF ); 		/* Runtime Ver		*/
#else	//For debugging (fixed value)
		ResBuf[8] = (SERVO_BOOT_VER >> 4);		/* Boot2 Ver		*/
		ResBuf[9] = (SERVO_BOOT_VER >> 0);		/* Boot2 Ver		*/
//<S072>		ResBuf[10] = 0x00;		/* Runtime Ver		*/
//<S072>		ResBuf[11] = 0x01; 		/* Runtime Ver		*/
		ResBuf[10] = (SERVO_FIRM_VER >> 24 ) & 0x00FF;		/* <S072> TODO: Erase after */
		ResBuf[11] = (SERVO_FIRM_VER >> 16 ) & 0x00FF;		/* <S072> TODO: Erase after */
#endif
		break;
	case 0x01:		/* OptionCPU (NetX etc...) */
		/* Leave it blank for the time being.	*/
		break;
	default:
		mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
		break;
	}

	return (MBUS_OK);								/* <S072> No return value */
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:02H Program write condition acquisition														*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLReadCondition50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
	UCHAR	*ResBuf;
	SHORT	rc = MBUS_OK;

	/* Send data start address		*/
	ResBuf = Hmsg->ResBuf;

	*TrnbLength = MBUS_DL_WAY_RD_RSPSIZE;		/* Determine transmit buffer size */

	switch( MpuNo )
	{
	case 0x00:	 /* MainCPU(SoC JL-086) */
	case 0x01:	/* OptionCPU (NetX etc...) *//* Can we use it in common? */
		ResBuf[6] = ( SYSDL_WAY >> 8 );		/* Download write conditions		*/
		ResBuf[7] = ( SYSDL_WAY & 0x00FF );	/* Download write conditions		*/
		ResBuf[8] = ( SYSDL_1DLEN >> 8 );	 		/* 1Number of bytes per message		*/
		ResBuf[9] = ( SYSDL_1DLEN & 0x00FF ); 		/* 1Number of bytes per message		*/
		break;
	default:
		mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
		break;
	}

	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		Memobus:03H Download start command															*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLStart50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;
	SHORT	rc = MBUS_OK;

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;
	
	*TrnbLength = MBUS_DL_START_RSPSIZE;		/* Determine transmit buffer size */

	switch( MpuNo )
	{
	case 0x00:	 /* MainCPU(SoC JL-086) */
		/* If it is a base block, start downloading	*/
		if( SYSDL_CHK_BASE_BLOCK() )
		{
			SYSDL_DLMODE_ON();	/* download mode on	*/
/* <S146> >>>>> */
			/* Sum check flag => off */
			SysDl.MicroSumCheckF = FALSE;
			SysDl.MirSumCheckF = FALSE;
			SysDl.RtmSumCheckF = FALSE;
			SysDl.BootSumCheckF = FALSE;
			/* Round */
			SysDl.CheckSumRequestF = FALSE;
			if( SysDl.CheckSumCalcF == TRUE )
			{
				SysDl.CheckSumCancelF = TRUE;
			}
#ifdef	_MERCURY_PLATFORM_ 		/*<S190>*/
			else
			{
				SysDl.CheckSumCancelF = FALSE;
			}
#endif	/* _MERCURY_PLATFORM_ *//*<S190>*/
			SysDl.CheckSumEndF = FALSE;
/* <<<<< <S146> */
			KpiRstLongTimer( &(SysDl.BpsChkTime) );			/* Reset BpsChkTimer	<S041>	*/
		}
		else
		{
			SYSDL_DLMODE_OFF();	/* download mode off	*/
			mbusFuncErrorManager50H( Hmsg, CONDTION_ERROR, TrnbLength );
		}
		break;
	case 0x01:		/* OptionCPU (NetX etc...) */
		/* Leave it blank for the time being.	*/
		/* Check communication between base block and host */
		break;
	default:
		mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
		break;
	}

	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		Memobus:04H Block erase command									<S077>					*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: SpecifiedMPUofSerialFlashSector erase command issued									*/
/*		Argument: Start address of sector to be erased														*/
/*		Return value:OK(0)/BUSY(1)/NG_ADDR(-1)/NG_CMD(-2)												*/
/*			OK:Sector erase operation completed																	*/
/*			BUSY:EraseExecuting(Continuation)																	*/
/*			NG_ADDR:Setting address error																*/
/*			NG_CMD:Other commands(write/read)Executing														*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLEraseBlock50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
	UCHAR	*CmdBuf;
	UCHAR	*ResBuf;
	UCHAR	FlashNum;		/* Erase destinationFlashNumber */
	ULONG	EraseAddr;		/* Erase destinationFlashStart address */
	LONG	EraseSts;		/* Block erase command execution status */
	SHORT	rc;

	EraseSts = 0/*ERASE_OK*/;
	rc = MBUS_OK;
	
	CmdBuf = Hmsg->CmdBuf;		/* Received message start address */
	ResBuf = Hmsg->ResBuf;		/* Send message start address */

	ResBuf[6] = CmdBuf[6];		/* Flash No		*/
	ResBuf[7] = CmdBuf[7];		/* reserved		*/
	ResBuf[8] = CmdBuf[8];	 	/* erase address		*/
	ResBuf[9] = CmdBuf[9];	 	/* erase address		*/
	ResBuf[10] = CmdBuf[10]; 	/* erase address		*/
	ResBuf[11] = CmdBuf[11]; 	/* erase address		*/

	if(SYSDL_CHK_DLMODE_ON())	/* Execute if in download mode	*/
	{
		switch( MpuNo )
		{
		case 0x00:	 /* MainCPU(SoC JL-086) */
			SYSDL_ERASE_ON();		/* Start Erase Flash		*/
			FlashNum = ResBuf[6];	/* FlashNum *//* Flashand */
			EraseAddr = ((ResBuf[8]<<24)|(ResBuf[9]<<16)|(ResBuf[10]<<8)|(ResBuf[11]));	/* 0xFFE00000to64KB(0x10000)Increments	*/
#ifndef _MERCURY_PLATFORM_			/*<S190>*/
			EraseSts = SysDLEraseBlock( EraseAddr, MpuNo, FlashNum );
#else			/*<S190>*/
			/* When processing messages during boot, do not touch the boot area */
			if( EraseAddr < FLS_BOOT_TOP_ADDRESS )
			{
				EraseSts = SysDLEraseBlock( EraseAddr, MpuNo, FlashNum );
			}
#endif	/* _MERCURY_PLATFORM_ *//*<S190>*/
			if( EraseSts == 0/*ERASE_OK*/ )
			{
				SYSDL_ERASE_OFF();	/* Finish Erase Flash		*/
/* <S146> >>>>> */
				/* Set sum check flag */
				if( FLS_MICRO_CODE_ADDRESS <= EraseAddr )
				{
					if( EraseAddr < FLS_MIRROR_EFW_ADDRESS )
					{
						SysDl.MicroSumCheckF = TRUE;
					}
					else if( EraseAddr < FLS_CODE_EFW_ADDRESS )
					{
						SysDl.MirSumCheckF = TRUE;
					}
					else if( EraseAddr < FLS_BOOT_TOP_ADDRESS )
					{
						SysDl.RtmSumCheckF = TRUE;
					}
					else if( EraseAddr <= FLS_BOOT_BTM_ADDRESS )
					{
						SysDl.BootSumCheckF = TRUE;
					}
				}
/* <<<<< <S146> */
			}
			else if( EraseSts == 1/*ERASE_BUSY*/ )
			{
				rc = MBUS_BUSY;
			}
			else	/* ERASE_NG */
			{
				rc = MBUS_NG;
			}
			break;
		case 0x01:		/* OptionCPU (NetX etc...) */
			/* Leave it blank for the time being.	*/
			/* Check communication between base block and host */
			break;
		default:
			mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
			break;
		}
	}
	else
	{
		mbusFuncErrorManager50H( Hmsg, CONDTION_ERROR, TrnbLength );
	}

	if( (EraseSts != 1)/*ERASE_BUSY*/ && ((ResBuf[1] & 0x80) == 0))
	{
		*TrnbLength = MBUS_DL_PRGDEL_RSPSIZE;		/* Determine transmit buffer size */
	}
	
	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		Memobus:05H Program write command									<S077>				*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: SpecifiedMPUofSerialFlashIssue write command										*/
/*		Argument:																						*/
/*			HMSGIF *Hmsg: Write destination start address													*/
/*			UCHAR MpuNo:MPUNumber(0-16)																*/
/*		Return value:OK(0)/BUSY(1)/NG_ADDR(-1)/NG_CMD(-2)												*/
/*			OK:Writing operation completed																		*/
/*			BUSY:Executing(Continuation)																		*/
/*			NG_ADDR:Setting address error																*/
/*			NG_CMD:Other commands(write/read)Executing														*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLWriteBlock50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
	UCHAR	*CmdBuf;
	UCHAR	*ResBuf;
	UCHAR	FlashNum;		/* Write destinationFlashNumber */
	ULONG	WriteAddr;		/* WriteFlashStart address */
	LONG	WriteSts;		/* Write command execution status */
	ULONG	WriteBuf[(SYSDL_1DLEN>>2)];		/* Write data buffer */
	USHORT	WriteDataNum;
	LONG	i,j;
	SHORT	rc;

	WriteSts = 0/*WRITE_OK*/;
	rc = MBUS_OK;
	
	/* Receive data start address		*/
	CmdBuf = Hmsg->CmdBuf;		/* Received message start address */
	ResBuf = Hmsg->ResBuf;		/* Send message start address */

	ResBuf[6] = CmdBuf[6];		/* Flash No		*/
	ResBuf[7] = CmdBuf[7];		/* reserved		*/
	ResBuf[8] = CmdBuf[8];	 	/* data num(Hi)		*/
	ResBuf[9] = CmdBuf[9];	 	/* data num(Lo)		*/
	ResBuf[10] = CmdBuf[10]; 	/* write address		*/
	ResBuf[11] = CmdBuf[11]; 	/* write address		*/
	ResBuf[12] = CmdBuf[12]; 	/* write address		*/
	ResBuf[13] = CmdBuf[13]; 	/* write address		*/

	WriteDataNum = ( (ResBuf[8]<<2) | ResBuf[9] );
	if( SYSDL_1DLEN < WriteDataNum )
	{
		mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
	}
	else
	{
		for(i=14,j=0;j<(SYSDL_1DLEN>>2);i+=4,j++)
		{
			WriteBuf[j] = (((CmdBuf[i+3]<<24))|((CmdBuf[i+2]<<16))|((CmdBuf[i+1]<<8))|(CmdBuf[i]));
		}

		if(SYSDL_CHK_DLMODE_ON())	/* Execute if in download mode	*/
		{
			switch( MpuNo )
			{
			case 0x00:	 /* MainCPU(SoC JL-086) */
				SYSDL_ERASE_ON();		/* Start Erase Flash		*/
				FlashNum = ResBuf[6];	/* FlashNum *//* Flashand */

				SYSDL_WRITE_ON();		/* Start Write Flash		*/
				FlashNum = ResBuf[6];	/* FlashNum *//* Flashand */
				WriteAddr = ((ResBuf[10]<<24)|(ResBuf[11]<<16)|(ResBuf[12]<<8)|(ResBuf[13]));	/* 0xFFE00000to64KB(0x10000)Increments	*/
#ifndef _MERCURY_PLATFORM_			/*<S190>*/
				WriteSts = SysDLWritePage( WriteAddr, WriteBuf, MpuNo, FlashNum );
#else			/*<S190>*/
				/* When processing messages during boot, do not touch the boot area */
				if( WriteAddr < FLS_BOOT_TOP_ADDRESS )
				{
					WriteSts = SysDLWritePage( WriteAddr, WriteBuf, MpuNo, FlashNum );
				}
#endif	/* _MERCURY_PLATFORM_ *//*<S190>*/
				if( WriteSts == 0/*WRITE_OK*/ )
				{
					SYSDL_WRITE_OFF();	/* Finish Write Flash		*/
				}
				else if( WriteSts == 1/*WRITE_BUSY*/ )
				{
					rc = MBUS_BUSY;
				}
				else	/* WRITE_NG */
				{
					rc = MBUS_NG;
				}
				break;
			case 0x01:		/* OptionCPU (NetX etc...) */
				/* Leave it blank for the time being.	*/
				/* Check communication between base block and host */
				break;
			default:
				mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
				break;
			}
		}
		else
		{
			mbusFuncErrorManager50H( Hmsg, CONDTION_ERROR, TrnbLength );
		}
	}

	if( (WriteSts != 1)/*WRITE_BUSY*/ && ((ResBuf[1] & 0x80) == 0) )
	{
		*TrnbLength = MBUS_DL_PRGWR_RSPSIZE;		/* Determine transmit buffer size */
	}
	
	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		Memobus:06H Block erase information command(Erase information transfer only.Do not erase)							*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLRecvEraseInfo50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;//<S0C7>

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;//<S0C7>

	*TrnbLength = MBUS_DL_DELSND_RSPSIZE;		/* Determine transmit buffer size */
	return MBUS_OK;//<S0C7>
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:07H Program transmission command(Program transmission only.Do not write)																	*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLRecvWriteInfo50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;//<S0C7>

	*TrnbLength = MBUS_DL_PRGSND_RSPSIZE;		/* Determine transmit buffer size */
	return MBUS_OK;//<S0C7>
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:08H Block bulk erase command															*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLEraseAll50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;//<S0C7>

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;//<S0C7>

	*TrnbLength = MBUS_DL_PRGDELA_RSPSIZE;		/* Determine transmit buffer size */
	return MBUS_OK;//<S0C7>
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:09H Program batch write command														*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLWriteAll50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;//<S0C7>

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;//<S0C7>

	*TrnbLength = MBUS_DL_PRGWRA_RSPSIZE;		/* Determine transmit buffer size */
	return MBUS_OK;//<S0C7>
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:0AH Download end command									<S077>				*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLFinish50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*CmdBuf;//<S0C7>
//	UCHAR	*ResBuf;
	SHORT	rc = MBUS_OK;
//<S146>	SHORT	rslt = OK;

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;

	*TrnbLength = MBUS_DL_END_RSPSIZE;		/* Determine transmit buffer size */

	switch( MpuNo )
	{
	case 0x00:	 /* MainCPU(SoC JL-086) */
		/* If it is a base block, start downloading	*/
		if(SYSDL_CHK_DLMODE_ON())	/* Execute if in download mode	*/
		{
/* <S146> */
//			//rslt = checkSum();		/* firmware check sum		*/
//			if( NG == rslt )
//			{
//				rc = MBUS_NG;
//				mbusFuncErrorManager50H( Hmsg, CONDTION_ERROR, TrnbLength );
//			}
//			else
//			{
//				SYSDL_DLMODE_RESET();
//			}
			SYSDL_DLMODE_RESET();
			SysDl.CheckSumRequestF = TRUE;	/* Checksum processing start request */
		}
		else
		{
			mbusFuncErrorManager50H( Hmsg, CONDTION_ERROR, TrnbLength );
		}
		break;
	case 0x01:		/* OptionCPU (NetX etc...) */
		/* Leave it blank for the time being.	*/
		/* Check communication between base block and host */
		break;
	default:
		mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
		break;
	}

	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		Memobus:0BH Program read command															*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLReadProg50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;//<S0C7>

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;//<S0C7>

	*TrnbLength = MBUS_DL_START_RSPSIZE;		/* Determine transmit buffer size */
	return MBUS_OK;//<S0C7>
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:0CH Program restart command															*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLReboot50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;//<S0C7>

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;//<S0C7>

	*TrnbLength = MBUS_DL_REBOOT_RSPSIZE;		/* Determine transmit buffer size */
	return MBUS_OK;//<S0C7>
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:0DH Serial IF speed change						<S041>								*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLChangeSifComSpd50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//UCHAR	*ResBuf;  /* <S072> */
SHORT	rc = MBUS_OK;  /* <S072> */
USHORT	*CmdBufW = (USHORT *)(Hmsg->CmdBuf);  /* <S072> */
USHORT	*ResBufW = (USHORT *)(Hmsg->ResBuf);  /* <S072> */

	/* Send data start address */
//	ResBuf = Hmsg->CmdBuf;

	*TrnbLength = MBUS_DL_SIFCHG_RSPSIZE;		/* Determine transmit buffer size */

	switch( MpuNo )
	{
	case 0x00:	 /* MainCPU(SoC JL-086) */
//		ResBufW[0]	= CmdBufW[0];						/*+ <S072> AxisAdr, FunctionCode			*/
//		ResBufW[1]	= CmdBufW[1];						/*+ <S072> SubFunctionCode					*/
//		ResBufW[2]	= CmdBufW[2];						/*+ <S072> MPUNumber							*/
		ResBufW[3]	= CmdBufW[3];						/*+ <S072> Transfer rate upper word	*/
		ResBufW[4]	= CmdBufW[4];						/*+ <S072> Transfer rate lower word	*/
		/* Leave it blank for the time being.	*/
		break;
	case 0x01:		/* OptionCPU (NetX etc...) */
		/* Leave it blank for the time being.	*/
		break;
	default:
		mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
		break;
	}

	return( rc );
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:0EH Read download history															*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLReadHistory50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;//<S0C7>

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;//<S0C7>

	*TrnbLength = MBUS_DL_HIS_RD_RSPSIZE;		/* Determine transmit buffer size */

	return (MBUS_OK);									/* <S072> No return value */
}


/****************************************************************************************************/
/*																									*/
/*		Memobus:0FH Write download history															*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLWriteHistory50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
//	UCHAR	*ResBuf;//<S0C7>

	/* Send data start address		*/
//	ResBuf = Hmsg->ResBuf;//<S0C7>

	*TrnbLength = MBUS_DL_HIS_WR_RSPSIZE;		/* Determine transmit buffer size */

	return (MBUS_OK);									/* <S072> No return value */
}


/* <S146> >>>>> */
/****************************************************************************************************/
/*																									*/
/*		Memobus:10H Checksum result acquisition															*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncSysDLGetCheckSumResult50H( HMSGIF Hmsg, UCHAR MpuNo, USHORT *TrnbLength )
{
	UCHAR	*ResBuf;
	UCHAR	Status;
	SHORT	rc = MBUS_OK;

	Status = 0x40;	/* Others */

	/* Send data start address		*/
	ResBuf = Hmsg->ResBuf;
	
	*TrnbLength = MBUS_DL_CHKSUM_RSPSIZE;		/* Determine transmit buffer size */

	switch( MpuNo )
	{
	case 0x00:	 /* MainCPU(SoC JL-086) */
		/* Set and return the current status	*/
		if( SysDl.CheckSumCalcF == TRUE )
		{
			Status = 0x80;
		}
		else if( SysDl.CheckSumEndF == TRUE )
		{
			if( SysDl.CheckSumResult == TRUE )
			{
				Status = 0x00;	/* ChecksumOK */
			}
			else
			{
				Status = 0x01;	/* ChecksumNG */
			}
		}
		ResBuf[6] = Status;
		ResBuf[7] = 0x00;
		break;
	case 0x01:		/* OptionCPU (NetX etc...) */
		/* Leave it blank for the time being.	*/
		/* Check communication between base block and host */
		break;
	default:
		mbusFuncErrorManager50H( Hmsg, RANGOUTS_ERROR, TrnbLength );
		break;
	}
	
	return( rc );
}
/* <<<<< <S146> */


/****************************************************************************************************/
/*																									*/
/*		Memobus: Abnormal response reply processing																	*/
/*																									*/
/****************************************************************************************************/
static SHORT	mbusFuncErrorManager50H( HMSGIF Hmsg, LONG ErrorCode, USHORT *TrnbLength )
{
	UCHAR  *CmdBuf;
	UCHAR  *ResBuf;

	/* Receive, send data start address */
	CmdBuf = Hmsg->CmdBuf;
	ResBuf = Hmsg->ResBuf;

	/* Create abnormal response data */
	ResBuf[0]  =  CmdBuf[0];						/* 0:Slave Addrress		*/
	ResBuf[1]  = (CmdBuf[1] | 0x80);				/* 1:Function Code		*/
	ResBuf[2]  =  (UCHAR)ErrorCode;					/* 4:Error Code			*/

	/* Error reply size=5byte	*/
	*TrnbLength = MBUS_DL_ERROR_RSPSIZE;

	return (MBUS_OK);								/* <S072> No return value */
}

#ifndef	_MERCURY_PLATFORM_			/*<S190>*/
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
	/* Unconfirmed */
	Buf[3] = (UCHAR)((data) >> 24);
	Buf[2] = (UCHAR)((data) >> 16);
	Buf[1] = (UCHAR)((data) >>  8);
	Buf[0] = (UCHAR)( data );
#endif
}
#endif	/*_MERCURY_PLATFORM_*//*<S190>*/


/****************************************************************************************************/
/*																									*/
/*		FlashBlock erase process																		*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: SpecifiedMPUofSerialFlashSector erase command issued									*/
/*		Argument:																						*/
/*			ULONG EraseAddr	:Start address of sector to be erased											*/
/*			UCHAR MpuNo		:EraseMPUNumber														*/
/*			UCHAR FlashNum	:EraseMPUofFlashNumber (for expansion)										*/
/*		Return value:OK(0)/BUSY(1)/NG_ADDR(-1)/NG_CMD(-2)												*/
/*			OK:Sector erase operation completed																	*/
/*			BUSY:EraseExecuting(Continuation)																	*/
/*			NG_ADDR:Setting address error																*/
/*			NG_CMD:Other commands(write/read)Executing														*/
/*																									*/
/****************************************************************************************************/
static SHORT	SysDLEraseBlock( ULONG EraseAddr, UCHAR MpuNo, UCHAR FlashNum )
{
	LONG	EraseSts;
	
	EraseSts = 0/*ERASE_OK*/;
	
	switch( MpuNo )
	{
		case 0x00:	 /* MainCPU(SoC JL-086) */
			EraseSts = SFlashEraseSectorSoC( EraseAddr );
			break;
		case 0x01:		/* OptionCPU (NetX etc...) */
//			EraseSts = FlashEraseSectorOption( EraseAddr );
			break;
		default:
			EraseSts = -1/*ERASE_MPUNO_ERROR*/;
			break;
	}

	return( EraseSts );
}


/****************************************************************************************************/
/*																									*/
/*		FlashPage write processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: SpecifiedMPUofSerialFlashPage write command issue									*/
/*		Argument:																						*/
/*			ULONG WriteAddr	:Start address of page to write											*/
/*			ULONG* WriteBuf	:Write data buffer													*/
/*			UCHAR MpuNo		:WriteMPUNumber														*/
/*			UCHAR FlashNum	:WriteMPUofFlashNumber (for expansion)										*/
/*		Return value:OK(0)/BUSY(1)/NG_ADDR(-1)/NG_CMD(-2)												*/
/*			OK		:Operation completed																		*/
/*			BUSY	:Executing(Continuation)																	*/
/*			NG_ADDR	:Setting address error																*/
/*			NG_CMD	:Other commands(write/read)Executing													*/
/*																									*/
/****************************************************************************************************/
static SHORT	SysDLWritePage( ULONG WriteAddr, ULONG* WriteBuf, UCHAR MpuNo, UCHAR FlashNum )
{
	LONG	WriteSts;
	
	WriteSts = 0/*WRITE_OK*/;
	
	switch( MpuNo )
	{
		case 0x00:	 /* MainCPU(SoC JL-086) */
			WriteSts = SFlashWritePageSoC( WriteAddr, WriteBuf );
			break;
		case 0x01:		/* OptionCPU (NetX etc...) */
//			WriteSts = FlashWriteSectorOption( WriteAddr );
			break;
		default:
			WriteSts = -1/*WRITE_MPUNO_ERROR*/;
			break;
	}

	return( WriteSts );
}


/****************************************************************************************************/
/*																									*/
/*		FirmDownload : Command message length acquisition 																*/
/*																									*/
/****************************************************************************************************/
LONG	MbusFuncSysDLGetMsgLength( UCHAR *CmdBuf )
{
	UCHAR	SubFuncCode;
	SHORT	dlsize;
	LONG	lret = -1;

	SubFuncCode = CmdBuf[3];	/* Subfunction code   */


	switch( SubFuncCode )/* Subfunction code */
	{
		case MBUS_DL_ID_RD:			/* 00H IDReading information */
			lret = 8;
			break;
		case MBUS_DL_SWVER_RD:			/* 01H Version read */
			lret = 8;
			break;
		case MBUS_DL_WAY_RD:		/* 02H Program write condition acquisition	*/
			lret = 8;
			break;
		case MBUS_DL_START:		/* 03H Download start command		*/
			lret = 8;
			break;
		case MBUS_DL_PRGDEL:		/* 04H Block erase command	*/
			lret = 14;
			break;
		case MBUS_DL_PRGWR:	/* 05H Program write command		*/
			dlsize = ((CmdBuf[8]<<8)|(CmdBuf[9]));
			if( (dlsize<0) || (dlsize>32768) )	/* Data count confirmation	*/
			{
				break;
			}
			lret = dlsize + 16;
			break;
		case MBUS_DL_DELSND:	/* 06H Block erase information command(Erase information transfer only.Do not erase)		*/
			lret = 14;
			break;
		case MBUS_DL_PRGSND:	/* 07H Program transmission command(Program transmission only.Do not write)		*/
			dlsize = ((CmdBuf[8]<<8)|(CmdBuf[9]));
			if( (dlsize<0) || (dlsize>32768) )	/* Data count confirmation	*/
			{
				break;
			}
			lret = dlsize + 16;
			break;
		case MBUS_DL_PRGDELA:	/* 08H Program batch erase command		*/
			lret = 8;
			break;
		case MBUS_DL_PRGWRA:	/* 09H Program batch write command		*/
			lret = 8;
			break;
		case MBUS_DL_END:	/* 0AH Download end command		*/
			lret = 8;
			break;
		case MBUS_DL_PRGRD:	/* 0BH Program read command		*/
			lret = 14;
			break;
		case MBUS_DL_REBOOT:	/* 0CH Program restart command		*/
			lret = 8;
			break;
		case MBUS_DL_SIFCHG:	/* 0DH Serial IF speed change		*/
//<S041>		lret = 14;
			lret = 12;
			break;
		case MBUS_DL_HIS_RD:	/* 0EH Read download history		*/
			lret = 8;
			break;
		case MBUS_DL_HIS_WR:	/* 0FH Write download history		*/
			lret = 10;
			break;
		case MBUS_DL_CHKSUM:	/* 10H Checksum result acquisition		*/ /* <S146> */
			lret = 8;
			break;
		default:					/* Command mismatch				*/
			break;
	}

	return( lret );

}


/* Encrypted format support <S146>  >>>>> */
/****************************************************************************************************/
/*																									*/
/*		MEMOBUS FirmDownload Checksum processing														*/
/*																									*/
/****************************************************************************************************/
/* <S182> No in-process history as it has been rewritten for phase management */
/* <S1A3> Additional support for subphase(No history within processing) */
void	MbusFuncSystemDownLoadCheckSum( void )
{
	SHORT	rc;			/* Return value		*/

	/* Initializing variables */
	rc = CHKSUM_OK;

	/* Checksum processing request confirmation */
	if( SysDl.CheckSumRequestF == TRUE )
	{
		/* Checksum processing requestOFF */
		SysDl.CheckSumRequestF = FALSE;

		/* Settings during processing */
		SysDl.CheckSumCalcF = TRUE;

		/* Processing phase initialization */
		SysDl.CheckSumPhase = 0;
	}

	/* If processing is in progress, perform checksum calculation */
	if( SysDl.CheckSumCalcF == TRUE )
	{
		/* Cancel request confirmation */
		if( SysDl.CheckSumCancelF == FALSE )
		{
			/* Checksum calculation */
			rc = checkSum();

			/* Result setting */
			/* Complete until final phase */
			if( (rc == CHKSUM_OK) && (SysDl.CheckSumPhase == CHKSUM_PHASE_END) )
			{
				/* Result : OK */
				SysDl.CheckSumResult = TRUE;
				/* Set termination */
				SysDl.CheckSumEndF = TRUE;
			}
			/* NGIn the case of */
			else if( rc == CHKSUM_NG )
			{
				/* Result : NG */
				SysDl.CheckSumResult = FALSE;
				/* Set termination */
				SysDl.CheckSumEndF = TRUE;
			}
		}

		/* Is there a cancellation request? If the check is completed, check ProcessingOFF */
		if( (SysDl.CheckSumCancelF == TRUE) || (SysDl.CheckSumEndF == TRUE)  )
		{
			/* ProcessingOFF */
			SysDl.CheckSumCalcF = FALSE;
		}
	}

	return;
}


/****************************************************************************************************/
/*																									*/
/*		ELFZIPCheck the file sum value		 														*/
/*																									*/
/****************************************************************************************************/
/* <S1A3> Changed to check with multiple scans(No history within processing) */
static SHORT	checkElfZipSum( UCHAR *pTopAddress )
{
	SHORT			rc;					/* Return value							*/
	FIRM_HEADER_T	*pHeader;			/* Pointer to firm header		*/
	UINT32			FirmFileSize;		/* File size					*/
	UINT32			*pZipLocalHeader;	/* ZIPPointer to local header	*/
	LONG			i;					/* Loop counter					*/

	/* Initializing variables */
	rc = CHKSUM_BUSY;

	pHeader = (FIRM_HEADER_T *)pTopAddress;
	FirmFileSize = pHeader->zip_file_size + sizeof(FIRM_HEADER_T);

	switch( SysDl.CheckSumPhase & CHKSUM_SUB_PHASE_MASK )
	{
	case 0:
		/*------------------*/
		/* File check */
		/*------------------*/
		/* Check for obviously abnormal file sizes */
		if( (pHeader->zip_file_size < sizeof(UINT32))
			|| (pHeader->zip_file_size > FLS_CODE_AREA_SIZE)
			|| (FirmFileSize > FLS_CODE_AREA_SIZE) )
		{
			rc = CHKSUM_NG;
		}

		/* ZIPSignature check */
		if( rc != CHKSUM_NG )
		{
			pZipLocalHeader = (UINT32 *)(pTopAddress + sizeof(FIRM_HEADER_T));

			if( *pZipLocalHeader != ZIP_LOCAL_HEADER_SIGNATURE )
			{
				rc = CHKSUM_NG;
			}
		}

		/* Initialize */
		if( rc != CHKSUM_NG )
		{
			SysDl.CheckSumVal = 0;
			SysDl.CheckSumMem = pTopAddress;
			SysDl.CheckSumIdx = 0;
			SysDl.CheckSumPhase++;
		}
		break;

	case 1:
		/* Add all areas */
		while( SysDl.CheckSumIdx < FirmFileSize )
		{
			SysDl.CheckSumVal += (UINT16)*SysDl.CheckSumMem;	/* 1Byte by byte addition		*/
			SysDl.CheckSumMem++;
			SysDl.CheckSumIdx++;
			/* 1End once blocks have been processed */
			if( !(SysDl.CheckSumIdx & CHKSUM_BLOCK_MASK ) )
			{
				break;
			}
		}
		/* Check the sum value when reaching the end */
		if( SysDl.CheckSumIdx >= FirmFileSize )
		{
			/*--------------*/
			/* Sum check */
			/*--------------*/
			/* Subtract entire checksum value part of firm header section */
			SysDl.CheckSumMem = (UCHAR *)&(pHeader->all_check_sum);
			for( i = 0; i < sizeof(pHeader->all_check_sum); i++ )
			{
				SysDl.CheckSumVal -= (UINT16)*SysDl.CheckSumMem;	/* 1Subtract by byte		*/
				SysDl.CheckSumMem++;
			}
			/* Checksum value addition	*/
			SysDl.CheckSumVal += pHeader->all_check_sum;

			/* Result judgment */
			if( SysDl.CheckSumVal == 0 )
			{
				rc = CHKSUM_OK;
			}
			else
			{
				rc = CHKSUM_NG;
			}
		}
		break;

	default:
		rc = CHKSUM_NG;
		break;
	}

	return rc;
}


/****************************************************************************************************/
/*																									*/
/*		Checking the sum value of the specified area		 															*/
/*																									*/
/****************************************************************************************************/
/* <S1A3> Changed to check with multiple scans(No history within processing) */
static SHORT	checkAreaSum( UCHAR *pTopAddress, LONG Length )
{
	SHORT	rc;					/* Return value			*/

	/* Initializing variables */
	rc = CHKSUM_BUSY;

	switch( SysDl.CheckSumPhase & CHKSUM_SUB_PHASE_MASK )
	{
	case 0:	/* Initialize */
		SysDl.CheckSumVal = 0;
		SysDl.CheckSumMem = pTopAddress;
		SysDl.CheckSumIdx = 0;
		SysDl.CheckSumPhase++;
		break;

	case 1:	/* All areas add & checksum confirmation */
		while( SysDl.CheckSumIdx < Length )
		{
			SysDl.CheckSumVal += (USHORT)*SysDl.CheckSumMem;	/* 1Byte by byte addition		*/
			SysDl.CheckSumMem++;
			SysDl.CheckSumIdx++;
			/* 1End once blocks have been processed */
			if( !(SysDl.CheckSumIdx & CHKSUM_BLOCK_MASK) )
			{
				break;
			}
		}
		/* Check the sum value when reaching the end */
		if( SysDl.CheckSumIdx >= Length )
		{
			/* Checksum value addition	*/
			SysDl.CheckSumVal += *((USHORT *)SysDl.CheckSumMem);

			/* Result judgment */
			if( SysDl.CheckSumVal == 0 )
			{
				rc = CHKSUM_OK;
			}
			else
			{
				rc = CHKSUM_NG;
			}
		}
		break;
	default:
		rc = CHKSUM_NG;
		break;
	}

	return rc;
}


/****************************************************************************************************/
/*																									*/
/*		Receive binary sum value check		 														*/
/*																									*/
/****************************************************************************************************/
/* <S182> No in-process history as it has been rewritten for phase management */
/* <S1A3> Changed to check with multiple scans(No history within processing) */
static SHORT	checkSum( void )
{
	SHORT	rc;					/* Return value			*/
	UINT32	*pZipLocalHeader;	/* ZIPPointer to local header	*/

	/* Initializing variables */
	rc = CHKSUM_OK;

	/* Execute processing for each main phase of this time */
	switch( SysDl.CheckSumPhase >> CHKSUM_MAIN_PHASE_SHFT )
	{
	case 0:
	/*------------------*/
	/* Mirror section check */
	/*------------------*/
		if( SysDl.MirSumCheckF == TRUE )
		{
			/* Simple judgment for factory shipment */
			pZipLocalHeader = (UINT32 *)(FLS_MIRROR_EFW_ADDRESS + sizeof(FIRM_HEADER_T));
			if( (*pZipLocalHeader != ZIP_LOCAL_HEADER_SIGNATURE)
				&& ( *((UCHAR *)FLS_FACTORY_PASSWD_ADDRESS) != 0xFF) )
			{
				/* ZIPIf the signature does not exist and a password is set, there is no need to check from the factory. */
			}
			else
			{
				/* Mirror section sum check */
				rc = checkElfZipSum( (UCHAR *)FLS_MIRROR_EFW_ADDRESS );
			}
		}
		break;

	case 1:
	/*----------------------*/
	/* Runtime part check */
	/*----------------------*/
		if( SysDl.RtmSumCheckF == TRUE )
		{
			/* Runtime part sum check */
			rc = checkElfZipSum( (UCHAR *)FLS_CODE_EFW_ADDRESS );
		}
		break;

	case 2:
	/*------------------*/
	/* Boot section check */
	/*------------------*/
		if( SysDl.BootSumCheckF == TRUE )
		{
			rc = checkAreaSum( (UCHAR *)FLS_BOOT_TOP_ADDRESS, FLS_BOOT_SUM_OFFSET );
		}
		break;

	case 3:
	/*--------------------------*/
	/* Micro code section check */
	/*--------------------------*/
		if( SysDl.MicroSumCheckF == TRUE )
		{
			rc = checkAreaSum( (UCHAR *)FLS_MICRO_CODE_ADDRESS, FLS_MICRO_SUM_OFFSET );
		}
		break;

	default:
	/*--------------*/
	/* Final phase */
	/*--------------*/
		SysDl.CheckSumPhase = CHKSUM_PHASE_END;
		break;
	}

	if( (rc == CHKSUM_OK) && (SysDl.CheckSumPhase != CHKSUM_PHASE_END) )
	{
		/* Phase update */
		SysDl.CheckSumPhase = (SysDl.CheckSumPhase + CHKSUM_MAIN_PHASE_ADD) & CHKSUM_MAIN_PHASE_MASK;
	}

	return rc;
}
/* <<<<< Encrypted format support <S146> */

/* <S1A2> >>>>> */
/****************************************************************************************************/
/*																									*/
/*		Clear work						 														*/
/*																									*/
/****************************************************************************************************/
void MbusFuncSysDlInitWork( void )
{
	MlibResetByteMemory( &SysDl, sizeof(SysDl) );
	return;
}
/* <<<<< <S1A2> */
/***************************************** end of file **********************************************/

/****************************************************************************************************/
/*																									*/
/*																									*/
/*		FlashIF.c : FlashProcessing interface															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : SoCInternal and optional modulesFlashIF processing is performed.								*/
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
/*	Note	:	First edition	2013.07.09	Y.Tanaka														*/
/*																									*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include "FlashIF.h"
#include "KLib.h"


/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
void	InitSFlashSoC( void );				/* The main bodySerialFlashInitialize		*/
UCHAR	SFlashModeChgSoC( UCHAR SPI, UCHAR Read, UCHAR PreFetch, UCHAR Com );		/* The main bodySerialFlashMode change		*/
LONG	SFlashEraseSectorSoC( ULONG EraseAddr );	/* The main bodySerialFlashSector erase	*/
LONG	SFlashWritePageSoC( ULONG WriteAddr, ULONG *WriteBuf );		/* The main bodySerialFlashPage write	*/
LONG	SFlashReadByteSoC( ULONG ReadAddr, UCHAR *ReadBuf );		/* The main bodySerialFlashRead byte	*/
//LONG	SFlashEraseSectorOption( ULONG EraseAddr );		/* Option boardSerialFlashBlock erase(DPM)	*/
//LONG	SFlashWritePageOption( ULONG WriteAddr, ULONG *WriteBuf );	/* Option boardSerialFlashPage write(DPM)	*/

/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		VARIABLE																					*/
/*																									*/
/****************************************************************************************************/
typedef struct{
	struct{
		UCHAR	SPI;	/* SPImode 0/1 - SPI0/SPI3								*/
		UCHAR	Read;	/* read mode 0/1 - normal/fast							*/
		UCHAR	PreFetch;	/* pre fetch mode 0/1 - off/on						*/
		UCHAR	Com;	/* Communication mode 0/1 - rom/direct					*/
	}Mode;
	struct{
		UCHAR	erase;	/* erase status 0/1 - none/busy							*/
		UCHAR	write;	/* write status 0/1 - none/busy							*/
		UCHAR	read;	/* read status 0/1 - none/busy							*/
		UCHAR	Comm;	/* any accese status 0/1 - none/busy					*/
	}Sts;
}SFLASHVAL;
static	SFLASHVAL	SFlashV;


/****************************************************************************************************/
/*																									*/
/*		The main bodySerialFlashInitialize																		*/
/*																									*/
/****************************************************************************************************/
void InitSFlashSoC( void )
{
	SerialRomInit( );
#ifdef	_MERCURY_PLATFORM_			/*<S190>*/
    MlibResetByteMemory( &SFlashV, sizeof(SFlashV) );
#endif	/*_MERCURY_PLATFORM_*/
}


/****************************************************************************************************/
/*																									*/
/*		The main bodySerialFlashMode change																	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/****************************************************************************************************/
UCHAR	SFlashModeChgSoC( UCHAR SPI, UCHAR Read, UCHAR PreFetch, UCHAR Com )
{
	UCHAR	rc;
	
	SFlashV.Mode.SPI = SPI;
	SFlashV.Mode.Read = Read;
	SFlashV.Mode.PreFetch = PreFetch;
	SFlashV.Mode.Com = Com;
//	rc = SerialRomModeChange( SFlashV.Mode );
	rc = 0;
	
	return( rc );
}


/****************************************************************************************************/
/*																									*/
/*		The main bodySerialFlashSector erase																	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:SerialFlash/Termination monitoring										*/
/*		Argument:																						*/
/*			ULONG EraseAddr	:Start address of sector to be erased(0xFFE00000to64KB(0x10000)Increments)		*/
/*		Return value:OK(0)/BUSY(1)/NG_ADDR(-1)/NG_CMD(-2)												*/
/*			OK:Sector erase operation completed																	*/
/*			BUSY:EraseExecuting(Continuation)																	*/
/*			NG_ADDR:Setting address error																*/
/*			NG_CMD:Other commands(write/read)Executing														*/
/*																									*/
/****************************************************************************************************/
LONG	SFlashEraseSectorSoC( ULONG EraseAddr )
{
	LONG rc;		/* -1/0/1 - NG/OK/BUSY	*/

	if( EraseAddr < 0xFFE00000 )
	{
		rc = -1/*NG_ADDR*/;
	}
	else if( (EraseAddr & 0x0000FFFF) != 0x0 )
	{
		rc = -1/*NG_ADDR*/;
	}
	else
	{
		if( SFlashV.Sts.Comm == 0/*NONE*/ )	/* No running command */
		{
			/* Sector Erase */
			SFlashV.Sts.Comm = 1/*BUSY*/;
			SFlashV.Sts.erase = 1/*BUSY*/;
			SerialRomSectorErase( EraseAddr );
			rc = 1/*BUSY*/;
		}
		else	/* There is a command being executed */
		{
			if( SFlashV.Sts.erase == 1/*BUSY*/ )	/* EraseExecuting */
			{
				SFlashV.Sts.erase = (SFlashChkStatus()&0x01);		/* EraseExecution status confirmation */
				if( SFlashV.Sts.erase == 0 )		/* EraseEnd */
				{
					SFlashV.Sts.Comm = 0/*OK*/;
					rc = 0/*OK*/;
				}
				else
				{
					rc = 1/*BUSY*/;
				}
			}
			else	/* Other processing in progress */
			{
				rc = -2/*NG_CMD*/;
			}
		}
	}

	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		The main bodySerialFlashPage write																	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: SpecifiedMPUofSerialFlashPage write command issue									*/
/*		Argument:																						*/
/*			ULONG WriteAddr	:Start address of page to write											*/
/*			ULONG* WriteBuf	:Write data buffer													*/
/*		Return value:OK(0)/BUSY(1)/NG_ADDR(-1)/NG_CMD(-2)												*/
/*			OK		:Operation completed																		*/
/*			BUSY	:Executing(Continuation)																	*/
/*			NG_ADDR	:Setting address error																*/
/*			NG_CMD	:Other commands(write/read)Executing													*/
/*																									*/
/****************************************************************************************************/
LONG	SFlashWritePageSoC( ULONG WriteAddr, ULONG *WriteBuf )
{
	LONG rc;		/* -1/0/1 - NG/OK/BUSY	*/

	if( WriteAddr < 0xFFE00000 )
	{
		rc = -1/*NG_ADDR*/;
	}
	else if( (WriteAddr & 0x000000FF) != 0x0 )
	{
		rc = -1/*NG_ADDR*/;
	}
	else
	{
		if( SFlashV.Sts.Comm == 0/*NONE*/ )	/* No running command */
		{
			/* Page Program */
			SFlashV.Sts.Comm = 1/*BUSY*/;
			SFlashV.Sts.write = 1/*BUSY*/;
			SerialRomPageProgram( WriteAddr, WriteBuf, 0x40 );
			rc = 1/*BUSY*/;
		}
		else	/* There is a command being executed */
		{
			if( SFlashV.Sts.write == 1/*BUSY*/ )	/* WriteExecuting */
			{
				SFlashV.Sts.write = (SFlashChkStatus()&0x01);		/* WriteExecution status confirmation */
				if( SFlashV.Sts.write == 0 )		/* WriteEnd */
				{
					SFlashV.Sts.Comm = 0/*OK*/;
					rc = 0/*OK*/;
				}
				else
				{
					rc = 1/*BUSY*/;
				}
			}
			else	/* Other processing in progress */
			{
				rc = -2/*NG_CMD*/;
			}
		}
	}

	return( rc );

}


/****************************************************************************************************/
/*																									*/
/*		The main bodySerialFlashRead byte																	*/
/*																									*/
/****************************************************************************************************/
LONG	SFlashReadByteSoC( ULONG ReadAddr, UCHAR *ReadBuf )
{
	INT nSize;
	UCHAR pBuff[10] = {0};

	nSize = sizeof(pBuff);
	SerialRomReadDataBytes( ReadAddr, pBuff, nSize );
}




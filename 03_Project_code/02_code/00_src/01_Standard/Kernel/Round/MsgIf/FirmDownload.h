/****************************************************************************************************/
/*																									*/
/*																									*/
/*		FirmDownload.h : MEMOBUSFirm download management											*/
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




/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Download condition definition																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#define	MPUNO		0x00						/* On IF boardCPUofMPU #0				*/
#define	FLASH_NUM	0x01						/* Number of flash memories							*/
/*--------------------------------------------------------------------------------------------------*/
#define	SYSDL_STS1	0x0001						/* bit0=1	:System download support			*/
													/* bit1=0	:Model typeIDNo code					*/
													/* bit2-15	:Reservation								*/
/*--------------------------------------------------------------------------------------------------*/
#define	SYSDL_STS2	0x0001						/* bit0=1	:#0 Flash System download support	*/
													/* bit1-15	:#1-#15 Flash None					*/
/*--------------------------------------------------------------------------------------------------*/
#define	SYSDL_STS3	0x0000						/* bit0=1	:#0 Flash MultifactureIDYes				*/
													/* bit1-15	:#1-#15 Flash None					*/
/*--------------------------------------------------------------------------------------------------*/
#define	SYSDL_STS4	0x0000						/* bit0=1	:#0 Flash DeviceIDYes				*/
													/* bit1-15	:#1-#15 Flash None					*/
/*--------------------------------------------------------------------------------------------------*/
#define	SYSDL_WAY	0x0000						/* bit0-3=0	:Flash write method selection			*/
													/*				=0:Communication and flash writing in parallel	*/
/*--------------------------------------------------------------------------------------------------*/
#define	SYSDL_1DLEN	0x0100						/* Number of bytes processed per message(=256byte)	*/


/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Download command related definitions																*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*		MEMOBUS Firm download	Subfunction code									*/
/*--------------------------------------------------------------------------------------------------*/
#define	MBUS_DL_ID_RD			0x00			/* IDReading information									*/
#define	MBUS_DL_SWVER_RD		0x01			/* Version read									*/
#define	MBUS_DL_WAY_RD			0x02			/* Program write condition acquisition							*/
#define	MBUS_DL_START			0x03			/* Download start command								*/
#define	MBUS_DL_PRGDEL			0x04			/* Block erase command									*/
#define	MBUS_DL_PRGWR			0x05			/* Program write command								*/
#define	MBUS_DL_DELSND			0x06			/* Block erase information command(Erase information transfer only.Do not erase)		*/
#define	MBUS_DL_PRGSND			0x07			/* Program transmission command(Program transmission only.Do not write)		*/
#define	MBUS_DL_PRGDELA			0x08			/* Block bulk erase command								*/
#define	MBUS_DL_PRGWRA			0x09			/* Program batch write command							*/
#define	MBUS_DL_END				0x0A			/* Download end command								*/
#define	MBUS_DL_PRGRD			0x0B			/* Program read command								*/
#define	MBUS_DL_REBOOT			0x0C			/* Program restart command								*/
#define	MBUS_DL_SIFCHG			0x0D			/* Serial IF speed change								*/
#define	MBUS_DL_HIS_RD			0x0E			/* Read download history							*/
#define	MBUS_DL_HIS_WR			0x0F			/* Write download history							*/
#define	MBUS_DL_CHKSUM			0x10			/* Checksum result acquisition								*/ /* <S146> */
#define MBUS_DL_SF_IDX			3				/* Subcommand position									*/ /* <S150> */
#define MBUS_DL_DATANUM_LEN		10				/* Packet length up to the number of data at program write command		*/ /* <S150> */
/*--------------------------------------------------------------------------------------------------*/
/*		Subfunction response size																*/
/*--------------------------------------------------------------------------------------------------*/
#define	MBUS_DL_ID_RD_RSPSIZE		22			/* IDReading information						<S077>		*/
#define	MBUS_DL_SWVER_RD_RSPSIZE	14			/* Version read					<S138>			*/
#define	MBUS_DL_WAY_RD_RSPSIZE		10			/* Program write condition acquisition							*/
#define	MBUS_DL_START_RSPSIZE		8			/* Download start command								*/
#define	MBUS_DL_PRGDEL_RSPSIZE		14			/* Block erase command									*/
#define	MBUS_DL_PRGWR_RSPSIZE		16			/* Program write command								*/
#define	MBUS_DL_DELSND_RSPSIZE		12			/* Block erase information command(Erase information transfer only.Do not erase)		*/
#define	MBUS_DL_PRGSND_RSPSIZE		14			/* Program transmission command(Program transmission only.Do not write)		*/
#define	MBUS_DL_PRGDELA_RSPSIZE		8			/* Block bulk erase command								*/
#define	MBUS_DL_PRGWRA_RSPSIZE		8			/* Program batch write command							*/
#define	MBUS_DL_END_RSPSIZE			8			/* Download end command								*/
//#define	MBUS_DL_PRGRD_RSPSIZE		x			/* Program read command								*/
#define	MBUS_DL_REBOOT_RSPSIZE		6			/* Program restart command								*/
//<S041>#define	MBUS_DL_SIFCHG_RSPSIZE		10			/* Serial IF speed change								*/
#define	MBUS_DL_SIFCHG_RSPSIZE		12			/* Serial IF speed change								*/
#define	MBUS_DL_HIS_RD_RSPSIZE		8			/* Read download history							*/
#define	MBUS_DL_HIS_WR_RSPSIZE		6			/* Write download history							*/
#define	MBUS_DL_CHKSUM_RSPSIZE		10			/* Checksum result acquisition								*/ /* <S146> */
#define MBUS_DL_ERROR_RSPSIZE		5			/* Error											*/
#define MBUS_DL_MIN_RSPSIZE			5			/* Minimum message size								*/
/*--------------------------------------------------------------------------------------------------*/
/*		MEMOBUS Firm download	Response													*/
/*--------------------------------------------------------------------------------------------------*/
#define	MBUS_OK		0
#define	MBUS_NG		-1
#define	MBUS_BUSY	1


/* <S1A3> >>>>> */
/*--------------------------------------------------------------------------------------------------*/
/*		Definition for sum check																			*/
/*--------------------------------------------------------------------------------------------------*/
#define	CHKSUM_OK		0
#define	CHKSUM_NG		-1
#define	CHKSUM_BUSY		1

#define CHKSUM_MAIN_PHASE_MASK	0xFF00							/* Mask pattern for main phase	*/
#define CHKSUM_MAIN_PHASE_SHFT	8								/* Right shift number for main phase		*/
#define CHKSUM_MAIN_PHASE_ADD	0x0100							/* Additional value for main phase			*/
#define CHKSUM_SUB_PHASE_MASK	0x00FF							/* Mask pattern for subphase		*/
#define CHKSUM_PHASE_END		0xFFFF							/* Checksum end pattern		*/

#define CHKSUM_BLOCK_SIZE		0x400							/* 1Number of bytes processed in scan		*/
																/* ※ 1Bit onlyON(0x400, 0x200, 0x100)	*/
#define CHKSUM_BLOCK_MASK		(CHKSUM_BLOCK_SIZE - 1)			/* 1Mask pattern for block end determination*/
/* <<<<< <S1A3> */

/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Download definition structure																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct{
	UCHAR	MpuNo;									/* MPU number									*/
	UCHAR	FlushNum;								/* Number of flash memories							*/
	USHORT	DlSts1;									/* Download status 1						*/
														/*   Bit0: Download compatible 0/1-Not supported/Correspondence		*/
														/*   Bit1: ModelIDCode presence/existence	0/1-None/Yes			*/
														/*   Bit2-15: Reserved area							*/
	USHORT	DlSts2;									/* Download status 2						*/
														/*   Bit0 :Flash #0	Download 0/1-Enabled/Not possible	*/
														/*   Bit1 :Flash #1	Download 0/1-Enabled/Not possible	*/
														/*   Bit2 :Flash #2	Download 0/1-Enabled/Not possible	*/
														/*   Bit3 :Flash #3	Download 0/1-Enabled/Not possible	*/
														/*   Bit4 :Flash #4	Download 0/1-Enabled/Not possible	*/
														/*   Bit5 :Flash #5	Download 0/1-Enabled/Not possible	*/
														/*   Bit6 :Flash #6    Download 0/1-Enabled/Not possible	*/
														/*   Bit7 :Flash #7	Download 0/1-Enabled/Not possible	*/
														/*   Bit8 :Flash #8	Download 0/1-Enabled/Not possible	*/
														/*   Bit9 :Flash #9	Download 0/1-Enabled/Not possible	*/
														/*   Bit10:Flash #10	Download 0/1-Enabled/Not possible	*/
														/*   Bit11:Flash #11	Download 0/1-Enabled/Not possible	*/
														/*   Bit12:Flash #12	Download 0/1-Enabled/Not possible	*/
														/*   Bit13:Flash #13	Download 0/1-Enabled/Not possible	*/
														/*   Bit14:Flash #14	Download 0/1-Enabled/Not possible	*/
														/*   Bit15:Flash #15	Download 0/1-Enabled/Not possible	*/
	USHORT	DlSts3;									/* Download status 3						*/
														/*   Bit0 :Flash #0	MultifactureID 0/1-None/Yes		*/
														/*   Bit1 :Flash #1	MultifactureID 0/1-None/Yes		*/
														/*   Bit2 :Flash #2	MultifactureID 0/1-None/Yes		*/
														/*   Bit3 :Flash #3	MultifactureID 0/1-None/Yes		*/
														/*   Bit4 :Flash #4	MultifactureID 0/1-None/Yes		*/
														/*   Bit5 :Flash #5	MultifactureID 0/1-None/Yes		*/
														/*   Bit6 :Flash #6	MultifactureID 0/1-None/Yes		*/
														/*   Bit7 :Flash #7	MultifactureID 0/1-None/Yes		*/
														/*   Bit8 :Flash #8	MultifactureID 0/1-None/Yes		*/
														/*   Bit9 :Flash #9	MultifactureID 0/1-None/Yes		*/
														/*   Bit10:Flash #10	MultifactureID 0/1-None/Yes		*/
														/*   Bit11:Flash #11	MultifactureID 0/1-None/Yes		*/
														/*   Bit12:Flash #12	MultifactureID 0/1-None/Yes		*/
														/*   Bit13:Flash #13	MultifactureID 0/1-None/Yes		*/
														/*   Bit14:Flash #14	MultifactureID 0/1-None/Yes		*/
														/*   Bit15:Flash #15	MultifactureID 0/1-None/Yes		*/
	USHORT	DlSts4;									/* Download status 4						*/
														/*   Bit0 :Flash #0	DeviceID  0/1-None/Yes		*/
														/*   Bit1 :Flash #1	DeviceID  0/1-None/Yes		*/
														/*   Bit2 :Flash #2	DeviceID  0/1-None/Yes		*/
														/*   Bit3 :Flash #3	DeviceID  0/1-None/Yes		*/
														/*   Bit4 :Flash #4	DeviceID  0/1-None/Yes		*/
														/*   Bit5 :Flash #5	DeviceID  0/1-None/Yes		*/
														/*   Bit6 :Flash #6	DeviceID  0/1-None/Yes		*/
														/*   Bit7 :Flash #7	DeviceID  0/1-None/Yes		*/
														/*   Bit8 :Flash #8	DeviceID  0/1-None/Yes		*/
														/*   Bit9 :Flash #9	DeviceID  0/1-None/Yes		*/
														/*   Bit10:Flash #10	DeviceID  0/1-None/Yes		*/
														/*   Bit11:Flash #11	DeviceID  0/1-None/Yes		*/
														/*   Bit12:Flash #12	DeviceID  0/1-None/Yes		*/
														/*   Bit13:Flash #13	DeviceID  0/1-None/Yes		*/
														/*   Bit14:Flash #14	DeviceID  0/1-None/Yes		*/
														/*   Bit15:Flash #15	DeviceID  0/1-None/Yes		*/
	USHORT	DlWay;									/* System download method selection					*/
														/*   Bit0-3 : Download method					*/
														/*   Bit4-15: Reserved area							*/
	USHORT	Prg1WrSize;								/* Specification of number of write bytes per message		*/
	USHORT	DlStatus;								/* Download status							*/
														/*   Bit0 : Download mode 0/1-OFF/ON		*/
														/*   Bit1 :FlashErasing 	0/1-OFF/ON			*/
														/*   Bit2 :FlashWriting in progress	0/1-OFF/ON			*/
														/*   Bit3 :FlashReading	0/1-OFF/ON			*/
														/*   Bit4-5: Processing status(00/01/10/11)				*/
														/*   			None/busy/Response permission/Reservation			*/
	ULONG	NowBps;										/*	System download current BPS value	<S041> */
	ULONG	BpsChkTime;									/* Bps Timeout Timer <S041>	*/
/* <S146> >>>>> */
	/* Message -> Sum check */
	BOOL	MicroSumCheckF;							/* Micro code section sum check necessity flag			*/
	BOOL	MirSumCheckF;							/* Mirror section sum check necessity flag					*/
	BOOL	RtmSumCheckF;							/* Runtime part sum check necessity flag				*/
	BOOL	BootSumCheckF;							/* Boot part sum check necessity flag					*/
	BOOL	CheckSumRequestF;						/* Checksum processing start request flag					*/
	BOOL	CheckSumCancelF;						/* Checksum processing cancellation request flag				*/
	/* Sum check -> Message */
	BOOL	CheckSumCalcF;							/* Checksum processing execution flag						*/
	BOOL	CheckSumEndF;							/* Checksum processing end flag						*/
	BOOL	CheckSumResult;							/* Checksum processing result	(TRUE:OK, FALSE:NG)			*/
	/* for Sum check */
	USHORT	CheckSumPhase;							/* Checksum phase								*/ /*<S182>*/
/* <S1A3> >>>>> */
	USHORT	CheckSumVal;							/* For sum value calculation										*/
	UCHAR	*CheckSumMem;							/* Memory pointer									*/
	LONG	CheckSumIdx;							/* Processing position											*/
/* <<<<< <S1A3> */
/* <<<<< <S146> */
} SYSDL;

extern SYSDL	SysDl;

/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Macro definition																					*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#define SYSDL_DLMODE_RESET()	(SysDl.DlStatus = 0x0000)			/* Download modeBitOperation	*/
#define SYSDL_DLMODE_ON()		(SysDl.DlStatus |= 0x0001)			/* Download modeBitOperation	*/
#define SYSDL_DLMODE_OFF()		(SysDl.DlStatus &= 0xFFFE)
#define SYSDL_CHK_DLMODE_ON()	((SysDl.DlStatus & 0x0001) == 1)
#define SYSDL_CHK_DLMODE_OFF()	((SysDl.DlStatus & 0x0001) != 1)
/*--------------------------------------------------------------------------------------------------*/
#define SYSDL_ERASE_ON()		(SysDl.DlStatus |= 0x0002)			/* Flash Erase BitOperation			*/
#define SYSDL_ERASE_OFF()		(SysDl.DlStatus &= 0xFFFD)
#define SYSDL_CHK_ERASE_ON()	((SysDl.DlStatus & 0x0002) == 1)
#define SYSDL_CHK_ERASE_OFF()	((SysDl.DlStatus & 0x0002) != 1)
/*--------------------------------------------------------------------------------------------------*/
#define SYSDL_WRITE_ON()		(SysDl.DlStatus |= 0x0004)			/* Flash Write BitOperation			*/
#define SYSDL_WRITE_OFF()		(SysDl.DlStatus &= 0xFFFB)
#define SYSDL_CHK_WRITE_ON()	((SysDl.DlStatus & 0x0004) == 1)
#define SYSDL_CHK_WRITE_OFF()	((SysDl.DlStatus & 0x0004) != 1)
/*--------------------------------------------------------------------------------------------------*/
#define SYSDL_READ_ON()			(SysDl.DlStatus |= 0x0008)			/* Flash Read BitOperation			*/
#define SYSDL_READ_OFF()		(SysDl.DlStatus &= 0xFFF7)
#define SYSDL_CHK_READ_ON()		((SysDl.DlStatus & 0x0008) == 1)
#define SYSDL_CHK_READ_OFF()	((SysDl.DlStatus & 0x0008) != 1)
/*--------------------------------------------------------------------------------------------------*/
#define SYSDL_CHK_FLASH_BUSY()	((SysDl.DlStatus & 0x000E) != 0)	/* FlashConfirm during operation TRUEin operation	*/
/*--------------------------------------------------------------------------------------------------*/


/* Temporary support (finally macroorAPI? ) */
#ifdef _TEST_BOARD_		/* for MICROSSP */
#define	SYSDL_CHK_BASE_BLOCK( )			(((*(VUSHORT*)(0x08000032)) & 0x0400) != 0)
#else					/* for SS-Pro7 */
#define	SYSDL_CHK_BASE_BLOCK( )			(((*(VUSHORT*)(0xC0000064)) & 0x0400) != 0)
#endif


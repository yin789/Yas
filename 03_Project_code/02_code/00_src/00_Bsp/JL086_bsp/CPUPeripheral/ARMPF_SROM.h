/************************************************************************************************************/
/*																											*/
/*																											*/
/*		ARMPF_SROM.h : MercurySoC peripheral SROM MEMCheader												*/
/*																											*/
/*																											*/
/************************************************************************************************************/
/*																											*/
/*																											*/
/*		SerialFlashmemory controller(SROM MEMC)Related header files											*/
/*																											*/
/*																											*/
/************** Copyright (C) Yaskawa Electric Corporation **************************************************/
/*																											*/
/*		Rev.1.00 : 2013.07.29  Y.Tanaka		First edition															*/
/*																											*/
/************************************************************************************************************/
#include	"ARMPF.h"


#ifndef	_ARMPF_SROM_MEMC_H_
#define	_ARMPF_SROM_MEMC_H_

//#include	"arm.h"

#define	ARMPF_SFMEMC_BASE	0xE8020000

/*----------------------------------------------------------------------------------------------------------*/
/*																											*/
/*		SROM MEMC Transfer mode control register(SFMSMD)Setting value														*/
/*																											*/
/*----------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------*/
/*		ReadMode selection																						*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_SLOWREAD		0									/* standardRead									*/
#define	SFMEMC_FASTREAD		1									/* Fast Read								*/
#define	SFMEMC_FASTREAD2O	2									/* Fast Read Dual Output					*/
#define	SFMEMC_FASTREAD2IO	3									/* Fast Read Dual I/O						*/
#define	SFMSMD_SLOWREAD		(SFMEMC_SLOWREAD    << 0 )			/* standardRead									*/
#define	SFMSMD_FASTREAD		(SFMEMC_FASTREAD    << 0 )			/* Fast Read								*/
#define	SFMSMD_FASTREAD2O	(SFMEMC_FASTREAD2O  << 0 )			/* Fast Read Dual Output					*/
#define	SFMSMD_FASTREAD2IO	(SFMEMC_FASTREAD2IO << 0 )			/* Fast Read Dual I/O						*/
/*----------------------------------------------------------------------------------------------------------*/
/*		SPIAfter bus accessSROM_CSZExtension function selection																*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_SPIEXT0		0
#define	SFMEMC_SPIEXT1		1
#define	SFMEMC_SPIEXT2		2
#define	SFMEMC_SPIEXT3		3
#define	SFMSMD_SPIEXT0		(SFMEMC_SPIEXT0 << 4 )
#define	SFMSMD_SPIEXT1		(SFMEMC_SPIEXT1 << 4 )
#define	SFMSMD_SPIEXT2		(SFMEMC_SPIEXT2 << 4 )
#define	SFMSMD_SPIEXT3		(SFMEMC_SPIEXT3 << 4 )
/*----------------------------------------------------------------------------------------------------------*/
/*		Prefetch function selection																				*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_NO_PREFETCH	0
#define	SFMEMC_PREFETCH		1
#define	SFMSMD_NO_PREFETCH	(SFMEMC_NO_PREFETCH << 6 )
#define	SFMSMD_PREFETCH		(SFMEMC_PREFETCH    << 6 )
/*----------------------------------------------------------------------------------------------------------*/
/*		Prefetch interruption function selection at locations other than byte boundaries															*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_ABORT_DISABLE 0
#define	SFMEMC_ABORT_ENABLE  1
#define	SFMSMD_ABORT_DISABLE	(SFMEMC_ABORT_DISABLE << 7 )
#define	SFMSMD_ABORT_ENABLE		(SFMEMC_ABORT_ENABLE  << 7 )
/*----------------------------------------------------------------------------------------------------------*/
/*		SPI Mode selection																						*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_SPIMODE0		0
#define	SFMEMC_SPIMODE3		1
#define	SFMSMD_SPIMODE0		(SFMEMC_SPIMODE0 << 8 )
#define	SFMSMD_SPIMODE3		(SFMEMC_SPIMODE3 << 8 )
/*----------------------------------------------------------------------------------------------------------*/
/*		For serial interface I/O Extending buffer output permission												*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_OEEXT0		0
#define	SFMEMC_OEEXT1		1
#define	SFMSMD_OEEXT0		(SFMEMC_OEEXT0 << 9 )
#define	SFMSMD_OEEXT1		(SFMEMC_OEEXT0 << 9 )
/*----------------------------------------------------------------------------------------------------------*/
/*		Hold time adjustment for serial transmission																	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_MOSUPWAIT0	0
#define	SFMEMC_MOSUPWAIT1	1
#define	SFMSMD_MOSUPWAIT0	(SFMEMC_MOSUPWAIT0 << 10 )
#define	SFMSMD_MOSUPWAIT1	(SFMEMC_MOSUPWAIT1 << 10 )
/*----------------------------------------------------------------------------------------------------------*/
/*		Setup time adjustment for serial transmission																*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_MOHLDWAIT0	0
#define	SFMEMC_MOHLDWAIT1	1
#define	SFMSMD_MOHLDWAIT0	(SFMEMC_MOHLDWAIT0 << 11 )
#define	SFMSMD_MOHLDWAIT1	(SFMEMC_MOHLDWAIT1 << 11 )


/*----------------------------------------------------------------------------------------------------------*/
/*																											*/
/*		Chip selection control register(SFMSSC)Setting value																*/
/*																											*/
/*----------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------*/
/*		SROM_CSZ Signal release timing selection																	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_SSB_H_DELAY0		0
#define	SFMEMC_SSB_H_DELAY1		1
#define	SFMSSC_SSB_H_DELAY0		(SFMEMC_SSB_H_DELAY0 << 4 )
#define	SFMSSC_SSB_H_DELAY1		(SFMEMC_SSB_H_DELAY1 << 4 )

/*----------------------------------------------------------------------------------------------------------*/
/*		SROM_CSZ Signal output timing selection																	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_SSB_L_DELAY0		0
#define	SFMEMC_SSB_L_DELAY1		1
#define	SFMSSC_SSB_L_DELAY0		(SFMEMC_SSB_L_DELAY0 << 5 )
#define	SFMSSC_SSB_L_DELAY1		(SFMEMC_SSB_L_DELAY1 << 5 )


/*----------------------------------------------------------------------------------------------------------*/
/*																											*/
/*		clock control register(SFMSKC)Setting value																	*/
/*																											*/
/*----------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------*/
/*		SROM_CLK of the signal Duty Selection of ratio correction function																*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_DUTYXX		0
#define	SFMEMC_DUTY50		1
#define	SFMSSC_DUTYXX		(SFMEMC_DUTYXX << 5 )
#define	SFMSKC_DUTY50		(SFMEMC_DUTY50 << 5 )


/*----------------------------------------------------------------------------------------------------------*/
/*																											*/
/*		SROM MEMC Transfer mode control register(SFMSMD)Setting value														*/
/*																											*/
/*----------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------*/
/*		SROM MEMC Transfer mode setting value																			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_MISO_DELAY0		0
#define	SFMEMC_MISO_DELAY1		1
#define	SFMDBG_MISO_DELAY0		(SFMEMC_MISO_DELAY0 << 0 )
#define	SFMDBG_MISO_DELAY1		(SFMEMC_MISO_DELAY1 << 0 )


/*----------------------------------------------------------------------------------------------------------*/
/*																											*/
/*		SROM MEMC Transfer mode control register(SFMSMD)Setting value														*/
/*																											*/
/*----------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------*/
/*		SROM MEMC Transfer mode setting value																			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_DCOM_DISABLE		0
#define	SFMEMC_DCOM_ENABLE		1
#define	SFMCMD_DCOM_DISABLE		(SFMEMC_DCOM_DISABLE << 0 )
#define	SFMCMD_DCOM_ENABLE		(SFMEMC_DCOM_ENABLE  << 0 )

/*----------------------------------------------------------------------------------------------------------*/
/*		SROM MEMC Transfer mode setting value																			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SFMEMC_INT_CLEAR		0
#define	SFMEMC_INT		 		1
#define	SFMCST_INT_CLEAR		(SFMEMC_INT_CLEAR << 7 )
#define	SFMCST_INT				(SFMEMC_INT << 7 )

/*----------------------------------------------------------------------------------------------------------*/
/*		SROM MEMC register																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	ARMPF_SFMEMC_SFMSMD		(ARMPF_SFMEMC_BASE + 0x000)		/* Transfer mode control register					*/
#define	ARMPF_SFMEMC_SFMSSC		(ARMPF_SFMEMC_BASE + 0x004)		/* Chip selection control register 					*/
#define	ARMPF_SFMEMC_SFMSKC		(ARMPF_SFMEMC_BASE + 0x008)		/* clock control register						*/
#define	ARMPF_SFMEMC_SFMSST		(ARMPF_SFMEMC_BASE + 0x00C)		/* status register						*/
#define	ARMPF_SFMEMC_SFMCOM		(ARMPF_SFMEMC_BASE + 0x010)		/* communication port								*/
#define	ARMPF_SFMEMC_SFMCMD		(ARMPF_SFMEMC_BASE + 0x014)		/* Communication mode control register					*/
#define	ARMPF_SFMEMC_SFMCST		(ARMPF_SFMEMC_BASE + 0x018)		/* Communication status register					*/
/*----------------------------------------------------------------------------------------------------------*/


#endif
/*-------end of file----------------------------------------------------------------------------------------*/

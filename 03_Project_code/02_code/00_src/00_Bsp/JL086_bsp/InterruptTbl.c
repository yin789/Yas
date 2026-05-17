/************************************************************************************************************/
/*																											*/
/*																											*/
/*		InterruptTbl.c : Mercury Interrupt related table definition													*/
/*																											*/
/*																											*/
/************************************************************************************************************/
/*																											*/
/*																											*/
/*		Interrupt configuration source file																			*/
/*																											*/
/*																											*/
/************** Copyright (C) Yaskawa Electric Corporation **************************************************/
/*																											*/
/*		Rev.1.00 : 2013.07.30  Y.Tanaka		First edition															*/
/*																											*/
/************************************************************************************************************/
#include	"BaseDef.h"
#include	"ARMPF_VIC.h"
#include	"Interrupt.h"
#include	"I2cDriver.h"


/************************************************************************************************************/
/*																											*/
/*		external functionexterndeclaration																					*/
/*																											*/
/************************************************************************************************************/
#ifndef _MERCURY_PLATFORM_ /* Supports encrypted formats <S146> */
extern	void	IntIgnore(void);
extern	void	IRQHandler0(void);
extern	void	IRQHandler1(void);
extern	void	ScanAHandler( void );
extern	void	ScanBHandler( void );
extern	void	ScanCHandler( void );
extern	void	ScanAIntHandler( void );
extern	void	ScanBIntHandler( void );
extern	void	ScanCIntHandler( void );
extern	void	UsbFuncIntIntu2f( void );
extern	void	UsbFuncIntIntu2fepc( void );
extern	void	IntCpuI2CBusIF( void );					/* @@ I2C Interrupt Handler */
#else /* _MERCURY_PLATFORM_ */ /* Supports encrypted formats <S146> */
#define	IntIgnore				0
#define	IRQHandler0				0
#define	IRQHandler1				0
#define	ScanAHandler			0
#define	ScanBHandler			0
#define	ScanCHandler			0
#define	ScanAIntHandler			0
#define	ScanBIntHandler			0
#define	ScanCIntHandler			0
#define	UsbFuncIntIntu2f		0
#define	UsbFuncIntIntu2fepc		0
#define	IntCpuI2CBusIF			0
#endif /* _MERCURY_PLATFORM_ */	/* Supports encrypted formats <S146> */
/************************************************************************************************************/
/*																											*/
/*		table data definition																					*/
/*																											*/
/************************************************************************************************************/
#pragma arm section rodata = "INIT_VECT_DATA"
/*----------------------------------------------------------------------------------------------------------*/
/*		Initialization table																					*/
/*----------------------------------------------------------------------------------------------------------*/
const INTVECINITTBL InitIntVecTbl[64] = {
/*	PLS					EDGC				RegHandler							*/
	ARMPF_VIC_EDGE,		ARMPF_VIC_FALL,		ScanAHandler,		/* 0:INTUDL0 - ScanA */
	ARMPF_VIC_EDGE,		ARMPF_VIC_FALL,		ScanBHandler,		/* 1:INTUDL1 - ScanB */
	ARMPF_VIC_EDGE,		ARMPF_VIC_FALL,		ScanCHandler,		/* 2:INTUDL2 - ScanC */
	ARMPF_VIC_EDGE,		ARMPF_VIC_FALL,		IRQHandler0,		/* 3:INTUDL3 - ScanN */
	ARMPF_VIC_EDGE,		ARMPF_VIC_FALL,		IRQHandler0,		/* 4:INTUDL4 - INT1 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_FALL,		IRQHandler0,		/* 5:INTUDL5 - INT2 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_FALL,		IRQHandler0,		/* 6:INTUDL6 - M3_INT1 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_FALL,		IRQHandler0,		/* 7:INTUDL7 - M3_INT2 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 8:reserved */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 9:reserved */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 10:INTPZ0 external terminal */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 11:INTPZ1 external terminal */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 12:INTPZ2 external terminal */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 13:INTPZ3 external terminal */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 14:INTTAUJ0 timer arrayTAUJ0 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 15:INTTAUJ1 timer arrayTAUJ1 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 16:INTTAUJ2 timer arrayTAUJ2 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 17:INTTAUJ3 timer arrayTAUJ3 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 18:INTTM0 APB-SStimer0 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 19:INTTM1 APB-SStimer1 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 20:INTTM2 APB-SStimer2 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 21:INTTM3 APB-SStimer3 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 22:INTDMAERR DMA-SS TYPE-AXI */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 23:INTDMA0 DMA-SS TYPE-AXI */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 24:INTDMA1 DMA-SS TYPE-AXI */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 25:INTDMA2 DMA-SS TYPE-AXI */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 26:INTDMA3 DMA-SS TYPE-AXI */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 27:CSI_INT APB-SS CSI0 */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 28:UART_INT APB-SS UART */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 29:IIC_INT APB-SS I2C */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 30:BUS_ERRINT0 BUS-SS */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler0,		/* 31:BUS_ERRINT1 BUS-SS */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 32:INTETH GbEther-SS */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 33:INTETH APB-SS ADC */
	ARMPF_VIC_LVL,		ARMPF_VIC_HIGH,		IRQHandler1,		/* 34:INTU2H USBhostBRIDGE */
	ARMPF_VIC_LVL,		ARMPF_VIC_HIGH,		IRQHandler1,		/* 35:INTU2HOHCI USBhostOHCI */
	ARMPF_VIC_LVL,		ARMPF_VIC_HIGH,		IRQHandler1,		/* 36:INTU2HEHCI USBhostEHCI */
	ARMPF_VIC_LVL,		ARMPF_VIC_HIGH,		IRQHandler1,		/* 37:INTU2HPME USBhostPME */
	ARMPF_VIC_LVL,		ARMPF_VIC_HIGH,		IRQHandler1,		/* 38:INTU2F USBfunctionBRIDGE */
	ARMPF_VIC_LVL,		ARMPF_VIC_HIGH,		IRQHandler1,		/* 39:INTU2FEPC USBfunctionEPC */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 40:PCIe_INTA_RC PCIe INTA */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 41:PCIe_INTB_RC PCIe INTB */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 42:PCIe_INTC_RC PCIe INTC */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 43:PCIe_INTD_RC PCIe INTD */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 44:PCIe_INTMSI_RC PCIe MSI */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 45:PCIe_MSG_INT PCIemessage */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 46:PCIe_PCIE_ERR_INT PCIe PCIe Error */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 47:PCIe_AXI_ERR_INT PCIe AXI Error */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 48:PCIe_INTALL PCIe PCIeInterrupt logical OR */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 49:SFMEMC_INT MEM-SS TYPE-SROM */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 50:BUS_ERRINT2 BUS-SS(AXIIC2 andAPBBUS betweenAXIAHB) */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 51:BUS_ERRINT3 BUS-SS(AHBICandAXIIC1 betweenAHBAXI) */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 52:BUS_ERRINT4 BUS-SS(CoreSight andAXIIC1 betweenAHBAXI) */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 53:INTETH_AXI GbEther-SS(AXI interrupt) */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 54:CPUSS_FPIXC CPU-SS FPU inexact exception */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 55:CPUSS_FPOFC CPU-SS FPU overflow exception */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 56:CPUSS_FPUFC CPU-SS FPU underflow exception */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 57:CPUSS_FPIOC CPU-SS FPU invalid operation exception */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 58:CPUSS_FPDZC CPU-SS FPU Division by zero exception */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 59:CPUSS_FPIDC CPU-SS FPU input subnormal exception */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 60:CPUSS_TRIGINT CPU-SS CTI Cross-trigger interrupt output from */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 61:PCISS_GPO[0] PCIe-SS General purpose Output register output */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 62:?? */
	ARMPF_VIC_EDGE,		ARMPF_VIC_RISE,		IRQHandler1,		/* 63:?? */
};


#pragma arm section rodata = "VECT_DATA"
/*----------------------------------------------------------------------------------------------------------*/
/*		Interrupt execution time table																				*/
/*----------------------------------------------------------------------------------------------------------*/
const INTVECTBL IntVecTbl[64] = {
/*	Priority				IntHandler						*/
	ARMPF_VIC_PRIORYTY1,	ScanAIntHandler,	/* 0:INTUDL0 - ScanA */
	ARMPF_VIC_PRIORYTY2,	ScanBIntHandler,	/* 1:INTUDL1 - ScanB */
	ARMPF_VIC_PRIORYTY3,	ScanCIntHandler,	/* 2:INTUDL2 - ScanC */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 3:INTUDL3 - ScanN */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 4:INTUDL4 - INT1 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 5:INTUDL5 - INT2 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 6:INTUDL6 - M3_INT0 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 7:INTUDL7 - M3_INT1 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 8:reserved */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 9:reserved */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 10:INTPZ0 external terminal */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 11:INTPZ1 external terminal */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 12:INTPZ2 external terminal */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 13:INTPZ3 external terminal */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 14:INTTAUJ0 timer arrayTAUJ0 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 15:INTTAUJ1 timer arrayTAUJ1 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 16:INTTAUJ2 timer arrayTAUJ2 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 17:INTTAUJ3 timer arrayTAUJ3 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 18:INTTM0 APB-SStimer0 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 19:INTTM1 APB-SStimer1 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 20:INTTM2 APB-SStimer2 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 21:INTTM3 APB-SStimer3 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 22:INTDMAERR DMA-SS TYPE-AXI */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 23:INTDMA0 DMA-SS TYPE-AXI */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 24:INTDMA1 DMA-SS TYPE-AXI */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 25:INTDMA2 DMA-SS TYPE-AXI */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 26:INTDMA3 DMA-SS TYPE-AXI */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 27:CSI_INT APB-SS CSI0 */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 28:UART_INT APB-SS UART */
	ARMPF_VIC_PRIORYTY3,	___CPU_I2C_HANDLER___( IntCpuI2CBusIF ),		/* 29:IIC_INT APB-SS I2C */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 30:BUS_ERRINT0 BUS-SS */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 31:BUS_ERRINT1 BUS-SS */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 32:INTETH GbEther-SS */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 33:INTETH APB-SS ADC */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 34:INTU2H USBhostBRIDGE */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 35:INTU2HOHCI USBhostOHCI */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 36:INTU2HEHCI USBhostEHCI */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 37:INTU2HPME USBhostPME */
#if 0	/* <S150> USBdon't use interrupts(Polling) */
	ARMPF_VIC_PRIORYTY7,	UsbFuncIntIntu2f,	/* 38:INTU2F USBfunctionBRIDGE */
	ARMPF_VIC_PRIORYTY7,	UsbFuncIntIntu2fepc,/* 39:INTU2FEPC USBfunctionEPC */
#else
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 38:INTU2F USBfunctionBRIDGE */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 39:INTU2FEPC USBfunctionEPC */
#endif
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 40:PCIe_INTA_RC PCIe INTA */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 41:PCIe_INTB_RC PCIe INTB */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 42:PCIe_INTC_RC PCIe INTC */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 43:PCIe_INTD_RC PCIe INTD */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 44:PCIe_INTMSI_RC PCIe MSI */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 45:PCIe_MSG_INT PCIemessage */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 46:PCIe_PCIE_ERR_INT PCIe PCIe Error */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 47:PCIe_AXI_ERR_INT PCIe AXI Error */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 48:PCIe_INTALL PCIe PCIeInterrupt logical OR */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 49:SFMEMC_INT MEM-SS TYPE-SROM */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 50:BUS_ERRINT2 BUS-SS(AXIIC2 andAPBBUS betweenAXIAHB) */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 51:BUS_ERRINT3 BUS-SS(AHBICandAXIIC1 betweenAHBAXI) */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 52:BUS_ERRINT4 BUS-SS(CoreSight andAXIIC1 betweenAHBAXI) */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 53:INTETH_AXI GbEther-SS(AXI interrupt) */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 54:CPUSS_FPIXC CPU-SS FPU inexact exception */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 55:CPUSS_FPOFC CPU-SS FPU overflow exception */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 56:CPUSS_FPUFC CPU-SS FPU underflow exception */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 57:CPUSS_FPIOC CPU-SS FPU invalid operation exception */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 58:CPUSS_FPDZC CPU-SS FPU Division by zero exception */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 59:CPUSS_FPIDC CPU-SS FPU input subnormal exception */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 60:CPUSS_TRIGINT CPU-SS CTI Cross-trigger interrupt output from */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 61:PCISS_GPO[0] PCIe-SS General purpose Output register output */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 62:?? */
	ARMPF_VIC_PRIORYTY15,	IntIgnore,			/* 63:?? */
};

/*-------end of file----------------------------------------------------------------------------------------*/

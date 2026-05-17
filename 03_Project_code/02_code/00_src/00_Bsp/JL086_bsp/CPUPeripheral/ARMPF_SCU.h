/************************************************************************************************************/
/*																											*/
/*																											*/
/*		ARMPF_SCU.h : MercurySoC peripheral SCUheader														*/
/*																											*/
/*																											*/
/************************************************************************************************************/
/*																											*/
/*																											*/
/*		system control unit(SCU)Related header files													*/
/*																											*/
/*																											*/
/************** Copyright (C) Yaskawa Electric Corporation **************************************************/
/*																											*/
/*		Rev.1.00 : 2013.07.30  Y.Tanaka		First edition															*/
/*																											*/
/************************************************************************************************************/
#include	"ARMPF.h"


#ifndef	_ARMPF_SCU_H_
#define	_ARMPF_SCU_H_

#define	ARMPF_SCU_BASE	0xEFF86000

/*----------------------------------------------------------------------------------------------------------*/
/*																											*/
/*		SCURegister setting value																					*/
/*																											*/
/*----------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------*/
/*		SCU register																						*/
/*----------------------------------------------------------------------------------------------------------*/
#define	ARMPF_SCU_DEVICE_ID		(ARMPF_SCU_BASE + 0x000)		/* deviceIDcode register					*/
#define	ARMPF_SCU_SOFT_RST		(ARMPF_SCU_BASE + 0x004)		/* soft reset register					*/
#define	ARMPF_SCU_CPU_HALT		(ARMPF_SCU_BASE + 0x008)		/* PCIexboot control register					*/
#define	ARMPF_SCU_CLK_EN		(ARMPF_SCU_BASE + 0x00C)		/* Clock control register					*/
#define	ARMPF_SCU_CLK_SEL		(ARMPF_SCU_BASE + 0x010)		/* Clock switching register					*/
#define	ARMPF_SCU_MODE			(ARMPF_SCU_BASE + 0x014)		/* Mode confirmation register						*/
#define	ARMPF_SCU_REMAP			(ARMPF_SCU_BASE + 0x018)		/* Remapping function control register					*/
#define	ARMPF_SCU_PCIE_INT		(ARMPF_SCU_BASE + 0x01C)		/* PCIexEndpoint interrupt register		*/
#define	ARMPF_SCU_PCIE_INTMSK	(ARMPF_SCU_BASE + 0x020)		/* PCIexEndpoint interrupt mask register	*/
#define	ARMPF_SCU_PCIE_INTSEL	(ARMPF_SCU_BASE + 0x024)		/* PCIexinterruptUDL-IFselection register			*/
#define	ARMPF_SCU_WDT_MODE		(ARMPF_SCU_BASE + 0x028)		/* UDL-WDTReset control register				*/
#define	ARMPF_SCU_5TBUF_EN		(ARMPF_SCU_BASE + 0x02C)		/* 5VTolerant buffer standby control register	*/
#define	ARMPF_SCU_RESERVE0		(ARMPF_SCU_BASE + 0x030)		/* reserved									*/
#define	ARMPF_SCU_NMI_SET		(ARMPF_SCU_BASE + 0x034)		/* NMIconfiguration register							*/
#define	ARMPF_SCU_INTSYS1_EN	(ARMPF_SCU_BASE + 0x038)		/* Peripheral circuit interrupt input synchronization/Asynchronous settings1register	*/
#define	ARMPF_SCU_INTSYS2_EN	(ARMPF_SCU_BASE + 0x03C)		/* Peripheral circuit interrupt input synchronization/Asynchronous settings2register	*/
#define	ARMPF_SCU_VICIFSYN_EN	(ARMPF_SCU_BASE + 0x040)		/* CPU-VICsynchronization between/Asynchronous configuration register		*/
#define	ARMPF_SCU_PCIE_CLKSEL	(ARMPF_SCU_BASE + 0x044)		/* PCIexClock selection register				*/
#define	ARMPF_SCU_RESET_STATUS	(ARMPF_SCU_BASE + 0x048)		/* Reset status register				*/
#define	ARMPF_SCU_UDLCNT		(ARMPF_SCU_BASE + 0x04C)		/* M3-UDLPRGSELconfiguration register					*/
#define	ARMPF_SCU_RESERVE1		(ARMPF_SCU_BASE + 0x050)		/* reserved									*/
#define	ARMPF_SCU_CPUSS_TRIGINT	(ARMPF_SCU_BASE + 0x054)		/* CTICross-trigger interrupt factor register from module	*/
#define	ARMPF_SCU_GBETHER_CONT	(ARMPF_SCU_BASE + 0x058)		/* GbEtherMode setting register				*/
#define	ARMPF_SCU_UDLINT_CONT	(ARMPF_SCU_BASE + 0x05C)		/* PCIexagainstUDLInterrupt selection register		*/
#define	ARMPF_SCU_UDLINT_STATUS	(ARMPF_SCU_BASE + 0x060)		/* UDLInterrupt factor register					*/

#define	ARMPF_SCU_APBREG15		(0xEFFF027C)					/* I2COperation control register(ES3Effective from then on)		<S152>			*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_DVICE_ID																						*/
/*----------------------------------------------------------------------------------------------------------*/
#define	JL086ID				0x086A								/* Device ID								*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_SOFT_RST																						*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SOFT_RESET			0									/* Execute soft reset						*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_CPU_HALT 																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	HALT_OFF			1									/* HALTCancellation									*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_CLK_EN 	 																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	UDL_NET_CLK			0									/* M3UDLclock							*/
#define	UDL_SERVO_CLK		1									/* ServoUDLclock							*/
#define	ADC_CLK				2									/* ADCclock								*/
#define	GBETHER_CLK			3									/* GbEtherclock							*/
#define	DDR3_CLK			4									/* DDR3clock								*/
#define	USB_CLK				5									/* USBclock								*/
#define	BUSOUT_CLK			6									/* BUSCLK_OUTclock						*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_CLK_SEL 	 																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	CLK_MODE1		0										/* mode1									*/
#define	CLK_MODE2		1										/* mode2									*/
#define	CLK_MODE3		2										/* mode3									*/
#define	CLK_MODE4		3										/* mode4									*/
#define	I2C_CKSEL		0										/* I2CClock frequency switching				*/
																/* mode1:357Kbps							*/
																/* mode2:384Kbps							*/
																/* mode3:400Kbps							*/
																/* mode4:Setting prohibited						*/
#define	DDR3_CKSEL		2										/* DDR3Clock frequency switching				*/
																/* mode1:600Mbps							*/
																/* mode2:800Mbps							*/
																/* mode3:1200Mbps						*/
																/* mode4:Setting prohibited						*/
#define	OUT_CKSEL		4										/* External bus clock frequency switching			*/
																/* mode1:BUSCLK(125MHz)					*/
																/* mode2:BUSCLK/2						*/
																/* mode3:BUSCLK/4						*/
																/* mode4:BUSCLK/8						*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_MODE		 																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	MOVE_MODE		0										/* operating mode								*/
#define	VECTOR_MODE		4										/* VECTORmode								*/
#define	CPU_CLK_MODE	6										/* CPUClock frequency selection					*/
#define	OSC_MODE		7										/* OSCbuffer mode						*/
#define	MODE_VNDMD		0										/* Renesas test mode						*/
#define	MODE_RSV0		1										/* Reserve mode							*/
#define	MODE_RSV1		2										/* Reserve mode							*/
#define	MODE_TMD3		3										/* simulation mode(TMD3)				*/
#define	MODE_TMD4		4										/* simulation mode(TMD4)				*/
#define	MODE_DBGMD		5										/* UDL_Servodebug mode					*/
#define	MODE_MDBGMD		6										/* UDL_M3debug mode						*/
#define	MODE_NORMAL		7										/* normal mode							*/
#define	BOOT_PROM		0										/* parallelROMboot(LOWVEC)				*/
#define	BOOT_PCI		1										/* PCIexboot(HIVEC)						*/
#define	BOOT_SROM		2										/* serialROMboot(HIVEC)					*/
#define	BOOT_M3			3										/* M3boot(HIVEC)							*/
#define	CPUCLK_250		0										/* CPU Clock 250MHz							*/
#define	CPUCLK_500		1										/* CPU Clock 500MHz							*/
#define	OSC_XTOFF		0										/* OSCBuffer not used						*/
#define	OSC_X25MD		1										/* X25OSCuse								*/
#define	OSC_X30MD		2										/* X30OSCuse								*/
#define	OSC_XTON		3										/* Use of oscillator								*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_REMAP	 																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	REMAP_OFF		0										/* remap disabled								*/
#define	REMAP_ON		1										/* Remapping enabled								*/
#define	DDR_REMAP		0										/* DDR REMAP								*/
#define	AXI_REMAP		1										/* AXI-RAM REMAP(read only)					*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_PCIE_INT, SSPRO7_PCIE_INTMSK																	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	PCIE_INTA		0										/* INTA_EPInterrupt issue/mask					*/
#define	PCIE_INTB		1										/* INTB_EPInterrupt issue/mask					*/
#define	PCIE_INTC		2										/* INTC_EPInterrupt issue/mask					*/
#define	PCIE_INTD		3										/* INTD_EPInterrupt issue/mask					*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_PCIE_INTSEL 																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	PCIE_INTSEL		0										/* PCIexInterrupt selection register					*/
#define	PCIE_INTMODE	4										/* Interrupt signal timing control register			*/
#define	PCIE_TLPEND_INT	0										/* PCIE_TLPEND_INTofUDLconnect to				*/
#define	PCIEX_GPO_0		1										/* PCIEX_GPO_0ofUDLconnect to					*/
#define	PCIE_INTA_RC	2										/* PCIE_INTA_RCofUDLconnect to					*/
#define	PCIE_INTB_RC	3										/* PCIE_INTB_RCofUDLconnect to					*/
#define	PCIE_INTC_RC	4										/* PCIE_INTC_RCofUDLconnect to					*/
#define	PCIE_INTD_RC	5										/* PCIE_INTD_RCofUDLconnect to					*/
#define	PCIE_INTMSI_RC	6										/* PCIE_INTMSI_RCofUDLconnect to				*/
#define	PCIE_MSG_INT_RC	7										/* PCIE_MSG_INT_RCofUDLconnect to				*/
#define	INT_8CLK_PLS	0										/* ACLKof8Output as clock width					*/
#define	INT_LVL01		1										/* level output								*/
#define	INT_3CLK_PLS	2										/* ACLKof3Output as clock width					*/
#define	INT_LVL11		3										/* level output								*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_WDT_MODE	 																					*/
/*----------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_5TBUF_EN	 																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	OV_CNT		0											/* OVTerminal standby control register				*/
#define	OC1_CNT		1											/* OC1Terminal standby control register			*/
#define	OC2_CNT		2											/* OC2Terminal standby control register			*/
#define	SYNCL_CNT	3											/* SUNCLTerminal standby control register			*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_NMI_SET	 																					*/
/*----------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_INTSYS1_EN 																					*/
/*----------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_INTSYS2_EN 																					*/
/*----------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_VICIFSYN_EN 																					*/
/*----------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_PCIE_CLKSEL 																					*/
/*----------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_RESET_STATUS																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SW_RST_STS		1										/* software reset						*/
#define	WDT_RST_STS		2										/* UDL WDTreset							*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_UDLCNT	 																					*/
/*----------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_CPUSS_TRIGINT																				*/
/*----------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_GBETHER_CONT																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	GB_PHYMODE		0										/* GbEtherCommunication mode setting					*/
#define	GB_SPEED		4										/* Communication speed setting								*/
#define	GB_RMIIMODE		8										/* RMIITransfer mode settings during transfer				*/
#define	GB_CLKSEL		12										/* RGMIIClock control settings during transfer			*/
#define	GB_PHY_MII		0x0										/* MIImode								*/
#define	GB_PHY_RMII		0x1										/* RMIINormal mode							*/
#define	GB_PHY_RMIIRX	0x9										/* RMIItransmit clockRXinput mode				*/
#define	GB_PHY_GMII		0xA										/* GMIImode								*/
#define	GB_PHY_RGMII	0xB										/* RGMIImode								*/
#define	GB_SPD_10M		0										/* Communication speed10Mbps							*/
#define	GB_SPD_100M		1										/* Communication speed100Mbps							*/
#define	GB_SPD_1G		2										/* Communication speed1Gbps							*/



/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_UDLINT_CONT 																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	PCI_EP_A_SEL	0										/* PCI EPinterruptASelect interrupt to output to		*/
#define	PCI_EP_B_SEL	4										/* PCI EPinterruptBSelect interrupt to output to		*/
#define	PCI_EP_C_SEL	8										/* PCI EPinterruptCSelect interrupt to output to		*/
#define	PCI_EP_D_SEL	12										/* PCI EPinterruptDSelect interrupt to output to		*/
#define	UDL_INTA		0x0										/* UDL_INTA(Servo)output					*/
#define	UDL_INTB		0x1										/* UDL_INTB(Servo)output					*/
#define	UDL_INTC		0x2										/* UDL_INTC(Servo)output					*/
#define	UDL_INTN		0x3										/* UDL_INTN(Servo)output					*/
#define	UDL_INT1		0x4										/* UDL_INT1(Servo)output					*/
#define	UDL_INT2		0x5										/* UDL_INT2(Servo)output					*/
#define	UDL_M3INT0		0x6										/* UDL-M3_INT0(Network)output 				*/
#define	UDL_M3INT1		0x7										/* UDL-M3_INT1(Network)output 				*/
#define	UDL_ALLSERVO	0x8										/* Servoall of the interrupts OR output 			*/
#define	UDL_ALLM3		0x9										/* M3UDLall of the interrupts OR output 			*/

/*----------------------------------------------------------------------------------------------------------*/
/*		SSPRO7_UDLINT_STATUS																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	UDL_INTA_OM		0x01									/* UDL_INTA(Servo)output					*/
#define	UDL_INTB_OM		0x02									/* UDL_INTB(Servo)output					*/
#define	UDL_INTC_OM		0x04									/* UDL_INTC(Servo)output					*/
#define	UDL_INTN_OM		0x08									/* UDL_INTN(Servo)output					*/
#define	UDL_INT1_OM		0x10									/* UDL_INT1(Servo)output					*/
#define	UDL_INT2_OM		0x20									/* UDL_INT2(Servo)output					*/
#define	UDL_M3INT0_OM	0x40									/* UDL-M3_INT0(Network)output 				*/
#define	UDL_M3INT1_OM	0x80									/* UDL-M3_INT1(Network)output 				*/


/*----------------------------------------------------------------------------------------------------------*/
/*		APBREG15	<S152>																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	I2C_LOW			12										/* */
#define	LOW_DISABLE		0										/* LowDetection disabled(ES2same as) */
#define	LOW_ENAABLE		1										/* LowDetection enabled */
/*----------------------------------------------------------------------------------------------------------*/
#define	I2C_RATE		8										/* */
#define	RATE_D1			0									/* 00:D1(ES2same as) */
#define	RATE_D2			1									/* 01:D2 */
#define	RATE_D3			2									/* 10:D3 */
#define	RATE_D4			3									/* 11:D4(for High speed mode */
/*----------------------------------------------------------------------------------------------------------*/
#define	I2C_CK			0										/* */
#define	CLK_738			0x00									/* 7.38MHz */
#define	CLK_750			0x01									/* 7.50MHz */
#define	CLK_762			0x02									/* 7.62MHz */
#define	CLK_774			0x03									/* 7.74MHz */
#define	CLK_787			0x04									/* 7.87MHz */
#define	CLK_800			0x05									/* 8.00MHz */
#define	CLK_814			0x06									/* 8.14MHz */
#define	CLK_828			0x07									/* 8.28MHz */
#define	CLK_842			0x08									/* 8.42MHz */
#define	CLK_857			0x09									/* 8.57MHz */
#define	CLK_873			0x0A									/* 8.73MHz */
#define	CLK_889			0x0B									/* 8.89MHz */
#define	CLK_906			0x0C									/* 9.06MHz */
#define	CLK_923			0x0D									/* 9.23MHz */
#define	CLK_941			0x0E									/* 9.41MHz */
#define	CLK_960			0x0F									/* 9.60MHz */
/*----------------------------------------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------------------------------------*/
/*		API MACRO																							*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_GET_ID()		(ARMPF_REG_READ( ARMPF_SCU_DEVICE_ID ) & 0xFFFF)			/* JL-086IDacquisition				*//* <S119>ES4Compatible with later *//* code organization<S191> */
#define	SCU_GET_VER()		(ARMPF_REG_READ( ARMPF_SCU_DEVICE_ID ) & 0xFFFF) >> 16 )	/* JL-086 ES versionacquisition	*//* <S191>ES4Compatible with later */
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SOFTRESET()		ARMPF_REG_WRITE( ARMPF_SCU_SOFT_RST, SOFT_RESET )	/* JL-086soft reset		*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_CPU_HALT()		ARMPF_REG_WRITE( ARMPF_SCU_CPU_HALT, HALT_OFF )		/* PCIat boot timeCPUHALTCancellation	*/
/*----------------------------------------------------------------------------------------------------------*/
#if 1
#define	SCU_CLK_START( x )	ARMPF_REG_BITCLR( ARMPF_SCU_CLK_EN, x )				/* module clock supply	*/
#define	SCU_CLK_STOP( x )	ARMPF_REG_BITSET( ARMPF_SCU_CLK_EN, x )				/* Module clock stop	*/
#else
#define	SCU_CLK_START( x )					/* module clock supply	*/
#define	SCU_CLK_STOP( x )					/* Module clock stop	*/
#endif
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_CLK_SEL( x, mode )	ARMPF_REG_VALSET( ARMPF_SCU_CLK_SEL, (mode << x) )	/* Module clock selection	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_GET_MODE( mode )	( ARMPF_REG_READ(ARMPF_SCU_MODE) >> mode )			/* Get operation mode			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SET_REMAP( ram )	ARMPF_REG_WRITE( ARMPF_SCU_REMAP, (REMAP_ON << ram) )	/* Execute remap			*/
#define	SCU_GET_REMAP( ram )	( ARMPF_REG_READ(ARMPF_SCU_REMAP) >> ram )		/* Get remap status			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SET_PCIEINT( x )	ARMPF_REG_BITSET( ARMPF_SCU_PCIE_INT, x )		/* PCIINTissue				*/
#define	SCU_CLR_PCIEINT( x )	ARMPF_REG_BITCLR( ARMPF_SCU_PCIE_INT, x )		/* PCIINTclear				*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_MASK_PCIEINT( x )	ARMPF_REG_BITSET( ARMPF_SCU_PCIE_INTMSK, x )	/* PCIINTmask				*/
#define	SCU_MASKCLR_PCIEINT( x )	ARMPF_REG_BITCLR( ARMPF_SCU_PCIE_INTMSK, x )	/* PCIINTmask disabled		*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SET_PCIE_INTSEL( x )	ARMPF_REG_VALSET( ARMPF_SCU_PCIE_INTSEL, (x << PCIE_INTSEL) )	/* PCIexInterrupt selection register set	*/
#define	SCU_SET_PCIE_INTMODE( x )	ARMPF_REG_VALSET( ARMPF_SCU_PCIE_INTSEL, (x << PCIE_INTMODE) )	/* Interrupt signal timing set	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_ENABLE_WDT_RST()	ARMPF_REG_BITSET( ARMPF_SCU_WDT_MODE, 0 )		/* UDL_WDTReset enabled		*/
#define	SCU_DISABLE_WDT_RST()	ARMPF_REG_BITCLR( ARMPF_SCU_WDT_MODE, 0 )		/* UDL_WDTReset disabled		*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SET_5TBUF_EN( x )	ARMPF_REG_BITSET( ARMPF_SCU_5TBUF_EN, x )		/* 5VTolerant buffer standby	*/
#define	SCU_CLR_5TBUF_EN( x )	ARMPF_REG_BITCLR( ARMPF_SCU_5TBUF_EN, x )		/* 5VTolerant buffer disabled			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_ENABLE_FIQ_MASK()	ARMPF_REG_BITSET( ARMPF_SCU_NMI_SET, 0 )		/* FIQmask enabled			*/
#define	SCU_DISABLE_FIQ_MASK()	ARMPF_REG_BITCLR( ARMPF_SCU_NMI_SET, 0 )		/* FIQmask disabled			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_INTSYS1_EN( x )		ARMPF_REG_WRITE( ARMPF_SCU_INTSYS1_EN, x )		/* Peripheral circuit interrupt input synchronization/Asynchronous settings1	*/
#define	SCU_INTSYS2_EN( x )		ARMPF_REG_WRITE( ARMPF_SCU_INTSYS2_EN, x )		/* Peripheral circuit interrupt input synchronization/Asynchronous settings2	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SET_VICIFSYN()		ARMPF_REG_BITSET( ARMPF_SCU_VICIFSYN_EN, 0 )	/* CPU-VICSynchronization settings between		*/
#define	SCU_CLR_VICIFSYN()		ARMPF_REG_BITCLR( ARMPF_SCU_VICIFSYN_EN, 0 )	/* CPU-VICAsynchronous settings between	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_PCIECLK_IN()		ARMPF_REG_BITSET( ARMPF_SCU_PCIE_CLKSEL, 0 )	/* PCIeInternal clock supply		*/
#define	SCU_PCIECLK_OUT()		ARMPF_REG_BITCLR( ARMPF_SCU_PCIE_CLKSEL, 0 )	/* PCIeExternal differential clock input	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_READ_RST_STATUS()	ARMPF_REG_READ(ARMPF_SCU_RESET_STATUS)			/* Reset status read	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SET_UDLM3_PRGSEL()	ARMPF_REG_WRITE( ARMPF_SCU_UDLCNT, 1 )			/* M3UDL PRGSELvalid			*/
#define	SCU_CLR_UDLM3_PRGSEL()	ARMPF_REG_WRITE( ARMPF_SCU_UDLCNT, 0 )			/* M3UDL PRGSELinvalid			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_READ_CPUSS_TRIGINT()	ARMPF_REG_READ(ARMPF_SCU_CPUSS_TRIGINT)		/* Cross trigger interrupt factor read	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SET_GB_PHYMODE( x )	ARMPF_REG_VALSET( ARMPF_SCU_GBETHER_CONT, (x << GB_PHYMODE) )	/* GbEtherCommunication mode setting		*/
#define	SCU_SET_GB_SPEED( x )	ARMPF_REG_VALSET( ARMPF_SCU_GBETHER_CONT, (x << GB_SPEED) )		/* Communication speed setting					*/
#define	SCU_SET_GB_HALF_DUPLX()	ARMPF_REG_BITSET( ARMPF_SCU_GBETHER_CONT, GB_RMIIMODE )			/* RMIITransfer half double transfer			*/
#define	SCU_SET_GB_FULL_DUPLX()	ARMPF_REG_BITCLR( ARMPF_SCU_GBETHER_CONT, GB_RMIIMODE )			/* RMIIFull duplex transfer during transfer			*/
#define	SCU_SET_GB_CLK_REVPHASE()	ARMPF_REG_BITSET( ARMPF_SCU_GBETHER_CONT, GB_CLKSEL )		/* RGMIIClock opposite phase during transfer	*/
#define	SCU_SET_GB_CLK_COPHASE()	ARMPF_REG_BITCLR( ARMPF_SCU_GBETHER_CONT, GB_CLKSEL )		/* RGMIIClock same phase during transfer	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_PCI_EP_A_SEL( x )	ARMPF_REG_VALSET( ARMPF_SCU_UDLINT_CONT, (x << PCI_EP_A_SEL) )	/* PCI EPinterruptAInterrupt settings to output to		*/
#define	SCU_PCI_EP_B_SEL( x )	ARMPF_REG_VALSET( ARMPF_SCU_UDLINT_CONT, (x << PCI_EP_B_SEL) )	/* PCI EPinterruptBInterrupt settings to output to		*/
#define	SCU_PCI_EP_C_SEL( x )	ARMPF_REG_VALSET( ARMPF_SCU_UDLINT_CONT, (x << PCI_EP_C_SEL) )	/* PCI EPinterruptCInterrupt settings to output to		*/
#define	SCU_PCI_EP_D_SEL( x )	ARMPF_REG_VALSET( ARMPF_SCU_UDLINT_CONT, (x << PCI_EP_D_SEL) )	/* PCI EPinterruptDInterrupt settings to output to		*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_READ_UDLINT_STATUS()	ARMPF_REG_READ(ARMPF_SCU_UDLINT_STATUS)		/* UDL INTInterrupt factor read	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	SCU_SET_APBREG15( a, b, c )	ARMPF_REG_VALSET( ARMPF_SCU_APBREG15, ( (a)<<(I2C_LOW) | (b)<<(I2C_RATE) | (c)<<(I2C_CK) ) )		/* <S152> */

#endif
/*-------end of file----------------------------------------------------------------------------------------*/

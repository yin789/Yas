/************************************************************************************************************/
/*																											*/
/*																											*/
/*		ARMPF_VIC.h : MercurySoC peripheral VICheader														*/
/*																											*/
/*																											*/
/************************************************************************************************************/
/*																											*/
/*																											*/
/*		interrupt controller(VIC)Related header files															*/
/*																											*/
/*																											*/
/************** Copyright (C) Yaskawa Electric Corporation **************************************************/
/*																											*/
/*		Rev.1.00 : 2013.07.14  Y.Tanaka		First edition															*/
/*																											*/
/************************************************************************************************************/
#include	"ARMPF.h"


#ifndef	_ARMPF_VIC_H_
#define	_ARMPF_VIC_H_


#define	ARMPF_VIC_BASE	0xEFFDF000							/* VICbase address							*/

/*----------------------------------------------------------------------------------------------------------*/
/*		Interrupt execution section selection																					*/
/*----------------------------------------------------------------------------------------------------------*/
#define	ARMPF_VIC_IRQ		0x00000000						/* IRQinterrupt									*/
#define	ARMPF_VIC_FIQ		0x00000001						/* FIQinterrupt									*/

/*----------------------------------------------------------------------------------------------------------*/
/*		Interrupt detection method selection	PLS																			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	ARMPF_VIC_LVL		0x00							/* level detection									*/
#define	ARMPF_VIC_EDGE		0x01							/* edge detection									*/

/*----------------------------------------------------------------------------------------------------------*/
/*		Interrupt detection enable level selection	LVLC0-7																	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	ARMPF_VIC_HIGH		0x01							/* Detected at high level								*/
#define	ARMPF_VIC_LOW 		0x02							/* Detected at low level								*/

/*----------------------------------------------------------------------------------------------------------*/
/*		Interrupt detection valid edge selection	EDGC0-7																	*/
/*----------------------------------------------------------------------------------------------------------*/
#define	ARMPF_VIC_RISE		0x01							/* Detected at rising edge								*/
#define	ARMPF_VIC_FALL		0x02							/* Detected at falling edge									*/
#define	ARMPF_VIC_BIEDGE	0x03							/* Detection on both edges								*/

/*----------------------------------------------------------------------------------------------------------*/
/*		interrupt priority	PRL0-127																			*/
/*----------------------------------------------------------------------------------------------------------*/
#define	ARMPF_VIC_PRIORYTY0		0							/* priority0(MAX)								*/
#define	ARMPF_VIC_PRIORYTY1		1							/* priority1										*/
#define	ARMPF_VIC_PRIORYTY2		2							/* priority2										*/
#define	ARMPF_VIC_PRIORYTY3		3							/* priority3										*/
#define	ARMPF_VIC_PRIORYTY4		4							/* priority4										*/
#define	ARMPF_VIC_PRIORYTY5		5							/* priority5										*/
#define	ARMPF_VIC_PRIORYTY6		6							/* priority6										*/
#define	ARMPF_VIC_PRIORYTY7		7							/* priority7										*/
#define	ARMPF_VIC_PRIORYTY8		8							/* priority8										*/
#define	ARMPF_VIC_PRIORYTY9		9							/* priority9										*/
#define	ARMPF_VIC_PRIORYTY10	10							/* priority10										*/
#define	ARMPF_VIC_PRIORYTY11	11							/* priority11										*/
#define	ARMPF_VIC_PRIORYTY12	12							/* priority12										*/
#define	ARMPF_VIC_PRIORYTY13	13							/* priority13										*/
#define	ARMPF_VIC_PRIORYTY14	14							/* priority14										*/
#define	ARMPF_VIC_PRIORYTY15	15							/* priority15(MIN)								*/

/*----------------------------------------------------------------------------------------------------------*/
/*		VIC register																						*/
/*----------------------------------------------------------------------------------------------------------*/
#define	ARMPF_VIC_IRQS0		(ARMPF_VIC_BASE + 0x000)		/* IRQstatus register0						*/
#define	ARMPF_VIC_IRQS1		(ARMPF_VIC_BASE + 0x004)		/* IRQstatus register1						*/
//#define	ARMPF_VIC_IRQS2		(ARMPF_VIC_BASE + 0x008)		/* IRQstatus register2						*//* JL-086Not available */
//#define	ARMPF_VIC_IRQS3		(ARMPF_VIC_BASE + 0x00C)		/* IRQstatus register3						*//* JL-086Not available */
#define	ARMPF_VIC_FIQS0		(ARMPF_VIC_BASE + 0x020)		/* FIQstatus register0						*/
#define	ARMPF_VIC_FIQS1		(ARMPF_VIC_BASE + 0x024)		/* FIQstatus register1						*/
//#define	ARMPF_VIC_FIQS2		(ARMPF_VIC_BASE + 0x028)		/* FIQstatus register2						*//* JL-086Not available */
//#define	ARMPF_VIC_FIQS3		(ARMPF_VIC_BASE + 0x02C)		/* FIQstatus register3						*//* JL-086Not available */
#define	ARMPF_VIC_RAIS0		(ARMPF_VIC_BASE + 0x040)		/* Interrupt input status register0			*/
#define	ARMPF_VIC_RAIS1		(ARMPF_VIC_BASE + 0x044)		/* Interrupt input status register1			*/
//#define	ARMPF_VIC_RAIS2		(ARMPF_VIC_BASE + 0x048)		/* Interrupt input status register2			*//* JL-086Not available */
//#define	ARMPF_VIC_RAIS3		(ARMPF_VIC_BASE + 0x04C)		/* Interrupt input status register3			*//* JL-086Not available */
#define	ARMPF_VIC_ISL0		(ARMPF_VIC_BASE + 0x060)		/* IRQ/FIQInterrupt selection register0					*/
#define	ARMPF_VIC_ISL1		(ARMPF_VIC_BASE + 0x064)		/* IRQ/FIQInterrupt selection register1					*/
//#define	ARMPF_VIC_ISL2		(ARMPF_VIC_BASE + 0x068)		/* IRQ/FIQInterrupt selection register2					*//* JL-086Not available */
//#define	ARMPF_VIC_ISL3		(ARMPF_VIC_BASE + 0x06C)		/* IRQ/FIQInterrupt selection register3					*//* JL-086Not available */
#define	ARMPF_VIC_IEN0		(ARMPF_VIC_BASE + 0x080)		/* Interrupt enable register0				*/
#define	ARMPF_VIC_IEN1		(ARMPF_VIC_BASE + 0x084)		/* Interrupt enable register1				*/
//#define	ARMPF_VIC_IEN2		(ARMPF_VIC_BASE + 0x088)		/* Interrupt enable register2				*//* JL-086Not available */
//#define	ARMPF_VIC_IEN3		(ARMPF_VIC_BASE + 0x08C)		/* Interrupt enable register3				*//* JL-086Not available */
#define	ARMPF_VIC_IEC0		(ARMPF_VIC_BASE + 0x0A0)		/* Interrupt Enable Clear Register0		*/
#define	ARMPF_VIC_IEC1		(ARMPF_VIC_BASE + 0x0A4)		/* Interrupt Enable Clear Register1		*/
//#define	ARMPF_VIC_IEC2		(ARMPF_VIC_BASE + 0x0A8)		/* Interrupt Enable Clear Register2		*//* JL-086Not available */
//#define	ARMPF_VIC_IEC3		(ARMPF_VIC_BASE + 0x0AC)		/* Interrupt Enable Clear Register3		*//* JL-086Not available */
#define	ARMPF_VIC_SWI0		(ARMPF_VIC_BASE + 0x0C0)		/* Software interrupt register0				*/
#define	ARMPF_VIC_SWI1		(ARMPF_VIC_BASE + 0x0C4)		/* Software interrupt register1				*/
//#define	ARMPF_VIC_SWI2		(ARMPF_VIC_BASE + 0x0C8)		/* Software interrupt register2				*//* JL-086Not available */
//#define	ARMPF_VIC_SWI3		(ARMPF_VIC_BASE + 0x0CC)		/* Software interrupt register3				*//* JL-086Not available */
#define	ARMPF_VIC_SWC0		(ARMPF_VIC_BASE + 0x0E0)		/* Software interrupt clear register0		*/
#define	ARMPF_VIC_SWC1		(ARMPF_VIC_BASE + 0x0E4)		/* Software interrupt clear register1		*/
//#define	ARMPF_VIC_SWC2		(ARMPF_VIC_BASE + 0x0E8)		/* Software interrupt clear register2		*//* JL-086Not available */
//#define	ARMPF_VIC_SWC3		(ARMPF_VIC_BASE + 0x0EC)		/* Software interrupt clear register3		*//* JL-086Not available */
#define	ARMPF_VIC_PLS0		(ARMPF_VIC_BASE + 0x100)		/* Interrupt detection type selection register0				*/
#define	ARMPF_VIC_PLS1		(ARMPF_VIC_BASE + 0x104)		/* Interrupt detection type selection register1				*/
//#define	ARMPF_VIC_PLS2		(ARMPF_VIC_BASE + 0x108)		/* Interrupt detection type selection register2				*//* JL-086Not available */
//#define	ARMPF_VIC_PLS3		(ARMPF_VIC_BASE + 0x10C)		/* Interrupt detection type selection register3				*//* JL-086Not available */
#define	ARMPF_VIC_PIC0		(ARMPF_VIC_BASE + 0x120)		/* EDGE DETECT BIT CLEAR REGISTER0			*/
#define	ARMPF_VIC_PIC1		(ARMPF_VIC_BASE + 0x124)		/* EDGE DETECT BIT CLEAR REGISTER1			*/
//#define	ARMPF_VIC_PIC2		(ARMPF_VIC_BASE + 0x128)		/* EDGE DETECT BIT CLEAR REGISTER2			*//* JL-086Not available */
//#define	ARMPF_VIC_PIC3		(ARMPF_VIC_BASE + 0x12C)		/* EDGE DETECT BIT CLEAR REGISTER3			*//* JL-086Not available */
#define	ARMPF_VIC_EDGC0		(ARMPF_VIC_BASE + 0x140)		/* Interrupt edge control register0		*/
#define	ARMPF_VIC_EDGC1		(ARMPF_VIC_BASE + 0x144)		/* Interrupt edge control register1		*/
#define	ARMPF_VIC_EDGC2		(ARMPF_VIC_BASE + 0x148)		/* Interrupt edge control register2		*/
#define	ARMPF_VIC_EDGC3		(ARMPF_VIC_BASE + 0x14C)		/* Interrupt edge control register3		*/
//#define	ARMPF_VIC_EDGC4		(ARMPF_VIC_BASE + 0x150)		/* Interrupt edge control register4		*//* JL-086Not available */
//#define	ARMPF_VIC_EDGC5		(ARMPF_VIC_BASE + 0x154)		/* Interrupt edge control register5		*//* JL-086Not available */
//#define	ARMPF_VIC_EDGC6		(ARMPF_VIC_BASE + 0x158)		/* Interrupt edge control register6		*//* JL-086Not available */
//#define	ARMPF_VIC_EDGC7		(ARMPF_VIC_BASE + 0x15C)		/* Interrupt edge control register7		*//* JL-086Not available */
#define	ARMPF_VIC_LVLC0		(ARMPF_VIC_BASE + 0x180)		/* Interrupt level control register0		*/
#define	ARMPF_VIC_LVLC1		(ARMPF_VIC_BASE + 0x184)		/* Interrupt level control register1		*/
#define	ARMPF_VIC_LVLC2		(ARMPF_VIC_BASE + 0x188)		/* Interrupt level control register2		*/
#define	ARMPF_VIC_LVLC3		(ARMPF_VIC_BASE + 0x18C)		/* Interrupt level control register3		*/
//#define	ARMPF_VIC_LVLC4		(ARMPF_VIC_BASE + 0x190)		/* Interrupt level control register4		*//* JL-086Not available */
//#define	ARMPF_VIC_LVLC5		(ARMPF_VIC_BASE + 0x194)		/* Interrupt level control register5		*//* JL-086Not available */
//#define	ARMPF_VIC_LVLC6		(ARMPF_VIC_BASE + 0x198)		/* Interrupt level control register6		*//* JL-086Not available */
//#define	ARMPF_VIC_LVLC7		(ARMPF_VIC_BASE + 0x19C)		/* Interrupt level control register7		*//* JL-086Not available */
#define	ARMPF_VIC_PRLM		(ARMPF_VIC_BASE + 0x1C0)		/* Interrupt priority level mask register			*/
#define	ARMPF_VIC_PRLC		(ARMPF_VIC_BASE + 0x1C4)		/* Interrupt priority level mask clear register	*/
#define	ARMPF_VIC_UEN 		(ARMPF_VIC_BASE + 0x1C8)		/* User mode enable register			*/
#define	ARMPF_VIC_HVA 		(ARMPF_VIC_BASE + 0x200)		/* Interrupt address register					*/
#define	ARMPF_VIC_ISS0 		(ARMPF_VIC_BASE + 0x210)		/* Interrupt Service Status Register0		*/
#define	ARMPF_VIC_ISS1 		(ARMPF_VIC_BASE + 0x214)		/* Interrupt Service Status Register1		*/
//#define	ARMPF_VIC_ISS2 		(ARMPF_VIC_BASE + 0x218)		/* Interrupt Service Status Register2		*//* JL-086Not available */
//#define	ARMPF_VIC_ISS3 		(ARMPF_VIC_BASE + 0x21C)		/* Interrupt Service Status Register3		*//* JL-086Not available */
#define	ARMPF_VIC_ISC0 		(ARMPF_VIC_BASE + 0x230)		/* Interrupt service current register0		*/
#define	ARMPF_VIC_ISC1 		(ARMPF_VIC_BASE + 0x234)		/* Interrupt service current register1		*/
//#define	ARMPF_VIC_ISC2 		(ARMPF_VIC_BASE + 0x238)		/* Interrupt service current register2		*//* JL-086Not available */
//#define	ARMPF_VIC_ISC3 		(ARMPF_VIC_BASE + 0x23C)		/* Interrupt service current register3		*//* JL-086Not available */
#define	ARMPF_VIC_VAD0		(ARMPF_VIC_BASE + 0x400)		/* Interrupt address storage register0				*/
#define	ARMPF_VIC_VAD1		(ARMPF_VIC_BASE + 0x404)		/* Interrupt address storage register1				*/
#define	ARMPF_VIC_VAD2		(ARMPF_VIC_BASE + 0x408)		/* Interrupt address storage register2				*/
#define	ARMPF_VIC_VAD3		(ARMPF_VIC_BASE + 0x40C)		/* Interrupt address storage register3				*/
#define	ARMPF_VIC_VAD4		(ARMPF_VIC_BASE + 0x410)		/* Interrupt address storage register4				*/
#define	ARMPF_VIC_VAD5		(ARMPF_VIC_BASE + 0x414)		/* Interrupt address storage register5				*/
#define	ARMPF_VIC_VAD6		(ARMPF_VIC_BASE + 0x418)		/* Interrupt address storage register6				*/
#define	ARMPF_VIC_VAD7		(ARMPF_VIC_BASE + 0x41C)		/* Interrupt address storage register7				*/
#define	ARMPF_VIC_VAD8		(ARMPF_VIC_BASE + 0x420)		/* Interrupt address storage register8				*/
#define	ARMPF_VIC_VAD9		(ARMPF_VIC_BASE + 0x424)		/* Interrupt address storage register9				*/
#define	ARMPF_VIC_VAD10		(ARMPF_VIC_BASE + 0x428)		/* Interrupt address storage register10				*/
#define	ARMPF_VIC_VAD11		(ARMPF_VIC_BASE + 0x42C)		/* Interrupt address storage register11				*/
#define	ARMPF_VIC_VAD12		(ARMPF_VIC_BASE + 0x430)		/* Interrupt address storage register12				*/
#define	ARMPF_VIC_VAD13		(ARMPF_VIC_BASE + 0x434)		/* Interrupt address storage register13				*/
#define	ARMPF_VIC_VAD14		(ARMPF_VIC_BASE + 0x438)		/* Interrupt address storage register14				*/
#define	ARMPF_VIC_VAD15		(ARMPF_VIC_BASE + 0x43C)		/* Interrupt address storage register15				*/
#define	ARMPF_VIC_VAD16		(ARMPF_VIC_BASE + 0x440)		/* Interrupt address storage register16				*/
#define	ARMPF_VIC_VAD17		(ARMPF_VIC_BASE + 0x444)		/* Interrupt address storage register17				*/
#define	ARMPF_VIC_VAD18		(ARMPF_VIC_BASE + 0x448)		/* Interrupt address storage register18				*/
#define	ARMPF_VIC_VAD19		(ARMPF_VIC_BASE + 0x44C)		/* Interrupt address storage register19				*/
#define	ARMPF_VIC_VAD20		(ARMPF_VIC_BASE + 0x450)		/* Interrupt address storage register20				*/
#define	ARMPF_VIC_VAD21		(ARMPF_VIC_BASE + 0x454)		/* Interrupt address storage register21				*/
#define	ARMPF_VIC_VAD22		(ARMPF_VIC_BASE + 0x458)		/* Interrupt address storage register22				*/
#define	ARMPF_VIC_VAD23		(ARMPF_VIC_BASE + 0x45C)		/* Interrupt address storage register23				*/
#define	ARMPF_VIC_VAD24		(ARMPF_VIC_BASE + 0x460)		/* Interrupt address storage register24				*/
#define	ARMPF_VIC_VAD25		(ARMPF_VIC_BASE + 0x464)		/* Interrupt address storage register25				*/
#define	ARMPF_VIC_VAD26		(ARMPF_VIC_BASE + 0x468)		/* Interrupt address storage register26				*/
#define	ARMPF_VIC_VAD27		(ARMPF_VIC_BASE + 0x46C)		/* Interrupt address storage register27				*/
#define	ARMPF_VIC_VAD28		(ARMPF_VIC_BASE + 0x470)		/* Interrupt address storage register28				*/
#define	ARMPF_VIC_VAD29		(ARMPF_VIC_BASE + 0x474)		/* Interrupt address storage register29				*/
#define	ARMPF_VIC_VAD30		(ARMPF_VIC_BASE + 0x478)		/* Interrupt address storage register30				*/
#define	ARMPF_VIC_VAD31		(ARMPF_VIC_BASE + 0x47C)		/* Interrupt address storage register31				*/
#define	ARMPF_VIC_VAD32		(ARMPF_VIC_BASE + 0x480)		/* Interrupt address storage register32				*/
#define	ARMPF_VIC_VAD33		(ARMPF_VIC_BASE + 0x484)		/* Interrupt address storage register33				*/
#define	ARMPF_VIC_VAD34		(ARMPF_VIC_BASE + 0x488)		/* Interrupt address storage register34				*/
#define	ARMPF_VIC_VAD35		(ARMPF_VIC_BASE + 0x48C)		/* Interrupt address storage register35				*/
#define	ARMPF_VIC_VAD36		(ARMPF_VIC_BASE + 0x490)		/* Interrupt address storage register36				*/
#define	ARMPF_VIC_VAD37		(ARMPF_VIC_BASE + 0x494)		/* Interrupt address storage register37				*/
#define	ARMPF_VIC_VAD38		(ARMPF_VIC_BASE + 0x498)		/* Interrupt address storage register38				*/
#define	ARMPF_VIC_VAD39		(ARMPF_VIC_BASE + 0x49C)		/* Interrupt address storage register39				*/
#define	ARMPF_VIC_VAD40		(ARMPF_VIC_BASE + 0x4A0)		/* Interrupt address storage register40				*/
#define	ARMPF_VIC_VAD41		(ARMPF_VIC_BASE + 0x4A4)		/* Interrupt address storage register41				*/
#define	ARMPF_VIC_VAD42		(ARMPF_VIC_BASE + 0x4A8)		/* Interrupt address storage register42				*/
#define	ARMPF_VIC_VAD43		(ARMPF_VIC_BASE + 0x4AC)		/* Interrupt address storage register43				*/
#define	ARMPF_VIC_VAD44		(ARMPF_VIC_BASE + 0x4B0)		/* Interrupt address storage register44				*/
#define	ARMPF_VIC_VAD45		(ARMPF_VIC_BASE + 0x4B4)		/* Interrupt address storage register45				*/
#define	ARMPF_VIC_VAD46		(ARMPF_VIC_BASE + 0x4B8)		/* Interrupt address storage register46				*/
#define	ARMPF_VIC_VAD47		(ARMPF_VIC_BASE + 0x4BC)		/* Interrupt address storage register47				*/
#define	ARMPF_VIC_VAD48		(ARMPF_VIC_BASE + 0x4C0)		/* Interrupt address storage register48				*/
#define	ARMPF_VIC_VAD49		(ARMPF_VIC_BASE + 0x4C4)		/* Interrupt address storage register49				*/
#define	ARMPF_VIC_VAD50		(ARMPF_VIC_BASE + 0x4C8)		/* Interrupt address storage register50				*/
#define	ARMPF_VIC_VAD51		(ARMPF_VIC_BASE + 0x4CC)		/* Interrupt address storage register51				*/
#define	ARMPF_VIC_VAD52		(ARMPF_VIC_BASE + 0x4D0)		/* Interrupt address storage register52				*/
#define	ARMPF_VIC_VAD53		(ARMPF_VIC_BASE + 0x4D4)		/* Interrupt address storage register53				*/
#define	ARMPF_VIC_VAD54		(ARMPF_VIC_BASE + 0x4D8)		/* Interrupt address storage register54				*/
#define	ARMPF_VIC_VAD55		(ARMPF_VIC_BASE + 0x4DC)		/* Interrupt address storage register55				*/
#define	ARMPF_VIC_VAD56		(ARMPF_VIC_BASE + 0x4E0)		/* Interrupt address storage register56				*/
#define	ARMPF_VIC_VAD57		(ARMPF_VIC_BASE + 0x4E4)		/* Interrupt address storage register57				*/
#define	ARMPF_VIC_VAD58		(ARMPF_VIC_BASE + 0x4E8)		/* Interrupt address storage register58				*/
#define	ARMPF_VIC_VAD59		(ARMPF_VIC_BASE + 0x4EC)		/* Interrupt address storage register59				*/
#define	ARMPF_VIC_VAD60		(ARMPF_VIC_BASE + 0x4F0)		/* Interrupt address storage register60				*/
#define	ARMPF_VIC_VAD61		(ARMPF_VIC_BASE + 0x4F4)		/* Interrupt address storage register61				*/
#define	ARMPF_VIC_VAD62		(ARMPF_VIC_BASE + 0x4F8)		/* Interrupt address storage register62				*/
#define	ARMPF_VIC_VAD63		(ARMPF_VIC_BASE + 0x4FC)		/* Interrupt address storage register63				*/
//#define	ARMPF_VIC_VAD64		(ARMPF_VIC_BASE + 0x500)		/* Interrupt address storage register64				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD65		(ARMPF_VIC_BASE + 0x504)		/* Interrupt address storage register65				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD66		(ARMPF_VIC_BASE + 0x508)		/* Interrupt address storage register66				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD67		(ARMPF_VIC_BASE + 0x50C)		/* Interrupt address storage register67				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD68		(ARMPF_VIC_BASE + 0x510)		/* Interrupt address storage register68				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD69		(ARMPF_VIC_BASE + 0x514)		/* Interrupt address storage register69				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD70		(ARMPF_VIC_BASE + 0x518)		/* Interrupt address storage register70				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD71		(ARMPF_VIC_BASE + 0x51C)		/* Interrupt address storage register71				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD72		(ARMPF_VIC_BASE + 0x520)		/* Interrupt address storage register72				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD73		(ARMPF_VIC_BASE + 0x524)		/* Interrupt address storage register73				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD74		(ARMPF_VIC_BASE + 0x528)		/* Interrupt address storage register74				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD75		(ARMPF_VIC_BASE + 0x52C)		/* Interrupt address storage register75				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD76		(ARMPF_VIC_BASE + 0x530)		/* Interrupt address storage register76				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD77		(ARMPF_VIC_BASE + 0x534)		/* Interrupt address storage register77				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD78		(ARMPF_VIC_BASE + 0x538)		/* Interrupt address storage register78				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD79		(ARMPF_VIC_BASE + 0x53C)		/* Interrupt address storage register79				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD80		(ARMPF_VIC_BASE + 0x540)		/* Interrupt address storage register80				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD81		(ARMPF_VIC_BASE + 0x544)		/* Interrupt address storage register81				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD82		(ARMPF_VIC_BASE + 0x548)		/* Interrupt address storage register82				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD83		(ARMPF_VIC_BASE + 0x54C)		/* Interrupt address storage register83				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD84		(ARMPF_VIC_BASE + 0x550)		/* Interrupt address storage register84				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD85		(ARMPF_VIC_BASE + 0x554)		/* Interrupt address storage register85				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD86		(ARMPF_VIC_BASE + 0x558)		/* Interrupt address storage register86				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD87		(ARMPF_VIC_BASE + 0x55C)		/* Interrupt address storage register87				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD88		(ARMPF_VIC_BASE + 0x560)		/* Interrupt address storage register88				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD89		(ARMPF_VIC_BASE + 0x564)		/* Interrupt address storage register89				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD90		(ARMPF_VIC_BASE + 0x568)		/* Interrupt address storage register90				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD91		(ARMPF_VIC_BASE + 0x56C)		/* Interrupt address storage register91				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD92		(ARMPF_VIC_BASE + 0x570)		/* Interrupt address storage register92				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD93		(ARMPF_VIC_BASE + 0x574)		/* Interrupt address storage register93				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD94		(ARMPF_VIC_BASE + 0x578)		/* Interrupt address storage register94				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD95		(ARMPF_VIC_BASE + 0x57C)		/* Interrupt address storage register95				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD96		(ARMPF_VIC_BASE + 0x580)		/* Interrupt address storage register96				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD97		(ARMPF_VIC_BASE + 0x584)		/* Interrupt address storage register97				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD98		(ARMPF_VIC_BASE + 0x588)		/* Interrupt address storage register98				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD99		(ARMPF_VIC_BASE + 0x58C)		/* Interrupt address storage register99				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD100	(ARMPF_VIC_BASE + 0x590)		/* Interrupt address storage register100				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD101	(ARMPF_VIC_BASE + 0x594)		/* Interrupt address storage register101				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD102	(ARMPF_VIC_BASE + 0x598)		/* Interrupt address storage register102				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD103	(ARMPF_VIC_BASE + 0x59C)		/* Interrupt address storage register103				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD104	(ARMPF_VIC_BASE + 0x5A0)		/* Interrupt address storage register104				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD105	(ARMPF_VIC_BASE + 0x5A4)		/* Interrupt address storage register105				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD106	(ARMPF_VIC_BASE + 0x5A8)		/* Interrupt address storage register106				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD107	(ARMPF_VIC_BASE + 0x5AC)		/* Interrupt address storage register107				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD108	(ARMPF_VIC_BASE + 0x5B0)		/* Interrupt address storage register108				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD109	(ARMPF_VIC_BASE + 0x5B4)		/* Interrupt address storage register109				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD110	(ARMPF_VIC_BASE + 0x5B8)		/* Interrupt address storage register110				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD111	(ARMPF_VIC_BASE + 0x5BC)		/* Interrupt address storage register111				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD112	(ARMPF_VIC_BASE + 0x5C0)		/* Interrupt address storage register112				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD113	(ARMPF_VIC_BASE + 0x5C4)		/* Interrupt address storage register113				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD114	(ARMPF_VIC_BASE + 0x5C8)		/* Interrupt address storage register114				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD115	(ARMPF_VIC_BASE + 0x5CC)		/* Interrupt address storage register115				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD116	(ARMPF_VIC_BASE + 0x5D0)		/* Interrupt address storage register116				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD117	(ARMPF_VIC_BASE + 0x5D4)		/* Interrupt address storage register117				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD118	(ARMPF_VIC_BASE + 0x5D8)		/* Interrupt address storage register118				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD119	(ARMPF_VIC_BASE + 0x5DC)		/* Interrupt address storage register119				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD120	(ARMPF_VIC_BASE + 0x5E0)		/* Interrupt address storage register120				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD121	(ARMPF_VIC_BASE + 0x5E4)		/* Interrupt address storage register121				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD122	(ARMPF_VIC_BASE + 0x5E8)		/* Interrupt address storage register122				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD123	(ARMPF_VIC_BASE + 0x5EC)		/* Interrupt address storage register123				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD124	(ARMPF_VIC_BASE + 0x5F0)		/* Interrupt address storage register124				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD125	(ARMPF_VIC_BASE + 0x5F4)		/* Interrupt address storage register125				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD126	(ARMPF_VIC_BASE + 0x5F8)		/* Interrupt address storage register126				*//* JL-086Not available */
//#define	ARMPF_VIC_VAD127	(ARMPF_VIC_BASE + 0x5FC)		/* Interrupt address storage register127				*//* JL-086Not available */
#define	ARMPF_VIC_PRL0		(ARMPF_VIC_BASE + 0x800)		/* Interrupt priority level storage register0				*/
#define	ARMPF_VIC_PRL1		(ARMPF_VIC_BASE + 0x804)		/* Interrupt priority level storage register1				*/
#define	ARMPF_VIC_PRL2		(ARMPF_VIC_BASE + 0x808)		/* Interrupt priority level storage register2				*/
#define	ARMPF_VIC_PRL3		(ARMPF_VIC_BASE + 0x80C)		/* Interrupt priority level storage register3				*/
#define	ARMPF_VIC_PRL4		(ARMPF_VIC_BASE + 0x810)		/* Interrupt priority level storage register4				*/
#define	ARMPF_VIC_PRL5		(ARMPF_VIC_BASE + 0x814)		/* Interrupt priority level storage register5				*/
#define	ARMPF_VIC_PRL6		(ARMPF_VIC_BASE + 0x818)		/* Interrupt priority level storage register6				*/
#define	ARMPF_VIC_PRL7		(ARMPF_VIC_BASE + 0x81C)		/* Interrupt priority level storage register7				*/
#define	ARMPF_VIC_PRL8		(ARMPF_VIC_BASE + 0x820)		/* Interrupt priority level storage register8				*/
#define	ARMPF_VIC_PRL9		(ARMPF_VIC_BASE + 0x824)		/* Interrupt priority level storage register9				*/
#define	ARMPF_VIC_PRL10		(ARMPF_VIC_BASE + 0x828)		/* Interrupt priority level storage register10				*/
#define	ARMPF_VIC_PRL11		(ARMPF_VIC_BASE + 0x82C)		/* Interrupt priority level storage register11				*/
#define	ARMPF_VIC_PRL12		(ARMPF_VIC_BASE + 0x830)		/* Interrupt priority level storage register12				*/
#define	ARMPF_VIC_PRL13		(ARMPF_VIC_BASE + 0x834)		/* Interrupt priority level storage register13				*/
#define	ARMPF_VIC_PRL14		(ARMPF_VIC_BASE + 0x838)		/* Interrupt priority level storage register14				*/
#define	ARMPF_VIC_PRL15		(ARMPF_VIC_BASE + 0x83C)		/* Interrupt priority level storage register15				*/
#define	ARMPF_VIC_PRL16		(ARMPF_VIC_BASE + 0x840)		/* Interrupt priority level storage register16				*/
#define	ARMPF_VIC_PRL17		(ARMPF_VIC_BASE + 0x844)		/* Interrupt priority level storage register17				*/
#define	ARMPF_VIC_PRL18		(ARMPF_VIC_BASE + 0x848)		/* Interrupt priority level storage register18				*/
#define	ARMPF_VIC_PRL19		(ARMPF_VIC_BASE + 0x84C)		/* Interrupt priority level storage register19				*/
#define	ARMPF_VIC_PRL20		(ARMPF_VIC_BASE + 0x850)		/* Interrupt priority level storage register20				*/
#define	ARMPF_VIC_PRL21		(ARMPF_VIC_BASE + 0x854)		/* Interrupt priority level storage register21				*/
#define	ARMPF_VIC_PRL22		(ARMPF_VIC_BASE + 0x858)		/* Interrupt priority level storage register22				*/
#define	ARMPF_VIC_PRL23		(ARMPF_VIC_BASE + 0x85C)		/* Interrupt priority level storage register23				*/
#define	ARMPF_VIC_PRL24		(ARMPF_VIC_BASE + 0x860)		/* Interrupt priority level storage register24				*/
#define	ARMPF_VIC_PRL25		(ARMPF_VIC_BASE + 0x864)		/* Interrupt priority level storage register25				*/
#define	ARMPF_VIC_PRL26		(ARMPF_VIC_BASE + 0x868)		/* Interrupt priority level storage register26				*/
#define	ARMPF_VIC_PRL27		(ARMPF_VIC_BASE + 0x86C)		/* Interrupt priority level storage register27				*/
#define	ARMPF_VIC_PRL28		(ARMPF_VIC_BASE + 0x870)		/* Interrupt priority level storage register28				*/
#define	ARMPF_VIC_PRL29		(ARMPF_VIC_BASE + 0x874)		/* Interrupt priority level storage register29				*/
#define	ARMPF_VIC_PRL30		(ARMPF_VIC_BASE + 0x878)		/* Interrupt priority level storage register30				*/
#define	ARMPF_VIC_PRL31		(ARMPF_VIC_BASE + 0x87C)		/* Interrupt priority level storage register31				*/
#define	ARMPF_VIC_PRL32		(ARMPF_VIC_BASE + 0x880)		/* Interrupt priority level storage register32				*/
#define	ARMPF_VIC_PRL33		(ARMPF_VIC_BASE + 0x884)		/* Interrupt priority level storage register33				*/
#define	ARMPF_VIC_PRL34		(ARMPF_VIC_BASE + 0x888)		/* Interrupt priority level storage register34				*/
#define	ARMPF_VIC_PRL35		(ARMPF_VIC_BASE + 0x88C)		/* Interrupt priority level storage register35				*/
#define	ARMPF_VIC_PRL36		(ARMPF_VIC_BASE + 0x890)		/* Interrupt priority level storage register36				*/
#define	ARMPF_VIC_PRL37		(ARMPF_VIC_BASE + 0x894)		/* Interrupt priority level storage register37				*/
#define	ARMPF_VIC_PRL38		(ARMPF_VIC_BASE + 0x898)		/* Interrupt priority level storage register38				*/
#define	ARMPF_VIC_PRL39		(ARMPF_VIC_BASE + 0x89C)		/* Interrupt priority level storage register39				*/
#define	ARMPF_VIC_PRL40		(ARMPF_VIC_BASE + 0x8A0)		/* Interrupt priority level storage register40				*/
#define	ARMPF_VIC_PRL41		(ARMPF_VIC_BASE + 0x8A4)		/* Interrupt priority level storage register41				*/
#define	ARMPF_VIC_PRL42		(ARMPF_VIC_BASE + 0x8A8)		/* Interrupt priority level storage register42				*/
#define	ARMPF_VIC_PRL43		(ARMPF_VIC_BASE + 0x8AC)		/* Interrupt priority level storage register43				*/
#define	ARMPF_VIC_PRL44		(ARMPF_VIC_BASE + 0x8B0)		/* Interrupt priority level storage register44				*/
#define	ARMPF_VIC_PRL45		(ARMPF_VIC_BASE + 0x8B4)		/* Interrupt priority level storage register45				*/
#define	ARMPF_VIC_PRL46		(ARMPF_VIC_BASE + 0x8B8)		/* Interrupt priority level storage register46				*/
#define	ARMPF_VIC_PRL47		(ARMPF_VIC_BASE + 0x8BC)		/* Interrupt priority level storage register47				*/
#define	ARMPF_VIC_PRL48		(ARMPF_VIC_BASE + 0x8C0)		/* Interrupt priority level storage register48				*/
#define	ARMPF_VIC_PRL49		(ARMPF_VIC_BASE + 0x8C4)		/* Interrupt priority level storage register49				*/
#define	ARMPF_VIC_PRL50		(ARMPF_VIC_BASE + 0x8C8)		/* Interrupt priority level storage register50				*/
#define	ARMPF_VIC_PRL51		(ARMPF_VIC_BASE + 0x8CC)		/* Interrupt priority level storage register51				*/
#define	ARMPF_VIC_PRL52		(ARMPF_VIC_BASE + 0x8D0)		/* Interrupt priority level storage register52				*/
#define	ARMPF_VIC_PRL53		(ARMPF_VIC_BASE + 0x8D4)		/* Interrupt priority level storage register53				*/
#define	ARMPF_VIC_PRL54		(ARMPF_VIC_BASE + 0x8D8)		/* Interrupt priority level storage register54				*/
#define	ARMPF_VIC_PRL55		(ARMPF_VIC_BASE + 0x8DC)		/* Interrupt priority level storage register55				*/
#define	ARMPF_VIC_PRL56		(ARMPF_VIC_BASE + 0x8E0)		/* Interrupt priority level storage register56				*/
#define	ARMPF_VIC_PRL57		(ARMPF_VIC_BASE + 0x8E4)		/* Interrupt priority level storage register57				*/
#define	ARMPF_VIC_PRL58		(ARMPF_VIC_BASE + 0x8E8)		/* Interrupt priority level storage register58				*/
#define	ARMPF_VIC_PRL59		(ARMPF_VIC_BASE + 0x8EC)		/* Interrupt priority level storage register59				*/
#define	ARMPF_VIC_PRL60		(ARMPF_VIC_BASE + 0x8F0)		/* Interrupt priority level storage register60				*/
#define	ARMPF_VIC_PRL61		(ARMPF_VIC_BASE + 0x8F4)		/* Interrupt priority level storage register61				*/
#define	ARMPF_VIC_PRL62		(ARMPF_VIC_BASE + 0x8F8)		/* Interrupt priority level storage register62				*/
#define	ARMPF_VIC_PRL63		(ARMPF_VIC_BASE + 0x8FC)		/* Interrupt priority level storage register63				*/
//#define	ARMPF_VIC_PRL64		(ARMPF_VIC_BASE + 0x900)		/* Interrupt priority level storage register64				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL65		(ARMPF_VIC_BASE + 0x904)		/* Interrupt priority level storage register65				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL66		(ARMPF_VIC_BASE + 0x908)		/* Interrupt priority level storage register66				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL67		(ARMPF_VIC_BASE + 0x90C)		/* Interrupt priority level storage register67				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL68		(ARMPF_VIC_BASE + 0x910)		/* Interrupt priority level storage register68				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL69		(ARMPF_VIC_BASE + 0x914)		/* Interrupt priority level storage register69				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL70		(ARMPF_VIC_BASE + 0x918)		/* Interrupt priority level storage register70				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL71		(ARMPF_VIC_BASE + 0x91C)		/* Interrupt priority level storage register71				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL72		(ARMPF_VIC_BASE + 0x920)		/* Interrupt priority level storage register72				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL73		(ARMPF_VIC_BASE + 0x924)		/* Interrupt priority level storage register73				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL74		(ARMPF_VIC_BASE + 0x928)		/* Interrupt priority level storage register74				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL75		(ARMPF_VIC_BASE + 0x92C)		/* Interrupt priority level storage register75				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL76		(ARMPF_VIC_BASE + 0x930)		/* Interrupt priority level storage register76				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL77		(ARMPF_VIC_BASE + 0x934)		/* Interrupt priority level storage register77				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL78		(ARMPF_VIC_BASE + 0x938)		/* Interrupt priority level storage register78				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL79		(ARMPF_VIC_BASE + 0x93C)		/* Interrupt priority level storage register79				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL80		(ARMPF_VIC_BASE + 0x940)		/* Interrupt priority level storage register80				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL81		(ARMPF_VIC_BASE + 0x944)		/* Interrupt priority level storage register81				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL82		(ARMPF_VIC_BASE + 0x948)		/* Interrupt priority level storage register82				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL83		(ARMPF_VIC_BASE + 0x94C)		/* Interrupt priority level storage register83				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL84		(ARMPF_VIC_BASE + 0x950)		/* Interrupt priority level storage register84				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL85		(ARMPF_VIC_BASE + 0x954)		/* Interrupt priority level storage register85				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL86		(ARMPF_VIC_BASE + 0x958)		/* Interrupt priority level storage register86				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL87		(ARMPF_VIC_BASE + 0x95C)		/* Interrupt priority level storage register87				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL88		(ARMPF_VIC_BASE + 0x960)		/* Interrupt priority level storage register88				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL89		(ARMPF_VIC_BASE + 0x964)		/* Interrupt priority level storage register89				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL90		(ARMPF_VIC_BASE + 0x968)		/* Interrupt priority level storage register90				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL91		(ARMPF_VIC_BASE + 0x96C)		/* Interrupt priority level storage register91				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL92		(ARMPF_VIC_BASE + 0x970)		/* Interrupt priority level storage register92				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL93		(ARMPF_VIC_BASE + 0x974)		/* Interrupt priority level storage register93				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL94		(ARMPF_VIC_BASE + 0x978)		/* Interrupt priority level storage register94				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL95		(ARMPF_VIC_BASE + 0x97C)		/* Interrupt priority level storage register95				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL96		(ARMPF_VIC_BASE + 0x980)		/* Interrupt priority level storage register96				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL97		(ARMPF_VIC_BASE + 0x984)		/* Interrupt priority level storage register97				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL98		(ARMPF_VIC_BASE + 0x988)		/* Interrupt priority level storage register98				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL99		(ARMPF_VIC_BASE + 0x98C)		/* Interrupt priority level storage register99				*//* JL-086Not available */
//#define	ARMPF_VIC_PRL100	(ARMPF_VIC_BASE + 0x990)		/* Interrupt priority level storage register100			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL101	(ARMPF_VIC_BASE + 0x994)		/* Interrupt priority level storage register101			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL102	(ARMPF_VIC_BASE + 0x998)		/* Interrupt priority level storage register102			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL103	(ARMPF_VIC_BASE + 0x99C)		/* Interrupt priority level storage register103			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL104	(ARMPF_VIC_BASE + 0x9A0)		/* Interrupt priority level storage register104			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL105	(ARMPF_VIC_BASE + 0x9A4)		/* Interrupt priority level storage register105			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL106	(ARMPF_VIC_BASE + 0x9A8)		/* Interrupt priority level storage register106			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL107	(ARMPF_VIC_BASE + 0x9AC)		/* Interrupt priority level storage register107			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL108	(ARMPF_VIC_BASE + 0x9B0)		/* Interrupt priority level storage register108			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL109	(ARMPF_VIC_BASE + 0x9B4)		/* Interrupt priority level storage register109			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL110	(ARMPF_VIC_BASE + 0x9B8)		/* Interrupt priority level storage register110			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL111	(ARMPF_VIC_BASE + 0x9BC)		/* Interrupt priority level storage register111			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL112	(ARMPF_VIC_BASE + 0x9C0)		/* Interrupt priority level storage register112			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL113	(ARMPF_VIC_BASE + 0x9C4)		/* Interrupt priority level storage register113			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL114	(ARMPF_VIC_BASE + 0x9C8)		/* Interrupt priority level storage register114			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL115	(ARMPF_VIC_BASE + 0x9CC)		/* Interrupt priority level storage register115			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL116	(ARMPF_VIC_BASE + 0x9D0)		/* Interrupt priority level storage register116			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL117	(ARMPF_VIC_BASE + 0x9D4)		/* Interrupt priority level storage register117			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL118	(ARMPF_VIC_BASE + 0x9D8)		/* Interrupt priority level storage register118			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL119	(ARMPF_VIC_BASE + 0x9DC)		/* Interrupt priority level storage register119			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL120	(ARMPF_VIC_BASE + 0x9E0)		/* Interrupt priority level storage register120			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL121	(ARMPF_VIC_BASE + 0x9E4)		/* Interrupt priority level storage register121			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL122	(ARMPF_VIC_BASE + 0x9E8)		/* Interrupt priority level storage register122			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL123	(ARMPF_VIC_BASE + 0x9EC)		/* Interrupt priority level storage register123			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL124	(ARMPF_VIC_BASE + 0x9F0)		/* Interrupt priority level storage register124			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL125	(ARMPF_VIC_BASE + 0x9F4)		/* Interrupt priority level storage register125			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL126	(ARMPF_VIC_BASE + 0x9F8)		/* Interrupt priority level storage register126			*//* JL-086Not available */
//#define	ARMPF_VIC_PRL127	(ARMPF_VIC_BASE + 0x9FC)		/* Interrupt priority level storage register127			*//* JL-086Not available */
#define	ARMPF_VIC_TCR		(ARMPF_VIC_BASE + 0xC00)		/* Test mode selection register					*/
#define	ARMPF_VIC_TICR		(ARMPF_VIC_BASE + 0xC04)		/* Test mode interrupt input control register		*/
#define	ARMPF_VIC_TACR		(ARMPF_VIC_BASE + 0xC08)		/* Test mode interrupt address control register	*/
#define	ARMPF_VIC_IOS		(ARMPF_VIC_BASE + 0xC0C)		/* Interrupt request status register				*/
#define	ARMPF_VIC_VAOS		(ARMPF_VIC_BASE + 0xC10)		/* Interrupt request address status register	*/

#endif
/*-------end of file----------------------------------------------------------------------------------------*/

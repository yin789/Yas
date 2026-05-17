/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ARMPF.h : MercurySoC peripheral base header												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		ARMPeripheral operation basic command definition header file											*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		Rev.1.00 : 2013.07.14  Y.Tanaka		First edition													*/
/*																									*/
/****************************************************************************************************/
#ifndef	_ARMPF_H_
#define	_ARMPF_H_


/*--------------------------------------------------------------------------------------------------*/
/*		Basic operations																					*/
/*--------------------------------------------------------------------------------------------------*/
#define	ARMPF_REG_WRITE( reg, data )	(*(VULONG*)(reg) = (data))
#define	ARMPF_REG_READ( reg )			(*(VULONG*)(reg))
#define	ARMPF_REG_WRITE_VERIFY( reg, data )	do{\
						ARMPF_REG_WRITE( reg, data );\
						while(ARM_REG_READ( (reg) ) == (data))}
#define	ARMPF_REG_BITSET( reg, bit )	ARMPF_REG_WRITE( reg, (ARMPF_REG_READ(reg) | (1<<bit) ))
#define	ARMPF_REG_BITCLR( reg, bit )	ARMPF_REG_WRITE( reg, (ARMPF_REG_READ(reg) & (~(1<<bit))) )
#define	ARMPF_REG_VALSET( reg, data )	ARMPF_REG_WRITE( reg, (ARMPF_REG_READ(reg) | (data) ) )		/* <S152> */
#define	ARMPF_REG_VALCLR( reg, data )	ARMPF_REG_WRITE( reg, (ARMPF_REG_READ(reg) & (~data)) )

/*--------------------------------------------------------------------------------------------------*/
/*		bit definition																					*/
/*--------------------------------------------------------------------------------------------------*/
#define	ARMPF_LONGALL1	0xFFFFFFFF
#define	ARMPF_LONGALL0	0x00000000
#define	ARMPF_WORDALL1	0xFFFF
#define	ARMPF_WORDALL0	0x0000
#define	ARMPF_BYTEALL1	0xFF
#define	ARMPF_BYTEALL0	0x00
/*--------------------------------------------------------------------------------------------------*/
#define	ARMPF_BIT00		0x00000001
#define	ARMPF_BIT01		0x00000002
#define	ARMPF_BIT02		0x00000004
#define	ARMPF_BIT03		0x00000008
#define	ARMPF_BIT04		0x00000010
#define	ARMPF_BIT05		0x00000020
#define	ARMPF_BIT06		0x00000040
#define	ARMPF_BIT07		0x00000080
/*--------------------------------------------------------------------------------------------------*/
#define	ARMPF_BIT08		0x00000100
#define	ARMPF_BIT09		0x00000200
#define	ARMPF_BIT10		0x00000400
#define	ARMPF_BIT11		0x00000800
#define	ARMPF_BIT12		0x00001000
#define	ARMPF_BIT13		0x00002000
#define	ARMPF_BIT14		0x00004000
#define	ARMPF_BIT15		0x00008000
/*--------------------------------------------------------------------------------------------------*/
#define	ARMPF_BIT16		0x00010000
#define	ARMPF_BIT17		0x00020000
#define	ARMPF_BIT18		0x00040000
#define	ARMPF_BIT19		0x00080000
#define	ARMPF_BIT20		0x00100000
#define	ARMPF_BIT21		0x00200000
#define	ARMPF_BIT22		0x00400000
#define	ARMPF_BIT23		0x00800000
/*--------------------------------------------------------------------------------------------------*/
#define	ARMPF_BIT24		0x01000000
#define	ARMPF_BIT25		0x02000000
#define	ARMPF_BIT26		0x04000000
#define	ARMPF_BIT27		0x08000000
#define	ARMPF_BIT28		0x10000000
#define	ARMPF_BIT29		0x20000000
#define	ARMPF_BIT30		0x40000000
#define	ARMPF_BIT31		0x80000000
/*--------------------------------------------------------------------------------------------------*/
#define	BIT_SET			1
#define	BIT_CLR			0
/*--------------------------------------------------------------------------------------------------*/

#endif
/*-------end of file--------------------------------------------------------------------------------*/

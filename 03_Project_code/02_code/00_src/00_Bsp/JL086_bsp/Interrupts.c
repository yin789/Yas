/************************************************************************************************************/
/*																											*/
/*																											*/
/*		Interrupt.c : Mercury Interrupt-related processing																*/
/*																											*/
/*																											*/
/************************************************************************************************************/
/*																											*/
/*																											*/
/*		Interrupt setting source file																			*/
/*																											*/
/*																											*/
/************** Copyright (C) Yaskawa Electric Corporation **************************************************/
/*																											*/
/*		Rev.1.00 : 2013.07.14  Y.Tanaka		First edition															*/
/*																											*/
/************************************************************************************************************/
#include	"BaseDef.h"
#include	"ARMPF_VIC.h"
#include	"Interrupt.h"
#include	"Klib.h"
#include	"HwDeviceIf.h"

/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
void 	InitInterrupt( void );							/* Interrupt-related initialization processing				*/
void 	InitInterruptStart( void );						/* Interrupt start						*/
void 	IntIgnore( void );									/* Infinite loop interrupt					*/
void 	Common_IRQHandler0( void );						/* ARMPF_VIC_ISC0	*/
void 	Common_IRQHandler1( void );						/* ARMPF_VIC_ISC1	*/
void 	ScanAIntHandler( void );							/* ScanAInterrupt pre-processing				*/
void 	ScanBIntHandler( void );							/* ScanBInterrupt pre-processing				*/
void 	ScanCIntHandler( void );							/* ScanCInterrupt pre-processing				*/
static	ULONG SearchIRQReqBit( VULONG value );			/* Interrupt source detection process					*/
BOOL	GetIntUDLStatus( UCHAR num );						/* INTUDL*Interrupt request acquisition processing			*///<S0C7>
void 	ReqIntUDLStatusClear( UCHAR num );				/* INTUDL*Interrupt request clear processing			*///<S0C7>

/****************************************************************************************************/
/*																									*/
/*		External functionexternDeclaration																			*/
/*																									*/
/****************************************************************************************************/
extern void SysScanAInt( void );
extern void SysScanBInt( void );
extern void SysScanCInt( void );
extern void __SET_VE( void );


/****************************************************************************************************/
/*																									*/
/*		VARIABLE																					*/
/*																									*/
/****************************************************************************************************/

#ifdef _MERCURY_PLATFORM_	/* Encrypted format support <S146> */
/****************************************************************************************************/
/*																									*/
/*		Interrupt-related initialization processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:VICregister setting, interrupt function set													*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void InitInterrupt(void)
{
	LONG i,j;
	ULONG PlsRegValue[2] = {0};
	ULONG EdgcRegValue[4] = {0};
	ULONG LvlRegValue[4] = {0};

//	__DI();

//	InitializeVIC();
	/* Clear interrupt enable/Edge detection bit clear setting	*/
	for(i=0;i<2;i++)
	{
		ARMPF_REG_WRITE( (ARMPF_VIC_IEC0 + (i*4)), ARMPF_LONGALL1 );
		ARMPF_REG_WRITE( (ARMPF_VIC_PIC0 + (i*4)), ARMPF_LONGALL1 );
	}
	/* Enable all interruptsIRQ	*/
	ARMPF_REG_WRITE( ARMPF_VIC_ISL0, ARMPF_LONGALL0 );
	ARMPF_REG_WRITE( ARMPF_VIC_ISL1, ARMPF_LONGALL0 );

	/* Read table dataVIC	*/
	/* Interrupt detection method setting	*/
	for(i=0;i<2;i++)
	{
		for(j=0;j<32;j++)
		{
			PlsRegValue[i] |= (InitIntVecTbl[ j+i*32 ].PLS << j);
		}
		ARMPF_REG_WRITE( (ARMPF_VIC_PLS0 + (i*4)) , PlsRegValue[i] );		/* Level/Edge setting	*/
	}
}
#else	/* _MERCURY_PLATFORM_ */	/* Encrypted format support <S146> */

#pragma arm section code = "INIT_VECT_CODE"
/****************************************************************************************************/
/*																									*/
/*		Interrupt-related initialization processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:VICregister setting, interrupt function set													*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void InitInterrupt(void)
{
	LONG i,j;
	ULONG PlsRegValue[2] = {0};
	ULONG EdgcRegValue[4] = {0};
	ULONG LvlRegValue[4] = {0};

//	__DI();

//	InitializeVIC();
	/* Clear interrupt enable/Edge detection bit clear setting	*/
	for(i=0;i<2;i++)
	{
		ARMPF_REG_WRITE( (ARMPF_VIC_IEC0 + (i*4)), ARMPF_LONGALL1 );
		ARMPF_REG_WRITE( (ARMPF_VIC_PIC0 + (i*4)), ARMPF_LONGALL1 );
	}
	/* Enable all interruptsIRQ	*/
	ARMPF_REG_WRITE( ARMPF_VIC_ISL0, ARMPF_LONGALL0 );
	ARMPF_REG_WRITE( ARMPF_VIC_ISL1, ARMPF_LONGALL0 );

	/* Read table dataVIC	*/
	/* Interrupt detection method setting	*/
	for(i=0;i<2;i++)
	{
		for(j=0;j<32;j++)
		{
			PlsRegValue[i] |= (InitIntVecTbl[ j+i*32 ].PLS << j);
		}
		ARMPF_REG_WRITE( (ARMPF_VIC_PLS0 + (i*4)) , PlsRegValue[i] );		/* Level/Edge setting	*/
	}

	for(i=0;i<4;i++)
	{
		for(j=0;j<16;j++)
		{
			if(InitIntVecTbl[j+i*16].PLS == ARMPF_VIC_EDGE)
			{/*For edge detection*/
				EdgcRegValue[i] |= (InitIntVecTbl[ j+i*16 ].EDGC << (j*2));
			}
			else if(InitIntVecTbl[j+i*16].PLS == ARMPF_VIC_LVL)
			{/*For level detection*/
				LvlRegValue[i] |= (InitIntVecTbl[ j+i*16 ].EDGC << (j*2));
			}
		}
		ARMPF_REG_WRITE( (ARMPF_VIC_EDGC0 + (i*4)) , EdgcRegValue[i] );		/* Edge detection method setting	*/
		ARMPF_REG_WRITE( (ARMPF_VIC_LVLC0 + (i*4)) , LvlRegValue[i] );		/* Level detection method setting	*/
	}

	/* Cancel all interrupt priority level masks	*/
	ARMPF_REG_WRITE( ARMPF_VIC_PRLC, ARMPF_WORDALL1 );

	/* Interrupt priority/Interrupt handler setting	*/
	for(i=0;i<64;i++)
	{
		ARMPF_REG_WRITE( (ARMPF_VIC_PRL0 + (i*4)), IntVecTbl[i].Priority );	/* Interrupt priority setting	*/
		ARMPF_REG_WRITE( (ARMPF_VIC_VAD0 + (i*4)), (ULONG)(InitIntVecTbl[i].RegHandler) );	/* Interrupt handler setting	*/
	}

	/* Enable interrupt/Set mask	*/
//	ARMPF_REG_WRITE( ARMPF_VIC_IEN0, 0x00000007 );	/* UDLEnable only interrupt for debug */
//	ARMPF_REG_WRITE( ARMPF_VIC_IEN0, 0x003C0007 );	/* UDL+IntervalTimerEnable only interrupt for debug */
//	ARMPF_REG_WRITE( ARMPF_VIC_IEN0, 0x003C0000 );	/* IntervalTimerEnable only interrupt for debug */
//	ARMPF_REG_WRITE( ARMPF_VIC_IEN1, 0x0FDE00C0 );
	ARMPF_REG_WRITE( ARMPF_VIC_IEN0, ARMPF_LONGALL0 );
	ARMPF_REG_WRITE( ARMPF_VIC_IEN1, ARMPF_LONGALL0 );

	__SET_VE();		/* VICEnabled			*/
}


/****************************************************************************************************/
/*																									*/
/*		Interrupt start processing																			*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: After clearing the interrupt factor, interrupt is enabled.													*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void InitInterruptStart(void)
{


	/* Set interrupt edge detection bit clear	*/
	ARMPF_REG_WRITE( ARMPF_VIC_PIC0, ARMPF_LONGALL1 );
	ARMPF_REG_WRITE( ARMPF_VIC_PIC1, ARMPF_LONGALL1 );		/*<S008>*/

	/* CP15 Interrupt enable	*/
	__EI();



	/* Set interrupt edge detection bit clear	*/
	ARMPF_REG_WRITE( ARMPF_VIC_PIC0, ARMPF_LONGALL1 );
	ARMPF_REG_WRITE( ARMPF_VIC_PIC1, ARMPF_LONGALL1 );		/*<S008>*/

	/* Enable interrupt/Set mask	*//* VIC_IEN0, immediatelyScanwill start moving, soVIC_IEN1first <S008>*/
#if 0	/* <S150> USBDo not use interrupt(Polling) */
	ARMPF_REG_WRITE( ARMPF_VIC_IEN1, 0x000000C0 );				/* USBOnly interrupts enabled*/
#endif
	ARMPF_REG_WRITE( ARMPF_VIC_IEN0, 0x00000007 | (1<<29) );	/* UDLInterrupt andI2CEnable only interrupt for debug */

}

/****************************************************************************************************/
/*																									*/
/*		Infinite loop																					*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: Executed by unspecified interrupt.															*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void IntIgnore(void)
{
	while(1);
}


#pragma arm section code = "VECT_CODE"
/****************************************************************************************************/
/*																									*/
/*		ARMPF_VIC_ISC0														*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:ARMPF_VIC_ISC0and call handler					*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void Common_IRQHandler0( void )
{
	ULONG	IRQNumber;
	ULONG	priority;
	ULONG	mask;

//IRQHandlerdisables interrupts, so it is unnecessary
//	/* __DI */

	/* Look for where the interrupt factor bit is setIRQNumber */
	IRQNumber = SearchIRQReqBit(ARMPF_REG_READ(ARMPF_VIC_ISC0));	/* ARMPF_VIC_ISC0is set */
	if( IRQNumber < 32 )		/* ARMPF_VIC_ISC0If an interrupt factor is found in */
	{
		/* Clear interrupt source */
		ARMPF_REG_WRITE( ARMPF_VIC_PIC0, (0x1 << IRQNumber) );

		/* Mask interrupts with the same priority or lower */
		priority = (0x1 << IntVecTbl[IRQNumber].Priority);
		mask = ( priority ^ (-priority) );	/* Set bits with the same priority or lower		*/
		ARMPF_REG_WRITE( ARMPF_VIC_PRLM, mask );

		__EI();
		IntVecTbl[IRQNumber].IntHandler();		/* Interrupt handler call		*/
		__DI();

		/* Mask interrupts with the same priority or lower */
		ARMPF_REG_WRITE( ARMPF_VIC_PRLC, mask );
	}

	/* Notification of end of interrupt processing(HVADummy light) */
	ARMPF_REG_WRITE( ARMPF_VIC_HVA, IRQNumber );
	__EI();

}


/****************************************************************************************************/
/*																									*/
/*		ARMPF_VIC_ISC1														*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:ARMPF_VIC_ISC1and call handler					*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void Common_IRQHandler1( void )
{
	ULONG	IRQNumber;
	ULONG	priority;
	ULONG	mask;

//IRQHandlerdisables interrupts, so it is unnecessary
//	/* __DI */

	/* Look for where the interrupt factor bit is setIRQNumber */
	IRQNumber = SearchIRQReqBit(ARMPF_REG_READ(ARMPF_VIC_ISC1));	/* ARMPF_VIC_ISC1is set */
	if( IRQNumber < 32 )		/* ARMPF_VIC_ISC1If an interrupt factor is found in */
	{
		/* Clear interrupt source */
		ARMPF_REG_WRITE( ARMPF_VIC_PIC1, (0x1 << IRQNumber) );

		/* Mask interrupts with the same priority or lower */
		priority = (0x1 << IntVecTbl[IRQNumber+32].Priority);		/*<S00B>*/
		mask = ( priority ^ (-priority) );	/* Set bits with the same priority or lower		*/
		ARMPF_REG_WRITE( ARMPF_VIC_PRLM, mask );

		__EI();
		IntVecTbl[IRQNumber+32].IntHandler();		/* Interrupt handler call		*/		/*<S008>*/
		__DI();

		/* Mask interrupts with the same priority or lower */
		ARMPF_REG_WRITE( ARMPF_VIC_PRLC, mask );
	}

	/* Notification of end of interrupt processing(HVADummy light) */
	ARMPF_REG_WRITE( ARMPF_VIC_HVA, IRQNumber );
	__EI();

}


/****************************************************************************************************/
/*																									*/
/*		ScanAInterrupt pre-processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:ScanAClear interrupt source/Pre- and post-processing such as masking										*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void ScanAIntHandler( void )
{
//	ULONG	priority;
//	ULONG	mask;

	HALdrv_OutputDebugSP( INTERRUPT_ScanA );		/* Change to port output in interrupt branch processing<S042> */

//IRQHandlerdisables interrupts, so it is unnecessary
//	/* __DI */

	/* Clear interrupt source */
	ARMPF_REG_WRITE( ARMPF_VIC_PIC0, 0x01 );

	/* Mask interrupts with the same priority or lower *//* Delete,  <S0E2> */
//	priority = (0x1 << IntVecTbl[0].Priority);
//	mask = ( priority ^ (-priority) );	/* Set bits with the same priority or lower		*/
//	ARMPF_REG_WRITE( ARMPF_VIC_PRLM, mask );

//	__EI(); /* <S01E> */
	SysScanAInt();		/* Interrupt handler call		*/
//	__DI(); /* <S01E> */

	/* Mask interrupts with the same priority or lower *//* <S0E2> */
//	ARMPF_REG_WRITE( ARMPF_VIC_PRLC, mask );

	/* Notification of end of interrupt processing(HVADummy light) */
	ARMPF_REG_WRITE( ARMPF_VIC_HVA, 0 );
//	__EI(); /* <S01E> */

	HALdrv_ClearDebugSP( INTERRUPT_ScanA );		/* Change to port output in interrupt branch processing<S042> */

}


/****************************************************************************************************/
/*																									*/
/*		ScanBInterrupt pre-processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:ScanBClear interrupt source/Pre- and post-processing such as masking										*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void ScanBIntHandler( void )
{
	ULONG	priority;
	ULONG	mask;

	HALdrv_OutputDebugSP( INTERRUPT_ScanB );		/* Change to port output in interrupt branch processing<S042> */

//IRQHandlerdisables interrupts, so it is unnecessary
//	/* __DI */

	/* Clear interrupt source */
	ARMPF_REG_WRITE( ARMPF_VIC_PIC0, 0x02 );

	/* Mask interrupts with the same priority or lower */
	priority = (0x1 << IntVecTbl[1].Priority);
	mask = ( priority ^ (-priority) );	/* Set bits with the same priority or lower		*/
	ARMPF_REG_WRITE( ARMPF_VIC_PRLM, mask );

	__EI();
	SysScanBInt();		/* Interrupt handler call		*/
	__DI();

	/* Mask interrupts with the same priority or lower */
	ARMPF_REG_WRITE( ARMPF_VIC_PRLC, mask );

	/* Notification of end of interrupt processing(HVADummy light) */
	ARMPF_REG_WRITE( ARMPF_VIC_HVA, 1 );
//	__EI(); /* <S01E> */

	HALdrv_ClearDebugSP( INTERRUPT_ScanB );		/* Change to port output in interrupt branch processing<S042> */

}


/****************************************************************************************************/
/*																									*/
/*		ScanCInterrupt pre-processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:ScanCClear interrupt source/Pre- and post-processing such as masking										*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void ScanCIntHandler( void )
{
	ULONG	priority;
	ULONG	mask;

#if CSW_NETIF_TYPE	!= NETIF_ANLGPLS_TYPE
	HALdrv_OutputDebugSP( INTERRUPT_ScanC );		/* Change to port output in interrupt branch processing<S042> */
#endif	//#if CSW_NETIF_TYPE	!= NETIF_ANLGPLS_TYPE

//IRQHandlerdisables interrupts, so it is unnecessary
//	/* __DI */

	/* Clear interrupt source */
	ARMPF_REG_WRITE( ARMPF_VIC_PIC0, 0x04 );

	/* Mask interrupts with the same priority or lower */
	priority = (0x1 << IntVecTbl[2].Priority);
	mask = ( priority ^ (-priority) );	/* Set bits with the same priority or lower		*/
	ARMPF_REG_WRITE( ARMPF_VIC_PRLM, mask );

	__EI();
	SysScanCInt();		/* Interrupt handler call		*/
	__DI();

	/* Mask interrupts with the same priority or lower */
	ARMPF_REG_WRITE( ARMPF_VIC_PRLC, mask );

	/* Notification of end of interrupt processing(HVADummy light) */
	ARMPF_REG_WRITE( ARMPF_VIC_HVA, 2 );
//	__EI(); /* <S01E> */

#if CSW_NETIF_TYPE	!= NETIF_ANLGPLS_TYPE
	HALdrv_ClearDebugSP( INTERRUPT_ScanC );		/* Change to port output in interrupt branch processing<S042> */
#endif	//#if CSW_NETIF_TYPE	!= NETIF_ANLGPLS_TYPE

}


/****************************************************************************************************/
/*																									*/
/*		Interrupt source detection process																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:ARMPF_VIC_ISCx(x=0,1)							*/
/*		Argument:ARMPF_VIC_ISCx																	*/
/*		Return value:ARMPF_VIC_ISCx,(0~31)										*/
/*				If nothing is standing32															*/
/*																									*/
/****************************************************************************************************/
static	ULONG SearchIRQReqBit(VULONG value)
{
	ULONG	rc;
	
	rc = (value & (-value)) - 1;
    rc = (rc & 0x55555555) + (rc >> 1 & 0x55555555);
    rc = (rc & 0x33333333) + (rc >> 2 & 0x33333333);
    rc = (rc & 0x0f0f0f0f) + (rc >> 4 & 0x0f0f0f0f);
    rc = (rc & 0x00ff00ff) + (rc >> 8 & 0x00ff00ff);
    return (rc & 0x0000ffff) + (rc >>16 & 0x0000ffff);
}

/****************************************************************************************************/
/*																									*/
/*	INTUDL*Interrupt request acquisition processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: From the interrupt input status registerUDL							*/
/*		Argument: None																					*/
/*		Return value:TRUE or FALSE																		*/
/*																									*/
/****************************************************************************************************/
BOOL GetIntUDLStatus(UCHAR num)
{
	BOOL	rc;
	VULONG	sts;
	
	sts = ARMPF_REG_READ(ARMPF_VIC_RAIS0);
	if((sts >> num) & 0x00000001)
	{/* SyncStatus	*/
		rc = TRUE;
	}
	else
	{/* UnSyncStatus	*/
		rc = FALSE;
	}
	return rc;
}

/****************************************************************************************************/
/*																									*/
/*	INTUDL*Interrupt request clear processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:UDLinterrupt																*/
/*		Argument: None																					*/
/*		Return value:TRUE or FALSE																		*/
/*																									*/
/****************************************************************************************************/
void ReqIntUDLStatusClear(UCHAR num)
{
	VULONG	sts;
	
	sts = ARMPF_REG_READ(ARMPF_VIC_PIC0);
	sts = (sts | (0x01 << num));
	ARMPF_REG_WRITE(ARMPF_VIC_PIC0, sts);
	
	return;
}
#endif /* _MERCURY_PLATFORM_ */	/* Encrypted format support <S146> */
/*-------end of file----------------------------------------------------------------------------------------*/

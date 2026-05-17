/************************************************************************************************************/
/*																											*/
/*																											*/
/*		JL086Core.c : Mercury CortexR4FRelated processing																*/
/*																											*/
/*																											*/
/************************************************************************************************************/
/*																											*/
/*																											*/
/*		CortexR4FSetting source file																			*/
/*																											*/
/*																											*/
/************** Copyright (C) Yaskawa Electric Corporation **************************************************/
/*																											*/
/*		Rev.1.00 : 2013.07.31  Y.Tanaka		First edition															*/
/*																											*/
/************************************************************************************************************/
#include	"BaseDef.h"
#include	"ARMPF_SCU.h"
#include	"ARMPF_VIC.h"
#include	"Interrupt.h"
#ifndef _MERCURY_PLATFORM_		/*<S190>*/
#include	"MercuryGlobals.h"			/* <S044> */
#include	"SerialEncIf.h"		/* <S076> */
#endif	/*_MERCURY_PLATFORM_*/
#ifdef SVFIF_CMDOPT		/* for debug EtherCATIntegrated	*/		/*<S008>*/
#include	"ARMPF_SRAM.h"
#endif		//#ifdef SVFIF_CMDOPT		/* for debug EtherCATIntegrated	*/		/*<S008>*/


/****************************************************************************************************/
/*																									*/
/*		External functionexternDeclaration																			*/
/*																									*/
/****************************************************************************************************/
extern void InitSFlashSoC( void );
#ifdef SVFIF_CMDOPT		/* for debug EtherCATIntegrated	*/		/*<S008>*/
extern void InitSRAMMemc( void );							/* ROM/SRAMController-related initialization processing	*/
#endif		//#ifdef SVFIF_CMDOPT		/* for debug EtherCATIntegrated	*/		/*<S008>*/


/****************************************************************************************************/
/*																									*/
/*		PROTOTYPE																					*/
/*																									*/
/****************************************************************************************************/
void InitCorePeripheral( void );							/* JL086Initialization main processing				*/
static void InitSCU( void );								/* SCURegister initialization processing				*/
static void InitScuClock( void );						/* SCUClock-related initialization processing			*/
static void InitScuUDLReg( void );						/* SCU UDLRelated initialization processing				*/
static void InitScuINTCReg( void );						/* SCUInterrupt-related initialization processing				*/
static void InitScustack( void );						/* SCUStack area initialization processing			*//*<S127>*/
#ifdef	_PCIE_USE_
static void InitScuPCIeReg( void );						/* SCU PCIeRelated initialization processing				*/
#endif
#ifdef	_GBETHER_USE_
static void InitScuGbEtherReg( void );					/* SCU GbEtherRelated initialization processing			*/
#endif
#ifndef _MERCURY_PLATFORM_			/*<S190>*/
void ScuSoftReset( AXIS_HANDLE	*AxisHandle );			/* JL086Soft reset processing		<S044>		*//* <S076> */
#endif	/*_MERCURY_PLATFORM_*/

/****************************************************************************************************/
/*																									*/
/*		VARIABLE																					*/
/*																									*/
/****************************************************************************************************/


/****************************************************************************************************/
/*																									*/
/*		JL086CPUPeripheral initialization processing																*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:CortexR4Peripheral initialization															*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void InitCorePeripheral( void )
{

	/* Initialize SCU */
	InitSCU();

	/* Initialize SerialFlashIF */
	InitSFlashSoC();

#ifdef SVFIF_CMDOPT		/* for debug EtherCATIntegrated	*/		/*<S008>*/
	/* Initialize ROM/SRAM */
	InitSRAMMemc();
#endif		//#ifdef SVFIF_CMDOPT		/* for debug EtherCATIntegrated	*/		/*<S008>*/

#ifndef _MERCURY_PLATFORM_			/*<S190>*/
	/* Initialize stack *//* <S127> */
	InitScustack();

	/* Initialize VIC */
	InitInterrupt();
#endif /*_MERCURY_PLATFORM_*/
}


/****************************************************************************************************/
/*																									*/
/*		SCUInitialization process																				*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:SCU(By module)																*/
/*				REMAPis to be performed, do it within this function												*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
static void InitSCU( void )
{
	/* Core ID check */
//	while(SCU_GET_ID() != JL086ID)
	if( JL086ID != SCU_GET_ID() )		/* ES4or later<S191> */
	{/* Insert alarm processing	*/
		;
	}

//	SCU_GET_VER();		/* ES4or later version <S191> */
	
	/* set module clock */
	InitScuClock();
	
	/* UDLRelated settings		*/
	InitScuUDLReg();

#ifndef _MERCURY_PLATFORM_			/*<S190>*/
	/* Interrupt related settings		*/
	InitScuINTCReg();
#endif /*_MERCURY_PLATFORM_*/

#ifdef	_PCIE_USE_
	/* PCIeRelated settings		*/
	InitScuPCIeReg();
#endif

#ifdef	_GBETHER_USE_
	/* PCIeRelated settings		*/
	InitScuGbEtherReg();
#endif


	return;

}


/****************************************************************************************************/
/*																									*/
/*		SCUClock-related initialization processing																	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:SSPRO7_CLK_EN, SSPRO7_CLK_SEL													*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
static void InitScuClock( void )
{

	/* SSPRO7_CLK_EN			*/
	SCU_CLK_START( UDL_NET_CLK );									/* M3UDLClock supply			*/
	SCU_CLK_START( UDL_SERVO_CLK );									/* ServoUDLClock supply			*/
#if (SVFSEL_ADC_USE == 1)||(SVFSEL_ONLY_OH_ADC_USE == 1)			/* <S0E0>:SVFSEL_ONLY_OH_ADC_USEaddition */
	SCU_CLK_START( ADC_CLK );										/* ADCClock supply				*/
#else
	SCU_CLK_STOP( ADC_CLK );										/* ADCClock stop				*/
#endif
	SCU_CLK_STOP( GBETHER_CLK );									/* GbEtherClock stop			*/
	SCU_CLK_STOP( DDR3_CLK );										/* DDR3Clock stop				*/
	SCU_CLK_START( USB_CLK );										/* USBClock supply				*/
	SCU_CLK_STOP( BUSOUT_CLK );										/* BUSCLK_OUTClock stop		*/

	/* SSPRO7_CLK_SEL			*/
//for ES1,2	<S152>
	SCU_CLK_SEL( I2C_CKSEL, CLK_MODE2 );							/* I2CClock 384Kbps			*/

//for ES3,4	<S152>
	SCU_CLK_SEL( I2C_CKSEL, CLK_MODE4 );							/* I2CClock APBREG15set with	*/
	SCU_SET_APBREG15( LOW_DISABLE, RATE_D3, CLK_828 );

//	SCU_CLK_SEL( DDR3_CKSEL, CLK_MODE2 );							/* DDR3Clock 600Mbps			*/
//	SCU_CLK_SEL( OUT_CKSEL, CLK_MODE2 );							/* External bus clock 125MHz		*/

	return;
}


/****************************************************************************************************/
/*																									*/
/*		SCU UDLRelated initialization processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:SSPRO7_WDT_MODE, SSPRO7_UDLCNT, SSPRO7_UDLINT_STATUSInitial setting		*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
static void InitScuUDLReg( void )
{

	/* SSPRO7_WDT_MODE			*/
	SCU_DISABLE_WDT_RST();											/* UDL_WDTReset disabled			*/

#ifdef	_M3UDL_MICRO_DOWNLOAD_
	/* SSPRO7_UDLCNT			*/
	SCU_SET_UDLM3_PRGSEL();											/* M3UDL PRGSELEnabled				*/
#endif

	/* SSPRO7_UDLINT_STATUS			*/
	SCU_READ_UDLINT_STATUS();										/* UDL INTInterrupt factor clear		*/



}


/****************************************************************************************************/
/*																									*/
/*		SCUInterrupt-related initialization processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:SSPRO7_CPUSS_TRIGINT, SSPRO7_NMI_SET, SSPRO7_INTSYS1_EN,								*/
/*				SSPRO7_INTSYS2_EN, SSPRO7_VICIFSYN_ENInitial setting										*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
static void InitScuINTCReg( void )
{

	/* SSPRO7_CPUSS_TRIGINT			*/
	SCU_READ_CPUSS_TRIGINT();										/* Clear cross trigger interrupt factor	*/

	/* SSPRO7_NMI_SET			*/
	SCU_ENABLE_FIQ_MASK();											/* FIQcan be masked				*/

	/* SSPRO7_INTSYS1_EN			*/
//	SCU_INTSYS1_EN(ARMPF_LONGALL1);									/* Synchronize all peripheral circuit interrupts		*//* <S0E2> */

	/* SSPRO7_INTSYS2_EN			*/
//	SCU_INTSYS2_EN(ARMPF_LONGALL1);									/* Synchronize all peripheral circuit interrupts		*//* <S0E2> */

	/* SSPRO7_VICIFSYN_EN			*/
//	SCU_SET_VICIFSYN();												/* VIC-CPUSynchronize				*//* <S0E2> */

}


/****************************************************************************************************/
/*																									*/
/*		SCUStack area initialization processing									<S127>								*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation: End of stack area20hofFFill (for alarm check)												*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
static void InitScustack( void )							/* SCUStack area initialization processing			*/
{
	LONG i;

	for(i=0;i<(0x20/4);i++)
	{
		*(VULONG*)(0x67FAF600 + i*4) = 0xFFFFFFFF;
	}
}


#ifdef	_PCIE_USE_
/****************************************************************************************************/
/*																									*/
/*		SCU PCIeRelated initialization processing																		*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:SSPRO7_PCIE_INT, SSPRO7_PCIE_INTMSK, SSPRO7_PCIE_INTSEL,								*/
/*				SSPRO7_PCIE_CLKSEL, SSPRO7_UDLINT_CONTInitial setting										*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
static void InitScuPCIeReg( void )
{

	/* SSPRO7_UDLINT_CONT			*/
	SCU_PCI_EP_A_SEL( UDL_INTA );								/* PCI EPInterruptsAtoUDL_INTAOutput(Temporary)	*/
	SCU_PCI_EP_B_SEL( UDL_INTB );								/* PCI EPInterruptsBtoUDL_INTBOutput(Temporary)	*/
	SCU_PCI_EP_C_SEL( UDL_INTC );								/* PCI EPInterruptsCtoUDL_INTCOutput(Temporary)	*/
	SCU_PCI_EP_S_SEL( UDL_INTN );								/* PCI EPInterruptsDtoUDL_INTNOutput(Temporary)	*/


}
#endif


#ifdef	_GBETHER_USE_
/****************************************************************************************************/
/*																									*/
/*		SCU GbEtherRelated initialization processing																	*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:SSPRO7_GBETHER_CONTInitial setting															*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
static void InitScuGbEtherReg( void )
{


}
#endif

#ifndef _MERCURY_PLATFORM_			/*<S190>*/
/****************************************************************************************************/
/*																									*/
/*		JL086SoCSoft reset processing								<S044><S076>						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		Operation:CortexR4SoC															*/
/*		Argument: None																					*/
/*		Return value: None																				*/
/*																									*/
/****************************************************************************************************/
void ScuSoftReset( AXIS_HANDLE	*AxisHandle )
{
	BOOL			softResetSts;
	BOOL			EncExchgSupported;	/* <S09C> */
	AXIS_HANDLE 	*AxisRsc;
	ASIC_HW_IF_STR 	*AsicHwReg;
	MENC_INIT_PRM	MencInitPrm;
	PRMDATA 		*Prm;
	SENC_ERROR      err[MAX_AXIS_NUMBER];
	SHORT			ax_no;
	ULONG 			wtimer;				/* <S1C5> */


	AsicHwReg = AxisHandle->SvAsicRegs->AsicHwReg;
	Prm = AxisHandle->UniPrms->Prm;
	softResetSts = 0;

	/*------------------------------------------------------------------------------*/
	/*		Get Parameters related to Motor Encoder									*/
	/*------------------------------------------------------------------------------*/
	MencInitPrm.MencP = &(Prm->MencP);
	MencInitPrm.b_prm2 = Prm->b_prm2;
	MencInitPrm.b_prm80 = Prm->b_prm80;
	MencInitPrm.mtlmt = Prm->mtlmt;
	MencInitPrm.scalepitch2l = Prm->scalepitch2l;
	MencInitPrm.spgfail = Prm->spgfail;

	for( ax_no = 0; ax_no < MAX_AXIS_NUMBER; ax_no++ )
	{
		AxisRsc = &AxisHandle[ax_no];
//		softResetSts |= ((AxisRsc->SeqCtrlOut->BeComplete)|(AxisRsc->SeqCtrlOut->TgOnSts));
		softResetSts |= ((AxisRsc->SeqCtrlOut->BeComplete)||(!(AxisRsc->SeqCtrlOut->MotStop)));		/* Review of resettable conditions<S119> */
	}

	if( softResetSts == 0 )
	{/* Can be reset only when "servo is not on" & "motor is not rotating"*/
		
		__DI();							/* Stop interrupt before reset */

		/* CommunicationASIC,Command option reset *//* <S14E> */
#if ( (CSW_NETIF_TYPE == NETIF_M2_TYPE) || (CSW_NETIF_TYPE == NETIF_M3_TYPE) )
        NdResetAsic( (VOID*)HW_NETLSI_BASE_ADDRESS );
#endif
   		HALdrv_ClearDebugSP( OP_SWRST );//Option reset (GPIO13) On	/* <S106> */
		KlibWaitms(100); //100ms Wait								/* <S106> */
		HALdrv_OutputDebugSP( OP_SWRST ); //Option reset Off			/* <S106> */

		for( ax_no = 0; ax_no < MAX_AXIS_NUMBER; ax_no++ )		/* SGD7W<S119> */
		{
			MENCV			*MencV;
			MencV = (&AxisHandle[ax_no])->MencV;
			/* Pointer set to encoder initialization parameter definition */	/* <S1C5> */
			MencInitPrm.MencP = &((&AxisHandle[ax_no])->UniPrms->Prm->MencP);
			if( (TRUE == MencV->EncConnect) && (SENC_COMRATE_8MBPS == MencV->PgBaudRate) )
			{/* Encoder connection&8MbpsCommunication confirmation */
	//			SencChangeP0Mode( MencV );		/* EncoderP0Mode switching *//* <S0F8> */
				SencInitChangeP0Mode( MencV );	/* EncoderP0Mode switching *//* <S0F8> */
				KlibRstLongTimer( &wtimer );	/* Corrected to exit by timeout even if switching fails */	/* <S1C5> */
				do{
					/* Communication speed4MbpsSwitch */
					err[0] = SencExchngBaudrate(MencV,
												&MencInitPrm,
												AsicHwReg,
												SENC_COMRATE_4MBPS,
												&EncExchgSupported );	/*<S09C>*/
					KlibWaitus(10);
//-				}while( err[0] != SENCERR_SUCCESS );
				/* Timeout even if switching fails(5Second) */	/* <S1C5> */
				}while(( err[0] != SENCERR_SUCCESS ) && (KlibGetLongTimerMs(wtimer) < 5000));
				MencV->PgBaudRate = SENC_COMRATE_4MBPS;
			}
		}
		SCU_SOFTRESET();
	}

	return;
}
#endif	/*_MERCURY_PLATFORM_*/


/*-------end of file----------------------------------------------------------------------------------------*/

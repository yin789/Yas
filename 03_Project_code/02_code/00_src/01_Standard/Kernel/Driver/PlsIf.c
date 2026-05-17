/****************************************************************************************************/
/*																									*/
/*																									*/
/*		PlsIf.c : Command Pulse Interface															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function : Command Pulse Interface																*/
/*																									*/
/*			1)Initialize Pulse Interface Module														*/
/*			2)Pulse Interface ScanA Handler															*/
/*			3)Pulse Interface ScanB Handler 														*/
/*			4)Pulse Interface Parameter Calculation 												*/
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
/*																									*/
/*		Rev.1.00 : 2013.8.13	K.Ozaki																*/
/*		Rev.1.10 : 2013.10.1	K.Ozaki			<S00C>												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#include	"KnlApi.h"

/****************************************************************************************************/
/*																									*/
/*		Function Prototypes																			*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Public Functions																			*/
/*--------------------------------------------------------------------------------------------------*/
void	KpxPlsScanAService( PLSIFV *pPls );	/* Command Pulse Manager(ScanB)							*/
void	KpxPlsScanBService( PLSIFV *pPls );	/* Command Pulse Manager(ScanB)							*/
//void	KpxPlsScanCService( PLSIFV *pPls );	/* Command Pulse Manager(ScanB)							*/
void	KpxInitPls( void *pAxRsc ); 		/* Initialize Command Pulse Interface 					*/
/*--------------------------------------------------------------------------------------------------*/
LONG	KpiPcalPerClrMode( PLSIFV *pPls, LONG data );	/* Initial Parameter calculation			*/
void	KpiPcalEdgeClrDisTime( 							/* Perr Clear Edge Clear Disable Time		*/
			PLSIFV *pPls, 
			USHORT time, 
			ULONG cycle );	
/*--------------------------------------------------------------------------------------------------*/
void	KpiChkPerClearSignal( PLSIFV *pPls ); 	/* Perr Clear Latch Signal Detect Service			*/
/*--------------------------------------------------------------------------------------------------*/
void	KpiPlsPerClrLatchPosition( 				/* Input PERR Clear Latched Position 				*/
			PLSIFV *pPls, 						/* 			 										*/
			LONG PosFbki,						/* Feedback Position 								*/
			LONG MotAngle,						/* Motor Angle  									*/
			LONG *pClrPos );					/* Latched Position 								*/

/*--------------------------------------------------------------------------------------------------*/
/*		Private Functions : Command Pulse Interface													*/
/*--------------------------------------------------------------------------------------------------*/
static void LpxPlsServiceA( struct _PLSIFV *this );	/* ScanA command pulse handler					*/
static void LpxPlsServiceB( struct _PLSIFV *this );	/* ScanB command pulse handler					*/
/*--------------------------------------------------------------------------------------------------*/
static void	LpxSetCmdFilSelPort( struct _PLSIFV *this ); /* set command pulse filter selection port	*/

/****************************************************************************************************/
/*																									*/
/*		Variables																					*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Constant Definition																			*/
/*																									*/
/****************************************************************************************************/
#define _UDL_REG_ADDR_  (ULONG)0xC0000000
#define _UDL_REG_ADDR_OFFSET_  (ULONG)0x00001000
#define AREG_LODSET_SETV			0x02C0
/*--------------------------------------------------------------------------------------------------*/

/****************************************************************************************************/
/*																									*/
/*		Macro Definition																			*/
/*																									*/
/****************************************************************************************************/
#define ASIC_WR( reg, value ) *(pPls->##reg) = (value)
#define ASIC_RD( reg, value ) (value) = (SHORT)*(pPls->##reg)

/*--------------------------------------------------------------------------------------------------*/
#define KPI_LATCH_PULSECNTR( ch ) { 																\
ASIC_HW_IF_STR *pAsic = (ASIC_HW_IF_STR *)(_UDL_REG_ADDR_ + (_UDL_REG_ADDR_OFFSET_ * ch)); 			\
		pAsic->AREG_LODSET = AREG_LODSET_SETV;														\
}
/*--------------------------------------------------------------------------------------------------*/
void	KpxLatchPulsCntr0( void ) { KPI_LATCH_PULSECNTR( 0 ); }	/* Latch Pulse Counter(Ch:0) */
void	KpxLatchPulsCntr1( void ) { KPI_LATCH_PULSECNTR( 1 ); }	/* Latch Pulse Counter(Ch:1) */

/*--------------------------------------------------------------------------------------------------*/
#define KPI_CHK_LATCHTIMER_CCU( ch )						/* Check Timer Latch 3u					*/\
LONG	lwk0,lwk1;											/*										*/\
ASIC_HW_IF_STR *pAsic = (ASIC_HW_IF_STR *)(_UDL_REG_ADDR_ + (_UDL_REG_ADDR_OFFSET_ * ch)); 			\
		lwk0 = (pAsic->AREG_STIMSTAT); 						/* Read Latch Status 					*/\
		lwk0 = (( lwk0 & (1<<11) ) ? (TRUE) : FALSE);		/* lwk == TRUE ? LatchComplete			*/\
		lwk1 = (pAsic->AREG_STIMLT3);						/* read Latch Timer						*/\
		lwk1 = ((lwk0) ? (lwk1) : 0 );						/* set Latch Value						*/\

#define KPI_CHK_LATCHTIMER_CCD( ch )						/* Check Timer Latch 3d					*/\
LONG	lwk0,lwk1;											/*										*/\
ASIC_HW_IF_STR *pAsic = (ASIC_HW_IF_STR *)(_UDL_REG_ADDR_ + (_UDL_REG_ADDR_OFFSET_ * ch)); 			\
		lwk0 = (pAsic->AREG_STIMSTAT); 						/* Read Latch Status 					*/\
		lwk0 = (( lwk0 & (1<<15) ) ? (TRUE) : FALSE);		/* lwk == TRUE ? LatchComplete			*/\
		lwk1 = (pAsic->AREG_STIMLT7);						/* read Latch Timer						*/\
		lwk1 = ((lwk0) ? (lwk1) : 0 );						/* set Latch Value						*/\
/*--------------------------------------------------------------------------------------------------*/
LONG	KpxChkClrLatchTimer0u( USHORT *pTim ) 
{ 
		KPI_CHK_LATCHTIMER_CCU( 0 ); 
		*pTim = (SHORT)lwk1;
		return lwk0;
}
LONG	KpxChkClrLatchTimer0d( USHORT *pTim ) 
{ 
		KPI_CHK_LATCHTIMER_CCD( 0 ); 
		*pTim = (SHORT)lwk1;
		return lwk0;
}
LONG	KpxChkClrLatchTimer1u( USHORT *pTim ) 
{ 
		KPI_CHK_LATCHTIMER_CCU( 1 ); 
		*pTim = (SHORT)lwk1;
		return lwk0;
}
LONG	KpxChkClrLatchTimer1d( USHORT *pTim ) 
{ 
		KPI_CHK_LATCHTIMER_CCD( 1 ); 
		*pTim = (SHORT)lwk1;
		return lwk0;
}
/*--------------------------------------------------------------------------------------------------*/


/****************************************************************************************************/
/*																									*/
/*		Initialize Virtual Serial Message Interface													*/
/*																									*/
/****************************************************************************************************/
void	KpxInitPls( void *pAxis )
{
AXIS_HANDLE *pAxRsc = (AXIS_HANDLE *)pAxis;
PLSIFV *pPls = pAxRsc->pPlsIfV;
ASIC_HW_IF_STR *pAsic = pAxRsc->SvAsicRegs->AsicHwReg;
LONG ch = pAxRsc->AxisNo;
/*--------------------------------------------------------------------------------------------------*/
/*	JL086ofPulseIFInitialization isSHal_InitAsicPulseIF( )											*/
/*--------------------------------------------------------------------------------------------------*/
	;
/*--------------------------------------------------------------------------------------------------*/
/* 	Reset Memory 																					*/
/*--------------------------------------------------------------------------------------------------*/
	/*
	 * ※ Leave only what is rewritten in initial parameter calculation
	 *
	 *
	 */
/*--------------------------------------------------------------------------------------------------*/
	MlibResetLongMemory( (ULONG *)(&(pPls->f)), (sizeof( PLSIFV )-sizeof( PLSIFP ))/4 );	

/*--------------------------------------------------------------------------------------------------*/
/*		Read Initial Pulse Command Counter															*/
/*--------------------------------------------------------------------------------------------------*/
	KPI_LATCH_PULSECNTR( ch );							/* Latch ASIC  Pulse Counter				*/
	pPls->PcmdHwCntr  = KPI_READ_PCMDCNTR( );			/* Read  Pulse Command Counter				*/
	pPls->dPcmdHwCntr = 0;								/* Reset Delta Pulse Command				*/	
/*--------------------------------------------------------------------------------------------------*/
	pPls->CMDLDL = &(pAsic->AREG_CMDLDL);				/* pointer to the command pulse load latch  */
	pPls->POCSET = &(pAsic->AREG_POCSET);				/* pointer to the command pulse data value  */
	pPls->CMDCLML = &(pAsic->AREG_CMDCLML);				/* pointer to the command pulse latch value */
/*--------------------------------------------------------------------------------------------------*/
	pPls->TIM = &(pAsic->AREG_TIM);						/* @@										*/
	pPls->SFBLT = &(pAsic->AREG_SFBLT4);				/* @@										*/
	pPls->MposSftR = pAxRsc->MencV->MposSftR;			/* @@										*/
	/*----------------------------------------------------------------------------------------------*/
	/*	InterimSGDV																*/
	/*----------------------------------------------------------------------------------------------*/
	if( pPls->P.PerClrMecha )
	{
		pAsic->AREG_ENCOFS = (ULONG)(-PFBKCOMPDELAY); /* set ENCOFS with Machine Base Timer Offset */
	}
	else
	{
		pAsic->AREG_ENCOFS = (ULONG)PFBKPOUTDELAY;  /* set ENCOFS with Controler Base Timer Offset */
	}
	/*----------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------*/
/*	Edge detection method setting																		*/
/*	※Calculate parameters first																	*/
/*	※Edge detection logic isUDLSet in logical settings.														*/
/*--------------------------------------------------------------------------------------------------*/
	pPls->CHK_LATCHTIMER = ((ch == 0 ) ? KpxChkClrLatchTimer0u : KpxChkClrLatchTimer1u);
	pPls->CHK_LATCHTIMER( &(pPls->PerClrEdgeTime) );	/* Clear Latch Status (As a precaution)			*/

/*--------------------------------------------------------------------------------------------------*/
	if( ch == 0 )
	{
		pPls->LATCH_PULSCNTR = KpxLatchPulsCntr0;		/* pointer to the Function to Latch Puls	*/
	}
	else
	{
		pPls->LATCH_PULSCNTR = KpxLatchPulsCntr1;		/* pointer to the Function to Latch Puls	*/
	}
/*--------------------------------------------------------------------------------------------------*/
	LpxSetCmdFilSelPort( pPls );						/* set hardware command pulse filter port	*/
/*--------------------------------------------------------------------------------------------------*/
	if( pAxRsc->UniPrms->Bprm->RvsDir )
	{
		pPls->RvsDir = (-1);							/* Reverse Mode */
	}
	else
	{
		pPls->RvsDir = (1);								/* Normal Mode */
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}

/****************************************************************************************************/
/*																									*/
/*		Command Pulse Manager( ScanA Cycle )														*/
/*																									*/
/****************************************************************************************************/
void	KpxPlsScanAService( PLSIFV *pPls )
{
	if(PLS_CH0_USE == 1){ LpxPlsServiceA( pPls ); }
//	if(PLS_CH1_USE == 1){ LpxPlsServiceA( &(LPlsIfV[1]) ); }
}

/****************************************************************************************************/
/*																									*/
/*		Command Pulse Manager( ScanB Cycle )														*/
/*																									*/
/****************************************************************************************************/
void	KpxPlsScanBService( PLSIFV *pPls )
{
	if(PLS_CH0_USE == 1){ LpxPlsServiceB( pPls ); }
//	if(PLS_CH1_USE == 1){ LpxPlsServiceB( &(LPlsIfV[1]) ); }
}

/****************************************************************************************************/
/*																									*/
/*		ScanA command pulse handler																	*/
/*																									*/
/****************************************************************************************************/
void	LpxPlsServiceA( struct _PLSIFV *pPls )	
{
/*--------------------------------------------------------------------------------------------------*/
/*		Read Latch Timer Current Value																*/
/*--------------------------------------------------------------------------------------------------*/
		ASIC_RD( TIM, pPls->LatchBaseTimeA );
/*--------------------------------------------------------------------------------------------------*/
/* 		Latch Asic Pulse Counter																	*/ 
/*--------------------------------------------------------------------------------------------------*/
		pPls->LATCH_PULSCNTR( );
/*--------------------------------------------------------------------------------------------------*/
		return;
}

/****************************************************************************************************/
/*																									*/
/*		ScanB command pulse handler																	*/
/*																									*/
/****************************************************************************************************/
void	LpxPlsServiceB( struct _PLSIFV *pPls )	
{
LONG	lwk0;
LONG	lwk1;
/*--------------------------------------------------------------------------------------------------*/
/*		Read Pulse Command Counter																	*/
/*--------------------------------------------------------------------------------------------------*/
		/*------------------------------------------------------------------------------------------*/
		/*	Updating the previous command position																		*/
		/*------------------------------------------------------------------------------------------*/
		if( pPls->f.PerClrEdge )							/* Deviation edge clear					*/
		{
			lwk0 = pPls->ClrPcmdPastCntr;					/* Change the previous command position to the correction position			*/
			lwk1 = pPls->PcmdHwCntr;
		}
		else
		{
			lwk0 = pPls->PcmdHwCntr;						/* Update the previous command position					*/
			lwk1 = pPls->PcmdHwCntr;
		}
		/*------------------------------------------------------------------------------------------*/
		/* Read and Renewal Pulse Command Counter Info. 											*/
		/*------------------------------------------------------------------------------------------*/
		pPls->PcmdHwCntr = KPI_READ_PCMDCNTR( );			/* Read Pulse Command Counter			*/
		pPls->dPcmdHwCntr = (SHORT)(pPls->PcmdHwCntr - lwk0);
		pPls->dPcmdInputHwCntr = (SHORT)(pPls->PcmdHwCntr - lwk1);
/*--------------------------------------------------------------------------------------------------*/
		pPls->PcmdRingCntr += pPls->dPcmdHwCntr;			/* Pulse Counter for Network I/F		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Pulse Command Input Signal																	*/
/*--------------------------------------------------------------------------------------------------*/
		KPX_READ_PCMD_STATUS( lwk0 );						/* Read Pcmd Status						*/
		pPls->f.TrcPcmdInon = KPX_CHECK_PCMD_CADAT( lwk0 );	/* Trace : Pulse Command Input On		*/
		pPls->f.TrcPcmdSign = KPX_CHECK_PCMD_CBDAT( lwk0 );	/* Trace : Pulse Command Input Sign		*/
		pPls->f.PerClrSignal = KPX_CHECK_PCMD_CCDAT( lwk0 );/* Position Error Clear Signal			*/

/*--------------------------------------------------------------------------------------------------*/
/*		Read Latch Base Timer		 																*/
/*--------------------------------------------------------------------------------------------------*/
		pPls->LatchBaseTimeB = pPls->LatchBaseTimeA;

/*--------------------------------------------------------------------------------------------------*/
/*		Check CLR Signal Edge, Read Latch Timer		 												*/
/*--------------------------------------------------------------------------------------------------*/
		pPls->f.EdgeDetected = pPls->CHK_LATCHTIMER( &(pPls->PerClrEdgeTime) );

/*--------------------------------------------------------------------------------------------------*/
/*		Check Perr Clear Signal (LTM)		 														*/
/*--------------------------------------------------------------------------------------------------*/
		KpiChkPerClearSignal( pPls );						/* check perr clear signal edge			*/

/*--------------------------------------------------------------------------------------------------*/
/*		@@ Next Command Position																	*/
/*--------------------------------------------------------------------------------------------------*/
		/*------------------------------------------------------------------------------------------*/
		/*	Position command correction calculation		Previously commanded position of the next scan -> BoutK.ClrPcmdPastCntr				*/
		/*------------------------------------------------------------------------------------------*/
		/*		■Processing when command latch is completed/incomplete (SGDS)									*/
		/*			.Complete⇒Set the previous command position of the next scan as the command position at edge input (latch)			*/
		/*			.Not completed⇒Set the previous command position of the next scan as the command position of the current scan							*/
		/*------------------------------------------------------------------------------------------*/
		if( pPls->f.ClrEnableFlg )							/*	When command latch is completed					*/
		{
			pPls->ClrPcmdPastCntr = pPls->ClrLatchRefPos;	/* Set the latch command position to the previous value			*/
		}
		else												/* When command latch is not completed						*/
		{
			pPls->ClrPcmdPastCntr = pPls->PcmdHwCntr;		/* Set the previous command position as the current command position		*/
		}
/*--------------------------------------------------------------------------------------------------*/
		return;
}

/****************************************************************************************************/
/*																									*/
/*		Position deviation clear signal monitoring service																*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*		<V904>  After executing position deviation clear (edge clear) to prevent chattering during position deviation clear	*/
/*				Position deviation clear signal input minimum time interval(Pn240)Ignore the clear signal and check the latched state while the elapsed time	*/
/*				A has been corrected.															*/
/*				Also, even if it is not in progress,ASICLatch completion"&&"CPUIf "No recognition" occurs twice in a row,	*/
/*				ASICLatch completion was corrected to be canceled. This uses the latch function ofASICremains latched	*/
/*				NextCPUrecognizes the latch signal,FBPosition command with counter and previous latch counter		*/
/*				Because there is a risk of generation polynomial											*/
/****************************************************************************************************/
void	KpiChkPerClearSignal( PLSIFV *pPls )
{
LONG	lwk;
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation clear signal invalid time elapsed?															*/
/*--------------------------------------------------------------------------------------------------*/
	if( pPls->EdgeClr_DisableCnt == 0 )
	{
/*--------------------------------------------------------------------------------------------------*/
/*		After position deviation clear signal invalid time elapsed															*/
/*--------------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*		Position deviation clear signal present																	*/
	/*----------------------------------------------------------------------------------------------*/
		if( pPls->f.EdgeDetected  )							/* Interrupt request check					*/
		{
			KPX_READ_PCMD_STATUS( lwk );					/* AREG_POCSETReading					*/
			if( lwk & 0x8000 )								/* Command counter latch completed				*/
			{
				pPls->f.ClrEnableFlg = TRUE;				/* Command counter latch completed				*/
				pPls->ClrLatchRefPos = (SHORT)*(pPls->CMDCLML);/* Command counter reading at edge input	*/
			}
			else
			{
				pPls->f.ClrEnableFlg = FALSE;				/* Command counter latch incomplete				*/
				pPls->ClrLatchRefPos = 0;					/* Unused in this case: set it to 0			*/
			}
			pPls->f.PerClrEdge = TRUE;						/* Position deviation clear edge signal setting			*/
			ASIC_WR( POCSET, (lwk & 0xFEFF));				/* Command counter latch release				*/
			KlibWaitus( 1 );								/* @Temporary measure	Waiting time confirmation required		*/
			ASIC_WR( POCSET, (lwk | 0x0100));				/* Command counter latch enable				*/
			pPls->EdgeClr_DisableCnt = pPls->P.EdgeClr_DisableTime;		/* Set elapsed time			*/
			pPls->f.EdgeClrUnmatch = FALSE;					/* Clear mismatch state					*/
		}
	/*----------------------------------------------------------------------------------------------*/
	/*		No position deviation clear signal																	*/
	/*----------------------------------------------------------------------------------------------*/
		else
		{
			pPls->f.PerClrEdge = FALSE;						/* Position deviation clear edge signal clear		*/
			pPls->f.ClrEnableFlg = FALSE;
			pPls->ClrLatchRefPos = 0;						/* Unused in this case: set it to 0			*/

			KPX_READ_PCMD_STATUS( lwk );					/* AREG_POCSETReading					*/
			if( lwk & 0x8000 )								/* Command counter latch completed?			*/
			{
				if( pPls->f.EdgeClrUnmatch == FALSE )		/* Isn't there a previous mismatch?				*/
				{
					pPls->f.EdgeClrUnmatch = TRUE;			/* Set mismatch state					*/
				}
				else
				{
					ASIC_WR( POCSET, (lwk & 0xFEFF));		/* Command counter latch release				*/
					KlibWaitus( 1 );						/* @Temporary measure	Waiting time confirmation required		*/
					ASIC_WR( POCSET, (lwk | 0x0100));		/* Command counter latch enable				*/
					pPls->f.EdgeClrUnmatch = FALSE;			/* Clear mismatch state					*/
				}
			}
			else
			{
				pPls->f.EdgeClrUnmatch = FALSE;				/* Clear mismatch state					*/
			}
		}
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Before position deviation clear signal invalid time elapses (invalid)													*/
/*--------------------------------------------------------------------------------------------------*/
	else
	{
		pPls->f.PerClrEdge = FALSE;							/* Position deviation clear edge signal clear		*/
		pPls->f.ClrEnableFlg = FALSE;
		pPls->ClrLatchRefPos = 0;							/* Unused in this case: set it to 0			*/

		KPX_READ_PCMD_STATUS( lwk );						/* AREG_POCSETReading					*/
		if( lwk & 0x8000 )									/* Command counter latch completed?			*/
		{
			ASIC_WR( POCSET, (lwk & 0xFEFF));				/* Command counter latch release				*/
			KlibWaitus( 1 );								/* @Temporary measure	Waiting time confirmation required		*/
			ASIC_WR( POCSET, (lwk | 0x0100));				/* Command counter latch enable				*/
		}

		pPls->f.EdgeClrUnmatch = FALSE;						/* Clearing signal recognition mismatch status			*/
		pPls->EdgeClr_DisableCnt--;							/* Decrement the elapsed time				*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*	Position deviation clear edge signal setting																	*/
/*--------------------------------------------------------------------------------------------------*/
	if( pPls->P.PerClrEdgeMode == FALSE ) { pPls->f.PerClrEdge = FALSE;	}	
/*--------------------------------------------------------------------------------------------------*/
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Initial Parameter calculation (Pn200)														*/
/*																									*/
/****************************************************************************************************/
LONG	KpiPcalPerClrMode( PLSIFV *pPls, LONG data )
{
/*--------------------------------------------------------------------------------------------------*/
/*		Pn200.1 : Clear signal format selection switch														*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( data >> 4 ) & 0x0F )
	{
		case 0x00:	pPls->P.PerClrSigSel = 0x00; break;	/* Clear the deviation when the signal goes high				*/
		case 0x01:  pPls->P.PerClrSigSel = 0x01; break;	/* Clear the deviation when the signal rises				*/
		case 0x02:  pPls->P.PerClrSigSel = 0x02; break;	/* Clear the deviation when the signal goes low				*/
		case 0x03:  pPls->P.PerClrSigSel = 0x03; break;	/* Clear the deviation when the signal falls				*/
		default : return (NG);
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Pn200.2 : Clear action selection setting																*/
/*--------------------------------------------------------------------------------------------------*/
	switch(( data >> 8 ) & 0x0F )
	{
		case 0x00:	pPls->P.PerClrMode = 0x00; break;	/* + at base blockCLRWhen the deviation clear	*/
		case 0x01:	pPls->P.PerClrMode = 0x01; break;	/* CLRsignal					*/
		case 0x02:	pPls->P.PerClrMode = 0x02; break;	/* When an alarm occurs+CLRWhen the deviation clear	*/
		case 0x03:	pPls->P.PerClrMode = 0x03; break;	/* Deviation not cleared(CLRSignal ignored)			*/
		default : return (NG);
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation level clear selection setting																*/
/*--------------------------------------------------------------------------------------------------*/
	if( pPls->P.PerClrSigSel == 0x00 )	 	{ pPls->P.PerClrLvlSel = 0x01;}	/* High Level			*/
	else if( pPls->P.PerClrSigSel == 0x02 )	{ pPls->P.PerClrLvlSel = 0x02;}	/* Low  Level			*/
	else                                 	{ pPls->P.PerClrLvlSel = 0x00;}	/* No   Level			*/
/*--------------------------------------------------------------------------------------------------*/
/*		Position deviation edge clear selection setting																*/
/*--------------------------------------------------------------------------------------------------*/
	if( ((pPls->P.PerClrSigSel == 0x01)||(pPls->P.PerClrSigSel == 0x03))&&(pPls->P.PerClrMode != 0x03) )
	{
		pPls->P.PerClrEdgeMode = TRUE;					/* CLRClear the deviation at the edge of the signal		*/
	}
	else
	{
		pPls->P.PerClrEdgeMode = FALSE;					/* CLRDo not clear the deviation at the edge of the signal		*/
	}
/*--------------------------------------------------------------------------------------------------*/
/*		Pn200.3 : Filter selection settings																	*/
/*--------------------------------------------------------------------------------------------------*/
	switch( (data >> 12) & 0x0F )
	{
	case 0x00: pPls->P.CmdOpenCollector = FALSE;   break; /* Line driver signal command filter 1	*/
	case 0x01: pPls->P.CmdOpenCollector = TRUE;    break; /* Command filter for open collector signal	*/
	case 0x02: pPls->P.CmdOpenCollector = FALSE;   break; /* Line driver signal command filter 2	*/
	default :  return (NG);
	}
/*--------------------------------------------------------------------------------------------------*/
/*	FBPosition correction calculation flag: TemporarilySGDV.													*/
/*--------------------------------------------------------------------------------------------------*/
	pPls->P.PerClrMecha = FALSE;
/*--------------------------------------------------------------------------------------------------*/
	return (OK);
}


/****************************************************************************************************/
/*																									*/
/*		Pn240: Position deviation clear signal input minimum time interval calculation	<V904>										*/
/*			Ignore clear signal for a certain period of time after position deviation clear (edge clear) valid edge recognition			*/
/*			time.																		*/
/*																									*/
/*		@@ parameters																				*/
/*		1) ch : Command Pulse Interface Channel Number												*/
/*		2) time : Perr Clear Disable Time [ms]														*/
/*		3) cycle : Scan cycle[us]																	*/
/*																									*/
/****************************************************************************************************/
void	KpiPcalEdgeClrDisTime( PLSIFV *pPls, USHORT time, ULONG cycle )	/* Deviation clear invalid time calculation	*/
{
		pPls->P.EdgeClr_DisableTime = (LONG)( (ULONG)time * 1000 / cycle );
		return;
}

/****************************************************************************************************/
/*																									*/
/*		Deviation clear signal latch position calculation																*/
/* 		Input PERR Clear Latched Position 															*/
/*																									*/
/****************************************************************************************************/
#if 0
void	KpiPlsPerClrLatchPosition( 				/* Input PERR Clear Latched Position 				*/
			PLSIFV *pPls, 						/* Channel No. 										*/
			LONG PosFbki,						/* Feedback Position 								*/
			LONG dPosFbki,						/* Delta Position 									*/
			LONG *pClrPos )						/* Latched Position 								*/
{
LONG dFbPosCmp;
	/*----------------------------------------------------------------------------------------------*/
	/*	FB position correction calculation		FB position increment value at edge input -> dFbPosCmp							*/
	/*----------------------------------------------------------------------------------------------*/
	if( pPls->P.PerClrMecha )							/* Machine position standard deviation clear					*/
	{
		dFbPosCmp = KPI_PFBKCOMPEN( dPosFbki, pPls->PerClrEdgeTime );
	}
	else												/* Clearing the upper controller standard deviation			*/
	{
		dFbPosCmp = KPI_PFBKCOMPENHC( dPosFbki, pPls->PerClrEdgeTime );
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	@@ FB position calculation			FB position at edge input -> BoutK.ClrFbPos							*/
	/*----------------------------------------------------------------------------------------------*/
	if ( pPls->f.EdgeDetected )
	{
		*pClrPos = PosFbki - dFbPosCmp;
//		pPls->ClrFbPosSft = PosFbki - dFbPosCmp; 		/* @@  Erase after								*/
	}
	/*----------------------------------------------------------------------------------------------*/
	return;
}
#endif

/****************************************************************************************************/
/*																									*/
/*		Deviation clear signal latch position calculation																*/
/* 		(UDL)																		*/
/*		TODO: Linear, Fulk																		*/
/*																									*/
/****************************************************************************************************/
void	KpiPlsPerClrLatchPosition( 				/* Input PERR Clear Latched Position 				*/
			PLSIFV *pPls, 						/* Pulse Interface 									*/
			LONG PosFbki,						/* Feedback Position [pulse] 						*/
			LONG MotAngle,						/* Motor Angle [pulse] 								*/
			LONG *pClrPos )						/* Latched Position [pulse]							*/
{
LONG dFbPosCmp;
LONG LatchPos = (((LONG)*(pPls->SFBLT)) << (pPls->MposSftR));
	/*----------------------------------------------------------------------------------------------*/
	/*	FB position correction calculation		FB position increment value at edge input -> dFbPosCmp							*/
	/*----------------------------------------------------------------------------------------------*/
	dFbPosCmp = (pPls->RvsDir) * ((MotAngle - LatchPos) >> (pPls->MposSftR));
	/*----------------------------------------------------------------------------------------------*/
	/*	@@ FB position calculation			FB position at edge input -> BoutK.ClrFbPos							*/
	/*----------------------------------------------------------------------------------------------*/
	if ( pPls->f.EdgeDetected )
	{
		*pClrPos = PosFbki - dFbPosCmp;
		pPls->ClrFbPos = PosFbki - dFbPosCmp; /* @@ For monitor: Erase later */ 
	}
	/*----------------------------------------------------------------------------------------------*/
	return;
}


/****************************************************************************************************/
/*																									*/
/*		Pulse command filter: Filter selection port setting												*/
/*																									*/
/****************************************************************************************************/
void	LpxSetCmdFilSelPort( PLSIFV *pPls )
{
/*--------------------------------------------------------------------------------------------------*/
/*	Iprm.f.CmdOpenCollector == TRUE (Pn200.3=1) ==> Command filter for open collector signal				*/
/*	Iprm.f.CmdOpenCollector == FALSE(Pn200.3=0) ==> Line driver signal command filter				*/
/*--------------------------------------------------------------------------------------------------*/
	if(pPls->P.CmdOpenCollector == TRUE)
	{
		KPI_PCMDFIL_ON( );		/* FILSEL ==> High Level	*/
	}
	else
	{
		KPI_PCMDFIL_OFF( );		/* FILSEL ==> Lo Level	*/
	}
}

/***************************************** end of file **********************************************/

/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Control parameter variable batch conversion process definition														*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			1. Batch conversion processing of parameter variables for non-adjustment, disturbance observer, etc.								*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2013.08.16	Y.Oka	For Mercury												*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "CtrlPrmTransducer.h"
#include "Mlib.h"
#include "Klib.h"



/****************************************************************************************************/
/*																									*/
/*		Control parameter variable batch conversion service initialization process												*/
/*																									*/
/****************************************************************************************************/
void	TranlateCtrlPrmInitialize( TRANSDUCE_CTRL *TransDuceCtrl )
{
	/* Initialize variables for conversion service */
	MlibResetLongMemory( TransDuceCtrl, sizeof(*TransDuceCtrl)>>2 );
	/* InitialatDIRECTMode */
	TransDuceCtrl->TranslateMode = TMODE_DIRECT;
}



/****************************************************************************************************/
/*																									*/
/*		Control parameter variable batch conversion request process														*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	When changing parameters such as no adjustment or disturbance observer, multiple related variables may be changed at the same time.	*/
/*				Switching is required.SGDVDisabling interruptsSGD7series	*/
/*				Use this function to request batch conversion,ScanAIfScanBActually execute conversion in processing	*/
/*				The risk of prohibited processing is avoided.													*/
/*				Note thatInitial, direct conversion is performed in this process.		*/
/*																									*/
/*		Caution:	Maximum size is32pieces,LONGSize(32bit). Use this function to set variable values ​​	*/
/*				In order to update, the transfer source and transfer destination must each4bytemust be aligned.	*/
/*				Also,2msor more elapses, a timeout will occur.					*/
/*																									*/
/****************************************************************************************************/
LONG	TranlateCtrlPrmRequest( TRANSDUCE_CTRL *TransDuceCtrl, void *Data, ULONG *Address,
																		UCHAR Size, CHAR Timing )
{
	LONG			i;
	LONG			ErrSts;
	ULONG			TimeOut;

	ErrSts = TR_SUCCESS;

	/* Check Interrupt Starting */
	if( TransDuceCtrl->TranslateMode == TMODE_DIRECT )
	{ /* Interrupt Stopped State */
		for( i=0; i<Size; i++ )
		{
			( (ULONG*)Address )[i] = ( (ULONG*)Data )[i];
		}
	}
	else
	{ /* Interrupt Active State */
		/* Check Request Data Size */
		if( Size > TR_DATA_MAX )
		{
			ErrSts = TR_DATA_ERROR;
		}

		/* Check Src Data Address and Dst Data Address */
		if( (Data == NULL) || (Address == NULL) )
		{
			ErrSts = TR_DATA_ERROR;
		}

		if( ErrSts != TR_DATA_ERROR )
		{
			/* Set Target Address */
			TransDuceCtrl->TargetAddress = Address;

			/* Set Parameter Data */
			for( i=0; i<Size; i++ )
			{
				TransDuceCtrl->DataArray[i] = ( (ULONG*)Data )[i];
			}

			/* Set Prameter Number (4byte unit) */
			TransDuceCtrl->TranslateSize = Size;

			/* Set Translate Request and Timing(ScanA or ScanB) */
			TransDuceCtrl->TranslateSts = Timing;

			/* Check and Wait Finish Last Translation */
			TimeOut = 0;
			while( TransDuceCtrl->TranslateSts != TR_READY )
			{
				/* Wait for Ready */
				KlibWaitus( KPI_SACYCLEUS );
				TimeOut++;
				if( TimeOut > (KPI_SCCYCLEMS * KPI_SACOUNTMS) )
				{
					ErrSts = TR_DATA_ERROR;
					TransDuceCtrl->TranslateSts = TR_READY;											  /* <S066> */
					break;																			  /* <S002> */
				}
			}
		}
	}
	return	ErrSts;
}



#pragma arm section code = "CALL_SCANA" /* <S1D2> */
/****************************************************************************************************/
/*																									*/
/*		Control parameter variable batch conversion process															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Overview:	Check the batch conversion request and copy the target data to the target address if there is a request.		*/
/*																									*/
/*		Caution:	ScanAIfScanB, set withScan.	*/
/*				. If the conversion is performed at the beginning of the interrupt process, at least before the execution of the process that uses the variable to be converted	*/
/*				It is necessary.																		*/
/*																									*/
/****************************************************************************************************/
void	TranlateCtrlPrm( TRANSDUCE_CTRL *TransDuceCtrl )
{
	LONG	i;

	/* Conversion execution */
	for( i=0; i<TransDuceCtrl->TranslateSize; i++ )
	{
		TransDuceCtrl->TargetAddress[i] = TransDuceCtrl->DataArray[i];
	}

	/* Conversion completion */
	TransDuceCtrl->TranslateSts = TR_READY;

	return;
}
#pragma arm section code /* <S1D2> */


/***************************************** end of file **********************************************/

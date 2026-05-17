/****************************************************************************
Description	: Vibration Frequency Analysis Module
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
Y.Oka  2010.11.16    created

*****************************************************************************/
#include "TuneLib.h"
#include "PnPrmListTbl.h"
#include "MLib.h"



/****************************************************************************************************/
/*																									*/
/*		Function Definition																			*/
/*																									*/
/****************************************************************************************************/
/* void		FftCalculateInitPrm( FFTANALYZE *FftAna );												*/
/* SHORT	FftAnalyzeExecute( FFTANALYZE *FftAna, SHORT *TraceData );								*/
/* void		FftSetAnaCondition( FFTANALYZE *FftAna, LONG SumpleNum, LONG AnaMaxFreq,				*/
/*															LONG AnaMinFreq, BOOL TuneLessSts );	*/
/* USHORT	FftGetAnaRsltVibmRf( FFTANALYZE *FftAna );												*/
/* void		FftSetForceAnaReq( FFTANALYZE *FftAna, BOOL Setting );									*/
/* BOOL		FftGetForceAnaReq( FFTANALYZE *FftAna );												*/
/****************************************************************************************************/
static	SHORT	fftAnalyzeCalc( 					/* Vibration frequency analysis main function						*/
					FFTANALYZE *FftAna,					/* Data for vibration frequency analysis					*/
					SHORT *TraceData, 					/* Trace data							*/
					LONG *fpeak ); 						/* Analysis frequency								*/
static	void	fftClearPeakDatas( 					/* Peak data clear function						*/
					FFTANALYZE *FftAna );				/* Data for vibration frequency analysis					*/
static	void	fftSpectrumPeakDetect(				/* Spectrum peak frequency detection function				*/
					FFTANALYZE *FftAna,					/* Data for vibration frequency analysis					*/
					ULONG swork,						/* Spectrum								*/
					ULONG freq );						/* Applicable frequency							*/
static	USHORT	fftSpectrumPeakCollect(				/* Spectrum peak frequency arrangement function				*/
					FFTANALYZE *FftAna,					/* Data for vibration frequency analysis					*/
					LONG *swork,						/* Spectrum peak integrated value					*/
					UCHAR sw,							/* Integral calculation processing changeover switch				*/
					LONG Fmax,							/* Analysis maximum frequency/number of frequency divisions				*/
					LONG Fmin );						/* Minimum analysis frequency/number of frequency divisions				*/
static	LONG	fftAddThta(							/* Phase increment calculation function								*/
					LONG thta_a,						/* Original phase									*/
					LONG thta_b,						/* Phase increment value								*/
					USHORT *sin_thta,					/* (Original phase + phase increment value):SINFor calculation			*/
					USHORT *cos_thta );					/* (Original phase + phase increment value +π/2):COSFor calculation	*/
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Vibration frequency analysis function initial parameter calculation function													*/
/*																									*/
/****************************************************************************************************/
void	FftCalculateInitPrm( FFTANALYZE *FftAna, LONG SampleTimeUs )
{
	/*----------------------------------------------------------------------------------------------*/
	/*		Vibration analysis pass/fail judgment spectrum correction value : FftAna->FftAnaPrm.Spectrumval						*/
	/*----------------------------------------------------------------------------------------------*/
//	KPI_DI();												/* Disable Interrupt					*/
	FftAna->FftAnaPrm.Spectrumval = ONFFT_OK_LMT;			/* [1x]Conversion is internal						*/
	FftAna->FftAnaPrm.Spectrumval2 = ONFFT_OK_LMT2;			/* [1x]Conversion is internal						*/
	FftAna->FftAnaPrm.SampleTime = SAMPLETIME(SampleTimeUs);
//	KPI_EI();												/* Enable Interrupt						*/

	return;
}



/****************************************************************************************************/
/*																									*/
/*		Vibration frequency analysis main function																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function:	FFTAlgorithm (improvedDFT) Find the frequency spectrum and perform the vibration frequency analysis.			*/
/*																									*/
/*		arg			: *TraceData	Trace data													*/
/*					: Sno			Number of data samples												*/
/*					: Fmax_calc		Analysis maximum frequency		[Hz]										*/
/*					: Fmin_calc		Analysis minimum frequency		[Hz]										*/
/*		out			: rc			Return code		[OK or NG]									*/
/* 		g-out		: *fpeak		Vibration frequency analysis value	[Hz]										*/
/****************************************************************************************************/
/*																									*/
/*		Ver.1.00 : 2004.02.02  K.Inoki	 															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
static	SHORT	fftAnalyzeCalc( FFTANALYZE *FftAna, SHORT *TraceData, LONG *fpeak )
{
	/*----------------------------------------------------------------------------------------------*/
	/*	Local variable definition																			*/
	/*----------------------------------------------------------------------------------------------*/
	USHORT	sft;
	USHORT	work1, work2;							/* Loop counter	Equivalent to k and n in the DFT general formula	*/
	USHORT	thh1, thh2;								/* sinX,cosXFor calculation.1024 = 2π					*/
	USHORT	cnt;									/* Number of combined peaks							*/
	LONG	Sno;									/* Number of data samples								*/
	LONG	Fmax_calc;								/* Analysis maximum frequency								*/
	LONG	Fmin_calc;								/* Analysis minimum frequency								*/
	LONG	Fdiv;									/* Frequency resolution									*/
	LONG	Fmax;									/* Analysis maximum frequency/number of frequency divisions					*/
	LONG	Fmin;									/* Minimum analysis frequency/number of frequency divisions					*/
	LONG	Sno_2;									/* Number of calculation divisions (number of samples for calculation)				*/

	LONG 	thta0, thtaw, thtaw2, dthta, thta;		/* Phase											*/
	LONG	rew, imw ,rew2, imw2;					/* A(k)~A(k+3)		*/
	LONG	res0, ims0;								/* A(k)The integrated value of the real part of each k, the integrated value of the imaginary part	*/
	LONG	res1, res11, ims1, ims11;				/* A(k+1)The integrated value of the real part of each k, the integrated value of the imaginary part	*/
	LONG	res2, ims2;								/* A(k+2)The integrated value of the real part of each k, the integrated value of the imaginary part	*/
	LONG	res3, res31, ims3 ,ims31;				/* A(k+3)The integrated value of the real part of each k, the integrated value of the imaginary part	*/
	LONG	res4, res5, res6, res7;					/* a(N/4)										*/
	LONG	ims4, ims5, ims6 ,ims7;

	LONG	x0;										/* A(k)~A(k+3)			*/
	LONG	smax;									/* A(k)~A(k+3)	*/
	ULONG	swork;									/* Spectrum value for each frequency						*/
	LONG	freq;									/* Detection frequency									*/
	LONG	swork2_32;								/* Spectrum integrated value work						*/
	ULONG	swork_sum;								/* Spectrum integrated value							*/
	ULONG	swork_av;								/* Spectrum average value							*/
	LONG	s,wk;									/* Calculation library work for frequency detection pass/fail determination	*/

	SHORT	rc;										/* Return code								*/
													/*	rc = 0  -> Normal end,rc = -1 -> Error termination	*/

	LONG	Sno_div2;								/* Sno/2		<V51E>							*/
	LONG	Sno_div4;								/* Sno/4		<V51E>							*/
	LONG	Sno_3div4;								/* Sno*3/4		<V51E>							*/

	/*----------------------------------------------------------------------------------------------*/
	/*	Data/variable initialization																		*/
	/*----------------------------------------------------------------------------------------------*/
	Sno = FftAna->FftAnaPrm.SumpleNum;				/* Number of data samples								*/
	Fmax_calc = FftAna->FftAnaPrm.AnaMaxFreq;		/* Analysis maximum frequency								*/
	Fmin_calc = FftAna->FftAnaPrm.AnaMinFreq;		/* Analysis minimum frequency								*/
	fftClearPeakDatas( FftAna );					/* Peak data clear function						*/
	cnt = 0;										/* Clear the number of combined peaks					*/
	swork_sum = 0;									/* Spectrum integrated value clear						*/
	smax = 0;										/* Spectrum maximum value clear						*/
	rc = OK;										/* Initialize return code to "normal completion"			*/

	/*----------------------------------------------------------------------------------------------*/
	/*	Calculation and setting of internal setting value																	*/
	/*----------------------------------------------------------------------------------------------*/
	Fdiv = FftAna->FftAnaPrm.SampleTime/Sno;		/* Frequency resolution	(1/Data acquisition cycle/Data count)		*/
	Fmax = Fmax_calc/Fdiv;							/* Analysis maximum frequency / Number of frequency divisions				*/
	Fmin = Fmin_calc/Fdiv;							/* Analysis minimum frequency / Number of frequency divisions				*/
	Sno_2 = Sno*2;									/* Number of calculation divisions (number of samples for calculation)				*/
	Sno_div2 = Sno>>1;								/* FFTIntermediate variable used in operation	Sno/2			*/
	Sno_div4 = Sno>>2;								/* FFTIntermediate variable used in operation	Sno/4			*/
	Sno_3div4 = (Sno*3)>>2;							/* FFTIntermediate variable used in operation	Sno*3/4			*/

	/*----------------------------------------------------------------------------------------------*/
	/*	Abnormal setting state detection																			*/
	/*  ①Upper limit of detection frequency is less than or equal to lower limit[(Fmax-Fmin)<=0]										*/
	/*	②Analysis maximum frequency 0 or less[Fmax_calc<=0], Analysis minimum frequency less than 0[Fmin_calc<0]						*/
	/*	In the above state, set the return code to NG as an abnormal setting.									*/
	/*	Frequency analysis value at abnormality isPn409upper limit (5000Hz)										*/
	/*	(Pn409=5000Hz  ⇒  1Stage notch filter disabled)													*/
	/*----------------------------------------------------------------------------------------------*/
	if( ((Fmax - Fmin) <= 0) || (Fmax_calc<=0) || (Fmin_calc<0) )
	{
		/* If an abnormal setting is made, a return code NG is returned */
		rc = NG;

		/* When an abnormal setting is made, the frequency analysis value is used.5000set */
		fpeak[0] = pndef_ntchfil1.UpperLimit;
		fpeak[1] = fpeak[0];
		fpeak[2] = fpeak[0];
		return ( rc );
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	High-speed DFT processing	Calculate A(k)																*/
	/* 		work1is incremented by 4 in one loop												*/
	/* 		Other than the abovework1++!										*/
	/*----------------------------------------------------------------------------------------------*/
	for( work1= (USHORT)Fmin; work1 < (USHORT)Fmax; work1++ ) {

		/*------------------------------------------------------------------------------------------*/
		/*	2πk/N calculation	thta0 = work1 * 1024 / SNO_2(sinXteethX=1024=2π)				*/
		/*------------------------------------------------------------------------------------------*/
		thta0= (work1 << 18) / Sno_2;				/* <<18later >>8 Therefore, the system flags[/79] [80] to stop commands[/78] [79]until					*/
													/* After all<<10 = 1024.sinXteethX=1024=2π	*/

		/*------------------------------------------------------------------------------------------*/
		/*	a(0),a(N/2)																		*/
		/*	"<<2" is changed to "Shift to improve accuracy														*/
		/*------------------------------------------------------------------------------------------*/
		/* k=nHour a(0)+a(N/2) */
		res0= ((SHORT)TraceData[0] + (SHORT)TraceData[Sno_div2]) << 2;
		/* k=n+1,n+3 a(0) */
		res1= res3= (SHORT)TraceData[0] << 2;
		/* k=n+2Hour a(0)-a(N/2) */
		res2= ((SHORT)TraceData[0] - (SHORT)TraceData[Sno_div2]) << 2;
		/* k=n+1,n+3 j×a(N/2) */
		ims11= ims31= (SHORT)TraceData[Sno_div2] << 2;

		/*------------------------------------------------------------------------------------------*/
		/*	Below, "work2" Initialization of variables in the loop														*/
		/*------------------------------------------------------------------------------------------*/
		thtaw = 0;									/* 2πnk / N							*/
		res11= res31= 0;
		ims0= ims1= ims2= ims3= 0;

		/*------------------------------------------------------------------------------------------*/
		/*	Σa(n)×exp(-j2πnk/N)												*/
		/*	In this loop,a(1)~a(N-1). However, calculate and multiply bya(N/4)								*/
		/*------------------------------------------------------------------------------------------*/
		for( work2= 1; work2 < Sno_div4; work2++ ) {
			/*--------------------------------------------------------------------------------------*/
			/*	2πk / N. Change the shift amount so that there is no loss of information in the calculation of phase increment value	dtata = work2 * 1024 / Sno_2						*/
			/*--------------------------------------------------------------------------------------*/
			dthta=  (work2 << 18) / Sno_2;			/* <<18later >>8 Therefore, the system flags[/79] [80] to stop commands[/78] [79]until					*/
													/* After all<<10 = 1024.sinXteethX=1024=2π	*/

			/*--------------------------------------------------------------------------------------*/
			/*	DFTCalculation of common term for calculation																*/
			/* 		a(work2) + a(N-work2)															*/
			/* 		a(N/2+work2) + a(N/2-work2)														*/
			/* 		a(work2) - a(N-work2)															*/
			/* 		a(N/2+work2) - a(N/2-work2)														*/
			/*--------------------------------------------------------------------------------------*/
			rew= (SHORT)TraceData[work2] + (SHORT)TraceData[Sno - work2];
			rew2 = ((SHORT)TraceData[Sno_div2 + work2] + (SHORT)TraceData[Sno_div2 - work2]);
			imw= (SHORT)TraceData[work2] - (SHORT)TraceData[Sno - work2];
			imw2 = ((SHORT)TraceData[Sno_div2 + work2] - (SHORT)TraceData[Sno_div2 - work2]);

			/*--------------------------------------------------------------------------------------*/
			/*	A(k)Calculation																			*/
			/*		sinis0~16384Because of ">>12" to prevent the value from becoming large			*/
			/*--------------------------------------------------------------------------------------*/
			thtaw = fftAddThta( thta0, thtaw, &thh1, &thh2 );		/* thtaw += thta0 				*/

			res0 += (((SHORT)(rew + rew2) * MlibFASTSINS16(thh2)) >> 12);
			ims0 += (((SHORT)(imw + imw2) * MlibFASTSINS16(thh1)) >> 12);

			/*--------------------------------------------------------------------------------------*/
			/*	A(k+1)Calculation																		*/
			/*		sinis0~16384Because of ">>12" to prevent the value from becoming large			*/
			/*--------------------------------------------------------------------------------------*/
			thtaw2 = fftAddThta( dthta, thtaw, &thh1, &thh2 );		/* thtaw2 = thtaw + dthta		*/

			res1 += (((SHORT)imw * MlibFASTSINS16(thh2)) >> 12);
			ims1 += (((SHORT)rew * MlibFASTSINS16(thh1)) >> 12);
			res11 += (((SHORT)imw2 * MlibFASTSINS16(thh1)) >> 12);
			ims11 += (((SHORT)rew2 * MlibFASTSINS16(thh2)) >> 12);

			/*--------------------------------------------------------------------------------------*/
			/*	A(k+2)Calculation																		*/
			/*		sinis0~16384Because of ">>12" to prevent the value from becoming large			*/
			/*--------------------------------------------------------------------------------------*/
			thtaw2 = fftAddThta( dthta, thtaw2, &thh1, &thh2 );		/* thtaw2 = thtaw2 + dthta		*/

			res2 += (((SHORT)(rew - rew2) * MlibFASTSINS16(thh2)) >> 12);
			ims2 += (((SHORT)(imw - imw2) * MlibFASTSINS16(thh1)) >> 12);

			/*--------------------------------------------------------------------------------------*/
			/*	A(k+3)Calculation																		*/
			/*		sinis0~16384Because of ">>12" to prevent the value from becoming large			*/
			/*--------------------------------------------------------------------------------------*/
			thtaw2 = fftAddThta( dthta, thtaw2, &thh1, &thh2 );		/* thtaw2 = thtaw2 + dthta		*/

			res3 += (((SHORT)imw * MlibFASTSINS16(thh2)) >> 12);
			ims3 += (((SHORT)rew * MlibFASTSINS16(thh1)) >> 12);
			res31 += (((SHORT)imw2 * MlibFASTSINS16(thh1)) >> 12);
			ims31 += (((SHORT)rew2 * MlibFASTSINS16(thh2)) >> 12);

		}

		/*------------------------------------------------------------------------------------------*/
		/*	a(N/4)																				*/
		/*		Snois not divisible by 4, we will calculate it here. Without this, calculation errors will occur.	*/
		/*		sinis0~16384Because of ">>12" to prevent the value from becoming large				*/
		/*------------------------------------------------------------------------------------------*/
		rew= (SHORT)TraceData[Sno_div4] + (SHORT)TraceData[Sno_3div4];
		imw= (SHORT)TraceData[Sno_div4] - (SHORT)TraceData[Sno_3div4];

		thta= work1 << 7;										/* π/4 × work1 					*/
																/* 2π×(Sno/4)/Sno_2=π/4 -> 128	*/
		thta = fftAddThta( thta, 0, &thh1, &thh2 );				/* thta = thta + 0					*/
		res4 = (((SHORT)rew * MlibFASTSINS16(thh2)) >> 12);		/* sin(thh2) = cos(thh)				*/
		ims4 = (((SHORT)imw * MlibFASTSINS16(thh1)) >> 12);
		res6 = (((SHORT)rew * MlibFASTSINS16(thh1)) >> 12);
		ims6 = (((SHORT)imw * MlibFASTSINS16(thh2)) >> 12);

		thta = fftAddThta( thta, 128, &thh1, &thh2 );			/* thta = thta + π/4				*/
		res5 = (((SHORT)imw * MlibFASTSINS16(thh2)) >> 12);
		ims5 = (((SHORT)rew * MlibFASTSINS16(thh1)) >> 12);
		res7 = (((SHORT)imw * MlibFASTSINS16(thh1)) >> 12);
		ims7 = (((SHORT)rew * MlibFASTSINS16(thh2)) >> 12);

		/*------------------------------------------------------------------------------------------*/
		/*	Calculation of real and imaginary parts																	*/
		/*		Calculate the real part and imaginary part of A(k) to A(k+3).								*/
		/*		Calculate the absolute value and save the maximum value. It will be used later as a standard for normalization when calculating spectrum values.	*/
		/*------------------------------------------------------------------------------------------*/
		/*------------------------------------------------------------------------------------------*/
		/*	A(k)																			*/
		/*------------------------------------------------------------------------------------------*/
		FftAna->var.Reals[work1]= res0 + res4;
		x0 = MlibAbs32(FftAna->var.Reals[work1]);
		if( x0 > smax ) smax = x0;

		/*------------------------------------------------------------------------------------------*/
		/*	A(k)																			*/
		/*------------------------------------------------------------------------------------------*/
		FftAna->var.Images[work1]= ims0 + ims4;
		x0 = MlibAbs32(FftAna->var.Images[work1]);
		if( x0 > smax ) smax = x0;

		/*------------------------------------------------------------------------------------------*/
		/*	A(k+1)																		*/
		/*------------------------------------------------------------------------------------------*/
		work1 ++;												/* work1 Increment				*/
		FftAna->var.Reals[work1]= res1 - res11 + res5;
		x0 = MlibAbs32(FftAna->var.Reals[work1]);
		if( x0 > smax ) smax = x0;

		/*------------------------------------------------------------------------------------------*/
		/*	A(k+1)																		*/
		/*------------------------------------------------------------------------------------------*/
		FftAna->var.Images[work1]= ims1 + ims11 + ims5;
		x0 = MlibAbs32(FftAna->var.Images[work1]);
		if( x0 > smax ) smax = x0;

		/*------------------------------------------------------------------------------------------*/
		/*	A(k+2)																		*/
		/*------------------------------------------------------------------------------------------*/
		work1 ++;												/* work1 Increment				*/
		FftAna->var.Reals[work1]= res2 - res6;
		x0 = MlibAbs32(FftAna->var.Reals[work1]);
		if( x0 > smax ) smax = x0;

		/*------------------------------------------------------------------------------------------*/
		/*	A(k+2)																		*/
		/*------------------------------------------------------------------------------------------*/
		FftAna->var.Images[work1]= ims2 + ims6;
		x0 = MlibAbs32(FftAna->var.Images[work1]);
		if( x0 > smax ) smax = x0;

		/*------------------------------------------------------------------------------------------*/
		/*	A(k+3)																		*/
		/*------------------------------------------------------------------------------------------*/
		work1 ++;												/* work1 Increment				*/
		FftAna->var.Reals[work1]= res3 + res31 - res7;
		x0 = MlibAbs32(FftAna->var.Reals[work1]);
		if( x0 > smax ) smax = x0;

		/*------------------------------------------------------------------------------------------*/
		/*	A(k+3)																		*/
		/*------------------------------------------------------------------------------------------*/
		FftAna->var.Images[work1]= ims3 - ims31 + ims7;
		x0 = MlibAbs32(FftAna->var.Images[work1]);
		if( x0 > smax ) smax = x0;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Normalization processing																					*/
	/*----------------------------------------------------------------------------------------------*/
	sft = (USHORT)(smax / REGULAR_SFT);							/* 16384normalized with					*/

	/*----------------------------------------------------------------------------------------------*/
	/*	Spectrum analysis (|Re|^2+|Im|^2)														*/
	/*----------------------------------------------------------------------------------------------*/
	for( work1= (USHORT)Fmin; work1 < (USHORT)Fmax; work1++ ) {	
		/*------------------------------------------------------------------------------------------*/
		/*	Spectrum calculation																		*/
		/*------------------------------------------------------------------------------------------*/
		if(sft == 0){
			swork = FftAna->var.Reals[work1] * FftAna->var.Reals[work1] ;
			swork += FftAna->var.Images[work1] * FftAna->var.Images[work1] ;
			sft = 1;
		}
		else{
			swork = (FftAna->var.Reals[work1] / sft) * (FftAna->var.Reals[work1] / sft);
			swork += ((FftAna->var.Images[work1] / sft) * (FftAna->var.Images[work1] / sft));
		}
		/*------------------------------------------------------------------------------------------*/
		/*	Frequency calculation																				*/
		/*------------------------------------------------------------------------------------------*/
		freq = (LONG)work1 * Fdiv;				

		/*------------------------------------------------------------------------------------------*/
		/*	Spectrum peak value detection																*/
		/*		Retain the peak values and frequencies of the top five.											*/
		/*------------------------------------------------------------------------------------------*/
		fftSpectrumPeakDetect( FftAna, swork, freq );			/* Spectrum peak detection			*/

		/*------------------------------------------------------------------------------------------*/
		/*	Spectrum peak value arrangement																*/
		/*		Once you reach the intermediate frequency of the analysis, summarize the nearby frequencies.							*/
		/*		Group the peak values into the top three													*/
		/*------------------------------------------------------------------------------------------*/
		if( work1 == (Fmax>>1) )
		{
			/* Detection frequency arrangement */
			cnt = fftSpectrumPeakCollect( FftAna, &swork2_32, 0, Fmax, Fmin );
			/* Omit the spectrum of the summarized frequencies from the average value calculation*/
			swork_sum -= swork2_32;
		}
		/* (Fmax-Fmin)is to prevent overflow and calculate the average value. */
		swork_sum += (swork / (Fmax - Fmin));
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Spectrum peak value arrangement (combining frequencies that are close to each other among peak values)								*/
	/*----------------------------------------------------------------------------------------------*/
	/* Detection frequency arrangement */
	cnt += fftSpectrumPeakCollect( FftAna,&swork2_32, 0, Fmax, Fmin );
	/* Omit all peak spectra from the average value calculation */
	swork_sum -= swork2_32;
	
	/*----------------------------------------------------------------------------------------------*/
	/*	Frequency detection pass/fail determination																			*/
	/*		If the following conditions are met, frequency detection will fail.												*/
	/*			.When the peak frequency is the lower limit of detection frequency(MINIMAM_FREQ[Hz])In the following cases						*/
	/*			.Spectral peak(smax)is the lower limit(MINIMAM_SPECTRUM)In the following cases						*/
	/*			.The spectrum value of the detection frequency is the average value of the spectrumPn464[%]					*/
	/*----------------------------------------------------------------------------------------------*/
	if((smax <= MINIMAM_SPECTRUM))
	{
		rc = NG;
	}
	/*----------------------------------------------------------------------------------------------*/
	/* Comparison of spectrum peak and average value																*/
	/* 		The average value is It is necessary to divide the integrated value by the number of loops, butswork_sumis the number of loops(Fmax - Fmin)	*/
	/*		is integrated,swork_sumitself is the average value.							*/
	/* 		However, if you consider the number of spectra summarized in the detection frequency organizing process, the average value will be as shown in the formula below	*/
	/*		Average value = swork_sum * (Fmax - Fmin) / (Fmax - Fmin - cnt)								*/
	/*----------------------------------------------------------------------------------------------*/
	s = 0;
	wk =MlibScalKxgain( (Fmax - Fmin), 1 , (Fmax - Fmin - cnt), &s, 24 );
#if (FLOAT_USE==TRUE)
	swork_av = (ULONG)( swork_sum * wk );
#else
	swork_av = (ULONG)MlibMulgain( swork_sum, wk );
#endif /* FLOAT_USE */

	if((swork_av * FftAna->FftAnaPrm.Spectrumval) > FftAna->var.swork_max[0])
	{ /* Error termination */
		rc = NG;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Adjustment-free online automatic notch frequency detection pass/fail judgment											*/
	/*----------------------------------------------------------------------------------------------*/
	if( (FftAna->FftAnaPrm.TuneLessSts == TRUE) && (FftAna->FftAnaPrm.ActiveFFtOn == FALSE) )
	{ /* For adjustment-less online automatic notch setting */
		if( (smax <= MINIMAM_SPECTRUM2) )
		{ /* Below spectrum peak */
			/* Error termination */
			rc = NG;
		}

		if( (swork_av * FftAna->FftAnaPrm.Spectrumval2) > FftAna->var.swork_max[0] )
		{ /* Peak frequency is below the specified magnification */
			/* Error termination */
			rc = NG;
		}
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Reset detection frequency by comparing with average value															*/
	/*----------------------------------------------------------------------------------------------*/
	if( (swork_av * FftAna->FftAnaPrm.Spectrumval) > FftAna->var.swork_max[0] )
	{
		FftAna->var.freq_max[0] = 0;
		FftAna->var.swork_max[0] = 0;
	}
	if( (swork_av * FftAna->FftAnaPrm.Spectrumval) > FftAna->var.swork_max[1] )
	{
		FftAna->var.freq_max[1] = 0;
		FftAna->var.swork_max[1] = 0;
	}
	if( (swork_av * FftAna->FftAnaPrm.Spectrumval) > FftAna->var.swork_max[2] )
	{
		FftAna->var.freq_max[2] = 0;
		FftAna->var.swork_max[2] = 0;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Set detection frequency																			*/
	/*----------------------------------------------------------------------------------------------*/
	fpeak[0] = FftAna->var.freq_max[0];
	fpeak[1] = FftAna->var.freq_max[1];
	fpeak[2] = FftAna->var.freq_max[2];

	return(rc);
}



/****************************************************************************************************/
/*																									*/
/*		Peak data clear function																		*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function:	Initialization of peak spectrum and its frequency variable								*/
/*																									*/
/*		arg			: None																			*/
/*		out			: None																			*/
/* 		g-out		: None																			*/
/****************************************************************************************************/
/*																									*/
/*		Ver.1.00 : 2004.02.02  K.Inoki	 															*/
/*																									*/
/****************************************************************************************************/
static	void	fftClearPeakDatas( FFTANALYZE *FftAna )
{
	MlibResetLongMemory( &(FftAna->var), sizeof(FftAna->var)>>2 );

	/* Spectrum minimum value initial value setting */
	FftAna->var.swork_min = 0xFFFFFFFF;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Spectrum peak frequency detection function															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function:	Detect the top five spectrum peaks and their frequencies.					*/
/*																									*/
/*		arg			: swork		Spectrum value														*/
/*					: freq		Frequencies corresponding to the spectrum										*/
/*		out			: None																			*/
/* 		g-out		: FftAna->var.swork_max[PEAK_DETECT_NUM]	Spectrum peak data			*/
/*					: FftAna->var.freq_max[PEAK_DETECT_NUM]		Frequency peak data					*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Ver.1.00 : 2004.02.02  K.Inoki	 															*/
/*																									*/
/****************************************************************************************************/
static	void	fftSpectrumPeakDetect( FFTANALYZE *FftAna, ULONG swork, ULONG freq )
{
	UCHAR	i;

	/*----------------------------------------------------------------------------------------------*/
	/*	Maximum spectrum peak value detection																*/
	/*		→Update top 1st to 5th														*/
	/*----------------------------------------------------------------------------------------------*/
	if( swork > FftAna->var.swork_max[0] )
	{
		for(i = 4; i > 0; i--)
		{
			FftAna->var.swork_max[i] = FftAna->var.swork_max[i-1];
			FftAna->var.freq_max[i] = FftAna->var.freq_max[i-1];
		}
		FftAna->var.swork_max[0] = swork;
		FftAna->var.freq_max[0] = freq;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Second spectrum peak value detection																*/
	/*		→Update top 2nd to 5th														*/
	/*----------------------------------------------------------------------------------------------*/
	else if( swork > FftAna->var.swork_max[1] )
	{
		for(i = 4; i > 1; i--)
		{
			FftAna->var.swork_max[i] = FftAna->var.swork_max[i-1];
			FftAna->var.freq_max[i] = FftAna->var.freq_max[i-1];
		}
		FftAna->var.swork_max[1] = swork;
		FftAna->var.freq_max[1] = freq;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Third spectrum peak value detection																*/
	/*		→Update top 3rd to 5th														*/
	/*----------------------------------------------------------------------------------------------*/
	else if( swork > FftAna->var.swork_max[2] )
	{
		for(i = 4; i > 2; i--)
		{
			FftAna->var.swork_max[i] = FftAna->var.swork_max[i-1];
			FftAna->var.freq_max[i] = FftAna->var.freq_max[i-1];
		}
		FftAna->var.swork_max[2]= swork;
		FftAna->var.freq_max[2]= freq;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	4th spectrum peak value detection																*/
	/*		→Update top 4th and 5th																*/
	/*----------------------------------------------------------------------------------------------*/
	else if( swork > FftAna->var.swork_max[3] )
	{
		FftAna->var.swork_max[4] = FftAna->var.swork_max[3];
		FftAna->var.swork_max[3] = swork;
		FftAna->var.freq_max[4] = FftAna->var.freq_max[3];
		FftAna->var.freq_max[3] = freq;
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	5th spectrum peak value detection																*/
	/*		→5th update																			*/
	/*----------------------------------------------------------------------------------------------*/
	else if( swork > FftAna->var.swork_max[4] )
	{
		FftAna->var.swork_max[4] = swork;
		FftAna->var.freq_max[4] = freq;
	}

	/*----------------------------------------------------------------------------------------------*/
	/*	Spectrum minimum value detection																		*/
	/*----------------------------------------------------------------------------------------------*/
	if( swork < FftAna->var.swork_min )
	{
		FftAna->var.swork_min = swork;
		FftAna->var.freq_min = freq;
	}
}



/****************************************************************************************************/
/*																									*/
/*		Spectrum peak frequency arrangement function															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function:	Among the top five spectrum peaks,±100[Hz]			*/
/*			Integrate frequencies with large spectrum and summarize up to top three								*/
/*			The 4th and 5th frequencies and spectrum are cleared to zero.								*/
/*		 	Also, the integrated value of the spectrum before and after this function processing is calculated for the 1st to 5th spectrum.	*/
/*			(Used in spectrum average value calculation)															*/
/*																									*/
/*		arg			: *swork	Spectrum peak integrated value(swRecalculate the lower [/72] [73] values of the [/70] [71] set of values[/71] [72]values until the calculation results differ)					*/
/*					: sw		Integral calculation processing changeover switch										*/
/*									= 0:*swork= Difference between integrated value of peak spectrum before and after combining		*/
/*									≠0:*swork= Integrated value of peak spectrum before combining				*/
/*		out			: cnt		Number of combined peaks												*/
/* 		g-out		: FftAna->var.swork_max[PEAK_DETECT_NUM]	Spectrum peak data			*/
/*					: FftAna->var.freq_max[PEAK_DETECT_NUM]		Frequency peak data					*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Ver.1.00 : 2004.02.02  K.Inoki	 															*/
/*																									*/
/****************************************************************************************************/
static	USHORT	fftSpectrumPeakCollect( FFTANALYZE *FftAna, LONG *swork32, UCHAR sw, LONG Fmax, LONG Fmin )
{
	USHORT	i;
	USHORT	cnt;

	/*----------------------------------------------------------------------------------------------*/
	/*	Initializing variables																				*/
	/*----------------------------------------------------------------------------------------------*/
	cnt = 0;												/* Clear spectrum peak number counter		*/
	*swork32 = 0;											/* Spectrum peak integrated value				*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Integrating peak frequency spectrum															*/
	/*		Averaging and integrating the integrated value as an overflow countermeasure(Fmax - Fmin)						*/
	/*----------------------------------------------------------------------------------------------*/
	for(i = 0; i < PEAK_DETECT_NUM; i ++)
	{
		if( FftAna->var.swork_max[i] != 0 )
		{ /* Spectrum peak present */
			cnt++;
			*swork32 += FftAna->var.swork_max[i] / (Fmax - Fmin);
		}
	}

	/************************************************************************************************/
	/*	FftAna->var.freq_max[1]check															*/
	/************************************************************************************************/
	/*----------------------------------------------------------------------------------------------*/
	/* 		FftAna->var.freq_max[0] and FftAna->var.freq_max[1] Difference100[Hz] 						*/
	/*			→ FftAna->var.freq_max[1]													*/
	/*----------------------------------------------------------------------------------------------*/
	if( MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[1]) < 100 )
	{
		FftAna->var.swork_max[1]= 0;
		FftAna->var.freq_max[1]= 0;
	}
	
	/************************************************************************************************/
	/*	FftAna->var.freq_max[2]check															*/
	/************************************************************************************************/
	/*----------------------------------------------------------------------------------------------*/
	/* 		FftAna->var.freq_max[1]is not grouped												*/
	/*----------------------------------------------------------------------------------------------*/
	if( FftAna->var.freq_max[1] != 0 )
	 {
		/*------------------------------------------------------------------------------------------*/
		/* 	FftAna->var.freq_max[2]andFftAna->var.freq_max[0]andFftAna->var.freq_max[1]			*/
		/*		because100[Hz]	→ FftAna->var.freq_max[2]						*/
		/*------------------------------------------------------------------------------------------*/
		if( (MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[2]) < 100) 
			|| (MlibAbs32(FftAna->var.freq_max[1] - FftAna->var.freq_max[2]) < 100) )
		{
			FftAna->var.swork_max[2]= 0;
			FftAna->var.freq_max[2]= 0;
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/* 		FftAna->var.freq_max[1]is grouped													*/
	/*----------------------------------------------------------------------------------------------*/
	else
	{
		/*------------------------------------------------------------------------------------------*/
		/* 	FftAna->var.freq_max[2] andFftAna->var.freq_max[0]100[Hz] 						*/
		/*		→ FftAna->var.freq_max[2]													*/
		/*------------------------------------------------------------------------------------------*/
		if( MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[2]) < 100 )
		{
			FftAna->var.swork_max[2]= 0;
			FftAna->var.freq_max[2]= 0;
		}
	}

	/************************************************************************************************/
	/*	FftAna->var.freq_max[3],[4]check														*/
	/************************************************************************************************/
	/*----------------------------------------------------------------------------------------------*/
	/* 		FftAna->var.freq_max[1]is not grouped												*/
	/*----------------------------------------------------------------------------------------------*/
	if( FftAna->var.freq_max[1] != 0 )
	{
		/*------------------------------------------------------------------------------------------*/
		/* 	FftAna->var.freq_max[2]is not grouped												*/
		/*		→ FftAna->var.freq_max[0]~[2]It has been confirmed thatFftAna->var.freq_max[3],[4]	*/
		/*------------------------------------------------------------------------------------------*/
		if( FftAna->var.freq_max[2] != 0 )
		{
			FftAna->var.swork_max[3]= 0;
			FftAna->var.swork_max[4]= 0;
			FftAna->var.freq_max[3]= 0;
			FftAna->var.freq_max[4]= 0;
		}
		/*------------------------------------------------------------------------------------------*/
		/* 	FftAna->var.freq_max[1] != 0, FftAna->var.freq_max[2]= 0								*/
		/*		→ FftAna->var.freq_max[2]is unconfirmed													*/
		/*------------------------------------------------------------------------------------------*/
		else
		{
			/*--------------------------------------------------------------------------------------*/
			/* 	FftAna->var.freq_max[3]andFftAna->var.freq_max[0]or								*/
			/*											FftAna->var.freq_max[1]100[Hz] 	*/
			/*		→ FftAna->var.freq_max[3]												*/
			/*--------------------------------------------------------------------------------------*/
			if( (MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[3]) < 100) 
				|| (MlibAbs32(FftAna->var.freq_max[1] - FftAna->var.freq_max[3]) < 100) )
			{	
				FftAna->var.swork_max[3]= 0;
				FftAna->var.freq_max[3]= 0;
				/*----------------------------------------------------------------------------------*/
				/* 	FftAna->var.freq_max[4]andFftAna->var.freq_max[0],FftAna->var.freq_max[1]	*/
				/*		100[Hz] → FftAna->var.freq_max[4]ofFftAna->var.freq_max[2]	*/
				/*----------------------------------------------------------------------------------*/
				if( (MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[4]) > 100) 
					&& (MlibAbs32(FftAna->var.freq_max[1] - FftAna->var.freq_max[4]) > 100) )
				{
					FftAna->var.swork_max[2]= FftAna->var.swork_max[4];
					FftAna->var.freq_max[2]= FftAna->var.freq_max[4];
				}
				/*----------------------------------------------------------------------------------*/
				/* FftAna->var.freq_max[0]~[2]It has been confirmed thatFftAna->var.freq_max[4]		*/
				/*	(FftAna->var.freq_max[3]is already cleared)								*/
				/*----------------------------------------------------------------------------------*/
				FftAna->var.swork_max[4]= 0;
				FftAna->var.freq_max[4]= 0;
			}
			/*--------------------------------------------------------------------------------------*/
			/* FftAna->var.freq_max[3] ofFftAna->var.freq_max[2] 								*/
			/*--------------------------------------------------------------------------------------*/
			else
			{
				FftAna->var.swork_max[2]= FftAna->var.swork_max[3];
				FftAna->var.freq_max[2]= FftAna->var.freq_max[3];
				/*----------------------------------------------------------------------------------*/
				/* FftAna->var.freq_max[0]~[2]It has been confirmed thatFftAna->var.freq_max[3],[4]	*/
				/*----------------------------------------------------------------------------------*/
				FftAna->var.swork_max[3]= 0;
				FftAna->var.freq_max[3]= 0;
				FftAna->var.swork_max[4]= 0;
				FftAna->var.freq_max[4]= 0;
			}
		}
	}

	/*----------------------------------------------------------------------------------------------*/
	/* 		FftAna->var.freq_max[1]is grouped													*/
	/*----------------------------------------------------------------------------------------------*/
	else
	{
		/*------------------------------------------------------------------------------------------*/
		/* 	FftAna->var.freq_max[2]is not grouped												*/
		/*  	→ FftAna->var.freq_max[2]ofFftAna->var.freq_max[1]							*/
		/*------------------------------------------------------------------------------------------*/
		if( FftAna->var.freq_max[2] != 0 )
		{
			FftAna->var.swork_max[1]= FftAna->var.swork_max[2];
			FftAna->var.freq_max[1]= FftAna->var.freq_max[2];
			/*--------------------------------------------------------------------------------------*/
			/* 	FftAna->var.freq_max[3]andFftAna->var.freq_max[0]or								*/
			/*											FftAna->var.freq_max[2]and100[Hz]	*/
			/*		→ FftAna->var.freq_max[3]												*/
			/*--------------------------------------------------------------------------------------*/
			if( (MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[3]) < 100) 
				|| (MlibAbs32(FftAna->var.freq_max[2] - FftAna->var.freq_max[3]) < 100) )
			{
				FftAna->var.swork_max[3]= 0;
				FftAna->var.freq_max[3]= 0;
				/*----------------------------------------------------------------------------------*/
				/* 	FftAna->var.freq_max[4]andFftAna->var.freq_max[0]or							*/
				/*										FftAna->var.freq_max[2]and100[Hz]	*/
				/*		→ FftAna->var.freq_max[4]											*/
				/*		   FftAna->var.freq_max[3],[4], so it can be connected in series withFftAna->var.freq_max[2]	*/
				/*		   																	*/
				/*----------------------------------------------------------------------------------*/
				if( (MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[4]) < 100) 
					|| (MlibAbs32(FftAna->var.freq_max[2] - FftAna->var.freq_max[4]) < 100) )
				{
					FftAna->var.swork_max[2]= 0;
					FftAna->var.freq_max[2]= 0;
				}
				/*----------------------------------------------------------------------------------*/
				/* 	FftAna->var.freq_max[4] ofFftAna->var.freq_max[2] 						*/
				/*----------------------------------------------------------------------------------*/
				else
				{
					FftAna->var.swork_max[2]= FftAna->var.swork_max[4];
					FftAna->var.freq_max[2]= FftAna->var.freq_max[4];
				}
				/*----------------------------------------------------------------------------------*/
				/* 	FftAna->var.freq_max[0]~[2]It has been confirmed thatFftAna->var.freq_max[4]		*/
				/*		(FftAna->var.freq_max[3]is already cleared)							*/
				/*----------------------------------------------------------------------------------*/
				FftAna->var.swork_max[4]= 0;
				FftAna->var.freq_max[4]= 0;
			}
			/*--------------------------------------------------------------------------------------*/
			/* 	FftAna->var.freq_max[3] ofFftAna->var.freq_max[2] 							*/
			/*--------------------------------------------------------------------------------------*/
			else
			{
				FftAna->var.swork_max[2]= FftAna->var.swork_max[3];
				FftAna->var.freq_max[2]= FftAna->var.freq_max[3];
				/*----------------------------------------------------------------------------------*/
				/* FftAna->var.freq_max[0]~[2]It has been confirmed thatFftAna->var.freq_max[3],[4]Clear	*/
				/*----------------------------------------------------------------------------------*/
				FftAna->var.swork_max[3]= 0;
				FftAna->var.freq_max[3]= 0;
				FftAna->var.swork_max[4]= 0;
				FftAna->var.freq_max[4]= 0;
			}
		}
		/*------------------------------------------------------------------------------------------*/
		/* 	FftAna->var.freq_max[1],[2]is grouped											*/
		/*------------------------------------------------------------------------------------------*/
		else
		{
			/*--------------------------------------------------------------------------------------*/
			/* 	FftAna->var.freq_max[3]andFftAna->var.freq_max[0]and100[Hz]					*/
			/*			→ FftAna->var.freq_max[3]											*/
			/*--------------------------------------------------------------------------------------*/
			if( MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[3]) < 100 )
			{
				FftAna->var.swork_max[3]= 0;
				FftAna->var.freq_max[3]= 0;
				/*----------------------------------------------------------------------------------*/
				/* 	FftAna->var.freq_max[4]andFftAna->var.freq_max[0]and100[Hz]		*/
				/*			→ FftAna->var.freq_max[4]ofFftAna->var.freq_max[1]				*/
				/*----------------------------------------------------------------------------------*/
				if( MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[4]) > 100 )
				{
					FftAna->var.swork_max[1]= FftAna->var.swork_max[4];
					FftAna->var.freq_max[1]= FftAna->var.freq_max[4];
				}
				/*----------------------------------------------------------------------------------*/
				/* FftAna->var.freq_max[0],[1]It has been confirmed thatFftAna->var.freq_max[4]		*/
				/*		(FftAna->var.freq_max[2],[3]is already cleared)						*/
				/*----------------------------------------------------------------------------------*/
				FftAna->var.swork_max[4]= 0;
				FftAna->var.freq_max[4]= 0;
			}
			/*--------------------------------------------------------------------------------------*/
			/* FftAna->var.freq_max[1],[2]is grouped, butFftAna->var.freq_max[3]but			*/
			/*																	Not combined	*/
			/*			→ FftAna->var.freq_max[3] ofFftAna->var.freq_max[1] 					*/
			/*--------------------------------------------------------------------------------------*/
			else
			{
				FftAna->var.swork_max[1]= FftAna->var.swork_max[3];
				FftAna->var.freq_max[1]= FftAna->var.freq_max[3];
				FftAna->var.swork_max[3]= 0;
				FftAna->var.freq_max[3]= 0;
				/*----------------------------------------------------------------------------------*/
				/* FftAna->var.freq_max[4]andFftAna->var.freq_max[0],FftAna->var.freq_max[1]and	*/
				/*															Difference100[Hz]	*/
				/*			→ FftAna->var.freq_max[4]ofFftAna->var.freq_max[2]				*/
				/*----------------------------------------------------------------------------------*/
				if( (MlibAbs32(FftAna->var.freq_max[0] - FftAna->var.freq_max[4]) > 100) 
					&& (MlibAbs32(FftAna->var.freq_max[1] - FftAna->var.freq_max[4]) > 100) )
				{
					FftAna->var.swork_max[2]= FftAna->var.swork_max[4];
					FftAna->var.freq_max[2]= FftAna->var.freq_max[4];
				}
				/*----------------------------------------------------------------------------------*/
				/* FftAna->var.freq_max[0]~[2]It has been confirmed thatFftAna->var.freq_max[4]		*/
				/*		(FftAna->var.freq_max[3]is already cleared)							*/
				/*----------------------------------------------------------------------------------*/
				FftAna->var.swork_max[4]= 0;
				FftAna->var.freq_max[4]= 0;
			}
		}
	}
	/*----------------------------------------------------------------------------------------------*/
	/*	Subtract the spectrum of the peak frequency after collecting the detection frequencies									*/
	/* 	 	→	Difference in spectral peak before and after combining detection frequencies											*/
	/*	 	→	Spectrum integrated value of combined frequencies													*/
	/* 	Also, Detect the number of combined peaks														*/
	/*		Averaging(Fmax - Fmin)															*/
	/*----------------------------------------------------------------------------------------------*/
	if( sw == 0 )										/* Difference between peak spectrum integrated values before and after combining	*/
	{
		for(i = 0; i < 5; i ++)
		{
			if( FftAna->var.swork_max[i] != 0 )
			{ /* Spectrum peak present */
				cnt--;
				*swork32 -= FftAna->var.swork_max[i] / (Fmax - Fmin);
			}
		}
	}
	return( cnt );
}



/****************************************************************************************************/
/*																									*/
/*		Phase calculation function	 																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function:	FFTcalculation.												*/
/*			thta1[deg]	= thta_a+thta_b															*/
/*			thta2[deg]	= thta1[deg] + 90[deg]	(1024 = 2π)										*/
/*																									*/
/*		arg			: thta_a	Original phase	  :thta(0~2π -> 0~262144In cases where normalization)						*/
/*					: thta_b	Phase increment value:thta(0~2π -> 0~262144In cases where normalization)						*/
/*					: *sin_thta	(Original phase + phase increment value +π/4):thta(0~2π -> 0~1024In cases where normalization)			*/
/*					: *cos_thta	(Original phase + phase increment value +π/4+π/2):thta(0~2π -> 0~1024In cases where normalization)	*/
/*		out			: thta_c	Original phase + phase increment value:thta(0~2π -> 0~262144In cases where normalization)				*/
/* 		g-out		: None																			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Ver.1.00 : 2004.02.02  K.Inoki	 															*/
/*																									*/
/****************************************************************************************************/
static	LONG	fftAddThta( LONG thta_a, LONG thta_b, USHORT *sin_thta_c, USHORT *cos_thta_c )
{
	LONG	thta1, thta2;
	LONG	thta_c;

	/*----------------------------------------------------------------------------------------------*/
	/*	Original phase + phase increment value	thta(0~2π -> 0~262144Normalization)											*/
	/*----------------------------------------------------------------------------------------------*/
	thta_c = thta_a + thta_b;

	thta1 = (thta_c +128) >> 8;								/* >>8,thta->1024=2πNormalization	*/
	*sin_thta_c = (USHORT)thta1;

	thta2 = (thta1 + 256);									/* thta1+256 -> 90degAdvance				*/
	*cos_thta_c = (USHORT)thta2;

	return( thta_c );
}



/****************************************************************************************************/
/*																									*/
/*		Vibration frequency analysis main function																	*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Function:	FFTAlgorithm (improvedDFT) Find the frequency spectrum and perform the vibration frequency analysis.			*/
/*																									*/
/*		arg			: *TraceData	Trace data													*/
/*					: Sno			Number of data samples												*/
/*					: Fmax_calc		Analysis maximum frequency		[Hz]										*/
/*					: Fmin_calc		Analysis minimum frequency		[Hz]										*/
/*		out			: rc			Return code		[OK or NG]									*/
/* 		g-out		: *ResSet		Resonance frequency analysis value	[Hz/-]										*/
/*					: *AntSet		Anti-resonance frequency analysis value	[Hz/-]										*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		Ver.1.00 : 2004.02.02  K.Inoki	 															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
SHORT	FftAnalyzeExecute( FFTANALYZE *FftAna, SHORT *TraceData )
{
	LONG	fpeak[3];
	LONG	Fmax_calc;
	LONG	Fmin_calc;
	FRQANA	*ResSet;
	FRQANA	*AntSet;
	SHORT	rc;

	Fmax_calc = FftAna->FftAnaPrm.AnaMaxFreq;
	Fmin_calc = FftAna->FftAnaPrm.AnaMinFreq;
	ResSet = &(FftAna->FftAnaRslt.VibmRf[0]);
	AntSet = &(FftAna->FftAnaRslt.VibmAf);

	/* Vibration frequency analysis processing */
	rc = fftAnalyzeCalc( FftAna, TraceData, fpeak );

	if( ((USHORT)FftAna->var.freq_max[0] <= Fmin_calc) || ((USHORT)FftAna->var.freq_max[0] > Fmax_calc) )
	{
		ResSet->Freq = 0;
		ResSet->Amp  = 0;
	}
	else
	{
		ResSet->Freq = (USHORT)FftAna->var.freq_max[0];
		ResSet->Amp  = (USHORT)FftAna->var.swork_max[0];
	}

	if( ((USHORT)FftAna->var.freq_max[1] <= Fmin_calc) || ((USHORT)FftAna->var.freq_max[1] > Fmax_calc) )
	{
		(ResSet+1)->Freq = 0;
		(ResSet+1)->Amp  = 0;
	}
	else
	{
		(ResSet+1)->Freq = (USHORT)FftAna->var.freq_max[1];
		(ResSet+1)->Amp  = (USHORT)FftAna->var.swork_max[1];
	}

	if( ((USHORT)FftAna->var.freq_max[2] <= Fmin_calc) || ((USHORT)FftAna->var.freq_max[2] > Fmax_calc) )
	{
		(ResSet+2)->Freq = 0;
		(ResSet+2)->Amp  = 0;
	}
	else
	{
		(ResSet+2)->Freq = (USHORT)FftAna->var.freq_max[2];
		(ResSet+2)->Amp  = (USHORT)FftAna->var.swork_max[2];
	}

	if( ((USHORT)FftAna->var.swork_min <= Fmin_calc) || ((USHORT)FftAna->var.swork_min > Fmax_calc) )
	{
		AntSet->Freq = 0;
		AntSet->Amp  = 0;
	}
	else
	{
		AntSet->Freq = (USHORT)FftAna->var.freq_min;
		AntSet->Amp  = (USHORT)FftAna->var.swork_min;
	}

	return rc;
}



/****************************************************************************************************/
/*																									*/
/*		Data settings for vibration frequency analysisAPI																*/
/*																									*/
/****************************************************************************************************/
void	FftSetAnaCondition( FFTANALYZE *FftAna, LONG SumpleNum, LONG AnaMaxFreq, 
										LONG AnaMinFreq, BOOL TuneLessSts )
{
	/* Setting each execution condition */
	FftAna->FftAnaPrm.SumpleNum = SumpleNum;
	FftAna->FftAnaPrm.AnaMaxFreq = AnaMaxFreq;
	FftAna->FftAnaPrm.AnaMinFreq = AnaMinFreq;
	FftAna->FftAnaPrm.TuneLessSts = TuneLessSts;
}



/****************************************************************************************************/
/*																									*/
/*		Online vibration monitor resonance frequency1AcquisitionAPI														*/
/*																									*/
/****************************************************************************************************/
USHORT	FftGetAnaRsltVibmRf( FFTANALYZE *FftAna )
{
	return	FftAna->FftAnaRslt.VibmRf[0].Freq;
}



/****************************************************************************************************/
/*																									*/
/*		Forced frequency analysis processing request settingAPI																*/
/*																									*/
/****************************************************************************************************/
void	FftSetForceAnaReq( FFTANALYZE *FftAna, BOOL Setting )
{
	FftAna->FftAnaPrm.ActiveFFtOn = Setting;
	return;
}



/****************************************************************************************************/
/*																									*/
/*		Forced frequency analysis processing request acquisitionAPI																*/
/*																									*/
/****************************************************************************************************/
BOOL	FftGetForceAnaReq( FFTANALYZE *FftAna )
{
	return	FftAna->FftAnaPrm.ActiveFFtOn;
}



/***************************************** end of file **********************************************/

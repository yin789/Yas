/****************************************************************************************************/
/*																									*/
/*																									*/
/*		Field-weakening control/voltage compensation control parameter calculation module											*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*	Note	:	First edition	2009.06.04	Y.Oka															*/
/*																									*/
/*	Log		:	Ver1.00																				*/
/*																									*/
/****************************************************************************************************/
#include "Basedef.h"
#include "JL086ServoIpHw.h"
#include "MLib.h"
#include "KLib.h"
#include "MicroPrmCalc.h"




void PcmKvvIniPrmCalc( MPKVVCOMP *MpKvvComp, BPRMDAT *Bprm,
                       USHORT	kvvfil, USHORT pnvoltgn, USHORT vrevgn, BOOL enable )
{
	MpKvvComp->conf.MpKvvFlg = enable;

/*--------------------------------------------------------------------------------------------------*/
/*		Gain calculation for micro voltage compensation gain calculation (2*ScanBCycle)										*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					  Ts								Ts : Axis. Cycle Time						*/
/*		KfMpKvv = -----------							Tx : Time Constant for LPF					*/
/*					Tx + Ts																			*/
/*																									*/
/*					VdcBase * VdetMax * 2^13			VdcBase  : Base DcVolt (Bprm.Vdc)			*/
/*		KdMpKvv = ---------------------------			VdcLevel : Detect DcVolt Level (PnE70)		*/
/*						   VdcLevel						VdetMax	 : at Vdet Max						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
	MpKvvComp->conf.KfMpKvv =
			MlibPcalKf1gain( (kvvfil * 10), (2 * Bprm->SvCycleUs), 0 );
#if (FLOAT_USE==TRUE)
	MpKvvComp->conf.KdMpKvv = Bprm->Vdc * (float)(Bprm->VdetMax * 8192) / (float)pnvoltgn;
#else
	MpKvvComp->conf.KdMpKvv =
			MlibScalKskxkx( Bprm->Vdc, (Bprm->VdetMax * 8192), pnvoltgn, NULL, -30 );
#endif /* FLOAT_USE */

/*--------------------------------------------------------------------------------------------------*/
/*		Main circuit voltage correction lower limit value calculation																	*/
/*--------------------------------------------------------------------------------------------------*/
	MpKvvComp->conf.CompLevel = Bprm->CompVolt * Bprm->VdetMax / pnvoltgn;
	MpKvvComp->conf.vrevgn = vrevgn;

}



/****************************************************************************************************/
/*																									*/
/*		Calculation of voltage compensation gain for micro ( Execution cycle : 250us )											*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*				 VdcBase * Vdetamxx * 2^13     KdMpKvv   VdcBase   : Base DcVolt (Bprm.Vdc)			*/
/*		Kvvin = --------------------------- = ---------	 VdcLevel  : Detect DcVolt Level (PnE70)	*/
/*				 VdcLevel * VdcDetect	       DcVolt  	 VdcDetect : Detect DcVolt [PnE70/VdetMax]	*/
/*																									*/
/****************************************************************************************************/
void	PcmMicroKvvCalc( MPKVVCOMP *MpKvvComp, LONG DcVolt, USHORT UvLevel )
{
	LONG	Vdcx;
	LONG	Kvvin;

	if( MpKvvComp->conf.MpKvvFlg )
	{
/*--------------------------------------------------------------------------------------------------*/
/*		Input processing : lower limit=Reference value(0x2000)of1/2x, Upper limit=Reference value(0x2000)of2x								*/
/*--------------------------------------------------------------------------------------------------*/
		Vdcx = (DcVolt < MpKvvComp->conf.CompLevel)? UvLevel : DcVolt;	/* <S068> */
//		Vdcx = (DcVolt < MpKvvComp->conf.CompLevel)? MpKvvComp->conf.CompLevel : DcVolt;
		Kvvin = MlibLimitul( (MpKvvComp->conf.KdMpKvv / Vdcx), 0x4000, 0x1000 );

/*--------------------------------------------------------------------------------------------------*/
/*		Filter processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		MpKvvComp->var.MpKvv = (SHORT)MlibLpfilter1( Kvvin, MpKvvComp->conf.KfMpKvv, MpKvvComp->var.MpKvv );
	}
	else
	{
		MpKvvComp->var.MpKvv = MpKvvComp->conf.vrevgn;
	}
	return;
}




#if( CSW_SVCF_WEAKENFIELD_USE == TRUE )
/****************************************************************************************************/
/*																									*/
/*		Field-weakening control parameter calculation ( Execution cycle : 250us )		<S024>								*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/****************************************************************************************************/
void	PcmWFPrmCalc( WEAKENFIELD *wf_ptr, LONG MotSpd, LONG DcVolt )
{
	LONG	work1;
	LONG	work2;
	LONG	SpdInv;
	LONG	SpdInv_2;
	LONG	xx[2];

/*--------------------------------------------------------------------------------------------------*/
/*		Main circuit voltage average value calculation[8192/Vdc]															*/
/*--------------------------------------------------------------------------------------------------*/
	wf_ptr->var.VdcSum += DcVolt;
	if ( ++wf_ptr->var.VdcSumCnt >= wf_ptr->var.VdcAvgCnt )
	{
		work1 = wf_ptr->var.VdcSum / wf_ptr->var.VdcSumCnt;
#if (FLOAT_USE==TRUE)
		work1 = (LONG)((float)work1 * wf_ptr->conf.KVdcAsic);
#else
		work1 = MlibMulgain( work1, wf_ptr->conf.KVdcAsic );
#endif /* FLOAT_USE */
		wf_ptr->var.VdcAsic = (SHORT)work1;
		wf_ptr->var.VdcSum = 0;
		wf_ptr->var.VdcSumCnt = 0;
	}

/*--------------------------------------------------------------------------------------------------*/
/*		1/ω,1/ω^2																			*/
/*--------------------------------------------------------------------------------------------------*/
	/* Normalizing speed2^24→15000. */
	work1 = MlibLABS( MotSpd );
//	work1 = MlibMulhigh32( work1, 38400 ); /* <S068> */
	work1 = MlibMulhigh32( work1, 3840000 );

	/* Determining speed */
	if ( work1 < 1000 )
	{
		/* 1000In the following cases,1000Calculate with. */
		work1 = 1000;
	}

	SpdInv = 16777216 / work1;			/* SpdInv = 1/ω */
	SpdInv_2 = SpdInv * SpdInv;			/* SpdInv_2 = 1/ω^2 */

/*--------------------------------------------------------------------------------------------------*/
/*		Voltage feedback loop gain calculation															*/
/*--------------------------------------------------------------------------------------------------*/
	/* Gain is set to prevent loss of precision24bitShifted to the left.ASICin24bitShift to the right. */
	wf_ptr->var.Kpv = MlibMulgain32( SpdInv, wf_ptr->conf.KpvBeforeDiv );
	wf_ptr->var.KpvL = (SHORT)(wf_ptr->var.Kpv & 0x0000ffff);
	wf_ptr->var.KpvH = (SHORT)((wf_ptr->var.Kpv>>16) & 0x0000ffff);

/*--------------------------------------------------------------------------------------------------*/
/*		Voltage feedback loop integral gain calculation														*/
/*--------------------------------------------------------------------------------------------------*/
	/* Gain is set to prevent loss of precision2^20.ASICin20bitShift to the right. */
	wf_ptr->var.Kiv = MlibMulgain32( SpdInv, wf_ptr->conf.KivBeforeDiv );
	wf_ptr->var.KivL = (SHORT)(wf_ptr->var.Kiv & 0x0000ffff);
	wf_ptr->var.KivH = (SHORT)((wf_ptr->var.Kiv>>16) & 0x0000ffff);

/*--------------------------------------------------------------------------------------------------*/
/*		dAxis current command limit calculation																		*/
/*--------------------------------------------------------------------------------------------------*/
	/* dAxis current command limit1Item calculation */
	work1 = wf_ptr->var.VdcAsic * wf_ptr->var.VdcAsic;
	work2 = MlibMulgainSx48( SpdInv_2, wf_ptr->conf.IdrefLimTerm1 );
	MlibMul3232( work1, work2, &xx );
	if( (xx[1] & 0xfc000000) != 0 )
	{
		/* Overflow countermeasure */
		work2 = 0x7fffffff;
	}
	else
	{
		work2 = ((xx[0]>>26) & 0x0000003f) | ((xx[1]<<6) & 0xffffffc0);
	}

	/* dAxis current command limit4Item calculation */
	work1 = MlibMulgainSx48( SpdInv, wf_ptr->conf.IdrefLimTerm4 );

	/* dAxis current command limit value calculation(1Item-2,3Item-4Item) */
	work2 = work2 - wf_ptr->conf.IdrefLimTerm23 - work1;

	if ( work2 < 0 )
	{
		if( work2 < -15000 )
		{
			wf_ptr->var.IdrefLim = -15000;
		}
		else
		{
			wf_ptr->var.IdrefLim = (SHORT)work2;
		}
	}
	else
	{
		wf_ptr->var.IdrefLim = 0;
	}
}
#endif /* #if( CSW_SVCF_WEAKENFIELD_USE == TRUE ) */





/****************************************************************************************************/
/*																									*/
/*		Voltage feedback loop proportional gain calculation constant											*/
/*																									*/
/****************************************************************************************************/
/*			         2π * Kpv * Tiv *10^-6															*/
/*		Kpv(Asic) = ------------------------														*/
/*					          ωL																	*/
/*			  2π * 10^-6 * Vdc		15000	  15000				     1								*/
/*		   = ------------------- * ------- * ------- * Kpv * Tiv * ----- * 2^24						*/
/*			  pole * Lx * 8192	    Nmax	  Imax					ωm								*/
/*			ofωm.2^40is a measure against loss of precision.											*/
/*			1/ωmteethScanBDo it with																	*/
/*																									*/
/*		1/ωmisScanBin2^24/ωm.												*/
/*																									*/
/*		Kpv     [Hz] : Voltage feedback loop gain												*/
/*		Tiv	    [us] : Voltage feedback loop integral time constant											*/
/*		Lx       [H] : Motor inductance															*/
/*		pole     [-] : Number of poles																		*/
/*		Namx [rad/s] : Overspeed																*/
/*		ωm [15000/Nmax] : Motor Speed																*/
/*																									*/
/****************************************************************************************************/
void	PcalVFBKp( WEAKENFIELD *WeakenField, ASICS *SvAsicRegs, BPRMDAT *Bprm, USHORT kv, USHORT tv )
{
#if (FLOAT_USE==TRUE)

	float	fw;

	fw = Bprm->Vdc * 62832.0F / (float)(81920000 * 2);
	fw = fw * (float)kv / Bprm->Keangle;
	fw = fw * (float)tv / Bprm->MotLd;
	fw = fw / 1000000.0F;
	fw = fw * 15000.0F / Bprm->OvrSpd;
	fw = fw * 15000.0F / Bprm->MaxCur;
	WeakenField->conf.KpvBeforeDiv = fw;

#else
	LONG		kx, sx;

	kx = MlibScalKskxkx( Bprm->Vdc, 62832, 81920000*2, &sx, 0 );
	kx = MlibPcalKxkxks( kx, kv, Bprm->Keangle, &sx, 0 );
	kx = MlibPcalKxkxks( kx, tv, Bprm->MotLd, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, 1000000, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, Bprm->OvrSpd, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, Bprm->MaxCur, &sx, 24 );

	WeakenField->conf.KpvBeforeDiv = kx;
#endif /* FLOAT_USE */

}




/****************************************************************************************************/
/*																									*/
/*		Voltage feedback loop integral gain calculation													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*			   		        Ts	    2π * Gain * Tv*10^-6	   Ts									*/
/*		Kiv(Asic) = Kpv * ------ = ----------------------- * -----									*/
/*					       Tiv      		 ωL			  Tiv									*/
/*			  2π * Ts*10^-9 * Vdc	   15000	 15000			  1									*/
/*		   = ---------------------- * ------- * ------- * Kpv * ----- * 2^20						*/
/*			    pole * Lx * 8192	    Nmax	 Imax			 ωm								*/
/*			ofωm.2^20is a measure against loss of precision.											*/
/*			1/ωmteethScanBDo it with																	*/
/*																									*/
/*		1/ωmisScanBin2^24/ωm.												*/
/*																									*/
/*		Kpv     [Hz] : Voltage feedback loop gain												*/
/*		Tiv	    [us] : Voltage feedback loop integral time constant											*/
/*		Lx       [H] : Motor inductance															*/
/*		pole     [-] : Number of poles																		*/
/*		Namx [rad/s] : Overspeed																*/
/*		ωm [15000/Nmax] : Motor Speed																*/
/*																									*/
/****************************************************************************************************/
void	PcalVFBKi( WEAKENFIELD *WeakenField, ASICS *SvAsicRegs, BPRMDAT *Bprm, USHORT kv)
{
#if (FLOAT_USE==TRUE)

	float	fw;

	fw = Bprm->Vdc * 62832.0F / (float)(81920000 * 2);
	fw = fw / Bprm->Keangle;
	fw = fw * (float)kv / Bprm->MotLd;
	fw = fw * (float)KPI_MACYCLENS / 1000000000.0F;
	fw = fw * 15000.0F / Bprm->OvrSpd;
	fw = fw * 15000.0F / Bprm->MaxCur;
	fw = fw * 1048576.0F / 16777216.0F;
	Axis->weaken_field_ptr->conf.KivBeforeDiv = fw;

#else

	LONG		kx, sx;

	kx = MlibScalKskxkx( Bprm->Vdc, 62832, 81920000*2, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 1, Bprm->Keangle, &sx, 0 );
	kx = MlibPcalKxkxks( kx, kv, Bprm->MotLd, &sx, 0 );
	kx = MlibPcalKxgain( kx, KPI_MACYCLENS, 1000000000, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, Bprm->OvrSpd, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, Bprm->MaxCur, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1048576, 16777216, &sx, 24 );

	WeakenField->conf.KivBeforeDiv = kx;
#endif
}




/****************************************************************************************************/
/*																									*/
/*		dAxis current command limit value calculation constant														*/
/*		(1Calculating item)																				*/
/*																									*/
/****************************************************************************************************/
/*		All calculations are not effective values,0-pThink about it.													*/
/*																									*/
/*				  Vmax^2 - (Keωm)^2 - (ωL)^2*Imax^2	   R										*/
/*		Id*lim = ------------------------------------- - -----*Imax 								*/
/*						   2 * ωL * Keωm				  ωL										*/
/*			of1Calculate items.																		*/
/*																									*/
/*		Vmax = Vdc * VdcAsic / 8192 * Prm.v1max / 100 * Prm.vmaxid / 100 / 2 [Vo-p]					*/
/*		Prm.v1max	:PnEB3[%]																		*/
/*		Prm.vmaxid	:PnEB4[%]																		*/
/*																									*/
/*				 (Vdc * Prm.v1max * Prm.vmaxid)^2 * (VdcAsic / 8192 )^2	      1						*/
/*		1Item = -------------------------------------------------------- * -------					*/
/*						  4√2 * 10^8 * pole * Lx * Emf			            ωm^2					*/
/*			ofωm^2and(VdcAsic / 8192)^2.											*/
/*																									*/
/*									 (Bprm.Vdc * Prm.v1max * Prm.vmaxid)^2	   15000	 15000^2		*/
/*		WeakenField.IdrefLimTerm1 = --------------------------------------- * ------- * --------- / 2^48*/
/*									 		4√2 * pole * Lx * Emf			   Imax	   	  Nmax^2		*/
/*																									*/
/*		1/ωmisScanBin2^24/ωm.												*/
/*																									*/
/*		Vdc  [V] 		  : DC Voltage																*/
/*		Emf  [Vrms/rad/s] : EMF Constant															*/
/*		Lx   [H]          : Inductance																*/
/*		Rx   [ohm] 		  : Resistance																*/
/*		Imax [Ao-p]       : Max. Current															*/
/*		Nmax [rad/s]      : Max. Speed																*/
/*		pole [-]          : pole No.																*/
/*		ωm  [15000/Nmax] : Motor Speed																*/
/*																									*/
/****************************************************************************************************/
void	PcalIdrefLimTerm1( WEAKENFIELD *WeakenField, ASICS *SvAsicRegs, BPRMDAT *Bprm, LONG v1max, LONG vmaxid)
{
	LONG		kx,sx;
	LONG		Vdc;
	LONG		Keangle;
	LONG		MotLd;
	LONG		MotEmf;
	LONG		MaxCur;
	LONG		OvrSpd;

	Vdc			= Bprm->Vdc;
	Keangle		= Bprm->Keangle;
	MotLd		= Bprm->MotLd;
	MotEmf		= Bprm->MotEmf;
	MaxCur		= Bprm->MaxCur;
	OvrSpd		= Bprm->OvrSpd;

	/* 1Calculating item */
	kx = MlibScalKskskx( Vdc, Vdc, 100000000, &sx, 0 );
	kx = MlibPcalKxgain( kx, 10000, 56569*2, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 1, Keangle, &sx, 0 );
	kx = MlibPcalKxkxks( kx, v1max, MotLd, &sx, 0 );
	kx = MlibPcalKxkxks( kx, v1max, MotEmf, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, MaxCur, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, OvrSpd, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, OvrSpd, &sx, 0 );
	kx = MlibPcalKxgain( kx, vmaxid, 16777216, &sx, 0 );
	kx = MlibPcalKxgain( kx, vmaxid, 16777216, &sx, 48 );

	WeakenField->conf.IdrefLimTerm1 = kx;

	WeakenField->conf.V1Max = (SHORT)(v1max * 8192 / 100);

}


/****************************************************************************************************/
/*																									*/
/*		dAxis current command limit value calculation constant														*/
/*		(2,3,4Calculating item)																			*/
/*																									*/
/****************************************************************************************************/
/*		All calculations are not effective values,0-pThink about it.													*/
/*																									*/
/*				  Vmax^2 - (Keωm)^2 - (ωL)^2*Imax^2	   R										*/
/*		Id*lim = ------------------------------------- - -----*Imax 								*/
/*						   2 * ωL * Keωm				  ωL										*/
/*			of2,3,4Calculate items.																	*/
/*																									*/
/*			 	 √2 * Emf																			*/
/*		2Item = -----------																			*/
/*			  	 pole * Lx 		   																	*/
/*																									*/
/*																									*/
/*			 	 pole * L * Imax^2																	*/
/*		3Item = -------------------																	*/
/*			       √2 * 4 * Emf																	*/
/*			 											15000										*/
/*		WeakenField.IdrefLimTerm23 = (2Item + 3Item) * -------										*/
/*			 											Imax										*/
/*																									*/
/*				 2 * Rx * Imax		1																*/
/*		4Item = --------------- * -----																*/
/*				   pole * Lx 	   ωm																*/
/*			ofωm.																	*/
/*			 								 15000	   15000										*/
/*		WeakenField.IdrefLimTerm4 = 4Item * -------	* ------- / 2^24								*/
/*			 								 Imax		Nmax										*/
/*																									*/
/*		1/ωmisScanBin2^24/ωm.												*/
/*																									*/
/*		Emf  [Vrms/rad/s] : EMF Constant															*/
/*		Lx   [H]          : Inductance																*/
/*		Rx   [ohm] 		  : Resistance																*/
/*		Imax [Ao-p]       : Max. Current															*/
/*		Nmax [rad/s]      : Max. Speed																*/
/*		pole [-]          : pole No.																*/
/*		ωm  [15000/Nmax] : Motor Speed																*/
/*																									*/
/****************************************************************************************************/
void	PcalIdrefLimTerm234(WEAKENFIELD* weaken_ptr, ASICS *SvAsicRegs, BPRMDAT* Bprm, DBYTEX oslv_pol, USHORT pnvoltgn)
{
	LONG		ky,sy;
	LONG		kz,sz;
	LONG		MotEmf;
	LONG		MotLd;
	LONG		MaxCur;
	LONG		MotR;
	LONG		OvrSpd;
#if (FLOAT_USE==TRUE)
	float	fw;
#else
	LONG		kx,sx;
#endif

	MotEmf		= Bprm->MotEmf;
	MotLd		= Bprm->MotLd;
	MaxCur		= Bprm->MaxCur;
	MotR		= Bprm->MotR;
	OvrSpd		= Bprm->OvrSpd;

	/* 2, 3Calculating item */
	kx = MlibScalKskxkx( MotEmf, 14142, 10000, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, (CHAR)(oslv_pol.b.h), &sx, 0 );
	kx = MlibPcalKxkxks( kx, 1, MotLd, &sx, 0 );

	ky = MlibScalKxgain( (CHAR)(oslv_pol.b.h), 10000, 56569, &sy, 0 );
	ky = MlibPcalKxkskx( ky, MotLd, 1, &sy, 0 );
	ky = MlibPcalKxksks( ky, MaxCur, MotEmf, &sy, 0 );
	ky = MlibPcalKxkskx( ky, MaxCur, 1, &sy, 0 );

	kz = MlibPcalKxaddx( kx, sx, ky, sy, &sz );
	kz = MlibPcalKxkxks( kz, 15000, MaxCur, &sz, -30 );
	weaken_ptr->conf.IdrefLimTerm23 = kz;

	/* 4Calculating item */
	kx = MlibScalKxkskx( 2, MotR, (CHAR)oslv_pol.b.h, &sx, 0 );
	kx = MlibPcalKxksks( kx, MaxCur, MotLd, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, MaxCur, &sx, 0 );
	kx = MlibPcalKxkxks( kx, 15000, OvrSpd, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, 16777216, &sx, 48 );
	weaken_ptr->conf.IdrefLimTerm4 = kx;

/*--------------------------------------------------------------------------------------------------*/
/*		Calculation of gain for voltage calculation for field weakening control (2*ScanBCycle)											*/
/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*					 VdcLevel * 2^13					VdcBase  : Base DcVolt (Bprm.Vdc)			*/
/*		KVdcAsic = -------------------					VdcLevel : Detect DcVolt Level (PnE70)		*/
/*					VdcBase * VdetMax					VdetMax	 : at Vdet Max						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#if (FLOAT_USE==TRUE)
	fw = (float)pnvoltgn * 8192.0F / Bprm->Vdc;
	fw = fw / (float)Bprm->VdetMax;
	weaken_ptr->conf.KVdcAsic = fw;
#else
	kx = MlibScalKxkxks( pnvoltgn, 8192, Bprm->Vdc, &sx, 0 );
	kx = MlibPcalKxgain( kx, 1, Bprm->VdetMax, &sx, 24 );
	weaken_ptr->conf.KVdcAsic = kx;
#endif /* FLOAT_USE */

}




/***************************************** end of file **********************************************/

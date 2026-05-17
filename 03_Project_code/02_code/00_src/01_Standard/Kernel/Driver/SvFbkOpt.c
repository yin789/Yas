/****************************************************************************************************/
/*																									*/
/*																									*/
/*		SvFbkOpt.c : Feedback option card detection processing											*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*	Machine Efficiency : Perform feedback option card detection process.										*/
/*																									*/
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
/*		Rev.1.00 : 2007.02.01  (Den-S-Kai)															*/
/*					Comment rearranging for Sigma5 BaseSoft											*/
/*		Rev.1.01 : 2007.02.03	R.Uda																*/
/*					Complete revision of option variables by reviewing option structure							*/
/*		Rev.2.00 : 2013.09.30	<S04D>																*/
/*					SigmaV->Sigma7Porting																*/
/*																									*/
/****************************************************************************************************/
#include	"SvOptCardIf.h"
#include	"HwDeviceIf.h"
#include 	"SerialEncIf.h"

/****************************************************************************************************/
/*																									*/
/*		Initialize Servo-Feedback Option I/F Variables												*/
/*																									*/
/****************************************************************************************************/
void	FbOptInitSvFeedbackOptVar( OPFBK_HNDL *FbkOpt, PRMDATA *Prm )
{

/*--------------------------------------------------------------------------------------------------*/
/*		Reset FbkOpt Parameters/Variables															*/
/*--------------------------------------------------------------------------------------------------*/
		MlibResetLongMemory( FbkOpt, sizeof(*FbkOpt)/4 );

/*--------------------------------------------------------------------------------------------------*/
/*		Initialize ID/Version/Number																*/
/*--------------------------------------------------------------------------------------------------*/
		FbkOpt->conf.OpInfo.IdType = FBKOP_ID_NONE;				/* Option ID			 		*/
		FbkOpt->conf.OpInfo.YNumber = 0xFFFF;					/* Option Y specification number				*/
		FbkOpt->conf.OpInfo.SoftVer = 0xFFFF;					/* Option software Ver			*/
		FbkOpt->conf.OpInfo.SoftYspVer = 0xFFFF;				/* Option software Y specification Ver		*/
		FbkOpt->conf.OpInfo.SoftTstVer = 0xFFFF;				/* Option software test Ver		*/
		FbkOpt->conf.OpInfo.PrmVer = 0xFFFF;					/* Option parameter Ver		*/

/*--------------------------------------------------------------------------------------------------*/
/*		Get The Required parameters																	*/
/*--------------------------------------------------------------------------------------------------*/
		FbkOpt->conf.PgBaudRateType = (ULONG)((Prm->encbaudrate >> 4 ) & 0x000F);
		
		return;
}


#if CSW_OPT_CARD_FBOPTION != CSW_FBOPTION_NO_SUPPORT
/****************************************************************************************************/
/*																									*/
/*		Detect Feedback Option Card																	*/
/* 				------------------------------------------------------------------------------------*/
/* 				: In this function,Axis number should be0axis/1Axis.											*/
/* 				------------------------------------------------------------------------------------*/
/* 				: In feed option,Ch1,Multi-axis amplifier1Overlapping with axis.			*/
/* 				: Feed option not supported for multi-axis amplifiers.									*/
/* 				: 	->	0Only axis.SGD7W,Disabled by compile switch							*/
/* 				------------------------------------------------------------------------------------*/
/* 				: HOST CPUis,0Set in axis area.											*/
/* 				: Serial encoder transmission and receptionUDLneed to be stored in,0Axis.						*/
/* 				: Regarding the position correction moduleUDLneed to be stored in,1Axis								*/
/****************************************************************************************************/
void	FbOptDetectFeedbackOption( USHORT *OpDetInfo, OPFBK_HNDL *FbkOpt,
											ALMMAN_HANDLE *AlmManager, ASIC_HW_IF_STR *AsicHwReg )
{

	USHORT	i,cnt = 0;
	LONG	rc = FALSE;
/*--------------------------------------------------------------------------------------------------*/
/*		IDSignal read processing(3Next read majority filter)													*/
/*--------------------------------------------------------------------------------------------------*/
		for(i=0; i<3; i++) 
		{
			/* Option card ID code reading */
			if( HALDRV_GET_GPIO03 )
			{
				cnt++;
			}
			
			/* Next read wait time [250us] */
//			KpiResetWatchdogWaitus( 250 );	//TODO:Sigma7There is no place where the same processing is performed?
		}

		/* 2times/3times or more is considered to be an optional card connection */
		if( cnt >= 2 )
		{
			FbkOpt->conf.OpInfo.f.Connect = TRUE;
		}
		else
		{
			FbkOpt->conf.OpInfo.f.Connect = FALSE;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Option detection processing																			*/
/*--------------------------------------------------------------------------------------------------*/

		/* If option is connected this time */
		if( FbkOpt->conf.OpInfo.f.Connect )
		{
			/* cntclearing the value <V785>	*/
			cnt = 0;

			/* CPURead loading information */
			for(i=0; i<3; i++) 
			{
				/* Option card ID code reading */
				if( HALDRV_GET_GPIO04 )
				{
					cnt++;
				}
				
				/* Next read wait time [250us] */
//				KpiResetWatchdogWaitus( 250 );	//TODO:Sigma7There is no place where the same processing is performed?
			}

			/* 2times/3times or moreCPURegarded as installed card */
			if( cnt >= 2 )
			{
				FbkOpt->conf.OpInfo.IfType = FBKOP_SCNV_MODULE;	/* Current situation,CPUOnly installed modules are serial conversion type 	*/
			}
			else
			{
				FbkOpt->conf.OpInfo.IfType = FBKOP_FULLCLS_ONLY;
			}

/*--------------------------------------------------------------------------------------------------*/
/*			Temporary treatment:Pulse Encoder Cardis supported										*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*			Temporary treatment:Pulse Encoder Cardis supported										*/
/*--------------------------------------------------------------------------------------------------*/

			switch( FbkOpt->conf.OpInfo.IfType ){
		/*------------------------------------------------------------------------------------------*/
		/*		Full-Closed I/F Card																*/
		/*------------------------------------------------------------------------------------------*/
				case FBKOP_FULLCLS_ONLY:
					*OpDetInfo |= 0x0100;		/* Option detection information(E51A)Normal detection(1)set*/
					FbkOpt->conf.OpInfo.IdType = FBKOP_ID_FULLCLOSE;
					FbkOpt->conf.OpInfo.YNumber = 0x0000;		/* Option Y specification number				*/
					FbkOpt->conf.OpInfo.SoftVer = 0x0000;		/* Option software Ver			*/
					FbkOpt->conf.OpInfo.SoftYspVer = 0x0000;	/* Option software Y specification Ver		*/
					FbkOpt->conf.OpInfo.SoftTstVer = 0x0000;	/* Option software test Ver		*/
					break;
		/*------------------------------------------------------------------------------------------*/
		/*		Pulse Encoder I/F Card																*/
		/*------------------------------------------------------------------------------------------*/
				case FBKOP_SCNV_MODULE:
					/* YASKAWAHandshake processing with protocol serial conversion module */
					rc = FbOptSCMHandShake( FbkOpt, AsicHwReg );
					/* Handshake successful */
					if( rc == TRUE )
					{
						*OpDetInfo |= 0x0100;	/* Option detection information(E51A)Normal detection(1)set*/
					}
					/* Handshake failure */
					else
					{
						*OpDetInfo |= 0x0700;	/* Option detection information(E51A)Communication error(7)set*/
					}
					break;
		/*------------------------------------------------------------------------------------------*/
		/*		Unsupported Card																	*/
		/*------------------------------------------------------------------------------------------*/
				case FBKOP_NOT_SUPPORTED:
				default:
					ALMSetServoAlarm( AlmManager, ALM_FBKOP_NSUP );	/* A.E75 : Feedback option card not supported*/
					*OpDetInfo |= 0x0700;		/* Option detection information(E51A)Not supported by(7)set*/
					break;
			}

		}
		/* If option is not connected this time */
		else
		{
			/* Alarm if the previous option was connected */
			if( FbkOpt->conf.OpInfo.LastIdType != FBKOP_ID_NONE )
			{
				ALMSetServoAlarm( AlmManager, ALM_FBKOP_DET );	/* A.E72 : Feedback option card detection failure	*/
				*OpDetInfo |= 0x0200;		/* Option detection information(E51A)Detection failed(2)set	*/
			}
			FbkOpt->conf.OpInfo.IfType = FBKOP_NOT_CONNECTED;
		}

/*--------------------------------------------------------------------------------------------------*/
/*			EEPROMWrite request																		*/
/*--------------------------------------------------------------------------------------------------*/
		if( (FbkOpt->conf.OpInfo.IdType != FBKOP_ID_NONE) && (FbkOpt->conf.OpInfo.IdType != FbkOpt->conf.OpInfo.LastIdType) )
		{
			FbkOpt->conf.OpInfo.f.WrEepReq = TRUE;
		}

		return;
}

/****************************************************************************************************/
/*																									*/
/*		Serial conversion type feedback option module handshake processing	<V785>Changed in			*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency :	Feedback sensor signalYASKAWA ProtocolReading the value of the feedback op	*/
/*			Performs handshake processing with the application module.											*/
/*																									*/
/****************************************************************************************************/
LONG FbOptSCMHandShake( OPFBK_HNDL *FbkOpt, ASIC_HW_IF_STR *AsicHwReg )
{
	SENCX	Enc;
	LONG	rc;
	UCHAR	PrmBuf[0x10];
	IDINFO	OptInfo;
	USHORT	work;
	USHORT	i,j;

/*--------------------------------------------------------------------------------------------------*/
/*		Local variable initialization																			*/
/*--------------------------------------------------------------------------------------------------*/
		rc = FALSE;
		MlibResetByteMemory( &OptInfo, sizeof(OptInfo) );

/*--------------------------------------------------------------------------------------------------*/
/*		Start processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		/* Set Initialize Channel	*/
		SHalDrv_InitChangeToP0Mode(AsicHwReg, 1 );			/* P0Mode setting				<S134>	*/
		Enc.TxBuf = (VULONG*)&(AsicHwReg->AREG_SE1_TXD01);	/* Transmission buffer address setting				*/
		Enc.RxBuf = (VULONG*)&(AsicHwReg->AREG_SE1_RXD01);	/* Reception buffer address setting				*/
		Enc.RxFlg = &(AsicHwReg->AREG_SEPGSET11);			/* Reception completion flag address setting			*/
		Enc.TxCmd = KPX_CDAT_ENC0_ESYNC;					/* Transmission start command data(ESYNC1)		*/
//		KpiResetWatchdogWaitus( 500 );						/* ResetWatchdog and Wait				*/

/*--------------------------------------------------------------------------------------------------*/
/*		Encoder communication speed setting(Pn250.1value set in)												*/
/*--------------------------------------------------------------------------------------------------*/
		if( (FbkOpt->conf.PgBaudRateType == SENCBR_AUTO_SEL)
						|| (FbkOpt->conf.PgBaudRateType == SENCBR_4MBPS_SEL) )
		{/* Initialize Current PG Baud Rate (4MBps) */
			FbkOpt->conf.PgBaudRate = SENC_COMRATE_4MBPS;
		}
		else
		{/* Initialize Current PG Baud Rate (8MBps) */
			FbkOpt->conf.PgBaudRate = SENC_COMRATE_8MBPS;
		}

/*--------------------------------------------------------------------------------------------------*/
/*		Connection check																				*/
/*--------------------------------------------------------------------------------------------------*/
		SHalDrv_SetSencComRate( AsicHwReg, FbkOpt->conf.PgBaudRate, 1 );
//		KpiResetWatchdogWaitus( 500 );						/* ResetWatchdog and Wait				*///TODO:Sigma7There is no place where the same processing is performed?

		rc = SencChkConnect( &Enc );							/* Check Connection						*/

/*--------------------------------------------------------------------------------------------------*/
		FbkOpt->var.f.ChekedConnect = TRUE;
		FbkOpt->var.f.Connect = (rc == TRUE)? TRUE: FALSE;

/*--------------------------------------------------------------------------------------------------*/
/*		Connection processing																				*/
/*--------------------------------------------------------------------------------------------------*/
		if( rc == TRUE )									/* Check Return Code					*/
		{
	/*----------------------------------------------------------------------------------------------*/
	/*		ID reading processing																			*/
	/*----------------------------------------------------------------------------------------------*/
			work = 0;
			for( work = 0; work < 0x0010; work += 8 )
			{
				if( SencReadParam8byte( &Enc, work, &PrmBuf[work] ) != TRUE )
				{
					rc = FALSE;
					break;
				}
			}
		}
	/*----------------------------------------------------------------------------------------------*/
	/*		ID processing																				*/
	/*----------------------------------------------------------------------------------------------*/
		if( rc == TRUE )
		{
			SencSetIDINFO( 0x00, PrmBuf, &OptInfo );
			SencSetIDINFO( 0x08, PrmBuf, &OptInfo );

			PrmBuf[2] = OptInfo.Model[1];
			PrmBuf[1] = OptInfo.Model[2];
			PrmBuf[0] = OptInfo.Model[3];
			work = 0;
			for( i = 0; i < 3; i++ )
			{
				for( j = 0; j < 0x10; j++ )			/* MlibAsciiTbl[]According to the size				*/
				{
					if( PrmBuf[i] == MlibAsciiTbl[j] ) break;
				}
				if( j < 0x10 )						/* MlibAsciiTbl[]According to the size				*/
				{
					work |= j<<(i*4);
				}
				else
				{
					work |= 0<<(i*4);
				}
			}
			FbkOpt->conf.OpInfo.IdType = work;
		}
/*--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------*/
/*		Termination processing																					*/
/*--------------------------------------------------------------------------------------------------*/
		SHalDrv_ChangeToP1Mode(AsicHwReg, 1 );				/* P1Mode setting						*/
//		KpiResetWatchdog( );	//TODO

	return( rc );
}
#endif     /* <S09F> */

//#if CSW_OPT_CARD_FBOPTION != CSW_FBOPTION_NO_SUPPORT    /* <S09F> *//* <S19A> */
/****************************************************************************************************/
/*																									*/
/*		Serial conversion type feedback option module information setting									*/
/*		(from Motor/Encoder parameter information)													*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency :	Serial conversion type feedback option module is used in semi-control or fulcrum control	*/
/*			Process called when.																*/
/*																									*/
/*		①Motor/Set feedback option module information from encoder parameter information		*/
/*																									*/
/*			Model type	:FbkOpt->conf.EncInfo.Model ⇒ FbkOpt->conf.OpInfo.ModelStr					*/
/*			SerialNO	:FbkOpt->conf.EncInfo.SNo 	⇒ FbkOpt->conf.OpInfo.SerNumStr					*/
/*			Manufacturing date:FbkOpt->conf.EncInfo.Date 	⇒ FbkOpt->conf.OpInfo.Date						*/
/*																									*/
/*		  ※Model type	:FbkOpt->conf.EncInfo.Modeland the register specifications are different,*/
/*					 Model data after sortingFbkOpt->conf.OpInfo.ModelStrneed to be stored in			*/
/*		  ※Manufacturing date:FbkOpt->conf.EncInfo.Date(Character)andFbkOpt->conf.OpInfo.Date(Binary), the type is	*/
/*					 Because it is different,Binary converted dataFbkOpt->conf.OpInfo.Date.	*/
/*																									*/
/*		②Feedback option module information to encoder/Copy to motor information area.			*/
/*		  (Because higher level may refer to it,FBOptPerform copying process)							*/
/*																									*/
/*		[Encoder related]																			*/
/*			Model type	:FbkOpt->conf.OpInfo.ModelStr	⇒ MencV->EncInfo.Model							*/
/*												⇒ FbkOpt->conf.EncInfo.Model						*/
/*			SerialNO	:FbkOpt->conf.OpInfo.SerNumStr 	⇒ MencV->EncInfo.SNo						*/
/*			Manufacturing date:FbkOpt->conf.OpInfo.Date 		⇒ MencV->EncInfo.Date							*/
/*																									*/
/*		  ※FbkOpt->conf.EncInfo.Modelcreates a process [/21] [22] that is called when,Register(C847H-C84EH,C896H-C8A1H,C8C3H-C8CAH)of	*/
/*			To maintain consistency.(I have to copy,C847H-C84EH,C8C3H-C8CAHdoes not meet the register specifications)		*/
/*																									*/
/*		[Motor related]																				*/
/*			Model type	:FbkOpt->conf.MotInfo.Model	⇒ MencV->MotInfo.Model							*/
/*			SerialNO	:FbkOpt->conf.MotInfo.SNo 	⇒ MencV->MotInfo.SNo								*/
/*			Manufacturing date:FbkOpt->conf.MotInfo.Date 	⇒ MencV->MotInfo.Date							*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
/*	<V844> In case of Fulk,Fulfilled in the semi-encoder information area(FBOptions)Incorrect copy of information		*/
/*		   Fixed a bug that was occurring(FBOption defectNo20),Process review							*/
/****************************************************************************************************/
void FbOptSetIDInfo( OPFBK_HNDL *FbkOpt, MENCV *MencV, BOOL SencFbOptUse )
{
	LONG	work, i;
	USHORT	Yflg;
	UCHAR	Chbuf[3];
#if CSW_OPT_CARD_FBOPTION != CSW_FBOPTION_NO_SUPPORT    /* <S19A> */
	/*----------------------------------------------------------------------------------------------*/
	/*	For types with feedback option module ID set in the encoder ID area	*/
	/*		※ Currently, only this type exists.														*/
	/*----------------------------------------------------------------------------------------------*/
		if( FbkOpt->conf.OpInfo.IfType == FBKOP_SCNV_MODULE )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Get encoder information,After setting processing,Set encoder ID information as feedback option		*/
		/*		Copy to module ID information,Clear encoder ID information.					*/
		/*------------------------------------------------------------------------------------------*/
		/*		Format copy																			*/
		/*------------------------------------------------------------------------------------------*/
		/*---------- FBOptCard model information setting (Fixed part)-------------------------------------------------*/
			FbkOpt->conf.OpInfo.ModelStr[0] = 'S';
			FbkOpt->conf.OpInfo.ModelStr[1] = 'G';
			FbkOpt->conf.OpInfo.ModelStr[2] = 'D';
			FbkOpt->conf.OpInfo.ModelStr[3] = 'V';
			FbkOpt->conf.OpInfo.ModelStr[4] = '-';
			FbkOpt->conf.OpInfo.ModelStr[5] = 'O';
		/*---------- FBOptCard model information setting (Variable section)-------------------------------------------------*/
			//ID (FBOptPrm:0x00-0x03)
			FbkOpt->conf.OpInfo.ModelStr[6] = FbkOpt->conf.EncInfo.Model[0];
			FbkOpt->conf.OpInfo.ModelStr[7] = FbkOpt->conf.EncInfo.Model[1];
			FbkOpt->conf.OpInfo.ModelStr[8] = FbkOpt->conf.EncInfo.Model[2];
			FbkOpt->conf.OpInfo.ModelStr[9] = FbkOpt->conf.EncInfo.Model[3];
			//Production revision  (FBOptPrm:0x04)
			FbkOpt->conf.OpInfo.ModelStr[10] = FbkOpt->conf.EncInfo.Model[4];
			//Parameter manufacture ID (FBOptPrm:0x05)
				//No information
			//YSpecification judgment
			Chbuf[0] = FbkOpt->conf.EncInfo.Model[5];
			Chbuf[1] = FbkOpt->conf.EncInfo.Model[6];
			Chbuf[2] = FbkOpt->conf.EncInfo.Model[7];
			for( i = 0; i < 3; i++ )
			{ //Each digit of character is0x30<=x<=0x39(0~9)rangeYDetermine as specification.others are treated as standard specifications.
				if(('0' <= Chbuf[i]) && (Chbuf[i] <= '9'))
				{
					Yflg = TRUE;
				}
				else
				{
					Yflg = FALSE;
					break;
				}
			}
			
			if(Yflg == TRUE)
			{
				if(((FbkOpt->conf.EncInfo.Model[5] != '0') || 
						(FbkOpt->conf.EncInfo.Model[6] != '0') || (FbkOpt->conf.EncInfo.Model[7] != '0')))
				{	/* Y specification setting(ALL='0'is standard specification) */
					FbkOpt->conf.OpInfo.ModelStr[11] = 'Y';
					FbkOpt->conf.OpInfo.ModelStr[12] = Chbuf[0];
					FbkOpt->conf.OpInfo.ModelStr[13] = Chbuf[1];
					FbkOpt->conf.OpInfo.ModelStr[14] = Chbuf[2];
				}
				else
				{
					/* All digits'0'is standard */
					Yflg = FALSE;
				}
			}

		/*------------------------------------------------------------------------------------------*/
		/*		Option Y specification number (0:Standard, 1~999(dec):YSpecification										*/
		/*------------------------------------------------------------------------------------------*/
			if(Yflg == FALSE)
			{
				FbkOpt->conf.OpInfo.YNumber = 0;
			}
			else
			{
				work = ( FbkOpt->conf.OpInfo.ModelStr[14] - 0x30 );
				work += ( FbkOpt->conf.OpInfo.ModelStr[13] - 0x30 ) * 10;
				work +=  ( FbkOpt->conf.OpInfo.ModelStr[12] - 0x30 ) * 100;
				FbkOpt->conf.OpInfo.YNumber = (USHORT)work;
			}

		/*------------------------------------------------------------------------------------------*/
		/*		Format copy(This information firstOpInfoBecause it makes,EncInfocopy to						*/
		/*------------------------------------------------------------------------------------------*/
			work = MlibMIN( sizeof(FbkOpt->conf.OpInfo.ModelStr), sizeof(FbkOpt->conf.EncInfo.Model) );
			MlibCopyByteMemory( (UCHAR*)(FbkOpt->conf.OpInfo.ModelStr), (UCHAR*)(FbkOpt->conf.EncInfo.Model), work );
		/*------------------------------------------------------------------------------------------*/
		/*		Copy serial number																	*/
		/*------------------------------------------------------------------------------------------*/
			work = MlibMIN( sizeof(FbkOpt->conf.EncInfo.SNo), sizeof(FbkOpt->conf.OpInfo.SerNumStr) );
			MlibCopyByteMemory( (UCHAR*)(FbkOpt->conf.OpInfo.SerNumStr), (UCHAR*)(FbkOpt->conf.EncInfo.SNo), work );
		/*------------------------------------------------------------------------------------------*/
		/*		Manufacturing date setting copy																	*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.OpInfo.Date[0] = (FbkOpt->conf.EncInfo.Date[0] - '0')*10 + (FbkOpt->conf.EncInfo.Date[1] - '0');
			FbkOpt->conf.OpInfo.Date[1] = (FbkOpt->conf.EncInfo.Date[3] - '0')*10 + (FbkOpt->conf.EncInfo.Date[4] - '0');
		/*------------------------------------------------------------------------------------------*/
		/*		Option ID information termination code insertion													*/
		/*------------------------------------------------------------------------------------------*/
			SencFillNullIDINFO( &FbkOpt->conf.EncInfo );

		/*------------------------------------------------------------------------------------------*/
		/*  Motor and encoder products obtained from optionsIDto existing area					*/
		/*   FbkOpt->conf.MotInfo ⇒ MencV->MotInfo													*/
		/*   FbkOpt->conf.MotInfo ⇒ MencV->MotInfo													*/
		/*------------------------------------------------------------------------------------------*/
			if(SencFbOptUse == TRUE)	
			{
				/* Encoder type	: Option type is encoder type				*/
				work = MlibMIN( sizeof(FbkOpt->conf.OpInfo.ModelStr), sizeof(MencV->EncInfo.Model) );
				MlibCopyByteMemory( (UCHAR*)(MencV->EncInfo.Model), (UCHAR*)(FbkOpt->conf.OpInfo.ModelStr), work );
				/* Serial number		: OptionsS/NEncoderS/Nto be					*/
				work = MlibMIN( sizeof(FbkOpt->conf.EncInfo.SNo), sizeof(MencV->EncInfo.SNo) );
				MlibCopyByteMemory( (UCHAR*)(MencV->EncInfo.SNo), (UCHAR*)(FbkOpt->conf.EncInfo.SNo), work );
				/* Manufacturing date			:	Set option manufacturing date as encoder manufacturing date		*/
				MencV->EncInfo.Date[0] = FbkOpt->conf.EncInfo.Date[0];
				MencV->EncInfo.Date[1] = FbkOpt->conf.EncInfo.Date[1];
				MencV->EncInfo.Date[2] = FbkOpt->conf.EncInfo.Date[2];
				MencV->EncInfo.Date[3] = FbkOpt->conf.EncInfo.Date[3];
				MencV->EncInfo.Date[4] = FbkOpt->conf.EncInfo.Date[4];

				/* Motor type	: Motor type in option is valid motor type			*/
				work = MlibMIN( sizeof(FbkOpt->conf.MotInfo.Model), sizeof(MencV->MotInfo.Model) );
				MlibCopyByteMemory( (UCHAR*)(MencV->MotInfo.Model), (UCHAR*)(FbkOpt->conf.MotInfo.Model), work );
				/* Serial number	:	Motor in optionS/NEnable motorS/Nto be				*/
				work = MlibMIN( sizeof(FbkOpt->conf.MotInfo.SNo), sizeof(MencV->MotInfo.SNo) );
				MlibCopyByteMemory( (UCHAR*)(MencV->MotInfo.SNo), (UCHAR*)(FbkOpt->conf.MotInfo.SNo), work );
				/* Manufacturing date		:	The manufacturing date of motor in option is the valid motor manufacturing date	*/
				work = MlibMIN( sizeof(FbkOpt->conf.MotInfo.Date), sizeof(MencV->MotInfo.Date) );
				MlibCopyByteMemory( (UCHAR*)(MencV->MotInfo.Date), (UCHAR*)(FbkOpt->conf.MotInfo.Date), work );
			}

		/*------------------------------------------------------------------------------------------*/
		/*		Option software Y specification Ver	(No source data:0Fixed)							*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.OpInfo.SoftYspVer = 0x0000;
		/*------------------------------------------------------------------------------------------*/
		/*		Option software test Ver	(No source data:0Fixed)							*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.OpInfo.SoftTstVer = 0x0000;
		/*------------------------------------------------------------------------------------------*/
		/*		Option parameter Ver		(No source data:0Fixed)							*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.OpInfo.PrmVer = 0x0000;
		}
/*--------------------------------------------------------------------------------------------------*/
#endif     /* <S19A> */
		return;
}
//#endif     /* <S09F> */

#if CSW_OPT_CARD_FBOPTION != CSW_FBOPTION_NO_SUPPORT    /* <S09F> */
/****************************************************************************************************/
/*																									*/
/*		Model information register:Set feedback option module information								*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency :	Set feedback option module information other than ID information in model information register		*/
/*																									*/
/****************************************************************************************************/
void FbOptInitRegister(OPFBK_HNDL *FbkOpt, MENCV *FencV, PRMDATA* Prm, BPRMDAT* Bprm)
{
	LONG	x;
	UCHAR	y;
		if( Bprm->FencUse == TRUE )
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Encoder information in feedback option module									*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.EncType = FencV->EncType;						/* Encoder type/Resolution	*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.XfbPulse2 = 1 << Prm->MencP.BitnoDpoint.b.l;		/* Encoder resolution			*/
		/*------------------------------------------------------------------------------------------*/
			y = (FencV->AbsoEncoder == TRUE)? 0x10 : 0x00;
			FbkOpt->conf.AbsEncType = ((y >> 4) & 0x0F) - 1;			/* Absolute value encoder type	*/
		/*------------------------------------------------------------------------------------------*/
															/* Encoder firmware version	*/
			FbkOpt->conf.EncVer = (FbkOpt->conf.OpInfo.IfType == FBKOP_SCNV_MODULE)? 0 : FencV->SoftVer;
		/*------------------------------------------------------------------------------------------*/
		}
		else if( Bprm->SencFbOptUse == TRUE )
		{
		/*------------------------------------------------------------------------------------------*/
		/*	Motor information in feedback option module											*/
		/*------------------------------------------------------------------------------------------*/
		/*	Set only when used in semi-closed control											*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.MotID = Prm->MencP.typm_e.b.l;									/* Motor ID	*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.MotWat.dw = MlibScalKskxkx( Bprm->MotW, 1, 1, NULL, -30 );	/* Motor capacity	*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.MotVolt = Bprm->MotVcode;							/* Motor voltage code		*/
		/*------------------------------------------------------------------------------------------*/

		/*------------------------------------------------------------------------------------------*/
		/*		Encoder information in feedback option module									*/
		/*------------------------------------------------------------------------------------------*/
			x = (Prm->MencP.typm_e.b.h & 0xF0)? 0x0100 : 0x0000;
			FbkOpt->conf.EncType = x | Prm->MencP.bit_dp.b.l;					/* Encoder type/Resolution	*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.XfbPulse2 = Bprm->XfbPulse2;					/* Encoder resolution			*/
		/*------------------------------------------------------------------------------------------*/
			FbkOpt->conf.AbsEncType = ((Prm->MencP.typm_e.b.h >> 4) & 0x0F) - 1;/* Absolute value encoder type	*/
		/*------------------------------------------------------------------------------------------*/
															/* Encoder firmware version	*/
			FbkOpt->conf.EncVer = (FbkOpt->conf.OpInfo.IfType == FBKOP_SCNV_MODULE)? 0 : Prm->MencP.encver;
		/*------------------------------------------------------------------------------------------*/
		}
	return;
}
#endif     /* <S09F> */



#if 0/* TODO:Sigma7, the same amplifier(Does not depend on parameter settings)can be connected to the rotary type and linear type */
/****************************************************************************************************/
/*		Product unsupported check process (Feedback option module								*/
/*																									*/
/*		Unsupported conditions … (1) Linear type startup(PnE0B.bit1=1)And feedback option module side	*/
/*							  Settings used in semi-closed control(Pn002.3=0, Pn00B.3=1),Feedback	*/
/*							  If motor constant in option module is not linear type.			*/
/*							  (Fulk check is performed using linear type product unsupport check).			*/
/*						  (2) Rotary type start(PnE0B.Bit1=0)And feedback option module side	*/
/*							  Settings used in semi-closed control(Pn002.3=0, Pn00B.3=1),Feedback	*/
/*							  If motor constant in option module is not rotation type.				*/
/****************************************************************************************************/
void FbOptCheckConnectNonSupport( OPFBK_HNDL *FbkOpt )
{
	LONG	rc;

	rc = FALSE;

	if( Kprm.f.SencFbOptUse == TRUE )
	{
	/*----------------------------------------------------------------------------------------------*/
	/*		Motorless test mode check															*/
	/*----------------------------------------------------------------------------------------------*/
		if( (Kprm.f.MotorLessTestMode == FALSE)						/* Not in motorless test mode	*/
				|| (MotorLessTestDatas.f.SemiEncConnect == TRUE))	/* Encoder connection				*/
		{
		/*------------------------------------------------------------------------------------------*/
		/*		Possibility determination for linear type amplifiers(For rotation parameters)			*/
		/*------------------------------------------------------------------------------------------*/
			if( (Kprm.AmpType == AMPTYPE_LINEAR) && ((MencV->TypeCode & 0x02) == 0x02) )
			{
				rc |= TRUE;										/* Determined as unsupported					*/
			}
		/*------------------------------------------------------------------------------------------*/
		/*		Possibility determination for rotation type amplifiers(For linear parameters)			*/
		/*------------------------------------------------------------------------------------------*/
			if( (Kprm.AmpType == AMPTYPE_ROTARY) && ((MencV->TypeCode & 0x02) == 0x00) )
			{
				rc |= TRUE;										/* Determined as unsupported					*/
			}
		/*------------------------------------------------------------------------------------------*/
		}
	/*----------------------------------------------------------------------------------------------*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*		Product unsupported alarm set																*/
/*--------------------------------------------------------------------------------------------------*/
	if( rc == TRUE )
	{
		ALMSetServoAlarm( AxisRsc->AlmManager, ALM_NOTSUPPORT );/* A.051 : Product unsupported alarm	*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return;
}
#endif


#if CSW_OPT_CARD_FBOPTION != CSW_FBOPTION_NO_SUPPORT    /* <S09F> */
/****************************************************************************************************/
/*																									*/
/*		Feedback option encoder selection reset processing										*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency :	At reset(When recalculating parameters)								*/
/*																									*/
/****************************************************************************************************/
BOOL	FbOptPrstSelectEncoder(OPFBK_HNDL *FbkOpt, PRMDATA* Prm, BPRMDAT* Bprm )
{
	BOOL PrmError;
	
	PrmError = FALSE;/* <S052> */
/*--------------------------------------------------------------------------------------------------*/
/*	Trying to use the serial converter type option that was used in semi-closed mode,Trying to use in full-closed mode		*/
/*	If you do,,CN2Connected motor parametersREADis not done, so,Regard it as abnormal.								*/
/*--------------------------------------------------------------------------------------------------*/
/*	Bprm->SencFbOptUse teeth Power on(CPUReset)This function is only set when,Serial before reset		*/
/*	Used as a condition for semi-closed use of the converter type option.								*/
/*--------------------------------------------------------------------------------------------------*/
	if( (Bprm->SencFbOptUse == TRUE) && ((Prm->b_prm2>>12) & 0x000F) )
	{
		PrmError = TRUE;								/* Semi-fulque parameter setting error(A.044)		*/
	}

/*--------------------------------------------------------------------------------------------------*/
/*	Serial converter type option used in full closed,Trying to use it in semi-closed mode		*/
/*	If you do,,Motor parameters in options areREADis not done, so,Regard it as abnormal.						*/
/*--------------------------------------------------------------------------------------------------*/
/*	FbkOpt->var.f.FullclosedUse teeth Power on(CPUReset)This function is only set when,Before reset		*/
/*	Used as a condition for full-closed use of the serial converter type option.						*/
/*--------------------------------------------------------------------------------------------------*/
	if( (FbkOpt->var.f.FullclosedUse == TRUE) && (((Prm->b_prmB>>12) & 0x000F) == 1) )
	{
		PrmError = TRUE;								/* Semi-fulque parameter setting error(A.044)		*/
	}
/*--------------------------------------------------------------------------------------------------*/
	return PrmError;
}
#endif     /* <S09F> */

#if CSW_OPT_CARD_FBOPTION != CSW_FBOPTION_NO_SUPPORT           /* <S09F> */
/****************************************************************************************************/
/*																									*/
/*		Fulk/Semiku parameter check															*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*	Machine Efficiency : Full-closed encoder/Check the semi-closed encoder settings.					*/
/*																									*/
/*		ChkSw																						*/
/*			TRUE : Option board connection check available													*/
/*			FALSE: Option board connection check not checked													*/
/*																									*/
/*		RETRUN																						*/
/*			TRUE : 	Normal																		*/
/*			FALSE: Parameter setting error																*/
/*																									*/
/****************************************************************************************************/
LONG	FbOptFullSemiPrmChk( MENCV *MencV, OPFBK_HNDL *FbkOpt, PRMDATA* Prm, BPRMDAT* Bprm, LONG ChkSw )
{
	LONG	rc = TRUE ;
	if((Prm->b_prm2>>12) & 0x000F)						/* Using in full-closed mode?					*/
	{
		if(Prm->syssw1 & 0x0001)							/* Full-closed compatible models?					*/
		{
			if(MencV->AxisMotType == MOTTYPE_ROTARY)			/* Rotation type?									*/
			{
				if( (ChkSw == TRUE ) &&					/* Feedback option board connection check available	 &&	*/
					(FbkOpt->conf.OpInfo.f.Connect == FALSE))/* Feedback option board not connected?	*/
				{
					rc = FALSE;
				}
				else if( (ChkSw == TRUE ) && (FbkOpt->conf.OpInfo.IfType == FBKOP_FULLCLS_ONLY) )
				{
					if((Prm->b_prmB>>12) & 0x000F)
					{
						rc = FALSE;
					}
				}
			}
			else										/* Linear type?								*/
			{
				rc = FALSE;
			}
		}
		else											/* Full-non-compatible models?						*/
		{
			rc = FALSE;
		}
	}
	else												/* Use it as semi-closed mode?					*/
	{
		/* Feedback option board connection check included and,												*/
		/* Feedback option board not connected,or,											*/
		/* Feedback option board connected,and Feedback option board type is full-closed					*/
		if((ChkSw == TRUE) &&
		   ( (FbkOpt->conf.OpInfo.f.Connect == FALSE) ||
		  	 ( (FbkOpt->conf.OpInfo.f.Connect == TRUE)&&(FbkOpt->conf.OpInfo.IfType == FBKOP_FULLCLS_ONLY) ) ) )
		{
			if((Prm->b_prmB>>12) & 0x000F)				/* Connect to feedback option?			*/
			{
				rc = FALSE;
			}
		}
	}
	return(rc);
}
#endif     /* <S09F> */

//#endif/* CSW_FBOPTION_SUPPORT *//* <S09F> */
/***************************************** end of file **********************************************/


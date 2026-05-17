/****************************************************************************************************/
/*																									*/
/*																									*/
/*		XlsAmonDef.h : Analog monitor definition header file												*/
/*																									*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*																									*/
/*		1. Analog monitor definition structure																	*/
/*																									*/
/*		2. Analog monitor gain calculation function																*/
/*																									*/
/*		3. Analog monitor definition tableExternDeclaration														*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/*																									*/
/************** Copyright (C) Yaskawa Electric Corporation ******************************************/
/*																									*/
/*		2013.10.03 : Made by Excel Macro															*/
/*																									*/
/*																									*/
/****************************************************************************************************/
#ifndef __XLS_AMON_DEF_H
#define __XLS_AMON_DEF_H
#include "Basedef.h"





/****************************************************************************************************/
/*																									*/
/*		Analog monitor definition structure																	*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Analog monitor definition structure																	*/
/*--------------------------------------------------------------------------------------------------*/
typedef struct  {
		UCHAR	SelNo;							/* SelectionNo											*/
		UCHAR	Vtype;							/* Variable types										*/
		LONG	(*GetVarData)( void* Axis );	/* Address of monitor variable acquisition function						*/
		KSGAIN	(*GetVarGain)( void* Axis );	/* Address of monitor gain calculation function					*/
} AMONDEF;

/*--------------------------------------------------------------------------------------------------*/
/*		Analog monitor output definition																		*/
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{
		UCHAR	Vtype;							/* Monitor variable type									*/
		LONG	(*GetVarData)( void* Axis );	/* Address of monitor variable acquisition function						*/
		LONG	Mgain;							/* Monitor output gain									*/
		LONG	Offset;							/* Monitor output offset								*/
		VUINT/*<S074>*/	Enable;					/* Access permission(TRUE:Permit, FALSE:Prohibited)				*/
		void*	Vaddr;							/* <S074> Monitor Variable Address					*/
//		CHAR	RegVoltMon_Flag_Ch0;			/* <S0E7> Monitor voltage flag Ch0(Register access)	*/	/* <S0FD> */
//		CHAR	RegVoltMon_Flag_Ch1;			/* <S0E7> Monitor voltage flag Ch1(Register access)	*/	/* <S0FD> */
//		LONG	RegVoltMonSetVal_Ch0;			/* <S0E7> Monitor voltage setting value Ch0(Register access)	*/	/* <S0FD> */
//		LONG	RegVoltMonSetVal_Ch1;			/* <S0E7> Monitor voltage setting value Ch1(Register access)	*/	/* <S0FD> */
} AMONOUT;


/*--------------------------------------------------------------------------------------------------*/
/*		AMONDEF Version																				*/
/*--------------------------------------------------------------------------------------------------*/
#define AMONDEF_VERSION			0x0001			/* AMONDEF Version									*/
/*--------------------------------------------------------------------------------------------------*/
/*		Variable Type Definition																	*/
/*--------------------------------------------------------------------------------------------------*/
#define XAMDEF_VxLONG			0x00			/* Type:LONG										*/
#define XAMDEF_VxSHORT			0x01			/* Type:SHORT										*/
#define XAMDEF_VxBIT			0x02			/* Type:BIT Signal									*/
#define XAMDEF_VxPRMVAR			0xF0			/* Type:PrmVariable								*/
/*--------------------------------------------------------------------------------------------------*/





/****************************************************************************************************/
/*																									*/
/*		Analog monitor gain calculation function																*/
/*																									*/
/****************************************************************************************************/
/*																									*/
/*		AmonGain = AmonGcalFunction( )																*/
/*		AmonOutput [V] = AmonGain * AmonVariable													*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Standard Function																			*/
/*--------------------------------------------------------------------------------------------------*/
KSGAIN	AmonGcalDummy( void *Axis );			/* Dummy Function (Return Zero Gain)				*/
KSGAIN	AmonGcalTorqueRef( void *Axis );		/* Torque Ref. [2^24/MaxTrq] --> [1V/100%]			*/
KSGAIN	AmonGcalMotorSpeed( void *Axis );		/* Motor Speed [2^24/OvrSpd] --> [1V/1000min-1]		*/
KSGAIN	AmonGcalPulseError( void *Axis );		/* Pulse Error [pulse] --> [0.05V/pulse]			*/
KSGAIN	AmonGcalBitSignal( void *Axis );		/* Bit Signal  [0,1] --> [0:0V, 1:5V]				*/
/*--------------------------------------------------------------------------------------------------*/
/*		User Defined Function																		*/
/*--------------------------------------------------------------------------------------------------*/
KSGAIN	AmonGcalPosCmdaSpd( void *Axis );		/* Position command speed										*/
KSGAIN	AmonGcalPulseErr01( void *Axis );		/* Position deviation between motor and load							*/
KSGAIN	AmonGcalActGain( void *Axis );			/* Valid gain										*/
KSGAIN	AmonGcalMotPfbkSpd( void *Axis );		/* Command speed before position loop								*/
KSGAIN	AmonGcalTrqRef15K( void *Axis );		/* dAxis current command										*/
/*--------------------------------------------------------------------------------------------------*/



/****************************************************************************************************/
/*																									*/
/*		Analog monitor definition tableExternDeclaration														*/
/*																									*/
/****************************************************************************************************/
extern  const AMONDEF AmonDefTbl[];				/* Analog monitor definition table						*/
/*--------------------------------------------------------------------------------------------------*/
extern  const USHORT AmonDefTblEntNum;			/* Analog monitor definition table registration number				*/
/*--------------------------------------------------------------------------------------------------*/
extern  const AMONDEF AmonDefTblUser[];			/* Analog monitor definition table						*/
/*--------------------------------------------------------------------------------------------------*/
extern  const USHORT AmonDefTblUserEntNum;		/* Analog monitor definition table registration number				*/
/*--------------------------------------------------------------------------------------------------*/
/*		Define Entry Number																			*/
/*--------------------------------------------------------------------------------------------------*/
#if		OPENSW_AMONDEF == 0
#define AMONDEFTBL_ENTNUM	AmonDefTblEntNum
#else
#define AMONDEFTBL_ENTNUM	(AmonDefTblEntNum + AmonDefTblUserEntNum)
#endif
/*--------------------------------------------------------------------------------------------------*/






#endif //__XLS_AMON_DEF_H
/***************************************** end of file **********************************************/

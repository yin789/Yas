/************************************************************************************************************/
/*																											*/
/*																											*/
/*		Interrupt.h : Mercury Interrupt-related processing																*/
/*																											*/
/*																											*/
/************************************************************************************************************/
/*																											*/
/*																											*/
/*		Interrupt setting header file																			*/
/*																											*/
/*																											*/
/************** Copyright (C) Yaskawa Electric Corporation **************************************************/
/*																											*/
/*		Rev.1.00 : 2013.07.14  Y.Tanaka		First edition															*/
/*		Rev.1.01 : 2013.07.30  Y.Tanaka		Code organization														*/
/*																											*/
/************************************************************************************************************/


/************************************************************************************************************/
/*																											*/
/*		PROTOTYPE																							*/
/*																											*/
/************************************************************************************************************/
extern	void	InitInterrupt( void );							/* Interrupt-related initialization processing					*/
extern	void	IntIgnore( void );								/* Infinite loop interrupt						*/
extern	void	Common_IRQHandler0( void );						/* ARMPF_VIC_ISC0		*/
extern	void	Common_IRQHandler1( void );						/* ARMPF_VIC_ISC1		*/
extern	void	ScanAIntHandler( void );						/* ScanAInterrupt pre-processing					*/
extern	void	ScanBIntHandler( void );						/* ScanBInterrupt pre-processing					*/
extern	void	ScanCIntHandler( void );						/* ScanCInterrupt pre-processing					*/
extern	BOOL	GetIntUDLStatus(UCHAR num);						/* INTUDL*Interrupt request acquisition processing				*///<S0C7>
extern	void 	ReqIntUDLStatusClear( UCHAR num );				/* INTUDL*Interrupt request clear processing				*///<S0C7>


/************************************************************************************************************/
/*		test																									*/
/*		External functionexternDeclaration																					*/
/*																											*/
/************************************************************************************************************/
extern void __EI( void );											/* Interrupt enabling setting						*/
extern void __DI( void );											/* Interrupt disable setting						*/


/************************************************************************************************************/
/*																											*/
/*		Macro definition																							*/
/*																											*/
/************************************************************************************************************/
#define	SetEnableInterrupt()	__EI()								/* Interrupt enabling setting(__EI)				*/
#define	SetDisableInterrupt()	__DI()								/* Interrupt disable setting(__DI)				*/


/************************************************************************************************************/
/*																											*/
/*		Structure definition																							*/
/*																											*/
/************************************************************************************************************/
typedef struct{
//	UCHAR	ISL;					/* Interrupt execution unit 0/1 - IRQ/FIQ											*///All causes at once"IRQ"Set
//	UCHAR	IEN;					/* Interrupt request enable 0/1 - Mask/Permit									*///All causes at once"Permit"Set
	UCHAR	PLS;					/* Interrupt detection type setting 0/1 - Level/Edge							*/
	UCHAR	EDGC;					/* Interrupt enable level/Edge specification										*/
									/* When detecting edge:0/1/2/3 - No detection/Rise/Fall/Both edges				*/
									/* When level detection:0/1/2/3 - No detection/High/Low/No detection					*/
	void (*RegHandler)(void);		/* Registration handler					*/
}INTVECINITTBL;

typedef struct{
	ULONG	Priority;				/* Interrupt enable level/Edge specification										*/
	void (*IntHandler)(void);		/* Interrupt handler					*/
}INTVECTBL;


/************************************************************************************************************/
/*																											*/
/*		Variable declaration																							*/
/*																											*/
/************************************************************************************************************/
extern const INTVECINITTBL InitIntVecTbl[64];
extern const INTVECTBL IntVecTbl[64];

enum INTUDL_NUM
{
INTUDL0 = 0,	/* 0: ScanA		*/
INTUDL1,        /* 1: ScanB		*/
INTUDL2,        /* 2: ScanC		*/
INTUDL3,        /* 3: ScanN		*/
INTUDL4,        /* 4: INT1		*/
INTUDL5,        /* 5: INT2		*/
INTUDL6,        /* 6: M3_INT0	*/
INTUDL7,        /* 7: M3_INT1	*/
};

#if (CSW_NETIF_TYPE == NETIF_M2_TYPE)
#define	INTUDL_ML INTUDL4
#else
#define	INTUDL_ML INTUDL7
#endif
/*-------end of file----------------------------------------------------------------------------------------*/

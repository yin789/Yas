/****************************************************************************/
/*																			*/
/*																			*/
/*		PRM Bank Header file												*/
/*																			*/
/*																			*/
/****************************************************************************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/************** Copyright Yaskawa Electric Corporation **********************/
/*																			*/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/
#ifndef		PRM_BANKDEF_H_
#define		PRM_BANKDEF_H_


/* defines */
#define BANK_NUM_MAX	16	/* Maximum number of banks */
#define BANK_MEM_MAX	15	/* Maximum number of members */
#define BANK_DATA_MAX	64	/* Maximum number of bank data */

typedef struct tagMTBANK {
	UCHAR		bankNum;				/* Number of banks */
	UCHAR		memNum;					/* Number of members */
	UCHAR		bankSel;				/* Bank selector current value */
	UCHAR		cmdSel;					/* Bank selector command value */
	ULONG		enaIdxBits;				/* Assignmenttable */
	struct {
		UCHAR	idx;					/* To allocation member definition tableIdx */
		UCHAR	highWd;					/* 4byteWhen using membersHigh WordSpecification */
	}memAttr[BANK_MEM_MAX];				/* Member allocation table */
	MG_PRM		mgPrm[BANK_NUM_MAX];	/* MG */
	ACF_PRM		acfPrm[BANK_NUM_MAX];	/* AccFilter */
} MTBANK;


#endif /* PRM_BANK_H_ */


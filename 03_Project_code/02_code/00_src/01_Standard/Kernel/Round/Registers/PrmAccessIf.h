/****************************************************************************
Description	: Register and Parameter Access Interface
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2010 All Rights Reserved

Project       : INGRAM

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
I.M.   2010.5.22     created

****************************************************************************/
#ifndef __PRM_ACCESS_IF_H
#define __PRM_ACCESS_IF_H

#include "EepromIF.h"
#include "AlarmManager.h"							/* <S0AA> */


/* defines */
typedef enum PRM_RSLT {
	PRM_RSLT_SUCCESS		= (LONG)0x00000000L,	/* Normal end */
	PRM_RSLT_CONTINUE		= (LONG)0x00000001L,	/* Normal end Continuing(FnFor) */
//	PRM_RSLT_READYTOGO		= (LONG)0x00000002L,	/* Normal end Waiting for execution start(FnFor)	*/
	PRM_RSLT_NEEDREBOOT		= (LONG)0x00000003L,	/* Normal end Power restart required	*/
	PRM_RSLT_RAMONLY		= (LONG)0x00000004L,	/* Normal end RAM Only Parameter */
	PRM_RSLT_NO_DIFF		= (LONG)0x00000005L,	/* Normal end EEPROMandRAMis a match */
	PRM_RSLT_REGNO_ERR		= (LONG)0x80110002L,	/* PnNumber error		*/
	PRM_RSLT_LIMIT_ERR		= (LONG)0x80120006L,	/* range(Upper/lower limit value)Error	*/
	PRM_RSLT_SIZE_ERR		= (LONG)0x80150004L,	/* Size error		*/
	PRM_RSLT_CALC_ERR		= (LONG)0x80130006L,	/* Calculation error		*/
	PRM_RSLT_RWACC_ERR		= (LONG)0x80140005L,	/* Access error	*/
	PRM_RSLT_CMD_ERR		= (LONG)0x80100001L,	/* CMDError		*/
	PRM_RSLT_CONDITION_ERR	= (LONG)0x80300008L,	/* Condition error	*/
	PRM_RSLT_NOACCRIGHT_ERR	= (LONG)0x80300009L,	/* (OtherCHDue to conflict)Execution not possible error	*/
} PRM_RSLT;


typedef union _VARIANT_T {
	ULONG	ulVal;
	LONG	lVal;
	USHORT	usArray[2];
	SHORT	sArray[2];
	UCHAR	ucArray[4];
	CHAR	cArray[4];

	USHORT	usVal;
	SHORT	sVal;
	UCHAR	ucVal;
	CHAR	cVal;
} VARIANT_T;


/****************************************************************************
 Structure of Parameter Attribute and table list
****************************************************************************/
#define PRMDEF_BASENBL			0					/* Attribute: Nibble display									*/
#define PRMDEF_BASEHEX			1					/* Attribute:16Hex display									*/
#define PRMDEF_BASEDEC			2					/* Attribute:10Hex display									*/
#define PRMDEF_BASEBIT			3					/* Attribute: Bit display									*/

#define ACCLVL_USER1			1					/* User 1										*/
#define ACCLVL_USER2			2					/* User 2										*/
#define ACCLVL_USER3			3					/* User 3										*//* <S056> */
//#define ACCLVL_SYSTEM			3					/* System											*//* <S0FE> */
#define ACCLVL_SYSTEM			4					/* System											*//* <S0FE> */

/* <S056>	Start */
#define WRITE_OK_SET_VAL		0x0000				/* Writing permission Setting value								*/
#define WRITE_NG_SET_VAL		0x0001				/* Writing prohibited Setting value								*/
#define ACCLVL_USER1_SET_VAL	0x1002				/* User 1 Setting value							*/
//#define ACCLVL_USER2_SET_VAL	0x2001				/* User 2 Setting value							*//* <S136> */
//#define ACCLVL_USER3_SET_VAL	0x3001				/* User 3 Setting value							*//* <S136> */
#define ACCLVL_USER2_SET_VAL	0x1501				/* User 2 Setting value							*//* <S136> */
#define ACCLVL_USER3_SET_VAL	0x2001				/* User 3 Setting value							*//* <S136> */
#define ACCLVL_SYSTEM_SET_VAL	0x4907				/* System level Setting value						*/

/*--------------------------------------------------------------------------------------------------*/
/*		Special Use Bit Definition																	*/
/*--------------------------------------------------------------------------------------------------*/
#define	KPI_CHKSPUSE_COMMON		0x01	/* ChkSpUse Bit for Common									*/
#define	KPI_CHKSPUSE_ROTARY		0x02	/* ChkSpUse Bit for Roatry Motor							*/
#define	KPI_CHKSPUSE_LINEAR		0x04	/* ChkSpUse Bit for Linear Motor							*/
#define	KPI_CHKSPUSE_FENC		0x08	/* ChkSpUse Bit for FullClosedEncoder						*/
#define	KPI_CHKSPUSE_SAFETY		0x10	/* ChkSpUse Bit for SafetyOptionModule						*/
#define	KPI_LCDOPEDSPSEL_SETUP	0x40	/* ChkSpUse Bit for SetUpParameter Display					*/
#define	KPI_LCDOPEDSPSEL_TUNING	0x80	/* ChkSpUse Bit for TuningParameter Display					*/
/*--------------------------------------------------------------------------------------------------*/
/* <S056>	End */

//#define PRMDEF_EEPROM_IF		0
//#define PRMDEF_EEPROM_SDA		1
//#define PRMDEF_EEPROM_COA		2
//#define PRMDEF_EEPROM_MBB		3
//#define PRMDEF_NO_EEPROM		4

typedef enum PRM_ACCCMD {
	PRM_ACCCMD_READ = 0,
	PRM_ACCCMD_WRITE,
	PRM_ACCCMD_EEPWRITE,
	PRM_ACCCMD_RECALC,
	PRM_ACCCMD_ADJINCDATA,				/* <S149> */
	PRM_ACCCMD_ADJDECDATA,				/* <S149> */
} PRM_ACCCMD;

typedef PRM_RSLT (*PRMCAL_CALLBACK)(PRM_ACCCMD, UINT, void*, LONG*);
typedef void *(*PRMPTR_CALLBACK)(UINT, void*);

typedef struct PRM_ATTR_T {
	USHORT			RegNumber;			/* Register number */
//	UINT			AccessLevel		:2;	/* 1:User1, 2:User2, 3:System	*/	/* <S11B> */
	UINT			AccessLevel		:3;	/* 1:User1, 2:User2, 3:System	*/	/* <S11B> */
	UINT			AxisCommon		:1;	/* common parameter for all axises */
	UINT			NeedReboot		:1;	/* new setting value is not activated until re-boot */
	UINT			Sign			:1;	/* signed or unsigned */
	UINT			ReadOnly		:1;	/* read only */
	UINT			FormatType		:2;	/* HEX / DEC / NBL/ BIT */
/*------------------------------------------------------------------------------------------*/
/*		Lcd Operator Display Attributes 													*/
/*------------------------------------------------------------------------------------------*/
	UINT			Decimals		:3;	/* Decimals (0~) */
	UINT			ClrVar			:1;	/* Clear Variable (1:Enable, 0: Disable) */
//	UINT			Reserve0		:4;	/* Reserved */	/* <S11B> */
	UINT			Reserve0		:3;	/* Reserved */	/* <S11B> */
/*------------------------------------------------------------------------------------------*/
	UINT			SpUse			:8;	/* For Special Use (Rotary,Linear,Flc,Spindle,etc)  */
/*------------------------------------------------------------------------------------------*/
	UINT			Reserved		:1;	/* reserved */
	UINT			EepromDevice	:3;	/* Eeprom device */
	UINT			DataLength		:4;	/* data size in bytes */
/*------------------------------------------------------------------------------------------*/

	USHORT			NumberOfArray;		/* number of array(reserved) */
	USHORT			EepromAddress;		/* EEPROM address */

	LONG			LowerLimit;			/* lower limit */
	LONG			UpperLimit;			/* upper limit */
	LONG			DefaultValue;		/* System default value */

	PRMPTR_CALLBACK	GetRamPtrCallback;	/* pointer to the callback function to get the RAM address */
	PRMCAL_CALLBACK	PrmCalCallback;		/* pointer to the callback function for parameter caluculation  */
} PRM_ATTR_T;
typedef const PRM_ATTR_T	CPRM_ATTR_T;
typedef const PRM_ATTR_T*	CPPRM_ATTR_T;


typedef struct PRMTBL {
	ULONG				Number;
	const PRM_ATTR_T*	Attribute;
} PRMTBL;
typedef const PRMTBL	CPRMTBL;
typedef const PRMTBL*	CPPRMTBL;

/*------------------------------------------------------------------------------------------*/
/*		<S031> Parameter Download Table Struct Definition									*/
/*------------------------------------------------------------------------------------------*/
typedef struct PDLTBL {
	USHORT				EepAddr;				/* Parameter Eeprom Address */
	USHORT				PnNo;					/* Parameter Number */
	const PRM_ATTR_T*	pPrmDef;				/* Pointer to the Parameter Definition */
} PDLTBL;
typedef const PDLTBL	CPDLTBL;

/****************************************************************************
 Structure of Configurations for Regsiter Manager
****************************************************************************/
typedef struct _REGIF_CONFIG_T {
//	EEP_DEVICE_HANDLE	*hEepDev;			/* Pointer to Eeprom device handle */ /* Not used <S1D2> */
	CPRMTBL				*PrmListTable;		/* Pointer to the Parameter list table */
	UINT				NumOfTableEntry;	/* number of parameters */
	void				*PrmCalParam;		/* Input parameter for the callback function "GetPtrCallback" */
	void				*GetRamPtrParam;	/* Input parameter for the callback function "PrmCalCallback" */
	USHORT				EepromBlockAddress;	/* EEPROM block address */
	UINT				*SysLevel;			/* System level switch */
	void				*WriteAccessOwner;	/* Register write access owner */
	BOOL				WriteAccessLock;	/* Register write access lock */
	PRM_RSLT			RegWrResult;		/* for Executing status of Ope register */
	ALMMAN_HANDLE		*AlmManager;		/* <S0AA> Erase after */
} REGIF_CONFIG_T;
typedef REGIF_CONFIG_T*	PREGIF_CONFIG_T;

typedef struct _REG_ACC_T {
	ULONG			MemAddr;	/* Memory Address */
	PREGIF_CONFIG_T hRegCfg;	/* Pointer to Register Configuration structure */
	CPPRM_ATTR_T	cPrmDef;	/* Pointer to Register Attribute */
	UINT			ArrayIdx;	/* Array Number */
	BOOL			EepWrite;	/* EEPROM Write info */
	void			*IfHandle;	/* Interface handle ID */
} REG_ACC_T;

/****************************************************************************
 Structure of Register Manager Handle
****************************************************************************/
typedef struct _REG_MANAGER_HANDLE {
	REGIF_CONFIG_T *hPnReg;			/* Paramter Register */
	REGIF_CONFIG_T *hFnReg;			/* Operation Register */
//	REGIF_CONFIG_T *hEngReg;		/* Engneering Register */
	REGIF_CONFIG_T *hUnReg;			/* Monitor Register */
	REGIF_CONFIG_T *hInfReg;		/* Information Register */
	REGIF_CONFIG_T *hTrcReg;		/* Trace buffer Register */
	REGIF_CONFIG_T *hNetworkIdReg;	/* Network ID Register */
} REG_MANAGER_HANDLE;

/*--------------------------------------------------------------------------------------------------*/
/* <S166> 																							*/
/*--------------------------------------------------------------------------------------------------*/
typedef enum {
	OPMODE_NULL = 0,
	OPMODE_OPE1,
	OPMODE_OPE2,
	OPMODE_TRACE,
	OPMODE_NUM,
} _enum_OPMODE;
/*--------------------------------------------------------------------------------------------------*/
typedef	struct	{							/* Register IF management variables								*/
		void	*Hmsg[OPMODE_NUM];			/* Message IF handle(Writing right management & operation response)		*/
} OPACCSV;
/*--------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern	"C"	{
#endif

/* API's */
void PrmInitialize(REGIF_CONFIG_T *RegCfg, UINT AxisNo, CPRMTBL *PrmListTable,
				   void *PrmCalParam, void *GetRamPtrParam);

void PrmLoadSystemDefaultValues(REGIF_CONFIG_T* RegCfg);
LONG PrmLoadEepromValues(REGIF_CONFIG_T* RegCfg, UINT devID, BOOL bAxCommon);
PRM_RSLT PrmRecalcAllParameters(REGIF_CONFIG_T *RegCfg, BOOL bAxCommon);
void PrmStoreSystemDefaultValues(REGIF_CONFIG_T* RegCfg, BOOL bAxCommon, UINT Level);

CPPRM_ATTR_T PrmFindTableIndex(REGIF_CONFIG_T *RegCfg, UINT RegNo, UINT *ArrayIdx);
PRM_RSLT PrmReadValue(REG_ACC_T *AccessPrm, LONG *value, UINT bSize);
PRM_RSLT PrmWriteValue(REG_ACC_T *AccessPrm, LONG *value, UINT bSize);
PRM_RSLT PrmStoreValueToEeprom(REGIF_CONFIG_T *RegCfg,CPPRM_ATTR_T RegAttr, UINT ArrayIdx);
PRM_RSLT PrmStoreAllValuesToEeprom(REGIF_CONFIG_T* RegCfg);
PRM_RSLT PrmReLoadValueFromEeprom(REGIF_CONFIG_T *RegCfg,
									CPPRM_ATTR_T RegAttr, UINT ArrayIdx);
PRM_RSLT PrmLoadValueFromEeprom(REGIF_CONFIG_T *RegCfg,
									 CPPRM_ATTR_T RegAttr, UINT ArrayIdx, LONG* pVal);/* <S027> */

CPPRM_ATTR_T RegMngrGetAttrByMemAddr(REG_MANAGER_HANDLE *hRegMngr, REG_ACC_T *AccessPrm);
void RegMngrLockWriteOpeRegister(REG_MANAGER_HANDLE *hRegMngr);
void RegMngrFreeWriteOpeRegister(REG_MANAGER_HANDLE *hRegMngr);
PRM_RSLT RegMngrGetWriteOpeRegStatus(REG_MANAGER_HANDLE *hRegMngr);
void RegMngrSetWriteOpeRegStatus(REG_MANAGER_HANDLE *hRegMngr, PRM_RSLT sts);
PRM_RSLT PrmWriteValueToEeprom(REGIF_CONFIG_T *RegCfg,
								CPPRM_ATTR_T RegAttr, UINT ArrayIdx);

void RegMngrLockWriteParameters(REG_MANAGER_HANDLE *hRegMngr);
void RegMngrFreeWriteParameters(REG_MANAGER_HANDLE *hRegMngr);
BOOL RegMngrGetWritePrmAccessLock(REG_MANAGER_HANDLE *hRegMngr);


#if defined(_VC_DEBUG_) /* 2012.08.22 Y.Oka Make Mercury Base Software */
LONG DbgPrmLoadEepValues(REGIF_CONFIG_T* RegCfg, UCHAR *EepBuf, BOOL bAxCommon);
#endif


#ifdef __cplusplus
}
#endif



#endif //__PRM_ACCESS_IF_H


/****************************************************************************
  Description   : Monitor Register Table
-----------------------------------------------------------------------------
  Author        : Yaskawa Electric Corp., Copyright (c) 2010
                  All Rights Reserved

    Project       : Mercury

  Create Date   : 2014/04/15

****************************************************************************/


#include "Basedef.h"
#include "MercuryGlobals.h"
#include "UnMonFunc.h"
#include "PrmAccessIf.h"


/****************************************************************************
    Definitions of Register Attribute
****************************************************************************/

/***************************************************************************/
/*  Attribute of 0x000 : Motor rotation/Movement speed */
/***************************************************************************/
const PRM_ATTR_T undef_SpdFbk = {
    0x000,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotorSpeed,                      /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x001 : Speed command */
/***************************************************************************/
const PRM_ATTR_T undef_SpdRef = {
    0x001,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadVelocityRef,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x002 : Internal torque/Thrust command */
/***************************************************************************/
const PRM_ATTR_T undef_TrqRef = {
    0x002,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadTorqueRef,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x003 : Electrical angle1(32Bit10Hex display) */
/***************************************************************************/
const PRM_ATTR_T undef_MotAngPls = {
    0x003,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotAngle,                        /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x005 : Electrical angle2(UPhase0deg) */
/***************************************************************************/
const PRM_ATTR_T undef_MotAngDeg = {
    0x005,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotPhase,                        /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x006 : Input signal monitor(dummy) */
/***************************************************************************/
const PRM_ATTR_T undef_InSignal = {
    0x006,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDigitalInputSignals,             /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x007 : Output signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_OutSignal = {
    0x007,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDigitalOutputSignals,            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x008 : Input command pulse speed */
/***************************************************************************/
const PRM_ATTR_T undef_PosRefSpd = {
    0x008,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadCmdSpeedCounter,                 /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x009 : Deviation counter(Position deviation amount) */
/***************************************************************************/
static void *undefGetRamPtr_PosErrCntr(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->BaseControls->PosManager.var.PosErrA.per32s);}

const PRM_ATTR_T undef_PosErrCntr = {
    0x009,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_PosErrCntr,             /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x00B : Cumulative load rate */
/***************************************************************************/
const PRM_ATTR_T undef_MotLoad = {
    0x00B,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadIsqrSum,                         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x00C : Regeneration load rate */
/***************************************************************************/
const PRM_ATTR_T undef_RegLoad = {
    0x00C,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadRegenePower,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x00D : DB resistance power consumption */
/***************************************************************************/
const PRM_ATTR_T undef_DbrxPower = {
    0x00D,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDBPower,                         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x00E : Input command pulse counter */
/***************************************************************************/
static void *undefGetRamPtr_PcmdCounter(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->UnMonitor->UnPcmdCntr);}

const PRM_ATTR_T undef_PcmdCounter = {
    0x00E,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    TRUE,                                   /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_PcmdCounter,            /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x010 : Motor encoder position */
/***************************************************************************/
const PRM_ATTR_T undef_PfbkCounter = {
    0x010,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    TRUE,                                   /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotorEncoderPosition,            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x012 : Full-closed feedback pulse counter */
/***************************************************************************/
const PRM_ATTR_T undef_FposCounter = {
    0x012,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    TRUE,                                   /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadFullcEncoderPosition,            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x014 : Full-closed feedback speed */
/***************************************************************************/
const PRM_ATTR_T undef_FcPfbkSpd = {
    0x014,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x016 : Total operating time */
/***************************************************************************/
const PRM_ATTR_T undef_PowerOnTime = {
    0x016,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadTimeStamp,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x018 : Motor maximum speed upper limit */
/***************************************************************************/
static void *undefGetRamPtr_DivMaxSpdX(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->MencV->DivInfo.MaxDivSpd);}

const PRM_ATTR_T undef_DivMaxSpdX = {
    0x018,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_DivMaxSpdX,             /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x019 : Division ratio setting upper limit at maximum speed */
/***************************************************************************/
static void *undefGetRamPtr_DivMaxRatX(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->MencV->DivInfo.MaxDivRat);}

const PRM_ATTR_T undef_DivMaxRatX = {
    0x019,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_DivMaxRatX,             /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x01A : Hall sensor information */
/***************************************************************************/
const PRM_ATTR_T undef_LmPoleSensorX = {
    0x01A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadPoleSensorInfo,                  /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x01B : Feedback pulse counter */
/***************************************************************************/
const PRM_ATTR_T undef_Pfbk2Counter = {
    0x01B,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    TRUE,                                   /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadPfbkCntr,                        /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x01D : Effective gain monitor */
/***************************************************************************/
const PRM_ATTR_T undef_ActGain = {
    0x01D,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadActGain,                         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x01E : Safety input/output signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_SafetyIOSignal = {
    0x01E,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadSafetySignals,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x01F : Safety module external input/output signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_SafetyExternalIOSignal = {
    0x01F,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x020 : Safety module safety function settings/Operation monitor */
/***************************************************************************/
const PRM_ATTR_T undef_SafetySettingMon = {
    0x020,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x021 : Safety module system status monitor */
/***************************************************************************/
const PRM_ATTR_T undef_SafetySysStatMon = {
    0x021,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x022 : Safety module safe speed arrival timeA */
/***************************************************************************/
const PRM_ATTR_T undef_SafetySpdReachTimeA = {
    0x022,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x024 : Safety module safe speed arrival timeB */
/***************************************************************************/
const PRM_ATTR_T undef_SafetySpdReachTimeB = {
    0x024,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x026 : Active mode internal command speed */
/***************************************************************************/
const PRM_ATTR_T undef_ActiveModeInterCmdSpd = {
    0x026,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x027 : Safety module motor speed */
/***************************************************************************/
const PRM_ATTR_T undef_SafetyMotorSpd = {
    0x027,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x028 : Safety module motor position */
/***************************************************************************/
const PRM_ATTR_T undef_SafetyMotorPos = {
    0x028,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x02A : Safety module monitoring speedA */
/***************************************************************************/
const PRM_ATTR_T undef_SafetyMonitoringSpdA = {
    0x02A,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x02B : Safety module monitoring speedB */
/***************************************************************************/
const PRM_ATTR_T undef_SafetyMonitoringSpdB = {
    0x02B,                                  /* Register Number */
    ACCLVL_USER3,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x02C : Servopack installation environment monitor */
/***************************************************************************/
const PRM_ATTR_T undef_AmpInstEnvMon = {
    0x02C,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnAmpInstEnvMon,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x02D : Motor installation environment monitor */
/***************************************************************************/
const PRM_ATTR_T undef_MotInstEnvMon = {
    0x02D,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnMotInstEnvMon,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x02E : Main circuit voltage value */
/***************************************************************************/
const PRM_ATTR_T undef_DcVolt = {
    0x02E,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDcVolt,                          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x030 : Backlash correction amount */
/***************************************************************************/
static void *undefGetRamPtr_BlshComp(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->BaseControls->PosManager.BlshCmp.var.Filo);}

const PRM_ATTR_T undef_BlshComp = {
    0x030,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_BlshComp,               /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x032 : Backlash correction amount setting limit value */
/***************************************************************************/
static void *undefGetRamPtr_BlshCompLimit(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->BaseControls->PosManager.BlshCmp.conf.MaxCompRef);}

const PRM_ATTR_T undef_BlshCompLimit = {
    0x032,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_BlshCompLimit,          /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x034 : Power consumption */
/***************************************************************************/
static void *undefGetRamPtr_electricityConsumption(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->PowerMonitor->var.UnConsumedPower);}

const PRM_ATTR_T undef_electricityConsumption = {
    0x034,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_electricityConsumption, /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x035 : Power consumption */
/***************************************************************************/
static void *undefGetRamPtr_PowerConsumption(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->PowerMonitor->var.UnEnergyConsumption);}

const PRM_ATTR_T undef_PowerConsumption = {
    0x035,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_PowerConsumption,       /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x037 : Cumulative power consumption (integer part) */
/***************************************************************************/
static void *undefGetRamPtr_TotalPowerConsumptionConst(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->PowerMonitor->var.UnCumulativeEnergyI);}

const PRM_ATTR_T undef_TotalPowerConsumptionConst = {
    0x037,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_TotalPowerConsumptionConst,/* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x039 : Cumulative power consumption (decimal part) */
/***************************************************************************/
static void *undefGetRamPtr_TotalPowerConsumptionDecimal(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->PowerMonitor->var.UnCumulativeEnergyD);}

const PRM_ATTR_T undef_TotalPowerConsumptionDecimal = {
    0x039,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_TotalPowerConsumptionDecimal,/* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x03E : Position amplifier deviation */
/***************************************************************************/
static void *undefGetRamPtr_PosErr32s(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->BaseControls->PosManager.var.PosErr);}

const PRM_ATTR_T undef_PosErr32s = {
    0x03E,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_PosErr32s,              /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x040 : Absolute value encoder multi-turn data */
/***************************************************************************/
const PRM_ATTR_T undef_MultiTurn = {
    0x040,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x041 : Absolute value encoder1Internal rotation position */
/***************************************************************************/
const PRM_ATTR_T undef_IncPulse = {
    0x041,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x043 : Absolute value encoder position data (lower) */
/***************************************************************************/
const PRM_ATTR_T undef_ScaleAbsPos_l = {
    0x043,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x045 : Absolute value encoder position data (upper) */
/***************************************************************************/
const PRM_ATTR_T undef_ScaleAbsPos_h = {
    0x045,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x046 : Motor rated speed [Rotary type:min-1,Linear type:mm/s] */
/***************************************************************************/
const PRM_ATTR_T undef_MotRatSpd = {
    0x046,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotorRatedSpeed,                 /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x047 : Motor maximum speed [Rotary type:min-1,Linear type:mm/s] */
/***************************************************************************/
const PRM_ATTR_T undef_MotMaxSpd = {
    0x047,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotorMaxSpeed,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x050 : Built-inFANLifetime monitor */
/***************************************************************************/
const PRM_ATTR_T undef_BuiltInFanLifeMon = {
    0x050,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    2,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadFanLifeTime,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x051 : Capacitor life monitor */
/***************************************************************************/
const PRM_ATTR_T undef_CondenserLifeMon = {
    0x051,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    2,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadCapaLifeTime,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x052 : Rush protection circuit life monitor */
/***************************************************************************/
const PRM_ATTR_T undef_CollisionProofCircuitLifeMon = {
    0x052,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    2,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadRushLifeTime,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x053 : DBCircuit life monitor */
/***************************************************************************/
const PRM_ATTR_T undef_DBCircuitLifeMon = {
    0x053,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    2,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDBLifeTime,                      /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x054 : Number of thrust protection relay operations */
/***************************************************************************/
const PRM_ATTR_T undef_CollisionProofRelayMotionNum = {
    0x054,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadRushSwitchingCount,              /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x056 : DBRelay operation count */
/***************************************************************************/
const PRM_ATTR_T undef_DBRelayMotionNum = {
    0x056,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDBSwitchingCount,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x080 : Maximum amount of movement in the forward direction when detecting a magnetic pole */
/***************************************************************************/
const PRM_ATTR_T undef_MpfMovePosP = {
    0x080,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    2,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMpfMaxPositiveMoveDist,          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x081 : Maximum amount of movement in the reverse direction when detecting a magnetic pole */
/***************************************************************************/
const PRM_ATTR_T undef_MpfMovePosN = {
    0x081,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    2,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMpfMaxNegativeMoveDist,          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x082 : Magnetic pole detection time required */
/***************************************************************************/
const PRM_ATTR_T undef_MpfFindingTime = {
    0x082,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    1,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMpfDetectingTime,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x083 : Magnetic pole detection phase offset amount */
/***************************************************************************/
const PRM_ATTR_T undef_MpfPhaseOffset = {
    0x083,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    1,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMpfPhaseOffset,                  /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x084 : Linear scale pitch */
/***************************************************************************/
const PRM_ATTR_T undef_LinearScalePitch = {
    0x084,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadLinearScalePitch,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x086 : Linear scale pitch index */
/***************************************************************************/
const PRM_ATTR_T undef_LinearScalePitchIndex = {
    0x086,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadLinearScalePitchExp,             /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x087 : Magnetic pole detection judgment monitor */
/***************************************************************************/
const PRM_ATTR_T undef_MpfJudgeMonitor = {
    0x087,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMpfStatus,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x104 : Serial encoder communication error count */
/***************************************************************************/
const PRM_ATTR_T undef_SencComErrCnt = {
    0x104,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadEncoderErrorCount,               /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x105 : Positioning settling time */
/***************************************************************************/
const PRM_ATTR_T undef_SettlingTime = {
    0x105,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    1,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x106 : Positioning overshoot amount */
/***************************************************************************/
const PRM_ATTR_T undef_OverShoot = {
    0x106,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x107 : Residual vibration frequency */
/***************************************************************************/
const PRM_ATTR_T undef_RvibFrequency = {
    0x107,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    1,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadRvibFrequency,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x10A : Heat sink temperature monitor */
/***************************************************************************/
const PRM_ATTR_T undef_HeatsinkTemp = {
    0x10A,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x10B : Positioning completion failure time accumulation */
/***************************************************************************/
const PRM_ATTR_T undef_CoinOffTime = {
    0x10B,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x10C : Differential speed for oscillation detection */
/***************************************************************************/
const PRM_ATTR_T undef_VibDetSpd = {
    0x10C,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x10D : Control board temperature monitor */
/***************************************************************************/
const PRM_ATTR_T undef_CtrlBoardTemp = {
    0x10D,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x10E : Force sensor monitor */
/***************************************************************************/
const PRM_ATTR_T undef_SifAdCnvData = {
    0x10E,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x10F : Encoder internal temperature */
/***************************************************************************/
static void *undefGetRamPtr_Temperature(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->MencV->Temperature);}

const PRM_ATTR_T undef_Temperature = {
    0x10F,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_Temperature,            /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x110 : Internal signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_InterSignalX = {
    0x110,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInterSignalX,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x120 : Internal input signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_IntInSignalX = {
    0x120,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalInputSignals,            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x130 : Internal output signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_IntOutSignalX = {
    0x130,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalOutputSignals,           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x140 : Main circuit bus voltage */
/***************************************************************************/
const PRM_ATTR_T undef_DcVoltx = {
    0x140,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDCVoltage,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x141 : Current detection value */
/***************************************************************************/
const PRM_ATTR_T undef_IqRate = {
    0x141,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadIqRate,                          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x142 : Cumulative load rate */
/***************************************************************************/
const PRM_ATTR_T undef_MotLoad2ms = {
    0x142,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotorLoad2ms,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x143 : Regeneration load rate */
/***************************************************************************/
const PRM_ATTR_T undef_RegerRate2ms = {
    0x143,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadRegeneLoad2ms,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x144 : DB resistance power consumption */
/***************************************************************************/
const PRM_ATTR_T undef_DbrxPower2ms = {
    0x144,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDbPower2ms,                      /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x145 : Maximum cumulative load factor */
/***************************************************************************/
const PRM_ATTR_T undef_MotLoad10s = {
    0x145,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMaxMotorLoad10s,                 /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x146 : Moment of inertia ratio/Weight ratio */
/***************************************************************************/
static void *undefGetRamPtr_JLRatio(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->BaseControls->JatHdl.EstimatJrat);}

const PRM_ATTR_T undef_JLRatio = {
    0x146,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_JLRatio,                /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x148 : T-REFMonitor */
/***************************************************************************/
const PRM_ATTR_T undef_TrefAdMon = {
    0x148,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadTrefAdMon,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x149 : V-REFMonitor */
/***************************************************************************/
const PRM_ATTR_T undef_VrefAdMon = {
    0x149,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadVrefAdMon,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x14A : Input command pulse frequency */
/***************************************************************************/
const PRM_ATTR_T undef_DPulseHwCntMon = {
    0x14A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDPulseHwCntMon,                  /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x14C : dAxis current command monitor */
/***************************************************************************/
const PRM_ATTR_T undef_IdrefRate = {
    0x14C,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadIdRefRate,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x14D : MEMOBUSError information (error occurrence address) */
/***************************************************************************/
const PRM_ATTR_T undef_MbusErrReg = {
    0x14D,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0xffff,                                 /* Upper Limit */
    0xffff,                                 /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMemobusErrorInfo,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x150 : BoardID */
/***************************************************************************/
const PRM_ATTR_T undef_SubstrateID = {
    0x150,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x170 : Average internal temperature */
/***************************************************************************/
const PRM_ATTR_T undef_AvgIntTmpr = {
    0x170,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    2,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAvgIntTmrp,                      /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x171 : Control board temperature */
/***************************************************************************/
static void *undefGetRamPtr_ControlSubstrateTemp(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->OverHeat->UnMon.UnCtrlBoadTemp);}

const PRM_ATTR_T undef_ControlSubstrateTemp = {
    0x171,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    1,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_ControlSubstrateTemp,   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x172 : Power board temperature */
/***************************************************************************/
static void *undefGetRamPtr_PowerSubstrateTemp(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->OverHeat->UnMon.UnPowerBoadTemp);}

const PRM_ATTR_T undef_PowerSubstrateTemp = {
    0x172,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    1,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_PowerSubstrateTemp,     /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x200 : Encoder internal temperature temperature monitor */
/***************************************************************************/
const PRM_ATTR_T undef_MencTemp = {
    0x200,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotroEncoderTemperature,         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x201 : ROMChecksum value (system level) */
/***************************************************************************/
const PRM_ATTR_T undef_RomChkSumVal = {
    0x201,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x203 : Setting error parameter number(A.040) */
/***************************************************************************/
static void *undefGetRamPtr_ErrPrmNo(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->AlmManager->PrmErrInfo.ErrPrmNo);}

const PRM_ATTR_T undef_ErrPrmNo = {
    0x203,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_ErrPrmNo,               /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x204 : Limit processing parameter number(NoAlarm) */
/***************************************************************************/
const PRM_ATTR_T undef_LmtPrmNo = {
    0x204,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x205 : Servopack bootROMChecksum value (system level) */
/***************************************************************************/
const PRM_ATTR_T undef_SvPacBootRomChkSumVal = {
    0x205,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x206 : Interface module bootROMChecksum value (system level) */
/***************************************************************************/
const PRM_ATTR_T undef_IfBootRomChkSumVal = {
    0x206,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x210 : User parametersCRCCalculated value */
/***************************************************************************/
const PRM_ATTR_T undef_PrmCrcValue = {
    0x210,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadUsrPrmCrcValue,                  /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x300 : 2msaroundCPULoad factor */
/***************************************************************************/
const PRM_ATTR_T undef_CurExeTimeS = {
    0x300,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadCPULoad,                         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x302 : 2msaroundCPUMaximum load factor */
/***************************************************************************/
const PRM_ATTR_T undef_MaxExeTimeS = {
    0x302,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMaxCPULoad,                      /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x304 : ScanBCurrent processing time */
/***************************************************************************/
const PRM_ATTR_T undef_CurExeTimeB = {
    0x304,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadScanBProcessTime,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x306 : ScanBMaximum processing time */
/***************************************************************************/
const PRM_ATTR_T undef_MaxExeTimeB = {
    0x306,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    TRUE,                                   /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadScanBMaxProcessTime,             /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x308 : ScanCCurrent processing time */
/***************************************************************************/
const PRM_ATTR_T undef_CurExeTimeC = {
    0x308,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadScanCProcessTime,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x30A : ScanCMaximum processing time */
/***************************************************************************/
const PRM_ATTR_T undef_MaxExeTimeC = {
    0x30A,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    TRUE,                                   /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadScanCMaxProcessTime,             /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x310 : μProgramIntADCurrent processing time */
/***************************************************************************/
const PRM_ATTR_T undef_CurExeTimeMI = {
    0x310,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMIntADProcessTime,               /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x312 : μProgramIntADMaximum processing time */
/***************************************************************************/
const PRM_ATTR_T undef_MaxExeTimeMI = {
    0x312,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMIntADMaxProcessTime,            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x314 : μProgramScanACurrent processing time */
/***************************************************************************/
const PRM_ATTR_T undef_CurExeTimeMA = {
    0x314,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMScanAProcessTime,               /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x316 : μProgramScanAMaximum processing time */
/***************************************************************************/
const PRM_ATTR_T undef_MaxExeTimeMA = {
    0x316,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    TRUE,                                   /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMScanAMaxProcessTime,            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x318 : μProgramRoundCurrent processing time */
/***************************************************************************/
const PRM_ATTR_T undef_CurExeTimeMR = {
    0x318,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMRoundProcessTime,               /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x31A : μProgramRoundMaximum processing time */
/***************************************************************************/
const PRM_ATTR_T undef_MaxExeTimeMR = {
    0x31A,                                  /* Register Number */
    ACCLVL_SYSTEM,                          /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    FALSE,                                  /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    3,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMRoundMaxProcessTime,            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x500 : Alarm history data */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTrace = {
    0x500,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    10,                                     /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlarmHistory,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x50A : Current alarm status */
/***************************************************************************/
static void *undefGetRamPtr_AlmCode(UINT ArrayIdx, void *AxParam)
{   return &((((AXIS_HANDLE*)AxParam)->AlmManager)->AlmCode);}

const PRM_ATTR_T undef_AlmCode = {
    0x50A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_AlmCode,                /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x50B : Data for speed command adjustment */
/***************************************************************************/
const PRM_ATTR_T undef_SpdRefAdj = {
    0x50B,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &uncal_VrefAdjData,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x50C : Torque command adjustment data (positive side) */
/***************************************************************************/
const PRM_ATTR_T undef_TrqRefAdjp = {
    0x50C,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &uncal_TrefAdjData,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x50D : Analog monitor(CH1)Zero adjustment data */
/***************************************************************************/
const PRM_ATTR_T undef_AmonCh1Zadj = {
    0x50D,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &uncal_AmonZadjCh1,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x50E : Analog monitor(CH2)Zero adjustment data */
/***************************************************************************/
const PRM_ATTR_T undef_AmonCh2Zadj = {
    0x50E,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &uncal_AmonZadjCh2,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x50F : Analog monitor (CH1) Gain adjustment data */
/***************************************************************************/
const PRM_ATTR_T undef_AmonCh1Gadj = {
    0x50F,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &uncal_AmonGadjCh1,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x510 : Analog monitor (CH2) Gain adjustment data */
/***************************************************************************/
const PRM_ATTR_T undef_AmonCh2Gadj = {
    0x510,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &uncal_AmonGadjCh2,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x511 : Current detection (UPhase) Zero adjustment data */
/***************************************************************************/
const PRM_ATTR_T undef_CurUphZadj = {
    0x511,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &uncal_CurZadjUphase,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x512 : Current detection (VPhase) Zero adjustment data */
/***************************************************************************/
const PRM_ATTR_T undef_CurVphZadj = {
    0x512,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &uncal_CurZadjVphase,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x513 : Motor type */
/***************************************************************************/
const PRM_ATTR_T undef_MotorType = {
    0x513,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotorID,                         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x514 : Motor capacity */
/***************************************************************************/
const PRM_ATTR_T undef_MotorCapacity = {
    0x514,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMotorWattage,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x515 : Encoder type */
/***************************************************************************/
const PRM_ATTR_T undef_EncoderType = {
    0x515,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadEncoderType,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x516 : YSpecification */
/***************************************************************************/
const PRM_ATTR_T undef_Yspecification = {
    0x516,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadYspecVer,                        /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x517 : Servopack software version */
/***************************************************************************/
const PRM_ATTR_T undef_SvPacSoftVer = {
    0x517,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadFirmVerBase,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x518 : Encoder software version */
/***************************************************************************/
const PRM_ATTR_T undef_EncoderSoftVer = {
    0x518,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadEncoderVer,                      /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x519 : Number of flash writes (system level) */
/***************************************************************************/
const PRM_ATTR_T undef_FlashWriteNum = {
    0x519,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x51A : Option card detection information */
/***************************************************************************/
static void *undefGetRamPtr_OpDetInfo(UINT ArrayIdx, void *AxParam)
{   return &(((AXIS_HANDLE*)AxParam)->OptCardHdl->OpDetInfo);}

const PRM_ATTR_T undef_OpDetInfo = {
    0x51A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    &undefGetRamPtr_OpDetInfo,              /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x51B : Servo status monitor */
/***************************************************************************/
const PRM_ATTR_T undef_SvStatus = {
    0x51B,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadSvStatus,                        /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x51C : Control mode monitor */
/***************************************************************************/
const PRM_ATTR_T undef_CtrlMode = {
    0x51C,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadServoControlMode,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x52A : Alarm history timestamp */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTmStamp = {
    0x52A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    10,                                     /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlarmTimeStamp,                  /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x53E : Abnormal traceback counter0 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter0 = {
    0x53E,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x53F : Abnormal traceback counter1 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter1 = {
    0x53F,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x540 : Abnormal traceback counter2 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter2 = {
    0x540,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x541 : Abnormal traceback counter3 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter3 = {
    0x541,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x542 : Abnormal traceback counter4 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter4 = {
    0x542,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x543 : Abnormal traceback counter5 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter5 = {
    0x543,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x544 : Abnormal traceback counter6 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter6 = {
    0x544,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x545 : Abnormal traceback counter7 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter7 = {
    0x545,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x546 : Abnormal traceback counter8 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter8 = {
    0x546,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x547 : Abnormal traceback counter9 */
/***************************************************************************/
const PRM_ATTR_T undef_AbnormalityTraceBackCounter9 = {
    0x547,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x548 : Alarm trace data address0 */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceDataAdd0 = {
    0x548,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x549 : Alarm trace data address1 */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceDataAdd1 = {
    0x549,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x54A : Alarm trace data address2 */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceDataAdd2 = {
    0x54A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x54B : Alarm trace data address3 */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceDataAdd3 = {
    0x54B,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x54C : Alarm trace data address4 */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceDataAdd4 = {
    0x54C,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x54D : Alarm trace data address5 */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceDataAdd5 = {
    0x54D,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x54E : Alarm trace data address6 */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceDataAdd6 = {
    0x54E,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x54F : Alarm trace data address7 */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceDataAdd7 = {
    0x54F,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x558 : Alarm trace status */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTraceStat = {
    0x558,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDummy,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x700 : Sampling time unit */
/***************************************************************************/
const PRM_ATTR_T undef_SamplingTime = {
    0x700,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadSamplingTimeUnit,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x701 : Data is16384 */
/***************************************************************************/
const PRM_ATTR_T undef_MaxTorque = {
    0x701,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMaxTorqueConvertValue,           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x702 : Data is16384 */
/***************************************************************************/
const PRM_ATTR_T undef_MaxSpeed = {
    0x702,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMaxSpeedConvertValue,            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x704 : Data is16384 */
/***************************************************************************/
const PRM_ATTR_T undef_Dummy = {
    0x704,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMaxSpeedExpConvertValue,         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x705 : Internal operating status */
/***************************************************************************/
const PRM_ATTR_T undef_NewServoOpmode = {
    0x705,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalServoMode,               /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x710 : DSW/RSWInput status */
/***************************************************************************/
const PRM_ATTR_T undef_SWInputsStatus = {
    0x710,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadHSWInputsStatus,                 /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x712 : FBBoard information */
/***************************************************************************/
const PRM_ATTR_T undef_FBA_BoardID = {
    0x712,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    1,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadFBboardID,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x713 : MBBoard information */
/***************************************************************************/
const PRM_ATTR_T undef_MB_BoardID = {
    0x713,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    1,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMBboardID,                       /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x714 : Converter load current(ADvalue) */
/***************************************************************************/
const PRM_ATTR_T undef_CnvCurrentAD = {
    0x714,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    1,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadConverterCurrentAD,              /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x716 : Converter serial input data */
/***************************************************************************/
const PRM_ATTR_T undef_CnvSerialInputs = {
    0x716,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    1,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadConverterSerialInputs,           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x718 : Unit internal temperature monitor */
/***************************************************************************/
const PRM_ATTR_T undef_MbTemperature = {
    0x718,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    1,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadMbTemperature,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x719 : Converter temperature monitor */
/***************************************************************************/
const PRM_ATTR_T undef_CnvTemperature = {
    0x719,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    1,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadCnvTemperature,                  /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x71A : Control board temperature monitor */
/***************************************************************************/
const PRM_ATTR_T undef_IfTemperature = {
    0x71A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    1,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadIfTemperature,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x720 : (R)Boot software version */
/***************************************************************************/
const PRM_ATTR_T undef_R_BootsoftVersion = {
    0x720,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    2,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadBootsoftVersion,                 /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x724 : (R)Bootstrap version */
/***************************************************************************/
const PRM_ATTR_T undef_R_BootstrapVersion = {
    0x724,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    TRUE,                                   /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    2,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadBootstrapVersion,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x800 : Motor rotation/Movement speed */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtSpdFbkX = {
    0x800,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtSpdFbk,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x801 : Speed command */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtSpdRefX = {
    0x801,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtSpdRef,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x802 : Internal torque/Thrust command */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtTrqRefX = {
    0x802,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtTrqRef,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x803 : Input command pulse speed */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtPosRefSpdX = {
    0x803,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtPosRefSpd,                  /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x804 : Deviation counter(Position deviation amount) */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtPosErrCntrX = {
    0x804,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtPosErrCntr,                 /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x810 : Internal signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtInterSignalX = {
    0x810,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtMonInterSig,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x814 : Internal input signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtIntInSignalX = {
    0x814,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtMonInSig,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x818 : Internal output signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtIntOutSignalX = {
    0x818,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtMonOutSig,                  /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x820 : Main circuit bus voltage */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtDcVoltxX = {
    0x820,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtDcVoltx,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x821 : Current detection value */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtIqRateX = {
    0x821,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtIqRate,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x822 : Cumulative load rate */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtMotLoad2msX = {
    0x822,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtMotLoad2ms,                 /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x823 : Regeneration load rate */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtRegerRate2msX = {
    0x823,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtRegerRate2ms,               /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x824 : DB resistance power consumption */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtDbrxPower2msX = {
    0x824,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtDbrxPower2ms,               /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x825 : Maximum cumulative load factor */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtMotLoad10sX = {
    0x825,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtMotLoad10s,                 /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x826 : Moment of inertia ratio/Weight ratio */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtJLRatioX = {
    0x826,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtJLRatio,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x830 : Serial encoder communication error count */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtSencComErrCntX = {
    0x830,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtSencComErrCnt,              /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x831 : MECHATROLINKCommunication error count */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtM2ComErrCntX = {
    0x831,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtM2ComErrCnt,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x832 : Safety input/output signal monitor */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtSafetyIOSignalX = {
    0x832,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtMonSafetyIOSig,             /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x833 : Alarm number */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtAlmCodeX = {
    0x833,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtAlmCode,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x834 : Timestamp */
/***************************************************************************/
const PRM_ATTR_T undef_AlmLtTmStampX = {
    0x834,                                  /* Register Number */
    ACCLVL_USER2,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadAlmLtTmStamp,                    /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x900 : Trace alarm number */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTrcAlmCode = {
    0x900,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0x901 : Trace alarm timestamp */
/***************************************************************************/
const PRM_ATTR_T undef_AlmTrcTmStamp = {
    0x901,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    NULL,                                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC00 : Feedback position(APOS) */
/***************************************************************************/
const PRM_ATTR_T undef_Apos = {
    0xC00,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadApos,                            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC02 : Current command position(CPOS) */
/***************************************************************************/
const PRM_ATTR_T undef_Cpos = {
    0xC02,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadCpos,                            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC04 : Position deviation(PERR) */
/***************************************************************************/
const PRM_ATTR_T undef_Perr = {
    0xC04,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadPerr,                            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC06 : Target position(TPOS) */
/***************************************************************************/
const PRM_ATTR_T undef_Tpos = {
    0xC06,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadTpos,                            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC08 : Latch position 1(LPOS1) */
/***************************************************************************/
const PRM_ATTR_T undef_Lpos1 = {
    0xC08,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadLpos1,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC0A : Latch position 2(LPOS2) */
/***************************************************************************/
const PRM_ATTR_T undef_Lpos2 = {
    0xC0A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadLpos2,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC0C : Latch position 3(LPOS3) */
/***************************************************************************/
const PRM_ATTR_T undef_Lpos3 = {
    0xC0C,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadLpos3,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC0E : Target speed(TSPD) */
/***************************************************************************/
const PRM_ATTR_T undef_Tspd = {
    0xC0E,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadTspd,                            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC10 : Feedback speed(FSPD) */
/***************************************************************************/
const PRM_ATTR_T undef_Fspd = {
    0xC10,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadFbspd,                           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC12 : Current position command speed(CSPD) */
/***************************************************************************/
const PRM_ATTR_T undef_Cspd = {
    0xC12,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    TRUE,                                   /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadCspd,                            /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC14 : Torque limit value(TRQ_LIM) */
/***************************************************************************/
const PRM_ATTR_T undef_TrqLim = {
    0xC14,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadTrqLim,                          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC16 : Speed limit value(SPD_LIM) */
/***************************************************************************/
const PRM_ATTR_T undef_SpdLim = {
    0xC16,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    4,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadSpdLim,                          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC18 : M-ⅢCommunication phase */
/***************************************************************************/
const PRM_ATTR_T undef_M3ComPhase = {
    0xC18,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadM3CommPhase,                     /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC19 : M-ⅢCurrently executing command code(RCMD) */
/***************************************************************************/
const PRM_ATTR_T undef_M3Rcmd = {
    0xC19,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadM3MainCommandCode,               /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xC1A : M-ⅢCurrently executing subcommand code(SUB_RCMD) */
/***************************************************************************/
const PRM_ATTR_T undef_M3SubRcmd = {
    0xC1A,                                  /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEHEX,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadM3SubCommandCode,                /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF000 : All input signal monitors1(for LCD Operator) */
/***************************************************************************/
const PRM_ATTR_T undef_AllInSignal1 = {
    0xF000,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDigitalAllInputSignals1,         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF001 : All input signal monitors2(for LCD Operator) */
/***************************************************************************/
const PRM_ATTR_T undef_AllInSignal2 = {
    0xF001,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDigitalAllInputSignals2,         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF002 : All output signal monitors(for LCD Operator) */
/***************************************************************************/
const PRM_ATTR_T undef_AllOutSignal = {
    0xF002,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadDigitalAllOutputSignals,         /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF003 : Internal signal monitor1 */
/***************************************************************************/
const PRM_ATTR_T undef_InterSignalX1 = {
    0xF003,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInterSignalX1,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF004 : Internal signal monitor2 */
/***************************************************************************/
const PRM_ATTR_T undef_InterSignalX2 = {
    0xF004,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInterSignalX2,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF005 : Internal signal monitor3 */
/***************************************************************************/
const PRM_ATTR_T undef_InterSignalX3 = {
    0xF005,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInterSignalX3,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF006 : Internal signal monitor4 */
/***************************************************************************/
const PRM_ATTR_T undef_InterSignalX4 = {
    0xF006,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInterSignalX4,                   /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF007 : Internal input signal monitor 1 */
/***************************************************************************/
const PRM_ATTR_T undef_IntInSignalX1 = {
    0xF007,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalInputSignals1,           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF008 : Internal input signal monitor 2 */
/***************************************************************************/
const PRM_ATTR_T undef_IntInSignalX2 = {
    0xF008,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalInputSignals2,           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF009 : Internal input signal monitor 3 */
/***************************************************************************/
const PRM_ATTR_T undef_IntInSignalX3 = {
    0xF009,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalInputSignals3,           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF00A : Internal input signal monitor 4 */
/***************************************************************************/
const PRM_ATTR_T undef_IntInSignalX4 = {
    0xF00A,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalInputSignals4,           /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF00B : Internal output signal monitor1 */
/***************************************************************************/
const PRM_ATTR_T undef_IntOutSignalX1 = {
    0xF00B,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalOutputSignals1,          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF00C : Internal output signal monitor2 */
/***************************************************************************/
const PRM_ATTR_T undef_IntOutSignalX2 = {
    0xF00C,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalOutputSignals2,          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF00D : Internal output signal monitor3 */
/***************************************************************************/
const PRM_ATTR_T undef_IntOutSignalX3 = {
    0xF00D,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEBIT,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnReadInternalOutputSignals3,          /* Parameter Read/Write Callback Function */
};


/***************************************************************************/
/*  Attribute of 0xF00E : Motor maximum speed upper limit/Division ratio setting upper limit */
/***************************************************************************/
const PRM_ATTR_T undef_SelSpdOrDiv = {
    0xF00E,                                 /* Register Number */
    ACCLVL_USER1,                           /* Access Level */
    FALSE,                                  /* Axis common */
    FALSE,                                  /* Need Reboot */
    FALSE,                                  /* With Sign */
    TRUE,                                   /* Read Only */
    PRMDEF_BASEDEC,                         /* Display format */
    0,                                      /* Decimals */
    0,                                      /* ClrVar */
    0,                                      /* RserveX */
    0,                                      /* SpUse */
    0,                                      /* reserved */
    PRMDEF_NO_EEPROM,                       /* EEPROM device */
    2,                                      /* Data Length */
    0,                                      /* Number of Array */
    0,                                      /* EEPROM address */
    0,                                      /* Lower Limit */
    0,                                      /* Upper Limit */
    0,                                      /* Default value */
    NULL,                                   /* Parameter varialbles  */
    &UnMaxSpdOrMaxDiv,                      /* Parameter Read/Write Callback Function */
};




/****************************************************************************
    Parameter(Register) List Table
****************************************************************************/
const PRMTBL UnRegTable[] = {
    {0x000   , &undef_SpdFbk                        },/* 0x000: Motor rotation/Movement speed */
    {0x001   , &undef_SpdRef                        },/* 0x001: Speed command */
    {0x002   , &undef_TrqRef                        },/* 0x002: Internal torque/Thrust command */
    {0x003   , &undef_MotAngPls                     },/* 0x003: Electrical angle1(32Bit10Hex display) */
    {0x005   , &undef_MotAngDeg                     },/* 0x005: Electrical angle2(UPhase0deg) */
    {0x006   , &undef_InSignal                      },/* 0x006: Input signal monitor(dummy) */
    {0x007   , &undef_OutSignal                     },/* 0x007: Output signal monitor */
    {0x008   , &undef_PosRefSpd                     },/* 0x008: Input command pulse speed */
    {0x009   , &undef_PosErrCntr                    },/* 0x009: Deviation counter(Position deviation amount) */
    {0x00B   , &undef_MotLoad                       },/* 0x00B: Cumulative load rate */
    {0x00C   , &undef_RegLoad                       },/* 0x00C: Regeneration load rate */
    {0x00D   , &undef_DbrxPower                     },/* 0x00D: DB resistance power consumption */
    {0x00E   , &undef_PcmdCounter                   },/* 0x00E: Input command pulse counter */
    {0x010   , &undef_PfbkCounter                   },/* 0x010: Motor encoder position */
    {0x012   , &undef_FposCounter                   },/* 0x012: Full-closed feedback pulse counter */
    {0x014   , &undef_FcPfbkSpd                     },/* 0x014: Full-closed feedback speed */
    {0x016   , &undef_PowerOnTime                   },/* 0x016: Total operating time */
    {0x018   , &undef_DivMaxSpdX                    },/* 0x018: Motor maximum speed upper limit */
    {0x019   , &undef_DivMaxRatX                    },/* 0x019: Division ratio setting upper limit at maximum speed */
    {0x01A   , &undef_LmPoleSensorX                 },/* 0x01A: Hall sensor information */
    {0x01B   , &undef_Pfbk2Counter                  },/* 0x01B: Feedback pulse counter */
    {0x01D   , &undef_ActGain                       },/* 0x01D: Effective gain monitor */
    {0x01E   , &undef_SafetyIOSignal                },/* 0x01E: Safety input/output signal monitor */
    {0x01F   , &undef_SafetyExternalIOSignal        },/* 0x01F: Safety module external input/output signal monitor */
    {0x020   , &undef_SafetySettingMon              },/* 0x020: Safety module safety function settings/Operation monitor */
    {0x021   , &undef_SafetySysStatMon              },/* 0x021: Safety module system status monitor */
    {0x022   , &undef_SafetySpdReachTimeA           },/* 0x022: Safety module safe speed arrival timeA */
    {0x024   , &undef_SafetySpdReachTimeB           },/* 0x024: Safety module safe speed arrival timeB */
    {0x026   , &undef_ActiveModeInterCmdSpd         },/* 0x026: Active mode internal command speed */
    {0x027   , &undef_SafetyMotorSpd                },/* 0x027: Safety module motor speed */
    {0x028   , &undef_SafetyMotorPos                },/* 0x028: Safety module motor position */
    {0x02A   , &undef_SafetyMonitoringSpdA          },/* 0x02A: Safety module monitoring speedA */
    {0x02B   , &undef_SafetyMonitoringSpdB          },/* 0x02B: Safety module monitoring speedB */
    {0x02C   , &undef_AmpInstEnvMon                 },/* 0x02C: Servopack installation environment monitor */
    {0x02D   , &undef_MotInstEnvMon                 },/* 0x02D: Motor installation environment monitor */
    {0x02E   , &undef_DcVolt                        },/* 0x02E: Main circuit voltage value */
    {0x030   , &undef_BlshComp                      },/* 0x030: Backlash correction amount */
    {0x032   , &undef_BlshCompLimit                 },/* 0x032: Backlash correction amount setting limit value */
    {0x034   , &undef_electricityConsumption        },/* 0x034: Power consumption */
    {0x035   , &undef_PowerConsumption              },/* 0x035: Power consumption */
    {0x037   , &undef_TotalPowerConsumptionConst    },/* 0x037: Cumulative power consumption (integer part) */
    {0x039   , &undef_TotalPowerConsumptionDecimal  },/* 0x039: Cumulative power consumption (decimal part) */
    {0x03E   , &undef_PosErr32s                     },/* 0x03E: Position amplifier deviation */
    {0x040   , &undef_MultiTurn                     },/* 0x040: Absolute value encoder multi-turn data */
    {0x041   , &undef_IncPulse                      },/* 0x041: Absolute value encoder1Internal rotation position */
    {0x043   , &undef_ScaleAbsPos_l                 },/* 0x043: Absolute value encoder position data (lower) */
    {0x045   , &undef_ScaleAbsPos_h                 },/* 0x045: Absolute value encoder position data (upper) */
    {0x046   , &undef_MotRatSpd                     },/* 0x046: Motor rated speed [Rotary type:min-1,Linear type:mm/s] */
    {0x047   , &undef_MotMaxSpd                     },/* 0x047: Motor maximum speed [Rotary type:min-1,Linear type:mm/s] */
    {0x050   , &undef_BuiltInFanLifeMon             },/* 0x050: Built-inFANLifetime monitor */
    {0x051   , &undef_CondenserLifeMon              },/* 0x051: Capacitor life monitor */
    {0x052   , &undef_CollisionProofCircuitLifeMon  },/* 0x052: Rush protection circuit life monitor */
    {0x053   , &undef_DBCircuitLifeMon              },/* 0x053: DBCircuit life monitor */
    {0x054   , &undef_CollisionProofRelayMotionNum  },/* 0x054: Number of thrust protection relay operations */
    {0x056   , &undef_DBRelayMotionNum              },/* 0x056: DBRelay operation count */
    {0x080   , &undef_MpfMovePosP                   },/* 0x080: Maximum amount of movement in the forward direction when detecting a magnetic pole */
    {0x081   , &undef_MpfMovePosN                   },/* 0x081: Maximum amount of movement in the reverse direction when detecting a magnetic pole */
    {0x082   , &undef_MpfFindingTime                },/* 0x082: Magnetic pole detection time required */
    {0x083   , &undef_MpfPhaseOffset                },/* 0x083: Magnetic pole detection phase offset amount */
    {0x084   , &undef_LinearScalePitch              },/* 0x084: Linear scale pitch */
    {0x086   , &undef_LinearScalePitchIndex         },/* 0x086: Linear scale pitch index */
    {0x087   , &undef_MpfJudgeMonitor               },/* 0x087: Magnetic pole detection judgment monitor */
    {0x104   , &undef_SencComErrCnt                 },/* 0x104: Serial encoder communication error count */
    {0x105   , &undef_SettlingTime                  },/* 0x105: Positioning settling time */
    {0x106   , &undef_OverShoot                     },/* 0x106: Positioning overshoot amount */
    {0x107   , &undef_RvibFrequency                 },/* 0x107: Residual vibration frequency */
    {0x10A   , &undef_HeatsinkTemp                  },/* 0x10A: Heat sink temperature monitor */
    {0x10B   , &undef_CoinOffTime                   },/* 0x10B: Positioning completion failure time accumulation */
    {0x10C   , &undef_VibDetSpd                     },/* 0x10C: Differential speed for oscillation detection */
    {0x10D   , &undef_CtrlBoardTemp                 },/* 0x10D: Control board temperature monitor */
    {0x10E   , &undef_SifAdCnvData                  },/* 0x10E: Force sensor monitor */
    {0x10F   , &undef_Temperature                   },/* 0x10F: Encoder internal temperature */
    {0x110   , &undef_InterSignalX                  },/* 0x110: Internal signal monitor */
    {0x120   , &undef_IntInSignalX                  },/* 0x120: Internal input signal monitor */
    {0x130   , &undef_IntOutSignalX                 },/* 0x130: Internal output signal monitor */
    {0x140   , &undef_DcVoltx                       },/* 0x140: Main circuit bus voltage */
    {0x141   , &undef_IqRate                        },/* 0x141: Current detection value */
    {0x142   , &undef_MotLoad2ms                    },/* 0x142: Cumulative load rate */
    {0x143   , &undef_RegerRate2ms                  },/* 0x143: Regeneration load rate */
    {0x144   , &undef_DbrxPower2ms                  },/* 0x144: DB resistance power consumption */
    {0x145   , &undef_MotLoad10s                    },/* 0x145: Maximum cumulative load factor */
    {0x146   , &undef_JLRatio                       },/* 0x146: Moment of inertia ratio/Weight ratio */
    {0x148   , &undef_TrefAdMon                     },/* 0x148: T-REFMonitor */
    {0x149   , &undef_VrefAdMon                     },/* 0x149: V-REFMonitor */
    {0x14A   , &undef_DPulseHwCntMon                },/* 0x14A: Input command pulse frequency */
    {0x14C   , &undef_IdrefRate                     },/* 0x14C: dAxis current command monitor */
    {0x14D   , &undef_MbusErrReg                    },/* 0x14D: MEMOBUSError information (error occurrence address) */
    {0x150   , &undef_SubstrateID                   },/* 0x150: BoardID */
    {0x170   , &undef_AvgIntTmpr                    },/* 0x170: Average internal temperature */
    {0x171   , &undef_ControlSubstrateTemp          },/* 0x171: Control board temperature */
    {0x172   , &undef_PowerSubstrateTemp            },/* 0x172: Power board temperature */
    {0x200   , &undef_MencTemp                      },/* 0x200: Encoder internal temperature temperature monitor */
    {0x201   , &undef_RomChkSumVal                  },/* 0x201: ROMChecksum value (system level) */
    {0x202   , &undef_MbusErrReg                    },/* 0x202: MEMOBUSError information (error occurrence address) */
    {0x203   , &undef_ErrPrmNo                      },/* 0x203: Setting error parameter number(A.040) */
    {0x204   , &undef_LmtPrmNo                      },/* 0x204: Limit processing parameter number(NoAlarm) */
    {0x205   , &undef_SvPacBootRomChkSumVal         },/* 0x205: Servopack bootROMChecksum value (system level) */
    {0x206   , &undef_IfBootRomChkSumVal            },/* 0x206: Interface module bootROMChecksum value (system level) */
    {0x210   , &undef_PrmCrcValue                   },/* 0x210: User parametersCRCCalculated value */
    {0x300   , &undef_CurExeTimeS                   },/* 0x300: 2msaroundCPULoad factor */
    {0x302   , &undef_MaxExeTimeS                   },/* 0x302: 2msaroundCPUMaximum load factor */
    {0x304   , &undef_CurExeTimeB                   },/* 0x304: ScanBCurrent processing time */
    {0x306   , &undef_MaxExeTimeB                   },/* 0x306: ScanBMaximum processing time */
    {0x308   , &undef_CurExeTimeC                   },/* 0x308: ScanCCurrent processing time */
    {0x30A   , &undef_MaxExeTimeC                   },/* 0x30A: ScanCMaximum processing time */
    {0x310   , &undef_CurExeTimeMI                  },/* 0x310: μProgramIntADCurrent processing time */
    {0x312   , &undef_MaxExeTimeMI                  },/* 0x312: μProgramIntADMaximum processing time */
    {0x314   , &undef_CurExeTimeMA                  },/* 0x314: μProgramScanACurrent processing time */
    {0x316   , &undef_MaxExeTimeMA                  },/* 0x316: μProgramScanAMaximum processing time */
    {0x318   , &undef_CurExeTimeMR                  },/* 0x318: μProgramRoundCurrent processing time */
    {0x31A   , &undef_MaxExeTimeMR                  },/* 0x31A: μProgramRoundMaximum processing time */
    {0x500   , &undef_AlmTrace                      },/* 0x500: Alarm history data */
    {0x50A   , &undef_AlmCode                       },/* 0x50A: Current alarm status */
    {0x50B   , &undef_SpdRefAdj                     },/* 0x50B: Data for speed command adjustment */
    {0x50C   , &undef_TrqRefAdjp                    },/* 0x50C: Torque command adjustment data (positive side) */
    {0x50D   , &undef_AmonCh1Zadj                   },/* 0x50D: Analog monitor(CH1)Zero adjustment data */
    {0x50E   , &undef_AmonCh2Zadj                   },/* 0x50E: Analog monitor(CH2)Zero adjustment data */
    {0x50F   , &undef_AmonCh1Gadj                   },/* 0x50F: Analog monitor (CH1) Gain adjustment data */
    {0x510   , &undef_AmonCh2Gadj                   },/* 0x510: Analog monitor (CH2) Gain adjustment data */
    {0x511   , &undef_CurUphZadj                    },/* 0x511: Current detection (UPhase) Zero adjustment data */
    {0x512   , &undef_CurVphZadj                    },/* 0x512: Current detection (VPhase) Zero adjustment data */
    {0x513   , &undef_MotorType                     },/* 0x513: Motor type */
    {0x514   , &undef_MotorCapacity                 },/* 0x514: Motor capacity */
    {0x515   , &undef_EncoderType                   },/* 0x515: Encoder type */
    {0x516   , &undef_Yspecification                },/* 0x516: YSpecification */
    {0x517   , &undef_SvPacSoftVer                  },/* 0x517: Servopack software version */
    {0x518   , &undef_EncoderSoftVer                },/* 0x518: Encoder software version */
    {0x519   , &undef_FlashWriteNum                 },/* 0x519: Number of flash writes (system level) */
    {0x51A   , &undef_OpDetInfo                     },/* 0x51A: Option card detection information */
    {0x51B   , &undef_SvStatus                      },/* 0x51B: Servo status monitor */
    {0x51C   , &undef_CtrlMode                      },/* 0x51C: Control mode monitor */
    {0x52A   , &undef_AlmTmStamp                    },/* 0x52A: Alarm history timestamp */
    {0x53E   , &undef_AbnormalityTraceBackCounter0  },/* 0x53E: Abnormal traceback counter0 */
    {0x53F   , &undef_AbnormalityTraceBackCounter1  },/* 0x53F: Abnormal traceback counter1 */
    {0x540   , &undef_AbnormalityTraceBackCounter2  },/* 0x540: Abnormal traceback counter2 */
    {0x541   , &undef_AbnormalityTraceBackCounter3  },/* 0x541: Abnormal traceback counter3 */
    {0x542   , &undef_AbnormalityTraceBackCounter4  },/* 0x542: Abnormal traceback counter4 */
    {0x543   , &undef_AbnormalityTraceBackCounter5  },/* 0x543: Abnormal traceback counter5 */
    {0x544   , &undef_AbnormalityTraceBackCounter6  },/* 0x544: Abnormal traceback counter6 */
    {0x545   , &undef_AbnormalityTraceBackCounter7  },/* 0x545: Abnormal traceback counter7 */
    {0x546   , &undef_AbnormalityTraceBackCounter8  },/* 0x546: Abnormal traceback counter8 */
    {0x547   , &undef_AbnormalityTraceBackCounter9  },/* 0x547: Abnormal traceback counter9 */
    {0x548   , &undef_AlmTraceDataAdd0              },/* 0x548: Alarm trace data address0 */
    {0x549   , &undef_AlmTraceDataAdd1              },/* 0x549: Alarm trace data address1 */
    {0x54A   , &undef_AlmTraceDataAdd2              },/* 0x54A: Alarm trace data address2 */
    {0x54B   , &undef_AlmTraceDataAdd3              },/* 0x54B: Alarm trace data address3 */
    {0x54C   , &undef_AlmTraceDataAdd4              },/* 0x54C: Alarm trace data address4 */
    {0x54D   , &undef_AlmTraceDataAdd5              },/* 0x54D: Alarm trace data address5 */
    {0x54E   , &undef_AlmTraceDataAdd6              },/* 0x54E: Alarm trace data address6 */
    {0x54F   , &undef_AlmTraceDataAdd7              },/* 0x54F: Alarm trace data address7 */
    {0x558   , &undef_AlmTraceStat                  },/* 0x558: Alarm trace status */
    {0x600   , &undef_MultiTurn                     },/* 0x600: Absolute value encoder multi-turn amount */
    {0x601   , &undef_IncPulse                      },/* 0x601: Absolute value encoder1Internal rotation pulse */
    {0x603   , &undef_ScaleAbsPos_l                 },/* 0x603: Absolute value scale position (lower) */
    {0x605   , &undef_ScaleAbsPos_h                 },/* 0x605: Absolute value scale position (upper) */
    {0x700   , &undef_SamplingTime                  },/* 0x700: Sampling time unit */
    {0x701   , &undef_MaxTorque                     },/* 0x701: Data is16384 */
    {0x702   , &undef_MaxSpeed                      },/* 0x702: Data is16384 */
    {0x704   , &undef_Dummy                         },/* 0x704: Data is16384 */
    {0x705   , &undef_NewServoOpmode                },/* 0x705: Internal operating status */
    {0x710   , &undef_SWInputsStatus                },/* 0x710: DSW/RSWInput status */
    {0x712   , &undef_FBA_BoardID                   },/* 0x712: FBBoard information */
    {0x713   , &undef_MB_BoardID                    },/* 0x713: MBBoard information */
    {0x714   , &undef_CnvCurrentAD                  },/* 0x714: Converter load current(ADvalue) */
    {0x716   , &undef_CnvSerialInputs               },/* 0x716: Converter serial input data */
    {0x718   , &undef_MbTemperature                 },/* 0x718: Unit internal temperature monitor */
    {0x719   , &undef_CnvTemperature                },/* 0x719: Converter temperature monitor */
    {0x71A   , &undef_IfTemperature                 },/* 0x71A: Control board temperature monitor */
    {0x720   , &undef_R_BootsoftVersion             },/* 0x720: (R)Boot software version */
    {0x724   , &undef_R_BootstrapVersion            },/* 0x724: (R)Bootstrap version */
    {0x800   , &undef_AlmLtSpdFbkX                  },/* 0x800: Motor rotation/Movement speed */
    {0x801   , &undef_AlmLtSpdRefX                  },/* 0x801: Speed command */
    {0x802   , &undef_AlmLtTrqRefX                  },/* 0x802: Internal torque/Thrust command */
    {0x803   , &undef_AlmLtPosRefSpdX               },/* 0x803: Input command pulse speed */
    {0x804   , &undef_AlmLtPosErrCntrX              },/* 0x804: Deviation counter(Position deviation amount) */
    {0x810   , &undef_AlmLtInterSignalX             },/* 0x810: Internal signal monitor */
    {0x814   , &undef_AlmLtIntInSignalX             },/* 0x814: Internal input signal monitor */
    {0x818   , &undef_AlmLtIntOutSignalX            },/* 0x818: Internal output signal monitor */
    {0x820   , &undef_AlmLtDcVoltxX                 },/* 0x820: Main circuit bus voltage */
    {0x821   , &undef_AlmLtIqRateX                  },/* 0x821: Current detection value */
    {0x822   , &undef_AlmLtMotLoad2msX              },/* 0x822: Cumulative load rate */
    {0x823   , &undef_AlmLtRegerRate2msX            },/* 0x823: Regeneration load rate */
    {0x824   , &undef_AlmLtDbrxPower2msX            },/* 0x824: DB resistance power consumption */
    {0x825   , &undef_AlmLtMotLoad10sX              },/* 0x825: Maximum cumulative load factor */
    {0x826   , &undef_AlmLtJLRatioX                 },/* 0x826: Moment of inertia ratio/Weight ratio */
    {0x830   , &undef_AlmLtSencComErrCntX           },/* 0x830: Serial encoder communication error count */
    {0x831   , &undef_AlmLtM2ComErrCntX             },/* 0x831: MECHATROLINKCommunication error count */
    {0x832   , &undef_AlmLtSafetyIOSignalX          },/* 0x832: Safety input/output signal monitor */
    {0x833   , &undef_AlmLtAlmCodeX                 },/* 0x833: Alarm number */
    {0x834   , &undef_AlmLtTmStampX                 },/* 0x834: Timestamp */
    {0x900   , &undef_AlmTrcAlmCode                 },/* 0x900: Trace alarm number */
    {0x901   , &undef_AlmTrcTmStamp                 },/* 0x901: Trace alarm timestamp */
    {0xC00   , &undef_Apos                          },/* 0xC00: Feedback position(APOS) */
    {0xC02   , &undef_Cpos                          },/* 0xC02: Current command position(CPOS) */
    {0xC04   , &undef_Perr                          },/* 0xC04: Position deviation(PERR) */
    {0xC06   , &undef_Tpos                          },/* 0xC06: Target position(TPOS) */
    {0xC08   , &undef_Lpos1                         },/* 0xC08: Latch position 1(LPOS1) */
    {0xC0A   , &undef_Lpos2                         },/* 0xC0A: Latch position 2(LPOS2) */
    {0xC0C   , &undef_Lpos3                         },/* 0xC0C: Latch position 3(LPOS3) */
    {0xC0E   , &undef_Tspd                          },/* 0xC0E: Target speed(TSPD) */
    {0xC10   , &undef_Fspd                          },/* 0xC10: Feedback speed(FSPD) */
    {0xC12   , &undef_Cspd                          },/* 0xC12: Current position command speed(CSPD) */
    {0xC14   , &undef_TrqLim                        },/* 0xC14: Torque limit value(TRQ_LIM) */
    {0xC16   , &undef_SpdLim                        },/* 0xC16: Speed limit value(SPD_LIM) */
    {0xC18   , &undef_M3ComPhase                    },/* 0xC18: M-ⅢCommunication phase */
    {0xC19   , &undef_M3Rcmd                        },/* 0xC19: M-ⅢCurrently executing command code(RCMD) */
    {0xC1A   , &undef_M3SubRcmd                     },/* 0xC1A: M-ⅢCurrently executing subcommand code(SUB_RCMD) */
    {0xF000  , &undef_AllInSignal1                  },/* 0xF000: All input signal monitors1(for LCD Operator) */
    {0xF001  , &undef_AllInSignal2                  },/* 0xF001: All input signal monitors2(for LCD Operator) */
    {0xF002  , &undef_AllOutSignal                  },/* 0xF002: All output signal monitors(for LCD Operator) */
    {0xF003  , &undef_InterSignalX1                 },/* 0xF003: Internal signal monitor1 */
    {0xF004  , &undef_InterSignalX2                 },/* 0xF004: Internal signal monitor2 */
    {0xF005  , &undef_InterSignalX3                 },/* 0xF005: Internal signal monitor3 */
    {0xF006  , &undef_InterSignalX4                 },/* 0xF006: Internal signal monitor4 */
    {0xF007  , &undef_IntInSignalX1                 },/* 0xF007: Internal input signal monitor 1 */
    {0xF008  , &undef_IntInSignalX2                 },/* 0xF008: Internal input signal monitor 2 */
    {0xF009  , &undef_IntInSignalX3                 },/* 0xF009: Internal input signal monitor 3 */
    {0xF00A  , &undef_IntInSignalX4                 },/* 0xF00A: Internal input signal monitor 4 */
    {0xF00B  , &undef_IntOutSignalX1                },/* 0xF00B: Internal output signal monitor1 */
    {0xF00C  , &undef_IntOutSignalX2                },/* 0xF00C: Internal output signal monitor2 */
    {0xF00D  , &undef_IntOutSignalX3                },/* 0xF00D: Internal output signal monitor3 */
    {0xF00E  , &undef_SelSpdOrDiv                   },/* 0xF00E: Motor maximum speed upper limit/Division ratio setting upper limit */
    {0xFFFF  , NULL                                 },/* 0xFFFF: End of Table List */
};
const ULONG UnRegTableEntNum = sizeof(UnRegTable) / sizeof(UnRegTable[0]);


/****************************************************************************
    Parameter(Register) List Table
****************************************************************************/
const PRMTBL UnMonTable[] = {
    {0x000   , &undef_SpdFbk                        },/* 0x000: Motor rotation/Movement speed */
    {0x001   , &undef_SpdRef                        },/* 0x001: Speed command */
    {0x002   , &undef_TrqRef                        },/* 0x002: Internal torque/Thrust command */
    {0x003   , &undef_MotAngPls                     },/* 0x003: Electrical angle1(32Bit10Hex display) */
    {0x004   , &undef_MotAngDeg                     },/* 0x005: Electrical angle2(UPhase0deg) */
    {0x005   , &undef_InSignal                      },/* 0x006: Input signal monitor(dummy) */
    {0x006   , &undef_OutSignal                     },/* 0x007: Output signal monitor */
    {0x007   , &undef_PosRefSpd                     },/* 0x008: Input command pulse speed */
    {0x008   , &undef_PosErrCntr                    },/* 0x009: Deviation counter(Position deviation amount) */
    {0x009   , &undef_MotLoad                       },/* 0x00B: Cumulative load rate */
    {0x00A   , &undef_RegLoad                       },/* 0x00C: Regeneration load rate */
    {0x00B   , &undef_DbrxPower                     },/* 0x00D: DB resistance power consumption */
    {0x00C   , &undef_PcmdCounter                   },/* 0x00E: Input command pulse counter */
    {0x00D   , &undef_PfbkCounter                   },/* 0x010: Motor encoder position */
    {0x00E   , &undef_FposCounter                   },/* 0x012: Full-closed feedback pulse counter */
    {0x00F   , &undef_FcPfbkSpd                     },/* 0x014: Full-closed feedback speed */
    {0x010   , &undef_SelSpdOrDiv                   },/* 0xF00E: Motor maximum speed upper limit/Division ratio setting upper limit */
    {0x011   , &undef_LmPoleSensorX                 },/* 0x01A: Hall sensor information */
    {0x012   , &undef_PowerOnTime                   },/* 0x016: Total operating time */
    {0x013   , &undef_Pfbk2Counter                  },/* 0x01B: Feedback pulse counter */
    {0x014   , &undef_ActGain                       },/* 0x01D: Effective gain monitor */
    {0x015   , &undef_SafetyIOSignal                },/* 0x01E: Safety input/output signal monitor */
    {0x016   , &undef_SafetyExternalIOSignal        },/* 0x01F: Safety module external input/output signal monitor */
    {0x017   , &undef_SafetySettingMon              },/* 0x020: Safety module safety function settings/Operation monitor */
    {0x018   , &undef_SafetySysStatMon              },/* 0x021: Safety module system status monitor */
    {0x019   , &undef_SafetySpdReachTimeA           },/* 0x022: Safety module safe speed arrival timeA */
    {0x01A   , &undef_SafetySpdReachTimeB           },/* 0x024: Safety module safe speed arrival timeB */
    {0x01B   , &undef_ActiveModeInterCmdSpd         },/* 0x026: Active mode internal command speed */
    {0x01C   , &undef_SafetyMotorSpd                },/* 0x027: Safety module motor speed */
    {0x01D   , &undef_SafetyMotorPos                },/* 0x028: Safety module motor position */
    {0x01E   , &undef_SafetyMonitoringSpdA          },/* 0x02A: Safety module monitoring speedA */
    {0x01F   , &undef_SafetyMonitoringSpdB          },/* 0x02B: Safety module monitoring speedB */
    {0x020   , &undef_MotRatSpd                     },/* 0x046: Motor rated speed [Rotary type:min-1,Linear type:mm/s] */
    {0x021   , &undef_MotMaxSpd                     },/* 0x047: Motor maximum speed [Rotary type:min-1,Linear type:mm/s] */
    {0x025   , &undef_AmpInstEnvMon                 },/* 0x02C: Servopack installation environment monitor */
    {0x026   , &undef_MotInstEnvMon                 },/* 0x02D: Motor installation environment monitor */
    {0x027   , &undef_BuiltInFanLifeMon             },/* 0x050: Built-inFANLifetime monitor */
    {0x028   , &undef_CondenserLifeMon              },/* 0x051: Capacitor life monitor */
    {0x029   , &undef_CollisionProofCircuitLifeMon  },/* 0x052: Rush protection circuit life monitor */
    {0x02A   , &undef_DBCircuitLifeMon              },/* 0x053: DBCircuit life monitor */
    {0x02B   , &undef_CollisionProofRelayMotionNum  },/* 0x054: Number of thrust protection relay operations */
    {0x02C   , &undef_DBRelayMotionNum              },/* 0x056: DBRelay operation count */
    {0x030   , &undef_BlshComp                      },/* 0x030: Backlash correction amount */
    {0x031   , &undef_BlshCompLimit                 },/* 0x032: Backlash correction amount setting limit value */
    {0x032   , &undef_electricityConsumption        },/* 0x034: Power consumption */
    {0x033   , &undef_PowerConsumption              },/* 0x035: Power consumption */
    {0x034   , &undef_TotalPowerConsumptionConst    },/* 0x037: Cumulative power consumption (integer part) */
    {0x035   , &undef_PosErr32s                     },/* 0x03E: Position amplifier deviation */
    {0x040   , &undef_MultiTurn                     },/* 0x040: Absolute value encoder multi-turn data */
    {0x041   , &undef_IncPulse                      },/* 0x041: Absolute value encoder1Internal rotation position */
    {0x042   , &undef_ScaleAbsPos_l                 },/* 0x043: Absolute value encoder position data (lower) */
    {0x043   , &undef_ScaleAbsPos_h                 },/* 0x045: Absolute value encoder position data (upper) */
    {0x050   , &undef_AllInSignal1                  },/* 0xF000: All input signal monitors1(for LCD Operator) */
    {0x051   , &undef_AllInSignal2                  },/* 0xF001: All input signal monitors2(for LCD Operator) */
    {0x052   , &undef_AllOutSignal                  },/* 0xF002: All output signal monitors(for LCD Operator) */
    {0x080   , &undef_MpfMovePosP                   },/* 0x080: Maximum amount of movement in the forward direction when detecting a magnetic pole */
    {0x081   , &undef_MpfMovePosN                   },/* 0x081: Maximum amount of movement in the reverse direction when detecting a magnetic pole */
    {0x082   , &undef_MpfFindingTime                },/* 0x082: Magnetic pole detection time required */
    {0x083   , &undef_MpfPhaseOffset                },/* 0x083: Magnetic pole detection phase offset amount */
    {0x084   , &undef_LinearScalePitch              },/* 0x084: Linear scale pitch */
    {0x085   , &undef_LinearScalePitchIndex         },/* 0x086: Linear scale pitch index */
    {0x086   , &undef_MpfJudgeMonitor               },/* 0x087: Magnetic pole detection judgment monitor */
    {0x104   , &undef_SencComErrCnt                 },/* 0x104: Serial encoder communication error count */
    {0x105   , &undef_SettlingTime                  },/* 0x105: Positioning settling time */
    {0x106   , &undef_OverShoot                     },/* 0x106: Positioning overshoot amount */
    {0x107   , &undef_RvibFrequency                 },/* 0x107: Residual vibration frequency */
    {0x10A   , &undef_HeatsinkTemp                  },/* 0x10A: Heat sink temperature monitor */
    {0x10B   , &undef_CoinOffTime                   },/* 0x10B: Positioning completion failure time accumulation */
    {0x10C   , &undef_VibDetSpd                     },/* 0x10C: Differential speed for oscillation detection */
    {0x10D   , &undef_CtrlBoardTemp                 },/* 0x10D: Control board temperature monitor */
    {0x10E   , &undef_SifAdCnvData                  },/* 0x10E: Force sensor monitor */
    {0x10F   , &undef_Temperature                   },/* 0x10F: Encoder internal temperature */
    {0x110   , &undef_InterSignalX                  },/* 0x110: Internal signal monitor */
    {0x110   , &undef_InterSignalX1                 },/* 0xF003: Internal signal monitor1 */
    {0x111   , &undef_InterSignalX2                 },/* 0xF004: Internal signal monitor2 */
    {0x112   , &undef_InterSignalX3                 },/* 0xF005: Internal signal monitor3 */
    {0x113   , &undef_InterSignalX4                 },/* 0xF006: Internal signal monitor4 */
    {0x120   , &undef_IntInSignalX                  },/* 0x120: Internal input signal monitor */
    {0x120   , &undef_IntInSignalX1                 },/* 0xF007: Internal input signal monitor 1 */
    {0x121   , &undef_IntInSignalX2                 },/* 0xF008: Internal input signal monitor 2 */
    {0x122   , &undef_IntInSignalX3                 },/* 0xF009: Internal input signal monitor 3 */
    {0x123   , &undef_IntInSignalX4                 },/* 0xF00A: Internal input signal monitor 4 */
    {0x130   , &undef_IntOutSignalX                 },/* 0x130: Internal output signal monitor */
    {0x130   , &undef_IntOutSignalX1                },/* 0xF00B: Internal output signal monitor1 */
    {0x131   , &undef_IntOutSignalX2                },/* 0xF00C: Internal output signal monitor2 */
    {0x132   , &undef_IntOutSignalX3                },/* 0xF00D: Internal output signal monitor3 */
    {0x140   , &undef_DcVoltx                       },/* 0x140: Main circuit bus voltage */
    {0x141   , &undef_IqRate                        },/* 0x141: Current detection value */
    {0x142   , &undef_MotLoad2ms                    },/* 0x142: Cumulative load rate */
    {0x143   , &undef_RegerRate2ms                  },/* 0x143: Regeneration load rate */
    {0x144   , &undef_DbrxPower2ms                  },/* 0x144: DB resistance power consumption */
    {0x145   , &undef_MotLoad10s                    },/* 0x145: Maximum cumulative load factor */
    {0x146   , &undef_JLRatio                       },/* 0x146: Moment of inertia ratio/Weight ratio */
    {0x148   , &undef_TrefAdMon                     },/* 0x148: T-REFMonitor */
    {0x149   , &undef_VrefAdMon                     },/* 0x149: V-REFMonitor */
    {0x14A   , &undef_DPulseHwCntMon                },/* 0x14A: Input command pulse frequency */
    {0x14C   , &undef_IdrefRate                     },/* 0x14C: dAxis current command monitor */
    {0x14D   , &undef_MbusErrReg                    },/* 0x14D: MEMOBUSError information (error occurrence address) */
    {0x170   , &undef_AvgIntTmpr                    },/* 0x170: Average internal temperature */
    {0x171   , &undef_ControlSubstrateTemp          },/* 0x171: Control board temperature */
    {0x172   , &undef_PowerSubstrateTemp            },/* 0x172: Power board temperature */
    {0x1D0   , &undef_AlmLtSpdFbkX                  },/* 0x800: Motor rotation/Movement speed */
    {0x1D1   , &undef_AlmLtSpdRefX                  },/* 0x801: Speed command */
    {0x1D2   , &undef_AlmLtTrqRefX                  },/* 0x802: Internal torque/Thrust command */
    {0x1D3   , &undef_AlmLtPosRefSpdX               },/* 0x803: Input command pulse speed */
    {0x1D4   , &undef_AlmLtPosErrCntrX              },/* 0x804: Deviation counter(Position deviation amount) */
    {0x1DF   , &undef_AlmLtDcVoltxX                 },/* 0x820: Main circuit bus voltage */
    {0x1E0   , &undef_AlmLtIqRateX                  },/* 0x821: Current detection value */
    {0x1E1   , &undef_AlmLtMotLoad2msX              },/* 0x822: Cumulative load rate */
    {0x1E2   , &undef_AlmLtRegerRate2msX            },/* 0x823: Regeneration load rate */
    {0x1E3   , &undef_AlmLtDbrxPower2msX            },/* 0x824: DB resistance power consumption */
    {0x1E4   , &undef_AlmLtMotLoad10sX              },/* 0x825: Maximum cumulative load factor */
    {0x1E5   , &undef_AlmLtJLRatioX                 },/* 0x826: Moment of inertia ratio/Weight ratio */
    {0x1E6   , &undef_AlmLtSencComErrCntX           },/* 0x830: Serial encoder communication error count */
    {0x1E7   , &undef_AlmLtM2ComErrCntX             },/* 0x831: MECHATROLINKCommunication error count */
    {0x1E8   , &undef_AlmLtSafetyIOSignalX          },/* 0x832: Safety input/output signal monitor */
    {0x1E9   , &undef_AlmLtAlmCodeX                 },/* 0x833: Alarm number */
    {0x1EA   , &undef_AlmLtTmStampX                 },/* 0x834: Timestamp */
    {0x200   , &undef_MencTemp                      },/* 0x200: Encoder internal temperature temperature monitor */
    {0x201   , &undef_RomChkSumVal                  },/* 0x201: ROMChecksum value (system level) */
    {0x202   , &undef_MbusErrReg                    },/* 0x202: MEMOBUSError information (error occurrence address) */
    {0x203   , &undef_ErrPrmNo                      },/* 0x203: Setting error parameter number(A.040) */
    {0x204   , &undef_LmtPrmNo                      },/* 0x204: Limit processing parameter number(NoAlarm) */
    {0x212   , &undef_CurExeTimeMA                  },/* 0x314: μProgramScanACurrent processing time */
    {0x213   , &undef_MaxExeTimeMA                  },/* 0x316: μProgramScanAMaximum processing time */
    {0x214   , &undef_CurExeTimeB                   },/* 0x304: ScanBCurrent processing time */
    {0x215   , &undef_MaxExeTimeB                   },/* 0x306: ScanBMaximum processing time */
    {0x216   , &undef_CurExeTimeC                   },/* 0x308: ScanCCurrent processing time */
    {0x217   , &undef_MaxExeTimeC                   },/* 0x30A: ScanCMaximum processing time */
    {0x220   , &undef_CurExeTimeS                   },/* 0x300: 2msaroundCPULoad factor */
    {0x221   , &undef_MaxExeTimeS                   },/* 0x302: 2msaroundCPUMaximum load factor */
    {0xFFFF  , NULL                                 },/* 0xFFFF: End of Table List */
};
const ULONG UnMonTableEntNum = sizeof(UnMonTable) / sizeof(UnMonTable[0]);



/******************************* End of File *******************************/


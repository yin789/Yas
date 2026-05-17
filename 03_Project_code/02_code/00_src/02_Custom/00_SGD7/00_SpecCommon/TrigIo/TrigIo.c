/****************************************************************************
Description	: Trigger I/O functions
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp., 
				Copyright (c) 2012 All Rights Reserved

Project       : SGDZ-BS63/80

----------------------------------------------------------------------------
Changes		:
Name   Date          Description
------------------------------------------------------------------------
H.N.   2012.03.05     created

****************************************************************************/
#include "IngramGlobals.h"
#include "HwDeviceIf.h"

/* define */
#define HW_PLD_TRIGIO_DELAYTIME_ADDR_L	(HW_PLD_BASE_ADDRESS + 0x90001C)	/* OUTE Output delay time setting registerL */
#define HW_PLD_TRIGIO_DELAYTIME_ADDR_H	(HW_PLD_BASE_ADDRESS + 0x90001E)	/* OUTF Output delay time setting registerH */
#define HW_PLD_TRIGIO_OUTPIN_ADDR		(HW_PLD_BASE_ADDRESS + 0x900002)	/* OUT1 Output terminal setting register */
#define HW_PLD_TRIGIO_ONTIME_ADDR		(HW_PLD_BASE_ADDRESS + 0x90000C)	/* OUT6 ONTime setting register */
#define HW_PLD_TRIGIO_STS_ADDR			(HW_PLD_BASE_ADDRESS + 0x100020)	/* IN10 Status monitor output register */

#define TRIG_IO_NORMAL 					0
#define TRIG_IO_MANUAL 					1
#define ENC_COM_DELAY 					131		/* Encoder communication delay[0.5us] 		 65.625us */
#define ENC_SAMPLING_TIME				125		/* Position information sampling time[0.5us] 62.5us */
#define DELAYTIME_CALC_RANGE			3		/* TriggerI/OConsiderationkTime calculation area[62.5us] 250us */

/* function prototype */

/* Trigger I/O HAL Driver */
static void hALdrv_WriteTrigIoDelayTimeL(USHORT delayTime, USHORT tbl_no);
static void hALdrv_WriteTrigIoDelayTimeH(USHORT delayTime, USHORT tbl_no);
static void hALdrv_WriteTrigIoPinSetting(USHORT pinNo, USHORT tbl_no);
static void hALdrv_WriteTrigIoOnTime(USHORT onTime, USHORT tbl_no);
static USHORT hALdrv_ReadTrigIoSts(void);

/* Trigger I/O Parameter register calculation function */
PRM_RSLT pncal_trigIoTbl1_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl1_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl2_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl2_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl3_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl3_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl4_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl4_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl5_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl5_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl6_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl6_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl7_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl7_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl8_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT pncal_trigIoTbl8_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);

PRM_RSLT fnCalcTriggerIOStsMon(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);
PRM_RSLT fnCalcTriggerIOControl(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue);

/* Trigger I/O Application processing */
static LONG calcDelayTime(TRIGIO_INFO *trigWk, LONG crtPos, SHORT bufPos);
void TrigIOSetDelayTime(AXIS_HANDLE	*Axis);
void TrigIOInitialize(AXIS_HANDLE *Axis);

/****************************************************************************/
/*																			*/
/*																			*/
/*		Trigger I/O HAL Driver Definition											*/
/*																			*/
/*																			*/
/****************************************************************************/


/****************************************************************************/
/* Summary: TriggerIO Output delay time setting process										*/
/****************************************************************************/
/* Argument		:	delayTime		Delay time[us]								*/
/*			:	tbl_no			Table number								*/
/* Return value	:	None														*/
/****************************************************************************/
static void hALdrv_WriteTrigIoDelayTimeL(USHORT delayTime, USHORT tbl_no)
{
	*((USHORT*)(HW_PLD_TRIGIO_DELAYTIME_ADDR_L+tbl_no*0x100)) = delayTime;
}

static void hALdrv_WriteTrigIoDelayTimeH(USHORT delayTime, USHORT tbl_no)
{
	*((USHORT*)(HW_PLD_TRIGIO_DELAYTIME_ADDR_H+tbl_no*0x100)) = delayTime;
}

/****************************************************************************/
/* Summary: TriggerIO Output terminal setting processing											*/
/****************************************************************************/
/* Argument		:	pinNo			Pin number									*/
/*			:	tbl_no			Table number								*/
/* Return value	:	None														*/
/****************************************************************************/
static void hALdrv_WriteTrigIoPinSetting(USHORT pinNo, USHORT tbl_no)
{
	*((VUSHORT*)(HW_PLD_TRIGIO_OUTPIN_ADDR + tbl_no*0x100)) = pinNo;
}

/****************************************************************************/
/* Summary: TriggerIO ONTime setting processing											*/
/****************************************************************************/
/* Argument		:	onTime			ONTime[us]									*/
/*			:	tbl_no			Table number								*/
/* Return value	:	None														*/
/****************************************************************************/
static void hALdrv_WriteTrigIoOnTime(USHORT onTime, USHORT tbl_no)
{
	*((VUSHORT*)(HW_PLD_TRIGIO_ONTIME_ADDR + tbl_no*0x100)) = onTime;
}

/****************************************************************************/
/* Summary: TriggerIOStatus monitor acquisition process											*/
/****************************************************************************/
/*			:	tbl_no			Table number								*/
/* Return value	:	TriggerIOStatus monitor output										*/
/****************************************************************************/
static USHORT hALdrv_ReadTrigIoSts(void)
{
	return *((VUSHORT*)(HW_PLD_TRIGIO_STS_ADDR));
}

/****************************************************************************/
/*																			*/
/*																			*/
/*		Trigger I/O Parameter register calculation function Definition						*/
/*																			*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*		Pn642 : TriggerIOTable1 Axis number, output pin, passing direction				*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl1_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT wk1, wk2, i;
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[0];

	wk1 = trigWk->trigConfig;
	trigWk->axisNo  =   wk1 & 0x000F;
	trigWk->pinNo   =  (wk1 & 0x00F0)>>4;
	wk2 = (wk1 & 0x0F00)>>8;
	if(wk2 == 1)
	{	trigWk->passDir = -1;	}
	else
	{	trigWk->passDir = 1;	}
	hALdrv_WriteTrigIoPinSetting(trigWk->pinNo, 0);// pinNo of ServoFPGA 
	
	for(i=0;i<TRIG_IO_LSTPOS_MAX;i++)
	{/* Selected axis MotAbsPosto buffer */
		trigWk->lstPos[i] = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	}
		
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		Pn643 : TriggerIOTable1 IOOutput time[us]							*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl1_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	hALdrv_WriteTrigIoOnTime(Axis->TrigInfo[0].onTime, 0);	// onTime of ServoFPGA 
	return PRM_RSLT_SUCCESS;
}

/* Table2 */
/****************************************************************************/
/*		Pn64A : TriggerIOTable2 Axis number, output pin, passing direction				*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl2_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT wk1, wk2, i;
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[1];

	wk1 = trigWk->trigConfig;
	trigWk->axisNo  =   wk1 & 0x000F;
	trigWk->pinNo   =  (wk1 & 0x00F0)>>4;
	wk2 = (wk1 & 0x0F00)>>8;
	if(wk2 == 1)
	{	trigWk->passDir = -1;	}
	else
	{	trigWk->passDir = 1;	}
	hALdrv_WriteTrigIoPinSetting(trigWk->pinNo, 1);// pinNo of ServoFPGA 
	
	for(i=0;i<TRIG_IO_LSTPOS_MAX;i++)
	{/* Selected axis MotAbsPosto buffer */
		trigWk->lstPos[i] = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	}
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		Pn64B : TriggerIOTable2 IOOutput time[us]							*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl2_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	hALdrv_WriteTrigIoOnTime(Axis->TrigInfo[1].onTime, 1);	// onTime of ServoFPGA 
	return PRM_RSLT_SUCCESS;
}

/* Table3 */
/****************************************************************************/
/*		Pn652 : TriggerIOTable3 Axis number, output pin, passing direction				*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl3_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT wk1, wk2, i;
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[2];

	wk1 = trigWk->trigConfig;
	trigWk->axisNo  =   wk1 & 0x000F;
	trigWk->pinNo   =  (wk1 & 0x00F0)>>4;
	wk2 = (wk1 & 0x0F00)>>8;
	if(wk2 == 1)
	{	trigWk->passDir = -1;	}
	else
	{	trigWk->passDir = 1;	}
	hALdrv_WriteTrigIoPinSetting(trigWk->pinNo, 2);// pinNo of ServoFPGA 
	
	for(i=0;i<TRIG_IO_LSTPOS_MAX;i++)
	{/* Selected axis MotAbsPosto buffer */
		trigWk->lstPos[i] = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	}
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		Pn653 : TriggerIOTable3 IOOutput time[us]							*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl3_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	hALdrv_WriteTrigIoOnTime(Axis->TrigInfo[2].onTime, 2);	// onTime of ServoFPGA 
	return PRM_RSLT_SUCCESS;
}

/* Table4 */
/****************************************************************************/
/*		Pn65A : TriggerIOTable4 Axis number, output pin, passing direction				*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl4_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT wk1, wk2, i;
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[3];

	wk1 = trigWk->trigConfig;
	trigWk->axisNo  =   wk1 & 0x000F;
	trigWk->pinNo   =  (wk1 & 0x00F0)>>4;
	wk2 = (wk1 & 0x0F00)>>8;
	if(wk2 == 1)
	{	trigWk->passDir = -1;	}
	else
	{	trigWk->passDir = 1;	}
	hALdrv_WriteTrigIoPinSetting(trigWk->pinNo, 3);// pinNo of ServoFPGA 
	
	for(i=0;i<TRIG_IO_LSTPOS_MAX;i++)
	{/* Selected axis MotAbsPosto buffer */
		trigWk->lstPos[i] = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	}
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		Pn65B : TriggerIOTable4 IOOutput time[us]							*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl4_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	hALdrv_WriteTrigIoOnTime(Axis->TrigInfo[3].onTime, 3);	// onTime of ServoFPGA 
	return PRM_RSLT_SUCCESS;
}

/* Table5 */
/****************************************************************************/
/*		Pn662 : TriggerIOTable5 Axis number, output pin, passing direction				*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl5_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT wk1, wk2, i;
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[4];

	wk1 = trigWk->trigConfig;
	trigWk->axisNo  =   wk1 & 0x000F;
	trigWk->pinNo   =  (wk1 & 0x00F0)>>4;
	wk2 = (wk1 & 0x0F00)>>8;
	if(wk2 == 1)
	{	trigWk->passDir = -1;	}
	else
	{	trigWk->passDir = 1;	}
	hALdrv_WriteTrigIoPinSetting(trigWk->pinNo, 4);// pinNo of ServoFPGA 
	
	for(i=0;i<TRIG_IO_LSTPOS_MAX;i++)
	{/* Selected axis MotAbsPosto buffer */
		trigWk->lstPos[i] = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	}
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		Pn663 : TriggerIOTable5 IOOutput time[us]							*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl5_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	hALdrv_WriteTrigIoOnTime(Axis->TrigInfo[4].onTime, 4);	// onTime of ServoFPGA 
	return PRM_RSLT_SUCCESS;
}

/* Table6 */
/****************************************************************************/
/*		Pn66A : TriggerIOTable6 Axis number, output pin, passing direction				*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl6_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT wk1, wk2, i;
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[5];

	wk1 = trigWk->trigConfig;
	trigWk->axisNo  =   wk1 & 0x000F;
	trigWk->pinNo   =  (wk1 & 0x00F0)>>4;
	wk2 = (wk1 & 0x0F00)>>8;
	if(wk2 == 1)
	{	trigWk->passDir = -1;	}
	else
	{	trigWk->passDir = 1;	}
	hALdrv_WriteTrigIoPinSetting(trigWk->pinNo, 5);// pinNo of ServoFPGA 
	
	for(i=0;i<TRIG_IO_LSTPOS_MAX;i++)
	{/* Selected axis MotAbsPosto buffer */
		trigWk->lstPos[i] = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	}
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		Pn66B : TriggerIOTable6 IOOutput time[us]							*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl6_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	hALdrv_WriteTrigIoOnTime(Axis->TrigInfo[5].onTime, 5);	// onTime of ServoFPGA 
	return PRM_RSLT_SUCCESS;
}

/* Table7 */
/****************************************************************************/
/*		Pn672 : TriggerIOTable7 Axis number, output pin, passing direction				*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl7_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT wk1, wk2, i;
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[6];

	wk1 = trigWk->trigConfig;
	trigWk->axisNo  =   wk1 & 0x000F;
	trigWk->pinNo   =  (wk1 & 0x00F0)>>4;
	wk2 = (wk1 & 0x0F00)>>8;
	if(wk2 == 1)
	{	trigWk->passDir = -1;	}
	else
	{	trigWk->passDir = 1;	}
	hALdrv_WriteTrigIoPinSetting(trigWk->pinNo, 6);// pinNo of ServoFPGA 
	
	for(i=0;i<TRIG_IO_LSTPOS_MAX;i++)
	{/* Selected axis MotAbsPosto buffer */
		trigWk->lstPos[i] = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	}
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		Pn673 : TriggerIOTable7 IOOutput time[us]							*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl7_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	hALdrv_WriteTrigIoOnTime(Axis->TrigInfo[6].onTime, 6);	// onTime of ServoFPGA 
	return PRM_RSLT_SUCCESS;
}

/* Table8 */
/****************************************************************************/
/*		Pn67A : TriggerIOTable8 Axis number, output pin, passing direction				*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl8_axisNo(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	USHORT wk1, wk2, i;
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[7];

	wk1 = trigWk->trigConfig;
	trigWk->axisNo  =   wk1 & 0x000F;
	trigWk->pinNo   =  (wk1 & 0x00F0)>>4;
	wk2 = (wk1 & 0x0F00)>>8;
	if(wk2 == 1)
	{	trigWk->passDir = -1;	}
	else
	{	trigWk->passDir = 1;	}
	hALdrv_WriteTrigIoPinSetting(trigWk->pinNo, 7);// pinNo of ServoFPGA 
	
	for(i=0;i<TRIG_IO_LSTPOS_MAX;i++)
	{/* Selected axis MotAbsPosto buffer */
		trigWk->lstPos[i] = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	}
	
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		Pn67B : TriggerIOTable8 IOOutput time[us]							*/
/****************************************************************************/
PRM_RSLT pncal_trigIoTbl8_onTime(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	hALdrv_WriteTrigIoOnTime(Axis->TrigInfo[7].onTime, 7);	// onTime of ServoFPGA 
	return PRM_RSLT_SUCCESS;
}

/* Register */
/****************************************************************************/
/*		 0x2300: TriggerIOStatus monitor register									*/
/****************************************************************************/
PRM_RSLT fnCalcTriggerIOStsMon(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{
	switch(Cmd)
	{
	case PRM_ACCCMD_READ:
		// wk = Servo FPGA;
		*pValue = (LONG)hALdrv_ReadTrigIoSts();
		break;	
	default:
		/* do nothing */
		break;
	}
	return PRM_RSLT_SUCCESS;
}

/****************************************************************************/
/*		 0x2301: TriggerIOControl register										*/
/****************************************************************************/
PRM_RSLT fnCalcTriggerIOControl(PRM_ACCCMD Cmd, UINT ArrayIdx, AXIS_HANDLE *Axis, LONG *pValue)
{

	USHORT	wk,wk1,wk2,tbl_no;
	BOOL	trigIOEnable_wk[8];	/* TriggerIOEnabled [0:Disabled,1:Enabled] */
	BOOL	trigIOMode_wk[8];	/* Mode [0:Normal mode,1:Manual output] */
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	
	trigWk = &Axis->TrigInfo[0];
	
	switch(Cmd)
	{
	case PRM_ACCCMD_WRITE:
	case PRM_ACCCMD_EEPWRITE:
		wk = (USHORT)*pValue;	
		wk1 = wk & 0x00FF;
		wk2 = (wk & 0xFF00)>>8;

#if 0
		// NOTE: Setting the global variable here would cause the ScanA trigger IO process to execute - NOT OK
		/* TriggerIOOutput mode */
		trigWk->trigIOMode =  wk2 & 0x01;
		(trigWk+1)->trigIOMode = (wk2 & 0x02)>>1;
		(trigWk+2)->trigIOMode = (wk2 & 0x04)>>2;
		(trigWk+3)->trigIOMode = (wk2 & 0x08)>>3;
		(trigWk+4)->trigIOMode = (wk2 & 0x10)>>4;
		(trigWk+5)->trigIOMode = (wk2 & 0x20)>>5;
		(trigWk+6)->trigIOMode = (wk2 & 0x40)>>6;
		(trigWk+7)->trigIOMode = (wk2 & 0x80)>>7;
	
		/* TriggerIOOutput function enabled */
		trigWk->trigIOEnable =  wk1 & 0x01;
		(trigWk+1)->trigIOEnable = (wk1 & 0x02)>>1;
		(trigWk+2)->trigIOEnable = (wk1 & 0x04)>>2;
		(trigWk+3)->trigIOEnable = (wk1 & 0x08)>>3;
		(trigWk+4)->trigIOEnable = (wk1 & 0x10)>>4;
		(trigWk+5)->trigIOEnable = (wk1 & 0x20)>>5;
		(trigWk+6)->trigIOEnable = (wk1 & 0x40)>>6;
		(trigWk+7)->trigIOEnable = (wk1 & 0x80)>>7;
#endif
		/* TriggerIOOutput mode */
		trigIOMode_wk[0] =  wk2 & 0x01;
		trigIOMode_wk[1] = (wk2 & 0x02)>>1;
		trigIOMode_wk[2] = (wk2 & 0x04)>>2;
		trigIOMode_wk[3] = (wk2 & 0x08)>>3;
		trigIOMode_wk[4] = (wk2 & 0x10)>>4;
		trigIOMode_wk[5] = (wk2 & 0x20)>>5;
		trigIOMode_wk[6] = (wk2 & 0x40)>>6;
		trigIOMode_wk[7] = (wk2 & 0x80)>>7;

		/* TriggerIOOutput function enabled */
		trigIOEnable_wk[0] =  wk1 & 0x01;
		trigIOEnable_wk[1] = (wk1 & 0x02)>>1;
		trigIOEnable_wk[2] = (wk1 & 0x04)>>2;
		trigIOEnable_wk[3] = (wk1 & 0x08)>>3;
		trigIOEnable_wk[4] = (wk1 & 0x10)>>4;
		trigIOEnable_wk[5] = (wk1 & 0x20)>>5;
		trigIOEnable_wk[6] = (wk1 & 0x40)>>6;
		trigIOEnable_wk[7] = (wk1 & 0x80)>>7;		
		
		for( tbl_no = 0; tbl_no < 8; tbl_no++ )
		{			
//			if((trigWk+tbl_no)->trigIOEnable == TRUE) 
			if(trigIOEnable_wk[tbl_no] == TRUE) 
			{
//				if((trigWk+tbl_no)->trigIOMode == TRIG_IO_NORMAL)	
				if(trigIOMode_wk[tbl_no]  == TRIG_IO_NORMAL)	
				{/* As initial value 0xFFFF set */
					hALdrv_WriteTrigIoDelayTimeH(0x0000, tbl_no);
					hALdrv_WriteTrigIoDelayTimeL(0xFFFF, tbl_no);				
				}
				else
				{/* ManualDelay of Set to output delay time setting register */
					wk1 = (USHORT)((trigWk+tbl_no)->manualDelay & 0x0000FFFF);
					wk2 = (USHORT)(((trigWk+tbl_no)->manualDelay & 0xFFFF0000) >> 16);				
					hALdrv_WriteTrigIoDelayTimeH(wk2, tbl_no);
					hALdrv_WriteTrigIoDelayTimeL(wk1, tbl_no);					
				}
			}
			else
			{/* TriggerIOIf disabled As output delay time0x7FFFFFFFset */
				hALdrv_WriteTrigIoDelayTimeH(0x7FFF, tbl_no);
				hALdrv_WriteTrigIoDelayTimeL(0xFFFF, tbl_no);
				(trigWk+tbl_no)->posReady = FALSE;	/* Position not ready */	
			}
		}
		
		/* TriggerIOOutput mode */
		trigWk->trigIOMode =  trigIOMode_wk[0];
		(trigWk+1)->trigIOMode = trigIOMode_wk[1];
		(trigWk+2)->trigIOMode = trigIOMode_wk[2];
		(trigWk+3)->trigIOMode = trigIOMode_wk[3];
		(trigWk+4)->trigIOMode = trigIOMode_wk[4];
		(trigWk+5)->trigIOMode = trigIOMode_wk[5];
		(trigWk+6)->trigIOMode = trigIOMode_wk[6];
		(trigWk+7)->trigIOMode = trigIOMode_wk[7];
	
		/* TriggerIOOutput function enabled */
		trigWk->trigIOEnable =  trigIOEnable_wk[0];
		(trigWk+1)->trigIOEnable = trigIOEnable_wk[1];
		(trigWk+2)->trigIOEnable = trigIOEnable_wk[2];
		(trigWk+3)->trigIOEnable = trigIOEnable_wk[3];
		(trigWk+4)->trigIOEnable = trigIOEnable_wk[4];
		(trigWk+5)->trigIOEnable = trigIOEnable_wk[5];
		(trigWk+6)->trigIOEnable = trigIOEnable_wk[6];
		(trigWk+7)->trigIOEnable = trigIOEnable_wk[7];
		
		break;

	case PRM_ACCCMD_READ:
	default:
		wk1 = hALdrv_ReadTrigIoSts();
		wk  = wk1 & 0xFF;
	
		wk |= trigWk->trigIOMode << 8;
		wk |= (trigWk+1)->trigIOMode << 9;
		wk |= (trigWk+2)->trigIOMode << 10;
		wk |= (trigWk+3)->trigIOMode << 11;
		wk |= (trigWk+4)->trigIOMode << 12;
		wk |= (trigWk+5)->trigIOMode << 13;	
		wk |= (trigWk+6)->trigIOMode << 14;			
		wk |= (trigWk+7)->trigIOMode << 15;	
		
		*pValue = (LONG)wk;
		break;
	}
	return PRM_RSLT_SUCCESS;
}
// ScanA? ⇒No problem

/****************************************************************************/
/*																			*/
/*																			*/
/*		Trigger I/O Application processing Definition								*/
/*																			*/
/*																			*/
/****************************************************************************/

/****************************************************************************
  Trigger IO Module initialization process 
****************************************************************************/
void TrigIOInitialize(AXIS_HANDLE *Axis)
{
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	USHORT tbl_no;			/* Table number */
	
	for( tbl_no = 0; tbl_no < 8; tbl_no++ )
	{
		trigWk = &Axis->TrigInfo[tbl_no];
		MlibResetByteMemory( trigWk, sizeof(TRIGIO_INFO) );
	}
}

/****************************************************************************
  Trigger IO TriggerI/OOutput time calculation process 
****************************************************************************/
static LONG calcDelayTime(TRIGIO_INFO *trigWk, LONG crtPos, SHORT bufPos)
{
	LONG delayTime;			/* I/OOutput delay time */
	LONG dtrigPos,dcrtPos;	/* Distance to trigger position, difference between previous value and current value */
	
	dtrigPos = trigWk->trigPosition - crtPos;
	dcrtPos  = crtPos - trigWk->lstPos[bufPos];
	
	if(trigWk->passDir*dtrigPos >= 0)
	{/* TriggerI/OPosition valid */
		trigWk->posReady = TRUE;
	}
	
	if(trigWk->posReady == TRUE)
	{
	    if(crtPos == trigWk->lstPos[bufPos])
		{/* When speed is zero */
			delayTime = 0x7FFFFFFF;
		}
		else if(MlibABS(dtrigPos) > 0x7FFFFFFF/(ENC_SAMPLING_TIME*(trigWk->mSpdCycle+1)))
		{/* TriggerI/OWhen output time calculation process overflows */
			delayTime = 0x7FFFFFFF;
//			delayTime = (0x7FFFFFFF/MlibABS(dcrtPos) - ENC_COM_DELAY)/2 -trigWk->onAdjTime;
		}
		else
		{/* Normal state */
			if(trigWk->passDir*dcrtPos>0)
			{/* Passing direction=Trigger direction */
				delayTime = (trigWk->passDir*dtrigPos*ENC_SAMPLING_TIME*(trigWk->mSpdCycle+1)
					/MlibABS(dcrtPos) - ENC_COM_DELAY)/2 - trigWk->onAdjTime;
			}
			else
			{/* When the passing direction is reverse */
					delayTime = 0x7FFFFFFF;	
			}
		}	
	}
	else
	{
		delayTime = 0x7FFFFFFF;
	}
	
	return delayTime;
}

/****************************************************************************
  Setting Trigger IO Delay Time for SeroFPGA 
****************************************************************************/
void TrigIOSetDelayTime(AXIS_HANDLE	*Axis)
{
	LONG delayTime = 0xFFFF;/* TriggerI/OOutput delay time[us] */
	USHORT tbl_no;			/* Table number */
	LONG crtPos;			/* Current position */
	TRIGIO_INFO *trigWk;	/* TriggerIOSetting information */
	SHORT bufPos;			/* ring buffer position */
	
	for( tbl_no = 0; tbl_no < 8; tbl_no++ )
	{
		trigWk = &Axis->TrigInfo[tbl_no];
		crtPos = (Axis+trigWk->axisNo)->MencV->MotAbsPos;
	
		if((trigWk->trigIOMode == TRIG_IO_NORMAL) && (trigWk->trigIOEnable == TRUE))
		{
			/* Calculate position in buffer */
			bufPos = trigWk->fbPosBufPtr - trigWk->mSpdCycle;
			if(bufPos < 0){bufPos = TRIG_IO_LSTPOS_MAX + bufPos;}
		
			/* delayTimeCalculation */
			delayTime = calcDelayTime(trigWk, crtPos, bufPos);
			
			/* ServoFPGAProcessing to setting range */
			if(delayTime > 0xFFFF)
			{
				delayTime = 0xFFFF;
			}
			else if(delayTime <= 0)
			{
				trigWk->trigIOEnable = FALSE;	// OnceI/OEnd after output
				delayTime = 0;
			}
			/* delayTimethe value ofServoFPGA */
			hALdrv_WriteTrigIoDelayTimeL((USHORT)delayTime, tbl_no);			
		}
		
		/* Update the buffer */
		trigWk->fbPosBufPtr++;
		if(trigWk->fbPosBufPtr >= TRIG_IO_LSTPOS_MAX)
		{trigWk->fbPosBufPtr = 0;}			
		trigWk->lstPos[trigWk->fbPosBufPtr] = crtPos;
	}	
}
// When in manual mode, set the manual delay time in the register using the parameter calculation function
// MotAbsPos is not held globally, soTrigIoHold in structure
// Encoder base timer value isJL-077toCPUside is not read.

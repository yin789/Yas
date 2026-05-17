/****************************************************************************
Description	: Definitions for each product functionality
----------------------------------------------------------------------------
File name		: SpecCommon.h		<S008>

Author			: Yaskawa Electric Corp.,
					Copyright (c) 2012 All Rights Reserved

Project			: Mercury

----------------------------------------------------------------------------
Changes :
	Name	Date		Description
  ------------------------------------------------------------------------
	Y.Oka	2013.08.17	created

******************************************************************************/
#ifndef _SPEC_COMMON_H
#define _SPEC_COMMON_H



/****************************************************************************
	Define the FirmWare Version
****************************************************************************/
#if (FLOAT_USE==TRUE)	/* FloatCompatible version is provisionally minorverF		<S00A>	*/
#define SERVO_FIRM_VER		0xF009F016	/* SoftwareVer. */
										/* byte3,4:MajorVer. */
										/* byte1,2:MinorVer. */
#else
#define SERVO_FIRM_VER		0xF0090016	/* SoftwareVer. */
										/* byte3,4:MajorVer. */
										/* byte1,2:MinorVer. */
#endif

/* <S056>	Start */
/********************************************************************/
/*																	*/
/*	"SERVO_YSPEC_VER” atYSpecification,FTSpecification,EXSpecificationVer, perform	*/
/*	Change definition name to "SERVO_SPECIAL_SPEC_VER"				*/
/*																	*/
/********************************************************************/
//#define SERVO_YSPEC_VER		0x0000	/* B4-B15:YSpecificationVer. */
										/* B0-B3:YSpecificationVer.Revision number */

#define SERVO_SPECIAL_SPEC_VER	0x0000	/* B4-B15:specialVer. */
										/* B0-B3:specialVer.Revision number */


#define REG_SPEC_VER	0x0E000101	/* Register spec version */
									/* B0-B7:DeviceID */
									/* B8-B15:SupplierID */
									/* B16-B23:Minor version */
									/* B24-B31:Major version */

#define SERVO_BOOT_VER	0x0002		/* Software bootVer. */

/************************************************************************************************************/
/*																											*/
/*ParameterVer.The management method is as follows																		*/
/*	.If the factory setting is changed, add 1 to the data revision number.(Data revision number:PRM_DATA_VERSION)					*/
/*	.When changing the format of parameters (increase/decrease the number of parameters, change the arrangement), add 1 to the format revision number,		*/
/*	and add 1 to the data revision number. Calculate the torque command filter time constant for the notch frequency from(Format revision number:PRM_FORMAT_VERSION)								*/
/*	.Revision number is defined in hexadecimal (binary)															*/
/*																											*/
/************************************************************************************************************/
/* ★Register support, register specification(900-133-284), "If it is outside the range of [/4] [5] in the range ofPRM_VERSION” parametersVer.Resign from management and use the following2,Ver.Changed to manage. */
//#define PRM_VERSION			0x0001		/* ParameterVer. */
#define PRM_DATA_VERSION		0x0001		/* ParameterVer.(Data revision number) */
#define PRM_FORMAT_VERSION		0x0001		/* ParameterVer.(Format revision number) */
/* <S056>	End */



/****************************************************************************
	Define the CSW_NETIF_TYPE
****************************************************************************/
#define	NETIF_ANLGPLS_TYPE   1			/* Analog/Pulse type				*/
#define	NETIF_M2_TYPE        2			/* MECHATROLINK-II type				*/
#define	NETIF_M3_TYPE        3			/* MECHATROLINK-III type			*/
#define	NETIF_CM_TYPE   	 4			/* ECATIntegratedCMDare treated the same.(As far as possible) <S008>	*/
#define	NETIF_SYNQNET_TYPE   5			/* Synqnet??						*/
#define	NETIF_ECAT_TYPE   	 6			/* EtherCATProcess the processSoCIf you do it with		*/



/****************************************************************************
	Define the CSW_BOARD_TYPE	<S0A2>
****************************************************************************/
#define	SGD7S_TYPE			1			/* Power Board is SGD7S Type		*/
#define	SGD7W_TYPE			2			/* Power Board is SGD7W Type		*/

#endif /* _SPEC_COMMON_H */
/***************************************** end of file **********************************************/

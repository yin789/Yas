/****************************************************************************
Description	: USB function driver
----------------------------------------------------------------------------
Author        : Yaskawa Electric Corp.,
				Copyright (c) 2013 All Rights Reserved

Project       : Mercury

Functions	  :
	-- APIs --
	UsbFuncInitializeDriver()		: USBDriver initialization processing
	UsbFuncChkAvailable()			: USBUsable judgment processing	<S138>
	 - Interrupt related -
	UsbFuncIntIntu2f()				: INTU2F Interrupt processing
	UsbFuncIntIntu2fepc()			: INTU2FEPC Interrupt processing
	 - Control transfer related -
	UsbFuncTaskControl()			: Control transfer task processing
	 - Bulk transfer related processing -
	UsbFuncPollBulkOutIn()			: USB Bulk transfer control processing
	UsbFuncResetBuffer()			: USB Endpoint1/2 Transmission/reception buffer clear processing
	 - USB MEMOBUSServer related -
	UsbFuncStartBkout()				: USB Endpoint2 Start reception(BLUK OUT)Processing
	UsbFuncStartBkin()				: USB Endpoint1 Transmission start(BLUK IN)Processing
	UsbFuncPortConfiguration()		: USBPort status notification service processing
	UsbFuncGetRcvSize()				: USB Endpoint2 Receive byte count acquisition process
	UsbFuncChkRcvComp()				: Bulk transfer(OUT)Status notification service processing
	UsbFuncChkTrnsComp()			: Bulk transfer(IN)Status notification service processing

	-- Locals --
	VendorRequest_Callback()		: Vendor request callback processing
	UsbFuncInitializeInfo()			: USBFunction information initialization processing
	UsbFuncClearFlag()				: Clear flag processing
	UsbFuncSetMaxpacket()			: Max Packet Size Setting processing
	UsbFuncGetInfoAddress()			: USBObtaining address of information area
	UsbFuncCheckPlugInout()			: Connection status confirmation process
	UsbFuncCheckSetupReceive()		: SETUPToken reception confirmation processing
	 - EPnTransfer related -
	UsbFuncSetStallEPn()			: EPn Stall response processing
	UsbFuncOutDataSplitCopy()		: OUTData division copy processing <S182>
	UsbFuncInDataSplitCopy()		: INData division copy processing	 <S182>
	 - Control transfer related -
	UsbFuncWriteEP0()				: EP0 Send processing
	UsbFuncSetStallEP0()			: EP0 Stall response processing
	UsbFuncSetNakEP0()				: EP0 NAKResponse processing
	UsbFuncClearNakEP0()			: EP0 NAKCancellation process
	UsbFuncSendNullEP0()			: EP0 NULLSend processing
	control_nodata()				: EP0 No DataProcessing
	control_read()					: EP0 Control ReadProcessing
	control_write()					: EP0 Control WriteProcessing
	decode_request()				: EP0 Request decode processing
	bmRequestType_standard()		: EP0 Standard request processing
	std_req_clear_feature()			: EP0 CLEAR_FEATUREProcessing
	Clear_Feature_EP()				: EP0 CLEAR_FEATUREProcessing( TARGET = EP )
	std_req_get_configuration()		: EP0 GET_CONFIGURATIONProcessing
	std_req_get_descriptor()		: EP0 GET_DESCRIPTORProcessing
	Get_Descriptor_Device()			: EP0 GET_DESCRIPTOR DEVICEProcessing
	Get_Descriptor_Configuration()	: EP0 GET_DESCRIPTOR CONFIGURATIONProcessing
	Get_Descriptor_String()			: EP0 GET_DESCRIPTOR STRINGProcessing
	Get_Descriptor_Qualifier()		: EP0 GET_DESCRIPTOR DEVICE QUALIFIERProcessing
	Get_Descriptor_OtherSpeed()		: EP0 GET_DESCRIPTOR OTHER_SPEED_CONFIGURATIONProcessing
	std_req_get_interface()			: EP0 GET_INTERFACEProcessing
	std_req_get_status()			: EP0 GET_STATUSProcessing
	Get_Status_Device()				: EP0 GET_STATUS DEVICEProcessing
	Get_Status_EP()					: EP0 GET_STATUS ENDPOINTProcessing
	std_req_set_address()			: EP0 SET_ADDRESSProcessing
	std_req_set_configuration()		: EP0 SET_CONFIGURATIONProcessing
	std_req_set_descriptor()		: EP0 SET_DESCRIPTORProcessing
	std_req_set_feature()			: EP0 SET_FEATUREProcessing
	Set_Feature_Device()			: EP0 SET_FEATUREProcessing( TARGET = DEVICE )
	Set_Feature_EP()				: EP0 SET_FEATUREProcessing( TARGET = EP )
	std_req_set_interface()			: EP0 SET_INTERFACEProcessing
	std_req_synch_frame()			: EP0 SYNCH_FRAMEProcessing
	Check_USB_DIR_EPn()				: EPDirectionUSB_DIR_OUT/INConversion processing
	udev_get_DeviceState()			: Device state acquisition processing
	info_get_CurrConfigDesc()		: info Current configuration descriptor acquisition process
	info_find_InterfaceDesc()		: info Interface descriptor acquisition process
	EPn_get_STALL()					: EPn STALLState acquisition processing

----------------------------------------------------------------------------
Changes		:
Name		Date			Description
------------------------------------------------------------------------
K.Sakamoto	2013.05.12		created

****************************************************************************/
#include "Basedef.h"
#include "config.h"
//<S146>#include "nbpfahb32vic128.h"
#include "ARMPF_USB_FUNC.h"
#include "usb_f.h"
#include "u2f_desc.h"
#include "MLib.h"
#include "Klib.h"/*<S0C7>*/
#ifdef	_MERCURY_PLATFORM_		/*<S190>*/
#ifndef NULL
	#define NULL 0
#endif
#endif	/* _MERCURY_PLATFORM_ */
/*===========================================================================*/
/* defines																	 */
/*===========================================================================*/
/* Max Packet Size (High Speed, Full Speed) */
#define BULK_MAXPACKET_SIZE_HS	 512					/* High SpeedBulk transfer maximum size		*/
#define BULK_MAXPACKET_SIZE_FS	  64					/* Full SpeedBulk transfer maximum size		*/
#define INT_MAXPACKET_SIZE_HS	1024					/* High Speed	*/
#define INT_MAXPACKET_SIZE_FS	  64					/* Full Speed	*/

/* Packet Size (High Speed, Full Speed) */
#define PACKET_SIZE_FULLSPEED_H		((UINT8)0x00)		/* FS Packet Size :  64byte	*/
#define PACKET_SIZE_FULLSPEED_L		((UINT8)0x40)		/*							*/
#define PACKET_SIZE_HIGHSPEED_H		((UINT8)0x02)		/* HS Packet Size : 512byte	*/
#define PACKET_SIZE_HIGHSPEED_L		((UINT8)0x00)		/*							*/

#define USB_VENDOR_ID_H				((UINT8)0x13)		/* Vendor ID = Yaskawa Electric(0x137C)	*/
#define USB_VENDOR_ID_L				((UINT8)0x7C)		/*										*/
#define USB_PRODUCT_ID_H			((UINT8)0x02)		/* Product ID (0x0250)		*/
#define USB_PRODUCT_ID_L			((UINT8)0x50)		/*							*/
#define USB_RELEASE_NUM_H			((UINT8)0x01)		/* Release Number (0x0100)	*/
#define USB_RELEASE_NUM_L			((UINT8)0x00)		/*							*/
#define USB_VERSION_H				((UINT8)0x02)		/* USB2.0					*/
#define USB_VERSION_L				((UINT8)0x00)		/*							*/

#define USB_INTERFACE_CLASS			((UCHAR)0x00)		/* bInterfaceClass    (none)			*/
#define USB_INTERFACE_SUB_CLASS		((UCHAR)0x00)		/* bInterfaceSubClass (none)			*/
#define USB_INTERFACE_PROTOCOL		((UCHAR)0xFF)		/* bInterfaceProtocol (Vendor original)	*/

/* USB Device State	*/
#define UDEV_ST_IDLE		BIT0						/* Idle(Attached)State	*/
#define UDEV_ST_DEFAULT		BIT1						/* DefaultState			*/
#define UDEV_ST_ADDRESS		BIT2						/* AddressState			*/
#define UDEV_ST_CONFIGURED	BIT3						/* ConfiguredState		*/
#define UDEV_ST_SUSPENDED	BIT4						/* SuspendState			*/

/* U2F FLAG	*/
#define U2F_ENABLE			1							/* Permit						*/
#define U2F_DISABLE			0							/* Prohibited						*/

/* When copying transmission/reception data1Copy size in scan */ /*<S182>*/
#define SCAN_COPY_SIZE		64							/* 1Copy size in scan(4*n)(Change prohibited)	*/

/*===========================================================================*/
/* globals in this file														 */
/*===========================================================================*/
static USB_FUNCTION_IF	UsbfIf;							/* Driver internal data.					*/
static BOOL				UsbFunctionAvailable;			/* USBUsable judgment flag				*//* <S138> */

/*===========================================================================*/
/* const datas																 */
/*===========================================================================*/
/*-----------------------------------------------*/
/* Device Descriptor							 */
/*-----------------------------------------------*/
const DEVICE_DESCRIPTOR UsbFunctionDeviceDescriptor = {
	(UCHAR)0x12,								/* bLength(18)				*/
	(UCHAR)USB_DESC_TYPE_DEVICE,				/* bDescriptor(1)			*/
	{
		(UINT8)USB_VERSION_L,						/* bcdUSB(0x0200)		*/
		(UINT8)USB_VERSION_H,						/* bcdUSB(0x0200)		*/
	},
	(UCHAR)0x00,								/* bDeviceClass				*/
	(UCHAR)0x00,								/* bDeviceSubClass			*/
	(UCHAR)0x00,								/* bDeviceProtocol			*/
	(UCHAR)0x40,								/* bMaxPacketSize0(64)		*/
	{
		(UINT8)USB_VENDOR_ID_L,						/* idVendor				*/
		(UINT8)USB_VENDOR_ID_H,						/* idVendor				*/
	},
	{
		(UINT8)USB_PRODUCT_ID_L,					/* idProduct			*/
		(UINT8)USB_PRODUCT_ID_H,					/* idProduct			*/
	},
	{
		(UINT8)USB_RELEASE_NUM_L,					/* bcdDevice			*/
		(UINT8)USB_RELEASE_NUM_H,					/* bcdDevice			*/
	},
	(UCHAR)STRIDX_MANUFACTURER,					/* iManufacturer			*/
	(UCHAR)STRIDX_PRODUCT,						/* iProduct					*/
/*	FIXME:With debugging action(Need to confirm at a later date) */
//	(UCHAR)STRIDX_SERIALNUMBER,					/* iSerialNumber			*/
	(UCHAR)0x00,					/* iSerialNumber			*//* for debug tanaka21	*/
	(UCHAR)0x01,								/* bNumConfigurations		*/
};

/*-----------------------------------------------*/
/* Qualifier Descriptor							 */
/*-----------------------------------------------*/
const DEVICE_QUALIFIER_DESCRIPTOR UsbFunctionQualifierDescriptor = {
	(UCHAR)0x0a,								/* bLength;					*/
	(UCHAR)USB_DESC_TYPE_DEVICE_QUALIFIER,		/* bDescriptorType;			*/
	{
		(UINT8)USB_VERSION_L,						/* bcdUSB(0x0200)		*/
		(UINT8)USB_VERSION_H,						/* bcdUSB(0x0200)		*/
	},
	(UCHAR)0x00,								/* bDeviceClass;			*/
	(UCHAR)0x00,								/* bDeviceSubClass;			*/
	(UCHAR)0x00,								/* bDeviceProtocol;			*/
	(UCHAR)0x40,								/* bMaxPacketSize0;			*/
	(UCHAR)0x01,								/* bNumConfigurations;		*/
	(UCHAR)0x00,								/* bReserved;				*/
};

/*-----------------------------------------------*/
/* Configuration Descriptor (FS)				 */
/*-----------------------------------------------*/
const DESC_CONFIG UsbFunctionConfigurationDescriptorFS = {
	/* Configuration	*/
	{
		(UCHAR)0x09,							/* bLength(9)				*/
		(UCHAR)USB_DESC_TYPE_CONFIGURATION,		/* bDescriptor(2)			*/
		{
			(UINT8)0x20,							/* wTotalLength(32)		*/
			(UINT8)0x00,							/* wTotalLength			*/
		},
		(UCHAR)0x01,							/* bNumInterfaces			*/
		(UCHAR)0x01,							/* bConfigurationValue		*/
		(UCHAR)STRIDX_CONFIGURATION,			/* iConfiguration			*/
		(UCHAR)0xc0,							/* bmAttributes				*/
		(UCHAR)0x00,							/* MaxPower					*/
	},
	/* Interface	*/
	{
		/* Interface[0]	*/
		{
			(UCHAR)0x09,							/* bLength(9)					*/
			(UCHAR)USB_DESC_TYPE_INTERFACE,			/* bDescriptorType				*/
			(UCHAR)0x00,							/* bInterfaceNumber				*/
			(UCHAR)0x00,							/* bAlternateSetting			*/
			(UCHAR)0x02,							/* bNumEndpoints				*/
			(UCHAR)USB_INTERFACE_CLASS,				/* bInterfaceClass				*/
			(UCHAR)USB_INTERFACE_SUB_CLASS,			/* bInterfaceSubClass			*/
			(UCHAR)USB_INTERFACE_PROTOCOL,			/* bInterfaceProtocol			*/
			(UCHAR)STRIDX_INTERFACE,				/* iInterface					*/
			/* Endpoint	*/
			{
				{
				/* EP1	*/
					(UCHAR)0x07,						/* bLength(7)					*/
					USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType				*/
					(UCHAR)0x81,						/* bEndpointAddress (EP1 IN)	*/
					(UCHAR)0x02,						/* bmAttributes (BULK)			*/
					{
						(UINT8)PACKET_SIZE_FULLSPEED_L,		/* wMaxPacketSize			*/
						(UINT8)PACKET_SIZE_FULLSPEED_H,		/* wMaxPacketSize			*/
					},
					(UCHAR)0x00,						/* bInterval					*/
				},
				/* EP2	*/
				{
					(UCHAR)0x07,						/* bLength(7)					*/
					USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType				*/
					(UCHAR)0x02,						/* bEndpointAddress (EP2 OUT)	*/
					(UCHAR)0x02,						/* bmAttributes (BULK)			*/
					{
						(UINT8)PACKET_SIZE_FULLSPEED_L,		/* wMaxPacketSize			*/
						(UINT8)PACKET_SIZE_FULLSPEED_H,		/* wMaxPacketSize			*/
					},
					(UCHAR)0x00,						/* bInterval					*/
				},
			},
		},
	},
};

/*-----------------------------------------------*/
/* Configuration Descriptor (HS)				 */
/*-----------------------------------------------*/
const DESC_CONFIG UsbFunctionConfigurationDescriptorHS = {
	/* Configuration	*/
	{
		(UCHAR)0x09,							/* bLength(9)							*/
		(UCHAR)USB_DESC_TYPE_CONFIGURATION,		/* bDescriptor(2)						*/
		{
			(UINT8)0x20,							/* wTotalLength(32)					*/
			(UINT8)0x00,							/* wTotalLength						*/
		},
		(UCHAR)0x01,							/* bNumInterfaces						*/
		(UCHAR)0x01,							/* bConfigurationValue					*/
		(UCHAR)STRIDX_CONFIGURATION,			/* iConfiguration						*/
		(UCHAR)0xc0,							/* bmAttributes							*/
		(UCHAR)0x00,							/* MaxPower								*/
	},
	/* Interface	*/
	{
		{
			(UCHAR)0x09,							/* bLength(9)							*/
			(UCHAR)USB_DESC_TYPE_INTERFACE,			/* bDescriptorType						*/
			(UCHAR)0x00,							/* bInterfaceNumber						*/
			(UCHAR)0x00,							/* bAlternateSetting					*/
			(UCHAR)0x02,							/* bNumEndpoints						*/
			(UCHAR)USB_INTERFACE_CLASS,				/* bInterfaceClass						*/
			(UCHAR)USB_INTERFACE_SUB_CLASS,			/* bInterfaceSubClass 					*/
			(UCHAR)USB_INTERFACE_PROTOCOL,			/* bInterfaceProtocol					*/
			(UCHAR)STRIDX_INTERFACE,				/* iInterface							*/
			/* Endpoint	*/
			{
				{
				/* EP1	*/
					(UCHAR)0x07,						/* bLength(7)					*/
					USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType				*/
					(UCHAR)0x81,						/* bEndpointAddress (EP1 IN)	*/
					(UCHAR)0x02,						/* bmAttributes (BULK)			*/
					{
						(UINT8)PACKET_SIZE_HIGHSPEED_L,		/* wMaxPacketSize			*/
						(UINT8)PACKET_SIZE_HIGHSPEED_H,		/* wMaxPacketSize			*/
					},
					(UCHAR)0x00,						/* bInterval					*/
				},
				/* EP2	*/
				{
					(UCHAR)0x07,						/* bLength(7)					*/
					USB_DESC_TYPE_ENDPOINT,				/* bDescriptorType				*/
					(UCHAR)0x02,						/* bEndpointAddress (EP2 OUT)	*/
					(UCHAR)0x02,						/* bmAttributes (BULK)			*/
					{
						(UINT8)PACKET_SIZE_HIGHSPEED_L,		/* wMaxPacketSize			*/
						(UINT8)PACKET_SIZE_HIGHSPEED_H,		/* wMaxPacketSize			*/
					},
					(UCHAR)0x00,						/* bInterval					*/
				},
			},
		},
	},
};

/****************************************************************************************************/
/*																									*/
/*		GET_DESCRIPTOR String StringDescriptor definition									*/
/*																									*/
/****************************************************************************************************/
/*--------------------------------------------------------------------------------------------------*/
/*		Language ID																					*/
/*--------------------------------------------------------------------------------------------------*/
const UCHAR LanguageIdStringDescriptor[]=
{
4,				/* This discrepter's size(byte)														*/
0x03,			/* Disclipter type(0x03Fixed)														*/
0x09,0x04		/* 0x0409 : ID=American language																	*/
};

/*--------------------------------------------------------------------------------------------------*/
/*		Manufacturer																						*/
/*--------------------------------------------------------------------------------------------------*/
const UCHAR ImanufacturerStringDescriptor[]=
{
58,		/* This discrepter's size(byte) */
0x03,	/* Disclipter type(0x03Fixed) */
/* String */
/* YASKAWA ELECTRIC CORPORATION */
/*   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
	'Y', 0 ,'A', 0 ,'S', 0 ,'K', 0 ,'A', 0 ,'W', 0 ,'A', 0 ,' ', 0 ,
	'E', 0 ,'L', 0 ,'E', 0 ,'C', 0 ,'T', 0 ,'R', 0 ,'I', 0 ,'C', 0 ,
	' ', 0 ,'C', 0 ,'O', 0 ,'R', 0 ,'P', 0 ,'O', 0 ,'R', 0 ,'A', 0 ,
	'T', 0 ,'I', 0 ,'O', 0 ,'N', 0
};

/*--------------------------------------------------------------------------------------------------*/
/*		Product																						*/
/*--------------------------------------------------------------------------------------------------*/
const UCHAR IproductStringDescriptor[]=
{
42,		/* This discrepter's size(byte) */
0x03,	/* Disclipter type(0x03Fixed) */
/* String */
/* YASKAWA SIGMA SERIES */
/*   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
	'Y', 0 ,'A', 0 ,'S', 0 ,'K', 0 ,'A', 0 ,'W', 0 ,'A', 0 ,' ', 0 ,
	'S', 0 ,'I', 0 ,'G', 0 ,'M', 0 ,'A', 0 ,' ', 0 ,'S', 0 ,'E', 0 ,
	'R', 0 ,'I', 0 ,'E', 0 ,'S', 0
};
/*--------------------------------------------------------------------------------------------------*/
/*		Serial number																				*/
/*--------------------------------------------------------------------------------------------------*/
const UCHAR IserialNumberStringDescriptor[]=
{
34,		/* This discrepter's size(byte) */
0x03,	/* Disclipter type(0x03Fixed) */
/* String */
/* 0000000000000000 (16Character) */
/*   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
	'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,
	'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,'0', 0 ,'0', 0
};
/*--------------------------------------------------------------------------------------------------*/
/*		Configuration																		*/
/*--------------------------------------------------------------------------------------------------*/
const UCHAR IconfigurationStringDescriptor[]=
{
54,	/* This discrepter's size (byte) */
0x03,	/* Disclipter type(0x03Fixed) */
/* String */
/* Sigma ACservo Basic Config */
/*   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
	'S', 0 ,'i', 0 ,'g', 0 ,'m', 0 ,'a', 0 ,' ', 0 ,'A', 0 ,'C', 0 ,
	's', 0 ,'e', 0 ,'r', 0 ,'v', 0 ,'o', 0 ,' ', 0 ,'B', 0 ,'a', 0 ,
	's', 0 ,'i', 0 ,'c', 0 ,' ', 0 ,'C', 0 ,'o', 0 ,'n', 0 ,'f', 0 ,
	'i', 0 ,'g', 0
};
/*--------------------------------------------------------------------------------------------------*/
/*		Interface																			*/
/*--------------------------------------------------------------------------------------------------*/
const UCHAR IinterfaceStringDescriptor[]=
{
24,		/* This discrepter's size(byte) */
0x03,	/* Disclipter type(0x03Fixed) */
/* String */
/* 2 Endpoints */
/*   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
	'2', 0 ,' ', 0 ,'E', 0 ,'n', 0 ,'d', 0 ,'p', 0 ,'o', 0 ,'i', 0 ,
	'n', 0 ,'t', 0 ,'s', 0
};

/*--------------------------------------------------------------------------------------------------*/
/*		USBFunction interrupt processing registration table													*/
/*--------------------------------------------------------------------------------------------------*/
#if 0 /*<S146> Interrupt setting is no longer needed and deleted */
/*	FIXME:Interrupt level settingsINTC_PRL_LVL7Is that okay? */
const tINTC_REGIST	intc_regist_table_u2f[2] =
{
	{INTC_IDX_U2F_INT,		UsbFuncIntIntu2f,		INTC_LEVEL_HI,	INTC_PRL_LVL7, 0},	/* INTU2F For interrupt processing	*/
	{INTC_IDX_U2F_EPC_INT,	UsbFuncIntIntu2fepc,	INTC_LEVEL_HI,	INTC_PRL_LVL7, 0}	/* INTU2FEPC Interrupt processing	*/
};
#endif

/*===========================================================================*/
/* PROTOTYPE																 */
/*===========================================================================*/
/* APIs	*/

/* Locals	*/
static	ER_RET	VendorRequest_Callback( DEVICE_REQUEST* pDeviceRequest );	/* Vendor request callback processing	*/
static	void	UsbFuncInitializeInfo( U2F_INFO *pUsbfInfo );				/* USBFunction information initialization processing		*/
static	void	UsbFuncClearFlag( USB_FLAG *pUsbfFlag );					/* Clear flag processing					*/
static	void	UsbFuncSetMaxpacket ( void );								/* Max Packet Size Setting processing				*/
static	USB_FUNCTION_IF*	UsbFuncGetInfoAddress( void );					/* USBObtaining address of information area		*/
static	UINT	UsbFuncCheckPlugInout( void );								/* USBConnection status confirmation process					*/
static	UINT	UsbFuncCheckSetupReceive( void );							/* SETUPToken reception confirmation processing			*/

/* EPnTransfer related	*/
static	void	UsbFuncSetStallEPn( UINT num );								/* EPn Stall response processing					*/
static	void	UsbFuncOutDataSplitCopy( USB_FUNCTION_IF *pUsbfIf );		/* OUTData division copy processing					*/ /*<S182>*/
static	void	UsbFuncInDataSplitCopy( USB_FUNCTION_IF *pUsbfIf );			/* INData division copy processing					*/ /*<S182>*/

/* Control transfer related	*/
static	INT		UsbFuncWriteEP0( VUCHAR* pBuf, INT size );					/* EP0 Send processing							*/
static	void	UsbFuncSetStallEP0( void );									/* EP0 Stall response processing					*/
#if 0	/* Currently not used <S150>*/
static	void	UsbFuncSetNakEP0( void );									/* EP0 NAKResponse processing						*/
static	void	UsbFuncClearNakEP0( void );									/* EP0 NAKCancellation process						*/
static	ER_RET	UsbFuncSendNullEP0( void );									/* EP0 NULLSend processing						*/
#endif

/*-------------------------------------*/
/* Request type processing			   */
/*-------------------------------------*/
static	ER_RET	control_nodata( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );	/* EP0 No DataProcessing			*/
static	ER_RET	control_read( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );	/* EP0 Control ReadProcessing		*/
static	ER_RET	control_write( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );	/* EP0 Control WriteProcessing	*/

/*-------------------------------------*/
/* Standard request processing		   */
/*-------------------------------------*/
static	ER_RET	decode_request( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );					/* EP0 Request decode processing */

static	ER_RET	bmRequestType_standard(  DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );			/* EP0 Standard request processing */
/* --- CLEAR_FEATURE     ( 1) [No data] --- */
static	ER_RET	std_req_clear_feature( DEVICE_REQUEST* pDeviceRequest );								/* EP0 CLEAR_FEATUREProcessing */
static	ER_RET	Clear_Feature_EP( UINT num );															/* EP0 CLEAR_FEATUREProcessing( TARGET = EP ) */
/* --- GET_CONFIGURATION ( 8) [Read]    --- */
static	ER_RET	std_req_get_configuration( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );		/* EP0 GET_CONFIGURATIONProcessing */
/* --- GET_DESCRIPTOR    ( 6) [Read]    --- */
static	ER_RET	std_req_get_descriptor( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );			/* EP0 GET_DESCRIPTORProcessing */
static	ER_RET	Get_Descriptor_Device( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );			/* EP0 GET_DESCRIPTOR DEVICEProcessing */
static	ER_RET	Get_Descriptor_Configuration( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );	/* EP0 GET_DESCRIPTOR CONFIGURATIONProcessing */
static	ER_RET	Get_Descriptor_String( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );			/* EP0 GET_DESCRIPTOR STRINGProcessing */
static	ER_RET	Get_Descriptor_Qualifier( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );		/* EP0 GET_DESCRIPTOR DEVICE QUALIFIERProcessing */
static	ER_RET	Get_Descriptor_OtherSpeed( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );		/* EP0 GET_DESCRIPTOR OTHER_SPEED_CONFIGURATIONProcessing */
/* --- GET_INTERFACE     (10) [Read]    --- */
static	ER_RET	std_req_get_interface( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );			/* EP0 GET_INTERFACEProcessing */
/* --- GET_STATUS        ( 0) [Read]    --- */
static	ER_RET	std_req_get_status( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );				/* EP0 GET_STATUSProcessing */
static	ER_RET	Get_Status_Device( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );				/* EP0 GET_STATUS DEVICEProcessing */
static	ER_RET	Get_Status_EP( DEVICE_REQUEST* pDeviceRequest );										/* EP0 GET_STATUS ENDPOINTProcessing */
/* --- SET_ADDRESS       ( 5) [No data] --- */
static	ER_RET	std_req_set_address( DEVICE_REQUEST* pDeviceRequest );									/* EP0 SET_ADDRESSProcessing */
/* --- SET_CONFIGURATION ( 9) [No data] --- */
static	ER_RET	std_req_set_configuration( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );		/* EP0 SET_CONFIGURATIONProcessing */
/* --- SET_DESCRIPTOR    ( 7) [Write]   --- */
static	ER_RET	std_req_set_descriptor( DEVICE_REQUEST* pDeviceRequest );								/* EP0 SET_DESCRIPTORProcessing */
/* --- SET_FEATURE       ( 3) [No data] --- */
static	ER_RET	std_req_set_feature( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );				/* EP0 SET_FEATUREProcessing */
static	ER_RET	Set_Feature_Device( UINT16 selector, UINT16 wIndex, U2F_INFO* pUsbfInfo );				/* EP0 SET_FEATUREProcessing( TARGET = DEVICE ) */
static	ER_RET	Set_Feature_EP( UINT num );																/* EP0 SET_FEATUREProcessing( TARGET = EP ) */
/* --- SET_INTERFACE     (11) [No data] --- */
static	ER_RET	std_req_set_interface( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo );			/* EP0 SET_INTERFACEProcessing */
/* --- SYNC_FRAME        (12) [No data] --- */
static	ER_RET	std_req_synch_frame( DEVICE_REQUEST* pDeviceRequest );									/* EP0 SYNCH_FRAMEProcessing */

static	ER_RET	Check_USB_DIR_EPn( UINT num, UINT8 direction );											/* EPDirectionUSB_DIR_OUT/INConversion processing */
static	UINT32	udev_get_DeviceState( void );															/* Device state acquisition processing */
static	DESC_CONFIG*			info_get_CurrConfigDesc( U2F_INFO* pUsbfInfo );							/* info Current configuration descriptor acquisition process */
static	INTERFACE_DESCRIPTOR*	info_find_InterfaceDesc( UINT16 index, U2F_INFO* pUsbfInfo );			/* info Interface descriptor acquisition process */
static	INT						EPn_get_STALL( UINT num );												/* EPn STALLState acquisition processing */


/*===========================================================================*/
/* PROGRAM																	 */
/*===========================================================================*/

/***************************************************************************
  Description: USBDriver initialization processing
----------------------------------------------------------------------------
  Parameters:
	MyAddress;			Own station address
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
void	UsbFuncInitializeDriver( ULONG MyAddress )
{
	UINT32			state;					/* For state operation					*/
	USB_FUNCTION_IF	*pUsbfIf;				/* USBInformation area pointer			*/
	USB_FLAG		*pUsbfFlag;				/* USBFlag area pointer		*/
//	INT				i;						/* Loop counter				*//*<S0C7>*/
	ULONG			StartTime;				/* Timer work				*//*<S138>*/

	UsbFunctionAvailable = FALSE;									/* USBUnusable setting(Rev.ASupports board) *//*<S138>*/

	pUsbfIf = UsbFuncGetInfoAddress();								/* USBObtaining address of information area	*/

	MlibResetLongMemory( pUsbfIf, sizeof( USB_FUNCTION_IF )/4  );	/* Clear management memory				*/

	pUsbfIf->UsbfInfo.MyAddress = MyAddress;						/* Own station address setting				*/

	pUsbfFlag = &(pUsbfIf->UsbfFlag);
	UsbFuncClearFlag( &(pUsbfIf->UsbfFlag) );						/* Clear flag					*/

	UsbFuncInitializeInfo( &(pUsbfIf->UsbfInfo) );					/* Class definition initialization				*/

	/*----------------------------------------------------------------------------------------------*/
	/*		EPC, PLL Reset release																	*/
	/*----------------------------------------------------------------------------------------------*/
	ARMPF_EPCTR &= ~(D_SYS_EPC_RST | D_SYS_PLL_RST);				/* Reset release */

	/*----------------------------------------------------------------------------------------------*/
	/*		System Bus-EPC Bridge Initial setting															*/
	/*----------------------------------------------------------------------------------------------*/
	ARMPF_SYSSCTR   = D_SYS_WAIT_MODE;								/* Slave wait operation specification	*/
#if 1	/* XXX:DMAEnable when using transfer */ /* <S026> Enabled due to initialization instruction */
	ARMPF_SYSMCTR   = 0x00000034;									/* DMAArbitration during transfer(bit 31): Round robin */
#endif
#if 0	/* <S150> VBUSDo not use interrupt(Polling) */
	ARMPF_SYSBINTEN = D_SYS_VBUS_INT;								/* VBUSInterrupt enabling						*/
#endif

	/*----------------------------------------------------------------------------------------------*/
	/*		EPC Initial setting																			*/
	/*----------------------------------------------------------------------------------------------*/
	/* --- PLL_LOCKWaiting --- */
	KlibRstLongTimer(&StartTime);		/* Start timekeeping *//*<S138>*/
	while ((ARMPF_EPCTR & D_SYS_PLL_LOCK) == 0)
	{
		/* !!!!!!!! ToDo PLL LOCKWaiting abnormality processing !!!!!!!! */
// <S138> >>>>>
		if( KlibGetLongTimerUs(StartTime) > 2000 )
		{
			return;
		}
		KlibWaitus(100);
// <<<<< <S138>
	}

	UsbFunctionAvailable = TRUE;							/* USBUsable setting(Rev.ASupports board) *//*<S138>*/

	/* --- Interrupt setting --- */
	/* USBControl register	*/
#if 1	/* XXX:High Speed Enable this when using */
	ARMPF_USB_CONTROL   = D_USB_INT_SEL | D_USB_SOF_RCV;			/* INTU2FEPCSet interrupt to level interrupt	*/
															/* SOF Use automatic recovery function when reception error occurs	*/
#else
	ARMPF_USB_CONTROL   = D_USB_INT_SEL | D_USB_SOF_RCV | D_USB_CONSTFS;	/* Above + FULL SpeedFixed	*/
#endif
	/* USBInterrupt enabling register	*/
#if 0	/* <S150> Do not use interrupt, change to polling */
	ARMPF_USB_INT_ENA   = D_USB_USB_RST_EN    |					/* Interrupt enable: Bus reset			*/
					D_USB_SPEED_MODE_EN |					/*             : Bus speed change		*/
					D_USB_EP0_EN        |					/*             :EP0 Interrupt			*/
					D_USB_EP1_EN        |					/*             :EP1 Interrupt			*/
					D_USB_EP2_EN;							/*             :EP2 Interrupt			*/
#endif

	/* --- Max PacketSetting --- */
	UsbFuncSetMaxpacket();

	/* --- EachEP Buffer clear --- */
	/* EPnControl register	*/
	ARMPF_EP0_CONTROL   = D_EP0_BCLR;								/* EP0 : Buffer clear				*/
	ARMPF_EP1_CONTROL   = D_EPn_BCLR;								/* EP1 : Bulk-in,  Buffer clear		*/
	ARMPF_EP2_CONTROL   = D_EPn_BCLR | D_EPn_DIR0;				/* EP2 : Bulk-out, Buffer clear		*/

	/* --- EachEP Interrupt enabling --- */
	/* EPnInterrupt enabling register	*/
#if 0	/* <S150> Do not use interrupt, change to polling	*/
	ARMPF_EP0_INT_ENA   = D_EP0_SETUP_EN;							/* EP0 : Interrupt enable:SETUP_INT					*/
#endif
#if 0	/* XXX:Enable when using transfer completion interrupt (items to enable must be reconsidered) */
	ARMPF_EP1_INT_ENA   = D_EPn_IN_EN | D_EPn_IN_END_EN;			/* EP1 : Interrupt enable:EP1_IN_INT, EP1_IN_END_INT	*/
															/*	D_EPn_IN_EN		: INTransfer completion interrupt enable		*/
															/*	D_EPn_IN_END_EN	: IN DMATransfer completion interrupt enable	*/
	ARMPF_EP2_INT_ENA   = D_EPn_OUT_EN | D_EPn_OUT_END_EN;		/* EP2 : Interrupt enable:EP2_OUT_INT, EP2_OUT_END_INT	*/
															/*	D_EPn_OUT_EN	: OUTTransfer completion interrupt enable		*/
															/*	D_EPn_OUT_END_EN: OUT DMATransfer completion interrupt enable	*/
#endif

	/* --- EachEP Enabling --- */
	ARMPF_EP1_CONTROL  |= D_EPn_EN;								/* EP1 : Enabling		*/
	ARMPF_EP2_CONTROL  |= D_EPn_EN;								/* EP2 : Enabling		*/

	/*=========================================*/
	/* Plugin/out processing (Startup status confirmation) */
	/*=========================================*/
	if ((ARMPF_EPCTR & D_SYS_VBUS_LEVEL) == D_SYS_VBUS_LEVEL)		/* VBUSLevel confirmation	*/
	{
		/* ====== Plugin processing ======	*/
		state       =  ARMPF_USB_CONTROL;							/* USBGet control register value						*/
		state      &= ~D_USB_CONNECTB;						/* (Function PHY USB Signal Enable)	*/
		state      |=  D_USB_PUE2;							/* (D+SignalPull-up ) Increase by						*/
		ARMPF_USB_CONTROL =  state;								/* Reflect the above operation at the same time							*/
		pUsbfFlag->UsbPlugStatus = U2F_PLUG_IN;				/* Update plug status to plug-in					*/
	}
	else
	{
		/* ====== Plugout processing ====== */
		state       =  ARMPF_USB_CONTROL;							/* USBGet control register value						*/
		state      |=  D_USB_CONNECTB;						/* (Function PHY USB Disable the signal)	*/
		state      &= ~D_USB_PUE2;							/* (D+SignalPull-up Do not do this)					*/
		ARMPF_USB_CONTROL =  state;								/* Reflect the above operation at the same time							*/
		pUsbfFlag->UsbPlugStatus = U2F_PLUG_OUT;			/* Update plug status to plug-out				*/
	}

#if 0	/* Interrupt.c, it is set all at once, so if it is neither	*/
	/*=========================================*/
	/* Interrupt setting							   */
	/*=========================================*/
	for( i = 0; i < (INT)(sizeof(intc_regist_table_u2f)/sizeof(tINTC_REGIST)); i++ )
	{
		intc_set_handler((tINTC_REGIST*)&intc_regist_table_u2f[i]);
		intc_enable_interrupt(intc_regist_table_u2f[i].index);
	}
#endif

		return;
}

//<S138> >>>>>
/***************************************************************************
  Description: USBUsable judgment processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	TRUE;			Usable
	FALSE;			Use prohibited
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
BOOL	UsbFuncChkAvailable( void )
{
	return UsbFunctionAvailable;
}
//<<<<< <S138>

/***************************************************************************
	APIs	Interrupt related
****************************************************************************/

/***************************************************************************
  Description: INTU2F Interrupt processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
void UsbFuncIntIntu2f( void )
{
	UINT32			state;					/* For state operation				*/
	USB_FUNCTION_IF	*pUsbfIf;				/* USBInformation area pointer		*/
	USB_FLAG		*pUsbfFlag;				/* USBFlag area pointer	*/

//<S138> >>>>>
	/* Rev.ASupports board */
	if( UsbFuncChkAvailable() == FALSE )
	{
		return;
	}
//<<<<< <S138>

	pUsbfIf = UsbFuncGetInfoAddress();		/* USBObtaining address of information area	*/
	pUsbfFlag = &(pUsbfIf->UsbfFlag);

	/*=========================================*/
	/* Plugin/out processing (VBUS)		   */
	/*=========================================*/
	if ((ARMPF_SYSBINT & D_SYS_VBUS_INT) == D_SYS_VBUS_INT)
	{
		ARMPF_SYSBINT |= D_SYS_VBUS_INT;								/* Clear interrupt source	*/
		if ((ARMPF_EPCTR & D_SYS_VBUS_LEVEL) == D_SYS_VBUS_LEVEL)		/* VBUSLevel confirmation		*/
		{
			/* ====== Plugin processing ====== */
			state       =  ARMPF_USB_CONTROL;							/* USBGet control register value						*/
			state      &= ~D_USB_CONNECTB;						/* (Function PHY USB Signal Enable)	*/
			state      |=  D_USB_PUE2;							/* (D+SignalPull-up ) Increase by						*/
			ARMPF_USB_CONTROL =  state;								/* Reflect the above operation at the same time							*/
			pUsbfFlag->UsbPlugStatus = U2F_PLUG_IN;				/* Update plug status to plug-in					*/
		}
		else
		{
			/* ====== Plugout processing ====== */
			state       =  ARMPF_USB_CONTROL;							/* USBGet control register value						*/
			state      |=  D_USB_CONNECTB;						/* (Function PHY USB Disable the signal)	*/
			state      &= ~D_USB_PUE2;							/* (D+SignalPull-up Do not do this)					*/
			state      &= ~D_USB_DEFAULT;						/* (endpoint0)				*/
			state      &= ~D_USB_CONF;							/* (endpoint0)			*/
			ARMPF_USB_CONTROL =  state;								/* Reflect the above operation at the same time							*/
			pUsbfFlag->UsbPlugStatus = U2F_PLUG_OUT;			/* Update plug status to plug-out				*/

			UsbFuncClearFlag( pUsbfFlag );						/* Clear flag								*/
		}
	}

#if 0	/* XXX:Existed as a comment in the sample source */
	intc_clear_eiint( IDX_INTU2F );								/* INTU2F Cause clear	*/
#endif
}

/***************************************************************************
  Description: INTU2FEPC Interrupt processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
void UsbFuncIntIntu2fepc ( void )
{
	USB_FUNCTION_IF	*pUsbfIf;				/* USBInformation area pointer		*/
	USB_FLAG		*pUsbfFlag;				/* USBFlag area pointer	*/

//<S138> >>>>>
	/* Rev.ASupports board */
	if( UsbFuncChkAvailable() == FALSE )
	{
		return;
	}
//<<<<< <S138>

	pUsbfIf = UsbFuncGetInfoAddress();		/* USBObtaining address of information area	*/
	pUsbfFlag = &(pUsbfIf->UsbfFlag);

	/*=========================================*/
	/* Bus reset Processing					   */
	/*=========================================*/
	if ((ARMPF_USB_INT_STA & D_USB_USB_RST_INT) == D_USB_USB_RST_INT)
	{
		ARMPF_USB_INT_STA = ~D_USB_USB_RST_INT;						/* Clear interrupt source		*/
		/* --- Max PacketSetting --- */
		UsbFuncSetMaxpacket();
		/* --- EP Buffer clear --- */
		ARMPF_EP0_CONTROL   |= D_EP0_BCLR;							/* EP0 : Buffer clear	*/
		ARMPF_EP1_CONTROL   |= D_EPn_BCLR;							/* EP1 : Buffer clear	*/
		ARMPF_EP2_CONTROL   |= D_EPn_BCLR;							/* EP2 : Buffer clear	*/
		/* --- Clear flag --- */
		UsbFuncClearFlag( pUsbfFlag );							/* Clear flag			*/
	}

	/*=========================================*/
	/* USBChange bus speed Processing			   */
	/*=========================================*/
	if ((ARMPF_USB_INT_STA & D_USB_SPEED_MODE_INT) == D_USB_SPEED_MODE_INT)
	{
		ARMPF_USB_INT_STA = ~D_USB_SPEED_MODE_INT;					/* Clear interrupt source		*/
		/* --- Max PacketSetting --- */
		UsbFuncSetMaxpacket();
	}

	/*=========================================*/
	/* EP0 Processing								   */
	/*=========================================*/
	/* ---  SETUPData reception --- */
	if ((ARMPF_EP0_STATUS & D_EP0_SETUP_INT) == D_EP0_SETUP_INT)
	{
		ARMPF_EP0_STATUS = ~D_EP0_SETUP_INT;							/* Clear interrupt source		*/
		pUsbfFlag->UsbSetupReceive++;
	}
#if 0	/* BulkInterrupts are not used for data transfer */
	/*=========================================*/
	/* EP1 Processing								   */
	/*=========================================*/
	/* ---  Bulk-In Data transmission --- */
	if ((ARMPF_EP1_STATUS & D_EPn_IN_INT) == D_EPn_IN_INT) {
		ARMPF_EP1_STATUS = ~D_EPn_IN_INT;								/* Clear interrupt source		*/
		pUsbfFlag->UsbEp1InCount++;
	}
	/* ---  Bulk-In DMATransfer end --- */
	if ((ARMPF_EP1_STATUS & D_EPn_IN_END_INT) == D_EPn_IN_END_INT) {
		ARMPF_EP1_STATUS = ~D_EPn_IN_END_INT;							/* Clear interrupt source		*/
	}
	/*=========================================*/
	/* EP2 Processing								   */
	/*=========================================*/
	/* ---  Bulk-Out Data reception --- */
	if ((ARMPF_EP2_STATUS & D_EPn_OUT_INT) == D_EPn_OUT_INT) {
		ARMPF_EP2_STATUS = ~D_EPn_OUT_INT;							/* Clear interrupt source		*/
		pUsbfFlag->UsbEp2OutCount++;
	}
	/* ---  Bulk-Out DMATransfer end --- */
	if ((ARMPF_EP2_STATUS & D_EPn_OUT_END_INT) == D_EPn_OUT_END_INT) {
		ARMPF_EP2_STATUS = ~D_EPn_OUT_END_INT;						/* Clear interrupt source		*/
	}
#endif

#if 0	/* XXX:Existed as a comment in the sample source */
	intc_clear_eiint( IDX_INTU2FEPC );							/* INTU2FEPC Cause clear	*/
#endif
}


/***************************************************************************
	APIs	Control transfer related
****************************************************************************/

/***************************************************************************
  Description: Control transfer task processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
void UsbFuncTaskControl( void )
{
	ER_RET			result;					/* Result							*/
	USB_FUNCTION_IF	*pUsbfIf;				/* USBInformation area pointer			*/
	U2F_INFO		*pUsbfInfo;				/* USBDevice information area pointer	*/
	DEVICE_REQUEST	*pDeviceRequest;		/* Device request pointer	*/

//<S138> >>>>>
	/* USBIf unusable, terminate immediately(Rev.ASupports board) */
	if( UsbFuncChkAvailable() == FALSE )
	{
		return;
	}
//<<<<< <S138>

	/* USBObtaining information area address	*/
	pUsbfIf = UsbFuncGetInfoAddress();
	/* USBGet device information area address	*/
	pUsbfInfo = &(pUsbfIf->UsbfInfo);
	/* Device request address acquisition	*/
	pDeviceRequest = &(pUsbfIf->UsbfInfo.usb.request);

	/*=========================================*/
	/* EP0 Processing (SETUPData reception)			   */
	/*=========================================*/
	if (pUsbfIf->UsbfFlag.UsbSetupReceive != pUsbfIf->UsbfFlag.UsbSetupSave)
	{
		/* Save for next comparison */
		pUsbfIf->UsbfFlag.UsbSetupSave = pUsbfIf->UsbfFlag.UsbSetupReceive;
		/* ------ Device request Read ------ */
		pDeviceRequest->bmRequestType = (UINT8) ((ARMPF_SETUP_DATA0 >>  0) & 0xff  );
		pDeviceRequest->bRequest      = (UINT8) ((ARMPF_SETUP_DATA0 >>  8) & 0xff  );
		pDeviceRequest->wValue        = (UINT16)((ARMPF_SETUP_DATA0 >> 16) & 0xffff);
		pDeviceRequest->wIndex        = (UINT16)((ARMPF_SETUP_DATA1 >>  0) & 0xffff);
		pDeviceRequest->wLength       = (UINT16)((ARMPF_SETUP_DATA1 >> 16) & 0xffff);

		/* ------ Request type determination ------ */
		if( pDeviceRequest->wLength == 0x0000 )
		{	/* Number of bytes in data stage0		*/
			/*  -> STATUSGo to the process				*/
			result = control_nodata( pDeviceRequest, pUsbfInfo );
		}
		else
		{	/* Number of bytes in data stage0,	*/
			if( (pDeviceRequest->bmRequestType & USB_DIR_IN) == USB_DIR_IN )
			{	/* Data transfer direction is incorrect Host=>Dev , it remains as a power of	*/
				/*  -> DATA IN & STATUSGo to the process	*/
				result = control_read( pDeviceRequest, pUsbfInfo );
			}
			else
			{	/* Dev=>Host , it remains as a power of					*/
				/*  -> DATA OUT & STATUSGo to the process	*/
				result = control_write( pDeviceRequest, pUsbfInfo );
			}
		}
		/* ------ Error processing --- */
		if (result != ER_OK)
		{
			/* ToDo:If there is a common process for errors in control transfer, write it here */
		}
	}
}

/***************************************************************************
	APIs	Bulk transfer related
****************************************************************************/

/***************************************************************************
  Description: USB Bulk transfer control processing (ScanPolling processing for interrupt factors by processing)
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	[Function]
		Monitor each interrupt status at regular intervals and process when an interrupt factor occurs.
			Monitored interrupts: Check in the order listed
				(1) OUTData reception complete(ARMPF_EP2_STATUS & D_EPn_OUT_INT)
				(2) OUTDataNULLPacket reception complete(ARMPF_EP2_STATUS & D_EPn_OUT_NULL_INT)
				(3) INTokenNAKResponse(ARMPF_EP1_STATUS & D_EPn_IN_NAK_ERR_INT)
	[Note](SGDVare left unchanged for now)
		This function should be executed in a task equivalent to or higher than the communication manager.
		When executing in the same task as the communication manager, it is more efficient to execute it before the communication manager.
----------------------------------------------------------------------------
	[Controlled object]
		The endpoints controlled by this module are as follows.
			Bulk out: Endpoint2
			Bulk in : Endpoint1
	[/66] [67] There was one around(Reference)]
		Receive a transfer start command from the communication manager using the following function.
			Bulk out: UsbFuncStartBkout()
			Bulk in : UsbFuncStartBkin()
		At start commandCPUofUSBFCheck and set the driver status as below.
			Bulk out: Receive possible -> UsbfIf.Ep1Status = BLKTRNS_ENABLE
					  Receive not possible -> UsbfIf.Ep1Status = BLKTRNS_DISABLE
			Bulk in : Transmission possible -> UsbfIf.Ep2Status = BLKTRNS_ENABLE
					  Send disabled -> UsbfIf.Ep2Status = BLKTRNS_DISABLE

****************************************************************************/
/*<S182> Changed data copy to be divided into multiple scans (no history in the function because there are many changes)  */
void	UsbFuncPollBulkOutIn( void )
{
	UINT32			MaxPacketSize;				/* Maximum packet size			*/
	USB_FUNCTION_IF	*pUsbfIf;					/* USBInformation area pointer			*/

	pUsbfIf = UsbFuncGetInfoAddress();			/* USBObtaining address of information area	*/

	/*----------------------------------------------------------------------------------------------*/
	/*	Bulk transfer interrupt factor flag check														*/
	/*----------------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------------*/
	/*	Caution! !																					*/
	/*																								*/
	/*	: Execute this processScanC, self-clearing warnings such asOUTData is double-buffered double-sided	*/
	/*	  accumulates, the following changes are required! !						*/
	/*																								*/
	/*	1.EP2ReceivingINcomes,OUTReception ends after confirming that all data has been read	*/
	/*----------------------------------------------------------------------------------------------*/

	/*------------------------------------------------------------------------------------------*/
	/*	Detailed cause check(1-1)	: OUTData reception complete												*/
	/*------------------------------------------------------------------------------------------*/
	if( (pUsbfIf->OutSplitCopy != TRUE) && (pUsbfIf->InSplitCopy != TRUE) )
	if( (ARMPF_EP2_STATUS & D_EPn_OUT_INT) == D_EPn_OUT_INT )	/* OUTData reception complete			*/
	{
		ARMPF_EP2_STATUS &= ~D_EPn_OUT_INT;						/* Interrupt factor clear				*/
		if( pUsbfIf->Ep2Status == BLKTRNS_ENABLE )				/* Reception permission check				*/
		{
																/* Save reception length					*/
			pUsbfIf->OutPacketSize = ARMPF_EP2_LEN_DCNT & D_EPn_LDATA;
			pUsbfIf->OutCopyRem = pUsbfIf->OutPacketSize;		/* Save remaining copy size			*/
			/*------------------------------------------------------------------------------*/
			/*	Reception size over check													*/
			/*------------------------------------------------------------------------------*/
			if( (pUsbfIf->OutLength + pUsbfIf->OutPacketSize) > pUsbfIf->OutLengthReq )
			{
				/*--------------------------------------------------------------------------*/
				/*	When the current data is received, the data is discarded to overwhelm the size of the receive buffer and the process is completed	*/
				/*	Have error handling performed at the application level.					*/
				/*--------------------------------------------------------------------------*/
				ARMPF_EP2_CONTROL |= D_EPn_ONAK;				/* EP2ofNAKSet as response only		*/
				pUsbfIf->Ep2Status = BLKTRNS_STALL;				/* STALLSet that it has responded	*/
			}
			else
			{
				/*--------------------------------------------------------------------------*/
				/*	If not over, split reception											*/
				/*--------------------------------------------------------------------------*/
				UsbFuncOutDataSplitCopy( pUsbfIf );
			}
		}
	}
	/*------------------------------------------------------------------------------------------*/
	/*	Detailed cause check(1-2)	: OUTData division copying											*/
	/*------------------------------------------------------------------------------------------*/
	if( pUsbfIf->OutSplitCopy == TRUE )
	{
		if( pUsbfIf->Ep2Status == BLKTRNS_ENABLE )
		{
			/*--------------------------------------------------------------------------*/
			/*	Continuing division reception												*/
			/*--------------------------------------------------------------------------*/
			UsbFuncOutDataSplitCopy( pUsbfIf );
		}
	}

	/*--------------------------------------------------------------------------------------*/
	/*	Detailed cause check(2)	: OUTDataNULLPacket reception complete									*/
	/*		OUTWhen data is an integer multiple of the max packet size,								*/
	/*		Finally to recognize data endNULLwithout outputting [11],								*/
	/*		NULLand directly setNULLcomes, reception is complete							*/
	/*--------------------------------------------------------------------------------------*/
	if( pUsbfIf->OutSplitCopy != TRUE )
	if( (ARMPF_EP2_STATUS & D_EPn_OUT_NULL_INT) == D_EPn_OUT_NULL_INT )
	{
		ARMPF_EP2_STATUS &= ~D_EPn_OUT_NULL_INT;		/* BKO1NLInterrupt factor clear			*/
		if( pUsbfIf->Ep2Status == BLKTRNS_ENABLE )		/* Reception permitted?						*/
		{
			ARMPF_EP2_CONTROL |= D_EPn_ONAK;			/* EP2ofNAKSet as response only			*/
			pUsbfIf->Ep2Status = BLKTRNS_END;			/* Set reception completion					*/
		}
	}

	/*--------------------------------------------------------------------------------------*/
	/*	Detailed cause check(3-1)	: INTokenNAKResponse										*/
	/*		NormalOUTAfter data,INToken arrives												*/
	/*		HoweverOUTWhen data is an integer multiple of the max packet size, it is sent to the host					*/
	/*		ThusOUTAt the end of dataNULLSuddenly without giving outINToken				*/
	/*		Therefore,Endpoint1, onlyEndpoint2OUTWhile waiting for data reception	*/
	/*		INcomes,OUTData reception completedINPerform token reception processing in parallel					*/
	/*--------------------------------------------------------------------------------------*/
	if( (pUsbfIf->OutSplitCopy != TRUE) && (pUsbfIf->InSplitCopy != TRUE) )
	if( (ARMPF_EP1_STATUS & D_EPn_IN_NAK_ERR_INT) == D_EPn_IN_NAK_ERR_INT )
	{
		ARMPF_EP1_STATUS &= ~D_EPn_IN_NAK_ERR_INT;		/* Clear flag						*/
		if( pUsbfIf->Ep1Status == BLKTRNS_ENABLE )		/* Transmission permission check					*/
		{
			/*------------------------------------------------------------------------------*/
			/*	Send data write permission check												*/
			/*------------------------------------------------------------------------------*/
			if( (ARMPF_EP1_STATUS & D_EPn_IN_EMPTY) == D_EPn_IN_EMPTY )	/* EP1ofIN EMPTY, writeOK		*/
			{
				/* Automatic data transmission disabled */
				ARMPF_EP1_CONTROL &= ~D_EPn_AUTO;

				/* EP1maximum packet size acquisition */
				MaxPacketSize = ARMPF_EP1_PCKT_ADRS & D_EPn_MPKT;

				/* Set packet size for this time */
				if( pUsbfIf->InLengthRem > MaxPacketSize )
				{
					pUsbfIf->InCopyRem = MaxPacketSize;
				}
				else
				{
					pUsbfIf->InCopyRem = pUsbfIf->InLengthRem;
				}

				/*--------------------------------------------------------------------------*/
				/*	Start division transmission															*/
				/*--------------------------------------------------------------------------*/
				UsbFuncInDataSplitCopy( pUsbfIf );
			}
		}
		else if( pUsbfIf->Ep2Status == BLKTRNS_ENABLE )	/* EP2ReceivingINcomes, reception is complete	*/
		{
			ARMPF_EP2_CONTROL |= D_EPn_ONAK;			/* EP2ofNAKSet as response only			*/
			pUsbfIf->Ep2Status = BLKTRNS_END;			/* Set reception completion					*/
		}
	}

	/*------------------------------------------------------------------------------------------*/
	/*	Detailed cause check(3-2)	: INData division copying											*/
	/*------------------------------------------------------------------------------------------*/
	if( pUsbfIf->InSplitCopy == TRUE )
	{
		if( pUsbfIf->Ep1Status == BLKTRNS_ENABLE )		/* Transmission permission check					*/
		{
			/*--------------------------------------------------------------------------*/
			/*	Continuing division reception												*/
			/*--------------------------------------------------------------------------*/
			UsbFuncInDataSplitCopy( pUsbfIf );
		}
		else if( pUsbfIf->Ep2Status == BLKTRNS_ENABLE )	/* EP2ReceivingINcomes, reception is complete	*/
		{
			ARMPF_EP2_CONTROL |= D_EPn_ONAK;			/* EP2ofNAKSet as response only			*/
			pUsbfIf->Ep2Status = BLKTRNS_END;			/* Set reception completion					*/
		}
	}
	return;
}


/***************************************************************************
  Description: USB Endpoint1/2 Transmission/reception buffer clear processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
void	UsbFuncResetBuffer( void )
{
	USB_FUNCTION_IF	*pUsbfIf;						/* USBInformation area pointer			*/

	pUsbfIf = UsbFuncGetInfoAddress();				/* USBObtaining address of information area	*/

	ARMPF_EP2_CONTROL |= D_EPn_ONAK;						/* Endpoint2ofNAKonly response			*/
	pUsbfIf->Ep1Status = BLKTRNS_DISABLE;			/* EP1to transmission permission				*/
	pUsbfIf->Ep2Status = BLKTRNS_DISABLE;			/* EP2to transmission permission				*/
	/*------------------------------------------------------------------------------------------*/
	/*	Receiving(OUT) Clear the buffer																*/
	/*------------------------------------------------------------------------------------------*/
	ARMPF_EP2_CONTROL |= D_EPn_BCLR;
	/*------------------------------------------------------------------------------------------*/
	/*	Transmission(IN) Clear the buffer																*/
	/*------------------------------------------------------------------------------------------*/
	ARMPF_EP1_CONTROL |= D_EPn_BCLR;

	return;
}

/***************************************************************************
	APIs	USB MEMOBUSServer processing related
****************************************************************************/

/***************************************************************************
  Description: USB Endpoint2 Start reception(BLUK OUT)Processing
----------------------------------------------------------------------------
  Parameters:
	Buffer;			Receive data storage start address
	OutLength;		Receive data storage buffer size
  Return:
	TRUE;			Reception permission
	FALSE;			Reception prohibited
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
ULONG	UsbFuncStartBkout( UCHAR *Buffer ,USHORT OutLength )
{
	ULONG			rc;								/* Return value						*/
	USB_FUNCTION_IF	*pUsbfIf;						/* USBInformation area pointer			*/

	rc = FALSE;										/* Initial value : FALSE				*/

	pUsbfIf = UsbFuncGetInfoAddress();				/* USBObtaining address of information area	*/

	/*----------------------------------------------------------------------------------------------*/
	/*	EP2Check availability :: Interface isConfigutationCan be used if already completed			*/
	/*----------------------------------------------------------------------------------------------*/
	if( (ARMPF_USB_STATUS & D_USB_CONF) == D_USB_CONF )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	EP2Processing when available																		*/
		/*------------------------------------------------------------------------------------------*/
		pUsbfIf->OutLengthReq = OutLength;			/* Save allowed reception length size					*/
		pUsbfIf->OutLength = 0;						/* Initialize received byte count					*/
		pUsbfIf->OutBufPtr = Buffer;				/* Receive data storage pointer			*/
		/*------------------------------------------------------------------------------------------*/
		/*	EP2Clear the buffer																		*/
		/*------------------------------------------------------------------------------------------*/
		ARMPF_EP2_CONTROL |= D_EPn_BCLR;
		/*------------------------------------------------------------------------------------------*/
		/*	Cancel division copy flag	<S182>														*/
		/*------------------------------------------------------------------------------------------*/
		pUsbfIf->OutSplitCopy = FALSE;
		/*------------------------------------------------------------------------------------------*/
		/*	Set status to ready to receive														*/
		/*------------------------------------------------------------------------------------------*/
		pUsbfIf->Ep2Status = BLKTRNS_ENABLE;		/* EP2to reception permission				*/
		ARMPF_EP2_CONTROL &= ~D_EPn_ONAK;			/* EP2ofNAKCancel response						*/

		rc = TRUE;
	}
	else
	{
		/*------------------------------------------------------------------------------------------*/
		/*	EP2If unusable, disable use and return											*/
		/*------------------------------------------------------------------------------------------*/
		pUsbfIf->Ep2Status = BLKTRNS_DISABLE;		/* EP2to reception prohibited				*/
		rc = FALSE;
	}
	return( rc );
}

/***************************************************************************
  Description: USB Endpoint1 Transmission start(BLUK IN)Processing
----------------------------------------------------------------------------
  Parameters:
	Buffer;			Start address of send data storage destination
	InLength;		Send data size
  Return:
	TRUE;			Transmission permission
	FALSE;			Transmission prohibited
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
ULONG	UsbFuncStartBkin( UCHAR *Buffer ,USHORT InLength )
{
	ULONG			rc;								/* Return value						*/
	USB_FUNCTION_IF	*pUsbfIf;						/* USBInformation area pointer			*/

	rc = FALSE;										/* Initial value : FALSE				*/

	pUsbfIf = UsbFuncGetInfoAddress();				/* USBObtaining address of information area	*/

	/*----------------------------------------------------------------------------------------------*/
	/*	EP1Check availability :: Interface isConfigutationCan be used if already completed			*/
	/*----------------------------------------------------------------------------------------------*/
	if( (ARMPF_USB_STATUS & D_USB_CONF) == D_USB_CONF )
	{
		/*------------------------------------------------------------------------------------------*/
		/*	EP1Processing when available																		*/
		/*------------------------------------------------------------------------------------------*/
		pUsbfIf->InLengthRem = InLength;			/* Initialize remaining transmission data size				*/
		pUsbfIf->InLength = 0;						/* Initialize number of transmitted bytes					*/
		pUsbfIf->InBufPtr = Buffer;					/* Receive data storage destination					*/
		/*------------------------------------------------------------------------------------------*/
		/*	EP1Clear the buffer																		*/
		/*------------------------------------------------------------------------------------------*/
		ARMPF_EP1_CONTROL |= D_EPn_BCLR;
		/*------------------------------------------------------------------------------------------*/
		/*	Cancel division copy flag	<S182>														*/
		/*------------------------------------------------------------------------------------------*/
		pUsbfIf->InSplitCopy = FALSE;
		/*------------------------------------------------------------------------------------------*/
		/*	Set status to ready to send														*/
		/*------------------------------------------------------------------------------------------*/
		ARMPF_EP2_STATUS &= ~D_EPn_OUT_NAK_ERR_INT;		/* EP2 OUTToken reception status clear			*/
		pUsbfIf->Ep1Status = BLKTRNS_ENABLE;		/* EP1to transmission permission				*/

		rc = TRUE;
	}
	else
	{
		/*------------------------------------------------------------------------------------------*/
		/*	EP1UnusableDMATurn without setting													*/
		/*------------------------------------------------------------------------------------------*/
		pUsbfIf->Ep1Status = BLKTRNS_DISABLE;		/* EP1to transmission permission				*/
		rc = FALSE;
	}
	return( rc );
}

/***************************************************************************
  Description: USBPort status notification service processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	TRUE;			Application layer communication possible
	FALSE;			Application layer communication not possible
----------------------------------------------------------------------------
  Note:
	Reply whether application layer communication is possible.
	.USBandSET_CONFIGURATION Val = 1 ofHostbutDeviceWhen you commandEndpoint0(This driver
	  In Iba,1and2)becomes available. Note that even ifEP1andEP2use.
****************************************************************************/
ULONG	UsbFuncPortConfiguration( void )
{
	ULONG	rc;								/* Self-function return value				*/
	UINT32	ret;							/* Other function return value				*/

	rc = FALSE;								/* Initial value : FALSE			*/

	/* Device state acquisition	*/
	ret = udev_get_DeviceState();
	if( ret == UDEV_ST_CONFIGURED)
	{	/* ConfiguredState has ended	*/
		rc = TRUE;
	}

	return( rc );
}

/***************************************************************************
  Description: USB Endpoint2 Receive byte count acquisition process
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	Receive data byte size
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
USHORT	UsbFuncGetRcvSize( void )
{
	USB_FUNCTION_IF	*pUsbfIf;				/* USBInformation area pointer		*/

	pUsbfIf = UsbFuncGetInfoAddress();		/* USBObtaining information area address	*/
	
	return( pUsbfIf->OutLength );			/* Receive data byte size	*/
}

/***************************************************************************
  Description: Bulk transfer(OUT)Status notification service processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	Bulk transfer(OUT)Status
	BLKTRNS_DISABLE;		Bulk transfer disabled
	BLKTRNS_ENABLE;			Bulk transfer enabled
	BLKTRNS_END;			Bulk transfer completed
	BLKTRNS_STALL;			An error occurred during bulk transferSTALLdid
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
ULONG	UsbFuncChkRcvComp( void )
{
	USB_FUNCTION_IF	*pUsbfIf;				/* USBInformation area pointer		*/

	pUsbfIf = UsbFuncGetInfoAddress();		/* USBObtaining information area address	*/

	return( (ULONG)(pUsbfIf->Ep2Status) );	/* Bulk transfer(OUT)Status		*/
}

/***************************************************************************
  Description: Bulk transfer(IN)Status notification service processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	Bulk transfer(IN)Status
	BLKTRNS_ENABLE;			Bulk transfer enabled
	BLKTRNS_END;			Bulk transfer completed
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
ULONG	UsbFuncChkTrnsComp( void )
{
	ULONG			rc;						/* Return value					*/
	USB_FUNCTION_IF	*pUsbfIf;				/* USBInformation area pointer		*/

	rc = BLKTRNS_ENABLE;					/* Initial value : BLKTRNS_ENABLE	*/

	pUsbfIf = UsbFuncGetInfoAddress();		/* USBObtaining information area address	*/

	/* Transmission is complete when the transmission buffer is empty*/
	if( (pUsbfIf->Ep1Status == BLKTRNS_END) 
		&& ((ARMPF_EP1_STATUS & D_EPn_IN_EMPTY) == D_EPn_IN_EMPTY) )
	{
		rc = BLKTRNS_END;
	}
	/* While waiting for transmission completion,OUTForce termination if token is received */
	if( (ARMPF_EP2_STATUS & D_EPn_OUT_NAK_ERR_INT) == D_EPn_OUT_NAK_ERR_INT )
	{
		rc = BLKTRNS_END;
	}
	return( rc );
}


/***************************************************************************
	Locals
****************************************************************************/

/***************************************************************************
  Description:Vendor request callback processing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;			Device request
  Return:
	ER_OK;					Success
	Others;					Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET VendorRequest_Callback( DEVICE_REQUEST* pDeviceRequest )
{
	UINT32			data;						/* Max Logical Unit Number	*/
	ER_RET			result;						/* Result						*/
	USB_FUNCTION_IF	*pUsbfIf;					/* USBInformation area pointer		*/

	/* Return value initialization			*/
	result = ER_SYS;

	/* USBObtaining information area address	*/
	pUsbfIf = UsbFuncGetInfoAddress();

	/* ConfiguredState end confirmation	*/
	if (pUsbfIf->UsbfInfo.usb.curr_config == 0) {
		result = ER_SYS;
		return result;
	}

	/* Request identification	*/
	switch( pDeviceRequest->bRequest )
	{
		/* Get own address	*/
		case USB_VENDER_GET_OWN_ADDRESS:
			/* Parameter check		*/
			if ( ( ( pDeviceRequest->bmRequestType != 0xc0   ) &&
				   ( pDeviceRequest->bmRequestType != 0xc3   ) ) ||
			     ( pDeviceRequest->wValue        != 0x0000     ) ||
			     ( pDeviceRequest->wIndex        != 0x0000     ) ||
			     ( ( pDeviceRequest->wLength       != 0x0001 ) &&		/* <S072> tanaka21 */
			      ( pDeviceRequest->wLength       != 0x0004 ) &&
			       ( pDeviceRequest->wLength       != 0x0002 ) )
			   )
			{
				result = ER_SYS;
				break;
			}

			/* Own station address transmission	*/
			data = 0;
			data |= pUsbfIf->UsbfInfo.MyAddress;
			UsbFuncWriteEP0( (UCHAR*)&data, pDeviceRequest->wLength );

			result = ER_OK;
			break;

		/* Undefined request processing	*/
		default:
			result = ER_SYS;
			break;
	}
	return result;
}

/***************************************************************************
  Description: USBFunction information initialization processing
----------------------------------------------------------------------------
  Parameters:
	pUsbfInfo;				USBPointer to device information area
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void	UsbFuncInitializeInfo( U2F_INFO *pUsbfInfo )
{
	/* DescriptorSetting	*/
	pUsbfInfo->desc.pDevice							= (DEVICE_DESCRIPTOR*)&UsbFunctionDeviceDescriptor;
	pUsbfInfo->desc.pQualifier						= (DEVICE_QUALIFIER_DESCRIPTOR*)&UsbFunctionQualifierDescriptor;
	pUsbfInfo->desc.pConf_fs						= (DESC_CONFIG*)&UsbFunctionConfigurationDescriptorFS;
	pUsbfInfo->desc.pConf_hs						= (DESC_CONFIG*)&UsbFunctionConfigurationDescriptorHS;
	pUsbfInfo->desc.pString[STRIDX_LANG]			= (STRING_DESCRIPTOR*)LanguageIdStringDescriptor;
	pUsbfInfo->desc.pString[STRIDX_MANUFACTURER]	= (STRING_DESCRIPTOR*)ImanufacturerStringDescriptor;
	pUsbfInfo->desc.pString[STRIDX_PRODUCT]			= (STRING_DESCRIPTOR*)IproductStringDescriptor;
	pUsbfInfo->desc.pString[STRIDX_SERIALNUMBER]	= (STRING_DESCRIPTOR*)IserialNumberStringDescriptor;
	pUsbfInfo->desc.pString[STRIDX_CONFIGURATION]	= (STRING_DESCRIPTOR*)IconfigurationStringDescriptor;
	pUsbfInfo->desc.pString[STRIDX_INTERFACE]		= (STRING_DESCRIPTOR*)IinterfaceStringDescriptor;
	pUsbfInfo->dev.ep_num							= U2F_EPn_MAX;				/* UsingEPNumber									*/
#if 0	/* XXX:Commented out in the sample source */
	pUsbfInfo->dev.ep_dir							= DIR_OUT_EP2;				/* EPnDirection(OUT:1,IN:0) Bit0=EP1 - Bit14=EP15	*/
#endif
	pUsbfInfo->cb_class_req							= 0;						/* Class RequestCallback function(None)			*/
	pUsbfInfo->cb_vendor_req						= VendorRequest_Callback;	/* Vendor RequestCallback function				*/

	return;
}

/***************************************************************************
  Description: Clear flag processing
----------------------------------------------------------------------------
  Parameters:
	pUsbfFlag;				USBPointer to flag
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void UsbFuncClearFlag( USB_FLAG *pUsbfFlag )
{
	pUsbfFlag->UsbSetupReceive = 0;						/* SETUPToken reception count	*/
	pUsbfFlag->UsbSetupSave = 0;						/* The above saved value				*/
}

/***************************************************************************
  Description: Max Packet Size Setting processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void UsbFuncSetMaxpacket ( void )
{
	UINT32	bulk_size;								/* For bulk transfer size	*/
	UINT32	interrupt_size;							/* For interrupt transfer size	*/
 
	/*---------------------------------*/
	/* USBCheck bus speed		   */
	/*---------------------------------*/
	if ( (ARMPF_USB_STATUS & D_USB_SPEED_MODE) == D_USB_SPEED_MODE )
	{
		/* High Speed	*/
		bulk_size      = BULK_MAXPACKET_SIZE_HS;
		interrupt_size = INT_MAXPACKET_SIZE_HS;
	}
	else
	{
		/* Full Speed	*/
		bulk_size      = BULK_MAXPACKET_SIZE_FS;
		interrupt_size = INT_MAXPACKET_SIZE_FS;
	}

	/*---------------------------------*/
	/* Max Packet Size Setting			   */
	/*---------------------------------*/
														/* EPn : BaseAdr  MaxPkt(HS/FS This driver sets the standard maximum value.)	*/
														/* EP0 : 0x0000        64 byte x 2Side								*/
	ARMPF_EP1_PCKT_ADRS = 0x00200000 | bulk_size;				/* EP1 : 0x0020    512/64 byte x 2Side	--   Bulk    Transfer --		*/
	ARMPF_EP2_PCKT_ADRS = 0x01200000 | bulk_size;				/* EP2 : 0x0120    512/64 byte x 2Side	--   Bulk    Transfer --		*/
	ARMPF_EP3_PCKT_ADRS = 0x02200000 | interrupt_size;		/* EP3 : 0x0220   1024/64 byte x 1Side	-- Interrupt Transfer --		*/
														/*  BaseAdrteeth1Word(4Byte)unit									*/
														/*		EP0teeth 64(byte) x 2 = 128(byte) = 64(word) = 0x0020 twist		*/
														/*		EP1ofBaseAdrteeth 0x0000+0x0020 = 0x0020 				*/
}

/***************************************************************************
  Description: USBObtaining address of information area
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	USBAddress of information area
----------------------------------------------------------------------------
  Note:
	USBObtain the address of information area using this function
****************************************************************************/
static USB_FUNCTION_IF*	UsbFuncGetInfoAddress( void )
{
	return &UsbfIf;
}

/***************************************************************************
  Description: Connection status confirmation process
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	U2F_PLUG_OUT;		USB Hostwhen not connected
	U2F_PLUG_IN;		USB Hostwhen connected
****************************************************************************/
static UINT UsbFuncCheckPlugInout( void )
{
	USB_FUNCTION_IF	*pUsbfIf;						/* USBInformation area pointer			*/

	pUsbfIf = UsbFuncGetInfoAddress();				/* USBObtaining address of information area	*/

	return pUsbfIf->UsbfFlag.UsbPlugStatus;			/* Return connection status				*/
}

/***************************************************************************
  Description: SETUPToken reception confirmation processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	0;		SETUPToken not received
	0;	SETUPToken reception
****************************************************************************/
static UINT UsbFuncCheckSetupReceive( void )
{
	USB_FUNCTION_IF	*pUsbfIf;						/* USBInformation area pointer			*/

	pUsbfIf = UsbFuncGetInfoAddress();				/* USBObtaining address of information area	*/

	/* SETUPReturn token reception status	*/
	return (pUsbfIf->UsbfFlag.UsbSetupReceive - pUsbfIf->UsbfFlag.UsbSetupSave);
}

/***************************************************************************
	Locals	EPnTransfer related
****************************************************************************/

/***************************************************************************
  Description: EPn Stall response processing
----------------------------------------------------------------------------
  Parameters:
	num;			EPNumber
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void UsbFuncSetStallEPn( UINT num )
{
	/* --- IN/OUT Judgment (Check by transfer direction of control register 0:IN, 1:OUT ) --- */
	if( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_DIR0) != D_EPn_DIR0 ) {
		/* --- BULK IN --- */
		/* INTokenSTALLResponse */
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) |= D_EPn_ISTL;
	} else {
		/* --- BULK OUT --- */
		/* OUT/PINGTokenSTALLResponse and enable this setting */
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) |= (D_EPn_OSTL_EN | D_EPn_OSTL);
	}
}


/* <S182> >>>>> */
/***************************************************************************
  Description: OUTData division copy processing
----------------------------------------------------------------------------
  Parameters:
	pUsbfIf;			USBInformation area pointer
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void UsbFuncOutDataSplitCopy( USB_FUNCTION_IF *pUsbfIf )
{
	INT				i;							/* Loop counter				*/
	USHORT			strind;						/* Processing position in the transmission/reception buffer		*/
	INT				pktlen;						/* Receive packet byte size		*/
	UINT32			TmpWordData;				/* Word data temporary		*/
	UINT32			MaxPacketSize;				/* Maximum packet size			*/
	UINT32			CopySize;					/* Copy size					*/

	CopySize = SCAN_COPY_SIZE;

	strind = pUsbfIf->OutLength;				/* Start of the data received this timeindex		*/
												/* Set reception length to be processed this time		*/
	if( pUsbfIf->OutCopyRem >= CopySize )
	{
		pktlen = CopySize;
		pUsbfIf->OutCopyRem -= CopySize;
	}
	else
	{
		pktlen = pUsbfIf->OutCopyRem;
		pUsbfIf->OutCopyRem = 0;
	}
	pUsbfIf->OutLength += (USHORT)pktlen;		/* End of the data received this timeindex	*/

	/*--------------------------------------------------------------------------*/
	/*	Receive																*/
	/*--------------------------------------------------------------------------*/
	while( strind < pUsbfIf->OutLength )
	{
		TmpWordData = ARMPF_EP2_READ;			/* 4Get byte data			*/
		for( i = 0; i < 4; i++ )
		{
			/* 1Byte by byte4Bytes are stored in the buffer. */
			pUsbfIf->OutBufPtr[strind] = (UINT8)(TmpWordData >> (8*i));
			strind++;
			/* Fractional number(1~3)Check byte	*/
			if( strind >= pUsbfIf->OutLength )
			{
				break;
			}
		}
	}
	/*--------------------------------------------------------------------------*/
	/*	Reception completion judgment															*/
	/*--------------------------------------------------------------------------*/
	/* Received amount this time(Up to max packet size)is copied, complete */
	if( pUsbfIf->OutCopyRem == 0 )
	{
		/* EP2 */
		MaxPacketSize = ARMPF_EP2_PCKT_ADRS & D_EPn_MPKT;

		if( pUsbfIf->OutPacketSize < MaxPacketSize )	/* If the current packet is less than the maximum size, the process is completed	*/
		{
			ARMPF_EP2_CONTROL |= D_EPn_ONAK;		/* EP2ofNAKSet as response only			*/
			pUsbfIf->Ep2Status = BLKTRNS_END;		/* Set reception completion					*/
		}
		pUsbfIf->OutSplitCopy = FALSE;				/* Cancel division copy flag			*/
	}
	else
	{
		pUsbfIf->OutSplitCopy = TRUE;				/* Set division copy flag			*/
	}
}


/***************************************************************************
  Description: INData division copy processing
----------------------------------------------------------------------------
  Parameters:
	pUsbfIf;			USBInformation area pointer
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void UsbFuncInDataSplitCopy( USB_FUNCTION_IF *pUsbfIf )
{
	INT				i;							/* Loop counter				*/
	USHORT			strind;						/* Processing position in the transmission/reception buffer		*/
	UINT32			TmpWordData;				/* Word data temporary		*/
	UINT32			TmpFlag;					/* For editing control register value			*/
	UINT32			CopySize;					/* Copy size					*/
	UCHAR			endflag;					/* Termination flag					*/

	endflag = FALSE;							/* Initial value : FALSE				*/

	/* EP1Reset copy size */
	CopySize = SCAN_COPY_SIZE;

	if( pUsbfIf->InLengthRem >= CopySize )
	{
		/*----------------------------------------------------------------------*/
		/*	Remaining sizeEP1maximum packet size or more							*/
		/*----------------------------------------------------------------------*/
		strind = pUsbfIf->InLength;				/* Start of data transmitted this timeindex		*/
		pUsbfIf->InLength += CopySize;			/* Next transmission data startindex		*/
		pUsbfIf->InLengthRem -= CopySize;		/* Residual size calculation					*/
		pUsbfIf->InCopyRem -= CopySize;			/* Calculation for remaining size for this packet	*/
	}
	else
	{
		/*----------------------------------------------------------------------*/
		/*	Remaining sizeEP1maximum packet size (NULL)			*/
		/*----------------------------------------------------------------------*/
		strind = pUsbfIf->InLength;				/* Start of data transmitted this timeindex		*/
		pUsbfIf->InLength += pUsbfIf->InLengthRem;	/* Next transmission data startindex	*/
		pUsbfIf->InCopyRem = 0;					/* Remaining size for this packet		*/
		endflag = TRUE;							/* Remember that the transmission is completed this time	*/
	}
	/*--------------------------------------------------------------------------*/
	/*	Data transmission processing															*/
	/*		※NULL, neverforDo not loop(Do not write)					*/
	/*--------------------------------------------------------------------------*/
	/* Send data settings */
	for( ; (strind + sizeof(UINT32) - 1) < pUsbfIf->InLength ; strind += 4 )
	{
		/* Word(4Byte)Create and write data */
		TmpWordData   = (UINT32)pUsbfIf->InBufPtr[strind+0];
		TmpWordData  |= (UINT32)pUsbfIf->InBufPtr[strind+1] << 8;
		TmpWordData  |= (UINT32)pUsbfIf->InBufPtr[strind+2] << 16;
		TmpWordData  |= (UINT32)pUsbfIf->InBufPtr[strind+3] << 24;
		/* Write buffer */
		ARMPF_EP1_WRITE = TmpWordData;
	}

	/* Data transmission timing */
	if( pUsbfIf->InCopyRem == 0 )
	{
		/* --- Preparation for data transmission permission --- */
		TmpFlag  = ARMPF_EP1_CONTROL;						/* EP1Get control register value		*/
		TmpFlag |= D_EPn_DEND;						/* Transmission permission setting added				*/

		/* Fractional byte processing(endflag=TRUEand the transmission index is less than the transmission size) */
		if( (endflag == TRUE) && (strind < pUsbfIf->InLength) )
		{
			/* Fractional byte calculation(0~3) : 1~3is processed when	*/
			pUsbfIf->InLengthRem &= (BIT1 | BIT0);

			TmpWordData = 0;						/* Data0Clear				*/

			/* Fractional data setting	*/
			for (i = 0; i < pUsbfIf->InLengthRem; i++) {
				TmpWordData |= (UINT32)pUsbfIf->InBufPtr[strind+i] << (8*i);
			}

			/* --- Write buffer ---	*/
			ARMPF_EP1_WRITE = TmpWordData;

			/* --- Setting the number of fractional bytes --- */
			TmpFlag |= ( ((UINT32)pUsbfIf->InLengthRem) << 5 ) & ( D_EP0_DW );
		}

		/* Data transmission */
		ARMPF_EP1_CONTROL |= TmpFlag;

		/*--------------------------------------------------------------------------*/
		/*	Set transmission complete															*/
		/*--------------------------------------------------------------------------*/
		if( endflag == TRUE )
		{
			pUsbfIf->Ep1Status = BLKTRNS_END;	/* Set transmission complete					*/
		}
		pUsbfIf->InSplitCopy = FALSE;			/* Cancel division copy flag				*/
	}
	else
	{
		pUsbfIf->InSplitCopy = TRUE;			/* Setting division copy flag				*/
	}
}
/* <<<<< <S182> */

/***************************************************************************
	Locals	Control transfer related
****************************************************************************/

/***************************************************************************
  Description: EP0 Send processing
----------------------------------------------------------------------------
  Parameters:
	pBuf;				Transmission data storage area
	size;				Send size
  Return:
	0or above;	Number of bytes transmitted
	When the value is negative;		Forced termination due to error occurrence
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static INT UsbFuncWriteEP0( VUCHAR* pBuf, INT size )
{
	INT	iRemainSize;						/* Remaining data length					*/

	UINT32	TmpData;						/* Work for creating write data	*/
	UINT32	temp;							/* Work for editing control register value	*/
	INT		i;								/* Loop counter				*/

	/* Remaining data length	*/
	iRemainSize = size;

	/* If there is remaining data, loop	*/
	while (iRemainSize > 0)
	{
		/*-----------------------------*/
		/* Buffer empty check	   */
		/*-----------------------------*/
		/* Wait until the buffer is empty	*/
		while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)
		{
			/* --- Forcibly terminate when cable is disconnected --- */
			if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
			{
				return ER_SYS;
			}
		}

		/*-----------------------------*/
		/* Write buffer			   */
		/*  (4byteunit, Max:64byte)	   */
		/*-----------------------------*/
		for (i = 0; ((i < 64) && (iRemainSize >= 4)); i += 4, iRemainSize -= 4)
		{
			/* --- Data generation --- */
			TmpData  = (UINT32)*pBuf++;
			TmpData |= (UINT32)*pBuf++ <<  8;
			TmpData |= (UINT32)*pBuf++ << 16;
			TmpData |= (UINT32)*pBuf++ << 24;
			/* --- Write buffer --- */
			ARMPF_EP0_WRITE = TmpData;
		}
		/* --- Preparation for data transmission permission --- */
		temp  = ARMPF_EP0_CONTROL;										/* CONTROLRegister Read	*/
		temp |= D_EP0_DEND;											/* Transmission permission					*/

		/*-----------------------------*/
		/* Fractional byte(3byte)Processing   */
		/*-----------------------------*/
		if ((i < 64) && (iRemainSize > 0) && (iRemainSize < 4))
		{
			/* --- Fractional data generation --- */
			TmpData = 0;
			for (i = 0; i < iRemainSize; i++)
			{
				TmpData |= (UINT32)*pBuf++ << (8*i);
			}
			/* --- Write buffer --- */
			ARMPF_EP0_WRITE = TmpData;
			/* --- Data transmission permission --- */
			temp |= ( ((UINT32)iRemainSize) << 5 ) & ( D_EP0_DW );	/* Fractional byte setting			*/
			iRemainSize = 0;
		}
		/*-----------------------------*/
		/* Data transmission permission			   */
		/*-----------------------------*/
		ARMPF_EP0_CONTROL = temp;											/* CONTROLRegister Write	*/
	}

	return size;
}

/***************************************************************************
  Description: EP0 Stall response processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void UsbFuncSetStallEP0( void )
{
	ARMPF_EP0_CONTROL |= D_EP0_STL;				/* Stall response setting			*/
}

#if 0	/* Currently not used <S150>*/
/***************************************************************************
  Description: EP0 NAKResponse processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void UsbFuncSetNakEP0( void )
{
	UINT32 temp;								/* Work for setting editing				*/

	temp  = ARMPF_EP0_CONTROL;						/* EP0Read control register value	*/
	temp |= D_EP0_INAK_EN;						/* EP0_INAKWriting enabled		*/
	temp |= (UINT32)(D_EP0_INAK | D_EP0_ONAK);	/* IN/OUT/PINGToken		*/
												/* ForcedNAKResponse setting				*/
	ARMPF_EP0_CONTROL = temp;							/* EP0Write to control register	*/
}

/***************************************************************************
  Description: EP0 NAKCancellation process
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static void UsbFuncClearNakEP0( void )
{
	UINT32 temp;								/* Work for setting editing				*/

	temp  = ARMPF_EP0_CONTROL;						/* EP0Read control register value	*/
	temp |= D_EP0_INAK_EN;						/* EP0_INAKWriting enabled		*/
	temp &= ~(UINT32)(D_EP0_INAK | D_EP0_ONAK);	/* IN/OUT/PINGToken		*/
												/* ForcedNAKResponse cancellation setting			*/
	ARMPF_EP0_CONTROL = temp;							/* EP0Write to control register	*/
}

/***************************************************************************
  Description: EP0 NULLSend processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	none
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET UsbFuncSendNullEP0( void )
{
	/*-----------------------------*/
	/* Buffer empty check	   */
	/*-----------------------------*/
	/* Wait until the buffer is empty	*/
	while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)
	{
		/* --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return ER_SYS;
		}
	}

	/*-----------------------------*/
	/* Data transmission permission			   */
	/*-----------------------------*/
	ARMPF_EP0_CONTROL |= D_EP0_DEND;					/* Transmission permission					*/

	return ER_OK;
}
#endif

/***************************************************************************
  Description: EP0 No DataProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET control_nodata( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UINT32	temp;					/* Work for setting editing		*/
	ER_RET	result;					/* Result					*/

	/* Return value initialization	*/
	result = ER_SYS;

	/*=========================================*/
	/* Decoding process							   */
	/*=========================================*/
	result = decode_request( pDeviceRequest, pUsbfInfo );
	if( ER_OK != result )
	{
		UsbFuncSetStallEP0();
		return result;
	}

	/*=========================================*/
	/* Waiting for status/stage transition			   */
	/*=========================================*/
	/* Wait until transition to status stage */
	while((ARMPF_EP0_STATUS & D_EP0_STG_START_INT) != D_EP0_STG_START_INT)
	{
		/* --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return result;
		}
	}
	ARMPF_EP0_STATUS = ~D_EP0_STG_START_INT;							/* Cause clear */

	if((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)
	{															/* When the buffer is not empty,			*/
		ARMPF_EP0_CONTROL |= D_EP0_BCLR;								/*  -> Empty the buffer.			*/
		while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)	/*  -> Wait until it's actually empty.	*/
		{
			/* --- Forcibly terminate when cable is disconnected --- */
			if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
			{
				return result;
			}
		}
	}

	/*=========================================*/
	/* INToken forceNAKCancel response			   */
	/*=========================================*/
	temp = ARMPF_EP0_CONTROL;
	temp |= D_EP0_INAK_EN;
	temp &= ~(UINT32)D_EP0_INAK;
	ARMPF_EP0_CONTROL = temp;

	/*=========================================*/
	/* Status stage					   */
	/*=========================================*/
	/* NULLData transmission */
	/* ------ Buffer empty check ------ */
	while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)	/* Wait until the buffer is empty */
	{
		/*/ --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return result;
		}
	}

	/* ------ Data transmission permission ------ */
	temp  = ARMPF_EP0_CONTROL;									/* CONTROLRegister Read	*/
	temp |= D_EP0_DEND;										/* Transmission permission					*/
	ARMPF_EP0_CONTROL = temp;										/* CONTROLRegister Write	*/

	/*----------------------------------------*/
	/* Status stage completion confirmation			  */
	/*----------------------------------------*/
	/* Wait until status stage ends */
	while((ARMPF_EP0_STATUS & D_EP0_STG_END_INT) != D_EP0_STG_END_INT)
	{
		/* --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return result;
		}
	}
	ARMPF_EP0_STATUS = ~D_EP0_STG_END_INT;						/* Cause clear */

	return ER_OK;
}


/***************************************************************************
  Description: EP0 Control ReadProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET control_read( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UINT32	temp;					/* Work for setting editing		*/
	ER_RET	result;					/* Result					*/
	UINT32	tcnt;					/* Timeout counter	<S1FE> */

	/* Return value initialization	*/
	result = ER_SYS;

	/*=========================================*/
	/* Buffer empty check				   */
	/*=========================================*/
	if((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)
	{															/* When the buffer is not empty,			*/
		ARMPF_EP0_CONTROL |= D_EP0_BCLR;								/*  -> Empty the buffer.			*/
		while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)	/*  -> Wait until it's actually empty.	*/
		{
			/* --- Forcibly terminate when cable is disconnected --- */
			if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
			{
				return result;
			}
		}
	}

	/*=========================================*/
	/* INToken forceNAKCancel response			   */
	/*=========================================*/
	temp = ARMPF_EP0_CONTROL;
	temp |= D_EP0_INAK_EN;
	temp &= ~(UINT32)D_EP0_INAK;
	ARMPF_EP0_CONTROL = temp;

	/*=========================================*/
	/* Decoding process							   */
	/*=========================================*/
	result = decode_request( pDeviceRequest, pUsbfInfo );
	if( ER_OK != result ) {
		UsbFuncSetStallEP0();
		return result;
	}

	/*=========================================*/
	/* Waiting for status/stage transition			   */
	/*=========================================*/
	/* Wait until transition to status stage */
	while((ARMPF_EP0_STATUS & D_EP0_STG_START_INT) != D_EP0_STG_START_INT)
	{
		/* --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return result;
		}
	}
	ARMPF_EP0_STATUS = ~D_EP0_STG_START_INT;								/* Cause clear */

	if((ARMPF_EP0_STATUS & D_EP0_OUT_EMPTY) != D_EP0_OUT_EMPTY)
	{																/* When the buffer is not empty,			*/
		ARMPF_EP0_CONTROL |= D_EP0_BCLR;									/*  -> Empty the buffer.			*/
		while((ARMPF_EP0_STATUS & D_EP0_OUT_EMPTY) != D_EP0_OUT_EMPTY)	/*  -> Wait until it's actually empty.	*/
		{
			/* --- Forcibly terminate when cable is disconnected --- */
			if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
			{
				return result;
			}
		}
	}

	/*=========================================*/
	/* OUTToken forceNAKCancel response			   */
	/*=========================================*/
	ARMPF_EP0_CONTROL &= ~(UINT32)D_EP0_ONAK;

	/*----------------------------------------*/
	/* Status stage					  */
	/*----------------------------------------*/
	/* NULLData reception */
	while((ARMPF_EP0_STATUS & D_EP0_OUT_NULL_INT) != D_EP0_OUT_NULL_INT)	/* NULLWait until data is received */
	{
		/* --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return result;
		}
	}
	ARMPF_EP0_STATUS = ~D_EP0_OUT_NULL_INT;								/* Cause clear */

	/*----------------------------------------*/
	/* Status stage completion confirmation			  */
	/*----------------------------------------*/
	/* Wait until status stage ends */
	KlibRstLongTimer(&tcnt);										/* Timer count start <S1FE> */
	while((ARMPF_EP0_STATUS & D_EP0_STG_END_INT) != D_EP0_STG_END_INT)
	{
		/* --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return result;
		}
		if( UsbFuncCheckSetupReceive() != 0 )
		{
			return result;
		}
		if( KlibGetLongTimerMs(tcnt) > 1000  )
		{/* 1s, it will be a timeout. <S1FE> */
			return result;
		}
	}
	ARMPF_EP0_STATUS = ~D_EP0_STG_END_INT;								/* Cause clear */

	return ER_OK;
}


/***************************************************************************
  Description: EP0 Control WriteProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET control_write( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UINT32	temp;					/* Work for setting editing		*/
	ER_RET	result;					/* Result					*/

	/* Return value initialization	*/
	result = ER_SYS;

	/*=========================================*/
	/* Buffer empty check				   */
	/*=========================================*/
	if((ARMPF_EP0_STATUS & D_EP0_OUT_EMPTY) != D_EP0_OUT_EMPTY)
	{																/* When the buffer is not empty,			*/
		ARMPF_EP0_CONTROL |= D_EP0_BCLR;									/*  -> Empty the buffer.			*/
		while((ARMPF_EP0_STATUS & D_EP0_OUT_EMPTY) != D_EP0_OUT_EMPTY)	/*  -> Wait until it's actually empty.	*/
		{
			/* --- Forcibly terminate when cable is disconnected --- */
			if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
			{
				return result;
			}
		}
	}

	/*=========================================*/
	/* OUTToken forceNAKCancel response			   */
	/*=========================================*/
	ARMPF_EP0_CONTROL &= ~(UINT32)D_EP0_ONAK;

	/*=========================================*/
	/* Decoding process							   */
	/*=========================================*/
	result = decode_request( pDeviceRequest, pUsbfInfo );
	if( ER_OK != result )
	{
		UsbFuncSetStallEP0();
		return result;
	}

	/*=========================================*/
	/* Waiting for status/stage transition			   */
	/*=========================================*/
	/* Wait until transition to status stage */
	while((ARMPF_EP0_STATUS & D_EP0_STG_START_INT) != D_EP0_STG_START_INT)
	{
		/* --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return result;
		}
	}
	ARMPF_EP0_STATUS = ~D_EP0_STG_START_INT;							/* Cause clear */

	if((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)
	{															/* When the buffer is not empty,			*/
		ARMPF_EP0_CONTROL |= D_EP0_BCLR;								/*  -> Empty the buffer.			*/
		while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)	/*  -> Wait until it's actually empty.	*/
		{
			/* --- Forcibly terminate when cable is disconnected --- */
			if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
			{
				return result;
			}
		}
	}

	/*=========================================*/
	/* INToken forceNAKCancel response			   */
	/*=========================================*/
	temp = ARMPF_EP0_CONTROL;
	temp |= D_EP0_INAK_EN;
	temp &= ~(UINT32)D_EP0_INAK;
	ARMPF_EP0_CONTROL = temp;

	/*----------------------------------------*/
	/* Status stage					  */
	/*----------------------------------------*/
	ARMPF_EP0_CONTROL |= D_EP0_DEND;

	/*----------------------------------------*/
	/* Status stage completion confirmation			  */
	/*----------------------------------------*/
	while((ARMPF_EP0_STATUS & D_EP0_STG_END_INT) != D_EP0_STG_END_INT)
	/* Wait until status stage ends */
	{
		/* --- Forcibly terminate when cable is disconnected --- */
		if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
		{
			return result;
		}
	}
	ARMPF_EP0_STATUS = ~D_EP0_STG_END_INT;							/* Cause clear */

	return ER_OK;
}


/*=============================================================================*/
/* USB STANDARD REQUEST														   */
/*=============================================================================*/

/***************************************************************************
  Description: EP0 Request decode processing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET decode_request(DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	ER_RET	result;					/* Result					*/

	/* Return value initialization	*/
	result = ER_SYS;

	/* Control transfer type determination */
	switch( pDeviceRequest->bmRequestType & USB_REQUEST )
	{
		/*-----------------------------*/
		/* Standard device request	   */
		/*-----------------------------*/
		case USB_REQUEST_STANDARD:
			result = bmRequestType_standard( pDeviceRequest, pUsbfInfo );
			break;

		/*-----------------------------*/
		/* Class request		   */
		/*-----------------------------*/
		case USB_REQUEST_CLASS:
			if( pUsbfInfo->cb_class_req )
			{
				result = pUsbfInfo->cb_class_req( pDeviceRequest );
			}
			else
			{
				result = ER_SYS;
			}
			break;

		/*-----------------------------*/
		/* Vendor-specific request		   */
		/*-----------------------------*/
		case USB_REQUEST_VENDOR:
			if( pUsbfInfo->cb_vendor_req )
			{
				result = pUsbfInfo->cb_vendor_req( pDeviceRequest );
			}
			else
			{
				result = ER_SYS;
			}
			break;

		/*-----------------------------*/
		/* Others					   */
		/*-----------------------------*/
		default:
			result = ER_SYS;
			break;
	}
	return result;
}

/***************************************************************************
  Description: EP0 Standard request processing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET bmRequestType_standard(  DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UCHAR	bRequest;			/* Request code	*/
	ER_RET	result;				/* Result				*/

	/* Initializing variables */
	bRequest = pDeviceRequest->bRequest;
	result = ER_SYS;

	switch( bRequest ) {
		case USB_STD_GET_STATUS:							/* GET STATUS			*/
			result = std_req_get_status( pDeviceRequest, pUsbfInfo );
			break;
		case USB_STD_CLEAR_FEATURE:							/* CLEAR FEATURE		*/
			result = std_req_clear_feature( pDeviceRequest );
			break;
		case USB_STD_SET_FEATURE:							/* SET FEATURE			*/
			result = std_req_set_feature( pDeviceRequest, pUsbfInfo );
			break;
		case USB_STD_SET_ADDRESS:							/* SET ADDRESS			*/
			result = std_req_set_address( pDeviceRequest );
			break;
		case USB_STD_GET_DESCRIPTOR:						/* GET DESCRIPTOR		*/
			result = std_req_get_descriptor( pDeviceRequest, pUsbfInfo );
			break;
		case USB_STD_SET_DESCRIPTOR:						/* SET DESCRIPTOR		*/
			result = std_req_set_descriptor( pDeviceRequest );
			break;
		case USB_STD_GET_CONFIGURATION:						/* GET CONFIGURATION	*/
			result = std_req_get_configuration( pDeviceRequest, pUsbfInfo );
			break;
		case USB_STD_SET_CONFIGURATION:						/* SET CONFIGURATION	*/
			result = std_req_set_configuration( pDeviceRequest, pUsbfInfo );
			break;
		case USB_STD_GET_INTERFACE:							/* GET INTERFACE		*/
			result = std_req_get_interface( pDeviceRequest, pUsbfInfo );
			break;
		case USB_STD_SET_INTERFACE:							/* SET INTERFACE		*/
			result = std_req_set_interface( pDeviceRequest, pUsbfInfo );
			break;
		case USB_STD_SYNCH_FRAME:							/* SYNC FRAME			*/
			result = std_req_synch_frame( pDeviceRequest );
			break;
		default:
			result = ER_SYS;
			break;
	}
	return result;
}


/***************************************************************************
  Description: EP0 CLEAR_FEATUREProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_clear_feature( DEVICE_REQUEST* pDeviceRequest )
{
	UINT8	recipient;				/* Processing target		*/
	UINT8	direction;				/* I/O direction			*/
	UINT16	selector;				/* Selector value			*/
	UINT16	wIndex;					/* Index value		*/
	UINT16	endpoint;				/* Endpoint number	*/
	ER_RET	result;					/* Result					*/
	USB_FUNCTION_IF	*pUsbfIf;		/* USBInformation area pointer	*/
	USB_INFO		*pUsbInfo;		/* USBInformation pointer		*/

	/* Initializing variables */
	pUsbfIf = UsbFuncGetInfoAddress();
	pUsbInfo = &(pUsbfIf->UsbfInfo.usb);

	recipient = (UINT8)(pDeviceRequest->bmRequestType & USB_TARGET);
	direction = (UINT8)(pDeviceRequest->bmRequestType & USB_DIR);
	selector  = pDeviceRequest->wValue;
	wIndex    = pDeviceRequest->wIndex;
	endpoint  = wIndex & 0xFF;

	result = ER_SYS;

	/*---------------------------------*/
	/* Parameter check(Constant location)  */
	/*---------------------------------*/
	if( ( 0x0000 != pDeviceRequest->wLength ) ||
		( USB_DIR_OUT != direction ) )
	{
		return ER_SYS;
	}

	switch( recipient )
	{
		case USB_TARGET_DEVICE:
			if( 0x0000 == wIndex )
			{
				if( USB_FEATURE_DEVICE_REMOTE_WAKEUP == selector )
				{
					/* Remote wakeup prohibition setting */
					pUsbInfo->flag_RemoteWakeup = U2F_DISABLE;
					result = ER_OK;
				}
				else
				{
					result = ER_SYS;
				}
			}
			else
			{
				result = ER_SYS;
			}
			break;
		case USB_TARGET_INTERFACE:
			result = ER_SYS;
			break;
		case USB_TARGET_ENDPOINT:
			if ( ( 0x0000 != ( wIndex & 0xFF00 ) )   ||
				 ( 0x0000 != ( wIndex & 0x0070 ) ) )
			{
				return ER_SYS;
			}

			if( USB_FEATURE_ENDPOINT_HALT == selector )
			{
				if( ( 0x0000 == endpoint ) || ( 0x0080 == endpoint ) )
				{
					/* EP0 */
					/* Buffer empty check */
					if((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)
					{															/* When the buffer is not empty,			*/
						ARMPF_EP0_CONTROL |= D_EP0_BCLR;								/*  -> Empty the buffer.			*/
						while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)	/*  -> Wait until it's actually empty.	*/
						{
							/* --- Forcibly terminate when cable is disconnected --- */
							if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
							{
								return ER_SYS;
							}
						}
					}
					result = ER_OK;
				}
				else
				{
					/* Other EP */
					if( ER_OK != Clear_Feature_EP( endpoint & ~((UINT)USB_DIR)) )
					{
						return ER_SYS;
					}
				}
				result = ER_OK;
			}
			else
			{
				return ER_SYS;
			}
			break;
		default:
			result = ER_SYS;
			break;
	}
	return result;
}

/***************************************************************************
  Description: EP0 CLEAR_FEATUREProcessing( TARGET = EP )
----------------------------------------------------------------------------
  Parameters:
	num;				EPNumber
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Clear_Feature_EP( UINT num )
{
	UINT32	temp;					/* Work for setting editing		*/
	UINT32	temp2;					/* Work for setting confirmation		*/

	/* ParameterConfirm */
	if(( num < 1 ) || ( num > 9))
	{
		return ER_PARAM;
	}

	/* EP EnableConfirm */
	if ( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_EN) != D_EPn_EN)
	{
		return ER_SYS;
	}

	/* EPn Clear Feature */
	if( (udev_get_DeviceState() & UDEV_ST_CONFIGURED) != 0 )
	{
		/* Buffer Clear */
		temp = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
		temp |= D_EPn_BCLR;													/* Transmission/reception buffer clear */
		*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) = temp;

		if( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_DIR0) != D_EPn_DIR0 )
		{	/* IN */
			/* PIDInitialize */
			temp = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
			temp |= D_EPn_IPIDCLR;											/* TransmissionPIDClear */
			*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) = temp;
			/* Buffer empty check */
			while( ((*((VUINT32*)(EPn_STATUS_ADR + (num - 1) * 0x20)) & D_EPn_IN_EMPTY) != D_EPn_IN_EMPTY) ||
			       ((*((VUINT32*)(EPn_STATUS_ADR + (num - 1) * 0x20)) & D_EPn_IN_DATA ) == D_EPn_IN_DATA ) )
			{	/*  -> Wait until it's actually empty. */
				/* --- Forcibly terminate when cable is disconnected --- */
				if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
				{
					return ER_SYS;
				}
			}
			/* EP STALLCancellation */
			temp = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
			temp &= ~(D_EPn_ISTL);											/* INToken STALLCancellation */
			*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) = temp;
		}
		else
		{	/* OUT */
			/* PIDInitialize */
			temp = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
			temp |= D_EPn_OPIDCLR;											/* ReceivingPIDClear */
			*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) = temp;
			/* EP STALLCancellation */
			temp = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
			temp |= D_EPn_OSTL_EN;											/* EPn_OSTL Enable writing to bit	*/
			temp &= ~(D_EPn_OSTL);											/* OUT/PINGToken STALLCancellation				*/
			*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) = temp;
		}
	}
	else
	{
		return ER_SYS;
	}

	temp = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
	temp2 = *((VUINT32*)(EPn_STATUS_ADR + (num - 1) * 0x20));
	if ((temp | temp2) != 0)
	{
		__nop();
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 GET_CONFIGURATIONProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_get_configuration( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	ER_RET	result;					/* Result					*/

	/* Initializing variables */
	result = ER_SYS;

	if( ( 0x0000 != pDeviceRequest->wValue  ) ||
	    ( 0x0000 != pDeviceRequest->wIndex  ) ||
	    ( 0x0001 != pDeviceRequest->wLength ) ||
	    ( USB_DIR != pDeviceRequest->bmRequestType ) )
	{
		return ER_SYS;
	}

	/* Configuration value is "1" is only supported */
	result = UsbFuncWriteEP0( (UCHAR*)&(pUsbfInfo->usb.curr_config), 1 );
	if( result < 0 )
	{
		return result;
	}
	return ER_OK;
}

/***************************************************************************
  Description: EP0 GET_DESCRIPTORProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_get_descriptor( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UINT16	wValue;					/* value					*/
	UINT16	wIndex;					/* Index value		*/
	UINT16	descriptor_type;		/* Descriptor type	*/
	ER_RET	result;					/* Result					*/

	/* Initializing variables */
	wValue			= pDeviceRequest->wValue;
	wIndex			= pDeviceRequest->wIndex;
	descriptor_type	= (wValue >> 8);

	result			= ER_SYS;

	/* Request type confirmation */
	if( USB_DIR != pDeviceRequest->bmRequestType )
	{
		return ER_SYS;
	}

	/* Processing for each descriptor type */
	switch( descriptor_type )
	{
		case USB_DESC_TYPE_DEVICE:
			if( ( 0x0000 == ( wValue & 0x00ff ) ) && ( 0x0000 == wIndex ) )
			{
				/* Device Descriptor */
				result = Get_Descriptor_Device( pDeviceRequest, pUsbfInfo );
			}
			else
			{
				return ER_SYS;
			}
			break;
		case USB_DESC_TYPE_CONFIGURATION:
			if( ( 0x0000 == ( wValue & 0x00ff ) ) && ( 0x0000 == wIndex ) )
			{
				/* Configuration Descriptor */
				result = Get_Descriptor_Configuration( pDeviceRequest, pUsbfInfo );
			}
			else
			{
				return ER_SYS;
			}
			break;
		case USB_DESC_TYPE_STRING:
			/* String Descriptor */
			result = Get_Descriptor_String( pDeviceRequest, pUsbfInfo );
			break;
		case USB_DESC_TYPE_DEVICE_QUALIFIER:
			if( ( 0x0000 == ( wValue & 0x00ff ) ) && ( 0x0000 == wIndex ) )
			{
				/* Device Qualifier Descriptor */
				result = Get_Descriptor_Qualifier( pDeviceRequest, pUsbfInfo );
			}
			else
			{
				return ER_SYS;
			}
			break;
		case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
			if( ( 0x0000 == ( wValue & 0x00ff ) ) && ( 0x0000 == wIndex ) )
			{
				/* Qther Speed Configuration Descriptor */
				result = Get_Descriptor_OtherSpeed( pDeviceRequest, pUsbfInfo );
			}
			else
			{
				return ER_SYS;
			}
			break;
		case USB_DESC_TYPE_INTERFACE_POWER:
			result = ER_SYS;
			break;
		default:
			result = ER_SYS;
			break;
	}
	return result;
}

/***************************************************************************
  Description: EP0 GET_DESCRIPTOR DEVICEProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Get_Descriptor_Device( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	void*	pData;					/* DEVICEDescriptor start address	*/
	INT		size;					/* DEVICEDescriptor data size	*/
	ER_RET	result;					/* Result									*/

	/* Initializing variables */
	size = 0;
	result = ER_SYS;

	/* DEVICEReturn descriptor */
	pData  = (void*)(pUsbfInfo->desc.pDevice);
	size   = (INT)((DEVICE_DESCRIPTOR*)pData)->bLength;
	if (size > (INT)pDeviceRequest->wLength)
	{
		size = (INT)pDeviceRequest->wLength;
	}
	result = UsbFuncWriteEP0( (UCHAR*)pData, size );
	if( result < 0 )
	{
		return result;
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 GET_DESCRIPTOR CONFIGURATIONProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Get_Descriptor_Configuration( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	DESC_CONFIG*	pConfigDesc;	/* CONFIGURATIONDescriptor start address	*/
	INT				size;			/* CONFIGURATIONDescriptor data size	*/
	ER_RET			result;			/* Result											*/

	/* Initializing variables */
	size	= 0;
	result	= ER_SYS;

	/* CONFIGURATIONReturn descriptor */
	if( (ARMPF_USB_STATUS & D_USB_SPEED_MODE) == D_USB_SPEED_MODE )
	{
		/* High Speed */
		pConfigDesc = pUsbfInfo->desc.pConf_hs;
		size = (INT)((pConfigDesc->conf_desc.wTotalLength[1] << 16) + pConfigDesc->conf_desc.wTotalLength[0]);
	}
	else
	{
		/* Full Speed */
		pConfigDesc = pUsbfInfo->desc.pConf_fs;
		size = (INT)((pConfigDesc->conf_desc.wTotalLength[1] << 16) + pConfigDesc->conf_desc.wTotalLength[0]);
	}

	/* Transmission */
	if (size > (INT)pDeviceRequest->wLength)
	{
		size = (INT)pDeviceRequest->wLength;
	}
	result = UsbFuncWriteEP0( (UCHAR*)pConfigDesc, size );
	if( result < 0 ) {
		return result;
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 GET_DESCRIPTOR STRINGProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Get_Descriptor_String( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	void*	pData;					/* STRINGDescriptor start address	*/
	INT		size;					/* STRINGDescriptor data size	*/
	ER_RET	result;					/* Result									*/
	INT		string_num;				/* STRINGNumber							*/

	/* Initializing variables */
	size	= 0;
	result	= ER_SYS;

	/* RequestedSTRINGNumber */
	string_num = (INT)(pDeviceRequest->wValue & 0xff);

	/* Not Support INDEX */
	if( ( string_num < 0 ) || ( STRIDX_MAX <= string_num ) )
	{
		return ER_SYS;
	}

	pData = (void*)(pUsbfInfo->desc.pString[string_num]);
	size  = (INT)(((STRING_DESCRIPTOR*)pData)->bLength);

	/* When data size is larger than request size */
	if (size > (INT)pDeviceRequest->wLength)
	{
		size = (INT)pDeviceRequest->wLength;
	}
	result = UsbFuncWriteEP0( (UCHAR*)pData, size );
	if( result < 0 )
	{
		return result;
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 GET_DESCRIPTOR DEVICE QUALIFIERProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Get_Descriptor_Qualifier( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	void*	pData;					/* DEVICE QUALIFIERDescriptor start address	*/
	INT		size;					/* DEVICE QUALIFIERDescriptor data size	*/
	ER_RET	result;					/* Result											*/

	/* Initializing variables */
	size	= 0;
	result	= ER_SYS;

	/* DEVICE QUALIFIERReturn descriptor */
	pData  = (void*)(pUsbfInfo->desc.pQualifier);
	size   = (INT)(((DEVICE_DESCRIPTOR*)pData)->bLength);

	/* When data size is larger than request size */
	if (size > (INT)pDeviceRequest->wLength)
	{
		size = (INT)pDeviceRequest->wLength;
	}

	result = UsbFuncWriteEP0( pData, size );
	if( result < 0 )
	{
		return result;
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 GET_DESCRIPTOR OTHER_SPEED_CONFIGURATIONProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Get_Descriptor_OtherSpeed( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	DESC_CONFIG*	pConfigDesc;	/* CONFIGURATIONDescriptor start address	*/
	DESC_CONFIG		ConfigDescWork;	/* CONFIGURATIONDescriptor copy work	*/
	INT				size;			/* CONFIGURATIONDescriptor data size	*/
	ER_RET			result;			/* Result											*/

	/* Initializing variables */
	size	= 0;
	result	= ER_SYS;

	/* CONFIGURATIONReturn descriptor */
	if( (ARMPF_USB_STATUS & D_USB_SPEED_MODE) == D_USB_SPEED_MODE )
	{
		/* CurrentHighSpeed(Full SpeedReturn descriptor) */
		pConfigDesc = pUsbfInfo->desc.pConf_fs;
		size = (INT)((pConfigDesc->conf_desc.wTotalLength[1] << 16) + pConfigDesc->conf_desc.wTotalLength[0]);
	}
	else
	{
		/* CurrentFullSpeed(High SpeedReturn descriptor)*/
		pConfigDesc = pUsbfInfo->desc.pConf_hs;
		size = (INT)((pConfigDesc->conf_desc.wTotalLength[1] << 16) + pConfigDesc->conf_desc.wTotalLength[0]);
	}

	/* Copy data and change descriptor type */
	memcpy(&ConfigDescWork, pConfigDesc, sizeof(DESC_CONFIG));
	ConfigDescWork.conf_desc.bDescriptorType = (UINT8)0x07;

	/* When data size is larger than request size */
	if (size > (INT)pDeviceRequest->wLength)
	{
		size = (INT)pDeviceRequest->wLength;
	}

	result = UsbFuncWriteEP0( (UCHAR*)&ConfigDescWork, size );
	if( result < 0 )
	{
		return result;
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 GET_INTERFACEProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_get_interface( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	INTERFACE_DESCRIPTOR*	pIfDesc;	/* INTERFACEDescriptor start address	*/
	UINT8					data;		/* Returned data work							*/
	ER_RET					result;		/* Result										*/

	/* Initializing variables */
	pIfDesc	= NULL;
	data	= (UINT8)0;
	result	= ER_SYS;

	/* Request content check */
	if( ( 0x0000      != pDeviceRequest->wValue )	||
		( 0x0001      != pDeviceRequest->wLength )	||
	    ( (UINT8)0x81 != pDeviceRequest->bmRequestType  ) )
	{
		return ER_SYS;
	}

	/* Device status isCONFIGURED */
	if( UDEV_ST_CONFIGURED == udev_get_DeviceState() )
	{
		/* INTERFACESearch descriptor */
		pIfDesc = info_find_InterfaceDesc( pDeviceRequest->wIndex, pUsbfInfo );
		if( pIfDesc != NULL )
		{
			data = pIfDesc->bAlternateSetting;
			result = UsbFuncWriteEP0( (UCHAR*)&data, 1 );
			if( result < 0 )
			{
				return ER_SYS;
			}
			else
			{
				return ER_OK;
			}
		}
	}

	return ER_SYS;
}

/***************************************************************************
  Description: EP0 GET_STATUSProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_get_status( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UINT8	recipient;				/* Processing target	*/
	UINT8	direction;				/* I/O direction		*/
	ER_RET  result;					/* Result				*/

	/* Initializing variables */
	recipient = (UINT8)(pDeviceRequest->bmRequestType & USB_TARGET);
	direction = (UINT8)(pDeviceRequest->bmRequestType & USB_DIR);
	result = ER_SYS;

	/* Request content check */
	if( ( 0x0000 != pDeviceRequest->wValue )	||
		( 0x0002 != pDeviceRequest->wLength )	||
		( USB_DIR_IN != direction ) )
	{
		return ER_SYS;
	}

	switch( recipient )
	{
		case USB_TARGET_DEVICE:
			if( 0x0000 == pDeviceRequest->wIndex )
			{
				result = Get_Status_Device( pDeviceRequest, pUsbfInfo );
			}
			else
			{
				return ER_SYS;
			}
			break;
		case USB_TARGET_INTERFACE:
			result = ER_SYS;
			break;
		case USB_TARGET_ENDPOINT:
			result = Get_Status_EP( pDeviceRequest );
			break;
		default:
			result = ER_SYS;
			break;
	}
	return result;
}

/***************************************************************************
  Description: EP0 GET_STATUS DEVICEProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Get_Status_Device( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UINT16	data;					/* For editing returned data		*/
	ER_RET	result;					/* Result					*/

	/* Initializing variables */
	data	= 0;
	result	= ER_SYS;

	/* bmAttributesofSelfPowerd(D6)Bit increment of transmitted dataD0fart */
	data = ( (UINT16)(pUsbfInfo->desc.pConf_hs->conf_desc.bmAttributes) >> 6 ) & (UINT16)BIT0;

	/* Remote wakeup isD1fart */
	if( U2F_ENABLE == pUsbfInfo->usb.flag_RemoteWakeup )
	{
		data |= (UINT8)BIT1;
	}
	/* STATUSReturning data */
	result = UsbFuncWriteEP0( (UCHAR*)&data, 2 );
	if( result < 0 )
	{
		return result;
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 GET_STATUS ENDPOINTProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Get_Status_EP( DEVICE_REQUEST* pDeviceRequest )
{
	UINT	num;					/* EPNumber				*/
	UINT8	direction;				/* Direction					*/
	INT		status;					/* STALLStatus			*/
	UINT16	data;					/* For editing returned data		*/
	ER_RET	result;					/* Result					*/
	UINT16	wIndex;					/* Index value		*/
	UINT	endpoint;				/* Endpoint number	*/

	/* Initializing variables */
	num			= (UINT)( (pDeviceRequest->wIndex) & ~(UINT32)USB_DIR );
	direction	= (UINT8)(pDeviceRequest->wIndex & USB_DIR);
	data		= 0;
	result		= ER_SYS;
	wIndex		= pDeviceRequest->wIndex;
	endpoint	= (UINT)(wIndex & 0xFF);

	/* Endpoint number check */
	if ( ( 0x0000 != ( wIndex & 0xFF00 ) )   ||
		 ( 0x0000 != ( wIndex & 0x0070 ) ) )
	{
		return ER_SYS;
	}

	if( ( 0x0000 == endpoint ) || ( 0x0080 == endpoint ) )
	{
		/* EP0 */
		data = 0;
	}
	else
	{
		/* Other EP */
		if( UDEV_ST_CONFIGURED == udev_get_DeviceState() )
		{
			/* Direction check */
			if( ER_OK != Check_USB_DIR_EPn( num, direction ) )
			{
				return ER_SYS;
			}

			status = EPn_get_STALL( num );	/* STALLState acquisition */
			if( status < 0 )
			{
				return ER_SYS;
			}
			if( 0 == status )
			{
				data = 0;			/* EPnteethSTALLnot	*/
			}
			else
			{
				data = 1;			/* EPnteethSTALLis	*/
			}
		}
		else
		{
			return ER_SYS;
		}
	}

	result = UsbFuncWriteEP0( (UCHAR*)&data, 2 );
	if( result < 0 ) {
		return result;
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 SET_ADDRESSProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_set_address( DEVICE_REQUEST* pDeviceRequest )
{
	UINT16	wValue;					/* Address value			*/

	/* Initializing variables */
	wValue = pDeviceRequest->wValue;

	/* Request content check */
	if( ( pDeviceRequest->bmRequestType != (UINT8)0x00 )	||
		( pDeviceRequest->wIndex != 0x0000 )		||
		( pDeviceRequest->wLength != 0x0000 ) )
	{
		return ER_SYS;
	}

	/* Address value check */
	if( wValue != ( wValue & 0x007F ) )
	{
		return ER_SYS;
	}

	ARMPF_USB_ADDRESS = (UINT32)wValue << 16;

	return ER_OK;
}

/***************************************************************************
  Description: EP0 SET_CONFIGURATIONProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_set_configuration( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UINT16	ConfigValue;			/* CONFIGURATIONNumber	*/
	UINT32	tmp;					/* Work for setting editing		*/
	ER_RET	result;					/* Result					*/

	/* Initializing variables */
	ConfigValue	= pDeviceRequest->wValue & 0x00ff;
	result		= ER_SYS;

	/* Request content check */
	if( ( 0x0000      != pDeviceRequest->wIndex )	||
		( 0x0000      != pDeviceRequest->wLength )	||
	    ( (UINT8)0x00 != pDeviceRequest->bmRequestType  ) )
	{
		return ER_SYS;
	}

	/* CONFIGURATIONPerform processing for each number */
	switch( ConfigValue )
	{
		case 0:
			/* AddressReturn to state */
			pUsbfInfo->usb.curr_config = 0;
			ARMPF_USB_CONTROL &= ~D_USB_CONF;

			/* --- EP Buffer clear --- */
			ARMPF_EP1_CONTROL   |= D_EPn_BCLR;							/* EP1 : Buffer clear		*/
			ARMPF_EP2_CONTROL   |= D_EPn_BCLR;							/* EP2 : Buffer clear		*/
			/* --- PIDInitialize --- */
			ARMPF_EP1_CONTROL |= D_EPn_IPIDCLR;							/* EP1 : TransmissionPIDClear			*/
			ARMPF_EP2_CONTROL |= D_EPn_OPIDCLR;							/* EP2 : ReceivingPIDClear			*/
			/* --- EP STALLCancellation --- */
			tmp = ARMPF_EP1_CONTROL;
			tmp &= ~(D_EPn_ISTL);									/* EP1 : INToken STALLCancellation	*/
			ARMPF_EP1_CONTROL = tmp;

			tmp = ARMPF_EP2_CONTROL;
			tmp |= D_EPn_OSTL_EN;									/* EPn_OSTL Enable writing to bit	*/
			tmp &= ~(D_EPn_OSTL);									/* EP2 : OUT/PINGToken STALLCancellation			*/
			ARMPF_EP2_CONTROL = tmp;

			result = ER_OK;
			break;
		case 1:
			/* ConfiguredState */
			/* Set Current Configuration Number */
			if (ConfigValue > (UINT16)(pUsbfInfo->desc.pDevice->bNumConfigurations))
			{
				return ER_SYS;
			}
			pUsbfInfo->usb.curr_config = ConfigValue;
			ARMPF_USB_CONTROL |= D_USB_CONF;
			result = ER_OK;
			break;
		default:
			/* Unsupported0 */
			pUsbfInfo->usb.curr_config = 0;
			result = ER_SYS;
			break;
	}
	return result;
}

/***************************************************************************
  Description: EP0 SET_DESCRIPTORProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_set_descriptor( DEVICE_REQUEST* pDeviceRequest )
{
	/* TODO: SET_DESCRIPTOR(Current statusSTALLReply) */
/*	return ER_NOTSUP; */
	return ER_SYS;
}

/***************************************************************************
  Description: EP0 SET_FEATUREProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_set_feature( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	UINT8	recipient;				/* Processing target		*/
	UINT8	direction;				/* I/O direction			*/
	UINT16	selector;				/* Selector value			*/
	UINT16	wIndex;					/* Index value		*/
	UINT	endpoint;				/* Endpoint number	*/
	ER_RET	result;					/* Result					*/
	UINT	num;					/* Number					*/
	UINT8	dir;					/* Direction					*/

	/* Initializing variables */
	recipient	= (UINT8)(pDeviceRequest->bmRequestType & USB_TARGET);
	direction	= (UINT8)(pDeviceRequest->bmRequestType & USB_DIR);
	selector	= pDeviceRequest->wValue;
	wIndex		= pDeviceRequest->wIndex;
	endpoint	= wIndex & 0xFF;
	result		= ER_SYS;

	/* Request content check */
	if( ( 0x0000 != pDeviceRequest->wLength ) ||
		( USB_DIR_OUT != direction ) )
	{
		return ER_SYS;
	}

	/* Perform processing for each processing target */
	switch( recipient )
	{
		case USB_TARGET_DEVICE:
			result = Set_Feature_Device( selector, wIndex, pUsbfInfo );
			break;
		case USB_TARGET_INTERFACE:
			result = ER_SYS;
			break;
		case USB_TARGET_ENDPOINT:
			/* Check index value */
			if ( ( 0x0000 != ( wIndex & 0xFF00 ) )   ||
				 ( 0x0000 != ( wIndex & 0x0070 ) ) )
			{
				return ER_SYS;
			}
			/* Selector value check */
			if( USB_FEATURE_ENDPOINT_HALT == selector )
			{
				/* Endpoint number check */
				if( ( 0x0000 == endpoint ) || ( 0x0080 == endpoint ) )
				{
					/* EP0 */
					/* Buffer empty check */
					if((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)
					{															/* When the buffer is not empty,			*/
						ARMPF_EP0_CONTROL |= D_EP0_BCLR;								/*  -> Empty the buffer.			*/
						while((ARMPF_EP0_STATUS & D_EP0_IN_EMPTY) != D_EP0_IN_EMPTY)	/*  -> Wait until it's actually empty.	*/
						{
							/* --- Forcibly terminate when cable is disconnected --- */
							if (UsbFuncCheckPlugInout() == U2F_PLUG_OUT)
							{
								return ER_SYS;
							}
						}
					}
				}
				else
				{
					/* Other EP */
					num = endpoint & ~(UINT)USB_DIR;
					dir = (UINT8)(endpoint & USB_DIR);

					/* Direction check */
					if( ER_OK != Check_USB_DIR_EPn( num, dir ) )
					{
						return ER_SYS;
					}

					if( ER_OK != Set_Feature_EP( num ) )
					{
						return ER_SYS;
					}
				}
				result = ER_OK;
			}
			else
			{
				return ER_SYS;
			}
			break;
		default:
			result = ER_SYS;
			break;
	}
	return result;
}

/***************************************************************************
  Description: EP0 SET_FEATUREProcessing( TARGET = DEVICE )
----------------------------------------------------------------------------
  Parameters:
	selector;			USB STD FEATURE Slectors
	wIndex;				Index value
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Set_Feature_Device( UINT16 selector, UINT16 wIndex, U2F_INFO* pUsbfInfo )
{
	ER_RET	result;					/* Result					*/
	UINT32	temp;					/* Work for setting editing		*/
	UINT32	tst_mode;				/* Work for setting editing		*/

	/* Initializing variables */
	result = ER_SYS;
	temp = 0;
	tst_mode = 0;

	switch( selector )
	{
		case USB_FEATURE_DEVICE_REMOTE_WAKEUP:
			if( 0x0000 == wIndex )
			{
				if( (UINT8)0 != ( (pUsbfInfo->desc.pConf_hs->conf_desc.bmAttributes) & (UINT8)BIT5 ) )
				{	/* BIT5:Remote Wakeup bit */
					/* Update of remote wakeup flag */
					pUsbfInfo->usb.flag_RemoteWakeup = U2F_ENABLE;
					result = ER_OK;
				}
				else
				{
					result = ER_SYS;
				}
			}
			else
			{
				return ER_SYS;
			}
			break;
		case USB_FEATURE_TST_MODE:
			if( wIndex <= 0x0005 )
			{
				temp     = ARMPF_USB_CONTROL;									/* CONTROLSave register	*/
				tst_mode = (UINT32)wIndex;								/* wIndexofUINT32fart			*/
				tst_mode = ( tst_mode << 16 ) & D_USB_USBTESTMODE;		/* TSTMODEbit	*/
				temp &= ~(UINT32)D_USB_USBTESTMODE;						/* TSTMODEClear the area once	*/
				temp |= tst_mode;										/* Setting value					*/

				/* Write to register */
				ARMPF_USB_CONTROL = temp;
				result = ER_OK;
			}
			else
			{
				result = ER_SYS;
			}
			break;
		case USB_FEATURE_ENDPOINT_HALT:
			result = ER_SYS;
			break;
		default:
			result = ER_SYS;
			break;
	}
	return result;
}

/***************************************************************************
  Description: EP0 SET_FEATUREProcessing( TARGET = EP )
----------------------------------------------------------------------------
  Parameters:
	num;				EPNumber
  Return:
	ER_OK; 				Success
	ER_PARAM;			Parameter error
	ER_SYS;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Set_Feature_EP( UINT num )
{
	/* ParameterConfirm */
	if(( num < 1 ) || ( num > 9))
	{
		return ER_PARAM;
	}

	/* EP EnableConfirm */
	if ( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_EN) != D_EPn_EN)
	{
		return ER_SYS;
	}

	/* EPn Clear Feature */
	if( ( udev_get_DeviceState() & UDEV_ST_CONFIGURED ) != 0 )
	{
		/* EP STALLSetting */
		UsbFuncSetStallEPn( num );
	}
	else
	{
		return ER_SYS;
	}

	return ER_OK;
}

/***************************************************************************
  Description: EP0 SET_INTERFACEProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
	pUsbfInfo;			USBPointer to device information area
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_set_interface( DEVICE_REQUEST* pDeviceRequest, U2F_INFO* pUsbfInfo )
{
	INTERFACE_DESCRIPTOR*	pIfDesc;	/* INTERFACEDescriptor start address	*/

	/* Initializing variables */
	pIfDesc = NULL;

	/* Request content check */
	if( ( 0x0000      != pDeviceRequest->wLength )	||
	    ( 0x0000      != pDeviceRequest->wValue  )	||			/* Alternate SettingBecause it is not supported,wValue, it flips. */
	    ( (UINT8)0x01 != pDeviceRequest->bmRequestType  ) )
	{
		return ER_SYS;
	}

	/* Configuration check */
	if( UDEV_ST_CONFIGURED == udev_get_DeviceState() )
	{
		pIfDesc = info_find_InterfaceDesc( pDeviceRequest->wIndex, pUsbfInfo );
		if( NULL == pIfDesc )
		{
			return ER_SYS;
		}
	}
	else
	{
		return ER_SYS;
	}

	/*-------------------------------------------------------*/
	/* specifiedInterfaceWhen configuring the device by number	 */
	/* Add below from here										 */
	/*-------------------------------------------------------*/
	/* pIfDesc                = Interface descriptor	*/
	/* pDeviceRequest->wIndex = Interface number			*/
	/* pDeviceRequest->wValue = Alternative setting value						*/

	return ER_OK;
}

/***************************************************************************
  Description: EP0 SYNCH_FRAMEProcessing
----------------------------------------------------------------------------
  Parameters:
	pDeviceRequest;		Device request
  Return:
	ER_OK; 				Success
	Others;				Failure
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET std_req_synch_frame( DEVICE_REQUEST* pDeviceRequest )
{
	/* TODO: SYNCH_FRAME(Current statusSTALLReply) */
/*	return ER_NOTSUP; */
	return ER_SYS;
}

/***************************************************************************
  Description: EPDirectionUSB_DIR_OUT/INConversion processing
----------------------------------------------------------------------------
  Parameters:
	num;				EPNumber
	direction;			Comparison direction
  Return:
	ER_OK; 				direction
	ER_SYS;				directionDifference from error
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static ER_RET Check_USB_DIR_EPn( UINT num, UINT8 direction )
{
	UINT8	dir;					/* Workpiece for direction check	*/

	/* EPnNumber confirmation */
	if(( num < 1 ) || ( num > 9))
	{
		return ER_PARAM;
	}

	/* Direction check */
	if( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_DIR0) == D_EPn_DIR0 )
	{	/* OUT */
		dir = USB_DIR_OUT;
	}
	else
	{	/* IN */
		dir = USB_DIR_IN;
	}

	/* directionfor comparison */
	if( dir != direction )
	{
		return ER_SYS;
	}

	return ER_OK;
}

/***************************************************************************
  Description: Device state acquisition processing
----------------------------------------------------------------------------
  Parameters:
	none
  Return:
	UDEV_ST_IDLE; 		Idle state
	UDEV_ST_DEFAULT;	Default state
	UDEV_ST_ADDRESS;	Address state
	UDEV_ST_CONFIGURED;	Configured state
	UDEV_ST_SUSPENDED;	Suspend state
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static UINT32 udev_get_DeviceState( void )
{
	UINT32	epctr;					/* EPCPeripheral control register value	*/
	UINT32	state;					/* USBStatus register value	*/

	/* EPCGet peripheral control register value */
	epctr	= ARMPF_EPCTR;

	if( 0 == (epctr & D_SYS_VBUS_LEVEL ) )
	{
		/* Idle(Attached)State */
		return UDEV_ST_IDLE;
	}

	if( 0 == (epctr & D_SYS_PLL_LOCK ) )
	{
		/* SuspendState */
		return UDEV_ST_SUSPENDED;
	}

	/* USBStatus register value acquisition */
	state = ARMPF_USB_STATUS;

	if( 0 == ( state & ( D_USB_CONF_ST | D_USB_DEFAULT ) ) )
	{
		return UDEV_ST_IDLE;
	}

	if( ( state & D_USB_SPND_OUT ) != 0 )
	{
		/* SuspendState */
		return UDEV_ST_SUSPENDED;
	}

	if( ( state & D_USB_CONF_ST ) != 0 )
	{
		/* ConfiguredState */
		return UDEV_ST_CONFIGURED;
	}
	else
	{
		if( (ARMPF_USB_ADDRESS & D_USB_USB_ADDR) == D_USB_USB_ADDR )
		{
			/* Address state */
			return UDEV_ST_ADDRESS;
		}
		else
		{
			if( ( state & D_USB_DEFAULT ) != 0 )
			{
				/* DefaultState */
				return UDEV_ST_DEFAULT;
			}
		}
	}

	return UDEV_ST_IDLE;
}

/***************************************************************************
  Description: info Current configuration descriptor acquisition process
----------------------------------------------------------------------------
  Parameters:
	pUsbfInfo;			USBPointer to device information area
  Return:
	Current configuration descriptor
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static DESC_CONFIG* info_get_CurrConfigDesc( U2F_INFO* pUsbfInfo )
{
	DESC_CONFIG*	pConfigDesc;	/* CONFIGURATIONDescriptor start address	*/

	/* Initializing variables */
	pConfigDesc = NULL;

	if( pUsbfInfo->usb.curr_config != 0 )
	{
		if( (ARMPF_USB_STATUS & D_USB_SPEED_MODE) == D_USB_SPEED_MODE )
		{	/* High Speed */
			pConfigDesc = pUsbfInfo->desc.pConf_hs;
		}
		else
		{	/* Full Speed */
			pConfigDesc = pUsbfInfo->desc.pConf_fs;
		}
	}
	else
	{
		/* No processing */
	}
	return pConfigDesc;
}

/***************************************************************************
  Description: info Interface descriptor acquisition process
----------------------------------------------------------------------------
  Parameters:
	index;				Interface number
	pUsbfInfo;			USBPointer to device information area
  Return:
	Interface descriptor
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static INTERFACE_DESCRIPTOR* info_find_InterfaceDesc( UINT16 index, U2F_INFO* pUsbfInfo )
{
	DESC_CONFIG*	pConfigDesc;	/* CONFIGURATIONDescriptor start address	*/
	UINT			NumIf;			/* 	*/

	/* Initializing variables */
	pConfigDesc = NULL;

	/* Interface number check */
	if( U2F_IF_MAX <= index )
	{
		return NULL;
	}

	pConfigDesc = info_get_CurrConfigDesc( pUsbfInfo );
	if( NULL == pConfigDesc )
	{
		return NULL;
	}

	/* Interface search */
	for( NumIf = 0; NumIf <= (UINT)(pConfigDesc->conf_desc.bNumInterfaces); NumIf++ )
	{
		/* Check if it matches the specified interface number */
		if( index == (UINT16)(pConfigDesc->if_desc[NumIf].bInterfaceNumber) )
		{
			/* found InterfaceDescriptor */
			return (INTERFACE_DESCRIPTOR*)(&pConfigDesc->if_desc[NumIf]);
		}
	}

	return NULL;	/* Not Found. */
}

/***************************************************************************
  Description: EPn STALLState acquisition processing
----------------------------------------------------------------------------
  Parameters:
	num;				EPNumber
  Return:
	0;					STALLnot
	1;					STALLis
	ER_PARAM;			Parameter error
----------------------------------------------------------------------------
  Note:
	none
****************************************************************************/
static INT EPn_get_STALL( UINT num )
{
	UINT32 status;

	/* ParameterConfirm */
	if(( num < 1 ) || ( num > 9))
	{
		return ER_PARAM;
	}

	/* EP EnableConfirm */
	status = *((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20));
	if( ( status & D_EPn_EN ) == 0 )
	{
		return ER_PARAM;
	}

	/* Halt(STALL) Confirmation */
	if( (*((VUINT32*)(EPn_CONTROL_ADR + (num - 1) * 0x20)) & D_EPn_DIR0) == D_EPn_DIR0 )
	{	/* OUT */
		/* STALLCheck */
		if( ( status & ( D_EPn_OSTL ) ) != 0 )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{	/* IN */
		/* STALLCheck */
		if( ( status & ( D_EPn_ISTL ) ) != 0 )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
}

/***************************************** end of file **********************************************/

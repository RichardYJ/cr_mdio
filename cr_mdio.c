/*
 * Copyright (c) 2014 Credo Semiconductor Incorporated 
 * All Rights Reserved
 *                                                                         
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of Credo Semiconductor Inc.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *                                                                         
 * No part of this code may be reproduced, stored in a retrieval system,
 * or transmitted, in any form or by any means, electronic, mechanical,
 * photocopying, recording, or otherwise, without the prior written
 * permission of Credo Semiconductor Inc.
 */

#ifdef _DEBUG
#ifdef _MSC_VER
/* Thanks Microsoft, I know what I am doing */
#pragma warning(disable:4996)
#endif
#endif
#include <stdio.h>
#include "cr_type.h"
#include "cr_mdio_api.h"
//#include "cr_api/cr_debug.h"
//#include "cr_api/cr_err.h"
uint16_t dwNumBytesToSend=0;
unsigned char OutputBuffer[1024];

#if 1
#define LOG_ERROR printf 
#define LOG_DEBUG printf
#define printf(...) ;

#pragma comment (lib, "ftd2xx.lib")
#endif

static const char *ftd2xx_status_string(FT_STATUS status)
{
    switch (status) {
        case FT_OK:                             return "OK";
        case FT_INVALID_HANDLE:                 return "invalid handle";
        case FT_DEVICE_NOT_FOUND:               return "device not found";
        case FT_DEVICE_NOT_OPENED:              return "device not opened";
        case FT_IO_ERROR:                       return "io error";
        case FT_INSUFFICIENT_RESOURCES:         return "insufficient resources";
        case FT_INVALID_PARAMETER:              return "invalid parameter";
        case FT_INVALID_BAUD_RATE:              return "invalid baud rate";

        case FT_DEVICE_NOT_OPENED_FOR_ERASE:    return "device not opened for erase";
        case FT_DEVICE_NOT_OPENED_FOR_WRITE:    return "device not opened for write";
        case FT_FAILED_TO_WRITE_DEVICE:         return "failed to write device";
        case FT_EEPROM_READ_FAILED:             return "eeprom read failed";
        case FT_EEPROM_WRITE_FAILED:            return "eeprom write failed";
        case FT_EEPROM_ERASE_FAILED:            return "eeprom erase failed";
        case FT_EEPROM_NOT_PRESENT:             return "eeprom not present";
        case FT_EEPROM_NOT_PROGRAMMED:          return "eeprom not programmed";
        case FT_INVALID_ARGS:                   return "invalid args";
        case FT_NOT_SUPPORTED:                  return "not supported";
        case FT_OTHER_ERROR:                    return "other error";
    }

    return "undefined FTD2xx error";
}

static FT_HANDLE ftdih;
static CR_PHY_DEV phy_dev;
static int port;

static CR_STATUS cr_mdio_init_ftd2xx(int sel)
{
    FT_STATUS status;
    DWORD dw_bytes_read;
    DWORD nrecv;
    unsigned char buf[32];

    status = FT_Open(sel, &ftdih);
    if (status != FT_OK) {
        LOG_ERROR("unable to open ftdi device0\n");
    	return CR_ERR_FAIL;
    }

    /* Reset USB device */
    status = FT_ResetDevice(ftdih); 
    if (status != FT_OK) {
        LOG_ERROR("failed to reset ftdi device\n");
    	return CR_ERR_FAIL;
    }
    
    /* Purge USB receive buffer first by reading out all old data from 
     * FT2232H receive buffer */

    /* Get the number of bytes in the FT2232H */
    dw_bytes_read = 0;
    status = FT_GetQueueStatus(ftdih, &dw_bytes_read); 
    if (status != FT_OK) {
        LOG_ERROR("failed to FT_GetQueueStatus\n");
    	return CR_ERR_FAIL;
    }

    /* Read out the data from FT2232H receive buffer */
    if ((status == FT_OK) && (dw_bytes_read > 0)) {
        FT_Read(ftdih, &buf, dw_bytes_read, &nrecv); 
        if (status != FT_OK) {
            LOG_ERROR("unable to set timeouts: %s\n",
                ftd2xx_status_string(status));
            return CR_ERR_FAIL;
        }
    }

    status = FT_SetTimeouts(ftdih, 100, 5000);
    if (status != FT_OK) {
        LOG_ERROR("unable to set timeouts: %s\n",
            ftd2xx_status_string(status));
        return CR_ERR_FAIL;
    }

    status = FT_SetBitMode(ftdih, 0x0b, 2);
    if (status != FT_OK) {
        LOG_ERROR("unable to enable bit i/o mode: %s\n",
            ftd2xx_status_string(status));
        return CR_ERR_FAIL;
    }

    MSLEEP(5);

    return CR_ERR_OK;
}

/* config ftdi device for mdio using*/
CR_STATUS cr_mdio_config(void)
{
    FT_STATUS status;
    /* 004A:400khz,SCL Frequency = 60/((1+0x004A)*2) (MHz) = 400khz   //128:100KHz */
    DWORD dw_clk_div_mdio = 0x001D; /* TODO: should be a config option 0x1d     SCL时钟速率的配置字段*/
	//DWORD dw_clk_div_mdio = 0x02ed;
	DWORD dw_bytes_send;
    DWORD dw_bytes_sent;
    unsigned char buf[32];
	uint8_t phaseClock = 0x8c;

    dw_bytes_send = 0;
    /* Ensure disable clock divide by 5 for 60Mhz master clock */
    buf[dw_bytes_send++] = (unsigned char)0x8A; 
    /* Ensure turn off adaptive clocking */
    buf[dw_bytes_send++] = (unsigned char)0x97; 
    /* 0x8D/0x8C Disable/Enable 3 phase data clock, used by I2C to allow data on both clock edges */
    buf[dw_bytes_send++] = (unsigned char)0x8D;  //0X8D
    
    /* Send off the commands */
    status = FT_Write(ftdih, buf, dw_bytes_send, &dw_bytes_sent); 
    if (status != FT_OK) {
        LOG_ERROR("failed to FT_Write: %s\n",
            ftd2xx_status_string(status));
        return CR_ERR_FAIL;
    }

    /* Clear output buffer */
    dw_bytes_send = 0;

    MSLEEP(5);

    /* Command to set directions of lower 8 pins and force value on bits set as output */
    buf[dw_bytes_send++] = (unsigned char)0x80; //command to set lower 8 pins
    /* Set SK,DO,GPIOL0 pins as output with bit 隆炉1隆炉, other pins as input with bit 隆庐0隆炉 */
    buf[dw_bytes_send++] = (unsigned char)0xF3; // set value of IO

    /* Set SDA, SCL high, WP disabled by SK, DO at bit 隆庐1隆炉, GPIOL0 at bit 隆庐0隆炉 */
    buf[dw_bytes_send++] = (unsigned char)0xF3; //set direction, 1=out, 0=in, MDC out  MOSI out MISO in

    /* The SK clock frequency can be worked out by below algorithm with divide by 5 set as off */
    /* SK frequency  = 60MHz /((1 +  [(1 +0xValueH*256) OR 0xValueL])*2) */
    /* Command to set clock divisor */
    buf[dw_bytes_send++] = (unsigned char)0x86; 
    /* Set 0xValueL of clock divisor */
    buf[dw_bytes_send++] = (unsigned char)(dw_clk_div_mdio & 0xFF); 
    /* Set 0xValueH of clock divisor */
    buf[dw_bytes_send++] = (unsigned char)((dw_clk_div_mdio >> 8) & 0xFF); 
    /* Send off the commands */
    status = FT_Write(ftdih, buf, dw_bytes_send, &dw_bytes_sent); 
    if (status != FT_OK) {
        LOG_ERROR("failed to FT_Write: %s\n",
            ftd2xx_status_string(status));
        return CR_ERR_FAIL;
    }

    /* Delay for a while */
    MSLEEP(5);

    dw_bytes_send = 0;
    /* 0x8C Enable 3 phase clocking */
    buf[dw_bytes_send++] = (unsigned char)phaseClock; //Disable
    LOG_DEBUG("cr_mdio_config:3 phase 0x%02x\n",phaseClock);
    /* Send off the commands */
    status = FT_Write(ftdih, buf, dw_bytes_send, &dw_bytes_sent);
    if (status != FT_OK) {
        LOG_ERROR("failed to FT_Write: %s\n",
            ftd2xx_status_string(status));
        return CR_ERR_FAIL;
    }
    MSLEEP(5);
    return CR_ERR_OK;
}

CR_STATUS cr_write_mdio(CR_U16 reg_addr, CR_U16 reg_val)
{
    FT_STATUS status;
    DWORD i;
    unsigned char buf[32];
    DWORD len;
    DWORD nsent;
    DWORD nsend;

#ifdef _DEBUG
    if (reg_addr != 0)
    {
        FILE *fp;
        fp = fopen("script.txt", "a+");
        fprintf(fp, "%04x  %04x\n", reg_addr, reg_val);
        fclose(fp);
    }
#endif

    nsend = 16;
    len = nsend+3+1;
    for (i = 0;i < len;i++) {
        buf[i] = (unsigned char)0xff;
    }
    
    LOG_DEBUG("cr_mdio write[%04x]%04x\n", reg_addr, reg_val);
    /* clock data byte in, MSB first */
    buf[0] = (unsigned char)0x10;
    /* length low */
    buf[1] = nsend - 1;
    /* length high */
    buf[2] = (unsigned char)0x00;
    buf[ 4+3] = (unsigned char)0x0f & (phy_dev.phy_addr >> 1);
    buf[ 5+3] = (phy_dev.phy_addr << 7) | (phy_dev.dev_addr << 2) | 0x02;
    buf[ 6+3] = (unsigned char)(reg_addr >> 8);
    buf[ 7+3] = (unsigned char)reg_addr;
    buf[12+3] = (phy_dev.phy_addr >> 1) | 0x10;
    buf[13+3] = (phy_dev.phy_addr << 7) | (phy_dev.dev_addr << 2) | 0x02;
    buf[14+3] = (unsigned char)(reg_val >> 8);
    buf[15+3] = (unsigned char)reg_val;
    buf[16+3] = (unsigned char)0x87;	//0x7f
//    buf[20] = (unsigned char)0x87; 

    status = FT_Write(ftdih, buf, len, &nsent);
    if (status != FT_OK) {
        LOG_ERROR("unable to FT_Write: %s\n",
            ftd2xx_status_string(status));
        return CR_ERR_FAIL;
    }

    return CR_ERR_OK;
}

CR_U16 cr_read_mdio(CR_U16 reg_addr)
{
    DWORD dw_bytes_read;
    DWORD tot_bytes_read;
    FT_STATUS status;
    unsigned char buf[32];
    DWORD i;
    DWORD len;
    DWORD nsend;
    DWORD nsent;
    int timeout;

    nsend = 16;  //reduce 17 to 14,for changing read mode
    len = nsend+3+1;
    for (i = 0;i < len;i++) {
        buf[i] = (unsigned char)0xff;
    }
#define MSBFST_IPOP

#ifdef MSBFST_IPOP
    buf[0] = (unsigned char)0x30; //byte in +, byte out +, MSB first  0x34	//高有效位在前的OPCODE
	LOG_DEBUG("cr_read_mdio MSBFST_IPOP  0x30\n");
#elif defined MSBFST_IPON
	buf[0] = (unsigned char)0x31; 
	LOG_DEBUG("cr_read_mdio MSBFST_IPON  0x31\n");
#else
	LOG_DEBUG("cr_read_mdio				 0x34\n");
	buf[0] = (unsigned char)0x34; //byte in +, byte out +, MSB first  0x34	//高有效位在前的OPCODE
#endif
    buf[1] = nsend - 1; //Length L											//要传输的字节数的低字节
    buf[2] = (unsigned char)0x00; //Length H							//要传输的字节数的高字节
    buf[ 4+3] = (unsigned char)0x0f & (phy_dev.phy_addr >> 1);			//此地址放phy addr的高7位	0x00
    buf[ 5+3] = (phy_dev.phy_addr << 7) | (phy_dev.dev_addr << 2) | 0x02;//此地址放phy addr和dev addr的合成，是0x06
    buf[ 6+3] = (unsigned char)(reg_addr >> 8);							//寄存器地址的高8位	0x00
    buf[ 7+3] = (unsigned char)reg_addr;								//寄存器地址的低8位	0x00
    buf[12+3] = (phy_dev.phy_addr >> 1) | 0x30;								//0x30
    buf[13+3] = (phy_dev.phy_addr << 7) | (phy_dev.dev_addr << 2) | 0x02;	//0x06
    buf[14+3] = (unsigned char)0xff;										
    buf[15+3] = (unsigned char)0xff;
    buf[16+3] = (unsigned char)0x87;				//0x7f
    /* Send answer back immediate command */
//    buf[20] = (unsigned char)0x87; 

    status = FT_Write(ftdih, buf, len, &nsent);
    if (status != FT_OK) {
        LOG_ERROR("unable to FT_Write: %s\n",
            ftd2xx_status_string(status));
        return CR_ERR_FAIL;
    }


    tot_bytes_read = 0;
    timeout = 5;
    while (tot_bytes_read < nsend && timeout--) {
        status = FT_Read(ftdih, &buf[tot_bytes_read], nsend - tot_bytes_read, &dw_bytes_read);
        if (status != FT_OK) {
            LOG_ERROR("FT_Read returned: %s\n", ftd2xx_status_string(status));
            return CR_ERR_FAIL;
        }
        tot_bytes_read += dw_bytes_read;
    }

    if (tot_bytes_read < nsend) {
        LOG_ERROR("couldn't read enough bytes from "
            "ftd2xx device (%i < %i)\n",
            (unsigned)tot_bytes_read,
            (unsigned)nsend);
        return CR_ERR_FAIL;
    }
    
    LOG_DEBUG("cr_mdio read [%04x]=%04x\n", reg_addr, (buf[14] << 8) | buf[15]);
#ifdef MSBFST_IPOP
	return ( (buf[13]&0x01)<<7|buf[14] >>1 )<<8 | ( (buf[14]&0x01)<<7|buf[15]>>1 );
#else
	return (buf[14] << 8) | buf[15];
#endif
}


static CR_STATUS cr_mdio_purge_ftd2xx(void)
{
    FT_STATUS status;

    status = FT_Purge(ftdih, FT_PURGE_RX | FT_PURGE_TX);
    if (status != FT_OK) {
        LOG_ERROR("error purging ftd2xx device: %s\n",
            ftd2xx_status_string(status));
        return CR_ERR_FAIL;
    }

    return CR_ERR_OK;
}

CR_STATUS cr_mdio_init(CR_U8 phy_addr, CR_U8 dev_addr, int sel, int mdio) {
    CR_STATUS status;
    
    LOG_DEBUG("cr_mdio init.\n");

    phy_dev.phy_addr = phy_addr;
    phy_dev.dev_addr = dev_addr;
	port = mdio;

    status = cr_mdio_init_ftd2xx(sel);
    if (status != CR_ERR_OK) 
        return status;
    
    status = cr_mdio_config();
    if (status != CR_ERR_OK) 
        return status;
    
    status = cr_mdio_purge_ftd2xx();
    if (status != CR_ERR_OK) 
        return status;

    MSLEEP(10);

    return CR_ERR_OK;
}

CR_STATUS cr_mdio_close(void)
{
    FT_STATUS status;

    LOG_DEBUG("cr_mdio close.\n");
    
    status = FT_Close(ftdih);
    if (status != FT_OK) {
        LOG_ERROR("Failed to close ftd2xx device!\n");
        return CR_ERR_FAIL;
    }

    return CR_ERR_OK;
}

void FT2232I2cStart(void)
{
	uint32_t dwCount;
//	uint16_t dwNumBytesToSend = 0;
//	unsigned char OutputBuffer[64];

	//printf("Redirect to FT2232I2cStart function.\n");
	//printf("Write 0x80, 0xF3, 0xF3\n");
	for (dwCount = 0; dwCount < 4; dwCount++) // Repeat commands to ensure the minimum period of the start hold time ie 800ns is achieved
	{
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xF3; //Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xF3; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}
	
	//printf("Write 0x80, 0xF1, 0xFB\n");
	for (dwCount = 0; dwCount < 4; dwCount++) // Repeat commands to ensure the minimum period of the start setup time ie 800ns is achieved
	{
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xF1; //Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}

    //printf("write 0x80, 0xf1, 0xfb\n");
	for (dwCount = 0; dwCount < 4; dwCount++) // Repeat commands to ensure the minimum period of the start setup time ie 800ns is achieved
	{
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xF1; //Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}

    //printf("Write 0x80, 0xF1, 0xFB\n");

	for (dwCount = 0; dwCount < 4; dwCount++) // Repeat commands to ensure the minimum period of the start setup time ie 800ns is achieved
	{
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xF1; //Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}

	//printf("Write 0x80, 0x00, 0xFB\n");
	OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
	//printf("Write 0x00\n");
	OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0x00; //Set SDA, SCL low, WP disabled by SK, DO, GPIOL0 at bit 0
	//printf("Write 0xFB\n");
	OutputBuffer[dwNumBytesToSend++] =(unsigned char) 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	//printf("Done! Back to read function.\n");
}


void FT2232I2cStop(void)
{
	uint32_t dwCount;
	//uint16_t dwNumBytesToSend=0;
	//unsigned char OutputBuffer[64];

	for (dwCount = 0; dwCount<4; dwCount++)
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] = 0xF0; //Set SDA low, SCL low, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] = 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}

	for (dwCount = 0; dwCount<4; dwCount++) // Repeat commands to ensure the minimum period of the stop setup time ie 800ns is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] = 0xF1; //Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] = 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}

	for (dwCount = 0; dwCount<4; dwCount++) // Repeat commands to ensure the minimum period of the stop setup time ie 800ns is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] = 0xF1; //Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] = 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}

	for (dwCount = 0; dwCount<4; dwCount++) // Repeat commands to ensure the minimum period of the stop hold time ie 800ns is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] = 0xF3; //Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] = 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}
	for (dwCount = 0; dwCount<4; dwCount++) // Repeat commands to ensure the minimum period of the stop hold time ie 800ns is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] = 0xF3; //Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] = 0xFB; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}
}


uint8_t   FT2232I2cSendByteAndCheckACK(uint8_t  dwDataSend)
{
	FT_STATUS ftStatus = FT_OK;
	//uint16_t dwNumBytesToSend = 0;
	DWORD dwNumBytesSent=0, dwNumBytesRead=0;
	//unsigned char OutputBuffer[32];
	unsigned char InputBuffer[32];

	OutputBuffer[dwNumBytesToSend++] = '\x11'; //MSB_FALLING_EDGE_CLOCK_BYTE_OUT; //Clock data byte out on 鈥搗e Clock Edge MSB first
	OutputBuffer[dwNumBytesToSend++] = '\x00';
	OutputBuffer[dwNumBytesToSend++] = '\x00'; //Data length of 0x0000 means 1 byte data to clock out(8 bit)
	OutputBuffer[dwNumBytesToSend++] = dwDataSend; //Add data to be send

												   //Get Acknowledge bit from SLAVE
	OutputBuffer[dwNumBytesToSend++] = '\x80'; //Command to set directions of lower 8 pins and force value on bits set as output
	OutputBuffer[dwNumBytesToSend++] = '\x00'; //Set SCL low, WP disabled by SK, GPIOL0 at bit 0
	OutputBuffer[dwNumBytesToSend++] = '\x11'; //Set SK, GPIOL0 pins as output with bit 1, DO and other pins as input with bit 0
	OutputBuffer[dwNumBytesToSend++] = '\x22'; //MSB_RISING_EDGE_CLOCK_BIT_IN; //Command to scan in ACK bit , -ve clock Edge MSB first
	OutputBuffer[dwNumBytesToSend++] = '\x0'; //Length of 0x0 means to scan in 1 bit

	OutputBuffer[dwNumBytesToSend++] = '\x87'; //Send answer back immediate command

	ftStatus = FT_Write(ftdih, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent); //Send off the commands
	dwNumBytesToSend = 0; //Clear output buffer

						  //Check if ACK bit received, may need to read more times to get ACK bit or fail if timeout
	ftStatus = FT_Read(ftdih, InputBuffer, 1, &dwNumBytesRead); //Read one byte from device receive buffer

	if ((ftStatus != FT_OK) || (dwNumBytesRead == 0))
	{
		return FALSE; /*Error, can't get the ACK bit from SLAVE*/
	}
	else if (((InputBuffer[0] & (unsigned char)('\x1')) != (unsigned char)('\x0'))) //Check ACK bit 0 on data byte read out
	{
		return FALSE; /*Error, can't get the ACK bit from SLAVE */
	}

	OutputBuffer[dwNumBytesToSend++] = '\x80'; //Command to set directions of lower 8 pins and force value on bits set as output
	OutputBuffer[dwNumBytesToSend++] = '\x02'; //Set SDA high, SCL low, WP disabled by SK at bit 0, DO, GPIOL0 at bit 1
	OutputBuffer[dwNumBytesToSend++] = '\x13'; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	return TRUE;
}

uint8_t FT2232I2cReceiveByte(void)
{
	//uint16_t dwNumBytesToSend = 0;
	DWORD dwNumBytesSent = 0, dwNumBytesRead = 0;
	FT_STATUS ftStatus;
	uint32_t dwCount;
	//unsigned char OutputBuffer[128];
	unsigned char InputBuffer[32];
	unsigned char ByteDataRead;
	//Get Acknowledge bit from SLAVE
	OutputBuffer[dwNumBytesToSend++] = '\x80'; //Command to set directions of lower 8 pins and force value on bits set as output
	OutputBuffer[dwNumBytesToSend++] = '\x00';
	OutputBuffer[dwNumBytesToSend++] = '\x11';

	OutputBuffer[dwNumBytesToSend++] = '\x20';  //MSB_UP_EDGE_CLOCK_BYTE_IN; //Command to clock data byte in on 鈥搗e Clock Edge MSB first
	OutputBuffer[dwNumBytesToSend++] = '\x00';
	OutputBuffer[dwNumBytesToSend++] = '\x00'; //Data length of 0x0000 means 1 byte data to clock in
	OutputBuffer[dwNumBytesToSend++] = '\x87'; //Send answer back immediate command

											   //send ack
	for (dwCount = 0; dwCount<12; dwCount++) // Repeat commands to ensure the minimum period of the stop hold time ie 2.5us is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] = 0x00; //Set SDA low, SCL low, WP disabled by SK at bit 0, DO, GPIOL0 at bit 1
		OutputBuffer[dwNumBytesToSend++] = 0x13; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}
	for (dwCount = 0; dwCount<12; dwCount++) // Repeat commands to ensure the minimum period of the stop setup time ie 2.5us is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] = 0x01; //Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		OutputBuffer[dwNumBytesToSend++] = 0x13; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
	}
	//send ack

	ftStatus = FT_Write(ftdih, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent); //Send off the commands
	dwNumBytesToSend = 0; //Clear output buffer

						  //Read two bytes from device receive buffer, first byte is data read from SLAVE, second byte is ACK bit
	ftStatus = FT_Read(ftdih, InputBuffer, 1, &dwNumBytesRead);
	ByteDataRead = InputBuffer[0]; //Return the data read from SLAVE
	if ((ftStatus != FT_OK) || (dwNumBytesRead == 0))
	{
		return 0; /*Error*/
	}
	return ByteDataRead;
}

void cr_write_i2c(CR_U16 RegAddr, CR_U16 Value)
{
	unsigned int  bSucceed = TRUE;
	FT2232I2cStart(); //Set START condition for I2C communication
	//printf("***************** dev_addr = %#x, RegAddr = %#x, Value = %#4x", phy_dev.dev_addr, RegAddr, Value);
	bSucceed = FT2232I2cSendByteAndCheckACK(phy_dev.phy_addr << 1); //Send 7 bit Device address byte and check if ACK bit is received
	bSucceed = FT2232I2cSendByteAndCheckACK(RegAddr >> 8);   //Send Reg addressH byte and check if ACK bit is received
	bSucceed = FT2232I2cSendByteAndCheckACK(RegAddr & 0xFF);   //Send Reg address L byte and check if ACK bit is received
	bSucceed = FT2232I2cSendByteAndCheckACK(Value >> 8);  //Write reg[0x02]
	bSucceed = FT2232I2cSendByteAndCheckACK(Value & 0xFF);       //Write reg[0x03]
	FT2232I2cStop(); //Set STOP condition for I2C communication
	dwNumBytesToSend = 0; //Clear output buffer

}

CR_U16 cr_read_i2c(CR_U16 RegAddr)
{
	uint16_t Value = 0;
	uint8_t ValueH = 0, ValueL = 0;

	BOOL bSucceed = TRUE;
	DWORD dwNumBytesRead = 0, dwNumInputBuffer = 0;
	FT_STATUS ftStatus;
	unsigned char InputBuffer[32];

	//Purge USB receive buffer first before read operation
	ftStatus = FT_GetQueueStatus(ftdih, &dwNumInputBuffer); // Get the number of bytes in the device receive buffer
	//printf("step: FT_GetQueueStatus\n");
	//printf("ftStatus is %d", ftStatus);
	if ((ftStatus == FT_OK) && (dwNumInputBuffer > 0))
		FT_Read(ftdih, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out all the data from receive buffer
	//printf("FT3323I2c Start\n");
	FT2232I2cStart();
	//printf("step: check ACK\n");
	//printf("**************dev_addr is %d\n", phy_dev.dev_addr);
	bSucceed = FT2232I2cSendByteAndCheckACK(phy_dev.phy_addr << 1); //Send 7 bit Device address byte and check if ACK bit is received
	//printf("************* RegAddr is %#x\n", RegAddr);
	bSucceed = FT2232I2cSendByteAndCheckACK(RegAddr >> 8);   //Send Reg addressH byte and check if ACK bit is received
	bSucceed = FT2232I2cSendByteAndCheckACK(RegAddr & 0xFF);   //Send Reg address L byte and check if ACK bit is received
	//printf("Step: stop");
	FT2232I2cStop();
	FT2232I2cStart(); //Set START condition for I2C communication

	bSucceed = FT2232I2cSendByteAndCheckACK((phy_dev.phy_addr << 1) + 0x01); //Send 7 bit Device address byte and check if ACK bit is received
	ValueH = FT2232I2cReceiveByte();
	ValueL = FT2232I2cReceiveByte();
	Value = ((unsigned short)ValueH) << 8;
	Value += (unsigned short)ValueL;

	FT2232I2cStop(); //Set STOP condition for I2C communication
	dwNumBytesToSend = 0; //Clear output buffer

	return Value;
}


CR_U8 cr_8bits_read_i2c(CR_U8 RegAddr)
{
	uint16_t Value = 0;
	uint8_t ValueH = 0, ValueL = 0;

	BOOL bSucceed = TRUE;
	DWORD dwNumBytesRead = 0, dwNumInputBuffer = 0;
	FT_STATUS ftStatus;
	unsigned char InputBuffer[32];
	
//	phy_dev.phy_addr = 0x50;		//yj added for test
//	printf("TRUE is: %d\n",TRUE);
	
	//Purge USB receive buffer first before read operation
	ftStatus = FT_GetQueueStatus(ftdih, &dwNumInputBuffer); // Get the number of bytes in the device receive buffer
	//printf("step: FT_GetQueueStatus\n");
	//printf("ftStatus is %d", ftStatus);
	if ((ftStatus == FT_OK) && (dwNumInputBuffer > 0))
		FT_Read(ftdih, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out all the data from receive buffer
	//printf("FT3323I2c Start\n");
	FT2232I2cStart();
	//printf("step: check ACK\n");
	//printf("**************dev_addr is %d\n", phy_dev.dev_addr);
	bSucceed = FT2232I2cSendByteAndCheckACK(phy_dev.phy_addr/*0x50*/ << 1); //Send 7 bit Device address byte and check if ACK bit is received
	printf("Write phy_dev.phy_addr(0x%x) is: %d\n", phy_dev.phy_addr, bSucceed);
	//printf("************* RegAddr is %#x\n", RegAddr);
	bSucceed = FT2232I2cSendByteAndCheckACK(RegAddr);   //Send Reg addressH byte and check if ACK bit is received
	printf("Write RegAddr(0x%x) is: %d\n",RegAddr,bSucceed);

	//bSucceed = FT2232I2cSendByteAndCheckACK(RegAddr);   //Send Reg address L byte and check if ACK bit is received
	//printf("Step: stop");
	FT2232I2cStop();
	FT2232I2cStart(); //Set START condition for I2C communication

	bSucceed = FT2232I2cSendByteAndCheckACK((phy_dev.phy_addr << 1) + 0x01); //Send 7 bit Device address byte and check if ACK bit is received
	printf("Write phy_dev.phy_addr(0x%x) is: %d\n",phy_dev.phy_addr,bSucceed);
	//ValueH = FT2232I2cReceiveByte();
	//ValueL = FT2232I2cReceiveByte();
	Value = FT2232I2cReceiveByte();
	printf("Read  value is: 0x%x\n",Value);
	
	//Value += (unsigned short)ValueL;

	FT2232I2cStop(); //Set STOP condition for I2C communication
	dwNumBytesToSend = 0; //Clear output buffer

	return Value;
}


void cr_8bits_write_i2c(CR_U8 RegAddr, CR_U8 Value)
{
	unsigned int  bSucceed = TRUE;
	FT2232I2cStart(); //Set START condition for I2C communication
	//printf("***************** dev_addr = %#x, RegAddr = %#x, Value = %#4x", phy_dev.dev_addr, RegAddr, Value);
	bSucceed = FT2232I2cSendByteAndCheckACK(phy_dev.phy_addr << 1); //Send 7 bit Device address byte and check if ACK bit is received
	bSucceed = FT2232I2cSendByteAndCheckACK(RegAddr);   //Send Reg addressH byte and check if ACK bit is received
	//bSucceed = FT2232I2cSendByteAndCheckACK(RegAddr & 0xFF);   //Send Reg address L byte and check if ACK bit is received
	bSucceed = FT2232I2cSendByteAndCheckACK(Value);  //Write reg[0x02]
	//bSucceed = FT2232I2cSendByteAndCheckACK(Value & 0xFF);       //Write reg[0x03]
	FT2232I2cStop(); //Set STOP condition for I2C communication
	dwNumBytesToSend = 0; //Clear output buffer

}


void cr_mdio_write(CR_U16 RegAddr, CR_U16 RegVal) 
{
    if (port == 0)
        cr_write_mdio(RegAddr, RegVal);
    else
        cr_write_i2c(RegAddr, RegVal);
}


CR_U16 cr_mdio_read(CR_U16 RegAddr) 
{
    if (port == 0)
        return cr_read_mdio(RegAddr);
    else
        return cr_read_i2c(RegAddr);
}

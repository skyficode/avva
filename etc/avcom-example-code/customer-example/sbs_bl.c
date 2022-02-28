/*****************************************************************************
 * Avcom of Virginia 
 * API Example Program
 *  
 * This is an example program to illustrate how to use the AVCOM API's.
 * This file and program is provided for illustration purposes and functions
 * listed are considered to be in early development stage. Therefore, exact 
 * syntax and use of used functions may be subject to change.
 * 
 * This program will run at a command prompt and connect to an Avcom Spectrum
 * Analyzer with the default Ethernet settings, and will print out the SNR
 * of a peak signal between 1209.5MHz and 1219.5MHz. The noise level is
 * considered to be +/-1MHz of the peak signal.
 *
 * A header file "aovspecan.h" will be provided to 
 * 
 * FOR WINDOWS:
 *		Additionally the required *.dll and *.lib files will be provided and 
 *		and must be included into project as developing environment specifies.
 * 
 * Copyright Avcom of Virginia (c) 2010
 * 
 * Author: Marc Severo
 *****************************************************************************/


/*** AOVSPECAN.H: API header file ***/
//#include "aovspecan.h"
#include "../api/aovspecan.h"

#include "../api/aov_core.h"

/*** Additional header files specific to example program ***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EXPORT int AOV_recData(void *handle, unsigned char *data, int *data_size);
EXPORT int AOV_sendData(void *handle, unsigned char *data, int data_size);
EXPORT int checkforhandle(void *handle);

int AVCOM_SA_UnlockCal ( void *handle )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	//if (checkforhandle(handle))
	//	return AOV_HANDLE_NOT_FOUND;

	k = 3;
	sa_tx[k++]		= SBS_UNLOCK;
	sa_tx[k++]		= 0x77;
	sa_tx[k++]		= 0x30;
	sa_tx[k++]		= 0x77;
	sa_tx[k++]		= 0x30;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	for (k=0;k<0xFFFFFF;k++);
	iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen);
/*	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if (sa_rx[3] !=  GUI_MESSAGE)
		return AOV_ERR_UNEXPECTED_CMD;

	if (memcmp(&sa_rx[4],"Unlocked",8))
		return AOV_ERR_UNEXPECTED_CMD;*/

	return AOV_ERR_NO_ERROR;
}

int AVCOM_SA_CalMode ( void *handle, int mode )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	//if (checkforhandle(handle))
	//	return AOV_HANDLE_NOT_FOUND;

	k = 3;
	sa_tx[k++]		= SBS_CAL_CMD;
	sa_tx[k++]		= mode;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	for (k=0;k<0xFFFFFF;k++);
	iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen);
/*	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if (sa_rx[3] !=  GUI_MESSAGE)
		return AOV_ERR_UNEXPECTED_CMD;

	if (memcmp(&sa_rx[4],"In cal mode",11))
		return AOV_ERR_UNEXPECTED_CMD;
*/
	return AOV_ERR_NO_ERROR;
}


int AVCOM_SA_BootMode ( void *handle, int mode )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	//if (checkforhandle(handle))
	//	return AOV_HANDLE_NOT_FOUND;

	k = 3;
	sa_tx[k++]		= SBS_BOOT_MODE;
	sa_tx[k++]		= mode;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	for (k=0;k<0xFFFFFF;k++);
	iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen);
/*	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if (sa_rx[3] !=  GUI_MESSAGE)
		return AOV_ERR_UNEXPECTED_CMD;

	if (memcmp(&sa_rx[4],"In cal mode",11))
		return AOV_ERR_UNEXPECTED_CMD;
*/
	return AOV_ERR_NO_ERROR;
}



int AVCOM_SA_BootKey ( void *handle )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	//if (checkforhandle(handle))
	//	return AOV_HANDLE_NOT_FOUND;

	k = 3;
	sa_tx[k++]		= SBS_BOOTLOADER;
	sa_tx[k++]		= BOOTLOAD_KEY;
	sa_tx[k++]		= 0x77;
	sa_tx[k++]		= 0x30;
	sa_tx[k++]		= 0x77;
	sa_tx[k++]		= 0x30;
	sa_tx[k++]		= 0x6A;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	for (k=0;k<0xFFFFFF;k++);
	iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen);
/*	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if (sa_rx[3] !=  GUI_MESSAGE)
		return AOV_ERR_UNEXPECTED_CMD;

	if (memcmp(&sa_rx[4],"In cal mode",11))
		return AOV_ERR_UNEXPECTED_CMD;
*/
	return AOV_ERR_NO_ERROR;
}


int AVCOM_SA_BootDescProg( void *handle,unsigned char product,unsigned char project,unsigned char pcb,unsigned char chass,char *serial)
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k,i;
	int recvlen;
	int iResult;
	unsigned char sum=0;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	//if (checkforhandle(handle))
	//	return AOV_HANDLE_NOT_FOUND;

	k = 3;
	sa_tx[k++]		= SBS_BOOTLOADER;
	sa_tx[k++]		= BOOTLOAD_DESC;
	sa_tx[k++]		= product;
	sa_tx[k++]		= pcb;
	sa_tx[k++]		= project;
	sa_tx[k++]		= chass;
	memcpy(&sa_tx[k],serial,16);
	k+=16;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	sa_tx[k++]		= 0xFF;
	for (i=3; i<k; i++)
		sum -= sa_tx[i];
			
	sa_tx[k++]		= sum;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
//	for (k=0;k<0xFFFFFF;k++);
	iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen);
/*	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if (sa_rx[3] !=  GUI_MESSAGE)
		return AOV_ERR_UNEXPECTED_CMD;

	if (memcmp(&sa_rx[4],"In cal mode",11))
		return AOV_ERR_UNEXPECTED_CMD;
*/
	return AOV_ERR_NO_ERROR;
}


int AVCOM_SA_LockSaveCal( void *handle )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k,i;
	int recvlen;
	int iResult;
	unsigned char sum=0;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	//if (checkforhandle(handle))
	//	return AOV_HANDLE_NOT_FOUND;

	k = 3;
	sa_tx[k++]		= SBS_LOCK_CMD;
	sa_tx[k++]		= 0x31;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
//	for (k=0;k<0xFFFFFF;k++);
/*	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

*/	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

/*	if (sa_rx[3] !=  GUI_MESSAGE)
		return AOV_ERR_UNEXPECTED_CMD;

	if (memcmp(&sa_rx[4],"In cal mode",11))
		return AOV_ERR_UNEXPECTED_CMD;
*/
	return AOV_ERR_NO_ERROR;
}



void pexit(int code)
{
	fprintf(stderr, "Program Failed. Code: %i\n",code);
	AVCOM_SA_Deinitialize();
	exit(code);
}


int main (void)
{
	void *sa_handle;
	char ip[128];
	int portno;
	int iResult;
	int flags;
	int i;
	char instr[128];
	unsigned char product;
	unsigned char project;
	unsigned char pcb;
	char serial[32];
	

/* Initialize environment and process any errors */
	if ( (iResult = AVCOM_SA_Initialize()) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	fprintf(stderr,"Enter ip [192.168.118.243]: ");	
//	fgets(ip,128,stdin);
	fgets(ip,128,stdin);
	if(ip[0] == 0x0A)
		memcpy(ip,"192.168.118.243",sizeof("192.168.118.243"));
	else {
		for (i=0; i < 256; i++)
		{
			if (ip[i] == 0x0A)
			{
				ip[i] = 0x00;
				break;
			}
		}
	}

	fprintf(stderr,"Enter port [26482]: ");
	fflush (stdin);
	fgets(instr,128,stdin);
	if(instr[0] == 0x0A)
		portno = 26482;
	else
		portno = atoi(instr);
/*
	fprintf(stderr,"Enter flags [0]: ");
	fflush (stdin);
	fgets(instr,128,stdin);
	if(instr[0] == 0x0A)
		flags = 0;
	else
		flags = atoi(instr);
*/
	flags = 0;

/* Register Spectrum Analyzer */
	if ( (iResult = AVCOM_SA_Register(&sa_handle, ip, portno, 0, flags)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	
/* Open TCP Socket */
	if ( (iResult = AVCOM_SA_Connect(sa_handle)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	if ( (iResult = AVCOM_SA_UnlockCal(sa_handle)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	if ( (iResult = AVCOM_SA_CalMode(sa_handle,1)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	if ( (iResult = AVCOM_SA_BootMode(sa_handle,1)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	if ( (iResult = AVCOM_SA_BootKey(sa_handle)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	fprintf(stderr,"Enter ProductID [0x5A]: 0x");
	fflush (stdin);
	fgets(instr,128,stdin);
	if(instr[0] == 0x0A)
		product = 0x5A;
	else
		sscanf(instr,"%x",&product);

	fprintf(stderr,"Enter PCB-FAB [0x5A]: 0x");
	fflush (stdin);
	fgets(instr,128,stdin);
	if(instr[0] == 0x0A)
		pcb = 0x5A;
	else
		sscanf(instr,"%x",&pcb);

	fprintf(stderr,"Enter Project Code [0x23]: 0x");
	fflush (stdin);
	fgets(instr,128,stdin);
	if(instr[0] == 0x0A)
		project = 0x23;
	else
		sscanf(instr,"%x",&project);

	fprintf(stderr, "Enter Serial [NULL]: ");
	fgets(serial,128,stdin);
	//fgets(filename,128,stdin);
	if(serial[0] == 0x0A)
		memset(&serial[0],'0',16);

	if ( (iResult = AVCOM_SA_BootDescProg(sa_handle,product,project,pcb,4,serial)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

//	if ( (iResult = AVCOM_SA_LockSaveCal(sa_handle)) != AOV_ERR_NO_ERROR)
//	{
//		pexit(iResult);
//	}

	if ( (iResult = AVCOM_SA_Disconnect(sa_handle)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	if ( (iResult = AVCOM_SA_Unregister(sa_handle)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

	if ( (iResult = AVCOM_SA_Deinitialize()) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult);
	}

//	fprintf(stderr, "Program Successful. SNR: %f\n",snr);

	return 0;

}




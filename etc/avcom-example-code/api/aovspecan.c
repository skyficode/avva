// AOV_api.cpp : Defines the exported functions for the DLL application.
//

//#include <stdio.h>
//#include <string.h>  //for memcpy and memset
#include <stdlib.h>  //for exit
#include <math.h>  //for exit


#include "aovspecan.h"
#include "aov_core.h"
#include "stdafx.h"
//#include "ftd2xx.h"


#ifdef _WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;

}
#endif

AOVhandle **plist=NULL;
int			plen=0;


int checkforhandle(void *handle)
{
	int i;
	
	for (i = 0; i < plen;  i++)
	{
		if ( plist[i] == handle)
			return AOV_ERR_NO_ERROR;
	}

	return AOV_HANDLE_NOT_FOUND;
}


int AVCOM_SA_Connect(void *handle) 
{
	int *socketfd;
	unsigned char sa_rx[1024];
	int bytes_recv;

	AOVhandle *lhandle = (AOVhandle *)handle;
	int iResult;
	struct timeval tv;
	
	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	socketfd = (&lhandle->socketfd);
	
	if (*socketfd != -1) // already open
	{
		return AOV_ERR_SOCKET_ALREADY_OPEN;
	}

	if ((*socketfd = socket(lhandle->servinfo->ai_family, lhandle->servinfo->ai_socktype, lhandle->servinfo->ai_protocol)) < 0)
		return AOV_ERR_SOCKET_FAILED;		// socket() failed

	
	tv.tv_sec = 20;
	tv.tv_usec = 0;  
  
	if ((iResult = setsockopt((int)*socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof(tv))) != 0)
		return AOV_ERR_SETSOCKETOPT_FAILED;

	if (connect((int)*socketfd, lhandle->servinfo->ai_addr, lhandle->servinfo->ai_addrlen) < 0)
	{
		//iResult = WSAGetLastError();  // Extra Error checking for windows only
		AVCOM_SA_Disconnect(handle);
		return AOV_ERR_CONNECT_FAILED;		// connect() failed
	}

	AOV_recData(handle, sa_rx, &bytes_recv);

	return AOV_ERR_NO_ERROR;
}


int AVCOM_SA_Disconnect(void *handle) 
{
	AOVhandle *lhandle = (AOVhandle *)handle;

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if (lhandle->socketfd == -1)
		return AOV_ERR_SOCKET_NOT_OPEN;

#ifdef _WIN32
	if (closesocket(lhandle->socketfd))
		return AOV_ERR_CLOSESOCKET_FAILED;
#else
	if (close(lhandle->socketfd))
		return AOV_ERR_CLOSE_FAILED;
#endif

	lhandle->socketfd = -1;

	return AOV_ERR_NO_ERROR;
}



int AOV_sendData(void *handle, unsigned char *data, int data_size)
{
	AOVhandle *lhandle = (AOVhandle *)handle;

// Currently only used inside API's so this is already done
//	if (checkforhandle(handle))
//		return AOV_HANDLE_NOT_FOUND;

	if (send(lhandle->socketfd, (char *)data, data_size, 0) != data_size)
		return AOV_ERR_SEND_FAILED;

	return AOV_ERR_NO_ERROR;
}



int AOV_recData(void *handle, unsigned char *data, int *data_size)
{
	int iResult;
	char onedata;
	int startpacket = 0;
	int packetnotfull = 1;
	int i,j;
	unsigned short packlen;
	AOVhandle *lhandle = (AOVhandle *)handle;

/* 
	Currently only used inside API's so this is already done
	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;
*/
	
	i = 0;
	do {
        iResult = recv(lhandle->socketfd, &onedata, 1, 0);
		if ((onedata == 0x02) && (!startpacket))
			startpacket = 1;
		if (startpacket)
		{
			data[i++] = onedata;
			if (i == 3)
			{
				j = 0;
				packlen = ((data[1]<<8) | (data[2]));
			}
			if (i > 3)
			{
				j++;
				if (j >= packlen)
					packetnotfull = 0;
			}
		}
    } while(( iResult > 0 ) && (packetnotfull));

	if (iResult < 0)
		return WSAGetLastError();
//		return AOV_ERR_RECV_FAILED;

	*data_size = i;

	if (iResult > 0)
		iResult = 0;

	return AOV_ERR_NO_ERROR;
}




int AVCOM_SA_GetSpectrumData(void *handle, sAOVwaveForm *trace)
{
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	unsigned short int data;
	unsigned long cf, sp;
	char reflvl;
	int txlen;
	int i,j,k;
	int bytes_recv;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; //currently unused
	
	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(handle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	if ((iResult = AOV_recData(handle, sa_rx, &bytes_recv)) != AOV_ERR_NO_ERROR)
		return iResult;

	j = 4;
	if (sa_rx[3] == WAVE_12BIT)
	{
		reflvl = sa_rx[493];
		if (lhandle->sbshw.sbs_fw < 0x0300)
			reflvl		*= -1;
		lhandle->cur_settings.ref_level = reflvl;
		lhandle->cur_settings.rbw = sa_rx[494];
		lhandle->cur_settings.input = sa_rx[495]-9;
		lhandle->cur_settings.lnb = sa_rx[496];


		for (i = 0; i < 320; i++)
		{
			data = 0;
			data  = ((sa_rx[j++] << 4) & 0xFF0);
			data |= ((sa_rx[j]   >> 4) & 0x00F);
			trace->amplitude[i] = data/80.0F + (reflvl-40);
			i++; data = 0;
			data  = ((sa_rx[j++] << 8) & 0xF00);
			data |= ((sa_rx[j++] & 0x0FF));
			trace->amplitude[i] = data/80.0F + (reflvl-40);
		}
		cf	 	 = 	(sa_rx[485] << 24); 
		cf	 	|=	(sa_rx[486] << 16);
		cf		|=	(sa_rx[487] <<  8);	
		cf		|=	(sa_rx[488]);
		lhandle->cur_settings.cf = cf;
		sp	 	 = 	(sa_rx[489] << 24); 
		sp	 	|=	(sa_rx[490] << 16);
		sp		|=	(sa_rx[491] <<  8);	
		sp		|=	(sa_rx[492]);
		lhandle->cur_settings.sp = sp;

	} else if (sa_rx[3] == WAVE_8BIT) {
		reflvl = sa_rx[333];
		if (lhandle->sbshw.sbs_fw < 0x0300)
			reflvl		*= -1;
		lhandle->cur_settings.ref_level = reflvl;
		lhandle->cur_settings.rbw = sa_rx[334];
		lhandle->cur_settings.input = sa_rx[335]-9;
		lhandle->cur_settings.lnb = sa_rx[336];

		for (i = 0; i < 320; i++ )
		{
			data = 0;
			data = sa_rx[j++];
			trace->amplitude[i] = data/5.0F + (reflvl-40);
		}
		cf	 	 = 	(sa_rx[325] << 24); 
		cf	 	|=	(sa_rx[326] << 16);
		cf		|=	(sa_rx[327] <<  8);	
		cf		|=	(sa_rx[328]);
		lhandle->cur_settings.cf = cf;
		sp	 	 = 	(sa_rx[329] << 24); 
		sp	 	|=	(sa_rx[330] << 16);
		sp		|=	(sa_rx[331] <<  8);	
		sp		|=	(sa_rx[332]);
		lhandle->cur_settings.sp = sp;
	}

	for (i = -160; i < 160; i++)
	{
		trace->frequency[i+160] = ( (double)(cf) + ( i * (double)(sp)/320.F )) *.0001F;
	}

	return AOV_ERR_NO_ERROR;
		
}





int AVCOM_SA_Register(void **handle, char *ip, int port, int mode, int flags)
{
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int i,k;
	int txlen;
	int recvlen;
	int iResult;
	int *socketfd;
	AOVhandle *lhandle;
	struct addrinfo hints;

	char portbuf[10];
	
	if (plist == NULL)
		return AOV_ERR_NOT_INITIALIZED;

	if (( lhandle = (void *)malloc( sizeof(AOVhandle) ) ) == AOV_ERR_NO_ERROR) {
		return AOV_ERR_MALLOC_FAILED;
	}

	socketfd = (&lhandle->socketfd);
	*socketfd = -1; // say it's not established
	
	plen++;
	plist = realloc( plist, (plen) * sizeof(AOVhandle *) );
	plist[plen-1] = lhandle;

	*handle = (void *)lhandle;	

	lhandle->version_major = AOVAPI_MAJOR;
	lhandle->version_minor = AOVAPI_MINOR;
	lhandle->version_patch = AOVAPI_PATCH;

	lhandle->ip = ip;
	lhandle->portno = port;

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	sprintf(portbuf,"%d",lhandle->portno);
	if ((iResult = getaddrinfo(lhandle->ip, portbuf, &hints, &lhandle->servinfo)) != 0) {
		AVCOM_SA_Unregister(lhandle);
    return AOV_ERR_GET_ADDR_INFO_FAIL;
}

	for ( i = 0; i < (plen-1); i++)
	{
		if (!(memcmp(plist[i]->servinfo->ai_addr->sa_data,lhandle->servinfo->ai_addr->sa_data,sizeof(plist[i]->servinfo->ai_addr->sa_data))))
		{
			if (port == plist[i]->portno)
				AVCOM_SA_Unregister(lhandle);
				return AOV_HANDLE_ALREADY_EXISTS;
		}
	}

	if ((iResult = AVCOM_SA_Connect(lhandle)) != AOV_ERR_NO_ERROR)
	{
		AVCOM_SA_Unregister(lhandle);
		return iResult;
	}

	k = 3;
	sa_tx[k++] = HW_DESC;
	sa_tx[k++] = 0x00;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
	{
		AVCOM_SA_Unregister(lhandle);
		return iResult;
	}

	if ((iResult = AOV_recData(lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
	{
		AVCOM_SA_Unregister(lhandle);
		return iResult;
	}

	if (sa_rx[3] != HW_DESC)
	{
		AVCOM_SA_Unregister(lhandle);
		return AOV_ERR_UNEXPECTED_CMD;
	}

	lhandle->sbshw.prodID = sa_rx[4];
	lhandle->sbshw.sbs_fw = (sa_rx[5] << 8) | sa_rx[6];
	
	lhandle->cur_settings.cf 	  = (sa_rx[8] << 24); 
	lhandle->cur_settings.cf 	 |=	(sa_rx[9] << 16);
	lhandle->cur_settings.cf 	 |=	(sa_rx[10] <<  8);
	lhandle->cur_settings.cf 	 |=	(sa_rx[11]);

	lhandle->cur_settings.sp 	  = (sa_rx[12] << 24); 
	lhandle->cur_settings.sp 	 |=	(sa_rx[13] << 16);
	lhandle->cur_settings.sp 	 |=	(sa_rx[14] <<  8);
	lhandle->cur_settings.sp 	 |=	(sa_rx[15]);

	lhandle->cur_settings.ref_level =	(char)sa_rx[16];
	if (lhandle->sbshw.sbs_fw < 0x0300)
		lhandle->cur_settings.ref_level *= -1;

	lhandle->cur_settings.rbw =	sa_rx[17];

	lhandle->sbshw.PCBfab = sa_rx[45];
	lhandle->sbshw.projID = sa_rx[58];
	switch (lhandle->sbshw.prodID)
	{
		case SBS_1100:
			lhandle->sbshw.freq_min = SBS_1100_LOWER_LIMIT;
			lhandle->sbshw.freq_max = SBS_1100_UPPER_LIMIT;
			break;

		case SBS_2150:
			lhandle->sbshw.freq_min = SBS_2150_LOWER_LIMIT;
			lhandle->sbshw.freq_max = SBS_2150_UPPER_LIMIT;
			break;

		case SBS_2300:
			lhandle->sbshw.freq_min = SBS_2500_LOWER_LIMIT;
			lhandle->sbshw.freq_max = SBS_2500_UPPER_LIMIT;
			break;

		case SBS_2500:
			lhandle->sbshw.freq_min = SBS_2500_LOWER_LIMIT;
			lhandle->sbshw.freq_max = SBS_2500_UPPER_LIMIT;
			if ( (lhandle->sbshw.projID & 0x0F) == 0x03 )
			{
				lhandle->sbshw.freq_min = SBS_2300_LOWER_LIMIT;
				lhandle->sbshw.freq_max = SBS_2300_UPPER_LIMIT;
			}
			break;
	}
			
	lhandle->avail_rbw.mask = sa_rx[18];
	if (lhandle->avail_rbw.mask & RBW_3MHZ) {
		lhandle->avail_rbw.rbw_3MHz = TRUE;
	} else {
		lhandle->avail_rbw.rbw_3MHz = FALSE;
	}

	if (lhandle->avail_rbw.mask & RBW_1MHZ) {
		lhandle->avail_rbw.rbw_1MHz = TRUE;
	} else {
		lhandle->avail_rbw.rbw_1MHz = FALSE;
	}
	
	if (lhandle->avail_rbw.mask & RBW_300KHZ) {
		lhandle->avail_rbw.rbw_300KHz = TRUE;
	} else {
		lhandle->avail_rbw.rbw_300KHz = FALSE;
	}
	
	if (lhandle->avail_rbw.mask & RBW_100KHZ) {
		lhandle->avail_rbw.rbw_100KHz = TRUE;
	} else {
		lhandle->avail_rbw.rbw_100KHz = FALSE;
	}
	
	if (lhandle->avail_rbw.mask & RBW_10KHZ) {
		lhandle->avail_rbw.rbw_10KHz = TRUE;
	} else {
		lhandle->avail_rbw.rbw_10KHz = FALSE;
	}
	
	if (lhandle->avail_rbw.mask & RBW_200KHZ) {
		lhandle->avail_rbw.rbw_200KHz = TRUE;
	} else {
		lhandle->avail_rbw.rbw_200KHz = FALSE;
	}

	lhandle->cur_settings.input	= sa_rx[19] - 9;
	lhandle->avail_inputs	= sa_rx[20] - 10;

	for ( i = 0; i < 16; i++ )
		lhandle->serial[i] = sa_rx[29+i];

	lhandle->cal_date.seconds = 0;
	lhandle->cal_date.minutes = 0;
	lhandle->cal_date.hours = 0;
	lhandle->cal_date.day = sa_rx[46]-10;
	lhandle->cal_date.month = sa_rx[47]-10;
	lhandle->cal_date.year = sa_rx[48]*100 + sa_rx[49];

	for (i = 0; i < 6; i++)
	{
		if ( i < lhandle->avail_inputs )
		{
			lhandle->input[i].rf_avail = TRUE;
			if (sa_rx[59] & (0x01 << i) ) {
				lhandle->input[i].db70_avail = TRUE;
			} else {
				lhandle->input[i].db70_avail = FALSE;
			}

			if (sa_rx[54] & (0x02 << i) ) {
				lhandle->input[i].lnb_avail = TRUE;
			} else {
				lhandle->input[i].lnb_avail = FALSE;
			}
		} else {
			lhandle->input[i].rf_avail = FALSE;
			lhandle->input[i].db70_avail = FALSE;
			lhandle->input[i].lnb_avail = FALSE;
		}
	}

	lhandle->cur_settings.lnb =	sa_rx[55];

	// Get LNB HW Desc Packet
	k = 3;
	sa_tx[k++] = LNB_DESC;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
	{
		AVCOM_SA_Unregister(lhandle);
		return iResult;
	}

	if ((iResult = AOV_recData(lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
	{
		AVCOM_SA_Unregister(lhandle);
		return iResult;
	}

	if (sa_rx[3] != LNB_DESC)
	{
		AVCOM_SA_Unregister(lhandle);
		return AOV_ERR_UNEXPECTED_CMD;
	}

	for (i=0; i < lhandle->avail_inputs; i++)
	{
		lhandle->input[i].dbrange.max = -10;
		if (lhandle->input[i].db70_avail)
			lhandle->input[i].dbrange.min = -70;
		else
			lhandle->input[i].dbrange.min = -50;

		if (lhandle->sbshw.sbs_fw > 0x02FF)
		{
			lhandle->input[i].dbrange.max -= sa_rx[37+i];
			lhandle->input[i].dbrange.min -= sa_rx[37+i];
		}
	}

	if ( (lhandle->sbshw.projID & 0xE) && (lhandle->sbshw.prodID != 0xFF) )
	{
		k = 3;
		sa_tx[k++] = BLKFIN_CMDS;
		sa_tx[k++] = READ_FP_HW_DESC;
		sa_tx[k++] = ETX;
		txlen = k - 3;
		sa_tx[0] = STX;
		sa_tx[1] = (txlen >> 8) & 0xFF;
		sa_tx[2] =  txlen & 0xFF;
		if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		{
			AVCOM_SA_Unregister(lhandle);
			return iResult;
		}

		if ((iResult = AOV_recData(lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		{
			AVCOM_SA_Unregister(lhandle);
			return iResult;
		}

		if ((sa_rx[3] != BLKFIN_CMDS) && (sa_rx[4] != READ_FP_HW_DESC))
		{
			AVCOM_SA_Unregister(lhandle);
			return AOV_ERR_UNEXPECTED_CMD;
		}

		lhandle->lcdhw.lcd_fw = (sa_rx[5] << 8) | sa_rx[6];
		lhandle->lcdhw.mbfab = sa_rx[7];
		lhandle->lcdhw.fpfab = sa_rx[8];
		lhandle->lcdhw.projID = (sa_rx[11] << 8) | sa_rx[12];
	}


	if (AOV_ALSO_CONNECT != flags)
		if ((iResult = AVCOM_SA_Disconnect(lhandle)) != AOV_ERR_NO_ERROR)
			return iResult;


	return (0);


}

int AVCOM_SA_Unregister(void *handle)
{
	int i;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	for ( i = 0; i < plen; i++ )
	{
		if (plist[i] == (AOVhandle *)handle)
			break;
	}

	if (i == plen)
		return AOV_HANDLE_NOT_FOUND;

	AVCOM_SA_Disconnect(lhandle);

	for (i=i; i < (plen-1); i++ )
	{
		plist[i] = plist[i+1];
	}

	plen --;
	if (plen) // if no active handles, don't realloc()
		plist = realloc( plist, (plen) * sizeof(AOVhandle *) );


	free(handle);
	return AOV_ERR_NO_ERROR;
}



int AVCOM_SA_SetAllSettings(void *handle, double cfreq, double span, int reflvl, eAOV_RBW rbw, int input, sAOVlnb *lnb)
{
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int k;
	int recvlen, txlen;
	int iResult;
	unsigned int icfreq, icfreqrx;
	unsigned int ispan, ispanrx;
	char creflvl, creflvlrx;
	unsigned char clnb;
	unsigned char clnbrx, crbwrx, crfrx;
	AOVhandle *lhandle = (AOVhandle *)handle;
	unsigned char *pfreq;


	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((cfreq < lhandle->sbshw.freq_min) || (cfreq < lhandle->sbshw.freq_min))
		return AOV_ERR_INVALID_CFREQ;

	if ((span < 0) || (span > 1300))
		return AOV_ERR_INVALID_SPAN;

	if ((input < 1) || (input > lhandle->avail_inputs))
		return AOV_ERR_INVALID_RF;

	if (!(((rbw == RBW_3MHZ)   && (lhandle->avail_rbw.rbw_3MHz))   ||
		  ((rbw == RBW_1MHZ)   && (lhandle->avail_rbw.rbw_1MHz))   ||
		  ((rbw == RBW_300KHZ) && (lhandle->avail_rbw.rbw_300KHz)) ||
		  ((rbw == RBW_100KHZ) && (lhandle->avail_rbw.rbw_100KHz)) ||
		  ((rbw == RBW_10KHZ)  && (lhandle->avail_rbw.rbw_10KHz))  ||
		  ((rbw == RBW_200KHZ) && (lhandle->avail_rbw.rbw_200KHz)) ))
		return AOV_ERR_INVALID_RBW;

	if ( (reflvl > lhandle->input[input-1].dbrange.max)	|| 
		 (reflvl < lhandle->input[input-1].dbrange.min)		)
		 return AOV_ERR_INVALID_RFLVL;

	icfreq		= (unsigned int)(cfreq * 10000);
	ispan		= (unsigned int)(span  * 10000);
	creflvl		= reflvl;
	if (lhandle->sbshw.sbs_fw < 0x0300)
		creflvl		*= -1;

	clnb = 0x40;		// no LNB check - how may it be wrong?
	if (lnb != NULL)
	{
		if (lnb->enable)
		{
			clnb |= 0x04;
			if (lnb->volt == 18)
				clnb |= 0x08;
			if (lnb->khz_22)
				clnb |= 0x20;
		}
	}


	k = 3;
	sa_tx[k++] = WAVE_CHANGE;
	pfreq = (unsigned char *)&icfreq;
	sa_tx[k++]		= pfreq[3];
	sa_tx[k++] 		= pfreq[2];
	sa_tx[k++]		= pfreq[1];
	sa_tx[k++] 		= pfreq[0];
	pfreq = (unsigned char *)&ispan;
	sa_tx[k++]		= pfreq[3];
	sa_tx[k++] 		= pfreq[2];
	sa_tx[k++]		= pfreq[1];
	sa_tx[k++] 		= pfreq[0];
	sa_tx[k++] = creflvl;
	sa_tx[k++] = rbw;
	sa_tx[k++] = input + 9;
	sa_tx[k++] = clnb;
	sa_tx[k++] = 0x00;
	sa_tx[k++] = 0x00;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	Sleep(400);
	
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	
	k=485;
	icfreqrx 	 = 	(sa_rx[k++] << 24); 
	icfreqrx	|=	(sa_rx[k++] << 16);
	icfreqrx	|=	(sa_rx[k++] <<  8);
	icfreqrx	|=	(sa_rx[k++]);
	ispanrx 	 = 	(sa_rx[k++] << 24); 
	ispanrx		|=	(sa_rx[k++] << 16);
	ispanrx		|=	(sa_rx[k++] <<  8);
	ispanrx		|=	(sa_rx[k++]);
	creflvlrx	 =	 sa_rx[k++];
	crbwrx		 =	 sa_rx[k++];
	crfrx		 =	 sa_rx[k++]-9;
	k += 4;
	clnbrx		 =	 sa_rx[k++];  

/* Cannot verify settigns have been changed if Auto-sweep mode is OFF
	if (icfreq != icfreqrx)
		return AOV_ERR_SET_CFREQ_FAILED;
	if (creflvl != creflvlrx)
		return AOV_ERR_SET_RFLVL_FAILED;
	if (rbw != crbwrx)
		return AOV_ERR_SET_RBW_FAILED;
	if (input != crfrx)
		return AOV_ERR_SET_RF_FAILED;
	if (clnb != (0x7C & clnbrx))
		return AOV_ERR_SET_LNB_FAILED;

	return AOV_ERR_NO_ERROR;
	if (ispan != ispanrx)
		return AOV_NOTE_SPAN_AUTOSET;
*/
	lhandle->cur_settings.input = input;

	return AOV_ERR_NO_ERROR;
}


int AVCOM_SA_Initialize(void)
{

#ifdef _WIN32
	WSADATA wsaData;
#endif


#ifdef _WIN32
	if (WSAStartup(MAKEWORD(2, 0),&wsaData) != 0)
		return AOV_ERR_WSASTARTUP_FAILED;		// WSAStartup() failed
#endif

	if (plist != NULL)
		return AOV_ALREADY_INITIALIZED;
	plist = malloc( sizeof(AOVhandle *) );
	plen = 0;

	
	return AOV_ERR_NO_ERROR;

}


int AVCOM_SA_Deinitialize(void)
{
	if (plist == NULL)
		return AOV_ALREADY_NOT_INITIALIZED;
	while (plen)
	{
		AVCOM_SA_Unregister(plist[plen-1]);
	}
	free(plist);
	plist = NULL;
	plen = 0;

#ifdef _WIN32
	if (WSACleanup())
		return AOV_ERR_WSACLEANUP_FAILED;
#endif

	return 0;

}

void *AVCOM_SA_GetHandle(char *ip, int port)
{
	int i;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results
	char portbuf[10];
	int iResult;

	

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	sprintf(portbuf,"%i",port);
	if ((iResult = getaddrinfo(ip, portbuf, &hints, &servinfo)) != 0) {
		return (void *)-1;
	}


	for ( i = 0; i < plen; i++)
	{
		if (!(memcmp(plist[i]->servinfo->ai_addr->sa_data,servinfo->ai_addr->sa_data,sizeof(plist[i]->servinfo->ai_addr->sa_data))))
		{
			if (port == plist[i]->portno)
				return (void *)plist[i];
		}
	}

	return (void *)-1;
}


int AVCOM_SA_SetCenterFrequency ( void *handle, double cfreq )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
	unsigned long icfreq;
	unsigned char *pfreq;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	
	if ((cfreq < lhandle->sbshw.freq_min) || (cfreq < lhandle->sbshw.freq_min))
		return AOV_ERR_INVALID_CFREQ;

	icfreq		= (unsigned int)(cfreq * 10000);

	k = 3;
	sa_tx[k++]		= CHANGE_INDIVIDUAL;
	sa_tx[k++]		= API_CHANGE_CF;
	pfreq = (unsigned char *)&icfreq;
	sa_tx[k++]		= pfreq[3];
	sa_tx[k++] 		= pfreq[2];
	sa_tx[k++]		= pfreq[1];
	sa_tx[k++] 		= pfreq[0];
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

/* // ERROR CHECKING DONE HERE - SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
*/	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
/*	k=485;
	icfreqrx 	 = 	(sa_rx[k++] << 24); 
	icfreqrx	|=	(sa_rx[k++] << 16);
	icfreqrx	|=	(sa_rx[k++] <<  8);
	icfreqrx	|=	(sa_rx[k++]);

	if (icfreq != icfreqrx)
		return AOV_ERR_SET_CFREQ_FAILED;
*/
	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  CHANGE_INDIVIDUAL) && (sa_rx[5] !=  API_CHANGE_CF))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_CFREQ_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

	return AOV_ERR_NO_ERROR;
}



int AVCOM_SA_SetSpan ( void *handle, double span )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
	unsigned long ispan;
	unsigned char *pspan;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((span < 0) || (span > 1300)) // uncertain limits
		return AOV_ERR_INVALID_SPAN;

	ispan			= (unsigned int)(span * 10000);

	k = 3;
	sa_tx[k++]		= CHANGE_INDIVIDUAL;
	sa_tx[k++]		= API_CHANGE_SP;
	pspan = (unsigned char *)&ispan;
	sa_tx[k++]		= pspan[3];
	sa_tx[k++] 		= pspan[2];
	sa_tx[k++]		= pspan[1];
	sa_tx[k++] 		= pspan[0];
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

/*// ERROR CHECKING DONE HERE - BUT SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
*/	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
/*	k=489;
	ispanrx 	 = 	(sa_rx[k++] << 24); 
	ispanrx		|=	(sa_rx[k++] << 16);
	ispanrx		|=	(sa_rx[k++] <<  8);
	ispanrx		|=	(sa_rx[k++]);

	if (ispan != ispanrx)
		return AOV_NOTE_SPAN_AUTOSET;		//NOT A COMPLETE ERROR CHECK
*/
	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  CHANGE_INDIVIDUAL) && (sa_rx[5] !=  API_CHANGE_SP))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_NOTE_SPAN_AUTOSET;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

	return AOV_ERR_NO_ERROR;
}




int AVCOM_SA_SetRefLevel ( void *handle, int reflvl )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
	char creflvl;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 
	

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

//	if ( (reflvl > lhandle->input[lhandle->cur_settings.input-1].dbrange.max)	|| 
//		 (reflvl < lhandle->input[lhandle->cur_settings.input-1].dbrange.min)		)
//		 return AOV_ERR_INVALID_RFLVL;

	creflvl		= reflvl;

	k = 3;
	sa_tx[k++]		= CHANGE_INDIVIDUAL;
	sa_tx[k++]		= API_CHANGE_RL;
	sa_tx[k++] = creflvl;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

/* // ERROR CHECKING DONE HERE - BUT SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
*/	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

/*	k=493;
	creflvlrx	 =	 sa_rx[k++];

	if (creflvl != creflvlrx)
		return AOV_ERR_SET_RFLVL_FAILED;		//NOT A COMPLETE ERROR CHECK
*/
	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  CHANGE_INDIVIDUAL) && (sa_rx[5] !=  API_CHANGE_RL))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_RFLVL_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

	return AOV_ERR_NO_ERROR;
}





int AVCOM_SA_SetResBandwidth ( void *handle, eAOV_RBW rbw )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if (!(((rbw == RBW_3MHZ)   && (lhandle->avail_rbw.rbw_3MHz))   ||
		  ((rbw == RBW_1MHZ)   && (lhandle->avail_rbw.rbw_1MHz))   ||
		  ((rbw == RBW_300KHZ) && (lhandle->avail_rbw.rbw_300KHz)) ||
		  ((rbw == RBW_100KHZ) && (lhandle->avail_rbw.rbw_100KHz)) ||
		  ((rbw == RBW_10KHZ)  && (lhandle->avail_rbw.rbw_10KHz))  ||
		  ((rbw == RBW_200KHZ) && (lhandle->avail_rbw.rbw_200KHz)) ))
		return AOV_ERR_INVALID_RBW;

	k = 3;
	sa_tx[k++] = CHANGE_INDIVIDUAL;
	sa_tx[k++] = API_CHANGE_RBW;
	sa_tx[k++] = rbw;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

/* // ERROR CHECKING DONE HERE - BUT SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
*/	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
/*
	k=494;
	crbwrx		 =	 sa_rx[k++];

	if (rbw != crbwrx)
		return AOV_ERR_SET_RBW_FAILED;		//NOT A COMPLETE ERROR CHECK
*/
	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  CHANGE_INDIVIDUAL) && (sa_rx[5] !=  API_CHANGE_RBW))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_RBW_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

	return AOV_ERR_NO_ERROR;
}




int AVCOM_SA_SetInput ( void *handle, int input, int flags )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((input < 1) || (input > lhandle->avail_inputs))
		return AOV_ERR_INVALID_RF;


	k = 3;
	sa_tx[k++] = CHANGE_INDIVIDUAL;
	sa_tx[k++] = API_CHANGE_INPUT;
	sa_tx[k++] = input + 9;
	sa_tx[k++] = flags;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

/* // ERROR CHECKING DONE HERE - BUT SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
*/	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
/*
	k=495;
	crfrx		 =	 sa_rx[k++]-9;

	if (input != crfrx)
		return AOV_ERR_SET_RF_FAILED;		//NOT A COMPLETE ERROR CHECK
*/
	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  CHANGE_INDIVIDUAL) && (sa_rx[5] !=  API_CHANGE_INPUT))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_RF_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

	lhandle->cur_settings.input = input;

	return AOV_ERR_NO_ERROR;
}




int AVCOM_SA_SetLNB ( void *handle, sAOVlnb *lnb )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	unsigned char clnb;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 
	int input = 1; // ERRRRRRRRRRRRRRRRRRRRROR ERROR DEBUG

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((input < 1) || (input > lhandle->avail_inputs))
		return AOV_ERR_INVALID_RF;

	clnb = 0x40;		// no LNB check - how may it be wrong?
	if (lnb != NULL)
	{
		if (lnb->enable)
		{
			clnb |= 0x04;
			if (lnb->volt)
				clnb |= 0x08;
			if (lnb->khz_22)
				clnb |= 0x20;
		}
	}


	k = 3;
	sa_tx[k++] = CHANGE_INDIVIDUAL;
	sa_tx[k++] = API_CHANGE_LNB;
	sa_tx[k++] = clnb;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

/* // ERROR CHECKING DONE HERE - BUT SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
*/	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
/*
	k=500;
	clnbrx		 =	 sa_rx[k++];

	if (clnb != (0x7C & clnbrx))
		return AOV_ERR_SET_LNB_FAILED;		//NOT A COMPLETE ERROR CHECK
*/
	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  CHANGE_INDIVIDUAL) && (sa_rx[5] !=  API_CHANGE_LNB))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_LNB_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

	return AOV_ERR_NO_ERROR;
}



int AVCOM_SA_LNBstruct(sAOVlnb *lnb, int LNB_EN, int LNB_18, int LNB_22 )
{
	if (LNB_EN) {
		lnb->enable = TRUE;
	} else {
		lnb->enable = FALSE;
	}

	if (LNB_18) {
		lnb->volt = 18;
	}else {
		lnb->volt = 13;
	}

	if (LNB_22) {
		lnb->khz_22 = TRUE;
	} else {
		lnb->khz_22 = FALSE;
	}
	return 0;
}





int AVCOM_SA_SetMarkerFreq ( void *handle, int markerno, double freq )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
//	unsigned long icfreq, icfreqrx;
	unsigned long icfreq;
	unsigned char *pfreq;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	
	if ((freq < lhandle->sbshw.freq_min) || (freq < lhandle->sbshw.freq_min))
		return AOV_ERR_INVALID_CFREQ;	
	
	if ((markerno < 0) || (markerno > 10))
		return AOV_ERR_INVALID_MARKER;

	icfreq		= (unsigned int)(freq * 10000);

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_SET_MARKER;
	sa_tx[k++]		= markerno;
	pfreq = (unsigned char *)&icfreq;
	sa_tx[k++]		= pfreq[3];
	sa_tx[k++] 		= pfreq[2];
	sa_tx[k++]		= pfreq[1];
	sa_tx[k++] 		= pfreq[0];
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  API_CMDS) && (sa_rx[5] !=  API_SET_MARKER))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_MARKER_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

/****************************************** NEED TO DO ERROR CHECKING
// ERROR CHECKING DONE HERE - SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
	k=485;
	icfreqrx 	 = 	(sa_rx[k++] << 24); 
	icfreqrx	|=	(sa_rx[k++] << 16);
	icfreqrx	|=	(sa_rx[k++] <<  8);
	icfreqrx	|=	(sa_rx[k++]);

	if (icfreq != icfreqrx)
		return AOV_ERR_SET_CFREQ_FAILED;
************************************************************/

	return AOV_ERR_NO_ERROR;
}





int AVCOM_SA_SetMarkerOpt ( void *handle, int markerno, int flags )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;
	
	if ((markerno < 0) || (markerno > 10))
		return AOV_ERR_INVALID_MARKER;

	if ((flags < 1) || (flags > 2))
		return AOV_ERR_INVALID_FLAG;

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_SET_MARKER_OPT;
	sa_tx[k++]		= markerno;
	sa_tx[k++]		= (unsigned char)flags;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  API_CMDS) && (sa_rx[5] !=  API_SET_MARKER_OPT))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_MARKER_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

/****************************************** NEED TO DO ERROR CHECKING
// ERROR CHECKING DONE HERE - SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
	k=485;
	icfreqrx 	 = 	(sa_rx[k++] << 24); 
	icfreqrx	|=	(sa_rx[k++] << 16);
	icfreqrx	|=	(sa_rx[k++] <<  8);
	icfreqrx	|=	(sa_rx[k++]);

	if (icfreq != icfreqrx)
		return AOV_ERR_SET_CFREQ_FAILED;
************************************************************/

	return AOV_ERR_NO_ERROR;
}





int AVCOM_SA_ClearMarker ( void *handle, int markerno)
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
//	unsigned long icfreq, icfreqrx;
//	unsigned char *pfreq;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((markerno < -1) || (markerno > 10))
		return AOV_ERR_INVALID_MARKER;

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_CLEAR_MARKER;
	sa_tx[k++]		= markerno;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  API_CMDS) && (sa_rx[5] !=  API_CLEAR_MARKER))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_CLEAR_MARKER_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}
/****************************************** NEED TO DO ERROR CHECKING
// ERROR CHECKING DONE HERE - SHOULD BE DONE IN ANALYZER
	k = 3;
	sa_tx[k++] = WAVE_REQ;
	sa_tx[k++] = SINGLE_12BIT;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData((void *)lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;
	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
	k=485;
	icfreqrx 	 = 	(sa_rx[k++] << 24); 
	icfreqrx	|=	(sa_rx[k++] << 16);
	icfreqrx	|=	(sa_rx[k++] <<  8);
	icfreqrx	|=	(sa_rx[k++]);

	if (icfreq != icfreqrx)
		return AOV_ERR_SET_CFREQ_FAILED;
************************************************************/

	return AOV_ERR_NO_ERROR;
}





int AVCOM_SA_GetMarker ( void *handle, int markerno, double *freq, double *db)
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
	unsigned long icfreqrx;
	float *idbrx;
//	double dbrx;
//	unsigned char *pfreq;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((markerno < -1) || (markerno > 10))
		return AOV_ERR_INVALID_MARKER;

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_GET_MARKER;
	sa_tx[k++]		= markerno;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	// If out of range, or error occured, it will return PACK_ACK with Error
	if ( (sa_rx[3] ==  PACKET_ACK) && (sa_rx[4] ==  API_CMDS) && (sa_rx[5] ==  API_GET_MARKER))
	{
		iResult = sa_rx[7] << 8;
		iResult |= sa_rx[8];
		*freq = 0;
		*db = 0;
		return iResult;
	}
	
	// If not the above packet, then should be API_GET_MARKER else unknown
	if  (sa_rx[4] !=  API_GET_MARKER) 
		return AOV_ERR_UNEXPECTED_CMD;

	// If we get here, then assume valid data
	k=5;
	icfreqrx 	 = 	(sa_rx[k++] << 24); 
	icfreqrx	|=	(sa_rx[k++] << 16);
	icfreqrx	|=	(sa_rx[k++] <<  8);
	icfreqrx	|=	(sa_rx[k++]);
	*freq	= icfreqrx / 10000.F;

//	idbrx		 = 	(sa_rx[k++] << 24); 
//	idbrx		|=	(sa_rx[k++] << 16);
//	idbrx		|=	(sa_rx[k++] <<  8);
//	idbrx		|=	(sa_rx[k++]);
//	*db		= ()idbrx;
	idbrx	= (float *)&sa_rx[k];
	*db	= *idbrx;

	return AOV_ERR_NO_ERROR;
}

int AVCOM_SA_GetCNR ( void *handle, int markerno, double *cnr )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
//	double finc;
	int recvlen;
	int iResult;
	//int rbw;
	float *idbrx;
	//unsigned long icfreq;
	//unsigned char *pfreq;
	//sAOVwaveForm trace;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((markerno < 1) || (markerno > 10))
		return AOV_ERR_INVALID_MARKER;

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_GET_CNR;
	sa_tx[k++]		= markerno;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ( (sa_rx[3] ==  PACKET_ACK) && (sa_rx[4] ==  API_CMDS) && (sa_rx[5] ==  API_GET_CNR) && (sa_rx[6] == 0x01))
	{
		iResult  = sa_rx[7] << 8;
		iResult |= sa_rx[8];
		return iResult;
	}

	if ( (sa_rx[3] !=  API_CMDS) && (sa_rx[4] !=  API_GET_CNR))
		return AOV_ERR_UNEXPECTED_CMD;

	k=6;
	idbrx	= (float *)&sa_rx[k];
	*cnr	= *idbrx;

/****************************************************************************
	if ((iResult = AVCOM_SA_GetSpectrumData((void *)lhandle, &trace)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((freq < trace.frequency[0]) || (freq > trace.frequency[319]))
		return AOV_ERR_FREQ_OOR;
	i = 0;


	finc = (trace.frequency[319] - trace.frequency[0]) / 320;

	i = (int)((freq - trace.frequency[0]) / finc);

	switch (lhandle->cur_settings.rbw)
	{
	case RBW_3MHZ:
		rbw = 3000000;
		break;
	case RBW_1MHZ:
		rbw = 1000000;
		break;
	case RBW_300KHZ:
		rbw = 300000;
		break;
	case RBW_100KHZ:
		rbw = 100000;
		break;
	case RBW_10KHZ:
		rbw = 10000;
		break;
	case RBW_200KHZ:
		rbw = 200000;
		break;
	}

	*cnr =  trace.amplitude[i] - 10* log10(rbw); //+0.52
	***************************************************************/

	return AOV_ERR_NO_ERROR;

}


int AVCOM_SA_GetSNR ( void *handle, int marker1, int marker2, double *db)
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
//	unsigned long icfreqrx;
	float *idbrx;
//	double dbrx;
//	unsigned char *pfreq;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((marker1 < -1) || (marker1 > 10))
		return AOV_ERR_INVALID_MARKER;

	if ((marker2 < -1) || (marker2 > 10))
		return AOV_ERR_INVALID_MARKER;

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_GET_SNR;
	sa_tx[k++]		= marker1;
	sa_tx[k++]		= marker2;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
	k=7;
	idbrx	= (float *)&sa_rx[k];
	*db	= *idbrx;

	return AOV_ERR_NO_ERROR;
}






int AVCOM_SA_SetAverage ( void *handle, int avg)
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
//	unsigned long icfreqrx;
//	float *idbrx;
//	double dbrx;
//	unsigned char *pfreq;
	int txlen;
	int k;
	int recvlen;
	int iResult;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((avg < 0) || (avg > 200))
		return AOV_ERR_INVALID_AVGERAGE;

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_AVERAGE_SET;
	sa_tx[k++]		= avg;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  API_CMDS) && (sa_rx[5] !=  API_AVERAGE_SET))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_AVG_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}
/*
	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;
	k=7;
	idbrx	= (float *)&sa_rx[k];
	*db	= *idbrx;
*/
	return AOV_ERR_NO_ERROR;
}






int AVCOM_SA_SetSweepMode ( void *handle, int mode )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	//unsigned short int data;
//	unsigned long icfreqrx;
//	float *idbrx;
//	double dbrx;
//	unsigned char *pfreq;
	int txlen;
	int k;
	int recvlen;
	int iResult;
//	int loop=1;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if ((mode < 0) || (mode > 1))
		return AOV_ERR_INVALID_SWEEP_MODE;

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_SWEEP_MODE;
	sa_tx[k++]		= mode;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
		return iResult;

	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  API_CMDS) && (sa_rx[5] !=  API_SWEEP_MODE))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
		return AOV_ERR_SET_SWEEP_FAILED;

	Sleep(100);

	return AOV_ERR_NO_ERROR;
}






int AVCOM_SA_Trigger ( void *handle, int flags )
{	
	unsigned char sa_tx[64];
	unsigned char sa_rx[1024];
	int txlen;
	int k;
	int recvlen;
	int iResult;
	int loop=1;
	AOVhandle *lhandle = (AOVhandle *)handle; 

	if (checkforhandle(handle))
		return AOV_HANDLE_NOT_FOUND;

	if (flags != 0)
		return AOV_ERR_INVALID_FLAG;

	k = 3;
	sa_tx[k++]		= API_CMDS;
	sa_tx[k++]		= API_TRIGGER;
	sa_tx[k++]		= flags;
	sa_tx[k++] = ETX;
	txlen = k - 3;
	sa_tx[0] = STX;
	sa_tx[1] = (txlen >> 8) & 0xFF;
	sa_tx[2] =  txlen & 0xFF;
	if ((iResult = AOV_sendData(lhandle, sa_tx, k)) != AOV_ERR_NO_ERROR)
		return iResult;

	while (loop)
	{
		if ((iResult = AOV_recData((void *)lhandle, sa_rx, &recvlen)) != AOV_ERR_NO_ERROR)
			return iResult;
		if (sa_rx[6] != 3)
			loop = 0;
	}

	if ( (sa_rx[3] !=  PACKET_ACK) && (sa_rx[4] !=  API_CMDS) && (sa_rx[5] !=  API_TRIGGER))
		return AOV_ERR_UNEXPECTED_CMD;

	if (sa_rx[6])
	{
		if (recvlen == 5)
			return AOV_ERR_SET_SWEEP_FAILED;
		
		iResult = (sa_rx[7] << 8);
		iResult |= sa_rx[8];
		return iResult;
	}

//	Sleep(200);

	return AOV_ERR_NO_ERROR;
}


/*
int AVCOM_SA_GetNumUSB ( unsigned int *numDevs )
{
	FT_STATUS ftStatus;

	ftStatus = FT_CreateDeviceInfoList(numDevs);
	if (ftStatus != FT_OK) 
		return AOV_ERR_USB_COMM;
	
	return AOV_ERR_NO_ERROR;
}

*/





/*****************************************************************************
 * Avcom of Virginia 
 * API Example Program
 *  
 * This is an example program to illustrate how to use the AVCOM API's.
 * This file and program is provided for illustration purposes.
 * 
 * This program is to be run at a command prompt and connect to an Avcom Spectrum
 * Analyzer with the default Ethernet settings, and will print out the SNR
 * of a peak signal between 1209.5MHz and 1219.5MHz on input 4. The noise level is
 * considered to be +/-1MHz of the peak signal. It will also print the CNR
 * at the noise level.
 *
 * Before and after acquiring the CNR and SNR of the signal, this program
 * will perform arbitrary functions to illustrate how to use the API's.
 * Results are written to FILE_OUTPUT and CONSOLE_OUTPUT.
 * 
 * To compile, create a project to include the following files as your 
 * developing environment specifies:
 *			- aovspecan.dll
 *			- aovspecan.lib
 *			- aovspecan.h
 * 
 * Copyright Avcom of Virginia (c) 2010
 * 
 *****************************************************************************/
//#define _BASIC_API		1
#define _ADVANCED_API	1

/*** AOVSPECAN.H: API header file ***/
#include "../api/aovspecan.h"

//#define CONSOLE_OUTPUT	CONSOLE_OUTPUT
#define CONSOLE_OUTPUT	stderr
#define FILE_OUTPUT		stdout

#if !defined(AOV_DEFAULT) 
#define AOV_DEFAULT	0
#endif

//Wait after each API Call?
//#define WAIT_AFTER_API Sleep(API_WAIT)

#if !defined(WAIT_AFTER_API)
#define WAIT_AFTER_API 
#endif

//If so, then how many milliseconds?
#define API_WAIT 00
#define BRUT_NUM 10

//Socket Timeout (seconds)
#define RECV_TIMEOUT	10

/*** Additional header files specific to example program ***/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")


	void		*sa_handle;
	char		*ip = "192.168.118.243";
//	char		*ip = "20.20.20.20";
	int			port = 26482;
	int			sweeps;
	double		freq, db, snr;
	double		span;
	double		fullSpan;
	double		cnr;
	double		cFreq;
	double		cFreqTmp;
	double		fLow;
	double		fHigh;
	int			iResult,rl,input,lnb_en,lnb_v,lnb_f, idx1, idx2, idx3, idx4;
	sAOVlnb		lnbControl;
	eAOV_RBW	rbw;
	int			avg;

	int			i_center;
	int			i_center_tmp;
	int			i_span_seg;
	int			i_span_full;
	int			i_step_size;
	int			i_sweeps;
	int			i_high;
	int			i_low;

	struct timeval		tv;
	BOOL		nagle;
	
	unsigned long int	idx_p,idx_f;

	static		sAOVwaveForm waveform;

typedef struct AOVhandle_ {
	int						socketfd;
	struct addrinfo			*servinfo;
	char					*ip;
	unsigned short			portno;
	char					serial[16];
	unsigned char			avail_inputs;
	int						version_major;
	int						version_minor;
	int						version_patch;


} AOVhandle;

AOVhandle	*lh;

int gilatTest(void);

DWORD	tmo;

void pexit(int code, char *module)
{
	int ir;
	fprintf(FILE_OUTPUT, "Program Failed. Code: %i  Module: %s\n",code,module);
	ir=WSAGetLastError();
	fprintf(FILE_OUTPUT, "WSA Last Error: %i \n",ir);
	AVCOM_SA_Deinitialize();
	//while(1);
	exit(code);
}


void pnotify(int code, char *module)
{
	fprintf(FILE_OUTPUT, "Program Error. Code: %i  Module: %s\n",code,module);
	pexit(code,module);
}

int doBrutality(void) {
	int ir;
	int i;
	ir=AOV_ERR_NO_ERROR;

	// Set Auto Sweep Mode to AUTO
	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode BB");
		return(iResult);
	}

	input = 1;
	for(i=0;i<BRUT_NUM;i++)
	{
		input++;
		if (input > 4) input =1;
		if ( (ir = AVCOM_SA_SetInput ( sa_handle, input, AOV_SWITCH_INPUT_ONLY )) != AOV_ERR_NO_ERROR)
		{
			pnotify(ir,"AVCOM_SA_SetInput BB");
			return(ir);
		}
	}
	
	span =1;
	for(i=0;i<BRUT_NUM;i++)
	{
		span++;
		if (input > 100) span =1;
		// Change the span 
		if ( (iResult = AVCOM_SA_SetSpan ( sa_handle, span )) != AOV_ERR_NO_ERROR)
		{
			pnotify(iResult,"AVCOM_SA_SetSpan BB");
			return(iResult);
		}
	}

	cFreq =750;
	for(i=0;i<BRUT_NUM;i++)
	{
		// Set the Center Frequency
		if ( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle,cFreq)) != AOV_ERR_NO_ERROR)
		{
			cFreq++;
			if (input > 1500) cFreq =750;
			pnotify(iResult,"AVCOM_SA_SetCenterFrequency BB");
			return(iResult);
		}
	}

	rbw = RBW_10KHZ;
	for(i=0;i<BRUT_NUM;i++)
	{
		// Change the rbw
		if ( (iResult = AVCOM_SA_SetResBandwidth ( sa_handle, rbw )) != AOV_ERR_NO_ERROR)
		{
			rbw ^= (RBW_10KHZ | RBW_300KHZ);
			pnotify(iResult,"AVCOM_SA_SetResBandwidth BB");
			return(iResult);
		}
	}

	// Set Auto Sweep Mode to AUTO
	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode BB");
		return(iResult);
	}

	input = 1;
	for(i=0;i<BRUT_NUM;i++)
	{
		input++;
		if (input > 4) input =1;
		if ( (ir = AVCOM_SA_SetInput ( sa_handle, input, AOV_SWITCH_INPUT_ONLY )) != AOV_ERR_NO_ERROR)
		{
			pnotify(ir,"AVCOM_SA_SetInput BB");
			return(ir);
		}
	}
	
	span =1;
	for(i=0;i<BRUT_NUM;i++)
	{
		span++;
		if (input > 100) span =1;
		// Change the span 
		if ( (iResult = AVCOM_SA_SetSpan ( sa_handle, span )) != AOV_ERR_NO_ERROR)
		{
			pnotify(iResult,"AVCOM_SA_SetSpan BB");
			return(iResult);
		}
	}

	cFreq =750;
	for(i=0;i<BRUT_NUM;i++)
	{
		// Set the Center Frequency
		if ( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle,cFreq)) != AOV_ERR_NO_ERROR)
		{
			cFreq++;
			if (input > 1500) cFreq =750;
			pnotify(iResult,"AVCOM_SA_SetCenterFrequency BB");
			return(iResult);
		}
	}

	rbw = RBW_10KHZ;
	for(i=0;i<BRUT_NUM;i++)
	{
		// Change the rbw
		if ( (iResult = AVCOM_SA_SetResBandwidth ( sa_handle, rbw )) != AOV_ERR_NO_ERROR)
		{
			rbw ^= (RBW_10KHZ | RBW_300KHZ);
			pnotify(iResult,"AVCOM_SA_SetResBandwidth BB");
			return(iResult);
		}
	}


	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
	{
		fprintf(FILE_OUTPUT, "BB Iteration: %i\n",1);
		pnotify(iResult,"AVCOM_SA_SetSweepMode AUTO BB 8");
		return(iResult);
	}



	for(i=0;i<BRUT_NUM;i++)
	{
		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_SetSweepMode OFF BB 8");
			return(iResult);
		}
		if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_Trigger BB 2");
			return(iResult);
		}

		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_SetSweepMode AUTO BB 2");
			return(iResult);
		}

	}

	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
	{
		fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
		pnotify(iResult,"AVCOM_SA_SetSweepMode OFF BB 3");
		return(iResult);
	}

	avg = 0;
	for(i=0;i<BRUT_NUM;i++)
	{
		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_SetSweepMode OFF BB 4");
			return(iResult);
		}

		if (avg == 0) avg = 2;
		else if (avg == 64) avg = 0;
		else avg *= 2;
		if ( (iResult = AVCOM_SA_SetAverage(sa_handle,avg)) != AOV_ERR_NO_ERROR)
		{
			pnotify(iResult,"AVCOM_SA_SetAverage");
			return(iResult);
		}
		if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_Trigger BB 2");
			return(iResult);
		}

		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_SetSweepMode AUTO BB 2");
			return(iResult);
		}

	}


	avg = 0;
	for(i=0;i<BRUT_NUM;i++)
	{
		if (avg == 0) avg = 2;
		else if (avg == 64) avg = 0;
		else avg *= 2;

		if ( (iResult = AVCOM_SA_SetAverage(sa_handle,avg)) != AOV_ERR_NO_ERROR)
		{
			pnotify(iResult,"AVCOM_SA_SetAverage");
			return(iResult);
		}
		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_SetSweepMode OFF BB 5");
			return(iResult);
		}
		if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_Trigger BB 2");
			return(iResult);
		}

		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_SetSweepMode AUTO BB 2");
			return(iResult);
		}

	}

	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
	{
		fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
		pnotify(iResult,"AVCOM_SA_SetSweepMode OFF BB 7");
		return(iResult);
	}

	for(i=0;i<BRUT_NUM;i++)
	{
		if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_Trigger BB 3");
			return(iResult);
		}
	}

	for(i=0;i<BRUT_NUM;i++)
	{
		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_SetSweepMode OFF BB 2");
			return(iResult);
		}

		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
		{
			fprintf(FILE_OUTPUT, "BB Iteration: %i\n",i);
			pnotify(iResult,"AVCOM_SA_SetSweepMode AUTO BB 2");
			return(iResult);
		}

	}

	return ir;
}


int doTheTest(void) {
/*
	// Set Auto Sweep Mode to AUTO 
	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode AUTO 1");
		return(iResult);
	}
	WAIT_AFTER_API;

	input = 2;
	// First change the input
	if ( (iResult = AVCOM_SA_SetInput ( sa_handle, input, AOV_SWITCH_INPUT_ONLY )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetInput 1");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Change the span 
	if ( (iResult = AVCOM_SA_SetSpan ( sa_handle, span )) != AOV_ERR_NO_ERROR)
	{
			pnotify(iResult,"AVCOM_SA_SetSpan 1");
			return(iResult);
	}
	WAIT_AFTER_API;

	// Set the Center Frequency
	if ( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle,cFreq)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetCenterFrequency 1");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Change the rbw
	if ( (iResult = AVCOM_SA_SetResBandwidth ( sa_handle, rbw )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetResBandwidth 1");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Change the Reference Level
	if ( (iResult = AVCOM_SA_SetRefLevel ( sa_handle, rl )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetRefLevel 1");
		return(iResult);
	}
	WAIT_AFTER_API;

		// Set Averaging to 8 traces

	if ( (iResult = AVCOM_SA_SetAverage(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAverage 1");
		return(iResult);
	}

	WAIT_AFTER_API;
*/
	// Set Auto Sweep Mode to OFF
	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode 2");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Get Trace Data


	// Trigger a sweep

	if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_Trigger 1");
		return(iResult);
	}
	WAIT_AFTER_API;

	//Get the waveform
	if ( (iResult = AVCOM_SA_GetSpectrumData(sa_handle, &waveform)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_GetSpectrumData 1");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Modify Center Frequency

	cFreq=750.0;
	if ( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle,cFreq)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetCenterFrequency 2");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Set Auto Sweep Mode to OFF 
	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode 3");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Set Averaging to 8 traces
	if ( (iResult = AVCOM_SA_SetAverage(sa_handle,64)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAverage 1");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Trigger a sweep
	if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_Trigger 2");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Set Averaging to 8 traces
	if ( (iResult = AVCOM_SA_SetAverage(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAverage 2");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Trigger a sweep
	if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_Trigger 3");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Set Averaging to 8 traces
	if ( (iResult = AVCOM_SA_SetAverage(sa_handle,16)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAverage3");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Trigger a sweep
	if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_Trigger 4");
		return(iResult);
	}
	WAIT_AFTER_API;
/*
	// Set marker 1 at the peak frequency
	if ( (iResult = AVCOM_SA_SetMarkerOpt(sa_handle,1,AOV_MARKER_MAX)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetMarkerOpt");
		return(iResult);
	}
	WAIT_AFTER_API;
	
	// Set marker 2 at +20% of peak frequency
	// First acquire marker1's frequency
	if ( (iResult = AVCOM_SA_GetMarker(sa_handle,1,&freq,&db)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_GetMarker");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Now set Marker 2 to 20% span from Marker 1
	// Note that we do not want to place the marker 
	// outside of the current sweep range.
	freq = 750;
	freq += ( span * 0.10F );
	if (freq > (cFreq + .5F * span))
		freq -= (span * 0.20F);
	if ( (iResult = AVCOM_SA_SetMarkerFreq (sa_handle,2,freq)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetMarkerFreq ");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Acquire Signal-to-Noise between markers
	if ( (iResult = AVCOM_SA_GetSNR(sa_handle,1,2,&snr)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_GetSNR 1");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Acquire a Carrier Noise calculation
	if ( (iResult = AVCOM_SA_GetCNR(sa_handle,2,&cnr)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_GetCNR 1");
		return(iResult);
	}


	WAIT_AFTER_API;
	// Clear Markers
	if ( (iResult = AVCOM_SA_ClearMarker(sa_handle,AOV_MARKER_ALL)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_ClearMarker 1");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Set Auto Sweep Mode to AUTO 
	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode 1");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Change all settings - May be executed through individual commands as well 
	span	= 100.0;		// Span: 10 MHz
	cFreq	= 1600.0;		// Freq: 520 MHz 
	rl		= -10;			// Reference level: -50dB 
    rbw     = RBW_300KHZ;	// Resolution Bandwidth: 100KHz 
	input   = 3;			// Input: 3

	// Change to Different Frequency, Span, Input, Reference Level, RBW
	if ( (iResult = AVCOM_SA_SetAllSettings(sa_handle,cFreq,span,rl,rbw,input,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAllSettings 3");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Change all settings - May be executed through individual commands as well 
//	span	= 100.0;		// Span: 10 MHz
//	cFreq	= 1600.0;		// Freq: 520 MHz 
//	rl		= -30;			// Reference level: -50dB 
//  rbw     = RBW_300KHZ;	// Resolution Bandwidth: 100KHz 
	input   = 1;			// Input: 3

	// Change to Different Frequency, Span, Input, Reference Level, RBW
	if ( (iResult = AVCOM_SA_SetAllSettings(sa_handle,cFreq,span,rl,rbw,input,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAllSettings 4");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Change all settings - May be executed through individual commands as well 
//	span	= 100.0;		// Span: 10 MHz
//	cFreq	= 1600.0;		// Freq: 520 MHz 
	rl		= -10;			// Reference level: -50dB 
//  rbw     = RBW_300KHZ;	// Resolution Bandwidth: 100KHz 
	input   = 3;			// Input: 3

	// Change to Different Frequency, Span, Input, Reference Level, RBW
	if ( (iResult = AVCOM_SA_SetAllSettings(sa_handle,cFreq,span,rl,rbw,input,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAllSettings 5");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Change all settings - May be executed through individual commands as well 
	span	= 200.0;		// Span: 10 MHz
	cFreq	= 1601.0;		// Freq: 520 MHz 
	rl		= -60;			// Reference level: -50dB 
    rbw     = RBW_100KHZ;	// Resolution Bandwidth: 100KHz 
	input   = 4;			// Input: 3

	// Change to Different Frequency, Span, Input, Reference Level, RBW
	if ( (iResult = AVCOM_SA_SetAllSettings(sa_handle,cFreq,span,rl,rbw,input,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAllSettings 5");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Change all settings - May be executed through individual commands as well 
	span	= 100.0;		// Span: 10 MHz
	cFreq	= 1600.0;		// Freq: 520 MHz 
	rl		= -30;			// Reference level: -50dB 
    rbw     = RBW_300KHZ;	// Resolution Bandwidth: 100KHz 
	input   = 1;			// Input: 3

	// Change to Different Frequency, Span, Input, Reference Level, RBW
	if ( (iResult = AVCOM_SA_SetAllSettings(sa_handle,cFreq,span,rl,rbw,input,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAllSettings 6");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Trigger a sweep
	if ( (iResult = AVCOM_SA_Trigger(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_Trigger 5");
		return(iResult);
	}
	WAIT_AFTER_API;

	//Get the wavweform
	if ( (iResult = AVCOM_SA_GetSpectrumData(sa_handle, &waveform)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_GetSpectrumData 2");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Reset Average 
	// This is good practice if also communicating via the AvcomGUI
	if ( (iResult = AVCOM_SA_SetAverage(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAverage");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Set Auto Sweep Mode to Auto 
	// Required before communicating with the AvcomGUI
	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode 4");
		return(iResult);
	}
	WAIT_AFTER_API;


	input   = 1;
	if ( (iResult = AVCOM_SA_SetInput ( sa_handle, input, AOV_DEFAULT )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetInput 2");
		return(iResult);
	}

	input   = 2;
	if ( (iResult = AVCOM_SA_SetInput ( sa_handle, input, AOV_DEFAULT )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetInput 3");
		return(iResult);
	}

	input   = 3;
	if ( (iResult = AVCOM_SA_SetInput ( sa_handle, input, AOV_DEFAULT )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetInput 4");
		return(iResult);
	}



	// Change the span 
	span	= 20.0;
	if ( (iResult = AVCOM_SA_SetSpan ( sa_handle, span )) != AOV_ERR_NO_ERROR)
	{
			pnotify(iResult,"AVCOM_SA_SetSpan 2");
			return(iResult);
	}

	// Change the span 
	span	= 30.0;
	if ( (iResult = AVCOM_SA_SetSpan ( sa_handle, span )) != AOV_ERR_NO_ERROR)
	{
			pnotify(iResult,"AVCOM_SA_SetSpan 3");
			return(iResult);
	}

	rbw     = RBW_100KHZ;
	if ( (iResult = AVCOM_SA_SetResBandwidth ( sa_handle, rbw )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetResBandwidth 2");
		return(iResult);
	}
	
	rbw     = RBW_10KHZ;
	if ( (iResult = AVCOM_SA_SetResBandwidth ( sa_handle, rbw )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetResBandwidth 3");
		return(iResult);
	}

	rl		= -30;
	// Change the Reference Level
	if ( (iResult = AVCOM_SA_SetRefLevel ( sa_handle, rl )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetRefLevel 2");
		return(iResult);
	}

	rl		= -15;
	// Change the Reference Level
	if ( (iResult = AVCOM_SA_SetRefLevel ( sa_handle, rl )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetRefLevel 3");
		return(iResult);
	}

	cFreq=651.0;
	if ( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle,cFreq)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetCenterFrequency 3");
		return(iResult);
	}

	cFreq=725;
	if ( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle,cFreq)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetCenterFrequency 7");
		return(iResult);
	}

	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode 5");
		return(iResult);
	}

	input   = 3;
	if ( (iResult = AVCOM_SA_SetInput ( sa_handle, input, AOV_DEFAULT )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetInput 6");
		return(iResult);
	}

	input   = 2;
	if ( (iResult = AVCOM_SA_SetInput ( sa_handle, input, AOV_DEFAULT )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetInput 7");
		return(iResult);
	}


	// Change the span 
	span	= 20.0;
	if ( (iResult = AVCOM_SA_SetSpan ( sa_handle, span )) != AOV_ERR_NO_ERROR)
	{
			pnotify(iResult,"AVCOM_SA_SetSpan 3");
			return(iResult);
	}

	// Change the span 
	span	= 30.0;
	if ( (iResult = AVCOM_SA_SetSpan ( sa_handle, span )) != AOV_ERR_NO_ERROR)
	{
			pnotify(iResult,"AVCOM_SA_SetSpan 4");
			return(iResult);
	}

	rbw     = RBW_100KHZ;
	if ( (iResult = AVCOM_SA_SetResBandwidth ( sa_handle, rbw )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetResBandwidth 3");
		return(iResult);
	}
	
	rbw     = RBW_10KHZ;
	if ( (iResult = AVCOM_SA_SetResBandwidth ( sa_handle, rbw )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetResBandwidth 4");
		return(iResult);
	}

	rl		= -30;
	// Change the Reference Level
	if ( (iResult = AVCOM_SA_SetRefLevel ( sa_handle, rl )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetRefLevel 4");
		return(iResult);
	}

	rl		= -35;
	// Change the Reference Level
	if ( (iResult = AVCOM_SA_SetRefLevel ( sa_handle, rl )) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetRefLevel 5");
		return(iResult);
	}

	cFreq=651.0;
	if ( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle,cFreq)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetCenterFrequency 5");
		return(iResult);
	}

	cFreq=725;
	if ( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle,cFreq)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetCenterFrequency 6");
		return(iResult);
	}

	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetSweepMode 6");
		return(iResult);
	}


	if ( (iResult = AVCOM_SA_SetAverage(sa_handle,64)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAverage 4");
		return(iResult);
	}
	// Set Averaging to 8 traces
	if ( (iResult = AVCOM_SA_SetAverage(sa_handle,0)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAverage 5");
		return(iResult);
	}

	// Set Averaging to 8 traces
	if ( (iResult = AVCOM_SA_SetAverage(sa_handle,16)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetAverage 6");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Set marker 1 at the peak frequency
	if ( (iResult = AVCOM_SA_SetMarkerOpt(sa_handle,1,AOV_MARKER_MAX)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetMarkerOpt 2");
		return(iResult);
	}
	WAIT_AFTER_API;
	
	// Set marker 2 at +20% of peak frequency
	// First acquire marker1's frequency
	if ( (iResult = AVCOM_SA_GetMarker(sa_handle,1,&freq,&db)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_GetMarker 2");
		return(iResult);
	}
	WAIT_AFTER_API;

	// Now set Marker 2 to 20% span from Marker 1
	// Note that we do not want to place the marker 
	// outside of the current sweep range.
//	freq = 750;
	freq += ( span * 0.10F );
	if (freq > (cFreq + .5F * span))
		freq -= (span * 0.20F);
	if ( (iResult = AVCOM_SA_SetMarkerFreq (sa_handle,2,freq)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_SetMarkerFreq 2");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Acquire Signal-to-Noise between markers
	if ( (iResult = AVCOM_SA_GetSNR(sa_handle,1,2,&snr)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_GetSNR 2");
		return(iResult);
	}
	WAIT_AFTER_API;


	// Acquire a Carrier Noise calculation
	if ( (iResult = AVCOM_SA_GetCNR(sa_handle,2,&cnr)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_GetCNR 2");
		return(iResult);
	}


	WAIT_AFTER_API;
	// Clear Markers
	if ( (iResult = AVCOM_SA_ClearMarker(sa_handle,AOV_MARKER_ALL)) != AOV_ERR_NO_ERROR)
	{
		pnotify(iResult,"AVCOM_SA_ClearMarker 2");
		return(iResult);
	}
	WAIT_AFTER_API;

*/










	return(iResult);


}

int main (void)
{
	int retry = 0;
	int j;


	for (j = 0; j < 0xFFFFFFFF; j++)
	{
		fprintf(stdout, "\nLoop  %i\n", j);
		iResult = gilatTest();
	}
	return 0;

	fprintf(FILE_OUTPUT, "API_HAMMER begin.\n");
	
	/* Initialize API environment and process any errors */
	if ( (iResult = AVCOM_SA_Initialize()) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult,"AVCOM_SA_Initialize");
	}
	WAIT_AFTER_API;

	/* Register Spectrum Analyzer */
	if ( (iResult = AVCOM_SA_Register(&sa_handle, ip, port, AOV_DEFAULT, AOV_DEFAULT)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult,"AVCOM_SA_Register");
	}
	WAIT_AFTER_API;

	/* Connect to SA*/
	if ( (iResult = AVCOM_SA_Connect(sa_handle)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult,"AVCOM_SA_Connect()");
	}
	/*if ( (iResult = reConnect(FALSE,FALSE)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult,"reConnect 1");
	}*/
/*
	// Set Auto Sweep Mode to AUTO
	if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_AUTO)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult,"AVCOM_SA_SetSweepMode 0");
	}
	WAIT_AFTER_API;
*/
	
	/* Change all settings - May be executed through individual commands as well */

	//Integer values in KHZ
	i_low		= 400000;					    //Even
	i_span_seg	= 640;						    //Even
	i_step_size = 2;						    //Even
//	i_center	= i_low+(i_span_seg)/2;		    //Even
	i_center	= 1500;
	//i_span_full = 1300000;				    //Even
	i_span_full = 64000;					    //Even
	i_sweeps	= i_span_full/i_span_seg;	    //Even
	i_high		= i_low + i_sweeps*i_span_seg;	//Even

	//Convert to float
	cFreq	 = ((double)i_center)/1000.0;
	fullSpan = ((double)i_span_full)/1000.0;
	fLow	 = ((double)i_low)/1000.0;
	fHigh	 = ((double)i_high)/1000.0;
	span	 = ((double)i_span_seg)/1000.0;

	sweeps	 = i_sweeps;

	rl		 = -70;			
    rbw      = RBW_10KHZ;	
	input    = 2;
	
	idx_p=0;
	idx_f=0;

	//Begin brutal pounding (rapid successive API calls in tight loops)
	iResult = AOV_ERR_NO_ERROR;
	for (j = 0; j < 0x9000000; j++)
	{

//		iResult=doBrutality();
		if (iResult != AOV_ERR_NO_ERROR)
		{
			pexit(iResult,"Failed brutal beating...");
		}
		else
		{
			fprintf(FILE_OUTPUT,"Survived brutal beating...\n");
		}


		// Change all settings - May be executed through individual commands as well 
		span	= 100.0;		// Span: 10 MHz
		cFreq	= 1600.0;		// Freq: 520 MHz 
		rl		= -30;			// Reference level: -50dB 
		rbw     = RBW_300KHZ;	// Resolution Bandwidth: 100KHz 
		input   = 3;			// Input: 3

		// Change to Different Frequency, Span, Input, Reference Level, RBW
		if ( (iResult = AVCOM_SA_SetAllSettings(sa_handle,cFreq,span,rl,rbw,input,0)) != AOV_ERR_NO_ERROR)
		{
			pnotify(iResult,"AVCOM_SA_SetAllSettings 0");
			return(iResult);
		}

		//Begin Test
		iResult = AOV_ERR_NO_ERROR;
		// Set Auto Sweep Mode to AUTO

		if ( (iResult = AVCOM_SA_SetSweepMode(sa_handle,AOV_SWEEP_OFF)) != AOV_ERR_NO_ERROR)
		{
			pexit(iResult,"AVCOM_SA_SetSweepMode 0");
		}

		for(idx1=0;idx1<200;idx1++)
		{
			fprintf(FILE_OUTPUT,"Test loop iteration: %i %i\n",j,idx1);
			cFreq	=	750.0;
			rl		=	-70;
			span	=	2;
			rbw     =	RBW_10KHZ;	
			if (iResult == AOV_ERR_NO_ERROR) iResult=doTheTest();
			if (iResult != AOV_ERR_NO_ERROR)
			{
				idx_f++;
				//fprintf(FILE_OUTPUT, "FAIL.\n");

			}
			else
			{
				idx_p++;
				//fprintf(FILE_OUTPUT, "SUCCESS: SNR: %f   CNR at Marker 2: %f\n", snr, cnr);


			}

		}
	}
	fprintf(FILE_OUTPUT,"Results. PASS: %8i             FAIL: %8i\n",idx_p,idx_f);

	// Disconnect Analyzer, Terminate handle, and exit environment	
	// NOTE: It is good practice to disconnect/terminate each handle when finished although 
	// AVCOM_SA_Deinitialize(); will attempt to disconnect and terminate all handles 
	if ( (iResult = AVCOM_SA_Disconnect(sa_handle)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult,"AVCOM_SA_Disconnect");
	}
	WAIT_AFTER_API;

	if ( (iResult = AVCOM_SA_Unregister(sa_handle)) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult,"AVCOM_SA_Unregister");
	}

	if ( (iResult = AVCOM_SA_Deinitialize()) != AOV_ERR_NO_ERROR)
	{
		pexit(iResult,"AVCOM_SA_Deinitialize");
	}
	WAIT_AFTER_API;

	fprintf(FILE_OUTPUT, "API_HAMMER end.\n");

	return 0;


}






































int gilatTest(void)
{
	int iResult = AOV_ERR_NO_ERROR;

	if( (iResult = AVCOM_SA_Initialize()) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_Initialize 1"); return(iResult);
	}

	if( (iResult = AVCOM_SA_Register(&sa_handle, ip, port, AOV_DEFAULT, 1)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_Initialize 1"); return(iResult);
	}

	input = 1;
	if( (iResult = AVCOM_SA_SetInput(sa_handle, input, 1)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetInput 1"); return(iResult);
	}

	rbw = RBW_100KHZ;
	if( (iResult = AVCOM_SA_SetResBandwidth(sa_handle, rbw)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetResBandwidth 1"); return(iResult);
	}

	cFreq = 651.0;
	if( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle, cFreq)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetCenterFrequency 1"); return(iResult);
	}

	span = 100.0;
	if( (iResult = AVCOM_SA_SetSpan(sa_handle, span)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetSpan 1"); return(iResult);
	}

	if( (iResult = AVCOM_SA_SetMarkerOpt(sa_handle, 1, AOV_MARKER_MAX)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetMarkerOpt 1"); return(iResult);
	}

	rl = -20;
	if( (iResult = AVCOM_SA_SetRefLevel(sa_handle, rl)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetRefLevel 1"); return(iResult);
	}

	rl = -15;
	if( (iResult = AVCOM_SA_SetRefLevel(sa_handle, rl)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetRefLevel 2"); return(iResult);
	}

	if( (iResult = AVCOM_SA_GetMarker(sa_handle, 1, &freq, &db)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_GetMarker 1"); return(iResult);
	}
	fprintf(stdout, "GilatTest AVCOM_SA_GetMarker 1 f=%f db=%f\n", freq, db);

	if( (iResult = AVCOM_SA_ClearMarker(sa_handle, 1)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_ClearMarker 1"); return(iResult);
	}

	input = 2;
	if( (iResult = AVCOM_SA_SetInput(sa_handle, input, 1)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetInput 2"); return(iResult);
	}

	rbw = RBW_100KHZ;
	if( (iResult = AVCOM_SA_SetResBandwidth(sa_handle, rbw)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetResBandwidth 2"); return(iResult);
	}

	rl = -65;
	if( (iResult = AVCOM_SA_SetRefLevel(sa_handle, rl)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetRefLevel 3"); return(iResult);
	}

	cFreq = 751.0;
	if( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle, cFreq)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetCenterFrequency 2"); return(iResult);
	}

	span = 150.0;
	if( (iResult = AVCOM_SA_SetSpan(sa_handle, span)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetSpan 2"); return(iResult);
	}

	if( (iResult = AVCOM_SA_SetMarkerOpt(sa_handle, 1, AOV_MARKER_MAX)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetMarkerOpt 2"); return(iResult);
	}

	if( (iResult = AVCOM_SA_GetMarker(sa_handle, 1, &freq, &db)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_GetMarker 2"); return(iResult);
	}
	fprintf(stdout, "GilatTest AVCOM_SA_GetMarker 2 f=%f db=%f\n", freq, db);

	if( (iResult = AVCOM_SA_ClearMarker(sa_handle, 1)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_ClearMarker 2"); return(iResult);
	}

	input = 3;
	if( (iResult = AVCOM_SA_SetInput(sa_handle, input, 1)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetInput 3"); return(iResult);
	}

	rbw = RBW_100KHZ;
	if( (iResult = AVCOM_SA_SetResBandwidth(sa_handle, rbw)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetResBandwidth 3"); return(iResult);
	}

	cFreq = 851.0;
	if( (iResult = AVCOM_SA_SetCenterFrequency(sa_handle, cFreq)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetCenterFrequency 3"); return(iResult);
	}

	span = 200.0;
	if( (iResult = AVCOM_SA_SetSpan(sa_handle, span)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetSpan 3"); return(iResult);
	}

	rl = -5;
	if( (iResult = AVCOM_SA_SetRefLevel(sa_handle, rl)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetRefLevel 4"); return(iResult);
	}

	if( (iResult = AVCOM_SA_SetMarkerOpt(sa_handle, 1, AOV_MARKER_MAX)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetMarkerOpt 3"); return(iResult);
	}

	if( (iResult = AVCOM_SA_GetMarker(sa_handle, 1, &freq, &db)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_GetMarker 3"); return(iResult);
	}
	fprintf(stdout, "GilatTest AVCOM_SA_GetMarker 3 f=%f db=%f\n", freq, db);

	if( (iResult = AVCOM_SA_GetCNR(sa_handle, 1, &cnr)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_GetCNR 1"); return(iResult);
	}
	fprintf(stdout, "GilatTest AVCOM_SA_GetCNR 1 cnr=%f\n", cnr);

	if( (iResult = AVCOM_SA_SetMarkerFreq(sa_handle, 2, freq+5)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_SetMarkerFreq 1"); return(iResult);
	}

	if( (iResult = AVCOM_SA_GetSNR(sa_handle, 1, 2, &snr)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_GetSNR 3"); return(iResult);
	}
	fprintf(stdout, "AVCOM_SA_GetSNR 1 snr=%f\n", snr);

	if( (iResult = AVCOM_SA_ClearMarker(sa_handle, 1)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_ClearMarker 3"); return(iResult);
	}

	if( (iResult = AVCOM_SA_Unregister(sa_handle)) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_Unregister 1"); return(iResult);
	}

	if( (iResult = AVCOM_SA_Deinitialize()) != AOV_ERR_NO_ERROR) {
		pnotify(iResult,"GilatTest AVCOM_SA_Deinitialize 1"); return(iResult);
	}

	return iResult;
}

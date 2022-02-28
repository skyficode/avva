/*************************************************************************//**
 * @file aovspecan.h
 * @author Avcom of Virginia
 * @version v0.0.43
 *
 * @section LICENSE
 *
 * This software is Copyright (c) 2010 Avcom of Virginia.
 * All Rights Reserved.
 *
 * @section DESCRIPTION
 *
 * This is the API Library for use with Avcom Spectrum Analyzers.
 *
 * Prior to using the API set, the environment must be initialized by first
 * calling AVCOM_SA_Initialize(). This sets up a few global parameters,
 * particularly a dynamic linked list which is useful in maintaining handle
 * information.
 *
 * After which, each spectrum analyzer can be registered a handle through
 * AVCOM_SA_Register(). 
 *
 * To recap:
 *	- Initialize the API Environment
 *	- Register any number of Avcom Spectrum Analyzers
 *	- Connect to Analyzer(s) if not done in the Register command
 *	- Perform custom tasks
 *	- Disconnect Analyzer(s)
 *	- Unregister Analyzer(s)
 *	- Deinitialize API Environment
 *	- Exit Program 
 *
 * @section EXAMPLE-PROGRAM
 *
 * For reference and example program aov_example.c is attached that will illustrate 
 * basic usage of this API set.
 *
 * @section WARRANTY
 *
 * This software is provided for your convince and without any warranty. 
 * In no event shall Avcom of Virginia or its employees be liable for any lost 
 * profits, revenue, sales, data or costs of procurement of substitute goods 
 * or services, property damage, interruption of business, loss of business 
 * information or for any indirect, incidental, economic or consequential damages.
 *
 * @example example.c
 *****************************************************************************/



#pragma once
#ifndef AOVSPECAN_H
#define AOVSPECAN_H

/* The following is important and used to distinguish between different IDE's */
#if (defined(_WIN32) && defined(_USRDLL))
#define EXPORT __declspec(dllexport)
#elif (defined(_WIN32))
#define EXPORT __declspec(dllimport)
#else
#define EXPORT
#endif


/*! Enum for True/False options. Typically found inside structures */
typedef enum eAOV_bool_ {
	FALSE,							//!< Boolean FALSE
	TRUE							//!< Boolean TRUE
} eAOV_bool;
//#endif


/*! Structure is used for retrieving waveform data from the Avcom 
 *  Spectrum Analyzer */
typedef struct sAOVwaveForm_ {
    double			amplitude[320]; //!< Amplitude data in dB
	double			frequency[320]; //!< Frequency in MHz
} sAOVwaveForm;




/*! Structure for passing LNB Power options to Avcom Spectrum Analyzers */
typedef struct sAOV_LNB_ {
	eAOV_bool				enable; //!< LNB Power enable
	int						volt;	//!< Voltage, either 13 or 18. Ignored if (enable == FALSE)
	eAOV_bool				khz_22; //!< 22KHz Option. Ignored if (enable == FALSE)
} sAOVlnb;



	
/*! Enumeration options for Resolution Bandwidth Options */
typedef enum eAOV_RBW_ {
	RBW_3MHZ		= 0x80,	//!< Set Resolution Bandwidth to 3MHz (if supported)
	RBW_1MHZ		= 0x40,	//!< Set Resolution Bandwidth to 1MHz (if supported)
	RBW_300KHZ		= 0x20,	//!< Set Resolution Bandwidth to 300KHz (if supported)
	RBW_100KHZ		= 0x10,	//!< Set Resolution Bandwidth to 100KHz (if supported)
	RBW_10KHZ		= 0x08,	//!< Set Resolution Bandwidth to 10KHz (if supported)
	RBW_200KHZ		= 0x02,	//!< Set Resolution Bandwidth to 200KHz (if supported)
}eAOV_RBW;



/*! Enumeration options for AVCOM_SA_SetMarkerOpt() */
typedef enum eMarkerOpt_ {
	AOV_MARKER_MAX	= 1,	//!< Set frequency marker to the peak amplitude
	AOV_MARKER_MIN	= 2,	//!< Set frequency marker to the minimum amplitude
} eMarkerOpt;


/*! Enumeration options for Marker Numbering */
typedef enum eMarkerNum_ {
	AOV_MARKER_1	= 1,	//!< Marker 1
	AOV_MARKER_2,			//!< Marker 2
	AOV_MARKER_3,			//!< Marker 3
	AOV_MARKER_4,			//!< Marker 4
	AOV_MARKER_5,			//!< Marker 5
	AOV_MARKER_6,			//!< Marker 6
	AOV_MARKER_7,			//!< Marker 7
	AOV_MARKER_8,			//!< Marker 8
	AOV_MARKER_9,			//!< Marker 9
	AOV_MARKER_10,			//!< Marker 10
	AOV_MARKER_ALL	= -1,	//!< All Markers: Option for clearing all markers
} eMarkerNum;

/*! Enumeration options for AVCOM_SA_SetSweepMode() */
typedef enum eSweepMode_ {
	AOV_SWEEP_OFF,			//!< Turn Auto Sweep mode OFF
	AOV_SWEEP_AUTO,			//!< Turn Auto Sweep mode ON
} eSweepMode;



//extern "C"


/*************************************************************************//**
 * Clears a specified marker or all markers
 * 
 * @return Status of function call
 *
 * @param [in,out]	handle		handle
 * @param [in]		markerno	Marker number (1-10) or AOV_MARKER_ALL
 *****************************************************************************/
EXPORT int AVCOM_SA_ClearMarker ( void *handle, int markerno );


/*************************************************************************//**
 * Opens the socket to the specified Spectrum Analyzer
 * 
 * @return Status of function call
 *
 * @param [in,out]	handle	Pointer to analyzer handle
 *
 * @pre	handle must be Registered before executing function
 *****************************************************************************/
EXPORT int AVCOM_SA_Connect(void *handle);


/*************************************************************************//**
 * De-Initialize the Avcom API environment
 *
 * This function should be called to cleanup the API environment before exiting a program
 * Any open connections will also be terminated and any registered handles will be terminated
 *
 * @note	This function must be called before calling any other functions
 * @note	You should call this function only once
 *
 * @return Status of function call
 *
 *****************************************************************************/
EXPORT int AVCOM_SA_Deinitialize(void);


/*************************************************************************//**
 * Closes the specified socket
 * 
 * @return Status of function call
 *
 * @param [in,out]	handle	Pointer to analyzer handle
 *****************************************************************************/
EXPORT int AVCOM_SA_Disconnect(void *handle);


/*************************************************************************//**
 * Acquires the carrier noise ratio at a given frequency 
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		markerno	Marker to calculate CNR on
 * @param [out]		cnr		CNR at requested frequency dBm/Hz
 *****************************************************************************/
EXPORT int AVCOM_SA_GetCNR ( void *handle, int markerno, double *cnr );


/*************************************************************************//**
 * Lookups a handle for a registered Spectrum Analyzer
 *
 * For cases when the handle has been 'lost' this function will locate the 
 * handle in the dynamic link and return the location to the desired handle.
 *
 * @warning	Returns (-1) if analyzer is not registered or is not found.
 *
 * @return Location of handle. 
 *
 * @param [in]	ip		IP Address of Spectrum Analyzer
 * @param [in]	port	Port Number of Spectrum Analyzer
 *****************************************************************************/
EXPORT void *AVCOM_SA_GetHandle(char *ip, int port);


/*************************************************************************//**
 * Reports the frequency and amplitude information of a selected marker
 * 
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		markerno	Marker number (1-10)
 * @param [out]		freq		Frequency of Marker
 * @param [out]		db			Amplitude of Marker (dBm)
 *****************************************************************************/
EXPORT int AVCOM_SA_GetMarker ( void *handle, int markerno, double *freq, double *db);


/*************************************************************************//**
 * Gets Signal-to-Noise Ratio between two markers 
 *
 * SNR is specified as (dBmax - dBmin)
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		marker1	Marker number (1-10)
 * @param [in]		marker2	Marker number (1-10)
 * @param [out]		db		Amplitude of Marker (dBm)
 *****************************************************************************/
EXPORT int AVCOM_SA_GetSNR ( void *handle, int marker1, int marker2, double *db);


/*************************************************************************//**
 * Retrieves a single trace from Spectrum Analyzer. Data is passed through a
 * defined pointer to an AOVwaveForm structure.
 * 
 * @return Status of function call
 *
 * @param [in,out]	handle	Pointer to analyzer handle
 * @param [out]		trace	Pointer to structure with 2-D amplitude/frequency information
 *****************************************************************************/
EXPORT int AVCOM_SA_GetSpectrumData(void *handle, sAOVwaveForm *trace);


/*************************************************************************//**
 * Initialized the Avcom API environment.
 *
 * This function must be called before any other, including AVCOM_SA_Register()
 *
 * @return Status of function call
 *****************************************************************************/
EXPORT int AVCOM_SA_Initialize(void);


/*************************************************************************//**
 * Shortcut for constructing the structure used to set LNB
 *
 * @return Status of function call
 *
 * @param [out]		lnb		Pointer to LNB Power setting Struct
 * @param [in]		LNB_EN	LNB_ENABLED / LNB_DISABLED
 * @param [in]		LNB_VOLT	LNB_18V / LNB_13V
 * @param [in]		LNB_22	LNB_22KHZ_ON / LNB_22KHZ_OFF
 *****************************************************************************/
EXPORT int AVCOM_SA_LNBstruct(sAOVlnb *lnb, int LNB_EN, int LNB_VOLT, int LNB_22 );


/*************************************************************************//**
 * Registers a Spectrum Analyzer to a handle and registers it to the 
 * dynamically linked list
 *
 * Modes are:
 *		- AOV_DEFAULT		The default and currently only mode (0)
 *
 * Special flags are:
 *		- AOV_DEFAULT		Connection will be closed after registering handle
 *		- AOV_ALSO_CONNECT	Connection will be open after registering handle
 * 
 * @return Status of function call
 *
 * @param [in,out]	handle	handle - pass unary operator, ex: &handle
 * @param [in]		ip		IP address of analyzer
 * @param [in]		port	Port number of analyzer
 * @param [in]		mode	Sets autonomy state
 * @param [in]		flags	Special Flags for function
 *
 * @pre	Environment must first be initialized by AVCOM_SA_Initialize()
 *****************************************************************************/
EXPORT int AVCOM_SA_Register(void **handle, char *ip, int port, int mode, int flags);


/*************************************************************************//**
 * Set all waveform settings via a single function
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		cfreq	Center Frequency (MHz)
 * @param [in]		span	Span (MHz)
 * @param [in]		reflvl	Reference Level (dB)
 * @param [in]		rbw		Resolution Bandwidth (see enum)
 * @param [in]		input	Input number
 * @param [in]		lnb		LNB Power Setting - pass a "NULL" if to be disabled.
 *****************************************************************************/
EXPORT int AVCOM_SA_SetAllSettings(void *handle, double cfreq, double span, int reflvl, eAOV_RBW rbw, int input, sAOVlnb *lnb);


/*************************************************************************//**
 * Sets number of traces to average
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		avg		Number of traces to average (0-250)
 *****************************************************************************/
EXPORT int AVCOM_SA_SetAverage ( void *handle, int avg);


/*************************************************************************//**
 * Set Center Frequency
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		cfreq	Center Frequency (MHz)
 *****************************************************************************/
EXPORT int AVCOM_SA_SetCenterFrequency ( void *handle, double cfreq );


/*************************************************************************//**
 * Set Input
 *
 * Changing the input can do one of two things:
 *
 * AOV_DEFAULT: The analyzer is capable of remember the previous
 *		settings for each input. This flag will load the inputs' previous 
 *		settings when the input is changed.
 *
 * AOV_SWITCH_INPUT_ONLY: The analyzer will change only the input, and will use
 *		the same center frequency, span, RBW and reference level that the 
 *		current input is set to.
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		input	Input number
 * @param [in]		flags	AOV_DEFAULT or AOV_SWITCH_INPUT_ONLY
 *
 * @note	If AOV_SWITCH_INPUT_ONLY is used, make sure Reference Level
			and LNB settings are vaild for the new input.
 *****************************************************************************/
EXPORT int AVCOM_SA_SetInput ( void *handle, int input, int flags ); 


/*************************************************************************//**
 * Set LNB Power
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		lnb		LNB Power Setting - pass a "NULL" if to be disabled.
 *
 * @note	The lnb parameter may be set by using AVCOM_SA_LNBstruct()
 *****************************************************************************/
EXPORT int AVCOM_SA_SetLNB ( void *handle, sAOVlnb *lnb );


/*************************************************************************//**
 * Sets a marker to a specified frequency
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		markerno	Marker number (1-10)
 * @param [in]		freq		Frequency (MHz)
 *****************************************************************************/
EXPORT int AVCOM_SA_SetMarkerFreq ( void *handle, int markerno, double freq );


/*************************************************************************//**
 * Sets a marker at a dynamic location
 * 
 * Flag values
 * - AOV_MARKER_MIN: Sets specified marker at minimum frequency
 * - AOV_MARKER_MAX: Sets specified marker at maximum frequency
 *
 * @return Status of function call
 *
 * @param [in,out]	handle		handle
 * @param [in]		markerno	Marker number (1-10)
 * @param [in]		flags		AOV_MARKER_MIN / AOV_MARKER_MAX
 *****************************************************************************/
EXPORT int AVCOM_SA_SetMarkerOpt ( void *handle, int markerno, int flags );


/*************************************************************************//**
 * Set Reference Level
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		reflvl	Reference Level (dB)
 *****************************************************************************/
EXPORT int AVCOM_SA_SetRefLevel ( void *handle, int reflvl );


/*************************************************************************//**
 * Set Resolution Bandwidth
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		rbw		Resolution Bandwidth (see AOV_RBW enum)
 *****************************************************************************/
EXPORT int AVCOM_SA_SetResBandwidth ( void *handle, eAOV_RBW rbw );


/*************************************************************************//**
 * Set Span
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		span		Span (MHz)
 *****************************************************************************/
EXPORT int AVCOM_SA_SetSpan ( void *handle, double span );


/*************************************************************************//**
 * Set the Sweep Mode
 *
 * Mode Values
 * - AOV_SWEEP_OFF
 * - AOV_SWEEP_AUTO
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		mode	Sweep Mode
 *
 * @warning The Avcom GUI will only work properly when in AOV_SWEEP_AUTO
 * @warning Set AOV_SWEEP_AUTO is recommended before exiting for Avcom GUI
 *		compliance.
 *****************************************************************************/
EXPORT int AVCOM_SA_SetSweepMode ( void *handle, int mode );


/*************************************************************************//**
 * Acquires a trace with the settings programmed into the Spectrum Analyzer 
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 * @param [in]		flags	Set to NULL
 *  
 * @note Command is only effective if auto-sweep is enabled.
 *****************************************************************************/
EXPORT int AVCOM_SA_Trigger ( void *handle, int flags );


/*************************************************************************//**
 * Unregistered's a handle and removes it from the dynamically linked list
 *
 * This will also disconnect analyzer if connection is open
 *
 * @return Status of function call
 *
 * @param [in,out]	handle	handle
 *****************************************************************************/
EXPORT int AVCOM_SA_Unregister(void *handle);

/*************************************************************************//**
 * Determines the number of Avcom Analyzers are connected via USB
 *
 * It is advisable to call this function before retrieving USB Device lists
 * This function is used to obtain necessary info prior to registering 
 * and analyzer on the USB port
 *
 * @return Status of function call
 *
 * @param [in,out]	numDevs	Number of devices attached
 *****************************************************************************/
//EXPORT int AVCOM_SA_GetNumUSB ( unsigned int *numDevs );


/****************            ERROR CODES           ***************************/
#define AOV_ERR_NO_ERROR			0		//!< No Error 
#define AOV_ERR_WSASTARTUP_FAILED	100000	//!< Windows Only: Winsock2 WSAStartup() function failed 
#define AOV_ERR_SOCKET_FAILED		100001	//!< Unable to open socket. Function socket() failed.
#define AOV_ERR_SETSOCKETOPT_FAILED	100002	//!< Unable to set socket options. Function setsockopt() failed.
#define AOV_ERR_CONNECT_FAILED		100003	//!< Unable to connect to socket. Function connect() failed.
#define AOV_ERR_CLOSESOCKET_FAILED	100004	//!< Windows Only: Unable to close socket. Function closesocket() failed.
#define AOV_ERR_WSACLEANUP_FAILED	100005	//!< Windows Only: Winsock2 WSACleanup() function failed.
#define AOV_ERR_CLOSE_FAILED		100006	//!< Linux Only: Unable to close socket. Function close() failed.
#define AOV_ERR_SEND_FAILED			100007	//!< Unable to send data. Function send() failed.
#define AOV_ERR_RECV_FAILED			100008	//!< Unable to receive data. Function recv() failed.  Often a timeout has occurred waiting for data.
#define AOV_ERR_MALLOC_FAILED		100009	//!< Unable to malloc() or realloc() data.
#define AOV_ERR_SOCKET_NOT_OPEN		100010	//!< Specified socket does not appear to be open.
#define AOV_ERR_SOCKET_ALREADY_OPEN	100011	//!< Specified socket appears to already be open.
#define AOV_ERR_GET_ADDR_INFO_FAIL	100012	//!< Function getaddrinfo() failed. Typically an invalid ip address/host name or port is specified.
#define AOV_ERR_NOT_INITIALIZED		100013	//!< The Avcom API environment does not appear to be initialized yet. Try running AVCOM_SA_Initialize() first. 
#define AOV_ERR_UNEXPECTED_CMD		100014	//!< An unexpected command was received from the Spectrum Analyzer.

#define AOV_ERR_SET_CFREQ_FAILED	200000	//!< Unable to set or verify that center frequency was set to desired frequency.
#define AOV_ERR_SET_SPAN_FAILED		200001	//!< Unable to set or verify that the Span was set correctly.
#define AOV_ERR_SET_RFLVL_FAILED	200002	//!< Unable to set or verify that the reference level Input was set correctly.
#define AOV_ERR_SET_RBW_FAILED		200003	//!< Unable to set or verify that the Resolution Bandwidth was set correctly.
#define AOV_ERR_SET_RF_FAILED		200004	//!< Unable to set or verify that the RF Input was set correctly.
#define AOV_ERR_SET_LNB_FAILED		200005	//!< Unable to set or verify the LNB Power setting was set correctly.
#define AOV_NOTE_SPAN_AUTOSET		200006	//!< The span has not been set to the expected value but appears to have been adjust to the max allowable given the center frequency.

#define AOV_ERR_INVALID_CFREQ		300000	//!< Entered Center Frequency is not valid
#define AOV_ERR_INVALID_SPAN		300001	//!< Entered Span is not valid
#define AOV_ERR_INVALID_RFLVL		300002	//!< Entered Reference Level is not valid
#define AOV_ERR_INVALID_RBW			300003	//!< Entered Resolution Bandwidth is not valid
#define AOV_ERR_INVALID_RF			300004	//!< Entered RF Input is not valid
#define AOV_ERR_INVALID_LNB			300005	//!< Entered LNB Power Setting is not valid
#define AOV_ERR_INVALID_MARKER		300006	//!< Entered Marker number is not valid
#define AOV_ERR_INVALID_FLAG		300007	//!< Entered Flag is not valid
#define AOV_ERR_INVALID_AVGERAGE	300008	//!< Entered Averaging value is not valid
#define AOV_ERR_INVALID_SWEEP_MODE	300009	//!< Entered Sweep Mode is not valid
#define AOV_ERR_CLEAR_MARKER_FAILED	300010	//!< Unable to clear all markers
#define AOV_ERR_SET_MARKER_FAILED	300011	//!< Unable to set Marker
#define AOV_ERR_SET_AVG_FAILED		300012	//!< Unable to set Averaging
#define AOV_ERR_SET_SWEEP_FAILED	300013	//!< Unable to set sweep mode
#define AOV_ERR_FREQ_OOR			300014	//!< Specified Frequency out of range

#define AOV_HANDLE_NOT_FOUND		400000	//!< The given handle does not appear to be valid.
#define AOV_HANDLE_ALREADY_EXISTS	400001	//!< Unable to create new handle as handle appears to already exist.
#define AOV_ALREADY_INITIALIZED		400002	//!< The Avcom API environment appears to already be initialized and cannot be initialized again.
#define AOV_ALREADY_NOT_INITIALIZED 400003	//!< The Avcom API environment appears to not be initialized and therefore cannot be de-initialized.

#define AOV_ERR_USB_COMM			500000	//!< Error communicating to the USB Driver


#define ANA_SET_CF_ERROR			1000	//!< Analyzer reports that the Center Frequency was not set correctly.
#define ANA_SET_SP_ERROR			1001	//!< Analyzer reports that the Span was not set correctly.
#define ANA_SET_RL_ERROR			1002	//!< Analyzer reports that the Reference Level was not set correctly.
#define ANA_SET_RBW_ERROR			1003	//!< Analyzer reports that the Resolution Bandwidth was not set correctly.
#define ANA_SET_RF_ERROR			1004	//!< Analyzer reports that the RF Input was not set correctly.
#define ANA_SET_LNB_ERROR			1005	//!< Analyzer reports that the LNB Power setting was not set correctly.
#define ANA_INVALID_CF				1015	//!< Analyzer reports that the desired Center Frequency is invalid.
#define ANA_INVALID_SP				1016	//!< Analyzer reports that the desired Span is invalid.
#define ANA_INVALID_RL				1017	//!< Analyzer reports that the desired Reference Level is invalid.
#define ANA_INVALID_RF				1018	//!< Analyzer reports that the desired RF Input is invalid.
#define ANA_INVALID_RBW				1019	//!< Analyzer reports that the desired Resolution Bandwidth is invalid.
#define ANA_MARKERS_OOR				1020	//!< Analyzer reports that the Marker is Out Of Range.
#define ANA_MARKERS_ACT				1021	//!< Analyzer reports that the Marker is not active.
#define ANA_MARKERS_NUM				1022	//!< Analyzer reports that the Marker number is invalid.


/****************            CONSTANTS             ***************************/
#define AOV_DEFAULT					0
#define AOV_ALSO_CONNECT			1				// Option for AVCOM_SA_Register()

#define	AOV_SWITCH_INPUT_ONLY		1				// Option for AVCOM_SA_SetInput()


#endif /* AOVSPECAN_H */

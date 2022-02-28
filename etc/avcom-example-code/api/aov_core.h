#pragma once
#ifndef AOV_CORE_H
#define AOV_CORE_H

#define AOVAPI_MAJOR  1
#define AOVAPI_MINOR  0
#define AOVAPI_PATCH  0



#ifndef TRUE
#define TRUE	1;
#endif
#ifndef FALSE
#define FALSE	0;
#endif

struct AOV_RBW_Avail_ {
	unsigned char mask;
	eAOV_bool rbw_3MHz;
	eAOV_bool rbw_1MHz;
	eAOV_bool rbw_300KHz;
	eAOV_bool rbw_200KHz;
	eAOV_bool rbw_100KHz;
	eAOV_bool rbw_10KHz;
};

struct AOV_time_ {
	int seconds;
	int minutes;
	int hours;
	int day;
	int month;
	int year;
};

struct AOV_dbrange_ {
	char		min;
	char		max;
};

struct AOV_inputs_ {
	eAOV_bool			rf_avail;
	eAOV_bool			db70_avail;
	eAOV_bool			lnb_avail;
	struct AOV_dbrange_	dbrange;
};

struct AOV_sbshw_ {
	unsigned char			prodID;
	unsigned char			projID;
	unsigned short			sbs_fw;
	unsigned char			PCBfab;
	double					freq_min;
	double					freq_max;
};

struct AOV_lcdhw_ {
	unsigned short			projID;		// LCD Project ID (Not in use)
	unsigned short			lcd_fw;
	unsigned char			mbfab;		// Motherboard PCB FAB
	unsigned char			fpfab;		// Front Panel PCB FAB
};

struct AOV_cur_set_ {
	int						cf;
	int						sp;
	int						ref_level;
	enum eAOV_RBW_			rbw;
	int						input;
	unsigned char			lnb;
};

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
	struct AOV_RBW_Avail_	avail_rbw;
	struct AOV_inputs_		input[6];
	struct AOV_time_		cal_date;
	struct AOV_sbshw_		sbshw;
	struct AOV_lcdhw_		lcdhw;
	struct AOV_cur_set_		cur_settings;

} AOVhandle;

#define		SBS_2150 			0x3A	//!< RSA-2150B-SBS Product ID
#define		SBS_1100 			0x4A	//!< RSA-1100B-SBS Product ID
#define		SBS_2500 			0x5A	//!< RSA-2500B-SBS Product ID
#define		SBS_2300 			0x6A	//!< RSA-2300B-SBS Product ID


#define 	SBS_1100_LOWER_LIMIT	1
#define 	SBS_1100_UPPER_LIMIT	1100
#define 	SBS_2150_LOWER_LIMIT	950
#define 	SBS_2150_UPPER_LIMIT	2150
#define 	SBS_2300_LOWER_LIMIT	5
#define 	SBS_2300_UPPER_LIMIT	2300
#define 	SBS_2500_LOWER_LIMIT	5
#define 	SBS_2500_UPPER_LIMIT	2500


/* Protocol Defines */
#define STX							0x02
#define ETX							0x03


#define	NO_PACKET					0x00
#define	CHANGE_INDIVIDUAL			0x01
#define	WAVE_REQ					0x03
#define	WAVE_CHANGE					0x04
#define	HW_DESC						0x07
#define	WAVE_8BIT					0x09
#define	WAVE_12BIT					0x0F
#define	LNB_DESC					0x0D
#define	DIALOUGE_CMD				0x20
#define	PACKET_ACK					0x21
#define	TRANS_STX_CMD				0x22
#define	BLOCK_PACKET				0x23
#define	REFLASH_CMD					0x26
#define	BLKFIN_CMDS					0x27
#define	API_CMDS					0x28
#define	SBS_UNLOCK					0x36
#define	SBS_CAL_CMD					0x30
#define	SBS_LOCK_CMD				0x31
#define SBS_BOOTLOADER				0x3A
#define SBS_BOOT_MODE				0x47
#define	GUI_MESSAGE					0x60

#define BOOTLOAD_KEY				0x0E
#define BOOTLOAD_DESC				0x10


#define	SINGLE_8BIT					0x03	//!< Coommand to request a single waveform
#define	SINGLE_12BIT				0x05	//!< Coommand to request a single waveform


#define _RBW_3MHZ					0x80
#define _RBW_1MHZ					0x40
#define _RBW_300KHZ					0x20
#define _RBW_100KHZ					0x10
#define _RBW_10KHZ					0x08
#define _RBW_3KHZ					0x04
#define _RBW_200KHZ					0x02



/* API Sub Commands */

#define API_CHANGE_CF				0x04
#define API_CHANGE_SP				0x05
#define API_CHANGE_RL				0x06
#define API_CHANGE_RBW				0x07
#define API_CHANGE_INPUT			0x08
#define API_CHANGE_LNB				0x09
#define API_SET_MARKER				0x16
#define API_CLEAR_MARKER			0x17
#define API_SET_MARKER_OPT			0x18
#define API_GET_MARKER				0x19
#define API_GET_SNR					0x1A
#define API_AVERAGE_SET				0x1B
#define API_SWEEP_MODE				0x1C
#define API_TRIGGER					0x1D
#define API_GET_CNR					0x1E

/* BLKFIN Sub commands */
#define 	RETRIVE_FP_VERSION		0x10
#define		ENTER_PASSTHROUGH		0x11
#define		EXIT_PASSTHROUGH		0x12
#define		SECTOR_LOCK				0x13
#define		SECTOR_UNLOCK			0x14
#define		WRITE_PROTECT_REGISTER	0x15
#define		READ_PROTECT_REGISTER	0x16
#define		DATAFLASH_READ			0x17
#define		DATAFLASH_WRITE			0x18
#define		GET_SAVED_NAMES			0x19
#define		RENAME_SAVED			0x1A
#define		EXPORT_SAVED			0x1B
#define		IMPORT_SAVED			0x1C
#define		GET_PRESET_NAMES		0x1D
#define		RENAME_PRESETS			0x1E
#define		EXPORT_PRESET			0x1F
#define		IMPORT_PRESET			0x20
#define		LOAD_PRESET				0x21
#define		READ_FP_HW_DESC			0x22
#define		WRITE_FP_HW_DESC		0x23		// NOTE: Sector A must be unlocked
#define		ERASE_PRESET			0x24
#define		ERASE_WAVEFORM			0x25
#define		ERASE_ALL_DATA			0x28
#define		SET_DATE_TIME			0x29


EXPORT int AOV_sendData(void *handle, unsigned char *data, int data_size);
EXPORT int AOV_recData(void *handle, unsigned char *data, int *data_size);

#endif /* AOV_CORE_H*/
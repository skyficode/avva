const { Parser } = require('binary-parser')

//          ********** IMPORTANT **********
//          USE THIS CODE AT YOUR OWN RISK!
//          Bugs and errors are possible here...
//          Sensative RF electronics may re-act poorly causing serious damange to equipment or perhaps even you.
//
// NOTE:    Developed using an SBS with v2.X, customized for SWE-DISH/Rockwell/DataPath.
//          As such, other units may not work...or, worse, invalid commands sequences breaks things!
// DEV:     UPPERCASE vars are used to match style of AVCOM's Protocol Documention

// NETWORK CONFIG
const NET_DEFAULT_IP = '192.168.138.242' // is SWE-DISH default, but documented default is '192.168.118.242'
const NET_DEFAULT_PORT = 26482

const NET_IP = process.env.AVCOM_IP || NET_DEFAULT_IP
const NET_PORT = process.env.AVCOM_IP || NET_DEFAULT_PORT

// FIELD TYPES
const HEX_DTYPE_SBS_HW = '07'
const HEX_DTYPE_LNB_POWER = '0D'
const HEX_DTYPE_WDR_REQ = '03'
const HEX_DTYPE_WDR = '09'
const HEX_DTYPE_CHANGE_REQ = '04'
const HEX_DTYPE_BAD_PKT = '08'
const HEX_DTYPE_MESG = '60'
const FIELD_PADD = '00'
const HEX_STX = '02'
const HEX_ETX = '03'

// HELPERS
const FAIL_TEST = function (testname) {
	const msg = `TEST_FAILED: ${testname}`
	console.error(msg)
	throw new Error(msg)
	return false
}

const HEX_FROM_NUM = function (num, pad) {
	return num.toString(16).padStart(pad, '0')
}

function HEX_FROM_NUM_TEST() {
	const test1 = HEX_FROM_NUM(3, 2) == '03' ? true : FAIL_TEST('HEX_FROM_NUM(1)')
	const test2 = HEX_FROM_NUM(3, 4) == '0003' ? true : FAIL_TEST('HEX_FROM_NUM(1,4)')
}
HEX_FROM_NUM_TEST()

const HEX_LEN = function (num) {
	return HEX_FROM_NUM(num, 4)
}

function HEX_LEN_TEST() {
	HEX_LEN(1) == '0001' ? true : FAIL_TEST('HEX_LEN(1)')
	HEX_LEN(4369) == '1111' ? true : FAIL_TEST('HEX_LEN(4369,4)')
}
HEX_LEN_TEST()

// HUMANIZING HELPERS
const HZ_FROM_STRING = function (hzstring, units) {
	const re = /(?<fq>[-]?\d+(\.\d+)*)\s*(?<GMK>[MmKkGg])?/
	const match = `${hzstring}`.toUpperCase().match(re)
	if (!match || !match.groups.fq) {
		console.error(
			'HZ_STRING is not parsable. Any number followed by a G/M/K should work.',
			hzstring,
			match
		)
		throw new Error('HZ_STRING is not parsable')
	}
	const fq = Number(match.groups.fq)
	if (!fq) {
		console.error(
			'HZ_STRING is missing a frequency. Any number followed by a G/M/K should work.',
			hzstring,
			match
		)
		throw new Error('HZ_STRING is missing a frequency')
	}
	if (!units) {
		units = ''
	}
	let GMK = ''
	if (match.groups.GMK) {
		GMK = match.groups.GMK.toUpperCase()
	}
	let hz = null
	switch (GMK) {
		case 'G':
			hz = fq * 1e9
			break
		case 'M':
			hz = fq * 1e6
			break
		case 'K':
			hz = fq * 1e3
			break
		default:
			hz = fq
	}
	let scaled = null
	switch (units.toUpperCase()) {
		case 'G':
			scaled = hz / 1e9
			break
		case 'M':
			scaled = hz / 1e6
			break
		case 'K':
			scaled = hz / 1e3
			break
		case '':
			scaled = hz
			break
		default:
			console.error('HZ_STRING is not formatable', hzstring, units)
			throw new Error('HZ_STRING is not formatable. Invalid units?')
	}
	scaled = `${scaled} ${units}Hz`
	console.log('HZ_FROM_STRING units:', scaled, '<= (', hzstring, units, ' )', fq, '~', hz, 'hz')
	return scaled
}

function HZ_FROM_STRING_TEST() {
	const results = []

	results.push(HZ_FROM_STRING('11.111 G h z') === '11111000000 Hz')
	results.push(HZ_FROM_STRING('3000000', 'M') === '3 MHz')
	results.push(HZ_FROM_STRING('10Mhz', 'M') === '10 MHz')
	results.push(HZ_FROM_STRING('3000000') === '3000000 Hz')
	results.push(HZ_FROM_STRING('3000K', 'M') === '3 MHz')
	results.push(HZ_FROM_STRING('-1Mhz', 'M') === '-1 MHz')

	results.push(HZ_FROM_STRING('-1Mhz', 'K') === '-1000 Khz')
	results.push(HZ_FROM_STRING('-1Mhz', 'G') === '-1000 Khz')
	results.push(HZ_FROM_STRING('-50 Mhz', 'M') === '-50 Mhz')
	results.push(HZ_FROM_STRING('-0.070 Ghz', 'G') === '-0.07 Ghz')

	results.push(HZ_FROM_STRING('-9000Mhz', 'G') === '-9 GHz')

	results.push(HZ_FROM_STRING('-2222 khz', 'M') === '-2.222 Mhz')

	console.log(results)
}
HZ_FROM_STRING_TEST()

// WAVEFORM DATA POINTS
/* Waveform data may be available in 8‐bit or 12‐bit resolutions. In both resolutions, there are 320 data points in each packet.
For 8‐bit resolution, each data point is one byte. Refer to eq(3) for calculating the amplitude in dB of the 8‐bit waveform data. In eq(3), dB is the amplitude, ‘x’ is the value in the 8‐bit waveform packet, and CurRL is the current reference level value in the waveform packet. */

// EQUATION eq(1) - for SBS_FM v1.X and v2.X
const dB8bitV2 = function (wdp, crl) {
	return 0.2 * wdp - (crl + 40)
}

// EQUATION eq(2) - for SBS_FM v3.X
const dB8bitV3 = function (wdp, crl) {
	return 0.2 * wdp + (crl - 40)
}

// TODO: 10-bit support

// FREQUENCY CALCULATIONS
// - Center frequency (CF) and Span (SP) are represented in the CSW protocol as a four‐byte or unsigned 32‐bit value. To convert a frequency in megahertz’s to the CSW protocol format (such as CF and SP), use equation eq(1) below where F is the frequency in the CSW protocol format. This is most frequently used in packet requests.

// EQUATION eq(1):   F = Freqency(MHz) * 10,000
const MHzCSW = function (mhz) {
	return fq * 10000
}

// EQUATION eq(2):   Freqency(MHz) = F / 10,000
const CSWMHz = function (F) {
	return F / 10000
}

// TABLE 6 - RBW Mask
const HEX_RBW = function (rbwstring) {
	switch (rbwstring.toLowerCase()) {
		case '3mhz':
			break
	}
}
/*
.bit8('RBW_3MHz')
.bit7('RBW_1MHz')
.bit6('RBW_300KHz') 
.bit5('RBW_100KHz')
.bit4('RBW_10KHz')
.bit3('RBW_3KHz')
.bit2('RBW_200KHz') 
.bit1('RBW_RESERVED')
*/

// TABLE 5 - RL

/* 
    SBS_FM < v3.0 - range -10 to -50, up -70 with addon
    SBS_FM > v3.0 - range +10 to -50, up -70 with addon
+10dB = n/a
‐10dB = 0x0A
‐30dB = 0x1E
‐50dB = 0x32
‐70dB = 0x46


*/

// TABLE 7 - LNB Power
/*
.bit3('LNB_EN')
.bit4('LNB_VSEL')
.bit6('LNB_22KHz')
.bit4('LNB_AVAIL')
*/

// TABLE 14 -  PCB‐FAB Byte
// TODO from doc, more complex, less useful?

// TABLE 15 - Options Bitmask
/*
.bit3('60dB Range')
.bit2('RESERVED') 
.bit1('Splitter')
*/

// TABLE 16 - Table 16: AvailLNB Bitmask
//      for inputs: 0=disabled, 1=enabled
/*
// ≤ v1.8 - AvailLNB Bitmask NOT USED
// ≥v1.9,≤v2.5 - only if 'AvailLNB' is supported

// ≥ v2.6 0
.bit7('AvailLNB_Input6') 
.bit6('AvailLNB_Input5')
.bit5('AvailLNB_Input4')
.bit4('AvailLNB_Input3')
.bit3('AvailLNB_Input2') 
.bit2('AvailLNB_Input1') 
.bit1('AvailLNB')
 */

// TABLE 17 - Avail70 Bitmask
//      for inputs: 0=disabled, 1=enabled
/*
.bit8('Avail70_Bit7') // ≥ v2.14 = 1, ≤ v2.13 = RESERVED
.bit6('AvailLNB_Input6') 
.bit5('AvailLNB_Input5')
.bit4('AvailLNB_Input4')
.bit3('AvailLNB_Input3')
.bit2('AvailLNB_Input2') 
.bit1('AvailLNB_Input1') 
*/

// TABLE 20 - RF LO Offset Mask Byte
//      for inputs: 0=LNB Power Mask Byte determines offset, 1=always use low band offset
/*
.bit8('LNB_LOM_BIT8', {assert: true})
.bit7('LNB_LOM_FUTURE')
.bit6('LNB_LOM_Input6') 
.bit5('LNB_LOM_Input5')
.bit4('LNB_LOM_Input4')
.bit3('LNB_LOM_Input3')
.bit2('LNB_LOM_Input2') 
.bit1('LNB_LOM_Input1') 
*/

// TABLE 21 - LNB Power Mask Byte
//      for inputs: 0=use 22KHz selection to select band, 1=use voltage to select band
/*
.bit8('LNB_PM_BIT8', {assert: true})
.bit7('LNB_PM_FUTURE')
.bit6('LNB_PM_Input6') 
.bit5('LNB_PM_Input5')
.bit4('LNB_PM_Input4')
.bit3('LNB_PM_Input3')
.bit2('LNB_PM_Input2') 
.bit1('LNB_PM_Input1') 
*/

// TABLE 10 - SBS_HW Description Request
const REQ_SBS_HW_SPEC = new Parser()
	.endianess('big')
	.uint8('STX', {
		assert: 0x02,
	})
	.uint16('LEN', {
		assert: 0x03,
	})
	.uint8('DTYPE', {
		assert: 0x07,
	})
	.uint8('PADD', {
		assert: 0x00,
	}) // Q: in doc, but is needed?
	.uint8('ETX', {
		assert: 0x03,
	})

const REQ_SBS_HW = function () {
	return Buffer.from('020003070003', 'hex')
}

function REQ_SBS_HW_TEST() {
	REQ_SBS_HW_SPEC.parse(REQ_SBS_HW())
}
REQ_SBS_HW_TEST()

// TABLE 11 - SBS_HW Description Response
const SBS_HW = Parser.start()
	.endianess('big', {
		desc: 'blah',
		docref: 'TABLE 12',
	})
	.uint8('ProdID') // see TABLE 12
	.uint8('SBS_FM_MAJOR') // SBS_FM version
	.uint8('SBS_FM_MINOR') // is broken into two fields
	.uint8('STRM_M') // Single, Stream, OFF
	.uint32('CurCF') // Current Center Frequency
	.uint32('CurSP') // Current Span Frequency
	.uint8('CurRL') // see TABLE 5, Current Reference Level
	.uint8('CurRBW') // from TABLE 6, Current Resolution Bandwidth (RBW)
	.uint8('AvailRBW') // from TABLE 6, Available Resolution Bandwidth (RBW)
	.uint8('CurRF') // Current RF Input (10d – 15d for inputs 1 through 6)
	.uint8('AvailRF') // Available RF Input (10d – 15d for inputs 1 through 6)
	.uint8('CurGain') // Engineering Use Only
	.uint8('OPT_BMK') // see TABLE 15, Options Bitmask
	.uint8('CurCOM') // Engineering Use Only
	.uint8('AvailCOM') // Engineering Use Only
	.int16('Cur_IEF') // Current Internal Extender Frequency Offset (MHz)
	.int16('Cur_EEF') // Current External Extender Frequency Offset (MHz)
	.string('SN', {
		length: 16,
	}) // Serial Number, ASCII
	.uint8('PCBFAB') // Analyzer PCB Fab (upper nibble) + Fab Rev (lower nibble)
	.uint8('Cal_Day') // Day of calibration – 10d is added to byte
	.uint8('Cal_Mon') // Month of calibration – 10d is added to byte
	.uint8('Cal_Year_MSB') // calibration year – example: 0x1409 = 0x14*100d + 0x09 = 2009d, MSB*100+LSB
	.uint8('Cal_Year_LSB') // calibration year – example: 0x1409 = 0x14*100d + 0x09 = 2009d, MSB*100+LSB
	.uint8('TEMP') // Board Temperature (nom 0x80 = 0°C)
	.uint8('TEMPmin') // Minimum Board Temperature (nom 0x80 = 0°C)
	.uint8('TEMPmax') // Maximum Board Temperature (nom 0x80 = 0°C)
	.uint8('AmpCal') // Engineering Only
	.uint8('AvailLNB') // see TABLE 16, Information regarding LNB Power options
	.uint8('CurLNB') // see TABLE 7, for older version, >= v1.9 is RESERVED
	.uint8('FirstRUN') // ENG use, 1stRUN is document but invalid name in node/JS
	.uint8('IsLOCK') // ENG use
	.uint8('ProjID') // TABLE 13, Project Information
	.uint8('Avail70')
	.array('FUTURE_HW', {
		type: 'uint8',
		length: 27,
	})

// TABLE 12 - ProdID Byte
const ProdName = function (ProdId) {
	switch (ProdId) {
		case 0x3a:
			return '2150'
		case 0x4a:
			return '1100'
		case 0x5a:
			return '2500 or 5000'
		default:
			return 'UNKNOWN'
	}
}

// TABLE 13 - ProjID bitmark
/*
SNG '010xxxxx'
PSA '100xxxxx'
RSA with Advanced API '001xxxxx'
Default '00000000'
RCTV‐5000 '00000001'
RSA‐2300 'xxx00011'
*/

// TABLE 18 - LNB Power Description Request
const REQ_LNB_POWER_SPEC = Parser.start()
	.endianess('big')
	.uint8('STX', {
		assert: 0x02,
	})
	.uint16('LEN', {
		assert: 0x02,
	})
	.uint8('DTYPE', {
		assert: 0x0d,
	})
	.uint8('ETX', {
		assert: 0x03,
	})

const REQ_LNB_POWER = function () {
	return Buffer.from('0200020D03', 'hex')
}

function REQ_LNB_POWER_TEST() {
	REQ_LNB_POWER_SPEC.parse(REQ_LNB_POWER())
}
REQ_LNB_POWER_TEST()

// TABLE 19 - LNB Power Description Response
const LNB_POWER = Parser.start()
	// .uint8('LNB_LOM') is made up of bits // see TABLE 20
	.bit1('LNB_LOM') // 'LNB_LOM'
	.bit1('LNB_LOM_FUTURE')
	.bit1('LNB_LOM_Input6')
	.bit1('LNB_LOM_Input5')
	.bit1('LNB_LOM_Input4')
	.bit1('LNB_LOM_Input3')
	.bit1('LNB_LOM_Input2')
	.bit1('LNB_LOM_Input1')
	// .uint8('LNB_PM') is made up of bits // see TABLE 21
	.bit1('LNB_PM')
	.bit1('LNB_PM_FUTURE')
	.bit1('LNB_PM_Input6')
	.bit1('LNB_PM_Input5')
	.bit1('LNB_PM_Input4')
	.bit1('LNB_PM_Input3')
	.bit1('LNB_PM_Input2')
	.bit1('LNB_PM_Input1')
	.uint8('LNB_RFP')
	.int16('RF1OFF1')
	.int16('RF1OFF2')
	.int16('RF2OFF1')
	.int16('RF2OFF2')
	.int16('RF3OFF1')
	.int16('RF3OFF2')
	.int16('RF4OFF1')
	.int16('RF4OFF2')
	.int16('RF5OFF1')
	.int16('RF5OFF2')
	.int16('RF6OFF1')
	.int16('RF6OFF2')
	.uint8('RF1LNB')
	.uint8('RF2LNB')
	.uint8('RF3LNB')
	.uint8('RF4LNB')
	.uint8('RF5LNB')
	.uint8('RF6LNB')
	.uint8('RF1FXDG')
	.uint8('RF2FXDG')
	.uint8('RF3FXDG')
	.uint8('RF4FXDG')
	.uint8('RF5FXDG')
	.uint8('RF6FXDG')
	.array('RESERVED', {
		type: 'int8',
		length: 4,
	})

// TABLE 22: Change Settings Packet (SBS_FM ≥ v1.9)
const REQ_CHANGE_SPEC = Parser.start()
	.endianess('big')
	.uint8('STX', {
		assert: 0x02,
	})
	.uint16('LEN', {
		assert: 0x02,
	})
	.uint8('DTYPE', {
		assert: 0x04,
	})
	.uint32('CF', {
		assert() {
			return true
		}, // TODO
	})
	.uint32('SP', {
		assert() {
			return true
		}, // TODO
	})
	.uint8('RL', {
		assert() {
			return true
		}, // TODO
	})
	.uint8('RBW', {
		assert() {
			return true
		}, // TODO
	})
	.uint8('RF', {
		assert() {
			return true
		}, // TODO
	})
	.uint8('LNB', {
		assert() {
			return true
		}, // TODO
	})
	.array('RESERVED', {
		length: 2,
		type: 'uint8',
	})
	.uint8('ETX', {
		assert: 0x03,
	})

const REQ_CHANGE = function (cf, sp, rl, rbw, rfinput, lnbpower, lnb_22K, lnb_18V) {
	if (!cf) cf = 1250
	if (!sp) sp = 600
	if (!rl) rl = -50
	if (!rbw) rbw = '1MHz'
	if (!rfinput) rfinput = 1
	if (!lnbpower) lnbpower = false
	if (!lnbpower) lnb_22K = false
	if (!lnb_18V) lnb_18K = false // 13V

	let hex = ''
	hex += HEX_STX
	hex += HEX_LEN(3)
	hex += HEX_DTYPE_CHANGE_REQ
	hex += HEX_FROM_NUM(MHzCSW(HZ_FROM_STRING(cf)), 4)
	hex += HEX_FROM_NUM(MHzCSW(HZ_FROM_STRING(rl)), 4)
	hex += HEX_ETX

	const wdr = Buffer.from(hex, 'hex')
	Buffer.from('0200020D03', 'hex')
}

function REQ_CHANGE_TEST() {
	// REQ_CHANGE_SPEC.parse(REQ_CHANGE());
	return null
}
REQ_CHANGE_TEST()

// TABLE 23: Change Settings Packet (SBS_FM ≤ v1.8)
// TODO: like 10-bit, there are some corner cases of parsing that need to be address to support SBS_FM ≤ v1.8 else too.

// TABLE 24: Waveform Data Request
const REQ_WDR_SPEC = new Parser()
	.endianess('big')
	.uint8('STX', {
		assert: 0x02,
	})
	.uint16('LEN', {
		assert: 0x03,
	})
	.uint8('DTYPE', {
		assert: 0x03,
	})
	.uint8('WAVET', {
		assert(val) {
			return val == 3 /* || val == 5 */
		}, // TODO: 10-bit
	})
	.uint8('ETX', {
		assert: 0x03,
	})

const REQ_WDR = function (wavetype) {
	// WAVET: 3 = 8-bin, 5 = 10-bit resolution
	const hex = `${HEX_STX}${HEX_LEN(3)}${HEX_DTYPE_WDR_REQ}${HEX_FROM_NUM(wavetype, 2)}${HEX_ETX}`
	const wdr = Buffer.from(hex, 'hex');
	REQ_WDR_SPEC.parse(wdr) // TODO: should try/catch?
	return wdr
}

function REQ_WDR_TEST() {
	REQ_WDR_SPEC.parse(REQ_WDR(3))
}
REQ_WDR_TEST()

// TABLE 25: Waveform Data Response
// TODO: partial 10-bit support,
// TODO: 8-bit for SBS_FM ≤ v1.8 will likely be broken currently since parse tree is not defined
const WDR = Parser.start()
	.array('DATA', {
		length() {
			return this.DTYPE == 0x09 ? 320 : 480
		}, // 8-bit = 320, 10-bit = 480
		type: 'uint8',
	})
	.uint8('ProdID')
	.uint32('CurCF')
	.uint32('CurSP')
	.uint8('CurRL')
	.uint8('CurRBW')
	.uint8('CurRF')
	.int16('Cur_IEF')
	.int16('Cur_EEF')
	.uint8('CurLNB') // TODO: not using in SBS_FM ≤ v1.8
	.array('RESERVED', {
		// TODO: not using in SBS_FM ≤ v1.8
		length: 2,
		type: 'uint8',
	})

// TABLE 28 - UNKNOWN PACKET RESPONSE
const BAD_PKT = Parser.start().uint8('RxDTYPE')

// internal format used to handle unknown DTYPES to use
const UNK_PKT = Parser.start().array('BLOB', {
	type: 'uint8',
	length() {
		return this.LEN - 2
	}, // gobble up all data, up to the ETX in AVCOOM_CSW
})

// TABLE 29 - DISPLAY TEXT RESPONSE
const MESG = Parser.start().string('MESG', {
	length() {
		return this.LEN - 2
	},
})

// TABLE 4 - PACKET FORMAT
const AVCOM_CSW = new Parser()
	.endianess('big')
	.uint8('STX', {
		assert: 0x02,
	})
	.uint16('LEN')
	.uint8('DTYPE') // TODO: GUI functions aren't support
	.choice('DTYPE_PKT', {
		tag: 'DTYPE',
		choices: {
			0x07: SBS_HW,
			0x0d: LNB_POWER,
			0x09: WDR,
			0x0f: WDR, // TODO: 10-bit support
			0x08: BAD_PKT,
			0x60: MESG,
		},
		defaultChoice: UNK_PKT, // a fake DTYPE_PKT to catch potentially valid item with STX LEN DTYPE ... ETX)
		length() {
			return this.LEN - 2
		}, // LEN includes DTYPE and ETX but not STX
	})
	.uint8('ETX', {
		assert: 0x03,
	})

// Prepare buffer to parse.
const sampleSBS_HW = Buffer.from(
	'020055075A02100000B71B00005265C02D40780A0B8000040D00000000303030303030303033313030343030332C1B0D140A9B7ABFFF00403FAAFF80F8B78162806E7074A580A1FFE77085282821232A4828B7A1FFFBFF03',
	'hex'
)
const sampleLNB_POWER = Buffer.from(
	'02002D0D808010000000000000000000000000000000000000000000000000404040404040FFFFFFFFFFFFFFFFFFFF03',
	'hex'
)
console.log(AVCOM_CSW.parse(sampleSBS_HW))
console.log(AVCOM_CSW.parse(sampleLNB_POWER))

// Parse buffer and show result
// console.log(AVCOM_CSW.parse(buf));

// console.log(require('util').inspect(JPEG.parse(data), { depth: null }));

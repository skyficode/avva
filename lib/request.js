
const { Parser } = require('binary-parser')
let CSW = require('./csw')

class Request {
    constructor() {
    }

    static _parserHardwareInfo() {
        return Parser.start()
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
            }) // TODO: in doc, but is needed...testing needed
            .uint8('ETX', {
                assert: 0x03,
            })
    }
    makeHardwareInfo() {
        return Buffer.from('020003070003', 'hex')
    }

    // TABLE 18 - LNB Power Description Request
    static _parserLNBPower() {
        return Parser.start()
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
    }
    makeLNBPower() {
        return Buffer.from('0200020D03', 'hex')
    }


    // TABLE 22: Change Settings Packet (SBS_FM ≥ v1.9)
    static _parserChangeConfig() {
        return Parser.start()
            .endianess('big')
            .uint8('STX', {
                assert: 0x02,
            })
            .uint16('LEN', {
                assert: 0x0010,
            })
            .uint8('DTYPE', {
                assert: 0x04,
            })
            .uint32('CF', {})
            .uint32('SP', {

            })
            .uint8('RL', {
                // TODO: RL > -50 to 0 for V2 ... see TABLE 5 for notes  
            })
            .uint8('RBW', {
                // TODO
            })
            .uint8('RF', {
                // TODO
            })
            .uint8('LNB', {
                // TODO
            })
            .uint16('RESERVED')
            .uint8('ETX', {
                assert: 0x03,
            })
    }
    makeChangeConfig(cf, sp, rl, rbw, rfinput, lnbpower, lnb_22K, lnb_18V) {
        if (!cf) cf = 1250
        if (!sp) sp = 600
        if (!rl) rl = 50
        if (!rbw) rbw = '1 MHz'
        if (!rfinput) rfinput = 10
        if (!lnbpower) lnbpower = false
        if (!lnb_22K) lnb_22K = false
        if (!lnb_18V) lnb_18V = false // 13V

        let hex = ''
        hex += CSW.STX
        hex += CSW.toLEN(0x0010)
        hex += CSW.DTYPES_STR.CHANGE_REQ
        hex += CSW.hexFromNumber(CSW.toCSW(CSW.normalizeHz(cf, {
            to: 'M',
            includeScale: false,
            assume: 'M',
            outType: 'number'
        })), 8)
        hex += CSW.hexFromNumber(CSW.toCSW(CSW.normalizeHz(sp, {
            to: 'M',
            includeScale: false,
            assume: 'M',
            outType: 'number'
        })), 8)
        hex += CSW.hexFromNumber(rl, 2)
        hex += CSW.toRBWHex(rbw)
        hex += CSW.hexFromNumber(rfinput, 2)
        hex += CSW.hexFromNumber(0, 2) //TODO: Deal with LNB power bitmask, for now just disable
        hex += CSW.hexFromNumber(0, 4)
        hex += CSW.ETX
        //console.log(hex)
        let wdr = Buffer.from(hex, 'hex')
        //Buffer.from('0200020D03', 'hex')
        return wdr;
    }

    // TABLE 23: Change Settings Packet (SBS_FM ≤ v1.8) [FUTURE]
    // TODO: like 10-bit, didn't implement the SBS_FM ≤ v1.8 syntax.


    // TABLE 23: Change Settings Packet (SBS_FM ≤ v1.8)
    // TODO: like 10-bit, there are some corner cases of parsing that need to be address to support SBS_FM ≤ v1.8 else too.

    // TABLE 24: Waveform Data Request
    static _parserWaveformData() {
        return Parser.start()
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
                assert: function (val) {
                    return val == 3 /* || val == 5 */
                }, // TODO: 10-bit
            })
            .uint8('ETX', {
                assert: 0x03,
            })
    }
    makeWaveformData(wavetype) {
        // TODO add CSW.toWAVET to validate: 3 = 8-bin, 5 = 10-bit resolution - currnetly passthru & 8-bit only
        if (wavetype !== 3) {
            wavetype = 3;
        }
        let hex = CSW.STX;
        hex += CSW.toLEN(3);
        hex += CSW.DTYPES_STR.WDR_REQ;
        hex += CSW.hexFromNumber(wavetype, 2);
        hex += CSW.ETX;
        const wdr = Buffer.from(hex, 'hex');
        //this.parserWaveformRequest.parse(wdr) // TODO: should try/catch?
        return wdr
    }
}

module.exports = Request
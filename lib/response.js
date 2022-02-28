const { Parser } = require('binary-parser')
let CSW = require('./csw')

class Response {
    constructor() {
        this.hwVer = {
            major: 0,
            minor: 0
        }
    }

    // TABLE 11 - SBS_HW Description Response
    parserHardwareInfo() {
        return Parser.start()
            .endianess('big')
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
    }

    static fakeHardwareInfo() {
        return Buffer.from(
            '020055075A02100000B71B00005265C02D40780A0B8000040D00000000303030303030303033313030343030332C1B0D140A9B7ABFFF00403FAAFF80F8B78162806E7074A580A1FFE77085282821232A4828B7A1FFFBFF03',
            'hex'
        )
    }

    // TABLE 19 - LNB Power Description Response
    parserPowerLNB() {
        return Parser.start()
            // .uint8('LNB_LOM') is made up of bits // see TABLE 20
            .uint8('LNB_LOM') // 'LNB_LOM'
            // .uint8('LNB_PM') is made up of bits // see TABLE 21
            .uint8('LNB_PM')
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
    }

    static fakePowerLNB() {
        return Buffer.from(
            '02002D0D808010000000000000000000000000000000000000000000000000404040404040FFFFFFFFFFFFFFFFFFFF03',
            'hex'
        )
    }

    // TABLE 25: Waveform Data Response
    // TODO: partial 10-bit support,
    // TODO: 8-bit for SBS_FM ≤ v1.8 will likely be broken currently since parse tree is not defined
    parserWaveformData() {
        return Parser.start()
            .array('DATA', {
                length: 320, // 8-bit = 320, 10-bit = 480
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
            .uint16('RESERVED')
    }
    static fakeWaveformData() {
        let hex = "0201550948494B4A4A4B4B4C4D4D4E4E4F4E50504E53525152535352586C77726253545577806E5F545453545354525251546973775A5553555352525455555453656A6A6A5E5F707F837D7D7F8B7A8691908F8E8D7E5A575B5859595858595756555756565757575656565754585556555457555657575856536FA07869585756565857595A5B5F667A766E8F90675F70A1817C74646C6B636D767478685C5D6B73747C7B7C7B7B7C7C7B7D7B79777979736F67605A5B8274818A8D8C8B846F6868676A788F7B6D635C63756E59595A5C5B5A5B595B5C5C5B5D5B5B595A5A5B5A5A575753547983928B7D596D755D575755555554565255576A7F5E5754525255545454545554576B70686C6D6F70635855565859575C76777375725B565657565554578387827E7E5A54525151505152525151514F5457504E4F4F4E4E4F5151504F515A00B71B00005265C02D400A0000000040FFFF03"
        //hex = String(hex.replace(/\D/g, ''));
        //console.log(hex, hex.length, hex.length.toString(16) / 2, hex.length / 2)
        return Buffer.from(hex, 'hex');
    }

    // TABLE 28 - UNKNOWN PACKET RESPONSE
    parserUnknownPacket() {
        return Parser.start().uint8('RxDTYPE')
    }

    // internal format used to handle unknown DTYPES to use
    parserGlob() {
        return Parser.start().array('BLOB', {
            type: 'uint8',
            length: function () {
                return parseInt(this.LEN, 16) - 2
            }, // gobble up all data, up to the ETX in AVCOM_CSW
        })
    }

    // TABLE 29 - DISPLAY TEXT RESPONSE
    parserTextMessage() {
        return Parser.start().string('MESG', {
            length: function (val) {
                //console.log('this', this, 'val', val, 'LEN', val.LEN)
                return val.LEN - 2
            },
        })
    }
    static fakeTextMessage() {
        let hex = "02000760736b79666903"
        //hex = String(hex.replace(/\D/g, ''));
        //console.log(hex, hex.length, hex.length.toString(16) / 2, hex.length / 2)
        return Buffer.from(hex, 'hex');
    }

    // TABLE 4 - PACKET FORMAT
    parser() {
        return Parser.start()
            .endianess('big')
            .uint8('STX', {
                assert: 0x02,
            })
            .uint16('LEN')
            .uint8('DTYPE') // TODO: GUI functions aren't support
            .choice('DTYPE_PKT', {
                tag: 'DTYPE',
                choices: {
                    0x07: this.parserHardwareInfo(),
                    0x0d: this.parserPowerLNB(),
                    0x09: this.parserWaveformData(),
                    0x0f: this.parserWaveformData(), // TODO: 10-bit support, diff data len calc needed
                    0x08: this.parserUnknownPacket(),
                    0x60: this.parserTextMessage(),
                },
                defaultChoice: this.parserGlob(), // a fake DTYPE_PKT to catch potentially valid item with STX LEN DTYPE ... ETX)
                length: function () {
                    console.log('length', LEN, this.LEN.length);
                    return this.LEN - 2;
                }, // LEN includes DTYPE and ETX but not STX*/
            })
            .uint8('ETX', {
                assert: 0x03,
            })
    }


    static parseTester() {
        return Parser.start()
            .endianess('big')
            .uint8('STX', {
                assert: 0x02,
            })
            .uint16('LEN', {})
            .uint8('DTYPE', {})
            .array('BLOB', {
                type: 'uint8',
                length: function () {
                    return this.LEN - 2
                }
            })
            .uint8('ETX', {
                assert: 0x03,
            })
    }


    static start(debug) {
        let reqParser = new Response()
        /*if (debug) {
            return reqParser.parseTester()
        }*/
        return reqParser.parser()
    }
}

module.exports = Response

class CSW {

    // HEXSTRINGS FOR AVCOM TYPES
    static get DTYPES_STR() {
        return {
            SBS_HW: '07',
            LNB_POWER: '0D',
            WDR_REQ: '03',
            WDR: '09',
            CHANGE_REQ: '04',
            BAD_PKT: '08',
            MESG: '60'
        }
    }
    static get DTYPES_NUM() {
        let dtypes = {};
        Object.keys(CSW.DTYPES_STR).map(function (key, index) {
            dtypes[key] = Buffer.from(CSW.DTYPES_STR[key], 'hex').readUInt8();
        });
        return dtypes;
    }

    static get STX() {
        return '02';
    }
    static get ETX() {
        return '03';
    }

    // HEXSTRING HELPERS
    static hexFromNumber(num, len) {
        if (!len) {
            len = 2
        }
        num = Number(num)
        if (typeof num === 'number') {
            if (Math.pow(16, len) > num) {
                return num.toString(16).padStart(len, '0')
            }
        }
        throw new Error(`CSW.hexFromNumber() has bad input ${num} with length ${len}`)
    }

    static toLEN(num) {
        return CSW.hexFromNumber(num, 4)
    }

    // TABLE 5 - RL
    /* 
    SBS_FM < v3.0 - range -10 to -50, up -70 with addon = unsigned UInt8
    SBS_FM > v3.0 - range +10 to -50, up -70 with addon = signed UInt8 (to support RL +10Mhz)
    */

    // TABLE 6 - RBW Mask
    // TODO: Use to these name in parse code instead of raw bits

    static get enumRBW() {
        return ['RESERVED', '200 KHz', '3 KHz', '10 KHz', '100 KHz', '300 KHz', '1 MHz', '3 MHz']
    }

    static toRBWHex(enumRBW) {
        if (!enumRBW || enumRBW.toUpperCase() === 'RESERVED') {
            return '00';
        }
        enumRBW = CSW.normalizeHz(enumRBW, {
            assume: 'K',
            outType: 'string',
            includeScale: true
        })
        let idx = CSW.enumRBW.findIndex((val) => enumRBW === val)
        if (idx === -1) {
            idx = 7
        }
        let bitmask = '0b1'.padEnd(idx + 3, '0');
        let result = CSW.hexFromNumber(Number(bitmask), 2)
        //console.log('toRBWHex',enumRBW,idx,bitmask,result)
        return result

    }

    static fromRBWHex(RBW) {
        if (RBW && typeof RBW === 'string') {
            RBW = Buffer.from(RBW, 'hex').readUInt8()
        }
        let vals = CSW.enumRBW.filter((val, idx) => RBW & (1 << idx))
        let result = vals[0]
        //console.log(RBW,RBW.toString(2),result,vals)
        return result
    }


    // TABLE 12 - ProdID Byte
    static toProductName(prodIdHex) {
        // TODO: Deal with all cases & use name in parser
        switch (prodIdHex) {
            case 0x3a:
                return '2150'
            case 0x4a:
                return '1100'
            case 0x5a:
                return '2500 or 5000'
            default:
                return 'TODO'
        }
    }

    // WAVEFORM DATA POINTS
    // TODO: 10-bit version of below

    static toDb(wdp, crl, useV3logic) {
        if (!useV3logic) {
            // eq(1) - for SBS_FM v1.X and v2.X
            return 0.2 * wdp - (crl + 40)
        } else { // eq(2) - for SBS_FM v3.X

            return 0.2 * wdp + (crl - 40)
        }
    }

    // FREQUENCY CALCULATIONS
    // Center frequency (CF) and Span (SP) are represented in the CSW protocol as a four‐byte or unsigned 32‐bit value. 
    // To convert a frequency in megahertz’s to the CSW protocol format (such as CF and SP), 
    // use equation eq(1) below where F is the frequency in the CSW protocol format. 
    // This is most frequently used in packet requests.
    static toCSW(mhz) { // eq(1):  F = Freqency(MHz) * 10,000
        return mhz * 10000
    }
    static fromCSW(F) { // eq(2):  Freqency(MHz) = F / 10,000
        return F / 10000
    }

    // HUMANIZING HELPERS
    // opts = {to: 'G[MK]', includeScale: false, assume: 'M[GK]', outType: 'string'|'number' '}
    // TODO: opt.to = 'automatic' (pick scale based on size)
    static normalizeHz(hzstring, opts) {

        // HANDLE OPTIONS TO USE
        if (!opts) {
            opts = {}
        }
        let outputUnits = opts.to;
        if (!outputUnits || typeof outputUnits !== 'string') {
            outputUnits = '';
        }
        outputUnits = outputUnits.toUpperCase()
        if (outputUnits.length > 0) {
            let char = outputUnits[0]
            if (!(char === 'G' || char === 'M' || char === 'K')) {
                outputUnits = '';
            }
        }
        let includeScale = opts.includeScale;
        if (typeof includeScale !== 'boolean') {
            includeScale = true;
        }

        // PARSE INPUT
        const re = /(?<fq>[-]?\d+(\.\d+)*)\s*(?<GMK>[MmKkGg])?/
        const match = `${hzstring}`.toUpperCase().match(re)
        if (!match || !match.groups.fq) {
            console.error(
                'HZ_STRING is not parsable. Any number followed by a G/M/K should work.',
                hzstring,
                match
            )
            throw new Error('CSW.normalizeHz() regex failed to parse input')
        }
        const fq = Number(match.groups.fq)
        if (!fq) {
            console.error(
                'CSW.normalizeHz() regex found no frequency in input, some number string is required.',
                hzstring,
                match
            )
            throw new Error('CSW.normalizeHz() is missing a frequency')
        }
        let GMK = ''
        let assume = opts.assume
        if (match.groups.GMK) {
            GMK = match.groups.GMK.toUpperCase()
        } else if (assume && typeof assume === 'string' && assume.length > 0) {
            let char = assume[0].toUpperCase()
            if (char === 'G' || char === 'M' || char === 'K') {
                GMK = char
            }
        }

        // CONVERT TO HZ AS BASELINE
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

        // SCALE AS DESIRED FOR OUTPUT
        if (GMK !== '' && outputUnits === '') {
            // keep same units if called without options
            outputUnits = GMK;
        }
        let scaled = null
        switch (outputUnits) {
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
                console.error('CSW.normalizeHz() output scale is invalid, must be M, G, K or blank.', hzstring, outputUnits)
                throw new Error('CSW.normalizeHz() outputUnits was not valid/unknown thus unhandled')
        }
        //scaled = `${scaled} ${units}Hz`
        //console.log('CSW.normalizeHz() got', scaled, '<= (', hzstring, 'to', outputUnits, 'assume', assume, '', )', fq, '~', hz, 'hz')

        // OUTPUT RESULTS, INCLUDE SCALE (GHz, MHz, etc) UNLESS opts.includeScale == false
        if (opts.outType && typeof opts.outType === 'string' && opts.outType === 'number') {
            return scaled
        } else if (includeScale) {
            return `${scaled} ${outputUnits}Hz`
        } else {
            return `${scaled}`
        }
    }
}
module.exports = CSW
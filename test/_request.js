let expect = require('chai').expect;
let Request = require('../lib/request')


describe('CSW request generator', function () {
    before(function () {
        this.req = new Request();
    })
    it('makeHardwareInfo() is valid', function () {
        let cmd = this.req.makeHardwareInfo()
        let csw = Request._parserHardwareInfo()
        let result = csw.parse(cmd)
        expect(result).to.have.all.keys(['STX', 'LEN', 'DTYPE', 'PADD', 'ETX'])
        expect(result).to.include({
            'DTYPE': 7
        })
    })
    it('makeLNBPower()', function () {
        let cmd = this.req.makeLNBPower()
        let csw = Request._parserLNBPower()
        let result = csw.parse(cmd)
        expect(result).to.have.all.keys(['STX', 'LEN', 'DTYPE', 'ETX'])
        expect(result).to.include({
            'DTYPE': 0x0d
        })
    })

    context('makeChangeConfig()', function () {
        it('basically works', function () {
            let cmd = this.req.makeChangeConfig()
            //console.log(cmd.toString('hex'));
            let csw = Request._parserChangeConfig()
            let result = csw.parse(cmd)
            //console.log(csw, result)
            expect(result).to.include.key('STX')
            expect(result).to.include.key('LEN')
            expect(result).to.include.key('ETX')
            expect(result).to.include({
                'DTYPE': 0x04
            })
        })
        it('needs more tests and review but working...')
    })
    context('makeWaveformData()', function () {
        it('basically works', function () {
            let cmd = this.req.makeWaveformData()
            //console.log(cmd.toString('hex'));
            let csw = Request._parserWaveformData()
            let result = csw.parse(cmd)
            //console.log(csw, result)
            expect(result).to.include.key('STX')
            expect(result).to.include.key('LEN')
            expect(result).to.include.key('LEN')
            expect(result).to.include.key('ETX')
            expect(result).to.include({
                'DTYPE': 0x03
            })
        })
        it('needs more tests and review but working...')
    })
})
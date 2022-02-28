
let expect = require('chai').expect;
let Response = require('../lib/response')


describe('CSW response parser', function () {
    context('using .testParser()', function () {
        before(function(){
            this.testParser = Response.parseTester()
        })
        it('finds HardwareInfo', function () {
            let cmd = Response.fakeHardwareInfo()
            let result = this.testParser.parse(cmd)
            expect(result)
                .to.include({
                    'DTYPE': 0x07
                })
        })
        it('finds LNBPower', function () {
            let cmd = Response.fakePowerLNB()
            let result = this.testParser.parse(cmd)
            expect(result)
                .to.include({
                    'DTYPE': 0x0D
                })
        })
        it('finds WaveformData', function () {
            let cmd = Response.fakeWaveformData()
            let result = this.testParser.parse(cmd)
            expect(result)
                .to.include({
                    'DTYPE': 0x09
                })
        })
        it('finds TextMessage', function () {
            let cmd = Response.fakeTextMessage()
            let result = this.testParser.parse(cmd)
            expect(result)
                .to.include({
                    'DTYPE': 0x60
                })
        })
        it('throws with bad data...')
    })
    context('using .start()', function () {
        before(function () {
            this.resp = Response;
            this.parser = this.resp.start()
        })
        it('parserHardwareInfo() works', function () {
            let cmd = this.resp.fakeHardwareInfo()
            //console.log(cmd.toString('hex'));
            let result = this.parser.parse(cmd)
            //console.log(result)
            expect(result)
                .to.include({
                    'DTYPE': 0x07
                })
        })
        it('parserPowerLNB() works', function () {
            let cmd = this.resp.fakePowerLNB()
            //console.log(cmd.toString('hex'));
            let result = this.parser.parse(cmd)
            //console.log(result)
            expect(result)
                .to.include({
                    'DTYPE': 0x0d
                })
        })
        it('parserWaveformData() works', function () {
            let cmd = this.resp.fakeWaveformData()
            //console.log(cmd.toString('hex'));
            let result = this.parser.parse(cmd)
            //console.log(result)
            expect(result)
                .to.include({
                    'DTYPE': 0x09,
                    'LEN': 341,
                })
            expect(result.DTYPE_PKT)
                .to.include({
                    'CurRL': 45
                })
        })
        it('parserTextMessage()', function() {
            let cmd = this.resp.fakeTextMessage()
            let result = this.parser.parse(cmd)
            //console.log(result)
            expect(result)
                .to.include({
                    'DTYPE': 0x60
                }).and
                .to.include.key('DTYPE_PKT')
            expect(result.DTYPE_PKT)
                .to.include.key('MESG')
            expect(result.DTYPE_PKT.MESG)
                .to.equal('skyfi')
        })
        it('parserUnknownPacket()')
        it('parserGlob()')
    })
})
require('dotenv').config()
let debug = require('debug')('test:debug')
let expect = require('chai').expect;
let Client = require('../lib/client')
let Simulator = require('../lib/simulator')

describe('CSW "client" to AVCOM SA protocol "server"', function () {
    let sim = null
    beforeEach(function(done) {
        sim = Simulator.start(done) 
    })
    afterEach(function(done) {
        sim.end(done)
    })
    it('new Client()', function() {
        let client = new Client()
        expect(client);
    })
    it('static connect()', function(done) {
        let client = Client.connect()
        expect(client)
        client.close(done)
    })
    it('open()')
    it('requestHardwareInfo()')
    it('requestConfigChange(cf, sp, rl, rbw)')
    it('requestLNBPower()')
    it('requestWaveformData(8-bit only')
    it('also needs tests')
})
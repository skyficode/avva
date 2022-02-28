require('dotenv').config()
let expect = require('chai').expect;
let net = require('net')
let Simulator = require('../lib/simulator')

describe('CSW Simulator', function () {
    context('mock AVCOM SA helpers', function () {
        it('start() exists', function(done) {
            let sim = Simulator.start()
            expect(sim).does.exist;
            sim.end(done)
        })
        it('queueResponse(<hexstring>) works', function(done) {
            let sim = Simulator.start(() => {
            sim.queueResponse("030001FF04")
            expect(sim.pendingResponses).has.lengthOf(1)
            })
            sim.end(done)
        })
        it('clients can connect', function(next) {
            let sim = Simulator.start(() =>{
            let socket = net.createConnection({
                port: process.env.AVCOM_PORT,
                host: 'localhost'
            });
            socket.on('connect', function() {
                socket.destroy()
                sim.end(() => next())
            });
            socket.on('error', function(error) {
                console.error(error)
                socket.destroy()
                sim.end(() => next(error))
            });})
        })
        it('basic operation to control responses works', function(next) {
            let sim = Simulator.start()
            let socket = net.createConnection({
                port: process.env.AVCOM_PORT,
                host: 'localhost'
            });
            socket.on('connect', function() {
                console.log('connect')
                socket.on('data', function(data) {
                    console.log('data', data)
                    socket.destroy()
                    sim.end(() => next())
                })
                sim.queueResponse("030001FF04")
            });
            socket.on('error', function(error) {
                socket.destroy()
                sim.end(() => next(error))
            });
        })
    })

})

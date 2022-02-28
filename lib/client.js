require('dotenv').config()
const EventEmitter = require('events');
let net = require('net')
let Request = require('./request.js');
let ResponseHandler = require('./handler.js')

class Client extends EventEmitter {
    constructor() {
        super();
        this.socket = null;
        this.connected = false;
        this.handler = false;
    }

    static connect() {
        let client = new Client();
        client.open();
        return client
    }

    open() {
        if (!this.socket) {
            this.socket = net.createConnection({
                port: process.env.AVCOM_PORT,
                host: process.env.AVCOM_IP
            });
            this.socket.on('connect', this.onConnect);
        } else {
            console.log('socket already exists...')
        }
    }

    emitBuffer(buffer) {
        console.log('emitBuffer got', buffer)
        this.emit('buffer', buffer);
    }

    onConnect(socket) {
        this.connected = true;
        this.handler = ResponseHandler.listen(socket)
        this.handler.on('buffer', this.emitBuffer)
    }

    requestHardwareInfo() {
        let req = new Request()
        let cmd = req.makeHardwareInfo()
        this.writeRaw(cmd)
    }

    requestConfigChange(cf, sp, rl, rbw, rfinput, lnbpower, lnb_22K, lnb_18V) {
        let req = new Request()
        let cmd = req.makeChangeConfig(cf, sp, rl, rbw, rfinput, lnbpower, lnb_22K, lnb_18V)
        this.writeRaw(cmd)
    }

    requestLNBPower() {
        let req = new Request()
        let cmd = req.makeLNBPower()
        this.writeRaw(cmd)
    }

    requestWaveformData(bits = 8) {
        let req = new Request()
        let cmd = req.makeWaveformData() //TODO 10-bit
        this.writeRaw(cmd)
    }

    writeRaw(buffer) {
        if (typeof buffer === 'string') {
            buffer = Buffer.from(buffer, 'hex')
        }
        // TODO: Verify string is hex and buffer is buffer... 
        this.socket.write(buffer)
    }

    close(done) {
        this.socket.end(done);
        this.socket = null;
        this.connected = false;
    }

    reset() {
        this.socket.close()
    }
}

module.exports = Client
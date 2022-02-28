
require('dotenv').config()
let net = require('net')
let debug = require('debug')
let log = {
    event: debug('simulator:event'),
    req: debug('simulator:got'),
    resp: debug('simulator:sent')
}
class Simulator {
    constructor(port) {
        this.server = null;
        this.port = port || process.env.AVCOM_PORT;
        this.pendingResponses = [];
        this.waiters = [];
        log.event('created')
    }

    static start(callback) {
        let sim = new Simulator()
        sim.listen(callback);
        log.event('started and listening')
        return sim;
    }

    queueResponse(buffer) {
        if (typeof buffer === 'string') {
            this.pendingResponses.push(Buffer.from(buffer, 'hex'))
        } else {
            this.pendingResponses.push(buffer)
        }
    }

    processResponseQueue(socket) {
        if (socket && this.pendingResponses) {
            while (this.pendingResponses.length > 0) {
                let buf = this.pendingResponses.shift()
                log.req(`\t${buf.toString('hex')} (${buf.length})`);
                socket.write(buf)
            }
        }
    }

    listen(onListen) {
        this.server = net.createServer();
        this.server.listen(this.port, () => {if(onListen) onListen();});
        this.server.on('connection', socket => {
            log.event('connection opened')
            socket.on('data', data => {
                log.req(`\t\t${data.toString('hex')} (${data.length})`);
                this.processResponseQueue(socket)
            });

            let bgSender = setInterval(() => {
                this.processResponseQueue(socket)
            }, 333)
            this.waiters.push(bgSender)
        });
        return this;
    }

    end(callback) {
        while (this.waiters.length > 0) {
            clearInterval(this.waiters.shift())
        }
        if (this.server) {
            this.server.close(callback)
            this.server = null
        }
    }
}

module.exports = Simulator
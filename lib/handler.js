let Response = require('./response')
const EventEmitter = require('events');

class ResponseHandler extends EventEmitter {
    constructor(socket) {
        super()
        this.queue = new Array()
        this.queueSize = 0;
        this.socket = socket || null;
        this.loopCount = 0;
    }

    static listen(socket) {
        let handler = new ResponseHandler(socket)
        socket.on('data', handler.onData)
        return handler;
    }

    onData(buffer) {
        this.queueSize = this.queue.push(buffer)
        this.processQueue()
    }

    parsePartial(hexstring) {
        let buf = hexstring;
        if (buf.length > 5) {
            let len = Buffer
                .from(buf[2] + buf[3] + buf[4] + buf[5], 'hex')
                .readUInt16BE();
            if (len < buf.length) {
                let parsedCmd = null;
                try {
                    parsedCmd = Response.parseTester(Buffer.from(buf, 'hex'));
                    return len + 4 + 1;
                } catch (error) {
                    console.error('FAILED to find command in pending buffer: ', error)
                    return -1;
                }
            }
        } else return -1;
    }

    processQueue() {
        // check count of recurision
        // too high
        // search next '03' in buffer and start there
        if (queueSize > 0) {
            let idxCol = -1
            let pendingBuffer = null;
            // go through received data
            for (let idxRow in this.queue) {
                if (typeof pendingBuffer === null) {
                    idxCol = this.queue[idxRow].indexOf(CSW.STX, 'hex')
                    if (stxIndex !== -1) {
                        // found one
                        pendingBuffer = this.queue[idxRow].hexSlice(stxIndex)
                    }
                } else {
                    pendingBuffer += this.queue[idxRow].hexSlice()
                }
                let indexEnd = this.parsePartial(pendingBuffer)
                if (indexEnd) {
                    // put remaining into unprocessed queue
                    //queue.unshift(pendingBuffer.slice(indexEnd))
                    this.loopCount = 0;
                    this.queue = [];
                    this.emit('command', indexEnd);
                    break;
                }
            }
            this.loopCount++
            console.log('processQueue() STX not found in any buffer. Will retry with more data.')

        } else {
            console.log('processQueue() has no queue to process')
        }
        // parsed ok
        // find end of result
        // remove buffer, add remaining to head
        // call again
        // no command
    }
}

module.exports = ResponseHandler;
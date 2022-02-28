# avva
#### JavaScript library for communicating with AVCOM's Spectrum Analyzers

*Beware: this library was under-development and maybe incomplete or buggy! This code is shared "as-is" – more example of the parsing `AVCOM CSW Protocol` then a ready-to-use package.*

The `avva` library communicates with an AVCOM SA using TCP and then can both send command and parse responses, including the spectrum data stream.  `avva` can be included in another project's `package.json`, same as any other node.js package, and used via the exported objects. 

 *However an AVCOM SA may only allow one TCP session so this approach has some limitations – specifically you may not be able to use the existing AVCOM tools at the same time as the `avva` library*

To provide support for multiple client users of the same spectrum analyser, the library could be extended to allow running as "deamon", and expose **POSSIBLE** **FUTURE** network APIs including:
  * Web server with basic HTML5 UI to emulate the existing SA clients
  * REST API for commands such as set FQ, etc.
  * WebSockets to stream spectrum data to most "modern things", like scientific notebooks including Wolfram, Observerable, or Julia+Pluto which offer rich graphing support.
  * Send a RTP **multicast** stream using "RTP events" containing the current spectrum data.  *Idea here be that the spectrum data could then be carried (e.g. `mux`) as part of a larger DVB GSE stream from a remote, for QA at a recieving earthstation hub.*

None of these are implemented today.
## Toolchain

#### `node` 

Any recent version should work.  This was developed using NodeJS v12 to v16 releases however.

#### `yarn`
This packages uses `yarn` for NodeJS packages.  Although, `npm` likely works.  

#### Environment Variables 
The dotenv package is used to load the needed AVCOM SA IP and port from environment variables or a `.env` file located in the root of the source code.  _The unit test need the .env set to `localhost` since the launch an poor-man's AVCOM simulator._

These environment variables are used:
```
AVCOM_IP=localhost
AVCOM_PORT=26482
```

## Specifications

The implementation is based on a PDF document provided by AVCOM that documents what they call `CSW Protocol`.  The protocol specs are included in `./etc` for reference here.  Their C/C++ example of CSW is also in the same directly.

## Using this source code

About the only thing you can do is run the unit test.  Even these were work in progress.  

Most of the source code in `./lib` should be "buggy but complete" - but without full unit test hard to know _how buggy_.  

To use the library in your own code, the unit test in `./test` should provide a good example of CSW usage.

### `yarn install` to install or update packages

```
➜  avva git:(main) ✗ yarn install
yarn install v1.22.17
[1/4] 🔍  Resolving packages...
success Already up-to-date.
✨  Done in 0.18s.
```

### `yarn audit` to check for knnown vulnerabilities in deps

```
➜  avva git:(main) ✗ yarn audit  
yarn audit v1.22.17
0 vulnerabilities found - Packages audited: 94
✨  Done in 1.21s.
```

### `yarn test` to run the unit tests


```
➜  avva git:(main) ✗ yarn test   
yarn run v1.22.17
$ mocha


  CSW "client" to AVCOM SA protocol "server"
    ✔ new Client()
    1) static connect()
    2) "after each" hook for "static connect()"

  CSW protocol helpers
    DTYPE enum
      - needs test
    hexFromNumber()
      ✔ convert 1 to "01"
      ✔ takes a zero-padding length
      ✔ overflowing zero stuffing length, throws
      ✔ with string input, throws
    RBW enum & converters
      - is untested and partially implemented
      ✔ enumRBW looks right
      ✔ toRBWHex() works for "1 Mhz"
      ✔ toRBWHex() can handle a abbreviation like "200K"
      ✔ toRBWHex() get the reserved bit
      ✔ fromRBWHex() works for "02" = 200K
      ✔ fromRBWHex() works for "64" = 1M
    Waveform datapoint
      - converts CSW/RBW format to dB
      - converts MHz to CSW
      - converts CSW to MHz
    normalizeHz()
      ✔ does something
      ✔ works for inside CF request
      - needs more tests and code refinement

  CSW response handler
    - needs implementation!
    - also needs tests

  CSW request generator
    ✔ makeHardwareInfo() is valid
    ✔ makeLNBPower()
    makeChangeConfig()
      ✔ basically works
      - needs more tests and review but working...
    makeWaveformData()
      ✔ basically works
      - needs more tests and review but working...

  CSW response parser
    using .testParser()
      ✔ finds HardwareInfo
      ✔ finds LNBPower
      ✔ finds WaveformData
      ✔ finds TextMessage
      - throws with bad data...
    using .start()
      ✔ parserHardwareInfo() works
      ✔ parserPowerLNB() works
      ✔ parserWaveformData() works
      ✔ parserTextMessage()
      - parserUnknownPacket()
      - parserGlob()

  CSW Simulator
    mock AVCOM SA helpers
      ✔ start() exists
      ✔ queueResponse(<hexstring>) works
      ✔ clients can connect
connect
data <Buffer 03 00 01 ff 04>
      ✔ basic operation to control responses works (339ms)


  29 passing (2s)
  13 pending
  2 failing

  1) CSW "client" to AVCOM SA protocol "server"
       static connect():
     Uncaught TypeError: Cannot read properties of undefined (reading 'on')
      at Function.listen (lib/handler.js:15:16)
      at Socket.onConnect (lib/client.js:40:40)
      at Socket.emit (node:events:532:35)
      at TCPConnectWrap.afterConnect [as oncomplete] (node:net:1143:10)

  2) CSW "client" to AVCOM SA protocol "server"
       "after each" hook for "static connect()":
     Error: Timeout of 2000ms exceeded. For async tests and hooks, ensure "done()" is called; if returning a Promise, ensure it resolves. (/Users/ammo/Git/shafdog/avva/test/_client.js)
      at listOnTimeout (node:internal/timers:559:17)
      at processTimers (node:internal/timers:502:7)


```

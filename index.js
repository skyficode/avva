require('dotenv').config()
require('debug')('test:debug')
module.exports = {
    Client: require('./lib/client'),
    Simulator: require('./lib/simulator'),
    CSW: require('./lib/csw'),
    Request: require('./lib/request'),
    Response: require('../lib/response'),
    Simulator: require('../lib/simulator')
}

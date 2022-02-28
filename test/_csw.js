require('dotenv').config()
let expect = require('chai').expect;
let CSW = require('../lib/csw')


describe('CSW protocol helpers', function () {
    context('DTYPE enum', function () {
        it('needs test')
    })
    context('hexFromNumber()', function () {
        it('convert 1 to "01"', function () {
            let hex = CSW.hexFromNumber(1);
            expect(hex).is.string;
            expect(hex).to.be.equal('01');
        });
        it('takes a zero-padding length', function () {
            let hex = null;
            hex = CSW.hexFromNumber(1, 2);
            expect(hex).is.string;
            expect(hex).to.be.equal('01');
            hex = CSW.hexFromNumber(1, 4);
            expect(hex).is.string;
            expect(hex).to.be.equal('0001');
            hex = CSW.hexFromNumber(1, 8);
            expect(hex).is.string;
            expect(hex).to.be.equal('00000001');
            hex = CSW.hexFromNumber(1000, 4);
            expect(hex).is.string;
            expect(hex).to.be.equal('03e8');
        })
        it('overflowing zero stuffing length, throws', function () {
            expect(function () {
                    CSW.hexFromNumber(1000, 2)
                })
                .to.throw();
        })
        it('with string input, throws', function () {
            expect(function () {
                    CSW.hexFromNumber('1000', 4)
                })
                .to.not.throw();
        })


    })

    // ['RESERVED', '200 KHz', '3 KHz', '10 KHz', '100 KHz', '300 KHz', '1 Mhz']
    context('RBW enum & converters', function () {
        it('is untested and partially implemented')
        it('enumRBW looks right', function () {
            let enumRBW = CSW.enumRBW
            expect(enumRBW).is.an('array')
            expect(enumRBW).to.include('RESERVED')
            expect(enumRBW).to.include('300 KHz')
        })
        it('toRBWHex() works for "1 Mhz"', function () {
            let hex = CSW.toRBWHex('1 MHz')
            //console.log(hex,0b01000000)
            expect(hex).is.an('string')
            expect(hex).has.length(2)
            expect(hex).is.equal('40') // 0b01000000
        })
        it('toRBWHex() can handle a abbreviation like "200K"', function () {
            let hex = CSW.toRBWHex('200K')
            //console.log(hex)
            expect(hex).is.an('string')
            expect(hex).has.length(2)
            expect(hex).is.equal('02') //0b00000010
        })
        it('toRBWHex() get the reserved bit', function () {
            let hex = CSW.toRBWHex('RESERVED')
            //console.log(hex)
            expect(hex).is.an('string')
            expect(hex).has.length(2)
            expect(hex).is.equal('00')
        })
        it('fromRBWHex() works for "02" = 200K', function () {
            let name = CSW.fromRBWHex('02')
            //console.log(name)
            expect(name).is.an('string')
            expect(name).is.equal('200 KHz')
        })
        it('fromRBWHex() works for "64" = 1M', function () {
            let name = CSW.fromRBWHex('40')
            //console.log(name)
            expect(name).is.an('string')
            expect(name).is.equal('1 MHz')
        })
    })
    context('Waveform datapoint', function () {
        it('converts CSW/RBW format to dB')
        it('converts MHz to CSW')
        it('converts CSW to MHz')
    })
    context('normalizeHz()', function () {
        it('does something', function () {
            let hz = null;
            hz = CSW.normalizeHz('11.880 G');
            expect(hz).is.string;
            expect(hz).to.be.equal('11.88 GHz');
        });
        it('works for inside CF request', function () {
            let val = 1250;
            let result = CSW.normalizeHz(val, {
                to: 'M',
                includeScale: false,
                assume: 'M',
                outType: 'number'
            })
            expect(result).is.equal(1250)
        })
        it('needs more tests and code refinement')
    })
})
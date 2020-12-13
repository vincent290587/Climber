
console.log('Create monitor')

const winston = require('winston')
const myWinstonOptions = {
    format: winston.format.simple(),
    transports: [
        new winston.transports.File({ filename: 'Tester.log' }),
    ],
    exitOnError: false,
    handleExceptions: true,
}
const logger = new winston.createLogger(myWinstonOptions)


try {
	const SerialPort = require('serialport')
	var port = new SerialPort('COM22', {
        baudRate: 115200,
        rtscts: true,
    })
    
    port.on('data', function(data) {
        console.log('< ' + data)
        logger.info('< ' + data)
    });
	
	port.on('close', () => {
        try {
            logger.error('Port reconnect')
            setTimeout(this.reconnect.bind(this), 5000);
        } catch(e) {
            console.log(e)
        }
    });
	
	port.on('error', () => {
        try {
            logger.error('Port reconnect')
            setTimeout(this.reconnect.bind(this), 5000);
        } catch(e) {
            console.log(e)
        }
    });
	
} catch(e) {
    console.log(e)
}

var slope_pc = 0

var interval = 1 * 1000; // 1 second;

for (var i = 0; i <=200; i++) {
    setTimeout( function (i) {

        if (i % 15 == 0) {
            if ((i / 15) % 2 == 0) slope_pc = 10
            else slope_pc = 0
        }
        var slope = (slope_pc + 200) * 100

        let ser_msg = '>S' + slope.toFixed(0) + '\n'
        console.log(ser_msg)
                
        try {
            port.write(ser_msg)
            logger.info(ser_msg)
        } catch(e) {
            console.log(e)
        }
    }, interval * i, i);
}
            


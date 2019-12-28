
try {
    console.log('Require: ZwiftPacketMonitor')
    var ZwiftPacketMonitor = require('@zwfthcks/zwift-packet-monitor')
    console.log('Create monitor')
} catch(e) {
    console.log(e)
}

try {
    var Cap = require('cap').Cap;
} catch(e) {
    console.log(e)
}

const winston = require('winston')
const myWinstonOptions = {
    format: winston.format.simple(),
    transports: [
        new winston.transports.File({ filename: 'Climber.log' }),
    ],
    exitOnError: false,
    handleExceptions: true,
}
const logger = new winston.createLogger(myWinstonOptions)

const ip = require('internal-ip').v4.sync()
var distance_prev=0
var altitude_prev=0
var nb_runs=0

try {
	const SerialPort = require('serialport')
	var port = new SerialPort('COM22', {
        baudRate: 115200
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

if (ZwiftPacketMonitor && Cap) {

    console.log('Listening on: ', ip, JSON.stringify(Cap.findDevice(ip),null,4));
    
    // determine network interface associated with external IP address
    interface = Cap.findDevice(ip);

    // ... and setup monitor on that interface:
    const monitor = new ZwiftPacketMonitor(interface)
    
    monitor.on('outgoingPlayerState', (playerState, serverWorldTime) => {
		
        if (playerState.distance > distance_prev + 5 && nb_runs > 0) {
		
			console.log(serverWorldTime, playerState)
            logger.info('New state')
            
            var angle = Math.asin((playerState.altitude - altitude_prev) / (200 * (playerState.distance - distance_prev)))
            var slope_pc = 200 * Math.tan(angle)
            var slope = (Math.round(slope_pc) / 2 + 200) * 100

            let ser_msg = '>S' + slope.toFixed(0) + '\n'
        
            try {
                port.write(ser_msg)
                logger.info(ser_msg)
            } catch(e) {
                console.log(e)
            }

            distance_prev = playerState.distance
            altitude_prev = playerState.altitude

        } else if (nb_runs == 0) {
            
            distance_prev = playerState.distance
            altitude_prev = playerState.altitude
            console.log('Init done')
        }

        nb_runs = nb_runs + 1

    })
    
    monitor.start()
	
    try {
        port.write('>S20000\n')
    } catch(e) {
        console.log(e)
    }
}

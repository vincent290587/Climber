
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
var slope_prev=0
var nb_runs=0

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

function simple_moving_averager(period) {
    var nums = [];
    return function(num) {
        nums.push(num);
        if (nums.length > period)
            nums.splice(0,1);  // remove the first element of the array
        var sum = 0;
        for (var i in nums)
            sum += nums[i];
        var n = period;
        if (nums.length < period)
            n = nums.length;
        return(sum/n);
    }
}

// moving average class
var m_sma = simple_moving_averager(3);

if (ZwiftPacketMonitor && Cap) {

    console.log('Listening on: ', ip, JSON.stringify(Cap.findDevice(ip),null,4));
    
    // determine network interface associated with external IP address
    interface = Cap.findDevice(ip);

    // ... and setup monitor on that interface:
    const monitor = new ZwiftPacketMonitor(interface)
    
    monitor.on('outgoingPlayerState', (playerState, serverWorldTime) => {
		
        logger.info('New state')
		
        if (playerState.distance > distance_prev + 3 && nb_runs > 4) {
		
			console.log(serverWorldTime, playerState)
            
            var angle = Math.asin((playerState.altitude - altitude_prev) / (200 * (playerState.distance - distance_prev)))
            // calculate moving average
			var slp_pc = 100 * Math.tan(angle)
            var slope_pc = m_sma(slp_pc)
			
			logger.info('Simulating ' + slp_pc + '% incline pckt ' + nb_runs)

			if (Math.abs(slope_pc - slope_prev) > 0.9) {
				try {
                    // convert it
                    var slope = (slope_pc + 200) * 100
                    let ser_msg = '>S' + slope.toFixed(0) + '\n'

					port.write(ser_msg)
					logger.info(ser_msg)
				} catch(e) {
					console.log(e)
				}
				
				slope_prev = slope_pc
			}

            distance_prev = playerState.distance
            altitude_prev = playerState.altitude

            nb_runs = 1;

        } else if (nb_runs == 0 || playerState.distance < distance_prev) {
            // also handle when player changes worlds without restarting the script
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

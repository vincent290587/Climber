
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

const ip = require('internal-ip').v4.sync();

try {
	const SerialPort = require('serialport')
	var port = new SerialPort('COM21', {
		baudRate: 115200
	})
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
		
        console.log(serverWorldTime, playerState)
		
		let ser_msg = '>V' + playerState.speed + 'P' + playerState.power + 'H' + playerState.climbing + '\n'
		port.write(ser_msg)
		
    })
    
    
    monitor.start()
}

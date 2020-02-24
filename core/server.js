const net = require('net')
const packetHandler = require('../network/packet-handler')
const http = require('http')

const port = 8281

const server = net.createServer()

let userList = []

http.createServer((req, res) => {
	res.write(`${userList.length}-1-1-1-1-1-1-1-1-1`)
	res.end()
}).listen(80)

server.listen(port, () => {
	console.log(`Server's listening on port ${port}.`)
})

server.on('connection', (socket) => {
	console.log('A new connection has been established.')

	socket.on('data', (chunk) => {
		packetHandler.segregate(chunk)
		/*chunk = packetSecurity.decrypt(chunk)
		chunk = chunk.subarray(4)
		console.log(chunk)
		chunk = packetSecurity.decrypt(chunk)
		console.log('----- PACKET HEADER -----')
		console.log('Size: ' + chunk.readUInt16LE(0))
		console.log('Key: ' + chunk.readUInt8(2))
		console.log('Checksum: ' + chunk.readUInt8(3))
		console.log('Packet ID: ' + chunk.readUInt16LE(4))
		console.log('Client ID: ' + chunk.readUInt16LE(6))
		console.log('Timestamp: ' + chunk.readUInt32LE(8))
		console.log('----- PACKET DATA -----')
		console.log('Username: ' + chunk.toString('ascii', 12, 28))
		console.log('Password: ' + chunk.toString('ascii', 28, 40));
		console.log('Version: ' + chunk.readUInt32LE(40));
		console.log('Unknown 1: ' + chunk.readUInt32LE(44))
		console.log('Keys: ' + chunk.toString('ascii', 44, 60));
		console.log('Address: ' + chunk.toString('ascii', 60))*/
	})

	socket.on('end', () => {
		console.log('Closing connection with the client.')
	})

	socket.on('error', (err) => {
		console.log(`Error: ${err}`)
	})
})
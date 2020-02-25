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
	})

	socket.on('end', () => {
		console.log('Closing connection with the client.')
	})

	socket.on('error', (err) => {
		console.log(`Error: ${err}`)
	})
})

exports.getServerTime = () => {
	return Date.now() / 1000 | 0
}
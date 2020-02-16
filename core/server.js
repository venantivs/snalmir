const net = require('net')

const port = 8281

const server = net.createServer()

server.listen(port, () => {
	console.log(`Server's listening on port ${port}.`)
})

server.on('connection', (socket) => {
	console.log('A new connection has been established.')

	socket.on('data', (chunk) => {
		console.log(chunk)
	})

	socket.on('end', () => {
		console.log('Closing connection with the client.')
	})

	socket.on('error', (err) => {
		console.log(`Error: ${err}`)
	})
})
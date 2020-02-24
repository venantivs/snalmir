const http = require('http')
const server = require('./server')

exports.createStatusServer = () => {
    http.createServer((req, res) => {
        res.write(`${server.users.length}-1-1-1-1-1-1-1-1-1`)
        res.end()
    }).listen(80)
}
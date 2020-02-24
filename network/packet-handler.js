const packetDef = require('./packets/packet-def')
const packetSecurity = require('./packet-security')

getPacketHeader = (packet) => {
    return {
        size: packet.readUInt16LE(0),
		key: packet.readUInt8(2),
		checksum: packet.readUInt8(3),
		packetId: packet.readUInt16LE(4),
		clientId: packet.readUInt16LE(6),
		timestamp: packet.readUInt32LE(8)
    }
}

packetToObject = (packet, packetHeader, packetObject) => {

    let obj = Object.assign({}, packetObject)
    let index = 12

    obj.packetHeader = packetHeader

    for (var key in packetObject) {
        switch (packetObject[key]) {
            case '@Int8':
                obj[key] = packet.readUInt8(index)
                index++
                break
            case '@UInt16':
                obj[key] = packet.readUInt16LE(index)
                index += 2
                break
            case '@UInt32':
                obj[key] = packet.readUInt32LE(index)
                index += 4
                break
            default:
                if (packetObject[key].includes('@Char[')) {
                    let arrSize = parseInt(packetObject[key].substr(6, packetObject[key].indexOf(']') - 1))
                    obj[key] = packet.toString('ascii', index, index + arrSize).replace(/\0.*$/g,'');
                    index += arrSize
                }
        }
    }

    return obj
}

objectToPacket = (object, packetHeader, packetObject) => {
    for (var key in packetObject) {
        switch (packetObject[key]) {
            
        }
    }
}

exports.segregate = (packet) => {

    if (packet.byteLength == 4)
        return

    if (packet.byteLength == 120)
        packet = packet.subarray(4)

    packet = packetSecurity.decrypt(packet)

    const packetHeader = getPacketHeader(packet)

    switch (packetHeader.packetId) {
        case 0x20D:
            return packetToObject(packet, packetHeader, packetDef.P20D)
    }
}
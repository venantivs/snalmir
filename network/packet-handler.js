const packetDef = require('./packets/packet-def')
const packetSecurity = require('./packet-security')

getPacketHeaderObject = (packet) => {
    return {
        size: packet.readUInt16LE(0),
		key: packet.readUInt8(2),
		checksum: packet.readUInt8(3),
		packetId: packet.readUInt16LE(4),
		clientId: packet.readUInt16LE(6),
		timestamp: packet.readUInt32LE(8)
    }
}

getPacketHeaderBuffer = (object) => {
    let packetHeaderBuffer = Buffer.alloc(12)

    packetHeaderBuffer.writeUInt16LE(object.packetHeader.size, 0)
    packetHeaderBuffer.writeUInt8(object.packetHeader.key, 2)
    packetHeaderBuffer.writeUInt8(object.packetHeader.checksum, 3)
    packetHeaderBuffer.writeUInt16LE(object.packetHeader.packetId, 4)
    packetHeaderBuffer.writeUInt16LE(object.packetHeader.clientId, 6)
    packetHeaderBuffer.writeUInt32LE(object.packetHeader.timestamp, 8)

    return packetHeaderBuffer
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
            case '@Int32':
                obj[key] = packet.readInt32LE(index)
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

objectToPacket = (object, packetObject) => {

    let packetBuffer = Buffer.alloc(object.packetHeader.size)
    let index = 12

    getPacketHeaderBuffer(object).copy(packetBuffer)

    for (var key in packetObject) {
        switch (packetObject[key]) {
            case '@Int8':
                packetBuffer.writeUInt8(object[key], index)
                index++
                break
            case '@UInt16':
                packetBuffer.writeUInt16LE(object[key], index)
                index += 2
                break
            case '@UInt32':
                packetBuffer.writeUInt32LE(object[key], index)
                index += 4
                break
            case '@Int32':
                packetBuffer.writeInt32LE(object[key], index)
                index += 4
                break
            default:
                if (packetObject[key].includes('@Char[')) {
                    let arrSize = parseInt(packetObject[key].substr(6, packetObject[key].indexOf(']') - 1))
                    packetBuffer.write(object[key], index, arrSize, 'ascii');
                    index += arrSize
                }
        }
    }

    return packetBuffer
}

exports.segregate = (packet) => {

    if (packet.byteLength == 4)
        return

    if (packet.byteLength == 120)
        packet = packet.subarray(4)

    packet = packetSecurity.decrypt(packet)

    const packetHeader = getPacketHeaderObject(packet)

    switch (packetHeader.packetId) {
        case 0x20D:
            console.log(packet)
            let object = packetToObject(packet, packetHeader, packetDef.P20D)
            console.log(object)
            console.log(objectToPacket(object, packetDef.P20D))
    }
}
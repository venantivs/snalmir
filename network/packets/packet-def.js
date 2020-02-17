module.exports = class PacketHeader {
    
    static def() {
        return [2, 1, 1, 2, 2, 4]
    }

    constructor(size, key, checksum, packetId, clientId, timeStamp) {
        this.size = size
        this.key = key
        this.checksum = checksum
        this.packetId = packetId
        this.clientId = clientId
        this.timeStamp = timeStamp
    }
}

module.exports = class P20D {
    static def = [12, 10, 2, 12, 56, 4, 20]

    constructor(packetHeader, password, unknown1, username, unknown2, version, unknown3) {
        this.packetHeader = packetHeader
        this.password = password
        this.unknown1 = unknown1
        this.username = username
        this.unknown2 = unknown2
        this.version = version
        this.unknown3 = unknown3
    }
}
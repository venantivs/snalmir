exports.PACKET_HEADER = {
    size: '@UInt16',
    key: '@Int8',
    checksum: '@Int8',
    packetId: '@UInt16',
    clientId: '@UInt16',
    timeStamp: '@UInt32'
}

exports.P20D = {
    packetHeader: '@PACKET_HEADER',
    username: '@Char[16]',
    password: '@Char[12]',
    version: '@UInt32',
    unknown1: '@UInt32',
    keys: '@Char[16]',
    address: '@Char[16]'
}
const mongoose = require('mongooose')

const itemSchema = mongoose.Schema({
    itemId: {
        type: Number,
        min: 0,
        required: true
    },
    effect: [
        {
            index: {
                type: Number,
                min: 0,
                required: true
            },
            value: {
                type: Number,
                min: 0,
                required: true
            }
        },
        {
            index: {
                type: Number,
                min: 0,
                required: true
            },
            value: {
                type: Number,
                min: 0,
                required: true
            }
        },
        {
            index: {
                type: Number,
                min: 0,
                required: true
            },
            value: {
                type: Number,
                min: 0,
                required: true
            }
        }
    ]
})

const profileSchema = mongoose.Schema({
    accountId: {
        type: Number,
        min: 0,
        required: true
    },
    accountName: {
        type: String,
        required: true
    },
    accountPass: {
        type: String,
        required: true
    },
    accountNumberPass: {
        type: String,
        required: true
    },
    realName: {
        type: String,
        required: true
    },
    email: {
        type: String,
        required: true
    },
    telephone: {
        type: String,
        required: true
    },
    address: {
        type: String,
        required: true
    },
    mobName: [
        {
            type: String,
            required: true
        },
        {
            type: String,
            required: true
        },
        {
            type: String,
            required: true
        },
        {
            type: String,
            required: true
        }
    ],
    gameServer: {
        type: Number,
        min: 0,
        required: true
    },
    keys: {
        type: String,
        required: true
    },
    ipAddress: {
        type: String,
        required: true
    },
    lastIp2: {
        type: String,
        required: true
    },
    lastIp3: {
        type: String,
        required: true
    },
    gold: {
        type: Number,
        required: true
    },
    charInfo: {
        type: Number,
        min: 0,
        required: true
    },
    cash: {
        type: Number,
        required: true
    },
    selChar: {
        type: Number,
        min: 0,
        required: true
    },
    cargo: [itemSchema],
    mode: {
        type: Number,
        required: true
    }
})

const positionSchema = mongoose.Schema({
    x: {
        type: Number,
        required: true
    },
    y: {
        type: Number,
        required: true
    }
})

const affectSchema = mongoose.Schema({
    index: {
        type: Number,
        min: 0,
        required: true
    },
    master: {
        type: Number,
        min: 0,
        required: true
    },
    value: {
        type: Number,
        required: true
    },
    time: {
        type: Number,
        required: true
    }
})

const mobSchema = mongoose.Schema({
    name: {
        type: String,
        required: true
    },
    capeInfo: {
        type: Number,
        required: true
    },
    info: {
        merchant: {
            type: Number,
            required: true
        },
        cityId: {
            type: Number,
            required: true
        }
    },
    guildIndex: {
        type: Number,
        min: 0,
        required: true
    },
    classInfo: {
        type: Number,
        min: 0,
        required: true
    },
    affectInfo: {
        slowMov: {
            type: Number,
            min: 0,
            required: true
        },
        drainHP: {
            type: Number,
            min: 0,
            required: true
        },
        visionDrop: {
            type: Number,
            min: 0,
            required: true
        },
        evasion: {
            type: Number,
            min: 0,
            required: true
        },
        snoop: {
            type: Number,
            min: 0,
            required: true
        },
        speedMov: {
            type: Number,
            min: 0,
            required: true
        },
        skillDelay: {
            type: Number,
            min: 0,
            required: true
        },
        resist: {
            type: Number,
            min: 0,
            required: true
        }
    },
    gold: {
        type: Number,
        required: true
    },
    exp: {
        type: Number,
        min: 0,
        required: true
    },
    last: positionSchema,
    bStatus: statusSchema,
    status: statusSchema,
    equip: [itemSchema],
    inventory: [itemSchema],
    learn: {
        type: Number,
        required: true
    },
    pStatus: {
        type: Number,
        min: 0,
        required: true
    },
    pSkill: {
        type: Number,
        min: 0,
        required: true
    },
    pMaster: {
        type: Number,
        min: 0,
        required: true
    },
    critical: {
        type: Number,
        min: 0,
        required: true
    },
    saveMana: {
        type: Number,
        min: 0,
        required: true
    },
    skillBar1: {
        type: String,
        required: true
    },
    guildMemberType: {
        type: Number,
        required: true
    },
    unknown1: {
        type: Number,
        min: 0,
        required: true
    },
    regenHP: {
        type: Number,
        required: true
    },
    regenMP: {
        type: Number,
        required: true
    },
    resist: {
        type: String,
        required: true
    },
    slotIndex: {
        type: Number,
        required: true
    },
    clientIndex: {
        type: Number,
        required: true
    },
    drilling: {
        type: Number,
        min: 0,
        required: true
    },
    skillBar2: {
        type: String,
        required: true
    },
    hold: {
        type: Number,
        required: true
    },
    tab: {
        type: String,
        required: true
    },
    absorption: {
        type: Number,
        min: 0,
        required: true
    },
    timeStamp: {
        type: Number,
        required: true
    },
    attackSpeed: {
        type: Number,
        min: 0,
        required: true
    },
    drainHP: {
        type: Number,
        required: true
    },
    archLevel: {
        type: Number,
        required: true
    },
    celestialLevel: {
        type: Number,
        required: true
    },
    questInfo: {
        type: Number,
        min: 0,
        required: true
    },
    guildName: {
        type: String,
        required: true
    },
    scriptureTime: {
        type: Number,
        required: true
    },
    unknown2: {
        type: String,
        required: true
    },
    current: positionSchema,
    dest: positionSchema,
    gema: positionSchema,
    evasion: {
        type: Number,
        required: true
    },
    sub: {
        type: Number,
        required: true
    },
    classMaster: {
        type: Number,
        required: true
    },
    accountLevel: {
        type: Number,
        min: 0,
        required: true
    },
    affect: [affectSchema],
    magicIncrement: {
        type: Number,
        min: 0,
        required: true
    },
    mobInstance: {
        type: Number,
        required: true
    },
    weaponDamage: {
        type: Number,
        required: true
    },
    unknown3: {
        type: String,
        required: true
    }
})

const subCelestialSchema = mongoose.Schema({
    bStatus: statusSchema,
    face: {
        type: Number,
        min: 0,
        required: true
    },
    classInfo: {
        type: Number,
        min: 0,
        required: true
    },
    classMaster: {
        type: Number,
        min: 0,
        required: true
    },
    exp: {
        type: Number,
        required: true
    },
    pStatus: {
        type: Number,
        required: true
    },
    pMaster: {
        type: Number,
        required: true
    },
    pSkill: {
        type: Number,
        required: true
    },
    learn: {
        type: Number,
        required: true
    },
    hold: {
        type: Number,
        required: true
    },
    sub: {
        type: Number,
        min: 0,
        required: true
    },
    skillBar1: {
        type: String,
        required: true
    },
    skillBar2: {
        type: String,
        required: true
    },
    affect: [affectSchema]
})

const accountSchema = mongoose.Schema({
    profile: profileSchema,
    mobAccount: [mobSchema],
    subCelestial: [subCelestialSchema]
})

exports.saveNewAccount = () => {
    
}
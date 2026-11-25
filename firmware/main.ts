// Change to true for a BLE-only bench test with no Maqueen attached.
const bleOnly = false
// Change to true when the chassis is a Maqueen Lite V5 (MBT0046).
// Leave false for the original Maqueen Lite chassis (ROB0148).
// The V5 chassis moves the line sensors and headlights behind its
// onboard chip (I2C), so those parts are driven differently below.
const chassisV5 = false
const notifyInterval = 30
const vehicleInterval = 30
const sonarInterval = 90

const frame = pins.createBuffer(20)
const motorPacket = pins.createBuffer(3)
const motorAddress = 0x10

function drive(register: number, requestedSpeed: number) {
    const speed = Math.constrain(requestedSpeed, -255, 255)
    motorPacket[0] = register
    motorPacket[1] = speed < 0 ? 1 : 0
    motorPacket[2] = Math.abs(speed)
    pins.i2cWriteBuffer(motorAddress, motorPacket)
}

// Maqueen Lite V5 chassis-chip registers (DFRobot pxt-maqueen, Maqueen_V5).
const v5SysInitRegister = 70
const v5VersionLengthRegister = 50
const v5LineStateRegister = 29
const v5HeadlightLeftRegister = 11
const v5HeadlightRightRegister = 12
const v5HeadlightWhite = 7
const v5HeadlightOff = 8

const registerPacket = pins.createBuffer(2)

function writeRegister(register: number, value: number) {
    registerPacket[0] = register
    registerPacket[1] = value
    pins.i2cWriteBuffer(motorAddress, registerPacket)
}

function initV5Chassis() {
    // The V5 chassis chip needs a system init before it answers motor
    // and sensor requests (Maqueen_V5.I2CInit in DFRobot's library).
    // It is ready once it reports a non-zero firmware version length.
    writeRegister(v5SysInitRegister, 1)
    basic.pause(100)
    for (let waited = 0; waited < 5000; waited += 100) {
        pins.i2cWriteNumber(motorAddress, v5VersionLengthRegister, NumberFormat.Int8LE)
        if (pins.i2cReadNumber(motorAddress, NumberFormat.Int8LE) != 0) {
            return
        }
        basic.pause(100)
    }
}

let lastHeadlightLeft = -1
let lastHeadlightRight = -1

function writeHeadlightsV5(leftOn: number, rightOn: number) {
    // V5 headlights are RGB lamps on the chassis chip. The Scratch
    // blocks only switch them on/off, so on maps to white and off to
    // black. Write only on change so the periodic vehicle loop does
    // not flood the chassis chip with I2C traffic.
    if (leftOn != lastHeadlightLeft) {
        writeRegister(v5HeadlightLeftRegister, leftOn ? v5HeadlightWhite : v5HeadlightOff)
        lastHeadlightLeft = leftOn
    }
    if (rightOn != lastHeadlightRight) {
        writeRegister(v5HeadlightRightRegister, rightOn ? v5HeadlightWhite : v5HeadlightOff)
        lastHeadlightRight = rightOn
    }
}

function readLineStateV5(): number {
    pins.i2cWriteNumber(motorAddress, v5LineStateRegister, NumberFormat.UInt8BE)
    return pins.i2cReadNumber(motorAddress, NumberFormat.UInt8BE)
}

function measureDistance(): number {
    pins.setPull(DigitalPin.P1, PinPullMode.PullNone)
    pins.digitalWritePin(DigitalPin.P1, 0)
    control.waitMicros(2)
    pins.digitalWritePin(DigitalPin.P1, 1)
    control.waitMicros(10)
    pins.digitalWritePin(DigitalPin.P1, 0)
    pins.setPull(DigitalPin.P2, PinPullMode.PullUp)
    const echo = pins.pulseIn(DigitalPin.P2, PulseValue.High, 500 * 42)
    return Math.round(echo / 42)
}

function writeWord(offset: number, value: number) {
    frame[offset] = value & 0xff
    frame[offset + 1] = (value >> 8) & 0xff
}

function run() {
    if (!bleOnly) {
        if (chassisV5) {
            initV5Chassis()
        }
        drive(0x00, 0)
        drive(0x02, 0)
        if (chassisV5) {
            writeHeadlightsV5(0, 0)
        } else {
            pins.digitalWritePin(DigitalPin.P8, 0)
            pins.digitalWritePin(DigitalPin.P12, 0)
        }
        pins.analogWritePin(AnalogPin.P0, 0)
    }
    // Like the legacy firmware, finish any first-run compass calibration before accepting drive commands.
    let heading = input.compassHeading()
    const status = LegacyBle.start()
    if (status != 0) {
        basic.showString("BLE " + status)
        return
    }

    let leftSpeed = 0
    let rightSpeed = 0
    let maqueenFlags = 0
    let distance = bleOnly ? 42 : 0
    control.inBackground(function () {
        while (true) {
            heading = input.compassHeading()
            basic.pause(100)
        }
    })

    if (!bleOnly) {
        control.inBackground(function () {
            while (true) {
                distance = measureDistance()
                basic.pause(sonarInterval)
            }
        })
        control.inBackground(function () {
            while (true) {
                leftSpeed = Math.constrain(LegacyBle.command(0), -255, 255)
                rightSpeed = Math.constrain(LegacyBle.command(1), -255, 255)
                drive(0x00, leftSpeed)
                drive(0x02, rightSpeed)

                const leds = LegacyBle.command(3)
                let leftLine = 0
                let rightLine = 0
                if (chassisV5) {
                    writeHeadlightsV5((leds >> 3) & 1, (leds >> 2) & 1)
                    // V5 line state bits: left = bit 2, right = bit 0,
                    // and a bit is 1 on black. The legacy Scratch values
                    // are the GPIO levels of the original chassis
                    // (0 on black, 1 on white), so invert them here.
                    const lineState = readLineStateV5()
                    leftLine = ((lineState >> 2) & 1) ? 0 : 1
                    rightLine = (lineState & 1) ? 0 : 1
                } else {
                    pins.digitalWritePin(DigitalPin.P8, (leds >> 3) & 1)
                    pins.digitalWritePin(DigitalPin.P12, (leds >> 2) & 1)
                    leftLine = pins.digitalReadPin(DigitalPin.P13)
                    rightLine = pins.digitalReadPin(DigitalPin.P14)
                }
                maqueenFlags = (leds & 0x0c) | (leftLine << 1) | rightLine
                basic.pause(vehicleInterval)
            }
        })
    }

    let kind = 1
    basic.forever(function () {
        frame.fill(0)
        frame[4] = input.buttonIsPressed(Button.A) ? 1 : 0
        frame[5] = input.buttonIsPressed(Button.B) ? 1 : 0
        frame[19] = kind

        if (kind == 1) {
            writeWord(16, heading)
            frame[18] = input.lightLevel()
        } else if (kind == 2) {
            writeWord(10, bleOnly ? LegacyBle.command(0) : leftSpeed)
            writeWord(12, bleOnly ? LegacyBle.command(1) : rightSpeed)
            writeWord(14, distance)
            writeWord(16, bleOnly ? LegacyBle.command(3) : maqueenFlags)
        } else {
            writeWord(10, Math.floor(input.magneticForce(Dimension.Strength)))
            writeWord(12, input.acceleration(Dimension.X))
            writeWord(14, input.acceleration(Dimension.Y))
            writeWord(16, input.acceleration(Dimension.Z))
        }
        LegacyBle.publish(frame)
        kind = kind == 3 ? 1 : kind + 1
        basic.pause(notifyInterval)
    })
}

run()

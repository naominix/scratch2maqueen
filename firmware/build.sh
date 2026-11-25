#!/bin/sh
# Builds the firmware HEX files into dist/:
#   scratch2maqueen-v2.hex            micro:bit V2, original Maqueen Lite chassis
#   scratch2maqueen-v1.hex            micro:bit V1, original Maqueen Lite chassis
#   scratch2maqueen-v2-v5.hex         micro:bit V2, Maqueen Lite V5 chassis (MBT0046)
#   scratch2maqueen-v1-v5.hex         micro:bit V1, Maqueen Lite V5 chassis (MBT0046)
#   scratch2maqueen-v2-ble-check.hex  micro:bit V2, BLE connection check, no chassis
#   scratch2maqueen-v1-ble-check.hex  micro:bit V1, BLE connection check, no chassis
# Each MakeCode build emits both targets (mbcodal = V2, mbdal = V1).
# The variants share one source tree; build.sh flips the compile-time
# flags in main.ts for each build and restores them afterwards.
set -e
cd "$(dirname "$0")"
mkdir -p dist

restore() {
    sed -i 's/^const bleOnly = true$/const bleOnly = false/' main.ts
    sed -i 's/^const chassisV5 = true$/const chassisV5 = false/' main.ts
}
trap restore EXIT

build() {
    npx --yes --package makecode@1.3.6 mkc build
    cp built/mbcodal-binary.hex "dist/$1"
    cp built/mbdal-binary.hex "dist/$2"
    echo "built dist/$1 dist/$2"
}

restore
build scratch2maqueen-v2.hex scratch2maqueen-v1.hex

sed -i 's/^const chassisV5 = false$/const chassisV5 = true/' main.ts
build scratch2maqueen-v2-v5.hex scratch2maqueen-v1-v5.hex

restore
sed -i 's/^const bleOnly = false$/const bleOnly = true/' main.ts
build scratch2maqueen-v2-ble-check.hex scratch2maqueen-v1-ble-check.hex

restore
echo "done"

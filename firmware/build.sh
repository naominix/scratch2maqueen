#!/bin/sh
# Builds the firmware HEX files into dist/:
#   scratch2maqueen-v2.hex          original Maqueen Lite chassis
#   scratch2maqueen-v2-v5.hex       Maqueen Lite V5 chassis (MBT0046)
#   scratch2maqueen-v2-ble-check.hex  BLE connection check, no chassis
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
    echo "built dist/$1"
}

restore
build scratch2maqueen-v2.hex

sed -i 's/^const chassisV5 = false$/const chassisV5 = true/' main.ts
build scratch2maqueen-v2-v5.hex

restore
sed -i 's/^const bleOnly = false$/const bleOnly = true/' main.ts
build scratch2maqueen-v2-ble-check.hex

restore
echo "done"

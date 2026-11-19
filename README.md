# Scratch2Maqueen

With Scratch2Maqueen, you can control [DFRobot Maqueen](https://wiki.dfrobot.com/micro:Maqueen_for_micro:bit_SKU:ROB0148-E(ROB0148))

[Scratch2Maqueen Website](https://champierre.github.io/scratch2maqueen/)
## micro:bit V2 firmware

For the original DFRobot Maqueen chassis with a micro:bit V2, download `scratch2maqueen-v2.hex` from the [firmware release](https://github.com/naominix/scratch2maqueen/releases/tag/firmware-v1.0.0) and copy it to the `MICROBIT` drive over USB. The release also includes a BLE connection-check HEX, firmware source, and build instructions.

micro:bit V2 と従来の Maqueen 車体をお使いの場合は、上記のリリースから車体用 HEX をダウンロードし、USB 接続した micro:bit の `MICROBIT` ドライブへコピーしてください。起動時にコンパスの校正を求められたら、校正を済ませてから Scratch2Maqueen 拡張で接続します。


## Usage

### Setup

1. You need the special hex file to be downloaded to your micro:bit. Open the MakeCode project below, then click Download to install the .hex file.

    &raquo;[MakeCode project:scratch2maqueen_0_0_2](https://makecode.microbit.org/_D6yYw7AxcUV2))

2. Open [https://champierre.github.io/scratch3/](https://champierre.github.io/scratch3/).

3. Open "Choose an Extension" window and select "Scratch2Maqueen".

    <img src="images/extension.png" />

4. Run [Scratch Link](https://scratch.mit.edu/microbit), then connect to the micro:bit set on Maqueen.

5. After you connect to the micro:bit, you need to calibrate the micro:bit compass. ([Calibrating the micro:bit Compass](https://support.microbit.org/support/solutions/articles/19000008874-calibrating-the-micro-bit-compass))

# Scratch2Maqueen

With Scratch2Maqueen, you can control [DFRobot Maqueen](https://wiki.dfrobot.com/micro:Maqueen_for_micro:bit_SKU:ROB0148-E(ROB0148))

[Scratch2Maqueen Website](https://champierre.github.io/scratch2maqueen/)
## micro:bit V2 firmware

For the original DFRobot Maqueen chassis with a micro:bit V2, download `scratch2maqueen-v2.hex` from the [firmware release](https://github.com/naominix/scratch2maqueen/releases/tag/firmware-v1.0.0) and copy it to the `MICROBIT` drive over USB. The release also includes a BLE connection-check HEX, firmware source, and build instructions.

micro:bit V2 と従来の Maqueen 車体をお使いの場合は、上記のリリースから車体用 HEX をダウンロードし、USB 接続した micro:bit の `MICROBIT` ドライブへコピーしてください。起動時にコンパスの校正を求められたら、校正を済ませてから Scratch2Maqueen 拡張で接続します。


### Limitations / 制約事項

| Area / 項目 | Limit / 制約 |
| --- | --- |
| Hardware / 対応機器 | micro:bit V2 and the original DFRobot Maqueen chassis. micro:bit V1 is unsupported. Other chassis, including Maqueen Plus, have not been tested. / micro:bit V2 と従来の DFRobot Maqueen 車体が対象です。micro:bit V1 は非対応で、Maqueen Plus など他の車体は未検証です。 |
| Peripherals / 周辺機器 | Infrared remote control, servo control, and general-purpose digital or analog pin access over BLE are unsupported. / 赤外線リモコン、サーボ制御、BLE 経由での汎用デジタル・アナログピンの読み書きは非対応です。 |
| Motor commands / モーター指令 | While BLE remains connected, the last motor speed stays active until another speed is sent. Send speed 0 to both motors to stop. / BLE 接続中は最後に送った速度を保持します。停止するには左右のモーターに速度 0 を送ってください。 |
| BLE check HEX / 接続確認用 HEX | Returns a fixed distance of 42 cm and does not control the chassis. Use the vehicle HEX for driving and sensor readings. / 距離は固定値 42 cm で、車体は制御しません。走行とセンサー計測には車体用 HEX を使ってください。 |
| Firmware editing / ファームウェアの編集 | Edit the included source and rebuild with MakeCode CLI to change the firmware. The distributed HEX is for flashing. / 配布 HEX は書き込み用です。ファームウェアを変更する場合は同梱ソースを編集し、MakeCode CLI で再ビルドしてください。 |

## Usage

### Setup

1. You need the special hex file to be downloaded to your micro:bit. Open the MakeCode project below, then click Download to install the .hex file.

    &raquo;[MakeCode project:scratch2maqueen_0_0_2](https://makecode.microbit.org/_D6yYw7AxcUV2))

2. Open [https://champierre.github.io/scratch3/](https://champierre.github.io/scratch3/).

3. Open "Choose an Extension" window and select "Scratch2Maqueen".

    <img src="images/extension.png" />

4. Run [Scratch Link](https://scratch.mit.edu/microbit), then connect to the micro:bit set on Maqueen.

5. After you connect to the micro:bit, you need to calibrate the micro:bit compass. ([Calibrating the micro:bit Compass](https://support.microbit.org/support/solutions/articles/19000008874-calibrating-the-micro-bit-compass))

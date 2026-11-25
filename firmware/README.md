# Scratch2Maqueen for micro:bit V1 / V2

Scratch2Maqueen の BLE プロトコルを使い、Maqueen 車体を micro:bit から制御する MakeCode プロジェクトです。micro:bit V1 と V2 の両方に対応し、従来の Maqueen Lite（ROB0148）と Maqueen Lite V5（MBT0046）の両車体に対応します。ソースは共通で、micro:bit と車体の組み合わせごとに HEX を分けてビルドします。

BLE プロトコルは両世代で共通です。V2 向けは CODAL、V1 向けは DAL で同じサービスを実装しています（`legacyble.cpp`）。V1 用の DAL 実装は Scratch More v0.3.2 の方式に沿っています。

## ビルド

このディレクトリで、Node.js と npm を使って実行します。MakeCode 9.0.12 でビルドを確認しました。

```sh
sh build.sh
```

`dist/` に6つの HEX ができます。

| ファイル | 用途 |
| --- | --- |
| `scratch2maqueen-v2.hex` | micro:bit V2、従来の Maqueen Lite 車体用 |
| `scratch2maqueen-v1.hex` | micro:bit V1、従来の Maqueen Lite 車体用 |
| `scratch2maqueen-v2-v5.hex` | micro:bit V2、Maqueen Lite V5 車体用 |
| `scratch2maqueen-v1-v5.hex` | micro:bit V1、Maqueen Lite V5 車体用 |
| `scratch2maqueen-v2-ble-check.hex` | micro:bit V2、車体なしの BLE 接続確認用 |
| `scratch2maqueen-v1-ble-check.hex` | micro:bit V1、車体なしの BLE 接続確認用 |

個別にビルドする場合は `main.ts` のフラグを切り替えてから `npx --yes --package makecode@1.3.6 mkc build` を実行します。1回のビルドで両世代の HEX ができ、`built/mbcodal-binary.hex` が V2 用、`built/mbdal-binary.hex` が V1 用です。書き込む micro:bit に合った方を使ってください（世代の違う HEX は書き込めません）。

- `chassisV5 = false`：従来車体。ラインセンサーは GPIO（P13/P14）、ヘッドライトは GPIO（P8/P12）で制御します。
- `chassisV5 = true`：V5 車体。起動時に車体チップへ SYS_INIT を送り、ラインセンサーと RGB ヘッドライトは車体チップ経由（I2C）で制御します。Scratch の LED ブロックは点灯＝白、消灯＝黒に割り当てます。中央のラインセンサーや色指定などの V5 専用機能はこのファームウェアではまだ扱いません。
- `bleOnly = true`：車体なしの接続確認用。距離は固定値 42 cm を返します。

モーターと超音波は両車体で制御方式が同じため、共通のコードを使います。

## 実機確認

1. 車体をつながずに試す場合は、micro:bit の世代に合った BLE 確認用 HEX（V2 は `scratch2maqueen-v2-ble-check.hex`、V1 は `scratch2maqueen-v1-ble-check.hex`）を書き込みます。起動時にコンパスの校正を求められたら、接続前に完了してください。
2. Scratch Link を起動し、既存の Scratch2Maqueen 拡張から接続します。車体なしの試験では「前方の障害物までの距離」が 42 cm になることを確認します。V1 では初回接続時に OS 側でペアリングを求められる場合があります。そのまま許可してください。
3. 車体を使う場合は、micro:bit と車体に合った HEX を書き込みます。車輪を浮かせて、左右の前進・後退・速度ゼロ、左右 LED、ラインセンサー、距離を確認します。
4. 走行中に BLE を切断し、両輪が停止することを確認します。再接続直後は停止したままで、新しい指令で走行を再開します。接続中は既存 Scratch と同じく速度指令を保持します。

BLE のサービス UUID は `0xF005`、モーターと LED の書き込みは `0xA0`、通知は旧形式の20バイトです。車体と共有する GPIO は BLE 側から読み書きしません。赤外線とサーボは実装範囲外です。

プロトコルの参照元は [Scratch2Maqueen](https://github.com/champierre/scratch2maqueen) と [Scratch More v0.3.2](https://github.com/yokobond/pxt-scratch-more/tree/v0.3.2) です。車体の制御仕様は [DFRobot Maqueen のソース](https://github.com/DFRobot/pxt-maqueen)（従来車体は固定版 2873abf、V5 は `Maqueen_V5` 名前空間）を参照しました。

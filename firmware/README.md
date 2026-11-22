# Scratch2Maqueen for micro:bit V2

Scratch2Maqueen の BLE プロトコルを使い、Maqueen 車体を micro:bit V2 から制御する MakeCode プロジェクトです。従来の Maqueen Lite（ROB0148）と Maqueen Lite V5（MBT0046）の両方に対応します。ソースは共通で、車体ごとに HEX を分けてビルドします。

## ビルド

このディレクトリで、Node.js と npm を使って実行します。MakeCode 9.0.12 でビルドを確認しました。

```sh
sh build.sh
```

`dist/` に3つの HEX ができます。

| ファイル | 用途 |
| --- | --- |
| `scratch2maqueen-v2.hex` | 従来の Maqueen Lite 車体用 |
| `scratch2maqueen-v2-v5.hex` | Maqueen Lite V5 車体用 |
| `scratch2maqueen-v2-ble-check.hex` | 車体なしの BLE 接続確認用 |

個別にビルドする場合は `main.ts` のフラグを切り替えてから `npx --yes --package makecode@1.3.6 mkc build` を実行します（`built/mbcodal-binary.hex` が V2 用です）。

- `chassisV5 = false`：従来車体。ラインセンサーは GPIO（P13/P14）、ヘッドライトは GPIO（P8/P12）で制御します。
- `chassisV5 = true`：V5 車体。起動時に車体チップへ SYS_INIT を送り、ラインセンサーと RGB ヘッドライトは車体チップ経由（I2C）で制御します。Scratch の LED ブロックは点灯＝白、消灯＝黒に割り当てます。中央のラインセンサーや色指定などの V5 専用機能はこのファームウェアではまだ扱いません。
- `bleOnly = true`：車体なしの接続確認用。距離は固定値 42 cm を返します。

モーターと超音波は両車体で制御方式が同じため、共通のコードを使います。

## 実機確認

1. 車体をつながずに試す場合は `scratch2maqueen-v2-ble-check.hex` を micro:bit V2 に書き込みます。起動時にコンパスの校正を求められたら、接続前に完了してください。
2. Scratch Link を起動し、既存の Scratch2Maqueen 拡張から接続します。車体なしの試験では「前方の障害物までの距離」が 42 cm になることを確認します。
3. 車体を使う場合は、車体に合った HEX（従来車体は `scratch2maqueen-v2.hex`、V5 は `scratch2maqueen-v2-v5.hex`）を書き込みます。車輪を浮かせて、左右の前進・後退・速度ゼロ、左右 LED、ラインセンサー、距離を確認します。
4. 走行中に BLE を切断し、両輪が停止することを確認します。再接続直後は停止したままで、新しい指令で走行を再開します。接続中は既存 Scratch と同じく速度指令を保持します。

BLE のサービス UUID は `0xF005`、モーターと LED の書き込みは `0xA0`、通知は旧形式の20バイトです。車体と共有する GPIO は BLE 側から読み書きしません。赤外線とサーボは実装範囲外です。

プロトコルの参照元は [Scratch2Maqueen](https://github.com/champierre/scratch2maqueen) と [Scratch More v0.3.2](https://github.com/yokobond/pxt-scratch-more/tree/v0.3.2) です。車体の制御仕様は [DFRobot Maqueen のソース](https://github.com/DFRobot/pxt-maqueen)（従来車体は固定版 2873abf、V5 は `Maqueen_V5` 名前空間）を参照しました。

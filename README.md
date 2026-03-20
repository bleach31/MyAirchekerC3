# MyAirCheckerC3

「AirChecker C3/USB」の公式ファームウェアの修正版。

メーカー製品情報

https://dragon-torch.tech/cat-components/other/dtp-accu/


## 変更点

- プラットフォームを**PlatformIO** (VS Code 拡張)に変更
- `InitLoRaModule` のレスポンス読み取り不具合

E220 への設定コマンド (11バイト) 送信後、レスポンスの読み取りループが 9600baud のバイト間隔 (~1ms) に対応できず、1バイトしか読めずに終了していました。残り10バイトがバッファに滞留し、後続のネゴシエーションで受信データと誤認され、送信先が不正な値になっていました。

```cpp
// 修正前 (バグ)
while (!SerialLoRa.available()) delayMicroseconds(10);
while (SerialLoRa.available()) { ... }  // 2バイト目到着前に0になり即終了

// 修正後
while (response.size() < command.size() && (millis() - startMs) < 1000) {
    if (SerialLoRa.available()) { ... }
    else delayMicroseconds(100);
}
```

- `LoRaRecvTask` の送信先読み取り位置

`RecieveFrame()` 呼び出し前の古いバッファから送信先アドレス/チャンネルを読んでおり、初回は 0x0000/ch0 が設定されていました。`RecieveFrame()` 完了後に移動しました。

- `Serial.flush()` → `SerialLoRa.flush()`

LoRa 送信後のフラッシュ対象が `Serial` (デバッグ用) になっていたのを `SerialLoRa` (LoRa モジュール用) に修正しました。


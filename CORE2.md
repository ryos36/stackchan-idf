<!--
SPDX-FileCopyrightText: 2026 Kenta IDA <fuga@fugafuga.org>
SPDX-License-Identifier: BSL-1.0
-->

# M5Stack Core2 移植メモ

Core2 (ESP32-D0WDQ6-V3, AXP192) 移植で見つかった、他ボードのコードを
流用する際に踏みやすい落とし穴の記録。

## AXP192 (PMIC) — i2c_dump.cpp の誤発火 (対応済み)

`main/i2c_dump.cpp` は CoreS3 の「LCD バックライトが消える」インシデント
向けの診断・自動復旧ツールで、内部 I2C アドレス `0x34` のレジスタ `0x03`
(AXP2101 の OTP/FW バージョン、常に `0x4A`) を読み、期待値と違えば復旧
処置としてレジスタ `0x90` に `0xBF` を書き込む、というロジックだった。
チップの実体を確認せず「0x34 に何か応答し、レジスタ 0x03 が 0x4A でない」
だけで発火する設計。

Core2 は同じアドレス `0x34` に別の PMIC (AXP192) を積んでおり、レジスタ
`0x03` の意味が異なるため誤って「異常」と判定され、write probe が発火
した。AXP192 のレジスタ `0x90` は GPIO0 の機能選択レジスタで、Core2 では
外部給電 (M-BUS / Port A) の有効/無効を制御しており、書き込まれた
`0xBF` は floating (無効) を意味する — 実機ログで確認済み。

**教訓**: 特定チップ前提で書かれた診断・復旧・ワークアラウンド系コードは、
新ボード追加時に「同じ I2C アドレスに別のチップが載っている」ケースで
誤発火しうる。read-only の診断であれば実害は薄いが、書き込みを伴う復旧
ロジックは要注意。

**対応**: `BoardProfile::has_m5base_i2c_chips` で AXP2101 搭載が確定して
いる M5Base / TakaoBase に呼び出しを限定した (`components/board/board.cpp`
の `profile_for()`、`main/app_main.cpp` の呼び出し箇所)。

## 描画 fps — 実測 11.6fps (許容、対応なし)

ESP32 の SPI DMA は PSRAM を直接読めない (ESP32-S3 の EDMA と異なる) ため、
`avatar::BufferedCanvas::end_frame()` の PSRAM フレームバッファ→パネル転送が
レジスタポーリング転送に落ちる。実機実測 (2026-08-12、320×240、-Os +
PSRAM キャッシュワークアラウンド有効 + PSRAM 40MHz):

- fps = 11.6 (目標 30fps の約39%)
- `end_frame()` 1 回あたり平均 32.9ms、最大 34.2ms

**判断**: 10fps あれば実用上十分と判断し、追加の最適化 (PSRAM キャッシュ
ワークアラウンドの解除、`-O2` への復帰、内部 RAM バンドステージングによる
DMA 転送化) は行わない。将来 fps 要件が上がった場合は、この2点から着手
できる余地を残す。

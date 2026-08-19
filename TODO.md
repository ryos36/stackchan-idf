# TODO

- `main/idf_component.yml` の `espressif/esp-sr` を `2.4.0` に固定している。
  `2.5.0` は新規依存 `espressif/esp-dl` (>=3.3.9) を追加し、これが約673KB
  無条件でリンクされ cores3 の `ota_0`/`ota_1` (各4MB) を約115KBオーバー
  フローさせる。`esp-dl` を必要とせずに `2.5.0` (以降) へ追従できるか
  (esp-sr 側の CMakeLists.txt の条件分岐を確認する、または上流に issue を
  出す) を今後調査する。

- **優先度メモ**: `components/jtts/src/hmm_synth.cpp` の HMM エンジン
  (`hts_engine` ベース) は、formant 合成よりも音質が良い。フラッシュ
  容量に余裕があり HMM ボイスを積めるボード (`CONFIG_JTTS_ENABLE_HMM`
  が有効な S3 系、CoreS3 等) では、formant 合成側のこれ以上の細かい
  調整は優先度が低い。

  Core2 は現行のパーティション表 (`partitions_core2.csv`) に voice 領域を
  割り当てていないため HMM が使えない (変更には USB 再書き込みが必要)。
  AtomS3 の no-PSRAM 構成のような本当の資源制約とは異なる — Core2 の
  フラッシュは 16MB あり、約 7MB が意図的に未割当のまま残っている
  (Core2 移植プランの Phase 0 での設計判断、「会話機能の有効化が具体化
  した時点で改めて表を切る」という再検討の余地を残した上での選択)。
  Core2 の声を良くしたい場合、formant の細かい調整を続けるより、
  パーティション表を切り直して HMM に到達する方が安く済む可能性がある。

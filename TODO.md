# TODO

- `main/idf_component.yml` の `espressif/esp-sr` を `2.4.0` に固定している。
  `2.5.0` は新規依存 `espressif/esp-dl` (>=3.3.9) を追加し、これが約673KB
  無条件でリンクされ cores3 の `ota_0`/`ota_1` (各4MB) を約115KBオーバー
  フローさせる。`esp-dl` を必要とせずに `2.5.0` (以降) へ追従できるか
  (esp-sr 側の CMakeLists.txt の条件分岐を確認する、または上流に issue を
  出す) を今後調査する。

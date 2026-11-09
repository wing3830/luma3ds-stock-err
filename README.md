# Luma3DS エラー処理 純正化 boot.firm

Luma3DS のエラー処理を純正（ストック）に戻したカスタムビルド。Luma自体の起動エラー画面（`error()` 表示）はそのまま残してある。

> これは Luma3DS の派生物です。原プロジェクト：Luma3DS（https://github.com/LumaTeam/Luma3DS ）
> ライセンスは原プロジェクトの GPLv3 を継承します（`LICENSE` は Luma3DS のものを同梱）。
> 公開時は原プロジェクトへのクレジットとライセンス継承が必須です。

## 何をする物か
- ARM11例外画面（`An exception occurred` のレジスタダンプ表示）をやめ、純正カーネルのハンドラに委譲する
- ARM9例外／svcBreak／panic へのパッチ、起動時のダンプ表示を撤去する
- Rosalina の errdisp 再実装を削除し、純正 ErrDisp アプレットの起動ブロックを解除する
- DS/GBA互換（LGY）側も同様に純正化する

適用差分の詳細は `stock-err.patch`（上流ソースへの全差分、9ファイル）と `docs/README_stock_err_original.md` を参照。

## 使い方
SDカード（必要ならCTRNANDも）のルートにある `boot.firm` を、この `boot.firm`（v3）に置き換える。戻すときは純正リリースの `boot.firm` に戻す。

## 技術・成果物
- ベース：Luma3DS master `1441535`、バージョン表記 v13.5-1441535d
- `boot.firm`（v3、約294KB）SHA-256：`956f9726d1c2e66e65c0751c73820013f5fbcd82bbef90936d40701ecfe88c10`
- `crash_test/` — 純正動作を確認するためのクラッシュ再現用テストアプリのソース

## 状態・確認済み事項
- v3は実機で確認済み（2026-11-09）：
  - 正常に起動すること
  - カード抜き時の ErrDisp が純正スタイルで動くこと
  - crash_test の全経路（ARM11データアボート／svcBreak／ERRF throw）が純正スタイルの挙動になること
  - v1/v2で起きていた「Save and exitしても毎回設定メニューが出る」問題が解消したこと（原因はタグなし shallow clone でバージョンが v0.0.0 になっていたこと。v3は履歴＋タグ取得後にビルド）
- ビルド面では、クリーンビルドで同一ハッシュの再現、firmtoolでの構造パース、objdumpでの分岐確認まで済んでいる

## 未確認・注意点
- 同梱の `docs/README_stock_err_original.md` には「実機未検証」と書かれているが、それはv1時点の記述で、v3では上記のとおり実機確認済みに更新されている（原文は記録としてそのまま残した）
- 純正 ErrDisp を戻したため、メモリ使用量の大きい場面では純正機同様の制約（起動失敗等）が出る可能性がある
- クラッシュダンプは出なくなる。それ自体がこのビルドの目的
- `boot.firm` バイナリをリポジトリ本体に入れるか、Release添付にするかは未決定（CHECKLIST参照）。旧版（v1/v2）と無改変比較用ビルドはこの準備フォルダには入れていない

## ライセンス
GPLv3（Luma3DSから継承）。`LICENSE` ファイルを参照。

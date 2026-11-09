# Luma3DS エラー処理 純正化 boot.firm

ベース: Luma3DS master `1441535` (2026-11-09 時点の最新) をソース改変してビルド。
Luma の起動エラー画面 (`Failed to apply FIRM patch` など `error()` の表示) は指示通り**そのまま残してある**。

## 成果物
- `boot.firm` — **v3** SHA-256: `956f9726d1c2e66e65c0751c73820013f5fbcd82bbef90936d40701ecfe88c10` (301,056 bytes)
  旧版: `boot_v1_stock_err.firm` (b399f760…), `boot_v2_menukey.firm` (04b8d289…)

## 【重要】v1/v2 で「再起動のたびに設定メニューが出る」バグと v3 での修正
v1・v2 にはビルド手順由来のバグがあった。ソースを shallow clone (タグなし) で取得して
ビルドしたため、`git describe` が失敗しバージョンが **v0.0.0** になっていた。
Luma は MCU 内の設定データにバージョンを持ち、`readConfigMcu()` が
「保存バージョン < 10.3.0 なら無効」として弾くため、0.0.0 のビルドでは
**Save and exit を何回やっても毎回設定メニューが開く**状態になっていた
(メニューのタイトルにも `Luma3DS v0.0.0-0 configuration` と出ていた)。
- v3 は履歴+タグを取得してからビルドし直し、バージョンは正しく **v13.5-1441535d**。
- v2 で入れた起動キーの入れ替え (MENUKEY) は不要だったので全て標準に戻した
  (設定メニュー = SELECT+電源、chainloader = START+電源)。
  v3 の差分は v1 と同じ「純正化 9 ファイル」のみ。
- v3 の初回起動時だけ、旧バージョンで書かれた MCU 設定が無効なため
  メニューが 1 回出る。そこで Save and exit すれば、次回からは出なくなる。
- `stock-err.patch` — 上流ソースへの全差分 (9ファイル)
- `boot_baseline_unmodified.firm` — 比較用の無改変ビルド (SHA-256: `5ae5520164a55920a403df8e4ea99ba9fdfa06e31385eec22d621b7869277ba9`)

## 何を純正に戻したか

### 1. ARM11 例外 (「An exception occurred」画面)
- `k11_extension`: `isExceptionFatal()` を常時 false に固定。config の
  `disable_arm11_exception_handlers` の値に関係なく、全例外が純正カーネルの
  ハンドラへ委譲される (逆アセンブルでも `mov r0,#0; bx lr` のみになっていることを確認済み)。
  → アプリのクラッシュ時は黒画面 / HOMEメニューの汎用エラーなど純正の挙動になる。
- `patchKernel11`: Kernel11 の kernelpanic を bkpt に書き換えるパッチを撤去 (純正 panic のまま)。
- 同じく `enableUserExceptionHandlersForCPUExc` の強制有効化 (ERR:F形式レジスタダンプ) を撤去。
  小売り機の純正では NS が開発機以外では有効化しないもの。

### 2. ARM9 例外 (Process9 / Kernel9)
- `patchNativeFirm` / `patch1x2xNativeAndSafeFirm` / `patchPrototypeNative` から
  `patchArm9ExceptionHandlersInstall` / `patchSvcBreak9` / `patchKernel9Panic` の呼び出しを全て撤去。
  → Kernel9 は自前の純正ハンドラをそのまま設置し、svcBreak も panic も純正のまま。
- Luma 本体 (arm9) 側の `installArm9Handlers()` と、起動時にダンプを表示する
  `detectAndProcessExceptionDumps()` の呼び出しを撤去。
  → 起動時に「An exception occurred」画面が出る経路自体が消えた
  (成果物バイナリから同文字列が消えていることを無改変ビルドと比較して確認済み)。

### 3. ErrDisp (「An error occurred (ErrDisp)」画面)
- Rosalina の err:f 再実装 (`errdisp.c` / `errdisp.h`) を**ファイルごと削除**し、
  スレッドも起動しないようにした。`err:f` ポートは Rosalina が作らなくなる。
- PM の `isTitleLaunchPrevented()` を常時 false にし、純正 ErrDisp アプレット
  (0004003000008A02 / SAFE_MODE 用 0004003020008A03) の起動ブロックを解除。
  → 致命的エラー時は純正 ErrDisp が起動して純正のエラー画面を出す。
- K11 extension の `ERRF__DumpException` パッチを撤去。
  小売り機の純正カーネルはユーザーランド例外を err:f に通知しない、その挙動に戻る。

### 4. DS / GBA 互換 (LGY) 側も同様に
- `patchLgyK11`: ユーザー例外処理を有効化する pointer-to-bool パッチと、
  kernelpanic の開発機チェック回避 (LCDフィル常時化) パッチを撤去。

## 残したもの (意図的)
- Luma の起動エラー (`error()`) 表示 — ユーザー指示により残置。
- Rosalina メニュー、GDB スタブ、プラグイン、チート等、エラー表示と無関係な機能は無改変。
  ただし GDB でデバッグ中のプロセスの例外は従来どおりデバッガ側で扱われる。
- qtm エラー回避用の err:f Throw フック (SAFE_FIRM Rosalina 有効時の特定エラーコードのみ) は
  エラー画面ではないので残置。

## 使い方
SD カード (必要なら CTRNAND も) のルートの `boot.firm` をこのファイルに置き換えるだけ。
`config.ini` の `disable_arm11_exception_handlers` は無効化済みなので何を設定しても挙動は変わらない。
戻すときは純正リリースの boot.firm に戻せばよい。

## 未検証・注意点
- **実機未検証。** ビルド成功・クリーンビルドで同一ハッシュ再現・FIRM構造の解析・
  逆アセンブルでの分岐確認までは済んでいるが、実機での起動/クラッシュ挙動は未確認。
  初回は SD の boot.firm のみ差し替えて (CTRNAND は純正のまま) テストし、
  起動しなくなっても B9S + CTRNAND 側で復旧できる状態で試すこと。
- ErrDisp は元々メモリ逼迫を理由に Luma が起動を止めていたもの。純正 ErrDisp を
  戻したため、メモリ使用量の大きいゲーム + アプレット同時起動の場面では
  純正機同様の制約 (起動失敗等) が出る可能性がある。
- homebrew (libctru) のアプリ内例外ダンプ表示は純正小売り挙動に合わせて出なくなる。
  クラッシュしたアプリは HOME の汎用エラー / 黒画面で終わるのが純正仕様。
- クラッシュダンプ (`crash_dump_*.bin` の保存確認画面や `luma/errdisp.txt`) も当然出なくなる。
  トラブル時の原因調査は困難になる。それがこのビルドの目的そのもの。

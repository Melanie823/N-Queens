# N-Queens 問題 — 多種無資訊搜尋方法比較

人工智慧課程作業：以 C 語言語法（`.cpp` 以便 g++ / Visual Studio 編譯）實作 N 皇后問題，並比較多種無資訊搜尋（Uninformed Search）方法。

## 問題說明
在 N×N 的棋盤上放 N 個皇后，使任何兩個皇后都不互相攻擊（同行、同列、對角線皆不可）。

## 實作的演算法（找第一個解）
| 方法 | 說明 |
|---|---|
| Backtracking DFS | DFS + 提前剪枝，效率最高 |
| BFS | 廣度優先，逐層展開 |
| DFS（無剪枝） | 深度優先，放滿後才判斷是否合法 |
| IDDFS | 逐漸加深深度限制的 DFS |

程式會輸出第一組解的棋盤圖示、每列皇后的位置，以及各方法的執行時間（毫秒），並可對不同 N 做比較。

## 檔案
- `n_queen/n_queen/n_queens.cpp` — 主程式
- `n_queen/n_queen.sln` — Visual Studio 方案
- `n_queens多種方法使用整理.pptx` — 各方法整理與比較簡報

## 編譯與執行
```bash
g++ -O2 -o n_queens n_queen/n_queen/n_queens.cpp
./n_queens
```
或以 Visual Studio 開啟 `n_queen/n_queen.sln` 執行。

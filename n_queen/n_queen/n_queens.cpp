/*
 * ============================================================
 * N-Queens Problem Solver — Uninformed Search Methods
 * 用 C 語言語法撰寫（副檔名 .cpp 以便 g++ 編譯）
 * ============================================================
 *
 * 【問題說明】
 *   在 N×N 的棋盤上放 N 個皇后，使任何兩個皇后都不互相攻擊。
 *   皇后可以攻擊同一行、同一列、以及對角線方向的所有格子。
 *
 * 【支援的四種演算法】（只找「第一個解」）
 *   1. Backtracking DFS — DFS + 提前剪枝，效率最高
 *   2. BFS             — 廣度優先，逐層展開
 *   3. DFS 無剪枝      — 深度優先，不提前判斷
 *   4. IDDFS           — 逐漸加深的 DFS
 *
 * 【輸出內容】
 *   - 第一組解的棋盤圖示
 *   - 解的路徑（每列皇后放在哪一欄）
 *   - 執行時間（毫秒）
 *
 * 【編譯與執行】
 *   g++ -O2 -o n_queens n_queens.cpp
 *   ./n_queens
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

 /* ─────────────────────────────────────────────────────────────
  * 常數設定
  * ───────────────────────────────────────────────────────────── */
#define MAX_N      20          /* 程式最大支援的棋盤大小 */
#define NODE_SIZE  (MAX_N + 1) /* BFS/DFS 每個節點佔用的整數數量
                                  前 MAX_N 個存各列皇后的欄位
                                  最後第 MAX_N 個存目前放到第幾列（深度） */
#define MAX_QUEUE  8000000     /* BFS/DFS 最多可以存幾個節點
                                  超過這個數量程式會自動停止 */

                                  /* ─────────────────────────────────────────────────────────────
                                   * 工具函式：印出分隔線
                                   * ───────────────────────────────────────────────────────────── */
void print_sep(char c, int len) {
    int i;
    for (i = 0; i < len; i++) putchar(c);
    putchar('\n');
}

/* ─────────────────────────────────────────────────────────────
 * 工具函式：印出棋盤圖示 + 路徑
 *   q — queens 陣列，q[i] = 第 i 列的皇后放在第幾欄（從 0 開始）
 *   n — 棋盤大小
 *
 *   範例輸出（n=4，解 [1,3,0,2]）：
 *     欄  1 2 3 4
 *   列 1: . Q . .
 *   列 2: . . . Q
 *   列 3: Q . . .
 *   列 4: . . Q .
 *
 *   路徑：[1, 3, 0, 2]
 *   意思：第1列放欄2、第2列放欄4、第3列放欄1、第4列放欄3
 * ───────────────────────────────────────────────────────────── */
void print_board(int* q, int n) {
    int i, j;

    /* 印欄位標號 */
    printf("\n  欄 ");
    for (j = 0; j < n; j++) printf("%d ", j + 1);
    printf("\n");

    /* 印每一列 */
    for (i = 0; i < n; i++) {
        printf("列%2d: ", i + 1);
        for (j = 0; j < n; j++)
            printf("%s", q[i] == j ? "Q " : ". ");
        printf("\n");
    }

    /* 印路徑（欄位從 1 開始計，方便人閱讀） */
    printf("\n  路徑（每列皇后的欄位）: [ ");
    for (i = 0; i < n; i++) printf("%d ", q[i] + 1);
    printf("]\n");
}

/* ─────────────────────────────────────────────────────────────
 * 工具函式：取絕對值
 * ───────────────────────────────────────────────────────────── */
int my_abs(int x) {
    return x < 0 ? -x : x;
}

/* ─────────────────────────────────────────────────────────────
 * 核心函式：衝突判斷 is_safe()
 *
 *   問題：把皇后放在「第 row 列、第 col 欄」是否安全？
 *   做法：檢查前面 0 ~ row-1 列已放好的皇后有沒有衝突。
 *
 *   衝突有兩種情況：
 *     1. 同欄：q[r] == col
 *     2. 對角線：|q[r] - col| == |r - row|
 *        （主對角線和副對角線都用同一個條件涵蓋）
 *
 *   回傳：1 = 安全（可以放）、0 = 衝突（不能放）
 * ───────────────────────────────────────────────────────────── */
int is_safe(int* q, int row, int col) {
    int r, c;
    for (r = 0; r < row; r++) {
        c = q[r];
        if (c == col || my_abs(c - col) == my_abs(r - row))
            return 0;  /* 衝突 */
    }
    return 1;  /* 安全 */
}

/* ─────────────────────────────────────────────────────────────
 * 結果結構：只保留「解」和「時間」
 * ───────────────────────────────────────────────────────────── */
typedef struct {
    int    found;           /* 1 = 找到解；0 = 無解；-1 = 跳過（n 太大）*/
    int    solution[MAX_N]; /* 第一組解的皇后位置 */
    double ms;              /* 執行時間（毫秒） */
} Result;

/* ═══════════════════════════════════════════════════════════════
 * 方法 1：Backtracking DFS（含剪枝）
 *
 * 【運作方式】
 *   遞迴地逐列嘗試放皇后。
 *   每次嘗試前先呼叫 is_safe()，衝突就直接跳過，
 *   不往下展開，整棵子樹省略，這就是「剪枝」。
 *
 * 【流程】
 *   row=0 → 嘗試 col=0,1,...,N-1
 *     → 安全就放下，遞迴到 row=1
 *     → row=1 再嘗試 col=0,1,...,N-1
 *     → ...
 *     → row=N → 所有列放好 → 找到解！
 *     → 不安全 or 下層無解 → 回溯，換下一個 col
 * ═══════════════════════════════════════════════════════════════ */
static int g_bt_found;
static int g_bt_sol[MAX_N];

void bt_recurse(int* q, int n, int row) {
    int col;

    if (g_bt_found) return;  /* 已找到解，整棵樹立刻停止 */

    /* 所有列都放好 → 找到第一組解 */
    if (row == n) {
        memcpy(g_bt_sol, q, n * sizeof(int));
        g_bt_found = 1;
        return;
    }

    /* 嘗試在第 row 列的每個欄位放皇后 */
    for (col = 0; col < n; col++) {

        /* ★ 剪枝：不安全就跳過這欄，連遞迴都不呼叫
           這欄以下的整棵子樹都被省略掉了               */
        if (is_safe(q, row, col)) {
            q[row] = col;               /* 放置皇后 */
            bt_recurse(q, n, row + 1);  /* 遞迴下一列 */
            q[row] = -1;                /* 回溯：移除皇后，嘗試下一欄 */
        }
    }
}

Result run_backtracking(int n) {
    Result  r;
    clock_t t1, t2;
    int     q[MAX_N];
    int     i;

    memset(&r, 0, sizeof(r));
    g_bt_found = 0;
    for (i = 0; i < n; i++) q[i] = -1;  /* 初始化：全部「未放置」*/

    t1 = clock();
    bt_recurse(q, n, 0);  /* 從第 0 列開始遞迴 */
    t2 = clock();

    r.found = g_bt_found;
    r.ms = (double)(t2 - t1) / CLOCKS_PER_SEC * 1000.0;
    if (g_bt_found) memcpy(r.solution, g_bt_sol, n * sizeof(int));
    return r;
}

/* ═══════════════════════════════════════════════════════════════
 * 方法 2：BFS（廣度優先搜尋）
 *
 * 【運作方式】
 *   用 FIFO Queue 逐層展開搜尋樹。
 *   先把空棋盤放進 Queue，不斷取出節點、展開下一層、
 *   把合法子節點放進 Queue 尾部，直到找到完整解。
 *
 * 【節點格式】
 *   int[MAX_N + 1]
 *   前 n 位：各列皇后的欄位（-1 = 還沒放）
 *   最後一位（[MAX_N]）：目前深度（放了幾個皇后）
 *
 * 【限制】
 *   n > 13 時 Queue 記憶體會超出限制，自動跳過。
 * ═══════════════════════════════════════════════════════════════ */
Result run_bfs(int n) {
    Result    r;
    clock_t   t1, t2;
    int* Q;         /* Queue 底層陣列（動態配置） */
    long long head, tail;
    int* cur, * nxt;
    int       depth, col;

    memset(&r, 0, sizeof(r));

    if (n > 13) {
        printf("  [BFS 跳過：n > 13 時 Queue 記憶體過大]\n");
        r.found = -1;
        return r;
    }

    Q = (int*)malloc((long long)MAX_QUEUE * NODE_SIZE * sizeof(int));
    if (!Q) { printf("  [BFS: 記憶體配置失敗]\n"); r.found = -1; return r; }

    head = tail = 0;

    /* 初始節點：空棋盤，深度 0 */
    cur = Q + tail * NODE_SIZE;
    memset(cur, -1, MAX_N * sizeof(int));
    cur[MAX_N] = 0;
    tail++;

    t1 = clock();

    /* BFS 主迴圈：Queue 不空就繼續 */
    while (head < tail) {

        /* 從 Queue 頭部取出節點（FIFO：先進先出） */
        cur = Q + head * NODE_SIZE;
        depth = cur[MAX_N];  /* 目前深度 = 放了幾個皇后 */
        head++;

        /* Goal Test：深度 = n 代表 N 列都放好 → 找到解 */
        if (depth == n) {
            memcpy(r.solution, cur, n * sizeof(int));
            r.found = 1;
            break;
        }

        /* Queue 快滿了就停止 */
        if (tail >= MAX_QUEUE - n) {
            printf("  [BFS: Queue 已滿，提早停止]\n");
            break;
        }

        /* 展開：嘗試在第 depth 列的每個欄位放皇后
           只把通過 is_safe() 的子節點加入 Queue */
        for (col = 0; col < n; col++) {
            if (is_safe(cur, depth, col)) {
                nxt = Q + tail * NODE_SIZE;
                memcpy(nxt, cur, NODE_SIZE * sizeof(int));
                nxt[depth] = col;       /* 放皇后 */
                nxt[MAX_N] = depth + 1; /* 深度 +1 */
                tail++;
            }
        }
    }

    t2 = clock();
    r.ms = (double)(t2 - t1) / CLOCKS_PER_SEC * 1000.0;

    free(Q);
    return r;
}

/* ═══════════════════════════════════════════════════════════════
 * 方法 3：DFS（無剪枝，Stack 版）
 *
 * 【運作方式】
 *   用 LIFO Stack 深度優先展開。
 *   【刻意不呼叫 is_safe()】，把所有子節點都推進 Stack，
 *   等到深度 = N 才做 goal test（到底才判斷合不合法）。
 *
 * 【和 Backtracking 的差別】
 *   Backtracking：放之前用 is_safe() 篩掉不合法路徑（有剪枝）
 *   DFS 無剪枝 ：走到底才判斷，所有分支都要走（無剪枝）
 *
 * 【限制】
 *   n > 10 時節點數過億，自動跳過。
 * ═══════════════════════════════════════════════════════════════ */

 /* goal test：完整檢查一組配置是否合法（兩兩比對所有皇后） */
int is_valid_solution(int* q, int n) {
    int r, r2;
    for (r = 0; r < n; r++) {
        for (r2 = r + 1; r2 < n; r2++) {
            if (q[r] == q[r2] ||
                my_abs(q[r] - q[r2]) == my_abs(r - r2))
                return 0;  /* 有衝突 */
        }
    }
    return 1;  /* 合法 */
}

Result run_dfs(int n) {
    Result    r;
    clock_t   t1, t2;
    int* S;   /* Stack 底層陣列（動態配置） */
    long long top;
    int* cur, * nxt;
    int       depth, col;

    memset(&r, 0, sizeof(r));

    if (n > 10) {
        printf("  [DFS 無剪枝跳過：n > 10 時節點數極大]\n");
        r.found = -1;
        return r;
    }

    S = (int*)malloc((long long)MAX_QUEUE * NODE_SIZE * sizeof(int));
    if (!S) { printf("  [DFS: 記憶體配置失敗]\n"); r.found = -1; return r; }

    top = 0;

    /* 初始節點：空棋盤，深度 0 */
    cur = S + top * NODE_SIZE;
    memset(cur, -1, MAX_N * sizeof(int));
    cur[MAX_N] = 0;
    top++;

    t1 = clock();

    /* DFS 主迴圈：Stack 不空就繼續 */
    while (top > 0) {

        /* 從 Stack 頂端取出節點（LIFO：後進先出） */
        top--;
        cur = S + top * NODE_SIZE;
        depth = cur[MAX_N];

        /* 深度 = n 才做 goal test（到底了才判斷合不合法） */
        if (depth == n) {
            if (is_valid_solution(cur, n)) {
                memcpy(r.solution, cur, n * sizeof(int));
                r.found = 1;
                break;
            }
            continue;  /* 這個配置不合法，繼續 */
        }

        if (top + n >= MAX_QUEUE) {
            printf("  [DFS: Stack 已滿，提早停止]\n");
            break;
        }

        /* 展開：把所有 N 個子節點推入 Stack
           ★ 這裡不呼叫 is_safe()，不管合不合法全部推入
           逆序推入讓 col=0 在頂端先被取出                */
        for (col = n - 1; col >= 0; col--) {
            nxt = S + top * NODE_SIZE;
            memcpy(nxt, cur, NODE_SIZE * sizeof(int));
            nxt[depth] = col;
            nxt[MAX_N] = depth + 1;
            top++;
        }
    }

    t2 = clock();
    r.ms = (double)(t2 - t1) / CLOCKS_PER_SEC * 1000.0;

    free(S);
    return r;
}

/* ═══════════════════════════════════════════════════════════════
 * 方法 4：IDDFS（Iterative Deepening DFS，迭代加深搜尋）
 *
 * 【運作方式】
 *   把深度限制 l 從 0 開始，每輪 +1，每輪都從空棋盤重新搜尋。
 *   每輪中深度超過 l 就截斷（不往下走）。
 *   直到某輪找到解為止（對 N-Queens 就是 l = N 那輪）。
 *
 * 【優點】
 *   - 記憶體只需 O(N)，和 Backtracking 一樣省
 *   - Complete（保證找得到解）
 *   - 每輪重新搜尋雖然看起來浪費，整體時間複雜度仍是 O(b^d)
 *
 * 【節點數比 Backtracking 多的原因】
 *   前幾輪（l < N）會搜尋淺層節點但找不到解，
 *   這些重複搜尋的節點數加起來比 Backtracking 多。
 * ═══════════════════════════════════════════════════════════════ */
static int g_iddfs_found;
static int g_iddfs_sol[MAX_N];
static int g_iddfs_limit;  /* 這一輪的深度上限 */

/*
 * iddfs_dls() — DLS（Depth-Limited Search）核心
 *   和 bt_recurse() 幾乎一樣，多了「超過深度限制就截斷」
 */
void iddfs_dls(int* q, int n, int row) {
    int col;

    if (g_iddfs_found) return;

    if (row == n) {
        memcpy(g_iddfs_sol, q, n * sizeof(int));
        g_iddfs_found = 1;
        return;
    }

    /* ★ IDDFS 獨有：超過這輪的深度限制就截斷，不往下展開 */
    if (row >= g_iddfs_limit) return;

    for (col = 0; col < n; col++) {
        if (is_safe(q, row, col)) {
            q[row] = col;
            iddfs_dls(q, n, row + 1);
            q[row] = -1;  /* 回溯 */
        }
    }
}

Result run_iddfs(int n) {
    Result  r;
    clock_t t1, t2;
    int     q[MAX_N];
    int     i, limit;

    memset(&r, 0, sizeof(r));
    g_iddfs_found = 0;

    t1 = clock();

    /* 外層迴圈：深度限制 l = 0, 1, 2, ..., n */
    for (limit = 0; limit <= n; limit++) {
        g_iddfs_limit = limit;  /* 設定這輪的深度上限 */
        g_iddfs_found = 0;
        for (i = 0; i < n; i++) q[i] = -1;  /* 每輪從空棋盤重新開始 */

        iddfs_dls(q, n, 0);

        if (g_iddfs_found) break;  /* 找到解就結束 */
    }

    t2 = clock();

    r.found = g_iddfs_found;
    r.ms = (double)(t2 - t1) / CLOCKS_PER_SEC * 1000.0;
    if (g_iddfs_found) memcpy(r.solution, g_iddfs_sol, n * sizeof(int));
    return r;
}

/* ─────────────────────────────────────────────────────────────
 * 工具函式：印出結果（只顯示解和時間）
 * ───────────────────────────────────────────────────────────── */
void print_result(const char* name, int n, Result* res) {
    print_sep('=', 52);
    printf("[ %s ]  n = %d\n", name, n);
    print_sep('-', 52);

    if (res->found == -1) {
        printf("  跳過（n 超過此演算法的限制）\n");
        return;
    }
    if (!res->found) {
        printf("  未找到解\n");
        return;
    }

    /* 只顯示兩件事：棋盤圖示（含路徑）和執行時間 */
    print_board(res->solution, n);
    printf("\n  執行時間：%.4f ms\n", res->ms);
}

/* ─────────────────────────────────────────────────────────────
 * 四種演算法時間比較表
 * ───────────────────────────────────────────────────────────── */
void run_comparison(int n_min, int n_max) {
    int    n;
    Result rbt, rbfs, rdfs, rids;

    print_sep('=', 65);
    printf("四種演算法時間比較（找第一解）  n = %d ~ %d\n", n_min, n_max);
    print_sep('=', 65);
    printf("%-4s  %-14s %-14s %-14s %-14s\n",
        "n", "Backtracking", "BFS", "DFS(無剪枝)", "IDDFS");
    printf("%-4s  %-14s %-14s %-14s %-14s\n",
        "", "(ms)", "(ms)", "(ms)", "(ms)");
    print_sep('-', 65);

    for (n = n_min; n <= n_max; n++) {
        rbt = run_backtracking(n);
        rbfs = run_bfs(n);
        rdfs = run_dfs(n);
        rids = run_iddfs(n);

        /* 跳過的演算法顯示 "N/A" */
        char bfs_t[16], dfs_t[16];
        if (rbfs.found == -1) strcpy(bfs_t, "N/A");
        else sprintf(bfs_t, "%.4f", rbfs.ms);

        if (rdfs.found == -1) strcpy(dfs_t, "N/A");
        else sprintf(dfs_t, "%.4f", rdfs.ms);

        printf("n=%-2d  %-14.4f %-14s %-14s %-14.4f\n",
            n, rbt.ms, bfs_t, dfs_t, rids.ms);
    }
    print_sep('=', 65);
}

/* ─────────────────────────────────────────────────────────────
 * 主程式：互動式選單
 * ───────────────────────────────────────────────────────────── */
int main(void) {
    int    choice, n, n_min, n_max;
    Result res;

    printf("\n");
    print_sep('*', 52);
    printf("   N-Queens Solver\n");
    printf("   Backtracking | BFS | DFS | IDDFS\n");
    print_sep('*', 52);

    while (1) {
        printf("\n選擇模式：\n");
        printf("  1. Backtracking DFS  （含剪枝）\n");
        printf("  2. BFS               （n <= 13）\n");
        printf("  3. DFS 無剪枝        （n <= 10）\n");
        printf("  4. IDDFS\n");
        printf("  5. 四種方法時間比較表\n");
        printf("  0. 離開\n");
        printf("輸入選項：");
        scanf("%d", &choice);

        if (choice == 0) break;

        if (choice >= 1 && choice <= 4) {
            printf("輸入 n（1 ~ 20）：");
            scanf("%d", &n);
            if (n < 1 || n > 20) {
                printf("[錯誤] n 需在 1 ~ 20 之間\n");
                continue;
            }
            if (choice == 1) { res = run_backtracking(n); print_result("Backtracking DFS", n, &res); }
            else if (choice == 2) { res = run_bfs(n);           print_result("BFS", n, &res); }
            else if (choice == 3) { res = run_dfs(n);           print_result("DFS 無剪枝", n, &res); }
            else { res = run_iddfs(n);         print_result("IDDFS", n, &res); }

        }
        else if (choice == 5) {
            printf("輸入 n 範圍（例：1 10）：");
            scanf("%d %d", &n_min, &n_max);
            if (n_min < 1)     n_min = 1;
            if (n_max > 20)    n_max = 20;
            if (n_min > n_max) n_min = n_max;
            run_comparison(n_min, n_max);

        }
        else {
            printf("無效選項\n");
        }
    }

    printf("\n程式結束。再見！\n");
    return 0;
}
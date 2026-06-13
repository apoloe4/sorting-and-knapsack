/*
 * 算法设计与分析 - 0-1背包问题实验
 * 四种算法：蛮力法 / 动态规划法 / 贪心法 / 回溯法
 *
 * 编译：gcc -O2 -o knapsack knapsack.c -lm
 * 用法：knapsack <算法> <物品数n> <背包容量C> [seed]
 *   算法: brute | dp | greedy | backtrack
 *
 * 说明：
 *  - 蛮力法只适合 n <= 30（否则 2^n 天文数字）
 *  - DP使用滚动数组，内存 O(C)
 *  - 价值为 100~1000 之间的随机浮点数（保留两位小数）
 *  - 重量为 1~100 之间的随机整数
 *  - 计时不含数据生成时间
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
static double get_time_ms(void) {
    LARGE_INTEGER freq, cnt;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cnt);
    return (double)cnt.QuadPart / (double)freq.QuadPart * 1000.0;
}
#else
#include <sys/time.h>
static double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}
#endif

/* =========================================================
 * 数据结构
 * ========================================================= */
typedef struct {
    int    id;
    int    weight;   /* 1~100 整数 */
    double value;    /* 100.00~1000.00，两位小数 */
} Item;

/* =========================================================
 * 随机数据生成
 * ========================================================= */
void generate_items(Item *items, int n, unsigned seed) {
    srand(seed);
    for (int i = 0; i < n; i++) {
        items[i].id     = i + 1;
        items[i].weight = rand() % 100 + 1;           /* 1~100 */
        int cents       = rand() % 90001 + 10000;      /* 10000~100000 分 -> 100.00~1000.00 */
        items[i].value  = cents / 100.0;
    }
}

/* =========================================================
 * 辅助：打印结果（只在 n <= 1000 时输出物品列表避免屏幕刷爆）
 * ========================================================= */
void print_result(Item *items, int *selected, int n, double total_val, int total_wt, double elapsed_ms,
                  const char *algo_name, int capacity) {
    printf("\n===== 算法：%s  n=%d  C=%d =====\n", algo_name, n, capacity);
    printf("执行时间: %.3f ms\n", elapsed_ms);
    printf("总重量: %d / %d\n", total_wt, capacity);
    printf("总价值: %.2f\n", total_val);
    int cnt = 0;
    for (int i = 0; i < n; i++) if (selected[i]) cnt++;
    printf("选中物品数: %d\n", cnt);
    if (n <= 1000) {
        printf("物品编号  重量  价值\n");
        for (int i = 0; i < n; i++) {
            if (selected[i])
                printf("  %-8d  %-6d  %.2f\n", items[i].id, items[i].weight, items[i].value);
        }
    }
}

/* =========================================================
 * 蛮力法（只适合 n <= 30）
 * ========================================================= */
void brute_force(Item *items, int n, long long capacity) {
    if (n > 30) {
        printf("[蛮力法] n=%d 超过30，跳过（2^n 过大）\n", n);
        return;
    }
    double t0 = get_time_ms();

    int    *best = (int *)calloc(n, sizeof(int));
    int    *cur  = (int *)calloc(n, sizeof(int));
    double  best_val = 0.0;

    long long total = 1LL << n;
    for (long long mask = 0; mask < total; mask++) {
        long long  wt  = 0;
        double     val = 0.0;
        for (int j = 0; j < n; j++) {
            if (mask >> j & 1) {
                wt  += items[j].weight;
                val += items[j].value;
            }
        }
        if (wt <= capacity && val > best_val) {
            best_val = val;
            for (int j = 0; j < n; j++) cur[j] = (mask >> j & 1) ? 1 : 0;
            memcpy(best, cur, n * sizeof(int));
        }
    }

    double elapsed = get_time_ms() - t0;

    int tot_wt = 0;
    for (int i = 0; i < n; i++) if (best[i]) tot_wt += items[i].weight;
    print_result(items, best, n, best_val, tot_wt, elapsed, "蛮力法", (int)capacity);

    free(best); free(cur);
}

/* =========================================================
 * 动态规划法（滚动数组，O(C) 空间；回溯重构选择方案）
 * 注意：当 C 非常大时（1000000）且 n 较大时，仍可能内存紧张
 * 对于 n>10000 && C==1000000 的情况，仅输出最优值，不回溯
 * ========================================================= */
void dp_solve(Item *items, int n, long long capacity) {
    double t0 = get_time_ms();

    /* 决定是否回溯选择方案（回溯需要 n×C 的二维表，可能太大）*/
    long long mem_needed = (long long)n * (capacity + 1) * sizeof(double); /* 字节 */
    int can_trace = (mem_needed <= (long long)12 * 1024 * 1024 * 1024LL); /* <=12GB */

    double *dp_prev = NULL, *dp_curr = NULL;
    double **dp2d = NULL;  /* 用于回溯的二维表 */

    if (can_trace) {
        /* 分配二维表 */
        dp2d = (double **)malloc((n + 1) * sizeof(double *));
        if (!dp2d) { can_trace = 0; goto fallback; }
        for (int i = 0; i <= n; i++) {
            dp2d[i] = (double *)calloc(capacity + 1, sizeof(double));
            if (!dp2d[i]) {
                /* 分配失败，回退到滚动数组 */
                for (int j = 0; j < i; j++) free(dp2d[j]);
                free(dp2d); dp2d = NULL; can_trace = 0;
                goto fallback;
            }
        }
        /* 填表 */
        for (int i = 1; i <= n; i++) {
            int    w = items[i-1].weight;
            double v = items[i-1].value;
            for (long long c = 0; c <= capacity; c++) {
                dp2d[i][c] = dp2d[i-1][c];
                if (c >= w && dp2d[i-1][c-w] + v > dp2d[i][c])
                    dp2d[i][c] = dp2d[i-1][c-w] + v;
            }
        }
        goto done_fill;
    }

fallback:;
    /* 滚动数组，无法回溯 */
    dp_prev = (double *)calloc(capacity + 1, sizeof(double));
    dp_curr = (double *)calloc(capacity + 1, sizeof(double));
    if (!dp_prev || !dp_curr) {
        printf("[DP] 内存不足，跳过 n=%d C=%lld\n", n, capacity);
        free(dp_prev); free(dp_curr);
        return;
    }
    for (int i = 0; i < n; i++) {
        int    w = items[i].weight;
        double v = items[i].value;
        for (long long c = capacity; c >= 0; c--) {
            dp_curr[c] = dp_prev[c];
            if (c >= w && dp_prev[c-w] + v > dp_curr[c])
                dp_curr[c] = dp_prev[c-w] + v;
        }
        double *tmp = dp_prev; dp_prev = dp_curr; dp_curr = tmp;
    }

done_fill:;
    double elapsed = get_time_ms() - t0;
    double best_val;
    int *selected = (int *)calloc(n, sizeof(int));

    if (can_trace && dp2d) {
        best_val = dp2d[n][capacity];
        /* 回溯选择方案 */
        long long c = capacity;
        for (int i = n; i >= 1; i--) {
            if (dp2d[i][c] != dp2d[i-1][c]) {
                selected[i-1] = 1;
                c -= items[i-1].weight;
            }
        }
        /* 释放 */
        for (int i = 0; i <= n; i++) free(dp2d[i]);
        free(dp2d);
    } else {
        best_val = dp_prev[capacity];
        /* 无法回溯，selected 全 0 */
        free(dp_prev); free(dp_curr);
    }

    int tot_wt = 0;
    for (int i = 0; i < n; i++) if (selected[i]) tot_wt += items[i].weight;
    print_result(items, selected, n, best_val, tot_wt, elapsed, "动态规划法", (int)capacity);
    free(selected);
}

/* =========================================================
 * 贪心法（按性价比 value/weight 降序贪心，不保证最优解）
 * ========================================================= */
typedef struct { double ratio; int idx; } RatioItem;

int cmp_ratio(const void *a, const void *b) {
    double diff = ((RatioItem*)b)->ratio - ((RatioItem*)a)->ratio;
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}

void greedy_solve(Item *items, int n, long long capacity) {
    double t0 = get_time_ms();

    RatioItem *ri = (RatioItem *)malloc(n * sizeof(RatioItem));
    int       *selected = (int *)calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        ri[i].ratio = items[i].value / (double)items[i].weight;
        ri[i].idx   = i;
    }
    qsort(ri, n, sizeof(RatioItem), cmp_ratio);

    long long  rem_cap   = capacity;
    double     total_val = 0.0;
    for (int k = 0; k < n; k++) {
        int i = ri[k].idx;
        if (items[i].weight <= rem_cap) {
            selected[i] = 1;
            rem_cap    -= items[i].weight;
            total_val  += items[i].value;
        }
    }

    double elapsed = get_time_ms() - t0;
    int tot_wt = (int)(capacity - rem_cap);
    print_result(items, selected, n, total_val, tot_wt, elapsed, "贪心法", (int)capacity);

    free(ri); free(selected);
}

/* =========================================================
 * 回溯法（带剪枝，用贪心上界剪枝）
 * 对于 n>5000，回溯法可能极慢，加时间限制（30s）自动截止
 * ========================================================= */
static Item   *bt_items;
static int     bt_n;
static long long bt_cap;
static double  bt_best_val;
static int    *bt_best;
static int    *bt_cur;
static long long bt_cur_wt;
static double  bt_cur_val;
static double  bt_time_limit_ms;
static double  bt_start_ms;
static int     bt_timeout;

/* 贪心上界：从第 idx 层开始，剩余容量 rem 的最大可能增益（允许部分装入） */
static double upper_bound(int idx, long long rem) {
    double bound = bt_cur_val;
    long long r  = rem;
    for (int i = idx; i < bt_n && r > 0; i++) {
        if (bt_items[i].weight <= r) {
            r     -= bt_items[i].weight;
            bound += bt_items[i].value;
        } else {
            bound += bt_items[i].value * r / bt_items[i].weight;
            break;
        }
    }
    return bound;
}

/* 回溯用的物品排序（按性价比降序，便于剪枝） */
int cmp_ratio_bt(const void *a, const void *b) {
    Item *ia = (Item *)a, *ib = (Item *)b;
    double ra = ia->value / (double)ia->weight;
    double rb = ib->value / (double)ib->weight;
    if (rb > ra) return 1;
    if (rb < ra) return -1;
    return 0;
}

void bt_dfs(int idx, int *orig_idx) {
    if (bt_timeout) return;
    if (get_time_ms() - bt_start_ms > bt_time_limit_ms) {
        bt_timeout = 1;
        return;
    }
    if (idx == bt_n) {
        if (bt_cur_val > bt_best_val) {
            bt_best_val = bt_cur_val;
            memcpy(bt_best, bt_cur, bt_n * sizeof(int));
        }
        return;
    }
    /* 上界剪枝 */
    if (upper_bound(idx, bt_cap - bt_cur_wt) <= bt_best_val) return;

    /* 选第 idx 个物品 */
    if (bt_cur_wt + bt_items[idx].weight <= bt_cap) {
        bt_cur[idx]  = 1;
        bt_cur_wt   += bt_items[idx].weight;
        bt_cur_val  += bt_items[idx].value;
        bt_dfs(idx + 1, orig_idx);
        bt_cur_wt   -= bt_items[idx].weight;
        bt_cur_val  -= bt_items[idx].value;
        bt_cur[idx]  = 0;
    }
    /* 不选第 idx 个物品 */
    bt_dfs(idx + 1, orig_idx);
}

void backtrack_solve(Item *items, int n, long long capacity) {
    /* 对于 n > 10000，回溯法极慢，直接跳过（设1分钟超时） */
    double time_limit = 60000.0; /* 60 秒 */
    if (n > 10000) time_limit = 10000.0; /* 大规模只给10秒 */

    /* 复制并排序（按性价比降序） */
    Item *sorted_items = (Item *)malloc(n * sizeof(Item));
    int  *orig_idx     = (int *)malloc(n * sizeof(int));
    memcpy(sorted_items, items, n * sizeof(Item));
    qsort(sorted_items, n, sizeof(Item), cmp_ratio_bt);
    for (int i = 0; i < n; i++) orig_idx[i] = sorted_items[i].id - 1;

    bt_items  = sorted_items;
    bt_n      = n;
    bt_cap    = capacity;
    bt_best_val = 0.0;
    bt_best   = (int *)calloc(n, sizeof(int));
    bt_cur    = (int *)calloc(n, sizeof(int));
    bt_cur_wt = 0;
    bt_cur_val = 0.0;
    bt_time_limit_ms = time_limit;
    bt_start_ms = get_time_ms();
    bt_timeout  = 0;

    double t0 = bt_start_ms;
    bt_dfs(0, orig_idx);
    double elapsed = get_time_ms() - t0;

    if (bt_timeout)
        printf("\n[回溯法] n=%d C=%lld: 超时（%.0f ms），输出当前最优解\n",
               n, capacity, elapsed);

    /* 将排序后的选择映射回原始编号 */
    int *selected_orig = (int *)calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        if (bt_best[i]) selected_orig[orig_idx[i]] = 1;
    }

    int tot_wt = 0;
    for (int i = 0; i < n; i++) if (selected_orig[i]) tot_wt += items[i].weight;
    print_result(items, selected_orig, n, bt_best_val, tot_wt, elapsed, "回溯法", (int)capacity);

    free(sorted_items); free(orig_idx);
    free(bt_best); free(bt_cur); free(selected_orig);
}

/* =========================================================
 * 主程序
 * ========================================================= */
int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("用法: %s <算法> <n> <C> [seed]\n", argv[0]);
        printf("算法: brute | dp | greedy | backtrack | all\n");
        return 1;
    }
    const char *algo = argv[1];
    int         n    = atoi(argv[2]);
    long long   cap  = (long long)atoll(argv[3]);
    unsigned    seed = (argc >= 5) ? (unsigned)atoi(argv[4]) : 42;

    if (n <= 0 || cap <= 0) {
        fprintf(stderr, "n 和 C 必须为正整数\n");
        return 1;
    }

    Item *items = (Item *)malloc(n * sizeof(Item));
    if (!items) { fprintf(stderr, "内存不足\n"); return 1; }

    generate_items(items, n, seed);

    printf("生成 %d 个物品，背包容量=%lld，随机种子=%u\n", n, cap, seed);

    if (strcmp(algo, "brute") == 0 || strcmp(algo, "all") == 0)
        brute_force(items, n, cap);
    if (strcmp(algo, "dp") == 0 || strcmp(algo, "all") == 0)
        dp_solve(items, n, cap);
    if (strcmp(algo, "greedy") == 0 || strcmp(algo, "all") == 0)
        greedy_solve(items, n, cap);
    if (strcmp(algo, "backtrack") == 0 || strcmp(algo, "all") == 0)
        backtrack_solve(items, n, cap);

    free(items);
    return 0;
}

/*
 * 算法设计与分析 - 排序算法实验
 * 实现：冒泡排序、归并排序、快速排序
 * 功能：统计比较操作次数，记录归并/快速排序每次递归的子问题规模
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* ======================== 全局计数器 ======================== */
long long bubble_compare_count = 0;
long long merge_compare_count  = 0;
long long quick_compare_count  = 0;

/* 子问题规模日志（最多记录 MAX_LOG 条，防止 100000 规模爆内存）*/
#define MAX_LOG 2000

typedef struct {
    int depth;   /* 递归深度 */
    int size;    /* 子问题规模 */
} SubproblemRecord;

SubproblemRecord merge_log[MAX_LOG];
int merge_log_cnt = 0;

SubproblemRecord quick_log[MAX_LOG];
int quick_log_cnt = 0;

/* ======================== 冒泡排序 ======================== */
void bubble_sort(int *arr, int n) {
    bubble_compare_count = 0;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            bubble_compare_count++;
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j]  = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

/* ======================== 归并排序 ======================== */
void merge(int *arr, int left, int mid, int right) {
    int len1 = mid - left + 1;
    int len2 = right - mid;
    int *L = (int *)malloc(len1 * sizeof(int));
    int *R = (int *)malloc(len2 * sizeof(int));

    for (int i = 0; i < len1; i++) L[i] = arr[left + i];
    for (int j = 0; j < len2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < len1 && j < len2) {
        merge_compare_count++;
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }
    while (i < len1) arr[k++] = L[i++];
    while (j < len2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void merge_sort(int *arr, int left, int right, int depth) {
    int size = right - left + 1;
    /* 记录子问题规模（仅在规模 >= 2 时才是实际递归分支）*/
    if (size >= 2 && merge_log_cnt < MAX_LOG) {
        merge_log[merge_log_cnt].depth = depth;
        merge_log[merge_log_cnt].size  = size;
        merge_log_cnt++;
    }
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(arr, left, mid, depth + 1);
        merge_sort(arr, mid + 1, right, depth + 1);
        merge(arr, left, mid, right);
    }
}

/* ======================== 快速排序 ======================== */
int partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        quick_compare_count++;
        if (arr[j] <= pivot) {
            i++;
            int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
        }
    }
    int tmp = arr[i + 1]; arr[i + 1] = arr[high]; arr[high] = tmp;
    return i + 1;
}

void quick_sort(int *arr, int low, int high, int depth) {
    int size = high - low + 1;
    if (size >= 2 && quick_log_cnt < MAX_LOG) {
        quick_log[quick_log_cnt].depth = depth;
        quick_log[quick_log_cnt].size  = size;
        quick_log_cnt++;
    }
    if (low < high) {
        int pi = partition(arr, low, high);
        quick_sort(arr, low, pi - 1, depth + 1);
        quick_sort(arr, pi + 1, high, depth + 1);
    }
}

/* ======================== 工具函数 ======================== */
void fill_random(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000000;
    }
}

void copy_arr(int *dst, const int *src, int n) {
    memcpy(dst, src, n * sizeof(int));
}

/* ======================== 主程序 ======================== */
int main(void) {
    srand((unsigned)42); /* 固定种子，保证可复现 */

    /* 实验①：两组100个随机数 */
    printf("===== 实验① 两组100个随机数的比较操作次数 =====\n\n");
    {
        int n = 100;
        int *raw1 = (int *)malloc(n * sizeof(int));
        int *raw2 = (int *)malloc(n * sizeof(int));
        int *arr  = (int *)malloc(n * sizeof(int));

        fill_random(raw1, n);
        srand((unsigned)12345);
        fill_random(raw2, n);
        srand((unsigned)42); /* 重置种子 */

        printf("-- 第一组数据（前20个）: ");
        for (int i = 0; i < 20 && i < n; i++) printf("%d ", raw1[i]);
        printf("...\n");

        copy_arr(arr, raw1, n);
        bubble_sort(arr, n);
        printf("   冒泡排序比较次数: %lld\n", bubble_compare_count);

        copy_arr(arr, raw1, n);
        merge_compare_count = 0;
        merge_log_cnt = 0;
        merge_sort(arr, 0, n - 1, 0);
        printf("   归并排序比较次数: %lld\n", merge_compare_count);

        copy_arr(arr, raw1, n);
        quick_compare_count = 0;
        quick_log_cnt = 0;
        quick_sort(arr, 0, n - 1, 0);
        printf("   快速排序比较次数: %lld\n\n", quick_compare_count);

        printf("-- 第二组数据（前20个）: ");
        for (int i = 0; i < 20 && i < n; i++) printf("%d ", raw2[i]);
        printf("...\n");

        copy_arr(arr, raw2, n);
        bubble_sort(arr, n);
        printf("   冒泡排序比较次数: %lld\n", bubble_compare_count);

        copy_arr(arr, raw2, n);
        merge_compare_count = 0;
        merge_log_cnt = 0;
        merge_sort(arr, 0, n - 1, 0);
        printf("   归并排序比较次数: %lld\n", merge_compare_count);

        copy_arr(arr, raw2, n);
        quick_compare_count = 0;
        quick_log_cnt = 0;
        quick_sort(arr, 0, n - 1, 0);
        printf("   快速排序比较次数: %lld\n\n", quick_compare_count);

        free(raw1); free(raw2); free(arr);
    }

    /* 实验②：不同规模下的比较操作次数 */
    int sizes[] = {10, 100, 1000, 2000, 5000, 10000, 100000};
    int nsizes  = sizeof(sizes) / sizeof(sizes[0]);

    printf("===== 实验② 不同规模下三种排序算法比较操作次数 =====\n");
    printf("%-10s %-20s %-20s %-20s\n",
           "规模n", "冒泡排序(次)", "归并排序(次)", "快速排序(次)");

    /* 将结果写入CSV，供Python绘图使用 */
    FILE *csv = fopen("sort_compare_counts.csv", "w");
    if (csv) fprintf(csv, "n,bubble,merge,quick\n");

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        int *raw = (int *)malloc(n * sizeof(int));
        int *arr = (int *)malloc(n * sizeof(int));
        if (!raw || !arr) { fprintf(stderr, "内存分配失败 n=%d\n", n); continue; }

        srand((unsigned)(42 + s));
        fill_random(raw, n);

        /* 冒泡排序 */
        copy_arr(arr, raw, n);
        bubble_sort(arr, n);
        long long bc = bubble_compare_count;

        /* 归并排序 */
        copy_arr(arr, raw, n);
        merge_compare_count = 0;
        merge_log_cnt = 0;
        merge_sort(arr, 0, n - 1, 0);
        long long mc = merge_compare_count;

        /* 快速排序 */
        copy_arr(arr, raw, n);
        quick_compare_count = 0;
        quick_log_cnt = 0;
        quick_sort(arr, 0, n - 1, 0);
        long long qc = quick_compare_count;

        printf("%-10d %-20lld %-20lld %-20lld\n", n, bc, mc, qc);
        if (csv) fprintf(csv, "%d,%lld,%lld,%lld\n", n, bc, mc, qc);

        free(raw); free(arr);
    }
    if (csv) fclose(csv);
    printf("\n（数据已保存至 sort_compare_counts.csv）\n\n");

    /* 实验③：归并排序和快速排序的子问题规模（以1000为例完整展示）*/
    printf("===== 实验③ 子问题规模记录（n=1000，各算法前50条）=====\n");
    {
        int n = 1000;
        int *raw = (int *)malloc(n * sizeof(int));
        int *arr = (int *)malloc(n * sizeof(int));
        srand((unsigned)99);
        fill_random(raw, n);

        /* 归并排序子问题规模 */
        copy_arr(arr, raw, n);
        merge_compare_count = 0;
        merge_log_cnt = 0;
        merge_sort(arr, 0, n - 1, 0);
        printf("\n-- 归并排序子问题规模（深度, 规模）共 %d 条：\n", merge_log_cnt);
        printf("  深度  规模\n");
        int show = merge_log_cnt < 50 ? merge_log_cnt : 50;
        for (int i = 0; i < show; i++) {
            printf("  %4d  %d\n", merge_log[i].depth, merge_log[i].size);
        }
        if (merge_log_cnt > 50) printf("  ... (共%d条，仅展示前50条)\n", merge_log_cnt);

        /* 快速排序子问题规模 */
        copy_arr(arr, raw, n);
        quick_compare_count = 0;
        quick_log_cnt = 0;
        quick_sort(arr, 0, n - 1, 0);
        printf("\n-- 快速排序子问题规模（深度, 规模）共 %d 条：\n", quick_log_cnt);
        printf("  深度  规模\n");
        show = quick_log_cnt < 50 ? quick_log_cnt : 50;
        for (int i = 0; i < show; i++) {
            printf("  %4d  %d\n", quick_log[i].depth, quick_log[i].size);
        }
        if (quick_log_cnt > 50) printf("  ... (共%d条，仅展示前50条)\n", quick_log_cnt);

        /* 将子问题规模完整写入CSV */
        FILE *mf = fopen("merge_subproblems.csv", "w");
        if (mf) {
            fprintf(mf, "depth,size\n");
            for (int i = 0; i < merge_log_cnt; i++)
                fprintf(mf, "%d,%d\n", merge_log[i].depth, merge_log[i].size);
            fclose(mf);
        }
        FILE *qf = fopen("quick_subproblems.csv", "w");
        if (qf) {
            fprintf(qf, "depth,size\n");
            for (int i = 0; i < quick_log_cnt; i++)
                fprintf(qf, "%d,%d\n", quick_log[i].depth, quick_log[i].size);
            fclose(qf);
        }
        printf("\n（子问题规模已保存至 merge_subproblems.csv 和 quick_subproblems.csv）\n");

        free(raw); free(arr);
    }

    /* 各规模均输出子问题规模统计摘要 */
    printf("\n===== 各规模下归并/快速排序子问题规模统计 =====\n");
    printf("%-10s %-20s %-20s\n", "规模n", "归并子问题数", "快速子问题数");

    FILE *sub_csv = fopen("subproblem_counts.csv", "w");
    if (sub_csv) fprintf(sub_csv, "n,merge_subproblems,quick_subproblems\n");

    int sizes2[] = {10, 100, 1000, 2000, 5000, 10000};
    int nsizes2  = sizeof(sizes2) / sizeof(sizes2[0]);

    for (int s = 0; s < nsizes2; s++) {
        int n = sizes2[s];
        int *raw = (int *)malloc(n * sizeof(int));
        int *arr = (int *)malloc(n * sizeof(int));
        if (!raw || !arr) continue;

        srand((unsigned)(42 + s));
        fill_random(raw, n);

        copy_arr(arr, raw, n);
        merge_compare_count = 0; merge_log_cnt = 0;
        merge_sort(arr, 0, n - 1, 0);
        int mc_cnt = merge_log_cnt;

        copy_arr(arr, raw, n);
        quick_compare_count = 0; quick_log_cnt = 0;
        quick_sort(arr, 0, n - 1, 0);
        int qc_cnt = quick_log_cnt;

        printf("%-10d %-20d %-20d\n", n, mc_cnt, qc_cnt);
        if (sub_csv) fprintf(sub_csv, "%d,%d,%d\n", n, mc_cnt, qc_cnt);

        free(raw); free(arr);
    }
    if (sub_csv) fclose(sub_csv);

    printf("\n全部实验数据已输出完毕。\n");
    return 0;
}

"""
0-1背包问题实验批处理脚本
自动运行 knapsack.exe，收集不同规模、不同容量下四种算法的执行时间
输出结果到 CSV 文件
"""
import subprocess
import csv
import re
import os
import sys
import time

EXE = r"C:\Users\f1969\WorkBuddy\2026-06-02-19-32-36\knapsack.exe"
BASE = r"C:\Users\f1969\WorkBuddy\2026-06-02-19-32-36"

# 物品数量（题目要求）
SIZES = [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000,
         20000, 40000, 80000, 160000, 320000]

# 背包容量
CAPACITIES = [10000, 100000, 1000000]

# 算法列表（蛮力法只跑极小规模验证，这里实际用 n=25 专项测试，不放入大表）
ALGOS = ['dp', 'greedy', 'backtrack']

SEED = 42

def run_knapsack(algo, n, cap, seed=42, timeout=300):
    """运行 knapsack.exe，返回 (elapsed_ms, total_val, total_wt, status)"""
    cmd = [EXE, algo, str(n), str(cap), str(seed)]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True,
                                encoding='utf-8', errors='replace', timeout=timeout)
        out = result.stdout
        # 解析执行时间
        m_time = re.search(r'执行时间:\s*([\d.]+)\s*ms', out)
        m_val  = re.search(r'总价值:\s*([\d.]+)', out)
        m_wt   = re.search(r'总重量:\s*(\d+)', out)
        elapsed = float(m_time.group(1)) if m_time else -1.0
        val     = float(m_val.group(1))  if m_val  else -1.0
        wt      = int(m_wt.group(1))     if m_wt   else -1
        status  = 'timeout_partial' if '超时' in out else 'ok'
        return elapsed, val, wt, status
    except subprocess.TimeoutExpired:
        return -1.0, -1.0, -1, 'timeout'
    except Exception as e:
        print(f"  ERROR: {e}")
        return -1.0, -1.0, -1, 'error'

# ---- 1. 先测试蛮力法（小规模）----
print("=" * 60)
print("蛮力法小规模测试（n=5,10,15,20,25）")
print("=" * 60)
brute_rows = []
for n in [5, 10, 15, 20, 25]:
    for cap in [100, 500, 1000]:
        t, v, w, s = run_knapsack('brute', n, cap, SEED)
        print(f"  n={n:5d}  C={cap:8d}  time={t:10.3f}ms  val={v:.2f}  status={s}")
        brute_rows.append({'n': n, 'capacity': cap, 'time_ms': f"{t:.3f}", 'value': f"{v:.2f}", 'status': s})

brute_csv = os.path.join(BASE, 'knapsack_brute.csv')
with open(brute_csv, 'w', newline='', encoding='utf-8') as f:
    writer = csv.DictWriter(f, fieldnames=['n','capacity','time_ms','value','status'])
    writer.writeheader()
    writer.writerows(brute_rows)
print(f"\n蛮力法数据已保存: {brute_csv}\n")

# ---- 2. DP / 贪心 / 回溯 大规模测试 ----
print("=" * 60)
print("DP / 贪心 / 回溯 大规模测试")
print("=" * 60)

# 结果存储: results[algo][cap] = list of (n, time_ms, val, wt, status)
results = {algo: {cap: [] for cap in CAPACITIES} for algo in ALGOS}

# 回溯法超时后的后续规模也跳过（避免浪费时间）
backtrack_timeout = {cap: False for cap in CAPACITIES}

for cap in CAPACITIES:
    print(f"\n--- 背包容量 C={cap} ---")
    for n in SIZES:
        for algo in ALGOS:
            if algo == 'backtrack' and backtrack_timeout[cap]:
                print(f"  [{algo}]  n={n:7d}  C={cap:8d}  跳过（前轮超时）")
                results[algo][cap].append((n, -1.0, -1.0, -1, 'skipped'))
                continue

            # 超时限制：DP/贪心较快，回溯较慢
            timeout_sec = 120 if algo in ('dp', 'greedy') else 60

            print(f"  [{algo}]  n={n:7d}  C={cap:8d} ...", end='', flush=True)
            t, v, w, s = run_knapsack(algo, n, cap, SEED, timeout=timeout_sec)
            print(f"  {t:10.3f} ms  val={v:.2f}  {s}")
            results[algo][cap].append((n, t, v, w, s))

            if algo == 'backtrack' and s in ('timeout', 'timeout_partial'):
                backtrack_timeout[cap] = True

# ---- 3. 保存完整 CSV ----
csv_path = os.path.join(BASE, 'knapsack_results.csv')
with open(csv_path, 'w', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    writer.writerow(['algo', 'n', 'capacity', 'time_ms', 'total_value', 'total_weight', 'status'])
    for algo in ALGOS:
        for cap in CAPACITIES:
            for row in results[algo][cap]:
                n, t, v, w, s = row
                writer.writerow([algo, n, cap, f"{t:.3f}", f"{v:.2f}", w, s])

print(f"\n完整数据已保存: {csv_path}")
print("批量实验完成！")

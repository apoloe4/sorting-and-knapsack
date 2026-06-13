"""
排序算法性能对比折线图生成脚本
生成两张图：
1. 三种排序算法比较次数对比（实际测量）
2. 与理论复杂度 n^2, n*log2(n), n*log2(n) 对比
"""

import csv
import math
import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'SimSun']
plt.rcParams['axes.unicode_minus'] = False

# ---- 读取 CSV 数据 ----
csv_path = os.path.join(os.path.dirname(__file__), 'sort_compare_counts.csv')
ns, bubbles, merges, quicks = [], [], [], []

with open(csv_path, newline='', encoding='utf-8') as f:
    reader = csv.DictReader(f)
    for row in reader:
        ns.append(int(row['n']))
        bubbles.append(int(row['bubble']))
        merges.append(int(row['merge']))
        quicks.append(int(row['quick']))

ns = np.array(ns)
bubbles = np.array(bubbles, dtype=float)
merges  = np.array(merges,  dtype=float)
quicks  = np.array(quicks,  dtype=float)

# ---- 图1：三种算法实际比较次数（对数坐标） ----
fig1, ax1 = plt.subplots(figsize=(10, 6))

ax1.plot(ns, bubbles, 'o-', color='#E74C3C', linewidth=2, markersize=6, label='冒泡排序 Bubble Sort')
ax1.plot(ns, merges,  's-', color='#2ECC71', linewidth=2, markersize=6, label='归并排序 Merge Sort')
ax1.plot(ns, quicks,  '^-', color='#3498DB', linewidth=2, markersize=6, label='快速排序 Quick Sort')

ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.set_xlabel('输入规模 n', fontsize=13)
ax1.set_ylabel('比较操作次数', fontsize=13)
ax1.set_title('三种排序算法比较操作次数对比（对数坐标）', fontsize=14, fontweight='bold')
ax1.legend(fontsize=11)
ax1.grid(True, which='both', linestyle='--', alpha=0.5)

# 标注各点数值
for x, y in zip(ns, bubbles):
    if x <= 10000:
        ax1.annotate(f'{int(y):,}', (x, y), textcoords='offset points',
                     xytext=(0, 8), ha='center', fontsize=8, color='#E74C3C')

fig1.tight_layout()
fig1.savefig('sort_compare_loglog.png', dpi=150, bbox_inches='tight')
print(f'图1 已保存: sort_compare_loglog.png')
plt.close(fig1)

# ---- 图2：三种算法实际比较次数（线性坐标，隐藏冒泡可更清晰看归并/快速差异） ----
fig2, axes = plt.subplots(1, 2, figsize=(14, 6))

# 左图：全部三种算法
ax2a = axes[0]
ax2a.plot(ns, bubbles, 'o-', color='#E74C3C', linewidth=2, markersize=6, label='冒泡排序')
ax2a.plot(ns, merges,  's-', color='#2ECC71', linewidth=2, markersize=6, label='归并排序')
ax2a.plot(ns, quicks,  '^-', color='#3498DB', linewidth=2, markersize=6, label='快速排序')
ax2a.set_xlabel('输入规模 n', fontsize=12)
ax2a.set_ylabel('比较操作次数', fontsize=12)
ax2a.set_title('三种排序算法比较次数（线性坐标）', fontsize=13, fontweight='bold')
ax2a.legend(fontsize=11)
ax2a.grid(True, linestyle='--', alpha=0.5)
ax2a.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{x/1e6:.1f}M' if x >= 1e6 else f'{x/1e3:.0f}K' if x >= 1e3 else f'{int(x)}'))

# 右图：仅归并与快速（不含冒泡，方便对比两者差异）
ax2b = axes[1]
ax2b.plot(ns, merges,  's-', color='#2ECC71', linewidth=2, markersize=6, label='归并排序')
ax2b.plot(ns, quicks,  '^-', color='#3498DB', linewidth=2, markersize=6, label='快速排序')
ax2b.set_xlabel('输入规模 n', fontsize=12)
ax2b.set_ylabel('比较操作次数', fontsize=12)
ax2b.set_title('归并排序 vs 快速排序（线性坐标）', fontsize=13, fontweight='bold')
ax2b.legend(fontsize=11)
ax2b.grid(True, linestyle='--', alpha=0.5)
ax2b.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{x/1e6:.1f}M' if x >= 1e6 else f'{x/1e3:.0f}K' if x >= 1e3 else f'{int(x)}'))

fig2.suptitle('排序算法性能对比', fontsize=15, fontweight='bold', y=1.01)
fig2.tight_layout()
fig2.savefig('sort_compare_linear.png', dpi=150, bbox_inches='tight')
print(f'图2 已保存: sort_compare_linear.png')
plt.close(fig2)

# ---- 图3：与理论复杂度曲线对比 ----
n_theory = np.logspace(1, 5, 300)  # 10 ~ 100000

# 理论归一化：让 n=100 时各理论曲线与实测值对齐（便于对比趋势）
idx_100 = np.where(ns == 100)[0][0]

bubble_coeff = bubbles[idx_100] / (100**2)
merge_coeff  = merges[idx_100]  / (100 * math.log2(100))
quick_coeff  = quicks[idx_100]  / (100 * math.log2(100))

theory_bubble = bubble_coeff * n_theory**2
theory_merge  = merge_coeff  * n_theory * np.log2(n_theory)
theory_quick  = quick_coeff  * n_theory * np.log2(n_theory)

fig3, ax3 = plt.subplots(figsize=(11, 7))

# 实测点
ax3.scatter(ns, bubbles, color='#E74C3C', zorder=5, s=60, label='冒泡排序（实测）')
ax3.scatter(ns, merges,  color='#2ECC71', zorder=5, s=60, marker='s', label='归并排序（实测）')
ax3.scatter(ns, quicks,  color='#3498DB', zorder=5, s=60, marker='^', label='快速排序（实测）')

# 理论曲线
ax3.plot(n_theory, theory_bubble, '--', color='#E74C3C', alpha=0.7, linewidth=1.5, label='理论 O(n^2)')
ax3.plot(n_theory, theory_merge,  '--', color='#2ECC71', alpha=0.7, linewidth=1.5, label='理论 O(n*log2(n))')
ax3.plot(n_theory, theory_quick,  '--', color='#3498DB', alpha=0.7, linewidth=1.5, label='理论 O(n*log2(n))')

ax3.set_xscale('log')
ax3.set_yscale('log')
ax3.set_xlabel('输入规模 n', fontsize=13)
ax3.set_ylabel('比较操作次数', fontsize=13)
ax3.set_title('实测比较次数 vs 理论复杂度（对数坐标）', fontsize=14, fontweight='bold')
ax3.legend(fontsize=10, ncol=2)
ax3.grid(True, which='both', linestyle='--', alpha=0.4)

fig3.tight_layout()
fig3.savefig('sort_compare_theory.png', dpi=150, bbox_inches='tight')
print(f'图3 已保存: sort_compare_theory.png')
plt.close(fig3)

# ---- 图4：子问题规模分布（归并 vs 快速，n=1000）----
def load_subproblems(path):
    depths, sizes = [], []
    with open(path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            depths.append(int(row['depth']))
            sizes.append(int(row['size']))
    return np.array(depths), np.array(sizes)

base = os.path.dirname(__file__)
m_depths, m_sizes = load_subproblems(os.path.join(base, 'merge_subproblems.csv'))
q_depths, q_sizes = load_subproblems(os.path.join(base, 'quick_subproblems.csv'))

fig4, axes4 = plt.subplots(1, 2, figsize=(14, 6))

# 归并排序：按深度统计各层子问题规模的分布
ax4a = axes4[0]
unique_merge_depths = sorted(set(m_depths))
box_data_merge = [m_sizes[m_depths == d] for d in unique_merge_depths]
ax4a.boxplot(box_data_merge, positions=unique_merge_depths, widths=0.5, patch_artist=True,
             boxprops=dict(facecolor='#2ECC7140', color='#2ECC71'),
             medianprops=dict(color='#27AE60', linewidth=2),
             whiskerprops=dict(color='#2ECC71'),
             capprops=dict(color='#2ECC71'),
             flierprops=dict(marker='.', color='#2ECC71', markersize=3))
ax4a.set_xlabel('递归深度', fontsize=12)
ax4a.set_ylabel('子问题规模', fontsize=12)
ax4a.set_title('归并排序子问题规模分布（n=1000）', fontsize=13, fontweight='bold')
ax4a.set_yscale('log')
ax4a.grid(True, axis='y', linestyle='--', alpha=0.5)

# 快速排序：散点图更能反映不均匀性
ax4b = axes4[1]
ax4b.scatter(q_depths, q_sizes, alpha=0.4, color='#3498DB', s=20, label='子问题')
# 添加各深度的中位数连线
unique_quick_depths = sorted(set(q_depths))
medians = [np.median(q_sizes[q_depths == d]) for d in unique_quick_depths]
ax4b.plot(unique_quick_depths, medians, 'r-o', markersize=4, linewidth=1.5, label='各深度中位数')
ax4b.set_xlabel('递归深度', fontsize=12)
ax4b.set_ylabel('子问题规模', fontsize=12)
ax4b.set_title('快速排序子问题规模分布（n=1000）', fontsize=13, fontweight='bold')
ax4b.set_yscale('log')
ax4b.legend(fontsize=10)
ax4b.grid(True, linestyle='--', alpha=0.5)

fig4.suptitle('归并 vs 快速排序 递归子问题规模对比', fontsize=14, fontweight='bold')
fig4.tight_layout()
fig4.savefig('sort_subproblem_dist.png', dpi=150, bbox_inches='tight')
print(f'图4 已保存: sort_subproblem_dist.png')
plt.close(fig4)

print('\n所有图表生成完毕！')
print('生成文件：')
print('  sort_compare_loglog.png   - 对数坐标比较次数对比')
print('  sort_compare_linear.png   - 线性坐标比较次数对比')
print('  sort_compare_theory.png   - 实测 vs 理论复杂度')
print('  sort_subproblem_dist.png  - 子问题规模分布')

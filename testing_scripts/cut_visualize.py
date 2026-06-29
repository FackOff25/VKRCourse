#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# Устанавливаем стиль
sns.set_style("whitegrid")
plt.rcParams['font.size'] = 10

# Путь к объединённому CSV-файлу
csv_file = "experiment_results/cut_experiment.csv" 
# Создаём директорию для графиков
output_dir = "plots_refinement"
os.makedirs(output_dir, exist_ok=True)

# Читаем данные
try:
    df = pd.read_csv(csv_file)
    print(f"Загружено {len(df)} записей из файла {csv_file}")
    print("\nУникальные storage_num:", sorted(df['storage_num'].unique()))
    print("Уникальные vertices:", sorted(df['vertices'].unique()))
    print("Уникальные эксперименты:", sorted(df['experiment'].unique()))
except FileNotFoundError:
    print(f"Ошибка: файл {csv_file} не найден")
    print("Убедитесь, что файл находится в той же директории, что и скрипт")
    exit()

# Разделяем данные на NONE и KL
df_none = df[df['experiment'].str.contains('NONE')].copy()
df_kl = df[df['experiment'].str.contains('KL')].copy()

print(f"\nНайдено {len(df_none)} записей с NONE")
print(f"Найдено {len(df_kl)} записей с KL")

# Объединяем данные по storage_num и vertices
# Переименовываем колонку final_cut_percent в соответствии с типом
df_none = df_none.rename(columns={'final_cut_percent': 'none_cut_percent'})
df_kl = df_kl.rename(columns={'final_cut_percent': 'kl_cut_percent'})

# Объединяем данные
df_merged = pd.merge(
    df_none[['storage_num', 'vertices', 'none_cut_percent']],
    df_kl[['storage_num', 'vertices', 'kl_cut_percent']],
    on=['storage_num', 'vertices'],
    how='inner'
)

# Сортируем данные
df_merged = df_merged.sort_values(['storage_num', 'vertices'])

# Получаем список уникальных storage_num
storage_nums = sorted(df_merged['storage_num'].unique())
print(f"\nОбъединено {len(df_merged)} записей для {len(storage_nums)} storage_num")

colors = plt.cm.tab10(range(len(storage_nums)))

for storage in storage_nums:
    storage_data = df_merged[df_merged['storage_num'] == storage].sort_values('vertices')
    
    # График с логарифмической шкалой
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # NONE (baseline) - синяя линия
    ax.plot(storage_data['vertices'], storage_data['none_cut_percent'], 
            marker='o', label='No Refinement', linewidth=2.5, markersize=9, 
            color='blue', linestyle='-')
    
    # KL (optimized) - красная линия
    ax.plot(storage_data['vertices'], storage_data['kl_cut_percent'], 
            marker='s', label='KL Refinement', linewidth=2.5, markersize=9, 
            color='red', linestyle='-')
    
    ax.set_title(f'Storage = {storage} (Log Scale)', fontsize=14, fontweight='bold')
    ax.set_xlabel('Number of Vertices (log scale)', fontsize=12)
    ax.set_ylabel('Cut Percent (%)', fontsize=12)
    ax.legend(fontsize=11, loc='best')
    ax.grid(True, alpha=0.3)
    
    # Логарифмическая шкала для оси X
    ax.set_xscale('log')
    ax.set_xticks(storage_data['vertices'])
    ax.set_xticklabels(storage_data['vertices'])
    
    for _, row in storage_data.iterrows():
        ax.annotate(f'{row["none_cut_percent"]:.2f}%', 
                   (row['vertices'], row['none_cut_percent']),
                   textcoords="offset points", xytext=(0, 10), 
                   ha='center', fontsize=9, color='blue')
        ax.annotate(f'{row["kl_cut_percent"]:.2f}%', 
                   (row['vertices'], row['kl_cut_percent']),
                   textcoords="offset points", xytext=(0, -15), 
                   ha='center', fontsize=9, color='red')
        
        delta = row['none_cut_percent'] - row['kl_cut_percent']
        mid_y = (row['none_cut_percent'] + row['kl_cut_percent']) / 2
        ax.annotate(f'Δ={delta:.4f}', 
                   (row['vertices'], mid_y),
                   textcoords="offset points", xytext=(0, 0), 
                   ha='center', fontsize=8, color='green',
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.7))
    
    plt.tight_layout()
    filename = f'{output_dir}/cuts_storage_{storage}_logscale.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Создан график: {filename}")

n_cols = 2
n_rows = (len(storage_nums) + n_cols - 1) // n_cols

fig, axes = plt.subplots(n_rows, n_cols, figsize=(12, 5*n_rows))
if n_rows * n_cols == 1:
    axes = [axes]
else:
    axes = axes.flatten()

for i, storage in enumerate(storage_nums):
    storage_data = df_merged[df_merged['storage_num'] == storage].sort_values('vertices')
    ax = axes[i]
    
    # NONE (baseline) - синяя линия
    ax.plot(storage_data['vertices'], storage_data['none_cut_percent'], 
            marker='o', label='No Refinement', linewidth=2, markersize=7, 
            color='blue', linestyle='-')
    
    # KL (optimized) - красная линия
    ax.plot(storage_data['vertices'], storage_data['kl_cut_percent'], 
            marker='s', label='KL Refinement', linewidth=2, markersize=7, 
            color='red', linestyle='-')
    
    ax.set_title(f'Storage = {storage}', fontsize=12, fontweight='bold')
    ax.set_xlabel('Vertices (log scale)', fontsize=10)
    ax.set_ylabel('Cut Percent (%)', fontsize=10)
    ax.legend(fontsize=9, loc='best')
    ax.grid(True, alpha=0.3)
    
    # Логарифмическая шкала
    ax.set_xscale('log')
    ax.set_xticks(storage_data['vertices'])
    ax.set_xticklabels(storage_data['vertices'], fontsize=8)
    
    # Добавляем значения на график
    for _, row in storage_data.iterrows():
        ax.annotate(f'{row["none_cut_percent"]:.2f}', 
                   (row['vertices'], row['none_cut_percent']),
                   textcoords="offset points", xytext=(0, 8), 
                   ha='center', fontsize=7, color='blue')
        ax.annotate(f'{row["kl_cut_percent"]:.2f}', 
                   (row['vertices'], row['kl_cut_percent']),
                   textcoords="offset points", xytext=(0, -12), 
                   ha='center', fontsize=7, color='red')
        
        delta = row['none_cut_percent'] - row['kl_cut_percent']
        mid_y = (row['none_cut_percent'] + row['kl_cut_percent']) / 2
        ax.annotate(f'Δ={delta:.4f}', 
                   (row['vertices'], mid_y),
                   textcoords="offset points", xytext=(0, 0), 
                   ha='center', fontsize=8, color='green',
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.7))

# Удаляем пустые подграфики
for i in range(len(storage_nums), len(axes)):
    fig.delaxes(axes[i])

plt.tight_layout()
plt.savefig(f'{output_dir}/cuts_all_storage_grid_2cols_logscale.png', dpi=300, bbox_inches='tight')
plt.close()
print(f"\nСоздан общий график с сеткой 2x{max(1, n_rows)}: {output_dir}/cuts_all_storage_grid_2cols_logscale.png")

# ============ ЧАСТЬ 4: ГРАФИК УЛУЧШЕНИЯ (DELTA) ============
fig, ax = plt.subplots(figsize=(14, 8))

for idx, storage in enumerate(storage_nums):
    storage_data = df_merged[df_merged['storage_num'] == storage].sort_values('vertices')
    delta = storage_data['none_cut_percent'] - storage_data['kl_cut_percent']
    
    ax.plot(storage_data['vertices'], delta, 
             marker='D', label=f's={storage}', linewidth=2.5, 
             markersize=10, color=colors[idx])

ax.set_title('Improvement from KL Optimization (NONE - KL)', fontsize=16, fontweight='bold')
ax.set_xlabel('Number of Vertices (log scale)', fontsize=13)
ax.set_ylabel('Improvement (Cut % reduction)', fontsize=13)
ax.legend(loc='best', fontsize=10)
ax.grid(True, alpha=0.3)
ax.set_xscale('log')
ax.set_xticks(sorted(df_merged['vertices'].unique()))
ax.set_xticklabels(sorted(df_merged['vertices'].unique()))
ax.axhline(y=0, color='black', linestyle='--', alpha=0.5, linewidth=1)

plt.tight_layout()
plt.savefig(f'{output_dir}/delta_improvement_logscale.png', dpi=300, bbox_inches='tight')
plt.close()
print(f"Создан график улучшения: {output_dir}/delta_improvement_logscale.png")

print(f"\nВсе графики сохранены в директории '{output_dir}/'")
print(f"\nСоздано {len(storage_nums)} отдельных графиков + 3 общих графика:")
print(f"- cuts_storage_X_logscale.png - отдельные графики для каждого storage_num")
print(f"- cuts_all_storage_grid_2cols_logscale.png - сетка 2 столбца")
print(f"- cuts_all_storage_combined_logscale.png - все линии на одном графике")
print(f"- delta_improvement_logscale.png - график улучшения")
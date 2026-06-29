#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# Устанавливаем стиль
sns.set_style("whitegrid")
plt.rcParams['font.size'] = 10

# Путь к CSV-файлу
csv_file = "experiment_results/ipt_experiment.csv"  # укажите правильный путь к вашему файлу
output_dir = "plots_ipt"
os.makedirs(output_dir, exist_ok=True)

# Читаем данные из CSV
try:
    df = pd.read_csv(csv_file)
    print(f"Загружено {len(df)} записей из файла {csv_file}")
    print("\nКолонки:", df.columns.tolist())
    print(f"\nУникальные storage_num: {sorted(df['storage_num'].unique())}")
    print(f"Уникальные vertices: {sorted(df['vertices'].unique())}")
except FileNotFoundError:
    print(f"Ошибка: файл {csv_file} не найден")
    print("Проверьте путь к файлу")
    exit()

# Извлекаем информацию из названия эксперимента
def parse_experiment(exp_name):
    """
    Парсит название эксперимента и возвращает:
    - assignment: 'SIMPLE' или 'FENNEL'
    - refinement: 'NONE', 'SCHISM+KL', 'WAWPART+KL' или 'SCHISM+NONE'
    """
    if exp_name.startswith('SIMPLE'):
        assignment = 'SIMPLE'
    elif exp_name.startswith('FENNEL'):
        assignment = 'FENNEL'
    else:
        assignment = 'OTHER'
    
    if 'WAWPART+KL' in exp_name:
        refinement = 'WAWPART+KL'
    elif 'SCHISM+KL' in exp_name:
        refinement = 'SCHISM+KL'
    elif 'SCHISM+NONE' in exp_name:
        refinement = 'SCHISM+NONE'
    else:
        refinement = 'UNKNOWN'
    
    return assignment, refinement

# Добавляем колонки с parsed данными
df[['assignment', 'refinement']] = df['experiment'].apply(
    lambda x: pd.Series(parse_experiment(x))
)

print(f"\nУникальные assignment: {df['assignment'].unique()}")
print(f"Уникальные refinement: {df['refinement'].unique()}")

# ============================================================
# Функция для создания сетки графиков для конкретного assignment
# ============================================================
def create_grid_plots_for_assignment(df_assignment, assignment_name, display_name):
    """
    Создает сетку графиков 2x3 или 3x2 для всех storage_num
    """
    
    storage_nums = sorted(df_assignment['storage_num'].unique())
    print(f"\n{display_name}: storage_nums = {storage_nums}")
    
    # Определяем размер сетки
    n_storages = len(storage_nums)
    if n_storages <= 3:
        n_rows, n_cols = 2, 2  # или 1x3, но лучше 2x2 для читаемости
    else:
        n_rows = (n_storages + 1) // 2  # 2 колонки
        n_cols = 2
    
    # 1. ОБЩИЙ ГРАФИК: все refinement на одном, сетка по storage_num
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(14, 5*n_rows))
    if n_rows * n_cols == 1:
        axes = [axes]
    else:
        axes = axes.flatten()
    
    # Список refinement методов с их настройками
    refinements = [
        ('SCHISM+NONE', 'No Refinement', 'blue', 'o', '-'),
        ('SCHISM+KL', 'SCHISM+KL', 'red', 's', '-'),
        ('WAWPART+KL', 'WAWPART+KL', 'green', '^', '-')
    ]
    
    for i, storage in enumerate(storage_nums):
        ax = axes[i]
        storage_data = df_assignment[df_assignment['storage_num'] == storage].copy()
        storage_data = storage_data.sort_values('vertices')
        
        for ref, label, color, marker, linestyle in refinements:
            subset = storage_data[storage_data['refinement'] == ref]
            if len(subset) == 0:
                continue
            
            # Рисуем линию
            ax.plot(subset['vertices'], subset['inter_shard_percent'],
                    marker=marker, label=label, linewidth=2.5, 
                    markersize=9, color=color, linestyle=linestyle)
            
            # Добавляем значения на точки
            for _, row in subset.iterrows():
                ax.annotate(f'{row["inter_shard_percent"]:.1f}%',
                           (row['vertices'], row['inter_shard_percent']),
                           textcoords="offset points", 
                           xytext=(0, 12 if ref == 'SCHISM+NONE' else -15 if ref == 'SCHISM+KL' else 12),
                           ha='center', fontsize=8, color=color,
                           bbox=dict(boxstyle='round,pad=0.2', facecolor='white', alpha=0.7))
        
        ax.set_title(f'Storage {storage}', fontsize=12, fontweight='bold')
        ax.set_xlabel('Vertices', fontsize=10)
        ax.set_ylabel('Inter-shard Traffic (%)', fontsize=10)
        ax.legend(fontsize=9, loc='best')
        ax.grid(True, alpha=0.3)
        ax.set_xscale('log')
    
    # Удаляем пустые подграфики
    for i in range(len(storage_nums), len(axes)):
        fig.delaxes(axes[i])
    
    plt.suptitle(f'{display_name}: Inter-shard Traffic vs Vertices by Storage', 
                fontsize=16, fontweight='bold', y=1.02)
    plt.tight_layout()
    
    filename = f'{output_dir}/columns_{assignment_name}_all_refinements_grid.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"  Создан общий сеточный график: {filename}")
    
    # 2. ОТДЕЛЬНЫЕ СЕТКИ для каждого refinement метода
    for ref, label, color, marker, linestyle in refinements:
        fig, axes = plt.subplots(n_rows, n_cols, figsize=(14, 5*n_rows))
        if n_rows * n_cols == 1:
            axes = [axes]
        else:
            axes = axes.flatten()
        
        for i, storage in enumerate(storage_nums):
            ax = axes[i]
            subset = df_assignment[(df_assignment['storage_num'] == storage) & 
                                   (df_assignment['refinement'] == ref)]
            if len(subset) == 0:
                ax.set_title(f'Storage {storage} (no data)', fontsize=12)
                ax.grid(True, alpha=0.3)
                continue
            
            subset = subset.sort_values('vertices')
            
            # Рисуем линию
            ax.plot(subset['vertices'], subset['inter_shard_percent'],
                    marker=marker, label=label, linewidth=3, 
                    markersize=11, color=color, linestyle=linestyle)
            
            # Добавляем значения на точки
            for _, row in subset.iterrows():
                ax.annotate(f'{row["inter_shard_percent"]:.2f}%',
                           (row['vertices'], row['inter_shard_percent']),
                           textcoords="offset points", xytext=(0, 12),
                           ha='center', fontsize=9, color=color,
                           bbox=dict(boxstyle='round,pad=0.3', facecolor='white', alpha=0.8))
            
            ax.set_title(f'Storage {storage}', fontsize=12, fontweight='bold')
            ax.set_xlabel('Vertices', fontsize=10)
            ax.set_ylabel('Inter-shard Traffic (%)', fontsize=10)
            ax.legend(fontsize=9, loc='best')
            ax.grid(True, alpha=0.3)
        
        # Удаляем пустые подграфики
        for i in range(len(storage_nums), len(axes)):
            fig.delaxes(axes[i])
        
        plt.suptitle(f'{display_name}: {label} - by Storage', 
                    fontsize=16, fontweight='bold', y=1.02)
        plt.tight_layout()
        
        ref_clean = ref.replace('+', '_')
        filename = f'{output_dir}/{assignment_name}_{ref_clean}_grid.png'
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"  Создан сеточный график для {label}: {filename}")
    
    # 3. ДОПОЛНИТЕЛЬНО: Сравнение всех storage на одном графике для каждого refinement
    for ref, label, color, marker, linestyle in refinements:
        fig, ax = plt.subplots(figsize=(14, 8))
        
        for storage in storage_nums:
            subset = df_assignment[(df_assignment['storage_num'] == storage) & 
                                   (df_assignment['refinement'] == ref)]
            if len(subset) == 0:
                continue
            
            subset = subset.sort_values('vertices')
            ax.plot(subset['vertices'], subset['inter_shard_percent'],
                    marker=marker, label=f'storage={storage}', 
                    linewidth=2.5, markersize=9, linestyle=linestyle)
            
            # Добавляем значения на точки
            for _, row in subset.iterrows():
                ax.annotate(f'{row["inter_shard_percent"]:.1f}%',
                           (row['vertices'], row['inter_shard_percent']),
                           textcoords="offset points", xytext=(5, 5),
                           ha='left', fontsize=8, color='darkgray')
        
        ax.set_title(f'{display_name}: {label} - Comparison across Storage Numbers', 
                    fontsize=15, fontweight='bold')
        ax.set_xlabel('Number of Vertices', fontsize=12)
        ax.set_ylabel('Inter-shard Traffic (%)', fontsize=12)
        ax.set_xscale('log')
        ax.legend(fontsize=10, loc='best')
        ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        ref_clean = ref.replace('+', '_')
        filename = f'{output_dir}/{assignment_name}_{ref_clean}_all_storages.png'
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"  Создан сравнительный график по storages: {filename}")

# ============================================================
# ГРУППА 1: Random assignment (SIMPLE)
# ============================================================
df_random = df[df['assignment'] == 'SIMPLE'].copy()
print(f"\nНайдено {len(df_random)} записей с Random assignment (SIMPLE)")
if len(df_random) > 0:
    create_grid_plots_for_assignment(df_random, 'random', 'Random Assignment (SIMPLE)')

# ============================================================
# ГРУППА 2: Fennel assignment (FENNEL)
# ============================================================
df_fennel = df[df['assignment'] == 'FENNEL'].copy()
print(f"\nНайдено {len(df_fennel)} записей с Fennel assignment")
if len(df_fennel) > 0:
    create_grid_plots_for_assignment(df_fennel, 'fennel', 'Fennel Assignment')

print(f"\nВсе графики сохранены в директории '{output_dir}/'")
print("\nСтруктура созданных графиков (все с префиксом 'columns_'):")
print("  Для каждого assignment (random/fennel):")
print("    - columns_random_all_refinements_grid.png - все методы, сетка по storage")
print("    - columns_random_SCHISM_NONE_grid.png - No Refinement, сетка по storage")
print("    - columns_random_SCHISM_KL_grid.png - SCHISM+KL, сетка по storage")
print("    - columns_random_WAWPART_KL_grid.png - WAWPART+KL, сетка по storage")
print("    - columns_random_SCHISM_NONE_all_storages.png - сравнение storage для No Refinement")
print("    - columns_random_SCHISM_KL_all_storages.png - сравнение storage для SCHISM+KL")
print("    - columns_random_WAWPART_KL_all_storages.png - сравнение storage для WAWPART+KL")
print("  Аналогично для fennel")
#!/usr/bin/env python3
import os
import subprocess
import time
import glob
import sys
from pathlib import Path
import pandas as pd
from datetime import datetime
from statistics import mean

# ==================== НАСТРОЙКИ ====================
CLIENT_BINARY = "../main.out"
GRAPHS_BASE = "graphs_for_experiments"
CONFIGS_BASE = "configs_for_experiments"
RESULTS_DIR = "experiment_results"
LOG_FILE = "/home/fackoff/uniCode/VKRCourse/testing_scripts/path_edges.log"

ALREADY_RAN = [
]

SHOW_CLIENT_OUTPUT = False

os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(f"{RESULTS_DIR}/raw_logs", exist_ok=True)


def get_short_description(storage_num, stream, req, opt, vertices):
    return f"{stream}+{req}+{opt}_s{storage_num}_{vertices}v"

def load_queries(query_file: str):
    queries = []
    with open(query_file, 'r') as f:
        lines = f.readlines()
    for line in lines[1:]:
        parts = line.strip().split()
        if len(parts) >= 3:
            try:
                queries.append((int(parts[1]), int(parts[2])))
            except:
                continue
    return queries

def run_experiment(config_path, graph_metis, coords, queries, exp_name):
    print(f"\n{'='*100}")
    print(f" Запуск эксперимента: {exp_name}")
    print(f"   Config: {config_path}")
    print(f"   Graph:  {graph_metis}")
    print(f"   Запросов: {len(queries)}")
    print(f"{'='*100}\n")

    if os.path.exists(LOG_FILE):
        os.rename(LOG_FILE, f"{RESULTS_DIR}/raw_logs/{exp_name}_{int(time.time())}.log")

    try:
        process = subprocess.Popen(
            [CLIENT_BINARY, config_path, "42"],  # фиксируем seed
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE if SHOW_CLIENT_OUTPUT else subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            text=True,
            bufsize=1,
            universal_newlines=True
        )

        # === Команды ===
        commands = [
            f"load {graph_metis} {coords} 500\n",
            "optimize\n"
        ]

        # Добавляем запросы + оптимизацию каждые 1000 запросов
        for i, (fr, to) in enumerate(queries):
            commands.append(f"path {fr} {to}\n")
            if (i + 1) % 1000 == 0:
                commands.append("optimize\n")   # или "opt\n", если команда поддерживает

        commands.append("cut\n")   # для контроля
        commands.append("exit\n")

        # Отправка команд
        for cmd in commands:
            if process.poll() is not None:
                print("Процесс завершился преждевременно!")
                break
            process.stdin.write(cmd)
            process.stdin.flush()
            # time.sleep(0.001)  # можно оставить маленькую задержку при необходимости

        process.wait(timeout=1200)

        # === Обработка лога ===
        if os.path.exists(LOG_FILE):
            df = pd.read_csv(LOG_FILE, sep=r'\s+', header=None, 
                           names=['path_edges', 'inter_shard'], engine='python')
            
            total_paths = len(df)
            total_edges = int(df['path_edges'].sum())
            total_inter = int(df['inter_shard'].sum())
            percent = (total_inter / total_edges * 100) if total_edges > 0 else 0.0

            # Статистика по неудачным путям
            failed = (df['path_edges'] == 0).sum()

            print(f"\n {exp_name} завершён")
            print(f"   Запросов отправлено: {len(queries)}")
            print(f"   Записей в логе: {total_paths} (failed: {failed})")
            print(f"   → {percent:.3f}% межшардовых ({total_inter}/{total_edges})")
            
            return {
                'experiment': exp_name,
                'storage_num': int(exp_name.split('_s')[1].split('_')[0]),
                'vertices': int(exp_name.split('_')[-1].replace('v','')),
                'total_paths': total_paths,
                'failed_paths': failed,
                'total_edges': total_edges,
                'total_inter_shard': total_inter,
                'inter_shard_percent': round(percent, 4)
            }
        else:
            print("Лог path_edges.log не найден!")
            return None

    except Exception as e:
        print(f"Ошибка в {exp_name}: {e}")
        return None


def main():
    results = []
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    result_file = f"{RESULTS_DIR}/ipt_experiment_{timestamp}.csv"

    graph_dirs = sorted(glob.glob(f"{GRAPHS_BASE}/graph_*v"), 
                       key=lambda x: int(Path(x).name.split('_')[-1].replace('v','')))

    for gdir in graph_dirs:
        v_str = Path(gdir).name.split('_')[-1].replace('v', '')
        metis_file = f"{gdir}/graph_{v_str}.metis"
        coords_file = f"{gdir}/coords_{v_str}.txt"
        query_file = f"{gdir}/queries_{v_str}.txt"

        for storage_dir in sorted(glob.glob(f"{CONFIGS_BASE}/storage_*")):
            storage_num = int(storage_dir.split('_')[-1])

            for config_dir in sorted(glob.glob(f"{storage_dir}/*")):
                config_file = f"{config_dir}/config.ini"
                if not os.path.exists(config_file):
                    continue

                dirname = os.path.basename(config_dir)
                parts = dirname.split('_')
                if len(parts) < 6:
                    continue
                    
                stream = parts[1]
                req = parts[3]
                opt = parts[5]

                if not os.path.exists(metis_file):
                    continue

                queries = load_queries(query_file)
                exp_name = get_short_description(storage_num, stream, req, opt, v_str)
                if exp_name in ALREADY_RAN:
                    print(f'{exp_name} has already been ran, skip')
                    continue
                result = run_experiment(config_file, metis_file, coords_file, queries, exp_name)

                if result:
                    results.append(result)
                    # Сохраняем после каждого эксперимента
                    pd.DataFrame(results).to_csv(result_file, index=False)
                    print(f"   → Сохранено ({len(results)} экспериментов)")

            time.sleep(4)  # пауза между экспериментами

    print(f"\n{'='*110}")
    print("Все эксперименты завершены!")
    print(f"Результаты сохранены в: {result_file}")

    if results:
        df = pd.DataFrame(results)
        print("\nСводка:")
        print(df[['experiment', 'vertices', 'inter_shard_percent']]
              .sort_values(['vertices', 'inter_shard_percent']))


if __name__ == "__main__":
    main()
#!/usr/bin/env python3
import os
import subprocess
import time
import glob
from pathlib import Path
import pandas as pd
from datetime import datetime

# ==================== НАСТРОЙКИ ====================
CLIENT_BINARY = "../main.out"
GRAPHS_BASE = "graphs_for_experiments"
CONFIGS_BASE = "configs_for_experiments"
RESULTS_DIR = "experiment_results"
LOG_FILE = "/home/fackoff/uniCode/VKRCourse/testing_scripts/path_edges.log"

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


def get_graph_for_config(storage_num, stream, req, opt, all_graph_dirs):
    """Выбирает граф в зависимости от конфигурации"""
    graph_dirs = sorted(all_graph_dirs, 
                       key=lambda x: int(Path(x).name.split('_')[-1].replace('v', '')))
    
    if not graph_dirs:
        return None, None, None

    if storage_num == 8:
        # Для 8 шардов — разные графы для разных комбинаций
        # Используем хэш для псевдослучайного, но воспроизводимого выбора
        key = f"{stream}_{req}_{opt}"
        idx = hash(key) % len(graph_dirs)
        chosen_dir = graph_dirs[idx]
    else:
        # Для других — самый большой
        chosen_dir = graph_dirs[-1]

    v = Path(chosen_dir).name.split('_')[-1].replace('v', '')
    metis_file = f"{chosen_dir}/graph_{v}.metis"
    coords_file = f"{chosen_dir}/coords_{v}.txt"
    query_file = f"{chosen_dir}/queries_{v}.txt"
    
    return metis_file, coords_file, query_file, v


def run_experiment(config_path, graph_metis, coords, queries, exp_name, v):
    print(f"\n{'='*100}")
    print(f" Запуск эксперимента: {exp_name}")
    print(f"   Config: {config_path}")
    print(f"   Graph:  {graph_metis}")
    print(f"   Запросов: {len(queries)}")
    print(f"{'='*100}\n")

    if os.path.exists(LOG_FILE):
        os.rename(LOG_FILE, f"{RESULTS_DIR}/raw_logs/{exp_name}_{int(time.time())}.log")

    try:
        # Настройка вывода в зависимости от флага
        if SHOW_CLIENT_OUTPUT:
            stdout = subprocess.PIPE
            stderr = subprocess.STDOUT
        else:
            stdout = subprocess.DEVNULL
            stderr = subprocess.DEVNULL

        process = subprocess.Popen(
            [CLIENT_BINARY, config_path],
            stdin=subprocess.PIPE,
            stdout=stdout,
            stderr=stderr,
            text=True,
            bufsize=1,
            universal_newlines=True
        )

        # Чтение вывода только если он включён
        if SHOW_CLIENT_OUTPUT:
            def print_output():
                for line in process.stdout:
                    print(line.strip(), flush=True)
            
            import threading
            output_thread = threading.Thread(target=print_output, daemon=True)
            output_thread.start()

        # === Команды ===
        commands = [
            f"load {graph_metis} {coords} 500\n",
            "optimize\n"
        ]

        i=0
        for fr, to in queries:
            i+=1
            commands.append(f"path {fr} {to}\n")
            if (i + 1) % 1000 == 0:
                commands.append(f"opt\n")


        commands.append("exit\n")

        i=0
        for cmd in commands:
            i=i+1
            if process.poll() is not None:
                break
            process.stdin.write(cmd)
            process.stdin.flush()
            if (i + 1) % 500 == 0:
                commands.append(f"Отправлено i команд\n")
            #time.sleep(0.003)  # уменьшил задержку

        #process.wait(timeout=900)

        # Обработка лога path_edges.log
        if os.path.exists(LOG_FILE):
            df = pd.read_csv(LOG_FILE, sep=r'\s+', header=None, 
                           names=['path_edges', 'inter_shard'], engine='python')
            
            total_paths = len(df)
            total_edges = int(df['path_edges'].sum())
            total_inter = int(df['inter_shard'].sum())
            percent = (total_inter / total_edges * 100) if total_edges > 0 else 0.0

            print(f"\n {exp_name} завершён → {percent:.3f}% межшардовых ({total_inter}/{total_edges})")
            
            return {
                'experiment': exp_name,
                'storage_num': 8,
                'total_paths': total_paths,
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
    done = ["FENNEL+SCHISM+KL_s8_5000v", "FENNEL+SCHISM+KL_s8_7500v", "FENNEL+SCHISM+KL_s8_10000v", "FENNEL+SCHISM+KL_s8_15000v", "FENNEL+SCHISM+KL_s8_20000v", ]
    results = []
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    result_file = f"{RESULTS_DIR}/results_storage8_{timestamp}.csv"

    # Только storage = 8
    storage_dir = f"{CONFIGS_BASE}/storage_8"
    if not os.path.exists(storage_dir):
        print(f"Папка {storage_dir} не найдена!")
        return

    all_graph_dirs = sorted(glob.glob(f"{GRAPHS_BASE}/graph_*v"), 
                           key=lambda x: int(Path(x).name.split('_')[-1].replace('v','')))

    print(f"Найдено графов: {len(all_graph_dirs)}")
    print(f"Запускаем эксперименты только для storage=8...\n")

    for config_dir in sorted(glob.glob(f"{storage_dir}/*")):
        config_file = f"{config_dir}/config.ini"
        if not os.path.exists(config_file):
            continue

        dirname = os.path.basename(config_dir)
        parts = dirname.split('_')
        stream = parts[1]
        req = parts[3]
        opt = parts[5]

        for gdir in all_graph_dirs:
            v_str = Path(gdir).name.split('_')[-1].replace('v', '')
            metis_file = f"{gdir}/graph_{v_str}.metis"
            coords_file = f"{gdir}/coords_{v_str}.txt"
            query_file = f"{gdir}/queries_{v_str}.txt"

            if not os.path.exists(query_file):
                print(f"⚠ Пропуск графа {v_str}v — нет queries")
                continue

            queries = load_queries(query_file)
            exp_name = get_short_description(8, stream, req, opt, v_str)

            if exp_name in done:
                print(f"Уже делал {exp_name}")
                continue
            result = run_experiment(config_file, metis_file, coords_file, queries, exp_name, v_str)

            if result:
                results.append(result)
                # Сохраняем после каждого эксперимента
                pd.DataFrame(results).to_csv(result_file, index=False)
                print(f"   → Сохранено ({len(results)} экспериментов)")

            time.sleep(4)  # пауза между экспериментами

    print(f"\n{'='*110}")
    print("Все эксперименты для storage=8 завершены!")
    print(f"Результаты сохранены в: {result_file}")

    if results:
        df = pd.DataFrame(results)
        print("\nСводка:")
        print(df[['experiment', 'vertices', 'inter_shard_percent']]
              .sort_values(['vertices', 'inter_shard_percent']))


if __name__ == "__main__":
    main()
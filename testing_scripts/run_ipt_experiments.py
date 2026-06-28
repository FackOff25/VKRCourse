#!/usr/bin/env python3
import os
import subprocess
import time
import glob
import sys
from pathlib import Path
import pandas as pd
from datetime import datetime
from concurrent.futures import ProcessPoolExecutor, as_completed
import multiprocessing
import signal

# ==================== НАСТРОЙКИ ====================
CLIENT_BINARY = "../main.out"
GRAPHS_BASE = "graphs_for_experiments"
CONFIGS_BASE = "configs_for_experiments"
RESULTS_DIR = "experiment_results"
MAX_WORKERS = max(1, int((multiprocessing.cpu_count() - 6) / 2))

SHOW_CLIENT_OUTPUT = False

os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(f"{RESULTS_DIR}/per_ipt_experiment_logs", exist_ok=True)

active_processes = []
executor = None

ALREADY_RAN = [
    "SIMPLE+SCHISM+NONE_s10_5000v",
    "FENNEL+SCHISM+NONE_s10_5000v",
    "FENNEL+SCHISM+KL_s10_5000v",
    "FENNEL+SCHISM+NONE_s12_5000v",
    "FENNEL+SCHISM+KL_s12_5000v",
    "FENNEL+WAWPART+KL_s10_5000v",
    "SIMPLE+SCHISM+NONE_s12_5000v"
    "FENNEL+SCHISM+KL_s16_5000v",
    "FENNEL+SCHISM+NONE_s16_5000v",
    "SIMPLE+SCHISM+KL_s12_5000v",
    "FENNEL+WAWPART+KL_s12_5000v",
    "SIMPLE+SCHISM+KL_s10_5000v",
    "SIMPLE+SCHISM+NONE_s16_5000v",
    "SIMPLE+WAWPART+KL_s12_5000v",
    "FENNEL+SCHISM+NONE_s4_5000v",
    "SIMPLE+WAWPART+KL_s10_5000v",
    "FENNEL+SCHISM+KL_s4_5000v",
    "SIMPLE+SCHISM+KL_s16_5000v",
    "FENNEL+WAWPART+KL_s16_5000v",
    "FENNEL+WAWPART+KL_s4_5000v",
    "FENNEL+SCHISM+KL_s6_5000v",
    "FENNEL+SCHISM+NONE_s6_5000v",
    "SIMPLE+WAWPART+KL_s16_5000v",
    "SIMPLE+SCHISM+KL_s4_5000v",
    "FENNEL+WAWPART+KL_s6_5000v",
    "SIMPLE+WAWPART+KL_s4_5000v",
    "SIMPLE+SCHISM+NONE_s6_5000v",
    "FENNEL+SCHISM+NONE_s8_5000v",
    "FENNEL+SCHISM+KL_s8_5000v",
    "SIMPLE+SCHISM+NONE_s8_5000v",
    "SIMPLE+SCHISM+KL_s6_5000v",
    "FENNEL+WAWPART+KL_s8_5000v",
    "SIMPLE+WAWPART+KL_s6_5000v",
    "FENNEL+SCHISM+NONE_s10_10000v",
    "FENNEL+SCHISM+KL_s10_10000v",
    "SIMPLE+SCHISM+KL_s8_5000v",
    "SIMPLE+SCHISM+NONE_s10_10000v",
    "SIMPLE+SCHISM+NONE_s4_5000v",
    "FENNEL+SCHISM+KL_s12_10000v",
    "SIMPLE+WAWPART+KL_s8_5000v",
    "FENNEL+WAWPART+KL_s10_10000v",
    "FENNEL+SCHISM+NONE_s12_10000v",
    "SIMPLE+SCHISM+NONE_s12_10000v",
    "FENNEL+SCHISM+KL_s16_10000v",
    "FENNEL+WAWPART+KL_s12_10000v",
    "SIMPLE+SCHISM+KL_s12_10000v",
    "FENNEL+SCHISM+NONE_s16_10000v",
    "SIMPLE+SCHISM+NONE_s16_10000v",
    "SIMPLE+SCHISM+KL_s10_10000v",
    "FENNEL+SCHISM+NONE_s4_10000v",
    "FENNEL+WAWPART+KL_s16_10000v",
    "SIMPLE+WAWPART+KL_s12_10000v",
    "SIMPLE+WAWPART+KL_s10_10000v",
    "FENNEL+SCHISM+KL_s4_10000v",
    "SIMPLE+WAWPART+KL_s16_10000v",
    "FENNEL+WAWPART+KL_s4_10000v",
    "FENNEL+SCHISM+NONE_s6_10000v",
    "FENNEL+SCHISM+KL_s6_10000v",
    "FENNEL+WAWPART+KL_s6_10000v",
    "SIMPLE+SCHISM+NONE_s6_10000v",
    "SIMPLE+SCHISM+KL_s4_10000v",
    "FENNEL+SCHISM+KL_s8_10000v",
    "SIMPLE+WAWPART+KL_s4_10000v",
    "FENNEL+SCHISM+NONE_s8_10000v",
    "SIMPLE+SCHISM+NONE_s8_10000v",
    "FENNEL+WAWPART+KL_s8_10000v",
    "SIMPLE+SCHISM+KL_s6_10000v",
    "FENNEL+SCHISM+NONE_s10_20000v",
    "SIMPLE+WAWPART+KL_s6_10000v",
    "FENNEL+SCHISM+KL_s10_20000v",
    "SIMPLE+SCHISM+NONE_s10_20000v",
    "SIMPLE+SCHISM+KL_s8_10000v",
    "SIMPLE+SCHISM+NONE_s4_10000v",
    "FENNEL+WAWPART+KL_s10_20000v",
    "SIMPLE+WAWPART+KL_s8_10000v",
    "FENNEL+SCHISM+NONE_s12_20000v",
    "SIMPLE+SCHISM+NONE_s12_20000v",
    "FENNEL+SCHISM+KL_s12_20000v",
    "SIMPLE+SCHISM+KL_s12_20000v",
    "FENNEL+WAWPART+KL_s12_20000v",
    "FENNEL+SCHISM+NONE_s16_20000v",
    "FENNEL+SCHISM+KL_s16_20000v",
    "SIMPLE+SCHISM+KL_s10_20000v",
    "SIMPLE+SCHISM+NONE_s16_20000v",
    "SIMPLE+SCHISM+KL_s16_20000v",
    "FENNEL+WAWPART+KL_s16_20000v",
    "FENNEL+SCHISM+NONE_s4_20000v",
    "SIMPLE+WAWPART+KL_s12_20000v",
    "SIMPLE+WAWPART+KL_s16_20000v",
    "SIMPLE+WAWPART+KL_s10_20000v",
    "FENNEL+WAWPART+KL_s4_20000v",
    "FENNEL+SCHISM+KL_s4_20000v",
    "FENNEL+SCHISM+NONE_s6_20000v",
    "FENNEL+SCHISM+KL_s6_20000v",
    "SIMPLE+SCHISM+NONE_s6_20000v",
    "FENNEL+WAWPART+KL_s6_20000v",
    "FENNEL+SCHISM+KL_s8_20000v",
    "FENNEL+SCHISM+NONE_s8_20000v"
]

def signal_handler(sig, frame):
    if multiprocessing.current_process().name != "MainProcess":
        return 

    global executor
    print("\n\nПолучен сигнал завершения (Ctrl+C)")

    if executor is not None:
        print("   Останавливаем ProcessPoolExecutor...")
        executor.shutdown(wait=False, cancel_futures=True)

    for proc in active_processes[:]:
        try:
            if proc.poll() is None:
                print(f"   Завершаем PID {proc.pid}...")
                proc.kill()
        except:
            pass
    active_processes.clear()    

    print("Все процессы завершены. Выход.")
    sys.exit(0)

def get_short_description(storage_num, stream, req, opt, vertices):
    return f"{stream}+{req}+{opt}_s{storage_num}_{vertices}v"

def load_queries(query_file: str):
    queries = []
    with open(query_file, 'r') as f:
        for line in f.readlines()[1:]:
            parts = line.strip().split()
            if len(parts) >= 3:
                try:
                    queries.append((int(parts[1]), int(parts[2])))
                except:
                    continue
    return queries

def wait_for_experiment_completion(process, exp, check_interval=2.0):
    
    print(exp + ": Ожидание завершения...", end="", flush=True)
    
    while True:
        # 1. Процесс уже завершился
        if process.poll() is not None:
            print(exp + ": процесс завершён")
            return True
        
        time.sleep(check_interval)


def run_single_experiment(args):
    config_path, graph_metis, coords, queries, exp_name = args
    
    # === Уникальные логи для этого эксперимента ===
    exp_log_dir = Path(f"{RESULTS_DIR}/per_ipt_experiment_logs/{exp_name}_{int(time.time())}")
    exp_log_dir.mkdir(parents=True, exist_ok=True)
    
    path_log = exp_log_dir / "path_edges.log"
    kl_log = exp_log_dir / "kl_experiment.log"
    
    env = os.environ.copy()
    # Можно передать через env, если потом модифицируете main.cpp
    env["PATH_LOG"] = str(path_log)
    env["KL_LOG"] = str(kl_log)

    process = None
    try:
        print(time.strftime("%Y-%m-%d %H:%M:%S") + ": starting " + exp_name)
        process = subprocess.Popen(
            [CLIENT_BINARY, config_path, "42"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE if SHOW_CLIENT_OUTPUT else subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            text=True,
            env=env
        )

        active_processes.append(process)

        commands = [f"load {graph_metis} {coords}\n", "optimize\n"]

        for i, (fr, to) in enumerate(queries):
            commands.append(f"path {fr} {to}\n")
            if (i + 1) % 1000 == 0:
                commands.append("optimize\n")

        commands.append("exit\n")

        for cmd in commands:
            if process.poll() is not None:
                break
            process.stdin.write(cmd)
            process.stdin.flush()
            time.sleep(0.001)

        wait_for_experiment_completion(process, exp_name)

        # === Обработка результатов ===
        if path_log.exists():
            df = pd.read_csv(path_log, sep=r'\s+', header=None, 
                           names=['path_edges', 'inter_shard'], engine='python')
            
            total_paths = len(df)
            failed = (df['path_edges'] == 0).sum()
            total_edges = int(df['path_edges'].sum())
            total_inter = int(df['inter_shard'].sum())
            percent = (total_inter / total_edges * 100) if total_edges > 0 else 0.0

            return {
                'experiment': exp_name,
                'storage_num': int(exp_name.split('_s')[1].split('_')[0]),
                'vertices': int(exp_name.split('_')[-1].replace('v','')),
                'total_paths': total_paths,
                'failed_paths': failed,
                'total_edges': total_edges,
                'total_inter_shard': total_inter,
                'inter_shard_percent': round(percent, 4),
                'log_dir': str(exp_log_dir)
            }
        return None

    except Exception as e:
        print(f"Ошибка в {exp_name}: {e}")
        return None

    finally:
        if process in active_processes:
            active_processes.remove(process)

def main():
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    results = []
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    result_file = f"{RESULTS_DIR}/ipt_experiment_{timestamp}.csv"

    tasks = []
    graph_dirs = sorted(glob.glob(f"{GRAPHS_BASE}/graph_*v"), 
                       key=lambda x: int(Path(x).name.split('_')[-1].replace('v','')))

    for gdir in graph_dirs:
        v_str = Path(gdir).name.split('_')[-1].replace('v', '')
        metis_file = f"{gdir}/graph_{v_str}.metis"
        coords_file = f"{gdir}/coords_{v_str}.txt"
        query_file = f"{gdir}/queries_{v_str}.txt"

        queries = load_queries(query_file)

        for storage_dir in sorted(glob.glob(f"{CONFIGS_BASE}/storage_*")):
            storage_num = int(storage_dir.split('_')[-1])
            for config_dir in sorted(glob.glob(f"{storage_dir}/*")):
                config_file = f"{config_dir}/config.ini"
                if not os.path.exists(config_file):
                    continue

                dirname = os.path.basename(config_dir)
                parts = dirname.split('_')
                if len(parts) < 6: continue

                stream, req, opt = parts[1], parts[3], parts[5]
                exp_name = get_short_description(storage_num, stream, req, opt, v_str)

                if exp_name in ALREADY_RAN:
                    print(f'{exp_name} has already been ran, skip')
                    continue

                tasks.append((config_file, metis_file, coords_file, queries, exp_name))

    print(f"Всего задач: {len(tasks)}. Запускаем в {MAX_WORKERS} процессах...")

    try:
        with ProcessPoolExecutor(max_workers=MAX_WORKERS) as ex:
            executor = ex
            future_to_exp = {ex.submit(run_single_experiment, task): task[4] for task in tasks}

            for future in as_completed(future_to_exp):
                exp_name = future_to_exp[future]
                try:
                    result = future.result()
                    if result:
                        results.append(result)
                        pd.DataFrame(results).to_csv(result_file, index=False)
                        print(f"{exp_name} завершён")
                except Exception as e:
                    print(f"{exp_name} упал: {e}")

    except KeyboardInterrupt:
        print("\nСкрипт прерван пользователем.")
    except Exception as e:
        print(f"Критическая ошибка: {e}")
    finally:
        # Финальная очистка
        print("Финальная очистка...")
        for proc in active_processes[:]:
            try:
                if proc.poll() is None:
                    proc.kill()
            except:
                pass
        active_processes.clear()

        if executor is not None:
            try:
                executor.shutdown(wait=False)
            except:
                pass

    print(f"\nРабота скрипта завершена. Результаты сохранены в {result_file}")


if __name__ == "__main__":
    main()
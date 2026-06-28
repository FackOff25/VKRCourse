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

from concurrent.futures import ProcessPoolExecutor, as_completed
import multiprocessing
import signal

# ==================== НАСТРОЙКИ ====================
CLIENT_BINARY = "../main.out"
GRAPHS_BASE = "graphs_for_experiments"
CONFIGS_BASE = "configs_for_experiments_2"
RESULTS_DIR = "experiment_results"
KL_LOG_FILE = "kl_experiment.log"

MAX_WORKERS = max(1, int((multiprocessing.cpu_count() - 4) / 2))

ALREADY_RAN = []

ALREADY_RAN_2 = [
    "FENNEL+SCHISM+KL_s16_5000v",
    "FENNEL+SCHISM+KL_s12_5000v",
    "FENNEL+SCHISM+KL_s10_5000v",
    "FENNEL+SCHISM+KL_s8_5000v",
    "FENNEL+SCHISM+KL_s6_5000v",
    "FENNEL+SCHISM+KL_s12_10000v",
    "FENNEL+SCHISM+KL_s16_10000v",
    "FENNEL+SCHISM+KL_s10_10000v",
    "FENNEL+SCHISM+KL_s4_5000v",
    "FENNEL+SCHISM+KL_s8_10000v",
    "FENNEL+SCHISM+KL_s6_10000v",
    "FENNEL+SCHISM+KL_s16_20000v",
    "FENNEL+SCHISM+KL_s12_20000v",
    "FENNEL+SCHISM+KL_s4_10000v",
    "FENNEL+SCHISM+KL_s10_20000v",
    "FENNEL+SCHISM+KL_s8_20000v",
    "FENNEL+SCHISM+KL_s6_20000v",
    "FENNEL+SCHISM+KL_s16_50000v",
    "FENNEL+SCHISM+KL_s12_50000v",
    "FENNEL+SCHISM+KL_s4_20000v",
    "FENNEL+SCHISM+KL_s10_50000v",
    "FENNEL+SCHISM+KL_s8_50000v",
    "FENNEL+SCHISM+KL_s6_50000v",
    "FENNEL+SCHISM+KL_s16_100000v",
    "FENNEL+SCHISM+KL_s10_100000v",
    "FENNEL+SCHISM+KL_s12_100000v",
    "FENNEL+SCHISM+KL_s4_50000v",
    "FENNEL+SCHISM+KL_s8_100000v",
    "FENNEL+SCHISM+KL_s6_100000v",
    "FENNEL+SCHISM+KL_s16_250000v",
    "FENNEL+SCHISM+KL_s12_250000v",
    "FENNEL+SCHISM+KL_s10_250000v",
    "FENNEL+SCHISM+KL_s4_100000v"
]


SHOW_CLIENT_OUTPUT = False

os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(f"{RESULTS_DIR}/per_cut_experiment_logs", exist_ok=True)

active_processes = []
executor = None


def signal_handler(sig, frame):
    """Обработчик только для главного процесса"""
    if multiprocessing.current_process().name != "MainProcess":
        return

    global executor
    print("\n\nПолучен сигнал завершения (Ctrl+C). Принудительно останавливаем всё...")

    # Завершаем все процессы
    for proc in active_processes[:]:
        try:
            if proc.poll() is None:
                print(f"   Завершаем PID {proc.pid}...")
                proc.kill()
        except:
            pass
    active_processes.clear()

    # Останавливаем executor
    if executor is not None:
        print("   Останавливаем ProcessPoolExecutor...")
        try:
            executor.shutdown(wait=False, cancel_futures=True)
        except:
            pass

    print("Все процессы завершены. Выход.")
    sys.exit(0)


def get_short_description(storage_num, stream, req, opt, vertices):
    return f"{stream}+{req}+{opt}_s{storage_num}_{vertices}v"

def wait_for_experiment_completion(process, exp, check_interval=2.0):
    
    print(exp + ": Ожидание завершения...", end="", flush=True)
    
    while True:
        # 1. Процесс уже завершился
        if process.poll() is not None:
            print(exp + ": процесс завершён")
            return True
        
        time.sleep(check_interval)

def run_experiment(args):
    config_path, graph_metis, coords, exp_name = args

    # Уникальная директория логов
    exp_log_dir = Path(f"{RESULTS_DIR}/per_cut_experiment_logs/kl_{exp_name}_{int(time.time())}")
    exp_log_dir.mkdir(parents=True, exist_ok=True)

    kl_log = exp_log_dir / "kl_experiment.log"

    env = os.environ.copy()
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

        commands = [
            f"load {graph_metis} {coords} 1000\n",
            "optimize\n",
            "cut\n",
            "exit\n"
        ]

        for cmd in commands:
            if process.poll() is not None:
                break
            process.stdin.write(cmd)
            process.stdin.flush()
            time.sleep(0.4)

        wait_for_experiment_completion(process, exp_name)

        # Обработка KL лога
        initial_cuts = []
        final_cuts = []
        iterations_list = []

        if kl_log.exists():
            with open(kl_log, 'r') as f:
                for line in f:
                    line = line.strip()
                    if not line.startswith("KL"):
                        continue
                    try:
                        parts = line.split()
                        ic = fc = None
                        iters = 0
                        for p in parts:
                            if p.startswith("initial_cut="):
                                ic = float(p.split('=')[1])
                            elif p.startswith("final_cut="):
                                fc = float(p.split('=')[1])
                            elif p.startswith("iterations="):
                                iters = int(p.split('=')[1])
                        if ic is not None:
                            initial_cuts.append(ic)
                        if fc is not None:
                            final_cuts.append(fc)
                        if iters:
                            iterations_list.append(iters)
                    except:
                        continue

        initial_cut = initial_cuts[0] if initial_cuts else None
        final_cut = final_cuts[-1] if final_cuts else None
        total_iterations = mean(iterations_list) if iterations_list else 0
        max_iterations = max(iterations_list) if iterations_list else 0

        result = {
            'experiment': exp_name,
            'storage_num': int(exp_name.split('_s')[1].split('_')[0]),
            'vertices': int(exp_name.split('_')[-1].replace('v', '')),
            #'initial_cut_percent': round(initial_cut, 4) if initial_cut is not None else None,
            'final_cut_percent': round(final_cut, 4) if final_cut is not None else None,
            #'delta_cut': round(final_cut - initial_cut, 4) if initial_cut is not None and final_cut is not None else None,
            'kl_iterations_total': total_iterations,
            'kl_iterations_max': max_iterations,
            'kl_pairs': len(initial_cuts),
            'log_dir': str(exp_log_dir)
        }

        print(time.strftime("%Y-%m-%d %H:%M:%S") + ": ended " + exp_name)

        return result

    except Exception as e:
        print(f"Ошибка в {exp_name}: {e}")
        return None
    finally:
        if process and process in active_processes:
            active_processes.remove(process)


def main():
    global executor
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    results = []
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    result_file = f"{RESULTS_DIR}/cut_experiment_{timestamp}.csv"

    tasks = []
    graph_dirs = sorted(glob.glob(f"{GRAPHS_BASE}/graph_*v"), 
                       key=lambda x: int(Path(x).name.split('_')[-1].replace('v','')))

    for gdir in graph_dirs:
        v_str = Path(gdir).name.split('_')[-1].replace('v', '')
        metis_file = f"{gdir}/graph_{v_str}.metis"
        coords_file = f"{gdir}/coords_{v_str}.txt"

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

                exp_name = get_short_description(storage_num, stream, req, opt, v_str)
                if exp_name in ALREADY_RAN:
                    print(f'{exp_name} has already been ran, skip')
                    continue

                tasks.append((config_file, metis_file, coords_file, exp_name))

    print(f"Всего задач: {len(tasks)}. Запускаем в {MAX_WORKERS} процессах...")

    try:
        with ProcessPoolExecutor(max_workers=MAX_WORKERS) as ex:
            executor = ex
            future_to_exp = {ex.submit(run_experiment, task): task[3] for task in tasks}

            for future in as_completed(future_to_exp):
                exp_name = future_to_exp[future]
                try:
                    result = future.result()
                    if result:
                        results.append(result)
                        pd.DataFrame(results).to_csv(result_file, index=False)
                        print(f"✓ {exp_name} завершён")
                except Exception as e:
                    print(f"✗ {exp_name} упал: {e}")

    except KeyboardInterrupt:
        print("\nСкрипт прерван пользователем.")
    except Exception as e:
        print(f"Критическая ошибка: {e}")
    finally:
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

    print(f"\nЭксперименты завершены! Результаты: {result_file}")


if __name__ == "__main__":
    main()
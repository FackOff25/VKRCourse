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
CONFIGS_BASE = "configs_for_experiments_2"
RESULTS_DIR = "experiment_results"
KL_LOG_FILE = "kl_experiment.log"

ALREADY_RAN = [
    'FENNEL+SCHISM+KL_s10_5000v',
    'SIMPLE+SCHISM+KL_s10_5000v',
    'FENNEL+SCHISM+KL_s12_5000v',
    'SIMPLE+SCHISM+KL_s12_5000v',
    'FENNEL+SCHISM+KL_s16_5000v',
    'SIMPLE+SCHISM+KL_s16_5000v',
    'FENNEL+SCHISM+KL_s4_5000v',
    'SIMPLE+SCHISM+KL_s4_5000v',
    'FENNEL+SCHISM+KL_s6_5000v',
    'SIMPLE+SCHISM+KL_s6_5000v',
    'FENNEL+SCHISM+KL_s8_5000v',
    'SIMPLE+SCHISM+KL_s8_5000v',
    'FENNEL+SCHISM+KL_s10_10000v',
    'SIMPLE+SCHISM+KL_s10_10000v',
    'FENNEL+SCHISM+KL_s12_10000v',
    'SIMPLE+SCHISM+KL_s12_10000v',
    'FENNEL+SCHISM+KL_s16_10000v',
    'SIMPLE+SCHISM+KL_s16_10000v',
    'FENNEL+SCHISM+KL_s4_10000v',
    'SIMPLE+SCHISM+KL_s4_10000v',
    'FENNEL+SCHISM+KL_s6_10000v',
    'SIMPLE+SCHISM+KL_s6_10000v',
    'FENNEL+SCHISM+KL_s8_10000v',
    'SIMPLE+SCHISM+KL_s8_10000v',
    'FENNEL+SCHISM+KL_s10_20000v',
    'SIMPLE+SCHISM+KL_s10_20000v',
    'FENNEL+SCHISM+KL_s12_20000v',
    'SIMPLE+SCHISM+KL_s12_20000v',
    'FENNEL+SCHISM+KL_s16_20000v',
    'SIMPLE+SCHISM+KL_s16_20000v',
    'FENNEL+SCHISM+KL_s4_20000v',
    'SIMPLE+SCHISM+KL_s4_20000v',
    'FENNEL+SCHISM+KL_s6_20000v',
    'SIMPLE+SCHISM+KL_s6_20000v',
    'FENNEL+SCHISM+KL_s8_20000v',
    'SIMPLE+SCHISM+KL_s8_20000v',
    'FENNEL+SCHISM+KL_s10_50000v',
    'SIMPLE+SCHISM+KL_s10_50000v',
    'FENNEL+SCHISM+KL_s12_50000v',
    'SIMPLE+SCHISM+KL_s12_50000v',
    'FENNEL+SCHISM+KL_s16_50000v',
    'SIMPLE+SCHISM+KL_s16_50000v',
    'FENNEL+SCHISM+KL_s4_50000v',
    'SIMPLE+SCHISM+KL_s4_50000v',
    'FENNEL+SCHISM+KL_s6_50000v',
    'FENNEL+SCHISM+KL_s8_50000v',
    'FENNEL+SCHISM+KL_s10_100000v',
    'FENNEL+SCHISM+KL_s12_100000v',
    'FENNEL+SCHISM+KL_s16_100000v',
    'FENNEL+SCHISM+KL_s4_100000v',
    'FENNEL+SCHISM+KL_s6_100000v',
    'FENNEL+SCHISM+KL_s8_100000v',
    'FENNEL+SCHISM+KL_s10_250000v',
    'FENNEL+SCHISM+KL_s12_250000v',
    'FENNEL+SCHISM+KL_s16_250000v'
]

SHOW_CLIENT_OUTPUT = False

os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(f"{RESULTS_DIR}/raw_logs", exist_ok=True)


def get_short_description(storage_num, stream, req, opt, vertices):
    return f"{stream}+{req}+{opt}_s{storage_num}_{vertices}v"


def run_experiment(config_path: str, graph_metis: str, coords: str, exp_name: str):
    print(f"\n{'='*100}")
    print(f" Запуск эксперимента: {exp_name}")
    print(f"   Config: {config_path}")
    print(f"   Graph:  {graph_metis}")
    print(f"{'='*100}\n")

    # Очищаем предыдущий KL лог
    if os.path.exists(KL_LOG_FILE):
        timestamp = int(time.time())
        os.rename(KL_LOG_FILE, f"{RESULTS_DIR}/raw_logs/kl_{exp_name}_{timestamp}.log")

    try:
        process = subprocess.Popen(
            [CLIENT_BINARY, config_path, "42"],  # фиксированный seed
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE if SHOW_CLIENT_OUTPUT else subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            text=True,
            bufsize=1,
            universal_newlines=True
        )

        commands = [
            f"load {graph_metis} {coords}\n",
            "optimize\n",
            "cut\n",
            "exit\n"
        ]

        stdout_output = []
        for cmd in commands:
            if process.poll() is not None:
                break
            process.stdin.write(cmd)
            process.stdin.flush()
            time.sleep(0.4)

        if SHOW_CLIENT_OUTPUT:
            stdout_output, _ = process.communicate(timeout=600000)
            print("=== CLIENT OUTPUT ===\n", stdout_output)
        else:
            process.wait(timeout=600000)

        initial_cuts = []
        final_cuts = []
        iterations_list = []
        deltas = []

        if os.path.exists(KL_LOG_FILE):
            with open(KL_LOG_FILE, 'r') as f:
                for line in f:
                    line = line.strip()
                    if not line.startswith("KL"):
                        continue
                    try:
                        parts = line.split()
                        ic = None
                        fc = None
                        iters = 0
                        delta = None

                        for p in parts:
                            if p.startswith("initial_cut="):
                                ic = float(p.split('=')[1])
                            elif p.startswith("final_cut="):
                                fc = float(p.split('=')[1])
                            elif p.startswith("iterations="):
                                iters = int(p.split('=')[1])
                            elif p.startswith("delta="):
                                delta = float(p.split('=')[1])

                        if ic is not None:
                            initial_cuts.append(ic)
                        if fc is not None:
                            final_cuts.append(fc)
                        if iters is not None:
                            iterations_list.append(iters)
                        if delta is not None:
                            deltas.append(delta)
                    except:
                        continue

        # === ЛОГИКА ВЫБОРА ЗНАЧЕНИЙ ===
        initial_cut = initial_cuts[0] if initial_cuts else None
        final_cut = final_cuts[-1] if final_cuts else None

        total_iterations = mean(iterations_list) if iterations_list else 0
        max_iterations = max(iterations_list) if iterations_list else 0   # ← НОВОЕ

        # Fallback через вывод команды cut
        if final_cut is None and stdout_output:
            for line in stdout_output.splitlines():
                if "Процент рёбер между хранилищами:" in line or "cut" in line.lower():
                    try:
                        final_cut = float(line.split()[-1].replace('%', ''))
                    except:
                        pass

        result = {
            'experiment': exp_name,
            'storage_num': int(exp_name.split('_s')[1].split('_')[0]),
            'vertices': int(exp_name.split('_')[-1].replace('v', '')),
            'initial_cut_percent': round(initial_cut, 4) if initial_cut is not None else None,
            'final_cut_percent': round(final_cut, 4) if final_cut is not None else None,
            'delta_cut': round(final_cut - initial_cut, 4) if initial_cut is not None and final_cut is not None else None,
            'kl_iterations_total': total_iterations,
            'kl_iterations_max': max_iterations,
            'kl_pairs': len(initial_cuts)
        }

        delta_str = f" (Δ = {result['delta_cut']})" if result['delta_cut'] is not None else ""
        print(f" → Initial: {result['initial_cut_percent']}% | Final: {result['final_cut_percent']}%{delta_str} "
              f"| KL max iters: {max_iterations} | pairs: {result['kl_pairs']}")

        return result

    except Exception as e:
        print(f"Ошибка в {exp_name}: {e}")
        return None


def main():
    results = []
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    result_file = f"{RESULTS_DIR}/cut_experiment_{timestamp}.csv"

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

                if not os.path.exists(metis_file):
                    continue

                exp_name = get_short_description(storage_num, stream, req, opt, v_str)
                if exp_name in ALREADY_RAN:
                    print(f'{exp_name} has already been ran, skip')
                    continue
                result = run_experiment(config_file, metis_file, coords_file, exp_name)

                if result:
                    results.append(result)
                    pd.DataFrame(results).to_csv(result_file, index=False)

    # Финальное сохранение
    df = pd.DataFrame(results)
    if not df.empty:
        df.to_csv(result_file, index=False)
        print(f"\n{'='*100}")
        print(f"Эксперименты завершены! Результаты: {result_file}")
        print("\nСводка (сортировка по delta):")
        print(df[['experiment', 'initial_cut_percent', 'final_cut_percent', 
                  'delta_cut', 'kl_iterations']].sort_values('delta_cut'))
    else:
        print("Нет результатов.")


if __name__ == "__main__":
    main()
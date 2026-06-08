#!/bin/bash

set -e  # Exit on error

BASE_DIR="configs_for_experiments"
mkdir -p "$BASE_DIR"

# Вариации параметров
STORAGE_NUMS=(4 6 8 10 12 16)                    # разные количества шардов
STREAMING_TYPES=("SIMPLE" "FENNEL")
REQUEST_WEIGHT_TYPES=("SCHISM" "WAWPART")
OPTIMIZER_TYPES=("NONE" "KL")

# Параметры Fennel (используются только при streaming_type=FENNEL)
FENNEL_BALANCING=1.5

echo "Генерация всех комбинаций конфигов..."

for STORAGE in "${STORAGE_NUMS[@]}"; do
    STORAGE_DIR="$BASE_DIR/storage_${STORAGE}"
    mkdir -p "$STORAGE_DIR"
    
    for STREAM in "${STREAMING_TYPES[@]}"; do
        for REQ in "${REQUEST_WEIGHT_TYPES[@]}"; do
            for OPT in "${OPTIMIZER_TYPES[@]}"; do
                if [[ $OPT == "NONE" && $REQ == "WAWPART" ]]; then
                    continue;
                fi
                
                # Имя папки для комбинации
                COMB_NAME="stream_${STREAM}_req_${REQ}_opt_${OPT}"
                CONFIG_DIR="$STORAGE_DIR/$COMB_NAME"
                mkdir -p "$CONFIG_DIR"
                
                CONFIG_FILE="$CONFIG_DIR/config.ini"
                DESC_FILE="$CONFIG_DIR/description.txt"
                CMD_FILE="$CONFIG_DIR/run_command_example.sh"
                
                # Генерация config.ini
                cat > "$CONFIG_FILE" << EOF
# Конфигурация системы распределённого хранения
# Комбинация: $COMB_NAME
# Количество хранилищ: $STORAGE

storage_num=$STORAGE
streaming_type=$STREAM
request_weight_type=$REQ
optimizer_type=$OPT
EOF

                # Добавляем параметры Fennel только если выбран FENNEL
                if [ "$STREAM" = "FENNEL" ]; then
                    cat >> "$CONFIG_FILE" << EOF

# Параметры Fennel
fennel_balancing_number=$FENNEL_BALANCING
EOF
                fi
                
                # Описание для графиков
                cat > "$DESC_FILE" << EOF
=== Конфигурация для экспериментов ===

Хранилищ (storage_num): $STORAGE
Тип стриминга: $STREAM
Тип взвешивания запросов: $REQ
Тип оптимизатора: $OPT

Полное название комбинации:
${STREAM} + ${REQ} + ${OPT} (${STORAGE} шардов)

Используется для построения графиков зависимости
количества межшардовых переходов от этапа эксперимента.
EOF
                
                # Пример команды запуска (для удобства)
                cat > "$CMD_FILE" << EOF
#!/bin/bash
# Пример запуска эксперимента с этой конфигурацией

./your_program \\
    --config "$CONFIG_FILE" \\
    --graph ../graphs_for_experiments/graph_10000v/graph_10000.metis \\
    --coords ../graphs_for_experiments/graph_10000v/coords_10000.txt \\
    --output results_${COMB_NAME}.csv

# Или через переменные окружения, если ваша программа так поддерживает
EOF
                chmod +x "$CMD_FILE"
                
                echo "  Создано: $COMB_NAME (storage=$STORAGE)"
            done
        done
    done
done

echo "=================================================="
echo "Все конфигурации успешно сгенерированы в: $BASE_DIR/"
echo "Количество комбинаций: $(( ${#STORAGE_NUMS[@]} * ${#STREAMING_TYPES[@]} * ${#REQUEST_WEIGHT_TYPES[@]} * ${#OPTIMIZER_TYPES[@]} ))"
echo ""
ls -R "$BASE_DIR" | head -n 30
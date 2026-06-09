python3 visualize.py \
    --files \
        files/2/none.txt \
        files/2/kl_schism.txt \
        files/2/kl_wawpart.txt \
        files/2/Fennel.txt \
        files/2/Fennel_kl_schism.txt \
        files/2/Fennel_kl_wawpart.txt\
    --labels \
        "Cлучайное распределение" \
        "KL+Schism" \
        "KL+WAWpart" \
        "Fennel" \
        "Fennel+KL+Schism" \
        "Fennel+KL+WAWpart" \
    --title "Доля межшардовых переходов с различным размером графа" \
    --xlabel "Количество вершин графа" \
    --ylabel "Доля межшардовых переходов" \
    --output files/2/result_plot.png

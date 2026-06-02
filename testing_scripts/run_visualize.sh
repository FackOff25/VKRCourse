python3 visualize.py \
    --files \
        files/2/none.txt \
        files/2/Fennel.txt \
        files/2/kl.txt \
        files/2/Fennel_kl.txt \
    --labels \
        "No optimization" \
        "Fennel" \
        "KL" \
        "Fennel + KL" \
    --title "Межшардовые переходы при разном размере графа (4 хранилища)" \
    --xlabel "Кол-во вершие" \
    --ylabel "Межшардовые переходы" \
    --output files/2/result_plot.png

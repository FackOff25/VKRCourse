#!/bin/bash
python3 graph_generator.py \
    --vertices 50000 \
    --avg-degree 7 \
    --graph-file "graphs_for_experiments/graph_50000v/graph_50000.metis" \
    --coords-file "graphs_for_experiments/graph_50000v/coords_50000.txt" \
    --coord-range 1000 \
    --seed 42

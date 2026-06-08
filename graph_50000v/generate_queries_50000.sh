#!/bin/bash
python3 request_generator.py \
    --graph "graphs_for_experiments/graph_50000v/graph_50000.metis" \
    --coords "graphs_for_experiments/graph_50000v/coords_50000.txt" \
    --queries 5000 \
    --output "graphs_for_experiments/graph_50000v/queries_50000.txt" \
    --local-radius 5 \
    --cluster-radius 50 \
    --seed 42

#!/bin/bash

# Script for generating graphs AND query sets for the experiments
# Generates graphs + 10,000 requests for each graph size

set -e  # Exit on error

# Base directory
BASE_DIR="graphs_for_experiments"
mkdir -p "$BASE_DIR"

# Graph sizes (adjust as needed)
VERTEX_COUNTS=(1000 5000 10000 20000 50000)

# Common parameters
AVG_DEGREE=7
COORD_RANGE=1000
SEED=42
NUM_QUERIES=10000

# Request generator parameters
LOCAL_RADIUS=5
CLUSTER_RADIUS=50

echo "=== Starting generation of graphs and queries ==="

for V in "${VERTEX_COUNTS[@]}"; do
    echo "=== Processing graph with $V vertices ==="
    
    GRAPH_DIR="$BASE_DIR/graph_${V}v"
    mkdir -p "$GRAPH_DIR"
    
    # === 1. Graph files ===
    GRAPH_FILE="$GRAPH_DIR/graph_${V}.metis"
    COORDS_FILE="$GRAPH_DIR/coords_${V}.txt"
    PARAMS_GRAPH="$GRAPH_DIR/params_graph_${V}.txt"
    CMD_GRAPH="$GRAPH_DIR/generate_graph_${V}.sh"
    
    echo "  Generating graph..."
    python3 graph_generator.py \
        --vertices "$V" \
        --avg-degree "$AVG_DEGREE" \
        --graph-file "$GRAPH_FILE" \
        --coords-file "$COORDS_FILE" \
        --coord-range "$COORD_RANGE" \
        --seed "$SEED"
    
    # Save graph parameters
    cat > "$PARAMS_GRAPH" << EOF
=== GRAPH GENERATION PARAMETERS ===
Vertices (N): $V
Average Degree: $AVG_DEGREE
Coord Range: $COORD_RANGE
Seed: $SEED
Graph File: $(basename "$GRAPH_FILE")
Coords File: $(basename "$COORDS_FILE")
Generated: $(date)
EOF
    
    cat > "$CMD_GRAPH" << EOF
#!/bin/bash
python3 graph_generator.py \\
    --vertices $V \\
    --avg-degree $AVG_DEGREE \\
    --graph-file "$GRAPH_FILE" \\
    --coords-file "$COORDS_FILE" \\
    --coord-range $COORD_RANGE \\
    --seed $SEED
EOF
    chmod +x "$CMD_GRAPH"
    
    # === 2. Requests generation ===
    echo "  Generating $NUM_QUERIES queries..."
    
    QUERIES_FILE="$GRAPH_DIR/queries_${V}.txt"
    PARAMS_QUERIES="$GRAPH_DIR/params_queries_${V}.txt"
    CMD_QUERIES="$GRAPH_DIR/generate_queries_${V}.sh"
    
    python3 request_generator.py \
        --graph "$GRAPH_FILE" \
        --coords "$COORDS_FILE" \
        --queries "$NUM_QUERIES" \
        --output "$QUERIES_FILE" \
        --local-radius "$LOCAL_RADIUS" \
        --cluster-radius "$CLUSTER_RADIUS" \
        --seed "$SEED"
    
    # Save queries parameters
    cat > "$PARAMS_QUERIES" << EOF
=== QUERY GENERATION PARAMETERS ===
Graph: $(basename "$GRAPH_FILE")
Vertices: $V
Number of queries: $NUM_QUERIES
Local Radius: $LOCAL_RADIUS
Cluster Radius: $CLUSTER_RADIUS
Seed: $SEED
Output File: $(basename "$QUERIES_FILE")
Generated: $(date)
EOF
    
    cat > "$CMD_QUERIES" << EOF
#!/bin/bash
python3 request_generator.py \\
    --graph "$GRAPH_FILE" \\
    --coords "$COORDS_FILE" \\
    --queries $NUM_QUERIES \\
    --output "$QUERIES_FILE" \\
    --local-radius $LOCAL_RADIUS \\
    --cluster-radius $CLUSTER_RADIUS \\
    --seed $SEED
EOF
    chmod +x "$CMD_QUERIES"
    
    echo "  ✓ Done: graph_${V}v"
done

echo ""
echo "=== Generation completed successfully! ==="
echo "All data saved in: $BASE_DIR/"
echo ""
tree "$BASE_DIR" 2>/dev/null || find "$BASE_DIR" -type f | head -20

import argparse
import random

parser = argparse.ArgumentParser(
    description="Generate random graph in METIS format"
)

parser.add_argument(
    "--vertices",
    type=int,
    required=True,
    help="Number of vertices"
)

parser.add_argument(
    "--avg-degree",
    type=int,
    default=6,
    help="Average vertex degree"
)

parser.add_argument(
    "--graph-file",
    default="graph.graph",
    help="Output METIS graph file"
)

parser.add_argument(
    "--coords-file",
    default="graph.xyz",
    help="Output coordinates file"
)

parser.add_argument(
    "--coord-range",
    type=float,
    default=1000.0,
    help="Coordinate range [0, coord-range]"
)

parser.add_argument(
    "--seed",
    type=int,
    default=None,
    help="Random seed"
)

args = parser.parse_args()

if args.seed is not None:
    random.seed(args.seed)

num_vertices = args.vertices
avg_degree = args.avg_degree

coords = {
    i: (
        random.uniform(0, args.coord_range),
        random.uniform(0, args.coord_range)
    )
    for i in range(num_vertices)
}

adj = {i: set() for i in range(num_vertices)}

# Гарантируем связность
for i in range(num_vertices - 1):
    adj[i].add(i + 1)
    adj[i + 1].add(i)

target_edges = num_vertices * avg_degree // 2
current_edges = num_vertices - 1

while current_edges < target_edges:
    u = random.randrange(num_vertices)
    v = random.randrange(num_vertices)

    if u == v:
        continue

    if v in adj[u]:
        continue

    adj[u].add(v)
    adj[v].add(u)

    current_edges += 1

# METIS
with open(args.graph_file, "w") as f:
    f.write(f"{num_vertices} {current_edges}\n")

    for vertex in range(num_vertices):
        neighbors = sorted(adj[vertex])
        f.write(
            " ".join(str(n + 1) for n in neighbors) + "\n"
        )

# Координаты
with open(args.coords_file, "w") as f:
    for vertex in range(num_vertices):
        x, y = coords[vertex]
        f.write(f"{x} {y}\n")

print(
    f"Generated graph: "
    f"{num_vertices} vertices, "
    f"{current_edges} edges"
)
print(f"METIS file: {args.graph_file}")
print(f"Coordinates file: {args.coords_file}")

#!/usr/bin/env python3

import argparse
import math
import random


def load_graph_metis(filename):
    """
    Загружает граф в формате METIS.
    Возвращает:
        adjacency: dict[int, set[int]]
    """

    adjacency = {}

    with open(filename, "r") as f:
        header = f.readline().split()

        if len(header) < 2:
            raise ValueError("Invalid METIS header")

        vertices_count = int(header[0])

        for vertex in range(1, vertices_count + 1):
            line = f.readline()

            if not line:
                adjacency[vertex] = set()
                continue

            neighbours = {
                int(x)
                for x in line.strip().split()
            }

            adjacency[vertex] = neighbours

    return adjacency


def load_coordinates(filename):
    """
    Загружает координаты вершин.
    Формат:
        x y
        x y
        ...
    Номер строки = номер вершины (с 1)
    """

    coords = {}

    with open(filename, "r") as f:
        for vertex_id, line in enumerate(f, start=1):
            x, y = map(float, line.split())
            coords[vertex_id] = (x, y)

    return coords


def vertices_in_radius(center, radius, coords):
    """
    Возвращает вершины,
    находящиеся в пределах заданного радиуса.
    """

    cx, cy = coords[center]

    result = []

    radius_sq = radius * radius

    for vertex, (x, y) in coords.items():
        dx = x - cx
        dy = y - cy

        if dx * dx + dy * dy <= radius_sq:
            result.append(vertex)

    return result


def bfs_neighbourhood(start, depth, adjacency):
    """
    Возвращает множество вершин,
    достижимых за depth шагов.
    """

    visited = {start}
    frontier = {start}

    for _ in range(depth):
        next_frontier = set()

        for v in frontier:
            next_frontier.update(adjacency[v])

        next_frontier -= visited

        if not next_frontier:
            break

        visited.update(next_frontier)
        frontier = next_frontier

    return list(visited)


def generate_random_query(vertices_count):
    source = random.randint(1, vertices_count)
    target = random.randint(1, vertices_count)

    return "RANDOM", source, target


def generate_local_query(vertices_count,
                         coords,
                         radius):

    source = random.randint(1, vertices_count)

    candidates = vertices_in_radius(
        source,
        radius,
        coords
    )

    if len(candidates) > 1:
        target = random.choice(candidates)
    else:
        target = random.randint(1, vertices_count)

    return "LOCAL", source, target


def generate_hub_query(vertices_count,
                       hubs):

    hub = random.choice(hubs)

    if random.random() < 0.5:
        source = hub
        target = random.randint(1, vertices_count)
    else:
        source = random.randint(1, vertices_count)
        target = hub

    return "HUB", source, target


def generate_cluster_query(vertices_count,
                           coords,
                           radius):

    center = random.randint(1, vertices_count)

    candidates = vertices_in_radius(
        center,
        radius,
        coords
    )

    if len(candidates) >= 2:
        source = random.choice(candidates)
        target = random.choice(candidates)
    else:
        source = random.randint(1, vertices_count)
        target = random.randint(1, vertices_count)

    return "CLUSTER", source, target


def generate_graph_local_query(vertices_count,
                               adjacency):

    source = random.randint(1, vertices_count)

    candidates = bfs_neighbourhood(
        source,
        depth=3,
        adjacency=adjacency
    )

    target = random.choice(candidates)

    return "GRAPH_LOCAL", source, target


def main():

    parser = argparse.ArgumentParser(
        description="Query generator for METIS graphs"
    )

    parser.add_argument(
        "--graph",
        required=True,
        help="METIS graph file"
    )

    parser.add_argument(
        "--coords",
        required=True,
        help="Coordinate file"
    )

    parser.add_argument(
        "--queries",
        type=int,
        required=True,
        help="Number of generated queries"
    )

    parser.add_argument(
        "--output",
        default="queries.txt"
    )

    parser.add_argument(
        "--local-radius",
        type=float,
        default=50.0
    )

    parser.add_argument(
        "--cluster-radius",
        type=float,
        default=150.0
    )

    parser.add_argument(
        "--seed",
        type=int
    )

    args = parser.parse_args()

    if args.seed is not None:
        random.seed(args.seed)

    print("Loading graph...")
    adjacency = load_graph_metis(args.graph)

    print("Loading coordinates...")
    coords = load_coordinates(args.coords)

    vertices_count = len(coords)

    hubs_count = max(1, vertices_count // 100)

    hubs = random.sample(
        range(1, vertices_count + 1),
        hubs_count
    )

    queries = []

    for _ in range(args.queries):

        r = random.random()

        if r < 0.40:
            query = generate_random_query(
                vertices_count
            )

        elif r < 0.70:
            query = generate_local_query(
                vertices_count,
                coords,
                args.local_radius
            )

        elif r < 0.90:
            query = generate_hub_query(
                vertices_count,
                hubs
            )

        elif r < 0.95:
            query = generate_cluster_query(
                vertices_count,
                coords,
                args.cluster_radius
            )

        else:
            query = generate_graph_local_query(
                vertices_count,
                adjacency
            )

        queries.append(query)

    with open(args.output, "w") as f:

        f.write(f"{len(queries)}\n")

        for qtype, source, target in queries:
            f.write(
                f"{qtype} {source} {target}\n"
            )

    print(
        f"Generated {len(queries)} queries "
        f"into {args.output}"
    )


if __name__ == "__main__":
    main()

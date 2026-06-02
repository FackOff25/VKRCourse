import argparse
import matplotlib.pyplot as plt


def load_file(path):
    """
    Формат файла:
    x y
    x y
    x y
    """
    x_vals = []
    y_vals = []

    with open(path, "r") as f:
        for line in f:
            line = line.strip()

            if not line:
                continue

            x, y = line.split()
            x_vals.append(float(x))
            y_vals.append(float(y))

    return x_vals, y_vals


def main():
    parser = argparse.ArgumentParser(
        description="Experiment results visualizer"
    )

    parser.add_argument(
        "--files",
        nargs="+",
        required=True,
        help="Input result files"
    )

    parser.add_argument(
        "--labels",
        nargs="+",
        default=None,
        help="Legend labels for files"
    )

    parser.add_argument(
        "--title",
        default="Experiment results"
    )

    parser.add_argument(
        "--xlabel",
        default="Step"
    )

    parser.add_argument(
        "--ylabel",
        default="Cross-shard transitions"
    )

    parser.add_argument(
        "--output",
        default=None,
        help="Save figure to file (optional)"
    )

    args = parser.parse_args()

    plt.figure(figsize=(10, 6))

    labels = args.labels
    if labels is None:
        labels = [f"run_{i}" for i in range(len(args.files))]

    if len(labels) != len(args.files):
        raise ValueError("Number of labels must match number of files")

    for file_path, label in zip(args.files, labels):
        x, y = load_file(file_path)
        plt.plot(x, y, marker="o", label=label)

    plt.title(args.title)
    plt.xlabel(args.xlabel)
    plt.ylabel(args.ylabel)
    plt.legend()
    plt.grid(True)

    if args.output:
        plt.savefig(args.output, dpi=300, bbox_inches="tight")
        print(f"Saved to {args.output}")
    else:
        plt.show()


if __name__ == "__main__":
    main()

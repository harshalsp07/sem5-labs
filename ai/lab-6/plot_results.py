"""
Plotting Script for Lab 06: Map Coloring CSP Performance Comparison.
Generates:
1. Graph 1: Number of Variables vs Average Computation Time (ms)
2. Graph 2: Number of Constraints vs Average Computation Time (ms)
"""

import os
import pandas as pd
import matplotlib.pyplot as plt

def generate_plots(csv_path: str = "/home/harshal/course/ai/lab-6/results.csv", output_dir: str = "/home/harshal/course/ai/lab-6/plots"):
    os.makedirs(output_dir, exist_ok=True)
    df = pd.read_csv(csv_path)

    # Set modern clean styling
    plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
    plt.rcParams.update({
        'font.sans-serif': 'DejaVu Sans',
        'font.family': 'sans-serif',
        'axes.edgecolor': '#cccccc',
        'axes.linewidth': 1.2,
        'grid.color': '#ebebeb',
        'grid.linestyle': '--',
        'grid.alpha': 0.7
    })

    color_dfs = '#1f77b4'  # Elegant blue
    color_ls = '#d62728'   # Crimson red

    # ----------------------------------------------------
    # Graph 1: Number of Variables vs Computation Time
    # ----------------------------------------------------
    fig, ax = plt.subplots(figsize=(8.5, 5.5), dpi=300)
    ax.plot(
        df["Variables"],
        df["DFS Avg. Time (ms)"],
        marker='o',
        linewidth=2.5,
        markersize=7,
        color=color_dfs,
        label="DFS-Backtracking (MRV + LCV + FC)"
    )
    ax.plot(
        df["Variables"],
        df["Local Search Avg. Time (ms)"],
        marker='s',
        linewidth=2.5,
        markersize=7,
        color=color_ls,
        label="Local Search (Min-Conflicts)"
    )

    ax.set_title("Graph 1: Number of Variables vs Computation Time", fontsize=14, fontweight='bold', pad=15)
    ax.set_xlabel("Number of Variables (Regions)", fontsize=12, labelpad=10)
    ax.set_ylabel("Average Computation Time (ms)", fontsize=12, labelpad=10)
    ax.set_xticks(df["Variables"])
    ax.legend(fontsize=11, loc="upper left", frameon=True, framealpha=0.9)
    plt.tight_layout()

    out_path1 = os.path.join(output_dir, "variables_vs_time.png")
    fig.savefig(out_path1, dpi=300)
    plt.close(fig)
    print(f"Generated: {out_path1}")

    # ----------------------------------------------------
    # Graph 2: Number of Constraints vs Computation Time
    # ----------------------------------------------------
    fig, ax = plt.subplots(figsize=(8.5, 5.5), dpi=300)
    ax.plot(
        df["Constraints"],
        df["DFS Avg. Time (ms)"],
        marker='o',
        linewidth=2.5,
        markersize=7,
        color=color_dfs,
        label="DFS-Backtracking (MRV + LCV + FC)"
    )
    ax.plot(
        df["Constraints"],
        df["Local Search Avg. Time (ms)"],
        marker='s',
        linewidth=2.5,
        markersize=7,
        color=color_ls,
        label="Local Search (Min-Conflicts)"
    )

    ax.set_title("Graph 2: Number of Constraints vs Computation Time", fontsize=14, fontweight='bold', pad=15)
    ax.set_xlabel("Number of Constraints (Edges)", fontsize=12, labelpad=10)
    ax.set_ylabel("Average Computation Time (ms)", fontsize=12, labelpad=10)
    ax.set_xticks(df["Constraints"])
    ax.legend(fontsize=11, loc="upper left", frameon=True, framealpha=0.9)
    plt.tight_layout()

    out_path2 = os.path.join(output_dir, "constraints_vs_time.png")
    fig.savefig(out_path2, dpi=300)
    plt.close(fig)
    print(f"Generated: {out_path2}")

if __name__ == "__main__":
    generate_plots()

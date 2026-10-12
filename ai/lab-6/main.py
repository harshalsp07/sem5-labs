"""
Main Runner for Lab 06: Map Coloring CSP Performance Comparison.
Executes DFS-Backtracking vs Local Search benchmarks across all problem sizes,
outputs formatted results according to Section 14, and generates Part F plots.
"""

from experiment import run_experiment
from plot_results import generate_plots

def main():
    print("=" * 60)
    print(" LAB 06: MAP COLORING CSP PERFORMANCE COMPARISON")
    print(" DFS-Backtracking (MRV + LCV + FC) vs Local Search (Min-Conflicts)")
    print("=" * 60)
    print()

    # Step 1: Run benchmarks and print formatted per-test-case output
    df = run_experiment(verbose=True)

    # Step 2: Display Summary Table (Part E)
    print("=" * 60)
    print(" PART E — RESULTS TABLE")
    print("=" * 60)
    display_cols = [
        "Variables",
        "Constraints",
        "DFS Avg. Time (ms)",
        "Local Search Avg. Time (ms)",
        "Avg. Backtracks",
        "Avg. Iterations",
        "DFS Solution",
        "Local Search Solution"
    ]
    print(df[display_cols].to_string(index=False))
    print()

    # Step 3: Generate Part F plots
    print("=" * 60)
    print(" PART F — GENERATING PLOTS")
    print("=" * 60)
    generate_plots()
    print("\nBenchmark experiment complete!")

if __name__ == "__main__":
    main()

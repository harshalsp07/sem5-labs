# Lab 06: Map Coloring CSP - DFS-Backtracking vs Local Search

This repository contains the complete implementation and empirical performance comparison of **DFS-Backtracking with Heuristics (MRV + LCV + Forward Checking)** and **Local Search with Min-Conflicts** applied to the **Map Coloring Problem**.

## File Structure
- `csp.py`: Core CSP representation, variable domains (`Red`, `Green`, `Blue`), constraints, and validation logic.
- `graph_generator.py`: Guaranteed 3-colorable connected graph generator ensuring fair benchmarks.
- `dfs_backtracking.py`: Systematic DFS-Backtracking solver equipped with MRV, LCV, and Forward Checking.
- `local_search.py`: Min-Conflicts Local Search solver with random initialization and tie-breaking.
- `experiment.py`: Benchmark harness executing 10 runs per problem size ($N \in \{3, 5, 7, 10, 15, 20, 30, 40, 50\}$).
- `plot_results.py`: Matplotlib plotting script generating publication-quality performance graphs.
- `main.py`: Entry point that executes tests, displays formatted Section 14 terminal output, prints the Part E table, and exports plots.
- `Lab6_Map_Coloring_CSP.ipynb`: Standalone, interactive Jupyter Notebook containing all algorithms, live experiments, plots, and student answers.
- `REPORT.md`: Comprehensive laboratory report containing the results table, plots, thorough answers to Questions 1–15, and conclusion.
- `results.csv`: Empirical benchmark data.
- `plots/`: Contains generated figures:
  - `variables_vs_time.png` (Graph 1: Number of Variables vs Computation Time)
  - `constraints_vs_time.png` (Graph 2: Number of Constraints vs Computation Time)

## Setup & Running

### 1. Activate Environment
```bash
source venv/bin/activate
```

### 2. Run the Complete Benchmark & Generate Output
```bash
python main.py
```

### 3. Generate Plots Independently (Optional)
```bash
python plot_results.py
```

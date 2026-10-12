"""
Benchmark Experiment Harness for Map Coloring CSP.
Compares DFS-Backtracking (MRV + LCV + Forward Checking) vs Local Search (Min-Conflicts).
Runs 10 trials per test case across variable sizes: [3, 5, 7, 10, 15, 20, 30, 40, 50].
"""

import time
import pandas as pd
from typing import Dict, List, Any
from graph_generator import generate_3_colorable_graph
from dfs_backtracking import DFSBacktrackingSolver
from local_search import MinConflictsLocalSearchSolver

PROBLEM_SIZES = [3, 5, 7, 10, 15, 20, 30, 40, 50]
NUM_RUNS = 10

def run_experiment(verbose: bool = True) -> pd.DataFrame:
    results: List[Dict[str, Any]] = []

    for idx, n_vars in enumerate(PROBLEM_SIZES, start=1):
        # Generate single graph instance used by BOTH algorithms
        csp = generate_3_colorable_graph(num_variables=n_vars, edge_multiplier=1.8, seed=1000 + n_vars)
        n_constraints = csp.num_constraints

        # 1. Evaluate DFS-Backtracking over 10 runs
        dfs_times = []
        dfs_assignments = []
        dfs_backtracks = []
        dfs_solutions_found = []
        dfs_final_solution = None

        for r in range(NUM_RUNS):
            solver = DFSBacktrackingSolver(csp)
            found, sol, t_ms, assigns, btracks = solver.solve()
            dfs_times.append(t_ms)
            dfs_assignments.append(assigns)
            dfs_backtracks.append(btracks)
            dfs_solutions_found.append(found)
            if found and dfs_final_solution is None:
                dfs_final_solution = sol

        dfs_avg_time = sum(dfs_times) / NUM_RUNS
        dfs_avg_assigns = sum(dfs_assignments) / NUM_RUNS
        dfs_avg_backtracks = sum(dfs_backtracks) / NUM_RUNS
        dfs_success = all(dfs_solutions_found)

        # 2. Evaluate Local Search (Min-Conflicts) over 10 runs
        ls_times = []
        ls_iterations = []
        ls_conflicts = []
        ls_solutions_found = []
        ls_final_solution = None

        for r in range(NUM_RUNS):
            solver = MinConflictsLocalSearchSolver(csp, max_iterations=10000, seed=5000 + r * 101 + n_vars)
            found, sol, t_ms, iters, confs = solver.solve()
            ls_times.append(t_ms)
            ls_iterations.append(iters)
            ls_conflicts.append(confs)
            ls_solutions_found.append(found)
            if found and ls_final_solution is None:
                ls_final_solution = sol

        ls_avg_time = sum(ls_times) / NUM_RUNS
        ls_avg_iterations = sum(ls_iterations) / NUM_RUNS
        ls_avg_conflicts = sum(ls_conflicts) / NUM_RUNS
        ls_success = any(ls_solutions_found)

        # Final coloring to display (prefer DFS guaranteed valid, or valid LS)
        final_coloring = dfs_final_solution or ls_final_solution

        if verbose:
            print("========================================")
            print(f"Test Case: {idx}")
            print("========================================")
            print(f"Number of Variables: {n_vars}")
            print(f"Number of Constraints: {n_constraints}")
            print("Number of Colors: 3\n")
            print("DFS-BACKTRACKING")
            print("----------------")
            print(f"Solution Found: {'Yes' if dfs_success else 'No'}")
            print(f"Average Time: {dfs_avg_time:.2f} ms")
            print(f"Average Assignments: {dfs_avg_assigns:.1f}")
            print(f"Average Backtracks: {dfs_avg_backtracks:.1f}\n")
            print("LOCAL SEARCH")
            print("------------")
            print(f"Solution Found: {'Yes' if ls_success else 'No'}")
            print(f"Average Time: {ls_avg_time:.2f} ms")
            print(f"Average Iterations: {ls_avg_iterations:.1f}")
            print(f"Average Conflicts: {ls_avg_conflicts:.1f}\n")
            print("Final Coloring:")
            if final_coloring:
                # Show all variables (or first 15 if very long, but let's print all for completeness)
                for var in csp.variables:
                    print(f"{var} → {final_coloring.get(var, 'Unassigned')}")
            else:
                print("No valid coloring found.")
            print()

        results.append({
            "Variables": n_vars,
            "Constraints": n_constraints,
            "DFS Avg. Time (ms)": round(dfs_avg_time, 3),
            "Local Search Avg. Time (ms)": round(ls_avg_time, 3),
            "Avg. Backtracks": round(dfs_avg_backtracks, 1),
            "Avg. Iterations": round(ls_avg_iterations, 1),
            "DFS Solution": "Yes" if dfs_success else "No",
            "Local Search Solution": "Yes" if ls_success else "No",
            "DFS Avg. Assignments": round(dfs_avg_assigns, 1),
            "LS Avg. Conflicts": round(ls_avg_conflicts, 2)
        })

    df = pd.DataFrame(results)
    df.to_csv("/home/harshal/course/ai/lab-6/results.csv", index=False)
    return df

if __name__ == "__main__":
    df = run_experiment(verbose=True)
    print("\n--- Summary Results Table ---")
    print(df.to_string(index=False))

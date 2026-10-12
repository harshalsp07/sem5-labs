"""
Verification script for CSP solvers.
"""

from graph_generator import generate_3_colorable_graph
from dfs_backtracking import DFSBacktrackingSolver
from local_search import MinConflictsLocalSearchSolver

def test():
    for n in [3, 5, 7, 10]:
        csp = generate_3_colorable_graph(num_variables=n, seed=42)
        print(f"\n--- Testing N = {n}, Constraints = {csp.num_constraints} ---")
        
        # Test DFS
        dfs = DFSBacktrackingSolver(csp)
        found_dfs, sol_dfs, t_dfs, assigns, btracks = dfs.solve()
        print(f"DFS: found={found_dfs}, time={t_dfs:.3f}ms, assigns={assigns}, backtracks={btracks}")
        assert found_dfs, "DFS failed to find solution!"
        assert csp.is_valid_solution(sol_dfs), "DFS solution invalid!"

        # Test Local Search
        ls = MinConflictsLocalSearchSolver(csp, seed=123)
        found_ls, sol_ls, t_ls, iters, confs = ls.solve()
        print(f"Local Search: found={found_ls}, time={t_ls:.3f}ms, iters={iters}, conflicts={confs}")
        assert found_ls, "Local search failed to find solution!"
        assert csp.is_valid_solution(sol_ls), "Local search solution invalid!"

    print("\nAll solver tests passed successfully!")

if __name__ == "__main__":
    test()

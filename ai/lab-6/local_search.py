"""
Local Search Solver with Min-Conflicts Heuristic for Map Coloring CSP.
"""

import time
import random
from typing import Dict, List, Optional, Tuple
from csp import MapColoringCSP

class MinConflictsLocalSearchSolver:
    def __init__(self, csp: MapColoringCSP, max_iterations: int = 10000, seed: Optional[int] = None):
        self.csp = csp
        self.max_iterations = max_iterations
        self.rng = random.Random(seed)
        
        self.iterations = 0
        self.conflicts = 0
        self.execution_time_ms = 0.0
        self.solution: Optional[Dict[str, str]] = None

    def solve(self) -> Tuple[bool, Dict[str, str], float, int, int]:
        """
        Execute Min-Conflicts Local Search.
        Returns:
        (solution_found, current_assignment, execution_time_ms, iterations, final_conflicts)
        """
        start_time = time.perf_counter()
        
        # Step 1: Generate Initial Complete Assignment randomly
        current_assignment: Dict[str, str] = {
            var: self.rng.choice(self.csp.colors) for var in self.csp.variables
        }

        self.iterations = 0
        self.conflicts = self.csp.count_conflicts(current_assignment)

        # If randomly generated assignment happens to be conflict-free
        if self.conflicts == 0:
            end_time = time.perf_counter()
            self.execution_time_ms = (end_time - start_time) * 1000.0
            self.solution = current_assignment
            return True, current_assignment, self.execution_time_ms, self.iterations, self.conflicts

        for step in range(1, self.max_iterations + 1):
            self.iterations = step
            
            # Step 2: Identify Conflicted Variables
            conflicted_vars = self.csp.get_conflicted_variables(current_assignment)
            if not conflicted_vars:
                # 0 conflicts reached
                break

            # Step 3: Select a Conflicted Variable (random choice)
            var_to_change = self.rng.choice(conflicted_vars)

            # Step 4: Try Alternative Values and evaluate number of conflicts for each
            best_values: List[str] = []
            min_conflicts = float('inf')

            for val in self.csp.colors:
                c = self.csp.var_conflicts_with_value(var_to_change, val, current_assignment)
                if c < min_conflicts:
                    min_conflicts = c
                    best_values = [val]
                elif c == min_conflicts:
                    best_values.append(val)

            # Choose the value that minimizes conflicts (random tie-breaking)
            chosen_val = self.rng.choice(best_values)
            current_assignment[var_to_change] = chosen_val

            # Update conflict count
            self.conflicts = self.csp.count_conflicts(current_assignment)
            if self.conflicts == 0:
                break

        end_time = time.perf_counter()
        self.execution_time_ms = (end_time - start_time) * 1000.0
        self.solution = current_assignment
        solution_found = (self.conflicts == 0) and self.csp.is_valid_solution(current_assignment)

        return solution_found, current_assignment, self.execution_time_ms, self.iterations, self.conflicts

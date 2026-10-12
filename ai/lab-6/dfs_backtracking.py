"""
DFS-Backtracking Solver for Map Coloring CSP with:
- Minimum Remaining Values (MRV) Variable Ordering Heuristic
- Least Constraining Value (LCV) Value Ordering Heuristic
- Forward Checking Inference
"""

import time
from typing import Dict, List, Optional, Tuple, Set
from csp import MapColoringCSP

class DFSBacktrackingSolver:
    def __init__(self, csp: MapColoringCSP):
        self.csp = csp
        self.assignments_attempted = 0
        self.backtracks = 0
        self.execution_time_ms = 0.0
        self.solution: Optional[Dict[str, str]] = None

    def select_unassigned_variable_mrv(
        self,
        assignment: Dict[str, str],
        domains: Dict[str, List[str]]
    ) -> str:
        """
        MRV Heuristic: Choose the unassigned variable with the fewest remaining legal values.
        Tie-breaking: Degree heuristic (most unassigned neighbors), then variable name.
        """
        unassigned = [v for v in self.csp.variables if v not in assignment]
        
        def mrv_key(v: str):
            domain_size = len(domains[v])
            unassigned_degree = sum(1 for n in self.csp.neighbors[v] if n not in assignment)
            # Minimize domain size, maximize unassigned degree, deterministic tie-break
            return (domain_size, -unassigned_degree, v)

        return min(unassigned, key=mrv_key)

    def order_domain_values_lcv(
        self,
        var: str,
        assignment: Dict[str, str],
        domains: Dict[str, List[str]]
    ) -> List[str]:
        """
        LCV Heuristic: Order values according to how few choices they eliminate
        from the domains of neighboring unassigned variables.
        """
        if len(domains[var]) <= 1:
            return list(domains[var])

        def eliminated_choices_count(val: str) -> int:
            count = 0
            for neighbor in self.csp.neighbors[var]:
                if neighbor not in assignment and val in domains[neighbor]:
                    count += 1
            return count

        return sorted(domains[var], key=eliminated_choices_count)

    def forward_check(
        self,
        var: str,
        val: str,
        assignment: Dict[str, str],
        domains: Dict[str, List[str]]
    ) -> Optional[Dict[str, List[str]]]:
        """
        Forward Checking:
        1. Remove the assigned color `val` from all unassigned neighbors.
        2. If any neighbor's domain becomes empty, return None (failure).
        3. Otherwise, return the updated domains dictionary.
        """
        new_domains = {v: list(d) for v, d in domains.items()}
        for neighbor in self.csp.neighbors[var]:
            if neighbor not in assignment:
                if val in new_domains[neighbor]:
                    new_domains[neighbor].remove(val)
                    if not new_domains[neighbor]:
                        # A neighbor has no legal values remaining -> fail immediately
                        return None
        return new_domains

    def backtrack_search(
        self,
        assignment: Dict[str, str],
        domains: Dict[str, List[str]]
    ) -> Optional[Dict[str, str]]:
        """
        Recursive DFS-Backtracking search procedure.
        """
        # Base case: complete assignment reached
        if len(assignment) == self.csp.num_variables:
            return assignment

        var = self.select_unassigned_variable_mrv(assignment, domains)
        ordered_values = self.order_domain_values_lcv(var, assignment, domains)

        for val in ordered_values:
            self.assignments_attempted += 1

            if self.csp.is_consistent(var, val, assignment):
                # Inference via Forward Checking
                pruned_domains = self.forward_check(var, val, assignment, domains)
                if pruned_domains is not None:
                    assignment[var] = val
                    result = self.backtrack_search(assignment, pruned_domains)
                    if result is not None:
                        return result
                    # Backtrack step: undo assignment
                    del assignment[var]
                    self.backtracks += 1
                else:
                    # Forward check failed: neighbor domain became empty
                    self.backtracks += 1
            else:
                self.backtracks += 1

        return None

    def solve(self) -> Tuple[bool, Optional[Dict[str, str]], float, int, int]:
        """
        Execute DFS-Backtracking and return:
        (solution_found, solution, execution_time_ms, assignments_attempted, backtracks)
        """
        self.assignments_attempted = 0
        self.backtracks = 0
        
        # Initial domains for all variables: ['Red', 'Green', 'Blue']
        initial_domains: Dict[str, List[str]] = {
            v: list(self.csp.colors) for v in self.csp.variables
        }
        
        start_time = time.perf_counter()
        solution = self.backtrack_search({}, initial_domains)
        end_time = time.perf_counter()
        
        self.execution_time_ms = (end_time - start_time) * 1000.0
        self.solution = solution
        found = solution is not None and self.csp.is_valid_solution(solution)
        
        return found, solution, self.execution_time_ms, self.assignments_attempted, self.backtracks

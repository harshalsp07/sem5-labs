"""
Constraint Satisfaction Problem (CSP) Representation for Map Coloring.
"""

from typing import Dict, List, Set, Tuple, Optional

COLORS = ["Red", "Green", "Blue"]

class MapColoringCSP:
    def __init__(self, variables: List[str], edges: List[Tuple[str, str]], colors: Optional[List[str]] = None):
        """
        Initialize Map Coloring CSP.
        
        :param variables: List of region variable names, e.g., ['A', 'B', 'C', ...]
        :param edges: List of undirected edges (A, B) representing constraints A != B
        :param colors: Available color domain (default: ['Red', 'Green', 'Blue'])
        """
        self.variables: List[str] = list(variables)
        self.colors: List[str] = list(colors) if colors else list(COLORS)
        
        # Adjacency list representation
        self.neighbors: Dict[str, Set[str]] = {v: set() for v in self.variables}
        
        # Normalized edges (ensure u < v for consistent counting and no duplicates)
        self.constraints: Set[Tuple[str, str]] = set()
        for u, v in edges:
            if u != v:
                self.neighbors[u].add(v)
                self.neighbors[v].add(u)
                edge = (u, v) if u < v else (v, u)
                self.constraints.add(edge)
                
        # Sorted list of unique constraints
        self.edges_list: List[Tuple[str, str]] = sorted(list(self.constraints))

    @property
    def num_variables(self) -> int:
        return len(self.variables)

    @property
    def num_constraints(self) -> int:
        return len(self.constraints)

    def is_consistent(self, var: str, color: str, assignment: Dict[str, str]) -> bool:
        """
        Check if assigning `color` to `var` violates constraints with already assigned neighbors.
        """
        for neighbor in self.neighbors[var]:
            if neighbor in assignment and assignment[neighbor] == color:
                return False
        return True

    def count_conflicts(self, assignment: Dict[str, str]) -> int:
        """
        Count the total number of constraint violations in a complete or partial assignment.
        Each edge constraint (u, v) is checked only once.
        """
        conflicts = 0
        for u, v in self.constraints:
            if u in assignment and v in assignment:
                if assignment[u] == assignment[v]:
                    conflicts += 1
        return conflicts

    def get_conflicted_variables(self, assignment: Dict[str, str]) -> List[str]:
        """
        Return the list of variables involved in at least one constraint violation.
        """
        conflicted = set()
        for u, v in self.constraints:
            if u in assignment and v in assignment:
                if assignment[u] == assignment[v]:
                    conflicted.add(u)
                    conflicted.add(v)
        return sorted(list(conflicted))

    def var_conflicts_with_value(self, var: str, value: str, assignment: Dict[str, str]) -> int:
        """
        Calculate how many conflicts `var` would have if assigned `value`,
        keeping all other assignments fixed.
        """
        count = 0
        for neighbor in self.neighbors[var]:
            if neighbor in assignment and assignment[neighbor] == value:
                count += 1
        return count

    def is_valid_solution(self, assignment: Dict[str, str]) -> bool:
        """
        Check if assignment covers all variables and satisfies all constraints.
        """
        if len(assignment) != self.num_variables:
            return False
        for var in self.variables:
            if var not in assignment or assignment[var] not in self.colors:
                return False
        return self.count_conflicts(assignment) == 0

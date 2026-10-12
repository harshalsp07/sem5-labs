"""
Graph Generator for Map Coloring CSP.
Generates guaranteed 3-colorable graphs with realistic map-like connectivity.
"""

import random
from typing import List, Tuple, Dict, Set
from csp import MapColoringCSP, COLORS

def get_variable_name(index: int) -> str:
    """
    Generate readable variable names: A, B, C, ..., Z, A1, B1, ...
    """
    if index < 26:
        return chr(ord('A') + index)
    cycle = index // 26
    char = chr(ord('A') + (index % 26))
    return f"{char}{cycle}"

def generate_3_colorable_graph(
    num_variables: int,
    edge_multiplier: float = 1.8,
    seed: int = 42
) -> MapColoringCSP:
    """
    Generate a guaranteed 3-colorable connected graph with `num_variables` regions.
    
    1. Partition vertices into 3 non-empty subsets (ground-truth coloring).
    2. Only add edges between vertices belonging to different color partitions.
    3. Ensure graph connectivity and realistic edge density.
    """
    if num_variables < 3:
        raise ValueError("Map coloring experiment requires at least 3 variables.")

    rng = random.Random(seed)
    variables = [get_variable_name(i) for i in range(num_variables)]
    
    # Step 1: Assign each variable a ground-truth color to guarantee 3-colorability
    # Ensure all 3 colors are represented if num_variables >= 3
    ground_truth: Dict[str, str] = {}
    ground_truth[variables[0]] = COLORS[0]
    ground_truth[variables[1]] = COLORS[1]
    ground_truth[variables[2]] = COLORS[2]
    
    for i in range(3, num_variables):
        ground_truth[variables[i]] = rng.choice(COLORS)
        
    # Group variables by assigned color
    color_groups: Dict[str, List[str]] = {c: [] for c in COLORS}
    for v, c in ground_truth.items():
        color_groups[c].append(v)
        
    edges: Set[Tuple[str, str]] = set()
    
    def add_valid_edge(u: str, v: str) -> bool:
        if u == v:
            return False
        if ground_truth[u] == ground_truth[v]:
            return False
        edge = (u, v) if u < v else (v, u)
        if edge not in edges:
            edges.add(edge)
            return True
        return False

    # Step 2: Build a spanning backbone to ensure graph connectivity
    # Connect each vertex (after the first) to at least one previous vertex of a different color
    for i in range(1, num_variables):
        u = variables[i]
        valid_candidates = [variables[j] for j in range(i) if ground_truth[variables[j]] != ground_truth[u]]
        if valid_candidates:
            v = rng.choice(valid_candidates)
            add_valid_edge(u, v)

    # Step 3: Add additional cross-color edges to achieve desired constraint density
    target_edges = max(len(edges), int(num_variables * edge_multiplier))
    
    # Collect all possible valid pairs
    possible_pairs: List[Tuple[str, str]] = []
    for i in range(num_variables):
        for j in range(i + 1, num_variables):
            u, v = variables[i], variables[j]
            if ground_truth[u] != ground_truth[v]:
                edge = (u, v) if u < v else (v, u)
                if edge not in edges:
                    possible_pairs.append(edge)
                    
    rng.shuffle(possible_pairs)
    
    while len(edges) < target_edges and possible_pairs:
        edges.add(possible_pairs.pop())
        
    csp = MapColoringCSP(variables, list(edges), COLORS)
    
    # Double check that ground_truth satisfies all constraints
    assert csp.is_valid_solution(ground_truth), "Internal error: Generated graph violates 3-colorability!"
    
    return csp

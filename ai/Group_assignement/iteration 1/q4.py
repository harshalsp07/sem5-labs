import sys
import time

def parse_line():
    line = input().strip()
    while not line:
        line = input().strip()
    return line

def calculate_cost(state, conflicts, S):
    # 1. Conflict penalty: 10 per conflicting pair in same slot
    num_conflicts = 0
    for a, b in conflicts:
        if state[a - 1] == state[b - 1]:
            num_conflicts += 1
    conflict_penalty = num_conflicts * 10

    # 2. Distribution penalty: 2 per excess course beyond 2 in any slot
    slot_count = [0] * (S + 1)
    for slot in state:
        slot_count[slot] += 1

    total_excess = 0
    for s in range(1, S + 1):
        if slot_count[s] > 2:
            total_excess += slot_count[s] - 2
    distribution_penalty = total_excess * 2

    total_cost = conflict_penalty + distribution_penalty
    return total_cost, conflict_penalty, distribution_penalty

def print_timetable(state, N):
    for i in range(N):
        print(f"C{i + 1} → Slot {state[i]}")

def run_hill_climbing(N, S, conflicts, initial_timetable, verbose=True):
    current = initial_timetable[:]
    current_cost, conflict_pen, dist_pen = calculate_cost(current, conflicts, S)

    if verbose:
        print("========================================")
        print("Algorithm: Hill Climbing")
        print("Problem: Exam Timetable Optimization")
        print("========================================")
        print("Initial Timetable:")
        print_timetable(current, N)
        print("Initial Conflict Cost =", conflict_pen)
        print("Initial Distribution Cost =", dist_pen)
        print("Initial Total Cost =", current_cost)
        print("----------------------------------------")

    iteration = 0
    total_neighbors_evaluated = 0
    start_time = time.perf_counter()

    while True:
        best_neighbor = None
        best_neighbor_cost = current_cost
        best_conflict = 0
        best_dist = 0
        neighbors_this_round = 0

        # Generate neighbors by changing slot of exactly one course
        for course in range(N):
            for slot in range(1, S + 1):
                if slot != current[course]:
                    neighbor = current[:]
                    neighbor[course] = slot
                    cost, conf_pen, d_pen = calculate_cost(neighbor, conflicts, S)
                    neighbors_this_round += 1

                    # Tie-breaking: select first such neighbor generated
                    if cost < best_neighbor_cost:
                        best_neighbor = neighbor[:]
                        best_neighbor_cost = cost
                        best_conflict = conf_pen
                        best_dist = d_pen

        total_neighbors_evaluated += neighbors_this_round

        # Stop if no neighbor is strictly better
        if best_neighbor is None:
            break

        iteration += 1
        current = best_neighbor
        current_cost = best_neighbor_cost

        if verbose:
            print(f"Iteration {iteration}")
            print("Best Neighbor Timetable:")
            print_timetable(current, N)
            print("Conflict Cost =", best_conflict)
            print("Distribution Cost =", best_dist)
            print("Total Cost =", best_neighbor_cost)
            print("----------------------------------------")

    exec_time = time.perf_counter() - start_time
    final_cost, final_conflict, final_dist = calculate_cost(current, conflicts, S)

    if verbose:
        print("========================================")
        print("Final Timetable:")
        print_timetable(current, N)
        print("Final Conflict Cost =", final_conflict)
        print("Final Distribution Cost =", final_dist)
        print("Final Cost =", final_cost)
        print("Iterations =", iteration)
        print("Total Neighboring States Evaluated =", total_neighbors_evaluated)
        print(f"Execution Time = {exec_time:.6f} seconds")
        print("Termination Reason: Local Optimum")
        print("========================================")

if __name__ == "__main__":
    # ---- Read Input from stdin ----
    first_line = parse_line()
    parts = first_line.split()
    if len(parts) >= 2:
        N, S = int(parts[0]), int(parts[1])
    elif len(parts) == 1 and len(parts[0]) == 2:
        N, S = int(parts[0][0]), int(parts[0][1])
    else:
        raise ValueError("Invalid N, S input")

    K = int(parse_line())
    conflicts = []
    for _ in range(K):
        line = parse_line()
        parts = line.split()
        if len(parts) >= 2:
            a, b = int(parts[0]), int(parts[1])
        else:
            a, b = int(parts[0][0]), int(parts[0][1])
        conflicts.append((a, b))

    tt_line = parse_line()
    parts = tt_line.split()
    if len(parts) == N:
        timetable = [int(x) for x in parts]
    else:
        # Handles inputs without spaces like '112342'
        timetable = [int(ch) for ch in parts[0][:N]]

    run_hill_climbing(N, S, conflicts, timetable, verbose=True)



import time

# ---- Read Input ----
def parse_line():
    line = input().strip()
    while not line:
        line = input().strip()
    return line

first_line = parse_line()
parts = first_line.split()
if len(parts) >= 2:
    N, M = int(parts[0]), int(parts[1])
elif len(parts) == 1 and len(parts[0]) == 2:
    N, M = int(parts[0][0]), int(parts[0][1])
else:
    raise ValueError("Invalid graph dimensions")

# Build adjacency list as a dictionary
adj = {}
for _ in range(M):
    edge_line = parse_line()
    parts = edge_line.split()
    if len(parts) >= 3:
        u, v, c = parts[0], parts[1], int(parts[2])
    elif len(parts) == 1:
        # e.g., 'AB4'
        u, v, c = parts[0][0], parts[0][1], int(parts[0][2:])
    elif len(parts) == 2:
        # e.g., 'AB 4'
        u, v, c = parts[0][0], parts[0][1], int(parts[1])
    else:
        raise ValueError(f"Invalid edge format: {edge_line}")

    if u not in adj:
        adj[u] = []
    if v not in adj:
        adj[v] = []
    adj[u].append((v, c))
    adj[v].append((u, c))

sg_line = parse_line()
parts = sg_line.split()
if len(parts) >= 2:
    start, goal = parts[0], parts[1]
else:
    start, goal = parts[0][0], parts[0][1]

# Read heuristic values
h = {}
for _ in range(N):
    h_line = parse_line()
    parts = h_line.split()
    if len(parts) >= 2:
        node, val = parts[0], int(parts[1])
    else:
        node, val = parts[0][0], int(parts[0][1:])
    h[node] = val

# ---- Greedy Best-First Search ----
# f(n) = h(n) only
start_time_greedy = time.perf_counter()
frontier = [(start, [start], 0)]   # (current_node, path, cost_so_far)
visited = set()
greedy_found = False
greedy_path = []
greedy_cost = 0
greedy_expanded = 0

while len(frontier) > 0:
    # Find node with smallest h(n) in frontier
    best = 0
    for i in range(1, len(frontier)):
        if h[frontier[i][0]] < h[frontier[best][0]]:
            best = i

    curr, path, cost = frontier.pop(best)

    if curr in visited:
        continue
    visited.add(curr)
    greedy_expanded += 1

    if curr == goal:
        greedy_found = True
        greedy_path = path
        greedy_cost = cost
        break

    if curr in adj:
        for neighbor, edge_cost in adj[curr]:
            if neighbor not in visited:
                frontier.append((neighbor, path + [neighbor], cost + edge_cost))

exec_time_greedy = time.perf_counter() - start_time_greedy

print("Algorithm: Greedy Best-First Search")
if greedy_found:
    print("Path Found: Yes")
    print("Path:", " -> ".join(greedy_path))
    print("Total Cost =", greedy_cost)
    print("Nodes Expanded =", greedy_expanded)
    print(f"Execution Time = {exec_time_greedy:.6f} seconds")
else:
    print("Path Found: No")
    print(f"Execution Time = {exec_time_greedy:.6f} seconds")

print()

# ---- A* Search ----
# f(n) = g(n) + h(n)
start_time_astar = time.perf_counter()
frontier2 = [(start, [start], 0)]   # (current_node, path, g_cost)
visited2 = set()
astar_found = False
astar_path = []
astar_cost = 0
astar_expanded = 0

while len(frontier2) > 0:
    # Find node with smallest f(n) = g(n) + h(n)
    best = 0
    best_f = frontier2[0][2] + h[frontier2[0][0]]
    for i in range(1, len(frontier2)):
        f_i = frontier2[i][2] + h[frontier2[i][0]]
        if f_i < best_f:
            best = i
            best_f = f_i
        elif f_i == best_f:
            # tie-break: prefer longer path (more explored)
            if len(frontier2[i][1]) > len(frontier2[best][1]):
                best = i
                best_f = f_i

    curr, path, g_cost = frontier2.pop(best)

    if curr in visited2:
        continue
    visited2.add(curr)
    astar_expanded += 1

    if curr == goal:
        astar_found = True
        astar_path = path
        astar_cost = g_cost
        break

    if curr in adj:
        for neighbor, edge_cost in adj[curr]:
            if neighbor not in visited2:
                frontier2.append((neighbor, path + [neighbor], g_cost + edge_cost))

exec_time_astar = time.perf_counter() - start_time_astar

print("Algorithm: A* Search")
if astar_found:
    print("Path Found: Yes")
    print("Path:", " -> ".join(astar_path))
    print("Total Cost =", astar_cost)
    print("Nodes Expanded =", astar_expanded)
    print(f"Execution Time = {exec_time_astar:.6f} seconds")
else:
    print("Path Found: No")
    print(f"Execution Time = {exec_time_astar:.6f} seconds")


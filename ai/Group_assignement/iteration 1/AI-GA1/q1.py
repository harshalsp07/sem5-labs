import time

# ---- Read Input ----
first_line = input().strip()
parts = first_line.split()
if len(parts) >= 2:
    R, C = int(parts[0]), int(parts[1])
elif len(parts) == 1 and len(parts[0]) == 2:
    R, C = int(parts[0][0]), int(parts[0][1])
else:
    raise ValueError("Invalid grid dimensions")

grid = []
for _ in range(R):
    grid.append(input().strip())

# Find start and goal positions
start_r = start_c = goal_r = goal_c = 0
for r in range(R):
    for c in range(C):
        if grid[r][c] == 'S':
            start_r, start_c = r, c
        elif grid[r][c] == 'G':
            goal_r, goal_c = r, c

# Successor order: UP -> RIGHT -> DOWN -> LEFT
dir_names = ["UP", "RIGHT", "DOWN", "LEFT"]
dr = [-1, 0, 1, 0]
dc = [0, 1, 0, -1]

# ---- BFS ----
start_time_bfs = time.perf_counter()
queue = [(start_r, start_c, [])]   # (row, col, path so far)
visited = set()
visited.add((start_r, start_c))
bfs_found = False
bfs_path = []
bfs_expanded = 0

while len(queue) > 0:
    r, c, path = queue.pop(0)
    bfs_expanded += 1

    if r == goal_r and c == goal_c:
        bfs_found = True
        bfs_path = path
        break

    for i in range(4):
        nr = r + dr[i]
        nc = c + dc[i]
        if 0 <= nr < R and 0 <= nc < C and grid[nr][nc] != '#' and (nr, nc) not in visited:
            visited.add((nr, nc))
            queue.append((nr, nc, path + [dir_names[i]]))

exec_time_bfs = time.perf_counter() - start_time_bfs

print("Algorithm: BFS")
if bfs_found:
    print("Path Found: Yes")
    print("Path:", " ".join(bfs_path))
    print("Number of Moves =", len(bfs_path))
    print("Nodes Expanded =", bfs_expanded)
    print(f"Execution Time = {exec_time_bfs:.6f} seconds")
else:
    print("Path Found: No")
    print(f"Execution Time = {exec_time_bfs:.6f} seconds")

print()

# ---- DFS ----
start_time_dfs = time.perf_counter()
stack = [(start_r, start_c, [])]
visited2 = set()
visited2.add((start_r, start_c))
dfs_found = False
dfs_path = []
dfs_expanded = 0

while len(stack) > 0:
    r, c, path = stack.pop()
    dfs_expanded += 1

    if r == goal_r and c == goal_c:
        dfs_found = True
        dfs_path = path
        break

    # Add in REVERSE order so that UP is popped first from stack
    for i in range(3, -1, -1):
        nr = r + dr[i]
        nc = c + dc[i]
        if 0 <= nr < R and 0 <= nc < C and grid[nr][nc] != '#' and (nr, nc) not in visited2:
            visited2.add((nr, nc))
            stack.append((nr, nc, path + [dir_names[i]]))

exec_time_dfs = time.perf_counter() - start_time_dfs

print("Algorithm: DFS")
if dfs_found:
    print("Path Found: Yes")
    print("Path:", " ".join(dfs_path))
    print("Number of Moves =", len(dfs_path))
    print("Nodes Expanded =", dfs_expanded)
    print(f"Execution Time = {exec_time_dfs:.6f} seconds")
else:
    print("Path Found: No")
    print(f"Execution Time = {exec_time_dfs:.6f} seconds")


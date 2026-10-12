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
    R, C = int(parts[0]), int(parts[1])
elif len(parts) == 1 and len(parts[0]) == 2:
    R, C = int(parts[0][0]), int(parts[0][1])
else:
    raise ValueError("Invalid dimensions")

grid = []
for _ in range(R):
    grid.append(list(parse_line()))

whose_turn = parse_line()   # "MAX" or "MIN"
depth = int(parse_line())

# Find positions of A, B, and all energy cells
a_pos = None
b_pos = None
energy_cells = set()
for r in range(R):
    for c in range(C):
        if grid[r][c] == 'A':
            a_pos = (r, c)
        elif grid[r][c] == 'B':
            b_pos = (r, c)
        elif grid[r][c] == 'E':
            energy_cells.add((r, c))

# Directions: UP, RIGHT, DOWN, LEFT
dir_names = ["UP", "RIGHT", "DOWN", "LEFT"]
dr = [-1, 0, 1, 0]
dc = [0, 1, 0, -1]

def get_moves(pos, board):
    """Get all valid moves from a position in normal order: UP, RIGHT, DOWN, LEFT"""
    moves = []
    r, c = pos
    for i in range(4):
        nr, nc = r + dr[i], c + dc[i]
        if 0 <= nr < R and 0 <= nc < C and board[nr][nc] != '#':
            moves.append((dir_names[i], (nr, nc)))
    return moves

def order_moves(moves, energy, use_heuristic):
    """Case 1: Normal move order. Case 2: Heuristic move order (energy collection first)."""
    if not use_heuristic:
        return moves
    collecting = [m for m in moves if m[1] in energy]
    non_collecting = [m for m in moves if m[1] not in energy]
    return collecting + non_collecting

def evaluate(a_pos, b_pos, a_score, b_score, energy, board):
    """Evaluation = MAX score - MIN score + positional advantage"""
    val = a_score - b_score

    # Positional advantage: closeness to nearest energy cell
    if len(energy) > 0:
        best_a = 999
        best_b = 999
        for (er, ec) in energy:
            dist_a = abs(a_pos[0] - er) + abs(a_pos[1] - ec)
            dist_b = abs(b_pos[0] - er) + abs(b_pos[1] - ec)
            if dist_a < best_a:
                best_a = dist_a
            if dist_b < best_b:
                best_b = dist_b
        val += (best_b - best_a)

    return val

# ---- Minimax ----
minimax_generated = 0
minimax_expanded = 0

def minimax(a_pos, b_pos, a_score, b_score, energy, is_max_turn, d):
    global minimax_generated, minimax_expanded
    minimax_expanded += 1

    # Terminal: depth 0 or no energy left
    if d == 0 or len(energy) == 0:
        return evaluate(a_pos, b_pos, a_score, b_score, energy, grid), None

    if is_max_turn:
        best_val = -999999
        best_move = None
        moves = get_moves(a_pos, grid)
        for move_name, new_pos in moves:
            minimax_generated += 1
            new_a_score = a_score
            new_energy = energy.copy()
            if new_pos in new_energy:
                new_a_score += 10
                new_energy.remove(new_pos)
            val, _ = minimax(new_pos, b_pos, new_a_score, b_score, new_energy, False, d - 1)
            if val > best_val:
                best_val = val
                best_move = move_name
        if best_move is None:
            return evaluate(a_pos, b_pos, a_score, b_score, energy, grid), None
        return best_val, best_move
    else:
        best_val = 999999
        best_move = None
        moves = get_moves(b_pos, grid)
        for move_name, new_pos in moves:
            minimax_generated += 1
            new_b_score = b_score
            new_energy = energy.copy()
            if new_pos in new_energy:
                new_b_score += 10
                new_energy.remove(new_pos)
            val, _ = minimax(a_pos, new_pos, a_score, new_b_score, new_energy, True, d - 1)
            if val < best_val:
                best_val = val
                best_move = move_name
        if best_move is None:
            return evaluate(a_pos, b_pos, a_score, b_score, energy, grid), None
        return best_val, best_move

start_time = time.perf_counter()
mm_score, mm_move = minimax(a_pos, b_pos, 0, 0, energy_cells.copy(), whose_turn == "MAX", depth)
mm_time = time.perf_counter() - start_time

print("Algorithm: Minimax")
print("Best Move:", mm_move)
print("Evaluation Score =", mm_score)
print("Nodes Generated =", minimax_generated)
print("Nodes Expanded =", minimax_expanded)
print("Search Depth =", depth)
print(f"Execution Time = {mm_time:.6f} seconds")

print()

# ---- Alpha-Beta Pruning Generic Runner ----
def run_alphabeta(use_heuristic=False):
    generated = 0
    expanded = 0
    pruned = 0

    def alphabeta(a_pos, b_pos, a_score, b_score, energy, is_max_turn, d, alpha, beta):
        nonlocal generated, expanded, pruned
        expanded += 1

        if d == 0 or len(energy) == 0:
            return evaluate(a_pos, b_pos, a_score, b_score, energy, grid), None

        if is_max_turn:
            best_val = -999999
            best_move = None
            moves = get_moves(a_pos, grid)
            moves = order_moves(moves, energy, use_heuristic)
            for move_name, new_pos in moves:
                generated += 1
                new_a_score = a_score
                new_energy = energy.copy()
                if new_pos in new_energy:
                    new_a_score += 10
                    new_energy.remove(new_pos)
                val, _ = alphabeta(new_pos, b_pos, new_a_score, b_score, new_energy, False, d - 1, alpha, beta)
                if val > best_val:
                    best_val = val
                    best_move = move_name
                if best_val > alpha:
                    alpha = best_val
                if alpha >= beta:
                    pruned += 1
                    break
            if best_move is None:
                return evaluate(a_pos, b_pos, a_score, b_score, energy, grid), None
            return best_val, best_move
        else:
            best_val = 999999
            best_move = None
            moves = get_moves(b_pos, grid)
            moves = order_moves(moves, energy, use_heuristic)
            for move_name, new_pos in moves:
                generated += 1
                new_b_score = b_score
                new_energy = energy.copy()
                if new_pos in new_energy:
                    new_b_score += 10
                    new_energy.remove(new_pos)
                val, _ = alphabeta(a_pos, new_pos, a_score, new_b_score, new_energy, True, d - 1, alpha, beta)
                if val < best_val:
                    best_val = val
                    best_move = move_name
                if best_val < beta:
                    beta = best_val
                if alpha >= beta:
                    pruned += 1
                    break
            if best_move is None:
                return evaluate(a_pos, b_pos, a_score, b_score, energy, grid), None
            return best_val, best_move

    t_start = time.perf_counter()
    score, move = alphabeta(a_pos, b_pos, 0, 0, energy_cells.copy(), whose_turn == "MAX", depth, -999999, 999999)
    elapsed = time.perf_counter() - t_start
    return score, move, generated, expanded, pruned, elapsed

# Case 1: Normal move order
ab_score1, ab_move1, ab_gen1, ab_exp1, ab_prun1, ab_time1 = run_alphabeta(use_heuristic=False)

print("Algorithm: Alpha-Beta (Normal Move Order)")
print("Best Move:", ab_move1)
print("Evaluation Score =", ab_score1)
print("Nodes Generated =", ab_gen1)
print("Nodes Expanded =", ab_exp1)
print("Nodes Pruned =", ab_prun1)
print("Search Depth =", depth)
print(f"Execution Time = {ab_time1:.6f} seconds")

print()

# Case 2: Heuristic move order (energy collection first)
ab_score2, ab_move2, ab_gen2, ab_exp2, ab_prun2, ab_time2 = run_alphabeta(use_heuristic=True)

print("Algorithm: Alpha-Beta (Heuristic Move Order)")
print("Best Move:", ab_move2)
print("Evaluation Score =", ab_score2)
print("Nodes Generated =", ab_gen2)
print("Nodes Expanded =", ab_exp2)
print("Nodes Pruned =", ab_prun2)
print("Search Depth =", depth)
print(f"Execution Time = {ab_time2:.6f} seconds")


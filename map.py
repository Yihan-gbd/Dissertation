import itertools, random
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

# ======== SMALL TEST ARENA: 1 sheet of A3, GRAY BORDER ========
INTERIOR_M   = 0.25       # 25 cm
BORDER_M     = 0.02       # 2 cm border
BORDER_GRAY  = 0.45       # 0=black, 1=white; ~45% gray gives a clear mid-range IR reading
GRID_N       = 5           # 5x5 grid
N_BLACK      = 4           # 4 black tiles
SEED         = 0
M_TO_IN      = 39.3701
MARGIN_M     = 0.003
# ===============================================================

tile = INTERIOR_M / GRID_N

edges = set()
for i in range(GRID_N):
    edges.update([(0,i),(GRID_N-1,i),(i,0),(i,GRID_N-1)])
all_cells = list(itertools.product(range(GRID_N), range(GRID_N)))
black_cells = set(random.Random(SEED).sample(
    [c for c in all_cells if c not in edges], N_BLACK))

outer = INTERIOR_M + 2 * BORDER_M
fig_side = outer + 2 * MARGIN_M

fig = plt.figure(figsize=(fig_side * M_TO_IN, fig_side * M_TO_IN))
ax = fig.add_axes([0, 0, 1, 1])
ax.set_xlim(0, fig_side)
ax.set_ylim(0, fig_side)
ax.set_aspect("equal")
ax.axis("off")
fig.patch.set_facecolor('white')

o = MARGIN_M
# Gray border ring
gray = str(BORDER_GRAY)
ax.add_patch(Rectangle((o, o), outer, outer, fc=gray, ec="none"))
# White interior
ax.add_patch(Rectangle((o+BORDER_M, o+BORDER_M), INTERIOR_M, INTERIOR_M, fc="white", ec="none"))
# Black tiles
ix0 = o + BORDER_M
for (r, c) in black_cells:
    x = ix0 + c * tile
    y = ix0 + (GRID_N - 1 - r) * tile
    ax.add_patch(Rectangle((x, y), tile, tile, fc="black", ec="none"))

fig.savefig("/home/claude/arena_gray.pdf")
fig.savefig("/home/claude/arena_gray.png", dpi=200)
plt.close(fig)

total_cm = outer * 100
print(f"=== Arena spec ===")
print(f"Total: {total_cm:.0f} cm x {total_cm:.0f} cm")
print(f"Interior: {INTERIOR_M*100:.0f} cm, Border: {BORDER_M*100:.0f} cm (gray={BORDER_GRAY})")
print(f"Tiles: {GRID_N}x{GRID_N} @ {tile*100:.1f} cm each")
print(f"Black tiles: {N_BLACK}")
print(f"\nLayout (X=black):")
for r in range(GRID_N):
    print(" ".join("X" if (r,c) in black_cells else "." for c in range(GRID_N)))

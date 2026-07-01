import itertools, random
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

# ======== A3 ARENA: rectangular, GRAY BORDER ========
# A3 paper: 29.7 cm × 42.0 cm
OUTER_W_M    = 0.297        # 29.7 cm
OUTER_H_M    = 0.420        # 42.0 cm
BORDER_M     = 0.05         # 5 cm border
BORDER_GRAY  = 0.45         # 0=black, 1=white; ~45% gray
INTERIOR_W   = OUTER_W_M - 2 * BORDER_M   # 19.7 cm
INTERIOR_H   = OUTER_H_M - 2 * BORDER_M   # 32.0 cm
GRID_COLS    = 5             # 5 columns
GRID_ROWS    = 8             # 8 rows  → 40 tiles total
# black:white = 0.25 → 8 black, 32 white
N_BLACK      = 8
SEED         = 0
M_TO_IN      = 39.3701
MARGIN_M     = 0.003
# =====================================================

tile_w = INTERIOR_W / GRID_COLS
tile_h = INTERIOR_H / GRID_ROWS

edges = set()
for c in range(GRID_COLS):
    edges.update([(0, c), (GRID_ROWS - 1, c)])
for r in range(GRID_ROWS):
    edges.update([(r, 0), (r, GRID_COLS - 1)])

all_cells = list(itertools.product(range(GRID_ROWS), range(GRID_COLS)))
black_cells = set(random.Random(SEED).sample(
    [c for c in all_cells if c not in edges], N_BLACK))

fig_w = OUTER_W_M + 2 * MARGIN_M
fig_h = OUTER_H_M + 2 * MARGIN_M

fig = plt.figure(figsize=(fig_w * M_TO_IN, fig_h * M_TO_IN))
ax = fig.add_axes([0, 0, 1, 1])
ax.set_xlim(0, fig_w)
ax.set_ylim(0, fig_h)
ax.set_aspect("equal")
ax.axis("off")
fig.patch.set_facecolor('white')

o = MARGIN_M
# Gray border ring
gray = str(BORDER_GRAY)
ax.add_patch(Rectangle((o, o), OUTER_W_M, OUTER_H_M, fc=gray, ec="none"))
# White interior
ax.add_patch(Rectangle((o + BORDER_M, o + BORDER_M), INTERIOR_W, INTERIOR_H, fc="white", ec="none"))
# Black tiles
ix0 = o + BORDER_M
iy0 = o + BORDER_M
for (r, c) in black_cells:
    x = ix0 + c * tile_w
    y = iy0 + (GRID_ROWS - 1 - r) * tile_h
    ax.add_patch(Rectangle((x, y), tile_w, tile_h, fc="black", ec="none"))

fig.savefig("arena_gray.pdf")
fig.savefig("arena_gray.png", dpi=200)
plt.close(fig)

print(f"=== Arena spec ===")
print(f"Total (A3): {OUTER_W_M*100:.1f} cm × {OUTER_H_M*100:.1f} cm")
print(f"Border: {BORDER_M*100:.0f} cm (gray={BORDER_GRAY})")
print(f"Interior: {INTERIOR_W*100:.1f} cm × {INTERIOR_H*100:.1f} cm")
print(f"Grid: {GRID_ROWS} rows × {GRID_COLS} cols = {GRID_ROWS*GRID_COLS} tiles")
print(f"Tile size: {tile_w*100:.2f} cm × {tile_h*100:.2f} cm")
print(f"Black tiles: {N_BLACK}, White tiles: {GRID_ROWS*GRID_COLS - N_BLACK}")
print(f"Black:White ratio: {N_BLACK/(GRID_ROWS*GRID_COLS - N_BLACK):.2f}")
print(f"\nLayout (X=black, .=white):")
for r in range(GRID_ROWS):
    print(" ".join("X" if (r, c) in black_cells else "." for c in range(GRID_COLS)))

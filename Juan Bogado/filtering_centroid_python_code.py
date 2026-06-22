import socket
import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import label, center_of_mass, find_objects, binary_opening, binary_closing
from scipy.ndimage import binary_erosion, binary_dilation, label

Xpoints = 7
Ypoints = 13
spacing = 2
originX = 0
originY = 0

grid = np.zeros((Xpoints, Ypoints), dtype=int)
#For software simulaton 
#HOST = "127.0.0.1"
#For hardware , labtop IP should be the same 
HOST = "192.168.125.2"
PORT = 5000

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print("Waiting for RAPID connection...")
conn, addr = server.accept()
print("Robot connected:", addr)

# -----------------------------
# RECEIVE GRID
# -----------------------------
running = True
while running:
    data = conn.recv(1024).decode()
    lines = data.strip().split("\n")

    for line in lines:
        if line == "END":
            running = False
            break

        parts = line.split(",")
        if len(parts) != 3:
            continue

        i = int(parts[0]) - 1
        j = int(parts[1]) - 1
        value = int(parts[2])
        grid[i, j] = value

print("\nRaw Grid matrix result:\n", grid)

# -----------------------------
# FILTERING STAGE (Minimum-change rectangle fitting)
# -----------------------------
binary = (grid == 0)

# Step 1: break weak connections
separated = binary_erosion(binary, structure=np.ones((2,2)))

# Step 2: label separated clusters
labeled_array, num_features = label(separated)

filtered_grid = np.ones_like(grid)

for cluster_id in range(1, num_features + 1):
    mask = (labeled_array == cluster_id)

    if np.sum(mask) < 3:
        continue

    # Step 3: recover original size locally
    recovered = binary_dilation(mask, structure=np.ones((2,2)))

    # Now apply your MIN-COST RECTANGLE here
    slc = find_objects(recovered)[0]
    r0, r1 = slc[0].start, slc[0].stop
    c0, c1 = slc[1].start, slc[1].stop

    sub = (grid[r0:r1, c0:c1] == 0).astype(int)

    H, W = sub.shape

    best_cost = float("inf")
    best_rect = None

    for top in range(H):
        for bottom in range(top+1, H+1):
            for left in range(W):
                for right in range(left+1, W+1):

                    rect_area = (bottom-top)*(right-left)
                    rect = sub[top:bottom, left:right]

                    zeros_inside = np.sum(rect)
                    ones_inside = rect_area - zeros_inside

                    outside = sub.copy()
                    outside[top:bottom, left:right] = 0
                    zeros_outside = np.sum(outside)

                    cost = ones_inside + zeros_outside

                    if cost < best_cost:
                        best_cost = cost
                        best_rect = (top, bottom, left, right)

    top, bottom, left, right = best_rect
    filtered_grid[r0+top:r0+bottom, c0+left:c0+right] = 0

print("\nFiltered Grid matrix result:\n", filtered_grid)

# -----------------------------
# DETECT CLUSTERS
# -----------------------------
labeled_array, num_features = label(filtered_grid == 0)
objects = find_objects(labeled_array)

center_grid = np.array(filtered_grid.shape) / 2
candidates = []

for cluster_id in range(1, num_features + 1):
    mask = (labeled_array == cluster_id)
    size = np.sum(mask)

    if size < 3:   # ignore tiny clusters (noise)
        continue

    # Centroid
    row, col = center_of_mass(mask)
    targetX = originX + (row + 0.5) * spacing
    targetY = originY + (col + 0.5) * spacing

    # Bounding box
    slc = objects[cluster_id - 1]
    height = slc[0].stop - slc[0].start
    width  = slc[1].stop - slc[1].start
    bbox_area = height * width

    compactness = size / bbox_area
    dist = np.linalg.norm(np.array([row, col]) - center_grid)

    score = (3.0 * size) + (2.0 * compactness) - (1.0 * dist)
    candidates.append((score, targetX, targetY, size, compactness, dist))

# -----------------------------
# SORT BY CONFIDENCE
# -----------------------------
candidates.sort(reverse=True, key=lambda x: x[0])

print("\nRanked Port Candidates:")
for rank, c in enumerate(candidates, 1):
    print(f"Rank {rank} | Score={c[0]:.2f} | Size={c[3]} | Compact={c[4]:.2f}")

# -----------------------------
# SEND TO RAPID
# -----------------------------
message = f"N={len(candidates)};"
for i, c in enumerate(candidates):
    message += f"X={c[1]:.2f};Y={c[2]:.2f}"
    if i < len(candidates) - 1:
        message += "|"

print("\nSending ranked ports to RAPID:")
print(message)
conn.sendall(message.encode())

conn.close()
server.close()

# -----------------------------
# PLOT WITH REAL COORDINATES
# -----------------------------
fig, ax = plt.subplots()
ax.imshow(
    filtered_grid,
    cmap='Greys',
    origin='upper',
    extent=[originY, originY + spacing * Ypoints,
            originX + spacing * Xpoints, originX]
)

for rank, c in enumerate(candidates, 1):
    x_mm = c[1]
    y_mm = c[2]
    ax.scatter(y_mm, x_mm, s=140, label=f"Rank {rank}")
    ax.text(y_mm, x_mm, f"{rank}", ha='center', va='center',
            fontsize=11, weight='bold')

ax.set_title("Ranked Port Candidates (Filtered Grid)")
ax.set_xlabel("Y (mm)")
ax.set_ylabel("X (mm)")
ax.set_aspect('equal')
ax.grid(True)
ax.legend()
plt.show()

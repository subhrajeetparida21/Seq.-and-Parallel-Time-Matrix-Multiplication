import pandas as pd
import matplotlib.pyplot as plt
import os

# --------------------------------------------------
# Create the 'result' folder if it does not exist
# This prevents FileNotFoundError while saving graphs
# --------------------------------------------------
os.makedirs("result", exist_ok=True)

# --------------------------------------------------
# Read performance data from CSV file
# The CSV should contain columns:
# size, seq_time, par_time, speedup
# --------------------------------------------------
df = pd.read_csv("results.csv")

# --------------------------------------------------
# GRAPH 1: Sequential vs Parallel Execution Time
# --------------------------------------------------
plt.figure()  # Create a new figure window

# Plot sequential execution time
plt.plot(df["size"], df["seq_time"], marker="o", label="Sequential")

# Plot parallel execution time
plt.plot(df["size"], df["par_time"], marker="o", label="Parallel")

# Add labels and title
plt.xlabel("Matrix Size (n)")
plt.ylabel("Time (seconds)")
plt.title("Sequential vs Parallel Execution Time")

# Show legend and grid
plt.legend()
plt.grid(True)

# Save the figure inside the result folder
plt.savefig("result/time_vs_size.png", dpi=200)


# --------------------------------------------------
# GRAPH 2: Speedup vs Matrix Size
# Speedup = Sequential Time / Parallel Time
# --------------------------------------------------
plt.figure()  # New figure for second graph

# Plot speedup values
plt.plot(df["size"], df["speedup"], marker="o")

# Add labels and title
plt.xlabel("Matrix Size (n)")
plt.ylabel("Speedup")
plt.title("Speedup vs Matrix Size")

# Show grid for better readability
plt.grid(True)

# Save the figure
plt.savefig("result/speedup_vs_size.png", dpi=200)


# --------------------------------------------------
# Confirmation message
# --------------------------------------------------
print("Graphs saved inside result folder.")

import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("result/results.csv")

plt.figure()
plt.plot(df["size"], df["seq_time"], marker="o", label="Sequential")
plt.plot(df["size"], df["par_time"], marker="o", label="Parallel")
plt.xlabel("Matrix Size (n)")
plt.ylabel("Time (seconds)")
plt.title("Sequential vs Parallel Execution Time")
plt.legend()
plt.grid(True)
plt.savefig("result/time_vs_size.png", dpi=200)

plt.figure()
plt.plot(df["size"], df["speedup"], marker="o")
plt.xlabel("Matrix Size (n)")
plt.ylabel("Speedup")
plt.title("Speedup vs Matrix Size")
plt.grid(True)
plt.savefig("result/speedup_vs_size.png", dpi=200)

print("Graphs saved inside result folder.")

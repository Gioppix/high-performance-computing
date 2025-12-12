# %% [markdown]
# # Benchmark Analysis
# Run the C++ program, copy the CSV output from the terminal, and paste it into the `raw_csv` variable below.

# %%
import io

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

# Set the visual style
sns.set_theme(style="whitegrid")
plt.rcParams["figure.figsize"] = [12, 6]

# %%
# PASTE YOUR C++ TERMINAL OUTPUT BETWEEN THE TRIPLE QUOTES BELOW
raw_csv = """
Nodes,Ratio,Edges,Comps,Threads,Algorithm,Time,Speedup,Status
50000,0.1,5000,45000,1,Serial,0.00103,1.52,OK
50000,0.10,5000,45000,1,Parallel2,0.00153,1.03,OK
50000,0.10,5000,45000,1,Parallel3,0.00144,1.09,OK
50000,0.10,5000,45000,2,Parallel2,0.00099,1.59,OK
50000,0.10,5000,45000,2,Parallel3,0.00078,2.00,OK
50000,0.10,5000,45000,7,Parallel2,0.00095,1.64,OK
50000,0.10,5000,45000,7,Parallel3,0.00061,2.58,OK
50000,0.10,5000,45000,12,Parallel2,0.00086,1.82,OK
50000,0.10,5000,45000,12,Parallel3,0.00061,2.55,OK
50000,0.50,25000,25000,1,Serial,0.00051,3.37,OK
50000,0.50,25000,25000,1,Parallel2,0.00194,0.89,OK
50000,0.50,25000,25000,1,Parallel3,0.00197,0.87,OK
50000,0.50,25000,25000,2,Parallel2,0.00147,1.17,OK
50000,0.50,25000,25000,2,Parallel3,0.00133,1.29,OK
50000,0.50,25000,25000,7,Parallel2,0.00146,1.18,OK
50000,0.50,25000,25000,7,Parallel3,0.00110,1.56,OK
50000,0.50,25000,25000,12,Parallel2,0.00153,1.12,OK
50000,0.50,25000,25000,12,Parallel3,0.00121,1.42,OK
50000,1.00,50000,33,1,Serial,0.00059,3.83,OK
50000,1.00,50000,33,1,Parallel2,0.00242,0.94,OK
50000,1.00,50000,33,1,Parallel3,0.00243,0.93,OK
50000,1.00,50000,33,2,Parallel2,0.00199,1.14,OK
50000,1.00,50000,33,2,Parallel3,0.00170,1.34,OK
50000,1.00,50000,33,7,Parallel2,0.00258,0.88,OK
50000,1.00,50000,33,7,Parallel3,0.00143,1.59,OK
50000,1.00,50000,33,12,Parallel2,0.00238,0.95,OK
50000,1.00,50000,33,12,Parallel3,0.00148,1.53,OK
50000,2.00,100000,1,1,Serial,0.00089,3.23,OK
50000,2.00,100000,1,1,Parallel2,0.00309,0.93,OK
50000,2.00,100000,1,1,Parallel3,0.00307,0.94,OK
50000,2.00,100000,1,2,Parallel2,0.00262,1.10,OK
50000,2.00,100000,1,2,Parallel3,0.00242,1.19,OK
50000,2.00,100000,1,7,Parallel2,0.00239,1.21,OK
50000,2.00,100000,1,7,Parallel3,0.00165,1.75,OK
50000,2.00,100000,1,12,Parallel2,0.00192,1.50,OK
50000,2.00,100000,1,12,Parallel3,0.00162,1.77,OK
50000,4.00,200000,1,1,Serial,0.00105,3.53,OK
50000,4.00,200000,1,1,Parallel2,0.00388,0.96,OK
50000,4.00,200000,1,1,Parallel3,0.00377,0.99,OK
50000,4.00,200000,1,2,Parallel2,0.00329,1.13,OK
50000,4.00,200000,1,2,Parallel3,0.00336,1.10,OK
50000,4.00,200000,1,7,Parallel2,0.00224,1.66,OK
50000,4.00,200000,1,7,Parallel3,0.00206,1.80,OK
50000,4.00,200000,1,12,Parallel2,0.00239,1.56,OK
50000,4.00,200000,1,12,Parallel3,0.00193,1.92,OK
500000,0.10,50000,450001,1,Serial,0.00692,1.92,OK
500000,0.10,50000,450001,1,Parallel2,0.01439,0.92,OK
500000,0.10,50000,450001,1,Parallel3,0.01316,1.01,OK
500000,0.10,50000,450001,2,Parallel2,0.00867,1.53,OK
500000,0.10,50000,450001,2,Parallel3,0.00808,1.65,OK
500000,0.10,50000,450001,7,Parallel2,0.00467,2.85,OK
500000,0.10,50000,450001,7,Parallel3,0.00420,3.17,OK
500000,0.10,50000,450001,12,Parallel2,0.00477,2.79,OK
500000,0.10,50000,450001,12,Parallel3,0.00418,3.18,OK
500000,0.50,250000,250003,1,Serial,0.00809,3.04,OK
500000,0.50,250000,250003,1,Parallel2,0.03133,0.79,OK
500000,0.50,250000,250003,1,Parallel3,0.02835,0.87,OK
500000,0.50,250000,250003,2,Parallel2,0.01870,1.32,OK
500000,0.50,250000,250003,2,Parallel3,0.01625,1.52,OK
500000,0.50,250000,250003,7,Parallel2,0.01155,2.13,OK
500000,0.50,250000,250003,7,Parallel3,0.00998,2.47,OK
500000,0.50,250000,250003,12,Parallel2,0.01065,2.31,OK
500000,0.50,250000,250003,12,Parallel3,0.00940,2.62,OK
500000,1.00,500000,33,1,Serial,0.01675,2.60,OK
500000,1.00,500000,33,1,Parallel2,0.04461,0.98,OK
500000,1.00,500000,33,1,Parallel3,0.04334,1.00,OK
500000,1.00,500000,33,2,Parallel2,0.02708,1.61,OK
500000,1.00,500000,33,2,Parallel3,0.02603,1.67,OK
500000,1.00,500000,33,7,Parallel2,0.01503,2.90,OK
500000,1.00,500000,33,7,Parallel3,0.01244,3.50,OK
500000,1.00,500000,33,12,Parallel2,0.01319,3.30,OK
500000,1.00,500000,33,12,Parallel3,0.01102,3.95,OK
500000,2.00,1000000,1,1,Serial,0.02901,2.18,OK
500000,2.00,1000000,1,1,Parallel2,0.06828,0.92,OK
500000,2.00,1000000,1,1,Parallel3,0.06833,0.92,OK
500000,2.00,1000000,1,2,Parallel2,0.03980,1.59,OK
500000,2.00,1000000,1,2,Parallel3,0.04008,1.57,OK
500000,2.00,1000000,1,7,Parallel2,0.02111,2.99,OK
500000,2.00,1000000,1,7,Parallel3,0.01845,3.42,OK
500000,2.00,1000000,1,12,Parallel2,0.01758,3.59,OK
500000,2.00,1000000,1,12,Parallel3,0.01558,4.05,OK
500000,4.00,2000000,1,1,Serial,0.03629,2.54,OK
500000,4.00,2000000,1,1,Parallel2,0.09151,1.01,OK
500000,4.00,2000000,1,1,Parallel3,0.09120,1.01,OK
500000,4.00,2000000,1,2,Parallel2,0.05444,1.69,OK
500000,4.00,2000000,1,2,Parallel3,0.05550,1.66,OK
500000,4.00,2000000,1,7,Parallel2,0.02710,3.40,OK
500000,4.00,2000000,1,7,Parallel3,0.02509,3.67,OK
500000,4.00,2000000,1,12,Parallel2,0.02196,4.19,OK
500000,4.00,2000000,1,12,Parallel3,0.02039,4.51,OK
5000000,0.10,500000,4500001,1,Serial,0.10865,1.66,OK
5000000,0.10,500000,4500001,1,Parallel2,0.18809,0.96,OK
5000000,0.10,500000,4500001,1,Parallel3,0.17146,1.05,OK
5000000,0.10,500000,4500001,2,Parallel2,0.10689,1.69,OK
5000000,0.10,500000,4500001,2,Parallel3,0.10038,1.80,OK
5000000,0.10,500000,4500001,7,Parallel2,0.05688,3.17,OK
5000000,0.10,500000,4500001,7,Parallel3,0.05062,3.56,OK
5000000,0.10,500000,4500001,12,Parallel2,0.05115,3.53,OK
5000000,0.10,500000,4500001,12,Parallel3,0.04649,3.88,OK
5000000,0.50,2500000,2500003,1,Serial,0.24014,2.38,OK
5000000,0.50,2500000,2500003,1,Parallel2,0.65377,0.87,OK
5000000,0.50,2500000,2500003,1,Parallel3,0.57240,1.00,OK
5000000,0.50,2500000,2500003,2,Parallel2,0.37156,1.54,OK
5000000,0.50,2500000,2500003,2,Parallel3,0.32038,1.78,OK
5000000,0.50,2500000,2500003,7,Parallel2,0.16173,3.53,OK
5000000,0.50,2500000,2500003,7,Parallel3,0.13561,4.21,OK
5000000,0.50,2500000,2500003,12,Parallel2,0.13646,4.19,OK
5000000,0.50,2500000,2500003,12,Parallel3,0.11471,4.98,OK
5000000,1.00,5000000,45,1,Serial,0.52220,1.70,OK
5000000,1.00,5000000,45,1,Parallel2,0.89027,1.00,OK
5000000,1.00,5000000,45,1,Parallel3,0.90323,0.99,OK
5000000,1.00,5000000,45,2,Parallel2,0.49179,1.81,OK
5000000,1.00,5000000,45,2,Parallel3,0.48401,1.84,OK
5000000,1.00,5000000,45,7,Parallel2,0.19573,4.55,OK
5000000,1.00,5000000,45,7,Parallel3,0.18225,4.88,OK
5000000,1.00,5000000,45,12,Parallel2,0.16063,5.54,OK
5000000,1.00,5000000,45,12,Parallel3,0.14519,6.13,OK
5000000,2.00,10000000,1,1,Serial,0.53655,2.61,OK
5000000,2.00,10000000,1,1,Parallel2,1.45484,0.96,OK
5000000,2.00,10000000,1,1,Parallel3,1.49540,0.94,OK
5000000,2.00,10000000,1,2,Parallel2,0.77734,1.80,OK
5000000,2.00,10000000,1,2,Parallel3,0.77551,1.81,OK
5000000,2.00,10000000,1,7,Parallel2,0.30408,4.61,OK
5000000,2.00,10000000,1,7,Parallel3,0.27912,5.02,OK
5000000,2.00,10000000,1,12,Parallel2,0.28130,4.98,OK
5000000,2.00,10000000,1,12,Parallel3,0.21886,6.40,OK
5000000,4.00,20000000,1,1,Serial,0.58001,2.88,OK
5000000,4.00,20000000,1,1,Parallel2,1.65227,1.01,OK
5000000,4.00,20000000,1,1,Parallel3,1.67908,1.00,OK
5000000,4.00,20000000,1,2,Parallel2,0.95933,1.74,OK
5000000,4.00,20000000,1,2,Parallel3,0.97615,1.71,OK
5000000,4.00,20000000,1,7,Parallel2,0.38917,4.30,OK
5000000,4.00,20000000,1,7,Parallel3,0.35115,4.76,OK
5000000,4.00,20000000,1,12,Parallel2,0.29169,5.73,OK
5000000,4.00,20000000,1,12,Parallel3,0.26746,6.25,OK
"""

# %%
# Parse the CSV data
# We strip whitespace just in case the copy-paste was messy
try:
    df = pd.read_csv(io.StringIO(raw_csv.strip()))

    # Ensure Serial is sorted first for legend consistency
    algos = sorted(df["Algorithm"].unique())
    if "Serial" in algos:
        algos.remove("Serial")
        algos.insert(0, "Serial")

    print(f"Successfully loaded {len(df)} rows.")
    print(df.head())
except Exception as e:
    print(f"Error parsing CSV: {e}")
    print("Make sure you pasted the C++ output exactly as it appears in the terminal.")

# %%
# 1. SPEEDUP ANALYSIS
# Speedup is now calculated relative to 1-thread Parallel3 baseline, not Serial
# This shows how well algorithms scale with threads

# Filter to only include parallel algorithms (Serial is shown for reference but has different baseline)
parallel_df = df[df["Algorithm"] != "Serial"].copy()

if not parallel_df.empty:
    g = sns.relplot(
        data=parallel_df,
        x="Ratio",
        y="Speedup",
        hue="Algorithm",
        style="Threads",
        col="Nodes",
        kind="line",
        markers=True,
        dashes=True,
        height=5,
        aspect=0.8,
        linewidth=2.5,
        palette="tab10",
    )

    g.set_axis_labels("Edge/Node Ratio", "Speedup (vs 1-thread baseline)")
    g.set_titles("Nodes: {col_name}")

    # Add reference line at 1.0
    for ax in g.axes.flat:
        ax.axhline(1, color="gray", linestyle="--", alpha=0.5)

    # Update x-axis labels to include component counts
    comp_lookup = df.groupby(["Nodes", "Ratio"])["Comps"].first()
    unique_ratios = sorted(parallel_df["Ratio"].unique())

    for ax in g.axes.flat:
        try:
            # Extract node count from title "Nodes: <count>"
            node_count = int(ax.get_title().split(":")[1].strip())

            new_labels = []
            for r in unique_ratios:
                if (node_count, r) in comp_lookup.index:
                    c = comp_lookup.loc[(node_count, r)]
                    new_labels.append(f"{r}\n({int(c)})")
                else:
                    new_labels.append(str(r))

            ax.set_xscale("log")
            ax.set_xticks(unique_ratios)
            ax.set_xticklabels(new_labels, fontsize=8, rotation=45)
        except (IndexError, ValueError):
            pass

    plt.subplots_adjust(top=0.85)
    g.fig.suptitle("Parallel Speedup vs. Graph Density (vs 1-thread)", fontsize=16)
    plt.show()
else:
    print("No parallel algorithm data found.")

# %%
# 2. ABSOLUTE PERFORMANCE (Log-Log Scale)
# Total execution time scaling with the number of edges.

plt.figure(figsize=(12, 7))

# We want to see how time grows with edges.
# We use style='Nodes' to distinguish the node buckets.
ax = sns.lineplot(
    data=df,
    x="Edges",
    y="Time",
    hue="Algorithm",
    style="Nodes",
    markers=True,
    hue_order=algos,
    linewidth=2,
    palette="tab10",
)

ax.set_xscale("log")
ax.set_yscale("log")
plt.title("Execution Time vs. Graph Size (Log-Log Scale)", fontsize=16)
plt.xlabel("Number of Edges", fontsize=12)
plt.ylabel("Time (seconds)", fontsize=12)
plt.grid(True, which="minor", ls=":", alpha=0.3)
plt.grid(True, which="major", ls="--", alpha=0.5)
plt.show()

# %%
# 3. BEST TRADEOFF HEATMAP
# Visualizing the maximum speedup achieved for every Node/Ratio configuration
# (ignoring which specific algorithm achieved it, effectively showing 'potential')

# Filter out Serial to see actual speedups
parallel_only_df = df[df["Algorithm"] != "Serial"]

if not parallel_only_df.empty:
    pivot_df = parallel_only_df.pivot_table(
        index="Nodes", columns="Ratio", values="Speedup", aggfunc="max"
    )

    plt.figure(figsize=(10, 6))
    sns.heatmap(
        pivot_df,
        annot=True,
        fmt=".2f",
        cmap="viridis",
        linewidths=0.5,
        cbar_kws={"label": "Max Speedup Factor"},
    )
    plt.title("Best Parallel Speedup Achieved (vs 1-thread)", fontsize=16)
    plt.xlabel("Edge Ratio")
    plt.ylabel("Node Count")
    plt.show()
else:
    print("No parallel data found to generate heatmap.")

# %%
# 4. CONNECTED COMPONENTS ANALYSIS
# Visualizing how connectivity changes with density (Phase Transition)

plt.figure(figsize=(10, 6))

# We only need one entry per graph configuration (Nodes + Ratio)
# filtering by 'Serial' usually gives us the canonical answer
connectivity_df = df[df["Algorithm"] == "Serial"].copy()

# If Serial is missing (e.g. user only ran parallel), drop duplicates
if connectivity_df.empty:
    connectivity_df = df.drop_duplicates(subset=["Nodes", "Ratio"]).copy()

# Treat Nodes as categorical for better coloring
connectivity_df["Nodes"] = connectivity_df["Nodes"].astype(str)

sns.lineplot(
    data=connectivity_df,
    x="Ratio",
    y="Comps",
    hue="Nodes",
    style="Nodes",
    palette="tab10",
    markers=True,
    dashes=False,
    linewidth=2,
)

# Annotate points with component counts
for _, row in connectivity_df.iterrows():
    plt.text(
        row["Ratio"],
        row["Comps"],
        f"{int(row['Comps'])}",
        fontsize=8,
        ha="center",
        va="bottom",
        fontweight="bold",
    )

plt.yscale("log")
plt.title("Connected Components vs. Graph Density", fontsize=16)
plt.xlabel("Edge/Node Ratio", fontsize=12)
plt.ylabel("Count of Components (Log Scale)", fontsize=12)
plt.grid(True, which="minor", ls=":", alpha=0.3)
plt.grid(True, which="major", ls="--", alpha=0.5)
plt.show()

# %%
# 5. SCALABILITY ANALYSIS
# Speedup vs Threads for a large graph configuration

plt.figure(figsize=(10, 6))

# Filter for a representative large case (e.g. max nodes, ratio 1.0)
max_nodes = df["Nodes"].max()
scalability_df = df[
    (df["Nodes"] == max_nodes) & (df["Ratio"] == 1.0) & (df["Algorithm"] != "Serial")
]

if not scalability_df.empty:
    sns.lineplot(
        data=scalability_df,
        x="Threads",
        y="Speedup",
        hue="Algorithm",
        markers=True,
        linewidth=2.5,
    )
    plt.title(f"Scalability (Nodes={max_nodes}, Ratio=1.0)", fontsize=16)
    plt.xlabel("Number of Threads", fontsize=12)
    plt.ylabel("Speedup (vs 1-thread baseline)", fontsize=12)
    plt.grid(True)

    # Add ideal speedup line (relative to 1-thread)
    max_threads = scalability_df["Threads"].max()
    plt.plot([1, max_threads], [1, max_threads], "k--", alpha=0.3, label="Ideal")
    plt.legend()
    plt.show()
else:
    print("No data found for scalability analysis.")

# %%
# 6. ALGORITHM COMPARISON AT DIFFERENT THREAD COUNTS
# Compare algorithms across different thread counts for all graph sizes

for thread_count in sorted(df[df["Algorithm"] != "Serial"]["Threads"].unique()):
    thread_df = df[(df["Threads"] == thread_count) | (df["Algorithm"] == "Serial")]

    plt.figure(figsize=(12, 6))

    ax = sns.lineplot(
        data=thread_df,
        x="Edges",
        y="Time",
        hue="Algorithm",
        style="Nodes",
        markers=True,
        linewidth=2,
        palette="tab10",
    )

    ax.set_xscale("log")
    ax.set_yscale("log")
    plt.title(f"Algorithm Performance Comparison (Threads={thread_count})", fontsize=16)
    plt.xlabel("Number of Edges", fontsize=12)
    plt.ylabel("Time (seconds)", fontsize=12)
    plt.grid(True, which="minor", ls=":", alpha=0.3)
    plt.grid(True, which="major", ls="--", alpha=0.5)
    plt.show()

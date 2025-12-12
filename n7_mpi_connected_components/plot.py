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
Nodes,Ratio,Edges,Comps,Threads,Algorithm,Time,Speedup,SerialSpeedup,Status
50000,0.1,5000,45000,1,Serial,0.00102,1.00,1.00,OK
50000,0.10,5000,45000,1,Paper1,0.00045,1.01,2.26,OK
50000,0.10,5000,45000,2,Paper1,0.00037,1.24,2.76,OK
50000,0.10,5000,45000,6,Paper1,0.00043,1.07,2.39,OK
50000,0.10,5000,45000,12,Paper1,0.00059,0.78,1.73,OK
50000,0.10,5000,45000,1,Parallel3,0.00114,1.10,0.90,OK
50000,0.10,5000,45000,2,Parallel3,0.00086,1.44,1.18,OK
50000,0.10,5000,45000,6,Parallel3,0.00083,1.51,1.23,OK
50000,0.10,5000,45000,12,Parallel3,0.00083,1.51,1.24,OK
50000,0.10,5000,45000,1,SV_improvement,0.00054,1.11,1.89,OK
50000,0.10,5000,45000,2,SV_improvement,0.00054,1.11,1.90,OK
50000,0.10,5000,45000,6,SV_improvement,0.00063,0.95,1.63,OK
50000,0.10,5000,45000,12,SV_improvement,0.00083,0.72,1.23,OK
50000,0.50,25000,25000,1,Serial,0.00046,1.00,1.00,OK
50000,0.50,25000,25000,1,Paper1,0.00040,1.12,1.15,OK
50000,0.50,25000,25000,2,Paper1,0.00047,0.94,0.97,OK
50000,0.50,25000,25000,6,Paper1,0.00052,0.87,0.89,OK
50000,0.50,25000,25000,12,Paper1,0.00077,0.58,0.60,OK
50000,0.50,25000,25000,1,Parallel3,0.00160,0.99,0.29,OK
50000,0.50,25000,25000,2,Parallel3,0.00120,1.32,0.38,OK
50000,0.50,25000,25000,6,Parallel3,0.00122,1.30,0.38,OK
50000,0.50,25000,25000,12,Parallel3,0.00123,1.29,0.37,OK
50000,0.50,25000,25000,1,SV_improvement,0.00072,1.02,0.64,OK
50000,0.50,25000,25000,2,SV_improvement,0.00069,1.07,0.67,OK
50000,0.50,25000,25000,6,SV_improvement,0.00076,0.97,0.61,OK
50000,0.50,25000,25000,12,SV_improvement,0.00099,0.74,0.46,OK
50000,1.00,50000,33,1,Serial,0.00056,1.00,1.00,OK
50000,1.00,50000,33,1,Paper1,0.00282,1.03,0.20,OK
50000,1.00,50000,33,2,Paper1,0.00195,1.49,0.29,OK
50000,1.00,50000,33,6,Paper1,0.00156,1.87,0.36,OK
50000,1.00,50000,33,12,Paper1,0.00199,1.46,0.28,OK
50000,1.00,50000,33,1,Parallel3,0.00229,0.93,0.24,OK
50000,1.00,50000,33,2,Parallel3,0.00158,1.35,0.35,OK
50000,1.00,50000,33,6,Parallel3,0.00148,1.44,0.38,OK
50000,1.00,50000,33,12,Parallel3,0.00159,1.35,0.35,OK
50000,1.00,50000,33,1,SV_improvement,0.00133,0.98,0.42,OK
50000,1.00,50000,33,2,SV_improvement,0.00112,1.17,0.50,OK
50000,1.00,50000,33,6,SV_improvement,0.00103,1.27,0.54,OK
50000,1.00,50000,33,12,SV_improvement,0.00115,1.14,0.49,OK
50000,2.00,100000,1,1,Serial,0.00090,1.00,1.00,OK
50000,2.00,100000,1,1,Paper1,0.00277,1.04,0.32,OK
50000,2.00,100000,1,2,Paper1,0.00193,1.50,0.46,OK
50000,2.00,100000,1,6,Paper1,0.00141,2.05,0.63,OK
50000,2.00,100000,1,12,Paper1,0.00151,1.92,0.59,OK
50000,2.00,100000,1,1,Parallel3,0.00301,0.92,0.30,OK
50000,2.00,100000,1,2,Parallel3,0.00238,1.17,0.38,OK
50000,2.00,100000,1,6,Parallel3,0.00176,1.58,0.51,OK
50000,2.00,100000,1,12,Parallel3,0.00161,1.72,0.55,OK
50000,2.00,100000,1,1,SV_improvement,0.00170,1.02,0.53,OK
50000,2.00,100000,1,2,SV_improvement,0.00140,1.24,0.64,OK
50000,2.00,100000,1,6,SV_improvement,0.00120,1.45,0.75,OK
50000,2.00,100000,1,12,SV_improvement,0.00122,1.42,0.73,OK
500000,0.10,50000,450001,1,Serial,0.00673,1.00,1.00,OK
500000,0.10,50000,450001,1,Paper1,0.00319,1.07,2.11,OK
500000,0.10,50000,450001,2,Paper1,0.00208,1.64,3.24,OK
500000,0.10,50000,450001,6,Paper1,0.00119,2.87,5.68,OK
500000,0.10,50000,450001,12,Paper1,0.00121,2.82,5.56,OK
500000,0.10,50000,450001,1,Parallel3,0.01282,1.03,0.53,OK
500000,0.10,50000,450001,2,Parallel3,0.00781,1.68,0.86,OK
500000,0.10,50000,450001,6,Parallel3,0.00450,2.92,1.50,OK
500000,0.10,50000,450001,12,Parallel3,0.00433,3.04,1.55,OK
500000,0.10,50000,450001,1,SV_improvement,0.00338,0.98,1.99,OK
500000,0.10,50000,450001,2,SV_improvement,0.00228,1.46,2.95,OK
500000,0.10,50000,450001,6,SV_improvement,0.00151,2.21,4.47,OK
500000,0.10,50000,450001,12,SV_improvement,0.00150,2.22,4.49,OK
500000,0.50,250000,250003,1,Serial,0.01039,1.00,1.00,OK
500000,0.50,250000,250003,1,Paper1,0.00495,1.01,2.10,OK
500000,0.50,250000,250003,2,Paper1,0.00371,1.35,2.80,OK
500000,0.50,250000,250003,6,Paper1,0.00239,2.09,4.34,OK
500000,0.50,250000,250003,12,Paper1,0.00210,2.38,4.94,OK
500000,0.50,250000,250003,1,Parallel3,0.03056,1.04,0.34,OK
500000,0.50,250000,250003,2,Parallel3,0.01847,1.72,0.56,OK
500000,0.50,250000,250003,6,Parallel3,0.01089,2.92,0.95,OK
500000,0.50,250000,250003,12,Parallel3,0.00937,3.39,1.11,OK
500000,0.50,250000,250003,1,SV_improvement,0.00513,1.02,2.03,OK
500000,0.50,250000,250003,2,SV_improvement,0.00312,1.68,3.33,OK
500000,0.50,250000,250003,6,SV_improvement,0.00264,1.99,3.94,OK
500000,0.50,250000,250003,12,SV_improvement,0.00235,2.24,4.43,OK
500000,1.00,500000,33,1,Serial,0.02139,1.00,1.00,OK
500000,1.00,500000,33,1,Paper1,0.03850,1.01,0.56,OK
500000,1.00,500000,33,2,Paper1,0.02157,1.80,0.99,OK
500000,1.00,500000,33,6,Paper1,0.01090,3.56,1.96,OK
500000,1.00,500000,33,12,Paper1,0.00967,4.02,2.21,OK
500000,1.00,500000,33,1,Parallel3,0.05050,1.04,0.42,OK
500000,1.00,500000,33,2,Parallel3,0.02672,1.97,0.80,OK
500000,1.00,500000,33,6,Parallel3,0.01339,3.94,1.60,OK
500000,1.00,500000,33,12,Parallel3,0.01131,4.66,1.89,OK
500000,1.00,500000,33,1,SV_improvement,0.01362,1.02,1.57,OK
500000,1.00,500000,33,2,SV_improvement,0.00878,1.59,2.44,OK
500000,1.00,500000,33,6,SV_improvement,0.00632,2.21,3.38,OK
500000,1.00,500000,33,12,SV_improvement,0.00505,2.76,4.23,OK
500000,2.00,1000000,1,1,Serial,0.03207,1.00,1.00,OK
500000,2.00,1000000,1,1,Paper1,0.03411,1.01,0.94,OK
500000,2.00,1000000,1,2,Paper1,0.01864,1.85,1.72,OK
500000,2.00,1000000,1,6,Paper1,0.00989,3.49,3.24,OK
500000,2.00,1000000,1,12,Paper1,0.00801,4.32,4.01,OK
500000,2.00,1000000,1,1,Parallel3,0.06527,1.09,0.49,OK
500000,2.00,1000000,1,2,Parallel3,0.03994,1.77,0.80,OK
500000,2.00,1000000,1,6,Parallel3,0.02030,3.49,1.58,OK
500000,2.00,1000000,1,12,Parallel3,0.01555,4.56,2.06,OK
500000,2.00,1000000,1,1,SV_improvement,0.02074,1.03,1.55,OK
500000,2.00,1000000,1,2,SV_improvement,0.01354,1.58,2.37,OK
500000,2.00,1000000,1,6,SV_improvement,0.00923,2.32,3.47,OK
500000,2.00,1000000,1,12,SV_improvement,0.00775,2.76,4.14,OK
5000000,0.10,500000,4500001,1,Serial,0.10475,1.00,1.00,OK
5000000,0.10,500000,4500001,1,Paper1,0.06468,0.99,1.62,OK
5000000,0.10,500000,4500001,2,Paper1,0.03537,1.81,2.96,OK
5000000,0.10,500000,4500001,6,Paper1,0.01351,4.73,7.75,OK
5000000,0.10,500000,4500001,12,Paper1,0.01097,5.83,9.55,OK
5000000,0.10,500000,4500001,1,Parallel3,0.19850,0.89,0.53,OK
5000000,0.10,500000,4500001,2,Parallel3,0.10917,1.63,0.96,OK
5000000,0.10,500000,4500001,6,Parallel3,0.05587,3.18,1.87,OK
5000000,0.10,500000,4500001,12,Parallel3,0.05315,3.34,1.97,OK
5000000,0.10,500000,4500001,1,SV_improvement,0.06845,0.86,1.53,OK
5000000,0.10,500000,4500001,2,SV_improvement,0.03965,1.49,2.64,OK
5000000,0.10,500000,4500001,6,SV_improvement,0.01485,3.98,7.05,OK
5000000,0.10,500000,4500001,12,SV_improvement,0.01470,4.02,7.13,OK
5000000,0.50,2500000,2500003,1,Serial,0.26310,1.00,1.00,OK
5000000,0.50,2500000,2500003,1,Paper1,0.12152,0.99,2.17,OK
5000000,0.50,2500000,2500003,2,Paper1,0.09184,1.31,2.86,OK
5000000,0.50,2500000,2500003,6,Paper1,0.03149,3.82,8.36,OK
5000000,0.50,2500000,2500003,12,Paper1,0.02540,4.74,10.36,OK
5000000,0.50,2500000,2500003,1,Parallel3,0.59257,0.98,0.44,OK
5000000,0.50,2500000,2500003,2,Parallel3,0.31961,1.82,0.82,OK
5000000,0.50,2500000,2500003,6,Parallel3,0.15275,3.81,1.72,OK
5000000,0.50,2500000,2500003,12,Parallel3,0.12438,4.68,2.12,OK
5000000,0.50,2500000,2500003,1,SV_improvement,0.10876,1.00,2.42,OK
5000000,0.50,2500000,2500003,2,SV_improvement,0.06455,1.69,4.08,OK
5000000,0.50,2500000,2500003,6,SV_improvement,0.02963,3.69,8.88,OK
5000000,0.50,2500000,2500003,12,SV_improvement,0.02116,5.16,12.44,OK
5000000,1.00,5000000,45,1,Serial,0.53439,1.00,1.00,OK
5000000,1.00,5000000,45,1,Paper1,0.68550,0.98,0.78,OK
5000000,1.00,5000000,45,2,Paper1,0.37334,1.80,1.43,OK
5000000,1.00,5000000,45,6,Paper1,0.15404,4.35,3.47,OK
5000000,1.00,5000000,45,12,Paper1,0.13565,4.94,3.94,OK
5000000,1.00,5000000,45,1,Parallel3,0.91250,1.01,0.59,OK
5000000,1.00,5000000,45,2,Parallel3,0.52413,1.76,1.02,OK
5000000,1.00,5000000,45,6,Parallel3,0.21122,4.38,2.53,OK
5000000,1.00,5000000,45,12,Parallel3,0.15941,5.80,3.35,OK
5000000,1.00,5000000,45,1,SV_improvement,0.27849,1.06,1.92,OK
5000000,1.00,5000000,45,2,SV_improvement,0.15015,1.97,3.56,OK
5000000,1.00,5000000,45,6,SV_improvement,0.06116,4.84,8.74,OK
5000000,1.00,5000000,45,12,SV_improvement,0.05323,5.56,10.04,OK
5000000,2.00,10000000,1,1,Serial,0.56359,1.00,1.00,OK
5000000,2.00,10000000,1,1,Paper1,0.66123,0.96,0.85,OK
5000000,2.00,10000000,1,2,Paper1,0.32895,1.92,1.71,OK
5000000,2.00,10000000,1,6,Paper1,0.12973,4.87,4.34,OK
5000000,2.00,10000000,1,12,Paper1,0.10693,5.91,5.27,OK
5000000,2.00,10000000,1,1,Parallel3,1.43280,1.02,0.39,OK
5000000,2.00,10000000,1,2,Parallel3,0.77710,1.88,0.73,OK
5000000,2.00,10000000,1,6,Parallel3,0.31419,4.65,1.79,OK
5000000,2.00,10000000,1,12,Parallel3,0.20795,7.02,2.71,OK
5000000,2.00,10000000,1,1,SV_improvement,0.35541,0.95,1.59,OK
5000000,2.00,10000000,1,2,SV_improvement,0.19255,1.76,2.93,OK
5000000,2.00,10000000,1,6,SV_improvement,0.09019,3.75,6.25,OK
5000000,2.00,10000000,1,12,SV_improvement,0.06681,5.06,8.44,OK
"""

# %%
# Parse the CSV data
try:
    df = pd.read_csv(io.StringIO(raw_csv.strip()))

    # Remove incomplete rows (e.g., "...")
    df = df.dropna()

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
    df = pd.DataFrame()  # Empty dataframe to prevent errors

# %%
# 1. SPEEDUP ANALYSIS
# Speedup is calculated relative to 1-thread baseline for each algorithm
# This shows how well algorithms scale with threads

if not df.empty:
    # Filter to only include parallel algorithms
    parallel_df = df[df["Algorithm"] != "Serial"].copy()

    if not parallel_df.empty and len(df["Nodes"].unique()) > 0:
        # Only create column facets if there are multiple node counts
        num_nodes = len(parallel_df["Nodes"].unique())

        if num_nodes > 1:
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
                col_wrap=min(3, num_nodes),  # Wrap columns if many node counts
            )
            g.set_axis_labels("Edge/Node Ratio", "Speedup (vs 1-thread baseline)")
            g.set_titles("Nodes: {col_name}")
        else:
            # Single node count - no faceting
            plt.figure(figsize=(10, 6))
            sns.lineplot(
                data=parallel_df,
                x="Ratio",
                y="Speedup",
                hue="Algorithm",
                style="Threads",
                markers=True,
                dashes=True,
                linewidth=2.5,
                palette="tab10",
            )
            plt.xlabel("Edge/Node Ratio", fontsize=12)
            plt.ylabel("Speedup (vs 1-thread baseline)", fontsize=12)
            plt.title(
                f"Parallel Speedup vs. Graph Density (Nodes={parallel_df['Nodes'].iloc[0]})",
                fontsize=16,
            )
            plt.axhline(1, color="gray", linestyle="--", alpha=0.5)
            plt.grid(True)
            plt.show()
            g = None

        if g is not None:
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

                    if len(unique_ratios) > 1:
                        ax.set_xscale("log")
                    ax.set_xticks(unique_ratios)
                    ax.set_xticklabels(new_labels, fontsize=8, rotation=45)
                except (IndexError, ValueError):
                    pass

            plt.subplots_adjust(top=0.85)
            g.figure.suptitle(
                "Parallel Speedup vs. Graph Density (vs 1-thread)", fontsize=16
            )
            plt.show()
    else:
        print("No parallel algorithm data found.")
else:
    print("No data loaded.")

# %%
# 2. ABSOLUTE PERFORMANCE (Log-Log Scale)
# Total execution time scaling with the number of edges.

if not df.empty:
    plt.figure(figsize=(12, 7))

    # Convert Nodes to string for better legend handling
    plot_df = df.copy()
    plot_df["Nodes"] = plot_df["Nodes"].astype(str)

    ax = sns.lineplot(
        data=plot_df,
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
else:
    print("No data to plot.")

# %%
# 3. BEST TRADEOFF HEATMAP
# Visualizing the maximum speedup achieved for every Node/Ratio configuration

if not df.empty:
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
else:
    print("No data loaded.")

# %%
# 4. CONNECTED COMPONENTS ANALYSIS
# Visualizing how connectivity changes with density (Phase Transition)

if not df.empty:
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
else:
    print("No data loaded.")

# %%
# 5. SCALABILITY ANALYSIS
# Speedup vs Threads for a large graph configuration

if not df.empty:
    plt.figure(figsize=(10, 6))

    # Filter for a representative large case (e.g. max nodes, max ratio)
    max_nodes = df["Nodes"].max()
    max_ratio = df["Ratio"].max()

    scalability_df = df[
        (df["Nodes"] == max_nodes)
        & (df["Ratio"] == max_ratio)
        & (df["Algorithm"] != "Serial")
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
        plt.title(f"Scalability (Nodes={max_nodes}, Ratio={max_ratio})", fontsize=16)
        plt.xlabel("Number of Threads", fontsize=12)
        plt.ylabel("Speedup (vs 1-thread baseline)", fontsize=12)
        plt.grid(True)

        # Add ideal speedup line (relative to 1-thread)
        max_threads = scalability_df["Threads"].max()
        min_threads = scalability_df["Threads"].min()
        plt.plot(
            [min_threads, max_threads],
            [min_threads, max_threads],
            "k--",
            alpha=0.3,
            label="Ideal",
        )
        plt.legend()
        plt.show()
    else:
        print(
            f"No data found for scalability analysis (Nodes={max_nodes}, Ratio={max_ratio})."
        )
else:
    print("No data loaded.")

# %%
# 6. ALGORITHM COMPARISON AT DIFFERENT THREAD COUNTS
# Compare algorithms across different thread counts for all graph sizes

if not df.empty:
    parallel_threads = df[df["Algorithm"] != "Serial"]["Threads"].unique()

    if len(parallel_threads) > 0:
        for thread_count in sorted(parallel_threads):
            thread_df = df[
                (df["Threads"] == thread_count) | (df["Algorithm"] == "Serial")
            ]

            if not thread_df.empty:
                plt.figure(figsize=(12, 6))

                # Convert Nodes to string for style
                plot_df = thread_df.copy()
                plot_df["Nodes"] = plot_df["Nodes"].astype(str)

                ax = sns.lineplot(
                    data=plot_df,
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
                plt.title(
                    f"Algorithm Performance Comparison (Threads={thread_count})",
                    fontsize=16,
                )
                plt.xlabel("Number of Edges", fontsize=12)
                plt.ylabel("Time (seconds)", fontsize=12)
                plt.grid(True, which="minor", ls=":", alpha=0.3)
                plt.grid(True, which="major", ls="--", alpha=0.5)
                plt.show()
    else:
        print("No parallel thread data found for comparison.")
else:
    print("No data loaded.")

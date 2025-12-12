# Parse the CSV data
import io

import matplotlib.pyplot as plt
import pandas as pd

# Assuming raw_csv is defined elsewhere in the code
# df = pd.read_csv(io.StringIO(raw_csv))

# Filter for a specific configuration to plot component sizes
# We'll need to extract component size data from the C++ output
# Since the C++ code outputs "ComponentID,Size\n" format, we need that data

# Note: The raw_csv above doesn't contain component size data yet
# We need to run the C++ program with the component size output

# For now, let's create placeholder code that will work once we have the data
# You'll need to paste the ComponentID,Size output from running the C++ program

component_csv = """
ComponentID,Size
152,1160
117,1305
76,1272
68,1135
12,1947
11,971
7,1586
36,1770
6,1526
5,2030
4,2052
16,1472
45,2036
3,1589
32,1379
15,1366
2,1201
1,1716
13,1932
0,1557
29,1869
18,1699
47,1175
19,2120
21,1120
50,967
24,1363
25,1914
26,1228
38,1244
39,1762
56,1121
57,1416
"""

# Uncomment and use this once you have the component size data:
df_components = pd.read_csv(io.StringIO(component_csv))

# Sort by size for better visualization
df_components = df_components.sort_values("Size", ascending=False).reset_index(
    drop=True
)

# Plot component sizes
fig, axes = plt.subplots(1, 2, figsize=(16, 6))

# Plot 1: Bar chart of top N components
top_n = min(50, len(df_components))
axes[0].bar(range(top_n), df_components["Size"].head(top_n))
axes[0].set_xlabel("Component Rank")
axes[0].set_ylabel("Component Size (Number of Nodes)")
axes[0].set_title(f"Top {top_n} Largest Connected Components")
axes[0].grid(True, alpha=0.3)

# Plot 2: Distribution of component sizes (log scale)
axes[1].hist(df_components["Size"], bins=50, edgecolor="black")
axes[1].set_xlabel("Component Size (Number of Nodes)")
axes[1].set_ylabel("Frequency")
axes[1].set_title("Distribution of Component Sizes")
axes[1].set_yscale("log")
axes[1].grid(True, alpha=0.3)

plt.tight_layout()
plt.show()

# Print statistics
print("\nComponent Statistics:")
print(f"Total components: {len(df_components)}")
print(f"Largest component: {df_components['Size'].max()} nodes")
print(f"Smallest component: {df_components['Size'].min()} nodes")
print(f"Mean component size: {df_components['Size'].mean():.2f} nodes")
print(f"Median component size: {df_components['Size'].median():.2f} nodes")

print(
    "Please run the C++ program and paste the ComponentID,Size output into the component_csv variable above."
)

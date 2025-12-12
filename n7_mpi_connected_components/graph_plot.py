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
1865945,8
1823587,1
153331,1
168,110845
85,155148
1466616,1
53,130224
71,98864
12,128519
41,155489
1351169,2
128,124752
10,121968
39,122654
68,91737
9,169328
8,173413
7,177560
36,127686
6,108011
5,143442
34,173687
4,114334
62,95062
3,130621
2,111646
31,95313
1,190476
13,101043
0,107991
14,120826
43,149844
16,99018
45,179241
135,113444
17,158865
46,127422
21,166113
140,117281
22,101500
24,133245
25,123828
2575457,1
637602,2
48,149544
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

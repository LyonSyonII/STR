import pandas as pd
import matplotlib.pyplot as plt
import argparse
import sys
import io

# Mapping from emoji to numeric state
STATE_MAP = {
    '⏹️': 0,  # Blocked
    '⏸️': 1,  # Ready
    '▶️': 2   # Running
}


def plot_task_states(df, time_col='time'):
    # Identify task columns (all except time)
    task_cols = [col for col in df.columns if col != time_col]
    if not task_cols:
        print(f"No task columns found after excluding '{time_col}'", file=sys.stderr)
        return

    # Map emoji states to numeric
    state_df = df.copy()
    for col in task_cols:
        state_df[col] = state_df[col].map(STATE_MAP)

    # Prepare subplots
    n_tasks = len(task_cols)
    fig, axes = plt.subplots(n_tasks, 1, sharex=True, figsize=(10, 2*n_tasks))
    if n_tasks == 1:
        axes = [axes]

    # Create step plots
    for ax, col in zip(axes, task_cols):
        ax.step(state_df[time_col], state_df[col], where='post')
        ax.set_ylim(-0.2, 2.2)
        ax.set_yticks([0, 1, 2])
        ax.set_yticklabels(['Blocked', 'Ready', 'Running'])
        ax.set_ylabel(col)
        ax.grid(True, linestyle='--', alpha=0.5)

    axes[-1].set_xlabel(time_col.capitalize())
    fig.suptitle('Task State Over Time')
    plt.tight_layout(rect=[0, 0.03, 1, 0.97])
    plt.show()


def main():
    parser = argparse.ArgumentParser(description='Plot task states from CSV data.')
    parser.add_argument('--file', '-f', default='data.csv', help='Path to CSV data file')
    parser.add_argument('--time-col', '-t', default='time', help='Name of the time column')
    args = parser.parse_args()

    # Read and preprocess CSV: skip lines that are just DAT and drop the trailing constant column
    try:
        with open(args.file, 'r', encoding='utf-8') as f:
            lines = [line.strip() for line in f if line.strip() and not line.strip().startswith('DAT')]
        # Read into DataFrame; last column is ignored
        df = pd.read_csv(
            io.StringIO("\n".join(lines)),
            header=None,
            names=['time', 'task1', 'task2', 'task3', 'task4', 'task5', '_ignore_']
        )
        df.drop(columns=['_ignore_'], inplace=True)
    except Exception as e:
        print(f"Error reading or preprocessing file '{args.file}': {e}", file=sys.stderr)
        sys.exit(1)

    # Ensure time column existence
    if args.time_col != 'time':
        if args.time_col not in df.columns:
            print(f"Warning: specified time column '{args.time_col}' not found, using 'time' column.", file=sys.stderr)
        else:
            df.rename(columns={args.time_col: 'time'}, inplace=True)

    # Call plotting function
    plot_task_states(df, time_col='time')


if __name__ == '__main__':
    main()

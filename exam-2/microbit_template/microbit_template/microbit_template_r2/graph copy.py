import pandas as pd
import matplotlib.pyplot as plt
import argparse
import sys
import io

# Only show relevant task states
STATE_LABELS = {
    0: 'Running',
    1: 'Ready',
    2: 'Blocked'
}

def compute_task_metrics(state_df, time_col, task_col):
    times = state_df[time_col].values.astype(float)
    states = state_df[task_col].values.astype(int)

    # Find blocked transitions (entry into Blocked)
    blocked_idxs = [i for i in range(1, len(states)) if states[i] == 2 and states[i-1] != 2]
    if not blocked_idxs:
        return 0.0, 0.0

    # First blocked marks end of initial compute window
    first_block = blocked_idxs[0]

    # Accumulate all Running durations up to first Blocked
    compute_time = sum(
        (times[i] - times[i-1])
        for i in range(1, first_block+1)
        if states[i-1] == 0
    )

    # Period between first two Blocked entries
    if len(blocked_idxs) > 1:
        period = times[blocked_idxs[1]] - times[blocked_idxs[0]]
    else:
        period = 0.0

    return compute_time, period

def plot_task_states(df, time_col='time'):
    task_cols = [col for col in df.columns if col != time_col]
    if not task_cols:
        print(f"No task columns found after excluding '{time_col}'", file=sys.stderr)
        return

    state_df = df.copy()
    for col in task_cols:
        state_df[col] = state_df[col].astype(int)

    n_tasks = len(task_cols)
    fig, axes = plt.subplots(n_tasks, 1, sharex=True, figsize=(10, 2*n_tasks))
    if n_tasks == 1:
        axes = [axes]

    for ax, col in zip(axes, task_cols):
        ax.step(state_df[time_col], state_df[col], where='post')
        ax.set_ylim(-0.5, 2.5)
        ax.set_yticks(list(STATE_LABELS.keys()))
        ax.set_yticklabels([STATE_LABELS[k] for k in STATE_LABELS])
        ax.invert_yaxis()
        ax.set_ylabel(col)
        ax.grid(True, linestyle='--', alpha=0.5)

        compute_time, period = compute_task_metrics(state_df, time_col, col)
        info = f"Compute Time: {compute_time:.2f} ms\nPeriod: {period:.2f} ms"
        ax.text(0.02, 0.85, info, transform=ax.transAxes, fontsize=9,
                bbox=dict(boxstyle="round", facecolor="white", alpha=0.5))

    axes[-1].set_xlabel(f"{time_col.capitalize()} (ms)")
    fig.suptitle('Task State Over Time (Filtered: Running, Ready, Blocked)')
    plt.tight_layout(rect=[0, 0.03, 1, 0.97])
    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Plot task states from CSV data.')
    parser.add_argument('--file', '-f', default='data.csv', help='Path to CSV data file')
    parser.add_argument('--time-col', '-t', default='time', help='Name of the time column')
    args = parser.parse_args()

    try:
        with open(args.file, 'r', encoding='utf-8') as f:
            lines = [line.strip() for line in f if line.strip() and not line.startswith('DAT')]
        raw_data = [line for line in lines if line]
        sample_row = raw_data[0].split(',')
        n_cols = len(sample_row)
        column_names = ['time'] + [f'task{i+1}' for i in range(n_cols - 1)]

        df = pd.read_csv(io.StringIO("\n".join(raw_data)), header=None, names=column_names)
        df['time'] = df['time'].astype(float)
    except Exception as e:
        print(f"Error reading or preprocessing file '{args.file}': {e}", file=sys.stderr)
        sys.exit(1)

    plot_task_states(df, time_col='time')

if __name__ == '__main__':
    main()

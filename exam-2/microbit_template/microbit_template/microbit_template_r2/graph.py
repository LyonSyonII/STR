import pandas as pd
import matplotlib.pyplot as plt
import sys
import io
import glob

# Only show relevant task states
STATE_LABELS = {
    0: 'Running',
    1: 'Ready',
    2: 'Blocked'
}

# Expected specs for each task (computation, deadline, period)
SPECS = {
    'task1': {'C': 2,  'D': 15, 'T': 30},
    'task2': {'C': 4,  'D': 20, 'T': 30},
    'task3': {'C': 10, 'D': 35, 'T': 40},
    'task4': {'C': 21, 'D': 40, 'T': 50},
    'task5': {'C': 5,  'D': 50, 'T': 50},
    'task6': {'C': 1,  'D': 1, 'T': 1},
}


def compute_task_metrics(state_df, time_col, task_col):
    times = state_df[time_col].values.astype(float)
    states = state_df[task_col].values.astype(int)

    # Find indices where state becomes Blocked
    blocked_idxs = [i for i in range(1, len(states)) if states[i] == 2 and states[i-1] != 2]
    if not blocked_idxs:
        return 0.0, 0.0
    first_block = blocked_idxs[0]

    # Sum all Running durations up to first Blocked
    compute_time = sum(
        (times[i] - times[i-1]) for i in range(1, first_block+1) if states[i-1] == 0
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
    fig, axes = plt.subplots(n_tasks, 1, sharex=True, figsize=(12, 2*n_tasks))
    if n_tasks == 1:
        axes = [axes]

    max_time = state_df[time_col].max()

    for ax, col in zip(axes, task_cols):
        # Plot state timeline
        ax.step(state_df[time_col], state_df[col], where='post')
        ax.set_ylim(-0.5, 2.5)
        ax.set_yticks(list(STATE_LABELS.keys()))
        ax.set_yticklabels([STATE_LABELS[k] for k in STATE_LABELS])
        ax.invert_yaxis()
        ax.set_ylabel(col)
        ax.grid(True, linestyle='--', alpha=0.5)

        if col in SPECS:
            spec = SPECS[col]
            D = spec['D']
            T = spec['T']

            # Draw period lines (task release times)
            t_release = 0.0
            while t_release <= max_time:
                ax.axvline(
                    t_release,
                    color='gray',
                    linestyle='--',
                    linewidth=0.75,
                    alpha=0.8,
                    label='Period' if t_release == 0 else ""
                )
                t_release += T

            # Draw deadlines (deadline = release + D)
            t_deadline = 0.0
            while t_deadline <= max_time:
                ax.axvline(
                    t_deadline + D,
                    color='red',
                    linestyle=':',
                    linewidth=1,
                    alpha=1.,
                    label='Deadline' if t_deadline == 0 else ""
                )
                t_deadline += T

            # Compute metrics for annotation
            compute_time, period = compute_task_metrics(state_df, time_col, col)
            info = f"Compute Time: {compute_time:.2f} ms\nPeriod: {period:.2f} ms"
            ax.text(0.02, 0.85, info, transform=ax.transAxes, fontsize=9,
                    bbox=dict(boxstyle="round", facecolor="white", alpha=0.5))

    axes[-1].set_xlabel(f"{time_col.capitalize()} (ms)")
    fig.suptitle('Task State Over Time with Deadlines, Compute & Period')
    plt.tight_layout(rect=[0, 0.03, 1, 0.97])
    plt.show()

def get_file():
    files = glob.glob("logs/*.log")
    files.sort(reverse=True)
    return files[0]

def main():
    file = get_file();

    try:
        with open(file, 'r', encoding='utf-8') as f:
            lines = [line.strip() for line in f if line.strip() and not line.startswith('DAT')]
        raw_data = [line for line in lines if line]
        sample_row = raw_data[0].split(',')
        n_cols = len(sample_row)
        column_names = ['time'] + [f'task{i+1}' for i in range(n_cols - 1)]

        footer_lines = list(reversed(raw_data)).index("---") + 1
        print(raw_data[0:-footer_lines])

        df = pd.read_csv(io.StringIO("\n".join(raw_data)), header=None, names=column_names, skipfooter=footer_lines)
        df['time'] = df['time'].astype(float)
    except Exception as e:
        print(f"Error reading or preprocessing file '{file}': {e}", file=sys.stderr)
        sys.exit(1)

    plot_task_states(df, time_col='time')


if __name__ == '__main__':
    main()

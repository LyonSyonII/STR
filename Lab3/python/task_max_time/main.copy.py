import pandas as pd

# Load the CSV data
df = pd.read_csv('task_state.csv')
df['timestamp'] = pd.to_datetime(df['timestamp'], format="mixed")

# Tasks to analyze
tasks = ['task1_state', 'task2_state', 'task3_state', 'task9_state']

# Initialize variables to track maximum durations and start times
max_durations = {task: pd.Timedelta(0) for task in tasks}
current_start = {task: None for task in tasks}

# Sort by timestamp to ensure chronological order
df = df.sort_values('timestamp')

# Iterate through each row to track Running states
for idx, row in df.iterrows():
    for task in tasks:
        current_state = row[task]
        timestamp = row['timestamp']
        
        if current_state == 'TaskState.Running':
            if current_start[task] is None:
                current_start[task] = timestamp  # Start of Running state
        else:
            if current_start[task] is not None:
                # Calculate duration and update max if needed
                duration = timestamp - current_start[task]
                if duration > max_durations[task]:
                    max_durations[task] = duration
                current_start[task] = None  # Reset start time

# Check for tasks still in Running state at the end of the log
# last_timestamp = df['timestamp'].iloc[-1]
# for task in tasks:
#     if current_start[task] is not None:
#         duration = last_timestamp - current_start[task]
#         if duration > max_durations[task]:
#             max_durations[task] = duration

# Convert results to seconds for readability
print("Maximum continuous time each task spent in 'Running' state:")
for task, duration in max_durations.items():
    print(f"{task}: {duration.total_seconds():.6f} seconds")
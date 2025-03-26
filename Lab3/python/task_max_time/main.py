import pandas as pd

# Load and prepare data
df = pd.read_csv('task_state.csv')
df['timestamp'] = pd.to_datetime(df['timestamp'], format="mixed")
df.sort_values('timestamp', inplace=True)

tasks = ['task1_state', 'task2_state', 'task3_state', 'task9_state']
results = {}

for task in tasks:
    max_duration = pd.Timedelta(0)
    current_run_start = None
    
    for idx, row in df.iterrows():
        current_state = row[task]
        timestamp = row['timestamp']
        
        if current_state == 'TaskState.Running':
            if current_run_start is None:  # Start tracking new run
                current_run_start = timestamp
        elif current_run_start is not None:
            if current_state == 'TaskState.Blocked' or current_state == 'TaskState.Suspended':
                # Calculate duration from Running start to Blocked
                duration = timestamp - current_run_start
                if duration > max_duration:
                    max_duration = duration
            # Reset tracking regardless of intermediate state
            current_run_start = None
    
            
    # Store result
    if max_duration > pd.Timedelta(0):
        results[task] = f"{max_duration.total_seconds():.6f} seconds"
    else:
        results[task] = "No Blocked state after Running"

# Display results
print("Maximum time from Running to Blocked (any intermediate states allowed):")
for task, duration in results.items():
    print(f"{task}: {duration}")
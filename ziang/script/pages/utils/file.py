from typing import Optional
import pandas as pd
from streamlit.runtime.uploaded_file_manager import UploadedFile
from script.models.task import Task

DELIMITERS = {
    "Semicolon": ";",
    "Comma": ",",
    "Tab": "\t",
}

VALID_COLUMN_AMOUNT = {
    2, # compute_time, deadline = period
    3, # compute_time, deadline, period
    4, # compute_time, deadline, period, priority
    5, # compute_time, deadline, period, priority, task_id
}

def _read_task_row(row: pd.Series, task_id: Optional[int] = None) -> Task:
    """
    Reads a row from the DataFrame and returns a Task object.
    """
    compute_time = float(row["compute_time"])
    deadline = int(row["deadline"])
    period = int(row["period"]) if "period" in row else deadline
    priority = int(row["priority"]) if "priority" in row else 0
    task_id = int(row["task_id"]) if "task_id" in row \
              else task_id if task_id is not None else 0

    return Task(
        compute_time=compute_time,
        deadline=deadline,
        period=period,
        priority=priority,
        task_id=task_id,
    )

def read_tasks_from_df(df: pd.DataFrame) -> list[Task]:
    """
    Reads a DataFrame and returns a list of Task objects.
    """
    num_columns = df.shape[1]

    if num_columns not in VALID_COLUMN_AMOUNT:
        raise ValueError(f"Invalid number of columns: {num_columns}"
                         f"\nExpected one of {VALID_COLUMN_AMOUNT} columns.")

    tasks: list[Task] = []
    i = 0
    for _, row in df.iterrows():
        i = i + 1
        if "task_id" in df.columns:
            task = _read_task_row(row)
        else:
            task = _read_task_row(row, task_id=i)

        tasks.append(task)

    return tasks

def read_tasks(file: UploadedFile, delimiter: str = ';', has_header: bool = True) -> list[Task]:
    """
    Reads a CSV file and returns a list of Task objects.
    """

    if delimiter not in DELIMITERS.values():
        raise ValueError(f"Invalid delimiter: {delimiter}. Expected one of {DELIMITERS.values()}")

    df = pd.read_csv(file, delimiter=delimiter, header=0 if has_header else None)
    return read_tasks_from_df(df)

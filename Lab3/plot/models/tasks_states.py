from typing import TypedDict
from plot.enums.task_state import TaskState
from datetime import datetime

class TasksState(TypedDict):
    timestamp: datetime
    task1_state: TaskState
    task2_state: TaskState
    task3_state: TaskState
    task4_state: TaskState
    task5_state: TaskState
    task6_state: TaskState
    debug_value: str

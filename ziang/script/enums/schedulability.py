from enum import Enum

class Schedulability(Enum):
    """
    Enum indicating the schedulability of a task.
    """

    SCHEDULABLE = 1
    UNSCHEDULABLE = 2
    CANNOT_GUARANTEE_SCHEDULABILITY = 3

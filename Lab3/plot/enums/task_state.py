""" Task states """
from enum import Enum

class TaskState(Enum):
    Running = 0
    Ready = 1
    Blocked = 2
    Suspended = 3
    Deleted = 4
    Invalid = 5

    @staticmethod
    def get_state_from_str(string: str) -> "TaskState":
        """
        Get the task state from a string.
        The string should be one of the following:

        - `TaskState.Running`

        or

        - `Running`

        The same for the other states.
        """

        if ("." in string):
            _, member_name = string.split(".")
            task_state = getattr(TaskState, member_name)
            return task_state

        return getattr(TaskState, string)

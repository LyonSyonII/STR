""" Task states """
from enum import Enum

class TaskState(Enum):
    Running = 0
    Ready = 1
    Blocked = 2
    Suspended = 3
    Deleted = 4
    Invalid = 5

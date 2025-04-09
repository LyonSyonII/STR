from typing import Optional
from dataclasses import dataclass
from script.models.task import Task

@dataclass
class Preemption:
    """
    Represents the preemption information
    """

    start_time: float
    end_time: float

    # The task that preempts the current task
    preempting_task: Task

    def __init__(self, start_time: float, end_time: float, preempting_task: Task):
        """
        Set the start time and end time of the preemption.
        Checks that it is a valid configuration.

        :param start_time: The start time of the preemption.
        :type start_time: float
        :param end_time: The end time of the preemption.
        :type end_time: float
        :param preempting_task: The task that preempts the current task.
        :type preempting_task: Task
        """

        if start_time < 0:
            raise ValueError("Start time must be greater than 0")

        if end_time < start_time:
            raise ValueError("End time must be greater than start time")

        self.start_time = start_time
        self.end_time = end_time
        self.preempting_task = preempting_task

@dataclass
class Event:
    """
    Represents the task execution information
    """

    start_time: float
    task: Task

    interruptions: list[Preemption]

    def __init__(self, start_time: float, task: Task):
        """
        Set the start time and the task that is executed at that time.
        Checks that it is a valid configuration.

        :param start_time: The start time of the task.
        :type start_time: float
        :param task: The task that is executed at that time.
        :type task: Task
        """

        if start_time < 0:
            raise ValueError("Start time must be greater than 0")

        self.start_time = start_time
        self.task = task
        self.interruptions = []

    @property
    def end_time(self) -> float:
        """
        Calculate the end time of the task.

        :return: The end time of the task.
        :rtype: float
        """

        wait_time = 0.0

        for interruption in self.interruptions:
            wait_time += interruption.end_time - interruption.start_time

        return self.start_time + self.task.compute_time + wait_time

    def add_interruption(self, interruption: Preemption) -> None:
        """
        Add an interruption to the task.

        :param interruption: The interruption to add.
        :type interruption: Preemption
        """
        self.interruptions.append(interruption)

        if not self.check_configuration():
            raise ValueError("Invalid configuration for the task"
                             f"\nStart time: {self.start_time}"
                             f"\nCompute time: {self.task.compute_time}"
                             f"\nDeadline: {self.task.deadline}"
                             f"\nNew Interruption: {interruption.start_time} - {interruption.end_time}")

    def check_configuration(self) -> bool:
        """
        Check if the configuration is valid.

        :return: True if the configuration is valid, False otherwise.
        :rtype: bool
        """
        return self.end_time - self.start_time <= self.task.deadline

@dataclass
class TimeMark:
    """
    Represents either a deadline or a task arrival time.
    """

    time: float
    task_id: int

@dataclass
class Scheduling:
    """
    Represents the scheduling information
    """

    # List of events. If not set it means that the scheduling is not possible.
    events: Optional[list[Event]] = None

    # The frame size of the scheduling.
    # Information used for the cyclic scheduling.
    frame_time: Optional[int] = None

    def is_schedulable(self) -> bool:
        """
        Check if the scheduling is possible.

        :return: True if the scheduling is possible, False otherwise.
        :rtype: bool
        """
        return self.events is not None and len(self.events) > 0

    @property
    def num_frames(self) -> int:
        """
        Get the number of frames in the scheduling.

        :return: The number of frames in the scheduling.
        :rtype: int
        """
        if self.events is None:
            return 0

        return len(self.events)

    @property
    def duration(self) -> float:
        """
        Get the duration of the scheduling.

        :return: The duration of the scheduling.
        :rtype: float
        """
        if self.events is None:
            return 0.0

        max_end_time = max(event.end_time for event in self.events)
        min_start_time = min(event.start_time for event in self.events)
        return max_end_time - min_start_time

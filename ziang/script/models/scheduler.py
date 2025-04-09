from abc import ABC, abstractmethod
from typing import Optional
from script.models.task import Task
from script.models.event import Scheduling
# from script.utils.print import print_table
from script.utils.task import hyperperiod, total_utilization, \
                              get_min_deadline, get_max_deadline, \
                              get_min_period, get_max_period, \
                              get_min_compute_time, get_max_compute_time

class Scheduler(ABC):
    """
    Abstract base class for all schedulers.
    """

    tasks: list[Task]

    def __init__(self, tasks: Optional[list[Task]] = None, sort_tasks: bool = True):
        """
        Initialize the scheduler.

        :param tasks: The list of tasks.
        :type tasks: list[Task]
        :param sort_tasks: Whether to sort the tasks or not.
        :type sort_tasks: bool
        """
        self.tasks = tasks if tasks is not None else []

        if sort_tasks:
            self.sort_tasks()

    @abstractmethod
    def is_schedulable(self) -> bool:
        """
        Check if the task can be scheduled.

        :return: The schedulability of the task.
        :rtype: bool
        """

    @abstractmethod
    def sort_tasks(self) -> None:
        """
        Sort the tasks in the scheduler.

        :return: None
        :rtype: None
        """

    @abstractmethod
    def get_scheduling(self) -> Scheduling:
        """
        Return a possible schedule for the tasks.
        Returns None if the tasks cannot be scheduled.

        :return: The schedule of the tasks.
        :rtype: Optional[dict[Task, list[Event]]]
        """

    def get_task(self, task_id: int) -> Task:
        """
        Get a task by its ID.

        :param task_id: The ID of the task.
        :type task_id: int
        :return: The task with the given ID.
        :rtype: Task
        """
        for task in self.tasks:
            if task.task_id == task_id:
                return task

        raise ValueError(f"Task with ID {task_id} not found.")

    @property
    def hyperperiod(self) -> int:
        """
        Calculate the hyperperiod of the tasks.

        :return: The hyperperiod of the tasks.
        :rtype: int
        """
        return hyperperiod(self.tasks)

    @property
    def total_utilization(self) -> float:
        """
        Calculate the total utilization of the tasks.

        :return: The total utilization of the tasks.
        :rtype: float
        """
        return total_utilization(self.tasks)

    @property
    @abstractmethod
    def utilization_bound(self) -> float:
        """
        Calculate the utilization bound of the tasks.

        :return: The utilization bound of the tasks.
        :rtype: float
        """

    @property
    def min_deadline(self) -> int:
        """
        Calculate the minimum deadline of the tasks.

        :return: The minimum deadline of the tasks.
        :rtype: int
        """
        return get_min_deadline(self.tasks)

    @property
    def max_deadline(self) -> int:
        """
        Calculate the maximum deadline of the tasks.

        :return: The maximum deadline of the tasks.
        :rtype: int
        """
        return get_max_deadline(self.tasks)

    @property
    def min_period(self) -> int:
        """
        Calculate the minimum period of the tasks.

        :return: The minimum period of the tasks.
        :rtype: int
        """
        return get_min_period(self.tasks)

    @property
    def max_period(self) -> int:
        """
        Calculate the maximum period of the tasks.

        :return: The maximum period of the tasks.
        :rtype: int
        """
        return get_max_period(self.tasks)

    @property
    def min_compute_time(self) -> float:
        """
        Calculate the minimum compute time of the tasks.

        :return: The minimum compute time of the tasks.
        :rtype: float
        """
        return get_min_compute_time(self.tasks)

    @property
    def max_compute_time(self) -> float:
        """
        Calculate the maximum compute time of the tasks.

        :return: The maximum compute time of the tasks.
        :rtype: float
        """
        return get_max_compute_time(self.tasks)

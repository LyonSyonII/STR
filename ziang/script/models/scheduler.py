from abc import ABC, abstractmethod
from typing import Optional
from script.models.task import Task
from script.utils.print import print_table
from script.utils.task import hyperperiod, total_utilization, \
                              get_min_deadline, get_max_deadline, \
                              get_min_period, get_max_period, \
                              get_min_compute_time, get_max_compute_time

class Scheduler(ABC):
    """
    Abstract base class for all schedulers.
    """

    tasks: list[Task]

    def __init__(self, tasks: Optional[list[Task]] = None):
        """
        Initialize the scheduler.
        :param tasks: The list of tasks.
        :type tasks: list[Task]
        """
        self.tasks = tasks if tasks is not None else []
        print_table(self.tasks)

    @abstractmethod
    def is_schedulable(self) -> bool:
        """
        Check if the task can be scheduled.

        :return: The schedulability of the task.
        :rtype: bool
        """

    @property
    def hyperperiod(self) -> float:
        """
        Calculate the hyperperiod of the tasks.

        :return: The hyperperiod of the tasks.
        :rtype: float
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

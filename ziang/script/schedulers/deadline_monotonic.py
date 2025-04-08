from script.models.scheduler import Scheduler
from script.models.task import Task
from script.utils.task import ResponseTimeAnalysis

class DeadlineMonotonicScheduler(Scheduler):
    """
    Cyclic scheduler class.
    """

    def __init__(self, tasks: list[Task], check_priority: bool = True):
        """
        Initialize the rate monotonic scheduler with a list of tasks.

        :param tasks: List of tasks to be scheduled.
        :param check_priority: If True, the tasks will be sorted by their period and assigned priorities.
        :type tasks: list[Task]
        """

        if check_priority:
            tasks.sort(key=lambda task: task.deadline)
            total_tasks = len(tasks)
            for i, task in enumerate(tasks):
                task.priority = total_tasks - i

        super().__init__(tasks)

    @property
    def utilization_bound(self) -> float:
        """
        Calculate the utilization bound for the rate monotonic scheduler.
        The utilization bound is calculated as the sum of the utilizations of all tasks.
        """
        n = len(self.tasks)
        return n * (2 ** (1 / n) - 1)

    def is_schedulable(self) -> bool:
        """
        Check if the task can be scheduled using cyclic scheduling.
        """
        return ResponseTimeAnalysis.check_response_time(self.tasks)

from script.models.scheduler import Scheduler
from script.models.task import Task
from script.utils.task import ResponseTimeAnalysis
from script.models.event import Event, Scheduling

class DeadlineMonotonicScheduler(Scheduler):
    """
    Cyclic scheduler class.
    """

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

    def sort_tasks(self):
        self.tasks.sort(key=lambda task: task.deadline)
        total_tasks = len(self.tasks)
        for i, task in enumerate(self.tasks):
            task.priority = total_tasks - i

    def get_scheduling(self) -> Scheduling:
        """
        Get the scheduling for the tasks using cyclic scheduling.
        """
        if not self.is_schedulable():
            return Scheduling(events=None)

        return Scheduling()

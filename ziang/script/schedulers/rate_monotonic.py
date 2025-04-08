from script.models.scheduler import Scheduler
from script.models.task import Task
from script.utils.task import ResponseTimeAnalysis


class RateMonotonicScheduler(Scheduler):
    """
    Rate Monotonic scheduler class.
    """

    def __init__(self, tasks: list[Task], check_priority: bool = True):
        """
        Initialize the rate monotonic scheduler with a list of tasks.

        :param tasks: List of tasks to be scheduled.
        :param check_priority: If True, the tasks will be sorted by their period and assigned priorities.
        :type tasks: list[Task]
        """

        if check_priority:
            tasks.sort(key=lambda task: task.period)
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

    def get_product(self) -> float:
        """
        Calculate the product of the utilizations of all tasks.
        """
        product = 1.0
        for task in self.tasks:
            product *= task.utilization + 1
        return product

    def condition1(self) -> bool:
        """
        Check if the task meets the first sufficient condition of a rate monotonic scheduler.
        This criterios is met if the total utilization is less than or equal n*(2^(1/n) - 1).
        """

        return self.total_utilization <= self.utilization_bound

    def condition2(self) -> bool:
        """
        Check if the task meets the first sufficient condition of a rate monotonic scheduler.
        This criterios is met if the product of the utilizations is less than or equal to 2
        """
        return self.get_product() <= 2.0

    def is_schedulable(self) -> bool:
        """
        Check if the task can be scheduled using cyclic scheduling.
        """
        sufficient_conditions = [self.condition1(), self.condition2()]

        if not all(sufficient_conditions):
            return ResponseTimeAnalysis.check_response_time(self.tasks)

        # It means that all sufficient conditions are met
        return True

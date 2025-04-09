from script.models.scheduler import Scheduler
from script.utils.task import ResponseTimeAnalysis
from script.models.event import Event, Scheduling


class RateMonotonicScheduler(Scheduler):
    """
    Rate Monotonic scheduler class.
    """

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

    def sort_tasks(self) -> None:
        """
        Sort the tasks in the scheduler.
        The tasks are sorted by their periods.
        """
        self.tasks.sort(key=lambda task: task.period)

    def get_scheduling(self) -> Scheduling:
        """
        Get the scheduling for the tasks using rate monotonic scheduling.
        """
        if not self.is_schedulable():
            return Scheduling(events=None)

        return Scheduling()

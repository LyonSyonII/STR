from script.models.scheduler import Scheduler
from script.utils.task import SecondaryPeriod


class CyclicScheduler(Scheduler):
    """
    Cyclic scheduler class.
    """

    @property
    def utilization_bound(self) -> float:
        """
        Calculate the utilization bound for the cyclic scheduler.
        The utilization bound is 1.
        """
        return 1.0


    def condition1(self) -> bool:
        """
        Check if the task meets the first condition for cyclic scheduling.
        This criterios is met if the total utilization is less than or equal to 1.
        """

        return self.total_utilization <= self.utilization_bound

    def condition2(self) -> bool:
        """
        Check if the task meets the second condition for cyclic scheduling.
        This criterios is met if the hyperperiod is greater than 0.
        """

        ts = SecondaryPeriod.get_ts(self.tasks, self.min_deadline, self.max_compute_time)
        return all(SecondaryPeriod.check_ts(t, self.tasks) for t in ts)

    def is_schedulable(self) -> bool:
        """
        Check if the task can be scheduled using cyclic scheduling.
        """
        conditions = [self.condition1(), self.condition2()]
        return all(conditions)

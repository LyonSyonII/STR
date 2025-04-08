from script.models.scheduler import Scheduler
from script.utils.task import ProcessorDemandCriterion

class EarliestDeadlineFirstScheduler(Scheduler):
    """
    Cyclic scheduler class.
    """

    @property
    def utilization_bound(self) -> float:
        """
        Calculate the utilization bound for the earliest deadline first scheduler.
        The utilization bound is calculated as the sum of the utilizations of all tasks.
        """
        return 1

    def condition1(self) -> bool:
        """
        Check the first condition for schedulability.
        It works is the total utilization of the tasks is less than or equal to 1
        and for all tasks the period is the same as the deadline.
        """
        return self.total_utilization <= self.utilization_bound and all(
            task.period == task.deadline for task in self.tasks
        )

    def condition2(self) -> bool:
        """
        Check the second condition for schedulability.
        Use this when condition1 cannot be used.
        It requires using processor on demand criterion
        """

        time_slots = ProcessorDemandCriterion.get_time_slots(self.tasks)
        return all(ProcessorDemandCriterion.check_g(self.tasks, 0, time_slot) for time_slot in time_slots)

    def is_schedulable(self) -> bool:
        """
        Check if the tasks are schedulable.
        :return: True if the tasks are schedulable, False otherwise.
        :rtype: bool
        """
        return self.condition1() or self.condition2()

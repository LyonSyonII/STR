from dataclasses import dataclass

@dataclass
class Task:
    compute_time: float
    deadline: int
    period: int
    task_id: int
    priority: int

    def __lt__(self, other: "Task") -> bool:
        """
        Compare two tasks based on their priority.

        :param other: The other task to compare with.
        :type other: Task
        :return: True if this task has a higher priority than the other task, False otherwise.
        :rtype: bool
        """
        return self.priority < other.priority

    @property
    def utilization(self) -> float:
        """
        Calculate the utilization of the task.

        :return: The utilization of the task.
        :rtype: float
        """
        return self.compute_time / self.period

@dataclass
class AperiodicTask(Task):
    arrival_time: float

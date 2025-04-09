import math
from script.models.task import Task

def total_utilization(tasks: list[Task]) -> float:
    """
    Calculate the total utilization of a set of tasks.

    :param tasks: The list of tasks.
    :return: The total utilization of the tasks.
    :rtype: float
    """
    return sum(task.utilization for task in tasks)

def hyperperiod(tasks: list[Task]) -> int:
    """
    Calculate the hyperperiod of a set of tasks.
    Note that this function only works if the periods are all integers.

    :param tasks: The list of tasks.
    :return: The hyperperiod of the tasks.
    :rtype: float
    """
    periods = [task.period for task in tasks]
    return math.lcm(*periods)

def get_min_deadline(tasks: list[Task]) -> int:
    """
    Get the minimum deadline of a set of tasks.

    :param tasks: The list of tasks.
    :return: The minimum deadline of the tasks.
    :rtype: float
    """
    return min(task.deadline for task in tasks)

def get_max_deadline(tasks: list[Task]) -> int:
    """
    Get the maximum deadline of a set of tasks.

    :param tasks: The list of tasks.
    :return: The maximum deadline of the tasks.
    :rtype: float
    """
    return max(task.deadline for task in tasks)

def get_min_period(tasks: list[Task]) -> int:
    """
    Get the minimum period of a set of tasks.

    :param tasks: The list of tasks.
    :return: The minimum period of the tasks.
    :rtype: float
    """
    return min(task.period for task in tasks)

def get_max_period(tasks: list[Task]) -> int:
    """
    Get the maximum period of a set of tasks.

    :param tasks: The list of tasks.
    :return: The maximum period of the tasks.
    :rtype: float
    """
    return max(task.period for task in tasks)

def get_min_compute_time(tasks: list[Task]) -> float:
    """
    Get the minimum compute time of a set of tasks.

    :param tasks: The list of tasks.
    :return: The minimum compute time of the tasks.
    :rtype: float
    """
    return min(task.compute_time for task in tasks)

def get_max_compute_time(tasks: list[Task]) -> float:
    """
    Get the maximum compute time of a set of tasks.

    :param tasks: The list of tasks.
    :return: The maximum compute time of the tasks.
    :rtype: float
    """
    return max(task.compute_time for task in tasks)

def get_max_priority_task(tasks: list[Task]) -> Task:
    """
    Get the task with the maximum priority.

    :param tasks: The list of tasks.
    :return: The task with the maximum priority.
    :rtype: Task
    """
    return max(tasks, key=lambda task: task.priority)

def get_min_priority_task(tasks: list[Task]) -> Task:
    """
    Get the task with the minimum priority.

    :param tasks: The list of tasks.
    :return: The task with the minimum priority.
    :rtype: Task
    """
    return min(tasks, key=lambda task: task.priority)


class SecondaryPeriod:
    """
    Secondary Period class for periodic tasks.
    This class is used to compute the time slots for cyclic scheduling.
    """

    @staticmethod
    def check_time_slot(ts: int, tasks: list[Task]) -> bool:
        """
        Check if the operation with ts works
        """
        return not any(SecondaryPeriod.compute_time_slot(ts, task.period) > task.deadline \
                       for task in tasks)

    @staticmethod
    def compute_time_slot(ts: int, period: int) -> int:
        """
        Calculation for `check_ts`
        """
        return 2 * ts - math.gcd(ts, period)

    @staticmethod
    def get_time_slots(tasks: list[Task], min_deadline: int, max_compute_time: float) -> list[int]:
        """
        Return a list of time slots between the minimum deadline and the maximum compute time.

        :param tasks: The list of tasks.
        :param min_deadline: The minimum deadline of the tasks.
        :param max_compute_time: The maximum compute time of the tasks.
        :return: A list of time slots.
        :rtype: list[int]
        """
        gen = range(min_deadline, math.ceil(max_compute_time) + 1) \
            if min_deadline < max_compute_time else \
            range(math.floor(max_compute_time), min_deadline + 1)

        time_slots = [time_slot for time_slot in gen if hyperperiod(tasks) % time_slot == 0]

        return time_slots


class ResponseTimeAnalysis:
    """
    Response Time Analysis (RTA) class for periodic tasks.
    """

    @staticmethod
    def get_higher_priority_tasks(task: Task, tasks: list[Task]) -> list[Task]:
        """
        Get the higher priority tasks of a task.

        :param task: The task to analyze.
        :param tasks: The list of tasks.
        :return: The higher priority tasks of the task.
        :rtype: list[Task]
        """
        return [t for t in tasks if t.priority > task.priority]

    @staticmethod
    def interference(task: Task, tasks: list[Task], response_time: float) -> float:
        """
        Calculate the interference of a task on another task.

        :param task: The task to analyze.
        :param tasks: The list of tasks. It is considered that this list is not parsed.
        :param response_time: The response time of the task.
        :return: The interference of the task on the other tasks.
        :rtype: float
        """

        # Faster exit
        if response_time == 0.0:
            return 0.0

        interference: float = 0.0
        tasks = ResponseTimeAnalysis.get_higher_priority_tasks(task, tasks)

        for t in tasks:
            interference += math.ceil(response_time / t.period) * t.compute_time

        return interference

    @staticmethod
    def iterate(w: float, task: Task, tasks: list[Task]) -> float:
        """
        Iterate to find the response time of a task.

        :param w: The current response time.
        :param task: The task to analyze.
        :param tasks: The list of tasks.
        :return: The new response time of the task.
        :rtype: float
        """

        return task.compute_time + ResponseTimeAnalysis.interference(task, tasks, w)

    @staticmethod
    def get_task_response_time(task: Task, tasks: list[Task]) -> float:
        """
        Check if the response time of a task is less than or equal to its deadline.

        :param task: The task to analyze.
        :param tasks: The list of tasks.
        :return: The response time of the task.
        :rtype: float
        """
        response_time = 0.0
        old_response_time = -1.0

        while response_time != old_response_time:
            old_response_time = response_time
            response_time = ResponseTimeAnalysis.iterate(response_time, task, tasks)

        return response_time

    @staticmethod
    def check_task_response_time(task: Task, tasks: list[Task]) -> bool:
        """
        Check if the response time of a task is less than or equal to its deadline.

        :param task: The task to analyze.
        :param tasks: The list of tasks.
        :return: True if the response time is less than or equal to the deadline, False otherwise.
        :rtype: bool
        """
        response_time = ResponseTimeAnalysis.get_task_response_time(task, tasks)
        return response_time <= task.deadline

    @staticmethod
    def check_response_time(tasks: list[Task]) -> bool:
        """
        Check if the response time of a task is less than or equal to its deadline.

        :param tasks: The list of tasks.
        :return: True if the response time is less than or equal to the deadline, False otherwise.
        :rtype: bool
        """

        return all(ResponseTimeAnalysis.check_task_response_time(task, tasks) for task in tasks)


class ProcessorDemandCriterion:
    """
    Processor Demand Criterion (PDC) class for periodic tasks.
    This class is used to check if a set of tasks can be scheduled using the processor demand criterion.
    """

    @staticmethod
    def get_l_star(tasks: list[Task]) -> float:
        """
        Get the L-start of a set of tasks.
        The L-start is the maximum period of the tasks.

        :param tasks: The list of tasks.
        :return: The L-start of the tasks.
        :rtype: float
        """
        total = 0.0
        for task in tasks:
            total += (task.period - task.deadline) * task.utilization

        return total / (1 - total_utilization(tasks))

    @staticmethod
    def get_max_time_slot(tasks: list[Task]) -> int:
        """
        Get the maximum time slot of a set of tasks.

        :param tasks: The list of tasks.
        :return: The maximum time slot of the tasks.
        :rtype: float
        """
        return min(hyperperiod(tasks), math.ceil(ProcessorDemandCriterion.get_l_star(tasks)))

    @staticmethod
    def get_time_slots(tasks: list[Task]) -> list[int]:
        """
        Return a list of time slots that must be checked
        """

        max_time = ProcessorDemandCriterion.get_max_time_slot(tasks)
        time_slots: set[int] = set()

        for task in tasks:
            k = 0
            absolute_deadline = ProcessorDemandCriterion.get_absolute_deadline(task, k)

            while absolute_deadline <= max_time:
                time_slots.add(absolute_deadline)
                k += 1
                absolute_deadline = ProcessorDemandCriterion.get_absolute_deadline(task, k)

        return sorted(time_slots)

    @staticmethod
    def get_absolute_deadline(task: Task, k: int, initial_phase: int = 0) -> int:
        """
        Get the absolute deadline of a task at a given time.

        :param task: The task to analyze.
        :param k: The number of periods to check.
        :param initial_phase: The initial phase of the task.
        :return: The absolute deadline of the task at the given time.
        :rtype: int
        """
        return task.period * k + task.deadline + initial_phase

    @staticmethod
    def get_contribution_simplified(task: Task, l_star: float) -> float:
        """
        Get the contribution of a task at a given time.
        This method uses a simplified version of the contribution calculation.
        It only works when t1 = 0 and t2 = l_star.

        :param task: The task to analyze.
        :param l_star: The L-start of the tasks.
        :return: The contribution of the task at the given time.
        """
        a = l_star + task.period - task.deadline
        return math.floor(a/task.period) * task.compute_time

    @staticmethod
    def get_contribution(task: Task, t1: int, t2: int) -> float:
        """
        Get the contribution of a task at a given time.

        :param task: The task to analyze.
        :param t1: The start time.
        :param t2: The end time.
        :return: The contribution of the task at the given time.
        """
        if t1 > t2:
            raise ValueError("t1 must be less than t2"
                             f"\nt1: {t1}"
                             f"\nt2: {t2}")

        a = math.floor((t2 + task.period - task.deadline) / task.period)
        b = math.ceil(t1 / task.period)
        return max(0, a - b) * task.compute_time

    @staticmethod
    def get_g(tasks: list[Task], t1: int, t2: int) -> float:
        """
        Get the g value of a task at a given time.

        :param tasks: The list of tasks.
        :param t1: The start time.
        :param t2: The end time.
        :return: The g value of the task at the given time.
        :rtype: float
        """

        if t1 > t2:
            raise ValueError("t1 must be less than t2"
                             f"\nt1: {t1}"
                             f"\nt2: {t2}")

        l_star = ProcessorDemandCriterion.get_l_star(tasks)
        total = 0.0

        if t1 == 0 and t2 == l_star:
            for task in tasks:
                total += ProcessorDemandCriterion.get_contribution_simplified(task, l_star)
        else:
            for task in tasks:
                total += ProcessorDemandCriterion.get_contribution(task, t1, t2)

        return total

    @staticmethod
    def check_g(tasks: list[Task], t1: int, t2: int) -> bool:
        """
        Check if the g value of a task at a given time is less than or equal to the time.

        :param tasks: The list of tasks.
        :param t1: The start time.
        :param t2: The end time.
        :return: True if the g value is less than or equal to the time, False otherwise.
        :rtype: bool
        """
        l_star = ProcessorDemandCriterion.get_l_star(tasks)
        return ProcessorDemandCriterion.get_g(tasks, t1, t2) <= l_star

"""
cyclic_executive.py

Small script I wrote for real-time systems that generates a cyclic executive
schedule given a set of independent tasks.

The script does not support more complicated situations such as split tasks
or periodic tasks with a non-zero phase.

Tasks are passed into functions such as suitable_frametimes,
feasible_schedule, and cyclic_executive as a variable amount of tuple
arguments.

Each tuple argument is a task in the form of (e, p, d*) where e is the
execution time of the task, p is the period of the task, and d (optional) is
the relative deadline of the task.

An example usage of cyclic_executive is the following:
    >>> cyclic_executive((6,2), (12,2), (36,4))
        {
            "tasks": [(6,2), (12,2), (36,4)],
            "slack_times": [0,0,2,0,4,2,0,4,2]
            "total_slack": 14,
            "hyperperiod": 36,
            "schedule": [
                [0,1],
                [2],
                [0],
                [0,1],
                [],
                [0],
                [0,1],
                [],
                [0],
            ]
        }

This is a call to construct a schedule given the tasks:
    T_1 = (p_1 = 6, e_1 = 2, d_1 = 6),
    T_2 = (p_2 = 12, e_2 = 2, d_2 = 4),
    T_3 = (p_3 = 36, e_3 = 4, d_3 = 36)
The returned cyclic executive schedule has a hyperperiod of 36, a frame size
of 4, and is as folllows:
    Frame 0: J_1,1, J_2,1
    Frame 1: J_3,1
    Frame 2: J_1,2
    Frame 3: J_1,3, J_2,2
    Frame 4:
    Frame 5: J_1,4
    Frame 6: J_1,5, J_2,3
    Frame 7:
    Frame 8: J_1,6

"""

import math
import itertools
import functools
import operator


def lcm(a, b):
    return a * b // math.gcd(a, b)


def factors(n):
    # prime factors of a number in a list form that shows
    # repeated prime factors
    def prime_factors(n):
        ret = []
        # Print the number of two's that divide n
        while n % 2 == 0:
            ret.append(2)
            n = n / 2
        # n must be odd at this point
        # so a skip of 2 ( i = i + 2) can be used
        for i in range(3, int(math.sqrt(n)) + 1, 2):
            # while i divides n , print i ad divide n
            while n % i == 0:
                ret.append(i)
                n = n / i
        # Condition if n is a prime
        # number greater than 2
        if n > 2:
            ret.append(n)
        return ret

    # produce power set of prime factors
    # multiplicatively accumulate each element in power set
    # add to return set
    lst = prime_factors(n)
    pwst = itertools.chain.from_iterable(
        map(lambda l: itertools.combinations(lst, l), range(len(lst) + 1)))
    return set(map(lambda i: int(functools.reduce(operator.mul, i, 1)), pwst))


def period(task):
    return task[0]


def exec_time(task):
    return task[1]


def deadline(task):
    return task[2] if len(task) == 3 else task[0]


def release_time(task, n):
    return period(task) * n


def utilization(task):
    return exec_time(task) / period(task)


# returns set of suitable frame times given tasks in variable argument list
# tasks are a tuple (p, e, d*) where p is period, e is execution time, and d,
# if included, is relative deadline
# e.g. suitable_frametimes((5, 1), (10, 2), (15, 3), (30, 3)) --> {3, 5}
def suitable_frametimes(*tasks):
    def condition_3(pd, f_time, d_line): return (2 * f_time - math.gcd(
        int(pd), int(f_time))) <= d_line

    # maximum exec time
    m_exec = max(map(exec_time, tasks))

    # all factors of periods which are greater than maximum execution time
    p2 = filter(lambda x: x >= m_exec,
                set.union(*map(lambda n: factors(n), map(period, tasks))))

    # all frame times for which condition 3 is met for all tasks
    p3 = filter(
        lambda f: all(
            map(lambda t: condition_3(period(t), f, deadline(t)), tasks)), p2)

    return set(map(int, p3))


# returns true if given set of tasks can be feasibly scheduled
# a set of tasks can be feasibly scheduled if their total utilization is <= 1
def feasible_schedule(*tasks):
    return sum(map(utilization, tasks)) <= 1


# returns a cyclic schedule given set of tasks if such a schedule is possible
# assumes no tasks has phase
# can pass in a keyword argument "frame_time" to specify a frame time if the schedule
# generated is too granular
def cyclic_executive(*tasks, **kwargs):
    frames = suitable_frametimes(*tasks)
    if len(frames) == 0:
        raise RuntimeError("No suitable frametime")

    if not feasible_schedule(*tasks):
        raise RuntimeError("Schedule infeasible")

    if "frame_time" in kwargs:
        frame_time = kwargs["frame_time"]
        if frame_time not in frames:
            raise RuntimeError("Passed in frame time is not suitable for cyclic executive.")
    else:
        frame_time = min(frames)

    hyperperiod = functools.reduce(lcm, map(period, tasks))
    num_frames = hyperperiod // frame_time

    # initialize available time and schedule
    available_time = [frame_time for _ in range(num_frames)]
    schedule = [list() for _ in range(num_frames)]

    # schedule tasks in order from most frequent to least frequent
    task_list = sorted(tasks, key=period)
    for j, t in enumerate(task_list):
        num_jobs = hyperperiod // period(t)
        for i in range(num_jobs):
            frame = math.ceil(release_time(t, i) / frame_time)
            scheduled = False
            c_off = 0
            # seek out a frame with enough available time to schedule job
            while (frame + c_off < num_frames) and (c_off * frame_time <= deadline(t) - exec_time(t)):
                c_fr = frame + c_off
                if available_time[c_fr] >= exec_time(t):
                    schedule[c_fr].append(j)
                    available_time[c_fr] = available_time[c_fr] - exec_time(t)
                    scheduled = True
                    break
                c_off = c_off + 1
            if not scheduled:
                raise RuntimeError(
                    "Could not schedule, job {} of task {} {} before its deadline or end of hyperperiod"
                    .format(i, j, t))

    return {
        "frame_time": frame_time,
        "tasks": task_list,
        "hyperperiod": hyperperiod,
        "schedule": schedule,
        "slack_times": available_time,
        "total_slack": sum(available_time)
    }

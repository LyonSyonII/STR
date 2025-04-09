from script.models.scheduler import Scheduler
from .cyclic import CyclicScheduler
from .rate_monotonic import RateMonotonicScheduler
from .deadline_monotonic import DeadlineMonotonicScheduler
from .earliest_deadline_first import EarliestDeadlineFirstScheduler

SCHEDULERS: dict[str, type[Scheduler]] = {
    "Cyclic": CyclicScheduler,
    "Rate Monotonic": RateMonotonicScheduler,
    "Deadline Monotonic": DeadlineMonotonicScheduler,
    "Earliest Deadline First": EarliestDeadlineFirstScheduler,
}

import math
from script.models.scheduler import Scheduler
from script.utils.task import SecondaryPeriod
from script.models.event import Event, Scheduling, TimeMark
from script.models.task import Task


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

    def get_valid_time_slots(self) -> list[int]:
        """
        Get the valid time slots for the cyclic scheduler.
        """
        time_slots = SecondaryPeriod.get_time_slots(self.tasks, self.min_deadline, self.max_compute_time)
        valid_time_slots = []

        for time_slot in time_slots:
            if SecondaryPeriod.check_time_slot(time_slot, self.tasks):
                valid_time_slots.append(time_slot)

        return valid_time_slots

    @property
    def time_slots(self) -> list[int]:
        """
        Get the time slots for the cyclic scheduler.
        """
        return self.get_valid_time_slots()

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

        return len(self.time_slots) > 0

    def is_schedulable(self) -> bool:
        """
        Check if the task can be scheduled using cyclic scheduling.
        """
        conditions = [self.condition1(), self.condition2()]
        return all(conditions)

    def sort_tasks(self):
        """
        Sort the tasks in the scheduler.
        Since the cyclic scheduler does not have a specific sorting order,
        use the period (ascending) as the default sorting order.
        """
        self.tasks.sort(key=lambda task: task.period)

    def get_task_start_times(self, task_id: int, until: float) -> list[Event]:
        """
        Get the start times for the tasks using cyclic scheduling.
        """
        task = self.get_task(task_id)

        if task is None:
            raise ValueError(f"Task {task_id} not found in the scheduler.")

        start_times: list[Event] = []
        accumulated_time = 0.0
        while accumulated_time < until:
            start_time = Event(start_time=accumulated_time, task=task)
            start_times.append(start_time)
            accumulated_time += task.period

        return start_times

    def get_start_times(self, until: float) -> list[Event]:
        """
        Get the start times for the tasks using cyclic scheduling.
        """

        start_times: list[Event] = []

        for task in self.tasks:
            task_start_times = self.get_task_start_times(task.task_id, until)
            start_times.extend(task_start_times)

        start_times.sort(key=lambda event: (event.start_time, event.task.task_id))
        return start_times


    def get_scheduling(self) -> Scheduling:
        """
        Get the scheduling for the tasks using cyclic scheduling.
        It only return one possible schedule, even if there are multiple possible schedules.
        """
        if not self.is_schedulable():
            return Scheduling(events=None)

        start_times = self.get_start_times(self.hyperperiod)
        print(f"Start times: {start_times}")

        # Sort the events in start_time so that they are ordered by:
        # 1. Start time (ascending)
        # 2. Task ID (ascending)
        #
        # Keep in mind that it also needs to be sorted according to their
        # compute time and their deadlines. Meaning a task that in can be
        # pushed back because other tasks precede it in the schedule
        accumulated_time = 0.0
        events: list[Event] = []
        pushed_back: list[Event] = start_times.copy()

        frame_time = min(self.time_slots)
        # frame_time = max(time_slots)
        num_frames = self.hyperperiod // frame_time

        # while len(pushed_back) > 0:
        #     event = pushed_back.pop(0)

        #     # Check if the task can be pushed back
        #     for other_events in start_times:
        #         if accumulated_time == 0.0:
        #             break

        #         if accumulated_time >= other_events.start_time:
        #             pushed_back.append(event)
        #             continue

        #     accumulated_time += event.task.compute_time

        #     # Check if between the start time and the end time of the task
        #     # there is a time mark (indicated by frame time)
        #     # If so, move the start time to the next time mark
        #     # and add the event to the new start times

        #     time_mark_in_between = False
        #     next_frame_number = 0
        #     for i in range(num_frames):
        #         if event.start_time <= frame_time * i < event.end_time:
        #             time_mark_in_between = True
        #             next_frame_number = i
        #             break

        #     if time_mark_in_between:
        #         # Move the start time to the next time mark
        #         event.start_time = frame_time * next_frame_number

        #     # Add the event to the new start times
        #     events.append(event)

        # # for start_time in start_times:

        # #     # Check if the task can be pushed back
        # #     for event in start_times:
        # #         if accumulated_time >= event.start_time:
        # #             pushed_back.append(event)
        # #             continue


        # # frame_time = min(self.time_slots)
        # # # frame_time = max(time_slots)
        # # num_frames = self.hyperperiod // frame_time

        frames: list[float] = [frame_time for _ in range(num_frames)]
        scheduled_tasks: list[list[Task]] = [[] for _ in range(num_frames)]
        events: list[Event] = []

        # # accumulated_time = 0.0
        # # for start_time in start_times:
        # #     task_start_time = start_time.start_time + accumulated_time
        # #     event = Event(start_time=task_start_time, task=start_time.task)
        # #     events.append(event)

        # #     accumulated_time = task_start_time + start_time.task.compute_time

        # for event in events:
        #     print(f"Task {event.task.task_id} scheduled: {event.start_time} - {event.end_time} ({event.task.compute_time})")

        # return Scheduling(events=events, frame_time=frame_time)

        for task in self.tasks:
            num_task_execution = self.hyperperiod // task.period

            for i in range(num_task_execution):
                task_release_time = task.period * i
                frame = math.ceil(task_release_time / frame_time)
                is_task_scheduled = False
                frame_offset = 0

                # Do while
                # 1. The frame is within the number of frames
                # 2. The frame time is less than the deadline - compute time
                # 3. The task is not scheduled yet
                print()
                print(f"At iteration {i} - task {task.task_id}: frame: {frame}")
                print(f"At iteration {i} - task {task.task_id}: frame_offset: {frame_offset}")
                print(f"At iteration {i} - task {task.task_id}: frame_time: {frame_time}")
                print(f"At iteration {i} - task {task.task_id}: num_frames: {num_frames}")
                print(f"At iteration {i} - task {task.task_id}: task.deadline: {task.deadline}")
                print(f"At iteration {i} - task {task.task_id}: task.compute_time: {task.compute_time}")
                print(f"At iteration {i} - task {task.task_id}: task_release_time: {task_release_time}")
                while ((frame + frame_offset) < num_frames) \
                    and ((frame_offset * frame_time) <= (task.deadline - task.compute_time)) \
                    and not is_task_scheduled:

                    current_frame = frame + frame_offset

                    print(f"At iteration {i} - task {task.task_id}: comparing value {frames[current_frame]} with {task.compute_time}")
                    if frames[current_frame] >= task.compute_time:
                        print(f"At iteration {i} - task {task.task_id}: allowed values")
                        scheduled_tasks[current_frame].insert(0, task)
                        frames[current_frame] = frames[current_frame] - task.compute_time
                        print(f"At iteration {i} - task {task.task_id}: Assigning to frame {current_frame} (time left: {frames[current_frame]}) compute time: {task.compute_time}")
                        is_task_scheduled = True
                        break

                    frame_offset += 1

                if not is_task_scheduled:
                    raise RuntimeError(f"Task {task.task_id} cannot be scheduled."
                                       "\n\nTask data:"
                                       f"\n\t- Task ID: {task.task_id}"
                                       f"\n\t- Compute Time: {task.compute_time}"
                                       f"\n\t- Deadline: {task.deadline}"
                                       f"\n\t- Period: {task.period}"
                                       "\n"
                                       f"\nCurrent frame: {frame}"
                                       f"\nCurrent frame offset: {frame_offset}"
                                       f"\nNumber of frames: {num_frames}"
                                       f"\nFrame time: {frame_time}"
                                       f"\nNumber of time that the task will execute: {num_task_execution}"
                                       f"\nCurrent task execution number: {i}"
                                       "\n"
                                       f"\nCurrent scheduled tasks:\n{scheduled_tasks}"
                                       "\n"
                                       f"\nCurrent frames:\n{frames}"
                                       f"\n"
                                       f"\nTasks in the scheduler:\n{self.tasks}")

        events: list[Event] = []
        current_time = 0.0
        for i, tasks in enumerate(scheduled_tasks):
            current_time = frame_time * i

            for task in tasks:
                print(f"Iteration {i} - task {task.task_id} - current time: {current_time}")
                event = Event(start_time=current_time, task=task)
                events.append(event)
                current_time += task.compute_time

        print(f"Frame time: {frame_time}")
        for event in events:
            print(f"Task {event.task.task_id} scheduled: {event.start_time} - {event.end_time} ({event.task.compute_time})")

        return Scheduling(events=events, frame_time=frame_time)

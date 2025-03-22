import matplotlib.pyplot as plt
import numpy as np
import math
from datetime import datetime

# local imports
from plot.config.logger_handler import LoggerHandler
from plot.enums.task_state import TaskState
from plot.models.motor_sensor import MotorSensorData
from plot.models.tasks_states import TasksState

class PlotUtilities:
    _logger = LoggerHandler.get_logger("plot")

    @staticmethod
    def plot_motor_data(motors_sensors: list[MotorSensorData],
                        output_file: str | None = None,
                        show: bool = False,
                        max_time: datetime | None = None):
        """
        Plot the motor data
        :param motors_sensors: list of MotorSensorData
        :param output_file: output file
        :param show: show the plot if set to True
        """
        if (max_time is not None):
            motors_sensors = [motor_sensor \
                              for motor_sensor in motors_sensors \
                              if motor_sensor['timestamp'] <= max_time]

        _, ax = plt.subplots()

        first_motor_sensor = motors_sensors[0]
        for i in range(1, len(motors_sensors)):
            start_motor_sensor = motors_sensors[i - 1]
            end_motor_sensor = motors_sensors[i]

            start_motor_time = start_motor_sensor['timestamp'] - first_motor_sensor['timestamp']
            end_motor_time = end_motor_sensor['timestamp'] - first_motor_sensor['timestamp']

            x = [start_motor_time.total_seconds() * 1000, end_motor_time.total_seconds() * 1000]
            a1 = [motors_sensors[i-1]['sensor_a1'], motors_sensors[i]['sensor_a1']]
            a2 = [motors_sensors[i-1]['sensor_a2'], motors_sensors[i]['sensor_a2']]
            a3 = [motors_sensors[i-1]['sensor_a3'], motors_sensors[i]['sensor_a3']]
            a4 = [motors_sensors[i-1]['sensor_a4'], motors_sensors[i]['sensor_a4']]
            a5 = [motors_sensors[i-1]['sensor_a5'], motors_sensors[i]['sensor_a5']]

            ax.plot(x, a1, 'b.-')
            ax.plot(x, a2, 'r.-')
            ax.plot(x, a3, 'c.-')
            ax.plot(x, a4, 'g.-')
            ax.plot(x, a5, 'k.-')

        ax.set_title('Motor sensors over time')
        ax.set_xlabel('Time (ms)')
        ax.set_ylabel('Sensor value')
        ax.legend(['a1', 'a2', 'a3', 'a4', 'a5'])

        if show:
            PlotUtilities._logger.warning("Showing plot of motor data")
            plt.ioff()
            plt.show()

        if output_file is not None:
            PlotUtilities._logger.warning(f"Saving plot of motor data to '{output_file}'")
            plt.savefig(output_file, dpi=200)

        plt.close()

    @staticmethod
    def plot_task_states(tasks_states: list[TasksState],
                         output_file: str | None = None,
                         show: bool = False,
                         max_time: datetime | None = None):
        """
        Plot the task states
        :param tasks_states: list of TasksState
        :param output_file: output file
        :param show: show the plot if set to True
        """

        if (max_time is not None):
            tasks_states = [task_state \
                            for task_state in tasks_states \
                            if task_state['timestamp'] <= max_time]

        first_task_state = tasks_states[0]
        times = [(task_state['timestamp'] - first_task_state['timestamp']).total_seconds() * 1000 for task_state in tasks_states]

        task1 = [task_state['task1_state'].value for task_state in tasks_states]
        task2 = [task_state['task2_state'].value for task_state in tasks_states]
        task3 = [task_state['task3_state'].value for task_state in tasks_states]
        task9 = [task_state['task9_state'].value for task_state in tasks_states]

        available_states = [TaskState.Running, TaskState.Ready, TaskState.Blocked, TaskState.Suspended]

        _, ax = plt.subplots()
        ax.step(times, task1, label="Read Hall", where="post")
        ax.step(times, np.array(task2) + len(available_states), label="Move Motor", where="post")
        ax.step(times, np.array(task3) + len(available_states)*2, label="Update Reference", where="post")
        ax.step(times, np.array(task9) + len(available_states)*3, label="Trace", where="post")

        # change the y-axis labels to the task states
        # Add more space to the left side of the plot so the labels are not cut off
        plt.subplots_adjust(left=0.2)
        ax.set_yticks(np.arange(0, len(available_states)*4, 1), labels=[state.name for state in available_states*4])
        ax.set_yticklabels([state.name for state in available_states*4])

        ax.set_title('Task states over time')
        ax.set_xlabel('Time (ms)')
        ax.set_ylabel('Task state')

        plt.subplots_adjust(bottom=0.2)
        ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), ncol=4)

        if show:
            PlotUtilities._logger.warning("Showing plot of task states")
            plt.ioff()
            plt.show()

        if output_file is not None:
            PlotUtilities._logger.warning(f"Saving plot of task states to '{output_file}'")
            plt.savefig(output_file, dpi=200)

        plt.close()

    @staticmethod
    def plot_debug_value(tasks_states: list[TasksState],
                         output_file: str | None = None,
                         show: bool = False,
                         max_time: datetime | None = None,
                         debug_label: str = "Debug value"):
        """
        Plots the debug value
        """

        if (max_time is not None):
            tasks_states = [task_state \
                            for task_state in tasks_states \
                            if task_state['timestamp'] <= max_time]

        interval = 1000
        steps = math.ceil(((tasks_states[-1]['timestamp'] - tasks_states[0]['timestamp']).total_seconds() * 1000) / interval) + 1

        PlotUtilities._logger.warning(f"A total of {steps} steps will be generated for the reference")

        reference_times = [int(i * interval) for i in range(steps)]
        reference_values = [-90 if i % 2 == 0 else 90 for i in range(steps)]

        _, ax = plt.subplots()
        ax.step(reference_times, reference_values, label="Reference", where="post", color="orange")

        first_task_state = tasks_states[0]
        for i in range(1, len(tasks_states)):
            start_task_state = tasks_states[i - 1]
            end_task_state = tasks_states[i]

            start_time = start_task_state['timestamp'] - first_task_state['timestamp']
            end_time = end_task_state['timestamp'] - first_task_state['timestamp']

            x = [start_time.total_seconds() * 1000, end_time.total_seconds() * 1000]
            y = [start_task_state['debug_value'], end_task_state['debug_value']]
            ax.plot(x, y, label=debug_label, color="blue", linestyle="-")

        # Alternative implementation using steps:
        #
        # task_timestamps: list[datetime] = [task_state['timestamp'] for task_state in tasks_states]
        # task_debug_values: list[float] = [task_state['debug_value'] for task_state in tasks_states]
        # t = np.array([(task_timestamp - task_timestamps[0]).total_seconds()*1000 for task_timestamp in task_timestamps])
        # tasks = np.array(task_debug_values)
        # ax.step(t, tasks, label=debug_label, where="post")

        ax.set_title(f"{debug_label} over time")
        ax.set_xlabel('Time (ms)')
        ax.set_ylabel(debug_label)
        ax.legend([debug_label, 'Reference'])

        if show:
            PlotUtilities._logger.warning("Showing plot of debug value")
            plt.ioff()
            plt.show()

        if output_file is not None:
            PlotUtilities._logger.warning(f"Saving plot of debug value to '{output_file}'")
            plt.savefig(output_file, dpi=200)

        plt.close()

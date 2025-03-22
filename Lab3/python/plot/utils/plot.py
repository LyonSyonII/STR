import matplotlib.pyplot as plt
import numpy as np

# local imports
from plot.config.logger_handler import LoggerHandler
from plot.enums.task_state import TaskState
from plot.models.motor_sensor import MotorSensorData
from plot.models.tasks_states import TasksState

class PlotUtilities:
    _logger = LoggerHandler.get_logger()

    @staticmethod
    def plot_motor_data(motors_sensors: list[MotorSensorData],
                        output_file: str | None = None,
                        show: bool = False):
        """
        Plot the motor data
        :param motors_sensors: list of MotorSensorData
        :param output_file: output file
        :param show: show the plot if set to True
        """
        _, ax = plt.subplots()

        for i in range(1, len(motors_sensors)):
            x = [i-1, i]
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

        ax.legend(['a1', 'a2', 'a3', 'a4', 'a5'])
        ax.set_title('Motor sensors data')

        if show:
            PlotUtilities._logger.warning("Showing plot of motor data")
            plt.ioff()
            plt.show()

        if output_file is not None:
            PlotUtilities._logger.warning(f"Saving plot of motor data to '{output_file}'")
            plt.savefig(output_file)

    @staticmethod
    def plot_task_states(tasks_states: list[TasksState],
                        output_file: str | None = None,
                        show: bool = False):
        """
        Plot the task states
        :param tasks_states: list of TasksState
        :param output_file: output file
        :param show: show the plot if set to True
        """

        _smallest_timestamp = min([task_state['timestamp'] for task_state in tasks_states])
        t = np.array([(task_state['timestamp'] - _smallest_timestamp).total_seconds() for task_state in tasks_states])
        task1 = np.array([task_state['task1_state'].value for task_state in tasks_states])
        task2 = np.array([task_state['task2_state'].value for task_state in tasks_states])
        task3 = np.array([task_state['task3_state'].value for task_state in tasks_states])
        task4 = np.array([task_state['task4_state'].value for task_state in tasks_states])
        task5 = np.array([task_state['task5_state'].value for task_state in tasks_states])
        task6 = np.array([task_state['task6_state'].value for task_state in tasks_states])

        _, ax = plt.subplots()
        ax.grid(True)
        ax.set_xlabel("Time (microseconds)")
        ax.set_ylabel("Task state")

        ax.step(t, task1.astype(float), label="$\tau_1$", where="post")
        ax.step(t, task2.astype(float) + 5, label="$\tau_2$", where="post")
        ax.step(t, task3.astype(float) + 10, label="$\tau_3$", where="post")
        ax.step(t, task4.astype(float) + 15, label="$\tau_4$", where="post")
        ax.step(t, task5.astype(float) + 20, label="$\tau_5$", where="post")
        ax.step(t, task6.astype(float) + 25, label="$\tau_6$", where="post")

        ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.05), ncol=6)
        ax.set_xticks(np.arange(0, max(t), 0.050))
        ax.set_yticks(np.arange(0, 35, 1), labels=[
            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
            TaskState.Suspended.name,

            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
            TaskState.Suspended.name,

            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
            TaskState.Suspended.name,

            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
            TaskState.Suspended.name,

            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
            TaskState.Suspended.name,

            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
            TaskState.Suspended.name,

            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
            TaskState.Suspended.name,

            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
            TaskState.Suspended.name,

            TaskState.Blocked.name,
            TaskState.Ready.name,
            TaskState.Running.name,
        ])

        ax.set_ylim(0, 35)
        ax.set_xlim(min(t), max(t))

        if show:
            PlotUtilities._logger.warning("Showing plot of task states")
            plt.ioff()
            plt.show()

        if output_file is not None:
            PlotUtilities._logger.warning(f"Saving plot of task states to '{output_file}'")
            plt.savefig(output_file)

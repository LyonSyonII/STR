""" Parser utilities """
from datetime import datetime, timedelta

from plot.enums.task_state import TaskState
from plot.models.motor_sensor import MotorSensorData
from plot.models.tasks_states import TasksState
from plot.config.logger_handler import LoggerHandler
from plot.config.parameter import START_TIME

class ParserUtilities:
    """
    Utilities to parse the serial data and return an object
    """

    _logger = LoggerHandler.get_logger()

    @staticmethod
    def _clean_data(data: str) -> str:
        """
        Clean the data by removing applying a series of transformations to it.

        - Remove leading and trailing whitespaces
        """
        return data.strip()

    @staticmethod
    def parse_motor_data(data: str) -> MotorSensorData | None:
        """
        Parse the motor data and return a MotorSensorData object.

        It's expected that the data is in the following format:

        ```
        {miliseconds},{sensor1_value},{sensor2_value},{sensor3_value},{sensor4_value},{sensor5_value},
        ```

        Where:

        - miliseconds: the time in miliseconds. It's expected to be an float with 2 decimal places
        - sensorX_value: the value of the sensor X. It's expected to be an integer
        """

        data = ParserUtilities._clean_data(data)

        try:
            sensor_values = data.split(",")
            miliseconds = float(sensor_values[0])
            sensor_a1 = int(sensor_values[1])
            sensor_a2 = int(sensor_values[2])
            sensor_a3 = int(sensor_values[3])
            sensor_a4 = int(sensor_values[4])
            sensor_a5 = int(sensor_values[5])

            timestamp = datetime(year=START_TIME.year, month=START_TIME.month, day=START_TIME.day) + timedelta(milliseconds=miliseconds)

            return MotorSensorData(timestamp=timestamp,
                                   sensor_a1=sensor_a1,
                                   sensor_a2=sensor_a2,
                                   sensor_a3=sensor_a3,
                                   sensor_a4=sensor_a4,
                                   sensor_a5=sensor_a5)

        except Exception as e:
            ParserUtilities._logger.error(f"Error while parsing motor data: {e}")
            return None

    @staticmethod
    def _get_task_state(state: int | bytes | str) -> TaskState | None:
        """
        Get the task state from the byte value

        The task state is the last 6 bits of the byte value
        """

        if (isinstance(state, bytes)):
            state = int.from_bytes(state, "big")

        elif (isinstance(state, str)):
            try:
                state = int(state)
            except Exception as e:
                state = int.from_bytes(state.encode(), "big")

        try:
            task_state = TaskState(state)
            return task_state
        except Exception as e:
            ParserUtilities._logger.error(f"Error while parsing task state: {e}")
            return None

    @staticmethod
    def parse_task_state(data: str) -> TasksState | None:
        """
        Parse the task state data and return a TasksStates object

        It's expected that the data is in the following format:

        ```
        {miliseconds},{task1_state},{task2_state},{task3_state},{task4_state},{task5_state},{task6_state},{debug_value}
        ```

        Where:

        - miliseconds: the time in miliseconds. It's expected to be an float with 2 decimal places
        - taskX_state: the state of the task X. It's expected to be an integer in bytes (not printable)
        - debug_value: a debug value. It's expected to be a random integer
        """

        data = ParserUtilities._clean_data(data)

        try:
            task_values = data.split(",")
            miliseconds = float(task_values[0])
            task1_state = ParserUtilities._get_task_state(task_values[1])
            task2_state = ParserUtilities._get_task_state(task_values[2])
            task3_state = ParserUtilities._get_task_state(task_values[3])
            task4_state = ParserUtilities._get_task_state(task_values[4])
            task5_state = ParserUtilities._get_task_state(task_values[5])
            task6_state = ParserUtilities._get_task_state(task_values[6])
            debug_value = task_values[7]

            if None in {task1_state, task2_state, task3_state, task4_state, task5_state, task6_state}:
                ParserUtilities._logger.error("Error while parsing task state"
                                              f"task1_state: {task1_state}"
                                              f"task2_state: {task2_state}"
                                              f"task3_state: {task3_state}"
                                              f"task4_state: {task4_state}"
                                              f"task5_state: {task5_state}"
                                              f"task6_state: {task6_state}")
                return None

            timestamp = datetime(year=START_TIME.year, month=START_TIME.month, day=START_TIME.day) + timedelta(milliseconds=miliseconds)

            return TasksState(timestamp=timestamp,
                               task1_state=task1_state,
                               task2_state=task2_state,
                               task3_state=task3_state,
                               task4_state=task4_state,
                               task5_state=task5_state,
                               task6_state=task6_state,
                               debug_value=debug_value) # type: ignore

        except Exception as e:
            ParserUtilities._logger.error(f"Error while parsing task state: {e}")
            return None

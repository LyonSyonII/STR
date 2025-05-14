import csv
from datetime import datetime

# local imports
from plot.enums.task_state import TaskState
from plot.config.parameter import CSVParameters
from plot.config.logger_handler import LoggerHandler
from plot.models.motor_sensor import MotorSensorData
from plot.models.tasks_states import TasksState

class CSVUtilities:
    _logger = LoggerHandler.get_logger("csv")

    @staticmethod
    def read_motor_data(input_file: str) -> list[MotorSensorData]:
        """
        Read the motor data from a CSV file.
        It's expected that the CSV file has the following columns:

        - timestamp
        - sensor_a1
        - sensor_a2
        - sensor_a3
        - sensor_a4
        - sensor_a5

        :param input_file: input file
        :return: list of MotorSensorData
        """

        motors_sensors: list[MotorSensorData] = []

        with open(input_file, 'r') as csvfile:
            CSVUtilities._logger.warning(f"Reading motor data from '{input_file}'")

            reader = csv.DictReader(csvfile,
                                    delimiter=CSVParameters.CSV_SEPARATOR.value,
                                    fieldnames=list(MotorSensorData.__annotations__.keys()))

            next(reader)  # skip the header

            for row in reader:
                try:
                    CSVUtilities._logger.debug(f"""
                        Reading motor data:
                        - timestamp: {row['timestamp']}
                        - sensor_a1: {row['sensor_a1']}
                        - sensor_a2: {row['sensor_a2']}
                        - sensor_a3: {row['sensor_a3']}
                        - sensor_a4: {row['sensor_a4']}
                        - sensor_a5: {row['sensor_a5']}
                    """)

                    timestamp: datetime = datetime.fromisoformat(row['timestamp'])
                    sensor_a1: int = int(row['sensor_a1'])
                    sensor_a2: int = int(row['sensor_a2'])
                    sensor_a3: int = int(row['sensor_a3'])
                    sensor_a4: int = int(row['sensor_a4'])
                    sensor_a5: int = int(row['sensor_a5'])

                    motor_data = MotorSensorData(timestamp=timestamp,
                                                 sensor_a1=sensor_a1,
                                                 sensor_a2=sensor_a2,
                                                 sensor_a3=sensor_a3,
                                                 sensor_a4=sensor_a4,
                                                 sensor_a5=sensor_a5)

                    motors_sensors.append(motor_data)
                except Exception as e:
                    CSVUtilities._logger.error(f"Error reading motor data:\n{e}")

        return motors_sensors

    @staticmethod
    def read_tasks_data(input_file: str) -> list[TasksState]:
        """
        Read the task states from a CSV file.
        It's expected that the CSV file has the following columns:

        - timestamp
        - task1_state
        - task2_state
        - task3_state
        - task9_state
        - debug_value

        :param input_file: input file
        :return: list of TasksState
        """

        tasks_states: list[TasksState] = []

        with open(input_file, 'r') as csvfile:
            CSVUtilities._logger.warning(f"Reading task states from '{input_file}'")

            reader = csv.DictReader(csvfile,
                                    delimiter=CSVParameters.CSV_SEPARATOR.value,
                                    fieldnames=list(TasksState.__annotations__.keys()))

            next(reader)  # skip the header

            for row in reader:
                try:
                    CSVUtilities._logger.debug(f"""
                        Reading task state:
                        - timestamp: {row['timestamp']}
                        - task1_state: {row['task1_state']}
                        - task2_state: {row['task2_state']}
                        - task3_state: {row['task3_state']}
                        - task9_state: {row['task9_state']}
                        - debug_value: {row['debug_value']}
                    """)

                    timestamp: datetime = datetime.fromisoformat(row['timestamp'])
                    task1_state: TaskState = TaskState.get_state_from_str(row['task1_state'])
                    task2_state: TaskState = TaskState.get_state_from_str(row['task2_state'])
                    task3_state: TaskState = TaskState.get_state_from_str(row['task3_state'])
                    task9_state: TaskState = TaskState.get_state_from_str(row['task9_state'])
                    debug_value: float = float(row['debug_value'])

                    task_state = TasksState(timestamp=timestamp,
                                            task1_state=task1_state,
                                            task2_state=task2_state,
                                            task3_state=task3_state,
                                            task9_state=task9_state,
                                            debug_value=debug_value)

                    tasks_states.append(task_state)

                except Exception as e:
                    CSVUtilities._logger.error(f"Error reading task state:\n{e}")

        return tasks_states

    @staticmethod
    def save_motor_data(motors_sensors: list[MotorSensorData], output_file: str):
        """
        Save the motor data to a CSV file
        :param motors_sensors: list of MotorSensorData
        :param output_file: output file
        """
        with open(output_file, 'w', newline='') as csvfile:
            CSVUtilities._logger.warning(f"Saving motor data to '{output_file}'")

            fieldnames = list(MotorSensorData.__annotations__.keys())
            CSVUtilities._logger.info(f"Writing using fieldnames: {fieldnames}")

            writer = csv.DictWriter(csvfile, fieldnames=fieldnames, delimiter=CSVParameters.CSV_SEPARATOR.value)

            writer.writeheader()
            for motor_sensor in motors_sensors:
                CSVUtilities._logger.debug(f"""
Writing motor data:
- timestamp: {motor_sensor['timestamp']}
- sensor_a1: {motor_sensor['sensor_a1']}
- sensor_a2: {motor_sensor['sensor_a2']}
- sensor_a3: {motor_sensor['sensor_a3']}
- sensor_a4: {motor_sensor['sensor_a4']}
- sensor_a5: {motor_sensor['sensor_a5']}
                                           """)
                writer.writerow(motor_sensor)

    @staticmethod
    def save_task_states(tasks_states: list[TasksState], output_file: str):
        """
        Save the task states to a CSV file
        :param tasks_states: list of TasksState
        :param output_file: output file
        """
        with open(output_file, 'w', newline='') as csvfile:
            CSVUtilities._logger.warning(f"Saving task states to '{output_file}'")

            fieldnames = list(TasksState.__annotations__.keys())
            CSVUtilities._logger.info(f"Writing using fieldnames: {fieldnames}")

            writer = csv.DictWriter(csvfile, fieldnames=fieldnames, delimiter=CSVParameters.CSV_SEPARATOR.value)

            writer.writeheader()
            for task_state in tasks_states:
                CSVUtilities._logger.debug(f"""
Writing task state:
- timestamp: {task_state['timestamp']}
- task1_state: {task_state['task1_state']}
- task2_state: {task_state['task2_state']}
- task3_state: {task_state['task3_state']}
- task9_state: {task_state['task9_state']}
- debug_value: {task_state['debug_value']}
                                           """)
                writer.writerow(task_state)

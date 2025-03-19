import csv

# local imports
from plot.config.parameter import CSV_SEPARATOR
from plot.config.logger_handler import LoggerHandler
from plot.models.motor_sensor import MotorSensorData
from plot.models.tasks_states import TasksState

class CSVUtilities:
    _logger = LoggerHandler.get_logger()

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

            writer = csv.DictWriter(csvfile, fieldnames=fieldnames, delimiter=CSV_SEPARATOR)

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

            writer = csv.DictWriter(csvfile, fieldnames=fieldnames, delimiter=CSV_SEPARATOR)

            writer.writeheader()
            for task_state in tasks_states:
                CSVUtilities._logger.debug(f"""
Writing task state:
- timestamp: {task_state['timestamp']}
- task1_state: {task_state['task1_state']}
- task2_state: {task_state['task2_state']}
- task3_state: {task_state['task3_state']}
- task4_state: {task_state['task4_state']}
- task5_state: {task_state['task5_state']}
- task6_state: {task_state['task6_state']}
- debug_value: {task_state['debug_value']}
                                           """)
                writer.writerow(task_state)

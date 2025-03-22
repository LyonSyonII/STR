import os
import asyncio
from datetime import datetime

# local imports
from plot.config.parameter import Environment, SerialHeaders, SerialParameters
from plot.config.logger_handler import LoggerHandler
from plot.utils.serial import SerialUtilities
from plot.utils.parser import ParserUtilities
from plot.utils.plot import PlotUtilities
from plot.utils.csv import CSVUtilities
from plot.models.motor_sensor import MotorSensorData
from plot.models.tasks_states import TasksState

_logger = LoggerHandler.get_logger()

async def read_serial_data() -> tuple[list[TasksState], list[MotorSensorData]]:
    """
    Read the data from the serial port and
    return the tasks states and motor sensors data
    """

    tasks_states: list[TasksState] = []
    motors_sensors: list[MotorSensorData] = []

    with SerialUtilities(port=SerialParameters.COM_PORT.value,
                         baud_rate=SerialParameters.BAUD_RATE.value,
                         serial_timeout=SerialParameters.SERIAL_TIMEOUT.value,
                         start_timeout=SerialParameters.START_TIMEOUT.value,
                         should_start=True) as ser:

        received_end_indicator = False

        while not received_end_indicator:
            data = ser.read_line_str()

            match data:
                case SerialHeaders.MOTOR_STATE.value:
                    data = ser.read_line_str()
                    _logger.info(f"Received motor data: {data}")

                    motor_data = ParserUtilities.parse_motor_data(data)

                    if motor_data is None:
                        _logger.warning(f"Unable to parse motor data: {data}")
                    else:
                        motors_sensors.append(motor_data)

                case SerialHeaders.TASK_STATE.value:
                    data = ser.read_line_str()
                    _logger.info(f"Received task state data: {data}")

                    task_data = ParserUtilities.parse_task_state(data)

                    if task_data is None:
                        _logger.warning(f"Unable to parse task data: {data}")
                    else:
                        tasks_states.append(task_data)

                case SerialHeaders.START.value:
                    _logger.warning(f"Start indicator received again: {data}")

                case SerialHeaders.END.value:
                    _logger.info(f"End indicator received: {data}")
                    received_end_indicator = True

                case _:
                    _logger.warning(f"Received unknown data: {data}")

    tasks_states.sort(key=lambda t: t["timestamp"])
    motors_sensors.sort(key=lambda t: t["timestamp"])

    return tasks_states, motors_sensors

async def save_data_to_csv(tasks_states: list[TasksState], motors_sensors: list[MotorSensorData], output_dir: str):
    """
    Saves the data into csv files
    """

    motor_sensor_file = os.path.join(output_dir, "motor_sensor.csv")
    task_state_file = os.path.join(output_dir, "task_state.csv")

    try:
        CSVUtilities.save_motor_data(motors_sensors, motor_sensor_file)
    except Exception as e:
        _logger.error(f"Unable to save motor data to CSV '{motor_sensor_file}:\n{e}")

    try:
        CSVUtilities.save_task_states(tasks_states, task_state_file)
    except Exception as e:
        _logger.error(f"Unable to save task states to CSV '{task_state_file}:\n{e}")

async def load_data_from_csv(output_dir: str) -> tuple[list[TasksState], list[MotorSensorData]]:
    """
    Load the data from the csv files.
    It expects the files to be named 'motor_sensor.csv' and 'task_state.csv'
    """

    motor_sensor_file = os.path.join(output_dir, "motor_sensor.csv")
    task_state_file = os.path.join(output_dir, "task_state.csv")

    tasks_states = []
    motors_sensors = []

    _logger.warning(f"Reading motor data from file '{motor_sensor_file}'")
    motors_sensors = CSVUtilities.read_motor_data(motor_sensor_file)

    _logger.warning(f"Reading task states from file '{task_state_file}'")
    tasks_states = CSVUtilities.read_tasks_data(task_state_file)

    return tasks_states, motors_sensors

async def plot_data(tasks_states: list[TasksState],
                    motors_sensors: list[MotorSensorData], output_dir: str,
                    max_time: datetime | None = None,
                    max_task_state_time: datetime | None = None):
    """
    Plot the data to the output directory
    """

    motor_sensor_file = os.path.join(output_dir, "motor_sensor.png")
    task_state_file = os.path.join(output_dir, "task_state.png")
    debug_task_state_file = os.path.join(output_dir, "task_state_debug.png")

    _logger.warning(f"Plotting motor data to file '{motor_sensor_file}'")
    PlotUtilities.plot_motor_data(motors_sensors, motor_sensor_file, max_time=max_time)

    _logger.warning(f"Plotting task states to file '{task_state_file}'")
    PlotUtilities.plot_task_states(tasks_states, task_state_file, max_time=max_task_state_time)

    _logger.warning(f"Plotting tasks states debug to file '{debug_task_state_file}'")
    PlotUtilities.plot_debug_value(tasks_states,
                                    debug_task_state_file,
                                    debug_label="Motor Angle",
                                    max_time=max_time)

async def main():
    """
    Main function to plot the data received from the serial port
    """

    data_dir = Environment.DATA_DIR.value
    tasks_states: list[TasksState] = []
    motors_sensors: list[MotorSensorData] = []
    max_time = datetime(year=2025,
                        month=3,
                        day=20,
                        hour=0,
                        minute=0,
                        second=10)
    max_task_state_time = datetime(year=2025,
                                   month=3,
                                   day=20,
                                   hour=0,
                                   minute=0,
                                   second=3)


    if (not SerialParameters.SKIP_SERIAL_READ.value):

        output_dir = os.path.join(Environment.OUTPUT_DIR.value,
                                  Environment.START_TIME.value.strftime("%Y-%m-%d_%H-%M-%S"))

        if (not os.path.exists(output_dir)):
            _logger.warning(f"Creating data directory: {output_dir}")
            os.makedirs(output_dir)

        if (Environment.DATA_DIR.value is None) \
            or (not os.path.exists(Environment.DATA_DIR.value)) \
            or (len(Environment.DATA_DIR.value) == 0) \
            or (Environment.DATA_DIR.value == ""):
            data_dir = output_dir

        tasks_states, motors_sensors = await read_serial_data()

        _logger.warning(f"Received {len(motors_sensors)} motor samples")
        for i, motor_sensor in enumerate(motors_sensors):
            _logger.info(f"Motor sensor {i}: {motor_sensor}")

        _logger.warning(f"Received {len(tasks_states)} task samples")
        for i, task_state in enumerate(tasks_states):
            _logger.info(f"Task state {i}: {task_state}")

        await save_data_to_csv(tasks_states, motors_sensors, output_dir)

        _logger.warning(f"Data saved to '{output_dir}'")

    else:
        if (not os.path.exists(Environment.DATA_DIR.value)):
            _logger.error(f"Data directory '{Environment.OUTPUT_DIR.value}' does not exist")
            return

        tasks_states, motors_sensors = await load_data_from_csv(data_dir)

        _logger.warning(f"Loaded {len(motors_sensors)} motor samples")
        _logger.warning(f"Loaded {len(tasks_states)} task samples")

    await plot_data(tasks_states, motors_sensors, data_dir, max_time, max_task_state_time)

if __name__ == '__main__':
    asyncio.run(main())

    _end_time = datetime.now()
    _logger.warning(f"Total execution time: {_end_time - Environment.START_TIME.value}")

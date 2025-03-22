import os
import asyncio
from datetime import datetime

# local imports
from plot.config.parameter import START_TIME, \
                                  BUFFER_SIZE, COM_PORT, BAUD_RATE, \
                                  START_INDICATOR, END_INDICATOR, TASK_STATE_INDICATOR, MOTOR_STATE_INDICATOR, \
                                  OUTPUT_DIR, \
                                  START_TIMEOUT, SERIAL_TIMEOUT, \
                                  PERFORMANCE_PROFILING, SerialArgs
from plot.config.logger_handler import LoggerHandler
from plot.utils.serial import SerialUtilities
from plot.utils.parser import ParserUtilities
from plot.utils.plot import PlotUtilities
from plot.utils.csv import CSVUtilities
from plot.models.motor_sensor import MotorSensorData
from plot.models.tasks_states import TasksState

_logger = LoggerHandler.get_logger()

async def main():
    """
    Main function to plot the data received from the serial port
    """
    _logger.debug(f"Connecting to '{COM_PORT}' at {BAUD_RATE} bauds\n"
                  f"Expected data size: {BUFFER_SIZE} samples\n"
                  f"Using start indicator: '{START_INDICATOR}'\n"
                  f"Using end indicator: '{END_INDICATOR}'\n"
                  f"Using task state indicator: '{TASK_STATE_INDICATOR}'\n"
                  f"Using motor state indicator: '{MOTOR_STATE_INDICATOR}'")


    tasks_states: list[TasksState] = []
    motors_sensors: list[MotorSensorData] = []

    with SerialUtilities(port=COM_PORT,
                         baud_rate=BAUD_RATE,
                         serial_timeout=SERIAL_TIMEOUT,
                         start_timeout=START_TIMEOUT,
                         should_start=True) as ser:

        received_end_indicator = False

        while not received_end_indicator:
            data = ser.read_line_str()

            match data:
                case value if value == MOTOR_STATE_INDICATOR:
                    data = ser.read_line_str()
                    _logger.info(f"Received motor data: {data}")

                    motor_data = ParserUtilities.parse_motor_data(data)

                    if motor_data is None:
                        _logger.warning(f"Unable to parse motor data: {data}")
                    else:
                        motors_sensors.append(motor_data)

                case value if value == TASK_STATE_INDICATOR:
                    data = ser.read_line_str()
                    _logger.info(f"Received task state data: {data}")

                    task_data = ParserUtilities.parse_task_state(data)

                    if task_data is None:
                        _logger.warning(f"Unable to parse task data: {data}")
                    else:
                        tasks_states.append(task_data)

                case SerialArgs.START:
                    _logger.warning(f"Start indicator received again: {data}")

                case value if value == END_INDICATOR:
                    _logger.info(f"End indicator received: {data}")
                    received_end_indicator = True

                case _:
                    _logger.warning(f"Received unknown data: {data}")

    _logger.warning(f"Received {len(motors_sensors)} motor samples")
    for i, motor_sensor in enumerate(motors_sensors):
        _logger.info(f"Motor sensor {i}: {motor_sensor}")

    _logger.warning(f"Received {len(tasks_states)} task samples")
    for i, task_state in enumerate(tasks_states):
        _logger.info(f"Task state {i}: {task_state}")

    output_subdir = os.path.join(OUTPUT_DIR, START_TIME.strftime("%Y-%m-%d_%H-%M-%S"))
    os.makedirs(output_subdir, exist_ok=True)

    motor_sensor_file = os.path.join(output_subdir, "motor_sensor.png")
    task_state_file = os.path.join(output_subdir, "task_state.png")
    motor_sensor_csv = os.path.join(output_subdir, "motor_sensor.csv")
    task_state_csv = os.path.join(output_subdir, "task_state.csv")

    try:
        CSVUtilities.save_motor_data(motors_sensors, motor_sensor_csv)
    except Exception as e:
        _logger.error(f"Unable to save motor data to CSV '{motor_sensor_csv}:\n{e}")

    CSVUtilities.save_task_states(tasks_states, task_state_csv)
    # try:
    #     CSVUtilities.save_task_states(tasks_states, task_state_csv)
    # except Exception as e:
    #     _logger.error(f"Unable to save task states to CSV '{task_state_csv}:\n{e}")

    try:
        PlotUtilities.plot_motor_data(motors_sensors, motor_sensor_file)
    except Exception as e:
        _logger.error(f"Unable to plot motor data to file '{motor_sensor_file}:\n{e}")

    try:
        PlotUtilities.plot_task_states(tasks_states, task_state_file)
    except Exception as e:
        _logger.error(f"Unable to plot task states to file '{task_state_file}:\n{e}")

if __name__ == '__main__':
    if (not PERFORMANCE_PROFILING):
        asyncio.run(main())
    else:
        from cProfile import Profile
        from pstats import SortKey, Stats

        with Profile() as pr:
            asyncio.run(main())
            stats = Stats(pr)
            stats.strip_dirs()
            stats.sort_stats(SortKey.TIME)
            stats.print_stats(15)

    _end_time = datetime.now()
    _logger.warning(f"Total execution time: {_end_time - START_TIME}")

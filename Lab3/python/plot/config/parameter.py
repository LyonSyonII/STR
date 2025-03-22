""" Configuration file for the move_domains project. """

# standard imports
from datetime import datetime
from enum import Enum
import os

class Environment(Enum):
    START_TIME = datetime.now()
    CWD = "."

    OUTPUT_DIR = os.path.join(CWD, "output")
    DATA_DIR = os.path.join(CWD, "output/2025-03-20_18-55-35")

    # Available log levels: debug, info, warning, error, critical
    LOG_LEVEL = "warning"
    LOG_TO_FILE = True

    PERFORMANCE_PROFILING = False

class CSVParameters(Enum):
    ENCODING = "utf-8"
    CSV_SEPARATOR = ","

class SerialHeaders(Enum):
    START = "=== START ==="
    END = "=== END ==="
    TASK_STATE = "DAT"
    MOTOR_STATE = "OSC"

class SerialParameters(Enum):
    SKIP_SERIAL_READ = True

    BUFFER_SIZE = 500
    COM_PORT = "COM5"
    BAUD_RATE = 115200

    START_TIMEOUT = 50
    SERIAL_TIMEOUT = 5

class PlotParameters(Enum):
    DPI = 200

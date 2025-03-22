""" Configuration file for the move_domains project. """

# standard imports
from datetime import datetime
import os

# third party imports
from dotenv import load_dotenv

# local imports
from .logger_handler import LoggerHandler

# Basic environment variables
START_TIME = datetime.now()
CWD = os.path.dirname(os.path.abspath(__file__))

load_dotenv(verbose=True)

# Serial port configuration
BUFFER_SIZE = int(os.getenv("BUFFER_SIZE", 10000))
COM_PORT = os.getenv("COM_PORT", "COM3")
BAUD_RATE = int(os.getenv("BAUD_RATE", 115200))

# Generated data configuration
OUTPUT_DIR = os.getenv("OUTPUT_DIR", os.path.join(CWD, "output"))
ENCODING = os.getenv("ENCODING", "utf-8")
CSV_SEPARATOR = os.getenv("CSV_SEPARATOR", ",")

# Logging configuration
LOG_LEVEL = str(os.getenv("LOG_LEVEL", "info"))
LOG_TO_FILE = bool(os.getenv("LOG_TO_FILE", True))

# Input data configuration
class SerialArgs():
    START = "===START==="


START_INDICATOR = os.getenv("START_INDICATOR", "===START===")
END_INDICATOR = os.getenv("END_INDICATOR", "===END===")
TASK_STATE_INDICATOR = os.getenv("TASK_STATE_INDICATOR", "DATA")
MOTOR_STATE_INDICATOR = os.getenv("MOTOR_STATE_INDICATOR", "OSC")

# Debugging utilities
PERFORMANCE_PROFILING = bool(os.getenv("PERFORMANCE_PROFILING", False))

# Timeouts
START_TIMEOUT = int(os.getenv("START_TIMEOUT", 5))
SERIAL_TIMEOUT = int(os.getenv("SERIAL_TIMEOUT", 5))

# setup the root logger
LoggerHandler.set_logger(level=LOG_LEVEL,
                         write_to_file=LOG_TO_FILE,
                         name="root")

import asyncio

from plot.config.parameter import Environment
from plot.config.logger_handler import LoggerHandler

async def _setup_loggers():
    """
    Setup the loggers
    """
    LoggerHandler.set_logger(level=Environment.LOG_LEVEL.value,
                            write_to_file=Environment.LOG_TO_FILE.value,
                            name="root")

    LoggerHandler.set_logger(level=Environment.LOG_LEVEL.value,
                             write_to_file=Environment.LOG_TO_FILE.value,
                             name="plot")

    LoggerHandler.set_logger(level=Environment.LOG_LEVEL.value,
                             write_to_file=Environment.LOG_TO_FILE.value,
                             name="serial")

    LoggerHandler.set_logger(level=Environment.LOG_LEVEL.value,
                             write_to_file=Environment.LOG_TO_FILE.value,
                             name="parser")

    LoggerHandler.set_logger(level=Environment.LOG_LEVEL.value,
                             write_to_file=Environment.LOG_TO_FILE.value,
                             name="csv")

async def setup():
    """
    Initial setup for the project
    """

    await _setup_loggers()

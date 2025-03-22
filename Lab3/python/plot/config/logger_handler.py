""" Handles the logging configuration. """

from typing import Union
from logging import getLogger, \
                    INFO, DEBUG, WARNING, ERROR, CRITICAL, \
                    StreamHandler, FileHandler, Formatter
from datetime import datetime
import os

class LoggerHandler:
    """
    Handles the logging configuration.
    """
    _levels = {
        "debug": DEBUG,
        "info": INFO,
        "warn": WARNING,
        "warning": WARNING,
        "error": ERROR,
        "critical": CRITICAL,
    }

    # key: logger name
    # value: if logger ever set
    _loggers: dict[str, bool] = {}

    @staticmethod
    def get_logger(name: str = "root"):
        """
        Returns a logger with the specified name.

        :param name: The name of the logger.
        :type name: str
        """

        if (name not in LoggerHandler._loggers):
            LoggerHandler.set_logger(name)

        return getLogger(name)

    @staticmethod
    def set_logger(name: str = "root",
                   level: Union[str, int] = "info",
                   base_dir: str = '',
                   write_to_file: bool = True,
                   encoding: str = 'utf-8-sig'):
        """
        Returns a logger with the specified name and log level.

        :param name: The name of the logger.
        :type name: str
        :param level: The log level.
        :type level: str
        :param base_dir: The base directory for the log file.
        :type base_dir: str
        :param encoding: The encoding for the log file.
        :type encoding: str
        """

        if (isinstance(level, str)):
            level = LoggerHandler._get_log_level(level)

        # Create the logger
        logger = getLogger(name)

        # Create the file handler
        log_dir = LoggerHandler._log_directory(base_dir)
        file_name = f"{datetime.now().strftime("%Y-%m-%d_%H-%M-%S")}.log"
        log_file = os.path.join(log_dir, file_name)

        # Create the console handler
        console_handler = StreamHandler()
        console_handler.setLevel(level)
        console_handler.set_name("console_handler")

        # Create the formatter and add it to the handlers
        formatter = Formatter(fmt="\n[%(levelname)s] %(asctime)s.%(msecs)03d - %(name)s - %(filename)s:%(lineno)d (%(funcName)s)"
                                  "\n%(message)s",
                              datefmt="%Y-%m-%dT%H:%M:%S", style='%')

        console_handler.setFormatter(formatter)

        # Remove all handler from the logger
        # This is done to prevent duplicate logs
        for handler in logger.handlers:
            logger.removeHandler(handler)

        logger.addHandler(console_handler)

        # Add a file handler if needed
        if (write_to_file):
            file_handler = FileHandler(
                log_file,
                encoding=encoding,
                errors='ignore'
            )
            file_handler.setLevel(DEBUG)
            file_handler.setFormatter(formatter)
            file_handler.set_name("file_handler")
            logger.addHandler(file_handler)

        # for handler in logger.handlers:
        #     print(handler.get_name())

        logger.setLevel(DEBUG)

        # Set the logger
        LoggerHandler._loggers[name] = True

    @staticmethod
    def _get_log_level(level: str):
        """
        Returns the log level based on the environment variable.
        """
        return LoggerHandler._levels.get(level, INFO)

    @staticmethod
    def get_log_level(logger_name: str):
        """
        Returns the log level.
        """
        if (logger_name not in LoggerHandler._loggers):
            return -1

        _logger = getLogger(logger_name)
        return _logger.getEffectiveLevel()

    @staticmethod
    def _log_directory(base_dir: str):
        """
        Handles the creation of the log directory.
        """
        log_dir = os.path.join(base_dir, ".logs")
        if (not os.path.exists(log_dir)):
            os.makedirs(log_dir)

        return log_dir

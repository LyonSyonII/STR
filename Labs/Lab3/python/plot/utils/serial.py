""" Serial utilities """

import serial
import serial.tools.list_ports
import time
from datetime import datetime

# local imports
from plot.config.parameter import SerialHeaders
from plot.config.logger_handler import LoggerHandler

class SerialUtilities:
    """
    Serial utilities
    """

    _port: str
    _baud_rate: int
    _serial: serial.Serial

    _logger = LoggerHandler.get_logger("serial")

    def __init__(self,
                 port: str,
                 baud_rate: int,
                 serial_timeout: int,
                 start_timeout: int,
                 should_start: bool = False):
        """
        Constructor

        :param port: serial port
        :type port: str
        :param baud_rate: baud rate
        :type baud_rate: int
        :param timeout: timeout in seconds for connection
        :type timeout: int
        """
        self._port = port
        self._baud_rate = baud_rate
        self._serial_timeout = serial_timeout
        self._start_timeout = start_timeout

        available_serial_ports = [
            port.device for port in serial.tools.list_ports.comports()]
        if (port not in available_serial_ports):
            raise Exception(f"Serial port '{port}' not available"
                            f"\nAvailable ports: {available_serial_ports}")

        self._logger.debug(f"Connecting to serial port '{self._port}' with baud rate {self._baud_rate}")

        self._serial = serial.Serial(self._port,
                                     self._baud_rate,
                                     timeout=self._serial_timeout)

        self._logger.info(f"Connected to serial port '{self._port}' at {self._baud_rate} bauds")

        if should_start:
            started = self.wait_for_start_signal(self._start_timeout, SerialHeaders.START.value)

            if not started:
                raise Exception(f"Start signal '{SerialHeaders.START.value}' not received after {self._start_timeout} seconds")

            self._logger.info("Start signal received successfully.")

    def __enter__(self):
        """
        Context manager enter
        """
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """
        Context manager exit
        """

        if (self._serial is not None) and (self._serial.is_open):
            self._logger.info(f"Closing serial port '{self._port}'")
            self._serial.close()

    def __del__(self):
        """
        Destructor
        """
        self._serial.close()

    def wait_for_start_signal(self, timeout: int, header: str) -> bool:
        """
        Wait for the start signal.
        This is a blocking function.

        :param timeout: timeout in seconds
        :type timeout: int
        :return: True if the start signal was received, False otherwise
        :rtype: bool
        """

        _start_time = datetime.now()
        timed_outed = False

        self._logger.debug(
            f"Looking for start signal: '{header}' for {timeout} seconds")

        while not timed_outed:
            if (datetime.now() - _start_time).seconds > timeout:
                timed_outed = (timeout > 0) and True

            read_line = self.read_line_str()

            if read_line == header:
                self._logger.info(f"After {datetime.now() - _start_time}, received start signal: '{header}'")
                return True

            self._logger.warning(f"Waiting for start signal: '{header}'")

            time.sleep(1)

        self._logger.error(f"After {datetime.now() - _start_time}, did not receive start signal: '{header}'")

        return False

    def read_line_str(self, encoding: str = "utf-8", errors: str = "ignore") -> str:
        """
        Read a line from the serial port

        :param encoding: encoding
        :type encoding: str
        :param error: error handling. Default is 'strict'. Other options are 'ignore' and 'replace'
        :type error: str
        :return: line read
        :rtype: str
        """
        data = self._serial.readline().decode(encoding, errors=errors).strip()
        self._logger.debug(f"Read: {data}")
        return data

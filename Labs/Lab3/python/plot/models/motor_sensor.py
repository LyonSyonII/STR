from typing import TypedDict
from datetime import datetime

class MotorSensorData(TypedDict):
    timestamp: datetime
    sensor_a1: int
    sensor_a2: int
    sensor_a3: int
    sensor_a4: int
    sensor_a5: int

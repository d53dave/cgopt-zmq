import psutil
from collections import deque
from statistics import mean
from typing import List


def _average_values(lst: List[float]) -> float:
    return mean(lst)


class Stats:
    def __init__(self, response_avg_count=3):
        self.workers = []
        self.average_reponse_time = 0
        self.response_times = deque(response_avg_count*[0], response_avg_count) 

    def update(self):
        self.cpu_utilization = psutil.cpu_percent(interval=None, percpu=True)
        self.mem = psutil.virtual_memory()
        self.average_reponse_time = Stats._average_values(self.response_times)

        return self

    def add_response_time(self, response_time):
        self.response_times.appendleft(float(response_time))

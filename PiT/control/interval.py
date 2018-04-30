import numpy as np

class Interval(object):
    """Simple time interval representation.
    """
    def __init__(self, dt, start, n):
        self.dt = dt
        self.start = start
        self.n = n
        return

    def generate(self):
        return self.dt * (np.arange(self.n+1) + self.start)
    
    def subdivide(self, num_time_slices):
        if self.n % num_time_slices:
            raise ValueError("time interval cannot be subdivided exactly")
        
        slice_steps = self.n / num_time_slices
        return [Interval(self.dt, self.start + slice_steps*i, slice_steps) for i in range(num_time_slices)]

    def to_dict(self):
        return {
            'start': self.start,
            'n': self.n,
            'dt': self.dt
            }

    @classmethod
    def from_dict(cls, d):
        return cls(d['dt'], d['start'], d['n'])
    
    pass

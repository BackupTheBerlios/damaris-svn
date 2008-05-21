import math

__all__ = ['rotate_signal']


def rotate_signal(timesignal, angle):
    "Rotate <timesignal> by <angle> degrees"
    # implicit change to float arrays!
    if timesignal.get_number_of_channels()!=2:
        raise Exception("rotation defined only for 2 channels")
    # simple case 0, 90, 180, 270 degree
    reduced_angle=divmod(angle, 90)
    if abs(reduced_angle[1])<1e-6:
        reduced_angle=reduced_angle[0]%4
        if reduced_angle==0:
            return
        elif reduced_angle==1:
            timesignal.y[1]*=-1            
            timesignal.y=[timesignal.y[1],timesignal.y[0]]
        elif reduced_angle==2:
            timesignal.y[0]*=-1
            timesignal.y[1]*=-1
        elif reduced_angle==3:
            timesignal.y[0]*=-1            
            timesignal.y=[timesignal.y[1],timesignal.y[0]]
    else:
        sin_angle=math.sin(angle/180.0*math.pi)
        cos_angle=math.cos(angle/180.0*math.pi)
        timesignal.y=[cos_angle*timesignal.y[0]-sin_angle*timesignal.y[1],
                      sin_angle*timesignal.y[0]+cos_angle*timesignal.y[1]]

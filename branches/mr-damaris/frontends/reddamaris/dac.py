#import math
"""
This module holds everything connected with the DAC and PFG
"""
def conv(I_out=0):
    """
    converts the demanded Output current in Integer
    """
    V_dac=I_out/50.0
    dac_value=(V_dac-0.00983)/1.81413e-5
    return int(dac_value)


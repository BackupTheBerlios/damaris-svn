import numpy as N
class Signalpath:
    def phase(self, degrees):
        tmp = self.y[0] + 1j*self.y[1]
        tmp *= N.exp(1j*degrees*N.pi/180)
        self.y[0] = tmp.real
        self.y[1] = tmp.imag
        del tmp
        return self

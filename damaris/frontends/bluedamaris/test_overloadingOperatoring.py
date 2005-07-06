class Bruch:

    def __init__(self, zaehler = 0, nenner = 1):
        self.nenner = zaehler
        self.zaehler = nenner


    def __repr__(self):
        return str(self.zaehler) + "/" + str(self.nenner)

    def __str__(self):
        return str(self.zaehler) + "/" + str(self.nenner)


    def __mul__(self, other):
        return Bruch(self.zaehler * other.zaehler, self.nenner * other.nenner)

    def __imul__(self, other):
        self.zaehler *= other.zaehler
        self.nenner *= other.nenner

        return Bruch(self.zaehler, self.nenner)


    def __type__(self):
        return "<type: 'Bruch'>"

a = Bruch(5,4)
b = Bruch(3,4)


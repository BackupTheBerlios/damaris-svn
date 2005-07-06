# -*- coding: cp1252 -*-
def test():
    i = 0
    while i < 5:
        yield "Schleife 1"
        i+=1

    while i < 10:
        yield "Schleife 2"
        i+=1

    return
    

try:
    for stri in test():
        print stri
except:
    raise

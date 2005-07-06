import time
import random
from numarray import *

start = time.clock()
end = time.clock()

a = []
b = []
c = []
d = []

for i in range(65536):
    a.append(random.randint(0,65536))
    b.append(random.randint(0,65536))
    c.append(random.randint(0,65536))
    d.append(random.randint(0,65536))

aa = array(a, type="Int16")
ba = array(b, type="Int16")
ca = array(c, type="Int16")
da = array(d, type="Int16")

start = time.clock()

aa_copy = array(aa, type="Int16")
ba_copy = array(ba, type="Int16")
ca_copy = array(ca, type="Int16")
da_copy = array(da, type="Int16")

end = time.clock()

print "Die Zeit zum kopieren von 4*65536 Samples dauerte: %fms" % ((end - start)*1000)

import numpy as N
import sys
if sys.version_info > (2,6,0):
	import numbers
else:
	pass

if sys.version_info > (2,6,0):
	def lin_range(start,stop,step):
		if isinstance(step, numbers.Integral):
			return N.linspace(start,stop,step)
		else:
			return N.arange(start,stop,step)
else:
	def lin_range(start,stop,step):
		return N.arange(start,stop,step)



def log_range(start, stop, stepno):
	if (start<=0 or stop<=0 or stepno<1):
		raise ValueError("start, stop must be positive and stepno must be >=1")
	return N.logspace(N.log10(start),N.log10(stop), num=stepno)


def staggered_range(some_range, size=3):
	m=0
	if isinstance(some_range, N.ndarray):
		is_numpy = True
		some_range = list(some_range)
	new_list=[]
	for k in xrange(len(some_range)):
		for i in xrange(size):
			try:
				index = (m*size)
				new_list.append(some_range.pop(index))
			except IndexError:
				break
		m+=1
	if is_numpy:
		new_list = N.asarray(new_list+some_range)
	else:
		new_list+=some_range
	return new_list
		

def combine_ranges(*ranges):
    new_list = []
    for r in ranges:
        new_list.extend(r)
    return new_list

combined_ranges=combine_ranges

def interleaved_range(some_list, left_out):
	"""
	in first run, do every n-th, then do n-1-th of the remaining values and so on...
	"""
	m=0
	new_list = []
	for j in xrange(left_out):
		for i in xrange(len(some_list)):
			if (i*left_out+m) < len(some_list):
				new_list.append(some_list[i*left_out+m])
			else:
				m+=1
				break
	if isinstance(some_list, N.ndarray):
		new_list = N.array(new_list) 
	return new_list


# These are the generators
def lin_range_iter(start,stop, step):
    this_one=float(start)+0.0
    if step>0:
        while (this_one<=float(stop)):
            yield this_one
            this_one+=float(step)
    else:
        while (this_one>=float(stop)):
            yield this_one
            this_one+=float(step)
        

def log_range_iter(start, stop, stepno):
    if (start<=0 or stop<=0 or stepno<1):
        raise ValueError("start, stop must be positive and stepno must be >=1")
    if int(stepno)==1:
        factor=1.0
    else:
        factor=(stop/start)**(1.0/int(stepno-1))
    for i in xrange(int(stepno)):
        yield start*(factor**i)

def staggered_range_iter(some_range, size = 1):
    """
    size=1: do one, drop one, ....
    size=n: do 1 ... n, drop n+1 ... 2*n
    in a second run the dropped values were done
    """
    left_out=[]
    try:
        while True:
            for i in xrange(size):
                yield some_range.next()
            for i in xrange(size):
                left_out.append(some_range.next())
    except StopIteration:
        pass
    
    # now do the droped ones
    for i in left_out:
        yield i

def combined_ranges_iter(*ranges):
    """
    iterate over one range after the other
    """
    for r in ranges:
        for i in r:
            yield i

combine_ranges_iter=combined_ranges_iter


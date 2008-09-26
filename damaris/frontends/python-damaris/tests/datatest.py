import sys
import numpy
import math
import os.path
import unittest

print "running tests on modules in source directory"
# assume, script is in tests directory and we are testing modules in src
sys.path[0:0]=[os.path.join(os.path.dirname(sys.argv[0]), "..", "src", "data")]
from MeasurementResult import *

class TestAccumulatedValueClass(unittest.TestCase):
	
	def setUp(self):
		# is called before each test
		pass
	
	def testInitialization_Empty(self):
		a=AccumulatedValue()
		self.assert_(a.mean() is None)
		self.assert_(a.mean_error() is None)
		self.assert_(a.sigma() is None)
		self.assert_(a.n==0)
	
	def testInitialization_1Value(self):
		a=AccumulatedValue(0)
		self.assert_(a.mean()==0)
		self.assertAlmostEqual(a.mean_error(),0)
		self.assertAlmostEqual(a.sigma(),0)
		self.assert_(a.n==1)

	def testInitialization_2Values(self):
		a=AccumulatedValue(1, 0.1)
		self.assert_(a.mean(),1.0)
		self.assertAlmostEqual(a.mean_error(), 0.1)
		self.assertAlmostEqual(a.sigma(), 0.1*math.sqrt(2.0))
		self.assert_(a.n==2)
	
	def testInitialization_3Values(self):
		a=AccumulatedValue(1, 0.1, 10)
		self.assertAlmostEqual(a.mean(),1)
		self.assertAlmostEqual(a.mean_error(), 0.1)
		self.assertAlmostEqual(a.sigma(), 0.1*math.sqrt(10.0))
		self.assert_(a.n==10)

	def testStatistics(self):
		test_dataset=numpy.arange(10.0)
		a=AccumulatedValue()
		for i in test_dataset:
			a+=i
		self.assert_(a.n==len(test_dataset))
		# sum x_i/n
		self.assertAlmostEqual(a.mean(), test_dataset.mean())
		# std_dev_n-1 x_i= sqrt(sum (x-xmean)**2/(n-1))
		self.assertAlmostEqual(a.sigma(), math.sqrt(((test_dataset-a.mean())**2).sum()/(len(test_dataset)-1.)))
		# std_dev_n-1 x_i/sqrt(n)
		self.assertAlmostEqual(a.mean_error(), a.sigma()/math.sqrt(len(test_dataset)))
		
	def tearDown(self):
		# is called after each test
		pass

class TestMeasurementResult(unittest.TestCase):

	def testImplicitCast(self):
		# check wether other data types will be converted to AccumulatedValue
		m=MeasurementResult("TestData")
		m[1.0]
		m[2.0]=2
		self.assert_(isinstance(m[1.0], AccumulatedValue))
		self.assert_(isinstance(m[2.0], AccumulatedValue))
	
	def testUninitalizedEntries(self):
		# assure that entries with no data are listed as xdata
		m=MeasurementResult("TestData")
		a=m[2.0]
		self.assert_(isinstance(a, AccumulatedValue))
		self.assert_(2.0 in m)
		self.assert_(2.0 not in m.get_xdata())
		m[2.0]+=1
		self.assert_(2.0 in m.get_xdata())
		
	def testZeroError(self):
		# AccumulatedValues with only one Accumulation should have 0 error
		m=MeasurementResult("TestData")
		m[0.0]
		m[1.0]=AccumulatedValue()
		m[2.0]=0
		m[3.0]=AccumulatedValue(0,1.0)
		k,v,e=m.get_errorplotdata()
		self.assert_(2.0 in k and 3.0 in k)
		self.assert_(1.0 not in k and 0.0 not in k)
		self.assertAlmostEqual(e[k==2.0][0], 0)
		self.assertAlmostEqual(e[k==3.0][0], 1.0)

if __name__=="__main__":
	unittest.main()
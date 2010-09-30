import serial
import re
import operator


DEBUG=False

temp_pattern = re.compile('\d+\.\d?')
# example answer '\x02PV279.8\x03/'
# [EOT] = \x04
# [STX] = \x02
# [ENQ] = \x05
# [ETX] = \x03
# [ACK] = \x06
# BCC = checksum

standard_device='0011'
EOT = '\x04'
STX = '\x02'
ENQ = '\x05'
ETX = '\x03'
ACK = '\x06'


"""
Paramter read example:

Master: [EOT]0011PV[ENQ]
Instrument:				[STX]PV16.4[ETX]{BCC}

Writing data:

Master: [EOT] {GID}{GID}{UID}{UID}[STX]{CHAN}(c1)(c2)<DATA>[ETX](BCC)

"""

def checksum(message):
	bcc = (reduce(operator.xor, map(ord,message)))
	return chr(bcc)

class Eurotherm(object):
	def __init__(self, serial_device, baudrate = 19200):
		self.s = serial.Serial(serial_device,
					baudrate = baudrate,
					bytesize=7,
					parity='E',
					stopbits=1,
					timeout=0.04)

	def read_param(self, param, device=standard_device):
		self.s.write(EOT+device+param+ENQ)

	def write_param(self, mnemonic, data, device=standard_device):
		if len(mnemonic) > 2:
			raise ValueError
		bcc = checksum(mnemonic + data + ETX)
		mes =  EOT+device+STX+mnemonic+data+ETX+bcc
		if DEBUG:
			for i in  mes:
				print i,hex(ord(i))
		self.s.write(mes)

	def get_current_temperature(self):
		self.read_param('PV')
		#s.write('\x040011PV\x05')
		answer = self.s.readline()
		try:
			temp = (re.findall(temp_pattern,answer)[0])
		except:
			print "received:", repr(answer)
			temp = "0"
		return temp

	def set_temperature(self, temperature):
		self.write_param('SL', str(temperature))

if __name__ == '__main__':
	import time
	delta=5
	date = time.strftime('%Y-%m-%d')
	f = open('templog_%s'%date,'w')
	f.write('# Start time: %s\n#delta t : %.1f s\n'%(time.asctime(), delta))
	et = Eurotherm("/dev/ttyUSB0")
	while True:
		for i in xrange(120):
			time.sleep(delta)
			#t = time.strftime()
			T = et.get_current_temperature()
				
			l = '%f %s\n'%(time.time(),T)
			print time.asctime(), T
			f.write(l)
			f.flush()
		f.write('# MARK -- %s --\n'%(time.asctime()))

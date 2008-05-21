class Persistance :
	def __init__(self, shots):
		self.shots = shots
		self.accu = 0
		self.counter = 0
		self.result_list = []

	def fade(self, res):
		self.counter += 1
		if self.accu == 0:
			self.accu=res+0
		self.result_list.append(res)
		if self.counter < 1:
			for i,ch in enumerate(self.accu.y):
				ch += res.y[i]
			
		elif len(self.result_list) == self.shots:
			self.counter = len(self.result_list)
			old_result = self.result_list.pop(0)
			for i,ch in enumerate(self.accu.y):
				ch *= self.shots
				ch -= old_result.y[i]
				ch += res.y[i]
		else:
			for i,ch in enumerate(self.accu.y):
				ch *= self.counter-1
				ch += res.y[i]
		self.accu /= self.counter	
		return self.accu

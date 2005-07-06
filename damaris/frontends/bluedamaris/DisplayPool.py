import numarray

class DisplayPool:
    
    def __init__(self):

        self.pool = { }


    def watch(self, in_result, in_channel):

        if in_channel in self.pool.keys():
            self.pool[in_channel].append(in_result)
            
        else:
            self.pool[in_channel] = []
            self.pool[in_channel].append(in_result)

        if len(self.pool[in_channel]) > 20:
            print "First element of %s deleted!" % in_channel
            del self.pool[in_channel][0]


    def getPending(self, in_channel = None):

        out_dict = { }

        if in_channel is None:

            for key in self.pool.keys():
                out_dict[key] = len(self.pool[key])

            return out_dict

        else:
            if in_channel not in self.pool.keys():
                print "Warning: no channel called %s" % in_channel
                return None
            else:
                return len(self.pool[in_channel])


    def getResult(self, in_channel, in_pos):

        if in_channel not in self.pool.keys():
            return None

        else:
            if in_pos >= len(self.pool[in_channel]): return None

            return self.pool[in_channel][in_pos]



    def getChannels(self):
        if len(self.pool) == 0: return None
        return self.pool.keys()



    def getNewestResult(self, in_channel):
        if in_channel not in self.pool.keys():
            return None

        if len(self.pool[in_channel]) == 0:
            return None

        return self.pool[in_channel][-1]
        

            

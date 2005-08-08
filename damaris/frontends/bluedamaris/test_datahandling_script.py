def data_handling(outer_space):

    acc = Accumulation(error = True)

    while 1:
        timesignal = outer_space.get_next_result()
        if timesignal is None: break
        
	acc = acc + timesignal

        print "Drawing %d..." % timesignal.get_job_id()
        outer_space.watch(timesignal, "Zeitsignal")
	outer_space.watch(acc, "Akkumulation")
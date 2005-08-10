def data_handling(outer_space):

    akk = { }

    while 1:
        timesignal = outer_space.get_next_result()
        if timesignal is None: break
        
        outer_space.watch(timesignal, "Zeitsignal")
        
        if akk.has_key(timesignal.get_description("tau")):
            akk[timesignal.get_description("tau")] += timesignal
            outer_space.watch(akk[timesignal.get_description("tau")], "Akku " + str(timesignal.get_description("tau")))
        else:
            akk[timesignal.get_description("tau")] = Accumulation(error = True) + timesignal
            outer_space.watch(akk[timesignal.get_description("tau")], "Akku " + str(timesignal.get_description("tau")))
        
        print "Drawing %d..." % timesignal.get_job_id()

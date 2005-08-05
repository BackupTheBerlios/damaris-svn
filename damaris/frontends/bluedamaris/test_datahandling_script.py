def data_handling(input):

    i = 0

    while 1:
        if i > 10: input.save_var_for_exp_script("quit_loop", True)
	else: input.save_var_for_exp_script("quit_loop", False)

        timesignal = input.get_next_result()
        if timesignal is None: break
        
        print "Drawing %d..." % timesignal.get_job_id()
        input.watch(timesignal, "Zeitsignal")

	i += 1

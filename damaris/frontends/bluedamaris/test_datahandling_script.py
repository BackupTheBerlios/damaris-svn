def data_handling(input):

    while input.jobs_pending():
        timesignal = input.get_next_result()
        print "Drawing %d..." % timesignal.get_job_id()
        input.draw(timesignal)
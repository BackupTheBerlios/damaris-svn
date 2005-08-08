def experiment_script(outer_space):

    pi = 1e-3

    tau = 1e-3
    px = 0
    py = 90
    mx = 180
    my = 270

    while tau <= 5e-3:

        for accu in range(1):

            exp = Experiment()
            print "Job %d erstellt!" % exp.get_job_id()
            exp.set_description("tau", tau)

            exp.set_frequency(100e6, 0)

            exp.rf_pulse(0, pi/2)
            exp.wait(tau)
            exp.rf_pulse(0, pi)
            exp.wait(tau + 1e-6)

            exp.record(512, 1e6)

            yield exp

        tau += 1e-3
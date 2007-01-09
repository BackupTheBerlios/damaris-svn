import ADC_Result
import Accumulation
import DaFFT
import tables
import time
from numpy import correlate, sqrt
import os,cPickle

def check_filename(filename):
    i=0
    stop = False	
    filename = "%s_%03i.h5"%(filename,i)
    while not stop:
        if os.path.isfile(filename):
            i += 1
            filename =  "%s_%03i.h5"%(filename[:-7],i)
        else: 
            stop = True
    return filename
    
def result_it():
    start_time = time.time()
    dacs = {}
    tps = {}
    tpfgs = {}

    var = 100
    fft = False
    for timesignal in results:
        if not isinstance(timesignal, ADC_Result.ADC_Result):
            print "No result: ",timesignal
            continue
        # get some information
	sr = timesignal.get_sampling_rate()
	al = len(timesignal.x)
        data["Timesignal"]=timesignal
        if fft:
            fft = DaFFT.FFT(timesignal)
            fftdata = fft.base_corr(0.1).fft()
            data["FFT"] = fftdata
        
	## Descriptions
        dac_value = int(timesignal.get_description("dac_value"))
        t_pfg = float(timesignal.get_description("t_pfg"))
        akku_number = int(timesignal.get_description("Akku"))+1
        akkus = int(timesignal.get_description("no_akku"))
        tau = float(timesignal.get_description("tau"))
        Delta = float(timesignal.get_description("Delta"))
        use_cor = (timesignal.get_description("use_cor"))
        index = int(timesignal.get_description("index"))
        delta1 = float(timesignal.get_description("delta1"))
        grad_G	= float(timesignal.get_description("G"))
        t_pfg_index = int(timesignal.get_description("t_pfg_index"))
        t_pfg_int = int(t_pfg*1e4)
        d1_int    = int(delta1*1e4)
        # easter egg :-)
        if use_cor == 'False':
            use_cor = False
        else:
            use_cor = True    
        
        # dac_value changed
        try:
            if not dac_old == dac_value:
                #print "dac_value changed:",dac_old,dac_value
                dac_old = dac_value
                var = 100
        except:
            dac_old = dac_value
        
        if dac_value == 0:
            zero_grad = True
        else:
            zero_grad = False
        dac_int = dac_value       
	if dac_value < 0:
	    dac_value = "neg_%s"%(str("%07i"%dac_value)[1:])
	else:
	    dac_value = "pos_%s"%(str("%07i"%dac_value)[1:])
	
	# dac_int is integer value
	# dac_value is a named value pos/neg_dacint

        if akku_number <= akkus:
            # Initializing
            if akku_number == 1:
                timesignal_akku = Accumulation.Accumulation(error=True)
                if fft:
                    fft_akku = Accumulation.Accumulation(error=True) 
            # accumulate        
            timesignal_akku += timesignal
            if fft:
                fft_akku += fftdata
               
            # plot current accumulation    
            
            data["Akku"]=timesignal_akku
            abs = timesignal_akku+0
            #abs.y[0] = sqrt(timesignal_akku.y[0]**2+timesignal_akku.y[1]**2)
            #abs.y[1] = timesignal_akku.y[0]*0
            #data["Abs"]=abs
            if fft:
                data["FFT-Akku"]=fft_akku
            # accumulation is finnished    
            if akku_number == akkus:
                #print "Result script",grad_G, Delta, t_pfg
                if use_cor:  # correction is applied, hence save the result
                    # Open a file in "a"ppend mode
                    h5file = tables.openFile(fn, mode = "a")

                    try:
                        h5file.root.raw_data._g_checkHasChild("g_tpfg%03i_delta%03i_%s"%(t_pfg_int,d1_int,dac_value))
                    except:
                        h5file.createGroup("/raw_data", "g_tpfg%03i_delta%03i_%s"%(t_pfg_int,d1_int,dac_value), 'the data')
                        print "data group %s created ... "%dac_value
                    
                    if fft:
                        try:
                            h5file.root.fft_data._g_checkHasChild("g_tpfg%03i_delta%03i_%s"%(t_pfg_int,d1_int,dac_value))
                        except:
                            h5file.createGroup("/fft_data", "g_tpfg%03i_delta%03i_%s"%(t_pfg_int,d1_int,dac_value), 'fftDaten')
                            print "fft data group %s created ... "%dac_value
                        
                
                    timesignal_akku.write_to_hdf(h5file, where="/raw_data/g_tpfg%03i_delta%03i_%s"%(t_pfg_int,d1_int,dac_value),
						    name="data_%04i_delta%04i"%(index,d1_int),
                                                    title="Data", compress=1)
                    if fft:
                        fft_akku.write_to_hdf(h5file, where="/fft_data/g_tpfg%03i_delta%03i_%s"%(t_pfg_int,d1_int,dac_value),
						    name="data_%04i_delta%04i"%(index,d1_int),
                                                    title="FFTData", compress=1)
                        #data['fftdac_value=%s_Delta=%f'%(dac_value,Delta)] = fft_akku
                    h5file.flush()
                    h5file.close()
                
                #data['dac_value=%s_Delta=%f'%(dac_value,Delta)] = timesignal_akku
                #h5file = tables.openFile(fn)
                # This is the FIRST run, and the zero gradient data is saved in signal_0 and the corection file is created
                thekey = (dac_int, t_pfg_index, index)
                if zero_grad and not use_cor:
                    signal_0 = sqrt(timesignal_akku.y[0]**2 + timesignal_akku.y[1]**2)
                    correctionfile = open('CORR','w')
                    t_corr = 0.0
		    dacs[thekey] =  t_corr
                    cPickle.dump(dacs,correctionfile)
                    #correctionfile.write("%i\t%e\n"%(dac_int,t_corr))
                    correctionfile.close()                       
                # Correction time is calculated for this signal and tp
		if not zero_grad and not use_cor:
                    corr_signal = correlate(sqrt(timesignal_akku.y[0]**2+timesignal_akku.y[1]**2),signal_0,mode=1)
                    correctionfile = open('CORR','w')
                    t_corr = (timesignal.x-timesignal.x.max()/2.0)[corr_signal.argmax()]
                    #print thekey,t_corr
		    dacs[thekey] =  t_corr
                    cPickle.dump(dacs, correctionfile)
		    #correctionfile.write("%i\t%e\n"%(dac_int,t_corr))
                    correctionfile.close()
			
                #h5file.close()
                var += 1
    end_time = time.time()            
    h5file = tables.openFile(fn)
    datasets=0
    for group in h5file.root.raw_data:
        datasets += group._v_nchildren
    h5file.close()    
    elapsed_time =  end_time - start_time
    ## send message
    import smtplib
    server = smtplib.SMTP('localhost')
    fr = 'pfg@fc2.fkp.physik.tu-darmstadt.de'
    to = 'markusro@element.fkp.physik.tu-darmstadt.de'
    msg = """From:pfg@fc2\nTo:markusro@element\nsubject:Messung beendet\n\nDie Messung ist beendet\n\tDauer: %f\n\tDatenpunkte: %i"""%(elapsed_time, datasets)
    server.sendmail(fr,to,msg)                                



filename = "data/NaX_file_22092006"
fn = check_filename(filename)    
#f=open(fn,'w')
#f.close()                

h5file = tables.openFile(fn, mode = "w", title = "Measurement Data")
h5file.createGroup("/", "raw_data", 'raw data')
h5file.createGroup("/", "fft_data", 'fft data')
h5file.root.raw_data._v_attrs.__setattr__('exp_script',open('NaX_exp.py').read())
h5file.root.raw_data._v_attrs.__setattr__('data_script',open('NaX_data.py').read())
h5file.flush()
h5file.close()

print time.time()

result=result_it

print time.time()

h5file.close()
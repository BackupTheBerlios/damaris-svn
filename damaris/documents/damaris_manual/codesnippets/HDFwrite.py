# import pytables module to
# support the binary HDF file format
import tables

def fid_result():
	# open file in "w"rite mode to generate a new, empty hdf5 file
	# this is not absolutely necessary if the file does not already exists
	nmr_file = tables.openFile('filename_to_store.h5', 'w')
	# open file in "a"ppend mode 
	nmr_file = tables.openFile('filename_to_store.h5', 'a')
	# loop over the results
	for timesignal in results:
		# plot the timesignal
		data["Timesignal"]=timesignal
		# write data
		timesignal.write_to_hdf(hdffile=nmr_file,
							# save it under the root group of the file 
							where="/", 		 
							# name of the group
							name="accu",	
							title="accudata",
							# enable lzo compression
							level="5",
							complib="zlib")
	# flush the data to the disk						
	nmr_file.flush()						
	# close the file
	nmr_file.close()

def result():
	return fid_result()

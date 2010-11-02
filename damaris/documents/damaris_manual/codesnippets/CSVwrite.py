# open file in "w"rite mode
nmr_file = open('filename_to_store.dat', 'w')
# write data
timesignal.write_to_csv(nmr_file)
# close the file
nmr_file.close()

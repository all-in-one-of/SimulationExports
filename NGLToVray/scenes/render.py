#!/usr/bin/python

import os
from subprocess import call
Files=os.listdir(".")

for file in Files :
	if file.endswith(".vrscene") :
		inputfile=file
		file=file.split(".")
		output="%s.%s.tiff" %(file[0],file[1])
		render="vray.bin -sceneFile %s -imgFile %s -display=0" %(inputfile,output)
		print render
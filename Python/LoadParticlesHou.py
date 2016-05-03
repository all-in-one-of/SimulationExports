import hou


########################################################################################################################
##  @brief a basic function to return a file name / absolute path stripping off $HIP etc
##	@param[in] _title the title to be displayed in the file box
##  @param[in] _wildCard the file selection wildcard i.e. *.obj etc
##  @param[in] _fileType the houdini file type option e.g. hou.fileType.Any
##  @returns a fully qualified file path or None
########################################################################################################################

def GetAbsoluteFileName(_title,_wildCard,_fileType) :
	# launch a file select and get the data
	file=hou.ui.selectFile(None,_title,False,_fileType,_wildCard)
	# if it was empty bomb out and return none
	if file =="" :
		return None
	else :
		# so we got some data we need to split it as we could have $JOB $HIP or $HOME prepended
		# to it  if we partition based on the / we get a tuple with "", "/","/....." where the
		# first element is going to be an environment var etc.
		file=file.partition("/")
		# we have $HOME so extract the full $HOME path and use it
		if file[0]=="$HOME" :
			prefix=str(hou.getenv("HOME"))
		elif file[0] == "$HIP" :
		#we have $HIP so extract the full $HIP path
			prefix=str(hou.getenv("HIP"))
		# we have a $JOB so extract the full $JOB path
		elif file[0] == "$JOB" :
			prefix=str(hou.getend("JOB"))
		# nothing so just blank the string
		else :
			prefix=str("")
	#now construct our new file name from the elements we've found
	return "%s/%s" %(prefix,file[2])


def createNull(parent,_name,x,y,z) :
	#create a null this will set loads of default values
	null = parent.createNode("null", _name, run_init_scripts=False, load_contents=True)
	# set the x,y,z values
	null.parm("tx").set(x)
	null.parm("ty").set(y)
	null.parm("tz").set(z)
	# now add a control to the null so we have something to visualise
	null.createNode("control", "ctrl"+_name, run_init_scripts=False, load_contents=True)
	# now grab the keyframe
	setKey = hou.Keyframe()
	# set to frame 0
	setKey.setFrame(0)
	# now key the tx/y and z values
	setKey.setValue(x)
	null.parm("tx").setKeyframe(setKey)
	setKey.setValue(y)
	null.parm("ty").setKeyframe(setKey)
	setKey.setValue(z)
	null.parm("tz").setKeyframe(setKey)
	# now add to our network node

def moveNull(_name,frame,x,y,z) :
	null=hou.node("/obj/"+_name)
	setKey = hou.Keyframe()
	setKey.setFrame(frame)
	setKey.setValue(x)
	null.parm("tx").setKeyframe(setKey)
	setKey.setValue(y)
	null.parm("ty").setKeyframe(setKey)
	setKey.setValue(z)
	null.parm("tz").setKeyframe(setKey)

def importParticleFile() :

	fileName=GetAbsoluteFileName("Select particle File","*.out",hou.fileType.Any)
	# get the the object leve as our parent
	if locals().get("hou_parent") is None:
		parent = hou.node("/obj")

	# make sure we got a filename
	if fileName !=None :
		subNetName=hou.ui.readInput("Please Enter name for the subnet")
		## @brief we now copy this to a new string
		subNetName=subNetName[1]

		subNet=parent.createNode("subnet", subNetName, run_init_scripts=False, load_contents=True)
		#open the file (could do a proper check here)
		file=open(fileName)
		frame=0
		numParticles=0
		# now process the file and create the nulls
		for line in file :
			line=line.split(" ")
			if line[0]=="NumParticles" :
				numParticles=int(line[1])
			elif line[0]=="Frame" :
				frame=int(line[1])
			else :
				name=line[0]
				x=float(line[1])
				y=float(line[2])
				z=float(line[3])
				if frame==0 :
					#we need to create our initial locators
					createNull(subNet,name,x,y,z)
				else :
					moveNull(subNetName+"/"+name,frame,x,y,z)

importParticleFile()


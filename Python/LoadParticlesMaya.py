import maya.OpenMaya as OM
import maya.OpenMayaAnim as OMA
import maya.cmds as cmds



def createLocator(_name,_x,_y,_z) :
	cmds.spaceLocator( name=_name)
	cmds.move(_x,_y,_z)
	cmds.setKeyframe()


def moveLocator(_name,x,y,z) :
	cmds.select(_name)
	cmds.move(x,y,z)
	cmds.setKeyframe()


def importParticleFile() :
	basicFilter = "*.out"

	fileName=cmds.fileDialog2(caption="Please select file to import",fileFilter=basicFilter, fm=1)
	if fileName[0] !=None :

		file=open(str(fileName[0]))
		frame=0
		numParticles=0
		#set to frame 0
		animControl=OMA.MAnimControl()
		animControl.setCurrentTime(OM.MTime(frame))

		for line in file :
			line=line.split(" ")
			if line[0]=="NumParticles" :
				numParticles=int(line[1])
			elif line[0]=="Frame" :
				frame=int(line[1])
				animControl.setCurrentTime(OM.MTime(frame))
			else :
				name=line[0]
				x=float(line[1])
				y=float(line[2])
				z=float(line[3])
				if frame==0 :
					#we need to create our initial locators
					createLocator(name,x,y,z)
				else :
					moveLocator(name,x,y,z)


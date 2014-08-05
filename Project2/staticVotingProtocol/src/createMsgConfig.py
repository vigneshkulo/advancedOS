import random

maxMsg = int(raw_input('Enter Max Message/Process: '))
print "Max Messages/Process: {}".format(maxMsg)

maxNodes = int(raw_input('Enter Max Nodes: '))
print "Max Nodes: {}".format(maxNodes)
print "----------------------------------------------------"

maxNodes += 1
mcGrp = "" 
fileOp = ""
delay = 0
sysFail = ""
fileChoice = 0
fp = open("msgconfig.dat", "w")
for j in range(maxNodes):
	for i in range(maxMsg):
		fileChoice = random.randrange(1, 3)
		delay = random.uniform(1, 5)
		if(5 == random.randrange(1, 10)): sysFail = "T"
		else: sysFail = "F"
		if( 1 == fileChoice): fileOp = "Read"
		else: fileOp = "Writ"
		s = "%03d %s %03d %s\n" % (j+1, fileOp, delay, sysFail);
		fp.write(s);

fp.close()

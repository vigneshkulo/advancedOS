import random

maxMsg = int(raw_input('Enter Max Message: '))
print "Max Messages: {}".format(maxMsg)

maxNodes = int(raw_input('Enter Max Nodes: '))
print "Max Nodes: {}".format(maxNodes)
print "----------------------------------------------------"

def func(n, end, start = 1):
    return range(start, n) + range(n+1, end)

maxNodes += 1
mcGrp = "" 
sendNode = 0
recvNode = 0
fp = open("msgconfig.dat", "w")
for i in range(maxMsg):
	sendNode = random.randrange(1, maxNodes)
	r = range(1, sendNode) + range(sendNode+1, maxNodes)
	numMcGrp = random.randrange(1, maxNodes-1)
	for j in range(numMcGrp):
		recvNode = random.choice(r)
		mcGrp += "%03d:" % recvNode 
		r.remove(recvNode)
	s = "%02d %03d <%s> <Random Message>\n" % (i+1, sendNode, mcGrp[:-1]);
	mcGrp = ""
	fp.write(s);

fp.close()

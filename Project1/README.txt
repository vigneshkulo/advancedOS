1. Create "msgconfig.dat" file using Python Script
	Run "python createMsgConfig.py"
	Enter Maximum number of messages to be in the input file and number of nodes running
	Example: Maximum Messages = 100, Number of Nodes = 10.

2. Use Makefile to compile the code.
	Run "make all" in terminal
	Executables "Application" and "skeen" will be created.
	Application executable will kick start skeen.

3. Use "runScript" to run the program. Max Nodes entered above should be consistent. 
	If Max Nodes = 10 (used to create "msgconfig.dat"), should run from Net01 to Net10.

4. After the programs exits. Compile the Test Program using commands
	gcc testProgram -o test

5. Run the test program using command
	./test
	Enter the Max Nodes value. Ex. 10 for the above case.
	It outputs success or failure message based on the Log Files generated.

6. "ipconfig.dat" is defined from net01 to net21.
	Format is:
	[Node Number] [Port Number] [IP Address]
	
	Ex. To add net22, add the following line in "ipconfig.dat"
	022 31564 10.176.67.85
	
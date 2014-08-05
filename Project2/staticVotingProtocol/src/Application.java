import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
//import java.util.Random;
//import java.util.Scanner;

public class Application {
	
	private static final int CHECK_OUT_FAIL = -1;
	private static final int READ_FAIL = -2;
	private static final int WRITE_FAIL = -3;
	private static final int CHECK_IN_FAIL = -4;
	private static final int MAX_RETRY = 7;
	
	static int procId;
	static int totalNodes;
	static int MaxFiles;
	static Thread receiveThread;
	static FileSystem localFS = new FileSystem();
	
	public static synchronized void main(String[] args) {
		
		int status = 0;
		String hostName;
		
		if(args.length != 2) System.out.println("* Application: Please Enter Total nodes and Total Files in the System");

		if(null != args[0]) Application.totalNodes = Integer.parseInt(args[0]);

		if(null != args[1])	Application.MaxFiles = Integer.parseInt(args[1]);
		
		Application localApp = new Application();
		
		try {
			hostName = InetAddress.getLocalHost().getHostName();
			hostName = hostName.replaceAll("[^0-9]", "");
			Application.procId = Integer.parseInt(hostName);
			System.out.println("* --------------------------");
			System.out.println("* I am : " + Application.procId + ", Total Nodes: " + Application.totalNodes + ", Total Files: " + Application.MaxFiles);
			System.out.println("* --------------------------");

		} catch (UnknownHostException e) {
			e.printStackTrace();
		}
		localFS.initialize();
		
		String tokens[];
		BufferedReader br = null;
		String currLine = null;
		

		try {
			Thread.sleep(5000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
//		Scanner reader = null;
		int retryCount = 0;
		int backOffTime = 1;

		try {
			br = new BufferedReader(new FileReader("msgconfig.dat"));
			while ((currLine = br.readLine()) != null) 
			{
//				System.out.println("* Enter Number to Continue");
//				reader = new Scanner(System.in);
//				reader.nextInt();
				
				tokens = currLine.split(" ");
				if(Application.procId != Integer.parseInt(tokens[0]))
					continue;
				
				System.out.println("* ---------------------------------------");
				

				switch(tokens[1])
				{
					case "Read":
							status = -1;
							retryCount = 0;
							backOffTime = 1;
							try {
								System.out.println("* Application: Read Operation: " + currLine);
								System.out.println("* ---------------------------------------");

								while(0 > status && retryCount < MAX_RETRY)
								{
									status = localApp.checkOut(FileSystem.SHARED_MODE, Integer.parseInt(tokens[2]));
									if(CHECK_OUT_FAIL == status) 
									{
										retryCount++;
										Thread.sleep(backOffTime * 1000);
										backOffTime *= 2;
										System.out.println("* Application: Retrying " + retryCount);
										continue;
									}
									localFS.writeLog( "[", Integer.parseInt(tokens[2]));
		
									status = localApp.readFile(Integer.parseInt(tokens[2]));
									if(READ_FAIL == status) 
									{
										retryCount++;
										Thread.sleep(backOffTime * 1000);
										backOffTime *= 2;
										System.out.println("* Application: Retrying " + retryCount);
										continue;
									}
									localFS.writeLog("Read", Integer.parseInt(tokens[2]));
									
									System.out.println("* Application: Delay for " + Integer.parseInt(tokens[2]) + " seconds");
									Thread.sleep(Integer.parseInt(tokens[2]) * 1000);
					
									status = localApp.checkIn(FileSystem.SHARED_MODE, Integer.parseInt(tokens[2]));
									if(CHECK_IN_FAIL == status)
									{
										retryCount++;
										Thread.sleep(backOffTime * 1000);
										backOffTime *= 2;
										System.out.println("* Application: Retrying " + retryCount);
										continue;
									}
									localFS.writeLog( "]", Integer.parseInt(tokens[2]));
								}
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
//							System.out.println("* Complete: " + status + ", " + retryCount);
					break;
					
					case "Writ": 
							status = -1;
							retryCount = 0;
							backOffTime = 1;
							
							System.out.println("* Application: Write Operation: " + currLine);
							System.out.println("* ---------------------------------------");

							try {
								while((0 > status) && (retryCount < MAX_RETRY))
								{
									status = localApp.checkOut(FileSystem.EXCLUSIVE_MODE, Integer.parseInt(tokens[2]));
									if(CHECK_OUT_FAIL == status)
									{
										retryCount++;
										Thread.sleep(backOffTime * 1000);
										backOffTime *= 2;
										System.out.println("* Application: Retrying " + retryCount);
										continue;
									}
									localFS.writeLog( "[", Integer.parseInt(tokens[2]));
		
									status = localApp.writeFile(Integer.parseInt(tokens[2]));
									if(WRITE_FAIL == status)
									{
										retryCount++;
										Thread.sleep(backOffTime * 1000);
										backOffTime *= 2;
										System.out.println("* Application: Retrying " + retryCount);
										continue;
									}
									localFS.writeLog("Writ", Integer.parseInt(tokens[2]));
		
										System.out.println("* Application: Delay for " + Integer.parseInt(tokens[2]) + " seconds");
										Thread.sleep(Integer.parseInt(tokens[2]) * 1000);
	
									status = localApp.checkIn(FileSystem.EXCLUSIVE_MODE, Integer.parseInt(tokens[2]));
									if(CHECK_IN_FAIL == status)
									{
										retryCount++;
										Thread.sleep(backOffTime * 1000);
										backOffTime *= 2;
										System.out.println("* Application: Retrying " + retryCount);
										continue;
									}
									localFS.writeLog("]", Integer.parseInt(tokens[2]));
								}
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
							
//							System.out.println("* Complete: " + status + ", " + retryCount);
					break;
					
					default: 
						System.out.println("* Application: Enter Valid Choice");
						break;
				}

				if(status < 0)
				{
					try {
						System.out.println("* Application: Delay for " + Integer.parseInt(tokens[2]) + " seconds");
						Thread.sleep(Integer.parseInt(tokens[2]) * 1000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
//				if(tokens[3].equals("T")) 
//				{
//					try {
//				        Random rn = new Random();
//				        long rand1 = (long) (rn.nextGaussian()*8 + 20) * 1000;
//				        FileSystem.block = true;
//						System.out.println("* --------------------- SYSTEM FAILURE: " +rand1+ " -------------------------");
//						Thread.sleep((long) (rand1));
//					    synchronized(localFS){
//					    	BlockThread.block = false;
//					    	localFS.notify();
//					    }					
//					    } catch (InterruptedException e) {
//						e.printStackTrace();
//					}
//				}
			}
			br.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

//		reader.close();
		System.out.println("* -------------------------------------------------------");
		System.out.println("* Application: Return Status: Waiting for Thread");
		
		try {
			receiveThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		return;
	}
	public synchronized void unblock() {
	    FileSystem.block = false;
	    notifyAll();
	}


	private int checkOut(int mode, int fileNo) {
		int ret;
		ret = localFS.acquireMyLock(mode, fileNo);
		if(-1 == ret)
		{
			System.out.println("* Application: Unable to Aquire My Lock");
			return CHECK_OUT_FAIL;
		}
		ret = localFS.acquireLock(mode, fileNo);
		if(-1 == ret)
		{
			System.out.println("* Application: Unable to Acquire Quorum");
			localFS.releaseMyLock(mode, fileNo);
			localFS.releaseLock(mode, fileNo);
			return CHECK_OUT_FAIL;
		}
		System.out.println("* Application: File Checked Out");
		return ret;
	}
	
	private int checkIn(int mode, int fileNo) {
		int ret;
		ret = localFS.releaseMyLock(mode, fileNo);
		if(-1 == ret)
		{
			System.out.println("* Application: Releasing My Lock Failed");
			return CHECK_IN_FAIL;
		}
		ret = localFS.releaseLock(mode, fileNo);
		if(-1 == ret)
		{
			System.out.println("* Application: Releasing Locks Failed");
			return CHECK_IN_FAIL;
		}
		System.out.println("* Application: File Checked In");
		return 0;
	}
	private int writeFile(int fileNo) {
		int ret;
		
		ret = localFS.writeFileAndUpdateOthers(fileNo);
		if(-1 == ret)
		{
			System.out.println("* Application: Writing Failed");
			checkIn(FileSystem.EXCLUSIVE_MODE, fileNo);
			return WRITE_FAIL;
		}
		System.out.println("* Application: File Written");

		return 0;		
	}
	private int readFile(int fileNo) {
		int ret;
		
		ret = localFS.readUpdatedVersion(fileNo);
		if(-1 == ret)
		{
			System.out.println("* Application: Reading Failed");
			checkIn(FileSystem.SHARED_MODE, fileNo);
			return READ_FAIL;
		}

		System.out.println("* Application: File Read");
		return 0;
	}

}

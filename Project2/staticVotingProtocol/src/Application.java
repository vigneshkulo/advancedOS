import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Scanner;

public class Application {

	static int procId;
	static int totalNodes;
	FileSystem localFS = new FileSystem();
	
	public static void main(String[] args) {
		
		int choice = 0;
		int status = 0;
		String hostName;
		
		Application.totalNodes = Integer.parseInt(args[0]);
		Application localApp = new Application();
		
		try {
			hostName = InetAddress.getLocalHost().getHostName();
			hostName = hostName.replaceAll("[^0-9]", "");
			Application.procId = Integer.parseInt(hostName);
			System.out.println("* --------------------------");
			System.out.println("* I am : " + Application.procId + ", Total Nodes: " + Application.totalNodes);
			System.out.println("* --------------------------");

		} catch (UnknownHostException e) {
			e.printStackTrace();
		}
		localApp.localFS.initialize();
		Scanner reader = new Scanner(System.in);
		while(true)
		{
			System.out.println("* ---------------------------------------");
			System.out.print("* Application: Enter 1. Read, 2. Write: ");
			while(reader.hasNextInt())
			{
				choice = reader.nextInt(); 
				break;
			}
			
			if(3 == choice) break;
			
			switch(choice)
			{
				case 1: status = localApp.checkOut(FileSystem.SHARED_MODE);
						status = localApp.readFile();
						status = localApp.checkIn(FileSystem.SHARED_MODE);
				break;
				
				case 2: status = localApp.checkOut(FileSystem.EXCLUSIVE_MODE);
						status = localApp.writeFile();
						status = localApp.checkIn(FileSystem.EXCLUSIVE_MODE);
				break;
				
				default: 
					System.out.println("* Application: Enter Valid Choice");
					break;
			}
		}
		reader.close();

		System.out.println("* -------------------------------------------------------");
		System.out.println("* Application: Return Status : " + status);
		return;
	}
	
	private int checkOut(int mode) {
		int ret;
		ret = localFS.acquireMyLock(mode);
		if(-1 == ret)
		{
			System.out.println("* Application: Requested Mode cannot be granted");
			return -1;
		}
		ret = localFS.acquireLock(mode);
		if(-1 == ret)
		{
			System.out.println("* Application: Unable to Acquire Lock");
			localFS.releaseMyLock(mode);
			return -1;
		}
		return ret;
	}
	
	private int checkIn(int mode) {
		int ret;
		ret = localFS.releaseMyLock(mode);
		if(-1 == ret)
		{
			System.out.println("* Application: Releasing My Lock Failed");
			return -1;
		}
		ret = localFS.releaseLock(mode);
		if(-1 == ret)
		{
			System.out.println("* Application: Releasing Locks Failed");
			return -1;
		}
		System.out.println("* --------------------------");
		System.out.println("* Application: File Checked In: " + FileSystem.accessMode);
		System.out.println("* --------------------------");

		return 0;
	}
	private int writeFile() {
		int ret;
		
		ret = localFS.writeFileAndUpdateOthers();
		if(-1 == ret)
		{
			System.out.println("* Application: Writing Failed");
			return -1;
		}
		System.out.println("* --------------------------");
		System.out.println("* Application: File Written");
		System.out.println("* --------------------------");

		return 0;		
	}
	private int readFile() {
		System.out.println("* --------------------------");
		System.out.println("* Application: File Read");
		System.out.println("* --------------------------");

		return 0;
	}

}

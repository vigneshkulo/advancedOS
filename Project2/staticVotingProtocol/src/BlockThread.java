import java.util.Random;


public class BlockThread implements Runnable {
	static boolean block = false;
	static BlockThread localApp = new BlockThread();

	public synchronized void run() {
//		Scanner reader;
		
		while (true) {
			while(block)
			{
				System.out.println("* FS: Blocked due to Failure");
				try {
				    synchronized(localApp){
				    	localApp.wait();
				    }
					System.out.println("* FS: Unblocked: " + block);
				} catch (InterruptedException e) {
				//	e.printStackTrace();
					continue;
				}
			}
			
			int a = 0;
//			reader = new Scanner(System.in);
//			System.out.println("Enter the first number: " + block);
//			a = reader.nextInt();
//			System.out.println("* User Input: " + a);
			if(1 == a)
			{
				break;
			}
		}
//	    reader.close();
	}
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		(new Thread(new BlockThread())).start();
		for(int i = 0; i < 10; i++)
		{
			try {
		        Random rn = new Random();
		        long rand1 = (long) (rn.nextGaussian()*8 + 20) * 1000;
		        BlockThread.block = true;
				System.out.println("* -------------------------------------------");
				System.out.println("* Application: " + i + ": System Failed: " + rand1);
				Thread.sleep((long) (4000));
			    synchronized(localApp){
			    	BlockThread.block = false;
			    	localApp.notify();
			    }
				System.out.println("* Application: " + i + ": System Recovered: " + rand1);

			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}

}

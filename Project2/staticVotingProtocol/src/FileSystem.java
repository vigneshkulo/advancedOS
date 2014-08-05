import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.Writer;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.util.ArrayList;
import java.util.Random;

import javax.swing.Timer;

import com.sun.nio.sctp.*;

public class FileSystem implements Runnable, ActionListener {
	static final int SHARED_MODE = 1;
	static final int EXCLUSIVE_MODE = 2;
	static final int RELEASE = 3;
	static final int WRITE = 4;
	static final int READ = 5;
	private static final int FALSE = -1;

	int[] portN = new int[45];
	int[] voteWgt = new int[45];
	String[] ipAddr = new String[45];

	int[] lockMembers = new int[45];
	
	int[] accessMode = new int[50];
	int[] versionNum = new int[50];
	int[] sharedCount = new int[50];
	int[] writeCount = new int[50];
	int[] readCount = new int[50];
	
	ArrayList<Integer> verNumList = new ArrayList<Integer>();
	
	Timer timer;
	Timer chkInActTimer;

	private SctpChannel localSock;
	private SctpServerChannel localRcvChannel = null;

	static boolean block = false;
	
	static int voteCount = 0;
	static int lockCount = 0;
	static int readQSize = 0;
	static int writQSize = 0;
	static int totalVotes = 0;

	public synchronized void run() {
		//System.out.println("* FS: Thread Started");
		chkInActTimer = new Timer(64000, this);

		readIP();
		String rcvData;
		String rplyData = null;
		String[] tokens;
		SctpChannel rcvSocket = null;
		SctpServerChannel rcvServer;
		InetSocketAddress serverAddr;
		ByteBuffer buffer = ByteBuffer.allocateDirect(100);
		CharBuffer charbuf = buffer.asCharBuffer();
		Charset charset = Charset.forName("ISO-8859-1");
		CharsetEncoder encoder = charset.newEncoder();
		CharsetDecoder decoder = charset.newDecoder();
		MessageInfo msgInfo = MessageInfo.createOutgoing(null, 0);

		try {

			rcvServer = SctpServerChannel.open();
			serverAddr = new InetSocketAddress(portN[Application.procId - 1]);
			rcvServer.bind(serverAddr);
			localRcvChannel = rcvServer;
			//System.out.println("* FS: Socket Created with PN: " + portN[Application.procId - 1]);
			//System.out.println();
			while (true) {
				while(block)
				{
					System.out.println("* FS: ------------- Blocked due to Failure -------------");
					try {
					    synchronized(Application.localFS){
					    	Application.localFS.wait();
					    }					
					    } catch (InterruptedException e) {
					//	e.printStackTrace();
						continue;
					}
				}
				chkInActTimer.start();
				rcvSocket = rcvServer.accept();
				chkInActTimer.stop();
				
				buffer.clear();
				rcvSocket.receive(buffer, null, null);
				buffer.flip();

				rcvData = decoder.decode(buffer).toString();
				//System.out.println("* -------------------------------------------");
				//System.out.println("* Receive Buffer Contents are: " + rcvData);

				tokens = rcvData.split("-");
				//System.out.println("* Request Mode: " + tokens[0] + ", Current Mode: " + accessMode);
				
				/* Write Data Message */
				if(WRITE == Integer.parseInt(tokens[0])) {
//					System.out.println("* FS: Data to be Updated: " + tokens[2]);
					writeFile(tokens[2], Integer.parseInt(tokens[1]));
					continue;
				}
				
				/* Release Lock Message */
				if (RELEASE == Integer.parseInt(tokens[0])) {
					if (SHARED_MODE == accessMode[Integer.parseInt(tokens[1]) - 1]) {
						if (1 == sharedCount[Integer.parseInt(tokens[1]) - 1])
							accessMode[Integer.parseInt(tokens[1]) - 1] = 0;
						
						readCount[Integer.parseInt(tokens[1]) - 1]--;
						sharedCount[Integer.parseInt(tokens[1]) - 1]--;
					} else if (EXCLUSIVE_MODE == accessMode[Integer.parseInt(tokens[1]) - 1]) {
						accessMode[Integer.parseInt(tokens[1]) - 1] = 0;
						writeCount[Integer.parseInt(tokens[1]) - 1]--;
					}
//					System.out.println("* FS: Lock Released: " + accessMode);
					continue;
				}
				
				/* Acquire Lock Message */
				if (EXCLUSIVE_MODE == Integer.parseInt(tokens[0])) {
					if (0 == accessMode[Integer.parseInt(tokens[1]) - 1]) {
						rplyData = Integer.toString(versionNum[Integer.parseInt(tokens[1]) - 1]);
						//System.out.println("* Reply Buffer Contents are: " + rplyData);
						accessMode[Integer.parseInt(tokens[1]) - 1] = EXCLUSIVE_MODE;
						writeCount[Integer.parseInt(tokens[1]) - 1]++;
					} else {
						rplyData = Integer.toString(FALSE);
						//System.out.println("* Reply Buffer Contents are: " + rplyData);
					}
				} else if (SHARED_MODE == Integer.parseInt(tokens[0])) {
					if (EXCLUSIVE_MODE != accessMode[Integer.parseInt(tokens[1]) - 1]) {
						rplyData = Integer.toString(versionNum[Integer.parseInt(tokens[1]) - 1]);
						//System.out.println("* Reply Buffer Contents are: " + rplyData);
						accessMode[Integer.parseInt(tokens[1]) - 1] = SHARED_MODE;
						sharedCount[Integer.parseInt(tokens[1]) - 1]++;
						readCount[Integer.parseInt(tokens[1]) - 1]++;
					} else {
						rplyData = Integer.toString(FALSE);
						//System.out.println("* Reply Buffer Contents are: " + rplyData);
					}
				} else if (READ == Integer.parseInt(tokens[0])) {
					int latestVersion = 0;
					String latestData = null;
					//System.out.println("* FS: Received Request for  Updated Read: ");
					String fname = "file" + Integer.parseInt(tokens[1]) + Application.procId;
					BufferedReader br = null;
					String currLine = null;
					try
					{
						br = new BufferedReader(new FileReader(fname));
						while ((currLine = br.readLine()) != null) 
						{
							if (currLine.contains("VersionNumber")) {
								tokens = currLine.split(":");
								latestVersion = Integer.parseInt(tokens[1]);
								//System.out.println("* FS: MyVersion Number: " + tokens[1]);
								continue;
							}
	
							if (currLine.contains("Data")) {
								tokens = currLine.split(":");
								latestData = tokens[1];
		
								//System.out.println("* FS: Data: " + tokens[1]);
								//System.out.println("* ---------------------------------------");
							}
							
//							System.out.println("* FS: Sending Latest  Version: " + latestVersion + ", Data: " + latestData);	
							rplyData = Integer.toString(latestVersion) + "," + latestData;
						}
						br.close();
					}
					catch (FileNotFoundException e) {
						e.printStackTrace();
						System.out.println("* FS: File Not Found");
						rplyData = Integer.toString(FALSE);
					}
				} else {
					rplyData = Integer.toString(FALSE);
					System.out.println("* FS: Invalid Msg: Reply Buffer Contents are: " + rplyData);
				}

				charbuf.clear();
				charbuf.put(rplyData);
				charbuf.flip();

				buffer.clear();
				encoder.encode(charbuf, buffer, true);
				buffer.flip();

				try {
					rcvSocket.send(buffer, msgInfo);
				} catch (IOException e) {
					// e.printStackTrace();
					System.out.println("* FS: Reply Failed");
				}
				buffer.clear();
			}

		} catch (IOException e) {
//			e.printStackTrace();
			System.out.println("* FS: SOCKET CLOSED DUE TO INACTIVITY");
		}
		buffer.clear();
		try {
			if(null != rcvSocket)
			{
				if(rcvSocket.isOpen())
					rcvSocket.close();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		System.out.println("* FS: Thread Exiting");
		System.out.println("* ----------------------------------");
		return;
	}

	public int acquireLock(int mode, int fileNo) {

		voteCount = 0;
		String sendData = null;
		String rplyData = null;
		SctpChannel[] sendSocket = new SctpChannel[Application.totalNodes];
		InetSocketAddress serverAddr = null;
		ByteBuffer buffer = ByteBuffer.allocateDirect(100);
		CharBuffer charbuf = buffer.asCharBuffer();
		MessageInfo msgInfo = MessageInfo.createOutgoing(null, 0);
		Charset charset = Charset.forName("ISO-8859-1");
		CharsetEncoder encoder = charset.newEncoder();
		CharsetDecoder decoder = charset.newDecoder();

		timer = new Timer(2000, this);
		voteCount = voteWgt[Application.procId - 1];
		lockCount = 1;
		int lockSize = 0;

		if (SHARED_MODE == mode)
			lockSize = readQSize;
		else if (EXCLUSIVE_MODE == mode)
			lockSize = writQSize;

		/* Send a request for Lock and Receive a Reply */
		for (int i = 0; i < Application.totalNodes; i++) {
			if (i + 1 == Application.procId)
				continue;

			if (voteCount >= lockSize)
				break;

			//System.out.println("* ---------------------------------------");
			//System.out.println("* FS: Connecting to: " + ipAddr[i] + ", " + portN[i]);
			serverAddr = new InetSocketAddress(ipAddr[i], portN[i]);

			try {
				sendSocket[i] = SctpChannel.open(serverAddr, 0, 0);
			} catch (IOException e) {
				// e.printStackTrace();
				System.out.println("* FS: Connection Refused by " + ipAddr[i]);
				continue;
			}

			sendData = Integer.toString(mode) + "-" + Integer.toString(fileNo);

			charbuf.clear();
			charbuf.put(sendData);
			charbuf.flip();

			buffer.clear();
			encoder.encode(charbuf, buffer, true);
			buffer.flip();

			try {
				sendSocket[i].send(buffer, msgInfo);
			} catch (IOException e) {
				// e.printStackTrace();
				System.out.println("* FS: Send Failed to " + ipAddr[i]);
				continue;
			}
			// //System.out.println("* FS: Send Data: " + sendData);
			buffer.clear();
			charbuf.clear();

			timer.start();
			// //System.out.println("* FS: Timer Started");
			localSock = sendSocket[i];

			try {
				buffer.clear();
				sendSocket[i].receive(buffer, null, null);

				timer.stop();
				// //System.out.println("* FS: Timer Stopped");

				buffer.flip();
				try {
					rplyData = decoder.decode(buffer).toString();
				} catch (CharacterCodingException e) {
					// e.printStackTrace();
					System.out.println("* FS: Decoding Error in " + ipAddr[i]);
					continue;
				}
				//System.out.println("* FS: Reply Buffer Contents are: " + (FALSE != Integer.parseInt(rplyData)) + ", " + Integer.parseInt(rplyData));

				sendSocket[i].close();

				if (FALSE != Integer.parseInt(rplyData)) {
					verNumList.add(Integer.parseInt(rplyData));
					lockMembers[lockCount - 1] = i + 1;
					voteCount += voteWgt[i];
					lockCount++;
					//System.out.println("* FS: Current Weight: " + voteCount);
				}

			} catch (IOException e) {
				// e.printStackTrace();
				System.out.println("* FS: Receive Failed: " + ipAddr[i]);
				continue;
			}
		}
		//System.out.println("* ---------------------------------------");
			
		if (voteCount >= lockSize)
		{
			if (SHARED_MODE == mode)
				readCount[fileNo-1]++;
			else if (EXCLUSIVE_MODE == mode)
				writeCount[fileNo-1]++;
			
			System.out.print("* FS: Lock Members: ");
			for (int i = 0; i < lockCount - 1; i++)
				System.out.print(lockMembers[i] + ", ");
			System.out.println();
			
			return 0;
		}
		else
			return -1;
	}

	public void initialize() {
		readIP();
		if(0 != ((double)totalVotes / 2) % 1)
		{
			writQSize = (int) (Math.ceil((double)totalVotes / 2));
		}
		else
			writQSize = (int) (Math.ceil((double)totalVotes / 2)) + 1;

		readQSize = totalVotes - writQSize + 1;
		System.out.println("* FS: Total Votes: " + totalVotes);
		System.out.println("* FS: Read Quorum Size: " + readQSize);
		System.out.println("* FS: Writ Quorum Size: " + writQSize);
		
		PrintWriter writer = null;
		try {
			writer = new PrintWriter("Log" + Application.procId);
			writer.print("");
			writer.close();
		} catch (FileNotFoundException e) {
//			e.printStackTrace();
			System.out.println("* FS: Log File Not Found");
		}
		
		Application.receiveThread = new Thread(new FileSystem());
		Application.receiveThread.start();
	}

	public void readIP() {
		int node = 0;
		totalVotes = 0;
		BufferedReader br = null;
		String strLine = "";
		String[] tokens;
		try {
			br = new BufferedReader(new FileReader("ipconfig.dat"));
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		try {
			while ((strLine = br.readLine()) != null) {
				tokens = strLine.split(" ");
				portN[node] = Integer.parseInt(tokens[1]);
				ipAddr[node] = tokens[2];
				voteWgt[node] = Integer.parseInt(tokens[3]);
				totalVotes += voteWgt[node];
				node++;
				if(Application.totalNodes <= node) break;
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void actionPerformed(ActionEvent arg0) {
		//System.out.println("* FS: Timer Expired");
		if(arg0.getSource() == timer)
		{
			try {
				if (null != this.localSock) {
					if (this.localSock.isOpen()) {
						timer.stop();
						this.localSock.close();
						System.out.println("* FS: Timer Expired: Closing Socket since no reply");
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		else if (arg0.getSource() == chkInActTimer)
		{
			System.out.println("* FS: SYSTEM INACTIVE");
			try {
				if (null != this.localRcvChannel) {
					if (this.localRcvChannel.isOpen()) {
						chkInActTimer.stop();
						this.localRcvChannel.close();
						System.out.println("* FS: Timer Expired: Closing Socket since no Activity");
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	protected int acquireMyLock(int mode, int fileNo) {
		//System.out.println("* FS: Current Access Mode: " + ((accessMode == 0) ? "NONE" : ((accessMode == 1) ? "SHARED" : "EXCLUSIVE")));
		if (EXCLUSIVE_MODE != accessMode[fileNo-1] && EXCLUSIVE_MODE != mode) {
			accessMode[fileNo-1] = SHARED_MODE;
			sharedCount[fileNo-1]++;
			verNumList.add(versionNum[fileNo-1]);
			return 0;
		} else if (EXCLUSIVE_MODE == mode && 0 == accessMode[fileNo-1]) {
			accessMode[fileNo-1] = EXCLUSIVE_MODE;
			verNumList.add(versionNum[fileNo-1]);
			return 0;
		} else
			return -1;
	}

	protected int releaseMyLock(int mode, int fileNo) {
		if (SHARED_MODE == mode) {
			if (1 == sharedCount[fileNo-1])
			{
				accessMode[fileNo-1] = 0;
			}
			sharedCount[fileNo-1]--;
		} else if (EXCLUSIVE_MODE == mode) {
			accessMode[fileNo-1] = 0;
		}
//		System.out.println("* FS: RML: Current Access Mode: " + ((accessMode == 0) ? "NONE" : ((accessMode == 1) ? "SHARED" : "EXCLUSIVE")));

		return 0;
	}

	protected int releaseLock(int mode, int fileNo) {
		String sendData = null;
		SctpChannel[] sendSocket = new SctpChannel[Application.totalNodes];
		InetSocketAddress serverAddr = null;
		ByteBuffer buffer = ByteBuffer.allocateDirect(100);
		CharBuffer charbuf = buffer.asCharBuffer();
		MessageInfo msgInfo = MessageInfo.createOutgoing(null, 0);
		Charset charset = Charset.forName("ISO-8859-1");
		CharsetEncoder encoder = charset.newEncoder();

//		System.out.println("* ---------------------------------------");
//		System.out.println("* FS: Releasing Locks");

		/* Send a request for Lock and Receive a Reply */
		for (int i = 0; i < lockCount - 1; i++) {

//			System.out.println("* ---------------------------------------");
//			System.out.println("* FS: Connecting to: "  + ipAddr[lockMembers[i] - 1] + ", "  + portN[lockMembers[i] - 1]);
			serverAddr = new InetSocketAddress(ipAddr[lockMembers[i] - 1],
					portN[lockMembers[i] - 1]);

			try {
				sendSocket[i] = SctpChannel.open(serverAddr, 0, 0);
			} catch (IOException e) {
				// e.printStackTrace();
				System.out.println("* FS: Connection Refused by " + ipAddr[lockMembers[i] - 1]);
				continue;
			}

			sendData = Integer.toString(RELEASE) + "-" + Integer.toString(fileNo);

			charbuf.clear();
			charbuf.put(sendData);
			charbuf.flip();

			buffer.clear();
			encoder.encode(charbuf, buffer, true);
			buffer.flip();

			try {
				sendSocket[i].send(buffer, msgInfo);
			} catch (IOException e) {
				// e.printStackTrace();
				//System.out.println("* FS: Send Failed");
				continue;
			}
			//System.out.println("* FS: Send Data: " + sendData);
			buffer.clear();
			charbuf.clear();

		}
		if (SHARED_MODE == mode && voteCount >= readQSize)
			readCount[fileNo-1]--;
		else if (EXCLUSIVE_MODE == mode && voteCount >= writQSize)
			writeCount[fileNo-1]--;
		verNumList.clear();
		return 0;
	}
	public int comparator(ArrayList<Integer> versions) {
		int maximum = versions.get(0);
		
		//System.out.print("* FS: Version Numbers: " + maximum + " ");
		for (int i = 1; i < versions.size(); i++) {
			//System.out.print(versions.get(i) + " ");
			if (versions.get(i) > maximum) {
				maximum = versions.get(i);
			}
		}
		//System.out.println();
		return maximum;
	}

	public String randomString() {
		char[] chars = "abcdefghijklmnopqrstuvwxyz".toCharArray();
		StringBuilder sb = new StringBuilder();
		Random random = new Random();
		for (int i = 0; i < 3; i++) {
			char c = chars[random.nextInt(chars.length)];
			sb.append(c);
		}
		String output = sb.toString();
		return output;
	}

	public void writeFile(String data, int fileNo) {
		String fname = "file" + fileNo + Application.procId;
		String[] divide = null;
		try {
			divide = data.split(",");
			FileWriter fout = new FileWriter(fname, false);
			PrintWriter fileout = new PrintWriter(fout, true);

			versionNum[fileNo-1] = Integer.parseInt(divide[0]);

			fileout.println("VersionNumber:" + divide[0]);
			fileout.print("Data:" + divide[1]);
			fileout.flush();
			fileout.close();
//			System.out.println("* FS: WRIT " + fileNo + ": Version Number: " + divide[0] + ", Data: " + divide[1]);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	protected int writeFileAndUpdateOthers(int fileNo) {
		int newVersion = comparator(verNumList) + 1;
		String data = randomString();
		//System.out.println("* ---------------------------------------");
		//System.out.println("* FS: Writing Version: " + newVersion + ", Data: " + data);
		String writeData = Integer.toString(newVersion) + "," + data;
		writeFile(writeData, fileNo);
		
		String sendData = null;
		SctpChannel[] sendSocket = new SctpChannel[Application.totalNodes];
		InetSocketAddress serverAddr = null;
		ByteBuffer buffer = ByteBuffer.allocateDirect(100);
		CharBuffer charbuf = buffer.asCharBuffer();
		MessageInfo msgInfo = MessageInfo.createOutgoing(null, 0);
		Charset charset = Charset.forName("ISO-8859-1");
		CharsetEncoder encoder = charset.newEncoder();

		//System.out.println("* ---------------------------------------");
		//System.out.println("* FS: Writing to Other Files");

		/* Send a request for Lock and Receive a Reply */
		for (int i = 0; i < lockCount - 1; i++) {

			//System.out.println("* ---------------------------------------");
			//System.out.println("* FS: Connecting to: "
//					+ ipAddr[lockMembers[i] - 1] + ", "
//					+ portN[lockMembers[i] - 1]);
			serverAddr = new InetSocketAddress(ipAddr[lockMembers[i] - 1],
					portN[lockMembers[i] - 1]);

			try {
				sendSocket[i] = SctpChannel.open(serverAddr, 0, 0);
			} catch (IOException e) {
				// e.printStackTrace();
				//System.out.println("* FS: Connection Refused");
				continue;
			}

			sendData = Integer.toString(WRITE) + "-" + Integer.toString(fileNo) + "-" + writeData;

			charbuf.clear();
			charbuf.put(sendData);
			charbuf.flip();

			buffer.clear();
			encoder.encode(charbuf, buffer, true);
			buffer.flip();

			try {
				sendSocket[i].send(buffer, msgInfo);
			} catch (IOException e) {
				// e.printStackTrace();
				//System.out.println("* FS: Send Failed");
				continue;
			}
			//System.out.println("* FS: Send Data: " + sendData);
			buffer.clear();
			charbuf.clear();

		}

		return 0;
	}
	
	public int readFile(int fileNo){
		String fname = "file" + fileNo + Application.procId;
		BufferedReader br = null;
		String currLine= "";
		try{
			br = new BufferedReader(new FileReader(fname));
			while ((currLine = br.readLine()) != null) 
			{
				if(currLine.contains("VersionNumber"))
				{
//					String[] tokens = currLine.split(":");
//					System.out.print("* FS: READ: Version Number: " + tokens[1]);
				}
				if(currLine.contains("Data"))
				{
//					String[] tokens = currLine.split(":");
//					System.out.println(", Data: " + tokens[1]); 
					break;
				}
			}	
		}
		catch (Exception e) {
//			e.printStackTrace();
			System.out.println("* FS: File Not Found");
			return -1;
		}
		try {
			br.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return 0;
	}
	
	protected int readUpdatedVersion(int fileNo) {
		String sendData = null;
		String rplyData = null;
		
		SctpChannel sendSocket = null;
		InetSocketAddress serverAddr = null;
		ByteBuffer buffer = ByteBuffer.allocateDirect(100);
		CharBuffer charbuf = buffer.asCharBuffer();
		MessageInfo msgInfo = MessageInfo.createOutgoing(null, 0);
		Charset charset = Charset.forName("ISO-8859-1");
		CharsetEncoder encoder = charset.newEncoder();
		CharsetDecoder decoder = charset.newDecoder();

		int maxVersion = comparator(verNumList);
		if (versionNum[fileNo-1] >= maxVersion) 
		{
			//System.out.println("* ---------------------------------------");
			//System.out.println("* FS: My version number is the latest");
			if(-1 == readFile(fileNo))
			{
				return -1;
			}
		} 
		else 
		{
			int max_ver_lockMem = verNumList.indexOf(maxVersion);
			int max_ver_proc_index = lockMembers[max_ver_lockMem-1];
			int send_req_id = max_ver_proc_index - 1;

			//System.out.println("* ---------------------------------------");
			//System.out.println("* FS: Sending Request to Max Version Process");

			/* Send a request for max version , data and Receive a Reply */
			//System.out.println("* ---------------------------------------");
			//System.out.println("* FS: Connecting to: "  + ipAddr[send_req_id] + ", " + portN[send_req_id]);
			try 
			{
				serverAddr = new InetSocketAddress(ipAddr[send_req_id], portN[send_req_id]);
			} catch (IllegalArgumentException e) {
				e.printStackTrace();
				System.out.println("* FS: Max Version: " + maxVersion + ", " + max_ver_lockMem);
				System.out.print("* FS: verNumList: ");
				for(int i = 0; i < verNumList.size(); i++) 
					System.out.print(verNumList.get(i) + ", ");
				System.out.println();
				System.out.println("* FS: Getting Data From: " + send_req_id);
				System.out.println("* FS: No Host: " + ipAddr[send_req_id] + ", " +  portN[send_req_id]);
				return -1;
			}
			try {
				sendSocket = SctpChannel.open(serverAddr, 0, 0);
			} catch (IOException e) {
				// e.printStackTrace();
				//System.out.println("* FS: Connection Refused");
				return -1;
			}

			sendData = Integer.toString(READ) + "-" + Integer.toString(fileNo);

			charbuf.clear();
			charbuf.put(sendData);
			charbuf.flip();

			buffer.clear();
			encoder.encode(charbuf, buffer, true);
			buffer.flip();

			try {
				sendSocket.send(buffer, msgInfo);
			} catch (IOException e) {
				// e.printStackTrace();
				//System.out.println("* FS: Send Failed");
				return -1;
			}
			//System.out.println("* FS: Send Data: " + sendData);
			buffer.clear();
			charbuf.clear();
			
			try {
				buffer.clear();
				sendSocket.receive(buffer, null, null);

				buffer.flip();
				try {
					rplyData = decoder.decode(buffer).toString();
				} catch (CharacterCodingException e) {
					// e.printStackTrace();
					System.out.println("* FS: ************ Decoding Error ************");
					return -1;
				}
//				System.out.println("* FS: Reply Buffer Contents are: " + rplyData);

				sendSocket.close();
				
				writeFile(rplyData, fileNo);
				if(-1 == readFile(fileNo))
				{
					return -1;
				}
				
			} catch (IOException e) {
				// e.printStackTrace();
				System.out.println("* FS: ************ Receive Failed ************");
				return -1;
			}

		}
		return 0;
	}
	
	void writeLog(String string, int fileNo) {
		Writer output;
		try {
			output = new BufferedWriter(new FileWriter("Log"+Application.procId, true));
			output.append(string);
			
			if("[" != string && "]" != string) 
				output.append(" " + fileNo +", Write Count = " + writeCount[fileNo-1] + ", Read Count = " + readCount[fileNo-1]);
			
			if("]" == string)
				output.append("\n");

			output.flush();
			output.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return;
	}
}

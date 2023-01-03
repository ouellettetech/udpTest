import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.io.IOException;

public class EchoServer extends Thread {

    private DatagramSocket socket;
    private boolean running;
    private byte[] buf = new byte[256];

    public static void main(String args[]){  
     System.out.println("Hello Java Args... " + args.length);
     if(args.length == 0){
        EchoServer serv = new EchoServer();
	serv.run();
	System.out.println("After starting server");
     } else {
         System.out.println("Test");
     }
    } 

    public EchoServer() {
        try{
	    socket = new DatagramSocket(4445);
	} catch (SocketException se){
		System.out.println("Socexception...");
	}
    }

    public void run() {
        running = true;

        while (running) {
            DatagramPacket packet 
              = new DatagramPacket(buf, buf.length);

		try{
	    		socket.receive(packet);
		} catch(IOException io){
			System.out.println("IOxception...");
		}
            
            InetAddress address = packet.getAddress();
            int port = packet.getPort();
            packet = new DatagramPacket(buf, buf.length, address, port);
            String received 
              = new String(packet.getData(), 0, packet.getLength());
            
            if (received.equals("end")) {
                running = false;
                continue;
            }
	    try{
	            socket.send(packet);
	    } catch(IOException io){
		    System.out.println("IO ception...");
	    }
        }
        socket.close();
    }
}

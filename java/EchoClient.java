import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.Date;
public class EchoClient {
    private DatagramSocket socket;
    private InetAddress address;

    public static void main(String args[]){
     System.out.println("Hello Java Args... " + args.length);
     if(args.length == 0){
        EchoClient client = new EchoClient();
	Date start = new Date();
	System.out.println("Start: " + start.getTime());
	System.out.println(client.sendEcho("Hello"));
	Date end = new Date();
	System.out.println("End: "+end.getTime());
	System.out.println("TTL: "+(end.getTime()-start.getTime()));
	client.close();
	System.out.println("Sent Message");
     } else {
         System.out.println("Test");
     }
    }

    private byte[] buf;

    public EchoClient() {

        try{
            socket = new DatagramSocket(4446);
        } catch (SocketException se){
                System.out.println("Socexception..."+se.getMessage());
        }
	try{
		address = InetAddress.getByName("192.168.151.16");
	} catch (UnknownHostException unknown) {
		System.out.println("Unkown Host");
	}
    }
    
    public String sendEcho(String msg) {
        buf = msg.getBytes();
        DatagramPacket packet
          = new DatagramPacket(buf, buf.length, address, 4445);

        try{
            socket.send(packet);
        } catch(IOException io){
            System.out.println("IOxception...");
        }

        packet = new DatagramPacket(buf, buf.length);

     	try{
        	socket.receive(packet);
        } catch(IOException io){
        	System.out.println("IOxception...");
        }

        String received = new String(
          packet.getData(), 0, packet.getLength());
        return received;
    }

    public void close() {
        socket.close();
    }
}

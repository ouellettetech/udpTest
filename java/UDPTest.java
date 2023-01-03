import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.io.IOException;
import java.net.UnknownHostException;
import junit.framework.*;
public class UDPTest {
    EchoClient client;

    @Before
    public void setup(){
        new EchoServer().start();
        client = new EchoClient();
    }

    @Test
    public void whenCanSendAndReceivePacket_thenCorrect() {
        String echo = client.sendEcho("hello server");
        assertEquals("hello server", echo);
        echo = client.sendEcho("server is working");
        assertFalse(echo.equals("hello server"));
    }

    @After
    public void tearDown() {
        client.sendEcho("end");
        client.close();
    }
}

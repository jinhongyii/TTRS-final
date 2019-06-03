package com.ttrs.ticketbookingapp.Network;

import android.util.Log;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;

public class Client {
    private static final String host = "108.61.126.68"; //or "108.61.126.68" (ip of remote server)
    private static final int port = 9001;

    private Socket socket;

    public void send(final String str, final SocketCallbackListener listener) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    socket = new Socket();
                    socket.setSoTimeout(5000); //change to 5000ms if using remote server, thanks to GFW!
                    socket.connect(new InetSocketAddress(host, port), 5000);

                    Log.d("NetworkActivity", "Sent: " + str);
                    OutputStream os = socket.getOutputStream();
                    os.write(str.getBytes("UTF-8"));
                    socket.shutdownOutput();

                    InputStream is = socket.getInputStream();
                    BufferedReader reader = new BufferedReader(new InputStreamReader(is));
                    StringBuilder ret = new StringBuilder();
                    String tmp;
                    while ((tmp = reader.readLine()) != null) {
                        ret.append(tmp + '\n');
                    }
                    listener.onFinish(ret.toString().trim());
                } catch (Exception e) {
                    listener.onError(e);
                } finally {
                    try {
                        socket.close();
                    } catch (Exception e) {
                        Log.e("NetworkActivity", "connection error" + e.toString());
                    }
                }
            }
        }).start();
    }
}

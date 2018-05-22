package com.avr.avrtermometr;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.Set;
import java.util.UUID;

public class TemperatureActivity2 extends AppCompatActivity {

    static Handler h;
    static final int RECEIVE_MESSAGE = 1;
    TextView temperatureTV;
    String deviceAddress;
    BluetoothAdapter bluetoothAdapter = null;
    BluetoothSocket bluetoothSocket = null;
    ConnectedThread connectedThread;
    Button sendTmpBtn;
    Button monitorBtn;
    EditText editTmpText;
    String actualTemperature;
    String actualTemperature2;
    TextView temperatureTV2;
    StringBuilder stringBuilder;
    String lastTemperature = "0";
    boolean flag = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_temperature2);

        temperatureTV = (TextView) findViewById(R.id.showTempTV);
        temperatureTV2 = (TextView) findViewById(R.id.showTempTV2);
        sendTmpBtn = (Button) findViewById(R.id.sendBtn);
        editTmpText = findViewById(R.id.editTempText);
        monitorBtn = (Button) findViewById(R.id.monitorBtn);

        h = new Handler(){
            public void handleMessage(android.os.Message message){
                switch (message.what){
                    case RECEIVE_MESSAGE:
                        byte[] readBuffer = (byte[]) message.obj;
                        String receivedString = new String(readBuffer, 0, message.arg1);
                        actualTemperature2 = receivedString;
                        actualTemperature = receivedString;
                        runOnUiThread(separateThread);
                        try {
                            if(receivedString.length()==3)
                                actualTemperature = receivedString;
                            else
                                actualTemperature = getValue(receivedString);
                        } catch (IndexOutOfBoundsException e) {
                            actualTemperature = lastTemperature;
                        }
                        runOnUiThread(separateThread);
                        lastTemperature = actualTemperature;
                        break;
                }
            }
        };


    bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        sendTmpBtn.setOnClickListener(new View.OnClickListener(){
           public void onClick(View v){
               connectedThread.write(editTmpText.getText().toString());
           }
        });
        monitorBtn.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                if(flag) {
                    flag = false;
                    monitorBtn.setText("MONITOR");
                    temperatureTV.clearComposingText();
                }
                else{
                    flag = true;
                    monitorBtn.setText("STOP");
                }
            }
        });
    }

    private String getValue(String buffer){
        char[] array = buffer.toCharArray();
        for (int i=0; i<array.length-1; i++){
            if(array[i]=='t' && array[i+1]!=' ') {
                //String newString = array[i]+""+array[i+1];
                return (array[i+1]+""+array[i+2]);
            }
        }
        return null;

    }

    private Runnable separateThread = new Runnable() {
        @Override
        public void run() {
            if(!temperatureTV.getText().equals(actualTemperature))
                temperatureTV.setText(actualTemperature);
        }
    };

    private Runnable separateThread2 = new Runnable() {
        @Override
        public void run() {
            //temperatureTV2.setText("separatethread2");
            //temperatureTV2.setText(actualTemperature2);
            //temperatureTV2.setText("separatethread2");

        }
    };

    private BluetoothSocket createBluetoothSocket(BluetoothDevice bluetoothDevice) throws IOException{
        UUID SERIAL_UUID = bluetoothDevice.getUuids()[0].getUuid();
        try{
            final Method method = bluetoothDevice.getClass().getMethod("createInsecureRfcommSocketToServiceRecord", new Class[] { UUID.class });
        }catch (NoSuchMethodException e1){
            e1.printStackTrace();
            Log.e(" ", "Error creating RFComm connection-no such method");
        }
        catch (Exception e){
            e.printStackTrace();
            Log.e(" ", "Error creating RFComm connection");
        }
        return bluetoothDevice.createRfcommSocketToServiceRecord(SERIAL_UUID);
    }

    @Override
    public void onResume(){
        super.onResume();
        getDeviceAddress("HC-05");
        BluetoothDevice bluetoothDevice = bluetoothAdapter.getRemoteDevice(deviceAddress);
        try {
            bluetoothSocket = createBluetoothSocket(bluetoothDevice);
        }catch (IOException e){
            Toast.makeText(getApplicationContext(), "Cant create socket", Toast.LENGTH_LONG).show();
            finish();
        }
        bluetoothAdapter.cancelDiscovery();
        try {
            bluetoothSocket.connect();
        }catch (IOException e){
            try {
                bluetoothSocket.close();
            }catch (IOException e2){
                Toast.makeText(getApplicationContext(), "Cant close socket", Toast.LENGTH_LONG).show();
                finish();
            }
        }
        connectedThread = new ConnectedThread(bluetoothSocket);
        connectedThread.start();
    }

    @Override
    public void onPause(){
        super.onPause();
        try{
            bluetoothSocket.close();
        }catch (IOException e){
            Toast.makeText(getApplicationContext(), "Exception closing socket", Toast.LENGTH_LONG).show();
            finish();
        }
    }

    private class ConnectedThread extends Thread{

        private final InputStream inputStream;
        private final OutputStream outputStream;

        public ConnectedThread(BluetoothSocket socket){
            InputStream tmpInputStream = null;
            OutputStream tmpOutputStream = null;

            try{
                tmpInputStream = socket.getInputStream();
                tmpOutputStream = socket.getOutputStream();
            }catch (IOException e){
                e.printStackTrace();
            }
            inputStream = tmpInputStream;
            outputStream = tmpOutputStream;
        }

        public void run(){
            byte[] readBuffer = new byte[100];
            int bytesRead;
                while (true){
                    if(flag){
                        try {
                            sleep(100);
                            bytesRead = inputStream.read(readBuffer);
                            //BufferedReader input = new BufferedReader(new InputStreamReader(inputStream));
                            //String line = input.readLine();
                            h.obtainMessage(RECEIVE_MESSAGE, bytesRead, -1, readBuffer).sendToTarget();
                            //h.obtainMessage(RECEIVE_MESSAGE, bytesRead, -1, line).sendToTarget();
                        } catch (IOException e) {
                            e.printStackTrace();
                            //break;
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                }
            }

        }

        public void write(String message){
            byte[] writeBuffer = message.getBytes();
            try{
                outputStream.write(Integer.parseInt(message));
                //outputStream.write(message.getBytes());
            }catch (IOException e){
                Toast.makeText(getApplicationContext(), "Cant send, check connection", Toast.LENGTH_LONG).show();
            }
        }

    }

    private void getDeviceAddress(String name) {
        Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
        for (BluetoothDevice device : pairedDevices) {
            if (device.getName().equals(name))
                this.deviceAddress = device.getAddress();

        }

        if (deviceAddress == null) {
            Toast.makeText(getApplicationContext(), "This device isn't paired", Toast.LENGTH_LONG).show();
            finish();
        }
    }

}

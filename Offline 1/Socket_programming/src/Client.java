import java.awt.*;
import java.io.*;
import java.net.Socket;
import java.util.Scanner;

public class Client {
    private static Socket socket;

    public static void receiveFile(String fileName, ObjectOutputStream outputStream, ObjectInputStream inputStream){

        try {
            //first check if Download folder exists
            File folder = new File("Download/");
            if (!folder.exists()) {
                folder.mkdir();
                System.out.println("Created Download folder");
            }

            //receive file size
            long fileSize = (long) inputStream.readObject();
            System.out.println("Download started: " + fileName + " (" + fileSize + " bytes)");
            File file = new File("Download/" + fileName);

            FileOutputStream fileOutputStream = new FileOutputStream(file);
            BufferedOutputStream bufferedOutputStream = new BufferedOutputStream(fileOutputStream);

            //receive file
            String ack = "";
            int byteRead = 0;
            int totalByteCount = 0;

            while(true){
                //check if input is acknowledgement
                Object input = inputStream.readObject();
                if(input.getClass().equals(ack.getClass())){
                    ack = (String) input;
                    break;
                }
                byte[] bytes = (byte[]) input;
                byteRead = bytes.length;
                totalByteCount += byteRead;
                bufferedOutputStream.write(bytes, 0, byteRead);

            }

            bufferedOutputStream.flush();
            bufferedOutputStream.close();
            fileOutputStream.close();

            System.out.println(ack);
            if(ack.equalsIgnoreCase("complete")){
                System.out.println("Download complete: " + fileName + " (" + totalByteCount + " bytes)");
            }
            else{
                System.out.println("Download failed");
            }

        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    public static void sendFile(ObjectOutputStream outputStream, ObjectInputStream inputStream){
        try {
            //select file from dialog box
            System.out.println("Select file to upload");
            FileDialog fileDialog = new FileDialog((Frame)null, "Select File to Open");
            fileDialog.setMode(FileDialog.LOAD);
            fileDialog.setVisible(true);
            System.out.println("File selected: " + fileDialog.getDirectory() + fileDialog.getFile());

            //send file name by taking input
            System.out.println("Enter file name");
            Scanner scanner = new Scanner(System.in);
            String fileName = scanner.nextLine();
            outputStream.writeObject(fileName);

            //send file size
            File file = new File(fileDialog.getDirectory() + fileDialog.getFile());
            long fileSize = file.length();
            outputStream.writeObject(fileSize);

            //check if file size is greater than buffer size and if buffer is available
            boolean bufferAvailable = (boolean) inputStream.readObject();
            if(!bufferAvailable){
                System.out.println("Buffer full");
                return;
            }

            //get chunk size from server
            int chunkSize = (int) inputStream.readObject();
            System.out.println("Chunk size: " + chunkSize);

            //get file id from server
            int fileID = (int) inputStream.readObject();
            System.out.println("File ID: " + fileID);
            InputStream fileInputStream = new FileInputStream(file);
            byte[] buffer = new byte[chunkSize];
            int length = 0;
            int totalCount = 0;
            socket.setSoTimeout(10000);

            //send file
            while ((length = fileInputStream.read(buffer)) != -1) {
                if(length == chunkSize){
                    outputStream.writeObject(buffer);
                }
                else{
                    byte[] temp = new byte[length];
                    for(int i = 0; i < length; i++){
                        temp[i] = buffer[i];
                    }
                    outputStream.writeObject(temp);
                }
                outputStream.reset();
                String ack = (String) inputStream.readObject();
                System.out.println(ack);
                Thread.sleep(1000);
                totalCount += 1;

            }
            outputStream.writeObject("completed");
            System.out.println("Upload complete: " + fileDialog.getFile() + " (" + totalCount + " chunks)");
            fileInputStream.close();

            String checkStatus = (String) inputStream.readObject();
            if(checkStatus.equalsIgnoreCase("success")){
                System.out.println("Upload successful");
            }
            else if (checkStatus.equalsIgnoreCase("failure")){
                System.out.println("Upload failed");
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) throws Exception {

            //create client socket
            socket = new Socket("localhost", 1234);
            System.out.println("Connected to server");

            //create input and output streams
            ObjectOutputStream outputStream = new ObjectOutputStream(socket.getOutputStream());
            ObjectInputStream inputStream = new ObjectInputStream(socket.getInputStream());

            //create client
            while (true) {
                System.out.println("Enter command: ");
                BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(System.in));
                String command = bufferedReader.readLine();

                //split command into array
                String[] commandArray = command.split(" ");

                //check command
                if (commandArray[0].equalsIgnoreCase("login")) {
                    //send command
                    outputStream.writeObject(command);

                    //receive acknowledgement
                    String ack = (String) inputStream.readObject();
                    System.out.println(ack);

                    //if login successful, start client
                    if (ack.equalsIgnoreCase("Login successful")) {
                        System.out.println("Welcome " + commandArray[1]);
                        while (true) {
                            System.out.println("Enter command: ");
                            command = bufferedReader.readLine();
                            commandArray = command.split(" ");

                            //check command
                            if (commandArray[0].equalsIgnoreCase("upload")) {
                                //send command
                                outputStream.writeObject(command);

                                //send file
                                sendFile(outputStream, inputStream);
                            } else if (commandArray[0].equalsIgnoreCase("download")) {
                                //send command
                                outputStream.writeObject(command);

                                //receive file
                                receiveFile(commandArray[1], outputStream, inputStream);
                            } else if (commandArray[0].equalsIgnoreCase("logout")) {
                                //send command
                                outputStream.writeObject(command);

                                //receive acknowledgement
                                ack = (String) inputStream.readObject();
                                System.out.println(ack);
                                break;
                            } else {
                                System.out.println("Invalid command");
                            }
                        }
                    }
                } else if (commandArray[0].equalsIgnoreCase("exit")) {
                    //send command
                    outputStream.writeObject(command);

                    //receive acknowledgement
                    String ack = (String) inputStream.readObject();
                    System.out.println(ack);
                    break;
                } else {
                    System.out.println("Invalid command");
                }
            }

            //close socket
            socket.close();
    }

}


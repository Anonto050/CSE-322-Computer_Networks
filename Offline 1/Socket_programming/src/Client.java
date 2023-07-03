import java.awt.*;
import java.io.*;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.Scanner;

import static java.lang.Integer.parseInt;

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
            if(ack.equalsIgnoreCase("completed")){
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
            if(!bufferAvailable) {
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
            socket.setSoTimeout(30000);

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

        }
        catch (SocketTimeoutException se){
            System.out.println(se);
            try{
                outputStream.writeObject("Timeout");
            }
            catch (IOException e){
                System.out.println(e);
            }
        }
        catch (Exception e) {
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

            Scanner scanner = new Scanner(System.in);

            while (true){
                //login
                System.out.println("Enter user ID");
                String userID = scanner.nextLine();
                outputStream.writeObject(userID);

                //check if already logged in
                boolean loggedIn = (boolean) inputStream.readObject();
                System.out.println("Logged in: " + loggedIn);
                if(loggedIn){
                    System.out.println("Login successful");
                    break;

                }
                System.out.println("User already logged in");
            }

            while (true) {
                //show options menu
                System.out.println("1. Lookup list of Clients");
                System.out.println("2. Uploaded files");
                System.out.println("3. Lookup public files of other clients");
                System.out.println("4. File Request");
                System.out.println("5. View unread messages");
                System.out.println("6. Upload a file");
                System.out.println("7. Logout");

                //take input from user
                String input = scanner.nextLine();
                outputStream.writeObject(input);

                //perform action based on input
                if (input.equalsIgnoreCase("1")) {
                    //lookup list of clients

                    //receive list of clients who are online
                    ArrayList<String> activeClients = (ArrayList<String>) inputStream.readObject();

                    ArrayList<String> clients = (ArrayList<String>) inputStream.readObject();

                    //print list of clients
                    // among all clients print active clients with "active" tag
                    System.out.println("List of clients:");
                    for (String client : clients) {
                        if (activeClients.contains(client)) {
                            System.out.println(client + " (active)");
                        } else {
                            System.out.println(client);
                        }
                    }

                } else if (input.equalsIgnoreCase("2")) {
                    //want to see uploaded files - both public and private

                    //receive list of files
                    ArrayList<String> publicFiles = (ArrayList<String>) inputStream.readObject();

                    //print list of files
                    System.out.println("List of public files:");
                    for (String file : publicFiles) {
                        System.out.println(file);
                    }

                    ArrayList<String> privateFiles = (ArrayList<String>) inputStream.readObject();
                    //System.out.println(privateFiles);

                    System.out.println("List of private files:");
                    for (String file : privateFiles) {
                        System.out.println(file);
                    }

                    //now download file if user wants to
                    System.out.println("Do you want to download any file? (y/n)");
                    String download = scanner.nextLine();
                    outputStream.writeObject(download);

                    if (download.equalsIgnoreCase("y")) {
                        System.out.println("Enter file ID : ");
                        String file_id = scanner.nextLine();
                        int fileID = parseInt(file_id);
                        outputStream.writeObject(fileID);

                        //check if file exists
                        String fileExists = (String) inputStream.readObject();
                        if (fileExists.equalsIgnoreCase("exists")) {

                            String fileName = (String) inputStream.readObject();
                            System.out.println("File name: " + fileName);
                            receiveFile(fileName, outputStream, inputStream);

                        } else {
                            System.out.println("File does not exist with the file name");
                        }
                    }

                    else if (download.equalsIgnoreCase("n")) {
                        continue;
                    }

                }
                else if (input.equalsIgnoreCase("3")) {

                    //Download public files of other clients
                    System.out.println("Enter client ID");

                    String clientID = scanner.nextLine();
                    outputStream.writeObject(clientID);

                    Object object = inputStream.readObject();
                    Integer clientExists = 0;

                    //check if client exists. If not, server will send 0 otherwise 1
                    if(object.getClass().equals(clientExists.getClass())){
                        //System.out.println(object);
                        clientExists = (Integer) object;
                        //System.out.println(clientExists);
                        if(clientExists == 0){
                            System.out.println(clientID + " does not exist");
                            continue;
                        }
                    }

                        ArrayList<String> publicFiles = (ArrayList<String>) inputStream.readObject();
                        System.out.println("List of public files : ");
                        for (String file : publicFiles) {
                            System.out.println(file);
                        }

                        //now download file if user wants to
                        System.out.println("Do you want to download any file? (y/n)");
                        String download = scanner.nextLine();
                        outputStream.writeObject(download);

                        if (download.equalsIgnoreCase("y")) {
                            System.out.println("Enter file name");
                            String fileName = scanner.nextLine();
                            outputStream.writeObject(fileName);

                            //check if file exists
                            String fileExists = (String) inputStream.readObject();
                            //System.out.println(fileExists);
                            if (fileExists.equalsIgnoreCase("exists")) {

                                System.out.println("File name: " + fileName);
                                receiveFile(fileName, outputStream, inputStream);

                            } else {
                                System.out.println("File does not exist with the file name : " + fileName);
                            }
                        } else if (download.equalsIgnoreCase("n")) {
                            continue;
                        }

                }

                else if (input.equalsIgnoreCase("4")) {
                    //file request
                    System.out.println("Enter description of your file request");
                    String destination = scanner.nextLine();
                    outputStream.writeObject(destination);

                } else if (input.equalsIgnoreCase("5")) {
                    //view unread messages
                    System.out.println("Unread messages or requests : ");

                    ArrayList<String> other_requests = (ArrayList<String>) inputStream.readObject();
                    for (String request : other_requests) {
                        System.out.println("Request ID : " + request);
                    }

                    System.out.println("Uploads for own requests : ");
                    ArrayList<String> own_requests = (ArrayList<String>) inputStream.readObject();
                    for (String request : own_requests) {
                        System.out.println("Request ID : " + request);
                    }
                }
                else if(input.equalsIgnoreCase("6")){
                     //upload file
                    //ask type of file
                    System.out.println("1. Public");
                    System.out.println("2. Private");
                    System.out.println("3. Respond to file request");
                    String type = scanner.nextLine();
                    outputStream.writeObject(type);

                    if(type.equalsIgnoreCase("3")){
                        //respond to file request
                        //enter request id
                        System.out.println("Enter request ID");
                        int requestID = parseInt(scanner.nextLine());
                        outputStream.writeObject(requestID);

                        //check if request is valid
                        boolean validRequest = (boolean) inputStream.readObject();
                        if(validRequest){
                            //send file
                            sendFile(outputStream, inputStream);
                        }
                        else{
                            System.out.println("Invalid request. No such request exists");
                            continue;
                        }
                    }
                    else{
                        //upload file
                        sendFile(outputStream, inputStream);
                    }
                }
                else if(input.equalsIgnoreCase("7")){
                    inputStream.close();
                    outputStream.close();
                    socket.close();
                    System.out.println("Logged out");
                    System.exit(0);
                }
                else{
                    System.out.println("Invalid input");
                }

                }
            }
}



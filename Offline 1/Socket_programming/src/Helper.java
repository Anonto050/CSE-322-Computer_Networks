
import java.io.*;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.ArrayList;
import java.awt.*;

public class Helper extends Thread {

    Socket socket;
    SocketAddress clientAddress;
    String clientID;
    boolean isAlive = true;
    FileOutputStream current_stream;
    String current_file_name = null;
    long current_file_size = 0;

    public Helper(Socket socket) {
        this.socket = socket;
        this.clientAddress = socket.getRemoteSocketAddress();
    }

    public void sendFile(File file, ObjectOutputStream out, ObjectInputStream in) {
        try{
           //send file size
              out.writeObject(file.length());

           //get chunk size from server
            int chunkSize = Server.getMaxChunkSize();
            //create input stream for file
            InputStream fileIn = new FileInputStream(file);
            //create buffer
            byte[] buffer = new byte[chunkSize];
            //read file into buffer
            //int bytesRead = fileIn.read(buffer, 0, chunkSize);
            //while there are bytes to read
            int length = 0;
            int count = 0;

            while ((length = fileIn.read(buffer)) != -1) {
                //write if length is equal to chunk size
                if (length == chunkSize) {
                    out.writeObject(buffer);
                } else {
                    //create new buffer with length
                    byte[] newBuffer = new byte[length];
                    //copy buffer into new buffer
                    System.arraycopy(buffer, 0, newBuffer, 0, length);
                    //write new buffer
                    out.writeObject(newBuffer);
                }
                out.reset();
                count++;

            }
            System.out.println("Chunks sent: " + count);
            out.writeObject("completed");
            //close file input stream
            fileIn.close();
        }
        catch (Exception e) {
            System.out.println(e);
        }
    }

    public String receive_file(String type, ObjectInputStream in, ObjectOutputStream out) {
        //get file name
        try {
            String fileName = (String) in.readObject();
            //get file size
            long fileSize = (long) in.readObject();
            current_file_size = fileSize;
            System.out.println(clientID + " is uploading " + fileName + " of size " + fileSize);

            //check if buffer size is ok
            boolean confirm = Server.confirmBufferSize(fileSize);
            out.writeObject(confirm);
            if (!confirm) {
                System.out.println("Buffer size exceeded");
                return null;
            } else {
                System.out.println("Buffer size ok");
            }

            //generate chunk size and send to client
            int chunkSize = Server.getRandomChunkSize();
            out.writeObject(chunkSize);

            current_file_name = "FTP_Server/" + clientID + "/" + type + "/" + fileName;
            int fileID = Server.addFile(current_file_name);
            System.out.println("File ID: " + fileID);
            out.writeObject(fileID);

            //create file output stream
            File file = new File(current_file_name);
            FileOutputStream fileOut = new FileOutputStream(file);
            BufferedOutputStream bufferOut = new BufferedOutputStream(fileOut);
            current_stream = fileOut;

            String ack = "";
            int count = 0;
            int bytesRead = 0;
            int totalBytesRead = 0;
            boolean finished = false;
            long remainder = (((fileSize % chunkSize) - 1) >> 31) ^ 1;
            long numChunks = (fileSize / chunkSize) + remainder;
            System.out.println("Number of chunks: " + numChunks);

            while (count < numChunks) {
                //read chunk

                //first check if the read object is ack. If it is, then receiving is done
                Object obj = in.readObject();
                //check if both class of objects are same
                if (obj.getClass().equals(ack.getClass())) {
                    ack = (String) obj;
                    System.out.println("Received ack: " + ack);
                    finished = true;
                    break;

                }
                    byte[] buffer = (byte[]) obj;
                    bytesRead = buffer.length;
                    totalBytesRead += bytesRead;
                    //write chunk to file
                    bufferOut.write(buffer, 0, bytesRead);
                    count++;

                System.out.println("Chunks received: " + count);
                //Thread.sleep(35000);
                out.writeObject("Total bytes read: " + totalBytesRead);
            }

            bufferOut.flush();
            bufferOut.close();
            fileOut.close();

            if(finished == false){
                ack = (String) in.readObject();
            }

            System.out.println(ack);

            //if ack is COMPLETE, then check two cases - SUCCESS or FAILURE
            if (ack.equalsIgnoreCase("COMPLETED")) {
                Server.clearBufferSize(totalBytesRead);
                if(totalBytesRead == fileSize){
                    System.out.println("File received successfully");
                    out.writeObject("SUCCESS");
                }
                else{
                    System.out.println("File received partially");
                    out.writeObject("FAILURE");

                    //delete file
                    File f = new File(current_file_name);
                    boolean check = f.delete();
                    if(check){
                        System.out.println("File deleted successfully after partial upload");
                    }
                    else{
                        System.out.println("File deletion failed after partial upload");
                    }
                }
            }
            if(ack.equalsIgnoreCase("TIMEOUT")){
                System.out.println("File transfer timed out");

                //delete file
                File f = new File(current_file_name);
                boolean check = f.delete();
                if(check){
                    System.out.println("File deleted successfully due to timeout");
                }
                else{
                    System.out.println("File deletion failed due to timeout");
                }
            }

            current_file_name = null;
            return fileName;
        }
        catch (Exception e) {
            System.out.println("Error in receiving file");
            System.out.println(e);
            return null;
        }
    }

    public void run(){
        try {
            ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream());
            ObjectInputStream in = new ObjectInputStream(socket.getInputStream());

            while (isAlive) {

                //get client ID
                clientID = (String) in.readObject();
                System.out.println("Client ID: " + clientID);

                //check if client is already logged in
                boolean check = Server.loginClient(clientID, clientAddress);
                out.writeObject(check);

                if (check == false) {
                    System.out.println("Client already logged in");
                    continue;
                }

                //read client request
                while (true){
                    System.out.println("Waiting for client request");
                        String choice = (String) in.readObject();
                        System.out.println("Choice: " + choice);

                    if (choice.equalsIgnoreCase("1")){
                        //Look up for users
                        System.out.println(clientID + " is looking up for users");
                        //get users who are online
                        ArrayList<String> activeUsers = Server.getActiveUsers();
                        //send users to client
                        out.writeObject(activeUsers);

                        //get users who logged in at least once
                        ArrayList<String> allUsers = Server.getAllPaths();
                        //send users to client
                        out.writeObject(allUsers);

                    }
                    else if(choice.equalsIgnoreCase("2")){
                        //show all public and private files one after another
                        System.out.println(clientID + " is looking up for files");
                        //get all public files
                        ArrayList<String> publicFiles = new ArrayList<String>();
                        String publicPath = "FTP_Server/" + clientID + "/public/";
                        File[] files = new File(publicPath).listFiles();
                        for (File file : files) {
                            if (file.isFile()) {
                                int fileID = Server.getFileID(publicPath + file.getName());
                                publicFiles.add(file.getName() + " : " + fileID);
                            }
                        }
                        //send public files to client
                        out.writeObject(publicFiles);

                        //get all private files
                        ArrayList<String> privateFiles = new ArrayList<String>();
                        String privatePath = "FTP_Server/" + clientID + "/private/";
                        files = new File(privatePath).listFiles();
                        for (File file : files) {
                            if (file.isFile()) {
                                int fileID = Server.getFileID(privatePath + file.getName());
                                privateFiles.add(file.getName() + " : " + fileID);
                            }
                        }
                        //send private files to client

                        out.writeObject(privateFiles);


                        //download file if download is requested

                        String downloadOption = (String) in.readObject();
                        //System.out.println("Download option: " + downloadOption);

                        if(downloadOption.equalsIgnoreCase("y")){
                            //download files
                            int fileID = (int) in.readObject();
                            System.out.println("File ID: " + fileID);
                            String filePath = Server.getFileURL(fileID);
                            System.out.println(clientID + " is downloading file: " + filePath);

                            if(filePath == null){
                                //no such file exists
                                System.out.println("File does not exist");
                                out.writeObject("No such file exists in your directory");
                                continue;
                            }
                            else if(filePath.startsWith("FTP_Server/" + clientID + "/public/") || filePath.startsWith("FTP_Server/" + clientID + "/private/")){
                             //file exists in client's directory
                                System.out.println("File exists in client's directory");
                                out.writeObject("exists");

                                //send file to client
                                File file = new File(filePath);
                                String fileName = file.getName();
                                out.writeObject(fileName);
                                sendFile(file, out, in);

                            }
                            else {
                                //no such file exists
                                System.out.println("File does not exist");
                                out.writeObject("No such file exists in your directory");
                                continue;
                            }
                        }
                        else if(downloadOption.equalsIgnoreCase("n")){
                            continue;
                        }

                    }
                    else if(choice.equalsIgnoreCase("3")){
                        //look up public files of others and download
                        System.out.println(clientID + " is looking up for public files of others");
                        //get the user whose files are to be looked up
                        String user = (String) in.readObject();
                        System.out.println("User: " + user);

                        //get the list of uses who logged in at least once
                        ArrayList<String> allUsers = Server.getAllPaths();
                        boolean checkUser = false;
                        for(String u : allUsers){
                            if(u.equalsIgnoreCase(user)){
                                checkUser = true;
                                break;
                            }
                        }
                        if(checkUser == false){
                            //no such user exists
                            Integer userExists = 0;
                            System.out.println("No such user exists");
                            out.writeObject(userExists);
                        }

                        else{
                            //user exists
                            Integer userExists = 1;
                            System.out.println("User exists");
                            System.out.println(clientID + " is looking up for public files of " + user);

                            out.writeObject(userExists);

                            //get all public files of the user
                            ArrayList<String> publicFiles = new ArrayList<String>();
                            String publicPath = "FTP_Server/" + user + "/public/";
                            File[] files = new File(publicPath).listFiles();
                            for (File file : files) {
                                if (file.isFile()) {
                                    //int fileID = Server.getFileID(publicPath + file.getName());
                                    publicFiles.add(file.getName());
                                }
                            }
                            //send public files to client
                            out.writeObject(publicFiles);

                            //download file if download is requested
                            //1. yes 2. no
                            String downloadOption = (String) in.readObject();
                            //System.out.println("Download option: " + downloadOption);


                            if(downloadOption.equalsIgnoreCase("y")){
                                //download files
                                String fileName = (String) in.readObject();
                                System.out.println("File Name: " + fileName);

                                File file = new File(publicPath + fileName);
                                if(file.exists()){
                                    //file exists
                                    System.out.println("File exists");
                                    out.writeObject("exists");

                                    //send file to client
                                    //out.writeObject(fileName);
                                    sendFile(file, out, in);
                                }
                                else{
                                    //file does not exist
                                    System.out.println("File does not exist");
                                    out.writeObject("does not exist");
                                    continue;
                                }

                            }
                            else if(downloadOption.equalsIgnoreCase("n")){
                                continue;
                            }
                        }
                    }
                    else if(choice.equalsIgnoreCase("4")){
                        //request for a file
                        System.out.println(clientID + " is requesting for a file");
                        //get file description
                        String fileDescription = (String) in.readObject();
                        Server.addRequest(fileDescription, clientID);

                    }
                    else if(choice.equalsIgnoreCase("5")){
                        //View all unread messages
                        System.out.println(clientID + " is viewing all unread messages");
                        ArrayList<Request> unreadMessages = Server.getRequests();
                        ArrayList<String> self_message = new ArrayList<String>();
                        //self_message.add("You have unread messages");
                        ArrayList<String> other_message = new ArrayList<String>();
                        //other_message.add("You have unread messages");

                        //request list is updating during the loop.

//                        for(Request r : unreadMessages){
//                            if(r.getRequester().equalsIgnoreCase(clientID)){
//                                //self message
//                                //get upload info of self
//                                ArrayList<uploadInfo> uploadInfos = r.getUploads();
//                                if(uploadInfos.size() == 0){
//                                    //no uploads
//                                    System.out.println("No uploads");
//                                    continue;
//                                }
//
//                                    //get all uploads
//                                    for(uploadInfo u : uploadInfos){
//                                        String message = r.getRequestID() + " : " + u.getUploaderId() + " uploaded " + u.getUploadUrl();
//                                        self_message.add(message);
//                                        System.out.println(message);
//                                    }
//                                    Server.removeRequest(r.getRequestID());
//
//                            }
//                            else{
//                                //other message
//                                other_message.add(r.getRequestID() + " : " + r.getRequester() + " requested for " + r.getRequestDescription());
//                            }
//                        }
                        for (int i = 0; i < unreadMessages.size(); i++) {
                            Request r = unreadMessages.get(i);
                            if(r.getRequester().equalsIgnoreCase(clientID)){
                                //self message
                                //get upload info of self
                                ArrayList<uploadInfo> uploadInfos = r.getUploads();
                                if(uploadInfos.size() == 0){
                                    //no uploads
                                    System.out.println("No uploads");
                                    continue;
                                }

                                //get all uploads
                                for(uploadInfo u : uploadInfos){
                                    String message = r.getRequestID() + " : " + u.getUploaderId() + " uploaded " + u.getUploadUrl();
                                    self_message.add(message);
                                    //System.out.println(message);
                                }
                                Server.removeRequest(r.getRequestID());

                            }
                            else{
                                //other message
                                other_message.add(r.getRequestID() + " : " + r.getRequester() + " requested for " + r.getRequestDescription());
                            }
                        }
                        //send other messages
                        out.writeObject(other_message);
                        //send self messages
                        out.writeObject(self_message);

                    }
                    else if(choice.equalsIgnoreCase("6")){
                         //receive the uploaded file by client
                        System.out.println(clientID + " is uploading a file");
                        String fileType = (String) in.readObject();
                        int requestID = -1;

                        if(fileType.equalsIgnoreCase("1")){
                            fileType = "public";
                        }
                        else if(fileType.equalsIgnoreCase("2")){
                            fileType = "private";
                        }
                        else if(fileType.equalsIgnoreCase("3")) {
                            fileType = "public";
                            requestID = (Integer) in.readObject();
                            boolean checkRequest = Server.requestExists(requestID);
                            out.writeObject(checkRequest);
                            if(checkRequest == false){
                                continue;
                            }
                        }
                        String fileName = receive_file(fileType, in, out);
                        System.out.println("File Name: " + fileName);
                        System.out.println("File Type: " + fileType);
                        //add file to the server
                        Server.addUpload(requestID, clientID, "FTP_Server/" + clientID + "/" + fileType + "/" + fileName);
                    }
                    else if(choice.equalsIgnoreCase("7")){
                          in.close();
                            out.close();
                            socket.close();
                            Server.logoutClient(clientID);
                            System.out.println(clientID + " logged out");
                            isAlive = false;
                            break;
                    }
                }

            }

        } catch (Exception e) {
            if(current_file_name != null){
                if(current_stream != null){
                    try {
                        current_stream.close();
                    } catch (IOException ioException) {
                        ioException.printStackTrace();
                    }
                }
                File file = new File(current_file_name);
                Server.clearBufferSize(current_file_size);

                boolean deleted = file.delete();
                if(deleted){
                    System.out.println(current_file_name + " deleted due to " + clientID + " disconnection");
                }
                else{
                    System.out.println(current_file_name + " not deleted due to " + clientID + " disconnection");
                }
                current_file_name = null;
            }
            System.out.println(e);
            System.out.println(clientID + " disconnected");
            Server.logoutClient(clientID);
        }
    }
}

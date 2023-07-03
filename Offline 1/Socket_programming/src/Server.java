import java.io.File;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.ArrayList;
import java.util.HashMap;

public class Server {
    private static int requestID;
    private static ArrayList<Request> requests = new ArrayList<Request>();
    private static int fileID;
    private static HashMap<Integer, String> files;
    private static HashMap<String, SocketAddress> Clients;
    private static long bufferSize = 0;
    private static long maxBufferSize = 500000;
    private static int maxChunkSize = 50000;
    private static int minChunkSize = 5000;

    public Server() {
        this.requestID = 0;
        this.requests = new ArrayList<Request>();
        this.fileID = 0;
        this.files = new HashMap<Integer, String>();
        this.Clients = new HashMap<String, SocketAddress>();
    }

    //confirm buffer size
    public static boolean confirmBufferSize(long size) {
        if (bufferSize + size > maxBufferSize) {
            return false;
        }
        bufferSize += size;
        System.out.println("Buffer size: " + bufferSize);
        return true;
    }

    //clear buffer size
    public static void clearBufferSize(long size) {
        bufferSize -= size;
        System.out.println("Buffer size: " + bufferSize);
    }

    //get random chunk size
    public static int getRandomChunkSize() {
        return (int) (Math.random() * (maxChunkSize - minChunkSize + 1) + minChunkSize);
    }

    //get max chunk size
    public static int getMaxChunkSize() {
        return maxChunkSize;
    }

    //add request

    public static void addRequest(String requestDescription, String requester) {
        //System.out.println("Request added: " + requestDescription);
        requests.add(new Request(requestID, requestDescription, requester));
        requestID++;
    }

    public static ArrayList<Request> getRequests() {
        return requests;
    }

    //add upload
    public static void addUpload(int requestID, String uploader, String uploadURL) {
        for (Request request : requests) {
            if (request.getRequestID() == requestID) {
                request.acceptRequest(uploader, uploadURL);
                break;
            }
        }
    }

    //remove request
    public static void removeRequest(int requestID) {
        for (Request request : requests) {
            if (request.getRequestID() == requestID) {
                requests.remove(request);
                break;
            }
        }
    }

    //check if request exists
    public static boolean requestExists(int requestID) {
        for (Request request : requests) {
            if (request.getRequestID() == requestID) {
                return true;
            }
        }
        return false;
    }

    //add file
    public static int addFile(String fileURL) {
        files.put(fileID, fileURL);
        fileID++;
        return fileID - 1;
    }

    //get file URL
    public static String getFileURL(int fileID) {
        return files.get(fileID);
    }

    //get file ID
    public static int getFileID(String fileURL) {
        for (int fileID : files.keySet()) {
            if (files.get(fileID).equals(fileURL)) {
                return fileID;
            }
        }
        return -1;
    }

    //login client and create folder using username
    public static boolean loginClient(String username, SocketAddress address) {

        //check if client is already logged in
        if (Clients.containsKey(username)) {
            System.out.println(username + "is already logged in");
            return false;
        }
        Clients.put(username, address);
        System.out.println(username + " logged in with address " + address);

        //create folder for client
        String url = "FTP_Server/" + username + "/";
        File folder = new File(url);
        if (!folder.exists()) {
            folder.mkdir();
            //create public and private folders
            File publicFolder = new File(url + "public/");
            publicFolder.mkdir();
            File privateFolder = new File(url + "private/");
            privateFolder.mkdir();

            System.out.println("Created folder for " + username);
        }
        return true;
    }

    //logout client
    public static void logoutClient(String username) {
        Clients.remove(username);
        System.out.println(username + " logged out");
    }

    //get active users
    public static ArrayList<String> getActiveUsers() {
        ArrayList<String> activeUsers = new ArrayList<String>();
        for (String username : Clients.keySet()) {
            activeUsers.add(username);
        }
        return activeUsers;
    }

    //get all paths from folders that have been created
    public static ArrayList<String> getAllPaths() {
        ArrayList<String> allPaths = new ArrayList<String>();
        File folder = new File("FTP_Server/");
        File[] listOfFiles = folder.listFiles();
        for (File file : listOfFiles) {
            if (file.isDirectory()) {
                allPaths.add(file.getName());
            }
        }
        return allPaths;
    }


    public static void main(String[] args) throws Exception {

        //create file server folder
        File folder = new File("FTP_Server/");
        if (!folder.exists()) {
            folder.mkdir();
            System.out.println("Created FTP_Server folder");
        }

        //create server socket
        Clients = new HashMap<String, SocketAddress>();
        files = new HashMap<Integer, String>();
        ServerSocket serverSocket = new ServerSocket(1234);

        //create server
        while (true) {
            Socket socket = serverSocket.accept();
            System.out.println("Client connected");

            //create thread for client
            Thread helper = new Helper(socket);
            helper.start();
        }

    }
}

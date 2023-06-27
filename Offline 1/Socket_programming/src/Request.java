import java.util.ArrayList;

public class Request {
    private int requestID;
    private String requestDescription;
    private String requester;
    private ArrayList<uploadInfo> uploads;

    public Request(int requestID, String requestDescription, String requester) {
        this.requestID = requestID;
        this.requestDescription = requestDescription;
        this.requester = requester;
        this.uploads = new ArrayList<uploadInfo>();
    }

    public int getRequestID() {
        return requestID;
    }

    public String getRequestDescription() {
        return requestDescription;
    }

    public String getRequester() {
        return requester;
    }

    public ArrayList<uploadInfo> getUploads() {
        return uploads;
    }

   //accept request

    public void acceptRequest(String uploader, String uploadURL) {
        uploads.add(new uploadInfo(uploader, uploadURL));
    }
}

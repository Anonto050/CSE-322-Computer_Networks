public class uploadInfo {
    public String uploaderId;
    public String uploadUrl;

    public uploadInfo(String uploaderId, String uploadUrl) {
        this.uploaderId = uploaderId;
        this.uploadUrl = uploadUrl;
    }

    public String toString() {
        return "uploaderId: " + uploaderId + ", uploadUrl: " + uploadUrl;
    }

    //getter and setter methods

    public String getUploaderId() {
        return uploaderId;
    }

    public void setUploaderId(String uploaderId) {
        this.uploaderId = uploaderId;
    }

    public String getUploadUrl() {
        return uploadUrl;
    }

    public void setUploadUrl(String uploadUrl) {
        this.uploadUrl = uploadUrl;
    }
}

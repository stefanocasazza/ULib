$(document).ready(function() {
    $(function() {
        $('#upload_form').uploadProgress({
            jqueryPath: "/js/jquery.js",
            progressBar: '#progress_indicator',
            progressUrl: '/usp/upload_progress.usp',
            start: function() {
                $("#upload_form").hide();
                filename = $("#id_file").val().split(/[\/\\]/).pop();
                $("#progress_filename").html('Uploading ' + filename + "...");
                $("#progress_container").show();
            },
            uploadProgressPath: "/js/jquery.uploadProgress.js",
            uploading: function(upload) {
                $("#progress_container").hide();
                if (upload.percents == 100) {
                    window.clearTimeout(this.timer);
                    $("#progress_filename").html('Processing ' + filename + "...");
                } else {
                    $("#progress_filename").html('Uploading ' + filename + ': ' + upload.percents + '%');
                }
					 $("#progress_container").show();
            },
            interval: 500
        });
    });
});

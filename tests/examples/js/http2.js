<p >You are currently connected using the protocol: <b  id="transport">checking...</b>.</p>
<p id="summary" />
<script>
    var url = "/";
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function(e) {
        if (this.readyState === 4) {
            var transport = this.status == 200 ? xhr.getResponseHeader("X-Undertow-Transport") : null;
            transport = transport == null ? "unknown" : transport;
            document.getElementById("transport").innerHTML = transport;
            var summary = "No HTTP/2 Support!";
            if (transport.indexOf("h2") == 0) {
                summary = "Congratulations! Your client is using HTTP/2.";
            }
            document.getElementById("summary").innerHTML = summary;
        }
    }
    xhr.open('HEAD', url, true);
    xhr.send();
</script>

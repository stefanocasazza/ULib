#!/usr/local/bin/haserl --upload-limit=4096 --upload-dir=/tmp
<html><body>
<form action="<% echo -n $SCRIPT_NAME %>" method=POST enctype="multipart/form-data" >
<input type=file name=uploadfile>
<input type=submit value=GO>
<br>
<% if test -n "$FORM_uploadfile"; then %>
		  <p>
		  You uploaded a file named <b><% echo -n $FORM_uploadfile_name %></b>, and it was
		  temporarily stored on the server as <i><% echo $FORM_uploadfile %></i>.  The
		  file was <% cat $FORM_uploadfile | wc -c %> bytes long.</p>
		  <% rm -f $FORM_uploadfile %><p>Don't worry, the file has just been deleted
		  from the web server.</p>
<% else %>
		  You haven't uploaded a file yet.
<% fi %>
</form>
</body></html>

<%# Uploading a File
Displays  a form that allows for file uploading.  This is accom-
plished by using the --upload-limit and by setting the form enc-
type  to  multipart/form-data.  If the client sends a file, then
some  information  regarding  the  file  is  printed,  and  then
deleted.   Otherwise,  the  form  states that the client has not
uploaded a file. %>

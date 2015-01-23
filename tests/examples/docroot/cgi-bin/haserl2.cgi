#!/usr/local/bin/haserl
Content-Type: text/html

<html>
<body>
<table border=1><tr>
<% for a in Red Blue Yellow Cyan; do %>
	  <td bgcolor="<% echo -n "$a" %>"><% echo -n "$a" %></td>
	  <% done %>
</tr></table>
</body>
</html>

<%# Looping with dynamic output
Sends a mime-type "text/html" document to the  client,  with  an
html table of with elements labeled with the background color. %>

#!/usr/local/bin/haserl
Content-Type: text/html

<% # define a user function
	table_element() {
		 echo "<td bgcolor=\"$1\">$1</td>"
	 }
%>
<html>
<body>
<table border=1><tr>
<% for a in Red Blue Yellow Cyan; do %>
	  <% table_element $a %>
	  <% done %>
</tr></table>
</body>
</html>

<%# Use Shell defined functions.
Same  as  above,  but  uses a shell function instead of embedded html.  %>

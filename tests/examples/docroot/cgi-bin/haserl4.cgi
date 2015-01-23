#!/usr/local/bin/haserl
<html><body>
<h1>Sample Form</h1>
<form action="<% echo -n $SCRIPT_NAME %>" method="GET">
<% # Do some basic validation of FORM_textfield
	# To prevent common web attacks
	FORM_textfield=$( echo "$FORM_textfield" | sed "s/[^A-Za-z0-9 ]//g" )
	%>
<input type=text name=textfield
	  Value="<% echo -n "$FORM_textfield" | tr a-z A-Z %>" cols=20>
<input type=submit value=GO>
</form></html>
</body>

<%# Self Referencing CGI with a form
Prints a form.  If the client enters text in the form,  the  CGI
is reloaded (defined by $SCRIPT_NAME) and the textfield is sani-
tized to prevent web attacks, then the form is redisplayed  with
the text the user entered.  The text is uppercased.  %>

#!/usr/bin/perl

my $agent = $ENV{HTTP_USER_AGENT};

if ($agent =~ /MSIE/)
	{
	print "Content-Type: text/html\n\n";
	print "\<span\ class\=\"code\"\>You\'re\ using\ Microsoft\&reg\;\ Internet\ Explorer\&reg\;\ 5\.0\<\/span\>\n";
	}
else
	{
	print "Content-Type: text/html\n\n";
	print "\<span\ class\=\"code\"\>You\'re\ not\ using\ Microsoft\&reg\;\ Internet\ Explorer\&reg\;\ 5\.0\<\/span\>\n";
	}

exit 0;

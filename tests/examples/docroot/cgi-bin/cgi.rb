require 'cgi'

cgi = CGI.new("html4")

cgi.out do
  cgi.html do
    cgi.body do
      cgi.h2 {"Esempio di CGI"} +
      cgi.i {"scritto in "} + 
      cgi.a("http://www.ruby-lang.org") { 'linguaggio ruby' }
    end
  end
end

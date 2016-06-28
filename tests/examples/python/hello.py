from cgi import parse_qs, escape, FieldStorage
import time
import shutil

def ping_app(environ, start_response):
    status = '200 OK'
    output = 'Pong!'

    response_headers = [('Content-type', 'text/plain'),
                        ('Content-Length', str(len(output)))]
    start_response(status, response_headers)
    return [output]

def hello_world_app(environ, start_response):
  parameters=parse_qs(environ.get('QUERY_STRING', ''))
  if 'sleep' in parameters:
    time.sleep(5)
  if 'subject' in parameters:
    subject=escape(parameters['subject'][0])
  else:
    subject='World'
  start_response('200 OK', [('Content-Type', 'text/html;charset=utf-8')])
  result=u'<p>Hello, %(subject)s!</p>\n' % {'subject': subject}
  for key, value in iter(sorted(environ.iteritems())):
    result+='<p>'+html_escape(key)+'='+html_escape(value)+'</p>\n'
  content_length=environ.get('CONTENT_LENGTH', 0)
  if content_length and content_length<100:
    result+='bytes read='+environ['wsgi.input'].read()
  return [result.encode('utf-8')]

def file_upload_app(environ, start_response):
  result=''
  if environ['REQUEST_METHOD'].upper()=='POST':
    start_response('200 OK', [('Content-Type', 'text/plain;charset=utf-8')])
    try:
      fs=FieldStorage(fp=environ['wsgi.input'], environ=environ, keep_blank_values=True, strict_parsing=True)
      if fs.list:
        count=0
        for item in fs.list:
          if item.filename:
            count+=1
            result+='%s: file; %s, %s, %s, %r\n' % (item.name, item.filename, item.type, item.disposition, item.file)
            with open('fupl-'+str(count), 'w') as fdst:
              shutil.copyfileobj(item.file, fdst, 8192)
            if hasattr(item.file, 'close'):
              item.file.close()
          else:
            result+='%s: value; %s\n' % (item.name, item.value)
    except Exception as e:
      result='multipart data parse failure: '+repr(e)
  else:
    start_response('200 OK', [('Content-Type', 'text/html;charset=utf-8')])
    result='''
      <form action="/py" method="post" enctype="multipart/form-data">
        Category:       <input type="text" name="category" />
        Select file(s): <input type="file" name="upload" multiple />
        <input type="submit" value="Start upload!" />
      </form>'''
  return [result]

def html_escape(s):
  if not s: return ''
  return unicode(s).replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;').replace('"', '&quot;').replace('\'', '&#39;')

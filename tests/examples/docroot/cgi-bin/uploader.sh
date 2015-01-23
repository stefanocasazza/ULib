#!/bin/sh

# uploader.sh

if [ "$REQUEST_METHOD" = "GET" ]; then

	cat <<END
<html>
<head>
  <title>Sample File Upload Form</title>
  <link type="text/css" href="/css/upload.css" rel="stylesheet">
  <script type="text/javascript" src="/js/jquery.js"></script>
  <script type="text/javascript" src="/js/jquery.uploadProgress.js"></script>
  <script type="text/javascript" src="/js/upload.js"></script>
</head>

<body>
  <h1>Sample File Upload Form</h1>

  <h2>Please fill in the file-upload form below</h2>
  <hr>

  <form id="upload_form" method="post" enctype="multipart/form-data" action="uploader.sh">
    File to upload: <input id="id_file" type="file" name="file" ><br><br>
    Notes about the file: <input type="text" name="note"><br><br>
    <input type="submit" value="Press"> to upload the file!
  </form>

  <div id="progress_container">
    <div id="progress_filename"></div>
	 <div id="progress_bar">
	   <div id="progress_indicator"></div>
	 </div>
  </div>

  <hr>
</body>
</html>
END

	exit 0

elif [ "$REQUEST_METHOD" = "POST" ]; then

	DIR=uploads

	mv $1	../$DIR

	FILE=/$DIR/`basename $1`

	cat <<END
<html>
<head>
  <title></title>
</head>
<body>
  <table align="center" class="table">
    <tr>
      <td class="table_header" colspan="2"><b>Your file have been uploaded!</b></td>
    </tr>
    <tr>
      <td class="table_body"><br>
      <b>File #1:</b> <a href="$FILE" target="_blank">$FILE</a><br>
      <br>
      <br>
      <a href="/cgi-bin/uploader.sh">Go Back</a><br></td>
    </tr>
  </table>
</body>
</html>
END

	exit 0
fi

# printenv -- just prints its environment

echo -e 'Content-Type: text/html; charset=utf8\r\n\r'
echo '<pre>'
env
echo '</pre>'

exit 1

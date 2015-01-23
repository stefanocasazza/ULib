window.setInterval("updateChat()", 1000);

function getRequest()
{
  try { return new XMLHttpRequest();                   } catch (e) { }
  try { return new ActiveXObject("Msxml2.XMLHttp");    } catch (e) { }
  try { return new ActiveXObject("Microsoft.XMLHTTP"); } catch (e) { }
  return null;
}

function ajaxGet(url, fn, failFn)
{
  request = getRequest();
  request.open("GET", url);
  request.onreadystatechange = function () {
      if (request.readyState == 4)
      {
        if (request.status == 200)
        {
          if (fn != null)
            fn(request);
        }
        else if (failFn != null)
          failFn(request);
      }
    }
  request.send(null);
}

var oldContent;

function onReceive(request)
{
  var c = document.getElementById("chat");

  if (request.responseText != oldContent)
  {
	 c.innerHTML = request.responseText;
	 oldContent  = request.responseText;
  }
}

function updateChat(ms)
{
  ajaxGet("cchat", onReceive);
}

function sendMessage()
{
  var p = document.getElementById("person");
  var m = document.getElementById("message");

  ajaxGet("cchat?person=" + escape(p.value) + "&message=" + escape(m.value), onReceive);

  m.value = "";

  m.focus();
}

function addText(t)
{
  var m = document.getElementById("message");

  m.value += t;
}

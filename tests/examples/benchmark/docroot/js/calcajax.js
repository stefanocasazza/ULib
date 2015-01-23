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
          fn(request);
        else if (failFn != null)
          failFn(request);
      }
    }
  request.send(null);
}

function calc(op)
{
	var arg1 = document.getElementById("arg1").value;
	var arg2 = document.getElementById("arg2").value;
	var url = "/servlet/docalc?arg1=" + escape(arg1)
								  + "&arg2=" + escape(arg2)
								  + "&op="   + escape(op);

	ajaxGet(url,
	  function(request)
	  {
		 var e = document.getElementById("result");
		 e.innerHTML = request.responseText;
		 e.style.display = "block";
	  } );
}


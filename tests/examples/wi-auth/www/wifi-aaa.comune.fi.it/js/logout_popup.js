
function doOnLoad(URL, redirurl) {

	popup = window.open(URL, 'logout_popup', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=800,height=600');

	if (redirurl) { self.location = redirurl; }
}

function CloseItOnClick()		  { window.close(); }
function CloseItAfterSomeTime() { setTimeout("window.close();", 5 * 1000); }

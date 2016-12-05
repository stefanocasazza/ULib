
function CloseItOnClick()	{ window.close(); }
function CloseItAfterSomeTime() { setTimeout("window.close();", 5 * 1000); }

function doOnLoad(URL, redirurl) {

	if (redirurl == 'IPHONE') window.close();
	else
		{
		popup = window.open(URL, 'logout_popup', 'toolbar=0,scrollbars=0,location=0,statusbar=0,menubar=0,resizable=0,width=800,height=600');

		self.location = redirurl;
		}
}

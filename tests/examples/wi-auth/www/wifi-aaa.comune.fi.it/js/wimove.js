(function(w, d) {
var wimoveFetchUrl = this.wimoveFetchUrl = function(url) {
  var request = false;
  if (window.XMLHttpRequest) {
    request = new XMLHttpRequest;
  } else if (window.ActiveXObject) {
    request = new ActiveXObject("Microsoft.XMLHttp");
  }
  if (request) {
    request.open("GET", url, false);
    request.send();
    if (request.status == 200) { return request.responseText; }
  }
  return false;
};

/*
var wimoveGetQueryVar = this.wimoveGetQueryVar = function(variable) {
  var query = window.location.search.substring(1); 
  var vars = query.split("&"); 
  for (var i=0;i<vars.length;i++) { 
    var pair = vars[i].split("="); 
    if (pair[0] == variable) { 
      return unescape(pair[1]); 
    } 
  } 
};

var wimoveBuildBanner = this.wimoveBuildBanner = function(baseUrl) {
  var bannerUrls = new Array();
  var ap=wimoveGetQueryVar("ap");
  var gateway=wimoveGetQueryVar("gateway");

  if (ap)
    ap=ap.replace(/@.*$/,'');
  else
    ap='ap';

  if (gateway) {
    gateway=gateway.replace(/:.*$/,'');
    ip=gateway.match(/[0-9]+/g);
    certid = ''+(parseInt(ip[2]) * 254 + parseInt(ip[3]))+'';
    for (var i = certid.length; i < 4; i++) certid = '0'+certid;
    bannerUrls.push(baseUrl + '/X' + certid + 'R' + ap + (isMobile() ? '/mobile' : '/full') + '/banner.html');
  }
  bannerUrls.push(baseUrl + '/default' + (isMobile() ? '/mobile' : '/full') + '/banner.html');

  var banner;
  for (var idx in bannerUrls) {  
    banner = wimoveFetchUrl(bannerUrls[idx]);
    if (banner !== false)
      break;
  }
  if (banner !== false) {
    document.write(banner);
  }
};
*/

var wimoveGetBanner = this.wimoveGetBanner = function(baseUrl,ap) {
  var bannerUrls = new Array();

  if (ap) bannerUrls.push(baseUrl + ap			  + (isMobile() ? '/mobile' : '/full') + '/banner.html');
			 bannerUrls.push(baseUrl + '/default' + (isMobile() ? '/mobile' : '/full') + '/banner.html');

  var banner;
  for (var idx in bannerUrls) {  
    banner = wimoveFetchUrl(bannerUrls[idx]);
    if (banner !== false)
      break;
  }
  if (banner !== false) {
    document.write(banner);
  }
};
})(this, this.document);

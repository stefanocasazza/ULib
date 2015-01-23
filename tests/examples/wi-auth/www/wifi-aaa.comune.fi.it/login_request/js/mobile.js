(function(w, d) {

  var deviceIphone  = "iphone";
  var deviceIpod    = "ipod";
  var deviceIpad    = "ipad";
  var deviceS60     = "series60";
  var deviceSymbian = "symbian";
  var engineWebKit  = "webkit";
  var deviceAndroid = "android";
  var deviceWinMob  = "windows ce";
  var deviceWinPho  = "windows phone";
  var deviceBB      = "blackberry";
  var devicePalm    = "palm";
  var deviceKindle  = "kindle";
  var deviceOperaM  = "opera mini";

  var uagent = navigator.userAgent.toLowerCase();

  var isIphone = this.isIphone = function(){ return uagent.search(deviceIphone) > -1; };
  var isIpod = this.isIpod = function(){ return uagent.search(deviceIpod) > -1; };
  var isIpad = this.isIpad = function(){ return uagent.search(deviceIpad) > -1; };
  var isIphoneOrIpod = this.isIphoneOrIpod = function(){ return isIphone() || isIpod(); };
  var isS60OssBrowser = this.isS60OssBrowser = function(){ return (uagent.search(engineWebKit) > -1) && (uagent.search(deviceS60) > -1 || uagent.search(deviceSymbian) > -1); };
  var isAndroid = this.isAndroid = function(){ return uagent.search(deviceAndroid) > -1; };
  var isAndroidWebKit = this.isAndroidWebKit = function(){ return isAndroid() && isWebkit(); };
  var isWindowsMobile = this.isWindowsMobile = function(){ return uagent.search(deviceWinMob) > -1; };
  var isWindowsPhone = this.isWindowsPhone = function(){ return uagent.search(deviceWinPho) > -1; };
  var isBlackBerry = this.isBlackBerry = function(){ return uagent.search(deviceBB) > -1; };
  var isPalmOS = this.isPalmOS = function(){ return uagent.search(devicePalm) > -1; };
  var isKindle = this.isKindle = function(){ return uagent.search(deviceKindle) > -1; };
  var isOperaMini = this.isOperaMini = function(){ return uagent.search(deviceOperaM) > -1; };

  var isMobile = this.isMobile = function() {
    return isIphoneOrIpod() || isS60OssBrowser() || isAndroid() || isWindowsMobile() || isWindowsPhone() || isBlackBerry() || isPalmOS() || isOperaMini();
  };

  var loadCss = this.loadCss =  function(full,mobile) {
    var css = d.createElement('link');
    css.rel = 'stylesheet';
    css.href = isMobile()?mobile:full;
    (d.head || d.getElementsByTagName('head')[0]).appendChild(css); 
  };

})(this, this.document);

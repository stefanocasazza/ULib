/*
* jquery.uploadProgress
*
* Copyright (c) 2008 Piotr Sarnacki (drogomir.com)
*
* Licensed under the MIT license:
* http://www.opensource.org/licenses/mit-license.php
*
*/
(function($) {
  $.fn.uploadProgress = function(options) {
  options = $.extend({
    dataType: "json",
    interval: 2000,
    progressBar: "#progressbar",
    progressUrl: "/progress",
    start: function() {},
    uploading: function() {},
    complete: function() {},
    success: function() {},
    error: function() {},
    preloadImages: [],
    uploadProgressPath: '/javascripts/jquery.uploadProgress.js',
    jqueryPath: '/javascripts/jquery.js',
    timer: ""
  }, options);
  
  $(function() {
    //preload images
    for(var i = 0; i<options.preloadImages.length; i++)
    {
     options.preloadImages[i] = $("<img>").attr("src", options.preloadImages[i]);
    }
    /* tried to add iframe after submit (to not always load it) but it won't work.
    safari can't get scripts properly while submitting files */
    if($.browser.safari && top.document == document) {
      /* iframe to send ajax requests in safari
       thanks to Michele Finotto for idea */
      iframe = document.createElement('iframe');
      iframe.name = "progressFrame";
      $(iframe).css({width: '0', height: '0', position: 'absolute', top: '-3000px'});
      document.body.appendChild(iframe);
      
      var d = iframe.contentWindow.document;
      d.open();
      /* weird - safari won't load scripts without this lines... */
      d.write('<html><head></head><body></body></html>');
      d.close();
      
      var b = d.body;
      var s = d.createElement('script');
      s.src = options.jqueryPath;
      /* must be sure that jquery is loaded */
      s.onload = function() {
        var s1 = d.createElement('script');
        s1.src = options.uploadProgressPath;
        b.appendChild(s1);
      }
      b.appendChild(s);
    }
  });
  
  return this.each(function(){
    $(this).bind('submit', function() {
      var uuid = "";
      for (i = 0; i < 32; i++) { uuid += Math.floor(Math.random() * 16).toString(16); }
      
      /* update uuid */
      options.uuid = uuid;
      /* start callback */
      options.start();
 
      /* patch the form-action tag to include the progress-id if X-Progress-ID has been already added just replace it */
      if(old_id = /X-Progress-ID=([^&]+)/.exec($(this).attr("action"))) {
        var action = $(this).attr("action").replace(old_id[1], uuid);
        $(this).attr("action", action);
      } else {
       $(this).attr("action", jQuery(this).attr("action") + "?X-Progress-ID=" + uuid);
      }
      var uploadProgress = $.browser.safari ? progressFrame.jQuery.uploadProgress : jQuery.uploadProgress;
      options.timer = window.setInterval(function() { uploadProgress(this, options) }, options.interval);
    });
  });
  };
 
jQuery.uploadProgress = function(e, options) {
  jQuery.ajax({
    type: "GET",
    dataType: options.dataType,
    url: options.progressUrl + "?X-Progress-ID=" + options.uuid,
	 /*
	  * url: options.progressUrl,
	  * beforeSend: function(xhr) { xhr.setRequestHeader("X-Progress-ID", options.uuid); },
	  */ 
    success: function(upload) {
      if (upload.state == 'uploading') {
		  upload.percents = Math.floor((parseInt(upload.received) / parseInt(upload.size))*1000)/10;
        var bar = $.browser.safari ? $(options.progressBar, parent.document) : $(options.progressBar);
        bar.css({width: upload.percents+'%'});
        options.uploading(upload);
      }
      
      if (upload.state == 'done' || upload.state == 'error') {
        window.clearTimeout(options.timer);
        options.complete(upload);
      }
      
      if (upload.state == 'done') {
        options.success(upload);
      }
      
      if (upload.state == 'error') {
        options.error(upload);
      }
    }
  });
};
 
})(jQuery);

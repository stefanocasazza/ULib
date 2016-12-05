function getItems() {

	var html='';

	jQuery.ajax({
	    url:'banner/slider/firenzedigitale.html',
	    type:'get',
	    dataType:'html',
	    success:function(data)
	   { 
	       var _html= jQuery(data);
	        rows = _html.find('.3u');
	        _html.find('div.container > div.row').children().each(function() {
		       html += '<div>\n';
		       html += $(this).html();
		       html += '</div>\n';
		});

	        $('.slider').slick('slickAdd',html);

	   },
	   error:function(data) {
		alert('error!');
	   }
	});
}

function getItems() {


       $.getJSON('banner/slider/firenzedigitale.json', function(data) {

        var html='';

	for (i = 0; i < data.rows.length; i++) {

                                html += '<div>\n';
                                html += '<section class="box feature last">';
                                html += '<a target="_blank" title="'+data.rows[i].Titolo+'" href="http://'+data.rows[i].URL+'" class="image fit"><img src="http://'+data.rows[i].Logo+'" alt="'+data.rows[i].Titolo+'" /></a>'; 
				html += '<div class="inner">';
				html += '<header>';
				html += '<h2>'+ data.rows[i].Titolo +'</h2>';
                                html +='</header>';
                                html += '</div>';
                                html += '</section>';
                                html += '</div>\n';

	}
	        $('.slider').slick('slickAdd',html);
       });

}

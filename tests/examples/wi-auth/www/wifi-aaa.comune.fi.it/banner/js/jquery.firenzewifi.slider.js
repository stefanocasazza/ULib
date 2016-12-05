function createSlider(nrMaxSlides) {

	$('.slider').slick({
	  dots: true,
	  infinite: true,
	  speed: 100,
	  slidesToShow: nrMaxSlides,
	  slidesToScroll: 1,
	  autoplay: true,
	  autoplaySpeed: 3000,
	  responsive: [
	    {
	      breakpoint: 1024,
	      settings: {
		slidesToShow: nrMaxSlides,
		slidesToScroll: 1,
		infinite: true,
		dots: true
	      }
	    },
	    {
	      breakpoint: 600,
	      settings: {
		slidesToShow: 1,
		slidesToScroll: 1
	      }
	    },
	    {
	      breakpoint: 480,
	      settings: {
		slidesToShow: 1,
		slidesToScroll: 1
	      }
	    }
	    // You can unslick at a given breakpoint now by adding:
	    // settings: "unslick"
	    // instead of a settings object
	  ]
	});
}

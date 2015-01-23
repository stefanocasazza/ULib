curvyCorners.addEvent(window, 'load', initCorners);

function initCorners() {
 var settings = {
	tl: { radius: 10 },
	tr: { radius: 10 },
	bl: { radius: 10 },
	br: { radius: 10 },
	antiAlias: true
	}

 curvyCorners(settings, "#loginbox");
}

/*
function stampa(without_element1,without_element2) {

	var self = this;

	this.element1 = without_element1.nodeName ? without_element1 : document.getElementById(without_element1);

	this.element1.style.visibility = "hidden";

	this.element2 = without_element2.nodeName ? without_element2 : document.getElementById(without_element2);

	this.element2.style.visibility = "hidden";

	self.print();

	this.wait = 5000;

	setTimeout( function(){ self.element1.style.visibility = "visible"; self.element2.style.visibility = "visible" }, self.wait );

	return false;
}
*/

function stampa(url) {

	window.open(url, 'Stampa', 'scrollbars=1,resizable=1');

	return false;
}

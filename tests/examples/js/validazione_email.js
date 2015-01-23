/* fonte del codice: www.nextlevelprogramming.com */

var user, domain, regex, _match, email;

/* costruttore dell'oggetto email */

function Email(e) {

	this.emailAddr = e;
	this.message	= "";
	this.valid		= false;
}

/* metodo che stampa su schermo il risultato della validazione dell'email */

function eMsg(msg, sColor) {

	var div = document.getElementById("cpe_emailAddress"); /* prende l'id del div contenitore del messaggio */

	div.style.color	 = sColor;
	div.style.fontSize = "0.9em";

	/* se il div contenitore ha già un messaggio viene rimosso */

	if (div.hasChildNodes()) div.removeChild(div.firstChild);

	/* altrimenti viene appeso direttamente */

	div.appendChild(document.createTextNode(msg));
}

/* questo metodo controlla la validità dell'indirizzo email attraverso espressioni regolari,
 * ad esempio controlla se sono state inserite stringhe vuote, spazi etc...
 */

function validate() {

	if (this.emailAddr					== null	||
		 this.emailAddr.length			== 0		||
		 this.emailAddr.indexOf(".")	== -1		||
		 this.emailAddr.indexOf("@")	== -1		||
		 this.emailAddr.indexOf(" ")	!= -1)
		{
		this.message = "email invalida";
		this.valid	 = false;

		return;
		}

	/* la parte user dell'email non può iniziare o finire per "." */

	regex = /(^\w{2,}\.?\w{2,})@/;

	/* viene chiamato il metodo exec che fa il match della stringa email con l'espressione regolare regex.
	 * se non c'è match viene restituito null, cioè false. se c'è match viene restituito un array contenente
	 * la (o le) posizione/i del matching nella stringa
	 */

	_match = regex.exec(this.emailAddr);

	if (_match) user = RegExp.$1;
	else
		{
		this.valid	 = false;
		this.message = "email invalida";

		return;
		}

	/* controlla il dominio della mail */

	regex = /@(\[\d{1,3}\.\d{1,3}\.\d{1,3}.\d{1,3}\])$/;

	_match = regex.exec(this.emailAddr);

	if (_match)
		{
		domain	  = RegExp.$1;
		this.valid = true;
		}
	else
		{
		/* il carattere @ seguito da almeno 2 caratteri che non sono un periodo (.)
		 * seguito da un periodo, seguito da zero o istanze di almeno due
		 * caratteri che termina con un periodo, seguito da due-tre caratteri che non sono periodi
		 */

		regex  = /@(\w{2,}\.(\w{2,}\.)?[a-zA-Z]{2,3})$/;
		_match = regex.exec(this.emailAddr);

		if (_match) domain = RegExp.$1;
		else
			{
			this.valid	 = false;
			this.message = "email invalida";

			return;
			}
		}

	this.valid = true;
	}

/* validate() l'oggetto email */

Email.prototype.validate = validate;

/* questa funzione instanzia l'oggetto email con il valore passato in input e chiama il metodo validate su di esso */

function checkAddress(val) {

	var eml = new Email(val);

	eml.validate();

	if (! eml.valid)
		{
		/* email non valida sintatticamente, viene stampato sulla pagina un messaggio di errore */

		eMsg(eml.message, "#de7275");

		return 0;
		}
	else
		{
		/* email valida sintatticamente, viene stampato sulla pagina un messaggio di convalida
		eMsg("email valida", "#6EAB03");
		*/

		return 1;
		}
}

function validazione() {

	var form  = document.forms[0];
	var email = form.cpe_emailAddress.value;

	if (checkAddress(email) == 1) form.submit();
	else
		{
		alert("attenzione email invalida");

		return false;
		}
}

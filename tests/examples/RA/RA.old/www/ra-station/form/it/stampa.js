  <script language="JavaScript" type="text/javascript">
//<![CDATA[
  function stampa(without_element) {

        var self = this;

        this.element = without_element.nodeName ? without_element : document.getElementById(without_element);

        this.element.style.visibility = "hidden";

        self.print();

        this.wait = 5000;

        setTimeout( function(){ self.element.style.visibility = "visible" }, self.wait);

        return false;
  }
  //]]>
  </script>

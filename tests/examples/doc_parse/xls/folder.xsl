<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<html>
  <body>
    <h4><xsl:value-of select="*/description"/></h4>
    <table border="1">
      <tr bgcolor="#f8fd9e">
        <th colspan="2" align="left">Dati Cartella</th>
      </tr>
      <tr>
        <td>Proprietario operazione</td>
        <td><xsl:value-of select="*/owner" /></td>
      </tr>
      <tr>
        <td>Descrizione operazione</td>
        <td><xsl:value-of select="*/description" /></td>
      </tr>
      <tr>
        <td>Identificativo utente</td>
        <td><xsl:value-of select="*/uid" /></td>
      </tr>
      <tr>
        <td>Data</td>
        <td><xsl:value-of select="*/timestamp" /></td>
      </tr>
    </table>
  </body>
</html>
</xsl:template>
</xsl:stylesheet>

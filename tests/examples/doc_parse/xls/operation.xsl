<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/">
    <html>
      <body>
        <h4>
          <xsl:value-of select="*/description" />
        </h4>
        <table border="1">
          <tr bgcolor="#f8fd9e">
            <th colspan="2" align="left">Dati operazione</th>
          </tr>
          <tr>
            <td>Proprietario operazione</td>
            <td>
              <xsl:value-of select="*/owner" />
            </td>
          </tr>
          <tr>
            <td>Descrizione operazione</td>
            <td>
              <xsl:value-of select="*/description" />
            </td>
          </tr>
          <tr>
            <td>Identificativo utente</td>
            <td>
              <xsl:value-of select="*/uid" />
            </td>
          </tr>
          <tr>
            <td>Data</td>
            <td>
              <xsl:value-of select="*/timestamp" />
            </td>
          </tr>

          <xsl:if test="*/ra-documents/document/dati_utente != ''">
          <tr bgcolor="#f8fd9e">
            <th colspan="2" align="left">Dati utente</th>
          </tr>
          </xsl:if>

          <!-- mobile -->

          <xsl:if test="*/waCell != ''">
          <tr bgcolor="#f8fd9e">
            <th colspan="2" align="left">Dati Cliente</th>
          </tr>
            <tr>
              <td>Cellulare</td>
              <td>
                <xsl:value-of select="*/waCell" />
              </td>
            </tr>
            <tr>
              <td>Data ricezione sms</td>
              <td>
                <xsl:value-of select="*/waSmsReceptionDate" />
              </td>
            </tr>
            <tr>
              <td>Identificativo carta</td>
              <td>
                <xsl:value-of select="*/waCardId" />
              </td>
            </tr>
            <tr>
              <td>Validita' (gg)</td>
              <td>
                <xsl:value-of select="*/waValidity" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/dati_utente/nome/@value != ''">
            <tr>
              <td>Nome</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/nome/@value" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/dati_utente/nome/@value != ''">
            <tr>
              <td>Cognome</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/cognome/@value" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/dati_utente/luogo_di_nascita/@value != ''">
            <tr>
              <td>Luogo di nascita</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/luogo_di_nascita/@value" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/dati_utente/data_di_nascita/@value != ''">
            <tr>
              <td>Data di nascita</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/data_di_nascita/@value" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/dati_utente/sesso/@value != ''">
            <tr>
              <td>Sesso</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/sesso/@value" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/dati_utente/cf/@value != ''">
            <tr>
              <td>Codice Fiscale</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/cf/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/indirizzo/@value != ''">

            <tr>
              <td>Indirizzo</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/indirizzo/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/comune/@value != ''">

            <tr>
              <td>Comune</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/comune/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/provincia/@value != ''">

            <tr>
              <td>Provincia</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/provincia/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/cap/@value != ''">

            <tr>
              <td>CAP</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/cap/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/tipo_di_documento/@value != ''">

            <tr>
              <td>Tipo di documento</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/tipo_di_documento/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/numero_documento/@value != ''">

            <tr>
              <td>Numero documento</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/numero_documento/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/ente_di_emissione/@value != ''">

            <tr>
              <td>Ente di emissione</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/ente_di_emissione/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/data_rilascio/@value != ''">

            <tr>
              <td>Data di rilascio</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/data_rilascio/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/telefono_fisso/@value != ''">

            <tr>
              <td>Telefono fisso</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/telefono_fisso/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/telefono_fax/@value != ''">

            <tr>
              <td>Fax</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/telefono_fax/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/dati_utente/telefono_cellulare/@value != ''">

            <tr>
              <td>Cellulare</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/dati_utente/telefono_cellulare/@value" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/installazione != ''">
          <tr bgcolor="#f8fd9e">
            <th colspan="2" align="left">Dati installazione</th>
          </tr>
          </xsl:if>

          <xsl:if test="*/ra-docoperazioneuments/document/installazione/indirizzo/@value != ''">

            <tr>
              <td>Indirizzo installazione</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/installazione/indirizzo/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/installazione/comune/@value != ''">

            <tr>
              <td>Comune installazione</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/installazione/comune/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/installazione/provincia/@value != ''">

            <tr>
              <td>Provincia installazione</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/installazione/provincia/@value" />
              </td>
            </tr>
          </xsl:if>
          <xsl:if test="*/ra-documents/document/installazione/cap/@value != ''">

            <tr>
              <td>CAP installazione</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/installazione/cap/@value" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/contratto/note/@value != ''">
            <tr>
              <td>Note contratto</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/contratto/note/@value" />
              </td>
            </tr>
          </xsl:if>

          <xsl:if test="*/ra-documents/document/network/name/@value != ''">
            <tr>
              <td>Rete</td>
              <td>
                <xsl:value-of select="*/ra-documents/document/network/name/@value" />
              </td>
            </tr>
          </xsl:if>

          <tr bgcolor="#f8fd9e">
            <th colspan="2" align="left">Servizi sottoscritti</th>
          </tr>
          <xsl:for-each select="*/subscribed-services/service">
            <tr>
              <td colspan="2">
                <table border="1">
                  <tr>
                    <xsl:for-each select="*">
                      <th bgcolor="#E5ECF9">
                        <xsl:value-of select="local-name()" />
                      </th>
                      <td>
                        <xsl:value-of select="current()" />
                      </td>
                    </xsl:for-each>
                  </tr>
                </table>
              </td>
            </tr>
          </xsl:for-each>

          <xsl:if test="*/customer-credentials != ''">
          <tr bgcolor="#f8fd9e">
            <th colspan="2" align="left">Credenziali cliente</th>
          </tr>
          </xsl:if>

          <xsl:for-each select="*/customer-credentials/*">
            <tr>
              <td colspan="2">
                <table border="1">
                      <th colspan="10" align="left" bgcolor="#E5ECF9">
			<xsl:value-of select="local-name()" />
                      </th>
                  <tr>
                    <xsl:for-each select="*">
                      <th bgcolor="#E5ECF9">
                        <xsl:value-of select="local-name()" />
                      </th>
                      <td>
                        <xsl:value-of select="current()" />
                      </td>
                    </xsl:for-each>
                  </tr>
                </table>
              </td>
            </tr>
          </xsl:for-each>

          <xsl:if test="*/customer-credentials != ''">
          <tr bgcolor="#f8fd9e">
            <th colspan="2" align="left">Credenziali operatore RA</th>
          </tr>

          </xsl:if>

          <xsl:for-each select="*/requester-credentials/*">
            <tr>
              <td colspan="2">
                <table border="1">
                      <th colspan="10" align="left" bgcolor="#E5ECF9">
			<xsl:value-of select="local-name()" />
                      </th>
                  <tr>
                    <xsl:for-each select="*">
                      <th bgcolor="#E5ECF9">
                        <xsl:value-of select="local-name()" />
                      </th>
                      <td>
                        <xsl:value-of select="current()" />
                      </td>
                    </xsl:for-each>
                  </tr>
                </table>
              </td>
            </tr>
          </xsl:for-each>

	<xsl:if test="(*/cpe-delivery-status != '') or (*/services-delivery-status != '')">
          <tr bgcolor="#f8fd9e">
            <th colspan="2" align="left">Stato consegna</th>
          </tr>
          </xsl:if>

	<xsl:if test="*/cpe-delivery-status != ''">
            <tr>
                      <th align="left" bgcolor="#E5ECF9">
                          CPE
                      </th>
                      <td>
                        <xsl:value-of select="*/cpe-delivery-status" />
                      </td>
            </tr>
	  </xsl:if>

	<xsl:if test="*/services-delivery-status != ''">
            <tr>
                      <th align="left" bgcolor="#E5ECF9">
                          Safe Messaging
                      </th>
                      <td>
                        <xsl:value-of select="*/services-delivery-status" />
                      </td>
            </tr>
	  </xsl:if>


        </table>
      </body>
    </html>
  </xsl:template>
</xsl:stylesheet>

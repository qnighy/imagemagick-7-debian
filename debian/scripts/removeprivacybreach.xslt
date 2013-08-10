<?xml version="1.0"?>

<xsl:stylesheet version="1.0"
		xmlns:xhtml="http://www.w3.org/1999/xhtml"
		xmlns="http://www.w3.org/1999/xhtml"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"
		exclude-result-prefixes="xhtml xsl xs">

  <xsl:output method="html" 
	      version="1.0" encoding="UTF-8" 
	      doctype-public="-//W3C//DTD XHTML 1.1//EN" 
	      doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd" 
	      indent="no"/>

  <!-- the identity template -->
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="xhtml:html">
    <xsl:text>&#10;</xsl:text>
    <xsl:comment>Regenerated with privacy breach removal script</xsl:comment>
    <xsl:text>&#10;</xsl:text>
    <xsl:copy>
        <xsl:apply-templates select="@* | node()" />
    </xsl:copy>
  </xsl:template>
    
  <!-- remove various privacy breach -->
  <xsl:template match="xhtml:script[@type='text/javascript' and contains(@src,'googlesyndication')]" />
  <xsl:template match="xhtml:script[@type='text/javascript' and contains(text(),'google_ad')]" />
  <xsl:template match="xhtml:script[@type='text/javascript' and contains(comment(),'google_ad')]" />
  <xsl:template match="xhtml:script[@type='text/javascript' and contains(text(),'apis.google.com/js/plusone.js')]" />
  <xsl:template match="xhtml:script[contains(text(),'GoogleAnalyticsObject')]" />
  <xsl:template match="xhtml:div[@class='g-plusone']" />
  <xsl:template match="meta[@name='google-site-verification']" />
  <xsl:template match="xhtml:script[@type='text/javascript' and contains(@src,'pagead/show_ads.js')]" />
  <xsl:template match="xhtml:script[@type='text/javascript' and contains(text(),'document.getElementById(&quot;gplusone&quot;)')]" />
  <xsl:template match="xhtml:script[@type='text/javascript' and contains(text(),'http://api.flattr.com/js/0.6/load.js?mode=auto')]" />
  
</xsl:stylesheet>
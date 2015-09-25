<?xml version="1.0"?>

<xsl:stylesheet version="1.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:xs="http://www.w3.org/2001/XMLSchema"
		exclude-result-prefixes="xsl xs">

  <xsl:output method="xml"
	      omit-xml-declaration="yes" 
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

  <xsl:template match="html">
    <xsl:text>&#10;</xsl:text>
    <xsl:comment>Regenerated with privacy breach removal script</xsl:comment>
    <xsl:text>&#10;</xsl:text>
    <xsl:copy>
        <xsl:apply-templates select="@* | node()" />
    </xsl:copy>
  </xsl:template>
    
  <!-- remove various privacy breach -->
  <xsl:template match="script[contains(@src,'googlesyndication')]" />
  <xsl:template match="script[contains(text(),'google_ad')]" />
  <xsl:template match="script[contains(comment(),'google_ad')]" />
  <xsl:template match="script[contains(text(),'apis.google.com/js/plusone.js')]" />
  <xsl:template match="script[contains(text(),'GoogleAnalyticsObject')]" />
  <xsl:template match="div[@class='g-plusone']" />
  <xsl:template match="meta[@name='google-site-verification']" />
  <xsl:template match="script[contains(@src,'pagead/show_ads.js')]" />
  <xsl:template match="script[contains(text(),'document.getElementById(&quot;gplusone&quot;)')]" />
  <xsl:template match="adsense" />
  <xsl:template match="CustomSearchEngine" />
  <xsl:template match="script[contains(@src,'google.com/jsapi')]" />
  <xsl:template match="script[contains(text(),'google.search.CustomSearchControl')]" />
  <xsl:template match="script[contains(@src,'adsbygoogle')]" />
  <xsl:template match="script[contains(text(),'window.adsbygoogle')]" />

  <!-- flattr -->
  <xsl:template match="script[contains(text(),'http://api.flattr.com/js/0.6/load.js?mode=auto')]" />
  <xsl:template match="img[contains(@src,'api.flattr.com/button/flattr-badge-large.png')]" />
  <xsl:template match="iframe[contains(@src,'tools.flattr.net/widgets/thing.html')]" />
  <xsl:template match="div[child::a[@class='FlattrButton']]" />
  <xsl:template match="noscript[child::a[@href='http://flattr.com/thing/947300/Convert-Edit-And-Compose-Images']]" />

  <!-- replace online jquery with local one -->
  <xsl:template match="script[contains(@src,'jquery.min.js')]">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:attribute name="src">
	<xsl:value-of select="'/usr/share/javascript/jquery/jquery.min.js'"/>
      </xsl:attribute>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="script[contains(@src,'jquery.mousewheel')]">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:attribute name="src">
	<xsl:value-of select="'/usr/share/javascript/jquery-mousewheel/jquery.mousewheel.js'"/>
      </xsl:attribute>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="script[contains(@src,'jquery.fancybox.pack.js')]">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:attribute name="src">
	<xsl:value-of select="'/usr/share/javascript/jquery-fancybox/jquery.fancybox.min.js'"/>
      </xsl:attribute>
    </xsl:copy>
  </xsl:template>
  
  <xsl:template match="link[contains(@href,'jquery.fancybox.css')]">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:attribute name="href">
	<xsl:value-of select="'/usr/share/javascript/jquery-fancybox/jquery.fancybox.css'"/>
      </xsl:attribute>
    </xsl:copy>
  </xsl:template>

  <!-- temporary remove optionnal package -->
  <xsl:template match="link[@type='text/css' and contains(@href,'jquery.fancybox-buttons.css')]" />
  <xsl:template match="script[contains(@src,'jquery.fancybox-buttons.js')]" />
  <xsl:template match="script[contains(@src,'jquery.fancybox-media.js')]" />
  <xsl:template match="link[@type='text/css' and contains(@href,'jquery.fancybox-thumbs.css')]" />
  <xsl:template match="script[contains(@src,'jquery.fancybox-thumbs.js')]" />

  <!-- remove rss meta data -->
  <xsl:template match="link[@href='http://imagemagick.org/ici.rdf']" />

</xsl:stylesheet>
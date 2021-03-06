<?xml version="1.0"?> 
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" omit-xml-declaration="yes" />

<!-- XSLT - Numbering - format= "&#x0049;" (Roman-Upper) & = "&#x0069;" (Roman-Lower)  - letter-value="alphabetic" | "traditional" -->
<!-- Created : Khalil Jabrane -->
<!-- Date: 04/12/2000 -->

<xsl:template match="/">  
	<TABLE>  
		<xsl:for-each select="//humanoid">
			<TR><TD>  
				<TD>
					<!-- Roman-Upper -->
					<xsl:number format="&#x0049;" letter-value="traditional" /> *					
					<xsl:value-of select = "text()"/>
				</TD>
				
				<TD>
					<!-- Roman-Lower -->
					<xsl:number format="&#x0069;" letter-value="alphabetic" /> *					
					<xsl:value-of select = "text()"/>
				</TD>
			</TD></TR>
		</xsl:for-each>  
	</TABLE>
</xsl:template>  

</xsl:stylesheet>  


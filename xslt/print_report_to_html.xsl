<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" version="1.0">
<xsl:output encoding="utf8" method="xml" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="DTD/xhtml1-strict.dtd"/>

<xsl:template match="report_print">
<html xml:lang="en" lang="en">

<head>
<title>
  Details: <xsl:value-of select="@title"/>
</title>
</head>

<body>

<h1>
  Table: <xsl:value-of select="@table"/>
</h1>

<xsl:apply-templates/>

</body>
</html>
</xsl:template>

<xsl:template match="group_by">
<table>
        <xsl:apply-templates/>
</table>
</xsl:template>

<xsl:template match="row">
<tr>
<xsl:apply-templates/>
</tr>
</xsl:template>


<xsl:template match="field">
<td> <xsl:value-of select="@value"/> </td>
</xsl:template>


</xsl:stylesheet>

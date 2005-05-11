<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" version="1.0">
<xsl:output encoding="utf8" method="xml" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="DTD/xhtml1-strict.dtd"/>

<xsl:template match="report_print">
<html xml:lang="en" lang="en">

<head>
<title>
  <xsl:value-of select="@table"/>: <xsl:value-of select="@title"/>
</title>

<!-- Very simple styling. -->
<style type="text/css">
.group_by
{
  margin-left: 2em;
}

.records
{
  margin-left: 2em;
}

</style>

</head>

<body>

<h1>
  <xsl:value-of select="@table"/>: <xsl:value-of select="@title"/>
</h1>


<xsl:apply-templates/>

</body>
</html>
</xsl:template>

<xsl:template match="group_by">
<div class="group_by">
<p>
<xsl:value-of select="@group_field"/>: <b><xsl:value-of select="@group_value"/></b>
</p>
<xsl:apply-templates select="group_by"/>
<p>
<table class="records">
  <xsl:apply-templates select="field_heading"/>
  <xsl:apply-templates select="row"/>
</table>
</p>
</div>
</xsl:template>

<xsl:template match="field_heading">
<th class="field_heading"> <xsl:value-of select="@title"/> </th>
</xsl:template>

<xsl:template match="row">
<tr class="row">
<xsl:apply-templates/>
</tr>
</xsl:template>

<xsl:template match="field">
<td class="field"> <xsl:value-of select="@value"/> </td>
</xsl:template>


</xsl:stylesheet>

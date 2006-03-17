<?xml version="1.0" encoding="iso-8859-1"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.w3.org/1999/xhtml" version="1.0">
<xsl:output encoding="utf8" method="xml" indent="yes" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="DTD/xhtml1-strict.dtd"/>

<xsl:template match="report_print">
<html xml:lang="en" lang="en">

<head>
<xsl:text>

</xsl:text>
<title>
  <xsl:value-of select="@table"/>: <xsl:value-of select="@title"/>
</title>
<xsl:text>

</xsl:text>

<!-- Very simple styling. -->
<style type="text/css">

table
{
  border-collapse: collapse; /* So we get border lines between the cells, instead of lines around separated cells squares */
  border-style: solid;
  border-color: black; /* Though it has a 0 width by default. */
  border-width: 0em;
}

.header
{
}

.footer
{
}

.group_by
{
  margin-left: 0em;
  border-style: solid;
  border-color: black;
  border-width: 0em;
}

.group_vertical
{
  margin-left: 0em;
  border-style: solid;
  border-color: black;
  border-width: 0em;
}

.group_by > .group_by
{
  margin-left: 2em; /* Indent all sub-groups, without indenting top-level groups. */
  border-style: solid;
  border-color: black;
  border-width: 0em;
}

.group_by_title
{
  margin-left: 0em;
  border-style: solid;
  border-color: black;
  border-width: 0.0em 0em 0.1em 0em; /* Put a line under the group-by title value. */
}

.summary
{
  margin-left: 2em;
}

.group_by_secondary_fields
{
}

.records
{
  margin-left: 0em;
}

th
{
  padding: 0.2em;
  border-style: solid;
  border-color: black; /* Though it has a 0 width by default. */
  border-width: 0em;
  vertical-align: top;
}

td
{
  padding: 0.2em;
  border-style: solid;
  border-color: black; /* Though it has a 0 width by default. */
  border-width: 0em;
  vertical-align: top;
}

.records_summary
{
  margin-left: 2em; /* Indent each summary set of records. */
}

</style>
<xsl:text>

</xsl:text>

</head>
<xsl:text>

</xsl:text>
<body>
<xsl:text>
</xsl:text>

<h1>
  <xsl:value-of select="@table"/>: <xsl:value-of select="@title"/>
</h1>
<xsl:text>
</xsl:text>

<xsl:apply-templates/>

</body>
</html>
</xsl:template>


<xsl:template match="header">
<xsl:text>
</xsl:text>
<div class="header">

<xsl:apply-templates />
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="footer">
<xsl:text>
</xsl:text>
<div class="footer">

<xsl:apply-templates />
<xsl:text>
</xsl:text>
</xsl:template>


<xsl:template match="group_by">
<div class="group_by">
<p>

<xsl:if test="string(@group_field)">
<div class="group_by_title">
<xsl:value-of select="@group_field"/>: <b><xsl:value-of select="@group_value"/></b>
</div>
<xsl:text>
</xsl:text>
</xsl:if>

<xsl:apply-templates select="secondary_fields"/>
</p>
<xsl:apply-templates select="group_by"/>
<p>

<xsl:text>
</xsl:text>

<xsl:variable name="attStyleBorderWidth">
<xsl:choose>
  <xsl:when test="string(@border_width)"><xsl:value-of select="@border_width"/></xsl:when>
  <xsl:otherwise>0</xsl:otherwise>
</xsl:choose>
</xsl:variable>

<table class="records" style="border-width: {$attStyleBorderWidth}em;">
<xsl:text>
</xsl:text>
  <xsl:apply-templates select="field_heading"/>
<xsl:text>
</xsl:text>
  <xsl:apply-templates select="row"/>
</table>
<xsl:text>

</xsl:text>

</p>
<xsl:apply-templates select="summary"/>
</div>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="secondary_fields">
<table class="group_by_secondary_fields">
<xsl:text>
</xsl:text>
  <xsl:apply-templates select="field_heading"/>
  <xsl:apply-templates select="row"/>
</table>
<xsl:text>

</xsl:text>
</xsl:template>

<xsl:template match="summary">
<div class="summary">
<p>
<table class="records_summary">
<xsl:text>
</xsl:text>
  <xsl:apply-templates select="field_heading"/>
  <xsl:apply-templates select="row"/>
</table>
<xsl:text>

</xsl:text>
</p>
</div>
</xsl:template>

<xsl:template match="ungrouped_records">
<div class="ungrouped_records">
<p>
<table class="records">
<xsl:text>
</xsl:text>
  <xsl:apply-templates select="field_heading"/>
  <xsl:apply-templates select="row"/>
</table>
<xsl:text>

</xsl:text>
</p>
</div>
</xsl:template>

<xsl:template match="group_vertical">
<xsl:variable name="attStyleBorderWidth">
<xsl:choose>
  <xsl:when test="string(../../@border_width)"><xsl:value-of select="../../@border_width"/></xsl:when>
  <xsl:otherwise>0</xsl:otherwise>
</xsl:choose>
</xsl:variable>

<xsl:text>

</xsl:text>
<td class="group_vertical" style="border-width: {$attStyleBorderWidth}em;">
<div class="group_vertical">
<table>
<xsl:apply-templates/>
</table>
</div>
</td>
<xsl:text>

</xsl:text>
</xsl:template>

<xsl:template match="field_heading">

<xsl:variable name="attAlign">
<xsl:choose>
  <xsl:when test="@field_type = 'numeric'">right</xsl:when>
  <xsl:otherwise>left</xsl:otherwise>
</xsl:choose>
</xsl:variable>

<xsl:variable name="attStyleBorderWidth">
<xsl:choose>
  <xsl:when test="string(../@border_width)"><xsl:value-of select="../@border_width"/></xsl:when>
  <xsl:otherwise>0</xsl:otherwise>
</xsl:choose>
</xsl:variable>

<th class="field_heading" align="{$attAlign}" style="border-width: {$attStyleBorderWidth}em;"> <xsl:value-of select="@title"/> </th>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="row">
<tr class="row">
<xsl:apply-templates/>
</tr>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="field">

<xsl:variable name="attAlign">
<xsl:choose>
  <xsl:when test="@field_type = 'numeric'">right</xsl:when>
  <xsl:otherwise>left</xsl:otherwise>
</xsl:choose>
</xsl:variable>

<xsl:variable name="attStyleBorderWidth">
<xsl:choose>
  <xsl:when test="string(../../@border_width)"><xsl:value-of select="../../@border_width"/></xsl:when>
  <xsl:otherwise>0</xsl:otherwise>
</xsl:choose>
</xsl:variable>

<td class="field" align="{$attAlign}" style="border-width: {$attStyleBorderWidth}em;"><xsl:value-of select="@value"/></td>
</xsl:template>


<xsl:template match="field_vertical">

<xsl:variable name="attAlign">
<xsl:choose>
  <xsl:when test="@field_type = 'numeric'">right</xsl:when>
  <xsl:otherwise>left</xsl:otherwise>
</xsl:choose>
</xsl:variable>

<xsl:variable name="attStyleBorderWidth">
<xsl:choose>
  <xsl:when test="string(../@border_width)"><xsl:value-of select="../@border_width"/></xsl:when>
  <xsl:otherwise>0</xsl:otherwise>
</xsl:choose>
</xsl:variable>

<tr>
<th class="field_heading" align="right" style="border-width: {$attStyleBorderWidth}em;"><xsl:value-of select="@title"/></th>
<td class="field" align="{$attAlign}" style="border-width: {$attStyleBorderWidth}em;"><xsl:value-of select="@value"/></td>
</tr>
<xsl:text>
</xsl:text>
</xsl:template>


</xsl:stylesheet>

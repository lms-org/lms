<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- For any node: Copy the node and process its children -->
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()" />
    </xsl:copy>
  </xsl:template>

  <!-- For <if> nodes: Remove the <if> tag and process its children -->
  <!-- TODO ignore <if> if inside module/config -->
  <xsl:template match="if">
    <xsl:apply-templates />
  </xsl:template>
</xsl:stylesheet>

<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

require_once("models/config.php");
require_once("models/header.php");


echo "
<body>
<div id='wrapper'>
<div id='top'><div id='logo'></div></div>
<div id='content'>
<h2>Imprint (German)</h2>
<div id='left-nav'>";

include("left-nav.php");

echo "
</div>
<div id='main'>
<h3>Anschrift</h3> <p> Richy Henle<br> Goethestr 8<br> 66130 Saarbrücken<br> <b>Web: </b><a href='http://82.211.60.84/'>http://82.211.60.84/</a><br> <b>E-Mail</b> henlerichy@yahoo.de<br> </p> <p> </p> <h3>Bildmaterial</h3> <p>Logo: <a href='https://www.graphicsprings.com/' target='_blank'>graphicsprings</a></p> <p> Dieses Impressum wurde erstellt mit dem Impressum-Generator vom <a href=\"//www.anwalt-suchservice.de\" target=\"_blank\">Anwalt-Suchservice</a>. </p> 
";

echo "</div>
  </form>
<div id='bottom'>";
include("bottom_nav.php");
echo "</div>
</div>
</div>
</body>
</html>";

?>
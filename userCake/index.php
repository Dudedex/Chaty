<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}
require_once("models/header.php");

echo "
<body>
<div id='wrapper'>
<div id='top'><div id='logo'></div></div>
<div id='content'>

<h2>Chaty Homepage</h2>
<div id='left-nav'>";
include("left-nav.php");

echo "
</div>
<div id='main'>
<h3>Chaty Accountmanagement Page</h3>
Manage your chaty account and your settings.
</div>
<div id='bottom'>";
include("bottom_nav.php");
echo "</div>
</div>
</body>
</html>";

?>

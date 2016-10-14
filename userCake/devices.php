<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}

//Prevent the user visiting the devices in page if he is not logged in
if(!isUserLoggedIn()) { header("Location: login.php"); die(); }

require_once("models/funcs.php");

if(!empty($_POST)) {
    $username = $_POST["username"];
    $deviceIdentifier = $_POST["deviceIdentifier"];
    deleteDevice($deviceIdentifier,$username);
}

require_once("models/header.php");


echo "
<body>
<div id='wrapper'>
<div id='top'><div id='logo'></div></div>
<div id='content'>
<h2>Devices</h2>
<div id='left-nav'>";

include("left-nav.php");

echo "
</div>
<div id='main'>
<id=\"form1\">
      <div class=\"div-table\">
             <div class=\"div-table-row\">
                <div class=\"div-table-col deviceIdentifierColumn table-header\" align=\"center\">Device identifier</div>
                <div class=\"div-table-col ipColumn table-header\" align=\"center\">IP-Address</div>
                <div class=\"div-table-col heartbeatColumn table-header\" align=\"center\">Last Contact Date</div>
                <div class=\"div-table-col tokenColumn table-header\" align=\"center\">Token Expiration Date</div>
                <div class=\"div-table-col actionColumn table-header\" align=\"center\">Actions</div>
             </div>";
$devices = fetchAllDevices($loggedInUser->username);
if(isset($devices) && is_array($devices) && !empty($devices)){
    foreach ($devices as $device) {
        if(isset($device['deviceIdentifier'])){
            echo "<div class=\"div-table-row\">
                    <div class=\"div-table-col deviceIdentifierColumn\" align=\"center\">".$device['deviceIdentifier']."</div>
                    <div class=\"div-table-col ipColumn\" align=\"center\">".$device['ipAddress']."</div>
                    <div class=\"div-table-col heartbeatColumn\" align=\"center\">".$device['lastHeartbeatDate']."</div>
                    <div class=\"div-table-col tokenColumn\" align=\"center\">".$device['tokenExpirationDate']."</div>
                    <div class=\"div-table-col actionColumn\" align=\"center\">
                    <form action=\"".$_SERVER['PHP_SELF']."\" method=\"post\">
                        <input type=\"hidden\" name=\"deviceIdentifier\" value=\"".$device['deviceIdentifier']."\" />
                        <input type=\"hidden\" name=\"username\" value=\"".$loggedInUser->username."\" />
                        <input type=\"submit\" class=\"btn-danger\" value=\"LOGOUT FROM DEVICE\"/>
                    </form></div>
                    </div>";
        }
    }
}

echo "</div>
  </form>
</div>
<div id='bottom'>";
include("bottom_nav.php");
echo "</div>
</div>
</body>
</html>";

?>

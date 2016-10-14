<?php
/*
UserCake Version: 2.0.2
http://usercake.com
*/

require_once("models/config.php");
if (!securePage($_SERVER['PHP_SELF'])){die();}

//Prevent the user visiting the logged in page if he is not logged in
if(!isUserLoggedIn()) { header("Location: login.php"); die(); }

if(!empty($_POST))
{
	$errors = array();
	$successes = array();
	$password = $_POST["password"];
	$password_new = $_POST["passwordc"];
	$password_confirm = $_POST["passwordcheck"];
	if(!empty($_POST['deleteRsaKey'])){
		$delete_rsa = $_POST['deleteRsaKey'];
	} else {
		$delete_rsa = 0;
	}
	
	$errors = array();
	$email = $_POST["email"];
    $alias = $_POST["alias"];
	
	//Perform some validation
	//Feel free to edit / change as required
	
	//Confirm the hashes match before updating a users password
	$entered_pass = generateHash($password, $loggedInUser->hash_pw);
	
	if (trim($password) == ""){
		$errors[] = lang("ACCOUNT_SPECIFY_PASSWORD");
	}
	else if($entered_pass != $loggedInUser->hash_pw)
	{
		//No match
		$errors[] = lang("ACCOUNT_PASSWORD_INVALID");
	}	
	if($email != $loggedInUser->email)
	{
		if(trim($email) == "")
		{
			$errors[] = lang("ACCOUNT_SPECIFY_EMAIL");
		}
		else if(!isValidEmail($email))
		{
			$errors[] = lang("ACCOUNT_INVALID_EMAIL");
		}
		else if(emailExists($email))
		{
			$errors[] = lang("ACCOUNT_EMAIL_IN_USE", array($email));	
		}
		
		//End data validation
		if(count($errors) == 0)
		{
			$loggedInUser->updateEmail($email);
			$successes[] = lang("ACCOUNT_EMAIL_UPDATED");
		}
	}

    if($alias != $loggedInUser->alias){
        if(count($errors) == 0) {
            $loggedInUser->updateAlias($alias);
            $successes[] = lang("ACCOUNT_ALIAS_UPDATED");
        }
    }

	if($delete_rsa == 1){
		if(count($errors) == 0) {
			$loggedInUser->updateRsaPublicKey(NULL);
			$successes[] = lang("ACCOUNT_RSA_PUBLIC_KEY_DELETED");
		}
	}
	
	if ($password_new != "" OR $password_confirm != "")
	{
		if(trim($password_new) == "")
		{
			$errors[] = lang("ACCOUNT_SPECIFY_NEW_PASSWORD");
		}
		else if(trim($password_confirm) == "")
		{
			$errors[] = lang("ACCOUNT_SPECIFY_CONFIRM_PASSWORD");
		}
		else if(minMaxRange(8,50,$password_new))
		{	
			$errors[] = lang("ACCOUNT_NEW_PASSWORD_LENGTH",array(8,50));
		}
		else if($password_new != $password_confirm)
		{
			$errors[] = lang("ACCOUNT_PASS_MISMATCH");
		}
		
		//End data validation
		if(count($errors) == 0)
		{
			//Also prevent updating if someone attempts to update with the same password
			$entered_pass_new = generateHash($password_new, $loggedInUser->hash_pw);
			
			if($entered_pass_new == $loggedInUser->hash_pw)
			{
				//Don't update, this fool is trying to update with the same password Â¬Â¬
				$errors[] = lang("ACCOUNT_PASSWORD_NOTHING_TO_UPDATE");
			}
			else
			{
				//This function will create the new hash and update the hash_pw property.
				$loggedInUser->updatePassword($password_new);
                $loggedInUser->resetTokenExpirationDateForUser($loggedInUser->username);
				$successes[] = lang("ACCOUNT_PASSWORD_UPDATED");
			}
		}
	}
	if(count($errors) == 0 AND count($successes) == 0){
		$errors[] = lang("NOTHING_TO_UPDATE");
	}
}

require_once("models/header.php");
echo "
<body>
<div id='wrapper'>
<div id='top'><div id='logo'></div></div>
<div id='content'>

<h2>User Settings</h2>
<div id='left-nav'>";
include("left-nav.php");

echo "
</div>
<div id='main'>";

echo resultBlock($errors,$successes);

echo "
<div id='regbox'>
<form name='updateAccount' action='".$_SERVER['PHP_SELF']."' method='post'>
<p>
<div class='input-label'>RSA Public Key (read only):</div>
<div class='bordered-div'>$loggedInUser->rsaPublicKey</div>
</p>
<p>
<div class='deleteRSA'><input style='margin-top:-2px;' type='checkbox' name='deleteRsaKey' value='1'/> Delete your RSA Public Key</div>
</p>
<p>
<div class='input-label'>Display Name (Alias):</div>
<input type='text' name='alias' value='".$loggedInUser->alias."' class='input-input'/>
</p>
<p>
<div class='input-label'>Email:</div>
<input type='text' name='email' value='".$loggedInUser->email."' class='input-input'/>
</p>
<p>
<div class='input-label'>New Pass:</div>
<input type='password' name='passwordc' class='input-input'/>
</p>
<p>
<div class='input-label'>Confirm Pass:</div>
<input type='password' name='passwordcheck' class='input-input'/>
</p>

<hr>
<div class=\"hint\">Type in password to save the new settings:</div>
<input type='password' name='password' class='input-input'/>


<p>
<input type='submit' value='Update' class='btn btn-primary input-label' />
</p>
</form>
</div>
</div>
<div id='bottom'>";
include("bottom_nav.php");
echo "</div>
</div>
</body>
</html>";

?>

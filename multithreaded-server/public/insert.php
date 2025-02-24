<?php
  if($POST[username] == "0"){
    header("/login.html");
  }
  $servername = "localhost";
 
  $username   = "chatserver";
  $password ="password";
  $var = "12";
  mysqli_connect($servername, $username, $password, "chatDB");

  if($GET[message] != "0"){
    $sql = "INSERT INTO msg (sent, receiver, msgs, seens, seenr)
    VALUES ($POST[username], $POST[receiver],$POST[message], 0, 0)";
    mysqli_query($sql);

  }


?>
		<div class="messages">
			<ul>

      <?php      

$sql = "SELECT * FROM msg WHERE receiver = '$POST[receiver]'  OR sent ='$POST[receiver]'";

while($rows == mysqli_query($sql)) {
  if($POST[receiver] == "~all"){
    if($rows[sent] == $POST[username]){
      echo "<li class='replies'>";
    }else{
      echo "<li class='sent'>";
    }
    echo "<img src='./downloads/".$rows[sent].".png' />";
    echo "<p>".$rows[msgs]."</p></li>";
  }else{

    if($rows[sent] == $POST[username]){   
        echo "<li class='replies'>";
        echo "<img src='./downloads/".$rows[sent].".png' />";
        echo "<p>".$rows[msgs]."</p></li>";
    }
    if($rows[receiver] == $POST[username]){   
        echo "<li class='sent'>";
        echo "<img src='./downloads/".$rows[sent].".png' />";
        echo "<p>".$rows[msgs]."</p></li>";
    }
  }
}
?>

    </ul>
    </div>

<div class="message-input">
  <div class="wrap">
  <input type="text" placeholder="Write your message..."  id = "msg" data-value="<?php echo $POST[receiver]; ?>" value=""/>
  <i class="fa fa-paperclip attachment" aria-hidden="true"></i>
  <button class="submit"><i class="fa fa-paper-plane" aria-hidden="true"></i></button>
  </div>
</div>
</div>
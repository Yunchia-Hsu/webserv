<body><?php
    $servername = "localhost";
    $username   = "chatserver";
    $password ="password";

    $sql = "CREATE DATABASE chatDB $username $password";
    mysqli_query($sql);

    mysqli_connect($servername, $username, $password, "chatDB");

    $sql = "CREATE TABLE users (username, password, logged, lastseen)"; 
    mysqli_query($sql);
    header("/signup.php");
?></body>
<?php
    
    if(!isset($_REQUEST["version"])){
        echo json_encode(array("status"=>false,"info"=>"Invalid Operation."));
        return 0;
    }
    
    $latest_ver = 1;
    $target_ver = number_format($_REQUEST["version"]);
    $package_config = json_decode(file_get_contents("package-info.json"),true);
    
    $result = array();
    
    foreach($package_config as $val)
        if(number_format($val["generatebuild"])>$target_ver)
            array_push($result,array("remote"=>$val["filepath"],"local"=>$val["local"]));
    
    echo json_encode($result);
    
?>
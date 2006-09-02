<?php
    include('gnuplot.php');

    $p = new GNUplot();

    foreach($_GET as $dat=>$dummy) {
        $data = PGData::createFromFile($dat, $dat);
        $p->plotData($data, 'points', '1:2');
    }

    $p->exe("set key left \n");
    $p->export('graph.png');
    $p->close();
?>

<img src='graph.png' \>


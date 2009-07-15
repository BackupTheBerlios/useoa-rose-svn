<?php
    function datFile($file) {
        print "<input type=\"checkbox\" name=\"$file\">";
        print "[<a href=\"$file\">dat</a>]";
        print "\n";
    }
?>

<html>
<title> Results </title>
<style>
    H2 { font-size: large; }
</style>
<body>

<center>
<h1>Results for tests on OpenAnalysis's implementation of the FIAlias algorithm</h1>
</center>


<form name="input" action="plotData.php" method="get">

<table border=1 cellpadding=5 width=100%>
    <tr>
        <td></td>
        <td align=center>SMG2000</td>
        <td align=center>IRS</td>
    </tr> <tr>
        <td align=right>Memory usage</td>
        <td align=center>
            <? datFile("memory_usage/smg2000/dat"); ?>
            [<a href="memory_usage/smg2000/log">log</a>]
            [<a href="memory_usage/smg2000/oa_build.log">oa_build.log</a>]
            [<a href="memory_usage/smg2000/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="memory_usage/smg2000/cmd.sh">cmd.sh</a>]
        </td><td align=center>
            <? datFile("memory_usage/irs/dat"); ?>
            [<a href="memory_usage/irs/log">log</a>]
            [<a href="memory_usage/irs/oa_build.log">oa_build.log</a>]
            [<a href="memory_usage/irs/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="memory_usage/irs/cmd.sh">cmd.sh</a>]
        </td>
    </tr> <tr>
        <td align=right>Time: full analysis</td>
        <td align=center>
            <? datFile("time-full_analysis/smg2000/dat"); ?>
            [<a href="time-full_analysis/smg2000/log">log</a>]
            [<a href="time-full_analysis/smg2000/oa_build.log">oa_build.log</a>]
            [<a href="time-full_analysis/smg2000/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="time-full_analysis/smg2000/cmd.sh">cmd.sh</a>] </td>
        <td align=center>
            <? datFile("time-full_analysis/irs/dat"); ?>
            [<a href="time-full_analysis/irs/log">log</a>]
            [<a href="time-full_analysis/irs/oa_build.log">oa_build.log</a>]
            [<a href="time-full_analysis/irs/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="time-full_analysis/irs/cmd.sh">cmd.sh</a>]
        </td>
    </tr> <tr>
        <td align=right>Time: no analysis</td>
        <td align=center>
            <? datFile("time-no_analysis/smg2000/dat"); ?>
            [<a href="time-no_analysis/smg2000/log">log</a>]
            [<a href="time-no_analysis/smg2000/oa_build.log">oa_build.log</a>]
            [<a href="time-no_analysis/smg2000/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="time-no_analysis/smg2000/cmd.sh">cmd.sh</a>] </td>
        <td align=center>
            <? datFile("time-no_analysis/irs/dat"); ?>
            [<a href="time-no_analysis/irs/log">log</a>]
            [<a href="time-no_analysis/irs/oa_build.log">oa_build.log</a>]
            [<a href="time-no_analysis/irs/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="time-no_analysis/irs/cmd.sh">cmd.sh</a>]
        </td>
    </tr> <tr>
        <td align=right>Time: Just FIAlias algorithm </td>
        <td align=center>
            <? datFile("time-just_fialias/smg2000/dat"); ?>
            [<a href="time-just_fialias/smg2000/log">log</a>]
            [<a href="time-just_fialias/smg2000/oa_build.log">oa_build.log</a>]
            [<a href="time-just_fialias/smg2000/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="time-just_fialias/smg2000/cmd.sh">cmd.sh</a>] </td>
        <td align=center>
            <? datFile("time-just_fialias/irs/dat"); ?>
            [<a href="time-just_fialias/irs/log">log</a>]
            [<a href="time-just_fialias/irs/oa_build.log">oa_build.log</a>]
            [<a href="time-just_fialias/irs/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="time-just_fialias/irs/cmd">cmd.sh</a>]
        </td>
    </tr> <tr>
        <td align=right>Profile: full analysis </td>
        <td align=center>
            [<a href="profile-full_analysis/smg2000/gprof.tgz">gprof.tgz</a>]
            [<a href="profile-full_analysis/smg2000/log">log</a>]
            [<a href="profile-full_analysis/smg2000/oa_build.log">oa_build.log</a>]
            [<a href="profile-full_analysis/smg2000/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="profile-full_analysis/smg2000/cmd.sh">cmd.sh</a>] </td>
        <td align=center>
            [<a href="profile-full_analysis/irs/gprof.tgz">gprof.tgz</a>]
            [<a href="profile-full_analysis/irs/log">log</a>]
            [<a href="profile-full_analysis/irs/oa_build.log">oa_build.log</a>]
            [<a href="profile-full_analysis/irs/UseOA-ROSE_build.log">USEOA_ROSE_build.log</a>]
            [<a href="profile-full_analysis/irs/cmd.sh">cmd.sh</a>] </td>
        </td>
    </tr>
</table>

<input type="submit" value="Generate graph" />

</form>

<h1>What you can view</h1>
<h2>Graph</h2>
Self explanatory.

<h2>dat</h2>
The numbers fed to GNUPlot to produce the graph.

<h2>log</h2>
The log file produced by the testing script.

<h2>oa_build.log</h2>
Build log for OpenAnalysis.  This specifies what CXXFLAGS (what level of optimization, whether profiling was turned on, etc.) OA was compiled with.

<h2>USEOA_ROSE_build.log</h2>
Like oa_build.log but applies to the UseOA-ROSE project.

<h2>cmd.sh</h2>
The command file passed to the testing script.

<h2>gprof.tgz</h2>
Archive of profiles of the run produced by gprof.

<h1>What's being measured in each graph (explanation of labels in first col)
</h1>

<h2>Memory usage</h2>
Record of the peak memory usage for OATest when its passed --oa-FIAliasAliasMap.

<h2>Time: Full Analysis</h2>
Record of the time required to run OATest when passed --oa-FIAliasAliasMap.

<h2>Time: No Analysis</h2>
Timed benchmark of how long it takes OATest to load the program but skip the
analysis.  The majority of this time (I'm guessing more than 99%) is spent
generating the AST with ROSE, so this can this can be thought of as the "how
much overhead does ROSE give us" graph.

<h2>Time: Just FIAlias algorithm </h2>
Record of the time it takes OATest to run the FIAlias algorithm on some test
case but not encode the results into an inter procedural alias map.

<h2>Profile: Full Analysis</h2>
gprof produced profiles of runs of OATest when passed --oa-FIAliasAliasMap.

<h1>How a program is being sliced and stitched</h1>
The program's are being sliced into approximate eighths.  It's approximate
because I don't want to start splitting up files. Each test case is generated
by taking some subset of the programs files and then stitching them together
(using the Unix cat command) into one giant lump o' C.

Due to oddities with include dependencies (these programs weren't made to
be compiled in one step as one file) I can't get the 8/8ths slice of SMG
to work or the 7/8ths and 8/8ths slice of IRS.

<h1>How LOC are being calculated</h1>
Lines of code is calculated by taking the source file to run some test on,
passing it through the C preprocessor (cpp) and then passing it to
<a href="http://www.dwheeler.com/sloccount/">SLOCCount</a>.

</body>
</html>

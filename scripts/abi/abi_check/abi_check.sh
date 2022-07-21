#!/bin/bash
set -x
dumpCommand=$1
partLibrary=$2
headersFile=$3
outputDumpFile=$4

complianceCheckCommand=$5
refAbiDump=$6
libName=$(basename $partLibrary)

complReportPath=$7

pluginLibrary=$8
pluginOutputDumpFile=$9
refPluginAbiDump=${10}
pluginName=$(basename $pluginLibrary)


chmod +x $dumpCommand
chmod +x $complianceCheckCommand

set -e
set -o pipefail

$dumpCommand $partLibrary -public-headers $headersFile -o $outputDumpFile 
$dumpCommand $pluginLibrary -public-headers $headersFile -o $pluginOutputDumpFile

$complianceCheckCommand -l $libName -old $refAbiDump -new $outputDumpFile -report-path $complReportPath/report_participant.html
$complianceCheckCommand -l $pluginName -old $refPluginAbiDump -new $pluginOutputDumpFile -report-path $complReportPath/report_plugin.html

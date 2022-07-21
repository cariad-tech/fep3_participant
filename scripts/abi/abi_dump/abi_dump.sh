#!/bin/bash
set -x
dumpCommand=$1
partLibrary=$2
headersFile=$3
outputDumpFile=$4
pluginLib=$5
pluginDumpFile=$6

chmod +x $dumpCommand

set -e
set -o pipefail

$dumpCommand $partLibrary -public-headers $headersFile -o $outputDumpFile
$dumpCommand $pluginLib -public-headers $headersFile -o $pluginDumpFile

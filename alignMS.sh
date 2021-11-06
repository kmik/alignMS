#!/bin/bash

curDir="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

export LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib

$curDir/bin/Release/alignMS $@

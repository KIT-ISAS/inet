#!/bin/bash

if ! [ -z "$NON_MATLAB_LD_LIBRARY_PATH" ]; then
	export LD_LIBRARY_PATH="$NON_MATLAB_LD_LIBRARY_PATH"
fi

cd "$(dirname "$0")"

python scons.py --max-drift=1 --implicit-cache --jobs=2
#--implicit-deps-unchanged


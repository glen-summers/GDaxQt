#!/bin/bash

qt=~/Qt/5.10.0
compiler=gcc_64
project=GDaxQt
kit=Desktop_Qt_5_10_0_GCC_64bit
source=${BASH_SOURCE%/*}
target=$source/../build-$project-$kit-Release
proSpec="$qt/$compiler/bin/qmake $source/$project.pro -spec linux-g++"

function build {
	$proSpec
	make qmake_all
	mkdir -p $target
	pushd $target
	make -r -w
	popd
}

function clean {
	pushd $target
	make -r -w clean
	popd
}

clear
echo "go $1..."
case "$1" in
	"")
		build
		;;
	clean)
		if [ -d "$target" ]; then
			clean
		fi
		;;
	gui)
		build
		$target/GDaxGui/GDaxGui
		;;
esac


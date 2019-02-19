#!/bin/sh

# colour assignments, see -> http://linuxtidbits.wordpress.com/2008/08/13/output-color-on-bash-scripts-advanced/
# ORANGE="\033[38;5;214m"
# GREEN="\033[38;5;46m"
# RED="\033[38;5;196m"
# COLOR_RESET="\033[39m"

black=$(tput setaf 0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
blue=$(tput setaf 4)
magenta=$(tput setaf 5)
cyan=$(tput setaf 6)
white=$(tput setaf 7)
normal=$(tput sgr0)

bold=$(tput bold)

export black=$black
export red=$red
export green=$green
export yellow=$yellow
export blue=$blue
export magenta=$magenta
export cyan=$cyan
export white=$white
export normal=$normal

prefix='oo'

cs_err() {
	printf "$red$bold$prefix $1$normal\n"
}

cs_warn() {
	printf "$yellow$bold$prefix $1$magenta\n"
}

cs_info() {
	printf "$blue$bold$prefix $1$normal\n"
}

cs_log() {
	printf "$normal$bold$prefix $1$normal\n"
}

cs_succ() {
	printf "$green$bold$prefix $1$normal\n"
}

bluenet_logo() {
	printf "$blue$bold\n"
	printf "$prefix  _|_|_|    _|                                            _|     \n"
	printf "$prefix  _|    _|  _|  _|    _|    _|_|    _|_|_|      _|_|    _|_|_|_| \n"
	printf "$prefix  _|_|_|    _|  _|    _|  _|_|_|_|  _|    _|  _|_|_|_|    _|     \n"
	printf "$prefix  _|    _|  _|  _|    _|  _|        _|    _|  _|          _|     \n"
	printf "$prefix  _|_|_|    _|    _|_|_|    _|_|_|  _|    _|    _|_|_|      _|_| \n"
	printf "$normal\n"
}

checkError() {
	result=$?
	if [ $result -ne 0 ]; then
		if [ -n "$1" ]; then
			cs_err "$1"
		fi
		exit $result
	fi
}

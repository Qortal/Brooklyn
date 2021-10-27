#!/usr/bin/env bash

DPI=${DPI:-142}
SCREEN=${SCREEN:-640x480}
PROFILE=${PROFILE:-standard}
VERBOSE=${VERBOSE:-0}
VALGRIND=${VALGRIND}
VALGRIND_LOG_FILE=${VALGRIND_LOG_FILE}
MASSIF=${MASSIF}
CALLGRIND=${CALLGRIND}
HOST_CURSOR=${HOST_CURSOR}
TEST_HOME=${TEST_HOME}

xinerama_auto_modes()
{
    local x y screen_res screen_res_x screen_res_y div_x div_y single_res single_res_x single_res_y walker_x walker_y

    div_x="${1%x*}"
    div_y="${1#*x}"


    # get your display resolution
    screen_res="$( xdpyinfo | grep dimensions | awk '{print $2}' )"
    screen_res_x="${screen_res%x*}"
    screen_res_y="${screen_res#*x}"

    # add some offset for your window-borders
    screen_res_x="$(( ${screen_res_x} - 20 ))"
    screen_res_y="$(( ${screen_res_y} - 40 ))"

    # set size for xephir
    SCREEN="${screen_res_x}x${screen_res_y}"


    # which size per screen ?
    single_res_x="$(( ${screen_res_x} / ${div_x} ))"
    single_res_y="$(( ${screen_res_y} / ${div_y} ))"
    single_res="${single_res_x}x${single_res_y}"


    walker_x=0
    walker_y=0


    for (( y=0 ; $y < ${div_y} ; y++ )) ; do
        for (( x=0 ; $x < ${div_x} ; x++ )) ; do
            # add a new horizontal xinerama screen
            xinerama_auto_args="${xinerama_auto_args} -fake-xinerama-screen ${single_res}+${walker_x}+${walker_y}"
            walker_x="$(( ${walker_x} + ${single_res_x} ))"
        done
        # previous loop finished, set horizontal back to 0
        walker_x=0
        # add a new vertical xinerama screen
        walker_y="$(( ${walker_y} + ${single_res_y} ))"
    done

    #echo "${xinerama_auto_args}"
}

show_help()
{
    cat <<HELP_EOF
Usage:

   $0 [options]

where options are:

  -c, --host-cursor        Uses host-cursor, otherwise uses Xephyr's.
  -d, --dpi=NUMBER         states the dots-per-inch to be used.
  -s, --screen=SPEC        WIDTHxHEIGHT[xDEPTH] to be used.
  -x, --fxs=WxH+X+Y        Fake xinerama screen resolution
                           (can be applied repeatedly)
  -ax 2x3                  Automatic set of fake xineramas, 2 horizontal
                           screens per 3 of vertical ones
  -p, --profile=NAME       Enlightenment profile name.
  -e, --empty-home[=PATH]  Run with \$HOME being a new, empty directory at /tmp


Debugger options:

  --valgrind=MODE      Run under valgrind to check for memory problems.
                       Same value as enlightenment_start, mode is "OR" of:
                          1 = plain valgrind to catch crashes (default)
                          2 = trace children (thumbnailer, efm slaves, ...)
                          4 = check leak
                          8 = show reachable after processes finish.
                        all = all of above
  --valgrind-log-file=FILENAME
                       Save valgrind log to file, see valgrind's
                       --log-file for details.
  --massif             Run under massif valgrind tool.
  --callgrind          Run under callgrind valgrind tool.


Misc options:
  -v, --verbose            Be verbose about what is being done.
  -h, --help               Show this help.


Useful or common options:

    --dpi=142 --screen=240x320x16
    --dpi=142 --screen=320x240x16
    --dpi=186 --screen=272x480x16
    --dpi=186 --screen=480x272x16
    --dpi=181 --screen=320x320x16
    --dpi=183 --screen=320x480x16 --host-cursor
    --dpi=183 --screen=480x320x16 --host-cursor
    --dpi=183 --screen=480x800x16
    --dpi=183 --screen=800x480x16
    --dpi=284 --screen=480x640x16
    --dpi=284 --screen=640x480x16 --host-cursor
    --dpi=183 --screen=960x400x16 --host-cursor
    --dpi=284 --screen=480x800 --host-cursor
    --dpi=284 --screen=480x640
    --dpi=284 --screen=640x480
    --dpi=181 --screen=320x320
    --dpi=186 --screen=272x480
    --dpi=142 --screen=240x320
    --dpi=142 --screen=240x240 --host-cursor

HELP_EOF
}

while [ $# -gt 0 ]; do
    arg=$1
    shift
    option=`echo "'$arg'" | cut -d'=' -f1 | tr -d "'"`
    value=`echo "'$arg'" | cut -d'=' -f2- | tr -d "'"`
    if [ x"$value" = x"$option" ]; then
        value=""
    fi

    case $option in
        -h|-help|--help)
            show_help
            exit 0
            ;;
        -v|-verbose|--verbose)
            VERBOSE=1
            ;;
        -e|-empty-home|--empty-home)
            if [ -z "$value" ]; then
                TEST_HOME="/tmp/e-test-${USER:-$UID}"
            else
                TEST_HOME="$value"
            fi
            ;;
        -d|-dpi|--dpi)
            if [ -z "$value" ]; then
                value=$1
                shift
            fi
            DPI=$value
            ;;
        -s|-screen|--screen)
            if [ -z "$value" ]; then
                value=$1
                shift
            fi
            SCREEN=$value
            ;;
        -x|-fxs|--fxs)
            if [ -z "$value" ]; then
                value=$1
                shift
            fi
            FAKE_XINERAMA_SCREEN="$FAKE_XINERAMA_SCREEN $value"
            ;;
        -p|-profile|--profile)
            if [ -z "$value" ]; then
                value=$1
                shift
            fi
            PROFILE=$value
            ;;
        -c|-host-cursor|--host-cursor)
            HOST_CURSOR="-host-cursor"
            ;;
        -valgrind|--valgrind)
            if [ -z "$value" ]; then
                value=$1
                shift
            fi
            VALGRIND=$value
            ;;
        -valgrind-log-file|--valgrind-log-file)
            if [ -z "$value" ]; then
                value=$1
                shift
            fi
            VALGRIND_LOG_FILE=$value
            ;;
        -massif|--massif)
            MASSIF="1"
            ;;
        -callgrind|--callgrind)
            CALLGRIND="1"
            ;;
        -ax|-auto-xinerama|--auto-xinerama)
            if [ -z "$value" ]; then
                value=$1
                shift
            fi
            xinerama_auto_modes $value
            ;;
        *)
            echo "Unknown option: $option" 1>&2
            ;;
    esac
done

debuggers=0

if [ ! -z "$MASSIF" ]; then
    DEBUGGER="-massif"
    ((debuggers++))
fi

if [ ! -z "$CALLGRIND" ]; then
    DEBUGGER="-callgrind"
    ((debuggers++))
fi

if [ ! -z "$VALGRIND" ]; then
    DEBUGGER="-valgrind=$VALGRIND"
    ((debuggers++))
fi

if [ $debuggers -gt 1 ]; then
    echo "WARNING: more than one debugger specified. Using $DEBUGGER" 1>&2
fi

if [ ! -z "$FAKE_XINERAMA_SCREEN" ]; then
	for fxsm in $FAKE_XINERAMA_SCREEN; do
		E_OPTIONS="$E_OPTIONS -fake-xinerama-screen $fxsm"
	done
fi

if [ ! -z "$TEST_HOME" ]; then
    if [ "${TEST_HOME#/tmp/}" != "$TEST_HOME" ]; then
        # just delete if inside /tmp
        rm -fr "$TEST_HOME/"
    fi
    mkdir -p "$TEST_HOME"
fi

unset E_RESTART E_START E_IPC_SOCKET E_START_TIME E_CONF_PROFILE E_PREFIX

if [ $VERBOSE -ne 0 ]; then
    cat <<EOF
Using:

   DPI=$DPI
   SCREEN=$SCREEN
   FAKE_XINERAMA_SCREEN=$FAKE_XINERAMA_SCREEN
   PROFILE=$PROFILE
   HOST_CURSOR=$HOST_CURSOR
   DEBUGGER=$DEBUGGER
      VALGRIND=$VALGRIND
      VALGRIND_LOG_FILE=$VALGRIND_LOG_FILE
      MASSIF=$MASSIF
      CALLGRIND=$CALLGRIND
   TEST_HOME=$TEST_HOME
EOF
    set -x
fi

Xephyr :1 -nolisten tcp -noreset -ac -br -dpi $DPI -screen $SCREEN $HOST_CURSOR &

sleep 1
export DISPLAY=:1
[ ! -z "$TEST_HOME" ] && export HOME="$TEST_HOME"
enlightenment_start \
    -i-really-know-what-i-am-doing-and-accept-full-responsibility-for-it \
    -profile $PROFILE \
	$E_OPTIONS $xinerama_auto_args \
    $DEBUGGER

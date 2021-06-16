#!/bin/sh

set -ex

DEQP_WIDTH=${DEQP_WIDTH:-256}
DEQP_HEIGHT=${DEQP_HEIGHT:-256}
DEQP_CONFIG=${DEQP_CONFIG:-rgba8888d24s8ms0}
DEQP_VARIANT=${DEQP_VARIANT:-master}

DEQP_OPTIONS="$DEQP_OPTIONS --deqp-surface-width=$DEQP_WIDTH --deqp-surface-height=$DEQP_HEIGHT"
DEQP_OPTIONS="$DEQP_OPTIONS --deqp-surface-type=${DEQP_SURFACE_TYPE:-pbuffer}"
DEQP_OPTIONS="$DEQP_OPTIONS --deqp-gl-config-name=$DEQP_CONFIG"
DEQP_OPTIONS="$DEQP_OPTIONS --deqp-visibility=hidden"

if [ -z "$DEQP_VER" ]; then
   echo 'DEQP_VER must be set to something like "gles2", "gles31-khr" or "vk" for the test run'
   exit 1
fi

if [ "$DEQP_VER" = "vk" ]; then
   if [ -z "$VK_DRIVER" ]; then
      echo 'VK_DRIVER must be to something like "radeon" or "intel" for the test run'
      exit 1
   fi
fi

if [ -z "$GPU_VERSION" ]; then
   echo 'GPU_VERSION must be set to something like "llvmpipe" or "freedreno-a630" (the name used in .gitlab-ci/deqp-gpu-version-*.txt)'
   exit 1
fi

INSTALL=`pwd`/install

# Set up the driver environment.
export LD_LIBRARY_PATH=`pwd`/install/lib/
export EGL_PLATFORM=surfaceless
export VK_ICD_FILENAMES=`pwd`/install/share/vulkan/icd.d/"$VK_DRIVER"_icd.${VK_CPU:-`uname -m`}.json

# the runner was failing to look for libkms in /usr/local/lib for some reason
# I never figured out.
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

RESULTS=`pwd`/${DEQP_RESULTS_DIR:-results}
mkdir -p $RESULTS

HANG_DETECTION_CMD=""

# Generate test case list file.
if [ "$DEQP_VER" = "vk" ]; then
   cp /deqp/mustpass/vk-$DEQP_VARIANT.txt /tmp/case-list.txt
   DEQP=/deqp/external/vulkancts/modules/vulkan/deqp-vk
   HANG_DETECTION_CMD="/parallel-deqp-runner/build/bin/hang-detection"
elif [ "$DEQP_VER" = "gles2" -o "$DEQP_VER" = "gles3" -o "$DEQP_VER" = "gles31" -o "$DEQP_VER" = "egl" ]; then
   cp /deqp/mustpass/$DEQP_VER-$DEQP_VARIANT.txt /tmp/case-list.txt
   DEQP=/deqp/modules/$DEQP_VER/deqp-$DEQP_VER
   SUITE=dEQP
elif [ "$DEQP_VER" = "gles2-khr" -o "$DEQP_VER" = "gles3-khr" -o "$DEQP_VER" = "gles31-khr" -o "$DEQP_VER" = "gles32-khr" ]; then
   cp /deqp/mustpass/$DEQP_VER-$DEQP_VARIANT.txt /tmp/case-list.txt
   DEQP=/deqp/external/openglcts/modules/glcts
   SUITE=dEQP
else
   cp /deqp/mustpass/$DEQP_VER-$DEQP_VARIANT.txt /tmp/case-list.txt
   DEQP=/deqp/external/openglcts/modules/glcts
   SUITE=KHR
fi

# If the caselist is too long to run in a reasonable amount of time, let the job
# specify what fraction (1/n) of the caselist we should run.  Note: N~M is a gnu
# sed extension to match every nth line (first line is #1).
if [ -n "$DEQP_FRACTION" ]; then
   sed -ni 1~$DEQP_FRACTION"p" /tmp/case-list.txt
fi

# If the job is parallel at the gitab job level, take the corresponding fraction
# of the caselist.
if [ -n "$CI_NODE_INDEX" ]; then
   sed -ni $CI_NODE_INDEX~$CI_NODE_TOTAL"p" /tmp/case-list.txt
fi

if [ -n "$DEQP_CASELIST_FILTER" ]; then
    sed -ni "/$DEQP_CASELIST_FILTER/p" /tmp/case-list.txt
fi

if [ -n "$DEQP_CASELIST_INV_FILTER" ]; then
    sed -ni "/$DEQP_CASELIST_INV_FILTER/!p" /tmp/case-list.txt
fi

if [ ! -s /tmp/case-list.txt ]; then
    echo "Caselist generation failed"
    exit 1
fi

if [ -e "$INSTALL/deqp-$GPU_VERSION-fails.txt" ]; then
    DEQP_RUNNER_OPTIONS="$DEQP_RUNNER_OPTIONS --baseline $INSTALL/deqp-$GPU_VERSION-fails.txt"
fi

# Default to an empty known flakes file if it doesn't exist.
touch $INSTALL/deqp-$GPU_VERSION-flakes.txt

if [ -e "$INSTALL/deqp-$GPU_VERSION-skips.txt" ]; then
    DEQP_SKIPS="$INSTALL/deqp-$GPU_VERSION-skips.txt"
fi

set +e

if [ -n "$DEQP_PARALLEL" ]; then
   JOB="--jobs $DEQP_PARALLEL"
elif [ -n "$FDO_CI_CONCURRENT" ]; then
   JOB="--jobs $FDO_CI_CONCURRENT"
else
   JOB="--jobs 4"
fi

# If this CI lab lacks artifacts support, print the whole list of failures/flakes.
if [ -n "$DEQP_NO_SAVE_RESULTS" ]; then
   SUMMARY_LIMIT="--summary-limit 0"
fi

run_cts() {
    deqp=$1
    caselist=$2
    output=$3
    deqp-runner \
        run \
        --deqp $deqp \
        --output $RESULTS \
        --caselist $caselist \
        --skips $INSTALL/deqp-all-skips.txt $DEQP_SKIPS \
        --flakes $INSTALL/deqp-$GPU_VERSION-flakes.txt \
        --testlog-to-xml  /deqp/executor/testlog-to-xml \
        $JOB \
        $SUMMARY_LIMIT \
	$DEQP_RUNNER_OPTIONS \
        -- \
        $DEQP_OPTIONS
}

parse_renderer() {
    RENDERER=`grep -A1 TestCaseResult.\*info.renderer $RESULTS/deqp-info.qpa | grep '<Text' | sed 's|.*<Text>||g' | sed 's|</Text>||g'`
    VERSION=`grep -A1 TestCaseResult.\*info.version $RESULTS/deqp-info.qpa | grep '<Text' | sed 's|.*<Text>||g' | sed 's|</Text>||g'`
    echo "Renderer: $RENDERER"
    echo "Version: $VERSION "

    if ! echo $RENDERER | grep -q $DEQP_EXPECTED_RENDERER; then
        echo "Expected GL_RENDERER $DEQP_EXPECTED_RENDERER"
        exit 1
    fi
}

check_renderer() {
    if echo $DEQP_VER | grep -q egl; then
        return
    fi
    echo "Capturing renderer info for GLES driver sanity checks"
    # If you're having trouble loading your driver, uncommenting this may help
    # debug.
    # export EGL_LOG_LEVEL=debug
    VERSION=`echo $DEQP_VER | cut -d '-' -f1 | tr '[a-z]' '[A-Z]'`
    export LD_PRELOAD=$TEST_LD_PRELOAD
    $DEQP $DEQP_OPTIONS --deqp-case=$SUITE-$VERSION.info.\* --deqp-log-filename=$RESULTS/deqp-info.qpa
    export LD_PRELOAD=
    parse_renderer
}

check_vk_device_name() {
    echo "Capturing device info for VK driver sanity checks"
    export LD_PRELOAD=$TEST_LD_PRELOAD
    $DEQP $DEQP_OPTIONS --deqp-case=dEQP-VK.info.device --deqp-log-filename=$RESULTS/deqp-info.qpa
    export LD_PRELOAD=
    DEVICENAME=`grep deviceName $RESULTS/deqp-info.qpa | sed 's|deviceName: ||g'`
    echo "deviceName: $DEVICENAME"
    if ! echo $DEVICENAME | grep -q "$DEQP_EXPECTED_RENDERER"; then
        echo "Expected deviceName $DEQP_EXPECTED_RENDERER"
        exit 1
    fi
}

report_load() {
    echo "System load: $(cut -d' ' -f1-3 < /proc/loadavg)"
    echo "# of CPU cores: $(cat /proc/cpuinfo | grep processor | wc -l)"
}

# wrapper to supress +x to avoid spamming the log
quiet() {
    set +x
    "$@"
    set -x
}

if [ "$GALLIUM_DRIVER" = "virpipe" ]; then
    # deqp is to use virpipe, and virgl_test_server llvmpipe
    export GALLIUM_DRIVER="$GALLIUM_DRIVER"

    VTEST_ARGS="--use-egl-surfaceless"
    if [ "$VIRGL_HOST_API" = "GLES" ]; then
        VTEST_ARGS="$VTEST_ARGS --use-gles"
    fi

    GALLIUM_DRIVER=llvmpipe \
    GALLIVM_PERF="nopt,no_filter_hacks" \
    virgl_test_server $VTEST_ARGS >$RESULTS/vtest-log.txt 2>&1 &

    sleep 1
fi

if [ $DEQP_VER = vk ]; then
    quiet check_vk_device_name
else
    quiet check_renderer
fi

RESULTS_CSV=$RESULTS/results.csv
FAILURES_CSV=$RESULTS/failures.csv

export LD_PRELOAD=$TEST_LD_PRELOAD

run_cts $DEQP /tmp/case-list.txt $RESULTS_CSV
DEQP_EXITCODE=$?

export LD_PRELOAD=
quiet report_load

# Remove all but the first 50 individual XML files uploaded as artifacts, to
# save fd.o space when you break everything.
find $RESULTS -name \*.xml | \
    sort -n |
    sed -n '1,+49!p' | \
    xargs rm -f

# If any QPA XMLs are there, then include the XSL/CSS in our artifacts.
find $RESULTS -name \*.xml \
    -exec cp /deqp/testlog.css /deqp/testlog.xsl "$RESULTS/" ";" \
    -quit

$HANG_DETECTION_CMD deqp-runner junit \
   --testsuite $DEQP_VER \
   --results $RESULTS/failures.csv \
   --output $RESULTS/junit.xml \
   --limit 50 \
   --template "See https://$CI_PROJECT_ROOT_NAMESPACE.pages.freedesktop.org/-/$CI_PROJECT_NAME/-/jobs/$CI_JOB_ID/artifacts/results/{{testcase}}.xml"

# Report the flakes to the IRC channel for monitoring (if configured):
if [ -n "$FLAKES_CHANNEL" ]; then
  python3 $INSTALL/report-flakes.py \
         --host irc.oftc.net \
         --port 6667 \
         --results $RESULTS_CSV \
         --known-flakes $INSTALL/deqp-$GPU_VERSION-flakes.txt \
         --channel "$FLAKES_CHANNEL" \
         --runner "$CI_RUNNER_DESCRIPTION" \
         --job "$CI_JOB_ID" \
         --url "$CI_JOB_URL" \
         --branch "${CI_MERGE_REQUEST_SOURCE_BRANCH_NAME:-$CI_COMMIT_BRANCH}" \
         --branch-title "${CI_MERGE_REQUEST_TITLE:-$CI_COMMIT_TITLE}"
fi

exit $DEQP_EXITCODE

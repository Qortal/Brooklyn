#!/bin/sh

set -ex

if [ -z "$GPU_VERSION" ]; then
   echo 'GPU_VERSION must be set to something like "llvmpipe" or "freedreno-a630" (the name used in your ci/piglit-gpu-version-*.txt)'
   exit 1
fi

INSTALL=`pwd`/install

# Set up the driver environment.
export LD_LIBRARY_PATH=`pwd`/install/lib/
export EGL_PLATFORM=surfaceless
export VK_ICD_FILENAMES=`pwd`/install/share/vulkan/icd.d/"$VK_DRIVER"_icd.${VK_CPU:-`uname -m`}.json

RESULTS=`pwd`/${PIGLIT_RESULTS_DIR:-results}
mkdir -p $RESULTS

if [ -n "$PIGLIT_FRACTION" -o -n "$CI_NODE_INDEX" ]; then
   FRACTION=`expr ${PIGLIT_FRACTION:-1} \* ${CI_NODE_TOTAL:-1}`
PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --fraction $FRACTION"
fi

# If the job is parallel at the gitab job level, take the corresponding fraction
# of the caselist.
if [ -n "$CI_NODE_INDEX" ]; then
   PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --fraction-start ${CI_NODE_INDEX}"
fi

if [ -e "$INSTALL/piglit-$GPU_VERSION-fails.txt" ]; then
    PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --baseline $INSTALL/piglit-$GPU_VERSION-fails.txt"
fi

# Default to an empty known flakes file if it doesn't exist.
touch $INSTALL/piglit-$GPU_VERSION-flakes.txt

if [ -e "$INSTALL/piglit-$GPU_VERSION-skips.txt" ]; then
    PIGLIT_SKIPS="$INSTALL/piglit-$GPU_VERSION-skips.txt"
fi

set +e

if [ -n "$PIGLIT_PARALLEL" ]; then
   PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --jobs $PIGLIT_PARALLEL"
elif [ -n "$FDO_CI_CONCURRENT" ]; then
   PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --jobs $FDO_CI_CONCURRENT"
else
   PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --jobs 4"
fi

RESULTS_CSV=$RESULTS/results.csv
FAILURES_CSV=$RESULTS/failures.csv

export LD_PRELOAD=$TEST_LD_PRELOAD

    piglit-runner \
        run \
        --piglit-folder /piglit \
        --output $RESULTS \
        --skips $INSTALL/piglit/piglit-all-skips.txt $PIGLIT_SKIPS \
        --flakes $INSTALL/piglit-$GPU_VERSION-flakes.txt \
        --profile $PIGLIT_PROFILES \
        --process-isolation \
	$PIGLIT_RUNNER_OPTIONS \
        -v -v

PIGLIT_EXITCODE=$?

export LD_PRELOAD=

deqp-runner junit \
   --testsuite $PIGLIT_PROFILES \
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
         --known-flakes $INSTALL/piglit-$GPU_VERSION-flakes.txt \
         --channel "$FLAKES_CHANNEL" \
         --runner "$CI_RUNNER_DESCRIPTION" \
         --job "$CI_JOB_ID" \
         --url "$CI_JOB_URL" \
         --branch "${CI_MERGE_REQUEST_SOURCE_BRANCH_NAME:-$CI_COMMIT_BRANCH}" \
         --branch-title "${CI_MERGE_REQUEST_TITLE:-$CI_COMMIT_TITLE}"
fi

exit $PIGLIT_EXITCODE

#!/bin/bash

# This script is used to run the 'perftest' app with every permutation of command
# line options available.  It dumps the results in a CSV format to stdout, which
# can be easily redirected to a file of your choice.
#
# Any command lines arguments passed to this script will be passed along to 'perftest'.
#
# FIXME: This currently only supports OpenGL. Using D3D9 should be an option
#        that gets toggled under Windows.

# Default number of frames to run the sample for. This could be made configurable
# if it seems to be worth it...?
numFrames=1000

# Loop starts at 1000 draw calls per frame, and goes up to this number with
# increments of 3000.
maxDrawCallsPerFrame=7000

ARGS=$@

# Actually run the performance tests with the current set of options

function runPerftest {
    curFps="`./perftest -use_glsl=$useGLSL -reset_constants=$resetConstants -use_vao=$useVAO -reset_vertex_pointers=$resetVertexPointers -num_frames=$numFrames -num_draw_calls_per_frame=$numDrawCalls $ARGS`"
    echo "$useGLSL,$resetConstants,$useVAO,$resetVertexPointers,$numFrames,$numDrawCalls,${curFps#*: }"
}

# If more options are added later, increase this number.
numOptions=4
lastOption=$(($numOptions - 1))

# Number of binary permutations is pow(2, numOptions)
numPermutations=$((2 ** $numOptions))

# Initalize array of binary options
declare -a ARRAY
for ((i=0;i<$numOptions;i++)); do
    ARRAY[$i]=0
done

# Helper function to generate an array of all of the binary permutations of the
# number of options we will pass to the program.
function flipBit {

    # Always flip the last "bit"
    if [ ${ARRAY[$lastOption]} -eq 0 ]; then
        ARRAY[$lastOption]=1
    else
        ARRAY[$lastOption]=0
    fi

    # Only flip the next "bit" if the last one flips to zero
    for ((j=$lastOption;j>0;j--)); do
        if [ ${ARRAY[$j]} -eq 0 ]; then
            if [ ${ARRAY[$j-1]} -eq 0 ]; then
                ARRAY[$j-1]=1
            else
                ARRAY[$j-1]=0
            fi
        else
            break
        fi
    done
}

# If more command line options are added later, place them in at the end of
# this list, as well as increase the 'numOptions' value above.
function toggleOptions {
    useGLSL=${ARRAY[0]}
    resetConstants=${ARRAY[1]}
    useVAO=${ARRAY[2]}
    resetVertexPointers=${ARRAY[3]}
}

# First line of CSV file contains column titles
echo "useGLSL,resetConstants,useVAO,resetVertexPointers,numFrames,numDrawCalls,FPS"

# Main perftest loop.
# Run every permutation, and increase the number of draw calls per frame by 3000 each run.
for ((numDrawCalls=1000;numDrawCalls<=$maxDrawCallsPerFrame;numDrawCalls+=3000)); do
    for ((i=0;i<numPermutations;i++)); do
        # Set the current command line options
        toggleOptions

        # Run the perftest app
        runPerftest

        # Set up the next permutation of binary options
        flipBit
    done
done


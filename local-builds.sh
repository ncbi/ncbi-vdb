#!/bin/bash

# consider the entire matrix to have a single timestamp
NOW=$(date "+%Y-%m-%d.%H.%M.%S")

# for most builds, we use a separate host to determine architecture
# and these will have a single item in the list called "default"
ARCH_LIST=$(make architectures)
if [ "$ARCH_LIST" = "default" ]
then
    CUR_ARCH=$(make architecture)
else
    CUR_ARCH=all-arch
fi

run ()
{
    echo "MAKE COMMANDS: make $*"

    for COMPILER in $(make compilers)
    do
        make $COMPILER 1> /dev/null

        for LINKAGE in dynamic static
        do

            make $LINKAGE 1> /dev/null

            for ARCHITECTURE in $ARCH_LIST 
            do

                if [ "$ARCHITECTURE" = "default" ]
                then
                    ARCHITECTURE=$CUR_ARCH
                else
                    make $ARCHITECTURE 1> /dev/null
                fi

                for OPTIMIZATION in debug release profile
                do

                    make $OPTIMIZATION 1> /dev/null

                    TARGET_DIR=$(make targdir)

                    if make $* 1> "$TARGET_DIR/$NOW-build.log" 2> "$TARGET_DIR/$NOW-build.err"
                    then
                        STATUS="COMPLETED"
                    else
                        STATUS=" FAILED  "
                    fi

                    printf "%-32s [ %s ]\n" $COMPILER.$LINKAGE.$ARCHITECTURE.$OPTIMIZATION "$STATUS"

                done

            done

        done

    done
}

# going to put a log of this activity into OS output directory
OS_DIR=$(make osdir)
[ -d "$OS_DIR" ] || mkdir -p "$OS_DIR"

# log based upon architecture to avoid competitions
LOG_FILE="$OS_DIR/$NOW-build-$CUR_ARCH.log"

# run the matrix of builds
run $* 2>&1 | tee "$LOG_FILE"

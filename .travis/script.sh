#!/bin/bash
#
# Copyright (c) 2019 The Eccoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

export TRAVIS_COMMIT_LOG=`git log --format=fuller -1`
if [ -n "$USE_SHELL" ]; then export CONFIG_SHELL="$USE_SHELL"; fi
OUTDIR=$BASE_OUTDIR/$TRAVIS_PULL_REQUEST/$TRAVIS_JOB_NUMBER-$HOST
BITCOIN_CONFIG_ALL="--disable-dependency-tracking --prefix=$TRAVIS_BUILD_DIR/depends/$HOST --bindir=$OUTDIR/bin --libdir=$OUTDIR/lib";
if [ "$USE_CLANG" = "false" ]; then depends/$HOST/native/bin/ccache --max-size=$CCACHE_SIZE; fi
test -n "$USE_SHELL" && eval '"$USE_SHELL" -c "./autogen.sh 2>&1 > autogen.out"' || ./autogen.sh 2>&1 > autogen.out || ( cat autogen.out && false)
mkdir build && cd build
echo "BITCOIN_CONFIG_ALL=$BITCOIN_CONFIG_ALL"
echo "BITCOIN_CONFIG=$BITCOIN_CONFIG"
echo "GOAL=$GOAL"
../configure $BITCOIN_CONFIG_ALL $BITCOIN_CONFIG 2>&1 > configure.out || ( cat configure.out && cat config.log && false)
if [ "$USE_CLANG" = "true" ]; then
    cat configure.out;
    head config.log;
fi
if [ "$RUN_FORMATTING_CHECK" = "true" ]; then make $MAKEJOBS check-formatting VERBOSE=1; fi
if [ "$USE_CLANG" = "false" ]; then
    stdbuf -i0 -o0 -e0 make $MAKEJOBS $GOAL 2>&1 | ../contrib/devtools/buildsilence.py || ( echo "Build failure. Verbose build follows." && make $GOAL V=1 ; false ) ;
else
    CXXFLAGS="-std=c++11 -Werror" make $MAKEJOBS $GOAL;
fi
if [ "USE_CLANG" = "false" ]; then export LD_LIBRARY_PATH=$TRAVIS_BUILD_DIR/depends/$HOST/lib; fi
if [ "$RUN_TESTS" = "true" ] && { [ "$HOST" = "i686-w64-mingw32" ] || [ "$HOST" = "x86_64-w64-mingw32" ]; }; then travis_wait make $MAKEJOBS check VERBOSE=1; fi
if [ "$RUN_TESTS" = "true" ] && ! { [ "$HOST" = "i686-w64-mingw32" ] || [ "$HOST" = "x86_64-w64-mingw32" ]; }; then make $MAKEJOBS check VERBOSE=1; fi
if [ "$RUN_TESTS" = "true" ]; then qa/pull-tester/rpc-tests.py --coverage --no-ipv6-rpc-listen; fi

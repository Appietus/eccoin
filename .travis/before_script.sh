#!/bin/bash
#
# Copyright (c) 2018 The BitcoinUnlimited developers
# Copyright (c) 2019 The Eccoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

unset CC; unset CXX
if [ "$CHECK_DOC" = 1 ]; then DOCKER_EXEC contrib/devtools/check-doc.py; fi
mkdir -p depends/SDKs depends/sdk-sources
if [ -n "$OSX_SDK" -a ! -f depends/sdk-sources/MacOSX${OSX_SDK}.sdk.tar.gz ]; then curl --location --fail $SDK_URL/MacOSX${OSX_SDK}.sdk.tar.gz -o depends/sdk-sources/MacOSX${OSX_SDK}.sdk.tar.gz; fi
if [ -n "$OSX_SDK" -a -f depends/sdk-sources/MacOSX${OSX_SDK}.sdk.tar.gz ]; then tar -C depends/SDKs -xf depends/sdk-sources/MacOSX${OSX_SDK}.sdk.tar.gz; fi
if [[ $HOST = *-mingw32 ]]; then DOCKER_EXEC update-alternatives --set $HOST-g++ \$\(which $HOST-g++-posix\); fi
if [ -z "$NODEPENDS" ]; then
  DOCKER_EXEC CONFIG_SHELL= make $MAKEJOBS -C depends HOST=$HOST $DEP_OPTS;
fi

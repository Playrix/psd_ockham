#!/usr/bin/env bash

BUILD_DIR="build/mac"
WORK_DIR="$PWD"

mkdir -p $BUILD_DIR
pushd $BUILD_DIR

cmake -G "Xcode" $WORK_DIR

popd
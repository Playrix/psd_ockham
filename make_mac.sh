build_path=mac
source_path=..

mkdir -p $build_path
pushd $build_path

cmake -G "Xcode" $source_path

popd
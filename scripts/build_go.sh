#!/bin/bash

rm -rf utils/server_scanner
pushd ../utils/server_scanner
go build -o ../../build/utils/server_scanner main.go
popd
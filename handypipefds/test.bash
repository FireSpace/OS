#!/usr/bin/env bash
exec 3<>in1
exec 4<>in2
exec 5<>out
echo 123 >in1
echo 456 >in2
./handypipefds 3 5 4 5 0 5

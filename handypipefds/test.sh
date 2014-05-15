#!/usr/bin/env bash

./test $@ | xargs ./handypipefds

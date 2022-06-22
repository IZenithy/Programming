#!/usr/bin/env bash

killall Xephyr

Xephyr -br -ac -reset -screen 1366x768 :1 &

sleep 1s
export DISPLAY=:1
./Build/app &

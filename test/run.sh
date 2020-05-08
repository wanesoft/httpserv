#!/bin/bash

for i in {1..1000}
do
	curl 127.0.0.1 &
	wget 127.0.0.1 &
	wget -r 127.0.0.1
done
#!/bin/bash

if [ "$ROLE" == "client" ]; then
    go build -C example/client/ . #produce a client executable
    go build -C example/tcp/ . #produce a tcp executable
else
    go build -C example . #produce example executable
fi

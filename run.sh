#!/bin/bash

# this script runs an experiment
# specify the scenario using the env var SCENARIO
# specify clients configuration choosing one of the following:
#   - TCP=1: just a single TCP client
#   - QUIC=1: just a single QUIC client
#   - TCP=2: 2 TCP client runs concurrently
#   - QUIC=2: 2 QUIC client runs concurrently
#   - TCP=1 QUIC=1: TCP and QUIC runs concurrently

#SCENARIO="drop-rate --delay=15ms --bandwidth=2Mbps --queue=25 --rate_to_client=2 --rate_to_server=2" \
docker compose build;
SCENARIO="rebind --delay=15ms --bandwidth=1Mbps --queue=25 --first-rebind=10s --rebind-freq=10s --rebind-addr" \
QUIC=1 \
docker-compose up

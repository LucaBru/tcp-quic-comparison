#!/bin/bash

# Set up the routing needed for the simulation.
/setup.sh

if [ "$ROLE" == "client" ]; then
    /wait-for-it.sh sim:57832 -s -t 30 &&
    echo "simulator is running 🚀"
    /wait-for-it.sh 193.167.100.100:4433 -s -t 30 &&
    echo "server is running 🚀"

    echo "TCP clients:" $TCP ", QUIC clients:" $QUIC
    read -r scenario_name tail <<< "$SCENARIO"
    FILENAME=$scenario_name"-TCP"$TCP"-QUIC"$QUIC".pcap"
    tcpdump -i eth0 -U -w "/logs/$FILENAME" &
    echo "dump are saved in logs/client/$FILENAME"

    launch_quic_client="client/quic/quic -insecure -keylog /logs/session_key https://193.167.100.100:4433/4000000"
    launch_tcp_client="client/tcp/tcp -insecure -keylog /logs/session_key https://193.167.100.100:4433/4000000"
    if [[ "$TCP" == "1" && -z "$QUIC" ]]; then
        ./run_clients.sh "$launch_tcp_client"
    elif [[ "$QUIC" == "1" && -z "$TCP" ]]; then
        ./run_clients.sh "$launch_quic_client"
    elif [[ "$TCP" == "2" && -z "$QUIC" ]]; then
        ./run_clients.sh "$launch_tcp_client" "client/tcp/tcp -concurrent -insecure -keylog /logs/session_key https://193.167.100.100:4433/4000000"
    elif [[ "$QUIC" == "2" && -z "$TCP" ]]; then
        ./run_clients.sh "$launch_quic_client" "client/quic/quic -concurrent -insecure -keylog /logs/session_key https://193.167.100.100:4433/4000000"
    elif [[ "$TCP" == "1" && "$QUIC" == "1" ]]; then
        ./run_clients.sh "$launch_quic_client" "$launch_tcp_client"
    else
        echo "set TCP and QUIC environment vars."
    fi
else
    ./server -tcp -bind 0.0.0.0:4433
fi

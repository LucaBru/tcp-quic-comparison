# TCP and QUIC: comparison across wireless-like scenarios

This project compares TCP and QUIC downloading 4MB of randomly generated data.  
QUIC implementation used: [quic-go](https://github.com/quic-go/quic-go.git)  
Network simulator: [ns-3](https://www.nsnam.org/)  
[Network simulation setup](https://github.com/quic-interop/quic-network-simulator.git)
[Report](report.pdf)

# Run experiments

To run the experiments, just clone the project and run [run.sh](run.sh) setting the configurations you prefer.  
For further analysis each run produce a .pcap file in _logs/client/_.
In the same directory a python script to generate graphs is available. 
Enjoy!

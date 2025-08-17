#include <iomanip>

#include "../helper/quic-packet.h"
#include "drop-rate-error-model.h"

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(DropRateErrorModel);

TypeId DropRateErrorModel::GetTypeId(void)
{
    static TypeId tid = TypeId("DropRateErrorModel")
                            .SetParent<ErrorModel>()
                            .AddConstructor<DropRateErrorModel>();
    return tid;
}

DropRateErrorModel::DropRateErrorModel()
    : rate(0), distr(0, 99), burst(INT_MAX), tcp_dropped(0), tcp_forwarded(0), udp_dropped(0), udp_forwarded(0), packets(0)
{
    std::random_device rd;
    rng = new std::mt19937(rd());
}

void DropRateErrorModel::DoReset(void) {}

bool DropRateErrorModel::DoCorrupt(Ptr<Packet> p)
{
    packets++;
    bool udp = IsUDPPacket(p);
    if (udp)
    {
        udp_forwarded++;
    }
    else
    {
        tcp_forwarded++;
    }

    if (distr(*rng) < rate)
    {
        if (udp)
        {
            udp_dropped++;
        }
        else
        {
            tcp_dropped++;
        }
        return true;
    }
    if (packets % 100 == 0)
    {
        cout << "tcp: " << fixed << setprecision(1) << (double)tcp_dropped / (tcp_dropped + tcp_forwarded) * 100 << " %";
        cout << "udp: " << fixed << setprecision(1) << (double)udp_dropped / (udp_dropped + udp_forwarded) * 100 << " %";
    }
    return false;
}

void DropRateErrorModel::SetDropRate(int rate_in)
{
    rate = rate_in;
}

void DropRateErrorModel::SetMaxDropBurst(int burst_in)
{
    burst = burst_in;
}

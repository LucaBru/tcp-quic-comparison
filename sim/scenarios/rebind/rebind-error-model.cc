#include "rebind-error-model.h"
#include "ns3/core-module.h"
#include "ns3/tcp-header.h"
#include <cassert>

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(RebindErrorModel);

bool IsTCPPacket(Ptr<Packet> p)
{
  PppHeader ppp_hdr;
  p->RemoveHeader(ppp_hdr);
  bool is_tcp = false;

  switch (ppp_hdr.GetProtocol())
  {
  case 0x21: // IPv4
  {
    Ipv4Header hdr;
    p->PeekHeader(hdr);
    is_tcp = hdr.GetProtocol() == 6; // TCP = 6
    break;
  }
  default:
    std::cout << "Unknown PPP protocol: " << ppp_hdr.GetProtocol() << std::endl;
    break;
  }

  p->AddHeader(ppp_hdr);
  return is_tcp;
}

class TcpPacket
{
  Ptr<Packet> p;
  PppHeader pppH;
  Ipv4Header ipH;
  TcpHeader tcpH;
  uint32_t tcpHdrLen;
  uint32_t totalHdrLen;
  vector<uint8_t> tcpPayload;

public:
  TcpPacket(Ptr<Packet> pkt)
      : p(pkt)
  {
    const uint32_t size = p->GetSize();
    uint8_t *buffer = new uint8_t[size];
    p->CopyData(buffer, size);
    pppH = PppHeader();
    uint32_t pppHdrLen = p->RemoveHeader(pppH);
    // TODO: This currently only works for IPv4.
    ipH = Ipv4Header();
    uint32_t ipHdrLen = p->RemoveHeader(ipH);
    tcpH = TcpHeader();
    tcpHdrLen = p->RemoveHeader(tcpH);
    totalHdrLen = pppHdrLen + ipHdrLen + tcpHdrLen;
    tcpPayload =
        vector<uint8_t>(&buffer[totalHdrLen], &buffer[totalHdrLen] + size - totalHdrLen);
  }

  void ReassemblePacket()
  {
    // Start with the TCP payload.
    Packet newPacket = Packet(tcpPayload.data(), tcpPayload.size());

    // Add the TCP header and recalculate the checksum.
    // tcpH.SetPayloadSize(tcpPayload.size()); // Optional, if needed for checksum calculation
    tcpH.EnableChecksums();
    tcpH.InitializeChecksum(ipH.GetSource(), ipH.GetDestination(), ipH.GetProtocol());
    newPacket.AddHeader(tcpH);

    // Add the IPv4 header and enable checksum calculation.
    ipH.EnableChecksum();
    newPacket.AddHeader(ipH);

    // Add the PPP header.
    newPacket.AddHeader(pppH);

    // Replace contents of the original packet with the reassembled one.
    p->RemoveAtEnd(p->GetSize());
    p->AddAtEnd(Ptr<Packet>(&newPacket));
  }

  Ipv4Header &GetIpv4Header()
  {
    return ipH;
  }
};

TypeId RebindErrorModel::GetTypeId(void)
{
  static TypeId tid = TypeId("RebindErrorModel")
                          .SetParent<ErrorModel>()
                          .AddConstructor<RebindErrorModel>();
  return tid;
}

RebindErrorModel::RebindErrorModel()
    : client("193.167.0.100"), server("193.167.100.100"), nat(client),
      rebind_addr(false)
{
  rng = CreateObject<UniformRandomVariable>();
}

void RebindErrorModel::DoReset() {}

void RebindErrorModel::SetRebindAddr(bool ra) { rebind_addr = ra; }

void RebindErrorModel::DoRebind()
{
  const Ipv4Address old_nat = nat;
  do
  {
    nat.Set((old_nat.Get() & 0xffffff00) | rng->GetInteger(1, 0xfe));
  } while (nat == old_nat || nat == client);
  cout << "client ip changed changed from " << old_nat << " to " << nat << endl;
}

bool RebindErrorModel::DoCorruptUdp(Ptr<Packet> p)
{
  QuicPacket qp = QuicPacket(p);

  const Ipv4Address &src_ip_in = qp.GetIpv4Header().GetSource();
  const Ipv4Address &dst_ip_in = qp.GetIpv4Header().GetDestination();
  if (src_ip_in == client)
  {
    qp.GetIpv4Header().SetSource(nat);
  }
  else if (src_ip_in == server)
  {
    qp.GetIpv4Header().SetDestination(client);
  }
  else
  {
    return true;
  }
  qp.ReassemblePacket();
  return false;
}

bool RebindErrorModel::DoCorruptTcp(Ptr<Packet> p)
{
  TcpPacket qp = TcpPacket(p);
  const Ipv4Address &src_ip_in = qp.GetIpv4Header().GetSource();
  const Ipv4Address &dst_ip_in = qp.GetIpv4Header().GetDestination();
  if (src_ip_in == client)
  {
    qp.GetIpv4Header().SetSource(nat);
  }
  else if (src_ip_in == server)
  {
    qp.GetIpv4Header().SetDestination(client);
  }
  else
  {
    return true;
  }

  qp.ReassemblePacket();
  return false;
}

bool RebindErrorModel::DoCorrupt(Ptr<Packet> p)
{
  if (IsUDPPacket(p))
  {
    return DoCorruptUdp(p);
  }
  else if (IsTCPPacket(p))
  {
    return DoCorruptTcp(p);
  }
  else
  {
    return false;
  }
}

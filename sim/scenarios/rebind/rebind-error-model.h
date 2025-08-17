#ifndef REBIND_ERROR_MODEL_H
#define REBIND_ERROR_MODEL_H

#include "../helper/quic-packet.h"
#include "ns3/error-model.h"
#include "ns3/random-variable-stream.h"
#include <unordered_map>

using namespace ns3;

class RebindErrorModel : public ErrorModel
{
public:
  static TypeId GetTypeId(void);
  RebindErrorModel();
  void SetDrop(int packet_num);
  Ptr<UniformRandomVariable> rng;
  void DoRebind();
  void SetRebindAddr(bool ra);

private:
  bool DoCorrupt(Ptr<Packet> p);
  bool DoCorruptUdp(Ptr<Packet> p);
  bool DoCorruptTcp(Ptr<Packet> p);
  void DoReset(void);
  Ipv4Address client, server, nat;
  bool rebind_addr;
};

#endif /* REBIND_ERROR_MODEL_H */
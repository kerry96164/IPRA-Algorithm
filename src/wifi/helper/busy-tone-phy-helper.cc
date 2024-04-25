#include "busy-tone-phy-helper.h"


namespace ns3 {

//NS_LOG_COMPONENT_DEFINE ("BusytonePhyHelper");

BusytonePhyHelper::BusytonePhyHelper ()
  : m_channel (0)
{
  m_phy.SetTypeId ("ns3::BusytonePhy");
}

BusytonePhyHelper::~BusytonePhyHelper ()
{
}

BusytonePhyHelper
BusytonePhyHelper::Default (void)
{
  BusytonePhyHelper helper;
  //helper.SetErrorRateModel ("ns3::NistErrorRateModel");check
  return helper;
}

void
BusytonePhyHelper::SetChannel (Ptr<BusytoneChannel> channel)
{
  m_channel = channel;
}

void
BusytonePhyHelper::SetChannel (std::string channelName)
{
  //Ptr<BusytoneChannel> channel = Names::Find<BusytoneChannel> (channelName);
  //m_channel = channel;
}

Ptr<BusytonePhy>
BusytonePhyHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
{
  Ptr<BusytonePhy> phy = m_phy.Create<BusytonePhy> ();
  //Ptr<ErrorRateModel> error = m_errorRateModel.Create<ErrorRateModel> ();
  //phy->SetErrorRateModel (error);check
  phy->SetChannel (m_channel);
  phy->SetDevice (device);
  return phy;
}

void
BusytonePhyHelper::Set (std::string name, const AttributeValue &v)
{
  m_phy.Set (name, v);
}

/*void/////check
BusytonePhyHelper::SetErrorRateModel (std::string name,
                                  std::string n0, const AttributeValue &v0,
                                  std::string n1, const AttributeValue &v1,
                                  std::string n2, const AttributeValue &v2,
                                  std::string n3, const AttributeValue &v3,
                                  std::string n4, const AttributeValue &v4,
                                  std::string n5, const AttributeValue &v5,
                                  std::string n6, const AttributeValue &v6,
                                  std::string n7, const AttributeValue &v7)
{
  m_errorRateModel = ObjectFactory ();
  m_errorRateModel.SetTypeId (name);
  m_errorRateModel.Set (n0, v0);
  m_errorRateModel.Set (n1, v1);
  m_errorRateModel.Set (n2, v2);
  m_errorRateModel.Set (n3, v3);
  m_errorRateModel.Set (n4, v4);
  m_errorRateModel.Set (n5, v5);
  m_errorRateModel.Set (n6, v6);
  m_errorRateModel.Set (n7, v7);
}*/


} //namespace ns3
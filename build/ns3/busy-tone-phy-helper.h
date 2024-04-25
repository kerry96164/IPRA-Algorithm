#ifndef BUSY_TONE_PHY_HELPER_H
#define BUSY_TONE_PHY_HELPER_H

//#include "ns3/trace-helper.h"
#include "ns3/busy-tone-phy.h"
#include "wifi-mac-helper.h"
#include "ns3/busy-tone-channel.h"

namespace ns3 {

//class BusytonePhy;
class WifiNetDevice;
class Node;

/**
 * \brief create PHY objects
 *
 * This base class must be implemented by new PHY implementation which wish to integrate
 * with the \ref ns3::WifiHelper class.
 */
class BusytonePhyHelper 
{
public:
  BusytonePhyHelper ();
  virtual ~BusytonePhyHelper ();
  static BusytonePhyHelper Default (void);
  void SetChannel (Ptr<BusytoneChannel> channel);
  void SetChannel (std::string channelName);
  /**
   * \param node the node on which the PHY object will reside
   * \param device the device within which the PHY object will reside
   *
   * \returns a new PHY object.
   *
   * Subclasses must implement this method to allow the ns3::WifiHelper class
   * to create PHY objects from ns3::WifiHelper::Install.
   *
   * Typically the device type will be of class WifiNetDevice but the
   * type of the pointer is generalized so that this method may be used
   * by other Wifi device variants such as WaveNetDevice.
   */
  virtual Ptr<BusytonePhy> Create (Ptr<Node> node, Ptr<NetDevice> device) const ;

  /**
   * \param name the name of the attribute to set
   * \param v the value of the attribute
   *
   * Set an attribute of the underlying PHY object.
   */
  void Set (std::string name, const AttributeValue &v);
  /**
   * \param name the name of the error rate model to set.
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * Set the error rate model and its attributes to use when Install is called.
   */
  /*void SetErrorRateModel (std::string name,/////check
                          std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                          std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                          std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                          std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                          std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                          std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                          std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                          std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());
  */
  ObjectFactory m_phy; ///< PHY object
  //ObjectFactory m_errorRateModel; ///< error rate model///////check
  Ptr<BusytoneChannel> m_channel; ///< yans wifi channel
};

} //namespace ns3

#endif /* BUSY_TONE_PHY_HELPER_H */
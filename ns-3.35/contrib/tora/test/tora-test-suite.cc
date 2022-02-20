/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Include a header file from your module to test.
#include "ns3/tora.h"

// An essential include is test.h
#include "ns3/test.h"

#include "ns3/tora-packet.h"

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;
using namespace tora;

// This is an example TestCase.
class ToraTestCase1 : public TestCase
{
public:
  ToraTestCase1 ();
  virtual ~ToraTestCase1 ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
ToraTestCase1::ToraTestCase1 ()
  : TestCase ("Tora test case (does nothing)")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
ToraTestCase1::~ToraTestCase1 ()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
ToraTestCase1::DoRun (void)
{
  // A wide variety of test macros are available in src/core/test.h
  NS_TEST_ASSERT_MSG_EQ (true, true, "true doesn't equal true for some reason");
  // Use this one for floating point comparisons
  NS_TEST_ASSERT_MSG_EQ_TOL (0.01, 0.01, 0.001, "Numbers are not equal within tolerance");
}


struct QryHeaderTest : public TestCase
{
  QryHeaderTest ():TestCase("TORA QRY")
  {

  }
  virtual void DoRun (void)
  {
    QryHeader h(Ipv4Address("10.20.30.40"));
    NS_TEST_ASSERT_MSG_EQ(h.GetDst(), Ipv4Address("10.20.30.40"), "trivial");

    h.SetDst(Ipv4Address("255.0.12.17"));
    NS_TEST_ASSERT_MSG_EQ(h.GetDst(), Ipv4Address("255.0.12.17"), "trivial");

    Ptr<Packet>p = Create<Packet>();
    p->AddHeader(h);
    QryHeader h2;
    uint32_t bytes = p->RemoveHeader(h2);
    NS_TEST_ASSERT_MSG_EQ (bytes, 4, "Qry is 4 byts long");
    NS_TEST_ASSERT_MSG_EQ (h2.GetDst(), Ipv4Address("255.0.12.17") ,"Round trip serialization works"); 

  }
};


// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class ToraTestSuite : public TestSuite
{
public:
  ToraTestSuite ();
};

ToraTestSuite::ToraTestSuite ()
  : TestSuite ("tora", UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new ToraTestCase1, TestCase::QUICK);
  AddTestCase (new QryHeaderTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static ToraTestSuite storaTestSuite;


// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WObject.h>
#include <Wt/WSignal.h>

#include <iostream>
#include <functional>

BOOST_AUTO_TEST_CASE( test_signals1 )
{
  Wt::Signal<> signal;

  bool success = false;

  signal.connect(std::bind([&](){
    success = true;
  }));

  signal();

  BOOST_REQUIRE(success);
}

BOOST_AUTO_TEST_CASE( test_signals2 )
{
  Wt::Signals::connection connection;

  {
    Wt::Signal<> signal;
    connection = signal.connect([](){
      std::cout << "What this does does not matter" << std::endl;
    });
  }

  Wt::Signal<> signal2;
  signal2.connect([](){
    std::cout << "What this does also does not matter" << std::endl;
  });

  // signal2 will normally take the memory location of signal
  // If the connection ring is incorrectly implemented, this could
  // cause the check below to fail. In that case, valgrind will also
  // detect an invalid read.

  BOOST_REQUIRE(!connection.isConnected());

  connection.disconnect();

  BOOST_REQUIRE(!connection.isConnected());
}

BOOST_AUTO_TEST_CASE( test_signals3 )
{
  Wt::Signal<> signal;

  bool executed = false;

  auto connection = signal.connect([&](){
    executed = true;
  });

  Wt::Signals::connection connectionCopy = connection;

  BOOST_REQUIRE(connection.isConnected());
  BOOST_REQUIRE(connectionCopy.isConnected());
  BOOST_REQUIRE(signal.isConnected());

  signal();

  BOOST_REQUIRE(executed);

  connectionCopy.disconnect();

  BOOST_REQUIRE(!connection.isConnected());
  BOOST_REQUIRE(!connectionCopy.isConnected());
  BOOST_REQUIRE(!signal.isConnected());

  executed = false;

  signal();

  BOOST_REQUIRE(!executed);
}

BOOST_AUTO_TEST_CASE( test_signals4 )
{
  Wt::Signal<> signal;
  Wt::Signals::connection connection;

  connection = signal.connect([&](){
    connection.disconnect();
  });

  signal();

  BOOST_REQUIRE(!signal.isConnected());
  BOOST_REQUIRE(!connection.isConnected());
}

class DeleteMe : public Wt::WObject
{
public:
  Wt::Signal<> signal;
  void doDelete() {
    delete this;
  }
};

BOOST_AUTO_TEST_CASE( test_signals5 )
{
  auto dm = new DeleteMe();

  bool before = false;
  bool after = false;

  Wt::Signal<> signal;
  signal.connect([&](){
    before = true;
  });
  auto connection = signal.connect(dm, &DeleteMe::doDelete);
  signal.connect([&](){
    after = true;
  });

  signal();

  BOOST_REQUIRE(signal.isConnected());
  BOOST_REQUIRE(before);
  BOOST_REQUIRE(after);
  BOOST_REQUIRE(!connection.isConnected());

  before = false;
  after = false;

  signal();

  BOOST_REQUIRE(before);
  BOOST_REQUIRE(after);
}

BOOST_AUTO_TEST_CASE( test_signals6 )
{
  auto dm = new DeleteMe();

  bool before = false;
  bool after = false;

  auto conn1 = dm->signal.connect([&](){
    before = true;
  });
  auto conn2 = dm->signal.connect([dm](){
    delete dm;
  });
  auto conn3 = dm->signal.connect([&](){
    after = true;
  });

  dm->signal();

  BOOST_REQUIRE(before);
  BOOST_REQUIRE(after);
  BOOST_REQUIRE(!conn1.isConnected());
  BOOST_REQUIRE(!conn2.isConnected());
  BOOST_REQUIRE(!conn3.isConnected());
}

BOOST_AUTO_TEST_CASE( test_signals7 )
{
  auto dm = new DeleteMe();

  bool before = false;
  bool after = false;

  auto conn1 = dm->signal.connect([&](){
    before = true;
  });
  auto conn2 = dm->signal.connect(dm, &DeleteMe::doDelete);
  auto conn3 = dm->signal.connect([&](){
    after = true;
  });

  dm->signal();

  BOOST_REQUIRE(before);
  BOOST_REQUIRE(after);
  BOOST_REQUIRE(!conn1.isConnected());
  BOOST_REQUIRE(!conn2.isConnected());
  BOOST_REQUIRE(!conn3.isConnected());
}

BOOST_AUTO_TEST_CASE( test_signals8 )
{
  int before = 0;
  int after = 0;
  int i = 0;

  auto dm = new DeleteMe();

  dm->signal.connect([&](){
    ++before;
  });
  dm->signal.connect([&,dm](){
    ++i;
    if (i == 1) {
      dm->signal();
    } else if (i == 2) {
      delete dm;
    }
  });
  dm->signal.connect([&](){
    ++after;
  });

  dm->signal();

  BOOST_REQUIRE(before == 2);
  BOOST_REQUIRE(after == 2);
  BOOST_REQUIRE(i == 2);
}

// This test should not leak
BOOST_AUTO_TEST_CASE( test_signals9 )
{
  Wt::Signal<> signal;

  signal.connect([](){
    throw 4;
  });

  try {
    signal();
  } catch (int i) {
    BOOST_REQUIRE(i == 4);
  }
}
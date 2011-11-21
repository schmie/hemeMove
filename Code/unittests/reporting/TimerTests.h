#ifndef HEMELB_UNITTESTS_REPORTING_TIMERTESTS_H
#define HEMELB_UNITTESTS_REPORTING_TIMERTESTS_H

#include <cppunit/TestFixture.h>
#include "reporting/Timers.h"
#include "reporting/Timers.hpp"

namespace hemelb
{
  namespace unittests
  {
    namespace reporting
    {
      class ClockMock
      {
        public:
          ClockMock() :
              fake_time(0)
          {
          }
          ;
        protected:
          double CurrentTime()
          {
            fake_time += 10.0;
            return fake_time;
          }
        private:
          double fake_time;
      };

      class MPICommsMock
      {
        public:
          MPICommsMock() :
              calls(1)
          {
          }
          ;
        protected:
          int Reduce(double *sendbuf,
                     double *recvbuf,
                     int count,
                     MPI_Datatype datatype,
                     MPI_Op op,
                     int root,
                     MPI_Comm comm)
          {
            CPPUNIT_ASSERT_EQUAL(10, count);
            for (int i = 0; i < count; i++)
            {
              CPPUNIT_ASSERT_EQUAL(10.0 * i, sendbuf[i]);
              recvbuf[i] = 5.0 * i * calls;
            }
            calls++;
            return 0;
          }
        private:
          unsigned int calls;
      };

      using namespace hemelb::reporting;
      class TimerTests : public CppUnit::TestFixture
      {
          CPPUNIT_TEST_SUITE(TimerTests);
          CPPUNIT_TEST(TestInitialization);
          CPPUNIT_TEST(TestStartStop);
          CPPUNIT_TEST(TestSetTime);
          CPPUNIT_TEST(TestMultipleStartStop);CPPUNIT_TEST_SUITE_END();
        public:
          void setUp()
          {
            timer = new TimerBase<ClockMock>();
          }

          void tearDown()
          {
            delete timer;
          }

          void TestInitialization()
          {
            // should default to zero time on startup
            CPPUNIT_ASSERT_EQUAL(0.0, timer->Get());
          }
          void TestStartStop()
          {
            timer->Start(); // clock mock at 10.0
            timer->Stop(); // clock mock at 20.0
            CPPUNIT_ASSERT_EQUAL(10.0, timer->Get());
          }
          void TestMultipleStartStop()
          {
            timer->Start(); // clock mock at 10.0
            timer->Stop(); // clock mock at 20.0
            timer->Start(); // clock mock at 30.0
            timer->Stop(); // clock mock at 40.0
            CPPUNIT_ASSERT_EQUAL(20.0, timer->Get());
          }
          void TestSetTime()
          {
            timer->Set(15.0);
            CPPUNIT_ASSERT_EQUAL(15.0, timer->Get());
            timer->Start(); // clock mock at 10.0
            timer->Stop(); // clock mock at 20.0
            CPPUNIT_ASSERT_EQUAL(25.0, timer->Get());
          }

        private:
          TimerBase<ClockMock> *timer;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(TimerTests);

      class TimersTests : public CppUnit::TestFixture
      {
          CPPUNIT_TEST_SUITE(TimersTests);
          CPPUNIT_TEST(TestInitialization);
          CPPUNIT_TEST(TestTimersSeparate);
          CPPUNIT_TEST(TestReduce);CPPUNIT_TEST_SUITE_END();
        public:
          void setUp()
          {
            timers = new TimersBase<ClockMock, MPICommsMock>();
          }

          void tearDown()
          {
            delete timers;
          }

          void TestInitialization()
          {
            CPPUNIT_ASSERT_EQUAL( (*timers)[Timers::total].Get(), 0.0);
            for (unsigned int i = 0; i < Timers::numberOfTimers; i++)
            {
              CPPUNIT_ASSERT_EQUAL( (*timers)[i].Get(), 0.0);
            }
          }

          void TestTimersSeparate()
          {
            for (unsigned int i = 0; i < Timers::numberOfTimers; i++)
            {
              for (unsigned int j = 0; j < i; j++)
              {
                (*timers)[i].Start();
                (*timers)[i].Stop();
              }
            }
            for (unsigned int i = 0; i < Timers::numberOfTimers; i++)
            {
              CPPUNIT_ASSERT_EQUAL( (*timers)[i].Get(), 10.0 * i);
            }
          }

          void TestReduce()
          {
            for (unsigned int i = 0; i < Timers::numberOfTimers; i++)
            {
              for (unsigned int j = 0; j < i; j++)
              {
                (*timers)[i].Start();
                (*timers)[i].Stop();
              }
            }
            timers->Reduce(); // trigger the mock
            for (unsigned int i = 0; i < Timers::numberOfTimers; i++)
            {
              CPPUNIT_ASSERT_EQUAL(timers->Maxes()[i], i * 5.0);
              CPPUNIT_ASSERT_EQUAL(timers->Means()[i], i * 10.0);
              CPPUNIT_ASSERT_EQUAL(timers->Mins()[i], i * 15.0);
            }
          }

        private:
          TimersBase<ClockMock, MPICommsMock> *timers;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(TimersTests);
    }
  }
}

#endif /* HEMELB_UNITTESTS_REPORTING_TIMERTESTS_H_ */

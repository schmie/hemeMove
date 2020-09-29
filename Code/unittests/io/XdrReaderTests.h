// -*- mode: C++ -*-
// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_UNITTESTS_IO_XDRREADERTESTS_H
#define HEMELB_UNITTESTS_IO_XDRREADERTESTS_H

#include <cppunit/TestFixture.h>
#include <type_traits>

#include "io/writers/xdr/XdrReader.h"

// This header is generated by xdr_gen.py
#include "unittests/io/xdr_test_data.h"

namespace hemelb
{
  namespace unittests
  {
    namespace io
    {      
      class XdrReaderTests : public CppUnit::TestFixture
      {
	CPPUNIT_TEST_SUITE(XdrReaderTests);
	CPPUNIT_TEST(TestInt32);
	CPPUNIT_TEST(TestUInt32);
	CPPUNIT_TEST(TestInt64);
	CPPUNIT_TEST(TestUInt64);

	CPPUNIT_TEST(TestFloat);
	CPPUNIT_TEST(TestDouble);

	CPPUNIT_TEST(TestString);
	CPPUNIT_TEST_SUITE_END();

	using buf_t = std::vector<char>;

	template<typename T>
	void TestBasic(const std::vector<T>& expected_values, const std::vector<char>& buffer) {
	  // Figure out sizes and counts
	  constexpr auto sz = sizeof(T);
	  static_assert(sz > 0U && sz <= 8U,
			"Only works on 8--64 bit types");
	  constexpr auto type_bits = sz*8U;
	  // XDR works on 32 bit words
	  constexpr auto coded_words = (sz - 1U)/4U + 1U;
	  constexpr auto coded_bytes = coded_words * 4U;
	  // Need a value for each bit being on + zero
	  constexpr auto n_vals = type_bits + 1U;
	  // Buffers for the encoded data
	  constexpr auto buf_size = n_vals * coded_bytes;
	  CPPUNIT_ASSERT_EQUAL(expected_values.size(), n_vals);
	  CPPUNIT_ASSERT_EQUAL(buffer.size(), buf_size);
	  // Make the decoder under test
	  auto our_coder = hemelb::io::writers::xdr::XdrMemReader(buffer.data(), buffer.size());

	  // Decode values and check they are equal
	  for (auto i = 0U; i < n_vals; ++i) {
	    T our_val;
	    our_coder.read(our_val);
	    CPPUNIT_ASSERT_EQUAL(expected_values[i], our_val);
	  }
	}

	template<typename INT>
	void TestInt() {
	  static_assert(std::is_integral<INT>::value, "only works on (u)ints");
	  auto&& values = test_data<INT>::unpacked();
	  auto&& buffer = test_data<INT>::packed();

	  TestBasic<INT>(values, buffer);
	}

	template<typename FLOAT>
	void TestFloating() {
	  static_assert(std::is_floating_point<FLOAT>::value, "Floats only please!");

	  auto&& values = test_data<FLOAT>::unpacked();
	  auto&& buffer = test_data<FLOAT>::packed();
	  
	  TestBasic(values, buffer);
	}

      public:
	void TestInt32() {
	  TestInt<int32_t>();
	}
	void TestUInt32() {
	  TestInt<uint32_t>();
	}
	void TestInt64() {
	  TestInt<int64_t>();
	}
	void TestUInt64() {
	  TestInt<uint64_t>();
	}

	void TestFloat() {
	  TestFloating<float>();
	}
	void TestDouble() {
	  TestFloating<double>();
	}

	void TestString() {
	  using UPC = std::unique_ptr<char[]>;
	  auto make_ones = [](size_t n) -> UPC {
	    // Fill with binary ones to trigger failure if we're not
	    // writing enough zeros
	    auto ans = UPC(new char[n]);
	    std::fill(ans.get(), ans.get() + n, ~'\0');
	    return ans;
	  };

	  // XDR strings are serialised as length, data (0-padded to a word)
	  auto coded_length = [](const std::string& s) {
	    return s.size() ? 4U * (2U + (s.size() - 1U) / 4U) : 4U;
	  };

	  auto&& values = test_data<std::string>::unpacked();
	  auto&& buffer = test_data<std::string>::packed();

	  std::size_t total_length = 0;
	  std::for_each(
	    values.begin(), values.end(),
	    [&](const std::string& v) {
	      total_length += coded_length(v);
	    });
	  CPPUNIT_ASSERT_EQUAL(buffer.size(), total_length);

	  auto our_coder = hemelb::io::writers::xdr::XdrMemReader(buffer.data(), buffer.size());

	  // Decode values and check they are equal
	  for (auto& expected_val: values) {
	    std::string our_val;
	    our_coder.read(our_val);
	    CPPUNIT_ASSERT_EQUAL(expected_val, our_val);
	  }

	}

      private:
      };
      CPPUNIT_TEST_SUITE_REGISTRATION(XdrReaderTests);
    }
  }
}

#endif

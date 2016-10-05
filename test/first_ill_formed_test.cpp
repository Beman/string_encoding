﻿//  unicode/test/first_ill_formed_test.cpp  --------------------------------------------//

//  © Copyright Beman Dawes 2016

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <iostream>
using std::cout;
using std::endl;
#include <boost/unicode/string_encoding.hpp>
#include <boost/unicode/detail/hex_string.hpp>
#include <boost/endian/conversion.hpp>
#include <string>
#include <cstring>
#include <iterator>
#define BOOST_LIGHTWEIGHT_TEST_OSTREAM std::cout
#include <boost/assert.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>

using namespace boost::unicode;
using boost::unicode::detail::hex_string;
using std::string;
using std::wstring;
using std::u16string;
using std::u32string;

namespace
{

  //  table3
  //
  //  Based on ISO/IEC 10646:2014 9.2 Table 3, Well-formed UTF-8 Octet sequences
  //
  //  The entries represent the first octet values 0xC2-0xF5
  //  Format bits:  llhh00cc
  //    ll is the offset above 0x80 for the second octet's lowest valid value.
  //    hh is the offset below 0xBF for the second octet's highest valid value.
  //    cc is the number of continuation octets (0x01-0x03).

  constexpr const unsigned char table3[] = {
    // one continuation octet
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, // 0xC2 - 0xCF
    1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, // 0xD0 - 0xDF
    // two continuation octets
    0x82u,   // 0xE0  A0-BF
    0x02u,   // 0xE1
    0x02u,   // 0xE2
    0x02u,   // 0xE3
    0x02u,   // 0xE4
    0x02u,   // 0xE5
    0x02u,   // 0xE6
    0x02u,   // 0xE7
    0x02u,   // 0xE8
    0x02u,   // 0xE9
    0x02u,   // 0xEA
    0x02u,   // 0xEB
    0x02u,   // 0xEC
    0x22u,   // 0xED  80-9F
    0x02u,   // 0xEE
    0x02u,   // 0xEF
    // three continuation octets
    0x43u,   // 0xF0  90-BF
    0x03u,   // 0xF1
    0x03u,   // 0xF2
    0x03u,   // 0xF3
    0x03u,   // 0xF4
   };

  unsigned lowest_trace;    // debugging aids
  unsigned highest_trace;

  inline
  std::pair<const char*, const char*>
    alt_first_ill_formed(const char* first, const char* last) BOOST_NOEXCEPT
  {
    const char* first_code_unit;

    for (; first != last;) // each code point
    {
      //  Loop invariants:
      //    first points to the next unprocessed code_unit
      //    first_code_unit points to the first code unit for the code point
      //    octet contains the value of the current code unit

      first_code_unit = first;
      bool error = false;
      unsigned octet = static_cast<unsigned char>(*first++);
      
      if (octet <= 0x7Fu)
        continue;  // 7-bit ASCII so nothing further to do

      //  The sequence 'a', 0xE0, 'b' must treat 0xE0 as having a missing continuation
      //  octet (i.e. error range [1, 2)) rather than treating 'b' as an invalid
      //  continuation octet (i.e. error range [1, 3)).
      //
      //  In terms of code, this means that paths through "else if" block below must
      //  increment first for each valid octet, but not for an invalid continuation octet. 

      else if (octet >= 0xC2u && octet <= 0xF4u)  // first octet is valid
      {
        // unpack the table entries
        unsigned continuations = table3[octet - 0xC2u];
        unsigned lowest = 0x80u + ((continuations & 0xC0) >> 2);
        unsigned highest = 0xBFu - (continuations & 0x30u);
        continuations &= 0x03u;

        lowest_trace = lowest;      // debugging aid
        highest_trace = highest;

        // validate the continuation octets
        for (;;)
        {
          if (first == last
            || (octet = static_cast<unsigned char>(*first)) < lowest
            || octet > highest)  // octet  is invalid
          {
            error = true;
            break;
          }
          ++first;
          if (--continuations == 0)
            break;
          // third and fourth octets, if present, require value in range 0x80u-0xBFu 
          lowest = 0x80u;
          highest = 0xBFu;
        }  // each continuation octet

      }  // first octet is valid and any continuation octets have been processed
      else  // first octet is invalid (and first has already been incremented)
        error = true;

      if (error)
      {
        //  First is pointing to last, the first octet after an invalid first octet,
        //  or to the first invalid continuation octet. Bypass octets not last and not a
        //  valid initial octet so that these invalid octets are included in the error
        //  range.

        for (;  //  bypass octets not last and not a valid initial octet
             first != last  // not last
               && (((octet = static_cast<unsigned char>(*first)) >= 0x80u
                 && octet <= 0xC1u)
                 || (octet >= 0xF5u && octet <= 0xFEu)); // not valid initial octet
             ++first) {}
        return std::make_pair(first_code_unit, first);
      }
    } // each code point

    BOOST_ASSERT(first == last);
    return std::make_pair(last, last);  // success
  }

/*
  ISO/IEC 10646:2014 9.4 says "Table 3 lists all the ranges (inclusive) of the octet
  sequences that are well-formed in UTF-8. Any UTF-8 sequence that does not match the
  patterns listed in table 3 is ill-formed." and also says "As a consequence of the
  well-formedness conditions specified in table 9.2 [sic], the following octet values
  are disallowed in UTF-8: C0-C1, F5-FE."
  
  pedantic_first_ill_formed() is pedantic implmentation of those rules.
*/

  inline
  std::pair<const char*, const char*>
    pedantic_first_ill_formed(const char* first, const char* last) BOOST_NOEXCEPT
  {
    const char* first_code_unit;

    for (; first != last;) // each code point
    {
      //  Loop invariants:
      //    first points to the next unprocessed code_unit
      //    first_code_unit points to the first code unit for the code point
      //    octet contains the value of the current code unit

      first_code_unit = first;
      bool error = false;
      unsigned octet = static_cast<unsigned char>(*first++);
      
      if (octet <= 0x7Fu)
        continue;  // 7-bit ASCII so nothing further to do

      //  The sequence 'a', 0xE0, 'b' must treat 0xE0 as having a missing continuation
      //  octet (i.e. error range [1, 2)) rather than treating 'b' as an invalid
      //  continuation octet (i.e. error range [1, 3)).
      //
      //  In terms of code, this means that paths through "else if" blocks below must
      //  increment first for each valid octet, but not for an invalid continuation octet. 

      else if (octet >= 0xC2u && octet <= 0xDFu)  // two octets required
      {
        if (first == last
            || (octet = static_cast<unsigned char>(*first)) < 0x80u
            || octet > 0xBFu)  // octet two is invalid
          error = true;
        else
          ++first;
      }
      else if (octet == 0xE0u)  // three octets case one
      {
        if (first == last
            || (octet = static_cast<unsigned char>(*first)) < 0xA0u
            || octet > 0xBFu)   // octet two is invalid
          error = true;
        else
        {
          if (++first == last
              || (octet = static_cast<unsigned char>(*first)) < 0x80u
              || octet > 0xBFu)  // octet three is invalid
            error = true;
          else
           ++first;
        }
      }
      else if ((octet >= 0xE1u && octet <= 0xECu)
        || (octet >= 0xEEu && octet <= 0xEFu))  // three octets case two
      {
        if (first == last  // octet two is invalid
            || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0xBFu)
          error = true;
        else
        {
          if (++first == last  // octet three is invalid
              || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0xBFu)
            error = true;
          else
            ++first;
        }
      }
      else if (octet == 0xEDu)  // three octets case three
      {
        if (first == last
            || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0x9Fu)
          error = true;  // octet two is invalid
        else
        {
          if (++first == last
              || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0xBFu)
            error = true;  // octet three is invalid
          else
            ++first;
        }
      }
      else if (octet == 0xF0u)  // four octets case one
      {
        if (first == last
          || (octet = static_cast<unsigned char>(*first)) < 0x90u || octet > 0xBFu)
          error = true;  // octet two is invalid
        else
        {
          if (++first == last
            || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0xBFu)
            error = true;  // octet three is invalid
          else
          {
            if (++first == last
              || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0xBFu)
              error = true;  // octet four is invalid
            else
              ++first;
          }
        }
      }
      else if (octet >= 0xF1u && octet <= 0xF4u)  // four octets case two
      {
        if (first == last  // octet two is invalid
            || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0xBFu)
          error = true;  // octet two is invalid
        else
        {
          if (++first == last
            || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0xBFu)
            error = true;  // octet three is invalid
          else
          {
            if (++first == last
              || (octet = static_cast<unsigned char>(*first)) < 0x80u || octet > 0xBFu)
              error = true;  // octet four is invalid
            else
              ++first;
          }
        }
      }
      else
        error = true;  // first octet is invalid (and first has already been incremented)

      if (error)
      {
        //  First is pointing to last, the first octet after an invalid first octet,
        //  or to the first invalid continuation octet. Bypass octets not last and not a
        //  valid initial octet so that these invalid octets are included in the error
        //  range.
        
        for (;  //  bypass octets not last and not a valid initial octet
             first != last  // not last
               && (((octet = static_cast<unsigned char>(*first)) >= 0x80u
                 && octet <= 0xC1u)
                 || (octet >= 0xF5u && octet <= 0xFEu)); // not valid initial octet
            ++first) {}
        return std::make_pair(first_code_unit, first);
      }
    } // each code point

    BOOST_ASSERT(first == last);
    return std::make_pair(last, last);                 // success
  }

  void utf8_test()
  {
    cout << "start utf8_test" << std::hex << endl;

    uint32_t i = 0u;
    do
    {
      if (i % 0x01000000u == 0)
        cout << (i>>24) << endl;  // report progress
      auto pedantic = pedantic_first_ill_formed(reinterpret_cast<const char *>(&i),
        reinterpret_cast<const char *>(&i) + 4);
      auto normal = alt_first_ill_formed(reinterpret_cast<const char *>(&i),
          reinterpret_cast<const char *>(&i)+4);
      //auto normal = boost::unicode::first_ill_formed(reinterpret_cast<const char *>(&i),
      //    reinterpret_cast<const char *>(&i)+4);
      //BOOST_TEST(pedantic == normal);
      if (pedantic != normal)
      {
        ++boost::detail::test_errors();
        cout << std::hex << lowest_trace << " " << highest_trace << '\n';
        cout << "    Error:  first     second   for "
             << hex_string(string(reinterpret_cast<const char *>(&i),
        reinterpret_cast<const char *>(&i) + 4)) << " at "
          << reinterpret_cast<void*>(&i) <<
          "\n pedantic:  " << (void*)pedantic.first << "  "
            << (void*)pedantic.second << '\n' <<
          //"   normal:  " << (void*)normal.first << "  "
          "      alt:  " << (void*)normal.first << "  "
            << (void*)normal.second << '\n';
      }

      if (boost::detail::test_errors() >= 10)
      {
        cout << "  maximum errors exceeded, test cancelled" << endl;
        break;
      }
      ++i;
    } while (i != 0xFFFFFFFFu);



    cout << "  end utf8_test" << endl;

  }

}  // unnamed namespace

int cpp_main(int, char*[])
{
  utf8_test();

  return boost::report_errors();
}

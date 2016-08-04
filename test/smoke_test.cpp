﻿//  unicode/test/smoke_test.cpp  -------------------------------------------------------//

//  © Copyright Beman Dawes 2015

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <iostream>
using std::cout;
using std::endl;
#include <typeinfo>
#include "../include/boost/unicode/string_encoding.hpp"
#include "../include/boost/unicode/codecvt_conversion.hpp"
#include "../include/boost/unicode/stream.hpp"
#include "../include/boost/unicode/detail/utf8_codecvt_facet.hpp"
#include "../include/boost/unicode/detail/hex_string.hpp"
#include <cassert>
#include <string>
#include <sstream>
#include <iterator>
#define BOOST_LIGHTWEIGHT_TEST_OSTREAM std::cout
#include <boost/core/lightweight_test.hpp>

#include <boost/config.hpp>
#if !defined(BOOST_NO_CXX11_HDR_CODECVT)
#  include <codecvt>  // for codecvt_utf8
#endif

using namespace boost::unicode;
using namespace boost::unicode::detail;
using std::string;
using std::wstring;
using std::u16string;
using std::u32string;

namespace
{
  const string     u8str(u8"$€𐐷𤭢");
  const u16string u16str(u"$€𐐷𤭢");
  const u32string u32str(U"$€𐐷𤭢");
  const wstring     wstr(L"$€𐐷𤭢");

  template <class T>
  void check_inserter(T x, const string& expected)
  {
    //cout << "      test...\n";
    std::stringstream ss;
    std::string result;
    ss << x;
    ss << '\n';
    ss >> result;
    //cout << "result:" << hex_string(result) << endl;
    //cout << "expect:" << hex_string(expected) << endl;

    BOOST_TEST_EQ(result, expected);
  }

  void inserter_test()
  {
    cout << "inserter_test" << endl;

    check_inserter(u8"$€𐐷𤭢", u8str);
    check_inserter(boost::string_view(u8str), u8str);
    check_inserter(u8str, u8str);

    check_inserter(u"$€𐐷𤭢", u8str);
    check_inserter(boost::u16string_view(u16str), u8str);
    check_inserter(u16str, u8str);

    check_inserter(U"$€𐐷𤭢", u8str);
    check_inserter(boost::u32string_view(u32str), u8str);
    check_inserter(u32str, u8str);

    check_inserter(L"$€𐐷𤭢", u8str);
    check_inserter(boost::wstring_view(wstr), u8str);
    check_inserter(wstr, u8str);

    cout << "  inserter_test done" << endl;
  }

  void codecvt_short_test()
  {
    cout << "codecvt_short_test" << endl;
    boost::unicode::detail::utf8_codecvt_facet ccvt(0);
    std::mbstate_t state = std::mbstate_t();
    cout << "  u8str.size() is " <<  u8str.size() << endl;
    cout << "  utf8_codecvt_facet length(): "
         << ccvt.length(state, u8str.data(), u8str.data()+u8str.size(), 100) << endl;
    cout << "  utf8_codecvt_facet max_length(): " << ccvt.max_length() << endl;

    const string    str(u8"$€0123456789你好abcdefghijklmnopqrstyvwxyz");
    const wstring   wstr(L"$€0123456789你好abcdefghijklmnopqrstyvwxyz");

    string tmp = codecvt_to_string(wstr, ccvt);
    BOOST_TEST(tmp == str);
    //cout << hex_string(tmp) << endl;
    cout << hex_string(str) << endl;

    wstring wstr_tmp = codecvt_to_wstring(str, ccvt);
    BOOST_TEST(wstr_tmp == wstr);
    //cout << hex_string(wstr_tmp) << endl;
    //cout << hex_string(wstr) << endl;

    string FFEE(u8"\uFFEE");  // U+FFEE HALFWIDTH WHITE CIRCLE

    cout << "utf8:" << hex_string(FFEE) << endl;
    cout << "     " << hex_string(codecvt_to_wstring(FFEE, ccvt)) << endl;
    cout << "     " << hex_string(codecvt_to_wstring(boost::string_view(FFEE.data(), 3),
      ccvt)) << endl;
    cout << "     " << hex_string(codecvt_to_wstring(boost::string_view(FFEE.data(), 2),
      ccvt)) << endl;
    cout << "     " << hex_string(codecvt_to_wstring(boost::string_view(FFEE.data(), 1),
      ccvt)) << endl;
    
    string    asian(u8"$€你好");
    cout << "asian:" << hex_string(asian) << " | " << hex_string(codecvt_to_wstring(asian, ccvt)) << endl;
    asian.erase(1, 1);
    cout << "asian:" << hex_string(asian) << " | " << hex_string(codecvt_to_wstring(asian, ccvt)) << endl;

    cout << "  codecvt_short_test done" << endl;
  }

  void convert_encoding_test()
  {
    cout << "convert_encoding_test" << endl;
    u16string ru16;
    convert_encoding<char16_t>(u8str.cbegin(), u8str.cend(), std::back_inserter(ru16));
    BOOST_TEST(ru16 == u16str);
    cout << "  convert_encoding_test done" << endl;
  }

  void to_utf_string_test()
  {
    cout << "to_utf_string_test" << endl;

    // three argument tests
    u16string ru16 = to_utf_string<char16_t, char>(boost::string_view(u8str),
      ufffd<char16_t>(), std::allocator<char16_t>());
    BOOST_TEST(ru16 == u16str);
    ru16 = to_utf_string<char16_t, char, std::char_traits<char>>(
      boost::string_view(u8str), ufffd<char16_t>(), std::allocator<char16_t>());
    BOOST_TEST(ru16 == u16str);
    ru16 = to_utf_string<char16_t, char>(u8str, ufffd<char16_t>(),
      std::allocator<char16_t>());
    ru16 = to_utf_string<char16_t, char, std::char_traits<char>>(
      u8str, ufffd<char16_t>(), std::allocator<char16_t>());
    BOOST_TEST(ru16 == u16str);
    cout << "  to_utf_string_test done" << endl;

    // two argument tests
    ru16 = to_utf_string<char16_t, char>(boost::string_view(u8str), ufffd<char16_t>());
    BOOST_TEST(ru16 == u16str);
    ru16 = to_utf_string<char16_t, char, std::char_traits<char>>(
      boost::string_view(u8str), ufffd<char16_t>());
    BOOST_TEST(ru16 == u16str);
    ru16 = to_utf_string<char16_t, char>(u8str, ufffd<char16_t>());
    BOOST_TEST(ru16 == u16str);
    ru16 = to_utf_string<char16_t, char, std::char_traits<char>>(
      u8str, ufffd<char16_t>());
    BOOST_TEST(ru16 == u16str);

    // one argument tests
    ru16 = to_utf_string<char16_t, char>(boost::string_view(u8str));
    BOOST_TEST(ru16 == u16str);
    ru16 = to_utf_string<char16_t, char, std::char_traits<char>>(boost::string_view(u8str));
    BOOST_TEST(ru16 == u16str);
    ru16 = to_utf_string<char16_t, char>(u8str);
    BOOST_TEST(ru16 == u16str);
    ru16 = to_utf_string<char16_t, char, std::char_traits<char>>(u8str);
    BOOST_TEST(ru16 == u16str);
  }
 
  void to_u8string_test()
  {
    cout << "to_u8string_test" << endl;
    u16string u16s(u"$€𐐷𤭢");
    string u8s(u8"$€𐐷𤭢");
    cout << u8s.size() << endl;

    string u8r = to_u8string(u16s);
    BOOST_TEST_EQ(u8r.size(), u8s.size());
    BOOST_TEST(u8r == u8s);
    cout << "u8r :" << hex_string(u8r) << endl;

    cout << "  to_u8string_test done" << endl;
  }

  void to_u16string_test()
  {
    cout << "to_u16string_test" << endl;
    to_u16string(u8str);

    cout << "  u32s.size() " << u32str.size() << endl;
    cout << "  u16s.size() " << u16str.size() << endl;
    u16string u16r = to_u16string(u32str);
    cout << "  u16r.size() " << u16r.size() << endl;
    BOOST_TEST_EQ(u16r.size(), 6u);
    BOOST_TEST(u16r == u16str);

    u16r.clear();
    u16r = to_u16string(u8str);
    BOOST_TEST_EQ(u16r.size(), u16str.size());
    BOOST_TEST(u16r == u16str);

    cout << "  to_u16string_test done" << endl;
  }

  void to_u32string_test()
  {
    cout << "to_u32string_test" << endl;
    BOOST_TEST(to_u32string(u8str) == u32str);

    string u8s(u8"$¢€𐍈");
    BOOST_TEST_EQ(u8s.size(), 10u);
    u32string u32r = to_u32string(u8s);
    BOOST_TEST_EQ(u32r.size(), 4u);
    u32string u32s = {0x24, 0xA2, 0x20AC, 0x10348};
    BOOST_TEST(u32r == u32s);

    u32string u32sr3 = to_u32string(u16str);
    BOOST_TEST(u32sr3 == u32str);

    cout << "  to_u32string_test done" << endl;
  }

  void all_utf_test()
  {
    cout << "all_utf_test" << endl;

    BOOST_TEST(to_wstring(wstr) == wstr);
    BOOST_TEST(to_wstring(u8str) == wstr);
    BOOST_TEST(to_wstring(u8str, ufffd<wchar_t>()) == wstr);
    BOOST_TEST(to_wstring(u16str) == wstr);
    BOOST_TEST(to_wstring(u32str) == wstr);
    BOOST_TEST(to_wstring(u32str, ufffd<wchar_t>()) == wstr);

    BOOST_TEST(to_u8string(wstr) == u8str);
    BOOST_TEST(to_u8string(u8str) == u8str);
    BOOST_TEST(to_u8string(u16str) == u8str);
    BOOST_TEST(to_u8string(u32str) == u8str);

    BOOST_TEST(to_u16string(wstr) == u16str);
    BOOST_TEST(to_u16string(u8str) == u16str);
    BOOST_TEST(to_u16string(u16str) == u16str);
    BOOST_TEST(to_u16string(u32str) == u16str);

    BOOST_TEST(to_u32string(wstr) == u32str);
    BOOST_TEST(to_u32string(u8str) == u32str);
    BOOST_TEST(to_u32string(u16str) == u32str);
    BOOST_TEST(to_u32string(u32str) == u32str);

    cout << "  all_utf_test done" << endl;
  }


  void all_codecvt_test()
  {
#if !defined(BOOST_NO_CXX11_HDR_CODECVT)
    cout << "all_codecvt_test" << endl;

    //  for now, limit code points to the BMP to ensure test codecvt facet support

    const string     u8s(u8"$€Ꭶ❄");
    const wstring      ws(L"$€Ꭶ❄");

    std::codecvt_utf8<wchar_t> ccvt_utf8; 

    cout << u8s.size() << endl;
    cout << hex_string(u8s) << endl;
    wstring w = codecvt_to_wstring(u8s, ccvt_utf8);
    cout << w.size() << endl;
    cout << hex_string(w) << endl;
    BOOST_TEST(w == ws);

    BOOST_TEST(codecvt_to_wstring(u8s, ccvt_utf8) == ws);
    BOOST_TEST(codecvt_to_wstring(u8s, ccvt_utf8, std::allocator<wchar_t>()) == ws);

    BOOST_TEST(codecvt_to_string(ws, ccvt_utf8) == u8s);
    BOOST_TEST(codecvt_to_string(ws, ccvt_utf8, std::allocator<char>()) == u8s);

    cout << "  all_codecvt_test done" << endl;
#endif
  }

  //  Probe CharTraits template argument deduction

  template <class CharT, class CharTraits>
  void f(boost::basic_string_view<CharT, CharTraits> v)
  {
    std::cout << "overload 1:" << v << std::endl;
  }

  template <class CharT>
  void f(boost::basic_string_view<CharT> v)
  {
    std::cout << "overload 2:" << v << std::endl;
  }

  template <class CharT, class CharTraits = typename std::char_traits<CharT>>
  void g(boost::basic_string_view<CharT, CharTraits> v)
  {
    std::cout << "g():" << v << std::endl;
  }

}  // unnamed namespace

int main()
{
  //  Probe CharTraits template argument deduction
  f(boost::string_view("string_view"));                     // works
  f<char>(string("std::string"));                           // works
  f<char>("C-string");                                      // works
  f<char, std::char_traits<char>>(string("std::string"));   // works
  f<char, std::char_traits<char>>("C-string");              // works
  g(boost::string_view("string_view"));                     // works
  //g<char>(string("std::string"));             // CharTraits template arg deduction fails
  //g<char>("C-string");                        // CharTraits template arg deduction fails
  g<char, std::char_traits<char>>(string("std::string"));   // works
  g<char, std::char_traits<char>>("C-string");              // works



  cout << "wstr   :" << hex_string(wstr) << endl;
  cout << "u8str  :" << hex_string(u8str) << endl;
  cout << "u16str :" << hex_string(u16str) << endl;
  cout << "u32str :" << hex_string(u32str) << endl;


  // TODO: Add test to verify InputIterators do not have to be contiguous and do not
  // have to meet any forward, bidirectional, or random access requirements.

  inserter_test();
  codecvt_short_test();
  convert_encoding_test();
  to_utf_string_test();
  to_u8string_test();
  to_u16string_test();
  to_u32string_test();
  all_utf_test();
  all_codecvt_test();

  return boost::report_errors();
}

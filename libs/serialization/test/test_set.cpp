/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// test_set.cpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// should pass compilation and execution

#include <fstream>

#include <boost/serialization/set.hpp>
#ifdef BOOST_HAS_HASH
#include <boost/serialization/hash_set.hpp>
#endif
#include <boost/archive/archive_exception.hpp>

#include "test_tools.hpp"

#include "A.hpp"

int test_main( int /* argc */, char* /* argv */[] )
{
    const char * testfile = tmpnam(NULL);
    BOOST_REQUIRE(NULL != testfile);

    // test array of objects
    std::set<A> aset;
    aset.insert(A());
    aset.insert(A());
    {   
        test_ostream os(testfile, TEST_STREAM_FLAGS);
        test_oarchive oa(os);
        oa << boost::serialization::make_nvp("aset", aset);
    }
    std::set<A> aset1;
    {
        test_istream is(testfile, TEST_STREAM_FLAGS);
        test_iarchive ia(is);
        ia >> boost::serialization::make_nvp("aset", aset1);
    }
    BOOST_CHECK(aset == aset1);
    
    std::multiset<A> amultiset;
    amultiset.insert(A());
    amultiset.insert(A());
    {   
        test_ostream os(testfile, TEST_STREAM_FLAGS);
        test_oarchive oa(os);
        oa << boost::serialization::make_nvp("amultiset", amultiset);
    }
    std::multiset<A> amultiset1;
    {
        test_istream is(testfile, TEST_STREAM_FLAGS);
        test_iarchive ia(is);
        ia >> boost::serialization::make_nvp("amultiset", amultiset1);
    }
    BOOST_CHECK(amultiset == amultiset1);
    
    #ifdef BOOST_HAS_HASH
    // test array of objects
    BOOST_STD_EXTENSION_NAMESPACE::hash_set<A> ahash_set;
    ahash_set.insert(A());
    ahash_set.insert(A());
    {   
        test_ostream os(testfile, TEST_STREAM_FLAGS);
        test_oarchive oa(os);
        oa << boost::serialization::make_nvp("ahash_set", ahash_set);
    }
    BOOST_STD_EXTENSION_NAMESPACE::hash_set<A> ahash_set1;
    {
        test_istream is(testfile, TEST_STREAM_FLAGS);
        test_iarchive ia(is);
        ia >> boost::serialization::make_nvp("ahash_set", ahash_set1);
    }
    BOOST_CHECK(ahash_set == ahash_set1);
    
    BOOST_STD_EXTENSION_NAMESPACE::hash_multiset<A> ahash_multiset;
    ahash_multiset.insert(A());
    ahash_multiset.insert(A());
    {   
        test_ostream os(testfile, TEST_STREAM_FLAGS);
        test_oarchive oa(os);
        oa << boost::serialization::make_nvp("ahash_multiset", ahash_multiset);
    }
    BOOST_STD_EXTENSION_NAMESPACE::hash_multiset<A> ahash_multiset1;
    {
        test_istream is(testfile, TEST_STREAM_FLAGS);
        test_iarchive ia(is);
        ia >> boost::serialization::make_nvp("ahash_multiset", ahash_multiset1);
    }
    BOOST_CHECK(ahash_multiset == ahash_multiset1);
    
#endif
	std::remove(testfile);
    return boost::exit_success;
}

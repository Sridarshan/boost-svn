// 
// fail_ref_csubarray2.cpp -
//   Testing constness of subarray operations.
//

#include "boost/multi_array_ref.hpp"

#define BOOST_INCLUDE_MAIN
#include "boost/test/test_tools.hpp"

#include "boost/array.hpp"

int
test_main(int,char**)
{
  const int ndims=3;
  typedef boost::multi_array_ref<int,ndims> array_ref;

  boost::array<array_ref::size_type,ndims> sma_dims = {{2,3,4}};

  int data[] = {77,77,77,77,77,77,77,77,77,77,77,77, 
                 77,77,77,77,77,77,77,77,77,77,77,77}; 

  array_ref sma(data,sma_dims);

  int num = 0;
  for (array_ref::index i = 0; i != 2; ++i)
    for (array_ref::index j = 0; j != 3; ++j)
      for (array_ref::index k = 0; k != 4; ++k)
	sma[i][j][k] = num++;

  const array_ref& sma_const = sma;

  array_ref::subarray<ndims-1>::type sba = sma_const[0]; // FAIL!
							 // preserve constness

  return boost::exit_success;
}

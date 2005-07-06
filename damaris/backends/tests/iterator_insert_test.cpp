/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include <list>
#include <cstdio>

int main() {

  // empty list
  std::list<int> test_list;

  test_list.push_back(2);
  std::list<int>::iterator i2=--test_list.end();
  test_list.push_back(4);

  test_list.push_front(0);
  test_list.insert(++test_list.begin(),1);
  test_list.insert(++i2,3);

  for (std::list<int>::const_iterator i=test_list.begin();i!=test_list.end();i++)
    printf("%d\n",*i);

  return 0;
}

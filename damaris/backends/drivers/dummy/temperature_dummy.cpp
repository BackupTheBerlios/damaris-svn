#include "dummy.h"

int main() {

  // temperature dummy
  tempcont* t=new dummy();
  
  sleep(5);
  t->set_history_stepsize(2);
  sleep(10);
  temp_history* h=t->get_history(0);
  h->print_xml(stdout);
  delete h;
  sleep(5);
  t->set_history_stepsize(0);
  h=t->get_history(0);
  h->print_xml(stdout);
  delete h;
  delete t;

}

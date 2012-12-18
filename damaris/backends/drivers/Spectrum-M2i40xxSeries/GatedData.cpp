#include "GatedData.h"
#include <cstring>

#ifndef SIZETPRINTFLETTER
#  ifndef __LP64__
#    define SIZETPRINTFLETTER "u"
#  else
#    define SIZETPRINTFLETTER "lu"
#  endif
#endif


/*
   size of all childrens' data
*/
size_t DataManagementNode::size() const {
	if (child==NULL)
		return n;

	size_t size_n=0;

	for (const DataManagementNode* i=child; i!=NULL; i=i->next)
		size_n+=i->size();

	return size_n*n;
}

/*
   reduce depth recursively
*/
DataManagementNode* DataManagementNode::reduce() {
	// simple case: no children
	if (child==NULL)
		return this;

	// evaluate all children's nodes and append remaining
	DataManagementNode* i=child;
	DataManagementNode* last_child=NULL;

	while (i->next!=NULL) {
		DataManagementNode* reduced_child=i->reduce();
		if (reduced_child->n!=0) {
			if (last_child==NULL)
				child=reduced_child;
			else
				last_child->next=reduced_child;
			last_child=reduced_child;
			last_child->parent=this;
			last_child->next=NULL;
		}
		DataManagementNode* next_child=i->next;

		// throw away unused one
		if (reduced_child->n==0 || i!=reduced_child) {
			i->parent=NULL;
			i->next=NULL;
			delete i;
		}
		i=next_child;
	}
	// now look at the remaining list
	if (child==NULL) {
		// return empty node
		n=0;
	} else
		// reduce depth by one
		if (child->next==NULL || child->child!=NULL) {
			n*=child->n;
			DataManagementNode* superficial=child;
			child=child->child;
			superficial->parent=NULL;
			delete superficial;
		}
	return this;
}

void DataManagementNode::print(FILE* f,size_t indent) {
	char* indent_space=new char[indent+1];
	memset(indent_space,' ',indent);
	indent_space[indent]=0;

	if (child==NULL)
		fprintf(f,"%s%" SIZETPRINTFLETTER " samples\n",indent_space, n);
	else {
		fprintf(f,"%s%" SIZETPRINTFLETTER ":\n",indent_space, n);
		child->print(f,indent+6);
	}

	delete indent_space;

	if (next!=NULL)
		next->print(f,indent);
  
}

DataManagementNode::DataManagementNode(const DataManagementNode& orig) {
  parent=NULL;
  if (orig.child!=NULL) {
    child=new DataManagementNode(*orig.child);
    for (DataManagementNode* i=child; i!=NULL; i=i->next) i->parent=this;
  }
  if (orig.next!=NULL) next=new DataManagementNode(*orig.next);
  n=orig.n;
}

/**
   recursive destruction
*/
DataManagementNode::~DataManagementNode() {
	while (child!=NULL) {
		DataManagementNode* this_one=child;
		child=child->next;
		this_one->parent=NULL;
		this_one->next=NULL;
		delete this_one;
	}
}

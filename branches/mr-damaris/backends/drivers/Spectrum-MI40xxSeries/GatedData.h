#include <cstddef>
#include <cstdio>

class DataManagementNode;

/**
   manages nested ADC data structure
*/
class DataManagementNode {
 public:
  /// parent node, top node has NULL
  DataManagementNode* parent;
  /// reference to nested loops
  DataManagementNode* child;
  /// next node in sequences
  DataManagementNode* next;

  /**
     if the child is NULL, n is the number of real samples
     otherwise repeatition number of substructure
  */
  size_t n;
  
  /**
     default constructor of NOP node
  */
  DataManagementNode(class DataManagementNode* par=NULL): parent(par), child(NULL), next(NULL), n(0) {}

  /**
     size of all childrens' data
  */
  size_t size() const;
  /**
     reduce depth recursively
  */
  DataManagementNode* reduce();


  void print(FILE* f,size_t indent=0);

  /**
     recursive destruction, call only for root
  */
  ~DataManagementNode();
};

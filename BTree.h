#ifndef BTREE_H
#define BTREE_H

#define M 5
#define MINITEMS ((M-1)/2)
#define MAXITEMS (M-1)
#define MAXCHILDREN M 
#define MINCHILDREN ((M+1)/2)
typedef enum{FAILURE, SUCCESS} status_code;
typedef enum{FALSE, TRUE} Boolean;
typedef int data_t;
typedef int key_type;
typedef int idx_t;
typedef struct element_t_tag {
    data_t data;
    key_type key;
}element_t;

typedef struct BTreeNode_tag {
    element_t elements[M-1];
    struct BTreeNode_tag* children[M];
    struct BTreeNode_tag* parent;
    int cnt;
}BTreeNode;

typedef struct LLNode_tag {
    element_t element;
    struct LLNode_tag* next;
}LLNode;


void InitialiseProgram();

/* helper functions */
void RotateRight(BTreeNode* leftsib, BTreeNode* ptr, idx_t par_idx, idx_t del_idx);
void RotateLeft(BTreeNode* rightsib, BTreeNode* ptr, idx_t par_idx, idx_t del_idx);
void MergeWithLeft(BTreeNode* leftsib, BTreeNode* ptr, BTreeNode* newnode, BTreeNode* par, element_t *deleptr, idx_t par_idx, idx_t del_idx);
void MergeWithRight(BTreeNode* rightsib, BTreeNode* ptr, BTreeNode* newnode, BTreeNode* par, element_t *deleptr, idx_t par_idx, idx_t del_idx);

Boolean isLeaf(BTreeNode* ptr);
idx_t PartitionIdx(element_t e, BTreeNode* bptr);
idx_t MedianIndex(BTreeNode* ptr, element_t *e_ptr);
// splits ptr into two, reassigns e 
status_code SplitAndReassign(
    BTreeNode* ptr, 
    element_t *e_ptr, 
    BTreeNode **lcptr, 
    BTreeNode **rcptr,
    BTreeNode *lprev,
    BTreeNode *rprev
);
/* User-visible templates */
void InitialiseBTree(BTreeNode* root);
status_code BTreeInsert(element_t e, BTreeNode **root_ptr);
status_code RecDelete(idx_t del_idx, BTreeNode* ptr, BTreeNode* newchild, BTreeNode **root_ptr);
BTreeNode* BTreeSearch(key_type k, BTreeNode* root, idx_t* ret_idx);
status_code BTreeDelete(key_type k, BTreeNode** root_ptr);

/* print functions */
void PrintBTree(BTreeNode* root);
// Preorder traversal
void FPrintBTree(BTreeNode* root);
void PrintStatus(const char* Opcode, status_code sc);
// Preorder traversal
void FPrintAllInfo(BTreeNode* root);
#endif
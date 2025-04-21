#ifndef FAMILYCRUD_H
#define FAMILYCRUD_H
#include "utils.h"

#define FAMILYINFOFILE "family_info.txt"

/* core wrappers */
status_code DeleteFamily(int familyID);
status_code UpdateFamily(int familyID);
/* core functions */
void InitialiseFamilyTree();
float GetTotalFamilyExpense(int familyID);
void GetCategoricalExpenseFamily(int familyID, expenseCategory cat);
status_code FamilyTreeInsert(FamilyElement e, FamilyNode **root_ptr);
status_code FamilyTreeDelete(family_key_t k, FamilyNode** root_ptr);
FamilyNode* FamilyTreeSearch(family_key_t k, FamilyNode* root, idx_t* ret_idx);
status_code RecDelete_Family(idx_t del_idx, FamilyNode* ptr, FamilyNode* newchild, FamilyNode **root_ptr);

/* helper functions - data management */
status_code CreateFamily(family_key_t FamilyID, UserElement ue);
void GenerateFamilyName(char* username, char* FamilyName);
family_key_t GenerateFamilyId();

/* helper functions - family tree structure */
status_code SplitAndReassign_Family(
    FamilyNode* ptr, 
    FamilyElement *e_ptr, 
    FamilyNode **lcptr, 
    FamilyNode **rcptr,
    FamilyNode *lprev,
    FamilyNode *rprev
);

/* helper functions - family tree structure */
idx_t PartitionIdx_Family(FamilyElement e, FamilyNode* bptr); 
Boolean isLeaf_Family(FamilyNode* ptr);
void RotateRight_Family(FamilyNode* leftsib, FamilyNode* ptr, idx_t par_idx, idx_t del_idx);
void RotateLeft_Family(FamilyNode* rightsib, FamilyNode* ptr, idx_t par_idx, idx_t del_idx);
void MergeWithLeft_Family(FamilyNode* leftsib, FamilyNode* ptr, FamilyNode* newchild, FamilyNode* par, FamilyElement* del_elem, idx_t par_idx, idx_t del_idx);
void MergeWithRight_Family(FamilyNode* rightsib, FamilyNode* ptr, FamilyNode* newchild, FamilyNode* par, FamilyElement* del_elem, idx_t par_idx, idx_t del_idx);

void FPrintAllInfo_Family(FamilyNode* root);
void FPrintInorderTable_Family(FamilyNode* root);
#endif
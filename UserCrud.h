#ifndef USERCRUD_H
#define USERCRUD_H

#include "utils.h"

#define USERINFOFILE "user_info.txt"


/* core wrappers */
status_code AddUser(User user);
status_code DeleteUser(int userID);
float GetIndividualExpense(int userID, int month);
status_code RangedSearch(int ExpID_low, int ExpID_high, int UserID);
status_code UpdateUser(int UserID);

/* core functions */
void InitialiseUserTree();
/* have to check if exists */
status_code UserTreeInsert(UserElement e, UserNode **root_ptr);
UserNode* UserTreeSearch(user_key_t k, UserNode* root, idx_t* ret_idx);
status_code RecDelete_User(idx_t del_idx, UserNode* ptr, UserNode* newchild, UserNode **root_ptr);
status_code UserTreeDelete(user_key_t k, UserNode** root_ptr);
status_code ReadUserData();
/* helper functions - data management */    
void GetCategoricalExpenseUserHelper(user_key_t k, ExpenseNode* root, float* categorical_expense);
void GetCategoricalExpenseUser(user_key_t k, float* categorical_expense);
void RangedSearchHelper(expense_key_t lower_bound, expense_key_t upper_bound, ExpenseNode* root);
void RangedExtrema(ExpenseNode* ptr, idx_t *start, idx_t *end, expense_key_t lower, expense_key_t upper);
status_code DeleteUserHelper_ExpensesDelete(user_key_t userID);
float GetMonthlyUserExpense(user_key_t k, float *monthly_expense);

user_key_t GenerateUserId();

float GetMonthlyUserExpenseHelper(user_key_t k, float *monthly_expense, ExpenseNode* root);
void SetDeletionList(ExpenseNode* ptr, user_key_t userID, LLExpNode **head, LLExpNode **tail);

/* helper functions - user tree structure */
status_code SplitAndReassign_User(
    UserNode* ptr, 
    UserElement *e_ptr, 
    UserNode **lcptr, 
    UserNode **rcptr,
    UserNode *lprev,
    UserNode *rprev
);
idx_t PartitionIdx_User(UserElement e, UserNode* bptr);
Boolean isLeaf_User(UserNode* ptr);

/* delete helpers */
void RotateRight_User(UserNode* leftsib, UserNode* ptr, idx_t par_idx, idx_t del_idx);
void RotateLeft_User(UserNode* rightsib, UserNode* ptr, idx_t par_idx, idx_t del_idx);
void MergeWithLeft_User(UserNode* leftsib, UserNode* ptr, UserNode* newnode, UserNode* par, UserElement *deleptr, idx_t par_idx, idx_t del_idx);
void MergeWithRight_User(UserNode* rightsib, UserNode* ptr, UserNode* newnode, UserNode* par, UserElement *deleptr, idx_t par_idx, idx_t del_idx);

// Preorder traversal
void FPrintAllInfo_User(UserNode* root);
// Inorder Traversal table
void FPrintInorderTable_User(UserNode* root);

#endif
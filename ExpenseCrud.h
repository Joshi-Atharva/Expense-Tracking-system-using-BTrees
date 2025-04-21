#ifndef EXPENSECRUD_H
#define EXPENSECRUD_H

#include "utils.h"
#define EXPENSEINFOFILE "expense_info.txt"


status_code AddExpense(Expense expense);
status_code DeleteExpense(int expenseID);
status_code PeriodSearch(char* start_date, char* end_date);
void PeriodSearchHelper(int low, int high, ExpenseNode* root);
status_code GetHighestExpenseDay(int familyID);
status_code GetHighestExpenseDayHelper_Creation(FamilyElement fe, ExpenseNode* root, ExpenseNode** root_date_ptr);
status_code GetHighestExpenseDayHelper_Max(ExpenseNode* root_date, float* max_expense, char* date);
status_code UpdateExpense(int expenseID);

/* core functions */
void InitialiseExpenseTree();
/* have to check if exists */
status_code ExpenseTreeInsert(ExpenseElement e, ExpenseNode **root_ptr);
ExpenseNode* ExpenseTreeSearch(expense_key_t k, ExpenseNode* root, idx_t* ret_idx);
status_code RecDelete_Expense(idx_t del_idx, ExpenseNode* ptr, ExpenseNode* newchild, ExpenseNode **root_ptr);
status_code ExpenseTreeDelete(expense_key_t k, ExpenseNode** root_ptr);

status_code ReadExpenseData();
ExpenseNode* ExpenseTreeSearchByExpenseID(int expense_id, ExpenseNode* root, idx_t* ret_idx);
ExpenseNode* ExpenseTreeSearchByUserID(int user_id, ExpenseNode* root, idx_t* ret_idx);
expense_key_t GenerateExpenseKey(Expense e);
Boolean InorderExpense(ExpenseNode* root, int* expense_id_ptr);
ExpenseNode* MinExpKeyOfUser(int user_id, ExpenseNode* root, idx_t* ret_idx);
Boolean UserInFam(int user_id, FamilyElement fe);
/* helper functions - expense tree structure */
status_code SplitAndReassign_Expense(
    ExpenseNode* ptr, 
    ExpenseElement *e_ptr, 
    ExpenseNode **lcptr, 
    ExpenseNode **rcptr,
    ExpenseNode *lprev,
    ExpenseNode *rprev
);
idx_t PartitionIdx_Expense(ExpenseElement e, ExpenseNode* bptr);
Boolean isLeaf_Expense(ExpenseNode* ptr);

/* delete helpers */
void RotateRight_Expense(ExpenseNode* leftsib, ExpenseNode* ptr, idx_t par_idx, idx_t del_idx);
void RotateLeft_Expense(ExpenseNode* rightsib, ExpenseNode* ptr, idx_t par_idx, idx_t del_idx);
void MergeWithLeft_Expense(ExpenseNode* leftsib, ExpenseNode* ptr, ExpenseNode* newnode, ExpenseNode* par, ExpenseElement *deleptr, idx_t par_idx, idx_t del_idx);
void MergeWithRight_Expense(ExpenseNode* rightsib, ExpenseNode* ptr, ExpenseNode* newnode, ExpenseNode* par, ExpenseElement *deleptr, idx_t par_idx, idx_t del_idx);

// Preorder traversal
void FPrintAllInfo_Expense(ExpenseNode* root);
void FPrintInorderTable_Expense(ExpenseNode* root);
#endif
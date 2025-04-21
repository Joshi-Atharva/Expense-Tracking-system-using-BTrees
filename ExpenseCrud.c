#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UserCrud.h"
#include "FamilyCrud.h"
#include "ExpenseCrud.h"
#include "utils.h"

extern UserNode* root_user;
extern FamilyNode* root_family;
extern ExpenseNode* root_expense;

status_code AddExpense(Expense expense) {
    status_code sc = SUCCESS;
    ExpenseElement ee;
    ee.data = expense;
    ee.key = GenerateExpenseKey(expense);
    /* check if already exists in database */
    idx_t ret_idx;
    ExpenseNode* eptr = ExpenseTreeSearch(ee.key, root_expense, &ret_idx);
    if( eptr != NULL ) {
        /* expense_id already exists */
        sc = FAILURE;
    }
    else { /* expense does not exist */
        /* find user and update family expenses */
        idx_t ret_idx;
        UserNode* uptr = UserTreeSearch(expense.user_id, root_user, &ret_idx);
        if( uptr != NULL ) {
            /* user_id exists */
            FamilyNode* fptr = FamilyTreeSearch(uptr->elements[ret_idx].data.family_id, root_family, &ret_idx);
            if( fptr != NULL ) {
                /* family_id exists */
                idx_t month = 10*(expense.date_of_expense[3] - '0') + (expense.date_of_expense[4] - '0');
                fptr->elements[ret_idx].data.monthly_family_expense[month - 1] += expense.expense_amount;
            }
            sc = ExpenseTreeInsert(ee, &root_expense);
        }
        else { /* No corresponding user_id */
            sc = FAILURE;
        }
    
    }
    return sc;
}
status_code DeleteExpense(int expenseID) {
    status_code sc = SUCCESS;
    idx_t ret_idx;
    ExpenseNode* eptr = ExpenseTreeSearch(expenseID, root_expense, &ret_idx);
    if( eptr != NULL ) {
        /* expense_id exists */

        /* update family expenses */
        int userID = eptr->elements[ret_idx].data.user_id;
        idx_t user_ret_idx;
        UserNode* uptr = UserTreeSearch(userID, root_user, &user_ret_idx);
        if( uptr != NULL ) {
            /* user_id exists */
            FamilyNode* fptr = FamilyTreeSearch(uptr->elements[user_ret_idx].data.family_id, root_family, &user_ret_idx);
            if( fptr != NULL ) {
                /* family_id exists */
                int month = 10*(eptr->elements[ret_idx].data.date_of_expense[3] - '0') + (eptr->elements[ret_idx].data.date_of_expense[4] - '0');
                fptr->elements[user_ret_idx].data.monthly_family_expense[month - 1] -= eptr->elements[ret_idx].data.expense_amount;
            }
        }
        /* delete the expense */ 
        sc = ExpenseTreeDelete(eptr->elements[ret_idx].key, &root_expense);
        if( sc == FAILURE ) {
            printf("Error: Expense deletion failed.\n");
        }
    }
    else {
        /* expense_id does not exist */
        printf("Error: Expense ID %d does not exist.\n", expenseID);
        sc = FAILURE;
    }
    return sc;
}
status_code PeriodSearch(char* start_date, char* end_date) {
    status_code sc = SUCCESS;
    int low, high;
    low = dateString_to_intKey(start_date);
    high = dateString_to_intKey(end_date);
    if( low > high ) {
        printf("Error: Invalid date range.\n");
        sc = FAILURE;
    }
    else {
        ExpenseNode* root = root_expense;
        /* call recursive function for complete traversal */
        PeriodSearchHelper(low, high, root);
    }
    return sc;
}
void PeriodSearchHelper(int low, int high, ExpenseNode* root) {
    if( root != NULL ) {
        char category[25];
        int date_key;
        int i = 0;
        while( i < root->cnt ) { /* inorder wrt expense_key */
            PeriodSearchHelper(low, high, root->children[i]);
            date_key = dateString_to_intKey(root->elements[i].data.date_of_expense);
            if( date_key >= low && date_key <= high ) {
                /* print the expense */
                expenseCategory_to_string(category, root->elements[i].data.expense_category);
                printf("User ID: %d\t Expense ID: %d\t Amount: %.2f \tDate: %s\t Category: %s\n", 
                    root->elements[i].data.user_id,
                    root->elements[i].data.expense_id, 
                    root->elements[i].data.expense_amount,
                    root->elements[i].data.date_of_expense,
                    category
                );
            }
            i++;
        }
        PeriodSearchHelper(low, high, root->children[i]);
    }
}
status_code GetHighestExpenseDay(int familyID) {
    status_code sc = SUCCESS;
    float max_expense = 0;
    char date[11];
    char category[25];
    idx_t family_ret_idx;
    FamilyNode* fptr = FamilyTreeSearch(familyID, root_family, &family_ret_idx);
    if( fptr != NULL ) {
        /* family_id exists */
        ExpenseNode* root_date;
        /* initialize the root_date */
        root_date = (ExpenseNode*)malloc(sizeof(ExpenseNode));
        root_date->cnt = 0;
        root_date->parent = NULL;
        root_date->children[0] = NULL;

        /* call recursive function for complete traversal - creation */
        GetHighestExpenseDayHelper_Creation(fptr->elements[family_ret_idx], root_expense, &root_date);

        /* call recursive function for complete traversal - finding max */
        GetHighestExpenseDayHelper_Max(root_date, &max_expense, date);
        /* print the max expense */
        printf("Family ID: %d\n", familyID);
        printf("Highest Expense Amount: %.2f\n", max_expense);
        printf("Date of Expense: %s\n", date);
    }
    else {
        /* family_id does not exist */
        printf("Error: Family ID %d does not exist.\n", familyID);
        sc = FAILURE;
    }
    return sc;
}
status_code GetHighestExpenseDayHelper_Creation(FamilyElement fe, ExpenseNode* root, ExpenseNode** root_date_ptr) {
    status_code sc = SUCCESS;
    if( root != NULL ) {
        char category[25];
        int i = 0;
        int userID;
        while( i < root->cnt ) { /* inorder wrt expense_key */
            sc = sc && GetHighestExpenseDayHelper_Creation(fe, root->children[i], root_date_ptr);
            userID = root->elements[i].data.user_id;
            if( UserInFam(userID, fe) ) {
                /* add the expense to the new tree */
                ExpenseElement ede;
                ede.data = root->elements[i].data;
                ede.key = dateString_to_intKey(root->elements[i].data.date_of_expense);
                /* check if already exists in database */
                idx_t ret_idx;
                ExpenseNode* eptr = ExpenseTreeSearch(ede.key, *root_date_ptr, &ret_idx);
                if( eptr != NULL ) {
                    /* expense_id already exists */
                    /* update the amount */
                    (*root_date_ptr)->elements[ret_idx].data.expense_amount += root->elements[i].data.expense_amount;
                }
                else { /* expense does not exist */
                    /* insert the new element */
                    sc = sc && ExpenseTreeInsert(ede, root_date_ptr);
                }
            }
            i++;
        }
        sc = sc && GetHighestExpenseDayHelper_Creation(fe, root->children[i], root_date_ptr);
    }
    return sc;
}
status_code GetHighestExpenseDayHelper_Max(ExpenseNode* root_date, float* max_expense, char* date) {
    status_code sc = SUCCESS;
    if( root_date != NULL ) {
        float temp;
        char temp_date[11];
        int i = 0;
        while( i < root_date->cnt ) { /* posorder wrt expense_key */
            sc = sc && GetHighestExpenseDayHelper_Max(root_date->children[i], &temp, temp_date);
            if( root_date->elements[i].data.expense_amount > temp ) {
                /* update the max */
                temp = root_date->elements[i].data.expense_amount;
                strcpy(temp_date, root_date->elements[i].data.date_of_expense);
            }
            if( temp > *max_expense ) {
                *max_expense = temp;
                strcpy(date, temp_date);
            }
            i++;
        }
        sc = sc && GetHighestExpenseDayHelper_Max(root_date->children[i], &temp, temp_date);
        if( temp > *max_expense ) {
            /* update the max */
            *max_expense = temp;
            strcpy(date, temp_date);
        }
    }
    return sc;
}
status_code UpdateExpense(int expenseID) {
    status_code sc = SUCCESS;
    idx_t ret_idx;
    ExpenseNode* eptr = ExpenseTreeSearchByExpenseID(expenseID, root_expense, &ret_idx);
    if( eptr != NULL ) {
        /* expense_id exists */
        /* ask the field to be updated */
        int field;
        printf("Enter the field to be updated:\n");
        printf("1. Expense Amount\n");
        printf("2. Date of Expense\n");
        printf("3. Category\n");
        scanf("%d", &field);
        
        idx_t family_ret_idx, user_ret_idx;
        UserNode* uptr; FamilyNode* fptr;
        switch(field) {
            case 1:
                printf("Enter new expense amount: ");
                float new_expense_amount;
                scanf("%f", &new_expense_amount);
            
                uptr = UserTreeSearch(eptr->elements[ret_idx].data.user_id, root_user, &user_ret_idx);
                if( uptr != NULL ) {
                    /* user_id exists */
                    fptr = FamilyTreeSearch(uptr->elements[user_ret_idx].data.family_id, root_family, &family_ret_idx);
                    if( fptr != NULL ) {
                        /* family_id exists */
                        int month = 10*(eptr->elements[ret_idx].data.date_of_expense[3] - '0') + (eptr->elements[ret_idx].data.date_of_expense[4] - '0');
                        fptr->elements[family_ret_idx].data.monthly_family_expense[month - 1] -= eptr->elements[ret_idx].data.expense_amount;
                        fptr->elements[family_ret_idx].data.monthly_family_expense[month - 1] += new_expense_amount;
                        
                        /* update the amount */
                        eptr->elements[ret_idx].data.expense_amount = new_expense_amount;
                    }
                    else {
                        sc = FAILURE;
                    }
                }

                break;
            case 2:
                printf("Enter new date of expense (DD-MM-YYYY): ");
                char new_date[11];
                scanf("%s", new_date);
            
                uptr = UserTreeSearch(eptr->elements[ret_idx].data.user_id, root_user, &user_ret_idx);
                if( uptr != NULL ) {
                    /* user_id exists */
                    fptr = FamilyTreeSearch(uptr->elements[user_ret_idx].data.family_id, root_family, &family_ret_idx);
                    if( fptr != NULL ) {
                        /* family_id exists */
                        int month = 10*(eptr->elements[ret_idx].data.date_of_expense[3] - '0') + (eptr->elements[ret_idx].data.date_of_expense[4] - '0');
                        fptr->elements[family_ret_idx].data.monthly_family_expense[month - 1] -= eptr->elements[ret_idx].data.expense_amount;
                        month = 10*(new_date[3] - '0') + (new_date[4] - '0');
                        fptr->elements[family_ret_idx].data.monthly_family_expense[month - 1] += eptr->elements[ret_idx].data.expense_amount;

                        /* update the date */
                        strcpy(eptr->elements[ret_idx].data.date_of_expense, new_date);
                    }
                    else {
                        sc = FAILURE;
                    }
                }
                break;
                
            default:
                printf("Invalid field.\n");
                sc = FAILURE;
        }
    }
    else {
        /* expense_id does not exist */
        sc = FAILURE;
    }
    return sc;
}
/* core functions */
void InitialiseExpenseTree() {
    root_expense->cnt = 0;
    root_expense->parent = NULL;
    root_expense->children[0] = NULL;
}
/* have to check if exists */
status_code ExpenseTreeInsert(ExpenseElement e, ExpenseNode **root_ptr) {
    status_code sc = SUCCESS;
    ExpenseNode* root = *root_ptr;
    ExpenseNode* ptr = root;
    if( ptr == NULL ) { /* make new node */
        ptr = (ExpenseNode*)malloc(sizeof(ExpenseNode));
        if( ptr == NULL ) sc = FAILURE;
        else {
            ptr->cnt = 1;
            ptr->parent = NULL;
            ptr->children[0] = NULL;
            ptr->children[1] = NULL;
            ptr->elements[0].data = e.data;
            ptr->elements[0].key = e.key;
        }
    }
    else { /* ptr != NULL */

        /* finding the way downto the correct leaf node */
        while( ptr->children[0] != NULL ) {
            ptr = ptr->children[PartitionIdx_Expense(e, ptr)];
        } // ptr points to the node of insertion
        
        Boolean done = FALSE;
        ExpenseNode *lcurr = ptr, *rcurr = ptr, *lprev = NULL, *rprev = NULL, *par;
        /* bottom up process till value is stably assigned */
        while( !done ) {
            if( ptr == NULL ) {
                ptr = (ExpenseNode*)malloc(sizeof(ExpenseNode));
                if( ptr == NULL ) sc = FAILURE;
                else {
                    ptr->cnt = 0;
                    ptr->children[0] = NULL;
                    ptr->parent = NULL;
                }
            }
            if( ptr->cnt < M-1 ) {
                /* capacity not exceeded, carry out direct insertion */
                idx_t idx = PartitionIdx_Expense(e, ptr);
                ptr->cnt = ptr->cnt + 1;

                /* insertion and shifting by  rotating temporaries */
                ExpenseElement temp, ins; // for elements array
                ExpenseNode* temp_ptr, *ins_ptr; // for children array
                ins = e;  
                ptr->children[idx] = lprev;
                ins_ptr = rprev;
                while( idx < ptr->cnt ) {
                    temp = ptr->elements[idx]; temp_ptr = ptr->children[idx+1];
                    ptr->elements[idx] = ins; ptr->children[idx+1] = ins_ptr;
                    ins = temp; ins_ptr = temp_ptr;
                    idx++;
                }

                if( lprev != NULL && rprev != NULL ) {
                    lprev->parent = ptr;
                    rprev->parent = ptr;
                }

                done = TRUE;
            }
            else {
                ExpenseNode* par = ptr->parent;
                /* capacity exceeded, split about partition_idx(e, ptr), reassign lcurr and rcurr */
                /* replace e <-> new median */
                sc = SplitAndReassign_Expense(ptr, &e, &lcurr, &rcurr, lprev, rprev);
                if( sc == FAILURE ) done = TRUE;

                lprev = lcurr;
                rprev = rcurr;

                ptr = par;
            }
        }

        if( ptr->parent == NULL ) {
            *root_ptr = ptr;
        }   
    }
    return sc;
}
ExpenseNode* ExpenseTreeSearch(expense_key_t k, ExpenseNode* root, idx_t* ret_idx) {
    ExpenseNode* ret_ptr = NULL;
    ExpenseElement e;
    // e.data = (Expense)k; // create a void expense
    e.key = k;
    idx_t idx;

    ExpenseNode* ptr = root; Boolean done = FALSE;
    while( ptr != NULL && !done ) {
        idx = PartitionIdx_Expense(e, ptr);
        if( idx > ptr->cnt ) {
            done = TRUE;
            ret_ptr = NULL;
            *ret_idx = -1;
        }
        else if( idx == ptr->cnt ) {
            ptr = ptr->children[idx];
        }
        else { /* idx < ptr->cnt */
            if( ptr->elements[idx].key == k ) {
                done = TRUE;
                ret_ptr = ptr;
                *ret_idx = idx;
            }
            else {
                ptr = ptr->children[idx];
            }
        }
    }
    return ret_ptr;
}
status_code RecDelete_Expense(idx_t del_idx, ExpenseNode* ptr, ExpenseNode* newchild, ExpenseNode **root_ptr) {
    status_code sc = SUCCESS;
    if( ptr == NULL ) {
        sc = FAILURE;
    }
    else { /* ptr != NULL */
        if( ptr->parent == NULL ) { /* root */
            if( ptr->cnt == 1 ) {
                free(ptr);
                *root_ptr = newchild;
            }
            else {
                /* delete the element */
                idx_t i = del_idx;
                ptr->children[del_idx] = newchild;
                if( newchild != NULL ) newchild->parent = ptr;
                while( i < ptr->cnt - 1 ) {
                    ptr->elements[i] = ptr->elements[i+1];
                    ptr->children[i+1] = ptr->children[i+2];
                    i++;
                }
                ptr->cnt--;
            }
        } /* end of conditional: ptr->parent == NULL */
        else { // ptr->parent != NULL
            if( ptr->cnt > MINITEMS ) {
                /* delete the element */
                idx_t i = del_idx;
                ptr->children[del_idx] = newchild;
                if( newchild != NULL ) newchild->parent = ptr;
                while( i < ptr->cnt - 1 ) {
                    ptr->elements[i] = ptr->elements[i+1];
                    ptr->children[i+1] = ptr->children[i+2];
                    i++;
                }
                ptr->cnt--;
            }
            else if( ptr->cnt == MINITEMS ) {
                /* find sibling with excess elements */
                ExpenseNode* par = ptr->parent; 
                ExpenseNode* leftsib = NULL, *rightsib = NULL;
                idx_t par_idx = PartitionIdx_Expense(ptr->elements[0], par);
                if( par_idx > 0 ) {
                    leftsib = par->children[par_idx-1];
                }
                if( par_idx < par->cnt ) {
                    rightsib = par->children[par_idx+1];
                }
                Boolean left_chosen = FALSE, right_chosen = FALSE;
                // check if left or right sibling has excess elements, if found, rotate, else implement family pool
                if( leftsib != NULL ) {
                    if( leftsib->cnt > MINITEMS ) {
                        /* borrow from left sibling - right rotation*/
                        ptr->children[del_idx + 1] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        RotateRight_Expense(leftsib, ptr, par_idx, del_idx);
                        left_chosen = TRUE;
                    }
                }
                if( !left_chosen && rightsib != NULL ) {
                    if( rightsib->cnt > MINITEMS ) {
                        /* borrow from right sibling */
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        RotateLeft_Expense(rightsib, ptr, par_idx, del_idx);
                        right_chosen = TRUE;
                    }
                }
                if( !right_chosen && !left_chosen ) { // deficit sibling
                    /* merge with sibling and parent */
                    ExpenseNode* newNode = (ExpenseNode*)malloc(sizeof(ExpenseNode));
                    ExpenseElement del_elem;
                    // assign relevant values (of left or right sibling and parent)
                    if( leftsib != NULL ) {
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        MergeWithLeft_Expense(leftsib, ptr, newNode, par, &del_elem, par_idx, del_idx);
                        sc = RecDelete_Expense(par_idx-1, par, newNode, root_ptr);
                    }
                    else if( rightsib != NULL ) {
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        MergeWithRight_Expense(rightsib, ptr, newNode, par, &del_elem, par_idx, del_idx);
                        sc = RecDelete_Expense(par_idx, par, newNode, root_ptr);
                    }    
                } /* end of conditional: deficit sibling */
                
            } /* end of conditional: ptr->cnt == MINITEMS */
           
        } /* end of conditional: ptr->parent != NULL */
    } /* end of conditional: ptr != NULL */
    return sc;
}
status_code ExpenseTreeDelete(expense_key_t k, ExpenseNode** root_ptr) {
    status_code sc = SUCCESS;
    idx_t del_idx;
    ExpenseNode *root = *root_ptr;
    ExpenseNode* ptr = ExpenseTreeSearch(k, root, &del_idx);
    if( ptr != NULL ) {
        if( isLeaf_Expense(ptr) ) {
            sc = RecDelete_Expense(del_idx, ptr, NULL, root_ptr);
        }
        else { /* is not a leaf */
            /* find max element in left subtree and replace */
            ExpenseNode* lptr = ptr->children[del_idx];
            while( !isLeaf_Expense(lptr) ) {
                lptr = lptr->children[lptr->cnt];
            }
            ExpenseElement e = lptr->elements[lptr->cnt - 1];
            ptr->elements[del_idx] = e; // bitcopying ExpenseElement
            /* delete the max element */
            sc = RecDelete_Expense(lptr->cnt-1, lptr, NULL, root_ptr);
        }
    }
    else {
        sc = FAILURE;
    }
    return sc;
}
status_code ReadExpenseData() {
    status_code sc = SUCCESS;
    FILE *file_ptr = fopen("expense_data.txt", "r");
    if( file_ptr == NULL ) {
        printf("\nError: file cannot be opened");
        sc = FAILURE;
    }
    else {
        Expense NewExpense;
        char ExpenseID[5], UserID[5], ExpenseCategory[25], ExpenseAmount[25], DateOfExpense[20];

        while( (sc == SUCCESS) && fscanf(file_ptr, "%s %s %s %s %s", ExpenseID, UserID, ExpenseCategory, ExpenseAmount, DateOfExpense) != EOF ) {
    
            NewExpense.expense_id = string_to_int(ExpenseID);
            NewExpense.user_id = string_to_int(UserID);
            NewExpense.expense_category = string_to_expenseCategory(ExpenseCategory);
            NewExpense.expense_amount = string_to_float(ExpenseAmount);
            strcpy(NewExpense.date_of_expense, DateOfExpense);

            sc = AddExpense(NewExpense);
        }
    }
    fclose(file_ptr);
    return sc;
}
ExpenseNode* ExpenseTreeSearchByExpenseID(int expense_id, ExpenseNode* root, idx_t* ret_idx) {
    ExpenseNode* ret_ptr = NULL;
    ExpenseNode* ptr = root_expense; Boolean done = FALSE;
    
    /* check all elements in the node */
    for( idx_t i = 0; i < ptr->cnt && !done; i++ ) {
        if( ptr->elements[i].data.expense_id == expense_id ) {
            done = TRUE;
            ret_ptr = ptr;
            *ret_idx = i;
        }
    }
    if( !done ) {
        /* recursively check the subtrees from left to right - with early exit if found */
        for( idx_t i = 0; i <= ptr->cnt && !done; i++ ) {
            if( ptr->children[i] != NULL ) {
                ret_ptr = ExpenseTreeSearchByExpenseID(expense_id, ptr->children[i], ret_idx);
                if( ret_ptr != NULL ) {
                    done = TRUE;
                }
            }
        }
    }
    return ret_ptr;
}

// Preorder search
ExpenseNode* ExpenseTreeSearchByUserID(int user_id, ExpenseNode* root, idx_t* ret_idx) {
    ExpenseNode* ret_ptr = NULL;
    ExpenseNode* ptr = root_expense; Boolean done = FALSE;
    
    /* check all elements in the node */
    for( idx_t i = 0; i < ptr->cnt && !done; i++ ) {
        if( (ptr->elements[i].data.user_id)/(10000) == user_id ) {
            done = TRUE;
            ret_ptr = ptr;
            *ret_idx = i;
        }
    }
    if( !done ) {
        /* recursively check the subtrees from left to right - with early exit if found */
        for( idx_t i = 0; i <= ptr->cnt && !done; i++ ) {
            if( ptr->children[i] != NULL ) {
                ret_ptr = ExpenseTreeSearchByUserID(user_id, ptr->children[i], ret_idx);
                if( ret_ptr != NULL ) {
                    done = TRUE;
                }
            }
        }
    }
    return ret_ptr;
}
expense_key_t GenerateExpenseKey(Expense e) {
    expense_key_t ret_val;
    ret_val = (1e4)*(e.user_id) + e.expense_id;
    return ret_val;
}
Boolean InorderExpense(ExpenseNode* root, int* expense_id_ptr) {
    Boolean ret_val = FALSE;
    if( root != NULL ) {
        ret_val = InorderExpense(root->children[0], expense_id_ptr);
        if( !ret_val ) {
            if( root->elements[0].key == *expense_id_ptr ) {
                *expense_id_ptr = *expense_id_ptr + 1;
            }
            else {
                ret_val = TRUE;
            }
        }
        if( !ret_val ) {
            int i = 0;
            while( i < root->cnt && !ret_val ) {
                ret_val = InorderExpense(root->children[i+1], expense_id_ptr);
                i++;
            }
        }
    }
    return ret_val;
}
ExpenseNode* MinExpKeyOfUser(int user_id, ExpenseNode* root, idx_t* ret_idx) {
    ExpenseNode* ret_ptr = NULL;
    ExpenseNode* ptr = root_expense, *prev = NULL; Boolean done = FALSE;
    expense_key_t lower_bound = (1e4)*(user_id) + 0;
    expense_key_t upper_bound = (1e4)*(user_id) + 9999;
    expense_key_t min_key = 0;
    ExpenseElement ee_lower_bound;
    ee_lower_bound.key = lower_bound;
    /* check all elements in the node */
    while( !done && ptr != NULL) {
        idx_t i = PartitionIdx_Expense(ee_lower_bound, ptr);
        if( i < ptr->cnt ) {
            if( ptr->elements[i].key <= upper_bound) {
                ret_ptr = ptr;
                *ret_idx = i;
            }
        }
        if( ptr->children[i] != NULL ) {
            ptr = ptr->children[i];
        }
        else {
            done = TRUE;
        }
    }
    return ret_ptr;
}
Boolean UserInFam(int user_id, FamilyElement fe) {
    Boolean ret_val = FALSE;
    for( idx_t i = 0; i < fe.data.family_size && !ret_val; i++ ) {
        if( fe.data.members[i] == user_id ) {
            ret_val = TRUE;
        }
    }
    return ret_val;
}
/* helper functions - expense tree structure */
status_code SplitAndReassign_Expense(
    ExpenseNode* ptr, 
    ExpenseElement *e_ptr, 
    ExpenseNode **lcptr, 
    ExpenseNode **rcptr,
    ExpenseNode *lprev,
    ExpenseNode *rprev
) {
    status_code sc = SUCCESS;
    if( ptr == NULL ) sc = FAILURE;
    else if( ptr->cnt < M - 1 ) sc = FAILURE;
    else {
        idx_t med_idx, ins_idx, rem_idx;
        Boolean done;
        ins_idx = 0; done = FALSE;
        while( !done && ins_idx < ptr->cnt ) {
            if( e_ptr->key < ptr->elements[ins_idx].key ) {
                done = TRUE;
            }
            else {
                ins_idx++;
            }
        }
        med_idx = (ptr->cnt + 1 - 1)/2;
        ExpenseElement e = *e_ptr;
        
        ExpenseNode* lcurr = (ExpenseNode*)malloc(sizeof(ExpenseNode));
        ExpenseNode* rcurr = (ExpenseNode*)malloc(sizeof(ExpenseNode));
        if( lcurr == NULL || rcurr == NULL ) {
            sc = FAILURE;
        }
        else {
            /* initialising left and right split parts, updating parent pointers of lprev and rprev */
            lcurr->cnt = M/2;
            rcurr->cnt = M/2;
            idx_t i = 0;
            while( i <= M/2 ) {
                lcurr->children[i] = NULL;
                rcurr->children[i] = NULL;
                i = i + 1;
            }
            idx_t lwrite = 0; idx_t ptr_read = 0; int left_child_offset = 0;
            int child_offset = 0;

            while( lwrite < M/2 ) {
                if( lwrite == ins_idx ) {
                    lcurr->elements[lwrite] = e; // bitcopying ExpenseElement
                    lcurr->children[lwrite + left_child_offset] = lprev;
                    child_offset = 1; 
                    left_child_offset = 1;
                    lcurr->children[lwrite + left_child_offset] = rprev;
                    if( lprev != NULL && rprev != NULL ) {
                        lprev->parent = lcurr;
                        rprev->parent = lcurr;
                    }
                    lwrite++; 
                }
                else {
                    lcurr->elements[lwrite] = ptr->elements[ptr_read]; // bitcopying ExpenseElement
                    lcurr->children[lwrite + left_child_offset] = ptr->children[ptr_read + child_offset];
                    if( ptr->children[ptr_read + child_offset] != NULL ) {
                        ptr->children[ptr_read + child_offset]->parent = lcurr;
                    }
                    lwrite++; ptr_read++;
                }
            }

            idx_t rwrite = 0; int right_child_offset = 0; 
            
            if( lwrite == ins_idx ) { // remember lwrite = med_idx
                lcurr->children[lwrite] = lprev;
                if( lprev != NULL && rprev != NULL ) {
                    lprev->parent = lcurr;
                    rprev->parent = rcurr;
                }
                rcurr->children[rwrite] = rprev;
                right_child_offset = 1;
                child_offset = 1;
            }
            else { /* propogate the new median */
                *e_ptr = ptr->elements[ptr_read];
                lcurr->children[lwrite] = ptr->children[ptr_read + child_offset];
                if( ptr->children[ptr_read + child_offset] != NULL ) {
                    ptr->children[ptr_read + child_offset]->parent = lcurr;
                }
                ptr_read++;
            }
            while( rwrite < M/2 ) {
                if( rwrite == ins_idx - (M+1)/2 ) {
                    rcurr->elements[rwrite] = e; // bitcopying ExpenseElement
                    rcurr->children[rwrite + right_child_offset] = lprev;
                    child_offset = 1;
                    right_child_offset = 1;
                    rcurr->children[rwrite + right_child_offset] = rprev;
                    rwrite++;
                    if( lprev != NULL && rprev != NULL ) {
                        lprev->parent = lcurr;
                        rprev->parent = rcurr;
                    }

                }
                else {
                    rcurr->elements[rwrite] = ptr->elements[ptr_read]; // bitcopying ExpenseElement
                    rcurr->children[rwrite + right_child_offset] = ptr->children[ptr_read + child_offset];
                    if( ptr->children[ptr_read + child_offset] != NULL ) {
                        ptr->children[ptr_read + child_offset]->parent = rcurr;
                    }
                    rwrite++; ptr_read++;
                }
            }

            *lcptr = lcurr;
            *rcptr = rcurr;
            ExpenseNode* par = ptr->parent;
            if( par != NULL ) {
                idx_t idx = PartitionIdx_Expense(e, par);
                par->children[idx] = lcurr;
            }
            free(ptr);
        }   
    }
    return sc;
}
idx_t PartitionIdx_Expense(ExpenseElement e, ExpenseNode* bptr) {
    int ret_val;
    if( bptr == NULL ) {
        ret_val = -1;
    }
    else {
        int idx = 0; Boolean done = FALSE;
        while( (idx < bptr->cnt) && !done ) {
            if( e.key <= bptr->elements[idx].key ) {  /* ie continue till bptr->elements[idx].key < e.key */
                done = TRUE;
            }
            else {
                idx = idx + 1;
            }
        }

        ret_val = idx; /* in both cases: done and !done */
    }
    return ret_val;
}
Boolean isLeaf_Expense(ExpenseNode* ptr) {
    Boolean ret_val = FALSE;
    if( ptr != NULL ) {
        if( ptr->children[0] == NULL ) {
            ret_val = TRUE;
        }
    }
    return ret_val;
}

/* delete helpers */
void RotateRight_Expense(ExpenseNode* leftsib, ExpenseNode* ptr, idx_t par_idx, idx_t del_idx) {
    ExpenseNode* par = ptr->parent;
    if( par != NULL ) {
        idx_t i = del_idx;
        while( i > 0 ) {
            ptr->elements[i] = ptr->elements[i-1];
            ptr->children[i] = ptr->children[i-1];
            i--;
        }
        ptr->elements[0] = par->elements[par_idx-1];
        ptr->children[0] = leftsib->children[leftsib->cnt];
        par->elements[par_idx-1] = leftsib->elements[leftsib->cnt - 1];
        leftsib->cnt--;
    }
}
void RotateLeft_Expense(ExpenseNode* rightsib, ExpenseNode* ptr, idx_t par_idx, idx_t del_idx) {
    ExpenseNode* par = ptr->parent;
    if( par != NULL ) {
        idx_t i = del_idx;
        while( i < ptr->cnt - 1 ) {
            ptr->elements[i] = ptr->elements[i+1];
            ptr->children[i+1] = ptr->children[i+2];
            i++;
        }
        ptr->elements[i] = par->elements[par_idx];
        par->elements[par_idx] = rightsib->elements[0];
        ptr->children[i+1] = rightsib->children[0];
        i = 0;
        while( i < rightsib->cnt - 1 ) {
            rightsib->elements[i] = rightsib->elements[i+1];
            rightsib->children[i] = rightsib->children[i+1];
            i++;
        }
        rightsib->cnt--;
    }
}
void MergeWithLeft_Expense(ExpenseNode* leftsib, ExpenseNode* ptr, ExpenseNode* newnode, ExpenseNode* par, ExpenseElement *deleptr, idx_t par_idx, idx_t del_idx) {
    newnode->cnt = leftsib->cnt + ptr->cnt + 1 - 1;
    idx_t read = 0, write = 0; int child_offset = 0;
    while( read < leftsib->cnt ) {
        newnode->elements[write] = leftsib->elements[read];
        newnode->children[write] = leftsib->children[read];
        read++; write++;
    }
    newnode->elements[write] = par->elements[par_idx-1];
    newnode->children[write] = leftsib->children[leftsib->cnt];
    write++;
    read = 0;
    while( read < ptr->cnt ) {
        if( read != del_idx ) { 
            newnode->elements[write] = ptr->elements[read];
            newnode->children[write + child_offset] = ptr->children[read + child_offset];
            write++;
        }
        else {
            newnode->children[write + child_offset] = ptr->children[read + child_offset];
            child_offset = 1;
        }
        read++;
    }
    *deleptr = par->elements[par_idx-1]; // bitcopying ExpenseElement
}
void MergeWithRight_Expense(ExpenseNode* rightsib, ExpenseNode* ptr, ExpenseNode* newnode, ExpenseNode* par, ExpenseElement *deleptr, idx_t par_idx, idx_t del_idx) {
    newnode->cnt = rightsib->cnt + ptr->cnt + 1 - 1;
    
    idx_t read = 0; idx_t write = 0; int child_offset = 0;
    while( read < ptr->cnt ) {
        if( read != del_idx ) { 
            newnode->elements[write] = ptr->elements[read];
            newnode->children[write + child_offset] = ptr->children[read + child_offset];
            write++;
        }
        else {
            newnode->children[write + child_offset] = ptr->children[read + child_offset];
            child_offset = 1;
        }
        read++;
    }
    newnode->elements[write] = par->elements[par_idx];
    newnode->children[write + child_offset] = rightsib->children[0];
    write++;
    read = 0;

    
    while( read < rightsib->cnt ) {
        newnode->elements[write] = rightsib->elements[read];
        newnode->children[write + child_offset] = rightsib->children[read + child_offset];
        read++; write++;
    }
    *deleptr = par->elements[par_idx];
}

// Preorder traversal
void FPrintAllInfo_Expense(ExpenseNode* root) {
    FILE* file_ptr;

    static int level = -1;
    level++;
    ExpenseElement *earr;
    ExpenseNode **carr;
    if( root != NULL ) {
        earr = root->elements;
        carr = root->children;
        file_ptr = fopen(EXPENSEINFOFILE, "a");
        fprintf(file_ptr, 
            "level: %d\n" 
                "\taddress: %p\n" 
                "\tparent: %p\n"
                "\tcnt: %d\n",
                level,
                root,
                root->parent,
                root->cnt
        );
        fprintf(file_ptr, "\telement.keys: {");
        for( idx_t i = 0; i < root->cnt; i++ ) {
            fprintf(file_ptr, "%d ", earr[i].key);
        }
        fprintf(file_ptr, "}\n\tchildren: {");
        for( idx_t i = 0; i <= root->cnt; i++) {
            fprintf(file_ptr, "%p ", carr[i]);
        }
        fprintf(file_ptr, "}\n");
        fclose(file_ptr);
        for( idx_t i = 0; i <= root->cnt; i++) {
            FPrintAllInfo_Expense(root->children[i]);
        }
    }
    level--;
}
void FPrintInorderTable_Expense(ExpenseNode* root) {
    FILE* file_ptr;
    file_ptr = fopen(EXPENSEINFOFILE, "a");
    if( file_ptr == NULL ) {
        printf("\nError: file cannot be opened");
    }
    else {
        if( root != NULL ) {
            FPrintInorderTable_Expense(root->children[0]);
            idx_t i = 0;
            while( i < root->cnt ) {
                fprintf(file_ptr, "UserID: %d, ExpenseID: %d, ExpenseCategory: %d, ExpenseAmount: %.2f, DateOfExpense: %s\n", 
                    root->elements[i].data.user_id,
                    root->elements[i].data.expense_id,
                    root->elements[i].data.expense_category,
                    root->elements[i].data.expense_amount,
                    root->elements[i].data.date_of_expense
                );
                fclose(file_ptr);
                FPrintInorderTable_Expense(root->children[i+1]);
                i++;
            }
        }
    }
}
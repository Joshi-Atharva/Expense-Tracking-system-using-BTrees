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
/* core wrappers */
status_code AddUser(User user) {
    status_code sc = SUCCESS;
    UserElement ue;
    ue.data = user;
    ue.key = user.user_id;
    int FamilyID = user.family_id;
    /* check if new user_id is to be generated */
    if( user.user_id == 0 ) {
        ue.key = GenerateUserId();
        if( FamilyID == 0 ) {
            FamilyID = GenerateFamilyId();
            ue.data.family_id = FamilyID;
            sc = CreateFamily(FamilyID, ue);
            sc = UserTreeInsert(ue, &root_user);
        }
        else {
            /* check if family_id exists */
            idx_t ret_idx;
            FamilyNode* fptr = FamilyTreeSearch(FamilyID, root_family, &ret_idx);
            if( fptr == NULL ) {
                /* family_id does not exist - Create New Family*/
                sc = CreateFamily(FamilyID, ue);
                sc = UserTreeInsert(ue, &root_user);
            }
            else if( fptr->elements[ret_idx].data.family_size == 4 ) {
                /* family is full */
                sc = FAILURE;
            }
            else {
                /* family exists and is not full */
                fptr->elements[ret_idx].data.members[fptr->elements[ret_idx].data.family_size] = ue.key;
                fptr->elements[ret_idx].data.family_size++;
                fptr->elements[ret_idx].data.total_family_income += ue.data.income;
                float monthly_expense[12];
                GetMonthlyUserExpense(ue.key, monthly_expense);
                /* updating family expenses */
                for(idx_t i = 0; i < 12; i++) {
                    fptr->elements[ret_idx].data.monthly_family_expense[i] += monthly_expense[i];
                }
                sc = UserTreeInsert(ue, &root_user);
            }
        }
    }
    else { /* user_id is already generated */
        /* search for already existing user */
        idx_t ret_idx;
        UserNode* uptr = UserTreeSearch(ue.key, root_user, &ret_idx);
        if( uptr != NULL ) {
            /* user_id already exists */
            sc = FAILURE;
        }
        else { /* user does not exist */
            /* check if new family is to be created */
            if( FamilyID == 0 ) {
                FamilyID = GenerateFamilyId();
                ue.data.family_id = FamilyID;
                sc = CreateFamily(FamilyID, ue);
                sc = UserTreeInsert(ue, &root_user);
            }
            else {
                /* check if family_id exists */
                idx_t ret_idx;
                FamilyNode* fptr = FamilyTreeSearch(FamilyID, root_family, &ret_idx);
                if( fptr == NULL ) {
                    /* family_id does not exist - Create New Family*/
                    sc = CreateFamily(FamilyID, ue);
                    sc = UserTreeInsert(ue, &root_user);
                }
                else if( fptr->elements[ret_idx].data.family_size == 4 ) {
                    /* family is full */
                    sc = FAILURE;
                }
                else {
                    /* family exists and is not full */
                    fptr->elements[ret_idx].data.members[fptr->elements[ret_idx].data.family_size] = ue.key;
                    fptr->elements[ret_idx].data.family_size++;
                    fptr->elements[ret_idx].data.total_family_income += ue.data.income;
                    float monthly_expense[12];
                    GetMonthlyUserExpense(ue.key, monthly_expense);
                    /* updating family expenses */
                    for(idx_t i = 0; i < 12; i++) {
                        fptr->elements[ret_idx].data.monthly_family_expense[i] += monthly_expense[i];
                    }
                    sc = UserTreeInsert(ue, &root_user);
                }
            }
        }
    }
    return sc;
}
status_code DeleteUser(int userID) {
    status_code sc = SUCCESS;
    idx_t user_ret_idx;
    UserNode* uptr = UserTreeSearch(userID, root_user, &user_ret_idx);
    if( uptr != NULL ) {
        /* user_id exists */
        idx_t family_ret_idx;
        FamilyNode* fptr = FamilyTreeSearch(uptr->elements[user_ret_idx].data.family_id, root_family, &family_ret_idx);
        if( fptr != NULL ) {
            /* family_id exists */
            idx_t i = 0;
            Boolean done = FALSE;
            while( i < fptr->elements[family_ret_idx].data.family_size && !done ) {
                if( fptr->elements[family_ret_idx].data.members[i] == userID ) {
                    done = TRUE;
                }
                else {
                    i++;
                }
            }
            /* i = index of userID in family */
            /* shifting elements */
            while( i < fptr->elements[family_ret_idx].data.family_size - 1 ) {
                fptr->elements[family_ret_idx].data.members[i] = fptr->elements[family_ret_idx].data.members[i+1];
                i++;
            }
            fptr->elements[family_ret_idx].data.family_size--;
            fptr->elements[family_ret_idx].data.total_family_income -= uptr->elements[user_ret_idx].data.income;
            if( fptr->elements[family_ret_idx].data.family_size == 0 ) {
                /* delete family node */
                sc = FamilyTreeDelete(fptr->elements[family_ret_idx].key, &root_family);
                if( sc == FAILURE ) {
                    printf("Error: Family deletion failed.\n");
                }
            } 
            else {/* updating family expenses */
                float monthly_expense[12];
                GetMonthlyUserExpense(userID, monthly_expense);
                for(idx_t i = 0; i < 12; i++) {
                    fptr->elements[family_ret_idx].data.monthly_family_expense[i] -= monthly_expense[i];
                }
            }
            /* delete all expense records of user */
            sc = DeleteUserHelper_ExpensesDelete(userID);
            if( sc == FAILURE ) {
                printf("Error: Expense deletion failed.\n");
            }
            /* delete user from user tree */
            user_key_t userkey = uptr->elements[user_ret_idx].key;
            sc = UserTreeDelete(userkey, &root_user);
            if( sc == FAILURE ) {
                printf("Error: User deletion failed.\n");
            }
        }
        else {
            /* family_id does not exist */
            sc = FAILURE;
        }
    }
    else {
        /* user_id does not exist */
        sc = FAILURE;
    }
    return sc;
}
float GetIndividualExpense(int userID, int month) {
    float monthly_expense[12];
    float categorical_expense[5];
    float total_expense = 0;
    char category[25];
    idx_t user_ret_idx;
    UserNode* uptr = UserTreeSearch(userID, root_user, &user_ret_idx);
    if( uptr != NULL ) {
        /* user_id exists */
        GetMonthlyUserExpense(userID, monthly_expense);
        for(idx_t i = 0; i < 5; i++) {
            categorical_expense[i] = 0;
        }
        GetCategoricalExpenseUser(userID, categorical_expense);
        printf("User ID: %d, Expense for month %d is %.2f\n", userID, month, monthly_expense[month-1]);

        /* category encoding */
        for( idx_t i = 0; i < 5; i++ ) {
            categorical_expense[i] = 10*categorical_expense[i] + i;
        }

        /* call sort function */
        QuickSort_float(categorical_expense, 0, 4);

        /* print user expense in descending order */
        printf("User ID: %d, Expense by category in descending order:\n", userID);
        for( idx_t i = 4; i >= 0; i-- ) {
            int cat = (int)categorical_expense[i] % 10;
            float exp = (float)((int)(categorical_expense[i] / 10));
            expenseCategory_to_string(category, cat);
            printf("Category: %s, Amount: %.2f\n", category, exp);
            total_expense += exp;
        }
        printf("Total Expense: %.2f\n", total_expense);
    }
    else {
        /* user_id does not exist */
        printf("Error: User ID %d does not exist.\n", userID);
    }
    return total_expense;
}
status_code RangedSearch(int ExpID_low, int ExpID_high, int UserID) {
    status_code sc = SUCCESS;
    if( ExpID_low > ExpID_high ) {
        printf("Error: Invalid range.\n");
        sc = FAILURE;
    }
    else {
        expense_key_t lower_bound, upper_bound;
        lower_bound = 10000*UserID + ExpID_low;
        upper_bound = 10000*UserID + ExpID_high;

        RangedSearchHelper(lower_bound, upper_bound, root_expense);
    }
    return sc;
}
status_code UpdateUser(int UserID) {
    status_code sc = SUCCESS;
    idx_t user_ret_idx;
    UserNode* uptr = UserTreeSearch(UserID, root_user, &user_ret_idx);
    if( uptr != NULL ) {
        /* user_id exists */
        /* ask field to be updated */
        int field;
        printf("Enter the field to be updated:\n");
        printf("1. User Name\n");
        printf("2. Family ID\n");
        printf("3. Income\n");
        scanf("%d", &field);
        switch(field) {
            case 1:
                printf("Enter new user name: ");
                scanf("%s", uptr->elements[user_ret_idx].data.username);
                break;
            case 2:
                printf("Enter new family ID: ");
                int newFamilyID;
                scanf("%d", &newFamilyID);
                User NewUser = uptr->elements[user_ret_idx].data;
                NewUser.family_id = newFamilyID;
                DeleteUser(UserID);
                AddUser(NewUser);
                break;
            case 3:
                printf("Enter new income: ");
                float newIncome;
                scanf("%f", &newIncome);
                idx_t family_ret_idx;
                FamilyNode* fptr = FamilyTreeSearch(uptr->elements[user_ret_idx].data.family_id, root_family, &family_ret_idx);
                if( fptr != NULL ) {
                    /* family_id exists */
                    fptr->elements[family_ret_idx].data.total_family_income -= uptr->elements[user_ret_idx].data.income;
                    fptr->elements[family_ret_idx].data.total_family_income += newIncome;
                    uptr->elements[user_ret_idx].data.income = newIncome;
                }
                else {
                    /* family_id does not exist */
                    sc = FAILURE;
                }
                break;
            default:
                printf("Invalid field.\n");
                sc = FAILURE;
        }
    }
    else {
        /* user_id does not exist */
        sc = FAILURE;
    }
    return sc;
}

    
/* core functions */
void InitialiseUserTree() {
    root_user->cnt = 0;
    root_user->parent = NULL;
    root_user->children[0] = NULL;
}
/* have to check if exists */
status_code UserTreeInsert(UserElement e, UserNode **root_ptr) {
    status_code sc = SUCCESS;
    UserNode* root = *root_ptr;
    UserNode* ptr = root;
    if( ptr == NULL ) { /* make new node */
        ptr = (UserNode*)malloc(sizeof(UserNode));
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
            ptr = ptr->children[PartitionIdx_User(e, ptr)];
        } // ptr points to the node of insertion
        
        Boolean done = FALSE;
        UserNode *lcurr = ptr, *rcurr = ptr, *lprev = NULL, *rprev = NULL, *par;
        /* bottom up process till value is stably assigned */
        while( !done ) {
            if( ptr == NULL ) {
                ptr = (UserNode*)malloc(sizeof(UserNode));
                if( ptr == NULL ) sc = FAILURE;
                else {
                    ptr->cnt = 0;
                    ptr->children[0] = NULL;
                    ptr->parent = NULL;
                }
            }
            if( ptr->cnt < M-1 ) {
                /* capacity not exceeded, carry out direct insertion */
                idx_t idx = PartitionIdx_User(e, ptr);
                ptr->cnt = ptr->cnt + 1;

                /* insertion and shifting by  rotating temporaries */
                UserElement temp, ins; // for elements array
                UserNode* temp_ptr, *ins_ptr; // for children array
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
                UserNode* par = ptr->parent;
                /* capacity exceeded, split about partition_idx(e, ptr), reassign lcurr and rcurr */
                /* replace e <-> new median */
                sc = SplitAndReassign_User(ptr, &e, &lcurr, &rcurr, lprev, rprev);
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
UserNode* UserTreeSearch(user_key_t k, UserNode* root, idx_t* ret_idx) {
    UserNode* ret_ptr = NULL;
    UserElement e;
    // e.data = (User)k; // create a void user
    e.key = k;
    idx_t idx;

    UserNode* ptr = root; Boolean done = FALSE;
    while( ptr != NULL && !done ) {
        idx = PartitionIdx_User(e, ptr);
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
status_code RecDelete_User(idx_t del_idx, UserNode* ptr, UserNode* newchild, UserNode **root_ptr) {
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
                UserNode* par = ptr->parent; 
                UserNode* leftsib = NULL, *rightsib = NULL;
                idx_t par_idx = PartitionIdx_User(ptr->elements[0], par);
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
                        RotateRight_User(leftsib, ptr, par_idx, del_idx);
                        left_chosen = TRUE;
                    }
                }
                if( !left_chosen && rightsib != NULL ) {
                    if( rightsib->cnt > MINITEMS ) {
                        /* borrow from right sibling */
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        RotateLeft_User(rightsib, ptr, par_idx, del_idx);
                        right_chosen = TRUE;
                    }
                }
                if( !right_chosen && !left_chosen ) { // deficit sibling
                    /* merge with sibling and parent */
                    UserNode* newNode = (UserNode*)malloc(sizeof(UserNode));
                    UserElement del_elem;
                    // assign relevant values (of left or right sibling and parent)
                    if( leftsib != NULL ) {
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        MergeWithLeft_User(leftsib, ptr, newNode, par, &del_elem, par_idx, del_idx);
                        sc = RecDelete_User(par_idx-1, par, newNode, root_ptr);
                    }
                    else if( rightsib != NULL ) {
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        MergeWithRight_User(rightsib, ptr, newNode, par, &del_elem, par_idx, del_idx);
                        sc = RecDelete_User(par_idx, par, newNode, root_ptr);
                    }    
                } /* end of conditional: deficit sibling */
                
            } /* end of conditional: ptr->cnt == MINITEMS */
           
        } /* end of conditional: ptr->parent != NULL */
    } /* end of conditional: ptr != NULL */
    return sc;
}
status_code UserTreeDelete(user_key_t k, UserNode** root_ptr) {
    status_code sc = SUCCESS;
    idx_t del_idx;
    UserNode *root = *root_ptr;
    UserNode* ptr = UserTreeSearch(k, root, &del_idx);
    if( ptr != NULL ) {
        if( isLeaf_User(ptr) ) {
            sc = RecDelete_User(del_idx, ptr, NULL, root_ptr);
        }
        else { /* is not a leaf */
            /* find max element in left subtree and replace */
            UserNode* lptr = ptr->children[del_idx];
            while( !isLeaf_User(lptr) ) {
                lptr = lptr->children[lptr->cnt];
            }
            UserElement e = lptr->elements[lptr->cnt - 1];
            ptr->elements[del_idx] = e; // bitcopying UserElement
            /* delete the max element */
            sc = RecDelete_User(lptr->cnt-1, lptr, NULL, root_ptr);
        }
    }
    else {
        sc = FAILURE;
    }
    return sc;
}
status_code ReadUserData() {
    extern UserNode* ulptr;
    status_code sc = SUCCESS;
    FILE *file_ptr = fopen("user_data.txt", "r");

    if( file_ptr == NULL ) {
        printf("\nError: file cannot be opened");
        sc = FAILURE;
    }
    else {
        User NewUser;
        char UserId[10], UserName[100], Income[50], FamilyIDstr[10], FamilyName[100];
        int FamilyID;
        while( (sc == SUCCESS) && fscanf(file_ptr, "%s %s %s %s", UserId, UserName, Income, FamilyIDstr) != EOF ) {

            // storing fields in NewUser
            FamilyName[0] = '\0'; // family name will be decided in AddUser() function
            NewUser.user_id = string_to_int(UserId);
            strcpy(NewUser.username, UserName);
            NewUser.income = string_to_float(Income);
            FamilyID = string_to_int(FamilyIDstr);
            NewUser.family_id = FamilyID;

            sc = AddUser(NewUser);
        }
    }
    fclose(file_ptr);
    return sc;
}
/* helper functions - data management */
void GetCategoricalExpenseUserHelper(user_key_t k, ExpenseNode* root, float* categorical_expense) {
    if( root != NULL ) {
        idx_t idx = 0;
        while( idx < root->cnt ) {
            if( root->elements[idx].data.user_id == k ) {
                categorical_expense[root->elements[idx].data.expense_category] += root->elements[idx].data.expense_amount;
            }
            GetCategoricalExpenseUserHelper(k, root->children[idx], categorical_expense);
            idx++;
        }
        GetCategoricalExpenseUserHelper(k, root->children[idx], categorical_expense);
    }
}
void GetCategoricalExpenseUser(user_key_t k, float* categorical_expense) {
    ExpenseNode* root = root_expense;
    for(idx_t i = 0; i < 5; i++) {
        categorical_expense[i] = 0;
    }
    idx_t i = 0;
    while( i < 5 ) {
        categorical_expense[i] = 0;
        i++;
    }
    GetCategoricalExpenseUserHelper(k, root, categorical_expense);
}
void RangedSearchHelper(expense_key_t lower_bound, expense_key_t upper_bound, ExpenseNode* root) {
    static int count = 0;
    if( root != NULL ) {
        
        idx_t start, end;
        RangedExtrema(root, &start, &end, lower_bound, upper_bound);
        idx_t i = start;
        
        // ouptuts values in sorted order in expense_keys, using inorder traversal
        while( i <= end ) {
            RangedSearchHelper(lower_bound, upper_bound, root->children[i]);

            count++;
            printf("%d. Expense ID: %d, User ID: %d, Category: %d, Amount: %.2f, Date: %s\n", 
                count,
                root->elements[start].data.expense_id, 
                root->elements[start].data.user_id,
                root->elements[start].data.expense_category,
                root->elements[start].data.expense_amount,
                root->elements[start].data.date_of_expense
            );
        
            i++;
        }
        if( i <= root->cnt) RangedSearchHelper(lower_bound, upper_bound, root->children[i]);
    }
}
void RangedExtrema(ExpenseNode* ptr, idx_t *start, idx_t *end, expense_key_t lower, expense_key_t upper) {
    if( ptr != NULL ) {
        /* finding min element in the range */
        idx_t i = 0;
        Boolean done = FALSE;
        *start = ptr->cnt;
        *end = -1;
        while( i < ptr->cnt && !done ) {
            if( ptr->elements[i].key >= lower ) {
                *start = i;
                done = TRUE;
            }
            else i++;
        }
        /* finding max element in the range */
        done = FALSE;
        i = ptr->cnt - 1;
        while( i >= 0 && !done ) {
            if( ptr->elements[i].key <= upper ) {
                *end = i;
                done = TRUE;
            }
            i--;
        }
    }
}
float GetMonthlyUserExpense(user_key_t k, float *monthly_expense) {
    float ret_val = 0;
    int month = 1;
    while( month <= 12 ) {
        monthly_expense[month - 1] = 0;
        month++;
    }
    ExpenseNode* root = root_expense;
    ret_val = GetMonthlyUserExpenseHelper(k, monthly_expense, root);
    return ret_val;
}
float GetMonthlyUserExpenseHelper(user_key_t k, float *monthly_expense, ExpenseNode* root) {
    float ret_val = 0;
    if( root != NULL ) {
        expense_key_t lower_bound = (1e4)*(k) + 0;
        expense_key_t upper_bound = (1e4)*(k) + 9999;

        /* full preorder traversal of the tree */
        idx_t idx = 0; int month;
        while( idx < root->cnt ) {
            if( root->elements[idx].key >= lower_bound && root->elements[idx].key <= upper_bound ) {
                ret_val += root->elements[idx].data.expense_amount;
                month = (root->elements[idx].data.date_of_expense[3] - '0')*10 + (root->elements[idx].data.date_of_expense[4] - '0');
                monthly_expense[month - 1] += root->elements[idx].data.expense_amount;

            }
            ret_val += GetMonthlyUserExpenseHelper(k, monthly_expense, root->children[idx]);
            idx++;
        }
        ret_val += GetMonthlyUserExpenseHelper(k, monthly_expense, root->children[idx]);
    }
    return ret_val;
}
user_key_t GenerateUserId() {
    user_key_t ret_val = 0;
    /* traverse the tree inorder and find the missing user_id, else return max + 1 */
    user_key_t max = 0;
    UserNode* ptr = root_user;
    while( ptr != NULL ) {
        max = ptr->elements[ptr->cnt - 1].key;
        ptr = ptr->children[ptr->cnt];
    }
    if( 1 <= max && max < 1000 ) {
        ret_val = max + 1;
    }
    else {
        /* find the first missing user_id */
        ret_val = max + 1;
    }
    return ret_val;
}

status_code DeleteUserHelper_ExpensesDelete(user_key_t userID) {
    /* delete all expense records of user by complete traversal */
    status_code sc = SUCCESS;
    ExpenseNode* ptr = root_expense;
    /* maintain a linked list of all expense records keys of user */
    LLExpNode *head = NULL, *tail = NULL;
    
    /* call recursive function for traversal and linked list creation */
    SetDeletionList(ptr, userID, &head, &tail);

    /* delete all nodes in the linked list */
    while( head != NULL ) {
        expense_key_t exp_key = head->data;
        sc = ExpenseTreeDelete(exp_key, &root_expense);
        if( sc == FAILURE ) {
            printf("Error: Expense deletion failed.\n");
        }
        /* free the linked list node */
        LLExpNode* temp = head;
        head = head->next;
        free(temp);

    }
    return sc;
}
void SetDeletionList(ExpenseNode* ptr, user_key_t userID, LLExpNode **head, LLExpNode **tail) {
    if( ptr != NULL ) {
        idx_t idx = 0;
        while( idx < ptr->cnt ) {
            if( ptr->elements[idx].data.user_id == userID ) {
                /* create a new node */
                LLExpNode *new_node = (LLExpNode*)malloc(sizeof(LLExpNode));
                new_node->data = 10000*userID + ptr->elements[idx].data.expense_id;
                new_node->next = NULL;
                if( *head == NULL ) {
                    *head = new_node;
                    *tail = new_node;
                }
                else {
                    (*tail)->next = new_node;
                    *tail = new_node;
                }
            }
            SetDeletionList(ptr->children[idx], userID, head, tail);
            idx++;
        }
        SetDeletionList(ptr->children[idx], userID, head, tail);
    }
}
/* helper functions - user tree structure */
status_code SplitAndReassign_User(
    UserNode* ptr, 
    UserElement *e_ptr, 
    UserNode **lcptr, 
    UserNode **rcptr,
    UserNode *lprev,
    UserNode *rprev
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
        UserElement e = *e_ptr;
        
        UserNode* lcurr = (UserNode*)malloc(sizeof(UserNode));
        UserNode* rcurr = (UserNode*)malloc(sizeof(UserNode));
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
                    lcurr->elements[lwrite] = e; // bitcopying UserElement
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
                    lcurr->elements[lwrite] = ptr->elements[ptr_read]; // bitcopying UserElement
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
                    rcurr->elements[rwrite] = e; // bitcopying UserElement
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
                    rcurr->elements[rwrite] = ptr->elements[ptr_read]; // bitcopying UserElement
                    rcurr->children[rwrite + right_child_offset] = ptr->children[ptr_read + child_offset];
                    if( ptr->children[ptr_read + child_offset] != NULL ) {
                        ptr->children[ptr_read + child_offset]->parent = rcurr;
                    }
                    rwrite++; ptr_read++;
                }
            }

            *lcptr = lcurr;
            *rcptr = rcurr;
            UserNode* par = ptr->parent;
            if( par != NULL ) {
                idx_t idx = PartitionIdx_User(e, par);
                par->children[idx] = lcurr;
            }
            free(ptr);
        }   
    }
    return sc;
}
idx_t PartitionIdx_User(UserElement e, UserNode* bptr) {
    int ret_val;
    if( bptr == NULL ) {
        ret_val = -1;
    }
    else {
        int idx = 0; Boolean done = FALSE;
        while( (idx < bptr->cnt) && !done ) {
            if(e.key <= bptr->elements[idx].key ) {  
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
Boolean isLeaf_User(UserNode* ptr) {
    Boolean ret_val = FALSE;
    if( ptr != NULL ) {
        if( ptr->children[0] == NULL ) {
            ret_val = TRUE;
        }
    }
    return ret_val;
}

/* delete helpers */
void RotateRight_User(UserNode* leftsib, UserNode* ptr, idx_t par_idx, idx_t del_idx) {
    UserNode* par = ptr->parent;
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
void RotateLeft_User(UserNode* rightsib, UserNode* ptr, idx_t par_idx, idx_t del_idx) {
    UserNode* par = ptr->parent;
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
void MergeWithLeft_User(UserNode* leftsib, UserNode* ptr, UserNode* newnode, UserNode* par, UserElement *deleptr, idx_t par_idx, idx_t del_idx) {
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
    *deleptr = par->elements[par_idx-1]; // bitcopying UserElement
}
void MergeWithRight_User(UserNode* rightsib, UserNode* ptr, UserNode* newnode, UserNode* par, UserElement *deleptr, idx_t par_idx, idx_t del_idx) {
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
void FPrintAllInfo_User(UserNode* root) {
    FILE* file_ptr;

    static int level = -1;
    level++;
    UserElement *earr;
    UserNode **carr;
    if( root != NULL ) {
        earr = root->elements;
        carr = root->children;
        file_ptr = fopen(USERINFOFILE, "a");
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
            FPrintAllInfo_User(root->children[i]);
        }
    }
    level--;
}
// Inorder Traversal table
void FPrintInorderTable_User(UserNode* root) {
    FILE* file_ptr;
    file_ptr = fopen(USERINFOFILE, "a");
    if( file_ptr == NULL ) {
        printf("\nError: file cannot be opened");
    }
    else {
        if( root != NULL ) {
            FPrintInorderTable_User(root->children[0]);
            idx_t i = 0;
            while( i < root->cnt ) {
                fprintf(file_ptr, "UserID: %d\tUserName: %s \tIncome: %.2f\tFamilyID: %d\n", 
                    root->elements[i].key, 
                    root->elements[i].data.username, 
                    root->elements[i].data.income,
                    root->elements[i].data.family_id
                );
                fclose(file_ptr);
                FPrintInorderTable_User(root->children[i+1]);
                i++;
            }
        }
    }
}
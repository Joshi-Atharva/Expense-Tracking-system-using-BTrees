#include <stdio.h>
#include <stdlib.h>
#include "FamilyCrud.h"
#include "UserCrud.h"
#include "ExpenseCrud.h"
#include "utils.h"
#include <string.h>
extern UserNode* root_user;
extern FamilyNode* root_family;

/* core wrappers */
status_code DeleteFamily(int familyID) {
    status_code sc = SUCCESS;
    idx_t family_ret_idx;
    FamilyNode* fptr = FamilyTreeSearch(familyID, root_family, &family_ret_idx);
    if( fptr != NULL ) {
        /* family_id exists */
        Boolean done = FALSE;
        while( !done && sc == SUCCESS ) {
            if( fptr->elements[family_ret_idx].data.family_size == 1 ) {
                sc = DeleteUser(fptr->elements[family_ret_idx].data.members[0]);
                if( sc == FAILURE ) {
                    printf("Error: User deletion failed for userid: %d.\n", fptr->elements[family_ret_idx].data.members[0]);
                }
                done = TRUE;
            }
            else {
                /* delete user from user tree */
                user_key_t userkey = fptr->elements[family_ret_idx].data.members[0];
                sc = DeleteUser(userkey);
                if( sc == FAILURE ) {
                    printf("Error: User deletion failed for userid: %d.\n", userkey);
                }
            }
        }
    }
    else {
        /* family_id does not exist */
        sc = FAILURE;
    }
    return sc;
}
float GetTotalFamilyExpense(int familyID) {
    float total_expense = 0;
    idx_t family_ret_idx;
    float deficit = 0;
    FamilyNode* fptr = FamilyTreeSearch(familyID, root_family, &family_ret_idx);
    if( fptr != NULL ) {
        /* family_id exists */
        for(idx_t i = 0; i < 12; i++) {
            total_expense += fptr->elements[family_ret_idx].data.monthly_family_expense[i];
        }
        deficit = fptr->elements[family_ret_idx].data.total_family_income - total_expense;
    }
    else {
        /* family_id does not exist */
        total_expense = -1;
        deficit = 0;
    }
    printf("\nTotal expense: %f\n", total_expense);
    printf("\nincome-expense deficit = %f\n", deficit);
    return total_expense;
}
void GetCategoricalExpenseFamily(int familyID, expenseCategory cat) {
    float user_expense[5];
    float family_expense[5];
    float cat_user_to_expense[4];
    idx_t i = 0;
    while( i < 5 ) {
        user_expense[i] = 0;
        family_expense[i] = 0;
        i++;
    }
    char category[25];
    idx_t family_ret_idx;
    FamilyNode* fptr = FamilyTreeSearch(familyID, root_family, &family_ret_idx);
    if( fptr != NULL ) {
        /* family_id exists */
        for(idx_t i = 0; i < fptr->elements[family_ret_idx].data.family_size; i++) {
            user_key_t userID = fptr->elements[family_ret_idx].data.members[i];
            GetCategoricalExpenseUser(userID, user_expense);
            for(idx_t j = 0; j < 5; j++) {
                family_expense[j] += user_expense[j];
            }
            cat_user_to_expense[i] = user_expense[cat];
        }
        /* print family expense */
        printf("Family ID: %d\n", familyID);
        expenseCategory_to_string(category, cat);
        printf("Family Expense Category: %s\n", category);
        printf("Family Expense Amount: %.2f\n", family_expense[cat]);

        /* iterate through user expense, multiply each entry by 10 and encode its user number in family */
        for( idx_t i = 0; i < fptr->elements[family_ret_idx].data.family_size; i++ ) {
            cat_user_to_expense[i] = 10*cat_user_to_expense[i] + i;
        } 
        /* sort */
        QuickSort_float(cat_user_to_expense, 0, fptr->elements[family_ret_idx].data.family_size - 1);

        /* print user expense */
        for( idx_t i = 0; i < 4; i++) {
            int user_no = ((int)cat_user_to_expense[i]) % 10;
            float user_exp = (float)((int)(cat_user_to_expense[i] / 10));
            printf("User No. %d spent %f\n", user_no, user_exp);
        }
    }
    else {
        /* family_id does not exist */
        printf("Error: Family ID %d does not exist.\n", familyID);
    }
}
status_code UpdateFamily(int familyID) {
    status_code sc = SUCCESS;
    idx_t family_ret_idx;
    char NewName[50];
    FamilyNode* fptr = FamilyTreeSearch(familyID, root_family, &family_ret_idx);
    if( fptr != NULL ) {
        /* ask the field to be updated */
        int field;
        printf("Enter the field to be updated:\n");
        printf("1. Family Name\n");
        scanf("%d", &field);
        switch(field) {
            case 1:
                printf("Enter new family name: ");
                scanf("%s", NewName);
                strcpy(fptr->elements[family_ret_idx].data.family_name, NewName);
                break;
            default:
                printf("Invalid field.\n");
                sc = FAILURE;
        }
    }
    else {
        /* family_id does not exist */
        sc = FAILURE;
    }
    return sc;
}

/* core functions */
void InitialiseFamilyTree() {
    root_family->cnt = 0;
    root_family->parent = NULL;
    root_family->children[0] = NULL;
}
/* have to check if exists */
status_code FamilyTreeInsert(FamilyElement e, FamilyNode **root_ptr) {
    status_code sc = SUCCESS;
    FamilyNode* root = *root_ptr;
    FamilyNode* ptr = root;
    if( ptr == NULL ) { /* make new node */
        ptr = (FamilyNode*)malloc(sizeof(FamilyNode));
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
            ptr = ptr->children[PartitionIdx_Family(e, ptr)];
        } // ptr points to the node of insertion
        
        Boolean done = FALSE;
        FamilyNode *lcurr = ptr, *rcurr = ptr, *lprev = NULL, *rprev = NULL, *par;
        /* bottom up process till value is stably assigned */
        while( !done ) {
            if( ptr == NULL ) {
                ptr = (FamilyNode*)malloc(sizeof(FamilyNode));
                if( ptr == NULL ) sc = FAILURE;
                else {
                    ptr->cnt = 0;
                    ptr->children[0] = NULL;
                    ptr->parent = NULL;
                }
            }
            if( ptr->cnt < M-1 ) {
                /* capacity not exceeded, carry out direct insertion */
                idx_t idx = PartitionIdx_Family(e, ptr);
                ptr->cnt = ptr->cnt + 1;

                /* insertion and shifting by  rotating temporaries */
                FamilyElement temp, ins; // for elements array
                FamilyNode* temp_ptr, *ins_ptr; // for children array
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
                FamilyNode* par = ptr->parent;
                /* capacity exceeded, split about partition_idx(e, ptr), reassign lcurr and rcurr */
                /* replace e <-> new median */
                sc = SplitAndReassign_Family(ptr, &e, &lcurr, &rcurr, lprev, rprev);
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
FamilyNode* FamilyTreeSearch(family_key_t k, FamilyNode* root, idx_t* ret_idx) {
    FamilyNode* ret_ptr = NULL;
    FamilyElement e;
    // e.data = (Family)k; // create a void family
    e.key = k;
    idx_t idx;

    FamilyNode* ptr = root; Boolean done = FALSE;
    while( ptr != NULL && !done ) {
        idx = PartitionIdx_Family(e, ptr);
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
status_code RecDelete_Family(idx_t del_idx, FamilyNode* ptr, FamilyNode* newchild, FamilyNode **root_ptr) {
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
                FamilyNode* par = ptr->parent; 
                FamilyNode* leftsib = NULL, *rightsib = NULL;
                idx_t par_idx = PartitionIdx_Family(ptr->elements[0], par);
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
                        RotateRight_Family(leftsib, ptr, par_idx, del_idx);
                        left_chosen = TRUE;
                    }
                }
                if( !left_chosen && rightsib != NULL ) {
                    if( rightsib->cnt > MINITEMS ) {
                        /* borrow from right sibling */
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        RotateLeft_Family(rightsib, ptr, par_idx, del_idx);
                        right_chosen = TRUE;
                    }
                }
                if( !right_chosen && !left_chosen ) { // deficit sibling
                    /* merge with sibling and parent */
                    FamilyNode* newNode = (FamilyNode*)malloc(sizeof(FamilyNode));
                    FamilyElement del_elem;
                    // assign relevant values (of left or right sibling and parent)
                    if( leftsib != NULL ) {
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        MergeWithLeft_Family(leftsib, ptr, newNode, par, &del_elem, par_idx, del_idx);
                        sc = RecDelete_Family(par_idx-1, par, newNode, root_ptr);
                    }
                    else if( rightsib != NULL ) {
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        MergeWithRight_Family(rightsib, ptr, newNode, par, &del_elem, par_idx, del_idx);
                        sc = RecDelete_Family(par_idx, par, newNode, root_ptr);
                    }    
                } /* end of conditional: deficit sibling */
                
            } /* end of conditional: ptr->cnt == MINITEMS */
           
        } /* end of conditional: ptr->parent != NULL */
    } /* end of conditional: ptr != NULL */
    return sc;
}
status_code FamilyTreeDelete(user_key_t k, FamilyNode** root_ptr) {
    status_code sc = SUCCESS;
    idx_t del_idx;
    FamilyNode *root = *root_ptr;
    FamilyNode* ptr = FamilyTreeSearch(k, root, &del_idx);
    if( ptr != NULL ) {
        if( isLeaf_Family(ptr) ) {
            sc = RecDelete_Family(del_idx, ptr, NULL, root_ptr);
        }
        else { /* is not a leaf */
            /* find max element in left subtree and replace */
            FamilyNode* lptr = ptr->children[del_idx];
            while( !isLeaf_Family(lptr) ) {
                lptr = lptr->children[lptr->cnt];
            }
            FamilyElement e = lptr->elements[lptr->cnt - 1];
            ptr->elements[del_idx] = e; // bitcopying FamilyElement
            /* delete the max element */
            sc = RecDelete_Family(lptr->cnt-1, lptr, NULL, root_ptr);
        }
    }
    else {
        sc = FAILURE;
    }
    return sc;
}

/* helper functions - data management */
void GenerateFamilyName(char* username, char* FamilyName) {
    int i = 0;
    while( username[i] != '\0' && i < NAME_SIZE - 10 ) {
        FamilyName[i] = username[i];
        i++;
    }
    char suffix[10] = "'s family";
    int j = 0;
    while( suffix[j] != '\0' ) {
        FamilyName[i] = suffix[j];
        i++;
        j++;
    }
    FamilyName[i] = '\0';
}
family_key_t GenerateFamilyId() {
    family_key_t ret_val = 0;
    /* traverse the tree inorder and find the missing family_id, else return max + 1 */
    family_key_t max = 0;
    FamilyNode* ptr = root_family;
    while( ptr != NULL ) {
        max = ptr->elements[ptr->cnt - 1].key;
        ptr = ptr->children[ptr->cnt];
    }
    if( 1 <= max && max <= 1000 ) {
        ret_val = max + 1;
    }
    else {
        /* find the first missing family_id */
        ret_val = max + 1;
    }
    return ret_val;
}
status_code CreateFamily(family_key_t FamilyID, UserElement ue) {
    status_code sc = SUCCESS;
    FamilyElement fe;
    fe.data.family_id = FamilyID;
    GenerateFamilyName(ue.data.username, fe.data.family_name);
    fe.data.members[0] = ue.key;
    fe.data.family_size = 1;
    fe.data.total_family_income = ue.data.income;
    for(idx_t i = 0; i < 12; i++) fe.data.monthly_family_expense[i] = 0;

    fe.key = FamilyID;
    sc = FamilyTreeInsert(fe, &root_family);
    return sc;
}
/* helper functions */
status_code SplitAndReassign_Family(
    FamilyNode* ptr, 
    FamilyElement *e_ptr, 
    FamilyNode **lcptr, 
    FamilyNode **rcptr,
    FamilyNode *lprev,
    FamilyNode *rprev
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
        FamilyElement e = *e_ptr;
        
        FamilyNode* lcurr = (FamilyNode*)malloc(sizeof(FamilyNode));
        FamilyNode* rcurr = (FamilyNode*)malloc(sizeof(FamilyNode));
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
                    lcurr->elements[lwrite] = e; // bitcopying FamilyElement
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
                    lcurr->elements[lwrite] = ptr->elements[ptr_read]; // bitcopying FamilyElement
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
                    rcurr->elements[rwrite] = e; // bitcopying FamilyElement
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
                    rcurr->elements[rwrite] = ptr->elements[ptr_read]; // bitcopying FamilyElement
                    rcurr->children[rwrite + right_child_offset] = ptr->children[ptr_read + child_offset];
                    if( ptr->children[ptr_read + child_offset] != NULL ) {
                        ptr->children[ptr_read + child_offset]->parent = rcurr;
                    }
                    rwrite++; ptr_read++;
                }
            }

            *lcptr = lcurr;
            *rcptr = rcurr;
            FamilyNode* par = ptr->parent;
            if( par != NULL ) {
                idx_t idx = PartitionIdx_Family(e, par);
                par->children[idx] = lcurr;
            }
            free(ptr);
        }   
    }
    return sc;
}
idx_t PartitionIdx_Family(FamilyElement e, FamilyNode* bptr) {
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
Boolean isLeaf_Family(FamilyNode* ptr) {
    Boolean ret_val = FALSE;
    if( ptr != NULL ) {
        if( ptr->children[0] == NULL ) {
            ret_val = TRUE;
        }
    }
    return ret_val;
}
void RotateRight_Family(FamilyNode* leftsib, FamilyNode* ptr, idx_t par_idx, idx_t del_idx) {
    FamilyNode* par = ptr->parent;
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
void RotateLeft_Family(FamilyNode* rightsib, FamilyNode* ptr, idx_t par_idx, idx_t del_idx) {
    FamilyNode* par = ptr->parent;
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
void MergeWithLeft_Family(FamilyNode* leftsib, FamilyNode* ptr, FamilyNode* newnode, FamilyNode* par, FamilyElement *deleptr, idx_t par_idx, idx_t del_idx) {
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
    *deleptr = par->elements[par_idx-1]; // bitcopying FamilyElement
}
void MergeWithRight_Family(FamilyNode* rightsib, FamilyNode* ptr, FamilyNode* newnode, FamilyNode* par, FamilyElement *deleptr, idx_t par_idx, idx_t del_idx) {
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
void FPrintAllInfo_Family(FamilyNode* root) {
    FILE* file_ptr;

    static int level = -1;
    level++;
    FamilyElement *earr;
    FamilyNode **carr;
    if( root != NULL ) {
        earr = root->elements;
        carr = root->children;
        file_ptr = fopen(FAMILYINFOFILE, "a");
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
            FPrintAllInfo_Family(root->children[i]);
        }
    }
    level--;
}

void FPrintInorderTable_Family(FamilyNode* root) {
    FILE* file_ptr;
    file_ptr = fopen(FAMILYINFOFILE, "a");
    if( file_ptr == NULL ) {
        printf("\nError: file cannot be opened");
    }
    else {
        if( root != NULL ) {
            FPrintInorderTable_Family(root->children[0]);
            idx_t i = 0;
            while( i < root->cnt ) {
                fprintf(file_ptr, 
                    "\nFamily ID: %d\n"
                    "\tFamily Name: %s\n"
                    "\tFamily Size: %d\n"
                    "\tTotal Family Income: %.2f\n",
                    root->elements[i].data.family_id,
                    root->elements[i].data.family_name,
                    root->elements[i].data.family_size,
                    root->elements[i].data.total_family_income
                );
                fprintf(file_ptr, "\tFamily Members: {");
                for( idx_t j = 0; j < root->elements[i].data.family_size; j++ ) {
                    fprintf(file_ptr, "%d ", root->elements[i].data.members[j]);
                }
                fprintf(file_ptr, "}\n");
                fprintf(file_ptr, "\tMonthly Family Expense: {");
                for( idx_t j = 0; j < 12; j++ ) {
                    fprintf(file_ptr, "%.2f ", root->elements[i].data.monthly_family_expense[j]);
                }
                fprintf(file_ptr, "}\n");
                fclose(file_ptr);
                FPrintInorderTable_Family(root->children[i+1]);
                i++;
            }
        }
    }
}
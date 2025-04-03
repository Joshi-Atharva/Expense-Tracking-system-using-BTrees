/* B+ tree implementation */

#include <stdio.h>
#include <stdlib.h>
#include "BTree.h"

void InitialiseProgram() {
    FILE* file_ptr = fopen("Logfile.txt", "w");
    fclose(file_ptr);
    FILE* sym_ptr = fopen("SymTable.txt", "w");
    fclose(sym_ptr);
    file_ptr = fopen("log.txt", "w");
    fclose(file_ptr);
}


/* helper functions */
void RotateRight(BTreeNode* leftsib, BTreeNode* ptr, idx_t par_idx, idx_t del_idx) {
    BTreeNode* par = ptr->parent;
    if( par != NULL ) {
        idx_t i = del_idx;
        while( i > 0 ) {
            ptr->elements[i] = ptr->elements[i-1];
            ptr->children[i+1] = ptr->children[i];
            i--;
        }
        ptr->elements[0] = par->elements[par_idx-1];
        par->elements[par_idx-1] = leftsib->elements[leftsib->cnt - 1];
        leftsib->cnt--;
        ptr->cnt++;
    }
}
void RotateLeft(BTreeNode* rightsib, BTreeNode* ptr, idx_t par_idx, idx_t del_idx) {
    BTreeNode* par = ptr->parent;
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
        ptr->cnt++;
    }
}
void MergeWithLeft(BTreeNode* leftsib, BTreeNode* ptr, BTreeNode* newnode, BTreeNode* par, element_t *deleptr, idx_t par_idx, idx_t del_idx) {
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
    *deleptr = par->elements[par_idx-1]; // bitcopying element_t
}
void MergeWithRight(BTreeNode* rightsib, BTreeNode* ptr, BTreeNode* newnode, BTreeNode* par, element_t *deleptr, idx_t par_idx, idx_t del_idx) {
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

Boolean isLeaf(BTreeNode* ptr) {
    Boolean ret_val = FALSE;
    if( ptr != NULL ) {
        if( ptr->children[0] == NULL ) {
            ret_val = TRUE;
        }
    }
    return ret_val;
}
idx_t PartitionIdx(element_t e, BTreeNode* bptr) {
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
idx_t MedianIndex(BTreeNode* ptr, element_t *e_ptr) {
    int n = ptr->cnt;
    int ret_val = (n-1)/2; // ceil(n/2) = floor((n-1)/2)
    e_ptr->data = (ptr->elements[ret_val]).data;
    e_ptr->key = (ptr->elements[ret_val]).key;
    return ret_val;
}
// splits ptr into two, reassigns e 
status_code SplitAndReassign(
    BTreeNode* ptr, 
    element_t *e_ptr, 
    BTreeNode **lcptr, 
    BTreeNode **rcptr,
    BTreeNode *lprev,
    BTreeNode *rprev
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
        element_t e = *e_ptr;
        
        BTreeNode* lcurr = (BTreeNode*)malloc(sizeof(BTreeNode));
        BTreeNode* rcurr = (BTreeNode*)malloc(sizeof(BTreeNode));
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
                    lcurr->elements[lwrite] = e; // bitcopying element_t
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
                    lcurr->elements[lwrite] = ptr->elements[ptr_read]; // bitcopying element_t
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
                    rcurr->elements[rwrite] = e; // bitcopying element_t
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
                    rcurr->elements[rwrite] = ptr->elements[ptr_read]; // bitcopying element_t
                    rcurr->children[rwrite + right_child_offset] = ptr->children[ptr_read + child_offset];
                    if( ptr->children[ptr_read + child_offset] != NULL ) {
                        ptr->children[ptr_read + child_offset]->parent = rcurr;
                    }
                    rwrite++; ptr_read++;
                }
            }

            *lcptr = lcurr;
            *rcptr = rcurr;
            BTreeNode* par = ptr->parent;
            if( par != NULL ) {
                idx_t idx = PartitionIdx(e, par);
                par->children[idx] = lcurr;
            }
            free(ptr);
        }   
    }
    return sc;
}

/* User-visible templates */
void InitialiseBTree(BTreeNode* root) {
    root->cnt = 0;
    root->parent = NULL;
    root->children[0] = NULL;
}
status_code BTreeInsert(element_t e, BTreeNode **root_ptr) {
    status_code sc = SUCCESS;
    BTreeNode* root = *root_ptr;
    BTreeNode* ptr = root;
    if( ptr == NULL ) { /* make new node */
        ptr = (BTreeNode*)malloc(sizeof(BTreeNode));
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
            ptr = ptr->children[PartitionIdx(e, ptr)];
        } // ptr points to the node of insertion
        
        Boolean done = FALSE;
        BTreeNode *lcurr = ptr, *rcurr = ptr, *lprev = NULL, *rprev = NULL, *par;
        /* bottom up process till value is stably assigned */
        while( !done ) {
            if( ptr == NULL ) {
                ptr = (BTreeNode*)malloc(sizeof(BTreeNode));
                if( ptr == NULL ) sc = FAILURE;
                else {
                    ptr->cnt = 0;
                    ptr->children[0] = NULL;
                    ptr->parent = NULL;
                }
            }
            if( ptr->cnt < M-1 ) {
                /* capacity not exceeded, carry out direct insertion */
                idx_t idx = PartitionIdx(e, ptr);
                ptr->cnt = ptr->cnt + 1;

                /* insertion and shifting by  rotating temporaries */
                element_t temp, ins; // for elements array
                BTreeNode* temp_ptr, *ins_ptr; // for children array
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
                BTreeNode* par = ptr->parent;
                /* capacity exceeded, split about partition_idx(e, ptr), reassign lcurr and rcurr */
                /* replace e <-> new median */
                sc = SplitAndReassign(ptr, &e, &lcurr, &rcurr, lprev, rprev);
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

status_code RecDelete(idx_t del_idx, BTreeNode* ptr, BTreeNode* newchild, BTreeNode **root_ptr) {
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
                BTreeNode* par = ptr->parent; 
                BTreeNode* leftsib = NULL, *rightsib = NULL;
                idx_t par_idx = PartitionIdx(ptr->elements[0], par);
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
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        RotateRight(leftsib, ptr, par_idx, del_idx);
                        left_chosen = TRUE;
                    }
                }
                if( !left_chosen && rightsib != NULL ) {
                    if( rightsib->cnt > MINITEMS ) {
                        /* borrow from right sibling */
                        ptr->children[del_idx+1] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        RotateLeft(rightsib, ptr, par_idx, del_idx);
                        right_chosen = TRUE;
                    }
                }
                if( !right_chosen && !left_chosen ) { // deficit sibling
                    /* merge with sibling and parent */
                    BTreeNode* newNode = (BTreeNode*)malloc(sizeof(BTreeNode));
                    element_t del_elem;
                    // assign relevant values (of left or right sibling and parent)
                    if( leftsib != NULL ) {
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        MergeWithLeft(leftsib, ptr, newNode, par, &del_elem, par_idx, del_idx);
                        sc = RecDelete(par_idx-1, par, newNode, root_ptr);
                    }
                    else if( rightsib != NULL ) {
                        ptr->children[del_idx] = newchild;
                        if( newchild != NULL ) newchild->parent = ptr;
                        MergeWithRight(rightsib, ptr, newNode, par, &del_elem, par_idx, del_idx);
                        sc = RecDelete(par_idx, par, newNode, root_ptr);
                    }    
                } /* end of conditional: deficit sibling */
                
            } /* end of conditional: ptr->cnt == MINITEMS */
           
        } /* end of conditional: ptr->parent != NULL */
    } /* end of conditional: ptr != NULL */
    return sc;
}

BTreeNode* BTreeSearch(key_type k, BTreeNode* root, idx_t* ret_idx) {
    BTreeNode* ret_ptr = NULL;
    element_t e;
    e.data = (data_t)k;
    e.key = k;
    idx_t idx;

    BTreeNode* ptr = root; Boolean done = FALSE;
    while( ptr != NULL && !done ) {
        idx = PartitionIdx(e, ptr);
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
status_code BTreeDelete(key_type k, BTreeNode** root_ptr) {
    status_code sc = SUCCESS;
    idx_t del_idx;
    BTreeNode *root = *root_ptr;
    BTreeNode* ptr = BTreeSearch(k, root, &del_idx);
    if( ptr != NULL ) {
        if( isLeaf(ptr) ) {
            sc = RecDelete(del_idx, ptr, NULL, root_ptr);
        }
        else if( ptr == *root_ptr ) {
            if( ptr->cnt <= MINITEMS ) {
                /* special case */
            }
        }
        else {
            sc = RecDelete(del_idx, ptr, ptr->children[del_idx], root_ptr);
        }
    }
    else {
        sc = FAILURE;
    }
    return sc;
}


/* print functions */
void PrintBTree(BTreeNode* root) {
    static int level = -1;
    level++;
    if( root != NULL ) {
        idx_t i = 0;
        printf("\n\tlevel %d (key, value): \n", level); 
        while( i < root->cnt ) {
            printf("\t\t(%d, %d) ", root->elements[i].data, root->elements[i].key);
            i = i + 1;
        }
        printf("\n");

        i = 0;
        while( i <= root->cnt ) {
            PrintBTree(root->children[i]);
            i = i + 1;
        }
    }
    level--;
}

// Preorder traversal
void FPrintBTree(BTreeNode* root) {
    static int level = -1;
    FILE* file_ptr;
    level++;
    if( root != NULL ) {
        idx_t i = 0;
        file_ptr = fopen("Logfile.txt", "a");
        fprintf(file_ptr, "\n\tlevel %d (key, value): \n", level); 
        while( i < root->cnt ) {
            fprintf(file_ptr, "\t\t(%d, %d) ", root->elements[i].data, root->elements[i].key);
            i = i + 1;
        }
        fprintf(file_ptr, "\n");
        fclose(file_ptr);
        i = 0;
        while( i <= root->cnt ) {
            FPrintBTree(root->children[i]);
            i = i + 1;
        }
    }
    level--;
}
void PrintStatus(const char* Opcode, status_code sc) {
    printf("Operation: %s Status: %s\n", Opcode, (sc == SUCCESS)?"successful":"failed");
}

// Preorder traversal
void FPrintAllInfo(BTreeNode* root) {
    FILE* file_ptr;

    static int level = -1;
    level++;
    element_t *earr;
    BTreeNode **carr;
    if( root != NULL ) {
        earr = root->elements;
        carr = root->children;
        file_ptr = fopen("SymTable.txt", "a");
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
            FPrintAllInfo(root->children[i]);
        }
    }
    level--;
}


    

#include <stdio.h>
#include <stdlib.h>
#include "BTree.h"

int main() {
    InitialiseProgram();
    // int arr[] = {1, 7, 6, 2, 11, 4, 8, 13, 10, 5, 19, 9, 18, 24, 3, 12, 14, 20, 21};
    int arr[] = {20, 10, 30, 40, 50, 5, 15, 25, 35, 45, 55, 1, 8, 12, 18, 22, 28, 32, 38, 42, 48, 52, 58, 3, 6, 16, 26, 46, 56, 60};
    int n = 30;
    element_t* e_arr = (element_t*)malloc(sizeof(element_t)*n);
    idx_t i = 0; element_t temp;
    while( i < n ) {
        temp.data = (data_t)arr[i];
        temp.key = (key_type)arr[i];
        e_arr[i] = temp; // bitcopying element_t

        i = i + 1;
    }

    BTreeNode* root = (BTreeNode*)malloc(sizeof(BTreeNode));
    InitialiseBTree(root);

    i = 0; status_code sc;
    FILE* file_ptr;
    while( i < n ) {
        file_ptr = fopen("Logfile.txt", "a");
        sc = BTreeInsert(e_arr[i], &root); PrintStatus("BTreeInsert()", sc);
        fprintf(file_ptr, "after insertion of (%d, %d):", e_arr[i].data, e_arr[i].key);
        fclose(file_ptr);
        FPrintBTree(root);
        i = i + 1;
    }

    FILE* log_ptr = fopen("log.txt", "a");
    int choice; Boolean exit = FALSE;

    // 1:
    element_t e;

    // 2:
    BTreeNode* ptr;

    while( !exit ) {
        printf("from following options:\n"
            "\t1: Insert\n"
            "\t2: Search\n"
            "\t3: Print/Fprint\n"
            "\t4: Delete\n"
            "\t0: Exit\n"
            "Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                printf("Enter data for insertion: ");
                scanf("%d", &e.data);
                e.key = (key_type)e.data;
                BTreeInsert(e, &root);
                fprintf(log_ptr, "Op: Insert(%d, %d)\n", e.data, e.key);
                break;
            case 2:
                printf("Enter key to be searched: ");
                scanf("%d", &e.key);
                e.data = (data_t)e.key;
                ptr = BTreeSearch(e.key, root, &i);
                if( ptr != NULL ) {
                    printf("Address: %p, data: %d, key: %d, idx: %d\n", ptr, ptr->elements[i].data, ptr->elements[i].key, i);
                }
                else {
                    printf("Not found\n");
                }
                fprintf(log_ptr, "Op: Search(%d, %d)\n", e.data, e.key);
                break;
            case 3:
                FPrintAllInfo(root);
                break;
            case 4:
                printf("Enter key to be deleted: ");
                scanf("%d", &e.key);
                sc = BTreeDelete(e.key, &root);
                PrintStatus("BTreeDelete()", sc);
                fprintf(log_ptr, "Op: Delete(%d, %d)\n", e.data, e.key);
                break;
            case 0:
                exit = TRUE;
                break;
            default:
                printf("Invalid input. Enter again\n");

        }
    }
    fclose(log_ptr);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "FamilyCrud.h"
#include "ExpenseCrud.h"
#include "UserCrud.h"
extern UserNode* root_user;
extern FamilyNode* root_family;
extern ExpenseNode* root_expense;
void InitialiseProgram() {
    root_user = (UserNode*)malloc(sizeof(UserNode));
    root_family = (FamilyNode*)malloc(sizeof(FamilyNode));
    root_expense = (ExpenseNode*)malloc(sizeof(ExpenseNode));
    FILE* file_ptr = fopen(USERINFOFILE, "w");
    if( file_ptr != NULL ) fclose(file_ptr);
    file_ptr = fopen(FAMILYINFOFILE, "w");
    if( file_ptr != NULL ) fclose(file_ptr);
    file_ptr = fopen(EXPENSEINFOFILE, "w");
    if( file_ptr != NULL ) fclose(file_ptr);
}
int string_to_int(char s[]) {
    int i = 0;
    int ans = 0;
    while (s[i] != '\0') {
        ans = 10*ans + (int)(s[i] - 48);
        i = i + 1;
    }
    return ans;
}
float string_to_float(char s[]) {
    // pithole: saves with some extra digits after decimal point following the least significant digits
    int i = 0;
    float whole = 0;
    float fraction = 0;
    float p_10 = 10;
    int decimal_point_flag = 0;
    if( s[0] == '.' ) {
        decimal_point_flag = 1;
        i = i + 1;
    }
    while(s[i] != '\0') {
        if( decimal_point_flag == 1 ) {
            fraction = fraction + (float)(s[i] - 48)/p_10;
            p_10 = p_10*10;
        }
        else if( s[i] == '.' ) {
            decimal_point_flag = 1;
        }
        else {
            whole = 10*whole + (float)(s[i] - 48);
        }
        i = i + 1;
    }
    return (whole+fraction);
}

int dateString_to_intKey(char s[]) {
    int day, month, year;
    day = (int)(s[0] - '0')*10 + (int)(s[1] - '0');
    month = (int)(s[3] - '0')*10 + (int)(s[4] - '0');
    year = (int)(s[6] - '0')*1000 + (int)(s[7] - '0')*100 + (int)(s[8] - '0')*10 + (int)(s[9] - '0');
    int key = (10000*year) + (100*month) + day;
    return key;
}
expenseCategory string_to_expenseCategory(char s[]) {
    expenseCategory ret_val = RENT;
    if( strcmp(s, "Rent") == 0 ) {
        ret_val = RENT;
    }
    else if( strcmp(s, "Utility") == 0 ) {
        ret_val = UTILITY;
    }
    else if( strcmp(s, "Groceries") == 0 ) {
        ret_val = GROCERIES;
    }
    else if( strcmp(s, "Stationary") == 0 ) {
        ret_val = STATIONARY;
    }
    else if( strcmp(s, "Leisure") == 0 ) {
        ret_val = LEISURE;
    }
    return ret_val;
}
void expenseCategory_to_string(char s[], expenseCategory cat) {
    if( cat == RENT ) {
        strcpy(s, "Rent");
    }
    else if( cat == UTILITY ) {
        strcpy(s, "Utility");
    }
    else if( cat == GROCERIES ) {
        strcpy(s, "Groceries");
    }
    else if( cat == STATIONARY ) {
        strcpy(s, "Stationary");
    }
    else if( cat == LEISURE ) {
        strcpy(s, "Leisure");
    }
}
int partition_int(int A[], int x, int low, int high) {
    int i = low, j = high, temp;
    while( i < j && A[i] <= x ) i = i + 1;
    while( i < j && A[j] > x ) j = j - 1;
    if( A[j] > x ) j = j - 1;

    while( i < j ) {
        temp = A[i];
        A[i] = A[j];
        A[j] = temp;

        while( A[i] <= x ) i = i + 1;
        while( A[j] > x ) j = j - 1;
    }
    return j;
}

void QuickSort_int(int A[], int low, int high) {
    int p, temp;
    if( low < high ) {
        p = partition_int(A, A[low], low, high);
        temp = A[low];
        A[low] = A[p];
        A[p] = temp;

        QuickSort_int(A, low, p-1);
        QuickSort_int(A, p+1, high);
    }
}

int partition_float(float A[], float x, int low, int high) {
    int i = low, j = high, temp;
    while( i < j && A[i] <= x ) i = i + 1;
    while( i < j && A[j] > x ) j = j - 1;
    if( A[j] > x ) j = j - 1;

    while( i < j ) {
        temp = A[i];
        A[i] = A[j];
        A[j] = temp;

        while( A[i] <= x ) i = i + 1;
        while( A[j] > x ) j = j - 1;
    }
    return j;
}

void QuickSort_float(float A[], int low, int high) {
    int p; float temp;
    if( low < high ) {
        p = partition_float(A, A[low], low, high);
        temp = A[low];
        A[low] = A[p];
        A[p] = temp;

        QuickSort_float(A, low, p-1);
        QuickSort_float(A, p+1, high);
    }
}
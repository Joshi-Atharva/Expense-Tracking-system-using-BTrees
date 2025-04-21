#ifndef UTILS_H
#define UTILS_H

#define M 5
#define MINITEMS ((M-1)/2) 
#define MAXITEMS (M-1)
#define MAXCHILDREN M
#define MINCHILDREN ((M+1)/2)
#define NAME_SIZE 50

typedef enum{RENT, UTILITY, GROCERIES, STATIONARY, LEISURE} expenseCategory;
typedef enum{FAILURE, SUCCESS} status_code;
typedef enum{FALSE, TRUE} Boolean;
typedef int idx_t;
typedef int user_key_t;
typedef unsigned int expense_key_t;
typedef struct UserTag {
    int user_id;
    int family_id;
    char username[50];
    float income;
}User;

typedef struct UserElementTag {
    User data;
    user_key_t key;
}UserElement;

typedef struct UserNodeTag {
    UserElement elements[M-1];
    struct UserNodeTag* children[M];
    struct UserNodeTag* parent;
    int cnt;
}UserNode;

typedef struct FamilyTag {
    int family_id;
    char family_name[NAME_SIZE];
    user_key_t members[4];
    int family_size;
    float total_family_income;
    float monthly_family_expense[12];
}Family;
typedef int family_key_t;
typedef struct FamilyElementTag {
    Family data;
    family_key_t key;
}FamilyElement;
typedef struct FamilyNodeTag {
    FamilyElement elements[M-1];
    struct FamilyNodeTag* children[M];
    struct FamilyNodeTag* parent;
    int cnt;
}FamilyNode;

typedef struct ExepnseTag {
    int expense_id;
    int user_id;
    expenseCategory expense_category;
    float expense_amount;
    char date_of_expense[11]; // DD-MM-YYYY
}Expense;

typedef struct ExpenseElementTag {
    Expense data;
    expense_key_t key;
}ExpenseElement;

typedef struct ExpenseNodeTag {
    ExpenseElement elements[M-1];
    struct ExpenseNodeTag* children[M];
    struct ExpenseNodeTag* parent;
    int cnt;
}ExpenseNode;

typedef struct LLExpNodeTag {
    expense_key_t data;
    struct LLExpNodeTag* next;
}LLExpNode;

void InitialiseProgram();
int string_to_int(char s[]);
float string_to_float(char s[]);
int dateString_to_intKey(char s[]);
expenseCategory string_to_expenseCategory(char s[]);
void expenseCategory_to_string(char s[], expenseCategory cat);
int partition_int(int A[], int x, int low, int high);
void QuickSort_int(int A[], int low, int high);
int partition_float(float A[], float x, int low, int high);
void QuickSort_float(float A[], int low, int high);
#endif
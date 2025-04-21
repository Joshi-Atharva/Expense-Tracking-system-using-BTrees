#include <stdio.h>
#include "utils.h"
#include "FamilyCrud.h"
#include "UserCrud.h"
#include "ExpenseCrud.h"

UserNode* root_user;
FamilyNode* root_family;
ExpenseNode* root_expense;

int main() {
    InitialiseProgram();
    InitialiseFamilyTree();
    InitialiseUserTree();
    InitialiseExpenseTree();

    status_code sc = ReadUserData();
    sc = sc && ReadExpenseData();
    if (sc == SUCCESS) {
        printf("data read successfully.\n");
    } else {
        printf("Failed to read data.\n");
    }
    FPrintAllInfo_Expense(root_expense);
    FPrintAllInfo_User(root_user);
    FPrintAllInfo_Family(root_family);

    Boolean exit = FALSE;
    while( !exit ) {
        printf("Select following choices:\n"
            "\t1. Add User\n"
            "\t2. Add Expense\n"
            "\t3. Delete User\n"
            "\t4. Delete Expense\n"
            "\t5. Update User\n"
            "\t6. Update Expense\n"
            "\t7. Print all info\n"
            "\t8. Print info tables to files\n"
            "\t9. Delete Family\n"
            "\t10. GetCategoricalExpenseFamily\n"
            "\t11. GetIndividualExpense\n"
            "\t12. Period Search (Over date)\n"
            "\t13. Ranged Search (Over ExpenseID)\n"
            "\t14. GetHighestExepenseDay\n"
            "\t15. Update Family\n"
            "\t16. GetTotalFamilyExpense\n"
            "\t0. Exit\n");
        int choice;
        scanf("%d", &choice);
        int temp;
        switch(choice) {
            case 1:
                printf("Add User\n");
                User NewUser;
                printf("Enter user ID: ");
                scanf("%d", &NewUser.user_id);
                printf("Enter family ID: ");
                scanf("%d", &NewUser.family_id);
                printf("Enter username: ");
                scanf("%s", NewUser.username);
                printf("Enter income: ");
                scanf("%f", &NewUser.income);
                status_code sc = AddUser(NewUser);
                if (sc == SUCCESS) {
                    printf("User added successfully.\n");
                } else {
                    printf("Failed to add user.\n");
                }

                break;
            case 2:
                printf("Add Expense\n");
                Expense NewExpense;
                printf("Enter expense ID: ");
                scanf("%d", &NewExpense.expense_id);
                printf("Enter user ID: ");
                scanf("%d", &NewExpense.user_id);
                printf("Enter expense category (0-4) for enum{RENT, UTILITY, GROCERIES, STATIONARY, LEISURE}: ");
                scanf("%d", &temp);
                NewExpense.expense_category = (expenseCategory)temp;
                printf("Enter expense amount: ");
                scanf("%f", &NewExpense.expense_amount);
                printf("Enter date of expense (DD-MM-YYYY): ");
                scanf("%s", NewExpense.date_of_expense);
                sc = AddExpense(NewExpense);
                if (sc == SUCCESS) {
                    printf("Expense added successfully.\n");
                } else {
                    printf("Failed to add expense.\n");
                }
                break;
            case 3:
                printf("Delete User\n");
                int user_id;
                printf("Enter user ID to delete: ");
                scanf("%d", &user_id);
                sc = DeleteUser(user_id);
                if (sc == SUCCESS) {
                    printf("User with ID %d deleted successfully.\n", user_id);
                } else {
                    printf("Failed to delete user with ID %d.\n", user_id);
                }
                break;
            case 4:
                printf("Delete Expense\n");
                int expense_id;
                printf("Enter expense ID to delete: ");
                scanf("%d", &expense_id);
                sc = DeleteExpense(expense_id);
                if (sc == SUCCESS) {
                    printf("Expense with ID %d deleted successfully.\n", expense_id);
                } else {
                    printf("Failed to delete expense with ID %d.\n", expense_id);
                }
                break;
            case 5:
                printf("Update User\n");
                int userID;
                printf("Enter user ID to update: ");
                scanf("%d", &userID);
                sc = UpdateUser(userID);
                if (sc == SUCCESS) {
                    printf("User with ID %d updated successfully.\n", userID);
                } else {
                    printf("Failed to update user with ID %d.\n", userID);
                }
                break;
            case 6:
                printf("Update Expense\n");
                int expenseID;
                printf("Enter expense ID to update: ");
                scanf("%d", &expenseID);
                sc = UpdateExpense(expenseID);
                if (sc == SUCCESS) {
                    printf("Expense with ID %d updated successfully.\n", expenseID);
                } else {
                    printf("Failed to update expense with ID %d.\n", expenseID);
                }

                break;
            case 7:
                printf("Print all info\n");
                FPrintAllInfo_User(root_user);
                FPrintAllInfo_Family(root_family);
                FPrintAllInfo_Expense(root_expense);
                break;
            case 8:
                printf("Printing user tables to file...\n");
                FPrintInorderTable_User(root_user);
                printf("Printing family tables to file...\n");
                FPrintInorderTable_Family(root_family);
                printf("Printing expense tables to file...\n");
                FPrintInorderTable_Expense(root_expense);
                printf("All tables printed to files.\n");
                break;
            case 9:
                printf("Delete Family\n");
                int family_id;
                printf("Enter family ID to delete: ");
                scanf("%d", &family_id);
                sc = DeleteFamily(family_id);
                if (sc == SUCCESS) {
                    printf("Family with ID %d deleted successfully.\n", family_id);
                } else {
                    printf("Failed to delete family with ID %d.\n", family_id);
                }
                break;
            case 10:
                printf("Get Categorical Expense Family\n");
                int familyID, cat;
                printf("Enter family ID: ");
                scanf("%d", &familyID);
                printf("Enter expense category (0-4) for enum{RENT, UTILITY, GROCERIES, STATIONARY, LEISURE}: ");
                scanf("%d", &cat);
                GetCategoricalExpenseFamily(familyID, cat);
                break;
            case 11:
                printf("Get Individual Expense\n");
                int month;
                printf("Enter user ID: ");
                scanf("%d", &userID);
                printf("Enter month (1-12): ");
                scanf("%d", &month);
                float total_expense = GetIndividualExpense(userID, month);
                break;
            case 12:
                printf("Period Search (Over date)\n");
                char start_date[11], end_date[11];
                printf("Enter start date (DD-MM-YYYY): ");
                scanf("%s", start_date);
                printf("Enter end date (DD-MM-YYYY): ");
                scanf("%s", end_date);
                sc = PeriodSearch(start_date, end_date);
                if (sc == SUCCESS) {
                    printf("Period search completed successfully.\n");
                } else {
                    printf("Failed to perform period search.\n");
                }
                break;
            case 13:
                printf("Ranged Search (Over ExpenseID)\n");
                int ExpID_low, ExpID_high, UserID;
                printf("Enter lower bound of Expense ID: ");
                scanf("%d", &ExpID_low);
                printf("Enter upper bound of Expense ID: ");
                scanf("%d", &ExpID_high);
                printf("Enter User ID: ");
                scanf("%d", &UserID);
                sc = RangedSearch(ExpID_low, ExpID_high, UserID);
                if (sc == SUCCESS) {
                    printf("Ranged search completed successfully.\n");
                } else {
                    printf("Failed to perform ranged search.\n");
                }
                break;
            case 14:
                printf("Get Highest Expense Day\n");
                printf("Enter family ID: ");
                scanf("%d", &familyID);
                sc = GetHighestExpenseDay(familyID);
                if (sc == SUCCESS) {
                    printf("Highest expense day retrieved successfully.\n");
                } else {
                    printf("Failed to retrieve highest expense day.\n");
                }
                break;
            case 15:
                printf("Update family ID\n");
                printf("Enter familyID: ");
                scanf("%d", &familyID);
                sc = UpdateFamily(familyID);
                break;
            case 16:
                printf("Get Total Family Expense\n");
                printf("Enter familyID: ");
                scanf("%d", &familyID);
                GetTotalFamilyExpense(familyID);
                break;
            case 0:
                printf("Exiting...\n");
                exit = TRUE;
                break;
            case -1: 
                printf("custom debug\n");
                idx_t ret_idx;
                ExpenseNode* ptr = MinExpKeyOfUser(983, root_expense, &ret_idx);
                if( ptr != NULL ) {
                    printf("Found expense with user_id 983 at index %d\n", ret_idx);
                    printf("Expense ID: %d\n", ptr->elements[ret_idx].data.expense_id);
                    printf("Expense Amount: %.2f\n", ptr->elements[ret_idx].data.expense_amount);
                    printf("Expense Category: %d\n", ptr->elements[ret_idx].data.expense_category);
                    printf("Date of Expense: %s\n", ptr->elements[ret_idx].data.date_of_expense);
                    printf("User ID: %d\n", ptr->elements[ret_idx].data.user_id);
                    printf("Expense Key: %d\n", ptr->elements[ret_idx].key);
                    printf("Expense Node Address: %p\n", ptr);
                } 
                else {
                    printf("No expense found for user_id 983\n");
                }
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                
                break;
        }
    }
    // Free allocated memory
    return 0;
}

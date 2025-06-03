/*

流程總覽:
1)用戶登入(支持創建多個用戶)

2)進入使用者專屬介面
-顯示 7 個主要選單功能
- 若有設定每月消費限制，會在畫面上方顯示各月份預算

3)使用者可依照選單操作，直到選擇退出系統

選項:
1.記帳（Add Expenses）
- 手動輸入日期、類別、金額與 備註(optional)
- 自動產生UNIQUE交易 ID（TH-XXXX）
- 若本月總額將超過設定預算，會提醒用戶超支，但仍允許寫入

2.刪除特定款項（Remove Expenses）
-使用交易 ID 來刪除特定消費紀錄
-由於每款項都是唯一的ID，所以不會有刪錯的問題

3.顯示全部歷史紀錄（View History Transactions）
- 印出所有消費紀錄，並加總顯示總支出金額

4.顯示特定月份的消費記錄 （View Monthly Report）
- 輸入月份（YYYY-MM），只顯示該月支出

5.轉成CSV 
- 將 <用戶>.txt(用戶數據) 檔案轉為 CSV 格式
- 若有在設定選項設定預算，會同時將 <用戶>_budget.txt 也轉為 CSV

6. 設定
- 目前支援「設定每月預算」
- 若已設定過某月，則會進行「覆寫」更新，可輕易調整和新增預算於其他月份
- 所有預算儲存在 <用戶>_budget.txt

7. 離開系統 (Quit)

資料檔案說明: 
- users.txt：紀錄所有用戶帳密
- <用戶>.txt：紀錄該用戶的消費紀錄
- <用戶>_budget.txt：紀錄每月預算限制
- <用戶>.csv 和 <用戶>_budget.csv：轉出後可用 Excel 開啟的報表

*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#define OPTIONS 7
#define MAX_ID 50
#define MAX_PASSWORD 50
#define USER_FILE "users.txt"
#define DATE_FORMAT 10
#define MAX_LINE 256

//For readability in menu
typedef enum
{
    ADD_EXPENSES = 1,
    REMOVE_EXPENSES,
    VIEW_HISTORY_TRANSACTION,
    VIEW_MONTHLY_REPORT,
    EXPORT_TO_CSV,
    SETTINGS,
    QUIT
}OPTION; 

//User attributes 
typedef struct
{
    char name[MAX_ID];
    char password[MAX_PASSWORD];
}User;


void display_menu(const User *p_current_user)
{
    char *menu_choices[OPTIONS] = {"Add Expenses", "Remove Expenses", "View History Transactions", "View Monthly Report", "Export to CSV", "Settings", "Exit"};

    printf("=======================\n");
    printf("Personal Budget Tracker\n");
    printf("Welcome back! %s\n", p_current_user->name);
    
    //If data exists, display the budget by months
    char budget_filename[100];
    sprintf(budget_filename, "%s_budget.txt", p_current_user->name);
    FILE *budget_limit_fp;
    budget_limit_fp = fopen(budget_filename, "r");

    if (budget_limit_fp)
    {
        printf("-----------------------\n");
        printf("Budget limit:\n");
        char line[MAX_LINE];
        char budget_month[8];
        float budget_amount;
        while (fgets(line, sizeof(line), budget_limit_fp))
        {
            sscanf(line, "%[^|]|%f", budget_month, &budget_amount);
            printf("%s: $%.2f\n", budget_month, budget_amount);
        }
        fclose(budget_limit_fp);
    }

    //Display options 
    printf("=======================\n");
    for (int i = 0; i < OPTIONS; i++)
    {
        printf("%d. %s\n", i+1, menu_choices[i]);
    }
    return;
}

//Small function that returns user input
int get_user_choice(void)
{
    int input;
    printf("Enter choice: ");
    scanf("%d", &input);
    while (getchar() !=  '\n');

    return input;
}

//Returns specific user struct
User login(void)
{
    User current_user;
    char line[MAX_LINE];
    bool isUserFound = false;
    bool isValidPass = false;

    //Prompt user to input ID and password
    printf("---Log in---\n");
    printf("Enter your name: ");
    scanf("%s", current_user.name);
    while (getchar() != '\n');
    printf("Enter your password: ");
    scanf("%s", current_user.password);
    while (getchar() != '\n');


    //Check whether if the file exist, if yes we search the whole file 
    FILE *fp; 

    fp = fopen(USER_FILE, "r");
    if (fp)
    {
        while (fgets(line, sizeof(line), fp) != NULL)
        {
            char file_username[MAX_ID];
            char file_password[MAX_PASSWORD];

            sscanf(line, "%[^|]|%s", file_username, file_password); //%[] scanset, reads until |

            //Check specific user's existance
            if (strcmp(current_user.name, file_username) == 0)
            {
                isUserFound = true;
                //Check password validity
                if (strcmp(current_user.password, file_password) == 0)
                {
                    isValidPass = true;
                    break; //break from the loop since we found 
                }
                else
                {
                    printf("Password is incorrect!\n");
                    break;
                }
            }
        }
        fclose(fp); //always close when finish using a file
    }

    
    if (isUserFound && !isValidPass) //if User found, but password incorrect, return empty user struct
    {
        printf("Try again!\n");
        User null;
        strcpy(null.name, "");
        strcpy(null.password, "");

        return null; 
    }
    else if (!isUserFound) //If user not found, register new and save it into the user.txt
    {
        printf("Registering new user \"%s\"...\n", current_user.name);

        fp = fopen(USER_FILE, "a");
        if (fp)
        {
            fprintf(fp, "%s|%s\n", current_user.name, current_user.password);
            fclose(fp);
        }
        else printf("Error occured.\n");
    }
    return current_user; //user struct
}

//Small anti-hacker detection
void exceed_attempt_message(void)
{
    printf("Unusual activity detected.\n");
    printf("Try again after 5 seconds...\n");
    Sleep(5000);
    return;
}


void add_expenses(User *p_current_user)
{
    //Create a data file for the specific user 
    char filename[100];
    sprintf(filename, "%s.txt", p_current_user->name);

    FILE *fp;
    fp = fopen(filename, "a");

    if (!fp) 
    {
        printf("Error occured.\n");
        return;
    }

    //Prompt user to add expense and store it into file 
    char generated_id[50], exist_id[50], date[50], category[50];
    float amount;
    bool isDuplicate;
    
    //Get Unqiue_ID by checking no duplicate id exist 
    char line[MAX_LINE];
    FILE *read_fp;
    
    //Generate ID
    do
    {
        isDuplicate = false;
        read_fp = fopen(filename, "r");
        int num = rand() % 10000;
        sprintf(generated_id, "TH-%04d", num);

        //check if it's alrdy existed 
        while (fgets(line, sizeof(line), read_fp))
        {
            sscanf(line, "%[^|]", exist_id);
            if (strcmp(generated_id, exist_id) == 0)
            {
                isDuplicate = true;
            }
        }

        fclose(read_fp);

    } while (isDuplicate);


    //Get Date
    printf("Date (YYYY-MM-DD): ");
    fgets(date, sizeof(date), stdin);
    date[strcspn(date, "\n")] = 0;

    //Handle invalid format for date
    while (strlen(date) != DATE_FORMAT)
    {
        printf("Invalid format! Try again.\n");
        printf("Date (YYYY-MM-DD): ");
        fgets(date, sizeof(date), stdin);
        date[strcspn(date, "\n")] = 0;
    }

    //Get Category
    printf("Category: ");
    fgets(category, sizeof(category), stdin);
    category[strcspn(category,"\n")] = 0;
    for (int i = 0; category[i]; i++) category[i] = toupper(category[i]);

    //Get Expenses Amount
    printf("Amount(NTD): ");
    scanf("%f", &amount);
    while(getchar() != '\n');

    while (amount <= 0)
    {
        printf("Amount must not be 0 or negative!\n");
        printf("Amount(NTD): ");
        scanf("%f", &amount);
        while (getchar() != '\n');
    }

    //Budget warning check
    char budget_filename[100];
    sprintf(budget_filename, "%s_budget.txt", p_current_user->name);
    FILE *budget_fp;
    budget_fp = fopen(budget_filename, "r");

    if (budget_fp)
    {
        char budget_line[MAX_LINE];
        char budget_date[8];
        float budget_limit;
        while (fgets(budget_line, sizeof(budget_line), budget_fp))
        {
            sscanf(budget_line, "%[^|]|%f", budget_date, &budget_limit);
            //if month found 
            if (strncmp(budget_date, date, 7) == 0)
            {
                //Read from expenses.txt to calculate total spendings of that month
                char total_spent_filename[100], line[MAX_LINE];
                sprintf(total_spent_filename, "%s.txt", p_current_user->name);
                FILE *read_total_spent_fp;
                read_total_spent_fp = fopen(total_spent_filename, "r");

                float total_month_expenses = 0;
                if (read_total_spent_fp)
                {
                    char temp_ID[50], temp_date[50], temp_category[50], temp_description[100];
                    float temp_amount;

                    while (fgets(line, sizeof(line), read_total_spent_fp))
                    {
                        sscanf(line, "%[^|]|%[^|]|%[^|]|%f|%[^\n]", temp_ID, temp_date, temp_category, &temp_amount, temp_description);
                        if (strncmp(temp_date, date, 7) == 0)
                        {
                            total_month_expenses += temp_amount;
                        }
                    }
                    fclose(read_total_spent_fp);
                }

                //Display results 
                if (total_month_expenses + amount > budget_limit)
                {
                    printf("Warning: This expenses will exceed the monthly budget $%.2f!\n", budget_limit);
                    float exceeded = (total_month_expenses + amount) - budget_limit;
                    printf("You are exceeding your budget by: $%.2f\n", exceeded);
                }
            }
        }
    fclose(budget_fp);
    }
    
    //Ask for descriptions
    char ask_w_description;
    char description[100];

    printf("Add description? (Y/N): ");
    scanf(" %c", &ask_w_description);
    while (getchar() != '\n');

    char up_ask_w_description = toupper(ask_w_description);

    if (up_ask_w_description == 'Y')
    {
        printf("Notes: ");
        fgets(description, sizeof(description), stdin);
        description[strcspn(description, "\n")] = 0;
    }
    else strcpy(description,"");
    
    //append into file
    fprintf(fp, "%s|%s|%s|%.2f|%s\n", generated_id, date, category, amount, (strlen(description)) > 0 ? description : "-");
    printf("New expenses added: %s|%s|$%.2f\n",date, category, amount);

    //Close file after use
    fclose(fp);

    return;
}

void remove_expenses_by_itemID(User *p_current_user)
{
    //read from file 
    char filename[150];
    sprintf(filename, "%s.txt", p_current_user->name);

    FILE *fp;
    fp = fopen(filename, "r");
    if (!fp)
    {
        printf("No record.\n");
        return;
    }

    //Prompt user enter item ID
    char remove_item_id[50];
    
    printf("Remove by ID (e.g., TH-0001): ");
    fgets(remove_item_id, sizeof(remove_item_id), stdin);
    remove_item_id[strcspn(remove_item_id, "\n")] = 0;

    //Copy all transaction except for the requested transaction by user
    FILE *temp_fp;
    temp_fp = fopen("temp.txt", "w"); //create new file
    if (!temp_fp) 
    {
    printf("Error creating temp file.\n");
    return;
    }

    char line[MAX_LINE], ID[50];
    bool found_ID = false;

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%[^|]", ID);
        if (strcmp(remove_item_id, ID) != 0)
        {
            fputs(line, temp_fp); //Copy all line to temp_file
        }
        else 
        {
            found_ID = true; //For show correct output message
        }
    }

    //Remember to CLOSE two files before removing!
    fclose(fp);
    fclose(temp_fp);

    //remove the original file
    remove(filename);
    //rename the copied temp file into filename
    rename("temp.txt", filename);

    if (found_ID) printf("Transaction \"%s\" has been removed successfully.\n", remove_item_id);
    else printf("Transaction \"%s\" not found.\n", remove_item_id);

    return;
}

void view_history_reports(User *p_current_user)
{
    //Load file 
    char filename[100];
    sprintf(filename, "%s.txt", p_current_user->name);

    //File handling
    FILE *fp;
    fp = fopen(filename, "r");

    if (!fp)
    {
        printf("No data exists.\n");
        return;
    }

    //Display all transactions
    char line[MAX_LINE];
    float total = 0;

    printf("====== Transaction History ======\n");
    while (fgets(line, sizeof(line), fp))
    {
        char ID[50], date[50],category[50], description[100];
        float amount;
        
        sscanf(line, "%[^|]|%[^|]|%[^|]|%f|%[^\n]", ID, date, category, &amount, description);
        printf("%s | %s | %s | $%.2f | %s\n", ID, date, category, amount, description);
        total += amount;
    }

    printf("Total expenses: $%.2f (NTD)\n", total);

    fclose(fp);
    return;
}

void view_monthly_report(User *p_current_user)
{
    //Load file
    char filename[100];
    sprintf(filename, "%s.txt", p_current_user->name);

    //Ask for a month 
    char target_month[8];
    printf("View month (YYYY-MM): ");
    fgets(target_month, sizeof(target_month), stdin);
    target_month[strcspn(target_month, "\n")] = 0;

    while (strlen(target_month) != 7)
    {
        printf("Invalid format! Try again.\n");
        printf("View month (YYYY-MM): ");
        fgets(target_month, sizeof(target_month), stdin);
        target_month[strcspn(target_month, "\n")] = 0;
    }

    //File handling
    FILE *fp;
    fp = fopen(filename, "r");

    if (!fp)
    {
        printf("No records found.\n");
        return;
    }

    //Read file line by line 
    char line[150];
    float total = 0;
    bool found = false;

    printf("====== %s Monthly Report ======\n", target_month);

    while (fgets(line, sizeof(line), fp))
    {
        char ID[50], date[50],category[50], description[100];
        float amount;
        
        sscanf(line, "%[^|]|%[^|]|%[^|]|%f|%[^\n]", ID, date, category, &amount, description);

        if (strncmp(date, target_month, 7) == 0)
        {
            printf("%s | %s | %s | $%.2f | %s\n", ID, date, category, amount, description);
            total += amount;
            found = true;
        }
    }

    fclose(fp);

    if (found) printf("Total Expenses for %s: $%.2f\n", target_month, total);
    else printf("No data found for %s.\n", target_month);

    return;
}

void export_expenses_to_CSV(User *p_current_user)
{
    //Load file
    char txt_filename[100], csv_filename[100];
    sprintf(txt_filename, "%s.txt", p_current_user->name);
    sprintf(csv_filename, "%s.csv", p_current_user->name);

    FILE *source;
    FILE *destination;
    source = fopen(txt_filename, "r");
    destination = fopen(csv_filename, "w");

    if (!destination || !source)
    {
        printf("Error occured\n");
        return;
    }

    //Header
    fprintf(destination, "ID,Date,Category,Amount,Description\n");

    //Read txt file
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), source))
    {
        //replace | to , (easier for excel)
        for (int i = 0; line[i]; i++)
        {
            if (line[i] == '|')
            {
                line[i] = ',';
            }
        }
        //Write csv file
        fprintf(destination, "%s", line);
    }

    fclose(source);
    fclose(destination);

    printf("%s exported to %s successfully.\n", txt_filename, csv_filename);

    return;
}

void export_budget_to_CSV(User *p_current_user)
{
    //Load File 
    char budget_txt_filename[100], budget_csv_filename[100];
    sprintf(budget_txt_filename, "%s_budget.txt", p_current_user->name);
    sprintf(budget_csv_filename, "%s_budget.csv", p_current_user->name);

    FILE *source = fopen(budget_txt_filename, "r");
    FILE *destination = fopen(budget_csv_filename, "w");

    if (!destination || !source)
    {
        printf("Error occured\n");
        return;
    }

    //Header
    fprintf(destination, "Date,Budget\n");

    //Read txt file 
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), source))
    {
        //replace | to , (easier for excel)
        for (int i = 0; line[i]; i++)
        {
            if (line[i] == '|')
            {
                line[i] = ',';
            }
        }
        //Write csv file
        fprintf(destination, "%s", line);
    }

    fclose(source);
    fclose(destination);

    printf("%s exported to %s successfully.\n", budget_txt_filename, budget_csv_filename);

    return;
}

void set_monthly_budget(User *p_current_user)
{
    //Prepare file name (budget file & expenses file)
    char budget_filename[100];
    char filename[100];
    sprintf(budget_filename, "%s_budget.txt", p_current_user->name);
    sprintf(filename, "%s.txt", p_current_user->name);
    
    //Get valid month with edge case handling
    char month[8];
    printf("Enter Month to set budget (YYYY-MM): ");
    fgets(month, sizeof(month), stdin);
    month[strcspn(month, "\n")] = 0;

    while (strlen(month) != 7)
    {
        printf("Invalid format! Please try again (YYYY-MM): ");
        fgets(month, sizeof(month), stdin);
        month[strcspn(month, "\n")] = 0;
    }

    //Calculate total expenses for the specific month (from John.txt)
    FILE *expenses_fp;
    expenses_fp = fopen(filename, "r");

    char ID[50], date[50], category[50], description[100];
    float amount, monthly_total = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), expenses_fp))
    {
        sscanf(line, "%[^|]|%[^|]|%[^|]|%f|%[^\n]", ID, date, category, &amount, description);
        if (strncmp(date, month, 7) == 0)
        {
            monthly_total += amount;
        }
    }
    fclose(expenses_fp);

    //if new budget_limit is lower than expense = invalid 
    float budget_limit;
    do
    {
        printf("Total spent in %s: $%.2f\n", month, monthly_total);
        printf("Enter budget limit (must be larger or equal to $%.2f): ", monthly_total);
        scanf("%f", &budget_limit);
        while (getchar() != '\n');

        if (budget_limit < monthly_total)
        {
            printf("Invalid budget limit! Try again!\n");
        }
    } while (budget_limit < monthly_total);
    
    //Update or set budget limit to particular month
    FILE *fp_old = fopen(budget_filename, "r");
    FILE *fp_temp = fopen("temp_budget.txt", "w");

    if (!fp_temp)
    {
        printf("Error creating temp file.\n");
        if (fp_old) fclose(fp_old);
        return;
    }

    char budget_line[MAX_LINE], existing_month[8];
    float existing_limit;
    bool updated = false;

    //If there exists a budget_file, we find the existing month with the user input month, and "delete from original file" then we update data as a new file
    if (fp_old)
    {
        while (fgets(budget_line, sizeof(budget_line), fp_old))
        {
            sscanf(budget_line, "%[^|]|%f", existing_month, &existing_limit);
            if (strcmp(existing_month, month) != 0) fputs(budget_line, fp_temp); // keep other months
            else updated = true; // skip current month (will overwrite)
        }
        fclose(fp_old);
    }

    // Append or updated new monthly budget
    fprintf(fp_temp, "%s|%.2f\n", month, budget_limit);
    fclose(fp_temp);

    remove(budget_filename);
    rename("temp_budget.txt", budget_filename);

    //Output message
    if (updated)
        printf("Updated budget for %s to $%.2f\n", month, budget_limit);
    else
        printf("Set new budget for %s: $%.2f\n", month, budget_limit);
}

void settings(User *p_current_user)
{
    //Display settings
    char *option_settings[2] = {"Set Monthly Budget", "Coming soon..."};
    printf("----- Settings -----\n");
    for (int i = 0; i < 2; i++)
    {
        printf("%d. %s\n", i+1, option_settings[i]);
    }

    //Prompt user to enter choice
    int userChoice_settings;
    printf("Choice: ");
    scanf("%d", &userChoice_settings);
    while (getchar() != '\n');

    switch (userChoice_settings)
    {
    case 1:
        set_monthly_budget(p_current_user);
        break;
    case 2:
        printf("Stay tuned for future updates!\n");
        break;
    default:
        printf("Invalid Option!\n");
        break;
    }
    return;
}

int main(void)
{
    srand(time(NULL));
    int attempt = 0;
    User current_user;
    User *p_current_user = &current_user;
    
    //Login Section
    while (attempt < 3)
    {
        current_user = login();

        if (strlen(current_user.name) != 0) break;
        attempt++;
    }
    if (attempt >= 3)
    {
        exceed_attempt_message();
        return 0;
    }

    //Menu Section
    int userChoice;

    do{
        display_menu(p_current_user);
        userChoice = get_user_choice();

        switch (userChoice)
        {
        case ADD_EXPENSES:
            add_expenses(p_current_user);
            break;
        case REMOVE_EXPENSES:
            remove_expenses_by_itemID(p_current_user);
            break;
        case VIEW_HISTORY_TRANSACTION:
            view_history_reports(p_current_user);
            break;
        case VIEW_MONTHLY_REPORT:
            view_monthly_report(p_current_user);
            break;
        case EXPORT_TO_CSV:
            export_expenses_to_CSV(p_current_user);
            export_budget_to_CSV(p_current_user);
            break;
        case SETTINGS:
            settings(p_current_user);
            break;
        case QUIT:
            printf("Thanks for using our program!\n");
            break;
        default:
            printf("Invalid option! Please choose 1~7.\n");
            break;
        }
    } while (userChoice != QUIT);

    return 0;
}

/*
FLowchart:
Log In(struct) -> Display Menu -> Get User Choice -> Save(Export to File) -> Exit

##Add Expenses##
-Budget goal warning 
-LOCK budget if exceed

Remove Expenses by item_id, month
*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include <ctype.h>
#define OPTIONS 5
#define MAX_ID 50
#define MAX_PASSWORD 50
#define USER_FILE "users.txt"
#define DATE_FORMAT 10

typedef enum
{
    ADD_EXPENSES = 1,
    VIEW_HISTORY_TRANSACTION,
    VIEW_MONTHLY_REPORT,
    EXPORT_TO_CSV,
    QUIT
}OPTION; 

typedef struct
{
    char name[MAX_ID];
    char password[MAX_PASSWORD];
}User;

void display_menu(const User *p_current_user)
{
    char *menu_choices[OPTIONS] = {"Add Expenses", "View history transactions", "View Monthly Report", "Export to CSV", "Exit"};

    printf("=======================\n");
    printf("Personal Budget Tracker\n");
    printf("Welcome back! %s\n", p_current_user->name);
    printf("=======================\n");
    for (int i = 0; i < OPTIONS; i++)
    {
        printf("%d. %s\n", i+1, menu_choices[i]);
    }
    return;
}

int get_user_choice(void)
{
    int input;
    printf("Enter choice: ");
    scanf("%d", &input);
    while (getchar() !=  '\n');

    return input;
}

User login(void)
{
    User current_user;
    char line[100];
    bool isUserFound = false;
    bool isValidPass = false;

    //Prompt user to input ID and password
    printf("---Log in---\n");
    printf("Enter your name: ");
    scanf("%s", current_user.name);
    printf("Enter your password: ");
    scanf("%s", current_user.password);

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

            if (strcmp(current_user.name, file_username) == 0)
            {
                isUserFound = true;
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

    //If user not found, register new and save it into the user.txt
    if (isUserFound && !isValidPass)
    {
        printf("Try again!\n");
        User null;
        strcpy(null.name, "");
        strcpy(null.password, "");

        return null; 
    }
    else if (!isUserFound)
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
    return current_user;
}

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

    //File handling
    FILE *fp;
    fp = fopen(filename, "a");

    if (!fp) 
    {
        printf("Error occured.\n");
        return;
    }

    //Prompt user to add expense and store it into file 
    char date[50];
    char category[50];
    float amount;

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

    printf("Category: ");
    fgets(category, sizeof(category), stdin);
    category[strcspn(category,"\n")] = 0;
    for (int i = 0; category[i]; i++) category[i] = toupper(category[i]);

    printf("Amount(NTD): ");
    scanf("%f", &amount);

    while (amount <= 0)
    {
        printf("Amount must not be 0 or negative!\n");
        printf("Amount(NTD): ");
        scanf("%f", &amount);
    }

    char ask_w_desription;
    char description[100];

    printf("Add description? (Y/N): ");
    scanf(" %c", &ask_w_desription);
    while (getchar() != '\n');

    char up_ask_w_desription = toupper(ask_w_desription);

    if (up_ask_w_desription == 'Y')
    {
        printf("Notes: ");
        fgets(description, sizeof(description), stdin);
        description[strcspn(description, "\n")] = 0;
    }
    else strcpy(description,"");
    
    //append into file
    fprintf(fp, "%s|%s|%.2f|%s\n", date, category, amount, description);
    printf("New expenses added: %s|%s|$%.2f\n",date, category, amount);

    //Close file after use
    fclose(fp);

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
    char line[100];
    float total = 0;

    printf("====== Transaction History ======\n");
    while (fgets(line, sizeof(line), fp))
    {
        char date[50],category[50], description[100];
        float amount;
        
        sscanf(line, "%[^|]|%[^|]|%f|%[^\n]", date, category, &amount, description);
        printf("%s | %s | $%.2f | %s\n", date, category, amount, description);
        total += amount;
    }

    printf("Total expenses: $%.2f (NTD)\n", total);
    
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
        char date[50],category[50], description[100];
        float amount;
        
        sscanf(line, "%[^|]|%[^|]|%f|%[^\n]", date, category, &amount, description);

        if (strncmp(date, target_month, 7) == 0)
        {
            printf("%s | %s | $%.2f | %s\n", date, category, amount, description);
            total += amount;
            found = true;
        }
    }

    fclose(fp);

    if (found) printf("Total Expenses for %s: $%.2f\n", target_month, total);
    else printf("No data found for %s.\n", target_month);

    return;
}

void export_to_CSV(User *p_current_user)
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

    fprintf(destination, "Date,Category,Amount,Description\n");
    //Read txt file
    char line[100];
    while (fgets(line, sizeof(line), source))
    {
        //replace | to , easier for python
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

int main(void)
{
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
        case VIEW_HISTORY_TRANSACTION:
            view_history_reports(p_current_user);
            break;
        case VIEW_MONTHLY_REPORT:
            view_monthly_report(p_current_user);
            break;
        case EXPORT_TO_CSV:
            export_to_CSV(p_current_user);
            break;
        case QUIT:
            printf("Thanks for using our program!\n");
            break;
        default:
            printf("Invalid option! Please choose 1~5.\n");
            break;
        }
    } while (userChoice != QUIT);

    return 0;
}
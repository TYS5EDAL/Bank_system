#include "bank_system.h"

/* Box drawing characters (kept as numeric codes; console codepage matters) */
#define LT 201 // "\u2554"
#define RT 187 // "\u2557"
#define LB 200 // "\u255A"
#define RB 188 // "\u255D"
#define H 205 // "\u2550"
#define V 186 // "\u2551"

FILE *accounts_file;
FILE *logs_file;
State_t state;

Account current_user;

/* Function declarations (English names) */
void load_accounts();
void open_logs();

void welcome_screen();
int login();
void create_account();
uint8_t main_menu();
void show_balance();
void deposit();
void withdraw();
void change_pin();
void logout();
void shutdown_app();
uint8_t admin_menu();
void list_accounts();
void delete_account();

static uint16_t read_integer(const char *prompt, uint16_t max_count);
static double read_double(const char *prompt);
static void update_pin(uint16_t id, uint16_t new_pin);
static void update_balance(uint16_t id, double amount);
static void write_log(const char *format, ...);

static void print_table_top(uint8_t width);
static void print_table_text(uint8_t width, const char *text);
static void print_table_bottom(uint8_t width);

/****************************************************************************************************************************************/
/*******************************************  State machine  **************************************************************************/
/****************************************************************************************************************************************/

void state_machine(){
    switch (state){
        case INIT:
            printf("\r\n");
            load_accounts();
            open_logs();
            state = LOGIN;
            break;
        case LOGIN:
            welcome_screen();
            {
                int result = login();
                if (result == 1) state = MAIN_MENU;
                else if (result == 2) state = ADMIN_MENU;
                else if (result == 3) { state = NEW_ACCOUNT; printf("\r\n"); }
                else state = EXIT_APP;
            }
            break;
        case NEW_ACCOUNT:
            create_account();
            state = LOGIN;
            break;
        case MAIN_MENU:
            switch(main_menu()){
                case 1:
                    state = BALANCE;
                    break;
                case 2:
                    state = DEPOSIT;
                    break;
                case 3:
                    state = WITHDRAWAL;
                    break;
                case 4:
                    state = CHANGE_PIN;
                    break;
                case 5:
                    state = LOGOUT;
                    break;
                default:
                    state = EXIT_APP;
                    break;
            }
            break;
        case BALANCE:
            show_balance();
            state = MAIN_MENU;
            break;
        case DEPOSIT:
            deposit();
            state = MAIN_MENU;
            break;
        case WITHDRAWAL:
            withdraw();
            state = MAIN_MENU;
            break;
        case CHANGE_PIN:
            change_pin();
            if(current_user.id == 9999) state = ADMIN_MENU;
            else state = MAIN_MENU;
            break;
        case LOGOUT:
            logout();
            state = LOGIN;
            break;
        case EXIT_APP:
            shutdown_app();
            exit(1);
            break;
        case ADMIN_MENU:
            switch(admin_menu()){
                case 1:
                    state = ACCOUNTS;
                    break;
                case 2:
                    state = DELETE_ACCOUNT;
                    break;
                case 3:
                    state = CHANGE_PIN;
                    break;
                case 4:
                    state = LOGOUT;
                    break;
                default:
                    state = EXIT_APP;
                    break;
            }
            break;
        case ACCOUNTS:
            list_accounts();
            state = ADMIN_MENU;
            break;
        case DELETE_ACCOUNT:
            delete_account();
            state = ADMIN_MENU;
            break;
        default:
            break;
        }
}

/****************************************************************************************************************************************/
/*******************************************  File loading  **************************************************************************/
/****************************************************************************************************************************************/

void load_accounts(){
    accounts_file = fopen("accounts.dat", "rb+");
    if (accounts_file == NULL) {
        /* file doesn't exist, create a new one */
        accounts_file = fopen("accounts.dat", "wb+");
        if (accounts_file == NULL) {
            perror("Failed to open or create accounts file");
            exit(1); 
        }
    }
    /* Ensure admin account exists (ID 9999) */
    Account admin = {9999, "ADMIN", 9999, 0}; fwrite(&admin, sizeof(admin), 1, accounts_file);
    // Example: Account user1 = {1000, "User1", 1234, 169.6}; fwrite(&user1, sizeof(user1), 1, accounts_file);
}
void open_logs(){
    logs_file = fopen("transactions.log", "a"); 
    if (logs_file == NULL) {
        perror("Failed to open or create log file");
        exit(1);
    }
}

/****************************************************************************************************************************************/
/************************************************  Login  *******************************************************************************/
/****************************************************************************************************************************************/

void welcome_screen(){
    print_table_top(40);
    print_table_text(40, "Welcome to FoxVault banking system");
    print_table_bottom(40);
}
int login(){
    uint16_t id;
    uint16_t pin;
    uint8_t attempts = 0;

    print_table_top(40);
    print_table_text(40, "Login");
    print_table_text(40, " ");
    print_table_text(40, "ID:9999 - Admin    ID:9998 - New account");
    print_table_bottom(40);

    while(attempts < 3){
        id = read_integer("ID", 9999);
        if(id == 9998){
            return 3;
        }

        pin = read_integer("PIN", 9999);
        bool id_found = false;
        rewind(accounts_file);
        while(fread(&current_user, sizeof(current_user), 1, accounts_file) == 1){
            if(current_user.id == id){
                id_found = true;
                if(current_user.pin == pin) {
                    if(id == 9999){
                        write_log("ID:%d %s - successful login", current_user.id, current_user.name);
                        return 2;
                    }else{
                        write_log("ID:%d - Successful login", current_user.id);
                        return 1;
                    }
                }else {
                    printf(" PIN does not match. Remaining attempts: %d\r\n\r\n", 2 - attempts);
                    write_log("ID:%d - Wrong PIN", current_user.id);
                    break;
                }
            }
        }
        if(!id_found) printf(" ID not found. Remaining attempts: %d\r\n\r\n", 2 - attempts);
        attempts++;
    }
    return 0;
}
void create_account(){
    Account new_user;
    uint16_t id;
    char name[50];
    uint16_t pin;
    uint16_t pin_confirm;

    print_table_top(40);
    print_table_text(40, "Create new account");
    print_table_bottom(40);

    bool id_exists = true;
    while(id_exists){
        id = read_integer("ID", 9997);
        id_exists = false;

        rewind(accounts_file);
        while(fread(&new_user, sizeof(new_user), 1, accounts_file) == 1){
            if(new_user.id == id) {
                id_exists = true;
                break;
            }
        }
        if(id_exists) printf(" Entered ID already exists. Enter another.\r\n\r\n");
        else new_user.id = id;
    }

    printf(" Name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    
    strncpy(new_user.name, name, sizeof(new_user.name) - 1);
    new_user.name[sizeof(new_user.name) - 1] = '\0';

    bool match = false;
    while(!match){
        pin = read_integer("New PIN", 9999);
        pin_confirm = read_integer("Confirm", 9999);
        if(pin == pin_confirm){
            match = true;
            new_user.pin = pin;
        }else{
            printf(" PINs do not match.\r\n\r\n");
        }
    }
    new_user.balance = 0;
    fwrite(&new_user, sizeof(new_user), 1, accounts_file);
    fflush(accounts_file);

    printf(" Account successfully created.\r\n\r\n");
    write_log("ID:%d - New account created", new_user.id);
}

/****************************************************************************************************************************************/
/*************************************************  User  *******************************************************************************/
/****************************************************************************************************************************************/

uint8_t main_menu(){
    printf("\r\n");
    print_table_top(40);
    print_table_text(40, "1. Show balance              ");
    print_table_text(40, "2. Deposit money             ");
    print_table_text(40, "3. Withdraw money            ");
    print_table_text(40, "4. Change PIN                ");
    print_table_text(40, "5. Logout                    ");
    print_table_text(40, "6. Exit program              ");
    print_table_bottom(40);
    
    uint8_t choice = (uint8_t)read_integer("Choose option", 9);
    while (choice < 1 || choice > 6){
        printf(" Choice must be between 1 and 6. Choose again.\r\n\r\n");
        choice = (uint8_t)read_integer("Choose option", 9);
    }
    
    printf("\r\n");
    return choice;
}
void show_balance(){
    print_table_top(40);
    print_table_text(40, "Account details");
    print_table_bottom(40);
    
    printf(" Balance: %.2f \r\n", current_user.balance);
    printf(" Name: %s      \r\n", current_user.name);
    printf(" ID: %d         \r\n", current_user.id);

    write_log("ID:%d - Balance viewed", current_user.id);
}
void deposit(){
    print_table_top(40);
    print_table_text(40, "Deposit");
    print_table_bottom(40);

    double amount = read_double("Deposit");
    update_balance(current_user.id, amount);
    current_user.balance += amount;
    write_log("ID:%d - Deposit +%.2f CZK", current_user.id, amount);
}
void withdraw(){
    print_table_top(40);
    print_table_text(40, "Withdraw");
    print_table_bottom(40);

    double amount = read_double("Withdraw");

    if (amount > current_user.balance) {
        printf(" Not enough funds in the account!\r\n");
        write_log("ID:%d - Failed withdraw attempt %.2f CZK", current_user.id, amount);
        return;
    }
    update_balance(current_user.id, (amount * (-1)));
    current_user.balance += (amount * (-1));
    write_log("ID:%d - Withdraw -%.2f CZK", current_user.id, amount);
}
void change_pin(){
    print_table_top(40);
    print_table_text(40, "Change PIN");
    print_table_bottom(40);

    uint16_t pin;
    uint8_t attempts = 0;
    bool verified = false;

    while(attempts < 3){
        pin = read_integer("PIN", 9999);
        if(pin == current_user.pin){
            verified = true;
            break;
        }else{
            printf(" PIN does not match. Remaining attempts: %d\r\n\r\n", 2 - attempts);
            write_log("ID:%d - Wrong verification PIN", current_user.id);
            attempts++;
        }
    }
    if(!verified){
        write_log("ID:%d - PIN verification failed", current_user.id);
        return;
    }
    printf("\r\n");
    uint16_t new_pin;
    uint16_t pin_confirm;

    attempts = 0;
    verified = false;

    while(attempts < 3){
        new_pin = read_integer("New PIN", 9999);
        pin_confirm = read_integer("Confirm", 9999);
        if(new_pin == pin_confirm){
            verified = true;
            break;
        }else{
            printf(" PINs do not match. Remaining attempts: %d\r\n\r\n", 2 - attempts);
            attempts++;
        }
    }
    if(verified){
        update_pin(current_user.id, new_pin);
        write_log("ID:%d - PIN successfully changed", current_user.id);
        printf(" PIN was successfully changed\r\n");
    }
}
void logout(){
    print_table_top(40);
    print_table_text(40, "Successfully logged out");
    print_table_bottom(40);
    printf("\r\n \r\n \r\n \r\n \r\n");

    write_log("ID:%d - Successfully logged out", current_user.id);
}
void shutdown_app(){
    if(accounts_file) fclose(accounts_file);
    if(logs_file) fclose(logs_file);

    if (current_user.id != 0)
        write_log("ID:%d - Application closed", current_user.id);
    else
        write_log("Application closed without login");
}

/****************************************************************************************************************************************/
/************************************************  Admin  *******************************************************************************/
/****************************************************************************************************************************************/

uint8_t admin_menu(){
    printf("\r\n");
    print_table_top(40);
    print_table_text(40, "1. List accounts             ");
    print_table_text(40, "2. Delete account            ");
    print_table_text(40, "3. Change PIN                ");
    print_table_text(40, "4. Logout                    ");
    print_table_text(40, "5. Exit program              ");
    print_table_bottom(40);
    
    uint8_t choice = (uint8_t)read_integer("Choose option", 9);
    while (choice < 1 || choice > 5){
        printf(" Choice must be between 1 and 5. Choose again.\r\n\r\n");
        choice = (uint8_t)read_integer("Choose option", 9);
    }
    
    printf("\r\n");
    return choice;
}
void list_accounts(){
    print_table_top(40);
    print_table_text(40, "All accounts");
    print_table_bottom(40);
    Account temp;
    uint8_t account_count = 0;
    rewind(accounts_file);
    while(fread(&temp, sizeof(temp), 1, accounts_file) == 1){
        if(temp.id != 9999){
            printf(" ID: %d\r\n", temp.id);
            printf(" Name: %s\r\n", temp.name);
            printf(" Balance: %.2f\r\n\r\n", temp.balance);
        }
        account_count++;
    }
    if(account_count == 1) printf(" No user accounts created.\r\n\r\n");
    write_log("ID:%d %s - Listed all accounts", current_user.id, current_user.name);
}
void delete_account(){
    print_table_top(40);
    print_table_text(40, "Delete account");
    print_table_bottom(40);
    Account temp;
    uint8_t account_count = 0;
    rewind(accounts_file);
    while(fread(&temp, sizeof(temp), 1, accounts_file) == 1){
        if(temp.id != 9999){
            printf(" ID: %d\r\n", temp.id);
            printf(" Name: %s\r\n", temp.name);
            printf(" Balance: %.2f\r\n\r\n", temp.balance);
        }
        account_count++;
    }
    if(account_count == 1) { printf(" No user accounts created.\r\n\r\n"); return; }

    uint8_t attempts = 0;
    uint16_t ID;
    bool ID_exists = false;
    while(attempts < 3){
        ID = read_integer("ID", 9999);
        rewind(accounts_file);
        while(fread(&temp, sizeof(temp), 1, accounts_file) == 1){
            if(temp.id == ID) {
                ID_exists = true;
                break;
            }
        }
        if(ID_exists) break;

        printf(" ID does not exist. Remaining attempts: %d\r\n\r\n", 2 - attempts);
        attempts++;
        
    }
    if(!ID_exists){
        write_log("ID:%d %s - Account deletion failed.", current_user.id, current_user.name);
        return;
    }

    FILE *temp_file = fopen("temp.dat", "wb");
    if (temp_file == NULL) {
        perror("Failed to create temporary file");
        return; 
    }

    rewind(accounts_file);
    while (fread(&temp, sizeof(temp), 1, accounts_file) == 1) {
        if (temp.id != ID) {
            fwrite(&temp, sizeof(temp), 1, temp_file);
        }
    }

    fclose(accounts_file);
    fclose(temp_file);

    remove("ucty.dat");
    rename("temp.dat", "ucty.dat");

    accounts_file = fopen("ucty.dat", "rb+");  /* reopen main file */
    if (!accounts_file) {
        perror("Failed to reopen ucty.dat");
        exit(1);
    }

    write_log("ID:%d %s - Account with ID: %d successfully deleted", current_user.id, current_user.name, ID);
    printf(" Account with ID %d was successfully deleted.\r\n\r\n", ID);
}

/****************************************************************************************************************************************/
/********************************************  Helper functions  ************************************************************************/
/****************************************************************************************************************************************/

/* Read integer from stdin with validation */
uint16_t read_integer(const char *prompt, uint16_t max_count){
    int number;
    char ch;
    while (1) {
        printf(" %s: ", prompt);
        fflush(stdout);
        if (scanf("%d%c", &number, &ch) == 2) {
            if (ch == '\n' && number >= (max_count / 10) + 1 && number <= max_count)
                return (uint16_t)number;
        }
        
        while ((ch = getchar()) != '\n' && ch != EOF);
        
        if(max_count < 10){
            printf(" %s must be a single-digit integer from 1 to %d. Enter %s again.\r\n", prompt, max_count, prompt); continue;
        }else if(max_count < 100){
            printf(" %s must be a two-digit integer from 10 to %d. Enter %s again.\r\n", prompt, max_count, prompt); continue;
        }else if(max_count < 1000){
            printf(" %s must be a three-digit integer from 100 to %d. Enter %s again.\r\n", prompt, max_count, prompt); continue;
        }else if(max_count < 10000){
            printf(" %s must be a four-digit integer from 1000 to %d. Enter %s again.\r\n", prompt, max_count, prompt); continue;
        }else if(max_count < 100000) {
            printf(" %s must be a five-digit integer from 10000 to %d. Enter %s again.\r\n", prompt, max_count, prompt); continue;
        }else printf(" %s must be an integer. Enter %s again.\r\n", prompt, prompt);
    }
}

/* Read positive double from stdin */
double read_double(const char *prompt){
    double number;
    char ch;
    while (1) {
        printf(" %s: ", prompt);
        fflush(stdout);
        if (scanf("%lf%c", &number, &ch) == 2 && ch == '\n' && number > 0) return number;
        printf(" %s must be a positive real number. Enter %s again.\r\n", prompt, prompt);
        while ((ch = getchar()) != '\n' && ch != EOF);
    }
}

/* Update PIN in the accounts file for given ID */
void update_pin(uint16_t id, uint16_t new_pin){
    rewind(accounts_file);
    Account a;
    long pos;

    while ((pos = ftell(accounts_file)), fread(&a, sizeof(Account), 1, accounts_file) == 1) {
        if (a.id == id) {
            a.pin = new_pin;

            fseek(accounts_file, pos, SEEK_SET); /* move back to start of this record */

            fwrite(&a, sizeof(a), 1, accounts_file);
            fflush(accounts_file);
        }
    }
}

/* Update balance in the accounts file for given ID */
void update_balance(uint16_t id, double amount){
    rewind(accounts_file);
    Account a;
    long pos;

    while ((pos = ftell(accounts_file)), fread(&a, sizeof(Account), 1, accounts_file) == 1) {
        if (a.id == id) {
            a.balance += amount;

            fseek(accounts_file, pos, SEEK_SET); /* move back to start of this record */

            fwrite(&a, sizeof(a), 1, accounts_file);
            fflush(accounts_file);
        }
    }
}

/* Write timestamped entry to log file */
void write_log(const char *format, ...){
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char timestr[30];

    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(logs_file, "[%s] ", timestr);

    va_list args;
    va_start(args, format);
    vfprintf(logs_file, format, args);
    va_end(args);

    fprintf(logs_file, "\n");
    fflush(logs_file);
}

/* Simple ASCII/box printing helpers */
void print_table_top(uint8_t width){
    printf("  %c", LT);
    for(int i = 0; i < width; i++) printf("%c", H);
    printf("%c\r\n", RT);
}
void print_table_text(uint8_t width, const char *text){
    int i;
    uint8_t text_len = strlen(text);
    uint8_t pad = (width - text_len) / 2;

    printf("  %c", V);
    for(i = 0; i < pad; i++) printf(" ");
    printf("%s", text);
    for(i = 0; i < width - text_len - pad; i++) printf(" ");
    printf("%c\r\n", V);
}
void print_table_bottom(uint8_t width){
    printf("  %c", LB);
    for(int i = 0; i < width; i++) printf("%c", H);
    printf("%c\r\n\r\n", RB);
}
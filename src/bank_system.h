#ifndef BANK_SYSTEM_H
#define BANK_SYSTEM_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Application states */
typedef enum{
    INIT,
    LOGIN,
    NEW_ACCOUNT,
    MAIN_MENU,
    BALANCE,
    DEPOSIT,
    WITHDRAWAL,
    CHANGE_PIN,
    LOGOUT,
    EXIT_APP,
    ADMIN_MENU,
    ACCOUNTS,
    DELETE_ACCOUNT
} State_t;

/* Account structure stored in ucty.dat */
typedef struct{
    uint16_t id;
    char name[50];
    uint16_t pin;
    double balance;
} Account;

/* Start the state machine loop */
void state_machine();

#endif
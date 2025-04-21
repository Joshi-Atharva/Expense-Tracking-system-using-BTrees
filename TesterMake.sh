#!/bin/bash
gcc -g tester.c utils.c UserCrud.c FamilyCrud.c ExpenseCrud.c -o tester 
./tester
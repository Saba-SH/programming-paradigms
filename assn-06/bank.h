#ifndef _BANK_H
#define _BANK_H

#include <semaphore.h>

typedef unsigned long AccountNumber;
typedef int64_t AccountAmount;
typedef unsigned long BranchID;

typedef struct Bank {
  unsigned int numberBranches;
  unsigned int numWorking;
  pthread_mutex_t workEnd;
  sem_t startNextDay;
  pthread_mutex_t reportTransferLock;
  struct       Branch  *branches;
  struct       Report  *report;
} Bank;

int Bank_Balance(Bank *bank, AccountAmount *balance);

Bank *Bank_Init(int numBranches, int numAccounts, AccountAmount initAmount,
                AccountAmount reportingAmount,
                int numWorkers);

int Bank_Validate(Bank *bank);
int Bank_Compare(Bank *bank1, Bank *bank2);



#endif /* _BANK_H */

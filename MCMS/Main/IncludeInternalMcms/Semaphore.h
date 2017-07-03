// Semaphore.h

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <string>
#include <iostream>

#include "DataTypes.h"
#include "StatusesGeneral.h"
#include "SharedDefines.h"

STATUS         RemoveSemaphoreIfExists(std::string& Path);
STATUS         CreateSemaphore(int* sid,
                               std::string& SemName,
                               BOOL multiProcess = FALSE);
STATUS         RemoveSemaphore(int sid);
STATUS         LockSemaphore(int sid);
STATUS         UnlockSemaphore(int sid);
STATUS         UndoSemaphore(int sid);
unsigned short get_member_count(int sid);

#endif


// Semaphore.cpp

#include "Semaphore.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "OsFileIF.h"
#include "TraceStream.h"
#include "DefinesGeneral.h"

STATUS CreateSemaphore(int* sid,
                       std::string& SemName,
                       BOOL multiProcess)
{
  key_t key;
  std::string SemPathFileName = MCU_TMP_DIR+"/semaphore/" + SemName;

  if (!multiProcess)
    RemoveSemaphoreIfExists(SemPathFileName);

  if (!CreateFile(SemPathFileName))
  {
    if (!multiProcess)
      return STATUS_FAIL;
  }

  if (-1 == (key = ftok(SemPathFileName.c_str(), 's')))
  {
    FTRACEWARN << "ftok: " << SemPathFileName.c_str()
               << ": " << strerror(errno) << " (" << errno << ")";
    return STATUS_FAIL;
  }

  if ((*sid = semget(key, 0, 0)) == -1 && errno == ENOENT)
  {
    if ((*sid = semget(key, 1, IPC_CREAT|IPC_EXCL|0666)) == -1)
    {
      FTRACEWARN << "semget: " << SemPathFileName.c_str()
                 << ": " << strerror(errno) << " (" << errno << ")";
      return STATUS_FAIL;
    }

    if (-1 == semctl(*sid, 0, SETVAL, 1))
    {
      FTRACEWARN << "semctl: " << SemPathFileName.c_str()
                 << ": " << strerror(errno) << " (" << errno << ")";
      return STATUS_FAIL;
    }
  }

  return STATUS_OK;
}

STATUS RemoveSemaphore(int sid)
{
  if (-1 == sid)
	  return STATUS_OK;
  if (-1 == semctl(sid, 0, IPC_RMID, 0))
  {
    FTRACEWARN << "semctl: " << sid
               << ": " << strerror(errno) << " (" << errno << ")";
    return STATUS_FAIL;
  }

  return STATUS_OK;
}

STATUS UndoSemaphore(int sid)
{
  int retval = semctl(sid, 0, GETVAL, 0);
  switch (retval)
  {
    case -1:
      FTRACEWARN << "semctl: " << sid
                 << ": " << strerror(errno) << " (" << errno << ")";
      return STATUS_FAIL;

    case 0:
      return UnlockSemaphore(sid);

    default:
      return STATUS_OK;
  }

  return STATUS_OK;
}

STATUS LockSemaphore(int sid)
{
  // SEM_UNDO maintains the sempahore , so when
  // a thread exits , it will release it's semaphore,
  // in case it was in posession of one.
  if (-1==sid)
	  return STATUS_FAIL;
  struct sembuf sem_lock = { 0, -1, SEM_UNDO};
  int res = 0;
  while ((res = semop(sid, &sem_lock, 1)))
  {
    if (errno != EINTR)
    {
      FTRACEWARN << "semop: " << sid
                 << ": " << strerror(errno) << " (" << errno << ")";
      break;
    }
  }

  if (-1 == res)
    return STATUS_FAIL;

  return STATUS_OK;
}

STATUS UnlockSemaphore(int sid)
{
  if (-1==sid)
  {
	  FTRACEWARN << "sid is negative";
	  return STATUS_FAIL;
  }
  struct sembuf sem_lock = { 0, 1, IPC_NOWAIT|SEM_UNDO};
  int res = 0;
  while ((res = semop(sid, &sem_lock, 1)))
  {
    if (errno != EINTR)
    {
      FTRACEWARN << "semop: " << sid
                 << ": " << strerror(errno) << " (" << errno << ")";
      break;
    }
  }

  if (-1 == res)
    return STATUS_FAIL;

  return STATUS_OK;
}

STATUS RemoveSemaphoreIfExists(std::string& fname)
{
  if (!IsFileExists(fname))
    return STATUS_OK;

  key_t key;
  int   l_sid;
  if (-1 == (key = ftok(fname.c_str(), 's')))
  {
    FTRACEWARN << "ftok: " << fname.c_str()
               << ": " << strerror(errno) << " (" << errno << ")";
    return STATUS_FAIL;
  }

  if ((l_sid = semget(key, 0, 0)) == -1 && errno == ENOENT)
  {
    FTRACEWARN << "semget: " << fname.c_str()
               << ": " << strerror(errno) << " (" << errno << ")";
    return STATUS_FAIL;
  }

  RemoveSemaphore(l_sid);
  unlink(fname.c_str());

  return STATUS_OK;
}

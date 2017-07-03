// FipsMode.h

#ifndef FIPS_MODE_H_
#define FIPS_MODE_H_

bool GetSNMPFipsMode();

int TestAndEnterFipsMode(bool alwaysFIPS = true);

#endif  // FIPS_MODE_H_

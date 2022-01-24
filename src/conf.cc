#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif

_Atomic(int) gFlipLeft(0);
_Atomic(int) gFlipRight(0);
_Atomic(int) gVehicleType(0);

int GetFilpLeft()
{
  int res = gFlipLeft.load();
  return res;
}

int SetFlipLeft(int desired)
{
  gFlipLeft.store(desired);
  return 0;
}

int GetFilpRight()
{
  int res = gFlipRight.load();
  return res;
}

int SetFlipRight(int desired)
{
  gFlipRight.store(desired);
  return 0;
}

int GetVehicleType()
{
  int res = gVehicleType.load();
  return res;
}

int SetVehicleTypet(int desired)
{
  gVehicleType.store(desired);
  return 0;
}

int InitConf()
{
  //TODO
  return 0;
}


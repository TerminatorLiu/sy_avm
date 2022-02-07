#include <conf.h>
#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif

_Atomic(int) gFlipLeft(0);
_Atomic(int) gFlipRight(0);
_Atomic(int) gVehicleType(0);
_Atomic(int) gVehicleImageNeedUpdate(0);

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

int SetVehicleType(int desired)
{
  gVehicleType.store(desired);
  return 0;
}

int SetVehicleImageNeedUpdate()
{
  gVehicleImageNeedUpdate.store(1);
  return 0;
}
int ResetVehicleImageNeedUpdate()
{
  gVehicleImageNeedUpdate.store(0);
  return 0;
}
int VehicleImageNeedUpdate()
{
  int res = gVehicleImageNeedUpdate.load();
  return res;
}
int InitConf()
{
  //TODO
  return 0;
}


#include <pthread.h>
#include "lua.h"
#include "luauser.h"

static pthread_mutex_t lua_mutex = PTHREAD_MUTEX_INITIALIZER;

void LuaLock(lua_State * L)
{
  pthread_mutex_lock(&lua_mutex);
}

void LuaUnlock(lua_State * L)
{ 
  pthread_mutex_unlock(&lua_mutex);
}


/*
static struct {
  pthread_mutex_t mutex;
  int init;
} Gl;

void LuaLockInitial(lua_State * L) 
{ 
  if (!Gl.init) {
    // Create mutex
    pthread_mutex_init(&Gl.mutex, NULL);
    Gl.init = 1;
  }
}

void LuaLockFinal(lua_State * L)
{ 
  // Destroy mutex
  if (Gl.init)
    {
      pthread_mutex_destroy(&Gl.mutex);
      Gl.init = 0;
    }
}

void LuaLock(lua_State * L)
{
  // Lock mutex
  pthread_mutex_lock(&Gl.mutex);
}

void LuaUnlock(lua_State * L)
{ 
  // Unlock mutex
  pthread_mutex_unlock(&Gl.mutex);
}
*/

#ifndef luauser_h
#define luauser_h

void LuaLock(lua_State * L);
void LuaUnlock(lua_State * L);
#define lua_lock(L) LuaLock(L)
#define lua_unlock(L) LuaUnlock(L)

/*
#ifdef luai_userstateopen
#undef luai_userstateopen
#endif
#define luai_userstateopen(L) LuaLockInitial(L)

#ifdef luai_userstatethread
#undef luai_userstatethread
#endif
#define luai_userstatethread(L,L1) LuaLockInitial(L1)

#ifdef luai_userstateclose
#undef luai_userstateclose
#endif
#define luai_userstateclose(L) LuaLockFinal(L)

void LuaLockInitial(lua_State * L);
void LuaLockFinal(lua_State * L);
*/

#endif

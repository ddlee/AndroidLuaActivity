/*
  Lua module to access Android assets
*/

#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "jnicontext.h"

#ifdef __cplusplus
extern "C"
{
#endif
  #include "lua.h"
  #include "lauxlib.h"
#ifdef __cplusplus
}
#endif

#include "luaasset.h"

#ifndef LOG_TAG
#define LOG_TAG "lua"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static AAssetManager* assetManager;

#define MODULENAME "asset"
#define ASSETPATH "?.lua;lua/?.lua;lua/?/init.lua"

#define BUFFERSIZE 1024
typedef struct LoadA {
  int extraline;
  AAsset *asset;
  char buff[BUFFERSIZE];
} LoadA;

#ifdef __cplusplus
extern "C"
#endif
// Read asset as char array
const char *getA(lua_State *L, void *ud, size_t *size) {
  LoadA *la = (LoadA *)ud;
  (void)L;
  if (la->extraline) {
    la->extraline = 0;
    *size = 1;
    return "\n";
  }
  *size = AAsset_read(la->asset, la->buff, sizeof(la->buff));
  return (*size > 0) ? la->buff : NULL;
}

LUA_API int luaL_loadasset(lua_State *L, const char *filename) {
  LoadA la;

  AAsset *asset = AAssetManager_open(assetManager,
				     filename,
				     AASSET_MODE_STREAMING);
  if (asset == NULL) {
    LOGE("Cannot open asset %s", filename);
    lua_pushfstring(L, "cannot open asset %s", filename);
    return LUA_ERRFILE;
  }

  la.extraline = 0;
  la.asset = asset;
  int status = lua_load(L, getA, &la, filename);
  AAsset_close(asset);

  return status;
}

static int lua_asset_loadfile(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  if (luaL_loadasset(L, filename) == 0) {
    // Successfully loaded file chunk
    return 1;
  }
  else {
    // Error loading file
    lua_pushnil(L);
    lua_insert(L, -2);
    return 2;
  }
}

static int lua_asset_dofile(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  if (luaL_doasset(L, filename)) {
    LOGE("asset error %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return 0;
  }
  return lua_gettop(L);
}

static int lua_asset_list(lua_State *L) {
  const char *path = luaL_optstring(L, 1, "");

  AAssetDir *assetDir = AAssetManager_openDir(assetManager,
					      path);
  if (assetDir == NULL) {
    LOGE("Cannot open asset path %s", path);
    lua_pushfstring(L, "cannot open asset path %s", path);
    return LUA_ERRFILE;
  }

  AAssetDir_rewind(assetDir);
  int nfiles = 0;
  const char* filename;
  lua_createtable(L, 0, 0);
  while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
    nfiles++;
    lua_pushstring(L, filename);
    lua_rawseti(L, -2, nfiles);
  }
  AAssetDir_close(assetDir);

  return 1;
}

// Check if asset exists
static int readable(const char *filename) {
  AAsset *asset = AAssetManager_open(assetManager,
				     filename,
				     AASSET_MODE_STREAMING);
  if (asset == NULL) return 0; // open failed
  AAsset_close(asset);
  return 1;
}

static int lua_asset_readable(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  if (readable(filename)) {
    lua_pushboolean(L, 1);
  }
  else {
    lua_pushboolean(L, 0);
  }
  return 1;
}

static int lua_asset_pointer(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  AAsset *asset = AAssetManager_open(assetManager,
				     filename,
				     AASSET_MODE_BUFFER);
  if (asset == NULL) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushlightuserdata(L, (void *)AAsset_getBuffer(asset));
  AAsset_close(asset);
  return 1;
}

static int lua_asset_length(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  AAsset *asset = AAssetManager_open(assetManager,
				     filename,
				     AASSET_MODE_STREAMING);
  if (asset == NULL) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushinteger(L, AAsset_getLength(asset));
  AAsset_close(asset);
  return 1;
}

static int lua_asset_string(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);

  AAsset *asset = AAssetManager_open(assetManager,
				     filename,
				     AASSET_MODE_BUFFER);
  if (asset == NULL) {
    LOGE("Cannot open asset %s", filename);
    lua_pushfstring(L, "cannot open asset %s", filename);
    return LUA_ERRFILE;
  }

  const char* str = (const char*)AAsset_getBuffer(asset);
  if (str == NULL) {
    AAsset_close(asset);
    return 0;
  }
  off_t len = AAsset_getLength(asset);
  lua_pushlstring(L, str, len);

  AAsset_close(asset);
  return 1;
}

// Find next search template in path string
static const char *pushnexttemplate (lua_State *L, const char *path) {
  const char *l;
  while (*path == *LUA_PATHSEP) path++;  /* skip separators */
  if (*path == '\0') return NULL;  /* no more templates */
  l = strchr(path, *LUA_PATHSEP);  /* find next separator */
  if (l == NULL) l = path + strlen(path);
  lua_pushlstring(L, path, l - path);  /* template */
  return l;
}

// Iterate through path string to find asset
static const char *findasset(lua_State *L, const char *name,
			     const char *path) {
  // Search for readable filename by substituting name in path
  lua_pushliteral(L, "");  /* error accumulator */
  while ((path = pushnexttemplate(L, path)) != NULL) {
    const char *filename;
    filename = luaL_gsub(L, lua_tostring(L, -1), "?", name);
    lua_remove(L, -2);  /* remove path template */
    if (readable(filename))  /* does file exist and is readable? */
      return filename;  /* return that file name */
    lua_pushfstring(L, "\n\tno asset %s", filename);
    lua_remove(L, -2);  /* remove file name */
    lua_concat(L, 2);  /* add entry to possible error message */
  }
  return NULL;  /* not found */
}

static int lua_asset_loader(lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);

  // Replace '.' with '/' in name
  name = luaL_gsub(L, name, ".", "/");

  /*
  // Retrieve asset["path"] from globals:
  lua_getfield(L, LUA_GLOBALSINDEX, MODULENAME);
  lua_getfield(L, -1, "path");
  lua_remove(L, -2); // module table
  */
  // C closure: asset table as upvalue, get asset["path"]
  lua_getfield(L, lua_upvalueindex(1), "path");

  const char *path = lua_tostring(L, -1);
  //  LOGI("asset.path: %s", path);

  if (path == NULL)
    luaL_error(L, "asset.path must be a string");

  filename = findasset(L, name, path);
  if (filename == NULL) return 1;  // library not found in this path
  if (luaL_loadasset(L, filename) != 0) {
    luaL_error(L, "error loading module %s from asset %s:\n\t%s",
	       lua_tostring(L, 1), filename, lua_tostring(L, -1));
  }
  return 1; // library loaded successfully
}


static const struct luaL_reg asset_lib[] = {
  {"loadfile", lua_asset_loadfile},
  {"dofile", lua_asset_dofile},
  {"list", lua_asset_list},
  {"readable", lua_asset_readable},
  {"pointer", lua_asset_pointer},
  {"length", lua_asset_length},
  {"string", lua_asset_string},
  {NULL, NULL}
};

LUALIB_API
int luaopen_asset (lua_State *L) {
  LOGI("luaopen_asset called");

  assetManager = jniGetAssetManager();
  /*
  // Uses Java VM and application context
  JNIEnv *env = jniGetEnv();
  // Problem using application context in emulator:
  // jobject context = jniGetApplicationContext();
  jobject context = jniGetActivity();
  jmethodID methodGetAssets =
    env->GetMethodID(env->GetObjectClass(context),
		     "getAssets",
		     "()Landroid/content/res/AssetManager;");
  if (methodGetAssets == 0) {
    LOGE("Could not get getAssets method");
    return 0;
  }
  jobject localAssetManager = 
    env->CallObjectMethod(context,
			  methodGetAssets);
  if (localAssetManager == 0) {
    LOGE("Could not get local Java Asset Manager");
    return 0;
  }
  jobject globalAssetManager = env->NewGlobalRef(localAssetManager);
  if (globalAssetManager == 0) {
    LOGE("Could not get global Java Asset Manager");
    return 0;
  }

  assetManager = AAssetManager_fromJava(env, globalAssetManager);
  */

  if (!assetManager) {
    LOGE("No asset manager");
    return 0;
  }

  // Register module functions:
  luaL_register(L, MODULENAME, asset_lib);


  // Following is used to add asset loader for require() to package.loaders:
  // Equivalent to the following lua code:
/* Lua code:
require("package")
package.assetpath =
   "?.lua;lua/?.lua;lua/?/init.lua;lib/?.lua;lib/?/init.lua;lib/lua/?.lua;lib/lua/?/init.lua";
package.assetloader = 
   function (name)
      name = string.gsub(name, '%.', '/');
      local path
      for path in string.gmatch(package.assetpath, "[^;]+") do
         fname = string.gsub(path, '?', name);
         print("lua asset loader:", fname)
         if (asset.readable(fname)) then
            return asset.loadfile(fname);
         end
      end
   end
--package.loaders[#package.loaders+1] = package.assetloader;
table.insert(package.loaders, 2, package.assetloader);
*/


  // Initialize path variable
  lua_pushstring(L, ASSETPATH);
  lua_setfield(L, -2, "path");

  // Create loader C closure
  lua_pushvalue(L, -1); // Copy module table as upvalue
  lua_pushcclosure(L, lua_asset_loader, 1);
  lua_setfield(L, -2, "loader");

  // Add asset loader function to package.loaders:
  int loader_pos = 2;  // Install after first loader that checks package.preload
  lua_getfield(L, LUA_GLOBALSINDEX, "package");
  if (lua_istable(L, -1)) {
    lua_getfield(L, -1, "loaders");
    if (lua_istable(L, -1)) {
      int nloaders = lua_objlen(L, -1);
      for (int i = nloaders; i>=loader_pos; i--) {
	lua_rawgeti(L, -1, i);
	lua_rawseti(L, -2, i+1); // t[i+1] = t[i];
      }
      // Set package.loaders[loader_pos] = asset.loader
      lua_getfield(L, -3, "loader");
      lua_rawseti(L, -2, loader_pos);
    }
    lua_pop(L, 1); // package.loaders table or nil
  }
  lua_pop(L, 1); // package table or nil

  return 1;
}

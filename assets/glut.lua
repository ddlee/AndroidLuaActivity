module(..., package.seeall);

require('egl');
require('inputevent');
require('unix');

RGB = 0;
RGBA = 0;
INDEX = 1;
SINGLE = 0;
DOUBLE = 2;
ACCUM = 4;
ALPHA = 8;
DEPTH = 1*16;
STENCIL = 2*16;
MULTISAMPLE = 8*16;
STEREO = 256;
LUMINANCE = 2*256;

local fReshape = nil;
local fDisplay = nil;
local fKeyboard = nil;
local fKeyboardUp = nil;
local fPassiveMotion = nil;
local fIdle = nil;
local timerEvent = nil;

local winName = nil;
local width = 300;
local height = 300;

local eglObj = egl.new();

local function onNativeWindowCreated(window)
   print("glut.onNativeWindowCreated ", window);

   eglObj:ChooseConfig({SURFACE_TYPE=egl.WINDOW_BIT,
                        RED_SIZE=8, GREEN_SIZE=8, BLUE_SIZE=8});
   eglObj:CreateSurface(window);
   w,h = eglObj:QuerySurfaceSize();

   if (fReshape) then
      fReshape(width, height);
   else
      gl.Viewport(0,0,width,height);
   end

   if (fDisplay) then
      fDisplay();
   end
end

local function onNativeWindowDestroyed(window)
   print("glut.onNativeWindowDestroyed", window);
end

local function onInputEvent(event)
   local type = inputevent.getType(event);
   local action = inputevent.getAction(event);
   local eventTime = inputevent.getEventTime(event);

   --Key event
   if (type == inputevent.AINPUT_EVENT_TYPE_KEY) then
      local key = inputevent.getKeyCode(event);
--      if (key == 4) then print("Back key") end
--      if (key == 82) then print("Menu key") end
      if (inputevent.getAction(event) == inputevent.AKEY_EVENT_ACTION_DOWN) then
	 if (fKeyboard) then
	    fKeyboard(key);
	 end
      elseif (inputevent.getAction(event) == inputevent.AKEY_EVENT_ACTION_UP) then
	 if (fKeyboardUp) then
	    fKeyboardUp(key);
	 end
      end
      return;
   end

   --Motion event
   if (type == inputevent.AINPUT_EVENT_TYPE_MOTION) then
      local x = inputevent.getX(event);
      local y = inputevent.getY(event);
      for i = 1,#x do
	 if (fPassiveMotion) then
	    fPassiveMotion(x[i], y[i]);
	 end
      end
   end
end

--Define global functions for luaactivity callbacks:
_G.onNativeWindowCreated = onNativeWindowCreated;
_G.onNativeWindowDestroyed = onNativeWindowDestroyed;
_G.onInputEvent = onInputEvent;


local function Stub() print("Unimplemented GLUT function"); end

function Init(argc, argv) end
function InitDisplayMode(mode) end
function InitWindowSize(w, h)
   width = w;
   height = h;
end
function InitWindowPosition(x, y) end

function CreateWindow(name) end
CreateSubWindow = Stub;
function DestroyWindow() end

function SwapBuffers()
   eglObj:SwapBuffers();
end

function DisplayFunc(f) fDisplay = f; end
function OverlayDisplayFunc(f) end
function ReshapeFunc(f) fReshape = f; end
function KeyboardFunc(f) end
function MouseFunc(f) end
function MotionFunc(f) end
function PassiveMotionFunc(f) fPassiveMotion = f; end
VisibilityFunc = Stub;
EntryFunc = Stub;
function SpecialFunc(f) end

function IdleFunc(f) fIdle = f; end
function TimerFunc(msecs, func, value)
   timerEvent = { t = unix.time() + msecs/1000,
		  f = func, v = value };
end

BitmapCharacter = Stub;
BitmapCharacterWidth = Stub;
StrokeCharacter = Stub;
StrokeCharacterWidth = Stub;

function WMCloseFunc(f)
end


-- Could possibly use coroutines to implement main loop
-- by calling "coroutine.resume()" through uipost...

local dt = 0.01;
local loop = true;
function main()
   if ((timerEvent) and (unix.time() > timerEvent.t)) then
      local f = timerEvent.f;
      local v = timerEvent.v;
      timerEvent = nil;
      f(v);
   end
   if (fIdle) then
      fIdle();
   end
   unix.usleep(1E6*dt);

   if (loop) then
      uipost("glut.main()");
   end
end

function MainLoop()
   uipost("glut.main()");
end

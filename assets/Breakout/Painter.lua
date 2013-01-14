module(..., package.seeall);

require('math');
require('gles1');
require('carray');

RED = 'red';
ORANGE = 'orange';
YELLOW = 'yellow';
GREEN = 'green';
WHITE = 'white';
BLACK = 'black';

local mt = getfenv();
mt.__index = mt;

function new()
   local obj = {};
   setmetatable(obj, mt);
   return obj;
end


local vRect = carray.new("float",
			 {0, 0,
			  1, 0,
			  0, 1,
			  1, 1});

function bar(self, x1, y1, x2, y2)
   --[[
   local v = carray.new("float",
			{x1, y1,
			 x2, y1,
			 x1, y2,
			 x2, y2});
   --]]
   gl.PushMatrix();
   gl.Translatef(x1, y1, 0);
   gl.Scalef(x2-x1, y2-y1, 0);
   gl.EnableClientState(gl.VERTEX_ARRAY);
   gl.VertexPointer(2, gl.FLOAT, 0, carray.pointer(vRect));
   gl.DrawArrays(gl.TRIANGLE_STRIP, 0, 4);
   gl.DisableClientState(gl.VERTEX_ARRAY);
   gl.PopMatrix();
end

local nCircle = 90;
local vCircle = carray.new("float", 2*(nCircle+1));
vCircle[1] = 0;
vCircle[2] = 0;
for i = 1,nCircle do
   local k = 2*i+1;
   local a = 2*math.pi*(i-1)/(nCircle-1);
   vCircle[k] = math.cos(a);
   vCircle[k+1] = math.sin(a);
end

function ball(self, x, y)
   --[[
   local v = carray.new("float",
			{x+3, y,
			 x, y+3,
			 x-3, y,
			 x, y-3});
   --]]
   gl.PushMatrix();
   gl.Translatef(x, y, 0);
   gl.Scalef(3, 3, 0);
   gl.EnableClientState(gl.VERTEX_ARRAY);
   gl.VertexPointer(2, gl.FLOAT, 0, carray.pointer(vCircle));
   gl.DrawArrays(gl.TRIANGLE_FAN, 0, nCircle+1);
   gl.DisableClientState(gl.VERTEX_ARRAY);
   gl.PopMatrix();
end

colorNames = {"yellow", "green", "orange", "red", "white", "black"};
colorTable = {{1,1,0},
	      {0,1,0},
	      {1,0.5,0},
	      {1,0,0},
	      {1,1,1},
	      {0,0,0}};
do
   local n = 0;
   local i;
   for i = 1,#colorNames do
      colorTable[colorNames[i]] = colorTable[i];
   end
end
nColor = #colorTable;

function setColor(self, name)
   c = colorTable[name];
   if (c) then
      gl.Color4f(c[1], c[2], c[3], 1);
   end
end

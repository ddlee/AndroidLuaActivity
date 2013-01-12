module(..., package.seeall);

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


local v8 = carray.new("float",
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
   v8[1] = x1; v8[2] = y1;
   v8[3] = x2; v8[4] = y1;
   v8[5] = x1; v8[6] = y2;
   v8[7] = x2; v8[8] = y2;   
   gl.EnableClientState(gl.VERTEX_ARRAY);
   gl.VertexPointer(2, gl.FLOAT, 0, carray.pointer(v8));
   gl.DrawArrays(gl.TRIANGLE_STRIP, 0, 4);
   gl.DisableClientState(gl.VERTEX_ARRAY);
end

function ball(self, x, y)
   --[[
   local v = carray.new("float",
			{x+3, y,
			 x, y+3,
			 x-3, y,
			 x, y-3});
   --]]
   v8[1] = x+3; v8[2] = y;
   v8[3] = x;   v8[4] = y+3;
   v8[5] = x-3; v8[6] = y;
   v8[7] = x;   v8[8] = y-3;   
   gl.EnableClientState(gl.VERTEX_ARRAY);
   gl.VertexPointer(2, gl.FLOAT, 0, carray.pointer(v8));
   gl.DrawArrays(gl.TRIANGLE_FAN, 0, 4);
   gl.DisableClientState(gl.VERTEX_ARRAY);
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
